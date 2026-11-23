#include <iostream>
#include <NIF_defs.h>
#include <NIF_SystemInfo.h>
#include "WAP_MainController.h"
#include "WAP_plugintrace.h"
#include "WAP_UserBox.h"
#include "WAP_AsyncResponder.h"
#include "WAP_CommonTables.h"
#include "WAP_Power.h"

#define	WAPD_NoSetting	(2)			// 該当のキーワードが設定されていない.

static const int32_t WAPD_GetListMinOffset		= 1;
static const int32_t WAPD_GetListMaxOffset		= 1000;
static const int32_t WAPD_GetListMinNum			= 1;
static const int32_t WAPD_GetListMaxNum			= 1000;
static const int32_t WAPD_GetListInitNum		= 10;			// ボックスリスト最大取得数 初期値.
static const char    WAPD_SelectNoSetting 		= -1;			// 親展受信機能可否 初期値.

// 処理種別.
enum WAPE_FuncKind {
	WAPE_FuncKindNone = 0,
	WAPE_FuncKindCreateUserBox,
	WAPE_FuncKindGetUserBoxList,
	WAPE_FuncKindDeleteUserBox,
	WAPE_FuncKindGetUserBoxInformation,
	WAPE_FuncKindOpenUserBox,
	WAPE_FuncKindSetUserBoxInformation
};

// 処理変換用テーブル.
static const struct{
	int32_t			si32_BoxReq;	// WebAPI種別.
	uint32_t		ui32_Method;	// メソッド.
	int32_t			si32_Funckind;	// 処理タイプ.
} g_s_ConvertTable[]={
	{	WAPE_BoxReqUriBoxes,		WAPD_HTTP_POST,		WAPE_FuncKindCreateUserBox			},
	{	WAPE_BoxReqUriBoxes,		WAPD_HTTP_GET,		WAPE_FuncKindGetUserBoxList			},
	{	WAPE_BoxReqUriBoxesNo,		WAPD_HTTP_DELETE,	WAPE_FuncKindDeleteUserBox			},
	{	WAPE_BoxReqUriBoxesNoInfo,	WAPD_HTTP_GET,		WAPE_FuncKindGetUserBoxInformation	},
	{	WAPE_BoxReqUriBoxesNo,		WAPD_HTTP_PUT,		WAPE_FuncKindOpenUserBox			},
	{	WAPE_BoxReqUriBoxesNoInfo,	WAPD_HTTP_PUT,		WAPE_FuncKindSetUserBoxInformation	}
};

// コールバック関数.
static void WAPC_UserBoxController_ans_deleteBox01( int32_t si32_BoxId, int32_t si32_Result ){ WAPC_UserBoxController::BoxDeleteCompleted( si32_BoxId, si32_Result, reinterpret_cast<uint64_t>(WAPC_UserBoxController_ans_deleteBox01) ); }
static void WAPC_UserBoxController_ans_deleteBox02( int32_t si32_BoxId, int32_t si32_Result ){ WAPC_UserBoxController::BoxDeleteCompleted( si32_BoxId, si32_Result, reinterpret_cast<uint64_t>(WAPC_UserBoxController_ans_deleteBox02) ); }
static void WAPC_UserBoxController_ans_deleteBox03( int32_t si32_BoxId, int32_t si32_Result ){ WAPC_UserBoxController::BoxDeleteCompleted( si32_BoxId, si32_Result, reinterpret_cast<uint64_t>(WAPC_UserBoxController_ans_deleteBox03) ); }
static void WAPC_UserBoxController_ans_deleteBox04( int32_t si32_BoxId, int32_t si32_Result ){ WAPC_UserBoxController::BoxDeleteCompleted( si32_BoxId, si32_Result, reinterpret_cast<uint64_t>(WAPC_UserBoxController_ans_deleteBox04) ); }
static void WAPC_UserBoxController_ans_deleteBox05( int32_t si32_BoxId, int32_t si32_Result ){ WAPC_UserBoxController::BoxDeleteCompleted( si32_BoxId, si32_Result, reinterpret_cast<uint64_t>(WAPC_UserBoxController_ans_deleteBox05) ); }
static void WAPC_UserBoxController_ans_deleteBox06( int32_t si32_BoxId, int32_t si32_Result ){ WAPC_UserBoxController::BoxDeleteCompleted( si32_BoxId, si32_Result, reinterpret_cast<uint64_t>(WAPC_UserBoxController_ans_deleteBox06) ); }
static void WAPC_UserBoxController_ans_deleteBox07( int32_t si32_BoxId, int32_t si32_Result ){ WAPC_UserBoxController::BoxDeleteCompleted( si32_BoxId, si32_Result, reinterpret_cast<uint64_t>(WAPC_UserBoxController_ans_deleteBox07) ); }
static void WAPC_UserBoxController_ans_deleteBox08( int32_t si32_BoxId, int32_t si32_Result ){ WAPC_UserBoxController::BoxDeleteCompleted( si32_BoxId, si32_Result, reinterpret_cast<uint64_t>(WAPC_UserBoxController_ans_deleteBox08) ); }
static void WAPC_UserBoxController_ans_deleteBox09( int32_t si32_BoxId, int32_t si32_Result ){ WAPC_UserBoxController::BoxDeleteCompleted( si32_BoxId, si32_Result, reinterpret_cast<uint64_t>(WAPC_UserBoxController_ans_deleteBox09) ); }
static void WAPC_UserBoxController_ans_deleteBox10( int32_t si32_BoxId, int32_t si32_Result ){ WAPC_UserBoxController::BoxDeleteCompleted( si32_BoxId, si32_Result, reinterpret_cast<uint64_t>(WAPC_UserBoxController_ans_deleteBox10) ); }

// コールバック関数テーブル.
static struct {
	FP_ANS_CHANGE_BOX_STATUS fp_Cb;
	Bool b_InUse;
} g_s_CbTable[] = {
	{	WAPC_UserBoxController_ans_deleteBox01,	False	},
	{	WAPC_UserBoxController_ans_deleteBox02,	False	},
	{	WAPC_UserBoxController_ans_deleteBox03,	False	},
	{	WAPC_UserBoxController_ans_deleteBox04,	False	},
	{	WAPC_UserBoxController_ans_deleteBox05,	False	},
	{	WAPC_UserBoxController_ans_deleteBox06,	False	},
	{	WAPC_UserBoxController_ans_deleteBox07,	False	},
	{	WAPC_UserBoxController_ans_deleteBox08,	False	},
	{	WAPC_UserBoxController_ans_deleteBox09,	False	},
	{	WAPC_UserBoxController_ans_deleteBox10,	False	}
};

// ボックス種別・所有者変更権限(組禁)テーブル.
// 該当すればNG.
static struct {
	WAPE_BoxUserType e_OperatorType;
	WAPE_BoxUserType e_SrcBoxOwnerType;
	WAPE_BoxUserType e_DstBoxOwnerType;
	WAPE_PasswordRules e_PassWordRules;
	WAPE_UserAuthentication e_UserAuth;
} g_s_TypeAndOwnerValidate[] = {
	{	WAPE_BoxUserType_None,		WAPE_BoxUserType_None,		WAPE_BoxUserType_BoxAdmin,	WAPE_PasswordRules_None,		WAPE_UserAuthentication_None				},	// ボックス管理者への所有者変更は全面的に不可.
	{	WAPE_BoxUserType_Public,	WAPE_BoxUserType_None,		WAPE_BoxUserType_None,		WAPE_PasswordRules_None,		WAPE_UserAuthentication_None				},	// パブリックユーザーによる変更は全面的に不可.
	{	WAPE_BoxUserType_User_Mfp,	WAPE_BoxUserType_Public,	WAPE_BoxUserType_User_Mid,	WAPE_PasswordRules_None,		WAPE_UserAuthentication_None				},
	{	WAPE_BoxUserType_User_Mfp,	WAPE_BoxUserType_User_Mfp,	WAPE_BoxUserType_Public,	WAPE_PasswordRules_On_Invalid,	WAPE_UserAuthentication_None				},
	{	WAPE_BoxUserType_User_Mfp,	WAPE_BoxUserType_User_Mfp,	WAPE_BoxUserType_User_Mid,	WAPE_PasswordRules_None,		WAPE_UserAuthentication_None				},
	{	WAPE_BoxUserType_User_Mfp,	WAPE_BoxUserType_Track,		WAPE_BoxUserType_Public,	WAPE_PasswordRules_On_Invalid,	WAPE_UserAuthentication_None				},
	{	WAPE_BoxUserType_User_Mfp,	WAPE_BoxUserType_Track,		WAPE_BoxUserType_User_Mid,	WAPE_PasswordRules_None,		WAPE_UserAuthentication_None				},
	{	WAPE_BoxUserType_User_Ext,	WAPE_BoxUserType_User_Ext,	WAPE_BoxUserType_Public,	WAPE_PasswordRules_On_Invalid,	WAPE_UserAuthentication_None				},
	{	WAPE_BoxUserType_User_Ext,	WAPE_BoxUserType_Track,		WAPE_BoxUserType_Public,	WAPE_PasswordRules_On_Invalid,	WAPE_UserAuthentication_None				},
	{	WAPE_BoxUserType_User_Mid,	WAPE_BoxUserType_Public,	WAPE_BoxUserType_User_Mfp,	WAPE_PasswordRules_None,		WAPE_UserAuthentication_None				},
	{	WAPE_BoxUserType_User_Mid,	WAPE_BoxUserType_Public,	WAPE_BoxUserType_Track,		WAPE_PasswordRules_None,		WAPE_UserAuthentication_None				},
	{	WAPE_BoxUserType_User_Mid,	WAPE_BoxUserType_User_Mid,	WAPE_BoxUserType_Public,	WAPE_PasswordRules_On_Invalid,	WAPE_UserAuthentication_None				},
	{	WAPE_BoxUserType_User_Mid,	WAPE_BoxUserType_User_Mid,	WAPE_BoxUserType_User_Mfp,	WAPE_PasswordRules_None,		WAPE_UserAuthentication_None				},
	{	WAPE_BoxUserType_User_Mid,	WAPE_BoxUserType_User_Mid,	WAPE_BoxUserType_Track,		WAPE_PasswordRules_None,		WAPE_UserAuthentication_None				},
	{	WAPE_BoxUserType_Track,		WAPE_BoxUserType_Track,		WAPE_BoxUserType_Public,	WAPE_PasswordRules_On_Invalid,	WAPE_UserAuthentication_None				},
	{	WAPE_BoxUserType_BoxAdmin,	WAPE_BoxUserType_Public,	WAPE_BoxUserType_User_Mid,	WAPE_PasswordRules_None,		WAPE_UserAuthentication_None				},
	{	WAPE_BoxUserType_BoxAdmin,	WAPE_BoxUserType_User_Mfp,	WAPE_BoxUserType_Public,	WAPE_PasswordRules_On_Invalid,	WAPE_UserAuthentication_None				},
	{	WAPE_BoxUserType_BoxAdmin,	WAPE_BoxUserType_User_Mfp,	WAPE_BoxUserType_User_Mid,	WAPE_PasswordRules_None,		WAPE_UserAuthentication_None				},
	{	WAPE_BoxUserType_BoxAdmin,	WAPE_BoxUserType_User_Ext,	WAPE_BoxUserType_Public,	WAPE_PasswordRules_On_Invalid,	WAPE_UserAuthentication_None				},
	{	WAPE_BoxUserType_BoxAdmin,	WAPE_BoxUserType_User_Mid,	WAPE_BoxUserType_Public,	WAPE_PasswordRules_On_Invalid,	WAPE_UserAuthentication_None				},
	{	WAPE_BoxUserType_BoxAdmin,	WAPE_BoxUserType_User_Mid,	WAPE_BoxUserType_Public,	WAPE_PasswordRules_None,		WAPE_UserAuthentication_IntermeddiateServer	},
	{	WAPE_BoxUserType_BoxAdmin,	WAPE_BoxUserType_User_Mid,	WAPE_BoxUserType_User_Mid,	WAPE_PasswordRules_None,		WAPE_UserAuthentication_None				},
	{	WAPE_BoxUserType_BoxAdmin,	WAPE_BoxUserType_Track,		WAPE_BoxUserType_Public,	WAPE_PasswordRules_On_Invalid,	WAPE_UserAuthentication_None				},
	{	WAPE_BoxUserType_BoxAdmin,	WAPE_BoxUserType_Track,		WAPE_BoxUserType_User_Mid,	WAPE_PasswordRules_None,		WAPE_UserAuthentication_None				},
	{	WAPE_BoxUserType_BoxAdmin,	WAPE_BoxUserType_BoxAdmin,	WAPE_BoxUserType_Public,	WAPE_PasswordRules_On_Invalid,	WAPE_UserAuthentication_None				},
	{	WAPE_BoxUserType_BoxAdmin,	WAPE_BoxUserType_BoxAdmin,	WAPE_BoxUserType_User_Mid,	WAPE_PasswordRules_None,		WAPE_UserAuthentication_None				},
	{	WAPE_BoxUserType_Admin,		WAPE_BoxUserType_Public,	WAPE_BoxUserType_User_Mid,	WAPE_PasswordRules_None,		WAPE_UserAuthentication_None				},
	{	WAPE_BoxUserType_Admin,		WAPE_BoxUserType_User_Mfp,	WAPE_BoxUserType_Public,	WAPE_PasswordRules_On_Invalid,	WAPE_UserAuthentication_None				},
	{	WAPE_BoxUserType_Admin,		WAPE_BoxUserType_User_Mfp,	WAPE_BoxUserType_User_Mid,	WAPE_PasswordRules_None,		WAPE_UserAuthentication_None				},
	{	WAPE_BoxUserType_Admin,		WAPE_BoxUserType_User_Ext,	WAPE_BoxUserType_Public,	WAPE_PasswordRules_On_Invalid,	WAPE_UserAuthentication_None				},
	{	WAPE_BoxUserType_Admin,		WAPE_BoxUserType_User_Mid,	WAPE_BoxUserType_Public,	WAPE_PasswordRules_On_Invalid,	WAPE_UserAuthentication_None				},
	{	WAPE_BoxUserType_Admin,		WAPE_BoxUserType_User_Mid,	WAPE_BoxUserType_Public,	WAPE_PasswordRules_None,		WAPE_UserAuthentication_IntermeddiateServer	},
	{	WAPE_BoxUserType_Admin,		WAPE_BoxUserType_User_Mid,	WAPE_BoxUserType_User_Mid,	WAPE_PasswordRules_None,		WAPE_UserAuthentication_None				},
	{	WAPE_BoxUserType_Admin,		WAPE_BoxUserType_Track,		WAPE_BoxUserType_Public,	WAPE_PasswordRules_On_Invalid,	WAPE_UserAuthentication_None				},
	{	WAPE_BoxUserType_Admin,		WAPE_BoxUserType_Track,		WAPE_BoxUserType_User_Mid,	WAPE_PasswordRules_None,		WAPE_UserAuthentication_None				},
	{	WAPE_BoxUserType_Admin,		WAPE_BoxUserType_BoxAdmin,	WAPE_BoxUserType_Public,	WAPE_PasswordRules_On_Invalid,	WAPE_UserAuthentication_None				},
	{	WAPE_BoxUserType_Admin,		WAPE_BoxUserType_BoxAdmin,	WAPE_BoxUserType_User_Mid,	WAPE_PasswordRules_None,		WAPE_UserAuthentication_None				},
};

/**
 * @brief コンストラクタ
 * @return なし
 */
WAPC_UserBoxController::WAPC_UserBoxController()
{
}

/**
 * @brief コンストラクタ
 * @return なし
 */
WAPC_UserBoxController::WAPC_UserBoxController( int32_t si32_BoxReq, int32_t si32_BoxNo, int32_t si32_DocNo )
{
	m_si32_BoxReq = si32_BoxReq;
	m_si32_BoxNo  = si32_BoxNo;
	m_si32_DocNo  = si32_DocNo;
}

/**
 * @brief デストラクタ
 * @return なし
 */
WAPC_UserBoxController::~WAPC_UserBoxController()
{
}

/**
 * @brief HTTPレスポンスの生成
 * @param[in] params HTTPリクエストヘッダ、ボディ
 * @return 警告情報取得の成否, WAPD_NoError(0)：成功、WAPD_AnyError(1)：失敗
 */
int32_t WAPC_UserBoxController::ResponseGenerate(WAPS_SubController *params)
{
	int32_t si32_Ret = WAPD_AnyError;
	uint32_t ui32_funckind = WAPE_FuncKindNone;
	m_ResponseHeader->Status = WAPD_STATUS_OK;
	Bool b_SendBodyOnHttp20X = False;

	//パラメータチェック.
	if( NULL == params ){
		WAPG_API_trace("%s(%d) %s : params is NULL\n", __FILE__, __LINE__, __FUNCTION__);
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		generateErrorResponseBody( WAPE_BoxError_None );
		si32_Ret = WAPD_AnyError;
		goto LABEL_END;
	}

	for( uint32_t ui32_loop = 0; ui32_loop < (sizeof(g_s_ConvertTable) / sizeof(g_s_ConvertTable[0])); ui32_loop++ )
	{
		if( g_s_ConvertTable[ui32_loop].si32_BoxReq == m_si32_BoxReq )
		{
			if( g_s_ConvertTable[ui32_loop].ui32_Method == params->Method )
			{
				ui32_funckind = g_s_ConvertTable[ui32_loop].si32_Funckind;
			}
		}
	}

	// 機能制限確認.
	if ( WAPE_FuncKindNone != ui32_funckind ) {
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
	{
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

	switch(ui32_funckind){
		// ボックス生成.
		case WAPE_FuncKindCreateUserBox:
			// Create user BOX.
			WAPD_DEBUG("Create user BOX start.\n");
			si32_Ret = createUserBox( params );
			b_SendBodyOnHttp20X = True;
			break;

		// ボックスリスト取得.
		case WAPE_FuncKindGetUserBoxList:
			// Get user BOX list.
			WAPD_DEBUG("Get user BOX list start.\n");
			si32_Ret = getUserBoxList( params );
			b_SendBodyOnHttp20X = True;
			break;

		// ボックスアクセス権取得.
		case WAPE_FuncKindOpenUserBox:
			// Open user BOX.
			WAPD_DEBUG("Open user BOX start.\n");
			si32_Ret = openUserBox( params );
			break;

		// ボックス削除(非同期).
		case WAPE_FuncKindDeleteUserBox:
			// Delete user BOX.
			WAPD_DEBUG("Delete user BOX start.\n");
			si32_Ret = deleteUserBox( params );
			break;

		// ボックス情報取得.
		case WAPE_FuncKindGetUserBoxInformation:
			// Get user BOX information.
			WAPD_DEBUG("Get user BOX information start.\n");
			si32_Ret = getUserBoxInformation( params );
			b_SendBodyOnHttp20X = True;
			break;

		// ボックス情報設定.
		case WAPE_FuncKindSetUserBoxInformation:
			// Set user BOX information.
			WAPD_DEBUG("Set user BOX information start.\n");
			si32_Ret = setUserBoxInformation( params );
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
	}
LABEL_END:
	switch( m_ResponseHeader->Status ) {
		// 200系.
		case WAPD_STATUS_OK:
		case WAPD_STATUS_CREATED:
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
			m_ResponseHeader->ContentType = const_cast<char*>("application/json");
			m_ResponseHeader->ContentTypeLength = sizeof("application/json") - 1;
			break;

		// 500系(サーバー要因).
		default:
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
int32_t WAPC_UserBoxController::AsyncResponseGenerate( WAPS_SubController *params )
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
	switch( si32_Funckind ) {
		// ボックス削除.
		case WAPE_FuncKindDeleteUserBox:
			WAPD_DEBUG("generateBoxDeleteResponse start.\n");
			si32_Ret = generateBoxDeleteResponse( params );
			b_SendBodyOnHttp20X = True;
			break;

		default:
			WAPG_API_trace("%s(%d) %s : Invalid Method.\n", __FILE__, __LINE__, __FUNCTION__);
			printf("405:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //405->ErrorStatus[WAPD_STATUS_METHOD_NOT_ALLOWED] in WAP_defs.h
			m_ResponseHeader->Status = WAPD_STATUS_METHOD_NOT_ALLOWED;
			generateErrorResponseBody( WAPE_BoxError_InvalidMethod );
			si32_Ret = WAPD_AnyError;
			break;
	}

LABEL_END:
	switch( m_ResponseHeader->Status ) {
		case WAPD_STATUS_OK:
			if( True == b_SendBodyOnHttp20X ) {
				m_ResponseHeader->ContentType = const_cast<char*>("application/json");
				m_ResponseHeader->ContentTypeLength = sizeof("application/json") - 1;
			}
			break;

		case WAPD_STATUS_BAD_REQUEST:
		case WAPD_STATUS_FORBIDDEN:
		case WAPD_STATUS_NOT_FOUND:
		case WAPD_STATUS_METHOD_NOT_ALLOWED:
			m_ResponseHeader->ContentType = const_cast<char*>("application/json");
			m_ResponseHeader->ContentTypeLength = sizeof("application/json") - 1;
			break;

		default:
			break;
	}
	return si32_Ret;
}

/**
 * @brief ユーザーボックス生成
 * @param[in] param サブコントローラー
 * @return 処理の成否, WAPD_NoError(0)：成功、WAPD_AnyError(1)：失敗
 */
int32_t WAPC_UserBoxController::createUserBox( WAPS_SubController *params )
{
	int32_t si32_Ret = WAPD_AnyError;
	// 前提条件チェック.
	si32_Ret = checkBoxCreateReqStatus();
	if( WAPD_FatalError == si32_Ret ){
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		generateErrorResponseBody( WAPE_BoxError_None );
		return WAPD_AnyError;
	} else if( WAPD_AnyError == si32_Ret ){
		printf("503:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //503->ErrorStatus[WAPD_STATUS_SERVICE_UNAVAILABLE] in WAP_defs.h
		m_ResponseHeader->Status = WAPD_STATUS_SERVICE_UNAVAILABLE;
		generateErrorResponseBody( WAPE_BoxError_None );
		return WAPD_AnyError;
	} else {
		// スコアチェック対応.
	}

	// ボックス番号取得.
	int32_t si32_BoxNo = 0;
	si32_Ret = getBoxNoFromBody( params, si32_BoxNo );
	if( WAPD_NoError != si32_Ret ) {
		return WAPD_AnyError;
	}

	// ボックス種別取得.
	APIE_BoxOptionCreateInfo e_BoxType = APIE_BoxmakerAuto;
	si32_Ret = getBoxTypeFromBody( params, e_BoxType );
	if( WAPD_NoError != si32_Ret ) {
		return WAPD_AnyError;
	}

	// ボックスパスワード取得.
	char sc_BoxPassword[WAPD_BoxPasswordMaxLength] = {0};
	si32_Ret = getBoxPasswordFromBody( params, sc_BoxPassword, e_BoxType );
	if( WAPD_NoError != si32_Ret ) {
		return WAPD_AnyError;
	}

	// 各パラメータ作成.
	// ユーザー番号の初期値設定.
	uint32_t ui32_UserNo = (uint32_t)params->UserNumber;
	// 管理番号の初期値設定.
	uint32_t ui32_TrackNo = (uint32_t)params->TrackNumber;

	// 部門認証のみ有.
	if( ( TYPD_IndexUserPublic == params->UserNumber ) && ( TYPD_NotSetting != params->TrackNumber ) ){
		if(e_BoxType == APIE_BoxmakerUser){
			WAPG_API_trace("%s(%d) %s : Illegal box type\n", __FILE__, __LINE__, __FUNCTION__);
			printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
			m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
			generateErrorResponseBody( WAPE_BoxError_InvalidBoxKind );
			return WAPD_AnyError;
        }

// 部門管理のみ有の場合は.
// params->UserNumberにTYPD_IndexUserPublicが入る仕様に変更となったため.
// 以下の処理は不要。よってコメントアウトする.
//		ui32_UserNo = TYPD_IndexUserPublic;

	// ユーザ認証有(Public以外).
	} else if( ( 0 != params->UserNumber ) && ( TYPD_IndexUserPublic != params->UserNumber ) ) {
		if ( TYPD_NotSetting == params->TrackNumber) {
			if( APIE_BoxmakerGroup == e_BoxType ) {
				WAPG_API_trace("%s(%d) %s : Illegal box type\n", __FILE__, __LINE__, __FUNCTION__);
				printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
				m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
				generateErrorResponseBody( WAPE_BoxError_InvalidBoxKind );
				return WAPD_AnyError;
	        }
		}

	// 未認証、又はユーザ認証有(Public).
	} else {
        if( ( APIE_BoxmakerGroup == e_BoxType ) || ( APIE_BoxmakerUser == e_BoxType ) ) {
			WAPG_API_trace("%s(%d) %s : Illegal box type\n", __FILE__, __LINE__, __FUNCTION__);
			printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
			m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
			generateErrorResponseBody( WAPE_BoxError_InvalidBoxKind );
			return WAPD_AnyError;
        }

// 未認証、又はユーザ認証有(Public)の場合は.
// params->UserNumberにTYPD_IndexUserPublicが入る仕様に変更となったため.
// 以下の処理は不要。よってコメントアウトする.
//		if( 0 == params->UserNumber ) {
//			// パネル実装に合わせ未認証時はPublicを設定.
//			ui32_UserNo = TYPD_IndexUserPublic;
//		}
	}

	uint32_t ui32_OwnerId = 0;
	int32_t si32_CreateOption = APID_CreateUserBox;
    // ボックス種別が共有.
    if( APIE_BoxmakerAuto == e_BoxType) {
		ui32_OwnerId = APID_BoxUserID_Public;

	// ボックス種別が個人.
	} else if( APIE_BoxmakerUser == e_BoxType ) {
		// Adminは個人ボックスを所有できない.
		if( TYPD_IndexUserAdmin == params->UserNumber ) {
			WAPG_API_trace("%s(%d) %s : Illegal box type\n", __FILE__, __LINE__, __FUNCTION__);
			printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
			m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
			generateErrorResponseBody( WAPE_BoxError_InvalidBoxKind );
			return WAPD_AnyError;
		}
        ui32_TrackNo = 0;

	// ボックス種別がグループ.
	} else if ( APIE_BoxmakerGroup == e_BoxType ) {
 		// Adminはグループボックスを所有できない.
 		// BoxAdminはグループボックスを所有できるが、作成は自動生成からのみで、WebAPIからは不可とする.
		if( ( TYPD_IndexUserAdmin == params->UserNumber ) || ( TYPD_IndexUserBoxAdmin == params->UserNumber ) ) {
			WAPG_API_trace("%s(%d) %s : Illegal box type\n", __FILE__, __LINE__, __FUNCTION__);
			printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
			m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
			generateErrorResponseBody( WAPE_BoxError_InvalidBoxKind );
			return WAPD_AnyError;
		}
		si32_CreateOption = APID_CreateGroupBox;
    	ui32_OwnerId = ui32_TrackNo;
	} else {
		// スコアチェック対応.
	}

	// APIC_BoxRegistryインスタンス取得.
	APIC_BoxRegistry* pc_BoxRegistry = APIC_BoxRegistry::newInstance();
	if(NULL == pc_BoxRegistry){
		WAPG_API_trace("%s(%d) %s : APIC_BoxRegistry instance NULL \n", __FILE__, __LINE__, __FUNCTION__);
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		generateErrorResponseBody( WAPE_BoxError_None );
		return WAPD_AnyError;
	}
	// アクセス権情報設定.
	APIC_AccessTokenManager* pc_AccessTokenManager = APIC_AccessTokenManager::newInstance();
	JLGE_InspectLogIF e_InspectLogIF = JLGE_InspectLogIfOperationPanel;
	if( NULL != pc_AccessTokenManager){
		pc_AccessTokenManager->getAppType(m_AccessToken, &e_InspectLogIF);
	}
	pc_BoxRegistry->setLogIFNo( e_InspectLogIF );

	// ボックス作成.
	int32_t si32_ReturnValue = APID_DocapiCommonErr;
    si32_ReturnValue = pc_BoxRegistry->createBox(
		ui32_UserNo,			// ユーザー番号.
		ui32_TrackNo,			// 部門番号.
		sc_BoxPassword,			// ボックスパスワード.
		si32_BoxNo,				// ボックス番号.
		TYPD_DefaultNonBox,		// ボックス種別.
		si32_CreateOption,		// ボックス作成オプション.
		ui32_OwnerId,			// 作成グループ.
		TYPE_RegistrationTypeOnlyOne,			// 監査ログタイプ.
		(short)params->AuthorityID,				// 認証ID.
		params->CostCenterID);	// CostCenterID	.
	WAPD_DEBUG1("createBox() si32_ReturnValue = %d\n", si32_ReturnValue);
	// APIC_BoxRegistryインスタンス削除.
	pc_BoxRegistry->deleteInstance();

	switch(si32_ReturnValue){
		case APID_DocapiMethodOk:
			si32_Ret = WAPD_NoError;

			m_Response = new Json::Value;
			(*m_Response)["box_no"] = si32_BoxNo;
			m_ResponseHeader->Status = WAPD_STATUS_CREATED;
			break;

		case APID_DocapiBoxidRegistErr:		// 指定Box番号登録済みエラー.
			printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
			m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
			generateErrorResponseBody( WAPE_BoxError_BoxNoAlreadyExist );
			break;

		case APID_DocapiOwnerAccessErr:		// オーナー名不正エラー.
			printf("403:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //403->ErrorStatus[WAPD_STATUS_FORBIDDEN] in WAP_defs.h
			m_ResponseHeader->Status = WAPD_STATUS_FORBIDDEN;
			generateErrorResponseBody( WAPE_BoxError_OwnerAccessErr );
			break;

		case APID_DocapiUserBoxOverErr:		// ユーザボックス数上限オーバー.
		case APID_DocapiGroupBoxOverErr:	// グループボックス数上限オーバー.
		case APID_DocapiAllBoxOverErr:		// 全ボックス数上限オーバー.
			printf("503:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //503->ErrorStatus[WAPD_STATUS_SERVICE_UNAVAILABLE] in WAP_defs.h
			m_ResponseHeader->Status = WAPD_STATUS_SERVICE_UNAVAILABLE;
			generateErrorResponseBody( WAPE_BoxError_None );
			break;

		case APID_DocapiUnavailableBoxErr:	// ボックス使用禁止エラー.
		case APID_DocapiCommonErr:			// その他の処理失敗エラー.
		default:
			m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
			printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
			generateErrorResponseBody( WAPE_BoxError_None );
			break;
	}

	return si32_Ret;
}

/**
 * @brief ボックスリスト取得
 * @param[in] param サブコントローラー
 * @return 処理の成否, WAPD_NoError(0)：成功、WAPD_AnyError(1)：失敗
 */
int32_t WAPC_UserBoxController::getUserBoxList( WAPS_SubController *params )
{
	int32_t si32_Ret = WAPD_AnyError;
	// 前提条件チェック.
	si32_Ret = checkBoxInfoReqStatus();
	if( WAPD_AnyError == si32_Ret ){
		printf("503:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //503->ErrorStatus[WAPD_STATUS_SERVICE_UNAVAILABLE] in WAP_defs.h
		m_ResponseHeader->Status = WAPD_STATUS_SERVICE_UNAVAILABLE;
		generateErrorResponseBody( WAPE_BoxError_None );
		return WAPD_AnyError;
	}

	// ユーザーボックス検索値取得.
	TYPE_SearchKey e_SearchKey = TYPE_SearchKeyAA;
	si32_Ret = getSearchKeyFromQuery( params, e_SearchKey );
	if( WAPD_NoError != si32_Ret ) {
		return WAPD_AnyError;
	}

	// リスト取得開始位置取得.
	int32_t si32_Offset = 1;
	si32_Ret = getOffsetFromQuery( params, si32_Offset );
	if( WAPD_NoError != si32_Ret ) {
		return WAPD_AnyError;
	}

	// 最大取得数取得.
	int32_t si32_Num = WAPD_GetListInitNum;
	si32_Ret = getNumFromQuery( params, si32_Num );
	if( WAPD_NoError != si32_Ret ) {
		return WAPD_AnyError;
	}

	// 各パラメータ作成.
	// ユーザー、部門情報取得.
	APIC_User* pc_User = NULL;
    APIC_Track* pc_Track = NULL;
	getUserTrackInformation( params, &pc_User, &pc_Track );

	const int32_t BOX_LIST_REQ_OPTION = 8;	// 8：検索取得要求(User＆部門＆Public).
	LIBC_List<APIC_BoxList> lst_BoxList;

	// APIC_BoxRegistryインスタンス取得.
	APIC_BoxRegistry* pc_BoxRegistry = APIC_BoxRegistry::newInstance();
	if(NULL == pc_BoxRegistry){
		WAPG_API_trace("%s(%d) %s : APIC_BoxRegistry instance NULL \n", __FILE__, __LINE__, __FUNCTION__);
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		generateErrorResponseBody( WAPE_BoxError_None );
		return WAPD_AnyError;
	}

	// ボックス作成.
	int32_t si32_ReturnValue = APID_DocapiCommonErr;
    si32_ReturnValue = pc_BoxRegistry->getBoxList(
		pc_User,				// ユーザー情報.
		(si32_Offset - 1),		// リスト取得開始位置.
		si32_Num,				// 取得最大数.
		1,						// リストソート(1:昇順).
		BOX_LIST_REQ_OPTION,	// 要求オプション.
		e_SearchKey,			// ユーザーボックス検索値.
		&lst_BoxList,			// ボックス一覧結果リスト.
		False,					// 検索時の常用BOX指定.
		LIBE_CharSetUTF8,		// Box名文字コード変換識別.
		pc_Track);				// 部門情報.
	WAPD_DEBUG1("getBoxList() si32_ReturnValue = %d\n", si32_ReturnValue);
	// APIC_BoxRegistryインスタンス削除.
	pc_BoxRegistry->deleteInstance();

	std::list<int> lst_TempBoxlist;
	switch(si32_ReturnValue) {
		case APID_DocapiMethodOk:
			{
				lst_BoxList.Lock();
				APIC_BoxList *pc_boxlist = lst_BoxList.findFirst();

				if ( pc_boxlist == NULL ) {
					lst_BoxList.UnLock();
					// 何もない場合は 204 No Content.
					si32_Ret = WAPD_NoError;
					printf("204:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //204->ErrorStatus[WAPD_STATUS_NO_CONTENT] in WAP_defs.h
					m_ResponseHeader->Status = WAPD_STATUS_NO_CONTENT;
					break;
				}

				m_Response = new Json::Value;
				uint32_t ui32_Counter = 0;
				while( pc_boxlist != NULL )
				{
					(*m_Response)["list"][ui32_Counter++] = pc_boxlist->sl_BoxID;
					pc_boxlist = lst_BoxList.findNext();
				}
				lst_BoxList.UnLock();
				// リスト後片付け(APIC_BoxRegistryヘッダ参照).
				releaseBoxListFromBoxRegistry( lst_BoxList );

				si32_Ret = WAPD_NoError;
				m_ResponseHeader->Status = WAPD_STATUS_OK;
				break;
			}

		case APID_DocapiNoData:				// 取得データなし.
			si32_Ret = WAPD_NoError;
			m_ResponseHeader->Status = WAPD_STATUS_NO_CONTENT;
			break;

		case APID_DocapiOwnerAccessErr:		// オーナー名不正エラー.
			printf("403:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //403->ErrorStatus[WAPD_STATUS_FORBIDDEN] in WAP_defs.h
			m_ResponseHeader->Status = WAPD_STATUS_FORBIDDEN;
			generateErrorResponseBody( WAPE_BoxError_OwnerAccessErr );
			break;

		case APID_DocapiCommonErr:			// その他の処理失敗エラー.
		default:
			m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
			printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
			generateErrorResponseBody( WAPE_BoxError_None );
			break;
	}

	return si32_Ret;
}

/**
 * @brief ボックスアクセス権取得
 * @param[in] param サブコントローラー
 * @return 処理の成否, WAPD_NoError(0)：成功、WAPD_AnyError(1)：失敗
 */
int32_t WAPC_UserBoxController::openUserBox( WAPS_SubController *params )
{
	int32_t si32_Ret = WAPD_AnyError;
	
	// 前提条件チェック.
	si32_Ret = checkBoxOpenDelReqStatus();
	if( WAPD_FatalError == si32_Ret ){
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		generateErrorResponseBody( WAPE_BoxError_None );
		return WAPD_AnyError;
	} else if( WAPD_AnyError == si32_Ret ){
		printf("503:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //503->ErrorStatus[WAPD_STATUS_SERVICE_UNAVAILABLE] in WAP_defs.h
		m_ResponseHeader->Status = WAPD_STATUS_SERVICE_UNAVAILABLE;
		generateErrorResponseBody( WAPE_BoxError_None );
		return WAPD_AnyError;
	} else {
		// スコアチェック対応.
	}

	// ボックス存在チェック.
	Bool b_IsExist = False;
	si32_Ret = boxExists( m_si32_BoxNo, b_IsExist );
	if( WAPD_NoError != si32_Ret ) {
		return WAPD_AnyError;
	}
	if( False == b_IsExist ) {
		printf("404:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //404->ErrorStatus[WAPD_STATUS_NOT_FOUND] in WAP_defs.h
		m_ResponseHeader->Status = WAPD_STATUS_NOT_FOUND;
		generateErrorResponseBody( WAPE_BoxError_BoxNotExist );
		return WAPD_AnyError;
	}
	
	// ユーザー、部門情報取得.
	APIC_User* pc_User = NULL;
    APIC_Track* pc_Track = NULL;
	getUserTrackInformation( params, &pc_User, &pc_Track );

	// ボックス情報取得.
	// ボックスオプションを取得するため.
	APIC_Box* pc_Box = APIC_Box::newInstance();
	if(NULL == pc_Box){
		WAPG_API_trace("%s(%d) %s : APIC_Box instance NULL \n", __FILE__, __LINE__, __FUNCTION__);
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		generateErrorResponseBody( WAPE_BoxError_None );
		return WAPD_AnyError;
	}
	pc_Box->setBoxId( m_si32_BoxNo );

	int32_t si32_ReturnValue = APID_DocapiCommonErr;
	APIC_BoxInfo c_BoxInfo;       // 取得Info値.
	si32_ReturnValue = pc_Box->getBoxInfo(
		pc_User,
		&c_BoxInfo,
		(LIBE_CharSet)APID_DefaultCharSet,
		pc_Track);

	switch(si32_ReturnValue) {
		case APID_DocapiMethodOk:
			break;

		case APID_DocapiOwnerAccessErr:		// オーナー名不正エラー.
			WAPG_API_trace("%s(%d) %s : getBoxOption APID_DocapiOwnerAccessErr \n", __FILE__, __LINE__, __FUNCTION__);
			printf("403:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //403->ErrorStatus[WAPD_STATUS_FORBIDDEN] in WAP_defs.h
			m_ResponseHeader->Status = WAPD_STATUS_FORBIDDEN;
			generateErrorResponseBody( WAPE_BoxError_OwnerAccessErr );
			return WAPD_AnyError;

		case APID_DocapiBoxidUnknownErr:	// 指定番号のBoxなしエラー.
			WAPG_API_trace("%s(%d) %s : getBoxOption APID_DocapiBoxidUnknownErr \n", __FILE__, __LINE__, __FUNCTION__);
			printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
			m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
			generateErrorResponseBody( WAPE_BoxError_BoxNotExist );
			return WAPD_AnyError;

		case APID_DocapiCommonErr:			// その他の処理失敗エラー.
		default:
			WAPG_API_trace("%s(%d) %s : getBoxOption failed \n", __FILE__, __LINE__, __FUNCTION__);
			m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
			printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
			generateErrorResponseBody( WAPE_BoxError_None );
			return WAPD_AnyError;
	}

	// ボックスオプション取得.
	// ボックス種別を取得するため.
	APIC_BoxOption c_BoxOption;
	si32_ReturnValue = pc_Box->getBoxOption(
		pc_User,					// ユーザー情報.
		c_BoxInfo.sc_BoxPassword,			// ボックスパスワード.
		&c_BoxOption,				// ボックスオプション情報.
		pc_Track);					// 部門情報.

	switch(si32_ReturnValue) {
		case APID_DocapiMethodOk:
			break;

		case APID_DocapiOwnerAccessErr:		// オーナー名不正エラー.
			WAPG_API_trace("%s(%d) %s : getBoxOption APID_DocapiOwnerAccessErr \n", __FILE__, __LINE__, __FUNCTION__);
			printf("403:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //403->ErrorStatus[WAPD_STATUS_FORBIDDEN] in WAP_defs.h
			m_ResponseHeader->Status = WAPD_STATUS_FORBIDDEN;
			generateErrorResponseBody( WAPE_BoxError_OwnerAccessErr );
			return WAPD_AnyError;

		case APID_DocapiBoxidUnknownErr:	// 指定番号のBoxなしエラー.
			WAPG_API_trace("%s(%d) %s : getBoxOption APID_DocapiBoxidUnknownErr \n", __FILE__, __LINE__, __FUNCTION__);
			printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
			m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
			generateErrorResponseBody( WAPE_BoxError_BoxNotExist );
			return WAPD_AnyError;

		case APID_DocapiPasswordAccessErr:	// パスワード不正.
		case APID_DocapiCommonErr:			// その他の処理失敗エラー.
		default:
			WAPG_API_trace("%s(%d) %s : getBoxOption failed \n", __FILE__, __LINE__, __FUNCTION__);
			m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
			printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
			generateErrorResponseBody( WAPE_BoxError_None );
			return WAPD_AnyError;
	}

	// 拡張認証時はBoxAdmin, Adminは拡張認証ユーザー所有のボックスへアクセスできない（本体＋拡張認証時はアクセス可能）.
	TYPE_UserAuthentication e_AuthType = APIC_UserAuthentication::newInstance()->getUserAuthentication();
	if( TYPE_UserAuthenticationIntermeddiateServer == e_AuthType ) {
		if( NULL == pc_User ) {
			WAPG_API_trace("%s(%d) %s : WAPC_UserBoxController::openUserBox pc_User is NULL\n", __FILE__, __LINE__, __FUNCTION__);
			m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
			printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
			generateErrorResponseBody( WAPE_BoxError_None );
			return WAPD_AnyError;
		}
		TYPE_UserType e_UserType = pc_User->getUserType();
		if( ( TYPE_UserTypeBoxAdmin == e_UserType ) || ( TYPE_UserTypeAdmin == e_UserType ) ) {
			// 拡張認証方式の場合、特殊ユーザーを除き、個人ボックスの所有者は全て拡張認証ユーザー.
			if( ( APIE_BoxmakerUser == c_BoxOption.sl_BoxCreateInfo ) &&
				( False == APIC_User::isSpecialUserNo( c_BoxInfo.ul_OwnUserID ) ) ) {
				WAPG_API_trace("%s(%d) %s : BoxAdmin and Admin is not permitted to access Mid-User's box. \n", __FILE__, __LINE__, __FUNCTION__);
				printf("403:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //403->ErrorStatus[WAPD_STATUS_FORBIDDEN] in WAP_defs.h
				m_ResponseHeader->Status = WAPD_STATUS_FORBIDDEN;
				generateErrorResponseBody( WAPE_BoxError_OwnerAccessErr );
				return WAPD_AnyError;
			}
		}
	}

	// ボックスパスワード取得.
	char sc_BoxPassword[WAPD_BoxPasswordMaxLength] = {0};
	si32_Ret = getBoxPasswordFromBody( params, sc_BoxPassword, c_BoxOption.sl_BoxCreateInfo );
	if( WAPD_NoError != si32_Ret ) {
		return WAPD_AnyError;
	}

	// Box状態変化通知オブザーバー登録.
	si32_Ret = attachBoxObserver();
	if( WAPD_NoError != si32_Ret ) {
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		generateErrorResponseBody( WAPE_BoxError_None );
		return WAPD_AnyError;
	}

	// APIC_BoxRegistryインスタンス取得.
	APIC_BoxRegistry* pc_BoxRegistry = APIC_BoxRegistry::newInstance();
	if(NULL == pc_BoxRegistry){
		WAPG_API_trace("%s(%d) %s : APIC_BoxRegistry instance NULL \n", __FILE__, __LINE__, __FUNCTION__);
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		generateErrorResponseBody( WAPE_BoxError_None );
		return WAPD_AnyError;
	}
	// アクセス権情報設定.
	APIC_AccessTokenManager* pc_AccessTokenManager = APIC_AccessTokenManager::newInstance();
	JLGE_InspectLogIF e_InspectLogIF = JLGE_InspectLogIfOperationPanel;
	if( NULL != pc_AccessTokenManager){
		pc_AccessTokenManager->getAppType(m_AccessToken, &e_InspectLogIF);
	}
	pc_BoxRegistry->setLogIFNo( e_InspectLogIF );

	// チェック情報(認証成功(1)/失敗(0)).
	int32_t si32_Check = 0;

	// ボックスアクセス権取得.
    si32_ReturnValue = pc_BoxRegistry->BoxLogIn_Chk_ErrDetail(
		pc_User,			// ユーザー情報.
		sc_BoxPassword,		// ボックスパスワード.
		m_si32_BoxNo,		// ボックス番号.
		&si32_Check,		// [out]チェック情報(認証成功(1)/失敗(0)).
		e_InspectLogIF,	// アクセス元情報.
		pc_Track,			// 部門情報.
		(short)params->AuthorityID );		// 認証ID.
	WAPD_DEBUG1("BoxLogIn_Chk_ErrDetail() si32_ReturnValue = %d\n", si32_ReturnValue);
	// APIC_BoxRegistryインスタンス削除.
	pc_BoxRegistry->deleteInstance();

	switch(si32_ReturnValue) {
		case APID_DocapiMethodOk:
			{
				if(1 != si32_Check){
					m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
					printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
					generateErrorResponseBody( WAPE_BoxError_None );
					return WAPD_AnyError;
				}

				// パスワード保存.
				TYPS_TokenBoxInfo st_TokenBoxInfo = {};		// コンストラクタ無.
				st_TokenBoxInfo.m_i32BoxID = m_si32_BoxNo;
				strncpy(st_TokenBoxInfo.m_scBoxPassword, sc_BoxPassword, sizeof(st_TokenBoxInfo.m_scBoxPassword) - 1 );

				APIC_AccessTokenManager::newInstance()->setBoxIdPassword(
					params->AccessToken,
					st_TokenBoxInfo );

				si32_Ret = WAPD_NoError;
				m_ResponseHeader->Status = WAPD_STATUS_OK;
				break;
			}

		case APID_DocapiOwnerAccessErr:		// ユーザー番号不一致またはBox属性権限不一致エラー.
			printf("403:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //403->ErrorStatus[WAPD_STATUS_FORBIDDEN] in WAP_defs.h
			m_ResponseHeader->Status = WAPD_STATUS_FORBIDDEN;
			generateErrorResponseBody( WAPE_BoxError_OwnerAccessErr );
			break;

		case APID_DocapiPasswordAccessErr:	// パスワード不一致エラー.
			printf("403:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //403->ErrorStatus[WAPD_STATUS_FORBIDDEN] in WAP_defs.h
			m_ResponseHeader->Status = WAPD_STATUS_FORBIDDEN;
			generateErrorResponseBody( WAPE_BoxError_PasswordAccessErr );
			break;

		case APID_DocapiBoxidUnknownErr:	// 指定番号のBoxなしエラー.
			printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
			m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
			generateErrorResponseBody( WAPE_BoxError_BoxNotExist );
			break;

		case APID_DocapiLoginCntErr:		// ログイン回数失敗エラー.
			printf("503:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //503->ErrorStatus[WAPD_STATUS_SERVICE_UNAVAILABLE] in WAP_defs.h
			m_ResponseHeader->Status = WAPD_STATUS_SERVICE_UNAVAILABLE;
			generateErrorResponseBody( WAPE_BoxError_None );
			break;

		case APID_DocapiParametaErr:		// パラメータエラー.
		case APID_DocapiCommonErr:			// 内部処理エラー.
		default:
			m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
			printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
			generateErrorResponseBody( WAPE_BoxError_None );
			break;
	}

	return si32_Ret;
}

/**
 * @brief ユーザーボックス削除
 * @param[in] param サブコントローラー
 * @return 処理の成否, WAPD_NoError(0)：成功、WAPD_AnyError(1)：失敗
 */
int32_t WAPC_UserBoxController::deleteUserBox( WAPS_SubController *params )
{
	int32_t si32_Ret = WAPD_AnyError;
	
	// 前提条件チェック.
	si32_Ret = checkBoxOpenDelReqStatus();
	if( WAPD_FatalError == si32_Ret ){
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		generateErrorResponseBody( WAPE_BoxError_None );
		return WAPD_AnyError;
	} else if( WAPD_AnyError == si32_Ret ){
		printf("503:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //503->ErrorStatus[WAPD_STATUS_SERVICE_UNAVAILABLE] in WAP_defs.h
		m_ResponseHeader->Status = WAPD_STATUS_SERVICE_UNAVAILABLE;
		generateErrorResponseBody( WAPE_BoxError_None );
		return WAPD_AnyError;
	} else {
		// スコアチェック対応.
	}

	// ボックス存在チェック.
	Bool b_IsExist = False;
	si32_Ret = boxExists( m_si32_BoxNo, b_IsExist );
	if( WAPD_NoError != si32_Ret ) {
		return WAPD_AnyError;
	}
	if( False == b_IsExist ) {
		printf("404:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //404->ErrorStatus[WAPD_STATUS_NOT_FOUND] in WAP_defs.h
		m_ResponseHeader->Status = WAPD_STATUS_NOT_FOUND;
		generateErrorResponseBody( WAPE_BoxError_BoxNotExist );
		return WAPD_AnyError;
	}
	
	// ユーザー、部門情報取得.
	APIC_User* pc_User = NULL;
    APIC_Track* pc_Track = NULL;
	getUserTrackInformation( params, &pc_User, &pc_Track );

	// ボックスパスワード取得.
	Bool b_Rtn = False;
	char sc_BoxPassword[WAPD_BoxPasswordMaxLength] = {};
	char* psc_BoxPassword = NULL;
	b_Rtn = APIC_AccessTokenManager::newInstance()->getBoxPassword(
		params->AccessToken,
		m_si32_BoxNo,
		sc_BoxPassword );
	if( False == b_Rtn ) {
		printf("403:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //403->ErrorStatus[WAPD_STATUS_FORBIDDEN] in WAP_defs.h
		m_ResponseHeader->Status = WAPD_STATUS_FORBIDDEN;
		generateErrorResponseBody( WAPE_BoxError_BoxNotOpened );
		return WAPD_AnyError;
	}

	ushort us_StringNum = 0;
	Bool b_CheckRet = getStringLengthUTF8(sc_BoxPassword, WAPE_CheckString_1String, us_StringNum);
	if(False  == b_CheckRet){
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		generateErrorResponseBody( WAPE_BoxError_None );
		return WAPD_AnyError;
	}

	if( 0 < us_StringNum ) {
		psc_BoxPassword = sc_BoxPassword;
	}

	// 非同期結果クラス生成.
	WAPC_DeleteBoxResult* pc_AsyncResult = new WAPC_DeleteBoxResult();
	if( NULL == pc_AsyncResult ) {
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		generateErrorResponseBody( WAPE_BoxError_None );
		return WAPD_AnyError;
	}
	pc_AsyncResult->setHandle( m_pv_Handle );

	// コールバック関数テーブルより、使用可能な関数ポインタを取得.
	FP_ANS_CHANGE_BOX_STATUS fp_Cb = NULL;
	fp_Cb = getBoxDeleteCb();
	if( NULL == fp_Cb ) {
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
		releaseBoxDeleteCb( fp_Cb );
		delete pc_AsyncResult;
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		generateErrorResponseBody( WAPE_BoxError_None );
		return WAPD_AnyError;
	}

	// APIC_BoxRegistryインスタンス取得.
	APIC_BoxRegistry* pc_BoxRegistry = APIC_BoxRegistry::newInstance();
	if(NULL == pc_BoxRegistry){
		WAPG_API_trace("%s(%d) %s : APIC_BoxRegistry instance NULL \n", __FILE__, __LINE__, __FUNCTION__);
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		generateErrorResponseBody( WAPE_BoxError_None );
		return WAPD_AnyError;
	}
	// アクセス権情報設定.
	APIC_AccessTokenManager* pc_AccessTokenManager = APIC_AccessTokenManager::newInstance();
	JLGE_InspectLogIF e_InspectLogIF = JLGE_InspectLogIfOperationPanel;
	if( NULL != pc_AccessTokenManager){
		pc_AccessTokenManager->getAppType(m_AccessToken, &e_InspectLogIF);
	}
	pc_BoxRegistry->setLogIFNo( e_InspectLogIF );

	// ボックス作成.
	int32_t si32_ReturnValue = APID_DocapiCommonErr;
	si32_ReturnValue = pc_BoxRegistry->req_deleteBox(
		pc_User,				// ユーザー情報.
		psc_BoxPassword,		// ボックスパスワード.
		m_si32_BoxNo,			// ボックス番号.
		fp_Cb,					// コールバック関数.
		TYPD_DefaultNonBox,		// ボックス種別.
		pc_Track,				// 部門情報.
		TYPE_RegistrationTypeOnlyOne,	// 監査ログタイプ.
		static_cast<short>( params->AuthorityID ) );	// 認証ID.
	pc_BoxRegistry->deleteInstance();

	if(APID_DocapiMethodOk != si32_ReturnValue ){
		switch(si32_ReturnValue) {
			case APID_DocapiOwnerAccessErr:		// オーナー名不正エラー.
				printf("403:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //403->ErrorStatus[WAPD_STATUS_FORBIDDEN] in WAP_defs.h
				m_ResponseHeader->Status = WAPD_STATUS_FORBIDDEN;
				generateErrorResponseBody( WAPE_BoxError_OwnerAccessErr );
				break;

			case APID_DocapiCommonErr:			// その他の処理失敗エラー.
			default:
				m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
				printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
				generateErrorResponseBody( WAPE_BoxError_None );
				break;
		}
		releaseBoxDeleteCb( fp_Cb );
		delete pc_AsyncResult;
		return WAPD_AnyError;
	}

	// 非同期API.
	// 非同期リクエストが成功(WAPD_STATUS_OK/WAPD_NoError)が確定してからフラグを立てる.
	m_ResponseHeader->Async = WAPD_ASYNC_ENABLE;
	return WAPD_NoError;
}

/**
 * @brief ユーザーボックス情報取得
 * @param[in] param サブコントローラー
 * @return 処理の成否, WAPD_NoError(0)：成功、WAPD_AnyError(1)：失敗
 */
int32_t WAPC_UserBoxController::getUserBoxInformation( WAPS_SubController *params )
{
	int32_t si32_Ret = WAPD_AnyError;
	
	// 前提条件チェック.
	si32_Ret = checkBoxInfoReqStatus();
	if( WAPD_AnyError == si32_Ret ){
		printf("503:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //503->ErrorStatus[WAPD_STATUS_SERVICE_UNAVAILABLE] in WAP_defs.h
		m_ResponseHeader->Status = WAPD_STATUS_SERVICE_UNAVAILABLE;
		generateErrorResponseBody( WAPE_BoxError_None );
		return WAPD_AnyError;
	}

	// ボックス存在チェック.
	Bool b_IsExist = False;
	si32_Ret = boxExists( m_si32_BoxNo, b_IsExist );
	if( WAPD_NoError != si32_Ret ) {
		return WAPD_AnyError;
	}
	if( False == b_IsExist ) {
		printf("404:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //404->ErrorStatus[WAPD_STATUS_NOT_FOUND] in WAP_defs.h
		m_ResponseHeader->Status = WAPD_STATUS_NOT_FOUND;
		generateErrorResponseBody( WAPE_BoxError_BoxNotExist );
		return WAPD_AnyError;
	}
	
	// ユーザー、部門情報取得.
	APIC_User* pc_User = NULL;
    APIC_Track* pc_Track = NULL;
	getUserTrackInformation( params, &pc_User, &pc_Track );

	// ボックス設定.
	APIC_Box* pc_Box = APIC_Box::newInstance();
	if(NULL == pc_Box){
		WAPG_API_trace("%s(%d) %s : APIC_Box instance NULL \n", __FILE__, __LINE__, __FUNCTION__);
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		generateErrorResponseBody( WAPE_BoxError_None );
		return WAPD_AnyError;
	}
	pc_Box->setBoxId( m_si32_BoxNo );

	m_Response = new Json::Value;
	APIC_BoxInfo c_BoxInfo;       /* 取得Info値 */

	// ボックス情報取得.
	int32_t si32_ReturnValue = APID_DocapiCommonErr;
    si32_ReturnValue = pc_Box->getBoxInfo(
		pc_User,
		&c_BoxInfo,
		(LIBE_CharSet)APID_DefaultCharSet,
		pc_Track);
	// pc_Box->deleteInstance();.

	switch(si32_ReturnValue) {
		case APID_DocapiMethodOk:
			{
				// ■ ボックス名.
				(*m_Response)["box_name"] = c_BoxInfo.sc_BoxName;

				// ■ ボックス作成日時.
				(*m_Response)["create_time"]["year"] = c_BoxInfo.c_CreateDate.ss_Year;
				(*m_Response)["create_time"]["month"] = c_BoxInfo.c_CreateDate.sc_Month;
				(*m_Response)["create_time"]["day"] = c_BoxInfo.c_CreateDate.sc_Day;
				(*m_Response)["create_time"]["hour"] = c_BoxInfo.c_CreateDate.sc_Hour;
				(*m_Response)["create_time"]["minute"] = c_BoxInfo.c_CreateDate.sc_Minute;
				(*m_Response)["create_time"]["second"] = c_BoxInfo.c_CreateDate.sc_Second;
				
				// ■ ボックス検索キー種別.
				char sc_SearchKey[WAPD_MaxKeyLength] = {0};
				if(False == WAPS_CommonTableString<WAPE_SearchKey, TYPE_SearchKey>::CommonToWapString(
					c_BoxInfo.sc_searchKey, sc_SearchKey)){
					m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
					printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
					generateErrorResponseBody( WAPE_BoxError_None );
					return WAPD_AnyError;
				}
				(*m_Response)["index"] = sc_SearchKey;
				break;
			}

		case APID_DocapiOwnerAccessErr:		// オーナー名不正エラー.
			printf("403:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //403->ErrorStatus[WAPD_STATUS_FORBIDDEN] in WAP_defs.h
			m_ResponseHeader->Status = WAPD_STATUS_FORBIDDEN;
			generateErrorResponseBody( WAPE_BoxError_OwnerAccessErr );
			return WAPD_AnyError;

		case APID_DocapiBoxidUnknownErr:	// 指定番号のBoxなしエラー.
			printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
			m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
			generateErrorResponseBody( WAPE_BoxError_BoxNotExist );
			return WAPD_AnyError;

		case APID_DocapiPasswordAccessErr:	// パスワード不正.
		case APID_DocapiCommonErr:			// その他の処理失敗エラー.
		default:
			m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
			printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
			generateErrorResponseBody( WAPE_BoxError_None );
			return WAPD_AnyError;
	}	

	// APIC_BoxRegistryインスタンス取得.
	APIC_BoxRegistry* pc_BoxRegistry = APIC_BoxRegistry::newInstance();
	if(NULL == pc_BoxRegistry){
		WAPG_API_trace("%s(%d) %s : APIC_BoxRegistry instance NULL \n", __FILE__, __LINE__, __FUNCTION__);
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		generateErrorResponseBody( WAPE_BoxError_None );
		return WAPD_AnyError;
	}
	// アクセス権情報設定.
	APIC_AccessTokenManager* pc_AccessTokenManager = APIC_AccessTokenManager::newInstance();
	JLGE_InspectLogIF e_InspectLogIF = JLGE_InspectLogIfOperationPanel;
	if( NULL != pc_AccessTokenManager){
		pc_AccessTokenManager->getAppType(m_AccessToken, &e_InspectLogIF);
	}
	pc_BoxRegistry->setLogIFNo( e_InspectLogIF );

	// 親展受信情報取得.
	APIS_ConfidentialBoxStruct s_Confidential = {};
	si32_ReturnValue = pc_BoxRegistry->ConfidentialInfoAction(
		m_si32_BoxNo, 				// 指定ボックス番号.
		WAPD_BOX_CONFBOX_OPE_GET, 	// アクセス種別.
		&s_Confidential, 			// 親展受信情報.
		pc_User, 					// ユーザー情報.
		pc_Track, 					// 部門情報.
		TYPE_RegistrationTypeOnlyOne,	// 監査ログタイプ.
		static_cast<short> ( params->AuthorityID ) );		// 認証ID.
	pc_BoxRegistry->deleteInstance();

	switch(si32_ReturnValue) {
		case APID_DocapiMethodOk:
			// ■ 親展受信機能の有無はボックス番号の一致で判断する.
			if( m_si32_BoxNo == s_Confidential.sl_BoxNo ){
				(*m_Response)["confidential_rx"] = true;
			} else {
				(*m_Response)["confidential_rx"] = false; //NIFの実装に倣って、念のため.
			}
			break;

		case APID_DocapiCommonErr:			// その他の処理失敗エラー.
		default:
			//「WAPD_BOX_CONFBOX_OPE_GETでエラー＝親展受信機能が設定されていない」
			// というI/F仕様であるため、ここでは親展受信機能無効と判断し、WebAPI実行エラーとはならない.
			(*m_Response)["confidential_rx"] = false;
			break;
	}

	// ■ ボックスパスワードの有無.
	Bool b_Rtn = False;
	char sc_BoxPassword[WAPD_BoxPasswordMaxLength] = {0};
	char* psc_BoxPassword = NULL;
	b_Rtn = APIC_AccessTokenManager::newInstance()->getBoxPassword(
		params->AccessToken,
		m_si32_BoxNo,
		sc_BoxPassword );
	if( False == b_Rtn ) {
		printf("403:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //403->ErrorStatus[WAPD_STATUS_FORBIDDEN] in WAP_defs.h
		m_ResponseHeader->Status = WAPD_STATUS_FORBIDDEN;
		generateErrorResponseBody( WAPE_BoxError_BoxNotOpened );
		return WAPD_AnyError;
	}

	ushort us_StringNum = 0;
	Bool b_CheckRet = getStringLengthUTF8(sc_BoxPassword, WAPE_CheckString_1String, us_StringNum);
	if(False  == b_CheckRet){
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		generateErrorResponseBody( WAPE_BoxError_None );
		return WAPD_AnyError;
	}

	if( 0 < us_StringNum ) {
		(*m_Response)["is_box_password"] = true;
		psc_BoxPassword = sc_BoxPassword;
	} else {
		(*m_Response)["is_box_password"] = false;
	}

	// 所有者情報取得.
	APIC_BoxOption c_BoxOption;
	si32_ReturnValue = pc_Box->getBoxOption(
		pc_User,					// ユーザー情報.
		psc_BoxPassword,			// ボックスパスワード.
		&c_BoxOption,				// ボックスオプション情報.
		pc_Track);					// 部門情報.
	pc_Box->deleteInstance();

	switch(si32_ReturnValue) {
		case APID_DocapiMethodOk:
			{
				// ボックス種別がシステムの場合はエラー終了.
				if( APIE_BoxmakerSystem == c_BoxOption.sl_BoxCreateInfo ) {
					printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
					m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
					generateErrorResponseBody( WAPE_BoxError_InvalidBoxKind );
					return WAPD_AnyError;
				}

				// ボックス種別.
				char sc_BoxType[WAPD_MaxKeyLength] = {0};
				if(False == WAPS_CommonTableString<WAPE_BoxType, APIE_BoxOptionCreateInfo>::CommonToWapString(
					c_BoxOption.sl_BoxCreateInfo, sc_BoxType)){
					m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
					printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
					generateErrorResponseBody( WAPE_BoxError_None );
					return WAPD_AnyError;
				}
				(*m_Response)["type"] = sc_BoxType;

				// ボックス所有者.
				char sc_BoxOwnerName[WAPD_BoxOwnerNameMaxLength] = {0};
				if( APIE_BoxmakerGroup == c_BoxOption.sl_BoxCreateInfo ){
					// グループボックスの場合.
					pc_Track = APIC_Track::newInstance(c_BoxInfo.ul_OwnUserID);
					if(NULL != pc_Track){
						pc_Track->getTrackName(sc_BoxOwnerName);
					}

					// 部門管理パスワードのみの場合、getTrackName()では部門名が空なので再取得.
					ushort us_StringNum = 0;
					Bool b_CheckRet = getStringLengthUTF8(sc_BoxOwnerName, WAPE_CheckString_1String, us_StringNum);
					if(False  == b_CheckRet){
						m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
						printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
						generateErrorResponseBody( WAPE_BoxError_None );
						return WAPD_AnyError;
					}
					if( 0 >= us_StringNum ) {
						if(NULL != pc_Track){
							pc_Track->getTrackDispNameUTF8(sc_BoxOwnerName, TYPE_CharSetUTF8);
						}
					}
				}
				else if( APIE_BoxmakerUser == c_BoxOption.sl_BoxCreateInfo ){
					// 個人ボックスの場合.
					TYPE_UserAuthentication e_UserAuth = APIC_UserAuthentication::newInstance()->getUserAuthentication();
					WAPE_BoxUserType e_BoxOwnerType = getBoxOwnerType( APIE_BoxmakerUser, c_BoxInfo.ul_OwnUserID );
					WAPE_BoxUserType e_OperatorType = getBoxUserType( pc_User, pc_Track );

					// 中間認証ユーザーのボックスは、自身以外は所有者名を見ることはできないので空に設定.
					if ( ( ( TYPE_UserAuthenticationIntermeddiateServer == e_UserAuth ) || 
					   ( TYPE_UserAuthenticationDeviceAndMidAuth == e_UserAuth ) ) && 
					   ( WAPE_BoxUserType_User_Mid != e_OperatorType ) &&
					   ( WAPE_BoxUserType_User_Mid == e_BoxOwnerType ) ) {
						sc_BoxOwnerName[0] = '\0';
					}
					else {
						pc_User = APIC_User::newInstance( c_BoxInfo.ul_OwnUserID );
						if(NULL != pc_User){
							pc_User->getUserNameUTF8( sc_BoxOwnerName );
						}
					}
				}
				else {
					// 共有ボックスの場合.
					// 所有者名は表示しない.
				}
				(*m_Response)["owner_info"]["owner_name"] = sc_BoxOwnerName;

				// 所有者の属する外部サーバー名.
				char sc_BoxServerName[WAPD_ServerNameMaxLength] = {0};
				if(APIE_BoxmakerUser == c_BoxOption.sl_BoxCreateInfo){
					// 個人ボックスの場合.
					APIC_User* pc_OwnerUser = APIC_User::newInstance(c_BoxInfo.ul_OwnUserID);
					if(pc_OwnerUser != NULL){
						// 認証方式が"本体+外部サーバー"以外の場合、既存(V1.2.0)処理を実施.
						if(TYPE_UserAuthenticationDeviceAndServer != APIC_UserAuthentication::newInstance()->getUserAuthentication()) {
							pc_OwnerUser->getServerName(sc_BoxServerName);
						// 認証方式が"本体+外部サーバー"の場合、ボックスユーザのログイン種別から外部サーバ認証ユーザか判断.
						} else {
							if(TYPE_UserAuthenticationServer == pc_OwnerUser->getLoginUserAuthenticationType()){
								// 外部サーバ認証ユーザの場合、認証サーバ名を取得し設定.
								pc_OwnerUser->getServerName(sc_BoxServerName);
							}
						}
					}
				}
				(*m_Response)["owner_info"]["server_name"] = sc_BoxServerName;

				// ドキュメント自動削除時間.
				int32_t si32_DeleteTime = 0;
				if(False == WAPS_CommonTable<WAPE_DelTime, APIE_BoxOptionDelTime>::CommonToWap(
					c_BoxOption.sl_AutoDeleteTime, si32_DeleteTime)){
					if ( ( APIE_5min > c_BoxOption.sl_AutoDeleteTime ) ||
						 ( APIE_12h  < c_BoxOption.sl_AutoDeleteTime ) ) {
						m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
						printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
						generateErrorResponseBody( WAPE_BoxError_None );
						return WAPD_AnyError;
					}
					si32_DeleteTime = static_cast<int32_t>(c_BoxOption.sl_AutoDeleteTime);
				}
				(*m_Response)["auto_document_delete_time"] = si32_DeleteTime;

				m_ResponseHeader->Status = WAPD_STATUS_OK;
				break;
			}

		case APID_DocapiOwnerAccessErr:		// オーナー名不正エラー.
			printf("403:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //403->ErrorStatus[WAPD_STATUS_FORBIDDEN] in WAP_defs.h
			m_ResponseHeader->Status = WAPD_STATUS_FORBIDDEN;
			generateErrorResponseBody( WAPE_BoxError_OwnerAccessErr );
			return WAPD_AnyError;

		case APID_DocapiBoxidUnknownErr:	// 指定番号のBoxなしエラー.
			printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
			m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
			generateErrorResponseBody( WAPE_BoxError_BoxNotExist );
			return WAPD_AnyError;

		case APID_DocapiPasswordAccessErr:	// パスワード不正.
		case APID_DocapiCommonErr:			// その他の処理失敗エラー.
		default:
			m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
			printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
			generateErrorResponseBody( WAPE_BoxError_None );
			return WAPD_AnyError;
	}	

	return WAPD_NoError;
}

/**
 * @brief ユーザーボックス情報設定
 * @param[in] param サブコントローラー
 * @return 処理の成否, WAPD_NoError(0)：成功、WAPD_AnyError(1)：失敗
 */
int32_t WAPC_UserBoxController::setUserBoxInformation( WAPS_SubController *params )
{
	int32_t si32_Ret = WAPD_AnyError;
	
	// 前提条件チェック.
	si32_Ret = checkSetBoxInfoReqStatus();
	if( WAPD_FatalError == si32_Ret ){
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		generateErrorResponseBody( WAPE_BoxError_None );
		return WAPD_AnyError;
	} else if( WAPD_AnyError == si32_Ret ){
		printf("503:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //503->ErrorStatus[WAPD_STATUS_SERVICE_UNAVAILABLE] in WAP_defs.h
		m_ResponseHeader->Status = WAPD_STATUS_SERVICE_UNAVAILABLE;
		generateErrorResponseBody( WAPE_BoxError_None );
		return WAPD_AnyError;
	} else {
		// スコアチェック対応.
	}

	// ボックス存在チェック.
	Bool b_IsExist = False;
	si32_Ret = boxExists( m_si32_BoxNo, b_IsExist );
	if( WAPD_NoError != si32_Ret ) {
		return WAPD_AnyError;
	}
	if( False == b_IsExist ) {
		printf("404:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //404->ErrorStatus[WAPD_STATUS_NOT_FOUND] in WAP_defs.h
		m_ResponseHeader->Status = WAPD_STATUS_NOT_FOUND;
		generateErrorResponseBody( WAPE_BoxError_BoxNotExist );
		return WAPD_AnyError;
	}
	
	// ユーザー、部門情報取得.
	APIC_User* pc_User = NULL;
    APIC_Track* pc_Track = NULL;
	getUserTrackInformation( params, &pc_User, &pc_Track );

	// ボックスパスワード取得.
	Bool b_Rtn = False;
	char sc_BoxPassword[WAPD_BoxPasswordMaxLength] = {};
	char* psc_BoxPassword = NULL;
	b_Rtn = APIC_AccessTokenManager::newInstance()->getBoxPassword(
		params->AccessToken,
		m_si32_BoxNo,
		sc_BoxPassword );
	if( False == b_Rtn ) {
		printf("403:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //403->ErrorStatus[WAPD_STATUS_FORBIDDEN] in WAP_defs.h
		m_ResponseHeader->Status = WAPD_STATUS_FORBIDDEN;
		generateErrorResponseBody( WAPE_BoxError_BoxNotOpened );
		return WAPD_AnyError;
	}

	ushort us_StringNum = 0;
	Bool b_CheckRet = getStringLengthUTF8(sc_BoxPassword, WAPE_CheckString_1String, us_StringNum);
	if(False  == b_CheckRet){
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		generateErrorResponseBody( WAPE_BoxError_None );
		return WAPD_AnyError;
	}

	if( 0 < us_StringNum ) {
		psc_BoxPassword = sc_BoxPassword;
	}

	// ボックスインスタンス取得.
	APIC_Box* pc_Box = APIC_Box::newInstance();
	if(NULL == pc_Box){
		WAPG_API_trace("%s(%d) %s : APIC_Box instance NULL \n", __FILE__, __LINE__, __FUNCTION__);
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		generateErrorResponseBody( WAPE_BoxError_None );
		return WAPD_AnyError;
	}
	pc_Box->setBoxId( m_si32_BoxNo );

	// 01.ボックス名設定.
	si32_Ret = setBoxName( params, pc_Box, pc_User, pc_Track, psc_BoxPassword );
	if( WAPD_NoError != si32_Ret ) {
		WAPG_API_trace("%s(%d) %s : setBoxName failed \n", __FILE__, __LINE__, __FUNCTION__);
		pc_Box->deleteInstance();
		return WAPD_AnyError;
	}

	// 02.文書自動削除時間設定.
	si32_Ret = setAutoDeleteTime( params, pc_Box, pc_User, pc_Track, psc_BoxPassword );
	if( WAPD_NoError != si32_Ret ) {
		WAPG_API_trace("%s(%d) %s : setAutoDeleteTime failed \n", __FILE__, __LINE__, __FUNCTION__);
		pc_Box->deleteInstance();
		return WAPD_AnyError;
	}

	// 03.ボックス種別および所有者設定.
	si32_Ret = setBoxTypeAndOwner( params, pc_Box, pc_User, pc_Track, psc_BoxPassword );
	if( WAPD_NoError != si32_Ret ) {
		WAPG_API_trace("%s(%d) %s : setBoxTypeAndOwner failed \n", __FILE__, __LINE__, __FUNCTION__);
		pc_Box->deleteInstance();
		return WAPD_AnyError;
	}

	// 04.ボックス検索キー種別設定.
	si32_Ret = setSearchKey( params, pc_Box, pc_User, pc_Track, psc_BoxPassword );
	if( WAPD_NoError != si32_Ret ) {
		WAPG_API_trace("%s(%d) %s : setSearchKey failed \n", __FILE__, __LINE__, __FUNCTION__);
		pc_Box->deleteInstance();
		return WAPD_AnyError;
	}

	// 05.親展受信機能情報設定.
	si32_Ret = setConfidentialRx( params, pc_Box, pc_User, pc_Track, psc_BoxPassword );
	if( WAPD_NoError != si32_Ret ) {
		WAPG_API_trace("%s(%d) %s : setConfidentialRx failed \n", __FILE__, __LINE__, __FUNCTION__);
		pc_Box->deleteInstance();
		return WAPD_AnyError;
	}

	// 06.ボックスパスワード設定.
	si32_Ret = setBoxPassword( params, pc_Box, pc_User, pc_Track, psc_BoxPassword );
	if( WAPD_NoError != si32_Ret ) {
		WAPG_API_trace("%s(%d) %s : setBoxPassword failed \n", __FILE__, __LINE__, __FUNCTION__);
		pc_Box->deleteInstance();
		return WAPD_AnyError;
	}

	pc_Box->deleteInstance();

	return WAPD_NoError;
}

/**
 * @brief リクエストボディからボックス番号を取得する
 * @param[in]	param サブコントローラー
 * @param[out]	si32_BoxNo ボックス番号
 * @return 処理の成否, WAPD_NoError(0)：成功、WAPD_AnyError(1)：失敗
 */
int32_t WAPC_UserBoxController::getBoxNoFromBody( WAPS_SubController *params, int32_t& si32_BoxNo )
{
	int32_t si32_Ret = WAPD_AnyError;
	Bool	b_GetBoxNo = False;		// ボックス番号取得フラグ.

	if( NULL == params->Param ) {
		// ボディが無い場合はでもエラーにしない.
		b_GetBoxNo = False;

	} else {
		if( !params->Param->isObject() ) {
			WAPG_API_trace("%s(%d) %s : Body is not Json Object \n", __FILE__, __LINE__, __FUNCTION__);
			printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
			m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
			generateErrorResponseBody( WAPE_BoxError_InvalidJsonSyntax );
			return WAPD_AnyError;
		}

		if( params->Param->isMember("box_no") ) {
			Json::Value& box_no = (*params->Param)["box_no"];
			if( box_no.type() != Json::intValue ) {
				WAPG_API_trace("%s(%d) %s : Illegal json value type box_no \n", __FILE__, __LINE__, __FUNCTION__);
				printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
				m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
				generateErrorResponseBody( WAPE_BoxError_InvalidJsonSyntax );
				return WAPD_AnyError;
			}

			si32_BoxNo = box_no.asInt();

			// 範囲チェック.
			if( ( WAPD_BoxNoMin > si32_BoxNo ) || ( WAPD_BoxNoMax < si32_BoxNo ) ) {
				WAPG_API_trace("%s(%d) %s : Illegal json value : box_no \n", __FILE__, __LINE__, __FUNCTION__);
				printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
				m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
				generateErrorResponseBody( WAPE_BoxError_InvalidBoxNo );
				return WAPD_AnyError;
			}

			// ボックス番号取得.
			b_GetBoxNo = True;

		} else {
			// ボックス番号取得出来ない.
			b_GetBoxNo = False;
		}
	}

	if ( False == b_GetBoxNo ) {
		// ボックス番号取得出来ないときは最小ボックス番号を取得.
		APIC_Box* pc_Box = APIC_Box::newInstance();
		if(NULL == pc_Box){
			WAPG_API_trace("%s(%d) %s : APIC_Box instance NULL \n", __FILE__, __LINE__, __FUNCTION__);
			m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
			printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
			generateErrorResponseBody( WAPE_BoxError_None );
			return WAPD_AnyError;
		}

		int32_t ret = pc_Box->getNoUsedBoxID( &si32_BoxNo );
		pc_Box->deleteInstance();
		if(ret != APID_DocapiMethodOk) {
			WAPG_API_trace("%s(%d) %s : getNoUsedBoxID Faild \n", __FILE__, __LINE__, __FUNCTION__);
			printf("503:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //503->ErrorStatus[WAPD_STATUS_SERVICE_UNAVAILABLE] in WAP_defs.h
			m_ResponseHeader->Status = WAPD_STATUS_SERVICE_UNAVAILABLE;
			generateErrorResponseBody( WAPE_BoxError_None );
			return WAPD_AnyError;
		}
	}

	WAPD_DEBUG1("si32_BoxNo = %d\n", si32_BoxNo);
	si32_Ret = WAPD_NoError;
	return si32_Ret;
}

/**
 * @brief リクエストボディからボックス種別を取得する
 * @param[in]	param サブコントローラー
 * @param[out]	sc_BoxType ボックス種別
 * @return 処理の成否, WAPD_NoError(0)：成功、WAPD_AnyError(1)：失敗
 */
int32_t WAPC_UserBoxController::getBoxTypeFromBody( WAPS_SubController *params, APIE_BoxOptionCreateInfo& e_BoxType )
{
	int32_t si32_Ret = WAPD_AnyError;
	if( NULL == params->Param ) {
		// ボディが無い場合はでもエラーにしない.
	} else {
		if( !params->Param->isObject() ) {
			WAPG_API_trace("%s(%d) %s : Body is not Json Object \n", __FILE__, __LINE__, __FUNCTION__);
			printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
			m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
			generateErrorResponseBody( WAPE_BoxError_InvalidJsonSyntax );
			return WAPD_AnyError;
		}

		if( params->Param->isMember("type") ) {
			Json::Value& type = (*params->Param)["type"];
			if( type.type() != Json::stringValue ) {
				WAPG_API_trace("%s(%d) %s : Illegal json value type : type \n", __FILE__, __LINE__, __FUNCTION__);
				printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
				m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
				generateErrorResponseBody( WAPE_BoxError_InvalidJsonSyntax );
				return WAPD_AnyError;
			}
			if( WAPD_MaxKeyLength <= type.asString().length() ) {
				WAPG_API_trace("%s(%d) %s : Illegal sc_BoxType value \n", __FILE__, __LINE__, __FUNCTION__);
				printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
				m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
				generateErrorResponseBody( WAPE_BoxError_InvalidBoxKind );
				return WAPD_AnyError;
			}

			char sc_BoxType[WAPD_MaxKeyLength] = {0};
			strncpy(sc_BoxType, type.asString().c_str(), sizeof(sc_BoxType) - 1 );
			if( False == WAPS_CommonTableString<WAPE_BoxType, APIE_BoxOptionCreateInfo>::WapStringToCommon(
				sc_BoxType, e_BoxType )){
				WAPG_API_trace("%s(%d) %s : Illegal sc_BoxType value \n", __FILE__, __LINE__, __FUNCTION__);
				printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
				m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
				generateErrorResponseBody( WAPE_BoxError_InvalidBoxKind );
				return WAPD_AnyError;
			}
		} else {
			// ボックス種別が設定されていない場合はパラメータの値は更新しない.
		}
	}

	WAPD_DEBUG1("e_BoxType = %d\n", e_BoxType);
	si32_Ret = WAPD_NoError;
	return si32_Ret;
}

/**
 * @brief リクエストボディからボックスパスワードを取得する
 * @param[in]	param サブコントローラー
 * @param[out]	sc_BoxPassword ボックスパスワード
 * @param[in]	e_BoxType ボックス種別
 * @return 処理の成否, WAPD_NoError(0)：成功、WAPD_AnyError(1)：失敗
 */
int32_t WAPC_UserBoxController::getBoxPasswordFromBody( WAPS_SubController *params, char (&sc_BoxPassword)[WAPD_BoxPasswordMaxLength], APIE_BoxOptionCreateInfo e_BoxType )
{
	if( NULL == params->Param ) {
		// ボディが無い場合でもエラーにしない.
	} else {
		if( !params->Param->isObject() ) {
			WAPG_API_trace("%s(%d) %s : Body is not Json Object \n", __FILE__, __LINE__, __FUNCTION__);
			printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
			m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
			generateErrorResponseBody( WAPE_BoxError_InvalidJsonSyntax );
			return WAPD_AnyError;
		}

		if( params->Param->isMember("box_password") ) {
			Json::Value& box_password = (*params->Param)["box_password"];
			if( box_password.type() != Json::stringValue ) {
				WAPG_API_trace("%s(%d) %s : Illegal json value type : box_password \n", __FILE__, __LINE__, __FUNCTION__);
				printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
				m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
				generateErrorResponseBody( WAPE_BoxError_InvalidJsonSyntax );
				return WAPD_AnyError;
			}

			std::string str_box_password = box_password.asString();
			// バイト数チェック.
			if ( (WAPD_BoxPasswordMaxLength - 1) < str_box_password.length() ){
				WAPD_DEBUG("Illegal password length Byte \n");
				printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
				m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
				generateErrorResponseBody( WAPE_BoxError_InvalidPassword );
				return WAPD_AnyError;
			}

			// sc_BoxPassword を初期化.
			memset(sc_BoxPassword, 0, sizeof(sc_BoxPassword));

			strncpy( sc_BoxPassword, box_password.asString().c_str(), sizeof(sc_BoxPassword) - 1 );

			// ボックス種別が共有ボックスの場合のみパスワード規約チェック.
			if( APIE_BoxmakerAuto == e_BoxType ) {
				if( WAPE_PasswordRules_On_Invalid == checkPasswordRules( sc_BoxPassword ) ) {
					WAPG_API_trace("%s(%d) %s : PasswordRulesOn check NG.\n", __FILE__, __LINE__, __FUNCTION__);
					printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
					m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
					generateErrorResponseBody( WAPE_BoxError_InvalidPassword );
					return WAPD_AnyError;
				}
			}
		} else {
			// ボックスパスワードが設定されていない場合はパラメータの値は更新しない.
		}
	}

	WAPD_DEBUG1("sc_BoxPassword = %s\n", sc_BoxPassword);
	return WAPD_NoError;
}

/**
 * @brief クエリーパラメータからユーザーボックス検索値を取得する
 * @param[in]	param サブコントローラー
 * @param[out]	sc_SearchKey ユーザーボックス検索値
 * @return 処理の成否, WAPD_NoError(0)：成功、WAPD_AnyError(1)：失敗
 */
int32_t WAPC_UserBoxController::getSearchKeyFromQuery( WAPS_SubController *params, TYPE_SearchKey& e_SearchKey )
{
	int32_t si32_Ret = WAPD_AnyError;
	if( NULL == params->Search ) {
		WAPG_API_trace("%s(%d) %s : Body is NULL \n", __FILE__, __LINE__, __FUNCTION__);
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		generateErrorResponseBody( WAPE_BoxError_None );
		return WAPD_AnyError;
	}
	if( !params->Search->isObject() ) {
		WAPG_API_trace("%s(%d) %s : Body is not Json Object \n", __FILE__, __LINE__, __FUNCTION__);
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		generateErrorResponseBody( WAPE_BoxError_None );
		return WAPD_AnyError;
	}

	if( params->Search->isMember("search") ) {
		Json::Value& search = (*params->Search)["search"];
		if( search.type() != Json::stringValue ) {
			WAPG_API_trace("%s(%d) %s : Illegal json value type : search \n", __FILE__, __LINE__, __FUNCTION__);
			m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
			printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
			generateErrorResponseBody( WAPE_BoxError_None );
			return WAPD_AnyError;
		}
		if( WAPD_MaxKeyLength <= search.asString().length() ) {
			WAPG_API_trace("%s(%d) %s : Illegal search key value \n", __FILE__, __LINE__, __FUNCTION__);
			printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
			m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
			generateErrorResponseBody( WAPE_BoxError_InvalidSearchKey );
			return WAPD_AnyError;
		}

		char sc_Search[WAPD_MaxKeyLength] = {0};
		strncpy(sc_Search, search.asString().c_str(), sizeof(sc_Search) - 1 );
		if(False == WAPS_CommonTableString<WAPE_SearchKey, TYPE_SearchKey>::WapStringToCommon(
			sc_Search, e_SearchKey )){
			WAPG_API_trace("%s(%d) %s : Illegal search key value \n", __FILE__, __LINE__, __FUNCTION__);
			printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
			m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
			generateErrorResponseBody( WAPE_BoxError_InvalidSearchKey );
			return WAPD_AnyError;
		}
	} else {
		// ボックス種別が設定されていない場合はパラメータの値は更新しない.
	}

	WAPD_DEBUG1("e_SearchKey = %d\n", e_SearchKey);
	si32_Ret = WAPD_NoError;
	return si32_Ret;
}

/**
 * @brief クエリーパラメータからリスト取得開始位置を取得する
 * @param[in]	param サブコントローラー
 * @param[out]	si32_Offset リスト取得開始位置
 * @return 処理の成否, WAPD_NoError(0)：成功、WAPD_AnyError(1)：失敗
 */
int32_t WAPC_UserBoxController::getOffsetFromQuery( WAPS_SubController *params, int32_t& si32_Offset )
{
	if( NULL == params->Search ) {
		WAPG_API_trace("%s(%d) %s : Query is NULL \n", __FILE__, __LINE__, __FUNCTION__);
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		generateErrorResponseBody( WAPE_BoxError_None );
		return WAPD_AnyError;
	}
	if( !params->Search->isObject() ) {
		WAPG_API_trace("%s(%d) %s : Query is not Json Object \n", __FILE__, __LINE__, __FUNCTION__);
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		generateErrorResponseBody( WAPE_BoxError_None );
		return WAPD_AnyError;
	}

	int32_t si32_Ret = WAPD_AnyError;

	// 初期値設定.
	si32_Offset = 1;

	if( params->Search->isMember("offset") ) {
		Json::Value& offset = (*params->Search)["offset"];
		if( offset.type() != Json::stringValue ) {
			WAPG_API_trace("%s(%d) %s : Illegal json value type : offset \n", __FILE__, __LINE__, __FUNCTION__);
			m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
			printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
			generateErrorResponseBody( WAPE_BoxError_None );
			return WAPD_AnyError;
		}

		const char* psc_String = offset.asString().c_str();
		int32_t si32_TempOffset = 0;
		uint32_t ui32_length = 0;
		if( WAPD_NoError != stoi(psc_String, si32_TempOffset, ui32_length) ) {
			WAPG_API_trace("%s(%d) %s : not string : offset \n", __FILE__, __LINE__, __FUNCTION__);
			printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
			m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
			generateErrorResponseBody( WAPE_BoxError_InvalidOffset );
			return WAPD_AnyError;
		}

		si32_Offset = si32_TempOffset;
		WAPD_DEBUG1("offset = %d\n", si32_Offset);

		// 範囲チェック(1～1000).
		if( ( WAPD_GetListMinOffset > si32_Offset ) || ( WAPD_GetListMaxOffset < si32_Offset ) ) {
			WAPG_API_trace("%s(%d) %s : Illegal offset value offset = %d\n", __FILE__, __LINE__, __FUNCTION__, si32_Offset);
			printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
			m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
			generateErrorResponseBody( WAPE_BoxError_InvalidOffset );
			return WAPD_AnyError;
		}
	} else {
		// リスト取得開始位置が設定されていない場合はパラメータの値は更新しない.
	}

	WAPD_DEBUG1("si32_Offset = %d\n", si32_Offset);
	si32_Ret = WAPD_NoError;
	return si32_Ret;
}

/**
 * @brief クエリーパラメータから最大取得数を取得する
 * @param[in]	param サブコントローラー
 * @param[out]	si32_Num 最大取得数
 * @return 処理の成否, WAPD_NoError(0)：成功、WAPD_AnyError(1)：失敗
 */
int32_t WAPC_UserBoxController::getNumFromQuery( WAPS_SubController *params, int32_t& si32_Num )
{
	if( NULL == params->Search ) {
		WAPG_API_trace("%s(%d) %s : Query is NULL \n", __FILE__, __LINE__, __FUNCTION__);
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		generateErrorResponseBody( WAPE_BoxError_None );
		return WAPD_AnyError;
	}
	if( !params->Search->isObject() ) {
		WAPG_API_trace("%s(%d) %s : Query is not Json Object \n", __FILE__, __LINE__, __FUNCTION__);
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		generateErrorResponseBody( WAPE_BoxError_None );
		return WAPD_AnyError;
	}

	int32_t si32_Ret = WAPD_AnyError;

	// 初期値設定.
	si32_Num = WAPD_GetListInitNum;

	if( params->Search->isMember("num") ) {
		Json::Value& num = (*params->Search)["num"];
		if( num.type() != Json::stringValue ) {
			WAPG_API_trace("%s(%d) %s : Illegal json value type : num \n", __FILE__, __LINE__, __FUNCTION__);
			m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
			printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
			generateErrorResponseBody( WAPE_BoxError_None );
			return WAPD_AnyError;
		}

		const char* psc_String = num.asString().c_str();
		int32_t si32_TempNum = 0;
		uint32_t ui32_length = 0;
		if( WAPD_NoError != stoi(psc_String, si32_TempNum, ui32_length) ) {
			WAPG_API_trace("%s(%d) %s : not string : num \n", __FILE__, __LINE__, __FUNCTION__);
			printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
			m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
			generateErrorResponseBody( WAPE_BoxError_InvalidNum );
			return WAPD_AnyError;
		}

		si32_Num = si32_TempNum;
		WAPD_DEBUG1("num = %d\n", si32_Num);

		// 範囲チェック(1～1000).
		if( ( WAPD_GetListMinNum > si32_Num ) || ( WAPD_GetListMaxNum < si32_Num ) ) {
			WAPG_API_trace("%s(%d) %s : Illegal num value \n", __FILE__, __LINE__, __FUNCTION__);
			printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
			m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
			generateErrorResponseBody( WAPE_BoxError_InvalidNum );
			return WAPD_AnyError;
		}
	}

	WAPD_DEBUG1("si32_Num = %d\n", si32_Num);
	si32_Ret = WAPD_NoError;
	return si32_Ret;
}

/**
 * @brief リクエストボディからボックス名を取得する
 * @param[in]	param サブコントローラー
 * @param[out]	sc_BoxName ボックス名
 * @return 処理の成否, WAPD_NoError(0)：成功、WAPD_AnyError(1)：失敗
 */
int32_t WAPC_UserBoxController::getBoxNameFromBody( WAPS_SubController *params, char (&sc_BoxName)[WAPD_BoxNameMaxLength] )
{
	if( NULL == params->Param ) {
		WAPG_API_trace("%s(%d) %s : Body is NULL \n", __FILE__, __LINE__, __FUNCTION__);
		printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
		m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
		generateErrorResponseBody( WAPE_BoxError_InvalidJsonSyntax );
		return WAPD_AnyError;
	}
	if( !params->Param->isObject() ) {
		WAPG_API_trace("%s(%d) %s : Body is not Json Object \n", __FILE__, __LINE__, __FUNCTION__);
		printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
		m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
		generateErrorResponseBody( WAPE_BoxError_InvalidJsonSyntax );
		return WAPD_AnyError;
	}

	int32_t si32_Ret = WAPD_AnyError;

	if( params->Param->isMember("box_name") ) {
		Json::Value& box_name = (*params->Param)["box_name"];
		if( box_name.type() != Json::stringValue ) {
			WAPG_API_trace("%s(%d) %s : Illegal json value type : box_name \n", __FILE__, __LINE__, __FUNCTION__);
			printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
			m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
			generateErrorResponseBody( WAPE_BoxError_InvalidJsonSyntax );
			return WAPD_AnyError;
		}

		memset( sc_BoxName, 0, WAPD_BoxNameMaxLength );
		strncpy( sc_BoxName, box_name.asString().c_str(), (WAPD_BoxNameMaxLength - 1) );
		std::string str_BoxName = sc_BoxName;
		// バイト数チェック.
		if ( ( WAPD_BoxNameMinChar > str_BoxName.length() ) || ( (WAPD_BoxNameMaxLength - 1) < str_BoxName.length() ) ){
			WAPG_API_trace("%s(%d) %s : Illegal password length Byte\n", __FILE__, __LINE__, __FUNCTION__);
			printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
			m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
			generateErrorResponseBody( WAPE_BoxError_InvalidBoxName );
			return WAPD_AnyError;
		}
		
		ushort us_StringNum = 0;
		Bool b_CheckRet = getStringLengthUTF8(sc_BoxName, WAPE_CheckString_1String, us_StringNum);
		if(False  == b_CheckRet){
			m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
			printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
			generateErrorResponseBody( WAPE_BoxError_None );
			return WAPD_AnyError;
		}

		// 1～20文字以内であること.
		if ( ( WAPD_BoxNameMinChar > us_StringNum ) || ( WAPD_BoxNameMaxChar < us_StringNum ) ){
			WAPG_API_trace("%s(%d) %s : Illegal password length \n", __FILE__, __LINE__, __FUNCTION__);
			printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
			m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
			generateErrorResponseBody( WAPE_BoxError_InvalidBoxName );
			return WAPD_AnyError;
		}
	}

	WAPD_DEBUG1("sc_BoxName = %s\n", sc_BoxName);
	si32_Ret = WAPD_NoError;
	return si32_Ret;
}

/**
 * @brief リクエストボディから文書削除時間を取得する
 * @param[in]	param サブコントローラー
 * @param[out]	sc_BoxName 文書削除時間
 * @return 処理の成否, WAPD_NoError(0)：成功、WAPD_AnyError(1)：失敗、WAPD_NoSetting(2)：設定なし
 */
int32_t WAPC_UserBoxController::getAutoDeleteTimeFromBody( WAPS_SubController *params, int32_t& si32_DeleteTime)
{
	int32_t si32_Ret = WAPD_AnyError;
	if( NULL == params->Param ) {
		WAPG_API_trace("%s(%d) %s : Body is NULL \n", __FILE__, __LINE__, __FUNCTION__);
		printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
		m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
		generateErrorResponseBody( WAPE_BoxError_InvalidJsonSyntax );
		return WAPD_AnyError;
	}
	if( !params->Param->isObject() ) {
		WAPG_API_trace("%s(%d) %s : Body is not Json Object \n", __FILE__, __LINE__, __FUNCTION__);
		printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
		m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
		generateErrorResponseBody( WAPE_BoxError_InvalidJsonSyntax );
		return WAPD_AnyError;
	}

	if( params->Param->isMember("auto_document_delete_time") ) {
		Json::Value& delete_time = (*params->Param)["auto_document_delete_time"];
		if( !delete_time.isObject() ) {
			WAPG_API_trace("%s(%d) %s : Illegal json type : auto_document_delete_time \n", __FILE__, __LINE__, __FUNCTION__);
			printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
			m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
			generateErrorResponseBody( WAPE_BoxError_InvalidJsonSyntax );
			return WAPD_AnyError;
		}

		if(delete_time.isMember("type")){
			Json::Value& type = delete_time["type"];

			if( type.type() != Json::stringValue ) {
				WAPG_API_trace("%s(%d) %s : Illegal json type : type \n", __FILE__, __LINE__, __FUNCTION__);
				printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
				m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
				generateErrorResponseBody( WAPE_BoxError_InvalidJsonSyntax );
				return WAPD_AnyError;
			}
			if( WAPD_MaxKeyLength <= type.asString().length() ) {
				WAPG_API_trace("%s(%d) %s : Illegal auto_document_delete_time value \n", __FILE__, __LINE__, __FUNCTION__);
				printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
				m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
				generateErrorResponseBody( WAPE_BoxError_InvalidAutoDeleteTime );
				return WAPD_AnyError;
			}

			if( WAPD_BOX_STR_MIN == type.asString() ){
				// WAPD_BOX_MINの場合、timeの値を取得.
				if(delete_time.isMember("time")){
					Json::Value& time = delete_time["time"];
					
					if(time.type() != Json::intValue){
						WAPG_API_trace("%s(%d) %s : Illegal auto_document_delete_time value \n", __FILE__, __LINE__, __FUNCTION__);
						printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
						m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
						generateErrorResponseBody( WAPE_BoxError_InvalidJsonSyntax );
						return WAPD_AnyError;
					}

					si32_DeleteTime = time.asInt();

					// 範囲チェック(5～720).
					if( ( WAPD_BOX_5MIN > si32_DeleteTime ) || ( WAPD_BOX_12HOUR < si32_DeleteTime ) ) {
						WAPG_API_trace("%s(%d) %s : Illegal auto_document_delete_time value \n", __FILE__, __LINE__, __FUNCTION__);
						printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
						m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
						generateErrorResponseBody( WAPE_BoxError_InvalidAutoDeleteTime );
						return WAPD_AnyError;
					}

				} else {
					WAPG_API_trace("%s(%d) %s : Illegal auto_document_delete_time value \n", __FILE__, __LINE__, __FUNCTION__);
					printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
					m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
					generateErrorResponseBody( WAPE_BoxError_InvalidAutoDeleteTime );
					return WAPD_AnyError;
				}
			} else {
				char sc_DeleteType[WAPD_MaxKeyLength] = {0};
				APIE_BoxOptionDelTime e_DeleteTime = APIE_no_del;
				strncpy(sc_DeleteType, type.asString().c_str(), sizeof(sc_DeleteType) - 1 );
				if(False == WAPS_CommonTableString<WAPE_DelTimeStr, APIE_BoxOptionDelTime>::WapStringToCommon(
					sc_DeleteType, e_DeleteTime )){
					WAPG_API_trace("%s(%d) %s : Illegal auto_document_delete_time value \n", __FILE__, __LINE__, __FUNCTION__);
					printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
					m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
					generateErrorResponseBody( WAPE_BoxError_InvalidAutoDeleteTime );
					return WAPD_AnyError;
				}

				si32_DeleteTime = static_cast<int32_t>(e_DeleteTime);
			}
		} else {
			// type 省略時はNG.
			WAPG_API_trace("%s(%d) %s : Illegal json type : type is nothing \n", __FILE__, __LINE__, __FUNCTION__);
			printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
			m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
			generateErrorResponseBody( WAPE_BoxError_InvalidJsonSyntax );
			return WAPD_AnyError;
		}
	} else {
		return WAPD_NoSetting;
	}

	WAPD_DEBUG1("si32_DeleteTime = %d\n", si32_DeleteTime);
	si32_Ret = WAPD_NoError;
	return si32_Ret;
}

/**
 * @brief リクエストボディから所有者名情報を取得する
 * @param[in]	param サブコントローラー
 * @param[out]	str_OwnerName 所有者名
 * @param[out]	str_ServerName サーバー名
 * @return 処理の成否, WAPD_NoError(0)：成功、WAPD_AnyError(1)：失敗
 */
int32_t WAPC_UserBoxController::getOwnerInformationFromBody( WAPS_SubController *params, std::string& str_OwnerName, std::string& str_ServerName )
{
	if( NULL == params->Param ) {
		WAPG_API_trace("%s(%d) %s : Body is NULL \n", __FILE__, __LINE__, __FUNCTION__);
		printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
		m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
		generateErrorResponseBody( WAPE_BoxError_InvalidJsonSyntax );
		return WAPD_AnyError;
	}
	if( !params->Param->isObject() ) {
		WAPG_API_trace("%s(%d) %s : Body is not Json Object \n", __FILE__, __LINE__, __FUNCTION__);
		printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
		m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
		generateErrorResponseBody( WAPE_BoxError_InvalidJsonSyntax );
		return WAPD_AnyError;
	}
	Json::Value& val_Root = *params->Param;

	// 所有者名.
	if( val_Root.isMember("owner_name") ) {
		Json::Value& owner = val_Root["owner_name"];

		if( owner.type() != Json::stringValue ) {
			WAPG_API_trace("%s(%d) %s : Illegal json type : owner_name \n", __FILE__, __LINE__, __FUNCTION__);
			printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
			m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
			generateErrorResponseBody( WAPE_BoxError_InvalidJsonSyntax );
			return WAPD_AnyError;
		}
		str_OwnerName = owner.asString();

		// バイト数チェック.
		if ( ( WAPD_BoxOwnerNameMinChar > str_OwnerName.length() ) ||
				( (WAPD_BoxOwnerNameMaxLength - 1) < str_OwnerName.length() ) ){
			WAPG_API_trace("%s(%d) %s : Illegal BoxOwnerName length Byte\n", __FILE__, __LINE__, __FUNCTION__);
			printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
			m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
			generateErrorResponseBody( WAPE_BoxError_InvalidBoxOwnerName );
			return WAPD_AnyError;
		}

		ushort us_StringNum = 0;
		Bool b_CheckRet = getStringLengthUTF8(str_OwnerName.c_str(), WAPE_CheckString_1String, us_StringNum);
		if(False  == b_CheckRet){
			WAPG_API_trace("%s(%d) %s : getStringLengthUTF8 failed \n", __FILE__, __LINE__, __FUNCTION__);
			m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
			printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
			generateErrorResponseBody( WAPE_BoxError_None );
			return WAPD_AnyError;
		}

		// 1～64文字以内であること.
		if ( ( WAPD_BoxOwnerNameMinChar > us_StringNum ) ||
				( WAPD_BoxOwnerNameMaxChar < us_StringNum ) ){
			WAPG_API_trace("%s(%d) %s : Illegal BoxOwnerName length \n", __FILE__, __LINE__, __FUNCTION__);
			printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
			m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
			generateErrorResponseBody( WAPE_BoxError_InvalidBoxOwnerName );
			return WAPD_AnyError;
		}
	}

	//サーバー名.
	if( val_Root.isMember("server_name") ) {
		Json::Value& server = val_Root["server_name"];

		if( server.type() != Json::stringValue ) {
			WAPG_API_trace("%s(%d) %s : Illegal json type : server_name \n", __FILE__, __LINE__, __FUNCTION__);
			printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
			m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
			generateErrorResponseBody( WAPE_BoxError_InvalidJsonSyntax );
			return WAPD_AnyError;
		}
		str_ServerName = server.asString();

		// バイト数チェック.
		if ( ( WAPD_ServerNameMinChar > str_ServerName.length() ) || 
				( WAPD_ServerNameMaxChar < str_ServerName.length() ) ){
			WAPG_API_trace("%s(%d) %s : Illegal ServerName length Byte\n", __FILE__, __LINE__, __FUNCTION__);
			printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
			m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
			generateErrorResponseBody( WAPE_BoxError_InvalidServerName );
			return WAPD_AnyError;
		}

		// マルチバイト文字が含まれる場合エラー.
		int32_t si32_Ret = WAPD_AnyError;
		Bool b_CheckRet = False;
		si32_Ret = containsMultiByteCharacterUTF8( str_ServerName.c_str(), b_CheckRet );
		if( WAPD_NoError != si32_Ret ) {
			m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
			printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
			generateErrorResponseBody( WAPE_BoxError_None );
			return WAPD_AnyError;
		}
		if( True == b_CheckRet ){
			WAPG_API_trace("%s(%d) %s : ServerName contains multi-byte character \n", __FILE__, __LINE__, __FUNCTION__);
			printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
			m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
			generateErrorResponseBody( WAPE_BoxError_InvalidServerName );
			return WAPD_AnyError;
		}

		// 1～32文字以内であること.
		if ( ( WAPD_ServerNameMinChar > str_ServerName.length() ) || 
				( WAPD_ServerNameMaxChar < str_ServerName.length() ) ){
			WAPG_API_trace("%s(%d) %s : Illegal ServerName length \n", __FILE__, __LINE__, __FUNCTION__);
			printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
			m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
			generateErrorResponseBody( WAPE_BoxError_InvalidServerName );
			return WAPD_AnyError;
		}
	}

	WAPD_DEBUG2("OwnerName = %s, ServerName = %s\n", str_OwnerName.c_str(), str_ServerName.c_str());
	return WAPD_NoError;
}

/**
 * @brief リクエストボディからボックス種別関連情報を取得する
 * @param[in]	param サブコントローラー
 * @param[out]	str_BoxType ボックス種別
 * @return 処理の成否, WAPD_NoError(0)：成功、WAPD_AnyError(1)：失敗
 */
int32_t WAPC_UserBoxController::getBoxTypeFromBody( WAPS_SubController *params, std::string& str_BoxType )
{
	if( NULL == params->Param ) {
		WAPG_API_trace("%s(%d) %s : Body is NULL \n", __FILE__, __LINE__, __FUNCTION__);
		printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
		m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
		generateErrorResponseBody( WAPE_BoxError_InvalidJsonSyntax );
		return WAPD_AnyError;
	}
	if( !params->Param->isObject() ) {
		WAPG_API_trace("%s(%d) %s : Body is not Json Object \n", __FILE__, __LINE__, __FUNCTION__);
		printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
		m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
		generateErrorResponseBody( WAPE_BoxError_InvalidJsonSyntax );
		return WAPD_AnyError;
	}
	Json::Value& val_Root = *params->Param;

	// ボックス種別.
	if( val_Root.isMember("type") ) {
		Json::Value& type = val_Root["type"];

		if( type.type() != Json::stringValue ) {
			WAPG_API_trace("%s(%d) %s : Illegal json type : type \n", __FILE__, __LINE__, __FUNCTION__);
			printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
			m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
			generateErrorResponseBody( WAPE_BoxError_InvalidJsonSyntax );
			return WAPD_AnyError;
		}

		str_BoxType = type.asString();

		if( ( str_BoxType != "PUBLIC" )
			&& ( str_BoxType != "PERSONAL" )
			&& ( str_BoxType != "GROUP" ) ) {
			WAPG_API_trace("%s(%d) %s : not supported box type\n", __FILE__, __LINE__, __FUNCTION__);
			printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
			m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
			generateErrorResponseBody( WAPE_BoxError_InvalidBoxKind );
			return WAPD_AnyError;
		}
	}

	WAPD_DEBUG1("BoxType = %s\n", str_BoxType.c_str());
	return WAPD_NoError;
}

/**
 * @brief リクエストボディから検索キー種別を取得する
 * @param[in]	param サブコントローラー
 * @param[out]	sc_str_SearchKey 検索キー種別
 * @return 処理の成否, WAPD_NoError(0)：成功、WAPD_AnyError(1)：失敗
 */
int32_t WAPC_UserBoxController::getSearchKeyFromBody( WAPS_SubController *params, string& str_SearchKey )
{
	int32_t si32_Ret = WAPD_AnyError;

	if( NULL == params->Param ) {
		WAPG_API_trace("%s(%d) %s : Body is NULL \n", __FILE__, __LINE__, __FUNCTION__);
		printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
		m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
		generateErrorResponseBody( WAPE_BoxError_InvalidJsonSyntax );
		return WAPD_AnyError;
	}
	if( !params->Param->isObject() ) {
		WAPG_API_trace("%s(%d) %s : Body is not Json Object \n", __FILE__, __LINE__, __FUNCTION__);
		printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
		m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
		generateErrorResponseBody( WAPE_BoxError_InvalidJsonSyntax );
		return WAPD_AnyError;
	}

	if( params->Param->isMember("search_key") ) {
		Json::Value& search_key = (*params->Param)["search_key"];
		if( search_key.type() != Json::stringValue ) {
			WAPG_API_trace("%s(%d) %s : Illegal json value type : search_key \n", __FILE__, __LINE__, __FUNCTION__);
			printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
			m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
			generateErrorResponseBody( WAPE_BoxError_InvalidJsonSyntax );
			return WAPD_AnyError;
		}
		if( WAPD_MaxKeyLength <= search_key.asString().length() ) {
			WAPG_API_trace("%s(%d) %s : Too long : search_key \n", __FILE__, __LINE__, __FUNCTION__);
			printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
			m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
			generateErrorResponseBody( WAPE_BoxError_InvalidSearchKey );
			return WAPD_AnyError;
		}

		str_SearchKey = search_key.asString();
	}

	WAPD_DEBUG1("str_SearchKey = %s\n", str_SearchKey.c_str());
	si32_Ret = WAPD_NoError;
	return si32_Ret;
}

/**
 * @brief リクエストボディからボックスパスワード情報を取得する
 * @param[in]	param サブコントローラー
 * @param[in]	pc_Box		ボックスインスタンス
 * @param[in]	pc_User		ユーザーインスタンス
 * @param[in]	pc_Track	部門インスタンス
 * @param[in]	psc_BoxPassword	ボックスパスワード
 * @param[out]	s_OwnerName ボックスパスワード設定情報
 * @return 処理の成否, WAPD_NoError(0)：成功、WAPD_AnyError(1)：失敗
 */
int32_t WAPC_UserBoxController::getBoxPasswordInformationFromBody(
	WAPS_SubController *params,
	APIC_Box* pc_Box,
	APIC_User* pc_User,
	APIC_Track* pc_Track,
	char* psc_BoxPassword,
	WAPS_BoxPasswordInformation& s_BoxPassword
	)
{
	int32_t si32_Ret = WAPD_AnyError;

	if( NULL == params->Param ) {
		// ボディが無い場合でもエラーにしない.
	} else {
		if( !params->Param->isObject() ) {
			WAPG_API_trace("%s(%d) %s : Body is not Json Object \n", __FILE__, __LINE__, __FUNCTION__);
			printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
			m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
			generateErrorResponseBody( WAPE_BoxError_InvalidJsonSyntax );
			return WAPD_AnyError;
		}

		if( params->Param->isMember("box_password_info") ) {
			Json::Value& box_password_info = (*params->Param)["box_password_info"];
			if( !box_password_info.isObject() ) {
				WAPG_API_trace("%s(%d) %s : Illegal json type : box_password_info \n", __FILE__, __LINE__, __FUNCTION__);
				printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
				m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
				generateErrorResponseBody( WAPE_BoxError_InvalidJsonSyntax );
				return WAPD_AnyError;
			}

			// 旧ユーザーボックスパスワード.
			if( box_password_info.isMember("old_box_password") ) {
				Json::Value& oldPass = box_password_info["old_box_password"];

				if( oldPass.type() != Json::stringValue ) {
					WAPG_API_trace("%s(%d) %s : Illegal json type : oldPass \n", __FILE__, __LINE__, __FUNCTION__);
					printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
					m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
					generateErrorResponseBody( WAPE_BoxError_InvalidJsonSyntax );
					return WAPD_AnyError;
				}
				s_BoxPassword.str_OldPassword = oldPass.asString();

				ushort us_StringNum = 0;

				// バイト数チェック.
				if ( (WAPD_BoxPasswordMaxLength - 1) < s_BoxPassword.str_OldPassword.length() ){
					WAPG_API_trace("%s(%d) %s : Illegal password length Byte\n", __FILE__, __LINE__, __FUNCTION__);
					printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
					m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
					generateErrorResponseBody( WAPE_BoxError_InvalidPassword );
					return WAPD_AnyError;
				}

				// 文字数チェック.
				Bool b_CheckRet = False;
				b_CheckRet = getStringLengthUTF8(s_BoxPassword.str_OldPassword.c_str(), WAPE_CheckString_1String, us_StringNum);
				if(False  == b_CheckRet){
					m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
					printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
					generateErrorResponseBody( WAPE_BoxError_None );
					return WAPD_AnyError;
				}

				if ( ( WAPD_BoxPasswordMinChar > us_StringNum ) || ( WAPD_BoxPasswordMaxChar < us_StringNum ) ){
					WAPG_API_trace("%s(%d) %s : Illegal password length \n", __FILE__, __LINE__, __FUNCTION__);
					printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
					m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
					generateErrorResponseBody( WAPE_BoxError_InvalidPassword );
					return WAPD_AnyError;
				}
			}

			// 新ユーザーボックスパスワード.
			if( box_password_info.isMember("new_box_password") ) {
				Json::Value& newPass = box_password_info["new_box_password"];

				if( newPass.type() != Json::stringValue ) {
					WAPG_API_trace("%s(%d) %s : Illegal json type : newPass \n", __FILE__, __LINE__, __FUNCTION__);
					printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
					m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
					generateErrorResponseBody( WAPE_BoxError_InvalidJsonSyntax );
					return WAPD_AnyError;
				}
				s_BoxPassword.str_NewPassword = newPass.asString();

				ushort us_StringNum = 0;

				// バイト数チェック.
				if ( (WAPD_BoxPasswordMaxLength - 1) < s_BoxPassword.str_NewPassword.length() ){
					WAPG_API_trace("%s(%d) %s : Illegal password length Byte\n", __FILE__, __LINE__, __FUNCTION__);
					printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
					m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
					generateErrorResponseBody( WAPE_BoxError_InvalidPassword );
					return WAPD_AnyError;
				}

				// 文字数チェック.
				Bool b_CheckRet = False;
				b_CheckRet = getStringLengthUTF8(s_BoxPassword.str_NewPassword.c_str(), WAPE_CheckString_1String, us_StringNum);
				if(False  == b_CheckRet){
					m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
					printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
					generateErrorResponseBody( WAPE_BoxError_None );
					return WAPD_AnyError;
				}

				if ( ( WAPD_BoxPasswordMinChar > us_StringNum ) || ( WAPD_BoxPasswordMaxChar < us_StringNum ) ){
					WAPG_API_trace("%s(%d) %s : Illegal password length \n", __FILE__, __LINE__, __FUNCTION__);
					printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
					m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
					generateErrorResponseBody( WAPE_BoxError_InvalidPassword );
					return WAPD_AnyError;
				}

				// ボックスオプション取得.
				APIC_BoxOption c_BoxOption;
				int32_t si32_ReturnValue = APID_DocapiCommonErr;
				si32_ReturnValue = pc_Box->getBoxOption(
					pc_User,					// ユーザー情報.
					psc_BoxPassword,			// ボックスパスワード.
					&c_BoxOption,				// ボックスオプション情報.
					pc_Track);					// 部門情報.

				switch(si32_ReturnValue) {
					case APID_DocapiMethodOk:
						break;

					case APID_DocapiOwnerAccessErr:		// オーナー名不正エラー.
						WAPG_API_trace("%s(%d) %s : getBoxOption APID_DocapiOwnerAccessErr \n", __FILE__, __LINE__, __FUNCTION__);
						m_ResponseHeader->Status = WAPD_STATUS_FORBIDDEN;
						generateErrorResponseBody( WAPE_BoxError_OwnerAccessErr );
						return WAPD_AnyError;

					case APID_DocapiBoxidUnknownErr:	// 指定番号のBoxなしエラー.
						WAPG_API_trace("%s(%d) %s : getBoxOption APID_DocapiBoxidUnknownErr \n", __FILE__, __LINE__, __FUNCTION__);
						printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
						m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
						generateErrorResponseBody( WAPE_BoxError_BoxNotExist );
						return WAPD_AnyError;

					case APID_DocapiPasswordAccessErr:	// パスワード不正.
					case APID_DocapiCommonErr:			// その他の処理失敗エラー.
					default:
						WAPG_API_trace("%s(%d) %s : getBoxOption failed \n", __FILE__, __LINE__, __FUNCTION__);
						m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
						generateErrorResponseBody( WAPE_BoxError_None );
						printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
						return WAPD_AnyError;
				}

				// パスワード規約チェック.
				// ボックス種別が共有ボックスの場合のみチェック.
				if( APIE_BoxmakerAuto == c_BoxOption.sl_BoxCreateInfo ) {
					if( WAPE_PasswordRules_On_Invalid == checkPasswordRules( s_BoxPassword.str_NewPassword.c_str() ) ) {
						WAPG_API_trace("%s(%d) %s : PasswordRulesOn check NG.\n", __FILE__, __LINE__, __FUNCTION__);
						printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
						m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
						generateErrorResponseBody( WAPE_BoxError_InvalidPassword );
						return WAPD_AnyError;
					}
				}
			}
		}
	}

	WAPD_DEBUG2("str_OldPassword = %s, str_NewPassword = %s\n", s_BoxPassword.str_OldPassword.c_str(), s_BoxPassword.str_NewPassword.c_str());
	si32_Ret = WAPD_NoError;
	return si32_Ret;
}

/**
 * @brief リクエストボディから親展受信機能情報を取得する
 * @param[in]	param サブコントローラー
 * @param[out]	s_OwnerName 親展受信機能情報
 * @return 処理の成否, WAPD_NoError(0)：成功、WAPD_AnyError(1)：失敗
 */
int32_t WAPC_UserBoxController::getConfidentialRxFromBody( WAPS_SubController *params, WAPS_ConfidentialRx& s_ConfidentialRx)
{
	int32_t si32_Ret = WAPD_AnyError;
	s_ConfidentialRx.sc_Select = WAPD_SelectNoSetting;

	if( NULL == params->Param ) {
		WAPG_API_trace("%s(%d) %s : Body is NULL \n", __FILE__, __LINE__, __FUNCTION__);
		printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
		m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
		generateErrorResponseBody( WAPE_BoxError_InvalidJsonSyntax );
		return WAPD_AnyError;
	}
	if( !params->Param->isObject() ) {
		WAPG_API_trace("%s(%d) %s : Body is not Json Object \n", __FILE__, __LINE__, __FUNCTION__);
		printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
		m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
		generateErrorResponseBody( WAPE_BoxError_InvalidJsonSyntax );
		return WAPD_AnyError;
	}

	if( params->Param->isMember("confidential_rx") ) {
		Json::Value& confidential_rx = (*params->Param)["confidential_rx"];
		if( !confidential_rx.isObject() ) {
			WAPG_API_trace("%s(%d) %s : Illegal json type : confidential_rx \n", __FILE__, __LINE__, __FUNCTION__);
			printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
			m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
			generateErrorResponseBody( WAPE_BoxError_InvalidJsonSyntax );
			return WAPD_AnyError;
		}

		// 親展受信機能可否.
		if( confidential_rx.isMember("select") ) {
			Json::Value& select = confidential_rx["select"];

			if(!select.isBool()){
				WAPG_API_trace("%s(%d) %s : Illegal json type : select \n", __FILE__, __LINE__, __FUNCTION__);
				printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
				m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
				generateErrorResponseBody( WAPE_BoxError_InvalidJsonSyntax );
				return WAPD_AnyError;
			}
			s_ConfidentialRx.sc_Select = static_cast<Bool>(select.asBool());
		} else {
			s_ConfidentialRx.sc_Select = WAPD_SelectNoSetting;
		}

		// 旧親展受信パスワード.
		if( confidential_rx.isMember("old_confidential_rx_password") ) {
			Json::Value& oldPass = confidential_rx["old_confidential_rx_password"];

			if( oldPass.type() != Json::stringValue ) {
				WAPG_API_trace("%s(%d) %s : Illegal json type : oldPass \n", __FILE__, __LINE__, __FUNCTION__);
				printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
				m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
				generateErrorResponseBody( WAPE_BoxError_InvalidJsonSyntax );
				return WAPD_AnyError;
			}
			s_ConfidentialRx.str_OldPassword = oldPass.asString();

			// バイト数チェック.
			if ( WAPD_ConfPasswordMaxChar < s_ConfidentialRx.str_OldPassword.length() ){
				WAPG_API_trace("%s(%d) %s : Illegal password length Byte\n", __FILE__, __LINE__, __FUNCTION__);
				printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
				m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
				generateErrorResponseBody( WAPE_BoxError_InvalidConfidentialRxPassword );
				return WAPD_AnyError;
			}

			// マルチバイト文字が含まれる場合エラー.
			Bool b_CheckRet = False;
			si32_Ret = containsMultiByteCharacterUTF8( s_ConfidentialRx.str_OldPassword.c_str(), b_CheckRet );
			if( WAPD_NoError != si32_Ret ) {
				m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
				printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
				generateErrorResponseBody( WAPE_BoxError_None );
				return WAPD_AnyError;
			}
			if( True == b_CheckRet ){
				WAPG_API_trace("%s(%d) %s : password contains multi-byte character \n", __FILE__, __LINE__, __FUNCTION__);
				printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
				m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
				generateErrorResponseBody( WAPE_BoxError_InvalidConfidentialRxPassword );
				return WAPD_AnyError;
			}

			// 文字数チェック.
			ushort us_StringNum = 0;
			b_CheckRet = getStringLengthUTF8(s_ConfidentialRx.str_OldPassword.c_str(), WAPE_CheckString_1String, us_StringNum);
			if(False  == b_CheckRet){
				m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
				printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
				generateErrorResponseBody( WAPE_BoxError_None );
				return WAPD_AnyError;
			}

			if ( ( WAPD_ConfPasswordMinChar > us_StringNum ) || ( WAPD_ConfPasswordMaxChar < us_StringNum ) ){
				WAPG_API_trace("%s(%d) %s : Illegal password length \n", __FILE__, __LINE__, __FUNCTION__);
				printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
				m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
				generateErrorResponseBody( WAPE_BoxError_InvalidConfidentialRxPassword );
				return WAPD_AnyError;
			}
		}

		// 新親展受信パスワード.
		if( confidential_rx.isMember("new_confidential_rx_password") ) {
			Json::Value& newPass = confidential_rx["new_confidential_rx_password"];

			if( newPass.type() != Json::stringValue ) {
				WAPG_API_trace("%s(%d) %s : Illegal json type : newPass \n", __FILE__, __LINE__, __FUNCTION__);
				printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
				m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
				generateErrorResponseBody( WAPE_BoxError_InvalidJsonSyntax );
				return WAPD_AnyError;
			}
			s_ConfidentialRx.str_NewPassword = newPass.asString();

			// バイト数チェック.
			if ( WAPD_ConfPasswordMaxChar < s_ConfidentialRx.str_NewPassword.length() ){
				WAPG_API_trace("%s(%d) %s : Illegal password length Byte\n", __FILE__, __LINE__, __FUNCTION__);
				printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
				m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
				generateErrorResponseBody( WAPE_BoxError_InvalidConfidentialRxPassword );
				return WAPD_AnyError;
			}

			// マルチバイト文字が含まれる場合エラー.
			Bool b_CheckRet = False;
			si32_Ret = containsMultiByteCharacterUTF8( s_ConfidentialRx.str_NewPassword.c_str(), b_CheckRet );
			if( WAPD_NoError != si32_Ret ) {
				m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
				printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
				generateErrorResponseBody( WAPE_BoxError_None );
				return WAPD_AnyError;
			}
			if( True == b_CheckRet ){
				WAPG_API_trace("%s(%d) %s : password contains multi-byte character \n", __FILE__, __LINE__, __FUNCTION__);
				printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
				m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
				generateErrorResponseBody( WAPE_BoxError_InvalidConfidentialRxPassword );
				return WAPD_AnyError;
			}

			// 文字数チェック.
			ushort us_StringNum = 0;
			b_CheckRet = getStringLengthUTF8(s_ConfidentialRx.str_NewPassword.c_str(), WAPE_CheckString_1String, us_StringNum);
			if(False  == b_CheckRet){
				m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
				printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
				generateErrorResponseBody( WAPE_BoxError_None );
				return WAPD_AnyError;
			}

			if ( ( WAPD_ConfPasswordMinChar > us_StringNum ) || ( WAPD_ConfPasswordMaxChar < us_StringNum ) ){
				WAPG_API_trace("%s(%d) %s : Illegal password length \n", __FILE__, __LINE__, __FUNCTION__);
				printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
				m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
				generateErrorResponseBody( WAPE_BoxError_InvalidConfidentialRxPassword );
				return WAPD_AnyError;
			}
			
			// パスワード規約チェック.
			if( WAPE_PasswordRules_On_Invalid == checkPasswordRules( s_ConfidentialRx.str_NewPassword.c_str() ) ) {
				WAPG_API_trace("%s(%d) %s : PasswordRulesOn check NG.\n", __FILE__, __LINE__, __FUNCTION__);
				printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
				m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
				generateErrorResponseBody( WAPE_BoxError_InvalidConfidentialRxPassword );
				return WAPD_AnyError;
			}
		}

		if((0 < s_ConfidentialRx.str_OldPassword.length()) &&
		   (0 < s_ConfidentialRx.str_NewPassword.length())){

			// 新旧パスワードが等しい.
			if(s_ConfidentialRx.str_OldPassword == s_ConfidentialRx.str_NewPassword){
				WAPG_API_trace("%s(%d) %s : Illegal password\n", __FILE__, __LINE__, __FUNCTION__);
				printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
				m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
				generateErrorResponseBody( WAPE_BoxError_InvalidConfidentialRxPassword );
				return WAPD_AnyError;
			}
		}
	}

	WAPD_DEBUG2("str_OldPassword = %s, str_NewPassword = %s\n", s_ConfidentialRx.str_OldPassword.c_str(), s_ConfidentialRx.str_NewPassword.c_str());
	si32_Ret = WAPD_NoError;
	return si32_Ret;
}

/**
 * @brief 所有者番号を取得する（ボックス種別/ボックス所有者変更用）
 * @param[in]	pc_User	実行ユーザー
 * @param[in]	pc_Track 実行ユーザー所属部門
 * @param[in]	e_BoxCreateInfo	ボックス種別
 * @param[in]	str_OwnerName	所有者名
 * @param[in]	str_ServerName	サーバー名
 * @param[out]	si_OwnerNumber	所有者番号
 * @return 処理の成否, WAPD_NoError(0)：成功、WAPD_AnyError(1)：失敗
 */
int32_t WAPC_UserBoxController::getNewOwnerNumber(
	APIC_User* pc_User,
	APIC_Track* pc_Track,
	APIE_BoxOptionCreateInfo e_BoxCreateInfo,
	std::string& str_OwnerName,
	std::string& str_ServerName,
	int32_t& si32_OwnerNumber )
{
	int32_t si32_Ret = WAPD_AnyError;

	// ボックス種別が個人.
	if( APIE_BoxmakerUser == e_BoxCreateInfo ){
		if( ( TYPE_UserAuthenticationDeviceAndMidAuth == APIC_UserAuthentication::newInstance()->getUserAuthentication() ) &&
			( WAPE_BoxUserType_User_Mid == getBoxUserType( pc_User, pc_Track ) ) ) {
			// 本体＋拡張認証時で、実行ユーザーが拡張ユーザーの場合.
			si32_OwnerNumber = APIC_User::getUserNoSearchByUserName( str_OwnerName.c_str(), APID_MiddleAuthServerIndex );
		}
		else {
			si32_OwnerNumber = APIC_User::getUserNoSearchByServerName( str_OwnerName.c_str(), str_ServerName.c_str() );
		}

		if( ( si32_OwnerNumber == 0 ) || 
			( si32_OwnerNumber == TYPD_IndexUserAdmin ) ||
			( si32_OwnerNumber == TYPD_IndexUserCE ) || 
			( si32_OwnerNumber == TYPD_IndexUserOther ) || 
			( si32_OwnerNumber == TYPD_IndexUserLMU ) ) {
			// ボックスを所持しえないユーザーである.
			WAPG_API_trace("%s(%d) %s : invalid user. si32_OwnerNumber=0x%08X, str_OwnerName=%s, str_ServerName=%s\n", __FILE__, __LINE__, __FUNCTION__, si32_OwnerNumber, str_OwnerName.c_str(), str_ServerName.c_str());
			printf("403:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //403->ErrorStatus[WAPD_STATUS_FORBIDDEN] in WAP_defs.h
			m_ResponseHeader->Status = WAPD_STATUS_FORBIDDEN;
			generateErrorResponseBody( WAPE_BoxError_OwnerAccessErr );
			return WAPD_AnyError;
		}
	// ボックス種別がグループ.
	} else if( APIE_BoxmakerGroup == e_BoxCreateInfo ) {
		si32_OwnerNumber = APIC_Track::getTrackNoSearchByTrackName(str_OwnerName.c_str());
		if( ( si32_OwnerNumber == 0 ) || 
			( si32_OwnerNumber == TYPD_IndexTrackBoxAdmin ) || 
			( si32_OwnerNumber == TYPD_IndexTrackAdmin ) ) {
			// ボックスを所持しえない部門である.
			WAPG_API_trace("%s(%d) %s : invalid track. si32_OwnerNumber=0x%08X, str_OwnerName=%s, str_ServerName=%s\n", __FILE__, __LINE__, __FUNCTION__, si32_OwnerNumber, str_OwnerName.c_str(), str_ServerName.c_str());
			printf("403:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //403->ErrorStatus[WAPD_STATUS_FORBIDDEN] in WAP_defs.h
			m_ResponseHeader->Status = WAPD_STATUS_FORBIDDEN;
			generateErrorResponseBody( WAPE_BoxError_OwnerAccessErr );
			return WAPD_AnyError;
		}
	// ボックス種別が共有.
	} else {
		si32_OwnerNumber = APID_BoxUserID_Public;
	}

	WAPD_DEBUG2("si32_OwnerNumber = %d(0x%08X)\n", si32_OwnerNumber, si32_OwnerNumber);
	si32_Ret = WAPD_NoError;
	return si32_Ret;
}

/**
 * @brief ボックス名を設定する
 * @param[in]	params　サブコントローラー
 * @param[in]	pc_Box	ボックスインスタンス
 * @param[in]	pc_User		ユーザーインスタンス
 * @param[in]	pc_Track	部門インスタンス
 * @param[in]	psc_BoxPassword	ボックスパスワード
 * @return 処理の成否, WAPD_NoError(0)：成功、WAPD_AnyError(1)：失敗
 */
int32_t WAPC_UserBoxController::setBoxName( WAPS_SubController* params, APIC_Box* pc_Box, APIC_User* pc_User, APIC_Track* pc_Track, char* psc_BoxPassword )
{
	int32_t si32_Ret = WAPD_AnyError;
	int32_t si32_ReturnValue = APID_DocapiCommonErr;

	// ■ リクエストボディにユーザーボックス名が設定されている.
	char sc_BoxName[WAPD_BoxNameMaxLength] = {0};
	si32_Ret = getBoxNameFromBody( params, sc_BoxName );
	if(WAPD_NoError != si32_Ret){
		return WAPD_AnyError;
	}

	ushort us_BoxNameStringNum = 0;
	Bool b_CheckRet = False;
	b_CheckRet = getStringLengthUTF8(sc_BoxName, WAPE_CheckString_1String, us_BoxNameStringNum);
	if(False  == b_CheckRet){
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		generateErrorResponseBody( WAPE_BoxError_None );
		return WAPD_AnyError;
	}

	if( 0 < us_BoxNameStringNum ) {
		// ユーザーボックス名設定.
		si32_ReturnValue = pc_Box->changeBoxName(
			pc_User,
			psc_BoxPassword,
			sc_BoxName,
			(LIBE_CharSet)APID_DefaultCharSet,
			pc_Track);
		
		switch(si32_ReturnValue) {
			case APID_DocapiMethodOk:
				m_ResponseHeader->Status = WAPD_STATUS_OK;
				break;

			case APID_DocapiOwnerAccessErr:		// オーナー名不正エラー.
				printf("403:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //403->ErrorStatus[WAPD_STATUS_FORBIDDEN] in WAP_defs.h
				m_ResponseHeader->Status = WAPD_STATUS_FORBIDDEN;
				generateErrorResponseBody( WAPE_BoxError_OwnerAccessErr );
				return WAPD_AnyError;

			case APID_DocapiBoxidUnknownErr:	// 指定番号のBoxなしエラー.
				printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
				m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
				generateErrorResponseBody( WAPE_BoxError_BoxNotExist );
				return WAPD_AnyError;

			case APID_DocapiPasswordAccessErr:	// パスワード不正エラー.
			case APID_DocapiCommonErr:			// その他の処理失敗エラー.
			default:
				m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
				printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
				generateErrorResponseBody( WAPE_BoxError_None );
				return WAPD_AnyError;
			}	
	}

	return WAPD_NoError;
}

/**
 * @brief 文書自動削除時間を設定する
 * @param[in]	params　サブコントローラー
 * @param[in]	pc_Box	ボックスインスタンス
 * @param[in]	pc_User		ユーザーインスタンス
 * @param[in]	pc_Track	部門インスタンス
 * @param[in]	psc_BoxPassword	ボックスパスワード
 * @return 処理の成否, WAPD_NoError(0)：成功、WAPD_AnyError(1)：失敗
 */
int32_t WAPC_UserBoxController::setAutoDeleteTime( WAPS_SubController* params, APIC_Box* pc_Box, APIC_User* pc_User, APIC_Track* pc_Track, char* psc_BoxPassword )
{
	int32_t si32_Ret = WAPD_AnyError;
	int32_t si32_ReturnValue = APID_DocapiCommonErr;

	// ■ リクエストボディに文書削除時間設定情報が設定されている.
	int32_t si32_DeleteTime = 0;
	si32_Ret = getAutoDeleteTimeFromBody( params, si32_DeleteTime );
	if(WAPD_AnyError == si32_Ret){
		return WAPD_AnyError;
	} else if( WAPD_NoError == si32_Ret ){
		APIC_BoxOption c_BoxOption;
		si32_ReturnValue = pc_Box->getBoxOption(
			pc_User,					// ユーザー情報.
			psc_BoxPassword,			// ボックスパスワード.
			&c_BoxOption,				// ボックスオプション情報.
			pc_Track);					// 部門情報.

		switch(si32_ReturnValue) {
			case APID_DocapiMethodOk:
				break;

			case APID_DocapiOwnerAccessErr:		// オーナー名不正エラー.
				printf("403:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //403->ErrorStatus[WAPD_STATUS_FORBIDDEN] in WAP_defs.h
				m_ResponseHeader->Status = WAPD_STATUS_FORBIDDEN;
				generateErrorResponseBody( WAPE_BoxError_OwnerAccessErr );
				return WAPD_AnyError;

			case APID_DocapiBoxidUnknownErr:	// 指定番号のBoxなしエラー.
				printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
				m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
				generateErrorResponseBody( WAPE_BoxError_BoxNotExist );
				return WAPD_AnyError;

			case APID_DocapiPasswordAccessErr:	// パスワード不正.
			case APID_DocapiCommonErr:			// その他の処理失敗エラー.
			default:
				m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
				printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
				generateErrorResponseBody( WAPE_BoxError_None );
				return WAPD_AnyError;
		}

		c_BoxOption.sl_AutoDeleteTime = si32_DeleteTime;
		si32_ReturnValue = pc_Box->setBoxOption(
			pc_User,					// ユーザー情報.
			psc_BoxPassword,			// ボックスパスワード.
			&c_BoxOption,				// ボックスオプション情報.
			pc_Track,					// 部門情報.
			TYPE_RegistrationTypeOnlyOne,
			static_cast<short> ( params->AuthorityID ) );

		switch(si32_ReturnValue) {
			case APID_DocapiMethodOk:
				si32_Ret = WAPD_NoError;
				m_ResponseHeader->Status = WAPD_STATUS_OK;
				break;

			case APID_DocapiOwnerAccessErr:		// オーナー名不正エラー.
				printf("403:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //403->ErrorStatus[WAPD_STATUS_FORBIDDEN] in WAP_defs.h
				m_ResponseHeader->Status = WAPD_STATUS_FORBIDDEN;
				generateErrorResponseBody( WAPE_BoxError_OwnerAccessErr );
				return WAPD_AnyError;

			case APID_DocapiBoxidUnknownErr:	// 指定番号のBoxなしエラー.
				printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
				m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
				generateErrorResponseBody( WAPE_BoxError_BoxNotExist );
				return WAPD_AnyError;

			case APID_DocapiPasswordAccessErr:	// パスワード不正.
			case APID_DocapiCommonErr:			// その他の処理失敗エラー.
			default:
				m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
				printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
				generateErrorResponseBody( WAPE_BoxError_None );
				return WAPD_AnyError;
		}
	} else {
		// WAPD_NoSettingの場合は何もしない
	}

	return WAPD_NoError;
}

/**
 * @brief ボックス種別および所有者を設定する
 * @param[in]	params		サブコントローラー
 * @param[in]	pc_Box		ボックスインスタンス
 * @param[in]	pc_User		ユーザーインスタンス
 * @param[in]	pc_Track	部門インスタンス
 * @param[in]	psc_BoxPassword	ボックスパスワード
 * @return 処理の成否, WAPD_NoError(0)：成功、WAPD_AnyError(1)：失敗
 */
int32_t WAPC_UserBoxController::setBoxTypeAndOwner( WAPS_SubController* params, APIC_Box* pc_Box, APIC_User* pc_User, APIC_Track* pc_Track, char* psc_BoxPassword )
{
	int32_t si32_Ret = WAPD_AnyError;
	int32_t si32_ReturnValue = APID_DocapiCommonErr;

	// リクエストボディよりボックス種別取得.
	std::string str_BoxType;
	si32_Ret = getBoxTypeFromBody( params, str_BoxType );
	if(WAPD_NoError != si32_Ret){
		WAPG_API_trace("%s(%d) %s : getBoxTypeFromBody failed \n", __FILE__, __LINE__, __FUNCTION__);
		return WAPD_AnyError;
	}

	// リクエストボディよりボックス所有者情報取得.
	std::string str_OwnerName;
	std::string str_ServerName;
	si32_Ret = getOwnerInformationFromBody( params, str_OwnerName, str_ServerName );
	if(WAPD_NoError != si32_Ret){
		WAPG_API_trace("%s(%d) %s : getOwnerInformationFromBody failed \n", __FILE__, __LINE__, __FUNCTION__);
		return WAPD_AnyError;
	}

	// ■ リクエストボディにボックス種別情報が設定されているか.
	if(0 < str_BoxType.length()){
		// パブリック以外のボックス種別に変更する場合は所有者名必須.
		if( ( str_BoxType != "PUBLIC" ) && ( 0 == str_OwnerName.length() ) ) {
			WAPG_API_trace("%s(%d) %s : owner_name empty.\n", __FILE__, __LINE__, __FUNCTION__);
			printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
			m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
			generateErrorResponseBody( WAPE_BoxError_InvalidBoxOwnerName );
			return WAPD_AnyError;
		}

		// ボックスオプション取得.
		// ボックス種別変更(changeBoxKind)のパラメータに設定するため.
		APIC_BoxOption c_BoxOption;
		si32_ReturnValue = pc_Box->getBoxOption(
			pc_User,					// ユーザー情報.
			psc_BoxPassword,			// ボックスパスワード.
			&c_BoxOption,				// ボックスオプション情報.
			pc_Track);					// 部門情報.

		switch(si32_ReturnValue) {
			case APID_DocapiMethodOk:
				break;

			case APID_DocapiOwnerAccessErr:		// オーナー名不正エラー.
				WAPG_API_trace("%s(%d) %s : getBoxOption APID_DocapiOwnerAccessErr \n", __FILE__, __LINE__, __FUNCTION__);
				printf("403:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //403->ErrorStatus[WAPD_STATUS_FORBIDDEN] in WAP_defs.h
				m_ResponseHeader->Status = WAPD_STATUS_FORBIDDEN;
				generateErrorResponseBody( WAPE_BoxError_OwnerAccessErr );
				return WAPD_AnyError;

			case APID_DocapiBoxidUnknownErr:	// 指定番号のBoxなしエラー.
				WAPG_API_trace("%s(%d) %s : getBoxOption APID_DocapiBoxidUnknownErr \n", __FILE__, __LINE__, __FUNCTION__);
				printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
				m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
				generateErrorResponseBody( WAPE_BoxError_BoxNotExist );
				return WAPD_AnyError;

			case APID_DocapiPasswordAccessErr:	// パスワード不正.
			case APID_DocapiCommonErr:			// その他の処理失敗エラー.
			default:
				WAPG_API_trace("%s(%d) %s : getBoxOption si32_ReturnValue=%d \n", __FILE__, __LINE__, __FUNCTION__, si32_ReturnValue);
				m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
				printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
				generateErrorResponseBody( WAPE_BoxError_None );
				return WAPD_AnyError;
		}

		// ボックス情報取得.
		// ボックス種別変更(changeBoxKind)のパラメータに設定するため.
		APIC_BoxInfo c_BoxInfo;       // 取得Info値.
		si32_ReturnValue = pc_Box->getBoxInfo(
			pc_User,
			&c_BoxInfo,
			(LIBE_CharSet)APID_DefaultCharSet,
			pc_Track);

		switch(si32_ReturnValue) {
			case APID_DocapiMethodOk:
				break;

			case APID_DocapiOwnerAccessErr:		// オーナー名不正エラー.
				WAPG_API_trace("%s(%d) %s : getBoxInfo APID_DocapiOwnerAccessErr \n", __FILE__, __LINE__, __FUNCTION__);
				printf("403:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //403->ErrorStatus[WAPD_STATUS_FORBIDDEN] in WAP_defs.h
				m_ResponseHeader->Status = WAPD_STATUS_FORBIDDEN;
				generateErrorResponseBody( WAPE_BoxError_OwnerAccessErr );
				return WAPD_AnyError;

			case APID_DocapiBoxidUnknownErr:	// 指定番号のBoxなしエラー.
				WAPG_API_trace("%s(%d) %s : getBoxInfo APID_DocapiBoxidUnknownErr \n", __FILE__, __LINE__, __FUNCTION__);
				printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
				m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
				generateErrorResponseBody( WAPE_BoxError_BoxNotExist );
				return WAPD_AnyError;

			case APID_DocapiPasswordAccessErr:	// パスワード不正.
			case APID_DocapiCommonErr:			// その他の処理失敗エラー.
			default:
				WAPG_API_trace("%s(%d) %s : getBoxInfo si32_ReturnValue=%d \n", __FILE__, __LINE__, __FUNCTION__, si32_ReturnValue);
				m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
				printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
				generateErrorResponseBody( WAPE_BoxError_None );
				return WAPD_AnyError;
		}

		char sc_BoxType[WAPD_MaxKeyLength] = {0};
		strncpy(sc_BoxType, str_BoxType.c_str(), sizeof(sc_BoxType) - 1 );
		APIE_BoxOptionCreateInfo e_NewBoxType = APIE_BoxmakerAuto;
		if(False == WAPS_CommonTableString<WAPE_BoxType, APIE_BoxOptionCreateInfo>::WapStringToCommon( sc_BoxType, e_NewBoxType )){
			WAPG_API_trace("%s(%d) %s : Illegal BoxType value \n", __FILE__, __LINE__, __FUNCTION__);
			printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
			m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
			generateErrorResponseBody( WAPE_BoxError_InvalidBoxKind );
			return WAPD_AnyError;
		}

		// 所有者番号設定.
		int32_t si32_NewOwnerID = 0;
		si32_Ret = getNewOwnerNumber(pc_User, pc_Track, e_NewBoxType, str_OwnerName, str_ServerName, si32_NewOwnerID);
		if(WAPD_NoError != si32_Ret){
			WAPG_API_trace("%s(%d) %s : getNewOwnerNumber failed \n", __FILE__, __LINE__, __FUNCTION__);
			return WAPD_AnyError;
		}

		// ボックス種別/所有者変更権限チェック.
		if( False == validateChangingBoxTypeAndOwner( pc_User, pc_Track, c_BoxOption.sl_BoxCreateInfo, c_BoxInfo.ul_OwnUserID, e_NewBoxType, si32_NewOwnerID, psc_BoxPassword ) ) {
			WAPG_API_trace("%s(%d) %s : validateChangingBoxTypeAndOwner invalid \n", __FILE__, __LINE__, __FUNCTION__);
			printf("403:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //403->ErrorStatus[WAPD_STATUS_FORBIDDEN] in WAP_defs.h
			m_ResponseHeader->Status = WAPD_STATUS_FORBIDDEN;
			generateErrorResponseBody( WAPE_BoxError_OwnerAccessErr );
			return WAPD_AnyError;
		}

		c_BoxOption.sl_BoxCreateInfo = e_NewBoxType;
		c_BoxInfo.ul_OwnUserID = static_cast<uint32_t>(si32_NewOwnerID);

		// ボックス種別変更.
		si32_ReturnValue = pc_Box->changeBoxKind(
			pc_User,				// ユーザー情報.
			psc_BoxPassword,		// ボックスパスワード.
			&c_BoxInfo,				// 設定インフォメーション値.
			&c_BoxOption, 			// 設定オプション指定値.
			(LIBE_CharSet)APID_DefaultCharSet,	// ボックス名文字コード変換識別.
			pc_Track,				// 部門管理情報.
			TYPE_RegistrationTypeOnlyOne,		// 監査ログタイプ.
			pc_User,				// ログインユーザー.
			pc_Track,				// ログイン部門.
			False,					//  管理者操作時のパスワード規約可否フラグ.
			static_cast<short> ( params->AuthorityID ),	// 	認証ID.
			params->CostCenterID);	//CostCenterID.

		switch(si32_ReturnValue) {
			case APID_DocapiMethodOk:
				break;

			case APID_DocapiOwnerAccessErr:		// オーナー名不正エラー.
				WAPG_API_trace("%s(%d) %s : changeBoxKind APID_DocapiOwnerAccessErr\n", __FILE__, __LINE__, __FUNCTION__);
				printf("403:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //403->ErrorStatus[WAPD_STATUS_FORBIDDEN] in WAP_defs.h
				m_ResponseHeader->Status = WAPD_STATUS_FORBIDDEN;
				generateErrorResponseBody( WAPE_BoxError_OwnerAccessErr );
				return WAPD_AnyError;

			case APID_DocapiBoxidUnknownErr:	// 指定番号のBoxなしエラー.
				WAPG_API_trace("%s(%d) %s : changeBoxKind APID_DocapiOwnerAccessErr\n", __FILE__, __LINE__, __FUNCTION__);
				printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
				m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
				generateErrorResponseBody( WAPE_BoxError_BoxNotExist );
				return WAPD_AnyError;

			case APID_DocapiUserBoxOverErr:		// ユーザボックス数上限オーバー.
			case APID_DocapiGroupBoxOverErr:	// グループボックス数上限オーバー.
			case APID_DocapiAllBoxOverErr:		// 全ボックス数上限オーバー.
				WAPG_API_trace("%s(%d) %s : changeBoxKind si32_ReturnValue=%d\n", __FILE__, __LINE__, __FUNCTION__, si32_ReturnValue);
				printf("503:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //503->ErrorStatus[WAPD_STATUS_SERVICE_UNAVAILABLE] in WAP_defs.h
				m_ResponseHeader->Status = WAPD_STATUS_SERVICE_UNAVAILABLE;
				generateErrorResponseBody( WAPE_BoxError_None );
				return WAPD_AnyError;

			case APID_DocapiPasswordAccessErr:	// パスワード不正.
			case APID_DocapiCommonErr:			// その他の処理失敗エラー.
			default:
				WAPG_API_trace("%s(%d) %s : changeBoxKind si32_ReturnValue=%d\n", __FILE__, __LINE__, __FUNCTION__, si32_ReturnValue);
				m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
				printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
				generateErrorResponseBody( WAPE_BoxError_None );
				return WAPD_AnyError;
		}
	}
	// ■ リクエストボディにボックス所有者情報が設定されているか.
	else if((0 < str_OwnerName.length())
			|| (0 < str_ServerName.length())) {
		// ボックス種別は指定されていないので、所有者のみ変更する.

		// ボックス情報取得.
		APIC_BoxInfo c_BoxInfo;       // 取得Info値.
		si32_ReturnValue = pc_Box->getBoxInfo(
			pc_User,
			&c_BoxInfo,
			(LIBE_CharSet)APID_DefaultCharSet,
			pc_Track);

		switch(si32_ReturnValue) {
			case APID_DocapiMethodOk:
				break;

			case APID_DocapiOwnerAccessErr:		// オーナー名不正エラー.
				WAPG_API_trace("%s(%d) %s : getBoxInfo APID_DocapiOwnerAccessErr \n", __FILE__, __LINE__, __FUNCTION__);
				printf("403:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //403->ErrorStatus[WAPD_STATUS_FORBIDDEN] in WAP_defs.h
				m_ResponseHeader->Status = WAPD_STATUS_FORBIDDEN;
				generateErrorResponseBody( WAPE_BoxError_OwnerAccessErr );
				return WAPD_AnyError;

			case APID_DocapiBoxidUnknownErr:	// 指定番号のBoxなしエラー.
				WAPG_API_trace("%s(%d) %s : getBoxInfo APID_DocapiBoxidUnknownErr \n", __FILE__, __LINE__, __FUNCTION__);
				printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
				m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
				generateErrorResponseBody( WAPE_BoxError_BoxNotExist );
				return WAPD_AnyError;

			case APID_DocapiPasswordAccessErr:	// パスワード不正.
			case APID_DocapiCommonErr:			// その他の処理失敗エラー.
			default:
				WAPG_API_trace("%s(%d) %s : getBoxInfo si32_ReturnValue=%d \n", __FILE__, __LINE__, __FUNCTION__, si32_ReturnValue);
				m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
				printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
				generateErrorResponseBody( WAPE_BoxError_None );
				return WAPD_AnyError;
		}

		APIC_BoxOption c_BoxOption;
		si32_ReturnValue = pc_Box->getBoxOption(
			pc_User,					// ユーザー情報.
			psc_BoxPassword,			// ボックスパスワード.
			&c_BoxOption,				// ボックスオプション情報.
			pc_Track);					// 部門情報.

		switch(si32_ReturnValue) {
			case APID_DocapiMethodOk:
				break;

			case APID_DocapiOwnerAccessErr:		// オーナー名不正エラー.
				WAPG_API_trace("%s(%d) %s : getBoxOption APID_DocapiOwnerAccessErr \n", __FILE__, __LINE__, __FUNCTION__);
				printf("403:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //403->ErrorStatus[WAPD_STATUS_FORBIDDEN] in WAP_defs.h
				m_ResponseHeader->Status = WAPD_STATUS_FORBIDDEN;
				generateErrorResponseBody( WAPE_BoxError_OwnerAccessErr );
				return WAPD_AnyError;

			case APID_DocapiBoxidUnknownErr:	// 指定番号のBoxなしエラー.
				WAPG_API_trace("%s(%d) %s : getBoxOption APID_DocapiBoxidUnknownErr \n", __FILE__, __LINE__, __FUNCTION__);
				printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
				m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
				generateErrorResponseBody( WAPE_BoxError_BoxNotExist );
				return WAPD_AnyError;

			case APID_DocapiPasswordAccessErr:	// パスワード不正.
			case APID_DocapiCommonErr:			// その他の処理失敗エラー.
			default:
				WAPG_API_trace("%s(%d) %s : getBoxOption si32_ReturnValue=%d \n", __FILE__, __LINE__, __FUNCTION__, si32_ReturnValue);
				m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
				printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
				generateErrorResponseBody( WAPE_BoxError_None );
				return WAPD_AnyError;
		}

		// 所有者番号設定.
		int32_t si32_NewOwnerID = 0;
		si32_Ret = getNewOwnerNumber(pc_User, pc_Track, c_BoxOption.sl_BoxCreateInfo, str_OwnerName, str_ServerName, si32_NewOwnerID);
		if(WAPD_NoError != si32_Ret){
			WAPG_API_trace("%s(%d) %s : getNewOwnerNumber si32_ReturnValue=%d \n", __FILE__, __LINE__, __FUNCTION__, si32_ReturnValue);
			return WAPD_AnyError;
		}

		// ボックス種別/所有者変更権限チェック.
		if( False == validateChangingBoxTypeAndOwner( pc_User, pc_Track, c_BoxOption.sl_BoxCreateInfo, c_BoxInfo.ul_OwnUserID, c_BoxOption.sl_BoxCreateInfo, si32_NewOwnerID, psc_BoxPassword ) ) {
			WAPG_API_trace("%s(%d) %s : validateChangingBoxTypeAndOwner invalid \n", __FILE__, __LINE__, __FUNCTION__);
			printf("403:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //403->ErrorStatus[WAPD_STATUS_FORBIDDEN] in WAP_defs.h
			m_ResponseHeader->Status = WAPD_STATUS_FORBIDDEN;
			generateErrorResponseBody( WAPE_BoxError_OwnerAccessErr );
			return WAPD_AnyError;
		}

		// Box所有者名変更.
		si32_ReturnValue = pc_Box->changeOwner(
			pc_User,				// ユーザー情報.
			psc_BoxPassword,		// ボックスパスワード.
			si32_NewOwnerID,		// 設定したい所有者情報.
			pc_Track,
			TYPE_RegistrationTypeOnlyOne,
			params->CostCenterID);

		switch(si32_ReturnValue) {
			case APID_DocapiMethodOk:
				break;

			case APID_DocapiOwnerAccessErr:		// オーナー名不正エラー.
				WAPG_API_trace("%s(%d) %s : changeOwner APID_DocapiOwnerAccessErr \n", __FILE__, __LINE__, __FUNCTION__);
				printf("403:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //403->ErrorStatus[WAPD_STATUS_FORBIDDEN] in WAP_defs.h
				m_ResponseHeader->Status = WAPD_STATUS_FORBIDDEN;
				generateErrorResponseBody( WAPE_BoxError_OwnerAccessErr );
				return WAPD_AnyError;

			case APID_DocapiBoxidUnknownErr:	// 指定番号のBoxなしエラー.
				WAPG_API_trace("%s(%d) %s : changeOwner APID_DocapiBoxidUnknownErr \n", __FILE__, __LINE__, __FUNCTION__);
				printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
				m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
				generateErrorResponseBody( WAPE_BoxError_BoxNotExist );
				return WAPD_AnyError;

			case APID_DocapiUserBoxOverErr:		// ユーザボックス数上限オーバー.
			case APID_DocapiGroupBoxOverErr:	// グループボックス数上限オーバー.
			case APID_DocapiAllBoxOverErr:		// 全ボックス数上限オーバー.
				WAPG_API_trace("%s(%d) %s : changeOwner si32_ReturnValue=%d\n", __FILE__, __LINE__, __FUNCTION__, si32_ReturnValue);
				printf("503:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //503->ErrorStatus[WAPD_STATUS_SERVICE_UNAVAILABLE] in WAP_defs.h
				m_ResponseHeader->Status = WAPD_STATUS_SERVICE_UNAVAILABLE;
				generateErrorResponseBody( WAPE_BoxError_None );
				return WAPD_AnyError;

			case APID_DocapiPasswordAccessErr:	// パスワード不正.
			case APID_DocapiCommonErr:			// その他の処理失敗エラー.
			default:
				WAPG_API_trace("%s(%d) %s : changeOwner si32_ReturnValue=%d\n", __FILE__, __LINE__, __FUNCTION__, si32_ReturnValue);
				m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
				printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
				generateErrorResponseBody( WAPE_BoxError_None );
				return WAPD_AnyError;
		}
	}
	else{
		// 何もしない.
	}

	return WAPD_NoError;
}


/**
 * @brief ボックス検索キー種別を設定する
 * @param[in]	params		サブコントローラー
 * @param[in]	pc_Box		ボックスインスタンス
 * @param[in]	pc_User		ユーザーインスタンス
 * @param[in]	pc_Track	部門インスタンス
 * @param[in]	psc_BoxPassword	ボックスパスワード
 * @return 処理の成否, WAPD_NoError(0)：成功、WAPD_AnyError(1)：失敗
 */
int32_t WAPC_UserBoxController::setSearchKey( WAPS_SubController* params, APIC_Box* pc_Box, APIC_User* pc_User, APIC_Track* pc_Track, char* psc_BoxPassword )
{
	int32_t si32_Ret = WAPD_AnyError;
	int32_t si32_ReturnValue = APID_DocapiCommonErr;

	// ■ リクエストボディにボックス検索キー種別が設定されている.
	string str_SearchKey;
	si32_Ret = getSearchKeyFromBody( params, str_SearchKey );
	if(WAPD_NoError != si32_Ret){
		return WAPD_AnyError;
	}
	if(0 < str_SearchKey.length()){
		char sc_SearchKey[WAPD_MaxKeyLength] = {0};
		TYPE_SearchKey e_SearchKey = TYPE_SearchKeyAA;
		strncpy(sc_SearchKey, str_SearchKey.c_str(), sizeof(sc_SearchKey) - 1 );
		if(False == WAPS_CommonTableString<WAPE_SearchKey, TYPE_SearchKey>::WapStringToCommon(
			sc_SearchKey, e_SearchKey )){
			WAPG_API_trace("%s(%d) %s : Illegal SearchKey value \n", __FILE__, __LINE__, __FUNCTION__);
			printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
			m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
			generateErrorResponseBody( WAPE_BoxError_InvalidSearchKey );
			return WAPD_AnyError;
		}

		// ボックス検索キー種別設定.
		si32_ReturnValue = pc_Box->setBoxSearchKey(
			pc_User,				// ユーザー情報.
			psc_BoxPassword,		// ボックスパスワード.
			e_SearchKey,			// ボックス検索キー種別.
			pc_Track);				// 部門管理情報.

		switch(si32_ReturnValue) {
			case APID_DocapiMethodOk:
				break;

			case APID_DocapiOwnerAccessErr:		// オーナー名不正エラー.
				printf("403:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //403->ErrorStatus[WAPD_STATUS_FORBIDDEN] in WAP_defs.h
				m_ResponseHeader->Status = WAPD_STATUS_FORBIDDEN;
				generateErrorResponseBody( WAPE_BoxError_OwnerAccessErr );
				return WAPD_AnyError;

			case APID_DocapiBoxidUnknownErr:	// 指定番号のBoxなしエラー.
				printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
				m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
				generateErrorResponseBody( WAPE_BoxError_BoxNotExist );
				return WAPD_AnyError;

			case APID_DocapiPasswordAccessErr:	// パスワード不正.
			case APID_DocapiCommonErr:			// その他の処理失敗エラー.
			default:
				m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
				printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
				generateErrorResponseBody( WAPE_BoxError_None );
				return WAPD_AnyError;
		}
	}

	return WAPD_NoError;
}

/**
 * @brief 親展受信機能情報を設定する
 * @param[in]	params		サブコントローラー
 * @param[in]	pc_Box		ボックスインスタンス
 * @param[in]	pc_User		ユーザーインスタンス
 * @param[in]	pc_Track	部門インスタンス
 * @param[in]	psc_BoxPassword	ボックスパスワード
 * @return 処理の成否, WAPD_NoError(0)：成功、WAPD_AnyError(1)：失敗
 */
int32_t WAPC_UserBoxController::setConfidentialRx( WAPS_SubController* params, APIC_Box* pc_Box, APIC_User* pc_User, APIC_Track* pc_Track, char* psc_BoxPassword )
{
	int32_t si32_Ret = WAPD_AnyError;
	int32_t si32_ReturnValue = APID_DocapiCommonErr;

	// ■ リクエストボディに親展受信機能情報が設定されている.
	WAPS_ConfidentialRx s_ConfidentialRx = {};
	si32_Ret = getConfidentialRxFromBody( params, s_ConfidentialRx );
	if(WAPD_NoError != si32_Ret){
		WAPG_API_trace("%s(%d) %s : getConfidentialRxFromBody failed\n", __FILE__, __LINE__, __FUNCTION__);
		return WAPD_AnyError;
	}
	if( WAPD_SelectNoSetting != s_ConfidentialRx.sc_Select ) {

		Bool b_IsConfidentialReception = True;  //親展受信設定をONにする

		// APIC_BoxRegistryインスタンス取得.
		APIC_BoxRegistry* pc_BoxRegistry = APIC_BoxRegistry::newInstance();
		if(NULL == pc_BoxRegistry){
			WAPG_API_trace("%s(%d) %s : APIC_BoxRegistry instance NULL \n", __FILE__, __LINE__, __FUNCTION__);
			m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
			printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
			generateErrorResponseBody( WAPE_BoxError_None );
			return WAPD_AnyError;
		}
		// アクセス権情報設定.
		APIC_AccessTokenManager* pc_AccessTokenManager = APIC_AccessTokenManager::newInstance();
		JLGE_InspectLogIF e_InspectLogIF = JLGE_InspectLogIfOperationPanel;
		if( NULL != pc_AccessTokenManager){
			pc_AccessTokenManager->getAppType(m_AccessToken, &e_InspectLogIF);
		}
		pc_BoxRegistry->setLogIFNo( e_InspectLogIF );

		// 親展受信情報取得.
		// 親展受信情報設定/削除のパラメータに設定するため.
		APIS_ConfidentialBoxStruct s_Confidential = {};
		si32_ReturnValue = pc_BoxRegistry->ConfidentialInfoAction(
			m_si32_BoxNo, 				// 指定ボックス番号.
			WAPD_BOX_CONFBOX_OPE_GET, 	// アクセス種別(取得).
			&s_Confidential, 			// 親展受信情報.
			pc_User, 					// ユーザー情報.
			pc_Track, 					// 部門情報.
			TYPE_RegistrationTypeOnlyOne,	// 監査ログタイプ.
			static_cast<short> ( params->AuthorityID ) );		// 認証ID.

		switch(si32_ReturnValue) {
			case APID_DocapiMethodOk:
				break;

			case APID_DocapiCommonErr:			// その他の処理失敗エラー.
			default:
				//「WAPD_BOX_CONFBOX_OPE_GETでエラー＝親展受信機能が設定されていない」.
				// というI/F仕様であるため、ここでは親展受信機能無効と判断し、WebAPI実行エラーとはならない.
				b_IsConfidentialReception = False;  	// 親展受信設定がOFFである.
				// 新規設定となるため、URIから指定されたBoxNoを構造体のBoxNoに代入.
				s_Confidential.sl_BoxNo = m_si32_BoxNo;
				break;
		}

		// 変更前に親展受信機能が有効であった場合、旧パスワードの一致チェック.
		if( True == b_IsConfidentialReception ) {
			// 旧パスワードと一致しない場合.
			if( 0 != strncmp(s_Confidential.sc_CommPwd, s_ConfidentialRx.str_OldPassword.c_str(), sizeof(s_Confidential.sc_CommPwd)) ){
				WAPG_API_trace("%s(%d) %s : Illegal old Password\n", __FILE__, __LINE__, __FUNCTION__);
				pc_BoxRegistry->deleteInstance();
				printf("403:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //403->ErrorStatus[WAPD_STATUS_FORBIDDEN] in WAP_defs.h
				m_ResponseHeader->Status = WAPD_STATUS_FORBIDDEN;
				generateErrorResponseBody( WAPE_BoxError_ConfidentialRxPasswordAccessErr );
				return WAPD_AnyError;
			}
		}

		int32_t si32_Operation = WAPD_BOX_CONFBOX_OPE_SET;
		if( True == s_ConfidentialRx.sc_Select ) {
			// 親展受信機能有効化（OFF→ON）または親展パスワード変更（ON→ON）.
			si32_Operation = WAPD_BOX_CONFBOX_OPE_SET;
			// 新パスワードを設定構造体にコピー.
			strncpy(s_Confidential.sc_CommPwd, s_ConfidentialRx.str_NewPassword.c_str(), sizeof(s_Confidential.sc_CommPwd) - 1 );
		}
		else {
			// 親展受信機能解除（ON→OFF）.
			// この場合、設定構造体にパスワードは特に必要ない.
			si32_Operation = WAPD_BOX_CONFBOX_OPE_DEL;
		}

		// 親展受信設定の無効（OFF）から無効（OFF）への設定はエラーなので、それ以外の場合のみ、親展受信情報を設定する.
		if ((True == b_IsConfidentialReception) || (True == s_ConfidentialRx.sc_Select)) {
			// 親展受信情報設定/削除.
			si32_ReturnValue = pc_BoxRegistry->ConfidentialInfoAction(
				m_si32_BoxNo, 				// 指定ボックス番号.
				si32_Operation, 			// アクセス種別(設定 or 削除).
				&s_Confidential, 			// 親展受信情報.
				pc_User, 					// ユーザー情報.
				pc_Track, 					// 部門情報.
				TYPE_RegistrationTypeOnlyOne,	// 監査ログタイプ.
				static_cast<short> ( params->AuthorityID ) );		// 認証ID.
			pc_BoxRegistry->deleteInstance();

			switch(si32_ReturnValue) {
				case APID_DocapiMethodOk:
					break;

				case APID_DocapiCommonErr:			// その他の処理失敗エラー.
				default:
					WAPG_API_trace("%s(%d) %s : APIC_BoxRegistry::ConfidentialInfoAction failed\n", __FILE__, __LINE__, __FUNCTION__);
					m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
					printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
					generateErrorResponseBody( WAPE_BoxError_None );
					return WAPD_AnyError;
				}
			}
	}

	return WAPD_NoError;
}

/**
 * @brief ボックスパスワードを設定する
 * @param[in]	params		サブコントローラー
 * @param[in]	pc_Box		ボックスインスタンス
 * @param[in]	pc_User		ユーザーインスタンス
 * @param[in]	pc_Track	部門インスタンス
 * @param[in]	psc_BoxPassword	ボックスパスワード
 * @return 処理の成否, WAPD_NoError(0)：成功、WAPD_AnyError(1)：失敗
 */
int32_t WAPC_UserBoxController::setBoxPassword( WAPS_SubController* params, APIC_Box* pc_Box, APIC_User* pc_User, APIC_Track* pc_Track, char* psc_BoxPassword )
{
	int32_t si32_Ret = WAPD_AnyError;
	int32_t si32_ReturnValue = APID_DocapiCommonErr;

	// ■ リクエストボディにボックスパスワード情報が設定されている.
	WAPS_BoxPasswordInformation s_BoxPassword = {};
	si32_Ret = getBoxPasswordInformationFromBody( params, pc_Box, pc_User, pc_Track, psc_BoxPassword, s_BoxPassword );
	if(WAPD_NoError != si32_Ret){
		WAPG_API_trace("%s(%d) %s : getBoxPasswordInformationFromBody failed\n", __FILE__, __LINE__, __FUNCTION__);
		return WAPD_AnyError;
	}
	if( ( 0 < s_BoxPassword.str_OldPassword.length() ) ||
	    ( 0 < s_BoxPassword.str_NewPassword.length() ) ){

		si32_ReturnValue = pc_Box->changePassword(
			 pc_User,
			 s_BoxPassword.str_OldPassword.c_str(),
			 s_BoxPassword.str_NewPassword.c_str(),
			 pc_Track,
			 TYPE_RegistrationTypeOnlyOne,
			 static_cast<short> ( params->AuthorityID ) );

		switch(si32_ReturnValue){
			case APID_DocapiMethodOk:
				break;

			case APID_DocapiOwnerAccessErr:		// オーナー名不正エラー.
				WAPG_API_trace("%s(%d) %s : APIC_Box::changePassword APID_DocapiOwnerAccessErr\n", __FILE__, __LINE__, __FUNCTION__);
				printf("403:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //403->ErrorStatus[WAPD_STATUS_FORBIDDEN] in WAP_defs.h
				m_ResponseHeader->Status = WAPD_STATUS_FORBIDDEN;
				generateErrorResponseBody( WAPE_BoxError_OwnerAccessErr );
				return WAPD_AnyError;

			case APID_DocapiBoxidUnknownErr:	// 指定番号のBoxなしエラー.
				WAPG_API_trace("%s(%d) %s : APIC_Box::changePassword APID_DocapiBoxidUnknownErr\n", __FILE__, __LINE__, __FUNCTION__);
				printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
				m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
				generateErrorResponseBody( WAPE_BoxError_BoxNotExist );
				return WAPD_AnyError;

			case APID_DocapiPasswordAccessErr:	// パスワード不正.
				WAPG_API_trace("%s(%d) %s : APIC_Box::changePassword APID_DocapiPasswordAccessErr\n", __FILE__, __LINE__, __FUNCTION__);
				printf("403:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //403->ErrorStatus[WAPD_STATUS_FORBIDDEN] in WAP_defs.h
				m_ResponseHeader->Status = WAPD_STATUS_FORBIDDEN;
				generateErrorResponseBody( WAPE_BoxError_PasswordAccessErr );
				return WAPD_AnyError;

			case APID_DocapiCommonErr:			// その他の処理失敗エラー.
			default:
				WAPG_API_trace("%s(%d) %s : APIC_Box::changePassword failed\n", __FILE__, __LINE__, __FUNCTION__);
				printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
				m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
				generateErrorResponseBody( WAPE_BoxError_None );
				return WAPD_AnyError;
		}
	}

	return WAPD_NoError;
}
/**
 * @brief コールバック関数ポインタ取得.
 * @retval NULL以外 コールバック関数ポインタ.
 * @retval NULL 空き関数なし.
 */
FP_ANS_CHANGE_BOX_STATUS WAPC_UserBoxController::getBoxDeleteCb()
{
	FP_ANS_CHANGE_BOX_STATUS fp_Cb = NULL;

	for( uint32_t ui32_loop = 0; ui32_loop < (sizeof(g_s_CbTable) / sizeof(g_s_CbTable[0])); ui32_loop++ ) {
		if( False == g_s_CbTable[ui32_loop].b_InUse ) {
			fp_Cb = g_s_CbTable[ui32_loop].fp_Cb;
			g_s_CbTable[ui32_loop].b_InUse = True;
			break;
		}
	}
	return fp_Cb;
}

/**
 * @brief コールバック関数ポインタ解放.
 * @return なし.
 */
void WAPC_UserBoxController::releaseBoxDeleteCb( FP_ANS_CHANGE_BOX_STATUS fp_Cb )
{
	for( uint32_t ui32_loop = 0; ui32_loop < (sizeof(g_s_CbTable) / sizeof(g_s_CbTable[0])); ui32_loop++ ) {
		if( fp_Cb == g_s_CbTable[ui32_loop].fp_Cb ) {
			g_s_CbTable[ui32_loop].b_InUse = False;
			break;
		}
	}
}

/**
 * @brief ボックス削除実行結果通知.
 * @param[in] si32_BoxId 削除されたボックス番号
 * @param[in] si32_Result 処理結果
 * @param[in] ui64_ReqId コールバック関数ID
 * @return なし.
 */
void WAPC_UserBoxController::BoxDeleteCompleted( int32_t si32_BoxId, int32_t si32_Result, uint64_t ui64_ReqId )
{
	void* vp_Handle = NULL;
	int32_t si32_cbStatus = WAPD_AnyError;

	// 非同期結果格納クラス取り出し.
	WAPC_DeleteBoxResult* pc_AsyncResult = NULL;
	pc_AsyncResult = dynamic_cast<WAPC_DeleteBoxResult*>(WAPC_AsyncResponseContainer::newInstance()->getById( ui64_ReqId ));
	if( NULL != pc_AsyncResult ) {
		pc_AsyncResult->setDeleteBoxId( si32_BoxId );
		pc_AsyncResult->setDeleteBoxResult( si32_Result );
		vp_Handle = pc_AsyncResult->getHandle();
		si32_cbStatus = WAPD_NoError;
	}

	// 非同期レスポンス用タスクにスイッチ.
	WAPC_AsyncResponder::newInstance()->postMessage( vp_Handle, si32_cbStatus );
}

/**
 * @brief ボックス削除 非同期レスポンス生成.
 * @param[in] ps_Params HTTPリクエストヘッダ、ボディ
 * @retval WAPD_NoError(0) 成功.
 * @retval WAPD_AnyError(1) 失敗.
 */
int32_t WAPC_UserBoxController::generateBoxDeleteResponse( WAPS_SubController *ps_Params )
{
	// 非同期結果クラス取り出し.
	WAPC_DeleteBoxResult* pc_AsyncResult = NULL;
	pc_AsyncResult = dynamic_cast<WAPC_DeleteBoxResult*>(WAPC_AsyncResponseContainer::newInstance()->getByHandle( m_pv_Handle ));
	if( NULL == pc_AsyncResult ) {
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		generateErrorResponseBody( WAPE_BoxError_None );
		return WAPD_AnyError;
	}

	// 結果取得.
	int32_t si32_BoxDeleteResult = APID_DocapiMethodOk;
	si32_BoxDeleteResult = pc_AsyncResult->getDeleteBoxResult();
	int32_t si32_DeleteBoxId = pc_AsyncResult->getDeleteBoxId();

	// 非同期結果クラス・関数ポインタ片付け.
	releaseBoxDeleteCb( reinterpret_cast<FP_ANS_CHANGE_BOX_STATUS>(pc_AsyncResult->getReqId()) );
	WAPC_AsyncResponseContainer::newInstance()->erase( pc_AsyncResult->getReqId() );
	delete pc_AsyncResult;

	// 結果チェック.
	switch( si32_BoxDeleteResult ) {
		case APID_DocapiMethodOk:
			m_Response = new Json::Value;
			if( NULL == m_Response ) {
				m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
				printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
				generateErrorResponseBody( WAPE_BoxError_None );
				return WAPD_AnyError;
			}
			(*m_Response)["delete_box_no"] = si32_DeleteBoxId;
			m_ResponseHeader->Status = WAPD_STATUS_OK;
			break;

		default:
			m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
			printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
			generateErrorResponseBody( WAPE_BoxError_None );
			return WAPD_AnyError;
	}
	return WAPD_NoError;
}

/**
 * @brief ボックス種別/所有者変更権限チェック.
 * @param[in] pc_User 実行ユーザーインスタンス. 部門ログインなどユーザーなしの場合はNULL.
 * @param[in] pc_Track 実行部門インスタンス. 所属していない場合はNULL.
 * @param[in] e_BoxType 現ボックス種別.
 * @param[in] ui32_OwnUserID 現所有者ID.
 * @param[in] e_NewBoxType 新ボックス種別.
 * @param[in] ui32_NewOwnUserID 新所有者ID.
 * @param[in] psc_Password ボックスパスワード. NULLは不可.
 * @return 判定結果.
 */
Bool WAPC_UserBoxController::validateChangingBoxTypeAndOwner(
	APIC_User* pc_User,
	APIC_Track* pc_Track,
	APIE_BoxOptionCreateInfo e_BoxType,
	uint32_t ui32_OwnUserID,
	APIE_BoxOptionCreateInfo e_NewBoxType,
	uint32_t ui32_NewOwnUserID,
	char* psc_BoxPassword
	)
{
	// 実行ユーザー種別.
	WAPE_BoxUserType e_OperatorType = WAPE_BoxUserType_None;
	e_OperatorType = getBoxUserType( pc_User, pc_Track );
	if( WAPE_BoxUserType_None == e_OperatorType ) {
		return False;
	}
	// ボックス所有者種別.
	WAPE_BoxUserType e_SrcBoxOwnerType = WAPE_BoxUserType_None;
	e_SrcBoxOwnerType = getBoxOwnerType( e_BoxType, ui32_OwnUserID );
	if( WAPE_BoxUserType_None == e_SrcBoxOwnerType ) {
		return False;
	}
	// ボックス新所有者種別.
	WAPE_BoxUserType e_DstBoxOwnerType = WAPE_BoxUserType_None;
	e_DstBoxOwnerType = getBoxOwnerType( e_NewBoxType, ui32_NewOwnUserID );
	if( WAPE_BoxUserType_None == e_DstBoxOwnerType ) {
		return False;
	}
	// パスワード規約状態.
	WAPE_PasswordRules e_PassWordRules = WAPE_PasswordRules_None;
	e_PassWordRules = checkPasswordRules( psc_BoxPassword );
	// 認証方式.
	WAPE_UserAuthentication e_UserAuth = WAPE_UserAuthentication_None;
	e_UserAuth = static_cast<WAPE_UserAuthentication>(APIC_UserAuthentication::newInstance()->getUserAuthentication());

	// 組禁テーブルぐるぐる.
	for( uint32_t si32_Index = 0; si32_Index < ( sizeof(g_s_TypeAndOwnerValidate) / sizeof(g_s_TypeAndOwnerValidate[0]) ); si32_Index++ ) {
		// Noneまたは一致するなら次項目のチェックへ、一致しなければcontinue.
		// 実行ユーザー種別.
		if( ( WAPE_BoxUserType_None != g_s_TypeAndOwnerValidate[si32_Index].e_OperatorType ) &&
			( e_OperatorType != g_s_TypeAndOwnerValidate[si32_Index].e_OperatorType ) ) {
				continue;
		}
		// ボックス所有者種別.
		if( ( WAPE_BoxUserType_None != g_s_TypeAndOwnerValidate[si32_Index].e_SrcBoxOwnerType ) &&
			( e_SrcBoxOwnerType != g_s_TypeAndOwnerValidate[si32_Index].e_SrcBoxOwnerType ) ) {
				continue;
		}
		// ボックス新所有者種別.
		if( ( WAPE_BoxUserType_None != g_s_TypeAndOwnerValidate[si32_Index].e_DstBoxOwnerType ) &&
			( e_DstBoxOwnerType != g_s_TypeAndOwnerValidate[si32_Index].e_DstBoxOwnerType ) ) {
				continue;
		}
		// パスワード規約状態.
		if( ( WAPE_PasswordRules_None != g_s_TypeAndOwnerValidate[si32_Index].e_PassWordRules ) &&
			( e_PassWordRules != g_s_TypeAndOwnerValidate[si32_Index].e_PassWordRules ) ) {
				continue;
		}
		// 認証方式.
		if( ( WAPE_UserAuthentication_None != g_s_TypeAndOwnerValidate[si32_Index].e_UserAuth ) &&
			( e_UserAuth != g_s_TypeAndOwnerValidate[si32_Index].e_UserAuth ) ) {
				continue;
		}
		// 全て一致したため組禁に抵触している.
		return False;
	}

	// 組禁には抵触しなかった.
	return True;
}
