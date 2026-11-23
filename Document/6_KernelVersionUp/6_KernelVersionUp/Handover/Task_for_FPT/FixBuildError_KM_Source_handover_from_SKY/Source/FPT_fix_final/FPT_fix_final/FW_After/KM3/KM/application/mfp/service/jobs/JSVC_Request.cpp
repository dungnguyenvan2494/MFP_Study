////////////////////////////////////////////////////////////////////////////////////////////////////.
//.
//    \file    ファイル名：JSVC_Request.cpp. 
//.
//    \brief    概要：ジョブログ オブザーバー ヘッダファイル.
//.
////////////////////////////////////////////////////////////////////////////////////////////////////.
#include <system/uac/APIC_AuthenticationServerSetting.h>
#include <system/job/APIC_JobList.h>
#include <system/job/JOBC_JobIf.h>
#include <system/job/JOBG_Api.h>			// Archi(Step3) 送信機能カスタマイズ add.
#include <system/fil/FILF_FlatFileIf.h>		// Archi(Step3) 送信機能カスタマイズ add.
#include <system/scn/APIC_Scanner.h>		//CIT252.
#include <system/doc/APIC_Doc.h>
#include "JSVC_Common.h"
#include "JSVC_Request.h"
#include "JSVC_JobManager.h"
#include "JSVC_JobSendRecv.h"
#include "JSVC_Logging.h"

///////////////////////////////////////////////////////////////////////////////.
///////////////////////////////////////////////////////////////////////////////.

#define	JSVC_CHARGE_MAP_EXPAND_OK	CHRE_CHARGE_MAP_EXPAND_OK
///////////////////////////////////////////////////////////////////////////////.
///////////////////////////////////////////////////////////////////////////////.

#define JSVE_DELETE_LIMIT_SECOND		(60*60*24)						// 不要ファイルの放置可能時間.

#define _DEBUG_JSVC_SEQ
#define JSVC_Request_JobResourceMax			259
#define JSVC_Request_ResourceWarningDBG		0							// ジョブリソース警告用でバックフラグ.

Bool ChkJobSpec;		//JobSpec設定状況ログ（0:定数値のみ　1:文字列も）.

//--------------------------------------------------------------------------------------------------.
//	JSVC_RequestBaseクラス定義.
//--------------------------------------------------------------------------------------------------.
// コンストラクタ.
JSVC_SubScnRequestBase::JSVC_SubScnRequestBase()
:
 ul_SubScenarioID(0)
, e_Request_Status(JSVE_Request_Status_Idol)
, ul_ScenarioID(0)
{
	;
}
// サブシナリオID設定.
JSVE_Request_Result JSVC_SubScnRequestBase::setSubScenarioID( uint32_t	ul_SubScenario_ID )		
{
	ul_SubScenarioID = ul_SubScenario_ID;

	return JSVE_Request_Result_Ok;
}
// サブシナリオID取得.
uint32_t JSVC_SubScnRequestBase::getSubScenarioID()
{
	return ul_SubScenarioID;
}
// リクエストステータス取得.
JSVE_Request_Status JSVC_SubScnRequestBase::getRequestStatus()
{
	return e_Request_Status;
}
// リクエストステータス設定.
void JSVC_SubScnRequestBase::setRequestStatus(JSVE_Request_Status eRequest_Status)
{
	e_Request_Status = eRequest_Status;
}
// ログID設定.
JSVE_Request_Result JSVC_SubScnRequestBase::setScenarioID( uint32_t	ul_ScenarioID_ )
{
	ul_ScenarioID = ul_ScenarioID_;

	return JSVE_Request_Result_Ok;
}
// ログID取得.
uint32_t JSVC_SubScnRequestBase::getScenarioID()
{
	return ul_ScenarioID;
}

JSVC_Node::JSVC_Node(uint32_t ul_MsgTypeId)
:	ul_Result(JSVD_JobsResult_InitializeValue),ul_ResultDetail(JSVD_JobsResult_InitializeValue),ul_MsgTypeID(ul_MsgTypeId)
{
}

//--------------------------------------------------------------------------------------------------.
//	JSVC_RequestBaseクラス定義.
//--------------------------------------------------------------------------------------------------.
JSVC_RequestBase::JSVC_RequestBase(uint32_t ul_RequestID,uint32_t ul_MsgTypeID)
: JSVC_Node(ul_MsgTypeID),MFPC_Request(ul_RequestID)
{
}

//--------------------------------------------------------------------------------------------------.
//	比較関数.
//--------------------------------------------------------------------------------------------------.
Bool JSVC_RequestBase::checkEqualRequest(MFPS_PacketHeader* pPacketHeader)
{
	if( (this->get_srcinfo()->reqkindid == pPacketHeader->reqkindid) &&
		(this->get_srcinfo()->requestid == pPacketHeader->requestid) &&
		memcmp(&(this->get_srcinfo()->saddr), &(pPacketHeader->saddr), sizeof(sockaddr_un)) == 0 )
	{
		return True;	// リクエストが一致.
	}else{
		return False;	// リクエストが不一致.
	}
}

//--------------------------------------------------------------------------------------------------.
//	JSVC_Requestクラス定義.
//--------------------------------------------------------------------------------------------------.
JSVC_Request::JSVC_Request( uint32_t ul_RequestID,uint32_t ul_MsgTypeID)
: JSVC_SubScnRequestBase(),JSVC_RequestBase(ul_RequestID,ul_MsgTypeID)
{
	// 初期値設定.
	ul_DetailParam = 0;				///< リクエストに関する詳細パラメータ.
	e_DataType = 0;					// 内包するデータの型を保持.
	e_Request_Status = JSVE_Request_Status_Idol;
	ul_SequenceId = 0;				// Step2 受信アプリ対応 Add.

	ul_DetailParam = 0;

	pc_JobManager = JSVC_JobManager::newInstance();
	pc_XmlAnalyze = JSVC_XmlAnalyzeTask::newInstance();

	ul_ScenarioID = 0;
	e_ReceptionModuleType = TYPE_ReceptionModuleType_NONE;

	// 関数テーブルへの関数のアドレスの割り当て--------------------------------
	int iEventCnt = 0;
	puc_FuncTable[iEventCnt]   = startAction00;
	puc_FuncTable[++iEventCnt] = startAction01;
	puc_FuncTable[++iEventCnt] = startAction02;
	puc_FuncTable[++iEventCnt] = startAction03;
	puc_FuncTable[++iEventCnt] = startAction04;
	puc_FuncTable[++iEventCnt] = startAction05;
	puc_FuncTable[++iEventCnt] = startAction06;
	puc_FuncTable[++iEventCnt] = startAction07;
	puc_FuncTable[++iEventCnt] = startAction08;
	puc_FuncTable[++iEventCnt] = startAction09;
	puc_FuncTable[++iEventCnt] = startAction10;
	puc_FuncTable[++iEventCnt] = startAction11;
	puc_FuncTable[++iEventCnt] = startAction12;
	puc_FuncTable[++iEventCnt] = startAction13;
	puc_FuncTable[++iEventCnt] = startAction14;
	puc_FuncTable[++iEventCnt] = startAction15;
	puc_FuncTable[++iEventCnt] = startAction16;
	puc_FuncTable[++iEventCnt] = startAction17;
	puc_FuncTable[++iEventCnt] = startAction18;
	puc_FuncTable[++iEventCnt] = startAction19;
	puc_FuncTable[++iEventCnt] = startAction20;
	puc_FuncTable[++iEventCnt] = startAction21;
	puc_FuncTable[++iEventCnt] = startAction22;
	puc_FuncTable[++iEventCnt] = startAction23;
}
// デストラクタ.
JSVC_Request::~JSVC_Request()
{
}
JSVC_ParentRequest* JSVC_Request::GetParentRequest(){
	return JSVC_JobManager::newInstance()->getParentRequest(getSubScenarioID());;
}
// ログID設定.
JSVE_Request_Result JSVC_Request::setScenarioID( uint32_t	ul_ScenarioID )
{
	JSVC_SubScnRequestBase::setScenarioID( ul_ScenarioID );

	return JSVE_Request_Result_Ok;
}
// ログID取得.
uint32_t JSVC_Request::getScenarioID()
{
	return JSVC_SubScnRequestBase::getScenarioID();
}

// ジョブインスタンス設定.
JSVE_Request_Result JSVC_Request::setJobInstance( APIC_Job*	pJob )
{
	//Str【 静的解析対応 : Sky花岡 : 2012.09.03 : CID-439621】.
	// 入力チェック.
	if ( NULL == pJob || NULL == GetParentRequest() ){
		return JSVE_Request_Result_Err;
	}
	//End【 静的解析対応 : Sky花岡 : 2012.09.03 : CID-439621】.

	return GetParentRequest()->setJobInstance(pJob);
}
// ジョブインスタンス取得.
APIC_Job* JSVC_Request::getJobInstance( )
{
	 APIC_Job*	pJob = NULL;
	JSVC_ParentRequest* pPReqest = GetParentRequest();

	if(( NULL != pPReqest )
	&& ( JSVE_Request_Result_Ok == pPReqest->getJobInstance(&pJob) )){
		return pJob;
	} else {
		return NULL;
	}
}
// 自クラスの型を返す.
JSVE_Request_Class_Type JSVC_Request::WhoAmI( void )
{
	return JSVE_Request_Class_Type_Request;
}
// リクエスト受付時のデータ保存処理.
Bool JSVC_Request::setData(const char* pRcvData,short ss_size)
{
	//エラーハンドリング以外で生成されないので.
	//データ保持は必要なし.
#ifdef _DEBUG_JSVC_SEQ
	MyTraceSLD(JSVD_DBG_INFO, "JSVC_Request::setData pRcvData:" , (intptr_t)pRcvData );		// 回避方法をトレースに変更.
	MyTraceSD(JSVD_DBG_INFO, "JSVC_Request::setData ss_size:" , ss_size );		// 回避方法をトレースに変更.
#endif
	return True;
}

// 情報通知.
JSVE_Request_Result JSVC_Request::startAction( JSVS_NotifyInfo*	ps_NotifyInfo )
{
	JSVE_Request_Result e_Request_Result = JSVE_Request_Result_Err;
	JSVE_Request_Event e_Request_Event = JSVE_Request_Event_Non;

	// パラメータチェック
	if(ps_NotifyInfo == NULL){
		MyTraceSS(JSVD_DBG_ERROR, "JSVC_Request::startAction param error" , "ps_NotifyInfo" );
		return e_Request_Result;
	}

	// ローカルイベントに変換する
 	e_Request_Event= changeLocalEvent(ps_NotifyInfo->e_JobManager_MsgTypeID);
 	ps_NotifyInfo->e_Request_Event = e_Request_Event;

	// 範囲外のイベントIDであれば無視する。
	if ((JSVE_Request_Event_Non >= e_Request_Event) ||
		(JSVE_Request_Event_Max <= e_Request_Event)){
		MyTraceSD(JSVD_DBG_ERROR, "JSVC_Request::startAction param Request_Event error" , e_Request_Event );
		return e_Request_Result;
	}

#ifdef _DEBUG_JSVC_SEQ
	{
		char szLogBuf[200+1];
		memset( szLogBuf ,0 ,sizeof(szLogBuf) );
		snprintf( szLogBuf ,200,"ReqSts(%d) ReqEvnt(%d)" ,e_Request_Status,e_Request_Event);
		MyTraceSS(JSVD_DBG_INFO, "JobL ", szLogBuf );
	}
#endif
	JSVE_Request_Status e_Request_Status_old = e_Request_Status;

	// 指定したイベントIDに対応する処理を実行する
	JSVE_Request_Result ret = (*puc_FuncTable[e_Request_Status])(ps_NotifyInfo, this);
#ifdef _DEBUG_JSVC_SEQ
	{
		char szLogBuf[200+1];
		memset( szLogBuf ,0 ,sizeof(szLogBuf) );
		snprintf( szLogBuf ,200,"->ReqSts(%d)" ,e_Request_Status);
		MyTraceSS(JSVD_DBG_INFO, "JobL ", szLogBuf );
	}
#endif
	// ログ.
	_setJsrvSeqLog( TYPE_FuncName_08, ul_SubScenarioID, e_Request_Event, e_Request_Status_old, e_Request_Status, NULL );

	return ret;
}

// 情報通知から状態に応じた処理を実行する.
///< 初期値.
JSVE_Request_Result JSVC_Request::startAction00( JSVS_NotifyInfo*	ps_NotifyInfo, JSVC_Request* pc_Request )
{
	JSVE_Request_Result e_Request_Result = JSVE_Request_Result_Ok;
#ifdef _DEBUG_JSVC_SEQ
	MyTraceSLD(JSVD_DBG_INFO, "JSVC_Request::startAction00 ps_NotifyInfo:" , (intptr_t)ps_NotifyInfo );	// 回避方法をトレースに変更.
	MyTraceSLD(JSVD_DBG_INFO, "JSVC_Request::startAction00 pc_Request:" , (intptr_t)pc_Request );		// 回避方法をトレースに変更.
#endif
	// 処理なし.

	return e_Request_Result;
}
// 情報通知から状態に応じた処理を実行する.
///< アイドル中(メモリ未確保).
JSVE_Request_Result JSVC_Request::startAction01( JSVS_NotifyInfo*	ps_NotifyInfo, JSVC_Request* pc_Request )
{
	JSVE_Request_Result e_Request_Result = JSVE_Request_Result_Err;

	TYPE_XmlResultStatus e_XmlResultStatus = TYPE_XmlResultStatus_ReqOK;

	switch( ps_NotifyInfo->e_Request_Event )
	{
		case JSVE_Request_Event_Non				:	///< 初期値.
			break;
		case JSVE_Request_Event_ReqMemAlloc		:	///< メモリ確保要求.
			{
				// ファイルI/Fでの動作かジョブリソース指定の動作かを判断.
				if ((uint32_t)-1 != ((JSVC_RequestJobSetting*)pc_Request)->getJobResourceID()) {		// warning対応 2013/09/21 ooba.
					//新処理ではリソースIDによる指定なので、繋ぐためのMSGを送信するのみ.
					pc_Request->setResult(JSVE_Request_Result_Ok);
					e_XmlResultStatus = TYPE_XmlResultStatus_ReqOK;
				} else {
					//旧処理ではファイル指定.
					e_XmlResultStatus = pc_Request->pc_XmlAnalyze->req_StartXmlAnalyze(pc_Request->ul_SubScenarioID, ((JSVC_RequestJobSetting*)pc_Request)->getFilePath(), pc_Request, ans_Analyze);
				}
				if (TYPE_XmlResultStatus_ReqOK == e_XmlResultStatus){																		// 受付完了.
					// JobManagerへ要求OKを通知する.
					pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Ok, JSVE_JobServiceResultDetail_Non, pc_Request);
					// 状態更新(XML解析中).
					pc_Request->e_Request_Status = JSVE_Request_Status_Analyze;
					// 結果設定.
					e_Request_Result = JSVE_Request_Result_Ok;

					//Accept送信後に贈る必要があるので再度リソースIDをチェックする.
					if ((uint32_t)-1 != ((JSVC_RequestJobSetting*)pc_Request)->getJobResourceID()) {		// warning対応 2013/09/21 ooba.
						JSVC_JobManager::newInstance()->notify( pc_Request , JSVE_JobServiceResult_OK , IPCE_JobsResultDetail_Non , JSVE_JobManager_MsgID_AnsXmlAnalyze , 0 );
					}
				}else{
					// JobManagerへ要求NGを通知する.
					pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, e_Request_Result, JSVE_JobServiceResultDetail_NotMemAlloc, pc_Request);
				}
			}
			break;
		case JSVE_Request_Event_AnsAnalyzeCancel:	///< XML解析削除応答.
			break;
		case JSVE_Request_Event_AnsAnalyzeEnd	:	///< XML解析完了応答.
			break;
		case JSVE_Request_Event_AnsMemAllocEnd	:	///< メモリ確保応答.
			break;
		case JSVE_Request_Event_ReqStart		:	///< サブシナリオ開始要求.
			// JobManagerへ要求NGを通知する.
			e_Request_Result = JSVE_Request_Result_Ok;
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Err, JSVE_JobServiceResultDetail_NotMemAlloc, pc_Request);
			break;
		case JSVE_Request_Event_ReqStop			:	///< サブシナリオ停止要求.
			// JobManagerへ要求NGを通知する.
			e_Request_Result = JSVE_Request_Result_Ok;
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Err, JSVE_JobServiceResultDetail_NotMemAlloc, pc_Request);
			break;
		case JSVE_Request_Event_AnsStopEnd		:	///< サブシナリオ停止応答.
			break;
		case JSVE_Request_Event_ReqUpdate		:	///< サブシナリオ設定変更要求.
			// JobManagerへ要求NGを通知する.
			e_Request_Result = JSVE_Request_Result_Ok;
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Err, JSVE_JobServiceResultDetail_NotMemAlloc, pc_Request);
			break;
		case JSVE_Request_Event_AnsUpdateEnd	:	///< サブシナリオ設定変更応答.
			break;
		case JSVE_Request_Event_ReqRestart		:	///< サブシナリオ再開要求.
			// JobManagerへ要求NGを通知する.
			e_Request_Result = JSVE_Request_Result_Ok;
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Err, JSVE_JobServiceResultDetail_NotMemAlloc, pc_Request);
			break;
		case JSVE_Request_Event_AnsRestartEnd	:	///< サブシナリオ再開応答.
			break;
		case JSVE_Request_Event_ReqCancel		:	///< サブシナリオ削除要求.
			// JobManagerへ要求NGを通知する.
			e_Request_Result = JSVE_Request_Result_Ok;
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Err, JSVE_JobServiceResultDetail_NotMemAlloc, pc_Request);
			break;
		case JSVE_Request_Event_AnsCancel		:	///< サブシナリオ削除応答.
			break;
		case JSVE_Request_Event_AnsEnd			:	///< サブシナリオ終了通知.
			break;
		case JSVE_Request_Event_ReqMemFree		:	///< メモリ開放要求.
			// JobManagerへ要求NGを通知する.
			e_Request_Result = JSVE_Request_Result_Ok;
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Err, JSVE_JobServiceResultDetail_NotMemAlloc, pc_Request);
			break;
		case JSVE_Request_Event_AnsMemFree		:	///< メモリ開放応答.
			break;
		case JSVE_RequestAnswer_Event_ChargeMapPrepared			:	///< 課金マップ展開完了.
			break;
		case JSVE_RequestAnswer_Event_NewInstance				:	///< 待ちうけ部からの着信通知.
			break;
		case JSVE_RequestAnswer_Event_addJob					:	///< ActiveJobListからの状態変化通知(ジョブの追加).
			break;
		case JSVE_RequestAnswer_Event_DocSearcher				:	///< DocSearcherからの状態変化通知.
			break;
		case JSVE_Request_Event_Max				:	///< 最大値(23).
			#ifdef _DEBUG_JSVC_SEQ
			MyTraceSD(JSVD_DBG_ERROR, "最大値:処理なし。startAction01" , 0 );
			#endif
			break;
		default:
			#ifdef _DEBUG_JSVC_SEQ
			MyTraceSD( JSVD_DBG_ERROR, "default。startAction01" , ps_NotifyInfo->e_Request_Event );
			#endif
			break;
	}

	return e_Request_Result;
}
// 情報通知から状態に応じた処理を実行する.
///< XML解析中.
JSVE_Request_Result JSVC_Request::startAction02( JSVS_NotifyInfo*	ps_NotifyInfo, JSVC_Request* pc_Request )
{
	JSVE_Request_Result e_Request_Result = JSVE_Request_Result_Err;

	TYPE_XmlResultStatus e_XmlResultStatus = TYPE_XmlResultStatus_ReqOK;

	switch( ps_NotifyInfo->e_Request_Event )
	{
		case JSVE_Request_Event_Non				:	///< 初期値.
			break;
		case JSVE_Request_Event_ReqMemAlloc		:	///< メモリ確保要求.
			// JobManagerへ要求NGを通知する.
			e_Request_Result = JSVE_Request_Result_Ok;
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Err, JSVE_JobServiceResultDetail_XmlAnalyze, pc_Request);
			break;
		case JSVE_Request_Event_AnsAnalyzeCancel:	///< XML解析削除応答.
			break;
		case JSVE_Request_Event_AnsAnalyzeEnd	:	///< XML解析完了応答.
			if ( ((JSVC_RequestJobSetting*)pc_Request)->getJobResourceID() != (uint32_t)-1 ){		// warning対応 2013/09/21 ooba.
				//============================================================================================.
				// 入力データチェック.
				// 前処理の結果をチェック.
				//============================================================================================.
				// 前処理の結果を確認.
				// 正常終了の場合、処理継続する.
				// 異常終了の場合、JobManagerへ要求NGを通知する.
				if (JSVE_Request_Result_Ok != ps_NotifyInfo->pReq->getResult()){
					MyTraceSD(JSVD_DBG_ERROR, "JSVC_RequestJobSetting::startAction02 1 ERR" , (int)0 );
					pc_Request->pc_JobManager->notify( pc_Request, e_Request_Result, JSVE_JobServiceResultDetail_XmlAnalyze, JSVE_JobManager_MsgID_MemoryAllocEnd, 0 );
					break;
				}
				// 不正なリクエスト型を使用していないか確認.
				if (JSVE_Request_Class_Type_Req_Job_Set != pc_Request->WhoAmI()) {
					MyTraceSD(JSVD_DBG_ERROR, "JSVC_RequestJobSetting::startAction02 2 ERR" , (int)0 );
					pc_Request->pc_JobManager->notify( pc_Request, JSVE_JobServiceResult_NG, JSVE_JobServiceResultDetail_IllegalRequest, JSVE_JobManager_MsgID_MemoryAllocEnd, 0 );
					break;
				}
				//============================================================================================.
				// 復活のチェック.
				// 再投入のサブシナリオがDocSearcherのリトライリストに存在しない場合.
				// サブシナリオは完了しているものとして扱う.
				//============================================================================================.
				//Str【 静的解析対応 : Sky花岡 : 2012.09.03 : CID-439589】.
				if( NULL == pc_Request->GetParentRequest() ){
					MyTraceSD(JSVD_DBG_ERROR, "JSVC_RequestJobSetting::startAction02 GetParentRequest ERR" , (int)0 );
					break;
				}
				//End【 静的解析対応 : Sky花岡 : 2012.09.03 : CID-439589】.
				if( TYPE_PowerOnReStartType_ON == pc_Request->GetParentRequest()->getPowerOnReStartType() ){
					pc_Request->GetParentRequest()->setListenerType(JSVC_IsListner_RetryListner);
					short ss_SearchIndex = 0;
					int32_t sl_getType = 0;
					if ( False == APIC_DocSearcher::newInstance()->isExistRetryListByJobID( pc_Request->ul_SubScenarioID, &ss_SearchIndex, &sl_getType )){
						MyTraceSD(JSVD_DBG_ERROR, "JSVC_RequestJobSetting::startAction02 3 ERR" , (int)0 );
						pc_Request->pc_JobManager->notify( pc_Request, JSVE_JobServiceResult_NG, JSVE_JobServiceResultDetail_NotRevivalEnd, JSVE_JobManager_MsgID_MemoryAllocEnd, 0 );
						break;
					}else{
						// 復旧待ち順番、処理タイプを保持する.
						pc_Request->GetParentRequest()->setRevivalIndex(ss_SearchIndex);
						pc_Request->GetParentRequest()->setRevivalJobType(sl_getType);
						// 発見順番がカレントでない場合、DocSearcherへのリトライを実行するタイミングではないため、
						// リクエストにリトライ待ちを設定し、次のタイミングまで待つ.
						if (ss_SearchIndex <= 0){
							// isExistRetryListByJobIDのAPIエラー.
							MyTraceSD(JSVD_DBG_ERROR, "JSVC_RequestJobSetting::startAction02 4 ERR" , (int)0 );
							break;
						}else if (ss_SearchIndex == 1){
							// 先頭の場合、復活送信のチェックへ.
						}else{
							// 次リクエストの復旧時のnewInstance時にDocSearcherへの再リトライ判断を行い.
							// jobRegistPermissionDocSearcherする.
							break;
						}
					}
				}
				//============================================================================================.
				// 復活送信のチェック.
				// 復活送信の場合は、リソースが確保されていない状態でServiceManagerからシナリオ投入される.
				// そのため、ここでチェックを行いDocSearcherに起動をかけるようにする。
				//============================================================================================.
				if( TYPE_PowerOnReStartType_ON == pc_Request->GetParentRequest()->getPowerOnReStartType() ){
					//----------------------------------------------------------------------------------------.
					// 受信系：DocSercherへリトライを促す.
					//----------------------------------------------------------------------------------------.
					MyTraceSS(JSVD_DBG_INFO, "JSVC_Request::startAction02 ","DocSercherへのリトライを促す");
					pc_Request->GetParentRequest()->setListenerType(JSVC_IsListner_RetryListner);
//					APIC_JobInstanceList::jobRegistPermissionDocSearcher(pc_Request->GetParentRequest()->getRevivalJobType());
					APIC_JobInstanceList::jobRegistPermissionDocSearcher(TYPE_JobTypeFaxSendTimer);
					APIC_JobInstanceList::jobRegistPermissionDocSearcher(TYPE_JobTypeFaxReceive);
					APIC_JobInstanceList::jobRegistPermissionDocSearcher(TYPE_JobTypeFaxSendMemory);
					APIC_JobInstanceList::jobRegistPermissionDocSearcher(TYPE_JobTypePrint);
					break;
				}
				//============================================================================================.
				// ジョブインスタンス取得.
				//============================================================================================.
				APIC_Job* pc_Job = getInstancebyResourceId( ((JSVC_RequestJobSetting*)pc_Request)->getJobResourceID() );

				//============================================================================================.
				// 入力データチェック.
				// JobSpecの内容をチェック.
				//============================================================================================.
				if ( NULL == pc_Job ) {
					MyTraceSD(JSVD_DBG_ERROR, "JSVC_RequestJobSetting::startAction02 cant' get jobInstanse ERR" , 0 );
					pc_Request->pc_JobManager->notify( pc_Request, JSVE_JobServiceResult_NG, IPCE_JobsResultDetail_JobPointerNull, JSVE_JobManager_MsgID_MemoryAllocEnd, 0 );
					break ;
				}
				if (False == pc_Request->checkJobSpec(pc_Job->getJobSpec())) {
					MyTraceSD(JSVD_DBG_ERROR, "JSVC_RequestJobSetting::startAction02 checkJobSpec ERR" , 0 );
					pc_Request->pc_JobManager->notify( pc_Request, JSVE_JobServiceResult_NG, IPCE_JobsResultDetail_InputErr, JSVE_JobManager_MsgID_MemoryAllocEnd, 0 );
					break ;
				}

				//============================================================================================.
				// バッファの確保と設定値の取り込み.
				//============================================================================================.
				// CQ377138対応 2013.01.23 Start.
				Bool b_InvalidJob = pc_Job->getInvalidJob();
				if (True == b_InvalidJob){
					pc_Request->pc_JobManager->notify( pc_Request, JSVE_JobServiceResult_NG, IPCE_JobsResultDetail_InputErr, JSVE_JobManager_MsgID_MemoryAllocEnd, 0 );
					break ;
				}
				// CQ377138対応 2013.01.23 End.

				//============================================================================================.
				// 親リクエストを取得.
				//============================================================================================.
				uint32_t					ulID;
				JSVC_ParentRequest*		pc_PRequest;
				ulID = pc_Request->getSubScenarioID();
				pc_PRequest = JSVC_JobManager::newInstance()->getParentRequest(ulID);
				if (NULL == pc_PRequest) {
					MyTraceSLD(JSVD_DBG_ERROR, "JSVC_RequestJobSetting::startAction02 getParentRequest ERR" , (intptr_t)pc_PRequest );
					pc_Request->pc_JobManager->notify( pc_Request, JSVE_JobServiceResult_NG, IPCE_JobsResultDetail_IdNotMatch, JSVE_JobManager_MsgID_MemoryAllocEnd, 0 );
					break ;
				}


				//============================================================================================.
				// ジョブへの関連付けを行なう.
				//============================================================================================.
				do
				{
				
					pc_PRequest->addInstance( pc_Job );
					// JobSpecの生成成功.
					// ・JOBが持っているJobSpecにシーケンスIDが設定済みの場合、すでにジョブが動作している.
					// ・受信系の場合は上位がJobSpecにシーケンスID（受信データID）をセットしてくる.
					if((0 != pc_Job->getJobSpec()->pc_Common->getSequenceId() )
					|| ( TYPD_ImageLogSendJobNon != pc_Job->getJobSpec()->pc_Common->getImageLogSend()) ){				// Archi(Step3) 送信機能カスタマイズ (条件追加).
					//--------------------------------------------------------------------------------------------.
					// 受信系の場合.
					//--------------------------------------------------------------------------------------------.

						MyTraceSS(JSVD_DBG_INFO, "主に受信系の動作","");

						//----------------------------------------------------------------------------------------.
						// 指定されたジョブが割付済だった場合はNGとする.
						// （同じインスタンスに対して複数回のメモリ確保要求が来ている）.
						//----------------------------------------------------------------------------------------.

						//----------------------------------------------------------------------------------------.
						// 受信種類をジョブインスタンスへ設定.
						// (暫定処理のため見直しの必要がある).
						//----------------------------------------------------------------------------------------.
						// 受信モジュール種別を設定する.
						// 2013/05/07 復旧ジョブシーケンス見直し対応(CQ379669) modify start.
						//IPCE_JobsRcvJobType e_JobsRcvJobType = JSVC_JobManager::newInstance()->getRcvJobType( pc_Job->getJobSpec(), pc_Job->getJobStatus() );
						IPCE_JobsRcvJobType e_JobsRcvJobType = JSVC_JobManager::newInstance()->getRcvJobType( pc_Job );
						// 2013/05/07 復旧ジョブシーケンス見直し対応(CQ379669) modify end.
						TYPE_ReceptionModuleType e_Type = TYPE_ReceptionModuleType_NONE;
						switch(e_JobsRcvJobType){
							case IPCE_RcvJobType_Non: e_Type = TYPE_ReceptionModuleType_NONE; break;
							case IPCE_RcvJobType_Net: e_Type = TYPE_ReceptionModuleType_NetRx; break;
							case IPCE_RcvJobType_Fax: e_Type = TYPE_ReceptionModuleType_FaxRx; break;
							case IPCE_RcvJobType_Tcp: e_Type = TYPE_ReceptionModuleType_TcpSocket; break;
							case IPCE_RcvJobType_PSWC: e_Type = TYPE_ReceptionModuleType_PSWC; break;
							case IPCE_RcvJobType_OpenAPI: e_Type = TYPE_ReceptionModuleType_Oap; break;
							case IPCE_RcvJobType_Browser: e_Type = TYPE_ReceptionModuleType_Browser; break;
							case IPCE_RcvJobType_ImageLog: e_Type = TYPE_ReceptionModuleType_IGLSend; break;

/* CQ370925対応 2012/08/28 S.Wakayama Start */
							case IPCE_RcvJobType_Backup: e_Type = TYPE_ReceptionModuleType_Backup; break;
							case IPCE_RcvJobType_FwUpdate: e_Type = TYPE_ReceptionModuleType_FwUpdate; break;
/* CQ370925対応 2012/08/28 S.Wakayama End */

							default: break;
						}
						pc_PRequest->setReceptionModuleType(e_Type);
						//----------------------------------------------------------------------------------------.
						// 検出したジョブインスタンスへ値を設定.
						//----------------------------------------------------------------------------------------.
						// JobIDのFixをオブザーバー通知する.
						JSVC_JobManager::newInstance()->SubScenarioFixedJobID(((JSVC_RequestJobSetting*)pc_Request)->GetParentRequest());
						// サブシナリオIDを設定.
						pc_Job->getJobStatus()->setSubScenarioID(pc_Request->ul_SubScenarioID);
						// ログIDを設定.
						pc_Job->getJobStatus()->setScenarioID(pc_Request->ul_ScenarioID);
						// JSVC_JobSpecインスタンスは開放しない.
						// サブシナリオの実行要求時にJobSpecの上書きを行う.

						// 2013/01/30 CQ377138対応 Jobインスタンス取得不可時のシナリオ登録応答 modify start.
						// 2012/09/21 CQ372110 IWSへのans対応 add start.
						// 受信系の場合は、すでにJobのans_start()の処理は完了しているので、結果OKで通知する.
						//pc_Request->pc_JobManager->NotifyJobAnsStart(JSVE_Request_Result_Ok, pc_Job);
						pc_Request->pc_JobManager->NotifyJobAnsStart(JSVE_Request_Result_Ok, pc_Request);
						// 2012/09/21 CQ372110 IWSへのans対応 add end.
						// 2013/01/30 CQ377138対応 Jobインスタンス取得不可時のシナリオ登録応答 modify end.
					}else{
					//--------------------------------------------------------------------------------------------.
					// コピー/復活系の場合.
					//--------------------------------------------------------------------------------------------.

						MyTraceSS(JSVD_DBG_INFO, "コピー系/復活系の動作","");

//Str【 静的解析対応 : Sky平林 : 2011.08.27 : CID-446755 】
						TYPE_JobType			e_JobType = TYPE_JobTypeCopy;
//End【 静的解析対応 : Sky平林 : 2011.08.27 : CID-446755 】
						JSVC_IsListnerType		ul_RetIsList;

						// 待ち受け/非待ち受けJobの判定.
						ul_RetIsList = JSVC_JobManager::newInstance()->isListenerFunc(pc_Job->getJobSpec(), pc_Request->ul_SubScenarioID, &e_JobType);
// ※先ほど消した処理はリスナー判定としては必要なため、先に呼ばれた結果リスナー On である場合はイレギュラーとして NG を返す
						if (JSVC_IsListner_Listner == ul_RetIsList) {
							MyTraceSS(JSVD_DBG_ERROR, "JSVC_Request::startAction02 コピー系/復活系なのにリスナー" , "" );
							// JobManagerへ要求NGを通知する.
							e_Request_Result = JSVE_Request_Result_Err;
							pc_Request->setResult(JSVE_JobServiceResult_NG);
							pc_Request->setResultDetail(IPCE_JobsResultDetail_CBNumOver);
							pc_Request->pc_JobManager->notify( pc_Request, e_Request_Result, JSVE_JobServiceResultDetail_XmlAnalyze, JSVE_JobManager_MsgID_MemoryAllocEnd, 0 );
							break;
						}
						if (JSVC_IsListner_NG == ul_RetIsList) {
							MyTraceSD(JSVD_DBG_ERROR, "JSVC_RequestJobSetting::newJobInstance_byBinaryFile isListenerFunc ERR" , (int)ul_RetIsList );
							pc_Request->pc_JobManager->notify( pc_Request, JSVE_JobServiceResult_NG, IPCE_JobsResultDetail_InputValueError, JSVE_JobManager_MsgID_MemoryAllocEnd, 0 );
							break ;
						}
						#if DEBUGLOG__JOBSTART_JUDGE
							MyTraceSD(JSVD_DBG_INFO, "JobType = ",e_JobType);
						#endif

						if( NULL != pc_Job->getKeyDevice() ){
							// CQ378969,CQ378971:強制メモリ複数印刷とセキュリティ複数印刷時に課金されない対応.2013/04/10 m.ooba start.
							TYPE_JobType e_PrintJobType = 0;
							int32_t sl_BoxNo = 0;
							// CQ378969,CQ378971:強制メモリ複数印刷とセキュリティ複数印刷時に課金されない対応.2013/04/10 m.ooba end.
							// 課金対象装置.
							switch( pc_Job->getJobSpec()->pc_Common->getChargeKindID() ){
							case SYSE_UnitId_VendorKM: // 統合ベンダー.
								// CQ378969,CQ378971:強制メモリ複数印刷とセキュリティ複数印刷時に課金されない対応.2013/04/10 m.ooba start.
								e_PrintJobType = pc_Job->getJobSpec()->pc_Common->c_JobType.getJobType();
								sl_BoxNo = pc_Job->getJobSpec()->pc_Common->c_JobType.getBoxNo();
								// 強制メモリ印刷とセキュリティ印刷時はsetFeeCollectionJob()を行わない.
								if (!((TYPE_JobTypeSecurePrint == e_PrintJobType) || 
								    ((TYPE_JobTypePrint == e_PrintJobType) && (DOCD_CompulsionRcvBox == sl_BoxNo)))){
									pc_Job->getKeyDevice()->setFeeCollectionJob( true, pc_Job->getKeyDevice() );
								}
								// CQ378969,CQ378971:強制メモリ複数印刷とセキュリティ複数印刷時に課金されない対応.2013/04/10 m.ooba end.
								break;
							default:
								break;
							}
						}


						//----------------------------------------------------------------------------------------.
						// 確保したジョブインスタンスへ値を設定.
						//----------------------------------------------------------------------------------------.
						// サブシナリオIDを設定.
						pc_Job->getJobStatus()->setSubScenarioID(pc_Request->ul_SubScenarioID);
						// ログIDを設定.
						pc_Job->getJobStatus()->setScenarioID(pc_Request->ul_ScenarioID);
						// 受信の場合はreq_startのトリガを取れないので、この時点で一旦Job制御フラグを変更しておく.
						pc_Job->getJobSpec()->pc_Common->setScenarioExecuteFlg(True);		//シナリオフラグをTrueへ(JobSからは常にTrue).
						pc_Job->getJobSpec()->pc_Common->setInitProcessFlg(False);			//InitプロセスフラグをFalseへ.
						pc_Job->getJobSpec()->pc_Common->setEndProcessFlg(False);			//EndプロセスフラグをFalseへ.
					}

					TYPE_JobType e_JobType = 0;
					if (JSVC_IsListner_RetryListner == JSVC_JobManager::newInstance()->isListenerFunc(pc_Job->getJobSpec(), pc_Request->ul_SubScenarioID, &e_JobType)) {
						//----------------------------------------------------------------------------------------.
						// 受信系：DocSercherへリトライを促す.
						//----------------------------------------------------------------------------------------.
						MyTraceSS(JSVD_DBG_INFO, "JSVC_Request::startAction02 ","DocSercherからのリトライ済み");
					} else {
						//----------------------------------------------------------------------------------------.
						// コピー/復活系：Chargeへ課金MAPの展開を要求.
						//----------------------------------------------------------------------------------------.

						uint32_t ul_index;
						ul_index = JSVC_JobManager::newInstance()->availableAnsFunctionIndex(JSVC_CB_INDEX_ans_CallbackExpandChargeMap, JSVC_CB_MAX_ans_CallbackExpandChargeMap, pc_Request );
						if( ul_index != (uint32_t)-1 )		// warning対応 2013/09/21 ooba.
						{
							// 課金MAP展開要求.
							TYPC_JobSpec *pJobSpec = pc_Job->getJobSpec();
							APIC_ChargingIF* pCharge = pc_Job->getAppAccount();
							if( NULL != pCharge )	//ここまでの処理でpJob、pJobSpecは取得できているのでpChargeだけ.
							{
								// 課金対象ジョブの登録.
								pCharge->setFeeCollectionJob( True );
								pCharge->reqExpandChargeMap( (void (*)( uint32_t ul_SubScenarioId, SYSE_ChgIFResultStatus e_Result ))(JSVC_JobManager::newInstance()->c_CBManage[ul_index].vp_Func), 
															pc_Job->getJobStatus()->getSubScenarioID(),
															pJobSpec->pc_Common->getChargeMapID(),
															pJobSpec->pc_Common->getChargeKindID());
								//課金MAP展開待ちへ遷移.
								pc_Request->e_Request_Status = JSVE_Request_Status_ChargeMapOpenWait;
								// 結果設定.
								e_Request_Result = JSVE_Request_Result_Ok;
							} else {
								// JobManagerへ要求NGを通知する.
								pc_Request->pc_JobManager->notify( pc_Request, e_Request_Result, IPCE_JobsResultDetail_ChargeGetErr, JSVE_JobManager_MsgID_MemoryAllocEnd, 0 );
								break ;
							}
						}
						else
						{
							MyTraceSD(JSVD_DBG_ERROR, "JSVC_Request::startAction02 AnsAnalyzeErr. e_Request_Event:" , ps_NotifyInfo->e_Request_Event );
							// JobManagerへ要求NGを通知する.
							e_Request_Result = JSVE_Request_Result_Err;
							pc_Request->setResult(JSVE_JobServiceResult_NG);
							pc_Request->setResultDetail(IPCE_JobsResultDetail_CBNumOver);
							pc_Request->pc_JobManager->notify( pc_Request, e_Request_Result, IPCE_JobsResultDetail_CBNumOver, JSVE_JobManager_MsgID_MemoryAllocEnd, 0 );
							break ;
						}
					}
				}while(0);

			} else {
				// 正常終了の場合、ジョブを開始する.
				// 異常終了の場合、JobManagerへ要求NGを通知する.
				if (JSVE_Request_Result_Ok == ps_NotifyInfo->pReq->getResult()){

					if (JSVE_Request_Class_Type_Req_Job_Set != pc_Request->WhoAmI()) {
						pc_Request->pc_JobManager->notify( pc_Request, JSVE_JobServiceResult_NG, JSVE_JobServiceResultDetail_IllegalRequest, JSVE_JobManager_MsgID_MemoryAllocEnd, 0 );
						break;
					}
				//Str【 静的解析対応 : Sky花岡 : 2012.09.03 : CID-439589】.
				if( NULL == pc_Request->GetParentRequest() ){
					MyTraceSD(JSVD_DBG_ERROR, "JSVC_RequestJobSetting::startAction02 GetParentRequest ERR" , (int)0 );
					break;
				}
				//End【 静的解析対応 : Sky花岡 : 2012.09.03 : CID-439589】.
					// JobSpecデータクラスを作成.
					pc_Request->GetParentRequest()->newJobSpec();
					JSVC_JobSpec* pc_JobSpec = pc_Request->GetParentRequest()->getJobSpec();
					LIBC_List<APIC_Job> c_MyList;
					Bool b_JobSpecDelFlg = True;
					if (False == ((JSVC_RequestJobSetting*)pc_Request)->getJobSpec_byBinaryFile(((JSVC_RequestJobSetting*)pc_Request)->getJobSpecFilePath(), pc_JobSpec)){
						// JobSpecの生成失敗.
						MyTraceSS(JSVD_DBG_ERROR, "JobSpecの展開","失敗");
						pc_Request->pc_JobManager->notify( pc_Request, JSVE_JobServiceResult_NG, JSVE_JobServiceResultDetail_InputErr, JSVE_JobManager_MsgID_MemoryAllocEnd, 0 );
					}else{
						APIC_Job* pc_Job = pc_Request->getJobInstance();
						if (NULL == pc_Job) {
							MyTraceSS(JSVD_DBG_INFO, "getJobInstance NG","");
						} else {
							if(( NULL != pc_JobSpec )
							&& (True == pc_JobSpec->bReadJobData) ){
								pc_Job->setTypcJobData(&(pc_JobSpec->cJobData));
							}
						}
						// JobSpecの生成成功.
						// ・JOBが持っているJobSpecにシーケンスIDが設定済みの場合、すでにジョブが動作している.
						// ・受信系の場合は上位がJobSpecにシーケンスID（受信データID）をセットしてくる.
						Bool b_GetJob = False;
//Str【 静的解析対応 : Sky伊藤(勝) : 2012.12.10 : CID-441051 】.
						if( ( NULL != pc_JobSpec ) 
						 && (    ( pc_JobSpec->getJobSpec()->pc_Common->getSequenceId()   != 0 )
						      || ( pc_JobSpec->getJobSpec()->pc_Common->getImageLogSend() != TYPD_ImageLogSendJobNon ) ) ){				// Archi(Step3) 送信機能カスタマイズ (条件追加).
//End【 静的解析対応 : Sky伊藤(勝) : 2012.12.10 : CID-441051 】.
							MyTraceSS(JSVD_DBG_INFO, "主に受信系の動作","");
							if (False == ((JSVC_RequestJobSetting*)pc_Request)->getJobInstance_byBinaryFile(pc_Request, ((JSVC_RequestJobSetting*)pc_Request)->getJobSpecFilePath(), pc_JobSpec)){
								// シナリオ/Jobの関連付け失敗.
								pc_Request->pc_JobManager->notify( pc_Request, JSVE_JobServiceResult_NG, JSVE_JobServiceResultDetail_IdNotMatch, JSVE_JobManager_MsgID_MemoryAllocEnd, 0 );
							}else{
								// JobIDのFixをオブザーバー通知する.
								JSVC_JobManager::newInstance()->SubScenarioFixedJobID(((JSVC_RequestJobSetting*)pc_Request)->GetParentRequest());
								// サブシナリオIDを設定.
								// ログIDを設定.
								b_GetJob = True;		// Archi(Step3) 送信機能カスタマイズ.
							}
							// JSVC_JobSpecインスタンスは開放しない.
							// サブシナリオの実行要求時にJobSpecの上書きを行う.
							b_JobSpecDelFlg = False;
						}else{
							MyTraceSS(JSVD_DBG_INFO, "コピー系/復活系の動作","");
							if (False == ((JSVC_RequestJobSetting*)pc_Request)->newJobInstance_byBinaryFile(pc_Request, ((JSVC_RequestJobSetting*)pc_Request)->getJobSpecFilePath())) {
								MyTraceSS(JSVD_DBG_ERROR, "ジョブポインタ取得","失敗");
								pc_Request->pc_JobManager->notify( pc_Request, JSVE_JobServiceResult_NG, IPCE_JobsResultDetail_InputErr, JSVE_JobManager_MsgID_MemoryAllocEnd, 0 );
							} else {
								// サブシナリオIDを設定.
								pc_Request->getJobInstance()->fixJobID(pc_Request->ul_SubScenarioID);
								// シナリオIDを設定.
								pc_Request->getJobInstance()->fixScenarioID(pc_Request->ul_ScenarioID);
								// 受信の場合はreq_startのトリガを取れないので、この時点で一旦Job制御フラグを変更しておく.
								pc_Request->getJobInstance()->getJobSpec()->pc_Common->setScenarioExecuteFlg(True);		//シナリオフラグをTrueへ(JobSからは常にTrue).
								pc_Request->getJobInstance()->getJobSpec()->pc_Common->setInitProcessFlg(False);		//InitプロセスフラグをFalseへ.
								pc_Request->getJobInstance()->getJobSpec()->pc_Common->setEndProcessFlg(False);			//EndプロセスフラグをFalseへ.
								b_GetJob = True;		// Archi(Step3) 送信機能カスタマイズ.
							}
						}
						if(pc_Request->getJobInstance() == NULL) {
							MyTraceSD(JSVD_DBG_INFO, "JSVC_Request::startAction02 pc_Request->getJobInstance NULL", (int)pc_Request->ul_SubScenarioID);	// warning対応 2013/09/18 ooba.
							pc_Request->GetParentRequest()->delJobSpec();
							break;
						}
						TYPE_JobType e_JobType = 0;
// ※先ほど消した処理はリスナー判定としては必要なため、先に呼ばれた結果リスナー On である場合はイレギュラーとして NG を返す
						JSVC_IsListnerType		ul_RetIsList;
						ul_RetIsList = JSVC_JobManager::newInstance()->isListenerFunc(pc_Request->getJobInstance()->getJobSpec(), pc_Request->ul_SubScenarioID, &e_JobType) ;
						if (JSVC_IsListner_Listner == ul_RetIsList) {
							MyTraceSS(JSVD_DBG_ERROR, "JSVC_Request::startAction02 受信系なのにRcvDataIdが０" , "" );
							// JobManagerへ要求NGを通知する.
							e_Request_Result = JSVE_Request_Result_Err;
							pc_Request->setResult(JSVE_JobServiceResult_NG);
							pc_Request->setResultDetail(IPCE_JobsResultDetail_CBNumOver);
							pc_Request->pc_JobManager->notify( pc_Request, e_Request_Result, JSVE_JobServiceResultDetail_XmlAnalyze, JSVE_JobManager_MsgID_MemoryAllocEnd, 0 );
						} else if (JSVC_IsListner_RetryListner == ul_RetIsList) {
							// DocSearcher起動ジョブでInstance取得に失敗した場合、deleteInstance()をトリガーに.
							// リトライ処理を行うため、擬似的にdeleteInstance()を行う.
							MyTraceSS(JSVD_DBG_INFO, "JSVC_Request::startAction02 ","DocSercherへのリトライを促す");
							e_JobType = TYPE_JobTypeFaxSendMemory;
							APIC_JobInstanceList::jobRegistPermissionDocSearcher(e_JobType);
						} else {
							if( b_GetJob == True )
							{

								uint32_t ul_index;
								ul_index = JSVC_JobManager::newInstance()->availableAnsFunctionIndex(JSVC_CB_INDEX_ans_CallbackExpandChargeMap, JSVC_CB_MAX_ans_CallbackExpandChargeMap, pc_Request );
								if( ul_index != (uint32_t)-1 )		// warning対応 2013/09/21 ooba.
								{
									// JobManagerへ要求OKを通知する.
									// 課金MAP展開要求.
									APIC_Job* pJob = pc_Request->getJobInstance();
									//Str【 静的解析対応 : Sky花岡 : 2012.09.03 : CID-439590】.
									if( NULL == pJob ){
										MyTraceSD(JSVD_DBG_ERROR, "JSVC_RequestJobSetting::startAction02 getJobInstance ERR" , (int)0 );
										break;
									}
									//End【 静的解析対応 : Sky花岡 : 2012.09.03 : CID-439590】.
									TYPC_JobSpec *pJobSpec = pJob->getJobSpec();
		
									APIC_ChargingIF* pCharge = pJob->getAppAccount();
		
									if( NULL != pCharge )	//ここまでの処理でpJob、pJobSpecは取得できているのでpChargeだけ.
									{
										// 課金対象ジョブの登録.
										pCharge->setFeeCollectionJob( True );
										pCharge->reqExpandChargeMap( (void (*)( uint32_t ul_SubScenarioId, SYSE_ChgIFResultStatus e_Result ))(JSVC_JobManager::newInstance()->c_CBManage[ul_index].vp_Func), 
																	pJob->getJobStatus()->getSubScenarioID(),
																	pJobSpec->pc_Common->getChargeMapID(),
																	pJobSpec->pc_Common->getChargeKindID());
										//課金MAP展開待ちへ遷移.
										pc_Request->e_Request_Status = JSVE_Request_Status_ChargeMapOpenWait;
										// 結果設定.
										e_Request_Result = JSVE_Request_Result_Ok;
									} else {
										// JobManagerへ要求NGを通知する.
										pc_Request->pc_JobManager->notify( pc_Request, e_Request_Result, IPCE_JobsResultDetail_ChargeGetErr, JSVE_JobManager_MsgID_MemoryAllocEnd, 0 );
									}
								}
								else
								{
									MyTraceSD(JSVD_DBG_ERROR, "JSVC_Request::startAction02 AnsAnalyzeErr. e_Request_Event:" , ps_NotifyInfo->e_Request_Event );
									// JobManagerへ要求NGを通知する.
									e_Request_Result = JSVE_Request_Result_Err;
									pc_Request->setResult(JSVE_JobServiceResult_NG);
									pc_Request->setResultDetail(IPCE_JobsResultDetail_CBNumOver);
									pc_Request->pc_JobManager->notify( pc_Request, e_Request_Result, JSVE_JobServiceResultDetail_XmlAnalyze, JSVE_JobManager_MsgID_MemoryAllocEnd, 0 );
								}
	
							}else{
								MyTraceSS(JSVD_DBG_ERROR, "startAction02","ジョブ特定失敗、メモリ確保処理を中断");
							}
						}
					}
					// JSVC_JobSpecインスタンスの開放.
					if (True == b_JobSpecDelFlg){
						pc_Request->GetParentRequest()->delJobSpec();
					}
			}else{
					pc_Request->pc_JobManager->notify( pc_Request, e_Request_Result, JSVE_JobServiceResultDetail_XmlAnalyze, JSVE_JobManager_MsgID_MemoryAllocEnd, 0 );
				}
			}
			break;
		case JSVE_Request_Event_AnsMemAllocEnd	:	///< メモリ確保応答.
			{
			uint32_t ul_id = pc_Request->getSubScenarioID();
			TYPE_JobType e_JobType;
			APIC_Job* pJob = pc_Request->getJobInstance();
// 364759対応 Sky岩崎 2012.05.09 Start.
//			JSVC_IsListnerType ul_RetIsList = JSVC_IsListner_NG;
			//Str【 静的解析対応 : Sky花岡 : 2012.09.03 : CID-439591】.
			if( NULL == (JSVC_JobManager::newInstance()->getParentRequest(pc_Request->getSubScenarioID())) ){
				MyTraceSD(JSVD_DBG_ERROR, "JSVC_RequestJobSetting::startAction02 getParentRequest ERR" , (int)0 );
				break;
			}
			//End【 静的解析対応 : Sky花岡 : 2012.09.03 : CID-439591】.
			JSVC_IsListnerType ul_RetIsList = (JSVC_JobManager::newInstance()->getParentRequest(pc_Request->getSubScenarioID()))->getListenerType();
// 364759対応 Sky岩崎 2012.05.09 End.
			if( pJob != NULL ){
				ul_RetIsList = JSVC_JobManager::newInstance()->isListenerFunc(pJob->getJobSpec(), ul_id, &e_JobType);
			}
// 364759対応 Sky岩崎 2012.05.09 Start.
//			if(( pJob != NULL )
//			&& ( ul_RetIsList == JSVC_IsListner_RetryListner )){
			if ( ul_RetIsList == JSVC_IsListner_RetryListner ){
// 364759対応 Sky岩崎 2012.05.09 End.
				if(( JSVC_JobManager::newInstance()->getDisableJobId() != 0 )
				&& ( JSVC_JobManager::newInstance()->getDisableJobId() == pc_Request->getSubScenarioID() )){
					//実行要求を受けたサブシナリオが完了済と判断されている場合.
					MyTraceSD(JSVD_DBG_INFO, "DocSercherによって既に完了済と判断されているジョブを終わらせる SubSce=", (int)pc_Request->getSubScenarioID());	// warning対応 2013/09/18 ooba.
					JSVC_JobManager::newInstance()->setDisableJobId(0);
					pc_Request->notifyJobService_Answer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Err, IPCE_JobsResultDetail_Completed, pc_Request);
// CQ:366448対応 Sky_iwasaki 2012.06.15 Start.
//					if( pJob != NULL ){
//						pJob->deleteInstance();
//					}
// CQ:366448対応 Sky_iwasaki 2012.06.15 End.
				}
// 364759対応 Sky岩崎 2012.05.09 Start.
				else if (NULL == pJob){
					MyTraceSD(JSVD_DBG_INFO, "DocSercherによって既に完了済と判断されているジョブを終わらせる2 SubSce=", (int)pc_Request->getSubScenarioID());	// warning対応 2013/09/18 ooba.
					pc_Request->notifyJobService_Answer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Err, IPCE_JobsResultDetail_Completed, pc_Request);
				}
// 364759対応 Sky岩崎 2012.05.09 End.
				else{
					pc_Request->notifyJobService_Answer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Ok, JSVE_JobServiceResultDetail_Non, pc_Request);
					// 状態更新(サブシナリオ開始待ち).
					pc_Request->e_Request_Status = JSVE_Request_Status_WaitStart;
				}
			}else
				if( JSVE_Request_Result_Ok == ps_NotifyInfo->pReq->getResult() ){
					pc_Request->notifyJobService_Answer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Ok, JSVE_JobServiceResultDetail_Non, pc_Request);
					// 状態更新(サブシナリオ開始待ち).
					pc_Request->e_Request_Status = JSVE_Request_Status_WaitStart;
				}
				else{
					// JobManagerへ要求NGを通知する.
					pc_Request->notifyJobService_Answer(ps_NotifyInfo->e_Request_Event, e_Request_Result, JSVE_JobServiceResultDetail_AnalyzeErr, pc_Request);
					// 状態更新(エラー停止).
					pc_Request->e_Request_Status = JSVE_Request_Status_ErrStoped;
					// ジョブインスタンス削除を行う.
// CQ:366448対応 Sky_iwasaki 2012.06.15 Start.
//					if( pc_Request->getJobInstance() ){		//NG理由が[ジョブポインタ取得失敗]の場合のケア.
//						pc_Request->getJobInstance()->deleteInstance();
//					}
// CQ:366448対応 Sky_iwasaki 2012.06.15 End.
				}
			}
			break;
		case JSVE_Request_Event_ReqStart		:	///< サブシナリオ開始要求.
			// JobManagerへ要求NGを通知する.
			e_Request_Result = JSVE_Request_Result_Ok;
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Err, JSVE_JobServiceResultDetail_NotMemAlloc, pc_Request);
			break;
		case JSVE_Request_Event_ReqStop			:	///< サブシナリオ停止要求.
			// JobManagerへ要求NGを通知する.
			e_Request_Result = JSVE_Request_Result_Ok;
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Err, JSVE_JobServiceResultDetail_NotMemAlloc, pc_Request);
			break;
		case JSVE_Request_Event_AnsStopEnd		:	///< サブシナリオ停止応答.
			break;
		case JSVE_Request_Event_ReqUpdate		:	///< サブシナリオ設定変更要求.
			// JobManagerへ要求NGを通知する.
			e_Request_Result = JSVE_Request_Result_Ok;
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Err, JSVE_JobServiceResultDetail_NotMemAlloc, pc_Request);
			break;
		case JSVE_Request_Event_AnsUpdateEnd	:	///< サブシナリオ設定変更応答.
			break;
		case JSVE_Request_Event_ReqRestart		:	///< サブシナリオ再開要求.
			// JobManagerへ要求NGを通知する.
			e_Request_Result = JSVE_Request_Result_Ok;
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Err, JSVE_JobServiceResultDetail_NotMemAlloc, pc_Request);
			break;
		case JSVE_Request_Event_AnsRestartEnd	:	///< サブシナリオ再開応答.
			break;
		case JSVE_Request_Event_ReqCancel		:	///< サブシナリオ削除要求.
			e_XmlResultStatus = pc_Request->pc_XmlAnalyze->req_CancelXmlAnalyze(pc_Request->ul_SubScenarioID);	// XML解析のキャンセル要求を行う.
			if (TYPE_XmlResultStatus_ReqOK == e_XmlResultStatus){																		// 受付完了.
				// JobManagerへ要求OKを通知する.
				pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Ok, JSVE_JobServiceResultDetail_Non, pc_Request);
				// 状態更新(XML解析削除要求中).
				pc_Request->e_Request_Status = JSVE_Request_Status_AnalyzeCancel;
				// 結果設定.
				e_Request_Result = JSVE_Request_Result_Ok;
			}else{
				// JobManagerへ要求NGを通知する.
				pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, e_Request_Result, JSVE_JobServiceResultDetail_NotMemAlloc, pc_Request);
			}
			break;
		case JSVE_Request_Event_AnsCancel		:	///< サブシナリオ削除応答.
			break;
		case JSVE_Request_Event_AnsEnd			:	///< サブシナリオ終了通知.
			break;
		case JSVE_Request_Event_ReqMemFree		:	///< メモリ開放要求.
			// JobManagerへ要求NGを通知する.
			e_Request_Result = JSVE_Request_Result_Ok;
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Err, JSVE_JobServiceResultDetail_NotMemAlloc, pc_Request);
			break;
		case JSVE_Request_Event_AnsMemFree		:	///< メモリ開放応答.
			break;
		case JSVE_RequestAnswer_Event_ChargeMapPrepared			:	///< 課金マップ展開完了.
			break;
		case JSVE_RequestAnswer_Event_NewInstance				:	///< 待ちうけ部からの着信通知.
			break;
		case JSVE_RequestAnswer_Event_addJob					:	///< ActiveJobListからの状態変化通知(ジョブの追加).
			break;
		case JSVE_RequestAnswer_Event_DocSearcher				:	///< DocSearcherからの状態変化通知.
			break;
		case JSVE_Request_Event_Max				:	///< 最大値(23).
			#ifdef _DEBUG_JSVC_SEQ
			MyTraceSD(JSVD_DBG_ERROR, "最大値:処理なし。startAction02" , 0 );
			#endif
			break;
		default:
			#ifdef _DEBUG_JSVC_SEQ
			MyTraceSD(JSVD_DBG_ERROR, "default。startAction02" , ps_NotifyInfo->e_Request_Event );
			#endif
			break;
	}

	return e_Request_Result;
}
// 情報通知から状態に応じた処理を実行する.
///< XML解析削除要求中.
JSVE_Request_Result JSVC_Request::startAction03( JSVS_NotifyInfo*	ps_NotifyInfo, JSVC_Request* pc_Request )
{
	JSVE_Request_Result e_Request_Result = JSVE_Request_Result_Err;

	TYPE_XmlResultStatus e_XmlResultStatus = TYPE_XmlResultStatus_ReqOK;

	switch( ps_NotifyInfo->e_Request_Event )
	{
		case JSVE_Request_Event_Non				:	///< 初期値.
			break;
		case JSVE_Request_Event_ReqMemAlloc		:	///< メモリ確保要求.
			// JobManagerへ要求NGを通知する.
			e_Request_Result = JSVE_Request_Result_Ok;
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Err, JSVE_JobServiceResultDetail_XmlAnalyze, pc_Request);
			break;
		case JSVE_Request_Event_AnsAnalyzeCancel:	///< XML解析削除応答.
			// JobManagerへ要求OKを通知する.
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Ok, JSVE_JobServiceResultDetail_UserCancel, pc_Request);
			// 状態更新(XML解析終了中).
			pc_Request->e_Request_Status = JSVE_Request_Status_AnalyzeEnd;
			// 結果設定.
			e_Request_Result = JSVE_Request_Result_Ok;
			break;
		case JSVE_Request_Event_AnsAnalyzeEnd	:	///< XML解析完了応答.
			// メモリ確保NGを通知する.
			pc_Request->pc_JobManager->notify( pc_Request, JSVE_Request_Result_Err, ps_NotifyInfo->pReq->getResultDetail(), JSVE_JobManager_MsgID_MemoryAllocEnd, 0 );
			// ステータス変更なし.
			e_Request_Result = JSVE_Request_Result_Ok;

			break;
		case JSVE_Request_Event_AnsMemAllocEnd	:	///< メモリ確保応答.
			if( JSVE_Request_Result_Ok == ps_NotifyInfo->pReq->getResult() ){
				pc_Request->notifyJobService_Answer( ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Ok, JSVE_JobServiceResultDetail_Non, pc_Request );
				pc_Request->e_Request_Status = JSVE_Request_Status_AnalyzeEnd;
			}
			else{
				pc_Request->notifyJobService_Answer( ps_NotifyInfo->e_Request_Event, e_Request_Result, (JSVE_JobServiceResultDetail)ps_NotifyInfo->pReq->getResultDetail(), pc_Request );
				pc_Request->e_Request_Status = JSVE_Request_Status_ErrStoped;
				// 削除要求が発生しているので、削除応答のために通知を投げておく.
				pc_Request->pc_JobManager->notify( pc_Request, JSVE_Request_Result_Ok, JSVE_JobServiceResultDetail_Non, JSVE_JobManager_MsgID_CancelEnd, 0 );
			}
			e_Request_Result = JSVE_Request_Result_Ok;
			break;
		case JSVE_Request_Event_ReqStart		:	///< サブシナリオ開始要求.
			// JobManagerへ要求NGを通知する.
			e_Request_Result = JSVE_Request_Result_Ok;
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Err, JSVE_JobServiceResultDetail_NotMemAlloc, pc_Request);
			break;
		case JSVE_Request_Event_ReqStop			:	///< サブシナリオ停止要求.
			// JobManagerへ要求NGを通知する.
			e_Request_Result = JSVE_Request_Result_Ok;
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Err, JSVE_JobServiceResultDetail_NotMemAlloc, pc_Request);
			break;
		case JSVE_Request_Event_AnsStopEnd		:	///< サブシナリオ停止応答.
			break;
		case JSVE_Request_Event_ReqUpdate		:	///< サブシナリオ設定変更要求.
			// JobManagerへ要求NGを通知する.
			e_Request_Result = JSVE_Request_Result_Ok;
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Err, JSVE_JobServiceResultDetail_NotMemAlloc, pc_Request);
			break;
		case JSVE_Request_Event_AnsUpdateEnd	:	///< サブシナリオ設定変更応答.
			break;
		case JSVE_Request_Event_ReqRestart		:	///< サブシナリオ再開要求.
			// JobManagerへ要求NGを通知する.
			e_Request_Result = JSVE_Request_Result_Ok;
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Err, JSVE_JobServiceResultDetail_NotMemAlloc, pc_Request);
			break;
		case JSVE_Request_Event_AnsRestartEnd	:	///< サブシナリオ再開応答.
			break;
		case JSVE_Request_Event_ReqCancel		:	///< サブシナリオ削除要求.
			e_XmlResultStatus = pc_Request->pc_XmlAnalyze->req_CancelXmlAnalyze(pc_Request->ul_SubScenarioID);	// XML解析のキャンセル要求を行う.
			if (TYPE_XmlResultStatus_ReqOK == e_XmlResultStatus){																		// 受付完了.
				// JobManagerへ要求OKを通知する.
				pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Ok, JSVE_JobServiceResultDetail_Non, pc_Request);
				// 状態更新(XML解析削除要求中).
				pc_Request->e_Request_Status = JSVE_Request_Status_AnalyzeCancel;
				// 結果設定.
				e_Request_Result = JSVE_Request_Result_Ok;
			}else{
				// JobManagerへ要求NGを通知する.
				pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, e_Request_Result, JSVE_JobServiceResultDetail_NotMemAlloc, pc_Request);
			}
			break;
		case JSVE_Request_Event_AnsCancel		:	///< サブシナリオ削除応答.
			pc_Request->notifyJobService_Answer( ps_NotifyInfo->e_Request_Event, (JSVE_Request_Result)ps_NotifyInfo->pReq->getResult(), (JSVE_JobServiceResultDetail)ps_NotifyInfo->pReq->getResultDetail(), pc_Request );
			pc_Request->e_Request_Status = JSVE_Request_Status_End;
			break;
		case JSVE_Request_Event_AnsEnd			:	///< サブシナリオ終了通知.
			break;
		case JSVE_Request_Event_ReqMemFree		:	///< メモリ開放要求.
			// JobManagerへ要求NGを通知する.
			e_Request_Result = JSVE_Request_Result_Ok;
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Err, JSVE_JobServiceResultDetail_NotMemAlloc, pc_Request);
			break;
		case JSVE_Request_Event_AnsMemFree		:	///< メモリ開放応答.
			break;
		case JSVE_RequestAnswer_Event_ChargeMapPrepared			:	///< 課金マップ展開完了.
			break;
		case JSVE_RequestAnswer_Event_NewInstance				:	///< 待ちうけ部からの着信通知.
			break;
		case JSVE_RequestAnswer_Event_addJob					:	///< ActiveJobListからの状態変化通知(ジョブの追加).
			break;
		case JSVE_RequestAnswer_Event_DocSearcher				:	///< DocSearcherからの状態変化通知.
			break;
		case JSVE_Request_Event_Max				:	///< 最大値(23).
			#ifdef _DEBUG_JSVC_SEQ
			MyTraceSD(JSVD_DBG_ERROR, "最大値:処理なし。startAction03" , 0 );
			#endif
			break;
		default:
			#ifdef _DEBUG_JSVC_SEQ
			MyTraceSD(JSVD_DBG_ERROR, "default。startAction03" , ps_NotifyInfo->e_Request_Event );
			#endif
			break;
	}

	return e_Request_Result;
}
// 情報通知から状態に応じた処理を実行する.
///< XML解析終了中.
JSVE_Request_Result JSVC_Request::startAction04( JSVS_NotifyInfo*	ps_NotifyInfo, JSVC_Request* pc_Request )
{
	JSVE_Request_Result e_Request_Result = JSVE_Request_Result_Err;

	switch( ps_NotifyInfo->e_Request_Event )
	{
		case JSVE_Request_Event_Non				:	///< 初期値.
			break;
		case JSVE_Request_Event_ReqMemAlloc		:	///< メモリ確保要求.
			// JobManagerへ要求NGを通知する.
			e_Request_Result = JSVE_Request_Result_Ok;
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Err, JSVE_JobServiceResultDetail_XmlAnalyze, pc_Request);
			break;
		case JSVE_Request_Event_AnsAnalyzeCancel:	///< XML解析削除応答.
			break;
		case JSVE_Request_Event_AnsAnalyzeEnd	:	///< XML解析完了応答.
			break;
		case JSVE_Request_Event_AnsMemAllocEnd	:	///< メモリ確保応答.
			// 正常終了の場合、アプリケーションプラットフォームへ応答をOKで通知する.
			// 異常終了の場合、アプリケーションプラットフォームへ応答をNGで通知する.
			if (JSVE_Request_Result_Ok == ps_NotifyInfo->pReq->getResult()){
				if( (pc_Request->ul_DetailParam & JSVD_DetailParamMask_XmlAnalyze) == TYPE_XmlResultStatus_SkipEnd ){
					pc_Request->notifyJobService_Answer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Ok, JSVE_JobServiceResultDetail_XmlSkipEnd, pc_Request);
				}
				else{
					pc_Request->notifyJobService_Answer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Ok, JSVE_JobServiceResultDetail_Non, pc_Request);
				}
				// 状態更新(サブシナリオ開始待ち).
				pc_Request->e_Request_Status = JSVE_Request_Status_WaitStart;
			}else{
				uint32_t ul_index;
				// ジョブキャンセル指示を行う.
				ul_index = JSVC_JobManager::newInstance()->availableAnsFunctionIndex( JSVC_CB_INDEX_ans_cancel, JSVC_CB_MAX_ans_cancel, pc_Request );
				if( ul_index != (uint32_t)-1 )		// warning対応 2013/09/21 ooba.
				{
					pc_Request->notifyJobService_Answer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Err, pc_Request->getResultDetail(), pc_Request);
					
					//Str【 静的解析対応 : Sky花岡 : 2012.09.03 : CID-439592】.
					if( NULL == pc_Request->getJobInstance() ){
						MyTraceSD(JSVD_DBG_ERROR, "JSVC_RequestJobSetting::startAction04 getJobInstance ERR" , (int)0 );
						break;
					}
					//End【 静的解析対応 : Sky花岡 : 2012.09.03 : CID-439592】.
// CQ504179対応		2014/10/8	1OPS JOB H.Tamura	Start.
					TYPE_req_start_Result e_req_start_Result = pc_Request->getJobInstance()->get_req_start_result();

// 2015/1/14 CQ:508862 Nagasawa mod sta.
					if( TYPE_req_start_Result_NG_JobReserve                  == e_req_start_Result  ||
						TYPE_req_start_Result_NG_JobLogFull                  == e_req_start_Result  ||
						TYPE_req_start_Result_NG_ImageLogFull                == e_req_start_Result  ||
						TYPE_req_start_Result_NG_ReportJobInit               == e_req_start_Result  ||
						TYPE_req_start_Result_NG_IsAvailable                 == e_req_start_Result  ||
						TYPE_req_start_Result_NG_IsCheckStartJob             == e_req_start_Result  ||
						TYPE_req_start_Result_NG_JobPlanner                  == e_req_start_Result  ||
						TYPE_req_start_Result_NG_IsEnabledJobStart_AddAction == e_req_start_Result  ||// 2016/05/10 CQ527913 H.nakano add.
						TYPE_req_start_Result_NG_makePSP_Error               == e_req_start_Result    // makePSPでエラー時に管理者ログインできない問題対応(静的解析対応として対応) Start 2016/10/12 Soma.
					  )
					{
						//Log.
						MyTraceSD(JSVD_DBG_ERROR, "JSVC_RequestJobSetting::startAction04 getJobInstance ans_start(NULL)" , (int)0 );
						// ジョブ破棄.
						pc_Request->getJobInstance()->deleteInstance();
					} else if (TYPE_req_start_Result_NG_UsedJob  == e_req_start_Result) {
						// すでに同じ ID のジョブ投入済みだった場合、deleteInstance/req_cancel しない.
						SYSF_logMsg("[JSVC_Request][startAction04] req_start_Result_NG_UsedJob\n",1,2,3,4,5,6);
					}
// 2015/1/14 CQ:508862 Nagasawa mod end.
					else
					{
						// ジョブキャンセル指示を行う.
						pc_Request->getJobInstance()->req_cancel((void (*)( APIC_Job* ))(JSVC_JobManager::newInstance()->c_CBManage[ul_index].vp_Func));
						if( pc_Request->getCancelFlg() == True ){
							// 明示的にCancelEnd通知を発行する.
							pc_Request->pc_JobManager->notify( pc_Request, JSVE_Request_Result_Ok, JSVE_JobServiceResultDetail_UserCancel, JSVE_JobManager_MsgID_CancelEnd, 0 );
						}
					}
// CQ504179対応		2014/10/8	1OPS JOB H.Tamura	End.
				}
				else
				{
					MyTraceSD(JSVD_DBG_ERROR, "JSVC_Request::startAction04 CancelErr. e_Request_Event:" , ps_NotifyInfo->e_Request_Event );
					e_Request_Result = JSVE_Request_Result_Err;
					pc_Request->setResult(JSVE_JobServiceResult_NG);
					//エラーの上書きはしない.
					if( pc_Request->getResultDetail() == JSVE_JobServiceResultDetail_Non ){
						pc_Request->setResultDetail(IPCE_JobsResultDetail_CBNumOver);
					}
					// JobManagerへ要求NGを通知する.
					pc_Request->pc_JobManager->notify( pc_Request, e_Request_Result, pc_Request->getResultDetail(), JSVE_JobManager_MsgID_MemoryAllocEnd, 0 );
				}
			}
			// 結果設定.
			e_Request_Result = JSVE_Request_Result_Ok;
			break;
		case JSVE_Request_Event_ReqStart		:	///< サブシナリオ開始要求.
			// JobManagerへ要求NGを通知する.
			e_Request_Result = JSVE_Request_Result_Ok;
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Err, JSVE_JobServiceResultDetail_NotMemAlloc, pc_Request);
			break;
		case JSVE_Request_Event_ReqStop			:	///< サブシナリオ停止要求.
			// JobManagerへ要求NGを通知する.
			e_Request_Result = JSVE_Request_Result_Ok;
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Err, JSVE_JobServiceResultDetail_NotMemAlloc, pc_Request);
			break;
		case JSVE_Request_Event_AnsStopEnd		:	///< サブシナリオ停止応答.
			break;
		case JSVE_Request_Event_ReqUpdate		:	///< サブシナリオ設定変更要求.
			// JobManagerへ要求NGを通知する.
			e_Request_Result = JSVE_Request_Result_Ok;
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Err, JSVE_JobServiceResultDetail_NotMemAlloc, pc_Request);
			break;
		case JSVE_Request_Event_AnsUpdateEnd	:	///< サブシナリオ設定変更応答.
			break;
		case JSVE_Request_Event_ReqRestart		:	///< サブシナリオ再開要求.
			// JobManagerへ要求NGを通知する.
			e_Request_Result = JSVE_Request_Result_Ok;
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Err, JSVE_JobServiceResultDetail_NotMemAlloc, pc_Request);
			break;
		case JSVE_Request_Event_AnsRestartEnd	:	///< サブシナリオ再開応答.
			break;
		case JSVE_Request_Event_ReqCancel		:	///< サブシナリオ削除要求.
			uint32_t ul_index;
			// ジョブキャンセル指示を行う.
			ul_index = JSVC_JobManager::newInstance()->availableAnsFunctionIndex( JSVC_CB_INDEX_ans_cancel, JSVC_CB_MAX_ans_cancel, pc_Request );
			if( ul_index != (uint32_t)-1 )			// warning対応 2013/09/21 ooba.
			{
				// 2012/08/21 CQ370114対応 start.
				// メモリ確保完了前のCancel要求の場合、Initの停止フラグを落とせない可能性があるため.
				// req_cancelコール前にフラグを落とす.
				pc_Request->getJobInstance()->getJobSpec()->pc_Common->setInitProcessFlg(False);
				// 2012/08/21 CQ370114対応 end.
				// JobManagerへ要求OKを通知する.
				pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Ok, JSVE_JobServiceResultDetail_Non, pc_Request);
				// ジョブキャンセル指示を行う.
				pc_Request->getJobInstance()->req_cancel((void (*)( APIC_Job* ))(JSVC_JobManager::newInstance()->c_CBManage[ul_index].vp_Func));
				// 状態更新(サブシナリオ削除要求中).
				pc_Request->e_Request_Status = JSVE_Request_Status_ReqCancel;
				// 結果設定.
				e_Request_Result = JSVE_Request_Result_Ok;
				pc_Request->setCancelFlg( True );
			}
			else
			{
				MyTraceSD(JSVD_DBG_ERROR, "JSVC_Request::startAction04 CancelErr. e_Request_Event:" , ps_NotifyInfo->e_Request_Event );
				e_Request_Result = JSVE_Request_Result_Err;
				pc_Request->setResult(JSVE_JobServiceResult_NG);
				pc_Request->setResultDetail(IPCE_JobsResultDetail_CBNumOver);
				// JobManagerへ要求NGを通知する.
				pc_Request->pc_JobManager->notify( pc_Request, e_Request_Result, JSVE_JobServiceResultDetail_XmlAnalyze, JSVE_JobManager_MsgID_MemoryAllocEnd, 0 );
			}
			break;
		case JSVE_Request_Event_AnsCancel		:	///< サブシナリオ削除応答.
			if (JSVE_Request_Result_Ok == ps_NotifyInfo->pReq->getResult()){
				pc_Request->notifyJobService_Answer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Ok, JSVE_JobServiceResultDetail_Non, pc_Request);
			}
			else{
				pc_Request->notifyJobService_Answer(ps_NotifyInfo->e_Request_Event, e_Request_Result, (JSVE_JobServiceResultDetail)ps_NotifyInfo->pReq->getResultDetail(), pc_Request);
			}
			pc_Request->e_Request_Status = JSVE_Request_Status_End;
			e_Request_Result = JSVE_Request_Result_Ok;
			break;
		case JSVE_Request_Event_AnsEnd			:	///< サブシナリオ終了通知.
			break;
		case JSVE_Request_Event_ReqMemFree		:	///< メモリ開放要求.
			// JobManagerへ要求NGを通知する.
			e_Request_Result = JSVE_Request_Result_Ok;
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Err, JSVE_JobServiceResultDetail_NotMemAlloc, pc_Request);
			break;
		case JSVE_Request_Event_AnsMemFree		:	///< メモリ開放応答.
			break;
		case JSVE_RequestAnswer_Event_ChargeMapPrepared			:	///< 課金マップ展開完了.
			break;
		case JSVE_RequestAnswer_Event_NewInstance				:	///< 待ちうけ部からの着信通知.
			break;
		case JSVE_RequestAnswer_Event_addJob					:	///< ActiveJobListからの状態変化通知(ジョブの追加).
			break;
		case JSVE_RequestAnswer_Event_DocSearcher				:	///< DocSearcherからの状態変化通知.
			break;
		case JSVE_Request_Event_Max				:	///< 最大値(23).
			#ifdef _DEBUG_JSVC_SEQ
			MyTraceSD(JSVD_DBG_ERROR, "最大値:処理なし。startAction04" , 0 );
			#endif
			break;
		default:
			#ifdef _DEBUG_JSVC_SEQ
			MyTraceSD(JSVD_DBG_ERROR, "default。startAction04" , ps_NotifyInfo->e_Request_Event );
			#endif
			break;
	}


	return e_Request_Result;
}
// 情報通知から状態に応じた処理を実行する.
///< サブシナリオ開始待ち.
JSVE_Request_Result JSVC_Request::startAction05( JSVS_NotifyInfo*	ps_NotifyInfo, JSVC_Request* pc_Request )
{
	JSVE_Request_Result e_Request_Result = JSVE_Request_Result_Err;

	switch( ps_NotifyInfo->e_Request_Event )
	{
		case JSVE_Request_Event_Non				:	///< 初期値.
			break;
		case JSVE_Request_Event_ReqMemAlloc		:	///< メモリ確保要求.
			// JobManagerへ要求NGを通知する.
			e_Request_Result = JSVE_Request_Result_Ok;
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Err, JSVE_JobServiceResultDetail_AllreadyMemAlloced, pc_Request);
			break;
		case JSVE_Request_Event_AnsAnalyzeCancel:	///< XML解析削除応答.
			break;
		case JSVE_Request_Event_AnsAnalyzeEnd	:	///< XML解析完了応答.
			break;
		case JSVE_Request_Event_AnsMemAllocEnd	:	///< メモリ確保応答.
			break;
		case JSVE_Request_Event_ReqStart		:	///< サブシナリオ開始要求.
			// サブシナリオ開始要求を行う.
			//Str【 静的解析対応 : Sky花岡 : 2012.09.03 : CID-439594】.
			if( NULL == pc_Request->getJobInstance() ){
				MyTraceSD(JSVD_DBG_ERROR, "JSVC_RequestJobSetting::startAction05 getJobInstance ERR" , (int)0 );
				break;
			}
			//End【 静的解析対応 : Sky花岡 : 2012.09.03 : CID-439594】.
			if ( 0 != pc_Request->getJobInstance()->getJobSpec()->pc_Common->getSequenceId() ){
				pc_Request->getJobInstance()->getJobSpec()->pc_Common->setInitProcessFlg(False);		//InitプロセスフラグをFalseへ.
				pc_Request->getJobInstance()->getJobSpec()->pc_Common->setEndProcessFlg(False);			//EndプロセスフラグをFalseへ.
				pc_Request->getJobInstance()->restart();
			}else{
				pc_Request->getJobInstance()->getJobSpec()->pc_Common->setScenarioExecuteFlg(True);		//シナリオフラグをTrueへ(JobSからは常にTrue).
				pc_Request->getJobInstance()->getJobSpec()->pc_Common->setInitProcessFlg(False);		//InitプロセスフラグをFalseへ.
				// 2012/10/19 CQ373523対応 modify start.
				//if(pc_Request->getJobInstance()->getJobSpec()->pc_Common->c_JobType.getJobType() == TYPE_JobTypeFaxSendTimer) {
				if ((TYPE_JobTypeFaxSendTimer == pc_Request->getJobInstance()->getJobSpec()->pc_Common->c_JobType.getJobType()) || 
					(TYPE_JobTypeFaxReceiveTimer == pc_Request->getJobInstance()->getJobSpec()->pc_Common->c_JobType.getJobType()))
				// 2012/10/19 CQ373523対応 modify end.
				{
					MyTraceSS(JSVD_DBG_ERROR, "JSVC_Request::startAction05 Not EndProcess", "TimerJob");
					pc_Request->getJobInstance()->getJobSpec()->pc_Common->setEndProcessFlg(False);			//EndプロセスフラグをFalseへ.
				} else {
					pc_Request->getJobInstance()->getJobSpec()->pc_Common->setEndProcessFlg(True);			//EndプロセスフラグをTrueへ.
				}
				pc_Request->getJobInstance()->restart();
			}
			// JobManagerへ要求OKを通知する.
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Ok, JSVE_JobServiceResultDetail_Non, pc_Request);

			// 状態更新(サブシナリオ開始).
			pc_Request->e_Request_Status = JSVE_Request_Status_Exec;
			JSVC_CtTrace(JSVD_DBG_INFO, "親リクエストのsetJobExecStatus初期化",0);
			
			//Str【 静的解析対応 : Sky花岡 : 2012.09.03 : CID-439593】.
			if( NULL == pc_Request->GetParentRequest() ){
				MyTraceSD(JSVD_DBG_ERROR, "JSVC_RequestJobSetting::startAction05 GetParentRequest ERR" , (int)0 );
				break;
			}
			//End【 静的解析対応 : Sky花岡 : 2012.09.03 : CID-439593】.
			pc_Request->GetParentRequest()->setJobExecStatus(JSVC_JOBEXEC_NOT_PAUSE);

			// 結果設定.
			e_Request_Result = JSVE_Request_Result_Ok;

			break;
		case JSVE_Request_Event_ReqStop			:	///< サブシナリオ停止要求.
			// JobManagerへ要求NGを通知する.
			e_Request_Result = JSVE_Request_Result_Ok;
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Err, JSVE_JobServiceResultDetail_NotExec, pc_Request);
			break;
		case JSVE_Request_Event_AnsStopEnd		:	///< サブシナリオ停止応答.
			break;
		case JSVE_Request_Event_ReqUpdate		:	///< サブシナリオ設定変更要求.
			//既に設定変更リクエストを受けていた場合は新しく受け付けずにNGを返す.
			//（先頭から順に処理するので自リクエストが最初にHITすれば実行可能と判断）.
			if( pc_Request != JSVC_JobManager::newInstance()->GetExecutingUpdateRequest( pc_Request->getSubScenarioID() ) )
			{
				//同一サブシナリオへの多重リクエストありなら受付でNG.
			// JobManagerへ要求NGを通知する.
				pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Err, JSVE_JobServiceResultDetail_MultiReq, pc_Request);
				//ステータスを送付済にしておく.
				((JSVC_RequestJobSetting*)pc_Request)->setSendUpdateEnd();
				e_Request_Result = JSVE_Request_Result_Err;
			} else {
				//重複チェックOKなのでUPDATE処理開始.
				
				// JobManagerへ要求OKを通知する.
				// 結果設定.
				e_Request_Result = JSVE_Request_Result_Ok;
				if (False == ((JSVC_RequestJobSetting*)pc_Request)->SubsinarioUpdate_byBinaryFile(pc_Request, ((JSVC_RequestJobSetting*)pc_Request)->getFilePath())) {		// XXXXX
					e_Request_Result = JSVE_Request_Result_Err;
					pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, e_Request_Result, JSVE_JobServiceResultDetail_AnalyzeErr, pc_Request);
				} else {
					// JobManagerへ要求OKを通知する.
					pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, e_Request_Result, JSVE_JobServiceResultDetail_Non, pc_Request);
				}

			}
			break;
		case JSVE_Request_Event_AnsUpdateEnd	:	///< サブシナリオ設定変更応答.
			{
//Str【 静的解析対応 : Sky田村 : 2012.08.30 : CID-436576 】.
//				MyTraceSS(JSVD_DBG_INFO, "APFへ設定変更完了を通知","");
//				//JSVC_SendRecv::MakeDataStreamにあわせて結果をRESからREQへ転記.
//				if (( NULL != ps_NotifyInfo->pReq )
//				&&  ( NULL != ps_NotifyInfo->pRes )){
//					ps_NotifyInfo->pReq->setResult( ps_NotifyInfo->pRes->getResult() );
//					ps_NotifyInfo->pReq->setResultDetail( ps_NotifyInfo->pRes->getResultDetail() );
//				}
//				//結果をAPFへ通知.
//				JSVC_JobSendRecv::newInstance()->sendMessageToAPF( JSVE_JobManager_MsgID_ChangeSettingEnd ,ps_NotifyInfo->pReq , ps_NotifyInfo->pRes );
//				e_Request_Result = JSVE_Request_Result_Complete;
				MyTraceSS(JSVD_DBG_INFO, "APFへ設定変更完了を通知","");
				if (( NULL != ps_NotifyInfo->pReq )
				&&  ( NULL != ps_NotifyInfo->pRes )){
					ps_NotifyInfo->pReq->setResult( ps_NotifyInfo->pRes->getResult() );
					ps_NotifyInfo->pReq->setResultDetail( ps_NotifyInfo->pRes->getResultDetail() );
					//結果をAPFへ通知.
					JSVC_JobSendRecv::newInstance()->sendMessageToAPF( JSVE_JobManager_MsgID_ChangeSettingEnd ,ps_NotifyInfo->pReq , ps_NotifyInfo->pRes );
					e_Request_Result = JSVE_Request_Result_Complete;
				}
				else
				{
					MyTraceSS(JSVD_DBG_INFO, "ps_NotifyInfo->pReq or ps_NotifyInfo->pRes is NULL","");
					e_Request_Result = JSVE_Request_Result_Err;
				}
//End【 静的解析対応 : Sky田村 : 2012.08.30 : CID-436576 】.
			}
			break;
		case JSVE_Request_Event_ReqRestart		:	///< サブシナリオ再開要求.
			// JobManagerへ要求NGを通知する.
			e_Request_Result = JSVE_Request_Result_Ok;
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Err, JSVE_JobServiceResultDetail_NotExec, pc_Request);
			break;
		case JSVE_Request_Event_AnsRestartEnd	:	///< サブシナリオ再開応答.
			break;
		case JSVE_Request_Event_ReqCancel		:	///< サブシナリオ削除要求.
			uint32_t ul_index;
			// ジョブキャンセル指示を行う.
			ul_index = JSVC_JobManager::newInstance()->availableAnsFunctionIndex( JSVC_CB_INDEX_ans_cancel, JSVC_CB_MAX_ans_cancel, pc_Request );
			if( ul_index != (uint32_t)-1 )				// warning対応 2013/09/21 ooba.
			{
				// JobManagerへ要求OKを通知する.
				pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Ok, JSVE_JobServiceResultDetail_Non, pc_Request);
				// ジョブキャンセル指示を行う.
				pc_Request->getJobInstance()->req_cancel((void (*)( APIC_Job* ))(JSVC_JobManager::newInstance()->c_CBManage[ul_index].vp_Func));

				uint32_t ulID = pc_Request->getSubScenarioID();
				JSVC_ParentRequest* pc_PRequest = NULL;
				pc_PRequest = JSVC_JobManager::newInstance()->getParentRequest(ulID);
				//Str【 静的解析対応 : Sky花岡 : 2012.09.03 : CID-439595】.
				if( NULL == pc_PRequest ){
					MyTraceSD(JSVD_DBG_ERROR, "JSVC_RequestJobSetting::startAction05 getParentRequest ERR" , (int)0 );
					break;
				}
				//End【 静的解析対応 : Sky花岡 : 2012.09.03 : CID-439595】.
				pc_PRequest->setCancelFlg( True );

				// 状態更新(サブシナリオ削除要求中).
				pc_Request->e_Request_Status = JSVE_Request_Status_ReqCancel;
				// 結果設定.
				e_Request_Result = JSVE_Request_Result_Ok;
			}
			else
			{
				MyTraceSD(JSVD_DBG_ERROR, "JSVC_Request::startAction05 CancelErr. e_Request_Event:" , ps_NotifyInfo->e_Request_Event );
				e_Request_Result = JSVE_Request_Result_Err;
				pc_Request->setResult(JSVE_JobServiceResult_NG);
				pc_Request->setResultDetail(IPCE_JobsResultDetail_CBNumOver);
				// JobManagerへ要求NGを通知する.
				pc_Request->pc_JobManager->notify( pc_Request, e_Request_Result, JSVE_JobServiceResultDetail_XmlAnalyze, JSVE_JobManager_MsgID_MemoryAllocEnd, 0 );
			}
			break;
		case JSVE_Request_Event_AnsCancel		:	///< サブシナリオ削除応答.
			break;
		case JSVE_Request_Event_AnsEnd			:	///< サブシナリオ終了通知.
			break;
		case JSVE_Request_Event_ReqMemFree		:	///< メモリ開放要求.
			// JobManagerへ要求NGを通知する.
			e_Request_Result = JSVE_Request_Result_Ok;
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Err, JSVE_JobServiceResultDetail_NotExec, pc_Request);
			break;
		case JSVE_Request_Event_AnsMemFree		:	///< メモリ開放応答.
			break;
		case JSVE_RequestAnswer_Event_ChargeMapPrepared			:	///< 課金マップ展開完了.
			break;
		case JSVE_RequestAnswer_Event_NewInstance				:	///< 待ちうけ部からの着信通知.
			break;
		case JSVE_RequestAnswer_Event_addJob					:	///< ActiveJobListからの状態変化通知(ジョブの追加).
			break;
		case JSVE_RequestAnswer_Event_DocSearcher				:	///< DocSearcherからの状態変化通知.
			break;
		case JSVE_Request_Event_Max				:	///< 最大値(23).
			#ifdef _DEBUG_JSVC_SEQ
			MyTraceSD(JSVD_DBG_ERROR, "最大値:処理なし。startAction05" , 0 );
			#endif
			break;
		default:
			#ifdef _DEBUG_JSVC_SEQ
			MyTraceSD(JSVD_DBG_ERROR, "default。startAction05" , ps_NotifyInfo->e_Request_Event );
			#endif
			break;
	}

	return e_Request_Result;
}
// 情報通知から状態に応じた処理を実行する.
///< サブシナリオ開始要求中.
JSVE_Request_Result JSVC_Request::startAction06( JSVS_NotifyInfo*	ps_NotifyInfo, JSVC_Request* pc_Request )
{
	JSVE_Request_Result e_Request_Result = JSVE_Request_Result_Err;

	switch( ps_NotifyInfo->e_Request_Event )
	{
		case JSVE_Request_Event_Non				:	///< 初期値.
			break;
		case JSVE_Request_Event_ReqMemAlloc		:	///< メモリ確保要求.
			// JobManagerへ要求NGを通知する.
			e_Request_Result = JSVE_Request_Result_Ok;
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Err, JSVE_JobServiceResultDetail_AllreadyMemAlloced, pc_Request);
			break;
		case JSVE_Request_Event_AnsAnalyzeCancel:	///< XML解析削除応答.
			break;
		case JSVE_Request_Event_AnsAnalyzeEnd	:	///< XML解析完了応答.
			break;
		case JSVE_Request_Event_AnsMemAllocEnd	:	///< メモリ確保応答.
			break;
		case JSVE_Request_Event_ReqStart		:	///< サブシナリオ開始要求.
			// JobManagerへ要求NGを通知する.
			e_Request_Result = JSVE_Request_Result_Ok;
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Err, JSVE_JobServiceResultDetail_Exec, pc_Request);
			break;
		case JSVE_Request_Event_ReqStop			:	///< サブシナリオ停止要求.
			// JobManagerへ要求NGを通知する.
			e_Request_Result = JSVE_Request_Result_Ok;
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Err, JSVE_JobServiceResultDetail_NotExec, pc_Request);
			break;
		case JSVE_Request_Event_AnsStopEnd		:	///< サブシナリオ停止応答.
			break;
		case JSVE_Request_Event_ReqUpdate		:	///< サブシナリオ設定変更要求.
			//既に設定変更リクエストを受けていた場合は新しく受け付けずにNGを返す.
			//（先頭から順に処理するので自リクエストが最初にHITすれば実行可能と判断）.
			if( pc_Request != JSVC_JobManager::newInstance()->GetExecutingUpdateRequest( pc_Request->getSubScenarioID() ) )
			{
				//同一サブシナリオへの多重リクエストありなら受付でNG.
				// JobManagerへ要求NGを通知する.
				pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Err, JSVE_JobServiceResultDetail_MultiReq, pc_Request);
				//ステータスを送付済にしておく.
				((JSVC_RequestJobSetting*)pc_Request)->setSendUpdateEnd();
				e_Request_Result = JSVE_Request_Result_Err;
			} else {
				//重複チェックOKなのでUPDATE処理開始.
				
				TYPE_XmlResultStatus e_XmlResultStatus = pc_Request->pc_XmlAnalyze->req_StartXmlAnalyze(pc_Request->ul_SubScenarioID, ((JSVC_RequestJobSetting*)pc_Request)->getFilePath(), pc_Request, ans_Analyze);
				if (TYPE_XmlResultStatus_ReqOK == e_XmlResultStatus){																		// 受付完了.
					// JobManagerへ要求OKを通知する.
					pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Ok, JSVE_JobServiceResultDetail_Non, pc_Request);
					// 結果設定.
					e_Request_Result = JSVE_Request_Result_Ok;
				}else{
					// JobManagerへ要求NGを通知する.
					pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, e_Request_Result, JSVE_JobServiceResultDetail_AnalyzeErr, pc_Request);
				}
			}
			break;
		case JSVE_Request_Event_AnsUpdateEnd	:	///< サブシナリオ設定変更応答.
			{
//Str【 静的解析対応 : Sky田村 : 2012.08.30 : CID-436577 】.
//				MyTraceSS(JSVD_DBG_INFO, "APFへ設定変更完了を通知","");
//				//JSVC_SendRecv::MakeDataStreamにあわせて結果をRESからREQへ転記.
//				if (( NULL != ps_NotifyInfo->pReq )
//				&&  ( NULL != ps_NotifyInfo->pRes )){
//					ps_NotifyInfo->pReq->setResult( ps_NotifyInfo->pRes->getResult() );
//					ps_NotifyInfo->pReq->setResultDetail( ps_NotifyInfo->pRes->getResultDetail() );
//				}
//				//結果をAPFへ通知.
//				JSVC_JobSendRecv::newInstance()->sendMessageToAPF( JSVE_JobManager_MsgID_ChangeSettingEnd ,ps_NotifyInfo->pReq , ps_NotifyInfo->pRes );
//				e_Request_Result = JSVE_Request_Result_Complete;
				MyTraceSS(JSVD_DBG_INFO, "APFへ設定変更完了を通知","");
				if (( NULL != ps_NotifyInfo->pReq )
				&&  ( NULL != ps_NotifyInfo->pRes )){
					ps_NotifyInfo->pReq->setResult( ps_NotifyInfo->pRes->getResult() );
					ps_NotifyInfo->pReq->setResultDetail( ps_NotifyInfo->pRes->getResultDetail() );
					//結果をAPFへ通知.
					JSVC_JobSendRecv::newInstance()->sendMessageToAPF( JSVE_JobManager_MsgID_ChangeSettingEnd ,ps_NotifyInfo->pReq , ps_NotifyInfo->pRes );
					e_Request_Result = JSVE_Request_Result_Complete;
				}
				else
				{
					MyTraceSS(JSVD_DBG_INFO, "ps_NotifyInfo->pReq or ps_NotifyInfo->pRes is NULL","");
					e_Request_Result = JSVE_Request_Result_Err;
				}
//End【 静的解析対応 : Sky田村 : 2012.08.30 : CID-436577 】.
			}
			break;
		case JSVE_Request_Event_ReqRestart		:	///< サブシナリオ再開要求.
			// JobManagerへ要求NGを通知する.
			e_Request_Result = JSVE_Request_Result_Ok;
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Err, JSVE_JobServiceResultDetail_NotExec, pc_Request);
			break;
		case JSVE_Request_Event_AnsRestartEnd	:	///< サブシナリオ再開応答.
			break;
		case JSVE_Request_Event_ReqCancel		:	///< サブシナリオ削除要求.
			uint32_t ul_index;
			// ジョブキャンセル指示を行う.
			ul_index = JSVC_JobManager::newInstance()->availableAnsFunctionIndex( JSVC_CB_INDEX_ans_cancel, JSVC_CB_MAX_ans_cancel, pc_Request );
			if( ul_index != (uint32_t)-1 )				// warning対応 2013/09/21 ooba.
			{
				// JobManagerへ要求OKを通知する.
				pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Ok, JSVE_JobServiceResultDetail_Non, pc_Request);
				//Str【 静的解析対応 : Sky花岡 : 2012.09.03 : CID-439596】.
				if( NULL == pc_Request->getJobInstance() ){
					MyTraceSD(JSVD_DBG_ERROR, "JSVC_RequestJobSetting::startAction06 getJobInstance ERR" , (int)0 );
					break;
				}
				//End【 静的解析対応 : Sky花岡 : 2012.09.03 : CID-439596】.
				// ジョブキャンセル指示を行う.
				pc_Request->getJobInstance()->req_cancel((void (*)( APIC_Job* ))(JSVC_JobManager::newInstance()->c_CBManage[ul_index].vp_Func));

				uint32_t ulID = pc_Request->getSubScenarioID();
				JSVC_ParentRequest* pc_PRequest = NULL;
				pc_PRequest = JSVC_JobManager::newInstance()->getParentRequest(ulID);
				//Str【 静的解析対応 : Sky花岡 : 2012.09.03 : CID-439597】.
				if( NULL == pc_PRequest ){
					MyTraceSD(JSVD_DBG_ERROR, "JSVC_RequestJobSetting::startAction06 getParentRequest ERR" , (int)0 );
					break;
				}
				//End【 静的解析対応 : Sky花岡 : 2012.09.03 : CID-439597】.
				pc_PRequest->setCancelFlg( True );

				// 状態更新(サブシナリオ削除要求中).
				pc_Request->e_Request_Status = JSVE_Request_Status_ReqCancel;
				// 結果設定.
				e_Request_Result = JSVE_Request_Result_Ok;
			}
			else
			{
				MyTraceSD(JSVD_DBG_ERROR, "JSVC_Request::startAction06 CancelErr. e_Request_Event:" , ps_NotifyInfo->e_Request_Event );
				e_Request_Result = JSVE_Request_Result_Err;
				pc_Request->setResult(JSVE_JobServiceResult_NG);
				pc_Request->setResultDetail(IPCE_JobsResultDetail_CBNumOver);
				// JobManagerへ要求NGを通知する.
				pc_Request->pc_JobManager->notify( pc_Request, e_Request_Result, JSVE_JobServiceResultDetail_XmlAnalyze, JSVE_JobManager_MsgID_MemoryAllocEnd, 0 );
			}
			break;
		case JSVE_Request_Event_AnsCancel		:	///< サブシナリオ削除応答.
			break;
		case JSVE_Request_Event_AnsEnd			:	///< サブシナリオ終了通知.
			break;
		case JSVE_Request_Event_ReqMemFree		:	///< メモリ開放要求.
			// JobManagerへ要求NGを通知する.
			e_Request_Result = JSVE_Request_Result_Ok;
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Err, JSVE_JobServiceResultDetail_NotMemAlloc, pc_Request);
			break;
		case JSVE_Request_Event_AnsMemFree		:	///< メモリ開放応答.
			break;
		case JSVE_RequestAnswer_Event_ChargeMapPrepared			:	///< 課金マップ展開完了.
			break;
		case JSVE_RequestAnswer_Event_NewInstance				:	///< 待ちうけ部からの着信通知.
			break;
		case JSVE_RequestAnswer_Event_addJob					:	///< ActiveJobListからの状態変化通知(ジョブの追加).
			break;
		case JSVE_RequestAnswer_Event_DocSearcher				:	///< DocSearcherからの状態変化通知.
			break;
		case JSVE_Request_Event_Max				:	///< 最大値(23).
			#ifdef _DEBUG_JSVC_SEQ
			MyTraceSD(JSVD_DBG_ERROR, "最大値:処理なし。startAction06" , 0 );
			#endif
			break;
		default:
			#ifdef _DEBUG_JSVC_SEQ
			MyTraceSD(JSVD_DBG_ERROR, "default。startAction06" , ps_NotifyInfo->e_Request_Event );
			#endif
			break;
	}

	return e_Request_Result;
}
// 情報通知から状態に応じた処理を実行する.
///< サブシナリオ実行中.
JSVE_Request_Result JSVC_Request::startAction07( JSVS_NotifyInfo*	ps_NotifyInfo, JSVC_Request* pc_Request )
{
	JSVE_Request_Result e_Request_Result = JSVE_Request_Result_Err;

	switch( ps_NotifyInfo->e_Request_Event )
	{
		case JSVE_Request_Event_Non				:	///< 初期値.
			break;
		case JSVE_Request_Event_ReqMemAlloc		:	///< メモリ確保要求.
			// JobManagerへ要求NGを通知する.
			e_Request_Result = JSVE_Request_Result_Ok;
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Err, JSVE_JobServiceResultDetail_AllreadyMemAlloced, pc_Request);
			break;
		case JSVE_Request_Event_AnsAnalyzeCancel:	///< XML解析削除応答.
			break;
		case JSVE_Request_Event_AnsAnalyzeEnd	:	///< XML解析完了応答.
			break;
		case JSVE_Request_Event_AnsMemAllocEnd	:	///< メモリ確保応答.
			break;
		case JSVE_Request_Event_ReqStart		:	///< サブシナリオ開始要求.
			// JobManagerへ要求NGを通知する.
			// （リトライ系の場合は既にジョブの実行を勧めているので２つめの要求をはねる）.
			{
			e_Request_Result = JSVE_Request_Result_Ok;
				JSVC_RequestBase* pReq = NULL;
				pReq = JSVC_JobManager::newInstance()->getRequestListByMsg( pc_Request->getSubScenarioID() , JSVE_JobManager_MsgID_Start );
				//２件あるときは受信したポインタと検索したポインタが一致しない（複数要求されている）.
				if( pReq  == pc_Request ){
					MyTraceSD(JSVD_DBG_REQANS, "開始を返信（リトライ系ジョブ）",(int)pc_Request->getSubScenarioID());
					pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Ok, JSVE_JobServiceResultDetail_Non, pc_Request);
				} else {
					MyTraceSD(JSVD_DBG_ERROR, "実行中の実行はNGとする",(int)pc_Request->getSubScenarioID());
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Err, JSVE_JobServiceResultDetail_Exec, pc_Request);
				}
			}
			break;
		case JSVE_Request_Event_ReqStop			:	///< サブシナリオ停止要求.
			{
				uint32_t ulID = pc_Request->getSubScenarioID();
				JSVC_ParentRequest* pc_PRequest = NULL;
				pc_PRequest = JSVC_JobManager::newInstance()->getParentRequest(ulID);
				//Str【 静的解析対応 : Sky花岡 : 2012.09.03 : CID-439600】.
				if( NULL == pc_PRequest ){
					MyTraceSD(JSVD_DBG_ERROR, "JSVC_RequestJobSetting::startAction07 getParentRequest ERR" , (int)0 );
					break;
				}
				//End【 静的解析対応 : Sky花岡 : 2012.09.03 : CID-439600】.
				if( False == pc_PRequest->getStopFlg() ) {
					uint32_t ul_index;
					ul_index = JSVC_JobManager::newInstance()->availableAnsFunctionIndex( JSVC_CB_INDEX_ans_stop, JSVC_CB_MAX_ans_stop, pc_Request );
					if( ul_index != (uint32_t)-1 ) {			// warning対応 2013/09/21 ooba.
						pc_PRequest->setStopFlg( True );
						// JobManagerへ要求OKを通知する.
						pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Ok, JSVE_JobServiceResultDetail_Non, pc_Request);
						//Str【 静的解析対応 : Sky花岡 : 2012.09.03 : CID-439599】.
						if( NULL == pc_Request->getJobInstance() ){
							MyTraceSD(JSVD_DBG_ERROR, "JSVC_RequestJobSetting::startAction07 getJobInstance ERR" , (int)0 );
							break;
						}
						//End【 静的解析対応 : Sky花岡 : 2012.09.03 : CID-439599】.
						// ジョブ停止指示を行う.
						pc_Request->getJobInstance()->req_stop((void (*)( APIC_Job* ))(JSVC_JobManager::newInstance()->c_CBManage[ul_index].vp_Func));
						// 状態更新(サブシナリオ停止要求中).
						pc_Request->e_Request_Status = JSVE_Request_Status_ReqStop;
						// 結果設定.
						e_Request_Result = JSVE_Request_Result_Ok;
					}
					else{
						MyTraceSD(JSVD_DBG_ERROR, "JSVC_Request::startAction07 ReqStopErr. e_Request_Event:" , ps_NotifyInfo->e_Request_Event );
						// JobManagerへ要求NGを通知する.
						pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Ok, JSVE_JobServiceResultDetail_Non, pc_Request);
						// 結果設定.
						e_Request_Result = JSVE_Request_Result_Err;
					}
				} else {
					// JobManagerへ要求NGを通知する.
					pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, e_Request_Result, JSVE_JobServiceResultDetail_Stop, pc_Request);
				}
			}
			break;
		case JSVE_Request_Event_AnsStopEnd		:	///< サブシナリオ停止応答.
			//停止中に遷移して、結果を返す.
			// 正常終了の場合、アプリケーションプラットフォームへ応答をOKで通知する.
			// 異常終了の場合、アプリケーションプラットフォームへ応答をNGで通知する.
			if (JSVE_Request_Result_Ok == ps_NotifyInfo->pReq->getResult()){
				//Str【 静的解析対応 : Sky花岡 : 2012.09.03 : CID-439598】.
				if( NULL == pc_Request->GetParentRequest() ){
					MyTraceSD(JSVD_DBG_ERROR, "JSVC_RequestJobSetting::startAction07 GetParentRequest ERR" , (int)0 );
					break;
				}
				//End【 静的解析対応 : Sky花岡 : 2012.09.03 : CID-439598】.
				if ( IPCE_JobsResultDetail_StopByStopKey == ps_NotifyInfo->pReq->getResultDetail() ){
					JSVC_CtTrace(JSVD_DBG_INFO, "JobからのStopEnd理由はストップキー",0);
					///全停止による停止.
					pc_Request->GetParentRequest()->setJobExecStatus(JSVC_JOBEXEC_PAUSE_STOPKEY);
					e_Request_Result = JSVE_Request_Result_ReqKeep;
					MyTraceSS(JSVD_DBG_INFO, "受けた停止要因が全停止なのでSKIP","実行中");
					break;
				} else if ( IPCE_JobsResultDetail_StopByWarning == ps_NotifyInfo->pReq->getResultDetail() ){
					//警告による停止.
					JSVC_CtTrace(JSVD_DBG_INFO, "JobからのStopEnd理由は警告",0);
				//Str【 静的解析対応 : Sky花岡 : 2012.09.03 : CID-439598】.
				if( NULL == pc_Request->GetParentRequest() ){
					MyTraceSD(JSVD_DBG_ERROR, "JSVC_RequestJobSetting::startAction07 GetParentRequest ERR" , (int)0 );
					break;
				}
				//End【 静的解析対応 : Sky花岡 : 2012.09.03 : CID-439598】.
					if ( JSVE_WARNING_AUTOSTART == pc_Request->getWarningInfo() ){
						JSVC_CtTrace(JSVD_DBG_REQANS, "警告は自発停止の警告のみ",0);
						pc_Request->GetParentRequest()->setJobExecStatus(JSVC_JOBEXEC_PAUSE_WARNING);
						e_Request_Result = JSVE_Request_Result_ReqKeep;
						MyTraceSS(JSVD_DBG_REQANS, "受けた停止要因が自発再開なのでSKIP","実行中");
						break;
					}
				} else {
					//その他の停止.
					JSVC_CtTrace(JSVD_DBG_INFO, "JobからのStopEnd理由は「その他」", (int)ps_NotifyInfo->pReq->getResultDetail());	// warning対応 2013/09/18 ooba.
				}
				pc_Request->notifyJobService_Answer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Ok, JSVE_JobServiceResultDetail_Non, pc_Request);
			}else{
				pc_Request->notifyJobService_Answer(ps_NotifyInfo->e_Request_Event, e_Request_Result, JSVE_JobServiceResultDetail_Non, pc_Request);
			}
			// 状態更新(サブシナリオ停止中).
			pc_Request->e_Request_Status = JSVE_Request_Status_Stop;
			// 結果設定.
			e_Request_Result = JSVE_Request_Result_Ok;
			break;
		case JSVE_Request_Event_ReqUpdate		:	///< サブシナリオ設定変更要求.
			//既に設定変更リクエストを受けていた場合は新しく受け付けずにNGを返す.
			//（先頭から順に処理するので自リクエストが最初にHITすれば実行可能と判断）.
			if( pc_Request != JSVC_JobManager::newInstance()->GetExecutingUpdateRequest( pc_Request->getSubScenarioID() ) )
			{
				//同一サブシナリオへの多重リクエストありなら受付でNG.
				// JobManagerへ要求NGを通知する.
				pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Err, JSVE_JobServiceResultDetail_MultiReq, pc_Request);
				//ステータスを送付済にしておく.
				((JSVC_RequestJobSetting*)pc_Request)->setSendUpdateEnd();
				e_Request_Result = JSVE_Request_Result_Err;
			} else {
				//重複チェックOKなのでUPDATE処理開始.
				// JobManagerへ要求OKを通知する.
				// 結果設定.
				e_Request_Result = JSVE_Request_Result_Ok;
				if (False == ((JSVC_RequestJobSetting*)pc_Request)->SubsinarioUpdate_byBinaryFile(pc_Request, ((JSVC_RequestJobSetting*)pc_Request)->getFilePath())) {		// XXXXX
					e_Request_Result = JSVE_Request_Result_Err;
					pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, e_Request_Result, JSVE_JobServiceResultDetail_AnalyzeErr, pc_Request);
				} else {
					// JobManagerへ要求OKを通知する.
					pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, e_Request_Result, JSVE_JobServiceResultDetail_Non, pc_Request);
				}
			}
			break;
		case JSVE_Request_Event_AnsUpdateEnd	:	///< サブシナリオ設定変更応答.
			{
//Str【 静的解析対応 : Sky田村 : 2012.08.30 : CID-436578 】.
//				MyTraceSS(JSVD_DBG_INFO, "APFへ設定変更完了を通知","");
//				//JSVC_SendRecv::MakeDataStreamにあわせて結果をRESからREQへ転記.
//				if (( NULL != ps_NotifyInfo->pReq )
//				&&  ( NULL != ps_NotifyInfo->pRes )){
//					ps_NotifyInfo->pReq->setResult( ps_NotifyInfo->pRes->getResult() );
//					ps_NotifyInfo->pReq->setResultDetail( ps_NotifyInfo->pRes->getResultDetail() );
//				}
//				//結果をAPFへ通知.
//				JSVC_JobSendRecv::newInstance()->sendMessageToAPF( JSVE_JobManager_MsgID_ChangeSettingEnd ,ps_NotifyInfo->pReq , ps_NotifyInfo->pRes );
//				e_Request_Result = JSVE_Request_Result_Complete;
				MyTraceSS(JSVD_DBG_INFO, "APFへ設定変更完了を通知","");
				if (( NULL != ps_NotifyInfo->pReq )
				&&  ( NULL != ps_NotifyInfo->pRes )){
					ps_NotifyInfo->pReq->setResult( ps_NotifyInfo->pRes->getResult() );
					ps_NotifyInfo->pReq->setResultDetail( ps_NotifyInfo->pRes->getResultDetail() );
					//結果をAPFへ通知.
					JSVC_JobSendRecv::newInstance()->sendMessageToAPF( JSVE_JobManager_MsgID_ChangeSettingEnd ,ps_NotifyInfo->pReq , ps_NotifyInfo->pRes );
					e_Request_Result = JSVE_Request_Result_Complete;
				}
				else
				{
					MyTraceSS(JSVD_DBG_INFO, "ps_NotifyInfo->pReq or ps_NotifyInfo->pRes is NULL","");
					e_Request_Result = JSVE_Request_Result_Err;
				}
//End【 静的解析対応 : Sky田村 : 2012.08.30 : CID-436578 】.
			}
			break;
		case JSVE_Request_Event_ReqRestart		:	///< サブシナリオ再開要求.
			// JobManagerへ要求NGを通知する.
// 2012.07.10 sk_takas CQ368128 modify -->
//			e_Request_Result = JSVE_Request_Result_Ok;
//			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Err, JSVE_JobServiceResultDetail_Exec, pc_Request);
			{
				APIC_Job* pc_Job = pc_Request->getJobInstance() ;

/* CQ369652対応 S.Wakayama 2012/08/16 Start. */
//				if ( NULL != pc_Job && True == pc_Job->getNetTxExtracted() ) {
				if (( NULL != pc_Job && True == pc_Job->getNetTxExtracted() ) ||
					( NULL != pc_Job && TYPE_OutputStatusSendSmbErrorStop == pc_Job->getJobStatus()->getOutputStatus() ) ||
					( NULL != pc_Job && TYPE_OutputStatusSendSmimeErrorStop == pc_Job->getJobStatus()->getOutputStatus() ) ) {
/* CQ369652対応 S.Wakayama 2012/08/16 End. */
					pc_Job->restart();
					// JobManagerへ要求OKを通知する.
					pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Ok, JSVE_JobServiceResultDetail_Non, pc_Request);
				} else {
					e_Request_Result = JSVE_Request_Result_Ok;
					pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Err, JSVE_JobServiceResultDetail_Exec, pc_Request);
				}
			}
// 2012.07.10 sk_takas CQ368128 modify <--
			break;
		case JSVE_Request_Event_AnsRestartEnd	:	///< サブシナリオ再開応答.
			break;
		case JSVE_Request_Event_ReqCancel		:	///< サブシナリオ削除要求.
			uint32_t ul_index;
			// ジョブキャンセル指示を行う.
			ul_index = JSVC_JobManager::newInstance()->availableAnsFunctionIndex( JSVC_CB_INDEX_ans_cancel, JSVC_CB_MAX_ans_cancel, pc_Request );
			if( ul_index != (uint32_t)-1 )				// warning対応 2013/09/21 ooba.
			{
				// JobManagerへ要求OKを通知する.
				pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Ok, JSVE_JobServiceResultDetail_Non, pc_Request);
				// ジョブキャンセル指示を行う.
				pc_Request->getJobInstance()->req_cancel((void (*)( APIC_Job* ))(JSVC_JobManager::newInstance()->c_CBManage[ul_index].vp_Func));
				// 状態更新(サブシナリオ削除要求中).
				pc_Request->e_Request_Status = JSVE_Request_Status_ReqCancel;
				uint32_t ulID = pc_Request->getSubScenarioID();
				JSVC_ParentRequest* pc_PRequest = NULL;
				pc_PRequest = JSVC_JobManager::newInstance()->getParentRequest(ulID);
				//Str【 静的解析対応 : Sky花岡 : 2012.09.03 : CID-439600】.
				if( NULL == pc_PRequest ){
					MyTraceSD(JSVD_DBG_ERROR, "JSVC_RequestJobSetting::startAction07 getParentRequest ERR" , (int)0 );
					break;
				}
				//End【 静的解析対応 : Sky花岡 : 2012.09.03 : CID-439600】.
				pc_PRequest->setCancelFlg( True );
				// 結果設定.
				e_Request_Result = JSVE_Request_Result_Ok;
			}
			else
			{
				MyTraceSD(JSVD_DBG_ERROR, "JSVC_Request::startAction07 CancelErr. e_Request_Event:" , ps_NotifyInfo->e_Request_Event );
				pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Err, IPCE_JobsResultDetail_CBNumOver, pc_Request);
			}
			break;
		case JSVE_Request_Event_AnsCancel		:	///< サブシナリオ削除応答.
			// 実行中に削除完了が来るケースは、ans_cancelが取得できなかったときのエラーケースのみ.
			// キャンセル要求を削除するために、上位へ通知を返し、キャンセル要求を削除する.
			if( ps_NotifyInfo->pRes->getResult() == JSVE_JobServiceResult_OK ){
				pc_Request->notifyJobService_Answer( ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Ok, ps_NotifyInfo->pRes->getResultDetail(), pc_Request );
			}
			else{
				pc_Request->notifyJobService_Answer( ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Err, ps_NotifyInfo->pRes->getResultDetail(), pc_Request );
			}
			break;
		case JSVE_Request_Event_AnsEnd			:	///< サブシナリオ終了通知.
			{
				uint32_t ulID = pc_Request->getSubScenarioID();
				JSVC_ParentRequest* pc_PRequest = NULL;
				pc_PRequest = JSVC_JobManager::newInstance()->getParentRequest(ulID);
				// CQ370161対策 Start.
				if (NULL == pc_PRequest){
					// 異常終了の場合、アプリケーションプラットフォームへ応答をNGで通知する.
					pc_Request->notifyJobService_Answer(ps_NotifyInfo->e_Request_Event, e_Request_Result, JSVE_JobServiceResultDetail_Non, pc_Request);
					// 状態更新(サブシナリオ終了中).
					pc_Request->e_Request_Status = JSVE_Request_Status_End;
					// 結果設定.
					e_Request_Result = JSVE_Request_Result_Ok;
					break;
				}
				// CQ370161対策 End.
				if( pc_PRequest->getWaitNextJob() ) {
					pc_Request->e_Request_Status = JSVE_Request_Status_WaitNextTrigger;
					e_Request_Result = JSVE_Request_Result_ReqKeep;
					break;
				} else {
					// 正常終了の場合、アプリケーションプラットフォームへ応答をOKで通知する.
					// 異常終了の場合、アプリケーションプラットフォームへ応答をNGで通知する.
					if (JSVE_Request_Result_Ok == ps_NotifyInfo->pReq->getResult()){
						pc_Request->notifyJobService_Answer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Ok, JSVE_JobServiceResultDetail_Non, pc_Request);
					}else{
						pc_Request->notifyJobService_Answer(ps_NotifyInfo->e_Request_Event, e_Request_Result, JSVE_JobServiceResultDetail_Non, pc_Request);
					}
					// 状態更新(サブシナリオ終了中).
					pc_Request->e_Request_Status = JSVE_Request_Status_End;
				}
				// 結果設定.
				e_Request_Result = JSVE_Request_Result_Ok;
			}
			break;
		case JSVE_Request_Event_ReqMemFree		:	///< メモリ開放要求.
			// JobManagerへ要求NGを通知する.
			e_Request_Result = JSVE_Request_Result_Ok;
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Err, JSVE_JobServiceResultDetail_Exec, pc_Request);
			break;
		case JSVE_Request_Event_AnsMemFree		:	///< メモリ開放応答.
			break;
		case JSVE_RequestAnswer_Event_ChargeMapPrepared			:	///< 課金マップ展開完了.
			break;
		case JSVE_RequestAnswer_Event_NewInstance				:	///< 待ちうけ部からの着信通知.
			break;
		case JSVE_RequestAnswer_Event_addJob					:	///< ActiveJobListからの状態変化通知(ジョブの追加).
			break;
		case JSVE_RequestAnswer_Event_DocSearcher				:	///< DocSearcherからの状態変化通知.
			break;
		case JSVE_Request_Event_Max				:	///< 最大値(23).
			#ifdef _DEBUG_JSVC_SEQ
			MyTraceSD(JSVD_DBG_ERROR, "最大値:処理なし。startAction07" , 0 );
			#endif
			break;
		default:
			#ifdef _DEBUG_JSVC_SEQ
			MyTraceSD(JSVD_DBG_ERROR, "default。startAction07" , ps_NotifyInfo->e_Request_Event );
			#endif
			break;
	}

	return e_Request_Result;
}
// 情報通知から状態に応じた処理を実行する.
///< サブシナリオ停止要求中.
JSVE_Request_Result JSVC_Request::startAction08( JSVS_NotifyInfo*	ps_NotifyInfo, JSVC_Request* pc_Request )
{
	JSVE_Request_Result e_Request_Result = JSVE_Request_Result_Err;

	switch( ps_NotifyInfo->e_Request_Event )
	{
		case JSVE_Request_Event_Non				:	///< 初期値.
			break;
		case JSVE_Request_Event_ReqMemAlloc		:	///< メモリ確保要求.
			// JobManagerへ要求NGを通知する.
			e_Request_Result = JSVE_Request_Result_Ok;
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Err, JSVE_JobServiceResultDetail_AllreadyMemAlloced, pc_Request);
			break;
		case JSVE_Request_Event_AnsAnalyzeCancel:	///< XML解析削除応答.
			break;
		case JSVE_Request_Event_AnsAnalyzeEnd	:	///< XML解析完了応答.
			break;
		case JSVE_Request_Event_AnsMemAllocEnd	:	///< メモリ確保応答.
			break;
		case JSVE_Request_Event_ReqStart		:	///< サブシナリオ開始要求.
			// JobManagerへ要求NGを通知する.
			e_Request_Result = JSVE_Request_Result_Ok;
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Err, JSVE_JobServiceResultDetail_Exec, pc_Request);
			break;
		case JSVE_Request_Event_ReqStop			:	///< サブシナリオ停止要求.
			// JobManagerへ要求NGを通知する.
			e_Request_Result = JSVE_Request_Result_Ok;
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Err, JSVE_JobServiceResultDetail_Stop, pc_Request);
			break;
		case JSVE_Request_Event_AnsStopEnd		:	///< サブシナリオ停止応答.
			//停止中に遷移して、結果を返す.
			// 正常終了の場合、アプリケーションプラットフォームへ応答をOKで通知する.
			// 異常終了の場合、アプリケーションプラットフォームへ応答をNGで通知する.
			if (JSVE_Request_Result_Ok == ps_NotifyInfo->pReq->getResult()){
				//Str【 静的解析対応 : Sky花岡 : 2012.09.03 : CID-439601】.
				if( NULL == pc_Request->GetParentRequest() ){
					MyTraceSD(JSVD_DBG_ERROR, "JSVC_RequestJobSetting::startAction08 GetParentRequest ERR" , (int)0 );
					break;
				}
				//End【 静的解析対応 : Sky花岡 : 2012.09.03 : CID-439601】.
				if ( IPCE_JobsResultDetail_StopByStopKey == ps_NotifyInfo->pReq->getResultDetail() ){
					JSVC_CtTrace(JSVD_DBG_INFO, "JobからのStopEnd理由はストップキー",0);
					///全停止による停止.
					pc_Request->GetParentRequest()->setJobExecStatus(JSVC_JOBEXEC_PAUSE_STOPKEY);
					e_Request_Result = JSVE_Request_Result_ReqKeep;
					MyTraceSS(JSVD_DBG_INFO, "受けた停止要因が全停止なのでSKIP","実行中");
					break;
				} else if ( IPCE_JobsResultDetail_StopByWarning == ps_NotifyInfo->pReq->getResultDetail() ){
					//警告による停止.
					JSVC_CtTrace(JSVD_DBG_INFO, "JobからのStopEnd理由は警告",0);
					if ( JSVE_WARNING_AUTOSTART == pc_Request->getWarningInfo() ){
						JSVC_CtTrace(JSVD_DBG_INFO, "警告は自発停止の警告のみ",0);
						pc_Request->GetParentRequest()->setJobExecStatus(JSVC_JOBEXEC_PAUSE_WARNING);
						e_Request_Result = JSVE_Request_Result_ReqKeep;
						MyTraceSS(JSVD_DBG_INFO, "受けた停止要因が自発再開なのでSKIP","実行中");
						break;
					}
				} else {
					//その他の停止.
					JSVC_CtTrace(JSVD_DBG_INFO, "JobからのStopEnd理由は「その他」", (int)ps_NotifyInfo->pReq->getResultDetail());	// warning対応 2013/09/18 ooba.
					
				}
				pc_Request->notifyJobService_Answer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Ok, JSVE_JobServiceResultDetail_Non, pc_Request);
			}else{
				pc_Request->notifyJobService_Answer(ps_NotifyInfo->e_Request_Event, e_Request_Result, JSVE_JobServiceResultDetail_Non, pc_Request);
			}
			// 状態更新(サブシナリオ停止中).
			pc_Request->e_Request_Status = JSVE_Request_Status_Stop;
			// 結果設定.
			e_Request_Result = JSVE_Request_Result_Ok;
			break;
		case JSVE_Request_Event_ReqUpdate		:	///< サブシナリオ設定変更要求.
			// JobManagerへ要求NGを通知する.
			//既に設定変更リクエストを受けていた場合は新しく受け付けずにNGを返す.
			//（先頭から順に処理するので自リクエストが最初にHITすれば実行可能と判断）.
			if( pc_Request != JSVC_JobManager::newInstance()->GetExecutingUpdateRequest( pc_Request->getSubScenarioID() ) )
			{
				//同一サブシナリオへの多重リクエストありなら受付でNG.
				// JobManagerへ要求NGを通知する.
				pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Err, JSVE_JobServiceResultDetail_MultiReq, pc_Request);
				//ステータスを送付済にしておく.
				((JSVC_RequestJobSetting*)pc_Request)->setSendUpdateEnd();
				e_Request_Result = JSVE_Request_Result_Err;
			} else {
				//重複チェックOKなのでUPDATE処理開始.
				// JobManagerへ要求OKを通知する.
				// 結果設定.
				e_Request_Result = JSVE_Request_Result_Ok;
				if (False == ((JSVC_RequestJobSetting*)pc_Request)->SubsinarioUpdate_byBinaryFile(pc_Request, ((JSVC_RequestJobSetting*)pc_Request)->getFilePath())) {		// XXXXX
					e_Request_Result = JSVE_Request_Result_Err;
					pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, e_Request_Result, JSVE_JobServiceResultDetail_AnalyzeErr, pc_Request);
				} else {
					// JobManagerへ要求OKを通知する.
					pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, e_Request_Result, JSVE_JobServiceResultDetail_Non, pc_Request);
				}
			}
			break;
		case JSVE_Request_Event_AnsUpdateEnd	:	///< サブシナリオ設定変更応答.
			{
//Str【 静的解析対応 : Sky田村 : 2012.08.30 : CID-436579 】.
//				MyTraceSS(JSVD_DBG_INFO, "APFへ設定変更完了を通知","");
//				if (( NULL != ps_NotifyInfo->pReq )
//				&&  ( NULL != ps_NotifyInfo->pRes )){
//					ps_NotifyInfo->pReq->setResult( ps_NotifyInfo->pRes->getResult() );
//					ps_NotifyInfo->pReq->setResultDetail( ps_NotifyInfo->pRes->getResultDetail() );
//				}
//				//結果をAPFへ通知.
//				JSVC_JobSendRecv::newInstance()->sendMessageToAPF( JSVE_JobManager_MsgID_ChangeSettingEnd ,ps_NotifyInfo->pReq , ps_NotifyInfo->pRes );
//				e_Request_Result = JSVE_Request_Result_Complete;
				MyTraceSS(JSVD_DBG_INFO, "APFへ設定変更完了を通知","");
				if (( NULL != ps_NotifyInfo->pReq )
				&&  ( NULL != ps_NotifyInfo->pRes )){
					ps_NotifyInfo->pReq->setResult( ps_NotifyInfo->pRes->getResult() );
					ps_NotifyInfo->pReq->setResultDetail( ps_NotifyInfo->pRes->getResultDetail() );
					//結果をAPFへ通知.
					JSVC_JobSendRecv::newInstance()->sendMessageToAPF( JSVE_JobManager_MsgID_ChangeSettingEnd ,ps_NotifyInfo->pReq , ps_NotifyInfo->pRes );
					e_Request_Result = JSVE_Request_Result_Complete;
				}
				else
				{
					MyTraceSS(JSVD_DBG_INFO, "ps_NotifyInfo->pReq or ps_NotifyInfo->pRes is NULL","");
					e_Request_Result = JSVE_Request_Result_Err;
				}
//End【 静的解析対応 : Sky田村 : 2012.08.30 : CID-436579 】.
			}
			break;
		case JSVE_Request_Event_ReqRestart		:	///< サブシナリオ再開要求.
			// JobManagerへ要求NGを通知する.
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, e_Request_Result, JSVE_JobServiceResultDetail_Exec, pc_Request);
			break;
		case JSVE_Request_Event_AnsRestartEnd	:	///< サブシナリオ再開応答.
			break;
		case JSVE_Request_Event_ReqCancel		:	///< サブシナリオ削除要求.
			uint32_t ul_index;
			// ジョブキャンセル指示を行う.
			ul_index = JSVC_JobManager::newInstance()->availableAnsFunctionIndex( JSVC_CB_INDEX_ans_cancel, JSVC_CB_MAX_ans_cancel, pc_Request );
			if( ul_index != (uint32_t)-1 )				// warning対応 2013/09/21 ooba.
			{
				// JobManagerへ要求OKを通知する.
				pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Ok, JSVE_JobServiceResultDetail_Non, pc_Request);
				//Str【 静的解析対応 : Sky花岡 : 2012.09.03 : CID-439602】.
				if( NULL == pc_Request->getJobInstance() ){
					MyTraceSD(JSVD_DBG_ERROR, "JSVC_RequestJobSetting::startAction08 getJobInstance ERR" , (int)0 );
					break;
				}
				//End【 静的解析対応 : Sky花岡 : 2012.09.03 : CID-439602】.
				// ジョブキャンセル指示を行う.
				pc_Request->getJobInstance()->req_cancel((void (*)( APIC_Job* ))(JSVC_JobManager::newInstance()->c_CBManage[ul_index].vp_Func));
				// 状態更新(サブシナリオ削除要求中).
				pc_Request->e_Request_Status = JSVE_Request_Status_ReqCancel;
				uint32_t ulID = pc_Request->getSubScenarioID();
				JSVC_ParentRequest* pc_PRequest = NULL;
				pc_PRequest = JSVC_JobManager::newInstance()->getParentRequest(ulID);
				//Str【 静的解析対応 : Sky花岡 : 2012.09.03 : CID-439603】.
				if( NULL == pc_PRequest ){
					MyTraceSD(JSVD_DBG_ERROR, "JSVC_RequestJobSetting::startAction08 getParentRequest ERR" , (int)0 );
					break;
				}
				//End【 静的解析対応 : Sky花岡 : 2012.09.03 : CID-439603】.
				pc_PRequest->setCancelFlg( True );
				// 結果設定.
				e_Request_Result = JSVE_Request_Result_Ok;
			}
			else
			{
				MyTraceSD(JSVD_DBG_ERROR, "JSVC_Request::startAction08 CancelErr. e_Request_Event:" , ps_NotifyInfo->e_Request_Event );
				e_Request_Result = JSVE_Request_Result_Err;
				pc_Request->setResult(JSVE_JobServiceResult_NG);
				pc_Request->setResultDetail(IPCE_JobsResultDetail_CBNumOver);
				// JobManagerへ要求NGを通知する.
				pc_Request->pc_JobManager->notify( pc_Request, e_Request_Result, JSVE_JobServiceResultDetail_XmlAnalyze, JSVE_JobManager_MsgID_MemoryAllocEnd, 0 );
			}
			break;
		case JSVE_Request_Event_AnsCancel		:	///< サブシナリオ削除応答.
			break;
		case JSVE_Request_Event_AnsEnd			:	///< サブシナリオ終了通知.
			{
				uint32_t ulID = pc_Request->getSubScenarioID();
				JSVC_ParentRequest* pc_PRequest = NULL;
				pc_PRequest = JSVC_JobManager::newInstance()->getParentRequest(ulID);
				//Str【 静的解析対応 : Sky花岡 : 2012.09.03 : CID-439603】.
				if( NULL == pc_PRequest ){
					MyTraceSD(JSVD_DBG_ERROR, "JSVC_RequestJobSetting::startAction08 getParentRequest ERR" , (int)0 );
					break;
				}
				//End【 静的解析対応 : Sky花岡 : 2012.09.03 : CID-439603】.
				if( pc_PRequest->getWaitNextJob() ) {
					pc_Request->e_Request_Status = JSVE_Request_Status_WaitNextTrigger;
				} else {
					// 正常終了の場合、アプリケーションプラットフォームへ応答をOKで通知する.
					// 異常終了の場合、アプリケーションプラットフォームへ応答をNGで通知する.
					if (JSVE_Request_Result_Ok == ps_NotifyInfo->pReq->getResult()){
						pc_Request->notifyJobService_Answer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Ok, JSVE_JobServiceResultDetail_Non, pc_Request);
					}else{
						pc_Request->notifyJobService_Answer(ps_NotifyInfo->e_Request_Event, e_Request_Result, JSVE_JobServiceResultDetail_Non, pc_Request);
					}
					// 状態更新(サブシナリオ終了中).
					pc_Request->e_Request_Status = JSVE_Request_Status_End;
				}
				// 結果設定.
				e_Request_Result = JSVE_Request_Result_Ok;
			}
			break;
		case JSVE_Request_Event_ReqMemFree		:	///< メモリ開放要求.
			// JobManagerへ要求NGを通知する.
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, e_Request_Result, JSVE_JobServiceResultDetail_Exec, pc_Request);
			break;
		case JSVE_Request_Event_AnsMemFree		:	///< メモリ開放応答.
			break;
		case JSVE_RequestAnswer_Event_ChargeMapPrepared			:	///< 課金マップ展開完了.
			break;
		case JSVE_RequestAnswer_Event_NewInstance				:	///< 待ちうけ部からの着信通知.
			break;
		case JSVE_RequestAnswer_Event_addJob					:	///< ActiveJobListからの状態変化通知(ジョブの追加).
			break;
		case JSVE_RequestAnswer_Event_DocSearcher				:	///< DocSearcherからの状態変化通知.
			break;
		case JSVE_Request_Event_Max				:	///< 最大値(23).
			#ifdef _DEBUG_JSVC_SEQ
			MyTraceSD(JSVD_DBG_ERROR, "最大値:処理なし。startAction08" , 0 );
			#endif
			break;
		default:
			#ifdef _DEBUG_JSVC_SEQ
			MyTraceSD(JSVD_DBG_ERROR, "default。startAction08" , ps_NotifyInfo->e_Request_Event );
			#endif
			break;
	}

	return e_Request_Result;
}
// 情報通知から状態に応じた処理を実行する.
///< サブシナリオ停止中.
JSVE_Request_Result JSVC_Request::startAction09( JSVS_NotifyInfo*	ps_NotifyInfo, JSVC_Request* pc_Request )
{
	JSVE_Request_Result e_Request_Result = JSVE_Request_Result_Err;

	Bool b_isStsNoChange = False; // 2014/12/21 ZeusS CQ:505266 Nagasawa add.

	switch( ps_NotifyInfo->e_Request_Event )
	{
		case JSVE_Request_Event_Non				:	///< 初期値.
			break;
		case JSVE_Request_Event_ReqMemAlloc		:	///< メモリ確保要求.
			// JobManagerへ要求NGを通知する.
			e_Request_Result = JSVE_Request_Result_Ok;
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Err, JSVE_JobServiceResultDetail_AllreadyMemAlloced, pc_Request);
			break;
		case JSVE_Request_Event_AnsAnalyzeCancel:	///< XML解析削除応答.
			break;
		case JSVE_Request_Event_AnsAnalyzeEnd	:	///< XML解析完了応答.
			break;
		case JSVE_Request_Event_AnsMemAllocEnd	:	///< メモリ確保応答.
			break;
		case JSVE_Request_Event_ReqStart		:	///< サブシナリオ開始要求.
			// JobManagerへ要求NGを通知する.
			e_Request_Result = JSVE_Request_Result_Ok;
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Err, JSVE_JobServiceResultDetail_Exec, pc_Request);
			break;
		case JSVE_Request_Event_ReqStop			:	///< サブシナリオ停止要求.
			// JobManagerへ要求NGを通知する.
			e_Request_Result = JSVE_Request_Result_Ok;
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Err, JSVE_JobServiceResultDetail_Stop, pc_Request);
			break;
		case JSVE_Request_Event_AnsStopEnd		:	///< サブシナリオ停止応答.
			break;
		case JSVE_Request_Event_ReqUpdate		:	///< サブシナリオ設定変更要求.
			// XML⇒JobSpec変換要求.
			if (NULL == pc_Request->getJobInstance()){
				// JobManagerへ要求NGを通知する.
				pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, e_Request_Result, JSVE_JobServiceResultDetail_AnalyzeErr, pc_Request);
			//既に設定変更リクエストを受けていた場合は新しく受け付けずにNGを返す.
			//（先頭から順に処理するので自リクエストが最初にHITすれば実行可能と判断）.
			}
			else if( pc_Request != JSVC_JobManager::newInstance()->GetExecutingUpdateRequest( pc_Request->getSubScenarioID() ) )
			{
				//同一サブシナリオへの多重リクエストありなら受付でNG.
				// JobManagerへ要求NGを通知する.
				pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Err, JSVE_JobServiceResultDetail_MultiReq, pc_Request);
				//ステータスを送付済にしておく.
				((JSVC_RequestJobSetting*)pc_Request)->setSendUpdateEnd();
				e_Request_Result = JSVE_Request_Result_Err;

			}else{
				if (JSVE_Request_Class_Type_Req_Job_Set != pc_Request->WhoAmI()) {
					pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, e_Request_Result, JSVE_JobServiceResultDetail_AnalyzeErr, pc_Request);
				}
				e_Request_Result = JSVE_Request_Result_Ok;
				if (False == ((JSVC_RequestJobSetting*)pc_Request)->SubsinarioUpdate_byBinaryFile(pc_Request, ((JSVC_RequestJobSetting*)pc_Request)->getFilePath())) {		// XXXXX
					e_Request_Result = JSVE_Request_Result_Err;
					pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, e_Request_Result, JSVE_JobServiceResultDetail_AnalyzeErr, pc_Request);
				} else {
					// JobManagerへ要求OKを通知する.
					pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, e_Request_Result, JSVE_JobServiceResultDetail_Non, pc_Request);
				}
			}
			break;
		case JSVE_Request_Event_AnsUpdateEnd	:	///< サブシナリオ設定変更応答.
			{
//Str【 静的解析対応 : Sky田村 : 2012.08.30 : CID-436580 】.
//				MyTraceSS(JSVD_DBG_INFO, "APFへ設定変更完了を通知","");
//				//JSVC_SendRecv::MakeDataStreamにあわせて結果をRESからREQへ転記.
//				if (( NULL != ps_NotifyInfo->pReq )
//				&&  ( NULL != ps_NotifyInfo->pRes )){
//					ps_NotifyInfo->pReq->setResult( ps_NotifyInfo->pRes->getResult() );
//					ps_NotifyInfo->pReq->setResultDetail( ps_NotifyInfo->pRes->getResultDetail() );
//				}
//				//結果をAPFへ通知.
//				JSVC_JobSendRecv::newInstance()->sendMessageToAPF( JSVE_JobManager_MsgID_ChangeSettingEnd ,ps_NotifyInfo->pReq , ps_NotifyInfo->pRes );
//				e_Request_Result = JSVE_Request_Result_Complete;
				MyTraceSS(JSVD_DBG_INFO, "APFへ設定変更完了を通知","");
				if (( NULL != ps_NotifyInfo->pReq )
				&&  ( NULL != ps_NotifyInfo->pRes )){
					ps_NotifyInfo->pReq->setResult( ps_NotifyInfo->pRes->getResult() );
					ps_NotifyInfo->pReq->setResultDetail( ps_NotifyInfo->pRes->getResultDetail() );
					//結果をAPFへ通知.
					JSVC_JobSendRecv::newInstance()->sendMessageToAPF( JSVE_JobManager_MsgID_ChangeSettingEnd ,ps_NotifyInfo->pReq , ps_NotifyInfo->pRes );
					e_Request_Result = JSVE_Request_Result_Complete;
				}
				else
				{
					MyTraceSS(JSVD_DBG_INFO, "ps_NotifyInfo->pReq or ps_NotifyInfo->pRes is NULL","");
					e_Request_Result = JSVE_Request_Result_Err;
				}
//End【 静的解析対応 : Sky田村 : 2012.08.30 : CID-436580 】.
			}
			break;
		case JSVE_Request_Event_ReqRestart		:	///< サブシナリオ再開要求.
			// JobManagerへ要求OKを通知する.
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Ok, JSVE_JobServiceResultDetail_Non, pc_Request);
			// ジョブ再開指示を行う.
			{
				uint32_t ulID = pc_Request->getSubScenarioID();
				JSVC_ParentRequest* pc_PRequest = NULL;
				pc_PRequest = JSVC_JobManager::newInstance()->getParentRequest(ulID);
				//Str【 静的解析対応 : Sky花岡 : 2012.09.03 : CID-439605】.
				if( NULL == pc_PRequest ){
					MyTraceSD(JSVD_DBG_ERROR, "JSVC_RequestJobSetting::startAction09 getParentRequest ERR" , (int)0 );
					break;
				}
				//End【 静的解析対応 : Sky花岡 : 2012.09.03 : CID-439605】.
				
// 2014/12/21 ZeusS CQ:505266 Nagasawa mod sta.
// ScanToHome 送信エラー発生時、連続読込みであれば状態遷移させない.
				TYPE_JobWarning e_Warning = pc_Request->getJobInstance()->getErrorStatus();
				
				if (ERROR_ScanToHomeSendError == e_Warning) {
					if (pc_Request->getJobInstance()->getJobSpec()->pc_Scan->c_MemoryIn.getMemoryInOn() == True) {
						b_isStsNoChange = True;
					}
				}
				
				if (False == b_isStsNoChange) {
					if( True == pc_PRequest->getStopFlg() ) {
						pc_PRequest->setStopFlg( False );
					}
				}
// 2014/12/21 ZeusS CQ:505266 Nagasawa mod end.
			}
			{
				uint32_t ulID = pc_Request->getSubScenarioID();
				JSVC_ParentRequest* pc_PRequest = NULL;
				pc_PRequest = JSVC_JobManager::newInstance()->getParentRequest(ulID);
				//Str【 静的解析対応 : Sky花岡 : 2012.09.03 : CID-439605】.
				if( NULL == pc_PRequest ){
					MyTraceSD(JSVD_DBG_ERROR, "JSVC_RequestJobSetting::startAction09 getParentRequest ERR" , (int)0 );
					break;
				}
				//End【 静的解析対応 : Sky花岡 : 2012.09.03 : CID-439605】.
				// 分割読込み停止中の場合は、APIC_Job->restart() 
				MyTraceSD(JSVD_DBG_INFO, "再開要求を受けて通常の再開を実行1",0);
				pc_Request->getJobInstance()->restart();

// 2014/12/21 ZeusS CQ:505266 Nagasawa mod sta.
				if (False == b_isStsNoChange) {
					// 再開時、停止完了通知済みフラグが立っていれば落とす.
					if (True == pc_PRequest->getEndNotice(JSVE_EndNoticeBit_NormalStopEnd)) {
						pc_PRequest->setEndNotice(JSVE_EndNoticeBit_NormalStopEnd, False);
					}
					if (True == pc_PRequest->getEndNotice(JSVE_EndNoticeBit_MemInStopEnd)) {
						pc_PRequest->setEndNotice(JSVE_EndNoticeBit_MemInStopEnd, False);
					}
				} else {
					// ScanToHome送信エラー時に状態遷移させないため以降の処理を skip.
					break;
				}
// 2014/12/21 ZeusS CQ:505266 Nagasawa mod end.
			}
			// 状態更新(サブシナリオ再開要求中).
			pc_Request->e_Request_Status = JSVE_Request_Status_Exec;
			JSVC_CtTrace(JSVD_DBG_INFO, "setJobExecStatusを初期化",0);
			//Str【 静的解析対応 : Sky花岡 : 2012.09.03 : CID-439604】.
			if( NULL == pc_Request->GetParentRequest() ){
				MyTraceSD(JSVD_DBG_ERROR, "JSVC_RequestJobSetting::startAction09 GetParentRequest ERR" , (int)0 );
				break;
			}
			//End【 静的解析対応 : Sky花岡 : 2012.09.03 : CID-439604】.
			pc_Request->GetParentRequest()->setJobExecStatus(JSVC_JOBEXEC_NOT_PAUSE);	//前段の処理で全停止した場合はAllRestartEndでもフラグ落とす.
			// 結果設定.
			e_Request_Result = JSVE_Request_Result_Ok;
			break;
		case JSVE_Request_Event_AnsRestartEnd	:	///< サブシナリオ再開応答.
			break;
		case JSVE_Request_Event_ReqCancel		:	///< サブシナリオ削除要求.
			uint32_t ul_index;
			// ジョブキャンセル指示を行う.
			ul_index = JSVC_JobManager::newInstance()->availableAnsFunctionIndex( JSVC_CB_INDEX_ans_cancel, JSVC_CB_MAX_ans_cancel, pc_Request );
			if( ul_index != (uint32_t)-1 )				// warning対応 2013/09/21 ooba.
			{
				// JobManagerへ要求OKを通知する.
				pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Ok, JSVE_JobServiceResultDetail_Non, pc_Request);
				//Str【 静的解析対応 : Sky花岡 : 2012.09.03 : CID-439606】.
				if( NULL == pc_Request->getJobInstance() ){
					MyTraceSD(JSVD_DBG_ERROR, "JSVC_RequestJobSetting::startAction10 getJobInstance ERR" , (int)0 );
					break;
				}
				//End【 静的解析対応 : Sky花岡 : 2012.09.03 : CID-439606】.
				// ジョブキャンセル指示を行う.
				pc_Request->getJobInstance()->req_cancel((void (*)( APIC_Job* ))(JSVC_JobManager::newInstance()->c_CBManage[ul_index].vp_Func));
				// 状態更新(サブシナリオ削除要求中).
				pc_Request->e_Request_Status = JSVE_Request_Status_ReqCancel;
				uint32_t ulID = pc_Request->getSubScenarioID();
				JSVC_ParentRequest* pc_PRequest = NULL;
				pc_PRequest = JSVC_JobManager::newInstance()->getParentRequest(ulID);
				//Str【 静的解析対応 : Sky花岡 : 2012.09.03 : CID-439605】.
				if( NULL == pc_PRequest ){
					MyTraceSD(JSVD_DBG_ERROR, "JSVC_RequestJobSetting::startAction09 getParentRequest ERR" , (int)0 );
					break;
				}
				//End【 静的解析対応 : Sky花岡 : 2012.09.03 : CID-439605】.
				pc_PRequest->setCancelFlg( True );
				// 結果設定.
				e_Request_Result = JSVE_Request_Result_Ok;
			}
			else
			{
				e_Request_Result = JSVE_Request_Result_Ok;
				// JobManagerへ要求NGを通知する.
				pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Err, IPCE_JobsResultDetail_CBNumOver, pc_Request);
			}
			break;
		case JSVE_Request_Event_AnsCancel		:	///< サブシナリオ削除応答.
			break;
		case JSVE_Request_Event_AnsEnd			:	///< サブシナリオ終了通知.
			{
				uint32_t ulID = pc_Request->getSubScenarioID();
				JSVC_ParentRequest* pc_PRequest = NULL;
				pc_PRequest = JSVC_JobManager::newInstance()->getParentRequest(ulID);
				//Str【 静的解析対応 : Sky花岡 : 2012.09.03 : CID-439605】.
				if( NULL == pc_PRequest ){
					MyTraceSD(JSVD_DBG_ERROR, "JSVC_RequestJobSetting::startAction09 getParentRequest ERR" , (int)0 );
					break;
				}
				//End【 静的解析対応 : Sky花岡 : 2012.09.03 : CID-439605】.
				if( pc_PRequest->getWaitNextJob() ) {
					pc_Request->e_Request_Status = JSVE_Request_Status_WaitNextTrigger;
				} else {
					// 正常終了の場合、アプリケーションプラットフォームへ応答をOKで通知する.
					// 異常終了の場合、アプリケーションプラットフォームへ応答をNGで通知する.
					if (JSVE_Request_Result_Ok == ps_NotifyInfo->pRes->getResult()){
						pc_Request->notifyJobService_Answer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Ok, JSVE_JobServiceResultDetail_Non, pc_Request);
					}else{
						pc_Request->notifyJobService_Answer(ps_NotifyInfo->e_Request_Event, e_Request_Result, JSVE_JobServiceResultDetail_Non, pc_Request);
					}
					// 状態更新(サブシナリオ終了中).
					pc_Request->e_Request_Status = JSVE_Request_Status_End;
				}
				// 結果設定.
				e_Request_Result = JSVE_Request_Result_Ok;
			}
			break;
		case JSVE_Request_Event_ReqMemFree		:	///< メモリ開放要求.
			// JobManagerへ要求NGを通知する.
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, e_Request_Result, JSVE_JobServiceResultDetail_Exec, pc_Request);
			break;
		case JSVE_Request_Event_AnsMemFree		:	///< メモリ開放応答.
			break;
		case JSVE_RequestAnswer_Event_ChargeMapPrepared			:	///< 課金マップ展開完了.
			break;
		case JSVE_RequestAnswer_Event_NewInstance				:	///< 待ちうけ部からの着信通知.
			break;
		case JSVE_RequestAnswer_Event_addJob					:	///< ActiveJobListからの状態変化通知(ジョブの追加).
			break;
		case JSVE_RequestAnswer_Event_DocSearcher				:	///< DocSearcherからの状態変化通知.
			break;
		case JSVE_Request_Event_Max				:	///< 最大値(23).
			#ifdef _DEBUG_JSVC_SEQ
			MyTraceSD(JSVD_DBG_ERROR, "最大値:処理なし。startAction09" , 0 );
			#endif
			break;
		default:
			#ifdef _DEBUG_JSVC_SEQ
			MyTraceSD(JSVD_DBG_ERROR, "default。startAction09" , ps_NotifyInfo->e_Request_Event );
			#endif
			break;
	}

	return e_Request_Result;
}
// 情報通知から状態に応じた処理を実行する.
///< サブシナリオ設定変更要求中.
JSVE_Request_Result JSVC_Request::startAction10( JSVS_NotifyInfo*	ps_NotifyInfo, JSVC_Request* pc_Request )
{
	JSVE_Request_Result e_Request_Result = JSVE_Request_Result_Err;

	switch( ps_NotifyInfo->e_Request_Event )
	{
		case JSVE_Request_Event_Non				:	///< 初期値.
			break;
		case JSVE_Request_Event_ReqMemAlloc		:	///< メモリ確保要求.
			// JobManagerへ要求NGを通知する.
			e_Request_Result = JSVE_Request_Result_Ok;
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Err, JSVE_JobServiceResultDetail_AllreadyMemAlloced, pc_Request);
			break;
		case JSVE_Request_Event_AnsAnalyzeEnd	:	///< XML解析完了応答.
			// JobManagerへ要求OKを通知する.
			if (JSVE_Request_Result_Ok == ps_NotifyInfo->pReq->getResult()){
				if (JSVE_Request_Class_Type_Req_Job_Set != pc_Request->WhoAmI()) {
					pc_Request->pc_JobManager->notify( pc_Request, JSVE_JobServiceResult_NG, JSVE_JobServiceResultDetail_IllegalRequest, JSVE_JobManager_MsgID_ChangeSettingEnd, 0 );
					break;
				}

				if (False == ((JSVC_RequestJobSetting*)pc_Request)->SubsinarioUpdate_byBinaryFile(pc_Request, ((JSVC_RequestJobSetting*)pc_Request)->getJobSpecFilePath())) {		// XXXXX
					pc_Request->pc_JobManager->notify( pc_Request, JSVE_JobServiceResult_NG, JSVE_JobServiceResultDetail_JobPointerNull, JSVE_JobManager_MsgID_ChangeSettingEnd, 0 );
				}
				else{
					//// 結果設定.
				    e_Request_Result = JSVE_Request_Result_Ok;
				}
			}else{
				pc_Request->pc_JobManager->notify( pc_Request, e_Request_Result, JSVE_JobServiceResultDetail_XmlAnalyze, JSVE_JobManager_MsgID_ChangeSettingEnd, 0 );
				// 結果設定.
				e_Request_Result = JSVE_Request_Result_Err;
			}
			break;
		case JSVE_Request_Event_AnsMemAllocEnd	:	///< メモリ確保応答.
			break;
		case JSVE_Request_Event_ReqStart		:	///< サブシナリオ開始要求.
			// JobManagerへ要求NGを通知する.
			e_Request_Result = JSVE_Request_Result_Ok;
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Err, JSVE_JobServiceResultDetail_Exec, pc_Request);
			break;
		case JSVE_Request_Event_ReqStop			:	///< サブシナリオ停止要求.
			// JobManagerへ要求NGを通知する.
			e_Request_Result = JSVE_Request_Result_Ok;
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Err, JSVE_JobServiceResultDetail_Stop, pc_Request);
			break;
		case JSVE_Request_Event_AnsStopEnd		:	///< サブシナリオ停止応答.
			break;
		case JSVE_Request_Event_ReqUpdate		:	///< サブシナリオ設定変更要求.
			// JobManagerへ要求NGを通知する.
			e_Request_Result = JSVE_Request_Result_Ok;
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Err, JSVE_JobServiceResultDetail_Exec, pc_Request);
			break;
		case JSVE_Request_Event_AnsUpdateEnd	:	///< サブシナリオ設定変更応答.
			// 正常終了の場合、アプリケーションプラットフォームへ応答をOKで通知する.
			// 異常終了の場合、アプリケーションプラットフォームへ応答をNGで通知する.
			if (JSVE_Request_Result_Ok == ps_NotifyInfo->pReq->getResult()){
				if( (pc_Request->ul_DetailParam & JSVD_DetailParamMask_XmlAnalyze) == TYPE_XmlResultStatus_SkipEnd ){
					pc_Request->notifyJobService_Answer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Ok, JSVE_JobServiceResultDetail_XmlSkipEnd, pc_Request);
				}
				else{
					pc_Request->notifyJobService_Answer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Ok, JSVE_JobServiceResultDetail_Non, pc_Request);
				}
			}else{
				pc_Request->notifyJobService_Answer(ps_NotifyInfo->e_Request_Event, e_Request_Result, (JSVE_JobServiceResultDetail)ps_NotifyInfo->pReq->getResultDetail(), pc_Request);
			}
			// 状態更新(サブシナリオ停止中).
			pc_Request->e_Request_Status = JSVE_Request_Status_Stop;
			// 結果設定.
			e_Request_Result = JSVE_Request_Result_Ok;
			break;
		case JSVE_Request_Event_ReqRestart		:	///< サブシナリオ再開要求.
			// JobManagerへ要求NGを通知する.
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, e_Request_Result, JSVE_JobServiceResultDetail_Exec, pc_Request);
			break;
		case JSVE_Request_Event_AnsRestartEnd	:	///< サブシナリオ再開応答.
			break;
		case JSVE_Request_Event_ReqCancel		:	///< サブシナリオ削除要求.
			uint32_t ul_index;
			// ジョブキャンセル指示を行う.
			ul_index = JSVC_JobManager::newInstance()->availableAnsFunctionIndex( JSVC_CB_INDEX_ans_cancel, JSVC_CB_MAX_ans_cancel, pc_Request );
			if( ul_index != (uint32_t)-1 )				// warning対応 2013/09/21 ooba.
			{
				// JobManagerへ要求OKを通知する.
				pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Ok, JSVE_JobServiceResultDetail_Non, pc_Request);
				pc_Request->pc_XmlAnalyze->req_CancelXmlAnalyze(pc_Request->ul_SubScenarioID);	// XML解析のキャンセル要求を行う.
				//Str【 静的解析対応 : Sky花岡 : 2012.09.04 : CID-439606】.
				if( NULL == pc_Request->getJobInstance() ){
					MyTraceSD(JSVD_DBG_ERROR, "JSVC_RequestJobSetting::startAction10 getJobInstance ERR" , (int)0 );
					break;
				}
				//End【 静的解析対応 : Sky花岡 : 2012.09.04 : CID-439606】.
				// ジョブキャンセル指示を行う.
				pc_Request->getJobInstance()->req_cancel((void (*)( APIC_Job* ))(JSVC_JobManager::newInstance()->c_CBManage[ul_index].vp_Func));

				uint32_t ulID = pc_Request->getSubScenarioID();
				JSVC_ParentRequest* pc_PRequest = NULL;
				pc_PRequest = JSVC_JobManager::newInstance()->getParentRequest(ulID);
				//Str【 静的解析対応 : Sky花岡 : 2012.09.03 : CID-439607】.
				if( NULL == pc_PRequest ){
					MyTraceSD(JSVD_DBG_ERROR, "JSVC_RequestJobSetting::startAction10 getParentRequest ERR" , (int)0 );
					break;
				}
				//End【 静的解析対応 : Sky花岡 : 2012.09.03 : CID-439607】.
				pc_PRequest->setCancelFlg( True );

				// 状態更新(サブシナリオ削除要求中).
				pc_Request->e_Request_Status = JSVE_Request_Status_ReqCancel;
				// 結果設定.
				e_Request_Result = JSVE_Request_Result_Ok;
			}
			else
			{
				MyTraceSD(JSVD_DBG_ERROR, "JSVC_Request::startAction10 CancelErr. e_Request_Event:" , ps_NotifyInfo->e_Request_Event );
				e_Request_Result = JSVE_Request_Result_Err;
				pc_Request->setResult(JSVE_JobServiceResult_NG);
				pc_Request->setResultDetail(IPCE_JobsResultDetail_CBNumOver);
				// JobManagerへ要求NGを通知する.
				pc_Request->pc_JobManager->notify( pc_Request, e_Request_Result, JSVE_JobServiceResultDetail_XmlAnalyze, JSVE_JobManager_MsgID_MemoryAllocEnd, 0 );
			}
			break;
		case JSVE_Request_Event_AnsCancel		:	///< サブシナリオ削除応答.
			break;
		case JSVE_Request_Event_AnsEnd			:	///< サブシナリオ終了通知.
			// 正常終了の場合、アプリケーションプラットフォームへ応答をOKで通知する.
			// 異常終了の場合、アプリケーションプラットフォームへ応答をNGで通知する.
			if (JSVE_Request_Result_Ok == ps_NotifyInfo->pReq->getResult()){
				pc_Request->notifyJobService_Answer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Ok, JSVE_JobServiceResultDetail_Non, pc_Request);
			}else{
				pc_Request->notifyJobService_Answer(ps_NotifyInfo->e_Request_Event, e_Request_Result, JSVE_JobServiceResultDetail_Non, pc_Request);
			}
			// 状態更新(サブシナリオ終了中).
			pc_Request->e_Request_Status = JSVE_Request_Status_End;
			// 結果設定.
			e_Request_Result = JSVE_Request_Result_Ok;
			break;
		case JSVE_Request_Event_ReqMemFree		:	///< メモリ開放要求.
			// JobManagerへ要求NGを通知する.
			e_Request_Result = JSVE_Request_Result_Ok;
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Err, JSVE_JobServiceResultDetail_Exec, pc_Request);
			break;
		case JSVE_Request_Event_AnsMemFree		:	///< メモリ開放応答.
			break;
		case JSVE_RequestAnswer_Event_ChargeMapPrepared			:	///< 課金マップ展開完了.
			break;
		case JSVE_RequestAnswer_Event_NewInstance				:	///< 待ちうけ部からの着信通知.
			break;
		case JSVE_RequestAnswer_Event_addJob					:	///< ActiveJobListからの状態変化通知(ジョブの追加).
			break;
		case JSVE_RequestAnswer_Event_DocSearcher				:	///< DocSearcherからの状態変化通知.
			break;
		case JSVE_Request_Event_Max				:	///< 最大値(23).
			#ifdef _DEBUG_JSVC_SEQ
			MyTraceSD(JSVD_DBG_ERROR, "最大値:処理なし。startAction10" , 0 );
			#endif
			break;
		default:
			#ifdef _DEBUG_JSVC_SEQ
			MyTraceSD(JSVD_DBG_ERROR, "default。startAction10" , ps_NotifyInfo->e_Request_Event );
			#endif
			break;
	}

	return e_Request_Result;
}
// 情報通知から状態に応じた処理を実行する.
///< サブシナリオ再開要求中.
JSVE_Request_Result JSVC_Request::startAction11( JSVS_NotifyInfo*	ps_NotifyInfo, JSVC_Request* pc_Request )
{
	JSVE_Request_Result e_Request_Result = JSVE_Request_Result_Err;

	switch( ps_NotifyInfo->e_Request_Event )
	{
		case JSVE_Request_Event_Non				:	///< 初期値.
			break;
		case JSVE_Request_Event_ReqMemAlloc		:	///< メモリ確保要求.
			// JobManagerへ要求NGを通知する.
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, e_Request_Result, JSVE_JobServiceResultDetail_AllreadyMemAlloced, pc_Request);
			break;
		case JSVE_Request_Event_AnsAnalyzeCancel:	///< XML解析削除応答.
			break;
		case JSVE_Request_Event_AnsAnalyzeEnd	:	///< XML解析完了応答.
			break;
		case JSVE_Request_Event_AnsMemAllocEnd	:	///< メモリ確保応答.
			break;
		case JSVE_Request_Event_ReqStart		:	///< サブシナリオ開始要求.
			// JobManagerへ要求NGを通知する.
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, e_Request_Result, JSVE_JobServiceResultDetail_Exec, pc_Request);
			break;
		case JSVE_Request_Event_ReqStop			:	///< サブシナリオ停止要求.
			// JobManagerへ要求NGを通知する.
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, e_Request_Result, JSVE_JobServiceResultDetail_Stop, pc_Request);
			break;
		case JSVE_Request_Event_AnsStopEnd		:	///< サブシナリオ停止応答.
			break;
		case JSVE_Request_Event_ReqUpdate		:	///< サブシナリオ設定変更要求.
			//既に設定変更リクエストを受けていた場合は新しく受け付けずにNGを返す.
			//（先頭から順に処理するので自リクエストが最初にHITすれば実行可能と判断）.
			if( pc_Request != JSVC_JobManager::newInstance()->GetExecutingUpdateRequest( pc_Request->getSubScenarioID() ) )
			{
				//受付でも確認しているので重複チェックとなるが・・.
				//同一サブシナリオへの多重リクエストありなら受付でNG.
				// JobManagerへ要求NGを通知する.
				pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Err, JSVE_JobServiceResultDetail_MultiReq, pc_Request);
				//ステータスを送付済にしておく.
				((JSVC_RequestJobSetting*)pc_Request)->setSendUpdateEnd();
				e_Request_Result = JSVE_Request_Result_Err;
			} else {
				//重複チェックOKなのでUPDATE処理開始.
				// JobManagerへ要求OKを通知する.
				// 結果設定.
				e_Request_Result = JSVE_Request_Result_Ok;
				if (False == ((JSVC_RequestJobSetting*)pc_Request)->SubsinarioUpdate_byBinaryFile(pc_Request, ((JSVC_RequestJobSetting*)pc_Request)->getFilePath())) {		// XXXXX
					e_Request_Result = JSVE_Request_Result_Err;
					pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, e_Request_Result, JSVE_JobServiceResultDetail_AnalyzeErr, pc_Request);
				} else {
					// JobManagerへ要求OKを通知する.
					pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, e_Request_Result, JSVE_JobServiceResultDetail_Non, pc_Request);
				}
			}

			break;
		case JSVE_Request_Event_AnsUpdateEnd	:	///< サブシナリオ設定変更応答.
			{
//Str【 静的解析対応 : Sky田村 : 2012.08.30 : CID-436581 】.
//				MyTraceSS(JSVD_DBG_INFO, "APFへ設定変更完了を通知","");
//				//JSVC_SendRecv::MakeDataStreamにあわせて結果をRESからREQへ転記.
//				if (( NULL != ps_NotifyInfo->pReq )
//				&&  ( NULL != ps_NotifyInfo->pRes )){
//					ps_NotifyInfo->pReq->setResult( ps_NotifyInfo->pRes->getResult() );
//					ps_NotifyInfo->pReq->setResultDetail( ps_NotifyInfo->pRes->getResultDetail() );
//				}
//				//結果をAPFへ通知.
//				JSVC_JobSendRecv::newInstance()->sendMessageToAPF( JSVE_JobManager_MsgID_ChangeSettingEnd ,ps_NotifyInfo->pReq , ps_NotifyInfo->pRes );
//				e_Request_Result = JSVE_Request_Result_Complete;
				MyTraceSS(JSVD_DBG_INFO, "APFへ設定変更完了を通知","");
				if (( NULL != ps_NotifyInfo->pReq )
				&&  ( NULL != ps_NotifyInfo->pRes )){
					ps_NotifyInfo->pReq->setResult( ps_NotifyInfo->pRes->getResult() );
					ps_NotifyInfo->pReq->setResultDetail( ps_NotifyInfo->pRes->getResultDetail() );
					//結果をAPFへ通知.
					JSVC_JobSendRecv::newInstance()->sendMessageToAPF( JSVE_JobManager_MsgID_ChangeSettingEnd ,ps_NotifyInfo->pReq , ps_NotifyInfo->pRes );
					e_Request_Result = JSVE_Request_Result_Complete;
				}
				else
				{
					MyTraceSS(JSVD_DBG_INFO, "ps_NotifyInfo->pReq or ps_NotifyInfo->pRes is NULL","");
					e_Request_Result = JSVE_Request_Result_Err;
				}
//End【 静的解析対応 : Sky田村 : 2012.08.30 : CID-436581 】.
			}
			break;
		case JSVE_Request_Event_ReqRestart		:	///< サブシナリオ再開要求.
			// JobManagerへ要求NGを通知する.
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, e_Request_Result, JSVE_JobServiceResultDetail_Exec, pc_Request);
			break;
		case JSVE_Request_Event_AnsRestartEnd	:	///< サブシナリオ再開応答.
			// 正常終了の場合、アプリケーションプラットフォームへ応答をOKで通知する.
			// 異常終了の場合、アプリケーションプラットフォームへ応答をNGで通知する.
			if (JSVE_Request_Result_Ok == ps_NotifyInfo->pReq->getResult()){
				pc_Request->notifyJobService_Answer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Ok, JSVE_JobServiceResultDetail_Non, pc_Request);
			}else{
				pc_Request->notifyJobService_Answer(ps_NotifyInfo->e_Request_Event, e_Request_Result, JSVE_JobServiceResultDetail_Non, pc_Request);
			}
			// 状態更新(サブシナリオ実行中).
			pc_Request->e_Request_Status = JSVE_Request_Status_Exec;
			// 結果設定.
			e_Request_Result = JSVE_Request_Result_Ok;
			break;
		case JSVE_Request_Event_ReqCancel		:	///< サブシナリオ削除要求.
			uint32_t ul_index;
			// ジョブキャンセル指示を行う.
			ul_index = JSVC_JobManager::newInstance()->availableAnsFunctionIndex( JSVC_CB_INDEX_ans_cancel, JSVC_CB_MAX_ans_cancel, pc_Request );
			if( ul_index != (uint32_t)-1 )				// warning対応 2013/09/21 ooba.
			{
				// JobManagerへ要求OKを通知する.
				pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Ok, JSVE_JobServiceResultDetail_Non, pc_Request);
				//Str【 静的解析対応 : Sky花岡 : 2012.09.03 : CID-439608】.
				if( NULL == pc_Request->getJobInstance() ){
					MyTraceSD(JSVD_DBG_ERROR, "JSVC_RequestJobSetting::startAction11 getJobInstance ERR" , (int)0 );
					break;
				}
				//End【 静的解析対応 : Sky花岡 : 2012.09.03 : CID-439608】.
				// ジョブキャンセル指示を行う.
				pc_Request->getJobInstance()->req_cancel((void (*)( APIC_Job* ))(JSVC_JobManager::newInstance()->c_CBManage[ul_index].vp_Func));

				uint32_t ulID = pc_Request->getSubScenarioID();
				JSVC_ParentRequest* pc_PRequest = NULL;
				pc_PRequest = JSVC_JobManager::newInstance()->getParentRequest(ulID);
				//Str【 静的解析対応 : Sky花岡 : 2012.09.03 : CID-439609】.
				if( NULL == pc_PRequest ){
					MyTraceSD(JSVD_DBG_ERROR, "JSVC_RequestJobSetting::startAction11 getParentRequest ERR" , (int)0 );
					break;
				}
				//End【 静的解析対応 : Sky花岡 : 2012.09.03 : CID-439609】.
				pc_PRequest->setCancelFlg( True );

				// 状態更新(サブシナリオ削除要求中).
				pc_Request->e_Request_Status = JSVE_Request_Status_ReqCancel;
				// 結果設定.
				e_Request_Result = JSVE_Request_Result_Ok;
			}
			else
			{
				MyTraceSD(JSVD_DBG_ERROR, "JSVC_Request::startAction11 CancelErr. e_Request_Event:" , ps_NotifyInfo->e_Request_Event );
				e_Request_Result = JSVE_Request_Result_Err;
				pc_Request->setResult(JSVE_JobServiceResult_NG);
				pc_Request->setResultDetail(IPCE_JobsResultDetail_CBNumOver);
				// JobManagerへ要求NGを通知する.
				pc_Request->pc_JobManager->notify( pc_Request, e_Request_Result, JSVE_JobServiceResultDetail_XmlAnalyze, JSVE_JobManager_MsgID_MemoryAllocEnd, 0 );
			}
			break;
		case JSVE_Request_Event_AnsCancel		:	///< サブシナリオ削除応答.
			break;
		case JSVE_Request_Event_AnsEnd			:	///< サブシナリオ終了通知.
			// 正常終了の場合、アプリケーションプラットフォームへ応答をOKで通知する.
			// 異常終了の場合、アプリケーションプラットフォームへ応答をNGで通知する.
			if (JSVE_Request_Result_Ok == ps_NotifyInfo->pReq->getResult()){
				pc_Request->notifyJobService_Answer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Ok, JSVE_JobServiceResultDetail_Non, pc_Request);
			}else{
				pc_Request->notifyJobService_Answer(ps_NotifyInfo->e_Request_Event, e_Request_Result, JSVE_JobServiceResultDetail_Non, pc_Request);
			}
			// 状態更新(サブシナリオ終了中).
			pc_Request->e_Request_Status = JSVE_Request_Status_End;
			// 結果設定.
			e_Request_Result = JSVE_Request_Result_Ok;
			break;
		case JSVE_Request_Event_ReqMemFree		:	///< メモリ開放要求.
			// JobManagerへ要求NGを通知する.
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, e_Request_Result, JSVE_JobServiceResultDetail_Exec, pc_Request);
			break;
		case JSVE_Request_Event_AnsMemFree		:	///< メモリ開放応答.
			break;
		case JSVE_RequestAnswer_Event_ChargeMapPrepared			:	///< 課金マップ展開完了.
			break;
		case JSVE_RequestAnswer_Event_NewInstance				:	///< 待ちうけ部からの着信通知.
			break;
		case JSVE_RequestAnswer_Event_addJob					:	///< ActiveJobListからの状態変化通知(ジョブの追加).
			break;
		case JSVE_RequestAnswer_Event_DocSearcher				:	///< DocSearcherからの状態変化通知.
			break;
		case JSVE_Request_Event_Max				:	///< 最大値(23).
			#ifdef _DEBUG_JSVC_SEQ
			MyTraceSD(JSVD_DBG_ERROR, "最大値:処理なし。startAction11" , 0 );
			#endif
			break;
		default:
			#ifdef _DEBUG_JSVC_SEQ
			MyTraceSD(JSVD_DBG_ERROR, "default。startAction11" , ps_NotifyInfo->e_Request_Event );
			#endif
			break;
	}

	return e_Request_Result;
}
// 情報通知から状態に応じた処理を実行する.
///< サブシナリオ削除要求中.
JSVE_Request_Result JSVC_Request::startAction12( JSVS_NotifyInfo*	ps_NotifyInfo, JSVC_Request* pc_Request )
{
	JSVE_Request_Result e_Request_Result = JSVE_Request_Result_Err;

	switch( ps_NotifyInfo->e_Request_Event )
	{
		case JSVE_Request_Event_Non				:	///< 初期値.
			break;
		case JSVE_Request_Event_ReqMemAlloc		:	///< メモリ確保要求.
			// JobManagerへ要求NGを通知する.
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, e_Request_Result, JSVE_JobServiceResultDetail_AllreadyMemAlloced, pc_Request);
			break;
		case JSVE_Request_Event_AnsAnalyzeCancel:	///< XML解析削除応答.
			break;
		case JSVE_Request_Event_AnsAnalyzeEnd	:	///< XML解析完了応答.
			break;
		case JSVE_Request_Event_AnsMemAllocEnd	:	///< メモリ確保応答.
			break;
		case JSVE_Request_Event_ReqStart		:	///< サブシナリオ開始要求.
			// JobManagerへ要求NGを通知する.
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, e_Request_Result, JSVE_JobServiceResultDetail_Cancel, pc_Request);
			break;
		case JSVE_Request_Event_ReqStop			:	///< サブシナリオ停止要求.
			// JobManagerへ要求NGを通知する.
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, e_Request_Result, JSVE_JobServiceResultDetail_Cancel, pc_Request);
			break;
		case JSVE_Request_Event_AnsStopEnd		:	///< サブシナリオ停止応答.
			// 正常終了の場合、アプリケーションプラットフォームへ応答をOKで通知する.
			// 異常終了の場合、アプリケーションプラットフォームへ応答をNGで通知する.
			if (JSVE_Request_Result_Ok == ps_NotifyInfo->pReq->getResult()){
				pc_Request->notifyJobService_Answer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Ok, JSVE_JobServiceResultDetail_Non, pc_Request);
			}else{
				pc_Request->notifyJobService_Answer(ps_NotifyInfo->e_Request_Event, e_Request_Result, JSVE_JobServiceResultDetail_Non, pc_Request);
			}
			// 結果設定.
			e_Request_Result = JSVE_Request_Result_Ok;
			break;
		case JSVE_Request_Event_ReqUpdate		:	///< サブシナリオ設定変更要求.
			// JobManagerへ要求NGを通知する.
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, e_Request_Result, JSVE_JobServiceResultDetail_Cancel, pc_Request);
			break;
		case JSVE_Request_Event_AnsUpdateEnd	:	///< サブシナリオ設定変更応答.
			break;
		case JSVE_Request_Event_ReqRestart		:	///< サブシナリオ再開要求.
			// JobManagerへ要求NGを通知する.
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, e_Request_Result, JSVE_JobServiceResultDetail_Cancel, pc_Request);
			break;
		case JSVE_Request_Event_AnsRestartEnd	:	///< サブシナリオ再開応答.
			break;
		case JSVE_Request_Event_ReqCancel		:	///< サブシナリオ削除要求.
// CQ511545対応 Start 2015/4/14 Soma.
			// 1ジョブで2回削除することで削除されるジョブが存在するため.
			// 削除要求中の状態ではあるがsystem側へ要求は流すようにする.
			// 自動転送の常時印刷ジョブ等、1ジョブで送信(転送)時の削除後に印刷が行われるケースがある.
			// そのため印刷も削除できるように対応が必要.
			uint32_t ul_index;
			// ジョブキャンセル指示を行う.
			ul_index = JSVC_JobManager::newInstance()->availableAnsFunctionIndex( JSVC_CB_INDEX_ans_cancel, JSVC_CB_MAX_ans_cancel, pc_Request );
			if( ul_index != (uint32_t)-1 )
			{
				// JobManagerへ要求OKを通知する.
				pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Ok, JSVE_JobServiceResultDetail_Non, pc_Request);
				// ジョブキャンセル指示を行う.
				pc_Request->getJobInstance()->req_cancel((void (*)( APIC_Job* ))(JSVC_JobManager::newInstance()->c_CBManage[ul_index].vp_Func));

				// 結果設定.
				e_Request_Result = JSVE_Request_Result_Ok;
			}
			else
			{
				MyTraceSD(JSVD_DBG_ERROR, "JSVC_Request::startAction12 CancelErr. e_Request_Event:" , ps_NotifyInfo->e_Request_Event );
				pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Err, IPCE_JobsResultDetail_CBNumOver, pc_Request);
			}
// CQ511545対応 End 2015/4/14 Soma.
			break;
		case JSVE_Request_Event_AnsCancel		:	///< サブシナリオ削除応答.
			// 正常終了の場合、アプリケーションプラットフォームへ応答をOKで通知する.
			// 異常終了の場合、アプリケーションプラットフォームへ応答をNGで通知する.
			if (JSVE_Request_Result_Ok == ps_NotifyInfo->pReq->getResult()){
				pc_Request->notifyJobService_Answer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Ok, JSVE_JobServiceResultDetail_Non, pc_Request);
			}else{
				pc_Request->notifyJobService_Answer(ps_NotifyInfo->e_Request_Event, e_Request_Result, JSVE_JobServiceResultDetail_Non, pc_Request);
			}
			// 状態更新(サブシナリオ終了中).
			pc_Request->e_Request_Status = JSVE_Request_Status_End;
			// 結果設定.

			/// このときStart/ReStartの要求が残留していればRcvCanEndを返して.
			/// 上位へ返してしまう流れに乗せる。.
			e_Request_Result = JSVE_Request_Result_RcvCanEnd;

			break;
		case JSVE_Request_Event_AnsEnd			:	///< サブシナリオ終了通知.
			{
				uint32_t ulID = pc_Request->getSubScenarioID();
				JSVC_ParentRequest* pc_PRequest = NULL;
				pc_PRequest = JSVC_JobManager::newInstance()->getParentRequest(ulID);
				//Str【 静的解析対応 : Sky花岡 : 2012.09.03 : CID-439610】.
				if( NULL == pc_PRequest ){
					MyTraceSD(JSVD_DBG_ERROR, "JSVC_RequestJobSetting::startAction12 getParentRequest ERR" , (int)0 );
					break;
				}
				//End【 静的解析対応 : Sky花岡 : 2012.09.03 : CID-439610】.
				if( pc_PRequest->getWaitNextJob() ) {
					pc_Request->e_Request_Status = JSVE_Request_Status_WaitNextTrigger;
				} else {
					// 正常終了の場合、アプリケーションプラットフォームへ応答をOKで通知する.
					// 異常終了の場合、アプリケーションプラットフォームへ応答をNGで通知する.
					if (JSVE_Request_Result_Ok == ps_NotifyInfo->pReq->getResult()){
						/// 元々ResultDetailが返るので、ここではIPCE_JobsResultDetail_EndByCanselが	.
						/// きたときにそのまま送信。.
						pc_Request->notifyJobService_Answer(ps_NotifyInfo->e_Request_Event
										, JSVE_Request_Result_Ok,static_cast<JSVE_JobServiceResultDetail>(ps_NotifyInfo->pReq->getResultDetail()) , pc_Request);
					}else{
						pc_Request->notifyJobService_Answer(ps_NotifyInfo->e_Request_Event, e_Request_Result, JSVE_JobServiceResultDetail_Non, pc_Request);
					}
					// 状態更新(サブシナリオ終了中).
					pc_Request->e_Request_Status = JSVE_Request_Status_End;
				}
				// 結果設定.
				e_Request_Result = JSVE_Request_Result_Ok;
			}
			break;
		case JSVE_Request_Event_ReqMemFree		:	///< メモリ開放要求.
			// JobManagerへ要求NGを通知する.
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, e_Request_Result, JSVE_JobServiceResultDetail_Cancel, pc_Request);
			break;
		case JSVE_Request_Event_AnsMemFree		:	///< メモリ開放応答.
			break;
		case JSVE_RequestAnswer_Event_ChargeMapPrepared			:	///< 課金マップ展開完了.
			break;
		case JSVE_RequestAnswer_Event_NewInstance				:	///< 待ちうけ部からの着信通知.
			break;
		case JSVE_RequestAnswer_Event_addJob					:	///< ActiveJobListからの状態変化通知(ジョブの追加).
			break;
		case JSVE_RequestAnswer_Event_DocSearcher				:	///< DocSearcherからの状態変化通知.
			break;
		case JSVE_Request_Event_Max				:	///< 最大値(23).
			#ifdef _DEBUG_JSVC_SEQ
			MyTraceSD(JSVD_DBG_ERROR, "最大値:処理なし。startAction12" , 0 );
			#endif
			break;
		default:
			#ifdef _DEBUG_JSVC_SEQ
			MyTraceSD(JSVD_DBG_ERROR, "default。startAction12" , ps_NotifyInfo->e_Request_Event );
			#endif
			break;
	}

	return e_Request_Result;
}
// 情報通知から状態に応じた処理を実行する.
///< サブシナリオ終了中.
JSVE_Request_Result JSVC_Request::startAction13( JSVS_NotifyInfo*	ps_NotifyInfo, JSVC_Request* pc_Request )
{
	JSVE_Request_Result e_Request_Result = JSVE_Request_Result_Err;

	switch( ps_NotifyInfo->e_Request_Event )
	{
		case JSVE_Request_Event_Non				:	///< 初期値.
			break;
		case JSVE_Request_Event_ReqMemAlloc		:	///< メモリ確保要求.
			// JobManagerへ要求NGを通知する.
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, e_Request_Result, JSVE_JobServiceResultDetail_AllreadyMemAlloced, pc_Request);
			break;
		case JSVE_Request_Event_AnsAnalyzeCancel:	///< XML解析削除応答.
			break;
		case JSVE_Request_Event_AnsAnalyzeEnd	:	///< XML解析完了応答.
			break;
		case JSVE_Request_Event_AnsMemAllocEnd	:	///< メモリ確保応答.
			break;
		case JSVE_Request_Event_ReqStart		:	///< サブシナリオ開始要求.
			// JobManagerへ要求NGを通知する.
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, e_Request_Result, JSVE_JobServiceResultDetail_Exec, pc_Request);
			break;
		case JSVE_Request_Event_ReqStop			:	///< サブシナリオ停止要求.
			// JobManagerへ要求NGを通知する.

			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, e_Request_Result, JSVE_JobServiceResultDetail_End, pc_Request);
			break;
		case JSVE_Request_Event_AnsStopEnd		:	///< サブシナリオ停止応答.
			break;
		case JSVE_Request_Event_ReqUpdate		:	///< サブシナリオ設定変更要求.
			// JobManagerへ要求NGを通知する.
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, e_Request_Result, JSVE_JobServiceResultDetail_End, pc_Request);
			break;
		case JSVE_Request_Event_AnsUpdateEnd	:	///< サブシナリオ設定変更応答.
			break;
		case JSVE_Request_Event_ReqRestart		:	///< サブシナリオ再開要求.
			// JobManagerへ要求NGを通知する.
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, e_Request_Result, JSVE_JobServiceResultDetail_End, pc_Request);
			break;
		case JSVE_Request_Event_AnsRestartEnd	:	///< サブシナリオ再開応答.
			break;
		case JSVE_Request_Event_ReqCancel		:	///< サブシナリオ削除要求.
			// JobManagerへ要求NGを通知する.
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, e_Request_Result, JSVE_JobServiceResultDetail_End, pc_Request);
			break;
		case JSVE_Request_Event_AnsCancel		:	///< サブシナリオ削除応答.
			break;
		case JSVE_Request_Event_AnsEnd			:	///< サブシナリオ終了通知.
			break;
		case JSVE_Request_Event_ReqMemFree		:	///< メモリ開放要求.
			{
				JSVC_ParentRequest* pc_PRequest = NULL;
				pc_PRequest = JSVC_JobManager::newInstance()->getParentRequest(pc_Request->getSubScenarioID());

				//Str【 静的解析対応 : Sky伊藤（勝） : 2012.12.03 : CID-439612/456502】.
				if( NULL == pc_PRequest ){
					MyTraceSD(JSVD_DBG_ERROR, "JSVC_RequestJobSetting::startAction13 getParentRequest ERR" , (int)0 );
					break;
				}
				//End【 静的解析対応 : Sky伊藤（勝） : 2012.12.03 : CID-439612/456502】.

				// 2012/11/12 CQ375076 タイマーFAX予約ジョブ破棄対応 modify start.
				//Str【 静的解析対応 : Sky花岡 : 2012.09.03 : CID-439611】.
				//if( NULL == pc_Request->getJobInstance() ){
				if ((JSVC_TimerRevDelSts_Non == pc_PRequest->getTimerReserveDeleteStatus()) &&
					(NULL == pc_Request->getJobInstance()))
				// 2012/11/12 CQ375076 タイマーFAX予約ジョブ破棄対応 modify end.
				{
					MyTraceSD(JSVD_DBG_ERROR, "JSVC_RequestJobSetting::startAction13 getJobInstance ERR" , (int)0 );
					break;
				}
				//End【 静的解析対応 : Sky花岡 : 2012.09.03 : CID-439611】.

				// 2013/05/07 復旧ジョブシーケンス見直し対応(CQ379669) modify start.
				// JobManagerへ要求OKを通知する.
				pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Ok, JSVE_JobServiceResultDetail_Non, pc_Request);
				// 状態更新(メモリ開放要求中).
				pc_Request->e_Request_Status = JSVE_Request_Status_ReqMemFree;

				// 2012/11/12 CQ375076 タイマーFAX予約ジョブ破棄対応 modify start.
				// EndProcess未実行の場合はジョブへのメモリ解放、実行済なら上位への返信のみが必要.
				if ((JSVC_TimerRevDelSts_Non == pc_PRequest->getTimerReserveDeleteStatus()) &&
					(True == pc_Request->getJobInstance()->getJobSpec()->pc_Common->getEndProcessFlg()))
				{
					// EndProcessに起動をかける.
					pc_Request->getJobInstance()->getJobSpec()->pc_Common->setEndProcessFlg(False);			//EndプロセスフラグをFalseへ.
					pc_Request->getJobInstance()->restart();
				} else {
				// 2012/11/12 CQ375076 タイマーFAX予約ジョブ破棄対応 modify end.
					JSVE_FunctionStatus e_FunctionStatus = JSVE_FunctionStatus_Non;
					e_FunctionStatus = pc_PRequest->getFunctionStatus(TYPE_FunctionNumber_FAXCOMMONSETTING);
					if ((JSVE_FunctionStatus_Non  != e_FunctionStatus) &&
						(JSVE_FunctionStatus_End  != e_FunctionStatus))
					{
						// EndProcess(TYPE_FunctionNumber_FAXCOMMONSETTING)の実行状態を確認し、実行中の場合は、要求応答通知のみを行う.
					} else {
						// すでにEndProcessが終了しているので完了通知を続けて行う.

						// 内部Msg：メモリ開放完了通知を発行する.
						JSVC_ResponseJobNotify* pc_ResJobNotify = NULL;
						JSVS_ResponseParamJobNotify st_ResJobNotify;
						memset( &st_ResJobNotify, 0x00, sizeof(st_ResJobNotify) );
						JSVS_SendRecv_MsgParam s_MsgParam;
						memset( &s_MsgParam, 0x00, sizeof(s_MsgParam) );

						st_ResJobNotify.ul_SubScenarioID	= pc_Request->getSubScenarioID();
						st_ResJobNotify.pJob				= NULL;
						st_ResJobNotify.ul_Result			= JSVE_Request_Result_Ok;
						st_ResJobNotify.ul_ResultDetail		= JSVE_JobServiceResultDetail_Non;
						st_ResJobNotify.ul_FuncSts			= 0;
						st_ResJobNotify.ul_AcceptNum		= 0;
						st_ResJobNotify.b_Continue			= 0;
						pc_ResJobNotify = new JSVC_ResponseJobNotify( JSVE_JobManager_MsgID_MemoryDeleteEnd );
						#if DEBUGLOG__REQUEST_LIFE
							MyTraceSLD(JSVD_DBG_INFO, "startAction13 new JSVC_ResponseJobNotify" , (intptr_t)pc_ResJobNotify );
						#endif
						if( pc_ResJobNotify != NULL ){
							pc_ResJobNotify->setData( (const char*)&st_ResJobNotify, sizeof(st_ResJobNotify) );
							s_MsgParam.ul_MsgTypeID			= JSVE_JobManager_MsgID_MemoryDeleteEnd;
							s_MsgParam.pRes					= (JSVC_ResponseBase*)pc_ResJobNotify;
							JSVC_JobManager::newInstance()->sendMessageToJob( &s_MsgParam );

							// メモリ確保完了通知後は、通知済みフラグをセットする.
							pc_PRequest->setEndNotice( JSVE_EndNoticeBit_MemDeleteEnd, True );
						}
					}
				}
				// 2013/05/07 復旧ジョブシーケンス見直し対応(CQ379669) modify end.

				// 結果設定.
				e_Request_Result = JSVE_Request_Result_Ok;
			}
			break;
		case JSVE_Request_Event_AnsMemFree		:	///< メモリ開放応答.
			break;
		case JSVE_RequestAnswer_Event_ChargeMapPrepared			:	///< 課金マップ展開完了.
			break;
		case JSVE_RequestAnswer_Event_NewInstance				:	///< 待ちうけ部からの着信通知.
			break;
		case JSVE_RequestAnswer_Event_addJob					:	///< ActiveJobListからの状態変化通知(ジョブの追加).
			break;
		case JSVE_RequestAnswer_Event_DocSearcher				:	///< DocSearcherからの状態変化通知.
			break;
		case JSVE_Request_Event_Max				:	///< 最大値(23).
			#ifdef _DEBUG_JSVC_SEQ
			MyTraceSD(JSVD_DBG_ERROR, "最大値:処理なし。startAction13" , 0 );
			#endif
			break;
		default:
			#ifdef _DEBUG_JSVC_SEQ
			MyTraceSD(JSVD_DBG_ERROR, "default。startAction13" , ps_NotifyInfo->e_Request_Event );
			#endif
			break;
	}

	return e_Request_Result;
}
// 情報通知から状態に応じた処理を実行する.
///< メモリ開放要求中.
JSVE_Request_Result JSVC_Request::startAction14( JSVS_NotifyInfo*	ps_NotifyInfo, JSVC_Request* pc_Request )
{
	JSVE_Request_Result e_Request_Result = JSVE_Request_Result_Err;

	switch( ps_NotifyInfo->e_Request_Event )
	{
		case JSVE_Request_Event_Non				:	///< 初期値.
			break;
		case JSVE_Request_Event_ReqMemAlloc		:	///< メモリ確保要求.
			// JobManagerへ要求NGを通知する.
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, e_Request_Result, JSVE_JobServiceResultDetail_MemFree, pc_Request);
			break;
		case JSVE_Request_Event_AnsAnalyzeCancel:	///< XML解析削除応答.
			break;
		case JSVE_Request_Event_AnsAnalyzeEnd	:	///< XML解析完了応答.
			break;
		case JSVE_Request_Event_AnsMemAllocEnd	:	///< メモリ確保応答.
			break;
		case JSVE_Request_Event_ReqStart		:	///< サブシナリオ開始要求.
			// JobManagerへ要求NGを通知する.
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, e_Request_Result, JSVE_JobServiceResultDetail_MemFree, pc_Request);
			break;
		case JSVE_Request_Event_ReqStop			:	///< サブシナリオ停止要求.
			// JobManagerへ要求NGを通知する.
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, e_Request_Result, JSVE_JobServiceResultDetail_MemFree, pc_Request);
			break;
		case JSVE_Request_Event_AnsStopEnd		:	///< サブシナリオ停止応答.
			break;
		case JSVE_Request_Event_ReqUpdate		:	///< サブシナリオ設定変更要求.
			// JobManagerへ要求NGを通知する.
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, e_Request_Result, JSVE_JobServiceResultDetail_MemFree, pc_Request);
			break;
		case JSVE_Request_Event_AnsUpdateEnd	:	///< サブシナリオ設定変更応答.
			break;
		case JSVE_Request_Event_ReqRestart		:	///< サブシナリオ再開要求.
			// JobManagerへ要求NGを通知する.
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, e_Request_Result, JSVE_JobServiceResultDetail_MemFree, pc_Request);
			break;
		case JSVE_Request_Event_AnsRestartEnd	:	///< サブシナリオ再開応答.
			break;
		case JSVE_Request_Event_ReqCancel		:	///< サブシナリオ削除要求.
// OP_BTS-11197 ボックスへの文書保存ジョブを読み込み完了後、保存中に削除できない対応 2019/8/8 m.ooba start.
			uint32_t ul_index;
			// ジョブキャンセル指示を行う.
			ul_index = JSVC_JobManager::newInstance()->availableAnsFunctionIndex( JSVC_CB_INDEX_ans_cancel, JSVC_CB_MAX_ans_cancel, pc_Request );
			if( ul_index != (uint32_t)-1 )
			{
				// JobManagerへ要求OKを通知する.
				pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Ok, JSVE_JobServiceResultDetail_Non, pc_Request);
				// ジョブキャンセル指示を行う.
				pc_Request->getJobInstance()->req_cancel((void (*)( APIC_Job* ))(JSVC_JobManager::newInstance()->c_CBManage[ul_index].vp_Func));

				// 結果設定.
				e_Request_Result = JSVE_Request_Result_Ok;
			}
			else
			{
				MyTraceSD(JSVD_DBG_ERROR, "JSVC_Request::startAction14 CancelErr. e_Request_Event:" , ps_NotifyInfo->e_Request_Event );
				pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Err, IPCE_JobsResultDetail_CBNumOver, pc_Request);
			}
// OP_BTS-11197 ボックスへの文書保存ジョブを読み込み完了後、保存中に削除できない対応 2019/8/8 m.ooba end.
			break;
		case JSVE_Request_Event_AnsCancel		:	///< サブシナリオ削除応答.
			break;
		case JSVE_Request_Event_AnsEnd			:	///< サブシナリオ終了通知.
			break;
		case JSVE_Request_Event_ReqMemFree		:	///< メモリ開放要求.
			// JobManagerへ要求NGを通知する.
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, e_Request_Result, JSVE_JobServiceResultDetail_MemFree, pc_Request);
			break;
		case JSVE_Request_Event_AnsMemFree		:	///< メモリ開放応答.
			// 正常終了の場合、アプリケーションプラットフォームへ応答をOKで通知する.
			// 異常終了の場合、アプリケーションプラットフォームへ応答をNGで通知する.
			if (JSVE_Request_Result_Ok == ps_NotifyInfo->pRes->getResult()){
				pc_Request->notifyJobService_Answer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Ok, JSVE_JobServiceResultDetail_Non, pc_Request);
			}else{
				pc_Request->notifyJobService_Answer(ps_NotifyInfo->e_Request_Event, e_Request_Result, JSVE_JobServiceResultDetail_Non, pc_Request);
			}
			// 状態更新(初期値).
			pc_Request->e_Request_Status = JSVE_Request_Status_Non;
			// 結果設定.
			e_Request_Result = JSVE_Request_Result_Ok;
			break;
		case JSVE_RequestAnswer_Event_ChargeMapPrepared			:	///< 課金マップ展開完了.
			break;
		case JSVE_RequestAnswer_Event_NewInstance				:	///< 待ちうけ部からの着信通知.
			break;
		case JSVE_RequestAnswer_Event_addJob					:	///< ActiveJobListからの状態変化通知(ジョブの追加).
			break;
		case JSVE_RequestAnswer_Event_DocSearcher				:	///< DocSearcherからの状態変化通知.
			break;
		case JSVE_Request_Event_Max				:	///< 最大値(23).
			#ifdef _DEBUG_JSVC_SEQ
			MyTraceSD(JSVD_DBG_ERROR, "最大値:処理なし。startAction14" , 0 );
			#endif
			break;
		default:
			#ifdef _DEBUG_JSVC_SEQ
			MyTraceSD(JSVD_DBG_ERROR, "default。startAction14" , ps_NotifyInfo->e_Request_Event );
			#endif
			break;
	}

	return e_Request_Result;
}
// 情報通知から状態に応じた処理を実行する.
// エラー停止によるジョブキャンセル待ち.
JSVE_Request_Result JSVC_Request::startAction15( JSVS_NotifyInfo*	ps_NotifyInfo, JSVC_Request* pc_Request )
{
	JSVE_Request_Result e_Request_Result = JSVE_Request_Result_Err;
#ifdef _DEBUG_JSVC_SEQ
	MyTraceSLD(JSVD_DBG_INFO, "JSVC_Request::startAction15 ps_NotifyInfo:" , (intptr_t)ps_NotifyInfo );	// 回避方法をトレースに変更.
	MyTraceSLD(JSVD_DBG_INFO, "JSVC_Request::startAction15 pc_Request:" , (intptr_t)pc_Request );		// 回避方法をトレースに変更.
#endif

	// 処理なし.

	return e_Request_Result;
}
// 情報通知から状態に応じた処理を実行する.
// 受信開始待ち.
JSVE_Request_Result JSVC_Request::startAction16( JSVS_NotifyInfo*	ps_NotifyInfo, JSVC_Request* pc_Request )
{
	JSVE_Request_Result e_Request_Result = JSVE_Request_Result_Err;

	switch( ps_NotifyInfo->e_Request_Event )
	{
		case JSVE_Request_Event_Non				:	///< 初期値.
			break;
		case JSVE_Request_Event_ReqMemAlloc		:	///< メモリ確保要求.
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, e_Request_Result, JSVE_JobServiceResultDetail_AllreadyMemAlloced, pc_Request);
			break;
		case JSVE_Request_Event_AnsAnalyzeCancel:	///< XML解析削除応答.
			break;
		case JSVE_Request_Event_AnsAnalyzeEnd	:	///< XML解析完了応答.
			break;
		case JSVE_Request_Event_AnsMemAllocEnd	:	///< メモリ確保応答.
			break;
		case JSVE_Request_Event_ReqStart		:	///< サブシナリオ開始要求.
			{
				uint32_t ulID = pc_Request->getSubScenarioID();
				JSVC_ParentRequest* pc_PRequest = NULL;
				pc_PRequest = JSVC_JobManager::newInstance()->getParentRequest(ulID);
				//Str【 静的解析対応 : Sky花岡 : 2012.09.03 : CID-439613】.
				if( NULL == pc_PRequest ){
					MyTraceSD(JSVD_DBG_ERROR, "JSVC_RequestJobSetting::startAction16 getParentRequest ERR" , (int)0 );
					break;
				}
				//End【 静的解析対応 : Sky花岡 : 2012.09.03 : CID-439613】.
				if( False == pc_PRequest->getCancelFlg() ) {
					pc_Request->e_Request_Status = JSVE_Request_Status_RcvCall;
					pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Ok, JSVE_JobServiceResultDetail_Non, pc_Request);
				} else {
					// ジョブインスタンスの削除.
// CQ:366448対応 Sky_iwasaki 2012.06.15 Start.
//					pc_Request->getJobInstance()->deleteInstance();
// CQ:366448対応 Sky_iwasaki 2012.06.15 End.
					pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Err, JSVE_JobServiceResultDetail_Cancel, pc_Request);
					e_Request_Result = JSVE_Request_Result_RecCanRes;	//キャンセル後のキャンセルリクエスト削除をManagerへ依頼.
				}
			}
			break;
		case JSVE_Request_Event_ReqStop			:	///< サブシナリオ停止要求.
			e_Request_Result = JSVE_Request_Result_Ok;	//notifyJobService_ReqAnswerの後でMSG削除するため.
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Err, JSVE_JobServiceResultDetail_RecvStartWait, pc_Request);
			break;
		case JSVE_Request_Event_AnsStopEnd		:	///< サブシナリオ停止応答.
			break;
		case JSVE_Request_Event_ReqUpdate		:	///< サブシナリオ設定変更要求.
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, e_Request_Result, JSVE_JobServiceResultDetail_RecvStartWait, pc_Request);
			break;
		case JSVE_Request_Event_AnsUpdateEnd	:	///< サブシナリオ設定変更応答.
			break;
		case JSVE_Request_Event_ReqRestart		:	///< サブシナリオ再開要求.
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, e_Request_Result, JSVE_JobServiceResultDetail_RecvStartWait, pc_Request);
			break;
		case JSVE_Request_Event_AnsRestartEnd	:	///< サブシナリオ再開応答.
			break;
		case JSVE_Request_Event_ReqCancel		:	///< サブシナリオ削除要求.
			{
				uint32_t ulID = pc_Request->getSubScenarioID();
				JSVC_ParentRequest* pc_PRequest = NULL;
				pc_PRequest = JSVC_JobManager::newInstance()->getParentRequest(ulID);
				//Str【 静的解析対応 : Sky花岡 : 2012.09.03 : CID-439613】.
				if( NULL == pc_PRequest ){
					MyTraceSD(JSVD_DBG_ERROR, "JSVC_RequestJobSetting::startAction16 getParentRequest ERR" , (int)0 );
					break;
				}
				//End【 静的解析対応 : Sky花岡 : 2012.09.03 : CID-439613】.
				pc_PRequest->setCancelFlg( True );
				pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Ok, JSVE_JobServiceResultDetail_Cancel, pc_Request);
			}
			break;
		case JSVE_Request_Event_AnsCancel		:	///< サブシナリオ削除応答.
			break;
		case JSVE_Request_Event_AnsEnd			:	///< サブシナリオ終了通知.
			break;
		case JSVE_Request_Event_ReqMemFree		:	///< メモリ開放要求.
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, e_Request_Result, JSVE_JobServiceResultDetail_RecvStartWait, pc_Request);
			break;
		case JSVE_Request_Event_AnsMemFree		:	///< メモリ開放応答.
			break;
		case JSVE_RequestAnswer_Event_ChargeMapPrepared			:	///< 課金マップ展開完了.
			break;
		case JSVE_RequestAnswer_Event_NewInstance				:	///< 待ちうけ部からの着信通知.
			break;
		case JSVE_RequestAnswer_Event_addJob					:	///< ActiveJobListからの状態変化通知(ジョブの追加).
			break;
		case JSVE_RequestAnswer_Event_DocSearcher				:	///< DocSearcherからの状態変化通知.
			break;
		case JSVE_Request_Event_Max				:	///< 最大値(23).
			#ifdef _DEBUG_JSVC_SEQ
			MyTraceSD(JSVD_DBG_INFO, "最大値:処理なし。startAction16" , 0 );
			#endif
			break;
		default:
			#ifdef _DEBUG_JSVC_SEQ
			MyTraceSD(JSVD_DBG_INFO, "default。startAction16" , ps_NotifyInfo->e_Request_Event );
			#endif
			break;
	}

	return e_Request_Result;
}
// 情報通知から状態に応じた処理を実行する.
// 受信準備中.
JSVE_Request_Result JSVC_Request::startAction17( JSVS_NotifyInfo*	ps_NotifyInfo, JSVC_Request* pc_Request )
{
	JSVE_Request_Result e_Request_Result = JSVE_Request_Result_Err;
#ifdef _DEBUG_JSVC_SEQ
	MyTraceSLD(JSVD_DBG_INFO, "JSVC_Request::startAction17 pc_Request:" , (intptr_t)pc_Request );		// 回避方法をトレースに変更.
#endif

	switch( ps_NotifyInfo->e_Request_Event )
	{
		case JSVE_Request_Event_Non				:	///< 初期値.
			break;
		case JSVE_Request_Event_ReqMemAlloc		:	///< メモリ確保要求.
			break;
		case JSVE_Request_Event_AnsAnalyzeCancel:	///< XML解析削除応答.
			break;
		case JSVE_Request_Event_AnsAnalyzeEnd	:	///< XML解析完了応答.
			break;
		case JSVE_Request_Event_AnsMemAllocEnd	:	///< メモリ確保応答.
			break;
		case JSVE_Request_Event_ReqStart		:	///< サブシナリオ開始要求.
			break;
		case JSVE_Request_Event_ReqStop			:	///< サブシナリオ停止要求.
			break;
		case JSVE_Request_Event_AnsStopEnd		:	///< サブシナリオ停止応答.
			break;
		case JSVE_Request_Event_ReqUpdate		:	///< サブシナリオ設定変更要求.
			break;
		case JSVE_Request_Event_AnsUpdateEnd	:	///< サブシナリオ設定変更応答.
			break;
		case JSVE_Request_Event_ReqRestart		:	///< サブシナリオ再開要求.
			break;
		case JSVE_Request_Event_AnsRestartEnd	:	///< サブシナリオ再開応答.
			break;
		case JSVE_Request_Event_ReqCancel		:	///< サブシナリオ削除要求.
			break;
		case JSVE_Request_Event_AnsCancel		:	///< サブシナリオ削除応答.
			break;
		case JSVE_Request_Event_AnsEnd			:	///< サブシナリオ終了通知.
			break;
		case JSVE_Request_Event_ReqMemFree		:	///< メモリ開放要求.
			break;
		case JSVE_Request_Event_AnsMemFree		:	///< メモリ開放応答.
			break;
		case JSVE_RequestAnswer_Event_ChargeMapPrepared			:	///< 課金マップ展開完了.
			break;
		case JSVE_RequestAnswer_Event_NewInstance				:	///< 待ちうけ部からの着信通知.
			break;
		case JSVE_RequestAnswer_Event_addJob					:	///< ActiveJobListからの状態変化通知(ジョブの追加).
			break;
		case JSVE_RequestAnswer_Event_DocSearcher				:	///< DocSearcherからの状態変化通知.
			break;
		case JSVE_Request_Event_Max				:	///< 最大値(23).
			#ifdef _DEBUG_JSVC_SEQ
			MyTraceSD(JSVD_DBG_INFO, "最大値:処理なし。startAction17" , 0 );
			#endif
			break;
		default:
			#ifdef _DEBUG_JSVC_SEQ
			MyTraceSD(JSVD_DBG_INFO, "default。startAction17" , ps_NotifyInfo->e_Request_Event );
			#endif
			break;
	}

	return e_Request_Result;
}
// 情報通知から状態に応じた処理を実行する.
// 着信待ち.
JSVE_Request_Result JSVC_Request::startAction18( JSVS_NotifyInfo*	ps_NotifyInfo, JSVC_Request* pc_Request )
{
	JSVE_Request_Result e_Request_Result = JSVE_Request_Result_Err;

	switch( ps_NotifyInfo->e_Request_Event )
	{
		case JSVE_Request_Event_Non				:	///< 初期値.
			break;
		case JSVE_Request_Event_ReqMemAlloc		:	///< メモリ確保要求.
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, e_Request_Result, JSVE_JobServiceResultDetail_AllreadyMemAlloced, pc_Request);
			break;
		case JSVE_Request_Event_AnsAnalyzeCancel:	///< XML解析削除応答.
			break;
		case JSVE_Request_Event_AnsAnalyzeEnd	:	///< XML解析完了応答.
			break;
		case JSVE_Request_Event_AnsMemAllocEnd	:	///< メモリ確保応答.
			break;
		case JSVE_Request_Event_ReqStart		:	///< サブシナリオ開始要求.
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, e_Request_Result, JSVE_JobServiceResultDetail_Exec, pc_Request);
			break;
		case JSVE_Request_Event_ReqStop			:	///< サブシナリオ停止要求.
			{
				uint32_t ulID = pc_Request->getSubScenarioID();
				JSVC_ParentRequest* pc_PRequest = NULL;
				pc_PRequest = JSVC_JobManager::newInstance()->getParentRequest(ulID);
				//Str【 静的解析対応 : Sky花岡 : 2012.09.03 : CID-439614】.
				if( NULL == pc_PRequest ){
					MyTraceSD(JSVD_DBG_ERROR, "JSVC_RequestJobSetting::startAction18 getParentRequest ERR" , (int)0 );
					break;
				}
				//End【 静的解析対応 : Sky花岡 : 2012.09.03 : CID-439614】.
				e_Request_Result = JSVE_Request_Result_Ok;
				if( False == pc_PRequest->getStopFlg() ) {
					pc_PRequest->setStopFlg( True );
					pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Ok, JSVE_JobServiceResultDetail_Non, pc_Request);
				} else {
					pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Err, JSVE_JobServiceResultDetail_Stop, pc_Request);
				}
			}
			break;
		case JSVE_Request_Event_AnsStopEnd		:	///< サブシナリオ停止応答.
			break;
		case JSVE_Request_Event_ReqUpdate		:	///< サブシナリオ設定変更要求.
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, e_Request_Result, JSVE_JobServiceResultDetail_RecvWait, pc_Request);
			break;
		case JSVE_Request_Event_AnsUpdateEnd	:	///< サブシナリオ設定変更応答.
			break;
		case JSVE_Request_Event_ReqRestart		:	///< サブシナリオ再開要求.
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, e_Request_Result, JSVE_JobServiceResultDetail_RecvWait, pc_Request);
			break;
		case JSVE_Request_Event_AnsRestartEnd	:	///< サブシナリオ再開応答.
			break;
		case JSVE_Request_Event_ReqCancel		:	///< サブシナリオ削除要求.
			{
				uint32_t ulID = pc_Request->getSubScenarioID();
				JSVC_ParentRequest* pc_PRequest = NULL;
				pc_PRequest = JSVC_JobManager::newInstance()->getParentRequest(ulID);
				//Str【 静的解析対応 : Sky花岡 : 2012.09.03 : CID-439614】.
				if( NULL == pc_PRequest ){
					MyTraceSD(JSVD_DBG_ERROR, "JSVC_RequestJobSetting::startAction18 getParentRequest ERR" , (int)0 );
					break;
				}
				//End【 静的解析対応 : Sky花岡 : 2012.09.03 : CID-439614】.
				if( False == pc_PRequest->getCancelFlg() ) {
					pc_PRequest->setCancelFlg( True );
					pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Ok, JSVE_JobServiceResultDetail_Cancel, pc_Request);
				} else {
					pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Err, JSVE_JobServiceResultDetail_Cancel, pc_Request);
				}
			}
			break;
		case JSVE_Request_Event_AnsCancel		:	///< サブシナリオ削除応答.
			break;
		case JSVE_Request_Event_AnsEnd			:	///< サブシナリオ終了通知.
			break;
		case JSVE_Request_Event_ReqMemFree		:	///< メモリ開放要求.
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, e_Request_Result, JSVE_JobServiceResultDetail_RecvWait, pc_Request);
			break;
		case JSVE_Request_Event_AnsMemFree		:	///< メモリ開放応答.
			break;
		case JSVE_RequestAnswer_Event_ChargeMapPrepared			:	///< 課金マップ展開完了.
			break;
		case JSVE_RequestAnswer_Event_NewInstance				:	///< 待ちうけ部からの着信通知.
			MyTraceSS(JSVD_DBG_REQANS, "MSG受信","JSVE_RequestAnswer_Event_NewInstance")
			pc_Request->e_Request_Status = JSVE_Request_Status_AnalizeHeader;
			break;
		case JSVE_RequestAnswer_Event_addJob					:	///< ActiveJobListからの状態変化通知(ジョブの追加).
			break;
		case JSVE_RequestAnswer_Event_DocSearcher				:	///< DocSearcherからの状態変化通知.
			break;
		case JSVE_Request_Event_Max				:	///< 最大値(23).
			#ifdef _DEBUG_JSVC_SEQ
			MyTraceSD(JSVD_DBG_ERROR, "最大値:処理なし。startAction18" , 0 );
			#endif
			break;
		default:
			#ifdef _DEBUG_JSVC_SEQ
			MyTraceSD(JSVD_DBG_ERROR, "default。startAction18" , ps_NotifyInfo->e_Request_Event );
			#endif
			break;
	}

	return e_Request_Result;
}
// 情報通知から状態に応じた処理を実行する.
// ヘッダ受信中.
JSVE_Request_Result JSVC_Request::startAction19( JSVS_NotifyInfo*	ps_NotifyInfo, JSVC_Request* pc_Request )
{
	JSVE_Request_Result e_Request_Result = JSVE_Request_Result_Err;

	switch( ps_NotifyInfo->e_Request_Event )
	{
		case JSVE_Request_Event_Non				:	///< 初期値.
			break;
		case JSVE_Request_Event_ReqMemAlloc		:	///< メモリ確保要求.
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, e_Request_Result, JSVE_JobServiceResultDetail_AllreadyMemAlloced, pc_Request);
			break;
		case JSVE_Request_Event_AnsAnalyzeCancel:	///< XML解析削除応答.
			break;
		case JSVE_Request_Event_AnsAnalyzeEnd	:	///< XML解析完了応答.
			break;
		case JSVE_Request_Event_AnsMemAllocEnd	:	///< メモリ確保応答.
			break;
		case JSVE_Request_Event_ReqStart		:	///< サブシナリオ開始要求.
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, e_Request_Result, JSVE_JobServiceResultDetail_Exec, pc_Request);
			break;
		case JSVE_Request_Event_ReqStop			:	///< サブシナリオ停止要求.
			{
				uint32_t ulID = pc_Request->getSubScenarioID();
				JSVC_ParentRequest* pc_PRequest = NULL;
				pc_PRequest = JSVC_JobManager::newInstance()->getParentRequest(ulID);
				//Str【 静的解析対応 : Sky花岡 : 2012.09.03 : CID-439616】.
				if( NULL == pc_PRequest ){
					MyTraceSD(JSVD_DBG_ERROR, "JSVC_RequestJobSetting::startAction19 getParentRequest ERR" , (int)0 );
					break;
				}
				//End【 静的解析対応 : Sky花岡 : 2012.09.03 : CID-439616】.
				if( False == pc_PRequest->getStopFlg() ) {
					pc_PRequest->setStopFlg( True );
					pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Ok, JSVE_JobServiceResultDetail_Stop, pc_Request);
				} else {
					pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Err, JSVE_JobServiceResultDetail_Stop, pc_Request);
				}
			}
			break;
		case JSVE_Request_Event_AnsStopEnd		:	///< サブシナリオ停止応答.
			break;
		case JSVE_Request_Event_ReqUpdate		:	///< サブシナリオ設定変更要求.
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, e_Request_Result, JSVE_JobServiceResultDetail_HeadderRecv, pc_Request);
			break;
		case JSVE_Request_Event_AnsUpdateEnd	:	///< サブシナリオ設定変更応答.
			break;
		case JSVE_Request_Event_ReqRestart		:	///< サブシナリオ再開要求.
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, e_Request_Result, JSVE_JobServiceResultDetail_HeadderRecv, pc_Request);
			break;
		case JSVE_Request_Event_AnsRestartEnd	:	///< サブシナリオ再開応答.
			break;
		case JSVE_Request_Event_ReqCancel		:	///< サブシナリオ削除要求.
			{
				uint32_t ulID = pc_Request->getSubScenarioID();
				JSVC_ParentRequest* pc_PRequest = NULL;
				pc_PRequest = JSVC_JobManager::newInstance()->getParentRequest(ulID);
				//Str【 静的解析対応 : Sky花岡 : 2012.09.03 : CID-439615】.
				if( NULL == pc_PRequest ){
					MyTraceSD(JSVD_DBG_ERROR, "JSVC_RequestJobSetting::startAction19 getParentRequest ERR" , (int)0 );
					break;
				}
				//End【 静的解析対応 : Sky花岡 : 2012.09.03 : CID-439615】.
				if( False == pc_PRequest->getCancelFlg() ) {
					pc_PRequest->setCancelFlg( True );
					pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Ok, JSVE_JobServiceResultDetail_Cancel, pc_Request);
				} else {
					pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Err, JSVE_JobServiceResultDetail_Cancel, pc_Request);
				}
			}
			break;
		case JSVE_Request_Event_AnsCancel		:	///< サブシナリオ削除応答.
			break;
		case JSVE_Request_Event_AnsEnd			:	///< サブシナリオ終了通知.
			break;
		case JSVE_Request_Event_ReqMemFree		:	///< メモリ開放要求.
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, e_Request_Result, JSVE_JobServiceResultDetail_HeadderRecv, pc_Request);
			break;
		case JSVE_Request_Event_AnsMemFree		:	///< メモリ開放応答.
			break;
		case JSVE_RequestAnswer_Event_ChargeMapPrepared			:	///< 課金マップ展開完了.
			break;
		case JSVE_RequestAnswer_Event_NewInstance				:	///< 待ちうけ部からの着信通知.
			break;
		case JSVE_RequestAnswer_Event_addJob					:	///< ActiveJobListからの状態変化通知(ジョブの追加).
			{
				uint32_t ulID = pc_Request->getSubScenarioID();
				JSVC_ParentRequest* pc_PRequest = NULL;
				pc_PRequest = JSVC_JobManager::newInstance()->getParentRequest(ulID);
				//Str【 静的解析対応 : Sky花岡 : 2012.09.03 : CID-439615】.
				if( NULL == pc_PRequest ){
					MyTraceSD(JSVD_DBG_ERROR, "JSVC_RequestJobSetting::startAction19 getParentRequest ERR" , (int)0 );
					break;
				}
				//End【 静的解析対応 : Sky花岡 : 2012.09.03 : CID-439615】.
				if( False == pc_PRequest->getCancelFlg() ) {
					if( False == pc_PRequest->getStopFlg() ) {
						pc_Request->e_Request_Status = JSVE_Request_Status_Exec;
					} else {
						// ジョブ停止指示を行う.
						APIC_ActiveJobList::newInstance()->req_stop(ans_stop);
						pc_Request->e_Request_Status = JSVE_Request_Status_ReqStop;
					}
				} else {
					int32_t sl_index;
					// ジョブキャンセル指示を行う.
					sl_index = JSVC_JobManager::newInstance()->availableAnsFunctionIndex( JSVC_CB_INDEX_ans_cancel, JSVC_CB_MAX_ans_cancel, pc_Request );
					if( sl_index != -1 )
					{
						//Str【 静的解析対応 : Sky花岡 : 2012.09.04 : CID-439615】.
						if( NULL == pc_Request->getJobInstance() ){
							MyTraceSD(JSVD_DBG_ERROR, "JSVC_RequestJobSetting::startAction19 getParentRequest ERR" , (int)0 );
							break;
						}
						//End【 静的解析対応 : Sky花岡 : 2012.09.04 : CID-439615】.
						// ジョブキャンセル指示を行う.
						pc_Request->getJobInstance()->req_cancel((void (*)( APIC_Job* ))(JSVC_JobManager::newInstance()->c_CBManage[sl_index].vp_Func));

						pc_PRequest->setCancelFlg( True );

						pc_Request->e_Request_Status = JSVE_Request_Status_ReqCancel;
					}
					else
					{
						MyTraceSD(JSVD_DBG_ERROR, "JSVC_Request::startAction19 CancelErr. e_Request_Event:" , ps_NotifyInfo->e_Request_Event );
						e_Request_Result = JSVE_Request_Result_Err;
						pc_Request->setResult(JSVE_JobServiceResult_NG);
						pc_Request->setResultDetail(IPCE_JobsResultDetail_CBNumOver);
						// JobManagerへ要求NGを通知する.
						pc_Request->pc_JobManager->notify( pc_Request, e_Request_Result, JSVE_JobServiceResultDetail_XmlAnalyze, JSVE_JobManager_MsgID_MemoryAllocEnd, 0 );
					}
				}
			}
			break;
		case JSVE_RequestAnswer_Event_DocSearcher				:	///< DocSearcherからの状態変化通知.
			break;
		case JSVE_Request_Event_Max				:	///< 最大値(23).
			#ifdef _DEBUG_JSVC_SEQ
			MyTraceSD(JSVD_DBG_ERROR, "最大値:処理なし。startAction19" , 0 );
			#endif
			break;
		default:
			#ifdef _DEBUG_JSVC_SEQ
			MyTraceSD(JSVD_DBG_ERROR, "default。startAction19" , ps_NotifyInfo->e_Request_Event );
			#endif
			break;
	}

	return e_Request_Result;
}
// 情報通知から状態に応じた処理を実行する.
// 停止準備中.
JSVE_Request_Result JSVC_Request::startAction20( JSVS_NotifyInfo*	ps_NotifyInfo, JSVC_Request* pc_Request )
{
	JSVE_Request_Result e_Request_Result = JSVE_Request_Result_Err;
#ifdef _DEBUG_JSVC_SEQ
	MyTraceSLD(JSVD_DBG_INFO, "JSVC_Request::startAction20 pc_Request:" , (intptr_t)pc_Request );		// 回避方法をトレースに変更.
#endif

	switch( ps_NotifyInfo->e_Request_Event )
	{
		case JSVE_Request_Event_Non				:	///< 初期値.
			break;
		case JSVE_Request_Event_ReqMemAlloc		:	///< メモリ確保要求.
			break;
		case JSVE_Request_Event_AnsAnalyzeCancel:	///< XML解析削除応答.
			break;
		case JSVE_Request_Event_AnsAnalyzeEnd	:	///< XML解析完了応答.
			break;
		case JSVE_Request_Event_AnsMemAllocEnd	:	///< メモリ確保応答.
			break;
		case JSVE_Request_Event_ReqStart		:	///< サブシナリオ開始要求.
			break;
		case JSVE_Request_Event_ReqStop			:	///< サブシナリオ停止要求.
			break;
		case JSVE_Request_Event_AnsStopEnd		:	///< サブシナリオ停止応答.
			break;
		case JSVE_Request_Event_ReqUpdate		:	///< サブシナリオ設定変更要求.
			break;
		case JSVE_Request_Event_AnsUpdateEnd	:	///< サブシナリオ設定変更応答.
			break;
		case JSVE_Request_Event_ReqRestart		:	///< サブシナリオ再開要求.
			break;
		case JSVE_Request_Event_AnsRestartEnd	:	///< サブシナリオ再開応答.
			break;
		case JSVE_Request_Event_ReqCancel		:	///< サブシナリオ削除要求.
			break;
		case JSVE_Request_Event_AnsCancel		:	///< サブシナリオ削除応答.
			break;
		case JSVE_Request_Event_AnsEnd			:	///< サブシナリオ終了通知.
			break;
		case JSVE_Request_Event_ReqMemFree		:	///< メモリ開放要求.
			break;
		case JSVE_Request_Event_AnsMemFree		:	///< メモリ開放応答.
			break;
		case JSVE_RequestAnswer_Event_ChargeMapPrepared			:	///< 課金マップ展開完了.
			break;
		case JSVE_RequestAnswer_Event_NewInstance				:	///< 待ちうけ部からの着信通知.
			break;
		case JSVE_RequestAnswer_Event_addJob					:	///< ActiveJobListからの状態変化通知(ジョブの追加).
			break;
		case JSVE_RequestAnswer_Event_DocSearcher				:	///< DocSearcherからの状態変化通知.
			break;
		case JSVE_Request_Event_Max				:	///< 最大値(23).
			#ifdef _DEBUG_JSVC_SEQ
			MyTraceSD(JSVD_DBG_ERROR, "最大値:処理なし。startAction20" , 0 );
			#endif
			break;
		default:
			#ifdef _DEBUG_JSVC_SEQ
			MyTraceSD(JSVD_DBG_ERROR, "default。startAction20" , ps_NotifyInfo->e_Request_Event );
			#endif
			break;
	}

	return e_Request_Result;
}
// 情報通知から状態に応じた処理を実行する.
// 次ジョブ実行待ち.
JSVE_Request_Result JSVC_Request::startAction21( JSVS_NotifyInfo*	ps_NotifyInfo, JSVC_Request* pc_Request )
{
	JSVE_Request_Result e_Request_Result = JSVE_Request_Result_Err;

	switch( ps_NotifyInfo->e_Request_Event )
	{
		case JSVE_Request_Event_Non				:	///< 初期値.
			break;
		case JSVE_Request_Event_ReqMemAlloc		:	///< メモリ確保要求.
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, e_Request_Result, JSVE_JobServiceResultDetail_AllreadyMemAlloced, pc_Request);
			break;
		case JSVE_Request_Event_AnsAnalyzeCancel:	///< XML解析削除応答.
			break;
		case JSVE_Request_Event_AnsAnalyzeEnd	:	///< XML解析完了応答.
			break;
		case JSVE_Request_Event_AnsMemAllocEnd	:	///< メモリ確保応答.
			break;
		case JSVE_Request_Event_ReqStart		:	///< サブシナリオ開始要求.
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, e_Request_Result, JSVE_JobServiceResultDetail_Exec, pc_Request);
			break;
		case JSVE_Request_Event_ReqStop			:	///< サブシナリオ停止要求.
			{
				uint32_t ulID = pc_Request->getSubScenarioID();
				JSVC_ParentRequest* pc_PRequest = NULL;
				pc_PRequest = JSVC_JobManager::newInstance()->getParentRequest(ulID);
				//Str【 静的解析対応 : Sky花岡 : 2012.09.03 : CID-439618】.
				if( NULL == pc_PRequest ){
					MyTraceSD(JSVD_DBG_ERROR, "JSVC_RequestJobSetting::startAction21 getParentRequest ERR" , (int)0 );
					break;
				}
				//End【 静的解析対応 : Sky花岡 : 2012.09.03 : CID-439618】.
				if( False == pc_PRequest->getStopFlg() ) {
					pc_PRequest->setStopFlg( True );
					pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Ok, JSVE_JobServiceResultDetail_Stop, pc_Request);
					e_Request_Result = JSVE_Request_Result_Ok;
				} else {
					pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Err, JSVE_JobServiceResultDetail_Stop, pc_Request);
					e_Request_Result = JSVE_Request_Result_Err;
				}
			}
			break;
		case JSVE_Request_Event_AnsStopEnd		:	///< サブシナリオ停止応答.
			break;
		case JSVE_Request_Event_ReqUpdate		:	///< サブシナリオ設定変更要求.
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, e_Request_Result, JSVE_JobServiceResultDetail_NextJobExecWait, pc_Request);
			break;
		case JSVE_Request_Event_AnsUpdateEnd	:	///< サブシナリオ設定変更応答.
			break;
		case JSVE_Request_Event_ReqRestart		:	///< サブシナリオ再開要求.
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, e_Request_Result, JSVE_JobServiceResultDetail_Exec, pc_Request);
			break;
		case JSVE_Request_Event_AnsRestartEnd	:	///< サブシナリオ再開応答.
			break;
		case JSVE_Request_Event_ReqCancel		:	///< サブシナリオ削除要求.
			{
				uint32_t ulID = pc_Request->getSubScenarioID();
				JSVC_ParentRequest* pc_PRequest = NULL;
				pc_PRequest = JSVC_JobManager::newInstance()->getParentRequest(ulID);
				//Str【 静的解析対応 : Sky花岡 : 2012.09.03 : CID-439618】.
				if( NULL == pc_PRequest ){
					MyTraceSD(JSVD_DBG_ERROR, "JSVC_RequestJobSetting::startAction21 getParentRequest ERR" , (int)0 );
					break;
				}
				//End【 静的解析対応 : Sky花岡 : 2012.09.03 : CID-439618】.
				if( False == pc_PRequest->getCancelFlg() ) {
					pc_PRequest->setCancelFlg( True );
					pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Ok, JSVE_JobServiceResultDetail_Cancel, pc_Request);
					e_Request_Result = JSVE_Request_Result_Ok;
				} else {
					pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Err, JSVE_JobServiceResultDetail_Cancel, pc_Request);
					e_Request_Result = JSVE_Request_Result_Err;
				}
			}
			break;
		case JSVE_Request_Event_AnsCancel		:	///< サブシナリオ削除応答.
			break;
		case JSVE_Request_Event_AnsEnd			:	///< サブシナリオ終了通知.
			break;
		case JSVE_Request_Event_ReqMemFree		:	///< メモリ開放要求.
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, e_Request_Result, JSVE_JobServiceResultDetail_NextJobExecWait, pc_Request);
			break;
		case JSVE_Request_Event_AnsMemFree		:	///< メモリ開放応答.
			break;
		case JSVE_RequestAnswer_Event_ChargeMapPrepared			:	///< 課金マップ展開完了.
			break;
		case JSVE_RequestAnswer_Event_NewInstance				:	///< 待ちうけ部からの着信通知.
			break;
		case JSVE_RequestAnswer_Event_addJob					:	///< ActiveJobListからの状態変化通知(ジョブの追加).
			{
				uint32_t ulID = pc_Request->getSubScenarioID();
				JSVC_ParentRequest* pc_PRequest = NULL;
				pc_PRequest = JSVC_JobManager::newInstance()->getParentRequest(ulID);
				//Str【 静的解析対応 : Sky花岡 : 2012.09.03 : CID-439618】.
				if( NULL == pc_PRequest ){
					MyTraceSD(JSVD_DBG_ERROR, "JSVC_RequestJobSetting::startAction21 getParentRequest ERR" , (int)0 );
					break;
				}
				//End【 静的解析対応 : Sky花岡 : 2012.09.03 : CID-439618】.
				if( False == pc_PRequest->getCancelFlg()
				&&  False == pc_PRequest->getStopFlg()
				){
					// 削除・停止フラグが共にFalseのときは、ステータスを実行中へ.
					pc_Request->e_Request_Status = JSVE_Request_Status_Exec;
				}
				else if( True == pc_PRequest->getCancelFlg() ){
					// 削除フラグがTrueのときは、ジョブキャンセル指示を行う.
					uint32_t ul_index;
					ul_index = JSVC_JobManager::newInstance()->availableAnsFunctionIndex( JSVC_CB_INDEX_ans_cancel, JSVC_CB_MAX_ans_cancel, pc_Request );
					if( ul_index != (uint32_t)-1 )				// warning対応 2013/09/21 ooba.
					{
						//Str【 静的解析対応 : Sky花岡 : 2012.09.03 : CID-439617】.
						if( NULL == pc_Request->getJobInstance() ){
							MyTraceSD(JSVD_DBG_ERROR, "JSVC_RequestJobSetting::startAction21 getParentRequest ERR" , (int)0 );
							break;
						}
						//End【 静的解析対応 : Sky花岡 : 2012.09.03 : CID-439617】.
						// ジョブキャンセル指示を行う.
						pc_Request->getJobInstance()->req_cancel((void (*)( APIC_Job* ))(JSVC_JobManager::newInstance()->c_CBManage[ul_index].vp_Func));

						pc_PRequest->setCancelFlg( True );

						pc_Request->e_Request_Status = JSVE_Request_Status_ReqCancel;
						e_Request_Result = JSVE_Request_Result_Ok;
					}
					else
					{
						// ans_cancel関数が取れなかったので、キャンセル要求は無効にする
						pc_Request->e_Request_Status = JSVE_Request_Status_Exec;
						pc_PRequest->setCancelFlg( False );

						// キャンセル要求を無効にするためのイベントを発生させる.
						JSVS_SendRecv_MsgParam s_MsgParam;
						JSVS_ResponseParamJobNotify st_ResJobNotify;
						memset( &st_ResJobNotify, 0x00, sizeof(st_ResJobNotify) );
						st_ResJobNotify.ul_SubScenarioID	= ps_NotifyInfo->pRes->getSubScenarioID();
						st_ResJobNotify.pJob				= ps_NotifyInfo->pRes->getJobInstance();
						st_ResJobNotify.ul_Result			= JSVE_Request_Result_Err;
						st_ResJobNotify.ul_ResultDetail		= IPCE_JobsResultDetail_CBNumOver;
						st_ResJobNotify.ul_FuncSts			= 0;
						st_ResJobNotify.ul_AcceptNum		= 0;
						st_ResJobNotify.b_Continue			= 0;
						JSVC_ResponseJobNotify* pc_ResJobNotify = new JSVC_ResponseJobNotify( JSVE_JobManager_MsgID_CancelEnd );
						#if DEBUGLOG__REQUEST_LIFE
							MyTraceSLD(JSVD_DBG_INFO, "startAction21 new JSVC_ResponseJobNotify1" , (intptr_t)pc_ResJobNotify );	// warning対応 2013/09/18 ooba.
						#endif
						if( pc_ResJobNotify ){
							pc_ResJobNotify->setData( (const char*)&st_ResJobNotify, sizeof(st_ResJobNotify) );
							s_MsgParam.ul_MsgTypeID	= JSVE_JobManager_MsgID_CancelEnd;
							s_MsgParam.pReq			= NULL;
							s_MsgParam.pRes			= (JSVC_ResponseBase*)pc_ResJobNotify;
							JSVC_JobManager::newInstance()->sendMessageToJob( &s_MsgParam );
						}
					}
				}
				else if( True == pc_PRequest->getStopFlg() ){
					// 停止フラグがTrueのときは、ジョブ停止指示を行う.
					APIC_ActiveJobList::newInstance()->req_stop(ans_stop);
					pc_Request->e_Request_Status = JSVE_Request_Status_ReqStop;
					e_Request_Result = JSVE_Request_Result_Ok;
				}
			}
			break;
		case JSVE_RequestAnswer_Event_DocSearcher				:	///< DocSearcherからの状態変化通知.
			// 通知なので、Responseのパラメータを参照する.
			if( JSVE_Request_Result_Ok == ps_NotifyInfo->pRes->getResult() ){
				// DocSearcherからの起動OKの場合は、addJob()通知を待つので、状態遷移しない.
				e_Request_Result = JSVE_Request_Result_Ok;
			}
			else{
				// DocSearcherからの起動NGの場合は、サブシナリオ再開されないので実行完了NGメッセージで終了させる.
				pc_Request->e_Request_Status = JSVE_Request_Status_Exec;

				JSVS_SendRecv_MsgParam s_MsgParam;
				JSVS_ResponseParamJobNotify st_ResJobNotify;
				memset( &st_ResJobNotify, 0x00, sizeof(st_ResJobNotify) );
				st_ResJobNotify.ul_SubScenarioID	= ps_NotifyInfo->pRes->getSubScenarioID();
				st_ResJobNotify.pJob				= ps_NotifyInfo->pRes->getJobInstance();
				st_ResJobNotify.ul_Result			= JSVE_Request_Result_Err;
				st_ResJobNotify.ul_ResultDetail		= IPCE_JobsResultDetail_DocSearcherNG;
				st_ResJobNotify.ul_FuncSts			= 0;
				st_ResJobNotify.ul_AcceptNum		= 0;
				st_ResJobNotify.b_Continue			= 0;
				JSVC_ResponseJobNotify* pc_ResJobNotify = new JSVC_ResponseJobNotify( JSVE_JobManager_MsgID_End );
				#if DEBUGLOG__REQUEST_LIFE
					MyTraceSLD(JSVD_DBG_INFO, "startAction21 new JSVC_ResponseJobNotify2" , (intptr_t)pc_ResJobNotify );	// warning対応 2013/09/18 ooba.
				#endif
				if( pc_ResJobNotify ){
					pc_ResJobNotify->setData( (const char*)&st_ResJobNotify, sizeof(st_ResJobNotify) );
					s_MsgParam.ul_MsgTypeID	= JSVE_JobManager_MsgID_End;
					s_MsgParam.pReq			= NULL;
					s_MsgParam.pRes			= (JSVC_ResponseBase*)pc_ResJobNotify;
					JSVC_JobManager::newInstance()->sendMessageToJob( &s_MsgParam );
				}
			}
			break;
		case JSVE_Request_Event_Max				:	///< 最大値(23).
			#ifdef _DEBUG_JSVC_SEQ
			MyTraceSD(JSVD_DBG_ERROR, "最大値:処理なし。startAction21" , 0 );
			#endif
			break;
		default:
			#ifdef _DEBUG_JSVC_SEQ
			MyTraceSD(JSVD_DBG_ERROR, "default。startAction21" , ps_NotifyInfo->e_Request_Event );
			#endif
			break;
	}

	return e_Request_Result;
}
// 情報通知から状態に応じた処理を実行する.
// 課金MAP展開待ち.
JSVE_Request_Result JSVC_Request::startAction22( JSVS_NotifyInfo*	ps_NotifyInfo, JSVC_Request* pc_Request )
{
	JSVE_Request_Result e_Request_Result = JSVE_Request_Result_Err;

	switch( ps_NotifyInfo->e_Request_Event )
	{
		case JSVE_Request_Event_Non				:	///< 初期値.
			break;
		case JSVE_Request_Event_ReqMemAlloc		:	///< メモリ確保要求.
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, e_Request_Result, JSVE_JobServiceResultDetail_AllreadyMemAlloced, pc_Request);
			break;
		case JSVE_Request_Event_AnsAnalyzeCancel:	///< XML解析削除応答.
			break;
		case JSVE_Request_Event_AnsAnalyzeEnd	:	///< XML解析完了応答.
			break;
		case JSVE_Request_Event_AnsMemAllocEnd	:	///< メモリ確保応答.
			if( IPCE_JobServiceResult_OK !=  pc_Request->getResult())
			{
				pc_Request->notifyJobService_Answer(JSVE_Request_Event_AnsMemAllocEnd, JSVE_Request_Result_Err, JSVE_JobServiceResultDetail_UserCancel, pc_Request);
				// ジョブインスタンス削除を行う.
// CQ:366448対応 Sky_iwasaki 2012.06.15 Start.
//				pc_Request->getJobInstance()->deleteInstance();
// CQ:366448対応 Sky_iwasaki 2012.06.15 End.
			}
			break;
		case JSVE_Request_Event_ReqStart		:	///< サブシナリオ開始要求.
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, e_Request_Result, JSVE_JobServiceResultDetail_WaitChargeMapExpand, pc_Request);
			break;
		case JSVE_Request_Event_ReqStop			:	///< サブシナリオ停止要求.
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, e_Request_Result, JSVE_JobServiceResultDetail_WaitChargeMapExpand, pc_Request);
			break;
		case JSVE_Request_Event_AnsStopEnd		:	///< サブシナリオ停止応答.
			break;
		case JSVE_Request_Event_ReqUpdate		:	///< サブシナリオ設定変更要求.

			//既に設定変更リクエストを受けていた場合は新しく受け付けずにNGを返す.
			//（先頭から順に処理するので自リクエストが最初にHITすれば実行可能と判断）.
			if( pc_Request != JSVC_JobManager::newInstance()->GetExecutingUpdateRequest( pc_Request->getSubScenarioID() ) )
			{
				//同一サブシナリオへの多重リクエストありなら受付でNG.
				// JobManagerへ要求NGを通知する.
				pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Err, JSVE_JobServiceResultDetail_MultiReq, pc_Request);
				//ステータスを送付済にしておく.
				((JSVC_RequestJobSetting*)pc_Request)->setSendUpdateEnd();
				// NGを通知後の継続処理が必要なため、結果はOKを返す.
				e_Request_Result = JSVE_Request_Result_Ok;
			} else {
				//重複チェックOKなのでUPDATE処理開始.
				
				TYPE_XmlResultStatus e_XmlResultStatus = pc_Request->pc_XmlAnalyze->req_StartXmlAnalyze(pc_Request->ul_SubScenarioID, ((JSVC_RequestJobSetting*)pc_Request)->getFilePath(), pc_Request, ans_Analyze);
				if (TYPE_XmlResultStatus_ReqOK == e_XmlResultStatus){																		// 受付完了.
					// JobManagerへ要求OKを通知する.
					pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Ok, JSVE_JobServiceResultDetail_Non, pc_Request);
					// 結果設定.
					e_Request_Result = JSVE_Request_Result_Ok;
				}else{
					// JobManagerへ要求NGを通知する.
					pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, e_Request_Result, JSVE_JobServiceResultDetail_AnalyzeErr, pc_Request);
					MyTraceSS(JSVD_DBG_INFO, "startAction22 JSVE_Request_Event_ReqUpdate elseルート JSVE_Request_Result_Err!","");
					// NGを通知後の継続処理が必要なため、結果はOKを返す.
					e_Request_Result = JSVE_Request_Result_Ok;
				}
			}
			break;
		case JSVE_Request_Event_AnsUpdateEnd	:	///< サブシナリオ設定変更応答.
			{
//Str【 静的解析対応 : Sky田村 : 2012.08.30 : CID-436582 】.
//				MyTraceSS(JSVD_DBG_INFO, "APFへ設定変更完了を通知","");
//				//JSVC_SendRecv::MakeDataStreamにあわせて結果をRESからREQへ転記.
//				if (( NULL != ps_NotifyInfo->pReq )
//				&&  ( NULL != ps_NotifyInfo->pRes )){
//					ps_NotifyInfo->pReq->setResult( ps_NotifyInfo->pRes->getResult() );
//					ps_NotifyInfo->pReq->setResultDetail( ps_NotifyInfo->pRes->getResultDetail() );
//				}
//				//結果をAPFへ通知.
//				JSVC_JobSendRecv::newInstance()->sendMessageToAPF( JSVE_JobManager_MsgID_ChangeSettingEnd ,ps_NotifyInfo->pReq , ps_NotifyInfo->pRes );
//				e_Request_Result = JSVE_Request_Result_Complete;
				MyTraceSS(JSVD_DBG_INFO, "APFへ設定変更完了を通知","");
				if (( NULL != ps_NotifyInfo->pReq )
				&&  ( NULL != ps_NotifyInfo->pRes )){
					ps_NotifyInfo->pReq->setResult( ps_NotifyInfo->pRes->getResult() );
					ps_NotifyInfo->pReq->setResultDetail( ps_NotifyInfo->pRes->getResultDetail() );
					//結果をAPFへ通知.
					JSVC_JobSendRecv::newInstance()->sendMessageToAPF( JSVE_JobManager_MsgID_ChangeSettingEnd ,ps_NotifyInfo->pReq , ps_NotifyInfo->pRes );
					e_Request_Result = JSVE_Request_Result_Complete;
				}
				else
				{
					MyTraceSS(JSVD_DBG_INFO, "ps_NotifyInfo->pReq or ps_NotifyInfo->pRes is NULL","");
					e_Request_Result = JSVE_Request_Result_Err;
				}
//End【 静的解析対応 : Sky田村 : 2012.08.30 : CID-436582 】.
			}
			break;
		case JSVE_Request_Event_ReqRestart		:	///< サブシナリオ再開要求.
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, e_Request_Result, JSVE_JobServiceResultDetail_WaitChargeMapExpand, pc_Request);
			break;
		case JSVE_Request_Event_AnsRestartEnd	:	///< サブシナリオ再開応答.
			break;
		case JSVE_Request_Event_ReqCancel		:	///< サブシナリオ削除要求.
			{
				uint32_t ulID = pc_Request->getSubScenarioID();
				JSVC_ParentRequest* pc_PRequest = NULL;
				pc_PRequest = JSVC_JobManager::newInstance()->getParentRequest(ulID);
				//Str【 静的解析対応 : Sky花岡 : 2012.09.03 : CID-439619】.
				if( NULL == pc_PRequest ){
					MyTraceSD(JSVD_DBG_ERROR, "JSVC_RequestJobSetting::startAction22 pc_PRequest ERR" , (int)0 );
					break;
				}
				//End【 静的解析対応 : Sky花岡 : 2012.09.03 : CID-439619】.
				if( False == pc_PRequest->getCancelFlg() ) {
					pc_PRequest->setCancelFlg( True );
					pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, JSVE_Request_Result_Ok, JSVE_JobServiceResultDetail_Cancel, pc_Request);
				} else {
					pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, e_Request_Result, JSVE_JobServiceResultDetail_Cancel, pc_Request);
					MyTraceSS(JSVD_DBG_INFO, "startAction22 JSVE_Request_Event_ReqCancel JSVE_Request_Result_Err!","");
					// NGを通知後の継続処理が必要なため、結果はOKを返す.
					e_Request_Result = JSVE_Request_Result_Ok;
				}
			}
			break;
		case JSVE_Request_Event_AnsCancel		:	///< サブシナリオ削除応答.
			break;
		case JSVE_Request_Event_AnsEnd			:	///< サブシナリオ終了通知.
			break;
		case JSVE_Request_Event_ReqMemFree		:	///< メモリ開放要求.
			pc_Request->notifyJobService_ReqAnswer(ps_NotifyInfo->e_Request_Event, e_Request_Result, JSVE_JobServiceResultDetail_WaitChargeMapExpand, pc_Request);
			break;
		case JSVE_Request_Event_AnsMemFree		:	///< メモリ開放応答.
			break;
		case JSVE_RequestAnswer_Event_ChargeMapPrepared			:	///< 課金マップ展開完了.
			{
				TYPE_JobType			e_JobType;
				JSVC_IsListnerType		ul_RetIsList;
				uint32_t ulID = pc_Request->getSubScenarioID();
				JSVC_ParentRequest* pc_PRequest = NULL;
				pc_PRequest = JSVC_JobManager::newInstance()->getParentRequest(ulID);
				//Str【 静的解析対応 : Sky花岡 : 2012.09.03 : CID-439619】.
				if( NULL == pc_PRequest ){
					MyTraceSD(JSVD_DBG_ERROR, "JSVC_RequestJobSetting::startAction22 pc_PRequest ERR" , (int)0 );
					break;
				}
				//End【 静的解析対応 : Sky花岡 : 2012.09.03 : CID-439619】.
				if( False == pc_PRequest->getCancelFlg() ) {
					ul_RetIsList = JSVC_JobManager::newInstance()->isListenerFunc(pc_Request->getJobInstance()->getJobSpec(), pc_Request->ul_SubScenarioID, &e_JobType);
					// 待ち受けの場合は上書きする.
					if ((True == ((JSVC_RequestJobSetting*)pc_Request)->isReceiveAPP())
					|| (TYPD_ImageLogSendJobNon != pc_Request->getJobInstance()->getJobSpec()->pc_Common->getImageLogSend() )
					){
						ul_RetIsList = JSVC_IsListner_Listner;
					}

					if (JSVC_IsListner_NG == ul_RetIsList) {
						pc_Request->pc_JobManager->notify( pc_Request, e_Request_Result, JSVE_JobServiceResultDetail_XmlAnalyze, JSVE_JobManager_MsgID_MemoryAllocEnd, 0 );
					} else if (JSVC_IsListner_NoListner == ul_RetIsList) {
						if (True == pc_Request->isStartJob()) {
							if (NULL == pc_Request->getJobInstance()) {
								// JobManagerへ要求NGを通知する.
								pc_Request->pc_JobManager->notify( pc_Request, e_Request_Result, JSVE_JobServiceResultDetail_XmlAnalyze, JSVE_JobManager_MsgID_MemoryAllocEnd, 0 );
								MyTraceSS(JSVD_DBG_INFO, "追加","getJobInstanceがNULL");
							}else{
								// ジョブタイプをFixする.
								// Jobタイプ決定処理.
								if ( True == JSVC_JobManager::newInstance()->fixJobType( pc_Request->getJobInstance()->getJobSpec(), TYPE_ReceptionModuleType_NONE, False, &e_JobType )) {
									pc_Request->getJobInstance()->getJobSpec()->pc_Common->c_JobType.setJobType( e_JobType );
								} else {
									MyTraceSD(JSVD_DBG_ERROR, "JSVC_Request::startAction22 fixJobType2 ERR" , False );
								}
								#if DEBUGLOG__JOBSTART_JUDGE
									// TYPE_JobTypeCopy                0   ///<	ジョブタイプ：コピー 
									// TYPE_JobTypeIntCopy             1   ///<	ジョブタイプ：割込コピー 
									// TYPE_JobTypeScan                2   ///<	ジョブタイプ：スキャン 
									// TYPE_JobTypeTwain               3   ///<	ジョブタイプ：Ｔｗａｉｎ 
									// TYPE_JobTypePrint               4   ///<	ジョブタイプ：プリント 
									// TYPE_JobTypeSecurePrint         5   ///<	ジョブタイプ：機密文書プリント 
									// TYPE_JobTypePdl                 6   ///<	ジョブタイプ：ＰＤＬ（ＲＩＰ処理前） 
									// TYPE_JobTypeFaxSendQuick        7   ///<	ジョブタイプ：ＦＡＸ（クイックメモリ送信） 
									// TYPE_JobTypeFaxSendMemory       8   ///<	ジョブタイプ：ＦＡＸ（メモリ送信） 
									// TYPE_JobTypeFaxSendTimer        9   ///<	ジョブタイプ：ＦＡＸ(タイマ送信） 
									// TYPE_JobTypeFaxReceive          10  ///<	ジョブタイプ：ＦＡＸ受信プリント 
									// TYPE_JobTypeIfaxReceive         11  ///<	ジョブタイプ：ＩＦａｘ受信 
									// TYPE_JobTypeFaxReceiveTimer     12  ///<	ジョブタイプ：ＦＡＸ（タイマーポーリング受信） 
									// TYPE_JobTypeFaxReceivePolling   13  ///<	ジョブタイプ：ＦＡＸ（ポーリング送信） 
									MyTraceSD(JSVD_DBG_INFO, "JobType = ",e_JobType);
								#endif
								pc_Request->getJobInstance()->getJobSpec()->pc_Common->setScenarioExecuteFlg(True);		//シナリオフラグをTrueへ(JobSからは常にTrue).
								pc_Request->getJobInstance()->getJobSpec()->pc_Common->setInitProcessFlg(True);		//Initプロセスフラグを[実行後停止]へ.
								pc_Request->getJobInstance()->getJobSpec()->pc_Common->setEndProcessFlg(True);		//Endプロセスフラグを[実行前停止]へ.
								uint32_t ul_index;
								// ジョブ開始指示.
								ul_index = JSVC_JobManager::newInstance()->availableAnsFunctionIndex( JSVC_CB_INDEX_ans_start, JSVC_CB_MAX_ans_start, pc_Request );
								if( ul_index != (uint32_t)-1 ) {				// warning対応 2013/09/21 ooba.
									// ジョブ開始指示.
									pc_Request->getJobInstance()->req_start((void (*)( APIC_Job* ))(JSVC_JobManager::newInstance()->c_CBManage[ul_index].vp_Func));
									// 結果設定.
									e_Request_Result = JSVE_Request_Result_Ok;
									// 状態更新(XML解析終了中).
									pc_Request->e_Request_Status = JSVE_Request_Status_AnalyzeEnd;
									pc_Request->ul_DetailParam = ((pc_Request->ul_DetailParam & (~JSVD_DetailParamMask_XmlAnalyze)) | (ps_NotifyInfo->pReq->getResultDetail() & JSVD_DetailParamMask_XmlAnalyze));
								} else {

									MyTraceSD(JSVD_DBG_ERROR, "JSVC_Request::startAction22 CancelErr. e_Request_Event:" , ps_NotifyInfo->e_Request_Event );
									pc_Request->setResult(JSVE_JobServiceResult_NG);
									pc_Request->setResultDetail(IPCE_JobsResultDetail_CBNumOver);
									// JobManagerへ要求NGを通知する.
									e_Request_Result = JSVE_Request_Result_Err;
									pc_Request->notifyJobService_Answer( JSVE_Request_Event_AnsMemAllocEnd , JSVE_Request_Result_Err , IPCE_JobsResultDetail_CBNumOver , pc_Request );
								}
							}
						}else{
							// JobManagerへ要求NGを通知する.
							pc_Request->pc_JobManager->notify( pc_Request, e_Request_Result, JSVE_JobServiceResultDetail_XmlAnalyze, JSVE_JobManager_MsgID_MemoryAllocEnd, 0 );
							// NGを通知後の継続処理が必要なため、結果はOKを返す.
							e_Request_Result = JSVE_Request_Result_Ok;
							MyTraceSS(JSVD_DBG_INFO, "startAction22 JSVE_RequestAnswer_Event_ChargeMapPrepared JSVE_Request_Result_Err!","");

						}
					} else {
						MyTraceSS(JSVD_DBG_INFO, "JSVC_Request::startAction22 ジョブタイプレガシー","");
						if (JSVC_IsListner_RetryListner == ul_RetIsList) {

							//ADD ここまできたら強制的にOKを送ってStartを待ちつつ継続 .
							pc_Request->notifyJobService_Answer( JSVE_Request_Event_AnsMemAllocEnd , JSVE_Request_Result_Ok , IPCE_JobsResultDetail_Non , pc_Request );

							MyTraceSS(JSVD_DBG_INFO, "JSVC_Request::startAction22 ","リトライジョブに対してrestart実行");
							TYPE_StopProcess e_StopProcess = 0;
							// プロセスの停止設定を解除してジョブを再開する.
							pc_Request->getJobInstance()->getJobSpec()->pc_Common->setStopProcess(e_StopProcess);
							pc_Request->getJobInstance()->getJobSpec()->pc_Common->setInitProcessFlg(False);
							pc_Request->getJobInstance()->getJobSpec()->pc_Common->setEndProcessFlg(False);
							pc_Request->getJobInstance()->restart();
							// 結果設定.
							e_Request_Result = JSVE_Request_Result_Ok;
							// 状態更新(サブシナリオ実行中).
							pc_Request->e_Request_Status = JSVE_Request_Status_Exec;
						} else {
							// 待受けの場合.
							// 結果設定.
							e_Request_Result = JSVE_Request_Result_Ok;
							// 状態更新(XML解析終了中).
							pc_Request->e_Request_Status = JSVE_Request_Status_AnalyzeEnd;

							pc_Request->pc_JobManager->notify( pc_Request, e_Request_Result, JSVE_JobServiceResultDetail_XmlAnalyze, JSVE_JobManager_MsgID_MemoryAllocEnd, 0 );
						}
					}
				} else {
					// APP P/Fへメモリ確保応答を"NG"でメッセージ送信.
					//コピー系動作の場合はreq_startせずにdeleteinstanceして終わる.
					//ＦＡＸ系動作の場合はstart待ちにならずにdeleteinstanceして終わる.
					//どちらの場合も上位層へMemAllocEndを通知してジョブ破棄とする.
					e_Request_Result = JSVE_Request_Result_RecCanRes;
					pc_Request->pc_JobManager->notify( pc_Request, JSVE_Request_Result_Err, JSVE_JobServiceResultDetail_UserCancel, JSVE_JobManager_MsgID_MemoryAllocEnd, 0 );
				}
			}
			break;
		case JSVE_RequestAnswer_Event_NewInstance				:	///< 待ちうけ部からの着信通知.
			break;
		case JSVE_RequestAnswer_Event_addJob					:	///< ActiveJobListからの状態変化通知(ジョブの追加).
			break;
		case JSVE_RequestAnswer_Event_DocSearcher				:	///< DocSearcherからの状態変化通知.
			break;
		case JSVE_Request_Event_Max				:	///< 最大値(23).
			#ifdef _DEBUG_JSVC_SEQ
			MyTraceSD(JSVD_DBG_ERROR, "最大値:処理なし。startAction22" , 0 );
			#endif
			break;
		default:
			#ifdef _DEBUG_JSVC_SEQ
			MyTraceSD(JSVD_DBG_ERROR, "default。startAction22" , ps_NotifyInfo->e_Request_Event );
			#endif
			break;
	}

	return e_Request_Result;
}//ADD-End	STEP2.

// エラー停止状態での処理.
///< 最大値(16)⇒(23).
JSVE_Request_Result JSVC_Request::startAction23( JSVS_NotifyInfo*	ps_NotifyInfo, JSVC_Request* pc_Request )
{
	JSVE_Request_Result e_Request_Result = JSVE_Request_Result_Err;
#ifdef _DEBUG_JSVC_SEQ
	MyTraceSLD(JSVD_DBG_INFO, "JSVC_Request::startAction23 ps_NotifyInfo:" , (intptr_t)ps_NotifyInfo );		// 回避方法をトレースに変更.
	MyTraceSLD(JSVD_DBG_INFO, "JSVC_Request::startAction23 pc_Request:" , (intptr_t)pc_Request );			// 回避方法をトレースに変更.
#endif

	#ifdef _DEBUG_JSVC_SEQ
	MyTraceSD(JSVD_DBG_INFO, "STEP1ではエラー発生後には処理がありません。" , 0 );
	#endif

	return e_Request_Result;
}

// JobManagerへリクエストの結果を通知する.
//		JSVE_Request_Event_ReqMemAlloc	->	パスチェック完了時点でSTEP2用notifyJobService_ReqAnswerを呼ぶこと.
//		JSVE_Request_Event_ReqStart		->	ans_startでSTEP2用notifyJobService_ReqAnswerを呼ぶこと.
//		JSVE_Request_Event_ReqStop		->	ans_stopでSTEP2用notifyJobService_ReqAnswerを呼ぶこと.
//		JSVE_Request_Event_ReqRestart	->	要求が出せたらSTEP2用notifyJobService_ReqAnswerを呼ぶこと.
//		JSVE_Request_Event_ReqCancel	->	ans_cancelでSTEP2用notifyJobService_ReqAnswerを呼ぶこと.
//		JSVE_Request_Event_ReqUpdate	->	パスチェック完了時点でSTEP2用notifyJobService_ReqAnswerを呼ぶこと.
//		JSVE_Request_Event_ReqMemFree	->	COPY系　：開放要求を出す前にSTEP2用notifyJobService_ReqAnswerを呼ぶこと.
//											COPY以外：状態チェックをパスした時点でSTEP2用notifyJobService_ReqAnswerを呼ぶこと.
//
void JSVC_Request::notifyJobService_ReqAnswer(JSVE_Request_Event e_Request_Event, JSVE_Request_Result e_Request_Result, JSVE_JobServiceResultDetail e_ResultDetail, JSVC_Request* pc_Request )
{
	JSVE_Request_Event e_RequestAnswer_Event = JSVE_Request_Event_Non;
	JSVE_JobManager_MsgID e_JobManager_MsgID = JSVE_JobManager_MsgID_Non;
	uint32_t	e_FuncSts = 0;

	// ローカルイベント編集.
	// 要求の種類に応じて、応答（ローカルイベント）を設定する.
	switch(e_Request_Event)
	{
	case JSVE_Request_Event_ReqMemAlloc:
		e_RequestAnswer_Event = JSVE_RequestAnswer_Event_MemoryAllocAccept;
		break;
	case JSVE_Request_Event_ReqStart:
		e_RequestAnswer_Event = JSVE_RequestAnswer_Event_StartAccept;
		break;
	case JSVE_Request_Event_ReqStop:
		e_RequestAnswer_Event = JSVE_RequestAnswer_Event_StopAccept;
		break;
	case JSVE_Request_Event_ReqRestart:
		e_RequestAnswer_Event = JSVE_RequestAnswer_Event_ReStartAccept;
		break;
	case JSVE_Request_Event_ReqCancel:
		e_RequestAnswer_Event = JSVE_RequestAnswer_Event_CancelAccept;
		break;
	case JSVE_Request_Event_ReqUpdate:
		e_RequestAnswer_Event = JSVE_RequestAnswer_Event_ChangeSettingAccept;
		break;
	case JSVE_Request_Event_ReqMemFree:
		e_RequestAnswer_Event = JSVE_RequestAnswer_Event_MemoryDeleteAccept;
		break;

	default:
		MyTraceSD( JSVD_DBG_ERROR, "JSVC_Request::notifyJobService_ReqAnswer unknown Event :" , e_Request_Event );
// 未対応イベント.
//		e_RequestAnswer_Event = JSVE_RequestAnswer_Event_StatusChange
//		e_RequestAnswer_Event = JSVE_RequestAnswer_Event_FunctionStatusChange
//		e_RequestAnswer_Event = JSVE_RequestAnswer_Event_Warning
		break;
	}

	// ローカルイベントからメインイベントに変換する.
	e_JobManager_MsgID = changeMainEvent(e_RequestAnswer_Event);

	// ログ.
	_setJsrvSeqLog( TYPE_FuncName_09, e_Request_Event, ul_SubScenarioID, (ushort)e_Request_Result, (ushort)e_ResultDetail, NULL );


	// イベントと結果に対応した通知パラメータ編集を行い、JobManagerにリクエストの応答を通知する.
	// JobManagerでタスクスイッチ後にリクエスト応答をアプリケーションプラットフォームへ通知する.
	pc_Request->pc_JobManager->notify( pc_Request , e_Request_Result , e_ResultDetail , e_JobManager_MsgID , e_FuncSts );
}

// JobManagerへリクエストの受付結果を通知する.
// 下回りや自身からe_Request_Eventを受けたときに自身へe_RequestAnswer_Event(Acceptイベント)を発行する.
//
//	INPUT
//		e_JobManager_MsgID	通知したいMSGのID.
//		pc_Request			結果を保持しているリクエスト.

void JSVC_Request::notifyJobService_ReqAnswer(JSVE_JobManager_MsgID e_JobManager_MsgID, JSVC_Request* pc_Request )
{
	uint32_t	e_FuncSts = 0;

	// ログ.
	_setJsrvSeqLog( TYPE_FuncName_09, e_JobManager_MsgID, pc_Request->ul_SubScenarioID, (ushort)pc_Request->ul_Result, (ushort)pc_Request->ul_ResultDetail, NULL );

	// イベントと結果に対応した通知パラメータ編集を行い、JobManagerにリクエストの応答を通知する.
	// JobManagerでタスクスイッチ後にリクエスト応答をアプリケーションプラットフォームへ通知する.
	pc_Request->pc_JobManager->notify( pc_Request , pc_Request->ul_Result , pc_Request->ul_ResultDetail , e_JobManager_MsgID , e_FuncSts );
}

// JobManagerへリクエストの実行結果を通知する.
void JSVC_Request::notifyJobService_Answer(JSVE_Request_Event e_Request_Event, JSVE_Request_Result e_Request_Result, JSVE_JobServiceResultDetail e_ResultDetail, JSVC_Request* pc_Request )
{
	JSVE_JobManager_MsgID e_JobManager_MsgID = JSVE_JobManager_MsgID_Non;

	pc_Request->setResult( e_Request_Result );

	// イベントと結果に対応した通知パラメータ編集を行い、アプリケーションプラットフォームへ結果を通知する.
	// ローカルイベントからメインイベントに変換する.
	e_JobManager_MsgID = changeMainEvent(e_Request_Event);

	// 通知メッセージ編集
	JSVS_SendRecv_MsgParam s_SendRecv_MsgParam;
	s_SendRecv_MsgParam.ul_MsgTypeID		= e_JobManager_MsgID;						// メッセージ種別.
	s_SendRecv_MsgParam.pReq = (JSVC_Request*)pc_Request;

	pc_Request->setResultDetail(changeDetailCondeJSVCToAPF(e_ResultDetail));	// 結果詳細.

	// ログ.
	_setJsrvSeqLog( TYPE_FuncName_10, s_SendRecv_MsgParam.ul_MsgTypeID, pc_Request->ul_SubScenarioID, (ushort)e_Request_Result, (ushort)changeDetailCondeJSVCToAPF(e_ResultDetail), NULL );

	// アプリケーションプラットフォームへメッセージ通知する.
	JSVC_JobSendRecv::newInstance()->sendMessageToAPF(&s_SendRecv_MsgParam);
}
// ジョブ開始判断.
Bool JSVC_Request::isStartJob()
{
	Bool b_ret = True;

	// ファンクション情報を参照して、ジョブ開始を判断する.
	// 受信系のFunctionのような待ちを必要とするジョブの場合、ジョブ開始NGとする.
	// 従来のAPIC_Job::req_startのタイミングでジョブ開始OKとする.

	if (NULL == GetParentRequest())	{
		#if DEBUGLOG__JOBSTART_JUDGE
			MyTraceSD(JSVD_DBG_ERROR, "JSVC_Request::isStartJob GetParentRequest=NULL",__LINE__ );
		#endif
		return False;
	}
	if (NULL == GetParentRequest()->getJobInstance()) {
		#if DEBUGLOG__JOBSTART_JUDGE
			MyTraceSD(JSVD_DBG_ERROR, "JSVC_Request::isStartJob JobPtr=NULL",__LINE__ );
		#endif
		return False;
	}

	if(( JSVE_Request_Status_AnalyzeEnd        != e_Request_Status )			// warning対応 2013/09/18 ooba.
//	if(( JSVE_Request_Event_AnsAnalyzeEnd      != e_Request_Status )
	&& ( JSVE_Request_Status_ChargeMapOpenWait != e_Request_Status ))
	{
		#if DEBUGLOG__JOBSTART_JUDGE
			MyTraceSD(JSVD_DBG_ERROR, "JSVC_Request::isStartJob  check1",__LINE__ );
		#endif
		return False;
	}
	
	if( False == checkJobSpec(GetParentRequest()->getJobInstance()->getJobSpec()) )
	{
		#if DEBUGLOG__JOBSTART_JUDGE
			MyTraceSD(JSVD_DBG_ERROR, "JSVC_Request::isStartJob  check2",__LINE__ );
		#endif
		return False;
	}

	return b_ret;
}

// Job開始のコールバック.
void JSVC_Request::ans_start(APIC_Job* pc_Job)
{
#ifdef _DEBUG_JSVC_SEQ
	MyTraceSLD(JSVD_DBG_INFO, "JSVC_Request::ans_start pc_Job:" , (intptr_t)pc_Job );		// 回避方法をトレースに変更.
#endif
}
// Job開始のコールバック.
void JSVC_Request::ans_start01(APIC_Job* pc_Job)
{
	char sc_index = 0 + JSVC_CB_INDEX_ans_start;
	ans_start_wrap(pc_Job,sc_index);
}
void JSVC_Request::ans_start02(APIC_Job* pc_Job)
{
	char sc_index = 1 + JSVC_CB_INDEX_ans_start;
	ans_start_wrap(pc_Job,sc_index);
}
void JSVC_Request::ans_start03(APIC_Job* pc_Job)
{
	char sc_index = 2 + JSVC_CB_INDEX_ans_start;
	ans_start_wrap(pc_Job,sc_index);
}
void JSVC_Request::ans_start04(APIC_Job* pc_Job)
{
	char sc_index = 3 + JSVC_CB_INDEX_ans_start;
	ans_start_wrap(pc_Job,sc_index);
}
void JSVC_Request::ans_start05(APIC_Job* pc_Job)
{
	char sc_index = 4 + JSVC_CB_INDEX_ans_start;
	ans_start_wrap(pc_Job,sc_index);
}
void JSVC_Request::ans_start06(APIC_Job* pc_Job)
{
	char sc_index = 5 + JSVC_CB_INDEX_ans_start;
	ans_start_wrap(pc_Job,sc_index);
}
void JSVC_Request::ans_start07(APIC_Job* pc_Job)
{
	char sc_index = 6 + JSVC_CB_INDEX_ans_start;
	ans_start_wrap(pc_Job,sc_index);
}
void JSVC_Request::ans_start08(APIC_Job* pc_Job)
{
	char sc_index = 7 + JSVC_CB_INDEX_ans_start;
	ans_start_wrap(pc_Job,sc_index);
}
void JSVC_Request::ans_start09(APIC_Job* pc_Job)
{
	char sc_index = 8 + JSVC_CB_INDEX_ans_start;
	ans_start_wrap(pc_Job,sc_index);
}
void JSVC_Request::ans_start10(APIC_Job* pc_Job)
{
	char sc_index = 9 + JSVC_CB_INDEX_ans_start;
	ans_start_wrap(pc_Job,sc_index);
}
// Job開始のコールバック.
void JSVC_Request::ans_start_wrap(APIC_Job* pc_Job,char sc_index)
{
	JSVC_RequestBase* pReq = NULL;
	void*	vpDataBuf = NULL;
	short ss_DataLen = 0;
	Bool b_Status;

	//排他区間ここから.
	{
		JSVC_CBManagerSemCtrl cCBManage;
		//CB管理領域から呼出パラメータを取得.
		pReq		= JSVC_JobManager::c_CBManage[(uchar)sc_index].pc_ReqBase;
		vpDataBuf	= JSVC_JobManager::c_CBManage[(uchar)sc_index].vp_DataBuf;
		ss_DataLen	= JSVC_JobManager::c_CBManage[(uchar)sc_index].ss_DataLen;
		b_Status	= JSVC_JobManager::c_CBManage[(uchar)sc_index].b_Status;
		//CB管理領域を開放.
		JSVC_JobManager::c_CBManage[(uchar)sc_index].pc_ReqBase = NULL;
		JSVC_JobManager::c_CBManage[(uchar)sc_index].vp_DataBuf = NULL;
		JSVC_JobManager::c_CBManage[(uchar)sc_index].ss_DataLen = 0;
		JSVC_JobManager::c_CBManage[(uchar)sc_index].b_Status   = False;		//管理流域を「未使用」に戻す.
	}
	//排他区間ここまで.

	//実装関数をコール.
	if( b_Status == True){
		ans_start_body( pc_Job , pReq );
	}
	//バッファはここで確実に削除.
	if( vpDataBuf ){
//Str【 静的解析対応 : Sky花岡 : 2012.08.30 : CID-436325】.
//JSVC_JobManager::c_CBManage[sc_index].vp_DataBufが使用されていない為、delete処理を削除.
//JSVC_JobManager::c_CBManage[sc_index].vp_DataBufを使用する場合、下記、delete処理を復活させる事。
//なお、void* で指されている領域を開放する場合はfreeで開放する事（確保はmalloc）。
//		delete vpDataBuf;
//End【 静的解析対応 : Sky花岡 : 2012.08.30 : CID-436325】.
		vpDataBuf = NULL;
	}
}
void JSVC_Request::ans_start_body( APIC_Job* pc_Job , JSVC_RequestBase* pRequst )
{
	#if DEBUGLOG__FUNCTION_CALLED_BACK
		MyTraceSD(JSVD_DBG_INFO, "ans_start_body",pRequst);
	#endif
	JSVC_Request* pReq = (JSVC_Request*)pRequst;
	uint32_t	e_FuncSts = 0;
	
	//パラメータチェックのみ.
	if (NULL != pReq  )
	{
		if (NULL != pReq->getJobInstance()) {
			if (NULL != pc_Job)
			{
				MyTraceSD(JSVD_DBG_INFO, "ans_start JobID", (int)pc_Job->getJobStatus()->getJobID() );
				pReq->ul_Result = JSVE_Request_Result_Ok;
				pReq->ul_ResultDetail = JSVE_JobServiceResultDetail_Non;
				// 2013/01/30 CQ377138対応 Jobインスタンス取得不可時のシナリオ登録応答 modify start.
				// 2012/09/21 CQ372110 IWSへのans対応 add start.
				// Jobのans_startを受けた通知を行う.
				//pReq->pc_JobManager->NotifyJobAnsStart(JSVE_Request_Result_Ok, pReq->getJobInstance());
				pReq->pc_JobManager->NotifyJobAnsStart(JSVE_Request_Result_Ok, pReq);
				// 2012/09/21 CQ372110 IWSへのans対応 add end.
				// 2013/01/30 CQ377138対応 Jobインスタンス取得不可時のシナリオ登録応答 modify end.
				pReq->pc_JobManager->notify( pReq , pReq->ul_Result , pReq->ul_ResultDetail , JSVE_JobManager_MsgID_StartAccept , e_FuncSts );
			} else{
				pReq->ul_Result = JSVE_Request_Result_Err;
				pReq->ul_ResultDetail = JSVE_JobServiceResultDetail_StartErr;
				MyTraceSS(JSVD_DBG_ERROR, "ans_start","NG1");
				// 2013/01/30 CQ377138対応 Jobインスタンス取得不可時のシナリオ登録応答 modify start.
				// 2012/09/21 CQ372110 IWSへのans対応 add start.
				// Jobのans_startを受けた通知を行う.
				//pReq->pc_JobManager->NotifyJobAnsStart(JSVE_Request_Result_Err, pReq->getJobInstance());
				pReq->pc_JobManager->NotifyJobAnsStart(JSVE_Request_Result_Err, pReq);
				// 2012/09/21 CQ372110 IWSへのans対応 add end.
				// 2013/01/30 CQ377138対応 Jobインスタンス取得不可時のシナリオ登録応答 modify end.
				pReq->pc_JobManager->notify( pReq , pReq->ul_Result , pReq->ul_ResultDetail , JSVE_JobManager_MsgID_MemoryAllocEnd , e_FuncSts );
			}
		} else {
			// Jobポインタが取得出来ないとどうすることも出来ない.
// 2016/03/24 CQ526145 共有Box保存中に個人Boxから200文書一括印刷でE301が発生する問題を対策 ht_agotou Start.
			printf("★★★[JOBS] JSVC_Request::ans_start_body pReq->getJobInstance() NULL!\n");
// 2016/03/24 CQ526145 共有Box保存中に個人Boxから200文書一括印刷でE301が発生する問題を対策 ht_agotou End.
		}
	} else {
		MyTraceSS(JSVD_DBG_ERROR, "ans_start","NG2");
	}
}
void JSVC_Request::ans_stop()
{
}
void JSVC_Request::ans_stop01()
{
	char sc_index = 0 + JSVC_CB_INDEX_ans_stop;
	ans_stop_wrap(sc_index);
}
void JSVC_Request::ans_stop02()
{
	char sc_index = 1 + JSVC_CB_INDEX_ans_stop;
	ans_stop_wrap(sc_index);
}
void JSVC_Request::ans_stop03()
{
	char sc_index = 2 + JSVC_CB_INDEX_ans_stop;
	ans_stop_wrap(sc_index);
}
void JSVC_Request::ans_stop04()
{
	char sc_index = 3 + JSVC_CB_INDEX_ans_stop;
	ans_stop_wrap(sc_index);
}
void JSVC_Request::ans_stop05()
{
	char sc_index = 4 + JSVC_CB_INDEX_ans_stop;
	ans_stop_wrap(sc_index);
}
void JSVC_Request::ans_stop06()
{
	char sc_index = 5 + JSVC_CB_INDEX_ans_stop;
	ans_stop_wrap(sc_index);
}
void JSVC_Request::ans_stop07()
{
	char sc_index = 6 + JSVC_CB_INDEX_ans_stop;
	ans_stop_wrap(sc_index);
}
void JSVC_Request::ans_stop08()
{
	char sc_index = 7 + JSVC_CB_INDEX_ans_stop;
	ans_stop_wrap(sc_index);
}
void JSVC_Request::ans_stop09()
{
	char sc_index = 8 + JSVC_CB_INDEX_ans_stop;
	ans_stop_wrap(sc_index);
}
void JSVC_Request::ans_stop10()
{
	char sc_index = 9 + JSVC_CB_INDEX_ans_stop;
	ans_stop_wrap(sc_index);
}
void JSVC_Request::ans_stop_wrap(char sc_index)
{
	JSVC_RequestBase* pReq = NULL;
	void*	vpDataBuf = NULL;
	short ss_DataLen = 0;
	Bool b_Status;

	//排他区間ここから.
	{
		JSVC_CBManagerSemCtrl cCBManage;
		//CB管理領域から呼出パラメータを取得.
		pReq		= JSVC_JobManager::c_CBManage[(uchar)sc_index].pc_ReqBase;
		vpDataBuf	= JSVC_JobManager::c_CBManage[(uchar)sc_index].vp_DataBuf;
		ss_DataLen	= JSVC_JobManager::c_CBManage[(uchar)sc_index].ss_DataLen;
		b_Status	= JSVC_JobManager::c_CBManage[(uchar)sc_index].b_Status;
		//CB管理領域を開放.
		JSVC_JobManager::c_CBManage[(uchar)sc_index].pc_ReqBase = NULL;
		JSVC_JobManager::c_CBManage[(uchar)sc_index].vp_DataBuf = NULL;
		JSVC_JobManager::c_CBManage[(uchar)sc_index].ss_DataLen = 0;
		JSVC_JobManager::c_CBManage[(uchar)sc_index].b_Status   = False;		//管理流域を「未使用」に戻す.
	}
	//排他区間ここまで.

	//実装関数をコール.
	if( b_Status == True){
		ans_stop_body( pReq );
	}
	//バッファはここで確実に削除.
	if( vpDataBuf ){
//Str【 静的解析対応 : Sky花岡 : 2012.08.30 : CID-436324】.
//JSVC_JobManager::c_CBManage[sc_index].vp_DataBufが使用されていない為、delete処理を削除.
//JSVC_JobManager::c_CBManage[sc_index].vp_DataBufを使用する場合、下記、delete処理を復活させる事。
//なお、void* で指されている領域を開放する場合はfreeで開放する事（確保はmalloc）。
//		delete vpDataBuf;
//End【 静的解析対応 : Sky花岡 : 2012.08.30 : CID-436324】.
		vpDataBuf = NULL;
	}
}
void JSVC_Request::ans_stop_body( JSVC_RequestBase* pReq )
{
	#if DEBUGLOG__FUNCTION_CALLED_BACK
		MyTraceSD(JSVD_DBG_INFO, "ans_stop_body",pReq);
	#endif
	// JobManagerでは不要な通知のため、通知は捨てる.
//DEL-Start Step2 CR.
//	printf("ans_stop");
//DEL-End Step2 CR.
	MyTraceSLD(JSVD_DBG_INFO, "ans_stop",(intptr_t)pReq);
}

void JSVC_Request::ans_cancel(APIC_Job* pc_Job)
{
	// JobManagerでは不要な通知のため、通知は捨てる.
	if (NULL != pc_Job){
		MyTraceSD(JSVD_DBG_INFO, "ans_cancel JobID", (int)pc_Job->getJobStatus()->getJobID() );
	}
  #ifdef _DEBUG_JSVC_SEQ
	else{
		MyTraceSLD(JSVD_DBG_ERROR, "ans_stop NG",(intptr_t)pc_Job);
	}
  #endif
}

void JSVC_Request::ans_cancel01(APIC_Job* pc_Job)
{
	char sc_index = 0 + JSVC_CB_INDEX_ans_cancel;
	ans_cancel_wrap( pc_Job, sc_index);
}
void JSVC_Request::ans_cancel02(APIC_Job* pc_Job)
{
	char sc_index = 1 + JSVC_CB_INDEX_ans_cancel;
	ans_cancel_wrap( pc_Job, sc_index);
}
void JSVC_Request::ans_cancel03(APIC_Job* pc_Job)
{
	char sc_index = 2 + JSVC_CB_INDEX_ans_cancel;
	ans_cancel_wrap( pc_Job, sc_index);
}
void JSVC_Request::ans_cancel04(APIC_Job* pc_Job)
{
	char sc_index = 3 + JSVC_CB_INDEX_ans_cancel;
	ans_cancel_wrap( pc_Job, sc_index);
}
void JSVC_Request::ans_cancel05(APIC_Job* pc_Job)
{
	char sc_index = 4 + JSVC_CB_INDEX_ans_cancel;
	ans_cancel_wrap( pc_Job, sc_index);
}
void JSVC_Request::ans_cancel06(APIC_Job* pc_Job)
{
	char sc_index = 5 + JSVC_CB_INDEX_ans_cancel;
	ans_cancel_wrap( pc_Job, sc_index);
}
void JSVC_Request::ans_cancel07(APIC_Job* pc_Job)
{
	char sc_index = 6 + JSVC_CB_INDEX_ans_cancel;
	ans_cancel_wrap( pc_Job, sc_index);
}
void JSVC_Request::ans_cancel08(APIC_Job* pc_Job)
{
	char sc_index = 7 + JSVC_CB_INDEX_ans_cancel;
	ans_cancel_wrap( pc_Job, sc_index);
}
void JSVC_Request::ans_cancel09(APIC_Job* pc_Job)
{
	char sc_index = 8 + JSVC_CB_INDEX_ans_cancel;
	ans_cancel_wrap( pc_Job, sc_index);
}
void JSVC_Request::ans_cancel10(APIC_Job* pc_Job)
{
	char sc_index = 9 + JSVC_CB_INDEX_ans_cancel;
	ans_cancel_wrap( pc_Job, sc_index);
}
void JSVC_Request::ans_cancel_wrap(APIC_Job* pc_Job, char sc_index)
{
	JSVC_RequestBase* pReq = NULL;
	void*	vpDataBuf = NULL;
	short ss_DataLen = 0;
	Bool b_Status;

	//排他区間ここから.
	{
		JSVC_CBManagerSemCtrl cCBManage;
		//CB管理領域から呼出パラメータを取得.
		pReq		= JSVC_JobManager::c_CBManage[(uchar)sc_index].pc_ReqBase;
		vpDataBuf	= JSVC_JobManager::c_CBManage[(uchar)sc_index].vp_DataBuf;
		ss_DataLen	= JSVC_JobManager::c_CBManage[(uchar)sc_index].ss_DataLen;
		b_Status	= JSVC_JobManager::c_CBManage[(uchar)sc_index].b_Status;
		//CB管理領域を開放.
		JSVC_JobManager::c_CBManage[(uchar)sc_index].pc_ReqBase = NULL;
		JSVC_JobManager::c_CBManage[(uchar)sc_index].vp_DataBuf = NULL;
		JSVC_JobManager::c_CBManage[(uchar)sc_index].ss_DataLen = 0;
		JSVC_JobManager::c_CBManage[(uchar)sc_index].b_Status   = False;		//管理流域を「未使用」に戻す.
	}
	//排他区間ここまで.

	//実装関数をコール.
	if( b_Status == True){
		ans_cancel_body( pc_Job, pReq );
	}
	//バッファはここで確実に削除.
	if( vpDataBuf ){
//Str【 静的解析対応 : Sky花岡 : 2012.08.30 : CID-436326】.
//JSVC_JobManager::c_CBManage[sc_index].vp_DataBufが使用されていない為、delete処理を削除.
//JSVC_JobManager::c_CBManage[sc_index].vp_DataBufを使用する場合、下記、delete処理を復活させる事。
//なお、void* で指されている領域を開放する場合はfreeで開放する事（確保はmalloc）。
//		delete vpDataBuf;
//End【 静的解析対応 : Sky花岡 : 2012.08.30 : CID-436326】.
		vpDataBuf = NULL;
	}
}
void JSVC_Request::ans_cancel_body( APIC_Job* pc_Job, JSVC_RequestBase* pReq )
{
	#if DEBUGLOG__FUNCTION_CALLED_BACK
		MyTraceSD(JSVD_DBG_INFO, "ans_cancel_body",pReq);
	#endif
#ifdef _DEBUG_JSVC_SEQ
	MyTraceSLD(JSVD_DBG_INFO, "JSVC_Request::ans_cancel_body pReq:" , (intptr_t)pReq );		// 回避方法をトレースに変更.
#endif
	// JobManagerでは不要な通知のため、通知は捨てる.
	if (NULL != pc_Job){
		MyTraceSD(JSVD_DBG_INFO, "ans_cancel JobID", (int)pc_Job->getJobStatus()->getJobID() );
	}
  #ifdef _DEBUG_JSVC_SEQ
	else{
		MyTraceSLD(JSVD_DBG_ERROR, "ans_cancel NG",(intptr_t)pc_Job);
	}
  #endif
}

void JSVC_Request::ans_changeExecTableInfo01(APIC_Job* pc_Job , TYPE_ChangeExecTableInfoResult e_ChangeExecTableInfoResult)
{
	char sc_index = 0 + JSVC_CB_INDEX_ans_changeExecTableInfo;
	ans_changeExecTableInfo_wrap(pc_Job,sc_index,e_ChangeExecTableInfoResult);
}
void JSVC_Request::ans_changeExecTableInfo02(APIC_Job* pc_Job , TYPE_ChangeExecTableInfoResult e_ChangeExecTableInfoResult)
{
	char sc_index = 1 + JSVC_CB_INDEX_ans_changeExecTableInfo;
	ans_changeExecTableInfo_wrap(pc_Job,sc_index,e_ChangeExecTableInfoResult);
}
void JSVC_Request::ans_changeExecTableInfo03(APIC_Job* pc_Job , TYPE_ChangeExecTableInfoResult e_ChangeExecTableInfoResult)
{
	char sc_index = 2 + JSVC_CB_INDEX_ans_changeExecTableInfo;
	ans_changeExecTableInfo_wrap(pc_Job,sc_index,e_ChangeExecTableInfoResult);
}
void JSVC_Request::ans_changeExecTableInfo04(APIC_Job* pc_Job , TYPE_ChangeExecTableInfoResult e_ChangeExecTableInfoResult)
{
	char sc_index = 3 + JSVC_CB_INDEX_ans_changeExecTableInfo;
	ans_changeExecTableInfo_wrap(pc_Job,sc_index,e_ChangeExecTableInfoResult);
}
void JSVC_Request::ans_changeExecTableInfo05(APIC_Job* pc_Job , TYPE_ChangeExecTableInfoResult e_ChangeExecTableInfoResult)
{
	char sc_index = 4 + JSVC_CB_INDEX_ans_changeExecTableInfo;
	ans_changeExecTableInfo_wrap(pc_Job,sc_index,e_ChangeExecTableInfoResult);
}
void JSVC_Request::ans_changeExecTableInfo06(APIC_Job* pc_Job , TYPE_ChangeExecTableInfoResult e_ChangeExecTableInfoResult)
{
	char sc_index = 5 + JSVC_CB_INDEX_ans_changeExecTableInfo;
	ans_changeExecTableInfo_wrap(pc_Job,sc_index,e_ChangeExecTableInfoResult);
}
void JSVC_Request::ans_changeExecTableInfo07(APIC_Job* pc_Job , TYPE_ChangeExecTableInfoResult e_ChangeExecTableInfoResult)
{
	char sc_index = 6 + JSVC_CB_INDEX_ans_changeExecTableInfo;
	ans_changeExecTableInfo_wrap(pc_Job,sc_index,e_ChangeExecTableInfoResult);
}
void JSVC_Request::ans_changeExecTableInfo08(APIC_Job* pc_Job , TYPE_ChangeExecTableInfoResult e_ChangeExecTableInfoResult)
{
	char sc_index = 7 + JSVC_CB_INDEX_ans_changeExecTableInfo;
	ans_changeExecTableInfo_wrap(pc_Job,sc_index,e_ChangeExecTableInfoResult);
}
void JSVC_Request::ans_changeExecTableInfo09(APIC_Job* pc_Job , TYPE_ChangeExecTableInfoResult e_ChangeExecTableInfoResult)
{
	char sc_index = 8 + JSVC_CB_INDEX_ans_changeExecTableInfo;
	ans_changeExecTableInfo_wrap(pc_Job,sc_index,e_ChangeExecTableInfoResult);
}
void JSVC_Request::ans_changeExecTableInfo10(APIC_Job* pc_Job , TYPE_ChangeExecTableInfoResult e_ChangeExecTableInfoResult)
{
	char sc_index = 9 + JSVC_CB_INDEX_ans_changeExecTableInfo;
	ans_changeExecTableInfo_wrap(pc_Job,sc_index,e_ChangeExecTableInfoResult);
}
void JSVC_Request::ans_changeExecTableInfo_wrap( APIC_Job* pc_Job , char sc_index, TYPE_ChangeExecTableInfoResult e_ChangeExecTableInfoResult)
{
#ifdef _DEBUG_JSVC_SEQ
	MyTraceSLD(JSVD_DBG_INFO, "JSVC_Request::ans_changeExecTableInfo_wrap pc_Job:" , (intptr_t)pc_Job );												// 回避方法をトレースに変更.
	MyTraceSD(JSVD_DBG_INFO, "JSVC_Request::ans_changeExecTableInfo_wrap sc_index:" , (int)sc_index );											// 回避方法をトレースに変更.
	MyTraceSD(JSVD_DBG_INFO, "JSVC_Request::ans_changeExecTableInfo_wrap e_ChangeExecTableInfoResult:" , (int)e_ChangeExecTableInfoResult );	// 回避方法をトレースに変更.
#endif
}

//JOB側の確認処理とJobSpecの変更は同期で行う必要があるため、.
//このコールバックはJobManagerへ通知せずに一気に処理を行う.
void JSVC_Request::ans_changeExecTableInfo( APIC_Job* pc_Job ,TYPE_ChangeExecTableInfoResult e_ChangeExecTableInfoResult )
{
#if DEBUGLOG__FUNCTION_CALLED_BACK
	MyTraceSLD(JSVD_DBG_INFO, "ans_changeExecTableInfo_body",(intptr_t)pReq);
#endif
#ifdef _DEBUG_JSVC_SEQ
	MyTraceSLD(JSVD_DBG_INFO, "JSVC_Request::ans_changeExecTableInfo pc_Job:" , (intptr_t)pc_Job );
#endif
	if( NULL == pc_Job )
	{
		return;
	}

	JSVC_RequestBase* pReq = NULL;
	pReq = JSVC_JobManager::newInstance()->getRequestListByMsg( pc_Job->getJobStatus()->getSubScenarioID() , JSVE_JobManager_MsgID_ChangeSetting );
	if( NULL == pReq )
	{
		return;
	}

#ifdef _DEBUG_JSVC_SEQ
	MyTraceSLD(JSVD_DBG_INFO, "JSVC_Request::ans_changeExecTableInfo pReq:" , (intptr_t)pReq );
#endif

	//設定変更リクエストの返信を行う.
	JSVC_RequestJobSetting* pRequest = (JSVC_RequestJobSetting*)pReq;
	//設定変更完了（Job->JS）を未送信なら処理実施.
	if( False == pRequest->getSendUpdateEnd() )
	{
		//返信用のレスポンスクラスを用意(非同期処理のため、動的メモリ化).
		JSVC_ResponseJobNotify* pcRes = new JSVC_ResponseJobNotify(JSVE_JobManager_MsgID_ChangeSettingEnd);
		#if DEBUGLOG__REQUEST_LIFE
			MyTraceSLD(JSVD_DBG_INFO, "ans_changeExecTableInfo new JSVC_ResponseJobNotify" , (intptr_t)pcRes );
		#endif
		
		//結果のチェック.
		if (TYPE_ChangeExecTableInfoResult_OK == e_ChangeExecTableInfoResult) {
			//OK.
			pcRes->setResult( JSVE_JobServiceResult_OK );
			pcRes->setResultDetail( JSVE_JobServiceResultDetail_Non );

			JSVC_ParentRequest* pc_PRequest = NULL;
			pc_PRequest = JSVC_JobManager::newInstance()->getParentRequest(pRequest->getSubScenarioID());
			//Str【 静的解析対応 : Sky花岡 : 2012.09.03 : CID-439622】.
			if( ( NULL != pc_PRequest )
			&& ( NULL != pc_PRequest->getJobSpec() )
			&& ( True == pc_PRequest->getJobSpec()->bReadJobData )){
			//End【 静的解析対応 : Sky花岡 : 2012.09.03 : CID-439622】.
				pc_Job->setTypcJobData(&(pc_PRequest->getJobSpec()->cJobData));
			}
		} else if ( TYPE_ChangeExecTableInfoResult_FunctionStatusErr == e_ChangeExecTableInfoResult ) {
			//NG(実行中もしくは実行済).
			pcRes->setResult( JSVE_JobServiceResult_NG );
			pcRes->setResultDetail( JSVE_JobServiceResultDetail_Exec );	//エラー詳細は「サブシナリオ実行中」で代用.
		} else {
			//NG.
			pcRes->setResult( JSVE_JobServiceResult_NG );
			pcRes->setResultDetail( JSVE_JobServiceResultDetail_InputErr );
		}

		//JobManagerへ設定変更完了を送信.	
		MyTraceSS(JSVD_DBG_INFO, "自身へ設定変更完了を通知","");
		JSVS_SendRecv_MsgParam s_MsgParam;
		s_MsgParam.ul_MsgTypeID	= JSVE_JobManager_MsgID_ChangeSettingEnd;
		s_MsgParam.pReq			= pReq;
		s_MsgParam.pRes			= (JSVC_ResponseBase*)pcRes;
		JSVC_JobManager::newInstance()->sendMessageToJob( &s_MsgParam );
		//設定変更完了（Job->JS）を送信済にする.
		pRequest->setSendUpdateEnd();
	}

	// 設定変更が完了したらジョブに再開のトリガをかける.
	if(( TYPE_ChangeExecTableInfoResult_OK == e_ChangeExecTableInfoResult )
	&& (
	    ( ((JSVC_Request*)pReq)->getRequestStatus() == JSVE_Request_Status_ReqStart			)	///< (6)サブシナリオ開始要求中.
	  ||( ((JSVC_Request*)pReq)->getRequestStatus() == JSVE_Request_Status_Exec				)	///< (7)サブシナリオ実行中.
	  ||( ((JSVC_Request*)pReq)->getRequestStatus() == JSVE_Request_Status_Update			)	///< (10)サブシナリオ設定変更要求中.
	  ||( ((JSVC_Request*)pReq)->getRequestStatus() == JSVE_Request_Status_ReqRestart		)	///< (11)サブシナリオ再開要求中.
	  ||( ((JSVC_Request*)pReq)->getRequestStatus() == JSVE_Request_Status_RcvWait1			)	///< (16)受信開始待ち.
	  ||( ((JSVC_Request*)pReq)->getRequestStatus() == JSVE_Request_Status_RcvWait2			)	///< (17)受信準備中.
	  ||( ((JSVC_Request*)pReq)->getRequestStatus() == JSVE_Request_Status_RcvCall			)	///< (18)着信待ち.
	  ||( ((JSVC_Request*)pReq)->getRequestStatus() == JSVE_Request_Status_AnalizeHeader		)	///< (19)ヘッダ受信中.
	  ||( ((JSVC_Request*)pReq)->getRequestStatus() == JSVE_Request_Status_ChargeMapOpenWait	))	///< (22)課金MAP展開待ち.
	){
		//設定変更が成功、かつ、JobServiceが実行中と判断しているなら、ジョブへ再開トリガを発行.
		MyTraceSS(JSVD_DBG_INFO, "JSVC_Request::ans_changeExecTableInfo","ジョブへ再開のトリガを発行");
		pc_Job->restart();
	}
}

// 課金サービス仕様変更対応 Start.
void JSVC_Request::ans_CallbackExpandChargeMap01(uint32_t ul_SubScenarioId, SYSE_ChgIFResultStatus e_Result)
{
	char sc_index = 0 + JSVC_CB_INDEX_ans_CallbackExpandChargeMap;
	ans_CallbackExpandChargeMap_wrap(ul_SubScenarioId, e_Result, sc_index);
}
void JSVC_Request::ans_CallbackExpandChargeMap02(uint32_t ul_SubScenarioId, SYSE_ChgIFResultStatus e_Result)
{
	char sc_index = 1 + JSVC_CB_INDEX_ans_CallbackExpandChargeMap;
	ans_CallbackExpandChargeMap_wrap(ul_SubScenarioId, e_Result, sc_index);
}
void JSVC_Request::ans_CallbackExpandChargeMap03(uint32_t ul_SubScenarioId, SYSE_ChgIFResultStatus e_Result)
{
	char sc_index = 2 + JSVC_CB_INDEX_ans_CallbackExpandChargeMap;
	ans_CallbackExpandChargeMap_wrap(ul_SubScenarioId, e_Result, sc_index);
}
void JSVC_Request::ans_CallbackExpandChargeMap04(uint32_t ul_SubScenarioId, SYSE_ChgIFResultStatus e_Result)
{
	char sc_index = 3 + JSVC_CB_INDEX_ans_CallbackExpandChargeMap;
	ans_CallbackExpandChargeMap_wrap(ul_SubScenarioId, e_Result, sc_index);
}
void JSVC_Request::ans_CallbackExpandChargeMap05(uint32_t ul_SubScenarioId, SYSE_ChgIFResultStatus e_Result)
{
	char sc_index = 4 + JSVC_CB_INDEX_ans_CallbackExpandChargeMap;
	ans_CallbackExpandChargeMap_wrap(ul_SubScenarioId, e_Result, sc_index);
}
void JSVC_Request::ans_CallbackExpandChargeMap06(uint32_t ul_SubScenarioId, SYSE_ChgIFResultStatus e_Result)
{
	char sc_index = 5 + JSVC_CB_INDEX_ans_CallbackExpandChargeMap;
	ans_CallbackExpandChargeMap_wrap(ul_SubScenarioId, e_Result, sc_index);
}
void JSVC_Request::ans_CallbackExpandChargeMap07(uint32_t ul_SubScenarioId, SYSE_ChgIFResultStatus e_Result)
{
	char sc_index = 6 + JSVC_CB_INDEX_ans_CallbackExpandChargeMap;
	ans_CallbackExpandChargeMap_wrap(ul_SubScenarioId, e_Result, sc_index);
}
void JSVC_Request::ans_CallbackExpandChargeMap08(uint32_t ul_SubScenarioId, SYSE_ChgIFResultStatus e_Result)
{
	char sc_index = 7 + JSVC_CB_INDEX_ans_CallbackExpandChargeMap;
	ans_CallbackExpandChargeMap_wrap(ul_SubScenarioId, e_Result, sc_index);
}
void JSVC_Request::ans_CallbackExpandChargeMap09(uint32_t ul_SubScenarioId, SYSE_ChgIFResultStatus e_Result)
{
	char sc_index = 8 + JSVC_CB_INDEX_ans_CallbackExpandChargeMap;
	ans_CallbackExpandChargeMap_wrap(ul_SubScenarioId, e_Result, sc_index);
}
void JSVC_Request::ans_CallbackExpandChargeMap10(uint32_t ul_SubScenarioId, SYSE_ChgIFResultStatus e_Result)
{
	char sc_index = 9 + JSVC_CB_INDEX_ans_CallbackExpandChargeMap;
	ans_CallbackExpandChargeMap_wrap(ul_SubScenarioId, e_Result, sc_index);
}
void JSVC_Request::ans_CallbackExpandChargeMap_wrap(uint32_t ul_SubScenarioId, SYSE_ChgIFResultStatus e_Result, char sc_index)
{
	JSVC_RequestBase* pReq = NULL;
	void*	vpDataBuf = NULL;
	short ss_DataLen = 0;
	Bool b_Status;

	//排他区間ここから.
	{
		JSVC_CBManagerSemCtrl cCBManage;
		//CB管理領域から呼出パラメータを取得.
		pReq		= JSVC_JobManager::c_CBManage[(uchar)sc_index].pc_ReqBase;
		vpDataBuf	= JSVC_JobManager::c_CBManage[(uchar)sc_index].vp_DataBuf;
		ss_DataLen	= JSVC_JobManager::c_CBManage[(uchar)sc_index].ss_DataLen;
		b_Status	= JSVC_JobManager::c_CBManage[(uchar)sc_index].b_Status;
		//CB管理領域を開放.
		JSVC_JobManager::c_CBManage[(uchar)sc_index].pc_ReqBase = NULL;
		JSVC_JobManager::c_CBManage[(uchar)sc_index].vp_DataBuf = NULL;
		JSVC_JobManager::c_CBManage[(uchar)sc_index].ss_DataLen = 0;
		JSVC_JobManager::c_CBManage[(uchar)sc_index].b_Status   = False;		//管理流域を「未使用」に戻す.
	}
	//排他区間ここまで.

	//実装関数をコール.
	if( b_Status == True){
		ans_CallbackExpandChargeMap_body( ul_SubScenarioId, e_Result, pReq );
	}
	//バッファはここで確実に削除.
	if( vpDataBuf ){
//Str【 静的解析対応 : Sky花岡 : 2012.08.30 : CID-436327】.
//JSVC_JobManager::c_CBManage[sc_index].vp_DataBufが使用されていない為、delete処理を削除.
//JSVC_JobManager::c_CBManage[sc_index].vp_DataBufを使用する場合、下記、delete処理を復活させる事。
//なお、void* で指されている領域を開放する場合はfreeで開放する事（確保はmalloc）。
//		delete vpDataBuf;
//End【 静的解析対応 : Sky花岡 : 2012.08.30 : CID-436327】.
		vpDataBuf = NULL;
	}
}
// 課金マップのコールバック.
void JSVC_Request::ans_CallbackExpandChargeMap_body( uint32_t ul_SubScenarioId, SYSE_ChgIFResultStatus e_Result, JSVC_RequestBase* pReq )
{
	#if DEBUGLOG__JOBSTART_JUDGE || DEBUGLOG__FUNCTION_CALLED_BACK
		MyTraceSD(JSVD_DBG_INFO, "JSVC_Request::ans_CallbackExpandChargeMap_body rerusl=",e_Result );
	#endif

	//パラメータチェックのみ.
	if (NULL != pReq  )
	{
		pReq->setResult( JSVE_Request_Result_Err );
		pReq->setResultDetail( JSVE_JobServiceResultDetail_StartErr );
		if (JSVE_Request_Class_Type_Req_Job_Set == pReq->WhoAmI() )
		{
			if ((0 == ul_SubScenarioId)
			||  ( SYSE_ChgIFResultStatus_OK != e_Result))
			{
				MyTraceSD(JSVD_DBG_ERROR, "ans_CallbackExpandChargeMap_body NG1",(int)ul_SubScenarioId);
			} else{
				pReq->setResult( JSVE_Request_Result_Ok );
				pReq->setResultDetail( JSVE_JobServiceResultDetail_Non );
			}
		} else {
			MyTraceSD(JSVD_DBG_ERROR, "ans_CallbackExpandChargeMap_body NG2",e_Result);
		}
		JSVC_JobManager::newInstance()->notify( pReq , pReq->getResult() , pReq->getResultDetail() , JSVE_JobManager_MsgID_ChargeMapPrepared , 0 );
		
	} else {
		MyTraceSD(JSVD_DBG_ERROR, "ans_CallbackExpandChargeMap_body NG3",e_Result);
	}
}
// 課金サービス仕様変更対応 End.

void JSVC_Request::ans_Analyze( uint32_t ul_SubScenarioID, JSVC_Request* pc_Req, char sc_ret, uint32_t ul_retDetail )
{
	#if DEBUGLOG__XML_ANALIZE
		MyTraceSS(JSVD_DBG_INFNO, "ans_Analyze" , "call" );
	#endif

	// JobManagerへメッセージ送信を行う.
	uint32_t ul_Result = JSVE_JobServiceResult_OK;
	uint32_t ul_ResultDetail = JSVE_JobServiceResultDetail_Non;

	if ( sc_ret != TYPE_XmlResult_RetOK){
		ul_Result = JSVE_JobServiceResult_NG;
	  #ifdef _DEBUG_JSVC_SEQ
		MyTraceSD(JSVD_DBG_ERROR, "ans_Analyze NG",(int)sc_ret);
	  #endif
	}
	ul_ResultDetail = ul_retDetail;
	JSVE_JobManager_MsgID e_JobManager_MsgID = JSVE_JobManager_MsgID_AnsXmlAnalyze;		// XML解析応答.
	uint32_t	e_FuncSts = 0;

	JSVC_JobManager::newInstance()->notify( pc_Req , ul_Result , ul_ResultDetail , e_JobManager_MsgID , e_FuncSts );


	// ログ.
	_setJsrvSeqLog( TYPE_FuncName_11, ul_SubScenarioID, 0, (ushort)sc_ret, (ushort)ul_retDetail, NULL );
}
// ローカルイベント変換処理.
// JobManagerのMessageIDを内部イベントに変換する.
JSVE_Request_Event JSVC_Request::changeLocalEvent(JSVE_JobManager_MsgID e_JobManager_MsgTypeID)
{
	JSVE_Request_Event e_Request_Event = JSVE_Request_Event_Non;

	switch(e_JobManager_MsgTypeID)
	{
	case JSVE_JobManager_MsgID_MemoryAlloc			:	e_Request_Event = JSVE_Request_Event_ReqMemAlloc;
		break;
	case JSVE_JobManager_MsgID_Start				:	e_Request_Event = JSVE_Request_Event_ReqStart;
		break;
	case JSVE_JobManager_MsgID_Stop					:	e_Request_Event = JSVE_Request_Event_ReqStop;
		break;
	case JSVE_JobManager_MsgID_ReStart				:	e_Request_Event = JSVE_Request_Event_ReqRestart;
		break;
	case JSVE_JobManager_MsgID_Cancel				:	e_Request_Event = JSVE_Request_Event_ReqCancel;
		break;
	case JSVE_JobManager_MsgID_ChangeSetting		:	e_Request_Event = JSVE_Request_Event_ReqUpdate;
		break;
	case JSVE_JobManager_MsgID_MemoryDelete			:	e_Request_Event = JSVE_Request_Event_ReqMemFree;
		break;
	case JSVE_JobManager_MsgID_MemoryAllocAccept	:	e_Request_Event = JSVE_RequestAnswer_Event_MemoryAllocAccept;
		break;
	case JSVE_JobManager_MsgID_MemoryAllocEnd		:	e_Request_Event = JSVE_Request_Event_AnsMemAllocEnd;
		break;
	case JSVE_JobManager_MsgID_StartAccept			:	e_Request_Event = JSVE_RequestAnswer_Event_StartAccept;
		break;
	case JSVE_JobManager_MsgID_End					:	e_Request_Event = JSVE_Request_Event_AnsEnd;
		break;
	case JSVE_JobManager_MsgID_StopAccept			:	e_Request_Event = JSVE_RequestAnswer_Event_StopAccept;
		break;
	case JSVE_JobManager_MsgID_StopEnd				:	e_Request_Event = JSVE_Request_Event_AnsStopEnd;
		break;
	case JSVE_JobManager_MsgID_ReStartAccept		:	e_Request_Event = JSVE_RequestAnswer_Event_ReStartAccept;
		break;
	case JSVE_JobManager_MsgID_CancelAccept			:	e_Request_Event = JSVE_RequestAnswer_Event_CancelAccept;
		break;
	case JSVE_JobManager_MsgID_CancelEnd			:	e_Request_Event = JSVE_Request_Event_AnsCancel;
		break;
	case JSVE_JobManager_MsgID_ChangeSettingAccept	:	e_Request_Event = JSVE_RequestAnswer_Event_ChangeSettingAccept;
		break;
	case JSVE_JobManager_MsgID_ChangeSettingEnd		:	e_Request_Event = JSVE_Request_Event_AnsUpdateEnd;
		break;
	case JSVE_JobManager_MsgID_MemoryDeleteAccept	:	e_Request_Event = JSVE_RequestAnswer_Event_MemoryDeleteAccept;
		break;
	case JSVE_JobManager_MsgID_MemoryDeleteEnd		:	e_Request_Event = JSVE_Request_Event_AnsMemFree;
		break;
	case JSVE_JobManager_MsgID_AnsXmlAnalyze		:	e_Request_Event = JSVE_Request_Event_AnsAnalyzeEnd;
		break;
	case JSVE_JobManager_MsgID_StatusChange			:	e_Request_Event = JSVE_RequestAnswer_Event_StatusChange;
		break;
	case JSVE_JobManager_MsgID_FunctionStatusChange	:	e_Request_Event = JSVE_RequestAnswer_Event_FunctionStatusChange;
		break;
	case JSVE_JobManager_MsgID_Warning				:	e_Request_Event = JSVE_RequestAnswer_Event_Warning;
		break;
	case JSVE_JobManager_MsgID_FunctionInfo			:
		break;
	case JSVE_JobManager_MsgID_FunctionInfoAccept	:
		break;
	case JSVE_JobManager_MsgID_ChargeMapPrepared	:	e_Request_Event = JSVE_RequestAnswer_Event_ChargeMapPrepared;
		break;
	case JSVE_JobManager_MsgID_newJobInstance		:	e_Request_Event = JSVE_RequestAnswer_Event_NewInstance;
		break;
	case JSVE_JobManager_MsgID_addJob				:	e_Request_Event = JSVE_RequestAnswer_Event_addJob;
		break;
	case JSVE_JobManager_MsgID_DocSearcher			:	e_Request_Event = JSVE_RequestAnswer_Event_DocSearcher;
		break;
	default:
		MyTraceSD(JSVD_DBG_ERROR, "changeLocalEvent INPUT ERR",(int)e_JobManager_MsgTypeID);
		break;
	}

	#if DEBUGLOG__COMMAND_SEQ
		MyTraceSD(JSVD_DBG_INFO, "changeLocalEvent INPUT ",(int)e_JobManager_MsgTypeID);
		MyTraceSD(JSVD_DBG_INFO, "changeLocalEvent PUTPUT",e_Request_Event);
	#endif

	return e_Request_Event;
}
// メインイベント変換処理.
// 内部イベントをJobManagerのMessageIDに変換する.
JSVE_JobManager_MsgID JSVC_Request::changeMainEvent( JSVE_Request_Event e_Request_Event )
{
	JSVE_JobManager_MsgID e_JobManager_MsgID = JSVE_JobManager_MsgID_Non;

	switch(e_Request_Event)
	{
	case JSVE_Request_Event_ReqMemAlloc					:	e_JobManager_MsgID = JSVE_JobManager_MsgID_MemoryAlloc;
		break;
	case JSVE_Request_Event_AnsAnalyzeCancel			:	e_JobManager_MsgID = JSVE_JobManager_MsgID_AnsXmlAnalyze;
		break;
	case JSVE_Request_Event_AnsAnalyzeEnd				:	e_JobManager_MsgID = JSVE_JobManager_MsgID_AnsXmlAnalyze;
		break;
	case JSVE_Request_Event_ReqStart					:	e_JobManager_MsgID = JSVE_JobManager_MsgID_Start;
		break;
	case JSVE_Request_Event_ReqStop						:	e_JobManager_MsgID = JSVE_JobManager_MsgID_Stop;
		break;
	case JSVE_Request_Event_ReqRestart					:	e_JobManager_MsgID = JSVE_JobManager_MsgID_ReStart;
		break;
	case JSVE_Request_Event_ReqCancel					:	e_JobManager_MsgID = JSVE_JobManager_MsgID_Cancel;
		break;
	case JSVE_Request_Event_ReqUpdate					:	e_JobManager_MsgID = JSVE_JobManager_MsgID_ChangeSetting;
		break;
	case JSVE_Request_Event_ReqMemFree					:	e_JobManager_MsgID = JSVE_JobManager_MsgID_MemoryDelete;
		break;
	case JSVE_RequestAnswer_Event_MemoryAllocAccept		:	e_JobManager_MsgID = JSVE_JobManager_MsgID_MemoryAllocAccept;
		break;
	case JSVE_Request_Event_AnsMemAllocEnd				:	e_JobManager_MsgID = JSVE_JobManager_MsgID_MemoryAllocEnd;
		break;
	case JSVE_RequestAnswer_Event_StartAccept			:	e_JobManager_MsgID = JSVE_JobManager_MsgID_StartAccept;
		break;
	case JSVE_Request_Event_AnsEnd						:	e_JobManager_MsgID = JSVE_JobManager_MsgID_End;
		break;
	case JSVE_RequestAnswer_Event_StopAccept			:	e_JobManager_MsgID = JSVE_JobManager_MsgID_StopAccept;
		break;
	case JSVE_Request_Event_AnsStopEnd					:	e_JobManager_MsgID = JSVE_JobManager_MsgID_StopEnd;
		break;
	case JSVE_RequestAnswer_Event_ReStartAccept			:	e_JobManager_MsgID = JSVE_JobManager_MsgID_ReStartAccept;
		break;
	case JSVE_RequestAnswer_Event_CancelAccept			:	e_JobManager_MsgID = JSVE_JobManager_MsgID_CancelAccept;
		break;
	case JSVE_Request_Event_AnsCancel					:	e_JobManager_MsgID = JSVE_JobManager_MsgID_CancelEnd;
		break;
	case JSVE_RequestAnswer_Event_ChangeSettingAccept	:	e_JobManager_MsgID = JSVE_JobManager_MsgID_ChangeSettingAccept;
		break;
	case JSVE_Request_Event_AnsUpdateEnd				:	e_JobManager_MsgID = JSVE_JobManager_MsgID_ChangeSettingEnd;
		break;
	case JSVE_RequestAnswer_Event_MemoryDeleteAccept	:	e_JobManager_MsgID = JSVE_JobManager_MsgID_MemoryDeleteAccept;
		break;
	case JSVE_Request_Event_AnsMemFree					:	e_JobManager_MsgID = JSVE_JobManager_MsgID_MemoryDeleteEnd;
		break;
	case JSVE_RequestAnswer_Event_StatusChange			:	e_JobManager_MsgID = JSVE_JobManager_MsgID_StatusChange;
		break;
	case JSVE_RequestAnswer_Event_FunctionStatusChange	:	e_JobManager_MsgID = JSVE_JobManager_MsgID_FunctionStatusChange;
		break;
	case JSVE_RequestAnswer_Event_Warning				:	e_JobManager_MsgID = JSVE_JobManager_MsgID_Warning;
		break;
	case JSVE_RequestAnswer_Event_ChargeMapPrepared		:	e_JobManager_MsgID = JSVE_JobManager_MsgID_ChargeMapPrepared;
		break;
	case JSVE_RequestAnswer_Event_NewInstance			:	e_JobManager_MsgID = JSVE_JobManager_MsgID_newJobInstance;
		break;
	case JSVE_RequestAnswer_Event_addJob				:	e_JobManager_MsgID = JSVE_JobManager_MsgID_addJob;
		break;
	case JSVE_RequestAnswer_Event_DocSearcher			:	e_JobManager_MsgID = JSVE_JobManager_MsgID_DocSearcher;
		break;
	default:
		MyTraceSD(JSVD_DBG_ERROR, "changeMainEvent INPUT ERR",e_Request_Event);
		break;
	}

	#if DEBUGLOG__COMMAND_SEQ
		MyTraceSD(JSVD_DBG_INFO, "changeMainEvent INPUT ",e_Request_Event);
		MyTraceSD(JSVD_DBG_INFO, "changeMainEvent PUTPUT",(int)e_JobManager_MsgID);
	#endif

	return e_JobManager_MsgID;
}

// 詳細結果コード変換処理.
// APF→JSVC.
JSVE_JobServiceResultDetail JSVC_Request::changeDetailCondeAPFToJSVC( IPCE_JobsResultDetail e_DetailOther )
{
	///コードが同じなのでキャストのみ.
	///詳細コード追加時に必要なら処理追加.
	JSVE_JobServiceResultDetail e_DetailSelf = (JSVE_JobServiceResultDetail)e_DetailOther;

	return e_DetailSelf;
}
// JSVC→APF.
IPCE_JobsResultDetail JSVC_Request::changeDetailCondeJSVCToAPF( JSVE_JobServiceResultDetail e_DetailSelf )
{
	///コードが同じなのでキャストのみ.
	///詳細コード追加時に必要なら処理追加.
	IPCE_JobsResultDetail e_DetailOther = (IPCE_JobsResultDetail)e_DetailSelf;

	return e_DetailOther;
}

// リクエスト判断.
// 応答受信時に応答に対するリクエストかどうか判断する.
short JSVC_Request::isResponseMsg( JSVC_Response* pRes )
{
	short ss_Ret = JSVD_REQUEST_JUST_MATCH;
	uint32_t ul_MsgTypeID = pRes->getMsgTypeId();

	//応答結果を見て、要求として受けたはずのMSGを割り出す .
	JSVE_JobManager_MsgID e_SrcMsg = JSVE_JobManager_MsgID_Non;
	switch(ul_MsgTypeID){

		//応答結果に呼応するリクエストは無い.
		case JSVE_JobManager_MsgID_Non:
		case JSVE_JobManager_MsgID_StatusChange:
		case JSVE_JobManager_MsgID_FunctionStatusChange:
		case JSVE_JobManager_MsgID_Warning:
		case JSVE_JobManager_MsgID_InitJobManager:
		case JSVE_JobManager_MsgID_HddStatusChanged:
		case JSVE_JobManager_MsgID_AnsXmlAnalyze:							// XML解析応答.
		case JSVE_JobManager_MsgID_AddSubScenarioLog:						// サブシナリオ履歴登録要求.
		case JSVE_JobManager_MsgID_GetSubScenarioLog_DataBaseID:			// サブシナリオ履歴データベースID取得要求.
		case JSVE_JobManager_MsgID_GetFaxJournal_DataBaseID:				// FaxジャーナルデータベースID取得要求.
		case JSVE_JobManager_MsgID_GetFaxJournal_RegNum:					// Faxジャーナル登録数取得要求.
		case JSVE_JobManager_MsgID_SetFaxAddress:							// Faxアドレス設定/取得要求.
		case JSVE_JobManager_MsgID_GetScanJournal_DataBaseID:				// ScanジャーナルデータベースID取得要求.
		case JSVE_JobManager_MsgID_GetScanJournal_RegNum:					// Scanジャーナル登録数取得要求.
		case JSVE_JobManager_MsgID_SetScanAddress:							// Scanアドレス設定/取得要求.
		case JSVE_JobManager_MsgID_GetReceiveRef_DataBaseID:				// 受信履歴データベースID取得要求.
		case JSVE_JobManager_MsgID_GetSendLog_DataBaseID:					// 送信履歴データベースID取得要求.
		case JSVE_JobManager_MsgID_GetSendLogSubScenarioNum:				// 送信履歴登録サブシナリオ数取得要求.
		case JSVE_JobManager_MsgID_StartSendLogAccess:						// 送信履歴の取得アクセス開始要求.
		case JSVE_JobManager_MsgID_EndSendLogAccess:						// 送信履歴の取得アクセス終了要求.
		case JSVE_JobManager_MsgID_GetSendLogAddressNum:					// サブシナリオ毎の送信先数取得要求.
		case JSVE_JobManager_MsgID_GetSendLogSubScenarioNo:					// サブシナリオ番号取得要求.
		case JSVE_JobManager_MsgID_JobLog_AddApplication:					// ジョブログ登録要求（操作系）.
		case JSVE_JobManager_MsgID_JobLog_AddPlatform:						// ジョブログ登録要求（P/Fから）.
		case JSVE_JobManager_MsgID_JobLog_CreateFile:						// ジョブログファイル作成要求.
		case JSVE_JobManager_MsgID_JobLog_SetComplete_getLogFile:			// ジョブログファイル取得完了設定要求.
		case JSVE_JobManager_MsgID_JobLog_GetComplete_getLogFile:			// ジョブログファイル取得完了取得要求.
		case JSVE_JobManager_MsgID_JobLog_GetLogFileState:					// ジョブログファイル作成状況取得要求.
		case JSVE_JobManager_MsgID_JobLog_GetFileExist:						// ジョブログファイル存在有無取得要求.
		case JSVE_JobManager_MsgID_JobLog_GetLimitOver:						// ジョブログファイル化上限超過有無取得要求.
		case JSVE_JobManager_MsgID_JobLog_GetLogFilePath:					// ジョブログパス取得要求.
		case JSVE_JobManager_MsgID_JobLog_GetWebDavLogFilePath:				// ジョブログパス取得要求（WebDav）.
		case JSVE_JobManager_MsgID_JobLog_GetLogVersion:					// ログバージョン取得要求.
		case JSVE_JobManager_MsgID_JobLog_GetJobLogStatus:					// ジョブログ状況取得要求.
		case JSVE_JobManager_MsgID_JobLog_GetDataStatus:					// ログ保存状況取得要求.
		case JSVE_JobManager_MsgID_JobLog_GetPreserve:						// 一時ログ情報の有無取得要求.
		case JSVE_JobManager_MsgID_JobLog_GetOverWrite:						// 一時ログ情報上書き有無取得要求.
		case JSVE_JobManager_MsgID_JobLog_IsNearFull:						// NearFull判定要求.
		case JSVE_JobManager_MsgID_JobLog_IsFull:							// Full判定要求.
		case JSVE_JobManager_MsgID_reqListPrintCancelDoc:					// 重たい処理解除通知受信要求.
		case JSVE_JobManager_MsgID_UpdateWarningCheck:						// ジョブリソース警告の更新＆チェック.
		//サブシナリオID特定する .
		case JSVE_JobManager_MsgID_ChargeMapPrepared:						// 課金マップ展開完了通知.
			break;
		//サブシナリオIDで特定し、親リクエストの状態変化のみ .
		case JSVE_JobManager_MsgID_newJobInstance:							// 待ちうけ部からの実行要求(newJobInstance要求).
			e_SrcMsg = JSVE_JobManager_MsgID_Start;
			break;
		//通知には対応するリクエストは無い .
		case JSVE_JobManager_MsgID_Job_JobEnd:								// Jobからのジョブ終了通知.
		case JSVE_JobManager_MsgID_Job_ExecTableInfoDataChanged:			// Jobからの状態遷移テーブル変化通知.
		case JSVE_JobManager_MsgID_DoneJobList_DataNotify:					// DoneJobListからのデータ変化通知.
		case JSVE_JobManager_MsgID_ImageLog_Notify:							// 画像ログからの状態変化通知.
		case JSVE_JobManager_MsgID_DataBase_Update:							// 履歴DB更新通知.
		case JSVE_JobManager_MsgID_JobLogStatus:							// ジョブログ状態変化通知.
		case JSVE_JobManager_MsgID_TempLogStatus:							// 一時ログ状態変化通知.
		case JSVE_JobManager_MsgID_JobLogFileStatus:						// ジョブログファイル状態変化通知.
		case JSVE_JobManager_MsgID_JobLogPreserve:							// ジョブログ保存状態変化通知.
		case JSVE_JobManager_MsgID_JobLogFileProgress:						// ファイル作成状況通知.
		case JSVE_JobManager_MsgID_HistoryDelete:							// 履歴削除結果通知.
		case JSVE_JobManager_MsgID_JobLogFileMake:							// ジョブログファイル作成結果通知.
		case JSVE_JobManager_MsgID_addJob:									// ActiveJobListからの状態変化通知(ジョブの追加).
		case JSVE_JobManager_MsgID_deleteJob:								// ActiveJobListからの状態変化通知(ジョブの破棄).
		case JSVE_JobManager_MsgID_changedOrder:							// ActiveJobListからの状態変化通知(ジョブの優先順位変更).
		case JSVE_JobManager_MsgID_DocSearcher:								// DocSearcherからの状態変化通知(ジョブ起動結果).
		case JSVE_JobManager_MsgID_JobDeleteInstance:						// APIC_Jobインスタンス破棄通知.
		case JSVE_JobManager_MsgID_RevivalJudgmentEnd:						// 復活判断完了通知.
		case JSVE_JobManager_MsgID_recieveSecureJob:						// ActiveJobListからの状態変化通知(ジョブの優先順位変更).
		case JSVE_JobManager_MsgID_NotifyAnsStart:							// 2012/09/21 CQ372110 IWSへのans対応 Jobのans_startの通知.
		case JSVE_JobManager_MsgID_StopTargetJobWasDeleted:					// comment Sky新大阪 [特ROM対応][IT5-6] プリントエラー解除時に他人のジョブの混在を防止する対応.

// comment [特ROM対応][IT5-26] エラー発生時のジョブの削除方法を自動又は手動に設定可能とする対応 ->.
		case JSVE_JobManager_MsgID_StartActiveJobListDeleteSet:				// ジョブ自動削除開始のオブザーバー通知.
		case JSVE_JobManager_MsgID_EndActiveJobListDeleteSet:				// ジョブ自動削除終了のオブザーバー通知.
// comment [特ROM対応][IT5-26] エラー発生時のジョブの削除方法を自動又は手動に設定可能とする対応 <-.

			e_SrcMsg = JSVE_JobManager_MsgID_Non;
			break;
		case JSVE_JobManager_MsgID_MemoryAllocEnd:
		case JSVE_JobManager_MsgID_MemoryAllocAccept:
			e_SrcMsg = JSVE_JobManager_MsgID_MemoryAlloc;
			break;
		case JSVE_JobManager_MsgID_StartAccept:
		case JSVE_JobManager_MsgID_End:
			e_SrcMsg = JSVE_JobManager_MsgID_Start;
			break;
		case JSVE_JobManager_MsgID_StopAccept:
		case JSVE_JobManager_MsgID_StopEnd:
			e_SrcMsg = JSVE_JobManager_MsgID_Stop;
			break;
		case JSVE_JobManager_MsgID_ReStartAccept:
		case JSVE_JobManager_MsgID_ReStartEnd:
			e_SrcMsg = JSVE_JobManager_MsgID_ReStart;
			break;
		case JSVE_JobManager_MsgID_CancelAccept:
		case JSVE_JobManager_MsgID_CancelEnd:
			e_SrcMsg = JSVE_JobManager_MsgID_Cancel;
			break;
		case JSVE_JobManager_MsgID_ChangeSettingAccept:
		case JSVE_JobManager_MsgID_ChangeSettingEnd:
			e_SrcMsg = JSVE_JobManager_MsgID_ChangeSetting;
			break;
		case JSVE_JobManager_MsgID_MemoryDeleteAccept:
		case JSVE_JobManager_MsgID_MemoryDeleteEnd:
			e_SrcMsg = JSVE_JobManager_MsgID_MemoryDelete;
			break;
		case JSVE_JobManager_MsgID_AllStopAccept:
		case JSVE_JobManager_MsgID_AllStopEnd:
			e_SrcMsg = JSVE_JobManager_MsgID_AllStop;
			break;
		case JSVE_JobManager_MsgID_AllReStartAccept:
		case JSVE_JobManager_MsgID_AllReStartEnd:
			e_SrcMsg = JSVE_JobManager_MsgID_AllReStart;
			break;
		case JSVE_JobManager_MsgID_FunctionInfoAccept:
			e_SrcMsg = JSVE_JobManager_MsgID_FunctionInfo;
			break;
		// 2012/11/12 CQ375076 タイマーFAX予約ジョブ破棄対応 add start.
		case JSVE_JobManager_MsgID_DeleteTimerFaxReserve:
			{
				JSVC_ParentRequest* pc_PRequest = NULL;
				uint32_t ulID = this->getSubScenarioID();
				pc_PRequest = JSVC_JobManager::newInstance()->getParentRequest(ulID);

				// リクエストのMsgIDによって、タイマー予約削除の状態を特定する.
				switch (this->getMsgTypeId()) {
				// 1.メモリ確保要求のリクエストに対しての場合、主電源OFF⇒ON後、シナリオ投入された状態で削除した状態.
				case JSVE_JobManager_MsgID_MemoryAlloc:
//Str【 静的解析対応 : Sky伊藤（勝） : 2012.12.03 : CID-456496】.
//					pc_PRequest->setTimerReserveDeleteStatus(JSVC_TimerRevDelSts_Senario);
					if(pc_PRequest != NULL)
					{
						pc_PRequest->setTimerReserveDeleteStatus(JSVC_TimerRevDelSts_Senario);
					}
//End【 静的解析対応 : Sky伊藤（勝） : 2012.12.03 : CID-456496】.
					break;
				// 2.サブシナリオ実行要求のリクエストに対しての場合、主電源OFFせずにタイマー予約削除した状態.
				case JSVE_JobManager_MsgID_Start:
//Str【 静的解析対応 : Sky伊藤（勝） : 2012.12.03 : CID-456496】.
//					pc_PRequest->setTimerReserveDeleteStatus(JSVC_TimerRevDelSts_Normal);
					if(pc_PRequest != NULL)
					{
						pc_PRequest->setTimerReserveDeleteStatus(JSVC_TimerRevDelSts_Normal);
					}
//End【 静的解析対応 : Sky伊藤（勝） : 2012.12.03 : CID-456496】.
					// この後、サブシナリオ実行完了のシーケンスに移行させるため、MsgID_Startを設定する.
					e_SrcMsg = JSVE_JobManager_MsgID_Start;
					break;
				// 上記以外の場合、サブシナリオ未投入時の破棄と判断する.
				default:
					break;
				}
			}
			break;
		// 2012/11/12 CQ375076 タイマーFAX予約ジョブ破棄対応 add end.
		default:
			MyTraceSD( JSVD_DBG_ERROR, "JSVC_Request::isResponseMsg unknown msgTypeId :" , (int)ul_MsgTypeID );	// warning対応 2013/09/18 ooba.
			break;
	}
	//引数で受け取ったMSGの元になるリクエストか？ .
	if(( JSVE_JobManager_MsgID_Non != e_SrcMsg )
	&& ( this->getMsgTypeId()      == e_SrcMsg )){
		ss_Ret = JSVD_REQUEST_JUST_MATCH;
	} else {
		//停止を受けた際には元のメッセージが実行要求の場合も考慮.
		if(( JSVE_JobManager_MsgID_StopEnd == ul_MsgTypeID )			//StopEndを受けたので.
		&& ((JSVE_JobManager_MsgID_Start   == this->getMsgTypeId() )			// Startか           .
		 || (JSVE_JobManager_MsgID_ReStart == this->getMsgTypeId() ))){		// ReStartを探す     .
				ss_Ret = JSVD_REQUEST_MAYBE_MATCH;
		 //完了を受けた際には元のメッセージが実行の場合と再開の場合があるため.
		} else if (( JSVE_JobManager_MsgID_End     == ul_MsgTypeID         )
		        && ( JSVE_JobManager_MsgID_ReStart == this->getMsgTypeId() )){
			ss_Ret = JSVD_REQUEST_MAYBE_MATCH;
		} else {
			ss_Ret = JSVD_REQUEST_NOT_MATCH;
		}
	}
	return ss_Ret;
}

/// 警告の有無と解除後の動作を調べる.
/// 発生している場合は解除後自発再開するか、再開を受けるまでとまったままか判別を行う.
JSVE_WarningStatus JSVC_Request::getWarningInfo(void)
{
	JSVE_WarningStatus eRet = JSVE_WARNING_NONE;
	APIC_Job* pc_Job = getJobInstance();
	ushort us_loop; 
	//Str【 静的解析対応 : Sky花岡 : 2012.09.03 : CID-439620】.
	if( NULL == pc_Job ){
		MyTraceSD(JSVD_DBG_ERROR, "JSVC_Request::getWarningInfo getJobInstance ERR" , (int)0 );
		return eRet;
	}
	//End【 静的解析対応 : Sky花岡 : 2012.09.03 : CID-439620】.
	/// ジョブステータスは警告有りになっているか？.
	if(( TYPE_InputStatusScanErrorStop   == pc_Job->getJobStatus()->getInputStatus()  )
	|| ( TYPE_OutputStatusPrintErrorStop == pc_Job->getJobStatus()->getOutputStatus() )){
		for( us_loop = CAUTION_DocReturnNum1 ; us_loop < ERROR_MAX ; us_loop++ )
		{
			if ( pc_Job->isWarning((TYPE_JobWarning)us_loop) ){
				switch(us_loop){
					case ERROR_PaperEmpty:
					case ERROR_BinOver:
					case ERROR_PrintPaperError:
					case ERROR_LCTMissSet:
					case ERROR_BumonTotalLimit:
					case ERROR_BlackBumonTotalLimit:
					case ERROR_FullColorBumonTotalLimit:
					case ERROR_UserTotalLimit:
					case ERROR_BlackUserTotalLimit:
					case ERROR_FullColorUserTotalLimit:
					case ERROR_OutOfMoneyTypeB:
					case ERROR_FuserRetract:					// 2014/12/16 機器構成変更No1 H.nakano add.
					case ERROR_ManualTrayMissSet:				// 2015/06/01 MinervaSBK KRQ-140 H.nakano add.
//2017.11.28 IT6 Eagle RQ-720 紙種自動検知対応 doi Start.
					case ERROR_MediaAutoDetectPaperError_Auto:
					case ERROR_MediaAutoDetectPaperError_Manual:
//2017.11.28 IT6 Eagle RQ-720 紙種自動検知対応 doi End.
// 2018.10.16 【EagleH】 RQ-720 lienvl Add Paper Detection Error Start.
					case ERROR_EnvelopeDetectAbnormal:             // 封筒検知異常警告(印刷開始時).
					case ERROR_MediaAutoDetectAbnormal:            // 紙種自動検知異常警告(印刷開始時).
// 2018.10.16 【EagleH】 RQ-720 lienvl Add paper Detection Error End.
// 2020.1.20 IT6_2.0MR2 制御No.38 用紙警告表示でJOBが止まる higaki Start.
					case ERROR_OHPLengthFeedDisable:              // ＯＨＰ縦用紙セット禁止警告.
					case ERROR_LetterHeadPaperJobError:           // レターヘッド小サイズ用紙セット禁止警告.
					case ERROR_ThinPaperJobError:                 // 薄紙用紙サイズセット禁止警告.
// 2020.1.20 IT6_2.0MR2 制御No.38 用紙警告表示でJOBが止まる higaki End.
						eRet = JSVE_WARNING_AUTOSTART;
						break;
					default:
						eRet = JSVE_WARNING_ELSE;
						break;
				};
				if ( JSVE_WARNING_ELSE == eRet ){
					//再開必要、再開不可の警告が見つかったら即抜け.
					break;
				}
			}
		}
	} else {
		//警告なし.
		eRet = JSVE_WARNING_NONE;
	}


	return eRet;
}

Bool JSVC_Request::getCancelFlg()
{
	Bool b_Ret;
	JSVC_ParentRequest* pPReqest = GetParentRequest();
	if( pPReqest != NULL ){
		b_Ret = pPReqest->getCancelFlg();
	}
	else{
		b_Ret = False;
	}
	return b_Ret;
}
void JSVC_Request::setCancelFlg( Bool b_Flg )
{
	JSVC_ParentRequest* pPReqest = GetParentRequest();
	if( pPReqest != NULL ){
		pPReqest->setCancelFlg( b_Flg );
	}
}
Bool JSVC_Request::getStopFlg()
{
	Bool b_Ret;
	JSVC_ParentRequest* pPReqest = GetParentRequest();
	if( pPReqest != NULL ){
		b_Ret = pPReqest->getStopFlg();
	}
	else{
		b_Ret = False;
	}
	return b_Ret;
}
void JSVC_Request::setStopFlg( Bool b_Flg )
{
	JSVC_ParentRequest* pPReqest = GetParentRequest();
	if( pPReqest != NULL ){
		pPReqest->setStopFlg( b_Flg );
	}
}

//==============================================================================.
//	関  数：checkJobSpec.
//	概  要：JobSpecチェック.
//	引  数：TYPC_JobSpec	: pJobSpec		: ジョブスペック.
//	戻り値：Bool			: True/False	: 正常/異常.
//	備  考：.
//==============================================================================.
Bool JSVC_Request::checkJobSpec(TYPC_JobSpec* pJobSpec)
{
	Bool b_Ret = False;		// 結果.

	// 設定値の範囲チェック.
	b_Ret = checkValue(pJobSpec);
	if( b_Ret == True )
	{
		// 設定値の組み合わせチェック.
		b_Ret = checkValueCombination(pJobSpec);
	}
	if( b_Ret == True )
	{
		JSVE_CombinationErr e_ret = JSVE_CombinationErr_Nothing;
		// サブシナリオ同士の組み合わせチェック.
		e_ret = pc_JobManager->checkJobSpecCombination(pJobSpec,this->getSubScenarioID());
		if( e_ret == JSVE_CombinationErr_Nothing )
		{
			b_Ret = True;
		}
		else
		{
			b_Ret = False;
		}
	}

	return b_Ret;
}

//==============================================================================.
//	関  数：checkValue.
//	概  要：設定値の範囲チェック.
//	引  数：TYPC_JobSpec	: pJobSpec		: ジョブスペック.
//	戻り値：Bool			: True/False	: 正常/異常.
//	備  考：.
//==============================================================================.
Bool JSVC_Request::checkValue(TYPC_JobSpec* pJobSpec)
{
	Bool b_Ret = True;					// 結果.
	ushort us_Cnt = 0;					// カウンタ.
	uint32_t ul_wFunctionExecNum = 0;	// ファンクション実行順(work).
	uint32_t ul_FunctionNumber = 0;
	Bool b_ExistFunction = 0;			// ファンクション情報有り.

	// Function情報有無チェック.
	for(us_Cnt = 0; us_Cnt < JOBD_FunctionMax; us_Cnt++)
	{
		ul_FunctionNumber   = pJobSpec->pc_Common->c_FunctionInfo[us_Cnt].getFunctionNumber();
		ul_wFunctionExecNum = pJobSpec->pc_Common->c_FunctionInfo[us_Cnt].getFunctionExecNum();
		if( TYPE_FunctionNumber_NON != ul_FunctionNumber ){
			if (0 == ul_wFunctionExecNum){
				// 異常(実行順の指定されていないFunctionがある).
			MyTraceSD(JSVD_DBG_ERROR, "JSVC_Request::checkValue ERR", (int)ul_wFunctionExecNum);
			b_Ret = False;
			break;
			}else{
				b_ExistFunction = True;
			}
		}
	}
	if( b_ExistFunction == False ){
		// 異常（Functionが設定されていない）.
		MyTraceSD(JSVD_DBG_ERROR, "JSVC_Request::checkValue ERR No FunctionInfo", 0);
		b_Ret = False;
	}

	return b_Ret;
}

//==============================================================================.
//	関  数：checkValueCombination.
//	概  要：設定値の組合せチェック.
//	引  数：TYPC_JobSpec	: pJobSpec		: ジョブスペック.
//	戻り値：Bool			: True/False	: 正常/異常.
//	備  考：.
//==============================================================================.
Bool JSVC_Request::checkValueCombination(TYPC_JobSpec* pJobSpec)
{
	Bool b_Ret = True;						// 結果.
	ushort us_Cnt = 0;						// カウンタ.
	ushort us_ChkCnt = 0;					// チェック用カウンタ.
	int32_t sl_DocID[TYPD_MaxProgramJob];	// ドキュメント番号.

	memset(sl_DocID,TYPD_NotSetting,sizeof(sl_DocID));
	pJobSpec->pc_Common->c_JobType.getDocumentNo(sl_DocID);

	// バインドチェック.
	// バインド（プログラム）かつ、ソート設定かをチェック.
	// ※ただし、ジョブタイプがコピージョブか割り込みコピージョブのときのみ.
	if ( pJobSpec->pc_Common->c_JobType.getJobType() == TYPE_JobTypeCopy ||
		 pJobSpec->pc_Common->c_JobType.getJobType() == TYPE_JobTypeIntCopy )
	{
		for(us_Cnt = 0; us_Cnt < TYPD_MaxProgramJob; us_Cnt++)
		{
			if(TYPD_NotSetting != sl_DocID[us_Cnt])		// TYPD_NotSetting:初期値(設定なし).
			{
				us_ChkCnt++;
			}
			if(1 < us_ChkCnt)	// 1より大きければバインド.
			{
				// ソートモードチェック.
				if(0 == pJobSpec->pc_Print->c_SortMode.getSortMode())	// 0:ソート設定なし.
				{
					MyTraceSS(JSVD_DBG_ERROR, "JSVC_Request::checkValueCombination ERR", "Bind without sort");
					b_Ret = False;
				}
				break;
			}
		}
	}

	return b_Ret;
}

//==============================================================================.
//	関  数：newJobInstance_byBinaryFile.
//	概  要：JobSpecのバイナリファイルからAPIC_Jobインスタンスを生成し親リクエストにセットする.
//	引  数：TYPC_JobSpec	: pJobSpec		: JobSpecのバイナリファイルパス.
//	戻り値：Bool			: True/False	: 正常/異常.
//	備  考：.
//==============================================================================.
Bool JSVC_RequestJobSetting::newJobInstance_byBinaryFile(JSVC_Request* pc_Req, char* p_FilePath)
{
	uint32_t				ulID;
	JSVC_ParentRequest*		pc_PRequest;
	APIC_Job*				pc_Job;
	TYPE_JobType			e_JobType;
	JSVC_IsListnerType		ul_RetIsList;
	JSVC_RequestJobSetting* pc_Request = (JSVC_RequestJobSetting*)pc_Req;

	// 親リクエストを取得.
	ulID = pc_Request->getSubScenarioID();
	pc_PRequest = JSVC_JobManager::newInstance()->getParentRequest(ulID);
	if (NULL == pc_PRequest) {
		MyTraceSLD(JSVD_DBG_ERROR, "JSVC_RequestJobSetting::newJobInstance_byBinaryFile getParentRequest ERR" , (intptr_t)pc_PRequest );
		return False;
	}

	// JobSpecデータクラスを作成.
	pc_Request->p_DataJobSpec = new JSVC_JobSpec;

	// JobSpecファイル読み込み.
	MyTraceSS(JSVD_DBG_INFO, "JSVC_RequestJobSetting call deSerialize" , p_FilePath );
	if (False == pc_Request->p_DataJobSpec->deSerialize(p_FilePath)) {
		MyTraceSS(JSVD_DBG_ERROR, "JSVC_RequestJobSetting::newJobInstance_byBinaryFile deSerialize ERR" , p_FilePath );
		delete pc_Request->p_DataJobSpec;
		pc_Request->p_DataJobSpec = NULL;
		return False;
	}

	// 待ち受け/非待ち受けJobの判定.
	ul_RetIsList = JSVC_JobManager::newInstance()->isListenerFunc(pc_Request->p_DataJobSpec->getJobSpec(), pc_Request->ul_SubScenarioID, &e_JobType);
	if (JSVC_IsListner_NG == ul_RetIsList) {
		MyTraceSD(JSVD_DBG_ERROR, "JSVC_RequestJobSetting::newJobInstance_byBinaryFile isListenerFunc ERR" , (int)ul_RetIsList );
		delete pc_Request->p_DataJobSpec;
		pc_Request->p_DataJobSpec = NULL;
		return False;
	} else if (JSVC_IsListner_RetryListner == ul_RetIsList) {

		// JobType確定済み.
		if ( False == JSVC_JobManager::newInstance()->fixJobType( pc_Request->p_DataJobSpec->getJobSpec(), getReceptionModuleType(), True, &e_JobType ))
		{
			MyTraceSD(JSVD_DBG_ERROR, "JSVC_RequestJobSetting::newJobInstance_byBinaryFile fixJobType1 ERR" , False );
		}
	} else {

		// Jobタイプ決定処理.
		if ( False == JSVC_JobManager::newInstance()->fixJobType( pc_Request->p_DataJobSpec->getJobSpec(), TYPE_ReceptionModuleType_NONE, False, &e_JobType ))
		{
			MyTraceSD(JSVD_DBG_ERROR, "JSVC_RequestJobSetting::newJobInstance_byBinaryFile fixJobType2 ERR" , False );
		}
	}
	#if DEBUGLOG__JOBSTART_JUDGE
		// TYPE_JobTypeCopy                0   ///<	ジョブタイプ：コピー 
		// TYPE_JobTypeIntCopy             1   ///<	ジョブタイプ：割込コピー 
		// TYPE_JobTypeScan                2   ///<	ジョブタイプ：スキャン 
		// TYPE_JobTypeTwain               3   ///<	ジョブタイプ：Ｔｗａｉｎ 
		// TYPE_JobTypePrint               4   ///<	ジョブタイプ：プリント 
		// TYPE_JobTypeSecurePrint         5   ///<	ジョブタイプ：機密文書プリント 
		// TYPE_JobTypePdl                 6   ///<	ジョブタイプ：ＰＤＬ（ＲＩＰ処理前） 
		// TYPE_JobTypeFaxSendQuick        7   ///<	ジョブタイプ：ＦＡＸ（クイックメモリ送信） 
		// TYPE_JobTypeFaxSendMemory       8   ///<	ジョブタイプ：ＦＡＸ（メモリ送信） 
		// TYPE_JobTypeFaxSendTimer        9   ///<	ジョブタイプ：ＦＡＸ(タイマ送信） 
		// TYPE_JobTypeFaxReceive          10  ///<	ジョブタイプ：ＦＡＸ受信プリント 
		// TYPE_JobTypeIfaxReceive         11  ///<	ジョブタイプ：ＩＦａｘ受信 
		// TYPE_JobTypeFaxReceiveTimer     12  ///<	ジョブタイプ：ＦＡＸ（タイマーポーリング受信） 
		// TYPE_JobTypeFaxReceivePolling   13  ///<	ジョブタイプ：ＦＡＸ（ポーリング送信） 
		MyTraceSD(JSVD_DBG_INFO, "JobType = ",e_JobType);
	#endif

	// JobSpec(編集領域)の内容をチェック.
	if (False == checkJobSpec(pc_Request->p_DataJobSpec->getJobSpec())) {
		MyTraceSD(JSVD_DBG_ERROR, "JSVC_RequestJobSetting::newJobInstance_byBinaryFile checkJobSpec ERR" , (int)ul_RetIsList );
		delete pc_Request->p_DataJobSpec;
		pc_Request->p_DataJobSpec = NULL;
		return False;
	}

	// Jobインスタンス生成.
	pc_Job = APIC_Job::newInstance( e_JobType );
	if (NULL == pc_Job) {
		MyTraceSLD(JSVD_DBG_ERROR, "JSVC_RequestJobSetting::newJobInstance_byBinaryFile newInstance ERR" , (intptr_t)pc_Job );
		delete pc_Request->p_DataJobSpec;
		pc_Request->p_DataJobSpec = NULL;
		return False;
	}

	// JobSpec設定.
	if (False == pc_Request->p_DataJobSpec->setJobSpec(pc_Job->getJobSpec())) {
		MyTraceSD(JSVD_DBG_ERROR, "JSVC_RequestJobSetting::newJobInstance_byBinaryFile setJobSpec ERR" , 0 );
		delete pc_Request->p_DataJobSpec;
		pc_Request->p_DataJobSpec = NULL;
		return False;
	}

	// 生成したJobインスタンスを親リクエストに追加.
	if( True == pc_Request->p_DataJobSpec->bReadJobData ){
		pc_Job->setTypcJobData(&(pc_Request->p_DataJobSpec->cJobData));
	}

	if( NULL != pc_Job->getKeyDevice() ){
		// 課金対象装置.
		switch( pc_Job->getJobSpec()->pc_Common->getChargeKindID() ){
		case SYSE_UnitId_VendorKM: // 統合ベンダー.
			pc_Job->getKeyDevice()->setFeeCollectionJob( true, pc_Job->getKeyDevice() );
			break;
		default:
			break;
		}
	}

	pc_PRequest->addInstance( pc_Job );
	delete pc_Request->p_DataJobSpec;
	pc_Request->p_DataJobSpec = NULL;
	return True;
}


//==============================================================================.
//	関  数：SubsinarioUpdate_byBinaryFile.
//	概  要：JobSpecのバイナリファイルからAPIC_JobインスタンスにJobSpecを反映する.
//	引  数：TYPC_JobSpec	: pJobSpec		: JobSpecのバイナリファイルパス.
//	戻り値：Bool			: True/False	: 正常/異常.
//	備  考：.
//==============================================================================.
Bool JSVC_RequestJobSetting::SubsinarioUpdate_byBinaryFile(JSVC_Request* pc_Req, char* p_FilePath)
{
	JSVC_RequestJobSetting* pc_Request = (JSVC_RequestJobSetting*)pc_Req;
	// JobSpecデータクラスを作成.
	pc_Request->p_DataJobSpec = new JSVC_JobSpec;

	// JobSpecファイル読み込み
	if (False == pc_Request->p_DataJobSpec->deSerialize(p_FilePath)) {
		MyTraceSS( JSVD_DBG_ERROR, "JSVC_RequestJobSetting::SubsinarioUpdate_byBinaryFile", " deSerialize False Return." ) ;
		delete pc_Request->p_DataJobSpec;
		pc_Request->p_DataJobSpec = NULL;
		return False;
	}

//Str【 静的解析対応 : Sky田村 : 2012.09.03 : CID-439626 】.
	if( pc_Request->getJobInstance() == NULL )
	{
		MyTraceSS( JSVD_DBG_ERROR, "JSVC_RequestJobSetting::SubsinarioUpdate_byBinaryFile", "pc_Request->getJobInstance() is NULL." ) ;
		return False;
	}
//End【 静的解析対応 : Sky田村 : 2012.09.03 : CID-439626 】.

	// Jobへテーブル更新トリガを投げる.
	pc_Request->getJobInstance()->req_changeExecTableInfo( &JSVC_Request::ans_changeExecTableInfo ,pc_Request->p_DataJobSpec->getJobSpec());

	return True;
}

// コンストラクタ.
JSVC_ParentRequest::JSVC_ParentRequest()
:	b_CancelFlg(False),b_StopFlg(False),m_bExchangeRestart(False),b_WaitNextJobPointer(False),e_ReceptionModuleType(TYPE_ReceptionModuleType_NONE)
//Str【 静的解析対応 : Sky佐藤宏 : 2012.09.05 : CID-447281 】.
,	e_PowerOnReStartType(TYPE_PowerOnReStartType_OFF),e_PDLProcessState(TYPE_ProcessSts_Non),e_APPInputProcessState(TYPE_ProcessSts_Non),ul_EndNoticeComplete(0)
//End【 静的解析対応 : Sky佐藤宏 : 2012.09.05 : CID-447281 】.
,	pc_JobSpec(NULL)
,	m_bDocDeleting(False)
,	m_IsLitenerResult(JSVC_IsListner_NG)
//Str【 静的解析対応 : Sky佐藤宏 : 2012.09.05 : CID-447281 】.
,	m_bWaitRevivalJob(False)
//End【 静的解析対応 : Sky佐藤宏 : 2012.09.05 : CID-447281 】.
,	ss_RevivalIndex(0),sl_RevivalJobType(0)
// 2012/11/12 CQ375076 タイマーFAX予約ジョブ破棄対応 add start.
, m_uc_TimerReserveDelete(JSVC_TimerRevDelSts_Non)
// 2012/11/12 CQ375076 タイマーFAX予約ジョブ破棄対応 add end.
{
	// 初期値設定.
	m_eJobExexStatus = JSVC_JOBEXEC_NOT_PAUSE;
	for ( int i = 0; i < JSVE_FunctionNumber_Max; i++ ){
		ul_FunctionStatusTable[i] = JSVE_FunctionStatus_Non;
	}
	
	for ( int i = 0; i < JSVD_JOBP_MAX; i++ ){
		uc_JobPointerState[i] = JSVE_Request_ObsStatus_Disable;
		pc_JobPtr[i] = NULL;
	}
//Str【 静的解析対応 : Sky佐藤宏 : 2012.09.05 : CID-447281 】.
	memset(&s_chargeApplicationSocket, 0, sizeof(sockaddr_un));
//End【 静的解析対応 : Sky佐藤宏 : 2012.09.05 : CID-447281 】.

	s_chargeApplicationSocket.sun_family = AF_UNIX;
	s_chargeApplicationSocket.sun_path[0] = 0;
}

// 自クラスの型を返す.
JSVE_Request_Class_Type JSVC_ParentRequest::WhoAmI()
{
	return JSVE_Request_Class_Type_Parent;
}

// ジョブインスタンス設定.
JSVE_Request_Result JSVC_ParentRequest::setJobInstance( APIC_Job*	pJob )
{
	JSVE_Request_Result e_Ret = JSVE_Request_Result_Ok;
	// 入力チェック.
	if (NULL == pJob){
		e_Ret = JSVE_Request_Result_Err;
	} else {
		addInstance( pJob );
	}
	return e_Ret;
}
// ジョブインスタンス取得.
JSVE_Request_Result JSVC_ParentRequest::getJobInstance( APIC_Job**	ppJob )
{
	// 入力チェック.
	if (NULL == ppJob){
		return JSVE_Request_Result_Err;
	}

	*ppJob = pc_JobPtr[0];

	return JSVE_Request_Result_Ok;
}

JSVE_Request_Result JSVC_ParentRequest::setExchangeRestart(Bool bExchangeRestart)
{
	m_bExchangeRestart = bExchangeRestart;
	return JSVE_Request_Result_Ok;
}
Bool JSVC_ParentRequest::getExchangeRestart()
{
	return m_bExchangeRestart;
}

//	サブシナリオIDの取得.
uint32_t JSVC_ParentRequest::getSubScenarioID()
{
	return JSVC_SubScnRequestBase::getSubScenarioID();
}

//	リクエスト状態の取得.
JSVE_Request_Status JSVC_ParentRequest::getRequestStatus()
{
	return JSVC_SubScnRequestBase::getRequestStatus();
}

void JSVC_ParentRequest::setFunctionStatus(JSVE_FunctionStatus e_FunctionStatus, JSVE_FunctionNumber e_FunctionNumber)
{
	ul_FunctionStatusTable[e_FunctionNumber] = e_FunctionStatus;
}
JSVE_FunctionStatus JSVC_ParentRequest::getFunctionStatus(JSVE_FunctionNumber e_FunctionNumber)
{
	return ul_FunctionStatusTable[e_FunctionNumber];
}

void JSVC_ParentRequest::addInstance( APIC_Job* pc_Job )
{
	// 入力チェック.
	if (NULL == pc_Job){
		MyTraceSD(JSVD_DBG_ERROR, "JSVC_ParentRequest::addInstance",1);
	} else {

		//管理リストへの登録.
		short ss_index = 0;
		for( ss_index = 0 ; ss_index < JSVD_JOBP_MAX ; ss_index++ ){
			if( JSVE_Request_ObsStatus_Disable == uc_JobPointerState[ss_index] )
			{
				break;
			}
		}
		if ( ss_index < JSVD_JOBP_MAX )
		{
			pc_JobPtr[ss_index] = pc_Job;
			uc_JobPointerState[ss_index] = JSVE_Request_ObsStatus_AfterAttach;
			pc_Job->attachObserver((APIC_JobObserver*)&cObsJob);					//アタッチの実施.
		} else {
			MyTraceSD(JSVD_DBG_INFO, "JSVC_ParentRequest::addInstance",2);
		}
	}
	return ;
}

// オブザーバ登録状態取得.
JSVE_Reqest_ObsStatus JSVC_ParentRequest::getJobPointerStatus( uchar uc_Index )
{
	return uc_JobPointerState[uc_Index];
}
// オブザーバ登録状態設定.
void JSVC_ParentRequest::setJobPointerStatus( uchar uc_Index, JSVE_Reqest_ObsStatus e_Status )
{
	uc_JobPointerState[uc_Index] = e_Status;
}
// Jobインスタンス取得.
APIC_Job* JSVC_ParentRequest::getJobPtr( uchar uc_Index )
{
	return pc_JobPtr[uc_Index];
}
// Jobインスタンス設定.
void JSVC_ParentRequest::setJobPtr( uchar uc_Index, APIC_Job* pc_Job )
{
	pc_JobPtr[uc_Index] = pc_Job;
}
// オブザーバインスタンス取得.
JSVC_JobObserver* JSVC_ParentRequest::getObserver()
{
	return &cObsJob;
}
// 受信モジュール種別取得.
TYPE_ReceptionModuleType JSVC_ParentRequest::getReceptionModuleType()
{
	return e_ReceptionModuleType;
}
// 受信モジュール種別設定.
void JSVC_ParentRequest::setReceptionModuleType( TYPE_ReceptionModuleType e_Type )
{
	e_ReceptionModuleType = e_Type;
}
void JSVC_ParentRequest::setReceptionModuleType(  TYPC_JobSpec* pc_JobSpec, uint32_t ul_JobID )
{
	TYPE_ReceptionModuleType e_ReceptionModuleType = TYPE_ReceptionModuleType_NONE;
	int32_t sl_DocumentNo[TYPD_MaxProgramJob];
	memset(sl_DocumentNo, TYPD_NotSetting, sizeof(sl_DocumentNo));
	TYPE_JobType e_JobType;
	TYPE_RxMode e_RxMode;
	TYPE_TxMode e_TxMode;

	if ( NULL != pc_JobSpec){
		// アプリからのサブシナリオ投入.
		// TYPE_ReceptionModuleType_NONE 初期値(Copy,Scan/Box/File入力ジョブ).
		e_ReceptionModuleType = TYPE_ReceptionModuleType_NONE;
		e_JobType = pc_JobSpec->pc_Common->c_JobType.getJobType();
		// レガシーからのジョブ投入.
		// Net受信(レガシージョブ).
		pc_JobSpec->pc_Common->c_JobType.getDocumentNo(sl_DocumentNo);
		e_TxMode = pc_JobSpec->pc_Fax->c_FaxAct.getTxMode();
		e_RxMode = pc_JobSpec->pc_Fax->c_FaxAct.getRxMode();

		MyTraceSD( JSVD_DBG_INFO, "JSVC_ParentRequest::setReceptionModuleType e_JobType:" , e_JobType );
		MyTraceSD( JSVD_DBG_INFO, "JSVC_ParentRequest::setReceptionModuleType e_TxMode:" , e_TxMode );
		MyTraceSD( JSVD_DBG_INFO, "JSVC_ParentRequest::setReceptionModuleType e_RxMode:" , e_RxMode );

		if ((TYPD_NotSetting == sl_DocumentNo[0]) &&
			((TYPE_JobTypePrint == e_JobType) || (TYPE_JobTypeSecurePrint == e_JobType)))
		{
			e_ReceptionModuleType = TYPE_ReceptionModuleType_NetRx;
		}
		if ((sl_DocumentNo[0] == TYPD_NotSetting) && 
			(e_JobType == TYPE_JobTypeFaxSendTimer) && 
			(e_TxMode == TYPE_TxModePcFax))
		{
			e_ReceptionModuleType = TYPE_ReceptionModuleType_NetRx;
		}
		if (((TYPE_JobTypeFaxSendQuick == e_JobType) || (TYPE_JobTypeFaxSendMemory == e_JobType)) &&
			(pc_JobSpec->pc_Common->c_JobType.getBoxNo() != DOCD_ManualForwardBox) &&
			(sl_DocumentNo[0] == TYPD_NotSetting) &&
			(e_TxMode == TYPE_TxModePcFax)){
			e_ReceptionModuleType = TYPE_ReceptionModuleType_NetRx;
		}
		if ((TYPE_JobTypeFaxReceive == e_JobType) &&
			( (e_RxMode == TYPE_RxModeIPAddFaxRx) || (e_RxMode == TYPE_RxModeInternetFaxRx) )){
			e_ReceptionModuleType = TYPE_ReceptionModuleType_NetRx;
		}
		if( pc_JobSpec->pc_Common->c_JobType.getListPrint() == True ){
			e_ReceptionModuleType = TYPE_ReceptionModuleType_NetRx;
		}

		// FaxRx(レガシージョブ).
		if (pc_JobSpec->pc_Common->getInterface() == TYPE_JobEntryIF_FAX){
			e_ReceptionModuleType = TYPE_ReceptionModuleType_FaxRx;
		}

		// TcpSocket(レガシージョブ).
		if ((pc_JobSpec->pc_Common->getInterface() == TYPE_JobEntryIF_TCPSocket)	||
			(pc_JobSpec->pc_Common->getInterface() == TYPE_JobEntryIF_WSDScan))
		{
			e_ReceptionModuleType = TYPE_ReceptionModuleType_TcpSocket;
		}

		// PSWC(レガシージョブ).
		if (pc_JobSpec->pc_Common->getInterface() == TYPE_JobEntryIF_PSWC){
			e_ReceptionModuleType = TYPE_ReceptionModuleType_PSWC;
		}

		// OpenAPI受信判断.
		if (pc_JobSpec->pc_Common->getInterface() == TYPE_JobEntryIF_ISOpenAPI){
			e_ReceptionModuleType = TYPE_ReceptionModuleType_Oap;
		}

		// Browser(レガシージョブ).
		if (True == pc_JobSpec->pc_Send->c_ComSend.getWebBrowserScanOn()) {
			e_ReceptionModuleType = TYPE_ReceptionModuleType_Browser;
		}

		// Fiery(レガシージョブ).
		if ((sl_DocumentNo[0] == TYPD_NotSetting) &&
			(e_JobType == TYPE_JobTypeScan || e_JobType == TYPE_JobTypeTwain) &&
			(APIC_SystemInfo::newInstance()->getExternalIFMode() == TYPE_ExternalIFMode1))
		{
			e_ReceptionModuleType = TYPE_ReceptionModuleType_Fiery;
		}
		if ((sl_DocumentNo[0] == TYPD_NotSetting) &&
			(e_JobType == TYPE_JobTypePrint || e_JobType == TYPE_JobTypeSecurePrint) &&
			(APIC_SystemInfo::newInstance()->getExternalIFMode() == TYPE_ExternalIFMode1))
		{
			e_ReceptionModuleType = TYPE_ReceptionModuleType_Fiery;
		}

/* CQ370925対応 2012/08/28 S.Wakayama Start */
		if (True == pc_JobSpec->pc_Send->c_ComSend.getFileSend()) {
			e_ReceptionModuleType = TYPE_ReceptionModuleType_FwUpdate;
		}
/* CQ370925対応 2012/08/28 S.Wakayama End */

		// IGL(レガシージョブ).
		if (TYPD_ImageLogSendJobNon != pc_JobSpec->pc_Common->getImageLogSend()){
			e_ReceptionModuleType = TYPE_ReceptionModuleType_IGLSend;
		}

		// 特殊ジョブ投入.
		if (True == JOBG_getDocSearcherStart(ul_JobID)){
			e_ReceptionModuleType = TYPE_ReceptionModuleType_DocSearcher;
		}

		// アプリまたはレガシーからのジョブ投入.
		if(True == pc_JobSpec->pc_Fax->c_FaxCommon.getTimerOn()){
			e_ReceptionModuleType = TYPE_ReceptionModuleType_SendTimer;
		}
	}else{
		MyTraceSS(JSVD_DBG_ERROR, "JSVC_ParentRequest::setReceptionModuleType","pc_JobSpec Error");
	}
}
// リスタートタイプ取得.
TYPE_PowerOnReStartType JSVC_ParentRequest::getPowerOnReStartType()
{
	return e_PowerOnReStartType;
}
// リスタートタイプ設定.
void JSVC_ParentRequest::setPowerOnReStartType( TYPE_PowerOnReStartType e_PowerOnReStartType )
{
	JSVC_ParentRequest::e_PowerOnReStartType = e_PowerOnReStartType;
}
// Function状態取得.
JSVE_FunctionStatus JSVC_ParentRequest::getFunctionStatusTable( JSVE_FunctionNumber ul_Index )
{
	return ul_FunctionStatusTable[ul_Index];
}
// Function状態設定.
void JSVC_ParentRequest::setFunctionStatusTable( JSVE_FunctionNumber ul_Index, JSVE_FunctionStatus e_Status )
{
	ul_FunctionStatusTable[ul_Index] = e_Status;
}
// PDLProcessの状態設定・取得.
void JSVC_ParentRequest::setPDLProcessState( TYPE_ProcessSts e_State )
{
	e_PDLProcessState = e_State;
}
TYPE_ProcessSts JSVC_ParentRequest::getPDLProcessState()
{
	return e_PDLProcessState;
}
// APPInputProcessの状態設定・取得.
void JSVC_ParentRequest::setAPPInputProcessState( TYPE_ProcessSts e_State )
{
	e_APPInputProcessState = e_State;
}
TYPE_ProcessSts JSVC_ParentRequest::getAPPInputProcessState()
{
	return e_APPInputProcessState;
}
// Init完了、Functionが全て待機中かどうか判定(メモリ確保要求完了判定).
Bool JSVC_ParentRequest::isRequestStatusReady()
{
	// InitProcess→終了.
	// 全てのFunctionが未使用か待機中.
	// このときにTrueを返す。その他は全てFalse.
	if( JSVE_FunctionStatus_End != ul_FunctionStatusTable[JSVE_FunctionNumber_CommonSetting] ){
		return False;
	}

	return True;
}

// Init完了、Functionが全て実行完了かどうか判定(実行要求完了判定).
Bool JSVC_ParentRequest::isRequestStatusExecEnd()
{
	// InitProcess→終了.
	// 全てのFunctionが未使用か終了.
	// EndProcess→待機中.
	// このときにTrueを返す。その他は全てFalse.
	if( JSVE_FunctionStatus_End != ul_FunctionStatusTable[JSVE_FunctionNumber_CommonSetting] ){
		return False;
	}
	for( int i_Cnt=JSVE_FunctionNumber_Non; i_Cnt<=JSVE_FunctionNumber_AddEmailContents; i_Cnt++ ){
		if( JSVE_FunctionStatus_Non != ul_FunctionStatusTable[i_Cnt]
		&&  JSVE_FunctionStatus_End != ul_FunctionStatusTable[i_Cnt]
		){
			return False;
		}
	}
	if( JSVE_FunctionStatus_Idle != ul_FunctionStatusTable[JSVE_FunctionNumber_FaxCommonSetting]
	&&  JSVE_FunctionStatus_Non != ul_FunctionStatusTable[JSVE_FunctionNumber_FaxCommonSetting]
	){
		// ★EndProcessの状態が待機中になってないので、とりあえず判定しない.
	}

	// 2ジョブで構成されているサブシナリオは通知を投げない.
	TYPE_JobType e_JobType = 0;
	int32_t sl_DocumentNo[TYPD_MaxProgramJob] = { TYPD_NotSetting };

	JSVC_IsListnerType iListener = JSVC_JobManager::newInstance()->isListenerFunc(
													getJobInstance()->getJobSpec(), 
													getSubScenarioID(),
													&e_JobType);

	e_JobType = getJobInstance()->getJobSpec()->pc_Common->c_JobType.getJobType();
	getJobInstance()->getJobSpec()->pc_Common->c_JobType.getDocumentNo(sl_DocumentNo);

	if(iListener == JSVC_IsListner_NoListner) {
		// 2013/05/07 復旧ジョブシーケンス見直し対応(CQ379669) del start.
		// タイマー予約ジョブも一度シナリオを終了する制御へと変更するため、処理を削除.
		//// 2012/08/20 CQ370710対応 Start.
		////if(e_JobType == TYPE_JobTypeFaxSendTimer && sl_DocumentNo[0] == TYPD_NotSetting) {
		//if( (e_JobType == TYPE_JobTypeFaxSendTimer) && 
		//	(getJobInstance()->getJobSpec()->pc_Common->c_JobType.getBoxNo() != DOCD_FaxSndBox) &&
		//	(getJobInstance()->getJobSpec()->pc_Common->c_JobType.getBoxNo() != DOCD_RevivalSndBox) ) {
		//	// タイマー予約ジョブ
		//	MyTraceSS(JSVD_DBG_REQANS, "isRequestStatusExecEnd ReqKeep " , "TimerReserveJob" );
		//	return False;
		////} else if(e_JobType == TYPE_JobTypeFaxSendTimer && sl_DocumentNo[0] != TYPD_NotSetting) {
		//} else if( (e_JobType == TYPE_JobTypeFaxSendTimer) && 
		//	((getJobInstance()->getJobSpec()->pc_Common->c_JobType.getBoxNo() == DOCD_FaxSndBox) ||
		//	 (getJobInstance()->getJobSpec()->pc_Common->c_JobType.getBoxNo() == DOCD_RevivalSndBox)) ) {
		//// 2012/08/20 CQ370710対応 End.
		//	// タイマー送信ジョブ
		//	//Nop.
		//	MyTraceSS(JSVD_DBG_REQANS, "isRequestStatusExecEnd ReqKeep " , "TimerSendJob" );
		//}
		// 2012/10/19 CQ373523対応 add start.
		//else if (TYPE_JobTypeFaxReceiveTimer == e_JobType) {
		//	// タイマーポーリング受信ジョブ時もサブシナリオを削除しない.
		//	MyTraceSS(JSVD_DBG_REQANS, "isRequestStatusExecEnd ReqKeep " , "TimerPollingReserveJob" );
		//	return False;
		//}
		// 2012/10/19 CQ373523対応 add end.
		// 2013/05/07 復旧ジョブシーケンス見直し対応(CQ379669) del end.
	} else if(iListener == JSVC_IsListner_Listner) {
		if (getJobInstance()->getTransmissonOn() != False) {
			// 転送/中継/TSIジョブ
			MyTraceSS(JSVD_DBG_REQANS, "isRequestStatusExecEnd ReqKeep " , "RelayJob" );
			return False;
		}
	} else {
		//Nop.
	}

	return True;
}

Bool JSVC_ParentRequest::isRequestStatusExecEnd2()
{
	// Init/EndProcessの実行状態を取得する.
// 2018/4/24 WHP メモリ破壊調査中に見つかった解放済み領域へのアクセス問題への対応 Nagasawa add sta.
// 並行で deleteInstance された場合に、解放済み領域へのアクセスとなってしまうため.
// Job のセマフォを握ることで deleteInstance と並行動作しないように修正.
	getJobInstance()->lock();
	
	uchar uc_InitProcessStatus = JOBG_getProcessState( getJobInstance()->getJobStatus()->getJobID(), 1 );
	uchar uc_EndProcessStatus = JOBG_getProcessState( getJobInstance()->getJobStatus()->getJobID(), 2 );

	getJobInstance()->unlock();
// 2018/4/24 WHP メモリ破壊調査中に見つかった解放済み領域へのアクセス問題への対応 Nagasawa add end.

	// 実行中でなければ、Falseとする.
	if ((6 == uc_InitProcessStatus) && (2 == uc_EndProcessStatus)){
		// 以降の処理を行う.
	}else{
		if((6 == ul_FunctionStatusTable[22]) && (1 == ul_FunctionStatusTable[23])) {
			for ( int icnt = 0; icnt < JSVE_FunctionNumber_Max -2; icnt++ ){
				if (!((0 == ul_FunctionStatusTable[icnt]) || (6 == ul_FunctionStatusTable[icnt]))) {
		return False;
				}
			}
		} else {
			return False;
		}
	}

	// 2ジョブで構成されているサブシナリオは通知を投げない
	TYPE_JobType e_JobType = 0;
	int32_t sl_DocumentNo[TYPD_MaxProgramJob] = { TYPD_NotSetting };

	JSVC_IsListnerType iListener = JSVC_JobManager::newInstance()->isListenerFunc(
													getJobInstance()->getJobSpec(), 
													getSubScenarioID(),
													&e_JobType);

	e_JobType = getJobInstance()->getJobSpec()->pc_Common->c_JobType.getJobType();
	getJobInstance()->getJobSpec()->pc_Common->c_JobType.getDocumentNo(sl_DocumentNo);

	if(iListener == JSVC_IsListner_NoListner) {
// 2018/08/07 minhdh AR1053349 apply CQ379669, del Start.
//// 2012/08/20 CQ370710対応 Start.
//		//if(e_JobType == TYPE_JobTypeFaxSendTimer && sl_DocumentNo[0] == TYPD_NotSetting) {
//		if( (e_JobType == TYPE_JobTypeFaxSendTimer) &&
//			(getJobInstance()->getJobSpec()->pc_Common->c_JobType.getBoxNo() != DOCD_FaxSndBox) &&
//			(getJobInstance()->getJobSpec()->pc_Common->c_JobType.getBoxNo() != DOCD_RevivalSndBox) ) {
//			// タイマー予約ジョブ
//			MyTraceSS(JSVD_DBG_REQANS, "isRequestStatusExecEnd2 ReqKeep " , "TimerReserveJob" );
//			return False;
//		//} else if(e_JobType == TYPE_JobTypeFaxSendTimer && sl_DocumentNo[0] != TYPD_NotSetting) {
//		} else if( (e_JobType == TYPE_JobTypeFaxSendTimer) &&
//			((getJobInstance()->getJobSpec()->pc_Common->c_JobType.getBoxNo() == DOCD_FaxSndBox) ||
//			 (getJobInstance()->getJobSpec()->pc_Common->c_JobType.getBoxNo() == DOCD_RevivalSndBox)) ) {
//// 2012/08/20 CQ370710対応 End.
//			// タイマー送信ジョブ
//			//Nop
//			MyTraceSS(JSVD_DBG_REQANS, "isRequestStatusExecEnd2 ReqKeep " , "TimerSendJob" );
//		}
//		// 2012/10/19 CQ373523対応 add start.
//		else if (TYPE_JobTypeFaxReceiveTimer == e_JobType) {
//			// タイマーポーリング受信ジョブ時もサブシナリオを削除しない.
//			MyTraceSS(JSVD_DBG_REQANS, "isRequestStatusExecEnd ReqKeep " , "TimerPollingReserveJob" );
//			return False;
//		}
//		// 2012/10/19 CQ373523対応 add end.
// 2018/08/07 minhdh AR1053349 apply CQ379669, del End.
	} else if(iListener == JSVC_IsListner_Listner) {
		if (getJobInstance()->getTransmissonOn() != False) {
			// 転送/中継/TSIジョブ
			MyTraceSS(JSVD_DBG_REQANS, "isRequestStatusExecEnd2 ReqKeep " , "RelayJob" );
			return False;
		}
	} else {
		//Nop
	}

	return True;
}

// Init完了、Functionが全て実行完了、End完了かどうか判定(メモリ開放要求完了判定).
Bool JSVC_ParentRequest::isRequestStatusDeleteEnd()
{
// CQ:365275対応 Sky_iwasaki 2012.05.17 Start.
	if (NULL != getJobInstance()){
		uchar uc_ImageLogSend = getJobInstance()->getJobSpec()->pc_Common->getImageLogSend();
		if ((TYPD_EasyLogOnlySendJob == uc_ImageLogSend) &&
			(JSVE_FunctionStatus_End == ul_FunctionStatusTable[JSVE_FunctionNumber_NetTx]))
		{
			return True;
		}
	}
// CQ:365275対応 Sky_iwasaki 2012.05.17 End.

	// InitProcess→終了.
	// 全てのFunctionが未使用か終了.
	// EndProcess→終了.
	// このときにTrueを返す。その他は全てFalse.
	if( JSVE_FunctionStatus_End != ul_FunctionStatusTable[JSVE_FunctionNumber_CommonSetting] ){
		return False;
	}
	for( int i_Cnt=JSVE_FunctionNumber_Non; i_Cnt<=JSVE_FunctionNumber_AddEmailContents; i_Cnt++ ){
		if( JSVE_FunctionStatus_Non != ul_FunctionStatusTable[i_Cnt]
		&&  JSVE_FunctionStatus_End != ul_FunctionStatusTable[i_Cnt]
		){
			return False;
		}
	}
	if( JSVE_FunctionStatus_End != ul_FunctionStatusTable[JSVE_FunctionNumber_FaxCommonSetting] ){
		return False;
	}
	
	//継続フラグの確認.
	if( False != b_WaitNextJobPointer){
		return False;
	}

	TYPE_JobType e_JobType = 0;
	JSVC_IsListnerType iListener = JSVC_JobManager::newInstance()->isListenerFunc(
													getJobInstance()->getJobSpec(), 
													getSubScenarioID(),
													&e_JobType);

	if(iListener == JSVC_IsListner_Listner) {
		if (getJobInstance()->getTransmissonOn() != False) {
			// 転送/中継/TSIジョブ
			MyTraceSS(JSVD_DBG_REQANS, "isRequestStatusDeleteEnd ReqKeep " , "RelayJob" );
			return False;
		}

		TYPE_JobType e_JobType	= getJobInstance()->getJobSpec()->pc_Common->c_JobType.getJobType();
		TYPE_TxMode e_TxMode 	= getJobInstance()->getJobSpec()->pc_Fax->c_FaxAct.getTxMode();
		Bool b_TimerOn 			= getJobInstance()->getJobSpec()->pc_Fax->c_FaxCommon.getTimerOn();

		// PC-FAXタイマー保存ジョブの場合.
		if (e_JobType == TYPE_JobTypePrint && e_TxMode == TYPE_TxModePcFax && b_TimerOn == True){
			// PC-FAXタイマー予約ジョブ.
			MyTraceSS(JSVD_DBG_REQANS, "isRequestStatusDeleteEnd ReqKeep " , "RelayJob" );
			return False;
		}
	} else {
		//Nop
	}

	return True;
}
// 市場問題(FQ-OPSD-1500097-OFP068)対応 2015/5/26 m.ooba start.
// 2ジョブ構成の送信ジョブ(2ジョブ目)判定(ジョブ開始通知).
Bool JSVC_ParentRequest::isRequestStatusStartOutputJob()
{
	TYPE_JobType e_JobType = 0;

	e_JobType = getJobInstance()->getJobSpec()->pc_Common->c_JobType.getJobType();

	// タイマー送信ジョブ.
	if( (e_JobType == TYPE_JobTypeFaxSendTimer) && 
		((getJobInstance()->getJobSpec()->pc_Common->c_JobType.getBoxNo() == DOCD_FaxSndBox) ||
		 (getJobInstance()->getJobSpec()->pc_Common->c_JobType.getBoxNo() == DOCD_RevivalSndBox)) )
	{
		MyTraceSS(JSVD_DBG_REQANS, "isRequestStatusStartOutputJob ReqKeep " , "TimerSendJob" );
		return True;
	}
	// DocSearcher起動かつ復旧ジョブでない場合.
	else if ( (True == getJobInstance()->getDocSearcherStartJob()) &&
			  (False == getJobInstance()->getWaitRevivalJob()) )
	{
		MyTraceSS(JSVD_DBG_REQANS, "isRequestStatusStartOutputJob ReqKeep " , "DocSearcher&NoRevival" );
		return True;
	}

	return False;
}
// Init完了、Functionが全て実行完了、End完了かつ2ジョブ構成(ジョブ終了通知).
Bool JSVC_ParentRequest::isRequestStatusDeleteEnd2()
{
	// 画像ログジョブは対象外.
	if (NULL != getJobInstance()){
		uchar uc_ImageLogSend = getJobInstance()->getJobSpec()->pc_Common->getImageLogSend();
		if ((TYPD_EasyLogOnlySendJob == uc_ImageLogSend) &&
			(JSVE_FunctionStatus_End == ul_FunctionStatusTable[JSVE_FunctionNumber_NetTx]))
		{
			return False;
		}
	}

	// InitProcess→終了.
	// 全てのFunctionが未使用か終了.
	// EndProcess→終了.
	// このときにTrueを返す。その他は全てFalse.
	if( JSVE_FunctionStatus_End != ul_FunctionStatusTable[JSVE_FunctionNumber_CommonSetting] ){
		return False;
	}
	for( int i_Cnt=JSVE_FunctionNumber_Non; i_Cnt<=JSVE_FunctionNumber_AddEmailContents; i_Cnt++ ){
		if( JSVE_FunctionStatus_Non != ul_FunctionStatusTable[i_Cnt]
		&&  JSVE_FunctionStatus_End != ul_FunctionStatusTable[i_Cnt]
		){
			return False;
		}
	}
	if( JSVE_FunctionStatus_End != ul_FunctionStatusTable[JSVE_FunctionNumber_FaxCommonSetting] ){
		return False;
	}
	
	TYPE_JobType e_JobType = 0;
	JSVC_IsListnerType iListener = JSVC_JobManager::newInstance()->isListenerFunc(
													getJobInstance()->getJobSpec(), 
													getSubScenarioID(),
													&e_JobType);

	if(iListener == JSVC_IsListner_Listner) {
		if (getJobInstance()->getTransmissonOn() != False) {
			// 転送/中継/TSIジョブ
			MyTraceSS(JSVD_DBG_REQANS, "isRequestStatusDeleteEnd ReqKeep " , "RelayJob" );
			return True;
		}

		TYPE_JobType e_JobType	= getJobInstance()->getJobSpec()->pc_Common->c_JobType.getJobType();
		TYPE_TxMode e_TxMode 	= getJobInstance()->getJobSpec()->pc_Fax->c_FaxAct.getTxMode();
		Bool b_TimerOn 			= getJobInstance()->getJobSpec()->pc_Fax->c_FaxCommon.getTimerOn();

		// PC-FAXタイマー保存ジョブの場合.
		if (e_JobType == TYPE_JobTypePrint && e_TxMode == TYPE_TxModePcFax && b_TimerOn == True){
			// PC-FAXタイマー予約ジョブ.
			MyTraceSS(JSVD_DBG_REQANS, "isRequestStatusDeleteEnd ReqKeep " , "RelayJob" );
			return True;
		}
	} else {
		//Nop
	}

	return False;
}
// 市場問題(FQ-OPSD-1500097-OFP068)対応 2015/5/26 m.ooba end.
// Init完了、Functionが全て実行完了もしくは停止中、End待機中かどうか判定(停止要求完了判定).
Bool JSVC_ParentRequest::isRequestStatusStopEnd()
{
	
	// Scanがエラー停止かつScanが停止状態.
	if(( APIC_ScannerWarning::newInstance()->isOccurAdfJam() != False )						//ADFでJAM発生中.
	&& (JSVE_FunctionStatus_Stop == ul_FunctionStatusTable[JSVE_FunctionNumber_Scan])){		//スキャンが停止.
		return True;
	}

	// InitProcess→終了.
	// 全てのFunctionが未使用か終了か停止中.
	// EndProcess→待機中.
	// このときにTrueを返す。その他は全てFalse.
	if( JSVE_FunctionStatus_End != ul_FunctionStatusTable[JSVE_FunctionNumber_CommonSetting] ){
		return False;
	}
	for( int i_Cnt=JSVE_FunctionNumber_Non; i_Cnt<=JSVE_FunctionNumber_AddEmailContents; i_Cnt++ ){
		if( JSVE_FunctionStatus_Non != ul_FunctionStatusTable[i_Cnt]
		&&  JSVE_FunctionStatus_End != ul_FunctionStatusTable[i_Cnt]
		&&  JSVE_FunctionStatus_Idle != ul_FunctionStatusTable[i_Cnt]
		&&  JSVE_FunctionStatus_Stop != ul_FunctionStatusTable[i_Cnt]
		){
			return False;
		}
	}
	if( JSVE_FunctionStatus_Idle != ul_FunctionStatusTable[JSVE_FunctionNumber_FaxCommonSetting]
	&&  JSVE_FunctionStatus_Non != ul_FunctionStatusTable[JSVE_FunctionNumber_FaxCommonSetting]
	){
		return False;
	}
	return True;
}

// Init完了、Functionがひとつでも停止になっているかどうか判定(自発停止完了判定).
Bool JSVC_ParentRequest::isExistStopedProcess()
{
	

	// InitProcess→終了.
	// ひとつでもFunctionが停止中.
	// このときにTrueを返す。その他は全てFalse.
	if( JSVE_FunctionStatus_End != ul_FunctionStatusTable[JSVE_FunctionNumber_CommonSetting] ){
		return False;
	}
	for( int i_Cnt=JSVE_FunctionNumber_Non; i_Cnt<=JSVE_FunctionNumber_AddEmailContents; i_Cnt++ ){
		if(  JSVE_FunctionStatus_Stop == ul_FunctionStatusTable[i_Cnt]
		){
			return True;
		}
	}
	return False;
}

// Init完了、Scan待機中、出力Functionが一つ以上実行中、EndProcess待機判定(分割読み込み停止).
Bool JSVC_ParentRequest::isRequestStatusMemInStopEnd()
{
	// InitProcess→終了.
	// ScanFunction→待機中.
	// その他のFunctionは未使用か待機中か実行中.
	// EndProcess→待機中.
	// このときにTrueを返す。その他は全てFalse.
	if( JSVE_FunctionStatus_End != ul_FunctionStatusTable[JSVE_FunctionNumber_CommonSetting] ){
		return False;
	}

	// メモリコピーの場合、ScanがEndになった状態でサブシナリオが停止する場合がある.
	// (確認コピー停止中画面表示中).
	// よって、下記条件を追加.
	if( getJobInstance()->getJobStatus()->getConfirmPrintStopState() != True )
	{
		if( JSVE_FunctionStatus_Idle != ul_FunctionStatusTable[JSVE_FunctionNumber_Scan] ){
			return False;
		}
	}

// 2013/06/06 CQ380573 Nagasawa add sta.
	Bool b_netTxSkip = False;
	
	if ((ul_FunctionStatusTable[JSVE_FunctionNumber_NetTx] == JSVE_FunctionStatus_End) &&
	    (getJobInstance()->getJobStatus()->getSendMode() == TYPE_SendModeSharedFolder))
		b_netTxSkip = True;
// 2013/06/06 CQ380573 Nagasawa add end.

	for( int i_Cnt=JSVE_FunctionNumber_NetRx; i_Cnt<=JSVE_FunctionNumber_AddEmailContents; i_Cnt++ ){
		if( JSVE_FunctionStatus_Non != ul_FunctionStatusTable[i_Cnt]
		&&  JSVE_FunctionStatus_Idle != ul_FunctionStatusTable[i_Cnt]
		&&  JSVE_FunctionStatus_Exec != ul_FunctionStatusTable[i_Cnt]
		){
// 2013/06/06 CQ380573 Nagasawa mod sta.
			//return False;
			if ((i_Cnt == JSVE_FunctionNumber_NetTx) && (b_netTxSkip == True)) {
				// SMB 共有フォルダ送信の場合に NetTx がエラー停止したケースでは処理を続行させる.
				;
			} else
				return False;
// 2013/06/06 CQ380573 Nagasawa mod end.
		}
	}
	if( JSVE_FunctionStatus_Idle != ul_FunctionStatusTable[JSVE_FunctionNumber_FaxCommonSetting]
	&&  JSVE_FunctionStatus_Non != ul_FunctionStatusTable[JSVE_FunctionNumber_FaxCommonSetting]
	){
		return False;
	}
	return True;
}

// 完了通知有無設定・取得.
void JSVC_ParentRequest::setEndNotice( JSVE_EndNoticeBit e_Bit, Bool b_Flg )
{
	if( b_Flg == True ){
		ul_EndNoticeComplete = (ul_EndNoticeComplete | e_Bit);
	}
	else{
		ul_EndNoticeComplete = (ul_EndNoticeComplete & ~e_Bit);
	}
}
Bool JSVC_ParentRequest::getEndNotice( JSVE_EndNoticeBit e_Bit )
{
	if( ul_EndNoticeComplete & e_Bit ){
		return True;
	}
	else{
		return False;
	}
}

//--------------------------------------------------------------------------------------------------.
//	JSVC_RequestJobSettingクラス定義.
//--------------------------------------------------------------------------------------------------.
// コンストラクタ.
JSVC_RequestJobSetting::JSVC_RequestJobSetting( uint32_t ul_RequestID ,uint32_t ul_MsgTypeID)
: JSVC_Request( ul_RequestID,ul_MsgTypeID ),pFile(NULL),ul_FilePathSize(0),p_DataJobSpec(NULL),pFileJobSpec(NULL), b_SendUpdateEnd(False)
	,ul_JobResourceID(-1)
{
}
// デストラクタ.
JSVC_RequestJobSetting::~JSVC_RequestJobSetting()
{
	if( pFile ) {
		free(pFile);
		pFile = NULL;
	}
	if( p_DataJobSpec ) {
		delete p_DataJobSpec;
		p_DataJobSpec = NULL;
	}
	if( pFileJobSpec ) {
		free(pFileJobSpec);
		pFileJobSpec = NULL;
	}
}
// 自クラスの型を返す
JSVE_Request_Class_Type JSVC_RequestJobSetting::WhoAmI()
{
	return JSVE_Request_Class_Type_Req_Job_Set;
}
// JobManagerから操作するI/Fを定義.
JSVE_Request_Result JSVC_RequestJobSetting::startAction(JSVS_NotifyInfo* pNotify)
{
	JSVE_Request_Result e_Request_Result;
	JSVC_ParentRequest* pc_PRequest = GetParentRequest();
	
	// リクエストデータ内の状態遷移処理.
	if( NULL != pc_PRequest ){
		setRequestStatus( pc_PRequest->getRequestStatus() );
	}
	e_Request_Result = JSVC_Request::startAction( pNotify );
	if( NULL != pc_PRequest ){
		pc_PRequest->setRequestStatus(getRequestStatus());
	}
	return e_Request_Result;
}
// リクエスト受付時のデータ保存処理.
Bool JSVC_RequestJobSetting::setData(const char* pRcvData,short ss_size)
{
#ifdef _DEBUG_JSVC_SEQ
	MyTraceSD(JSVD_DBG_INFO, "JSVC_RequestJobSetting::setData ss_size:" , ss_size );		// 回避方法をトレースに変更.
#endif

	MyTraceSD( JSVD_DBG_INFO, "JSVC_RequestJobSetting::setData MsgTypeId :" , (int)getMsgTypeId() );	// warning対応 2013/09/18 ooba.
	switch( getMsgTypeId() ){
		case JSVE_JobManager_MsgID_MemoryAlloc:
			{
				// メモリ確保要求時のデータ構造体.
				JSVS_ReqPara_MemoryAlloc* pMsg = (JSVS_ReqPara_MemoryAlloc*)pRcvData;

				setSubScenarioID( pMsg->ul_SubScenarioID );							// サブシナリオID.
				setScenarioID( pMsg->ul_ScenarioID );								// シナリオID.
				e_DataType = 1;														// データ種別を設定(ファイル有).
				if( pFile != NULL ){												// もし獲得済であれば開放してから.
					free(pFile);
					pFile = NULL;
				}
				if( pMsg->ul_ReqDataSize <= JSVD_XMLFILEPATH_MAX ){
					// ファイルパスサイズ.
					ul_FilePathSize = pMsg->ul_ReqDataSize;
					pFile = (char*)malloc(ul_FilePathSize+1); 
//Str【 静的解析対応 : Sky田村 : 2012.09.03 : CID-439627 】.
					if( pFile != NULL )
					{
					memset( pFile, 0x00, ul_FilePathSize+1 );
					// XMLファイル.
					strncpy( pFile, pMsg->sc_ReqData, pMsg->ul_ReqDataSize );
				}
					else
					{
						MyTraceSS( JSVD_DBG_ERROR, "JSVC_RequestJobSetting::setData", "malloc Error pFile 1" );
					}
//End【 静的解析対応 : Sky田村 : 2012.09.03 : CID-439627 】.
				}
				else{
					ul_FilePathSize = JSVD_XMLFILEPATH_MAX;							// ファイルパスサイズの確定.
					pFile = (char*)malloc(ul_FilePathSize+1); 						// メモリの取得.
//Str【 静的解析対応 : Sky田村 : 2012.09.03 : CID-439627 】.
					if( pFile != NULL )
					{
					memset( pFile, 0x00, ul_FilePathSize+1 );
					strncpy( pFile, pMsg->sc_ReqData, JSVD_XMLFILEPATH_MAX );		// ファイルパスを保存.
				}
					else
					{
						MyTraceSS( JSVD_DBG_ERROR, "JSVC_RequestJobSetting::setData", "malloc Error pFile 2" );
					}
//End【 静的解析対応 : Sky田村 : 2012.09.03 : CID-439627 】.
				}
				this->ul_JobResourceID = pMsg->ul_JobResouceID;
			}
			break;

		case JSVE_JobManager_MsgID_ChangeSetting:
			{
				// 設定変更要求時のデータ構造体.
				JSVS_ReqPara_ChangeSetting* pMsg = (JSVS_ReqPara_ChangeSetting*)pRcvData;

				setSubScenarioID( pMsg->ul_SubScenarioID );							// サブシナリオID.
				setScenarioID( pMsg->ul_ScenarioID );								// シナリオID.
				e_DataType = 1;														// データ種別を設定(ファイル有).
				if( pFile != NULL ){												// もし獲得済であれば開放してから.
					free(pFile);
					pFile = NULL;
				}
				if( pMsg->ul_ReqDataSize <= JSVD_XMLFILEPATH_MAX ){
					// ファイルパスサイズ.
					ul_FilePathSize = pMsg->ul_ReqDataSize;
					pFile = (char*)malloc(ul_FilePathSize+1); 
//Str【 静的解析対応 : Sky田村 : 2012.09.03 : CID-439627 】.
					if( pFile != NULL )
					{
					memset( pFile, 0x00, ul_FilePathSize+1 );
					// XMLファイル.
					strncpy( pFile, pMsg->sc_ReqData, pMsg->ul_ReqDataSize );
				}
					else
					{
						MyTraceSS( JSVD_DBG_ERROR, "JSVC_RequestJobSetting::setData", "malloc Error pFile 3" );
					}
//End【 静的解析対応 : Sky田村 : 2012.09.03 : CID-439627 】.
				}
				else{
					ul_FilePathSize = JSVD_XMLFILEPATH_MAX;							// ファイルパスサイズの確定.
					pFile = (char*)malloc(ul_FilePathSize+1); 						// メモリの取得.
//Str【 静的解析対応 : Sky田村 : 2012.09.03 : CID-439627 】.
					if( pFile != NULL )
					{
					memset( pFile, 0x00, ul_FilePathSize+1 );
					strncpy( pFile, pMsg->sc_ReqData, JSVD_XMLFILEPATH_MAX );		// ファイルパスを保存.
				}
					else
					{
						MyTraceSS( JSVD_DBG_ERROR, "JSVC_RequestJobSetting::setData", "malloc Error pFile 4" );
					}
//End【 静的解析対応 : Sky田村 : 2012.09.03 : CID-439627 】.
				}
			}
			break;
		default:
			MyTraceSS( JSVD_DBG_ERROR, "JSVC_RequestJobSetting::setData", "unknown MsgType" );
			break ;
	}
	return True;
}
// リクエスト受付時のデータ保存処理.
Bool JSVC_RequestJobSetting::setDataToParent(const char* pRcvData,short ss_size,JSVC_ParentRequest* p_PReq)
{
#ifdef _DEBUG_JSVC_SEQ
	MyTraceSD(JSVD_DBG_INFO, "JSVC_RequestJobSetting::setDataToParent ss_size:" , ss_size );		// 回避方法をトレースに変更.
#endif
	if( p_PReq == NULL || pRcvData == NULL ){
		return False;
	}
	if( JSVE_JobManager_MsgID_MemoryAlloc == getMsgTypeId() ) 
	{
		// メモリ確保要求時のデータ構造体.
		JSVS_ReqPara_MemoryAlloc* pMsg = (JSVS_ReqPara_MemoryAlloc*)pRcvData;
		p_PReq->setPowerOnReStartType( pMsg->e_PowerOnReStartType );
		p_PReq->setCharegeInfo(&(pMsg->saddr));
	}
	return True;
}
// 指定した応答に関連するリクエストかどうかを判定.
short JSVC_RequestJobSetting::isResponseMsg(JSVC_Response* pRes)
{
	// ジョブ系の判別はベースに持っているIFをそのまま使う.
	return JSVC_Request::isResponseMsg( pRes );
}

char* JSVC_RequestJobSetting::getJobSpecFilePath()
{
	return pFileJobSpec;
}

void JSVC_RequestJobSetting::setJobSpecFilePath( const char* psc_File )
{
	if( pFileJobSpec != NULL ){
		free( pFileJobSpec );
		pFileJobSpec = NULL;
	}
	pFileJobSpec = (char*)malloc( strlen(psc_File)+1 );
	if( pFileJobSpec != NULL ){
		memset( pFileJobSpec, 0x00, strlen(psc_File)+1 );
		strncpy( pFileJobSpec, psc_File, strlen(psc_File) );
	}
}

// JobSpecファイルパスを返す.
char* JSVC_RequestJobSetting::getFilePath()
{
	return pFile;
}

//--------------------------------------------------------------------------------------------------.
//	JSVC_RequestJobControlクラス定義.
//--------------------------------------------------------------------------------------------------.
// コンストラクタ.
JSVC_RequestJobControl::JSVC_RequestJobControl( uint32_t ul_RequestID ,uint32_t ul_MsgTypeID)
: JSVC_Request( ul_RequestID,ul_MsgTypeID )
, ul_DocID(0)
{
}
// 自クラスの型を返す.
JSVE_Request_Class_Type JSVC_RequestJobControl::WhoAmI()
{
	return JSVE_Request_Class_Type_Req_Job_Cntl;
}
// JobManagerから操作するI/Fを定義.
JSVE_Request_Result JSVC_RequestJobControl::startAction(JSVS_NotifyInfo* pNotify)
{
	JSVE_Request_Result e_Request_Result;
	JSVC_ParentRequest* pc_PRequest = GetParentRequest();
	// リクエストデータ内の状態遷移処理.
	if( NULL != pc_PRequest ){
		setRequestStatus( pc_PRequest->getRequestStatus() );
	}
	e_Request_Result = JSVC_Request::startAction( pNotify );
	if( NULL != pc_PRequest ){
		pc_PRequest->setRequestStatus(getRequestStatus());
	}
	return e_Request_Result;
}

//
// リクエスト受付時のデータ保存処理.
//  return  True:成功	False:失敗.
//
Bool JSVC_RequestJobControl::setData(const char* pRcvData,short ss_size)
{
#ifdef _DEBUG_JSVC_SEQ
	MyTraceSD(JSVD_DBG_INFO, "JSVC_RequestJobControl::setData ss_size:" , ss_size );		// 回避方法をトレースに変更.
#endif
	//ジョブ設定系のメッセージ構成に合わせて参照するためのキャスト.
	struct JSVS_ReqMsgParam_DelInputData {
		uint32_t ul_SubScenarioID;
		uint32_t ul_DocID;
	};
	JSVS_ReqMsgParam_DelInputData* pMsg = (JSVS_ReqMsgParam_DelInputData*)pRcvData;
	// サブシナリオID.
	setSubScenarioID(pMsg->ul_SubScenarioID);
	// データ種別.
	e_DataType = 0;		// データ種別を設定(ファイル無).

	return True;
}
// 指定した応答に関連するリクエストかどうかを判定.
short JSVC_RequestJobControl::isResponseMsg(JSVC_Response* pRes)
{
	// ジョブ系の判別はベースに持っているIFをそのまま使う.
	return JSVC_Request::isResponseMsg( pRes );
}

//--------------------------------------------------------------------------------------------------.
//	JSVC_RequestJobControlPreviewクラス定義.
//--------------------------------------------------------------------------------------------------.
// コンストラクタ.
JSVC_RequestJobControlPreview::JSVC_RequestJobControlPreview(uint32_t ul_RequestID,uint32_t ul_MsgTypeID)
:	JSVC_Request(ul_RequestID,ul_MsgTypeID) , ul_AcceptNum(0),pst_VerifyParam(NULL),sl_PageNum(0),pst_OPOData(NULL)
{
	;
}
// デストラクタ.
JSVC_RequestJobControlPreview::~JSVC_RequestJobControlPreview()
{
	if ( pst_VerifyParam != NULL ){
		delete pst_VerifyParam;
		pst_VerifyParam = NULL;
	}
	if( pst_OPOData != NULL ){
		delete pst_OPOData;
	}
}
// 自クラスの型を返す.
JSVE_Request_Class_Type JSVC_RequestJobControlPreview::WhoAmI(void)
{
	return JSVE_Request_Class_Type_Req_Job_Prev;
}
// JobManagerから操作するI/Fを定義.
JSVE_Request_Result JSVC_RequestJobControlPreview::startAction(JSVS_NotifyInfo* pNotify)
{
	JSVE_Request_Result e_Request_Result = JSVE_Request_Result_Ok;
	JSVC_ParentRequest* pc_PRequest = GetParentRequest();
	// リクエストデータ内の状態遷移処理.
	if ( NULL == pc_PRequest ){
		MyTraceSD(JSVD_DBG_ERROR, "JSVC_RequestJobControlPreview::startAction can't get parent  SubSceID=", (int)getSubScenarioID() );	// warning対応 2013/09/18 ooba.
		return JSVE_Request_Result_Err;
	}

	switch(pNotify->e_JobManager_MsgTypeID){
		// 要求系MSG.
		case JSVE_JobManager_MsgID_MakePreviewImage:
			{
				uint32_t ul_index = -1;
				// ジョブ開始指示.
				ul_index = JSVC_JobManager::newInstance()->availableAnsFunctionIndex( JSVC_CB_INDEX_ans_verify_preview_data, JSVC_CB_MAX_ans_verify_preview_data, this );
				if( ul_index != (uint32_t)-1 ) {				// warning対応 2013/09/21 ooba.
					JSVC_RequestJobControlPreview* pReq = (JSVC_RequestJobControlPreview*)pNotify->pReq;
					TYPS_VerifyPreviewData s_VerifyPreviewData;
					memset( &s_VerifyPreviewData, 0x00, sizeof(s_VerifyPreviewData) );
					s_VerifyPreviewData.sl_PageNum				 = (int32_t)pReq->pst_VerifyParam->sl_PageNum;
					s_VerifyPreviewData.sl_DocIndex				 = (int32_t)pc_PRequest->getJobInstance()->getDocID();
					s_VerifyPreviewData.sc_ReqImgType			 = (char)pReq->pst_VerifyParam->e_ReqImgType;
					s_VerifyPreviewData.e_VerifyFormat			 = (TYPE_FileFormat)pReq->pst_VerifyParam->e_VerifyFormat;
					s_VerifyPreviewData.sc_ImageDir				 = (char)pReq->pst_VerifyParam->e_ImageDir;
					s_VerifyPreviewData.sc_ImagePlace			 = (char)pReq->pst_VerifyParam->e_ImagePlace;
					s_VerifyPreviewData.ss_ReqImageDotSizeWidth	 = (short)pReq->pst_VerifyParam->sl_ReqImageDotSizeWidth;
					s_VerifyPreviewData.ss_ReqImageDotSizeHight	 = (short)pReq->pst_VerifyParam->sl_ReqImageDotSizeHight;
					s_VerifyPreviewData.psc_PreviewImageFilePath = &(pReq->pst_VerifyParam->sc_ImageDataFileName[0]);
					s_VerifyPreviewData.ul_ImageDataPointerSize	 = (uint32_t)pReq->pst_VerifyParam->ul_ImageDataPointerSize;
					s_VerifyPreviewData.b_DoSynthesize			 = (Bool)pReq->pst_VerifyParam->ul_DoSynthesize;
					// 出力順序情報のバッファをリクエストに生成する.
					TYPS_PPreviewOnePOrder* pTemp = new TYPS_PPreviewOnePOrder();
					if(pTemp) {
						*pTemp = pc_PRequest->getJobInstance()->getPPreviewOPO( pc_PRequest->getJobInstance()->getJobStatus()->getJobID(), pReq->pst_VerifyParam->sl_PageNum );
						pReq->setOPOData( *pTemp );
						delete pTemp;
					}
					s_VerifyPreviewData.ps_PreviewOnePOrder		 = (TYPS_PPreviewOnePOrder*)pReq->getOPOData();
					ul_AcceptNum = pc_PRequest->getJobInstance()->req_getVerifyPreviewDataForJobService(&s_VerifyPreviewData,ans_getVerifyPreviewData01);
					this->ul_Result = JSVE_Request_Result_Ok;
					this->ul_ResultDetail = IPCE_JobsResultDetail_Non;

				   // Step2 静的解析対応 sky_muko CID.290480 不要コード削除 start.
					//this->ul_AcceptNum = ul_AcceptNum;
				   // Step2 静的解析対応 sky_muko CID.290480 不要コード削除 end.

					// CQ.359785 Start.
					JSVS_SendRecv_MsgParam sParam;
					sParam.ul_MsgTypeID = JSVE_JobManager_MsgID_MakePreviewImageAccept;
					sParam.pReq = this;
					sParam.pRes = NULL;
					JSVC_JobSendRecv::newInstance()->sendMessageToAPF(&sParam);
					// CQ.359785 End.

					//応答はJSVE_JobManager_MsgID_MakePreviewImageAcceptにて.
					e_Request_Result = JSVE_Request_Result_Ok;
				} else {
					//受付NGとする .
					MyTraceSD(JSVD_DBG_ERROR, "JSVC_RequestJobControlPreview::startAction MakePrev Err. e_Request_Event:" , pNotify->e_Request_Event );
					this->setResult(JSVE_JobServiceResult_NG);
					this->setResultDetail(IPCE_JobsResultDetail_CBNumOver);
					// JobManagerへ要求NGを通知する.
					e_Request_Result = JSVE_Request_Result_Err;
					JSVC_JobSendRecv::newInstance()->sendMessageToAPF( JSVE_JobManager_MsgID_MakePreviewImageAccept, this, NULL );
				}
			}
			break;
		case JSVE_JobManager_MsgID_DeletePreviewImage:
			{
				JSVC_RequestJobControlPreview* pReq = (JSVC_RequestJobControlPreview*)pNotify->pReq;
				int32_t sl_PageNo = pReq->pst_VerifyParam->sl_PageNum;
				Bool b_Result = pc_PRequest->getJobInstance()->deleteVerifyPreviewPage(sl_PageNo);
				if( False != b_Result )
				{
					this->ul_Result = JSVE_Request_Result_Ok;
					this->ul_ResultDetail = IPCE_JobsResultDetail_Non;
					//応答はJSVE_JobManager_MsgID_DeletePreviewImageAcceptにて.
					e_Request_Result = JSVE_Request_Result_Complete;
				} else {
					this->ul_Result = JSVE_Request_Result_Err;
					this->ul_ResultDetail = IPCE_JobsResultDetail_PrevDeleteFailure;
					e_Request_Result = JSVE_Request_Result_Err;
					MyTraceSD(JSVD_DBG_ERROR, "JSVC_RequestJobControlPreview::startAction NG",(int)pNotify->e_JobManager_MsgTypeID);
				}
				JSVC_JobSendRecv::newInstance()->sendMessageToAPF( JSVE_JobManager_MsgID_DeletePreviewImageAccept, this, NULL );
			}
			break;
		case JSVE_JobManager_MsgID_CancelPreviewImage:
			{
				//要求済のリクエストを検索.
				JSVC_RequestJobControlPreview* pc_RequestPrev = NULL;
				pc_RequestPrev = (JSVC_RequestJobControlPreview*)JSVC_JobManager::newInstance()->getRequestListByMsg( getSubScenarioID() ,JSVE_JobManager_MsgID_MakePreviewImage );
				while( pc_RequestPrev ){
					//見つかったら要求時のul_AcceptNumでキャンセルを実行.
					ul_AcceptNum = pc_RequestPrev->ul_AcceptNum;
				pc_PRequest->getJobInstance()->req_VerifyPreviewCancel(ul_AcceptNum);

					//中止した分については元の要求に応答してからリクエスト削除
					JSVC_ResponseJobNotify cRes(JSVE_JobManager_MsgID_MakePreviewImageEnd);
					cRes.setResult(JSVE_Request_Result_Err);
					cRes.setResultDetail(JSVE_JobServiceResultDetail_EndByCansel);
					JSVS_SendRecv_MsgParam sParam;
					sParam.ul_MsgTypeID = JSVE_JobManager_MsgID_MakePreviewImageEnd;
					sParam.pReq = this;
					sParam.pRes = &cRes;
					JSVC_JobSendRecv::newInstance()->sendMessageToAPF(&sParam);

					JSVC_JobManager::newInstance()->deleteRequest(pc_RequestPrev->get_srcinfo());
					pc_RequestPrev = NULL;

					//次があるか検索.
					pc_RequestPrev = (JSVC_RequestJobControlPreview*)JSVC_JobManager::newInstance()->getRequestListByMsg( getSubScenarioID() ,JSVE_JobManager_MsgID_MakePreviewImage );
				}
				this->ul_Result = JSVE_Request_Result_Ok;
				this->ul_ResultDetail = IPCE_JobsResultDetail_Non;
				e_Request_Result = JSVE_Request_Result_Complete;

				JSVC_JobSendRecv::newInstance()->sendMessageToAPF( JSVE_JobManager_MsgID_CancelPreviewImageAccept, this, NULL );
			}
			break;
		case JSVE_JobManager_MsgID_GetPageInfo_CanBeMade:
			{
				((JSVC_RequestJobControlPreview*)this)->sl_PageNum = pc_PRequest->getJobInstance()->getVerifyPreviewPageNum();
				this->ul_Result = JSVE_Request_Result_Ok;
				this->ul_ResultDetail = IPCE_JobsResultDetail_Non;
				JSVS_SendRecv_MsgParam sParam;
				sParam.ul_MsgTypeID = JSVE_JobManager_MsgID_GetPageInfo_CanBeMadeAccept;
				sParam.pReq = this;
				JSVC_JobSendRecv::newInstance()->sendMessageToAPF(&sParam);
				e_Request_Result = JSVE_Request_Result_Complete;		//同期で終わったから消していいことを伝える.
			}
			break;
		// 応答系MSG.
		case JSVE_JobManager_MsgID_MakePreviewImageAccept:
			{
				JSVC_Response* pRes = (JSVC_Response*)(pNotify->pRes);
				if(( pRes == NULL )
				|| ( pRes->WhoAmI() != JSVE_Request_Class_Type_Response )){
					e_Request_Result = JSVE_Request_Result_Err;
					return e_Request_Result;
				}
				this->ul_Result = pRes->getResult();
				this->ul_ResultDetail = pRes->getResultDetail();
				JSVS_SendRecv_MsgParam sParam;
				sParam.ul_MsgTypeID = pNotify->e_JobManager_MsgTypeID;
				sParam.pReq = this;
				sParam.pRes = pRes;
				JSVC_JobSendRecv::newInstance()->sendMessageToAPF(&sParam);
				e_Request_Result = JSVE_Request_Result_Ok;
			}
			break;
		case JSVE_JobManager_MsgID_DeletePreviewImageAccept:
		case JSVE_JobManager_MsgID_CancelPreviewImageAccept:
		case JSVE_JobManager_MsgID_GetPageInfo_CanBeMadeAccept:
			{
				JSVC_Response* pRes = (JSVC_Response*)(pNotify->pRes);
				if(( pRes == NULL )
				|| ( pRes->WhoAmI() != JSVE_Request_Class_Type_Response )){
					e_Request_Result = JSVE_Request_Result_Err;
					return e_Request_Result;
				}
				this->ul_Result = pRes->getResult();
				this->ul_ResultDetail = pRes->getResultDetail();
				JSVS_SendRecv_MsgParam sParam;
				sParam.ul_MsgTypeID = pNotify->e_JobManager_MsgTypeID;
				sParam.pReq = this;
				JSVC_JobSendRecv::newInstance()->sendMessageToAPF(&sParam);
				e_Request_Result = JSVE_Request_Result_Complete;
			}
			break;
		// 完了系MSG.
		case JSVE_JobManager_MsgID_MakePreviewImageEnd:
			{
				int32_t sl_PageNo = this->pst_VerifyParam->sl_PageNum;
				JSVC_Response* pRes = (JSVC_Response*)(pNotify->pRes);
				char c_ImgDir = (char)this->pst_VerifyParam->e_ImageDir;
				pc_PRequest->getJobInstance()->rotateVerifyPreviewPage(sl_PageNo,c_ImgDir);
				this->ul_Result = pRes->getResult();
				this->ul_ResultDetail = pRes->getResultDetail();
				JSVS_SendRecv_MsgParam sParam;
				sParam.ul_MsgTypeID = JSVE_JobManager_MsgID_MakePreviewImageEnd;
				sParam.pReq = this;
				sParam.pRes = pNotify->pRes;
				JSVC_JobSendRecv::newInstance()->sendMessageToAPF(&sParam);
				e_Request_Result = JSVE_Request_Result_Complete;
			}
			break;
		case JSVE_JobManager_MsgID_DeletePreviewImageEnd:
		case JSVE_JobManager_MsgID_CancelPreviewImageEnd:
		case JSVE_JobManager_MsgID_GetPageInfo_CanBeMadeEnd:
			{
				JSVC_Response* pRes = (JSVC_Response*)(pNotify->pRes);
				if(( pRes == NULL )
				|| ( pRes->WhoAmI() != JSVE_Request_Class_Type_Response )){
					e_Request_Result = JSVE_Request_Result_Err;
					return e_Request_Result;
				}
				this->ul_Result = pRes->getResult();
				this->ul_ResultDetail = pRes->getResultDetail();
				JSVS_SendRecv_MsgParam sParam;
				sParam.ul_MsgTypeID = pNotify->e_JobManager_MsgTypeID;
				sParam.pReq = this;
				JSVC_JobSendRecv::newInstance()->sendMessageToAPF(&sParam);
				e_Request_Result = JSVE_Request_Result_Complete;
			}
			break;
	}

	return e_Request_Result;
}
// リクエスト受付時のデータ保存処理.
Bool JSVC_RequestJobControlPreview::setData(const char* pRcvData,short ss_size)
{
#ifdef _DEBUG_JSVC_SEQ
	MyTraceSD(JSVD_DBG_INFO, "JSVC_RequestJobControlPreview::setData ss_size:" , ss_size );		// 回避方法をトレースに変更.
#endif
	//プレビュー設定系のメッセージ構成に合わせて参照するためのキャスト.
	struct JSVS_ReqMsgParam_Preview {
		uint32_t ul_SubScenarioID;
		IPCS_VerifyReqImgPara st_VerifyParam;
	};
	JSVS_ReqMsgParam_Preview* pMsg = (JSVS_ReqMsgParam_Preview*)pRcvData;
	// サブシナリオID.
	setSubScenarioID(pMsg->ul_SubScenarioID);
	// データ種別.
	e_DataType = 0;		// データ種別を設定(ファイル無).

	if (( getMsgTypeId() == JSVE_JobManager_MsgID_MakePreviewImage )
	||  ( getMsgTypeId() == JSVE_JobManager_MsgID_DeletePreviewImage )){
		if( pst_VerifyParam != NULL ){			// もし獲得済であれば開放してから.
			delete pst_VerifyParam;
			pst_VerifyParam = NULL;
		}
		pst_VerifyParam = new IPCS_VerifyReqImgPara;
		memcpy( pst_VerifyParam , &(pMsg->st_VerifyParam) ,sizeof(IPCS_VerifyReqImgPara) );
	}
	MyTraceSD( JSVD_DBG_INFO, "JSVC_RequestJobControlPreview::setData SubScenarioID:" , (int)pMsg->ul_SubScenarioID );	// warning対応 2013/09/18 ooba.

	// サブシナリオ実行状態を復帰.
	JSVC_ParentRequest* pc_PRequest = GetParentRequest();
	if( NULL != pc_PRequest){
		setRequestStatus(pc_PRequest->getRequestStatus());
	} else {
		MyTraceSS( JSVD_DBG_ERROR, "JSVC_RequestJobControlPreview::setData ", "GetParent is NULL." ) ;
		return False;
	}

	return True;
}
// 指定した応答に関連するリクエストかどうかを判定.
//  return		JSVD_REQUEST_NOT_MATCH		関係なし.
//				JSVD_REQUEST_MAYBE_MATCH	関係あり（他候補の可能性あり）.
//				JSVD_REQUEST_JUST_MATCH		関係あり（他候補の可能性なし）.
short JSVC_RequestJobControlPreview::isResponseMsg(JSVC_Response* pRes)
{
	short ss_Ret = JSVD_REQUEST_NOT_MATCH;
	// 要求の確認.
	uint32_t ulReqMsgTypeId = getMsgTypeId();
	uint32_t ulResMsgTypeId = pRes->getMsgTypeId();
	MyTraceSD( JSVD_DBG_INFO, "JSVC_RequestJobControlPreview::isResponseMsg MsgType:" , (int)ulReqMsgTypeId );	// warning対応 2013/09/18 ooba.
	// 要求別の判断.
	switch(ulReqMsgTypeId){
		case JSVE_JobManager_MsgID_MakePreviewImage:		// プレビュー画像作成要求.
			if(( ulResMsgTypeId == JSVE_JobManager_MsgID_MakePreviewImageAccept )
			|| ( ulResMsgTypeId == JSVE_JobManager_MsgID_MakePreviewImageEnd    )){
				if( getAcceptNum() == pRes->getGeneralPurposeData() ){
					ss_Ret = JSVD_REQUEST_JUST_MATCH;
				}
			}
			break;
		case JSVE_JobManager_MsgID_DeletePreviewImage:		// プレビュー画像削除要求.
			if( ulResMsgTypeId == JSVE_JobManager_MsgID_DeletePreviewImageAccept ){		//応答のみ.
				if( getAcceptNum() == pRes->getGeneralPurposeData() ){
					ss_Ret = JSVD_REQUEST_JUST_MATCH;
				}
			}
			break;
		case JSVE_JobManager_MsgID_CancelPreviewImage:		// プレビュー画像作成中止要求.
			if(( ulResMsgTypeId == JSVE_JobManager_MsgID_CancelPreviewImageAccept )
			|| ( ulResMsgTypeId == JSVE_JobManager_MsgID_CancelPreviewImageEnd    )){
				if( getAcceptNum() == pRes->getGeneralPurposeData() ){
					ss_Ret = JSVD_REQUEST_JUST_MATCH;
				}
			}
			break;
		case JSVE_JobManager_MsgID_GetPageInfo_CanBeMade:	// プレビュー画像作成可能ページ情報取得要求.
			if(( ulResMsgTypeId == JSVE_JobManager_MsgID_GetPageInfo_CanBeMadeAccept )
			|| ( ulResMsgTypeId == JSVE_JobManager_MsgID_GetPageInfo_CanBeMadeEnd    )){
				if( getAcceptNum() == pRes->getGeneralPurposeData() ){
					ss_Ret = JSVD_REQUEST_JUST_MATCH;
				}
			}
			break;
		default:
			MyTraceSS( JSVD_DBG_ERROR, "JSVC_RequestJobControlPreview::isResponseMsg", "unknown MsgType" );
			break;
	}
	return ss_Ret;
}
// プレビュー要求管理番号の取得.
uint32_t JSVC_RequestJobControlPreview::getAcceptNum()
{
	return ul_AcceptNum;
}
// 画像作成要求時の管理番号を設定する.
void JSVC_RequestJobControlPreview::setAcceptNum(uint32_t ul_AcceptNumber)
{
	ul_AcceptNum = ul_AcceptNumber;
}

void JSVC_RequestJobControlPreview::setOPOData( TYPS_PPreviewOnePOrder& st_OPOData )
{
	if( pst_OPOData == NULL ){
		pst_OPOData = new TYPS_PPreviewOnePOrder;
	}
	memset( pst_OPOData, 0x00, sizeof(TYPS_PPreviewOnePOrder) );
	memcpy( pst_OPOData, &st_OPOData, sizeof(TYPS_PPreviewOnePOrder) );
}

TYPS_PPreviewOnePOrder* JSVC_RequestJobControlPreview::getOPOData()
{
	return pst_OPOData;
}

// プレビュー作成完了コールバック.
//	ul_AcceptNum				IN	要求管理番号.
//	e_Result					IN	プレビュー画像生成結果.
//	s_ActualPreviewPara			IN	プレビュー画像パラメータ(結果).
void JSVC_RequestJobControlPreview::ans_getVerifyPreviewData01(uint32_t ul_AcceptNum,TYPE_PreviewResult e_Result,TYPS_ActualPreviewPara s_ActualPreviewPara )
{
	char sc_index = 0 + JSVC_CB_INDEX_ans_verify_preview_data;
	ans_getVerifyPreviewData_wrap( ul_AcceptNum, e_Result,s_ActualPreviewPara ,sc_index);
}
void JSVC_RequestJobControlPreview::ans_getVerifyPreviewData02(uint32_t ul_AcceptNum,TYPE_PreviewResult e_Result,TYPS_ActualPreviewPara s_ActualPreviewPara )
{
	char sc_index = 1 + JSVC_CB_INDEX_ans_verify_preview_data;
	ans_getVerifyPreviewData_wrap( ul_AcceptNum, e_Result,s_ActualPreviewPara ,sc_index);
}
void JSVC_RequestJobControlPreview::ans_getVerifyPreviewData03(uint32_t ul_AcceptNum,TYPE_PreviewResult e_Result,TYPS_ActualPreviewPara s_ActualPreviewPara )
{
	char sc_index = 2 + JSVC_CB_INDEX_ans_verify_preview_data;
	ans_getVerifyPreviewData_wrap( ul_AcceptNum, e_Result,s_ActualPreviewPara ,sc_index);
}
void JSVC_RequestJobControlPreview::ans_getVerifyPreviewData04(uint32_t ul_AcceptNum,TYPE_PreviewResult e_Result,TYPS_ActualPreviewPara s_ActualPreviewPara )
{
	char sc_index = 3 + JSVC_CB_INDEX_ans_verify_preview_data;
	ans_getVerifyPreviewData_wrap( ul_AcceptNum, e_Result,s_ActualPreviewPara ,sc_index);
}
void JSVC_RequestJobControlPreview::ans_getVerifyPreviewData05(uint32_t ul_AcceptNum,TYPE_PreviewResult e_Result,TYPS_ActualPreviewPara s_ActualPreviewPara )
{
	char sc_index = 4 + JSVC_CB_INDEX_ans_verify_preview_data;
	ans_getVerifyPreviewData_wrap( ul_AcceptNum, e_Result,s_ActualPreviewPara ,sc_index);
}
void JSVC_RequestJobControlPreview::ans_getVerifyPreviewData06(uint32_t ul_AcceptNum,TYPE_PreviewResult e_Result,TYPS_ActualPreviewPara s_ActualPreviewPara )
{
	char sc_index = 5 + JSVC_CB_INDEX_ans_verify_preview_data;
	ans_getVerifyPreviewData_wrap( ul_AcceptNum, e_Result,s_ActualPreviewPara ,sc_index);
}
void JSVC_RequestJobControlPreview::ans_getVerifyPreviewData07(uint32_t ul_AcceptNum,TYPE_PreviewResult e_Result,TYPS_ActualPreviewPara s_ActualPreviewPara )
{
	char sc_index = 6 + JSVC_CB_INDEX_ans_verify_preview_data;
	ans_getVerifyPreviewData_wrap( ul_AcceptNum, e_Result,s_ActualPreviewPara ,sc_index);
}
void JSVC_RequestJobControlPreview::ans_getVerifyPreviewData08(uint32_t ul_AcceptNum,TYPE_PreviewResult e_Result,TYPS_ActualPreviewPara s_ActualPreviewPara )
{
	char sc_index = 7 + JSVC_CB_INDEX_ans_verify_preview_data;
	ans_getVerifyPreviewData_wrap( ul_AcceptNum, e_Result,s_ActualPreviewPara ,sc_index);
}
void JSVC_RequestJobControlPreview::ans_getVerifyPreviewData09(uint32_t ul_AcceptNum,TYPE_PreviewResult e_Result,TYPS_ActualPreviewPara s_ActualPreviewPara )
{
	char sc_index = 8 + JSVC_CB_INDEX_ans_verify_preview_data;
	ans_getVerifyPreviewData_wrap( ul_AcceptNum, e_Result,s_ActualPreviewPara ,sc_index);
}
void JSVC_RequestJobControlPreview::ans_getVerifyPreviewData10(uint32_t ul_AcceptNum,TYPE_PreviewResult e_Result,TYPS_ActualPreviewPara s_ActualPreviewPara )
{
	char sc_index = 9 + JSVC_CB_INDEX_ans_verify_preview_data;
	ans_getVerifyPreviewData_wrap( ul_AcceptNum, e_Result,s_ActualPreviewPara ,sc_index);
}
// Job開始のコールバック.
void JSVC_RequestJobControlPreview::ans_getVerifyPreviewData_wrap(uint32_t ul_AcceptNum,TYPE_PreviewResult e_Result,TYPS_ActualPreviewPara s_ActualPreviewPara,char sc_index)
{
	JSVC_RequestBase* pReq = NULL;
	void*	vpDataBuf = NULL;
	short ss_DataLen = 0;
	Bool b_Status;

	//排他区間ここから.
	{
		JSVC_CBManagerSemCtrl cCBManage;
		//CB管理領域から呼出パラメータを取得.
		pReq		= JSVC_JobManager::c_CBManage[(uchar)sc_index].pc_ReqBase;
		vpDataBuf	= JSVC_JobManager::c_CBManage[(uchar)sc_index].vp_DataBuf;
		ss_DataLen	= JSVC_JobManager::c_CBManage[(uchar)sc_index].ss_DataLen;
		b_Status	= JSVC_JobManager::c_CBManage[(uchar)sc_index].b_Status;
		//CB管理領域を開放.
		JSVC_JobManager::c_CBManage[(uchar)sc_index].pc_ReqBase = NULL;
		JSVC_JobManager::c_CBManage[(uchar)sc_index].vp_DataBuf = NULL;
		JSVC_JobManager::c_CBManage[(uchar)sc_index].ss_DataLen = 0;
		JSVC_JobManager::c_CBManage[(uchar)sc_index].b_Status   = False;		//管理領域を「未使用」に戻す.
	}
	//排他区間ここまで.

	//実装関数をコール.
	if( b_Status == True){
		ans_getVerifyPreviewData_body( ul_AcceptNum,e_Result,s_ActualPreviewPara , pReq );
	}
	//バッファはここで確実に削除.
	if( vpDataBuf ){
//Str【 静的解析対応 : Sky花岡 : 2012.08.30 : CID-436330】.
//JSVC_JobManager::c_CBManage[sc_index].vp_DataBufが使用されていない為、delete処理を削除.
//JSVC_JobManager::c_CBManage[sc_index].vp_DataBufを使用する場合、下記、delete処理を復活させる事。
//なお、void* で指されている領域を開放する場合はfreeで開放する事（確保はmalloc）。
//		delete vpDataBuf;
//End【 静的解析対応 : Sky花岡 : 2012.08.30 : CID-436330】.
		vpDataBuf = NULL;
	}
}
void JSVC_RequestJobControlPreview::ans_getVerifyPreviewData_body( uint32_t ul_AcceptNum,TYPE_PreviewResult e_Result,TYPS_ActualPreviewPara s_ActualPreviewPara , JSVC_RequestBase* pRequest )
{
	//リクエスト元を特定.
	JSVC_RequestJobControlPreview* pReq = (JSVC_RequestJobControlPreview*)pRequest;
	JSVC_ResponseJobNotify* pRes = new JSVC_ResponseJobNotify(JSVE_JobManager_MsgID_MakePreviewImageEnd);
	#if DEBUGLOG__REQUEST_LIFE
		MyTraceSLD(JSVD_DBG_INFO, "ans_getVerifyPreviewData_body new JSVC_ResponseJobNotify" , (intptr_t)pRes );
	#endif


	//パラメータの作成.
	if ( NULL != pRes  )
	{
		MyTraceSD(JSVD_DBG_INFO, "ans_getVerifyPreviewData ul_AcceptNum", (int)ul_AcceptNum ) ;
		//パラメータ取得用の構造体を初期化.
		JSVS_ResponseParamJobNotify s_ResponseParam;
		memset( &s_ResponseParam , 0 , sizeof(s_ResponseParam) );

		s_ResponseParam.ul_Result = e_Result;
		s_ResponseParam.ul_ResultDetail = JSVE_JobServiceResultDetail_Non;
		s_ResponseParam.ul_AcceptNum = ul_AcceptNum;
		s_ResponseParam.s_ActualPreviewPara = s_ActualPreviewPara;
		pRes->setData( (char*)&s_ResponseParam , sizeof(s_ResponseParam) );
		
		//JSVC_JobManagerへ送る.
		JSVC_JobManager::newInstance()->JSVC_JobManager::notify( JSVE_JobManager_MsgID_MakePreviewImageEnd, (JSVC_RequestBase*)pReq, (JSVC_ResponseBase*)pRes);
	}
	else{
		MyTraceSS(JSVD_DBG_ERROR, "ans_getVerifyPreviewData","NG");
	}
}

// Archi(Step3) 送信機能カスタマイズ Start.
//--------------------------------------------------------------------------------------------------.
//	JSVC_RequestVariousFileDataクラス定義.
//--------------------------------------------------------------------------------------------------.
// コンストラクタ.
JSVC_RequestVariousFileData::JSVC_RequestVariousFileData(uint32_t ul_RequestID,uint32_t ul_MsgTypeID)
:JSVC_Request( ul_RequestID,ul_MsgTypeID ),pFile(NULL),ul_FilePathSize(0),e_CreateFileType(0),sl_FileID(0)
{
}
// デストラクタ.
JSVC_RequestVariousFileData::~JSVC_RequestVariousFileData()
{
	if (pFile) 
	{
		free(pFile);
		pFile = NULL;
	}
}
// 自クラスの型を返す.
JSVE_Request_Class_Type JSVC_RequestVariousFileData::WhoAmI()
{
	return JSVE_Request_Class_Type_Req_VariousFile;
}
// 処理の実行.
JSVE_Request_Result	JSVC_RequestVariousFileData::startAction(JSVS_NotifyInfo* pNotify )	///< JobManagerから操作するI/Fを定義.
{
	JSVE_Request_Result eRet = JSVE_Request_Result_Ok;
	if( NULL != pNotify )
	{
		switch (pNotify->e_JobManager_MsgTypeID)
		{
		case JSVE_JobManager_MsgID_RcvNotify_CreateVariousFileData:
		case JSVE_JobManager_MsgID_VariousFileDataSend_Copy:
		case JSVE_JobManager_MsgID_VariousFileDataSend_Delete:
			{
				//ワーカースレッド（XMLタスク）へ処理要求.
				//実際の処理はJSVC_RequestVariousFileData::AsyncAction()で実施.
				this->pc_XmlAnalyze->req_StartVariousFileDataControl( (JSVC_Request*)this );
			
			}
			break;
		case JSVE_JobManager_MsgID_RcvNotify_CreateVariousFileDataEnd:
		case JSVE_JobManager_MsgID_VariousFileDataSend_CopyEnd:
		case JSVE_JobManager_MsgID_VariousFileDataSend_DeleteEnd:
			{
				//ワーカースレッド内からの結果を受けてへ上位へ返信.
				//実際にはJSVC_RequestVariousFileData::AsyncAction()内で行われた処理結果.
				JSVS_SendRecv_MsgParam sParam;
				sParam.ul_MsgTypeID = pNotify->e_JobManager_MsgTypeID;
				sParam.pReq = pNotify->pReq;
				sParam.pRes = pNotify->pRes;
				JSVC_JobSendRecv::newInstance()->sendMessageToAPF(&sParam);
				
				eRet = JSVE_Request_Result_Complete;
			}
			break;
		default:
			MyTraceSD( JSVD_DBG_ERROR, "JSVC_RequestVariousFileData::startAction unknown msgtype:" , (int)pNotify->e_JobManager_MsgTypeID );	// warning対応 2013/09/18 ooba.
			eRet = JSVE_Request_Result_Err;
			break;
		}

	} else {
		eRet = JSVE_Request_Result_Err;
	}

	return eRet;
}
void JSVC_RequestVariousFileData::AsyncAction()
{
	JSVC_ResponseVariousFileData* pRes = NULL;
	
	switch(getMsgTypeId())
	{
		//ファイル作成処理.
		case JSVE_JobManager_MsgID_RcvNotify_CreateVariousFileData:
			pRes = new JSVC_ResponseVariousFileData( JSVE_JobManager_MsgID_RcvNotify_CreateVariousFileDataEnd );
			#if DEBUGLOG__REQUEST_LIFE
				MyTraceSLD(JSVD_DBG_INFO, "JSVC_RequestVariousFileData::AsyncAction new JSVC_ResponseVariousFileData 1" , (intptr_t)pRes );
			#endif
			if( NULL != pRes )
			{
				LIBC_List<APIC_Job> c_MyList;
				APIC_Job* pc_myJob = NULL;
				Bool b_ret = False;
				// サブシナリオIDをキーにJobポインタの検索を行う.
				APIC_ActiveJobList::newInstance()->getList(TYPE_ActiveJobListAll, 1, 350, &c_MyList);
				for (pc_myJob = c_MyList.findFirst(); pc_myJob; pc_myJob = c_MyList.findNext())
				{
					// 未投入かつ受信系のジョブを検索.
					if (getSubScenarioID() == pc_myJob->getJobStatus()->getSubScenarioID()) {
						// ファイル作成要求.
						b_ret = JOBG_CreateFile(pc_myJob->getJobStatus()->getJobID(), getCreateFileType(), (uchar*)getFilePath());
						break;
					}
				}

				if (False == b_ret) {
					// 作成失敗時.
					pRes->setResult( JSVE_Request_Result_Err );
					pRes->setResultDetail( IPCE_JobsResultDetail_FileSystemError );
					MyTraceSLD(JSVD_DBG_ERROR, "JSVC_RequestVariousFileData::AsyncAction subScenario Not found   ID=",(intptr_t)getSubScenarioID());
				} else {
					//OKなら下記の処理.
					pRes->setResult( JSVE_Request_Result_Ok );
					pRes->setResultDetail( JSVE_JobServiceResultDetail_Non );
				}
			} else {
				//確保失敗.
				MyTraceSS(JSVD_DBG_ERROR, "JSVC_RequestVariousFileData::AsyncAction","malloc error 1");
			}
			break;
		//ファイルコピー処理.
		case JSVE_JobManager_MsgID_VariousFileDataSend_Copy:
			pRes = new JSVC_ResponseVariousFileData( JSVE_JobManager_MsgID_VariousFileDataSend_CopyEnd );
			#if DEBUGLOG__REQUEST_LIFE
				MyTraceSLD(JSVD_DBG_INFO, "JSVC_RequestVariousFileData::AsyncAction new JSVC_ResponseVariousFileData 2" , (intptr_t)pRes );
			#endif
			if( NULL != pRes )
			{
				uint32_t ul_result = JOBE_ApiResult_OK;	// Jobのファイル操作APIコール時の結果.
				int32_t sl_FlatFileID = 0xFFFFFFFF;		// コピー先のフラットファイルID.

				// アプリ指定のファイルをMFPProcessの領域へコピーする.
				ul_result = copyToTempFlatFile(getFilePath(), &sl_FlatFileID);
				if (JOBE_ApiResult_OK != ul_result) {
					MyTraceSS(JSVD_DBG_ERROR, "JSVC_RequestVariousFileData::AsyncAction","copyToTempFlatFile Error");
					pRes->setResult( JSVE_Request_Result_Err );
					pRes->setResultDetail( IPCE_JobsResultDetail_FileSystemError );
					break;
				}

				// ファイルクローズ後応答の情報を設定.
				pRes->setResult( JSVE_Request_Result_Ok );
				pRes->setResultDetail( JSVE_JobServiceResultDetail_Non );
				pRes->setFileID( sl_FlatFileID );
			} else {
				//確保失敗.
				MyTraceSS(JSVD_DBG_ERROR, "JSVC_RequestVariousFileData::AsyncAction","malloc error 2");
			}
			break;
		//ファイル削除処理.
		case JSVE_JobManager_MsgID_VariousFileDataSend_Delete:
			pRes = new JSVC_ResponseVariousFileData( JSVE_JobManager_MsgID_VariousFileDataSend_DeleteEnd );
			#if DEBUGLOG__REQUEST_LIFE
				MyTraceSLD(JSVD_DBG_INFO, "JSVC_RequestVariousFileData::AsyncAction new JSVC_ResponseVariousFileData 3" , (intptr_t)pRes );
			#endif
			if( NULL != pRes )
			{
				uint32_t ul_result = JOBE_ApiResult_OK;	// Jobのファイル操作APIコール時の結果.

				// コピーした一時ファイルを削除する.
				ul_result = deleteTempFlatFile(getFileID());
				if (JOBE_ApiResult_OK != ul_result) {
					MyTraceSD(JSVD_DBG_ERROR, "JSVC_RequestVariousFileData::AsyncAction deleteTempFlatFile Error FileId=",(int)getFileID() );
					pRes->setResult( JSVE_Request_Result_Err );
					pRes->setResultDetail( IPCE_JobsResultDetail_FileSystemError );
					break;
				}

				//OKなら下記の処理.
				pRes->setResult( JSVE_Request_Result_Ok );
				pRes->setResultDetail( JSVE_JobServiceResultDetail_Non );
			} else {
				//確保失敗.
				MyTraceSS(JSVD_DBG_ERROR, "JSVC_RequestVariousFileData::AsyncAction","malloc error 3");
			}
			break;
		default:
			MyTraceSD( JSVD_DBG_ERROR,  "JSVC_RequestVariousFileData::AsyncAction unknown msgtype:" , (int)getMsgTypeId() );	// warning対応 2013/09/18 ooba.
//Str【 静的解析対応 : Sky田村 : 2012.08.30 : CID-436583 】.
			MyTraceSS(JSVD_DBG_ERROR, "JSVC_RequestVariousFileData::AsyncAction","malloc error 3");
//End【 静的解析対応 : Sky田村 : 2012.08.30 : CID-436583 】.
			break;
	}
	

//Str【 静的解析対応 : Sky陰山 : 2012.10.29 : CID-436583 】.
	if( NULL != pRes )
	{
		//結果をJSVC_JobManagerへ送る.
		//上位への返信とリクエストの削除は自タスクのコンテキストで実施.
		JSVC_JobManager::newInstance()->notify( pRes->getMsgTypeId(), (JSVC_RequestBase*)this, (JSVC_ResponseBase*)pRes);
	}
	else
	{
		MyTraceSS(JSVD_DBG_ERROR, "JSVC_RequestVariousFileData::AsyncAction","notify error");
	}
//End【 静的解析対応 : Sky陰山 : 2012.10.29 : CID-436583 】.
	
}
//
// リクエスト受付時のデータ保存処理.
//
//	return	True　:保存OK
//			False :保存NG
//
Bool JSVC_RequestVariousFileData::setData(const char* pRcvData,short ss_size)
{
	Bool bRet = True;
#ifdef _DEBUG_JSVC_SEQ
	MyTraceSD(JSVD_DBG_INFO, "JSVC_RequestVariousFileData::setData ss_size:" , ss_size );	// 回避方法をトレースに変更.
#endif
	if( NULL != pRcvData )
	{
		switch( getMsgTypeId() )
		{
		case JSVE_JobManager_MsgID_RcvNotify_CreateVariousFileData:
		case JSVE_JobManager_MsgID_VariousFileDataSend_Copy:
			{
				JSVS_ReqPara_FileCreate* pData = (JSVS_ReqPara_FileCreate*)pRcvData;
				setSubScenarioID( pData->ul_SubScenarioID );
				e_CreateFileType = pData->e_CreateFileType;
				ul_FilePathSize	= pData->ul_ReqDataSize;
				if( JSVD_XMLFILEPATH_MAX > ul_FilePathSize )
				{
					pFile = (char*)malloc( ul_FilePathSize + 1 );
					if( pFile )
					{
						memset( pFile , 0 ,ul_FilePathSize + 1 );
						strncpy( pFile , &(pData->sc_ReqData[0]),ul_FilePathSize );
						MyTraceSS(JSVD_DBG_INFO, "JSVC_RequestVariousFileData::setData FileNameIs",pFile);
					}
				}
			}
			break;
		case JSVE_JobManager_MsgID_VariousFileDataSend_Delete:
			{
				JSVS_ReqPara_FileDelete* pData = (JSVS_ReqPara_FileDelete*)pRcvData;
				setSubScenarioID( pData->ul_SubScenarioID );
				sl_FileID	= pData->sl_FileID;
				MyTraceSD(JSVD_DBG_INFO, "JSVC_RequestVariousFileData::setData FileIdIs",(int)sl_FileID);
			}
			break;
		default:
			MyTraceSD( JSVD_DBG_ERROR, "JSVC_RequestVariousFileData::setData unknown msgtype:" , (int)getMsgTypeId() );	// warning対応 2013/09/18 ooba.
			bRet = False;
			break;
		}
	}

	return bRet;
}

// 指定した応答に関連するリクエストかどうかを判定.
// Acceptを先に処理するためにJSVD_REQUEST_MAYBE_MATCHとJSVD_REQUEST_JUST_MATCHの .
// 違いを設けてreturnする.
short JSVC_RequestVariousFileData::isResponseMsg(JSVC_Response* pRes)
{
	short ss_Ret = JSVD_REQUEST_NOT_MATCH;

	if( this->getSubScenarioID() == pRes->getSubScenarioID() ){

		uint32_t ul_MsgTypeID = pRes->getMsgTypeId();
		switch( this->getMsgTypeId() )
		{
			case JSVE_JobManager_MsgID_RcvNotify_CreateVariousFileData:
				if( ul_MsgTypeID == JSVE_JobManager_MsgID_RcvNotify_CreateVariousFileDataEnd )
				{
					ss_Ret = JSVD_REQUEST_MAYBE_MATCH;
				}
				else if ( ul_MsgTypeID == JSVE_JobManager_MsgID_RcvNotify_CreateVariousFileDataAccept )
				{
					ss_Ret = JSVD_REQUEST_JUST_MATCH;
				}
				break;
			case JSVE_JobManager_MsgID_VariousFileDataSend_Copy:
				if( ul_MsgTypeID == JSVE_JobManager_MsgID_VariousFileDataSend_CopyEnd )
				{
					ss_Ret = JSVD_REQUEST_MAYBE_MATCH;
				}
				else if ( ul_MsgTypeID == JSVE_JobManager_MsgID_VariousFileDataSend_CopyAccept )
				{
					ss_Ret = JSVD_REQUEST_JUST_MATCH;
				}
				break;
			case JSVE_JobManager_MsgID_VariousFileDataSend_Delete:
				if( ul_MsgTypeID == JSVE_JobManager_MsgID_VariousFileDataSend_DeleteEnd )
				{
					ss_Ret = JSVD_REQUEST_MAYBE_MATCH;
				}
				else if ( ul_MsgTypeID == JSVE_JobManager_MsgID_VariousFileDataSend_DeleteAccept )
				{
					ss_Ret = JSVD_REQUEST_JUST_MATCH;
				}
				break;
			default:
				MyTraceSD( JSVD_DBG_ERROR, "JSVC_RequestVariousFileData::isResponseMsg unknown msgtype:" , (int)this->getMsgTypeId() );	// warning対応 2013/09/18 ooba.
				break ;
		}
	}

	return ss_Ret;
}
// pFileのポインタ取得関数.
char* JSVC_RequestVariousFileData::getFilePath()
{
	return pFile;
}

// ul_FilePathSizeの値取得関数.
uint32_t JSVC_RequestVariousFileData::getFilePathSize()
{
	return ul_FilePathSize;
}

// 作成種別取得関数.
JSVE_CreateFileType JSVC_RequestVariousFileData::getCreateFileType()
{
	return e_CreateFileType;
}

// FileID取得関数.
int32_t JSVC_RequestVariousFileData::getFileID()
{
	return sl_FileID;
}
// ファイル作成コールバック.
void JSVC_RequestVariousFileData::ans_createFile(uintptr_t ul_CallBackID, int32_t sl_Ret)
{
	JSVC_ResponseVariousFileData* pRes = new JSVC_ResponseVariousFileData(JSVE_JobManager_MsgID_RcvNotify_CreateVariousFileDataEnd);
	#if DEBUGLOG__REQUEST_LIFE
		MyTraceSLD(JSVD_DBG_INFO, "JSVC_RequestVariousFileData::ans_createFile new JSVC_ResponseVariousFileData" , (intptr_t)pRes );
	#endif

	//パラメータの作成.
	if ( NULL != pRes  )
	{
		//パラメータ取得用の構造体を初期化.
		JSVS_VariousFileDataNoticeMsg s_ResponseParam;
		memset( &s_ResponseParam , 0 , sizeof(s_ResponseParam) );

		s_ResponseParam.e_JobServiceResult = JSVE_JobServiceResult_OK;
		s_ResponseParam.e_JobServiceResultDetail = JSVE_JobServiceResultDetail_Non;
		pRes->setData( (char*)&s_ResponseParam , sizeof(s_ResponseParam)-sizeof(int32_t) );
		
		//JSVC_JobManagerへ送る.
		JSVC_JobManager::newInstance()->JSVC_JobManager::notify( JSVE_JobManager_MsgID_RcvNotify_CreateVariousFileDataEnd, (JSVC_RequestBase*)ul_CallBackID, (JSVC_ResponseBase*)pRes);
	}
	else{
		MyTraceSS(JSVD_DBG_ERROR, "JSVC_RequestVariousFileData::ans_createFile","NG");
	}
}
// ファイルコピーコールバック.
void JSVC_RequestVariousFileData::ans_copyVariousFileData(uintptr_t ul_CallBackID, int32_t sl_Ret)
{
	JSVC_ResponseVariousFileData* pRes = new JSVC_ResponseVariousFileData(JSVE_JobManager_MsgID_VariousFileDataSend_CopyEnd);
	#if DEBUGLOG__REQUEST_LIFE
		MyTraceSLD(JSVD_DBG_INFO, "JSVC_RequestVariousFileData::ans_copyVariousFileData new JSVC_ResponseVariousFileData" , (intptr_t)pRes );
	#endif

	//パラメータの作成.
	if ( NULL != pRes  )
	{
		//パラメータ取得用の構造体を初期化.
		JSVS_VariousFileDataNoticeMsg s_ResponseParam;
		memset( &s_ResponseParam , 0 , sizeof(s_ResponseParam) );

		s_ResponseParam.e_JobServiceResult = JSVE_JobServiceResult_OK;
		s_ResponseParam.e_JobServiceResultDetail = JSVE_JobServiceResultDetail_Non;
		s_ResponseParam.sl_FileID = sl_Ret;
		pRes->setData( (char*)&s_ResponseParam , sizeof(s_ResponseParam) );
		
		//JSVC_JobManagerへ送る.
		JSVC_JobManager::newInstance()->JSVC_JobManager::notify( JSVE_JobManager_MsgID_VariousFileDataSend_CopyEnd, (JSVC_RequestBase*)ul_CallBackID, (JSVC_ResponseBase*)pRes);
	}
	else{
		MyTraceSS(JSVD_DBG_ERROR, "JSVC_RequestVariousFileData::ans_copyVariousFileData","NG");
	}
}
// ファイル削除コールバック.
void JSVC_RequestVariousFileData::ans_deleteVariousFileData(uintptr_t ul_CallBackID, int32_t sl_Ret)
{
	JSVC_ResponseVariousFileData* pRes = new JSVC_ResponseVariousFileData(JSVE_JobManager_MsgID_VariousFileDataSend_DeleteEnd);
	#if DEBUGLOG__REQUEST_LIFE
		MyTraceSLD(JSVD_DBG_INFO, "JSVC_RequestVariousFileData::ans_deleteVariousFileData new JSVC_ResponseVariousFileData" , (intptr_t)pRes );
	#endif

	//パラメータの作成.
	if ( NULL != pRes  )
	{
		//パラメータ取得用の構造体を初期化.
		JSVS_VariousFileDataNoticeMsg s_ResponseParam;
		memset( &s_ResponseParam , 0 , sizeof(s_ResponseParam) );

		s_ResponseParam.e_JobServiceResult = JSVE_JobServiceResult_OK;
		s_ResponseParam.e_JobServiceResultDetail = JSVE_JobServiceResultDetail_Non;
		pRes->setData( (char*)&s_ResponseParam , sizeof(s_ResponseParam)-sizeof(int32_t) );
		
		//JSVC_JobManagerへ送る.
		JSVC_JobManager::newInstance()->JSVC_JobManager::notify( JSVE_JobManager_MsgID_VariousFileDataSend_DeleteEnd, (JSVC_RequestBase*)ul_CallBackID, (JSVC_ResponseBase*)pRes);
	}
	else{
		MyTraceSS(JSVD_DBG_ERROR, "JSVC_RequestVariousFileData::ans_deleteVariousFileData","NG");
	}
}
//--------------------------------------------------------------------------------------------------.
//	JSVC_RequestTimerクラス定義.
//--------------------------------------------------------------------------------------------------.
// コンストラクタ.
JSVC_RequestTimer::JSVC_RequestTimer(uint32_t ul_RequestID,uint32_t ul_MsgTypeID)
:JSVC_Request( ul_RequestID,ul_MsgTypeID )
{
//Str【 静的解析対応 : Sky佐藤宏 : 2012.08.27 : CID-447280 】.
	s_TimerId	 = 0;
//End【 静的解析対応 : Sky佐藤宏 : 2012.08.27 : CID-447280 】.
	ul_ParamData = -1;
}

// デストラクタ.
JSVC_RequestTimer::~JSVC_RequestTimer()
{
}

// 自クラスの型を返す.
JSVE_Request_Class_Type JSVC_RequestTimer::WhoAmI()
{
	return JSVE_Request_Class_Type_Req_Timer;
}

// リクエスト受付時のデータ保存処理.
Bool JSVC_RequestTimer::setData(const char* pRcvData,short ss_size)
{
	s_TimerId = *(WDOG_ID*)pRcvData;
	return True;
}

// リクエスト受付時のデータ保存処理.
Bool JSVC_RequestTimer::setData(uint32_t ul_ParamData)
{
	JSVC_RequestTimer::ul_ParamData = ul_ParamData;
	return True;
}

// 指定した応答に関連するリクエストかどうかを判定.
short JSVC_RequestTimer::isResponseMsg(JSVC_Response*)
{
	return JSVD_REQUEST_NOT_MATCH;
}

// JobManagerから操作するI/Fを定義.
JSVE_Request_Result JSVC_RequestTimer::startAction(JSVS_NotifyInfo* pNotify)
{
	JSVE_Request_Result e_Ret = JSVE_Request_Result_Complete;		// 制御構成変更_No.28_節電 2014/03/17 m.ooba.

	switch( this->getMsgTypeId() )
	{
	case JSVE_JobManager_MsgID_FileCheckTimer_Expired:
	case JSVE_JobManager_MsgID_PowerChanged:		// 電源ステータス変化通知. 制御構成変更_No.28_節電 2014/03/17 m.ooba
		{
// 制御構成変更_No.28_節電 2014/03/17 m.ooba start.
			// 不要ファイル削除処理.
			Bool b_DeleteFlg = False;

			APIC_Power::newInstance()->setPowerSaveDelay( __FILE__, __LINE__ );
			if (True == isPowerSaveStatus()){		// HDDアクセス可能状態判別.
				// HDDアクセス可能な場合のみ不要ファイル削除実施.
				e_Ret = disusedFileDelete();
				b_DeleteFlg = True;			// 削除処理に失敗しても実施済みとみなす.
			}
			APIC_Power::newInstance()->resetPowerSaveDelay( __FILE__, __LINE__ );
			// 不要ファイル削除実施フラグの設定.
			JSVC_JobManager::newInstance()->setDisusedFileDeleteFlg(b_DeleteFlg);
// 制御構成変更_No.28_節電 2014/03/17 m.ooba end.
		}
		break;
	case JSVE_JobManager_MsgID_WarningCheckTimer_Expired:
		{
#if JSVC_Request_ResourceWarningDBG
			printf("★★★★★JSVE_JobManager_MsgID_WarningCheckTimer_Expired\n");
#endif
			//パラメータ取得用の構造体を初期化.
			JSVS_JobResourceWarningNoticeMsg s_ResponseParam;
			JSVC_ResponseJobResourceWarning* pRes = new JSVC_ResponseJobResourceWarning(JSVE_JobManager_MsgID_JobResourceWarningDataChanged);
#if JSVC_Request_ResourceWarningDBG
			MyTraceSD(JSVD_DBG_INFO, "JSVC_RequestTimer::startAction new JSVC_ResponseJobResourceWarning" , (int)pRes );
#endif

			//パラメータの作成.
			if ( NULL != pRes  )
			{
				memset( &s_ResponseParam , 0 , sizeof(s_ResponseParam) );

				APIC_Job* pc_Job = NULL;
				TYPC_JobWarning* pc_JobWarning = NULL;

				pc_Job = getInstancebyResourceId( ul_ParamData );
				if (NULL != pc_Job){
					pc_Job->lock();

					pc_JobWarning = pc_Job->getJobWarning();

					if ( NULL != pc_JobWarning ){

						// メッセージ編集.
						JSVS_SendRecv_MsgParam sParam;
						// Response編集.
						s_ResponseParam.ul_JobResourceID = ul_ParamData;		// ジョブリソースID.
// OP_BTS-3176対応 警告領域枯渇のため領域拡張8(32byte)->16(64byte) 2019/03/07 soma.
						for(short j = 0 ; j < 16 ; j++ ){
							s_ResponseParam.ul_JobWarning[j] = pc_JobWarning->getWarningData(j);
						}
						pRes->setData( (char*)&s_ResponseParam , sizeof(s_ResponseParam) );

						sParam.ul_MsgTypeID = JSVE_JobManager_MsgID_JobResourceWarningDataChanged;
						sParam.pReq = NULL;
						sParam.pRes = (JSVC_ResponseBase*)pRes;

						// メッセージ送信
						JSVC_JobSendRecv::newInstance()->sendMessageToAPF(&sParam);
#if JSVC_Request_ResourceWarningDBG
						printf("★★★★★sendMessageToAPF(JSVE_JobManager_MsgID_JobResourceWarningDataChanged)ResouceID[%d]\n", i);
#endif
					}else{
						// pc_JobWarningがNULLの場合、何もしない.
					}
					pc_Job->unlock();
				}else{
					// pc_JobがNULLの場合、何もしない.
				}
//Str【 静的解析対応 : Sky亀井 : 2012.08.29 : CID-440837 】.
				delete pRes;
//End【 静的解析対応 : Sky亀井 : 2012.08.29 : CID-440837 】.
			}
			else{
				MyTraceSS(JSVD_DBG_ERROR, "JSVC_RequestTimer::startAction","NG");
			}

		}
		break;
	default:
		MyTraceSD( JSVD_DBG_ERROR, "JSVC_RequestTimer::startAction unknown msgtype:" , (int)this->getMsgTypeId() );	// warning対応 2013/09/18 ooba.
		break ;
	}

// 制御構成変更_No.28_節電 2014/03/17 m.ooba start.
//	return JSVE_Request_Result_Complete;
	return e_Ret;
// 制御構成変更_No.28_節電 2014/03/17 m.ooba end.
}

// 制御構成変更_No.28_節電 2014/03/17 m.ooba start.
Bool JSVC_RequestTimer::isPowerSaveStatus()
{
	Bool b_Ret = False;
	TYPE_PowerSaveStatus e_CurrentPower = APIC_Power::newInstance()->getPowerSaveStatus();

	if(( e_CurrentPower == TYPE_PowerSaveStatusIdle								) ||
	   ( e_CurrentPower == TYPE_PowerSaveStatusLowPowerDelay					) ||
	   ( e_CurrentPower == TYPE_PowerSaveStatusPrtEngOnlyLowPowerCancel			) ||
	   ( e_CurrentPower == TYPE_PowerSaveStatusPrtEngOnlyLowPowerCancelPanelOn	) ||
	   ( e_CurrentPower == TYPE_PowerSaveStatusSleepDelay						) ||
	   ( e_CurrentPower == TYPE_PowerSaveStatusPrtEngOnlySleepCancel			) ||
	   ( e_CurrentPower == TYPE_PowerSaveStatusPrtEngOnlySleepCancelPanelOn		) ||
	   ( e_CurrentPower == TYPE_PowerSaveStatusSubPowerOffDelay					) ){
		// HDDアクセス可能状態.
		b_Ret = True;
	}
	return b_Ret;
}

JSVE_Request_Result JSVC_RequestTimer::disusedFileDelete()
{
	//タイムアウトの処理は時間切れファイルの削除のみのため、.
	//特にMSGIDのチェック等は行わずに処理を実施する.
	//（定期的に不要ファイルを削除するか判定するだけなので弊害は無い）.
	
	JSVC_JobManager* pMan = JSVC_JobManager::newInstance();
	//何らかのジョブが実行中か判定（実行中は処理しない）.
	if(( pMan == NULL )
	|| ( pMan->getRequestListByMsg( JSVE_JobManager_MsgID_Start )   != NULL )
	|| ( pMan->getRequestListByMsg( JSVE_JobManager_MsgID_ReStart ) != NULL ))
	{
		//特に送信しないが、デバッグの利便性を考慮して結果を格納.
		setResult( JSVE_JobServiceResult_NG );
		setResultDetail( IPCE_JobsResultDetail_Exec );
		return JSVE_Request_Result_Err;
	}
	
	uint32_t ul_result = JOBE_ApiResult_OK;// Jobのファイル操作APIコール時の結果.
	int32_t sl_FileID = 0;					// ファイルID.
	LIBC_List<int32_t> c_ExistIdList;		// 管理中のファイルリスト（ポインタをセットせずにIDをキャストして保持させる） .
	LIBC_List<int32_t> c_DelIdList;			// 削除対象ファイルリスト（ポインタをセットせずにIDをキャストして保持させる） .
	LIBC_List<int32_t> c_UsingIdList;		// 使用中のファイルリスト（ポインタをセットせずにIDをキャストして保持させる） .
	
	// 管理中のフラットファイルのFlatFileIDリストを取得.
	ul_result = getFlatFileIDList(&c_ExistIdList);
	if (JOBE_ApiResult_OK != ul_result) {
		return JSVE_Request_Result_Err;
	}
	
	// 削除対象となるファイルを検索する.
	// FlatFileIDが0の場合を逃さないために、最初のIDは必ず見るようにする.
	// 但しgetCount()が0でない場合に限る.
	sl_FileID = (int32_t)(intptr_t)c_ExistIdList.findFirst();
	if (0 != c_ExistIdList.getCount()) {
		FILC_FlatFileIf* pc_BaseDir = FILF_getFlatFileInstance(FILE_DirTypeFlatFile);
		//Str【 静的解析対応 : Sky花岡 : 2012.09.03 : CID-439623】.
		if (NULL == pc_BaseDir) {
			return JSVE_Request_Result_Err;
		}
		//End【 静的解析対応 : Sky花岡 : 2012.09.03 : CID-439623】.
		do {
			struct stat st_stat;
			char sc_FilePath[JOBD_FlatFileIDListPath_Max+1];
			memset(sc_FilePath, 0x00, sizeof(sc_FilePath));
	
			// FlatFileIDよりフルパスを取得.
			pc_BaseDir->IDtoPath(sc_FilePath, sl_FileID, PathTypeFile);
			// フラットファイル情報を取得.
			if (ERROR == FILF_Stat(sc_FilePath, &st_stat)) {
				// 特に送信しないが、デバッグの利便性を考慮して結果を格納.
				setResult(JSVE_JobServiceResult_NG);
				setResultDetail(IPCE_JobsResultDetail_FileSystemError);
				break;
			}
	
			// 期限切れかどうかチェック.
			time_t	nowTime;
			nowTime = time(NULL);
			time_t createTime = st_stat.st_ctime;					//年→月.
	
			// ファイル作成から２４H経過していれば削除対象のリストへ追加.
			if (nowTime > (createTime + JSVE_DELETE_LIMIT_SECOND)) {
				c_DelIdList.add((int32_t*)(intptr_t)sl_FileID);
			}
			sl_FileID = (int32_t)(intptr_t)c_ExistIdList.findNext();
		} while (sl_FileID);
	}
	
	// 実行中ジョブで使用中のファイルを検索.
	// 該当フォルダ単位で削除不可リストへ追加していく.
	short ss_index = 0;
	for (ss_index = 0; ss_index < JSVD_JOBP_MAX; ss_index++) {
		JSVC_ParentRequest* pPreq = JSVC_JobManager::newInstance()->getParentRequestByIndex(ss_index);
		if (pPreq == NULL) {
			break;
		}
		TYPC_Common* pCommon = pPreq->getJobInstance()->getJobSpec()->pc_Common;
		if (TYPD_NotSetting != pCommon->getSimpleLogFlatFileID()) {
			c_UsingIdList.add((int32_t*)(intptr_t)pCommon->getSimpleLogFlatFileID());
		}
		if (TYPD_NotSetting != pCommon->getControlFileID()) {
			c_UsingIdList.add((int32_t*)(intptr_t)pCommon->getControlFileID());
		}
		if (TYPD_NotSetting != pCommon->getTerminationFileID()) {
			c_UsingIdList.add((int32_t*)(intptr_t)pCommon->getTerminationFileID());
		}
	}
	
	// 削除対象の抽出.
	// 削除対象リストから削除不可リストのファイルを削除.
	intptr_t sl_UseFileIndex = (intptr_t)c_UsingIdList.findFirst();
	intptr_t sl_DelFileIndex = (intptr_t)c_DelIdList.findFirst();
	if (0 != c_UsingIdList.getCount()) {
		do {
			// 確認対象がなくなっていれば処理しない.
			if (0 == c_DelIdList.getCount()) {
				break;
			} else {
				do {
					// 使用中ファイルに含まれているか？.
					if (sl_UseFileIndex == sl_DelFileIndex ) {
						// 削除対象リストから削除.
						c_DelIdList.del((int32_t*)sl_DelFileIndex);
						break;
					}
					sl_DelFileIndex = (intptr_t)c_DelIdList.findNext();
				} while (sl_DelFileIndex);
			}
			sl_UseFileIndex = (intptr_t)c_UsingIdList.findNext();
		} while (sl_UseFileIndex);
	}
	
	// 削除対象でないファイルIDのリストを作成.
	sl_FileID = (int32_t)(intptr_t)c_DelIdList.findFirst();
	if (0 != c_DelIdList.getCount()) {
		do {
			c_ExistIdList.del((int32_t*)(intptr_t)sl_FileID);
			sl_FileID = (int32_t)(intptr_t)c_DelIdList.findNext();
		} while (sl_FileID);
	}
	
	// 不要な一時ファイルを削除する.
	ul_result = deleteUnnecessaryFlatFile(&c_ExistIdList);
	if (JOBE_ApiResult_OK != ul_result) {
		MyTraceSD( JSVD_DBG_ERROR, "JSVC_RequestTimer::startAction deleteUnnecessaryFlatFile is NG ret:", (int)ul_result ) ;	// warning対応 2013/09/18 ooba.
		return JSVE_Request_Result_Err;
	}
	// 一時ファイルの削除が正常に完了したら、紐づく管理ファイルの削除を行う.
	ul_result = deleteUnnecessaryManagerFile(&c_DelIdList);
	if (JOBE_ApiResult_OK != ul_result) {
		MyTraceSD( JSVD_DBG_ERROR, "JSVC_RequestTimer::startAction deleteUnnecessaryManagerFile is NG ret:", (int)ul_result ) ;	// warning対応 2013/09/18 ooba.
		return JSVE_Request_Result_Err;
	}
	
	// リスト内容を明示的に削除.
	c_ExistIdList.delAll();
	c_DelIdList.delAll();
	c_UsingIdList.delAll();
	
	return JSVE_Request_Result_Complete;
}
// 制御構成変更_No.28_節電 2014/03/17 m.ooba end.

//--------------------------------------------------------------------------------------------------.
//	JSVC_ResponseVariousFileDataクラス定義.
//--------------------------------------------------------------------------------------------------.
// コンストラクタ.
JSVC_RequestSetJobStatus::JSVC_RequestSetJobStatus(uint32_t ul_RequestID,uint32_t ul_MsgTypeID)
:	JSVC_Request( ul_RequestID,ul_MsgTypeID ), ul_SubScenarioID(0),sl_Size(0)
{
	memset( &st_JobStatusInfo , 0 , sizeof( IPCS_JobStatusInfo ) );
}
// デストラクタ
JSVC_RequestSetJobStatus::~JSVC_RequestSetJobStatus()
{
}
// 自身の型を示す.
JSVE_Request_Class_Type JSVC_RequestSetJobStatus::WhoAmI()
{
	return JSVE_Request_Class_Type_Res_SetJobStatus;
}
// JobManagerから操作するI/Fを定義.
JSVE_Request_Result JSVC_RequestSetJobStatus::startAction(JSVS_NotifyInfo* pNotify)
{
	JSVE_Request_Result e_Request_Result = JSVE_Request_Result_Complete;
	uint32_t ul_ResultDetail = IPCE_JobsResultDetail_Non;

	// 親リクエストを取得.
	uint32_t ulID = getSubScenarioID();
	JSVC_ParentRequest* pc_PRequest = NULL;
	APIC_Job* pc_Job = NULL;
	TYPC_JobStatus* pJobStatus = NULL;

	//値の設定処理.
	do {
		//親リクエストの検索.
		pc_PRequest = JSVC_JobManager::newInstance()->getParentRequest(ulID);
		if ( NULL == pc_PRequest ) {
			MyTraceSD(JSVD_DBG_ERROR, "JSVC_RequestSetJobStatus::startAction getParentRequest ERR  SubScen=" , (int)ulID );
			e_Request_Result = JSVE_Request_Result_Err;
			ul_ResultDetail = IPCE_JobsResultDetail_NotMemAlloc;
			break;
		}

		//JosStatusを取得.
		pc_Job = pc_PRequest->getJobInstance();
		if( NULL != pc_Job ){
			pJobStatus = pc_Job->getJobStatus();
			if( NULL == pJobStatus ){
				MyTraceSD(JSVD_DBG_ERROR, "JSVC_RequestSetJobStatus::startAction getJobStatus ERR  SubScen=" , (int)ulID );
				e_Request_Result = JSVE_Request_Result_Err;
				ul_ResultDetail = IPCE_JobsResultDetail_JobPointerNull;
				break;
			}
		} else {
			MyTraceSD(JSVD_DBG_ERROR, "JSVC_RequestSetJobStatus::startAction getJobInstance ERR  SubScen=" , (int)ulID );
			e_Request_Result = JSVE_Request_Result_Err;
			ul_ResultDetail = IPCE_JobsResultDetail_JobPointerNull;
			break;
		}


		//値の設定.
		// 同一MSGに対する設定処理を追加する場合は以下にcaseを追加してください.
		switch(st_JobStatusInfo.e_Kind){
			case IPCE_JobStatusKind_UserDeleteJobCause:						//	e_UserDeleteJobCause		ユーザー破棄の結果詳細.
				pJobStatus->setUserDeleteJobCause( (const TYPE_DeleteJobCause)st_JobStatusInfo.un.ss_Data );
				break;
			case IPCE_JobStatusKind_ConfirmPrintStopState:					//	setConfirmPrintStopState()	.
				pJobStatus->setConfirmPrintStopState( (Bool)st_JobStatusInfo.un.ss_Data );
				break;
			case IPCE_JobStatusKind_DuplexPrint:							//	setDuplexPrint()			.
				pJobStatus->setDuplexPrint( (Bool)st_JobStatusInfo.un.ss_Data );
				break;
			case IPCE_JobStatusKind_NumOfCopies:							//	setNumOfCopies()			.
				pJobStatus->setNumOfCopies( (int32_t)st_JobStatusInfo.un.sl_Data );
				break;
			case IPCE_JobStatusKind_NumOfOriginals:							//	setNumOfOriginals()			.
				pJobStatus->setNumOfOriginals( (unsigned short)st_JobStatusInfo.un.ss_Data );
				break;
			case IPCE_JobStatusKind_Restoring:								//	setRestoring()				.
				pJobStatus->setRestoring( (Bool)st_JobStatusInfo.un.ss_Data );
				break;
			case IPCE_JobStatusKind_TransactionID:							//	setTransactionID()			.
				pJobStatus->setTransactionID( (uint32_t)st_JobStatusInfo.un.sl_Data );
				break;
			// 無効値に対してはNotSUpportを返す.
			case IPCE_JobStatusKind_Non:
			default:
				MyTraceSD(JSVD_DBG_ERROR, "JSVC_RequestSetJobStatus::startAction JobStatusKind ERR  kind=" , (int)st_JobStatusInfo.e_Kind );
				ul_ResultDetail = IPCE_JobsResultDetail_NotSupport;
				break;
		}
	} while(0);

	//レスポンスクラスへ結果を設定.
	JSVC_ResponseSetJobStatus c_res(JSVE_JobManager_MsgID_SetJobStatusAccept);
	c_res.setData( (const char*)&ulID , sizeof(uint32_t) );		//セットが必要なデータはuint32_tひとつのため.
	if( JSVE_Request_Result_Err != e_Request_Result ){
		c_res.setResult(JSVE_JobServiceResult_OK);
		c_res.setResultDetail(IPCE_JobsResultDetail_Non);
		pc_Job->req_checkJobStatusChange();						//ジョブへ変更トリガを通知.
	}else{
		c_res.setResult(JSVE_JobServiceResult_NG);
		c_res.setResultDetail(ul_ResultDetail);
	}

	//結果の送信.
	JSVS_SendRecv_MsgParam sParam;
	sParam.ul_MsgTypeID = JSVE_JobManager_MsgID_SetJobStatusAccept;
	sParam.pReq = this;
	sParam.pRes = &c_res;
	JSVC_JobSendRecv::newInstance()->sendMessageToAPF(&sParam);


	return e_Request_Result;
}
// リクエスト受付時のデータ保存処理.
Bool JSVC_RequestSetJobStatus::setData(const char* pRcvData,short ss_size)
{
	JSVS_ReqPara_SetJobStatus *pParam = (JSVS_ReqPara_SetJobStatus*)pRcvData;

	if( NULL == pParam )
	{
		MyTraceSS(JSVD_DBG_ERROR, "JSVC_RequestSetJobStatus::setData","入力NULL" );
		return False;
	}
	ul_SubScenarioID = pParam->ul_SubScenarioID;
	//先に初期化.
	memset( &st_JobStatusInfo , 0 ,sizeof(IPCS_JobStatusInfo) );
	//必要分をコピー.
	uint32_t ul_copySize = static_cast<uint32_t>(( sizeof(IPCS_JobStatusInfo) > (uint32_t)pParam->sl_size ) ? pParam->sl_size : sizeof(IPCS_JobStatusInfo)); // warning対応 2013/09/21 ooba.
	memcpy( &st_JobStatusInfo , &pParam->st_JobStatusInfo ,ul_copySize );

	return True;
}
// 指定した応答に関連するリクエストかどうかを判定.
short JSVC_RequestSetJobStatus::isResponseMsg(JSVC_Response*)
{
	//同期なのでJSVD_REQUEST_NOT_MATCHを固定で返しておく.
	return JSVD_REQUEST_NOT_MATCH;
}

JSVC_RequestListOperation::JSVC_RequestListOperation(uint32_t ul_RequestID,uint32_t ul_MsgTypeID)
:	JSVC_Request( ul_RequestID,ul_MsgTypeID ), e_Type(0), e_Kind(0), ul_Start(0), ul_Num(0)
{
	ul_SubScenarioID[0] = 0 ;
	sc_AppType = 0;			// CQ514349/CQ514406対応 2015/7/16  m.ooba.
	e_NetworkSpecific = TYPE_NetworkSpecific_ALL;		//2017/09/28 RQ-686 複合機の複数ネットワーク対応 doi add.
}
JSVC_RequestListOperation::~JSVC_RequestListOperation()
{
}
JSVE_Request_Class_Type JSVC_RequestListOperation::WhoAmI()
{
	return JSVE_Request_Class_Type_Req_GetActiveSubScenarioList ;
}

#define JSVD_DeleteExecDelay	10	// 2015/7/6 CQ515758 ジョブリソース解放要求時のジョブリソース解放前にDelayをかける ht_agotou.

JSVE_Request_Result JSVC_RequestListOperation::startAction(JSVS_NotifyInfo* pNotify)
{
	JSVE_Request_Result e_Request_Result = JSVE_Request_Result_Complete;

	switch( getMsgTypeId() ){
		case JSVE_JobManager_MsgID_GetActiveSubScenarioList:
			{
				short ss_ret = 0 ;

				//値の設定処理.
				do {
					LIBC_List<APIC_Job> c_List;

					switch( this->e_Type ) {
					case IPCE_JobListType_List:
						ss_ret = APIC_ActiveJobList::newInstance()->getList( static_cast<TYPE_ActiveJobList>(e_Kind), static_cast<short>(ul_Start),
																			 static_cast<short>(ul_Num), &c_List, e_NetworkSpecific );	//2017/09/28 RQ-686 複合機の複数ネットワーク対応 doi change.
						break ;
					case IPCE_JobListType_ListPanel:
						ss_ret = APIC_ActiveJobList::newInstance()->getListPanel( static_cast<TYPE_ActiveJobList>(e_Kind), static_cast<short>(ul_Start),
																				  static_cast<short>(ul_Num), &c_List, e_NetworkSpecific );	//2017/09/28 RQ-686 複合機の複数ネットワーク対応 doi change.
						break ;
					default:
						break ;
					}

					APIC_Job *pc_Job_Sh = c_List.findFirst();
					uint32_t ul_Index = 0 ;
					while( pc_Job_Sh != NULL ) {
						// 現状固定で取ることにする
						ul_SubScenarioID[ul_Index] = pc_Job_Sh->getJobStatus()->getSubScenarioID() ;
						ul_Index++ ;
						pc_Job_Sh = c_List.findNext();
					}
					this->ul_Num = ul_Index;
				} while(0) ;

				//レスポンスクラスへ結果を設定.
				JSVC_ResponseListOperation* pc_res = new JSVC_ResponseListOperation(JSVE_JobManager_MsgID_GetActiveSubScenarioListAccept);
				short ss_size = static_cast<short>(sizeof(JSVS_GetActiveSubScenarioListAnsMsg)+sizeof(uint32_t)*this->ul_Num);
				//応答用データ用に一時保存バッファを確保.
				JSVS_GetActiveSubScenarioListAnsMsg* ps_Res = NULL;
				ps_Res = (JSVS_GetActiveSubScenarioListAnsMsg*)malloc(ss_size);
				if( NULL != ps_Res ){
					memset( ps_Res, 0, ss_size );
					ps_Res->ul_Num = this->ul_Num;
					if (this->ul_Num) {
						memcpy((void*)&(ps_Res->ul_SubScenarioID[0]), (void*)&ul_SubScenarioID[0], this->ul_Num*sizeof(uint32_t));
					}
					pc_res->setData( (const char*)ps_Res , ss_size, ss_ret );																// 2012.02.20 sk_takas for IWS-CIT No.226
					free(ps_Res);
					pc_res->setResult(JSVE_JobServiceResult_OK);
					pc_res->setResultDetail(IPCE_JobsResultDetail_Non);
				} else {
					pc_res->setResult(JSVE_JobServiceResult_NG);
					pc_res->setResultDetail(IPCE_JobsResultDetail_MemAllocFailed);
					MyTraceSLD(JSVD_DBG_ERROR, "JSVC_RequestListOperation::startAction ERR Can't alloc memory" , (intptr_t)ps_Res );
				}

				//結果の送信.
				JSVS_SendRecv_MsgParam sParam;
				sParam.ul_MsgTypeID = JSVE_JobManager_MsgID_GetActiveSubScenarioListAccept;
				sParam.pReq = (JSVC_RequestBase*)this;
				sParam.pRes = (JSVC_ResponseBase*)pc_res;
				JSVC_JobSendRecv::newInstance()->sendMessageToAPF(&sParam);
				delete pc_res;
			}
			break;
		case JSVE_JobManager_MsgID_GetJobResource:
			{
				JSVC_ResponseListOperation* pRes = new JSVC_ResponseListOperation(JSVE_JobManager_MsgID_GetJobResourceAccept);
				//最大サイズを見越してヒープを確保.
				short ss_BufferSize = sizeof(JSVS_ListOperationAnsMsg)+sizeof(uint32_t)*JSVC_Request_JobResourceMax;
				JSVS_ListOperationAnsMsg* pAnsMsg = (JSVS_ListOperationAnsMsg*)malloc( ss_BufferSize );
//Str【 静的解析対応 : Sky田村 : 2012.09.03 : CID-439629 】.
				if( pAnsMsg == NULL )
				{
					MyTraceSD(JSVD_DBG_ERROR, "JSVC_RequestListOperation::startAction malloc ERROR pAnsMsg " , ss_BufferSize );
					pRes->setResult(JSVE_JobServiceResult_NG);
					pRes->setResultDetail(IPCE_JobsResultDetail_MemAllocFailed);
				}
				else
				{
//Str【 静的解析対応 : Sky伊藤（勝） : 2012.12.03 : CID-454790】.
					//要求数はul_Numに入っているので、ここで.
					//カウントを設定.
					pAnsMsg->ul_Num = ul_Num;

					//一旦、成功をセットしておく.
					pRes->setResult(JSVE_JobServiceResult_OK);
					pRes->setResultDetail(IPCE_JobsResultDetail_Non);

					//ジョブからインスタンスを確保し、ジョブリソースIDをヒープへ入れる.
					if( pAnsMsg->ul_Num ){
						uint32_t ul_Loop = 0;
						uint32_t ul_JobResourceID = 0;
						for( ul_Loop = 0 ; ul_Loop < pAnsMsg->ul_Num ; ul_Loop++ ){
							
							//ジョブタイプを指定してジョブリソースを確保.
							//（ジョブタイプはul_SubScenarioID[ss_Loop]に入っている）.
// CQ514349/CQ514406対応 2015/7/16  m.ooba start.
							APIC_Job* pJob = APIC_Job::newInstance( static_cast<TYPE_JobType>(ul_SubScenarioID[ul_Loop]), 0, sc_AppType );
							//APIC_Job* pJob = APIC_Job::newInstance( ul_SubScenarioID[ul_Loop] );
// CQ514349/CQ514406対応 2015/7/16  m.ooba end.
							if( pJob ){
								
								ul_JobResourceID = pJob->getJobResourceID();
								
								//確保したジョブリソースのIDを結果にセット.
								pAnsMsg->ul_ListData[ul_Loop] = ul_JobResourceID;
							} else {
								pRes->setResult(JSVE_JobServiceResult_NG);
								pRes->setResultDetail(IPCE_JobsResultDetail_JobPointerNull);
								break;
							}
						}
					}
					
					//レスポンスクラスへ値を設定.
					pRes->setData( (char*)pAnsMsg ,ss_BufferSize );

					//ヒープを解放.
					free( pAnsMsg );
					
					//要求元へ結果を通知.
					JSVS_SendRecv_MsgParam sParam;
					sParam.ul_MsgTypeID = JSVE_JobManager_MsgID_GetJobResourceAccept;
					sParam.pReq = (JSVC_RequestBase*)this;
					sParam.pRes = (JSVC_ResponseBase*)pRes;
					JSVC_JobSendRecv::newInstance()->sendMessageToAPF(&sParam);
				}
				delete pRes;
//End【 静的解析対応 : Sky伊藤（勝） : 2012.12.03 : CID-454790】.
//End【 静的解析対応 : Sky田村 : 2012.09.03 : CID-439629 】.
			}
			break;
// CQ526787 ZeusS_ZX0 MR問題恒久対応 2016/4/4 m.ooba start.
		case JSVE_JobManager_MsgID_ReleaseJobResource:
			{
				// MFP側からの要求の場合はdeleteInstanceさせない.BlackAPI側の場合はJSVE_JobManager_MsgID_ReleaseJobResource2のcase文に入る.
				JSVC_ResponseListOperation* pRes = new JSVC_ResponseListOperation(JSVE_JobManager_MsgID_ReleaseJobResourceAccept);

				//レスポンスクラスへ値を設定.
				pRes->setResult(JSVE_JobServiceResult_OK);
				pRes->setResultDetail(IPCE_JobsResultDetail_Non);

				//要求元へ結果を通知.
				JSVS_SendRecv_MsgParam sParam;
				sParam.ul_MsgTypeID = JSVE_JobManager_MsgID_ReleaseJobResourceAccept;
				sParam.pReq = (JSVC_RequestBase*)this;
				sParam.pRes = (JSVC_ResponseBase*)pRes;
				JSVC_JobSendRecv::newInstance()->sendMessageToAPF(&sParam);

				delete pRes;

			break;
			}
		case JSVE_JobManager_MsgID_ReleaseJobResource2:
// CQ526787 ZeusS_ZX0 MR問題恒久対応 2016/4/4 m.ooba end.
			{
				//要求数はul_Numに入っている.
				JSVC_ResponseListOperation* pRes = new JSVC_ResponseListOperation(JSVE_JobManager_MsgID_ReleaseJobResource2Accept);

				//対象ジョブリソースIDはul_SubScenarioID[ss_Loop]に入っている。.
				if( ul_Num ){
					uint32_t ul_Loop = 0;
					for( ul_Loop = 0 ; ul_Loop < ul_Num ; ul_Loop++ ){
						
						//ジョブリソースIDを指定して解放処理を実施.
						//（ジョブリソースIDはul_SubScenarioID[ss_Loop]に入っている）.
						APIC_Job* pJob = getInstancebyResourceId(ul_SubScenarioID[ul_Loop]);
						if( pJob){
							SYSF_taskDelay( JSVD_DeleteExecDelay );	// 2015/7/6 CQ515758 ジョブリソース解放要求時のジョブリソース解放前にDelayをかける ht_agotou.

							// CQ370161対策 Start.
							//pJob->deleteInstance();
							LIBC_List<APIC_Job> c_MyList;
							APIC_Job* pc_myJob = NULL;
							Bool b_deleteExec = True;
							APIC_ActiveJobList::newInstance()->getList(TYPE_ActiveJobListAll, 1, 350, &c_MyList);
							for (pc_myJob = c_MyList.findFirst(); pc_myJob; pc_myJob = c_MyList.findNext()){
								if( pc_myJob == pJob ){
									b_deleteExec = False;
								}
							}

							// 2012/09/11 CQ367774対応 add start.
							// サブシナリオIDが設定されていない.
							// DeleteInstanceを行う.
							if (True == b_deleteExec) {
								if (0 != pJob->getJobStatus()->getSubScenarioID()) {
									if ((TYPE_req_start_Result_NORMAL	== pJob->get_req_start_result()) &&
										(TYPE_req_start_Result_RETRY	== pJob->get_req_start_result()) &&
										(TYPE_req_start_Result_NONRETRY	== pJob->get_req_start_result()))
									{
										b_deleteExec = False;
									} else {
										// サブシナリオIDが設定されているが, ans_start(NULL)の場合DeleteInstanceを行う.
									}
								} else {
									// サブシナリオIDが設定されていない場合DeleteInstanceを行う.
									// (Panelが確保しているInstanceのResourceIDと復旧シナリオが覚えているResourceIDが被る事があり.
									//  その場合はここではガード出来ずdeleteしてしまう。
									//  上記ケースについてはServiceManagerにてガードする必要がある).
								}
							}
							// 2012/09/11 CQ367774対応 add end.

// 2016/1/13 CQ523488 復旧JobでSegFし、リブート後Fax送信できない問題を修正 ht_agotou Start.
							if( (True == b_deleteExec)
							 && (False == pJob->getWaitRevivalJob()) ) {	// 復旧Jobの場合はここでdeleteInstanceさせない.
								pJob->deleteInstance();
							}
// 2016/1/13 CQ523488 復旧JobでSegFし、リブート後Fax送信できない問題を修正 ht_agotou End.
							// CQ370161対策 End.
						} else {
							//取れなくても結果的になくなればいいのでエラーにしない.
						}
					}
				}

				//レスポンスクラスへ値を設定.
				pRes->setResult(JSVE_JobServiceResult_OK);
				pRes->setResultDetail(IPCE_JobsResultDetail_Non);

				//要求元へ結果を通知.
				JSVS_SendRecv_MsgParam sParam;
				sParam.ul_MsgTypeID = JSVE_JobManager_MsgID_ReleaseJobResource2Accept;
				sParam.pReq = (JSVC_RequestBase*)this;
				sParam.pRes = (JSVC_ResponseBase*)pRes;
				JSVC_JobSendRecv::newInstance()->sendMessageToAPF(&sParam);

				delete pRes;
			}
			break;
		case JSVE_JobManager_MsgID_SetScenarioID:
			{
				JSVC_ResponseListOperation* pRes = new JSVC_ResponseListOperation(JSVE_JobManager_MsgID_SetScenarioIDAccept);
				//最大サイズを見越してヒープを確保.
				short ss_BufferSize = sizeof(JSVS_SetScenarioIDAnsMsg)+sizeof(uint32_t)*JSVC_Request_JobResourceMax;
				JSVS_SetScenarioIDAnsMsg* pAnsMsg = (JSVS_SetScenarioIDAnsMsg*)malloc( ss_BufferSize );
//Str【 静的解析対応 : Sky陰山 : 2012.10.29 : CID-439629 】.
				if( pAnsMsg == NULL )
				{
					MyTraceSD(JSVD_DBG_ERROR, "JSVC_RequestListOperation::startAction malloc ERROR pAnsMsg " , ss_BufferSize );
				}
				else
				{
//End【 静的解析対応 : Sky陰山 : 2012.10.29 : CID-439629 】.

				//要求数はul_Numに入っているので、ここで.
				//カウントを設定.
				pAnsMsg->ul_Num = ul_Num;

				//一旦、成功をセットしておく.
				pRes->setResult(JSVE_JobServiceResult_OK);
				pRes->setResultDetail(IPCE_JobsResultDetail_Non);

				//ジョブにシナリオID,サブシナリオIDを割り当ててもらい、シナリオID,サブシナリオIDをヒープへ入れる.
				if( pAnsMsg->ul_Num ){
					uint32_t ul_Loop = 0;
					for( ul_Loop = 0 ; ul_Loop < pAnsMsg->ul_Num ; ul_Loop++ ){
						// シナリオID割り当て,
						// リソースIDはul_SubScenarioID[ss_Loop]に入っている.
						if (0 == ul_Loop){
							pAnsMsg->ul_ScenarioID = JOBG_assignScenarioID(ul_SubScenarioID[ul_Loop]);
							MyTraceSD( JSVD_DBG_ERROR, "SetScenarioID ScenarioID:" , (int)pAnsMsg->ul_ScenarioID ) ;	// warning対応 2013/09/18 ooba.
						}
						//確保したジョブリソースのIDを結果にセット.
						pAnsMsg->ul_SubScenarioID[ul_Loop] = JOBG_assignSubScenarioID(ul_SubScenarioID[ul_Loop]);
						MyTraceSD( JSVD_DBG_ERROR, "SetScenarioID SubScenarioID:" , (int)pAnsMsg->ul_SubScenarioID[ul_Loop] ) ;	// warning対応 2013/09/18 ooba.
					}

					// ベーシック版対応.
					// シナリオIDをジョブIDで上書きする.
					// ワークフローの機能搭載時には不具合となるため、対応の必要がある.
					// この特別対応のため、上記のJOBG_assignScenarioIDは不要となっているが、
					// 機能拡張版では必要な処理となるため、残しておき0番目のデータをシナリオIDに.
					// 強制的に更新するものとする.
					if ( pAnsMsg->ul_ScenarioID != pAnsMsg->ul_SubScenarioID[0]){
						pAnsMsg->ul_ScenarioID = pAnsMsg->ul_SubScenarioID[0];
					}
					// シナリオIDが有効値でない場合、エラーとする.
					if( 1 > pAnsMsg->ul_ScenarioID ){
						pRes->setResult(JSVE_JobServiceResult_NG);
						pRes->setResultDetail(IPCE_JobsResultDetail_JobPointerNull);
					}
				}

				//レスポンスクラスへ値を設定.
				pRes->setData( (char*)pAnsMsg ,ss_BufferSize );

				//ヒープを解放.
				free( pAnsMsg );
				
				//要求元へ結果を通知.
				JSVS_SendRecv_MsgParam sParam;
				sParam.ul_MsgTypeID = JSVE_JobManager_MsgID_SetScenarioIDAccept;
				sParam.pReq = (JSVC_RequestBase*)this;
				sParam.pRes = (JSVC_ResponseBase*)pRes;
				JSVC_JobSendRecv::newInstance()->sendMessageToAPF(&sParam);
				
//Str【 静的解析対応 : Sky陰山 : 2012.10.29 : CID-439629 】.
				}
//End【 静的解析対応 : Sky陰山 : 2012.10.29 : CID-439629 】.
				delete pRes;
			}
			break;
		default:
		break;
	}
	return e_Request_Result ;
}
Bool JSVC_RequestListOperation::setData(const char* pRcvData,short ss_size)
{
	switch( getMsgTypeId() ){
		case JSVE_JobManager_MsgID_GetActiveSubScenarioList:
			{
				JSVS_ReqPara_GetActiveSubScenarioList *pParam = (JSVS_ReqPara_GetActiveSubScenarioList*)pRcvData;

				if( NULL == pParam )
				{
					MyTraceSS(JSVD_DBG_ERROR, "JSVC_RequestGetActiveSubScenarioList::setData","入力NULL" );
					return False;
				}

				// データをセット
				this->e_Type = pParam->e_Type ;
				this->e_Kind = pParam->e_Kind ;
				this->ul_Start = pParam->ul_Start ;
				this->ul_Num = pParam->ul_Num ;
				this->e_NetworkSpecific = pParam->e_NetworkSpecific ;		//2017/09/28 RQ-686 複合機の複数ネットワーク対応 doi add.
			}
			break;
		case JSVE_JobManager_MsgID_GetJobResource:
			{
				JSVS_ReqPara_ByJobType *pParam = (JSVS_ReqPara_ByJobType*)pRcvData;
				ul_Num = pParam->ul_rcount;
				if( ul_Num != 0 ){
					short ss_Loop = 0;
					memset( (void*)&ul_SubScenarioID[0] , 0xFF , sizeof(ul_SubScenarioID) );
					for( ss_Loop = 0 ; ss_Loop < (short)ul_Num ; ss_Loop++ ){
						//配列サイズを越えてのライトは防ぐ.
						if ( sizeof(ul_SubScenarioID)/sizeof(ul_SubScenarioID[0]) <= (uint32_t)ss_Loop ){		// warning対応 2013/09/21 ooba.
							break;
						}
						//確保指定のジョブタイプを入れておく.
						ul_SubScenarioID[ss_Loop] = pParam->ul_JobType[ss_Loop];
					}
// CQ514349/CQ514406対応 2015/7/16  m.ooba start.
					// アプリタイプ保存.
					sc_AppType = pParam->sc_AppType;
// CQ514349/CQ514406対応 2015/7/16  m.ooba end.
				}
			}
			break;
		case JSVE_JobManager_MsgID_ReleaseJobResource:
			{
				JSVS_ReqPara_ByResource *pParam = (JSVS_ReqPara_ByResource*)pRcvData;
				ul_Num = pParam->ul_rcount;
				if( ul_Num != 0 ){
					short ss_Loop = 0;
					memset( (void*)&ul_SubScenarioID[0] , 0xFF , sizeof(ul_SubScenarioID) );
					for( ss_Loop = 0 ; ss_Loop < (short)ul_Num ; ss_Loop++ ){
						//配列サイズを越えてのライトは防ぐ.
						if ( sizeof(ul_SubScenarioID)/sizeof(ul_SubScenarioID[0]) <= (uint32_t)ss_Loop ){		// warning対応 2013/09/21 ooba.
							break;
						}
						//解放対象のジョブリソースIDを入れておく.
						ul_SubScenarioID[ss_Loop] = pParam->ul_JobResourceID[ss_Loop];
					}
				}
			}
			break;
// CQ526787 ZeusS_ZX0 MR問題恒久対応 2016/4/4 m.ooba start.
		case JSVE_JobManager_MsgID_ReleaseJobResource2:
			{
				JSVS_ReqPara_ByResource *pParam = (JSVS_ReqPara_ByResource*)pRcvData;
				ul_Num = pParam->ul_rcount;
				if( ul_Num != 0 ){
					short ss_Loop = 0;
					memset( (void*)&ul_SubScenarioID[0] , 0xFF , sizeof(ul_SubScenarioID) );
					for( ss_Loop = 0 ; ss_Loop < (short)ul_Num ; ss_Loop++ ){
						//配列サイズを越えてのライトは防ぐ.
						if ( sizeof(ul_SubScenarioID)/sizeof(ul_SubScenarioID[0]) <= (size_t)ss_Loop ){
							break;
						}
						//解放対象のジョブリソースIDを入れておく.
						ul_SubScenarioID[ss_Loop] = pParam->ul_JobResourceID[ss_Loop];
					}
				}
			}
			break;
// CQ526787 ZeusS_ZX0 MR問題恒久対応 2016/4/4 m.ooba end.
		case JSVE_JobManager_MsgID_SetScenarioID:
			{
				JSVS_ReqPara_ByResource *pParam = (JSVS_ReqPara_ByResource*)pRcvData;
				ul_Num = pParam->ul_rcount;
				if( ul_Num != 0 ){
					short ss_Loop = 0;
					memset( (void*)&ul_SubScenarioID[0] , 0xFF , sizeof(ul_SubScenarioID) );
					for( ss_Loop = 0 ; ss_Loop < (short)ul_Num ; ss_Loop++ ){
						//配列サイズを越えてのライトは防ぐ.
						if ( sizeof(ul_SubScenarioID)/sizeof(ul_SubScenarioID[0]) <= (uint32_t)ss_Loop ){		// warning対応 2013/09/21 ooba.
							break;
						}
						//解放対象のジョブリソースIDを入れておく.
						ul_SubScenarioID[ss_Loop] = pParam->ul_JobResourceID[ss_Loop];
					}
				}
			}
			break;
		default:
			return False;
	}

	return True;
}
short JSVC_RequestListOperation::isResponseMsg(JSVC_Response* pResponse)
{
	//同期なのでJSVD_REQUEST_NOT_MATCHを固定で返しておく.
	return JSVD_REQUEST_NOT_MATCH;
}

//--------------------------------------------------------------------------------------------------.
//	JSVC_RequestOneShotクラス定義.
//--------------------------------------------------------------------------------------------------.
// コンストラクタ.
JSVC_RequestOneShot::JSVC_RequestOneShot(uint32_t ul_RequestID,uint32_t ul_MsgTypeID)
:	JSVC_RequestBase(ul_RequestID,ul_MsgTypeID), ul_Param(0)
, ul_SubScenarioID(0), ul_Data(0), ul_SndHistNum(0), ul_AddressNum(0)
{
}
// 自クラスの型を返す.
JSVE_Request_Class_Type JSVC_RequestOneShot::WhoAmI()
{
	return JSVE_Request_Class_Type_OneShot;
}
// リクエスト受付時のデータ保存処理.
Bool JSVC_RequestOneShot::setData(const char* pRcvData,short ss_size)
{
#ifdef _DEBUG_JSVC_SEQ
	MyTraceSLD(JSVD_DBG_INFO, "JSVC_RequestOneShot::setData pRcvData:" , (intptr_t)pRcvData );	// 回避方法をトレースに変更.
	MyTraceSD(JSVD_DBG_INFO, "JSVC_RequestOneShot::setData ss_size:" , ss_size );		// 回避方法をトレースに変更.
	MyTraceSD(JSVD_DBG_INFO, "JSVC_RequestOneShot::setData MsgType:" , (int)getMsgTypeId() );		// 回避方法をトレースに変更.// warning対応 2013/09/18 ooba.
#endif
	switch ( getMsgTypeId() ) {
	case JSVE_JobManager_MsgID_GetCurrentJob:
		{
			JSVS_ReqPara_GetCurrentJob* pMsg = (JSVS_ReqPara_GetCurrentJob*)pRcvData;
			ul_Param = pMsg->ul_JobDivision ;
		}
		break ;
	case JSVE_JobManager_MsgID_ForceEject:						// 強制排紙要求.
		// パラメータがないので Nop.
		break ;
	case JSVE_JobManager_MsgID_InterruptMode:					// 割り込み指示要求.
		{
			JSVS_ReqPara_InterruptMode* pMsg = (JSVS_ReqPara_InterruptMode*)pRcvData;
			ul_Param = pMsg->ul_IntMode ;
		}
		break ;
	case JSVE_JobManager_MsgID_SetRevivalAddressBook:			// 再送信宛先再設定.
		{
			JSVS_ReqPara_JobResource* pMsg = (JSVS_ReqPara_JobResource*)pRcvData;
			ul_Param = pMsg->ul_JobResourceID;
		}
		break ;
	case JSVE_JobManager_MsgID_UdateJobWarning:
		{
			JSVS_ReqPara_JobResourceWarning* pMsg = (JSVS_ReqPara_JobResourceWarning*)pRcvData;
			ul_Param = pMsg->ul_JobResourceID;			// ジョブリソースID.
			ul_SubScenarioID = pMsg->e_WarningType;		// ジョブリソース警告種類.
			ul_Data = pMsg->ul_On;						// On(1)/Off(0).
#if JSVC_Request_ResourceWarningDBG
			printf("★★★★★UdateJobWarning::setData Param ul_JobResourceID[%d]\n", ul_Param);
			printf("★★★★★UdateJobWarning::setData Param e_WarningType   [%d]\n", ul_SubScenarioID);
			printf("★★★★★UdateJobWarning::setData Param ul_On           [%d]\n", ul_Data);
#endif
		}
		break;
	case JSVE_JobManager_MsgID_SyncJobResourceWarningInfo:			// ジョブリソース警告情報同期要求.
		{
			JSVS_ReqPara_JobResource* pMsg = (JSVS_ReqPara_JobResource*)pRcvData;
			ul_Param = pMsg->ul_JobResourceID;
		}
		break ;
	default:
		{
			JSVS_ReqMsgParam_Base3* pMsg = (JSVS_ReqMsgParam_Base3*)pRcvData;

			//	送信履歴内の管理番号.
			ul_SndHistNum = pMsg->ul_SndHistNum;

			// 送信履歴データベースID取得要求時はアドレス番号を取得.
			if ( getMsgTypeId() == JSVE_JobManager_MsgID_GetSendLog_DataBaseID ){
				ul_AddressNum = pMsg->ul_AddressNum;
			}
		}
		break ;
	}
	return False;
}
// JobManagerから操作するI/Fを定義.
JSVE_Request_Result JSVC_RequestOneShot::startAction(JSVS_NotifyInfo* pNotify)
{
	JSVE_Request_Result e_Request_Result = JSVE_Request_Result_Err;
	uint32_t ulReqMsgTypeId = pNotify->pReq->getMsgTypeId();
	JSVS_SendRecv_MsgParam sParam ;
	JSVC_ResponseOneShotBase* pc_Res = NULL ;
	JSVS_ResponseParamOneShotBase s_ResBase ;

	switch(ulReqMsgTypeId){

	// 要求系同期.
	case JSVE_JobManager_MsgID_GetCurrentJob:					// カレントジョブ取得.
		{
			APIC_Job* pc_Job = NULL ;
			JSVC_RequestOneShot* pc_Req = (JSVC_RequestOneShot*)pNotify->pReq ;

			switch ( pc_Req->ul_Param ) {
			case IPCE_GetCurrentJobDivision_Scan:
				pc_Job = APIC_ActiveJobList::newInstance()->getCurrentScanJob() ;
				e_Request_Result = JSVE_Request_Result_Complete ;
				break ;
			case IPCE_GetCurrentJobDivision_Print:
				pc_Job = APIC_ActiveJobList::newInstance()->getCurrentPrintJob() ;
				e_Request_Result = JSVE_Request_Result_Complete ;
				break ;
			default:
				e_Request_Result = JSVE_Request_Result_Err ;
				break ;
			}
			if ( e_Request_Result != JSVE_Request_Result_Err ) {
				this->ul_Result = IPCE_JobServiceResult_OK ;
				this->ul_ResultDetail = IPCE_JobsResultDetail_Non ;
				if ( pc_Job != NULL ) {
					s_ResBase.ul_Data1 = pc_Job->getJobStatus()->getSubScenarioID() ;
// 2016/1/13 CQ523488 復旧JobでSegFし、リブート後Fax送信できない問題を修正 ht_agotou Start.
					s_ResBase.ul_Data2 = pc_Job->getJobResourceID();
// 2016/1/13 CQ523488 復旧JobでSegFし、リブート後Fax送信できない問題を修正 ht_agotou End.
				}
			} else {
				this->ul_Result = IPCE_JobServiceResult_NG ;
				this->ul_ResultDetail = IPCE_JobsResultDetail_InputErr ;
			}
		}

		pc_Res = new JSVC_ResponseOneShotBase(ulReqMsgTypeId) ;
		#if DEBUGLOG__REQUEST_LIFE
			MyTraceSLD(JSVD_DBG_INFO, "JSVC_RequestOneShot::startAction new JSVC_ResponseOneShotBase1" , (intptr_t)pc_Res );	// warning対応 2013/09/18 ooba.
		#endif
		// Response クラスのメモリ確保に失敗したらそのままエラーで APF に返す.
		if ( pc_Res == NULL ) {
			//設定してAPFへ通信
			this->ul_Result = IPCE_JobServiceResult_NG ;
			this->ul_ResultDetail = IPCE_JobsResultDetail_MemAllocFailed ;
			sParam.ul_MsgTypeID = pNotify->e_JobManager_MsgTypeID;
			sParam.pReq = this;
			JSVC_JobSendRecv::newInstance()->sendMessageToAPF(&sParam);
			//途中で止める.
			return  JSVE_Request_Result_Err ;
		}
		pc_Res->setData((char*)&s_ResBase,sizeof(JSVS_ResponseParamOneShotBase));
		sParam.ul_MsgTypeID = ulReqMsgTypeId ;
		sParam.pReq = this ;
		sParam.pRes = pc_Res ;
		JSVC_JobSendRecv::newInstance()->sendMessageToAPF(&sParam) ;
		e_Request_Result = JSVE_Request_Result_Complete ;
// 2012.08.29 sk_takas for CQ#370048 -->
		delete pc_Res ;
		pc_Res = NULL ;
// 2012.08.29 sk_takas for CQ#370048 <--

		break ;
	case JSVE_JobManager_MsgID_ForceEject:						// 強制排紙要求.
		APIC_ActiveJobList::newInstance()->forceEject() ;

		pc_Res = new JSVC_ResponseOneShotBase(ulReqMsgTypeId) ;
		#if DEBUGLOG__REQUEST_LIFE
			MyTraceSLD(JSVD_DBG_INFO, "JSVC_RequestOneShot::startAction new JSVC_ResponseOneShotBase2" , (intptr_t)pc_Res );	// warning対応 2013/09/18 ooba.
		#endif
		// Response クラスのメモリ確保に失敗したらそのままエラーで APF に返す.
		if ( pc_Res == NULL ) {
			//設定してAPFへ通信
			this->ul_Result = IPCE_JobServiceResult_NG ;
			this->ul_ResultDetail = IPCE_JobsResultDetail_MemAllocFailed ;
			sParam.ul_MsgTypeID = pNotify->e_JobManager_MsgTypeID;
			sParam.pReq = this;
			JSVC_JobSendRecv::newInstance()->sendMessageToAPF(&sParam);
			//途中で止める.
			return  JSVE_Request_Result_Err ;
		}
		this->ul_Result = IPCE_JobServiceResult_OK ;
		this->ul_ResultDetail = IPCE_JobsResultDetail_Non ;
		pc_Res->setData((char*)&s_ResBase,sizeof(JSVS_ResponseParamOneShotBase));
		sParam.ul_MsgTypeID = ulReqMsgTypeId ;
		sParam.pReq = this ;
		sParam.pRes = pc_Res ;
		JSVC_JobSendRecv::newInstance()->sendMessageToAPF(&sParam) ;
		e_Request_Result = JSVE_Request_Result_Complete ;
// 2012.08.29 sk_takas for CQ#370048 -->
		delete pc_Res ;
		pc_Res = NULL ;
// 2012.08.29 sk_takas for CQ#370048 <--

		break ;
	case JSVE_JobManager_MsgID_ExtendPrintStart:				// プリント延長指示要求.
		APIC_ActiveJobList::newInstance()->extendPrintStart() ;

		pc_Res = new JSVC_ResponseOneShotBase(ulReqMsgTypeId) ;
		#if DEBUGLOG__REQUEST_LIFE
			MyTraceSLD(JSVD_DBG_INFO, "JSVC_RequestOneShot::startAction new JSVC_ResponseOneShotBase3" , (intptr_t)pc_Res );	// warning対応 2013/09/18 ooba.
		#endif
		// Response クラスのメモリ確保に失敗したらそのままエラーで APF に返す.
		if ( pc_Res == NULL ) {
			//設定してAPFへ通信
			this->ul_Result = IPCE_JobServiceResult_NG ;
			this->ul_ResultDetail = IPCE_JobsResultDetail_MemAllocFailed ;
			sParam.ul_MsgTypeID = pNotify->e_JobManager_MsgTypeID;
			sParam.pReq = this;
			JSVC_JobSendRecv::newInstance()->sendMessageToAPF(&sParam);
			//途中で止める.
			return  JSVE_Request_Result_Err ;
		}
		this->ul_Result = IPCE_JobServiceResult_OK ;
		this->ul_ResultDetail = IPCE_JobsResultDetail_Non ;
		pc_Res->setData((char*)&s_ResBase,sizeof(JSVS_ResponseParamOneShotBase));
		sParam.ul_MsgTypeID = ulReqMsgTypeId ;
		sParam.pReq = this ;
		sParam.pRes = pc_Res ;
		JSVC_JobSendRecv::newInstance()->sendMessageToAPF(&sParam) ;
		e_Request_Result = JSVE_Request_Result_Complete ;
// 2012.08.29 sk_takas for CQ#370048 -->
		delete pc_Res ;
		pc_Res = NULL ;
// 2012.08.29 sk_takas for CQ#370048 <--

		break ;
	case JSVE_JobManager_MsgID_GetSendModeForRightFax:			// RightFaxの送信種別取得.
		s_ResBase.ul_Data1 = (uint32_t)APIC_ActiveJobList::newInstance()->getSendModeForRightFax() ;

		pc_Res = new JSVC_ResponseOneShotBase(ulReqMsgTypeId) ;
		#if DEBUGLOG__REQUEST_LIFE
			MyTraceSLD(JSVD_DBG_INFO, "JSVC_RequestOneShot::startAction new JSVC_ResponseOneShotBase4" , (intptr_t)pc_Res );	// warning対応 2013/09/18 ooba.
		#endif
		// Response クラスのメモリ確保に失敗したらそのままエラーで APF に返す.
		if ( pc_Res == NULL ) {
			//設定してAPFへ通信
			this->ul_Result = IPCE_JobServiceResult_NG ;
			this->ul_ResultDetail = IPCE_JobsResultDetail_MemAllocFailed ;
			sParam.ul_MsgTypeID = pNotify->e_JobManager_MsgTypeID;
			sParam.pReq = this;
			JSVC_JobSendRecv::newInstance()->sendMessageToAPF(&sParam);
			//途中で止める.
			return  JSVE_Request_Result_Err ;
		}
		this->ul_Result = IPCE_JobServiceResult_OK ;
		this->ul_ResultDetail = IPCE_JobsResultDetail_Non ;
		pc_Res->setData((char*)&s_ResBase,sizeof(JSVS_ResponseParamOneShotBase));
		sParam.ul_MsgTypeID = ulReqMsgTypeId ;
		sParam.pReq = this ;
		sParam.pRes = pc_Res ;
		JSVC_JobSendRecv::newInstance()->sendMessageToAPF(&sParam) ;
		e_Request_Result = JSVE_Request_Result_Complete ;
// 2012.08.29 sk_takas for CQ#370048 -->
		delete pc_Res ;
		pc_Res = NULL ;
// 2012.08.29 sk_takas for CQ#370048 <--

		break ;

	// 要求系非同期.
	// ※応答系とセットで実装すること.
	case JSVE_JobManager_MsgID_InterruptMode:					// 割り込み指示要求.
		{
			JSVC_RequestOneShot* pc_Req = (JSVC_RequestOneShot*)pNotify->pReq ;
			int32_t sl_index ;
			Bool bl_IntMode = True ;

			bl_IntMode = ( pc_Req->ul_Param == IPCE_InterruptMode_On ) ;
			sl_index = JSVC_JobManager::newInstance()->availableAnsFunctionIndex( JSVC_CB_INDEX_ans_interruptMode, JSVC_CB_MAX_ans_interruptMode, pNotify->pReq ) ;
			if ( -1 == sl_index )
			{
				this->ul_Result = IPCE_JobServiceResult_NG ;
				this->ul_ResultDetail = IPCE_JobsResultDetail_CBNumOver ;
				sParam.ul_MsgTypeID = ulReqMsgTypeId ;
				sParam.pReq = this ;
				JSVC_JobSendRecv::newInstance()->sendMessageToAPF(&sParam) ;
				e_Request_Result = JSVE_Request_Result_Err ;
			}else{
				APIC_ActiveJobList::newInstance()->req_interruptMode( bl_IntMode, (void (*)(Bool))(JSVC_JobManager::newInstance()->c_CBManage[sl_index].vp_Func) ) ;
				this->ul_Result = IPCE_JobServiceResult_OK ;
				this->ul_ResultDetail = IPCE_JobsResultDetail_Non ;
				sParam.ul_MsgTypeID = JSVE_JobManager_MsgID_InterruptModeAccept;
				sParam.pReq = this ;
				JSVC_JobSendRecv::newInstance()->sendMessageToAPF(&sParam) ;
				e_Request_Result = JSVE_Request_Result_Ok ;
			}
		}
		break ;

	// 応答系
	case JSVE_JobManager_MsgID_InterruptModeAccept:				// 割り込み指示応答.

		if ( pNotify->pRes == NULL ) {
			// pRes が NULL⇒コールバックでメモリ確保できなかった異常シーケンス.
			this->ul_Result = IPCE_JobServiceResult_NG;
			this->ul_ResultDetail = IPCE_JobsResultDetail_MemAllocFailed ;
		} else {
			// コールバックにて pRes に設定しているためここではセットしない.
			this->ul_Result = JSVE_Request_Result_Ok;
			this->ul_ResultDetail = IPCE_JobsResultDetail_Non ;
		}
		sParam.ul_MsgTypeID = ulReqMsgTypeId ;
		sParam.pReq = pNotify->pReq ;
		sParam.pRes = pNotify->pRes ;
		JSVC_JobSendRecv::newInstance()->sendMessageToAPF(&sParam);
		e_Request_Result = JSVE_Request_Result_Complete;

		break ;
	case JSVE_JobManager_MsgID_SetRevivalAddressBook:			// 再送信宛先再設定.
		{
			//ジョブリソースIDがul_Paramに入っているので、これでジョブポインタを取得.
			JSVC_ResponseOneShotBase cRes(JSVE_JobManager_MsgID_SetRevivalAddressBookAccept);
			
			APIC_Job* pJob = getInstancebyResourceId(ul_Param);
			if ( pJob ){
				//インスタンスが取れたら再送信先を再設定する.
				pJob->setRevivalAddressBook();
				cRes.setResult(JSVE_Request_Result_Ok);
				cRes.setResultDetail(IPCE_JobsResultDetail_Non);
			} else {
				//インスタンスなしの場合はエラーを返す.
				cRes.setResult(IPCE_JobServiceResult_NG);
				cRes.setResultDetail(IPCE_JobsResultDetail_JobPointerNull);
			}

			cRes.setData(NULL,0);	//設定データが無いが、内部の調整値を変更するためにコールしておく.

			sParam.ul_MsgTypeID = JSVE_JobManager_MsgID_SetRevivalAddressBookAccept;
			sParam.pReq = pNotify->pReq;
			sParam.pRes = &cRes;
			JSVC_JobSendRecv::newInstance()->sendMessageToAPF(&sParam);
			e_Request_Result = JSVE_Request_Result_Complete;
			
		}
		break ;
	case JSVE_JobManager_MsgID_UdateJobWarning:
		{
			JSVC_ResponseOneShotBase cRes(JSVE_JobManager_MsgID_UdateJobWarningAccept);

			// ジョブリソースIDからジョブインスタンスを取得する.
#if JSVC_Request_ResourceWarningDBG
			printf("★★★★★UdateJobWarning::startAction resetWarning ul_JobResourceID   [%d]\n", ul_Param);
#endif
			APIC_Job* pJob = getInstancebyResourceId(ul_Param);
			if ( pJob ){
				// 「On(1)/Off(0)」がOffの場合、リセットする.
				// 「On(1)/Off(0)」がOnの場合、セットする.
				// ul_Dataに「On(1)/Off(0)」をセットしているので、注意が必要.
				// ul_SubScenarioIDに「ジョブリソース警告種類」を設定しているので、注意が必要.
				if ( False == ul_Data ){
#if JSVC_Request_ResourceWarningDBG
					printf("★★★★★UdateJobWarning::startAction resetWarning e_WarningType   [%d]\n", ul_SubScenarioID);
#endif
					pJob->resetWarning( (TYPE_JobWarning)ul_SubScenarioID );
				}else{
#if JSVC_Request_ResourceWarningDBG
					printf("★★★★★UdateJobWarning::startAction setWarning e_WarningType   [%d]\n", ul_SubScenarioID);
#endif
					pJob->setWarning( (TYPE_JobWarning)ul_SubScenarioID );
				}
				cRes.setResult(JSVE_Request_Result_Ok);
				cRes.setResultDetail(IPCE_JobsResultDetail_Non);

// CQ370001 20120823 Sky福原 ->.
				TYPC_JobWarning c_JobWarning;				// 作業用領域.
				TYPC_JobWarning* pc_JobWarning = NULL;		// 更新対象の警告情報.

				// 作業領域のクリア.
				c_JobWarning.initData();
				pc_JobWarning = pJob->getJobWarning();
// CQ371508 20120911 Sky福原 ->.
				// 共有メモリ上のJobデータの警告を作業用領域に取得
				c_JobWarning.updataWarningData( pc_JobWarning );
// CQ371508 20120911 Sky福原 <-.
				if ( NULL != pc_JobWarning )
				{
					// 警告判断および警告情報の生成.
					c_JobWarning.setWarningData( (TYPE_JobWarning)ul_SubScenarioID, pJob->isWarning( (TYPE_JobWarning)ul_SubScenarioID ) );

					// 前回のジョブリソース警告状態と比較する.
					if ( False == c_JobWarning.isMatchWarningData( pc_JobWarning ) ){
						// 警告状態を更新する.
						// 次回の比較に使用する.
						pc_JobWarning->updataWarningData( &c_JobWarning );

						// 警告情報に変化があった場合のみ通知を行う.
						// JSVE_JobManager_MsgID_WarningCheckTimer_Expiredについては再利用している.
						JSVC_RequestTimer* pReq = new JSVC_RequestTimer(0,JSVE_JobManager_MsgID_WarningCheckTimer_Expired);
//Str【 静的解析対応 : Sky前田 : 2012.12.03 : CID-454789 】.
//						pReq->setData( ul_Param );
//						JSVS_SendRecv_MsgParam s_SendRecv_MsgParam;
//						s_SendRecv_MsgParam.ul_MsgTypeID = JSVE_JobManager_MsgID_WarningCheckTimer_Expired;
//						s_SendRecv_MsgParam.pReq = (JSVC_RequestBase*)pReq;
//						s_SendRecv_MsgParam.pRes = NULL;
//						JSVC_JobManager::newInstance()->sendMessageToJob( &s_SendRecv_MsgParam );
						if(pReq != NULL)
						{
							pReq->setData( ul_Param );
							JSVS_SendRecv_MsgParam s_SendRecv_MsgParam;
							s_SendRecv_MsgParam.ul_MsgTypeID = JSVE_JobManager_MsgID_WarningCheckTimer_Expired;
							s_SendRecv_MsgParam.pReq = (JSVC_RequestBase*)pReq;
							s_SendRecv_MsgParam.pRes = NULL;
							JSVC_JobManager::newInstance()->sendMessageToJob( &s_SendRecv_MsgParam );
						}
//End【 静的解析対応 : Sky前田 : 2012.12.03 : CID-454789 】.
					}else{
						// 警告状態に変化なし.
					}
// AR1014739対応 2017/5/30 Soma Start.
					// 原稿サイズ検出エラー警告が解除される場合、原稿とり忘れ警告も解除される可能性もあるため共有メモリを更新しておく.
					if ((ERROR_DocSizeDetectErrorAMS_S == (TYPE_JobWarning)ul_SubScenarioID) ||
					    (ERROR_DocSizeDetectErrorAPS_S == (TYPE_JobWarning)ul_SubScenarioID) ||
					    (ERROR_DocSizeDetectErrorOthers_S == (TYPE_JobWarning)ul_SubScenarioID) ||
					    (ERROR_DocSizeDetectErrorScan_S == (TYPE_JobWarning)ul_SubScenarioID) ||
					    (ERROR_DocSizeDetectErrorFax_S == (TYPE_JobWarning)ul_SubScenarioID)) {
						// 警告情報更新(非同期).
						// [changelog] 2018/01/15 Cuong #245395 Fix static check for IT5_4.2 F00-U2(07) start.
						JSVC_JobManager::newInstance()->UpdateJobWarningAsync(static_cast<short>(ul_Param));
						// [changelog] 2018/01/15 Cuong #245395 Fix static check for IT5_4.2 F00-U2(07) end.
					}
// AR1014739対応 2017/5/30 Soma End.
				}else{
					// Nop.
				}
// CQ370001 20120823 Sky福原 <-.

			} else {
				//インスタンスなしの場合はエラーを返す.
				cRes.setResult(IPCE_JobServiceResult_NG);
				cRes.setResultDetail(IPCE_JobsResultDetail_JobPointerNull);
			}

			cRes.setData(NULL,0);	//設定データが無いが、内部の調整値を変更するためにコールしておく.

			sParam.ul_MsgTypeID = JSVE_JobManager_MsgID_UdateJobWarningAccept;
			sParam.pReq = pNotify->pReq;
			sParam.pRes = &cRes;
			JSVC_JobSendRecv::newInstance()->sendMessageToAPF(&sParam);
			e_Request_Result = JSVE_Request_Result_Complete;
#if JSVC_Request_ResourceWarningDBG
			printf("★★★★★UdateJobWarning::startAction UdateJobWarningAccept Send\n");
#endif
		}
		break;
	case JSVE_JobManager_MsgID_SyncJobResourceWarningInfo:			// ジョブリソース警告情報同期要求.
		{
			JSVC_ResponseJobResourceWarning cRes(JSVE_JobManager_MsgID_SyncJobResourceWarningInfoAccept);

			TYPC_JobWarning c_JobResourceWarning;

// 設定警告対応 Sukigara s.
			if ((uint32_t)JSVE_UpdateJobWarningType_ALL == ul_Param){				// warning対応 2013/09/21 ooba.
				JSVC_JobManager::newInstance()->UpdateJobWarning(JSVE_UpdateJobWarningType_ALL);

				//パラメータの作成.
				JSVS_SyncJobResourceWarningInfoAnsMsg s_ResponseParam;
				memset( &s_ResponseParam , 0 , sizeof(s_ResponseParam) );
				// ジョブリソースID.
				s_ResponseParam.ul_JobResourceID = ul_Param;
				// ジョブリソース警告.
				// レスポンスクラスへ設定.
				cRes.setData( (char*)&s_ResponseParam , sizeof(s_ResponseParam) );

				// レスポンスクラスの結果編集.
				cRes.setResult(JSVE_Request_Result_Ok);
				cRes.setResultDetail(IPCE_JobsResultDetail_Non);
			}else{
			// ジョブリソースIDからジョブインスタンスを取得する.
			APIC_Job* pc_Job = getInstancebyResourceId(ul_Param);
			TYPC_JobWarning* pc_JobResourceWarning = NULL;
			if ( NULL != pc_Job ){
				pc_JobResourceWarning = pc_Job->getJobWarning();
			}

			if (( NULL != pc_Job ) && ( NULL != pc_JobResourceWarning ))
			{
				// 警告判断および警告情報の生成.
				for( int i = 0 ; i < ERROR_MAX ; i++ ){
					// 警告状態をデータ設定.
					c_JobResourceWarning.setWarningData( (TYPE_JobWarning)i, pc_Job->isWarning( (TYPE_JobWarning)i ) );
				}

				// 前回のジョブリソース警告状態と比較する.
				if ( False == c_JobResourceWarning.isMatchWarningData( pc_JobResourceWarning ) ){
					// 警告情報に変化があった場合のみ更新を行う.
					pc_JobResourceWarning->updataWarningData( &c_JobResourceWarning );
				}

				//パラメータの作成.
				JSVS_SyncJobResourceWarningInfoAnsMsg s_ResponseParam;
				memset( &s_ResponseParam , 0 , sizeof(s_ResponseParam) );
				// ジョブリソースID.
				s_ResponseParam.ul_JobResourceID = ul_Param;
				// ジョブリソース警告.
				// 更新後の警告状態をレスポンスへ設定する.
// OP_BTS-3176対応 警告領域枯渇のため領域拡張8(32byte)->16(64byte) 2019/03/07 soma.
				for( short j = 0 ; j < 16 ; j++ ){
					s_ResponseParam.ul_JobWarning[j] = pc_JobResourceWarning->getWarningData(j);
				}
				// レスポンスクラスへ設定.
				cRes.setData( (char*)&s_ResponseParam , sizeof(s_ResponseParam) );

				// レスポンスクラスの結果編集.
				cRes.setResult(JSVE_Request_Result_Ok);
				cRes.setResultDetail(IPCE_JobsResultDetail_Non);
			} else {
				//インスタンスなしの場合はエラーを返す.
				cRes.setResult(IPCE_JobServiceResult_NG);
				cRes.setResultDetail(IPCE_JobsResultDetail_JobPointerNull);
			}
// 設定警告対応 Sukigara e.			
			}

			// メッセージ送信.
			JSVS_SendRecv_MsgParam sParam;
			sParam.ul_MsgTypeID = JSVE_JobManager_MsgID_SyncJobResourceWarningInfoAccept;
			sParam.pReq = pNotify->pReq;
			sParam.pRes = &cRes;
			JSVC_JobSendRecv::newInstance()->sendMessageToAPF(&sParam);

			e_Request_Result = JSVE_Request_Result_Complete;		//同期で終わったから消していいことを伝える.
		}
		break;
	case JSVE_JobManager_MsgID_GetReceiveRef_DataBaseID:
	case JSVE_JobManager_MsgID_GetSendLog_DataBaseID:
	case JSVE_JobManager_MsgID_GetSendLogSubScenarioNum:
	case JSVE_JobManager_MsgID_StartSendLogAccess:
	case JSVE_JobManager_MsgID_EndSendLogAccess:
	case JSVE_JobManager_MsgID_GetSendLogAddressNum:
	case JSVE_JobManager_MsgID_GetSendLogSubScenarioNo:
	case JSVE_JobManager_MsgID_DeleteLog :
		{
			//受信履歴データベースID取得.
			this->ul_Result = IPCE_JobServiceResult_NG;
			this->ul_ResultDetail = IPCE_JobsResultDetail_NotSupport;
			sParam.ul_MsgTypeID = pNotify->e_JobManager_MsgTypeID;
			sParam.pReq = this;
			JSVC_JobSendRecv::newInstance()->sendMessageToAPF(&sParam);
			//結果のセット.
			e_Request_Result = JSVE_Request_Result_Err;
		}
		break;
	default:
		MyTraceSS(JSVD_DBG_ERROR, "JSVC_RequestOneShot::startAction","想定外の呼出");
		e_Request_Result = JSVE_Request_Result_NotSupport;
		break;
	}

	return e_Request_Result ;
}
// 指定した応答に関連するリクエストかどうかを判定.
//  return		JSVD_REQUEST_NOT_MATCH		関係なし.
//				JSVD_REQUEST_MAYBE_MATCH	関係あり（他候補の可能性あり）.
//				JSVD_REQUEST_JUST_MATCH		関係あり（他候補の可能性なし）.
short JSVC_RequestOneShot::isResponseMsg(JSVC_Response* pRes)
{
#ifdef _DEBUG_JSVC_SEQ
	MyTraceSLD(JSVD_DBG_INFO, "JSVC_RequestOneShot::isResponseMsg pRes:" , (intptr_t)pRes );	// 回避方法をトレースに変更.
#endif
	// 全て同期の要求なので応答を受けるMSGが無い（全てJSVD_REQUEST_NOT_MATCH）.
	return JSVD_REQUEST_NOT_MATCH;
}
void JSVC_RequestOneShot::ans_interruptMode01( Bool bl_IntMode )
{
	short ss_index = 0 + JSVC_CB_INDEX_ans_interruptMode ;
	ans_interruptMode_wrap( bl_IntMode, ss_index ) ;
}
void JSVC_RequestOneShot::ans_interruptMode02( Bool bl_IntMode )
{
	short ss_index = 1 + JSVC_CB_INDEX_ans_interruptMode ;
	ans_interruptMode_wrap( bl_IntMode, ss_index ) ;
}
void JSVC_RequestOneShot::ans_interruptMode03( Bool bl_IntMode )
{
	short ss_index = 2 + JSVC_CB_INDEX_ans_interruptMode ;
	ans_interruptMode_wrap( bl_IntMode, ss_index ) ;
}
void JSVC_RequestOneShot::ans_interruptMode04( Bool bl_IntMode )
{
	short ss_index = 3 + JSVC_CB_INDEX_ans_interruptMode ;
	ans_interruptMode_wrap( bl_IntMode, ss_index ) ;
}
void JSVC_RequestOneShot::ans_interruptMode05( Bool bl_IntMode )
{
	short ss_index = 4 + JSVC_CB_INDEX_ans_interruptMode ;
	ans_interruptMode_wrap( bl_IntMode, ss_index ) ;
}
void JSVC_RequestOneShot::ans_interruptMode06( Bool bl_IntMode )
{
	short ss_index = 5 + JSVC_CB_INDEX_ans_interruptMode ;
	ans_interruptMode_wrap( bl_IntMode, ss_index ) ;
}
void JSVC_RequestOneShot::ans_interruptMode07( Bool bl_IntMode )
{
	short ss_index = 6 + JSVC_CB_INDEX_ans_interruptMode ;
	ans_interruptMode_wrap( bl_IntMode, ss_index ) ;
}
void JSVC_RequestOneShot::ans_interruptMode08( Bool bl_IntMode )
{
	short ss_index = 7 + JSVC_CB_INDEX_ans_interruptMode ;
	ans_interruptMode_wrap( bl_IntMode, ss_index ) ;
}
void JSVC_RequestOneShot::ans_interruptMode09( Bool bl_IntMode )
{
	short ss_index = 8 + JSVC_CB_INDEX_ans_interruptMode ;
	ans_interruptMode_wrap( bl_IntMode, ss_index ) ;
}
void JSVC_RequestOneShot::ans_interruptMode10( Bool bl_IntMode )
{
	short ss_index = 9 + JSVC_CB_INDEX_ans_interruptMode ;
	ans_interruptMode_wrap( bl_IntMode, ss_index ) ;
}
void JSVC_RequestOneShot::ans_interruptMode_wrap( Bool bl_IntMode, short ss_index )
{
	JSVC_RequestOneShot* pc_Req = NULL;
	void*	vpDataBuf = NULL;
	short ss_DataLen = 0;
	Bool b_Status;

	//排他区間ここから.
	{
		JSVC_CBManagerSemCtrl cCBManage;
		//CB管理領域から呼出パラメータを取得.
		pc_Req		= reinterpret_cast<JSVC_RequestOneShot*>(JSVC_JobManager::c_CBManage[ss_index].pc_ReqBase);
		vpDataBuf	= JSVC_JobManager::c_CBManage[ss_index].vp_DataBuf;
		ss_DataLen	= JSVC_JobManager::c_CBManage[ss_index].ss_DataLen;
		b_Status	= JSVC_JobManager::c_CBManage[ss_index].b_Status;
		//CB管理領域を開放.
		JSVC_JobManager::c_CBManage[ss_index].pc_ReqBase = NULL;
		JSVC_JobManager::c_CBManage[ss_index].vp_DataBuf = NULL;
		JSVC_JobManager::c_CBManage[ss_index].ss_DataLen = 0;
		JSVC_JobManager::c_CBManage[ss_index].b_Status   = False;		//管理領域を「未使用」に戻す.
	}
	//排他区間ここまで.

	//CB管理領域に存在.
	if ( b_Status == True ) {
		JSVS_ResponseParamOneShotBase s_ResBase ;
		JSVC_ResponseOneShotBase* pc_Res = NULL ;
		pc_Res = new JSVC_ResponseOneShotBase(JSVE_JobManager_MsgID_InterruptModeEnd) ;
		#if DEBUGLOG__REQUEST_LIFE
			MyTraceSLD(JSVD_DBG_INFO, "ans_interruptMode_wrap new JSVC_ResponseOneShotBase" , (intptr_t)pc_Res );	// warning対応 2013/09/18 ooba.
		#endif

		s_ResBase.ul_Data1 = bl_IntMode ;
		pc_Res->setData( (char*)&s_ResBase, sizeof(JSVS_ResponseParamOneShotBase) ) ;
		//JSVC_JobManagerへ送る.
		JSVC_JobManager::newInstance()->JSVC_JobManager::notify( JSVE_JobManager_MsgID_InterruptModeEnd, pc_Req, pc_Res ) ;
	}
	//バッファはここで確実に削除.
	if ( vpDataBuf ) {
//Str【 静的解析対応 : Sky花岡 : 2012.08.30 : CID-436328】.
//JSVC_JobManager::c_CBManage[sc_index].vp_DataBufが使用されていない為、delete処理を削除.
//JSVC_JobManager::c_CBManage[sc_index].vp_DataBufを使用する場合、下記、delete処理を復活させる事。
//なお、void* で指されている領域を開放する場合はfreeで開放する事（確保はmalloc）。
//		delete vpDataBuf;
//End【 静的解析対応 : Sky花岡 : 2012.08.30 : CID-436328】.
		vpDataBuf = NULL;
	}
}

//--------------------------------------------------------------------------------------------------.
//	JSVC_RequestOneShotAsyncLogクラス定義.
//--------------------------------------------------------------------------------------------------.
// コンストラクタ.
JSVC_RequestOneShotAsyncLog::JSVC_RequestOneShotAsyncLog(uint32_t ul_RequestID, uint32_t ul_MsgTypeID)
	: JSVC_RequestOneShot(ul_RequestID, ul_MsgTypeID)
	, ul_Param1(0)
	, ul_Param2(0)
	, b_ExecFlg(False)
	, vp_LogData1(NULL)
	, vp_LogData2(NULL)
{
	/* NOP */
}

// デストラクタ.
JSVC_RequestOneShotAsyncLog::~JSVC_RequestOneShotAsyncLog()
{
	// 動的領域のメモリ解放.
	if (vp_LogData1)
	{
		free(vp_LogData1);
		vp_LogData1 = NULL;
	}

	if (vp_LogData2)
	{
		free(vp_LogData2);
		vp_LogData2 = NULL;
	}
}

// 自身のリクエストクラス種別を返す.
JSVE_Request_Class_Type JSVC_RequestOneShotAsyncLog::WhoAmI()
{
	return JSVE_Request_Class_Type_OneShot_Async;
}

// 受信データバッファの保存処理.
Bool JSVC_RequestOneShotAsyncLog::setData(const char* pRcvData, short ss_size)
{
	// MsgID毎に必要なデータを格納する.
	MyTraceSD( JSVD_DBG_INFO, "JSVC_RequestOneShotAsyncLog::setData MsgType:" , (int)getMsgTypeId() );	// warning対応 2013/09/18 ooba.
	switch ( getMsgTypeId() )
	{
	case JSVE_JobManager_MsgID_JobLog_CreateFile:				// ジョブログファイル作成要求.
		{
			JSVS_ReqPara_JobLogCreateFile* pst_Data = (JSVS_ReqPara_JobLogCreateFile*)(pRcvData);
			ul_Param1 = pst_Data->ul_MakeMode;
		}
		break;
	case JSVE_JobManager_MsgID_DeleteLog:						// ログ削除要求.
		{
			JSVS_ReqPara_DeleteLog* pst_Data = (JSVS_ReqPara_DeleteLog*)(pRcvData);
			ul_Param1 = pst_Data->ul_LogType;
			ul_Param2 = pst_Data->ul_DelMode;
		}
		break;
	case JSVE_JobManager_MsgID_RegistImageLog:					// アプリからの画像ログ登録要求.
		{
			// 少なくともバッファ破壊を起こさないよう最低限のチェックを入れておく.
			// 必要であればデータチェックモジュールに処理を移すこと.
			if ((uint32_t)ss_size < sizeof(uint32_t)*2)				// warning対応 2013/09/21 ooba.
			{
				// 受信データ長が最低限の長さに満たない.
				MyTraceSS(JSVD_DBG_ERROR, "JSVC_RequestOneShotAsyncLog::setData", "pRcvData size not enough");
				return False;
			}
			
			if ( ((uint32_t) *pRcvData) > (ss_size - sizeof(uint32_t)) )
			{
				// 可変長の長さが受信データサイズと一致しない(1).
				MyTraceSS(JSVD_DBG_ERROR, "JSVC_RequestOneShotAsyncLog::setData", "pRcvData overrun 1");
				return False;
			}

			// 可変長バッファの長さを保持.
			this->ul_Param1 = (uint32_t) *pRcvData;											// EasyLogFilePathLength
			uint32_t	ul_Param1_StreamByte;
			if(this->ul_Param1 == 0)
			{
				ul_Param1_StreamByte = sizeof(uint32_t);
			}
			else
			{
				ul_Param1_StreamByte = static_cast<uint32_t>(this->ul_Param1 + ((-this->ul_Param1) & (sizeof(uint32_t)-1)));
			}
			this->ul_Param2 = (uint32_t) *(pRcvData + sizeof(uint32_t) + ul_Param1_StreamByte);	// ImageLogFilePathLength
			
			if ((ul_Param1_StreamByte + this->ul_Param2) != (ss_size - sizeof(uint32_t) * 2))
			{
				// 可変長の長さが受信データサイズと一致しない(2).
				MyTraceSS(JSVD_DBG_ERROR, "JSVC_RequestOneShotAsyncLog::setData", "pRcvData overrun 2");
				return False;
			}

			// 念のため、NULL終端もチェック.
			if ( ( (pRcvData+sizeof(uint32_t))[(this->ul_Param1)-1] != 0x00) ||
				 ( pRcvData[ss_size-1] != 0x00 ) )
			{
				MyTraceSS(JSVD_DBG_ERROR, "JSVC_RequestOneShotAsyncLog::setData", "Invalid buffer string");
				return False;
			}

			// EasyLogFilePath.
			if (this->vp_LogData1)
			{
				free(this->vp_LogData1);
				this->vp_LogData1 = NULL;
			}
			if (this->ul_Param1)
			{
				vp_LogData1 = malloc(sizeof(char) * this->ul_Param1);
				if (vp_LogData1)
				{
					strncpy((char*) (this->vp_LogData1), pRcvData+sizeof(uint32_t), this->ul_Param1);
				}
			}

			// ImageLogFilePath
			if (this->vp_LogData2)
			{
				free(this->vp_LogData2);
				this->vp_LogData2 = NULL;
			}
			if (this->ul_Param2)
			{
				vp_LogData2 = malloc(sizeof(char) * this->ul_Param2);
				if (vp_LogData2)
				{
					strncpy((char*)(this->vp_LogData2), pRcvData+(sizeof(uint32_t)*2)+this->ul_Param1, this->ul_Param2);
				}
			}
		}
		break;
	default:
		// ありえない.
		MyTraceSS(JSVD_DBG_ERROR, "JSVC_RequestOneShotAsyncLog::setData", "invalid MsgID");
		return False;
	};

	return True;
}

// リクエスト処理.
JSVE_Request_Result JSVC_RequestOneShotAsyncLog::startAction(JSVS_NotifyInfo* pNotify)
{
	JSVE_Request_Result e_Request_Result = JSVE_Request_Result_NotSupport;
	JSVC_RequestOneShotAsyncLog* pc_Request = (JSVC_RequestOneShotAsyncLog*)(pNotify->pReq);

	JSVS_SendRecv_MsgParam st_Msg;
	memset(&st_Msg, 0x00, sizeof(st_Msg));

	JSVS_ResponseParam_OneShotLog st_ResParam;
	memset(&st_ResParam, 0x00, sizeof(JSVS_ResponseParam_OneShotLog));

	// MsgID毎に該当するAPIをコールする.
	// コマンドの対応一覧表を参照のこと.
	switch ( pNotify->e_JobManager_MsgTypeID )
	{
	case JSVE_JobManager_MsgID_JobLog_CreateFile:				// ジョブログファイル作成要求.
		{
			JSVC_ResponseOneShot_Log c_Response( JSVE_JobManager_MsgID_JobLog_CreateFileAccept );
			JLGE_Result e_ApiResult = JLGE_Result_Err;
			
			// ファイル状況、ジョブログ状況チェック.
			JLGE_JobLogStatus e_JobLogStatus = APIC_JLG::newInstance()->getJobLogStatus();
			JLGE_LogState e_LogState = APIC_JLG::newInstance()->getLogState();
			if ((e_LogState == JLGE_LogStateMaking) ||
				(e_JobLogStatus != JLGE_JobLogStatusEnable))
			{
				// ファイル作成中、ジョブログ動作中以外の場合は要求NGとする.
				e_ApiResult = JLGE_Result_Err;
			}
			else
			{
				e_ApiResult = APIC_JLG::newInstance()->LogCreateFile((JLGE_MakeMode)(ul_Param1));
			}

			if (e_ApiResult == JLGE_Result_Ok)
			{
				st_ResParam.ul_Result = IPCE_JobServiceResult_OK;
				e_Request_Result = JSVE_Request_Result_Ok;
				
				// 完了通知を受け取るために、処理要求中フラグをセットしておく.
				pc_Request->b_ExecFlg = True;
			}
			else
			{
				st_ResParam.ul_Result = IPCE_JobServiceResult_NG;
				e_Request_Result = JSVE_Request_Result_Err;
			}
			st_ResParam.ul_Param1 = ul_Param1;
			c_Response.setData((const char*)&st_ResParam, sizeof(st_ResParam));
			st_Msg.ul_MsgTypeID = JSVE_JobManager_MsgID_JobLog_CreateFileAccept;
			st_Msg.pReq = this;
			st_Msg.pRes = &c_Response;

			JSVC_JobSendRecv::newInstance()->sendMessageToAPF(&st_Msg);
		}
		break;
		
	case JSVE_JobManager_MsgID_DeleteLog:						// ログ削除要求.
		{
			Bool b_Failure = False;		// 失敗判定フラグ.
			int si_Result = OK;

			// 種別毎にAPIを呼び分け.
			switch ( pc_Request->ul_Param1 )
			{
			case IPCE_JobService_LogKind_SubSenarioLog:				// サブシナリオ履歴.
				APIC_DoneJobList::newInstance()->delJobLogAll();
				break;
			case IPCE_JobService_LogKind_ScanJournal:				// Scanジャーナル.
				si_Result = APIC_DoneJobList::newInstance()->deleteScanJournal();
				if (si_Result != OK)
				{
					b_Failure = True;
				}
				break;
			case IPCE_JobService_LogKind_FaxJournal:				// Faxジャーナル.
				si_Result = APIC_DoneJobList::newInstance()->deleteFaxJournal();
				if (si_Result != OK)
				{
					b_Failure = True;
				}
				break;
			case IPCE_JobService_LogKind_SendLog:					// 送信履歴.
				si_Result = APIC_DoneJobList::newInstance()->deleteSendLog();
				if (si_Result != OK)
				{
					b_Failure = True;
				}
				break;
			case IPCE_JobService_LogKind_ALL:						// 全ログ.
				APIC_DoneJobList::newInstance()->delJobLogAll();					// サブシナリオ履歴.
				si_Result = APIC_DoneJobList::newInstance()->deleteScanJournal();	// Scanジャーナル.
				if (si_Result != OK)
				{
					b_Failure = True;
					break;
				}
				si_Result = APIC_DoneJobList::newInstance()->deleteFaxJournal();	// Faxジャーナル.
				if (si_Result != OK)
				{
					b_Failure = True;
					break;
				}
				si_Result = APIC_DoneJobList::newInstance()->deleteSendLog();		// 送信履歴.
				if (si_Result != OK)
				{
					b_Failure = True;
					break;
				}
				// fall-throw.
			case IPCE_JobService_LogKind_JobLog:					// ジョブログ.
				{
					// JobLog状態確認.
					JLGE_JobLogStatus e_JobLogStatus = APIC_JLG::newInstance()->getJobLogStatus();
					if (e_JobLogStatus != JLGE_JobLogStatusEnable)
					{
						// JobLog有効時以外は要求NGとする.
						b_Failure = True;
					}
					else
					{
						// コールバック関数を取り出す.
						uint32_t ul_Index = JSVC_JobManager::newInstance()->availableAnsFunctionIndex(JSVC_CB_INDEX_ans_deleteJobLog, JSVC_CB_MAX_ans_deleteJobLog, pc_Request);
						if (ul_Index != (uint32_t)-1)					// warning対応 2013/09/21 ooba.
						{
							APIC_JLG::newInstance()->LogDelete((JLGE_DelMode)ul_Param2, (void (*)(int))(JSVC_JobManager::newInstance()->c_CBManage[ul_Index].vp_Func));
						}
						else
						{
							b_Failure = True;
						}
					}
				}
				break;
			default:												// 不正.
				MyTraceSS(JSVD_DBG_ERROR, "JSVC_RequestOneShotAsyncLog::startAction", "Invalid Param");
				b_Failure = True;
				break;
			}
			
			// AppP/Fへ通知.
			JSVC_ResponseOneShot_Log* pc_Response = new JSVC_ResponseOneShot_Log(JSVE_JobManager_MsgID_DeleteLogAccept);
#if DEBUGLOG__REQUEST_LIFE
			MyTraceSLD(JSVD_DBG_INFO, "JSVC_RequestOneShotAsyncLog::startAction new JSVC_ResponseOneShot_Log" , (intptr_t)pc_Response );	// warning対応 2013/09/18 ooba.
#endif
			
			if (b_Failure)
			{
				st_ResParam.ul_Result = IPCE_JobServiceResult_NG;
			}
			else
			{
				
				st_ResParam.ul_Result = IPCE_JobServiceResult_OK;
			}
			
			st_ResParam.ul_Param1 = pc_Request->ul_Param1;
			pc_Response->setData((const char*)&st_ResParam, sizeof(st_ResParam));
			st_Msg.ul_MsgTypeID = JSVE_JobManager_MsgID_DeleteLogAccept;
			st_Msg.pReq = this;
			st_Msg.pRes = pc_Response;
			
			JSVC_JobSendRecv::newInstance()->sendMessageToAPF(&st_Msg);
			
			delete pc_Response;
			pc_Response = NULL;
			
			if (b_Failure)
			{
				return JSVE_Request_Result_Err;
			}
			
			// 同期処理で削除完了したものは、完了通知を発行する.
			if (( pc_Request->ul_Param1 == IPCE_JobService_LogKind_SubSenarioLog) ||
				( pc_Request->ul_Param1 == IPCE_JobService_LogKind_ScanJournal)   ||
				( pc_Request->ul_Param1 == IPCE_JobService_LogKind_FaxJournal)    ||
				( pc_Request->ul_Param1 == IPCE_JobService_LogKind_SendLog)        )
			{
				// pc_Responseは通知の為に使いまわしする.
				pc_Response = new JSVC_ResponseOneShot_Log( JSVE_JobManager_MsgID_HistoryDelete );
#if DEBUGLOG__REQUEST_LIFE
				MyTraceSLD(JSVD_DBG_INFO, "JSVC_RequestOneShotAsyncLog::startAction new JSVC_ResponseOneShot_Log" , (intptr_t)pc_Response );	// warning対応 2013/09/18 ooba.
#endif // DEBUGLOG_REQUEST_LIFE.
				memset(&st_ResParam, 0x00, sizeof(st_ResParam));
				memset(&st_Msg, 0x00, sizeof(st_Msg));
				
				st_ResParam.ul_Result = IPCE_JobServiceResult_OK;
				st_ResParam.ul_Param1 = pc_Request->ul_Param1;
				pc_Response->setData((const char*)&st_ResParam, sizeof(st_ResParam));
				st_Msg.ul_MsgTypeID = JSVE_JobManager_MsgID_HistoryDelete;
				st_Msg.pReq = this;
				st_Msg.pRes = pc_Response;
				
				JSVC_JobManager::newInstance()->sendMessageToJob(&st_Msg);
				
				// ここではレスポンスの開放は行わない（非同期通知のため）.
			}
			
			// 削除完了通知にてリクエスト削除するため、ここではKeepを返し、リクエストを保持する.
			e_Request_Result = JSVE_Request_Result_Ok;
		}
		break;
		
	case JSVE_JobManager_MsgID_JobLogFileProgress:				// ジョブログファイル状況通知.
		{
			st_Msg.ul_MsgTypeID = pNotify->e_JobManager_MsgTypeID;
			st_Msg.pReq = pNotify->pReq;
			st_Msg.pRes = pNotify->pRes;
			JSVC_JobSendRecv::newInstance()->sendMessageToAPF(&st_Msg);
			e_Request_Result = JSVE_Request_Result_Ok;
		}
		break;
		
	case JSVE_JobManager_MsgID_JobLogFileMake:					// ジョブログファイル作成結果通知.
		{
			st_Msg.ul_MsgTypeID = pNotify->e_JobManager_MsgTypeID;
			st_Msg.pReq = pNotify->pReq;
			st_Msg.pRes = pNotify->pRes;
			if (reinterpret_cast<JSVC_ResponseOneShot_Log*>(pNotify->pRes)->getParam1() == IPCD_FileSts_End)
			{
				e_Request_Result = JSVE_Request_Result_Complete;
			}
			else
			{
				e_Request_Result = JSVE_Request_Result_Err;
			}
			JSVC_JobSendRecv::newInstance()->sendMessageToAPF(&st_Msg);
		}
		break;
		
	case JSVE_JobManager_MsgID_HistoryDelete:					// ログ削除完了通知.
		{
			st_Msg.ul_MsgTypeID = pNotify->e_JobManager_MsgTypeID;
			st_Msg.pReq = pNotify->pReq;
			st_Msg.pRes = pNotify->pRes;
			JSVC_JobSendRecv::newInstance()->sendMessageToAPF(&st_Msg);
			e_Request_Result = JSVE_Request_Result_Complete;
		}
		break;

	case JSVE_JobManager_MsgID_RegistImageLog:					// アプリからの画像ログ登録要求.
		{
			JSVC_ResponseOneShot_Log c_Response( JSVE_JobManager_MsgID_RegistImageLogAccept );
			Bool b_ApiResult = False;
			
			// IGLへ登録要求.
			TYPS_ImageLogFromApp s_ImageLog;
			memset(&s_ImageLog, 0x00, sizeof(TYPS_ImageLogFromApp));
			memcpy(s_ImageLog.sc_EasyLogFilePath,  this->vp_LogData1, (TYPD_FromAppMaxPathSize < this->ul_Param1) ? TYPD_FromAppMaxPathSize : this->ul_Param1);
			memcpy(s_ImageLog.sc_ImageLogFilePath, this->vp_LogData2, (TYPD_FromAppMaxPathSize < this->ul_Param2) ? TYPD_FromAppMaxPathSize : this->ul_Param2);
			
			b_ApiResult = APIC_ImageLogControll::newInstance()->req_RegistImageLogFromApp(&s_ImageLog, this,
																						  (void (*)(IGLE_RegRetFromApp, void*)) (this->ans_RegistImageLogFromApp));
			
			if (b_ApiResult)
			{
				st_ResParam.ul_Result = IPCE_JobServiceResult_OK;
				e_Request_Result = JSVE_Request_Result_Ok;
				b_ExecFlg = True;
			}
			else
			{
				st_ResParam.ul_Result = IPCE_JobServiceResult_NG;
				e_Request_Result = JSVE_Request_Result_Err;
			}
			
			c_Response.setData((const char*)&st_ResParam, sizeof(st_ResParam));
			st_Msg.ul_MsgTypeID = JSVE_JobManager_MsgID_RegistImageLogAccept;
			st_Msg.pReq = this;
			st_Msg.pRes = &c_Response;
			
			JSVC_JobSendRecv::newInstance()->sendMessageToAPF(&st_Msg);
		}
		break;

	case JSVE_JobManager_MsgID_RegistImageLogNotify:			// 画像ログ登録通知.
		{
			st_Msg.ul_MsgTypeID = pNotify->e_JobManager_MsgTypeID;
			st_Msg.pReq = pNotify->pReq;
			st_Msg.pRes = pNotify->pRes;
			JSVC_JobSendRecv::newInstance()->sendMessageToAPF(&st_Msg);
			e_Request_Result = JSVE_Request_Result_Complete;
		}
		break;

	default:													// 不正.
		// ありえない.
		break;
	}

	return e_Request_Result;
}

// 応答に関連するリクエストか判断.
short JSVC_RequestOneShotAsyncLog::isResponseMsg(JSVC_Response* pRes)
{
	short ss_Result = JSVD_REQUEST_NOT_MATCH;
	JSVE_JobManager_MsgID myMsgID = getMsgTypeId();
	JSVE_JobManager_MsgID resMsgID = pRes->getMsgTypeId();

	switch ( myMsgID )
	{
	case JSVE_JobManager_MsgID_DeleteLog:						// ログ削除要求.
		if (resMsgID == JSVE_JobManager_MsgID_HistoryDelete)
		{
			// 削除要求中 and 履歴削除結果通知.
			ss_Result = JSVD_REQUEST_JUST_MATCH;
		}
		break;
	case JSVE_JobManager_MsgID_JobLog_CreateFile:				// ジョブログファイル作成要求.
		if ((b_ExecFlg) &&
			((resMsgID == JSVE_JobManager_MsgID_JobLogFileMake) ||
			 (resMsgID == JSVE_JobManager_MsgID_JobLogFileProgress))
			)
		{
			// 作成要求中 and (ジョブログファイル作成結果通知 or ファイル作成状況通知).
			ss_Result = JSVD_REQUEST_JUST_MATCH;
		}
		break;
	case JSVE_JobManager_MsgID_RegistImageLog:					// アプリからの画像ログ登録要求.
		if((b_ExecFlg) &&
		   (resMsgID == JSVE_JobManager_MsgID_RegistImageLogNotify))
		{
			ss_Result = JSVD_REQUEST_JUST_MATCH;
		}
		break;
	default:													// その他.
		/* NOP */
		break;
	}

	return ss_Result;
}

// 削除要求コールバック関数 01.
void JSVC_RequestOneShotAsyncLog::ans_deleteJobLog01(int ret)
{
	// コールバックラッパへ自身のIndexを付与してバイパスする.
	return ans_deleteJobLog_wrap(ret, 0 + JSVC_CB_INDEX_ans_deleteJobLog);
}

// 削除要求コールバック関数 02.
void JSVC_RequestOneShotAsyncLog::ans_deleteJobLog02(int ret)
{
	// コールバックラッパへ自身のIndexを付与してバイパスする.
	return ans_deleteJobLog_wrap(ret, 1 + JSVC_CB_INDEX_ans_deleteJobLog);
}

// 削除要求コールバック関数 03.
void JSVC_RequestOneShotAsyncLog::ans_deleteJobLog03(int ret)
{
	// コールバックラッパへ自身のIndexを付与してバイパスする.
	return ans_deleteJobLog_wrap(ret, 2 + JSVC_CB_INDEX_ans_deleteJobLog);
}

// 削除要求コールバック関数 04.
void JSVC_RequestOneShotAsyncLog::ans_deleteJobLog04(int ret)
{
	// コールバックラッパへ自身のIndexを付与してバイパスする.
	return ans_deleteJobLog_wrap(ret, 3 + JSVC_CB_INDEX_ans_deleteJobLog);
}

// 削除要求コールバック関数 05.
void JSVC_RequestOneShotAsyncLog::ans_deleteJobLog05(int ret)
{
	// コールバックラッパへ自身のIndexを付与してバイパスする.
	return ans_deleteJobLog_wrap(ret, 4 + JSVC_CB_INDEX_ans_deleteJobLog);
}

// 削除コールバックからのラッパ.
void JSVC_RequestOneShotAsyncLog::ans_deleteJobLog_wrap(int ret, char sc_index)
{
	JSVC_RequestBase* pReq = NULL;
	void*	vpDataBuf = NULL;
	short ss_DataLen = 0;
	Bool b_Status;
	unsigned char uc_index = (unsigned char)sc_index;	// warning対応 2013/09/18 ooba.

	//排他区間ここから.
	{
		JSVC_CBManagerSemCtrl cCBManage;
		//CB管理領域から呼出パラメータを取得.
		// warning対応 2013/09/18 start ooba. indexをsc_indexからuc_indexに変更.
		pReq		= JSVC_JobManager::c_CBManage[uc_index].pc_ReqBase;
		vpDataBuf	= JSVC_JobManager::c_CBManage[uc_index].vp_DataBuf;
		ss_DataLen	= JSVC_JobManager::c_CBManage[uc_index].ss_DataLen;
		b_Status	= JSVC_JobManager::c_CBManage[uc_index].b_Status;
		//CB管理領域を開放.
		JSVC_JobManager::c_CBManage[uc_index].pc_ReqBase = NULL;
		JSVC_JobManager::c_CBManage[uc_index].vp_DataBuf = NULL;
		JSVC_JobManager::c_CBManage[uc_index].ss_DataLen = 0;
		JSVC_JobManager::c_CBManage[uc_index].b_Status   = False;		//管理領域を「未使用」に戻す.
		// warning対応 2013/09/18 end ooba.
	}
	//排他区間ここまで.

	// 本処理コール.
	if (b_Status)
	{
		ans_deleteJobLog_body(ret, pReq);
	}
}

// 削除コールバックの本処理.
void JSVC_RequestOneShotAsyncLog::ans_deleteJobLog_body(int ret, JSVC_RequestBase* pRequest)
{
	JSVS_ResponseParam_OneShotLog st_ResParam;
	memset(&st_ResParam, 0x00, sizeof(JSVS_ResponseParam_OneShotLog));

	JSVS_SendRecv_MsgParam st_Msg;
	memset(&st_Msg, 0x00, sizeof(JSVS_SendRecv_MsgParam));

	JSVC_ResponseOneShot_Log* pc_Response = new JSVC_ResponseOneShot_Log(JSVE_JobManager_MsgID_HistoryDelete);
#if DEBUGLOG__REQUEST_LIFE
	MyTraceSLD(JSVD_DBG_INFO, "JSVC_RequestOneShotAsyncLog::ans_deleteJobLog_body new JSVC_ResponseOneShot_Log" , (intptr_t)pc_Response );	// warning対応 2013/09/18 ooba.
#endif // DEBUGLOG_REQUEST_LIFE.

	if (ret == JLGE_Result_Ok)
	{
		st_ResParam.ul_Result = IPCE_JobServiceResult_OK;
	}
	else
	{
		st_ResParam.ul_Result = IPCE_JobServiceResult_NG;
	}

	st_ResParam.ul_Param1 = ((JSVC_RequestOneShotAsyncLog*)pRequest)->getParam1();
	pc_Response->setData((const char*)&st_ResParam, sizeof(st_ResParam));
	st_Msg.ul_MsgTypeID = JSVE_JobManager_MsgID_HistoryDelete;
	st_Msg.pReq = pRequest;
	st_Msg.pRes = pc_Response;

	JSVC_JobManager::newInstance()->sendMessageToJob(&st_Msg);
	
	// pc_ResponseはmessageLoopで解放されることを期待する.
}

void JSVC_RequestOneShotAsyncLog::ans_RegistImageLogFromApp(IGLE_RegRetFromApp ret, void* pRequest)
{
	JSVS_ResponseParam_OneShotLog st_ResParam;
	memset(&st_ResParam, 0x00, sizeof(JSVS_ResponseParam_OneShotLog));

	JSVS_SendRecv_MsgParam st_Msg;
	memset(&st_Msg, 0x00, sizeof(JSVS_SendRecv_MsgParam));

	JSVC_ResponseOneShot_Log* pc_Response = new JSVC_ResponseOneShot_Log(JSVE_JobManager_MsgID_RegistImageLogNotify);
#if DEBUGLOG__REQUEST_LIFE
	MyTraceSLD(JSVD_DBG_INFO, "JSVC_RequestOneShotAsyncLog::ans_RegistImageLogFromApp new JSVC_ResponseOneShot_Log" , (intptr_t)pc_Response );	// warning対応 2013/09/18 ooba.
#endif // DEBUGLOG_REQUEST_LIFE.

	if (ret == IGLE_RegRetFromAppOK)
	{
		st_ResParam.ul_Result = IPCE_JobServiceResult_OK;
	}
	else
	{
		st_ResParam.ul_Result = IPCE_JobServiceResult_NG;
	}

	pc_Response->setData((const char*)&st_ResParam, sizeof(st_ResParam));
	st_Msg.ul_MsgTypeID = JSVE_JobManager_MsgID_RegistImageLogNotify;
	st_Msg.pReq = (JSVC_RequestBase*) pRequest;
	st_Msg.pRes = pc_Response;

	JSVC_JobManager::newInstance()->sendMessageToJob(&st_Msg);
	
	// pc_ResponseはmessageLoopで解放されることを期待する.
}

//--------------------------------------------------------------------------------------------------.
//	JSVC_RequestOneShotSetJobLogクラス定義.
//--------------------------------------------------------------------------------------------------.

// コンストラクタ.
JSVC_RequestOneShotSetJobLog::JSVC_RequestOneShotSetJobLog(uint32_t ul_RequestID, uint32_t ul_MsgTypeID)
	: JSVC_RequestOneShot(ul_RequestID, ul_MsgTypeID)
	, ul_Param1(0)
	, ul_Param2(0)
	, vp_LogData(NULL)
{
	/* NOP */
}

// デストラクタ.
JSVC_RequestOneShotSetJobLog::~JSVC_RequestOneShotSetJobLog()
{
	// 動的確保領域を解放する.
	if ( vp_LogData )
	{
		free( vp_LogData );
		vp_LogData = NULL;
	}
}

// 自身のリクエストクラス種別を返す.
JSVE_Request_Class_Type JSVC_RequestOneShotSetJobLog::WhoAmI()
{
	// 固定値.
	return JSVE_Request_Class_Type_OneShot_Log;
}

// 受信データバッファの保存処理.
// 上位から渡されたデータをそのまま保持しておく.
Bool JSVC_RequestOneShotSetJobLog::setData(const char* pRcvData, short ss_size)
{
	
	Bool b_Result = True;

	// 動的確保領域をチェックして解放しておく.
	if ( this->vp_LogData )
	{
		free( this->vp_LogData );
		this->vp_LogData = NULL;
	}

	// MsgID毎に該当するAPIをコールする.
	// コマンドの対応一覧表を参照のこと.
	MyTraceSD( JSVD_DBG_INFO, "JSVC_RequestOneShotSetJobLog::setData MsgType:" , (int)getMsgTypeId() );	// warning対応 2013/09/18 ooba.
	switch ( getMsgTypeId() )
	{
	case JSVE_JobManager_MsgID_AddSubScenarioLog:				// サブシナリオ履歴登録要求.
		{
			IPCS_CompressSubScenarioLog& st_CompressInfo  = ((JSVS_ReqPara_AddSubScenarioLog*)(pRcvData))->st_CompressInfo;
			vp_LogData = malloc(sizeof(IPCS_CompressSubScenarioLog));
			if (vp_LogData)
			{
				memcpy(vp_LogData, &st_CompressInfo, sizeof(IPCS_CompressSubScenarioLog));
			}
			else
			{
				// メモリ確保失敗.
				b_Result = False;
			}
		}
		break;
	case JSVE_JobManager_MsgID_GetSendLog_DataBaseID:			// 送信履歴データベースID取得要求.
		{
			JSVS_ReqPara_GetSendLogDataBaseID& st_Param = *(JSVS_ReqPara_GetSendLogDataBaseID*)(pRcvData);
			ul_Param1 = st_Param.ul_ManageNumber;
			ul_Param2 = st_Param.ul_AddressNum;
		}
		break;
	case JSVE_JobManager_MsgID_GetSendLogAddressNum:			// サブシナリオ毎の送信先数取得要求.
		{
			JSVS_ReqPara_GetSendLogAddressNum& st_Param = *(JSVS_ReqPara_GetSendLogAddressNum*)(pRcvData);
			ul_Param1 = st_Param.ul_ManageNumber;
		}
		break;
	case JSVE_JobManager_MsgID_GetSendLogSubScenarioNo:			// サブシナリオ番号取得要求.
		{
			JSVS_ReqPara_GetSendLogManageNumber& st_Param = *(JSVS_ReqPara_GetSendLogManageNumber*)(pRcvData);
			ul_Param1 = st_Param.ul_SubScenarioID;
		}
		break;
	case JSVE_JobManager_MsgID_JobLog_AddApplication:			// ジョブログ登録要求（操作系）.
		{
			JSVS_ReqPara_AddJobLog_App& st_Param = *(JSVS_ReqPara_AddJobLog_App*)(pRcvData);
			ul_Param1 = st_Param.ul_Type;
			
			switch (ul_Param1)
			{
			case IPCE_AddOpeLog_Common:
				vp_LogData = malloc( sizeof(IPCS_JLG_Common) );
				if (vp_LogData)
				{
					memcpy(vp_LogData, &(st_Param.st_OpeLogCom), sizeof(IPCS_JLG_Common));
				}
				break;
			case IPCE_AddOpeLog_Manager:
				vp_LogData = malloc( sizeof(IPCS_JLG_Manager) );
				if (vp_LogData)
				{
					memcpy(vp_LogData, &(st_Param.st_OpeLogManager), sizeof(IPCS_JLG_Manager));
				}
				break;
			case IPCE_AddOpeLog_Box:
				vp_LogData = malloc( sizeof(IPCS_JLG_Box) );
				if (vp_LogData)
				{
					memcpy(vp_LogData, &(st_Param.st_OpeLogBox), sizeof(IPCS_JLG_Box));
				}
				break;
			case IPCE_AddOpeLog_UserAuth:
				vp_LogData = malloc( sizeof(IPCS_JLG_UserAuth) );
				if (vp_LogData)
				{
					memcpy(vp_LogData, &(st_Param.st_OpeLogUserAuth), sizeof(IPCS_JLG_UserAuth));
				}
				break;
			case IPCE_AddOpeLog_TrackAuth:
				vp_LogData = malloc( sizeof(IPCS_JLG_TrackAuth) );
				if (vp_LogData)
				{
					memcpy(vp_LogData, &(st_Param.st_OpeLogTrackAuth), sizeof(IPCS_JLG_TrackAuth));
				}
				break;
			default:
				// パラメータ異常.
				break;
			}
			
			if (!vp_LogData)
			{
				// メモリ確保失敗.
				b_Result = False;
			}
		}
		break;
	case JSVE_JobManager_MsgID_JobLog_AddPlatform:				// ジョブログ登録要求（P/Fから）.
		{
			JSVS_ReqPara_AddJobLog_APF& st_Param = *(JSVS_ReqPara_AddJobLog_APF*)(pRcvData);
			ul_Param1 = st_Param.ul_Type;
			
			switch (ul_Param1)
			{
			case IPCE_AddJLG_APF_ScenarioStart:
			case IPCE_AddJLG_APF_ScenarioEnd:
				{
					vp_LogData = malloc(sizeof(IPCS_JLG_Scenario));
					if (vp_LogData)
					{
						memcpy(vp_LogData, &(st_Param.st_Scenario), sizeof(IPCS_JLG_Scenario));
					}
				}
				break;
			case IPCE_AddJLG_APF_ServiceApp:
				{
					vp_LogData = malloc(sizeof(IPCS_JLG_ServiceApp));
					if (vp_LogData)
					{
						memcpy(vp_LogData, &(st_Param.st_ServiceApp), sizeof(IPCS_JLG_ServiceApp));
					}
				}
				break;
			default:
				// パラメータ異常.
				break;
			}
			
			if (!vp_LogData)
			{
				// メモリ確保失敗.
				b_Result = False;
			}
		}
		break;
	case JSVE_JobManager_MsgID_JobLog_SetComplete_getLogFile:	// ジョブログファイル取得完了設定要求.
		{
			JSVS_ReqPara_JobLogSetComplete_getLogFile& st_Param = *(JSVS_ReqPara_JobLogSetComplete_getLogFile*)(pRcvData);
			ul_Param1 = st_Param.ul_Param;
		}
		break;
	case JSVE_JobManager_MsgID_JobLog_GetWebDavLogFilePath:		// ジョブログパス取得要求（WebDav）.
		{
			JSVS_ReqPara_JobLogGetWebDavLogFilePath& st_Param = *(JSVS_ReqPara_JobLogGetWebDavLogFilePath*)(pRcvData);
			ul_Param1 = st_Param.ul_Connect;
		}
		break;
	default:													// その他.
		MyTraceSS(JSVD_DBG_INFO, "JSVC_RequestOneShotSetJobLog::setData", "No Request Parameters");
		break;
	}

	return b_Result;
}

// リクエスト処理.
JSVE_Request_Result JSVC_RequestOneShotSetJobLog::startAction(JSVS_NotifyInfo* pNotify)
{
	JSVC_ResponseOneShot_Log c_Response(pNotify->e_JobManager_MsgTypeID);

	JSVS_ResponseParam_OneShotLog st_ResParam;
	memset(&st_ResParam, 0x00, sizeof(JSVS_ResponseParam_OneShotLog));

	JSVS_SendRecv_MsgParam st_Msg;
	memset(&st_Msg, 0x00, sizeof(JSVS_SendRecv_MsgParam));

	JSVE_Request_Result e_Request_Result = JSVE_Request_Result_NotSupport;
	switch (pNotify->pReq->getMsgTypeId())
	{
	case JSVE_JobManager_MsgID_AddSubScenarioLog:				// サブシナリオ履歴登録要求.
		{
			// パラメータ生成.
			IPCS_CompressSubScenarioLog& st_CompressInfo = *(IPCS_CompressSubScenarioLog*)(this->vp_LogData);
			
			CompressJobLog st_CompressJobLog;
			st_CompressJobLog.ul_JobID = st_CompressInfo.ul_JobID;
			st_CompressJobLog.b_joblogdisp = st_CompressInfo.b_joblogdisp;
			st_CompressJobLog.e_SendMode = (TYPE_SendMode) (0xFF & st_CompressInfo.e_SendMode);
			st_CompressJobLog.e_OutputTray = (TYPE_OutputTray)(0xFF & st_CompressInfo.e_OutputTray);
			st_CompressJobLog.e_JobResult = (TYPE_JobResult) (0xFF & st_CompressInfo.e_JobResult);
			st_CompressJobLog.ul_TransactionID = st_CompressInfo.ul_TransactionID;
			st_CompressJobLog.sl_NumOfCopies = st_CompressInfo.sl_NumOfCopies;
			st_CompressJobLog.c_StartTime.ss_Year = st_CompressInfo.st_StartTime.ss_Year;
			st_CompressJobLog.c_StartTime.sc_Month = st_CompressInfo.st_StartTime.sc_Month;
			st_CompressJobLog.c_StartTime.sc_Day = st_CompressInfo.st_StartTime.sc_Day;
			st_CompressJobLog.c_StartTime.sc_Hour = st_CompressInfo.st_StartTime.sc_Hour;
			st_CompressJobLog.c_StartTime.sc_Minute = st_CompressInfo.st_StartTime.sc_Minute;
			st_CompressJobLog.c_StartTime.sc_Second = st_CompressInfo.st_StartTime.sc_Second;
			st_CompressJobLog.c_EndTime.ss_Year = st_CompressInfo.st_EndTime.ss_Year;
			st_CompressJobLog.c_EndTime.sc_Month = st_CompressInfo.st_EndTime.sc_Month;
			st_CompressJobLog.c_EndTime.sc_Day = st_CompressInfo.st_EndTime.sc_Day;
			st_CompressJobLog.c_EndTime.sc_Hour = st_CompressInfo.st_EndTime.sc_Hour;
			st_CompressJobLog.c_EndTime.sc_Minute = st_CompressInfo.st_EndTime.sc_Minute;
			st_CompressJobLog.c_EndTime.sc_Second = st_CompressInfo.st_EndTime.sc_Second;
			strncpy( st_CompressJobLog.sc_UserName, st_CompressInfo.sc_UserName, strlen(st_CompressInfo.sc_UserName) );
			st_CompressJobLog.e_JobType = (TYPE_JobType)(0xFF & st_CompressInfo.e_JobType);
			st_CompressJobLog.ss_NumOfOriginals = st_CompressInfo.ss_NumOfOriginals;
			strncpy( st_CompressJobLog.sc_FileName, st_CompressInfo.sc_FileName, strlen(st_CompressInfo.sc_FileName));
			st_CompressJobLog.e_DeleteJobCause = (TYPE_DeleteJobCause)(0xFF & st_CompressInfo.e_DeleteJobCause);
			st_CompressJobLog.us_ListTab = st_CompressInfo.us_ListTab;
			strncpy( st_CompressJobLog.uc_DestName, st_CompressInfo.uc_DestName, strlen(st_CompressInfo.uc_DestName));
			st_CompressJobLog.e_JobForceEndCause = (TYPE_JobForceEndCause)(0xFF & st_CompressInfo.e_JobForceEndCause);
			st_CompressJobLog.c_FaxTransTime = TYPC_HourMinute(st_CompressInfo.s_FaxTransTime.sc_Hour, st_CompressInfo.s_FaxTransTime.sc_Minute);
			st_CompressJobLog.sc_FaxTransSecond = st_CompressInfo.sc_FaxTransSecond;
			memcpy( st_CompressJobLog.uc_DriverJobID, st_CompressInfo.uc_DriverJobID, sizeof(uchar)*26);
			st_CompressJobLog.sl_NumOfCurrentTotalCopies = st_CompressInfo.sl_NumOfCurrentTotalCopies;
			st_CompressJobLog.sl_NumOfCurrentTotalPages = st_CompressInfo.sl_NumOfCurrentTotalPages;
			st_CompressJobLog.sl_UserNo = st_CompressInfo.sl_UserNo;	// 2013/09/10 【US-20U】20万人対応.
			st_CompressJobLog.sl_TrackNo = st_CompressInfo.sl_TrackNo;	// 2013/09/10 【US-20U】20万人対応.
			strncpy( st_CompressJobLog.sc_ServerName, st_CompressInfo.sc_ServerName, strlen(st_CompressInfo.sc_ServerName));
			st_CompressJobLog.sl_MidAuthId = st_CompressInfo.sl_MidAuthId;
			st_CompressJobLog.e_UniversalAddressType = (TYPE_UniversalAddressType)(0xFF & st_CompressInfo.e_UniversalAddressType);
			// [changelog] 2018/01/15 Cuong #245356 Fix static check for IT5_4.2 F00-U2(07) start.
			st_CompressJobLog.sc_UserType = static_cast<TYPE_UserType> (st_CompressInfo.sc_UserType);
			// [changelog] 2018/01/15 Cuong ##245356 Fix static check for IT5_4.2 F00-U2(07) end.
			st_CompressJobLog.b_AddressFlg = st_CompressInfo.b_AddressFlg;
			st_CompressJobLog.NetworkTroubleCode = st_CompressInfo.NetworkTroubleCode;
			st_CompressJobLog.b_MultiBoxFlg = st_CompressInfo.b_MultiBoxFlg;
			st_CompressJobLog.ul_ErrorPageNum = st_CompressInfo.ul_ErrorPageNum;
			st_CompressJobLog.ul_FunctionType = st_CompressInfo.ul_FunctionType;
			st_CompressJobLog.ul_ScenarioID = st_CompressInfo.ul_ScenarioID;
			st_CompressJobLog.ul_SubScenarioID = st_CompressInfo.ul_SubScenarioID;
			st_CompressJobLog.ul_BoxId = st_CompressInfo.ul_BoxId;
			st_CompressJobLog.ul_DocId = st_CompressInfo.ul_DocId;
			st_CompressJobLog.b_InputMultiDocFlg = st_CompressInfo.b_InputMultiDocFlg;
			strncpy( st_CompressJobLog.sc_InputFileName, st_CompressInfo.sc_InputFileName, strlen(st_CompressInfo.sc_InputFileName) );
			st_CompressJobLog.b_InputMultiFileFlg = st_CompressInfo.b_InputMultiFileFlg;
			st_CompressJobLog.e_PreviewMode = (TYPE_PreviewMode)(0xFF & st_CompressInfo.e_PreviewMode);
			st_CompressJobLog.e_ImageConvertKind = (TYPE_ImageConvertKind)(0xFF & st_CompressInfo.e_ImageConvertKind);
			strncpy( st_CompressJobLog.sc_ServiceName, st_CompressInfo.sc_ServiceName, strlen(st_CompressInfo.sc_ServiceName) );
			
			// APIコール.
			APIC_DoneJobList::newInstance()->addJobLogScenario( &st_CompressJobLog );
			st_ResParam.ul_Result = IPCE_JobServiceResult_OK;
			c_Response.setData((const char*)&st_ResParam, sizeof(st_ResParam));
			e_Request_Result = JSVE_Request_Result_Complete;
		}
		break;
	case JSVE_JobManager_MsgID_GetSubScenarioLog_DataBaseID:	// サブシナリオ履歴データベースID取得要求.
		{
			// パラメータ生成.
			Bool b_ApiResult = False;
			SYSS_Config st_Config;
			
			// APIコール.
			b_ApiResult = APIC_DoneJobList::newInstance()->getSubScenarioDataBaseID(&st_Config);
			
			// 結果セット.
			if (b_ApiResult)
			{
				st_ResParam.ul_Result = IPCE_JobServiceResult_OK;
			}
			else
			{
				st_ResParam.ul_Result = IPCE_JobServiceResult_NG;
			}
			// [changelog] 2018/01/15 Dat #245273 Fix static check for IT5_4.2 F00-U2(07) start.
			if (st_Config.UseFile != 0)
		   	{
			   st_ResParam.st_DBConfig.b_UseFile = True;
		   	}
		   	else
		   	{
			   st_ResParam.st_DBConfig.b_UseFile = False;
		   	}
			// [changelog] 2018/01/15 Dat #245273 Fix static check for IT5_4.2 F00-U2(07) end.
			st_ResParam.st_DBConfig.si_DbNum = st_Config.DbNum;
			strncpy(st_ResParam.st_DBConfig.sc_FilePath, st_Config.FilePath, 255);
			st_ResParam.st_DBConfig.si_NumOfCache = st_Config.NumOfCache;
			st_ResParam.st_DBConfig.si_RecordBytes = st_Config.RecordBytes;
			st_ResParam.st_DBConfig.ul_MaxKey = st_Config.MaxKey;
			st_ResParam.st_DBConfig.sc_KeySorting = st_Config.KeySorting;
			st_ResParam.st_DBConfig.sc_ExclusiveKey = st_Config.ExclusiveKey;
			st_ResParam.st_DBConfig.st_NvRamConf.b_UseNvRam = st_Config.NvRamConf.UseNvRam;
			st_ResParam.st_DBConfig.st_NvRamConf.b_PackedHeader = st_Config.NvRamConf.PackedHeader;
			st_ResParam.st_DBConfig.st_NvRamConf.ui_Offset = st_Config.NvRamConf.Offset;
			st_ResParam.st_DBConfig.st_NvRamConf.ui_DbSize = st_Config.NvRamConf.DbSize;
			
			c_Response.setData((const char*)&st_ResParam, sizeof(st_ResParam));
			e_Request_Result = JSVE_Request_Result_Complete;
		}
		break;
	case JSVE_JobManager_MsgID_GetFaxJournal_DataBaseID:		// FaxジャーナルデータベースID取得要求.
		{
			// パラメータ生成.
			Bool b_ApiResult = False;
			SYSS_Config st_Config;
			
			// APIコール.
			b_ApiResult = APIC_DoneJobList::newInstance()->getFaxJournalDataBaseID(&st_Config);
			
			// 結果セット.
			if (b_ApiResult)
			{
				st_ResParam.ul_Result = IPCE_JobServiceResult_OK;
			}
			else
			{
				st_ResParam.ul_Result = IPCE_JobServiceResult_NG;
			}
			// [changelog] 2018/01/15 Dat #245273 Fix static check for IT5_4.2 F00-U2(07) start.
			if (st_Config.UseFile != 0)
		   	{
			   st_ResParam.st_DBConfig.b_UseFile = True;
		   	}
		   	else
		   	{
			   st_ResParam.st_DBConfig.b_UseFile = False;
		   	}
			// [changelog] 2018/01/15 Dat #245273 Fix static check for IT5_4.2 F00-U2(07) end.
			st_ResParam.st_DBConfig.si_DbNum = st_Config.DbNum;
			strncpy(st_ResParam.st_DBConfig.sc_FilePath, st_Config.FilePath, 255);
			st_ResParam.st_DBConfig.si_NumOfCache = st_Config.NumOfCache;
			st_ResParam.st_DBConfig.si_RecordBytes = st_Config.RecordBytes;
			st_ResParam.st_DBConfig.ul_MaxKey = st_Config.MaxKey;
			st_ResParam.st_DBConfig.sc_KeySorting = st_Config.KeySorting;
			st_ResParam.st_DBConfig.sc_ExclusiveKey = st_Config.ExclusiveKey;
			st_ResParam.st_DBConfig.st_NvRamConf.b_UseNvRam = st_Config.NvRamConf.UseNvRam;
			st_ResParam.st_DBConfig.st_NvRamConf.b_PackedHeader = st_Config.NvRamConf.PackedHeader;
			st_ResParam.st_DBConfig.st_NvRamConf.ui_Offset = st_Config.NvRamConf.Offset;
			st_ResParam.st_DBConfig.st_NvRamConf.ui_DbSize = st_Config.NvRamConf.DbSize;
			
			c_Response.setData((const char*)&st_ResParam, sizeof(st_ResParam));
			e_Request_Result = JSVE_Request_Result_Complete;
		}
		break;
	case JSVE_JobManager_MsgID_GetFaxJournal_RegNum:			// Faxジャーナル登録数取得要求.
		{
			int si_JournalRegNum = APIC_DoneJobList::newInstance()->getFaxJournalRegNum();
			st_ResParam.ul_Result = IPCE_JobServiceResult_OK;
			st_ResParam.ul_Param1 = si_JournalRegNum;
			c_Response.setData((const char*)&st_ResParam, sizeof(st_ResParam));
			e_Request_Result = JSVE_Request_Result_Complete;
		}
		break;
	case JSVE_JobManager_MsgID_GetScanJournal_DataBaseID:		// ScanジャーナルデータベースID取得要求.
		{
			// パラメータ生成.
			Bool b_ApiResult = False;
			SYSS_Config st_Config;
			
			// APIコール.
			b_ApiResult = APIC_DoneJobList::newInstance()->getScanJournalDataBaseID(&st_Config);
			
			// 結果セット.
			if (b_ApiResult)
			{
				st_ResParam.ul_Result = IPCE_JobServiceResult_OK;
			}
			else
			{
				st_ResParam.ul_Result = IPCE_JobServiceResult_NG;
			}
			// [changelog] 2018/01/15 Dat #245273 Fix static check for IT5_4.2 F00-U2(07) start.
			if (st_Config.UseFile != 0)
		   	{
			   st_ResParam.st_DBConfig.b_UseFile = True;
		   	}
		   	else
		   	{
			   st_ResParam.st_DBConfig.b_UseFile = False;
		   	}
			// [changelog] 2018/01/15 Dat #245273 Fix static check for IT5_4.2 F00-U2(07) end.
			st_ResParam.st_DBConfig.si_DbNum = st_Config.DbNum;
			strncpy(st_ResParam.st_DBConfig.sc_FilePath, st_Config.FilePath, 255);
			st_ResParam.st_DBConfig.si_NumOfCache = st_Config.NumOfCache;
			st_ResParam.st_DBConfig.si_RecordBytes = st_Config.RecordBytes;
			st_ResParam.st_DBConfig.ul_MaxKey = st_Config.MaxKey;
			st_ResParam.st_DBConfig.sc_KeySorting = st_Config.KeySorting;
			st_ResParam.st_DBConfig.sc_ExclusiveKey = st_Config.ExclusiveKey;
			st_ResParam.st_DBConfig.st_NvRamConf.b_UseNvRam = st_Config.NvRamConf.UseNvRam;
			st_ResParam.st_DBConfig.st_NvRamConf.b_PackedHeader = st_Config.NvRamConf.PackedHeader;
			st_ResParam.st_DBConfig.st_NvRamConf.ui_Offset = st_Config.NvRamConf.Offset;
			st_ResParam.st_DBConfig.st_NvRamConf.ui_DbSize = st_Config.NvRamConf.DbSize;
			
			c_Response.setData((const char*)&st_ResParam, sizeof(st_ResParam));
			e_Request_Result = JSVE_Request_Result_Complete;
		}
		break;
	case JSVE_JobManager_MsgID_GetScanJournal_RegNum:			// Scanジャーナル登録数取得要求.
		{
			int si_Value = APIC_DoneJobList::newInstance()->getScanJournalRegNum();
			st_ResParam.ul_Result = IPCE_JobServiceResult_OK;
			st_ResParam.ul_Param1 = si_Value;
			c_Response.setData((const char*)&st_ResParam, sizeof(st_ResParam));
			e_Request_Result = JSVE_Request_Result_Complete;
		}
		break;
	case JSVE_JobManager_MsgID_GetReceiveRef_DataBaseID:		// 受信履歴データベースID取得要求.
		{
			// パラメータ生成.
			Bool b_ApiResult = False;
			SYSS_Config st_Config;
			
			// APIコール.
			b_ApiResult = APIC_DoneJobList::newInstance()->getReceiveRefDataBaseID(&st_Config);
			
			// 結果セット.
			if (b_ApiResult)
			{
				st_ResParam.ul_Result = IPCE_JobServiceResult_OK;
			}
			else
			{
				st_ResParam.ul_Result = IPCE_JobServiceResult_NG;
			}
			// [changelog] 2018/01/15 Dat #245273 Fix static check for IT5_4.2 F00-U2(07) start.
			if (st_Config.UseFile != 0)
		   	{
			   st_ResParam.st_DBConfig.b_UseFile = True;
		   	}
		   	else
		   	{
			   st_ResParam.st_DBConfig.b_UseFile = False;
		   	}
			// [changelog] 2018/01/15 Dat #245273 Fix static check for IT5_4.2 F00-U2(07) end.
			st_ResParam.st_DBConfig.si_DbNum = st_Config.DbNum;
			strncpy(st_ResParam.st_DBConfig.sc_FilePath, st_Config.FilePath, 255);
			st_ResParam.st_DBConfig.si_NumOfCache = st_Config.NumOfCache;
			st_ResParam.st_DBConfig.si_RecordBytes = st_Config.RecordBytes;
			st_ResParam.st_DBConfig.ul_MaxKey = st_Config.MaxKey;
			st_ResParam.st_DBConfig.sc_KeySorting = st_Config.KeySorting;
			st_ResParam.st_DBConfig.sc_ExclusiveKey = st_Config.ExclusiveKey;
			st_ResParam.st_DBConfig.st_NvRamConf.b_UseNvRam = st_Config.NvRamConf.UseNvRam;
			st_ResParam.st_DBConfig.st_NvRamConf.b_PackedHeader = st_Config.NvRamConf.PackedHeader;
			st_ResParam.st_DBConfig.st_NvRamConf.ui_Offset = st_Config.NvRamConf.Offset;
			st_ResParam.st_DBConfig.st_NvRamConf.ui_DbSize = st_Config.NvRamConf.DbSize;
			
			c_Response.setData((const char*)&st_ResParam, sizeof(st_ResParam));
			e_Request_Result = JSVE_Request_Result_Complete;
		}
		break;
	case JSVE_JobManager_MsgID_GetSendLog_DataBaseID:			// 送信履歴データベースID取得要求.
		{
			Bool b_Result = APIC_DoneJobList::newInstance()->getSendLogDataBaseID(static_cast<short>(this->ul_Param1), static_cast<short>(this->ul_Param2), st_ResParam.sc_FilePath);
			if (b_Result)
			{
				st_ResParam.ul_Result = IPCE_JobServiceResult_OK;
			}
			else
			{
				st_ResParam.ul_Result = IPCE_JobServiceResult_NG;
			}
			c_Response.setData((const char*)&st_ResParam, sizeof(st_ResParam));
			e_Request_Result = JSVE_Request_Result_Complete;
		}
		break;
	case JSVE_JobManager_MsgID_GetSendLogSubScenarioNum:		// 送信履歴登録サブシナリオ数取得要求.
		{
			short ss_Value = APIC_DoneJobList::newInstance()->getSendLogJobNum();
			st_ResParam.ul_Result = IPCE_JobServiceResult_OK;
			st_ResParam.ul_Param1 = ss_Value;
			c_Response.setData((const char*)&st_ResParam, sizeof(st_ResParam));
			e_Request_Result = JSVE_Request_Result_Complete;
		}
		break;
	case JSVE_JobManager_MsgID_StartSendLogAccess:				// 送信履歴の取得アクセス開始要求.
		{
			Bool b_Value = APIC_DoneJobList::newInstance()->startSendLogAccess();
			st_ResParam.ul_Result = IPCE_JobServiceResult_OK;
			st_ResParam.ul_Param1 = b_Value;
			c_Response.setData((const char*)&st_ResParam, sizeof(st_ResParam));
			e_Request_Result = JSVE_Request_Result_Complete;
		}
		break;
	case JSVE_JobManager_MsgID_EndSendLogAccess:				// 送信履歴の取得アクセス終了要求.
		{
			APIC_DoneJobList::newInstance()->endSendLogAccess();
			st_ResParam.ul_Result = IPCE_JobServiceResult_OK;
			c_Response.setData((const char*)&st_ResParam, sizeof(st_ResParam));
			e_Request_Result = JSVE_Request_Result_Complete;
		}
		break;
	case JSVE_JobManager_MsgID_GetSendLogAddressNum:			// サブシナリオ毎の送信先数取得要求.
		{
			// [changelog] 2018/01/15 Dat #245306 Fix static check for IT5_4.2 F00-U2(07) start.
			short ss_Value = APIC_DoneJobList::newInstance()->getSendLogAddressNum( static_cast<short>(this->ul_Param1) );
			// [changelog] 2018/01/15 Dat #245306 Fix static check for IT5_4.2 F00-U2(07) end.
			st_ResParam.ul_Result = IPCE_JobServiceResult_OK;
			st_ResParam.ul_Param1 = ss_Value;
			c_Response.setData((const char*)&st_ResParam, sizeof(st_ResParam));
			e_Request_Result = JSVE_Request_Result_Complete;
		}
		break;
	case JSVE_JobManager_MsgID_GetSendLogSubScenarioNo:			// サブシナリオ番号取得要求.
		{
			// APIコール.
			short ss_SubScenarioNo = APIC_DoneJobList::newInstance()->getSendLogJobNo(this->ul_Param1);
			st_ResParam.ul_Result = IPCE_JobServiceResult_OK;
			st_ResParam.ul_Param1 = ss_SubScenarioNo;
			c_Response.setData((const char*)&st_ResParam, sizeof(st_ResParam));
			e_Request_Result = JSVE_Request_Result_Complete;
		}
		break;
	case JSVE_JobManager_MsgID_JobLog_AddApplication:			// ジョブログ登録要求（操作系）.
		{
			// パラメータ生成.
			JLGE_Result ul_Result = JLGE_Result_Err;
			
			// 種別範囲チェック.
			if ((IPCE_AddOpeLog_Common <= this->ul_Param1) && (this->ul_Param1 <= IPCE_AddOpeLog_TrackAuth))
			{
				IPCS_JLG_Common* ps_Param = (IPCS_JLG_Common*)(this->vp_LogData);
				
				// Commonデータは共通の為、予め生成しておく.
				// 共用体の先頭部はCommonになっている前提.
				JLGS_OpeLogCom st_OpeLogCommon;
				st_OpeLogCommon.ul_Code = ps_Param->st_OpeLogCommon.ul_Code;
				st_OpeLogCommon.s_Time.ss_Year = ps_Param->st_OpeLogCommon.s_Time.ss_Year;
				st_OpeLogCommon.s_Time.sc_Month = ps_Param->st_OpeLogCommon.s_Time.sc_Month;
				st_OpeLogCommon.s_Time.sc_Day = ps_Param->st_OpeLogCommon.s_Time.sc_Day;
				st_OpeLogCommon.s_Time.sc_Hour = ps_Param->st_OpeLogCommon.s_Time.sc_Hour;
				st_OpeLogCommon.s_Time.sc_Minute = ps_Param->st_OpeLogCommon.s_Time.sc_Minute;
				st_OpeLogCommon.s_Time.sc_Second = ps_Param->st_OpeLogCommon.s_Time.sc_Second;
				st_OpeLogCommon.s_Time.sc_Reserve = ps_Param->st_OpeLogCommon.s_Time.sc_Reserve;
				st_OpeLogCommon.us_Result = ps_Param->st_OpeLogCommon.us_Result;
// 2016/02/01 CQ521285 監査ログHCD-PP対応	Sky平田	Start.
				st_OpeLogCommon.us_EventType = getEventTypeByResultCode( st_OpeLogCommon.us_Result );
// 2016/02/01 CQ521285 監査ログHCD-PP対応	Sky平田	End.
				st_OpeLogCommon.ul_OpeCode = ps_Param->st_OpeLogCommon.ul_OpeCode;
				strncpy(st_OpeLogCommon.sc_Name, ps_Param->st_OpeLogCommon.sc_Name, strlen(ps_Param->st_OpeLogCommon.sc_Name));
				strncpy(st_OpeLogCommon.sc_ExtServerName, ps_Param->st_OpeLogCommon.sc_ExtServerName, strlen(ps_Param->st_OpeLogCommon.sc_ExtServerName));
				st_OpeLogCommon.ul_TrackCode = ps_Param->st_OpeLogCommon.ul_TrackCode;
				strncpy(st_OpeLogCommon.sc_TrackName, ps_Param->st_OpeLogCommon.sc_TrackName, strlen(ps_Param->st_OpeLogCommon.sc_TrackName));
				memcpy(st_OpeLogCommon.uc_CardID, ps_Param->st_OpeLogCommon.uc_CardID, JLGD_DataLenCardIDOpeLog+1);
				st_OpeLogCommon.uc_IF = (JLGE_InspectLogIF)(0xFF & ps_Param->st_OpeLogCommon.uc_IF);
				strncpy(st_OpeLogCommon.sc_ClientInfo, ps_Param->st_OpeLogCommon.sc_ClientInfo, strlen(ps_Param->st_OpeLogCommon.sc_ClientInfo));

				// 各種別毎のパラメータの生成とAPIコール.
				// ここでは先の範囲チェックにより、default発生がありえないので、コンパイル時テーブル高速化を期待してdefaultケースは記載しない.
				switch (this->ul_Param1)
				{
				case IPCE_AddOpeLog_Common:						// 共通操作.
					{
						// APIコール.
						ul_Result = APIC_JLG::newInstance()->addOpeLog(&st_OpeLogCommon);
					}
					break;
				case IPCE_AddOpeLog_Manager:					// 管理者設定操作.
					{
						// 種別パラメータ生成.
						IPCS_JLG_Manager* ps_Param = (IPCS_JLG_Manager*)(this->vp_LogData);
						JLGS_OpeLogContent st_OpeLogContent;
						st_OpeLogContent.uc_OperationContent = (JLGE_InspectLogOperatCont)(0xFF & ps_Param->st_OpeLogContent.uc_OperationContent);
						
						// APIコール.
						ul_Result = APIC_JLG::newInstance()->addOpeLogManager(&st_OpeLogCommon, &st_OpeLogContent);
					}
					break;
				case IPCE_AddOpeLog_Box:						// ボックス内操作.
					{
						// 種別パラメータ生成.
						IPCS_JLG_Box* ps_Param = (IPCS_JLG_Box*)(this->vp_LogData);
						
						JLGS_OpeLogBox st_OpeLogBox;
						st_OpeLogBox.sl_TargetBoxNo = ps_Param->st_OpeLogBox.sl_TargetBoxNo;
						strncpy(st_OpeLogBox.sc_TargetDocumentName, ps_Param->st_OpeLogBox.sc_TargetDocumentName, strlen(ps_Param->st_OpeLogBox.sc_TargetDocumentName));
						strncpy(st_OpeLogBox.sc_AfterDocumentName, ps_Param->st_OpeLogBox.sc_AfterDocumentName, strlen(ps_Param->st_OpeLogBox.sc_AfterDocumentName));
						st_OpeLogBox.sl_MoveBoxNo = ps_Param->st_OpeLogBox.sl_MoveBoxNo;
						st_OpeLogBox.sl_CopyBoxNo = ps_Param->st_OpeLogBox.sl_CopyBoxNo;
						memcpy(st_OpeLogBox.sc_SecretID, ps_Param->st_OpeLogBox.sc_SecretID, JLGD_DataLenSecretID);
						
						// APIコール.
						ul_Result = APIC_JLG::newInstance()->addOpeLogBox(&st_OpeLogCommon, &st_OpeLogBox);
					}
					break;
				case IPCE_AddOpeLog_UserAuth:					// ユーザー認証設定操作.
					{
						// 種別パラメータ生成.
						IPCS_JLG_UserAuth* ps_Param = (IPCS_JLG_UserAuth*)(this->vp_LogData);
						
						JLGS_OpeLogUserAuth st_OpeLogUserAuth;
						st_OpeLogUserAuth.ul_UserID = ps_Param->st_OpeLogUserAuth.ul_UserID;
						strncpy(st_OpeLogUserAuth.sc_UserName, ps_Param->st_OpeLogUserAuth.sc_UserName, strlen(ps_Param->st_OpeLogUserAuth.sc_UserName));
						strncpy(st_OpeLogUserAuth.sc_RelationExtServerName, ps_Param->st_OpeLogUserAuth.sc_RelationExtServerName, strlen(ps_Param->st_OpeLogUserAuth.sc_RelationExtServerName));
						
						// APIコール.
						ul_Result = APIC_JLG::newInstance()->addOpeLogUserAuthentication(&st_OpeLogCommon, &st_OpeLogUserAuth);
					}
					break;
				case IPCE_AddOpeLog_TrackAuth:					// 部門認証設定操作.
					{
						// 種別パラメータ生成.
						IPCS_JLG_TrackAuth* ps_Param = (IPCS_JLG_TrackAuth*)(this->vp_LogData);
						
						JLGS_OpeLogTrackAuth st_OpeLogTrackAuth;
						st_OpeLogTrackAuth.ul_TrackCode = ps_Param->st_OpeLogTrackAuth.ul_TrackCode;
						strncpy(st_OpeLogTrackAuth.sc_TrackName, ps_Param->st_OpeLogTrackAuth.sc_TrackName, strlen(ps_Param->st_OpeLogTrackAuth.sc_TrackName));
						
						// APIコール.
						ul_Result = APIC_JLG::newInstance()->addOpeLogTrackAuthentication(&st_OpeLogCommon, &st_OpeLogTrackAuth);
					}
					break;
				}
			}
			else
			{
				// 種別範囲外.
			}

			if (ul_Result == JLGE_Result_Ok)
			{
				st_ResParam.ul_Result = IPCE_JobServiceResult_OK;
				st_ResParam.ul_Param1 = this->ul_Param1;
			}
			else
			{
				st_ResParam.ul_Result = IPCE_JobServiceResult_NG;
			}
			c_Response.setData((const char*)&st_ResParam, sizeof(st_ResParam));
			e_Request_Result = JSVE_Request_Result_Complete;
		}
		break;
	case JSVE_JobManager_MsgID_JobLog_AddPlatform:				// ジョブログ登録要求（P/Fから）.
		{
			// パラメータ生成.
			JLGE_Result ul_Result = JLGE_Result_Err;

			// 種別毎パラメータ生成とAPIコール.
			if ((this->ul_Param1 == IPCE_AddJLG_APF_ScenarioStart) ||
				(this->ul_Param1 == IPCE_AddJLG_APF_ScenarioEnd)    )
			{
				IPCS_JLG_Scenario* ps_Param = (IPCS_JLG_Scenario*)vp_LogData;
				
				JLGS_JobLogScenarioInfo st_ScenarioInfo;
				st_ScenarioInfo.ul_No = ps_Param->ul_No;
				strncpy(st_ScenarioInfo.sc_Nam, ps_Param->sc_Name, strlen(ps_Param->sc_Name));
				st_ScenarioInfo.s_EntTim.ss_Year  = ps_Param->s_EntTim.ss_Year;
				st_ScenarioInfo.s_EntTim.sc_Month = ps_Param->s_EntTim.sc_Month;
				st_ScenarioInfo.s_EntTim.sc_Day = ps_Param->s_EntTim.sc_Day;
				st_ScenarioInfo.s_EntTim.sc_Hour = ps_Param->s_EntTim.sc_Hour;
				st_ScenarioInfo.s_EntTim.sc_Minute = ps_Param->s_EntTim.sc_Minute;
				st_ScenarioInfo.s_EntTim.sc_Second = ps_Param->s_EntTim.sc_Second;
				st_ScenarioInfo.s_EntTim.sc_Reserve = ps_Param->s_EntTim.sc_Reserve;
				st_ScenarioInfo.s_FinTim.ss_Year = ps_Param->s_FinTim.ss_Year;
				st_ScenarioInfo.s_FinTim.sc_Month = ps_Param->s_FinTim.sc_Month;
				st_ScenarioInfo.s_FinTim.sc_Day = ps_Param->s_FinTim.sc_Day;
				st_ScenarioInfo.s_FinTim.sc_Hour = ps_Param->s_FinTim.sc_Hour;
				st_ScenarioInfo.s_FinTim.sc_Minute = ps_Param->s_FinTim.sc_Minute;
				st_ScenarioInfo.s_FinTim.sc_Second = ps_Param->s_FinTim.sc_Second;
				st_ScenarioInfo.s_FinTim.sc_Reserve = ps_Param->s_FinTim.sc_Reserve;
				st_ScenarioInfo.ul_SubID = ps_Param->ul_SubID;
				st_ScenarioInfo.ul_ServiceID = ps_Param->ul_ServiceID;
				strncpy(st_ScenarioInfo.sc_ServiceName, ps_Param->sc_ServiceName, strlen(ps_Param->sc_ServiceName));
				st_ScenarioInfo.us_Result = ps_Param->us_Result;
				
				if (this->ul_Param1 == IPCE_AddJLG_APF_ScenarioStart)
				{
					ul_Result = APIC_JLG::newInstance()->addJLG_JobScenarioStart(&st_ScenarioInfo);
				}
				else
				{
					ul_Result = APIC_JLG::newInstance()->addJLG_JobScenarioEnd(&st_ScenarioInfo);
				}
			}
			else if (this->ul_Param1 == IPCE_AddJLG_APF_ServiceApp)
			{
				IPCS_JLG_ServiceApp* ps_Param = (IPCS_JLG_ServiceApp*)vp_LogData;
				
				JLGS_JobLogServiceAppInfo st_AppInfo;
				st_AppInfo.ul_No = ps_Param->ul_No;
				strncpy(st_AppInfo.sc_AppName, ps_Param->sc_AppName, strlen(ps_Param->sc_AppName));
				st_AppInfo.s_EntTim.ss_Year  = ps_Param->s_EntTim.ss_Year;
				st_AppInfo.s_EntTim.sc_Month = ps_Param->s_EntTim.sc_Month;
				st_AppInfo.s_EntTim.sc_Day = ps_Param->s_EntTim.sc_Day;
				st_AppInfo.s_EntTim.sc_Hour = ps_Param->s_EntTim.sc_Hour;
				st_AppInfo.s_EntTim.sc_Minute = ps_Param->s_EntTim.sc_Minute;
				st_AppInfo.s_EntTim.sc_Second = ps_Param->s_EntTim.sc_Second;
				st_AppInfo.s_EntTim.sc_Reserve = ps_Param->s_EntTim.sc_Reserve;
				st_AppInfo.s_FinTim.ss_Year = ps_Param->s_FinTim.ss_Year;
				st_AppInfo.s_FinTim.sc_Month = ps_Param->s_FinTim.sc_Month;
				st_AppInfo.s_FinTim.sc_Day = ps_Param->s_FinTim.sc_Day;
				st_AppInfo.s_FinTim.sc_Hour = ps_Param->s_FinTim.sc_Hour;
				st_AppInfo.s_FinTim.sc_Minute = ps_Param->s_FinTim.sc_Minute;
				st_AppInfo.s_FinTim.sc_Second = ps_Param->s_FinTim.sc_Second;
				st_AppInfo.s_FinTim.sc_Reserve = ps_Param->s_FinTim.sc_Reserve;
				st_AppInfo.us_Result = ps_Param->us_Result;
				
				ul_Result = APIC_JLG::newInstance()->addJLG_ServiceAppInfo(&st_AppInfo);
			}
			else
			{
				// 種別範囲外.
			}

			if (ul_Result == JLGE_Result_Ok)
			{
				st_ResParam.ul_Result = IPCE_JobServiceResult_OK;
				st_ResParam.ul_Param1 = this->ul_Param1;
			}
			else
			{
				st_ResParam.ul_Result = IPCE_JobServiceResult_NG;
			}
			c_Response.setData((const char*)&st_ResParam, sizeof(st_ResParam));
			e_Request_Result = JSVE_Request_Result_Complete;
		}
		break;
	case JSVE_JobManager_MsgID_JobLog_SetComplete_getLogFile:	// ジョブログファイル取得完了設定要求.
		{
			// パラメータ生成.
			uint32_t ul_Param = this->ul_Param1;

			// APIコール.
// 2016/02/01 CQ521285 監査ログ HCD-PP対応 Sky大嶋 MOD Start.
			JLGE_Result e_Result = APIC_JLG::newInstance()->setComplete_getLogFile( (Bool) ul_Param, JLGE_LogComplete_Service );
// 2016/02/01 CQ521285 監査ログ HCD-PP対応 Sky大嶋 MOD End.
			if (e_Result == JLGE_Result_Ok)
			{
				st_ResParam.ul_Result = IPCE_JobServiceResult_OK;
			}
			else
			{
				st_ResParam.ul_Result = IPCE_JobServiceResult_NG;
			}
			c_Response.setData((const char*)&st_ResParam, sizeof(st_ResParam));
			e_Request_Result = JSVE_Request_Result_Complete;
		}
		break;
	case JSVE_JobManager_MsgID_JobLog_GetComplete_getLogFile:	// ジョブログファイル取得完了取得要求.
		{
			Bool b_Value = APIC_JLG::newInstance()->getComplete_getLogFile();
			st_ResParam.ul_Result = IPCE_JobServiceResult_OK;
			st_ResParam.ul_Param1 = b_Value;
			c_Response.setData((const char*)&st_ResParam, sizeof(st_ResParam));
			e_Request_Result = JSVE_Request_Result_Complete;
		}
		break;
	case JSVE_JobManager_MsgID_JobLog_GetLogFileState:			// ジョブログファイル作成状況取得要求.
		{
			JLGE_LogState sl_Value = APIC_JLG::newInstance()->getLogState();
			st_ResParam.ul_Result = IPCE_JobServiceResult_OK;
			st_ResParam.ul_Param1 = sl_Value;
			c_Response.setData((const char*)&st_ResParam, sizeof(st_ResParam));
			e_Request_Result = JSVE_Request_Result_Complete;
		}
		break;
	case JSVE_JobManager_MsgID_JobLog_GetFileExist:				// ジョブログファイル存在有無取得要求.
		{
			Bool b_Value = APIC_JLG::newInstance()->JobLogFileExist();
			st_ResParam.ul_Result = IPCE_JobServiceResult_OK;
			st_ResParam.ul_Param1 = (uint32_t) b_Value;
			c_Response.setData((const char*)&st_ResParam, sizeof(st_ResParam));
			e_Request_Result = JSVE_Request_Result_Complete;
		}
		break;
	case JSVE_JobManager_MsgID_JobLog_GetLimitOver:				// ジョブログファイル化上限超過有無取得要求.
		{
			Bool b_Value = APIC_JLG::newInstance()->getLimitOver();
			st_ResParam.ul_Result = IPCE_JobServiceResult_OK;
			st_ResParam.ul_Param1 = (uint32_t) b_Value;
			c_Response.setData((const char*)&st_ResParam, sizeof(st_ResParam));
			e_Request_Result = JSVE_Request_Result_Complete;
		}
		break;
	case JSVE_JobManager_MsgID_JobLog_GetLogFilePath:			// ジョブログパス取得要求.
		{
			JLGE_Result sl_Result = APIC_JLG::newInstance()->getJobLogPath(st_ResParam.sc_FilePath);
			if (sl_Result == JLGE_Result_Ok)
			{
				st_ResParam.ul_Result = IPCE_JobServiceResult_OK;
			}
			else
			{
				st_ResParam.ul_Result = IPCE_JobServiceResult_NG;
			}
			c_Response.setData((const char*)&st_ResParam, sizeof(st_ResParam));
			e_Request_Result = JSVE_Request_Result_Complete;
		}
		break;
	case JSVE_JobManager_MsgID_JobLog_GetWebDavLogFilePath:		// ジョブログパス取得要求（WebDav）.
		{
			JLGE_Result sl_Result = APIC_JLG::newInstance()->getWebDAVJobLogPath(st_ResParam.sc_FilePath, (JLGE_Connect)(this->ul_Param1));
			if (sl_Result == JLGE_Result_Ok)
			{
				st_ResParam.ul_Result = IPCE_JobServiceResult_OK;
			}
			else
			{
				st_ResParam.ul_Result = IPCE_JobServiceResult_NG;
			}
			c_Response.setData((const char*)&st_ResParam, sizeof(st_ResParam));
			e_Request_Result = JSVE_Request_Result_Complete;
		}
		break;
	case JSVE_JobManager_MsgID_JobLog_GetLogVersion:			// ログバージョン取得要求.
		{
			ushort us_Major;
			ushort us_Minor;
			APIC_JLG::newInstance()->getLogVersion(&us_Major, &us_Minor);
			st_ResParam.ul_Result = IPCE_JobServiceResult_OK;
			st_ResParam.ul_Param1 = us_Major;
			st_ResParam.ul_Param2 = us_Minor;
			c_Response.setData((const char*)&st_ResParam, sizeof(st_ResParam));
			e_Request_Result = JSVE_Request_Result_Complete;
		}
		break;
	case JSVE_JobManager_MsgID_JobLog_GetJobLogStatus:			// ジョブログ状況取得要求.
		{
			JLGE_JobLogStatus ul_Value = APIC_JLG::newInstance()->getJobLogStatus();
			st_ResParam.ul_Result = IPCE_JobServiceResult_OK;
			st_ResParam.ul_Param1 = ul_Value;
			c_Response.setData((const char*)&st_ResParam, sizeof(st_ResParam));
			e_Request_Result = JSVE_Request_Result_Complete;
		}
		break;
	case JSVE_JobManager_MsgID_JobLog_GetDataStatus:			// ログ保存状況取得要求.
		{
			JLGE_JobLogDataStatus ul_Value = APIC_JLG::newInstance()->getDataStatus();
			st_ResParam.ul_Result = IPCE_JobServiceResult_OK;
			st_ResParam.ul_Param1 = ul_Value;
			c_Response.setData((const char*)&st_ResParam, sizeof(st_ResParam));
			e_Request_Result = JSVE_Request_Result_Complete;
		}
		break;
	case JSVE_JobManager_MsgID_JobLog_GetPreserve:				// 一時ログ情報の有無取得要求.
		{
			Bool b_Value = APIC_JLG::newInstance()->getPreserve();
			st_ResParam.ul_Result = IPCE_JobServiceResult_OK;
			st_ResParam.ul_Param1 = b_Value;
			c_Response.setData((const char*)&st_ResParam, sizeof(st_ResParam));
			e_Request_Result = JSVE_Request_Result_Complete;
		}
		break;
	case JSVE_JobManager_MsgID_JobLog_GetOverWrite:				// 一時ログ情報上書き有無取得要求.
		{
			Bool b_Value = APIC_JLG::newInstance()->getOverWrite();
			st_ResParam.ul_Result = IPCE_JobServiceResult_OK;
			st_ResParam.ul_Param1 = b_Value;
			c_Response.setData((const char*)&st_ResParam, sizeof(st_ResParam));
			e_Request_Result = JSVE_Request_Result_Complete;
		}
		break;
	case JSVE_JobManager_MsgID_JobLog_IsNearFull:				// NearFull判定要求.
		{
			Bool b_Value = APIC_JLG::newInstance()->isNearFull();
			st_ResParam.ul_Result = IPCE_JobServiceResult_OK;
			st_ResParam.ul_Param1 = b_Value;
			c_Response.setData((const char*)&st_ResParam, sizeof(st_ResParam));
			e_Request_Result = JSVE_Request_Result_Complete;
		}
		break;
	case JSVE_JobManager_MsgID_JobLog_IsFull:					// Full判定要求.
		{
			Bool b_Value = APIC_JLG::newInstance()->isFull();
			st_ResParam.ul_Result = IPCE_JobServiceResult_OK;
			st_ResParam.ul_Param1 = b_Value;
			c_Response.setData((const char*)&st_ResParam, sizeof(st_ResParam));
			e_Request_Result = JSVE_Request_Result_Complete;
		}
		break;
	case JSVE_JobManager_MsgID_reqListPrintCancelDoc:			// 重たい処理解除通知受信要求.
		{
			APIC_DoneJobList::newInstance()->reqListPrintCancelDoc();
			st_ResParam.ul_Result = IPCE_JobServiceResult_OK;
			st_ResParam.ul_Param1 = True;
			c_Response.setData((const char*)&st_ResParam, sizeof(st_ResParam));
			e_Request_Result = JSVE_Request_Result_Complete;
		}
		break;
	default:
		// ありえない.
		MyTraceSS(JSVD_DBG_ERROR, "JSVC_RequestOneShotSetJobLog::startAction", "Invalid MsgID");
		break;
	}

	// 処理成功の場合はすぐにAppP/Fへ通知する.
	if (e_Request_Result == JSVE_Request_Result_Complete)
	{
		st_Msg.ul_MsgTypeID = this->getMsgTypeId();
		st_Msg.pReq = this;
		st_Msg.pRes = &c_Response;
		JSVC_JobSendRecv::newInstance()->sendMessageToAPF(&st_Msg);
	}

	return e_Request_Result;
}

// 応答に関連するリクエストか判断.
short JSVC_RequestOneShotSetJobLog::isResponseMsg(JSVC_Response* pRes)
{
	// 同期処理のみを扱うClassの為、常にNot Matchを返す.
	return JSVD_REQUEST_NOT_MATCH;
}


//--------------------------------------------------------------------------------------------------.
//	JSVC_RequestOneShotFunctionInfoクラス定義.
//--------------------------------------------------------------------------------------------------.
//コンストラクタ.
JSVC_RequestOneShotFunctionInfo::JSVC_RequestOneShotFunctionInfo(uint32_t ul_RequestID,uint32_t ul_MsgTypeID)
:	JSVC_RequestOneShot(ul_RequestID,ul_MsgTypeID),pFile(NULL),ul_FilePathSize(0),ul_JobServiceReqID(0),pst_PreviewParam(NULL),pc_Job(NULL)
, ul_AcceptNum(0)
{
}
//デストラクタ.
JSVC_RequestOneShotFunctionInfo::~JSVC_RequestOneShotFunctionInfo()
{
	if(pFile){
		free(pFile);
		pFile = NULL;
	}
	if(pst_PreviewParam){
		free(pst_PreviewParam);
		pst_PreviewParam = NULL;
	}
	if(pc_Job){
		pc_Job->deleteInstance();
		pc_Job = NULL;
	}
}
// 自身の型を示す.
JSVE_Request_Class_Type JSVC_RequestOneShotFunctionInfo::WhoAmI()
{
	return JSVE_Request_Class_Type_Req_Func_Info;
}
// リクエスト受付時のデータ保存処理.
Bool JSVC_RequestOneShotFunctionInfo::setData(const char* pRcvData,short ss_size)
{
#ifdef _DEBUG_JSVC_SEQ
	MyTraceSLD(JSVD_DBG_INFO, "JSVC_RequestOneShotFunctionInfo::setData pRcvData:" , (intptr_t)pRcvData );	// 回避方法をトレースに変更.
	MyTraceSD(JSVD_DBG_INFO, "JSVC_RequestOneShotFunctionInfo::setData ss_size:" , ss_size );		// 回避方法をトレースに変更.
#endif
	MyTraceSD( JSVD_DBG_INFO, "JSVC_RequestOneShotFunctionInfo::setData MsgType:" , (int)getMsgTypeId() );	// warning対応 2013/09/18 ooba.
	switch( getMsgTypeId() ){
		case JSVE_JobManager_MsgID_MakeNormalPreviewImage:
			{
				// 通常プレビュー画像作成要求時のデータ構造体.
				JSVS_ReqPara_MakeNormalPreviewImage* pMsg = (JSVS_ReqPara_MakeNormalPreviewImage*)pRcvData;

				if( pst_PreviewParam != NULL ){
					free(pst_PreviewParam);
					pst_PreviewParam = NULL;
				}
				pst_PreviewParam = (IPCS_PreviewReqImgPara*)malloc(sizeof(IPCS_PreviewReqImgPara));
				if( pst_PreviewParam != NULL ){
					memset( pst_PreviewParam, 0x00, sizeof(IPCS_PreviewReqImgPara) );
					memcpy( pst_PreviewParam, &(pMsg->st_PreviewParam), sizeof(IPCS_PreviewReqImgPara) );
				}
			}
			break;

		case JSVE_JobManager_MsgID_FunctionInfo:
			{
				if( NULL != pFile ){
					free(pFile);
					pFile = NULL;
				}
			}
			break;

		default:
			MyTraceSS( JSVD_DBG_ERROR, "JSVC_RequestOneShotFunctionInfo::setData", "unknown MsgType" );
			break;
	}
	return True;
}
// JobManagerから操作するI/Fを定義.
JSVE_Request_Result JSVC_RequestOneShotFunctionInfo::startAction(JSVS_NotifyInfo* pNotify)
{
#ifdef _DEBUG_JSVC_SEQ
	MyTraceSLD(JSVD_DBG_INFO, "JSVC_RequestOneShotFunctionInfo::startAction pNotify:" , (intptr_t)pNotify );	// 回避方法をトレースに変更.
#endif
	JSVS_SendRecv_MsgParam sParam ;
	JSVE_Request_Result e_Request_Result = JSVE_Request_Result_Err;

	switch( pNotify->pReq->getMsgTypeId() ){
		// 通常プレビュー画像作成要求.
		case JSVE_JobManager_MsgID_MakeNormalPreviewImage:
			switch( pNotify->e_JobManager_MsgTypeID ){
				case JSVE_JobManager_MsgID_MakeNormalPreviewImage:					// 画像作成要求.
					{
						pc_Job = APIC_Job::newInstance(TYPE_JobTypePrint);
						if( pc_Job == NULL  ){
							// インスタンス取得できないときは、エラー通知で終了.
							MyTraceSD(JSVD_DBG_ERROR, "JSVC_RequestOneShotFunctionInfo::startAction Err! newInstance is NULL  MsgID:" , (int)pNotify->e_JobManager_MsgTypeID );	// warning対応 2013/09/18 ooba.
							this->setResult( IPCE_JobServiceResult_NG );
							this->setResultDetail( IPCE_JobsResultDetail_MemAllocFailed );
							e_Request_Result = JSVE_Request_Result_Err;
						}
						else if( pst_PreviewParam == NULL ){
							// パラメータが取得されてないときは、エラー通知で終了.
							MyTraceSD(JSVD_DBG_ERROR, "JSVC_RequestOneShotFunctionInfo::startAction Err! PreviewParam is NULL  MsgID:" , (int)pNotify->e_JobManager_MsgTypeID );	// warning対応 2013/09/18 ooba.
							this->setResult( IPCE_JobServiceResult_NG );
							this->setResultDetail( IPCE_JobsResultDetail_MemAllocFailed );
							e_Request_Result = JSVE_Request_Result_Err;
							pc_Job->deleteInstance();			// インスタンス開放.
							pc_Job = NULL;
						}
						else{
							uint32_t ul_index = -1;
							// ジョブ開始指示.
							ul_index = JSVC_JobManager::newInstance()->availableAnsFunctionIndex( JSVC_CB_INDEX_ans_normal_preview_data, JSVC_CB_MAX_ans_normal_preview_data, this );
							if( ul_index != (uint32_t)-1 ){			// warning対応 2013/09/21 ooba.
//								JSVC_RequestOneShotFunctionInfo* PReq = (JSVC_RequestOneShotFunctionInfo*)pNotify->pReq;		// 2014.11.25 T.Kitade 未使用変数削除.
								TYPS_NormalPreviewData s_NormalPreviewData;
								memset( &s_NormalPreviewData, 0x00, sizeof(s_NormalPreviewData) );

								s_NormalPreviewData.sl_BoxID					= (int32_t)pst_PreviewParam->sl_BoxID;
								s_NormalPreviewData.sl_DocID					= (int32_t)pst_PreviewParam->sl_DocID;
								s_NormalPreviewData.sl_PageNum					= (int32_t)pst_PreviewParam->sl_PageNum;
								s_NormalPreviewData.e_PreviewFormat				= (TYPE_FileFormat)pst_PreviewParam->e_PreviewFormat;
								s_NormalPreviewData.sc_ImageDir					= (char)pst_PreviewParam->e_ImageDir;
								s_NormalPreviewData.sc_ImagePlace				= (char)pst_PreviewParam->e_ImagePlace;
								s_NormalPreviewData.ss_ReqImageDotSizeWidth		= (short)pst_PreviewParam->sl_ReqImageDotSizeWidth;
								s_NormalPreviewData.ss_ReqImageDotSizeHight		= (short)pst_PreviewParam->sl_ReqImageDotSizeHight;
								s_NormalPreviewData.psc_PreviewImageFilePath	= &(pst_PreviewParam->sc_ImageDataFileName[0]);
								s_NormalPreviewData.ul_ImageDataPointerSize		= (uint32_t)pst_PreviewParam->ul_ImageDataPointerSize;
								// APIC_JobのAPIにより、要求を行う.
								ul_AcceptNum = pc_Job->req_getNormalPreviewDataForJobService( &s_NormalPreviewData, (void (*)(uint32_t, TYPE_PreviewResult, TYPS_ActualPreviewPara))(JSVC_JobManager::newInstance()->c_CBManage[ul_index].vp_Func) );
								this->setResult( IPCE_JobServiceResult_OK );
								this->setResultDetail( IPCE_JobsResultDetail_Non );
								e_Request_Result = JSVE_Request_Result_Ok;
							}
							else{
								// 受付NGと判断する.
								MyTraceSD(JSVD_DBG_ERROR, "JSVC_RequestOneShotFunctionInfo::startAction Err! CBManage  MsgID:" , (int)pNotify->e_JobManager_MsgTypeID );	// warning対応 2013/09/18 ooba.
								this->setResult( IPCE_JobServiceResult_NG );
								this->setResultDetail( IPCE_JobsResultDetail_CBNumOver );
								e_Request_Result = JSVE_Request_Result_Err;
								pc_Job->deleteInstance();			// インスタンス開放.
								pc_Job = NULL;
							}
						}
						sParam.ul_MsgTypeID	= JSVE_JobManager_MsgID_MakeNormalPreviewImageAccept;
						sParam.pReq			= this;
						sParam.pRes			= NULL;
						JSVC_JobSendRecv::newInstance()->sendMessageToAPF( &sParam );
					}
					break;

				case JSVE_JobManager_MsgID_MakeNormalPreviewImageEnd:				// 画像作成完了通知.
					{
						JSVC_Response* pRes = (JSVC_Response*)(pNotify->pRes);
//						JSVC_RequestOneShotFunctionInfo* pReq = (JSVC_RequestOneShotFunctionInfo*)(pNotify->pReq);		// 2014.11.25 T.Kitade 未使用変数削除.
						this->ul_Result = pRes->getResult();
						this->ul_ResultDetail = pRes->getResultDetail();
						JSVS_SendRecv_MsgParam sParam;
						sParam.ul_MsgTypeID = JSVE_JobManager_MsgID_MakeNormalPreviewImageEnd;
						sParam.pReq = this;
						sParam.pRes = pNotify->pRes;
						JSVC_JobSendRecv::newInstance()->sendMessageToAPF(&sParam);
						e_Request_Result = JSVE_Request_Result_Complete;
						pc_Job->deleteInstance();			// インスタンス開放.
						pc_Job = NULL;
					}
					break;
	
				default:
					break;
			}
			break;

		// FunctionInfo取得要求.
		case JSVE_JobManager_MsgID_FunctionInfo:
			{
				//既にバッファが取られている場合はクリア.
				if( NULL != pFile ){
					free(pFile);
					pFile = NULL;
				}
				//必要バッファを用意.
				pFile = (char*)malloc (JSVD_FunctionInfo_PathLength);
//Str【 静的解析対応 : Sky田村 : 2012.09.03 : CID-439633 】.
				if( pFile != NULL )
				{
				//ファイルのフルパスを生成してバッファへ.
				FILC_BaseDirIf::newInstance( FILE_DirTypeFunctionInfo );
				memset( pFile, 0, JSVD_FunctionInfo_PathLength );
				snprintf( pFile, (JSVD_FunctionInfo_PathLength-1), "%s/%s", JSVD_FunctionInfo_PathName, JSVD_FunctionInfo_FileName );
				//返信.
				JSVC_JobSendRecv::newInstance()->sendMessageToAPF( getMsgTypeId() , this , NULL );
				MyTraceSLD(JSVD_DBG_INFO, "FunctionInfoFile -> " , (intptr_t)pFile );	// warning対応 2013/09/18 ooba.
				e_Request_Result = JSVE_Request_Result_Complete;		//startAction後にリクエストは消去.
			}
				else
				{
					MyTraceSD(JSVD_DBG_ERROR, "JSVC_RequestOneShotFunctionInfo::startAction Err! malloc Error pFile:" , JSVD_FunctionInfo_PathLength );
					e_Request_Result = JSVE_Request_Result_Err;
				}
//End【 静的解析対応 : Sky田村 : 2012.09.03 : CID-439633 】.
			}
			break;

		default:
			break;
	}

	return e_Request_Result;
}
// 指定した応答に関連するリクエストかどうかを判定.
//  return		JSVD_REQUEST_NOT_MATCH		関係なし.
//				JSVD_REQUEST_MAYBE_MATCH	関係あり（他候補の可能性あり）.
//				JSVD_REQUEST_JUST_MATCH		関係あり（他候補の可能性なし）.
short JSVC_RequestOneShotFunctionInfo::isResponseMsg(JSVC_Response* pRes)
{
#ifdef _DEBUG_JSVC_SEQ
	MyTraceSLD(JSVD_DBG_INFO, "JSVC_RequestOneShotFunctionInfo::isResponseMsg pRes:" , (intptr_t)pRes );	// 回避方法をトレースに変更.
#endif
	return JSVD_REQUEST_NOT_MATCH;
}

char* JSVC_RequestOneShotFunctionInfo::getFilePath()
{
							///< pFileのポインタ取得関数.
	return pFile;
}
uint32_t JSVC_RequestOneShotFunctionInfo::getFilePathSize()
{
						///< ul_FilePathSizeの値取得関数
	return ul_FilePathSize;
}

// 通常プレビュー作成完了コールバック.
void JSVC_RequestOneShotFunctionInfo::ans_getNormalPreviewData01(uint32_t ul_AcceptNum, TYPE_PreviewResult e_Result, TYPS_ActualPreviewPara s_ActualPreviewPara)
{
	char sc_index = 0 + JSVC_CB_INDEX_ans_normal_preview_data;
	ans_getNormalPreviewData_wrap( ul_AcceptNum, e_Result, s_ActualPreviewPara, sc_index );
}
void JSVC_RequestOneShotFunctionInfo::ans_getNormalPreviewData02(uint32_t ul_AcceptNum, TYPE_PreviewResult e_Result, TYPS_ActualPreviewPara s_ActualPreviewPara)
{
	char sc_index = 1 + JSVC_CB_INDEX_ans_normal_preview_data;
	ans_getNormalPreviewData_wrap( ul_AcceptNum, e_Result, s_ActualPreviewPara, sc_index );
}
void JSVC_RequestOneShotFunctionInfo::ans_getNormalPreviewData03(uint32_t ul_AcceptNum, TYPE_PreviewResult e_Result, TYPS_ActualPreviewPara s_ActualPreviewPara)
{
	char sc_index = 2 + JSVC_CB_INDEX_ans_normal_preview_data;
	ans_getNormalPreviewData_wrap( ul_AcceptNum, e_Result, s_ActualPreviewPara, sc_index );
}
void JSVC_RequestOneShotFunctionInfo::ans_getNormalPreviewData04(uint32_t ul_AcceptNum, TYPE_PreviewResult e_Result, TYPS_ActualPreviewPara s_ActualPreviewPara)
{
	char sc_index = 3 + JSVC_CB_INDEX_ans_normal_preview_data;
	ans_getNormalPreviewData_wrap( ul_AcceptNum, e_Result, s_ActualPreviewPara, sc_index );
}
void JSVC_RequestOneShotFunctionInfo::ans_getNormalPreviewData05(uint32_t ul_AcceptNum, TYPE_PreviewResult e_Result, TYPS_ActualPreviewPara s_ActualPreviewPara)
{
	char sc_index = 4 + JSVC_CB_INDEX_ans_normal_preview_data;
	ans_getNormalPreviewData_wrap( ul_AcceptNum, e_Result, s_ActualPreviewPara, sc_index );
}
void JSVC_RequestOneShotFunctionInfo::ans_getNormalPreviewData06(uint32_t ul_AcceptNum, TYPE_PreviewResult e_Result, TYPS_ActualPreviewPara s_ActualPreviewPara)
{
	char sc_index = 5 + JSVC_CB_INDEX_ans_normal_preview_data;
	ans_getNormalPreviewData_wrap( ul_AcceptNum, e_Result, s_ActualPreviewPara, sc_index );
}
void JSVC_RequestOneShotFunctionInfo::ans_getNormalPreviewData07(uint32_t ul_AcceptNum, TYPE_PreviewResult e_Result, TYPS_ActualPreviewPara s_ActualPreviewPara)
{
	char sc_index = 6 + JSVC_CB_INDEX_ans_normal_preview_data;
	ans_getNormalPreviewData_wrap( ul_AcceptNum, e_Result, s_ActualPreviewPara, sc_index );
}
void JSVC_RequestOneShotFunctionInfo::ans_getNormalPreviewData08(uint32_t ul_AcceptNum, TYPE_PreviewResult e_Result, TYPS_ActualPreviewPara s_ActualPreviewPara)
{
	char sc_index = 7 + JSVC_CB_INDEX_ans_normal_preview_data;
	ans_getNormalPreviewData_wrap( ul_AcceptNum, e_Result, s_ActualPreviewPara, sc_index );
}
void JSVC_RequestOneShotFunctionInfo::ans_getNormalPreviewData09(uint32_t ul_AcceptNum, TYPE_PreviewResult e_Result, TYPS_ActualPreviewPara s_ActualPreviewPara)
{
	char sc_index = 8 + JSVC_CB_INDEX_ans_normal_preview_data;
	ans_getNormalPreviewData_wrap( ul_AcceptNum, e_Result, s_ActualPreviewPara, sc_index );
}
void JSVC_RequestOneShotFunctionInfo::ans_getNormalPreviewData10(uint32_t ul_AcceptNum, TYPE_PreviewResult e_Result, TYPS_ActualPreviewPara s_ActualPreviewPara)
{
	char sc_index = 9 + JSVC_CB_INDEX_ans_normal_preview_data;
	ans_getNormalPreviewData_wrap( ul_AcceptNum, e_Result, s_ActualPreviewPara, sc_index );
}
void JSVC_RequestOneShotFunctionInfo::ans_getNormalPreviewData_wrap(uint32_t ul_AcceptNum, TYPE_PreviewResult e_Result, TYPS_ActualPreviewPara s_ActualPreviewPara,char sc_index)
{
	JSVC_RequestBase* pReq = NULL;
	void*	vpDataBuf = NULL;
	short ss_DataLen = 0;
	Bool b_Status;
	unsigned char uc_index = (unsigned char)sc_index;	// warning対応 2013/09/18 ooba.
	
	//排他区間ここから.
	{
		JSVC_CBManagerSemCtrl cCBManage;
		//CB管理領域から呼出パラメータを取得.
		// warning対応 2013/09/18 start ooba. indexをsc_indexからuc_indexに変更.
		pReq		= JSVC_JobManager::c_CBManage[uc_index].pc_ReqBase;
		vpDataBuf	= JSVC_JobManager::c_CBManage[uc_index].vp_DataBuf;
		ss_DataLen	= JSVC_JobManager::c_CBManage[uc_index].ss_DataLen;
		b_Status	= JSVC_JobManager::c_CBManage[uc_index].b_Status;
		//CB管理領域を開放.
		JSVC_JobManager::c_CBManage[uc_index].pc_ReqBase = NULL;
		JSVC_JobManager::c_CBManage[uc_index].vp_DataBuf = NULL;
		JSVC_JobManager::c_CBManage[uc_index].ss_DataLen = 0;
		JSVC_JobManager::c_CBManage[uc_index].b_Status   = False;		//管理領域を「未使用」に戻す.
		// warning対応 2013/09/18 end ooba.
	}
	//排他区間ここまで.

	//実装関数をコール.
	if( b_Status == True){
		ans_getNormalPreviewData_body( ul_AcceptNum, e_Result, s_ActualPreviewPara, pReq );
	}
	//バッファはここで確実に削除.
	if( vpDataBuf ){
//Str【 静的解析対応 : Sky花岡 : 2012.08.30 : CID-436331】.
//JSVC_JobManager::c_CBManage[sc_index].vp_DataBufが使用されていない為、delete処理を削除.
//JSVC_JobManager::c_CBManage[sc_index].vp_DataBufを使用する場合、下記、delete処理を復活させる事。
//なお、void* で指されている領域を開放する場合はfreeで開放する事（確保はmalloc）。
//		delete vpDataBuf;
//End【 静的解析対応 : Sky花岡 : 2012.08.30 : CID-436331】.
		vpDataBuf = NULL;
	}
}
void JSVC_RequestOneShotFunctionInfo::ans_getNormalPreviewData_body(uint32_t ul_AcceptNum, TYPE_PreviewResult e_Result, TYPS_ActualPreviewPara s_ActualPreviewPara, JSVC_RequestBase* pRequest)
{
	//リクエスト元を特定.
	JSVC_RequestOneShotFunctionInfo* pReq = (JSVC_RequestOneShotFunctionInfo*)pRequest;

// 2016/07/08 CQ532138 画像アクセスログ対応 Sky大嶋 Start.
	if(pReq && pReq->pst_PreviewParam){
		addOpeLogPreviewImage(pReq->pst_PreviewParam->sl_DocID, pReq->pst_PreviewParam->sl_BoxID, e_Result);
	}
// 2016/07/08 CQ532138 画像アクセスログ対応 Sky大嶋 End.

	JSVC_ResponseJobNotify* pRes = new JSVC_ResponseJobNotify(JSVE_JobManager_MsgID_MakeNormalPreviewImageEnd);
	#if DEBUGLOG__REQUEST_LIFE
		MyTraceSLD(JSVD_DBG_INFO, "ans_getNormalPreviewData_body new JSVC_ResponseJobNotify", (intptr_t)pRes );
	#endif

	// パラメータの作成.
	if( NULL != pRes ){
		MyTraceSD(JSVD_DBG_INFO, "ans_getNormalPreviewData ul_AcceptNum", (int)ul_AcceptNum );	// warning対応 2013/09/18 ooba.

		//パラメータ取得用の構造体を初期化.
		JSVS_ResponseParamJobNotify s_ResponseParam;
		memset( &s_ResponseParam , 0 , sizeof(s_ResponseParam) );

		switch( e_Result ){
			case TYPE_PreviewResult_OK:
				s_ResponseParam.ul_Result = IPCE_JobServiceResult_OK;
				s_ResponseParam.ul_ResultDetail = IPCE_JobsResultDetail_Non;
				break;
			case TYPE_PreviewResult_NG:
			case TYPE_PreviewResult_MemoryOver:
			case TYPE_PreviewResult_Nothing:
			case TYPE_PreviewResult_OutOfSupport:
			case TYPE_PreviewResult_Cancel:
			case TYPE_PreviewResult_PrintError:
				s_ResponseParam.ul_Result = IPCE_JobServiceResult_NG;
				s_ResponseParam.ul_ResultDetail = IPCE_JobsResultDetail_NotSupport;
				break;
			default:
				s_ResponseParam.ul_Result = IPCE_JobServiceResult_NG;
				s_ResponseParam.ul_ResultDetail = IPCE_JobsResultDetail_Non;
				break;
		}
		s_ResponseParam.ul_AcceptNum = ul_AcceptNum;
		s_ResponseParam.s_ActualPreviewPara = s_ActualPreviewPara;
		pRes->setData( (char*)&s_ResponseParam , sizeof(s_ResponseParam) );

		//JSVC_JobManagerへ送る.
		JSVC_JobManager::newInstance()->JSVC_JobManager::notify( JSVE_JobManager_MsgID_MakeNormalPreviewImageEnd, (JSVC_RequestBase*)pReq, (JSVC_ResponseBase*)pRes);
	}
	else{
		MyTraceSS(JSVD_DBG_ERROR, "ans_getNormalPreviewData","NG");
	}
}

// 2016/07/08 CQ532138 画像アクセスログ対応 Sky大嶋 Start.
void JSVC_RequestOneShotFunctionInfo::addOpeLogPreviewImage(int32_t sl_DocID, int32_t sl_BoxID, TYPE_PreviewResult e_Result)
{
	JLGS_OpeLogCom s_OpeLogCom;
	JLGS_OpeLogFileAccess s_OpeLogFileAccess;
	
	memset(&s_OpeLogCom, 0x00, sizeof(s_OpeLogCom));

	APIC_JLG* p_Jlg = APIC_JLG::newInstance();
	
	p_Jlg->getNowTime(&s_OpeLogCom.s_Time);
	s_OpeLogCom.ul_Code			= JLGE_InspectLogReadBoxDoc;
	s_OpeLogCom.uc_IF			= JLGE_InspectLogIfSystem;
	s_OpeLogCom.us_Result		= (TYPE_ThumbnailResult_OK == e_Result) ? JLGE_InspectLogResultOK : JLGE_InspectLogResultNG;
	s_OpeLogCom.us_EventType	= getEventTypeByResultCode( s_OpeLogCom.us_Result );
	
	s_OpeLogFileAccess.sl_DocNo	= sl_DocID;
	s_OpeLogFileAccess.sl_BoxNo	= sl_BoxID;
	APIC_Doc::newInstance()->getDocName(s_OpeLogFileAccess.sl_DocNo, s_OpeLogFileAccess.sl_BoxNo, reinterpret_cast<char*>(s_OpeLogFileAccess.uc_DocName), sizeof(s_OpeLogFileAccess.uc_DocName), LIBE_CharSet(-1));
	
	p_Jlg->addOpeLogFileAccess(&s_OpeLogCom, &s_OpeLogFileAccess);
}
// 2016/07/08 CQ532138 画像アクセスログ対応 Sky大嶋 End.

JSVC_RequestForReqJobService::JSVC_RequestForReqJobService( uint32_t ul_RequestID, uint32_t ul_MsgTypeID )
:	JSVC_Request(ul_RequestID,ul_MsgTypeID)
, sl_StatusConfirm(0), ul_Param1(0), pc_dispatchfunc(NULL)
{
}
JSVE_Request_Class_Type JSVC_RequestForReqJobService::WhoAmI()
{
	return JSVE_Request_Class_Type_Req_SrvState ;
}

Bool JSVC_RequestForReqJobService::setData(const char* pRcvData,short ss_size)
{
	Bool bl_Ret = True ;
	if ( pRcvData != NULL ) {
		JSVS_ReqPara_JobServiceStatus* pc_Param = (JSVS_ReqPara_JobServiceStatus*)pRcvData ;
		sl_StatusConfirm = pc_Param->sl_StatusConfirm ;
		ul_Param1 = pc_Param->ul_Param1 ;
	} else {
		MyTraceSS( JSVD_DBG_ERROR, "JSVC_RequestForReqJobService::setData RcvData", " is NULL" );
		bl_Ret = False ;
	}
	MyTraceSD( JSVD_DBG_INFO, "JSVC_RequestForReqJobService::setData confirm :" , (int)sl_StatusConfirm );	// warning対応 2013/09/18 ooba.
	switch ( sl_StatusConfirm ) {
	case IPCE_JobService_StatusConfirm_ImageLogTask:
		pc_dispatchfunc = &JSVC_RequestForReqJobService::dispatchAction01 ;
		break ;
	case IPCE_JobService_StatusConfirm_ImageLogFull:
		pc_dispatchfunc = &JSVC_RequestForReqJobService::dispatchAction02 ;
		break ;
	case IPCE_JobService_StatusConfirm_ExistJob:
		pc_dispatchfunc = &JSVC_RequestForReqJobService::dispatchAction03 ;
		break ;
	case IPCE_JobService_StatusConfirm_ExistPrintRxJob:
		pc_dispatchfunc = &JSVC_RequestForReqJobService::dispatchAction04 ;
		break ;
	case IPCE_JobService_StatusConfirm_ExistPdlAppInputProcess:
		pc_dispatchfunc = &JSVC_RequestForReqJobService::dispatchAction05 ;
		break ;
	case IPCE_JobService_StatusConfirm_ExistPrintData:
		pc_dispatchfunc = &JSVC_RequestForReqJobService::dispatchAction06 ;
		break ;
	case IPCE_JobService_StatusConfirm_ExistFaxData:
		pc_dispatchfunc = &JSVC_RequestForReqJobService::dispatchAction07 ;
		break ;
	case IPCE_JobService_StatusConfirm_ExistUsingAdressBookJob:
		pc_dispatchfunc = &JSVC_RequestForReqJobService::dispatchAction08 ;
		break ;
	case IPCE_JobService_StatusConfirm_ExistActiveJob:
		pc_dispatchfunc = &JSVC_RequestForReqJobService::dispatchAction09 ;
		break ;
	case IPCE_JobService_StatusConfirm_EnableManualPrint:
		pc_dispatchfunc = &JSVC_RequestForReqJobService::dispatchAction10 ;
		break ;
	case IPCE_JobService_StatusConfirm_WarningJobList:
		pc_dispatchfunc = &JSVC_RequestForReqJobService::dispatchAction11 ;
		break ;
	case IPCE_JobService_StatusConfirm_JobIdle:
		pc_dispatchfunc = &JSVC_RequestForReqJobService::dispatchAction12 ;
		break ;
	case IPCE_JobService_StatusConfirm_ExistRcvUsrJob:
		pc_dispatchfunc = &JSVC_RequestForReqJobService::dispatchAction13 ;
		break ;
	case IPCE_JobService_StatusConfirm_isAvailable_TwainJobThrow:
		pc_dispatchfunc = &JSVC_RequestForReqJobService::dispatchAction14 ;
		break ;
	// [IT5-82] 長尺紙印刷の際、「長尺紙印刷」の許可設定を不要とした特FWの作成依頼 st.
	case IPCE_JobService_StatusConfirm_ExistActiveJobOtherThanJobId:
		pc_dispatchfunc = &JSVC_RequestForReqJobService::dispatchAction15 ;
		break ;
	// [IT5-82] 長尺紙印刷の際、「長尺紙印刷」の許可設定を不要とした特FWの作成依頼 ed.
	default:
		MyTraceSS( JSVD_DBG_ERROR, "JSVC_RequestForReqJobService::setData" , "unknown confirm" );
		pc_dispatchfunc = NULL ;
		break ;
	}
	return bl_Ret ;
}
JSVE_Request_Result JSVC_RequestForReqJobService::startAction(JSVS_NotifyInfo* pNotify)
{
	JSVE_Request_Result e_Request_Result = JSVE_Request_Result_Err;
	JSVC_ResponseJobServiceStatus* pc_Res = NULL ;
	JSVS_ResponseParamJobServiceStatus s_ResBase ;
	JSVS_SendRecv_MsgParam sParam;

	// Response クラスを作成.
	pc_Res = new JSVC_ResponseJobServiceStatus( getMsgTypeId() ) ;
	#if DEBUGLOG__REQUEST_LIFE
		MyTraceSLD(JSVD_DBG_INFO, "JSVC_RequestForReqJobService::startAction new JSVC_ResponseJobServiceStatus" , (intptr_t)pc_Res );	// warning対応 2013/09/18 ooba.
	#endif
	if ( pc_Res == NULL ) {
		this->ul_Result = JSVE_Request_Result_Err ;
		this->ul_ResultDetail = IPCE_JobsResultDetail_MemAllocFailed ;
		//結果のセット.
		e_Request_Result = JSVE_Request_Result_Err ;
	} else {
		if ( pc_dispatchfunc != NULL ) {
			(this->*(pc_dispatchfunc))(&s_ResBase) ;
			this->ul_Result = JSVE_Request_Result_Ok;
			this->ul_ResultDetail = IPCE_JobsResultDetail_Non;
			//結果のセット.
			e_Request_Result = JSVE_Request_Result_Complete;
		// 見つからなかった場合はエラーにする.
		} else {
			this->ul_Result = JSVE_Request_Result_Err ;
			this->ul_ResultDetail = IPCE_JobsResultDetail_InputErr ;
			//結果のセット.
			e_Request_Result = JSVE_Request_Result_Err ;
		}
		// 結果を Response にセット.
		pc_Res->setData( (char*)&s_ResBase, sizeof(JSVS_ResponseParamJobServiceStatus) ) ;
	}

	sParam.ul_MsgTypeID = getMsgTypeId() ;
	sParam.pReq = this ;
	sParam.pRes = pc_Res ;
	JSVC_JobSendRecv::newInstance()->sendMessageToAPF(&sParam);
// 2012.08.29 sk_takas for CQ#370048 -->
	delete pc_Res ;
	pc_Res = NULL ;
// 2012.08.29 sk_takas for CQ#370048 <--

	return e_Request_Result ;
}
// IPCE_JobService_StatusConfirm_ImageLogTask 実行用
void JSVC_RequestForReqJobService::dispatchAction01(JSVS_ResponseParamJobServiceStatus* pc_Res)
{
	// API をコール.
	pc_Res->ul_Data1 = APIC_ImageLogControll::newInstance()->getImageLogStatus() ;
}
// IPCE_JobService_StatusConfirm_ImageLogFull 実行用
void JSVC_RequestForReqJobService::dispatchAction02(JSVS_ResponseParamJobServiceStatus* pc_Res)
{
	// API をコール.
	pc_Res->ul_Data1 = APIC_ImageLogControll::newInstance()->isImageLogFull() ;
}
// IPCE_JobService_StatusConfirm_ExistJob 実行用
void JSVC_RequestForReqJobService::dispatchAction03(JSVS_ResponseParamJobServiceStatus* pc_Res)
{
	// API をコール.
	APIC_Job* pJob = getInstancebyResourceId(this->ul_Param1);
	pc_Res->ul_Data1 = APIC_JobInstanceList::newInstance()->isExistJob(pJob);
}
// IPCE_JobService_StatusConfirm_ExistPrintRxJob 実行用.
void JSVC_RequestForReqJobService::dispatchAction04(JSVS_ResponseParamJobServiceStatus* pc_Res)
{
	// API をコール.
	pc_Res->ul_Data1 = APIC_ActiveJobList::newInstance()->isExistPrintRxJob() ;
}
// IPCE_JobService_StatusConfirm_ExistPdlAppInputProcess 実行用.
void JSVC_RequestForReqJobService::dispatchAction05(JSVS_ResponseParamJobServiceStatus* pc_Res)
{
	// API をコール.
	pc_Res->ul_Data1 = APIC_ActiveJobList::newInstance()->isExistPdlAppInputProcess() ;
}
// IPCE_JobService_StatusConfirm_ExistPrintData 実行用.
void JSVC_RequestForReqJobService::dispatchAction06(JSVS_ResponseParamJobServiceStatus* pc_Res)
{
	// API をコール.
	pc_Res->ul_Data1 = APIC_ActiveJobList::newInstance()->isExistPrintData() ;
}
// IPCE_JobService_StatusConfirm_ExistFaxData 実行用.
void JSVC_RequestForReqJobService::dispatchAction07(JSVS_ResponseParamJobServiceStatus* pc_Res)
{
	// API をコール.
	pc_Res->ul_Data1 = APIC_ActiveJobList::newInstance()->isExistFaxData( (TYPE_isExistFaxType)this->ul_Param1 ) ;

	MyTraceSD( JSVD_DBG_INFO, "pc_Res->ul_Data1[%d]" , (int)pc_Res->ul_Data1 );	// warning対応 2013/09/18 ooba.
}
// IPCE_JobService_StatusConfirm_ExistUsingAdressBookJob 実行用
void JSVC_RequestForReqJobService::dispatchAction08(JSVS_ResponseParamJobServiceStatus* pc_Res)
{
	// API をコール.
	pc_Res->ul_Data1 = APIC_ActiveJobList::newInstance()->isExistUsingAdressBookJob() ;
}
// IPCE_JobService_StatusConfirm_ExistActiveJob 実行用.
void JSVC_RequestForReqJobService::dispatchAction09(JSVS_ResponseParamJobServiceStatus* pc_Res)
{
	// API をコール.
	pc_Res->ul_Data1 = APIC_ActiveJobList::newInstance()->isExistActiveJob() ;
}
// IPCE_JobService_StatusConfirm_EnableManualPrint 実行用.
void JSVC_RequestForReqJobService::dispatchAction10(JSVS_ResponseParamJobServiceStatus* pc_Res)
{
	// API をコール.
	pc_Res->ul_Data1 = APIC_ActiveJobList::newInstance()->isEnableManualPrint() ;
}
// IPCE_JobService_StatusConfirm_WarningJobList 実行用.
void JSVC_RequestForReqJobService::dispatchAction11(JSVS_ResponseParamJobServiceStatus* pc_Res)
{
	// API をコール.
	pc_Res->ul_Data1 = APIC_ActiveJobList::newInstance()->isWarningJobList( (TYPE_ActiveJobList)this->ul_Param1 ) ;
}
// IPCE_JobService_StatusConfirm_JobIdle 実行用.
void JSVC_RequestForReqJobService::dispatchAction12(JSVS_ResponseParamJobServiceStatus* pc_Res)
{
	// API をコール.
	pc_Res->ul_Data1 = APIC_ActiveJobList::newInstance()->isJobIdle() ;
}
// IPCE_JobService_StatusConfirm_ExistRcvUsrJob 実行用.
void JSVC_RequestForReqJobService::dispatchAction13(JSVS_ResponseParamJobServiceStatus* pc_Res)
{
	// API をコール.
	pc_Res->ul_Data1 = APIC_ActiveJobList::newInstance()->isExistRcvUsrJob( this->ul_Param1 ) ;
}
// IPCE_JobService_StatusConfirm_isAvailable_TwainJobThrow 実行用.
void JSVC_RequestForReqJobService::dispatchAction14(JSVS_ResponseParamJobServiceStatus* pc_Res)
{
	// API をコール.
	uint32_t ul_ApiRet = 0;
	uint32_t ul_isRet = 0;
	pc_Res->ul_Data1 = 0;
	ul_ApiRet = JOBG_isAvailable_TwainJobThrow(&ul_isRet);
	if (JOBE_ApiResult_OK == ul_ApiRet){
		if (JOBE_TwainThrow_OK == ul_isRet){
			pc_Res->ul_Data1 = 1;
		}
	}
}
// [IT5-82] 長尺紙印刷の際、「長尺紙印刷」の許可設定を不要とした特FWの作成依頼 st.
// IPCE_JobService_StatusConfirm_ExistActiveJobOtherThanJobId 実行用.
void JSVC_RequestForReqJobService::dispatchAction15(JSVS_ResponseParamJobServiceStatus* pc_Res)
{
	// API をコール.
	pc_Res->ul_Data1 = APIC_ActiveJobList::newInstance()->isExistActiveJobOtherThanJobId( this->ul_Param1 ) ;
}
// [IT5-82] 長尺紙印刷の際、「長尺紙印刷」の許可設定を不要とした特FWの作成依頼 ed.
JSVC_RequestForReqJobServiceSub::JSVC_RequestForReqJobServiceSub( uint32_t ul_RequestID, uint32_t ul_MsgTypeID )
:	JSVC_Request(ul_RequestID,ul_MsgTypeID)
, sl_StatusConfirm(0), ul_Param1(0), ul_Param2(0), pc_dispatchfunc(NULL)
// CQ367177 対応 2012/08/18 上位に返すメッセージ構造体に、WarningPaperSizeサイズ値の項目を追加する ->.
, pc_dispatchfunc2(NULL)
// CQ367177 対応 2012/08/18 上位に返すメッセージ構造体に、WarningPaperSizeサイズ値の項目を追加する <-.
{
}
JSVE_Request_Class_Type JSVC_RequestForReqJobServiceSub::WhoAmI()
{
	return JSVE_Request_Class_Type_Req_SrvState ;
}

Bool JSVC_RequestForReqJobServiceSub::setData(const char* pRcvData,short ss_size)
{
	Bool bl_Ret = True ;
	if ( pRcvData != NULL ) {
		JSVS_ReqPara_JobServiceStatus_Sub* pc_Param = (JSVS_ReqPara_JobServiceStatus_Sub*)pRcvData ;
		sl_StatusConfirm = pc_Param->sl_StatusConfirm ;
		ul_Param1 = static_cast<uint32_t>(pc_Param->ul_Param1) ;
		ul_Param2 = pc_Param->ul_Param2 ;
		setSubScenarioID(pc_Param->ul_JobResourceID) ;				// 注意：ジョブリソースIDをサブシナリオIDのエリアに設定している.
	} else {
		bl_Ret = False ;
	}
	MyTraceSD( JSVD_DBG_INFO, "JSVC_RequestForReqJobServiceSub::setData confirm :" , (int)sl_StatusConfirm );	// warning対応 2013/09/18 ooba.
	switch ( sl_StatusConfirm ) {
	case IPCE_JobService_JobStatusConfirm_FaxEnable:
		pc_dispatchfunc = &JSVC_RequestForReqJobServiceSub::dispatchAction01 ;
		break ;
	case IPCE_JobService_JobStatusConfirm_WarningPaperSize:
		pc_dispatchfunc2 = &JSVC_RequestForReqJobServiceSub::dispatchAction02 ;
		break ;
	case IPCE_JobService_JobStatusConfirm_WarningPaperType:
		pc_dispatchfunc = &JSVC_RequestForReqJobServiceSub::dispatchAction03 ;
		break ;
	case IPCE_JobService_JobStatusConfirm_WarningFeedTray:
		pc_dispatchfunc = &JSVC_RequestForReqJobServiceSub::dispatchAction04 ;
		break ;
	case IPCE_JobService_JobStatusConfirm_ADFOriginalSizeFixed:
		pc_dispatchfunc = &JSVC_RequestForReqJobServiceSub::dispatchAction05 ;
		break ;
	case IPCE_JobService_JobStatusConfirm_PlatenOriginalSizeFixed:
		pc_dispatchfunc = &JSVC_RequestForReqJobServiceSub::dispatchAction06 ;
		break ;
	case IPCE_JobService_JobStatusConfirm_ScanSendState:
		pc_dispatchfunc = &JSVC_RequestForReqJobServiceSub::dispatchAction07 ;
		break ;
	case IPCE_JobService_JobStatusConfirm_LimitOverCopyKit:
		pc_dispatchfunc = &JSVC_RequestForReqJobServiceSub::dispatchAction08 ;
		break ;
	case IPCE_JobService_JobStatusConfirm_Warning:
		pc_dispatchfunc = &JSVC_RequestForReqJobServiceSub::dispatchAction09 ;
		break ;
	case IPCE_JobService_JobStatusConfirm_RedialJob:
		pc_dispatchfunc = &JSVC_RequestForReqJobServiceSub::dispatchAction10 ;
		break ;
	default:
		MyTraceSS( JSVD_DBG_ERROR, "JSVC_RequestForReqJobServiceSub::setData", "unknown confirm" );
		pc_dispatchfunc = NULL ;
		break ;
	}
	return bl_Ret ;
}
JSVE_Request_Result JSVC_RequestForReqJobServiceSub::startAction(JSVS_NotifyInfo* pNotify)
{
	JSVE_Request_Result e_Request_Result = JSVE_Request_Result_Err;
	JSVC_ResponseJobServiceStatusSub* pc_Res = NULL ;
	JSVS_ResponseParamJobServiceStatusSub s_ResBase ;
// CQ367177 対応 2012/08/18 上位に返すメッセージ構造体に、WarningPaperSizeサイズ値の項目を追加する ->.
	JSVS_ResponseParamJobServiceStatusSub_WarningPaperSize s_ResBase2 ;
// CQ367177 対応 2012/08/18 上位に返すメッセージ構造体に、WarningPaperSizeサイズ値の項目を追加する <-.
	JSVS_SendRecv_MsgParam sParam;

	// Response クラスを作成.
	pc_Res = new JSVC_ResponseJobServiceStatusSub( getMsgTypeId() ) ;
	#if DEBUGLOG__REQUEST_LIFE
		MyTraceSLD(JSVD_DBG_INFO, "JSVC_RequestForReqJobServiceSub::startAction new JSVC_ResponseJobServiceStatusSub" , (intptr_t)pc_Res );	// warning対応 2013/09/18 ooba.
	#endif
	// 仕様変更:ジョブ警告の発生状態については、JobServiceへの問い合わせは行わないようにする.
	// 共有メモリ上の警告情報を新APIが参照する.
	// JobServiceとしては、機能縮小は行わず、問い合わせがあった場合、初期設計どおりに警告判断結果を返すものとする.
	// 注意：getSubScenarioIDで取得した値はリソースIDであり、サブシナリオIDではない.
	if (( 0 > getSubScenarioID())
	||  ( JSVC_Request_JobResourceMax < getSubScenarioID())
	||  ( getInstancebyResourceId(getSubScenarioID()) == NULL )){
		MyTraceSD(JSVD_DBG_INFO, "JSVC_RequestForReqJobServiceSub::startAction 		sl_StatusConfirm", (int)sl_StatusConfirm);	// warning対応 2013/09/18 ooba.
		//結果のセット.
		this->ul_Result = JSVE_Request_Result_Err ;
		this->ul_ResultDetail = IPCE_JobsResultDetail_InputErr ;
		e_Request_Result = JSVE_Request_Result_Err ;
	} else if ( pc_Res == NULL ) {
		//結果のセット.
		this->ul_Result = JSVE_Request_Result_Err ;
		this->ul_ResultDetail = IPCE_JobsResultDetail_MemAllocFailed ;
		e_Request_Result = JSVE_Request_Result_Err ;
	} else {
// CQ367177 対応 2012/08/18 上位に返すメッセージ構造体に、WarningPaperSizeサイズ値の項目を追加する ->.
		if( sl_StatusConfirm == IPCE_JobService_JobStatusConfirm_WarningPaperSize )
		{
			if ( pc_dispatchfunc2 != NULL ) {
				// 要求メッセージIDにあうディスパッチ関数の実行
				(this->*(pc_dispatchfunc2))(&s_ResBase2) ;
				//結果のセット.
				this->ul_Result = JSVE_Request_Result_Ok;
				this->ul_ResultDetail = IPCE_JobsResultDetail_Non;
				e_Request_Result = JSVE_Request_Result_Complete;
			// 見つからなかった場合はエラーにする.
			} else {
				this->ul_Result = JSVE_Request_Result_Err ;
				this->ul_ResultDetail = IPCE_JobsResultDetail_InputErr ;
				//結果のセット.
				e_Request_Result = JSVE_Request_Result_Err ;
			}
			// 結果を Response にセット.
			pc_Res->setData( (char*)&s_ResBase2, sizeof(JSVS_ResponseParamJobServiceStatusSub_WarningPaperSize) ) ;
		}
		else
		{
			if ( pc_dispatchfunc != NULL ) {
				// 要求メッセージIDにあうディスパッチ関数の実行
				(this->*(pc_dispatchfunc))(&s_ResBase) ;
				//結果のセット.
				this->ul_Result = JSVE_Request_Result_Ok;
				this->ul_ResultDetail = IPCE_JobsResultDetail_Non;
				e_Request_Result = JSVE_Request_Result_Complete;
			// 見つからなかった場合はエラーにする.
			} else {
				this->ul_Result = JSVE_Request_Result_Err ;
				this->ul_ResultDetail = IPCE_JobsResultDetail_InputErr ;
				//結果のセット.
				e_Request_Result = JSVE_Request_Result_Err ;
			}
			// 結果を Response にセット.
			pc_Res->setData( (char*)&s_ResBase, sizeof(JSVS_ResponseParamJobServiceStatusSub) ) ;
		}
// CQ367177 対応 2012/08/18 上位に返すメッセージ構造体に、WarningPaperSizeサイズ値の項目を追加する <-.
	}

	sParam.ul_MsgTypeID = getMsgTypeId() ;
	sParam.pReq = this ;
	sParam.pRes = pc_Res ;
	JSVC_JobSendRecv::newInstance()->sendMessageToAPF(&sParam);
// 2012.08.29 sk_takas for CQ#370048 -->
	delete pc_Res ;
	pc_Res = NULL ;
// 2012.08.29 sk_takas for CQ#370048 <--

	return e_Request_Result ;
}
// IPCE_JobService_JobStatusConfirm_FaxEnable 実行用.
void JSVC_RequestForReqJobServiceSub::dispatchAction01(JSVS_ResponseParamJobServiceStatusSub* pc_Res)
{
	// API をコール.
	APIC_Job* pc_Job = getInstancebyResourceId(getSubScenarioID());
	if (NULL != pc_Job){
		pc_Res->ul_Data1 = pc_Job->isFaxEnable( (TYPE_JobType)this->ul_Param1 ) ;
	}else{
		pc_Res->ul_Data1 = False;
	}
}
// CQ367177 対応 2012/08/18 上位に返すメッセージ構造体に、WarningPaperSizeサイズ値の項目を追加する ->.
// IPCE_JobService_JobStatusConfirm_WarningPaperSize 実行用.
//void JSVC_RequestForReqJobServiceSub::dispatchAction02(JSVS_ResponseParamJobServiceStatusSub* pc_Res)
void JSVC_RequestForReqJobServiceSub::dispatchAction02(JSVS_ResponseParamJobServiceStatusSub_WarningPaperSize* pc_Res)
// CQ367177 対応 2012/08/18 上位に返すメッセージ構造体に、WarningPaperSizeサイズ値の項目を追加する <-.
{
	TYPC_PaperSize c_Ret ;
	// API をコール.
	APIC_Job* pc_Job = getInstancebyResourceId(getSubScenarioID());
	if (NULL != pc_Job){
		c_Ret = pc_Job->isWarningPaperSize() ;
	}
	pc_Res->ul_Data1 = (uint32_t)c_Ret.e_SizeCode ;
// CQ367177 対応 2012/08/18 上位に返すメッセージ構造体に、WarningPaperSizeサイズ値の項目を追加する ->.
//	pc_Res->ul_Data2 = (ulong)c_Ret.c_SizeMM.x ;
//	pc_Res->ul_Data3 = (ulong)c_Ret.c_SizeMM.y ;
	pc_Res->f_Data2 = c_Ret.c_SizeMM.x ;
	pc_Res->f_Data3 = c_Ret.c_SizeMM.y ;
// CQ367177 対応 2012/08/18 上位に返すメッセージ構造体に、WarningPaperSizeサイズ値の項目を追加する <-.
	pc_Res->ul_Data4 = (uint32_t)c_Ret.b_WideOn ;

}
// IPCE_JobService_JobStatusConfirm_WarningPaperType 実行用.
void JSVC_RequestForReqJobServiceSub::dispatchAction03(JSVS_ResponseParamJobServiceStatusSub* pc_Res)
{
	TYPC_PaperType c_Ret ;
	// API をコール.
	APIC_Job* pc_Job = getInstancebyResourceId(getSubScenarioID());
	if (NULL != pc_Job){
		c_Ret = pc_Job->isWarningPaperType() ;
	}
	pc_Res->ul_Data1 = (uint32_t)c_Ret.getPaperType() ;
	pc_Res->ul_Data2 = (uint32_t)c_Ret.getPaperColor() ;
	pc_Res->ul_Data3 = (uint32_t)c_Ret.getFeed2ndSidedPaper() ;
	pc_Res->ul_Data4 = (uint32_t)c_Ret.getWidePaperImagePosition() ;
	pc_Res->ul_Data5 = (uint32_t)c_Ret.getCustomPaperType() ;
}
// IPCE_JobService_JobStatusConfirm_WarningFeedTray 実行用.
void JSVC_RequestForReqJobServiceSub::dispatchAction04(JSVS_ResponseParamJobServiceStatusSub* pc_Res)
{
	// API をコール.
//Str【 静的解析対応 : Sky平林 : 2011.08.27 : CID-446756 】
	Bool bl_Ret = False;
//End【 静的解析対応 : Sky平林 : 2011.08.27 : CID-446756 】
	APIC_Job* pc_Job = getInstancebyResourceId(getSubScenarioID());
	if (NULL != pc_Job){
		pc_Res->ul_Data1 = pc_Job->isWarningFeedTray( bl_Ret ) ;
	}else{
		pc_Res->ul_Data1 = TYPE_FeedTray1;
	}
	pc_Res->ul_Data2 = bl_Ret ;
}
// IPCE_JobService_JobStatusConfirm_ADFOriginalSizeFixed 実行用.
void JSVC_RequestForReqJobServiceSub::dispatchAction05(JSVS_ResponseParamJobServiceStatusSub* pc_Res)
{
	// API をコール.
	APIC_Job* pc_Job = getInstancebyResourceId(getSubScenarioID());
	if (NULL != pc_Job){
		pc_Res->ul_Data1 = pc_Job->isADFOriginalSizeFixed() ;
	}else{
		pc_Res->ul_Data1 = False;
	}
}
// IPCE_JobService_JobStatusConfirm_PlatenOriginalSizeFixed 実行用.
void JSVC_RequestForReqJobServiceSub::dispatchAction06(JSVS_ResponseParamJobServiceStatusSub* pc_Res)
{
	// API をコール.
	APIC_Job* pc_Job = getInstancebyResourceId(getSubScenarioID());
	if (NULL != pc_Job){
		pc_Res->ul_Data1 = pc_Job->isPlatenOriginalSizeFixed() ;
	}else{
		pc_Res->ul_Data1 = False;
	}
}
// IPCE_JobService_JobStatusConfirm_ScanSendState 実行用.
void JSVC_RequestForReqJobServiceSub::dispatchAction07(JSVS_ResponseParamJobServiceStatusSub* pc_Res)
{
	// API をコール.
	// 引数の APIC_Job* は旧 Panel でも同じものを使っていたのでそれに倣って実装.
	APIC_Job* pc_Job = getInstancebyResourceId(getSubScenarioID());
	if (NULL != pc_Job){
		pc_Res->ul_Data1 = pc_Job->isScanSendState( pc_Job ) ;
	}else{
		pc_Res->ul_Data1 = False;
	}
}
// IPCE_JobService_JobStatusConfirm_LimitOverCopyKit 実行用.
void JSVC_RequestForReqJobServiceSub::dispatchAction08(JSVS_ResponseParamJobServiceStatusSub* pc_Res)
{
	// API をコール.
	APIC_Job* pc_Job = getInstancebyResourceId(getSubScenarioID());
	if (NULL != pc_Job){
		pc_Res->ul_Data1 = pc_Job->isLimitOverCopyKit( (TYPE_JobWarning)this->ul_Param1, (Bool)this->ul_Param2 ) ;
	}else{
		pc_Res->ul_Data1 = False;
	}
}
// IPCE_JobService_JobStatusConfirm_Warning 実行用.
void JSVC_RequestForReqJobServiceSub::dispatchAction09(JSVS_ResponseParamJobServiceStatusSub* pc_Res)
{
	// API をコール.
	APIC_Job* pc_Job = getInstancebyResourceId(getSubScenarioID());
	if (NULL != pc_Job){
		pc_Res->ul_Data1 = pc_Job->isWarning( (TYPE_JobWarning)this->ul_Param1 ) ;
	}else{
		pc_Res->ul_Data1 = False;
	}
}
// IPCE_JobService_JobStatusConfirm_RedialJob 実行用.
void JSVC_RequestForReqJobServiceSub::dispatchAction10(JSVS_ResponseParamJobServiceStatusSub* pc_Res)
{
	// API をコール.
	APIC_Job* pc_Job = getInstancebyResourceId(getSubScenarioID());
	if (NULL != pc_Job){
// 2017/6/13 IT5_4.1 Fax リダイアル問題対応 mod sta.
		pc_Res->ul_Data1 = pc_Job->checkRedialJob((TYPE_CommLine)ul_Param1) ;
// 2017/6/13 IT5_4.1 Fax リダイアル問題対応 mod end.
	}else{
		pc_Res->ul_Data1 = False;
	}
}
//--------------------------------------------------------------------------------------------------.
//	JSVC_RequestForJobInstanceクラス定義.
//--------------------------------------------------------------------------------------------------.
JSVC_RequestForJobInstance*	JSVC_RequestForJobInstance::pc_ReqInst_ForWaitOutput ;
JSVC_RequestForJobInstance*	JSVC_RequestForJobInstance::pc_ReqInst_ForInputBox ;
// コンストラクタ.
JSVC_RequestForJobInstance::JSVC_RequestForJobInstance(uint32_t ul_RequestID,uint32_t ul_MsgTypeID)
:	JSVC_Request(ul_RequestID,ul_MsgTypeID)
{
	pc_ReqInst_ForWaitOutput = NULL ;
	pc_ReqInst_ForInputBox = NULL ;
}
// 自クラスの型を返す.
JSVE_Request_Class_Type JSVC_RequestForJobInstance::WhoAmI()
{
	return JSVE_Request_Class_Type_Req_Job_Inst;
}
// リクエスト受付時のデータ保存処理.
Bool JSVC_RequestForJobInstance::setData(const char* pRcvData,short ss_size)
{
	Bool bl_Ret = True ;
	MyTraceSD( JSVD_DBG_INFO, "JSVC_RequestForJobInstance::setData MsgTypeID:" , (int)getMsgTypeId() ) ;	// warning対応 2013/09/18 ooba.
	switch ( getMsgTypeId() )
	{
	// ※以下の case は全て s_ReqBase にセットするので break を外している.
	case JSVE_JobManager_MsgID_BindCheck :
	case JSVE_JobManager_MsgID_GetForceChangeTray :
	case JSVE_JobManager_MsgID_WaitOutput :
	case JSVE_JobManager_MsgID_CancelWaitOutput :
	case JSVE_JobManager_MsgID_InputBox :
	case JSVE_JobManager_MsgID_GetPrintAfterSend :
	case JSVE_JobManager_MsgID_GetPrintStopCause :
	case JSVE_JobManager_MsgID_GetDocInfo :
	case JSVE_JobManager_MsgID_GetJimmonErrorPageNum :
	case JSVE_JobManager_MsgID_startManualPrint :					// 2012/02/28 CQ361534 対応.
	case JSVE_JobManager_MsgID_startManualFax :						// MZ-83 FAX送信Jobの優先送信機能対応 2013/08/20 S.Higaki.
		// 変なパラメータが来たら弾けるよう、ss_size をチェックしておく.
		// ※パラメータが設定されない＝サブシナリオIDが設定されないので API が実行できずエラーになる
		if ( ss_size == sizeof(JSVS_ReqMsgParam_Base1) ) {
			m_uParam.s_ReqBase = *((JSVS_ReqMsgParam_Base1*)pRcvData) ;
			setSubScenarioID(m_uParam.s_ReqBase.ul_SubScenarioID);
		}
		break ;
	case JSVE_JobManager_MsgID_SetForceChangeTray :
		// 変なパラメータが来たら弾けるよう、ss_size をチェックしておく.
		// ※パラメータが設定されない＝サブシナリオIDが設定されないので API が実行できずエラーになる
		if ( ss_size == sizeof(JSVS_ReqPara_SetForceChangeTray) ) {
			m_uParam.s_SetForceChangeTray = *((JSVS_ReqPara_SetForceChangeTray*)pRcvData) ;
			setSubScenarioID(m_uParam.s_SetForceChangeTray.ul_SubScenarioID);
		}
		break ;
	case JSVE_JobManager_MsgID_StartRedialJob :
		// 変なパラメータが来たら弾けるよう、ss_size をチェックしておく.
		// ※パラメータが設定されない＝サブシナリオIDが設定されないので API が実行できずエラーになる
		if ( ss_size == sizeof(JSVS_ReqPara_StartRedialJob) ) {
			m_uParam.s_StartRedialJob = *((JSVS_ReqPara_StartRedialJob*)pRcvData) ;
			setSubScenarioID(m_uParam.s_StartRedialJob.ul_SubScenarioID);
		}
		break ;
	case JSVE_JobManager_MsgID_VerifyJimmonPassword :
		// 変なパラメータが来たら弾けるよう、ss_size をチェックしておく.
		// ※パラメータが設定されない＝サブシナリオIDが設定されないので API が実行できずエラーになる
		if ( ss_size == sizeof(JSVS_ReqPara_VerifyJimmonPassword) ) {
			m_uParam.s_VerifyJimmonPassword = *((JSVS_ReqPara_VerifyJimmonPassword*)pRcvData) ;
			setSubScenarioID(m_uParam.s_VerifyJimmonPassword.ul_SubScenarioID);
		}
		break ;
	default :
		MyTraceSS( JSVD_DBG_ERROR, "JSVC_RequestForJobInstance::setData", "Unknown MsgTypeID" ) ;
		bl_Ret = False ;
		break ;
	}
	return bl_Ret ;
}
JSVE_Request_Result JSVC_RequestForJobInstance::startAction(JSVS_NotifyInfo* pNotify)
{
	JSVE_Request_Result e_Request_Result = JSVE_Request_Result_Err;
	JSVS_SendRecv_MsgParam sParam;
	APIC_Job* pc_Job = this->getJobInstance() ;
	uint32_t ul_index ;
	Bool b_ret ;
	uint32_t ulReqMsgTypeId = pNotify->e_JobManager_MsgTypeID ;
	JSVC_ResponseForJobInstance* pc_Res = NULL ;
	JSVS_ResponseParamForJobInstance s_ResBase ;

	if ((JSVE_JobManager_MsgID_BindCheck == ulReqMsgTypeId) ||
		(JSVE_JobManager_MsgID_BindCheckAccept == ulReqMsgTypeId) ||
		(JSVE_JobManager_MsgID_CancelWaitOutput == ulReqMsgTypeId))
	{
		// リソースIDを元にJobインスタンスを取得する.
		pc_Job = getInstancebyResourceId( getSubScenarioID() );
	}

	// Job ポインタが NULL なのは、要求サブシナリオ ID がジョブに絡んでいないことになるか、端からいないか.
	if ( pc_Job == NULL ) {
		this->ul_Result = JSVE_Request_Result_Err;
		this->ul_ResultDetail = IPCE_JobsResultDetail_InputErr ;
		sParam.ul_MsgTypeID = pNotify->e_JobManager_MsgTypeID;
		sParam.pReq = this ;

		JSVC_ResponseForJobInstance c_Res(ulReqMsgTypeId);
		c_Res.setResult( JSVE_Request_Result_Err );
		c_Res.setResultDetail(IPCE_JobsResultDetail_InputErr);
		sParam.pRes = &c_Res;

		JSVC_JobSendRecv::newInstance()->sendMessageToAPF(&sParam);
		e_Request_Result = JSVE_Request_Result_Err ;
	} else {
		switch ( ulReqMsgTypeId ) {
		// 要求系同期.
		case JSVE_JobManager_MsgID_SetForceChangeTray :
			// ここで API コール.
			// [changelog] 2018/01/15 Cuong #245377 Fix static check for IT5_4.2 F00-U2(07) start.
			pc_Job->setForceChangeTray( static_cast<TYPE_FeedTray>(m_uParam.s_SetForceChangeTray.ul_FeedTray) ) ;
			// [changelog] 2018/01/15 Cuong #245377 Fix static check for IT5_4.2 F00-U2(07) end.

			pc_Res = new JSVC_ResponseForJobInstance(ulReqMsgTypeId) ;
			#if DEBUGLOG__REQUEST_LIFE
				MyTraceSLD(JSVD_DBG_INFO, "JSVC_RequestForJobInstance::startAction new JSVC_ResponseForJobInstance1" , (intptr_t)pc_Res );	// warning対応 2013/09/18 ooba.
			#endif
			// Response クラスのメモリ確保に失敗したらそのままエラーで APF に返す.
			if ( pc_Res == NULL ) {
				//設定してAPFへ通信
				this->ul_Result = IPCE_JobServiceResult_NG ;
				this->ul_ResultDetail = IPCE_JobsResultDetail_MemAllocFailed ;
				sParam.ul_MsgTypeID = ulReqMsgTypeId ;
				sParam.pReq = this;
				JSVC_JobSendRecv::newInstance()->sendMessageToAPF(&sParam);
				return JSVE_Request_Result_Err ;
			}
			this->ul_Result = JSVE_Request_Result_Ok;
			this->ul_ResultDetail = IPCE_JobsResultDetail_Non ;
			pc_Res->setData( (char*)&s_ResBase, sizeof(JSVS_ResponseParamForJobInstance) ) ;
			sParam.ul_MsgTypeID = pNotify->e_JobManager_MsgTypeID;
			sParam.pReq = this ;
			sParam.pRes = pc_Res ;
			JSVC_JobSendRecv::newInstance()->sendMessageToAPF(&sParam);
// 2012.08.29 sk_takas for CQ#370048 -->
			delete pc_Res ;
			pc_Res = NULL ;
// 2012.08.29 sk_takas for CQ#370048 <--
			e_Request_Result = JSVE_Request_Result_Complete;
//Str【 静的解析対応 : Sky竪道 : 2013.01.10 : CID-440839 】再見直し.
// 上記CQ#370048の対応と重複しているため削除.
////Str【 静的解析対応 : Sky亀井 : 2012.08.27 : CID-440839 】.
//			delete pc_Res;
////End【 静的解析対応 : Sky亀井 : 2012.08.27 : CID-440839 】.
//End【 静的解析対応 : Sky竪道 : 2013.01.10 : CID-440839 】再見直し.

			break ;
		case JSVE_JobManager_MsgID_GetForceChangeTray :
			// ここで処理結果を入れる.
			s_ResBase.ul_Data1 = (uint32_t)pc_Job->getForceChangeTray() ;

			pc_Res = new JSVC_ResponseForJobInstance(ulReqMsgTypeId) ;
			#if DEBUGLOG__REQUEST_LIFE
				MyTraceSLD(JSVD_DBG_INFO, "JSVC_ResponseForJobInstance::startAction new JSVC_ResponseForJobInstance2" , (intptr_t)pc_Res );	// warning対応 2013/09/18 ooba.
			#endif
			// Response クラスのメモリ確保に失敗したらそのままエラーで APF に返す.
			if ( pc_Res == NULL ) {
				//設定してAPFへ通信
				this->ul_Result = IPCE_JobServiceResult_NG ;
				this->ul_ResultDetail = IPCE_JobsResultDetail_MemAllocFailed ;
				sParam.ul_MsgTypeID = ulReqMsgTypeId ;
				sParam.pReq = this;
				JSVC_JobSendRecv::newInstance()->sendMessageToAPF(&sParam);
				return JSVE_Request_Result_Err ;
			}
			this->ul_Result = JSVE_Request_Result_Ok;
			this->ul_ResultDetail = IPCE_JobsResultDetail_Non ;
			pc_Res->setData( (char*)&s_ResBase, sizeof(JSVS_ResponseParamForJobInstance) ) ;
			sParam.ul_MsgTypeID = pNotify->e_JobManager_MsgTypeID;
			sParam.pReq = this ;
			sParam.pRes = pc_Res ;
			JSVC_JobSendRecv::newInstance()->sendMessageToAPF(&sParam);
			e_Request_Result = JSVE_Request_Result_Complete;
// 2012.08.29 sk_takas for CQ#370048 -->
			delete pc_Res ;
			pc_Res = NULL ;
// 2012.08.29 sk_takas for CQ#370048 <--

			break ;
		case JSVE_JobManager_MsgID_GetDocInfo:
			// ここで処理結果を入れる.
			s_ResBase.ul_Data1 = (uint32_t)pc_Job->getBoxID() ;
			s_ResBase.ul_Data2 = (uint32_t)pc_Job->getDocID() ;

			pc_Res = new JSVC_ResponseForJobInstance(ulReqMsgTypeId) ;
			#if DEBUGLOG__REQUEST_LIFE
				MyTraceSLD(JSVD_DBG_INFO, "JSVC_ResponseForJobInstance::startAction new JSVC_ResponseForJobInstance3" , (intptr_t)pc_Res );	// warning対応 2013/09/18 ooba.
			#endif
			// Response クラスのメモリ確保に失敗したらそのままエラーで APF に返す.
			if ( pc_Res == NULL ) {
				//設定してAPFへ通信
				this->ul_Result = IPCE_JobServiceResult_NG ;
				this->ul_ResultDetail = IPCE_JobsResultDetail_MemAllocFailed ;
				sParam.ul_MsgTypeID = ulReqMsgTypeId ;
				sParam.pReq = this;
				JSVC_JobSendRecv::newInstance()->sendMessageToAPF(&sParam);
				return JSVE_Request_Result_Err ;
			}
			this->ul_Result = JSVE_Request_Result_Ok;
			this->ul_ResultDetail = IPCE_JobsResultDetail_Non ;
			pc_Res->setData( (char*)&s_ResBase, sizeof(JSVS_ResponseParamForJobInstance) ) ;
			sParam.ul_MsgTypeID = pNotify->e_JobManager_MsgTypeID;
			sParam.pReq = this ;
			sParam.pRes = pc_Res ;
			JSVC_JobSendRecv::newInstance()->sendMessageToAPF(&sParam);
			e_Request_Result = JSVE_Request_Result_Complete;
// 2012.08.29 sk_takas for CQ#370048 -->
			delete pc_Res ;
			pc_Res = NULL ;
// 2012.08.29 sk_takas for CQ#370048 <--

			break ;
		case JSVE_JobManager_MsgID_GetPrintAfterSend:
			// ここで処理結果を入れる.
			s_ResBase.ul_Data1 = (uint32_t)pc_Job->getPrintAfterSend() ;

			pc_Res = new JSVC_ResponseForJobInstance(ulReqMsgTypeId) ;
			#if DEBUGLOG__REQUEST_LIFE
				MyTraceSLD(JSVD_DBG_INFO, "JSVC_ResponseForJobInstance::startAction new JSVC_ResponseForJobInstance4" , (intptr_t)pc_Res );	// warning対応 2013/09/18 ooba.
			#endif
			// Response クラスのメモリ確保に失敗したらそのままエラーで APF に返す.
			if ( pc_Res == NULL ) {
				//設定してAPFへ通信
				this->ul_Result = IPCE_JobServiceResult_NG ;
				this->ul_ResultDetail = IPCE_JobsResultDetail_MemAllocFailed ;
				sParam.ul_MsgTypeID = ulReqMsgTypeId ;
				sParam.pReq = this;
				JSVC_JobSendRecv::newInstance()->sendMessageToAPF(&sParam);
				return JSVE_Request_Result_Err ;
			}
			this->ul_Result = JSVE_Request_Result_Ok;
			this->ul_ResultDetail = IPCE_JobsResultDetail_Non ;
			pc_Res->setData( (char*)&s_ResBase, sizeof(JSVS_ResponseParamForJobInstance) ) ;
			sParam.ul_MsgTypeID = pNotify->e_JobManager_MsgTypeID;
			sParam.pReq = this ;
			sParam.pRes = pc_Res ;
			JSVC_JobSendRecv::newInstance()->sendMessageToAPF(&sParam);
			e_Request_Result = JSVE_Request_Result_Complete;
// 2012.08.29 sk_takas for CQ#370048 -->
			delete pc_Res ;
			pc_Res = NULL ;
// 2012.08.29 sk_takas for CQ#370048 <--

			break ;
		case JSVE_JobManager_MsgID_GetPrintStopCause:
			// ここで処理結果を入れる.
			s_ResBase.ul_Data1 = (uint32_t)pc_Job->getPrintStopCause() ;

			pc_Res = new JSVC_ResponseForJobInstance(ulReqMsgTypeId) ;
			#if DEBUGLOG__REQUEST_LIFE
				MyTraceSLD(JSVD_DBG_INFO, "JSVC_ResponseForJobInstance::startAction new JSVC_ResponseForJobInstance5" , (intptr_t)pc_Res );	// warning対応 2013/09/18 ooba.
			#endif
			// Response クラスのメモリ確保に失敗したらそのままエラーで APF に返す.
			if ( pc_Res == NULL ) {
				//設定してAPFへ通信
				this->ul_Result = IPCE_JobServiceResult_NG ;
				this->ul_ResultDetail = IPCE_JobsResultDetail_MemAllocFailed ;
				sParam.ul_MsgTypeID = ulReqMsgTypeId ;
				sParam.pReq = this;
				JSVC_JobSendRecv::newInstance()->sendMessageToAPF(&sParam);
				return JSVE_Request_Result_Err ;
			}
			this->ul_Result = JSVE_Request_Result_Ok;
			this->ul_ResultDetail = IPCE_JobsResultDetail_Non ;
			pc_Res->setData( (char*)&s_ResBase, sizeof(JSVS_ResponseParamForJobInstance) ) ;
			sParam.ul_MsgTypeID = pNotify->e_JobManager_MsgTypeID;
			sParam.pReq = this ;
			sParam.pRes = pc_Res ;
			JSVC_JobSendRecv::newInstance()->sendMessageToAPF(&sParam);
			e_Request_Result = JSVE_Request_Result_Complete;
// 2012.08.29 sk_takas for CQ#370048 -->
			delete pc_Res ;
			pc_Res = NULL ;
// 2012.08.29 sk_takas for CQ#370048 <--

			break ;
		case JSVE_JobManager_MsgID_GetJimmonErrorPageNum:
			// ここで処理結果を入れる.
			s_ResBase.ul_Data1 = (uint32_t)pc_Job->getJimonErrorPageNum() ;

			pc_Res = new JSVC_ResponseForJobInstance(ulReqMsgTypeId) ;
			#if DEBUGLOG__REQUEST_LIFE
				MyTraceSLD(JSVD_DBG_INFO, "JSVC_ResponseForJobInstance::startAction new JSVC_ResponseForJobInstance6" , (intptr_t)pc_Res );	// warning対応 2013/09/18 ooba.
			#endif
			// Response クラスのメモリ確保に失敗したらそのままエラーで APF に返す.
			if ( pc_Res == NULL ) {
				//設定してAPFへ通信
				this->ul_Result = IPCE_JobServiceResult_NG ;
				this->ul_ResultDetail = IPCE_JobsResultDetail_MemAllocFailed ;
				sParam.ul_MsgTypeID = ulReqMsgTypeId ;
				sParam.pReq = this;
				JSVC_JobSendRecv::newInstance()->sendMessageToAPF(&sParam);
				return JSVE_Request_Result_Err ;
			}
			this->ul_Result = JSVE_Request_Result_Ok;
			this->ul_ResultDetail = IPCE_JobsResultDetail_Non ;
			pc_Res->setData( (char*)&s_ResBase, sizeof(JSVS_ResponseParamForJobInstance) ) ;
			sParam.ul_MsgTypeID = pNotify->e_JobManager_MsgTypeID;
			sParam.pReq = this ;
			sParam.pRes = pc_Res ;
			JSVC_JobSendRecv::newInstance()->sendMessageToAPF(&sParam);
			e_Request_Result = JSVE_Request_Result_Complete;
// 2012.08.29 sk_takas for CQ#370048 -->
			delete pc_Res ;
			pc_Res = NULL ;
// 2012.08.29 sk_takas for CQ#370048 <--

			break ;
		case JSVE_JobManager_MsgID_VerifyJimmonPassword:
			// ここで処理結果を入れる.
			s_ResBase.ul_Data1 = (uint32_t)pc_Job->verifyJimonPassword( m_uParam.s_VerifyJimmonPassword.sc_Password ) ;

			pc_Res = new JSVC_ResponseForJobInstance(ulReqMsgTypeId) ;
			#if DEBUGLOG__REQUEST_LIFE
				MyTraceSLD(JSVD_DBG_INFO, "JSVC_ResponseForJobInstance::startAction new JSVC_ResponseForJobInstance7" , (intptr_t)pc_Res );	// warning対応 2013/09/18 ooba.
			#endif
			// Response クラスのメモリ確保に失敗したらそのままエラーで APF に返す.
			if ( pc_Res == NULL ) {
				//設定してAPFへ通信
				this->ul_Result = IPCE_JobServiceResult_NG ;
				this->ul_ResultDetail = IPCE_JobsResultDetail_MemAllocFailed ;
				sParam.ul_MsgTypeID = ulReqMsgTypeId ;
				sParam.pReq = this;
				JSVC_JobSendRecv::newInstance()->sendMessageToAPF(&sParam);
				return JSVE_Request_Result_Err ;
			}
			this->ul_Result = JSVE_Request_Result_Ok;
			this->ul_ResultDetail = IPCE_JobsResultDetail_Non ;
			pc_Res->setData( (char*)&s_ResBase, sizeof(JSVS_ResponseParamForJobInstance) ) ;
			sParam.ul_MsgTypeID = pNotify->e_JobManager_MsgTypeID;
			sParam.pReq = this ;
			sParam.pRes = pc_Res ;
			JSVC_JobSendRecv::newInstance()->sendMessageToAPF(&sParam);
			e_Request_Result = JSVE_Request_Result_Complete;
// 2012.08.29 sk_takas for CQ#370048 -->
			delete pc_Res ;
			pc_Res = NULL ;
// 2012.08.29 sk_takas for CQ#370048 <--

			break ;
		case JSVE_JobManager_MsgID_CancelWaitOutput:
			pc_Job->cancelWaitOutput() ;

			pc_Res = new JSVC_ResponseForJobInstance(ulReqMsgTypeId) ;
			#if DEBUGLOG__REQUEST_LIFE
				MyTraceSLD(JSVD_DBG_INFO, "JSVC_ResponseForJobInstance::startAction new JSVC_ResponseForJobInstance8" , (intptr_t)pc_Res );	// warning対応 2013/09/18 ooba.
			#endif
			// Response クラスのメモリ確保に失敗したらそのままエラーで APF に返す.
			if ( pc_Res == NULL ) {
				//設定してAPFへ通信
				this->ul_Result = IPCE_JobServiceResult_NG ;
				this->ul_ResultDetail = IPCE_JobsResultDetail_MemAllocFailed ;
				sParam.ul_MsgTypeID = ulReqMsgTypeId ;
				sParam.pReq = this;
				JSVC_JobSendRecv::newInstance()->sendMessageToAPF(&sParam);
				return JSVE_Request_Result_Err ;
			}
			this->ul_Result = JSVE_Request_Result_Ok;
			this->ul_ResultDetail = IPCE_JobsResultDetail_Non ;
			pc_Res->setData( (char*)&s_ResBase, sizeof(JSVS_ResponseParamForJobInstance) ) ;
			sParam.ul_MsgTypeID = pNotify->e_JobManager_MsgTypeID;
			sParam.pReq = this ;
			sParam.pRes = pc_Res ;
			JSVC_JobSendRecv::newInstance()->sendMessageToAPF(&sParam);
			e_Request_Result = JSVE_Request_Result_Complete;
// 2012.08.29 sk_takas for CQ#370048 -->
			delete pc_Res ;
			pc_Res = NULL ;
// 2012.08.29 sk_takas for CQ#370048 <--

			break ;
		case JSVE_JobManager_MsgID_StartRedialJob:
			pc_Job->startRedialJob( (uchar)m_uParam.s_StartRedialJob.ul_LineNo ) ;

			pc_Res = new JSVC_ResponseForJobInstance(ulReqMsgTypeId) ;
			#if DEBUGLOG__REQUEST_LIFE
				MyTraceSLD(JSVD_DBG_ERROR, "JSVC_ResponseForJobInstance::startAction new JSVC_ResponseForJobInstance9" , (intptr_t)pc_Res );	// warning対応 2013/09/18 ooba.
			#endif
			// Response クラスのメモリ確保に失敗したらそのままエラーで APF に返す.
			if ( pc_Res == NULL ) {
				//設定してAPFへ通信
				this->ul_Result = IPCE_JobServiceResult_NG ;
				this->ul_ResultDetail = IPCE_JobsResultDetail_MemAllocFailed ;
				sParam.ul_MsgTypeID = ulReqMsgTypeId ;
				sParam.pReq = this;
				JSVC_JobSendRecv::newInstance()->sendMessageToAPF(&sParam);
				return JSVE_Request_Result_Err ;
			}
			this->ul_Result = JSVE_Request_Result_Ok;
			this->ul_ResultDetail = IPCE_JobsResultDetail_Non ;
			pc_Res->setData( (char*)&s_ResBase, sizeof(JSVS_ResponseParamForJobInstance) ) ;
			sParam.ul_MsgTypeID = pNotify->e_JobManager_MsgTypeID;
			sParam.pReq = this ;
			sParam.pRes = pc_Res ;
			JSVC_JobSendRecv::newInstance()->sendMessageToAPF(&sParam);
			e_Request_Result = JSVE_Request_Result_Complete;
// 2012.08.29 sk_takas for CQ#370048 -->
			delete pc_Res ;
			pc_Res = NULL ;
// 2012.08.29 sk_takas for CQ#370048 <--

			break ;
		// 要求系非同期.
		// ※応答系とあわせて実装すること.
		case JSVE_JobManager_MsgID_BindCheck:						// バインドチェック要求.
			//応答はJSVE_JobManager_MsgID_BindCheckAcceptにて.
			ul_index = JSVC_JobManager::newInstance()->availableAnsFunctionIndex( JSVC_CB_INDEX_ans_checkBind, JSVC_CB_MAX_ans_checkBind, pNotify->pReq ) ;
			if( ul_index != (uint32_t)-1 ) {			// warning対応 2013/09/21 ooba.
				pc_Job->req_checkBind( (void (*)(APIC_Job*, TYPE_BindCheckResult))(JSVC_JobManager::newInstance()->c_CBManage[ul_index].vp_Func) ) ;		//2017/10/19 AR1030163 doi change.
				this->ul_Result = JSVE_Request_Result_Ok;
				this->ul_ResultDetail = IPCE_JobsResultDetail_Non;
				e_Request_Result = JSVE_Request_Result_Ok;
			} else {
				// JobService へ NG を返す.
				MyTraceSD(JSVD_DBG_ERROR, "JSVC_RequestForJobInstance::startAction e_Request_Event:" , pNotify->e_Request_Event );
				this->ul_Result = JSVE_JobServiceResult_NG ;
				this->ul_ResultDetail = IPCE_JobsResultDetail_CBNumOver;
				sParam.ul_MsgTypeID = JSVE_JobManager_MsgID_BindCheckAccept;
				sParam.pReq = this ;
				sParam.pRes = pc_Res ;
				JSVC_JobSendRecv::newInstance()->sendMessageToAPF(&sParam);
				e_Request_Result = JSVE_Request_Result_Err ;
			}
			break ;
		case JSVE_JobManager_MsgID_WaitOutput:
			// ※req_waitOutput は同じコンテキストでそのままコールバックを返す仕組みにつき、
			//   ここではコールバック関数を一つだけとする.
			if ( pc_ReqInst_ForWaitOutput == NULL ) {
				pc_ReqInst_ForWaitOutput = this ;
				pc_Job->req_waitOutput( JSVC_RequestForJobInstance::ans_WaitOutput ) ;
				this->ul_Result = JSVE_Request_Result_Ok;
				this->ul_ResultDetail = IPCE_JobsResultDetail_Non;
				e_Request_Result = JSVE_Request_Result_Ok;
			} else {
				// JobService へ NG を返す.
				MyTraceSD(JSVD_DBG_ERROR, "JSVC_RequestForJobInstance::startAction e_Request_Event:" , pNotify->e_Request_Event );
				this->ul_Result = JSVE_JobServiceResult_NG ;
				this->ul_ResultDetail = IPCE_JobsResultDetail_CBNumOver;
				sParam.ul_MsgTypeID = pNotify->e_JobManager_MsgTypeID;
				sParam.pReq = this ;
				sParam.pRes = pc_Res ;
				JSVC_JobSendRecv::newInstance()->sendMessageToAPF(&sParam);
				e_Request_Result = JSVE_Request_Result_Err ;
			}

			break ;
		case JSVE_JobManager_MsgID_InputBox:
			// ※req_inputBox は同じコンテキストでそのままコールバックを返す仕組みにつき、
			//   ここではコールバック関数を一つだけとする.
			if ( pc_ReqInst_ForInputBox == NULL ) {
				pc_ReqInst_ForInputBox = this ;
				pc_Job->req_inputBox( JSVC_RequestForJobInstance::ans_InputBox ) ;
				this->ul_Result = JSVE_Request_Result_Ok;
				this->ul_ResultDetail = IPCE_JobsResultDetail_Non;
				e_Request_Result = JSVE_Request_Result_Ok;
			} else {
				// JobService へ NG を返す.
				MyTraceSD(JSVD_DBG_ERROR, "JSVC_RequestForJobInstance::startAction e_Request_Event:" , pNotify->e_Request_Event );
				this->ul_Result = JSVE_JobServiceResult_NG ;
				this->ul_ResultDetail = IPCE_JobsResultDetail_CBNumOver;
				sParam.ul_MsgTypeID = pNotify->e_JobManager_MsgTypeID;
				sParam.pReq = this ;
				sParam.pRes = pc_Res ;
				JSVC_JobSendRecv::newInstance()->sendMessageToAPF(&sParam);
				e_Request_Result = JSVE_Request_Result_Err ;
			}

			break ;
		// 応答系.
		case JSVE_JobManager_MsgID_BindCheckAccept:
		case JSVE_JobManager_MsgID_WaitOutputAccept:
		case JSVE_JobManager_MsgID_InputBoxAccept:
			if ( pNotify->pRes == NULL ) {
				// pRes が NULL⇒コールバックでメモリ確保できなかった異常シーケンス.
				this->ul_Result = IPCE_JobServiceResult_NG;
				this->ul_ResultDetail = IPCE_JobsResultDetail_MemAllocFailed ;
			} else {
				this->ul_Result = JSVE_Request_Result_Ok;
				this->ul_ResultDetail = IPCE_JobsResultDetail_Non ;
			}
			if ( ulReqMsgTypeId == JSVE_JobManager_MsgID_WaitOutputAccept )	{ pc_ReqInst_ForWaitOutput = NULL ; }
			if ( ulReqMsgTypeId == JSVE_JobManager_MsgID_InputBoxAccept )	{ pc_ReqInst_ForInputBox = NULL ; }
			sParam.ul_MsgTypeID = ulReqMsgTypeId ;
			sParam.pReq = this ;
			sParam.pRes = pNotify->pRes ;
			JSVC_JobSendRecv::newInstance()->sendMessageToAPF(&sParam);
			e_Request_Result = JSVE_Request_Result_Complete;
			break ;
		case JSVE_JobManager_MsgID_startManualPrint:
			// ここで処理結果を入れる.
			b_ret = True;
			// リクエストクラスにNULLを指定しているため、「ans_start (NG2)」のエラーをログ表示することがあるが、期待どおりとなっている.
			ul_index = JSVC_JobManager::newInstance()->availableAnsFunctionIndex( JSVC_CB_INDEX_ans_start, JSVC_CB_MAX_ans_start, NULL );
			if( ul_index != (uint32_t)-1 ) {			// warning対応 2013/09/21 ooba.
				b_ret = pc_Job->req_startManualPrint((void (*)( APIC_Job* ))(JSVC_JobManager::newInstance()->c_CBManage[ul_index].vp_Func)) ;
			}else{
				b_ret = False;
			}
			pc_Res = new JSVC_ResponseForJobInstance(ulReqMsgTypeId) ;
			#if DEBUGLOG__REQUEST_LIFE
				MyTraceSLD(JSVD_DBG_INFO, "JSVC_ResponseForJobInstance::startAction new JSVC_ResponseForJobInstance10" , (intptr_t)pc_Res );
			#endif
			// Response クラスのメモリ確保に失敗したらそのままエラーで APF に返す.
			if (( pc_Res == NULL ) || (False == b_ret)){
				//設定してAPFへ通信
				this->ul_Result = IPCE_JobServiceResult_NG ;
				this->ul_ResultDetail = IPCE_JobsResultDetail_MemAllocFailed ;
				sParam.ul_MsgTypeID = ulReqMsgTypeId ;
				sParam.pReq = this;
				if ( pc_Res != NULL ){
					sParam.pRes = pc_Res ;
				}
				JSVC_JobSendRecv::newInstance()->sendMessageToAPF(&sParam);
//Str【 静的解析対応 : Sky赤松 : 2012.12.03 : CID-440839 】.
				if ( pc_Res != NULL ){
					delete pc_Res ;
//Str【 静的解析対応 : Sky竪道 : 2013.01.10 : CID-440839 】再見直し.
					pc_Res = NULL ;
//End【 静的解析対応 : Sky竪道 : 2013.01.10 : CID-440839 】再見直し.
				}
//End【 静的解析対応 : Sky赤松 : 2012.12.03 : CID-440839 】.
				return JSVE_Request_Result_Err ;
			}
			this->ul_Result = JSVE_Request_Result_Ok;
			this->ul_ResultDetail = IPCE_JobsResultDetail_Non ;
			pc_Res->setData( (char*)&s_ResBase, sizeof(JSVS_ResponseParamForJobInstance) ) ;
			sParam.ul_MsgTypeID = pNotify->e_JobManager_MsgTypeID;
			sParam.pReq = this ;
			sParam.pRes = pc_Res ;
			JSVC_JobSendRecv::newInstance()->sendMessageToAPF(&sParam);
			e_Request_Result = JSVE_Request_Result_Complete;
// 2012.08.29 sk_takas for CQ#370048 -->
			delete pc_Res ;
			pc_Res = NULL ;
// 2012.08.29 sk_takas for CQ#370048 <--
			break ;

		// MZ-83 FAX送信Jobの優先送信機能対応 2013/08/20 S.Higaki end.
		case JSVE_JobManager_MsgID_startManualFax:
			// ここで処理結果を入れる.
			b_ret = True;
			// リクエストクラスにNULLを指定しているため、「ans_start (NG2)」のエラーをログ表示することがあるが、期待どおりとなっている.
			ul_index = JSVC_JobManager::newInstance()->availableAnsFunctionIndex( JSVC_CB_INDEX_ans_start, JSVC_CB_MAX_ans_start, NULL );
			if( ul_index != (uint32_t)-1 ) {				// warning対応 2013/09/21 ooba.
				b_ret = pc_Job->req_startManualFax((void (*)( APIC_Job* ))(JSVC_JobManager::newInstance()->c_CBManage[ul_index].vp_Func)) ;
			}else{
				b_ret = False;
			}
			pc_Res = new JSVC_ResponseForJobInstance(ulReqMsgTypeId) ;
			#if DEBUGLOG__REQUEST_LIFE
				MyTraceSLD(JSVD_DBG_INFO, "JSVC_ResponseForJobInstance::startAction new JSVC_ResponseForJobInstance10" , (intptr_t)pc_Res );
			#endif
			// Response クラスのメモリ確保に失敗したらそのままエラーで APF に返す.
			if (( pc_Res == NULL ) || (False == b_ret)){
				//設定してAPFへ通信
				this->ul_Result = IPCE_JobServiceResult_NG ;
				this->ul_ResultDetail = IPCE_JobsResultDetail_MemAllocFailed ;
				sParam.ul_MsgTypeID = ulReqMsgTypeId ;
				sParam.pReq = this;
				if ( pc_Res != NULL ){
					sParam.pRes = pc_Res ;
				}
				JSVC_JobSendRecv::newInstance()->sendMessageToAPF(&sParam);
				if ( pc_Res != NULL ){
					delete pc_Res ;
					pc_Res = NULL ;
				}
				return JSVE_Request_Result_Err ;
			}
			this->ul_Result = JSVE_Request_Result_Ok;
			this->ul_ResultDetail = IPCE_JobsResultDetail_Non ;
			pc_Res->setData( (char*)&s_ResBase, sizeof(JSVS_ResponseParamForJobInstance) ) ;
			sParam.ul_MsgTypeID = pNotify->e_JobManager_MsgTypeID;
			sParam.pReq = this ;
			sParam.pRes = pc_Res ;
			JSVC_JobSendRecv::newInstance()->sendMessageToAPF(&sParam);
			e_Request_Result = JSVE_Request_Result_Complete;
			delete pc_Res ;
			pc_Res = NULL ;
			break ;
			// MZ-83 FAX送信Jobの優先送信機能対応 2013/08/20 S.Higaki end.
		}
	}
	return e_Request_Result ;
}
//2017/10/19 AR1030163 doi start.
void JSVC_RequestForJobInstance::ans_checkBind01( APIC_Job* pc_Job, TYPE_BindCheckResult us_result )
{
	short ss_index = 0 + JSVC_CB_INDEX_ans_checkBind ;
	ans_checkBind_wrap( pc_Job, us_result, ss_index ) ;
}
void JSVC_RequestForJobInstance::ans_checkBind02( APIC_Job* pc_Job, TYPE_BindCheckResult us_result )
{
	short ss_index = 1 + JSVC_CB_INDEX_ans_checkBind ;
	ans_checkBind_wrap( pc_Job, us_result, ss_index ) ;
}
void JSVC_RequestForJobInstance::ans_checkBind03( APIC_Job* pc_Job, TYPE_BindCheckResult us_result )
{
	short ss_index = 2 + JSVC_CB_INDEX_ans_checkBind ;
	ans_checkBind_wrap( pc_Job, us_result, ss_index ) ;
}
void JSVC_RequestForJobInstance::ans_checkBind04( APIC_Job* pc_Job, TYPE_BindCheckResult us_result )
{
	short ss_index = 3 + JSVC_CB_INDEX_ans_checkBind ;
	ans_checkBind_wrap( pc_Job, us_result, ss_index ) ;
}
void JSVC_RequestForJobInstance::ans_checkBind05( APIC_Job* pc_Job, TYPE_BindCheckResult us_result )
{
	short ss_index = 4 + JSVC_CB_INDEX_ans_checkBind ;
	ans_checkBind_wrap( pc_Job, us_result, ss_index ) ;
}
void JSVC_RequestForJobInstance::ans_checkBind06( APIC_Job* pc_Job, TYPE_BindCheckResult us_result )
{
	short ss_index = 5 + JSVC_CB_INDEX_ans_checkBind ;
	ans_checkBind_wrap( pc_Job, us_result, ss_index ) ;
}
void JSVC_RequestForJobInstance::ans_checkBind07( APIC_Job* pc_Job, TYPE_BindCheckResult us_result )
{
	short ss_index = 6 + JSVC_CB_INDEX_ans_checkBind ;
	ans_checkBind_wrap( pc_Job, us_result, ss_index ) ;
}
void JSVC_RequestForJobInstance::ans_checkBind08( APIC_Job* pc_Job, TYPE_BindCheckResult us_result )
{
	short ss_index = 7 + JSVC_CB_INDEX_ans_checkBind ;
	ans_checkBind_wrap( pc_Job, us_result, ss_index ) ;
}
void JSVC_RequestForJobInstance::ans_checkBind09( APIC_Job* pc_Job, TYPE_BindCheckResult us_result )
{
	short ss_index = 8 + JSVC_CB_INDEX_ans_checkBind ;
	ans_checkBind_wrap( pc_Job, us_result, ss_index ) ;
}
void JSVC_RequestForJobInstance::ans_checkBind10( APIC_Job* pc_Job, TYPE_BindCheckResult us_result )
{
	short ss_index = 9 + JSVC_CB_INDEX_ans_checkBind ;
	ans_checkBind_wrap( pc_Job, us_result, ss_index ) ;
}
// 2012/11/05 CQ373087対応 start.
void JSVC_RequestForJobInstance::ans_checkBind11( APIC_Job* pc_Job, TYPE_BindCheckResult us_result )
{
	short ss_index = 10 + JSVC_CB_INDEX_ans_checkBind ;
	ans_checkBind_wrap( pc_Job, us_result, ss_index ) ;
}
void JSVC_RequestForJobInstance::ans_checkBind12( APIC_Job* pc_Job, TYPE_BindCheckResult us_result )
{
	short ss_index = 11 + JSVC_CB_INDEX_ans_checkBind ;
	ans_checkBind_wrap( pc_Job, us_result, ss_index ) ;
}
void JSVC_RequestForJobInstance::ans_checkBind13( APIC_Job* pc_Job, TYPE_BindCheckResult us_result )
{
	short ss_index = 12 + JSVC_CB_INDEX_ans_checkBind ;
	ans_checkBind_wrap( pc_Job, us_result, ss_index ) ;
}
void JSVC_RequestForJobInstance::ans_checkBind14( APIC_Job* pc_Job, TYPE_BindCheckResult us_result )
{
	short ss_index = 13 + JSVC_CB_INDEX_ans_checkBind ;
	ans_checkBind_wrap( pc_Job, us_result, ss_index ) ;
}
void JSVC_RequestForJobInstance::ans_checkBind15( APIC_Job* pc_Job, TYPE_BindCheckResult us_result )
{
	short ss_index = 14 + JSVC_CB_INDEX_ans_checkBind ;
	ans_checkBind_wrap( pc_Job, us_result, ss_index ) ;
}
void JSVC_RequestForJobInstance::ans_checkBind16( APIC_Job* pc_Job, TYPE_BindCheckResult us_result )
{
	short ss_index = 15 + JSVC_CB_INDEX_ans_checkBind ;
	ans_checkBind_wrap( pc_Job, us_result, ss_index ) ;
}
void JSVC_RequestForJobInstance::ans_checkBind17( APIC_Job* pc_Job, TYPE_BindCheckResult us_result )
{
	short ss_index = 16 + JSVC_CB_INDEX_ans_checkBind ;
	ans_checkBind_wrap( pc_Job, us_result, ss_index ) ;
}
void JSVC_RequestForJobInstance::ans_checkBind18( APIC_Job* pc_Job, TYPE_BindCheckResult us_result )
{
	short ss_index = 17 + JSVC_CB_INDEX_ans_checkBind ;
	ans_checkBind_wrap( pc_Job, us_result, ss_index ) ;
}
void JSVC_RequestForJobInstance::ans_checkBind19( APIC_Job* pc_Job, TYPE_BindCheckResult us_result )
{
	short ss_index = 18 + JSVC_CB_INDEX_ans_checkBind ;
	ans_checkBind_wrap( pc_Job, us_result, ss_index ) ;
}
void JSVC_RequestForJobInstance::ans_checkBind20( APIC_Job* pc_Job, TYPE_BindCheckResult us_result )
{
	short ss_index = 19 + JSVC_CB_INDEX_ans_checkBind ;
	ans_checkBind_wrap( pc_Job, us_result, ss_index ) ;
}
void JSVC_RequestForJobInstance::ans_checkBind21( APIC_Job* pc_Job, TYPE_BindCheckResult us_result )
{
	short ss_index = 20 + JSVC_CB_INDEX_ans_checkBind ;
	ans_checkBind_wrap( pc_Job, us_result, ss_index ) ;
}
void JSVC_RequestForJobInstance::ans_checkBind22( APIC_Job* pc_Job, TYPE_BindCheckResult us_result )
{
	short ss_index = 21 + JSVC_CB_INDEX_ans_checkBind ;
	ans_checkBind_wrap( pc_Job, us_result, ss_index ) ;
}
void JSVC_RequestForJobInstance::ans_checkBind23( APIC_Job* pc_Job, TYPE_BindCheckResult us_result )
{
	short ss_index = 22 + JSVC_CB_INDEX_ans_checkBind ;
	ans_checkBind_wrap( pc_Job, us_result, ss_index ) ;
}
void JSVC_RequestForJobInstance::ans_checkBind24( APIC_Job* pc_Job, TYPE_BindCheckResult us_result )
{
	short ss_index = 23 + JSVC_CB_INDEX_ans_checkBind ;
	ans_checkBind_wrap( pc_Job, us_result, ss_index ) ;
}
void JSVC_RequestForJobInstance::ans_checkBind25( APIC_Job* pc_Job, TYPE_BindCheckResult us_result )
{
	short ss_index = 24 + JSVC_CB_INDEX_ans_checkBind ;
	ans_checkBind_wrap( pc_Job, us_result, ss_index ) ;
}
void JSVC_RequestForJobInstance::ans_checkBind26( APIC_Job* pc_Job, TYPE_BindCheckResult us_result )
{
	short ss_index = 25 + JSVC_CB_INDEX_ans_checkBind ;
	ans_checkBind_wrap( pc_Job, us_result, ss_index ) ;
}
void JSVC_RequestForJobInstance::ans_checkBind27( APIC_Job* pc_Job, TYPE_BindCheckResult us_result )
{
	short ss_index = 26 + JSVC_CB_INDEX_ans_checkBind ;
	ans_checkBind_wrap( pc_Job, us_result, ss_index ) ;
}
void JSVC_RequestForJobInstance::ans_checkBind28( APIC_Job* pc_Job, TYPE_BindCheckResult us_result )
{
	short ss_index = 27 + JSVC_CB_INDEX_ans_checkBind ;
	ans_checkBind_wrap( pc_Job, us_result, ss_index ) ;
}
void JSVC_RequestForJobInstance::ans_checkBind29( APIC_Job* pc_Job, TYPE_BindCheckResult us_result )
{
	short ss_index = 28 + JSVC_CB_INDEX_ans_checkBind ;
	ans_checkBind_wrap( pc_Job, us_result, ss_index ) ;
}
void JSVC_RequestForJobInstance::ans_checkBind30( APIC_Job* pc_Job, TYPE_BindCheckResult us_result )
{
	short ss_index = 29 + JSVC_CB_INDEX_ans_checkBind ;
	ans_checkBind_wrap( pc_Job, us_result, ss_index ) ;
}
void JSVC_RequestForJobInstance::ans_checkBind31( APIC_Job* pc_Job, TYPE_BindCheckResult us_result )
{
	short ss_index = 30 + JSVC_CB_INDEX_ans_checkBind ;
	ans_checkBind_wrap( pc_Job, us_result, ss_index ) ;
}
void JSVC_RequestForJobInstance::ans_checkBind32( APIC_Job* pc_Job, TYPE_BindCheckResult us_result )
{
	short ss_index = 31 + JSVC_CB_INDEX_ans_checkBind ;
	ans_checkBind_wrap( pc_Job, us_result, ss_index ) ;
}
void JSVC_RequestForJobInstance::ans_checkBind33( APIC_Job* pc_Job, TYPE_BindCheckResult us_result )
{
	short ss_index = 32 + JSVC_CB_INDEX_ans_checkBind ;
	ans_checkBind_wrap( pc_Job, us_result, ss_index ) ;
}
void JSVC_RequestForJobInstance::ans_checkBind34( APIC_Job* pc_Job, TYPE_BindCheckResult us_result )
{
	short ss_index = 33 + JSVC_CB_INDEX_ans_checkBind ;
	ans_checkBind_wrap( pc_Job, us_result, ss_index ) ;
}
void JSVC_RequestForJobInstance::ans_checkBind35( APIC_Job* pc_Job, TYPE_BindCheckResult us_result )
{
	short ss_index = 34 + JSVC_CB_INDEX_ans_checkBind ;
	ans_checkBind_wrap( pc_Job, us_result, ss_index ) ;
}
void JSVC_RequestForJobInstance::ans_checkBind36( APIC_Job* pc_Job, TYPE_BindCheckResult us_result )
{
	short ss_index = 35 + JSVC_CB_INDEX_ans_checkBind ;
	ans_checkBind_wrap( pc_Job, us_result, ss_index ) ;
}
void JSVC_RequestForJobInstance::ans_checkBind37( APIC_Job* pc_Job, TYPE_BindCheckResult us_result )
{
	short ss_index = 36 + JSVC_CB_INDEX_ans_checkBind ;
	ans_checkBind_wrap( pc_Job, us_result, ss_index ) ;
}
void JSVC_RequestForJobInstance::ans_checkBind38( APIC_Job* pc_Job, TYPE_BindCheckResult us_result )
{
	short ss_index = 37 + JSVC_CB_INDEX_ans_checkBind ;
	ans_checkBind_wrap( pc_Job, us_result, ss_index ) ;
}
void JSVC_RequestForJobInstance::ans_checkBind39( APIC_Job* pc_Job, TYPE_BindCheckResult us_result )
{
	short ss_index = 38 + JSVC_CB_INDEX_ans_checkBind ;
	ans_checkBind_wrap( pc_Job, us_result, ss_index ) ;
}
void JSVC_RequestForJobInstance::ans_checkBind40( APIC_Job* pc_Job, TYPE_BindCheckResult us_result )
{
	short ss_index = 39 + JSVC_CB_INDEX_ans_checkBind ;
	ans_checkBind_wrap( pc_Job, us_result, ss_index ) ;
}
void JSVC_RequestForJobInstance::ans_checkBind41( APIC_Job* pc_Job, TYPE_BindCheckResult us_result )
{
	short ss_index = 40 + JSVC_CB_INDEX_ans_checkBind ;
	ans_checkBind_wrap( pc_Job, us_result, ss_index ) ;
}
void JSVC_RequestForJobInstance::ans_checkBind42( APIC_Job* pc_Job, TYPE_BindCheckResult us_result )
{
	short ss_index = 41 + JSVC_CB_INDEX_ans_checkBind ;
	ans_checkBind_wrap( pc_Job, us_result, ss_index ) ;
}
void JSVC_RequestForJobInstance::ans_checkBind43( APIC_Job* pc_Job, TYPE_BindCheckResult us_result )
{
	short ss_index = 42 + JSVC_CB_INDEX_ans_checkBind ;
	ans_checkBind_wrap( pc_Job, us_result, ss_index ) ;
}
void JSVC_RequestForJobInstance::ans_checkBind44( APIC_Job* pc_Job, TYPE_BindCheckResult us_result )
{
	short ss_index = 43 + JSVC_CB_INDEX_ans_checkBind ;
	ans_checkBind_wrap( pc_Job, us_result, ss_index ) ;
}
void JSVC_RequestForJobInstance::ans_checkBind45( APIC_Job* pc_Job, TYPE_BindCheckResult us_result )
{
	short ss_index = 44 + JSVC_CB_INDEX_ans_checkBind ;
	ans_checkBind_wrap( pc_Job, us_result, ss_index ) ;
}
void JSVC_RequestForJobInstance::ans_checkBind46( APIC_Job* pc_Job, TYPE_BindCheckResult us_result )
{
	short ss_index = 45 + JSVC_CB_INDEX_ans_checkBind ;
	ans_checkBind_wrap( pc_Job, us_result, ss_index ) ;
}
void JSVC_RequestForJobInstance::ans_checkBind47( APIC_Job* pc_Job, TYPE_BindCheckResult us_result )
{
	short ss_index = 46 + JSVC_CB_INDEX_ans_checkBind ;
	ans_checkBind_wrap( pc_Job, us_result, ss_index ) ;
}
void JSVC_RequestForJobInstance::ans_checkBind48( APIC_Job* pc_Job, TYPE_BindCheckResult us_result )
{
	short ss_index = 47 + JSVC_CB_INDEX_ans_checkBind ;
	ans_checkBind_wrap( pc_Job, us_result, ss_index ) ;
}
void JSVC_RequestForJobInstance::ans_checkBind49( APIC_Job* pc_Job, TYPE_BindCheckResult us_result )
{
	short ss_index = 48 + JSVC_CB_INDEX_ans_checkBind ;
	ans_checkBind_wrap( pc_Job, us_result, ss_index ) ;
}
void JSVC_RequestForJobInstance::ans_checkBind50( APIC_Job* pc_Job, TYPE_BindCheckResult us_result )
{
	short ss_index = 49 + JSVC_CB_INDEX_ans_checkBind ;
	ans_checkBind_wrap( pc_Job, us_result, ss_index ) ;
}
void JSVC_RequestForJobInstance::ans_checkBind51( APIC_Job* pc_Job, TYPE_BindCheckResult us_result )
{
	short ss_index = 50 + JSVC_CB_INDEX_ans_checkBind ;
	ans_checkBind_wrap( pc_Job, us_result, ss_index ) ;
}
void JSVC_RequestForJobInstance::ans_checkBind52( APIC_Job* pc_Job, TYPE_BindCheckResult us_result )
{
	short ss_index = 51 + JSVC_CB_INDEX_ans_checkBind ;
	ans_checkBind_wrap( pc_Job, us_result, ss_index ) ;
}
void JSVC_RequestForJobInstance::ans_checkBind53( APIC_Job* pc_Job, TYPE_BindCheckResult us_result )
{
	short ss_index = 52 + JSVC_CB_INDEX_ans_checkBind ;
	ans_checkBind_wrap( pc_Job, us_result, ss_index ) ;
}
void JSVC_RequestForJobInstance::ans_checkBind54( APIC_Job* pc_Job, TYPE_BindCheckResult us_result )
{
	short ss_index = 53 + JSVC_CB_INDEX_ans_checkBind ;
	ans_checkBind_wrap( pc_Job, us_result, ss_index ) ;
}
void JSVC_RequestForJobInstance::ans_checkBind55( APIC_Job* pc_Job, TYPE_BindCheckResult us_result )
{
	short ss_index = 54 + JSVC_CB_INDEX_ans_checkBind ;
	ans_checkBind_wrap( pc_Job, us_result, ss_index ) ;
}
void JSVC_RequestForJobInstance::ans_checkBind56( APIC_Job* pc_Job, TYPE_BindCheckResult us_result )
{
	short ss_index = 55 + JSVC_CB_INDEX_ans_checkBind ;
	ans_checkBind_wrap( pc_Job, us_result, ss_index ) ;
}
void JSVC_RequestForJobInstance::ans_checkBind57( APIC_Job* pc_Job, TYPE_BindCheckResult us_result )
{
	short ss_index = 56 + JSVC_CB_INDEX_ans_checkBind ;
	ans_checkBind_wrap( pc_Job, us_result, ss_index ) ;
}
void JSVC_RequestForJobInstance::ans_checkBind58( APIC_Job* pc_Job, TYPE_BindCheckResult us_result )
{
	short ss_index = 57 + JSVC_CB_INDEX_ans_checkBind ;
	ans_checkBind_wrap( pc_Job, us_result, ss_index ) ;
}
void JSVC_RequestForJobInstance::ans_checkBind59( APIC_Job* pc_Job, TYPE_BindCheckResult us_result )
{
	short ss_index = 58 + JSVC_CB_INDEX_ans_checkBind ;
	ans_checkBind_wrap( pc_Job, us_result, ss_index ) ;
}
void JSVC_RequestForJobInstance::ans_checkBind60( APIC_Job* pc_Job, TYPE_BindCheckResult us_result )
{
	short ss_index = 59 + JSVC_CB_INDEX_ans_checkBind ;
	ans_checkBind_wrap( pc_Job, us_result, ss_index ) ;
}
void JSVC_RequestForJobInstance::ans_checkBind61( APIC_Job* pc_Job, TYPE_BindCheckResult us_result )
{
	short ss_index = 60 + JSVC_CB_INDEX_ans_checkBind ;
	ans_checkBind_wrap( pc_Job, us_result, ss_index ) ;
}
void JSVC_RequestForJobInstance::ans_checkBind62( APIC_Job* pc_Job, TYPE_BindCheckResult us_result )
{
	short ss_index = 61 + JSVC_CB_INDEX_ans_checkBind ;
	ans_checkBind_wrap( pc_Job, us_result, ss_index ) ;
}
void JSVC_RequestForJobInstance::ans_checkBind63( APIC_Job* pc_Job, TYPE_BindCheckResult us_result )
{
	short ss_index = 62 + JSVC_CB_INDEX_ans_checkBind ;
	ans_checkBind_wrap( pc_Job, us_result, ss_index ) ;
}
void JSVC_RequestForJobInstance::ans_checkBind64( APIC_Job* pc_Job, TYPE_BindCheckResult us_result )
{
	short ss_index = 63 + JSVC_CB_INDEX_ans_checkBind ;
	ans_checkBind_wrap( pc_Job, us_result, ss_index ) ;
}
void JSVC_RequestForJobInstance::ans_checkBind65( APIC_Job* pc_Job, TYPE_BindCheckResult us_result )
{
	short ss_index = 64 + JSVC_CB_INDEX_ans_checkBind ;
	ans_checkBind_wrap( pc_Job, us_result, ss_index ) ;
}
void JSVC_RequestForJobInstance::ans_checkBind66( APIC_Job* pc_Job, TYPE_BindCheckResult us_result )
{
	short ss_index = 65 + JSVC_CB_INDEX_ans_checkBind ;
	ans_checkBind_wrap( pc_Job, us_result, ss_index ) ;
}
// 2012/11/05 CQ373087対応 end.

void JSVC_RequestForJobInstance::ans_checkBind_wrap( APIC_Job* pc_Job, TYPE_BindCheckResult us_result, short ss_index )
//2017/10/19 AR1030163 doi end.
{
	JSVC_RequestBase* pc_Req = NULL;
	void*	vpDataBuf = NULL;
	short ss_DataLen = 0;
	Bool b_Status;

	//排他区間ここから.
	{
		JSVC_CBManagerSemCtrl cCBManage;
		//CB管理領域から呼出パラメータを取得.
		pc_Req		= JSVC_JobManager::c_CBManage[ss_index].pc_ReqBase;
		vpDataBuf	= JSVC_JobManager::c_CBManage[ss_index].vp_DataBuf;
		ss_DataLen	= JSVC_JobManager::c_CBManage[ss_index].ss_DataLen;
		b_Status	= JSVC_JobManager::c_CBManage[ss_index].b_Status;
		//CB管理領域を開放.
		JSVC_JobManager::c_CBManage[ss_index].pc_ReqBase = NULL;
		JSVC_JobManager::c_CBManage[ss_index].vp_DataBuf = NULL;
		JSVC_JobManager::c_CBManage[ss_index].ss_DataLen = 0;
		JSVC_JobManager::c_CBManage[ss_index].b_Status   = False;		//管理領域を「未使用」に戻す.
	}
	//排他区間ここまで.

	//CB管理領域に存在.
	if ( b_Status == True ) {
		JSVS_ResponseParamForJobInstance s_ResBase ;
		JSVC_ResponseForJobInstance* pc_Res = NULL ;
		pc_Res = new JSVC_ResponseForJobInstance(JSVE_JobManager_MsgID_BindCheckAccept) ;
		#if DEBUGLOG__REQUEST_LIFE
			MyTraceSLD(JSVD_DBG_INFO, "ans_checkBind_wrap new JSVC_ResponseForJobInstance" , (intptr_t)pc_Res );	// warning対応 2013/09/18 ooba.
		#endif
		
		//Job ポインタが存在＝チェックOK.
		if ( pc_Job != NULL ) {
			s_ResBase.ul_Data1 = IPCE_JobServiceResult_OK ;
		}
		// NULL の場合は NG だった.
		else {
			s_ResBase.ul_Data1 = IPCE_JobServiceResult_NG ;
		}
		s_ResBase.ul_Data2 = us_result;		//2017/10/19 AR1030163 doi add.
		if ( pc_Res != NULL ) {
			pc_Res->setData( (char*)&s_ResBase, sizeof(JSVS_ResponseParamForJobInstance) ) ;
		}
		//JSVC_JobManagerへ送る.
		JSVC_JobManager::newInstance()->JSVC_JobManager::notify( JSVE_JobManager_MsgID_BindCheckAccept, pc_Req, pc_Res ) ;
	}
	//バッファはここで確実に削除.
	if ( vpDataBuf ) {
//Str【 静的解析対応 : Sky花岡 : 2012.08.30 : CID-436329】.
//JSVC_JobManager::c_CBManage[sc_index].vp_DataBufが使用されていない為、delete処理を削除.
//JSVC_JobManager::c_CBManage[sc_index].vp_DataBufを使用する場合、下記、delete処理を復活させる事。
//なお、void* で指されている領域を開放する場合はfreeで開放する事（確保はmalloc）。
//		delete vpDataBuf;
//End【 静的解析対応 : Sky花岡 : 2012.08.30 : CID-436329】.
		vpDataBuf = NULL;
	}
}
void JSVC_RequestForJobInstance::ans_WaitOutput(APIC_Job* pc_Job)
{
	JSVC_RequestBase* pc_Req = pc_ReqInst_ForWaitOutput ;

	JSVS_ResponseParamForJobInstance s_ResBase ;
	JSVC_ResponseForJobInstance* pc_Res = NULL ;
	pc_Res = new JSVC_ResponseForJobInstance(JSVE_JobManager_MsgID_WaitOutputAccept) ;
	#if DEBUGLOG__REQUEST_LIFE
		MyTraceSLD(JSVD_DBG_INFO, "ans_WaitOutput new JSVC_ResponseForJobInstance" , (intptr_t)pc_Res );	// warning対応 2013/09/18 ooba.
	#endif
	
	//Job ポインタが存在＝チェックOK.
	if ( pc_Job != NULL ) {
		s_ResBase.ul_Data1 = IPCE_JobServiceResult_OK ;
	}
	// NULL の場合は NG だった.
	else {
		s_ResBase.ul_Data1 = IPCE_JobServiceResult_NG ;
	}
	if ( pc_Res != NULL ) {
		pc_Res->setData( (char*)&s_ResBase, sizeof(JSVS_ResponseParamForJobInstance) ) ;
	}
	//JSVC_JobManagerへ送る.
	JSVC_JobManager::newInstance()->JSVC_JobManager::notify( JSVE_JobManager_MsgID_WaitOutputAccept, pc_Req, pc_Res ) ;
}
void JSVC_RequestForJobInstance::ans_InputBox(APIC_Job* pc_Job)
{
	JSVC_RequestBase* pc_Req = pc_ReqInst_ForInputBox ;

	JSVS_ResponseParamForJobInstance s_ResBase ;
	JSVC_ResponseForJobInstance* pc_Res = NULL ;
	pc_Res = new JSVC_ResponseForJobInstance(JSVE_JobManager_MsgID_InputBoxAccept) ;
	#if DEBUGLOG__REQUEST_LIFE
		MyTraceSLD(JSVD_DBG_INFO, "ans_InputBox new JSVC_ResponseForJobInstance" , (intptr_t)pc_Res );	// warning対応 2013/09/18 ooba.
	#endif
	
	//Job ポインタが存在＝チェックOK.
	if ( pc_Job != NULL ) {
		s_ResBase.ul_Data1 = IPCE_JobServiceResult_OK ;
	}
	// NULL の場合は NG だった.
	else {
		s_ResBase.ul_Data1 = IPCE_JobServiceResult_NG ;
	}
	if ( pc_Res != NULL ) {
		pc_Res->setData( (char*)&s_ResBase, sizeof(JSVS_ResponseParamForJobInstance) ) ;
	}
	//JSVC_JobManagerへ送る.
	JSVC_JobManager::newInstance()->JSVC_JobManager::notify( JSVE_JobManager_MsgID_InputBoxAccept, pc_Req, pc_Res ) ;
}

//--------------------------------------------------------------------------------------------------.
//	JSVC_RequestForProgramJobクラス定義.
//--------------------------------------------------------------------------------------------------.
//コンストラクタ.
JSVC_RequestForProgramJob::JSVC_RequestForProgramJob(uint32_t ul_RequestID, uint32_t ul_MsgTypeID)
:	JSVC_Request(ul_RequestID,ul_MsgTypeID), m_ulDocID(0), m_bReqDel(False)

{
}

// デストラクタ.
JSVC_RequestForProgramJob::~JSVC_RequestForProgramJob()
{
}

// JobManagerから操作するI/Fを定義.
JSVE_Request_Result JSVC_RequestForProgramJob::startAction(JSVS_NotifyInfo* pNotify)
{
	JSVE_Request_Result e_Request_Result = JSVE_Request_Result_Ok;
	JSVC_ParentRequest* pc_PRequest = GetParentRequest();
	JSVS_SendRecv_MsgParam sParam;

	if ( NULL == pc_PRequest ){
		MyTraceSD(JSVD_DBG_ERROR, "JSVC_RequestForProgramJob::startAction can't get parent  SubSceID=", (int)getSubScenarioID() );	// warning対応 2013/09/18 ooba.
		return JSVE_Request_Result_Err;
	}

	switch(pNotify->e_JobManager_MsgTypeID){
		// 原稿束情報取得要求.
		case JSVE_JobManager_MsgID_GetInputDataID:
			{
				JSVC_ResponseForProgramJob	response(JSVE_JobManager_MsgID_GetInputDataIDAccept);

				if( JSVE_Request_Status_Stop == pc_PRequest->getRequestStatus() ) {
					if( !pc_PRequest->getDocDeleting() ) {
						int32_t docID = APIC_ActiveJobList::newInstance()->getProgJobDocID();
						if(docID > 0) {
							response.setResult( IPCE_JobServiceResult_OK );
							response.setResultDetail( IPCE_JobsResultDetail_Non );

							// docIDのセット.
							JSVS_GetInputDataIDAnsMsg res;
							res.ul_DocID = docID;
							response.setData( (const char*)&res, sizeof(res));

							// パラメータセット.
							sParam.ul_MsgTypeID = JSVE_JobManager_MsgID_GetInputDataIDAccept;
							sParam.pReq = this;
							sParam.pRes = &response;

							JSVC_JobSendRecv::newInstance()->sendMessageToAPF(&sParam);
							e_Request_Result = JSVE_Request_Result_Complete;
						}
						else {
							// doc保存済みの原稿束がないのでエラー.
							response.setResult( IPCE_JobServiceResult_NG );
							response.setResultDetail( IPCE_JobsResultDetail_NotFoundInputData );
							sParam.ul_MsgTypeID = JSVE_JobManager_MsgID_GetInputDataIDAccept;
							sParam.pReq = this;
							sParam.pRes = &response;
							JSVC_JobSendRecv::newInstance()->sendMessageToAPF(&sParam);
							e_Request_Result = JSVE_Request_Result_Err;
						}
					}
					else {
						// 原稿束削除中なのでエラー.
						response.setResult( IPCE_JobServiceResult_NG );
						response.setResultDetail( IPCE_JobsResultDetail_DisOpeInputData );
						sParam.ul_MsgTypeID = JSVE_JobManager_MsgID_GetInputDataIDAccept;
						sParam.pReq = this;
						sParam.pRes = &response;
						JSVC_JobSendRecv::newInstance()->sendMessageToAPF(&sParam);
						e_Request_Result = JSVE_Request_Result_Err;
					}
				}
				else {
					// サブシナリオ停止中じゃないのでエラー.
					response.setResult( IPCE_JobServiceResult_NG );
					response.setResultDetail( IPCE_JobsResultDetail_DisOpeInputData );
					sParam.ul_MsgTypeID = JSVE_JobManager_MsgID_GetInputDataIDAccept;
					sParam.pReq = this;
					sParam.pRes = &response;
					JSVC_JobSendRecv::newInstance()->sendMessageToAPF(&sParam);
					e_Request_Result = JSVE_Request_Result_Err;
				}
			}
			break;

		// 原稿束削除要求.
		case JSVE_JobManager_MsgID_DelInputData:
			{
				JSVC_ResponseForProgramJob	response(JSVE_JobManager_MsgID_DelInputDataAccept);

				if( JSVE_Request_Status_Stop == pc_PRequest->getRequestStatus() ) {
					if( !pc_PRequest->getDocDeleting() ) {
						APIC_ActiveJobList::newInstance()->req_ProgramJobDocDelete(m_ulDocID, ans_doc_delete);

						// 状態を削除中にする.
						pc_PRequest->setDocDeleting(True);
						m_bReqDel = True;

						response.setResult( IPCE_JobServiceResult_OK );
						response.setResultDetail( IPCE_JobsResultDetail_Non );
						sParam.ul_MsgTypeID = JSVE_JobManager_MsgID_DelInputDataAccept;
						sParam.pReq = this;
						sParam.pRes = &response;
						JSVC_JobSendRecv::newInstance()->sendMessageToAPF(&sParam);
						e_Request_Result = JSVE_Request_Result_Ok;
					}
					else {
						// 原稿束削除中なのでエラー.
						response.setResult( IPCE_JobServiceResult_NG );
						response.setResultDetail( IPCE_JobsResultDetail_DisOpeInputData );
						sParam.ul_MsgTypeID = JSVE_JobManager_MsgID_DelInputDataAccept;
						sParam.pReq = this;
						sParam.pRes = &response;
						JSVC_JobSendRecv::newInstance()->sendMessageToAPF(&sParam);
						e_Request_Result = JSVE_Request_Result_Err;
					}
				}
				else {
					// サブシナリオ停止中じゃないのでエラー.
					response.setResult( IPCE_JobServiceResult_NG );
					response.setResultDetail( IPCE_JobsResultDetail_DisOpeInputData );
					sParam.ul_MsgTypeID = JSVE_JobManager_MsgID_DelInputDataAccept;
					sParam.pReq = this;
					sParam.pRes = &response;
					JSVC_JobSendRecv::newInstance()->sendMessageToAPF(&sParam);
					e_Request_Result = JSVE_Request_Result_Err;
				}
			}
			break;

		// 原稿束削除完了.
		case JSVE_JobManager_MsgID_DelInputDataEnd:
			{
				if(!pNotify->pRes) {
					// レスポンスクラス生成に失敗しているので、ここで作る。
					JSVC_ResponseForProgramJob	response(JSVE_JobManager_MsgID_DelInputDataEnd);

					// メモリ確保失敗
					response.setResult( IPCE_JobServiceResult_NG );
					response.setResultDetail( IPCE_JobsResultDetail_MemAllocFailed );
					sParam.ul_MsgTypeID = JSVE_JobManager_MsgID_DelInputDataEnd;
					sParam.pReq = this;
					sParam.pRes = &response;
					JSVC_JobSendRecv::newInstance()->sendMessageToAPF(&sParam);
					e_Request_Result = JSVE_Request_Result_Err;
				}
				else {
					if( IPCE_JobServiceResult_OK == pNotify->pRes->getResult() ) {
						// 削除OK.
						pNotify->pRes->setResult( IPCE_JobServiceResult_OK );
						pNotify->pRes->setResultDetail( IPCE_JobsResultDetail_Non );
						e_Request_Result = JSVE_Request_Result_Complete;
					}
					else {
						// 削除NG.
						pNotify->pRes->setResult( IPCE_JobServiceResult_NG );
						pNotify->pRes->setResultDetail( IPCE_JobsResultDetail_OpeErrInputData );
						e_Request_Result = JSVE_Request_Result_Err;
					}
					sParam.ul_MsgTypeID = JSVE_JobManager_MsgID_DelInputDataEnd;
					sParam.pReq = this;
					sParam.pRes = pNotify->pRes;
					JSVC_JobSendRecv::newInstance()->sendMessageToAPF(&sParam);
				}

				// 削除中状態の解除
				pc_PRequest->setDocDeleting(False);
				m_bReqDel = False;
			}
			break;

		default:
			MyTraceSD(JSVD_DBG_ERROR, "JSVC_RequestForProgramJob::startAction ERR ID=", (int)pNotify->e_JobManager_MsgTypeID );	// warning対応 2013/09/18 ooba.
			e_Request_Result = JSVE_Request_Result_Err;
			break ;
	}

	return e_Request_Result;
}

// 指定した応答に関連するリクエストかどうかを判定.
short JSVC_RequestForProgramJob::isResponseMsg(JSVC_Response* pRes)
{
	short ss_Ret = JSVD_REQUEST_NOT_MATCH;
	uint32_t ul_MsgTypeID = pRes->getMsgTypeId();

#ifdef _DEBUG_JSVC_SEQ
	MyTraceSD(JSVD_DBG_INFO, "JSVC_RequestForProgramJob::isResponseMsg:" , (int)ul_MsgTypeID );	// warning対応 2013/09/18 ooba.
#endif

	switch(ul_MsgTypeID) {
		case JSVE_JobManager_MsgID_DelInputDataAccept:
		case JSVE_JobManager_MsgID_DelInputDataEnd:
			// 削除要求中のインスタンスの場合、マッチ.
			if( m_bReqDel ) {
				ss_Ret = JSVD_REQUEST_JUST_MATCH;
			}
			else {
				MyTraceSS(JSVD_DBG_ERROR, "JSVC_RequestForProgramJob::isResponseMsg:" , "no request" );
			}
			break;
		default:
			break;
	}

	return ss_Ret;
}

// リクエスト受付時のデータ保存処理.
Bool JSVC_RequestForProgramJob::setData(const char* pRcvData, short ss_size)
{
	Bool ret = True;

	MyTraceSD(JSVD_DBG_INFO, "JSVC_RequestForProgramJob::setData MsgType:", (int)getMsgTypeId() );	// warning対応 2013/09/18 ooba.
	switch( getMsgTypeId() ){
		case JSVE_JobManager_MsgID_GetInputDataID:
			{
				JSVS_ReqPara_SubScenarioBase* pMsg = (JSVS_ReqPara_SubScenarioBase*)pRcvData;
				setSubScenarioID( pMsg->ul_SubScenarioID );				// サブシナリオID.
			}
			break;
		case JSVE_JobManager_MsgID_DelInputData:
			{
				JSVS_ReqPara_DelInputData* pMsg = (JSVS_ReqPara_DelInputData*)pRcvData;
				setSubScenarioID( pMsg->ul_SubScenarioID );				// サブシナリオID.
				m_ulDocID = pMsg->ul_DocID;
			}
			break;
		default:
			MyTraceSS(JSVD_DBG_ERROR, "JSVC_RequestForProgramJob::setData", "unknown MsgType" );
			ret = False;
			break;
	}

	return ret;
}

// 自身の型を示す.
JSVE_Request_Class_Type JSVC_RequestForProgramJob::WhoAmI()
{
	return JSVE_Request_Class_Type_Prg_Job;
}

// 原稿束削除のコールバック関数
void JSVC_RequestForProgramJob::ans_doc_delete(Bool b_res)
{
	JSVC_ResponseForProgramJob* pRes = new JSVC_ResponseForProgramJob(JSVE_JobManager_MsgID_DelInputDataEnd);
	#if DEBUGLOG__REQUEST_LIFE
		MyTraceSLD(JSVD_DBG_INFO, "JSVC_RequestForProgramJob::ans_doc_delete new JSVC_ResponseForProgramJob" , (intptr_t)pRes );	// warning対応 2013/09/18 ooba.
	#endif

	if(pRes) {
		JSVS_SubScenarioInfoNoticeMsg msg;
		if(b_res) {
			msg.e_JobServiceResult = IPCE_JobServiceResult_OK;
			msg.e_JobServiceResultDetail = IPCE_JobsResultDetail_Non;
		}
		else {
			msg.e_JobServiceResult = IPCE_JobServiceResult_NG;
			msg.e_JobServiceResultDetail = IPCE_JobsResultDetail_OpeErrInputData;
		}
		pRes->setData( (const char*)&msg, sizeof(msg) );
	}

	JSVC_JobManager::newInstance()->notify(JSVE_JobManager_MsgID_DelInputDataEnd, NULL, pRes);
}

//--------------------------------------------------------------------------------------------------.
//	JSVC_ResponseBaseクラス定義.
//--------------------------------------------------------------------------------------------------.
// コンストラクタ.
JSVC_ResponseBase::JSVC_ResponseBase(uint32_t ul_MsgTypeID)
: JSVC_Node(ul_MsgTypeID),ul_SubScenarioID(0),pJob(NULL),ss_tedium(-1)
{
	;
}
// デストラクタ.
JSVC_ResponseBase::~JSVC_ResponseBase()
{
}

//--------------------------------------------------------------------------------------------------.
//	JSVC_Responseクラス定義.
//--------------------------------------------------------------------------------------------------.
// コンストラクタ.
JSVC_Response::JSVC_Response(uint32_t ul_MsgTypeID)
: JSVC_ResponseBase(ul_MsgTypeID),e_DataType(0),ul_Data(0)
{
}
// デストラクタ.
JSVC_Response::~JSVC_Response()
{
}
// 自クラスの型を返す.
JSVE_Request_Class_Type JSVC_Response::WhoAmI()
{
	return JSVE_Request_Class_Type_Response;
}
//
// リクエスト受付時のデータ保存処理.
//
//	return	True　:保存OK
//			False :保存NG
//
Bool JSVC_Response::setData(const char* pRcvData,short ss_size)
{
	Bool bRet = True;
#ifdef _DEBUG_JSVC_SEQ
	MyTraceSD(JSVD_DBG_INFO, "JSVC_Response::setData ss_size:" , ss_size );	// 回避方法をトレースに変更.
#endif

	JSVS_ResponseParam *pRcv = (JSVS_ResponseParam*)pRcvData;
	ul_Result = pRcv->ul_Result;
	ul_SubScenarioID = pRcv->ul_SubScenarioID;

	switch( getMsgTypeId() ){
		case JSVE_JobManager_MsgID_GetPageInfo_CanBeMadeAccept:	// プレビュー画像作成可能ページ情報取得応答.
			ul_Data = pRcv->ul_PageNo;
			break;
		case JSVE_JobManager_MsgID_MakePreviewImageAccept:		// プレビュー画像作成応答.
		case JSVE_JobManager_MsgID_DeletePreviewImageAccept:	// プレビュー画像削除応答.
		case JSVE_JobManager_MsgID_CancelPreviewImageAccept:	// プレビュー画像作成中止応答.
			// 個別パラメータは無し.
			break;
		case JSVE_JobManager_MsgID_ChargeMapPrepared:			// 課金マップ展開完了通知.
		case JSVE_JobManager_MsgID_MemoryAllocAccept:			// サブシナリオメモリ確保応答.
		case JSVE_JobManager_MsgID_StartAccept:					// サブシナリオ実行応答.
		case JSVE_JobManager_MsgID_StopAccept:					// サブシナリオ停止応答.
		case JSVE_JobManager_MsgID_ReStartAccept:				// サブシナリオ再開応答.
		case JSVE_JobManager_MsgID_CancelAccept:				// サブシナリオ削除応答（キャンセル）.
		case JSVE_JobManager_MsgID_ChangeSettingAccept:			// サブシナリオ設定変更応答.
		case JSVE_JobManager_MsgID_MemoryDeleteAccept:			// サブシナリオメモリ破棄応答.
		case JSVE_JobManager_MsgID_AllStopAccept:				// 全サブシナリオ停止応答.
		case JSVE_JobManager_MsgID_AllReStartAccept:			// 全サブシナリオ再開応答.
			ul_ResultDetail = pRcv->ul_ResultDetail;
			break;
		case JSVE_JobManager_MsgID_InitJobManager:				// JobManager初期処理.
		case JSVE_JobManager_MsgID_HddStatusChanged:			// HDDステータス変化通知.
		case JSVE_JobManager_MsgID_UpdateWarningCheck:			// ジョブリソース警告の更新＆チェック.
			break;

		default:
			MyTraceSD( JSVD_DBG_ERROR, "JSVC_Response::setData unknown MsgType :" , (int)getMsgTypeId() );	// warning対応 2013/09/18 ooba.
			bRet = False;
			break;
	}
	return bRet;
}

//--------------------------------------------------------------------------------------------------.
//	JSVC_ResponseJobNotifyクラス定義.
//--------------------------------------------------------------------------------------------------.
// コンストラクタ.
JSVC_ResponseJobNotify::JSVC_ResponseJobNotify(uint32_t ul_MsgTypeID)
:	JSVC_Response(ul_MsgTypeID),e_FuncSts(0),ul_Status(0),ul_PreviewReqId(0),ul_WorkData(0),b_Continue(False)
{
}

// 自クラスの型を返す.
JSVE_Request_Class_Type JSVC_ResponseJobNotify::WhoAmI()
{
	return JSVE_Request_Class_Type_Res_Notify;
}
//
// リクエスト受付時のデータ保存処理.
//
//	return	True　:保存OK
//			False :保存NG
//
Bool JSVC_ResponseJobNotify::setData(const char* pRcvData,short ss_size)
{
	Bool bRet = True;
#ifdef _DEBUG_JSVC_SEQ
	MyTraceSD(JSVD_DBG_INFO, "JSVC_ResponseJobNotify::setData ss_size:" , ss_size );	// 回避方法をトレースに変更.
#endif

	JSVS_ResponseParamJobNotify *pRcv = (JSVS_ResponseParamJobNotify*)pRcvData;
	ul_Result = pRcv->ul_Result;
	ul_ResultDetail = pRcv->ul_ResultDetail;
	ul_SubScenarioID = pRcv->ul_SubScenarioID;
	pJob = pRcv->pJob;

	switch( getMsgTypeId() ){
		case JSVE_JobManager_MsgID_JobLog_CreateFileEnd:		// ジョブログファイル作成完了.
			break;
		case JSVE_JobManager_MsgID_Job_JobEnd:					// Jobからのジョブ終了通知.
		case JSVE_JobManager_MsgID_End:							// サブシナリオ実行完了.
			b_Continue = pRcv->b_Continue;
			break;
		case JSVE_JobManager_MsgID_MakePreviewImageEnd:			// プレビュー画像作成完了.
		case JSVE_JobManager_MsgID_MakeNormalPreviewImageEnd:	// 通常プレビュー画像作成完了.
			ul_PreviewReqId = pRcv->ul_AcceptNum;
			break;
		case JSVE_JobManager_MsgID_DoneJobList_DataNotify:		// DoneJobListからのデータ変化通知.
		case JSVE_JobManager_MsgID_DataBase_Update:				// 履歴DB更新通知.
		case JSVE_JobManager_MsgID_JobLogStatus:				// ジョブログ状態変化通知.
		case JSVE_JobManager_MsgID_TempLogStatus:				// 一時ログ状態変化通知.
		case JSVE_JobManager_MsgID_JobLogFileStatus:			// ジョブログファイル状態変化通知.
		case JSVE_JobManager_MsgID_JobLogPreserve:				// ジョブログ保存状態変化通知.
		case JSVE_JobManager_MsgID_JobLogFileProgress:			// ファイル作成状況通知.
			break;
		case JSVE_JobManager_MsgID_HistoryDelete:				// 履歴削除結果通知.
		case JSVE_JobManager_MsgID_MemoryAllocEnd:				// サブシナリオメモリ確保完了.
		case JSVE_JobManager_MsgID_StopEnd:						// サブシナリオ停止完了.
		case JSVE_JobManager_MsgID_ReStartEnd:					// サブシナリオ再開完了.
		case JSVE_JobManager_MsgID_CancelEnd:					// サブシナリオ削除完了（キャンセル）.
		case JSVE_JobManager_MsgID_ChangeSettingEnd:			// サブシナリオ設定変更完了.
		case JSVE_JobManager_MsgID_MemoryDeleteEnd:				// サブシナリオメモリ破棄完了.
		case JSVE_JobManager_MsgID_AllStopEnd:					// 全サブシナリオ停止完了.
		case JSVE_JobManager_MsgID_AllReStartEnd:				// 全サブシナリオ再開完了.
			break;
		case JSVE_JobManager_MsgID_JobLogFileMake:				// ジョブログファイル作成結果通知.
		case JSVE_JobManager_MsgID_newJobInstance:				// 待ちうけ部からの実行要求(newJobInstance要求).
		case JSVE_JobManager_MsgID_addJob:						// ActiveJobListからの状態変化通知(ジョブの追加).
		case JSVE_JobManager_MsgID_deleteJob:					// ActiveJobListからの状態変化通知(ジョブの破棄).
		case JSVE_JobManager_MsgID_changedOrder:				// ActiveJobListからの状態変化通知(ジョブの優先順位変更).
		case JSVE_JobManager_MsgID_DocSearcher:					// DocSearcherからの状態変化通知(ジョブ起動結果).
		case JSVE_JobManager_MsgID_JobDeleteInstance:			// APIC_Jobインスタンス破棄通知.
		case JSVE_JobManager_MsgID_recieveSecureJob:			// ActiveJobListからの状態変化通知(ジョブの優先順位変更).
		case JSVE_JobManager_MsgID_NotifyAnsStart:				// 2012/09/21 CQ372110 IWSへのans対応 Jobのans_startの通知.
		case JSVE_JobManager_MsgID_StopTargetJobWasDeleted:		// comment Sky新大阪 [特ROM対応][IT5-6] プリントエラー解除時に他人のジョブの混在を防止する対応.

// comment [特ROM対応][IT5-26] エラー発生時のジョブの削除方法を自動又は手動に設定可能とする対応 ->.
		case JSVE_JobManager_MsgID_StartActiveJobListDeleteSet:		// ジョブ自動削除開始のオブザーバ通知.
		case JSVE_JobManager_MsgID_EndActiveJobListDeleteSet:		// ジョブ自動削除終了のオブザーバ通知.
// comment [特ROM対応][IT5-26] エラー発生時のジョブの削除方法を自動又は手動に設定可能とする対応 <-.

			break;
		case JSVE_JobManager_MsgID_RevivalJudgmentEnd:			// 復活判断完了通知.
			break;
// 市場問題(FQ-OPSD-1500097-OFP068)対応 2015/5/26 m.ooba start.
		case JSVE_JobManager_MsgID_JobStartNotify:				// サブシナリオ状態変化通知(ジョブ開始).
		case JSVE_JobManager_MsgID_JobEndNotify:				// サブシナリオ状態変化通知(ジョブ開始).
			break;
// 市場問題(FQ-OPSD-1500097-OFP068)対応 2015/5/26 m.ooba end.
		default:
			MyTraceSD( JSVD_DBG_ERROR, "JSVC_ResponseJobNotify::setData unknown MsgType :" , (int)getMsgTypeId() );	// warning対応 2013/09/18 ooba.
			bRet = False;
			break;
	}

	return bRet;
}

// 2013/01/30 CQ377138対応 Jobインスタンス取得不可時のシナリオ登録応答 add start.
// 汎用データ設定.
void JSVC_ResponseJobNotify::setWorkData(uint32_t ul_work)
{
	ul_WorkData = ul_work;
}
// 汎用データ取得.
uint32_t JSVC_ResponseJobNotify::getWorkData()
{
	return ul_WorkData;
}
// 2013/01/30 CQ377138対応 Jobインスタンス取得不可時のシナリオ登録応答 add end.




//--------------------------------------------------------------------------------------------------.
//	JSVC_ResponseJobNotifyWarningクラス定義.
//--------------------------------------------------------------------------------------------------.
// コンストラクタ.
JSVC_ResponseJobNotifyWarning::JSVC_ResponseJobNotifyWarning(uint32_t ul_MsgTypeID)
:	JSVC_Response(ul_MsgTypeID)
	,e_Warning(0), sc_ImageDataFull(0), e_DataType(0)
{
}
// 自クラスの型を返す.
JSVE_Request_Class_Type JSVC_ResponseJobNotifyWarning::WhoAmI()
{
	return JSVE_Request_Class_Type_Res_Warning;
}
//
// リクエスト受付時のデータ保存処理.
//
//	return	True　:保存OK
//			False :保存NG
//
Bool JSVC_ResponseJobNotifyWarning::setData(const char* pRcvData,short ss_size)
{
	Bool bRet = True;
#ifdef _DEBUG_JSVC_SEQ
	MyTraceSD(JSVD_DBG_INFO, "JSVC_ResponseJobNotifyWarning::setData ss_size:" , ss_size );	// 回避方法をトレースに変更.
#endif

	JSVS_ResponseParamJobNotifyWarning *pRcv = (JSVS_ResponseParamJobNotifyWarning*)pRcvData;
	ul_Result = pRcv->ul_Result;
	ul_ResultDetail = pRcv->ul_ResultDetail;

	switch( getMsgTypeId() ){
		case JSVE_JobManager_MsgID_ImageLog_Notify:				// 画像ログからの状態変化通知.
			//フルかどうかチェックする.
			sc_ImageDataFull = pRcv->sc_ImageDataFull;
			break;
		case JSVE_JobManager_MsgID_Warning:						// サブシナリオ警告通知.
			e_Warning = pRcv->ul_Warning;
			break;
		default:
			MyTraceSD( JSVD_DBG_ERROR, "JSVC_ResponseJobNotifyWarning::setData unknown MsgType :" , (int)getMsgTypeId() );	// warning対応 2013/09/18 ooba.
			bRet = False;
			break;
	}

	return bRet;
}

uint32_t JSVC_ResponseJobNotifyWarning::getWarning()						///< 警告情報取得関数.
{
	return e_Warning;
}
uint32_t JSVC_ResponseJobNotifyWarning::getDataType()						///< データ種別取得関数.
{
	return e_DataType;
}

//--------------------------------------------------------------------------------------------------.
//	JSVC_ResponseJobSrvNotifyクラス定義.
//--------------------------------------------------------------------------------------------------.
// コンストラクタ.
JSVC_ResponseJobSrvNotify::JSVC_ResponseJobSrvNotify(uint32_t ul_MsgTypeID)
:	JSVC_Response(ul_MsgTypeID)
, e_FuncSts(0), ul_JobServiceReqID(0)
{
}
// 自クラスの型を返す.
JSVE_Request_Class_Type JSVC_ResponseJobSrvNotify::WhoAmI()
{
	return JSVE_Request_Class_Type_Self_Notify;
}
//
// リクエスト受付時のデータ保存処理.
//
//	return	True　:保存OK
//			False :保存NG
//
Bool JSVC_ResponseJobSrvNotify::setData(const char* pRcvData,short ss_size)
{
	Bool bRet = True;
#ifdef _DEBUG_JSVC_SEQ
	MyTraceSD(JSVD_DBG_INFO, "JSVC_ResponseJobSrvNotify::setData ss_size:" , ss_size );	// 回避方法をトレースに変更.
#endif

	JSVS_ResponseParamJobSrvNotify *pRcv = (JSVS_ResponseParamJobSrvNotify*)pRcvData;
	ul_Result = pRcv->ul_Result;
	ul_ResultDetail = pRcv->ul_ResultDetail;

	switch( getMsgTypeId() ){
		case JSVE_JobManager_MsgID_AnsXmlAnalyze:				// XML解析応答.
			//個別パラメータなし.
			ul_SubScenarioID = pRcv->ul_SubScenarioID;
			break;
		default:
			MyTraceSD( JSVD_DBG_ERROR, "JSVC_ResponseJobSrvNotify::setData unknown MsgType :" , (int)getMsgTypeId() );	// warning対応 2013/09/18 ooba.
			bRet = False;
			break;
	}

	return bRet;
}
//--------------------------------------------------------------------------------------------------.
//	JSVC_ResponseJobNotifyStatusクラス定義.
//--------------------------------------------------------------------------------------------------.
JSVC_ResponseJobNotifyStatus::JSVC_ResponseJobNotifyStatus(uint32_t ul_MsgTypeID)
:	JSVC_Response(ul_MsgTypeID),ul_Index(0),ul_Size(0),ul_DataType(0)
{
	memset(&un_Value, 0 ,sizeof(un_Value) );
}
// 自クラスの型を返す.
JSVE_Request_Class_Type JSVC_ResponseJobNotifyStatus::WhoAmI()
{
	return JSVE_Request_Class_Type_Res_Notify_Sts;
}
//
// リクエスト受付時のデータ保存処理.
//
//	return	True　:保存OK
//			False :保存NG
//
Bool JSVC_ResponseJobNotifyStatus::setData(const char* pRcvData,short ss_size)
{
	Bool bRet = True;

	JSVS_ResponseParamJobNotifyStatus *pRcv = (JSVS_ResponseParamJobNotifyStatus*)pRcvData;
	ul_Result = pRcv->ul_Result;
	ul_ResultDetail = pRcv->ul_ResultDetail;

	switch( getMsgTypeId() ){
		case JSVE_JobManager_MsgID_StatusChange:				// ステータス変化通知（JobStatus変更）.
		case JSVE_JobManager_MsgID_recieveSecureJob:			// .
		case JSVE_JobManager_MsgID_StopTargetJobWasDeleted:		// comment Sky新大阪 [特ROM対応][IT5-6] プリントエラー解除時に他人のジョブの混在を防止する対応.

// comment [特ROM対応][IT5-26] エラー発生時のジョブの削除方法を自動又は手動に設定可能とする対応 ->.
		case JSVE_JobManager_MsgID_StartActiveJobListDeleteSet:		// ジョブ自動削除開始のオブザーバ通知.
		case JSVE_JobManager_MsgID_EndActiveJobListDeleteSet:		// ジョブ自動削除終了のオブザーバ通知.
// comment [特ROM対応][IT5-26] エラー発生時のジョブの削除方法を自動又は手動に設定可能とする対応 <-.

			if ( (uint32_t)ss_size <= sizeof(JSVS_ResponseParamJobNotifyStatus) )		// warning対応 2013/09/21 ooba.
			{
				ul_SubScenarioID = pRcv->ul_SubScenarioID;		// サブシナリオＩＤ.
				pJob			 = pRcv->pJob			;		// ジョブインスタンスのポインタ.
				ul_Index = pRcv->ul_Index;						// ステータス情報の種類.
				ul_Size = pRcv->ul_Size;						// un_Valueの実サイズ.
				memcpy( &un_Value , &(pRcv->un_Value) ,sizeof(un_Value) );
			}
			break;
		case JSVE_JobManager_MsgID_FunctionStatusChange:		// Function状態変化通知.
			//FunctionSysはResultDetailに入る.
			ul_Index = pRcv->ul_Index;							// ステータス情報の種類.
			ul_SubScenarioID = pRcv->ul_SubScenarioID;			// サブシナリオＩＤ.
			pJob			 = pRcv->pJob			;			// ジョブインスタンスのポインタ.
			break;
		default:
			MyTraceSD( JSVD_DBG_ERROR, "JSVC_ResponseJobNotifyStatus::setData unknown MsgType :" , (int)getMsgTypeId() );	// warning対応 2013/09/18 ooba.
			bRet = False;
			break;
	}

	return bRet;
}

uint32_t JSVC_ResponseJobNotifyStatus::getIndex()							///< ステータス情報種類取得.
{
	return ul_Index;
}
uint32_t JSVC_ResponseJobNotifyStatus::getDataType()						///< ステータス情報データ型取得関数.
{
	return getIndex();
}
IPCU_DetailValue* JSVC_ResponseJobNotifyStatus::getValuePointer()					///< ステータス情報のインスタンスポインタ取得.
{
	return &un_Value;
}

//--------------------------------------------------------------------------------------------------.
//	JSVC_ResponseJobFunctionInfoクラス定義.
//--------------------------------------------------------------------------------------------------.
JSVC_ResponseJobFunctionInfo::JSVC_ResponseJobFunctionInfo(uint32_t ul_MsgTypeID)
:	JSVC_Response(ul_MsgTypeID), pFile(NULL)
	,ul_FilePathSize(0), ul_JobServiceReqID(0)
{
}
JSVC_ResponseJobFunctionInfo::~JSVC_ResponseJobFunctionInfo()
{
	if ( NULL != pFile ){
		free(pFile);
		pFile = NULL;
	}
}

// 自クラスの型を返す.
JSVE_Request_Class_Type JSVC_ResponseJobFunctionInfo::WhoAmI()
{
	return JSVE_Request_Class_Type_Res_Func_Info;
}
//
// リクエスト受付時のデータ保存処理.
//
//	return	True　:保存OK
//			False :保存NG
//
Bool JSVC_ResponseJobFunctionInfo::setData(const char* pRcvData,short ss_size)
{
	Bool bRet = True;
	if ( ss_size != sizeof(JSVS_ResponseParamJobFunctionInfo) ) {
		MyTraceSD(JSVD_DBG_ERROR, "JSVC_ResponseJobFunctionInfo::setData size failed" , ss_size );
	}// warning回避.

	JSVS_ResponseParamJobFunctionInfo *pRcv = (JSVS_ResponseParamJobFunctionInfo*)pRcvData;
	ul_Result = pRcv->ul_Result;
	ul_ResultDetail = pRcv->ul_ResultDetail;

	// JSVC_ResponseJobFunctionInfo::pFileはデストラクタで.
	// 自動でfreeされるので、newではなくmallocで取得すること.
	// また同様の理由により、開放したらNULLクリアしておくこと.
	switch( getMsgTypeId() ){
		case JSVE_JobManager_MsgID_FunctionInfoAccept:			// ファンクション情報取得応答.
			ul_FilePathSize = pRcv->ul_FilePathSize ;
			ul_JobServiceReqID = pRcv->ul_JobServiceReqID ;
			if ( ul_FilePathSize > 0 ) {
				if( pFile != NULL ) {	// もし獲得済であれば開放してから.
					free( pFile ) ;
					pFile = NULL ;
				}
				pFile = (char*)malloc(ul_FilePathSize + 1) ; 
				if ( pFile == NULL ) {
					MyTraceSS(JSVD_DBG_ERROR, "JSVC_ResponseJobFunctionInfo::setData failed" , "malloc" );
					bRet = False;
				} else {
// CQ372043対応 Sky赤松 2012.09.20 同一不具合の確認による修正 Start.
					memset( pFile, 0x00, ul_FilePathSize + 1);
// CQ372043対応 Sky赤松 2012.09.20 同一不具合の確認による修正 End.
					strncpy( pFile, pRcv->pFile, ul_FilePathSize ) ;
				}
			}
			break;
		default:
			MyTraceSD(JSVD_DBG_ERROR, "JSVC_ResponseJobFunctionInfo::setData illegal MsgTypeID" , (int)getMsgTypeId() );
			bRet = False;
			break;
	}

	return bRet;
}

// 送信時に確保すべきデータサイズ.
short JSVC_ResponseJobFunctionInfo::getSndDataSize()
{
	switch (getMsgTypeId())
	{
	case JSVE_JobManager_MsgID_FunctionInfoAccept:					// ファンクション情報取得応答.
		return sizeof(JSVS_FunctionInfoAnsMsg);
	default:														// その他.
		// ありえないが念のため.
		return -1;
	}
}

// 送信時にデータを設定する.
short JSVC_ResponseJobFunctionInfo::MakeDataStream(JSVC_RequestBase* pc_Request, char* pSndData)
{
	short ss_Result = -1;

	switch (getMsgTypeId())
	{
	case JSVE_JobManager_MsgID_FunctionInfoAccept:					// ファンクション情報取得応答.
		{
			JSVS_FunctionInfoAnsMsg* ps_Buffer = (JSVS_FunctionInfoAnsMsg*) pSndData;
			// 結果.
			ps_Buffer->e_JobServiceResult = ul_Result;
			// 結果詳細.
			ps_Buffer->e_JobServiceResultDetail = ul_ResultDetail;
			// 文字列長チェック.
			if (ul_FilePathSize > JSVD_XMLFILEPATH_MAX)
			{
				// オーバーしている場合は最大サイズ.
				ps_Buffer->ul_InfoLen = JSVD_XMLFILEPATH_MAX;
			}
			else
			{
				// 実際に格納されているサイズ.
				ps_Buffer->ul_InfoLen = ul_FilePathSize;
			}
			// ファイルパス.
			strncpy(ps_Buffer->sc_NotifyData, pFile, ps_Buffer->ul_InfoLen);
			ss_Result = sizeof(JSVS_FunctionInfoAnsMsg);
		}
		break;
	default:														// その他.
		// ありえないが念のため.
		break;
	}

	return ss_Result;
}

// リクエスト受付時のデータ保存処理.
//
//	return	True　:保存OK
//			False :保存NG
//
// コンストラクタ.
JSVC_ResponseOneShot::JSVC_ResponseOneShot(uint32_t ul_MsgTypeID)
:	JSVC_Response(ul_MsgTypeID),b_Value(False),ss_Value(0)
{
}

// デストラクタ.
JSVC_ResponseOneShot::~JSVC_ResponseOneShot()
{
}

JSVE_Request_Class_Type JSVC_ResponseOneShot::WhoAmI()
{
	return JSVE_Request_Class_Type_OneShot;
}

Bool JSVC_ResponseOneShot::setData(const char* pRcvData,short ss_size)
{
	Bool bRet = True;

	if( pRcvData != NULL && ss_size != 0 )
	{
		JSVS_ResponseParamOneShot *pRcv = (JSVS_ResponseParamOneShot*)pRcvData;
		b_Value = pRcv->b_Value;
		ss_Value = pRcv->ss_Value;
	}
	else
	{
		bRet = False;
	}

	return bRet;
}

// コンストラクタ.
JSVC_ResponseOneShotSetJobLog::JSVC_ResponseOneShotSetJobLog(uint32_t ul_MsgTypeID)
:	JSVC_Response(ul_MsgTypeID),b_Value(False),e_Result(JLGE_Result_Ok),sl_Value(0),us_Value1(0),us_Value2(0),pFile(NULL)
{
}

// デストラクタ.
JSVC_ResponseOneShotSetJobLog::~JSVC_ResponseOneShotSetJobLog()
{
	if( pFile ) {
		free(pFile);
		pFile = NULL;
	}
}
JSVE_Request_Class_Type JSVC_ResponseOneShotSetJobLog::WhoAmI()
{
	return JSVE_Request_Class_Type_OneShot_Log;
}

Bool JSVC_ResponseOneShotSetJobLog::setData(const char* pRcvData,short ss_size)
{
	Bool bRet = True;
	short ss_WorkSize = 0;
#ifdef _DEBUG_JSVC_SEQ
	MyTraceSD(JSVD_DBG_INFO, "JSVC_ResponseOneShotSetJobLog::setData ss_size:" , ss_size );	// 回避方法をトレースに変更.
#endif

	if( pRcvData != NULL )
	{
		JSVS_ResponseParamOneShotSetJobLog *pRcv = (JSVS_ResponseParamOneShotSetJobLog*)pRcvData;
		b_Value = pRcv->b_Value;
		e_Result = pRcv->e_Result;
		sl_Value = pRcv->sl_Value;
		us_Value1 = pRcv->us_Value1;
		us_Value2 = pRcv->us_Value2;

		ss_WorkSize = (sizeof(b_Value) + sizeof(e_Result) + sizeof(sl_Value) + sizeof(us_Value1) + sizeof(us_Value2));
		if( ss_WorkSize > 0 ){
			if( pFile != NULL ){	// もし獲得済であれば開放してから.
				free(pFile);
				pFile = NULL;
			}
			pFile = (char*)malloc(ss_WorkSize+1); 
//Str【 静的解析対応 : Sky田村 : 2012.09.03 : CID-439632 】.
			if( pFile == NULL )
			{
				MyTraceSS( JSVD_DBG_ERROR, "JSVC_ResponseOneShotSetJobLog::setData ", "pFile is NULL(malloc Error)" );
				bRet = False;
			}
			else
			{
// CQ372043対応 Sky赤松 2012.09.20 同一不具合の確認による修正 Start.
				memset( pFile, 0x00, ss_WorkSize+1);
// CQ372043対応 Sky赤松 2012.09.20 同一不具合の確認による修正 End.

				strncpy(pFile,pRcv->sc_Value,ss_WorkSize);
			}
//End【 静的解析対応 : Sky田村 : 2012.09.03 : CID-439632 】.
		}
	}
	else
	{
		MyTraceSS( JSVD_DBG_ERROR, "JSVC_ResponseOneShotSetJobLog::setData ", "RcvData is NULL" );
		bRet = False;
	}

	return bRet;
}

// コンストラクタ.
JSVC_ResponseForJobInstance::JSVC_ResponseForJobInstance(uint32_t ul_MsgTypeID)
:	JSVC_Response(ul_MsgTypeID),ul_SubScenarioID(0xffffffff),ul_Data1(0xffffffff),ul_Data2(0xffffffff)
{
}

// デストラクタ.
JSVC_ResponseForJobInstance::~JSVC_ResponseForJobInstance()
{
}
JSVE_Request_Class_Type JSVC_ResponseForJobInstance::WhoAmI()
{
	return JSVE_Request_Class_Type_OneShot;
}
Bool JSVC_ResponseForJobInstance::setData(const char* pRcvData,short ss_size)
{
	Bool bRet = True;

	if( pRcvData != NULL && ss_size == sizeof(JSVS_ResponseParamForJobInstance) ) {
		JSVS_ResponseParamForJobInstance *pRcv = (JSVS_ResponseParamForJobInstance*)pRcvData;
		ul_Data1= pRcv->ul_Data1 ;
		ul_Data2= pRcv->ul_Data2 ;
	} else {
		bRet = False;
	}
	switch ( getMsgTypeId() ) {
	case JSVE_JobManager_MsgID_SetForceChangeTray: 
		ss_tedium = 2 * sizeof(uint32_t) ;
		break ;
	case JSVE_JobManager_MsgID_GetForceChangeTray:
		ss_tedium = 1 * sizeof(uint32_t) ;
		break ;
	case JSVE_JobManager_MsgID_GetDocInfo:
		ss_tedium = 0 * sizeof(uint32_t) ;
		break ;
	case JSVE_JobManager_MsgID_GetPrintAfterSend:
		ss_tedium = 1 * sizeof(uint32_t) ;
		break ;
	case JSVE_JobManager_MsgID_GetPrintStopCause:
		ss_tedium = 1 * sizeof(uint32_t) ;
		break ;
	case JSVE_JobManager_MsgID_GetJimmonErrorPageNum:
		ss_tedium = 1 * sizeof(uint32_t) ;
		break ;
	case JSVE_JobManager_MsgID_VerifyJimmonPassword:
		ss_tedium = 1 * sizeof(uint32_t) ;
		break ;
	case JSVE_JobManager_MsgID_CancelWaitOutput:
		ss_tedium = 1 * sizeof(uint32_t) ;
		break ;
	case JSVE_JobManager_MsgID_StartRedialJob:
		ss_tedium = 1 * sizeof(uint32_t) ;
		break ;
	case JSVE_JobManager_MsgID_BindCheckAccept:
		ss_tedium = 0 * sizeof(uint32_t) ;			//2017/10/19 AR1030163 doi change.
		break ;
	case JSVE_JobManager_MsgID_WaitOutputAccept:
		ss_tedium = 1 * sizeof(uint32_t) ;
		break ;
	case JSVE_JobManager_MsgID_InputBoxAccept:
		ss_tedium = 1 * sizeof(uint32_t) ;
		break ;
	case JSVE_JobManager_MsgID_startManualPrint:
		ss_tedium = 0 * sizeof(uint32_t) ;
		break ;
	// MZ-83 FAX送信Jobの優先送信機能対応 2013/08/20 S.Higaki start.
	case JSVE_JobManager_MsgID_startManualFax:
		ss_tedium = 0 * sizeof(uint32_t) ;
		break ;
	// MZ-83 FAX送信Jobの優先送信機能対応 2013/08/20 S.Higaki end.
	default:
		MyTraceSD( JSVD_DBG_ERROR, "JSVC_ResponseForJobInstance::setData unknown msgTypeId :" , (int)getMsgTypeId() );	// warning対応 2013/09/18 ooba.
		ss_tedium = -1 ;
		break ;
	}
	return bRet;
}
short JSVC_ResponseForJobInstance::getSndDataSize()
{
	short ss_ret = -1 ;
	if ( ss_tedium != -1 ) {
		// MAX は JSVS_OneShotForJobInstanceAnsMsg だが余分なサイズ分を引く.
		ss_ret = sizeof(JSVS_OneShotForJobInstanceAnsMsg) - ss_tedium ;
	} else {
		// 照合する冗長サイズが見つからない場合、とりあえずフルサイズとっておく.
		ss_ret = sizeof(JSVS_OneShotForJobInstanceAnsMsg) ;
	}

	return ss_ret;
}

short JSVC_ResponseForJobInstance::MakeDataStream(JSVC_RequestBase* pc_Req, char* pSndData)
{
	short ss_ret = -1 ;

	// データ設定.
	if ( pSndData != NULL ) {
		JSVS_OneShotForJobInstanceAnsMsg *pSnd = (JSVS_OneShotForJobInstanceAnsMsg*)pSndData;
		pSnd->ul_SubScenarioID = ((JSVC_RequestForJobInstance*)pc_Req)->getSubScenarioID() ;
		uint32_t ul_Result = ((JSVC_RequestForJobInstance*)pc_Req)->getResult() ;
		uint32_t ul_Detail = ((JSVC_RequestForJobInstance*)pc_Req)->getResultDetail() ;
		if(( ul_Result == JSVD_JobsResult_InitializeValue ) || ( ul_Detail == JSVD_JobsResult_InitializeValue )){
			ul_Result = IPCE_JobServiceResult_NG;
			ul_Detail = IPCE_JobsResultDetail_OutPutError ;
		}
		pSnd->e_JobServiceResult = ul_Result ;
// AR1019645対応 2017/4/28 Soma Start.
		// 除外されることがあるデータは以降で設定する.
		switch ( getMsgTypeId() ) {
		case JSVE_JobManager_MsgID_GetDocInfo:
		case JSVE_JobManager_MsgID_startManualPrint:
		case JSVE_JobManager_MsgID_startManualFax:
		case JSVE_JobManager_MsgID_BindCheckAccept:			//2017/10/19 AR1030163 doi change.
			//ss_tedium = 0 * sizeof(ulong)群 余分データなし.
			pSnd->ul_Data2 = this->ul_Data2 ;
			// breakなし.
		case JSVE_JobManager_MsgID_GetForceChangeTray:
		case JSVE_JobManager_MsgID_GetPrintAfterSend:
		case JSVE_JobManager_MsgID_GetPrintStopCause:
		case JSVE_JobManager_MsgID_GetJimmonErrorPageNum:
		case JSVE_JobManager_MsgID_VerifyJimmonPassword:
		case JSVE_JobManager_MsgID_CancelWaitOutput:
		case JSVE_JobManager_MsgID_StartRedialJob:
		case JSVE_JobManager_MsgID_WaitOutputAccept:
		case JSVE_JobManager_MsgID_InputBoxAccept:
			//ss_tedium = 1 * sizeof(ulong)群 余分データulong型1つ(ul_Data2).
			pSnd->ul_Data1 = this->ul_Data1 ;
			// breakなし.
		case JSVE_JobManager_MsgID_SetForceChangeTray: 
			//ss_tedium = 2 * sizeof(ulong)群 余分データulong型2つ(ul_Data1,ul_Data2).
			pSnd->e_JobServiceResultDetail = ul_Detail ;
			break ;
		default:
			//ss_tedium = -1 異常系となるがサイズはフルサイズ確保しているため全て設定.
			pSnd->e_JobServiceResultDetail = ul_Detail ;
			pSnd->ul_Data1 = this->ul_Data1 ;
			pSnd->ul_Data2 = this->ul_Data2 ;
			break ;
		}
// AR1019645対応 2017/4/28 Soma End.
		if ( ss_tedium != -1 ) {
			// MAX は JSVS_OneShotForJobInstanceAnsMsg だが余分なサイズ分を引く.
			ss_ret = sizeof(JSVS_OneShotForJobInstanceAnsMsg) - ss_tedium ;
		} else {
			// 照合する冗長サイズが見つからない場合、とりあえずフルサイズとっておく.
			ss_ret = sizeof(JSVS_OneShotForJobInstanceAnsMsg) ;
		}
	}

	return ss_ret ;
}
JSVC_ResponseOneShotBase::JSVC_ResponseOneShotBase(uint32_t ul_MsgTypeID)
:	JSVC_Response(ul_MsgTypeID),ul_Data1(0xffffffff)
{
}
// デストラクタ.
JSVC_ResponseOneShotBase::~JSVC_ResponseOneShotBase()
{
}
JSVE_Request_Class_Type JSVC_ResponseOneShotBase::WhoAmI()
{
	return JSVE_Request_Class_Type_OneShot;
}
Bool JSVC_ResponseOneShotBase::setData(const char* pRcvData,short ss_size)
{
	Bool bRet = True;

	if( pRcvData != NULL && ss_size == sizeof(JSVS_ResponseParamOneShotBase) ) {
		JSVS_ResponseParamOneShotBase *pRcv = (JSVS_ResponseParamOneShotBase*)pRcvData;
		ul_Data1= pRcv->ul_Data1 ;
	} else if (JSVE_JobManager_MsgID_SetRevivalAddressBookAccept == getMsgTypeId() ){	// 再送信宛先再設定.
		bRet = True;
	} else if (JSVE_JobManager_MsgID_UdateJobWarningAccept == getMsgTypeId() ){
		bRet = True;
	} else {
		bRet = False;
	}
	switch ( getMsgTypeId() ) {
	case JSVE_JobManager_MsgID_GetCurrentJob:
		ss_tedium = 0 * sizeof(uint32_t) ;
		break ;
	case JSVE_JobManager_MsgID_ForceEject:
		ss_tedium = 1 * sizeof(uint32_t) ;
		break ;
	case JSVE_JobManager_MsgID_ExtendPrintStart:
		ss_tedium = 1 * sizeof(uint32_t) ;
		break ;
	case JSVE_JobManager_MsgID_GetSendModeForRightFax:
		ss_tedium = 0 * sizeof(uint32_t) ;
		break ;
	case JSVE_JobManager_MsgID_InterruptModeEnd:
		ss_tedium = 0 * sizeof(uint32_t) ;
		break ;
	case JSVE_JobManager_MsgID_SetRevivalAddressBookAccept:	// 再送信宛先再設定.
	case JSVE_JobManager_MsgID_UdateJobWarningAccept:
		ss_tedium = 0;
		break;
	default:
		MyTraceSD( JSVD_DBG_ERROR, "JSVC_ResponseOneShotBase::setData unknown msgTypeId :" , (int)getMsgTypeId() );	// warning対応 2013/09/18 ooba.
		ss_tedium = -1 ;
		break ;
	}
	return bRet;
}
short JSVC_ResponseOneShotBase::getSndDataSize()
{
	short ss_ret = -1;
	switch( getMsgTypeId() ){
		case JSVE_JobManager_MsgID_SetRevivalAddressBookAccept:
		case JSVE_JobManager_MsgID_UdateJobWarningAccept:
			ss_ret = sizeof(JSVS_OneShotReq_AnsMsg) ;
#if JSVC_Request_ResourceWarningDBG
			printf("★★★★★UdateJobWarning::getSndDataSize ss_ret[%d]\n", ss_ret);
#endif
			break;
		case JSVE_JobManager_MsgID_InterruptModeEnd:
			ss_ret = sizeof(JSVS_NoticeMsg_Base);
			break;
		default:
			ss_ret = sizeof(JSVS_OneShotAnsMsg) ;
			// [changelog] 2018/01/15 Cuong #245407 Fix static check for IT5_4.2 F00-U2(07) start.
			break;
			// [changelog] 2018/01/15 Cuong #245407 Fix static check for IT5_4.2 F00-U2(07) end.
	}
	return ss_ret;
}

short JSVC_ResponseOneShotBase::MakeDataStream(JSVC_RequestBase* pc_Req, char* pSndData)
{
	short ss_ret = -1 ;

	// データ設定.
	if ( pSndData != NULL ) {
		switch( getMsgTypeId() ){
			case JSVE_JobManager_MsgID_SetRevivalAddressBookAccept:
			case JSVE_JobManager_MsgID_UdateJobWarningAccept:
			{
				//送信データバッファを送信型にキャスト.
				JSVS_OneShotReq_AnsMsg* pMsg = (JSVS_OneShotReq_AnsMsg*)pSndData;
				
				pMsg->e_JobServiceResult	= getResult();			//結果の格納.
				pMsg->ul_Param				= getResultDetail();	//結果詳細の格納.
				ss_ret = sizeof(JSVS_OneShotReq_AnsMsg);			//データ部のサイズを指定.

#if JSVC_Request_ResourceWarningDBG
				if (JSVE_JobManager_MsgID_UdateJobWarningAccept == getMsgTypeId()){
					printf("★★★★★UdateJobWarning::MakeDataStream getResult      [%d]\n", getResult());
					printf("★★★★★UdateJobWarning::MakeDataStream getResultDetail[%d]\n", getResultDetail());
					printf("★★★★★UdateJobWarning::MakeDataStream ss_ret         [%d]\n", ss_ret);
				}
#endif
			}
				break;
				
			case JSVE_JobManager_MsgID_InterruptModeEnd:
				JSVS_NoticeMsg_Base* pNoticeMsg_Base;
				pNoticeMsg_Base = (JSVS_NoticeMsg_Base*)pSndData;

				pNoticeMsg_Base->ul_Param1 = ul_Data1;
				ss_ret = sizeof( JSVS_NoticeMsg_Base );
				break;
			default:
			{
				JSVS_OneShotAnsMsg *pSnd = (JSVS_OneShotAnsMsg*)pSndData;
				uint32_t ul_Result = ((JSVC_RequestOneShot*)pc_Req)->getResult() ;
				uint32_t ul_Detail = ((JSVC_RequestOneShot*)pc_Req)->getResultDetail() ;
				if(( ul_Result == JSVD_JobsResult_InitializeValue ) || ( ul_Detail == JSVD_JobsResult_InitializeValue )){
					ul_Result = IPCE_JobServiceResult_NG;
					ul_Detail = IPCE_JobsResultDetail_OutPutError ;
				}
				pSnd->e_JobServiceResult = ul_Result ;
				pSnd->e_JobServiceResultDetail = ul_Detail ;
				pSnd->ul_Data1 = this->ul_Data1 ;

				if ( ss_tedium != -1 ) {
					// MAX は JSVS_OneShotAnsMsg だが余分なサイズ分を引く.
					ss_ret = sizeof(JSVS_OneShotAnsMsg) - ss_tedium ;
				} else {
					// 照合する冗長サイズが見つからない場合、とりあえずフルサイズとっておく.
					ss_ret = sizeof(JSVS_OneShotAnsMsg) ;
				}
			}
			break;
		}
	}

	return ss_ret ;
}

// コンストラクタ.
JSVC_ResponseOneShot_Log::JSVC_ResponseOneShot_Log(uint32_t ul_MsgTypeID)
	: JSVC_Response(ul_MsgTypeID)
	, ul_Param1(0)
	, ul_Param2(0)
	, vp_LogData(NULL)
{
	/* NOP */
}

// デストラクタ.
JSVC_ResponseOneShot_Log::~JSVC_ResponseOneShot_Log()
{
	// 動的確保領域が存在していれば解放する.
	if (vp_LogData)
	{
		free( vp_LogData );
		vp_LogData = NULL;
	}
}

// 自身のレスポンスクラス種別を返す.
JSVE_Request_Class_Type JSVC_ResponseOneShot_Log::WhoAmI()
{
	// 固定値.
	return JSVE_Request_Class_Type_Res_OneShotLog;
}

// 通知データバッファの保存処理.
Bool JSVC_ResponseOneShot_Log::setData(const char* pRcvData, short ss_size)
{
	Bool b_Result = False;
	JSVS_ResponseParam_OneShotLog* pst_Data = (JSVS_ResponseParam_OneShotLog*)pRcvData;

	// 共通.
	ul_Result = pst_Data->ul_Result;
	
	// データバッファを解放.
	if (vp_LogData)
	{
		free( vp_LogData );
		vp_LogData = NULL;
	}

	// MsgTypeIDに従ってデータバッファを保存する.
	switch ( getMsgTypeId() )
	{
	// データはDataBaseID.
	case JSVE_JobManager_MsgID_GetSubScenarioLog_DataBaseID:	// サブシナリオ履歴データベースID取得要求.
	case JSVE_JobManager_MsgID_GetFaxJournal_DataBaseID:		// FaxジャーナルデータベースID取得要求.
	case JSVE_JobManager_MsgID_GetScanJournal_DataBaseID:		// ScanジャーナルデータベースID取得要求.
	case JSVE_JobManager_MsgID_GetReceiveRef_DataBaseID:		// 受信履歴データベースID取得要求.
		{
			// メモリ確保してデータバッファを保存.
			vp_LogData = malloc(sizeof(IPCS_DataBaseConfig));
			if (vp_LogData)
			{
				memset( vp_LogData, 0x00, sizeof(IPCS_DataBaseConfig));
				memcpy( vp_LogData, &pst_Data->st_DBConfig, sizeof(IPCS_DataBaseConfig));
				b_Result = True;
			}
			else
			{
				// メモリ確保失敗.
			}
		}
		break;
		
	// データは文字列.
	case JSVE_JobManager_MsgID_GetSendLog_DataBaseID:			// 送信履歴データベースID取得要求.
	case JSVE_JobManager_MsgID_JobLog_GetLogFilePath:			// ジョブログパス取得要求.
	case JSVE_JobManager_MsgID_JobLog_GetWebDavLogFilePath:		// ジョブログパス取得要求（WebDav）.
		{
			// メモリ確保して文字列を保存.
			vp_LogData = malloc(strlen(pst_Data->sc_FilePath)+1);
			if (vp_LogData)
			{
// CQ372043対応 Sky赤松 2012.09.20 Start.
				memset( vp_LogData, 0x00, sizeof(char) * (strlen(pst_Data->sc_FilePath)+1));
// CQ372043対応 Sky赤松 2012.09.20 End.
				strncpy(static_cast<char*>(vp_LogData), pst_Data->sc_FilePath, strlen(pst_Data->sc_FilePath));
				b_Result = True;
			}
			else
			{
				// メモリ確保失敗.
			}
		}
		break;
		
	// データは値.
	// ログバージョン取得要求のみ2つあるので、予め2つ目を保持してから流す.
	case JSVE_JobManager_MsgID_JobLog_GetLogVersion:			// ログバージョン取得要求.
		ul_Param2 = pst_Data->ul_Param2;
		/* fall-throw */
	case JSVE_JobManager_MsgID_GetFaxJournal_RegNum:			// Faxジャーナル登録数取得要求.
	case JSVE_JobManager_MsgID_GetScanJournal_RegNum:			// Scanジャーナル登録数取得要求.
	case JSVE_JobManager_MsgID_GetSendLogSubScenarioNum:		// 送信履歴登録サブシナリオ数取得要求.
	case JSVE_JobManager_MsgID_GetSendLogAddressNum:			// サブシナリオ毎の送信先数取得要求.
	case JSVE_JobManager_MsgID_GetSendLogSubScenarioNo:			// サブシナリオ番号取得要求.
	case JSVE_JobManager_MsgID_JobLog_AddApplication:			// ジョブログ登録要求（操作系）.
	case JSVE_JobManager_MsgID_JobLog_AddPlatform:				// ジョブログ登録要求（P/Fから）.
	case JSVE_JobManager_MsgID_JobLog_GetComplete_getLogFile:	// ジョブログファイル取得完了取得要求.
	case JSVE_JobManager_MsgID_JobLog_GetLogFileState:			// ジョブログファイル作成状況取得要求.
	case JSVE_JobManager_MsgID_JobLog_GetFileExist:				// ジョブログファイル存在有無取得要求.
	case JSVE_JobManager_MsgID_JobLog_GetLimitOver:				// ジョブログファイル化上限超過有無取得要求.
	case JSVE_JobManager_MsgID_JobLog_GetJobLogStatus:			// ジョブログ状況取得要求.
	case JSVE_JobManager_MsgID_JobLog_GetDataStatus:			// ログ保存状況取得要求.
	case JSVE_JobManager_MsgID_JobLog_GetPreserve:				// 一時ログ情報の有無取得要求.
	case JSVE_JobManager_MsgID_JobLog_GetOverWrite:				// 一時ログ情報上書き有無取得要求.
	case JSVE_JobManager_MsgID_JobLog_IsNearFull:				// NearFull判定要求.
	case JSVE_JobManager_MsgID_JobLog_IsFull:					// Full判定要求.
	case JSVE_JobManager_MsgID_JobLogStatus:					// ジョブログ状態変化通知.
	case JSVE_JobManager_MsgID_JobLogPreserve:					// フル/ニアフル状態変化通知.
	case JSVE_JobManager_MsgID_TempLogStatus:					// 一時ログ状態変化通知.
	case JSVE_JobManager_MsgID_JobLogFileStatus:				// ファイル状態変化通知.
	case JSVE_JobManager_MsgID_JobLogFileMake:					// ファイル状態変化通知(作成終了or失敗).
	case JSVE_JobManager_MsgID_JobLogFileProgress:				// ファイル作成状況通知.
	case JSVE_JobManager_MsgID_DataBase_Update:					// データベース更新通知.
	case JSVE_JobManager_MsgID_JobLog_CreateFileAccept:			// ファイル作成要求受付.
	case JSVE_JobManager_MsgID_DeleteLogAccept:					// ログ削除要求受付.
	case JSVE_JobManager_MsgID_HistoryDelete:					// 履歴削除結果通知.
		ul_Param1 = pst_Data->ul_Param1;
		b_Result = True;
		break;
		
	// データなし.
	case JSVE_JobManager_MsgID_AddSubScenarioLog:				// サブシナリオ履歴登録要求.
	case JSVE_JobManager_MsgID_StartSendLogAccess:				// 送信履歴の取得アクセス開始要求.
	case JSVE_JobManager_MsgID_EndSendLogAccess:				// 送信履歴の取得アクセス終了要求.
	case JSVE_JobManager_MsgID_JobLog_SetComplete_getLogFile:	// ジョブログファイル取得完了設定要求.
	case JSVE_JobManager_MsgID_RegistImageLogAccept:			// アプリからの画像ログ登録受付.
	case JSVE_JobManager_MsgID_RegistImageLogNotify:			// アプリからの画像ログ登録結果.
	case JSVE_JobManager_MsgID_reqListPrintCancelDoc:			// 重たい処理解除通知受信要求.
	case JSVE_JobManager_MsgID_RevivalJudgmentEnd:				// 復活判断完了通知.
		b_Result = True;
		break;
		
	default:
		MyTraceSS(JSVD_DBG_ERROR, "JSVC_ResponseOneShot_Log::setData", "invalid msg id");
		break;
	}

	return b_Result;
}

// 通知データサイズの取得.
short JSVC_ResponseOneShot_Log::getSndDataSize()
{
	// MsgTypeIDに従って適切なサイズを返す.
	switch ( getMsgTypeId() )
	{
	case JSVE_JobManager_MsgID_GetSubScenarioLog_DataBaseID:	// サブシナリオ履歴データベースID取得要求.
	case JSVE_JobManager_MsgID_GetFaxJournal_DataBaseID:		// FaxジャーナルデータベースID取得要求.
	case JSVE_JobManager_MsgID_GetScanJournal_DataBaseID:		// ScanジャーナルデータベースID取得要求.
	case JSVE_JobManager_MsgID_GetReceiveRef_DataBaseID:		// 受信履歴データベースID取得要求.
		return sizeof(JSVS_OneShotReq_AnsMsg_DBConfig);
		
	case JSVE_JobManager_MsgID_AddSubScenarioLog:				// サブシナリオ履歴登録要求.
	case JSVE_JobManager_MsgID_StartSendLogAccess:				// 送信履歴の取得アクセス開始要求.
	case JSVE_JobManager_MsgID_EndSendLogAccess:				// 送信履歴の取得アクセス終了要求.
	case JSVE_JobManager_MsgID_JobLog_SetComplete_getLogFile:	// ジョブログファイル取得完了設定要求.
	case JSVE_JobManager_MsgID_GetFaxJournal_RegNum:			// Faxジャーナル登録数取得要求.
	case JSVE_JobManager_MsgID_GetScanJournal_RegNum:			// Scanジャーナル登録数取得要求.
	case JSVE_JobManager_MsgID_GetSendLogSubScenarioNum:		// 送信履歴登録サブシナリオ数取得要求.
	case JSVE_JobManager_MsgID_GetSendLogAddressNum:			// サブシナリオ毎の送信先数取得要求.
	case JSVE_JobManager_MsgID_GetSendLogSubScenarioNo:			// サブシナリオ番号取得要求.
	case JSVE_JobManager_MsgID_JobLog_AddApplication:			// ジョブログ登録要求（操作系）.
	case JSVE_JobManager_MsgID_JobLog_AddPlatform:				// ジョブログ登録要求（P/Fから）.
	case JSVE_JobManager_MsgID_JobLog_GetComplete_getLogFile:	// ジョブログファイル取得完了取得要求.
	case JSVE_JobManager_MsgID_JobLog_GetLogFileState:			// ジョブログファイル作成状況取得要求.
	case JSVE_JobManager_MsgID_JobLog_GetFileExist:				// ジョブログファイル存在有無取得要求.
	case JSVE_JobManager_MsgID_JobLog_GetLimitOver:				// ジョブログファイル化上限超過有無取得要求.
	case JSVE_JobManager_MsgID_JobLog_GetJobLogStatus:			// ジョブログ状況取得要求.
	case JSVE_JobManager_MsgID_JobLog_GetDataStatus:			// ログ保存状況取得要求.
	case JSVE_JobManager_MsgID_JobLog_GetPreserve:				// 一時ログ情報の有無取得要求.
	case JSVE_JobManager_MsgID_JobLog_GetOverWrite:				// 一時ログ情報上書き有無取得要求.
	case JSVE_JobManager_MsgID_JobLog_IsNearFull:				// NearFull判定要求.
	case JSVE_JobManager_MsgID_JobLog_IsFull:					// Full判定要求.
	case JSVE_JobManager_MsgID_JobLogStatus:					// ジョブログ状態変化通知.
	case JSVE_JobManager_MsgID_JobLogPreserve:					// フル/ニアフル状態変化通知.
	case JSVE_JobManager_MsgID_TempLogStatus:					// 一時ログ状態変化通知.
	case JSVE_JobManager_MsgID_JobLogFileStatus:				// ファイル状態変化通知.
	case JSVE_JobManager_MsgID_JobLogFileMake:					// ファイル状態変化通知(作成終了or失敗).
	case JSVE_JobManager_MsgID_JobLogFileProgress:				// ファイル作成状況通知.
	case JSVE_JobManager_MsgID_DataBase_Update:					// データベース更新通知.
	case JSVE_JobManager_MsgID_HistoryDelete:					// 履歴削除結果通知.
	case JSVE_JobManager_MsgID_JobLog_CreateFileAccept:			// ファイル作成要求受付.
	case JSVE_JobManager_MsgID_DeleteLogAccept:					// ログ削除要求受付.
	case JSVE_JobManager_MsgID_RegistImageLogAccept:			// アプリからの画像ログ登録受付.
	case JSVE_JobManager_MsgID_RegistImageLogNotify:			// アプリからの画像ログ登録結果.
	case JSVE_JobManager_MsgID_reqListPrintCancelDoc:			// 重たい処理解除通知受信要求.
		return sizeof( JSVS_OneShotReq_AnsMsg );
		
	case JSVE_JobManager_MsgID_GetSendLog_DataBaseID:			// 送信履歴データベースID取得要求.
	case JSVE_JobManager_MsgID_JobLog_GetLogFilePath:			// ジョブログパス取得要求.
	case JSVE_JobManager_MsgID_JobLog_GetWebDavLogFilePath:		// ジョブログパス取得要求（WebDav）.
		return sizeof( JSVS_OneShotReq_AnsMsg_File );
		
	case JSVE_JobManager_MsgID_JobLog_GetLogVersion:			// ログバージョン取得要求.
		return sizeof( JSVS_OneShotReq_AnsMsg_Option );
		
	case JSVE_JobManager_MsgID_RevivalJudgmentEnd:				// 復活判断完了通知.
		return 0;
		
	default:
		// ありえない.
		return -1;
	}
}

// 通信データバッファ生成処理.
short JSVC_ResponseOneShot_Log::MakeDataStream(JSVC_RequestBase* pc_Req, char* pSndData)
{
	short ss_Result = -1;

	// MsgTypeIDに従って適切なデータバッファを生成する.
	switch ( getMsgTypeId() )
	{
	// 応答パラメータが結果のみ.
	case JSVE_JobManager_MsgID_AddSubScenarioLog:				// サブシナリオ履歴登録要求.
	case JSVE_JobManager_MsgID_StartSendLogAccess:				// 送信履歴の取得アクセス開始要求.
	case JSVE_JobManager_MsgID_EndSendLogAccess:				// 送信履歴の取得アクセス終了要求.
	case JSVE_JobManager_MsgID_JobLog_SetComplete_getLogFile:	// ジョブログファイル取得完了設定要求.
	case JSVE_JobManager_MsgID_RegistImageLogAccept:			// アプリからの画像ログ登録受付.
	case JSVE_JobManager_MsgID_RegistImageLogNotify:			// アプリからの画像ログ登録結果.
		{
			JSVS_OneShotReq_AnsMsg* ps_DataBuff = (JSVS_OneShotReq_AnsMsg*)pSndData;
			ps_DataBuff->e_JobServiceResult = ul_Result;
			ss_Result = sizeof(JSVS_OneShotReq_AnsMsg);
		}
		break;

	// 応答パラメータが結果とDB情報.
	case JSVE_JobManager_MsgID_GetSubScenarioLog_DataBaseID:	// サブシナリオ履歴データベースID取得要求.
	case JSVE_JobManager_MsgID_GetFaxJournal_DataBaseID:		// FaxジャーナルデータベースID取得要求.
	case JSVE_JobManager_MsgID_GetScanJournal_DataBaseID:		// ScanジャーナルデータベースID取得要求.
	case JSVE_JobManager_MsgID_GetReceiveRef_DataBaseID:		// 受信履歴データベースID取得要求.
		{
			JSVS_OneShotReq_AnsMsg_DBConfig* ps_DataBuff = (JSVS_OneShotReq_AnsMsg_DBConfig*)pSndData;
			ps_DataBuff->e_JobServiceResult = ul_Result;
			memcpy((void*)(&ps_DataBuff->st_DBConfig), vp_LogData, sizeof(IPCS_DataBaseConfig));
			ss_Result = sizeof(JSVS_OneShotReq_AnsMsg_DBConfig);
		}
		break;

	// 応答パラメータが結果と1つ.
	case JSVE_JobManager_MsgID_GetFaxJournal_RegNum:			// Faxジャーナル登録数取得要求.
	case JSVE_JobManager_MsgID_GetScanJournal_RegNum:			// Scanジャーナル登録数取得要求.
	case JSVE_JobManager_MsgID_GetSendLogSubScenarioNum:		// 送信履歴登録サブシナリオ数取得要求.
	case JSVE_JobManager_MsgID_GetSendLogAddressNum:			// サブシナリオ毎の送信先数取得要求.
	case JSVE_JobManager_MsgID_GetSendLogSubScenarioNo:			// サブシナリオ番号取得要求.
	case JSVE_JobManager_MsgID_JobLog_AddApplication:			// ジョブログ登録要求（操作系）.
	case JSVE_JobManager_MsgID_JobLog_AddPlatform:				// ジョブログ登録要求（P/Fから）.
	case JSVE_JobManager_MsgID_JobLog_GetComplete_getLogFile:	// ジョブログファイル取得完了取得要求.
	case JSVE_JobManager_MsgID_JobLog_GetLogFileState:			// ジョブログファイル作成状況取得要求.
	case JSVE_JobManager_MsgID_JobLog_GetFileExist:				// ジョブログファイル存在有無取得要求.
	case JSVE_JobManager_MsgID_JobLog_GetLimitOver:				// ジョブログファイル化上限超過有無取得要求.
	case JSVE_JobManager_MsgID_JobLog_GetJobLogStatus:			// ジョブログ状況取得要求.
	case JSVE_JobManager_MsgID_JobLog_GetDataStatus:			// ログ保存状況取得要求.
	case JSVE_JobManager_MsgID_JobLog_GetPreserve:				// 一時ログ情報の有無取得要求.
	case JSVE_JobManager_MsgID_JobLog_GetOverWrite:				// 一時ログ情報上書き有無取得要求.
	case JSVE_JobManager_MsgID_JobLog_IsNearFull:				// NearFull判定要求.
	case JSVE_JobManager_MsgID_JobLog_IsFull:					// Full判定要求.
	case JSVE_JobManager_MsgID_JobLogStatus:					// ジョブログ状態変化通知.
	case JSVE_JobManager_MsgID_JobLogPreserve:					// フル/ニアフル状態変化通知.
	case JSVE_JobManager_MsgID_TempLogStatus:					// 一時ログ状態変化通知.
	case JSVE_JobManager_MsgID_JobLogFileStatus:				// ファイル状態変化通知.
	case JSVE_JobManager_MsgID_JobLogFileMake:					// ファイル状態変化通知(作成終了or失敗).
	case JSVE_JobManager_MsgID_JobLogFileProgress:				// ファイル作成状況通知.
	case JSVE_JobManager_MsgID_DataBase_Update:					// データベース更新通知.
	case JSVE_JobManager_MsgID_HistoryDelete:					// 履歴削除結果通知.
	case JSVE_JobManager_MsgID_JobLog_CreateFileAccept:			// ファイル作成要求受付.
	case JSVE_JobManager_MsgID_DeleteLogAccept:					// ログ削除要求受付.
		{
			JSVS_OneShotReq_AnsMsg* ps_DataBuff = (JSVS_OneShotReq_AnsMsg*)pSndData;
			ps_DataBuff->e_JobServiceResult = ul_Result;
			ps_DataBuff->ul_Param = ul_Param1;
			ss_Result = sizeof(JSVS_OneShotReq_AnsMsg);
		}
		break;

	// 応答パラメータが結果と文字列.
	case JSVE_JobManager_MsgID_GetSendLog_DataBaseID:			// 送信履歴データベースID取得要求.
	case JSVE_JobManager_MsgID_JobLog_GetLogFilePath:			// ジョブログパス取得要求.
	case JSVE_JobManager_MsgID_JobLog_GetWebDavLogFilePath:		// ジョブログパス取得要求（WebDav）.
		{
			JSVS_OneShotReq_AnsMsg_File* ps_DataBuff = (JSVS_OneShotReq_AnsMsg_File*)pSndData;
			ps_DataBuff->e_JobServiceResult = ul_Result;
			strncpy(ps_DataBuff->sc_FileName, static_cast<char*>(vp_LogData), strlen(static_cast<char*>(vp_LogData)));
			ss_Result = sizeof(JSVS_OneShotReq_AnsMsg_File);
		}
		break;

	// 応答パラメータが結果と2つ.
	case JSVE_JobManager_MsgID_JobLog_GetLogVersion:			// ログバージョン取得要求.
		{
			JSVS_OneShotReq_AnsMsg_Option* ps_DataBuff = (JSVS_OneShotReq_AnsMsg_Option*)pSndData;
			ps_DataBuff->e_JobServiceResult = ul_Result;
			ps_DataBuff->ul_Param1 = ul_Param1;
			ps_DataBuff->ul_Param2 = ul_Param2;
			ss_Result = sizeof(JSVS_OneShotReq_AnsMsg_Option);
		}
		break;

	case JSVE_JobManager_MsgID_RevivalJudgmentEnd:				// 復活判断完了通知.
		{
			ss_Result = 0;
		}
		break;

	default:
		// ありえない.
		break;
	}

	return ss_Result;
}

JSVC_ResponseJobServiceStatus::JSVC_ResponseJobServiceStatus(uint32_t ul_MsgTypeID)
:	JSVC_Response(ul_MsgTypeID), ul_Data1(0xffffffff)
{
}
JSVC_ResponseJobServiceStatus::~JSVC_ResponseJobServiceStatus()
{
}
JSVE_Request_Class_Type JSVC_ResponseJobServiceStatus::WhoAmI()
{
	return JSVE_Request_Class_Type_Req_SrvState ;
}
Bool JSVC_ResponseJobServiceStatus::setData(const char* pRcvData, short ss_size)
{
	JSVS_ResponseParamJobServiceStatus* pc_Rcv = NULL ;
	Bool bl_Ret = False ;

	if ( pRcvData != NULL && ss_size == sizeof(JSVS_ResponseParamJobServiceStatus) ) {
		pc_Rcv = (JSVS_ResponseParamJobServiceStatus*)pRcvData ;
		ul_Data1 = pc_Rcv->ul_Data1 ;
		bl_Ret = True ;
	}

	return bl_Ret ;
}
short JSVC_ResponseJobServiceStatus::getSndDataSize()
{
	return sizeof(JSVS_JobServiceStatusAnsMsg) ;
}
short JSVC_ResponseJobServiceStatus::MakeDataStream(JSVC_RequestBase* pc_Req, char* pSndData)
{
	short ss_ret = -1 ;

	// データ設定.
	if ( pSndData != NULL ) {
		JSVS_JobServiceStatusAnsMsg *pSnd = (JSVS_JobServiceStatusAnsMsg*)pSndData;
		uint32_t ul_Result = ((JSVC_RequestForReqJobService*)pc_Req)->getResult() ;
		uint32_t ul_Detail = ((JSVC_RequestForReqJobService*)pc_Req)->getResultDetail() ;
		if(( ul_Result == JSVD_JobsResult_InitializeValue ) || ( ul_Detail == JSVD_JobsResult_InitializeValue )){
			ul_Result = IPCE_JobServiceResult_NG;
			ul_Detail = IPCE_JobsResultDetail_OutPutError ;
		}
		pSnd->e_JobServiceResult = ul_Result ;
		pSnd->e_JobServiceResultDetail = ul_Detail ;
		pSnd->sl_StatusConfirm = ((JSVC_RequestForReqJobService*)pc_Req)->getStatusConfirm() ;
		pSnd->ul_Data1 = ul_Data1 ;

		// ここはサイズ固定.
		ss_ret = sizeof(JSVS_JobServiceStatusAnsMsg) ;
	}

	return ss_ret ;
}
JSVC_ResponseJobServiceStatusSub::JSVC_ResponseJobServiceStatusSub(uint32_t ul_MsgTypeID)
:	JSVC_Response(ul_MsgTypeID), ul_Data1(0xffffffff), ul_Data2(0xffffffff)
//Str【 静的解析対応 : Sky赤松 : 2012.12.03 : CID-454959 】.
, ul_Data3(0xffffffff), ul_Data4(0xffffffff), ul_Data5(0xffffffff), f_Data6( 0.0f ), f_Data7( 0.0f )
//End【 静的解析対応 : Sky赤松 : 2012.12.03 : CID-454959 】.
{
}
JSVC_ResponseJobServiceStatusSub::~JSVC_ResponseJobServiceStatusSub()
{
}
JSVE_Request_Class_Type JSVC_ResponseJobServiceStatusSub::WhoAmI()
{
	return JSVE_Request_Class_Type_Req_SrvState ;
}
Bool JSVC_ResponseJobServiceStatusSub::setData(const char* pRcvData,short ss_size)
{
	JSVS_ResponseParamJobServiceStatusSub* pc_Rcv = NULL ;
// CQ367177 対応 2012/08/18 上位に返すメッセージ構造体に、WarningPaperSizeサイズ値の項目を追加する ->.
	JSVS_ResponseParamJobServiceStatusSub_WarningPaperSize* pc_Rcv2 = NULL ;
// CQ367177 対応 2012/08/18 上位に返すメッセージ構造体に、WarningPaperSizeサイズ値の項目を追加する <-.
	Bool bl_Ret = False ;

	if ( pRcvData != NULL && ss_size == sizeof(JSVS_ResponseParamJobServiceStatusSub) ) {

		pc_Rcv = (JSVS_ResponseParamJobServiceStatusSub*)pRcvData ;
		this->ul_Data1 = pc_Rcv->ul_Data1 ;
		this->ul_Data2 = pc_Rcv->ul_Data2 ;
		this->ul_Data3 = pc_Rcv->ul_Data3 ;
		this->ul_Data4 = pc_Rcv->ul_Data4 ;
		this->ul_Data5 = pc_Rcv->ul_Data5 ;
// CQ367177 対応 2012/08/18 上位に返すメッセージ構造体に、WarningPaperSizeサイズ値の項目を追加する ->.
		this->f_Data6  = 0.0f ;
		this->f_Data7  = 0.0f ;
// CQ367177 対応 2012/08/18 上位に返すメッセージ構造体に、WarningPaperSizeサイズ値の項目を追加する <-.
		bl_Ret = True ;
	}
// CQ367177 対応 2012/08/18 上位に返すメッセージ構造体に、WarningPaperSizeサイズ値の項目を追加する ->.
	else if ( pRcvData != NULL && ss_size == sizeof(JSVS_ResponseParamJobServiceStatusSub_WarningPaperSize) )
	{

		pc_Rcv2 = (JSVS_ResponseParamJobServiceStatusSub_WarningPaperSize*)pRcvData ;
		this->ul_Data1 = pc_Rcv2->ul_Data1 ;
		this->ul_Data2 = 0xffffffff ;
		this->ul_Data3 = 0xffffffff ;
		this->ul_Data4 = pc_Rcv2->ul_Data4 ;
		this->ul_Data5 = 0xffffffff ;
		this->f_Data6  = pc_Rcv2->f_Data2 ;
		this->f_Data7  = pc_Rcv2->f_Data3 ;
		bl_Ret = True ;

	}
// CQ367177 対応 2012/08/18 上位に返すメッセージ構造体に、WarningPaperSizeサイズ値の項目を追加する <-.

	return bl_Ret ;
}
short JSVC_ResponseJobServiceStatusSub::getSndDataSize()
{
	return sizeof(JSVS_JobServiceStatusSubAnsMsg) ;
}

short JSVC_ResponseJobServiceStatusSub::MakeDataStream(JSVC_RequestBase* pc_Req, char* pSndData)
{
	short ss_ret = -1 ;

	// データ設定.
	if ( pSndData != NULL ) {
		JSVS_JobServiceStatusSubAnsMsg *pSnd = (JSVS_JobServiceStatusSubAnsMsg*)pSndData;
		pSnd->sl_StatusConfirm = ((JSVC_RequestForReqJobServiceSub*)pc_Req)->getStatusConfirm() ;
		uint32_t ul_Result = ((JSVC_RequestForReqJobServiceSub*)pc_Req)->getResult() ;
		if(ul_Result == JSVD_JobsResult_InitializeValue){
			ul_Result = IPCE_JobServiceResult_NG;
		}
		//detailに関しては元々処理していないため、Resultに関してのみ処理する.
		pSnd->e_JobServiceResult = ul_Result ;
		pSnd->ul_Data1 = this->ul_Data1 ;
		pSnd->ul_Data2 = this->ul_Data2 ;
		pSnd->ul_Data3 = this->ul_Data3 ;
		pSnd->ul_Data4 = this->ul_Data4 ;
		pSnd->ul_Data5 = this->ul_Data5 ;
// CQ367177 対応 2012/08/18 上位に返すメッセージ構造体に、WarningPaperSizeサイズ値の項目を追加する ->.
		pSnd->f_Data6  = this->f_Data6 ;
		pSnd->f_Data7  = this->f_Data7 ;
// CQ367177 対応 2012/08/18 上位に返すメッセージ構造体に、WarningPaperSizeサイズ値の項目を追加する <-.

		switch ( ((JSVC_RequestForReqJobServiceSub*)pc_Req)->getStatusConfirm() ) {
		case IPCE_JobService_JobStatusConfirm_FaxEnable:
			ss_tedium = 4 * sizeof(uint32_t) ;
			break ;
// CQ367177 対応 2012/08/18 上位に返すメッセージ構造体に、WarningPaperSizeサイズ値の項目を追加する ->.
//		case IPCE_JobService_JobStatusConfirm_WarningPaperSize:
//			ss_tedium = 1 * sizeof(ulong) ;
//			break ;
// CQ367177 対応 2012/08/18 上位に返すメッセージ構造体に、WarningPaperSizeサイズ値の項目を追加する <-.
		case IPCE_JobService_JobStatusConfirm_WarningPaperType:
			ss_tedium = 0 * sizeof(uint32_t) ;
			break ;
		case IPCE_JobService_JobStatusConfirm_WarningFeedTray:
			ss_tedium = 3 * sizeof(uint32_t) ;
			break ;
		case IPCE_JobService_JobStatusConfirm_ADFOriginalSizeFixed:
			ss_tedium = 4 * sizeof(uint32_t) ;
			break ;
		case IPCE_JobService_JobStatusConfirm_PlatenOriginalSizeFixed:
			ss_tedium = 4 * sizeof(uint32_t) ;
			break ;
		case IPCE_JobService_JobStatusConfirm_ScanSendState:
			ss_tedium = 4 * sizeof(uint32_t) ;
			break ;
		case IPCE_JobService_JobStatusConfirm_LimitOverCopyKit:
			ss_tedium = 4 * sizeof(uint32_t) ;
			break ;
		case IPCE_JobService_JobStatusConfirm_Warning:
			ss_tedium = 4 * sizeof(uint32_t) ;
			break ;
		case IPCE_JobService_JobStatusConfirm_RedialJob:
			ss_tedium = 4 * sizeof(uint32_t) ;
			break ;
// CQ367177 対応 2012/08/18 上位に返すメッセージ構造体に、WarningPaperSizeサイズ値の項目を追加する <-.
		case IPCE_JobService_JobStatusConfirm_WarningPaperSize:
// CQ367177 対応 2012/08/18 上位に返すメッセージ構造体に、WarningPaperSizeサイズ値の項目を追加する <-.
		default:
			ss_tedium = -1 ;
			break ;
		}
		if ( ss_tedium != -1 ) {
			// MAX は JSVS_JobServiceStatusSubAnsMsg だが余分なサイズ分を引く.
			ss_ret = sizeof(JSVS_JobServiceStatusSubAnsMsg) - ss_tedium ;
		} else {
			// 照合する冗長サイズが見つからない場合、とりあえずフルサイズとっておく.
			ss_ret = sizeof(JSVS_JobServiceStatusSubAnsMsg) ;
		}
	}
	return ss_ret ;
}

//--------------------------------------------------------------------------------------------------.
//	JSVC_ResponseForProgramJobクラス定義.
//--------------------------------------------------------------------------------------------------.
//コンストラクタ.
JSVC_ResponseForProgramJob::JSVC_ResponseForProgramJob(uint32_t ul_MsgTypeID)
:	JSVC_Response(ul_MsgTypeID),m_ulDocID(0)
{
}

// デストラクタ.
JSVC_ResponseForProgramJob::~JSVC_ResponseForProgramJob()
{
}

// 自身の型を示す.
JSVE_Request_Class_Type JSVC_ResponseForProgramJob::WhoAmI()
{
	return JSVE_Request_Class_Type_Res_Prg_Job;
}

// リクエスト受付時のデータ保存処理.
Bool JSVC_ResponseForProgramJob::setData(const char* pRcvData, short ss_size)
{
	Bool ret = True;

	switch( getMsgTypeId() ){
		case JSVE_JobManager_MsgID_GetInputDataIDAccept:
			{
				JSVS_GetInputDataIDAnsMsg* pMsg = (JSVS_GetInputDataIDAnsMsg*)pRcvData;
				m_ulDocID = pMsg->ul_DocID;
			}
			break;
		case JSVE_JobManager_MsgID_DelInputDataAccept:
			// データはない.
			break;
		case JSVE_JobManager_MsgID_DelInputDataEnd:
			{
				JSVS_SubScenarioInfoNoticeMsg* pMsg = (JSVS_SubScenarioInfoNoticeMsg*)pRcvData;
				setResult(pMsg->e_JobServiceResult);
				setResultDetail(pMsg->e_JobServiceResultDetail);
			}
			break;
		default:
			MyTraceSD(JSVD_DBG_ERROR, "JSVC_ResponseForProgramJob::setData ERR ID=", (int)getMsgTypeId() );	// warning対応 2013/09/18 ooba.
			ret = False;
			break;
	}

	return ret;
}

// 送信時に確保すべきデータサイズ.
short JSVC_ResponseForProgramJob::getSndDataSize()
{
	short ss_ret = -1;

	switch( getMsgTypeId() ){
		case JSVE_JobManager_MsgID_GetInputDataIDAccept:
			ss_ret = sizeof(JSVS_GetInputDataIDAnsMsg);
			break;
		case JSVE_JobManager_MsgID_DelInputDataAccept:
			ss_ret = sizeof(JSVS_DelInputDataAnsMsg);
			break;
		case JSVE_JobManager_MsgID_DelInputDataEnd:
			ss_ret = sizeof(JSVS_SubScenarioInfoNoticeMsg);
			break;
		default:
			MyTraceSD(JSVD_DBG_ERROR, "JSVC_ResponseForProgramJob::getSndDataSize ERR ID=", (int)getMsgTypeId() );	// warning対応 2013/09/18 ooba.
			break;
	}

	return ss_ret;
}

// 送信時にデータを設定する.
short JSVC_ResponseForProgramJob::MakeDataStream(JSVC_RequestBase* pc_Req, char* pSndData)
{
	short ss_ret = -1 ;

	// データ設定.
	if ( pSndData != NULL ) {
		switch( getMsgTypeId() ){
			case JSVE_JobManager_MsgID_GetInputDataIDAccept:
				{
					JSVS_GetInputDataIDAnsMsg* pSnd = (JSVS_GetInputDataIDAnsMsg*)pSndData;
					pSnd->ul_SubScenarioID = ((JSVC_Request*)pc_Req)->getSubScenarioID();
					pSnd->e_JobServiceResult = getResult();
					pSnd->ul_DocID = m_ulDocID;
				}
				break;
			case JSVE_JobManager_MsgID_DelInputDataAccept:
				{
					JSVS_DelInputDataAnsMsg* pSnd = (JSVS_DelInputDataAnsMsg*)pSndData;
					pSnd->ul_SubScenarioID = ((JSVC_Request*)pc_Req)->getSubScenarioID();
					pSnd->e_JobServiceResult = getResult();
				}
				break;
			case JSVE_JobManager_MsgID_DelInputDataEnd:
				{
					JSVS_SubScenarioInfoNoticeMsg* pSnd = (JSVS_SubScenarioInfoNoticeMsg*)pSndData;
					pSnd->e_JobService_ScenarioSts = IPCE_JobService_ScenarioSts_DelInputDataEnd;	// 固定値.
					pSnd->ul_SubScenarioID = ((JSVC_Request*)pc_Req)->getSubScenarioID();
					pSnd->e_JobServiceResult = getResult();
					pSnd->e_JobServiceResultDetail = getResultDetail();
				}
				break;
			default:
				MyTraceSD(JSVD_DBG_ERROR, "JSVC_ResponseForProgramJob::MakeDataStream ERR ID=", (int)getMsgTypeId() );	// warning対応 2013/09/18 ooba.
				break;
		}
		ss_ret = getSndDataSize();
	}

	return ss_ret;
}

//関数------------------------------------------------------------------------------------------------------------------.
//概要　　：ジョブインスタンス取得.
//戻り値　：APIC_Job*		ジョブインスタンス.
//引数　　：.
//備考　　：.
//作成者　：Sky_宮川.
//レビュー：レビュー実施日と名前を記述.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
APIC_Job* JSVC_ParentRequest::getJobInstance()
{
	return getJobInstance(-1);
}
APIC_Job* JSVC_ParentRequest::getJobInstance( short ss_index )
{
	APIC_Job* pc_Job = NULL;

	if ( ss_index < 0 ){
		// 最後に追加したジョブインスタンスを返す.
		getJobInstance( &pc_Job );

	} else if( ss_index < JSVD_JOBP_MAX ){
		// 指定されたジョブインスタンスを返す.
		pc_Job = pc_JobPtr[ss_index];
		
	} else {
		// NULLを返す.
	}

	return pc_Job;
}

//関数------------------------------------------------------------------------------------------------------------------.
//概要　　：課金アプリのソケット情報取得.
//戻り値　：struct sockaddr_un*		ソケット情報.
//引数　　：.
//備考　　：.
//作成者　：Sky_宮川.
//レビュー：レビュー実施日と名前を記述.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
struct sockaddr_un* JSVC_ParentRequest::getCharegeInfo()
{
	return &s_chargeApplicationSocket;
}
// 課金アプリのソケット情報の取得.
void JSVC_ParentRequest::setCharegeInfo(struct sockaddr_un* ps_ChargeAddr)
{
	memcpy( &s_chargeApplicationSocket , ps_ChargeAddr ,sizeof(s_chargeApplicationSocket) );
}

//==============================================================================.
//	関  数：getJobInstance_byBinaryFile.
//	概  要：受信アプリからのシナリオ投入の場合のみ処理を行う.
//			JobSpecのバイナリファイルからJobSpecクラスへのデシリアライズを行う.
//			APIC_Jobインスタンスを取得し親リクエストにセットする.
//	引  数：TYPC_JobSpec	: pJobSpec		: JobSpecのバイナリファイルパス.
//	戻り値：Bool			: True/False	: 正常/異常.
//	備  考：.
//==============================================================================.
Bool JSVC_RequestJobSetting::getJobInstance_byBinaryFile(JSVC_Request* pc_Req, char* p_FilePath, JSVC_JobSpec* pc_JobSpec)
{
	uint32_t					ulID = 0;
	JSVC_ParentRequest*		pc_PRequest = NULL;
	APIC_Job*				pc_Job = NULL;
	JSVC_RequestJobSetting* pc_Request = (JSVC_RequestJobSetting*)pc_Req;

	// JobSpecデータクラスを作成.
	pc_Request->p_DataJobSpec = pc_JobSpec;

	// JobSpec(編集領域)の内容をチェック.
	if (False == checkJobSpec(pc_Request->p_DataJobSpec->getJobSpec())) {
		MyTraceSS(JSVD_DBG_ERROR, "JSVC_RequestJobSetting::getJobInstance_byBinaryFile checkJobSpec ERR" , p_FilePath );
		return False;
	}

	// シーケンスIDでジョブを検索する.
	LIBC_List<APIC_Job> c_MyList;
	APIC_Job* pc_myJob = NULL;
	APIC_ActiveJobList::newInstance()->getList(TYPE_ActiveJobListAll, 1, 350, &c_MyList);
	for (pc_myJob = c_MyList.findFirst(); pc_myJob; pc_myJob = c_MyList.findNext())
	{
		if(pc_myJob->getJobSpec()->pc_Common->c_JobType.getJobType() == TYPE_JobTypePdl){
			// 親ジョブは対象外.
			continue;
		}
		if( 0 != pc_myJob->getJobSpec()->pc_Common->getSequenceId() ){
			//既に投入済のJobなのでマッチング対象から外す.
			continue;
		}
		if(( TYPD_ImageLogSendJobNon != pc_JobSpec->getJobSpec()->pc_Common->getImageLogSend())
		&& ( pc_JobSpec->getJobSpec()->pc_Common->getImageLogSend() == pc_myJob->getJobSpec()->pc_Common->getImageLogSend()) ){
			// 画像ログジョブ.
			break;
		}

		if(pc_Request->p_DataJobSpec->getJobSpec()->pc_Common->getSequenceId() == pc_myJob->getJobStatus()->getJobID())
		{
			// 受信アプリ投入ジョブ.
			break;
		}
	}
	if (NULL != pc_myJob){
		pc_Job = pc_myJob;
	}else{
		MyTraceSLD(JSVD_DBG_ERROR, "JSVC_RequestJobSetting::getJobInstance_byBinaryFile getInstance ERR" , (intptr_t)pc_Job );	// warning対応 2013/09/18 ooba.
		pc_Request->p_DataJobSpec = NULL;
		return False;
	}
	uint32_t ul_seqId = pc_Request->p_DataJobSpec->getJobSpec()->pc_Common->getSequenceId();

	pc_Request->p_DataJobSpec = NULL;

	// 親リクエストを取得し、生成したJobインスタンスを親リクエストに追加.
	ulID = pc_Request->getSubScenarioID();
	pc_PRequest = JSVC_JobManager::newInstance()->getParentRequest(ulID);
	if (NULL == pc_PRequest) {
		MyTraceSLD(JSVD_DBG_ERROR, "JSVC_RequestJobSetting::getJobInstance_byBinaryFile getParentRequest ERR" , (intptr_t)pc_PRequest );	// warning対応 2013/09/18 ooba.
		return False;
	}else{
		//次回検索時にマッチング対象から外すためにIDをセットしておく.
		pc_myJob->getJobSpec()->pc_Common->setSequenceId( ul_seqId );

		pc_PRequest->addInstance( pc_Job );

		// 受信モジュール種別を設定する.
		// 2013/05/07 復旧ジョブシーケンス見直し対応(CQ379669) modify start.
		//IPCE_JobsRcvJobType e_JobsRcvJobType = JSVC_JobManager::newInstance()->getRcvJobType( pc_Job->getJobSpec(), pc_Job->getJobStatus() );
		IPCE_JobsRcvJobType e_JobsRcvJobType = JSVC_JobManager::newInstance()->getRcvJobType( pc_Job );
		// 2013/05/07 復旧ジョブシーケンス見直し対応(CQ379669) modify end.
		TYPE_ReceptionModuleType e_Type = TYPE_ReceptionModuleType_NONE;
		switch(e_JobsRcvJobType){
			case IPCE_RcvJobType_Non: e_Type = TYPE_ReceptionModuleType_NONE; break;
			case IPCE_RcvJobType_Net: e_Type = TYPE_ReceptionModuleType_NetRx; break;
			case IPCE_RcvJobType_Fax: e_Type = TYPE_ReceptionModuleType_FaxRx; break;
			case IPCE_RcvJobType_Tcp: e_Type = TYPE_ReceptionModuleType_TcpSocket; break;
			case IPCE_RcvJobType_PSWC: e_Type = TYPE_ReceptionModuleType_PSWC; break;
			case IPCE_RcvJobType_OpenAPI: e_Type = TYPE_ReceptionModuleType_Oap; break;
			case IPCE_RcvJobType_Browser: e_Type = TYPE_ReceptionModuleType_Browser; break;
			case IPCE_RcvJobType_ImageLog: e_Type = TYPE_ReceptionModuleType_IGLSend; break;

/* CQ370925対応 2012/08/28 S.Wakayama Start */
			case IPCE_RcvJobType_Backup: e_Type = TYPE_ReceptionModuleType_Backup; break;
			case IPCE_RcvJobType_FwUpdate: e_Type = TYPE_ReceptionModuleType_FwUpdate; break;
/* CQ370925対応 2012/08/28 S.Wakayama End */

			default: break;
		}
		pc_PRequest->setReceptionModuleType(e_Type);
	}
	return True;
}

Bool JSVC_RequestJobSetting::getJobSpec_byBinaryFile(char* p_FilePath, JSVC_JobSpec* p_DataJobSpec)
{
	// JobSpecファイル読み込み.
	MyTraceSS(JSVD_DBG_INFO, "JSVC_RequestJobSetting call deSerialize" , p_FilePath );
	if (False == p_DataJobSpec->deSerialize(p_FilePath)) {
		MyTraceSS(JSVD_DBG_ERROR, "JSVC_RequestJobSetting::getJobSpec_byBinaryFile deSerialize ERR" , p_FilePath );
		return False;
	}
	return True;
}

Bool JSVC_RequestJobSetting::setOverWriteJobSpec(JSVC_Request* pc_Request)
{
	Bool b_ret = False;

//Str【 静的解析対応 : Sky田村 : 2012.09.03 : CID-439625 】.
	if( pc_Request->GetParentRequest() == NULL )
	{
		MyTraceSS( JSVD_DBG_ERROR, "JSVC_RequestJobSetting::setOverWriteJobSpec()", "pc_Request->GetParentRequest() is NULL" );
		return b_ret;
	}
//End【 静的解析対応 : Sky田村 : 2012.09.03 : CID-439625 】.
//Str【 静的解析対応 : Sky陰山 : 2012.10.29 : CID-439625 】.
	if( GetParentRequest() == NULL )
	{
		MyTraceSS( JSVD_DBG_ERROR, "JSVC_RequestJobSetting::setOverWriteJobSpec()", "GetParentRequest() is NULL" );
		return b_ret;
	}
//End【 静的解析対応 : Sky陰山 : 2012.10.29 : CID-439625 】.

	JSVC_JobSpec* pc_JobSpec = pc_Request->GetParentRequest()->getJobSpec();
	APIC_Job* pc_Job = ((JSVC_RequestJobSetting*)pc_Request)->getJobInstance();
	if ((NULL != pc_Job) &&
		(NULL != pc_JobSpec)){
		// ジョブスペックの上書き設定.
		pc_JobSpec->setJobSpec(pc_Job->getJobSpec());

		// JobSpecを削除する.
		GetParentRequest()->delJobSpec();

		b_ret = True;
	}
	return b_ret;
}
Bool JSVC_RequestJobSetting::isReceiveAPP()
{
	Bool b_ret = False;
//Str【 静的解析対応 : Sky田村 : 2012.09.03 : CID-439624 】.
	if( GetParentRequest() == NULL )
	{
		MyTraceSS( JSVD_DBG_ERROR, "JSVC_RequestJobSetting::isReceiveAPP()", "GetParentRequest() is NULL" );
		return b_ret;
	}
//End【 静的解析対応 : Sky田村 : 2012.09.03 : CID-439624 】.
	JSVC_JobSpec* pc_JobSpec = GetParentRequest()->getJobSpec();
	if ((NULL != pc_JobSpec) &&
		(0 != pc_JobSpec->getJobSpec()->pc_Common->getSequenceId())){
		b_ret = True;
	}
	return b_ret;
}

//--------------------------------------------------------------------------------------------------.
//	JSVC_RequestOneShotReciveAPPクラス定義.
//--------------------------------------------------------------------------------------------------.
//コンストラクタ.
JSVC_RequestOneShotReceiveAPP::JSVC_RequestOneShotReceiveAPP(uint32_t ul_RequestID,uint32_t ul_MsgTypeID)
:	JSVC_RequestBase(ul_RequestID,ul_MsgTypeID)
, ul_JobServiceReqID(0), ul_SubScenarioID(0), ul_SequenceId(0)
, pFile(NULL), ul_FilePathSize(0)
	,ull_UserID(0),ull_TrackID(0),e_RcvJobType(0)// 20万人対応  2013/11/27 Soma.
{
}
//デストラクタ.
JSVC_RequestOneShotReceiveAPP::~JSVC_RequestOneShotReceiveAPP()
{
	if(pFile){
		free(pFile);
		pFile = NULL;
	}
}
// 自身の型を示す.
JSVE_Request_Class_Type JSVC_RequestOneShotReceiveAPP::WhoAmI()
{
	return JSVE_Request_Class_Type_Req_RcvApp;
}
// リクエスト受付時のデータ保存処理.
Bool JSVC_RequestOneShotReceiveAPP::setData(const char* pRcvData,short ss_size)
{
#ifdef _DEBUG_JSVC_SEQ
	MyTraceSLD(JSVD_DBG_INFO, "JSVC_RequestOneShotReceiveAPP::setData pRcvData:" , (intptr_t)pRcvData );		// 回避方法をトレースに変更.// warning対応 2013/09/18 ooba.
	MyTraceSD(JSVD_DBG_INFO, "JSVC_RequestOneShotReceiveAPP::setData ss_size:" , ss_size );		// 回避方法をトレースに変更.
#endif

	MyTraceSD( JSVD_DBG_INFO, "JSVC_RequestOneShotReceiveAPP::setData MsgType:" , (int)getMsgTypeId() ); // warning対応 2013/09/18 ooba.
	switch( getMsgTypeId() ){
		case JSVE_JobManager_MsgID_RcvNotify_Add:					// 受信通知登録要求					SRVM向け.
			// パラメータなしの要求.
			break;
		case JSVE_JobManager_MsgID_RcvNotify_GetSetting:			// 受信情報設定取得					API向け.
		case JSVE_JobManager_MsgID_RcvNotify_GetStatus:				// 受信情報ステータス取得			API向け.
		case JSVE_JobManager_MsgID_RcvNotify_GetUserInfo:			// 受信情報ユーザ情報取得			API向け.
			// JSVS_ReqPara_ReceiveApp.
			{
				JSVS_ReqPara_ReceiveApp *pMsg = (JSVS_ReqPara_ReceiveApp*)pRcvData;
				// 受信データID(シーケンスID).
				ul_SequenceId = pMsg->ul_RcvDataID;
				// JobSpecファイル.
				if( pFile != NULL ){								// もし獲得済であれば開放してから.
					free(pFile);
					pFile = NULL;
				}
				if( pMsg->ul_ReqDataSize <= JSVD_XMLFILEPATH_MAX ){
					// ファイルパスサイズ.
					ul_FilePathSize = pMsg->ul_ReqDataSize;
					pFile = (char*)malloc(ul_FilePathSize+1); 
//Str【 静的解析対応 : Sky田村 : 2012.09.03 : CID-439631 】.
					if( pFile != NULL )
					{
					memset( pFile, 0x00, ul_FilePathSize+1 );
					strncpy( pFile, pMsg->sc_ReqData, pMsg->ul_ReqDataSize );
				}
					else
					{
						MyTraceSS( JSVD_DBG_ERROR, "JSVC_RequestOneShotReceiveAPP::setData", "malloc Error pFile 1" );
					}
//End【 静的解析対応 : Sky田村 : 2012.09.03 : CID-439631 】.
				}
				else{
					ul_FilePathSize = JSVD_XMLFILEPATH_MAX;							// ファイルパスサイズの確定.
					pFile = (char*)malloc(ul_FilePathSize+1); 						// メモリの取得.
//Str【 静的解析対応 : Sky田村 : 2012.09.03 : CID-439631 】.
					if( pFile != NULL )
					{
					memset( pFile, 0x00, ul_FilePathSize+1 );
					strncpy( pFile, pMsg->sc_ReqData, JSVD_XMLFILEPATH_MAX );		// ファイルパスを保存.
				}
					else
					{
						MyTraceSS( JSVD_DBG_ERROR, "JSVC_RequestOneShotReceiveAPP::setData", "malloc Error pFile 2" );
					}
//End【 静的解析対応 : Sky田村 : 2012.09.03 : CID-439631 】.
				}
//Str【 静的解析対応 : Sky田村 : 2012.09.03 : CID-439631 】.
				if( pFile != NULL )
				{
				MyTraceSS(JSVD_DBG_INFO, "JSVC_RequestOneShotReceiveAPP::setData FilePath1" ,pFile );
			}
//End【 静的解析対応 : Sky田村 : 2012.09.03 : CID-439631 】.
			}
			break;
		case JSVE_JobManager_MsgID_SubScenario_GetSetting:			// サブシナリオ設定取得				SRVM向け.
		case JSVE_JobManager_MsgID_SubScenario_GetStatus:			// サブシナリオステータス情報取得	SRVM向け.
			// JSVS_ReqPara_ReceiveApp_GetSubScenarioStatus.
			{
				JSVS_ReqPara_ReceiveApp_GetSubScenarioStatus *pMsg = (JSVS_ReqPara_ReceiveApp_GetSubScenarioStatus*)pRcvData;
				// サブシナリオID.
				ul_SubScenarioID = pMsg->ul_SubScenarioID;
				// JobSpecファイル.
				if( pFile != NULL ){								// もし獲得済であれば開放してから.
					free(pFile);
					pFile = NULL;
				}
				if( pMsg->ul_ReqDataSize <= JSVD_XMLFILEPATH_MAX ){
					// ファイルパスサイズ.
					ul_FilePathSize = pMsg->ul_ReqDataSize;
					pFile = (char*)malloc(ul_FilePathSize+1); 
//Str【 静的解析対応 : Sky田村 : 2012.09.03 : CID-439631 】.
					if( pFile != NULL )
					{
					memset( pFile, 0x00, ul_FilePathSize+1 );
					strncpy( pFile, pMsg->sc_ReqData, pMsg->ul_ReqDataSize );
				}
					else
					{
						MyTraceSS( JSVD_DBG_ERROR, "JSVC_RequestOneShotReceiveAPP::setData", "malloc Error pFile 3" );
					}
//End【 静的解析対応 : Sky田村 : 2012.09.03 : CID-439631 】.
				}
				else{
					ul_FilePathSize = JSVD_XMLFILEPATH_MAX;							// ファイルパスサイズの確定.
					pFile = (char*)malloc(ul_FilePathSize+1); 						// メモリの取得.
//Str【 静的解析対応 : Sky田村 : 2012.09.03 : CID-439631 】.
					if( pFile != NULL )
					{
					memset( pFile, 0x00, ul_FilePathSize+1 );
					strncpy( pFile, pMsg->sc_ReqData, JSVD_XMLFILEPATH_MAX );		// ファイルパスを保存.
				}
					else
					{
						MyTraceSS( JSVD_DBG_ERROR, "JSVC_RequestOneShotReceiveAPP::setData", "malloc Error pFile 4" );
					}
//End【 静的解析対応 : Sky田村 : 2012.09.03 : CID-439631 】.
				}
//Str【 静的解析対応 : Sky田村 : 2012.09.03 : CID-439631 】.
				if( pFile != NULL )
				{
				MyTraceSS(JSVD_DBG_INFO, "JSVC_RequestOneShotReceiveAPP::setData FilePath2" ,pFile );
				}
//End【 静的解析対応 : Sky田村 : 2012.09.03 : CID-439631 】.
			}
			break;
		case JSVE_JobManager_MsgID_RcvNotify_Cancel:				// 受信キャンセル要求.
			// JSVS_ReqPara_ReceiveAppCancel.
			{
				JSVS_ReqPara_ReceiveAppCancel *pMsg = (JSVS_ReqPara_ReceiveAppCancel*)pRcvData;
				// シーケンスID(受信データID).
				ul_SequenceId = pMsg->ul_SequenceId;
			}
			break;
		case JSVE_JobManager_MsgID_RcvNotify_GetRecieveDataIDList:			// 受信情報設定(受信データIDリスト)要求.
			// JSVS_ReqPara_ReceiveAppRcvDataIDList.
			{
				JSVS_ReqPara_ReceiveAppRcvDataIDList *pMsg = (JSVS_ReqPara_ReceiveAppRcvDataIDList*)pRcvData;
// Start 20万人対応  2013/11/27 Soma.
				int32_t sl_UserID = (int32_t)pMsg->ull_UserID;
				ull_UserID = (uint64_t)sl_UserID;
				int32_t sl_TrackID = (int32_t)pMsg->ull_TrackID;
				ull_TrackID = (uint64_t)sl_TrackID;
// End 20万人対応  2013/11/27 Soma.
				e_RcvJobType = pMsg->e_RcvJobType;
				MyTraceSLD(JSVD_DBG_INFO, "setData UserID " , (int64_t)ull_UserID );// 20万人対応  2013/11/27 Soma.
				MyTraceSLD(JSVD_DBG_INFO, "setData TrackID " , (int64_t)ull_TrackID );// 20万人対応  2013/11/27 Soma.
				MyTraceSD(JSVD_DBG_INFO, "setData e_RcvJobType " , (int)e_RcvJobType );
			}
			break;
		default:
			MyTraceSS( JSVD_DBG_ERROR, "JSVC_RequestOneShotReceiveAPP::setData", "unknown MsgType" );
			break;
	}

	return True;
}
// JobManagerから操作するI/Fを定義.
JSVE_Request_Result JSVC_RequestOneShotReceiveAPP::startAction(JSVS_NotifyInfo* pNotify)
{
#ifdef _DEBUG_JSVC_SEQ
	MyTraceSLD(JSVD_DBG_INFO, "JSVC_RequestOneShotReceiveAPP::startAction pNotify:" , (intptr_t)pNotify );	// 回避方法をトレースに変更.// warning対応 2013/09/18 ooba.
#endif

	// 関数内処理結果.
	JSVE_Request_Result e_Request_Result = JSVE_Request_Result_Err;

	// 内部変数.
	uint32_t _ul_MsgTypeID = 0;
	LIBC_List<APIC_Job> c_MyList;
	APIC_Job* pc_myJob = NULL;
	Bool b_SearchRet = False;
	JSVC_JobSpec c_JobSpec;

	if( pNotify != NULL ){
		if(( pNotify->e_JobManager_MsgTypeID == JSVE_JobManager_MsgID_RcvNotify_AddAccept )
		|| ( pNotify->e_JobManager_MsgTypeID == JSVE_JobManager_MsgID_RcvNotify_GetSettingAccept )
		|| ( pNotify->e_JobManager_MsgTypeID == JSVE_JobManager_MsgID_RcvNotify_GetStatusAccept )
		|| ( pNotify->e_JobManager_MsgTypeID == JSVE_JobManager_MsgID_RcvNotify_GetUserInfoAccept )
		|| ( pNotify->e_JobManager_MsgTypeID == JSVE_JobManager_MsgID_SubScenario_GetSettingAccept )
		|| ( pNotify->e_JobManager_MsgTypeID == JSVE_JobManager_MsgID_SubScenario_GetStatusAccept )
		|| ( pNotify->e_JobManager_MsgTypeID == JSVE_JobManager_MsgID_RcvNotify_CancelAccept )
		){
			// CQ377138対応 2013.01.23 Start.
			// サブシナリオが降りてきた場合、対応するジョブインスタンスがないため
			// 破棄処理中のサブシナリオであった場合、結果をNGで返すように変更する.
			if ( pNotify->e_JobManager_MsgTypeID == JSVE_JobManager_MsgID_RcvNotify_GetSettingAccept ){
				Bool b_ForceChange = False;
				JSVC_ParentRequest* pc_PRequest = NULL;
				APIC_Job* pc_Job = NULL;
				APIC_Job* pc_myJob = NULL;
				Bool b_ExistJob = False;
				LIBC_List<APIC_Job> c_MyList;
				uint32_t ul_SubScenarioID = -1;
				TYPE_OutputStatus e_OutputStatus = 0;
				Bool b_InvalidJob = False;

				JSVC_RequestOneShotReceiveAPP* 	pc_ReqReceiveAPP = (JSVC_RequestOneShotReceiveAPP*)pNotify->pReq;
				JSVC_ResponseReceiveApp* 		pc_ResReceiveAPP = (JSVC_ResponseReceiveApp*)pNotify->pRes;
				ul_SubScenarioID = pc_ReqReceiveAPP->getSequenceId();
				pc_PRequest = JSVC_JobManager::newInstance()->getParentRequest(ul_SubScenarioID);
				if ( NULL != pc_PRequest ) {
					pc_Job = pc_PRequest->getJobInstance();
					if (NULL != pc_Job){
						pc_Job->lock();
						APIC_ActiveJobList::newInstance()->getList(TYPE_ActiveJobListAll, 1, 350, &c_MyList);
						for (pc_myJob = c_MyList.findFirst(); pc_myJob; pc_myJob = c_MyList.findNext()){
							if( pc_myJob == pc_Job ){
								b_ExistJob = True;
							}
						}
						if ( True == b_ExistJob ){
							e_OutputStatus = pc_Job->getJobStatus()->getOutputStatus();
//							if ( TYPE_OutputStatusDeleting == e_OutputStatus ){
								b_ForceChange = True;
//							}
							b_InvalidJob = pc_Job->getInvalidJob();
							if (True == b_InvalidJob){
								b_ForceChange = True;
							}
						}
						pc_Job->unlock();
					}

					if (True == b_ForceChange){
						pc_ResReceiveAPP->setResult(JSVE_Request_Result_Err);
						pc_ResReceiveAPP->setResultDetail(JSVE_JobServiceResultDetail_IdNotMatch);
					}
				}
			}
			// CQ377138対応 2013.01.23 End.
			#ifdef _DEBUG_JSVC_SEQ
				MyTraceSD(JSVD_DBG_INFO, "JSVC_RequestOneShotReceiveAPP::startAction send Accept :" , (int)pNotify->e_JobManager_MsgTypeID );	// 回避方法をトレースに変更.// warning対応 2013/09/18 ooba.
			#endif
			JSVS_SendRecv_MsgParam s_SendRecv_MsgParam ;
			s_SendRecv_MsgParam.ul_MsgTypeID = pNotify->e_JobManager_MsgTypeID;
			s_SendRecv_MsgParam.pReq = (JSVC_RequestBase*)pNotify->pReq;
			s_SendRecv_MsgParam.pRes = (JSVC_ResponseBase*)pNotify->pRes;
			JSVC_JobSendRecv::newInstance()->sendMessageToAPF( &s_SendRecv_MsgParam );
			return JSVE_Request_Result_Complete;
		}
		else{
			//Accept以外の処理は後続で実施.
		}
	}

//Str【 静的解析対応 : Sky田村 : 2012.08.30 : CID-436584 】.
	if( pNotify == NULL )
	{
		#ifdef _DEBUG_JSVC_SEQ
			MyTraceSLD(JSVD_DBG_ERROR, "JSVC_RequestOneShotReceiveAPP::startAction pNotify is NULL:" , (intptr_t)pNotify );	// warning対応 2013/09/18 ooba.
		#endif
		return JSVE_Request_Result_Err;
	}
//End【 静的解析対応 : Sky田村 : 2012.08.30 : CID-436584 】.

	// リクエストクラスのキャスト.
	JSVC_RequestOneShotReceiveAPP* 	pc_ReqReceiveAPP = (JSVC_RequestOneShotReceiveAPP*)pNotify->pReq;

	// レスポンス用構造体.
	JSVS_ResponseParamReceiveApp st_ResReceiveApp;
	memset( &st_ResReceiveApp, 0x00, sizeof(st_ResReceiveApp) );

	// メッセージ送信構造体.
	JSVS_SendRecv_MsgParam s_MsgParam;
	memset( &s_MsgParam, 0x00, sizeof(s_MsgParam) );

	// リクエストのNULLチェック.
	// startActionのCALL元でもチェックがあるため、ここは通らないが念のため実装する.
	if (NULL == pc_ReqReceiveAPP){
		// ありえないCALL状態となっているため、あえてエラーとしレスポンスは正しく返さないようにしておく.
		#ifdef _DEBUG_JSVC_SEQ
			MyTraceSLD(JSVD_DBG_ERROR, "JSVC_RequestOneShotReceiveAPP::startAction pNotify->pReq:" , (intptr_t)pc_ReqReceiveAPP );	// 回避方法をトレースに変更.	// warning対応 2013/09/18 ooba.
		#endif
		return JSVE_Request_Result_Err;
	}

	// 要求メッセージ毎の戻りメッセージを決定する.
	if (JSVE_JobManager_MsgID_RcvNotify_Add ==getMsgTypeId()){
		_ul_MsgTypeID = JSVE_JobManager_MsgID_RcvNotify_AddAccept;
	}else if (JSVE_JobManager_MsgID_RcvNotify_GetSetting ==getMsgTypeId()){
		_ul_MsgTypeID = JSVE_JobManager_MsgID_RcvNotify_GetSettingAccept;
	}else if (JSVE_JobManager_MsgID_RcvNotify_GetStatus ==getMsgTypeId()){
		_ul_MsgTypeID = JSVE_JobManager_MsgID_RcvNotify_GetStatusAccept;
	}else if (JSVE_JobManager_MsgID_RcvNotify_GetUserInfo ==getMsgTypeId()){
		_ul_MsgTypeID = JSVE_JobManager_MsgID_RcvNotify_GetUserInfoAccept;
	}else if (JSVE_JobManager_MsgID_SubScenario_GetSetting ==getMsgTypeId()){
		_ul_MsgTypeID = JSVE_JobManager_MsgID_SubScenario_GetSettingAccept;
	}else if (JSVE_JobManager_MsgID_SubScenario_GetStatus ==getMsgTypeId()){
		_ul_MsgTypeID = JSVE_JobManager_MsgID_SubScenario_GetStatusAccept;
	}else if (JSVE_JobManager_MsgID_RcvNotify_Cancel ==getMsgTypeId()){
		_ul_MsgTypeID = JSVE_JobManager_MsgID_RcvNotify_CancelAccept;
	}else if (JSVE_JobManager_MsgID_RcvNotify_GetRecieveDataIDList ==getMsgTypeId()){
		_ul_MsgTypeID = JSVE_JobManager_MsgID_RcvNotify_GetRecieveDataIDListAccept;
	}else{
		// 処理範疇でないメッセージのため、あえてエラーとしレスポンスは正しく返さないようにしておく.
		#ifdef _DEBUG_JSVC_SEQ
			MyTraceSLD(JSVD_DBG_ERROR, "JSVC_RequestOneShotReceiveAPP::startAction NotCreate JSVC_ResponseReceiveApp" , (intptr_t)pc_ReqReceiveAPP );	// 回避方法をトレースに変更.	// warning対応 2013/09/18 ooba.
		#endif
		return JSVE_Request_Result_Err;
	}

	// 返信用データクラスの生成.
	JSVC_ResponseReceiveApp* pc_ResReceiveApp = NULL;
	pc_ResReceiveApp = new JSVC_ResponseReceiveApp(_ul_MsgTypeID);
	#if DEBUGLOG__REQUEST_LIFE
		MyTraceSLD(JSVD_DBG_INFO, "JSVC_RequestOneShotReceiveAPP::startAction new JSVC_ResponseReceiveApp" , (intptr_t)pc_ResReceiveApp );
	#endif
	//生成ログ.

	// シーケンスIDまたはサブシナリオIDをキーにJobポインタの検索を行う.
	// 後続のメッセージ別処理で判断が必要な共通処理のため、先に実施しておく.
	// 後続処理では、b_SearchRetの結果で判断する.
	switch( getMsgTypeId() ){
		case JSVE_JobManager_MsgID_RcvNotify_GetSetting:			// 受信情報設定取得					API向け.
		case JSVE_JobManager_MsgID_RcvNotify_GetStatus:				// 受信情報ステータス取得			API向け.
		case JSVE_JobManager_MsgID_RcvNotify_GetUserInfo:			// 受信情報ユーザ情報取得			API向け.
		case JSVE_JobManager_MsgID_RcvNotify_Cancel:				// 受信キャンセル要求				API/SRVM向け.
			// シーケンスIDをキーにJobポインタの検索を行う.
			MyTraceSD(JSVD_DBG_INFO, "JSVC_RequestOneShotReceiveAPP::startAction Search SeqID" , (int)pc_ReqReceiveAPP->getSequenceId() );
			APIC_ActiveJobList::newInstance()->getList(TYPE_ActiveJobListAll, 1, 350, &c_MyList);
			for (pc_myJob = c_MyList.findFirst(); pc_myJob; pc_myJob = c_MyList.findNext()){
				if(pc_myJob->getJobSpec()->pc_Common->c_JobType.getJobType() == TYPE_JobTypePdl){
					// 親ジョブは対象外.
					continue;
				}
				if(pc_ReqReceiveAPP->getSequenceId() == pc_myJob->getJobStatus()->getJobID()){
					// Jobポインタが見つかった場合、内部フラグを設定し、Jobインスタンスを設定する.
					b_SearchRet = True;
					break;
				}
			}
			break;
		case JSVE_JobManager_MsgID_SubScenario_GetSetting:			// サブシナリオ設定取得				SRVM向け.
		case JSVE_JobManager_MsgID_SubScenario_GetStatus:			// サブシナリオステータス情報取得	SRVM向け.
			APIC_ActiveJobList::newInstance()->getList(TYPE_ActiveJobListAll, 1, 350, &c_MyList);
			MyTraceSD(JSVD_DBG_INFO, "JSVC_RequestOneShotReceiveAPP::startAction Search SbuScenID" , (int)pc_ReqReceiveAPP->getSubScenarioID() );
			for (pc_myJob = c_MyList.findFirst(); pc_myJob; pc_myJob = c_MyList.findNext()){
				if(pc_myJob->getJobSpec()->pc_Common->c_JobType.getJobType() == TYPE_JobTypePdl){
					// 親ジョブは対象外.
					continue;
				}
				if(pc_ReqReceiveAPP->getSubScenarioID() == pc_myJob->getJobStatus()->getSubScenarioID()){
					// Jobポインタが見つかった場合、内部フラグを設定し、Jobインスタンスを設定する.
					b_SearchRet = True;
					break;
				}
			}
			break;
		default:
			break;
	}

	// 通知情報の編集.
	// 以後の処理でメッセージに対応する処理を行うが、結果のみを上書き設定する.
	st_ResReceiveApp.ul_SequenceId 			= pc_ReqReceiveAPP->getSequenceId();
	st_ResReceiveApp.ul_Result 				= IPCE_JobServiceResult_NG;
	st_ResReceiveApp.ul_ResultDetail 		= JSVE_JobServiceResultDetail_Non;
	st_ResReceiveApp.e_RcvNotifyKind 		= 0;
	st_ResReceiveApp.ul_SubScenarioID 		= pc_ReqReceiveAPP->getSubScenarioID();
	st_ResReceiveApp.pJob 					= pc_myJob;									// NULLとなる場合があるため、メッセージ受信時のデータアクセス時は注意.
																						// 見る必要なないはず.

	// メッセージに対応したそれぞれの処理を行う.
	switch( getMsgTypeId() ){
		case JSVE_JobManager_MsgID_RcvNotify_Add:					// 受信通知登録要求					SRVM向け.
			// パラメータなしの要求.
			{
				JSVS_SendRecv_MsgParam s_SendRecv_MsgParam ;
				s_SendRecv_MsgParam.ul_MsgTypeID = pc_ResReceiveApp->getMsgTypeId();
				s_SendRecv_MsgParam.pReq = (JSVC_RequestBase*)this;
				s_SendRecv_MsgParam.pRes = (JSVC_ResponseBase*)pc_ResReceiveApp;
				JSVC_JobSendRecv::newInstance()->sendMessageToAPF( &s_SendRecv_MsgParam );
			}
			#if DEBUGLOG__REQUEST_LIFE
				//破棄ログ.
				MyTraceSLD(JSVD_DBG_INFO, "JSVC_RequestOneShotReceiveAPP::startAction delete res " , (intptr_t)pc_ResReceiveApp );
			#endif
			delete pc_ResReceiveApp;
			pc_ResReceiveApp = NULL;
			// リクエストは管理リストに残したままとする.
			e_Request_Result = JSVE_Request_Result_Ok;
			st_ResReceiveApp.ul_Result = IPCE_JobServiceResult_OK;

			MyTraceSLD(JSVD_DBG_INFO,  "JSVC_RequestOneShotReceiveAPP::startAction End 2" , (intptr_t)pc_ReqReceiveAPP );	// warning対応 2013/09/18 ooba.
			return e_Request_Result;
		case JSVE_JobManager_MsgID_RcvNotify_GetSetting:			// 受信情報設定取得					API向け.
		case JSVE_JobManager_MsgID_SubScenario_GetSetting:			// サブシナリオ設定取得				SRVM向け.
			// JSVS_ReqPara_ReceiveApp.
			// 該当Jobポインタが見つかった場合.
			if (True == b_SearchRet){
				if (NULL == pc_myJob){
					e_Request_Result = JSVE_Request_Result_Err;
					MyTraceSLD(JSVD_DBG_ERROR, "JSVC_RequestOneShotReceiveAPP::startAction pc_Job NULL" , (intptr_t)pc_ReqReceiveAPP );	// warning対応 2013/09/18 ooba.
				}else{
					// 受信情報設定取得の場合,課金種別を設定する.
					if(JSVE_JobManager_MsgID_RcvNotify_GetSetting == getMsgTypeId()){
						TYPE_KeyMode e_check = APIC_KeyDevice::getKeyMode() ;
						// 364483対応 Sky陰山 2012.05.18 Start.
						// キーカウンター設定時は、装着・非装着を判断する.
						// Sky(山村) 2012/07/12 CQ.367657 ↓.
						// if( TYPE_KeyModeKeyCounter == e_check ).
						if ( TYPE_KeyModeKeyCounter == e_check || 
							TYPE_KeyModeVendor == e_check )
						// Sky(山村) 2012/07/12 CQ.367657 ↑.
						{
							if( False == APIC_KeyDevice::isKeyCounterCopyEnable() )
							{
								// キーカウンター非装着時は無しを設定する.
								e_check = TYPE_KeyModeNone;
							}
						}
						// 364483対応 Sky陰山 2012.05.18 end.
						
						if(NULL != pc_myJob->getKeyDevice()
						&& NULL != pc_myJob->getJobSpec()
						&& NULL != pc_myJob->getJobSpec()->pc_Common){
							if( TYPE_KeyModeKeyCounter == e_check
							 || TYPE_KeyModeVendor == e_check
							 || TYPE_KeyModeAdminMachineKM == e_check ) {
								pc_myJob->getJobSpec()->pc_Common->setChargeKindID(e_check);
							}
							else if( pc_myJob->getKeyDevice()->isFeeCollectionKeyDevice()){
								pc_myJob->getJobSpec()->pc_Common->setChargeKindID(TYPE_KeyModeVendorKM);
							}
						}
					}
					// シリアライズする.
					//c_JobSpec.deSerialize_JobSpec(pc_myJob->getJobSpec());
					//c_JobSpec.Serialize(pc_ReqReceiveAPP->getFilePath());

					pc_ResReceiveApp->ul_Param1 = pc_myJob->getJobStatus()->getScenarioID();
					pc_ResReceiveApp->ul_Param2 = pc_myJob->getJobResourceID();

					// リクエスト処理の完了を設定.
					e_Request_Result = JSVE_Request_Result_Ok;
					st_ResReceiveApp.ul_Result 				= IPCE_JobServiceResult_OK;
				}
			}else{
				// 該当Jobポインタが見つからない場合.
				st_ResReceiveApp.ul_ResultDetail = JSVE_JobServiceResultDetail_IdNotMatch;
				e_Request_Result = JSVE_Request_Result_Err;
				MyTraceSLD(JSVD_DBG_INFO,  "JSVC_RequestOneShotReceiveAPP::startAction SearchResultNo" , (intptr_t)pc_ReqReceiveAPP );	// warning対応 2013/09/18 ooba.
				// 2012/11/02 CQ374171対応 add start.
				// 受信系ジョブのスタートがかかってから受信情報取得要求が来るまでの間にJobが破棄された場合.
				// AcceptMsgをそのまま行うと、リクエストポインタが指す領域をdeleteした後にアクセスしてしまうため.
				// このタイミングで上位へ応答を返すようにする.
				JSVS_SendRecv_MsgParam s_SendRecv_MsgParam ;
				switch (getMsgTypeId()) {
					case JSVE_JobManager_MsgID_RcvNotify_GetSetting:
						s_SendRecv_MsgParam.ul_MsgTypeID = JSVE_JobManager_MsgID_RcvNotify_GetSettingAccept;
						break;
					case JSVE_JobManager_MsgID_SubScenario_GetSetting:
						s_SendRecv_MsgParam.ul_MsgTypeID = JSVE_JobManager_MsgID_SubScenario_GetSettingAccept;
						break;
					default:
						break;
				}
				s_SendRecv_MsgParam.pReq = (JSVC_RequestBase*)this;
				// レスポンスクラスへのメンバー設定.
				pc_ResReceiveApp->setData( (const char*)&st_ResReceiveApp, sizeof(st_ResReceiveApp) );
				s_SendRecv_MsgParam.pRes = (JSVC_ResponseBase*)pc_ResReceiveApp;
				JSVC_JobSendRecv::newInstance()->sendMessageToAPF( &s_SendRecv_MsgParam );
				return e_Request_Result;
				// 2012/11/02 CQ374171対応 add end.
			}
			break;
		case JSVE_JobManager_MsgID_RcvNotify_GetStatus:				// 受信情報ステータス取得			API向け.
			//Non-break.
		case JSVE_JobManager_MsgID_SubScenario_GetStatus:			// サブシナリオステータス情報取得	SRVM向け.
			// JSVS_ReqPara_ReceiveApp.
			if (True == b_SearchRet){
				if (NULL == pc_myJob){
				e_Request_Result = JSVE_Request_Result_Err;
					MyTraceSLD(JSVD_DBG_ERROR, "JSVC_RequestOneShotReceiveAPP::startAction pc_Job NULL 2" , (intptr_t)pc_ReqReceiveAPP );	// warning対応 2013/09/18 ooba.
				}else{
					// JobStatusを取得し、シリアライズする.
					TYPC_JobStatus* pJobStatus = pc_myJob->getJobStatus();
					JSVC_JobStatus c_js;
					c_js.Serialize(pc_ReqReceiveAPP->getFilePath(),pJobStatus);	
					// 上記のSerializeについては削除し、シナリオIDとリソースIDを通知するように変更する必要がある.

					// リクエスト処理の完了を設定.
					e_Request_Result = JSVE_Request_Result_Ok;
					st_ResReceiveApp.ul_Result = IPCE_JobServiceResult_OK;
			}
			} else {
				st_ResReceiveApp.ul_ResultDetail = JSVE_JobServiceResultDetail_IdNotMatch;
			}
			break;
		case JSVE_JobManager_MsgID_RcvNotify_GetUserInfo:			// 受信情報ユーザ情報取得			API向け.
			// JSVS_ReqPara_ReceiveApp.
			// 該当Jobポインタが見つかった場合.
			if (True == b_SearchRet){
				if (NULL == pc_myJob){
					e_Request_Result = JSVE_Request_Result_Err;
					MyTraceSLD(JSVD_DBG_ERROR, "JSVC_RequestOneShotReceiveAPP::startAction pc_Job NULL" , (intptr_t)pc_ReqReceiveAPP );	// warning対応 2013/09/18 ooba
				}else{
					// シリアライズする.
					APIC_User* pc_User = NULL;
					APIC_Track* pc_Track =  NULL;
					// IDが有効値の場合はJobSpecのIDを使用し、範囲外の場合はNULLとし、ユーザー情報を取得しない.
					if (pc_myJob->getJobSpec()->pc_Common->c_UserAcount.getUserNo() > 0){
						pc_User = APIC_User::newInstance(pc_myJob->getJobSpec()->pc_Common->c_UserAcount.getUserNo());
					}
					// IDが有効値の場合はJobSpecのIDを使用し、範囲外の場合はNULLとし、部門情報を取得しない.
					if (pc_myJob->getJobSpec()->pc_Common->c_UserAcount.getTrackNo() > 0){
						pc_Track = APIC_Track::newInstance(pc_myJob->getJobSpec()->pc_Common->c_UserAcount.getTrackNo());
					}
					APIC_UserAuthentication* pc_UserAuthentication = APIC_UserAuthentication::newInstance();
					APIC_TrackManager* pc_TrackManager = APIC_TrackManager::newInstance();

					// 受信情報ユーザ情報の編集処理.
					JSVS_RcvNotify_UserInfo s_RcvNotify_UserInfo;
					memset( &s_RcvNotify_UserInfo, 0x00, sizeof(s_RcvNotify_UserInfo) );

					if (NULL != pc_User){
						// Start 20万人対応  2013/11/27 Soma.
						s_RcvNotify_UserInfo.ull_UserID					= JSVC_JobManager::newInstance()->getExtensionUserID(pc_User->getUserNo());
						// End 20万人対応  2013/11/27 Soma.
						pc_User->getUserNameUTF8(s_RcvNotify_UserInfo.sc_UserName);
						Bool b_Server = False;
						if (TYPE_UserAuthenticationServer == pc_UserAuthentication->getUserAuthentication()){
							b_Server = True;
						}
						pc_User->getUserPassword(s_RcvNotify_UserInfo.sc_UserPassword, b_Server);

						char sc_ServerName[APID_MaxLengthOfServerName + 1];
						memset(sc_ServerName, 0, sizeof(sc_ServerName));
						pc_User->getLoginServerName( sc_ServerName );
						// サーバ名より、サーバインデックスを取得する。
// 2016/4/15 認証拡張FUM RQ-523 [St.]
						APIC_AuthenticationSeverSetting* pc_ServerSetting = APIC_AuthenticationSeverSetting::newInstance();
						if( NULL != pc_ServerSetting ){
							if( False == pc_ServerSetting->checkServerGroup() ){
								s_RcvNotify_UserInfo.sc_ServerIndex = pc_ServerSetting->getServerIndexFromName( sc_ServerName );
							} else {
								s_RcvNotify_UserInfo.sc_ServerIndex = static_cast<APID_AuthenticationServerIndex>(pc_ServerSetting->getServerGroupIndexFromName( sc_ServerName ));
							}
						}
// 2016/4/15 認証拡張FUM RQ-523 [Ed.]
					}
					if (NULL != pc_UserAuthentication){
						s_RcvNotify_UserInfo.sc_UserAuthentication		= pc_UserAuthentication->getUserAuthentication();
					}
					if (NULL != pc_Track){
						// Start 20万人対応  2013/11/27 Soma.
						s_RcvNotify_UserInfo.ull_TrackID					= JSVC_JobManager::newInstance()->getExtensionTrackID(pc_Track->getTrackNo());
						// End 20万人対応  2013/11/27 Soma.
						pc_Track->getTrackName(s_RcvNotify_UserInfo.sc_TrackName);
						pc_Track->getTrackDispNameUTF8(s_RcvNotify_UserInfo.sc_TrackDispName);
						pc_Track->getTrackPassword(s_RcvNotify_UserInfo.sc_TrackPassword);
					}
					if (NULL != pc_TrackManager){
						s_RcvNotify_UserInfo.sc_TrackMode				= pc_TrackManager->getTrackMode();
					}
					MyTraceSD(JSVD_DBG_INFO, "GetRecieveDataIDList UserID " , (int)s_RcvNotify_UserInfo.ull_UserID );// 20万人対応  2013/11/27 Soma.
					MyTraceSD(JSVD_DBG_INFO, "GetRecieveDataIDList TrackID " , (int)s_RcvNotify_UserInfo.ull_TrackID );// 20万人対応  2013/11/27 Soma.

					s_RcvNotify_UserInfo.uc_InspectLogIF = pc_myJob->getJobSpec()->pc_Common->getInterface();
					s_RcvNotify_UserInfo.sc_EnableSimpleAuthenticationPrint = APIC_UserAuthenticationSetting::newInstance()->getEnableSimpleAuthenticationPrint();

					// シリアライズ処理.
					Bool bRtn = False;
					FILE* fp = NULL;
					do{
						fp = JSVF_Fopen(pc_ReqReceiveAPP->getFilePath(), "wb");
						if(fp == NULL){
							break;
						}
						JSVF_Fwrite(&s_RcvNotify_UserInfo, sizeof(s_RcvNotify_UserInfo), 1, fp);		// JSVS_RcvNotify_UserInfo.

						if(JSVF_Feof(fp) != 0 || JSVF_Ferror(fp) != 0){
							break;
						}
						bRtn = True;
					}while(0);

					if(fp != NULL){
						JSVF_Fclose(fp);
					}

					// リクエスト処理の完了を設定.
					e_Request_Result = JSVE_Request_Result_Ok;
					st_ResReceiveApp.ul_Result = IPCE_JobServiceResult_OK;
				}
			}else{
				// 該当Jobポインタが見つからない場合.
				e_Request_Result = JSVE_Request_Result_Err;
				st_ResReceiveApp.ul_ResultDetail = JSVE_JobServiceResultDetail_IdNotMatch;
				MyTraceSLD(JSVD_DBG_ERROR,"JSVC_RequestOneShotReceiveAPP::startAction SearchResultNo" , (intptr_t)pc_ReqReceiveAPP );	// warning対応 2013/09/18 ooba.
			}
			break;
		case JSVE_JobManager_MsgID_RcvNotify_Cancel:				// 受信キャンセル要求				API/SRVM向け.
			// JSVS_ReqPara_ReceiveAppCancel.
			{
				// シーケンスIDに対応するジョブポインタを特定し、req_Cancelを行う.
				// 破棄要因については、トラブル発生(TYPE_DeleteJobCause_Fatal)とする.
				// 該当Jobポインタが見つかった場合.
				if (True == b_SearchRet){
					if (NULL == pc_myJob){
						e_Request_Result = JSVE_Request_Result_Err;
						MyTraceSLD(JSVD_DBG_ERROR,"JSVC_RequestOneShotReceiveAPP::startAction pc_Job NULL_2" , (intptr_t)pc_ReqReceiveAPP );	// warning対応 2013/09/18 ooba.
					}else{
						pc_myJob->req_cancel(ans_cancel_seq,TYPE_DeleteJobCause_Fatal);
						e_Request_Result = JSVE_Request_Result_Ok;
						st_ResReceiveApp.ul_Result = IPCE_JobServiceResult_OK;
					}
				}else{
					// 該当Jobポインタが見つからない場合.
					e_Request_Result = JSVE_Request_Result_Err;
					st_ResReceiveApp.ul_ResultDetail = JSVE_JobServiceResultDetail_IdNotMatch;
					MyTraceSLD(JSVD_DBG_ERROR,"JSVC_RequestOneShotReceiveAPP::startAction SearchResultNo_2" , (intptr_t)pc_ReqReceiveAPP );
				}
			}
			break;
		case JSVE_JobManager_MsgID_RcvNotify_GetRecieveDataIDList:		// 受信データIDリスト取得要求.
			{
				//取得データのサイズが動的に変わるので、ここだけ処理が特別.

				uint32_t ul_DataIDCount = 0;

				MyTraceSD(JSVD_DBG_INFO, "GetRecieveDataIDList UserID " , (int)ull_UserID );// 20万人対応  2013/11/27 Soma.
				MyTraceSD(JSVD_DBG_INFO,  "GetRecieveDataIDList TrackID " , (int)ull_TrackID );// 20万人対応  2013/11/27 Soma.
				MyTraceSD(JSVD_DBG_INFO,  "GetRecieveDataIDList e_RcvJobType " , (int)e_RcvJobType );

				// シーケンスIDをキーにJobポインタの検索を行ない、.
				// 受信したジョブの数を調べる
				APIC_ActiveJobList::newInstance()->getList(TYPE_ActiveJobListAll, 1, 350, &c_MyList);
				for (pc_myJob = c_MyList.findFirst(); pc_myJob; pc_myJob = c_MyList.findNext())
				{
					// 未投入かつ受信系のジョブを検索(要求元の指定によるフィルタリングあり).
					if (( 0 == pc_myJob->getJobSpec()->pc_Common->getSequenceId() ) &&											// 未割り当て確認.
						// 2013/05/07 復旧ジョブシーケンス見直し対応(CQ379669) modify start.
						//( IPCE_RcvJobType_Non != JSVC_JobManager::newInstance()->getRcvJobType(pc_myJob->getJobSpec(), pc_myJob->getJobStatus()) ) &&		// レガシー投入確認.
						( IPCE_RcvJobType_Non != JSVC_JobManager::newInstance()->getRcvJobType(pc_myJob) ) &&		// レガシー投入確認.
						// 2013/05/07 復旧ジョブシーケンス見直し対応(CQ379669) modify end.
						( True == isRequestedRecieveData( ull_UserID ,ull_TrackID ,e_RcvJobType ,pc_myJob )) &&					// 検索条件確認.
// 2012.06.05 Sukigara add s.
						( JOBE_RestartTable_JobSpecFixed == pc_myJob->getRestartTableState() ))									// Fix済み確認.
// 2012.06.05 Sukigara add e.
					{
						//見つかったものを保持しながら、カウンタをインクリメントしていく.
						ul_DataIDCount++;
					}
				}
				//データを保持するための一時領域を確保.
				short  ss_mallocSize =  static_cast<short>((ul_DataIDCount+1) * sizeof(JSVS_RcvDataIDList));	//サイズ０を防止.
				uint32_t* pDataHead = (uint32_t*)malloc(ss_mallocSize);
//Str【 静的解析対応 : Sky田村 : 2012.09.03 : CID-439630 】.
				if( pDataHead == NULL )
				{
					e_Request_Result = JSVE_Request_Result_Err;
					MyTraceSD(JSVD_DBG_ERROR,"JSVC_RequestOneShotReceiveAPP::startAction malloc Error:" , ss_mallocSize );
					return e_Request_Result;		//startAction後にリクエストは消去.
				}
//End【 静的解析対応 : Sky田村 : 2012.09.03 : CID-439630 】.
				memset(pDataHead,0,ss_mallocSize);
				JSVS_RcvDataIDList *pData = (JSVS_RcvDataIDList *)pDataHead;// 20万人対応  2013/11/27 Soma.

				//データが存在する場合は.
				if(ul_DataIDCount>0)
				{
					for (pc_myJob = c_MyList.findFirst(); pc_myJob; pc_myJob = c_MyList.findNext())
					{
						// 未投入かつ受信系のジョブを検索(要求元の指定によるフィルタリングあり).
						if (( 0 == pc_myJob->getJobSpec()->pc_Common->getSequenceId() ) &&											// 未割り当て確認.
							// 2013/05/07 復旧ジョブシーケンス見直し対応(CQ379669) modify start.
							//( IPCE_RcvJobType_Non != JSVC_JobManager::newInstance()->getRcvJobType(pc_myJob->getJobSpec(), pc_myJob->getJobStatus()) ) &&		// レガシー投入確認.
							( IPCE_RcvJobType_Non != JSVC_JobManager::newInstance()->getRcvJobType(pc_myJob) ) &&		// レガシー投入確認.
							// 2013/05/07 復旧ジョブシーケンス見直し対応(CQ379669) modify end.
							( True == isRequestedRecieveData( ull_UserID ,ull_TrackID ,e_RcvJobType ,pc_myJob )) &&					// 検索条件確認.// 20万人対応  2013/11/27 Soma.

// 2012.06.05 Sukigara add s.
							( JOBE_RestartTable_JobSpecFixed == pc_myJob->getRestartTableState() ))									// Fix済み確認.
// 2012.06.05 Sukigara add e.
						{
// Start 20万人対応  2013/11/27 Soma.
							pData->ull_UserID = JSVC_JobManager::newInstance()->getExtensionUserID(pc_myJob->getJobSpec()->pc_Common->c_UserAcount.getUserNo());
							pData->ull_TrackID = JSVC_JobManager::newInstance()->getExtensionTrackID(pc_myJob->getJobSpec()->pc_Common->c_UserAcount.getTrackNo());
							pData->ul_RcvDataID = pc_myJob->getJobStatus()->getJobID();

							pData += sizeof(JSVS_RcvDataIDList);
// End 20万人対応  2013/11/27 Soma.
						}
					}
				}
				//取得結果をレスポンスへ反映.
				pc_ResReceiveApp->setResult( IPCE_JobServiceResult_OK );
				pc_ResReceiveApp->setRcvDataIDCount(ul_DataIDCount);
				pc_ResReceiveApp->setData( (char*)pDataHead , ss_mallocSize  );

				//一時領域を削除.
				free( pDataHead );
				pDataHead = NULL;

				MyTraceSD(JSVD_DBG_INFO, "GetRecieveDataIDList findDataNum " , (int)ul_DataIDCount );

				//結果を上位へ返信する.
				JSVC_JobSendRecv::newInstance()->sendMessageToAPF( JSVE_JobManager_MsgID_RcvNotify_GetRecieveDataIDListAccept , this , pc_ResReceiveApp );

				//送信が済んだらdeleteして抜ける.
				#if DEBUGLOG__REQUEST_LIFE
					MyTraceSLD(JSVD_DBG_INFO, "JSVC_RequestOneShotReceiveAPP::startAction delete res 2" , (intptr_t)pc_ResReceiveApp );
				#endif
				delete pc_ResReceiveApp;
				pc_ResReceiveApp = NULL;
				return JSVE_Request_Result_Complete;		//startAction後にリクエストは消去.

			}
			break;

		default:
			break;
	}

	// 関数内処理結果がキープでなければ、内部メッセージを送信する.
	if (JSVE_Request_Result_ReqKeep != e_Request_Result){
		// レスポンスクラスへのメンバー設定.
		pc_ResReceiveApp->setData( (const char*)&st_ResReceiveApp, sizeof(st_ResReceiveApp) );

		// メッセージ送信.
		s_MsgParam.ul_MsgTypeID			= _ul_MsgTypeID;
		s_MsgParam.pReq					= (JSVC_RequestBase*)this;
		s_MsgParam.pRes					= (JSVC_ResponseBase*)pc_ResReceiveApp;
		JSVC_JobManager::newInstance()->sendMessageToJob( &s_MsgParam );
		// pc_ResReceiveAppはメッセージ受信時の処理の最後でdeleteとなる.
	}else{
		// nop.
		// pc_ResReceiveAppもdelete済みなので特に処理なし.
	}

	MyTraceSLD(JSVD_DBG_INFO, "JSVC_RequestOneShotReceiveAPP::startAction End" , (intptr_t)pc_ReqReceiveAPP );

	return e_Request_Result;		//startAction後にリクエストは消去.
}
// 指定した応答に関連するリクエストかどうかを判定.
//  return		JSVD_REQUEST_NOT_MATCH		関係なし.
//				JSVD_REQUEST_MAYBE_MATCH	関係あり（他候補の可能性あり）.
//				JSVD_REQUEST_JUST_MATCH		関係あり（他候補の可能性なし）.
short JSVC_RequestOneShotReceiveAPP::isResponseMsg(JSVC_Response* pRes)
{
#ifdef _DEBUG_JSVC_SEQ
	MyTraceSLD(JSVD_DBG_INFO, "JSVC_RequestOneShotReciveAPP::isResponseMsg pRes:" , (intptr_t)pRes );	// 回避方法をトレースに変更.	// warning対応 2013/09/18 ooba.
#endif
	return JSVD_REQUEST_NOT_MATCH;
}
// pFileのポインタ取得関数.
char* JSVC_RequestOneShotReceiveAPP::getFilePath()
{
	return pFile;
}
// ul_FilePathSizeの値取得関数.
uint32_t JSVC_RequestOneShotReceiveAPP::getFilePathSize()
{
	return ul_FilePathSize;
}
// サブシナリオID取得.
uint32_t JSVC_RequestOneShotReceiveAPP::getSubScenarioID()
{
	return JSVC_RequestOneShotReceiveAPP::ul_SubScenarioID;
}
// シーケンスID設定(JobID).
uint32_t JSVC_RequestOneShotReceiveAPP::getSequenceId()
{
	return JSVC_RequestOneShotReceiveAPP::ul_SequenceId;
}
void JSVC_RequestOneShotReceiveAPP::ans_cancel_seq(APIC_Job* pc_Job)
{
	// 不要な通知のため、通知は捨てる.
}

// Archi(Step3) 送信機能カスタマイズ Start.
//受信データIDリストの受信に対してジョブ単位での返却判定を実施する.
// pJobが第一～第三パラメータで指定された条件を満たすか確認する.
// ただし、条件に0が指定されている部分は無条件で満たすと判定する.
// 20万人対応 変数の型をlongからlonglongへ変更  2013/11/27 Soma.
Bool JSVC_RequestOneShotReceiveAPP::isRequestedRecieveData(uint64_t  ull_UserID,uint64_t  ull_TrackID,IPCE_JobsRcvJobType e_RcvJobType,APIC_Job* pJob)
{
	Bool bRet = False;
	MyTraceSLD(JSVD_DBG_ERROR, "isRequestedRecieveData param_UserID:" , (int64_t)ull_UserID );	// warning対応 2013/09/18 ooba.
	MyTraceSLD(JSVD_DBG_ERROR, "isRequestedRecieveData param_TrackID:" , (int64_t)ull_TrackID );	// warning対応 2013/09/18 ooba.
	MyTraceSD(JSVD_DBG_ERROR, "isRequestedRecieveData param_e_RcvJobType:" , (int)e_RcvJobType );	// warning対応 2013/09/18 ooba.
	MyTraceSD(JSVD_DBG_ERROR, "isRequestedRecieveData Spec_UserID:" , pJob->getJobSpec()->pc_Common->c_UserAcount.getUserNo() );
	MyTraceSD(JSVD_DBG_ERROR, "isRequestedRecieveData Spec_TrackID:" , pJob->getJobSpec()->pc_Common->c_UserAcount.getTrackNo() );
	//ユーザと部門を先にチェックしておく.
	if( ((uint32_t)TYPD_NotSetting != ull_UserID) && ( ull_UserID != (uint32_t)pJob->getJobSpec()->pc_Common->c_UserAcount.getUserNo() ))	// warning対応 2013/09/21 ooba.
	{
// CQ365358 Start.
		if (( JSVE_SpUserId_anonymous == ull_UserID ) 
		&& ( TYPD_NotSetting == pJob->getJobSpec()->pc_Common->c_UserAcount.getUserNo() )){
			// AnonyMous指定の場合、TYPD_NotSettingを許容する.
// CQ365358 End.
		}else{
			return False;
		}
	}
	if( ((uint32_t)TYPD_NotSetting != ull_TrackID) && ( ull_TrackID != (uint32_t)pJob->getJobSpec()->pc_Common->c_UserAcount.getTrackNo() ))	// warning対応 2013/09/21 ooba.
	{
		if (( TYPD_IndexTrackPublic == ull_TrackID ) && ( TYPD_NotSetting == pJob->getJobSpec()->pc_Common->c_UserAcount.getTrackNo() )){
			// Public指定の場合、TYPD_NotSettingを許容する.
		}else{
			return False;
		}
	}

	//受信ジョブ種別の判定を実施.
	switch ( e_RcvJobType )
	{
		case IPCE_RcvJobType_Non:				///<	指定無し.
			bRet = True;
			break;
		case IPCE_RcvJobType_Net:				///<	Net受信.
			{
				char sc_rxMode = pJob->getJobSpec()->pc_Fax->c_FaxAct.getRxMode();
				if((TYPE_RxModeIPAddFaxRx		== sc_rxMode )
				|| (TYPE_RxModeInternetFaxRx	== sc_rxMode ))
				{
					bRet = True;
				} else {
					bRet = False;
				}
			}
			break;
		case IPCE_RcvJobType_Fax:				///<	Fax受信.
			{
				char sc_rxMode = pJob->getJobSpec()->pc_Fax->c_FaxAct.getRxMode();
				if((TYPE_RxModeNormal	== sc_rxMode )
// [changelog] 2018/11/14 lienvl EagleH RQ-272 Support new IP-Fax (T.38) Start.
				|| (TYPE_RxModeIPFaxRx	== sc_rxMode )
// [changelog] 2018/11/14 lienvl EagleH RQ-272 Support new IP-Fax (T.38) End.
				|| (TYPE_RxModePcRx		== sc_rxMode ))
				{
					bRet = True;
				} else {
					bRet = False;
				}
			}
			break;
		case IPCE_RcvJobType_Tcp:				///<	Tcp受信.
			if( TYPE_JobEntryIF_TCPSocket == pJob->getJobSpec()->pc_Common->getInterface() )
			{
				bRet = True;
			} else {
				bRet = False;
			}
			break;
		case IPCE_RcvJobType_PSWC:				///<	PSWC受信.
			if( TYPE_JobEntryIF_PSWC == pJob->getJobSpec()->pc_Common->getInterface() )
			{
				bRet = True;
			} else {
				bRet = False;
			}
			break;
		case IPCE_RcvJobType_OpenAPI:			///<	OpenAPI受信.
			if( TYPE_JobEntryIF_ISOpenAPI == pJob->getJobSpec()->pc_Common->getInterface() )
			{
				bRet = True;
			} else {
				bRet = False;
			}
			break;
		case IPCE_RcvJobType_Browser:			///<	Browser受信.
			if( True == pJob->getJobSpec()->pc_Send->c_ComSend.getWebBrowserScanOn() )
			{
				bRet = True;
			} else {
				bRet = False;
			}
			break;
		case IPCE_RcvJobType_ImageLog:			///<	ImageLog受信.
			if( TYPD_ImageLogSendJobNon != pJob->getJobSpec()->pc_Common->getImageLogSend() )
			{
				bRet = True;
			} else {
				bRet = False;
			}
			break;
		default:
			bRet = False;
			break;
	}
	return bRet;
}

//--------------------------------------------------------------------------------------------------.
//	JSVC_ResponseReceiveAppクラス定義.
//--------------------------------------------------------------------------------------------------.
// コンストラクタ.
JSVC_ResponseReceiveApp::JSVC_ResponseReceiveApp(uint32_t ul_MsgTypeID)
:	JSVC_Response(ul_MsgTypeID), ul_Param1(0),ul_Param2(0),
	ul_SequenceId(0),e_RcvNotifyKind(0),ul_RcvDataIDCount(0),ps_RcvDataIDList(NULL)
{
}
// デストラクタ.
JSVC_ResponseReceiveApp::~JSVC_ResponseReceiveApp()
{
	if( ps_RcvDataIDList )
	{
		free(ps_RcvDataIDList);
		ps_RcvDataIDList = NULL;
	}
}
// 自クラスの型を返す.
JSVE_Request_Class_Type JSVC_ResponseReceiveApp::WhoAmI()
{
	return JSVE_Request_Class_Type_Res_RcvApp;
}
//
// リクエスト受付時のデータ保存処理.
//
//	return	True　:保存OK
//			False :保存NG
//
Bool JSVC_ResponseReceiveApp::setData(const char* pRcvData,short ss_size)
{
	Bool bRet = True;
#ifdef _DEBUG_JSVC_SEQ
	MyTraceSD(JSVD_DBG_INFO, "JSVC_ResponseReceiveApp::setData ss_size:" , ss_size );	// 回避方法をトレースに変更.
#endif

	JSVS_ResponseParamReceiveApp *pRcv = (JSVS_ResponseParamReceiveApp*)pRcvData;

	switch( getMsgTypeId() ){
		case JSVE_JobManager_MsgID_RcvNotify_Add:					// 受信通知登録要求					SRVM向け.
		case JSVE_JobManager_MsgID_RcvNotify_GetSetting:			// 受信情報設定取得					API向け.
		case JSVE_JobManager_MsgID_RcvNotify_GetStatus:				// 受信情報ステータス取得			API向け.
		case JSVE_JobManager_MsgID_RcvNotify_GetUserInfo:			// 受信情報ユーザ情報取得			API向け.
		case JSVE_JobManager_MsgID_SubScenario_GetSetting:			// サブシナリオ設定取得				SRVM向け.
		case JSVE_JobManager_MsgID_SubScenario_GetStatus:			// サブシナリオステータス情報取得	SRVM向け.
		case JSVE_JobManager_MsgID_RcvNotify_Cancel:				// 受信キャンセル要求				API/SRVM向け.
		case JSVE_JobManager_MsgID_RcvNotify_AddAccept:				// 受信通知登録要求					SRVM向け.
		case JSVE_JobManager_MsgID_RcvNotify_GetSettingAccept:		// 受信情報設定取得					API向け.
		case JSVE_JobManager_MsgID_RcvNotify_GetStatusAccept:		// 受信情報ステータス取得			API向け.
		case JSVE_JobManager_MsgID_RcvNotify_GetUserInfoAccept:		// 受信情報ユーザ情報取得			API向け.
		case JSVE_JobManager_MsgID_SubScenario_GetSettingAccept:	// サブシナリオ設定取得				SRVM向け.
		case JSVE_JobManager_MsgID_SubScenario_GetStatusAccept:		// サブシナリオステータス情報取得	SRVM向け.
		case JSVE_JobManager_MsgID_RcvNotify_CancelAccept:			// 受信キャンセル要求				API/SRVM向け.
		case JSVE_JobManager_MsgID_RcvNotify_AddEnd:				// 受信通知登録要求					SRVM向け.
		case JSVE_JobManager_MsgID_RcvNotify_GetSettingEnd:			// 受信情報設定取得					API向け.
		case JSVE_JobManager_MsgID_RcvNotify_GetStatusEnd:			// 受信情報ステータス取得			API向け.
		case JSVE_JobManager_MsgID_RcvNotify_GetUserInfoEnd:		// 受信情報ユーザ情報取得			API向け.
		case JSVE_JobManager_MsgID_SubScenario_GetSettingEnd:		// サブシナリオ設定取得				SRVM向け.
		case JSVE_JobManager_MsgID_SubScenario_GetStatusEnd:		// サブシナリオステータス情報取得	SRVM向け.
		case JSVE_JobManager_MsgID_RcvNotify_CancelEnd:				// 受信キャンセル要求				API/SRVM向け.
		case JSVE_JobManager_MsgID_JobSpecFixed:					// JobSpec確定通知.
		// ActiveJobListからの状態変化通知(ジョブの優先順位変更).
		case JSVE_JobManager_MsgID_recieveSecureJob:
		case JSVE_JobManager_MsgID_StopTargetJobWasDeleted:			// comment Sky新大阪 [特ROM対応][IT5-6] プリントエラー解除時に他人のジョブの混在を防止する対応.

// comment [特ROM対応][IT5-26] エラー発生時のジョブの削除方法を自動又は手動に設定可能とする対応 ->.
		case JSVE_JobManager_MsgID_StartActiveJobListDeleteSet:		// ジョブ自動削除開始のオブザーバ通知.
		case JSVE_JobManager_MsgID_EndActiveJobListDeleteSet:		// ジョブ自動削除終了のオブザーバ通知.
// comment [特ROM対応][IT5-26] エラー発生時のジョブの削除方法を自動又は手動に設定可能とする対応 <-.

			// JSVS_ResponseParamReceiveAppからクラス内メンバーへデータ設定.
			ul_SequenceId		= pRcv->ul_SequenceId;				// シーケンスID.
			ul_Result			= pRcv->ul_Result;					// 結果.
			ul_ResultDetail		= pRcv->ul_ResultDetail;			// 結果詳細.
			e_RcvNotifyKind		= pRcv->e_RcvNotifyKind;			// 受信種類
			ul_SubScenarioID	= pRcv->ul_SubScenarioID;			// サブシナリオID.
			pJob				= pRcv->pJob;						// Jobインスタンス.
			break;
		case JSVE_JobManager_MsgID_RcvNotify_GetRecieveDataIDListAccept:		// 受信データIDリスト取得要求.
			{
				//確保済のメモリがあれば解放しておく.
				if ( ps_RcvDataIDList ){
					free( ps_RcvDataIDList );
					ps_RcvDataIDList = NULL;
				}
				//サイズそのまま確保する.
				ps_RcvDataIDList = (JSVS_RcvDataIDList*) malloc(ss_size);
//Str【 静的解析対応 : Sky田村 : 2012.09.03 : CID-439628 】.
				if( ps_RcvDataIDList == NULL )
				{
					MyTraceSD( JSVD_DBG_ERROR, "JSVC_ResponseReceiveApp::setData malloc Error:" , ss_size );
					bRet = False;
					return bRet;
				}
//End【 静的解析対応 : Sky田村 : 2012.09.03 : CID-439628 】.
				memcpy( ps_RcvDataIDList , pRcvData , ss_size );
			}
			break;

		//処理が同期なのでこのMSGはこないが、defaultへ流れるようにしておく.
		case JSVE_JobManager_MsgID_RcvNotify_GetRecieveDataIDListEnd:		// 受信データIDリスト取得要求.
		default:
			MyTraceSD( JSVD_DBG_ERROR, "JSVC_ResponseReceiveApp::setData unknown msgTypeId :" , (int)getMsgTypeId() );	// warning対応 2013/09/18 ooba.
			bRet = False;
			break;
	}

	return bRet;
}

short JSVC_ResponseReceiveApp::getSndDataSize()
{
	short ss_size = -1;

	switch( getMsgTypeId() )
	{
		case JSVE_JobManager_MsgID_RcvNotify_AddAccept:						// 受信通知登録要求					SRVM向け.
		case JSVE_JobManager_MsgID_RcvNotify_GetSettingAccept:				// 受信情報設定取得					API向け.
		case JSVE_JobManager_MsgID_RcvNotify_GetStatusAccept:				// 受信情報ステータス取得			API向け.
		case JSVE_JobManager_MsgID_RcvNotify_GetUserInfoAccept:				// 受信情報ユーザ情報取得			API向け.
		case JSVE_JobManager_MsgID_SubScenario_GetSettingAccept:			// サブシナリオ設定取得				SRVM向け.
		case JSVE_JobManager_MsgID_SubScenario_GetStatusAccept:				// サブシナリオステータス情報取得	SRVM向け.
		case JSVE_JobManager_MsgID_RcvNotify_CancelAccept:					// 受信キャンセル要求				API/SRVM向け.
			// [changelog] 2018/01/15 Cuong #245418 Fix static check for IT5_4.2 F00-U2(07) start.
			ss_size = static_cast<short>(sizeof( JSVS_ReceiveAppAnsMsg ));
			// [changelog] 2018/01/15 Cuong #245418 Fix static check for IT5_4.2 F00-U2(07) end.
			break;
		case JSVE_JobManager_MsgID_JobSpecFixed:							// JobSpec確定通知.
			// [changelog] 2018/01/15 Cuong #245418 Fix static check for IT5_4.2 F00-U2(07) start.
			ss_size = static_cast<short>(sizeof( JSVS_ReceiveAppNoticeMsg ));
			// [changelog] 2018/01/15 Cuong #245418 Fix static check for IT5_4.2 F00-U2(07) end.
			break;
		case JSVE_JobManager_MsgID_RcvNotify_GetRecieveDataIDListAccept:		// 受信データIDリスト取得要求.
			//可変長データとなるので最大理論値を返す.
			// [changelog] 2018/01/15 Cuong #245418 Fix static check for IT5_4.2 F00-U2(07) start.
			ss_size = static_cast<short>(sizeof(JSVS_ReceiveAppIdListAnsMsg) - sizeof(uint32_t) + sizeof(JSVS_RcvDataIDList) * ul_RcvDataIDCount);
			// [changelog] 2018/01/15 Cuong #245418 Fix static check for IT5_4.2 F00-U2(07) end.
			break;
		default:
			// 異常値.
			MyTraceSD(JSVD_DBG_ERROR, "許容範囲外の値",(int)getMsgTypeId() );
			ss_size = -1;
			break;
	}
	return ss_size;
}
short JSVC_ResponseReceiveApp::MakeDataStream(JSVC_RequestBase* pc_Req, char* pSndData)
{
	short ss_size = -1;
	if ( pSndData != NULL ) {
		JSVE_JobManager_MsgID ul_MsgId = JSVE_JobManager_MsgID_Non;
		if( pc_Req ){
			ul_MsgId = pc_Req->getMsgTypeId();
		} else {
			ul_MsgId = this->getMsgTypeId();
		}
		switch( ul_MsgId )
		{
	// 同期で一気に処理してしまうのでJSVE_JobManager_MsgID_RcvNotify_Addに変更（Acceptではなく）.
		case JSVE_JobManager_MsgID_RcvNotify_Add:							// 受信通知登録要求					SRVM向け.
		case JSVE_JobManager_MsgID_RcvNotify_GetSettingAccept:				// 受信情報設定取得					API向け.
		case JSVE_JobManager_MsgID_RcvNotify_GetStatusAccept:				// 受信情報ステータス取得			API向け.
		case JSVE_JobManager_MsgID_RcvNotify_GetUserInfoAccept:				// 受信情報ユーザ情報取得			API向け.
		case JSVE_JobManager_MsgID_RcvNotify_GetSetting:					// 受信情報設定取得					API向け.
		case JSVE_JobManager_MsgID_RcvNotify_GetStatus:						// 受信情報ステータス取得			API向け.
		case JSVE_JobManager_MsgID_RcvNotify_GetUserInfo:					// 受信情報ユーザ情報取得			API向け.
		case JSVE_JobManager_MsgID_RcvNotify_Cancel:						// 受信キャンセル要求				API/SRVM向け.
		case JSVE_JobManager_MsgID_RcvNotify_CancelAccept:					// 受信キャンセル要求				API/SRVM向け.
			{
				JSVS_ReceiveAppAnsMsg* pc_ReceiveAppAnsMsg;
				ss_size = sizeof(JSVS_ReceiveAppAnsMsg);
				pc_ReceiveAppAnsMsg = (JSVS_ReceiveAppAnsMsg*)pSndData;
				// 結果.
				pc_ReceiveAppAnsMsg->e_JobServiceResult = getResult();
				// 結果詳細.
				pc_ReceiveAppAnsMsg->e_JobServiceResultDetail = getResultDetail();
				// 受信データID.
				pc_ReceiveAppAnsMsg->ul_RcvDataID = getSequenceId();

				pc_ReceiveAppAnsMsg->ul_ScenarioID		= ul_Param1;
				pc_ReceiveAppAnsMsg->ul_JobResourceID	= ul_Param2;
			}
			break;
		case JSVE_JobManager_MsgID_SubScenario_GetSettingAccept:			// サブシナリオ設定取得				SRVM向け.
		case JSVE_JobManager_MsgID_SubScenario_GetStatusAccept:				// サブシナリオステータス情報取得	SRVM向け.
		case JSVE_JobManager_MsgID_SubScenario_GetSetting:					// サブシナリオ設定取得				SRVM向け.
		case JSVE_JobManager_MsgID_SubScenario_GetStatus:					// サブシナリオステータス情報取得	SRVM向け.
			{
				JSVS_ReceiveAppAnsMsg* pc_ReceiveAppAnsMsg;
				ss_size = sizeof(JSVS_ReceiveAppAnsMsg);
				pc_ReceiveAppAnsMsg = (JSVS_ReceiveAppAnsMsg*)pSndData;
				// 結果.
				pc_ReceiveAppAnsMsg->e_JobServiceResult = getResult();
				// 結果詳細.
				pc_ReceiveAppAnsMsg->e_JobServiceResultDetail = getResultDetail();
				// 受信データID.
				pc_ReceiveAppAnsMsg->ul_RcvDataID = getSubScenarioID();

				pc_ReceiveAppAnsMsg->ul_ScenarioID		= ul_Param1;
				pc_ReceiveAppAnsMsg->ul_JobResourceID	= ul_Param2;
			}
			break;
		case JSVE_JobManager_MsgID_JobSpecFixed:							// JobSpec確定通知.
			{
				//JobSpec確定通知を受けて送信するのは「受信通知」となる、.
				JSVS_ReceiveAppNoticeMsg* ps_ReceiveAppNoticeMsg;
				ss_size = sizeof(JSVS_ReceiveAppNoticeMsg);
				ps_ReceiveAppNoticeMsg = (JSVS_ReceiveAppNoticeMsg*)pSndData;
				APIC_Job *pJob = getJobInstance();
				if( pJob->getJobSpec() )
				{
					if( pJob->getJobSpec()->pc_Common )
					{
						// Start 20万人対応  2013/11/27 Soma.
						ps_ReceiveAppNoticeMsg->ull_UserID = JSVC_JobManager::newInstance()->getExtensionUserID(pJob->getJobSpec()->pc_Common->c_UserAcount.getUserNo());
						ps_ReceiveAppNoticeMsg->ull_TrackID = JSVC_JobManager::newInstance()->getExtensionTrackID(pJob->getJobSpec()->pc_Common->c_UserAcount.getTrackNo());
						// End 20万人対応  2013/11/27 Soma.
						ps_ReceiveAppNoticeMsg->ul_RcvDataID = pJob->getJobStatus()->getJobID();
					}
				}
			}
			break;
		case JSVE_JobManager_MsgID_RcvNotify_GetRecieveDataIDList:			// 受信データIDリスト取得要求.
			//データ作成.
			{
				JSVS_ReceiveAppIdListAnsMsg* pData = (JSVS_ReceiveAppIdListAnsMsg*)pSndData;
				pData->e_JobServiceResult = getResult();
				pData->e_JobServiceResultDetail = getResultDetail();
				pData->sl_DataCount = ul_RcvDataIDCount;
				memcpy( (void*)&(pData->s_RcvData[0]) , (void*)ps_RcvDataIDList, sizeof(JSVS_RcvDataIDList)*ul_RcvDataIDCount);
				//サイズを計算.
				//可変長となるので実データサイズを計算して返す.
				ss_size  = sizeof(JSVS_ReceiveAppIdListAnsMsg) - sizeof(uint32_t);	//基本サイズ.
				ss_size += sizeof(JSVS_RcvDataIDList) * ul_RcvDataIDCount;		//可変長部のサイズ.
			}
			break;
		case JSVE_JobManager_MsgID_RcvNotify_GetRecieveDataIDListAccept:	// 受信データIDリスト取得応答.
			{
				// ありえないパターン.
				MyTraceSD(JSVD_DBG_ERROR, "【Debug】受信データIDリスト取得応答",(int)getMsgTypeId());
			}
			break;

		default:
			// 異常値.
			MyTraceSD(JSVD_DBG_ERROR, "許容範囲外の値",(int)getMsgTypeId());
			ss_size = -1;
			// [changelog] 2018/01/15 Dat #245347 Fix static check for IT5_4.2 F00-U2(07) start.
			break;
			// [changelog] 2018/01/15 Dat #245347 Fix static check for IT5_4.2 F00-U2(07) end.
		}
	}
	return ss_size;
}

// シーケンスID取得.
uint32_t JSVC_ResponseReceiveApp::getSequenceId()
{
	return JSVC_ResponseReceiveApp::ul_SequenceId;
}
// 受信種類取得.
JSVE_RcvNotifyKind JSVC_ResponseReceiveApp::getRcvNotifyKind()
{
	return JSVC_ResponseReceiveApp::e_RcvNotifyKind;
}



//--------------------------------------------------------------------------------------------------.
//	JSVC_ResponseVariousFileDataクラス定義.
//--------------------------------------------------------------------------------------------------.
// コンストラクタ.
JSVC_ResponseVariousFileData::JSVC_ResponseVariousFileData(uint32_t ul_MsgTypeID)
: JSVC_Response(ul_MsgTypeID),sl_FileID(0)
{
}
// デストラクタ
JSVC_ResponseVariousFileData::~JSVC_ResponseVariousFileData()
{
}
// 自身の型を示す.
JSVE_Request_Class_Type JSVC_ResponseVariousFileData::WhoAmI()
{
	return JSVE_Request_Class_Type_Res_VariousFile;
}
// リクエスト受付時のデータ保存処理.
Bool JSVC_ResponseVariousFileData::setData(const char* pRcvData,short ss_size)
{
	if( NULL != pRcvData )
	{	
		JSVS_VariousFileDataNoticeMsg* pData = (JSVS_VariousFileDataNoticeMsg*)pRcvData;
		setResult( pData->e_JobServiceResult );
		setResultDetail( pData->e_JobServiceResultDetail );
		if( getMsgTypeId() == JSVE_JobManager_MsgID_VariousFileDataSend_CopyEnd )
		{
			sl_FileID = pData->sl_FileID;
		}
	}else{
		return False;
	}
	return True;
}

short JSVC_ResponseVariousFileData::getSndDataSize()
{
	short ss_size = -1;

	switch( getMsgTypeId() )
	{
		case JSVE_JobManager_MsgID_VariousFileDataSend_CopyAccept:				// 受信情報設定取得					API向け.
		case JSVE_JobManager_MsgID_RcvNotify_CreateVariousFileDataAccept:		// 受信通知登録要求					SRVM向け.
		case JSVE_JobManager_MsgID_VariousFileDataSend_DeleteAccept:			// 受信情報ステータス取得			API向け.
		case JSVE_JobManager_MsgID_VariousFileDataSend_CopyEnd:					// 受信情報設定取得					API向け.
		case JSVE_JobManager_MsgID_RcvNotify_CreateVariousFileDataEnd:			// 受信通知登録要求					SRVM向け.
		case JSVE_JobManager_MsgID_VariousFileDataSend_DeleteEnd:				// 受信情報ステータス取得			API向け.
		case JSVE_JobManager_MsgID_RcvNotify_CreateVariousFileData:				// ファイル作成要求.
		case JSVE_JobManager_MsgID_VariousFileDataSend_Copy:					// ファイルコピー要求.
		case JSVE_JobManager_MsgID_VariousFileDataSend_Delete:					// ファイル削除要求.
			ss_size = sizeof( JSVS_VariousFileDataNoticeMsg );
			break;

		default:
			// 異常値.
			MyTraceSD(JSVD_DBG_ERROR, "許容範囲外の値",(int)getMsgTypeId() );
			ss_size = -1;
			break;
	}
	return ss_size;
}
short JSVC_ResponseVariousFileData::MakeDataStream(JSVC_RequestBase* pc_Req, char* pSndData)
{
	short ss_size = -1;
	if ( pSndData != NULL ) {

		switch( pc_Req->getMsgTypeId() )
		{
			//受付完了時.
			case JSVE_JobManager_MsgID_RcvNotify_CreateVariousFileDataAccept:	// 受信情報ユーザ情報取得			API向け.
			case JSVE_JobManager_MsgID_VariousFileDataSend_CopyAccept:			// サブシナリオステータス情報取得	SRVM向け.
			case JSVE_JobManager_MsgID_VariousFileDataSend_DeleteAccept:		// 受信キャンセル要求				API/SRVM向け.
			case JSVE_JobManager_MsgID_RcvNotify_CreateVariousFileData:			// ファイル作成要求.
			case JSVE_JobManager_MsgID_VariousFileDataSend_Copy:				// ファイルコピー要求.
			case JSVE_JobManager_MsgID_VariousFileDataSend_Delete:				// ファイル削除要求.
				{
					JSVS_VariousFileDataNoticeMsg* pData = (JSVS_VariousFileDataNoticeMsg*)pSndData;
					pData->e_JobServiceResult = getResult();
					pData->e_JobServiceResultDetail = getResultDetail();
					ss_size = sizeof(JSVS_VariousFileDataNoticeMsg) - sizeof(int32_t);	//ファイルIDは不要.
				}
				break;
		
			//実行完了時.
			case JSVE_JobManager_MsgID_RcvNotify_CreateVariousFileDataEnd:		// 受信情報ユーザ情報取得			API向け.
			case JSVE_JobManager_MsgID_VariousFileDataSend_DeleteEnd:			// 受信キャンセル要求				API/SRVM向け.
				{
					JSVS_VariousFileDataNoticeMsg* pData = (JSVS_VariousFileDataNoticeMsg*)pSndData;
					pData->e_JobServiceResult = getResult();
					pData->e_JobServiceResultDetail = getResultDetail();
					ss_size = sizeof(JSVS_VariousFileDataNoticeMsg) - sizeof(int32_t);	//ファイルIDは不要.
				}
				break;
			case JSVE_JobManager_MsgID_VariousFileDataSend_CopyEnd:				// サブシナリオステータス情報取得	SRVM向け.
				{
					JSVS_VariousFileDataNoticeMsg* pData = (JSVS_VariousFileDataNoticeMsg*)pSndData;
					pData->e_JobServiceResult = getResult();
					pData->e_JobServiceResultDetail = getResultDetail();
					pData->sl_FileID = getFileID();
					ss_size = sizeof(JSVS_VariousFileDataNoticeMsg);
				}
				break;

			default:
				// 異常値.
				MyTraceSD(JSVD_DBG_ERROR, "許容範囲外の値",(int)getMsgTypeId());
				ss_size = -1;
				// [changelog] 2018/01/15 Dat #245266 Fix static check for IT5_4.2 F00-U2(07) start.
				break;
				// [changelog] 2018/01/15 Dat #245266 Fix static check for IT5_4.2 F00-U2(07) end.

		}
	}
	return ss_size;
}
// ファイルID取得.
int32_t JSVC_ResponseVariousFileData::getFileID()
{
	return sl_FileID;
}


//--------------------------------------------------------------------------------------------------.
//	JSVC_ResponseVariousFileDataクラス定義.
//--------------------------------------------------------------------------------------------------.
// コンストラクタ.
JSVC_ResponseSetJobStatus::JSVC_ResponseSetJobStatus(uint32_t ul_MsgTypeID)
: JSVC_Response(ul_MsgTypeID),	ul_SubScenarioID(0)
{
}
// デストラクタ
JSVC_ResponseSetJobStatus::~JSVC_ResponseSetJobStatus()
{
}
//< 自身の型を示す.
JSVE_Request_Class_Type JSVC_ResponseSetJobStatus::WhoAmI()
{
	return JSVE_Request_Class_Type_Res_SetJobStatus;
}
///< リクエスト受付時のデータ保存処理.
Bool JSVC_ResponseSetJobStatus::setData(const char* pRcvData,short ss_size)
{
	ul_SubScenarioID = *(uint32_t*)pRcvData;
	return True;
}
// 送信時に確保すべきデータサイズ.
short JSVC_ResponseSetJobStatus::getSndDataSize()
{
	return sizeof(JSVS_SubScenarioAnsMsg);
}
short JSVC_ResponseSetJobStatus::MakeDataStream(JSVC_RequestBase* pc_Req, char* pSndData)
{
	JSVS_SubScenarioAnsMsg* p_msg = (JSVS_SubScenarioAnsMsg*)pSndData;

	p_msg->ul_SubScenarioID			= ul_SubScenarioID;
	p_msg->e_JobServiceResult		= getResult();
	p_msg->e_JobServiceResultDetail	= getResultDetail();

	return sizeof(JSVS_SubScenarioAnsMsg);
}
JSVC_ResponseListOperation::JSVC_ResponseListOperation(uint32_t ul_MsgTypeID)
: JSVC_Response(ul_MsgTypeID),	ul_Num(0),	ul_Total(0)
{
	ul_ScenarioID = 0;
//Str【 静的解析対応 : Sky田村 : 2012.12.13 : CID-447283】.
//	ul_SubScenarioID[0] = 0 ;
	memset( ul_SubScenarioID, 0x00, sizeof( ul_SubScenarioID ) );
	ul_JobResourceID = 0;
	e_JobServiceResultDetail = IPCE_JobsResultDetail_Non;
//End【 静的解析対応 : Sky田村 : 2012.12.13 : CID-447283】.
//Str【 静的解析対応 : Sky田中 : 2012.08.30 : CID-447283 】.
	ul_State = 0;
//End【 静的解析対応 : Sky田中 : 2012.08.30 : CID-447283 】.
	
}
JSVC_ResponseListOperation::~JSVC_ResponseListOperation()
{
}
JSVE_Request_Class_Type JSVC_ResponseListOperation::WhoAmI()
{
	return JSVE_Request_Class_Type_Res_GetActiveSubScenarioList ;
}
Bool JSVC_ResponseListOperation::setData(const char* pRcvData,short ss_size,short ss_total)
{
	switch( getMsgTypeId() ){
		case JSVE_JobManager_MsgID_GetActiveSubScenarioListAccept:
			if( NULL != pRcvData ) {
				JSVS_GetActiveSubScenarioListAnsMsg* pData = (JSVS_GetActiveSubScenarioListAnsMsg*)pRcvData;

				this->ul_Num = pData->ul_Num ;
				this->ul_Total = ss_total ;
				uint32_t ul_Index ;
				for ( ul_Index = 0 ; ul_Index < this->ul_Num ; ul_Index++ ) {
					// 現状固定配列
					this->ul_SubScenarioID[ul_Index] = pData->ul_SubScenarioID[ul_Index] ;
				}
			} else {
				return False;
			}
			break;
		case JSVE_JobManager_MsgID_GetJobResourceAccept:
			{
				JSVS_ListOperationAnsMsg* pAnsMsg = (JSVS_ListOperationAnsMsg*)pRcvData;
				ul_Num = pAnsMsg->ul_Num;
				if( ul_Num ){
					memcpy( (void*)&(this->ul_SubScenarioID[0]) ,(void*)&(pAnsMsg->ul_ListData[0]) ,sizeof(uint32_t)*ul_Num );
				}
			}
			break;
		case JSVE_JobManager_MsgID_ReleaseJobResourceAccept:
			{
				//データ自体の保存としては処理不要.
				//result/resultDetailしか使わないのでここでは返信用構造体は不要.
			}
			break;
// CQ526787 ZeusS_ZX0 MR問題恒久対応 2016/4/4 m.ooba start.
		case JSVE_JobManager_MsgID_ReleaseJobResource2Accept:
			{
				//データ自体の保存としては処理不要.
				//result/resultDetailしか使わないのでここでは返信用構造体は不要.
			}
			break;
// CQ526787 ZeusS_ZX0 MR問題恒久対応 2016/4/4 m.ooba end.
		case JSVE_JobManager_MsgID_SetScenarioIDAccept:
			{
				if( NULL != pRcvData ) {
					JSVS_SetScenarioIDAnsMsg* pData = (JSVS_SetScenarioIDAnsMsg*)pRcvData;

					this->ul_Num = pData->ul_Num;
					this->ul_ScenarioID = pData->ul_ScenarioID;
					uint32_t ul_Index ;
					for ( ul_Index = 0 ; ul_Index < this->ul_Num ; ul_Index++ ) {
						// 現状固定配列
						this->ul_SubScenarioID[ul_Index] = pData->ul_SubScenarioID[ul_Index] ;
					}
				} else {
					return False;
				}
			}
			break;
		default:
			return False;
	}
	return True;
}
short JSVC_ResponseListOperation::getSndDataSize()
{
	switch( getMsgTypeId() ){
		case JSVE_JobManager_MsgID_GetActiveSubScenarioListAccept:
			if (ul_Num > 0) {
				return static_cast<short>(sizeof(JSVS_GetActiveSubScenarioListAnsMsg)+sizeof(uint32_t)*(ul_Num-1));
			} else {
				return static_cast<short>(sizeof(JSVS_GetActiveSubScenarioListAnsMsg)-sizeof(uint32_t));
			}
			break;
		case JSVE_JobManager_MsgID_GetJobResourceAccept:
			{
				short ss_BufferSize = static_cast<short>(sizeof(JSVS_ListOperationAnsMsg)+sizeof(uint32_t)*JSVC_Request_JobResourceMax);
				return ss_BufferSize;
			}
			break;
		case JSVE_JobManager_MsgID_ReleaseJobResourceAccept:
			{
				return sizeof(JSVS_OneShotReq_AnsMsg);
			}
			break;
// CQ526787 ZeusS_ZX0 MR問題恒久対応 2016/4/4 m.ooba start.
		case JSVE_JobManager_MsgID_ReleaseJobResource2Accept:
			{
				return sizeof(JSVS_OneShotReq_AnsMsg);
			}
			break;
// CQ526787 ZeusS_ZX0 MR問題恒久対応 2016/4/4 m.ooba end.
		case JSVE_JobManager_MsgID_JobResourceDataChanged:
			{
				return sizeof(JSVS_JobResourceNoticeMsg);
			}
			break;
		case JSVE_JobManager_MsgID_SetScenarioIDAccept:
			{
				if (ul_Num > 0) {
					return static_cast<short>(sizeof(JSVS_SetScenarioIDAnsMsg)+sizeof(uint32_t)*(ul_Num-1));
				} else {
					return static_cast<short>(sizeof(JSVS_SetScenarioIDAnsMsg)+sizeof(uint32_t));
				}
			}
			break;
		default:
			return False;
	}
}
short JSVC_ResponseListOperation::MakeDataStream(JSVC_RequestBase* pc_Req, char* pSndData)
{
	switch( getMsgTypeId() ){
		case JSVE_JobManager_MsgID_GetActiveSubScenarioListAccept:
			{
				JSVS_GetActiveSubScenarioListAnsMsg* p_msg = (JSVS_GetActiveSubScenarioListAnsMsg*)pSndData;

				p_msg->e_JobServiceResult		= getResult();
				p_msg->e_JobServiceResultDetail	= getResultDetail();
				p_msg->ul_Num					= this->ul_Num ;
				p_msg->ul_Total					= this->ul_Total ;
				if (ul_Num > 0) {
					for (uint32_t ul_cnt = 0; ul_cnt < ul_Num; ul_cnt++) {
						p_msg->ul_SubScenarioID[ul_cnt]	= this->ul_SubScenarioID[ul_cnt];
					}
					return static_cast<short>(sizeof(JSVS_GetActiveSubScenarioListAnsMsg)+sizeof(uint32_t)*(ul_Num-1));
				} else {
					return static_cast<short>(sizeof(JSVS_GetActiveSubScenarioListAnsMsg)-sizeof(uint32_t));
				}
			}
			break;
		case JSVE_JobManager_MsgID_GetJobResourceAccept:
			{
				JSVS_ListOperationAnsMsg* pMsg = (JSVS_ListOperationAnsMsg*)pSndData;

				short ss_BufferSize = static_cast<short>(sizeof(JSVS_ListOperationAnsMsg)-sizeof(uint32_t)+sizeof(uint32_t)*ul_Num);
				
				pMsg->e_JobServiceResult		= getResult();
				pMsg->e_JobServiceResultDetail	= getResultDetail();
				pMsg->ul_Num					= ul_Num;
				if( ul_Num ){
					memcpy( (void*)&(pMsg->ul_ListData[0]) ,(void*)&(this->ul_SubScenarioID[0]) ,sizeof(uint32_t)*ul_Num );
				}
				return ss_BufferSize;
			}
			break;
		case JSVE_JobManager_MsgID_ReleaseJobResourceAccept:
			{
				JSVS_OneShotReq_AnsMsg* pMsg = (JSVS_OneShotReq_AnsMsg*)pSndData;
				pMsg->e_JobServiceResult		= getResult();
				pMsg->ul_Param					= getResultDetail();

				return sizeof(JSVS_OneShotReq_AnsMsg);
			}
			break;
// CQ526787 ZeusS_ZX0 MR問題恒久対応 2016/4/4 m.ooba start.
		case JSVE_JobManager_MsgID_ReleaseJobResource2Accept:
			{
				JSVS_OneShotReq_AnsMsg* pMsg = (JSVS_OneShotReq_AnsMsg*)pSndData;
				pMsg->e_JobServiceResult		= getResult();
				pMsg->ul_Param					= getResultDetail();

				return sizeof(JSVS_OneShotReq_AnsMsg);
			}
			break;
// CQ526787 ZeusS_ZX0 MR問題恒久対応 2016/4/4 m.ooba end.
		case JSVE_JobManager_MsgID_JobResourceDataChanged:
			{
				JSVS_JobResourceNoticeMsg* pMsg = (JSVS_JobResourceNoticeMsg*)pSndData;
				
				pMsg->ul_JobSerourceID = ul_JobResourceID;
				pMsg->ul_State = ul_State;
				pMsg->ul_ScenarioID = ul_ScenarioID;
				pMsg->ul_SubScenarioID = ul_SubScenarioID[0];

				return sizeof(JSVS_JobResourceNoticeMsg);
			}
			break;
		case JSVE_JobManager_MsgID_SetScenarioIDAccept:
			{
				JSVS_SetScenarioIDAnsMsg* p_msg = (JSVS_SetScenarioIDAnsMsg*)pSndData;

				p_msg->e_JobServiceResult		= getResult();
				p_msg->e_JobServiceResultDetail	= getResultDetail();
				p_msg->ul_ScenarioID			= this->ul_ScenarioID;
				p_msg->ul_Num					= this->ul_Num ;
				short ss_BufferSize = 0;
				if (ul_Num > 0) {
					for (uint32_t ul_cnt = 0; ul_cnt < ul_Num; ul_cnt++) {
						p_msg->ul_SubScenarioID[ul_cnt]	= this->ul_SubScenarioID[ul_cnt];
					}
					// [changelog] 2018/01/15 Cuong #245400 Fix static check for IT5_4.2 F00-U2(07) start.
					ss_BufferSize = static_cast<short>(sizeof(JSVS_SetScenarioIDAnsMsg)-sizeof(uint32_t)+sizeof(uint32_t)*ul_Num);
					// [changelog] 2018/01/15 Cuong #245400 Fix static check for IT5_4.2 F00-U2(07) end.
				} else {
					// [changelog] 2018/01/15 Cuong #245400 Fix static check for IT5_4.2 F00-U2(07) start.
					ss_BufferSize = static_cast<short>(sizeof(JSVS_SetScenarioIDAnsMsg)-sizeof(uint32_t));
					// [changelog] 2018/01/15 Cuong #245400 Fix static check for IT5_4.2 F00-U2(07) end.
				}
				return ss_BufferSize;
			}
			break;
		default:
			return False;
	}

}
//Notice通知用.
void JSVC_ResponseListOperation::setNoticeData( uint32_t ul_JobResourceID_, uint32_t ul_State_, uint32_t ul_ScenarioID_, uint32_t ul_SubScenarioID_ )
{
	ul_JobResourceID	= ul_JobResourceID_;
	ul_State			= ul_State_;
	ul_ScenarioID		= ul_ScenarioID_;
	ul_SubScenarioID[0]	= ul_SubScenarioID_;
}

JSVC_ResponseJobResourceWarning::JSVC_ResponseJobResourceWarning(uint32_t ul_MsgTypeID)
: JSVC_Response(ul_MsgTypeID)
{
	ul_JobResourceID = 0;
// OP_BTS-3176対応 警告領域枯渇のため領域拡張8(32byte)->16(64byte) 変数名のsizeofへ変更 2019/03/07 soma.
	memset( ul_JobWarning, 0x00, sizeof(ul_JobWarning));
}
JSVC_ResponseJobResourceWarning::~JSVC_ResponseJobResourceWarning()
{
}
JSVE_Request_Class_Type JSVC_ResponseJobResourceWarning::WhoAmI()
{
	return JSVE_Request_Class_Type_Res_JobResourceWarning ;
}
Bool JSVC_ResponseJobResourceWarning::setData(const char* pRcvData,short ss_size)
{
	switch( getMsgTypeId() ){
		case JSVE_JobManager_MsgID_JobResourceWarningDataChanged:
			if( NULL != pRcvData ) {
				JSVS_JobResourceWarningNoticeMsg* pData = (JSVS_JobResourceWarningNoticeMsg*)pRcvData;

				this->ul_JobResourceID = pData->ul_JobResourceID ;
				uint32_t ul_Index ;
// OP_BTS-3176対応 警告領域枯渇のため領域拡張8(32byte)->16(64byte) ついでに不要printf文削除 2019/03/07 soma.
				for ( ul_Index = 0 ; ul_Index < 16 ; ul_Index++ ) {
					// 現状固定配列
					this->ul_JobWarning[ul_Index] = pData->ul_JobWarning[ul_Index] ;
				}
			} else {
				return False;
			}
			break;
		case JSVE_JobManager_MsgID_SyncJobResourceWarningInfoAccept:
			if( NULL != pRcvData ) {
				JSVS_SyncJobResourceWarningInfoAnsMsg* pData = (JSVS_SyncJobResourceWarningInfoAnsMsg*)pRcvData;

				this->ul_JobResourceID = pData->ul_JobResourceID ;
				uint32_t ul_Index ;
// OP_BTS-3176対応 警告領域枯渇のため領域拡張8(32byte)->16(64byte) 2019/03/07 soma.
				for ( ul_Index = 0 ; ul_Index < 16 ; ul_Index++ ) {
					// 現状固定配列
					this->ul_JobWarning[ul_Index] = pData->ul_JobWarning[ul_Index] ;
				}
			} else {
				return False;
			}
			break;
		default:
			return False;
	}
	return True;
}
short JSVC_ResponseJobResourceWarning::getSndDataSize()
{
	switch( getMsgTypeId() ){
		case JSVE_JobManager_MsgID_JobResourceWarningDataChanged:
#if JSVC_Request_ResourceWarningDBG
			printf("★★★★★JSVC_ResponseJobResourceWarning::getSndDataSize\n");
#endif
			return sizeof(JSVS_JobResourceWarningNoticeMsg);
			break;
		case JSVE_JobManager_MsgID_SyncJobResourceWarningInfoAccept:
			return sizeof(JSVS_SyncJobResourceWarningInfoAnsMsg);
			break;
		default:
#if JSVC_Request_ResourceWarningDBG
			printf("★★★★★JSVC_ResponseJobResourceWarning::getSndDataSize False\n");
#endif
			return False;
	}
}
short JSVC_ResponseJobResourceWarning::MakeDataStream(JSVC_RequestBase* pc_Req, char* pSndData)
{
	switch( getMsgTypeId() ){
		case JSVE_JobManager_MsgID_JobResourceWarningDataChanged:
			{
				JSVS_JobResourceWarningNoticeMsg* p_msg = (JSVS_JobResourceWarningNoticeMsg*)pSndData;

				p_msg->ul_JobResourceID		= ul_JobResourceID;
				uint32_t ul_Index ;
// OP_BTS-3176対応 警告領域枯渇のため領域拡張8(32byte)->16(64byte) 2019/03/07 soma.
				for ( ul_Index = 0 ; ul_Index < 16 ; ul_Index++ ) {
					p_msg->ul_JobWarning[ul_Index]	= this->ul_JobWarning[ul_Index];
				}
#if JSVC_Request_ResourceWarningDBG
				printf("★★★★★JSVC_ResponseJobResourceWarning::MakeDataStream\n");
#endif
				return sizeof(JSVS_JobResourceWarningNoticeMsg);
			}
			break;
		case JSVE_JobManager_MsgID_SyncJobResourceWarningInfoAccept:
			{
				JSVS_SyncJobResourceWarningInfoAnsMsg* p_msg = (JSVS_SyncJobResourceWarningInfoAnsMsg*)pSndData;

				p_msg->e_JobServiceResult		= getResult();
				p_msg->e_JobServiceResultDetail	= getResultDetail();

				p_msg->ul_JobResourceID		= ul_JobResourceID;
				uint32_t ul_Index ;
// OP_BTS-3176対応 警告領域枯渇のため領域拡張8(32byte)->16(64byte) 2019/03/07 soma.
				for ( ul_Index = 0 ; ul_Index < 16 ; ul_Index++ ) {
					p_msg->ul_JobWarning[ul_Index]	= this->ul_JobWarning[ul_Index];
				}
				return sizeof(JSVS_SyncJobResourceWarningInfoAnsMsg);
			}
			break;
		default:
#if JSVC_Request_ResourceWarningDBG
				printf("★★★★★JSVC_ResponseJobResourceWarning::MakeDataStream False\n");
#endif
			return False;
	}
}

JSVC_ParentRequest::~JSVC_ParentRequest()
{
	if (NULL != JSVC_ParentRequest::pc_JobSpec){
		delete JSVC_ParentRequest::pc_JobSpec;
		JSVC_ParentRequest::pc_JobSpec = NULL;
	}
}
void JSVC_ParentRequest::setJobSpec(JSVC_JobSpec* pc_JobSpec)
{
	if ((NULL != pc_JobSpec) &&
		(NULL != JSVC_ParentRequest::pc_JobSpec)){
		JSVC_ParentRequest::pc_JobSpec->deSerialize_JobSpec(pc_JobSpec->getJobSpec());
	}
}
JSVC_JobSpec* JSVC_ParentRequest::getJobSpec()
{
	return JSVC_ParentRequest::pc_JobSpec;
}
void JSVC_ParentRequest::newJobSpec()
{
	JSVC_ParentRequest::pc_JobSpec = new JSVC_JobSpec;
}
void JSVC_ParentRequest::delJobSpec()
{
	if (NULL != pc_JobSpec){
		delete pc_JobSpec;
		JSVC_ParentRequest::pc_JobSpec = NULL;
	}
}

// 原稿束削除中状態を設定する.
void JSVC_ParentRequest::setDocDeleting(Bool bDeleting)
{
	m_bDocDeleting = bDeleting;
}

// 原稿束削除中状態を取得する.
Bool JSVC_ParentRequest::getDocDeleting()
{
	return m_bDocDeleting;
}

void JSVC_ParentRequest::resetFunctionStatus()
{
	for ( int i = 0; i < JSVE_FunctionNumber_Max; i++ ){
		ul_FunctionStatusTable[i] = JSVE_FunctionStatus_Non;
	}
}

// 2012/05/24 CQ364929対応 start.
// 復活のためシナリオを終了しないようにする必要があるか判断する.
Bool JSVC_ParentRequest::isWaitRevivalJob()
{
	Bool				b_ret				= False;
	Bool				b_WaitRevival		= this->getWaitRevivalJob();

	if (True == b_WaitRevival) {
		b_ret = True;
	} else {
		b_ret = False;
	}


	return b_ret;
}
// 2012/05/24 CQ364929対応 end.
