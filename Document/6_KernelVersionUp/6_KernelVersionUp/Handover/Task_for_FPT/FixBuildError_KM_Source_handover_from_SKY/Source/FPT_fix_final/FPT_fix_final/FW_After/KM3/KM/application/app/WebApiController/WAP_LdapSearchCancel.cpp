// -----------------------------------------------------------------------------.
//
// @file		WAP_LdapSearchCancel.cpp.
// @brief		LDAP宛先検索キャンセル 個別コントローラクラス.
// @author		日立 井原.
// @since		2017/8/8.
// @date		2018/3/12 日立田中 LDAP宛先検索キャンセル結果取得の仕様削除により不要処理削除.
//
// -----------------------------------------------------------------------------.
#include "WAP_LdapSearchCancel.h"
#include "WAP_RestApi.h"
#include "WAP_LdapSearchStatusMgr.h"
#include "WAP_Power.h"

/**
 * @brief		コンストラクタ.
 * @return		なし.
 * @author		日立 井原.
 * @since		2017/8/8.
 */
WAPC_LdapSearchCancelController::WAPC_LdapSearchCancelController()
{
}

/**
 * @brief		デストラクタ.
 * @return		なし.
 * @author		日立 井原.
 * @since		2017/8/8.
 */
WAPC_LdapSearchCancelController::~WAPC_LdapSearchCancelController()
{
}

/**
 * @brief		HTTPレスポンスの生成([H.LDAP宛先検索(単検索/複合検索)キャンセルAPI]).
 * @param[in]	params HTTPリクエストヘッダ、ボディ.
 * @return		LDAP宛先検索キャンセルの成否, WAPD_NoError(0)：成功、WAPD_AnyError(1)：失敗.
 * @author		日立 井原.
 * @since		2017/8/8.
 * @date		2018/3/12 日立田中 LDAP宛先検索キャンセル結果取得の仕様削除により不要処理削除.
 */
int32_t WAPC_LdapSearchCancelController::ResponseGenerate( WAPS_SubController* ps_Param )
{
	m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
	int32_t si32_ret = WAPD_AnyError;

	if ( NULL == ps_Param ) {
		WAPG_API_trace("[ERR] WAPC_LdapSearchCancelController::ResponseGenerate() NULL Parameter!.\n");
		return si32_ret;
	}
	
	// Content-Typeチェック.
	if ( WAPD_CONTENT_TYPE_JSON != ps_Param->ContentType ) {
		WAPG_API_trace("[ERR] WAPC_LdapSearchCancelController::ResponseGenerate() ContentType is not Json.\n");
		return si32_ret;
	}

	// コマンドチェック.
	switch( ps_Param->Method ) {
	case WAPD_HTTP_POST:	// POST.
		{
		WAPG_API_trace("WAPC_LdapSearchCancelController::ResponseGenerate() POST.\n");
		// WebAPI電源管理.
		WAPC_Power* pc_Power = WAPC_Power::newInstance();
		if ( NULL == pc_Power ) {
		    WAPG_API_trace("%s(%d) WAPC_Power::newInstance NULL 1\n", __FILE__, __LINE__);
		    LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, WAP_LOGF_LEVEL_FATAL, "WAPC_Power::newInstance NULL 1");
		}

		// HDDデータアクセス用電源管理の省電力抑止.
		int32_t si32_Ret = pc_Power->waitStartAccessHDD();
		if( WAPD_AnyError == si32_Ret ){
			WAPG_API_trace("%s(%d) WAPC_Power::waitStartAccessHDD Error\n", __FILE__, __LINE__);
		}

		// 状態の確認とLDAP宛先検索キャンセル処理を実施.
		si32_ret = postCancelResponseGenerate( ps_Param );

		// HDDデータアクセス用電源管理の省電力抑止解除.
		si32_Ret = pc_Power->EndAccessHDD();
		if( WAPD_AnyError == si32_Ret ){
			WAPG_API_trace("%s(%d) WAPC_Power::EndAccessHDD Error\n", __FILE__, __LINE__);
		}
		}
		break;
		
	default:
		printf("405:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //405->ErrorStatus[WAPD_STATUS_METHOD_NOT_ALLOWED] in WAP_defs.h
		m_ResponseHeader->Status = WAPD_STATUS_METHOD_NOT_ALLOWED;
		WAPG_API_trace("[ERR] WAPC_LdapSearchCancelController::ResponseGenerate() Method is uncorrect!.\n");
		si32_ret = WAPD_AnyError;
		break;
	}
	
	return si32_ret;
}

/**
 * @brief		LDAP宛先検索キャンセル結果の通知用コールバック([L.LDAP検索キャンセル完了通知]).
 * @param[in]	e_LdapCancelError	LDAP接続結果.
 * @param[in]	si32_NicFumResult	NICFUM結果.
 * @return		なし.
 * @author		日立 井原.
 * @since		2017/8/8.
 * @date		2018/3/12 日立田中 LDAP宛先検索キャンセル結果取得の仕様削除により不要処理削除.
 */
void WAPC_LdapSearchCancelController::notifyLdapSearchCancelResult( TYPE_LdapCancelError e_LdapCancelError, int32_t si32_NicFumResult )
{
	WAPG_API_trace("WAPC_LdapSearchCancelController::notifyLdapSearchCancelResult() [%d][%ld].\n", e_LdapCancelError, si32_NicFumResult);
	// LDAP接続確認/検索状態管理クラスのインスタンス.
	WAPC_LdapSearchStatusMgr* pc_LdapSearchStatusMgr = WAPC_LdapSearchStatusMgr::newInstance();
	if ( NULL == pc_LdapSearchStatusMgr ) {
		WAPG_API_trace("[ERR] WAPC_LdapSearchCancelController::notifyLdapSearchCancelResult() WAPC_LdapSearchStatusMgr::newInstance() is NULL!.\n");
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
		m_ResponseHeader_Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		return;
	}
	
	// LDAP接続確認/検索状態の取得.
	const WAPE_LdapSearchStatus e_LdapSearchStatus = pc_LdapSearchStatusMgr->getLdapSearchStatus();
	if ( (WAPE_LdapSearchStatus_SearchCancel == e_LdapSearchStatus)					// LDAP検索キャンセル中.
	|| ( (WAPE_LdapSearchStatus_SearchCancelComplete == e_LdapSearchStatus) ) ) {	// LDAP検索キャンセル完了.
	} else if ( WAPE_LdapSearchStatus_SearchComplete== e_LdapSearchStatus ) {	// LDAP検索完了.
		// なにもしない.
	} else {
		WAPG_API_trace("[ERR] WAPC_LdapSearchCancelController::notifyLdapSearchCancelResult() getLdapSearchStatus() Error!.\n");
		m_ResponseHeader_Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
	}
}

/**
 * @brief		状態の確認とLDAP宛先検索キャンセル処理(POST用).
 * @param[in]	ps_Param HTTPリクエストヘッダ、ボディ.
 * @return		状態の確認とLDAP宛先検索キャンセル処理の成否, WAPD_NoError(0)：成功、WAPD_AnyError(1)：失敗.
 * @author		日立 井原.
 * @since		2017/8/8.
 * @date		2017/9/14 PFD1-2G 佐藤菜.AW.1029302 状態遷移を修正.
 */
int32_t WAPC_LdapSearchCancelController::postCancelResponseGenerate( const WAPS_SubController* ps_Param )
{
	int32_t si32_ret = WAPD_AnyError;
	
	// LDAP接続確認/検索状態管理クラスのインスタンスを取得.
	WAPC_LdapSearchStatusMgr* pc_LdapSearchStatusMgr = WAPC_LdapSearchStatusMgr::newInstance();
	if ( NULL == pc_LdapSearchStatusMgr ) {
		WAPG_API_trace("[ERR] WAPC_LdapSearchCancelController::postCancelResponseGenerate() WAPC_LdapSearchStatusMgr::newInstance() is NULL!.\n");
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		return WAPD_AnyError;
	}
	
	// LDAP接続確認/検索状態の取得.
	const WAPE_LdapSearchStatus e_LdapSearchStatus = pc_LdapSearchStatusMgr->getLdapSearchStatus();
	if ( WAPE_LdapSearchStatus_Search == e_LdapSearchStatus ) {						// LDAP検索中.
		// LDAP検索キャンセルを開始.
		si32_ret = startCancelLdapSearch();
	} else if ( ( WAPE_LdapSearchStatus_SearchComplete == e_LdapSearchStatus ) ||		// LDAP検索完了.
				( WAPE_LdapSearchStatus_GetAddress == e_LdapSearchStatus ) ) {			// 宛先リスト取得可能.
		// 即時LDAP検索キャンセル完了処理を実施.
		si32_ret = ImmediatelyCancelLdapSearch();
	} else if ( ( WAPE_LdapSearchStatus_SearchCancel == e_LdapSearchStatus ) ||				// LDAP検索キャンセル中.
				( WAPE_LdapSearchStatus_SearchCancelComplete == e_LdapSearchStatus ) ) {	// LDAP検索キャンセル完了.
		// 重複リクエストのためNG.
		printf("503:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //503->ErrorStatus[WAPD_STATUS_SERVICE_UNAVAILABLE] in WAP_defs.h
		m_ResponseHeader->Status = WAPD_STATUS_SERVICE_UNAVAILABLE;
		si32_ret = WAPD_AnyError;
	} else {
		// 検索未実施.
		printf("204:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //204->ErrorStatus[WAPD_STATUS_NO_CONTENT] in WAP_defs.h
		m_ResponseHeader->Status = WAPD_STATUS_NO_CONTENT;
		si32_ret = WAPD_NoError;
	}
	
	return si32_ret;
}

/**
 * @brief		LDAPサーバー検索キャンセルを実施する.
 * @param		なし.
 * @return		LDAPサーバー検索キャンセルの成否, WAPD_NoError(0)：成功、WAPD_AnyError(1)：失敗.
 * @author		日立 井原.
 * @since		2017/8/8.
 */
int32_t WAPC_LdapSearchCancelController::startCancelLdapSearch()
{
	// LDAP検索APIのインスタンスを取得.
	APIC_LdapSearch* pc_LdapSearch = APIC_LdapSearch::newInstance();
	if ( NULL == pc_LdapSearch ) {
		WAPG_API_trace("[ERR] WAPC_LdapSearchCancelController::startCancelLdapSearch() APIC_LdapSearch::newInstance() is NULL!.\n");
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
		return WAPD_AnyError;
	}
	
	// LDAP接続確認/検索状態管理クラスのインスタンスを取得.
	WAPC_LdapSearchStatusMgr* pc_LdapSearchStatusMgr = WAPC_LdapSearchStatusMgr::newInstance();
	if ( NULL == pc_LdapSearchStatusMgr ) {
		WAPG_API_trace("[ERR] WAPC_LdapSearchCancelController::startCancelLdapSearch() WAPC_LdapSearchStatusMgr::newInstance() is NULL!.\n");
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
		return WAPD_AnyError;
	}
	
	// LDAP接続確認/検索状態を「LDAP検索キャンセル中」に設定.
	int32_t si32_ret = pc_LdapSearchStatusMgr->setLdapSearchStatus( WAPE_LdapSearchStatus_SearchCancel );
	if ( WAPD_AnyError == si32_ret ) {
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		WAPG_API_trace("[ERR] WAPC_LdapSearchCancelController::startCancelLdapSearch() setLdapSearchStatus Error.\n");
		return WAPD_AnyError;
	}
	
	// LDAP検索キャンセルを実施.
	m_ResponseHeader_Status = WAPD_STATUS_OK;
	pc_LdapSearch->req_cancel( notifyLdapSearchCancelResult );
	
	// HTTPレスポンスヘッダの状態をWAPD_STATUS_OKに変更.
	m_ResponseHeader->Status = WAPD_STATUS_OK;
	
	return WAPD_NoError;
}

/**
 * @brief 		即時LDAP宛先検索キャンセル完了処理.
 * @par			すでにLDAP宛先検索完了状態のため、キャンセルが不要の場合の処理を行う.
 * @param		なし.
 * @return		LDAPサーバー検索キャンセルの成否, WAPD_NoError(0)：成功、WAPD_AnyError(1)：失敗.
 * @author		日立 井原.
 * @since		2017/8/8.
 * @date		2017/9/14 PFD1-2G 佐藤菜.AW.1029302 状態遷移を修正.
 * @date		2018/3/12 日立田中 LDAP宛先検索キャンセル結果取得の仕様削除により不要処理削除.
 */
int32_t WAPC_LdapSearchCancelController::ImmediatelyCancelLdapSearch()
{
	// LDAP接続確認/検索状態管理クラスのインスタンスを取得.
	WAPC_LdapSearchStatusMgr* pc_LdapSearchStatusMgr = WAPC_LdapSearchStatusMgr::newInstance();
	if ( NULL == pc_LdapSearchStatusMgr ) {
		WAPG_API_trace("[ERR] WAPC_LdapSearchCancelController::ImmediatelyCancelLdapSearch() WAPC_LdapSearchStatusMgr::newInstance() is NULL!.\n");
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
		return WAPD_AnyError;
	}
	
	// LDAP接続確認/検索状態の取得.
	WAPE_LdapSearchStatus e_LdapSearchStatus = pc_LdapSearchStatusMgr->getLdapSearchStatus();
	if ( WAPE_LdapSearchStatus_SearchComplete == e_LdapSearchStatus ) {			// LDAP検索完了.
		// LDAP接続確認/検索状態を「LDAP検索キャンセル完了」に設定.
		int32_t si32_ret = pc_LdapSearchStatusMgr->setLdapSearchStatus( WAPE_LdapSearchStatus_SearchCancelComplete );
		if ( WAPD_AnyError == si32_ret ) {
			m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
			printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
			WAPG_API_trace("[ERR] WAPC_LdapSearchCancelController::ImmediatelyCancelLdapSearch() setLdapSearchStatus(CancelComplete) Error.\n");
			return WAPD_AnyError;
		}
	} else if ( WAPE_LdapSearchStatus_GetAddress == e_LdapSearchStatus ) {		// 宛先リスト取得可能.
		// LDAP接続確認/検索状態を「Idle」に設定.
		int32_t si32_ret = pc_LdapSearchStatusMgr->setLdapSearchStatus( WAPE_LdapSearchStatus_Idle );
		if ( WAPD_AnyError == si32_ret ) {
			m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
			printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
			WAPG_API_trace("[ERR] WAPC_LdapSearchCancelController::ImmediatelyCancelLdapSearch() setLdapSearchStatus(Idle) Error.\n");
			return WAPD_AnyError;
		}
	} else {
		// なにもしない.
	}
	m_ResponseHeader->Status = WAPD_STATUS_OK;
	
	return WAPD_NoError;
}

