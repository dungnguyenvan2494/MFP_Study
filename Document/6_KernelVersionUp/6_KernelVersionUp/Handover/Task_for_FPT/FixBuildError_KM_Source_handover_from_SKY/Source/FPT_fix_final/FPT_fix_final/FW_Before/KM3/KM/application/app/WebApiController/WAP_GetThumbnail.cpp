//-----------------------------------------------------------------------------.
// @file		WAP_GetThumbnail.cpp.
// @brief		サムネイル取得 個別コントローラクラス.
// @author		日立 西原.
// @since		2017/12/25.
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
#include "system/prv/APIC_ThumbnailConverter.h"
#include "system/nvd/APIC_MyPanelSetting.h" //2018/10/22 EagleH 制御構成変更_No.53 sindhu added.
#include "system/doc/APIC_Doc.h"
#include "system/doc/DOCC_BoxIdDef.h"
#include "system/jlg/APIC_JLGDefine.h"

#include "APIC_WebAPI.h"
#include "WAP_SubController.h"
#include "WAP_defs.h"
#include "WAP_plugintrace.h"
#include "WAP_CommonLib.h"
#include "WAP_BoxCommonLib.h"
#include "WAP_AsyncResponder.h"

#include "WAP_GetThumbnail.h"
#include "WAP_Power.h"

//-----------------------------------------------------------------------------.
// definitions.
//-----------------------------------------------------------------------------.

// ----------------------.
// サムネイル画像取得定数.
// ----------------------.

// 画像サイズ.
#ifdef DEF_RES_WSVGA
static const short		WAPD_ImgWidth 	= 130;					// サムネイル画像(幅).
static const short		WAPD_ImgHight 	= 130;					// サムネイル画像(高さ).
/*#else //DEF_RES_WSVGA
static const short		WAPD_ImgWidth 	= 104;					// サムネイル画像(幅).
static const short		WAPD_ImgHight 	= 104;					// サムネイル画像(高さ).*/
#endif //DEF_RES_WSVGA
/*static const uint32_t	WAPD_ImageSize	= WAPD_ImgWidth*WAPD_ImgHight*3;	// 画像バッファサイズ.
static const uint32_t	WAPD_Base64Size	= ((WAPD_ImageSize+1)*4)/3;			// Base64変換バッファサイズ.*/
	uint32_t	WAPD_ImageSize;    //2018/10/22 EagleH 制御構成変更_No.53 sindhu added.
	uint32_t	WAPD_Base64Size;   //2018/10/22 EagleH 制御構成変更_No.53 sindhu added.

// 再作成指定.
// Converter側制限によりSIMではTureを使用する必要があるために定義している.
#ifdef SIMULATOR
static const Bool		WAPD_Recreate	= True;
#else
static const Bool		WAPD_Recreate	= False;
#endif

static const uint32_t	WAPD_PageNum	= 1;		// ページ番号.


// --------------.
// Jsonキーワード.
// --------------.

// ボディパラメータ.
typedef char WAPD_BodyParam;
	static const WAPD_BodyParam WAPD_BodyParam_ImageData[]		= "image_data";			// イメージデータ.
	static const WAPD_BodyParam WAPD_BodyParam_ImageSize[]		= "image_size";			// イメージサイズ.
	static const WAPD_BodyParam WAPD_BodyParam_X[]				= "x";					// イメージ横幅.
	static const WAPD_BodyParam WAPD_BodyParam_Y[]				= "y";					// イメージ縦幅.
	static const WAPD_BodyParam WAPD_BodyParam_ErrorDetail[]	= "error_detail";		// エラー詳細.
	static const WAPD_BodyParam WAPD_BodyParam_ContentType[]	= "application/json";	// コンテンツタイプ.


//-----------------------------------------------------------------------------.
// テーブル定義.
//-----------------------------------------------------------------------------.

//-----------------------------------------------------------------------------.
// WAPC_GetThumbnailクラス.
//-----------------------------------------------------------------------------.
/**
 * @brief		コンストラクタ.
 * @return		なし.
 * @author		日立 西原.
 * @since		2017/12/25.
 */
WAPC_GetThumbnail::WAPC_GetThumbnail()
{
	// 処理無し.
}

/**
 * @brief		デストラクタ.
 * @return		なし.
 * @author		日立 西原.
 * @since		2017/12/25.
 */
WAPC_GetThumbnail::~WAPC_GetThumbnail()
{
	// 処理無し.
}

/**
 * @brief		コントローラの初期化.
 * @return		初期化の成否, WAPD_NoError(0)：成功、WAPD_AnyError(1)：失敗.
 * @author		日立 西原.
 * @since		2017/12/25.
 */
int32_t WAPC_GetThumbnail::Initialize()
{
	// 処理無し.
	return WAPD_NoError;
}

/**
 * @brief		コントローラの終了.
 * @return		初期化の成否, WAPD_NoError(0)：成功、WAPD_AnyError(1)：失敗.
 * @author		日立 西原.
 * @since		2017/12/25.
 */
int32_t WAPC_GetThumbnail::Finalize()
{
	// 処理無し.
	return WAPD_NoError;
}

/**
 * @brief		コントローラのレスポンス生成.
 * @param[in]	ps_Param パラメータ構造体.
 * @return		レスポンス生成の成否, WAPD_NoError(0)：成功、WAPD_AnyError(1)：失敗.
 * @author		日立 西原.
 * @since		2017/12/25.
 * @date		2018/03/12 日立田中 非同期改善対応.
 */
int32_t WAPC_GetThumbnail::ResponseGenerate( WAPS_SubController* ps_Param )
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
	WAPC_GetThumbnail_DataMng*	pc_DataMng = WAPC_GetThumbnail_DataMng::newInstance();
	if( NULL == pc_DataMng ) {
		WAPG_API_trace("[ERR]%s WAPC_GetThumbnail_DataMng::newInstance() is NULL!.\n", __PRETTY_FUNCTION__ );
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
int32_t WAPC_GetThumbnail::AsyncResponseGenerate( WAPS_SubController* ps_Param )
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
	WAPC_GetThumbnail_DataMng*	pc_DataMng = WAPC_GetThumbnail_DataMng::newInstance();
	if( NULL == pc_DataMng ) {
		WAPG_API_trace("[ERR]%s WAPC_GetThumbnail_DataMng::newInstance() is NULL!.\n", __PRETTY_FUNCTION__ );
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
	if( WAPE_GetThumbnailStatus_Initialize == pc_DataMng->getStatus() ) {
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
 * @since		2017/12/25.
 */
int32_t WAPC_GetThumbnail::FunctionExecute()
{
	// 処理無し.
	return WAPD_NoError;
}

/**
 * @brief		サムネイル取得完了 コールバック関数.
 * @param[in]	ui32_AcceptNum			受付番号.
 * @param[in]	e_Result				処理結果.
 * @param[in]	s_ActualThumbnailPara	サムネイル画像データ.
 * @return		なし.
 * @author		日立 西原.
 * @since		2017/12/25.
 * @date		2018/03/12 日立田中 非同期改善対応.
 */
void WAPC_GetThumbnail::cb_notifyComplete( uint32_t ui32_AcceptNum, TYPE_ThumbnailResult e_Result, TYPS_ActualThumbnailPara s_ActualThumbnailPara)
{
	// 管理データアクセス用インスタンス取得.
	WAPC_GetThumbnail_DataMng*	pc_DataMng = WAPC_GetThumbnail_DataMng::newInstance();
	if( NULL == pc_DataMng ) {
		WAPG_API_trace("[ERR]%s WAPC_GetThumbnail_DataMng::newInstance() is NULL!.\n", __PRETTY_FUNCTION__ );
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
		return;
	}

	// リクエスト処理の排他用セマフォ取得.
	pc_DataMng->takeProcSem();

	// 状態チェック.
	WAPE_GetThumbnailStatus e_Step = pc_DataMng->getStatus();
	if( WAPE_GetThumbnailStatus_WaitComp != e_Step ) {
		// [状態不一致].
		WAPG_API_trace("[ERR]%s Status Mismatch!.\n", __PRETTY_FUNCTION__ );

		// サムネイル画像データが存在する場合には念のため領域開放しておく.
		if( NULL != s_ActualThumbnailPara.psc_ImageDataPointer ) {
			free( s_ActualThumbnailPara.psc_ImageDataPointer );
		}

		// リクエスト処理の排他用セマフォ開放.
		pc_DataMng->giveProcSem();

		// コールバックを受け付けるべき状態でない場合には処理を行わずに無視する.
		return;
	}

	// 通知された画像データサイズは結果に関わらずここで登録
	pc_DataMng->setTotalDataSize( s_ActualThumbnailPara.ul_TotalDataSize );
	pc_DataMng->setImageSize( s_ActualThumbnailPara.ss_ImageDotSizeWidth, s_ActualThumbnailPara.ss_ImageDotSizeHight );

	// 受付番号チェック.
	uint32_t ui32_ReqNum = pc_DataMng->getAcceptNum();
	if( ui32_AcceptNum != ui32_ReqNum ) {
		// サムネイル画像取得要求時に発行された受付番号と不一致の場合.
		pc_DataMng->setConverterResult( WAPE_GetThumbnailResultCode_SystemError );
	}
	else {
		// 非同期処理から受け取った結果をデータ管理オブジェクトの値に変換して登録.
		switch( e_Result ) {

		// [正常].
		case TYPE_ThumbnailResult_OK :
			pc_DataMng->setConverterResult( WAPE_GetThumbnailResultCode_OK );
			break;

		// [失敗].
		case TYPE_ThumbnailResult_NG :
			pc_DataMng->setConverterResult( WAPE_GetThumbnailResultCode_Failure );
			break;

		// [メモリオーバー].
		case TYPE_ThumbnailResult_MemoryOver :
			pc_DataMng->setConverterResult( WAPE_GetThumbnailResultCode_Memory_Over );
			break;

		// [ドキュメントが存在しない].
		case TYPE_ThumbnailResult_Nothing :
			pc_DataMng->setConverterResult( WAPE_GetThumbnailResultCode_Nothing );
			break;

		// [サポート外].
		case TYPE_ThumbnailResult_OutOfSupport :
			pc_DataMng->setConverterResult( WAPE_GetThumbnailResultCode_Out_Of_Support );
			break;

		// [Cancel要求の応答].
		case TYPE_ThumbnailResult_Cancel :
			pc_DataMng->setConverterResult( WAPE_GetThumbnailResultCode_SystemError );
			break;

		// [その他].
		default :
			pc_DataMng->setConverterResult( WAPE_GetThumbnailResultCode_SystemError );
			break;
		}
	}
	int32_t		si32_DocID = 0;		// 文書ID.
	int32_t 	si32_BoxID = 0;		// ボックスID.
	pc_DataMng->getDocBoxID( &si32_DocID, &si32_BoxID );
	
	addOpeLogThumbnailImage( si32_DocID, si32_BoxID, e_Result );

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
	pc_DataMng->setStatus( WAPE_GetThumbnailStatus_WaitResp );

	// (3)排他用セマフォ開放.
	pc_DataMng->giveProcSem();

	return;
}

/**
 * @brief		POST(非同期リクエスト)処理.
 * @param[in]	ps_Param HTTPリクエストヘッダ、ボディ.
 * @return		処理結果, WAPD_NoError(0)：成功、WAPD_AnyError(1)：失敗.
 * @author		日立 西原.
 * @since		2017/12/25.
 * @date		2018/01/23 日立村上 2LAN対応_ボックスID指定の変更.
 * @date		2018/03/12 日立田中 非同期改善対応.
 */
int32_t WAPC_GetThumbnail::ExecPostRequest( const WAPS_SubController* ps_Param )
{
	int32_t		si32_ChkResult	= WAPD_STATUS_OK;	// 実行可否チェック結果(汎用).

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
	WAPC_GetThumbnail_DataMng*	pc_DataMng = WAPC_GetThumbnail_DataMng::newInstance();
	if( NULL == pc_DataMng ) {
		WAPG_API_trace("[ERR]%s WAPC_GetThumbnail_DataMng::newInstance() is NULL!.\n", __PRETTY_FUNCTION__ );
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		return WAPD_AnyError;
	}

	// 状態チェック.
	WAPE_GetThumbnailStatus e_Step = pc_DataMng->getStatus();
	if( WAPE_GetThumbnailStatus_Initialize != e_Step ) {
		// [状態不一致].
		WAPG_API_trace("[ERR]%s Status Mismatch!.\n", __PRETTY_FUNCTION__ );
		// クライアントへ結果返却.
		printf("503:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //503->ErrorStatus[WAPD_STATUS_SERVICE_UNAVAILABLE] in WAP_defs.h
		m_ResponseHeader->Status = WAPD_STATUS_SERVICE_UNAVAILABLE;
		makeErrorDetail( WAPE_GetThumbnailResultCode_Prosessing );
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

	// パラメータ(BoxID)取得.
	int32_t	si32_BoxID = 0;
	Bool bResult = cp_WAPC_BoxCommonLib->getRequestParam_BoxID( ps_Param->Param, ps_Param->UserNumber, &si32_BoxID , ps_Param->TrackNumber);	// 2022/08/19 FPT-TuanBV1 IT6_2.3 OP_BTS-39017 Modified.
	if( False == bResult ) {
		// 取得失敗の場合.
		printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
		m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
		return WAPD_AnyError;
	}

	// Jsonパラメータ(DocID)取得.
	int32_t	si32_DocID = 0;
	bResult = cp_WAPC_BoxCommonLib->getRequestParam_DocID( ps_Param->Param, &si32_DocID );
	if( False == bResult ) {
		// 取得失敗の場合.
		printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
		m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
		return WAPD_AnyError;
	}

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
			makeErrorDetail( WAPE_GetThumbnailResultCode_Inaccessible_State );
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


	// サムネイル画像取得リクエスト.
	TYPS_ReqThumbnailPara	st_ReqThumbnailPara;
	//2018/10/22 EagleH 制御構成変更_No.53 Start .
	APIC_PanelSetting* pc_PanelSetting = APIC_PanelSetting::newInstance(); 
	if( NULL == pc_PanelSetting ){
	WAPG_API_trace("[ERR]%s APIC_PanelSetting::newInstance() is NULL!.\n", __PRETTY_FUNCTION__ );
	LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
	m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
	printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
	return WAPD_AnyError; 
	}
#ifndef DEF_RES_WSVGA
	uint32_t		WAPD_ImgWidth;
	uint32_t		WAPD_ImgHight;
	WAPD_ImgWidth = pc_PanelSetting ->getThumbnailLength(); 
	WAPD_ImgHight = pc_PanelSetting ->getThumbnailLength();
	WAPD_ImageSize = WAPD_ImgWidth*WAPD_ImgHight*3;
	WAPD_Base64Size = ((WAPD_ImageSize+1)*4)/3;
#endif
	//2018/10/22 EagleH 制御構成変更_No.53 End .
	st_ReqThumbnailPara.sl_BoxID				= si32_BoxID;										// ボックス番号.
	st_ReqThumbnailPara.sl_DocID				= si32_DocID;										// ドキュメント番号.
	st_ReqThumbnailPara.sl_PageNum				= WAPD_PageNum;										// ページ番号	1～.
	st_ReqThumbnailPara.e_ThumbnailFormat		= TYPE_FileFormatJpeg;								// 出力画像ファイル形式.
	st_ReqThumbnailPara.e_ImageDir				= TYPE_ImageDir0;									// 画像回転角度.
	st_ReqThumbnailPara.ss_ReqImageDotSizeWidth = WAPD_ImgWidth;									// 出力画像ドットサイズ(幅)		80 - 160  (機能仕様書の制限).
	st_ReqThumbnailPara.ss_ReqImageDotSizeHight = WAPD_ImgHight;									// 出力画像ドットサイズ(高さ)	80 - 160  (機能仕様書の制限).
	st_ReqThumbnailPara.e_ImagePlace			= TYPE_ImagePlaceMemory;							// 画像データをメモリに出力するか、ファイルに出力するか.
	st_ReqThumbnailPara.e_Priority				= TYPE_ThumbnailPriority1;							// 優先順位.
	st_ReqThumbnailPara.b_Recreate				= WAPD_Recreate;									// サムネイル再作成.
	st_ReqThumbnailPara.psc_ImageDataPointer	= static_cast<char *>( malloc(WAPD_ImageSize) );	// 画像受信領域を確保.
	if( NULL == st_ReqThumbnailPara.psc_ImageDataPointer ) {
		// 領域確保失敗.
		WAPG_API_trace("[ERR]%s malloc() is NULL!.\n", __PRETTY_FUNCTION__ );
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		return WAPD_AnyError;
	}

	st_ReqThumbnailPara.ul_ImageDataPointerSize	= WAPD_ImageSize;
	memset( st_ReqThumbnailPara.psc_ImageDataPointer, 0, WAPD_ImageSize );

	// レスポンスクラスの取得.
	WAPC_AsyncResponse* pc_AsyncResponse = new WAPC_AsyncResponse;
	if( NULL == pc_AsyncResponse ) {
		free( st_ReqThumbnailPara.psc_ImageDataPointer );
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

	// 文書ID ボックスID保存.
	pc_DataMng->setDocBoxID( si32_DocID, si32_BoxID );

	// アクセストークン.
	pc_DataMng->setAccessToken( m_AccessToken );

	uint32_t ui32_AcceptNum = APIC_ThumbnailConverter::newInstance()->req_getThumbnailData( st_ReqThumbnailPara, &WAPC_GetThumbnail::cb_notifyComplete );
	if( 0 == ui32_AcceptNum ){
		// 画像取得要求失敗.
		free( st_ReqThumbnailPara.psc_ImageDataPointer );
		WAPG_API_trace("[ERR]%s req_getThumbnailData() Error!.\n", __PRETTY_FUNCTION__ );
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
	pc_DataMng->setStatus( WAPE_GetThumbnailStatus_WaitComp );

	// 他の管理情報は非同期処理前に初期化しておく.
	pc_DataMng->setConverterResult( WAPE_GetThumbnailResultCode_OK );
	pc_DataMng->setImageParam( st_ReqThumbnailPara.psc_ImageDataPointer, 0 );
	pc_DataMng->setImageSize( 0, 0 );

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
 * @since		2017/12/25.
 * @date		2018/03/12 日立田中 非同期改善対応.
 */
int32_t WAPC_GetThumbnail::ExecPostResponse( const WAPS_SubController* ps_Param )
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
	WAPC_GetThumbnail_DataMng*	pc_DataMng = WAPC_GetThumbnail_DataMng::newInstance();
	if( NULL == pc_DataMng ) {
		WAPG_API_trace("[ERR]%s WAPC_GetThumbnail_DataMng::newInstance() is NULL!.\n", __PRETTY_FUNCTION__ );
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		return WAPD_AnyError;
	}

	// 状態チェック.
	WAPE_GetThumbnailStatus e_Step = pc_DataMng->getStatus();
	if( WAPE_GetThumbnailStatus_WaitResp != e_Step ) {
		// [状態不一致].
		WAPG_API_trace("[ERR]%s Status Mismatch!.\n", __PRETTY_FUNCTION__ );

		// クライアントへ結果返却.
		printf("405:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //405->ErrorStatus[WAPD_STATUS_METHOD_NOT_ALLOWED] in WAP_defs.h
		m_ResponseHeader->Status = WAPD_STATUS_METHOD_NOT_ALLOWED;
		return WAPD_AnyError;
	}

	// 取得結果のチェック.
	WAPE_GetThumbnailResultCode e_ResultCode = pc_DataMng->getConverterResult();
	char*		psc_ImageDataPointer = NULL;
	uint32_t	ui32_TotalDataSize = 0;
	short		ss_ImageDotSizeWidth = 0;
	short		ss_ImageDotSizeHight = 0;
	pc_DataMng->getImageParam( &psc_ImageDataPointer, &ui32_TotalDataSize );
	pc_DataMng->getImageSize( &ss_ImageDotSizeWidth, &ss_ImageDotSizeHight );

	// システム異常時.
	if( WAPE_GetThumbnailResultCode_SystemError == e_ResultCode ) {
		WAPG_API_trace("[ERR]%s getConverterResult() Error!.\n", __PRETTY_FUNCTION__ );
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		si32_Result = WAPD_AnyError;
	}

	// その他のエラー発生時.
	else if( WAPE_GetThumbnailResultCode_OK != e_ResultCode ) {
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
		si32_Result = makeImageResponce( psc_ImageDataPointer, ui32_TotalDataSize, ss_ImageDotSizeWidth, ss_ImageDotSizeHight );
	}

	// 画像取得に使用した領域を開放.
	if( NULL != psc_ImageDataPointer ) {
		free( psc_ImageDataPointer );
	}

	// 状態遷移[→初期状態].
	pc_DataMng->setStatus( WAPE_GetThumbnailStatus_Initialize );

	return si32_Result;
}

/**
 * @brief		サムネイル画像レスポンスデータ生成.
 * @param[in]	psc_ImageDataPointer	: 設定する画像データポインタ.
 * @param[in]	ui32_TotalDataSize		: 設定する画像データサイズ.
 * @return		処理結果, WAPD_NoError(0)：成功、WAPD_AnyError(1)：失敗.
 * @author		日立 西原.
 * @since		2017/12/25.
 */
int32_t WAPC_GetThumbnail::makeImageResponce( char* psc_ImageDataPointer, uint32_t ui32_TotalDataSize, short ss_ImageDotSizeWidth, short ss_ImageDotSizeHight )
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
	char *psc_Base64 = static_cast<char *>( malloc( WAPD_Base64Size ) );
	if( NULL == psc_Base64 ) {
		WAPG_API_trace("[ERR]%s malloc() is NULL!.\n", __PRETTY_FUNCTION__ );
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
		// レスポンスデータ(システム異常)生成.
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		return WAPD_AnyError;
	}

	// 取得した画像データをBASE64文字列に変換.
	memset( psc_Base64, 0, WAPD_Base64Size );
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
	(*m_Response)[WAPD_BodyParam_ImageSize][WAPD_BodyParam_X] = static_cast<int32_t>(ss_ImageDotSizeWidth);
	(*m_Response)[WAPD_BodyParam_ImageSize][WAPD_BodyParam_Y] = static_cast<int32_t>(ss_ImageDotSizeHight);

	m_ResponseHeader->ContentType       = const_cast<char *>(WAPD_BodyParam_ContentType);
	m_ResponseHeader->ContentTypeLength = static_cast<uint32_t>(strlen(m_ResponseHeader->ContentType));

	m_ResponseHeader->Status = WAPD_STATUS_OK;

	free( psc_Base64 );		// BASE64変換領域を開放.
	return WAPD_NoError;
}


/**
 * @brief		サムネイル取得失敗要因データ生成.
 * @param[in]	e_ResultCode	: 画像取得結果(WAPE_GetThumbnailResultCode).
 * @return		なし.
 * @author		日立 西原.
 * @since		2017/12/25.
 */
void WAPC_GetThumbnail::makeErrorDetail( WAPE_GetThumbnailResultCode e_ResultCode )
{
	static const char*	psc_ErrorDetailStr[] =
	{
		"PROSESSING"			,		// [ 1]処理実施中のため取得不可.
		"INACCESSIBLE_STATE"	,		// [ 2]文書アクセス不可状態.
		"FAILURE"				,		// [ 3]サムネイル画像取得失敗.
		"MEMORY_OVER"			,		// [ 4]メモリーオーバーフロー.
		"NOTHING"				,		// [ 5]取得画像なし.
		"OUT_OF_SUPPORT"		,		// [ 6]サポート外.
		NULL
	};

	// 入力値範囲外の場合は補正(念のため).
	if( ( WAPE_GetThumbnailResultCode_Prosessing > e_ResultCode ) || ( WAPE_GetThumbnailResultCode_Out_Of_Support <e_ResultCode ) ) {
		e_ResultCode = WAPE_GetThumbnailResultCode_Out_Of_Support;
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

/**
 * @brief		ジョブログ(監査ログ)保存.
 * @param[out]	si32_DocID		：文書ID.
 * @param[out]	si32_BoxID		：ボックスID.
 * @param[out]	e_Result		：サムネイルデータアクセス結果.
 * @return 		なし.
 * @author		.
 * @since		2018/10/01.
 */
void WAPC_GetThumbnail::addOpeLogThumbnailImage( int32_t si32_DocID, int32_t si32_BoxID, TYPE_ThumbnailResult e_Result )
{
	JLGS_OpeLogFileAccess s_OpeLogFileAccess;
	JLGS_OpeLogCom s_OpeLogCom;
	TYPS_TokenAuthUserInfo	st_AuthUserInfo = {};
	char m_AccessToken[TYPD_AccessTokenSize + 1] ={};
	
	memset( &s_OpeLogCom, 0x00, sizeof( s_OpeLogCom ) );

	WAPC_GetThumbnail_DataMng*	pc_DataMng = WAPC_GetThumbnail_DataMng::newInstance();
	if( NULL == pc_DataMng ) {
		WAPG_API_trace("[ERR]%s WAPC_GetThumbnail_DataMng::newInstance() is NULL!.\n", __PRETTY_FUNCTION__ );
		return;
	}

	// アクセストークン.
	pc_DataMng->getAccessToken( m_AccessToken );

	// アクセス権情報設定.
	APIC_AccessTokenManager* pc_AccessTokenManager = APIC_AccessTokenManager::newInstance();
	JLGE_InspectLogIF e_InspectLogIF = JLGE_InspectLogIfOperationPanel;
	if( NULL == pc_AccessTokenManager ) {
		WAPG_API_trace("[ERR]%s APIC_AccessTokenManager::newInstance() is NULL!.\n", __PRETTY_FUNCTION__ );
		return;
	}
	pc_AccessTokenManager->getAppType( m_AccessToken, &e_InspectLogIF );

	// アクセストークン管理から権限を取得.
	Bool bl_UserInfo = pc_AccessTokenManager->getUserInfo( m_AccessToken, &st_AuthUserInfo );
	if( False == bl_UserInfo ){
		WAPD_DEBUG( "Panel token :getUserInfo Failed\n" );
	} else {
		s_OpeLogCom.ul_OpeCode = JLGE_InspectLogOperateIDL_Non;
		APIC_User* pc_User = APIC_User::newInstance( st_AuthUserInfo.m_stTokenUserInfo.m_i32UserNo );
		if( NULL != pc_User ){
			uint32_t ui32_UserNo = JLGE_InspectLogOperateIDL_Non;
			TYPE_UserType e_LoginUserType = pc_User->getUserType();
			// ユーザータイプを取得.
			switch( e_LoginUserType ) {
				case TYPE_UserTypeUser: ui32_UserNo = JLGE_InspectLogOperateIDL_User | ( int32_t )( pc_User->getUserNo() ); break;
				case TYPE_UserTypeCE: ui32_UserNo = JLGE_InspectLogOperateIDL_CE; break;
				case TYPE_UserTypeAdmin: ui32_UserNo = JLGE_InspectLogOperateIDL_Admin; break;
				case TYPE_UserTypeBoxAdmin: ui32_UserNo = JLGE_InspectLogOperateIDL_BoxAdmin; break;
				case TYPE_UserTypePublicUser: ui32_UserNo = JLGE_InspectLogOperateIDL_PublicUser; break;
				default: WAPG_API_trace("[ERR]%s getUserType() Error!.\n", __PRETTY_FUNCTION__ ); break;
			}
			s_OpeLogCom.ul_OpeCode		= ui32_UserNo;
			char sc_ExServerName[JLGD_DataLenExtSvName + 1] = {};
			pc_User->getLoginServerName( sc_ExServerName );
			strncpy( s_OpeLogCom.sc_ExtServerName, sc_ExServerName, JLGD_DataLenExtSvName );
		
			char sc_UserNameTmp[APID_UAC_MAX_UserName] = {};
			pc_User->getUserNameUTF8(sc_UserNameTmp, TYPE_CharSetUTF8);
			strncpy( s_OpeLogCom.sc_Name, sc_UserNameTmp, JLGD_DataLenUserName );
		}

		s_OpeLogCom.ul_TrackCode = JLGE_InspectLogOperateIDL_Non;
		APIC_Track* pc_Track = APIC_Track::newInstanceAdmin(st_AuthUserInfo.m_stTokenUserInfo.m_i32TrackNo);
		if( NULL != pc_Track ){
			s_OpeLogCom.ul_TrackCode = JLGE_InspectLogTrackIDL_Track | ( int32_t )( pc_Track->getTrackNo() );
			char	sc_TrackName[ UACD_MAX_TrackName ]	= {};
			pc_Track->getTrackName( sc_TrackName );
			strncpy( s_OpeLogCom.sc_TrackName, sc_TrackName, JLGD_DataLenTrackName );
		}
	}
	

	s_OpeLogCom.ul_Code = JLGE_InspectLogReadBoxDoc;
	s_OpeLogCom.us_Result	= (TYPE_ThumbnailResult_OK == e_Result) ? JLGE_InspectLogResultOK : JLGE_InspectLogResultNG;
	s_OpeLogCom.us_EventType = getEventTypeByResultCode( s_OpeLogCom.us_Result );
	
	s_OpeLogCom.uc_IF	= e_InspectLogIF;

	memset( &s_OpeLogFileAccess, 0x00, sizeof(s_OpeLogFileAccess) );
	s_OpeLogFileAccess.sl_DocNo		= si32_DocID;		//文書ID.
	s_OpeLogFileAccess.sl_BoxNo		= si32_BoxID;		//ボックスID.
	APIC_Doc::newInstance()->getDocName(si32_DocID, si32_BoxID, reinterpret_cast<char*>( s_OpeLogFileAccess.uc_DocName ), sizeof(s_OpeLogFileAccess.uc_DocName), LIBE_CharSetDefault);

	// ジョブログ保存処理.
	APIC_JLG* pc_JLG = APIC_JLG::newInstance();
	if( NULL != pc_JLG){
		pc_JLG->getNowTime( &s_OpeLogCom.s_Time );
		pc_JLG->addOpeLogFileAccess( &s_OpeLogCom, &s_OpeLogFileAccess );
	}
}



//-----------------------------------------------------------------------------.
// WAPC_GetThumbnail_DataMngクラス.
//	GetThumbnail個別コントローラとコールバック処理間で共有するデータを格納する.
//-----------------------------------------------------------------------------.
/**
 * @brief		コンストラクタ.
 * @return		なし.
 * @author		日立 西原.
 * @since		2017/12/25.
 */
WAPC_GetThumbnail_DataMng::WAPC_GetThumbnail_DataMng()
{
	// リクエスト処理の排他用セマフォ生成.
	m_ProcSemID = SYSF_semMCreate( SEM_Q_PRIORITY | SEM_DELETE_SAFE | SEM_INVERSION_SAFE );
	if ( NULL == m_ProcSemID ) {
		WAPG_API_trace("[ERR]%s SYSF_semMCreate() Error!.\n", __PRETTY_FUNCTION__ );
		// Fatal通知.
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
	}

	// 管理情報クリア.
	m_e_Status = WAPE_GetThumbnailStatus_Initialize;
	m_ui32_AcceptNum = 0;
	m_e_ResultCode = WAPE_GetThumbnailResultCode_OK;
	memset( &m_s_ImageParam, 0, sizeof(m_s_ImageParam) );
}

/**
 * @brief		デストラクタ.
 * @return		なし.
 * @author		日立 西原.
 * @since		2017/12/25.
 */
WAPC_GetThumbnail_DataMng::~WAPC_GetThumbnail_DataMng()
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
 * @since		2017/12/25.
 */
WAPC_GetThumbnail_DataMng* WAPC_GetThumbnail_DataMng::newInstance()
{
	static WAPC_GetThumbnail_DataMng c_WAPC_GetThumbnail_DataMng;
	return &c_WAPC_GetThumbnail_DataMng;
}

/**
 * @brief		リクエスト処理の排他用セマフォ取得.
 * @param		なし.
 * @return 		なし.
 * @author		日立 西原.
 * @since		2017/12/25.
 */
void WAPC_GetThumbnail_DataMng::takeProcSem( void )
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
 * @since		2017/12/25.
 */
void WAPC_GetThumbnail_DataMng::giveProcSem( void )
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
 * @param[in]	eStatus	: 処理状態(WAPE_GetThumbnailStatus).
 * @return 		なし.
 * @author		日立 西原.
 * @since		2017/12/25.
 */
void WAPC_GetThumbnail_DataMng::setStatus( WAPE_GetThumbnailStatus e_Status )
{
	m_e_Status = e_Status;
	return;
}

/**
 * @brief		リクエスト受付状態の参照.
 * @param[in]	なし.
 * @return 		処理状態(WAPE_GetThumbnailStatus).
 * @author		日立 西原.
 * @since		2017/12/25.
 */
WAPE_GetThumbnailStatus WAPC_GetThumbnail_DataMng::getStatus( void )
{
	return m_e_Status;
}

/**
 * @brief		画像取得要求受付番号の設定.
 * @param[in]	ui32_AcceptNum	: 受付番号(1～0xffffffff).
 * @return 		なし.
 * @author		日立 西原.
 * @since		2017/12/25.
 */
void WAPC_GetThumbnail_DataMng::setAcceptNum( uint32_t ui32_AcceptNum )
{
	m_ui32_AcceptNum = ui32_AcceptNum;
	return;
}

/**
 * @brief		画像取得要求受付番号の参照.
 * @param		なし.
 * @return 		受付番号(1～0xffffffff).
 * @author		日立 西原.
 * @since		2017/12/25.
 */
uint32_t WAPC_GetThumbnail_DataMng::getAcceptNum( void )
{
	return m_ui32_AcceptNum;
}

/**
 * @brief		画像取得結果の設定.
 * @param		si_ResultCode	: 画像取得結果(WAPE_GetThumbnailResultCode).
 * @return 		なし.
 * @author		日立 西原.
 * @since		2017/12/25.
 */
void WAPC_GetThumbnail_DataMng::setConverterResult( WAPE_GetThumbnailResultCode e_ResultCode )
{
	m_e_ResultCode = e_ResultCode;
	return;
}

/**
 * @brief		画像取得結果の参照.
 * @param		なし.
 * @return 		画像取得結果(WAPE_GetThumbnailResultCode).
 * @author		日立 西原.
 * @since		2017/12/25.
 */
WAPE_GetThumbnailResultCode WAPC_GetThumbnail_DataMng::getConverterResult( void )
{
	return m_e_ResultCode;
}

/**
 * @brief		出力画像パラメータの設定.
 * @param[in]	psc_ImageDataPointer	: 設定する画像データポインタ.
 * @param[in]	ui32_TotalDataSize		: 設定する画像データサイズ.
 * @return 		なし.
 * @author		日立 西原.
 * @since		2017/12/25.
 */
void WAPC_GetThumbnail_DataMng::setImageParam( char* psc_ImageDataPointer, uint32_t ui32_TotalDataSize )
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
 * @since		2017/12/25.
 */
void WAPC_GetThumbnail_DataMng::setTotalDataSize( uint32_t ui32_TotalDataSize )
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
 * @since		2017/12/25.
 */
void WAPC_GetThumbnail_DataMng::getImageParam( char** psc_ImageDataPointer, uint32_t* pui32_TotalDataSize )
{
	*psc_ImageDataPointer	= m_s_ImageParam.psc_ImageDataPointer;
	*pui32_TotalDataSize	= m_s_ImageParam.ui32_TotalDataSize;
	return;
}

/**
 * @brief		出力画像サイズの設定.
 * @param[in]	ss_ImageDotSizeWidth	: 設定する画像サイズ（横幅）.
 * @param[in]	ss_ImageDotSizeHight	: 設定する画像サイズ（縦幅）.
 * @return 		なし.
 * @author		ADS 藤田.
 * @since		2018/07/23.
 */
void WAPC_GetThumbnail_DataMng::setImageSize( short ss_ImageDotSizeWidth, short ss_ImageDotSizeHight )
{
	m_s_ImageParam.ss_ImageDotSizeWidth = ss_ImageDotSizeWidth;
	m_s_ImageParam.ss_ImageDotSizeHight = ss_ImageDotSizeHight;
	return;
}

/**
 * @brief		出力画像サイズの参照.
 * @param[in]	pss_ImageDotSizeWidth	: 設定した画像サイズ（横幅）.
 * @param[in]	pss_ImageDotSizeHight	: 設定した画像サイズ（縦幅）.
 * @return 		なし.
 * @author		ADS 藤田.
 * @since		2018/07/23.
 */
void WAPC_GetThumbnail_DataMng::getImageSize( short* pss_ImageDotSizeWidth, short* pss_ImageDotSizeHight )
{
	*pss_ImageDotSizeWidth	= m_s_ImageParam.ss_ImageDotSizeWidth;
	*pss_ImageDotSizeHight	= m_s_ImageParam.ss_ImageDotSizeHight;
	return;
}

/**
 * @brief		文書・ボックスIDの設定.
 * @param[in]	si32_DocID		：文書ID.
 * @param[in]	si32_BoxID		：ボックスID.
 * @return 		なし.
 * @author		.
 * @since		2018/10/01.
 */
void WAPC_GetThumbnail_DataMng::setDocBoxID( int32_t si32_DocID, int32_t si32_BoxID )
{
	m_s_DocBoxID.si32_DocID = si32_DocID;
	m_s_DocBoxID.si32_BoxID = si32_BoxID;
	return;
}

/**
 * @brief		文書・ボックスIDの参照.
 * @param[in]	si32_DocID		：文書ID.
 * @param[in]	si32_BoxID		：ボックスID.
 * @return 		なし.
 * @author		.
 * @since		2018/10/01.
 */
void WAPC_GetThumbnail_DataMng::getDocBoxID( int32_t* psi32_DocID, int32_t* psi32_BoxIDt )
{
	*psi32_DocID	= m_s_DocBoxID.si32_DocID;
	*psi32_BoxIDt	= m_s_DocBoxID.si32_BoxID;
	return;
}

/**
 * @brief		アクセストークンの設定.
 * @param[in]	psc_AccessToken		：アクセストークン.
 * @return 		なし.
 * @author		.
 * @since		2018/10/01.
 */
void WAPC_GetThumbnail_DataMng::setAccessToken( char* psc_AccessToken )
{
	if( NULL == psc_AccessToken ) {	WAPG_API_trace("[ERR]%s setAccessToken psc_AccessToken is NULL!.\n", __PRETTY_FUNCTION__ );	return;	}
	strncpy( m_AccessToken, psc_AccessToken, TYPD_AccessTokenSize );
	return;
}

/**
 * @brief		アクセストークンの参照.
 * @param[out]	psc_AccessToken		：アクセストークン.
 * @return 		なし.
 * @author		.
 * @since		2018/10/01.
 */
void WAPC_GetThumbnail_DataMng::getAccessToken( char* psc_AccessToken )
{
	if( NULL == psc_AccessToken ) {	WAPG_API_trace("[ERR]%s getAccessToken psc_AccessToken is NULL!.\n", __PRETTY_FUNCTION__ );	return;	}
	strncpy( psc_AccessToken, m_AccessToken, TYPD_AccessTokenSize );
	return;
}
