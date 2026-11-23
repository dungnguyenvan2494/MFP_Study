//-----------------------------------------------------------------------------.
// @file		WAP_GetPreviewNew.cpp.
// @brief		プレビュー取得 個別コントローラクラス.
// @author		日立 西原.
// @since		2018/01/05.
// @date		2018/01/23 日立村上 2LAN対応_ボックスID指定の変更.
// @date		2018/03/12 日立田中 非同期改善対応.
//-----------------------------------------------------------------------------.
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <vector>
#include <string>

#include "SYSC_OSWrapper.h"
#include "json/json.h"
#include "json/reader.h"
#include "json/writer.h"
#include "json/value.h"

#include "system/sys/sysg/LOGF_debug.h"
#include "system/prv/APIC_PreviewConverter.h"
#include "system/doc/APIC_Doc.h"
#include "system/doc/DOCC_BoxIdDef.h"

#include "APIC_WebAPI.h"
#include "WAP_SubController.h"
#include "WAP_defs.h"
#include "WAP_plugintrace.h"
#include "WAP_CommonLib.h"
#include "WAP_BoxCommonLib.h"
#include "WAP_AsyncResponder.h"

#include "WAP_GetPreviewNew.h"
#include "WAP_Power.h"
#include "system/job/APIC_JobList.h"

#include <execinfo.h>

//-----------------------------------------------------------------------------.
// definitions.
//-----------------------------------------------------------------------------.

// ----------------------.
// プレビュー画像取得定数.
// ----------------------.


/*// 画像サイズ.
#ifdef DEF_RES_WSVGA
#ifdef DEF_THAMES_I
static const short		WAPD_ImgWidth 	= 340;					// プレビュー画像(幅).
static const short		WAPD_ImgHight 	= 340;					// プレビュー画像(高さ).
#else	//DEF_THAMES_I.
static const short		WAPD_ImgWidth 	= 350;					// プレビュー画像(幅).
static const short		WAPD_ImgHight 	= 350;					// プレビュー画像(高さ).
#endif	//DEF_THAMES_I.
#else //DEF_RES_WSVGA
#ifdef DEF_THAMES_I
static const short		WAPD_ImgWidth 	= 272;					// プレビュー画像(幅).
static const short		WAPD_ImgHight 	= 272;					// プレビュー画像(高さ).
#else	//DEF_THAMES_I.
static const short		WAPD_ImgWidth 	= 280;					// プレビュー画像(幅).
static const short		WAPD_ImgHight 	= 280;					// プレビュー画像(高さ).
#endif	//DEF_THAMES_I.
#endif //DEF_RES_WSVGA
*/

// [画像バッファサイズについて].
//   BlackAPI側の制御にてバッファサイズを８倍した領域を参照しているため×８としておく必要がある.
uint32_t	WAPD_imageSize	= 0;		// 画像バッファサイズ. 
uint32_t	WAPD_base64Size	= 0;		// Base64変換バッファサイズ.		

// 2023/06/09 FPT-ThuyNN8 OP_BTS-43599 Created Start.
const int32_t 	WAPD_TypeSmallScanSize = -1;
const int32_t	WAPD_TYPEReqImgTypeScan = 1; 	// 繧ｹ繧ｭ繝｣繝ｳ逕ｻ蜒?
const int32_t	WAPD_TYPEFileFormatJpeg = 5;	// 逕ｻ蜒上ヵ繧｡繧､繝ｫ蠖｢蠑擾ｼ夲ｼｪ?ｼｰ?ｼ･?ｼｧ蠖｢蠑? \n
const int32_t	WAPD_TYPEImageDir0 = 0; 		// 0蠎ｦ
const int32_t	WAPD_TYPEImagePlaceMemory = 0; 	// 繝｡繝｢繝ｪ
const int32_t	WAPD_TYPEThumbnailPriorityHigh = 0; // High;
// 2023/06/09 FPT-ThuyNN8 OP_BTS-43599 Created End.

// --------------.
// Jsonキーワード.
// --------------.

// ボディパラメータ.
typedef char WAPD_BodyParam;
	static const WAPD_BodyParam WAPD_BodyParam_ImageData[]		= "image_data";			// イメージデータ.
	static const WAPD_BodyParam WAPD_BodyParam_ErrorDetail[]	= "error_detail";		// エラー詳細.
	static const WAPD_BodyParam WAPD_BodyParam_ContentType[]	= "application/json";	// コンテンツタイプ.


//-----------------------------------------------------------------------------.
// テーブル定義.
//-----------------------------------------------------------------------------.

//-----------------------------------------------------------------------------.
// WAPC_GetPreviewNewクラス.
//-----------------------------------------------------------------------------.
/**
 * @brief		コンストラクタ.
 * @return		なし.
 * @author		日立 西原.
 * @since		2018/01/05.
 */
WAPC_GetPreviewNew::WAPC_GetPreviewNew()
{
	// 処理無し.
}

/**
 * @brief		デストラクタ.
 * @return		なし.
 * @author		日立 西原.
 * @since		2018/01/05.
 */
WAPC_GetPreviewNew::~WAPC_GetPreviewNew()
{
	// 処理無し.
}

/**
 * @brief		コントローラの初期化.
 * @return		初期化の成否, WAPD_NoError(0)：成功、WAPD_AnyError(1)：失敗.
 * @author		日立 西原.
 * @since		2018/01/05.
 */
int32_t WAPC_GetPreviewNew::Initialize()
{
	// 処理無し.
	return WAPD_NoError;
}

/**
 * @brief		コントローラの終了.
 * @return		初期化の成否, WAPD_NoError(0)：成功、WAPD_AnyError(1)：失敗.
 * @author		日立 西原.
 * @since		2018/01/05.
 */
int32_t WAPC_GetPreviewNew::Finalize()
{
	// 処理無し.
	return WAPD_NoError;
}

/**
 * @brief		コントローラのレスポンス生成.
 * @param[in]	ps_Param パラメータ構造体.
 * @return		レスポンス生成の成否, WAPD_NoError(0)：成功、WAPD_AnyError(1)：失敗.
 * @author		日立 西原.
 * @since		2018/01/05.
 * @date		2018/03/12 日立田中 非同期改善対応.
 */
int32_t WAPC_GetPreviewNew::ResponseGenerate( WAPS_SubController* ps_Param )
{
	int32_t si32_Ret = WAPD_AnyError;

	// パラメータNULLチェック.
	if( NULL == ps_Param ) {
		WAPG_API_trace("[ERR]%s NULL Parameter!.\n", __PRETTY_FUNCTION__ );
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
		// クライアントへ結果返却.
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		return WAPD_AnyError;
	}

	// 管理データアクセス用インスタンス取得.
	WAPC_GetPreviewNew_DataMng*	pc_DataMng = WAPC_GetPreviewNew_DataMng::newInstance();
	if( NULL == pc_DataMng ) {
		WAPG_API_trace("[ERR]%s WAPC_GetPreviewNew_DataMng::newInstance() is NULL!.\n", __PRETTY_FUNCTION__ );
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		return WAPD_AnyError;
	}

	// リクエスト処理の排他用セマフォ取得.
	pc_DataMng->takeProcSem();

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
		if ( WAPD_AnyError == si32_Ret ) {
			// WebAPI電源管理.
			WAPC_Power* pc_Power = WAPC_Power::newInstance();
			if ( NULL == pc_Power ) {
			    WAPG_API_trace("%s(%d) WAPC_Power::newInstance NULL 1\n", __FILE__, __LINE__);
			    LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, WAP_LOGF_LEVEL_FATAL, "WAPC_Power::newInstance NULL 1");
				return WAPD_AnyError;
			}

			// HDDデータアクセス用電源管理の省電力抑止解除.
			int32_t si32_RetForPow = pc_Power->EndAccessHDD();
			if( WAPD_AnyError == si32_RetForPow ){
				m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
				printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
				WAPG_API_trace("%s(%d) WAPC_Power::EndAccessHDD Error\n", __FILE__, __LINE__);
			}
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
	pc_DataMng->giveProcSem();

	return si32_Ret;
}

/**
 * @brief		コントローラの非同期レスポンス生成.
 * @param[in]	ps_Param パラメータ構造体.
 * @return		レスポンス生成の成否, WAPD_NoError(0)：成功、WAPD_AnyError(1)：失敗.
 * @author		日立 田中.
 * @since		2018/03/12.
 */
int32_t WAPC_GetPreviewNew::AsyncResponseGenerate( WAPS_SubController* ps_Param )
{
	int32_t si32_Ret = WAPD_AnyError;

	// パラメータNULLチェック.
	if( NULL == ps_Param ) {
		WAPG_API_trace("[ERR]%s NULL Parameter!.\n", __PRETTY_FUNCTION__ );
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
		// クライアントへ結果返却.
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		return WAPD_AnyError;
	}

	// 管理データアクセス用インスタンス取得.
	WAPC_GetPreviewNew_DataMng*	pc_DataMng = WAPC_GetPreviewNew_DataMng::newInstance();
	if( NULL == pc_DataMng ) {
		WAPG_API_trace("[ERR]%s WAPC_GetPreview_DataMng::newInstance() is NULL!.\n", __PRETTY_FUNCTION__ );
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		return WAPD_AnyError;
	}

	// リクエスト処理の排他用セマフォ取得.
	pc_DataMng->takeProcSem();

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
	if( WAPE_GetPreviewNewStatus_Initialize == pc_DataMng->getStatus() ) {
		// レスポンスクラスの取得.
		WAPC_AsyncResponse* pc_AsyncResponse = dynamic_cast<WAPC_AsyncResponse*>(WAPC_AsyncResponseContainer::newInstance()->getByHandle( m_pv_Handle ));
		if( NULL != pc_AsyncResponse ) {
			// 非同期レスポンスクラスのリストから削除.
			WAPC_AsyncResponseContainer::newInstance()->erase( pc_AsyncResponse->getReqId() );
			delete pc_AsyncResponse;
		}
	}

	// リクエスト処理の排他用セマフォ開放.
	pc_DataMng->giveProcSem();

	return si32_Ret;
}

/**
 * @brief		個別コントローラの共通非同期機能実行.
 * @return		非同期機能実行, WAPD_NoError(0)：成功、WAPD_AnyError(1)：失敗.
 * @author		日立 西原.
 * @since		2018/01/05.
 */
int32_t WAPC_GetPreviewNew::FunctionExecute()
{
	// WebAPI電源管理.
	WAPC_Power* pc_Power = WAPC_Power::newInstance();
	if ( NULL == pc_Power ) {
	    WAPG_API_trace("%s(%d) WAPC_Power::newInstance NULL 1\n", __FILE__, __LINE__);
	    LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, WAP_LOGF_LEVEL_FATAL, "WAPC_Power::newInstance NULL 1");
		return WAPD_AnyError;
	}

	// HDDデータアクセス用電源管理の省電力抑止解除.
	int32_t si32_Ret = pc_Power->EndAccessHDD();
	if( WAPD_AnyError == si32_Ret ){
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		WAPG_API_trace("%s(%d) WAPC_Power::EndAccessHDD Error\n", __FILE__, __LINE__);
	}

	return si32_Ret;
}

/**
 * @brief		プレビュー取得完了 コールバック関数.
 * @param[in]	ui32_AcceptNum			受付番号.
 * @param[in]	e_Result				処理結果.
 * @param[in]	s_ActualPreviewPara	プレビュー画像データ.
 * @return		なし.
 * @author		日立 西原.
 * @since		2018/01/05.
 * @date		2018/03/12 日立田中 非同期改善対応.
 */
void WAPC_GetPreviewNew::cb_notifyComplete( uint32_t ui32_AcceptNum, TYPE_PreviewResult e_Result, TYPS_ActualPreviewPara s_ActualPreviewPara)
{
	// 管理データアクセス用インスタンス取得.
	WAPC_GetPreviewNew_DataMng*	pc_DataMng = WAPC_GetPreviewNew_DataMng::newInstance();
	if( NULL == pc_DataMng ) {
		WAPG_API_trace("[ERR]%s WAPC_GetPreviewNew_DataMng::newInstance() is NULL!.\n", __PRETTY_FUNCTION__ );
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
		return;
	}

	// リクエスト処理の排他用セマフォ取得.
	pc_DataMng->takeProcSem();

	// 状態チェック.
	WAPE_GetPreviewNewStatus e_Step = pc_DataMng->getStatus();
	if( WAPE_GetPreviewNewStatus_WaitComp != e_Step ) {
		// [状態不一致].
		WAPG_API_trace("[ERR]%s Status Mismatch!.\n", __PRETTY_FUNCTION__ );
		// プレビュー画像データが存在する場合には念のため領域開放しておく.
		if( NULL != s_ActualPreviewPara.psc_ImageDataPointer ) {
			free( s_ActualPreviewPara.psc_ImageDataPointer );
		}

		// リクエスト処理の排他用セマフォ開放.
		pc_DataMng->giveProcSem();

		// コールバックを受け付けるべき状態でない場合には処理を行わずに無視する.
		return;
	}

	// 通知された画像データサイズは結果に関わらずここで登録
	pc_DataMng->setTotalDataSize( s_ActualPreviewPara.ul_TotalDataSize );

	// 受付番号チェック.
	uint32_t ui32_ReqNum = pc_DataMng->getAcceptNum();
	if( ui32_AcceptNum != ui32_ReqNum ) {
		// プレビュー画像取得要求時に発行された受付番号と不一致の場合.
		pc_DataMng->setConverterResult( WAPE_GetPreviewNewResultCode_SystemError );
	}
	else {
		// 非同期処理から受け取った結果をデータ管理オブジェクトの値に変換して登録.
		switch( e_Result ) {

		// [正常].
		case TYPE_PreviewResult_OK :
			pc_DataMng->setConverterResult( WAPE_GetPreviewNewResultCode_OK );
			break;

		// [失敗].
		case TYPE_PreviewResult_NG :
			pc_DataMng->setConverterResult( WAPE_GetPreviewNewResultCode_Failure );
			break;

		// [メモリオーバー].
		case TYPE_PreviewResult_MemoryOver :
			pc_DataMng->setConverterResult( WAPE_GetPreviewNewResultCode_Memory_Over );
			break;

		// [ドキュメントが存在しない].
		case TYPE_PreviewResult_Nothing :
			pc_DataMng->setConverterResult( WAPE_GetPreviewNewResultCode_Nothing );
			break;

		// [サポート外].
		case TYPE_PreviewResult_OutOfSupport :
			pc_DataMng->setConverterResult( WAPE_GetPreviewNewResultCode_Out_Of_Support );
			break;

		// [Cancel要求の応答].
		case TYPE_PreviewResult_Cancel :
			pc_DataMng->setConverterResult( WAPE_GetPreviewNewResultCode_SystemError );
			break;

		// [警告停止発生]
		case TYPE_PreviewResult_PrintError :
			pc_DataMng->setConverterResult( WAPE_GetPreviewNewResultCode_PrintError );
			break;

			// [その他].
		default :
			pc_DataMng->setConverterResult( WAPE_GetPreviewNewResultCode_SystemError );
			break;
		}
	}

	// 終了処理.
	// (1)全体コントローラーへ「非同期処理完了」を通知する.
	// レスポンスクラスの取得.
	WAPC_AsyncResponse* pc_AsyncResponse = dynamic_cast<WAPC_AsyncResponse*>(WAPC_AsyncResponseContainer::newInstance()->getById( reinterpret_cast<uint64_t>(cb_notifyComplete) ));
	if( NULL == pc_AsyncResponse ) {
		WAPG_API_trace("[ERR]%s pc_AsyncResponse is NULL!.\n", __PRETTY_FUNCTION__ );
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
		// リクエスト処理の排他用セマフォ開放.
		pc_DataMng->giveProcSem();
		return;
	}
	// メッセージ送信.
	WAPC_AsyncResponder::newInstance()->postMessage( pc_AsyncResponse->getHandle(), WAPD_NoError );

	// (2)状態遷移（→レスポンス応答待ち）.
	pc_DataMng->setStatus( WAPE_GetPreviewNewStatus_WaitResp );

	// (3)排他用セマフォ開放.
	pc_DataMng->giveProcSem();

	return;
}

/**
 * @brief		POST(非同期リクエスト)処理.
 * @param[in]	ps_Param HTTPリクエストヘッダ、ボディ.
 * @return		処理結果, WAPD_NoError(0)：成功、WAPD_AnyError(1)：失敗.
 * @author		日立 西原.
 * @since		2018/01/05.
 * @date		2018/01/23 日立村上 2LAN対応_ボックスID指定の変更.
 * @date		2018/03/12 日立田中 非同期改善対応.
 */
int32_t WAPC_GetPreviewNew::ExecPostRequest( const WAPS_SubController* ps_Param )
{
	int32_t			si32_ChkResult  = WAPD_STATUS_OK;	// 実行可否チェック結果(汎用).

	// 同期処理に初期化しておく.
	m_ResponseHeader->Async = WAPD_ASYNC_DISABLE;

	// パラメータNULLチェック.
	if( NULL == ps_Param ) {
		WAPG_API_trace("[ERR]%s NULL Parameter!.\n", __PRETTY_FUNCTION__ );
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
		// クライアントへ結果返却.
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		return WAPD_AnyError;
	}

	// 管理データアクセス用インスタンス取得.
	WAPC_GetPreviewNew_DataMng*	pc_DataMng = WAPC_GetPreviewNew_DataMng::newInstance();
	if( NULL == pc_DataMng ) {
		WAPG_API_trace("[ERR]%s WAPC_GetPreviewNew_DataMng::newInstance() is NULL!.\n", __PRETTY_FUNCTION__ );
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		return WAPD_AnyError;
	}

	// 状態チェック.
	WAPE_GetPreviewNewStatus e_Step = pc_DataMng->getStatus();
	if( WAPE_GetPreviewNewStatus_Initialize != e_Step ) {
		// [状態不一致].
		WAPG_API_trace("[ERR]%s Status Mismatch!.\n", __PRETTY_FUNCTION__ );
		// クライアントへ結果返却.
		printf("503:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //503->ErrorStatus[WAPD_STATUS_SERVICE_UNAVAILABLE] in WAP_defs.h
		m_ResponseHeader->Status = WAPD_STATUS_SERVICE_UNAVAILABLE;
		makeErrorDetail( WAPE_GetPreviewNewResultCode_Prosessing );
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

	// 共通ライブラリインスタンス取得.
	WAPC_BoxCommonLib*	cp_WAPC_BoxCommonLib = WAPC_BoxCommonLib::newInstance();
	if( NULL == cp_WAPC_BoxCommonLib ) {
		WAPG_API_trace("[ERR]%s WAPC_BoxCommonLib::newInstance() is NULL!.\n", __PRETTY_FUNCTION__ );
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		return WAPD_AnyError;
	}

	// Jsonパラメータ(DocID)取得.
	int32_t	si32_DocID = 0;
	Bool bResult = cp_WAPC_BoxCommonLib->getRequestParam_DocID( ps_Param->Param, &si32_DocID );
	if( False == bResult ) {
		// 取得失敗の場合.
		printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
		m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
		return WAPD_AnyError;
	}

	// パラメータ(BoxID)取得.
	int32_t	si32_BoxID = 0;
	bResult = cp_WAPC_BoxCommonLib->getRequestParam_BoxID( ps_Param->Param, ps_Param->UserNumber, &si32_BoxID , ps_Param->TrackNumber);	// 2022/08/19 FPT-TuanBV1 IT6_2.3 OP_BTS-39017 Modified.
	if( False == bResult ) {
		// 2023/06/09 FPT-ThuyNN8 OP_BTS-43599 Modified Start.
		if ( si32_DocID != WAPD_TypeSmallScanSize ) {
			// 取得失敗の場合.
			printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
			m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
			return WAPD_AnyError;
		}
		// 2023/06/09 FPT-ThuyNN8 OP_BTS-43599 Modified End.
	}

	// Jsonパラメータ(PageNum)取得.
	int32_t	si32_PageNum = 0;
	bResult = cp_WAPC_BoxCommonLib->getRequestParam_PageNum( ps_Param->Param, &si32_PageNum );
	if( False == bResult ) {
		printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
		m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
		return WAPD_AnyError;
	}

	//2023/6/02 DEVALL_OP-3555 ThuanVN3 Modified Start.
	//Jsonパラメータ(Image)パラメータ
    int32_t si32_ImageW = 0;
	int32_t si32_ImageH = 0;
	bResult = cp_WAPC_BoxCommonLib->getRequestParam_Image(ps_Param->Param, &si32_ImageW, &si32_ImageH );
	if( False == bResult ) {
		// 取得失敗の場合.
		printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
		m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
		return WAPD_AnyError;
	}
	WAPD_imageSize  = si32_ImageW*si32_ImageH*3 + 6;		// 画像バッファサイズ.
	WAPD_base64Size = ((WAPD_imageSize+1)*4)/3;			// Base64変換バッファサイズ.
	
	// 2023/06/09 FPT-ThuyNN8 OP_BTS-43599 Modified Start.
	uint32_t ui32_AcceptNum = 0;
	if ( si32_DocID == WAPD_TypeSmallScanSize ) {
		TYPS_ReqVerifyPara st_reqVerifyPara;
		APIC_ActiveJobList*	pc_ActiveJoblist = APIC_ActiveJobList::newInstance();
		if ( NULL == pc_ActiveJoblist ) {
			WAPG_API_trace("[ERR]%s APIC_ActiveJobList::newInstance() is NULL!.\n", __PRETTY_FUNCTION__ );
			LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
			m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
			printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
			return WAPD_AnyError;
		}
		APIC_Job*	pc_Job = pc_ActiveJoblist->getCurrentScanJob();
		st_reqVerifyPara.s_VerifyReqImgPara.sl_PageNum = si32_PageNum;
	    st_reqVerifyPara.s_VerifyReqImgPara.sl_PageNumAfterDelete = si32_PageNum;
	    st_reqVerifyPara.s_VerifyReqImgPara.sl_DocIndex = 0;
	    st_reqVerifyPara.s_VerifyReqImgPara.e_ReqImgType = WAPD_TYPEReqImgTypeScan;
	    st_reqVerifyPara.s_VerifyReqImgPara.e_VerifyFormat = WAPD_TYPEFileFormatJpeg;	// 逕ｻ蜒上ヵ繧｡繧､繝ｫ蠖｢蠑擾ｼ夲ｼｪ?ｼｰ?ｼ･?ｼｧ蠖｢蠑? \n

	    st_reqVerifyPara.s_VerifyReqImgPara.e_ImageDir = WAPD_TYPEImageDir0;
	    st_reqVerifyPara.s_VerifyReqImgPara.e_ImagePlace = WAPD_TYPEImagePlaceMemory;
	    st_reqVerifyPara.s_VerifyReqImgPara.e_Priority = WAPD_TYPEThumbnailPriorityHigh;
	    st_reqVerifyPara.s_VerifyReqImgPara.ss_ReqImageDotSizeWidth = si32_ImageW;
	    st_reqVerifyPara.s_VerifyReqImgPara.ss_ReqImageDotSizeHight = si32_ImageH;

	    st_reqVerifyPara.s_VerifyReqImgPara.psc_ImageDataPointer = static_cast<char *>( malloc(WAPD_imageSize) );
		if( NULL ==  st_reqVerifyPara.s_VerifyReqImgPara.psc_ImageDataPointer ) {
			// 領域確保失敗.
			WAPG_API_trace("[ERR]%s malloc() is NULL!.\n", __PRETTY_FUNCTION__ );
			LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
			m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
			printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
			return WAPD_AnyError;
		}
		st_reqVerifyPara.s_VerifyReqImgPara.ul_ImageDataPointerSize = WAPD_imageSize;
		memset( st_reqVerifyPara.s_VerifyReqImgPara.psc_ImageDataPointer, 0x00, WAPD_imageSize );
		// レスポンスクラスの取得.
		WAPC_AsyncResponse* pc_AsyncResponse = new WAPC_AsyncResponse;
		if( NULL == pc_AsyncResponse ) {
			free(st_reqVerifyPara.s_VerifyReqImgPara.psc_ImageDataPointer);
			WAPG_API_trace("[ERR]%s new WAPC_AsyncResponse() is NULL!.\n", __PRETTY_FUNCTION__ );
			LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
			// クライアントへ結果返却.
			m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
			printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
			return WAPD_AnyError;
		}
		pc_AsyncResponse->setHandle( m_pv_Handle );
		pc_AsyncResponse->setReqId( reinterpret_cast<uint64_t>(cb_notifyComplete) );
		WAPC_AsyncResponseContainer::newInstance()->push( pc_AsyncResponse );

	    st_reqVerifyPara.s_VerifyReqImgPara.psc_BGImageDataPointer = 0x00; // NULL
	    st_reqVerifyPara.s_VerifyReqImgPara.ul_BGImageDataPointerSize = 0;
	    st_reqVerifyPara.s_VerifyReqImgPara.psc_ImageDataFileName = 0x00;// NULL;

		int32_t	si32_JobID = pc_Job->getJobStatus()->getJobID();
	    st_reqVerifyPara.s_VerifyReqImgPara.ps_PreviewOnePOrder = &pc_Job->getPPreviewOPO(si32_JobID, si32_PageNum);

	    st_reqVerifyPara.s_VerifyReqImgPara.b_DoSynthesize = 0; // False
	    st_reqVerifyPara.pc_PrvProcess = 0x00; // NULL

		ui32_AcceptNum = pc_Job->req_getVerifyPreviewData( st_reqVerifyPara, &WAPC_GetPreviewNew::cb_notifyComplete );
		if( 0 == ui32_AcceptNum ){
			// 画像取得要求失敗.
			free(st_reqVerifyPara.s_VerifyReqImgPara.psc_ImageDataPointer);
			WAPG_API_trace("[ERR]%s req_getPreviewData() Error!.\n", __PRETTY_FUNCTION__ );
			LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
			m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
			printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
			WAPC_AsyncResponseContainer::newInstance()->erase( pc_AsyncResponse->getReqId() );
			delete pc_AsyncResponse;
			return WAPD_AnyError;
		}
		// 受付番号をデータ管理エリアに登録.
		pc_DataMng->setAcceptNum( ui32_AcceptNum );

		// 状態遷移[→Callback待ち].
		pc_DataMng->setStatus( WAPE_GetPreviewNewStatus_WaitComp );

		// 他の管理情報は非同期処理前に初期化しておく.
		pc_DataMng->setConverterResult( WAPE_GetPreviewNewResultCode_OK );
		pc_DataMng->setImageParam( st_reqVerifyPara.s_VerifyReqImgPara.psc_ImageDataPointer, 0 );
	} else {
		// BOXアクセス条件チェック.
		WAPS_CheckBoxDocAccess s_CheckBoxDocAccess;
		s_CheckBoxDocAccess.m_si32_UserID = ps_Param->UserNumber;
		s_CheckBoxDocAccess.m_si32_BoxId = si32_BoxID;
		si32_ChkResult = cp_WAPC_BoxCommonLib->checkBoxService( s_CheckBoxDocAccess, m_AccessToken );
		if( WAPD_STATUS_OK != si32_ChkResult ) {
			// BOXアクセスできない場合.
			m_ResponseHeader->Status = si32_ChkResult;
			if( WAPD_STATUS_SERVICE_UNAVAILABLE == si32_ChkResult ) {
				// アクセス不可の場合は詳細コード設定.
				makeErrorDetail( WAPE_GetPreviewNewResultCode_Inaccessible_State );
			}
			return WAPD_AnyError;
		}

		// ドキュメントアクセス権チェック.
		s_CheckBoxDocAccess.m_si32_DocList[0] = si32_DocID;
		si32_ChkResult = cp_WAPC_BoxCommonLib->checkDocAccess( s_CheckBoxDocAccess );
		if( WAPD_STATUS_OK != si32_ChkResult ) {
			// 指定ドキュメントにアクセスできない場合.
			m_ResponseHeader->Status = si32_ChkResult;
			return WAPD_AnyError;
		}

		// 選択可能ページ数内チェック.
		si32_ChkResult = cp_WAPC_BoxCommonLib->checkPageNum_Max( si32_BoxID, si32_DocID, si32_PageNum );
		if( WAPD_STATUS_OK != si32_ChkResult ) {
			// 選択可能ページ数外の場合.
			m_ResponseHeader->Status = si32_ChkResult;
			return WAPD_AnyError;
		}

		// プレビュー画像取得リクエスト.
		TYPS_ReqPreviewPara	st_ReqPreviewPara;

		st_ReqPreviewPara.sl_BoxID					= si32_BoxID;										// ボックス番号.
		st_ReqPreviewPara.sl_DocID					= si32_DocID;										// ドキュメント番号.
		st_ReqPreviewPara.sl_PageNum				= si32_PageNum;										// ページ番号	1～.
		st_ReqPreviewPara.e_PreviewFormat			= TYPE_FileFormatJpeg;								// 出力画像ファイル形式.
		st_ReqPreviewPara.e_ImageDir				= TYPE_ImageDir0 ;									// 画像回転角度.
		st_ReqPreviewPara.ss_ReqImageDotSizeWidth 	= si32_ImageW;										// 出力画像ドットサイズ(幅).
		st_ReqPreviewPara.ss_ReqImageDotSizeHight 	= si32_ImageH;										// 出力画像ドットサイズ(高さ).
		st_ReqPreviewPara.e_ImagePlace				= TYPE_ImagePlaceMemory;							// 画像データをメモリに出力するか、ファイルに出力するか.
		st_ReqPreviewPara.b_RtPreviewJob			= False;											// リアルタイムプレビュージョブかの判別用フラグ.
		st_ReqPreviewPara.psc_ImageDataPointer		= static_cast<char *>( malloc(WAPD_imageSize) );	// 画像受信領域を確保.
		if( NULL == st_ReqPreviewPara.psc_ImageDataPointer ) {
			// 領域確保失敗.
			WAPG_API_trace("[ERR]%s malloc() is NULL!.\n", __PRETTY_FUNCTION__ );
			LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
			m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
			printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
			return WAPD_AnyError;
		}

		st_ReqPreviewPara.ul_ImageDataPointerSize	= WAPD_imageSize;
		memset( st_ReqPreviewPara.psc_ImageDataPointer, 0x00, WAPD_imageSize );

		// レスポンスクラスの取得.
		WAPC_AsyncResponse* pc_AsyncResponse = new WAPC_AsyncResponse;
		if( NULL == pc_AsyncResponse ) {
			free(st_ReqPreviewPara.psc_ImageDataPointer);
			WAPG_API_trace("[ERR]%s new WAPC_AsyncResponse() is NULL!.\n", __PRETTY_FUNCTION__ );
			LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
			// クライアントへ結果返却.
			m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
			printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
			return WAPD_AnyError;
		}
		pc_AsyncResponse->setHandle( m_pv_Handle );
		pc_AsyncResponse->setReqId( reinterpret_cast<uint64_t>(cb_notifyComplete) );
		WAPC_AsyncResponseContainer::newInstance()->push( pc_AsyncResponse );

		ui32_AcceptNum = APIC_PreviewConverter::newInstance()->req_getPreviewData( st_ReqPreviewPara, &WAPC_GetPreviewNew::cb_notifyComplete );
		if( 0 == ui32_AcceptNum ){
			// 画像取得要求失敗.
			free( st_ReqPreviewPara.psc_ImageDataPointer );
			WAPG_API_trace("[ERR]%s req_getPreviewData() Error!.\n", __PRETTY_FUNCTION__ );
			LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
			m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
			printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
			WAPC_AsyncResponseContainer::newInstance()->erase( pc_AsyncResponse->getReqId() );
			delete pc_AsyncResponse;
			return WAPD_AnyError;
		}
		
		// 受付番号をデータ管理エリアに登録.
		pc_DataMng->setAcceptNum( ui32_AcceptNum );

		// 状態遷移[→Callback待ち].
		pc_DataMng->setStatus( WAPE_GetPreviewNewStatus_WaitComp );

		// 他の管理情報は非同期処理前に初期化しておく.
		pc_DataMng->setConverterResult( WAPE_GetPreviewNewResultCode_OK );
		pc_DataMng->setImageParam( st_ReqPreviewPara.psc_ImageDataPointer, 0 );
	}
	// 2023/06/09 FPT-ThuyNN8 OP_BTS-43599 Modified End.

	// 非同期設定(非同期処理を行う場合はこれが必要).
	m_ResponseHeader->Async = WAPD_ASYNC_ENABLE;

	m_ResponseHeader->Status = WAPD_STATUS_OK;
	return WAPD_NoError;
}

/**
 * @brief		POST(非同期レスポンス)処理.
 * @param[in]	ps_Param HTTPリクエストヘッダ、ボディ.
 * @return		処理結果, WAPD_NoError(0)：成功、WAPD_AnyError(1)：失敗.
 * @author		日立 西原.
 * @since		2018/01/05.
 * @date		2018/03/12 日立田中 非同期改善対応.
 */
int32_t WAPC_GetPreviewNew::ExecPostResponse( const WAPS_SubController* ps_Param )
{
	int32_t	si32_Result = WAPD_AnyError;

	// パラメータNULLチェック.
	if( NULL == ps_Param ) {
		WAPG_API_trace("[ERR]%s NULL Parameter!.\n", __PRETTY_FUNCTION__ );
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
		// クライアントへ結果返却.
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		return WAPD_AnyError;
	}

	// 管理データアクセス用インスタンス取得.
	WAPC_GetPreviewNew_DataMng*	pc_DataMng = WAPC_GetPreviewNew_DataMng::newInstance();
	if( NULL == pc_DataMng ) {
		WAPG_API_trace("[ERR]%s WAPC_GetPreviewNew_DataMng::newInstance() is NULL!.\n", __PRETTY_FUNCTION__ );
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		return WAPD_AnyError;
	}

	// 状態チェック.
	WAPE_GetPreviewNewStatus e_Step = pc_DataMng->getStatus();
	if( WAPE_GetPreviewNewStatus_WaitResp != e_Step ) {
		// [状態不一致].
		WAPG_API_trace("[ERR]%s Status Mismatch!.\n", __PRETTY_FUNCTION__ );

		// クライアントへ結果返却.
		printf("405:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //405->ErrorStatus[WAPD_STATUS_METHOD_NOT_ALLOWED] in WAP_defs.h
		m_ResponseHeader->Status = WAPD_STATUS_METHOD_NOT_ALLOWED;
		return WAPD_AnyError;
	}

	// 取得結果のチェック.
	WAPE_GetPreviewNewResultCode e_ResultCode = pc_DataMng->getConverterResult();
	char*		psc_ImageDataPointer = NULL;
	uint32_t	ui32_TotalDataSize = 0;
	pc_DataMng->getImageParam( &psc_ImageDataPointer, &ui32_TotalDataSize );

	// システム異常時.
	if( WAPE_GetPreviewNewResultCode_SystemError == e_ResultCode ) {
		WAPG_API_trace("[ERR]%s getConverterResult() Error!.\n", __PRETTY_FUNCTION__ );
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		si32_Result = WAPD_AnyError;
	}

	// その他のエラー発生時.
	else if( WAPE_GetPreviewNewResultCode_OK != e_ResultCode ) {
		WAPG_API_trace("[ERR]%s Service Unavailable!.\n", __PRETTY_FUNCTION__ );
		printf("503:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //503->ErrorStatus[WAPD_STATUS_SERVICE_UNAVAILABLE] in WAP_defs.h
		m_ResponseHeader->Status = WAPD_STATUS_SERVICE_UNAVAILABLE;
		makeErrorDetail( e_ResultCode );
		si32_Result = WAPD_AnyError;
	}

	else if( ( NULL == psc_ImageDataPointer ) || ( 0 == ui32_TotalDataSize ) ) {
		WAPG_API_trace("[ERR]%s getImageParam() Error!.\n", __PRETTY_FUNCTION__ );
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		si32_Result = WAPD_AnyError;
	}

	// 取得成功.
	else {
		// レスポンスボディ(取得画像情報)を生成.
		si32_Result = makeImageResponce( psc_ImageDataPointer, ui32_TotalDataSize );
	}

	// 画像取得に使用した領域を開放.
	if( NULL != psc_ImageDataPointer ) {
		free( psc_ImageDataPointer );
	}

	// 状態遷移[→初期状態].
	pc_DataMng->setStatus( WAPE_GetPreviewNewStatus_Initialize );

	return si32_Result;
}

/**
 * @brief		プレビュー画像レスポンスデータ生成.
 * @param[in]	psc_ImageDataPointer	: 設定する画像データポインタ.
 * @param[in]	ui32_TotalDataSize		: 設定する画像データサイズ.
 * @return		処理結果, WAPD_NoError(0)：成功、WAPD_AnyError(1)：失敗.
 * @author		日立 西原.
 * @since		2018/01/05.
 */
int32_t WAPC_GetPreviewNew::makeImageResponce( char* psc_ImageDataPointer, uint32_t ui32_TotalDataSize )
{

	// パラメータNULLチェック.
	if( NULL == psc_ImageDataPointer ) {
		WAPG_API_trace("[ERR]%s NULL Parameter!.\n", __PRETTY_FUNCTION__ );
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
		// レスポンスデータ(システム異常)生成.
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		return WAPD_AnyError;
	}

	// BASE64文字列変換領域を確保.
	char *psc_Base64 = static_cast<char *>( malloc( WAPD_base64Size ) ); 
	if( NULL == psc_Base64 ) {
		WAPG_API_trace("[ERR]%s malloc() is NULL!.\n", __PRETTY_FUNCTION__ );
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
		// レスポンスデータ(システム異常)生成.
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		return WAPD_AnyError;
	}

	// 取得した画像データをBASE64文字列に変換.
	memset( psc_Base64, 0, WAPD_base64Size ); 
	WAPC_CommonLib::newInstance()->chgBintoBase64( const_cast<const unsigned char *>(reinterpret_cast<const unsigned char*>(psc_ImageDataPointer)), ui32_TotalDataSize, psc_Base64 );
	// レスポンスデータ生成.
	m_Response = new Json::Value;
	if( NULL == m_Response ) {
		WAPG_API_trace("[ERR]%s new Json::Value() is NULL!.\n", __PRETTY_FUNCTION__ );
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
		// レスポンスデータ(システム異常)生成.
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h

		free( psc_Base64 );		// BASE64変換領域を開放.
		return WAPD_AnyError;
	}

	(*m_Response)[WAPD_BodyParam_ImageData] = psc_Base64;

	m_ResponseHeader->ContentType       = const_cast<char *>(WAPD_BodyParam_ContentType);
	m_ResponseHeader->ContentTypeLength = static_cast<uint32_t>(strlen(m_ResponseHeader->ContentType));

	m_ResponseHeader->Status = WAPD_STATUS_OK;

	free( psc_Base64 );		// BASE64変換領域を開放.
	return WAPD_NoError;
}


/**
 * @brief		プレビュー取得失敗要因データ生成.
 * @param[in]	e_ResultCode	: 画像取得結果(WAPE_GetPreviewNewResultCode).
 * @return		なし.
 * @author		日立 西原.
 * @since		2018/01/05.
 */
void WAPC_GetPreviewNew::makeErrorDetail( WAPE_GetPreviewNewResultCode e_ResultCode )
{
	static const char*	psc_ErrorDetailStr[] =
	{
		"PROSESSING"			,		// [ 1]処理実施中のため取得不可.
		"INACCESSIBLE_STATE"	,		// [ 2]文書アクセス不可状態.
		"FAILURE"				,		// [ 3]プレビュー画像取得失敗.
		"MEMORY_OVER"			,		// [ 4]メモリーオーバーフロー.
		"NOTHING"				,		// [ 5]取得画像なし.
		"PRINT_ERROR"			,		// [ 6]警告停止発生.
		"OUT_OF_SUPPORT"		,		// [ 7]サポート外.
		NULL
	};

	// 入力値範囲外の場合は補正(念のため).
	if( ( WAPE_GetPreviewNewResultCode_Prosessing > e_ResultCode ) || ( WAPE_GetPreviewNewResultCode_Out_Of_Support < e_ResultCode ) ) {
		e_ResultCode = WAPE_GetPreviewNewResultCode_Out_Of_Support;
	}

	m_Response = new Json::Value(Json::objectValue);
	if( NULL == m_Response ) {
		WAPG_API_trace("[ERR]%s new Json::Value() is NULL!.\n", __PRETTY_FUNCTION__ );
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
		// レスポンスデータ(システム異常)に差し替え.
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		return;
	}

	Json::Value cErrorCode((Json::Value)(psc_ErrorDetailStr[e_ResultCode-1]));
	(*m_Response)[WAPD_BodyParam_ErrorDetail]	= cErrorCode;

	m_ResponseHeader->ContentType       = const_cast<char *>(WAPD_BodyParam_ContentType);
	m_ResponseHeader->ContentTypeLength = static_cast<uint32_t>(strlen(m_ResponseHeader->ContentType));

	return;
}




//-----------------------------------------------------------------------------.
// WAPC_GetPreviewNew_DataMngクラス.
//	GetPreviewNew個別コントローラとコールバック処理間で共有するデータを格納する.
//-----------------------------------------------------------------------------.
/**
 * @brief		コンストラクタ.
 * @return		なし.
 * @author		日立 西原.
 * @since		2018/01/05.
 */
WAPC_GetPreviewNew_DataMng::WAPC_GetPreviewNew_DataMng()
{
	// リクエスト処理の排他用セマフォ生成.
	m_ProcSemID = SYSF_semMCreate( SEM_Q_PRIORITY | SEM_DELETE_SAFE | SEM_INVERSION_SAFE );
	if ( NULL == m_ProcSemID ) {
		WAPG_API_trace("[ERR]%s SYSF_semMCreate() Error!.\n", __PRETTY_FUNCTION__ );
		// Fatal通知.
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
	}

	// 管理情報クリア.
	m_e_Status = WAPE_GetPreviewNewStatus_Initialize;
	m_ui32_AcceptNum = 0;
	m_e_ResultCode = WAPE_GetPreviewNewResultCode_OK;
	memset( &m_s_ImageParam, 0, sizeof(m_s_ImageParam) );
}

/**
 * @brief		デストラクタ.
 * @return		なし.
 * @author		日立 西原.
 * @since		2018/01/05.
 */
WAPC_GetPreviewNew_DataMng::~WAPC_GetPreviewNew_DataMng()
{
	// セマフォ削除.
	if ( NULL != m_ProcSemID ) {
		SYSF_semDelete(m_ProcSemID);
	}
}

/**
 * @brief		インスタンス取得.
 * @return		なし.
 * @author		日立 西原.
 * @since		2018/01/05.
 */
WAPC_GetPreviewNew_DataMng* WAPC_GetPreviewNew_DataMng::newInstance()
{
	static WAPC_GetPreviewNew_DataMng c_WAPC_GetPreviewNew_DataMng;
	return &c_WAPC_GetPreviewNew_DataMng;
}

/**
 * @brief		リクエスト処理の排他用セマフォ取得.
 * @param		なし.
 * @return 		なし.
 * @author		日立 西原.
 * @since		2018/01/05.
 */
void WAPC_GetPreviewNew_DataMng::takeProcSem( void )
{
	STATUS ret = SYSF_semTake( m_ProcSemID, WAIT_FOREVER );
	if ( ERROR == ret ) {
		WAPG_API_trace("[ERR]%s SYSF_semTake() Error!.\n", __PRETTY_FUNCTION__ );
		// Fatal通知.
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
	}

	return;
}

/**
 * @brief		リクエスト処理の排他用セマフォ開放.
 * @param		なし.
 * @return 		なし.
 * @author		日立 西原.
 * @since		2018/01/05.
 */
void WAPC_GetPreviewNew_DataMng::giveProcSem( void )
{
	STATUS ret = SYSF_semGive( m_ProcSemID );
	if ( ERROR == ret ) {
		WAPG_API_trace("[ERR]%s SYSF_semGive() Error!.\n", __PRETTY_FUNCTION__ );
		// Fatal通知.
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
	}

	return;
}

/**
 * @brief		リクエスト受付状態の設定.
 * @param[in]	eStatus	: 処理状態(WAPE_GetPreviewNewStatus).
 * @return 		なし.
 * @author		日立 西原.
 * @since		2018/01/05.
 */
void WAPC_GetPreviewNew_DataMng::setStatus( WAPE_GetPreviewNewStatus e_Status )
{
	m_e_Status = e_Status;
	return;
}

/**
 * @brief		リクエスト受付状態の参照.
 * @param[in]	なし.
 * @return 		処理状態(WAPE_GetPreviewNewStatus).
 * @author		日立 西原.
 * @since		2018/01/05.
 */
WAPE_GetPreviewNewStatus WAPC_GetPreviewNew_DataMng::getStatus( void )
{
	return m_e_Status;
}

/**
 * @brief		画像取得要求受付番号の設定.
 * @param[in]	ui32_AcceptNum	: 受付番号(1～0xffffffff).
 * @return 		なし.
 * @author		日立 西原.
 * @since		2018/01/05.
 */
void WAPC_GetPreviewNew_DataMng::setAcceptNum( uint32_t ui32_AcceptNum )
{
	m_ui32_AcceptNum = ui32_AcceptNum;
	return;
}

/**
 * @brief		画像取得要求受付番号の参照.
 * @param		なし.
 * @return 		受付番号(1～0xffffffff).
 * @author		日立 西原.
 * @since		2018/01/05.
 */
uint32_t WAPC_GetPreviewNew_DataMng::getAcceptNum( void )
{
	return m_ui32_AcceptNum;
}

/**
 * @brief		画像取得結果の設定.
 * @param		si_ResultCode	: 画像取得結果(WAPE_GetPreviewNewResultCode).
 * @return 		なし.
 * @author		日立 西原.
 * @since		2018/01/05.
 */
void WAPC_GetPreviewNew_DataMng::setConverterResult( WAPE_GetPreviewNewResultCode e_ResultCode )
{
	m_e_ResultCode = e_ResultCode;
	return;
}

/**
 * @brief		画像取得結果の参照.
 * @param		なし.
 * @return 		画像取得結果(WAPE_GetPreviewNewResultCode).
 * @author		日立 西原.
 * @since		2018/01/05.
 */
WAPE_GetPreviewNewResultCode WAPC_GetPreviewNew_DataMng::getConverterResult( void )
{
	return m_e_ResultCode;
}

/**
 * @brief		出力画像パラメータの設定.
 * @param[in]	psc_ImageDataPointer	: 設定する画像データポインタ.
 * @param[in]	ui32_TotalDataSize		: 設定する画像データサイズ.
 * @return 		なし.
 * @author		日立 西原.
 * @since		2018/01/05.
 */
void WAPC_GetPreviewNew_DataMng::setImageParam( char* psc_ImageDataPointer, uint32_t ui32_TotalDataSize )
{
	m_s_ImageParam.psc_ImageDataPointer = psc_ImageDataPointer;
	m_s_ImageParam.ui32_TotalDataSize	= ui32_TotalDataSize;
	return;
}

/**
 * @brief		出力画像データサイズの設定.
 * @param[in]	ui32_TotalDataSize		: 設定する画像データサイズ.
 * @return 		なし.
 * @author		日立 西原.
 * @since		2018/01/05.
 */
void WAPC_GetPreviewNew_DataMng::setTotalDataSize( uint32_t ui32_TotalDataSize )
{
	m_s_ImageParam.ui32_TotalDataSize	= ui32_TotalDataSize;
	return;
}

/**
 * @brief		出力画像パラメータの参照.
 * @param[out]	psc_ImageDataPointer	: 取得した画像データポインタ.
 * @param[out]	ui32_TotalDataSize		: 取得した画像データサイズ.
 * @return 		なし.
 * @author		日立 西原.
 * @since		2018/01/05.
 */
void WAPC_GetPreviewNew_DataMng::getImageParam( char** psc_ImageDataPointer, uint32_t* pui32_TotalDataSize )
{
	*psc_ImageDataPointer	= m_s_ImageParam.psc_ImageDataPointer;
	*pui32_TotalDataSize	= m_s_ImageParam.ui32_TotalDataSize;
	return;
}

