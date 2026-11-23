#include <iostream>
#include "WAP_MainController.h"
#include "WAP_plugintrace.h"
#include "WAP_UserBoxDocument.h"
#include "WAP_AsyncResponder.h"
#include "WAP_CommonTables.h"
#include "WAP_Power.h"

// 処理種別.
enum WAPE_FuncKind {
	WAPE_FuncKindNone = 0,
	WAPE_FuncKindDocumentsOperation,
	WAPE_FuncKindGetDocumentList,
	WAPE_FuncKindGetDocumentInformation,
	WAPE_FuncKindSetDocumentInformation,
	WAPE_FuncKindDeleteDocument
};

// 処理変換用テーブル.
static const struct {
	int32_t			si32_BoxReq;	// WebAPI種別.
	uint32_t		ui32_Method;	// メソッド.
	int32_t			si32_Funckind;	// 処理タイプ.
} g_s_ConvertTable[] = {
	{	WAPE_BoxReqUriBoxesNoDocNo,		WAPD_HTTP_PUT,		WAPE_FuncKindDocumentsOperation		},
	{	WAPE_BoxReqUriBoxesNoDoc,		WAPD_HTTP_GET,		WAPE_FuncKindGetDocumentList		},
	{	WAPE_BoxReqUriBoxesNoDocNoInfo,	WAPD_HTTP_GET,		WAPE_FuncKindGetDocumentInformation	},
	{	WAPE_BoxReqUriBoxesNoDocNoInfo,	WAPD_HTTP_PUT,		WAPE_FuncKindSetDocumentInformation	},
	{	WAPE_BoxReqUriBoxesNoDocNo,		WAPD_HTTP_DELETE,	WAPE_FuncKindDeleteDocument			},
};

// ドキュメント操作コールバック関数（nginx最大接続数：10）.
static void WAPC_UserBoxDocumentController_ans_changeDocFilePosition01( int32_t si32_Result, void* pv_ResultBuffer ){ WAPC_UserBoxDocumentController::documentOperationCompleted( si32_Result, pv_ResultBuffer, reinterpret_cast<uint64_t>(WAPC_UserBoxDocumentController_ans_changeDocFilePosition01) ); }
static void WAPC_UserBoxDocumentController_ans_changeDocFilePosition02( int32_t si32_Result, void* pv_ResultBuffer ){ WAPC_UserBoxDocumentController::documentOperationCompleted( si32_Result, pv_ResultBuffer, reinterpret_cast<uint64_t>(WAPC_UserBoxDocumentController_ans_changeDocFilePosition02) ); }
static void WAPC_UserBoxDocumentController_ans_changeDocFilePosition03( int32_t si32_Result, void* pv_ResultBuffer ){ WAPC_UserBoxDocumentController::documentOperationCompleted( si32_Result, pv_ResultBuffer, reinterpret_cast<uint64_t>(WAPC_UserBoxDocumentController_ans_changeDocFilePosition03) ); }
static void WAPC_UserBoxDocumentController_ans_changeDocFilePosition04( int32_t si32_Result, void* pv_ResultBuffer ){ WAPC_UserBoxDocumentController::documentOperationCompleted( si32_Result, pv_ResultBuffer, reinterpret_cast<uint64_t>(WAPC_UserBoxDocumentController_ans_changeDocFilePosition04) ); }
static void WAPC_UserBoxDocumentController_ans_changeDocFilePosition05( int32_t si32_Result, void* pv_ResultBuffer ){ WAPC_UserBoxDocumentController::documentOperationCompleted( si32_Result, pv_ResultBuffer, reinterpret_cast<uint64_t>(WAPC_UserBoxDocumentController_ans_changeDocFilePosition05) ); }
static void WAPC_UserBoxDocumentController_ans_changeDocFilePosition06( int32_t si32_Result, void* pv_ResultBuffer ){ WAPC_UserBoxDocumentController::documentOperationCompleted( si32_Result, pv_ResultBuffer, reinterpret_cast<uint64_t>(WAPC_UserBoxDocumentController_ans_changeDocFilePosition06) ); }
static void WAPC_UserBoxDocumentController_ans_changeDocFilePosition07( int32_t si32_Result, void* pv_ResultBuffer ){ WAPC_UserBoxDocumentController::documentOperationCompleted( si32_Result, pv_ResultBuffer, reinterpret_cast<uint64_t>(WAPC_UserBoxDocumentController_ans_changeDocFilePosition07) ); }
static void WAPC_UserBoxDocumentController_ans_changeDocFilePosition08( int32_t si32_Result, void* pv_ResultBuffer ){ WAPC_UserBoxDocumentController::documentOperationCompleted( si32_Result, pv_ResultBuffer, reinterpret_cast<uint64_t>(WAPC_UserBoxDocumentController_ans_changeDocFilePosition08) ); }
static void WAPC_UserBoxDocumentController_ans_changeDocFilePosition09( int32_t si32_Result, void* pv_ResultBuffer ){ WAPC_UserBoxDocumentController::documentOperationCompleted( si32_Result, pv_ResultBuffer, reinterpret_cast<uint64_t>(WAPC_UserBoxDocumentController_ans_changeDocFilePosition09) ); }
static void WAPC_UserBoxDocumentController_ans_changeDocFilePosition10( int32_t si32_Result, void* pv_ResultBuffer ){ WAPC_UserBoxDocumentController::documentOperationCompleted( si32_Result, pv_ResultBuffer, reinterpret_cast<uint64_t>(WAPC_UserBoxDocumentController_ans_changeDocFilePosition10) ); }

// ドキュメント操作コールバック関数テーブル.
static struct {
	FP_ANS_CHANGE_DOCFILE_POSITON fp_Cb;
	Bool b_InUse;
} g_s_DocOperationCbTable[] = {
	{	WAPC_UserBoxDocumentController_ans_changeDocFilePosition01,	False	},
	{	WAPC_UserBoxDocumentController_ans_changeDocFilePosition02,	False	},
	{	WAPC_UserBoxDocumentController_ans_changeDocFilePosition03,	False	},
	{	WAPC_UserBoxDocumentController_ans_changeDocFilePosition04,	False	},
	{	WAPC_UserBoxDocumentController_ans_changeDocFilePosition05,	False	},
	{	WAPC_UserBoxDocumentController_ans_changeDocFilePosition06,	False	},
	{	WAPC_UserBoxDocumentController_ans_changeDocFilePosition07,	False	},
	{	WAPC_UserBoxDocumentController_ans_changeDocFilePosition08,	False	},
	{	WAPC_UserBoxDocumentController_ans_changeDocFilePosition09,	False	},
	{	WAPC_UserBoxDocumentController_ans_changeDocFilePosition10,	False	},
};

// ドキュメントリスト取得コールバック関数（nginx最大接続数：10）.
static void WAPC_UserBoxDocumentController_ans_getDocList02( APIC_DocListInfo* pc_DocListInfo, void* pv_Param ){ WAPC_UserBoxDocumentController::getDocumentListCompleted( pc_DocListInfo, pv_Param, reinterpret_cast<uint64_t>(WAPC_UserBoxDocumentController_ans_getDocList02) ); }
static void WAPC_UserBoxDocumentController_ans_getDocList01( APIC_DocListInfo* pc_DocListInfo, void* pv_Param ){ WAPC_UserBoxDocumentController::getDocumentListCompleted( pc_DocListInfo, pv_Param, reinterpret_cast<uint64_t>(WAPC_UserBoxDocumentController_ans_getDocList01) ); }
static void WAPC_UserBoxDocumentController_ans_getDocList03( APIC_DocListInfo* pc_DocListInfo, void* pv_Param ){ WAPC_UserBoxDocumentController::getDocumentListCompleted( pc_DocListInfo, pv_Param, reinterpret_cast<uint64_t>(WAPC_UserBoxDocumentController_ans_getDocList03) ); }
static void WAPC_UserBoxDocumentController_ans_getDocList04( APIC_DocListInfo* pc_DocListInfo, void* pv_Param ){ WAPC_UserBoxDocumentController::getDocumentListCompleted( pc_DocListInfo, pv_Param, reinterpret_cast<uint64_t>(WAPC_UserBoxDocumentController_ans_getDocList04) ); }
static void WAPC_UserBoxDocumentController_ans_getDocList05( APIC_DocListInfo* pc_DocListInfo, void* pv_Param ){ WAPC_UserBoxDocumentController::getDocumentListCompleted( pc_DocListInfo, pv_Param, reinterpret_cast<uint64_t>(WAPC_UserBoxDocumentController_ans_getDocList05) ); }
static void WAPC_UserBoxDocumentController_ans_getDocList06( APIC_DocListInfo* pc_DocListInfo, void* pv_Param ){ WAPC_UserBoxDocumentController::getDocumentListCompleted( pc_DocListInfo, pv_Param, reinterpret_cast<uint64_t>(WAPC_UserBoxDocumentController_ans_getDocList06) ); }
static void WAPC_UserBoxDocumentController_ans_getDocList07( APIC_DocListInfo* pc_DocListInfo, void* pv_Param ){ WAPC_UserBoxDocumentController::getDocumentListCompleted( pc_DocListInfo, pv_Param, reinterpret_cast<uint64_t>(WAPC_UserBoxDocumentController_ans_getDocList07) ); }
static void WAPC_UserBoxDocumentController_ans_getDocList08( APIC_DocListInfo* pc_DocListInfo, void* pv_Param ){ WAPC_UserBoxDocumentController::getDocumentListCompleted( pc_DocListInfo, pv_Param, reinterpret_cast<uint64_t>(WAPC_UserBoxDocumentController_ans_getDocList08) ); }
static void WAPC_UserBoxDocumentController_ans_getDocList09( APIC_DocListInfo* pc_DocListInfo, void* pv_Param ){ WAPC_UserBoxDocumentController::getDocumentListCompleted( pc_DocListInfo, pv_Param, reinterpret_cast<uint64_t>(WAPC_UserBoxDocumentController_ans_getDocList09) ); }
static void WAPC_UserBoxDocumentController_ans_getDocList10( APIC_DocListInfo* pc_DocListInfo, void* pv_Param ){ WAPC_UserBoxDocumentController::getDocumentListCompleted( pc_DocListInfo, pv_Param, reinterpret_cast<uint64_t>(WAPC_UserBoxDocumentController_ans_getDocList10) ); }

// ドキュメントリスト取得コールバック関数テーブル.
static struct {
	FP_ANS_GET_DOCLIST fp_Cb;
	Bool b_InUse;
} g_s_GetDocListCbTable[] = {
	{	WAPC_UserBoxDocumentController_ans_getDocList01,	False	},
	{	WAPC_UserBoxDocumentController_ans_getDocList02,	False	},
	{	WAPC_UserBoxDocumentController_ans_getDocList03,	False	},
	{	WAPC_UserBoxDocumentController_ans_getDocList04,	False	},
	{	WAPC_UserBoxDocumentController_ans_getDocList05,	False	},
	{	WAPC_UserBoxDocumentController_ans_getDocList06,	False	},
	{	WAPC_UserBoxDocumentController_ans_getDocList07,	False	},
	{	WAPC_UserBoxDocumentController_ans_getDocList08,	False	},
	{	WAPC_UserBoxDocumentController_ans_getDocList09,	False	},
	{	WAPC_UserBoxDocumentController_ans_getDocList10,	False	},
};

// ドキュメント削除コールバック関数（nginx最大接続数：10）.
static void WAPC_UserBoxDocumentController_ans_deleteDoc01( int32_t si32_BoxId, int32_t si32_DocId, int32_t si32_Rtn ){ WAPC_UserBoxDocumentController::deleteDocumentCompleted( si32_BoxId, si32_DocId, si32_Rtn, reinterpret_cast<uint64_t>(WAPC_UserBoxDocumentController_ans_deleteDoc01) ); }
static void WAPC_UserBoxDocumentController_ans_deleteDoc02( int32_t si32_BoxId, int32_t si32_DocId, int32_t si32_Rtn ){ WAPC_UserBoxDocumentController::deleteDocumentCompleted( si32_BoxId, si32_DocId, si32_Rtn, reinterpret_cast<uint64_t>(WAPC_UserBoxDocumentController_ans_deleteDoc02) ); }
static void WAPC_UserBoxDocumentController_ans_deleteDoc03( int32_t si32_BoxId, int32_t si32_DocId, int32_t si32_Rtn ){ WAPC_UserBoxDocumentController::deleteDocumentCompleted( si32_BoxId, si32_DocId, si32_Rtn, reinterpret_cast<uint64_t>(WAPC_UserBoxDocumentController_ans_deleteDoc03) ); }
static void WAPC_UserBoxDocumentController_ans_deleteDoc04( int32_t si32_BoxId, int32_t si32_DocId, int32_t si32_Rtn ){ WAPC_UserBoxDocumentController::deleteDocumentCompleted( si32_BoxId, si32_DocId, si32_Rtn, reinterpret_cast<uint64_t>(WAPC_UserBoxDocumentController_ans_deleteDoc04) ); }
static void WAPC_UserBoxDocumentController_ans_deleteDoc05( int32_t si32_BoxId, int32_t si32_DocId, int32_t si32_Rtn ){ WAPC_UserBoxDocumentController::deleteDocumentCompleted( si32_BoxId, si32_DocId, si32_Rtn, reinterpret_cast<uint64_t>(WAPC_UserBoxDocumentController_ans_deleteDoc05) ); }
static void WAPC_UserBoxDocumentController_ans_deleteDoc06( int32_t si32_BoxId, int32_t si32_DocId, int32_t si32_Rtn ){ WAPC_UserBoxDocumentController::deleteDocumentCompleted( si32_BoxId, si32_DocId, si32_Rtn, reinterpret_cast<uint64_t>(WAPC_UserBoxDocumentController_ans_deleteDoc06) ); }
static void WAPC_UserBoxDocumentController_ans_deleteDoc07( int32_t si32_BoxId, int32_t si32_DocId, int32_t si32_Rtn ){ WAPC_UserBoxDocumentController::deleteDocumentCompleted( si32_BoxId, si32_DocId, si32_Rtn, reinterpret_cast<uint64_t>(WAPC_UserBoxDocumentController_ans_deleteDoc07) ); }
static void WAPC_UserBoxDocumentController_ans_deleteDoc08( int32_t si32_BoxId, int32_t si32_DocId, int32_t si32_Rtn ){ WAPC_UserBoxDocumentController::deleteDocumentCompleted( si32_BoxId, si32_DocId, si32_Rtn, reinterpret_cast<uint64_t>(WAPC_UserBoxDocumentController_ans_deleteDoc08) ); }
static void WAPC_UserBoxDocumentController_ans_deleteDoc09( int32_t si32_BoxId, int32_t si32_DocId, int32_t si32_Rtn ){ WAPC_UserBoxDocumentController::deleteDocumentCompleted( si32_BoxId, si32_DocId, si32_Rtn, reinterpret_cast<uint64_t>(WAPC_UserBoxDocumentController_ans_deleteDoc09) ); }
static void WAPC_UserBoxDocumentController_ans_deleteDoc10( int32_t si32_BoxId, int32_t si32_DocId, int32_t si32_Rtn ){ WAPC_UserBoxDocumentController::deleteDocumentCompleted( si32_BoxId, si32_DocId, si32_Rtn, reinterpret_cast<uint64_t>(WAPC_UserBoxDocumentController_ans_deleteDoc10) ); }

// ドキュメント削除コールバック関数テーブル.
static struct {
	FP_ANS_DELETE_DOC fp_Cb;
	Bool b_InUse;
} g_s_DeleteDocCbTable[] = {
	{	WAPC_UserBoxDocumentController_ans_deleteDoc01,	False	},
	{	WAPC_UserBoxDocumentController_ans_deleteDoc02,	False	},
	{	WAPC_UserBoxDocumentController_ans_deleteDoc03,	False	},
	{	WAPC_UserBoxDocumentController_ans_deleteDoc04,	False	},
	{	WAPC_UserBoxDocumentController_ans_deleteDoc05,	False	},
	{	WAPC_UserBoxDocumentController_ans_deleteDoc06,	False	},
	{	WAPC_UserBoxDocumentController_ans_deleteDoc07,	False	},
	{	WAPC_UserBoxDocumentController_ans_deleteDoc08,	False	},
	{	WAPC_UserBoxDocumentController_ans_deleteDoc09,	False	},
	{	WAPC_UserBoxDocumentController_ans_deleteDoc10,	False	},
};

static const int32_t WAPD_UserBoxDocumentListOffsetDefault	= 1;						// ユーザーボックスドキュメントリストデフォルト取得開始位置.
static const int32_t WAPD_UserBoxDocumentListOffsetMin		= 1;						// ユーザーボックスドキュメントリスト最小取得開始位置.
static const int32_t WAPD_UserBoxDocumentListOffsetMax		= DOCD_DocFileCntBoxMax;	// ユーザーボックスドキュメントリスト最大取得開始位置.
static const int32_t WAPD_UserBoxDocumentListOffsetMax_1T   = DOCD_DocFileCntBoxMax_1T;  //2019-08-14【EagleH】 RQ-560 対応 Hcl-Rajeshwari start.
static const int32_t WAPD_UserBoxDocumentListNumDefault		= 10;						// ユーザーボックスドキュメントリストデフォルト取得数.
static const int32_t WAPD_UserBoxDocumentListNumMin			= 1;						// ユーザーボックスドキュメントリスト最小取得数.
static const int32_t WAPD_UserBoxDocumentListNumMax			= DOCD_DocFileCntBoxMax;	// ユーザーボックスドキュメントリスト最大取得数.
static const int32_t WAPD_UserBoxDocumentListNumMax_1T      = DOCD_DocFileCntBoxMax_1T;  //2019-08-14【EagleH】 RQ-560 対応 Hcl-Rajeshwari start.
static const int32_t WAPD_UserBoxDocumentListNumAll			= 0xFFFF;					// ユーザーボックスドキュメントリスト全取得.
static const int32_t WAPD_DocumentNameMinChar				= 1;						// ドキュメント名最小文字数.
static const int32_t WAPD_DocumentNameMaxChar				= 30;						// ドキュメント名最大文字数.
static const int32_t WAPD_DocumentNameMaxLength				= (WAPD_DocumentNameMaxChar * WAPD_Utf8Size);  //ドキュメント名最大バイト数

/**
 * @brief コンストラクタ
 * @return なし
 */
WAPC_UserBoxDocumentController::WAPC_UserBoxDocumentController()
{
}

/**
 * @brief コンストラクタ
 * @return なし
 */
WAPC_UserBoxDocumentController::WAPC_UserBoxDocumentController( int32_t si32_BoxReq, int32_t si32_BoxNo, int32_t si32_DocNo )
{
	m_si32_BoxReq = si32_BoxReq;
	m_si32_BoxNo  = si32_BoxNo;
	m_si32_DocNo  = si32_DocNo;
}

/**
 * @brief デストラクタ
 * @return なし
 */
WAPC_UserBoxDocumentController::~WAPC_UserBoxDocumentController()
{
}

/**
 * @brief HTTPレスポンスの生成
 * @param[in] params HTTPリクエストヘッダ、ボディ
 * @retval WAPD_NoError(0) 成功.
 * @retval WAPD_AnyError(1) 失敗.
 */
int32_t WAPC_UserBoxDocumentController::ResponseGenerate( WAPS_SubController *params )
{
	int32_t si32_Ret = WAPD_AnyError;
	int32_t si32_Funckind = WAPE_FuncKindNone;
	Bool b_SendBodyOnHttp20X = False;

	// パラメータチェック.
	if( NULL == params ) {
		WAPG_API_trace("%s(%d) %s : params is NULL\n", __FILE__, __LINE__, __FUNCTION__);
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		generateErrorResponseBody( WAPE_BoxError_None );
		si32_Ret = WAPD_AnyError;
		goto LABEL_END;
	}

	// 処理解析.
	for( uint32_t ui32_Index = 0; ui32_Index < (sizeof(g_s_ConvertTable) / sizeof(g_s_ConvertTable[0])); ++ui32_Index ) {
		if( g_s_ConvertTable[ui32_Index].si32_BoxReq == m_si32_BoxReq ) {
			if( g_s_ConvertTable[ui32_Index].ui32_Method == params->Method ) {
				si32_Funckind = g_s_ConvertTable[ui32_Index].si32_Funckind;
				break;
			}
		}
	}

	// 機能制限確認.
	if ( WAPE_FuncKindNone != si32_Funckind ) {
		TYPS_FunctionLimitation s_FunctionLimitation;
		getFunctionLimitation( params, s_FunctionLimitation );

		// ボックス操作が禁止の場合は403を返す.
		if ( TYPE_FuncLimit_Disable == s_FunctionLimitation.e_EnableBoxOperation ) {
			WAPD_DEBUG1("WAPC_UserBoxController::%s Forbidden Disable\n", __FUNCTION__ );
			WAPG_API_trace("%s(%d) %s : Forbidden Disable.\n", __FILE__, __LINE__, __FUNCTION__);
			printf("403:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //403->ErrorStatus[WAPD_STATUS_FORBIDDEN] in WAP_defs.h
			m_ResponseHeader->Status = WAPD_STATUS_FORBIDDEN;
			generateErrorResponseBody( WAPE_BoxError_ForbiddenFunctionLimitation );
			si32_Ret = WAPD_AnyError;
			goto LABEL_END;
		}
	}

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

	switch( si32_Funckind ) {
		// ドキュメント操作(非同期).
		case WAPE_FuncKindDocumentsOperation:
			WAPG_API_trace("%s(%d) %s : Documents operation in user BOX start.\n", __FILE__, __LINE__, __FUNCTION__);
			si32_Ret = documentOperation( params );
			b_SendBodyOnHttp20X = False;
			break;

		// ドキュメントリスト取得(非同期).
		case WAPE_FuncKindGetDocumentList:
			WAPG_API_trace("%s(%d) %s : Get document list in user BOX start.\n", __FILE__, __LINE__, __FUNCTION__);
			si32_Ret = getDocumentList( params );
			b_SendBodyOnHttp20X = False;
			break;

		// ドキュメント情報取得(非同期).
		case WAPE_FuncKindGetDocumentInformation:
			WAPG_API_trace("%s(%d) %s : Get document information start.\n", __FILE__, __LINE__, __FUNCTION__);
			si32_Ret = getDocumentInformation( params );
			b_SendBodyOnHttp20X = False;
			break;

		// ドキュメント情報設定.
		case WAPE_FuncKindSetDocumentInformation:
			WAPG_API_trace("%s(%d) %s : Set document information start.\n", __FILE__, __LINE__, __FUNCTION__);
			si32_Ret = setDocumentInformation( params );
			b_SendBodyOnHttp20X = False;
			break;

		// ドキュメント削除(削除).
		case WAPE_FuncKindDeleteDocument:
			WAPG_API_trace("%s(%d) %s : Delete document in user BOX start.\n", __FILE__, __LINE__, __FUNCTION__);
			si32_Ret = deleteDocument( params );
			b_SendBodyOnHttp20X = False;
			break;

		default:
			WAPG_API_trace("%s(%d) %s : Invalid Method.\n", __FILE__, __LINE__, __FUNCTION__);
			printf("405:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //405->ErrorStatus[WAPD_STATUS_METHOD_NOT_ALLOWED] in WAP_defs.h
			m_ResponseHeader->Status = WAPD_STATUS_METHOD_NOT_ALLOWED;
			generateErrorResponseBody( WAPE_BoxError_InvalidMethod );
			si32_Ret = WAPD_AnyError;
			break;
	}

	// HDDデータアクセス用電源管理の省電力抑止解除.
	si32_ret = pc_Power->EndAccessHDD();
	if( WAPD_AnyError == si32_ret ){
		WAPG_API_trace("%s(%d) WAPC_Power::EndAccessHDD Error\n", __FILE__, __LINE__);
	}

LABEL_END:
	switch( m_ResponseHeader->Status ) {
		// 200系.
		case WAPD_STATUS_OK:
			if( True == b_SendBodyOnHttp20X ) {
				m_ResponseHeader->ContentType = const_cast<char*>("application/json");
				m_ResponseHeader->ContentTypeLength = sizeof("application/json") - 1;
			}
			break;

		// 400系(ユーザー要因).
		case WAPD_STATUS_BAD_REQUEST:
		case WAPD_STATUS_FORBIDDEN:
		case WAPD_STATUS_NOT_FOUND:
		case WAPD_STATUS_METHOD_NOT_ALLOWED:
			// エラー詳細を返すためContentType付加.
			m_ResponseHeader->ContentType = const_cast<char*>("application/json");
			m_ResponseHeader->ContentTypeLength = sizeof("application/json") - 1;
			break;

		// 500系(サーバー要因).
		default:
			// エラー詳細を返さないためContentTypeなし.
			break;
	}

	return si32_Ret;
}

/**
 * @brief 非同期レスポンスの生成
 * @param[in] params HTTPリクエストヘッダ、ボディ
 * @retval WAPD_NoError(0) 成功.
 * @retval WAPD_AnyError(1) 失敗.
 */
int32_t WAPC_UserBoxDocumentController::AsyncResponseGenerate( WAPS_SubController *params )
{
	int32_t si32_Ret = WAPD_AnyError;
	int32_t si32_Funckind = WAPE_FuncKindNone;
	Bool b_SendBodyOnHttp20X = False;

	// パラメータチェック.
	if( NULL == params ) {
		WAPG_API_trace("%s(%d) %s : params is NULL.\n", __FILE__, __LINE__, __FUNCTION__);
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		generateErrorResponseBody( WAPE_BoxError_None );
		si32_Ret = WAPD_AnyError;
		goto LABEL_END;
	}

	// 処理解析.
	for( uint32_t ui32_Index = 0; ui32_Index < (sizeof(g_s_ConvertTable) / sizeof(g_s_ConvertTable[0])); ++ui32_Index ) {
		if( g_s_ConvertTable[ui32_Index].si32_BoxReq == m_si32_BoxReq ) {
			if( g_s_ConvertTable[ui32_Index].ui32_Method == params->Method ) {
				si32_Funckind = g_s_ConvertTable[ui32_Index].si32_Funckind;
				break;
			}
		}
	}
	switch( si32_Funckind ) {
		// ドキュメント操作.
		case WAPE_FuncKindDocumentsOperation:
			WAPG_API_trace("%s(%d) %s : generateDocumentOperationResponse start.\n", __FILE__, __LINE__, __FUNCTION__);
			si32_Ret = generateDocumentOperationResponse( params );
			b_SendBodyOnHttp20X = True;
			break;

		// ドキュメントリスト取得.
		case WAPE_FuncKindGetDocumentList:
			WAPG_API_trace("%s(%d) %s : generateGetDocumentListResponse start.\n", __FILE__, __LINE__, __FUNCTION__);
			si32_Ret = generateGetDocumentListResponse( params );
			b_SendBodyOnHttp20X = True;
			break;

		// ドキュメント情報取得.
		case WAPE_FuncKindGetDocumentInformation:
			WAPG_API_trace("%s(%d) %s : generateGetDocumentInformationResponse start.\n", __FILE__, __LINE__, __FUNCTION__);
			si32_Ret = generateGetDocumentInformationResponse( params );
			b_SendBodyOnHttp20X = True;
			break;

		// ドキュメント削除.
		case WAPE_FuncKindDeleteDocument:
			WAPG_API_trace("%s(%d) %s : generateDeleteDocumentResponse start.\n", __FILE__, __LINE__, __FUNCTION__);
			si32_Ret = generateDeleteDocumentResponse( params );
			b_SendBodyOnHttp20X = False;
			break;

		default:
			WAPG_API_trace("%s(%d) %s : Invalid Method.\n", __FILE__, __LINE__, __FUNCTION__);
			printf("405:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //405->ErrorStatus[WAPD_STATUS_METHOD_NOT_ALLOWED] in WAP_defs.h
			m_ResponseHeader->Status = WAPD_STATUS_METHOD_NOT_ALLOWED;
			generateErrorResponseBody( WAPE_BoxError_InvalidMethod );
			si32_Ret = WAPD_AnyError;
			b_SendBodyOnHttp20X = False;
			break;
	}

LABEL_END:
	switch( m_ResponseHeader->Status ) {
		// 200系.
		case WAPD_STATUS_OK:
			if( True == b_SendBodyOnHttp20X ) {
				m_ResponseHeader->ContentType = const_cast<char*>("application/json");
				m_ResponseHeader->ContentTypeLength = sizeof("application/json") - 1;
			}
			break;

		// 400系(ユーザー要因).
		case WAPD_STATUS_BAD_REQUEST:
		case WAPD_STATUS_FORBIDDEN:
		case WAPD_STATUS_NOT_FOUND:
		case WAPD_STATUS_METHOD_NOT_ALLOWED:
			// エラー詳細を返すためContentType付加.
			m_ResponseHeader->ContentType = const_cast<char*>("application/json");
			m_ResponseHeader->ContentTypeLength = sizeof("application/json") - 1;
			break;

		// 500系(サーバー要因).
		default:
			// エラー詳細を返さないためContentTypeなし.
			break;
	}

	return si32_Ret;
}

/**
 * @brief ドキュメント操作API 実行.
 * @param[in] ps_Params HTTPリクエストヘッダ、ボディ
 * @retval WAPD_NoError(0) 成功.
 * @retval WAPD_AnyError(1) 失敗.
 */
int32_t WAPC_UserBoxDocumentController::documentOperation( WAPS_SubController *ps_Params )
{
	int32_t si32_Ret = WAPD_AnyError;

	// 前提条件チェック.
	si32_Ret = checkDocOperationReqStatus();
	if( WAPD_FatalError == si32_Ret ) {
		WAPG_API_trace("%s(%d) %s : checkDocOperationReqStatus Fatal error occurred.\n", __FILE__, __LINE__, __FUNCTION__);
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		generateErrorResponseBody( WAPE_BoxError_None );
		return WAPD_AnyError;
	}
	else if( WAPD_AnyError == si32_Ret ) {
		WAPG_API_trace("%s(%d) %s : checkDocOperationReqStatus busy.\n", __FILE__, __LINE__, __FUNCTION__);
		printf("503:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //503->ErrorStatus[WAPD_STATUS_SERVICE_UNAVAILABLE] in WAP_defs.h
		m_ResponseHeader->Status = WAPD_STATUS_SERVICE_UNAVAILABLE;
		generateErrorResponseBody( WAPE_BoxError_None );
		return WAPD_AnyError;
	} else {
		// スコアチェック対応.
	}

	// パラメータ取得.
	int32_t si32_DstBoxNo = 0;
	WAPE_BoxOperation e_Operation = WAPE_BoxOperation_None;
	si32_Ret = parseJsonOperationParam( ps_Params->Param, &e_Operation, &si32_DstBoxNo );
	if( WAPD_NoError != si32_Ret ) {
		WAPG_API_trace("%s(%d) %s : parseJsonOperationParam failed.\n", __FILE__, __LINE__, __FUNCTION__);
		printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
		m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
		generateErrorResponseBody( WAPE_BoxError_InvalidJsonSyntax );
		return WAPD_AnyError;
	}
	// コピー/移動元ボックスとコピー/移動先ボックスが同じでないか.
	if( m_si32_BoxNo == si32_DstBoxNo ) {
		WAPG_API_trace("%s(%d) %s : Both Src BoxNo and Dst BoxNo are same Number.\n", __FILE__, __LINE__, __FUNCTION__);
		printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
		m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
		generateErrorResponseBody( WAPE_BoxError_InvalidBoxNo );
		return WAPD_AnyError;
	}
	// コピー/移動先ボックスNo範囲チェック.
	if( ( si32_DstBoxNo < WAPD_BoxNoMin ) || ( WAPD_BoxNoMax < si32_DstBoxNo ) ) {
		WAPG_API_trace("%s(%d) %s : Invalid DstBoxNo.\n", __FILE__, __LINE__, __FUNCTION__);
		printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
		m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
		generateErrorResponseBody( WAPE_BoxError_InvalidBoxNo );
		return WAPD_AnyError;
	}

	// ボックスドキュメントAPI共通処理.
	APIC_User* pc_SrcUser = NULL;							// コピー/移動先ボックスユーザー.
	APIC_Track* pc_SrcTrack = NULL;							// コピー/移動先ボックス部門.
	char sc_BoxPassword[WAPD_BoxPasswordMaxLength] = {};	// ボックスパスワード.
	APIC_DocInfo c_DocInfo;									// ドキュメント情報.
	si32_Ret = documentCommonPreparation( ps_Params, &pc_SrcUser, &pc_SrcTrack, sc_BoxPassword, &c_DocInfo );
	if( WAPD_NoError != si32_Ret ) {
		WAPG_API_trace("%s(%d) %s : documentCommonPreparation failed.\n", __FILE__, __LINE__, __FUNCTION__);
		return WAPD_AnyError;
	}

	// ボックスパスワードなしの場合は、DocのI/FにはNULLを渡さなければならないため、
	// パスワードを一旦ポインタで受ける必要がある.
	char* psc_BoxPassword = NULL;
	if( 0 < strlen(sc_BoxPassword) ) {
		psc_BoxPassword = sc_BoxPassword;
	}

	// コピー/移動先ボックス存在確認.
	Bool b_IsExist = False;
	si32_Ret = boxExists( si32_DstBoxNo, b_IsExist );
	if( WAPD_NoError != si32_Ret ) {
		WAPG_API_trace("%s(%d) %s : boxExists failed.\n", __FILE__, __LINE__, __FUNCTION__);
		return WAPD_AnyError;
	}
	if( False == b_IsExist ) {
		WAPG_API_trace("%s(%d) %s : box not exists.\n", __FILE__, __LINE__, __FUNCTION__);
		printf("404:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //404->ErrorStatus[WAPD_STATUS_NOT_FOUND] in WAP_defs.h
		m_ResponseHeader->Status = WAPD_STATUS_NOT_FOUND;
		generateErrorResponseBody( WAPE_BoxError_BoxNotExist );
		return WAPD_AnyError;
	}

	// コピー/移動先ボックスインスタンス取得.
	APIC_Box* pc_DstBox = NULL;
	pc_DstBox = APIC_Box::newInstance();
	if( NULL == pc_DstBox ) {
		WAPG_API_trace("%s(%d) %s : APIC_Box::newInstance faild.\n", __FILE__, __LINE__, __FUNCTION__);
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		generateErrorResponseBody( WAPE_BoxError_None );
		return WAPD_AnyError;
	}
	pc_DstBox->setBoxId( si32_DstBoxNo );

	// コピー/移動先ボックス種別チェック.
    APIC_BoxInfo c_DstBoxInfo;		// コピー先ボックス情報.
	APIC_User* p_DstUser 	= NULL;	// コピー先ユーザー.
    APIC_Track* p_DstTrack	= NULL;	// コピー先部門管理情報.
	si32_Ret = getDstBoxInfoAndOwner( si32_DstBoxNo, pc_DstBox, pc_SrcUser, pc_SrcTrack, &c_DstBoxInfo, &p_DstUser, &p_DstTrack );
	pc_DstBox->deleteInstance();
	if( WAPD_NoError != si32_Ret ) {
		WAPG_API_trace("%s(%d) %s : getDstBoxInfoAndOwner faild.\n", __FILE__, __LINE__, __FUNCTION__);
		return WAPD_AnyError;
	}

	// 非同期結果クラス生成.
	WAPC_DocumentOperationResult* pc_AsyncResult = NULL;
	pc_AsyncResult = new WAPC_DocumentOperationResult();
	if( NULL == pc_AsyncResult ) {
		WAPG_API_trace("%s(%d) %s : new WAPC_DocumentOperationResult faild.\n", __FILE__, __LINE__, __FUNCTION__);
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		generateErrorResponseBody( WAPE_BoxError_None );
		return WAPD_AnyError;
	}
	pc_AsyncResult->setHandle( m_pv_Handle );

	// コールバック関数テーブルより、使用可能な関数ポインタを取得.
	FP_ANS_CHANGE_DOCFILE_POSITON fp_Cb = NULL;
	fp_Cb = getDocumentOperationCb();
	if( NULL == fp_Cb ) {
		WAPG_API_trace("%s(%d) %s : getDocumentOperationCb faild.\n", __FILE__, __LINE__, __FUNCTION__);
		delete pc_AsyncResult;
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		generateErrorResponseBody( WAPE_BoxError_None );
		return WAPD_AnyError;
	}
	pc_AsyncResult->setReqId( reinterpret_cast<uint64_t>(fp_Cb) );

	// 非同期結果クラス格納.
	si32_Ret = WAPC_AsyncResponseContainer::newInstance()->push( pc_AsyncResult );
	if( WAPD_NoError != si32_Ret ) {
		WAPG_API_trace("%s(%d) %s : WAPC_AsyncResponseContainer::push faild.\n", __FILE__, __LINE__, __FUNCTION__);
		releaseDocumentOperationCb( fp_Cb );
		delete pc_AsyncResult;
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		generateErrorResponseBody( WAPE_BoxError_None );
		return WAPD_AnyError;
	}

	// コピー/移動元ボックスインスタンス取得.
	APIC_Box* pc_SrcBox = NULL;
	pc_SrcBox = APIC_Box::newInstance();
	if( NULL == pc_SrcBox ) {
		WAPG_API_trace("%s(%d) %s : APIC_Box::newInstance faild.\n", __FILE__, __LINE__, __FUNCTION__);
		WAPC_AsyncResponseContainer::newInstance()->erase( pc_AsyncResult->getReqId() );
		releaseDocumentOperationCb( fp_Cb );
		delete pc_AsyncResult;
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		generateErrorResponseBody( WAPE_BoxError_None );
		return WAPD_AnyError;
	}
	APIC_AccessTokenManager* pc_AccessTokenManager = APIC_AccessTokenManager::newInstance();
	JLGE_InspectLogIF e_InspectLogIF = JLGE_InspectLogIfOperationPanel;
	if( NULL != pc_AccessTokenManager){
		pc_AccessTokenManager->getAppType(m_AccessToken, &e_InspectLogIF);
	}
	// アクセス権情報設定.
	pc_SrcBox->setLogIFNo( e_InspectLogIF );
	
	pc_SrcBox->setBoxId( m_si32_BoxNo );

	// コピー/移動実行.
	int32_t si32_DocRtn = APID_DocapiMethodOk;
	int32_t si32_DstDocNo = 0;
	switch( e_Operation ) {
		case WAPE_BoxOperation_Copy:
			si32_DocRtn = pc_SrcBox->cpyDoc( pc_SrcUser, psc_BoxPassword, m_si32_BoxNo, m_si32_DocNo, p_DstUser, c_DstBoxInfo.sc_BoxPassword, si32_DstBoxNo, &si32_DstDocNo, fp_Cb, pc_SrcTrack, p_DstTrack, static_cast<short>( m_AuthorityID ) );
			break;

		case WAPE_BoxOperation_Move:
			si32_DocRtn = pc_SrcBox->movDoc( pc_SrcUser, psc_BoxPassword, m_si32_BoxNo, m_si32_DocNo, p_DstUser, c_DstBoxInfo.sc_BoxPassword, si32_DstBoxNo, &si32_DstDocNo, fp_Cb, pc_SrcTrack, p_DstTrack, static_cast<short>( m_AuthorityID ) );
			break;

		default:
			WAPG_API_trace("%s(%d) %s : Invalid operation.\n", __FILE__, __LINE__, __FUNCTION__);
			m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
			printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
			generateErrorResponseBody( WAPE_BoxError_None );
			pc_SrcBox->deleteInstance();
			WAPC_AsyncResponseContainer::newInstance()->erase( pc_AsyncResult->getReqId() );
			releaseDocumentOperationCb( fp_Cb );
			delete pc_AsyncResult;
			return WAPD_AnyError;
	}
	pc_SrcBox->deleteInstance();
	if( APID_DocapiMethodOk != si32_DocRtn ) {
		switch( si32_DocRtn ) {
			case APID_DocapiDocOverErr:
				WAPG_API_trace("%s(%d) %s : APID_DocapiDocOverErr.\n", __FILE__, __LINE__, __FUNCTION__);
				printf("503:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //503->ErrorStatus[WAPD_STATUS_SERVICE_UNAVAILABLE] in WAP_defs.h
				m_ResponseHeader->Status = WAPD_STATUS_SERVICE_UNAVAILABLE;
				generateErrorResponseBody( WAPE_BoxError_None );
				break;

			case APID_DocapiOwnerAccessErr:
			case APID_DocapiPasswordAccessErr:
			case APID_DocapiBoxidUnknownErr:
			case APID_DocapiDocidUnknownErr:
				// 上記エラー条件は既にここまでのシーケンスでクリアしているはずである（通常ありえないケース）.
				// よってユーザー要因の400系エラーを返すわけにはいかない.
			case APID_DocapiCommonErr:
			default:
				WAPG_API_trace("%s(%d) %s : cpyDoc/movDoc failed.\n", __FILE__, __LINE__, __FUNCTION__);
				m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
				printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
				generateErrorResponseBody( WAPE_BoxError_None );
				break;
		}
		WAPC_AsyncResponseContainer::newInstance()->erase( pc_AsyncResult->getReqId() );
		releaseDocumentOperationCb( fp_Cb );
		delete pc_AsyncResult;
		return WAPD_AnyError;
	}
	// コピー/移動先ボックスでのドキュメント番号を保持.
	pc_AsyncResult->setDstDocNo( si32_DstDocNo );

	m_ResponseHeader->Status = WAPD_STATUS_OK;
	// 非同期API.
	// 非同期リクエストが成功(WAPD_STATUS_OK/WAPD_NoError)が確定してからフラグを立てる.
	m_ResponseHeader->Async = WAPD_ASYNC_ENABLE;
	return WAPD_NoError;
}

/**
 * @brief ドキュメントリスト取得API 実行.
 * @param[in] ps_Params HTTPリクエストヘッダ、ボディ
 * @retval WAPD_NoError(0) 成功.
 * @retval WAPD_AnyError(1) 失敗.
 */
int32_t WAPC_UserBoxDocumentController::getDocumentList( WAPS_SubController *ps_Params )
{
	int32_t si32_Ret = WAPD_AnyError;

	// 前提条件チェック.
	si32_Ret = checkGetDocListReqStatus();
	if( WAPD_AnyError == si32_Ret ) {
		WAPG_API_trace("%s(%d) %s : checkGetDocListReqStatus busy.\n", __FILE__, __LINE__, __FUNCTION__);
		printf("503:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //503->ErrorStatus[WAPD_STATUS_SERVICE_UNAVAILABLE] in WAP_defs.h
		m_ResponseHeader->Status = WAPD_STATUS_SERVICE_UNAVAILABLE;
		generateErrorResponseBody( WAPE_BoxError_None );
		return WAPD_AnyError;
	}

	// リスト取得開始位置取得.
	int32_t si32_Offset = WAPD_UserBoxDocumentListOffsetDefault;
	si32_Ret = getOffsetFromQuery( ps_Params, si32_Offset );
	if( WAPD_NoError != si32_Ret ) {
		WAPG_API_trace("%s(%d) %s : getOffsetFromQuery failed.\n", __FILE__, __LINE__, __FUNCTION__);
		return WAPD_AnyError;
	}

	// 最大取得数取得.
	int32_t si32_Num = WAPD_UserBoxDocumentListNumDefault;
	si32_Ret = getNumFromQuery( ps_Params, si32_Num );
	if( WAPD_NoError != si32_Ret ) {
		WAPG_API_trace("%s(%d) %s : getNumFromQuery failed.\n", __FILE__, __LINE__, __FUNCTION__);
		return WAPD_AnyError;
	}

	// 文書のユーザー・部門情報を取得.
	APIC_User* pc_User = NULL;		// ユーザーインスタンス.
	APIC_Track* pc_Track = NULL;	// 部門インスタンス.
	getUserTrackInformation( ps_Params, &pc_User, &pc_Track );

	// ボックス存在確認.
	Bool b_IsExist = False;
	si32_Ret = boxExists( m_si32_BoxNo, b_IsExist );
	if( WAPD_NoError != si32_Ret ) {
		WAPG_API_trace("%s(%d) %s : boxExists failed.\n", __FILE__, __LINE__, __FUNCTION__);
		return WAPD_AnyError;
	}
	if( False == b_IsExist ) {
		WAPG_API_trace("%s(%d) %s : box not exists.\n", __FILE__, __LINE__, __FUNCTION__);
		printf("404:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //404->ErrorStatus[WAPD_STATUS_NOT_FOUND] in WAP_defs.h
		m_ResponseHeader->Status = WAPD_STATUS_NOT_FOUND;
		generateErrorResponseBody( WAPE_BoxError_BoxNotExist );
		return WAPD_AnyError;
	}

	// ボックスパスワードを取得.
	Bool b_Rtn = False;
	char sc_BoxPassword[WAPD_BoxPasswordMaxLength] = {};	// ボックスパスワード.
	b_Rtn = APIC_AccessTokenManager::newInstance()->getBoxPassword( ps_Params->AccessToken, m_si32_BoxNo, sc_BoxPassword );
	if( False == b_Rtn ) {
		WAPG_API_trace("%s(%d) %s : APIC_AccessTokenManager::getBoxPassword failed.\n", __FILE__, __LINE__, __FUNCTION__);
		printf("403:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //403->ErrorStatus[WAPD_STATUS_FORBIDDEN] in WAP_defs.h
		m_ResponseHeader->Status = WAPD_STATUS_FORBIDDEN;
		generateErrorResponseBody( WAPE_BoxError_BoxNotOpened );
		return WAPD_AnyError;
	}
	// ボックスパスワードなしの場合は、DocのI/FにはNULLを渡さなければならないため、
	// パスワードを一旦ポインタで受ける必要がある.
	char* psc_BoxPassword = NULL;
	if( 0 < strlen(sc_BoxPassword) ) {
		psc_BoxPassword = sc_BoxPassword;
	}

	// リスト取得要求.
	APIC_DocInfo c_DocInfo;	// 未使用.
	si32_Ret = requestGetDocumentList( pc_User, pc_Track, psc_BoxPassword, si32_Offset, si32_Num, c_DocInfo );
	if( WAPD_NoError != si32_Ret ) {
		WAPG_API_trace("%s(%d) %s : requestGetDocumentList failed.\n", __FILE__, __LINE__, __FUNCTION__);
		return WAPD_AnyError;
	}

	m_ResponseHeader->Status = WAPD_STATUS_OK;
	// 非同期API.
	// 非同期リクエストが成功(WAPD_STATUS_OK/WAPD_NoError)が確定してからフラグを立てる.
	m_ResponseHeader->Async = WAPD_ASYNC_ENABLE;

	return WAPD_NoError;
}

/**
 * @brief ドキュメント情報取得API 実行.
 * @param[in] ps_Params HTTPリクエストヘッダ、ボディ
 * @retval WAPD_NoError(0) 成功.
 * @retval WAPD_AnyError(1) 失敗.
 */
int32_t WAPC_UserBoxDocumentController::getDocumentInformation( WAPS_SubController *ps_Params )
{
	int32_t si32_Ret = WAPD_AnyError;

	// 前提条件チェック.
	si32_Ret = checkGetDocInfoReqStatus();
	if( WAPD_AnyError == si32_Ret ) {
		WAPG_API_trace("%s(%d) %s : checkGetDocInfoReqStatus busy.\n", __FILE__, __LINE__, __FUNCTION__);
		printf("503:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //503->ErrorStatus[WAPD_STATUS_SERVICE_UNAVAILABLE] in WAP_defs.h
		m_ResponseHeader->Status = WAPD_STATUS_SERVICE_UNAVAILABLE;
		generateErrorResponseBody( WAPE_BoxError_None );
		return WAPD_AnyError;
	}

	// ボックスドキュメントAPI共通処理.
	APIC_User* pc_User = NULL;								// ユーザーインスタンス.
	APIC_Track* pc_Track = NULL;							// 部門インスタンス.
	char sc_BoxPassword[WAPD_BoxPasswordMaxLength] = {};	// ボックスパスワード.
	APIC_DocInfo c_DocInfo;									// ドキュメント情報.
	si32_Ret = documentCommonPreparation( ps_Params, &pc_User, &pc_Track, sc_BoxPassword, &c_DocInfo );
	if( WAPD_NoError != si32_Ret ) {
		WAPG_API_trace("%s(%d) %s : documentCommonPreparation failed.\n", __FILE__, __LINE__, __FUNCTION__);
		return WAPD_AnyError;
	}

	// ボックスパスワードなしの場合は、DocのI/FにはNULLを渡さなければならないため、
	// パスワードを一旦ポインタで受ける必要がある.
	char* psc_BoxPassword = NULL;
	if( 0 < strlen(sc_BoxPassword) ) {
		psc_BoxPassword = sc_BoxPassword;
	}

	// APIC_DocInfoのメンバにus_total_page_cnt(総ページ数)があるが、ここにはページ数は入っていない.
	// そのためドキュメントページ数は、ドキュメントリスト取得要求結果からの取得が必要.
	int32_t si32_Offset = WAPD_UserBoxDocumentListOffsetMin;
	int32_t si32_Num = WAPD_UserBoxDocumentListNumAll;
	si32_Ret = requestGetDocumentList( pc_User, pc_Track, psc_BoxPassword, si32_Offset, si32_Num, c_DocInfo );
	if( WAPD_NoError != si32_Ret ) {
		WAPG_API_trace("%s(%d) %s : requestGetDocumentList failed.\n", __FILE__, __LINE__, __FUNCTION__);
		return WAPD_AnyError;
	}

	m_ResponseHeader->Status = WAPD_STATUS_OK;
	// 非同期API.
	// 非同期リクエストが成功(WAPD_STATUS_OK/WAPD_NoError)が確定してからフラグを立てる.
	m_ResponseHeader->Async = WAPD_ASYNC_ENABLE;

	return WAPD_NoError;
}

/**
 * @brief ドキュメント情報設定API 実行.
 * @param[in] ps_Params HTTPリクエストヘッダ、ボディ
 * @retval WAPD_NoError(0) 成功.
 * @retval WAPD_AnyError(1) 失敗.
 */
int32_t WAPC_UserBoxDocumentController::setDocumentInformation( WAPS_SubController *ps_Params )
{
	int32_t si32_Ret = WAPD_AnyError;

	// 前提条件チェック
	si32_Ret = checkSetDocReqStatus();
	if( WAPD_FatalError == si32_Ret ) {
		WAPG_API_trace("%s(%d) %s : checkSetDocReqStatus Fatal error occurred.\n", __FILE__, __LINE__, __FUNCTION__);
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		generateErrorResponseBody( WAPE_BoxError_None );
		return WAPD_AnyError;
	}
	else if( WAPD_AnyError == si32_Ret ) {
		WAPG_API_trace("%s(%d) %s : checkSetDocReqStatus busy.\n", __FILE__, __LINE__, __FUNCTION__);
		printf("503:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //503->ErrorStatus[WAPD_STATUS_SERVICE_UNAVAILABLE] in WAP_defs.h
		m_ResponseHeader->Status = WAPD_STATUS_SERVICE_UNAVAILABLE;
		generateErrorResponseBody( WAPE_BoxError_None );
		return WAPD_AnyError;
	} else {
		// スコアチェック対応.
	}

	// パラメータ取得.
	char sc_DocumetName[APID_DocFileNameMAX] = {};
	si32_Ret = parseJsonSetDocumentInfoParam( ps_Params->Param, sc_DocumetName );
	if( WAPD_NoError != si32_Ret ) {
		WAPG_API_trace("%s(%d) %s : parseJsonSetDocumentInfoParam failed.\n", __FILE__, __LINE__, __FUNCTION__);
		printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
		m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
		generateErrorResponseBody( WAPE_BoxError_InvalidJsonSyntax );
		return WAPD_AnyError;
	}

	std::string str_DocumentName = sc_DocumetName;
	// バイト数チェック.
	if( ( str_DocumentName.length() < WAPD_DocumentNameMinChar ) || ( WAPD_DocumentNameMaxLength < str_DocumentName.length() ) ) {
		WAPG_API_trace("%s(%d) %s : Invalid document name length Byte.\n", __FILE__, __LINE__, __FUNCTION__);
		printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
		m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
		generateErrorResponseBody( WAPE_BoxError_InvalidDocumentName );
		return WAPD_AnyError;
	}
	// 文字列長チェック(マルチバイト文字混在).
	// 1～30文字.
	ushort us_StringLength = 0;
	Bool b_Rtn = getStringLengthUTF8( sc_DocumetName, WAPE_CheckString_1String, us_StringLength );
	if( False == b_Rtn ) {
		WAPG_API_trace("%s(%d) %s : getStringLengthUTF8 failed.\n", __FILE__, __LINE__, __FUNCTION__);
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		generateErrorResponseBody( WAPE_BoxError_None );
		return WAPD_AnyError;
	}
	if( ( us_StringLength < WAPD_DocumentNameMinChar ) || ( WAPD_DocumentNameMaxChar < us_StringLength ) ) {
		WAPG_API_trace("%s(%d) %s : Invalid document name length.\n", __FILE__, __LINE__, __FUNCTION__);
		printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
		m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
		generateErrorResponseBody( WAPE_BoxError_InvalidDocumentName );
		return WAPD_AnyError;
	}

	// ボックスドキュメントAPI共通処理.
	APIC_User* pc_User = NULL;								// ユーザーインスタンス.
	APIC_Track* pc_Track = NULL;							// 部門インスタンス.
	char sc_BoxPassword[WAPD_BoxPasswordMaxLength] = {};	// ボックスパスワード.
	APIC_DocInfo c_DocInfo;									// ドキュメント情報.
	si32_Ret = documentCommonPreparation( ps_Params, &pc_User, &pc_Track, sc_BoxPassword, &c_DocInfo );
	if( WAPD_NoError != si32_Ret ) {
		WAPG_API_trace("%s(%d) %s : documentCommonPreparation failed.\n", __FILE__, __LINE__, __FUNCTION__);
		return WAPD_AnyError;
	}

	// ボックスパスワードなしの場合は、DocのI/FにはNULLを渡さなければならないため、
	// パスワードを一旦ポインタで受ける必要がある.
	char* psc_BoxPassword = NULL;
	if( 0 < strlen(sc_BoxPassword) ) {
		psc_BoxPassword = sc_BoxPassword;
	}

	// 変更できるのはドキュメント名のみ.
	// 取得したAPIC_DocInfoのドキュメント名を変更.
	strncpy( c_DocInfo.sc_DocName, sc_DocumetName, APID_DocFileNameMAX - 1 );

	// ドキュメントインスタンス取得.
	APIC_Doc* pc_Doc = NULL;
	pc_Doc = APIC_Doc::newInstance();
	if( NULL == pc_Doc ) {
		WAPG_API_trace("%s(%d) %s : APIC_Doc::newInstance failed.\n", __FILE__, __LINE__, __FUNCTION__);
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		generateErrorResponseBody( WAPE_BoxError_None );
		return WAPD_AnyError;
	}
	// アクセス権情報設定処理.
	APIC_AccessTokenManager* pc_AccessTokenManager = APIC_AccessTokenManager::newInstance();
	JLGE_InspectLogIF e_InspectLogIF = JLGE_InspectLogIfOperationPanel;
	if( NULL != pc_AccessTokenManager ){
		pc_AccessTokenManager->getAppType( m_AccessToken, &e_InspectLogIF );
	}
	pc_Doc->setLogIFNo( e_InspectLogIF );

	pc_Doc->setBoxId( m_si32_BoxNo );
	pc_Doc->setDocId( m_si32_DocNo );

	// ドキュメント情報設定.
	int32_t si32_DocRtn = APID_DocapiMethodOk;
	si32_DocRtn = pc_Doc->setDocInfo( pc_User, psc_BoxPassword, &c_DocInfo, static_cast<LIBE_CharSet>(APID_DefaultCharSet), pc_Track, 0, static_cast<short>( m_AuthorityID ) );
	pc_Doc->deleteInstance();
	if( APID_DocapiMethodOk != si32_DocRtn ) {
		switch( si32_DocRtn ) {
			case APID_DocapiDataUsedErr:
				WAPG_API_trace("%s(%d) %s : APIC_Doc::setDocInfo busy.\n", __FILE__, __LINE__, __FUNCTION__);
				printf("503:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //503->ErrorStatus[WAPD_STATUS_SERVICE_UNAVAILABLE] in WAP_defs.h
				m_ResponseHeader->Status = WAPD_STATUS_SERVICE_UNAVAILABLE;
				generateErrorResponseBody( WAPE_BoxError_None );
				return WAPD_AnyError;

			case APID_DocapiOwnerAccessErr:
			case APID_DocapiPasswordAccessErr:
			case APID_DocapiBoxidUnknownErr:
			case APID_DocapiDocidUnknownErr:
				// 上記エラー条件は既にここまでのシーケンスでクリアしているはずである（通常ありえないケース）.
				// よってユーザー要因の400系エラーを返すわけにはいかない.
			case APID_DocapiCommonErr:
			default:
				WAPG_API_trace("%s(%d) %s : APIC_Doc::setDocInfo failed.\n", __FILE__, __LINE__, __FUNCTION__);
				m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
				printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
				generateErrorResponseBody( WAPE_BoxError_None );
				return WAPD_AnyError;
		}
	}

	m_ResponseHeader->Status = WAPD_STATUS_OK;
	return WAPD_NoError;
}

/**
 * @brief ドキュメント削除API 実行.
 * @param[in] ps_Params HTTPリクエストヘッダ、ボディ
 * @retval WAPD_NoError(0) 成功.
 * @retval WAPD_AnyError(1) 失敗.
 */
int32_t WAPC_UserBoxDocumentController::deleteDocument( WAPS_SubController *ps_Params )
{
	int32_t si32_Ret = WAPD_AnyError;

	// 前提条件チェック.
	si32_Ret = checkDeleteDocReqStatus();
	if( WAPD_FatalError == si32_Ret ) {
		WAPG_API_trace("%s(%d) %s : checkDeleteDocReqStatus fatal error occurred.\n", __FILE__, __LINE__, __FUNCTION__);
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		generateErrorResponseBody( WAPE_BoxError_None );
		return WAPD_AnyError;
	}
	else if( WAPD_AnyError == si32_Ret ) {
		WAPG_API_trace("%s(%d) %s : checkDeleteDocReqStatus busy.\n", __FILE__, __LINE__, __FUNCTION__);
		printf("503:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //503->ErrorStatus[WAPD_STATUS_SERVICE_UNAVAILABLE] in WAP_defs.h
		m_ResponseHeader->Status = WAPD_STATUS_SERVICE_UNAVAILABLE;
		generateErrorResponseBody( WAPE_BoxError_None );
		return WAPD_AnyError;
	} else {
		// スコアチェック対応.
	}

	// ボックスドキュメントAPI共通処理.
	APIC_User* pc_User = NULL;								// ユーザーインスタンス.
	APIC_Track* pc_Track = NULL;							// 部門インスタンス.
	char sc_BoxPassword[WAPD_BoxPasswordMaxLength] = {};	// ボックスパスワード.
	APIC_DocInfo c_DocInfo;									// ドキュメント情報.
	si32_Ret = documentCommonPreparation( ps_Params, &pc_User, &pc_Track, sc_BoxPassword, &c_DocInfo );
	if( WAPD_NoError != si32_Ret ) {
		WAPG_API_trace("%s(%d) %s : documentCommonPreparation failed.\n", __FILE__, __LINE__, __FUNCTION__);
		return WAPD_AnyError;
	}

	// ボックスパスワードなしの場合は、DocのI/FにはNULLを渡さなければならないため、
	// パスワードを一旦ポインタで受ける必要がある.
	char* psc_BoxPassword = NULL;
	if( 0 < strlen(sc_BoxPassword) ) {
		psc_BoxPassword = sc_BoxPassword;
	}

	// 非同期結果クラス生成.
	WAPC_DeleteDocumentResult* pc_AsyncResult = NULL;
	pc_AsyncResult = new WAPC_DeleteDocumentResult();
	if( NULL == pc_AsyncResult ) {
		WAPG_API_trace("%s(%d) %s : new WAPC_DeleteDocumentResult failed.\n", __FILE__, __LINE__, __FUNCTION__);
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		generateErrorResponseBody( WAPE_BoxError_None );
		return WAPD_AnyError;
	}
	pc_AsyncResult->setHandle( m_pv_Handle );

	// コールバック関数テーブルより、使用可能な関数ポインタを取得.
	FP_ANS_DELETE_DOC fp_Cb = NULL;
	fp_Cb = getDeleteDocumentCb();
	if( NULL == fp_Cb ) {
		WAPG_API_trace("%s(%d) %s : getDeleteDocumentCb failed.\n", __FILE__, __LINE__, __FUNCTION__);
		delete pc_AsyncResult;
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		generateErrorResponseBody( WAPE_BoxError_None );
		return WAPD_AnyError;
	}
	pc_AsyncResult->setReqId( reinterpret_cast<uint64_t>(fp_Cb) );

	// 非同期結果クラス格納.
	si32_Ret = WAPC_AsyncResponseContainer::newInstance()->push( pc_AsyncResult );
	if( WAPD_NoError != si32_Ret ) {
		WAPG_API_trace("%s(%d) %s : WAPC_AsyncResponseContainer::push failed.\n", __FILE__, __LINE__, __FUNCTION__);
		releaseDeleteDocumentCb( fp_Cb );
		delete pc_AsyncResult;
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		generateErrorResponseBody( WAPE_BoxError_None );
		return WAPD_AnyError;
	}

	// ボックスインスタンス取得.
	APIC_Box* pc_Box = NULL;
	pc_Box = APIC_Box::newInstance();
	if( NULL == pc_Box ) {
		WAPG_API_trace("%s(%d) %s : APIC_Box::newInstance failed.\n", __FILE__, __LINE__, __FUNCTION__);
		WAPC_AsyncResponseContainer::newInstance()->erase( pc_AsyncResult->getReqId() );
		releaseDeleteDocumentCb( fp_Cb );
		delete pc_AsyncResult;
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		generateErrorResponseBody( WAPE_BoxError_None );
		return WAPD_AnyError;
	}
	APIC_AccessTokenManager* pc_AccessTokenManager = APIC_AccessTokenManager::newInstance();
	JLGE_InspectLogIF e_InspectLogIF = JLGE_InspectLogIfOperationPanel;
	if( NULL != pc_AccessTokenManager){
		pc_AccessTokenManager->getAppType(m_AccessToken, &e_InspectLogIF);
	}
	// アクセス権情報設定.
	pc_Box->setLogIFNo( e_InspectLogIF );

	pc_Box->setBoxId( m_si32_BoxNo );

	// ドキュメント削除要求.
	int32_t si32_DocRtn = APID_DocapiMethodOk;
	si32_DocRtn = pc_Box->req_deleteDoc( pc_User, psc_BoxPassword, m_si32_DocNo, fp_Cb, APID_DefaultWatchDocID, pc_Track, static_cast<short>( m_AuthorityID ) );
	pc_Box->deleteInstance();
	if( APID_DocapiMethodOk != si32_DocRtn ) {
		switch( si32_DocRtn ) {
			case APID_DocapiDataUsedErr:
				WAPG_API_trace("%s(%d) %s : APIC_Box::req_deleteDoc busy.\n", __FILE__, __LINE__, __FUNCTION__);
				printf("503:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //503->ErrorStatus[WAPD_STATUS_SERVICE_UNAVAILABLE] in WAP_defs.h
				m_ResponseHeader->Status = WAPD_STATUS_SERVICE_UNAVAILABLE;
				generateErrorResponseBody( WAPE_BoxError_None );
				break;

			case APID_DocapiOwnerAccessErr:
			case APID_DocapiPasswordAccessErr:
			case APID_DocapiBoxidUnknownErr:
			case APID_DocapiDocidUnknownErr:
				// 上記エラー条件は既にここまでのシーケンスでクリアしているはずである（通常ありえないケース）.
				// よってユーザー要因の400系エラーを返すわけにはいかない.
			case APID_DocapiCommonErr:
			default:
				WAPG_API_trace("%s(%d) %s : APIC_Box::req_deleteDoc failed.\n", __FILE__, __LINE__, __FUNCTION__);
				m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
				printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
				generateErrorResponseBody( WAPE_BoxError_None );
				break;
		}
		WAPC_AsyncResponseContainer::newInstance()->erase( pc_AsyncResult->getReqId() );
		releaseDeleteDocumentCb( fp_Cb );
		delete pc_AsyncResult;
		return WAPD_AnyError;
	}

	m_ResponseHeader->Status = WAPD_STATUS_OK;
	// 非同期API.
	// 非同期リクエストが成功(WAPD_STATUS_OK/WAPD_NoError)が確定してからフラグを立てる.
	m_ResponseHeader->Async = WAPD_ASYNC_ENABLE;
	return WAPD_NoError;
}

/**
 * @brief ボックスドキュメントAPI共通事前処理.
 * @param[in] ps_Params HTTPリクエストヘッダ、ボディ
 * @param[out] ppc_User ユーザーインスタンス.
 * @param[out] ppc_Track 部門インスタンス.
 * @param[out] psc_BoxPassword ボックスパスワード.
 * @param[out] pc_DocInfo ドキュメント情報.
 * @retval WAPD_NoError(0) 成功.
 * @retval WAPD_AnyError(1) 失敗.
 */
int32_t WAPC_UserBoxDocumentController::documentCommonPreparation( WAPS_SubController *ps_Params, APIC_User** ppc_User, APIC_Track** ppc_Track, char* psc_BoxPassword, APIC_DocInfo* pc_DocInfo )
{
	int32_t si32_Ret = WAPD_AnyError;

	// 文書のユーザー・部門情報を取得.
	getUserTrackInformation( ps_Params, ppc_User, ppc_Track );

	// ボックス存在確認.
	Bool b_IsExist = False;
	si32_Ret = boxExists( m_si32_BoxNo, b_IsExist );
	if( WAPD_NoError != si32_Ret ) {
		WAPG_API_trace("%s(%d) %s : boxExists failed.\n", __FILE__, __LINE__, __FUNCTION__);
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		generateErrorResponseBody( WAPE_BoxError_None );
		return WAPD_AnyError;
	}
	if( False == b_IsExist ) {
		WAPG_API_trace("%s(%d) %s : box not exists.\n", __FILE__, __LINE__, __FUNCTION__);
		printf("404:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //404->ErrorStatus[WAPD_STATUS_NOT_FOUND] in WAP_defs.h
		m_ResponseHeader->Status = WAPD_STATUS_NOT_FOUND;
		generateErrorResponseBody( WAPE_BoxError_BoxNotExist );
		return WAPD_AnyError;
	}

	// ボックスパスワードを取得.
	Bool b_Rtn = False;
	char sc_BoxPassword[WAPD_BoxPasswordMaxLength] = {};	// ボックスパスワード.
	b_Rtn = APIC_AccessTokenManager::newInstance()->getBoxPassword( ps_Params->AccessToken, m_si32_BoxNo, sc_BoxPassword );
	if( False == b_Rtn ) {
		WAPG_API_trace("%s(%d) %s : box not opened.\n", __FILE__, __LINE__, __FUNCTION__);
		printf("403:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //403->ErrorStatus[WAPD_STATUS_FORBIDDEN] in WAP_defs.h
		m_ResponseHeader->Status = WAPD_STATUS_FORBIDDEN;
		generateErrorResponseBody( WAPE_BoxError_BoxNotOpened );
		return WAPD_AnyError;
	}
	strncpy( psc_BoxPassword, sc_BoxPassword, WAPD_BoxPasswordMaxLength );
	// ボックスパスワードなしの場合は、DocのI/FにはNULLを渡さなければならないため、
	// パスワードを一旦ポインタで受ける必要がある.
	char* psc_TempBoxPassword = NULL;
	if( 0 < strlen(sc_BoxPassword) ) {
		psc_TempBoxPassword = sc_BoxPassword;
	}

	// ドキュメントインスタンス取得.
	APIC_Doc* pc_Doc = NULL;
	pc_Doc = APIC_Doc::newInstance();
	if( NULL == pc_Doc ) {
		WAPG_API_trace("%s(%d) %s : APIC_Doc::newInstance failed.\n", __FILE__, __LINE__, __FUNCTION__);
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		generateErrorResponseBody( WAPE_BoxError_None );
		return WAPD_AnyError;
	}
	pc_Doc->setBoxId( m_si32_BoxNo );
	pc_Doc->setDocId( m_si32_DocNo );

	// ドキュメント存在確認.
	int32_t si32_DocRtn = APID_DocapiMethodOk;
	si32_DocRtn = pc_Doc->getDocInfo( *ppc_User, psc_TempBoxPassword, pc_DocInfo, static_cast<LIBE_CharSet>(APID_DefaultCharSet), *ppc_Track );
	pc_Doc->deleteInstance();
	if( APID_DocapiMethodOk != si32_DocRtn ) {
		switch( si32_DocRtn ) {
			case APID_DocapiDocidUnknownErr:
				// [memo] 2018/09/11 第1プラットホーム開発部 第2グループ Sky平井.
				// 現在、存在しないDocNoに対して、getDocInfoを行っても、APID_DocapiDocidUnknownErrが返らず、.
				// 代わりにAPID_DocapiCommonErrが返るため、ステータスコード400が返せない.
				// Docモジュール担当者に相談するも、DocNoはMFP外に見せる前提で設計していないという理由で対応を断られた.
				// (ただし既にIWSでDocNoはMFP外に出ている概念であり、この理由は破綻している).
				// APIC_Boxから文書リストを取得してリストを順に舐めれば、WAPE_BoxError_DocumentNotExistを返すことは実現できるが、.
				// ドキュメント操作API全てが内部的に非同期処理を行わねばならなくなる.
				// そのため性能とのトレードオフで、存在しないDocNo指定のステータスコードは500(Internal Server Error)とする.
				// この件で市場要望が出た場合、再度Docモジュールに相談することとする.
				WAPG_API_trace("%s(%d) %s : document not exists.\n", __FILE__, __LINE__, __FUNCTION__);
				printf("404:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //404->ErrorStatus[WAPD_STATUS_NOT_FOUND] in WAP_defs.h
				m_ResponseHeader->Status = WAPD_STATUS_NOT_FOUND;
				generateErrorResponseBody( WAPE_BoxError_DocumentNotExist );
				return WAPD_AnyError;

			case APID_DocapiCommonErr:
			case APID_DocapiOwnerAccessErr:
			case APID_DocapiPasswordAccessErr:
			case APID_DocapiBoxidUnknownErr:
				// 上記エラー条件は既にここまでのシーケンスでクリアしているはずである（通常ありえないケース）.
				// よってユーザー要因の400系エラーを返すわけにはいかない.
			default:
				WAPG_API_trace("%s(%d) %s : APIC_Doc::getDocInfo failed.\n", __FILE__, __LINE__, __FUNCTION__);
				m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
				printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
				generateErrorResponseBody( WAPE_BoxError_None );
				return WAPD_AnyError;
		}
	}

	return WAPD_NoError;
}

/**
 * @brief JSONからドキュメント操作パラメータを取得.
 * @param[in] pval_Body HTTPリクエストボディJSON.
 * @param[out] pe_Operation 操作種別.
 * @param[out] psi32_DstBoxNo 宛先ボックスNo.
 * @retval WAPD_NoError(0) 成功.
 * @retval WAPD_AnyError(1) 失敗.
 */
int32_t WAPC_UserBoxDocumentController::parseJsonOperationParam( Json::Value* pval_Body, WAPE_BoxOperation* pe_Operation, int32_t* psi32_DstBoxNo )
{
	if( NULL == pval_Body ) {
		WAPG_API_trace("%s(%d) %s : pval_Body is NULL.\n", __FILE__, __LINE__, __FUNCTION__);
		return WAPD_AnyError;
	}

	Json::Value& val_Ref = *pval_Body;
	if( ! val_Ref.isObject() ) {
		WAPG_API_trace("%s(%d) %s : val_Ref is not object.\n", __FILE__, __LINE__, __FUNCTION__);
		return WAPD_AnyError;
	}

	// コピーと移動が同時指定されていないか.
	if( val_Ref.isMember( "copy" ) && val_Ref.isMember( "move" ) ) {
		WAPG_API_trace("%s(%d) %s : selected both copy and move.\n", __FILE__, __LINE__, __FUNCTION__);
		return WAPD_AnyError;
	}

	// パラメータ取得.
	// コピー操作.
	if( val_Ref.isMember( "copy" ) ) {
		if( ! val_Ref["copy"].isObject() ) {
			WAPG_API_trace("%s(%d) %s : copy is not object.\n", __FILE__, __LINE__, __FUNCTION__);
			return WAPD_AnyError;
		}
		*pe_Operation = WAPE_BoxOperation_Copy;

		if( val_Ref["copy"].isMember( "box_no" ) ) {
			if( ! val_Ref["copy"]["box_no"].isInt() ) {
				WAPG_API_trace("%s(%d) %s : box_no is not number.\n", __FILE__, __LINE__, __FUNCTION__);
				return WAPD_AnyError;
			}
			*psi32_DstBoxNo = val_Ref["copy"]["box_no"].asInt();
		}
		else {
			WAPG_API_trace("%s(%d) %s : box_no is not member.\n", __FILE__, __LINE__, __FUNCTION__);
			return WAPD_AnyError;
		}
	}
	// 移動操作.
	else if ( val_Ref.isMember( "move" ) ) {
		if( ! val_Ref["move"].isObject() ) {
			WAPG_API_trace("%s(%d) %s : move is not object.\n", __FILE__, __LINE__, __FUNCTION__);
			return WAPD_AnyError;
		}
		*pe_Operation = WAPE_BoxOperation_Move;

		if( val_Ref["move"].isMember( "box_no" ) ) {
			if( ! val_Ref["move"]["box_no"].isInt() ) {
				WAPG_API_trace("%s(%d) %s : box_no is not number.\n", __FILE__, __LINE__, __FUNCTION__);
				return WAPD_AnyError;
			}
			*psi32_DstBoxNo = val_Ref["move"]["box_no"].asInt();
		}
		else {
			WAPG_API_trace("%s(%d) %s : box_no is not member.\n", __FILE__, __LINE__, __FUNCTION__);
			return WAPD_AnyError;
		}
	}
	else {
		WAPG_API_trace("%s(%d) %s : move is not member.\n", __FILE__, __LINE__, __FUNCTION__);
		return WAPD_AnyError;
	}

	return WAPD_NoError;
}

/**
 * @brief コピー/移動先ボックス情報と所有者を取得.
 * @param[in] si32_DstBoxNo コピー/移動先ボックスNo.
 * @param[in] pc_DstBox コピー/移動先ボックスインスタンス.
 * @param[in] pc_SrcUser コピー/移動元文書ユーザー.
 * @param[in] pc_SrcTrack コピー/移動元文書グループ.
 * @param[out] pc_DstBoxInfo ボックス情報.
 * @param[out] ppc_DstUser コピー/移動先ボックスユーザー.
 * @param[out] ppc_DstTrack コピー/移動先ボックスグループ.
 * @retval WAPD_NoError(0) 成功.
 * @retval WAPD_AnyError(1) 失敗.
 */
int32_t WAPC_UserBoxDocumentController::getDstBoxInfoAndOwner( int32_t si32_DstBoxNo, APIC_Box* pc_DstBox, APIC_User* pc_SrcUser, APIC_Track* pc_SrcTrack, APIC_BoxInfo* pc_DstBoxInfo, APIC_User** ppc_DstUser, APIC_Track** ppc_DstTrack )
{
// NIFではif0部分の判定方法であったが、.
// このようなロジックとなった理由が不明のため、else部分のロジックを採用.
# if 0
	// (※NIFC_boxinfo::checkCreatBoxUserより転載).
	// getBoxInfo()のみでは、正確なユーザーが取得出来ないので、.
	// getBoxOption()で再度判定をする.

	int32_t si32_DocRtn = APID_DocapiMethodOk;
	si32_DocRtn = pc_DstBox->getBoxInfo( NULL, pc_DstBoxInfo, static_cast<LIBE_CharSet>(APID_DefaultCharSet), NULL );	// パブリックユーザー.
	if( si32_DocRtn != APID_DocapiMethodOk ) {
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		generateErrorResponseBody( WAPE_BoxError_None );
		return WAPD_AnyError;
	}

	// (※NIFC_boxinfo::checkCreatBoxUserより転載).
	// 所有ユーザー情報を再取得する.
	// 上記にも記載したが、getBoxInfo()のみでは、正確なユーザーが取得出来ないので、.
	// getBoxOption()で再度判定をする.

	// 指定ボックスが共有ボックスか判定してみる.
    APIC_BoxOption c_BoxOption;	// ボックス情報.
    si32_DocRtn = pc_DstBox->getBoxOption( NULL, pc_DstBoxInfo->sc_BoxPassword, &c_BoxOption, NULL );	// パブリックユーザー.
	if( si32_DocRtn == APID_DocapiMethodOk ) {
		// 共有ボックスなので、コピー/移動先ボックスの所有者はパブリックユーザー.
		*ppc_DstUser = APIC_User::newInstance( TYPD_IndexUserPublic );
	}
	else{
		// 共有ボックスではないため、個人ボックスか判定する.
		si32_DocRtn = pc_DstBox->getBoxOption( pc_SrcUser, pc_DstBoxInfo->sc_BoxPassword, &c_BoxOption, NULL );	// ログインユーザー(個人).
		if( si32_DocRtn == APID_DocapiMethodOk ) {
			// 個人ボックスなので、コピー/移動先ボックスの所有者をクライアントユーザーにする.
			*ppc_DstUser = pc_SrcUser;
		}
		else {
			// 共有ボックス、個人ボックスではないため、グループボックスか判定する.
			si32_DocRtn = pc_DstBox->getBoxOption( NULL, pc_DstBoxInfo->sc_BoxPassword, &c_BoxOption, pc_SrcTrack ); // 部門情報.
			if( si32_DocRtn == APID_DocapiMethodOk ) {
				// グループボックスなので、コピー/移動先ボックスの所有者をクライアントの所属グループにする.
				*ppc_DstTrack = pc_SrcTrack;
			} else {
				// コピー/移動先が共有/個人/グループボックス以外はエラー.
				printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
				m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
				generateErrorResponseBody( WAPE_BoxError_DestBoxAccessNotParmitted );
				return WAPD_AnyError;
			}
		}
	}

	return WAPD_NoError;

# else
	APIC_BoxRegistry* pc_BoxRegistry = NULL;
	pc_BoxRegistry = APIC_BoxRegistry::newInstance();
	if( NULL == pc_BoxRegistry ) {
		WAPG_API_trace("%s(%d) %s : pc_BoxRegistry is NULL.\n", __FILE__, __LINE__, __FUNCTION__);
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		generateErrorResponseBody( WAPE_BoxError_None );
		return WAPD_AnyError;
	}

	int32_t si32_DocRtn = APID_DocapiMethodOk;
	LIBC_List<APIC_BoxList> c_BoxList;

	si32_DocRtn = pc_BoxRegistry->getBoxList( pc_SrcUser, 0, -1, 1, APID_BoxList_ReqOpt_PUG, (TYPE_SearchKey)-1, &c_BoxList, False, LIBE_CharSetUTF8, pc_SrcTrack );
	pc_BoxRegistry->deleteInstance();
	if( si32_DocRtn != APID_DocapiMethodOk ) {
		WAPG_API_trace("%s(%d) %s : APIC_BoxRegistry::getBoxList failed.\n", __FILE__, __LINE__, __FUNCTION__);
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		generateErrorResponseBody( WAPE_BoxError_None );
		return WAPD_AnyError;
	}

	APIC_BoxList* pc_BoxListNode = NULL;
	APIE_BoxListBoxKind e_BoxListKind = APIE_PublicBox;
	c_BoxList.Lock();
	pc_BoxListNode = c_BoxList.findFirst();
	while ( NULL != pc_BoxListNode ) {
		if( si32_DstBoxNo == pc_BoxListNode->sl_BoxID ) {
			break;
		}
		pc_BoxListNode = c_BoxList.findNext();
	}
	c_BoxList.UnLock();
	if( NULL == pc_BoxListNode ) {
		// コピー/移動元文書ユーザーを元にリスト取得して、コピー先ボックス番号が見つからない場合はエラー.
		WAPG_API_trace("%s(%d) %s : dst box access not parmitted.\n", __FILE__, __LINE__, __FUNCTION__);
		// リスト後片付け(APIC_BoxRegistryヘッダ参照).
		releaseBoxListFromBoxRegistry( c_BoxList );
		printf("403:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //403->ErrorStatus[WAPD_STATUS_FORBIDDEN] in WAP_defs.h
		m_ResponseHeader->Status = WAPD_STATUS_FORBIDDEN;
		generateErrorResponseBody( WAPE_BoxError_DestBoxAccessNotParmitted );
		return WAPD_AnyError;
	}

	e_BoxListKind = pc_BoxListNode->sl_BoxKind;
	switch( e_BoxListKind ) {
		case APIE_PublicBox:
			// 共有ボックスなので、コピー/移動先ボックスの所有者はパブリックユーザー.
			*ppc_DstUser = APIC_User::newInstance( TYPD_IndexUserPublic );
			*ppc_DstTrack = NULL;
			break;

		case APIE_UserBox:
			// 個人ボックスなので、コピー/移動先ボックスの所有者をクライアントユーザーにする.
			*ppc_DstUser = pc_SrcUser;
			*ppc_DstTrack = NULL;
			break;

		case APIE_GroupBox:
			// グループボックスなので、コピー/移動先ボックスの所有者をクライアントの所属グループにする.
			*ppc_DstUser = NULL;
			*ppc_DstTrack = pc_SrcTrack;
			break;

		default:
			WAPG_API_trace("%s(%d) %s : invalid box kind.\n", __FILE__, __LINE__, __FUNCTION__);
			// リスト後片付け(APIC_BoxRegistryヘッダ参照).
			releaseBoxListFromBoxRegistry( c_BoxList );
			m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
			printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
			generateErrorResponseBody( WAPE_BoxError_None );
			return WAPD_AnyError;
	}
	// リスト後片付け(APIC_BoxRegistryヘッダ参照).
	releaseBoxListFromBoxRegistry( c_BoxList );

	si32_DocRtn = pc_DstBox->getBoxInfo( *ppc_DstUser, pc_DstBoxInfo, static_cast<LIBE_CharSet>(APID_DefaultCharSet), *ppc_DstTrack );
	if( si32_DocRtn != APID_DocapiMethodOk ) {
		WAPG_API_trace("%s(%d) %s : APIC_Box::getBoxInfo failed.\n", __FILE__, __LINE__, __FUNCTION__);
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		generateErrorResponseBody( WAPE_BoxError_None );
		return WAPD_AnyError;
	}

	return WAPD_NoError;
#endif
}

/**
 * @brief ドキュメント操作コールバック関数ポインタ取得.
 * @retval NULL以外 コールバック関数ポインタ.
 * @retval NULL 空き関数なし.
 */
FP_ANS_CHANGE_DOCFILE_POSITON WAPC_UserBoxDocumentController::getDocumentOperationCb()
{
	FP_ANS_CHANGE_DOCFILE_POSITON fp_Cb = NULL;

	for( uint32_t ui32_Index = 0; ui32_Index < (sizeof(g_s_DocOperationCbTable) / sizeof(g_s_DocOperationCbTable[0])); ++ui32_Index ) {
		if( False == g_s_DocOperationCbTable[ui32_Index].b_InUse ) {
			fp_Cb = g_s_DocOperationCbTable[ui32_Index].fp_Cb;
			g_s_DocOperationCbTable[ui32_Index].b_InUse = True;
			break;
		}
	}
	return fp_Cb;
}

/**
 * @brief ドキュメント操作コールバック関数ポインタ解放.
 * @param[in] fp_Cb コールバック関数ポインタ.
 * @return なし.
 */
void WAPC_UserBoxDocumentController::releaseDocumentOperationCb( FP_ANS_CHANGE_DOCFILE_POSITON fp_Cb )
{
	for( uint32_t ui32_Index = 0; ui32_Index < (sizeof(g_s_DocOperationCbTable) / sizeof(g_s_DocOperationCbTable[0])); ++ui32_Index ) {
		if( fp_Cb == g_s_DocOperationCbTable[ui32_Index].fp_Cb ) {
			g_s_DocOperationCbTable[ui32_Index].b_InUse = False;
			break;
		}
	}
}

/**
 * @brief ドキュメント操作実行結果通知.
 * @param[in] si32_Result 処理結果.
 * @param[in] pv_ResultBuffer 未使用.
 * @param[in] ui64_ReqId 非同期リクエストID.
 * @return なし.
 */
void WAPC_UserBoxDocumentController::documentOperationCompleted( int32_t si32_Result, void* pv_ResultBuffer, uint64_t ui64_ReqId )
{
	void* vp_Handle = NULL;
	int32_t si32_cbStatus = WAPD_AnyError;

	// 非同期結果格納クラス取り出し.
	WAPC_DocumentOperationResult* pc_AsyncResult = NULL;
	pc_AsyncResult = dynamic_cast<WAPC_DocumentOperationResult*>(WAPC_AsyncResponseContainer::newInstance()->getById( ui64_ReqId ));
	if( NULL != pc_AsyncResult ) {
		WAPG_API_trace("%s(%d) %s : WAPC_AsyncResponseContainer::getById failed.\n", __FILE__, __LINE__, __FUNCTION__);
		pc_AsyncResult->setOperationResult( si32_Result );
		vp_Handle = pc_AsyncResult->getHandle();
		si32_cbStatus = WAPD_NoError;
	}

	// 非同期レスポンス用タスクにスイッチ.
	WAPC_AsyncResponder::newInstance()->postMessage( vp_Handle, si32_cbStatus );
}

/**
 * @brief ドキュメント操作 非同期レスポンス生成.
 * @param[in] ps_Params HTTPリクエストヘッダ、ボディ
 * @retval WAPD_NoError(0) 成功.
 * @retval WAPD_AnyError(1) 失敗.
 */
int32_t WAPC_UserBoxDocumentController::generateDocumentOperationResponse( WAPS_SubController *ps_Params )
{
	// 非同期結果クラス取り出し.
	WAPC_DocumentOperationResult* pc_AsyncResult = NULL;
	pc_AsyncResult = dynamic_cast<WAPC_DocumentOperationResult*>(WAPC_AsyncResponseContainer::newInstance()->getByHandle( m_pv_Handle ));
	if( NULL == pc_AsyncResult ) {
		WAPG_API_trace("%s(%d) %s : WAPC_AsyncResponseContainer::getByHandle failed.\n", __FILE__, __LINE__, __FUNCTION__);
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		generateErrorResponseBody( WAPE_BoxError_None );
		return WAPD_AnyError;
	}

	// 結果取得.
	int32_t si32_OperationResult = APID_DOCD_MethodOk;
	si32_OperationResult = pc_AsyncResult->getOperationResult();
	int32_t si32_DstDocId = 0;
	si32_DstDocId = pc_AsyncResult->getDstDocNo();

	// 非同期結果クラス・関数ポインタ片付け.
	WAPC_AsyncResponseContainer::newInstance()->erase( pc_AsyncResult->getReqId() );
	releaseDocumentOperationCb( reinterpret_cast<FP_ANS_CHANGE_DOCFILE_POSITON>(pc_AsyncResult->getReqId()) );
	delete pc_AsyncResult;

	// 結果チェック.
	switch( si32_OperationResult ) {
		case APID_DOCD_MethodOk:
			m_Response = new Json::Value;
			(*m_Response)["dst_doc_no"] = static_cast<Json::Int>(si32_DstDocId);
			m_ResponseHeader->Status = WAPD_STATUS_OK;
			break;

		case APID_DOCD_RetHddFull:
		case APID_DOCD_RetPageCntOver:
			WAPG_API_trace("%s(%d) %s : HDD or pageCnt full.\n", __FILE__, __LINE__, __FUNCTION__);
			printf("503:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //503->ErrorStatus[WAPD_STATUS_SERVICE_UNAVAILABLE] in WAP_defs.h
			m_ResponseHeader->Status = WAPD_STATUS_SERVICE_UNAVAILABLE;
			generateErrorResponseBody( WAPE_BoxError_None );
			return WAPD_AnyError;

		default:
			WAPG_API_trace("%s(%d) %s : document operation failed.\n", __FILE__, __LINE__, __FUNCTION__);
			m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
			printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
			generateErrorResponseBody( WAPE_BoxError_None );
			return WAPD_AnyError;
	}

	return WAPD_NoError;
}

/**
 * @brief ドキュメントリスト取得要求.
 * @param[in] pc_User ユーザーインスタンス.
 * @param[in] pc_Track 部門インスタンス.
 * @param[in] psc_BoxPassword ボックスパスワード.
 * @param[in] si32_Offset オフセット.
 * @param[in] si32_Num 取得数.
 * @param[in] c_DocInfo ドキュメント情報.
 * @retval WAPD_NoError(0) 成功.
 * @retval WAPD_AnyError(1) 失敗.
 */
int32_t WAPC_UserBoxDocumentController::requestGetDocumentList( APIC_User* pc_User, APIC_Track* pc_Track, const char* psc_BoxPassword, int32_t si32_Offset, int32_t si32_Num, APIC_DocInfo& c_DocInfo )
{
	// 非同期結果クラス生成.
	WAPC_GetDocumentListResult* pc_AsyncResult = NULL;
	pc_AsyncResult = new WAPC_GetDocumentListResult();
	if( NULL == pc_AsyncResult ) {
		WAPG_API_trace("%s(%d) %s : new WAPC_GetDocumentListResult failed.\n", __FILE__, __LINE__, __FUNCTION__);
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		generateErrorResponseBody( WAPE_BoxError_None );
		return WAPD_AnyError;
	}
	pc_AsyncResult->setHandle( m_pv_Handle );

	// コールバック関数テーブルより、使用可能な関数ポインタを取得.
	FP_ANS_GET_DOCLIST fp_Cb = NULL;
	fp_Cb = getGetDocumentListCb();
	if( NULL == fp_Cb ) {
		WAPG_API_trace("%s(%d) %s : getGetDocumentListCb failed.\n", __FILE__, __LINE__, __FUNCTION__);
		delete pc_AsyncResult;
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		generateErrorResponseBody( WAPE_BoxError_None );
		return WAPD_AnyError;
	}
	pc_AsyncResult->setReqId( reinterpret_cast<uint64_t>(fp_Cb) );

	// 非同期結果クラスにAPIC_DocInfoをセット.
	pc_AsyncResult->setDocInfo( c_DocInfo );

	// 非同期結果クラス格納.
	int32_t si32_Ret = WAPD_AnyError;
	si32_Ret = WAPC_AsyncResponseContainer::newInstance()->push( pc_AsyncResult );
	if( WAPD_NoError != si32_Ret ) {
		WAPG_API_trace("%s(%d) %s : WAPC_AsyncResponseContainer::push failed.\n", __FILE__, __LINE__, __FUNCTION__);
		releaseGetDocumentListCb( fp_Cb );
		delete pc_AsyncResult;
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		generateErrorResponseBody( WAPE_BoxError_None );
		return WAPD_AnyError;
	}

	// パラメータをAPIC_DocListInfoに設定していく.
	APIC_DocListInfo* pc_DocListInfo = NULL;
	pc_DocListInfo = pc_AsyncResult->getDocListInfo();
	if( NULL == pc_DocListInfo ) {
		WAPG_API_trace("%s(%d) %s : WAPC_GetDocumentListResult::getDocListInfo failed.\n", __FILE__, __LINE__, __FUNCTION__);
		WAPC_AsyncResponseContainer::newInstance()->erase( pc_AsyncResult->getReqId() );
		delete pc_AsyncResult;
		releaseGetDocumentListCb( fp_Cb );
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		generateErrorResponseBody( WAPE_BoxError_None );

		return WAPD_AnyError;
	}
	pc_DocListInfo->listOrder = si32_Offset - 1;
	pc_DocListInfo->listCnt = si32_Num;

	// ボックスインスタンス取得.
	APIC_Box* pc_Box = NULL;
	pc_Box = APIC_Box::newInstance();
	if( NULL == pc_Box ) {
		WAPG_API_trace("%s(%d) %s : APIC_Box::newInstance failed.\n", __FILE__, __LINE__, __FUNCTION__);
		WAPC_AsyncResponseContainer::newInstance()->erase( pc_AsyncResult->getReqId() );
		releaseGetDocumentListCb( fp_Cb );
		delete pc_AsyncResult;
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		generateErrorResponseBody( WAPE_BoxError_None );

		return WAPD_AnyError;
	}
	pc_Box->setBoxId( m_si32_BoxNo );

	// ドキュメントリスト取得要求.
	int32_t si32_DocRtn = APID_DocapiMethodOk;
	si32_DocRtn = pc_Box->req_getDocList( pc_User, psc_BoxPassword, pc_DocListInfo, fp_Cb, pc_Track );
	pc_Box->deleteInstance();

	if( APID_DocapiMethodOk != si32_DocRtn ) {
		switch( si32_DocRtn ) {
			case APID_DocapiCommonErr:
			case APID_DocapiOwnerAccessErr:
			case APID_DocapiPasswordAccessErr:
			case APID_DocapiBoxidUnknownErr:
			case APID_DocapiParametaErr:
				// 上記エラー条件は既にここまでのシーケンスでクリアしているはずである（通常ありえないケース）.
				// よってユーザー要因の400系エラーを返すわけにはいかない.
			default:
				WAPG_API_trace("%s(%d) %s : APIC_Box::req_getDocList failed.\n", __FILE__, __LINE__, __FUNCTION__);
				m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
				printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
				generateErrorResponseBody( WAPE_BoxError_None );
				break;
		}
		WAPC_AsyncResponseContainer::newInstance()->erase( pc_AsyncResult->getReqId() );
		releaseGetDocumentListCb( fp_Cb );
		delete pc_AsyncResult;

		return WAPD_AnyError;
	}

	return WAPD_NoError;
}

/**
 * @brief ドキュメントリスト取得実行結果通知.
 * @param[in] pc_DocListInfo 未使用.
 * @param[in] pv_Param 未使用.
 * @param[in] ui64_ReqId 非同期リクエストID.
 * @return なし.
 */
void WAPC_UserBoxDocumentController::getDocumentListCompleted( APIC_DocListInfo* pc_DocListInfo, void* pv_Param, uint64_t ui64_ReqId )
{
	void* vp_Handle = NULL;
	int32_t si32_cbStatus = WAPD_AnyError;

	// 非同期結果格納クラス取り出し.
	WAPC_GetDocumentListResult* pc_AsyncResult = NULL;
	pc_AsyncResult = dynamic_cast<WAPC_GetDocumentListResult*>(WAPC_AsyncResponseContainer::newInstance()->getById( ui64_ReqId ));
	if( NULL != pc_AsyncResult ) {
		vp_Handle = pc_AsyncResult->getHandle();
		si32_cbStatus = WAPD_NoError;
	}

	// 非同期レスポンス用タスクにスイッチ.
	WAPC_AsyncResponder::newInstance()->postMessage( vp_Handle, si32_cbStatus );
}

/**
 * @brief ドキュメントリスト取得コールバック関数ポインタ取得.
 * @retval NULL以外 コールバック関数ポインタ.
 * @retval NULL 空き関数なし.
 */
FP_ANS_GET_DOCLIST WAPC_UserBoxDocumentController::getGetDocumentListCb()
{
	FP_ANS_GET_DOCLIST fp_Cb = NULL;

	for( uint32_t ui32_Index = 0; ui32_Index < (sizeof(g_s_GetDocListCbTable) / sizeof(g_s_GetDocListCbTable[0])); ++ui32_Index ) {
		if( False == g_s_GetDocListCbTable[ui32_Index].b_InUse ) {
			fp_Cb = g_s_GetDocListCbTable[ui32_Index].fp_Cb;
			g_s_GetDocListCbTable[ui32_Index].b_InUse = True;
			break;
		}
	}
	return fp_Cb;
}

/**
 * @brief ドキュメントリスト取得コールバック関数ポインタ解放.
 * @param[in] fp_Cb コールバック関数ポインタ.
 * @return なし.
 */
void WAPC_UserBoxDocumentController::releaseGetDocumentListCb( FP_ANS_GET_DOCLIST fp_Cb )
{
	for( uint32_t ui32_Index = 0; ui32_Index < (sizeof(g_s_GetDocListCbTable) / sizeof(g_s_GetDocListCbTable[0])); ++ui32_Index ) {
		if( fp_Cb == g_s_GetDocListCbTable[ui32_Index].fp_Cb ) {
			g_s_GetDocListCbTable[ui32_Index].b_InUse = False;
			break;
		}
	}
}

/**
 * @brief ドキュメントリスト取得 非同期レスポンス生成.
 * @param[in] ps_Params HTTPリクエストヘッダ、ボディ
 * @retval WAPD_NoError(0) 成功.
 * @retval WAPD_AnyError(1) 失敗.
 */
int32_t WAPC_UserBoxDocumentController::generateGetDocumentListResponse( WAPS_SubController *ps_Params )
{
	// 非同期結果クラス取り出し.
	WAPC_GetDocumentListResult* pc_AsyncResult = NULL;
	pc_AsyncResult = dynamic_cast<WAPC_GetDocumentListResult*>(WAPC_AsyncResponseContainer::newInstance()->getByHandle( m_pv_Handle ));
	if( NULL == pc_AsyncResult ) {
		WAPG_API_trace("%s(%d) %s : WAPC_AsyncResponseContainer::getByHandle failed.\n", __FILE__, __LINE__, __FUNCTION__);
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		generateErrorResponseBody( WAPE_BoxError_None );
		return WAPD_AnyError;
	}

	// 結果取得.
	APIC_DocListInfo* pc_DocListInfo = NULL;
	pc_DocListInfo = pc_AsyncResult->getDocListInfo();
	if( NULL == pc_DocListInfo ) {
		WAPG_API_trace("%s(%d) %s : WAPC_GetDocumentListResult::getDocListInfo failed.\n", __FILE__, __LINE__, __FUNCTION__);
		WAPC_AsyncResponseContainer::newInstance()->erase( pc_AsyncResult->getReqId() );
		releaseGetDocumentListCb( reinterpret_cast<FP_ANS_GET_DOCLIST>(pc_AsyncResult->getReqId()) );
		delete pc_AsyncResult;
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		generateErrorResponseBody( WAPE_BoxError_None );
		return WAPD_AnyError;
	}

	// ドキュメントが1つも無い場合は204(No Content).
	if ( 0 == pc_DocListInfo->pList->getCount() ) {
		WAPG_API_trace("%s(%d) %s : document list is empty.\n", __FILE__, __LINE__, __FUNCTION__);
		printf("204:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //204->ErrorStatus[WAPD_STATUS_NO_CONTENT] in WAP_defs.h
		m_ResponseHeader->Status = WAPD_STATUS_NO_CONTENT;
	}
	else {
		m_Response = new Json::Value;
		Json::UInt ui_Index = 0;
		APIC_DocListTbl* pc_Doclist = NULL;

		pc_DocListInfo->pList->Lock();
		pc_Doclist = pc_DocListInfo->pList->findFirst();
		while ( NULL != pc_Doclist ) {
			(*m_Response)["document_list"][ui_Index] = pc_Doclist->docID;
			pc_Doclist = pc_DocListInfo->pList->findNext();
			ui_Index++;
		}
		pc_DocListInfo->pList->UnLock();

		m_ResponseHeader->Status = WAPD_STATUS_OK;
	}

	// 非同期結果クラス・関数ポインタ片付け.
	WAPC_AsyncResponseContainer::newInstance()->erase( pc_AsyncResult->getReqId() );
	releaseGetDocumentListCb( reinterpret_cast<FP_ANS_GET_DOCLIST>(pc_AsyncResult->getReqId()) );
	delete pc_AsyncResult;

	return WAPD_NoError;
}

/**
 * @brief ドキュメント情報取得 非同期レスポンス生成.
 * @param[in] ps_Params HTTPリクエストヘッダ、ボディ
 * @retval WAPD_NoError(0) 成功.
 * @retval WAPD_AnyError(1) 失敗.
 */
int32_t WAPC_UserBoxDocumentController::generateGetDocumentInformationResponse( WAPS_SubController *ps_Params )
{
	// 非同期結果クラス取り出し.
	WAPC_GetDocumentListResult* pc_AsyncResult = NULL;
	pc_AsyncResult = dynamic_cast<WAPC_GetDocumentListResult*>(WAPC_AsyncResponseContainer::newInstance()->getByHandle( m_pv_Handle ));
	if( NULL == pc_AsyncResult ) {
		WAPG_API_trace("%s(%d) %s : WAPC_AsyncResponseContainer::getByHandle failed.\n", __FILE__, __LINE__, __FUNCTION__);
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		generateErrorResponseBody( WAPE_BoxError_None );
		return WAPD_AnyError;
	}

	// ドキュメント情報取り出し.
	APIC_DocInfo* pc_DocInfo = NULL;
	pc_DocInfo = pc_AsyncResult->getDocInfo();
	if( NULL == pc_DocInfo ) {
		WAPG_API_trace("%s(%d) %s : WAPC_GetDocumentListResult::getDocInfo failed.\n", __FILE__, __LINE__, __FUNCTION__);
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		generateErrorResponseBody( WAPE_BoxError_None );
		return WAPD_AnyError;
	}

	int32_t si32_DocRtn = DOCD_RetOk;
	int32_t si32_Rtn = WAPD_AnyError;
	char sc_DocUserName[UACD_MAX_UserName] = {};

	APIC_DocListInfo* pc_DocListInfo = NULL;
	APIC_DocListTbl* pc_Doclist = NULL;
	uint32_t ui32_PageCnt = 0;

	m_Response = new Json::Value;
	Json::Value& val_Ref = *m_Response;

	// 文書名.
	val_Ref["document_name"] = pc_DocInfo->sc_DocName;

	// 文書登録機能.
	char sc_DocFunc[WAPD_MaxKeyLength] = {};
	if( False == WAPS_CommonTableString<WAPE_TypeOfParam_DocFuncJson, int32_t>::CommonToWapString( pc_DocInfo->sl_DocFunc, sc_DocFunc ) ) {
		WAPG_API_trace("%s(%d) %s : CommonToWapString failed.\n", __FILE__, __LINE__, __FUNCTION__);
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		generateErrorResponseBody( WAPE_BoxError_None );
		return WAPD_AnyError;
	}
	val_Ref["document_regist_function"] = sc_DocFunc;

	// 文書生成時間.
	val_Ref["document_create_time"]["year"] = static_cast<Json::Int>(pc_DocInfo->t_DocTime.ss_Year);
	val_Ref["document_create_time"]["month"] = static_cast<Json::Int>(pc_DocInfo->t_DocTime.sc_Month);
	val_Ref["document_create_time"]["day"] = static_cast<Json::Int>(pc_DocInfo->t_DocTime.sc_Day);
	val_Ref["document_create_time"]["hour"] = static_cast<Json::Int>(pc_DocInfo->t_DocTime.sc_Hour);
	val_Ref["document_create_time"]["minute"] = static_cast<Json::Int>(pc_DocInfo->t_DocTime.sc_Minute);
	val_Ref["document_create_time"]["second"] = static_cast<Json::Int>(pc_DocInfo->t_DocTime.sc_Second);

	// 文書最終取り出し時間.
	val_Ref["document_final_time"]["year"] = static_cast<Json::Int>(pc_DocInfo->t_LastGetTime.ss_Year);
	val_Ref["document_final_time"]["month"] = static_cast<Json::Int>(pc_DocInfo->t_LastGetTime.sc_Month);
	val_Ref["document_final_time"]["day"] = static_cast<Json::Int>(pc_DocInfo->t_LastGetTime.sc_Day);
	val_Ref["document_final_time"]["hour"] = static_cast<Json::Int>(pc_DocInfo->t_LastGetTime.sc_Hour);
	val_Ref["document_final_time"]["minute"] = static_cast<Json::Int>(pc_DocInfo->t_LastGetTime.sc_Minute);
	val_Ref["document_final_time"]["second"] = static_cast<Json::Int>(pc_DocInfo->t_LastGetTime.sc_Second);


	// ジョブスペックインスタンス生成.
	TYPC_JobSpec c_JobSpec;
	c_JobSpec.pc_Common  		= new TYPC_Common;
	c_JobSpec.pc_Scan			= new TYPC_Scan;
	c_JobSpec.pc_Print			= new TYPC_Print;
	c_JobSpec.pc_Send			= new TYPC_Send;
	c_JobSpec.pc_Fax			= new TYPC_Fax;
	c_JobSpec.pc_Interpreter	= new TYPC_Interpreter;
	c_JobSpec.pc_IPP			= new TYPC_IPP;
	c_JobSpec.pc_ImageConvert	= new TYPC_ImageConvert;
	c_JobSpec.pc_Plugin			= new TYPC_Plugin;

	// 文書のジョブスペック取得.
	si32_DocRtn = DOCC_DocFileAccessIf::getJobSpec( m_si32_BoxNo, m_si32_DocNo, &c_JobSpec );
	if( DOCD_RetOk != si32_DocRtn ) {
		WAPG_API_trace("%s(%d) %s : DOCC_DocFileAccessIf::getJobSpec failed.\n", __FILE__, __LINE__, __FUNCTION__);
		goto LABEL_ERROR;
	}

	// 文書ユーザー名.
	c_JobSpec.pc_Common->c_UserAcount.getUserNameUTF8( sc_DocUserName );
	val_Ref["document_user_name"] = sc_DocUserName;

	// 保存文書のバインド可否.
	val_Ref["enable_bind"] = ( True == checkEnableBind( &c_JobSpec ) );

	// 保存文書のカラー情報.
	si32_Rtn = checkColorInfo( &c_JobSpec, m_Response );
	if( WAPD_NoError != si32_Rtn ) {
		WAPG_API_trace("%s(%d) %s : checkColorInfo failed.\n", __FILE__, __LINE__, __FUNCTION__);
		goto LABEL_ERROR;
	}

	// 保存文書に設定される応用設定に付随したカラー情報.
	si32_Rtn = checkStampColorInfo( &c_JobSpec, m_Response );
	if( WAPD_NoError != si32_Rtn ) {
		WAPG_API_trace("%s(%d) %s : checkStampColorInfo failed.\n", __FILE__, __LINE__, __FUNCTION__);
		goto LABEL_ERROR;
	}

	// 保存文書のページ数取得.
	pc_DocListInfo = pc_AsyncResult->getDocListInfo();
	if( NULL == pc_DocListInfo ) {
		WAPG_API_trace("%s(%d) %s : WAPC_GetDocumentListResult::getDocListInfo failed.\n", __FILE__, __LINE__, __FUNCTION__);
		WAPC_AsyncResponseContainer::newInstance()->erase( pc_AsyncResult->getReqId() );
		releaseGetDocumentListCb( reinterpret_cast<FP_ANS_GET_DOCLIST>(pc_AsyncResult->getReqId()) );
		delete pc_AsyncResult;
		goto LABEL_ERROR;
	}

	pc_DocListInfo->pList->Lock();
	pc_Doclist = pc_DocListInfo->pList->findFirst();
	while ( NULL != pc_Doclist ) {
		if ( m_si32_DocNo == pc_Doclist->docID ) {
			ui32_PageCnt = pc_Doclist->pageCnt;
			break;
		}
		pc_Doclist = pc_DocListInfo->pList->findNext();
	}
	pc_DocListInfo->pList->UnLock();

	val_Ref["document_pages"] = static_cast<Json::UInt>(ui32_PageCnt);

	// 非同期結果クラス・関数ポインタ片付け.
	WAPC_AsyncResponseContainer::newInstance()->erase( pc_AsyncResult->getReqId() );
	releaseGetDocumentListCb( reinterpret_cast<FP_ANS_GET_DOCLIST>(pc_AsyncResult->getReqId()) );
	delete pc_AsyncResult;

	// 正常完了.
	delete c_JobSpec.pc_Common;
	delete c_JobSpec.pc_Scan;
	delete c_JobSpec.pc_Print;
	delete c_JobSpec.pc_Send;
	delete c_JobSpec.pc_Fax;
	delete c_JobSpec.pc_Interpreter;
	delete c_JobSpec.pc_IPP;
	delete c_JobSpec.pc_ImageConvert;
	delete c_JobSpec.pc_Plugin;
	m_ResponseHeader->Status = WAPD_STATUS_OK;
	return WAPD_NoError;

LABEL_ERROR:
	// エラー終了.
	delete c_JobSpec.pc_Common;
	delete c_JobSpec.pc_Scan;
	delete c_JobSpec.pc_Print;
	delete c_JobSpec.pc_Send;
	delete c_JobSpec.pc_Fax;
	delete c_JobSpec.pc_Interpreter;
	delete c_JobSpec.pc_IPP;
	delete c_JobSpec.pc_ImageConvert;
	delete c_JobSpec.pc_Plugin;
	m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
	printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
	generateErrorResponseBody( WAPE_BoxError_None );
	return WAPD_AnyError;
}

/**
 * @brief バインド可否をチェック..
 * @param[in] pc_JobSpec ジョブスペック.
 * @retval True バインド可.
 * @retval False バインド不可.
 */
Bool WAPC_UserBoxDocumentController::checkEnableBind( TYPC_JobSpec* pc_JobSpec )
{
	// バインドと組み合わせ禁止のモードをチェック.
	// イメージリピート.
	if( True == pc_JobSpec->pc_Common->c_Repeat.getOn() ) {
		return False;
	}
	// オーバーレイ(登録オーバーレイにて蓄積された画像はバインド可能).
	if( ( TYPE_OverlayOff != pc_JobSpec->pc_Common->getOverlay().e_Overlay )
	&& ( TYPE_OverlayOn_Load != pc_JobSpec->pc_Common->getOverlay().e_Overlay ) ) {
		return False;
	}
	// 差込ページ.
	if( True == pc_JobSpec->pc_Scan->c_OriginalInsertion.getOn() ) {
		return False;
	}
	// カタログ連写.
	if( TYPE_CatalogShotNone != pc_JobSpec->pc_Scan->c_CatalogShot.getCatalogShot() ) {
		return False;
	}
	// 拡大連写.
	if( TYPE_MultiPageModeOff != pc_JobSpec->pc_Scan->c_MultiPage.getMultiPageMode() ) {
		return False;
	}
	// 小冊子.
	if( True == pc_JobSpec->pc_Print->c_Nin1.getBookletOn() ) {
		return False;
	}
	// 用紙挿入モードON(表紙).
	if( TYPE_PaperInsertionOff != pc_JobSpec->pc_Print->c_Intersheet.getFrontMode().getPaperInsertion() ) {
		return False;
	}
	// 用紙挿入モードON(裏表紙).
	if( TYPE_PaperInsertionOff != pc_JobSpec->pc_Print->c_Intersheet.getBackMode().getPaperInsertion() ) {
		return False;
	}
	// 用紙挿入モードON(中間).
	if(	TYPE_PaperInsertionOff != pc_JobSpec->pc_Print->c_Intersheet.getMiddleMode().getPaperInsertion() ) {
		return False;
	}
	// 用紙挿入モードON(OHP合紙).
	if( TYPE_PaperInsertionOff != pc_JobSpec->pc_Print->c_OhpInsertion.getPaperInsertion() ) {
		return False;
	}
	// 章分け.
	if( True == pc_JobSpec->pc_Print->c_Chapter.getOn() ) {
		return False;
	}

	return True;
}

/**
 * @brief 保存文書のカラー情報をチェック.
 * @param[in] pc_JobSpec ジョブスペック.
 * @retval WAPD_NoError(0) 成功.
 * @retval WAPD_AnyError(1) 失敗.
 */
int32_t WAPC_UserBoxDocumentController::checkColorInfo( TYPC_JobSpec* pc_JobSpec, Json::Value* val_Json )
{
	Json::Value& val_Ref = *val_Json;
	TYPE_MonoColor e_MonoColor = TYPE_MonoColorRed;
	char sc_MonoColor[WAPD_MaxKeyLength] = {};
	Json::UInt ui_JsonIndex = 0;

	// 保存時の単色/2色カラーによる判定.
	// 2色カラー判定.
	if( True == pc_JobSpec->pc_Common->c_2ColorMode.get2ColorOn() ) {
		// 2色カラーである.
		// 2色カラー時はブラックはデフォルトで有効.
		val_Ref["document_colorinfo"]["scan_color"][ui_JsonIndex] = "BLACK";
		// 2色目.
		e_MonoColor = pc_JobSpec->pc_Common->c_2ColorMode.getMonoColor();
		if( False == WAPS_CommonTableString<WAPE_TypeOfParam_MonoColorJson, TYPE_MonoColor>::CommonToWapString( e_MonoColor, sc_MonoColor ) ) {
			WAPG_API_trace("%s(%d) %s : CommonToWapString failed.\n", __FILE__, __LINE__, __FUNCTION__);
			return WAPD_AnyError;
		}
		ui_JsonIndex++;
		val_Ref["document_colorinfo"]["scan_color"][ui_JsonIndex] = sc_MonoColor;

	}
	// 単色カラー判定.
	else if( True == pc_JobSpec->pc_Common->c_MonoColor.getMonoColorOn() ) {
		// 単色カラーである.
		e_MonoColor = pc_JobSpec->pc_Common->c_MonoColor.getMonoColor();
		if( False == WAPS_CommonTableString<WAPE_TypeOfParam_MonoColorJson, TYPE_MonoColor>::CommonToWapString( e_MonoColor, sc_MonoColor ) ) {
			WAPG_API_trace("%s(%d) %s : CommonToWapString failed.\n", __FILE__, __LINE__, __FUNCTION__);
			return WAPD_AnyError;
		}
		val_Ref["document_colorinfo"]["scan_color"][ui_JsonIndex] = sc_MonoColor;
	}
	else {
		// 原稿カラー種別による判定.
		TYPE_OriginalsColorType e_OriginalsColorType = TYPE_OriginalsColorTypeColor;
		TYPC_JobStatus* pc_JobStatus = new TYPC_JobStatus;
		APIC_Doc* pc_Doc = APIC_Doc::newInstance();
		pc_Doc->setBoxId(m_si32_BoxNo);
		pc_Doc->setDocId(m_si32_DocNo);
		pc_Doc->getJobStatus( pc_JobStatus );
		pc_Doc->deleteInstance();
		e_OriginalsColorType = pc_JobStatus->getOriginalsColorType();
		delete pc_JobStatus;

		switch( e_OriginalsColorType ) {
			// モノクロ.
			case TYPE_OriginalsColorTypeMono:
				val_Ref["document_colorinfo"]["scan_color"][ui_JsonIndex] = "BLACK";
				break;

			// フルカラー/混載.
			case TYPE_OriginalsColorTypeColor:
			case TYPE_OriginalsColorTypePageMix:
				val_Ref["document_colorinfo"]["scan_color"][ui_JsonIndex] = "FULLCOLOR";
				break;

			default:
				WAPG_API_trace("%s(%d) %s : TYPC_JobStatus::getOriginalsColorType failed.\n", __FILE__, __LINE__, __FUNCTION__);
				return WAPD_AnyError;
		}
	}

	return WAPD_NoError;
}

/**
 * @brief 保存文書に設定される応用設定に付随したカラー情報をチェック.
 * @param[in] pc_JobSpec ジョブスペック.
 * @retval WAPD_NoError(0) 成功.
 * @retval WAPD_AnyError(1) 失敗.
 */
int32_t WAPC_UserBoxDocumentController::checkStampColorInfo( TYPC_JobSpec* pc_JobSpec, Json::Value* val_Json )
{
	Json::Value& val_Ref = *val_Json;
	Bool b_UsingColor[WAPE_StampColor_Max] = {};
	WAPE_StampColor e_StampColor = WAPE_StampColor_Black;

	// スタンプ設定.
	if( True == pc_JobSpec->pc_Common->c_Stamp.getOn() ) {
		if( False == WAPS_CommonTable<WAPE_TypeOfParam_StampColor, TYPE_StampColor>::CommonToWap( pc_JobSpec->pc_Common->c_Stamp.getStampColor(), e_StampColor ) ) {
			WAPG_API_trace("%s(%d) %s : CommonToWap failed.\n", __FILE__, __LINE__, __FUNCTION__);
			return WAPD_AnyError;
		}
		b_UsingColor[e_StampColor] = True;
	}
	// 日付/時刻設定.
	if( True == pc_JobSpec->pc_Common->c_DateStamp.getOn() ) {
		if( False == WAPS_CommonTable<WAPE_TypeOfParam_DateStampColor, TYPE_DateStampColor>::CommonToWap( pc_JobSpec->pc_Common->c_DateStamp.getDateStampColor(), e_StampColor ) ) {
			WAPG_API_trace("%s(%d) %s : CommonToWap failed.\n", __FILE__, __LINE__, __FUNCTION__);
			return WAPD_AnyError;
		}
		b_UsingColor[e_StampColor] = True;
	}
	// ページ番号設定.
	if( True == pc_JobSpec->pc_Common->c_PageStamp.getOn() ) {
		if( False == WAPS_CommonTable<WAPE_TypeOfParam_PageStampColor, TYPE_PageStampColor>::CommonToWap( pc_JobSpec->pc_Common->c_PageStamp.getPageStampColor(), e_StampColor ) ) {
			WAPG_API_trace("%s(%d) %s : CommonToWap failed.\n", __FILE__, __LINE__, __FUNCTION__);
			return WAPD_AnyError;
		}
		b_UsingColor[e_StampColor] = True;
	}
	// コピープロテクト設定、コピーガード設定、パスワードコピー設定.
	const int32_t si32_CopyPasswordLength = 8;	// パスワードコピーのパスワード長.
	char sc_CopyPassword[si32_CopyPasswordLength+1] = {};
	pc_JobSpec->pc_Common->getJimonPattern2()->getPasswordString(sc_CopyPassword);
	if( ( True == pc_JobSpec->pc_Common->c_JimonPattern.getOn() )
	|| ( True == pc_JobSpec->pc_Common->getJimonPattern2()->getCopyGuardOn() )
	|| ( ( True == pc_JobSpec->pc_Common->getJimonPattern2()->getPasswordCopyOn() ) && ( 0 < strlen( sc_CopyPassword ) ) ) ) {
		if( False == WAPS_CommonTable<WAPE_TypeOfParam_JimonColor, TYPE_JimonColor>::CommonToWap( pc_JobSpec->pc_Common->c_JimonPattern.getJimonColor(), e_StampColor ) ) {
			WAPG_API_trace("%s(%d) %s : CommonToWap failed.\n", __FILE__, __LINE__, __FUNCTION__);
			return WAPD_AnyError;
		}
		b_UsingColor[e_StampColor] = True;
	}
	// 繰り返しスタンプ設定.
	if( True == pc_JobSpec->pc_Common->c_Watermark.getOn() ) {
		if( False == WAPS_CommonTable<WAPE_TypeOfParam_WatermarkColor, TYPE_WatermarkColor>::CommonToWap( pc_JobSpec->pc_Common->c_Watermark.getWatermarkColor(), e_StampColor ) ) {
			WAPG_API_trace("%s(%d) %s : CommonToWap failed.\n", __FILE__, __LINE__, __FUNCTION__);
			return WAPD_AnyError;
		}
		b_UsingColor[e_StampColor] = True;
	}
	// ウォーターマーク設定.
	if( True == pc_JobSpec->pc_Common->c_OriginalWatermark.getOn() ) {
		if( False == WAPS_CommonTable<WAPE_TypeOfParam_WatermarkColor, TYPE_WatermarkColor>::CommonToWap( pc_JobSpec->pc_Common->c_OriginalWatermark.getColor(), e_StampColor ) ) {
			WAPG_API_trace("%s(%d) %s : CommonToWap failed.\n", __FILE__, __LINE__, __FUNCTION__);
			return WAPD_AnyError;
		}
		b_UsingColor[e_StampColor] = True;
	}
	// オーバーレイ設定.
	if( TYPE_OverlayOn == pc_JobSpec->pc_Common->getOverlay().e_Overlay ) {
		if( False == WAPS_CommonTable<WAPE_TypeOfParam_OverlayColor, TYPE_OverlayColorKind>::CommonToWap( pc_JobSpec->pc_Common->getOverlay().e_OverlayColorKind, e_StampColor ) ) {
			WAPG_API_trace("%s(%d) %s : CommonToWap failed.\n", __FILE__, __LINE__, __FUNCTION__);
			return WAPD_AnyError;
		}
		b_UsingColor[e_StampColor] = True;
	}
	// 呼び出しオーバーレイ設定.
	if( ( TYPE_OverlayOn_Load == pc_JobSpec->pc_Common->getOverlay().e_Overlay )
	&& ( 0 != pc_JobSpec->pc_Common->getOverlay().uc_ID ) ) {
		if( False == WAPS_CommonTable<WAPE_TypeOfParam_OverlayColor, TYPE_OverlayColorKind>::CommonToWap( pc_JobSpec->pc_Common->getOverlay().e_OverlayColorKind, e_StampColor ) ) {
			WAPG_API_trace("%s(%d) %s : CommonToWap failed.\n", __FILE__, __LINE__, __FUNCTION__);
			return WAPD_AnyError;
		}
		b_UsingColor[e_StampColor] = True;
	}
	// 呼び出しオーバーレイ設定(裏面).
	if( ( TYPE_OverlayOn_Load == pc_JobSpec->pc_Common->getOverlayBack().e_Overlay )
	&& ( 0 != pc_JobSpec->pc_Common->getOverlayBack().uc_ID ) ) {
		if( False == WAPS_CommonTable<WAPE_TypeOfParam_OverlayColor, TYPE_OverlayColorKind>::CommonToWap( pc_JobSpec->pc_Common->getOverlayBack().e_OverlayColorKind, e_StampColor ) ) {
			WAPG_API_trace("%s(%d) %s : CommonToWap failed.\n", __FILE__, __LINE__, __FUNCTION__);
			return WAPD_AnyError;
		}
		b_UsingColor[e_StampColor] = True;
	}
	// ヘッダー/フッター設定.
	if( ( True == pc_JobSpec->pc_Common->c_HeaderFooterStamp.getHeaderOn() )
	|| ( True == pc_JobSpec->pc_Common->c_HeaderFooterStamp.getFooterOn() ) ) {
		if( False == WAPS_CommonTable<WAPE_TypeOfParam_HeaderFooterColor, TYPE_HeaderFooterStampColor>::CommonToWap( pc_JobSpec->pc_Common->c_HeaderFooterStamp.getHeaderFooterStampColor(), e_StampColor ) ) {
			WAPG_API_trace("%s(%d) %s : CommonToWap failed.\n", __FILE__, __LINE__, __FUNCTION__);
			return WAPD_AnyError;
		}
		b_UsingColor[e_StampColor] = True;
	}

	char sc_StampColor[WAPD_MaxKeyLength] = {};
	Json::UInt ui_JsonIndex = 0;
	for( e_StampColor = WAPE_StampColor_Black; e_StampColor < WAPE_StampColor_Max; ++e_StampColor ) {
		if( True == b_UsingColor[e_StampColor] ) {
			memset( sc_StampColor, 0, WAPD_MaxKeyLength );
			if( False == WAPS_CommonTableString<WAPE_TypeOfParam_StampColorJson, WAPE_StampColor>::CommonToWapString( e_StampColor, sc_StampColor ) ) {
				WAPG_API_trace("%s(%d) %s : CommonToWapString failed.\n", __FILE__, __LINE__, __FUNCTION__);
				return WAPD_AnyError;
			}
			val_Ref["document_colorinfo"]["stamp_color"][ui_JsonIndex] = sc_StampColor;
			ui_JsonIndex++;
		}
	}

	return WAPD_NoError;
}

/**
 * @brief JSONからドキュメント操作パラメータを取得.
 * @param[in] pval_Body HTTPリクエストボディJSON.
 * @param[out] psc_DocumetName ドキュメント名.
 * @retval WAPD_NoError(0) 成功.
 * @retval WAPD_AnyError(1) 失敗.
 */
int32_t WAPC_UserBoxDocumentController::parseJsonSetDocumentInfoParam( Json::Value* pval_Body, char* psc_DocumetName )
{
	if( NULL == pval_Body ) {
		WAPG_API_trace("%s(%d) %s : pval_Body is NULL.\n", __FILE__, __LINE__, __FUNCTION__);
		return WAPD_AnyError;
	}

	Json::Value& val_Ref = *pval_Body;
	if( ! val_Ref.isObject() ) {
		WAPG_API_trace("%s(%d) %s : val_Ref is not object.\n", __FILE__, __LINE__, __FUNCTION__);
		return WAPD_AnyError;
	}

	// パラメータ取得.
	// ドキュメント名.
	if( val_Ref.isMember( "document_name" ) ) {
		if( ! val_Ref["document_name"].isString() ) {
			WAPG_API_trace("%s(%d) %s : document_name is not string.\n", __FILE__, __LINE__, __FUNCTION__);
			return WAPD_AnyError;
		}
		strncpy( psc_DocumetName, val_Ref["document_name"].asCString(), (APID_DocFileNameMAX - 1) );
	}
	else {
		WAPG_API_trace("%s(%d) %s : document_name is not member.\n", __FILE__, __LINE__, __FUNCTION__);
		return WAPD_AnyError;
	}

	return WAPD_NoError;
}

/**
 * @brief ドキュメント削除実行結果通知.
 * @param[in] si32_BoxId ボックスID.
 * @param[in] si32_DocId ドキュメントID.
 * @param[in] si32_Rtn 処理結果.
 * @param[in] ui64_ReqId 非同期リクエストID.
 * @return なし.
 */
void WAPC_UserBoxDocumentController::deleteDocumentCompleted( int32_t si32_BoxId, int32_t si32_DocId, int32_t si32_Rtn, uint64_t ui64_ReqId )
{
	void* vp_Handle = NULL;
	int32_t si32_cbStatus = WAPD_AnyError;

	// 非同期結果格納クラス取り出し.
	WAPC_DeleteDocumentResult* pc_AsyncResult = NULL;
	pc_AsyncResult = dynamic_cast<WAPC_DeleteDocumentResult*>(WAPC_AsyncResponseContainer::newInstance()->getById( ui64_ReqId ));
	if( NULL != pc_AsyncResult ) {
		pc_AsyncResult->setDeleteResult( si32_Rtn );
		vp_Handle = pc_AsyncResult->getHandle();
		si32_cbStatus = WAPD_NoError;
	}

	// 非同期レスポンス用タスクにスイッチ.
	WAPC_AsyncResponder::newInstance()->postMessage( vp_Handle, si32_cbStatus );
}

/**
 * @brief ドキュメント削除コールバック関数ポインタ取得.
 * @retval NULL以外 コールバック関数ポインタ.
 * @retval NULL 空き関数なし.
 */
FP_ANS_DELETE_DOC WAPC_UserBoxDocumentController::getDeleteDocumentCb()
{
	FP_ANS_DELETE_DOC fp_Cb = NULL;

	for( uint32_t ui32_Index = 0; ui32_Index < (sizeof(g_s_DeleteDocCbTable) / sizeof(g_s_DeleteDocCbTable[0])); ++ui32_Index ) {
		if( False == g_s_DeleteDocCbTable[ui32_Index].b_InUse ) {
			fp_Cb = g_s_DeleteDocCbTable[ui32_Index].fp_Cb;
			g_s_DeleteDocCbTable[ui32_Index].b_InUse = True;
			break;
		}
	}
	return fp_Cb;
}

/**
 * @brief ドキュメント削除コールバック関数ポインタ解放.
 * @param[in] fp_Cb コールバック関数ポインタ.
 * @return なし.
 */
void WAPC_UserBoxDocumentController::releaseDeleteDocumentCb( FP_ANS_DELETE_DOC fp_Cb )
{
	for( uint32_t ui32_Index = 0; ui32_Index < (sizeof(g_s_DeleteDocCbTable) / sizeof(g_s_DeleteDocCbTable[0])); ++ui32_Index ) {
		if( fp_Cb == g_s_DeleteDocCbTable[ui32_Index].fp_Cb ) {
			g_s_DeleteDocCbTable[ui32_Index].b_InUse = False;
			break;
		}
	}
}

/**
 * @brief ドキュメント削除 非同期レスポンス生成.
 * @param[in] ps_Params HTTPリクエストヘッダ、ボディ
 * @retval WAPD_NoError(0) 成功.
 * @retval WAPD_AnyError(1) 失敗.
 */
int32_t WAPC_UserBoxDocumentController::generateDeleteDocumentResponse( WAPS_SubController *ps_Params )
{
	// 非同期結果クラス取り出し.
	WAPC_DeleteDocumentResult* pc_AsyncResult = NULL;
	pc_AsyncResult = dynamic_cast<WAPC_DeleteDocumentResult*>(WAPC_AsyncResponseContainer::newInstance()->getByHandle( m_pv_Handle ));
	if( NULL == pc_AsyncResult ) {
		WAPG_API_trace("%s(%d) %s : WAPC_AsyncResponseContainer::getByHandle failed.\n", __FILE__, __LINE__, __FUNCTION__);
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		generateErrorResponseBody( WAPE_BoxError_None );
		return WAPD_AnyError;
	}

	// 結果取得.
	int32_t si32_DeleteResult = APID_DocapiMethodOk;
	si32_DeleteResult = pc_AsyncResult->getDeleteResult();

	// 非同期結果クラス・関数ポインタ片付け.
	WAPC_AsyncResponseContainer::newInstance()->erase( pc_AsyncResult->getReqId() );
	releaseDeleteDocumentCb( reinterpret_cast<FP_ANS_DELETE_DOC>(pc_AsyncResult->getReqId()) );
	delete pc_AsyncResult;

	// 結果チェック.
	switch( si32_DeleteResult ) {
		case APID_DocapiMethodOk:
			m_Response = new Json::Value;
			(*m_Response)["delete_doc_no"] = static_cast<Json::Int>(m_si32_DocNo);
			m_ResponseHeader->Status = WAPD_STATUS_OK;
			break;

		default:
			WAPG_API_trace("%s(%d) %s : document delete failed.\n", __FILE__, __LINE__, __FUNCTION__);
			m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
			printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
			generateErrorResponseBody( WAPE_BoxError_None );
			return WAPD_AnyError;
	}

	return WAPD_NoError;
}

/**
 * @brief クエリーパラメータからリスト取得開始位置を取得する
 * @param[in] ps_Params HTTPリクエストヘッダ、ボディ
 * @param[out]	si32_Offset リスト取得開始位置
 * @return 処理の成否, WAPD_NoError(0)：成功、WAPD_AnyError(1)：失敗
 */
int32_t WAPC_UserBoxDocumentController::getOffsetFromQuery( WAPS_SubController *params, int32_t& si32_Offset )
{
	int32_t si32_Ret = WAPD_AnyError;
	if( NULL == params->Search ) {
		WAPG_API_trace("%s(%d) %s : params->Search is NULL.\n", __FILE__, __LINE__, __FUNCTION__);
		return WAPD_AnyError;
	}

	Json::Value& val_Ref = *params->Search;
	if( ! val_Ref.isObject() ) {
		WAPG_API_trace("%s(%d) %s : val_Ref is not object.\n", __FILE__, __LINE__, __FUNCTION__);
		return WAPD_AnyError;
	}

	// 初期値設定
	si32_Offset = WAPD_UserBoxDocumentListOffsetDefault;

	if( val_Ref.isMember( "offset" ) ) {
		if( ! val_Ref["offset"].isString() ) {
			WAPG_API_trace("%s(%d) %s : offset is not string.\n", __FILE__, __LINE__, __FUNCTION__);
			m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
			printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
			generateErrorResponseBody( WAPE_BoxError_None );
			return WAPD_AnyError;
		}

		std::string str_Offset = val_Ref["offset"].asString();
		int32_t si32_TempOffset = 0;
		int32_t si32_TempOffset_Max = 0;  //2019-08-14【EagleH】 RQ-560 対応 Hcl-Rajeshwari start.
		uint32_t ui32_length = 0;
		si32_Ret = stoi( str_Offset, si32_TempOffset, ui32_length );
		if( WAPD_NoError != si32_Ret ) {
			WAPG_API_trace("%s(%d) %s : stoi failed.\n", __FILE__, __LINE__, __FUNCTION__);
			printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
			m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
			generateErrorResponseBody( WAPE_BoxError_InvalidOffset );
			return WAPD_AnyError;
		}

		// 範囲チェック.
 //2019-08-14【EagleH】 RQ-560 対応 Hcl-Rajeshwari start.
		TYPE_StorageConfig e_StorageConfig = TYPE_StorageConfig_None;
        e_StorageConfig = APIC_SystemInfo::newInstance()->isStorageConfiguration();
        if(TYPE_StorageConfig_SSD_1TB == e_StorageConfig){
     		si32_TempOffset_Max = WAPD_UserBoxDocumentListOffsetMax_1T;
        }else{
     		si32_TempOffset_Max = WAPD_UserBoxDocumentListOffsetMax;	
     	}
		//if( ( WAPD_UserBoxDocumentListOffsetMin > si32_TempOffset ) || ( WAPD_UserBoxDocumentListOffsetMax < si32_TempOffset ) ) {	
		if( ( WAPD_UserBoxDocumentListOffsetMin > si32_TempOffset ) || ( si32_TempOffset_Max < si32_TempOffset ) ) {  //2019-08-14【EagleH】 RQ-560 対応 Hcl-Rajeshwari end.
			WAPG_API_trace("%s(%d) %s : invalid offset range.\n", __FILE__, __LINE__, __FUNCTION__);
			printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
			m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
			generateErrorResponseBody( WAPE_BoxError_InvalidOffset );
			return WAPD_AnyError;
		}
		si32_Offset = si32_TempOffset;
	}

	return WAPD_NoError;
}

/**
 * @brief クエリーパラメータからリスト取得数を取得する
 * @param[in] ps_Params HTTPリクエストヘッダ、ボディ
 * @param[out]	si32_Offset リスト取得開始位置
 * @return 処理の成否, WAPD_NoError(0)：成功、WAPD_AnyError(1)：失敗
 */
int32_t WAPC_UserBoxDocumentController::getNumFromQuery( WAPS_SubController *params, int32_t& si32_Num )
{
	int32_t si32_Ret = WAPD_AnyError;
	if( NULL == params->Search ) {
		WAPG_API_trace("%s(%d) %s : params->Search is NULL.\n", __FILE__, __LINE__, __FUNCTION__);
		return WAPD_AnyError;
	}

	Json::Value& val_Ref = *params->Search;
	if( ! val_Ref.isObject() ) {
		WAPG_API_trace("%s(%d) %s : val_Ref is not object.\n", __FILE__, __LINE__, __FUNCTION__);
		return WAPD_AnyError;
	}

	// 初期値設定
	si32_Num = WAPD_UserBoxDocumentListNumDefault;

	if( val_Ref.isMember( "num" ) ) {
		if( ! val_Ref["num"].isString() ) {
			WAPG_API_trace("%s(%d) %s : num is not string.\n", __FILE__, __LINE__, __FUNCTION__);
			m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
			printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
			generateErrorResponseBody( WAPE_BoxError_None );
			return WAPD_AnyError;
		}

		std::string str_Num = val_Ref["num"].asString();
		int32_t si32_TempNum = 0;
		int32_t si32_TempNum_Max = 0;  //2019-08-14【EagleH】 RQ-560 対応 Hcl-Rajeshwari.
		uint32_t ui32_length = 0;
		si32_Ret = stoi( str_Num, si32_TempNum, ui32_length );
		if( WAPD_NoError != si32_Ret ) {
			WAPG_API_trace("%s(%d) %s : stoi failed.\n", __FILE__, __LINE__, __FUNCTION__);
			printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
			m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
			generateErrorResponseBody( WAPE_BoxError_InvalidNum );
			return WAPD_AnyError;
		}

		// 範囲チェック.
 //2019-08-14【EagleH】 RQ-560 対応 Hcl-Rajeshwari start.
		TYPE_StorageConfig e_StorageConfig = TYPE_StorageConfig_None;
        e_StorageConfig = APIC_SystemInfo::newInstance()->isStorageConfiguration();
        if(TYPE_StorageConfig_SSD_1TB == e_StorageConfig){
     		si32_TempNum_Max = WAPD_UserBoxDocumentListNumMax_1T;
        }else{
     		si32_TempNum_Max = WAPD_UserBoxDocumentListNumMax;	
     	}
		//if( ( WAPD_UserBoxDocumentListNumMin > si32_TempNum ) || ( WAPD_UserBoxDocumentListNumMax < si32_TempNum ) ) {
		if( ( WAPD_UserBoxDocumentListNumMin > si32_TempNum ) || ( si32_TempNum_Max < si32_TempNum ) ) {  //2019-08-14【EagleH】 RQ-560 対応 Hcl-Rajeshwari end.
			WAPG_API_trace("%s(%d) %s : invalid num range.\n", __FILE__, __LINE__, __FUNCTION__);
			printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
			m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
			generateErrorResponseBody( WAPE_BoxError_InvalidNum );
			return WAPD_AnyError;
		}
		si32_Num = si32_TempNum;
	}

	return WAPD_NoError;
}

/**
 * @brief コンストラクタ.
 * @return なし.
 */
WAPC_GetDocumentListResult::WAPC_GetDocumentListResult()
{
	m_c_DocListInfo.listOrder			= 0;
	m_c_DocListInfo.listCnt				= 0;
	m_c_DocListInfo.listSort			= 0x01;
	m_c_DocListInfo.listMode			= 0x01;
	m_c_DocListInfo.function			= 0;
	m_c_DocListInfo.index				= 0;
	m_c_DocListInfo.userId				= 0;
	m_c_DocListInfo.pList				= &m_c_DocListTbl;
	m_c_DocListInfo.selectSecretDocID	= NULL;
	m_c_DocListInfo.selectSecretDocCnt	= 0;
	m_c_DocListInfo.Apic_BoxAddr		= NULL;
	m_c_DocListInfo.secretDocLoginMode	= False;
	m_c_DocListInfo.secretDocNgDefCnt	= 0;
	memset( m_c_DocListInfo.secret_DocId, 0, sizeof(m_c_DocListInfo.secret_DocId) );
	memset( m_c_DocListInfo.secret_Passwd, 0, sizeof(m_c_DocListInfo.secret_Passwd) );
	memset( m_c_DocListInfo.sc_PinCode, 0, sizeof(m_c_DocListInfo.sc_PinCode) );
}
