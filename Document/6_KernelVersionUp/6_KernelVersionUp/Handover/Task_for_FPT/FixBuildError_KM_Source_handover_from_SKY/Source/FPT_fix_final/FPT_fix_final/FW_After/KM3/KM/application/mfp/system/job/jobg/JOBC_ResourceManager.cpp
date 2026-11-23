//
//
//ファイル名：JOBC_ResourceManager.cpp
//
//概要：リソース管理クラス
//

//*****************************************
// #include
//*****************************************
#include	<string.h>
#include	"JOBC_ResourceManager.h"
#include    "JOBC_ResourceManagerObserver.h"
#include	"JOBC_ResourceConcretePrinterObserber.h"
#include	"JOBC_ResourceConcreteScannerObserber.h"
#include	"JOBC_ResourceConcreteImageObserver.h"
#include "system/nvd/APIC_BoxSetting.h"
#include "system/doc/DOCC_DocIf.h"
#include	"JOBC_StateTableCommon.h"

#include "../../shlib/LogLib/LIBC_LogLib.h"

#include "JOBC_Job.h"
// 2017/11/22 AR1029174 SegF時新詳細コード対応 s.uekama start.
#include "system/sys/APIC_FatalSignalDetailCode.h"
// 2017/11/22 AR1029174 SegF時新詳細コード対応 s.uekama end.

// [changelog] 2017/12/20 AR1029174 khueqt Supported SegF NewDetailCode for JOBC_RMState IT5_4.2 start.
static uchar getBoxDetailCode(JOBC_Job* pJob);
static uchar uc_boxDetailCode = 0;
// [changelog] 2017/12/20 AR1029174 khueqt Supported SegF NewDetailCode for JOBC_RMState IT5_4.2 end.

extern "C" {
extern void DBGF_PrintNoOperationPath(char* psc_FileName, int32_t sl_LineNo); 
}

static const int32_t JOBD_OVERLAY_RESTORE_MAX =	3;

//*****************************************
// #DBG-START
//*****************************************

#ifdef TORNADO // 実機にて有効
#define RMLOG(p0,p1,p2,p3,p4,p5)   SYSF_logMsg(p0,p1,p2,p3,p4,p5,6);
#else
#define RMLOG(p0,p1,p2,p3,p4,p5)   TRACE(p0,p1,p2,p3,p4);
#endif  // #ifdef TORNADO

static const int32_t DEF_RMLOG_MAX =	50;

struct RMLOG
{
	int32_t						Tick;		// 時間
	int32_t						State;		// 状態
	int32_t						NextState;	// 	次状態
	int32_t						Event;		// イベント
	int32_t						JobID;		// Job番号
	int32_t						result;		// 結果
};

//  0:表示なし
//  1:Loop_Main()+changeState()関数
//  2:ResourceManager終了時
//  3:各State関数
static const int32_t RM_ErrLog =	0;
static const int32_t RM_Main =		1;
static const int32_t RM_End =		2;
static const int32_t RM_State =	3;

static	uchar	RMDBG=RM_ErrLog;


static	RMLOG	s_RMLOG[DEF_RMLOG_MAX];
static	short	rmlog_key=0;

extern "C" 
{
	void setRMDBG( uchar RmDbg );
	void getRMLOG();
	void delRMLOG();
	void getResourceManagerLog();//SysLogALL用
}

// 概要：ログレベル設定
void setRMDBG( uchar RmDbg )
{
    RMDBG = RmDbg;
}

struct JOBS_ResourceManagerDebugLog {
	int32_t number;
	const char* message;
};
// StateLog
const JOBS_ResourceManagerDebugLog s_StateLog[] = {
	{JOBE_ResourceManagerStateUnknown,							"Unknown"},
	{JOBE_ResourceManagerStateDriving,							"Driving"},
	{JOBE_ResourceManagerStateWaitingForRetryTimer,				"WaitingForRetryTimer"},
	{JOBE_ResourceManagerStateWaitingDocumentCopyEnd,			"WaitingDocumentCopyEnd"},
	{JOBE_ResourceManagerStateWaitingDocRestore,				"WaitingDocRestore"},
	{JOBE_ResourceManagerStateCancellingDocRestore,				"CancellingDocRestore"},
	{JOBE_ResourceManagerStateWaitingDeserializeEnd,			"WaitingDeserializeEnd"},
	{JOBE_ResourceManagerStateWaitingGetInstanceEnd,			"WaitingGetInstanceEnd"},
	{JOBE_ResourceManagerStateCancellingWaitForDocDelete,		"CancellingWaitForDocDelete"}
};
// EventLog
const JOBS_ResourceManagerDebugLog s_EventLog[] = {
	{JOBD_MSG_BOXJOB_NON,		"None"},
	{JOBD_MSG_BOXJOB_START,		"startBoxJobReserve"},
	{JOBD_MSG_BOXJOB_CANCEL,	"cancelBoxJobReserve"},
	{JOBD_MSG_BOXJOB_TIMER,		"TimerExpired"},
	{JOBD_MSG_BOXJOB_ANS_DSZ,	"ans_deserializeDoc"},
	{JOBD_MSG_BOXJOB_ANS_CDP,	"ans_changeDocFilePosition"},
	{JOBD_MSG_BOXJOB_ANS_GIT,	"ans_getInstance"}
};

// 概要：ログメッセージ設定
void setRMLOG(
	int32_t		State,
	int32_t		NextState,
	int32_t		event,
	JOBC_Job*	pJob,
	int32_t		result)
{
	if( JOBD_MSG_BOXJOB_TIMER == event ){
		// JOBD_MSG_BOXJOB_TIMER → JOBD_MSG_BOXJOB_TIMER の場合残さない
		short oldkey = rmlog_key;
		if( 0 == oldkey ) oldkey = DEF_RMLOG_MAX;
		oldkey--;
		if( JOBD_MSG_BOXJOB_TIMER == s_RMLOG[oldkey].Event ) return;
	}
	s_RMLOG[rmlog_key].Tick		= SYSF_tickGet();
	s_RMLOG[rmlog_key].State	= State;
	s_RMLOG[rmlog_key].NextState= NextState;
	s_RMLOG[rmlog_key].Event	= event;

	int32_t jobid=0;
	if( NULL!=pJob){
		jobid = pJob->getJobId();
	}
	s_RMLOG[rmlog_key].JobID	= jobid;
	s_RMLOG[rmlog_key].result	= result;

	LIBS_ResourceManagerLog s_ResourceManagerLog = {};
	memset(&s_ResourceManagerLog, 0x00, sizeof(s_ResourceManagerLog));

	// ログ書き込み用構造体にデータ格納.
	s_ResourceManagerLog.sl_Tick		= s_RMLOG[rmlog_key].Tick;
	s_ResourceManagerLog.sl_State 		= s_RMLOG[rmlog_key].State;
	s_ResourceManagerLog.sl_NextState 	= s_RMLOG[rmlog_key].NextState;
	s_ResourceManagerLog.sl_Event 		= s_RMLOG[rmlog_key].Event;
	s_ResourceManagerLog.sl_JobID 		= s_RMLOG[rmlog_key].JobID;
	s_ResourceManagerLog.sl_result 		= s_RMLOG[rmlog_key].result;
	s_ResourceManagerLog.uc_statemsg	= static_cast<uchar>(State);
	s_ResourceManagerLog.uc_nextstatemsg= static_cast<uchar>(NextState);
	s_ResourceManagerLog.uc_eventmsg	= static_cast<uchar>(event);

	// 書き込み用カテゴリー指定.
	short ss_LogCategory = 0;
	ss_LogCategory |= LIBD_LogCategorySetting_Copy;			// コピー.
	ss_LogCategory |= LIBD_LogCategorySetting_Print;		// プリント.
	ss_LogCategory |= LIBD_LogCategorySetting_Box;			// ボックス.
	ss_LogCategory |= LIBD_LogCategorySetting_Scanner;		// Scan送信.
	ss_LogCategory |= LIBD_LogCategorySetting_PSWC;			// PSWC.
	ss_LogCategory |= LIBD_LogCategorySetting_Fax;			// Fax.
	ss_LogCategory |= LIBD_LogCategorySetting_NetFax;		// NetFax.

	// インデックス指定.
	short ss_Index = (-1);		// 次回インデックス位置指定.

	// LogLibAPIをコール.
	LIBF_WriteSystemLog(LIBD_LogID_ResourceManagerLog, LIBD_LogOutPutLevel1, ss_LogCategory, &s_ResourceManagerLog, &ss_Index);

	rmlog_key++;
	rmlog_key %= DEF_RMLOG_MAX;
}

// 概要：ログメッセージ取得
void getRMLOG()
{
	short j = rmlog_key;
	RMLOG("★ JOBC_ResourceManagerState-LogStart ★\n",0,0,0,0,0);
	for(short num=0;num<DEF_RMLOG_MAX;num++)
	{
		if(JOBD_MSG_BOXJOB_NON != s_RMLOG[j].Event){
			RMLOG(" tick:%ld JobID:%ld state:%s event:%s result:%ld \n",
				s_RMLOG[j].Tick,
				s_RMLOG[j].JobID,
				s_StateLog[s_RMLOG[j].State].message,
				s_EventLog[s_RMLOG[j].Event].message,
				s_RMLOG[j].result);
		}
		else if(JOBE_ResourceManagerStateUnknown != s_RMLOG[j].NextState){
			RMLOG(" tick:%ld JobID:%ld state:%s → %s \n",
				s_RMLOG[j].Tick,
				s_RMLOG[j].JobID,
				s_StateLog[s_RMLOG[j].State].message,
				s_StateLog[s_RMLOG[j].NextState].message,0);
		}
		else if(DOCD_RetOk != s_RMLOG[j].result){
			RMLOG(" ★ tick:%ld JobID:%ld state:%s Doc->result:%ld \n",
				s_RMLOG[j].Tick,
				s_RMLOG[j].JobID,
				s_StateLog[s_RMLOG[j].State].message,
				s_RMLOG[j].result,0);
		}
		else {
			// no operation.
			DBGF_PrintNoOperationPath(__FILE__, __LINE__);
		}
		j++;
		j %= DEF_RMLOG_MAX;
	}
	RMLOG("★ JOBC_ResourceManagerState-LogEnd   ★\n",0,0,0,0,0);
}
// 概要：ログメッセージ出力
void dspRMLOG(
	JOBC_ResourceManagerState*	pState,
	JOBS_RMStateMsgBuff*		rMsg)
{
	int32_t jobid=0;
	if( NULL!=rMsg->pJob ){
		jobid = rMsg->pJob->getJobId();
	}
	RMLOG(static_cast<char*>("★ JOBC_RM::Loop_Main() tickGet=%ld JobID:%ld state:%s event:%s result:%ld ★\n"),
		SYSF_tickGet(),
		jobid,
		s_StateLog[static_cast<int>(pState->who())].message,
		s_EventLog[static_cast<int>(rMsg->sl_EventID)].message,
		rMsg->ret);
}
void dspRMLOG2(
	JOBC_Job*					pJob,
	JOBC_ResourceManagerState*	pState,
	JOBC_ResourceManagerState*	pNextState)
{
	int32_t jobid=0;
	if( NULL!=pJob ){
		jobid = pJob->getJobId();
	}
	RMLOG("★ JOBC_RM::changeState() tickGet=%ld JobID:%ld state:%s → %s ★\n",
		SYSF_tickGet(),
		jobid,
		s_StateLog[static_cast<int>(pState->who())].message,
		s_StateLog[static_cast<int>(pNextState->who())].message,0);
}
// 概要：ログメッセージ初期化
void delRMLOG()
{
	for(short num=0;num<DEF_RMLOG_MAX;num++)
	{
		s_RMLOG[num].Tick		= 0;
		s_RMLOG[num].State		= JOBE_ResourceManagerStateUnknown;
		s_RMLOG[num].NextState	= JOBE_ResourceManagerStateUnknown;
		s_RMLOG[num].Event		= JOBD_MSG_BOXJOB_NON;
		s_RMLOG[num].JobID		= 0;
		s_RMLOG[num].result		= DOCD_RetOk;
	}
	rmlog_key=0;
}

// 概要：ログメッセージ取得(SysLogALL用)
void getResourceManagerLog()
{
	short j = rmlog_key;
	printf("★ JOBC_ResourceManagerState-LogStart ★\n");
	for(short num=0;num<DEF_RMLOG_MAX;num++)
	{
		if(JOBD_MSG_BOXJOB_NON != s_RMLOG[j].Event){
			printf("(%u) JobID:%ld state:%s event:%s result:%ld \n",
				static_cast<uint32_t>(s_RMLOG[j].Tick),
				s_RMLOG[j].JobID,
				s_StateLog[s_RMLOG[j].State].message,
				s_EventLog[s_RMLOG[j].Event].message,
				s_RMLOG[j].result);
		}
		else if(JOBE_ResourceManagerStateUnknown != s_RMLOG[j].NextState){
			printf("(%u) JobID:%ld state:%s → %s \n",
				static_cast<uint32_t>(s_RMLOG[j].Tick),
				s_RMLOG[j].JobID,
				s_StateLog[s_RMLOG[j].State].message,
				s_StateLog[s_RMLOG[j].NextState].message);
		}
		else if(DOCD_RetOk != s_RMLOG[j].result){
			printf("(%u)★ JobID:%ld state:%s Doc->result:%ld \n",
				static_cast<uint32_t>(s_RMLOG[j].Tick),
				s_RMLOG[j].JobID,
				s_StateLog[s_RMLOG[j].State].message,
				s_RMLOG[j].result);
		}
		else {
			// no operation.
			DBGF_PrintNoOperationPath(__FILE__, __LINE__);
		}
		j++;
		j %= DEF_RMLOG_MAX;
	}
	printf("★ JOBC_ResourceManagerState-LogEnd   ★\n");
}
void _getResourceManagerLog(void)
{
	getResourceManagerLog();
}

//*****************************************
// #DBG-END
//*****************************************


// 実体定義
//概要	  ：自インスタンス
//備考	  ：
JOBC_ResourceManager* JOBC_ResourceManager::_pResourceManager;
//概要	  ：QueID
//備考	  ：
MSG_Q_ID  JOBC_ResourceManager::_QueId;
MSG_Q_ID  JOBC_ResourceManager::RMState_QueId;
//概要	  ：コンクリエイトオブザーバーアドレス
//備考	  ：
// プリンタシステム構成
APIC_PrinterSystem*		JOBC_ResourceManager::_pPrinterSystem;
// プリンタ警告
APIC_PrinterWarning*	JOBC_ResourceManager::_pPrinterWarning;
// プリンタ状態
APIC_PrinterStatus*		JOBC_ResourceManager::_pPrinterStatus;
// 給紙トレイ情報
APIC_FeedTray*			JOBC_ResourceManager::_pFeedTray;
// 排紙トレイ情報
APIC_OutputTray*		JOBC_ResourceManager::_pOutputTray;
// プリンタ情報
APIC_PrinterInfo*		JOBC_ResourceManager::_pPrinterInfo;
// スキャンデバイス
APIC_ScannerSystem*		JOBC_ResourceManager::_pScannerSystem;
// スキャナー警告情報
APIC_ScannerWarning*	JOBC_ResourceManager::_pScannerWarning;
// スキャナー状態情報
APIC_ScannerStatus*		JOBC_ResourceManager::_pScannerStatus;
// スキャナー原稿情報
APIC_Original*			JOBC_ResourceManager::_pOriginal;
//概要	  ：デバイス状態
//備考	  ：
JOBS_DeviceStatusInfo	JOBC_ResourceManager::_DeviceStatus = {};
// イメージメモリ
IMGC_ImageMemoryPool*   JOBC_ResourceManager::_pImage;
// 2017/11/22 AR1029174 SegF時新詳細コード対応 s.uekama start.
// メッセージ種別.
uchar JOBC_ResourceManager::uc_msgTypeLocal=0;
// 2017/11/22 AR1029174 SegF時新詳細コード対応 s.uekama end.

//*****************************************
// public
//*****************************************
//
//概要	  ：コンストラクタ
//作成者  ：hayamori
//レビュー：未実施（レビュー実施日と名前を記述）
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
//
JOBC_ResourceManager::JOBC_ResourceManager()
{
	// メンバー初期化
	// 自インスタンス格納エリア
	_pResourceManager = NULL;
	// QueID
	memset( &_QueId, 0, sizeof(MSG_Q_ID));
	// デバイス状態
	memset( &_DeviceStatus, 0, sizeof(JOBS_DeviceStatusInfo));
	// FAX送信フラグ
	FaxSendFlag = FALSE;

	// ResourceManagerStateインスタンスへのポインタ
	changeState(JOBC_ResourceManagerStateDriving::newInstance());
	// Leafインスタンス空き待ちタイマー
	mTimerForRestoreWait = new JOBC_TimerObserverForResourceManager(JOBD_MSG_BOXJOB_TIMER,5000);
	// 処理中ジョブ情報
	clearRestoringJobInfo();
	//概要    ：予約ジョブリスト
	mReserveList = JOBC_ReserveBoxJobList::newInstance();

	return;
}


//
//概要	  ：デストラクタ
//作成者  ：hayamori
//レビュー：未実施（レビュー実施日と名前を記述）
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
//
JOBC_ResourceManager::~JOBC_ResourceManager()
{
	// 処理なし
	return;
}


//
//概要	  ：インスタンス取得 
//作成者  ：hayamori
//レビュー：未実施（レビュー実施日と名前を記述）
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
//
JOBC_ResourceManager* 
JOBC_ResourceManager::newInstance()
{
	return	JOBC_ResourceManager::_pResourceManager;
}


//
//概要	  ：初期設定取得 
//作成者  ：hayamori
//レビュー：未実施（レビュー実施日と名前を記述）
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
//
void
JOBC_ResourceManager::initClass()
{
	THR_ID retcode = 0;

	// インスタンス生成
	JOBC_ResourceManager::_pResourceManager = new JOBC_ResourceManager;

	// メッセージQueue作成
	JOBC_ResourceManager::_QueId = 
			SYSF_msgQCreate( JOBD_ResouceManegerGetQueNum,		// max messages that can be queued 
							 JOBD_ResouceManegerMsgLength,		// max bytes in a message 
							 MSG_Q_FIFO);						// message queue options 
	if( JOBC_ResourceManager::_QueId == NULL ){
		// メッセージQue生成失敗
		APIC_setFatalDetailCodeE002(CE002_DetailCode0x50F7);
		APIC_ThrowSystemError(SYSE_SysErrTypeFatalLogicalError,
							  LOGF_DEBUG_EVENT_KIND_JOB,
							  const_cast<const char *>("JOBC_ResourceManager::initial SYSF_msgQCreate Error!!"));
		return;
	}

	// 各オブザーバーの初期設定
	initObserver();

	// タスクの生成起動
	retcode = SYSF_taskSpawn( static_cast<char *>("JOBC_ResourceManager"),
					100,
					VX_FP_TASK,
					5120,
					reinterpret_cast<FUNCPTR>(JOBC_ResourceManager::ResourceManager_InitTask),
					0,0,0,0,0,0,0,0,0,0
					);
	if(retcode == ERROR){
		// 致命的システムエラー
		APIC_setFatalDetailCodeE002(CE002_DetailCode0x50F8);
		APIC_ThrowSystemError(SYSE_SysErrTypeFatalLogicalError,
							  LOGF_DEBUG_EVENT_KIND_JOB,
							  const_cast<const char *>("JOBC_ResourceManager::initClass SYSF_taskSpawn Error!!"));
	}

	// メッセージQueue作成(ReserveBoxJob用)
	JOBC_ResourceManager::RMState_QueId= 
			SYSF_msgQCreate( JOBD_RMStateGetQueNum,		// max messages that can be queued 
							 JOBD_RMStateMsgLength,		// max bytes in a message 
							 MSG_Q_FIFO);				// message queue options 
	if( JOBC_ResourceManager::RMState_QueId == NULL ){
		// 致命的システムエラー
		APIC_setFatalDetailCodeE002(CE002_DetailCode0x50F9);
		APIC_ThrowSystemError(SYSE_SysErrTypeSystemExhausted,
			LOGF_DEBUG_EVENT_KIND_JOB,
			const_cast<const char *>("JOBC_ResourceManager::initClass MsgQue作成失敗"));
	}
	// タスクの生成起動
	retcode = SYSF_taskSpawn( static_cast<char *>("JOBC_RMState"),
					100,VX_FP_TASK,25600,								// №65166対応 5120 ⇒ 7680 ⇒ 25600へ変更 Sky_iwasaki 
					reinterpret_cast<FUNCPTR>(JOBC_ResourceManager::RMState_InitTask),
					0,0,0,0,0,0,0,0,0,0
					);
	if(retcode == ERROR){
		// 致命的システムエラー
		APIC_setFatalDetailCodeE002(CE002_DetailCode0x50FA);
		APIC_ThrowSystemError(SYSE_SysErrTypeSystemExhausted,
			LOGF_DEBUG_EVENT_KIND_JOB,
			const_cast<const char *>("JOBC_ResourceManager::initClass task作成失敗"));
	}

	// ログメッセージ初期化
	delRMLOG();

	return;
}

//
//概要	  ：実行可能状態チェック 
//作成者  ：sakuragi
//レビュー：未実施（レビュー実施日と名前を記述）
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
//
Bool
JOBC_ResourceManager::isAvailable( JOBC_Job* pJob )
{
	Bool warning = False;
	// パラメータチェック
	if ( !( pJob != NULL ) ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" ); }
	
	for(int cnt=static_cast<int>(ATTENTION_CTonerNearEmpty);cnt<=static_cast<int>(ERROR_SpecificOriginalDetect);cnt++)
	{
		warning = _pPrinterWarning->isWarning(static_cast<TYPE_PrinterWarning>(cnt));
		if( True == warning ){
			return warning;
		}
	}
    warning = isAvailable_21_1_1(); //機種固有処理
	return warning;
}
//
//概要	  ：オブザーバ登録 
//作成者  ：hayamori
//レビュー：未実施（レビュー実施日と名前を記述）
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
//
void
JOBC_ResourceManager::attachObserver( JOBC_ResourceManagerObserver* pc_ResourceManagerObserver )
{
	if ( !( pc_ResourceManagerObserver != NULL ) ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" ); }
	_pc_List.add( pc_ResourceManagerObserver ); // オブザーバーの追加 

	return;
}


//
//概要	  ：オブザーバ削除
//作成者  ：hayamori
//レビュー：未実施（レビュー実施日と名前を記述）
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
//
void
JOBC_ResourceManager::detachObserver( JOBC_ResourceManagerObserver* pc_ResourceManagerObserver )
{
	if ( !( pc_ResourceManagerObserver != NULL ) ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" ); }
	_pc_List.del( pc_ResourceManagerObserver ); // オブザーバーの追加 

	return;
}
//.
//概要	  ：イメージメモリプール予約.
//作成者  ：sakuragi.
//レビュー：未実施（レビュー実施日と名前を記述）.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
// 2011/03/02 1OSW遠藤(日立JTE) CQ336076:静的解析対応(CID12952).
//.
uint32_t
JOBC_ResourceManager::reserveMemory( JOBC_Job* pc_Job, uint32_t ul_MemorySize )
{	
	if ( !( pc_Job != NULL ) ) {
		APIC_setFatalDetailCodeE002(CE002_DetailCode0x50F5);
		JOBC_Job::setSysErr( TYPD_TroubleCode_E002, __LINE__ );
	}
	// 機種依存部（Color）にてのみ使用
	return pc_Job->getImageMemoryPool()->reserve( 
				pc_Job->getJobStatus()->getJobID(),
				ul_MemorySize,
				pc_Job->getJobSpec()->pc_Common->c_JobType.getJobType()
				);
}
//.
//概要	  ：イメージメモリプール取得.
//作成者  ：sakuragi.
//レビュー：未実施（レビュー実施日と名前を記述）.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
// 2011/03/02 1OSW遠藤(日立JTE) CQ336076:静的解析対応(CID12951).
//.
uint32_t
JOBC_ResourceManager::allocMemory( JOBC_Job* pc_Job, uint32_t ul_MemorySize, IMGC_Image* pImage)
{	
	if ( !( pc_Job != NULL ) ) {
		APIC_setFatalDetailCodeE002(CE002_DetailCode0x50F6);
		JOBC_Job::setSysErr( TYPD_TroubleCode_E002, __LINE__ );
	}
	if ( !( pImage != NULL ) ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" ); }
	return pc_Job->getImageMemoryPool()->allocMemory( 
				pc_Job->getJobStatus()->getJobID(),
				pc_Job->getJobSpec()->pc_Common->c_JobType.getJobType(),
				ul_MemorySize,
				pImage,
				True
				);
}
//
//概要	  ：FAX送信フラグ設定
//作成者  ：sakuragi
//レビュー：未実施（レビュー実施日と名前を記述）
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
//
void
JOBC_ResourceManager::setFaxSendFlag( Bool _FaxSendFlag )
{
	FaxSendFlag = _FaxSendFlag ;
	
	return ;
}
//
//概要	  ：FAX送信フラグ収集
//作成者  ：sakuragi
//レビュー：未実施（レビュー実施日と名前を記述）
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
//
Bool
JOBC_ResourceManager::getFaxSendFlag()
{	
	return FaxSendFlag;
}
//
//概要	  ：ポーリング送信予約チェック
//作成者  ：sakuragi
//レビュー：未実施（レビュー実施日と名前を記述）
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
//
Bool
JOBC_ResourceManager::checkPollingSendReserve()
{	
	Bool ret = False;
	DOCC_BoxIf *c_pBox = DOCC_BoxIf::newInstance(DOCD_PollingSndBox);
	if(NULL == c_pBox)
	{
		return False;
	}
	if(c_pBox->getExistBox() == DOCD_Exist)
	{
		if(c_pBox->getBoxDocCnt() > 0)
		{
			ret = True;
		}
		else
		{
			ret = False;
		}
	}
	else
	{
		ret = False;
	}
	if(NULL != c_pBox){
		c_pBox->deleteInstance();
	}
	return ret;
}
//
//概要	  ：掲示板送信予約チェック
//作成者  ：sakuragi
//レビュー：未実施（レビュー実施日と名前を記述）
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
//
Bool
JOBC_ResourceManager::checkBuBoardSendReserve()
{
	APIC_BbBoxSetting *c_pBbBoxSetting = APIC_BbBoxSetting::newInstance();
	if(NULL == c_pBbBoxSetting)
	{
		return False;
	}

	LIST c_List = {};
	int32_t sl_boxRet = c_pBbBoxSetting->getAllBox(&c_List);
	if( 0>sl_boxRet ){
		return False;
	}
	Bool ret = False;
	SYSS_RecordListHeader *pRec = NULL, *pTmp = NULL;
	char *pData = NULL;
	pRec = reinterpret_cast<SYSS_RecordListHeader *>(SYSF_lstFirst(&c_List));
	while( NULL != pRec){
		pTmp = pRec;
		pData = reinterpret_cast<char *>(++pTmp);
		TYPS_BbBoxInfo *s_pBbBoxInfo = reinterpret_cast<TYPS_BbBoxInfo *>(pData);
		if(NULL == s_pBbBoxInfo){
			return False;
		}
		DOCC_BoxIf *c_pBox = DOCC_BoxIf::newInstance(s_pBbBoxInfo->sl_BoxNo);
		if(NULL == c_pBox)
		{
			return False;
		}
		if(c_pBox->getExistBox() == DOCD_Exist)
		{
			if(c_pBox->getBoxDocCnt() > 0)
			{
				ret = True;
				if(NULL != c_pBox){
					c_pBox->deleteInstance();
				}
				break;
			}
		}
		if(NULL != c_pBox){
			c_pBox->deleteInstance();
		}

		pRec = reinterpret_cast<SYSS_RecordListHeader *>(SYSF_lstNext(&pRec->Node));

	}
	c_pBbBoxSetting->deleteList(&c_List);

	return ret;
}

//
//概要	  ：ポーリング送信予約・掲示板チェック
//作成者  ：sakuragi
//レビュー：未実施（レビュー実施日と名前を記述）
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
Bool
JOBC_ResourceManager::checkBoxReserve_phaseB(int32_t BoxNo)
{	
	Bool	rtn=False;
	ushort ins_num = 0;
	JOBC_Job* pJob = NULL ;

	// インスタンス生成
	DOCC_BoxIf *c_pBox = DOCC_BoxIf::newInstance(BoxNo);
	if(NULL == c_pBox)
	{
		return rtn;
	}

	// BOX存在&BOX個数判定
	if( DOCD_Exist == c_pBox->getExistBox() && 0 < c_pBox->getBoxDocCnt() ){
		rtn = True;
		// 使用中のポーリングインスタンス取得
		pJob = JOBC_Job::getPollingSendInstance(ins_num);
		if( 0 != ins_num && NULL != pJob){
			if( DOCD_PollingSndBox == BoxNo ){
				// ポーリング送信判定
#ifdef TORNADO
//				SYSF_logMsg( "JOBC_ResourceManager::phaseB() Polling RxMode(%d)\n",
//								pJob->getJobSpec()->pc_Fax->c_FaxAct.getRxMode(),0,0,0,0,0 );
#endif
				if( TYPE_RxModePolledTx == pJob->getJobSpec()->pc_Fax->c_FaxAct.getRxMode()){
					rtn = False;
				}
			}
			else{
				// 掲示板送信判定
#ifdef TORNADO
//				SYSF_logMsg( "JOBC_ResourceManager::phaseB() keijiban RxMode(%d),BoxID(%d)\n",
//								pJob->getJobSpec()->pc_Fax->c_FaxAct.getRxMode(),
//								pJob->getJobSpec()->pc_Fax->c_FaxAct.getRxMemoryRcvBoxNo(),0,0,0,0 );
#endif
				if( TYPE_RxModeBulletinTx == pJob->getJobSpec()->pc_Fax->c_FaxAct.getRxMode() &&
					BoxNo == pJob->getJobSpec()->pc_Fax->c_FaxAct.getRxMemoryRcvBoxNo()){
					rtn = False;
				}
			}
		}
	}
	// インスタンス削除
	if(NULL != c_pBox){
		c_pBox->deleteInstance();
	}
#ifdef TORNADO
//	SYSF_logMsg( "JOBC_ResourceManager::checkBoxReserve_phaseB() rtn(%d)BoxNo(%d),ins_num(%d)\n",rtn,BoxNo,ins_num,0,0,0 );
#endif
	return rtn;
}

//*****************************************
// private
//*****************************************
//
//概要	  ：各オブザーバーの初期設定
//作成者  ：sakuragi
//レビュー：未実施（レビュー実施日と名前を記述）
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
//
void
JOBC_ResourceManager::initObserver()
{

	// デバイスクラスアドレス収集関数起動
	getDeviceAddress();
	// デバイス状態初期設定関数起動
	setDeviceStatusInit();
	// コンクリエイトデバイスオブザーバ生成,登録関数起動
	setConcreteObserver();
	// イメージクラスアドレス収集関数起動
	getImageAddress();
	// イメージオブザーバ生成,登録関数起動
	setImageObserver();

	return;
}

//
//概要　　：タスク起動関数
//作成者  ：hayamori
//レビュー：未実施（レビュー実施日と名前を記述）
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
//
void
JOBC_ResourceManager::ResourceManager_InitTask()
{
	if(  NULL != _pResourceManager ){
		// [changelog] 2017/11/14 Cuong AR1029174 Supported SegF NewDetailCode for JOBC_ResourceManager IT5_4.2 start.
		APIC_JobNewDetailCode::newInstance(TYPE_TaskDetailCode_JOBC_ResourceManager, True);
		// [changelog] 2017/11/14 Cuong AR1029174 Supported SegF NewDetailCode for JOBC_ResourceManager IT5_4.2 end.
		_pResourceManager -> Task_Main();
	}
	else{
		// 自インスタンス生成失敗してる？
	}
	return;
}
//
//概要　　：タスク起動関数
//作成者  ：hayamori
//レビュー：未実施（レビュー実施日と名前を記述）
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
//
void
JOBC_ResourceManager::RMState_InitTask()
{
	if(  NULL != _pResourceManager ){
// 2017/11/22 AR1029174 SegF時新詳細コード対応 s.uekama start.
		APIC_setFatalCallBack(TYPE_TaskDetailCode_JOBC_RMState, RMStateCallBackFunc, TYPE_FatalCallBackPriority_Owner );
//		APIC_setFatalCallBack(TYPE_TaskDetailCode_JOBC_RMState, RMStateCallBackFunc_Group1, TYPE_FatalCallBackPriority_Group1 );
//		APIC_setFatalCallBack(TYPE_TaskDetailCode_JOBC_RMState, RMStateCallBackFunc_Group2, TYPE_FatalCallBackPriority_Group2 );
// 2017/11/22 AR1029174 SegF時新詳細コード対応 s.uekama end.
		_pResourceManager -> Loop_Main();
	}
	else{
		// システムエラー
		APIC_setFatalDetailCodeE002(CE002_DetailCode0x50FB);
		APIC_ThrowSystemError(SYSE_SysErrTypeFatalLogicalError,
			LOGF_DEBUG_EVENT_KIND_JOB,
			const_cast<const char *>("JOBC_ResourceManager::RMState_InitTask 自インスタンス未生成異常"));
	}
	return;
}


//
//概要　　：初期設定時にタスク生成起動され,メッセージを受信し各機能処理を実行する。
//作成者  ：hayamori
//レビュー：未実施（レビュー実施日と名前を記述）
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
//
void
JOBC_ResourceManager::Task_Main()
{
	int  result = 0;
	char recieveBuffer[160];
	APIC_JobNewDetailCode *pc_Instance = NULL;		// [changelog] 2017/11/14 AR1029174 Cuong Supported SegF NewDetailCode IT5_4.2.
	//無限ループ
	while(TRUE) 
	{
		// receive message
		result = SYSF_msgQReceive( 
					_QueId, 						// message queue from which to receive 
					&recieveBuffer[0],				// buffer to receive message 
					JOBD_ResouceManegerMsgLength,	// length of buffer 
					-1 );							// ticks to wait 
		// [changelog] 2017/11/14 AR1029174 Cuong Supported SegF NewDetailCode IT5_4.2 start.
		pc_Instance = APIC_JobNewDetailCode::getInstance( TYPE_TaskDetailCode_JOBC_ResourceManager );
		if (pc_Instance == NULL)
		{
			return;
		}
		// [changelog] 2017/11/14 AR1029174 Cuong Supported SegF NewDetailCode IT5_4.2 end.
		if(result == ERROR) return;

		// 受信バッファ情報収集
		JOBS_ResouceManegerMsgBuff* pRcevBuffer = reinterpret_cast<JOBS_ResouceManegerMsgBuff*>(recieveBuffer);

		// [changelog] 2017/11/14 AR1029174 Cuong Supported SegF NewDetailCode IT5_4.2 start.
		// Set the NewDetailCode.
		pc_Instance->setNewDetailCode( 12, static_cast<uchar>((pRcevBuffer->sl_notifyCode >> 4 ) & 0xF) );
		pc_Instance->setNewDetailCode( 11, static_cast<uchar>(pRcevBuffer->sl_notifyCode & 0xF) );
		// [changelog] 2017/11/14 AR1029174 Cuong Supported SegF NewDetailCode IT5_4.2 end.
		//メッセージ機能処理実行
		switch( pRcevBuffer -> sl_notifyCode )
		{
			
			case RESOURCEEVENTIDPRINTSYSTEM:		// プリンタシステム構成変更通知
				setPrinterSystem();
				break;
			
			case RESOURCEEVENTIDPRINTWARNING:		// プリンタ警告変更通知
				setPrinterWarning();
				break;
			
			case RESOURCEEVENTIDPRINTSTATUS:		// プリンタ状態変更通知
				setPrinterStatus();
				break;
			
			case RESOURCEEVENTIDPRINTFEEDTRAY:		// 給紙トレイ情報変更通知
				break;
			
			case RESOURCEEVENTIDPRINTOUTPUTTRAY:	// 排紙トレイ情報変更通知
				break;
			
			case RESOURCEEVENTIDPRINTINFO:			// プリンタ情報変更通知
				setPrinterInfo();
				break;
			
			case RESOURCEEVENTIDSCANSYSTEM:			// スキャナーシステム構成変更通知
				setScannerSystem();
				break;
			
			case RESOURCEEVENTIDSCANWARNING:		// スキャナー警告情報
				setScannerWarning();
				break;
			
			case RESOURCEEVENTIDSCANSTATUS:			// スキャナー状態情報
				setScannerStatus();
				break;
			
			case RESOURCEEVENTIDSCANORIGINAL:		// スキャナー原稿情報
				break;
			
			case RESOURCEEVENTIDIMAGE: 				// イメージメモリ変更通知
				break;

			default:								// 例外
				break;
		}

		//実行の結果通知する場合、Observerにより通知する。	
		//notify(result); //Observer通知実行
		// イベント事象を通知
		notifyObserver(pRcevBuffer->sl_notifyCode);
	}
	return;
}


//
//概要	  ：デバイスクラスアドレス収集
//作成者  ：hayamori
//レビュー：未実施（レビュー実施日と名前を記述）
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
//
void
JOBC_ResourceManager::getDeviceAddress()
{
	// プリンタデバイス
	// プリンタシステム
	_pPrinterSystem		= APIC_PrinterSystem::newInstance();
	// プリンタ警告
	_pPrinterWarning	= APIC_PrinterWarning::newInstance();
	// プリンタ状態
	_pPrinterStatus		= APIC_PrinterStatus::newInstance();
	// 給紙トレイ情報
	_pFeedTray			= APIC_FeedTray::newInstance();
	// 排紙トレイ情報
	_pOutputTray		= APIC_OutputTray::newInstance();
	// プリンタ情報
	_pPrinterInfo		= APIC_PrinterInfo::newInstance();

	// スキャンデバイス
	// スキャンシステム
	_pScannerSystem		= APIC_ScannerSystem::newInstance();
	// スキャナー警告情報
	_pScannerWarning	= APIC_ScannerWarning::newInstance();
	// スキャナー状態情報
	_pScannerStatus		= APIC_ScannerStatus::newInstance();
	// スキャナー原稿情報
	_pOriginal			= APIC_Original::newInstance();

	return;
}


//
//概要	  ：デバイス状態初期設定
//作成者  ：hayamori
//レビュー：未実施（レビュー実施日と名前を記述）
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
//
void
JOBC_ResourceManager::setDeviceStatusInit()
{
	// プリンタデバイス状態収集
	// プリンタシステム構成クラス
	setPrinterSystem();
	// プリンタ警告クラス
	setPrinterWarning();
	// プリンタ状態
	setPrinterStatus();
	// プリンタ情報
	setPrinterInfo();

	// スキャナーデバイス情報
	// スキャナーシステム構成
	setScannerSystem();
	// スキャナー警告情報
	setScannerWarning();
	// スキャナー状態情報
	setScannerStatus();

	return;
}


//
//概要	  ：コンクリエイトデバイスオブザーバ生成,登録
//作成者  ：hayamori
//レビュー：未実施（レビュー実施日と名前を記述）
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
//
void
JOBC_ResourceManager::setConcreteObserver()
{
	// プリンタデバイス
	// プリンタシステム
	JOBC_ConcretePrinterSystemObserver* pPrinterSystemObserver = new JOBC_ConcretePrinterSystemObserver( _QueId );
	_pPrinterSystem->attachPrinterSystemObserver( pPrinterSystemObserver );
	// プリンタ警告
	JOBC_ConcretePrinterWarningObserver* pPrinterWarningObserver = new JOBC_ConcretePrinterWarningObserver( _QueId );
	_pPrinterWarning->attachPrinterWarningObserver( pPrinterWarningObserver );
	// プリンタ状態
	JOBC_ConcretePrinterStatusObserver* pPrinterStatusObserver = new JOBC_ConcretePrinterStatusObserver( _QueId );
	_pPrinterStatus->attachPrinterStatusObserver( pPrinterStatusObserver );
	// 給紙トレイ情報
	JOBC_ConcreteFeedTrayObserver* pFeedTrayObserver = new JOBC_ConcreteFeedTrayObserver( _QueId );
	_pFeedTray->attachFeedTrayObserver( pFeedTrayObserver );
	// 排紙トレイ情報
	JOBC_ConcreteOutputTrayObserver* pOutputTrayObserver = new JOBC_ConcreteOutputTrayObserver( _QueId );
	_pOutputTray->attachOutputTrayObserver( pOutputTrayObserver );
	// プリンタ情報
	JOBC_ConcretePrinterInfoObserver* pInfoObserver = new JOBC_ConcretePrinterInfoObserver( _QueId );
	_pPrinterInfo->attachPrinterInfoObserver( pInfoObserver );

	// スキャンデバイス
	// プリンタシステム
	JOBC_ConcreteScannerSystemObserver* pScannerSystemObserver = new JOBC_ConcreteScannerSystemObserver( _QueId );
	_pScannerSystem->attachScannerSystemObserver( pScannerSystemObserver );
	// スキャナー警告情報
	JOBC_ConcreteScannerWarningObserver* pScannerWarningObserver = new JOBC_ConcreteScannerWarningObserver( _QueId );
	_pScannerWarning->attachScannerWarningObserver( pScannerWarningObserver );
	// スキャナー状態情報
	JOBC_ConcreteScannerStatusObserver* pScannerStatusObserver = new JOBC_ConcreteScannerStatusObserver( _QueId );
	_pScannerStatus->attachScannerStatusObserver( pScannerStatusObserver );
	return;
}

//
//概要    ：イメージクラスアドレス収集
//作成者  ：sakuragi
//レビュー：未実施（レビュー実施日と名前を記述）
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
//
void
JOBC_ResourceManager::getImageAddress()
{
	// イメージ
	_pImage = IMGC_ImageMemoryPool::newInstance(IMGE_FileMemory);

	return;
}

//
//概要    ：イメージオブザーバ生成,登録
//作成者  ：sakuragi
//レビュー：未実施（レビュー実施日と名前を記述）
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
//
void
JOBC_ResourceManager::setImageObserver()
{
	// イメージ
	// 構成
	JOBC_ConcreteImageObserver* pImageObserver = new JOBC_ConcreteImageObserver( _QueId );
	_pImage->attachObserver( pImageObserver );

	return;
}

//
//概要	  ：プリンタシステム状態設定
//        ：プリンタシステム構成（ＡＰＩ）APIC_PrinterSystem
//作成者  ：hayamori
//レビュー：未実施（レビュー実施日と名前を記述）
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
//
void
JOBC_ResourceManager::setPrinterSystem()
{
	// 両面ユニット有り 
	_DeviceStatus.PrinterDevice.PrinterSystem.ExistDuplex = _pPrinterSystem -> isExistDuplex();
	// フィニッシャ有り 
	_DeviceStatus.PrinterDevice.PrinterSystem.ExistFinisher = _pPrinterSystem -> isExistFinisher();
	// フィニッシャタイプ
	_DeviceStatus.PrinterDevice.PrinterSystem.FinisherType = _pPrinterSystem -> getFinisherType();
	// ステープル機能.
	_DeviceStatus.PrinterDevice.PrinterSystem.StapleFunction = _pPrinterSystem -> getStapleFunction();
	// パンチ機能.
	_DeviceStatus.PrinterDevice.PrinterSystem.PunchFunction = _pPrinterSystem -> getPunchFunction();
	// 紙折り機能.
	_DeviceStatus.PrinterDevice.PrinterSystem.FoldFunction = _pPrinterSystem -> getFoldFunction();
	// シフト機能.
	_DeviceStatus.PrinterDevice.PrinterSystem.ShiftFunction = _pPrinterSystem -> isShiftFunction();
	// カバーシートフィーダ 
	_DeviceStatus.PrinterDevice.PrinterSystem.CoverSheetFeeder = _pPrinterSystem -> isCoverSheetFeeder();
	// 第３トレイ装着情報	
	_DeviceStatus.PrinterDevice.PrinterSystem.OptionTray3 = _pPrinterSystem -> isOptionTray3();
	// ＬＣＴタイプ 	
	_DeviceStatus.PrinterDevice.PrinterSystem.LCTType = _pPrinterSystem -> getLCTType();

	return;
}


//
//概要	  ：プリンタ警告状態設定
//        ：プリンタ警告（ＡＰＩ）APIC_PrinterWarning
//作成者  ：hayamori
//レビュー：未実施（レビュー実施日と名前を記述）
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
//
void
JOBC_ResourceManager::setPrinterWarning()
{
	// ＦＡＴＡＬコード
	_DeviceStatus.PrinterDevice.PrinterWarning.Fatal = _pPrinterWarning -> getFatal();
	// ＦＡＴＡＬが発生しているか
	_DeviceStatus.PrinterDevice.PrinterWarning.OccurFatal = _pPrinterWarning -> isOccurFatal();
	// 周辺機器コミュニケーションエラーコード
	_DeviceStatus.PrinterDevice.PrinterWarning.CommError = _pPrinterWarning -> getCommError();
	// 周辺機器コミュニケーションエラーが発生しているか
	_DeviceStatus.PrinterDevice.PrinterWarning.OccurCommError = _pPrinterWarning -> isOccurCommError();
	// ＨＤＤファン異常
	_DeviceStatus.PrinterDevice.PrinterWarning.HDDFanTrouble = _pPrinterWarning -> isHDDFanTrouble();
	// ジャムコードの取得
	_DeviceStatus.PrinterDevice.PrinterWarning.PrinterJam = _pPrinterWarning -> getPrinterJam();
	// ジャムが発生しているか
	_DeviceStatus.PrinterDevice.PrinterWarning.OccurPrinterJam = _pPrinterWarning -> isOccurPrinterJam();
	// 何れかのドアが開いているか
	_DeviceStatus.PrinterDevice.PrinterWarning.PrinterDoorOpen = _pPrinterWarning -> isPrinterDoorOpen();
	// Ｙトナー未装着	
	_DeviceStatus.PrinterDevice.PrinterWarning.YellowTonerUnset = _pPrinterWarning -> isYellowTonerUnset();
	// Ｍトナー未装着	
	_DeviceStatus.PrinterDevice.PrinterWarning.MagentaTonerUnset = _pPrinterWarning -> isMagentaTonerUnset();
	// Ｃトナー未装着	
	_DeviceStatus.PrinterDevice.PrinterWarning.CyanTonerUnset = _pPrinterWarning -> isCyanTonerUnset();
	// Ｋトナー未装着	
	_DeviceStatus.PrinterDevice.PrinterWarning.BlackTonerUnset = _pPrinterWarning -> isBlackTonerUnset();
	// Ｙトナー状態
	_DeviceStatus.PrinterDevice.PrinterWarning.YellowTonerStatus = _pPrinterWarning -> getYellowTonerStatus();
	// Ｍトナー状態
	_DeviceStatus.PrinterDevice.PrinterWarning.MagentaTonerStatus = _pPrinterWarning -> getMagentaTonerStatus();
	// Ｃトナー状態
	_DeviceStatus.PrinterDevice.PrinterWarning.CyanTonerStatus = _pPrinterWarning -> getCyanTonerStatus();
	// Ｋトナー状態
	_DeviceStatus.PrinterDevice.PrinterWarning.BlackTonerStatus = _pPrinterWarning -> getBlackTonerStatus();
	// トナー回収ボトル状態
	_DeviceStatus.PrinterDevice.PrinterWarning.TonerBottleStatus = _pPrinterWarning -> getTonerBottleStatus();
	// サイズエラー 
	_DeviceStatus.PrinterDevice.PrinterWarning.SizeError = _pPrinterWarning -> getSizeError();
	// メディアエラー
	_DeviceStatus.PrinterDevice.PrinterWarning.MediaError = _pPrinterWarning -> getMediaError();
	// Ｃ色ＩＵライフ情報
	_DeviceStatus.PrinterDevice.PrinterWarning.CyanIULife = _pPrinterWarning -> getCyanIULife();
	// Ｍ色ＩＵライフ情報
	_DeviceStatus.PrinterDevice.PrinterWarning.MagentaIULife = _pPrinterWarning -> getMagentaIULife();
	// Ｙ色ＩＵライフ情報
	_DeviceStatus.PrinterDevice.PrinterWarning.YellowIULife = _pPrinterWarning -> getYellowIULife();
	// Ｋ色ＩＵライフ情報
	_DeviceStatus.PrinterDevice.PrinterWarning.BlackIULife = _pPrinterWarning -> getBlackIULife();
	// 定着ユニットライフ情報
	_DeviceStatus.PrinterDevice.PrinterWarning.FuserUnitLife = _pPrinterWarning -> getFuserUnitLife();
	// 転写ベルトユニットライフ情報
	_DeviceStatus.PrinterDevice.PrinterWarning.TransferBeltUnitLife = _pPrinterWarning -> getTransferBeltUnitLife();
	// 転写クリーナーユニットライフ情報
	_DeviceStatus.PrinterDevice.PrinterWarning.TransCleanUnitLife = _pPrinterWarning -> getTransCleanUnitLife();
	// ２次転写ローラーユニットライフ情報
	_DeviceStatus.PrinterDevice.PrinterWarning.TransRollerUnitLife2nd = _pPrinterWarning -> get2ndTransRollerUnitLife();
	// 紙粉除去クリーナー／オゾンフィルタ情報
	_DeviceStatus.PrinterDevice.PrinterWarning.OzonFilterUnitLife = _pPrinterWarning -> getOzonFilterUnitLife();
	// ステープルエンプティ１(奥)
	_DeviceStatus.PrinterDevice.PrinterWarning.StapleEmpty1 = _pPrinterWarning -> isStapleEmpty();
    setPrinterWarning_21_1_2(); //機種固有処理
    setPrinterWarning_21_1_3(); //機種固有処理
	// コピー除去要求
	_DeviceStatus.PrinterDevice.PrinterWarning.PaperRemove = _pPrinterWarning -> isPaperRemove();
	// パンチくずフル 
	_DeviceStatus.PrinterDevice.PrinterWarning.PunchDustFull = _pPrinterWarning -> isPunchDustFull();
	// エレベートトレイ上昇未完
	_DeviceStatus.PrinterDevice.PrinterWarning.ElevateTrayNotLiftUp = _pPrinterWarning -> isElevateTrayNotLiftUp();
    setPrinterWarning_21_1_4(); //機種固有処理
}
//
//概要	  ：プリンタ状態設定
//        ：プリンタ状態（ＡＰＩ）APIC_PrinterStatus
//作成者  ：hayamori
//レビュー：未実施（レビュー実施日と名前を記述）
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
//
void
JOBC_ResourceManager::setPrinterStatus()
{
	// 初期化中
	_DeviceStatus.PrinterDevice.PrinterStatus.Initializing = _pPrinterStatus -> isInitializing();
	// ウォームアップ中
	_DeviceStatus.PrinterDevice.PrinterStatus.WarmingUp = _pPrinterStatus -> isWarmingUp();
	// 画像安定化中 
	_DeviceStatus.PrinterDevice.PrinterStatus.ImageKeeping = _pPrinterStatus -> isImageKeeping();
	// クリーニング中
	_DeviceStatus.PrinterDevice.PrinterStatus.Cleaning = _pPrinterStatus -> isCleaning();
	// 装置構成確定 
	_DeviceStatus.PrinterDevice.PrinterStatus.SystemConfigurationFixed = _pPrinterStatus -> isSystemConfigurationFixed();

	return;
}

//
//概要	  ：プリンタ情報設定
//        ：プリンタ情報（ＡＰＩ）APIC_PrinterInfo
//作成者  ：hayamori
//レビュー：未実施（レビュー実施日と名前を記述）
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
//
void
JOBC_ResourceManager::setPrinterInfo()
{
	// 先端画像欠損幅（ｍｍ単位）
	_DeviceStatus.PrinterDevice.PrinterInfo.TopMargine = _pPrinterInfo -> getTopMargine();
	// 後端画像欠損幅（ｍｍ単位）
	_DeviceStatus.PrinterDevice.PrinterInfo.BottomMargine = _pPrinterInfo -> getBottomMargine();
	// 左端画像欠損幅（ｍｍ単位）
	_DeviceStatus.PrinterDevice.PrinterInfo.LeftMargine = _pPrinterInfo -> getLeftMargine();
	// 右端画像欠損幅（ｍｍ単位）
	_DeviceStatus.PrinterDevice.PrinterInfo.RightMargine = _pPrinterInfo -> getRightMargine();

	return;
}

//
//概要	  ：スキャナ状態設定
//        ：スキャナシステム構成（ＡＰＩ）APIC_ScannerSystem
//作成者  ：hayamori
//レビュー：未実施（レビュー実施日と名前を記述）
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
//
void
JOBC_ResourceManager::setScannerSystem()
{
	// ＡＤＦ装着＆装着タイプ
	_DeviceStatus.ScannerDevice.ScannerSystem.ADFType = _pScannerSystem -> getADFType();

	return;
}


//
//概要	  ：スキャナ警告状態設定
//        ：スキャナ警告（ＡＰＩ）APIC_ScannerWarning
//作成者  ：hayamori
//レビュー：未実施（レビュー実施日と名前を記述）
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
//
void
JOBC_ResourceManager::setScannerWarning()
{
	// ＦＡＴＡＬコードの取得
	_DeviceStatus.ScannerDevice.ScannerWarning.Fatal = _pScannerWarning -> getFatal();
	// ＦＡＴＡＬが発生しているか
	_DeviceStatus.ScannerDevice.ScannerWarning.OccurFatal = _pScannerWarning -> isOccurFatal();
	// 周辺機器コミュニケーションエラーコード
	_DeviceStatus.ScannerDevice.ScannerWarning.CommError = _pScannerWarning -> getCommError();
	// 周辺機器コミュニケーションエラーが発生しているか
	_DeviceStatus.ScannerDevice.ScannerWarning.OccurCommError = _pScannerWarning -> isOccurCommError();
	// ＡＤＦジャムコード
	_DeviceStatus.ScannerDevice.ScannerWarning.AdfJam = _pScannerWarning -> getAdfJam();
	// ＡＤＦジャムが発生しているか
	_DeviceStatus.ScannerDevice.ScannerWarning.OccurAdfJam = _pScannerWarning -> isOccurAdfJam();
	// 何れかのドアが開いているか
	_DeviceStatus.ScannerDevice.ScannerWarning.AdfDoorOpen = _pScannerWarning -> isAdfDoorOpen();
	// プラテンカバーが開いているか
	_DeviceStatus.ScannerDevice.ScannerWarning.PlatenCoverOpen = _pScannerWarning -> isPlatenCoverOpen();

	return;
}


//
//概要	  ：スキャナー状態設定
//        ：スキャナ状態（ＡＰＩ）APIC_ScannerStatus
//作成者  ：hayamori
//レビュー：未実施（レビュー実施日と名前を記述）
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
//
void
JOBC_ResourceManager::setScannerStatus()
{
	// 初期化中
	_DeviceStatus.ScannerDevice.ScannerStatus.Initializing = _pScannerStatus -> isInitializing();
	// ＡＤＦ動作中 
	_DeviceStatus.ScannerDevice.ScannerStatus.ADFBusy = _pScannerStatus -> isADFBusy();
	// 装置構成確定 
	_DeviceStatus.ScannerDevice.ScannerStatus.SystemConfigurationFixed = _pScannerStatus -> isSystemConfigurationFixed();

	return;
}


//
//概要	  ：スキャナ原稿情報設定
//        ：原稿情報（ＡＰＩ）APIC_Original
//作成者  ：hayamori
//レビュー：未実施（レビュー実施日と名前を記述）
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
//
void
JOBC_ResourceManager::setScannerPaper()
{
	// 手置き・原稿有り
	_DeviceStatus.ScannerDevice.Original.PlatenOriginalExist = _pOriginal -> isPlatenOriginalExist();
	// 手置き・原稿サイズ確定
	_DeviceStatus.ScannerDevice.Original.PlatenOriginalSizeFixed = _pOriginal -> isPlatenOriginalSizeFixed();
	// 手置き・原稿サイズ
	_DeviceStatus.ScannerDevice.Original.PlatenOriginalSize = _pOriginal -> getPlatenOriginalSize();
	// ＡＤＦ・原稿有り
	_DeviceStatus.ScannerDevice.Original.AdfOriginalExist = _pOriginal -> isAdfOriginalExist();
	// ＡＤＦ・原稿サイズ確定
	_DeviceStatus.ScannerDevice.Original.AdfOriginalSizeFixed = _pOriginal -> isAdfOriginalSizeFixed();
	// ＡＤＦ・原稿サイズ
	_DeviceStatus.ScannerDevice.Original.AdfOriginalSize = _pOriginal -> getAdfOriginalSize();
	// ＥＥ時の濃度（モノクロ機）
	_DeviceStatus.ScannerDevice.Original.MonoDensity = _pOriginal -> getMonoDensity();
	// ＡＣＳ結果（カラー機）
	_DeviceStatus.ScannerDevice.Original.ResultACS = _pOriginal -> getResultACS();

	return;
}

//
//概要　　：イベント事象を通知
//戻り値　：無し
//備考　　：Observer通知実行
//作成者　：sakuragi
//レビュー：
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
//
void 
JOBC_ResourceManager::notifyObserver(uint32_t ul_notifycode)
{

	_pc_List.Lock();
	JOBC_ResourceManagerObserver* pc_ResourceManagerObserver = _pc_List.findFirst();

	while( pc_ResourceManagerObserver != NULL )
	{
		pc_ResourceManagerObserver->update(ul_notifycode);// データ変化通知 
		pc_ResourceManagerObserver = _pc_List.findNext();
	}
	_pc_List.UnLock();

	return;
}

//関数-------------------------------------------------------------------------------------------------------------------
//概要　　：コンストラクタ
//作成者　：櫻木
//レビュー：未実施（レビュー実施日と名前を記述）
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
JOBC_ResourceManagerObserver::JOBC_ResourceManagerObserver()
{
}
//関数-------------------------------------------------------------------------------------------------------------------
//概要　　：デストラクタ 
//作成者　：櫻木
//レビュー：未実施（レビュー実施日と名前を記述）
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
JOBC_ResourceManagerObserver::~JOBC_ResourceManagerObserver()
{
}


//２nd 関数---------------------------------------------------------------------------------------------------------------

//概要    ：BoxJob予約リスト取得
//備考    ：Box内DocumentのLeaf/Imageへの展開処理中のジョブリストを返す。
void 
JOBC_ResourceManager::getReserveList(
	std::deque<JOBC_Job*>& reserveList)
{
}

//概要    ：展開中ジョブ情報設定
//備考    ：展開するJobへのポインタ及び展開するDoc数を設定する
void 
JOBC_ResourceManager::setRestoringJobInfo(JOBC_Job* pJob)
{
	pc_RestoringJob = pJob;

	// JobSpecよりDocIDを取得
	int32_t sl_DocID[TYPD_MaxProgramJob];

	int32_t sl_DocCnt = 0;
	for( int i = 0 ; i < TYPD_MaxProgramJob ; i++ ){
		sl_DocID[i] = TYPD_NotSetting;
	}

	pJob->getJobSpec()->pc_Common->c_JobType.getDocumentNo(sl_DocID);

	// Doc数算出
	int32_t l_No = 0;

	if( False != pJob->getJobSpec()->pc_Common->c_JobType.getProgramJobOn() ){
		l_No = TYPD_MaxProgramJob;
	}
	else{
		l_No = TYPD_MaxBindJob;
	}

	for(int32_t num=0;num<l_No;num++)
	{ if( 0 < sl_DocID[static_cast<int>(num)] ) sl_DocCnt++; }


	uc_NumOfDocNeeded = static_cast<uchar>(sl_DocCnt);
	uc_NumOfDocRestored = 0x00;
}

//概要    ：BoxJob開始指示
//備考    ：Box内DocumentのLeaf/Imageへの展開処理を順次行う。
//作成者  ：sakuragi
//レビュー：未実施（レビュー実施日と名前を記述）
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
void 
JOBC_ResourceManager::req_reserveBoxJob(
	JOBC_Job* pJob, 
	void (*ans_reserveBoxJob)(JOBC_Job* pJob, DOCC_CompositeDoc* pDoc, int32_t sl_result))
{
	JOBS_RMStateMsgBuff sMsg;
	sMsg.sl_EventID					= JOBD_MSG_BOXJOB_START;
	sMsg.pJob						= pJob;
	sMsg.ans_reserveBoxJob			= ans_reserveBoxJob;
	sendMsgRMState(&sMsg);
}

//概要    ：BoxJob破棄指示
//備考    ：Box内DocumentのLeaf/Imageへの展開処理を中止する。
//作成者  ：sakuragi
//レビュー：未実施（レビュー実施日と名前を記述）
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
// 2005/11/24 Kosuge,T DocFileを消すかどうかを上位から指定できるように変更。
void 
JOBC_ResourceManager::req_cancelBoxJobReserve(
	JOBC_Job* pJob, 
	void (*ans_cancelBoxJobReserve)(JOBC_Job* pJob, int32_t sl_result),
	Bool b_deleteDocFile)
{
	JOBS_RMStateMsgBuff sMsg;
	sMsg.sl_EventID					= JOBD_MSG_BOXJOB_CANCEL;
	sMsg.pJob						= pJob;
	sMsg.ans_cancelBoxJobReserve	= ans_cancelBoxJobReserve;
	sMsg.b_needExtraAction			= b_deleteDocFile;
	sendMsgRMState(&sMsg);
}

//概要    ：コールバック通知(req_DeSerialize)
//作成者  ：sakuragi
//レビュー：未実施（レビュー実施日と名前を記述）
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
void 
JOBC_ResourceManager::ans_DeSerializeForResourceManager(
	int32_t ret, void *p2 )
{
	JOBS_RMStateMsgBuff sMsg;
	sMsg.sl_EventID					= JOBD_MSG_BOXJOB_ANS_DSZ;
	sMsg.ret						= ret;
	sendMsgRMState(&sMsg);
}

//概要    ：コールバック通知(req_changeDocFilePosition)
//作成者  ：sakuragi
//レビュー：未実施（レビュー実施日と名前を記述）
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
void 
JOBC_ResourceManager::ans_changeDocFilePositionForResourceManager(
	int32_t ret,
	void* pJob )
{
	JOBS_RMStateMsgBuff sMsg;
	sMsg.sl_EventID					= JOBD_MSG_BOXJOB_ANS_CDP;
	sMsg.pJob						= reinterpret_cast<JOBC_Job*>(pJob);
	sMsg.ret						= ret;
	sendMsgRMState(&sMsg);
}

//概要    ：コールバック通知(req_getInstance)
//作成者  ：sakuragi
//レビュー：未実施（レビュー実施日と名前を記述）
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
void 
JOBC_ResourceManager::ans_getInstanceForResourceManager(
	DOCC_CompositeDoc* pDoc,
	void* pJob,
	int32_t ret )
{
	JOBS_RMStateMsgBuff sMsg;
	sMsg.sl_EventID					= JOBD_MSG_BOXJOB_ANS_GIT;
	sMsg.pJob						= reinterpret_cast<JOBC_Job*>(pJob);
	sMsg.pDoc						= pDoc;
	sMsg.ret						= ret;
	sendMsgRMState(&sMsg);
}

//
//概要　　：Msg送信
//作成者  ：sakuragi
//レビュー：未実施（レビュー実施日と名前を記述）
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
//
void
JOBC_ResourceManager::sendMsgRMState( JOBS_RMStateMsgBuff* sMsg )
{
	STATUS	status = 0;

	status = SYSF_msgQSend(	_pResourceManager->RMState_QueId,
							reinterpret_cast<char *>(sMsg),
							static_cast<UINT>(sizeof(JOBS_RMStateMsgBuff)),
							NO_WAIT,
							MSG_PRI_NORMAL);
	return;
}

//
//概要　　：Msg受信
//作成者  ：sakuragi
//レビュー：未実施（レビュー実施日と名前を記述）
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
//
void
JOBC_ResourceManager::Loop_Main()
{
	int  result = 0;
	JOBS_RMStateMsgBuff rMsg;

	//無限ループ
	while(TRUE) 
	{
		// receive message
		result = SYSF_msgQReceive( 
					RMState_QueId, 					// message queue from which to receive 
					reinterpret_cast<char *>(&rMsg),					// buffer to receive message 
					sizeof(JOBS_RMStateMsgBuff),	// length of buffer 
					WAIT_FOREVER );					// ticks to wait 

		if( result != sizeof(JOBS_RMStateMsgBuff) ){
			continue;
		}
		//メッセージ機能処理実行
		if(RM_Main<=RMDBG) dspRMLOG(this->pc_ResourceManagerState,&rMsg);
		setRMLOG(this->pc_ResourceManagerState->who(),JOBE_ResourceManagerStateUnknown,rMsg.sl_EventID,rMsg.pJob,rMsg.ret);
// 2017/11/22 AR1029174 SegF時新詳細コード対応 s.uekama start.
		// メッセージ種別.
		uc_msgTypeLocal = static_cast<uchar>(rMsg.sl_EventID);
// 2017/11/22 AR1029174 SegF時新詳細コード対応 s.uekama start.

		switch( rMsg.sl_EventID )
		{
			case JOBD_MSG_BOXJOB_START:				// BoxJob開始指示
				this->pc_ResourceManagerState->evtReserveBoxJob(this,rMsg.pJob);
				break;

			case JOBD_MSG_BOXJOB_CANCEL:			// BoxJob破棄指示
				this->pc_ResourceManagerState->evtCancelBoxJobReserve(this,rMsg.pJob,rMsg.b_needExtraAction);
				break;

			case JOBD_MSG_BOXJOB_TIMER:				// タイマー満了通知
				this->pc_ResourceManagerState->evtTimerExpired(this);
				break;

			case JOBD_MSG_BOXJOB_ANS_DSZ:			// コールバック通知(req_DeSerialize)
				this->pc_ResourceManagerState->evtAnsDeserializeDoc(this,rMsg.ret);
				break;

			case JOBD_MSG_BOXJOB_ANS_CDP:			// コールバック通知(req_changeDocFilePosition)
				this->pc_ResourceManagerState->evtAnsChangeDocFilePosition(this,rMsg.ret);
				break;

			case JOBD_MSG_BOXJOB_ANS_GIT:			// コールバック通知(req_getInstance)
				this->pc_ResourceManagerState->evtAnsGetInstance(this,rMsg.pDoc,rMsg.ret);
				break;

			default:								// 例外
				break;
		}
	}
	return;
}

// 2017/11/28 AR1029174 SegF時新詳細コード対応 s.uekama start.
// オーナーコールバック.
void JOBC_ResourceManager::RMStateCallBackFunc(ushort us_TaskDetailCode)
{
	uint64_t	ull_detailCode = 0;
	uchar		uc_CodeWork[8] = {};
	
	memset(uc_CodeWork, 0, sizeof(uc_CodeWork));

	//--------------------------------------------------------------------------
	// モジュール番号.
	//--------------------------------------------------------------------------
	uc_CodeWork[7] = JOBD_NewDetailCode_JOB;			// sys:Job.
	
	//--------------------------------------------------------------------------
	// パラメータ出力.
	//--------------------------------------------------------------------------
	uc_CodeWork[6] = JOBD_NEWDETAILCODE_OWNER;			// オーナー.

	uc_CodeWork[5] = (uc_msgTypeLocal << 4);			// イベントID.
	// [changelog] 2017/12/20 AR1029174 khueqt IT5_4.2 start.
	uc_CodeWork[4] = uc_boxDetailCode;	// Set box detail code.
	// [changelog] 2017/12/20 AR1029174 khueqt IT5_4.2 end.
	uc_CodeWork[3] = 0;		// 未使用.
	uc_CodeWork[2] = 0;		// 未使用.
	uc_CodeWork[1] = 0;		// 未使用.
	uc_CodeWork[0] = 0;		// 未使用.

	memcpy(&ull_detailCode, uc_CodeWork, sizeof(uc_CodeWork));
	APIC_setNewDetailCode(us_TaskDetailCode , ull_detailCode);
}

// グループ1コールバック.
void JOBC_ResourceManager::RMStateCallBackFunc_Group1(ushort us_TaskDetailCode)
{
}

// グループ2コールバック.
void JOBC_ResourceManager::RMStateCallBackFunc_Group2(ushort us_TaskDetailCode)
{
}
// 2017/11/28 AR1029174 SegF時新詳細コード対応 s.uekama end.

// < class JOBC_ResourceManagerState >-------------------------------------------------

//概要    ：状態に入ったときに必ず行う処理
//作成者  ：sakuragi
//レビュー：未実施（レビュー実施日と名前を記述）
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
void
JOBC_ResourceManagerState::onEntry(JOBC_ResourceManager* pManager)
{
	// 処理なし
}

//概要    ：状態を抜けるときに必ず行う処理
//作成者  ：sakuragi
//レビュー：未実施（レビュー実施日と名前を記述）
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
void
JOBC_ResourceManagerState::onExit(JOBC_ResourceManager* pManager)
{
	// 処理なし
}

//概要    ：BoxJob予約指示
//作成者  ：sakuragi
//レビュー：未実施（レビュー実施日と名前を記述）
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
void
JOBC_ResourceManagerState::evtReserveBoxJob(
	JOBC_ResourceManager* pManager,
	JOBC_Job* pJob)
{
	if(RM_State<=RMDBG) RMLOG("★JOBC_ResourceManagerState::evtReserveBoxJob★\n",0,0,0,0,0);
	// [changelog] 2017/12/20 AR1029174 khueqt Supported SegF NewDetailCode for JOBC_RMState IT5_4.2 start.
	uc_boxDetailCode = getBoxDetailCode(pJob);
	// [changelog] 2017/12/20 AR1029174 khueqt Supported SegF NewDetailCode for JOBC_RMState IT5_4.2 end.

	// add_List
	if( False == addListReserveBoxJob(pManager,pJob)){
		return;
	}
}

//概要    ：BoxJob予約破棄指示
//作成者  ：sakuragi
//レビュー：未実施（レビュー実施日と名前を記述）
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
// 2005/11/24 Kosuge,T DocFileを消すかどうかを上位から指定できるように変更。
void
JOBC_ResourceManagerState::evtCancelBoxJobReserve(
	JOBC_ResourceManager* pManager,
	JOBC_Job* pJob,
	Bool b_deleteDocFile)
{
	if(RM_State<=RMDBG) RMLOG("★JOBC_ResourceManagerState::evtCancelBoxJobReserve★\n",0,0,0,0,0);
	// [changelog] 2017/12/20 AR1029174 khueqt Supported SegF NewDetailCode for JOBC_RMState IT5_4.2 start.
	uc_boxDetailCode = getBoxDetailCode(pJob);
	// [changelog] 2017/12/20 AR1029174 khueqt Supported SegF NewDetailCode for JOBC_RMState IT5_4.2 end.

	// check_BoxList
	JOBC_ElementOfReserveBoxJobList* pc_CheckBoxJobList 
		= pManager->mReserveList->searchAndGetReservation(pJob);
	if( NULL == pc_CheckBoxJobList ){
		// callback
		switch(pJob->getBoxJobState())
		{
		case JOBE_BoxJobState_Preview_Requesting	:	// メモリ展開要求中の場合 
		case JOBE_BoxJobState_Preview_Restoring		:	// メモリ展開中の場合 
		case JOBE_BoxJobState_Preview_Completed		:	// メモリ展開完了の場合 
		case JOBE_BoxJobState_Preview_CancelReq		:	// メモリ破棄要求中の場合 
			pJob->ans_cancelBoxToPreviewReserve(pJob,JOBD_ReserveBoxJob_OK);
			break;

		default										:	// その他の場合 
			pJob->ans_cancelBoxJobReserve(pJob,JOBD_ReserveBoxJob_OK);
			break;
		}
	}
	else{
		// 予約削除後DocFileを消すかどうかを設定(上位の指示通り動かす)
		pc_CheckBoxJobList->decideDocFileDelete(b_deleteDocFile);
		if( pJob != pManager->getRestoringJob() ){
			// DeleteDocument判定
			if( False == pc_CheckBoxJobList->checkDocFileDelete() ){
				// closeDoc & Doc::deleteInstance()
				if( False == closeDoc_deleteInstance(pManager,pc_CheckBoxJobList)){
					return;
				}
			}
			else{
				// Doc::deleteAll()
				if( False == deleteAll(pManager,pc_CheckBoxJobList)){
					return;
				}
			}
			// callback & deleteList
			callback_deleteBoxJobList(pManager,pc_CheckBoxJobList,JOBD_ReserveBoxJob_OK);
		}
	}
}

//概要    ：CompositeDoc生成完了通知
//作成者  ：sakuragi
//レビュー：未実施（レビュー実施日と名前を記述）
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
void
JOBC_ResourceManagerState::evtAnsGetInstance(
	JOBC_ResourceManager* pManager,
	DOCC_CompositeDoc* pDoc,
	int32_t sl_result)
{
	// 処理なし
}

//概要    ：CompositeDoc生成完了通知
//作成者  ：sakuragi
//レビュー：未実施（レビュー実施日と名前を記述）
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
void
JOBC_ResourceManagerState::evtAnsDeserializeDoc(
	JOBC_ResourceManager* pManager,
	int32_t sl_result)
{
	// 処理なし
}

//概要    ：タイマー満了通知
//作成者  ：sakuragi
//レビュー：未実施（レビュー実施日と名前を記述）
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
void
JOBC_ResourceManagerState::evtTimerExpired(
	JOBC_ResourceManager* pManager)
{
	// 処理なし
}

//概要    ：DocFileCopy完了通知
//作成者  ：sakuragi
//レビュー：未実施（レビュー実施日と名前を記述）
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
void
JOBC_ResourceManagerState::evtAnsChangeDocFilePosition(
	JOBC_ResourceManager* pManager,
	int32_t sl_result)
{
	// 処理なし
}

//概要    ：DocFileDelete完了通知
//作成者  ：sakuragi
//レビュー：未実施（レビュー実施日と名前を記述）
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
void
JOBC_ResourceManagerState::evtAnsDeleteDoc(
	JOBC_ResourceManager* pManager,
	int32_t sl_result)
{
	// 処理なし
}

//概要    ：状態遷移処理
//作成者  ：sakuragi
//レビュー：未実施（レビュー実施日と名前を記述）
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
void
JOBC_ResourceManagerState::changeState(
	JOBC_ResourceManager* pManager,
	JOBC_ResourceManagerState* pNextState)
{
	if(RM_Main<=RMDBG) dspRMLOG2(pManager->getRestoringJob(),this,pNextState);
	setRMLOG(this->who(),pNextState->who(),JOBD_MSG_BOXJOB_NON,pManager->getRestoringJob(),0);

	this->onExit(pManager);
	pManager->changeState(pNextState);
	pNextState->onEntry(pManager);
}

//概要    ：リスト追加.
//備考    ：.
//作成者  ：sakuragi.
//レビュー：未実施（レビュー実施日と名前を記述）.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
// 			2011.01.27 日立JTE亀山 CQ336075 Coverity5(NULL間接参照対応).
//          2011.07.07 S.Higaki CQ.350243 省電力対応.
Bool
JOBC_ResourceManagerState::addListReserveBoxJob(
	JOBC_ResourceManager* pManager,
	JOBC_Job* pJob)
{
	int32_t DocRtn = DOCD_RetOk;
	Bool rtn = True;

	int32_t sl_DocID[TYPD_MaxProgramJob];

	int32_t sl_DocCnt = 0;
	DOCC_BoxIf* pBox=NULL;
	DOCC_CompositeDoc* pDoc=NULL;

	// HDDが有る場合、HDD Ready(フォーマット済み)になるまで待ち続ける.
	waitHddReady();

	// JobSpecよりDocIDを取得
	for( int i = 0 ; i < TYPD_MaxProgramJob ; i++ ){
		sl_DocID[i] = TYPD_NotSetting;
	}

	pJob->getJobSpec()->pc_Common->c_JobType.getDocumentNo(sl_DocID);

	// Doc数算出
	int32_t l_No = 0;

	if( False != pJob->getJobSpec()->pc_Common->c_JobType.getProgramJobOn() ){
		l_No = TYPD_MaxProgramJob;
	}
	else{
		l_No = TYPD_MaxBindJob;
	}

	for(int32_t num=0;num<l_No;num++)
	{ if( 0 < sl_DocID[static_cast<int>(num)] )	sl_DocCnt++; }


	

	JOBC_ElementOfReserveBoxJobList* pc_ReserveBoxJobList = NULL;
	switch(pJob->getBoxJobState())
	{
	case JOBE_BoxJobState_Preview_Requesting	:	// メモリ展開要求中の場合 
	case JOBE_BoxJobState_Preview_Restoring		:	// メモリ展開中の場合 
	case JOBE_BoxJobState_Preview_Completed		:	// メモリ展開完了の場合 
	case JOBE_BoxJobState_Preview_CancelReq		:	// メモリ破棄要求中の場合 
		// 仕上がりプレビュー時は、仕上がりプレビューのコールバックを指定 
		pc_ReserveBoxJobList = new JOBC_ElementOfReserveBoxJobList(pJob,pJob->ans_reserveBoxToPreview,pJob->ans_cancelBoxToPreviewReserve);
		break;
	default										:	// その他の場合 
		// その他の場合は通常のコールバックを指定 
		pc_ReserveBoxJobList = new JOBC_ElementOfReserveBoxJobList(pJob,pJob->ans_reserveBoxJob,pJob->ans_cancelBoxJobReserve);
		break;
	}
	if( NULL == pc_ReserveBoxJobList ){
		RMLOG("★JOBC_ResourceManagerState::addListReserveBoxJob pc_ReserveBoxJobList=NULL!!★\n",0,0,0,0,0);
		APIC_setFatalDetailCodeE002(CE002_DetailCode0x50F3);
		APIC_ThrowSystemError(SYSE_SysErrTypeSystemExhausted,LOGF_DEBUG_EVENT_KIND_JOB,
			const_cast<const char *>("JOBC_ResourceManagerState::evtReserveBoxJob JOBC_ElementOfReserveBoxJobListインスタンス生成失敗"));
		return False;
	}

	do
	{
		// Box::newInstance()
		pBox = DOCC_BoxIf::newInstance(pJob->getJobSpec()->pc_Common->c_JobType.getBoxNo());
		if( NULL == pBox ){
			RMLOG("★JOBC_ResourceManagerState::addListReserveBoxJob pBox=NULL!!★\n",0,0,0,0,0);
			DocRtn = DOCD_RetParameterErr;
			break;
		}

		if( 1 != sl_DocCnt ) { // BindJob
			for(int32_t num=0;num<sl_DocCnt;num++)
			{
				// Doc::newInstance()
				pDoc = DOCC_CompositeDoc::newInstance();
				if( NULL == pDoc ){
					RMLOG("★JOBC_ResourceManagerState::addListReserveBoxJob Bind-pDoc=NULL!!★\n",0,0,0,0,0);
					DocRtn = DOCD_RetDocNotExist;
					break;
				}
				// Set_DocPtr
				pc_ReserveBoxJobList->setBindingDoc(static_cast<uchar>(num+1),pDoc);
				// 連結pBox->getDoc()
				DocRtn = pBox->getDoc(pc_ReserveBoxJobList->getBindingDoc(static_cast<uchar>(num+1)),sl_DocID[static_cast<int>(num)]);
				if( DOCD_RetOk != DocRtn ){
					RMLOG("★JOBC_ResourceManagerState::addListReserveBoxJob Bind-getDoc-NG!!★\n",0,0,0,0,0);
					break;
				}

				if( NULL == pc_ReserveBoxJobList->getBindingDoc(static_cast<uchar>(num+1)) ) {
					APIC_setNewDetailCode(0, makeNewDetailCodeJob0AType01(JOBD_NewDetailCodeD002_PlaceCode_ResourceManager_001, num+1));	// 1027261 新詳細コード追加対応 2017/8/8 m.ooba.
					JOBC_Job::setSysErr( TYPD_TroubleCode_D002, __LINE__ );
				}
				// OpenDoc
				DocRtn = pc_ReserveBoxJobList->getBindingDoc(static_cast<uchar>(num+1))->openDoc();
				if( DOCD_RetOk != DocRtn ){
					RMLOG("★JOBC_ResourceManagerState::addListReserveBoxJob Bind-openDoc-NG!!★\n",0,0,0,0,0);
					break;
				}
			}
			if( DOCD_RetOk != DocRtn ) break; // do while から抜ける
		}

		// ParentDoc
		// Doc::newInstance()
		pDoc = DOCC_CompositeDoc::newInstance();
		if( NULL == pDoc ){
			RMLOG("★JOBC_ResourceManagerState::addListReserveBoxJob pDoc=NULL!!★\n",0,0,0,0,0);
			DocRtn = DOCD_RetDocNotExist;
			break;
		}
		// Set_DocPtr
		pc_ReserveBoxJobList->setCompositeDoc(pDoc);
		// BindJobかどうか
		if( False == pc_ReserveBoxJobList->isBindJob() ){
			// 連結pBox->getDoc()
			DocRtn = pBox->getDoc(pc_ReserveBoxJobList->getCompositeDoc(),sl_DocID[0]);
			if( DOCD_RetOk != DocRtn ){
				RMLOG("★JOBC_ResourceManagerState::addListReserveBoxJob getDoc-NG!!★\n",0,0,0,0,0);
				break;
			}
			// OpenDoc
			DocRtn = pc_ReserveBoxJobList->getCompositeDoc()->openDoc();
			// 既にファイルオープンされている場合は許容するものとする。
			if ( DOCD_RetOk != DocRtn )
			{
				// 文書保持設定が「しない」の場合は、Docが削除されるケースがあるため、
				// ファイルの２重オープンを許可しない。
				// 許可した場合、同じDOCを使用している後続ジョブが動けなくなってしまう。
				APIC_SystemSetting* _pc_SysSetting = APIC_SystemSetting::newInstance();
				Bool	b_AllDoc=False;
				if(_pc_SysSetting!=NULL)
				{
					b_AllDoc = _pc_SysSetting->getDeleteEnableInPickingDoc();
				}
				if (((b_AllDoc == False) && ( APID_DOCD_RetDocOpened == DocRtn)) || //文書保持設定「する」&&既にファイルを開いている．
					(pJob->getJobSpec()->pc_Common->c_JobType.getBoxNo() == DOCD_ImageDealerBox))	// イメージディラーBox である.

				{
					//イメージディーラーBoxの場合は、すでに他ジョブがファイルオープンしていていも、他ジョブがファイルを削除することがないので、NGではない。
					DocRtn = DOCD_RetOk;
				}else{
					RMLOG("★JOBC_ResourceManagerState::addListReserveBoxJob openDoc-NG!!★\n",0,0,0,0,0);
					break;
				}
			}
		}
		// Box::deleteInstance()
		pBox->deleteInstance();
		// Need DocFileCopy???
		if(checkDocumentCopy(pJob) == True) pc_ReserveBoxJobList->setCopiedBoxID(DOCD_FaxSndBox);

		// callback
		pc_ReserveBoxJobList->execReserveEnd(JOBD_ReserveBoxJob_Accepted);
		// add_List
		pManager->mReserveList->addReservation(pc_ReserveBoxJobList);

	}while(False);

	if( DOCD_RetOk != DocRtn ){
		// ジョブ破棄・システムエラー発生
		if( NULL != pBox) pBox->deleteInstance();
		setDocAccessErr(pManager,pc_ReserveBoxJobList,DocRtn);
		rtn = False;
	}
	return rtn;
}

//概要    ：DocumentCopy判定
//戻り値  ：True :BoxFax送信
//        ：False:BoxPrint,BoxNet送信
//作成者  ：sakuragi
//レビュー：未実施（レビュー実施日と名前を記述）
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
// 2006/04/20 渡邊由和(13開発) バインド配信対応(Mosel)
Bool 
JOBC_ResourceManagerState::checkDocumentCopy(
	JOBC_Job* pJob)
{
	int32_t sl_BoxID =  pJob->getJobSpec()->pc_Common->c_JobType.getBoxNo();
	TYPE_JobType e_JobType = pJob->getJobSpec()->pc_Common->c_JobType.getJobType();

	// Box状態が仕上がりプレビューのステータスの場合、Falseをリターン 
	switch(pJob->getBoxJobState())
	{
	case JOBE_BoxJobState_Preview_Requesting	:	// メモリ展開要求中の場合 
	case JOBE_BoxJobState_Preview_Restoring		:	// メモリ展開中の場合 
	case JOBE_BoxJobState_Preview_Completed		:	// メモリ展開完了の場合 
	case JOBE_BoxJobState_Preview_CancelReq		:	// メモリ破棄要求中の場合 
		return (False);
	default										:	// その他の場合 
		break;
	}

	// ユニバーサルアドレスタイプを取得
	// Box配信かつスキャン宛先がOFFかつユーザBoxからの送信ジョブ(つまり、BoxからのFax送信)の場合はTrueを返却
	if((e_JobType == TYPE_JobTypeFaxSendMemory) && 
	   (isUniAddrScan(pJob->getJobSpec()) != True) &&
	   ((True == isUserBoxJob(sl_BoxID) ) ||
	    (DOCD_ImageDealerBox == sl_BoxID) ||
	    (DOCD_CompulsionRcvBox == sl_BoxID)))
	{
		return(True);
	}

	return(False);
}

//概要    ：DocumentPage判定
//戻り値  ：True:OK,False:NG
//作成者  ：sakuragi
//レビュー：未実施（レビュー実施日と名前を記述）
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
Bool 
JOBC_ResourceManagerState::checkDocumentPage(
	JOBC_ResourceManager* pManager,
	JOBC_Job* pJob)
{
	int32_t sl_DocID[TYPD_MaxProgramJob];

	for( int i = 0 ; i < TYPD_MaxProgramJob ; i++ ){
		sl_DocID[i] = TYPD_NotSetting;
	}
	pJob->getJobSpec()->pc_Common->c_JobType.getDocumentNo(sl_DocID);

	

	// 展開page
	uint32_t ul_tenkaiPage =
		DOCC_DocFileAccessIf::getLeafCnt(
			pJob->getJobSpec()->pc_Common->c_JobType.getBoxNo(),
			sl_DocID[pManager->getNumOfDocRestored()]);
	// 空きpage
	uint32_t ul_akiPage = DOCD_MaxPage - DOCC_Leaf::getUsedLeafInstCnt();
	return ( ul_tenkaiPage <= ul_akiPage ) ? True : False;
}

//概要    ：Cancel状態遷移(処理中ジョブ・指定ジョブチェック)
//作成者  ：sakuragi
//レビュー：未実施（レビュー実施日と名前を記述）
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
// 2005/11/24 Kosuge,T DocFileを消すかどうかを上位から指定できるように変更。
void 
JOBC_ResourceManagerState::evtCancelBoxJobReserveByJob(
	JOBC_ResourceManager* pManager,
	JOBC_Job* pJob,
	Bool b_deleteDocFile)
{
	JOBC_ElementOfReserveBoxJobList* pc_Element = NULL;
	if( pJob == pManager->getRestoringJob() && 
		(pc_Element = pManager->mReserveList->searchAndGetReservation(pJob)) != NULL){

		// 予約削除後DocFileを消すかどうかを設定(上位の指示通り動かす)
		pc_Element->decideDocFileDelete(b_deleteDocFile);
		// CancellingDocRestore状態遷移
		changeState(pManager,JOBC_ResourceManagerStateCancellingDocRestore::newInstance());
	}
	else{
		JOBC_ResourceManagerState::evtCancelBoxJobReserve(pManager,pJob,b_deleteDocFile);
	}
}

//概要    ：closeDoc & Doc::deleteInstance()
//作成者  ：sakuragi
//レビュー：未実施（レビュー実施日と名前を記述）
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
Bool 
JOBC_ResourceManagerState::closeDoc_deleteInstance(
	JOBC_ResourceManager* pManager,
	JOBC_ElementOfReserveBoxJobList* pc_ReserveBoxJobList)
{
	int32_t DocRtn = DOCD_RetOk;
	Bool rtn = True;
	do
	{
		// オーバーレイのCompositeDoc解放＆初期化 
		// =>オーバーレイのCompositeDocを先に削除しなければisBindJob()が全てTrueとなるので注意 
		clearOverLayDoc(pManager);

		if( True == pc_ReserveBoxJobList->isBindJob() ){
			// BindJobは親DocのcloseDocが不要。
			unsigned char num=1;
			while( NULL != pc_ReserveBoxJobList->getBindingDoc(num)){
				DocRtn = pc_ReserveBoxJobList->getBindingDoc(num)->closeDoc();	// CloseDoc
				if( DOCD_RetOk != DocRtn ){
					RMLOG("JOBC_ResourceManagerState::closeDoc_deleteInstance Bind-closeDoc-NG!!★\n",0,0,0,0,0);
					break;
				}
				pc_ReserveBoxJobList->getBindingDoc(num)->deleteInstance();		// Doc::deleteInstance()
				num++;
			}
		}
		else{
			// CloseDoc
			if( NULL != pc_ReserveBoxJobList->getCompositeDoc() ){
				DocRtn = pc_ReserveBoxJobList->getCompositeDoc()->closeDoc();
				if( DOCD_RetOk != DocRtn ){
					RMLOG("JOBC_ResourceManagerState::closeDoc_deleteInstance closeDoc-NG!!★\n",0,0,0,0,0);
					break;
				}
			}
		}
		if( NULL != pc_ReserveBoxJobList->getCompositeDoc() ){
			// Doc::deleteInstance()
			pc_ReserveBoxJobList->getCompositeDoc()->deleteInstance();
		}
	}while(False);

	if( DOCD_RetOk != DocRtn ){
		// ジョブ破棄・システムエラー発生
		setDocAccessErr(pManager,pc_ReserveBoxJobList,DocRtn);
		rtn = False;
	}
	return rtn;
}

//概要    ：Doc::deleteAll() OriginalのDocumentをファイルごと削除するときに利用する。
//作成者  ：sakuragi
//レビュー：未実施（レビュー実施日と名前を記述）
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
Bool 
JOBC_ResourceManagerState::deleteAll(
	JOBC_ResourceManager* pManager,
	JOBC_ElementOfReserveBoxJobList* pc_ReserveBoxJobList)
{
	int32_t DocRtn = DOCD_RetOk;
	Bool rtn = True;
	DOCC_BoxIf* pBox=NULL;

	do
	{
		// Box::newInstance()
		pBox = DOCC_BoxIf::newInstance(
			pc_ReserveBoxJobList->getJob()->getJobSpec()->pc_Common->c_JobType.getBoxNo());
		if( NULL == pBox ){
			RMLOG("★JOBC_ResourceManagerState::deleteAll pBox=NULL!!★\n",0,0,0,0,0);
			DocRtn = DOCD_RetParameterErr;
			break;
		}
		// Doc::deleteAll() 予約時にDocOpenされている。
		DocRtn = pc_ReserveBoxJobList->getCompositeDoc()->deleteAll(pBox);// OpenDoc中必須
		if( DOCD_RetOk != DocRtn ){
			RMLOG("★JOBC_ResourceManagerState::deleteAll deleteAll-NG!!★\n",0,0,0,0,0);
			break;
		}
		// Box::deleteInstance()
		pBox->deleteInstance();

	}while(False);

	if( DOCD_RetOk != DocRtn ){
		// ジョブ破棄・システムエラー発生
		if( NULL != pBox) pBox->deleteInstance();
		setDocAccessErr(pManager,pc_ReserveBoxJobList,DocRtn);
		rtn = False;
	}
	return rtn;
}

//概要    ：Doc::deleteAll() Copy先
//作成者  ：sakuragi
//レビュー：2005/10/27 Kosuge,T この処理は予約ジョブ毎の処理になるので
//                              JOBC_ElementOfReserveBoxJobListクラスのメソッドにするべきかも。
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
Bool 
JOBC_ResourceManagerState::deleteAllForCopyEnd(
	JOBC_ResourceManager* pManager,
	JOBC_ElementOfReserveBoxJobList* pc_ReserveBoxJobList)
{
	int32_t DocRtn = DOCD_RetOk;
	Bool rtn = True;
	DOCC_BoxIf* pBox=NULL;
	DOCC_CompositeDoc* pDoc=NULL;
	do
	{
		// Box::newInstance()
		pBox = DOCC_BoxIf::newInstance(pc_ReserveBoxJobList->getCopiedBoxID());
		if( NULL == pBox ){
			RMLOG("★JOBC_ResourceManagerState::deleteAllForCopyEnd pBox=NULL!!★\n",0,0,0,0,0);
			DocRtn = DOCD_RetParameterErr;
			break;
		}
		// Doc::newInstance()
		pDoc = DOCC_CompositeDoc::newInstance();
		if( NULL == pDoc ){
			RMLOG("★JOBC_ResourceManagerState::deleteAllForCopyEnd pDoc=NULL!!★\n",0,0,0,0,0);
			DocRtn = DOCD_RetDocNotExist;
			break;
		}
		// 連結pBox->getDoc()
		DocRtn = pBox->getDoc(pDoc,pc_ReserveBoxJobList->getCopiedDocID());
		if( DOCD_RetOk != DocRtn ){
			RMLOG("★JOBC_ResourceManagerState::deleteAllForCopyEnd getDoc-NG!!★\n",0,0,0,0,0);
			break;
		}
		// Doc::openDoc
		DocRtn = pDoc->openDoc();
		if( DOCD_RetOk != DocRtn ){
			RMLOG("★JOBC_ResourceManagerState::deleteAllForCopyEnd openDoc-NG!!★\n",0,0,0,0,0);
			break;
		}
		// Doc::deleteAll()
		DocRtn = pDoc->deleteAll(pBox);// OpenDoc中必須
		if( DOCD_RetOk != DocRtn ){
			pDoc->closeDoc();
			RMLOG("★JOBC_ResourceManagerState::deleteAllForCopyEnd deleteAll-NG!!★\n",0,0,0,0,0);
			break;
		}
		// Box::deleteInstance()
		pBox->deleteInstance();

	}while(False);

	if( DOCD_RetOk != DocRtn ){
		// ジョブ破棄・システムエラー発生
		if( NULL != pBox) pBox->deleteInstance();
		if( NULL != pDoc) pDoc->deleteInstance();
		setDocAccessErr(pManager,pc_ReserveBoxJobList,DocRtn);
		rtn = False;
	}
	return rtn;
}

//概要    ：callback & deleteList
//作成者  ：sakuragi
//レビュー：未実施（レビュー実施日と名前を記述）
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
void 
JOBC_ResourceManagerState::callback_deleteBoxJobList(
	JOBC_ResourceManager* pManager,
	JOBC_ElementOfReserveBoxJobList* pc_ReserveBoxJobList,
	int32_t sl_result,
	Bool startflg)
{
	// callback
	if(False != startflg){
		pc_ReserveBoxJobList->execReserveEnd(sl_result);
	}
	else{
		pc_ReserveBoxJobList->execCancelCompleted(sl_result);
	}
	// delete_List
	pManager->mReserveList->deleteReservation(pc_ReserveBoxJobList);
	// インスタンス削除
	delete pc_ReserveBoxJobList;
}

//概要    ：Doc_AccessError
//作成者  ：sakuragi
//レビュー：未実施（レビュー実施日と名前を記述）
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
void 
JOBC_ResourceManagerState::setDocAccessErr(
	JOBC_ResourceManager* pManager,
	JOBC_ElementOfReserveBoxJobList* pc_DocErrBoxJobList,
	int32_t sl_result)
{
	// DBGLOG
	RMLOG("★JOBC_ResourceManagerState::setDocAccessErr::DOC-ERR!!(%d)★\n",sl_result,0,0,0,0);
	setRMLOG(this->who(),JOBE_ResourceManagerStateUnknown,JOBD_MSG_BOXJOB_NON,pc_DocErrBoxJobList->getJob(),sl_result);

	Bool b_isRecoverable = False;

	switch(sl_result){
	case DOCD_RetHddFull:				// ディスク容量不足
	case DOCD_RetBoxNotExist:			// ボックスが存在しない
	case DOCD_RetDocNotExist:			// DocFileが存在しない
	case DOCD_RetDocOpened:				// DocFileがオープンされている
		b_isRecoverable = True;
		break;
	default:
		break;
	}

	if( True == b_isRecoverable ){
		// オーバーレイのCompositeDoc解放＆初期化 
		// =>オーバーレイのCompositeDocを先に削除しなければisBindJob()が全てTrueとなるので注意 
		clearOverLayDoc(pManager);

		// ジョブ破棄
		// closeDoc & Doc::deleteInstance()
		if( True == pc_DocErrBoxJobList->isBindJob() ){
			// BindJobは親DocのcloseDocが不要。
			unsigned char num=1;
			while( NULL != pc_DocErrBoxJobList->getBindingDoc(num)){
				pc_DocErrBoxJobList->getBindingDoc(num)->closeDoc();		// CloseDoc
				pc_DocErrBoxJobList->getBindingDoc(num)->deleteInstance();	// Doc::deleteInstance()
				num++;
			}
		}
		else{
			// CloseDoc
			if( NULL != pc_DocErrBoxJobList->getCompositeDoc() ){
				pc_DocErrBoxJobList->getCompositeDoc()->closeDoc();
			}
		}
		if( NULL != pc_DocErrBoxJobList->getCompositeDoc() ){
			// Doc::deleteInstance()
			pc_DocErrBoxJobList->getCompositeDoc()->deleteInstance();
		}
		// callback & deleteList
		callback_deleteBoxJobList(pManager,pc_DocErrBoxJobList,sl_result,True);
		// Driving状態遷移
		changeState(pManager,JOBC_ResourceManagerStateDriving::newInstance());
	}
	else{
		// システムエラー発生
		APIC_setNewDetailCode(CE002_DetailCode0x50F4, makeNewDetailCodeJob01(0));
		APIC_ThrowSystemError(SYSE_SysErrTypeSystemExhausted,LOGF_DEBUG_EVENT_KIND_JOB,
		const_cast<const char *>("JOBC_ResourceManagerState::setDocAccessErr DOC-ERR"));
	}
}

//概要    ：BoxToJobの仕上がりプレビュー用オーバーレイのシリアライズ処理 
//作成者  ：13開発 榎本(SKY) 
//レビュー：未実施（レビュー実施日と名前を記述） 
//変更履歴：変更日、変更者、変更内容（変更理由）を記述 
Bool
JOBC_ResourceManagerState::req_OverLayDeSerialize(
												  JOBC_ResourceManager* pManager)	// ResourceManagerのポインタ 
{
	int32_t slDocRtn = DOCD_RetOk;		// Doc戻り値 
	int32_t	slCnt = 0;					// カウンタ 
	DOCC_BoxIf * pBox = NULL;			// ボックスポインタ 
	DOCC_CompositeDoc * pDoc = NULL;	// ドキュメントポインタ 
	TYPS_Overlay sOverlay;				// オーバーレイ情報 
	TYPC_Overlay cOverlay;				// 登録オーバーレイ情報 

	// 仕上がりプレビューのボックス状態ではない場合、何もしない 
	switch(pManager->getRestoringJob()->getBoxJobState())
	{
	case JOBE_BoxJobState_Preview_Requesting	:	// メモリ展開要求中の場合 
	case JOBE_BoxJobState_Preview_Restoring		:	// メモリ展開中の場合 
	case JOBE_BoxJobState_Preview_Completed		:	// メモリ展開完了の場合 
	case JOBE_BoxJobState_Preview_CancelReq		:	// メモリ破棄要求中の場合 
		break;
	default										:	// その他の場合 
		return False;
	}
	// オーバーレイ情報取得 
	sOverlay = pManager->getRestoringJob()->getJobSpec()->pc_Common->getOverlay();
	// オーバーレイが指定されていない場合、何もしない 
	if(sOverlay.e_Overlay != TYPE_OverlayOn_Load)
	{
		return False;
	}
	// 不揮発から登録オーバーレイ情報を取得 
	// 呼び出しオーバーレイのIDが範囲内(1～101)かどうかをチェック.
	if ((0 < sOverlay.uc_ID) && (NVDD_MAX_OperationSettingOverlay >= sOverlay.uc_ID))
	{
		// 登録オーバーレイ情報を取得.
		APIC_OperationSetting::newInstance()->getOverlay(sOverlay.uc_ID,&cOverlay);
	}

	// オーバーレイのCompositeDoc格納場所を探す 
	for(slCnt=0; slCnt<JOBD_JOBCCATCHCOMPOSITEDOC_MAX; slCnt++)
	{
		// 空きCompositeDocがあった場合 
		if(pManager->getRestoringJob()->getBoxToPrintDoc(slCnt) == NULL)
		{
			break;
		}
	}
	// 空きCompositeDocを取得異常の場合、何もしない 
	if( (slCnt <= 0)								||	// コンポジットDocが何も設定されていない場合 
		(slCnt >= JOBD_JOBCCATCHCOMPOSITEDOC_MAX))		// 空きコンポジットDocがない場合 
	{
		RMLOG("★JOBC_ResourceManagerStateWaitingDeserializeEnd::req_OverlayDeSerialize 空きCompositeDoc取得異常!!(Cnt=%d)★\n",static_cast<int>(slCnt),0,0,0,0);
		return False;
	}
	// 空きページ数チェック(オーバーレイの1ページをメモリ上に展開可能かチェック) 
	if(DOCD_MaxPage - DOCC_Leaf::getUsedLeafInstCnt() <= 0)
	{
		RMLOG("★JOBC_ResourceManagerStateWaitingDeserializeEnd::req_OverlayDeSerialize 空きページ数なし!!★\n",0,0,0,0,0);
		return False;
	}

	// オーバーレイボックスのインスタンスを生成 
	pBox = DOCC_BoxIf::newInstance(DOCD_OverLayBox);
	if( NULL == pBox ){
		RMLOG("★JOBC_ResourceManagerStateWaitingDeserializeEnd::req_OverlayDeSerialize OverLay Box インスタンス生成異常!!★\n",0,0,0,0,0);
		return False;
	}
	// ドキュメントのインスタンス生成 
	pDoc = DOCC_CompositeDoc::newInstance();
	if( NULL == pDoc )
	{
		RMLOG("★JOBC_ResourceManagerStateWaitingDeserializeEnd::req_OverlayDeSerialize OverLar Doc インスタンス生成異常!!★\n",0,0,0,0,0);
		pBox->deleteInstance();											// オーバーレイボックスのインスタンス削除 
		return False;
	}
	// コンポジットドック設定 
	pManager->getRestoringJob()->setBoxToPrintDoc(pDoc, slCnt);
	// 連結pBox->getDoc() 
	slDocRtn = pBox->getDoc(pManager->getRestoringJob()->getBoxToPrintDoc(slCnt), cOverlay.sl_DocID);
	if( DOCD_RetOk != slDocRtn )
	{
		RMLOG("★JOBC_ResourceManagerStateWaitingDeserializeEnd::req_OverlayDeSerialize OverLay Doc getDoc異常!!(Ret=%d)★\n",static_cast<int>(slDocRtn),0,0,0,0);
		pDoc->deleteInstance();											// ドキュメントのインスタンス削除 
		pManager->getRestoringJob()->setBoxToPrintDoc(NULL, slCnt);		// ドキュメント情報削除 
		pBox->deleteInstance();											// オーバーレイボックスのインスタンス削除 
		return False;
	}
	// OpenDoc 
	slDocRtn = pManager->getRestoringJob()->getBoxToPrintDoc(slCnt)->openDoc();
	if((DOCD_RetOk != slDocRtn) && (APID_DOCD_RetDocOpened != slDocRtn))
	{
		RMLOG("★JOBC_ResourceManagerStateWaitingDeserializeEnd::req_OverlayDeSerialize OverLay Doc オープン異常!!(Ret=%d)★\n",static_cast<int>(slDocRtn),0,0,0,0);
		pDoc->deleteInstance();											// ドキュメントのインスタンス削除 
		pManager->getRestoringJob()->setBoxToPrintDoc(NULL, slCnt);		// ドキュメント情報削除 
		pBox->deleteInstance();											// オーバーレイボックスのインスタンス削除 
		return False;
	}
	// オーバーレイボックスのインスタンス削除 
	pBox->deleteInstance();
	// シリアライズ要求 
	slDocRtn = pManager->getRestoringJob()->getBoxToPrintDoc(slCnt)->req_DeSerialize(
		cOverlay.sl_DocID, DOCD_OverLayBox, pManager->ans_DeSerializeForResourceManager, pManager->getRestoringJob());
	if(DOCD_RetOk != slDocRtn)
	{
		RMLOG("★JOBC_ResourceManagerStateWaitingDeserializeEnd::req_OverlayDeSerialize OverLay Doc シリアライズ異常!!(Ret=%d)★\n",static_cast<int>(slDocRtn),0,0,0,0);
		// Docクローズ(クローズ異常の場合、メッセージのみ出力) 
		slDocRtn = pDoc->closeDoc();
		if(DOCD_RetOk != slDocRtn)
		{
			RMLOG("★JOBC_ResourceManagerStateWaitingDeserializeEnd::req_OverlayDeSerialize OverLay Doc クローズ異常!!(Ret=%d)★\n",static_cast<int>(slDocRtn),0,0,0,0);
		}
		pDoc->deleteInstance();											// ドキュメントのインスタンス削除 
		pManager->getRestoringJob()->setBoxToPrintDoc(NULL, slCnt);		// ドキュメント情報削除 
		return False;
	}
	return True;
}

//概要    ：BoxToJobの仕上がりプレビュー用オーバーレイ(裏面）のシリアライズ処理
//作成者  ：11開発 田村
//レビュー：未実施（レビュー実施日と名前を記述） 
//変更履歴：変更日、変更者、変更内容（変更理由）を記述 
Bool
JOBC_ResourceManagerState::req_OverLayDeSerializeBack(
												  JOBC_ResourceManager* pManager)	// ResourceManagerのポインタ 
{
	int32_t slDocRtn = DOCD_RetOk;		// Doc戻り値 
	int32_t	slCnt = 0;					// カウンタ 
	DOCC_BoxIf * pBox = NULL;			// ボックスポインタ 
	DOCC_CompositeDoc * pDoc = NULL;	// ドキュメントポインタ 
	TYPS_Overlay sOverlayBack;			// オーバーレイ情報 
	TYPC_Overlay cOverlayBack;			// 登録オーバーレイ情報 

	// 仕上がりプレビューのボックス状態ではない場合、何もしない 
	switch(pManager->getRestoringJob()->getBoxJobState())
	{
	case JOBE_BoxJobState_Preview_Requesting	:	// メモリ展開要求中の場合 
	case JOBE_BoxJobState_Preview_Restoring		:	// メモリ展開中の場合 
	case JOBE_BoxJobState_Preview_Completed		:	// メモリ展開完了の場合 
	case JOBE_BoxJobState_Preview_CancelReq		:	// メモリ破棄要求中の場合 
		break;
	default										:	// その他の場合 

//		RMLOG("★JOBC_ResourceManagerStateWaitingDeserializeEnd::req_OverlayDeSerializeBack RETURN [getBoxJobState()=%d]★\n",
//								pManager->getRestoringJob()->getBoxJobState(),
//								0,0,0,0);
		return False;
	}

	// オーバーレイ情報取得 
	sOverlayBack = pManager->getRestoringJob()->getJobSpec()->pc_Common->getOverlayBack();

	// オーバーレイが指定されていない場合、何もしない 
	if(sOverlayBack.e_Overlay != TYPE_OverlayOn_Load)
	{
//		RMLOG("★JOBC_ResourceManagerStateWaitingDeserializeEnd::req_OverlayDeSerializeBack RETURN [e_Overlay=%d]★\n",
//								sOverlayBack.e_Overlay,
//								0,0,0,0);
		return False;
	}

	// 不揮発から登録オーバーレイ情報を取得 
	// 呼び出しオーバーレイのIDが範囲内(1～101)かどうかをチェック.
	if ((0 < sOverlayBack.uc_ID) && (NVDD_MAX_OperationSettingOverlay >= sOverlayBack.uc_ID))
	{
		// 登録オーバーレイ情報を取得.
		APIC_OperationSetting::newInstance()->getOverlay(sOverlayBack.uc_ID,&cOverlayBack);
	}

	// オーバーレイのCompositeDoc格納場所を探す 
	for(slCnt=0; slCnt<JOBD_JOBCCATCHCOMPOSITEDOC_MAX; slCnt++)
	{
		// 空きCompositeDocがあった場合 
		if(pManager->getRestoringJob()->getBoxToPrintDoc(slCnt) == NULL)
		{
			break;
		}
	}

	// 空きCompositeDocを取得異常の場合、何もしない 
	if( (slCnt <= 0)								||	// コンポジットDocが何も設定されていない場合 
		(slCnt >= JOBD_JOBCCATCHCOMPOSITEDOC_MAX))		// 空きコンポジットDocがない場合 
	{
		RMLOG("★JOBC_ResourceManagerStateWaitingDeserializeEnd::req_OverlayDeSerializeBack 空きCompositeDoc取得異常!!(Cnt=%d)★\n",static_cast<int>(slCnt),0,0,0,0);
		return False;
	}
	// 空きページ数チェック(オーバーレイの1ページをメモリ上に展開可能かチェック) 
	if(DOCD_MaxPage - DOCC_Leaf::getUsedLeafInstCnt() <= 0)
	{
		RMLOG("★JOBC_ResourceManagerStateWaitingDeserializeEnd::req_OverlayDeSerializeBack 空きページ数なし!!★\n",0,0,0,0,0);
		return False;
	}

	// オーバーレイボックスのインスタンスを生成 
	pBox = DOCC_BoxIf::newInstance(DOCD_OverLayBox);
	if( NULL == pBox ){
		RMLOG("★JOBC_ResourceManagerStateWaitingDeserializeEnd::req_OverlayDeSerializeBack OverLay Box インスタンス生成異常!!★\n",0,0,0,0,0);
		return False;
	}
	// ドキュメントのインスタンス生成 
	pDoc = DOCC_CompositeDoc::newInstance();
	if( NULL == pDoc )
	{
		RMLOG("★JOBC_ResourceManagerStateWaitingDeserializeEnd::req_OverlayDeSerializeBack OverLar Doc インスタンス生成異常!!★\n",0,0,0,0,0);
		pBox->deleteInstance();											// オーバーレイボックスのインスタンス削除 
		return False;
	}
	// コンポジットドック設定 
	pManager->getRestoringJob()->setBoxToPrintDoc(pDoc, slCnt);

	// 登録オーバーレイ裏面情報のみ場合、Indexの2番目に格納する処理.
	if(pManager->getRestoringJob()->getBoxToPrintDoc(1) == NULL) slCnt++;

	// 連結pBox->getDoc() 
	slDocRtn = pBox->getDoc(pManager->getRestoringJob()->getBoxToPrintDoc(slCnt), cOverlayBack.sl_DocID);
	if( DOCD_RetOk != slDocRtn )
	{
		RMLOG("★JOBC_ResourceManagerStateWaitingDeserializeEnd::req_OverlayDeSerializeBack OverLay Doc getDoc異常!!(Ret=%d)★\n",static_cast<int>(slDocRtn),0,0,0,0);
		pDoc->deleteInstance();											// ドキュメントのインスタンス削除 
		pManager->getRestoringJob()->setBoxToPrintDoc(NULL, slCnt);		// ドキュメント情報削除 
		pBox->deleteInstance();											// オーバーレイボックスのインスタンス削除 
		return False;
	}
	// OpenDoc 
	slDocRtn = pManager->getRestoringJob()->getBoxToPrintDoc(slCnt)->openDoc();
	if((DOCD_RetOk != slDocRtn) && (APID_DOCD_RetDocOpened != slDocRtn))
	{
		RMLOG("★JOBC_ResourceManagerStateWaitingDeserializeEnd::req_OverlayDeSerializeBack OverLay Doc オープン異常!!(Ret=%d)★\n",static_cast<int>(slDocRtn),0,0,0,0);
		pDoc->deleteInstance();											// ドキュメントのインスタンス削除 
		pManager->getRestoringJob()->setBoxToPrintDoc(NULL, slCnt);		// ドキュメント情報削除 
		pBox->deleteInstance();											// オーバーレイボックスのインスタンス削除 
		return False;
	}
	// オーバーレイボックスのインスタンス削除 
	pBox->deleteInstance();
	// シリアライズ要求 
	slDocRtn = pManager->getRestoringJob()->getBoxToPrintDoc(slCnt)->req_DeSerialize(
		cOverlayBack.sl_DocID, DOCD_OverLayBox, pManager->ans_DeSerializeForResourceManager, pManager->getRestoringJob());
	if(DOCD_RetOk != slDocRtn)
	{
		RMLOG("★JOBC_ResourceManagerStateWaitingDeserializeEnd::req_OverlayDeSerializeBack OverLay Doc シリアライズ異常!!(Ret=%d)★\n",static_cast<int>(slDocRtn),0,0,0,0);
		// Docクローズ(クローズ異常の場合、メッセージのみ出力) 
		slDocRtn = pDoc->closeDoc();
		if(DOCD_RetOk != slDocRtn)
		{
			RMLOG("★JOBC_ResourceManagerStateWaitingDeserializeEnd::req_OverlayDeSerializeBack OverLay Doc クローズ異常!!(Ret=%d)★\n",static_cast<int>(slDocRtn),0,0,0,0);
		}
		pDoc->deleteInstance();											// ドキュメントのインスタンス削除 
		pManager->getRestoringJob()->setBoxToPrintDoc(NULL, slCnt);		// ドキュメント情報削除 
		return False;
	}
	return True;
}

//概要    ：オーバーレイのCompositeDoc解放＆初期 
//作成者  ：13開発 榎本(SKY) 
//レビュー：未実施（レビュー実施日と名前を記述） 
//変更履歴：変更日、変更者、変更内容（変更理由）を記述 
void 
JOBC_ResourceManagerState::clearOverLayDoc(
										   JOBC_ResourceManager * pManager)	// JOBC_ResourceManagerポインタ 
{
	int32_t slDocRtn = DOCD_RetOk;	// DOC戻り値 

	// ResourceManagerのポインタがNULLの場合、オーバーレイの解放不可 
	if(pManager == NULL)
	{
		RMLOG("★JOBC_ResourceManagerState::clearOverLayDoc ResourceManagerポインタ NULL!!★\n",0,0,0,0,0);
		return;
	}
	// JobのポインタがNULLの場合、オーバーレイの解放不可 
	if(pManager->getRestoringJob() == NULL) 
	{
		RMLOG("★JOBC_ResourceManagerState::clearOverLayDoc Jobポインタ NULL!!★\n",0,0,0,0,0);
		return;
	}
	// 仕上がりプレビューのボックス状態ではない場合、何もしない 
	switch(pManager->getRestoringJob()->getBoxJobState())
	{
	case JOBE_BoxJobState_Preview_Requesting	:	// メモリ展開要求中の場合 
	case JOBE_BoxJobState_Preview_Restoring		:	// メモリ展開中の場合 
	case JOBE_BoxJobState_Preview_Completed		:	// メモリ展開完了の場合 
	case JOBE_BoxJobState_Preview_CancelReq		:	// メモリ破棄要求中の場合 
		break;
	default										:	// その他の場合 
		return;
	}
	// CompositeDoc数ループ 
	for( int32_t slCnt=0; slCnt<JOBD_JOBCCATCHCOMPOSITEDOC_MAX; slCnt++)
	{
		// CompositeDoc取得異常の場合、全てのドキュメントチェック完了とみなし終了 
		if(pManager->getRestoringJob()->getBoxToPrintDoc(slCnt) == NULL)
		{
			break;
		}
		// 指定されたCompositeDocがオーバーレイBoxではない場合、何もしない 
		if(pManager->getRestoringJob()->getBoxToPrintDoc(slCnt)->getBoxID() != DOCD_OverLayBox)
		{
			continue;
		}
		// オーバーレイクローズ 
		slDocRtn = pManager->getRestoringJob()->getBoxToPrintDoc(slCnt)->closeDoc();
		if(slDocRtn != DOCD_RetOk)
		{
			RMLOG("★JOBC_ResourceManagerState::clearOverLayDocOverlayDoc closeDoc NG!!(Rtn=%d)★\n",static_cast<int>(slDocRtn),0,0,0,0);
		}
		// インスタンス削除 
		pManager->getRestoringJob()->getBoxToPrintDoc(slCnt)->deleteInstance();
		// Compositeポインタクリア 
		pManager->getRestoringJob()->setBoxToPrintDoc(NULL, slCnt);
	}
	return;
}

//
//概要　　：waitHddReady
//備考　　：HDDが有る場合、HDD Ready(フォーマット済み)になるまで待ち続ける.
//　　　　：HDDが無い場合、何もしない.
//作成者　：S.Higaki
//レビュー：
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//          2011/07/07 S.Higaki CQ.350243 省電力対応.
void JOBC_ResourceManagerState::waitHddReady()
{
    APIC_HddInfo* pc_HddInfo = APIC_HddInfo::newInstance();
    // HDDが無い場合、何もしない
    if (pc_HddInfo->getHddExist() == False) {
        return;
    }

    // HDDが有る場合
    TYPE_HddStatus e_HddStatus = pc_HddInfo->getHddStatus();
	RMLOG("JOBC_ResourceManagerState::waitHddReady e_HddStatus=%d \n",static_cast<int>(e_HddStatus),0,0,0,0);

    // HDD Ready(フォーマット済み)の場合
    if(e_HddStatus == TYPE_HddStatusFormated) {
        return;
    }

    // HDD Ready(フォーマット済み)でない場合
    // HDD Ready(フォーマット済み)になるまで待ち続ける(max30秒)
    int32_t sl_LoopCnt=0;
    while (true) {
        // HDD Ready(フォーマット済み)になったら終了
        if(pc_HddInfo->getHddStatus() == TYPE_HddStatusFormated) {
            break;
        }
        
        sl_LoopCnt++;
        if((sl_LoopCnt % 600) == 0){
			// 30秒でエラーメッセージ出力.
			RMLOG("★JOBC_ResourceManagerState::waitHddReady timeout error e_HddStatus=%d ★\n",static_cast<int>(e_HddStatus),0,0,0,0);
            break;
		}
        
#ifdef TORNADO
#ifdef LINUX
        SYSF_taskDelay(std::max((SYSF_sysClkRateGet() * 50) / 1000, 1));
#else
        SYSF_taskDelay(max((SYSF_sysClkRateGet() * 50) / 1000, 1));
#endif // LINUX        
#endif
    }
    return;
}

// [changelog] 2017/12/20 AR1029174 khueqt Supported SegF NewDetailCode for JOBC_RMState IT5_4.2 start.
uchar getBoxDetailCode(JOBC_Job* pJob)
{
	uchar uc_boxIdConverted = 0;
	
	switch (pJob->getJobSpec()->pc_Common->c_JobType.getBoxNo())
	{
		
		case DOCD_FunctionBoxID:
		{
			uc_boxIdConverted = 1;
			break;
		}
		case DOCD_FaxKmrdsBox:
		{
			uc_boxIdConverted = 2;
			break;
		}
		case DOCD_FaxSndBox:
		{
			uc_boxIdConverted = 3;
			break;
		}
		case DOCD_FaxRcvBox:
		{
			uc_boxIdConverted = 4;
			break;
		}
		case DOCD_ManualForwardBox:
		{
			uc_boxIdConverted = 5;
			break;
		}
		case DOCD_PdlRcvBox:
		{
			uc_boxIdConverted = 6;
			break;
		}
		case DOCD_IFaxSndBox:
		{
			uc_boxIdConverted = 7;
			break;
		}
		case DOCD_FaxReportBox:
		{
			uc_boxIdConverted = 8;
			break;
		}
		case DOCD_PCFaxSndBox:
		{
			uc_boxIdConverted = 9;
			break;
		}
		case DOCD_FaxForwardBox:
		{
			uc_boxIdConverted = 10;
			break;
		}
		case DOCD_OverLayBox:
		{
			uc_boxIdConverted = 11;
			break;
		}
		case DOCD_IFaxRcvBox:
		{
			uc_boxIdConverted = 12;
			break;
		}
		case DOCD_TSIFaxBox:
		{
			uc_boxIdConverted = 13;
			break;
		}
		case DOCD_PDFBox:
		{
			uc_boxIdConverted = 14;
			break;
		}
		case DOCD_ProgramJobBox:
		{
			uc_boxIdConverted = 15;
			break;
		}
		case DOCD_TouchAndGoBox:
		{
			uc_boxIdConverted = 16;
			break;
		}
		case DOCD_GeneralDocBox:
		{
			uc_boxIdConverted = 17;
			break;
		}
		case DOCD_ImageLogBox:
		{
			uc_boxIdConverted = 18;
			break;
		}
		case DOCD_ImageDealerBox:
		{
			uc_boxIdConverted = 19;
			break;
		}
		case DOCD_PDFDigitalIdBox:
		{
			uc_boxIdConverted = 20;
			break;
		}
		case DOCD_EncFile_BoxId:
		{
			uc_boxIdConverted = 21;
			break;
		}
		case DOCD_ImageInsertionBox:
		{
			uc_boxIdConverted = 22;
			break;
		}
		case DOCD_UbiquitousBox:
		{
			uc_boxIdConverted = 23;
			break;
		}
		case DOCD_ScanToURLBox:
		{
			uc_boxIdConverted = 24;
			break;
		}
		case DOCD_SystemBoxID:
		{
			uc_boxIdConverted = 30;
			break;
		}
		case DOCD_RevivalSndBox:
		{
			uc_boxIdConverted = 31;
			break;
		}
		case DOCD_PollingSndBox:
		{
			uc_boxIdConverted = 32;
			break;
		}
		case DOCD_ConfirmationPrintBox:
		{
			uc_boxIdConverted = 33;
			break;
		}
		case DOCD_SecretDocumentsBox:
		{
			uc_boxIdConverted = 34;
			break;
		}
		case DOCD_PinPrintBox:
		{
			uc_boxIdConverted = 35;
			break;
		}
		case DOCD_CompulsionRcvBox:
		{
			uc_boxIdConverted = 0;
			break;
		}
		default:
		{
			uc_boxIdConverted = 0xFF;
			break;
		}
	}
	
	return uc_boxIdConverted;
}
// [changelog] 2017/12/20 AR1029174 khueqt Supported SegF NewDetailCode for JOBC_RMState IT5_4.2 end.

// < class JOBC_ResourceManagerStateDriving >------------------------------------------
// class JOBC_ResourceManagerStateDriving : public JOBC_ResourceManagerState

//概要    ：インスタンス生成
//作成者  ：sakuragi
//レビュー：未実施（レビュー実施日と名前を記述）
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
JOBC_ResourceManagerStateDriving*
JOBC_ResourceManagerStateDriving::newInstance()
{
	static JOBC_ResourceManagerStateDriving* pInstance;
	if( NULL == pInstance ){
		try
		{
			pInstance = new JOBC_ResourceManagerStateDriving;
		}
		catch(...)
		{
			APIC_setFatalDetailCodeE002(CE002_DetailCode0x50F0);
			APIC_ThrowSystemError(SYSE_SysErrTypeSystemExhausted,LOGF_DEBUG_EVENT_KIND_JOB,
				const_cast<const char *>("JOBC_ResourceManagerStateDriving インスタンス生成失敗"));
			return NULL;
		}
	}
	return pInstance;
}

//概要    ：状態に入ったときに必ず行う処理
//作成者  ：sakuragi
//レビュー：未実施（レビュー実施日と名前を記述）
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
void
JOBC_ResourceManagerStateDriving::onEntry(
	JOBC_ResourceManager* pManager)
{
	if(RM_State<=RMDBG) RMLOG("★JOBC_ResourceManagerStateDriving::onEntry★\n",0,0,0,0,0);

	// Clear
	pManager->clearRestoringJobInfo();
	// Get_FirstList
	JOBC_ElementOfReserveBoxJobList* pc_FirstBoxJobList 
		= pManager->mReserveList->getFirstReservation();
	// CheckList
	if( NULL == pc_FirstBoxJobList ) return;

	// Copy判定
	if( True == checkDocumentCopy( pc_FirstBoxJobList->getJob()) ){
		// WaitingDocumentCopyEnd状態遷移
		changeState(pManager,JOBC_ResourceManagerStateWaitingDocumentCopyEnd::newInstance());
	}
	else{
		if( True == checkDocumentPage(pManager,pc_FirstBoxJobList->getJob()) ){
			// WaitingDocRestore状態遷移
			changeState(pManager,JOBC_ResourceManagerStateWaitingDocRestore::getStateOnEntry(pManager));
		}
		else{
			// WaitingForRetryTimer状態遷移
			changeState(pManager,JOBC_ResourceManagerStateWaitingForRetryTimer::newInstance());
		}
	}
}

//概要    ：状態を抜けるときに必ず行う処理
//作成者  ：sakuragi
//レビュー：未実施（レビュー実施日と名前を記述）
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
void
JOBC_ResourceManagerStateDriving::onExit(
	JOBC_ResourceManager* pManager)
{
	if(RM_State<=RMDBG) RMLOG("★JOBC_ResourceManagerStateDriving::onExit★\n",0,0,0,0,0);

	// Get_FirstList
	JOBC_ElementOfReserveBoxJobList* pc_FirstBoxJobList 
		= pManager->mReserveList->getFirstReservation();
	if( NULL == pc_FirstBoxJobList ) return;
	// 展開中ジョブ設定
	pManager->setRestoringJobInfo(pc_FirstBoxJobList->getJob());
}

//概要    ：BoxJob予約指示
//作成者  ：sakuragi
//レビュー：未実施（レビュー実施日と名前を記述）
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
void
JOBC_ResourceManagerStateDriving::evtReserveBoxJob(
	JOBC_ResourceManager* pManager,
	JOBC_Job* pJob)
{
	if(RM_State<=RMDBG) RMLOG("★JOBC_ResourceManagerStateDriving::evtReserveBoxJob★\n",0,0,0,0,0);

	// add_List
	if( False == addListReserveBoxJob(pManager,pJob)){
		return;
	}
	// Copy判定
	if( True == checkDocumentCopy(pJob) ){
		// WaitingDocumentCopyEnd状態遷移
		changeState(pManager,JOBC_ResourceManagerStateWaitingDocumentCopyEnd::newInstance());
	}
	else{
		if( True == checkDocumentPage(pManager,pJob) ){
			// WaitingDocRestore状態遷移
			changeState(pManager,JOBC_ResourceManagerStateWaitingDocRestore::getStateOnEntry(pManager));
		}
		else{
			// WaitingForRetryTimer状態遷移
			changeState(pManager,JOBC_ResourceManagerStateWaitingForRetryTimer::newInstance());
		}
	}
}

// < class JOBC_ResourceManagerStateWaitingForRetryTimer >-----------------------------
// class JOBC_ResourceManagerStateWaitingForRetryTimer : public JOBC_ResourceManagerState

//概要    ：インスタンス生成
//作成者  ：sakuragi
//レビュー：未実施（レビュー実施日と名前を記述）
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
JOBC_ResourceManagerStateWaitingForRetryTimer*
JOBC_ResourceManagerStateWaitingForRetryTimer::newInstance()
{
	static JOBC_ResourceManagerStateWaitingForRetryTimer* pInstance;
	if( NULL == pInstance ){
		try
		{
			pInstance = new JOBC_ResourceManagerStateWaitingForRetryTimer;
		}
		catch(...)
		{
			APIC_setFatalDetailCodeE002(CE002_DetailCode0x50ED);
			APIC_ThrowSystemError(SYSE_SysErrTypeSystemExhausted,LOGF_DEBUG_EVENT_KIND_JOB,
				const_cast<const char *>("JOBC_ResourceManagerStateWaitingForRetryTimer インスタンス生成失敗"));
			return NULL;
		}
	}
	return pInstance;
}

//概要    ：状態に入ったときに必ず行う処理
//作成者  ：sakuragi
//レビュー：未実施（レビュー実施日と名前を記述）
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
void
JOBC_ResourceManagerStateWaitingForRetryTimer::onEntry(
	JOBC_ResourceManager* pManager)
{
	if(RM_State<=RMDBG) RMLOG("★JOBC_ResourceManagerStateWaitingForRetryTimer::onEntry★\n",0,0,0,0,0);

	// Timer_Start
	pManager->mTimerForRestoreWait->start();
}

//概要    ：BoxJob予約破棄指示
//作成者  ：sakuragi
//レビュー：未実施（レビュー実施日と名前を記述）
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
// 2005/11/24 Kosuge,T DocFileを消すかどうかを上位から指定できるように変更。
void
JOBC_ResourceManagerStateWaitingForRetryTimer::evtCancelBoxJobReserve(
	JOBC_ResourceManager* pManager,
	JOBC_Job* pJob,
	Bool b_deleteDocFile)
{
	if(RM_State<=RMDBG) RMLOG("★JOBC_ResourceManagerStateWaitingForRetryTimer::evtCancelBoxJobReserve★\n",0,0,0,0,0);

	JOBC_ElementOfReserveBoxJobList* pc_CancelBoxJobList =
		pManager->mReserveList->searchAndGetReservation(pJob);
	if( NULL == pc_CancelBoxJobList ) return;

	// 予約削除後DocFileを消すかどうかを設定(上位の指示通り動かす)
	pc_CancelBoxJobList->decideDocFileDelete(b_deleteDocFile);

	// Copy判定
	if( pJob == pManager->getRestoringJob() &&
	    True == checkDocumentCopy( pc_CancelBoxJobList->getJob()) ){
		// Copy完了
		// Doc::deleteAll()
		if( False == deleteAllForCopyEnd(pManager,pc_CancelBoxJobList)){
			return;
		}
	}

	// DeleteDocument判定
	if( False == pc_CancelBoxJobList->checkDocFileDelete() ){
		// closeDoc & Doc::deleteInstance()
		if( False == closeDoc_deleteInstance(pManager,pc_CancelBoxJobList)){
			return;
		}
	}
	else{
		// Doc::deleteAll()
		if( False == deleteAll(pManager,pc_CancelBoxJobList)){
			return;
		}
	}
	// callback & deleteList
	callback_deleteBoxJobList(pManager,pc_CancelBoxJobList,JOBD_ReserveBoxJob_OK);

	if( pJob == pManager->getRestoringJob() ){
		// 処理中ジョブの場合
		pManager->mTimerForRestoreWait->stop();
		// Driving状態遷移
		changeState(pManager,JOBC_ResourceManagerStateDriving::newInstance());
		// DBGLOG
		if(RM_End<=RMDBG) RMLOG("★JOBC_ResourceManagerStateWaitingForRetryTimer::JOB-CANCEL!!★\n",0,0,0,0,0);
	}
}

//概要    ：タイマー満了通知
//作成者  ：sakuragi
//レビュー：未実施（レビュー実施日と名前を記述）
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
void
JOBC_ResourceManagerStateWaitingForRetryTimer::evtTimerExpired(
	JOBC_ResourceManager* pManager)
{
	if(RM_State<=RMDBG) RMLOG("★JOBC_ResourceManagerStateWaitingForRetryTimer::evtTimerExpired★\n",0,0,0,0,0);

	if( True == checkDocumentPage(pManager,pManager->getRestoringJob()) ){
		// WaitingDocRestore状態遷移
		changeState(pManager,JOBC_ResourceManagerStateWaitingDocRestore::getStateOnEntry(pManager));
	}
	else{
		// Timer_ReStart
		pManager->mTimerForRestoreWait->start();
	}
}

// < class JOBC_ResourceManagerStateWaitingDocumentCopyEnd >---------------------------
// class JOBC_ResourceManagerStateWaitingDocumentCopyEnd : public JOBC_ResourceManagerState

//概要    ：インスタンス生成
//作成者  ：sakuragi
//レビュー：未実施（レビュー実施日と名前を記述）
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
JOBC_ResourceManagerStateWaitingDocumentCopyEnd*
JOBC_ResourceManagerStateWaitingDocumentCopyEnd::newInstance()
{
	static JOBC_ResourceManagerStateWaitingDocumentCopyEnd* pInstance;
	if( NULL == pInstance ){
		try
		{
			pInstance = new JOBC_ResourceManagerStateWaitingDocumentCopyEnd;
		}
		catch(...)
		{
			APIC_setFatalDetailCodeE002(CE002_DetailCode0x50EE);
			APIC_ThrowSystemError(SYSE_SysErrTypeSystemExhausted,LOGF_DEBUG_EVENT_KIND_JOB,
				const_cast<const char *>("JOBC_ResourceManagerStateWaitingDocumentCopyEnd インスタンス生成失敗"));
			return NULL;
		}
	}
	return pInstance;
}

//概要    ：状態に入ったときに必ず行う処理
//作成者  ：sakuragi
//レビュー：未実施（レビュー実施日と名前を記述）
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
void
JOBC_ResourceManagerStateWaitingDocumentCopyEnd::onEntry(JOBC_ResourceManager* pManager)
{
	if(RM_State<=RMDBG) RMLOG("★JOBC_ResourceManagerStateWaitingDocumentCopyEnd::onEntry★\n",0,0,0,0,0);

	// Copy実施(req_changeDocFilePosition)
	if( False == moveDocFile(pManager,pManager->getRestoringJob())){
		return;
	}
}

//概要    ：BoxJob予約破棄指示
//作成者  ：sakuragi
//レビュー：未実施（レビュー実施日と名前を記述）
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
// 2005/11/24 Kosuge,T DocFileを消すかどうかを上位から指定できるように変更。
void
JOBC_ResourceManagerStateWaitingDocumentCopyEnd::evtCancelBoxJobReserve(
	JOBC_ResourceManager* pManager,
	JOBC_Job* pJob,
	Bool b_deleteDocFile)
{
	if(RM_State<=RMDBG) RMLOG("★JOBC_ResourceManagerStateWaitingDocumentCopyEnd::evtCancelBoxJobReserve★\n",0,0,0,0,0);

	// 状態遷移(処理中ジョブ・指定ジョブチェック)
	evtCancelBoxJobReserveByJob(pManager,pJob,b_deleteDocFile);
}

//概要    ：DocFileCopy完了通知
//作成者  ：sakuragi
//レビュー：未実施（レビュー実施日と名前を記述）
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
void
JOBC_ResourceManagerStateWaitingDocumentCopyEnd::evtAnsChangeDocFilePosition(
	JOBC_ResourceManager* pManager,
	int32_t sl_result)
{
	if(RM_State<=RMDBG) RMLOG("★JOBC_ResourceManagerStateWaitingDocumentCopyEnd::evtAnsChangeDocFilePosition★\n",0,0,0,0,0);

	JOBC_ElementOfReserveBoxJobList* pc_CopyEndBoxJobList =
		pManager->mReserveList->searchAndGetReservation(pManager->getRestoringJob());
	if( NULL == pc_CopyEndBoxJobList ) return;

	if( DOCD_RetOk != sl_result ){
		// Copy失敗(ジョブ破棄)
		setDocAccessErr(pManager,pc_CopyEndBoxJobList,sl_result);
	}
	else{
		// Copy成功
		if( True == checkDocumentPage(pManager,pManager->getRestoringJob()) ){
			// WaitingDocRestore状態遷移
			changeState(pManager,JOBC_ResourceManagerStateWaitingDocRestore::getStateOnEntry(pManager));
		}
		else{
			// WaitingForRetryTimer状態遷移
			changeState(pManager,JOBC_ResourceManagerStateWaitingForRetryTimer::newInstance());
		}
	}
}

//概要    ：DocumentFileCopy実施
//作成者  ：sakuragi
//レビュー：未実施（レビュー実施日と名前を記述）
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
Bool 
JOBC_ResourceManagerStateWaitingDocumentCopyEnd::moveDocFile(
	JOBC_ResourceManager* pManager,
	JOBC_Job* pJob)
{
	int32_t DocRtn = DOCD_RetOk;
	Bool rtn = True;
	DOCC_BoxIf* pMoveBox=NULL;
	JOBC_ElementOfReserveBoxJobList* pc_CopyEndBoxJobList =
			pManager->mReserveList->searchAndGetReservation(pManager->getRestoringJob());
	if( NULL == pc_CopyEndBoxJobList ) return False;

	do
	{
		// DocFile格納位置変更情報テーブル
		DOCS_DocFilePositionTbl s_DocFilePosTbl = {};
		memset(&s_DocFilePosTbl,0x00,sizeof(DOCS_DocFilePositionTbl));
		// モードをコピーに設定
		s_DocFilePosTbl.state = DOCD_DocFile_Copy;
		// 移動元の情報設定
		s_DocFilePosTbl.preInfo.boxId = pJob->getJobSpec()->pc_Common->c_JobType.getBoxNo();
		int32_t sl_DocID[TYPD_MaxProgramJob];
		memset(sl_DocID,0x00,sizeof(sl_DocID));
		pJob->getJobSpec()->pc_Common->c_JobType.getDocumentNo(sl_DocID);
		s_DocFilePosTbl.preInfo.docId = sl_DocID[0];

		// DOCC_Boxインスタンス生成
		pMoveBox = DOCC_BoxIf::newInstance(DOCD_FaxSndBox);
		if( NULL == pMoveBox ){
			RMLOG("★JOBC_ResourceManagerStateWaitingDocumentCopyEnd::moveDocFile pMoveBox=NULL!!★\n",0,0,0,0,0);
			DocRtn = DOCD_RetParameterErr;
			break;
		}
		// DOCD_FaxSndBoxの存在判定
		DocRtn = pMoveBox->getExistBox();
		if( DOCD_Exist != DocRtn ){
			RMLOG("★JOBC_ResourceManagerStateWaitingDocumentCopyEnd::moveDocFile getExistBox-NG!!★\n",0,0,0,0,0);
			break;
		}
		// DOCC_CompositeDocインスタンス生成
		DOCC_CompositeDoc* pMoveDocLocal = DOCC_CompositeDoc::newInstance();
		if( NULL == pMoveDocLocal ){
			RMLOG("★JOBC_ResourceManagerStateWaitingDocumentCopyEnd::moveDocFile pMoveDocLocal=NULL!!★\n",0,0,0,0,0);
			DocRtn = DOCD_RetDocNotExist;
			break;
		}
		// DocID取得
		int32_t sl_newDocID = pMoveBox->getDocID();
		if( sl_newDocID <= DOCD_RetOk ){
			RMLOG("★JOBC_ResourceManagerStateWaitingDocumentCopyEnd::moveDocFile getDocID-NG!!★\n",0,0,0,0,0);
			DocRtn = sl_newDocID;
			break;
		}
		// DOCC_CompositeDocインスタンスにDocID設定
		DocRtn = pMoveDocLocal->setCompositeDocID(sl_newDocID);
		if( DOCD_RetOk != DocRtn){
			RMLOG("★JOBC_ResourceManagerStateWaitingDocumentCopyEnd::moveDocFile setCompositeDocID-NG!!★\n",0,0,0,0,0);
			break;
		}
		// 空DocFile作成
		DocRtn = pMoveBox->addDoc(pMoveDocLocal);
		if( DOCD_RetOk != DocRtn){
			RMLOG("★JOBC_ResourceManagerStateWaitingDocumentCopyEnd::moveDocFile addDoc-NG!!★\n",0,0,0,0,0);
			break;
		}
		// 移動先の情報設定
		s_DocFilePosTbl.nextInfo.boxId = DOCD_FaxSndBox;
		s_DocFilePosTbl.nextInfo.docId = pMoveDocLocal->getCompositeDocID();
		// 移動先のDocIDをジョブのメンバに保持する
		pJob->setMovedDocID(pMoveDocLocal->getCompositeDocID());
		// 予約情報にCopy先DocIDをセット
		if(pc_CopyEndBoxJobList != NULL) pc_CopyEndBoxJobList->setCopiedDocID(pMoveDocLocal->getCompositeDocID());

		// DocFile移動設定が完了したのでBoxとCompositeDocのインスタンスは削除する
		// DOCC_Boxインスタンス削除
		pMoveBox->deleteInstance();
		// DOCC_Docインスタンス削除
		pMoveDocLocal->deleteInstance();

		// DOCD_FaxSndBoxにCopy実施
		DocRtn = DOCC_BoxIf::req_changeDocFilePosition(
			&s_DocFilePosTbl,
			pManager->ans_changeDocFilePositionForResourceManager,
			reinterpret_cast<void*>(pJob));
		if( DOCD_RetOk != DocRtn ){
			RMLOG("★JOBC_ResourceManagerStateWaitingDocumentCopyEnd::moveDocFile req_changeDocFilePosition-NG!!★\n",0,0,0,0,0);
			break;
		}
	
	}while(False);

	if( DOCD_RetOk != DocRtn ){
		// DOCC_Boxインスタンス削除
		if( NULL != pMoveBox) pMoveBox->deleteInstance();
		// ジョブ破棄・システムエラー発生
		setDocAccessErr(pManager,pc_CopyEndBoxJobList,DocRtn);
		rtn = False;
	}
	return rtn;
}

// < class JOBC_ResourceManagerStateWaitingDocRestore >--------------------------------
// class JOBC_ResourceManagerStateWaitingDocRestore : public JOBC_ResourceManagerState

//概要    ：本状態内での遷移先取得
//作成者  ：sakuragi
//レビュー：未実施（レビュー実施日と名前を記述）
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
JOBC_ResourceManagerState*
JOBC_ResourceManagerStateWaitingDocRestore::getStateOnEntry(JOBC_ResourceManager* pManager)
{
	// Copy判定
	JOBC_ElementOfReserveBoxJobList* pc_FirstBoxJobList 
		= pManager->mReserveList->getFirstReservation();
	if( NULL == pc_FirstBoxJobList ) return NULL;

	return ( True == checkDocumentCopy( pc_FirstBoxJobList->getJob()) ) ? 
	reinterpret_cast<JOBC_ResourceManagerState*>(JOBC_ResourceManagerStateWaitingGetInstanceEnd::newInstance()) : 
	reinterpret_cast<JOBC_ResourceManagerState*>(JOBC_ResourceManagerStateWaitingDeserializeEnd::newInstance());
}

//概要    ：BoxJob予約破棄指示
//作成者  ：sakuragi
//レビュー：未実施（レビュー実施日と名前を記述）
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
// 2005/11/24 Kosuge,T DocFileを消すかどうかを上位から指定できるように変更。
void
JOBC_ResourceManagerStateWaitingDocRestore::evtCancelBoxJobReserve(
	JOBC_ResourceManager* pManager,
	JOBC_Job* pJob,
	Bool b_deleteDocFile)
{
	if(RM_State<=RMDBG) RMLOG("★JOBC_ResourceManagerStateWaitingDocRestore::evtCancelBoxJobReserve★\n",0,0,0,0,0);

	// 状態遷移(処理中ジョブ・指定ジョブチェック)
	evtCancelBoxJobReserveByJob(pManager,pJob,b_deleteDocFile);
}

// < class JOBC_ResourceManagerStateWaitingDeserializeEnd >--------------------------------
// class JOBC_ResourceManagerStateWaitingDeserializeEnd : public JOBC_ResourceManagerState

//概要    ：インスタンス生成
//作成者  ：sakuragi
//レビュー：未実施（レビュー実施日と名前を記述）
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
JOBC_ResourceManagerStateWaitingDeserializeEnd*
JOBC_ResourceManagerStateWaitingDeserializeEnd::newInstance()
{
	static JOBC_ResourceManagerStateWaitingDeserializeEnd* pInstance;
	if( NULL == pInstance ){
		try
		{
			pInstance = new JOBC_ResourceManagerStateWaitingDeserializeEnd;
		}
		catch(...)
		{
			APIC_setFatalDetailCodeE002(CE002_DetailCode0x50EF);
			APIC_ThrowSystemError(SYSE_SysErrTypeSystemExhausted,LOGF_DEBUG_EVENT_KIND_JOB,
				const_cast<const char *>("JOBC_ResourceManagerStateWaitingDeserializeEnd インスタンス生成失敗"));
			return NULL;
		}
	}
	return pInstance;
}

//概要    ：状態に入ったときに必ず行う処理
//作成者  ：sakuragi
//レビュー：未実施（レビュー実施日と名前を記述）
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
void
JOBC_ResourceManagerStateWaitingDeserializeEnd::onEntry(JOBC_ResourceManager* pManager)
{
	if(RM_State<=RMDBG) RMLOG("★JOBC_ResourceManagerStateWaitingDeserializeEnd::onEntry\n",0,0,0,0,0);

	// インスタンス化実施(req_DeSerialize)
	if( False == req_DeSerialize(pManager)){
		return;
	}
}

//概要    ：CompositeDoc生成完了通知
//作成者  ：sakuragi
//レビュー：未実施（レビュー実施日と名前を記述）
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
void
JOBC_ResourceManagerStateWaitingDeserializeEnd::evtAnsDeserializeDoc(
	JOBC_ResourceManager* pManager,
	int32_t sl_result)
{
	JOBC_ElementOfReserveBoxJobList* pc_StartBoxJobList =
		pManager->mReserveList->searchAndGetReservation(pManager->getRestoringJob());
	if( NULL == pc_StartBoxJobList ) return;

	if(RM_State<=RMDBG) RMLOG("★JOBC_ResourceManagerStateWaitingDeserializeEnd::evtAnsDeserializeDoc★\n",0,0,0,0,0);

	// 展開済みドキュメント数を更新する前で全ドキュメント数と展開済みのドキュメント数が一致した場合、 
	// オーバーレイの要求のAnsが戻ってきたとみなす 
	if(pManager->getNumOfDocNeeded() <= pManager->getNumOfDocRestored())
	{
		// オーバーレイのシリアライズが異常の場合 
		if( DOCD_RetOk != sl_result )
		{
			RMLOG("★JOBC_ResourceManagerStateWaitingDeserializeEnd(Overlay)::OverLay Serialize Err!!(Ret=%d)★\n",static_cast<int>(sl_result),0,0,0,0);
			// オーバーレイを解除する(面倒な為、Timerにはいれない) 
			clearOverLayDoc(pManager);
		}

		// DOC数更新
		pManager->incrementNumOfDocRestored();

		if(RM_State<=RMDBG){
			RMLOG("★JOBC_ResourceManagerStateWaitingDeserializeEnd(Overlay)::OverLay_1 NumOfDocNeeded()=[%d],NumOfDocRestored()=[%d]★\n",
									pManager->getNumOfDocNeeded(),
									pManager->getNumOfDocRestored(),
									0,0,0);
		}

		// バインドあり時、リストアカウンター(sl_Cnt)は、バインド数＋オーバーレイ画像表面、裏面の2とする。
		// バインドなし時、リストアカウンター(sl_Cnt)は、1文書＋オーバーレイ画像表面、裏面の2とする。
		int32_t	sl_Cnt = 0;
		
		if( True == pc_StartBoxJobList->isBindJob() ){
			sl_Cnt = pManager->getNumOfDocNeeded()+2;
		}
		else{
			sl_Cnt = JOBD_OVERLAY_RESTORE_MAX;
		}

		// オーバーレイ表面＋裏面情報設定で、リストア未完了の判定
		if(pManager->getNumOfDocRestored() < sl_Cnt)
		{
			// オーバーレイ表面情報設定なしの場合、裏面のデシリアライズ処理を行う。情報設定なしの場合、何もしない。
			if(req_OverLayDeSerializeBack(pManager) == True)
			{
				if(RM_State<=RMDBG){
					RMLOG("★JOBC_ResourceManagerStateWaitingDeserializeEnd(OverlayBack)::OverLay_2 NumOfDocNeeded()=[%d],NumOfDocRestored()=[%d]★\n",
											pManager->getNumOfDocNeeded(),
											pManager->getNumOfDocRestored(),
											0,0,0);
				}
				return;
			}
		}

		if(RM_State<=RMDBG){
			RMLOG("★JOBC_ResourceManagerStateWaitingDeserializeEnd(Overlay)::OverLay_3 NumOfDocNeeded()=[%d],NumOfDocRestored()=[%d] sl_Cnt=[%d] ★\n",
									pManager->getNumOfDocNeeded(),
									pManager->getNumOfDocRestored(),
									sl_Cnt,
									0,0);
		}


		// callback & deleteList 
		callback_deleteBoxJobList(pManager,pc_StartBoxJobList,JOBD_ReserveBoxJob_OK,True);
		// Driving状態遷移 
		changeState(pManager,JOBC_ResourceManagerStateDriving::newInstance());
		// DBGLOG 
		if(RM_End<=RMDBG) RMLOG("★JOBC_ResourceManagerStateWaitingDeserializeEnd(Overlay)::JOBSinario-START!!★\n",0,0,0,0,0);
		return;
	}

	if( DOCD_RetOk != sl_result ){
		// WaitingForRetryTimer状態遷移
		changeState(pManager,JOBC_ResourceManagerStateWaitingForRetryTimer::newInstance());
	}
	else{
		// DOC数更新
		pManager->incrementNumOfDocRestored();

		if(RM_State<=RMDBG){
			RMLOG("★JOBC_ResourceManagerStateWaitingDeserializeEnd(Overlay)::OverLay_4 NumOfDocNeeded()=[%d],NumOfDocRestored()=[%d]★\n",
									pManager->getNumOfDocNeeded(),
									pManager->getNumOfDocRestored(),
									0,0,0);
		}

		if( pManager->getNumOfDocNeeded() == pManager->getNumOfDocRestored() ){
			// 展開終了

			if( True == pc_StartBoxJobList->isBindJob() ){
				// Bind-連結
				unsigned char num=1;
				while( NULL != pc_StartBoxJobList->getBindingDoc(num)){
					int32_t DocRtn = pc_StartBoxJobList->getCompositeDoc()->addByPtr(pc_StartBoxJobList->getBindingDoc(num));
					if( DOCD_RetOk != DocRtn ){
						RMLOG("★JOBC_ResourceManagerStateWaitingDeserializeEnd::evtAnsDeserializeDoc addByPtr-NG!!★\n",0,0,0,0,0);
						// ジョブ破棄・システムエラー発生
						setDocAccessErr(pManager,pc_StartBoxJobList,DocRtn);
						return;
					}
					num++;
				}
			}
			// BoxToJobの仕上がりプレビューにてオーバーレイの設定がされている場合はもう一回シリアライズする 
			if(req_OverLayDeSerialize(pManager) == True)
			{
				if(RM_State<=RMDBG){
					RMLOG("★JOBC_ResourceManagerStateWaitingDeserializeEnd(Overlay)::OverLay_5 NumOfDocNeeded()=[%d],NumOfDocRestored()=[%d]★\n",
											pManager->getNumOfDocNeeded(),
											pManager->getNumOfDocRestored(),
											0,0,0);
				}
				return;
			}

			// オーバーレイ表面情報設定なしの場合、裏面のデシリアライズ処理を行う。情報設定なしの場合、何もしない。
			if(req_OverLayDeSerializeBack(pManager) == True)
			{
				if(RM_State<=RMDBG){
					RMLOG("★JOBC_ResourceManagerStateWaitingDeserializeEnd(OverlayBack)::OverLay_6 NumOfDocNeeded()=[%d],NumOfDocRestored()=[%d]★\n",
											pManager->getNumOfDocNeeded(),
											pManager->getNumOfDocRestored(),
											0,0,0);
				}
				return;
			}


			// callback & deleteList
			callback_deleteBoxJobList(pManager,pc_StartBoxJobList,JOBD_ReserveBoxJob_OK,True);
			// Driving状態遷移
			changeState(pManager,JOBC_ResourceManagerStateDriving::newInstance());
			// DBGLOG
			if(RM_End<=RMDBG) RMLOG("★JOBC_ResourceManagerStateWaitingDeserializeEnd::JOBSinario-START!!★\n",0,0,0,0,0);
		}
		else{
			// 展開未終了
			if( True == checkDocumentPage(pManager,pManager->getRestoringJob())){
				// インスタンス化実施(req_DeSerialize)
				if( False == req_DeSerialize(pManager)){
					return;
				}
			}
			else{
				// WaitingForRetryTimer状態遷移
				changeState(pManager,JOBC_ResourceManagerStateWaitingForRetryTimer::newInstance());
			}
		}
	}
}

//概要    ：インスタンス化実施(req_DeSerialize).
//作成者  ：sakuragi.
//レビュー：未実施（レビュー実施日と名前を記述）.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
// 			2011.01.27 日立JTE亀山 CQ336075 Coverity5(NULL間接参照対応).
Bool 
JOBC_ResourceManagerStateWaitingDeserializeEnd::req_DeSerialize(
	JOBC_ResourceManager* pManager)
{
	int32_t DocRtn = DOCD_RetOk;
	Bool rtn = True;

	int32_t sl_DocID[TYPD_MaxProgramJob];

	for( int i = 0 ; i < TYPD_MaxProgramJob ; i++ ){
		sl_DocID[i] = TYPD_NotSetting;
	}
	
	
	pManager->getRestoringJob()->getJobSpec()->pc_Common->c_JobType.getDocumentNo(sl_DocID);

	JOBC_ElementOfReserveBoxJobList* pc_DeSerializeBoxJobList =
		pManager->mReserveList->searchAndGetReservation(pManager->getRestoringJob());
	if( NULL == pc_DeSerializeBoxJobList ) return False;

	// インスタンス化実施(req_DeSerialize)
	if( False == pc_DeSerializeBoxJobList->isBindJob() ){
		// NoBind
		DocRtn = pc_DeSerializeBoxJobList->getCompositeDoc()->req_DeSerialize(
					sl_DocID[0],
					pManager->getRestoringJob()->getJobSpec()->pc_Common->c_JobType.getBoxNo(),
					pManager->ans_DeSerializeForResourceManager, pManager->getRestoringJob());
	}
	else{
		// Bind
		if( NULL == pc_DeSerializeBoxJobList->getBindingDoc(static_cast<uchar>(pManager->getNumOfDocRestored()+1)) ) {
			APIC_setNewDetailCode(0, makeNewDetailCodeJob0AType01(JOBD_NewDetailCodeD002_PlaceCode_ResourceManager_002, (pManager->getNumOfDocRestored()+1)));	// 1027261 新詳細コード追加対応 2017/8/8 m.ooba.
			JOBC_Job::setSysErr( TYPD_TroubleCode_D002, __LINE__ );
		}
		DocRtn = pc_DeSerializeBoxJobList->getBindingDoc(static_cast<uchar>(pManager->getNumOfDocRestored()+1))->req_DeSerialize(
				sl_DocID[pManager->getNumOfDocRestored()],
				pManager->getRestoringJob()->getJobSpec()->pc_Common->c_JobType.getBoxNo(),
				pManager->ans_DeSerializeForResourceManager, pManager->getRestoringJob());
	}
	
	if( DOCD_RetOk != DocRtn ){
		// ジョブ破棄・システムエラー発生
		setDocAccessErr(pManager,pc_DeSerializeBoxJobList,DocRtn);
		rtn = False;
	}
	return rtn;
}

// < class JOBC_ResourceManagerStateWaitingGetInstanceEnd >-----------------------------
// class JOBC_ResourceManagerStateWaitingGetInstanceEnd : public JOBC_ResourceManagerState

//概要    ：インスタンス生成
//作成者  ：sakuragi
//レビュー：未実施（レビュー実施日と名前を記述）
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
JOBC_ResourceManagerStateWaitingGetInstanceEnd*
JOBC_ResourceManagerStateWaitingGetInstanceEnd::newInstance()
{
	static JOBC_ResourceManagerStateWaitingGetInstanceEnd* pInstance;
	if( NULL == pInstance ){
		try
		{
			pInstance = new JOBC_ResourceManagerStateWaitingGetInstanceEnd;
		}
		catch(...)
		{
			APIC_setFatalDetailCodeE002(CE002_DetailCode0x50EC);
			APIC_ThrowSystemError(SYSE_SysErrTypeSystemExhausted,LOGF_DEBUG_EVENT_KIND_JOB,
				const_cast<const char *>("JOBC_ResourceManagerStateWaitingGetInstanceEnd インスタンス生成失敗"));
			return NULL;
		}
	}
	return pInstance;
}

//概要    ：状態に入ったときに必ず行う処理
//作成者  ：sakuragi
//レビュー：未実施（レビュー実施日と名前を記述）
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
void
JOBC_ResourceManagerStateWaitingGetInstanceEnd::onEntry(JOBC_ResourceManager* pManager)
{
	if(RM_State<=RMDBG) RMLOG("★JOBC_ResourceManagerStateWaitingGetInstanceEnd::onEntry★\n",0,0,0,0,0);

	// インスタンス化実施(req_getInstance)
	if( False == req_GetInstance(pManager)){
		return;
	}
}

//概要    ：CompositeDoc生成完了通知
//作成者  ：sakuragi
//レビュー：未実施（レビュー実施日と名前を記述）
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
void
JOBC_ResourceManagerStateWaitingGetInstanceEnd::evtAnsGetInstance(
	JOBC_ResourceManager* pManager,
	DOCC_CompositeDoc* pDoc,
	int32_t sl_result)
{
	// TODO:a.deserializeのPatternもそうだが、Retryして展開の可能性がないDocからのErrorはRetryすべきでないかも。
	//        →Doc側でRetry可能なErrorのときに識別できないErrorCodeを返す課題がある。改修してね。
	JOBC_ElementOfReserveBoxJobList* pc_StartBoxJobList =
		pManager->mReserveList->searchAndGetReservation(pManager->getRestoringJob());
	if( NULL == pc_StartBoxJobList ) return;

	if(RM_State<=RMDBG) RMLOG("★JOBC_ResourceManagerStateWaitingGetInstanceEnd::evtAnsGetInstance★\n",0,0,0,0,0);

	if( DOCD_RetOk != sl_result ){
		// WaitingForRetryTimer状態遷移
		changeState(pManager,JOBC_ResourceManagerStateWaitingForRetryTimer::newInstance());
	}
	else{
		// add_List(再設定)
		// Set_DocPtr
		pc_StartBoxJobList->setCompositeDoc(pDoc);
		// OpenDoc
		int32_t DocRtn = pc_StartBoxJobList->getCompositeDoc()->openDoc();
		if( DOCD_RetOk != DocRtn ){
			RMLOG("★JOBC_ResourceManagerStateWaitingGetInstanceEnd::evtAnsGetInstance openDoc-NG!!★\n",0,0,0,0,0);
			// ジョブ破棄・システムエラー発生
			setDocAccessErr(pManager,pc_StartBoxJobList,DocRtn);
			return;
		}

		// 展開終了
		// callback & deleteList
		callback_deleteBoxJobList(pManager,pc_StartBoxJobList,sl_result,True);
		// Driving状態遷移
		changeState(pManager,JOBC_ResourceManagerStateDriving::newInstance());
		// DBGLOG
		if(RM_End<=RMDBG) RMLOG("★JOBC_ResourceManagerStateWaitingGetInstanceEnd::JOBSinario-START!!★\n",0,0,0,0,0);
	}
}

//概要    ：インスタンス化実施(req_getInstance)
//作成者  ：sakuragi
//レビュー：未実施（レビュー実施日と名前を記述）
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
Bool 
JOBC_ResourceManagerStateWaitingGetInstanceEnd::req_GetInstance(
	JOBC_ResourceManager* pManager)
{
	int32_t DocRtn = DOCD_RetOk;
	Bool rtn = True;
	JOBC_ElementOfReserveBoxJobList* pc_StartBoxJobList =
		pManager->mReserveList->searchAndGetReservation(pManager->getRestoringJob());
	if( NULL == pc_StartBoxJobList ) return False;

	DocRtn = DOCC_CompositeDoc::req_getInstance(
				pc_StartBoxJobList->getCopiedDocID(),
				pc_StartBoxJobList->getCopiedBoxID(),
				pManager->ans_getInstanceForResourceManager,
				reinterpret_cast<void*>(pManager->getRestoringJob()));
	if( DOCD_RetOk != DocRtn ){
		// ジョブ破棄・システムエラー発生
		setDocAccessErr(pManager,pc_StartBoxJobList,DocRtn);
		rtn = False;
	}
	return rtn;
}

// < class JOBC_ResourceManagerStateCancellingDocRestore >-----------------------------
// class JOBC_ResourceManagerStateCancellingDocRestore : public JOBC_ResourceManagerState

//概要    ：インスタンス生成
//作成者  ：sakuragi
//レビュー：未実施（レビュー実施日と名前を記述）
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
JOBC_ResourceManagerStateCancellingDocRestore*
JOBC_ResourceManagerStateCancellingDocRestore::newInstance()
{
	static JOBC_ResourceManagerStateCancellingDocRestore* pInstance;
	if( NULL == pInstance ){
		try
		{
			pInstance = new JOBC_ResourceManagerStateCancellingDocRestore;
		}
		catch(...)
		{
			APIC_setFatalDetailCodeE002(CE002_DetailCode0x50F2);
			APIC_ThrowSystemError(SYSE_SysErrTypeSystemExhausted,LOGF_DEBUG_EVENT_KIND_JOB,
				const_cast<const char *>("JOBC_ResourceManagerStateCancellingDocRestore インスタンス生成失敗"));
			return NULL;
		}
	}
	return pInstance;
}

//概要    ：CompositeDoc生成完了通知
//作成者  ：sakuragi
//レビュー：未実施（レビュー実施日と名前を記述）
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
void
JOBC_ResourceManagerStateCancellingDocRestore::evtAnsGetInstance(
	JOBC_ResourceManager* pManager,
	DOCC_CompositeDoc* pDoc,
	int32_t sl_result)
{
	int32_t DocRtn = DOCD_RetOk;
	DOCC_BoxIf* pBox=NULL;
	
	if(RM_State<=RMDBG) RMLOG("★JOBC_ResourceManagerStateCancellingDocRestore::evtAnsGetInstance★\n",0,0,0,0,0);

	JOBC_ElementOfReserveBoxJobList* pc_CancelBoxJobList =
		pManager->mReserveList->searchAndGetReservation(pManager->getRestoringJob());
	if( NULL == pc_CancelBoxJobList ) return;

	do
	{

		if( DOCD_RetOk == sl_result ){
			// Box::newInstance()
			pBox = DOCC_BoxIf::newInstance(pc_CancelBoxJobList->getCopiedBoxID());
			if( NULL == pBox ){
				RMLOG("★JOBC_ResourceManagerStateCancellingDocRestore::evtAnsGetInstance pBox=NULL!!★\n",0,0,0,0,0);
				DocRtn = DOCD_RetParameterErr;
				break;
			}
			// Doc::openDoc()
			DocRtn = pDoc->openDoc();
			if( DOCD_RetOk != DocRtn ){
				RMLOG("★JOBC_ResourceManagerStateCancellingDocRestore::evtAnsGetInstance openDoc-NG!!★\n",0,0,0,0,0);
				break;
			}
			// Doc::deleteAll()
			DocRtn = pDoc->deleteAll(pBox);
			if( DOCD_RetOk != DocRtn ){
				RMLOG("★JOBC_ResourceManagerStateCancellingDocRestore::evtAnsGetInstance deleteAll-NG!!★\n",0,0,0,0,0);
				break;
			}
			// Box::deleteInstance()
			pBox->deleteInstance();
		}

		// DeleteDocument判定
		if( False == pc_CancelBoxJobList->checkDocFileDelete() ){
			// closeDoc & Doc::deleteInstance()
			if( False == closeDoc_deleteInstance(pManager,pc_CancelBoxJobList)){
				return;
			}
		}
		else{
			// Doc::deleteAll()
			if( False == deleteAll(pManager,pc_CancelBoxJobList)){
				return;
			}
		}
		// callback & deleteList
		callback_deleteBoxJobList(pManager,pc_CancelBoxJobList,sl_result);
		// Driving状態遷移
		changeState(pManager,JOBC_ResourceManagerStateDriving::newInstance());
		// DBGLOG
		if(RM_End<=RMDBG) RMLOG("★JOBC_ResourceManagerStateCancellingDocRestore::JOB-CANCEL!!★\n",0,0,0,0,0);

	}while(False);

	if( DOCD_RetOk != DocRtn ){
		// ジョブ破棄・システムエラー発生
		if( NULL != pBox) pBox->deleteInstance();
		setDocAccessErr(pManager,pc_CancelBoxJobList,DocRtn);
	}
}

//概要    ：CompositeDoc生成完了通知
//作成者  ：sakuragi
//レビュー：未実施（レビュー実施日と名前を記述）
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
void
JOBC_ResourceManagerStateCancellingDocRestore::evtAnsDeserializeDoc(
	JOBC_ResourceManager* pManager,
	int32_t sl_result)
{
	if(RM_State<=RMDBG) RMLOG("★JOBC_ResourceManagerStateCancellingDocRestore::evtAnsDeserializeDoc★\n",0,0,0,0,0);

	JOBC_ElementOfReserveBoxJobList* pc_CancelBoxJobList =
		pManager->mReserveList->searchAndGetReservation(pManager->getRestoringJob());
	if( NULL == pc_CancelBoxJobList ) return;

	// DeleteDocument判定
	if( False == pc_CancelBoxJobList->checkDocFileDelete() ){
		// closeDoc & Doc::deleteInstance()
		if( False == closeDoc_deleteInstance(pManager,pc_CancelBoxJobList)){
			return;
		}
	}
	else{
		// Doc::deleteAll()
		if( False == deleteAll(pManager,pc_CancelBoxJobList)){
			return;
		}
	}
	// callback & deleteList
	callback_deleteBoxJobList(pManager,pc_CancelBoxJobList,sl_result);
	// Driving状態遷移
	changeState(pManager,JOBC_ResourceManagerStateDriving::newInstance());
	// DBGLOG
	if(RM_End<=RMDBG) RMLOG("★JOBC_ResourceManagerStateCancellingDocRestore::JOB-CANCEL!!★\n",0,0,0,0,0);
}

//概要    ：DocFileCopy完了通知
//作成者  ：sakuragi
//レビュー：未実施（レビュー実施日と名前を記述）
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
void
JOBC_ResourceManagerStateCancellingDocRestore::evtAnsChangeDocFilePosition(
	JOBC_ResourceManager* pManager,
	int32_t sl_result)
{
	if(RM_State<=RMDBG) RMLOG("★JOBC_ResourceManagerStateCancellingDocRestore::evtAnsChangeDocFilePosition★\n",0,0,0,0,0);

	JOBC_ElementOfReserveBoxJobList* pc_CancelBoxJobList =
		pManager->mReserveList->searchAndGetReservation(pManager->getRestoringJob());
	if( NULL == pc_CancelBoxJobList ) return;

	if( DOCD_RetOk == sl_result ){
		// Copy成功
		// Doc::deleteAll()
		if( False == deleteAllForCopyEnd(pManager,pc_CancelBoxJobList)){
			return;
		}
	}

	// DeleteDocument判定
	if( False == pc_CancelBoxJobList->checkDocFileDelete() ){
		// closeDoc & Doc::deleteInstance()
		if( False == closeDoc_deleteInstance(pManager,pc_CancelBoxJobList)){
			return;
		}
	}
	else{
		// Doc::deleteAll()
		if( False == deleteAll(pManager,pc_CancelBoxJobList)){
			return;
		}
	}
	// callback & deleteList
	callback_deleteBoxJobList(pManager,pc_CancelBoxJobList,sl_result);
	// Driving状態遷移
	changeState(pManager,JOBC_ResourceManagerStateDriving::newInstance());
	// DBGLOG
	if(RM_End<=RMDBG) RMLOG("★JOBC_ResourceManagerStateCancellingDocRestore::JOB-CANCEL!!★\n",0,0,0,0,0);
}

// < class JOBC_ResourceManagerStateCancellingWaitForDocDelete >-----------------------
// class JOBC_ResourceManagerStateCancellingWaitForDocDelete : public JOBC_ResourceManagerState

//概要    ：インスタンス生成
//作成者  ：sakuragi
//レビュー：未実施（レビュー実施日と名前を記述）
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
JOBC_ResourceManagerStateCancellingWaitForDocDelete*
JOBC_ResourceManagerStateCancellingWaitForDocDelete::newInstance()
{
	static JOBC_ResourceManagerStateCancellingWaitForDocDelete* pInstance;
	if( NULL == pInstance ){
		try
		{
			pInstance = new JOBC_ResourceManagerStateCancellingWaitForDocDelete;
		}
		catch(...)
		{
			APIC_setFatalDetailCodeE002(CE002_DetailCode0x50F1);
			APIC_ThrowSystemError(SYSE_SysErrTypeSystemExhausted,LOGF_DEBUG_EVENT_KIND_JOB,
				const_cast<const char *>("JOBC_ResourceManagerStateCancellingWaitForDocDelete インスタンス生成失敗"));
			return NULL;
		}
	}
	return pInstance;
}

//概要    ：状態に入ったときに必ず行う処理
//作成者  ：sakuragi
//レビュー：未実施（レビュー実施日と名前を記述）
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
void
JOBC_ResourceManagerStateCancellingWaitForDocDelete::onEntry(
	JOBC_ResourceManager* pManager)
{
	// 処理なし
}

//概要    ：DocFileDelete完了通知
//作成者  ：sakuragi
//レビュー：未実施（レビュー実施日と名前を記述）
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
void
JOBC_ResourceManagerStateCancellingWaitForDocDelete::evtAnsDeleteDoc(
	JOBC_ResourceManager* pManager,
	int32_t sl_result)
{
	// 処理なし
}

//class JOBC_TimerObserverForResourceManager

// 概要：default constructor
JOBC_TimerObserverForResourceManager::JOBC_TimerObserverForResourceManager(){}

// 概要：default constructor
JOBC_TimerObserverForResourceManager::JOBC_TimerObserverForResourceManager(
	int32_t sl_eventKindParam,	// イベント種別
	uint32_t sl_reqTime) :		// タイマー設定時間(msec)
sl_eventKind(sl_eventKindParam)
{
	SYSC_Timer* pTimer = SYSC_Timer::newInstance();
	m_WdId = pTimer->attachObserver(this);
	if(m_WdId == NULL) {
		APIC_setFatalDetailCodeE002(CE002_DetailCode0x50AA);
		APIC_ThrowSystemError(
		    SYSE_SysErrTypeSystemExhausted,                 // システム資源不足（OS資源不足など）
			LOGF_DEBUG_EVENT_KIND_JOB, 
			"SYSC_Timer::attachObserver() failed");
	}

	// タイマ値の設定
	setTimerValue(sl_reqTime);
}

// 概要：destructor
JOBC_TimerObserverForResourceManager::~JOBC_TimerObserverForResourceManager()
{
	SYSC_Timer* pTimer = SYSC_Timer::newInstance();
	pTimer->detachObserver(m_WdId);
}

// 概要：Timerタスクからのタイマー満了通知
void 
JOBC_TimerObserverForResourceManager::expire(
	WDOG_ID wdId)
{
	// 該当タイマー判定
	if( this->m_WdId == wdId ){
		JOBS_RMStateMsgBuff sMsg;
		sMsg.sl_EventID	= sl_eventKind;
		JOBC_ResourceManager::sendMsgRMState(&sMsg);
	}
}

// 概要：タイマー開始
void 
JOBC_TimerObserverForResourceManager::start()
{
	SYSC_Timer*	pTimer = SYSC_Timer::newInstance();
	//タイマの起動
	pTimer->req_notifyWhen(&m_ReqTime, m_WdId);
}

// 概要：タイマー停止
void 
JOBC_TimerObserverForResourceManager::stop()
{
	SYSC_Timer*	pTimer = SYSC_Timer::newInstance();
	//タイマの停止
	pTimer->cancel(m_WdId);
}

// 概要：タイマー時間設定
void 
JOBC_TimerObserverForResourceManager::setTimerValue(
	uint32_t sl_reqTime)
{
	m_ReqTime.RelTime =sl_reqTime;	//ms単位の時間
	m_ReqTime.eTimerType = SYSE_RELATIVE;
}

//class JOBC_ElementOfReserveBoxJobList

// 概要：default constructor
JOBC_ElementOfReserveBoxJobList::JOBC_ElementOfReserveBoxJobList() :
	sl_CopiedDocID(TYPD_NotSetting),sl_CopiedBoxID(TYPD_NotSetting),b_deleteDocFile(False){}

// 概要：default constructor
JOBC_ElementOfReserveBoxJobList::JOBC_ElementOfReserveBoxJobList(
	JOBC_Job* pJobParam,
	void (*ans_reserveBoxJobParam)(JOBC_Job* pJob, DOCC_CompositeDoc* pDoc, int32_t sl_result),
	void (*ans_cancelBoxJobReserveParam)(JOBC_Job* pJob, int32_t sl_result)) :
pJob(pJobParam),pDoc(NULL),sl_CopiedDocID(TYPD_NotSetting),sl_CopiedBoxID(TYPD_NotSetting),
ans_reserveBoxJob(ans_reserveBoxJobParam),ans_cancelBoxJobReserve(ans_cancelBoxJobReserveParam),
b_deleteDocFile(False)
{
	// 電源遅延フラグ設定
	APIC_Power::newInstance()->setPowerSaveDelay(__FILE__, __LINE__);
}

// 概要：destructor
JOBC_ElementOfReserveBoxJobList::~JOBC_ElementOfReserveBoxJobList()
{
	// 電源遅延フラグ解除
	APIC_Power::newInstance()->resetPowerSaveDelay(__FILE__, __LINE__);
}

// 概要：要素に処理するDocのポインタをセット
void JOBC_ElementOfReserveBoxJobList::setCompositeDoc(DOCC_CompositeDoc* pDocParam)
{
	this->pDoc = pDocParam;
	if(pJob != NULL) pJob->setBoxToPrintDoc(pDocParam, 0);
}

// 概要：BindするCompositeDocのポインタを返す
// 備考：indexは1から10まで指定可能。返せないときはNullを返す。
DOCC_CompositeDoc* 
JOBC_ElementOfReserveBoxJobList::getBindingDoc(unsigned char uc_index) const 
{ 
	return (pJob != NULL && uc_index > 0x00 && uc_index <= JOBD_JOBCCATCHCOMPOSITEDOC_MAX - 1) ? 
	pJob->getBoxToPrintDoc(static_cast<uint32_t>(uc_index)) : NULL;
}

// 概要：BindするCompositeDocのポインタをセットする
// 備考：indexは1から10まで指定可能。
void 
JOBC_ElementOfReserveBoxJobList::setBindingDoc(
	unsigned char uc_index,
	DOCC_CompositeDoc* pBindingDoc) const
{
	if (pJob != NULL && uc_index > 0x00 && uc_index <= JOBD_JOBCCATCHCOMPOSITEDOC_MAX - 1)
	{
		// JobでのBindDocの管理は配列の０番目にに親Docが入るので引数をそのまま指定する。
		pJob->setBoxToPrintDoc(pBindingDoc, static_cast<uint32_t>(uc_index));
	}
}

// 概要：BoxJob開始応答を実行
void 
JOBC_ElementOfReserveBoxJobList::execReserveEnd(int32_t sl_result)
{
	ans_reserveBoxJob(pJob, pDoc, sl_result);
}

// 概要：BoxJob破棄応答を実行
void 
JOBC_ElementOfReserveBoxJobList::execCancelCompleted(int32_t sl_result)
{
	ans_cancelBoxJobReserve(pJob, sl_result);
}

//class JOBC_ReserveBoxJobList

// 概要：default constructor
JOBC_ReserveBoxJobList::JOBC_ReserveBoxJobList()
{
	m_semId = SYSF_semMCreate(SEM_Q_PRIORITY|SEM_INVERSION_SAFE);
}

// 概要：destructor
JOBC_ReserveBoxJobList::~JOBC_ReserveBoxJobList()
{
	SYSF_semDelete(m_semId);
}

// 概要：インスタンス生成
JOBC_ReserveBoxJobList* 
JOBC_ReserveBoxJobList::newInstance()
{
	static JOBC_ReserveBoxJobList* pInstance;
	if( NULL == pInstance ){
		try
		{
			pInstance = new JOBC_ReserveBoxJobList;
		}
		catch(...)
		{
			APIC_setFatalDetailCodeE002(CE002_DetailCode0x50FC);
			APIC_ThrowSystemError(SYSE_SysErrTypeSystemExhausted,LOGF_DEBUG_EVENT_KIND_JOB,
				const_cast<const char *>("JOBC_ReserveBoxJobList インスタンス生成失敗"));
			return NULL;
		}
	}
	return pInstance;
}

// 概要：予約情報を登録する
void 
JOBC_ReserveBoxJobList::addReservation(
	JOBC_ElementOfReserveBoxJobList* pElement)
{
	lock();
	// 既にリスト登録している要素は再登録させない。(ガード処理)
	Bool b_addDeny = False;
	if (m_reserveList.empty() == False) {
		for(JOBT_DequeForReserveBoxJobList::iterator it = m_reserveList.begin(); it != m_reserveList.end(); it++) {
			if(*it == pElement) {
				b_addDeny = True;
				break;
			}
		}
	}

	if(b_addDeny == False) m_reserveList.push_back(pElement);
	unlock();
}

// 概要：指定された予約情報を削除する
void 
JOBC_ReserveBoxJobList::deleteReservation(
	JOBC_ElementOfReserveBoxJobList* pElement)
{
	lock();

	if (m_reserveList.empty() == False) {
		for(JOBT_DequeForReserveBoxJobList::iterator del_it = m_reserveList.begin(); del_it != m_reserveList.end(); del_it++) {
			if(*del_it == pElement) {
				m_reserveList.erase(del_it);
				break;
			}
		}
	}

	unlock();
}

// 概要：指定されたジョブポインタをキーに予約情報を取得する
JOBC_ElementOfReserveBoxJobList* 
JOBC_ReserveBoxJobList::searchAndGetReservation(
	JOBC_Job* pJob)
{
	lock();
	JOBC_ElementOfReserveBoxJobList* pElement = NULL;

	if (m_reserveList.empty() == False) {
		for(JOBT_DequeForReserveBoxJobList::iterator search_it = m_reserveList.begin(); search_it != m_reserveList.end(); search_it++) {
			if((*search_it)->getJob() == pJob) {
				pElement = *search_it;
				break;
			}
		}
	}

	unlock();
	return pElement;
}

// 概要；予約リストに登録されている全てのジョブを返す
void JOBC_ReserveBoxJobList::getReserveBoxJobList(
	std::deque<JOBC_Job*>& reserveList)
{
	lock();
	if (m_reserveList.empty() == False) {
		for(JOBT_DequeForReserveBoxJobList::iterator it = m_reserveList.begin(); it != m_reserveList.end(); it++) {
			reserveList.push_back((*it)->getJob());
		}
	}

	unlock();
}

// 概要：予約リストの先頭の予約情報を返す
JOBC_ElementOfReserveBoxJobList* 
JOBC_ReserveBoxJobList::getFirstReservation()
{
	lock();
	JOBC_ElementOfReserveBoxJobList* pElement = NULL;
	if(m_reserveList.empty() != true) pElement = m_reserveList.front();
	unlock();
	return pElement;
}

// 概要：リストの排他を行う
void JOBC_ReserveBoxJobList::lock()
{
	SYSF_semTake(m_semId,WAIT_FOREVER);
}

// 概要：リストの排他を解除する
void JOBC_ReserveBoxJobList::unlock()
{
	SYSF_semGive(m_semId);
}

// debug log---------------------------------------------------------------------------
extern "C" 
{
	void getisAv();
    void getisAv_21_1_5(APIC_ScannerWarning* pScannerWarning);//機種固有処理
}

void getisAv()
{
	Bool warning = False;
	APIC_PrinterWarning* pPrinterWarning = APIC_PrinterWarning::newInstance();
	APIC_ScannerWarning* pScannerWarning = APIC_ScannerWarning::newInstance();

	for(int cnt=(int)ATTENTION_CTonerNearEmpty;cnt<=(int)ERROR_SpecificOriginalDetect;cnt++)
	{
		warning = pPrinterWarning->isWarning(static_cast<TYPE_PrinterWarning>(cnt));
		if( True == warning ){
			printf("JOBC_ResourceManager::PrinterWarning(%d)\n",cnt);
			return;
		}
	}
    getisAv_21_1_5(pScannerWarning); //機種固有処理
	return;
}

