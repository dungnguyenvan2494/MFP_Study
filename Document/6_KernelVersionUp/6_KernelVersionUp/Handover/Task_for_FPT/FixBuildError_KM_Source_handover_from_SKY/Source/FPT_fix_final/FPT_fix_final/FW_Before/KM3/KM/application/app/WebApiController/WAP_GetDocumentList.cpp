//-----------------------------------------------------------------------------.
// @file		WAP_GetDocumentList.cpp.
// @brief		文書リスト取得 個別コントローラクラス.
// @author		日立 田中.
// @since		2017/12/20.
// @date		2018/01/23 日立村上 2LAN対応_ボックスID指定の変更.
// @date		2018/03/12 日立田中 非同期改善対応.
//-----------------------------------------------------------------------------.
#include <stdio.h>
#include <string.h>
#include <memory.h>

#include "SYSC_OSWrapper.h"
#include "json/json.h"
#include "json/reader.h"
#include "json/writer.h"
#include "json/value.h"

#include "system/sys/sysg/LOGF_debug.h"

#include "system/job/APIC_SecureList.h"
#include "system/doc/APIC_Doc.h"
#include "system/doc/DOCC_BoxIdDef.h"
#include "system/nvd/APIC_Setting_SystemSetting.h"		// OP_BTS-11408 Naveen Added.

#include "WAP_plugintrace.h"
#include "WAP_BoxCommonLib.h"
#include "WAP_AsyncResponder.h"

#include "WAP_GetDocumentList.h"
#include "WAP_Power.h"

int32_t		WAPC_GetDocumentList::m_si32_getJobSpecResult = 0;	// req_GetJobSpec用リターン値.
SEM_ID		WAPC_GetDocumentList::m_sem_AnsGetJobSpec = 0;		// ジョブスペック取得待ちセマフォ.
SEM_ID		WAPC_GetDocumentList::m_sem_GetDocList = 0;

//-----------------------------------------------------------------------------.
// definitions.
//-----------------------------------------------------------------------------.

//-----------------------------------------------------------------------------.
// テーブル定義.
//-----------------------------------------------------------------------------.

//-----------------------------------------------------------------------------.
// WAPC_GetDocumentListクラス.
//-----------------------------------------------------------------------------.
/**
 * @brief		コンストラクタ.
 * @return		なし.
 * @author		日立 田中.
 * @since		2017/12/20.
 * @date        2023/03/02 日立佐藤菜 OP_BTS-40914 セマフォ生成済みの場合は生成しないように修正.
 */
WAPC_GetDocumentList::WAPC_GetDocumentList()
{
	// インスタンス取得.
	m_pc_Box = APIC_Box::newInstance();
	if( NULL == m_pc_Box ) {
		WAPG_API_trace("[ERR]%s APIC_Box::newInstance() is NULL!.\n", __PRETTY_FUNCTION__ );
		// Fatal通知.
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
	}

	// セマフォ生成(ジョブスペック取得待ち用).
	if( 0 == m_sem_AnsGetJobSpec ) {
		m_sem_AnsGetJobSpec = SYSF_semBCreate( SEM_Q_FIFO, SEM_EMPTY );
		if ( NULL == m_sem_AnsGetJobSpec ) {
			WAPG_API_trace("[ERR]%s SYSF_semBCreate() Error!.\n", __PRETTY_FUNCTION__ );
			// Fatal通知.
			LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
		}
	}
	// セマフォ生成(処理排他用).
	if( 0 == m_sem_GetDocList ) {
		m_sem_GetDocList = SYSF_semMCreate( SEM_Q_PRIORITY | SEM_DELETE_SAFE | SEM_INVERSION_SAFE );
		if( NULL == m_sem_GetDocList ) {
			WAPG_API_trace("[ERR]%s SYSF_semMCreate() Error!.\n", __PRETTY_FUNCTION__ );
			// Fatal通知.
			LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
		}
	}

	// メンバ変数初期化.
	memset( &m_s_DocListInfo, 0, sizeof(m_s_DocListInfo) );
	memset( &m_s_SecretDocListInfo, 0, sizeof(m_s_SecretDocListInfo) );	// OP_BTS-11408 Naveen Added.
	m_si32_UserId		= 0;
	// 2020/09/02 nomura Start.
	m_si32_BoxId = 0;
	// 2020/09/02 nomura End.
	m_si32_TrackId = 0;		// 2022/08/19 FPT-TuanBV1 IT6_2.3 OP_BTS-39017 Modified.
}

/**
 * @brief		デストラクタ.
 * @return		なし.
 * @author		日立 田中.
 * @since		2017/12/20.
 */
WAPC_GetDocumentList::~WAPC_GetDocumentList()
{
	m_pc_Box->deleteInstance();
}

/**
 * @brief		コントローラの初期化.
 * @return 		初期化の成否, WAPD_NoError(0)：成功、WAPD_AnyError(1)：失敗.
 * @author		日立 田中.
 * @since		2017/12/20.
 */
int32_t WAPC_GetDocumentList::Initialize()
{
	return WAPD_NoError;
}

/**
 * @brief		コントローラの終了.
 * @return		処理結果, WAPD_NoError(0)：成功、WAPD_AnyError(1)：失敗.
 * @author		日立 田中.
 * @since		2017/12/20.
 */
int32_t WAPC_GetDocumentList::Finalize()
{
	return WAPD_NoError;
}

/**
 * @brief		コントローラのレスポンス生成.
 * @param[in]	ps_Param パラメータ構造体.
 * @return		レスポンス生成の成否, WAPD_NoError(0)：成功、WAPD_AnyError(1)：失敗.
 * @author		日立 田中.
 * @since		2017/12/20.
 * @date		2018/03/12 日立田中 非同期改善対応.
 */
int32_t WAPC_GetDocumentList::ResponseGenerate( WAPS_SubController* ps_Param )
{
	int32_t si32_Ret = WAPD_AnyError;

	// パラメータNULLチェック.
	if( NULL == ps_Param ) {
		WAPG_API_trace("[ERR]%s NULL Parameter!.\n", __PRETTY_FUNCTION__ );
		// Fatal通知.
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
		// クライアントへ結果返却.
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		return WAPD_AnyError;
	}

	// 管理データアクセス用インスタンス取得.
	WAPC_GetDocumentList_DataMng*	pc_DataMng = WAPC_GetDocumentList_DataMng::newInstance();
	if( NULL == pc_DataMng ) {
		WAPG_API_trace("[ERR]%s WAPC_GetDocumentList_DataMng::newInstance() is NULL!.\n", __PRETTY_FUNCTION__ );
		// Fatal通知.
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
		// クライアントへ結果返却.
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		return WAPD_AnyError;
	}

	// リクエスト処理の排他用セマフォ取得.
	pc_DataMng->semTake();

	// リクエストメソッド分岐.
	switch( ps_Param->Method ) {
		case WAPD_HTTP_POST:
			// WebAPI電源管理.
			WAPC_Power* pc_Power = WAPC_Power::newInstance();
			if ( NULL == pc_Power ) {
			    WAPG_API_trace("%s(%d) WAPC_Power::newInstance NULL 1\n", __FILE__, __LINE__);
			    LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, WAP_LOGF_LEVEL_FATAL, "WAPC_Power::newInstance NULL 1");
			}

			// HDDデータアクセス用電源管理の省電力抑止.
			int32_t si32_ret = pc_Power->waitStartAccessHDD();
			if( WAPD_AnyError == si32_ret ){
				WAPG_API_trace("%s(%d) WAPC_Power::waitStartAccessHDD Error\n", __FILE__, __LINE__);
			}

			si32_Ret = ExecPostRequest(ps_Param);

			// HDDデータアクセス用電源管理の省電力抑止解除.
			si32_ret = pc_Power->EndAccessHDD();
			if( WAPD_AnyError == si32_ret ){
				WAPG_API_trace("%s(%d) WAPC_Power::EndAccessHDD Error\n", __FILE__, __LINE__);
			}
			break;

		case WAPD_HTTP_GET:
		case WAPD_HTTP_PUT:
		case WAPD_HTTP_DELETE:
		default:
			// [リクエスト不正].
			WAPG_API_trace("[ERR]%s Request Error!.\n", __PRETTY_FUNCTION__ );
			// クライアントへ結果返却.
			printf("405:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //405->ErrorStatus[WAPD_STATUS_METHOD_NOT_ALLOWED] in WAP_defs.h
			m_ResponseHeader->Status = WAPD_STATUS_METHOD_NOT_ALLOWED;
			break;
	}

	// リクエスト処理の排他用セマフォ開放.
	pc_DataMng->semGive();

	return si32_Ret;
}

/**
 * @brief		コントローラの非同期レスポンス生成.
 * @param[in]	ps_Param パラメータ構造体.
 * @return		レスポンス生成の成否, WAPD_NoError(0)：成功、WAPD_AnyError(1)：失敗.
 * @author		日立 田中.
 * @since		2018/03/12.
 */
int32_t WAPC_GetDocumentList::AsyncResponseGenerate( WAPS_SubController* ps_Param )
{
	int32_t si32_Ret = WAPD_AnyError;

	// パラメータNULLチェック.
	if( NULL == ps_Param ) {
		WAPG_API_trace("[ERR]%s NULL Parameter!.\n", __PRETTY_FUNCTION__ );
		// Fatal通知.
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
		// クライアントへ結果返却.
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		return WAPD_AnyError;
	}

	// 管理データアクセス用インスタンス取得.
	WAPC_GetDocumentList_DataMng*	pc_DataMng = WAPC_GetDocumentList_DataMng::newInstance();
	if( NULL == pc_DataMng ) {
		WAPG_API_trace("[ERR]%s WAPC_GetDocumentList_DataMng::newInstance() is NULL!.\n", __PRETTY_FUNCTION__ );
		// Fatal通知.
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
		// クライアントへ結果返却.
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		return WAPD_AnyError;
	}

	// リクエスト処理の排他用セマフォ取得.
	pc_DataMng->semTake();

	// リクエストメソッド分岐.
	switch( ps_Param->Method ) {
		case WAPD_HTTP_POST:
			si32_Ret = ExecPostResponse(ps_Param);
			break;

		default:
			// [リクエスト不正].
			WAPG_API_trace("[ERR]%s Request Error!.\n", __PRETTY_FUNCTION__ );
			// クライアントへ結果返却.
			printf("405:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //405->ErrorStatus[WAPD_STATUS_METHOD_NOT_ALLOWED] in WAP_defs.h
			m_ResponseHeader->Status = WAPD_STATUS_METHOD_NOT_ALLOWED;
			break;
	}

	// 初期状態に遷移した場合.
	if( WAPE_GetDocumentListStatus_Initialize == pc_DataMng->getStatus() ) {
		// レスポンスクラスの取得.
		WAPC_AsyncResponse* pc_AsyncResponse = dynamic_cast<WAPC_AsyncResponse*>(WAPC_AsyncResponseContainer::newInstance()->getByHandle( m_pv_Handle ));
		if( NULL != pc_AsyncResponse ) {
			// 非同期レスポンスクラスのリストから削除.
			WAPC_AsyncResponseContainer::newInstance()->erase( pc_AsyncResponse->getReqId() );
			delete pc_AsyncResponse;
		}
	}

	// リクエスト処理の排他用セマフォ開放.
	pc_DataMng->semGive();

	return si32_Ret;
}

/**
 * @brief		個別コントローラの共通非同期機能実行.
 * @return		非同期機能実行結果, WAPD_NoError(0)：成功、WAPD_AnyError(1)：失敗.
 * @author		日立 田中.
 * @since		2017/12/20.
 */
int32_t WAPC_GetDocumentList::FunctionExecute()
{
	// メソッド実行後に後処理が必要な場合にはここで実施.

	return WAPD_NoError;
}

/**
 * @brief		非同期：ジョブスペック取得コールバック関数.
 * @param[in]	pJobSpec	ジョブスペック.
 * @param[in]	ptr			コールバックパラメータ.
 * @param[in]	sl_Ret		結果情報.
 * @return		なし.
 * @author		日立 田中.
 * @since		2017/12/20.
 */
void WAPC_GetDocumentList::ans_getJobSpec( TYPC_JobSpec* pJobSpec, void* ptr, int32_t sl_Ret )
{
	m_si32_getJobSpecResult = sl_Ret;
	// 通知待ち解除.
	unlockAnsGetJobSpec();
}

/**
 * @brief		非同期：リスト取得コールバック関数.
 * @param[in]	pc_SecureDocListInfo	結果情報.
 * @param[in]	ptr						コールバックパラメータ.
 * @return		なし.
 * @author		日立 田中.
 * @since		2017/12/20.
 * @date		2018/03/12 日立田中 非同期改善対応.
 */
void WAPC_GetDocumentList::ans_getSecureDocList( APIC_DocListInfo* pc_SecureDocListInfo, void* ptr )
{
	// 管理データアクセス用インスタンス取得.
	WAPC_GetDocumentList_DataMng*	pc_DataMng = WAPC_GetDocumentList_DataMng::newInstance();
	if( NULL == pc_DataMng ) {
		WAPG_API_trace("[ERR]%s WAPC_GetDocumentList_DataMng::newInstance() is NULL!.\n", __PRETTY_FUNCTION__ );
		// Fatal通知.
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
		return;
	}

	// リクエスト処理の排他用セマフォ取得.
	pc_DataMng->semTake();

	// 状態チェック.
	if( WAPE_GetDocumentListStatus_WaitComp != pc_DataMng->getStatus() ) {
		// [状態不一致].
		WAPG_API_trace("[ERR]%s Status Mismatch!.\n", __PRETTY_FUNCTION__ );

		// リクエスト処理の排他用セマフォ開放.
		pc_DataMng->semGive();

		// 通知は無視する.
		return;
	}

	// レスポンスクラスの取得.
	WAPC_AsyncResponse* pc_AsyncResponse = dynamic_cast<WAPC_AsyncResponse*>(WAPC_AsyncResponseContainer::newInstance()->getById( reinterpret_cast<uint64_t>(ans_getSecureDocList) ));
	if( NULL == pc_AsyncResponse ) {
		WAPG_API_trace("[ERR]%s pc_AsyncResponse is NULL!.\n", __PRETTY_FUNCTION__ );
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
		// リクエスト処理の排他用セマフォ開放.
		pc_DataMng->semGive();
		return;
	}
	// メッセージ送信.
	WAPC_AsyncResponder::newInstance()->postMessage( pc_AsyncResponse->getHandle(), WAPD_NoError );
	// 状態遷移[→POST(非同期レスポンス)待ち].
	pc_DataMng->setStatus( WAPE_GetDocumentListStatus_WaitResp );

	// リクエスト処理の排他用セマフォ開放.
	pc_DataMng->semGive();
	return;
}

// HCL-Naveen No.38_BookletPrinting WP 20180214 Added Start.
// Naveen Added.
void WAPC_GetDocumentList::docListCallbBack( APIC_DocListInfo* pc_SecureDocListInfo, void* ptr )
{
	//printf("\n Naveen PKI clbl \n");
	
	// 管理データアクセス用インスタンス取得.
	WAPC_GetDocumentList_DataMng*	pc_DataMng = WAPC_GetDocumentList_DataMng::newInstance();
	if( NULL == pc_DataMng ) {
		WAPG_API_trace("[ERR]%s WAPC_GetDocumentList_DataMng::newInstance() is NULL!.\n", __PRETTY_FUNCTION__ );
		// Fatal通知.
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
		return;
	}

	// リクエスト処理の排他用セマフォ取得.
	pc_DataMng->semTake();
	// 状態チェック.
	if( WAPE_GetDocumentListStatus_WaitComp != pc_DataMng->getStatus() ) {
		// [状態不一致].
		WAPG_API_trace("[ERR]%s Status Mismatch!.\n", __PRETTY_FUNCTION__ );

		// リクエスト処理の排他用セマフォ開放.
		pc_DataMng->semGive();

		// 通知は無視する.
		return;
	}

	// レスポンスクラスの取得.
	WAPC_AsyncResponse* pc_AsyncResponse = dynamic_cast<WAPC_AsyncResponse*>(WAPC_AsyncResponseContainer::newInstance()->getById( reinterpret_cast<uint64_t>(docListCallbBack) ));
	if( NULL == pc_AsyncResponse ) {
		WAPG_API_trace("[ERR]%s pc_AsyncResponse is NULL!.\n", __PRETTY_FUNCTION__ );
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
		// リクエスト処理の排他用セマフォ開放.
		pc_DataMng->semGive();
		return;
	}
	// メッセージ送信.
	WAPC_AsyncResponder::newInstance()->postMessage( pc_AsyncResponse->getHandle(), WAPD_NoError );
	// 状態遷移[→POST(非同期レスポンス)待ち].
	pc_DataMng->setStatus( WAPE_GetDocumentListStatus_WaitResp );

	// リクエスト処理の排他用セマフォ開放.
	pc_DataMng->semGive();
	return;
}
// HCL-Naveen No.38_BookletPrinting WP 20180214 Added End.

// HCL-Naveen No.38_BookletPrinting Mode2-WP 20190422 Added Start.
void WAPC_GetDocumentList::selectedDocCheck()
{
	
	// 管理データアクセス用インスタンス取得.
	WAPC_GetDocumentList_DataMng*	pc_DataMng = WAPC_GetDocumentList_DataMng::newInstance();
	if( NULL == pc_DataMng ) {
		WAPG_API_trace("[ERR]%s WAPC_GetDocumentList_DataMng::newInstance() is NULL!.\n", __PRETTY_FUNCTION__ );
		// Fatal通知.
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
		return;
	}

	// リクエスト処理の排他用セマフォ取得.
	pc_DataMng->semTake();
	// 状態チェック.
	if( WAPE_GetDocumentListStatus_WaitComp != pc_DataMng->getStatus() ) {
		// [状態不一致].
		WAPG_API_trace("[ERR]%s Status Mismatch!.\n", __PRETTY_FUNCTION__ );

		// リクエスト処理の排他用セマフォ開放.
		pc_DataMng->semGive();

		// 通知は無視する.
		return;
	}

	// レスポンスクラスの取得.
	WAPC_AsyncResponse* pc_AsyncResponse = dynamic_cast<WAPC_AsyncResponse*>(WAPC_AsyncResponseContainer::newInstance()->getById( reinterpret_cast<uint64_t>(selectedDocCheck) ));
	if( NULL == pc_AsyncResponse ) {
		WAPG_API_trace("[ERR]%s pc_AsyncResponse is NULL!.\n", __PRETTY_FUNCTION__ );
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
		// リクエスト処理の排他用セマフォ開放.
		pc_DataMng->semGive();
		return;
	}
	// メッセージ送信.
	WAPC_AsyncResponder::newInstance()->postMessage( pc_AsyncResponse->getHandle(), WAPD_NoError );
	// 状態遷移[→POST(非同期レスポンス)待ち].
	pc_DataMng->setStatus( WAPE_GetDocumentListStatus_WaitResp );

	// リクエスト処理の排他用セマフォ開放.
	pc_DataMng->semGive();
	return;
}
// HCL-Naveen No.38_BookletPrinting Mode2-WP 20190422 Added End.

// HCL-Naveen No.38_BookletPrinting Mode2-WP 20190422 Added Start.
void WAPC_GetDocumentList::selectedDocCheck2(int32_t* ret)
{
	
	// 管理データアクセス用インスタンス取得.
	WAPC_GetDocumentList_DataMng*	pc_DataMng = WAPC_GetDocumentList_DataMng::newInstance();
	if( NULL == pc_DataMng ) {
		WAPG_API_trace("[ERR]%s WAPC_GetDocumentList_DataMng::newInstance() is NULL!.\n", __PRETTY_FUNCTION__ );
		// Fatal通知.
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
		return;
	}

	// リクエスト処理の排他用セマフォ取得.
	pc_DataMng->semTake();
	// 状態チェック.
	if( WAPE_GetDocumentListStatus_WaitComp != pc_DataMng->getStatus() ) {
		// [状態不一致].
		WAPG_API_trace("[ERR]%s Status Mismatch!.\n", __PRETTY_FUNCTION__ );

		// リクエスト処理の排他用セマフォ開放.
		pc_DataMng->semGive();

		// 通知は無視する.
		return;
	}

	// レスポンスクラスの取得.
	WAPC_AsyncResponse* pc_AsyncResponse = dynamic_cast<WAPC_AsyncResponse*>(WAPC_AsyncResponseContainer::newInstance()->getById( reinterpret_cast<uint64_t>(selectedDocCheck2) ));
	if( NULL == pc_AsyncResponse ) {
		WAPG_API_trace("[ERR]%s pc_AsyncResponse is NULL!.\n", __PRETTY_FUNCTION__ );
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
		// リクエスト処理の排他用セマフォ開放.
		pc_DataMng->semGive();
		return;
	}
	// メッセージ送信.
	WAPC_AsyncResponder::newInstance()->postMessage( pc_AsyncResponse->getHandle(), WAPD_NoError );
	// 状態遷移[→POST(非同期レスポンス)待ち].
	pc_DataMng->setStatus( WAPE_GetDocumentListStatus_WaitResp );

	// リクエスト処理の排他用セマフォ開放.
	pc_DataMng->semGive();
	return;
}
// HCL-Naveen No.38_BookletPrinting Mode2-WP 20190422 Added End.


// Naveen Added.
void WAPC_GetDocumentList::SecuredocListCallbBack( APIS_SecretDocListInfo* pc_SecureDocListInfo, void* ptr )
{
	
	// 管理データアクセス用インスタンス取得.
	WAPC_GetDocumentList_DataMng*	pc_DataMng = WAPC_GetDocumentList_DataMng::newInstance();
	if( NULL == pc_DataMng ) {
		WAPG_API_trace("[ERR]%s WAPC_GetDocumentList_DataMng::newInstance() is NULL!.\n", __PRETTY_FUNCTION__ );
		// Fatal通知.
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
		return;
	}

	// リクエスト処理の排他用セマフォ取得.
	pc_DataMng->semTake();
	// 状態チェック.
	if( WAPE_GetDocumentListStatus_WaitComp != pc_DataMng->getStatus() ) {
		// [状態不一致].
		WAPG_API_trace("[ERR]%s Status Mismatch!.\n", __PRETTY_FUNCTION__ );

		// リクエスト処理の排他用セマフォ開放.
		pc_DataMng->semGive();

		// 通知は無視する.
		return;
	}

	// レスポンスクラスの取得.
	WAPC_AsyncResponse* pc_AsyncResponse = dynamic_cast<WAPC_AsyncResponse*>(WAPC_AsyncResponseContainer::newInstance()->getById( reinterpret_cast<uint64_t>(SecuredocListCallbBack) ));
	if( NULL == pc_AsyncResponse ) {
		WAPG_API_trace("[ERR]%s pc_AsyncResponse is NULL!.\n", __PRETTY_FUNCTION__ );
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
		// リクエスト処理の排他用セマフォ開放.
		pc_DataMng->semGive();
		return;
	}
	// メッセージ送信.
	WAPC_AsyncResponder::newInstance()->postMessage( pc_AsyncResponse->getHandle(), WAPD_NoError );
	// 状態遷移[→POST(非同期レスポンス)待ち].
	pc_DataMng->setStatus( WAPE_GetDocumentListStatus_WaitResp );

	// リクエスト処理の排他用セマフォ開放.
	pc_DataMng->semGive();
	return;
}
// HCL-Naveen No.38_BookletPrinting WP 20180214 Added End.

/**
 * @brief		POST(非同期リクエスト)処理.
 * @param[in]	ps_Param HTTPリクエストヘッダ、ボディ.
 * @return		処理結果, WAPD_NoError(0)：成功、WAPD_AnyError(1)：失敗.
 * @author		日立 田中.
 * @since		2017/12/20.
 * @date		2018/01/23 日立村上 2LAN対応_ボックスID指定の変更.
 * @date		2018/03/12 日立田中 非同期改善対応.
 * @date		2023/03/01 日立佐藤菜 OP_BTS-40914 req_SecretDocChkの戻り値を確認するように修正.
 */
int32_t WAPC_GetDocumentList::ExecPostRequest( const WAPS_SubController* ps_Param )
{
	int32_t	si32_BoxId = 0;		// ボックスID.
	int32_t	si32_ListOrder = 0;	// リスト開始位置.
	int32_t	si32_ListCnt = 0;	// リスト個数.
	int32_t	si32_ListSort = 0;	// リストのソートモード.
	int32_t	si32_CheckBox = 0;	// 認証＆プリントボックスアクセス条件チェック結果.
	Bool	b_Abbr = False;		// 省略有無.
	Bool	b_Ret = False;		// 要求結果.

// HCL-Naveen No.38_BookletPrinting WP 20180214 Added Start.
// Naveen Added.
	char	secret_DocId[APID_SecretDocIdMax] = {0,};
	char	secret_Passwd[APID_SecretDocPasswordMax] = {0,};
	char	MemRx_Passwd[APID_SecretDocPasswordMax] = {0,};
	
	Bool b_secureDoc_flag = False;
	Bool b_securePass_flag = False;
// Naveen Added.
// HCL-Naveen No.38_BookletPrinting WP 20180214 Added End.	

// HCL-Naveen No.38_BookletPrinting Mode2-WP 20190422 Added Start.
	int32_t		si32_AuthMode = 0;		// 1 -> Mode 1 2-> Mode 2.
	Bool 		b_PasswordCheck = False;
	int32_t 	si32_SelectedDocid[100] = {0,};
	int32_t 	si32_SelectedDocidMax = 0;
// HCL-Naveen No.38_BookletPrinting Mode2-WP 20190422 Added Start.
	
	// 同期処理に初期化しておく.
	m_ResponseHeader->Async = WAPD_ASYNC_DISABLE;

	// パラメータNULLチェック.
	if( NULL == ps_Param ) {
		WAPG_API_trace("[ERR]%s NULL Parameter!.\n", __PRETTY_FUNCTION__ );
		// Fatal通知.
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
		// クライアントへ結果返却.
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		return WAPD_AnyError;
	}

	// 管理データアクセス用インスタンス取得.
	WAPC_GetDocumentList_DataMng*	pc_DataMng = WAPC_GetDocumentList_DataMng::newInstance();
	if( NULL == pc_DataMng ) {
		WAPG_API_trace("[ERR]%s WAPC_GetDocumentList_DataMng::newInstance() is NULL!.\n", __PRETTY_FUNCTION__ );
		// Fatal通知.
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
		// クライアントへ結果返却.
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		return WAPD_AnyError;
	}

	pc_DataMng->setMaxMatchedDocid(0);	// Initialize the MaxMatchedDocid.		// Naveen Added. // HCL-Naveen No.38_BookletPrinting Mode2-WP 20190422 Added.
	
	pc_DataMng->setSecureDocStatus(WAPE_GetSecureDocListReqStatus_NoStatus);	// Initialize the SecureDocStatus.	// Naveen Added.
	
	// 状態チェック.
	WAPE_GetDocumentListStatus e_Step = pc_DataMng->getStatus();
	if( WAPE_GetDocumentListStatus_Initialize != e_Step ) {
		// [状態不一致].
		WAPG_API_trace("[ERR]%s Status Mismatch!.\n", __PRETTY_FUNCTION__ );
		// クライアントへ結果返却.
		printf("503:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //503->ErrorStatus[WAPD_STATUS_SERVICE_UNAVAILABLE] in WAP_defs.h
		m_ResponseHeader->Status = WAPD_STATUS_SERVICE_UNAVAILABLE;
		return WAPD_AnyError;
	}

	// コンテンツタイプチェック.
	if ( WAPD_CONTENT_TYPE_JSON != ps_Param->ContentType ) {
		// [リクエスト不正].
		WAPG_API_trace("[ERR]%s ContentType Error!.\n", __PRETTY_FUNCTION__ );
		// クライアントへ結果返却.
		printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
		m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
		return WAPD_AnyError;
	}

	// -------------------------------------------------------------------.
	// リクエストボディのパラメータチェック.
	// -------------------------------------------------------------------.
	// 共通ライブラリインスタンス取得.
	WAPC_BoxCommonLib*	cp_WAPC_BoxCommonLib = WAPC_BoxCommonLib::newInstance();
	if( NULL == cp_WAPC_BoxCommonLib ) {
		WAPG_API_trace("[ERR]%s WAPC_BoxCommonLib::newInstance() is NULL!.\n", __PRETTY_FUNCTION__ );
		// Fatal通知.
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
		// クライアントへ結果返却.
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		return WAPD_AnyError;
	}

	// パラメータ(box_type)からボックスID取得.
	b_Ret = cp_WAPC_BoxCommonLib->getRequestParam_BoxID( ps_Param->Param, ps_Param->UserNumber, &si32_BoxId , ps_Param->TrackNumber);	// 2022/08/19 FPT-TuanBV1 IT6_2.3 OP_BTS-39017 Modified.
	// 2020/09/02 nomura Start.
	// レスポンスパラメータ設定時に、Box種別が必要なので、メンバー変数に登録.
	m_si32_BoxId = si32_BoxId;
	// 2020/09/02 nomura Start.
	if( False == b_Ret ) {
		// 取得失敗の場合.
		printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
		m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
		return WAPD_AnyError;
	}

	// パラメータ(list_offset)取得.
	b_Ret = cp_WAPC_BoxCommonLib->getRequestParam_ListOffset( ps_Param->Param, &si32_ListOrder, &b_Abbr );
	if( False == b_Ret ) {
		// パラメータ不正の場合.
		printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
		m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
		return WAPD_AnyError;
	}
	if( True == b_Abbr ) {
		// "list_offset"省略.
		si32_ListOrder = TYPD_getRequestParam_ListOffset_Min;
	}

	// パラメータ(list_cnt)取得.
	b_Ret = cp_WAPC_BoxCommonLib->getRequestParam_ListCnt( ps_Param->Param, &si32_ListCnt, &b_Abbr );
	if( False == b_Ret ) {
		// パラメータ不正の場合.
		printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
		m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
		return WAPD_AnyError;
	}
	if( True == b_Abbr ) {
		// "list_cnt"省略.
		si32_ListCnt = TYPD_getRequestParam_ListCnt_Max;
	}

	// パラメータ(list_sort)取得.
	b_Ret = cp_WAPC_BoxCommonLib->getRequestParam_ListSort( ps_Param->Param, &si32_ListSort, &b_Abbr );
	if( False == b_Ret ) {
		// パラメータ不正の場合.
		printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
		m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
		return WAPD_AnyError;
	}
	if( True == b_Abbr ) {
		// "list_sort"省略.
		si32_ListSort = DOCD_DocFile_SortDown;	// 降順.
	}
	// HCL-Naveen No.38_BookletPrinting Mode2-WP 20190422 Added Start.
	if( si32_BoxId == DOCD_SecretDocumentsBox)	// If in case of Security documents
	{
		b_Ret = cp_WAPC_BoxCommonLib->getRequestParam_AuthMode( ps_Param->Param, &si32_AuthMode );
		
		b_Ret = cp_WAPC_BoxCommonLib->getRequestParam_PassCheck( ps_Param->Param, &b_PasswordCheck );
		
		
		if(si32_AuthMode == 2
			&& b_PasswordCheck == True)
		{
				b_Ret = cp_WAPC_BoxCommonLib->getRequestParam_SelectedDocs( ps_Param->Param, si32_SelectedDocid, &si32_SelectedDocidMax, si32_AuthMode, b_PasswordCheck );
				// Read DOC ID's from Array
		}
		
		if( False == b_Ret ) {
			// パラメータ不正の場合.
			printf("\n getRequestParam for AuthMode or PassCheck or SelectedDocs is error \n");
			printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
			m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
			return WAPD_AnyError;
		}
	}	
	// HCL-Naveen No.38_BookletPrinting Mode2-WP 20190422 Added End.
	
	// HCL-Naveen No.38_BookletPrinting WP 20180214 Added Start.
	// Naveen 
	if( si32_BoxId == DOCD_SecretDocumentsBox
		&& si32_AuthMode != 3 )	// Naveen Added. // If in case of Security documents		// NO need to check in case of Easy Print.
	{
		// secure_document_id
		b_Ret = cp_WAPC_BoxCommonLib->getRequestParam_SecureDocID( ps_Param->Param, secret_DocId, &b_secureDoc_flag );
		if( False == b_Ret ) {
			// パラメータ不正の場合.
			printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
			m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
			return WAPD_AnyError;
		}
	}
	
	// document_password
	b_Ret = cp_WAPC_BoxCommonLib->getRequestParam_DocPass( ps_Param->Param, secret_Passwd, &b_securePass_flag, si32_BoxId );
	if( False == b_Ret ) {
		// パラメータ不正の場合.
		printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
		m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
		return WAPD_AnyError;
	}
	// Naveen 
	// HCL-Naveen No.38_BookletPrinting WP 20180214 Added End.

	// -------------------------------------------------------------------.
	// 認証＆プリントボックスアクセス条件チェック.
	// -------------------------------------------------------------------.
	m_si32_UserId = ps_Param->UserNumber;										// ユーザーID保持.
	m_si32_TrackId = ps_Param->TrackNumber;		// 2022/08/19 FPT-TuanBV1 IT6_2.3 OP_BTS-39017 Modified.
	WAPS_CheckBoxDocAccess s_CheckBoxDocAccess;
	s_CheckBoxDocAccess.m_si32_UserID = m_si32_UserId;
	s_CheckBoxDocAccess.m_si32_TrackNumber = m_si32_TrackId;		// 2022/08/19 FPT-TuanBV1 IT6_2.3 OP_BTS-39017 Modified.
	s_CheckBoxDocAccess.m_si32_BoxId = si32_BoxId;
	si32_CheckBox = cp_WAPC_BoxCommonLib->checkBoxService( s_CheckBoxDocAccess, m_AccessToken );
	if( WAPD_STATUS_OK != si32_CheckBox ) {
		// ボックスアクセス不可.
		printf("503:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //503->ErrorStatus[WAPD_STATUS_SERVICE_UNAVAILABLE] in WAP_defs.h
		m_ResponseHeader->Status = WAPD_STATUS_SERVICE_UNAVAILABLE;
		return WAPD_AnyError;
	}

	// -------------------------------------------------------------------.
	// 文書リスト一覧取得リクエスト.
	// -------------------------------------------------------------------.
	APIC_User *pc_UserInstance = APIC_User::newInstance( m_si32_UserId );		// ユーザー情報.
	// 2022/08/19 FPT-TuanBV1 IT6_2.3 OP_BTS-39017 Modified Start.
	APIC_Track* pc_Track = NULL;
	TYPE_NetworkSpecific e_Network;
	
	// 部門情報取得.
	pc_Track = APIC_Track::newInstance( m_si32_TrackId );
	
	if ( NULL != pc_UserInstance ) {
		 e_Network = pc_UserInstance->isNetwork();				// ログインユーザーのネットワーク種別を取得.
	}
	else if ( NULL != pc_Track ) {
		e_Network = TYPE_NetworkSpecific_NW1;
	}
	else {
		// [ユーザー不正].
		WAPG_API_trace("[ERR]%s UserNumber Error!.\n", __PRETTY_FUNCTION__ );
		// クライアントへ結果返却.
		printf("503:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //503->ErrorStatus[WAPD_STATUS_SERVICE_UNAVAILABLE] in WAP_defs.h
		m_ResponseHeader->Status = WAPD_STATUS_SERVICE_UNAVAILABLE;
		return WAPD_AnyError;
	}
	// 2022/08/19 FPT-TuanBV1 IT6_2.3 OP_BTS-39017 Modified End.

	// Naveen Start.

	if( si32_AuthMode == 3 )		// Will Enter in case of auth_mode = EASY_PRINT.
	{
		memset( &m_s_SecretDocListInfo, 0, sizeof(m_s_SecretDocListInfo) );
		// リクエストパラメータ設定.
		m_s_SecretDocListInfo.sl_ListOrder		= si32_ListOrder;								// リスト開始位置.
		m_s_SecretDocListInfo.sl_ListCnt		= si32_ListCnt;									// リスト個数.
		m_s_SecretDocListInfo.sl_ListSort		= si32_ListSort;								// リストのソートモー
		
		m_s_SecretDocListInfo.pc_List		= &m_c_SecretDocListTblList;					// 取得リストを格納する領域.
		m_s_SecretDocListInfo.pc_List->delAll();    										// リスト開放.
		
		
		
		WAPC_AsyncResponse* pc_AsyncResponse = new WAPC_AsyncResponse;
		if( NULL == pc_AsyncResponse ) {
			WAPG_API_trace("[ERR]%s new WAPC_AsyncResponse() is NULL!.\n", __PRETTY_FUNCTION__ );
			LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
			// クライアントへ結果返却.
			m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
			printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
			return WAPD_AnyError;
		}
		pc_AsyncResponse->setHandle( m_pv_Handle );

		pc_AsyncResponse->setReqId( reinterpret_cast<uint64_t>(SecuredocListCallbBack) );

		WAPC_AsyncResponseContainer::newInstance()->push( pc_AsyncResponse );
		
		pc_DataMng->setSecureDocStatus( WAPE_GetSecureDocListReqStatus_SecDocEasyPrint );		// Naveen Added.
		
		
		int b_sam = 0;
		
		b_sam = m_pc_Box->req_getSecretDocList_All( &m_s_SecretDocListInfo,
													&WAPC_GetDocumentList::SecuredocListCallbBack, 
													e_Network );
		
		
		if( 0 != b_sam) {
			WAPG_API_trace("[ERR]%s req_getSecureListPanel Error!.\n", __PRETTY_FUNCTION__ );
			// クライアントへ結果返却.
			m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
			printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
			WAPC_AsyncResponseContainer::newInstance()->erase( pc_AsyncResponse->getReqId() );
			delete pc_AsyncResponse;
			return WAPD_AnyError;
		}
		
		
		// 状態遷移[→Callback待ち].
		pc_DataMng->setStatus( WAPE_GetDocumentListStatus_WaitComp );

		// 非同期設定(非同期処理を行う場合はこれが必要).
		m_ResponseHeader->Async = WAPD_ASYNC_ENABLE;

		m_ResponseHeader->Status = WAPD_STATUS_OK;
		
		
		return WAPD_NoError;
	
	}
	
	// Naveen End.
	
	// リクエストパラメータ初期化.
	memset( &m_s_DocListInfo, 0, sizeof(m_s_DocListInfo) );
	// リクエストパラメータ設定.
	m_s_DocListInfo.listOrder	= si32_ListOrder;								// リスト開始位置.
	m_s_DocListInfo.listCnt		= si32_ListCnt;									// リスト個数.
	m_s_DocListInfo.listSort	= si32_ListSort;								// リストのソートモード.

	// ネットワーク種別のチェック.
	if( TYPE_NetworkSpecific_NW1 == e_Network ) {
		// ネットワーク1の場合.
		m_s_DocListInfo.listMode	= APID_DocListMode_GetTouchAndGoDocAuto;		// 自動(生成時間順).
	} else {
		// ネットワーク2の場合.
		m_s_DocListInfo.listMode	= APID_DocListMode_GetTouchAndGoDocAutoNW2;		// 自動(生成時間順).
	}
	
	// HCL-Naveen No.38_BookletPrinting WP 20180214 Added Start.
	
	if (b_secureDoc_flag) // <= this falg will be set if there is Secure doc id in JSON.
	{ 
		::strncpy(m_s_DocListInfo.secret_DocId,  secret_DocId , sizeof(m_s_DocListInfo.secret_DocId) - 1); //2021/11/15 FPT-DiepTV2 strcpy, strcat, sprintf使用状況調査協力のお願い Modify.
	//	::strcpy(m_s_DocListInfo.secret_DocId,  "sam");
		m_s_DocListInfo.listMode = 0x05;
	}
	
	if (b_securePass_flag) // <= this falg will be set for Password in document during Secure Print.
	{ 
		::strncpy(m_s_DocListInfo.secret_Passwd, secret_Passwd , sizeof(m_s_DocListInfo.secret_Passwd) - 1); //2021/11/15 FPT-DiepTV2 strcpy, strcat, sprintf使用状況調査協力のお願い Modify.
	//	::strcpy(m_s_DocListInfo.secret_Passwd, "2132");
		m_s_DocListInfo.listMode = 0x05;
	}
	
	if( si32_BoxId == DOCD_CompulsionRcvBox )	// In Case of Memory RX
	{
// 2019/04/25 【IT6_EagleH】【Klock work ID:486820 [SV.STRBO.BOUND_COPY.UNTERM]】 HCLT-Vijayaraghavan Modification Start.
//		::strncpy(MemRx_Passwd, secret_Passwd, sizeof(MemRx_Passwd));
		::strncpy(MemRx_Passwd, secret_Passwd, sizeof(MemRx_Passwd)-1);
// 2019/04/25 【IT6_EagleH】【Klock work ID:486820 [SV.STRBO.BOUND_COPY.UNTERM]】 HCLT-Vijayaraghavan Modification End.
	//	::strncpy(MemRx_Passwd,"1",sizeof(MemRx_Passwd));
	
		m_s_DocListInfo.listMode = 0x01;
	}
	
	// HCL-Naveen No.38_BookletPrinting WP 20180214 Added End.
	
	m_s_DocListInfo.pList		= &m_c_DocListTblList;							// 取得リストを格納する領域.
	m_s_DocListInfo.pList->delAll();    										// リスト開放.
		m_pc_Box->setBoxId( si32_BoxId );
	//printf("\n Naveen Entered PKI check 6 \n");
	// HCL-Naveen No.38_BookletPrinting WP 20180214 Added Start.
	if( si32_BoxId == DOCD_SecretDocumentsBox 
		|| si32_BoxId == DOCD_SecretDocumentsBoxNW2 
		|| si32_BoxId == DOCD_CompulsionRcvBox 
		|| si32_BoxId == DOCD_EncFile_BoxId )		// PKI related IT6_2.1 制御構成変更_No.67 WP IT6_21-204 20200819 HCLNaveen.
	{
		//printf("\n Naveen Entered PKI check 7 \n");
		// レスポンスクラスの取得.
		
		WAPC_AsyncResponse* pc_AsyncResponse = new WAPC_AsyncResponse;
		if( NULL == pc_AsyncResponse ) {
			WAPG_API_trace("[ERR]%s new WAPC_AsyncResponse() is NULL!.\n", __PRETTY_FUNCTION__ );
			LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
			// クライアントへ結果返却.
			m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
			printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
			return WAPD_AnyError;
		}
		pc_AsyncResponse->setHandle( m_pv_Handle );
		// HCL-Naveen No.38_BookletPrinting Mode2-WP 20190422 Modified Start.
		if( si32_AuthMode == 2
			&& b_PasswordCheck == True)
		{
		//	pc_AsyncResponse->setReqId( reinterpret_cast<uint64_t>(selectedDocCheck) );
			pc_AsyncResponse->setReqId( reinterpret_cast<uint64_t>(selectedDocCheck2) );	// Naveen Added.
		}else{
			pc_AsyncResponse->setReqId( reinterpret_cast<uint64_t>(docListCallbBack) );
		}
		// HCL-Naveen No.38_BookletPrinting Mode2-WP 20190422 Modified End.
		WAPC_AsyncResponseContainer::newInstance()->push( pc_AsyncResponse );
		
		APIC_Box*					m_pc_Box;
		
		m_pc_Box = APIC_Box::newInstance();
		
		// アクセス権情報設定.
	//	m_pc_Box->setLogIFNo( e_InspectLogIF );

		// 操作するBOXを指定.
		m_pc_Box->setBoxId( si32_BoxId );
		
		int b_sam = 0;
		Bool b_sRet = True;		// OP_BTS-8609 SSB-Fix 27062019 Naveen Added.
		
		// HCL-Naveen No.38_BookletPrinting Mode2-WP 20190422 Modified Start.
		if( si32_AuthMode != 2 )	// si32_AuthMode == 2 when Authmode is Mode 2.
		{
			// OP_BTS-8609 SSB-Fix 27062019 Naveen Modified Start.
			// OP_BTS-8609 SSB Modified.
			if( si32_BoxId == DOCD_SecretDocumentsBox
				|| si32_BoxId == DOCD_SecretDocumentsBoxNW2 )
			{
			
				// リクエスト発行.
				b_sRet = APIC_SecureList::newInstance()->req_getSecureListPanel(
					pc_UserInstance,									// ユーザー情報(ログイン中のユーザー情報).
					NULL,												// ボックスパスワード(認証＆プリントBOXの場合は対象外).
					&m_s_DocListInfo,									// セキュアプリントのリスト(受信中ジョブリストと蓄積リスト).
					&WAPC_GetDocumentList::docListCallbBack,		// コールバック関数.
					False,												// セキュリティ文書Boxの[認証操作禁止機能=「モード２」の場合を判定するフラグ.
					si32_BoxId,											// BoxID.
					m_pc_Box,											// ボックス情報.
					False,												// セキュリティ文書Boxのモード２のパスワード有無.
					pc_Track											// 部門情報(ログイン中の部門情報).		// 2022/08/19 FPT-TuanBV1 IT6_2.3 OP_BTS-39017 Modified.
				);
			// PKI related IT6_2.1 制御構成変更_No.67 WP IT6_21-204 20200819 HCLNaveen.	
			}else if( si32_BoxId == DOCD_EncFile_BoxId ) {
				//printf("\n Naveen Entered PKI final Call \n");
				
				m_s_DocListInfo.userId = m_si32_UserId;
				
				//printf("\n m_si32_UserId = %d",m_si32_UserId);
				//printf("\n pc_UserInstance->getUserNo() = %d",pc_UserInstance->getUserNo());
				
				b_sam = m_pc_Box->req_getEncFileList( &m_s_DocListInfo, 
									 &WAPC_GetDocumentList::docListCallbBack );		// PKI Naveen.
			}else{
			// PKI related IT6_2.1 制御構成変更_No.67 WP IT6_21-204 20200819 HCLNaveen.
				//printf("\n Naveen Entered PKI check 10 \n");
				b_sam = m_pc_Box->req_getDocList( pc_UserInstance, 
									 MemRx_Passwd, 	// Will have a valid value only in case of MemoryRX.
									 &m_s_DocListInfo, 
									 &WAPC_GetDocumentList::docListCallbBack, 
									 pc_Track );		// 2022/08/19 FPT-TuanBV1 IT6_2.3 OP_BTS-39017 Modified.
			}
			// OP_BTS-8609 SSB Modified.
			// OP_BTS-8609 SSB-Fix 27062019 Naveen Modified End.		
		
		} else {
			
			// Incase of Mode2
			if( b_PasswordCheck == False)
			{
			//	::strcpy(m_s_DocListInfo.secret_DocId,  "sam");		// Giving Only SecID
					
				m_s_DocListInfo.listMode = 0x09;
				
				// OP_BTS-8609 SSB-Fix 27062019 Naveen Modified Start.
				// OP_BTS-8609 SSB Modified.
				if( si32_BoxId == DOCD_SecretDocumentsBox
					|| si32_BoxId == DOCD_SecretDocumentsBoxNW2 )
				{
					pc_DataMng->setSecureDocStatus( WAPE_GetSecureDocListReqStatus_Mode2SecDocList );
					
					b_sRet = APIC_SecureList::newInstance()->req_getSecureListPanel(
						pc_UserInstance,									// ユーザー情報(ログイン中のユーザー情報).
						NULL,												// ボックスパスワード(認証＆プリントBOXの場合は対象外).
						&m_s_DocListInfo,									// セキュアプリントのリスト(受信中ジョブリストと蓄積リスト).
						&WAPC_GetDocumentList::docListCallbBack,		// コールバック関数.
						True,												// セキュリティ文書Boxの[認証操作禁止機能=「モード２」の場合を判定するフラグ.
						si32_BoxId,											// BoxID.
						m_pc_Box,											// ボックス情報.
						False,												// セキュリティ文書Boxのモード２のパスワード有無.
						pc_Track										// 部門情報(ログイン中の部門情報).		// 2022/08/19 FPT-TuanBV1 IT6_2.3 OP_BTS-39017 Modified.
					);
				
				}else{
					b_sam = m_pc_Box->req_getSecretDocList( &m_s_DocListInfo,
											&WAPC_GetDocumentList::docListCallbBack, 
											e_Network );
				}
				// OP_BTS-8609 SSB Modified.
				// OP_BTS-8609 SSB-Fix 27062019 Naveen Modified End.
				
			}else{
				// This is Flow 2 -> to check the password for DocID's
				APIC_Doc*			pc_Doc = NULL;
				pc_Doc = APIC_Doc::newInstance();
				
				char sc_SecureDocPass[APID_SecretDocPasswordMax];
				memset( sc_SecureDocPass, '\0', sizeof(sc_SecureDocPass) );
				
				int j=0;
				// Naveen Modified.
				int k=0;
				APIC_SelectSecretDocData docData;
				memset( &docData, 0, sizeof(docData) );
				
				for(int i=0;i<si32_SelectedDocidMax;i++)
				{
					memset( sc_SecureDocPass, '\0', sizeof(sc_SecureDocPass) );
					getSecureDocJobPass( static_cast<int32_t>(si32_BoxId), static_cast<int32_t>(si32_SelectedDocid[i]),pc_Doc, sc_SecureDocPass );
					
					
					if( 0 == strcmp( sc_SecureDocPass, secret_Passwd ) )
					{
						pc_DataMng->setMatchedDocidList(j,si32_SelectedDocid[i]);
						j++;
						pc_DataMng->setMaxMatchedDocid(j);
					}
					else{
						docData.secDocID[k] = si32_SelectedDocid[i];
						k++;
					}
				}
				
				pc_Doc->deleteInstance();	// Naveen Added.	
				
				if( k == 0
					&& 0 != pc_DataMng->getMaxMatchedDocid() )
				{
					// If Password are correct fpr all docs.
					pc_AsyncResponse->setReqId( reinterpret_cast<uint64_t>(selectedDocCheck) );
					pc_DataMng->setSecureDocStatus( WAPE_GetSecureDocListReqStatus_AllPassCorrect );
					
				}
				
				docData.secDocCnt = k;
				
				if( docData.secDocCnt != 0 )
				{
					
					m_pc_Box->setBoxId( DOCD_SecretDocumentsBox );	//FailSafe
					
					pc_DataMng->setSecureDocStatus( WAPE_GetSecureDocListReqStatus_PassPenalityCheck );
					
					char	secret_DocId_loc[APID_SecretDocIdMax] = {0,};
				//	::strncpy(secret_DocId_loc,"nav1",sizeof(secret_DocId_loc));
					::strncpy(secret_DocId_loc,secret_DocId,sizeof(secret_DocId_loc)-1); //【EagleBK】静的解析対応依頼 HCL-Chitra 13/09/2019 Modified.
					
					b_sam = m_pc_Box->req_SecretDocChk( secret_DocId_loc, 
													secret_Passwd,
													&docData,
													&WAPC_GetDocumentList::selectedDocCheck2, 
													pc_UserInstance, 
													APIC_Track::newInstance( m_si32_TrackId ), 		// 2022/08/19 FPT-TuanBV1 IT6_2.3 OP_BTS-39017 Modified.
													TYPD_NotSetting, 
													e_Network );
				}
				// Naveen Modified.
			}
		}
		
// HCL-Naveen No.38_BookletPrinting Mode2-WP 20190422 Added End.
		//printf("\n b_sam =%d",b_sam);
		//printf("\n b_sRet =%d",b_sRet);
		
		if( 0 != b_sam || b_sRet == False) {	// OP_BTS-8609 SSB-Fix 27062019 Naveen Added.
			WAPG_API_trace("[ERR]%s req_getSecureListPanel Error!.\n", __PRETTY_FUNCTION__ );
			// クライアントへ結果返却.
			m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
			printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
			WAPC_AsyncResponseContainer::newInstance()->erase( pc_AsyncResponse->getReqId() );
			delete pc_AsyncResponse;
			return WAPD_AnyError;
		}
	
	}else {
	// HCL-Naveen No.38_BookletPrinting WP 20180214 Added End.
	// レスポンスクラスの取得.
	WAPC_AsyncResponse* pc_AsyncResponse = new WAPC_AsyncResponse;
	if( NULL == pc_AsyncResponse ) {
		WAPG_API_trace("[ERR]%s new WAPC_AsyncResponse() is NULL!.\n", __PRETTY_FUNCTION__ );
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
		// クライアントへ結果返却.
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		return WAPD_AnyError;
	}
	pc_AsyncResponse->setHandle( m_pv_Handle );
	pc_AsyncResponse->setReqId( reinterpret_cast<uint64_t>(ans_getSecureDocList) );
	WAPC_AsyncResponseContainer::newInstance()->push( pc_AsyncResponse );

	// リクエスト発行.
	b_Ret = APIC_SecureList::newInstance()->req_getSecureListPanel(
		pc_UserInstance,									// ユーザー情報(ログイン中のユーザー情報).
		NULL,												// ボックスパスワード(認証＆プリントBOXの場合は対象外).
		&m_s_DocListInfo,									// セキュアプリントのリスト(受信中ジョブリストと蓄積リスト).
		&WAPC_GetDocumentList::ans_getSecureDocList,		// コールバック関数.
		False,												// セキュリティ文書Boxの[認証操作禁止機能=「モード２」の場合を判定するフラグ.
		si32_BoxId,											// BoxID.
		m_pc_Box,											// ボックス情報.
		False,												// セキュリティ文書Boxのモード２のパスワード有無.
		pc_Track										// 部門情報(ログイン中の部門情報).		// 2022/08/19 FPT-TuanBV1 IT6_2.3 OP_BTS-39017 Modified.
	);
	if( False == b_Ret ) {
		WAPG_API_trace("[ERR]%s req_getSecureListPanel Error!.\n", __PRETTY_FUNCTION__ );
		// クライアントへ結果返却.
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		WAPC_AsyncResponseContainer::newInstance()->erase( pc_AsyncResponse->getReqId() );
		delete pc_AsyncResponse;
		return WAPD_AnyError;
	}
	
	}  // Naveen Added	// HCL-Naveen No.38_BookletPrinting WP 20180214 Added.

	// 状態遷移[→Callback待ち].
	pc_DataMng->setStatus( WAPE_GetDocumentListStatus_WaitComp );

	// 非同期設定(非同期処理を行う場合はこれが必要).
	m_ResponseHeader->Async = WAPD_ASYNC_ENABLE;

	m_ResponseHeader->Status = WAPD_STATUS_OK;
	
	// HCL-Naveen No.38_BookletPrinting Mode2-WP 20190422 Added Start.
	if( WAPE_GetSecureDocListReqStatus_AllPassCorrect == pc_DataMng->getSecureDocStatus() )		// Naveen Modified.
	{
		WAPC_GetDocumentList::selectedDocCheck();
	}
	// HCL-Naveen No.38_BookletPrinting Mode2-WP 20190422 Added End.
	
	return WAPD_NoError;
}

/**
 * @brief		POST(非同期レスポンス)処理.
 * @param[in]	ps_Param HTTPリクエストヘッダ、ボディ.
 * @return		処理結果, WAPD_NoError(0)：成功、WAPD_AnyError(1)：失敗.
 * @author		日立 田中.
 * @since		2017/12/20.
 * @date		2018/03/12 日立田中 非同期改善対応.
 */
int32_t WAPC_GetDocumentList::ExecPostResponse( const WAPS_SubController* ps_Param )
{
	APIC_Doc*			pc_Doc = NULL;									// APIC_Doc インスタンスポインタ.
	APIC_DocListTbl*	pc_PtrScureListTbl = NULL;						// ドキュメントリストポインタ格納.
	short				ss_cntDocList = 0;								// 文書リスト数.
//	char				sc_docJobNameDisp[DOCD_EncPdfUserNameMaxLen+1];	// ユーザー名(登録元)取得領域.		// HCL-Naveen OP_BTS-28324 20201102 Commented.
	char				sc_docJobNameDisp[64+1];	// ユーザー名(登録元)取得領域.		// HCL-Naveen OP_BTS-28324 20201102 Added.
	uint32_t			ui32_JobId = 0;									// ジョブNo.
	uint32_t			ui32_JobResourceId = 0;							// ジョブリソース番号.
	uint32_t			ui32_OriginalColorType = -1;					// 原稿カラー種別.

	// パラメータNULLチェック.
	if( NULL == ps_Param ) {
		WAPG_API_trace("[ERR]%s NULL Parameter!.\n", __PRETTY_FUNCTION__ );
		// Fatal通知.
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
		// クライアントへ結果返却.
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		return WAPD_AnyError;
	}

	// 管理データアクセス用インスタンス取得.
	WAPC_GetDocumentList_DataMng*	pc_DataMng = WAPC_GetDocumentList_DataMng::newInstance();
	if( NULL == pc_DataMng ) {
		WAPG_API_trace("[ERR]%s WAPC_GetDocumentList_DataMng::newInstance() is NULL!.\n", __PRETTY_FUNCTION__ );
		// Fatal通知.
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
		// クライアントへ結果返却.
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		return WAPD_AnyError;
	}

	// 状態チェック.
	if( WAPE_GetDocumentListStatus_WaitResp != pc_DataMng->getStatus() ) {
		// [状態不一致].
		WAPG_API_trace("[ERR]%s Status Mismatch!.\n", __PRETTY_FUNCTION__ );
		// クライアントへ結果返却.
		printf("405:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //405->ErrorStatus[WAPD_STATUS_METHOD_NOT_ALLOWED] in WAP_defs.h
		m_ResponseHeader->Status = WAPD_STATUS_METHOD_NOT_ALLOWED;
		return WAPD_AnyError;
	}
	// Naveen Modified.
	if(WAPE_GetSecureDocListReqStatus_SecDocEasyPrint != pc_DataMng->getSecureDocStatus() ) {
		if( 0 == m_s_DocListInfo.pList->getCount() 
			&& 0 == pc_DataMng->getMaxMatchedDocid() ) {		//if there are selected docs. // HCL-Naveen No.38_BookletPrinting Mode2-WP 20190422 Modified.
			// Naveen Comment. should not execute when trying to get the all docids of Secure Print.
			// 文書リストが存在しない場合.
			// printf("204:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //204->ErrorStatus[WAPD_STATUS_NO_CONTENT] in WAP_defs.h	// 2023/07/20 FPT-KhanhDN7 [OP_BTS-44393] Delete.
			m_ResponseHeader->Status = WAPD_STATUS_NO_CONTENT;
			if(WAPE_GetSecureDocListReqStatus_PassPenalityCheck == pc_DataMng->getSecureDocStatus() )
			{
				m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
				printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
			}
			// 状態遷移[→初期状態].
			pc_DataMng->setStatus( WAPE_GetDocumentListStatus_Initialize );
			return WAPD_NoError;
		}
	}
	// Naveen Modified.
	
	pc_Doc = APIC_Doc::newInstance();
	if( NULL == pc_Doc ) {
		WAPG_API_trace("[ERR]%s APIC_Doc::newInstance() is NULL!.\n", __PRETTY_FUNCTION__ );
		// Fatal通知.
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
		// クライアントへ結果返却.
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		return WAPD_AnyError;
	}

	m_Response = new Json::Value;
	if( NULL == m_Response ) {
		WAPG_API_trace("[ERR]%s new Json::Value() is NULL!.\n", __PRETTY_FUNCTION__ );
		// Fatal通知.
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
		// インスタンス破棄.
		pc_Doc->deleteInstance();
		// クライアントへ結果返却.
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		return WAPD_AnyError;
	}
	
	// Naveen Added.
	if(WAPE_GetSecureDocListReqStatus_SecDocEasyPrint == pc_DataMng->getSecureDocStatus())
	{
		APIS_SecretDocTbl*	pc_PtrScureDocListTbl = NULL;
		Json::Value& cbResult = ( *m_Response )["document_id_list"];
		int i=0;
		while( 1 ) {
			
			// 1番目のドキュメントリスト取得.
			pc_PtrScureDocListTbl = m_s_SecretDocListInfo.pc_List->findFirst();
			if( NULL != pc_PtrScureDocListTbl ) {
				printf("\n Naveen Entered Response 3\n");
				
					cbResult[i]["secure_document_id"] = const_cast<const char *>(pc_PtrScureDocListTbl->sc_SecretDocId);
					printf("\n Value is %s \n",const_cast<const char *>(pc_PtrScureDocListTbl->sc_SecretDocId));
					i++;
					
					m_s_SecretDocListInfo.pc_List->del( pc_PtrScureDocListTbl );
					free( pc_PtrScureDocListTbl );	
			} else {
				// 文書リスト取得完了.
				break;
			}
		
		}
		
		// インスタンス破棄.
		pc_Doc->deleteInstance();

		m_ResponseHeader->Status = WAPD_STATUS_OK;
		m_ResponseHeader->ContentType       = const_cast<char *>("application/json");
		m_ResponseHeader->ContentTypeLength = static_cast<uint32_t>(strlen(m_ResponseHeader->ContentType));

		// 状態遷移[→初期状態].
		pc_DataMng->setStatus( WAPE_GetDocumentListStatus_Initialize );
		
		return WAPD_NoError;
		
		
	}
	
	// Naveen Added.

// HCL-Naveen No.38_BookletPrinting Mode2-WP 20190422 Added Start.

	if( WAPE_GetSecureDocListReqStatus_PassPenalityCheck == pc_DataMng->getSecureDocStatus()
		|| (0 != pc_DataMng->getMaxMatchedDocid()) )
	{
		Json::Value& cbResult = ( *m_Response )["password_match_list"];
		
		for (int32_t i = 0; i < pc_DataMng->getMaxMatchedDocid(); i++) {
			cbResult[i]["matched_doc_id"] = pc_DataMng->getMatchedDocidList(i);
			cbResult[i]["doc_penalty_status"] = false;
		}

	} else {
// HCL-Naveen No.38_BookletPrinting Mode2-WP 20190422 Added End.

	Json::Value& cResult = ( *m_Response )[WAPD_RespBodyParamGetDocList_DocData];
	cResult[WAPD_RespBodyParamGetDocList_DocListSize] = m_s_DocListInfo.pList->getCount();	// 文書リスト数をレスポンスボディにセット.
	printf("\nFunction : %s, Line : %d ## Response## Doc List size : %d \n", __FUNCTION__, __LINE__, m_s_DocListInfo.pList->getCount() );
	Json::Value& cDocList = cResult[WAPD_RespBodyParamGetDocList_DocList];

	// -------------------------------------------------------------------.
	//	文書リストからレスポンスデータを作成する.
	// -------------------------------------------------------------------.
	int iCount = 0;
	while( 1 ) {
		// 1番目のドキュメントリスト取得.
		pc_PtrScureListTbl = m_s_DocListInfo.pList->findFirst();
		if( NULL != pc_PtrScureListTbl ) {				// 最初のドキュメントリストがNULLではない = 処理中.
			if( NULL != pc_PtrScureListTbl->pJob ) {	// ジョブポインタが存在する場合は、受信中ジョブのためジョブスペックは必要ない.
				//---------------------------.
				// 受信中文書の場合.
				//---------------------------.
				ui32_JobId = pc_PtrScureListTbl->pJob->getJobStatus()->getJobID();			// 受信中JobのJobIDを取得.
				ui32_JobResourceId = pc_PtrScureListTbl->pJob->getJobResourceId();			// ジョブリソース番号取得.

				// レスポンスボディに追加.
				cDocList[ss_cntDocList][WAPD_RespBodyParamGetDocList_DocTime]	= 0;
				cDocList[ss_cntDocList][WAPD_RespBodyParamGetDocList_UserName] 	= const_cast<const char *>(pc_PtrScureListTbl->sc_UserName);
				cDocList[ss_cntDocList][WAPD_RespBodyParamGetDocList_PageCnt]	= 0;
				cDocList[ss_cntDocList][WAPD_RespBodyParamGetDocList_JobRecouceId]	= ui32_JobResourceId;
				cDocList[ss_cntDocList][WAPD_RespBodyParamGetDocList_JobStatus]		= WAPD_GetDocumentListJobStatusReceive;
			} else if( DOCD_EncFile_BoxId != m_si32_BoxId ) {
				//---------------------------.
				// 蓄積文書の場合.
				//---------------------------.
				// ユーザー名(登録元)取得領域初期化.
				memset( sc_docJobNameDisp, '\0', sizeof(sc_docJobNameDisp) );
				// ユーザー名(登録元)取得.
				getDocJobName( static_cast<int32_t>(pc_PtrScureListTbl->boxID), static_cast<int32_t>(pc_PtrScureListTbl->docID),
							   pc_Doc, sc_docJobNameDisp );
				// ジョブNo取得.
				getJobDispId( static_cast<int32_t>(pc_PtrScureListTbl->boxID), static_cast<int32_t>(pc_PtrScureListTbl->docID),
							  pc_Doc, &ui32_JobId );
				// 原稿カラー情報取得.
				getDocOriginalColor( static_cast<int32_t>(pc_PtrScureListTbl->boxID), static_cast<int32_t>(pc_PtrScureListTbl->docID),
									 &ui32_OriginalColorType );

				// レスポンスボディに追加.
				cDocList[ss_cntDocList][WAPD_RespBodyParamGetDocList_DocTime]	= static_cast<uint32_t>(pc_PtrScureListTbl->docTime);
				cDocList[ss_cntDocList][WAPD_RespBodyParamGetDocList_UserName] 	= const_cast<const char *>(sc_docJobNameDisp);
				cDocList[ss_cntDocList][WAPD_RespBodyParamGetDocList_PageCnt]	= static_cast<uint32_t>(pc_PtrScureListTbl->pageCnt);
				cDocList[ss_cntDocList][WAPD_RespBodyParamGetDocList_JobRecouceId]	= 0;
				cDocList[ss_cntDocList][WAPD_RespBodyParamGetDocList_JobStatus]		= WAPD_GetDocumentListJobStatusAccumulated;
				cDocList[ss_cntDocList][WAPD_RespBodyParamGetDocList_OriginalColor]	= ui32_OriginalColorType;
			} else {
				// 2020/09/02 nomura Start.
				// PKI暗号化Boxの場合、ジョブステータスは取得できないので、パラメータ設定しない.
				// レスポンスボディに追加.
				cDocList[ss_cntDocList][WAPD_RespBodyParamGetDocList_DocTime]	= static_cast<uint32_t>(pc_PtrScureListTbl->docTime);
				cDocList[ss_cntDocList][WAPD_RespBodyParamGetDocList_UserName] 	= const_cast<const char *>(pc_PtrScureListTbl->sc_UserName);
				cDocList[ss_cntDocList][WAPD_RespBodyParamGetDocList_PageCnt]	= static_cast<uint32_t>(pc_PtrScureListTbl->pageCnt);
				cDocList[ss_cntDocList][WAPD_RespBodyParamGetDocList_JobRecouceId]	= 0;
				cDocList[ss_cntDocList][WAPD_RespBodyParamGetDocList_JobStatus]		= WAPD_GetDocumentListJobStatusAccumulated;
				// 2020/09/02 nomura End.
			}
			//---------------------------.
			// 受信中・蓄積文書の共通処理.
			//---------------------------.
			// レスポンスボディに追加.
			cDocList[ss_cntDocList][WAPD_RespBodyParamGetDocList_DocId]		= static_cast<uint32_t>(pc_PtrScureListTbl->docID);
			cDocList[ss_cntDocList][WAPD_RespBodyParamGetDocList_TrackName]	= const_cast<const char *>(pc_PtrScureListTbl->sc_TrackName);
			cDocList[ss_cntDocList][WAPD_RespBodyParamGetDocList_DocName]	= const_cast<const char *>(pc_PtrScureListTbl->docName);
			iCount++;
			
			// Naveen Added.
			if( WAPE_GetSecureDocListReqStatus_Mode2SecDocList == pc_DataMng->getSecureDocStatus() )
			{
				cDocList[ss_cntDocList]["doc_penalty_status"] = false;
				
				int sc_count = APIC_SystemSetting::newInstance()->getDisableUserLoginCnt();	// ﾍﾟﾅﾙﾃｨを掛ける設定回数を取得.
				
				if(pc_PtrScureListTbl->secretDocNgCnt >= sc_count)
				{
					cDocList[ss_cntDocList]["doc_penalty_status"] = true;
					cDocList[ss_cntDocList]["penalty_count"] = pc_PtrScureListTbl->secretDocNgCnt;
				}
			}
			// Naveen Added.
			switch(pc_PtrScureListTbl->uc_CarbonCopyJobType) {
				case TYPD_CarbonCopyJobNoType:
					cDocList[ss_cntDocList][WAPD_RespBodyParamGetDocList_CarbonType] = WAPD_GetDocumentListCarbonCopyJobNoType0;
					break;
				case TYPD_CarbonCopyJobType1:
					cDocList[ss_cntDocList][WAPD_RespBodyParamGetDocList_CarbonType] = WAPD_GetDocumentListCarbonCopyJobNoType1;
					break;
				case TYPD_CarbonCopyJobType2:
					cDocList[ss_cntDocList][WAPD_RespBodyParamGetDocList_CarbonType] = WAPD_GetDocumentListCarbonCopyJobNoType2;
					break;
				default:
					cDocList[ss_cntDocList][WAPD_RespBodyParamGetDocList_CarbonType] = "";
					break;
			}

			// 2020/09/02 nomura Start.
			if( DOCD_EncFile_BoxId != m_si32_BoxId ) {
			cDocList[ss_cntDocList][WAPD_RespBodyParamGetDocList_JobDispId]	= ui32_JobId;
			}
			// 2020/09/02 nomura End.

			m_s_DocListInfo.pList->del( pc_PtrScureListTbl );								// リストノード解放.
			free( pc_PtrScureListTbl );														// 取得領域解放.
			ss_cntDocList++;																// カウントUP.
		} else {
			// 文書リスト取得完了.
			break;
		}
	}
	printf("\nFile : %s, Line : %d  #### Doc total Count = %d \n", __FILE__, __LINE__, iCount);
	
	}	// Naveen Added.	// HCL-Naveen No.38_BookletPrinting Mode2-WP 20190422 Added.
	// インスタンス破棄.
	pc_Doc->deleteInstance();

	m_ResponseHeader->Status = WAPD_STATUS_OK;
	m_ResponseHeader->ContentType       = const_cast<char *>("application/json");
	m_ResponseHeader->ContentTypeLength = static_cast<uint32_t>(strlen(m_ResponseHeader->ContentType));

	// 状態遷移[→初期状態].
	pc_DataMng->setStatus( WAPE_GetDocumentListStatus_Initialize );

	return WAPD_NoError;
}

/**
 * @brief		ジョブスペック取得待ち.
 * @return		なし.
 * @author		日立 田中.
 * @since		2017/12/20.
 */
void WAPC_GetDocumentList::lockAnsGetJobSpec()
{
	STATUS ret = SYSF_semTake( m_sem_AnsGetJobSpec, WAIT_FOREVER );		// 応答待機.
	if ( ERROR == ret ) {
		WAPG_API_trace("[ERR]%s SYSF_semTake() Error!.\n", __PRETTY_FUNCTION__ );
		// Fatal通知.
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
	}

	return;
}

/**
 * @brief		ジョブスペック取得待ち解除.
 * @return		なし.
 * @author		日立 田中.
 * @since		2017/12/20.
 */
void WAPC_GetDocumentList::unlockAnsGetJobSpec()
{
	STATUS ret = SYSF_semGive( m_sem_AnsGetJobSpec );		// 通知待ち解除.
	if ( ERROR == ret ) {
		WAPG_API_trace("[ERR]%s SYSF_semGive() Error!.\n", __PRETTY_FUNCTION__ );
		// Fatal通知.
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
	}

	return;
}

/**
 * @brief		ユーザ名取得.
 * @param[in]	si32_boxId ボックスID.
 * @param[in]	si32_docId 文書番号.
 * @param[in]	pc_Doc APIC_Docのインスタンス.
 * @param[out]	psc_UserName ユーザー名格納ポインタ.
 * @return		なし.
 * @author		日立 田中.
 * @since		2017/12/20.
 * @date		2023/03/01 日立佐藤菜 OP_BTS-40914 req_GetJobSpecの戻り値を確認するように修正.
 */
void WAPC_GetDocumentList::getDocJobName( int32_t si32_boxId, int32_t si32_docId, APIC_Doc* pc_Doc, char* psc_UserName )
{
	if( ( NULL == pc_Doc ) || ( NULL == psc_UserName ) ) {
		WAPG_API_trace("[ERR]%s NULL Parameter!.\n", __PRETTY_FUNCTION__ );
		return;
	}
	// BoxIDとDocIDを設定.
	pc_Doc->setBoxId( si32_boxId );
	pc_Doc->setDocId( si32_docId );
	memset( &m_c_JobSpec,		0x00, sizeof(TYPC_JobSpec) );
	memset( &m_c_Common,		0x00, sizeof(TYPC_Common) );
	memset( &m_c_Scan,			0x00, sizeof(TYPC_Scan) );
	memset( &m_c_Print,			0x00, sizeof(TYPC_Print) );
	memset( &m_c_Send,			0x00, sizeof(TYPC_Send) );
	memset( &m_c_Fax,			0x00, sizeof(TYPC_Fax) );
	memset( &m_c_Interpreter,	0x00, sizeof(TYPC_Interpreter) );
	memset( &m_c_IPP,			0x00, sizeof(TYPC_IPP) );
	m_c_JobSpec.pc_Common		= &m_c_Common;
	m_c_JobSpec.pc_Scan			= &m_c_Scan;
	m_c_JobSpec.pc_Print		= &m_c_Print;
	m_c_JobSpec.pc_Send			= &m_c_Send;
	m_c_JobSpec.pc_Fax			= &m_c_Fax;
	m_c_JobSpec.pc_Interpreter	= &m_c_Interpreter;
	m_c_JobSpec.pc_IPP			= &m_c_IPP;

	STATUS stat_ret = SYSF_semTake( m_sem_GetDocList, WAIT_FOREVER );
	if( ERROR == stat_ret ) {
		WAPG_API_trace("[ERR]%s SYSF_semTake() Error!.\n", __PRETTY_FUNCTION__ );
		// Fatal通知.
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
	}

	// ジョブスペック要求.
	int32_t i32_Ret = 0;
	i32_Ret = pc_Doc->req_GetJobSpec( APIC_User::newInstance( m_si32_UserId ),	// ユーザー情報.
							NULL,										// ボックスパスワード.
							&m_c_JobSpec,								// 取得したいJobSpec情報.
							&WAPC_GetDocumentList::ans_getJobSpec,		// コールバック関数.
							APIC_Track::newInstance( m_si32_TrackId ));		// 2022/08/19 FPT-TuanBV1 IT6_2.3 OP_BTS-39017 Modified.

	if( 0 != i32_Ret ){
		WAPG_API_trace("[ERR]%s req_GetJobSpec Error.\n", __PRETTY_FUNCTION__ );
		stat_ret = SYSF_semGive( m_sem_GetDocList );
		if ( ERROR == stat_ret ) {
			WAPG_API_trace("[ERR]%s SYSF_semGive() Error!.\n", __PRETTY_FUNCTION__ );
			// Fatal通知.
			LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
		}
		return;
	}

	lockAnsGetJobSpec();	// ジョブスペック要求のコールバック待ち.

	if( APID_DocapiMethodOk == m_si32_getJobSpecResult ) {
		TYPC_JobSpec* pc_JobSpec = &m_c_JobSpec;
		// ジョブスペックから登録元取得.
		pc_JobSpec->pc_Common->c_UserAcount.getUserNameUTF8( psc_UserName, TYPE_CharSetUTF8 );
	}
	stat_ret = SYSF_semGive( m_sem_GetDocList );
	if ( ERROR == stat_ret ) {
		WAPG_API_trace("[ERR]%s SYSF_semGive() Error!.\n", __PRETTY_FUNCTION__ );
		// Fatal通知.
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
	}
}

/**
 * @brief		蓄積文書のカラー情報取得.
 * @param[in]	si32_boxId ボックスID.
 * @param[in]	si32_docId 文書番号.
 * @param[out]	pui32_OriginalsColor 蓄積文書のカラー情報格納ポインタ.
 * @return		なし.
 * @author		XX
 * @since		2019/10/XX.
 */
void WAPC_GetDocumentList::getDocOriginalColor( int32_t si32_boxId, int32_t si32_docId, uint32_t* pui32_OriginalColorType )
{
	// パラメータチェック
	if ( NULL == pui32_OriginalColorType ) {
		WAPG_API_trace("[ERR]%s NULL Parameter!.\n", __PRETTY_FUNCTION__);
		return;
	}

	TYPC_JobStatus e_JobStatus;
	DOCC_DocFileAccess::getJobStatus( si32_boxId, si32_docId, reinterpret_cast<char*>(&e_JobStatus) );
	// ジョブステータスから原稿カラー情報を取得.
	*pui32_OriginalColorType = e_JobStatus.getOriginalsColorType();

	// For Debug: チェックイン時には削除すること.
	//printf("[%s][%d] OriginalColorType: %d\n", __FUNCTION__, __LINE__, *pui32_OriginalColorType);
}

// HCL-Naveen No.38_BookletPrinting Mode2-WP 20190422 Added Start.
// 2023/03/01 日立佐藤菜 OP_BTS-40914 req_GetJobSpecの戻り値を確認するように修正.
void WAPC_GetDocumentList::getSecureDocJobPass( int32_t si32_boxId, int32_t si32_docId, APIC_Doc* pc_Doc, char* psc_SecureDocPass )
{
	if( ( NULL == pc_Doc ) || ( NULL == psc_SecureDocPass ) ) {
		WAPG_API_trace("[ERR]%s NULL Parameter!.\n", __PRETTY_FUNCTION__ );
		return;
	}
	// BoxIDとDocIDを設定.
	pc_Doc->setBoxId( si32_boxId );
	pc_Doc->setDocId( si32_docId );
	memset( &m_c_JobSpec,		0x00, sizeof(TYPC_JobSpec) );
	memset( &m_c_Common,		0x00, sizeof(TYPC_Common) );
	memset( &m_c_Scan,			0x00, sizeof(TYPC_Scan) );
	memset( &m_c_Print,			0x00, sizeof(TYPC_Print) );
	memset( &m_c_Send,			0x00, sizeof(TYPC_Send) );
	memset( &m_c_Fax,			0x00, sizeof(TYPC_Fax) );
	memset( &m_c_Interpreter,	0x00, sizeof(TYPC_Interpreter) );
	memset( &m_c_IPP,			0x00, sizeof(TYPC_IPP) );
	m_c_JobSpec.pc_Common		= &m_c_Common;
	m_c_JobSpec.pc_Scan			= &m_c_Scan;
	m_c_JobSpec.pc_Print		= &m_c_Print;
	m_c_JobSpec.pc_Send			= &m_c_Send;
	m_c_JobSpec.pc_Fax			= &m_c_Fax;
	m_c_JobSpec.pc_Interpreter	= &m_c_Interpreter;
	m_c_JobSpec.pc_IPP			= &m_c_IPP;
	
	STATUS stat_ret = SYSF_semTake( m_sem_GetDocList, WAIT_FOREVER );
	if( ERROR == stat_ret ) {
		WAPG_API_trace("[ERR]%s SYSF_semTake() Error!.\n", __PRETTY_FUNCTION__ );
		// Fatal通知.
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
	}
	
	// ジョブスペック要求.
	int32_t i32_Ret = 0;
	i32_Ret = pc_Doc->req_GetJobSpec( APIC_User::newInstance( m_si32_UserId ),	// ユーザー情報.
							NULL,										// ボックスパスワード.
							&m_c_JobSpec,								// 取得したいJobSpec情報.
							&WAPC_GetDocumentList::ans_getJobSpec,		// コールバック関数.
							APIC_Track::newInstance( m_si32_TrackId ));		// 2022/08/19 FPT-TuanBV1 IT6_2.3 OP_BTS-39017 Modified.

	if( 0 != i32_Ret ){
		WAPG_API_trace("[ERR]%s req_GetJobSpec Error.\n", __PRETTY_FUNCTION__ );
		stat_ret = SYSF_semGive( m_sem_GetDocList );
		if ( ERROR == stat_ret ) {
			WAPG_API_trace("[ERR]%s SYSF_semGive() Error!.\n", __PRETTY_FUNCTION__ );
			// Fatal通知.
			LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
		}
		return;
	}

	lockAnsGetJobSpec();	// ジョブスペック要求のコールバック待ち.

	if( APID_DocapiMethodOk == m_si32_getJobSpecResult ) {
		TYPC_JobSpec* pc_JobSpec = &m_c_JobSpec;
		// ジョブスペックから登録元取得.
	//	pc_JobSpec->pc_Common->c_UserAcount.getUserNameUTF8( psc_UserName, TYPE_CharSetUTF8 );
		pc_JobSpec->pc_Common->getDocumentPassword(psc_SecureDocPass);
	}
	
	stat_ret = SYSF_semGive( m_sem_GetDocList );
	if ( ERROR == stat_ret ) {
		WAPG_API_trace("[ERR]%s SYSF_semGive() Error!.\n", __PRETTY_FUNCTION__ );
		// Fatal通知.
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
	}
}
// HCL-Naveen No.38_BookletPrinting Mode2-WP 20190422 Added End.


/**
 * @brief		ジョブNo取得.
 * @param[in]	si32_boxId ボックスID.
 * @param[in]	si32_docId 文書番号.
 * @param[in]	pc_Doc APIC_Docのインスタンス.
 * @param[out]	pui32_JobId ジョブNo格納ポインタ.
 * @return		なし.
 * @author		日立 田中.
 * @since		2017/12/20.
 */
void WAPC_GetDocumentList::getJobDispId( int32_t si32_boxId, int32_t si32_docId, APIC_Doc* pc_Doc, uint32_t* pui32_JobId )
{
	if( ( NULL == pc_Doc ) || ( NULL == pui32_JobId ) ) {
		WAPG_API_trace("[ERR]%s NULL Parameter!.\n", __PRETTY_FUNCTION__ );
		return;
	}
	// BoxIDとDocIDを設定.
	pc_Doc->setBoxId( si32_boxId );
	pc_Doc->setDocId( si32_docId );
	// ジョブNo取得.
	pc_Doc->getJobID( pui32_JobId );
}

//-----------------------------------------------------------------------------.
// WAPC_GetDocumentList_DataMngクラス.
//	GetDocumentList個別コントローラとコールバック処理間で共有するデータを格納する.
//-----------------------------------------------------------------------------.
/**
 * @brief		コンストラクタ.
 * @return		なし.
 * @author		日立 田中.
 * @since		2017/12/20.
 */
WAPC_GetDocumentList_DataMng::WAPC_GetDocumentList_DataMng()
{
	// セマフォ生成.
	m_SemID = SYSF_semMCreate( SEM_Q_PRIORITY | SEM_DELETE_SAFE | SEM_INVERSION_SAFE );
	if ( NULL == m_SemID ) {
		WAPG_API_trace("[ERR]%s SYSF_semMCreate() Error!.\n", __PRETTY_FUNCTION__ );
		// Fatal通知.
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
	}

	// 管理情報クリア.
	m_e_Status = WAPE_GetDocumentListStatus_Initialize;
	m_e_SecureDocStatus = WAPE_GetSecureDocListReqStatus_NoStatus; //【EagleBK】静的解析対応依頼 HCL-Chitra 13/09/2019 Modified.
}

/**
 * @brief		デストラクタ.
 * @return		なし.
 * @author		日立 田中.
 * @since		2017/12/20.
 */
WAPC_GetDocumentList_DataMng::~WAPC_GetDocumentList_DataMng()
{
	// セマフォ削除.
	if ( NULL != m_SemID ) {
		SYSF_semDelete(m_SemID);
	}
}

/**
 * @brief		インスタンス取得.
 * @return		なし.
 * @author		日立 田中.
 * @since		2017/12/20.
 */
WAPC_GetDocumentList_DataMng* WAPC_GetDocumentList_DataMng::newInstance()
{
	static WAPC_GetDocumentList_DataMng c_WAPC_GetDocumentList_DataMng;
	return &c_WAPC_GetDocumentList_DataMng;
}

/**
 * @brief		処理状態設定.
 * @param[in]	eStatus			:処理状態.
 * @return 		なし.
 * @author		日立 田中.
 * @since		2017/12/20.
 */
void WAPC_GetDocumentList_DataMng::setStatus( WAPE_GetDocumentListStatus e_Status )
{
	m_e_Status = e_Status;
	return;
}

/**
 * @brief		処理状態取得.
 * @param[in]	なし.
 * @return 		処理状態.
 * @author		日立 田中.
 * @since		2017/12/20.
 */
WAPE_GetDocumentListStatus WAPC_GetDocumentList_DataMng::getStatus( void )
{
	return m_e_Status;
}

// HCL-Naveen No.38_BookletPrinting Mode2-WP 20190422 Added Start.
void WAPC_GetDocumentList_DataMng::setMaxMatchedDocid( int32_t psi32_MatchedDocidListMax )
{
	si32_MatchedDocidListMax = psi32_MatchedDocidListMax;
}

int32_t WAPC_GetDocumentList_DataMng::getMaxMatchedDocid()
{
	return si32_MatchedDocidListMax;
}

void WAPC_GetDocumentList_DataMng::setMatchedDocidList( int32_t index, int32_t MatchedDocid )
{
	si32_MatchedDocidList[index] = MatchedDocid;
}

int32_t WAPC_GetDocumentList_DataMng::getMatchedDocidList( int32_t index )
{
	return si32_MatchedDocidList[index];
}
// HCL-Naveen No.38_BookletPrinting Mode2-WP 20190422 Added End.

// Naveen Added.
void WAPC_GetDocumentList_DataMng::setSecureDocStatus( WAPE_GetSecureDocListReqStatus e_Status )
{
	m_e_SecureDocStatus = e_Status;
	return;
}

WAPE_GetSecureDocListReqStatus WAPC_GetDocumentList_DataMng::getSecureDocStatus( void )
{
	return m_e_SecureDocStatus;
}
// Naveen Added.

/**
 * @brief		アクセスセマフォ取得.
 * @param[in]	なし.
 * @return 		なし.
 * @author		日立 田中.
 * @since		2017/12/20.
 */
void WAPC_GetDocumentList_DataMng::semTake( void )
{
	STATUS ret = SYSF_semTake( m_SemID, WAIT_FOREVER );
	if ( ERROR == ret ) {
		WAPG_API_trace("[ERR]%s SYSF_semTake() Error!.\n", __PRETTY_FUNCTION__ );
		// Fatal通知.
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
	}

	return;
}

/**
 * @brief		アクセスセマフォ開放.
 * @param[in]	なし.
 * @return 		なし.
 * @author		日立 田中.
 * @since		2017/12/20.
 */
void WAPC_GetDocumentList_DataMng::semGive( void )
{
	STATUS ret = SYSF_semGive( m_SemID );
	if ( ERROR == ret ) {
		WAPG_API_trace("[ERR]%s SYSF_semGive() Error!.\n", __PRETTY_FUNCTION__ );
		// Fatal通知.
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
	}

	return;
}
