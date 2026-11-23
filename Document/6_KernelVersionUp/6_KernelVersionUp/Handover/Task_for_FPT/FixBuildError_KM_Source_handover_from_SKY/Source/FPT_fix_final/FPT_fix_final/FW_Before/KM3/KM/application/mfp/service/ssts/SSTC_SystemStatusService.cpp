//************************************************************************************************************************.
//ファイル名：SSTC_SystemStatusService.cpp.
//
//概要：SystemStatusサービスクラス.
//************************************************************************************************************************.

//************************************************************************************************************************.
//インクルード・ファイル.
//************************************************************************************************************************.
// 2021/10/22 APF OP_BTS-35403 start
#include	<algorithm>
// 2021/10/22 APF OP_BTS-35403 end
#include	<memory>									// memxxxx...
#include	<cstddef>									// offsetof...
#include	<cstdlib>									// mkstemp...
#include	<semaphore.h>

#include	"SSTC_SystemStatusService.h"
#include	"service/include/MFPC_CommonDef.h"
#include	"SSTC_SystemStatusUsedObserver.h"

#include	"system/scn/APIC_ScannerExternalIF.h"
#include	"system/prt/prtc/prtcImp/ccmntdef.h"		// EDI_EnvHistorySize：環境履歴格納配列サイズ.

#include	"system/sys/APIC_RdsApi.h"
#include	"system/doc/APIC_BoxRegistry.h"
#include	"system/fax/APIC_FaxDevice.h"
#include	"system/prt/APIC_Printer.h"
//@ 2011.9.6 ads modify ->
//#include	"system/sys/APIC_UsbMemDeviceInfo.h"
#include "system/saa/APIC_UsbMemDeviceInfo.h"
//@ 2011.9.6 ads modify <-
#include	"system/fil/FIL_FsWrapperC.h"
// STEP2：2010.12.10 <ADD> Start T.Takahashi.
#include	"system/sys/APIC_SystemInfo.h"
#include	"system/prt/prtc/prtcImp/ENGC_PrinterImpColor.h"
// STEP2：2010.12.10 <ADD> End   T.Takahashi.
// Job実行可否対応 2011.11.22 ADS K.Tsuboi Start.
#include	"system/sys/APIC_KeyDevice.h"
// Job実行可否対応 2011.11.22 ADS K.Tsuboi End.
#include	"system/uac/APIC_UserAcount.h"
#include 	"system/sys/APIC_OptionSetting.h" //2013/10/16 日立JTE 古川【DS-IGL】画像ログオプション化対応.

// 2014.01.09-STA  パネルHW情報の取得.
#include	"system/thr/PanelHW/THRC_PanelHardCom.h"
// 2014.01.09-END  パネルHW情報の取得.

#include	"../../../api/jobspec/inc/JobSpec_define.h"

//FWリファクタリング NVDC_Setting.hを分割.
#include "system/nvd/nvdg/NVDC_Setting_MachineSetting.h"

// MFPマルチ自動評価ツール対応 Start.
#include "system/dbg/dbgg/DBGC_MultiAutoTest.h"
// MFPマルチ自動評価ツール対応 End.

// 【RQ-747 VirusScanFUM for 機種】 start.
#include "system/nvd/APIC_Setting_VirusScanSetting.h"
#include "system/sys/APIC_VirusScan.h"
#include "system/sys/APIC_VirusScan_DEF.h"
// 【RQ-747 VirusScanFUM for 機種】 end.

#if defined(DEF_A4_Cus) && defined(DEF_KRONOSS_Cus)
extern "C" {
extern void NPAG_setChagneSysFatal( TYPS_SysStatusFatal sysFatal );
extern void NPAG_setChagneSccanerFatal( ushort us_Code , TYPE_FatalLevel e_FatalLevel );
extern void NPAG_setChagnePrinterFatal( TYPC_FatalCommErrorJamCode c_Error , char sc_Val );
extern void NPAG_setChagnePrinterJam( TYPC_FatalCommErrorJamCode c_Error );
extern void NPAG_setChagneTcError( TYPC_FatalCommErrorJamCode c_Error );
extern void NPAG_setChagneIuError( TYPC_FatalCommErrorJamCode c_Error );
}
Bool call_NPAG_setChagneSysFatal = False;
Bool call_NPAG_setChagneSccanerFatal = False;
Bool call_NPAG_setChagnePrinterFatal = False;
Bool call_NPAG_setChagnePrinterJam = False;
Bool call_NPAG_setChagneTcError = False;
Bool call_NPAG_setChagneIuError = False;
#endif // defined(DEF_A4_Cus) && defined(DEF_KRONOSS_Cus).


SSTC_SystemStatusService* SSTC_SystemStatusService::m_pcInstance = NULL;
														//-インスタンスポインタ.
MSG_Q_ID	SSTC_SystemStatusService::m_msgQID = NULL;	// メッセージキューID.
WDOG_ID		SSTC_SystemStatusService::m_wdogID = NULL;	// ウォッチドッグID.
SEM_ID  	SSTC_SystemStatusService::m_semID  = 0;		// セマフォID.
SEM_ID  	SSTC_SystemStatusService::m_semIDShm = 0;	// セマフォID(共有メモリ用) [CQ359780].
int32_t		SSTC_SystemStatusService::m_Tick   = 0;		// 定期起床タイマー計算値.
MFPE_LogLevel		SSTC_SystemStatusService::m_LogLevelSetting = MFPE_LogLevelError;	// ログ出力許容レベル.

// SystemStatusInfoファイル出力先.
#ifdef LINUX
	#define 	SSTD_OUTPUT_FILEPATH		"/ramdisk/SystemStatusInfo.XXXXXX"	// ramdiskに出力.
#else
	#define 	SSTD_OUTPUT_FILEPATH		"c:\\SystemStatusInfo.000000"	// Windows Sim環境用.
#endif

// STEP2：2010.12.10 <ADD> Start T.Takahashi.
//===== 変化点通知設定要求格納テーブル情報 =====.
#define             SSTD_Char_Bits                (8)
#define             SSTD_Short_Bits               (16)
#define             SSTD_Long_Bits                (32)
#define             SSTD_Mask_DataID              (0x0000ffff)
static const ushort SSTD_NotificationSetting    = ((512 / SSTD_Char_Bits) / sizeof(uint32_t));
static const ushort SSTD_NotificationSettingDataSize = (sizeof(uint32_t) * SSTD_NotificationSetting);
// STEP2：2010.12.10 <ADD> End   T.Takahashi.

//************************************************************************************************************************.
//概要　　：コンストラクタ.
//戻り値　：無し.
//備考　　：特に無し.
//作成者　：谷本／高橋.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
// 2012. 1.31 [CQ359780] getProgJobXXXStatusをポーリング→Observerに変更.
//************************************************************************************************************************.
SSTC_SystemStatusService::SSTC_SystemStatusService()
{
	SST_LOG(MFPE_LogLevelTrace);
	
	// 初期化.
	set_serviceid( MFPD_TARGETID_SYSTEMSTATUSSERVICE );	// サービスID.
	
	m_ulCounterPrinterStatusObserver = 0;			// APIC_PrinterStatusObserver::dataChanged()発生回数.
	m_ulCounterPrinterWarningObserver = 0;			// APIC_PrinterWarningObserver::dataChanged()発生回数.
	m_ulCounterSoftDipSwObserver = 0;				// APIC_SoftDipSwObserver::dataChanged()発生回数.
	m_ulCounterSystemInfoObserver = 0;				// APIC_SystemInfoObserverObserver::dataChanged()発生回数.
	m_ulCounterPowerStatusChanged = 0;				// APIC_PowerObserver::powerStatusChanged()発生回数.

	m_bStartNotifyFlag = False;							// 通知開始フラグ.
	m_vChangedInfo.clear();								// 変化情報.
	m_vStartNotifyDataID.clear();						// 通知開始前のため処理できなかった警告ID.
	// STEP2：2010.12.10 <ADD> Start T.Takahashi.
	// 変化点通知設定.
	for( ushort us_GroupIdx = 0; us_GroupIdx < TYPE_NTF_GroupID_Max; us_GroupIdx++ ){
		m_pulNotificationMap[ us_GroupIdx ] = NULL;
	}
	// STEP2：2010.12.10 <ADD> End   T.Takahashi.
	
	// 構造体の初期化.
	memset( (char *)&m_stSysStatInfo, (char)SSTD_ZERO_OUT, sizeof( m_stSysStatInfo ) );
	
	m_pcShMemMan = NULL;    /* 静的解析 */
	do {
		// セマフォの生成(Shm用).
		m_semIDShm = SYSF_semMCreate( (SEM_Q_PRIORITY | SEM_DELETE_SAFE | SEM_INVERSION_SAFE), False );
		if( m_semIDShm == 0 ){
			initStaticMember();
// 2015.10.16 ZeusSZX0 制御76-1 新詳細コード s.
//			APIC_ThrowSystemErrorDetailCode(SYSE_SysErrTypeFatalSystemCallError, LOGF_DEBUG_EVENT_KIND_SYS_STATUS, "SSTC_SystemStatusService SYSF_semMCreate() for Shm == 0",CE002_DetailCode0x5830);
			APIC_ThrowSystemErrorNewDetailCode(SYSE_SysErrTypeFatalSystemCallError, LOGF_DEBUG_EVENT_KIND_SYS_STATUS, "SSTC_SystemStatusService SYSF_semMCreate() for Shm == 0", 0, createDetailCode(0x3042));
// 2015.10.16 ZeusSZX0 制御76-1 新詳細コード e.
			break;
		}
		
		// 共有メモリ用初期処理. 
		m_pcShMemMan = SSTC_SysStatusSharedMemory::newInstance();
		lock();
		STATUS s_sts = m_pcShMemMan->initGetShm();
		unlock();
		if( s_sts != OK ){
			initStaticMember();
// 2015.10.16 ZeusSZX0 制御76-1 新詳細コード s.
//			APIC_ThrowSystemError(SYSE_SysErrGeneral, LOGF_DEBUG_EVENT_KIND_SYS_STATUS, "SSTC_SystemStatusService initGetShm() != OK");
			APIC_ThrowSystemErrorNewDetailCode(SYSE_SysErrGeneral, LOGF_DEBUG_EVENT_KIND_SYS_STATUS, "SSTC_SystemStatusService initGetShm() != OK", 0, createDetailCode(0x3043));
// 2015.10.16 ZeusSZX0 制御76-1 新詳細コード e.
			break;
		}
		
		// ウォッチドッグタイマ生成.
		m_wdogID = SYSF_wdCreate();
		if( m_wdogID == NULL ){
			initStaticMember();
// 2015.10.16 ZeusSZX0 制御76-1 新詳細コード s.
//			APIC_ThrowSystemErrorDetailCode(SYSE_SysErrTypeFatalSystemCallError, LOGF_DEBUG_EVENT_KIND_SYS_STATUS, "SSTC_SystemStatusService SYSF_wdCreate() == NULL",CE002_DetailCode0x5831);
			APIC_ThrowSystemErrorNewDetailCode(SYSE_SysErrTypeFatalSystemCallError, LOGF_DEBUG_EVENT_KIND_SYS_STATUS, "SSTC_SystemStatusService SYSF_wdCreate() == NULL", 0, createDetailCode(0x3044));
// 2015.10.16 ZeusSZX0 制御76-1 新詳細コード e.
			break;
		}
		
		// メッセージキュー生成. <TODO>.
// 2012.08.29 sk_takas for CQ#370048 -->
		m_msgQID = SYSF_msgQCreate( SSTD_MSGQ_MAX, sizeof( SSTS_SysStatusMsg ), MSG_Q_FIFO );
// 2012.08.29 sk_takas for CQ#370048 <--
		if( m_msgQID == NULL ){
			initStaticMember();
// 2015.10.16 ZeusSZX0 制御76-1 新詳細コード s.
//			APIC_ThrowSystemErrorDetailCode(SYSE_SysErrTypeFatalSystemCallError, LOGF_DEBUG_EVENT_KIND_SYS_STATUS, "SSTC_SystemStatusService SYSF_msgQCreate() == NULL",CE002_DetailCode0x5832);
			APIC_ThrowSystemErrorNewDetailCode(SYSE_SysErrTypeFatalSystemCallError, LOGF_DEBUG_EVENT_KIND_SYS_STATUS, "SSTC_SystemStatusService SYSF_msgQCreate() == NULL", 0, createDetailCode(0x3045));
// 2015.10.16 ZeusSZX0 制御76-1 新詳細コード e.
			break;
		}
		
		// セマフォの生成.
// 2011/12/6 [FVT Bug No.86] <MOD> Start.
//		m_semID = SYSF_semMCreate( (SEM_Q_PRIORITY | SEM_DELETE_SAFE | SEM_INVERSION_SAFE), True );
		m_semID = SYSF_semMCreate( (SEM_Q_PRIORITY | SEM_DELETE_SAFE | SEM_INVERSION_SAFE), False );
// 2011/12/6 [FVT Bug No.86] <MOD> End.
		if( m_semID == 0 ){
			initStaticMember();
// 2015.10.16 ZeusSZX0 制御76-1 新詳細コード s.
//			APIC_ThrowSystemErrorDetailCode(SYSE_SysErrTypeFatalSystemCallError, LOGF_DEBUG_EVENT_KIND_SYS_STATUS, "SSTC_SystemStatusService SYSF_semMCreate() == 0",CE002_DetailCode0x5833);
			APIC_ThrowSystemErrorNewDetailCode(SYSE_SysErrTypeFatalSystemCallError, LOGF_DEBUG_EVENT_KIND_SYS_STATUS, "SSTC_SystemStatusService SYSF_semMCreate() == 0", 0, createDetailCode(0x3046));
// 2015.10.16 ZeusSZX0 制御76-1 新詳細コード e.
			break;
		}
// 2014.01.09-STA  パネルHW情報の取得.
		// パネルHW情報の初期値設定(共有メモリとメンバ変数).
		initPanelHardResolution();
// 2014.01.09-END  パネルHW情報の取得.

		
	}while(0);
}

//************************************************************************************************************************.
//概要　　：デストラクタ.
//戻り値　：無し.
//備考　　：特に無し.
//作成者　：谷本.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//************************************************************************************************************************.
SSTC_SystemStatusService::~SSTC_SystemStatusService()
{
	SST_LOG(MFPE_LogLevelAll);
	
	// オブザーバのデタッチ.
	detachAllObserver();

	// STEP2：2010.12.10 <ADD> Start T.Takahashi.
	// 変化点通知設定テーブルの開放.
	for( ushort us_GroupIdx = 0; us_GroupIdx < TYPE_NTF_GroupID_Max; us_GroupIdx++ ){
		if ( m_pulNotificationMap[ us_GroupIdx ] != NULL ) { delete [] m_pulNotificationMap[ us_GroupIdx ]; }
	}
	// STEP2：2010.12.10 <ADD> End   T.Takahashi.
}

//************************************************************************************************************************.
//概要　　：SystemStatusサービス・インスタンス取得.
//戻り値　：SystemStatusサービス・インスタンス.
//備考　　：特に無し.
//作成者　：谷本.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//************************************************************************************************************************.
SSTC_SystemStatusService* SSTC_SystemStatusService::newInstance()
{
	SST_LOG(MFPE_LogLevelTrace);
	
	static SSTC_SystemStatusService c_Instance;
	
	return &c_Instance;
}

//************************************************************************************************************************.
//概要　　：定期起床.
//戻り値　：無し.
//備考　　：特に無し.
//作成者　：谷本.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
// 2012.08.07, ADS井上, FVT2不具合対応[CQ369958].
//  - 定期起床タイマ起動位置変更    起動位置を cyclicLoop() から messageLoop() へ.
//************************************************************************************************************************.
void SSTC_SystemStatusService::cyclicLoop()
{
	SST_LOG(MFPE_LogLevelAll);
	SSTS_SysStatusMsg s_Message;
	
	// 初期化.
	memset( (char *)&s_Message, (char)SSTD_ZERO_OUT, sizeof( SSTS_SysStatusMsg ) );
	
	s_Message.e_MsgType = SSTE_SysStatusMsgType_WatchDogTimerExpire;
	
	// 定期起床メッセージ送信.
//[ CIT Bug No.238].
//	SYSF_msgQSend( m_msgQID, ( char * )&s_Message, sizeof( SSTS_SysStatusMsg ), NO_WAIT, MSG_PRI_NORMAL );
	SYSF_msgQSend( m_msgQID, ( char * )&s_Message, sizeof( SSTS_SysStatusMsg ), WAIT_FOREVER, MSG_PRI_NORMAL );
//[ CIT Bug No.238].
}

//************************************************************************************************************************.
//概要　　：初期化用関数.
//引数　　：<in>　argc				引数数.
//　　　　：<in>　argv				引数リスト.
//戻り値　：正常(MFPD_OK)または異常(MFPD_NG).
//備考　　：特に無し.
//作成者　：谷本.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
// 2011.12.08, ADS坪井, CIT不具合対応[マージ後不具合管理票 No.164].
//  - initObserverData():オブザーバで取得するデータの初期化処理追加.
//************************************************************************************************************************.
MFPD_Bool SSTC_SystemStatusService::service_initialize( int argc, char** argv )
{
	SST_LOG(MFPE_LogLevelTrace);
	
	// メッセージ受信用タスク生成. 優先度をデフォルトの206->208に変更([FVT Bug No.86]).
	THR_ID ev_MyTaskID = SYSF_taskSpawn( "SystemStatusService", 48, VX_FP_TASK, 50000, (FUNCPTR)SSTC_SystemStatusService::messageLoop, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1);	// 起動シーケンス変更
	if( ev_MyTaskID == 0){
		initStaticMember();
// 2015.10.16 ZeusSZX0 制御76-1 新詳細コード s.
//		APIC_ThrowSystemErrorDetailCode(SYSE_SysErrTypeFatalSystemCallError, LOGF_DEBUG_EVENT_KIND_SYS_STATUS, "SSTC_SystemStatusService SYSF_taskSpawn(() == 0",CE002_DetailCode0x5834);
		APIC_ThrowSystemErrorNewDetailCode(SYSE_SysErrTypeFatalSystemCallError, LOGF_DEBUG_EVENT_KIND_SYS_STATUS, "SSTC_SystemStatusService SYSF_taskSpawn(() == 0", 0, createDetailCode(0x3047));
// 2015.10.16 ZeusSZX0 制御76-1 新詳細コード e.
		return MFPD_NG;
	}

	// インスタンスを取得.
	m_pcInstance = SSTC_SystemStatusService::newInstance();
	
	MFPD_Bool b_Ret = MFPD_NG;
	if( m_wdogID != NULL && m_msgQID != NULL && m_semID != 0 && m_semIDShm != 0 ){	// 異常が発生していなければ.
		
		// Service固有の初期化処理.
		start_task();
		
		// ステータス更新.
		set_status( MFPD_SERVICE_STATUS_READY );
		
		msgQSend(SSTE_SysStatusMsgType_ServiceInitialize);
		
		b_Ret = MFPD_OK;
	}
	SST_LOG2(MFPE_LogLevelTrace, "End. b_Ret=(%d).\n", b_Ret);
	
	return b_Ret;
}

//************************************************************************************************************************.
//概要　　：オブザーバデータ更新要求関数.
//引数　　：なし
//戻り値　：正常(MFPD_OK)または異常(MFPD_NG).
//備考　　：特に無し.
//作成者　：森脇.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//************************************************************************************************************************.
MFPD_Bool SSTC_SystemStatusService::reqUpdateObserverData()
{
	SST_LOG(MFPE_LogLevelTrace);

	msgQSend( SSTE_SysStatusMsgType_UpdateObserverData );

	return MFPD_OK;
}

//************************************************************************************************************************.
//概要　　：SystemStatusデータ更新要求関数.
//引数　　：なし
//戻り値　：正常(MFPD_OK)または異常(MFPD_NG).
//備考　　：特に無し.
//作成者　：森脇.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//************************************************************************************************************************.
MFPD_Bool SSTC_SystemStatusService::reqUpdateSystemStatusData()
{
	SST_LOG(MFPE_LogLevelTrace);

	sem_t* p_sem_ssts_dataupdate = NULL;
	sem_unlink( "/sem_ssts_dataupdate" );
	p_sem_ssts_dataupdate = sem_open( "/sem_ssts_dataupdate", O_CREAT|O_RDWR, S_IRWXU|S_IRWXG|S_IRWXO, 0 );
	if( p_sem_ssts_dataupdate == SEM_FAILED ) {
		errorLog_sysServiceSsts( "/sem_ssts_dataupdate open() ERROR!! errno = [%d].\n", errno );
	}

	// SystemStatusデータ更新要求.
	msgQSend( SSTE_SysStatusMsgType_UpdateSystemStatusData );

	if (p_sem_ssts_dataupdate != NULL) {
		int sem_ret = 0;
		do {
			// SystemStatusデータ更新待ち.
			sem_ret = sem_wait( p_sem_ssts_dataupdate );
		} while (sem_ret == -1 && errno == EINTR);

		sem_close( p_sem_ssts_dataupdate );
		sem_unlink( "/sem_ssts_dataupdate" );
	}

	return MFPD_OK;
}

//************************************************************************************************************************.
//概要　　：メッセージ受信.
//引数　　：<in>　d_kind			引数数.
//　　　　：<in>　n_PktSize			受信数.
//　　　　：<in>　ul_ProcId			プロックID.
//戻り値　：無し.
//備考　　：オーバーライド.
//作成者　：高橋.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//************************************************************************************************************************.
void SSTC_SystemStatusService::receive_callback( MFPD_KindSocket d_Kind, size_t n_PktSize, MFPD_ProcID ul_ProcId )
{
	SST_LOG2(MFPE_LogLevelTrace, "d_Kind=(%d) n_PktSizer=(%d)  ul_ProcId=(%ld).\n", d_Kind, n_PktSize, ul_ProcId );
	
	// 受信処理.
	char *pc_Buff = new char [ n_PktSize ];
	
	do {
		if( pc_Buff == NULL ){
// 2015.10.16 ZeusSZX0 制御76-1 新詳細コード s.
//			APIC_ThrowSystemError(SYSE_SysErrGeneral, LOGF_DEBUG_EVENT_KIND_SYS_STATUS, "SSTC_SystemStatusService receive_callback() new error.");
			APIC_ThrowSystemErrorNewDetailCode(SYSE_SysErrGeneral, LOGF_DEBUG_EVENT_KIND_SYS_STATUS, "SSTC_SystemStatusService receive_callback() new error.", 0, createDetailCode(0x3048));
// 2015.10.16 ZeusSZX0 制御76-1 新詳細コード e.
			break;
		}
		memset( pc_Buff, (char)SSTD_ZERO_OUT, n_PktSize );
		MFPS_PacketHeader* pst_Packet = (MFPS_PacketHeader *)pc_Buff;
		
		ssize_t n_RecvSize = recv_data( d_Kind, (char*)pst_Packet, n_PktSize );
		SST_LOG2(MFPE_LogLevelTrace, "n_RecvSize=(%d) n_PktSize(%d).\n", n_RecvSize, n_PktSize );

// 2015.10.16 ZeusSZX0 制御76-1 新詳細コード s.
		if( FUNCALL_SSTD_RegistSystemData == pst_Packet->reqkindid){
// 2017/10/19 APF 新詳細コード改善 SegF付加情報対応 Start.
			ushort us_OldDetailCode = 0;
			uint64_t ull_NewDetailCode = 0;
//			ullong ull_DetailCode;
			memcpy( &us_OldDetailCode, pst_Packet->data, sizeof(us_OldDetailCode) );
			memcpy( &ull_NewDetailCode, pst_Packet->data + sizeof(us_OldDetailCode), sizeof(ull_NewDetailCode) );
//			memcpy( &ull_DetailCode, pst_Packet->data, sizeof(ull_DetailCode) );
			APIC_setNewDetailCode(us_OldDetailCode, ull_NewDetailCode);
//			APIC_setNewDetailCode(0, ull_DetailCode);
// 2017/10/19 APF 新詳細コード改善 SegF付加情報対応 End.
			continue;
		}
// 2015.10.16 ZeusSZX0 制御76-1 新詳細コード e.

		// リクエスト情報を登録 <Act_31462> CHG start.
		if( addRequest( ul_ProcId, pst_Packet ) == MFPD_NG ){
			SST_ERR_LOG("addRequest() error.");
			break;
		}
		// <Act_31462> CHG end.
		
		// リクエストIDをキューの最後に保存.
		struct ST_ConvertTable {
			uint32_t               ul_reqkindid;
			SSTE_SysStatusMsgType  e_MsgType;
		} static st_ConvertTable [] = {
			{ FUNCALL_SSTD_GetSystemStatusInfo   , SSTE_SysStatusMsgType_SysStatusGetReq        },
			{ FUNCALL_SSTD_StartStatusChanged    , SSTE_SysStatusMsgType_StartChangeData        },
			{ NOTICE_SSTD_StatusChanged          , SSTE_SysStatusMsgType_NotifyChangedData      },
			{ FUNCALL_SSTD_TroubleReset          , SSTE_SysStatusMsgType_TroubleReset           },
			{ FUNCALL_SSTD_SetSystemStatusData   , SSTE_SysStatusMsgType_SetSystemStatusData    },    // STEP2：2010.12.10 <ADD> T.Takahashi.
			{ FUNCALL_SSTD_SetNotificationSetting, SSTE_SysStatusMsgType_SetNotificationSetting }     // STEP2：2010.12.10 <ADD> T.Takahashi.
		};
		
		// プロックIDの保存.
		for( ushort us_CnvTblIdx = 0; us_CnvTblIdx < sizeof( st_ConvertTable ) / sizeof( struct ST_ConvertTable ) ; us_CnvTblIdx++ ){
			if( st_ConvertTable[ us_CnvTblIdx ].ul_reqkindid == pst_Packet->reqkindid ){
				m_QueueReqID[ st_ConvertTable[ us_CnvTblIdx ].e_MsgType ].push( ul_ProcId );
				SST_LOG2(MFPE_LogLevelTrace, "reqkindid=(0x%08x) us_CnvTblIdx(%d).\n", pst_Packet->reqkindid, us_CnvTblIdx );
				break;
			}
		}
		
		// ディスパッチ処理.
		switch( pst_Packet->reqkindid ){
			case  FUNCALL_SSTD_GetSystemStatusInfo:									// システムステータス情報取得要求.
				{
				msgQSend( SSTE_SysStatusMsgType_SysStatusGetReq );					// システムステータス情報取得要求メッセージを送信.
				break;
				}
			case  FUNCALL_SSTD_StartStatusChanged:									// 変化点通知開始要求.
				{
				// 変化点通知用のヘッダを登録 <20110126 CHG>.
				MFPS_PacketHeader *pst_NotifyChangeDestination = new MFPS_PacketHeader;
				if( pst_NotifyChangeDestination == NULL ){
// 2015.10.16 ZeusSZX0 制御76-1 新詳細コード s.
//					APIC_ThrowSystemError(SYSE_SysErrGeneral, LOGF_DEBUG_EVENT_KIND_SYS_STATUS, "SSTC_SystemStatusService receive_callback() new error 2.");
					APIC_ThrowSystemErrorNewDetailCode(SYSE_SysErrGeneral, LOGF_DEBUG_EVENT_KIND_SYS_STATUS, "SSTC_SystemStatusService receive_callback() new error 2.", 0, createDetailCode(0x3049));
// 2015.10.16 ZeusSZX0 制御76-1 新詳細コード e.
					break;
				}
				memcpy( pst_NotifyChangeDestination, pst_Packet->data, sizeof(MFPS_PacketHeader) );
				m_vNotifyChange.push_back( *pst_NotifyChangeDestination );
//Str【 静的解析対応 : 稲井 : 2013/08/08 : ID-18903】.
				delete pst_NotifyChangeDestination;
//End【 静的解析対応 : 稲井 : 2013/08/08 : ID-18903】.
				msgQSend( SSTE_SysStatusMsgType_StartChangeData );					// 変化点通知開始要求メッセージを送信.
				break;
				}
				
			// 変化点通知はディスパッチ処理の対象外.
			
			case  FUNCALL_SSTD_TroubleReset:										// トラブルリセット要求.
				
				msgQSend( SSTE_SysStatusMsgType_TroubleReset );						// トラブルリセット要求メッセージを送信.
				break;
				
			// STEP2：2010.12.10 <ADD> Start T.Takahashi.
			case  FUNCALL_SSTD_SetSystemStatusData:									// システムステータス更新要求.
				
				msgQSend( SSTE_SysStatusMsgType_SetSystemStatusData, pst_Packet->datasize, pst_Packet->data );
				break;
				
			case  FUNCALL_SSTD_SetNotificationSetting:								// 変化点通知設定要求.
				
				msgQSend( SSTE_SysStatusMsgType_SetNotificationSetting, pst_Packet->datasize, pst_Packet->data );
				break;
			// STEP2：2010.12.10 <ADD> End   T.Takahashi.

// MFPマルチ自動評価ツール対応 Start.
			case  FUNCALL_SSTD_AutoPlayComplete:									// 画面自動再生完了通知.
				{
					DBGC_MultiAutoTest* 	pc_MultiAutoTest = DBGC_MultiAutoTest::newInstance();
					if( NULL != pc_MultiAutoTest ){
						pc_MultiAutoTest->PnlAutoPlayComplete();	// 評価管理タスクへ通知.
					}else{
						errorLog_sysServiceSsts("***pc_MultiAutoTest=NULL \n");
					}
				}
				break;
// MFPマルチ自動評価ツール対応 End.
			default:
				SST_ERR_LOG("unknown reqkindid.");
				break;
		}
	} while( 0 );
	if( pc_Buff != NULL ){ delete [] pc_Buff; }
}

// STEP2：2010.12.10 <CHG> Start T.Takahashi.
//************************************************************************************************************************.
//概要　　：メッセージ送信.
//引数　　：<in>　e_MsgType			メッセージタイプ.
//引数　　：<in>　us_DataSize		データサイズ.
//引数　　：<in>　p_DataBuff		データバッファ.
//戻り値　：無し.
//備考　　：第２、３引数はデフォルトパラメータ対応.
//作成者　：高橋.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//************************************************************************************************************************.
void SSTC_SystemStatusService::msgQSend( SSTE_SysStatusMsgType e_MsgType, size_t ul_DataSize, void *pv_DataBuff )
{
	SST_LOG2(MFPE_LogLevelTrace, "e_MsgType=(0x%08x) ul_DataSize=(%ld) .\n", e_MsgType, ul_DataSize);
	
	SSTS_SysStatusMsg s_Message;
	
	// 初期化.
	memset( (char *)&s_Message, (char)SSTD_ZERO_OUT, sizeof( SSTS_SysStatusMsg ) );
	
	do {
		// データバッファ取得.
		char *pc_Buff = ( ul_DataSize == 0 ) ? NULL : new char [ ul_DataSize ];
		if( ul_DataSize > 0 && pc_Buff == NULL ){
// 2015.10.16 ZeusSZX0 制御76-1 新詳細コード s.
//			APIC_ThrowSystemError(SYSE_SysErrGeneral, LOGF_DEBUG_EVENT_KIND_SYS_STATUS, "SSTC_SystemStatusService msgQSend() new error.");
			APIC_ThrowSystemErrorNewDetailCode(SYSE_SysErrGeneral, LOGF_DEBUG_EVENT_KIND_SYS_STATUS, "SSTC_SystemStatusService msgQSend() new error.", 0, createDetailCode(0x304A));
// 2015.10.16 ZeusSZX0 制御76-1 新詳細コード e.
			break;
		}

		if(NULL != pc_Buff && NULL != pv_DataBuff) {    /* 静的解析 */
			memcpy( pc_Buff, pv_DataBuff, ul_DataSize );
		}

		// パラメータセット.
		s_Message.e_MsgType         = e_MsgType;						// ID.
		s_Message.ul_DataSize       = ul_DataSize;						// データサイズ.
		s_Message.p_ChangedDataBuff = pc_Buff;							// データ.
		
		// メッセージ送信.
		if (SYSF_msgQSend( m_msgQID, (char *)&s_Message, (uint)sizeof(SSTS_SysStatusMsg), WAIT_FOREVER, MSG_PRI_NORMAL ) == ERROR ) {
			// システムエラー処理.
// 2015.10.16 ZeusSZX0 制御76-1 新詳細コード s.
//			APIC_ThrowSystemErrorDetailCode( SYSE_SysErrTypeFatalSystemCallError, LOGF_DEBUG_EVENT_KIND_SYS_STATUS, (const char *)"SSTC_SystemStatusService msgQue() ERROR!" ,CE002_DetailCode0x5835);
			APIC_ThrowSystemErrorNewDetailCode( SYSE_SysErrTypeFatalSystemCallError, LOGF_DEBUG_EVENT_KIND_SYS_STATUS, (const char *)"SSTC_SystemStatusService msgQue() ERROR!", 0, createDetailCode(0x304B) );
// 2015.10.16 ZeusSZX0 制御76-1 新詳細コード e.
		}
	} while( 0 );
}
// STEP2：2010.12.10 <CHG> End   T.Takahashi.

//************************************************************************************************************************.
//概要　　：メッセージループ.
//戻り値　：無し.
//備考　　：特に無し.
//作成者　：谷本.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
// 2011.12.08, ADS坪井, CIT不具合対応[マージ後不具合管理票 No.164].
//  - initObserverData():オブザーバで取得するデータの初期化処理追加.
// 2012.08.07, ADS井上, FVT2不具合対応[CQ369958].
//  - 定期起床タイマ起動位置変更    起動位置を cyclicLoop() から messageLoop() へ.
//************************************************************************************************************************.
void SSTC_SystemStatusService::messageLoop()
{
	SST_LOG(MFPE_LogLevelTrace);
	
	SSTS_SysStatusMsg s_Message;
	
	while( True ){
		
		// 受信メッセージエリア初期化.
		memset( (char *)&s_Message, (char)SSTD_ZERO_OUT, sizeof(SSTS_SysStatusMsg) );


		// CQ383198調査用仕込み start.
		int qnum = 0;
		qnum = SYSF_msgQNumMsgs(m_msgQID);
		if ((SSTD_MSGQ_MAX - 1) <= qnum) {
			errorLog_sysServiceSsts("******************** SystemStatusService MsgQue %d Over! ******************** \n", (SSTD_MSGQ_MAX - 1));
		}
		// CQ383198調査用仕込み end.


		// メッセージ受信.
		if(SYSF_msgQReceive( m_msgQID, (char*)&s_Message, sizeof(SSTS_SysStatusMsg), WAIT_FOREVER ) == ERROR ){
// 2015.10.16 ZeusSZX0 制御76-1 新詳細コード s.
//			APIC_ThrowSystemErrorDetailCode(SYSE_SysErrTypeFatalSystemCallError, LOGF_DEBUG_EVENT_KIND_SYS_STATUS, "SSTC_SystemStatusService::messageLoop() SYSF_msgQReceive() == ERROR",CE002_DetailCode0x5836);
			APIC_ThrowSystemErrorNewDetailCode(SYSE_SysErrTypeFatalSystemCallError, LOGF_DEBUG_EVENT_KIND_SYS_STATUS, "SSTC_SystemStatusService::messageLoop() SYSF_msgQReceive() == ERROR", 0, createDetailCode(0x304C));
// 2015.10.16 ZeusSZX0 制御76-1 新詳細コード e.
			break;
		}
		
		SST_LOG2(MFPE_LogLevelAll, "s_Message.e_MsgType=(%d).\n", (int)s_Message.e_MsgType);
		switch( s_Message.e_MsgType ){
			case SSTE_SysStatusMsgType_SysStatusGetReq:					// システムステータス情報取得要求(ソケットデータ受信).
				m_pcInstance->procGetSystemStatus();
				break;
			case SSTE_SysStatusMsgType_StartChangeData:					// 変化データ通知開始要求(ソケットデータ受信).
				m_pcInstance->procStartChangeData();
				break;
			case SSTE_SysStatusMsgType_NotifyChangedData:				// 変化データ受信(Observer通知).
				m_pcInstance->procNotifyChangedData( s_Message.e_DataID, (uchar *)s_Message.p_ChangedDataBuff );
				break;
			case SSTE_SysStatusMsgType_WatchDogTimerExpire:				// 定期起床メッセージ受信.
				m_pcInstance->procTimerWakeUp();
				// 定期起床タイマ起動(2回目以降).
				SYSF_wdStart( m_wdogID, m_Tick, ( FUNCPTR )SSTC_SystemStatusService::cyclicLoop, 0 );
				break;
			case SSTE_SysStatusMsgType_TroubleReset:					// トラブルリセット(ソケットデータ受信).
				m_pcInstance->troubleReset();
				break;
			// STEP2：2010.12.10 <ADD> Start T.Takahashi.
			case SSTE_SysStatusMsgType_SetSystemStatusData:				// システムステータス更新要求(ソケットデータ受信).
				m_pcInstance->procSetSystemStatusData( s_Message.ul_DataSize, (uchar *)s_Message.p_ChangedDataBuff );
				break;
			case SSTE_SysStatusMsgType_SetNotificationSetting:			// 変化点通知設定要求(ソケットデータ受信).
				m_pcInstance->procSetNotificationSetting( s_Message.ul_DataSize, (uchar *)s_Message.p_ChangedDataBuff );
				break;
			// STEP2：2010.12.10 <ADD> End   T.Takahashi.
			case SSTE_SysStatusMsgType_ServiceInitialize:
				// オブザーバへのアタッチ.
				m_pcInstance->attachAllObserver();
				// オブザーバで取得するデータの初期化処理.
				m_pcInstance->initObserverData();
				break;
			case SSTE_SysStatusMsgType_UpdateObserverData:				// オブザーバデータ更新要求.
				// オブザーバで取得するデータを更新する.
				m_pcInstance->updateObserverData();
				break;
			case SSTE_SysStatusMsgType_UpdateSystemStatusData:			// SystemStatusデータ更新要求.
				// オブザーバで取得するデータを更新する.
				m_pcInstance->updateSystemStatusData();
				break;
			case SSTE_SysStatusMsgType_UpdateWarning:					// 警告情報更新要求.
				m_pcInstance->updateWarning();
				break;
			default:
				break;
		}
		// STEP2：2010.12.10 <ADD> Start T.Takahashi.
		if( s_Message.p_ChangedDataBuff != NULL ){
			SST_LOG2(MFPE_LogLevelTrace, "delete [0x%p].\n", s_Message.p_ChangedDataBuff);
			delete [] (char *)s_Message.p_ChangedDataBuff;
		}
		// STEP2：2010.12.10 <ADD> End   T.Takahashi.
	}
}

//************************************************************************************************************************.
//概要　　：スタティックなメンバー変数の初期化.
//戻り値　：メッセージキューID.
//備考　　：m_pcInstanceは除く.
//作成者　：高橋.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//************************************************************************************************************************.
void SSTC_SystemStatusService::initStaticMember()
{
	SST_LOG(MFPE_LogLevelTrace);
	m_wdogID = NULL;						// ウォッチドッグID.
	m_msgQID = NULL;						// メッセージキューID.
	m_semID  = 0;							// セマフォID.
	m_semIDShm = 0;							// セマフォID(共有メモリ).
	m_Tick   = 0;							// 定期起床タイマー計算値.
}

//************************************************************************************************************************.
//概要　　：メッセージキューID取得.
//戻り値　：メッセージキューID.
//備考　　：特に無し.
//作成者　：福井.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//************************************************************************************************************************.
MSG_Q_ID SSTC_SystemStatusService::getMsgQueID()
{
	return m_msgQID;
}

// <Act_31462> ADD start.
//************************************************************************************************************************.
//概要　　：リクエスト情報をリクエスト管理mapへ登録.
//引数　　：<in>　ul_ProcId			プロックID.
//引数　　：<in>　pst_Packet		パケットヘッダのアドレス.
//戻り値　：正常：MFPD_OK or 異常：MFPD_NG.
//備考　　：特に無し.
//作成者　：高橋.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//************************************************************************************************************************.
MFPD_Bool SSTC_SystemStatusService::addRequest( MFPD_ProcID ul_ProcId, MFPS_PacketHeader *pst_Packet )
{
	SST_LOG(MFPE_LogLevelAll);
	// 静的解析対応 2011.07.06 Start.
	MFPD_Bool ret = MFPD_NG;
	
	do {
		// パラメータチェック.
		if( pst_Packet == NULL ){
			SST_ERR_LOG(" pst_Packet == NULL.\n");
			break;
		}
		
		// 登録用のバッファを確保.
		MFPC_Request *pc_Req = new MFPC_Request( ul_ProcId );
		if( pc_Req == NULL ){
			SST_ERR_LOG("pc_Req == NULL.\n");
			break;
		}
	// 静的解析対応 2011.07.06 End.
		
		// リクエスト情報を登録.
		pc_Req->set_srcinfo( pst_Packet );
		ret = add_request( pc_Req );
	} while( 0 );
	
	SST_LOG2(MFPE_LogLevelTrace, "End. ret=(%d).\n", ret);
	return ret;
}

//************************************************************************************************************************.
//概要　　：リクエスト管理mapからリクエスト情報を削除.
//引数　　：<in>　ul_ProcId			プロックID.
//戻り値　：正常：MFPD_OK or 異常：MFPD_NG.
//備考　　：特に無し.
//作成者　：高橋.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//************************************************************************************************************************.
MFPD_Bool SSTC_SystemStatusService::delRequest( MFPD_ProcID ul_ProcId )
{
	SST_LOG(MFPE_LogLevelAll);
	MFPD_Bool     ret    = MFPD_NG;
	MFPC_Request *pc_Req = find_request( ul_ProcId );
	
	do {
		// 検索結果チェック.
		if( pc_Req == NULL ){
			SST_ERR_LOG("pc_Req == NULL.\n");
			break;
		}
		
		// リクエスト情報の削除.
		ret = del_request( ul_ProcId );
		
		delete pc_Req;
	} while( 0 );
	
	SST_LOG2(MFPE_LogLevelTrace, "End. ret=(%d).\n", ret);
	return ret;
}
// <Act_31462> ADD end.

//************************************************************************************************************************.
//概要　　：定期起床タイマ満了時処理.
//戻り値　：無し.
//備考　　：特に無し.
//作成者　：谷本.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
// 2012. 1.31 [CQ359780] getProgJobXXXStatusをポーリング→Observerに変更.
// 2012. 5.15 [CQ363516] パフォーマンス改善 ポーリング周期を500msと2sに変更.
//************************************************************************************************************************.
void SSTC_SystemStatusService::procTimerWakeUp()
{
	SST_LOG(MFPE_LogLevelAll);
	
	static unsigned char uc_WakeUpCount = 0;

	// 変化情報初期化.
	m_vChangedInfo.clear();
	
	// 情報取得フラグ設定.
	uc_WakeUpCount++;
	if (uc_WakeUpCount >= SSTD__CYCLIC_LONG_COUNT) {	// 長周期のカウンタ(SSTD__CYCLIC_MS * SSTD__CYCLIC_LONG_COUNT <2s>)の場合.
		uc_WakeUpCount = 0;

		getStatusTimerWakeUp();			// ステータス情報取得(定期起床) 全データ取得.

	} else {
		getStatusTimerWakeUp_High();	// ステータス情報取得(定期起床) (500ms)周期データ取得.
	}
	
	TYPE_NTF_GroupID  e_writeGroupID    = TYPE_NTF_GroupID_End;
	for( std::vector<SSTS_ChangedInfo>::iterator it = m_vChangedInfo.begin(); it != m_vChangedInfo.end(); ++it ){
		
		// グループ情報取得 該当なしの場合はエラーログ出力.
		TYPE_NTF_GroupID  e_GroupID    = TYPE_NTF_GroupID_End;
		void 			 *pv_GroupData = NULL;
		
		if( SSTC_SystemStatusService::getGroupInfo( (it)->e_DataID, &e_GroupID, &pv_GroupData ) == False ){
			// エラーログ出力.
			SST_ERR_LOG("getGroupInfo() == False.\n");
			continue;
		}
		if( e_GroupID != e_writeGroupID ){
			// 共有メモリ更新.
			m_pcInstance->lock();
			m_pcShMemMan->writeData( e_GroupID, pv_GroupData );
			m_pcInstance->unlock();
			e_writeGroupID = e_GroupID;
		}
	}
	
	if( m_bStartNotifyFlag == True ){
		for( std::vector<SSTS_ChangedInfo>::iterator it = m_vChangedInfo.begin(); it != m_vChangedInfo.end(); ++it ){
			// データ変化通知.
			notifyChangedData( (it)->e_DataID );
		}
	}
	// 変化データをクリア.
	m_vChangedInfo.clear();
}

//*****************************************************************************.
//概要　　：SystemStatusで取得するデータを更新する.
//引数　　：なし.
//戻り値　：なし.
//作成者　：ADS.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//*****************************************************************************.
void SSTC_SystemStatusService::updateSystemStatusData()
{
	SST_LOG(MFPE_LogLevelAll);

	// 変化情報初期化.
	m_vChangedInfo.clear();

	// ステータス情報取得(定期起床) 全データ取得.
	getStatusTimerWakeUp();

	TYPE_NTF_GroupID  e_writeGroupID    = TYPE_NTF_GroupID_End;
	for( std::vector<SSTS_ChangedInfo>::iterator it = m_vChangedInfo.begin(); it != m_vChangedInfo.end(); ++it ){
		// グループ情報取得 該当なしの場合はエラーログ出力.
		TYPE_NTF_GroupID  e_GroupID    = TYPE_NTF_GroupID_End;
		void 			 *pv_GroupData = NULL;
		
		if( SSTC_SystemStatusService::getGroupInfo( (it)->e_DataID, &e_GroupID, &pv_GroupData ) == False ){
			// エラーログ出力.
			SST_ERR_LOG("getGroupInfo() == False.\n");
			continue;
		}
		if( e_GroupID != e_writeGroupID ){
			// 共有メモリ更新.
			m_pcInstance->lock();
			m_pcShMemMan->writeData( e_GroupID, pv_GroupData );
			m_pcInstance->unlock();
			e_writeGroupID = e_GroupID;
		}
	}

	if( m_bStartNotifyFlag == True ){
		for( std::vector<SSTS_ChangedInfo>::iterator it = m_vChangedInfo.begin(); it != m_vChangedInfo.end(); ++it ){
			// データ変化通知.
			notifyChangedData( (it)->e_DataID );
		}
	}
	// 変化データをクリア.
	m_vChangedInfo.clear();

	sem_t* p_sem_ssts_dataupdate = sem_open( "/sem_ssts_dataupdate", O_RDWR );
	if( p_sem_ssts_dataupdate == SEM_FAILED ) {
		errorLog_sysServiceSsts( "/sem_ssts_dataupdate open() ERROR!! errno = [%d].\n", errno );
	}
	else {
		// SystemStatusデータ更新待ち解除.
		sem_post( p_sem_ssts_dataupdate );
		sem_close( p_sem_ssts_dataupdate );
	}
}

//************************************************************************************************************************.
//概要　　：システムステータス情報取得要求受付時処理.
//戻り値　：無し.
//備考　　：特に無し.
//作成者　：谷本.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
// 2012. 1.31 [CQ359780] getProgJobXXXStatusをポーリング→Observerに変更.
//************************************************************************************************************************.
void SSTC_SystemStatusService::procGetSystemStatus()
{
	SST_LOG(MFPE_LogLevelTrace);
	
	TYPD_SSTS_STATUS sc_Result  = TYPD_SSTS_StatusSuccess;
	
	// 警告情報とデバイス情報を更新する (定期起床時のメソッドを流用).
	getStatusTimerWakeUp();

	char sc_TempFilePath[ TYPD_SSTS_PathMax + 1 ];
	do {
		// 共有メモリへのデータ書き込み処理.
		m_pcInstance->lock();
		STATUS ret = m_pcShMemMan->writeData( TYPE_NTF_GroupID_All, (void *)&m_stSysStatInfo );
		m_pcInstance->unlock();
		if( ret == ERROR ){
			SST_ERR_LOG("writeData() == ERROR.\n");
			sc_Result = TYPD_SSTS_StatusError;
			break;
		}
		
		// ファイルへの出力.
		memset(&sc_TempFilePath[0], (char)SSTD_ZERO_OUT, sizeof(sc_TempFilePath) );
		sc_Result = writeSystemStatusInfo( &sc_TempFilePath[0], &m_stSysStatInfo, sizeof( m_stSysStatInfo ) );
		if( sc_Result != TYPD_SSTS_StatusSuccess ){
			SST_LOG2(MFPE_LogLevelError, "writeSystemStatusInfo() Failed! sc_Result=(%d).\n", sc_Result);
			break;
		}
	} while( 0 );
	
	// システムステータス情報取得応答.
	ans_getSystemStatusInfo( const_cast<char *>(&sc_TempFilePath[0]), sc_Result );
}

//************************************************************************************************************************.
//概要　　：変化点通知開始要求受付時処理.
//戻り値　：無し.
//備考　　：特に無し.
//作成者　：高橋.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//************************************************************************************************************************.
void SSTC_SystemStatusService::procStartChangeData()
{
	SST_LOG(MFPE_LogLevelTrace);
	
	do{
		// 変化点通知開始要求フラグチェック.
		if( m_bStartNotifyFlag == True ) { break; }
		
		// 変化点通知開始要求フラグ更新.
		m_bStartNotifyFlag = True;
		
		// 2021/10/22 APF OP_BTS-35403 start
		// 以下の流れで定期起床タイマ起動前に getStatusTimerWakeUp() がコールされている.
		//   ① [ntfm] case FUNCALL_SSTD_CheckInitComplete: FUNCALL_SSTD_GetSystemStatusInfo送信.
		//   ② [ssts] case FUNCALL_SSTD_GetSystemStatusInfo: SSTE_SysStatusMsgType_SysStatusGetReq送信.
		//   ③ [ssts] case SSTE_SysStatusMsgType_SysStatusGetReq: procGetSystemStatus() をコール.
		//   ④ [ssts] SSTC_SystemStatusService::procGetSystemStatus(): getStatusTimerWakeUp() をコール(定期起床時のメソッドを流用)
		// そのため、定期起床タイマ満了時と同等の変化点検知が行われており、ここで通知しなければ以降に変化点がなければ通知タイミングがないイベントがある.
		for( std::vector<TYPE_NTF_DataID>::iterator it = m_vStartNotifyDataID.begin(); it != m_vStartNotifyDataID.end(); ++it ){
			// データ変化通知.
			notifyChangedData( *it );
		}
		// 2021/10/22 APF OP_BTS-35403 end
		
		// 変化情報初期化.
		m_vChangedInfo.clear();
		
		// 定期起床タイマ起動.
		if( m_Tick == 0 ){
			m_Tick = (int32_t)SYSF_sysClkRateGet() * SSTD__CYCLIC_MS / 1000; 	// ｍｓ→ティック数に変換（タイマーを動作させる為、計算結果が０の場合は１ティックとする）.
			if( m_Tick == 0 ){ m_Tick = 1; }
			SYSF_wdStart( m_wdogID, m_Tick, (FUNCPTR)SSTC_SystemStatusService::cyclicLoop, 0 );
		}
	}while(0);
	
	// 変化点通知開始要求応答.
	ans_StartChangeData( TYPD_SSTS_StatusSuccess );
	
	SST_LOG2(MFPE_LogLevelTrace, "End. m_bStartNotifyFlag=(%d) m_Tick=(%ld).\n", m_bStartNotifyFlag, m_Tick);
}

//************************************************************************************************************************.
//概要　　：データ変化通知受信時処理.
//引数　　：<in>　e_DataID			データID.
//引数　　：<in>　puc_data			受信データ.
//戻り値　：無し.
//備考　　：特に無し.
//作成者　：谷本.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
// 2012. 1.31 [CQ359780] getProgJobXXXStatusをポーリング→Observerに変更.
// 2012. 5.15 [CQ363516] パフォーマンス改善 Observerで検出可能な情報をポーリング→Observerに変更.
//************************************************************************************************************************.
void SSTC_SystemStatusService::procNotifyChangedData(TYPE_NTF_DataID e_DataID, uchar* puc_data)
{
	SST_LOG2(MFPE_LogLevelTrace, "e_DataID=(0x%x) m_bStartNotifyFlag=(%d).\n", e_DataID, (int)m_bStartNotifyFlag);	
	
	do {

		Bool b_Post = notifyObserverData(e_DataID);
		if (b_Post != True) { break; }

		// 2011/05/10 <CHG> Start.
		// データを m_stSysStatInfo の該当データにセット.
		SSTE_NotifyData e_DataType = setChangedData( e_DataID, puc_data );
		if( e_DataType == SSTE_NotifyData_Unknown ){
			SST_ERR_LOG("setChangedData() Failed! SSTE_NotifyData_Unknown.\n");
			break;
		}
		
		if( e_DataType == SSTE_NotifyData_Exist ){
			// グループ情報を取得.
			TYPE_NTF_GroupID  e_GroupID    = TYPE_NTF_GroupID_End;
			void 			 *pv_GroupData = NULL;
			
			if( getGroupInfo( e_DataID, &e_GroupID, &pv_GroupData ) == False ){
				SST_ERR_LOG("getGroupInfo() Failed!.\n");
				break;
			}
			// 共有メモリ更新.
			m_pcInstance->lock();
			m_pcShMemMan->writeData( e_GroupID, pv_GroupData );
			m_pcInstance->unlock();
		}
		// 2011/05/10 <CHG> End.
		
		// データ変化通知.
		if( m_bStartNotifyFlag == True ){
			notifyChangedData( e_DataID );
		}
	} while( 0 );
}

//************************************************************************************************************************.
//概要　　：トラブルリセット処理.
//戻り値　：無し.
//備考　　：特に無し.
//作成者　：谷本.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//************************************************************************************************************************.
void SSTC_SystemStatusService::troubleReset()
{
	SST_LOG(MFPE_LogLevelTrace);
	
	// トラブルリセット実行処理.
	APIC_ScannerWarning::newInstance()->troubleReset();		// スキャナ警告.
	APIC_PrinterWarning::newInstance()->troubleReset();		// プリンタ警告.
	APIC_SystemStatus::newInstance()->troubleReset();		// システムステータス.
	
	// トラブルリセット応答.
	ans_TroubleReset( TYPD_SSTS_StatusSuccess );
}

// STEP2：2010.12.10 <ADD> Start T.Takahashi.
//************************************************************************************************************************.
//概要　　：システムステータス更新要求受付時処理.
//引数　　：<in>　us_DataSize		受信サイズ.
//引数　　：<in>　puc_Data			受信データ.
//戻り値　：無し.
//備考　　：特に無し.
//作成者　：高橋.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//************************************************************************************************************************.
void SSTC_SystemStatusService::procSetSystemStatusData( size_t ul_DataSize, uchar *puc_Data )
{
	SST_LOG(MFPE_LogLevelTrace);
	
	TYPD_SSTS_STATUS sc_Result = TYPD_SSTS_StatusError;
	
	do{
		//----- 最低限必要な受信サイズチェック ------.
		if( ul_DataSize < (sizeof(TYPE_NTF_DataID) + sizeof(int32_t)) ){
			SST_ERR_LOG("ul_DataSize < Minimum value .\n");
			break;
		}
		
		// 各データの取出し.
		TYPE_NTF_DataID e_DataID;                // データID.
		uint32_t ul_Size;                        // データサイズ.
		
		memcpy( &e_DataID, puc_Data, sizeof(TYPE_NTF_DataID) );
		puc_Data += sizeof(TYPE_NTF_DataID);
		
		memcpy( &ul_Size, puc_Data, sizeof(uint32_t) );
		puc_Data += sizeof(uint32_t);
		
		//----- 受信サイズチェック -----.
		if( ul_DataSize != ( sizeof(TYPE_NTF_DataID) + sizeof(uint32_t) + ul_Size ) ){
			SST_ERR_LOG("us_DataSize error .\n");
			break;
		}
		
		//===== パラメータの妥当性チェック =====.
		if( !chkSetSystemStatusDataParam( e_DataID, ul_Size ) ){
			SST_ERR_LOG("e_DataID or parameter size error.\n");
			break;
		}
		
		// 該当データ更新実行.
		setSystemStatusData( e_DataID, puc_Data );
		
		sc_Result = TYPD_SSTS_StatusSuccess;
	}while(0);
	
	// システムステータス更新要求応答.
	ans_SetSystemStatusData( sc_Result );
	
	SST_LOG2(MFPE_LogLevelTrace, "End. sc_Result=(%d).\n", (int)sc_Result );
}

//************************************************************************************************************************.
//概要　　：変化点通知設定要求受付時処理.
//引数　　：<in>　us_DataSize		受信サイズ.
//引数　　：<in>　puc_Data			受信データ.
//戻り値　：無し.
//備考　　：特に無し.
//作成者　：高橋.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//************************************************************************************************************************.
void SSTC_SystemStatusService::procSetNotificationSetting( size_t ul_DataSize, uchar *puc_Data )
{
	SST_LOG(MFPE_LogLevelTrace);
	
	TYPD_SSTS_STATUS sc_Result = TYPD_SSTS_StatusError;
	
	do{
		// 受信サイズチェック.
		if( ul_DataSize != ( sizeof(TYPE_NTF_GroupID) + SSTD_NotificationSettingDataSize) ){
			SST_ERR_LOG("us_DataSize != recvdat value .\n");
			break;
		}
		
		// GroupIDの取出しと妥当性判定.
		TYPE_NTF_GroupID e_GroupID = (TYPE_NTF_GroupID)*puc_Data;
		if( !checkGroupID( e_GroupID ) ){
			SST_ERR_LOG("GroupID Mismatch.\n");
			break;
		}
		
		// 設定データの先頭に移動.
		puc_Data    += sizeof( TYPE_NTF_GroupID );
		ul_DataSize -= sizeof( TYPE_NTF_GroupID );
		
		// 設定実行.
		setNotificationSetting( e_GroupID, ul_DataSize, puc_Data );
		
		sc_Result = TYPD_SSTS_StatusSuccess;
	}while(0);
	
	// 変化点通知設定要求応答.
	ans_NotificationSetting( sc_Result );
}
// STEP2：2010.12.10 <ADD> End   T.Takahashi.

//************************************************************************************************************************.
//概要　　：データＩＤからグループＩＤとグループデータを取得.
//引数　　：<in>　e_DataID			データID.
//引数　　：<in>　pe_GroupID		該当したグループID.
//引数　　：<in>　ppv_GroupData		該当したグループのデータの先頭.
//戻り値　：該当するグループIDが存在したか.
//備考　　：特に無し.
//作成者　：高橋.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//************************************************************************************************************************.
Bool SSTC_SystemStatusService::getGroupInfo( TYPE_NTF_DataID e_DataID, TYPE_NTF_GroupID *pe_GroupID, void **ppv_GroupData )
{
	SST_LOG(MFPE_LogLevelTrace);
	
	// グループIDと各データとの紐付けテーブル.
	struct ST_GroupDataTable {
		TYPE_NTF_GroupID	e_GroupID;		// グループID.
		void			   *pv_GroupData;	// グループデータ.
	} static st_GroupDataTable[] = {
		{ TYPE_NTF_GroupID_Warning		 , (void *)&m_stSysStatInfo.st_WarningInfoParam		  },
		{ TYPE_NTF_GroupID_RemotePanel	 , (void *)&m_stSysStatInfo.st_RemotePanelInfoParam	  },
		{ TYPE_NTF_GroupID_Address		 , (void *)&m_stSysStatInfo.st_AddressInfoParam		  },
		{ TYPE_NTF_GroupID_Service		 , (void *)&m_stSysStatInfo.st_ServiceInfoParam		  },
		{ TYPE_NTF_GroupID_System		 , (void *)&m_stSysStatInfo.st_SystemInfoParam		  },
		{ TYPE_NTF_GroupID_Scanner		 , (void *)&m_stSysStatInfo.st_ScannerInfoParam		  },
		{ TYPE_NTF_GroupID_Commutication , (void *)&m_stSysStatInfo.st_CommunicateInfoParam	  },
		{ TYPE_NTF_GroupID_PrinterDevice , (void *)&m_stSysStatInfo.st_PrinterDeviceInfoParam },
		{ TYPE_NTF_GroupID_Device		 , (void *)&m_stSysStatInfo.st_DeviceInfoParam		  },
		{ TYPE_NTF_GroupID_Network		 , (void *)&m_stSysStatInfo.st_NetworkInfoParam		  },
// 2014.01.09-STA  パネルHW情報の取得.
//		{ TYPE_NTF_GroupID_Panel		 , (void *)NULL										  },
		{ TYPE_NTF_GroupID_Panel		 , (void *)&m_stSysStatInfo.st_PanelInfoParam		  },
// 2014.01.09-END  パネルHW情報の取得.
		{ TYPE_NTF_GroupID_Other		 , (void *)&m_stSysStatInfo.st_OtherInfoParam		  },
		{ TYPE_NTF_GroupID_End			 , (void *)NULL										  }
	};
	Bool b_ExistFlag = False;
	for( struct ST_GroupDataTable *pst_GroupDataTbl = &st_GroupDataTable[ 0 ]; pst_GroupDataTbl->e_GroupID != TYPE_NTF_GroupID_End; pst_GroupDataTbl++ ){
		if( ( ( TYPE_NTF_GroupID )( e_DataID >> SSTD_Short_Bits ) ) == pst_GroupDataTbl->e_GroupID ){
			*pe_GroupID    = pst_GroupDataTbl->e_GroupID;
			*ppv_GroupData = pst_GroupDataTbl->pv_GroupData;
			b_ExistFlag    = True;
			break;
		}
	}
	SST_LOG2(MFPE_LogLevelTrace, "End. e_DataID=(0x%08x)  pe_GroupID=(0x%04x).\n", e_DataID,  *pe_GroupID);
	return( b_ExistFlag );
}

//************************************************************************************************************************.
//概要　　：変化データ登録処理.
//引数　　：<in>　e_DataID			データID.
//引数　　：<in>　puc_Data			受信データ.
//戻り値　：受信したデータIDの判定結果<不明、書込みデータ無、書込みデータ有>.
//備考　　：特に無し.
//作成者　：高橋.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
// 2011.12.08, ADS坪井, CIT不具合対応[マージ後不具合管理票 No.164].
//  - APIC_FaxObserver::xxx()通知情報     st_DevXX->st_DevXX[TYPE_CommLine2].
//  - アクセス状態変化通知（アプリ単位）  st_Dev8->st_Dev8[TYPE_UsbMemAppIdAll].
// 2011.12.15. ADS データなしObserverのデータ追加対応.
// 2012/09/13, ADS坪井, CQ369866:Android接続中通知追加.
//************************************************************************************************************************.
// 2011/05/10 <CHG> Start.
SSTE_NotifyData SSTC_SystemStatusService::setChangedData( TYPE_NTF_DataID e_DataID, uchar* puc_Data )
// 2011/05/10 <CHG> End.
{
	SST_LOG(MFPE_LogLevelAll);
	
	SSTE_NotifyData e_DataType = SSTE_NotifyData_Unknown;	// 2011/05/10 <CHG>.
	
	switch( e_DataID ){
		// 通知データが伴わないもの.
		case TYPE_NTF_DataID_COM_2:
		case TYPE_NTF_DataID_COM_3:
		case TYPE_NTF_DataID_COM_4:
//		case TYPE_NTF_DataID_COM_5:
		case TYPE_NTF_DataID_COM_6:
		case TYPE_NTF_DataID_DEV_5:
		case TYPE_NTF_DataID_DEV_13:
		case TYPE_NTF_DataID_DEV_18:
		case TYPE_NTF_DataID_DEV_19:
		case TYPE_NTF_DataID_DEV_22:
		case TYPE_NTF_DataID_DEV_23:
		case TYPE_NTF_DataID_DEV_24:
		case TYPE_NTF_DataID_DEV_25:
		case TYPE_NTF_DataID_DEV_26:
		case TYPE_NTF_DataID_DEV_27:
		case TYPE_NTF_DataID_DEV_28:
		case TYPE_NTF_DataID_DEV_29:
		case TYPE_NTF_DataID_DEV_30:
		case TYPE_NTF_DataID_DEV_31:
		case TYPE_NTF_DataID_DEV_32:
		case TYPE_NTF_DataID_DEV_33:
		case TYPE_NTF_DataID_DEV_34:
		case TYPE_NTF_DataID_DEV_35:
		case TYPE_NTF_DataID_DEV_36:
		case TYPE_NTF_DataID_DEV_37:
//		case TYPE_NTF_DataID_DEV_39:
		case TYPE_NTF_DataID_DEV_40:
		case TYPE_NTF_DataID_DEV_41:
		case TYPE_NTF_DataID_DEV_42:
		case TYPE_NTF_DataID_DEV_43:
		case TYPE_NTF_DataID_DEV_44:
		case TYPE_NTF_DataID_DEV_45:
		case TYPE_NTF_DataID_DEV_47:
		case TYPE_NTF_DataID_DEV_48:
		case TYPE_NTF_DataID_DEV_49:
		case TYPE_NTF_DataID_DEV_50:
		case TYPE_NTF_DataID_DEV_51:
		case TYPE_NTF_DataID_DEV_52:
//		case TYPE_NTF_DataID_DEV_53:
		case TYPE_NTF_DataID_DEV_54:
		case TYPE_NTF_DataID_DEV_55:
//		case TYPE_NTF_DataID_DEV_56:
//		case TYPE_NTF_DataID_DEV_57:
		case TYPE_NTF_DataID_DEV_58:
		case TYPE_NTF_DataID_DEV_59:
		case TYPE_NTF_DataID_DEV_60:
		case TYPE_NTF_DataID_DEV_61:
		case TYPE_NTF_DataID_DEV_63:
		case TYPE_NTF_DataID_DEV_64:
		case TYPE_NTF_DataID_DEV_65:
		case TYPE_NTF_DataID_DEV_66:
		case TYPE_NTF_DataID_DEV_67:
		case TYPE_NTF_DataID_DEV_68:
		case TYPE_NTF_DataID_DEV_69:
		case TYPE_NTF_DataID_DEV_70:
		case TYPE_NTF_DataID_DEV_71:
		case TYPE_NTF_DataID_DEV_72:
		case TYPE_NTF_DataID_DEV_73:
		case TYPE_NTF_DataID_DEV_74:
		case TYPE_NTF_DataID_DEV_75:
		case TYPE_NTF_DataID_DEV_78:
		case TYPE_NTF_DataID_DEV_81:
		case TYPE_NTF_DataID_NET_2:
		case TYPE_NTF_DataID_NET_4:
		case TYPE_NTF_DataID_NET_5:
		case TYPE_NTF_DataID_NET_7:
		case TYPE_NTF_DataID_NET_8:
		case TYPE_NTF_DataID_NET_9:
		case TYPE_NTF_DataID_NET_10:
		case TYPE_NTF_DataID_NET_11:
		case TYPE_NTF_DataID_NET_12:
		case TYPE_NTF_DataID_NET_13:
//		case TYPE_NTF_DataID_NET_14:
		case TYPE_NTF_DataID_NET_15:
		case TYPE_NTF_DataID_NET_16:
		case TYPE_NTF_DataID_NET_17:
		case TYPE_NTF_DataID_NET_18:
		case TYPE_NTF_DataID_NET_19:
		case TYPE_NTF_DataID_NET_20:
		case TYPE_NTF_DataID_NET_21:
		case TYPE_NTF_DataID_NET_22:
		case TYPE_NTF_DataID_NET_23:
		case TYPE_NTF_DataID_NET_24:
//		case TYPE_NTF_DataID_NET_25:
		case TYPE_NTF_DataID_NET_26:
		case TYPE_NTF_DataID_NET_27:
		case TYPE_NTF_DataID_NET_28:
		case TYPE_NTF_DataID_NET_29:
		case TYPE_NTF_DataID_NET_30:
		case TYPE_NTF_DataID_NET_31:
		case TYPE_NTF_DataID_NET_33:
		case TYPE_NTF_DataID_NET_34:
		case TYPE_NTF_DataID_NET_35:
		case TYPE_NTF_DataID_NET_36:
		case TYPE_NTF_DataID_NET_37:
		case TYPE_NTF_DataID_NET_38:
		case TYPE_NTF_DataID_NET_40:
		case TYPE_NTF_DataID_NET_41:
		case TYPE_NTF_DataID_NET_42:
		case TYPE_NTF_DataID_NET_43:
		case TYPE_NTF_DataID_NET_44:
		case TYPE_NTF_DataID_NET_45:
		case TYPE_NTF_DataID_NET_46:
		case TYPE_NTF_DataID_NET_47:
//		case TYPE_NTF_DataID_OTH_1:
		case TYPE_NTF_DataID_OTH_3:
		case TYPE_NTF_DataID_OTH_4:
		case TYPE_NTF_DataID_OTH_5:
		case TYPE_NTF_DataID_OTH_8:
		case TYPE_NTF_DataID_PNL_1:
		case TYPE_NTF_DataID_PNL_2:
		case TYPE_NTF_DataID_PNL_3:
		case TYPE_NTF_DataID_PNL_4:
		case TYPE_NTF_DataID_DEV_PRT_107:
		case TYPE_NTF_DataID_DEV_PRT_108:
//		case TYPE_NTF_DataID_DEV_PRT_109:
//		case TYPE_NTF_DataID_DEV_PRT_110:
		case TYPE_NTF_DataID_DEV_PRT_111:
		case TYPE_NTF_DataID_DEV_PRT_112:
//		case TYPE_NTF_DataID_RPL_1:
//		case TYPE_NTF_DataID_RPL_2:
		case TYPE_NTF_DataID_SCN_1:
		case TYPE_NTF_DataID_SCN_2:
		case TYPE_NTF_DataID_SCN_3:
//		case TYPE_NTF_DataID_SCN_4:
//		case TYPE_NTF_DataID_SCN_5:
//		case TYPE_NTF_DataID_SCN_6:
//		case TYPE_NTF_DataID_SCN_7:
		case TYPE_NTF_DataID_SCN_8:
		case TYPE_NTF_DataID_SRV_6:
		case TYPE_NTF_DataID_SRV_7:
		case TYPE_NTF_DataID_SRV_8:
		case TYPE_NTF_DataID_SRV_9:
		case TYPE_NTF_DataID_SYS_1:
		case TYPE_NTF_DataID_SYS_2:
//		case TYPE_NTF_DataID_SYS_3:
		case TYPE_NTF_DataID_SYS_5:
		case TYPE_NTF_DataID_SYS_6:
		case TYPE_NTF_DataID_SYS_7:
//		case TYPE_NTF_DataID_SYS_8:
		case TYPE_NTF_DataID_SYS_11:
		case TYPE_NTF_DataID_SYS_12:
//		case TYPE_NTF_DataID_SYS_13:
		// 新API対応：2011.05.26 <ADD> Start T.Takahashi.
		case TYPE_NTF_DataID_SYS_87:
		case TYPE_NTF_DataID_SYS_88:
		case TYPE_NTF_DataID_SYS_89:
		case TYPE_NTF_DataID_SYS_90:
		case TYPE_NTF_DataID_SYS_91:
		// 新API対応：2011.05.26 <ADD> End T.Takahashi.
// CQ376793 HDDフォーマット中状態の共有メモリへの追加 2013/1/7 Soma Start.
//		case TYPE_NTF_DataID_SYS_94:			// HDD関連通知対応 2011.06.30 T.Takahashi.
// CQ376793 HDDフォーマット中状態の共有メモリへの追加 2013/1/7 Soma End.
		case TYPE_NTF_DataID_SYS_95:			// 認証プラグイン対応. ADS 2011.11.01.
// 2013/06/26 制御構成変更No1201"HDDイレース後レポート出力対応" add start.
		case TYPE_NTF_DataID_SYS_110:
// 2013/06/26 制御構成変更No1201"HDDイレース後レポート出力対応" add end.
		case TYPE_NTF_DataID_RPL_7:
// comment 2014/1/15 OPS1-Sky南波 [特Rom対応][BK2ND-6][機種マージ]Reiffeisen過去対応のBK機種展開要望 Start.
		case TYPE_NTF_DataID_SYS_111:
// comment 2014/1/15 OPS1-Sky南波 [特Rom対応][BK2ND-6][機種マージ]Reiffeisen過去対応のBK機種展開要望 End.
// 【RQ-747 VirusScanFUM for FVT2】 2018.04.23 Sky.A.Tamura Add Start ->.
		case TYPE_NTF_DataID_OTH_15:
// 【RQ-747 VirusScanFUM for FVT2】 2018.04.23 Sky.A.Tamura Add End   <-.
// 【RQ-747 VirusScanFUM for FVT2】 2018.05.23 Sky.K.Ogawa Add Start ->.
		case TYPE_NTF_DataID_OTH_16:
// 【RQ-747 VirusScanFUM for FVT2】 2018.05.23 Sky.K.Ogawa Add End   <-.
			e_DataType = SSTE_NotifyData_NotExist;	// 2011/05/10 <CHG>.
			break;
	    
		default:						// 通知データが存在する、またはＩＤ異常.
			struct ST_DataChangeTabl {			// データIDとデータの紐付けテーブル.
				TYPE_NTF_DataID	e_DataID;		// データID.
				ushort			us_DataSize;	// データサイズ.
				void		   *pv_Data;		// データ格納アドレス.
			} static st_DataChangeTable[] = {
				{ TYPE_NTF_DataID_ADR_1 , sizeof(TYPE_COMMON_Result)                    , &( m_stSysStatInfo.st_AddressInfoParam.e_Adr1 )	   },
				{ TYPE_NTF_DataID_ADR_2 , sizeof(TYPS_SysStatusAddressNumber)           , &( m_stSysStatInfo.st_AddressInfoParam.st_Adr2 )	   },
				{ TYPE_NTF_DataID_ADR_3 , sizeof(TYPE_COMMON_Result)                    , &( m_stSysStatInfo.st_AddressInfoParam.e_Adr3 )	   },
				{ TYPE_NTF_DataID_COM_1 , sizeof(uchar)                                 , &( m_stSysStatInfo.st_CommunicateInfoParam.uc_Com1 ) },
				{ TYPE_NTF_DataID_COM_5 , sizeof(TYPS_SysStatusRds) 					, &( m_stSysStatInfo.st_CommunicateInfoParam.st_Com5 ) },	// [SystemStatus] データなしObserverのデータ追加対応：2011.12.15 <ADD>.
				{ TYPE_NTF_DataID_COM_7 , sizeof(TYPE_RdsTrgKind)                       , &( m_stSysStatInfo.st_CommunicateInfoParam.e_Com7 )  },
// CQ375788対応 12/12/12 ->.
//				{ TYPE_NTF_DataID_DEV_1 , sizeof(TYPS_CallerID) * TYPE_CommLine2        , &( m_stSysStatInfo.st_DeviceInfoParam.st_Dev1 )	   },
//				{ TYPE_NTF_DataID_DEV_2 , sizeof(TYPS_CallerName) * TYPE_CommLine2      , &( m_stSysStatInfo.st_DeviceInfoParam.st_Dev2 )	   },
				{ TYPE_NTF_DataID_DEV_1 , sizeof(TYPS_CallerID) * TYPE_CommLine4        , &( m_stSysStatInfo.st_DeviceInfoParam.st_Dev1 )	   },
				{ TYPE_NTF_DataID_DEV_2 , sizeof(TYPS_CallerName) * TYPE_CommLine4      , &( m_stSysStatInfo.st_DeviceInfoParam.st_Dev2 )	   },
// CQ375788対応 12/12/12 <-.
				{ TYPE_NTF_DataID_DEV_3 , sizeof(TYPS_SysStatusIGCdeviceList)           , &( m_stSysStatInfo.st_DeviceInfoParam.st_Dev3 )	   },
				{ TYPE_NTF_DataID_DEV_4 , sizeof(TYPE_ChangePowerSave)                  , &( m_stSysStatInfo.st_DeviceInfoParam.e_Dev4 )	   },
				{ TYPE_NTF_DataID_DEV_6 , sizeof(TYPE_PmCnt)                            , &( m_stSysStatInfo.st_DeviceInfoParam.e_Dev6 )	   },
				{ TYPE_NTF_DataID_DEV_7 , sizeof(uint32_t)                              , &( m_stSysStatInfo.st_DeviceInfoParam.ul_Dev7 )	   },
				{ TYPE_NTF_DataID_DEV_8 , sizeof(TYPS_SysStatusUsbMemDeviceAppAccess) * TYPE_UsbMemAppIdAll , &( m_stSysStatInfo.st_DeviceInfoParam.st_Dev8 )	   },
				{ TYPE_NTF_DataID_DEV_9 , sizeof(uint32_t)                              , &( m_stSysStatInfo.st_DeviceInfoParam.ul_Dev9 )	   },
				{ TYPE_NTF_DataID_DEV_10, sizeof(TYPE_CountDown)                        , &( m_stSysStatInfo.st_DeviceInfoParam.c_Dev10 )	   },
				{ TYPE_NTF_DataID_DEV_11, sizeof(Bool)                                  , &( m_stSysStatInfo.st_DeviceInfoParam.b_Dev11 )	   },
// CQ375788対応 12/12/12 ->.
//				{ TYPE_NTF_DataID_DEV_12, sizeof(TYPE_FaxErrKind) * TYPE_CommLine2      , &( m_stSysStatInfo.st_DeviceInfoParam.e_Dev12 )	   },
				{ TYPE_NTF_DataID_DEV_12, sizeof(TYPE_FaxErrKind) * TYPE_CommLine4      , &( m_stSysStatInfo.st_DeviceInfoParam.e_Dev12 )	   },
// CQ375788対応 12/12/12 <-.
				{ TYPE_NTF_DataID_DEV_14, sizeof(TYPE_ExchangeLifeCnt)                  , &( m_stSysStatInfo.st_DeviceInfoParam.e_Dev14 )	   },
				{ TYPE_NTF_DataID_DEV_15, sizeof(uint32_t)                              , &( m_stSysStatInfo.st_DeviceInfoParam.ul_Dev15 )	   },
// CQ375788対応 12/12/12 ->.
//				{ TYPE_NTF_DataID_DEV_16, sizeof(FAXE_DetectRs) * TYPE_CommLine2        , &( m_stSysStatInfo.st_DeviceInfoParam.e_Dev16 )	   },
//				{ TYPE_NTF_DataID_DEV_17, sizeof( FAXS_CmdRs) * TYPE_CommLine2          , &( m_stSysStatInfo.st_DeviceInfoParam.st_Dev17 )	   },
				{ TYPE_NTF_DataID_DEV_16, sizeof(FAXE_DetectRs) * TYPE_CommLine4        , &( m_stSysStatInfo.st_DeviceInfoParam.e_Dev16 )	   },
				{ TYPE_NTF_DataID_DEV_17, sizeof( FAXS_CmdRs) * TYPE_CommLine4          , &( m_stSysStatInfo.st_DeviceInfoParam.st_Dev17 )	   },
// CQ375788対応 12/12/12 <-.
				{ TYPE_NTF_DataID_DEV_20, sizeof(TYPS_SysStatusAuthDeviceCommon)        , &( m_stSysStatInfo.st_DeviceInfoParam.st_Dev20 )	   },
				{ TYPE_NTF_DataID_DEV_21, sizeof(uint32_t)                              , &( m_stSysStatInfo.st_DeviceInfoParam.ul_Dev21 )	   },
				{ TYPE_NTF_DataID_DEV_38, sizeof(TYPE_ExchangeLifeCnt)                  , &( m_stSysStatInfo.st_DeviceInfoParam.e_Dev38 )	   },
				{ TYPE_NTF_DataID_DEV_39, sizeof(TYPS_SysStatusFeedTray)				, &( m_stSysStatInfo.st_DeviceInfoParam.st_Dev39 )	   },	// [SystemStatus] データなしObserverのデータ追加対応：2011.12.15 <ADD>.
// CQ375788対応 12/12/12 ->.
//				{ TYPE_NTF_DataID_DEV_46, sizeof(Bool) * TYPE_CommLine2                 , &( m_stSysStatInfo.st_DeviceInfoParam.b_Dev46 )	   },
				{ TYPE_NTF_DataID_DEV_46, sizeof(Bool) * TYPE_CommLine4                 , &( m_stSysStatInfo.st_DeviceInfoParam.b_Dev46 )	   },
// CQ375788対応 12/12/12 <-.
				// [SystemStatus] データなしObserverのデータ追加対応：2011.12.15 <ADD> Start.
				{ TYPE_NTF_DataID_DEV_53, sizeof(TYPS_SysStatusMemoryInfo)				, &( m_stSysStatInfo.st_DeviceInfoParam.st_Dev53 )	   },
				{ TYPE_NTF_DataID_DEV_56, sizeof(TYPS_SysStatusOriginal)				, &( m_stSysStatInfo.st_DeviceInfoParam.st_Dev56 )	   },
				{ TYPE_NTF_DataID_DEV_57, sizeof(TYPS_SysStatusOutputTray)				, &( m_stSysStatInfo.st_DeviceInfoParam.st_Dev57 )	   },
				// [SystemStatus] データなしObserverのデータ追加対応：2011.12.15 <ADD> End.
				{ TYPE_NTF_DataID_DEV_62, sizeof(TYPS_SysStatusNewPowerStatusChanged)   , &( m_stSysStatInfo.st_DeviceInfoParam.st_Dev62 )	   },
// CQ375788対応 12/12/12 ->.
//				{ TYPE_NTF_DataID_DEV_76, sizeof(TYPE_TestResult) * TYPE_CommLine2      , &( m_stSysStatInfo.st_DeviceInfoParam.e_Dev76 )	   },
				{ TYPE_NTF_DataID_DEV_76, sizeof(TYPE_TestResult) * TYPE_CommLine4      , &( m_stSysStatInfo.st_DeviceInfoParam.e_Dev76 )	   },
// CQ375788対応 12/12/12 <-.
				{ TYPE_NTF_DataID_DEV_77, sizeof(Bool)                                  , &( m_stSysStatInfo.st_DeviceInfoParam.b_Dev77 )	   },
// CQ375788対応 12/12/12 ->.
//				{ TYPE_NTF_DataID_DEV_79, sizeof(FAXS_CommInfoKPM) * TYPE_CommLine2     , &( m_stSysStatInfo.st_DeviceInfoParam.st_Dev79 )	   },
//				{ TYPE_NTF_DataID_DEV_80, sizeof(FAXE_InfoRs) * TYPE_CommLine2          , &( m_stSysStatInfo.st_DeviceInfoParam.e_Dev80 )	   },
				{ TYPE_NTF_DataID_DEV_79, sizeof(FAXS_CommInfoKPM) * TYPE_CommLine4     , &( m_stSysStatInfo.st_DeviceInfoParam.st_Dev79 )	   },
				{ TYPE_NTF_DataID_DEV_80, sizeof(FAXE_InfoRs) * TYPE_CommLine4          , &( m_stSysStatInfo.st_DeviceInfoParam.e_Dev80 )	   },
// CQ375788対応 12/12/12 <-.
				{ TYPE_NTF_DataID_NET_3 , sizeof(TYPE_SnmpTrapEvent)                    , &( m_stSysStatInfo.st_NetworkInfoParam.e_Net3 )	   },
				{ TYPE_NTF_DataID_NET_6 , sizeof(uchar)                                 , &( m_stSysStatInfo.st_NetworkInfoParam.uc_Net6 )	   },
				// [SystemStatus] データなしObserverのデータ追加対応：2011.12.15 <ADD> Start.
				{ TYPE_NTF_DataID_NET_14, sizeof(TYPS_SysStatusInternetISW)             , &( m_stSysStatInfo.st_NetworkInfoParam.st_Net14 )	   },
				{ TYPE_NTF_DataID_NET_25, sizeof(TYPS_SysStatusNetworkDevice)           , &( m_stSysStatInfo.st_NetworkInfoParam.st_Net25 )	   },
				// [SystemStatus] データなしObserverのデータ追加対応：2011.12.15 <ADD> End.
				{ TYPE_NTF_DataID_NET_32, sizeof(short)                                 , &( m_stSysStatInfo.st_NetworkInfoParam.ss_Net32 )	   },
				{ TYPE_NTF_DataID_NET_39, sizeof(TYPE_SsdpParam)                        , &( m_stSysStatInfo.st_NetworkInfoParam.e_Net39 )	   },
				{ TYPE_NTF_DataID_OTH_1 , sizeof(TYPS_SysStatusSearchablePDFSetting)	, &( m_stSysStatInfo.st_OtherInfoParam.st_Oth1 )	   },	// [SystemStatus] データなしObserverのデータ追加対応：2011.12.15 <ADD>.
				{ TYPE_NTF_DataID_OTH_2 , sizeof(TYPE_FaxListReportSettingObserver) 	, &( m_stSysStatInfo.st_OtherInfoParam.e_Oth2 )		   },
				{ TYPE_NTF_DataID_OTH_6 , sizeof(TYPS_SysStatusDummyIFFaultNotify)      , &( m_stSysStatInfo.st_OtherInfoParam.st_Oth6 )	   },
				{ TYPE_NTF_DataID_OTH_7 , sizeof(uint)                                  , &( m_stSysStatInfo.st_OtherInfoParam.e_Oth7 )		   },
				// [SystemStatus] データなしObserverのデータ追加対応：2011.12.15 <ADD> Start.
				{ TYPE_NTF_DataID_DEV_PRT_109 , sizeof(TYPS_SysStatusPrinterServiceInfo), &( m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt109 ) },
				{ TYPE_NTF_DataID_DEV_PRT_110 , sizeof(TYPS_SysStatusPrinterServiceSetting)	, &( m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt110 ) },
// 2012/11/22 CQ375479 Sky福原 start↓.
				{ TYPE_NTF_DataID_DEV_PRT_142 , sizeof(TYPS_PaperSize) * TYPD_SSTS_FeedTrayMax , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt142 ) }, 
				{ TYPE_NTF_DataID_DEV_PRT_143 , sizeof(TYPS_PaperType) * TYPD_SSTS_FeedTrayMax , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt143 ) }, 
// 2012/11/22 CQ375479 Sky福原 end↓.
				{ TYPE_NTF_DataID_RPL_1 , sizeof(TYPS_SysStatusPrinterSystem)					, &( m_stSysStatInfo.st_RemotePanelInfoParam.st_Rpl1 ) },
				{ TYPE_NTF_DataID_RPL_2 , sizeof(TYPS_SysStatusRemotePanelStatusFiery)			, &( m_stSysStatInfo.st_RemotePanelInfoParam.st_Rpl2 ) },
				{ TYPE_NTF_DataID_RPL_3 , sizeof(TYPS_SysStatusRemotePanelStatusRequestDisplay)	, &( m_stSysStatInfo.st_RemotePanelInfoParam.st_Rpl3 ) },
				{ TYPE_NTF_DataID_RPL_4 , sizeof(TYPS_SysStatusRemotePanelStatusCommunication)	, &( m_stSysStatInfo.st_RemotePanelInfoParam.st_Rpl4 ) },
				{ TYPE_NTF_DataID_RPL_5 , sizeof(TYPS_SysStatusRemotePanelStatusFunctionNotice)	, &( m_stSysStatInfo.st_RemotePanelInfoParam.st_Rpl5 ) },
				{ TYPE_NTF_DataID_RPL_6 , sizeof(TYPS_SysStatusRemotePanelStatusStatusNotice)	, &( m_stSysStatInfo.st_RemotePanelInfoParam.st_Rpl6 ) },
				{ TYPE_NTF_DataID_RPL_8 , sizeof(TYPS_SysStatusRemotePanelStatusRMP_Message)	, &( m_stSysStatInfo.st_RemotePanelInfoParam.st_Rpl8 ) },
				{ TYPE_NTF_DataID_RPL_9 , sizeof(TYPS_SysStatusRemotePanelStatusRMP_Message)	, &( m_stSysStatInfo.st_RemotePanelInfoParam.st_Rpl9 ) },
				{ TYPE_NTF_DataID_RPL_10, sizeof(Bool)											, &( m_stSysStatInfo.st_RemotePanelInfoParam.b_Rpl10 ) },

				{ TYPE_NTF_DataID_SCN_4 , sizeof(TYPS_SysStatusScannerServiceInfo)		, &( m_stSysStatInfo.st_ScannerInfoParam.st_Scn4 )	   },
				{ TYPE_NTF_DataID_SCN_5 , sizeof(TYPS_SysStatusScannerServiceSetting)	, &( m_stSysStatInfo.st_ScannerInfoParam.st_Scn5 )	   },
				{ TYPE_NTF_DataID_SCN_6 , sizeof(TYPS_SysStatusScannerStatus)			, &( m_stSysStatInfo.st_ScannerInfoParam.st_Scn6 )	   },
				{ TYPE_NTF_DataID_SCN_7 , sizeof(TYPS_SysStatusScannerSystem)			, &( m_stSysStatInfo.st_ScannerInfoParam.st_Scn7 )	   },
				// [SystemStatus] データなしObserverのデータ追加対応：2011.12.15 <ADD> End.
				{ TYPE_NTF_DataID_SRV_1 , sizeof(short)                                 , &( m_stSysStatInfo.st_ServiceInfoParam.ss_Srv1 )	   },
				{ TYPE_NTF_DataID_SRV_2 , sizeof(JLGE_Observer_FileSts)                 , &( m_stSysStatInfo.st_ServiceInfoParam.e_Srv2 )	   },
				{ TYPE_NTF_DataID_SRV_3 , sizeof(JLGE_Observer_DataSts)                 , &( m_stSysStatInfo.st_ServiceInfoParam.e_Srv3 )	   },
				{ TYPE_NTF_DataID_SRV_4 , sizeof(JLGE_Observer_LogSts)                  , &( m_stSysStatInfo.st_ServiceInfoParam.e_Srv4 )	   },
				{ TYPE_NTF_DataID_SRV_5 , sizeof(JLGE_Observer_JobLogSts)               , &( m_stSysStatInfo.st_ServiceInfoParam.e_Srv5 )	   },
				{ TYPE_NTF_DataID_SYS_3 , sizeof(TYPS_SysStatusFatal)					, &( m_stSysStatInfo.st_SystemInfoParam.st_Sys3 )	   },	// [SystemStatus] データなしObserverのデータ追加対応：2011.12.15 <ADD>.
				{ TYPE_NTF_DataID_SYS_4 , sizeof(Bool)                                  , &( m_stSysStatInfo.st_SystemInfoParam.b_Sys4 )	   },
				{ TYPE_NTF_DataID_SYS_8 , sizeof(TYPS_SysStatusSystem)					, &( m_stSysStatInfo.st_SystemInfoParam.st_Sys8 )	   },	// [SystemStatus] データなしObserverのデータ追加対応：2011.12.15 <ADD>.
				{ TYPE_NTF_DataID_SYS_9 , sizeof(TYPE_BmlPrintStatus)                   , &( m_stSysStatInfo.st_SystemInfoParam.e_Sys9 )	   },
				{ TYPE_NTF_DataID_SYS_10, sizeof(TYPE_BmlinksParam)                     , &( m_stSysStatInfo.st_SystemInfoParam.e_Sys10 )	   },
				{ TYPE_NTF_DataID_SYS_13, sizeof(TYPS_SysStatusHddInfo) 				, &( m_stSysStatInfo.st_SystemInfoParam.st_Sys13 )	   },	// [SystemStatus] データなしObserverのデータ追加対応：2011.12.15 <ADD>.
				// 新API対応：2011.05.26 <ADD> Start T.Takahashi.
				{ TYPE_NTF_DataID_SYS_86, sizeof(TYPS_SysStatusPMInfo)                  , &( m_stSysStatInfo.st_SystemInfoParam.st_Sys86 )	   },
				// 新API対応：2011.05.26 <ADD> Start T.Takahashi.
// CQ376793 HDDフォーマット中状態の共有メモリへの追加 2013/1/7 Soma Start.
				{ TYPE_NTF_DataID_SYS_94, sizeof(Bool)                                  , &( m_stSysStatInfo.st_SystemInfoParam.b_Sys94 )	   },
// CQ376793 HDDフォーマット中状態の共有メモリへの追加 2013/1/7 Soma End.
// 2014.01.09-STA  パネルHW情報の取得.
// 変化データ登録なので対象外.				{ TYPE_NTF_DataID_PNL_5, sizeof(TYPS_PanelHardResolution)               , &( m_stSysStatInfo.st_PanelInfoParam.st_PanelHardResolution )	   },
// 2014.01.09-END  パネルHW情報の取得.
// Eagle:制御構成変更No.104_テストパターン印刷のターミナルコマンド対応(生産対応）start.
				{ TYPE_NTF_DataID_SYS_116, sizeof(TYPS_TestPatternPrintInfo)			, &( m_stSysStatInfo.st_SystemInfoParam.st_Sys116 )	   },
// Eagle:制御構成変更No.104_テストパターン印刷のターミナルコマンド対応(生産対応）end.
				{ TYPE_NTF_DataID_SYS_117, sizeof(Bool)                                 , &( m_stSysStatInfo.st_SystemInfoParam.b_Sys117 )	   },	// HDD物理フォーマット中状態の共有メモリ

				{ TYPE_NTF_DataID_End   , 0                                             , NULL												   }
			};
			for( struct ST_DataChangeTabl *pst_DataTbl = &st_DataChangeTable[ 0 ]; pst_DataTbl->e_DataID != TYPE_NTF_DataID_End; pst_DataTbl++ ){
				if( e_DataID == pst_DataTbl->e_DataID ){
					// データが無い場合は存在なしを返す.
					if( puc_Data == NULL ){
						SST_ERR_LOG("error：DataID Match! however puc_Data==NULL.\n");
					}else{
						memcpy( pst_DataTbl->pv_Data, puc_Data, pst_DataTbl->us_DataSize );
						e_DataType = SSTE_NotifyData_Exist;	// 2011/05/10 <CHG>.
					}
					break;
				}
			}
			break;		// defaltのbreak.
	}
	
	SST_LOG2(MFPE_LogLevelTrace, "End. e_DataID=(0x%08x) e_DataType=(%d).\n", e_DataID, e_DataType);
	return( e_DataType );	// 2011/05/10 <CHG>.
}

//************************************************************************************************************************.
//概要　　：変化点通知処理.
//引数　　：<in>　e_DataID			データID.
//戻り値　：処理結果 正常終了時：TYPD_SSTS_StatusSuccess　異常終了時：TYPD_SSTS_StatusError.
//備考　　：特に無し.
//作成者　：谷本.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//************************************************************************************************************************.
TYPD_SSTS_STATUS SSTC_SystemStatusService::notifyChangedData( TYPE_NTF_DataID e_DataID )
{
	SST_LOG2(MFPE_LogLevelTrace, "e_DataID=(0x%08x).\n", e_DataID);
	
	TYPD_SSTS_STATUS  sc_Return = TYPD_SSTS_StatusError;
	uchar            *pc_Buff   = NULL;
	
	do {
		// STEP2：2010.12.10 <CHG&ADD> Start T.Takahashi.
		
		// 通知設定でない場合は送信せずに正常終了.
		if( !chkSendNotifySetting( e_DataID ) ){
			sc_Return = TYPD_SSTS_StatusSuccess;
			break;
		}
		
		// メモリ確保.
		ushort  us_SendSize = sizeof(MFPS_PacketHeader) + sizeof(e_DataID) - 1;
// 2012.08.29 sk_takas for CQ#370048 -->
		pc_Buff     = new uchar[ us_SendSize ];
// 2012.08.29 sk_takas for CQ#370048 <--
		
		if( pc_Buff == NULL ){
// 2015.10.16 ZeusSZX0 制御76-1 新詳細コード s.
//			APIC_ThrowSystemError(SYSE_SysErrGeneral, LOGF_DEBUG_EVENT_KIND_SYS_STATUS, "SSTC_SystemStatusService notifyChangedData(1)");
			APIC_ThrowSystemErrorNewDetailCode(SYSE_SysErrGeneral, LOGF_DEBUG_EVENT_KIND_SYS_STATUS, "SSTC_SystemStatusService notifyChangedData(1)", 0, createDetailCode(0x304D));
// 2015.10.16 ZeusSZX0 制御76-1 新詳細コード e.
			break;
		}
		
		// <20110126 CHG Start>.
		for( std::vector<MFPS_PacketHeader>::iterator it = m_vNotifyChange.begin(); it != m_vNotifyChange.end(); ++it ){
			
			// バッファ初期化.
			memset( pc_Buff, (char)SSTD_ZERO_OUT, us_SendSize );
			
			// 送信先退避.
			struct sockaddr_un dstAddr;
			memset( (char *)&dstAddr, 0, sizeof(dstAddr) );
			dstAddr = it->saddr;
			
			// 送信用ヘッダ初期化.
			MFPS_PacketHeader *pst_Head = (MFPS_PacketHeader *)pc_Buff;
			init_head( MFPD_SOCKKIND_ASYNC, it->reqkindid, it->requestid, MFPD_TARGETID_SYSTEMSTATUSSERVICE, pst_Head );
			
			// 送信情報セット.
			memcpy( pst_Head->data, &e_DataID, sizeof(e_DataID) );
			pst_Head->datasize = sizeof(e_DataID);
			
			// 非同期データ送信処理をコール.
			if( send_data( MFPD_SOCKKIND_ASYNC, &dstAddr, (const MFPS_PacketHeader *)pst_Head, us_SendSize ) < 0 ){
// 2015.10.16 ZeusSZX0 制御76-1 新詳細コード s.
//				APIC_ThrowSystemError(SYSE_SysErrGeneral, LOGF_DEBUG_EVENT_KIND_SYS_STATUS, "SSTC_SystemStatusService notifyChangedData(2)");
				APIC_ThrowSystemErrorNewDetailCode(SYSE_SysErrGeneral, LOGF_DEBUG_EVENT_KIND_SYS_STATUS, "SSTC_SystemStatusService notifyChangedData(2)", 0, createDetailCode(0x304E));
// 2015.10.16 ZeusSZX0 制御76-1 新詳細コード e.
				break;
			}
		}
		// <20110126 CHG End  >.
		
		// STEP2：2010.12.10 <CHG&ADD> End   T.Takahashi.
		sc_Return = TYPD_SSTS_StatusSuccess;
	} while( 0 );
	
	// メモリ開放.
	if ( pc_Buff != NULL ){ delete [] pc_Buff; };
	
	SST_LOG2(MFPE_LogLevelTrace, "End. sc_Return=(%d).\n", (int)sc_Return);
	
	return( sc_Return );
}

//************************************************************************************************************************.
//概要　　：変化点通知設定を判定する.
//引数　　：<in>　e_DataID			データID.
//戻り値　：処理結果 正常終了時：True　異常終了時：False.
//備考　　：特に無し.
//作成者　：高橋.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//************************************************************************************************************************.
Bool SSTC_SystemStatusService::chkSendNotifySetting( TYPE_NTF_DataID e_DataID )
{
	TYPE_NTF_GroupID e_GroupID = (TYPE_NTF_GroupID)(e_DataID >> SSTD_Short_Bits);
	Bool   b_Ret      = True;
	
	SST_LOG2(MFPE_LogLevelTrace, "e_DataID=(0x%08x) us_GroupId=(0x%04x).\n", e_DataID, e_GroupID);
	
	do{
		if( !checkGroupID( e_GroupID ) ){
			break;														// GroupIDが範囲外の場合は設定なし.
		}
		
		if( m_pulNotificationMap[ e_GroupID ] == NULL ){				// 設定なしの場合.
			break;
		}
		uint32_t  *pul_Top = m_pulNotificationMap[ e_GroupID ];
		ushort  us_Data = (ushort)((e_DataID & SSTD_Mask_DataID) - 1 );
		
		// 通知なし設定は正常終了.
		b_Ret = ( (*( pul_Top + ((us_Data - (us_Data % SSTD_Long_Bits)) / SSTD_Long_Bits)) & (1 << (us_Data % SSTD_Long_Bits))) != 0 );
		
		SST_LOG2(MFPE_LogLevelTrace, "tbl idx =(%d).\n"    , ((us_Data - (us_Data % SSTD_Long_Bits)) / SSTD_Long_Bits));
		SST_LOG2(MFPE_LogLevelTrace, "tbl val =(0x%08x).\n", *( pul_Top + ((us_Data - (us_Data % SSTD_Long_Bits)) / SSTD_Long_Bits)));
		SST_LOG2(MFPE_LogLevelTrace, "chk bit =(0x%08x).\n", (1 << (us_Data % SSTD_Long_Bits)));
		SST_LOG2(MFPE_LogLevelTrace, "==       (%d).\n", ( (*( pul_Top + ((us_Data - (us_Data % SSTD_Long_Bits)) / SSTD_Long_Bits)) & (1 << (us_Data % SSTD_Long_Bits))) != 0 ));
	}while( 0 );
	
	SST_LOG2(MFPE_LogLevelTrace, "End. b_Ret =(%d).\n", b_Ret);
	return b_Ret;
}

//************************************************************************************************************************.
//概要　　：システムステータス情報取得応答.
//引数　　：<in>　psc_FilePath		ファイルパス.
//引数　　：<in>　sc_Result			処理結果.
//戻り値　：処理結果 正常終了時：TYPD_SSTS_StatusSuccess　異常終了時：TYPD_SSTS_StatusError.
//備考　　：特に無し.
//作成者　：谷本.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//************************************************************************************************************************.
TYPD_SSTS_STATUS SSTC_SystemStatusService::ans_getSystemStatusInfo( const char* psc_FilePath, TYPD_SSTS_STATUS sc_Result )
{
	SST_LOG2(MFPE_LogLevelTrace, "psc_FilePath=(%s) sc_Result=(%d).\n", psc_FilePath, (int)sc_Result);
	
	TYPD_SSTS_STATUS  sc_Return   = TYPD_SSTS_StatusError;
	size_t            st_PathLen  = strlen( psc_FilePath );
	ushort            us_SendSize = (( TYPD_SSTS_PathMax + 1 ) + sizeof( sc_Result ));
	uchar            *pc_Buff     = new uchar[ us_SendSize ];
	
	do {
		if( pc_Buff == NULL ){
// 2015.10.16 ZeusSZX0 制御76-1 新詳細コード s.
//			APIC_ThrowSystemError(SYSE_SysErrGeneral, LOGF_DEBUG_EVENT_KIND_SYS_STATUS, "SSTC_SystemStatusService ans_getSystemStatusInfo(1)");
			APIC_ThrowSystemErrorNewDetailCode(SYSE_SysErrGeneral, LOGF_DEBUG_EVENT_KIND_SYS_STATUS, "SSTC_SystemStatusService ans_getSystemStatusInfo(1)", 0, createDetailCode(0x304F));
// 2015.10.16 ZeusSZX0 制御76-1 新詳細コード e.
			break;
		}
		
		// バッファ初期化.
		memset( pc_Buff, (char)SSTD_ZERO_OUT, us_SendSize );
		
		// ProcID取得.
		MFPD_ProcID ul_ProcId = getStoredProcId( SSTE_SysStatusMsgType_SysStatusGetReq, True );
		
		// システムステータス情報取得応答用の情報（ファイルパス、要求結果）を設定.
		if ( sc_Result == TYPD_SSTS_StatusSuccess ){ memcpy( &pc_Buff[0], psc_FilePath, st_PathLen ); }		// パスを登録は作成成功時のみとする.
		memcpy( &pc_Buff[ TYPD_SSTS_PathMax + 1 ], (void *)&sc_Result, sizeof(sc_Result) );
		
		// 非同期データ送信処理をコール.
		if( send_data( ul_ProcId, (const char *)pc_Buff, us_SendSize ) < 0 ){
// 2015.10.16 ZeusSZX0 制御76-1 新詳細コード s.
//			APIC_ThrowSystemError(SYSE_SysErrGeneral, LOGF_DEBUG_EVENT_KIND_SYS_STATUS, "SSTC_SystemStatusService ans_getSystemStatusInfo(2)");
			APIC_ThrowSystemErrorNewDetailCode(SYSE_SysErrGeneral, LOGF_DEBUG_EVENT_KIND_SYS_STATUS, "SSTC_SystemStatusService ans_getSystemStatusInfo(2)", 0, createDetailCode(0x3050));
// 2015.10.16 ZeusSZX0 制御76-1 新詳細コード e.
			break;
		}
		// リクエスト情報を破棄 <Act_31462 Add>.
		delRequest( ul_ProcId );
		
		sc_Return = TYPD_SSTS_StatusSuccess;
	} while( 0 );
	
		// メモリ開放.
	if( pc_Buff != NULL ){ delete [] pc_Buff; }
	
	SST_LOG2(MFPE_LogLevelTrace, "End. sc_Return=(%d).\n", (int)sc_Return);
	
	return( sc_Return );
}

//************************************************************************************************************************.
//概要　　：変化点通知開始要求応答.
//引数　　：<in>　sc_Result			処理結果.
//戻り値　：処理結果 正常終了時：TYPD_SSTS_StatusSuccess　異常終了時：TYPD_SSTS_StatusSuccess.
//備考　　：特に無し.
//作成者　：高橋.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//************************************************************************************************************************.
TYPD_SSTS_STATUS SSTC_SystemStatusService::ans_StartChangeData(TYPD_SSTS_STATUS sc_Result)
{
	SST_LOG2(MFPE_LogLevelTrace, "sc_Result=(%d).\n", (int)sc_Result);
	
	TYPD_SSTS_STATUS sc_Return = ans_CommonFunc( sc_Result, SSTE_SysStatusMsgType_StartChangeData, True );
	
	SST_LOG2(MFPE_LogLevelTrace, "End. sc_Return=(%d).\n", (int)sc_Return);
	
	return( sc_Return );
}

//************************************************************************************************************************.
//概要　　：トラブルリセット応答.
//引数　　：<in>　sc_Result			処理結果.
//戻り値　：処理結果 正常終了時：TYPD_SSTS_StatusSuccess　異常終了時：TYPD_SSTS_StatusSuccess.
//備考　　：特に無し.
//作成者　：高橋.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//************************************************************************************************************************.
TYPD_SSTS_STATUS SSTC_SystemStatusService::ans_TroubleReset(TYPD_SSTS_STATUS sc_Result)
{
	SST_LOG2(MFPE_LogLevelTrace, "sc_Result=(%d).\n", (int)sc_Result);
	
	TYPD_SSTS_STATUS sc_Return = ans_CommonFunc( sc_Result, SSTE_SysStatusMsgType_TroubleReset, True );
	
	SST_LOG2(MFPE_LogLevelTrace, "End. sc_Return=(%d).\n", (int)sc_Return);
	
	return( sc_Return );
}

// STEP2：2010.12.10 <ADD> Start T.Takahashi.
//************************************************************************************************************************.
//概要　　：システムステータス更新要求応答.
//引数　　：<in>　sc_Result			処理結果.
//戻り値　：処理結果 正常終了時：TYPD_SSTS_StatusSuccess　異常終了時：TYPD_SSTS_StatusSuccess.
//備考　　：特に無し.
//作成者　：高橋.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//************************************************************************************************************************.
TYPD_SSTS_STATUS SSTC_SystemStatusService::ans_SetSystemStatusData( TYPD_SSTS_STATUS sc_Result )
{
	SST_LOG2(MFPE_LogLevelTrace, "sc_Result=(%d).\n", (int)sc_Result);
	
	TYPD_SSTS_STATUS sc_Return = ans_CommonFunc( sc_Result, SSTE_SysStatusMsgType_SetSystemStatusData, True );
	
	SST_LOG2(MFPE_LogLevelTrace, "End. sc_Return=(%d).\n", (int)sc_Return);
	
	return( sc_Return );
}

//************************************************************************************************************************.
//概要　　：変化点通知設定要求応答.
//引数　　：<in>　sc_Result			処理結果.
//戻り値　：処理結果 正常終了時：TYPD_SSTS_StatusSuccess　異常終了時：TYPD_SSTS_StatusSuccess.
//備考　　：特に無し.
//作成者　：高橋.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//************************************************************************************************************************.
TYPD_SSTS_STATUS SSTC_SystemStatusService::ans_NotificationSetting( TYPD_SSTS_STATUS sc_Result )
{
	SST_LOG2(MFPE_LogLevelTrace, "sc_Result=(%d).\n", (int)sc_Result);
	
	TYPD_SSTS_STATUS sc_Return = ans_CommonFunc( sc_Result, SSTE_SysStatusMsgType_SetNotificationSetting, True );
	
	SST_LOG2(MFPE_LogLevelTrace, "End. sc_Return=(%d).\n", (int)sc_Return);
	
	return( sc_Return );
}

//************************************************************************************************************************.
//概要　　：汎用応答処理.
//引数　　：<in>　sc_Result			処理結果.
//引数　　：<in>　e_MsgType			メッセージタイプ.
//引数　　：<in>　b_PopFlag			削除指示.
//戻り値　：処理結果 正常終了時：TYPD_SSTS_StatusSuccess　異常終了時：TYPD_SSTS_StatusSuccess.
//備考　　：特に無し.
//作成者　：高橋.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//************************************************************************************************************************.
TYPD_SSTS_STATUS SSTC_SystemStatusService::ans_CommonFunc( TYPD_SSTS_STATUS sc_Result, SSTE_SysStatusMsgType e_MsgType, Bool b_PopFlag )
{
	SST_LOG(MFPE_LogLevelTrace);
	TYPD_SSTS_STATUS  sc_Return   = TYPD_SSTS_StatusError;
	ushort            us_SendSize = sizeof(sc_Result);
	uchar            *pc_Buff     = new uchar[ us_SendSize ];
	
	do {
		if( pc_Buff == NULL ){
// 2015.10.16 ZeusSZX0 制御76-1 新詳細コード s.
//			APIC_ThrowSystemError(SYSE_SysErrGeneral, LOGF_DEBUG_EVENT_KIND_SYS_STATUS, "SSTC_SystemStatusService ans_CommonFunc() error 1.");
			APIC_ThrowSystemErrorNewDetailCode(SYSE_SysErrGeneral, LOGF_DEBUG_EVENT_KIND_SYS_STATUS, "SSTC_SystemStatusService ans_CommonFunc() error 1.", 0, createDetailCode(0x3051));
// 2015.10.16 ZeusSZX0 制御76-1 新詳細コード e.
			break;
		}
		memset( pc_Buff, (char)SSTD_ZERO_OUT, us_SendSize );
		
		// ProcID取得.
		MFPD_ProcID ul_ProcId = getStoredProcId( e_MsgType, b_PopFlag );
		
		// 応答情報を設定.
		memcpy( pc_Buff, (void *)&sc_Result, sizeof(sc_Result) );
		
		// 非同期データ送信処理をコール.
		if( send_data( ul_ProcId, (const char *)pc_Buff,  us_SendSize ) < 0 ){
// 2015.10.16 ZeusSZX0 制御76-1 新詳細コード s.
//			APIC_ThrowSystemError(SYSE_SysErrGeneral, LOGF_DEBUG_EVENT_KIND_SYS_STATUS, "SSTC_SystemStatusService ans_CommonFunc() error 2.");
			APIC_ThrowSystemErrorNewDetailCode(SYSE_SysErrGeneral, LOGF_DEBUG_EVENT_KIND_SYS_STATUS, "SSTC_SystemStatusService ans_CommonFunc() error 2.", 0, createDetailCode(0x3052));
// 2015.10.16 ZeusSZX0 制御76-1 新詳細コード e.
			break;
		}
		// リクエスト情報を破棄 <Act_31462 Add>.
		delRequest( ul_ProcId );
		
		sc_Return = TYPD_SSTS_StatusSuccess;
	} while( 0 );
	
		// メモリ開放.
	if( pc_Buff != NULL ){ delete [] pc_Buff; }
	SST_LOG2(MFPE_LogLevelTrace, "End. sc_Return=(%d).\n", (int)sc_Return);
	return sc_Return;
}
// STEP2：2010.12.10 <ADD> End   T.Takahashi.

//************************************************************************************************************************.
//概要　　：ステータス情報取得（定期起床）.
//戻り値　：無し.
//備考　　：特に無し.
//作成者　：谷本.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
// 2012. 5.15 [CQ363516] パフォーマンス改善 Observerで検出可能な情報をポーリング→Observerに変更.
// 2012. 5.15 [CQ363516] パフォーマンス改善 ポーリング周期を500msと2sに変更.
// 2018/09/29 [RQ-674] SCD2-motohashi ジョブログ関連の警告取得処理は定期起床処理で行うように変更.
//************************************************************************************************************************.
void SSTC_SystemStatusService::getStatusTimerWakeUp()
{
	SST_LOG(MFPE_LogLevelAll);
	
	// 警告情報取得.
	getRdsWarningStatus_Low();	// RDS警告情報取得処理.
	getBoxWarningStatus_High();	// BOX警告情報取得処理.
	getFaxWarningStatus_High();	// FAX警告情報取得処理.
	getPrtWarningStatus_High();	// PRT警告情報取得処理.
	getScnWarningStatus_High();	// SCN警告情報取得処理.
	getScnWarningStatus_Low();
	
	// STEP2：2010.12.17 <ADD> Start T.Takahashi.
	// システム情報取得.
	getSystemInfoStatus_High();

	// システム/スキャナ情報取得 (初期化完了時にポーリング止める).
	getSystemScannerInfoStatus_Low_Stop();
	
	// スキャナ情報取得.
	getScannerInfoStatus_High();
	
	// デバイス情報取得.
	getDeviceInfoStatus_High();
	// STEP2：2010.12.17 <ADD> End   T.Takahashi.
	
	// デバイス情報プリンタ取得.
	getPrinterDeviceStatus_High();
	getPrinterDeviceStatus_Low();

// 2014.01.09-STA  パネルHW情報の取得.
	// パネルHW情報取得.
	// 現状パネルHW情報は変化ないため呼出をコメント.
	//getPanelInfoStatus_PanelHardResolution_Low(); // パネルHW情報.
// 2014.01.09-END  パネルHW情報の取得.

	getJlgWarningStatus_Low();	// JLG警告情報取得処理.
}

//************************************************************************************************************************.
//概要　　：ステータス情報取得（定期起床）.
//引数　　：<in>　us_Data			データ取得フラグ.
//戻り値　：無し.
//備考　　：特に無し.
//作成者　：谷本.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
// 2012. 5.15 [CQ363516] パフォーマンス改善 ポーリング周期を500msと2sに変更.
//************************************************************************************************************************.
void SSTC_SystemStatusService::getStatusTimerWakeUp_High()
{
	SST_LOG(MFPE_LogLevelAll);
	
	// 警告情報取得.
	getBoxWarningStatus_High();	// BOX警告情報取得処理.
	getFaxWarningStatus_High();	// FAX警告情報取得処理.
	getPrtWarningStatus_High();	// PRT警告情報取得処理.
	getScnWarningStatus_High();	// SCN警告情報取得処理.
	
	// STEP2：2010.12.17 <ADD> Start T.Takahashi.
	// システム情報取得.
	getSystemInfoStatus_High();
	
	// スキャナ情報取得.
	getScannerInfoStatus_High();
	
	// デバイス情報取得.
	getDeviceInfoStatus_High();
	// STEP2：2010.12.17 <ADD> End   T.Takahashi.
	
	// デバイス情報プリンタ取得.
	getPrinterDeviceStatus_High();
	
}

//************************************************************************************************************************.
//概要　　：プロックID取得.
//引数　　：<in>　e_MsgType			メッセージタイプ.
//引数　　：<in>　b_PopFlag			削除指示.
//戻り値　：プロックID. または 0( プロックIDが保存されていないかメッセージタイプが不正 )
//備考　　：特に無し.
//作成者　：高橋.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//************************************************************************************************************************.
MFPD_ProcID SSTC_SystemStatusService::getStoredProcId( SSTE_SysStatusMsgType e_MsgType, Bool b_PopFlag )
{
	SST_LOG2(MFPE_LogLevelTrace, "e_MsgType=(0x%08x) b_PopFlag=(%d).\n", e_MsgType, b_PopFlag);
	
	MFPD_ProcID ul_ProcId = 0;
	
	do {
		// メッセージタイプパラメタのチェック.
		if( e_MsgType >= SSTE_SysStatusMsgType_Max ) { break; }
		
		// 変化点通知はプロックIDを使用しない.
		if( e_MsgType == SSTE_SysStatusMsgType_NotifyChangedData ) { break; }
		
		// プロックIDが空の場合は処理を継続しない.
		if( m_QueueReqID[ e_MsgType ].empty() ) { break; }
		
		// 取得実行.
		ul_ProcId = m_QueueReqID[ e_MsgType ].front();
		
		// 指示に従いpopを実行.
		if( b_PopFlag ) { m_QueueReqID[ e_MsgType ].pop(); }
	} while(0);
	
	SST_LOG2(MFPE_LogLevelTrace, "End. ul_ProcId=(%ld).\n", ul_ProcId);
	
	return ul_ProcId;
}

//************************************************************************************************************************.
//概要　　：ローカルセマフォロック処理.
//戻り値　：無し.
//備考　　：特に無し.
//作成者　：谷本.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
// 2012. 1.31 [CQ359780] getProgJobXXXStatusをポーリング→Observerに変更.
//************************************************************************************************************************.
void SSTC_SystemStatusService::lock( SEM_ID id /*=m_semIDShm*/ )
{
	SST_LOG(MFPE_LogLevelTrace);
	// ロック.
	SYSF_semTake( id, WAIT_FOREVER );
}

//************************************************************************************************************************.
//概要　　：ローカルセマフォアンロック処理.
//戻り値　：無し.
//備考　　：特に無し.
//作成者　：谷本.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
// 2012. 1.31 [CQ359780] getProgJobXXXStatusをポーリング→Observerに変更.
//************************************************************************************************************************.
void SSTC_SystemStatusService::unlock( SEM_ID id /*=m_semIDShm*/ )
{
	SST_LOG(MFPE_LogLevelTrace);
	// アンロック.
	SYSF_semGive( id );
}

// STEP2：2010.12.10 <ADD> Start T.Takahashi.
//************************************************************************************************************************.
//概要　　：システム情報取得処理.
//戻り値　：無し.
//備考　　：特に無し.
//作成者　：高橋.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
// 2012. 5.15 [CQ363516] パフォーマンス改善 Observerで検出可能な情報をポーリング→Observerに変更.
// 2012. 5.15 [CQ363516] パフォーマンス改善 ポーリング周期を500msと2sに変更.
// 2012. 8.31 [CQ371105] 静的解析 APIC_SystemInfo::newInstance()はNULLリターンしないため、NULLチェックを外す.
//************************************************************************************************************************.
void SSTC_SystemStatusService::getSystemInfoStatus_High()
{
	SST_LOG(MFPE_LogLevelAll);
	SSTS_ChangedInfo stChangedInfo;

	ushort           us_Idx;
	Bool             b_Val;
	
	//************************************************************************.
	// 情報を取得してシステム情報と比較し変化があれば変化情報に追加.
	//************************************************************************.
	APIC_SystemInfo* pc_SystemInfo = APIC_SystemInfo::newInstance();
	
	// 特定原稿検出エラー.
	b_Val = pc_SystemInfo->getSODError();
	if( m_stSysStatInfo.st_SystemInfoParam.b_Sys48 != b_Val ){
		m_stSysStatInfo.st_SystemInfoParam.b_Sys48 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SYS_48;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_SystemInfoParam.b_Sys48 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// JobService関連追加 Start 2011.02.09 <ADD>
	
	APIC_SystemStatus *pc_SystemStatus = APIC_SystemStatus::newInstance();
	
	// プログラムジョブの最後に投入されたDocID.
	int32_t sl_DocId = pc_SystemStatus->getProgJobLastDocID();
	if( m_stSysStatInfo.st_SystemInfoParam.sl_Sys77 != sl_DocId ){
		m_stSysStatInfo.st_SystemInfoParam.sl_Sys77  = sl_DocId;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SYS_77;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_SystemInfoParam.sl_Sys77 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// 部数（プログラムジョブ用）.
	int32_t sl_Val = pc_SystemStatus->getProgJobNumOfCopies();
	if( m_stSysStatInfo.st_SystemInfoParam.sl_Sys78 != sl_Val ){
		m_stSysStatInfo.st_SystemInfoParam.sl_Sys78 = sl_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SYS_78;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_SystemInfoParam.sl_Sys78 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// JobService関連追加 End   2011.02.09 <ADD>
	
	
	// ATS/AMS対応：2011.05.10 <ADD> Start T.Takahashi.
	
	// プログラムジョブDocIDの登録数.
	sl_Val = pc_SystemStatus->getProgJobDocIDCounter();
	if( m_stSysStatInfo.st_SystemInfoParam.sl_Sys80 != sl_Val ){
		m_stSysStatInfo.st_SystemInfoParam.sl_Sys80  = sl_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SYS_80;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_SystemInfoParam.sl_Sys80 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// ATS/AMS対応：2011.05.10 <ADD> End T.Takahashi.
	
	// Job実行可否対応 2011.11.22 ADS K.Tsuboi Start.
	// ジョブ起動チェック.
	Bool b_Sys103 = False;
	for( us_Idx = 0; us_Idx < (ushort)TYPE_JobEnableCheckStatusMax; ++us_Idx ){
		b_Val = pc_SystemStatus->chkJobStarting((TYPE_JobEnableCheckStatus)us_Idx);
		if( m_stSysStatInfo.st_SystemInfoParam.b_Sys103[us_Idx] != b_Val ){
			m_stSysStatInfo.st_SystemInfoParam.b_Sys103[us_Idx]  = b_Val;
			b_Sys103 = True;
		}
	}
	if( b_Sys103 ){
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SYS_103;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_SystemInfoParam.b_Sys103[ 0 ] );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	// Job実行可否対応 2011.11.22 ADS K.Tsuboi End.

	SST_LOG2(MFPE_LogLevelAll, "End.\n", NULL);
}

//************************************************************************************************************************.
//概要　　：システム/スキャナー情報取得処理 (初期化完了時にポーリング止める).
//戻り値　：無し.
//備考　　：特に無し.
//作成者　：高橋.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
// 2012. 5.15 [CQ363516] パフォーマンス改善 Observerで検出可能な情報をポーリング→Observerに変更.
// 2012. 5.15 [CQ363516] パフォーマンス改善 ポーリング周期を500msと2sに変更.
// 2012. 5.31 [CQ363516] パフォーマンス改善 高周期のポーリング処理で、各条件に該当する場合に変化点検出処理を実行する.
//************************************************************************************************************************.
void SSTC_SystemStatusService::getSystemScannerInfoStatus_Low_Stop()
{
	SST_LOG(MFPE_LogLevelAll);

	static Bool b_PrevInitializing = True;		// 前回の初期化中状態 [初期値:True].
	static Bool b_PrevInitializingPrt = True;	// 前回の初期化中(Printer)状態 [初期値:True].
	static Bool b_PrevInitializingScn = True;	// 前回の初期化中(Scanner)状態 [初期値:True].

	static uint32_t ul_PrevCounterSystemInfo = (uint32_t)-1;			// 前回のAPIC_SystemInfoObserverObserver::dataChanged()発生回数 [初期値:-1].
	static uint32_t ul_PrevCounterDipSw = (uint32_t)-1;					// 前回のAPIC_SoftDipSwObserver::dataChanged()発生回数 [初期値:-1].
	static uint32_t ul_PrevCounterPowerStatusChanged = (uint32_t)-1;	// 前回のAPIC_PowerObserver::powerStatusChanged()発生回数 [初期値:-1].

	Bool b_Check = False;	// 検出フラグ OFF.

	if (b_PrevInitializing == True) {	// 初期化中の場合.

		if (b_PrevInitializingPrt == True) { b_PrevInitializingPrt = SSTD_ISINITIALIZINGPRINTER(); }
		if (b_PrevInitializingScn == True) { b_PrevInitializingScn = APIC_ScannerStatus::newInstance()->isInitializing(); }

		b_Check = True;	// 検出フラグ ON.

		SST_LOG2(MFPE_LogLevelTrace, "b_PrevInitializingPrt=(%d->%d), prt=(%d), scn=(%d).\n", b_PrevInitializing, b_PrevInitializingPrt || b_PrevInitializingScn, b_PrevInitializingPrt, b_PrevInitializingScn);
		b_PrevInitializing = b_PrevInitializingPrt || b_PrevInitializingScn;
	}

	if ( (ul_PrevCounterSystemInfo != m_ulCounterSystemInfoObserver)	// APIC_SystemInfoObserverObserver::dataChanged() or .
		|| (ul_PrevCounterDipSw != m_ulCounterSoftDipSwObserver) ) {	// APIC_SoftDipSwObserver::dataChanged()が発生していた場合.
		b_Check = True;	// 検出フラグ ON.
	}
	ul_PrevCounterSystemInfo = m_ulCounterSystemInfoObserver;	// APIC_SystemInfoObserverObserver::dataChanged()発生回数退避.
	ul_PrevCounterDipSw = m_ulCounterSoftDipSwObserver;			// APIC_SoftDipSwObserver::dataChanged()発生回数退避.

	if ( (ul_PrevCounterPowerStatusChanged != m_ulCounterPowerStatusChanged)	// APIC_PowerObserver::powerStatusChanged() or .
		|| (b_Check == True) ) {												// 検出フラグONの場合.
		getSystemInfoStatus_Low_stop2();	// システム情報取得処理(2).
	}
	ul_PrevCounterPowerStatusChanged = m_ulCounterPowerStatusChanged;	// APIC_PowerObserver::powerStatusChanged()発生回数退避.

	if (b_Check == True) {	// 検出フラグONの場合.
		getSystemInfoStatus_Low_stop();		// システム情報取得処理(1).
		getScannerInfoStatus_Low_Stop();	// スキャナ情報取得処理.
	}

}

//************************************************************************************************************************.
//概要　　：システム情報取得処理 (初期化完了時にポーリング止める).
//戻り値　：無し.
//備考　　：特に無し.
//作成者　：高橋.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
// 2012. 5.31 [CQ363516] パフォーマンス改善 高周期のポーリング処理で、各条件に該当する場合に変化点検出処理を実行する.
//************************************************************************************************************************.
void SSTC_SystemStatusService::getSystemInfoStatus_Low_stop()
{
	SST_LOG(MFPE_LogLevelAll);
	SSTS_ChangedInfo stChangedInfo;

	//************************************************************************.
	// 情報を取得してシステム情報と比較し変化があれば変化情報に追加.
	//************************************************************************.
	APIC_SystemInfo* pc_SystemInfo = APIC_SystemInfo::newInstance();

	// モデル( Low / Standard ).
	TYPE_VersionOfModel e_VofMdl = pc_SystemInfo->getTypeOfModel();
	if( m_stSysStatInfo.st_SystemInfoParam.e_Sys19 != e_VofMdl ){
		m_stSysStatInfo.st_SystemInfoParam.e_Sys19 = e_VofMdl;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SYS_19;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_SystemInfoParam.e_Sys19 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// カラー機／モノクロ機.
	Bool b_Val = pc_SystemInfo->isColorOrMonoMachine();
	if( m_stSysStatInfo.st_SystemInfoParam.b_Sys20 != b_Val ){
		m_stSysStatInfo.st_SystemInfoParam.b_Sys20 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SYS_20;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_SystemInfoParam.b_Sys20 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// エンジン解像度.
	short ss_Val = pc_SystemInfo->getEngineResolution();
	if( m_stSysStatInfo.st_SystemInfoParam.ss_Sys21 != ss_Val ){
		m_stSysStatInfo.st_SystemInfoParam.ss_Sys21 = ss_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SYS_21;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_SystemInfoParam.ss_Sys21 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// スキャナー有り.
	b_Val = pc_SystemInfo->isScanerExist();
	if( m_stSysStatInfo.st_SystemInfoParam.b_Sys22 != b_Val ){
		m_stSysStatInfo.st_SystemInfoParam.b_Sys22 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SYS_22;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_SystemInfoParam.b_Sys22 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// JpegAsic有り 判定.
	b_Val = pc_SystemInfo->isJpegAsicExist();
	if( m_stSysStatInfo.st_SystemInfoParam.b_Sys25 != b_Val ){
		m_stSysStatInfo.st_SystemInfoParam.b_Sys25 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SYS_25;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_SystemInfoParam.b_Sys25 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// USBホスト基板有り.
	b_Val = pc_SystemInfo->isUSBHostExist();
	if( m_stSysStatInfo.st_SystemInfoParam.b_Sys28 != b_Val ){
		m_stSysStatInfo.st_SystemInfoParam.b_Sys28 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SYS_28;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_SystemInfoParam.b_Sys28 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// Etherインストール.
	b_Val = pc_SystemInfo->isEtherInstalled();
	if( m_stSysStatInfo.st_SystemInfoParam.b_Sys40 != b_Val ){
		m_stSysStatInfo.st_SystemInfoParam.b_Sys40 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SYS_40;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_SystemInfoParam.b_Sys40 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// Wirelessインストール.
	b_Val = pc_SystemInfo->isWirelessInstalled();
	if( m_stSysStatInfo.st_SystemInfoParam.b_Sys41 != b_Val ){
		m_stSysStatInfo.st_SystemInfoParam.b_Sys41 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SYS_41;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_SystemInfoParam.b_Sys41 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// TokenRingインストール.
	b_Val = pc_SystemInfo->isTokenRingInstalled();
	if( m_stSysStatInfo.st_SystemInfoParam.b_Sys42 != b_Val ){
		m_stSysStatInfo.st_SystemInfoParam.b_Sys42 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SYS_42;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_SystemInfoParam.b_Sys42 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// USBインストール.
	b_Val = pc_SystemInfo->isUSBInstalled();
	if( m_stSysStatInfo.st_SystemInfoParam.b_Sys43 != b_Val ){
		m_stSysStatInfo.st_SystemInfoParam.b_Sys43 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SYS_43;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_SystemInfoParam.b_Sys43 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// IEEE1394インストール.
	b_Val = pc_SystemInfo->isIEEE1394Installed();
	if( m_stSysStatInfo.st_SystemInfoParam.b_Sys44 != b_Val ){
		m_stSysStatInfo.st_SystemInfoParam.b_Sys44 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SYS_44;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_SystemInfoParam.b_Sys44 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// プロテクションチップ情報.
	b_Val = pc_SystemInfo->isProtectionChipExist();
	if( m_stSysStatInfo.st_SystemInfoParam.b_Sys45 != b_Val ){
		m_stSysStatInfo.st_SystemInfoParam.b_Sys45 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SYS_45;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_SystemInfoParam.b_Sys45 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// エンジン出荷仕向け.
	b_Val = pc_SystemInfo->getEngineSystemType();
	if( m_stSysStatInfo.st_SystemInfoParam.b_Sys49 != b_Val ){
		m_stSysStatInfo.st_SystemInfoParam.b_Sys49 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SYS_49;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_SystemInfoParam.b_Sys49 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// メーカー名.
	char sc_Maker[ TYPD_SSTS_MakerNameLength ];
	char *psc_Maker = NULL;
	memset( sc_Maker, (char)SSTD_ZERO_OUT, TYPD_SSTS_MakerNameLength );
	psc_Maker = pc_SystemInfo->getManufacturer();
// 2019/12/03 [EagleBK] [OP_BTS-11276] HIENG START.
//	memcpy( sc_Maker, psc_Maker, TYPD_SSTS_MakerNameLength );
	strncpy( sc_Maker, psc_Maker, TYPD_SSTS_MakerNameLength - 1 );
// 2019/12/03 [EagleBK] [OP_BTS-11276] HIENG END.
	if( strncmp( m_stSysStatInfo.st_SystemInfoParam.sc_Sys50, sc_Maker, TYPD_SSTS_MakerNameLength ) != 0 ){
		memset( m_stSysStatInfo.st_SystemInfoParam.sc_Sys50, (char)SSTD_ZERO_OUT, TYPD_SSTS_MakerNameLength );
		strncpy( m_stSysStatInfo.st_SystemInfoParam.sc_Sys50, sc_Maker, TYPD_SSTS_MakerNameLength );
//Str【 静的解析対応 : Sky田村 : 2012.12.11 : CID-435212】.
		m_stSysStatInfo.st_SystemInfoParam.sc_Sys50[TYPD_SSTS_MakerNameLength-1] = '\0';
//End【 静的解析対応 : Sky田村 : 2012.12.11 : CID-435212】.
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SYS_50;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_SystemInfoParam.sc_Sys50 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// 省略メーカー名.
	char sc_SMaker[ TYPD_SSTS_ShortMakerNameLength ];
	psc_Maker = NULL;
	memset( sc_SMaker, (char)SSTD_ZERO_OUT, TYPD_SSTS_ShortMakerNameLength );
	psc_Maker = pc_SystemInfo->getShortMfg();
// 2020/01/16 [EagleBK] [AddressSanitizer_2] HIENG START.
//	memcpy( sc_SMaker, psc_Maker, TYPD_SSTS_ShortMakerNameLength );
	strncpy( sc_SMaker, psc_Maker, TYPD_SSTS_ShortMakerNameLength - 1 );
// 2020/01/16 [EagleBK] [AddressSanitizer_2] HIENG END.
	if( strncmp( m_stSysStatInfo.st_SystemInfoParam.sc_Sys51, sc_SMaker, TYPD_SSTS_ShortMakerNameLength ) != 0 ){
		memset( m_stSysStatInfo.st_SystemInfoParam.sc_Sys51, (char)SSTD_ZERO_OUT, TYPD_SSTS_ShortMakerNameLength );
		strncpy( m_stSysStatInfo.st_SystemInfoParam.sc_Sys51, sc_SMaker, TYPD_SSTS_ShortMakerNameLength );
//Str【 静的解析対応 : Sky田村 : 2012.12.11 : CID-435212】.
		m_stSysStatInfo.st_SystemInfoParam.sc_Sys51[TYPD_SSTS_ShortMakerNameLength-1] = '\0';
//End【 静的解析対応 : Sky田村 : 2012.12.11 : CID-435212】.
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SYS_51;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_SystemInfoParam.sc_Sys51 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// ブランド名.
	char sc_Brand[ TYPD_SSTS_BrandNameLength ];
	psc_Maker = NULL;
	memset( sc_Brand, (char)SSTD_ZERO_OUT, TYPD_SSTS_BrandNameLength );
	psc_Maker = pc_SystemInfo->getBrandName();
// 2020/01/16 [EagleBK] [AddressSanitizer_2] HIENG START.
//	memcpy( sc_Brand, psc_Maker, TYPD_SSTS_BrandNameLength );
	strncpy( sc_Brand, psc_Maker, TYPD_SSTS_BrandNameLength - 1 );
// 2020/01/16 [EagleBK] [AddressSanitizer_2] HIENG END.
	if( strncmp( m_stSysStatInfo.st_SystemInfoParam.sc_Sys52, sc_Brand, TYPD_SSTS_BrandNameLength ) != 0 ){
		memset( m_stSysStatInfo.st_SystemInfoParam.sc_Sys52, (char)SSTD_ZERO_OUT, TYPD_SSTS_BrandNameLength );
		strncpy( m_stSysStatInfo.st_SystemInfoParam.sc_Sys52, sc_Brand, TYPD_SSTS_BrandNameLength );
//Str【 静的解析対応 : Sky田村 : 2012.12.11 : CID-435212】.
		m_stSysStatInfo.st_SystemInfoParam.sc_Sys52[TYPD_SSTS_BrandNameLength-1] = '\0';
//End【 静的解析対応 : Sky田村 : 2012.12.11 : CID-435212】.
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SYS_52;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_SystemInfoParam.sc_Sys52 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// 製品名(モデル名).
	char sc_ModelName[ TYPD_SSTS_ModelNameLength ];
	psc_Maker = NULL;
	memset( sc_ModelName, (char)SSTD_ZERO_OUT, TYPD_SSTS_ModelNameLength );
	psc_Maker = pc_SystemInfo->getModelName();
// 2020/01/16 [EagleBK] [AddressSanitizer_2] HIENG START.
//	memcpy( sc_ModelName, psc_Maker, TYPD_SSTS_ModelNameLength );
	strncpy( sc_ModelName, psc_Maker, TYPD_SSTS_ModelNameLength - 1 );
// 2020/01/16 [EagleBK] [AddressSanitizer_2] HIENG END.
	if( strncmp( m_stSysStatInfo.st_SystemInfoParam.sc_Sys53, sc_ModelName, TYPD_SSTS_ModelNameLength ) != 0 ){
		memset( m_stSysStatInfo.st_SystemInfoParam.sc_Sys53, (char)SSTD_ZERO_OUT, TYPD_SSTS_ModelNameLength );
		strncpy( m_stSysStatInfo.st_SystemInfoParam.sc_Sys53, sc_ModelName, TYPD_SSTS_ModelNameLength );
//Str【 静的解析対応 : Sky田村 : 2012.12.11 : CID-435212】.
		m_stSysStatInfo.st_SystemInfoParam.sc_Sys53[TYPD_SSTS_ModelNameLength-1] = '\0';
//End【 静的解析対応 : Sky田村 : 2012.12.11 : CID-435212】.
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SYS_53;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_SystemInfoParam.sc_Sys53 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// パネル起動完了済みの設定 取得.
	b_Val = pc_SystemInfo->isPanelActivated();
	if( m_stSysStatInfo.st_SystemInfoParam.b_Sys57 != b_Val ){
		m_stSysStatInfo.st_SystemInfoParam.b_Sys57 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SYS_57;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_SystemInfoParam.b_Sys57 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// Thames1P モデル.
	TYPE_PrinterModelKind e_PrtMdlKind = pc_SystemInfo->getPrinterModelKind();
	if( m_stSysStatInfo.st_SystemInfoParam.e_Sys58 != e_PrtMdlKind ){
		m_stSysStatInfo.st_SystemInfoParam.e_Sys58 = e_PrtMdlKind;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SYS_58;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_SystemInfoParam.e_Sys58 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// 起動時にiモデルかどうか判定.
	TYPS_IFunctionOn_InfoTubeOn st_IFuncOn = pc_SystemInfo->isIFunctionOnInfoTubeOn();
	if( m_stSysStatInfo.st_SystemInfoParam.st_Sys61.b_IFunctionOn != st_IFuncOn.b_IFunctionOn ||
		m_stSysStatInfo.st_SystemInfoParam.st_Sys61.b_InfoTubeOn  != st_IFuncOn.b_InfoTubeOn ){
		m_stSysStatInfo.st_SystemInfoParam.st_Sys61 = st_IFuncOn;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SYS_61;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_SystemInfoParam.st_Sys61 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// Emu機判定.
	b_Val = pc_SystemInfo->isEmulatorMachine();
	if( m_stSysStatInfo.st_SystemInfoParam.b_Sys63 != b_Val ){
		m_stSysStatInfo.st_SystemInfoParam.b_Sys63 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SYS_63;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_SystemInfoParam.b_Sys63 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// MFPの機能バージョン.
	SYSE_FuncVer e_FuncVer = pc_SystemInfo->getFuncVer();
	if( m_stSysStatInfo.st_SystemInfoParam.e_Sys64 != e_FuncVer ){
		m_stSysStatInfo.st_SystemInfoParam.e_Sys64 = e_FuncVer;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SYS_64;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_SystemInfoParam.e_Sys64 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// i-option kit装着有無取得.
	b_Val = pc_SystemInfo->isIoptionKitOn();
	if( m_stSysStatInfo.st_SystemInfoParam.b_Sys65 != b_Val ){
		m_stSysStatInfo.st_SystemInfoParam.b_Sys65 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SYS_65;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_SystemInfoParam.b_Sys65 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}

	// MFPが外部メモリから起動されたかどうかを判別する.
	b_Val = pc_SystemInfo->isMediaBootType();
	if( m_stSysStatInfo.st_SystemInfoParam.b_Sys69 != b_Val ){
		m_stSysStatInfo.st_SystemInfoParam.b_Sys69 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SYS_69;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_SystemInfoParam.b_Sys69 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// 製品機判定.
	b_Val = pc_SystemInfo->isProductMachine();
	if( m_stSysStatInfo.st_SystemInfoParam.b_Sys70 != b_Val ){
		m_stSysStatInfo.st_SystemInfoParam.b_Sys70 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SYS_70;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_SystemInfoParam.b_Sys70 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// アウトラインフォントデータのチェックサム値.
	ushort us_Val = pc_SystemInfo->getOutLineFontRomCheckSum();
	if( m_stSysStatInfo.st_SystemInfoParam.us_Sys72 != us_Val ){
		m_stSysStatInfo.st_SystemInfoParam.us_Sys72 = us_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SYS_72;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_SystemInfoParam.us_Sys72 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// 1パス両面 裏面用Asic200基板 装着状態取得.
	b_Val = pc_SystemInfo->isCompExpandAsicReverseSideExist();
	if( m_stSysStatInfo.st_SystemInfoParam.b_Sys73 != b_Val ){
		m_stSysStatInfo.st_SystemInfoParam.b_Sys73 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SYS_73;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_SystemInfoParam.b_Sys73 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// DSモデルであるか否かの判断.
	b_Val = pc_SystemInfo->isDSModel();
	if( m_stSysStatInfo.st_SystemInfoParam.b_Sys74 != b_Val ){
		m_stSysStatInfo.st_SystemInfoParam.b_Sys74 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SYS_74;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_SystemInfoParam.b_Sys74 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	SST_LOG2(MFPE_LogLevelAll, "End.\n", NULL);
}

//************************************************************************************************************************.
//概要　　：システム情報取得処理 (初期化完了時にポーリング止める).
//戻り値　：無し.
//備考　　：特に無し.
//作成者　：高橋.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
// 2012. 5.31 [CQ363516] パフォーマンス改善 高周期のポーリング処理で、各条件に該当する場合に変化点検出処理を実行する.
//************************************************************************************************************************.
void SSTC_SystemStatusService::getSystemInfoStatus_Low_stop2()
{
	SST_LOG(MFPE_LogLevelAll);
	SSTS_ChangedInfo stChangedInfo;

	//************************************************************************.
	// 情報を取得してシステム情報と比較し変化があれば変化情報に追加.
	//************************************************************************.
	APIC_SystemInfo* pc_SystemInfo = APIC_SystemInfo::newInstance();

	// 起動時に特別なモード(サービスマンモード)で起動したか.
	Bool b_Val = pc_SystemInfo->isExtraMode();
	if( m_stSysStatInfo.st_SystemInfoParam.b_Sys46 != b_Val ){
		m_stSysStatInfo.st_SystemInfoParam.b_Sys46 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SYS_46;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_SystemInfoParam.b_Sys46 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// 起動時に特別なモード(カードラインモード)で起動したか.
	b_Val = pc_SystemInfo->isCardLineMode();
	if( m_stSysStatInfo.st_SystemInfoParam.b_Sys47 != b_Val ){
		m_stSysStatInfo.st_SystemInfoParam.b_Sys47 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SYS_47;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_SystemInfoParam.b_Sys47 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	SST_LOG2(MFPE_LogLevelAll, "End.\n", NULL);
}

//************************************************************************************************************************.
//概要　　：スキャナ情報取得処理 (初期化完了時にポーリング止める).
//戻り値　：無し.
//備考　　：特に無し.
//作成者　：高橋.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
// 2012. 5.15 [CQ363516] パフォーマンス改善 Observerで検出可能な情報をポーリング→Observerに変更.
// 2012. 5.15 [CQ363516] パフォーマンス改善 ポーリング周期を500msと2sに変更.
//************************************************************************************************************************.
void SSTC_SystemStatusService::getScannerInfoStatus_Low_Stop()
{
	SST_LOG(MFPE_LogLevelAll);
	SSTS_ChangedInfo stChangedInfo;
	
	APIC_ScannerSystem* pc_ScanerSystem = APIC_ScannerSystem::newInstance();
	
	// ＡＤＦ装着＆装着タイプ.
	TYPE_ADFType e_Type = pc_ScanerSystem->getADFType();
	if( m_stSysStatInfo.st_ScannerInfoParam.e_Scn9 != e_Type ){
		m_stSysStatInfo.st_ScannerInfoParam.e_Scn9 = e_Type;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SCN_9;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_ScannerInfoParam.e_Scn9 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// ADF読取機構.
	TYPE_ADFMechanism e_Mecha = pc_ScanerSystem->getADFMechanism();
	if( m_stSysStatInfo.st_ScannerInfoParam.e_Scn10 != e_Mecha ){
		m_stSysStatInfo.st_ScannerInfoParam.e_Scn10 = e_Mecha;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SCN_10;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_ScannerInfoParam.e_Scn10 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}

//Str MinervaSBK 機器構成変更_No.13 DFのFCOT短縮対応 H.Ochi 15.02.26.
	// 原稿トレイ事前上昇機能有無.
	Bool b_Val = pc_ScanerSystem->isEdhTrayUpEnable();
	if( m_stSysStatInfo.st_ScannerInfoParam.b_Scn31 != b_Val ){
		m_stSysStatInfo.st_ScannerInfoParam.b_Scn31  = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SCN_31;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_ScannerInfoParam.b_Scn31 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
//End MinervaSBK 機器構成変更_No.13 DFのFCOT短縮対応 H.Ochi 15.02.26.

//Str【機器構成変更No.11 DFスキュー量パネル表示】M.Koyanagi 2015/04/27.
	// DF調整値バックアップ機能有無.
	b_Val = pc_ScanerSystem->isEdhAdjustBackUp();
	if( m_stSysStatInfo.st_ScannerInfoParam.b_Scn32 != b_Val ){
		m_stSysStatInfo.st_ScannerInfoParam.b_Scn32  = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SCN_32;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_ScannerInfoParam.b_Scn32 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
//End【機器構成変更No.11 DFスキュー量パネル表示】M.Koyanagi 2015/04/27.

//Str【CQ521390:getEDHProductTypeのBlackAPI化対応】M.Koyanagi 2015/11/12.
	// DF製品タイプを取得.
	TYPE_EDHProductType e_EDHProductType = pc_ScanerSystem->getEDHProductType();
	if( m_stSysStatInfo.st_ScannerInfoParam.e_Scn34 != e_EDHProductType ){
		m_stSysStatInfo.st_ScannerInfoParam.e_Scn34  = e_EDHProductType;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SCN_34;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_ScannerInfoParam.e_Scn34 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
//End【CQ521390:getEDHProductTypeのBlackAPI化対応】M.Koyanagi 2015/11/12.

//Str【Eagle 機器構成変更No.86:重送検出機能追加対応】M.Koyanagi 2017/09/05.
	// 重送検知機能有無.
	b_Val = pc_ScanerSystem->isExistDetectMultiFeed();
	if( m_stSysStatInfo.st_ScannerInfoParam.b_Scn36 != b_Val ){
		m_stSysStatInfo.st_ScannerInfoParam.b_Scn36  = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SCN_36;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_ScannerInfoParam.b_Scn36 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
//End【Eagle 機器構成変更No.86:重送検出機能追加対応】M.Koyanagi 2017/09/05.

// 【追加】2017.12.22 鈴木希 [start] Eagle 機器構成変更_No.86 重送検出センサ調整対応. 
	// 重送検知調整の方式を取得. 
	TYPE_EDHAdjustTypeMultiFeedDetect e_AdjustTypeMultiFeedDetec = pc_ScanerSystem->getAdjustTypeMultiFeedDetect();
	if( m_stSysStatInfo.st_ScannerInfoParam.e_Scn37 != e_AdjustTypeMultiFeedDetec ){
		m_stSysStatInfo.st_ScannerInfoParam.e_Scn37  = e_AdjustTypeMultiFeedDetec;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SCN_37;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_ScannerInfoParam.e_Scn37 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
// 【追加】2017.12.22 鈴木希 [ end ] Eagle 機器構成変更_No.86 重送検出センサ調整対応. 

//Str【DenebMLK 制御構成変更No.20 DF不要機能削除】M.Koyanagi 2018/05/14.
	// ガラス清掃機能有無判定.
	b_Val = pc_ScanerSystem->isGlassCleaningEnable();
	if( m_stSysStatInfo.st_ScannerInfoParam.b_Scn38 != b_Val ){
		m_stSysStatInfo.st_ScannerInfoParam.b_Scn38  = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SCN_38;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_ScannerInfoParam.b_Scn38 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
//End【DenebMLK 制御構成変更No.20 DF不要機能削除】M.Koyanagi 2018/05/14.

//Str【Sparrow 制御構成変更No.254 【AIO1ラインCIS】CIS変更 CCD/CIS搭載判定】T.Kurihara 2018/06/22.
#ifdef DEF_A4_Cus
	// プラテンのスキャナセンサー種類を取得.
	TYPE_PlatenScannerType e_PlatenScannerType = pc_ScanerSystem->getPlatenScannerType();
	if( m_stSysStatInfo.st_ScannerInfoParam.e_Scn39 != e_PlatenScannerType ){
		m_stSysStatInfo.st_ScannerInfoParam.e_Scn39  = e_PlatenScannerType;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SCN_39;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_ScannerInfoParam.e_Scn39 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
#endif	// DEF_A4_Cus
//End【Sparrow 制御構成変更No.254 【AIO1ラインCIS】CIS変更 CCD/CIS搭載判定】T.Kurihara 2018/06/22.

#ifdef DEF_A4_Cus
//Str. Sparrow 機器構成変更_No.285_AR1062416課題追加対応 読取可能最大サイズ取得対応 S.Kagawa 2018/10/31.
	// プラテン読取可能最大サイズ取得.
	TYPC_PaperSize c_MaxReadableSize = pc_ScanerSystem->getMaxPlatenReadableSize();
	if( m_stSysStatInfo.st_ScannerInfoParam.st_Scn40.e_SizeCode != c_MaxReadableSize.e_SizeCode ||
		m_stSysStatInfo.st_ScannerInfoParam.st_Scn40.sf_x       != c_MaxReadableSize.c_SizeMM.x ||
		m_stSysStatInfo.st_ScannerInfoParam.st_Scn40.sf_y       != c_MaxReadableSize.c_SizeMM.y ||
		m_stSysStatInfo.st_ScannerInfoParam.st_Scn40.b_WideOn   != c_MaxReadableSize.b_WideOn){
		m_stSysStatInfo.st_ScannerInfoParam.st_Scn40.e_SizeCode  = c_MaxReadableSize.e_SizeCode;
		m_stSysStatInfo.st_ScannerInfoParam.st_Scn40.sf_x        = c_MaxReadableSize.c_SizeMM.x;
		m_stSysStatInfo.st_ScannerInfoParam.st_Scn40.sf_y        = c_MaxReadableSize.c_SizeMM.y;
		m_stSysStatInfo.st_ScannerInfoParam.st_Scn40.b_WideOn    = c_MaxReadableSize.b_WideOn;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SCN_40;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_ScannerInfoParam.st_Scn40 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	// ADF読取可能最大サイズ取得.
	c_MaxReadableSize = pc_ScanerSystem->getMaxAdfReadableSize();
	if( m_stSysStatInfo.st_ScannerInfoParam.st_Scn41.e_SizeCode != c_MaxReadableSize.e_SizeCode ||
		m_stSysStatInfo.st_ScannerInfoParam.st_Scn41.sf_x       != c_MaxReadableSize.c_SizeMM.x ||
		m_stSysStatInfo.st_ScannerInfoParam.st_Scn41.sf_y       != c_MaxReadableSize.c_SizeMM.y ||
		m_stSysStatInfo.st_ScannerInfoParam.st_Scn41.b_WideOn   != c_MaxReadableSize.b_WideOn){
		m_stSysStatInfo.st_ScannerInfoParam.st_Scn41.e_SizeCode  = c_MaxReadableSize.e_SizeCode;
		m_stSysStatInfo.st_ScannerInfoParam.st_Scn41.sf_x        = c_MaxReadableSize.c_SizeMM.x;
		m_stSysStatInfo.st_ScannerInfoParam.st_Scn41.sf_y        = c_MaxReadableSize.c_SizeMM.y;
		m_stSysStatInfo.st_ScannerInfoParam.st_Scn41.b_WideOn    = c_MaxReadableSize.b_WideOn;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SCN_41;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_ScannerInfoParam.st_Scn41 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
//End. Sparrow 機器構成変更_No.285_AR1062416課題追加対応 読取可能最大サイズ取得対応 S.Kagawa 2018/10/31.
#endif	// DEF_A4_Cus

// 【追加】2022.07.05 鈴木希 [start] IT6_2.2MR3 機器構成変更No.1 DF-713 副走査方向の調整マージン不足対応. 
	// ADFフィード位置調整範囲の拡大機能. 
	b_Val = pc_ScanerSystem->isAdfFeedPosAdjustRangeEnlargeFunc();
	if( m_stSysStatInfo.st_ScannerInfoParam.b_Scn42 != b_Val ){
		m_stSysStatInfo.st_ScannerInfoParam.b_Scn42  = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SCN_42;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_ScannerInfoParam.b_Scn42 );
		AddChangedInfo(&stChangedInfo);
	}
// 【追加】2022.07.05 鈴木希 [ end ] IT6_2.2MR3 機器構成変更No.1 DF-713 副走査方向の調整マージン不足対応. 



	SST_LOG2(MFPE_LogLevelAll, "End.\n", NULL);
}

//************************************************************************************************************************.
//概要　　：スキャナ情報取得処理.
//戻り値　：無し.
//備考　　：特に無し.
//作成者　：高橋.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
// 2012. 5.15 [CQ363516] パフォーマンス改善 Observerで検出可能な情報をポーリング→Observerに変更.
// 2012. 5.15 [CQ363516] パフォーマンス改善 ポーリング周期を500msと2sに変更.
//************************************************************************************************************************.
void SSTC_SystemStatusService::getScannerInfoStatus_High()
{
	SST_LOG(MFPE_LogLevelAll);
	SSTS_ChangedInfo stChangedInfo;
	
	// スキャナ警告追加：2011.2.18 <ADD> Start.
	
	APIC_ScannerWarning *pc_ScannerWarning = APIC_ScannerWarning::newInstance();
	
//	2017/03/17 IT5_4.0 1017494 Sky嵯峨山 ST.
	Bool b_Val = False;
	TYPC_FatalCommErrorJamCode c_FatalCommErrJamCode;
	Bool b_BootSelfCheckMode = APIC_SystemInfo::newInstance()->isBootSelfCheckMode();
	Bool b_AutoSelfCheckOn = APIC_SystemStatus::newInstance()->getAutoSelfCheckOn();
//	Bootシーケンス診断または、自動自己診断時には、LocalUIへ通知しないように検知を抑制.
	if ( ( False == b_BootSelfCheckMode ) && ( False == b_AutoSelfCheckOn ) ) {
		// ＦＡＴＡＬコードの取得.
		c_FatalCommErrJamCode = pc_ScannerWarning->getFatal();
		if( m_stSysStatInfo.st_ScannerInfoParam.us_Scn11 != c_FatalCommErrJamCode.us_Code ){
			m_stSysStatInfo.st_ScannerInfoParam.us_Scn11  = c_FatalCommErrJamCode.us_Code;
			stChangedInfo.e_DataID = TYPE_NTF_DataID_SCN_11;
			stChangedInfo.p_Value  = &( m_stSysStatInfo.st_ScannerInfoParam.us_Scn11 );
			// 2021/10/22 APF OP_BTS-35403 start
			//m_vChangedInfo.push_back(stChangedInfo);
			AddChangedInfo(&stChangedInfo);
			// 2021/10/22 APF OP_BTS-35403 end
#if defined(DEF_A4_Cus) && defined(DEF_KRONOSS_Cus)
			call_NPAG_setChagneSccanerFatal = True;
#endif // defined(DEF_A4_Cus) && defined(DEF_KRONOSS_Cus).
		}
	
		// ＦＡＴＡＬが発生しているか.
		b_Val = pc_ScannerWarning->isOccurFatal();
		if( m_stSysStatInfo.st_ScannerInfoParam.b_Scn12 != b_Val ){
			m_stSysStatInfo.st_ScannerInfoParam.b_Scn12  = b_Val;
			stChangedInfo.e_DataID = TYPE_NTF_DataID_SCN_12;
			stChangedInfo.p_Value  = &( m_stSysStatInfo.st_ScannerInfoParam.b_Scn12 );
			// 2021/10/22 APF OP_BTS-35403 start
			//m_vChangedInfo.push_back(stChangedInfo);
			AddChangedInfo(&stChangedInfo);
			// 2021/10/22 APF OP_BTS-35403 end
		}
	
		// ＦＡＴＡＬ Ｌｅｖｅｌ確認処理.
		TYPE_FatalLevel e_FatalLevel = pc_ScannerWarning->getFatalLevel();
		if( m_stSysStatInfo.st_ScannerInfoParam.e_Scn13 != e_FatalLevel ){
			m_stSysStatInfo.st_ScannerInfoParam.e_Scn13  = e_FatalLevel;
			stChangedInfo.e_DataID = TYPE_NTF_DataID_SCN_13;
			stChangedInfo.p_Value  = &( m_stSysStatInfo.st_ScannerInfoParam.e_Scn13 );
			// 2021/10/22 APF OP_BTS-35403 start
			//m_vChangedInfo.push_back(stChangedInfo);
			AddChangedInfo(&stChangedInfo);
			// 2021/10/22 APF OP_BTS-35403 end
#if defined(DEF_A4_Cus) && defined(DEF_KRONOSS_Cus)
			call_NPAG_setChagneSccanerFatal = True;
#endif // defined(DEF_A4_Cus) && defined(DEF_KRONOSS_Cus).
		}
#if defined(DEF_A4_Cus) && defined(DEF_KRONOSS_Cus)
		if( True == call_NPAG_setChagneSccanerFatal ){
			TYPE_FatalLevel e_FatalLevel = pc_ScannerWarning->getFatalLevel();
			// for sys:NPA.
			call_NPAG_setChagneSccanerFatal = False;
			NPAG_setChagneSccanerFatal( c_FatalCommErrJamCode.us_Code , e_FatalLevel );
		}
#endif // defined(DEF_A4_Cus) && defined(DEF_KRONOSS_Cus).
	}
//	2017/03/17 IT5_4.0 1017494 Sky嵯峨山 ED.
// 2016/09/02 APF 制御構成変更No51 制御トラブル検出時のHW自己診断自動化 s.
	// 自動自己診断実施判定.
	TYPS_FatalInfoForAutoSelfCheck st_FatalInfoForAutoSelfCheck = {
		TYPE_FatalOccurredKindScan,
		m_stSysStatInfo.st_ScannerInfoParam.us_Scn11,
		m_stSysStatInfo.st_ScannerInfoParam.e_Scn13
	};
	Bool b_AutoSelfCheck = APIC_SystemStatus::newInstance()->judgeAutoSelfCheck( &st_FatalInfoForAutoSelfCheck );
	if( m_stSysStatInfo.st_ScannerInfoParam.b_Scn35 != b_AutoSelfCheck ){
		m_stSysStatInfo.st_ScannerInfoParam.b_Scn35  = b_AutoSelfCheck;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SCN_35;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_ScannerInfoParam.b_Scn35 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
// 2016/09/02 APF 制御構成変更No51 制御トラブル検出時のHW自己診断自動化 e.
	
	// 周辺機器コミュニケーションエラーコードの取得.
	c_FatalCommErrJamCode = pc_ScannerWarning->getCommError();
	if( m_stSysStatInfo.st_ScannerInfoParam.us_Scn14 != c_FatalCommErrJamCode.us_Code ){
		m_stSysStatInfo.st_ScannerInfoParam.us_Scn14  = c_FatalCommErrJamCode.us_Code;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SCN_14;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_ScannerInfoParam.us_Scn14 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// 周辺機器コミュニケーションエラーが発生しているか .
	b_Val = pc_ScannerWarning->isOccurCommError();
	if( m_stSysStatInfo.st_ScannerInfoParam.b_Scn15 != b_Val ){
		m_stSysStatInfo.st_ScannerInfoParam.b_Scn15  = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SCN_15;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_ScannerInfoParam.b_Scn15 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// ＡＤＦジャムコードの取得.
	c_FatalCommErrJamCode = pc_ScannerWarning->getAdfJam();
	if( m_stSysStatInfo.st_ScannerInfoParam.us_Scn16 != c_FatalCommErrJamCode.us_Code ){
		m_stSysStatInfo.st_ScannerInfoParam.us_Scn16  = c_FatalCommErrJamCode.us_Code;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SCN_16;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_ScannerInfoParam.us_Scn16 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// ＡＤＦジャムが発生しているか.
	b_Val = pc_ScannerWarning->isOccurAdfJam();
	if( m_stSysStatInfo.st_ScannerInfoParam.b_Scn17 != b_Val ){
		m_stSysStatInfo.st_ScannerInfoParam.b_Scn17  = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SCN_17;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_ScannerInfoParam.b_Scn17 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// ジャム残留用紙が存在するか.
	Bool b_JamPaperExist[TYPD_SSTS_AdfJamPaperItemNum];
	Bool b_UpdFlg = False;
	memset( b_JamPaperExist, (char)SSTD_ZERO_OUT, TYPD_SSTS_AdfJamPaperItemNum );
	ushort us_Idx = 0;
	for( us_Idx = 0; us_Idx < (ushort)TYPD_SSTS_AdfJamPaperItemNum; ++us_Idx ){
		b_JamPaperExist[ us_Idx ] = pc_ScannerWarning->isJamPaperExist( (TYPE_AdfJamPos)us_Idx );
		if( m_stSysStatInfo.st_ScannerInfoParam.b_Scn18[ us_Idx ] != b_JamPaperExist[ us_Idx ] ){
			m_stSysStatInfo.st_ScannerInfoParam.b_Scn18[ us_Idx ]  = b_JamPaperExist[ us_Idx ];
			if ( b_UpdFlg == False ){ b_UpdFlg = True; }
		}
	}
	if( b_UpdFlg ){
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SCN_18;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_ScannerInfoParam.b_Scn18[0] );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// ドアが開いているか（個々のドア）.
	Bool b_AdfDoorOpen[TYPD_SSTS_AdfDoorOpenItemNum];
	b_UpdFlg = False;
	memset( b_AdfDoorOpen, (char)SSTD_ZERO_OUT, TYPD_SSTS_AdfDoorOpenItemNum );
	for( us_Idx = 0; us_Idx < (ushort)TYPD_SSTS_AdfDoorOpenItemNum; ++us_Idx ){
		b_AdfDoorOpen[ us_Idx ] = pc_ScannerWarning->isAdfDoorOpen( (TYPE_AdfDoor)us_Idx );
		if( m_stSysStatInfo.st_ScannerInfoParam.b_Scn19[ us_Idx ] != b_AdfDoorOpen[ us_Idx ] ){
			m_stSysStatInfo.st_ScannerInfoParam.b_Scn19[ us_Idx ]  = b_AdfDoorOpen[ us_Idx ];
			if ( b_UpdFlg == False ){ b_UpdFlg = True; }
		}
	}
	if( b_UpdFlg ){
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SCN_19;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_ScannerInfoParam.b_Scn19[0] );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// 何れかのドアが開いているか.
	b_Val = pc_ScannerWarning->isAdfDoorOpen();
	if( m_stSysStatInfo.st_ScannerInfoParam.b_Scn20 != b_Val ){
		m_stSysStatInfo.st_ScannerInfoParam.b_Scn20  = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SCN_20;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_ScannerInfoParam.b_Scn20 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// プラテンカバーが開いているか.
	b_Val = pc_ScannerWarning->isPlatenCoverOpen();
	if( m_stSysStatInfo.st_ScannerInfoParam.b_Scn21 != b_Val ){
		m_stSysStatInfo.st_ScannerInfoParam.b_Scn21  = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SCN_21;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_ScannerInfoParam.b_Scn21 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// スキャナ警告追加：2011.2.18 <ADD> End.
	
	// ATS/AMS対応：2011.05.10 <ADD> Start T.Takahashi.
	
	APIC_Original* pc_Original = APIC_Original::newInstance();
	
	// ＡＤＦ・原稿サイズ.
	TYPC_PaperSize c_PaperSize = pc_Original->getAdfOriginalSize();
	if( m_stSysStatInfo.st_ScannerInfoParam.st_Scn22.e_SizeCode != c_PaperSize.e_SizeCode ||
		m_stSysStatInfo.st_ScannerInfoParam.st_Scn22.sf_x       != c_PaperSize.c_SizeMM.x ||
		m_stSysStatInfo.st_ScannerInfoParam.st_Scn22.sf_y       != c_PaperSize.c_SizeMM.y ||
		m_stSysStatInfo.st_ScannerInfoParam.st_Scn22.b_WideOn   != c_PaperSize.b_WideOn){
		m_stSysStatInfo.st_ScannerInfoParam.st_Scn22.e_SizeCode  = c_PaperSize.e_SizeCode;
		m_stSysStatInfo.st_ScannerInfoParam.st_Scn22.sf_x        = c_PaperSize.c_SizeMM.x;
		m_stSysStatInfo.st_ScannerInfoParam.st_Scn22.sf_y        = c_PaperSize.c_SizeMM.y;
		m_stSysStatInfo.st_ScannerInfoParam.st_Scn22.b_WideOn    = c_PaperSize.b_WideOn;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SCN_22;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_ScannerInfoParam.st_Scn22 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// 手置き・原稿サイズ.
	c_PaperSize = pc_Original->getPlatenOriginalSize();
	if( m_stSysStatInfo.st_ScannerInfoParam.st_Scn23.e_SizeCode != c_PaperSize.e_SizeCode ||
		m_stSysStatInfo.st_ScannerInfoParam.st_Scn23.sf_x       != c_PaperSize.c_SizeMM.x ||
		m_stSysStatInfo.st_ScannerInfoParam.st_Scn23.sf_y       != c_PaperSize.c_SizeMM.y ||
		m_stSysStatInfo.st_ScannerInfoParam.st_Scn23.b_WideOn   != c_PaperSize.b_WideOn){
		m_stSysStatInfo.st_ScannerInfoParam.st_Scn23.e_SizeCode  = c_PaperSize.e_SizeCode;
		m_stSysStatInfo.st_ScannerInfoParam.st_Scn23.sf_x        = c_PaperSize.c_SizeMM.x;
		m_stSysStatInfo.st_ScannerInfoParam.st_Scn23.sf_y        = c_PaperSize.c_SizeMM.y;
		m_stSysStatInfo.st_ScannerInfoParam.st_Scn23.b_WideOn    = c_PaperSize.b_WideOn;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SCN_23;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_ScannerInfoParam.st_Scn23 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// ＡＤＦ・ガラス上に原稿あり.判定.
	b_Val = pc_Original->isADFDocOnPlaten();
	if( m_stSysStatInfo.st_ScannerInfoParam.b_Scn24 != b_Val ){
		m_stSysStatInfo.st_ScannerInfoParam.b_Scn24  = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SCN_24;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_ScannerInfoParam.b_Scn24 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// ＡＤＦ・原稿有り 判定.
	b_Val = pc_Original->isAdfOriginalExist();
	if( m_stSysStatInfo.st_ScannerInfoParam.b_Scn25 != b_Val ){
		m_stSysStatInfo.st_ScannerInfoParam.b_Scn25  = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SCN_25;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_ScannerInfoParam.b_Scn25 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// ＡＤＦ・原稿サイズ確定 判定.
	b_Val = pc_Original->isAdfOriginalSizeFixed();
	if( m_stSysStatInfo.st_ScannerInfoParam.b_Scn26 != b_Val ){
		m_stSysStatInfo.st_ScannerInfoParam.b_Scn26  = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SCN_26;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_ScannerInfoParam.b_Scn26 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// 手置き・原稿有り 判定.
	b_Val = pc_Original->isPlatenOriginalExist();
	if( m_stSysStatInfo.st_ScannerInfoParam.b_Scn27 != b_Val ){
		m_stSysStatInfo.st_ScannerInfoParam.b_Scn27  = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SCN_27;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_ScannerInfoParam.b_Scn27 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// 手置き・原稿サイズ確定 判定.
	b_Val = pc_Original->isPlatenOriginalSizeFixed();
	if( m_stSysStatInfo.st_ScannerInfoParam.b_Scn28 != b_Val ){
		m_stSysStatInfo.st_ScannerInfoParam.b_Scn28  = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SCN_28;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_ScannerInfoParam.b_Scn28 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// ATS/AMS対応：2011.05.10 <ADD> End T.Takahashi.
	
//Str【CQ517055対応 KRQ-49 不定形用紙サイズ自動検出機能の搭載】M.Koyanagi 2015/08/18.
	// 不定形原稿サイズの取得.
	c_PaperSize = pc_Original->getUnDefineOriginalSize();
	if( m_stSysStatInfo.st_ScannerInfoParam.st_Scn33.e_SizeCode != c_PaperSize.e_SizeCode ||
		m_stSysStatInfo.st_ScannerInfoParam.st_Scn33.sf_x       != c_PaperSize.c_SizeMM.x ||
		m_stSysStatInfo.st_ScannerInfoParam.st_Scn33.sf_y       != c_PaperSize.c_SizeMM.y ||
		m_stSysStatInfo.st_ScannerInfoParam.st_Scn33.b_WideOn   != c_PaperSize.b_WideOn){
		m_stSysStatInfo.st_ScannerInfoParam.st_Scn33.e_SizeCode  = c_PaperSize.e_SizeCode;
		m_stSysStatInfo.st_ScannerInfoParam.st_Scn33.sf_x        = c_PaperSize.c_SizeMM.x;
		m_stSysStatInfo.st_ScannerInfoParam.st_Scn33.sf_y        = c_PaperSize.c_SizeMM.y;
		m_stSysStatInfo.st_ScannerInfoParam.st_Scn33.b_WideOn    = c_PaperSize.b_WideOn;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SCN_33;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_ScannerInfoParam.st_Scn33 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
//End【CQ517055対応 KRQ-49 不定形用紙サイズ自動検出機能の搭載】M.Koyanagi 2015/08/18.
	
	SST_LOG2(MFPE_LogLevelAll, "End.\n", NULL);
}

//************************************************************************************************************************.
//概要　　：デバイス情報取得処理.
//戻り値　：無し.
//備考　　：特に無し.
//作成者　：高橋.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
// 2012. 5.15 [CQ363516] パフォーマンス改善 Observerで検出可能な情報をポーリング→Observerに変更.
// 2012. 5.15 [CQ363516] パフォーマンス改善 ポーリング周期を500msと2sに変更.
//************************************************************************************************************************.
void SSTC_SystemStatusService::getDeviceInfoStatus_High()
{
	SST_LOG(MFPE_LogLevelAll);
	SSTS_ChangedInfo stChangedInfo;
	
	// Job実行可否対応 2011.11.22 ADS K.Tsuboi Start.
	// キーモード(APIC_KeyDevice).
	TYPE_KeyMode e_Dev106 = APIC_KeyDevice::getKeyMode();
	if( e_Dev106 != m_stSysStatInfo.st_DeviceInfoParam.e_Dev106 ){
		m_stSysStatInfo.st_DeviceInfoParam.e_Dev106 = e_Dev106;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_106;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_DeviceInfoParam.e_Dev106 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	// Job実行可否対応 2011.11.22 ADS K.Tsuboi End.
	
	SST_LOG2(MFPE_LogLevelAll, "End.\n", NULL);
}
// STEP2：2010.12.10 <ADD> End   T.Takahashi.

// 2012. 5.15 [CQ363516] パフォーマンス改善 ポーリング周期を500msと2sに変更.
// 長尺紙可能用紙種類判定処理.
static TYPE_PaperType e_PaperTypeTable[] = {
		TYPE_PaperTypeNormal,			// 用紙種類：普通紙.
		TYPE_PaperRecycle1,				// 用紙種類：再生紙(1).
		TYPE_PaperRecycle2,				// 用紙種類：再生紙2.
		TYPE_PaperHiQualty1,			// 用紙種類：上質紙(1).
		TYPE_PaperHiQualty2,			// 用紙種類：上質紙2.
		TYPE_PaperPostCard,				// 用紙種類：はがき.
		TYPE_PaperTypeEnvelope,			// 用紙種類：封筒.
		TYPE_PaperTypeOHP,				// 用紙種類：ＯＨＰ.
		TYPE_PaperTypeLabel,			// 用紙種類：ラベル紙.
		TYPE_PaperType2ndOriginal,		// 用紙種類：第2原図.
		TYPE_PaperTypeUser1, 			// ユーザー紙1（普通紙用）.
		TYPE_PaperTypeUser2, 			// ユーザー紙2（普通紙用）.
		TYPE_PaperTypeUser3, 			// ユーザー紙3（厚紙１用）.
		TYPE_PaperTypeUser4, 			// ユーザー紙4（厚紙１＋用）.
		TYPE_PaperTypeUser5, 			// ユーザー紙5（厚紙２用）.
		TYPE_PaperTypeUser6, 			// ユーザー紙6（厚紙３用）.
		TYPE_PaperTypeIndex,			// 用紙種類：インデックス紙 → 回転補正有り(サイズがインデックス指定になっていること).
		TYPE_PaperTypeLetterHead,		// 用紙種類：レターヘッド紙 → 回転補正有り.
		TYPE_PaperTypeSpecial1,			// 用紙種類：特殊紙(1)→ 通常APSでは選択対象外.
		TYPE_PaperTypeNotDuplex1,		// 用紙種類：両面不可紙(1)→ 両面が不可+ただし、片面はAPS優先.
		TYPE_PaperTypeThin1,			// 用紙種類：薄紙(1).
		TYPE_PaperTypeThick1,			// 用紙種類：厚紙(1).
		TYPE_PaperTypeThick2,			// 用紙種類：厚紙2.
		TYPE_PaperTypeThick3,			// 用紙種類：厚紙3.
		TYPE_PaperTypeColor,			// 用紙種類：色紙.
		TYPE_PaperTypeGeneral,			// 用紙種類：普通紙ウラ紙.
		TYPE_PaperTypeDontcare,			// 用紙種類：用紙種類指定無し（用紙サイズのみで用紙選択する）.
		TYPE_PaperTypeThick1Plus,		// 用紙種類：厚紙(1+).
		TYPE_PaperTypeThick4,			// 用紙種類：厚紙(4).
		TYPE_PaperTypeNormal_2nd,		// 普通紙両面2面目.
		TYPE_PaperTypeThick1_2nd,		// 厚紙1両面2面目.
		TYPE_PaperTypeThick1Plus_2nd,	// 厚紙1+両面2面目.
		TYPE_PaperTypeThick2_2nd,		// 厚紙2両面2面目.
		TYPE_PaperTypeThick3_2nd,		// 厚紙3両面2面目.
		TYPE_PaperTypeThick4_2nd,		// 厚紙4両面2面目.
		TYPE_PaperTypeUser1_2nd,		// ユーザー紙1両面2面目（普通紙）.
		TYPE_PaperTypeUser2_2nd,		// ユーザー紙2両面2面目（普通紙）.
		TYPE_PaperTypeUser3_2nd,		// ユーザー紙3両面2面目（厚紙1）.
		TYPE_PaperTypeUser4_2nd,		// ユーザー紙4両面2面目（厚紙1+）.
		TYPE_PaperTypeUser5_2nd,		// ユーザー紙5両面2面目（厚紙2）.
		TYPE_PaperTypeUser6_2nd,		// ユーザー紙6両面2面目（厚紙3）.
		TYPE_PaperTypeUser7,			// ユーザー紙7（薄紙用）.
		TYPE_PaperTypeUser7_2nd,		// ユーザー紙7両面2面目（薄紙）.
// ADS坪井 → 2011/12/01 【パフォーマンス改善】isWarning対応 Start.
		TYPE_PaperTypeCustom1,			//	カスタマイズ用紙1.
		TYPE_PaperTypeCustom2,			//	カスタマイズ用紙2.
		TYPE_PaperTypeCustom3,			//	カスタマイズ用紙3.
		TYPE_PaperTypeCustom4,			//	カスタマイズ用紙4.
		TYPE_PaperTypeCustom5,			//	カスタマイズ用紙5.
		TYPE_PaperTypeCustom6,			//	カスタマイズ用紙6.
		TYPE_PaperTypeCustom7,			//	カスタマイズ用紙7.
		TYPE_PaperTypeCustom8,			//	カスタマイズ用紙8.
		TYPE_PaperTypeCustom9,			//	カスタマイズ用紙9.
		TYPE_PaperTypeCustom10,			//	カスタマイズ用紙10.
		TYPE_PaperTypeCustom11,			//	カスタマイズ用紙11.
		TYPE_PaperTypeCustom12,			//	カスタマイズ用紙12.
		TYPE_PaperTypeCustom13,			//	カスタマイズ用紙13.
		TYPE_PaperTypeCustom14,			//	カスタマイズ用紙14.
		TYPE_PaperTypeCustom15,			//	カスタマイズ用紙15.
		TYPE_PaperTypeCustom16,			//	カスタマイズ用紙16.
		TYPE_PaperTypeCustom17,			//	カスタマイズ用紙17.
		TYPE_PaperTypeCustom18,			//	カスタマイズ用紙18.
		TYPE_PaperTypeCustom19,			//	カスタマイズ用紙19.
		TYPE_PaperTypeCustom20,			//	カスタマイズ用紙20.
		TYPE_PaperTypeCustom1_2nd,		//	カスタマイズ用紙1両面2面目.
		TYPE_PaperTypeCustom2_2nd,		//	カスタマイズ用紙2両面2面目.
		TYPE_PaperTypeCustom3_2nd,		//	カスタマイズ用紙3両面2面目.
		TYPE_PaperTypeCustom4_2nd,		//	カスタマイズ用紙4両面2面目.
		TYPE_PaperTypeCustom5_2nd,		//	カスタマイズ用紙5両面2面目.
		TYPE_PaperTypeCustom6_2nd,		//	カスタマイズ用紙6両面2面目.
		TYPE_PaperTypeCustom7_2nd,		//	カスタマイズ用紙7両面2面目.
		TYPE_PaperTypeCustom8_2nd,		//	カスタマイズ用紙8両面2面目.
		TYPE_PaperTypeCustom9_2nd,		//	カスタマイズ用紙9両面2面目.
		TYPE_PaperTypeCustom10_2nd,		//	カスタマイズ用紙10両面2面目.
		TYPE_PaperTypeCustom11_2nd,		//	カスタマイズ用紙11両面2面目.
		TYPE_PaperTypeCustom12_2nd,		//	カスタマイズ用紙12両面2面目.
		TYPE_PaperTypeCustom13_2nd,		//	カスタマイズ用紙13両面2面目.
		TYPE_PaperTypeCustom14_2nd,		//	カスタマイズ用紙14両面2面目.
		TYPE_PaperTypeCustom15_2nd,		//	カスタマイズ用紙15両面2面目.
		TYPE_PaperTypeCustom16_2nd,		//	カスタマイズ用紙16両面2面目.
		TYPE_PaperTypeCustom17_2nd,		//	カスタマイズ用紙17両面2面目.
		TYPE_PaperTypeCustom18_2nd,		//	カスタマイズ用紙18両面2面目.
		TYPE_PaperTypeCustom19_2nd,		//	カスタマイズ用紙19両面2面目.
		TYPE_PaperTypeCustom20_2nd,		//	カスタマイズ用紙20両面2面目.
		TYPE_PaperTypeThin_2nd			//<	薄紙両面2面目.
// ADS坪井 → 2011/12/01 【パフォーマンス改善】isWarning対応 End.
#ifdef DEF_A3A4COMM
		,TYPE_PaperTypeNormalEx,		//	普通紙(Ex).
		TYPE_PaperTypeNormalEx_2nd,		//	普通紙(Ex)両面2面目.
		TYPE_PaperTypeGloss1,			//	用紙種類：光沢1.
		TYPE_PaperTypeGloss1Plus,		//	用紙種類：光沢1+.
		TYPE_PaperTypeGloss2			//	用紙種類：光沢2.
#endif //#ifdef DEF_A3A4COMM.
};

//************************************************************************************************************************.
//概要　　：デバイス情報プリンタ取得処理.
//戻り値　：無し.
//備考　　：特に無し.
//作成者　：谷本.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
// 2012. 5.15 [CQ363516] パフォーマンス改善 ポーリング周期を500msと2sに変更.
// 2012. 5.31 [CQ363516] パフォーマンス改善 高周期のポーリング処理で、各条件に該当する場合に変化点検出処理を実行する.
//************************************************************************************************************************.
void SSTC_SystemStatusService::getPrinterDeviceStatus_High()
{
	SST_LOG(MFPE_LogLevelAll);
	
	static uint32_t ul_PrevCounterWarning = (uint32_t)-1;	// 前回のAPIC_PrinterWarningObserver::dataChanged()発生回数 [初期値:-1].
	static uint32_t ul_PrevCounterStatus = (uint32_t)-1;	// 前回のAPIC_PrinterStatusObserver::dataChanged()発生回数 [初期値:-1].
	static Bool b_PrevDipSw = False;			// 前回のDipSw( 13, 4 )情報 [初期値:False].
	static Bool b_PrevLongPicturePrint = False;	// 前回のgetLongPicturePrint情報 [初期値:False].

	Bool b_DataObserver = False;	// APIC_PrinterStatusObserver::dataChanged()発生フラグ.
	
	// 一般の情報(下記以外).
	getPrinterDeviceStatus_High_general();

	// 警告情報.
	if (ul_PrevCounterWarning != m_ulCounterPrinterWarningObserver) {	// APIC_PrinterWarningObserver::dataChanged()が発生していた場合.
		getPrinterDeviceStatus_Opt_Warning();
	}
	ul_PrevCounterWarning = m_ulCounterPrinterWarningObserver;	// APIC_PrinterWarningObserver::dataChanged()発生回数退避.

	// UnitLife/Toner/ElevateTray/PunchScraps etc/Jam情報.
	if (ul_PrevCounterStatus != m_ulCounterPrinterStatusObserver) {	// APIC_PrinterStatusObserver::dataChanged()が発生していた場合.
		b_DataObserver = True;	// APIC_PrinterStatusObserver::dataChanged()発生フラグ 設定.
		getPrinterDeviceStatus_Opt_UnitLife();		// UnitLife情報.
		getPrinterDeviceStatus_Opt_Toner();			// Toner情報.
		getPrinterDeviceStatus_Opt_Other();			// ElevateTray/PunchScraps etc情報.
		getPrinterDeviceStatus_Opt_Jam();			// Jam情報.

	}
	ul_PrevCounterStatus = m_ulCounterPrinterStatusObserver;	// APIC_PrinterStatusObserver::dataChanged()発生回数退避.

	// Staple情報.
	Bool b_Value = SSTD_GETSOFTDIPSWBIT( 13, 4 );	// DipSw( 13, 4 )情報取得.
	if ( (b_DataObserver == True)	// APIC_PrinterStatusObserver::dataChanged()が発生していた場合.
		 || (b_PrevDipSw != b_Value) ) {	// APIC_SoftDipSw::getSoftDipSwBit( 13, 4 )が変化していた場合.
		getPrinterDeviceStatus_Opt_Staple();
	}
	b_PrevDipSw = b_Value;	// DipSw( 13, 4 )情報退避.

	// DoorOpen情報.
	b_Value = SSTD_GETLONGPICTUREPRINT();	// getLongPicturePrint情報取得.
	if ( (b_DataObserver == True)	// APIC_PrinterStatusObserver::dataChanged()が発生していた場合.
		 || (b_PrevLongPicturePrint != b_Value) ) {	// APIC_SystemSetting::getLongPicturePrintが変化していた場合.
		getPrinterDeviceStatus_Opt_DoorOpen();
	}
	b_PrevLongPicturePrint = b_Value;	// getLongPicturePrint情報退避.

	SST_LOG2(MFPE_LogLevelAll, "End.\n", NULL);
}

//************************************************************************************************************************.
//概要　　：デバイス情報プリンタ取得処理 (一般の情報).
//戻り値　：無し.
//備考　　：特に無し.
//作成者　：谷本.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
// 2012. 5.31 [CQ363516] パフォーマンス改善 高周期のポーリング処理で、各条件に該当する場合に変化点検出処理を実行する.
// 2014. 4.14 Sky徳永 Zeus5BK 機器構成変更No3 PRT関連異常警告項目追加対応.
//************************************************************************************************************************.
void SSTC_SystemStatusService::getPrinterDeviceStatus_High_general()
{
	SST_LOG(MFPE_LogLevelAll);
	SSTS_ChangedInfo stChangedInfo;
	
	APIC_PrinterWarning* pc_APIPrinterWarning = APIC_PrinterWarning::newInstance();
	APIC_PrinterStatus* pc_APIPrinterStatus = APIC_PrinterStatus::newInstance();
	
	//************************************************************************.
	// 情報を取得してシステムステータス情報と比較し変化があれば変化情報に追加.
	//************************************************************************.
	
	// Ｋトナー状態【★CSRC用】.
	char sc_Val = pc_APIPrinterWarning->getBlackTonerStatusForCSRC();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt13 != sc_Val){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt13 = sc_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_13;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt13);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// Ｃトナー状態【★CSRC用】.
	sc_Val = pc_APIPrinterWarning->getCyanTonerStatusForCSRC();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt24 != sc_Val){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt24 = sc_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_24;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt24);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// Ｍトナー状態【★CSRC用】.
	sc_Val = pc_APIPrinterWarning->getMagentaTonerStatusForCSRC();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt37 != sc_Val){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt37 = sc_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_37;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt37);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// STEP2：2010.12.10 <CHG> Start   T.Takahashi.
	//   同一のテーブルをパラメータに使用しているＡＰＩを一箇所に纏めた.
	//   STEP2での追加されたDataIDは 142 ～ 146.
	
	static TYPE_FeedTray e_FeedTrayTable[] = {
		TYPE_FeedTrayManual,										// トレイ選択：手差し.
		TYPE_FeedTray1,												// トレイ選択：トレイ1. 
		TYPE_FeedTray2,												// トレイ選択：トレイ2.
		TYPE_FeedTray3,												// トレイ選択：トレイ3.
		TYPE_FeedTray4,												// トレイ選択：トレイ4.
		TYPE_FeedTray5,												// トレイ選択：トレイ5.
		TYPE_FeedTrayPI1,											// トレイ選択：PIトレイ１ FS110m/FS210m/FS120mフィニッシャーに、オプションのPI110カバーシートフィーダが装着されている時有効.
		TYPE_FeedTrayPI2											// トレイ選択：PIトレイ２ FS110m/FS210m/FS120mフィニッシャーに、オプションのPI110カバーシートフィーダが装着されている時有効.
//		TYPE_FeedTrayUnselect										// トレイ選択：未選択.  APIC_****でエラーになるので引き渡さない.
	};
	
	APIC_FeedTray* pc_FeedTray                    = APIC_FeedTray::newInstance();
	Bool           b_SizeMediaErrorAtPaperSizeFlg = False;
	Bool           b_SizeMediaErrorAtPaperTypeFlg = False;
	Bool           b_SizeCodeFlg                  = False;
	Bool           b_PaperTypeFlg                 = False;
	Bool           b_TrayTypeFlg                  = False;
	// ATS/AMS対応：2011.05.10 <DEL/ADD> Start T.Takahashi.
	Bool           b_PaperStatusFlg               = False;
	Bool           b_TrayStatusFlg                = False;
	// ATS/AMS対応：2011.05.10 <DEL/ADD> End T.Takahashi.
	
// 2022/9/22 RQ-912対応 start.
	Bool           b_MediaAutoDetectFlg                = False;
	Bool           b_IsMediaUndecidedFlg                = False;
// 2022/9/22 RQ-912対応 end.
	
	for( ushort us_Idx = 0; us_Idx < sizeof( e_FeedTrayTable ) / sizeof( TYPE_FeedTray ); ++us_Idx ){ 
		
		TYPC_PaperSize c_PaperSize = pc_APIPrinterWarning->getSizeMediaErrorAtPaperSize( e_FeedTrayTable[ us_Idx ] );
		// サイズ／メディアエラー用紙サイズ情報.
		if( m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt45[ (ushort)e_FeedTrayTable[ us_Idx ] ].e_SizeCode != c_PaperSize.e_SizeCode ||
			m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt45[ (ushort)e_FeedTrayTable[ us_Idx ] ].sf_x       != c_PaperSize.c_SizeMM.x ||
			m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt45[ (ushort)e_FeedTrayTable[ us_Idx ] ].sf_y       != c_PaperSize.c_SizeMM.y ||
			m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt45[ (ushort)e_FeedTrayTable[ us_Idx ] ].b_WideOn   != c_PaperSize.b_WideOn ){
			m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt45[ (ushort)e_FeedTrayTable[ us_Idx ] ].e_SizeCode  = c_PaperSize.e_SizeCode;
			m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt45[ (ushort)e_FeedTrayTable[ us_Idx ] ].sf_x        = c_PaperSize.c_SizeMM.x;
			m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt45[ (ushort)e_FeedTrayTable[ us_Idx ] ].sf_y        = c_PaperSize.c_SizeMM.y;
			m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt45[ (ushort)e_FeedTrayTable[ us_Idx ] ].b_WideOn    = c_PaperSize.b_WideOn;
			if( b_SizeMediaErrorAtPaperSizeFlg == False ){
				b_SizeMediaErrorAtPaperSizeFlg = True;
			}
		}
		
		TYPC_PaperType c_PaperType = pc_APIPrinterWarning->getSizeMediaErrorAtPaperType( e_FeedTrayTable[ us_Idx ] );
		// サイズ／メディアエラー用紙種類情報.
		if( m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt46[ (ushort)e_FeedTrayTable[ us_Idx ] ].e_PaperType              != c_PaperType.getPaperType()         ||
			m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt46[ (ushort)e_FeedTrayTable[ us_Idx ] ].e_PaperColor             != c_PaperType.getPaperColor()        ||
			m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt46[ (ushort)e_FeedTrayTable[ us_Idx ] ].b_Feed2ndSidedPaper      != c_PaperType.getFeed2ndSidedPaper() ||
			m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt46[ (ushort)e_FeedTrayTable[ us_Idx ] ].e_WidePaperImagePosition != c_PaperType.getWidePaperImagePosition() ||
// ADS坪井 → 2011/12/01 【パフォーマンス改善】isWarning対応 Start.
			m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt46[ (ushort)e_FeedTrayTable[ us_Idx ] ].e_CustomPaperType        != c_PaperType.getCustomPaperType() ||
// ADS坪井 → 2011/12/01 【パフォーマンス改善】isWarning対応 End.
			m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt46[ (ushort)e_FeedTrayTable[ us_Idx ] ].b_PaperTypeNormalEx      != c_PaperType.getPaperTypeNormalEx() ){
			m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt46[ (ushort)e_FeedTrayTable[ us_Idx ] ].e_PaperType               = c_PaperType.getPaperType();
			m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt46[ (ushort)e_FeedTrayTable[ us_Idx ] ].e_PaperColor              = c_PaperType.getPaperColor();
			m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt46[ (ushort)e_FeedTrayTable[ us_Idx ] ].b_Feed2ndSidedPaper       = c_PaperType.getFeed2ndSidedPaper();
			m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt46[ (ushort)e_FeedTrayTable[ us_Idx ] ].e_WidePaperImagePosition  = c_PaperType.getWidePaperImagePosition();
// ADS坪井 → 2011/12/01 【パフォーマンス改善】isWarning対応 Start.
			m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt46[ (ushort)e_FeedTrayTable[ us_Idx ] ].e_CustomPaperType         = c_PaperType.getCustomPaperType();
// ADS坪井 → 2011/12/01 【パフォーマンス改善】isWarning対応 End.
			m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt46[ (ushort)e_FeedTrayTable[ us_Idx ] ].b_PaperTypeNormalEx       = c_PaperType.getPaperTypeNormalEx();
			if( b_SizeMediaErrorAtPaperTypeFlg == False ){
				b_SizeMediaErrorAtPaperTypeFlg = True;
			}
		}
		
		// ATS/AMS対応：2011.05.10 <CHG/ADD> Start T.Takahashi.
		
		// 用紙サイズ.
		c_PaperSize = pc_FeedTray->getPaperSize( e_FeedTrayTable[ us_Idx ] );
		if( m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt142[ (ushort)e_FeedTrayTable[ us_Idx ] ].e_SizeCode != c_PaperSize.e_SizeCode ||
			m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt142[ (ushort)e_FeedTrayTable[ us_Idx ] ].sf_x       != c_PaperSize.c_SizeMM.x ||
			m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt142[ (ushort)e_FeedTrayTable[ us_Idx ] ].sf_y       != c_PaperSize.c_SizeMM.y ||
			m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt142[ (ushort)e_FeedTrayTable[ us_Idx ] ].b_WideOn   != c_PaperSize.b_WideOn ){
			m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt142[ (ushort)e_FeedTrayTable[ us_Idx ] ].e_SizeCode  = c_PaperSize.e_SizeCode;
			m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt142[ (ushort)e_FeedTrayTable[ us_Idx ] ].sf_x        = c_PaperSize.c_SizeMM.x;
			m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt142[ (ushort)e_FeedTrayTable[ us_Idx ] ].sf_y        = c_PaperSize.c_SizeMM.y;
			m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt142[ (ushort)e_FeedTrayTable[ us_Idx ] ].b_WideOn    = c_PaperSize.b_WideOn;
			if( b_SizeCodeFlg == False ){
				b_SizeCodeFlg = True;
			}
		}
		
		// 用紙種類.
		c_PaperType = pc_FeedTray->getPaperType( e_FeedTrayTable[ us_Idx ] );
		if( m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt143[ (ushort)e_FeedTrayTable[ us_Idx ] ].e_PaperType              != c_PaperType.getPaperType()         ||
			m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt143[ (ushort)e_FeedTrayTable[ us_Idx ] ].e_PaperColor             != c_PaperType.getPaperColor()        ||
			m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt143[ (ushort)e_FeedTrayTable[ us_Idx ] ].b_Feed2ndSidedPaper      != c_PaperType.getFeed2ndSidedPaper() ||
			m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt143[ (ushort)e_FeedTrayTable[ us_Idx ] ].e_WidePaperImagePosition != c_PaperType.getWidePaperImagePosition() ||
// ADS坪井 → 2011/12/01 【パフォーマンス改善】isWarning対応 Start.
			m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt143[ (ushort)e_FeedTrayTable[ us_Idx ] ].e_CustomPaperType        != c_PaperType.getCustomPaperType() ||
// ADS坪井 → 2011/12/01 【パフォーマンス改善】isWarning対応 End.
			m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt143[ (ushort)e_FeedTrayTable[ us_Idx ] ].b_PaperTypeNormalEx      != c_PaperType.getPaperTypeNormalEx() ){
			m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt143[ (ushort)e_FeedTrayTable[ us_Idx ] ].e_PaperType               = c_PaperType.getPaperType();
			m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt143[ (ushort)e_FeedTrayTable[ us_Idx ] ].e_PaperColor              = c_PaperType.getPaperColor();
			m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt143[ (ushort)e_FeedTrayTable[ us_Idx ] ].b_Feed2ndSidedPaper       = c_PaperType.getFeed2ndSidedPaper();
			m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt143[ (ushort)e_FeedTrayTable[ us_Idx ] ].e_WidePaperImagePosition  = c_PaperType.getWidePaperImagePosition();
// ADS坪井 → 2011/12/01 【パフォーマンス改善】isWarning対応 Start.
			m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt143[ (ushort)e_FeedTrayTable[ us_Idx ] ].e_CustomPaperType         = c_PaperType.getCustomPaperType();
// ADS坪井 → 2011/12/01 【パフォーマンス改善】isWarning対応 End.
			m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt143[ (ushort)e_FeedTrayTable[ us_Idx ] ].b_PaperTypeNormalEx       = c_PaperType.getPaperTypeNormalEx();
			if( b_PaperTypeFlg == False ){
				b_PaperTypeFlg = True;
			}
		}
		
		// トレイタイプ.
		TYPE_TrayType e_TrayType = pc_FeedTray->getTrayType( e_FeedTrayTable[ us_Idx ] );
		if( m_stSysStatInfo.st_PrinterDeviceInfoParam.e_DevPrt144[ (ushort)e_FeedTrayTable[ us_Idx ] ] != e_TrayType ){
			m_stSysStatInfo.st_PrinterDeviceInfoParam.e_DevPrt144[ (ushort)e_FeedTrayTable[ us_Idx ] ]  = e_TrayType;
			if( b_TrayTypeFlg == False ){
				b_TrayTypeFlg = True;
			}
		}
		
		// 用紙状態.
		TYPE_PaperStatus e_PaperStatus = pc_FeedTray->getPaperStatus( e_FeedTrayTable[ us_Idx ] );
		if( m_stSysStatInfo.st_PrinterDeviceInfoParam.e_DevPrt145[ (ushort)e_FeedTrayTable[ us_Idx ] ] != e_PaperStatus ){
			m_stSysStatInfo.st_PrinterDeviceInfoParam.e_DevPrt145[ (ushort)e_FeedTrayTable[ us_Idx ] ]  = e_PaperStatus;
			if( b_PaperStatusFlg == False ){
				b_PaperStatusFlg = True;
			}
		}
		
		// 給紙トレイ状態.
		TYPE_TrayStatus e_TrayStatus = pc_FeedTray->getTrayStatus( e_FeedTrayTable[ us_Idx ] );
		if( m_stSysStatInfo.st_PrinterDeviceInfoParam.e_DevPrt146[ (ushort)e_FeedTrayTable[ us_Idx ] ] != e_TrayStatus ){
			m_stSysStatInfo.st_PrinterDeviceInfoParam.e_DevPrt146[ (ushort)e_FeedTrayTable[ us_Idx ] ]  = e_TrayStatus;
			if( b_TrayStatusFlg == False ){
				b_TrayStatusFlg = True;
			}
		}
		
		// ATS/AMS対応：2011.05.10 <CHG/ADD> End T.Takahashi.
// 2022/9/22 RQ-912対応 start.
		// メディア自動検知.
		Bool b_MediaAutoDetect = pc_FeedTray->getMediaAutoDetect( e_FeedTrayTable[ us_Idx ] );
		if( m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt180[ (ushort)e_FeedTrayTable[ us_Idx ] ] != b_MediaAutoDetect ){
			m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt180[ (ushort)e_FeedTrayTable[ us_Idx ] ]  = b_MediaAutoDetect;
			if( b_MediaAutoDetectFlg == False ){
				b_MediaAutoDetectFlg = True;
			}
		}
		// メディア未確定確認.
		Bool b_isMediaUndecided = pc_FeedTray->isMediaUndecided( e_FeedTrayTable[ us_Idx ] );
		if( m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt181[ (ushort)e_FeedTrayTable[ us_Idx ] ] != b_isMediaUndecided ){
			m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt181[ (ushort)e_FeedTrayTable[ us_Idx ] ]  = b_isMediaUndecided;
			if( b_IsMediaUndecidedFlg == False ){
				b_IsMediaUndecidedFlg = True;
			}
		}
// 2022/9/22 RQ-912対応 end.
		
	}
	
	// それぞれの更新チェック.
	if( b_SizeMediaErrorAtPaperSizeFlg ){
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_45;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt45[ 0 ]);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	if( b_SizeMediaErrorAtPaperTypeFlg ){
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_46;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt46[ 0 ]);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}

	// ATS/AMS対応：2011.05.10 <CHG/ADD> Start T.Takahashi.
	
	if( b_SizeCodeFlg ){
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_142;
		stChangedInfo.p_Value = &( m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt142[ 0 ] );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	if( b_PaperTypeFlg ){
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_143;
		stChangedInfo.p_Value = &( m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt143[ 0 ] );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	if( b_TrayTypeFlg ){
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_144;
		stChangedInfo.p_Value = &( m_stSysStatInfo.st_PrinterDeviceInfoParam.e_DevPrt144[ 0 ] );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	if( b_PaperStatusFlg ){
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_145;
		stChangedInfo.p_Value = &( m_stSysStatInfo.st_PrinterDeviceInfoParam.e_DevPrt145[ 0 ] );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	if( b_TrayStatusFlg ){
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_146;
		stChangedInfo.p_Value = &( m_stSysStatInfo.st_PrinterDeviceInfoParam.e_DevPrt146[ 0 ] );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// ATS/AMS対応：2011.05.10 <CHG/ADD> End T.Takahashi.
	
	// STEP2：2010.12.10 <CHG> End     T.Takahashi.

// 2022/9/29 RQ-912対応 start.
	if( b_MediaAutoDetectFlg ){
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_180;
		stChangedInfo.p_Value = &( m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt180[ 0 ] );
		AddChangedInfo(&stChangedInfo);
	}
	if( b_IsMediaUndecidedFlg ){
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_181;
		stChangedInfo.p_Value = &( m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt181[ 0 ] );
		AddChangedInfo(&stChangedInfo);
	}
// 2022/9/29 RQ-912対応 end.

	//Str【MZ_3rd】【UV-01273 手差しアンマッチ状態対応（共有メモリ化） : 奥本(ICS) : 2013/04/30.】
	// 手差しアンマッチ状態.
	sc_Val = pc_FeedTray->isManualTrayPaperSizeUnmatch();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt150 != sc_Val){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt150 = sc_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_150;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt150);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
//End【MZ_3rd】【UV-01273 手差しアンマッチ状態対応（共有メモリ化） : 奥本(ICS) : 2013/04/30.】
	
	// トナー回収ボトル状態【★CSRC用】.
	sc_Val = pc_APIPrinterWarning->getTonerBottleStatusForCSRC();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt50 != sc_Val){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt50 = sc_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_50;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt50);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// Ｙトナー状態【★CSRC用】.
	sc_Val = pc_APIPrinterWarning->getYellowTonerStatusForCSRC();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt63 != sc_Val){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt63 = sc_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_63;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt63);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// DevPrt65:サイズ/メディアエラー解除(データなしの為、取得処理なし).
	
	// 装置構成確定.
	Bool b_Val = pc_APIPrinterStatus->isSystemConfigurationFixed();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt73 != b_Val){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt73 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_73;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt73);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// STEP2：2010.12.10 <CHG> End     T.Takahashi.
	
	// ＰＲＴ関連異常警告が発生しているか(指定Pコードは存在するか).
	static TYPE_PRTWarningCode  e_PrtWarningTable[] = {
		TYPE_PRTWarningCode_WP5,									// WP- 5:ＡＩＤＣセンサ異常（手前）.
		TYPE_PRTWarningCode_WP6,									// WP- 6:シアンＩＵ異常.
		TYPE_PRTWarningCode_WP7,									// WP- 7:マゼンタＩＵ異常.
		TYPE_PRTWarningCode_WP8,									// WP- 8:イエローＩＵ異常.
		TYPE_PRTWarningCode_WP9,									// WP- 9:ブラックＩＵ異常.
		TYPE_PRTWarningCode_WP21,									// WP-21:色ずれ補正異常（テストパターン異常）.
		TYPE_PRTWarningCode_WP22,									// WP-22:色ずれ補正異常（補正量異常）.
		TYPE_PRTWarningCode_WP26,									// WP-26:Ｋ１次転写ＡＴＶＣ異常.
		TYPE_PRTWarningCode_WP27,									// WP-27:２次転写ＡＴＶＣ異常.
		TYPE_PRTWarningCode_WP28,									// WP-28:ＡＩＤＣセンサ異常（奥）.
		TYPE_PRTWarningCode_1stTransferATVCCMY,						// WP-29:カラー１次転写ＡＴＶＣ異常.
		TYPE_PRTWarningCode_PCHomeSensorErrorCMY,					// WP-30:カラーＰＣホームセンサ異常.
		TYPE_PRTWarningCode_PCHomeSensorErrorK,						// WP-31:Ｋ色ＰＣホームセンサ異常.
		TYPE_PRTWarningCode_1,										// Engine：副走査ビーム補正エラー.
		TYPE_PRTWarningCode_2,										// Engine：片寄り補正間に合わず.
		TYPE_PRTWarningCode_3,										// Engine：ＡＤＦ傾き補正間に合わず.
		TYPE_PRTWarningCode_4,										// Engine：圧縮メモリオーバーフロー.
		TYPE_PRTWarningCode_7,										// Engine：ＦＮＳアラーム（トレイ／断裁）.
		TYPE_PRTWarningCode_8,										// Engine：ＦＮＳアラーム（ステイプル）.
		TYPE_PRTWarningCode_9,										// Engine：ADF　 NF発生回数.
		TYPE_PRTWarningCode_10,										// Engine：ADF特殊エラー１発生回数.
		TYPE_PRTWarningCode_11,										// Engine：ADF特殊エラー２発生回数.
		TYPE_PRTWarningCode_12,										// Engine：ADF特殊エラー３発生回数.
		TYPE_PRTWarningCode_13,										// Engine：ADF特殊エラー４発生回数.
		TYPE_PRTWarningCode_15,										// Engine：定着アラーム発生回数.
		TYPE_PRTWarningCode_16,										// Engine：トナー無停止発生回数.
		TYPE_PRTWarningCode_17,										// Engine：ＡＧＣリトライ回数.
		TYPE_PRTWarningCode_18,										// Engine：ＡＤＦ傾き補正データ異常回数.
		TYPE_PRTWarningCode_SkewTrouble,							// WP-14:スキュー補正異常.
		TYPE_PRTWarningCode_CHCleaningTrouble1,						// WP-16:帯電チャージャー清掃異常１.
		TYPE_PRTWarningCode_CHCleaningTrouble2,						// WP-18:帯電チャージャー清掃異常２.
		TYPE_PRTWarningCode_CDCExcessTrouble,						// L-01:Ｃ　ＤＣ累積回転距離超過警告.
		TYPE_PRTWarningCode_MDCExcessTrouble,						// L-02:Ｍ　ＤＣ累積回転距離超過警告.
		TYPE_PRTWarningCode_YDCExcessTrouble,						// L-03:Ｙ　ＤＣ累積回転距離超過警告.
		TYPE_PRTWarningCode_KDCExcessTrouble,						// L-04:Ｋ　ＤＣ累積回転距離超過警告.
		TYPE_PRTWarningCode_BeltExcessTrouble,						// L-05:転写ベルト累積回転距離超過警告.
		TYPE_PRTWarningCode_FuserTempTrouble,						// P-32:定着センサ温度検出不良.
		TYPE_PRTWarningCode_KDevCMaterialExcessTrouble,				// L-6：K 現像剤 累積回転距離超過警告.
		TYPE_PRTWarningCode_OzonFilterExcessTrouble,				// L-7：オゾンフィルター累積回転時間超過警告.
		TYPE_PRTWarningCode_PaperDustRemovalCleanerExcessTrouble,	// L-8：紙粉除去クリーナー累積回転時間超過警告.
		TYPE_PRTWarningCode_KDevCUnitExcessTrouble,					// L-9：K 現像ユニット累積回転時間超過警告.
		TYPE_PRTWarningCode_FuserUnitExcessTrouble,					// L-11：定着ユニット累積回転距離超過警告.
		TYPE_PRTWarningCode_TtransferRollerExcessTrouble,			// L-10:転写ローラ累積回転距離超過警告.
		TYPE_PRTWarningCode_DmaxAutoAdjust,							// Dmax補正結果確認用識別定義（Ｐコールしない).
// ADS坪井 → 2011/12/01 【パフォーマンス改善】isWarning対応 Start.
		TYPE_PRTWarningCode_LDTrouble,								// WP-33:LD異常(Zeus).
		TYPE_PRTWarningCode_CDCExcessTroubleForPNL,					// L-01:Ｃ　ＩＵ／ＤＣ／現像ユニット超過警告.（ライフ非表示用）.
		TYPE_PRTWarningCode_MDCExcessTroubleForPNL,					// L-02:Ｍ　ＩＵ／ＤＣ／現像ユニット超過警告.（ライフ非表示用）.
		TYPE_PRTWarningCode_YDCExcessTroubleForPNL,					// L-03:Ｙ　ＩＵ／ＤＣ／現像ユニット超過警告.（ライフ非表示用）.
		TYPE_PRTWarningCode_KDCExcessTroubleForPNL,					// L-04:Ｋ　ＩＵ／ＤＣ／現像ユニット超過警告.（ライフ非表示用）.
		TYPE_PRTWarningCode_BeltExcessTroubleForPNL,				// L-05:転写ベルト超過警告.（ライフ非表示用）.
		TYPE_PRTWarningCode_KDevCMaterialExcessTroubleForPNL,		// L-06：Ｋ 現像剤 超過警告.（ライフ非表示用）.
		TYPE_PRTWarningCode_KDevCUnitExcessTroubleForPNL,			// L-09：現像ユニット超過警告.（ライフ非表示用）.
		TYPE_PRTWarningCode_FuserUnitExcessTroubleForPNL,			// L-11：定着ユニット超過警告.（ライフ非表示用）.
// ADS坪井 → 2011/12/01 【パフォーマンス改善】isWarning対応 End.
		TYPE_PRTWarningCode_TonarFilterExcessTrouble,				// L-12:トナーフィルタ超過警告.
// 2017/12/15 機器構成変更_No.52 AR.1030189 気圧センサ異常追加対応 Start.
		TYPE_PRTWarningCode_AirPressureSensorTrouble,				// P-34：気圧センサ異常警告.
// 2017/12/15 機器構成変更_No.52 AR.1030189 気圧センサ異常追加対応 End.
// 2017/12/15 APF RQ-720 AR.1030074 紙種自動検知対応 Start.
		TYPE_PRTWarningCode_MediaSensorTrouble,						// P-35:メディアセンサー異常警告.
// 2017/12/15 APF RQ-720 AR.1030074 紙種自動検知対応 End.
// 2018/10/15 APF RQ-720 s
		TYPE_PRTWarningCode_MediaSensorTrouble2,					// P-36:メディアセンサー異常2警告.
// 2018/10/15 APF RQ-720 e
// 2019/2/13 APF 機器構成変更_No.31 Start.
		TYPE_PRTWarningCode_PHScanerDirt,							// P-37:PH 光学系汚れ警告.
// 2019/2/13 APF 機器構成変更_No.31 End.
// 2019/2/13 APF 機器構成変更_No.30 Start.
		TYPE_PRTWarningCode_PaperWidthSensor,						// P-38:用紙幅センサー異常.
// 2019/2/13 APF 機器構成変更_No.30 End.
// 2019/2/28 APF 機器構成変更_No.29 Start.
		TYPE_PRTWarningCode_TackingFan,								// P-39:タッキングファン異常.
// 2019/2/28 APF 機器構成変更_No.29 End.
// 2019/5/17 APF 機器構成変更_No.2 Start.
		TYPE_PRTWarningCode_FuserMotorTorque,						// P-40:定着モータトルク異常.
// 2019/5/17 APF 機器構成変更_No.2 End.
// 2020/4/20 APF Eagleシリーズ（Eagle_Z+/Z/X/0/1/2/2.5、EagleBK_Z+/Z/X/0/1/2） : 機器構成変更No.1 温度センサPコール追加 Start.
		TYPE_PRTWarningCode_MachineTempSensorTrouble,				// P-41:機内温度センサ異常.
		TYPE_PRTWarningCode_PHTempSensorTrouble,					// P-42:PH温度センサ異常.
		TYPE_PRTWarningCode_PaperTempSensorTrouble					// P-43:用紙温度センサ異常.
// 2020/4/20 APF Eagleシリーズ（Eagle_Z+/Z/X/0/1/2/2.5、EagleBK_Z+/Z/X/0/1/2） : 機器構成変更No.1 温度センサPコール追加 End.
	};
	
	Bool b_UpdFlg = False;
	for( ushort us_PrtWarningIdx = 0; us_PrtWarningIdx < sizeof( e_PrtWarningTable ) / sizeof( TYPE_PRTWarningCode ) ; us_PrtWarningIdx++){
		b_Val = pc_APIPrinterWarning->isPRTWarning( e_PrtWarningTable[ us_PrtWarningIdx ] );
		if(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt99[ (ushort)e_PrtWarningTable[ us_PrtWarningIdx ] ] != b_Val){
			m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt99[ (ushort)e_PrtWarningTable[ us_PrtWarningIdx ] ] = b_Val;
			if( b_UpdFlg == False ){
				b_UpdFlg = True;
				m_ulCounterPrinterWarningObserver++;	// 発生回数インクリメント.
			}
		}
	}
	if( b_UpdFlg ){
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_99;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt99[ 0 ]);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// プリンタ状態（ＡＰＩ）・CSRC警告情報参照許可/不許可状態.
	b_Val = pc_APIPrinterStatus->getCsrcWarnInfoEnable();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt106 != b_Val){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt106 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_106;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt106);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
//Str HeliosL 機器構成変更No.1 定着手動離間対応 : 谷 : 2014/12/09.
	// 定着離間状態.
	b_Val = pc_APIPrinterStatus->isFuserRetract();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt151 != b_Val){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt151 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_151;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt151);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
//End HeliosL 機器構成変更No.1 定着手動離間対応 : 谷 : 2014/12/09.
	
//Str MinervaSBK KRQ-140 手差しトレイ開閉検知センサの搭載 : 谷 : 2015/06/08.
	// 手差しトレイセット不良状態取得.
	b_Val = pc_APIPrinterStatus->isManualTrayMissSet();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt152 != b_Val){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt152 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_152;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt152);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
//End MinervaSBK KRQ-140 手差しトレイ開閉検知センサの搭載 : 谷 : 2015/06/08.
	
// 2017/09/08 APF 制御構成変更No34 トレイオープン時の警告表示改善 Start.
#if defined(DEF_A4_Cus) && defined(DEF_KRONOSS_Cus)
	TYPE_FeedTrayUnset2Error e_FeedTrayUnset2Error = pc_APIPrinterWarning->getFeedTrayUnset2Error();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.e_DevPrt179 != e_FeedTrayUnset2Error){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.e_DevPrt179 = e_FeedTrayUnset2Error;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_179;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.e_DevPrt179);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
#endif // #if defined(DEF_A4_Cus) && defined(DEF_KRONOSS_Cus).
// 2017/09/08 APF 制御構成変更No34 トレイオープン時の警告表示改善 End.
	
	SST_LOG2(MFPE_LogLevelAll, "End.\n", NULL);
}

//************************************************************************************************************************.
//概要　　：デバイス情報プリンタ取得処理 (警告情報).
//戻り値　：無し.
//備考　　：特に無し.
//作成者　：谷本.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
// 2012. 5.31 [CQ363516] パフォーマンス改善 高周期のポーリング処理で、各条件に該当する場合に変化点検出処理を実行する.
//************************************************************************************************************************.
void SSTC_SystemStatusService::getPrinterDeviceStatus_Opt_Warning()
{
	SST_LOG(MFPE_LogLevelAll);
	SSTS_ChangedInfo stChangedInfo;
	
	APIC_PrinterWarning* pc_APIPrinterWarning = APIC_PrinterWarning::newInstance();
	
	// ERRORが発生しているか(ATTENTION/CAUTIONは含まない).
	Bool b_Val = pc_APIPrinterWarning->isError();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt86 != b_Val){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt86 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_86;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt86);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}

	SST_LOG2(MFPE_LogLevelAll, "End.\n", NULL);
}

//************************************************************************************************************************.
//概要　　：デバイス情報プリンタ取得処理 (UnitLife情報).
//戻り値　：無し.
//備考　　：特に無し.
//作成者　：谷本.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
// 2012. 5.31 [CQ363516] パフォーマンス改善 高周期のポーリング処理で、各条件に該当する場合に変化点検出処理を実行する.
//************************************************************************************************************************.
void SSTC_SystemStatusService::getPrinterDeviceStatus_Opt_UnitLife()
{
	SST_LOG(MFPE_LogLevelAll);
	SSTS_ChangedInfo stChangedInfo;
	
	APIC_PrinterWarning* pc_APIPrinterWarning = APIC_PrinterWarning::newInstance();

	// ２次転写ローラーユニットライフ情報.
	TYPS_UnitLife st_Val = pc_APIPrinterWarning->get2ndTransRollerUnitLife();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt4.b_NewUnit   != st_Val.b_NewUnit  ||
		m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt4.e_UnitLife != st_Val.e_UnitLife ||
		m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt4.e_UnitSet  != st_Val.e_UnitSet ){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt4 = st_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_4;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt4);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// Ｋ色ＤＣ消耗率情報.
	char sc_Val = pc_APIPrinterWarning->getBlackDCConsumptionRate();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt5 != sc_Val){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt5 = sc_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_5;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt5);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// Ｋ色ＤＣライフ情報.
	st_Val = pc_APIPrinterWarning->getBlackDCLife();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt6.b_NewUnit   != st_Val.b_NewUnit  ||
		m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt6.e_UnitLife != st_Val.e_UnitLife ||
		m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt6.e_UnitSet  != st_Val.e_UnitSet ){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt6 = st_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_6;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt6);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// Ｋ色ＤｅｖＣ消耗率情報.
	sc_Val = pc_APIPrinterWarning->getBlackDevCConsumptionRate();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt7 != sc_Val){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt7 = sc_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_7;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt7);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// Ｋ色ＤｅｖＣライフ情報.
	st_Val = pc_APIPrinterWarning->getBlackDevCLife();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt8.b_NewUnit   != st_Val.b_NewUnit  ||
		m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt8.e_UnitLife != st_Val.e_UnitLife ||
		m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt8.e_UnitSet  != st_Val.e_UnitSet ){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt8 = st_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_8;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt8);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// K現像剤消耗率情報.
	sc_Val = pc_APIPrinterWarning->getBlackDevCMaterialConsumptionRate();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt9 != sc_Val){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt9 = sc_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_9;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt9);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// ：K現像剤情報.
	st_Val = pc_APIPrinterWarning->getBlackDevCMaterialLife();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt10.b_NewUnit   != st_Val.b_NewUnit  ||
		m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt10.e_UnitLife != st_Val.e_UnitLife ||
		m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt10.e_UnitSet  != st_Val.e_UnitSet ){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt10 = st_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_10;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt10);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// Ｋ色ＩＵライフ情報.
	st_Val = pc_APIPrinterWarning->getBlackIULife();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt11.b_NewUnit   != st_Val.b_NewUnit  ||
		m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt11.e_UnitLife != st_Val.e_UnitLife ||
		m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt11.e_UnitSet  != st_Val.e_UnitSet ){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt11 = st_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_11;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt11);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
    // Ｃ色ＤＣ消耗率情報.
	sc_Val = pc_APIPrinterWarning->getCyanDCConsumptionRate();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt17 != sc_Val){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt17 = sc_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_17;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt17);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// Ｃ色ＤＣライフ情報.
	st_Val = pc_APIPrinterWarning->getCyanDCLife();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt18.b_NewUnit   != st_Val.b_NewUnit  ||
		m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt18.e_UnitLife != st_Val.e_UnitLife ||
		m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt18.e_UnitSet  != st_Val.e_UnitSet ){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt18 = st_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_18;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt18);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// Ｃ色ＤｅｖＣ消耗率情報.
	sc_Val = pc_APIPrinterWarning->getCyanDevCConsumptionRate();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt19 != sc_Val){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt19 = sc_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_19;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt19);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// Ｃ色ＤｅｖＣライフ情報.
	st_Val = pc_APIPrinterWarning->getCyanDevCLife();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt20.b_NewUnit   != st_Val.b_NewUnit  ||
		m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt20.e_UnitLife != st_Val.e_UnitLife ||
		m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt20.e_UnitSet  != st_Val.e_UnitSet ){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt20 = st_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_20;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt20);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// Ｃ色ＩＵ消耗率情報.
	sc_Val = pc_APIPrinterWarning->getCyanIUConsumptionRate();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt21 != sc_Val){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt21 = sc_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_21;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt21);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// Ｃ色ＩＵライフ情報.
	st_Val = pc_APIPrinterWarning->getCyanIULife();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt22.b_NewUnit   != st_Val.b_NewUnit  ||
		m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt22.e_UnitLife != st_Val.e_UnitLife ||
		m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt22.e_UnitSet  != st_Val.e_UnitSet ){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt22 = st_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_22;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt22);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}

	// 定着ユニット消耗率情報.
	sc_Val = pc_APIPrinterWarning->getFuserUnitConsumptionRate();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt27 != sc_Val){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt27 = sc_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_27;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt27);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// ：定着ユニットライフ情報.
	st_Val = pc_APIPrinterWarning->getFuserUnitLife();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt28.b_NewUnit   != st_Val.b_NewUnit  ||
		m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt28.e_UnitLife != st_Val.e_UnitLife ||
		m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt28.e_UnitSet  != st_Val.e_UnitSet ){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt28 = st_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_28;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt28);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// Ｍ色ＤＣ消耗率情報.
	sc_Val = pc_APIPrinterWarning->getMagentaDCConsumptionRate();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt30 != sc_Val){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt30 = sc_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_30;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt30);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// Ｍ色ＤＣライフ情報.
	st_Val = pc_APIPrinterWarning->getMagentaDCLife();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt31.b_NewUnit   != st_Val.b_NewUnit  ||
		m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt31.e_UnitLife != st_Val.e_UnitLife ||
		m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt31.e_UnitSet  != st_Val.e_UnitSet ){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt31 = st_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_31;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt31);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// Ｍ色ＤｅｖＣ消耗率情報.
	sc_Val = pc_APIPrinterWarning->getMagentaDevCConsumptionRate();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt32 != sc_Val){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt32 = sc_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_32;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt32);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// Ｍ色ＤｅｖＣライフ情報.
	st_Val = pc_APIPrinterWarning->getMagentaDevCLife();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt33.b_NewUnit   != st_Val.b_NewUnit  ||
		m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt33.e_UnitLife != st_Val.e_UnitLife ||
		m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt33.e_UnitSet  != st_Val.e_UnitSet ){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt33 = st_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_33;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt33);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// Ｍ色ＩＵ消耗率情報.
	sc_Val = pc_APIPrinterWarning->getMagentaIUConsumptionRate();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt34 != sc_Val){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt34 = sc_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_34;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt34);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// Ｍ色ＩＵライフ情報.
	st_Val = pc_APIPrinterWarning->getMagentaIULife();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt35.b_NewUnit   != st_Val.b_NewUnit  ||
		m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt35.e_UnitLife != st_Val.e_UnitLife ||
		m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt35.e_UnitSet  != st_Val.e_UnitSet ){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt35 = st_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_35;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt35);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
    // オゾンフィルタ消耗率情報.
	sc_Val = pc_APIPrinterWarning->getOzonFilterConsumptionRate();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt40 != sc_Val){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt40 = sc_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_40;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt40);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// 紙粉除去クリーナー／オゾンフィルタ情報.
	st_Val = pc_APIPrinterWarning->getOzonFilterUnitLife();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt41.b_NewUnit   != st_Val.b_NewUnit  ||
		m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt41.e_UnitLife != st_Val.e_UnitLife ||
		m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt41.e_UnitSet  != st_Val.e_UnitSet ){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt41 = st_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_41;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt41);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// 紙紛除去クリーナ消耗率情報.
	sc_Val = pc_APIPrinterWarning->getPaperDustRemovalCleanerConsumptionRate();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt42 != sc_Val){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt42 = sc_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_42;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt42);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// 噴煙フィルタ情報.
	st_Val = pc_APIPrinterWarning->getSmokeFilterUnitLife();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt47.b_NewUnit   != st_Val.b_NewUnit  ||
		m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt47.e_UnitLife != st_Val.e_UnitLife ||
		m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt47.e_UnitSet  != st_Val.e_UnitSet ){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt47 = st_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_47;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt47);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// 転写クリーナーユニットライフ情報.
	st_Val = pc_APIPrinterWarning->getTransCleanUnitLife();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt52.b_NewUnit   != st_Val.b_NewUnit  ||
		m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt52.e_UnitLife != st_Val.e_UnitLife ||
		m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt52.e_UnitSet  != st_Val.e_UnitSet ){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt52 = st_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_52;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt52);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// 転写ベルトユニット消耗率情報.
	sc_Val = pc_APIPrinterWarning->getTransferBeltUnitConsumptionRate();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt53 != sc_Val){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt53 = sc_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_53;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt53);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// 転写ベルトユニットライフ情報.
	st_Val = pc_APIPrinterWarning->getTransferBeltUnitLife();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt54.b_NewUnit   != st_Val.b_NewUnit  ||
		m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt54.e_UnitLife != st_Val.e_UnitLife ||
		m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt54.e_UnitSet  != st_Val.e_UnitSet ){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt54 = st_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_54;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt54);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// 転写ローラ消耗率情報.
	sc_Val = pc_APIPrinterWarning->getTransRollerConsumptionRate();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt55 != sc_Val){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt55 = sc_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_55;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt55);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// Ｙ色ＤＣ消耗率情報.
	sc_Val = pc_APIPrinterWarning->getYellowDCConsumptionRate();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt56 != sc_Val){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt56 = sc_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_56;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt56);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// Ｙ色ＤＣライフ情報.
	st_Val = pc_APIPrinterWarning->getYellowDCLife();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt57.b_NewUnit   != st_Val.b_NewUnit  ||
		m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt57.e_UnitLife != st_Val.e_UnitLife ||
		m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt57.e_UnitSet  != st_Val.e_UnitSet ){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt57 = st_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_57;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt57);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// Ｙ色ＤｅｖＣ消耗率情報.
	sc_Val = pc_APIPrinterWarning->getYellowDevCConsumptionRate();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt58 != sc_Val){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt58 = sc_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_58;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt58);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// Ｙ色ＤｅｖＣライフ情報.
	st_Val = pc_APIPrinterWarning->getYellowDevCLife();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt59.b_NewUnit   != st_Val.b_NewUnit  ||
		m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt59.e_UnitLife != st_Val.e_UnitLife ||
		m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt59.e_UnitSet  != st_Val.e_UnitSet ){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt59 = st_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_59;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt59);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// Ｙ色ＩＵ消耗率情報.
	sc_Val = pc_APIPrinterWarning->getYellowIUConsumptionRate();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt60 != sc_Val){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt60 = sc_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_60;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt60);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
    
	// Ｙ色ＩＵライフ情報.
	st_Val = pc_APIPrinterWarning->getYellowIULife();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt61.b_NewUnit   != st_Val.b_NewUnit  ||
		m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt61.e_UnitLife != st_Val.e_UnitLife ||
		m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt61.e_UnitSet  != st_Val.e_UnitSet ){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt61 = st_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_61;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt61);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// ＩＵライフ状態確認処理.
	TYPS_SysStatusIULifeStatus	st_IULife;
	APIC_PrinterWarning::newInstance()->chkCMYIULife( st_IULife.b_CIUStatus, st_IULife.b_MIUStatus, st_IULife.b_YIUStatus );
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt75.b_CIUStatus  != st_IULife.b_CIUStatus ||
		m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt75.b_MIUStatus != st_IULife.b_MIUStatus ||
		m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt75.b_YIUStatus != st_IULife.b_YIUStatus ) {
		m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt75 = st_IULife;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_75;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt75);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}

#ifdef DEF_A4_Cus
	// Ｋ色ＩＵ消耗率情報.
	sc_Val = pc_APIPrinterWarning->getBlackIUConsumptionRate();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt159 != sc_Val){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt159 = sc_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_159;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt159);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}

	// Y IU新品解除不良.
	Bool b_Val = pc_APIPrinterWarning->isYellowIUNewTrouble();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt160 != b_Val){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt160	= b_Val;
		stChangedInfo.e_DataID									= TYPE_NTF_DataID_DEV_PRT_160;
		stChangedInfo.p_Value									= &(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt160);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back( stChangedInfo );
		AddChangedInfo( &stChangedInfo );
		// 2021/10/22 APF OP_BTS-35403 end
	}

	// M IU新品解除不良.
	b_Val = pc_APIPrinterWarning->isMagentaIUNewTrouble();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt161 != b_Val){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt161	= b_Val;
		stChangedInfo.e_DataID									= TYPE_NTF_DataID_DEV_PRT_161;
		stChangedInfo.p_Value									= &(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt161);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back( stChangedInfo );
		AddChangedInfo( &stChangedInfo );
		// 2021/10/22 APF OP_BTS-35403 end
	}

	// C IU新品解除不良.
	b_Val = pc_APIPrinterWarning->isCyanIUNewTrouble();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt162 != b_Val){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt162	= b_Val;
		stChangedInfo.e_DataID									= TYPE_NTF_DataID_DEV_PRT_162;
		stChangedInfo.p_Value									= &(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt162);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back( stChangedInfo );
		AddChangedInfo( &stChangedInfo );
		// 2021/10/22 APF OP_BTS-35403 end
	}

	// K IU新品解除不良.
	b_Val = pc_APIPrinterWarning->isBlackIUNewTrouble();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt163 != b_Val){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt163	= b_Val;
		stChangedInfo.e_DataID									= TYPE_NTF_DataID_DEV_PRT_163;
		stChangedInfo.p_Value									= &(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt163);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back( stChangedInfo );
		AddChangedInfo( &stChangedInfo );
		// 2021/10/22 APF OP_BTS-35403 end
	}
#endif	/* DEF_A4_Cus */

//Str 【KronosS WBS_16_26 KronosS警告追加対応 : S.Itoh 2016.10.31】.
#if defined(DEF_A4_Cus) && defined(DEF_KRONOSS_Cus)
	// TCエラーサブコード取得.
	TYPC_FatalCommErrorJamCode c_Error = pc_APIPrinterWarning->getTCErrorSubcode();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.us_DevPrt169 != c_Error.us_Code){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.us_DevPrt169	= c_Error.us_Code;
		stChangedInfo.e_DataID									= TYPE_NTF_DataID_DEV_PRT_169;
		stChangedInfo.p_Value									= &(m_stSysStatInfo.st_PrinterDeviceInfoParam.us_DevPrt169);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back( stChangedInfo );
		AddChangedInfo( &stChangedInfo );
		// 2021/10/22 APF OP_BTS-35403 end
		call_NPAG_setChagneTcError = True;
	}
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.us_DevPrt170 != c_Error.us_YYY){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.us_DevPrt170	= c_Error.us_YYY;
		stChangedInfo.e_DataID									= TYPE_NTF_DataID_DEV_PRT_170;
		stChangedInfo.p_Value									= &(m_stSysStatInfo.st_PrinterDeviceInfoParam.us_DevPrt170);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back( stChangedInfo );
		AddChangedInfo( &stChangedInfo );
		// 2021/10/22 APF OP_BTS-35403 end
		call_NPAG_setChagneTcError = True;
	}
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.us_DevPrt171 != c_Error.us_XX){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.us_DevPrt171	= c_Error.us_XX;
		stChangedInfo.e_DataID									= TYPE_NTF_DataID_DEV_PRT_171;
		stChangedInfo.p_Value									= &(m_stSysStatInfo.st_PrinterDeviceInfoParam.us_DevPrt171);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back( stChangedInfo );
		AddChangedInfo( &stChangedInfo );
		// 2021/10/22 APF OP_BTS-35403 end
		call_NPAG_setChagneTcError = True;
	}
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.uc_DevPrt172 != c_Error.uc_AlphaCharacter){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.uc_DevPrt172	= c_Error.uc_AlphaCharacter;
		stChangedInfo.e_DataID									= TYPE_NTF_DataID_DEV_PRT_172;
		stChangedInfo.p_Value									= &(m_stSysStatInfo.st_PrinterDeviceInfoParam.uc_DevPrt172);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back( stChangedInfo );
		AddChangedInfo( &stChangedInfo );
		// 2021/10/22 APF OP_BTS-35403 end
		call_NPAG_setChagneTcError = True;
	}

	if( True == call_NPAG_setChagneTcError ){
		// for sys:NPA.
		call_NPAG_setChagneTcError = False;
		NPAG_setChagneTcError( c_Error );
	}

	// IUエラーサブコード取得.
	c_Error = pc_APIPrinterWarning->getIUErrorSubcode();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.us_DevPrt173 != c_Error.us_Code){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.us_DevPrt173	= c_Error.us_Code;
		stChangedInfo.e_DataID									= TYPE_NTF_DataID_DEV_PRT_173;
		stChangedInfo.p_Value									= &(m_stSysStatInfo.st_PrinterDeviceInfoParam.us_DevPrt173);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back( stChangedInfo );
		AddChangedInfo( &stChangedInfo );
		// 2021/10/22 APF OP_BTS-35403 end
		call_NPAG_setChagneIuError = True;
	}
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.us_DevPrt174 != c_Error.us_YYY){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.us_DevPrt174	= c_Error.us_YYY;
		stChangedInfo.e_DataID									= TYPE_NTF_DataID_DEV_PRT_174;
		stChangedInfo.p_Value									= &(m_stSysStatInfo.st_PrinterDeviceInfoParam.us_DevPrt174);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back( stChangedInfo );
		AddChangedInfo( &stChangedInfo );
		// 2021/10/22 APF OP_BTS-35403 end
		call_NPAG_setChagneIuError = True;
	}
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.us_DevPrt175 != c_Error.us_XX){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.us_DevPrt175	= c_Error.us_XX;
		stChangedInfo.e_DataID									= TYPE_NTF_DataID_DEV_PRT_175;
		stChangedInfo.p_Value									= &(m_stSysStatInfo.st_PrinterDeviceInfoParam.us_DevPrt175);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back( stChangedInfo );
		AddChangedInfo( &stChangedInfo );
		// 2021/10/22 APF OP_BTS-35403 end
		call_NPAG_setChagneIuError = True;
	}
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.uc_DevPrt176 != c_Error.uc_AlphaCharacter){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.uc_DevPrt176	= c_Error.uc_AlphaCharacter;
		stChangedInfo.e_DataID									= TYPE_NTF_DataID_DEV_PRT_176;
		stChangedInfo.p_Value									= &(m_stSysStatInfo.st_PrinterDeviceInfoParam.uc_DevPrt176);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back( stChangedInfo );
		AddChangedInfo( &stChangedInfo );
		// 2021/10/22 APF OP_BTS-35403 end
		call_NPAG_setChagneIuError = True;
	}

	if( True == call_NPAG_setChagneIuError ){
		// for sys:NPA.
		call_NPAG_setChagneIuError = False;
		NPAG_setChagneIuError( c_Error );
	}

#endif    // #if defined(DEF_A4_Cus) && defined(DEF_KRONOSS_Cus)
//End 【KronosS WBS_16_26 KronosS警告追加対応 : S.Itoh 2016.10.31】.


	SST_LOG2(MFPE_LogLevelAll, "End.\n", NULL);
}

//************************************************************************************************************************.
//概要　　：デバイス情報プリンタ取得処理 (Toner情報).
//戻り値　：無し.
//備考　　：特に無し.
//作成者　：谷本.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
// 2012. 5.31 [CQ363516] パフォーマンス改善 高周期のポーリング処理で、各条件に該当する場合に変化点検出処理を実行する.
//************************************************************************************************************************.
void SSTC_SystemStatusService::getPrinterDeviceStatus_Opt_Toner()
{
	SST_LOG(MFPE_LogLevelAll);
	SSTS_ChangedInfo stChangedInfo;
	
	APIC_PrinterWarning* pc_APIPrinterWarning = APIC_PrinterWarning::newInstance();
	
	// Ｋトナー状態.
	char sc_Val = pc_APIPrinterWarning->getBlackTonerStatus();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt12 != sc_Val){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt12 = sc_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_12;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt12);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// Ｃトナー残量情報.
	sc_Val = pc_APIPrinterWarning->getCTonerQuantity();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt16 != sc_Val){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt16 = sc_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_16;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt16);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// Ｃトナー状態.
	sc_Val = pc_APIPrinterWarning->getCyanTonerStatus();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt23 != sc_Val){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt23 = sc_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_23;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt23);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// Ｋトナー残量情報.
	sc_Val = pc_APIPrinterWarning->getKTonerQuantity();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt29 != sc_Val){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt29 = sc_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_29;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt29);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// Ｍトナー状態.
	sc_Val = pc_APIPrinterWarning->getMagentaTonerStatus();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt36 != sc_Val){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt36 = sc_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_36;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt36);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// Ｍトナー残量情報.
	sc_Val = pc_APIPrinterWarning->getMTonerQuantity();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt39 != sc_Val){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt39 = sc_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_39;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt39);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// トナー回収ボトル状態.
	sc_Val = pc_APIPrinterWarning->getTonerBottleStatus();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt49 != sc_Val){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt49 = sc_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_49;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt49);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// トナーフィルタ消耗率情報.
	sc_Val = pc_APIPrinterWarning->getTonerFilterConsumptionRate();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt51 != sc_Val){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt51 = sc_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_51;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt51);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// Ｙトナー状態.
	sc_Val = pc_APIPrinterWarning->getYellowTonerStatus();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt62 != sc_Val){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt62 = sc_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_62;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt62);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// Ｙトナー残量情報.
	sc_Val = pc_APIPrinterWarning->getYTonerQuantity();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt64 != sc_Val){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt64 = sc_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_64;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt64);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// ＣＭＹトナーエンプティ状態.
	TYPS_SysStatusScanStart	st_Empty;
	APIC_PrinterWarning::newInstance()->chkCMYTonerEmpty( st_Empty.b_CTCStatus, st_Empty.b_MTCStatus, st_Empty.b_YTCStatus );
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt76.b_CTCStatus  != st_Empty.b_CTCStatus ||
		m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt76.b_MTCStatus != st_Empty.b_MTCStatus ||
		m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt76.b_YTCStatus != st_Empty.b_YTCStatus ){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt76 = st_Empty;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_76;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt76);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// Ｋトナー誤装着.
	Bool b_Val = pc_APIPrinterWarning->isBlackTonerMissset();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt79 != b_Val){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt79 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_79;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt79);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// Ｋトナー未装着.
	b_Val = pc_APIPrinterWarning->isBlackTonerUnset();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt80 != b_Val){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt80 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_80;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt80);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// Ｃトナー誤装着.
	b_Val = pc_APIPrinterWarning->isCyanTonerMissset();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt81 != b_Val){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt81 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_81;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt81);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// Ｃトナー未装着.
	b_Val = pc_APIPrinterWarning->isCyanTonerUnset();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt82 != b_Val){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt82 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_82;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt82);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// Ｍトナー誤装着.
	b_Val = pc_APIPrinterWarning->isMagentaTonerMissset();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt90 != b_Val){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt90 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_90;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt90);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// Ｍトナー未装着.
	b_Val = pc_APIPrinterWarning->isMagentaTonerUnset();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt91 != b_Val){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt91 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_91;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt91);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// Ｙトナー誤装着.
	b_Val = pc_APIPrinterWarning->isYellowTonerMissset();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt103 != b_Val){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt103 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_103;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt103);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// Ｙトナー未装着.
	b_Val = pc_APIPrinterWarning->isYellowTonerUnset();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt104 != b_Val){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt104 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_104;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt104);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
#ifdef DEF_A3A4COMM
//Str【A3A4共通化FUM  ユニット装着情報１共通化: M.Saitou : 2014.12.22】.
#if defined(DEF_A4_Cus)
	// Ｍトナー新品解除不良.
	b_Val = pc_APIPrinterWarning->isMagentaTonerNewTrouble();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt154 != b_Val){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt154 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_154;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt154);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}

	// Ｃトナー新品解除不良.
	b_Val = pc_APIPrinterWarning->isCyanTonerNewTrouble();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt155 != b_Val){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt155 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_155;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt155);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}

	// Ｋトナー新品解除不良.
	b_Val = pc_APIPrinterWarning->isBlackTonerNewTrouble();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt157 != b_Val){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt157 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_157;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt157);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}

	// Ｙトナー新品解除不良.
	b_Val = pc_APIPrinterWarning->isYellowTonerNewTrouble();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt158 != b_Val){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt158 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_158;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt158);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
#else
	// Ｙトナー新品解除不良.
	b_Val = pc_APIPrinterWarning->isYellowTonerNewTrouble();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt152 != b_Val){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt152 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_152;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt152);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}

	// Ｍトナー新品解除不良.
	b_Val = pc_APIPrinterWarning->isMagentaTonerNewTrouble();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt153 != b_Val){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt153 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_153;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt153);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}

	// Ｃトナー新品解除不良.
	b_Val = pc_APIPrinterWarning->isCyanTonerNewTrouble();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt154 != b_Val){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt154 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_154;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt154);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}

	// Ｋトナー新品解除不良.
	b_Val = pc_APIPrinterWarning->isBlackTonerNewTrouble();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt155 != b_Val){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt155 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_155;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt155);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
#endif //defined(DEF_A4_Cus)
//End【A3A4共通化FUM  ユニット装着情報１共通化: M.Saitou : 2014.12.22】.
#endif // DEF_A3A4COMM
	SST_LOG2(MFPE_LogLevelAll, "End.\n", NULL);
}

//************************************************************************************************************************.
//概要　　：デバイス情報プリンタ取得処理 (Staple情報).
//戻り値　：無し.
//備考　　：特に無し.
//作成者　：谷本.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
// 2012. 5.31 [CQ363516] パフォーマンス改善 高周期のポーリング処理で、各条件に該当する場合に変化点検出処理を実行する.
//************************************************************************************************************************.
void SSTC_SystemStatusService::getPrinterDeviceStatus_Opt_Staple()
{
	SST_LOG(MFPE_LogLevelAll);
	SSTS_ChangedInfo stChangedInfo;
	
	APIC_PrinterWarning* pc_APIPrinterWarning = APIC_PrinterWarning::newInstance();

	// 中綴じステープラ動作状態.
	char sc_Val = pc_APIPrinterWarning->getCenterStaplerStatus();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt14 != sc_Val){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt14 = sc_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_14;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt14);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// 平綴じステープラ動作状態.
	sc_Val = pc_APIPrinterWarning->getStaplerStatus();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt48 != sc_Val){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt48 = sc_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_48;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt48);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// ステープルエンプティ2.
	Bool b_Val = pc_APIPrinterWarning->is2ndStapleEmpty();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt77 != b_Val){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt77 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_77;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt77);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// ステープルエンプティ3.
	b_Val = pc_APIPrinterWarning->is3rdStapleEmpty();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt78 != b_Val){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt78 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_78;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt78);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// ステープルエンプティ1.
	b_Val = pc_APIPrinterWarning->isStapleEmpty();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt102 != b_Val){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt102 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_102;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt102);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	SST_LOG2(MFPE_LogLevelAll, "End.\n", NULL);
}

//************************************************************************************************************************.
//概要　　：デバイス情報プリンタ取得処理 (ElevateTray/PunchScraps etc情報).
//戻り値　：無し.
//備考　　：特に無し.
//作成者　：谷本.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
// 2012. 5.31 [CQ363516] パフォーマンス改善 高周期のポーリング処理で、各条件に該当する場合に変化点検出処理を実行する.
//************************************************************************************************************************.
void SSTC_SystemStatusService::getPrinterDeviceStatus_Opt_Other()
{
	SST_LOG(MFPE_LogLevelAll);
	SSTS_ChangedInfo stChangedInfo;
	
	APIC_PrinterWarning* pc_APIPrinterWarning = APIC_PrinterWarning::newInstance();

	// エレベートトレイ上昇未完.
	Bool b_Val = pc_APIPrinterWarning->isElevateTrayNotLiftUp();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt83 != b_Val){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt83 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_83;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt83);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// LCT装着状態.
	b_Val = APIC_PrinterWarning::newInstance()->isLCTUnFixed();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt89 != b_Val){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt89 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_89;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt89);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// パンチくずフル.
	b_Val = pc_APIPrinterWarning->isPunchDustFull();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt100 != b_Val){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt100 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_100;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt100);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// ステープル屑箱セット.
	b_Val = pc_APIPrinterWarning->isStapleDustBoxUnset();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt101 != b_Val){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt101 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_101;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt101);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// Z折りユニットパンチくずフル.
	b_Val = pc_APIPrinterWarning->isZUnitPunchDustFull();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt105 != b_Val){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt105 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_105;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt105);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	SST_LOG2(MFPE_LogLevelAll, "End.\n", NULL);
}

//************************************************************************************************************************.
//概要　　：デバイス情報プリンタ取得処理 (Jam情報).
//戻り値　：無し.
//備考　　：特に無し.
//作成者　：谷本.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
// 2012. 5.31 [CQ363516] パフォーマンス改善 高周期のポーリング処理で、各条件に該当する場合に変化点検出処理を実行する.
//************************************************************************************************************************.
void SSTC_SystemStatusService::getPrinterDeviceStatus_Opt_Jam()
{
	SST_LOG(MFPE_LogLevelAll);
	SSTS_ChangedInfo stChangedInfo;
	
	APIC_PrinterWarning* pc_APIPrinterWarning = APIC_PrinterWarning::newInstance();

	// 周辺機器コミュニケーションエラーコード.
	TYPC_FatalCommErrorJamCode c_Error = pc_APIPrinterWarning->getCommError();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.us_DevPrt15 != c_Error.us_Code){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.us_DevPrt15 = c_Error.us_Code;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_15;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.us_DevPrt15);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}

//	2017/03/17 IT5_4.0 1017494 Sky嵯峨山 ST.
	char sc_Val = TYPE_FatalLevel_Non;
	Bool b_BootSelfCheckMode = APIC_SystemInfo::newInstance()->isBootSelfCheckMode();
	Bool b_AutoSelfCheckOn = APIC_SystemStatus::newInstance()->getAutoSelfCheckOn();
//	Bootシーケンス診断または、自動自己診断時には、LocalUIへ通知しないように検知を抑制.
	if ( ( False == b_BootSelfCheckMode ) && ( False == b_AutoSelfCheckOn ) ) {
		// プリンタ警告（ＦＡＴＡＬコード）.
		c_Error = pc_APIPrinterWarning->getFatal();
		if(m_stSysStatInfo.st_PrinterDeviceInfoParam.us_DevPrt25 != c_Error.us_Code){
			m_stSysStatInfo.st_PrinterDeviceInfoParam.us_DevPrt25 = c_Error.us_Code;
			stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_25;
			stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.us_DevPrt25);
			// 2021/10/22 APF OP_BTS-35403 start
			//m_vChangedInfo.push_back(stChangedInfo);
			AddChangedInfo(&stChangedInfo);
			// 2021/10/22 APF OP_BTS-35403 end
#if defined(DEF_A4_Cus) && defined(DEF_KRONOSS_Cus)
			call_NPAG_setChagnePrinterFatal = True;
#endif // defined(DEF_A4_Cus) && defined(DEF_KRONOSS_Cus).
		}
//Str 【KronosS WBS_16 LXK トラブルコード追加 課題1005716 S.Itoh 2016.10.13】.
#if defined(DEF_A4_Cus) && defined(DEF_KRONOSS_Cus)
		// FatalLxkエンジンYYYコード.
		if(m_stSysStatInfo.st_PrinterDeviceInfoParam.us_DevPrt167 != c_Error.us_YYY){
			m_stSysStatInfo.st_PrinterDeviceInfoParam.us_DevPrt167 = c_Error.us_YYY;
			stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_167;
			stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.us_DevPrt167);
			// 2021/10/22 APF OP_BTS-35403 start
			//m_vChangedInfo.push_back(stChangedInfo);
			AddChangedInfo(&stChangedInfo);
			// 2021/10/22 APF OP_BTS-35403 end
			call_NPAG_setChagnePrinterFatal = True;
		}
		// FatalLxkエンジンXXコード.
		if(m_stSysStatInfo.st_PrinterDeviceInfoParam.us_DevPrt168 != c_Error.us_XX){
			m_stSysStatInfo.st_PrinterDeviceInfoParam.us_DevPrt168 = c_Error.us_XX;
			stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_168;
			stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.us_DevPrt168);
			// 2021/10/22 APF OP_BTS-35403 start
			//m_vChangedInfo.push_back(stChangedInfo);
			AddChangedInfo(&stChangedInfo);
			// 2021/10/22 APF OP_BTS-35403 end
			call_NPAG_setChagnePrinterFatal = True;
		}

//Str 【KronosS AR1016161_JAM/FATAL_AlphaCharacter追加 : S.Itoh 2017.03.03】.
		// FatalLxkエンジンAlphaCharacter.
		if(m_stSysStatInfo.st_PrinterDeviceInfoParam.uc_DevPrt178 != c_Error.uc_AlphaCharacter){
			m_stSysStatInfo.st_PrinterDeviceInfoParam.uc_DevPrt178 = c_Error.uc_AlphaCharacter;
			stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_178;
			stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.uc_DevPrt178);
			// 2021/10/22 APF OP_BTS-35403 start
			//m_vChangedInfo.push_back(stChangedInfo);
			AddChangedInfo(&stChangedInfo);
			// 2021/10/22 APF OP_BTS-35403 end
			call_NPAG_setChagnePrinterFatal = True;
		}
//End 【KronosS AR1016161_JAM/FATAL_AlphaCharacter追加 : S.Itoh 2017.03.03】.

#endif    // #if defined(DEF_A4_Cus) && defined(DEF_KRONOSS_Cus)
//End 【KronosS WBS_16 LXK トラブルコード追加 課題1005716 S.Itoh 2016.10.13】.
		
		// ＦＡＴＡＬ Ｌｅｖｅｌ確認.
		sc_Val = pc_APIPrinterWarning->getFatalLevel();
		if(m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt26 != sc_Val){
			m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt26 = sc_Val;
			stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_26;
			stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt26);
			// 2021/10/22 APF OP_BTS-35403 start
			//m_vChangedInfo.push_back(stChangedInfo);
			AddChangedInfo(&stChangedInfo);
			// 2021/10/22 APF OP_BTS-35403 end
#if defined(DEF_A4_Cus) && defined(DEF_KRONOSS_Cus)
			call_NPAG_setChagnePrinterFatal = True;
#endif // defined(DEF_A4_Cus) && defined(DEF_KRONOSS_Cus).
		}
#if defined(DEF_A4_Cus) && defined(DEF_KRONOSS_Cus)
		if( True == call_NPAG_setChagnePrinterFatal ){
			// for sys:NPA.
			call_NPAG_setChagnePrinterFatal = False;
			NPAG_setChagnePrinterFatal( c_Error , sc_Val );
		}
#endif // defined(DEF_A4_Cus) && defined(DEF_KRONOSS_Cus).
	}
//	2017/03/17 IT5_4.0 1017494 Sky嵯峨山 ED.
// 2016/09/02 APF 制御構成変更No51 制御トラブル検出時のHW自己診断自動化 s.
	// 自動自己診断実施判定.
	TYPS_FatalInfoForAutoSelfCheck st_FatalInfoForAutoSelfCheck = {
		TYPE_FatalOccurredKindPrint,
		m_stSysStatInfo.st_PrinterDeviceInfoParam.us_DevPrt25,
		m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt26
	};
	Bool b_AutoSelfCheck = APIC_SystemStatus::newInstance()->judgeAutoSelfCheck( &st_FatalInfoForAutoSelfCheck );
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt164 != b_AutoSelfCheck){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt164 = b_AutoSelfCheck;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_164;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt164);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
// 2016/09/02 APF 制御構成変更No51 制御トラブル検出時のHW自己診断自動化 e.

	// メディアエラー.
	sc_Val = pc_APIPrinterWarning->getMediaError();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt38 != sc_Val){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt38 = sc_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_38;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt38);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// ジャムコード.
	c_Error = pc_APIPrinterWarning->getPrinterJam();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.us_DevPrt43 != c_Error.us_Code){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.us_DevPrt43 = c_Error.us_Code;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_43;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.us_DevPrt43);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
#if defined(DEF_A4_Cus) && defined(DEF_KRONOSS_Cus)
		call_NPAG_setChagnePrinterJam = True;
#endif // defined(DEF_A4_Cus) && defined(DEF_KRONOSS_Cus).
	}
//Str 【KronosS WBS_16 LXK トラブルコード追加 課題1005716 S.Itoh 2016.10.13】.
#if defined(DEF_A4_Cus) && defined(DEF_KRONOSS_Cus)
	// JAMLxkエンジンYYYコード.
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.us_DevPrt165 != c_Error.us_YYY){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.us_DevPrt165 = c_Error.us_YYY;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_165;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.us_DevPrt165);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
		call_NPAG_setChagnePrinterJam = True;
	}
	// JAMLxkエンジンXXコード.
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.us_DevPrt166 != c_Error.us_XX){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.us_DevPrt166 = c_Error.us_XX;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_166;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.us_DevPrt166);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
		call_NPAG_setChagnePrinterJam = True;
	}

//Str 【KronosS AR1016161_JAM/FATAL_AlphaCharacter追加 : S.Itoh 2017.03.03】.
	// JamLxkエンジンAlphaCharacter.
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.uc_DevPrt177 != c_Error.uc_AlphaCharacter){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.uc_DevPrt177 = c_Error.uc_AlphaCharacter;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_177;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.uc_DevPrt177);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
		call_NPAG_setChagnePrinterFatal = True;
	}
//End 【KronosS AR1016161_JAM/FATAL_AlphaCharacter追加 : S.Itoh 2017.03.03】.

#endif    // #if defined(DEF_A4_Cus) && defined(DEF_KRONOSS_Cus)
//End 【KronosS WBS_16 LXK トラブルコード追加 課題1005716 S.Itoh 2016.10.13】.	

#if defined(DEF_A4_Cus) && defined(DEF_KRONOSS_Cus)
	if( True == call_NPAG_setChagnePrinterJam ){
		// for sys:NPA.
		call_NPAG_setChagnePrinterJam = False;
		NPAG_setChagnePrinterJam( c_Error );
	}
#endif // defined(DEF_A4_Cus) && defined(DEF_KRONOSS_Cus).
	
	// サイズエラー.
	sc_Val = pc_APIPrinterWarning->getSizeError();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt44 != sc_Val){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt44 = sc_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_44;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt44);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// ＨＤＤファン異常が発生しているか.
	Bool b_Val = pc_APIPrinterWarning->isHDDFanTrouble();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt87 != b_Val){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt87 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_87;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt87);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// ジャム残留用紙が存在するか.
	static TYPE_PrinterJamPos e_JamPosTable[] = {
		TYPE_PrinterJamPosFeedTray1,								//	１段目給紙部.
		TYPE_PrinterJamPosFeedTray2,								//	２段目給紙部.
		TYPE_PrinterJamPosFeedTray3,								//	３段目給紙部.
		TYPE_PrinterJamPosFeedTray4,								//	４段目給紙部.
// DenebMLK:機器構成変更_No.7_5段増設プリント機能追加対応 start.
		TYPE_PrinterJamPosFeedTray5,								//	５段目給紙部.
// DenebMLK:機器構成変更_No.7_5段増設プリント機能追加対応 end.
		TYPE_PrinterJamPosFeedTrayManual,							//	手差し給紙部.
		TYPE_PrinterJamPosFeedTrayDuplex,							//	再給紙部.
		TYPE_PrinterJamPosVTransfer,								//	縦搬送部.
		TYPE_PrinterJamPos2ndTrans,									//	２次転写部.
		TYPE_PrinterJamPosFuser,									//	定着部.
		TYPE_PrinterJamPosPaperEject,								//	ペーパー排紙部.
		TYPE_PrinterJamPosDuplexTrans,								//	両面搬送部.
		TYPE_PrinterJamPosFnStaple,									//	ステープル部.
		TYPE_PrinterJamPosFnHTransReverse,							//	水平搬送部反転部（１０ＭＢ・ＦＮ－Ｘ）.
		TYPE_PrinterJamPosFnOp,										//	排紙オプション部.
		TYPE_PrinterJamPosFnFolder,									//	紙折り部（NTSでも使用）.
		TYPE_PrinterJamPosFnHTrans,									//	水平搬送部.
		TYPE_PrinterJamPosFn3rdTray,								//	第３トレイ.
		TYPE_PrinterJamPosFnStapleSafety,							//	ステープル安全機構部.
		TYPE_PrinterJamPosFnPunch,									//	パンチ部.
		TYPE_PrinterJamPosFnTransfer,								//	フィニッシャ搬送部.
		TYPE_PrinterJamPosFnEntrance,								//	入り口部.
		TYPE_PrinterJamPosFnSaddleEject,							//	サドル排出部.
		TYPE_PrinterJamPosFnMailBin,								//	メールビン部.
		TYPE_PrinterJamPosLCTFeed,									//	ＬＣＴ給紙部.
		TYPE_PrinterJamPosLCTTrans,									//	ＬＣＴ搬送部.
		TYPE_PrinterJamPos34FeedHTrans,								//	３／４段目水平搬送部.
		TYPE_PrinterJamPos34FeedMRoller,							//	３／４段目中間ローラ部.
		TYPE_PrinterJamPosFnOPTray1,								//	第1トレイ排出部（FNS）.
		TYPE_PrinterJamPosFnPIUnderTrans,							//	PI下段の搬送部残留紙状態. 
		TYPE_PrinterJamPosFnPIUpperTrans,							//	PI上段の搬送部残留紙状態.
		TYPE_PrinterJamPosFnZUEnt,									//	ZUの入口部残留紙状態.
		TYPE_PrinterJamPosFnZUTrans,								//	ZUの搬送部残留紙状態.
		TYPE_PrinterJamPosFnZUExit,									//	ZUの排出部残留紙状態.
		TYPE_PrinterJamPos2ndProcessTray,							//	 第2処理トレイ部(フィニッシャ).
		TYPE_PrinterJamPosFnHTransEnt,								//	 水平搬送入り口.
		TYPE_PrinterJamPosFnTransfer2,								//	 搬送部2（フィニッシャ).
		TYPE_PrinterJamPosLCCFeed,									//	 移載ＬＣＴ給紙部.
// comment 2015/05/21 OPS1-Sky西村 MinervaSBK 機器構成変更No30 JAM処理性改善 パージ機能対応.
//		TYPE_PrinterJamPosLCCTrans									//	 移載ＬＣＴ搬送部.
		TYPE_PrinterJamPosLCCTrans,									//	 移載ＬＣＴ搬送部.
		TYPE_PrinterJamPosRevEject,									//	 反転排出口.
		TYPE_PrinterJamPosSwitchBack,								//	 スイッチバック部.
	};
	Bool b_UpdFlg = False;
	for( ushort us_JamPosIdx = 0; us_JamPosIdx < sizeof( e_JamPosTable ) / sizeof( TYPE_PrinterJamPos ) ; us_JamPosIdx++) {
		b_Val = pc_APIPrinterWarning->isJamPaperExist( e_JamPosTable[ us_JamPosIdx ] );
		if( m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt88[ (ushort)e_JamPosTable[ us_JamPosIdx ] ] != b_Val ){
			m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt88[ (ushort)e_JamPosTable[ us_JamPosIdx ] ] = b_Val;
			if( b_UpdFlg == False ){
				b_UpdFlg = True;
			}
		}
	}
	if( b_UpdFlg ){
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_88;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt88[ 0 ]);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}

	// 周辺機器コミュニケーションエラーが発生しているか.
	b_Val = pc_APIPrinterWarning->isOccurCommError();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt92 != b_Val){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt92 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_92;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt92);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
//	2017/03/17 IT5_4.0 1017494 Sky嵯峨山 ST.
//	Bootシーケンス診断または、自動自己診断時には、LocalUIへ通知しないように検知を抑制.
	if ( ( False == b_BootSelfCheckMode ) && ( False == b_AutoSelfCheckOn ) ) {
		// ＦＡＴＡＬが発生しているか.
		b_Val = pc_APIPrinterWarning->isOccurFatal();
		if(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt93 != b_Val){
			m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt93 = b_Val;
			stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_93;
			stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt93);
			// 2021/10/22 APF OP_BTS-35403 start
			//m_vChangedInfo.push_back(stChangedInfo);
			AddChangedInfo(&stChangedInfo);
			// 2021/10/22 APF OP_BTS-35403 end
		}
	}
//	2017/03/17 IT5_4.0 1017494 Sky嵯峨山 ED.	
	// ジャムが発生しているか.
	b_Val = pc_APIPrinterWarning->isOccurPrinterJam();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt94 != b_Val){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt94 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_94;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt94);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// コピー除去要求.
	b_Val = APIC_PrinterWarning::newInstance()->isPaperRemove();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt95 != b_Val){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt95 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_95;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt95);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// ＰＩプレ給紙用紙残留検知処理.
	b_Val = APIC_PrinterWarning::newInstance()->isPIPreFeedRemainPaper();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt96 != b_Val){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt96 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_96;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt96);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	SST_LOG2(MFPE_LogLevelAll, "End.\n", NULL);
}

//************************************************************************************************************************.
//概要　　：デバイス情報プリンタ取得処理 (DoorOpen情報).
//戻り値　：無し.
//備考　　：特に無し.
//作成者　：谷本.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
// 2012. 5.31 [CQ363516] パフォーマンス改善 高周期のポーリング処理で、各条件に該当する場合に変化点検出処理を実行する.
//************************************************************************************************************************.
void SSTC_SystemStatusService::getPrinterDeviceStatus_Opt_DoorOpen()
{
	SST_LOG(MFPE_LogLevelAll);
	SSTS_ChangedInfo stChangedInfo;
	
	APIC_PrinterWarning* pc_APIPrinterWarning = APIC_PrinterWarning::newInstance();

	// 何れかのドアが開いているか .
	Bool b_Val = pc_APIPrinterWarning->isPrinterDoorOpen();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt97 != b_Val){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt97 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_97;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt97);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// ドアが開いているか（個々のドア）.
	static TYPE_PrinterDoor e_PrinterDoorTable[] = {
		TYPE_PrinterDoorFront,										// 本体・前扉.
		TYPE_PrinterDoorRight,										// 本体・右扉.
		TYPE_PrinterDoorLeft,										// 本体・左扉.
		TYPE_PrinterDoor2ndFeedJamCover,							// 本体・２段目給紙キャビネット・ジャム処理カバー.
		TYPE_PrinterDoor3rdFeedJamCover,							// 本体・３段目給紙キャビネット・ジャム処理カバー.
		TYPE_PrinterDoor4thFeedJamCover,							// 本体・４段目給紙キャビネット・ジャム処理カバー.
		TYPE_PrinterDoorLCCFeedJamCover,							// 本体・ＬＣＣ給紙キャビネット・ジャム処理カバー.
		TYPE_PrinterDoorEjectJamCover,								// 本体・排出扉.
		TYPE_PrinterDoorDuplex,										// 本体・両面ユニット.
		TYPE_PrinterDoorFnOpen,										// フィニッシャ・フィニッシャ（フィニッシャを本体から離した状態）.
		TYPE_PrinterDoorFnUpper,									// フィニッシャ・上カバー（各フィニッシャ共通定義：機種依存あり）.
		TYPE_PrinterDoorFnFront,									// フィニッシャ・フィニッシャ前カバー（フィニッシャの前面を覆うカバー）.
		TYPE_PrinterDoorFnTransmitLevel,							// フィニッシャ・水平搬送部.
		TYPE_PrinterDoorFn3rdTray,									// フィニッシャ・第３トレイ.
		TYPE_PrinterDoorFnMailCover,								// メールカバー.
		TYPE_PrinterDoorFnMiddleGuide,								// 中間ガイドセット不良.
		TYPE_PrinterDoorFnSaddle,									// サドルトレイセット不良.
		TYPE_PrinterDoorFnEjectShutter,								// 排出シャッタセット不良.
// 2012/7/15 CQ367196 ADD-S.
//		TYPE_PrinterDoorFrontLap,									// 本体・前扉(右/左共通).
//		TYPE_PrinterDoorSideLap,									// 本体・右扉(本体右側板).
//		TYPE_PrinterDoorHorizontalFeedLap,							// 垂直搬送扉.
//		TYPE_PrinterDoorFnOpenLap,									// フィニッシャ(各種共通).
//		TYPE_PrinterDoorSideLCTLap,									// LCTサイドドア.
//		TYPE_PrinterDoorPSLap,										// Stella固有の為　概要未記入.
//		TYPE_PrinterDoorCFLap,										// カバーシートフィーダドア(上部).
		TYPE_PrinterDoorPZLap,										// パンチ、Z折ドア(Z折ドア).
//		TYPE_PrinterDoorTonerLap,									// 本体・トナードア(トナー補給扉).
		TYPE_PrinterDoorPKPunchScraps,								// PK用パンチ屑ボックス.
		TYPE_PrinterDoorZUPunchScraps,								// ZU用パンチ屑ボックス.
		TYPE_PrinterDoorFeedJamCover,								// 本体・給紙キャビネット・ジャム処理カバー.
		TYPE_PrinterDoorManualTrayUnfixed,							// 手差し未装着不良.
		TYPE_PrinterDoorTonerCartridge,								// トナー補給ドア.
		TYPE_PrinterDoorLCCHTransUnfixed,							// ＬＣＣ水平搬送装部未装着.
		TYPE_PrinterDoorZUnitNonPower,								// Ｚ折りユニット電源抜け(ZU).
		TYPE_PrinterDoorStapleDustBoxUnset							// ステープル屑箱未セット.
// 2012/7/15 CQ367196 ADD-E.
#ifdef DEF_A3A4COMM
		,TYPE_PrinterDoorStapleCover								// ステープルカバー(VenusFNS).
#if defined(DEF_A4_Cus) && defined(DEF_KRONOSS_Cus)
		,TYPE_PrinterDoorTrapDoor									// KronosS Finisher（トラップドア）.
#endif    // #if defined(DEF_A4_Cus) && defined(DEF_KRONOSS_Cus).
#else
		,TYPE_PrinterDoor5thFeedJamCover							// 本体・５段目給紙キャビネット・ジャム処理カバー.		//Add【AR1058643対応：doi：2018.11.29】.
#endif //ifdef DEF_A3A4COMM
	};
	
	Bool b_UpdFlg = False;
	for( ushort us_PrinterDoorIdx = 0; us_PrinterDoorIdx < sizeof( e_PrinterDoorTable ) / sizeof( TYPE_PrinterDoor ) ; us_PrinterDoorIdx++){
		b_Val = pc_APIPrinterWarning->isPrinterDoorOpen( e_PrinterDoorTable[ us_PrinterDoorIdx ] );
		if( m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt98[ (ushort)e_PrinterDoorTable[ us_PrinterDoorIdx ] ] != b_Val ){
			m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt98[ (ushort)e_PrinterDoorTable[ us_PrinterDoorIdx ] ] = b_Val;
			if( b_UpdFlg == False ){
				b_UpdFlg = True;
			}
		}
	}
	if( b_UpdFlg ){
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_98;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt98[ 0 ]);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	SST_LOG2(MFPE_LogLevelAll, "End.\n", NULL);
}

//************************************************************************************************************************.
//概要　　：デバイス情報プリンタ取得処理.
//戻り値　：無し.
//備考　　：特に無し.
//作成者　：谷本.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
// 2012. 5.15 [CQ363516] パフォーマンス改善 ポーリング周期を500msと2sに変更.
//************************************************************************************************************************.
void SSTC_SystemStatusService::getPrinterDeviceStatus_Low()
{
	SST_LOG(MFPE_LogLevelAll);
	SSTS_ChangedInfo stChangedInfo;

	APIC_PrinterStatus* pc_APIPrinterStatus = APIC_PrinterStatus::newInstance();

	// 「環境履歴」情報取得処理結果.
	ushort us_EDI_EnvHistory[ EDI_EnvHistorySize ];
	Bool b_UpdFlg = False;
	Bool b_Val = pc_APIPrinterStatus->getEnvironmentalHistoryInfoData( &us_EDI_EnvHistory[ 0 ] );
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt1.b_EnvHistoryResult != b_Val){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt1.b_EnvHistoryResult  = b_Val;
		b_UpdFlg = True;
	}
	for( ushort us_Idx = 0; us_Idx < EDI_EnvHistorySize; us_Idx++ ){
		if( m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt1.us_EnvHistoryData[ us_Idx ] != us_EDI_EnvHistory[ us_Idx ] ){
			m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt1.us_EnvHistoryData[ us_Idx ]  = us_EDI_EnvHistory[ us_Idx ];
			if( b_UpdFlg == False ){
				b_UpdFlg = True;
			}
		}
	}
	if( b_UpdFlg == True ){
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_1;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt1);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// 「用紙到達時間」情報取得処理.
	static TYPE_EDI_PaperReachTime e_PaperReachTimeTable[] = {
		TYPE_EDI_PaperReachTime_1stTraySpd1Ave,						// 用紙到達時間（１段目給紙口【速度1】 平均）.
		TYPE_EDI_PaperReachTime_1stTraySpd1Stdev,					// 用紙到達時間（１段目給紙口【速度1】 標準偏差）.
		TYPE_EDI_PaperReachTime_1stTraySpd1TotalNum,				// 用紙到達時間（１段目給紙口【速度1】 通紙枚数）.
		TYPE_EDI_PaperReachTime_1stTraySpd2Ave,						// 用紙到達時間（１段目給紙口【速度２】 平均）.
		TYPE_EDI_PaperReachTime_1stTraySpd2Stdev,					// 用紙到達時間（１段目給紙口【速度２】 標準偏差）.
		TYPE_EDI_PaperReachTime_1stTraySpd2TotalNum,				// 用紙到達時間（１段目給紙口【速度２】 通紙枚数）.
		TYPE_EDI_PaperReachTime_1stTraySpd3Ave,						// 用紙到達時間（１段目給紙口【速度３】 平均）.
		TYPE_EDI_PaperReachTime_1stTraySpd3Stdev,					// 用紙到達時間（１段目給紙口【速度３】 標準偏差）.
		TYPE_EDI_PaperReachTime_1stTraySpd3TotalNum,				// 用紙到達時間（１段目給紙口【速度３】 通紙枚数）.
		TYPE_EDI_PaperReachTime_1stTraySpd4Ave,						// 用紙到達時間（１段目給紙口【速度４】 平均）.
		TYPE_EDI_PaperReachTime_1stTraySpd4Stdev,					// 用紙到達時間（１段目給紙口【速度４】 標準偏差）.
		TYPE_EDI_PaperReachTime_1stTraySpd4TotalNum,				// 用紙到達時間（１段目給紙口【速度４】 通紙枚数）.
		TYPE_EDI_PaperReachTime_2ndTraySpd1Ave,						// 用紙到達時間（２段目給紙口【速度1】 平均）.
		TYPE_EDI_PaperReachTime_2ndTraySpd1Stdev,					// 用紙到達時間（２段目給紙口【速度1】 標準偏差）.
		TYPE_EDI_PaperReachTime_2ndTraySpd1TotalNum,				// 用紙到達時間（２段目給紙口【速度1】 通紙枚数）.
		TYPE_EDI_PaperReachTime_2ndTraySpd2Ave,						// 用紙到達時間（２段目給紙口【速度２】 平均）.
		TYPE_EDI_PaperReachTime_2ndTraySpd2Stdev,					// 用紙到達時間（２段目給紙口【速度２】 標準偏差）.
		TYPE_EDI_PaperReachTime_2ndTraySpd2TotalNum,				// 用紙到達時間（２段目給紙口【速度２】 通紙枚数）.
		TYPE_EDI_PaperReachTime_2ndTraySpd3Ave,						// 用紙到達時間（２段目給紙口【速度３】 平均）.
		TYPE_EDI_PaperReachTime_2ndTraySpd3Stdev,					// 用紙到達時間（２段目給紙口【速度３】 標準偏差）.
		TYPE_EDI_PaperReachTime_2ndTraySpd3TotalNum,				// 用紙到達時間（２段目給紙口【速度３】 通紙枚数）.
		TYPE_EDI_PaperReachTime_2ndTraySpd4Ave,						// 用紙到達時間（２段目給紙口【速度４】 平均）.
		TYPE_EDI_PaperReachTime_2ndTraySpd4Stdev,					// 用紙到達時間（２段目給紙口【速度４】 標準偏差）.
		TYPE_EDI_PaperReachTime_2ndTraySpd4TotalNum,				// 用紙到達時間（２段目給紙口【速度４】 通紙枚数）.
		TYPE_EDI_PaperReachTime_3rdTraySpd1Ave,						// 用紙到達時間（３段目給紙口【速度１】 平均）.
		TYPE_EDI_PaperReachTime_3rdTraySpd1Stdev,					// 用紙到達時間（３段目給紙口【速度１】 標準偏差）.
		TYPE_EDI_PaperReachTime_3rdTraySpd1TotalNum,				// 用紙到達時間（３段目給紙口【速度１】 通紙枚数）.
		TYPE_EDI_PaperReachTime_3rdTraySpd2Ave,						// 用紙到達時間（３段目給紙口【速度２】 平均）.
		TYPE_EDI_PaperReachTime_3rdTraySpd2Stdev,					// 用紙到達時間（３段目給紙口【速度２】 標準偏差）.
		TYPE_EDI_PaperReachTime_3rdTraySpd2TotalNum,				// 用紙到達時間（３段目給紙口【速度２】 通紙枚数）.
		TYPE_EDI_PaperReachTime_4thTraySpd1Ave,						// 用紙到達時間（４段目給紙口【速度１】 平均）.
		TYPE_EDI_PaperReachTime_4thTraySpd1Stdev,					// 用紙到達時間（４段目給紙口【速度１】 標準偏差）.
		TYPE_EDI_PaperReachTime_4thTraySpd1TotalNum,				// 用紙到達時間（４段目給紙口【速度１】 通紙枚数）.
		TYPE_EDI_PaperReachTime_4thTraySpd2Ave,						// 用紙到達時間（４段目給紙口【速度２】 平均）.
		TYPE_EDI_PaperReachTime_4thTraySpd2Stdev,					// 用紙到達時間（４段目給紙口【速度２】 標準偏差）.
		TYPE_EDI_PaperReachTime_4thTraySpd2TotalNum,				// 用紙到達時間（４段目給紙口【速度２】 通紙枚数）.
		TYPE_EDI_PaperReachTime_LCTTraySpd1Ave,						// 用紙到達時間（LCT給紙口【速度１】 平均）.
		TYPE_EDI_PaperReachTime_LCTTraySpd1Stdev,					// 用紙到達時間（LCT給紙口【速度１】 標準偏差）.
		TYPE_EDI_PaperReachTime_LCTTraySpd1TotalNum,				// 用紙到達時間（LCT給紙口【速度１】 通紙枚数）.
		TYPE_EDI_PaperReachTime_LCTTraySpd2Ave,						// 用紙到達時間（LCT給紙口【速度２】 平均）.
		TYPE_EDI_PaperReachTime_LCTTraySpd2Stdev,					// 用紙到達時間（LCT給紙口【速度２】 標準偏差）.
		TYPE_EDI_PaperReachTime_LCTTraySpd2TotalNum,				// 用紙到達時間（LCT給紙口【速度２】 通紙枚数）.
		TYPE_EDI_PaperReachTime_HandTraySpd1Ave,					// 用紙到達時間（手差し給紙口【速度１】 平均）.
		TYPE_EDI_PaperReachTime_HandTraySpd1Stdev,					// 用紙到達時間（手差し給紙口【速度１】 標準偏差）.
		TYPE_EDI_PaperReachTime_HandTraySpd1TotalNum,				// 用紙到達時間（手差し給紙口【速度１】 通紙枚数）.
		TYPE_EDI_PaperReachTime_HandTraySpd2Ave,					// 用紙到達時間（手差し給紙口【速度２】 平均）.
		TYPE_EDI_PaperReachTime_HandTraySpd2Stdev,					// 用紙到達時間（手差し給紙口【速度２】 標準偏差）.
		TYPE_EDI_PaperReachTime_HandTraySpd2TotalNum,				// 用紙到達時間（手差し給紙口【速度２】 通紙枚数）.
		TYPE_EDI_PaperReachTime_HandTraySpd3Ave,					// 用紙到達時間（手差し給紙口【速度３】 平均）.
		TYPE_EDI_PaperReachTime_HandTraySpd3Stdev,					// 用紙到達時間（手差し給紙口【速度３】 標準偏差）.
		TYPE_EDI_PaperReachTime_HandTraySpd3TotalNum,				// 用紙到達時間（手差し給紙口【速度３】 通紙枚数）.
		TYPE_EDI_PaperReachTime_HandTraySpd4Ave,					// 用紙到達時間（手差し給紙口【速度４】 平均）.
		TYPE_EDI_PaperReachTime_HandTraySpd4Stdev,					// 用紙到達時間（手差し給紙口【速度４】 標準偏差）.
		TYPE_EDI_PaperReachTime_HandTraySpd4TotalNum,				// 用紙到達時間（手差し給紙口【速度４】 通紙枚数）.
		TYPE_EDI_PaperReachTime_ADURevSpd1Ave,						// 用紙到達時間（ADU反転部【速度１】 平均）.
		TYPE_EDI_PaperReachTime_ADURevSpd1Stdev,					// 用紙到達時間（ADU反転部【速度１】 標準偏差）.
		TYPE_EDI_PaperReachTime_ADURevSpd1TotalNum,					// 用紙到達時間（ADU反転部【速度１】 通紙枚数）.
		TYPE_EDI_PaperReachTime_ADURevSpd2Ave,						// 用紙到達時間（ADU反転部【速度２】 平均）.
		TYPE_EDI_PaperReachTime_ADURevSpd2Stdev,					// 用紙到達時間（ADU反転部【速度２】 標準偏差）.
		TYPE_EDI_PaperReachTime_ADURevSpd2TotalNum,					// 用紙到達時間（ADU反転部【速度２】 通紙枚数）.
		TYPE_EDI_PaperReachTime_ADURevSpd3Ave,						// 用紙到達時間（ADU反転部【速度３】 平均）.
		TYPE_EDI_PaperReachTime_ADURevSpd3Stdev,					// 用紙到達時間（ADU反転部【速度３】 標準偏差）.
		TYPE_EDI_PaperReachTime_ADURevSpd3TotalNum,					// 用紙到達時間（ADU反転部【速度３】 通紙枚数）.
		TYPE_EDI_PaperReachTime_ADURevSpd4Ave,						// 用紙到達時間（ADU反転部【速度４】 平均）.
		TYPE_EDI_PaperReachTime_ADURevSpd4Stdev,					// 用紙到達時間（ADU反転部【速度４】 標準偏差）.
		TYPE_EDI_PaperReachTime_ADURevSpd4TotalNum,					// 用紙到達時間（ADU反転部【速度４】 通紙枚数）.
		TYPE_EDI_PaperReachTime_ADURevAccele1Ave,					// 用紙到達時間（ADU反転部【加速１】 平均）.
		TYPE_EDI_PaperReachTime_ADURevAccele1Stdev,					// 用紙到達時間（ADU反転部【加速１】 標準偏差）.
		TYPE_EDI_PaperReachTime_ADURevAccele1TotalNum,				// 用紙到達時間（ADU反転部【加速１】 通紙枚数）.
		TYPE_EDI_PaperReachTime_ADURevAccele2Ave,					// 用紙到達時間（ADU反転部【加速２】 平均）.
		TYPE_EDI_PaperReachTime_ADURevAccele2Stdev,					// 用紙到達時間（ADU反転部【加速２】 標準偏差）.
		TYPE_EDI_PaperReachTime_ADURevAccele2TotalNum,				// 用紙到達時間（ADU反転部【加速２】 通紙枚数）.
		TYPE_EDI_PaperReachTime_ADURevAccele1ShortAve,				// 用紙到達時間（ADU反転部【加速１short】 平均）.
		TYPE_EDI_PaperReachTime_ADURevAccele1ShortStdev,			// 用紙到達時間（ADU反転部【加速１short】 標準偏差）.
		TYPE_EDI_PaperReachTime_ADURevAccele1ShortTotalNum,			// 用紙到達時間（ADU反転部【加速１short】 通紙枚数）.
		TYPE_EDI_PaperReachTime_ADURevAccele2ShortAve,				// 用紙到達時間（ADU反転部【加速２short】 平均）.
		TYPE_EDI_PaperReachTime_ADURevAccele2ShortStdev,			// 用紙到達時間（ADU反転部【加速２short】 標準偏差）.
		TYPE_EDI_PaperReachTime_ADURevAccele2ShortTotalNum,			// 用紙到達時間（ADU反転部【加速２short】 通紙枚数）.
		TYPE_EDI_PaperReachTime_4thTraySpd3Ave,						// 用紙到達時間（４段目給紙口【速度３】 平均）.
		TYPE_EDI_PaperReachTime_4thTraySpd3Stdev,					// 用紙到達時間（４段目給紙口【速度３】 標準偏差）.
		TYPE_EDI_PaperReachTime_4thTraySpd3TotalNum,				// 用紙到達時間（４段目給紙口【速度３】 通紙枚数）.
		TYPE_EDI_PaperReachTime_4thTraySpd4Ave,						// 用紙到達時間（４段目給紙口【速度４】 平均）.
		TYPE_EDI_PaperReachTime_4thTraySpd4Stdev,					// 用紙到達時間（４段目給紙口【速度４】 標準偏差）.
		TYPE_EDI_PaperReachTime_4thTraySpd4TotalNum,				// 用紙到達時間（４段目給紙口【速度４】 通紙枚数）.
		TYPE_EDI_PaperReachTime_LCTTraySpd3Ave,						// 用紙到達時間（LCT給紙口【速度３】 平均）.
		TYPE_EDI_PaperReachTime_LCTTraySpd3Stdev,					// 用紙到達時間（LCT給紙口【速度３】 標準偏差）.
		TYPE_EDI_PaperReachTime_LCTTraySpd3TotalNum,				// 用紙到達時間（LCT給紙口【速度３】 通紙枚数）.
		TYPE_EDI_PaperReachTime_LCTTraySpd4Ave,						// 用紙到達時間（LCT給紙口【速度４】 平均）.
		TYPE_EDI_PaperReachTime_LCTTraySpd4Stdev,					// 用紙到達時間（LCT給紙口【速度４】 標準偏差）.
		TYPE_EDI_PaperReachTime_LCTTraySpd4TotalNum,				// 用紙到達時間（LCT給紙口【速度４】 通紙枚数）.
		TYPE_EDI_PaperReachTime_3rdTraySpd3Ave,						// 用紙到達時間（３段目給紙口【速度３】 平均）.
		TYPE_EDI_PaperReachTime_3rdTraySpd3Stdev,					// 用紙到達時間（３段目給紙口【速度３】 標準偏差）.
		TYPE_EDI_PaperReachTime_3rdTraySpd3TotalNum,				// 用紙到達時間（３段目給紙口【速度３】 通紙枚数）.
		TYPE_EDI_PaperReachTime_3rdTraySpd4Ave,						// 用紙到達時間（３段目給紙口【速度４】 平均）.
		TYPE_EDI_PaperReachTime_3rdTraySpd4Stdev,					// 用紙到達時間（３段目給紙口【速度４】 標準偏差）.
		TYPE_EDI_PaperReachTime_3rdTraySpd4TotalNum					// 用紙到達時間（３段目給紙口【速度４】 通紙枚数）.
	};
	
	b_UpdFlg = False;
	for( ushort us_PaperReachTimeIdx = 0; us_PaperReachTimeIdx < sizeof( e_PaperReachTimeTable ) / sizeof( TYPE_EDI_PaperReachTime ) ; us_PaperReachTimeIdx++){
		ushort us_Val = pc_APIPrinterStatus->getPaperReachTime( e_PaperReachTimeTable[ us_PaperReachTimeIdx ] );
		if(m_stSysStatInfo.st_PrinterDeviceInfoParam.us_DevPrt2[ (ushort)e_PaperReachTimeTable[ us_PaperReachTimeIdx ] ] != us_Val){
			m_stSysStatInfo.st_PrinterDeviceInfoParam.us_DevPrt2[ (ushort)e_PaperReachTimeTable[ us_PaperReachTimeIdx ] ] = us_Val;
			if( b_UpdFlg == False ){
				b_UpdFlg = True;
			}
		}
	}
	if( b_UpdFlg ){
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_2;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.us_DevPrt2[ 0 ]);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// 「安定化系カウンタ」情報取得処理.
	static TYPE_EDI_Stabilization e_StabilizationTable[] = {
		TYPE_EDI_BootStabilizeCntUnder,								// マシン起動時の安定化回数（下位）.
		TYPE_EDI_BootStabilizeCntUpper,								// マシン起動時の安定化回数（上位）.
		TYPE_EDI_BootNotStabilizeCntUnder,							// マシン起動時に安定化が不要な回数 （下位）.
		TYPE_EDI_BootNotStabilizeCntUpper,							// マシン起動時に安定化が不要な回数 （上位）.
		TYPE_EDI_PnlStabilizeCnt,									// パネルからの安定化回数.
		TYPE_EDI_PnumStabilizeCnt,									// プリント枚数による安定化回数.
		TYPE_EDI_ChangeEnvStabilizeCnt,								// 環境変化による安定化回数.
		TYPE_EDI_ImgAdjStabilizeCnt,								// 画像調整必須時の安定化回数.
		TYPE_EDI_TotalStabilizeTime									// 積算安定化時間.
#ifdef DEF_A3A4COMM
		,TYPE_EDI_DevStopStabilizeCntLow,							// 現像停止時間による安定化回数 (下位).
		TYPE_EDI_DevStopStabilizeCntHig,							// 現像停止時間による安定化回数 (上位).
		TYPE_EDI_RecoveryTonerEmptyStabilizeCntLow,					// トナーエンプティ回復による安定化回数 (下位).
		TYPE_EDI_RecoveryTonerEmptyStabilizeCntHig,					// トナーエンプティ回復による安定化回数 (上位).
		TYPE_EDI_PnlStabilizeCntLow,								// パネルからの指示による安定化回数 (下位).
		TYPE_EDI_PnlStabilizeCntHig,								// パネルからの指示による安定化回数 (上位).
		TYPE_EDI_PHChangeStabilizeCntLow,							// PH温度変化による安定化回数 (下位).
		TYPE_EDI_PHChangeStabilizeCntHig,							// PH温度変化による安定化回数 (上位).
		TYPE_EDI_LCAdjustEnvStabilizeCntLow,						// LD調整後の環境変化による安定化回数 (下位).
		TYPE_EDI_LCAdjustEnvStabilizeCntHig,						// LD調整後の環境変化による安定化回数 (上位).
		TYPE_EDI_AdjStabilizeCntLow,								// 画像調整必須時（Pコール）による安定化回数 (下位).
		TYPE_EDI_AdjStabilizeCntHig,								// 画像調整必須時（Pコール）による安定化回数 (上位).
		TYPE_EDI_LCAdjustPageStabilizeCntLow,						// LD調整後のプリント枚数による安定化回数 (下位).
		TYPE_EDI_LCAdjustPageStabilizeCntHig,						// LD調整後のプリント枚数による安定化回数 (上位).
		TYPE_EDI_GammaAutoStabilizeCntLow,							// γ調整後のプリント枚数による安定化回数 (下位).
		TYPE_EDI_GammaAutoStabilizeCntHig							// γ調整後のプリント枚数による安定化回数 (上位).
#endif // DEF_A3A4COMM
	};
	
	b_UpdFlg = False;
	for( ushort us_StabilizationIdx = 0; us_StabilizationIdx < sizeof( e_StabilizationTable ) / sizeof( TYPE_EDI_Stabilization ) ; us_StabilizationIdx++){
		ushort us_Val = pc_APIPrinterStatus->getStabilizationCounterInfoData( e_StabilizationTable[ us_StabilizationIdx ] );
		if(m_stSysStatInfo.st_PrinterDeviceInfoParam.us_DevPrt3[ (ushort)e_StabilizationTable[ us_StabilizationIdx ] ] != us_Val){
			m_stSysStatInfo.st_PrinterDeviceInfoParam.us_DevPrt3[ (ushort)e_StabilizationTable[ us_StabilizationIdx ] ] = us_Val;
			if( b_UpdFlg == False ){
				b_UpdFlg = True;
			}
		}
	}
	if( b_UpdFlg ){
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_3;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.us_DevPrt3);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	SST_LOG2(MFPE_LogLevelAll, "End.\n", NULL);
}

//************************************************************************************************************************.
//概要　　：RDS警告情報取得処理.
//戻り値　：無し.
//備考　　：特に無し.
//作成者　：高橋.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
// 2012. 5.15 [CQ363516] パフォーマンス改善 ポーリング周期を500msと2sに変更.
//************************************************************************************************************************.
void SSTC_SystemStatusService::getRdsWarningStatus_Low()
{
	SST_LOG(MFPE_LogLevelAll);
	
	typedef struct st_CheckWarning {
		TYPE_RdsWarning			e_Rds;					// RDS警告情報.
		TYPE_NTF_DataID				e_DataID;				// データID.
		Bool 				   	   *pb_Val;					// 警告の有無.
	}ST_CHK_WARNING;
	static ST_CHK_WARNING st_CheckRdsWarningTable[] = {
		{ ATTENTION_ModemError		, TYPE_NTF_DataID_WRN_RDS_1, &( m_stSysStatInfo.st_WarningInfoParam.st_RdsWarningInfoParam.b_WrnRds1 ) },	// モデム異常.
		{ ATTENTION_ControllerError	, TYPE_NTF_DataID_WRN_RDS_2, &( m_stSysStatInfo.st_WarningInfoParam.st_RdsWarningInfoParam.b_WrnRds2 ) },	// コントローラ異常.
		{ (TYPE_RdsWarning)0		, TYPE_NTF_DataID_End		, NULL						   												}	// end.
	};
	SSTS_ChangedInfo stChangedInfo;
	Bool b_Val;
	
	for( ST_CHK_WARNING *pst_WarTbl = &st_CheckRdsWarningTable[ 0 ]; pst_WarTbl->e_DataID != TYPE_NTF_DataID_End; pst_WarTbl++ ){
		b_Val = APIC_Rds::newInstance()->isWarning( pst_WarTbl->e_Rds );
		if( *( pst_WarTbl->pb_Val ) != b_Val ){
			*( pst_WarTbl->pb_Val ) = b_Val;
			stChangedInfo.e_DataID = pst_WarTbl->e_DataID;
			stChangedInfo.p_Value  = pst_WarTbl->pb_Val;
			// 2021/10/22 APF OP_BTS-35403 start
			//m_vChangedInfo.push_back( stChangedInfo );
			AddChangedInfo( &stChangedInfo );
			// 2021/10/22 APF OP_BTS-35403 end
		}
	}

}

//************************************************************************************************************************.
//概要　　：BOX警告情報取得処理.
//戻り値　：無し.
//備考　　：isWarning()に第２パラメータがあり 現状ではデフォルトパラメータを利用してパラメータは１つとしている.
//　　　　：DOCE_BoxDocumentOverを設定する場合は第２パラメータが必須との記述があるのだが現状では常にDOCD_FALSE.
//　　　　：を返す実装となっている為、下の実装としている。.
//　　　　：またSystemStatusとしては上記の第２パラメータの取得方法がない.
//作成者　：高橋.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
// 2012. 5.15 [CQ363516] パフォーマンス改善 ポーリング周期を500msと2sに変更.
//************************************************************************************************************************.
void SSTC_SystemStatusService::getBoxWarningStatus_High()
{
	SST_LOG(MFPE_LogLevelAll);

	typedef struct st_CheckWarning {
		APIE_BoxWarningState	e_Box;					// BOX警告情報.
		TYPE_NTF_DataID			e_DataID;				// データID.
		Bool 				   	*pb_Val;				// 警告の有無.
	}ST_CHK_WARNING;
	static ST_CHK_WARNING st_CheckBoxWarningTable[] = {
		{ APIE_HddFull	    			, TYPE_NTF_DataID_WRN_BOX_1, &( m_stSysStatInfo.st_WarningInfoParam.st_BoxWarningInfoParam.b_WrnBox1 ) },	// HDD容量がFULLになったか.
		{ APIE_BoxCountOver				, TYPE_NTF_DataID_WRN_BOX_2, &( m_stSysStatInfo.st_WarningInfoParam.st_BoxWarningInfoParam.b_WrnBox2 ) },	// Box登録数Over(1000個に達したか).
		{ APIE_BoxDocumentOver			, TYPE_NTF_DataID_WRN_BOX_3, &( m_stSysStatInfo.st_WarningInfoParam.st_BoxWarningInfoParam.b_WrnBox3 ) },	// Box内Document数Over(200数に達したか).
		{ APIE_DocumentPageCountOver	, TYPE_NTF_DataID_WRN_BOX_4, &( m_stSysStatInfo.st_WarningInfoParam.st_BoxWarningInfoParam.b_WrnBox4 ) },	// Documentページ数Over(9000ページに達したか).
		{ (APIE_BoxWarningState)0		, TYPE_NTF_DataID_End      , NULL																		 }	// end.
	};
	SSTS_ChangedInfo stChangedInfo;
	Bool b_Val;
	
	APIC_BoxRegistry *pc_Instance = APIC_BoxRegistry::newInstance();
	if( pc_Instance != NULL ){
		for( ST_CHK_WARNING *pst_WarTbl = &st_CheckBoxWarningTable[ 0 ]; pst_WarTbl->e_DataID != TYPE_NTF_DataID_End; pst_WarTbl++ ){
			b_Val = pc_Instance->isWarning( pst_WarTbl->e_Box );
			if( *( pst_WarTbl->pb_Val ) != b_Val ){
				*( pst_WarTbl->pb_Val ) = b_Val;
				stChangedInfo.e_DataID = pst_WarTbl->e_DataID;
				stChangedInfo.p_Value  = pst_WarTbl->pb_Val;
				// 2021/10/22 APF OP_BTS-35403 start
				//m_vChangedInfo.push_back( stChangedInfo );
				AddChangedInfo( &stChangedInfo );
				// 2021/10/22 APF OP_BTS-35403 end
			}
		}
		pc_Instance->deleteInstance();
	}

}

//************************************************************************************************************************.
//概要　　：FAX警告情報取得処理.
//戻り値　：無し.
//備考　　：特に無し.
//作成者　：高橋.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
// 2012. 5.15 [CQ363516] パフォーマンス改善 ポーリング周期を500msと2sに変更.
// 2012. 5.23 [CQ363516] パフォーマンス改善 FAX動作中のみ、500ms周期でポーリング処理を行う.
//************************************************************************************************************************.
void SSTC_SystemStatusService::getFaxWarningStatus_High()
{
	SST_LOG(MFPE_LogLevelAll);
	
	static Bool b_PrevLineStatus1 = True;	// 前回の通信状態(回線1) [初期値:True].
	static Bool b_PrevLineStatus2 = True;	// 前回の通信状態(回線2) [初期値:True].
	// UV-00877 help add start.
	static Bool b_PrevLineStatus3 = True;	// 前回の通信状態(回線3) [初期値:True].
	static Bool b_PrevLineStatus4 = True;	// 前回の通信状態(回線4) [初期値:True].
	// UV-00877 help add end.

	APIC_FaxDevice* pc_FaxDevice = APIC_FaxDevice::newInstance();

	// (回線1).
	TYPE_CommLine e_CommLine = TYPE_CommLine1;
	Bool b_LineStatus1 = True;	// 通信状態(回線1).
	if ( (pc_FaxDevice->getTaskStatus(FAXE_StatusNon, e_CommLine) == True)	// 未装着 or 待機中 の場合.
// CQ376114 対応 Sukigara s.
//		 || (pc_FaxDevice->getTaskStatus(FAXE_StatusReady, e_CommLine) == True)
		) { b_LineStatus1 = False; }
// CQ376114 対応 Sukigara e.
	if ( (b_LineStatus1 == True) || (b_PrevLineStatus1 == True) ) {	// 通信中 or 前回が通信中 の場合.
		getFaxWarningStatus_High_Line1();
	}
	SST_LOG2(MFPE_LogLevelTrace, "LineStatus1=(%d->%d).\n", b_PrevLineStatus1, b_LineStatus1);
	b_PrevLineStatus1 = b_LineStatus1;	// 通信状態退避(回線1).

	// (回線2).
	e_CommLine = TYPE_CommLine2;
	Bool b_LineStatus2 = True;	// 通信状態(回線2).
	if ( (pc_FaxDevice->getTaskStatus(FAXE_StatusNon, e_CommLine) == True)	// 未装着 or 待機中 の場合.
// CQ376114 対応 Sukigara s.
//		 || (pc_FaxDevice->getTaskStatus(FAXE_StatusReady, e_CommLine) == True)
		) { b_LineStatus2 = False; }
// CQ376114 対応 Sukigara e.
	if ( (b_LineStatus2 == True) || (b_PrevLineStatus2 == True) ) {	// 通信中 or 前回が通信中 の場合.
		getFaxWarningStatus_High_Line2();
	}
	SST_LOG2(MFPE_LogLevelTrace, "LineStatus2=(%d->%d).\n", b_PrevLineStatus2, b_LineStatus2);
	b_PrevLineStatus2 = b_LineStatus2;	// 通信状態退避(回線2).

	// UV-00877 help add start.
	// (回線3).
	e_CommLine = TYPE_CommLine3;
	Bool b_LineStatus3 = True;	// 通信状態(回線3).
	if ( (pc_FaxDevice->getTaskStatus(FAXE_StatusNon, e_CommLine) == True)	// 未装着 or 待機中 の場合.
// CQ376114 対応 Sukigara s.
//		 || (pc_FaxDevice->getTaskStatus(FAXE_StatusReady, e_CommLine) == True)
		) { b_LineStatus3 = False; }
// CQ376114 対応 Sukigara e.
	if ( (b_LineStatus3 == True) || (b_PrevLineStatus3 == True) ) {	// 通信中 or 前回が通信中 の場合.
		getFaxWarningStatus_High_Line3();
	}
	SST_LOG2(MFPE_LogLevelTrace, "LineStatus3=(%d->%d).\n", b_PrevLineStatus3, b_LineStatus3);
	b_PrevLineStatus3 = b_LineStatus3;	// 通信状態退避(回線3).

	// (回線4).
	e_CommLine = TYPE_CommLine4;
	Bool b_LineStatus4 = True;	// 通信状態(回線4).
	if ( (pc_FaxDevice->getTaskStatus(FAXE_StatusNon, e_CommLine) == True)	// 未装着 or 待機中 の場合.
// CQ376114 対応 Sukigara s.
//		 || (pc_FaxDevice->getTaskStatus(FAXE_StatusReady, e_CommLine) == True)
		) { b_LineStatus4 = False; }
// CQ376114 対応 Sukigara e.
	if ( (b_LineStatus4 == True) || (b_PrevLineStatus4 == True) ) {	// 通信中 or 前回が通信中 の場合.
		getFaxWarningStatus_High_Line4();
	}
	SST_LOG2(MFPE_LogLevelTrace, "LineStatus4=(%d->%d).\n", b_PrevLineStatus4, b_LineStatus4);
	b_PrevLineStatus4 = b_LineStatus4;	// 通信状態退避(回線4).
	// UV-00877 help add end.

// 2018/02/27 APF 制御構成変更No.82 リソースフル状態の表示対応 s.
	APIC_SystemInfo* pc_SystemInfo = APIC_SystemInfo::newInstance();
	// FAXボードが装着されている場合のみ実行
	if( ( True == pc_SystemInfo->getFax1Exist() )							// ファクス(主回線) 設定値の取得.
		|| ( True == pc_SystemInfo->getFax2Exist() )						// ファクス(副回線) 設定値の取得.
		|| ( True == pc_SystemInfo->getFaxExistAll( TYPE_CommLine3 ) )		// ファクス(回線3) 設定値の取得.
		|| ( True == pc_SystemInfo->getFaxExistAll( TYPE_CommLine4 ) ) ){	// ファクス(回線4) 設定値の取得.
		getFaxWarningStatus_High_Other();
	}
// 2018/02/27 APF 制御構成変更No.82 リソースフル状態の表示対応 e.
}

//************************************************************************************************************************.
//概要　　：FAX警告情報取得処理 (回線1).
//戻り値　：無し.
//備考　　：特に無し.
//作成者　：高橋.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
// 2012. 5.23 [CQ363516] パフォーマンス改善 FAX動作中のみ、500ms周期でポーリング処理を行う.
//************************************************************************************************************************.
void SSTC_SystemStatusService::getFaxWarningStatus_High_Line1()
{
	SST_LOG(MFPE_LogLevelAll);
	
	typedef struct st_CheckWarning {
		FAXE_Warning			e_Kind;					// FAX警告情報.
		TYPE_NTF_DataID				e_DataID;				// データID.
		Bool 				   	   *pb_Val;					// 警告の有無.
	}ST_CHK_WARNING;
	static ST_CHK_WARNING st_CheckFaxWarningTable[] = {
		{ FAXE_WarningDial	, TYPE_NTF_DataID_WRN_FAX_1,  &( m_stSysStatInfo.st_WarningInfoParam.st_FaxWarningInfoParam.st_WrnFax1.b_Line1 ) },	// ダイアルエラー(回線1).
		{ FAXE_WarningXmt	, TYPE_NTF_DataID_WRN_FAX_3,  &( m_stSysStatInfo.st_WarningInfoParam.st_FaxWarningInfoParam.st_WrnFax2.b_Line1 ) },	// 送信中エラー(回線1).
		{ FAXE_WarningRcv	, TYPE_NTF_DataID_WRN_FAX_5,  &( m_stSysStatInfo.st_WarningInfoParam.st_FaxWarningInfoParam.st_WrnFax3.b_Line1 ) },	// 受信中エラー(回線1).
		{ FAXE_WarningTest	, TYPE_NTF_DataID_WRN_FAX_7,  &( m_stSysStatInfo.st_WarningInfoParam.st_FaxWarningInfoParam.st_WrnFax4.b_Line1 ) },	// テストのエラー(回線1).
		{ FAXE_WarningIsw	, TYPE_NTF_DataID_WRN_FAX_9,  &( m_stSysStatInfo.st_WarningInfoParam.st_FaxWarningInfoParam.st_WrnFax5.b_Line1 ) },	// Iswのエラー(回線1).
		{ FAXE_WarningNvRam , TYPE_NTF_DataID_WRN_FAX_11, &( m_stSysStatInfo.st_WarningInfoParam.st_FaxWarningInfoParam.st_WrnFax6.b_Line1 ) },	// NvRam書き込み中のエラー(回線1).
		{ FAXE_WarningInit	, TYPE_NTF_DataID_WRN_FAX_13, &( m_stSysStatInfo.st_WarningInfoParam.st_FaxWarningInfoParam.st_WrnFax7.b_Line1 ) },	// 初期化中のエラー(回線1).
		{ (FAXE_Warning)0	, TYPE_NTF_DataID_End		 , NULL														   		  				  }		// end.
	};
	SSTS_ChangedInfo stChangedInfo;
	Bool b_Val;

	for( ST_CHK_WARNING *pst_WarTbl = &st_CheckFaxWarningTable[ 0 ];  pst_WarTbl->e_DataID != TYPE_NTF_DataID_End; pst_WarTbl++ ){
		b_Val = APIC_FaxDevice::newInstance()->isWarning( pst_WarTbl->e_Kind, TYPE_CommLine1 );
		if( *( pst_WarTbl->pb_Val ) != b_Val ){
			*( pst_WarTbl->pb_Val ) = b_Val;
			stChangedInfo.e_DataID = pst_WarTbl->e_DataID;
			stChangedInfo.p_Value  = pst_WarTbl->pb_Val;
			// 2021/10/22 APF OP_BTS-35403 start
			//m_vChangedInfo.push_back( stChangedInfo );
			AddChangedInfo( &stChangedInfo );
			// 2021/10/22 APF OP_BTS-35403 end
		}
	}

}

//************************************************************************************************************************.
//概要　　：FAX警告情報取得処理 (回線2).
//戻り値　：無し.
//備考　　：特に無し.
//作成者　：高橋.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
// 2012. 5.23 [CQ363516] パフォーマンス改善 FAX動作中のみ、500ms周期でポーリング処理を行う.
//************************************************************************************************************************.
void SSTC_SystemStatusService::getFaxWarningStatus_High_Line2()
{
	SST_LOG(MFPE_LogLevelAll);
	
	typedef struct st_CheckWarning {
		FAXE_Warning			e_Kind;					// FAX警告情報.
		TYPE_NTF_DataID				e_DataID;				// データID.
		Bool 				   	   *pb_Val;					// 警告の有無.
	}ST_CHK_WARNING;
	static ST_CHK_WARNING st_CheckFaxWarningTable[] = {
		{ FAXE_WarningDial	, TYPE_NTF_DataID_WRN_FAX_2,  &( m_stSysStatInfo.st_WarningInfoParam.st_FaxWarningInfoParam.st_WrnFax1.b_Line2 ) },	// ダイアルエラー(回線2).
		{ FAXE_WarningXmt	, TYPE_NTF_DataID_WRN_FAX_4,  &( m_stSysStatInfo.st_WarningInfoParam.st_FaxWarningInfoParam.st_WrnFax2.b_Line2 ) },	// 送信中エラー(回線2).
		{ FAXE_WarningRcv	, TYPE_NTF_DataID_WRN_FAX_6,  &( m_stSysStatInfo.st_WarningInfoParam.st_FaxWarningInfoParam.st_WrnFax3.b_Line2 ) },	// 受信中エラー(回線2).
		{ FAXE_WarningTest	, TYPE_NTF_DataID_WRN_FAX_8,  &( m_stSysStatInfo.st_WarningInfoParam.st_FaxWarningInfoParam.st_WrnFax4.b_Line2 ) },	// テストのエラー(回線2).
		{ FAXE_WarningIsw	, TYPE_NTF_DataID_WRN_FAX_10, &( m_stSysStatInfo.st_WarningInfoParam.st_FaxWarningInfoParam.st_WrnFax5.b_Line2 ) },	// Iswのエラー(回線2).
		{ FAXE_WarningNvRam , TYPE_NTF_DataID_WRN_FAX_12, &( m_stSysStatInfo.st_WarningInfoParam.st_FaxWarningInfoParam.st_WrnFax6.b_Line2 ) },	// NvRam書き込み中のエラー(回線2).
		{ FAXE_WarningInit	, TYPE_NTF_DataID_WRN_FAX_14, &( m_stSysStatInfo.st_WarningInfoParam.st_FaxWarningInfoParam.st_WrnFax7.b_Line2 ) },	// 初期化中のエラー(回線2).
		{ (FAXE_Warning)0	, TYPE_NTF_DataID_End		 , NULL														   		  				  }		// end.
	};
	SSTS_ChangedInfo stChangedInfo;
	Bool b_Val;

	for( ST_CHK_WARNING *pst_WarTbl = &st_CheckFaxWarningTable[ 0 ];  pst_WarTbl->e_DataID != TYPE_NTF_DataID_End; pst_WarTbl++ ){
		b_Val = APIC_FaxDevice::newInstance()->isWarning( pst_WarTbl->e_Kind, TYPE_CommLine2 );
		if( *( pst_WarTbl->pb_Val ) != b_Val ){
			*( pst_WarTbl->pb_Val ) = b_Val;
			stChangedInfo.e_DataID = pst_WarTbl->e_DataID;
			stChangedInfo.p_Value  = pst_WarTbl->pb_Val;
			// 2021/10/22 APF OP_BTS-35403 start
			//m_vChangedInfo.push_back( stChangedInfo );
			AddChangedInfo( &stChangedInfo );
			// 2021/10/22 APF OP_BTS-35403 end
		}
	}

}

//************************************************************************************************************************.
//概要　　：FAX警告情報取得処理 (回線3).
//戻り値　：無し.
//備考　　：特に無し.
//作成者　：高橋.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
// UV-00877 add.
//************************************************************************************************************************.
void SSTC_SystemStatusService::getFaxWarningStatus_High_Line3()
{
	SST_LOG(MFPE_LogLevelAll);
	
	typedef struct st_CheckWarning {
		FAXE_Warning			e_Kind;					// FAX警告情報.
		TYPE_NTF_DataID				e_DataID;				// データID.
		Bool 				   	   *pb_Val;					// 警告の有無.
	}ST_CHK_WARNING;
	static ST_CHK_WARNING st_CheckFaxWarningTable[] = {
		{ FAXE_WarningDial	, TYPE_NTF_DataID_WRN_FAX_15,  &( m_stSysStatInfo.st_WarningInfoParam.st_FaxWarningInfoParam.st_WrnFax1.b_Line3 ) },	// ダイアルエラー(回線3).
		{ FAXE_WarningXmt	, TYPE_NTF_DataID_WRN_FAX_17,  &( m_stSysStatInfo.st_WarningInfoParam.st_FaxWarningInfoParam.st_WrnFax2.b_Line3 ) },	// 送信中エラー(回線3).
		{ FAXE_WarningRcv	, TYPE_NTF_DataID_WRN_FAX_19,  &( m_stSysStatInfo.st_WarningInfoParam.st_FaxWarningInfoParam.st_WrnFax3.b_Line3 ) },	// 受信中エラー(回線3).
		{ FAXE_WarningTest	, TYPE_NTF_DataID_WRN_FAX_21,  &( m_stSysStatInfo.st_WarningInfoParam.st_FaxWarningInfoParam.st_WrnFax4.b_Line3 ) },	// テストのエラー(回線3).
		{ FAXE_WarningIsw	, TYPE_NTF_DataID_WRN_FAX_23,  &( m_stSysStatInfo.st_WarningInfoParam.st_FaxWarningInfoParam.st_WrnFax5.b_Line3 ) },	// Iswのエラー(回線3).
		{ FAXE_WarningNvRam , TYPE_NTF_DataID_WRN_FAX_25,  &( m_stSysStatInfo.st_WarningInfoParam.st_FaxWarningInfoParam.st_WrnFax6.b_Line3 ) },	// NvRam書き込み中のエラー(回線3).
		{ FAXE_WarningInit	, TYPE_NTF_DataID_WRN_FAX_27,  &( m_stSysStatInfo.st_WarningInfoParam.st_FaxWarningInfoParam.st_WrnFax7.b_Line3 ) },	// 初期化中のエラー(回線3).
		{ (FAXE_Warning)0	, TYPE_NTF_DataID_End		 , NULL														   		  				  }		// end.
	};
	SSTS_ChangedInfo stChangedInfo;
	Bool b_Val;

	for( ST_CHK_WARNING *pst_WarTbl = &st_CheckFaxWarningTable[ 0 ];  pst_WarTbl->e_DataID != TYPE_NTF_DataID_End; pst_WarTbl++ ){
		b_Val = APIC_FaxDevice::newInstance()->isWarning( pst_WarTbl->e_Kind, TYPE_CommLine3 );
		if( *( pst_WarTbl->pb_Val ) != b_Val ){
			*( pst_WarTbl->pb_Val ) = b_Val;
			stChangedInfo.e_DataID = pst_WarTbl->e_DataID;
			stChangedInfo.p_Value  = pst_WarTbl->pb_Val;
			// 2021/10/22 APF OP_BTS-35403 start
			//m_vChangedInfo.push_back( stChangedInfo );
			AddChangedInfo( &stChangedInfo );
			// 2021/10/22 APF OP_BTS-35403 end
		}
	}

}

//************************************************************************************************************************.
//概要　　：FAX警告情報取得処理 (回線4).
//戻り値　：無し.
//備考　　：特に無し.
//作成者　：高橋.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
// UV-00877 add.
//************************************************************************************************************************.
void SSTC_SystemStatusService::getFaxWarningStatus_High_Line4()
{
	SST_LOG(MFPE_LogLevelAll);
	
	typedef struct st_CheckWarning {
		FAXE_Warning			e_Kind;					// FAX警告情報.
		TYPE_NTF_DataID				e_DataID;				// データID.
		Bool 				   	   *pb_Val;					// 警告の有無.
	}ST_CHK_WARNING;
	static ST_CHK_WARNING st_CheckFaxWarningTable[] = {
		{ FAXE_WarningDial	, TYPE_NTF_DataID_WRN_FAX_16,  &( m_stSysStatInfo.st_WarningInfoParam.st_FaxWarningInfoParam.st_WrnFax1.b_Line4 ) },	// ダイアルエラー(回線4).
		{ FAXE_WarningXmt	, TYPE_NTF_DataID_WRN_FAX_18,  &( m_stSysStatInfo.st_WarningInfoParam.st_FaxWarningInfoParam.st_WrnFax2.b_Line4 ) },	// 送信中エラー(回線4).
		{ FAXE_WarningRcv	, TYPE_NTF_DataID_WRN_FAX_20,  &( m_stSysStatInfo.st_WarningInfoParam.st_FaxWarningInfoParam.st_WrnFax3.b_Line4 ) },	// 受信中エラー(回線4).
		{ FAXE_WarningTest	, TYPE_NTF_DataID_WRN_FAX_22,  &( m_stSysStatInfo.st_WarningInfoParam.st_FaxWarningInfoParam.st_WrnFax4.b_Line4 ) },	// テストのエラー(回線4).
		{ FAXE_WarningIsw	, TYPE_NTF_DataID_WRN_FAX_24,  &( m_stSysStatInfo.st_WarningInfoParam.st_FaxWarningInfoParam.st_WrnFax5.b_Line4 ) },	// Iswのエラー(回線4).
		{ FAXE_WarningNvRam , TYPE_NTF_DataID_WRN_FAX_26,  &( m_stSysStatInfo.st_WarningInfoParam.st_FaxWarningInfoParam.st_WrnFax6.b_Line4 ) },	// NvRam書き込み中のエラー(回線4).
		{ FAXE_WarningInit	, TYPE_NTF_DataID_WRN_FAX_28,  &( m_stSysStatInfo.st_WarningInfoParam.st_FaxWarningInfoParam.st_WrnFax7.b_Line4 ) },	// 初期化中のエラー(回線4).
		{ (FAXE_Warning)0	, TYPE_NTF_DataID_End		 , NULL														   		  				  }		// end.
	};
	SSTS_ChangedInfo stChangedInfo;
	Bool b_Val;

	for( ST_CHK_WARNING *pst_WarTbl = &st_CheckFaxWarningTable[ 0 ];  pst_WarTbl->e_DataID != TYPE_NTF_DataID_End; pst_WarTbl++ ){
		b_Val = APIC_FaxDevice::newInstance()->isWarning( pst_WarTbl->e_Kind, TYPE_CommLine4 );
		if( *( pst_WarTbl->pb_Val ) != b_Val ){
			*( pst_WarTbl->pb_Val ) = b_Val;
			stChangedInfo.e_DataID = pst_WarTbl->e_DataID;
			stChangedInfo.p_Value  = pst_WarTbl->pb_Val;
			// 2021/10/22 APF OP_BTS-35403 start
			//m_vChangedInfo.push_back( stChangedInfo );
			AddChangedInfo( &stChangedInfo );
			// 2021/10/22 APF OP_BTS-35403 end
		}
	}

}

// 2018/02/27 APF 制御構成変更No.82 リソースフル状態の表示対応 s.
//************************************************************************************************************************.
//概要　　：FAX警告情報取得処理(回線処理以外).
//戻り値　：無し.
//備考　　：特に無し.
//作成者　：吉村.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
// 2018. 2.27 [制御構成変更No.82] リソースフル状態の表示対応.
//************************************************************************************************************************.
void SSTC_SystemStatusService::getFaxWarningStatus_High_Other()
{
	FAXE_ResourceWarning e_ResourceWarning = APIC_FaxDevice::newInstance()->getResourceWarning();		// リソース警告種別の取得
	SSTS_ChangedInfo stChangedInfo;
	
// 2018/04/18 APF AR.1044038 Start.
//	if(m_stSysStatInfo.st_WarningInfoParam.st_FaxWarningInfoParam.e_WrnFax8 != e_ResourceWarning){
	if(m_stSysStatInfo.st_DeviceInfoParam.e_Dev107 != e_ResourceWarning){
		m_stSysStatInfo.st_DeviceInfoParam.e_Dev107 = e_ResourceWarning;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_107;				// リソースの警告種別
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_DeviceInfoParam.e_Dev107);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end

		m_stSysStatInfo.st_WarningInfoParam.st_FaxWarningInfoParam.b_WrnFax8 = (e_ResourceWarning ==  FAXE_ResourceWarning_Non) ? False : True;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_WRN_FAX_29;				// FAX通信不可警告
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_WarningInfoParam.st_FaxWarningInfoParam.b_WrnFax8);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
// 2018/04/18 APF AR.1044038 End.
}
// 2018/02/27 APF 制御構成変更No.82 リソースフル状態の表示対応 e.

//************************************************************************************************************************.
//概要　　：Prt警告情報取得処理.
//戻り値　：無し.
//備考　　：特に無し.
//作成者　：高橋.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
// 2012. 5.15 [CQ363516] パフォーマンス改善 ポーリング周期を500msと2sに変更.
// 2012. 5.31 [CQ363516] パフォーマンス改善 高周期のポーリング処理で、各条件に該当する場合に変化点検出処理を実行する.
//************************************************************************************************************************.
void SSTC_SystemStatusService::getPrtWarningStatus_High()
{
	SST_LOG(MFPE_LogLevelAll);

	static uint32_t ul_PrevCounterWarning = (uint32_t)-1;	// 前回のAPIC_PrinterWarningObserver::dataChanged()発生回数 [初期値:-1].
	static uint32_t ul_PrevCounterDipSw = (uint32_t)-1;		// 前回のAPIC_SoftDipSwObserver::dataChanged()発生回数 [初期値:-1].
	static Bool b_PrevUnitChangeIU = False;	// 前回のユニット交換選択・イメージングユニット（True：サービス交換／False：ユーザー交換） [初期値:False].
// 2016. 5.13 [ZeusSBK/Icarus] MZV-171 消耗品のライフ警告非表示対応 Start.
//	static Bool b_PrevDipSw_1 = False;	// 前回のDipSw( 32, 1 )情報 [初期値:False].
//	static Bool b_PrevDipSw_2 = False;	// 前回のDipSw( 32, 2 )情報 [初期値:False].
//	static Bool b_PrevDipSw_3 = False;	// 前回のDipSw( 32, 3 )情報 [初期値:False].
//	static Bool b_PrevDipSw_4 = False;	// 前回のDipSw( 32, 4 )情報 [初期値:False].
	static Bool b_PrevDipSw_1 = False;	// 前回のDipSw( 111, 6 )情報 [初期値:False].
// 2016. 5.13 [ZeusSBK/Icarus] MZV-171 消耗品のライフ警告非表示対応 End.
	
	if ( (ul_PrevCounterWarning != m_ulCounterPrinterWarningObserver)	// APIC_PrinterWarningObserver::dataChanged() or .
		|| (ul_PrevCounterDipSw != m_ulCounterSoftDipSwObserver) ) {	// APIC_SoftDipSwObserver::dataChanged()が発生していた場合.

		// 一般の警告(パネル警告表示用 以外).
		getPrtWarningStatus_Opt_general();

		// ＤＣユーザー交換超過警告.
		Bool b_Val = (Bool)APIC_PrinterServiceSetting::newInstance()->getPrinterAdjust(TYPE_PrinterAdjustUnitChangeIU);	// ユニット交換選択・イメージングユニット情報取得.
		if ( (b_Val == False) || (b_PrevUnitChangeIU != b_Val) ) {	// APIC_SoftDipSw::getSoftDipSwBit( 32, 1 )がFalseもしくは、値が変化していた場合.
			getPrtWarningStatus_Opt_DCUserExchange();
		}
		b_PrevUnitChangeIU = b_Val;	// ユニット交換選択・イメージングユニット情報情報退避.

		Bool b_DipSwPnl = False;
		Bool b_DipSw = False;

// 2016. 5.13 [ZeusSBK/Icarus] MZV-171 消耗品のライフ警告非表示対応 Start.
//		// パネル警告表示用:IU/DCユニット.
//		b_DipSw = SSTD_GETSOFTDIPSWBIT( 32, 1 );	// DipSw( 32, 1 )情報取得.
//		if ( (b_DipSw == False) || (b_PrevDipSw_1 != b_DipSw) ) {	// APIC_SoftDipSw::getSoftDipSwBit( 32, 1 )がFalseもしくは、値が変化していた場合.
//			getPrtWarningStatus_Opt_pnl_IcDc();
//			b_DipSwPnl = True;
//		}
//		b_PrevDipSw_1 = b_DipSw;	// DipSw( 32, 1 )情報退避.
//
//		// パネル警告表示用:現像ユニット.
//		b_DipSw = SSTD_GETSOFTDIPSWBIT( 32, 2 );	// DipSw( 32, 2 )情報取得.
//		if ( (b_DipSw == False) || (b_PrevDipSw_2 != b_DipSw) ) {	// APIC_SoftDipSw::getSoftDipSwBit( 32, 2 )がFalseもしくは、値が変化していた場合.
//			getPrtWarningStatus_Opt_pnl_DevC();
//			b_DipSwPnl = True;
//		}
//		b_PrevDipSw_2 = b_DipSw;	// DipSw( 32, 2 )情報退避.
//
//		// パネル警告表示用:転写ベルト（現像剤）.
//		b_DipSw = SSTD_GETSOFTDIPSWBIT( 32, 3 );	// DipSw( 32, 3 )情報取得.
//		if ( (b_DipSw == False) || (b_PrevDipSw_3 != b_DipSw) ) {	// APIC_SoftDipSw::getSoftDipSwBit( 32, 3 )がFalseもしくは、値が変化していた場合.
//			getPrtWarningStatus_Opt_pnl_TransBelt();
//			b_DipSwPnl = True;
//		}
//		b_PrevDipSw_3 = b_DipSw;	// DipSw( 32, 3 )情報退避.
//
//		// パネル警告表示用:定着ユニット.
//		b_DipSw = SSTD_GETSOFTDIPSWBIT( 32, 4 );	// DipSw( 32, 4 )情報取得.
//		if ( (b_DipSw == False) || (b_PrevDipSw_4 != b_DipSw) ) {	// APIC_SoftDipSw::getSoftDipSwBit( 32, 4 )がFalseもしくは、値が変化していた場合.
//			getPrtWarningStatus_Opt_pnl_Fuser();
//			b_DipSwPnl = True;
//		}
//		b_PrevDipSw_4 = b_DipSw;	// DipSw( 32, 4 )情報退避.

		b_DipSw = SSTD_GETSOFTDIPSWBIT( 111, 6 );	// DipSw( 111, 6 )情報取得.
#if defined( DEF_A4_Cus ) && defined( DEF_KRONOSS_Cus )
		// KronosSではIUに限りDipSwに依らず警告表示する.
		// パネル警告表示用:IU/DCユニット.
		getPrtWarningStatus_Opt_pnl_IcDc();
		if ( (b_DipSw == False) || (b_PrevDipSw_1 != b_DipSw) ) {	// APIC_SoftDipSw::getSoftDipSwBit( 111, 6 )がFalseもしくは、値が変化していた場合.
#else // defined( DEF_A4_Cus ) && defined( DEF_KRONOSS_Cus ).
		if ( (b_DipSw == False) || (b_PrevDipSw_1 != b_DipSw) ) {	// APIC_SoftDipSw::getSoftDipSwBit( 111, 6 )がFalseもしくは、値が変化していた場合.
			// パネル警告表示用:IU/DCユニット.
			getPrtWarningStatus_Opt_pnl_IcDc();
#endif // defined( DEF_A4_Cus ) && defined( DEF_KRONOSS_Cus ).

			// パネル警告表示用:現像ユニット.
			getPrtWarningStatus_Opt_pnl_DevC();

			// パネル警告表示用:オゾンフィルター.
			getPrtWarningStatus_Opt_pnl_OzonFilter();

			// パネル警告表示用:定着ユニット.
			getPrtWarningStatus_Opt_pnl_Fuser();

			// パネル警告表示用:転写ベルト（現像剤）.
			getPrtWarningStatus_Opt_pnl_TransBelt();

			// パネル警告表示用:転写ローラー.
			getPrtWarningStatus_Opt_pnl_TransRoller();

			// パネル警告表示用:トナー(噴煙)フィルター.
			getPrtWarningStatus_Opt_pnl_SmokeFilter();

			// パネル警告表示用:紙粉除去クリーナー.
			getPrtWarningStatus_Opt_pnl_PaperDust();

// 2017/05/16 APF AR.1018782NG Start.
#if defined(DEF_A4_Cus) && defined(DEF_KRONOSS_Cus)
			// パネル警告表示用:メンテナンスキット.
			getPrtWarningStatus_Opt_pnl_MaintenanceKit();
#endif    // #if defined(DEF_A4_Cus) && defined(DEF_KRONOSS_Cus)
// 2017/05/16 APF AR.1018782NG End.
			b_DipSwPnl = True;
		}
		b_PrevDipSw_1 = b_DipSw;	// DipSw( 111, 6 )情報退避.
// 2016. 5.13 [ZeusSBK/Icarus] MZV-171 消耗品のライフ警告非表示対応 End.

		// パネル警告表示用:その他.
		if (b_DipSwPnl == True) {	// APIC_SoftDipSw::getSoftDipSwBit( 32, 1～4 )の何れかがFalseもしくは、値が変化していた場合.
			getPrtWarningStatus_Opt_pnl_Other();
		}

	}

	ul_PrevCounterWarning = m_ulCounterPrinterWarningObserver;	// APIC_PrinterWarningObserver::dataChanged()発生回数退避.
	ul_PrevCounterDipSw = m_ulCounterSoftDipSwObserver;	// APIC_SoftDipSwObserver::dataChanged()発生回数退避.

}

//************************************************************************************************************************.
//概要　　：Prt警告情報取得処理 (一般の警告(パネル警告表示用 以外)).
//戻り値　：無し.
//備考　　：特に無し.
//作成者　：高橋.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
// 2012. 5.31 [CQ363516] パフォーマンス改善 高周期のポーリング処理で、各条件に該当する場合に変化点検出処理を実行する.
//************************************************************************************************************************.
void SSTC_SystemStatusService::getPrtWarningStatus_Opt_general()
{
	SST_LOG(MFPE_LogLevelAll);
	
	typedef struct st_CheckWarning {
		TYPE_PrinterWarning		e_Prt;					// PRT警告情報.
		TYPE_NTF_DataID				e_DataID;				// データID.
		Bool 				   	   *pb_Val;					// 警告の有無.
	}ST_CHK_WARNING;
	static ST_CHK_WARNING st_CheckPrtWarningTable[] = {
		{ ATTENTION_CTonerNearEmpty	 			,     TYPE_NTF_DataID_WRN_PRT_1  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt1 )   },	// シアン色トナーニアエンプティ.
		{ ATTENTION_MTonerNearEmpty	 			,     TYPE_NTF_DataID_WRN_PRT_2  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt2 )   },	// マゼンダ色トナーニアエンプティ.
		{ ATTENTION_YTonerNearEmpty	 			,     TYPE_NTF_DataID_WRN_PRT_3  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt3 )   },	// イエロー色トナーニアエンプティ.
		{ ATTENTION_KTonerNearEmpty	 			,     TYPE_NTF_DataID_WRN_PRT_4  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt4 )   },	// ブラック色トナーニアエンプティ.
		{ ATTENTION_CTonerEmptyStop	 			,     TYPE_NTF_DataID_WRN_PRT_5  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt5 )   },	// シアン色トナーエンプティストップ.
		{ ATTENTION_MTonerEmptyStop	 			,     TYPE_NTF_DataID_WRN_PRT_6  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt6 )   },	// マゼンダ色トナーエンプティストップ.
		{ ATTENTION_YTonerEmptyStop	 			,     TYPE_NTF_DataID_WRN_PRT_7  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt7 )   },	// イエロー色トナーエンプティストップ.
		{ ATTENTION_KTonerEmptyStop	 			,     TYPE_NTF_DataID_WRN_PRT_8  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt8 )   },	// ブラック色トナーエンプティストップ.
		{ ATTENTION_UsedTonerNearFull			,     TYPE_NTF_DataID_WRN_PRT_9  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt9 )   },	// 廃トナーニアフル.
		{ ATTENTION_PRTWarning					,     TYPE_NTF_DataID_WRN_PRT_10 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt10 )  },	// ＰＲＴ関連異常警告.
		{ ATTENTION_IUNearLifeC					,     TYPE_NTF_DataID_WRN_PRT_11 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt11 )  },	// ＩＵニアライフ・シアン.
		{ ATTENTION_IUNearLifeM					,     TYPE_NTF_DataID_WRN_PRT_12 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt12 )  },	// ＩＵニアライフ・マゼンダ.
		{ ATTENTION_IUNearLifeY					,     TYPE_NTF_DataID_WRN_PRT_13 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt13 )  },	// ＩＵニアライフ・イエロー.
		{ ATTENTION_IUNearLifeK					,     TYPE_NTF_DataID_WRN_PRT_14 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt14 )  },	// ＩＵニアライフ・ブラック.
		{ ATTENTION_DCNearLifeC					,     TYPE_NTF_DataID_WRN_PRT_15 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt15 )  },	// ＤＣニアライフ・シアン.
		{ ATTENTION_DCNearLifeM					,     TYPE_NTF_DataID_WRN_PRT_16 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt16 )  },	// ＤＣニアライフ・マゼンダ.
		{ ATTENTION_DCNearLifeY					,     TYPE_NTF_DataID_WRN_PRT_17 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt17 )  },	// ＤＣニアライフ・イエロー.
		{ ATTENTION_DCNearLifeK					,     TYPE_NTF_DataID_WRN_PRT_18 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt18 )  },	// ＤＣニアライフ・ブラック.
		{ ATTENTION_DevCNearLifeC				,     TYPE_NTF_DataID_WRN_PRT_19 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt19 )  },	// ＤｅｖＣニアライフ・シアン.
		{ ATTENTION_DevCNearLifeM				,     TYPE_NTF_DataID_WRN_PRT_20 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt20 )  },	// ＤｅｖＣニアライフ・マゼンダ.
		{ ATTENTION_DevCNearLifeY				,     TYPE_NTF_DataID_WRN_PRT_21 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt21 )  },	// ＤｅｖＣニアライフ・イエロー.
		{ ATTENTION_DevCNearLifeK				,     TYPE_NTF_DataID_WRN_PRT_22 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt22 )  },	// ＤｅｖＣニアライフ・ブラック.
		{ ATTENTION_FuserUnitNearLife			,     TYPE_NTF_DataID_WRN_PRT_23 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt23 )  },	// 定着ユニットニアライフ.
		{ ATTENTION_StapleEmpty					,     TYPE_NTF_DataID_WRN_PRT_24 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt24 )  },	// ステープルエンプティ1(奥).
		{ ATTENTION_2ndStapleEmpty				,     TYPE_NTF_DataID_WRN_PRT_25 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt25 )  },	// ステープルエンプティ2(手前).
		{ ATTENTION_3rdStapleEmpty				,     TYPE_NTF_DataID_WRN_PRT_26 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt26 )  },	// ステープルエンプティ3.
		{ ATTENTION_ElevateTrayNotLiftUp		,     TYPE_NTF_DataID_WRN_PRT_27 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt27 )  },	// エレベートトレイ上昇未完.
		{ ATTENTION_PunchScrapsFullUser			,     TYPE_NTF_DataID_WRN_PRT_28 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt28 )  },	// パンチくずフル(ユーザー交換時).
		{ ATTENTION_PunchScrapsFullService		,     TYPE_NTF_DataID_WRN_PRT_29 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt29 )  },	// パンチくずフル(サービス交換時).
// 2016. 5.13 [ZeusSBK/Icarus] MZV-171 消耗品のライフ警告非表示対応 Start.
//		{ ATTENTION_TransRollerUnitNearLife		,     TYPE_NTF_DataID_WRN_PRT_30 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt30 )  },	// 転写ローラーユニットニアライフ.
//		{ ATTENTION_OzonFilterNearLife			,     TYPE_NTF_DataID_WRN_PRT_31 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt31 )  },	// 紙紛除去クリーナー／オゾンフィルタニアライフ.
//		{ ATTENTION_SmokeFilterUnitLife			,     TYPE_NTF_DataID_WRN_PRT_32 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt32 )  },	// 噴煙フィルタニアライフ.
// 2016. 5.13 [ZeusSBK/Icarus] MZV-171 消耗品のライフ警告非表示対応 End.
		{ ATTENTION_TransBeltUnitNearLife		,     TYPE_NTF_DataID_WRN_PRT_33 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt33 )  },	// 転写ベルトユニットニアライフ.
		{ ATTENTION_PMCounterOver				,     TYPE_NTF_DataID_WRN_PRT_34 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt34 )  },	// ＰＭカウンタオーバー(ATTENTION).
		{ ATTENTION_PrinterDoorOpen				,     TYPE_NTF_DataID_WRN_PRT_35 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt35 )  },	// プリンタセット不良(ATTENTION).
		{ ERROR_OHPPaperLengthFeedDisable		,     TYPE_NTF_DataID_WRN_PRT_40 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt40 )  },	// ＯＨＰ縦用紙セット禁止警告.
		{ ERROR_PrinterJam						,     TYPE_NTF_DataID_WRN_PRT_41 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt41 )  },	// プリンタジャム検出.
		{ ERROR_CTonerEmptyStop					,     TYPE_NTF_DataID_WRN_PRT_42 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt42 )  },	// シアン色トナーエンプティストップ.
		{ ERROR_MTonerEmptyStop					,     TYPE_NTF_DataID_WRN_PRT_43 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt43 )  },	// マゼンダ色トナーエンプティストップ.
		{ ERROR_YTonerEmptyStop					,     TYPE_NTF_DataID_WRN_PRT_44 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt44 )  },	// イエロー色トナーエンプティストップ.
		{ ERROR_KTonerEmptyStop					,     TYPE_NTF_DataID_WRN_PRT_45 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt45 )  },	// ブラック色トナーエンプティストップ.
		{ ERROR_PaperSizeChanged				,     TYPE_NTF_DataID_WRN_PRT_46 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt46 )  },	// 用紙サイズ不一致.
		{ ERROR_PaperKindChanged				,     TYPE_NTF_DataID_WRN_PRT_47 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt47 )  },	// 用紙種類不一致.
		{ ERROR_TrayOpen						,     TYPE_NTF_DataID_WRN_PRT_48 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt48 )  },	// 給紙トレイセット不良.
		{ ERROR_PrinterDoorOpen					,     TYPE_NTF_DataID_WRN_PRT_49 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt49 )  },	// プリンタセット不良.
		{ ERROR_UsedTonerBottleUnset			,     TYPE_NTF_DataID_WRN_PRT_50 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt50 )  },	// 廃トナーボトル（廃棄トナーボトル）セット不良.
		{ ERROR_TonerBottleUnsetC				,     TYPE_NTF_DataID_WRN_PRT_51 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt51 )  },	// トナーボトルセット不良・シアン.
		{ ERROR_TonerBottleUnsetM				,     TYPE_NTF_DataID_WRN_PRT_52 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt52 )  },	// トナーボトルセット不良・マゼンダ.
		{ ERROR_TonerBottleUnsetY				,     TYPE_NTF_DataID_WRN_PRT_53 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt53 )  },	// トナーボトルセット不良・イエロー.
		{ ERROR_TonerBottleUnsetK				,     TYPE_NTF_DataID_WRN_PRT_54 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt54 )  },	// トナーボトルセット不良・ブラック.
		{ ERROR_IULifeC							,     TYPE_NTF_DataID_WRN_PRT_55 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt55 )  },	// ＩＵライフ・シアン.
		{ ERROR_IULifeM							,     TYPE_NTF_DataID_WRN_PRT_56 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt56 )  },	// ＩＵライフ・マゼンダ.
		{ ERROR_IULifeY							,     TYPE_NTF_DataID_WRN_PRT_57 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt57 )  },	// ＩＵライフ・イエロー.
		{ ERROR_IULifeK							,     TYPE_NTF_DataID_WRN_PRT_58 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt58 )  },	// ＩＵライフ・ブラック.
		{ ERROR_DCLifeC							,     TYPE_NTF_DataID_WRN_PRT_59 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt59 )  },	// ＤＣライフ・シアン.
		{ ERROR_DCLifeM							,     TYPE_NTF_DataID_WRN_PRT_60 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt60 )  },	// ＤＣライフ・マゼンダ.
		{ ERROR_DCLifeY							,     TYPE_NTF_DataID_WRN_PRT_61 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt61 )  },	// ＤＣライフ・イエロー.
		{ ERROR_DCLifeK							,     TYPE_NTF_DataID_WRN_PRT_62 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt62 )  },	// ＤＣライフ・ブラック.
		{ ERROR_DevCLifeC						,     TYPE_NTF_DataID_WRN_PRT_63 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt63 )  },	// ＤｅｖＣライフ・シアン.
		{ ERROR_DevCLifeM						,     TYPE_NTF_DataID_WRN_PRT_64 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt64 )  },	// ＤｅｖＣライフ・マゼンダ.
		{ ERROR_DevCLifeY						,     TYPE_NTF_DataID_WRN_PRT_65 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt65 )  },	// ＤｅｖＣライフ・イエロー.
		{ ERROR_DevCLifeK						,     TYPE_NTF_DataID_WRN_PRT_66 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt66 )  },	// ＤｅｖＣライフ・ブラック.
		{ ERROR_TransCleanerUnitLife			,     TYPE_NTF_DataID_WRN_PRT_67 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt67 )  },	// 転写クリーナーユニットライフ.
		{ ERROR_FuserUnitLife					,     TYPE_NTF_DataID_WRN_PRT_68 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt68 )  },	// 定着ユニットライフ.
		{ ERROR_IUUnsetC						,     TYPE_NTF_DataID_WRN_PRT_69 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt69 )  },	// ＩＵセット不良・シアン.
		{ ERROR_IUUnsetM						,     TYPE_NTF_DataID_WRN_PRT_70 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt70 )  },	// ＩＵセット不良・マゼンダ.
		{ ERROR_IUUnsetY						,     TYPE_NTF_DataID_WRN_PRT_71 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt71 )  },	// ＩＵセット不良・イエロー.
		{ ERROR_IUUnsetK						,     TYPE_NTF_DataID_WRN_PRT_72 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt72 )  },	// ＩＵセット不良・ブラック.
		{ ERROR_DCUnsetC						,     TYPE_NTF_DataID_WRN_PRT_73 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt73 )  },	// ＤＣセット不良・シアン.
		{ ERROR_DCUnsetM						,     TYPE_NTF_DataID_WRN_PRT_74 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt74 )  },	// ＤＣセット不良・マゼンダ.
		{ ERROR_DCUnsetY						,     TYPE_NTF_DataID_WRN_PRT_75 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt75 )  },	// ＤＣセット不良・イエロー.
		{ ERROR_DCUnsetK						,     TYPE_NTF_DataID_WRN_PRT_76 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt76 )  },	// ＤＣセット不良・ブラック.
		{ ERROR_DevCUnsetC						,     TYPE_NTF_DataID_WRN_PRT_77 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt77 )  },	// ＤｅｖＣセット不良・シアン.
		{ ERROR_DevCUnsetM						,     TYPE_NTF_DataID_WRN_PRT_78 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt78 )  },	// ＤｅｖＣセット不良・マゼンダ.
		{ ERROR_DevCUnsetY						,     TYPE_NTF_DataID_WRN_PRT_79 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt79 )  },	// ＤｅｖＣセット不良・イエロー.
		{ ERROR_DevCUnsetK						,     TYPE_NTF_DataID_WRN_PRT_80 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt80 )  },	// ＤｅｖＣセット不良・ブラック.
		{ ERROR_IUMissetC						,     TYPE_NTF_DataID_WRN_PRT_81 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt81 )  },	// ＩＵ誤装着・シアン.
		{ ERROR_IUMissetM						,     TYPE_NTF_DataID_WRN_PRT_82 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt82 )  },	// ＩＵ誤装着・マゼンダ.
		{ ERROR_IUMissetY						,     TYPE_NTF_DataID_WRN_PRT_83 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt83 )  },	// ＩＵ誤装着・イエロー.
		{ ERROR_IUMissetK						,     TYPE_NTF_DataID_WRN_PRT_84 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt84 )  },	// ＩＵ誤装着・ブラック.
		{ ERROR_DCMissetC						,     TYPE_NTF_DataID_WRN_PRT_85 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt85 )  },	// ＤＣ誤装着・シアン.
		{ ERROR_DCMissetM						,     TYPE_NTF_DataID_WRN_PRT_86 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt86 )  },	// ＤＣ誤装着・マゼンダ.
		{ ERROR_DCMissetY						,     TYPE_NTF_DataID_WRN_PRT_87 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt87 )  },	// ＤＣ誤装着・イエロー.
		{ ERROR_DCMissetK						,     TYPE_NTF_DataID_WRN_PRT_88 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt88 )  },	// ＤＣ誤装着・ブラック.
		{ ERROR_DevCMissetC						,     TYPE_NTF_DataID_WRN_PRT_89 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt89 )  },	// ＤｅｖＣ誤装着・シアン.
		{ ERROR_DevCMissetM						,     TYPE_NTF_DataID_WRN_PRT_90 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt90 )  },	// ＤｅｖＣ誤装着・マゼンダ.
		{ ERROR_DevCMissetY						,     TYPE_NTF_DataID_WRN_PRT_91 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt91 )  },	// ＤｅｖＣ誤装着・イエロー.
		{ ERROR_DevCMissetK						,     TYPE_NTF_DataID_WRN_PRT_92 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt92 )  },	// ＤｅｖＣ誤装着・ブラック.
		{ ERROR_TransBeltUnitUnset				,     TYPE_NTF_DataID_WRN_PRT_93 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt93 )  },	// 転写ベルトユニットセット不良.
		{ ERROR_FuserUnitUnset					,     TYPE_NTF_DataID_WRN_PRT_94 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt94 )  },	// 定着ユニットセット不良.
		{ ERROR_UsedTonerBottleFull				,     TYPE_NTF_DataID_WRN_PRT_95 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt95 )  },	// 廃トナーフル.
		{ ERROR_RemovePaperOnFinisherTray		,     TYPE_NTF_DataID_WRN_PRT_96 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt96 )  },	// 処理トレイ用紙取り忘れ.
// 2016. 5.13 [ZeusSBK/Icarus] MZV-171 消耗品のライフ警告非表示対応 Start.
//		{ ERROR_TransRollerUnitLifeUser			,     TYPE_NTF_DataID_WRN_PRT_97 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt97 )  },	// 転写ローラーユニットライフ(ユーザー交換時).
//		{ ERROR_TransRollerUnitLifeService		,     TYPE_NTF_DataID_WRN_PRT_98 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt98 )  },	// 転写ローラーユニットライフ(サービス交換時).
//		{ ERROR_OzonFilterLifeUser				,     TYPE_NTF_DataID_WRN_PRT_99 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt99 )  },	// 紙紛除去クリーナー／オゾンフィルタライフ(ユーザー交換時).
//		{ ERROR_OzonFilterLifeService			,     TYPE_NTF_DataID_WRN_PRT_100, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt100 ) },	// 紙紛除去クリーナー／オゾンフィルタライフ(サービス交換時).
//		{ ERROR_SmokeFilterUnitLife				,     TYPE_NTF_DataID_WRN_PRT_101, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt101 ) },	// 噴煙フィルタライフ.
// 2016. 5.13 [ZeusSBK/Icarus] MZV-171 消耗品のライフ警告非表示対応 End.
		{ ERROR_TransBeltUnitLife				,     TYPE_NTF_DataID_WRN_PRT_102, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt102 ) },	// 転写ベルトユニットライフ.
		{ ERROR_PMCounterOver					,     TYPE_NTF_DataID_WRN_PRT_103, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt103 ) },	// ＰＭカウンタオーバー.
		{ ERROR_PaperSizeNotDetectAtTrayManual	,     TYPE_NTF_DataID_WRN_PRT_104, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt104 ) },	// 手差しトレイのサイズ検出出来ず.
		{ ERROR_SpecificOriginalDetect			,     TYPE_NTF_DataID_WRN_PRT_105, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt105 ) },	// 特定原稿検出.
		{ ERROR_TotalCounterUnset				,     TYPE_NTF_DataID_WRN_PRT_106, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt106 ) },	// トータルカウンタセット不良.
		{ ERROR_BinOverStartup					,     TYPE_NTF_DataID_WRN_PRT_107, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt107 ) },	// 起動時ビン／トレイ容量オーバー.
		{ ERROR_TCMissetC						,     TYPE_NTF_DataID_WRN_PRT_108, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt108 ) },	// ＴＣ誤装着・シアン.
		{ ERROR_TCMissetM						,     TYPE_NTF_DataID_WRN_PRT_109, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt109 ) },	// ＴＣ誤装着・マゼンダ.
		{ ERROR_TCMissetY						,     TYPE_NTF_DataID_WRN_PRT_110, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt110 ) },	// ＴＣ誤装着・イエロー.
		{ ERROR_TCMissetK						,     TYPE_NTF_DataID_WRN_PRT_111, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt111 ) },	// ＴＣ誤装着・ブラック.
		{ ERROR_LetterHeadPaperError			,     TYPE_NTF_DataID_WRN_PRT_112, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt112 ) },	// レターヘッド紙小サイズ禁止警告.
		{ ATTENTION_TCUnsetC					,     TYPE_NTF_DataID_WRN_PRT_113, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt113 ) },	// トナーカートリッジセット不良・シアン.
		{ ATTENTION_TCUnsetM					,     TYPE_NTF_DataID_WRN_PRT_114, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt114 ) },	// トナーカートリッジセット不良・マゼンダ.
		{ ATTENTION_TCUnsetY					,     TYPE_NTF_DataID_WRN_PRT_115, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt115 ) },	// トナーカートリッジセット不良・イエロー.
		{ ATTENTION_TCUnsetK					,     TYPE_NTF_DataID_WRN_PRT_116, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt116 ) },	// トナーカートリッジセット不良・ブラック.
		{ ATTENTION_ZUnitPunchScrapsFullUser	,     TYPE_NTF_DataID_WRN_PRT_117, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt117 ) },	// Z折りユニットパンチくずフル(ユーザー交換時).
		{ ATTENTION_ZUnitPunchScrapsFullService	,     TYPE_NTF_DataID_WRN_PRT_118, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt118 ) },	// Z折りユニットパンチくずフル(サービス交換時).
		{ ATTENTION_TCMissetC					,     TYPE_NTF_DataID_WRN_PRT_119, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt119 ) },	// ＴＣ誤装着・シアン（ＡＴＴＥＮＴＩＯＮレベル）.
		{ ATTENTION_TCMissetM					,     TYPE_NTF_DataID_WRN_PRT_120, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt120 ) },	// ＴＣ誤装着・マゼンダ（ＡＴＴＥＮＴＩＯＮレベル）.
		{ ATTENTION_TCMissetY					,     TYPE_NTF_DataID_WRN_PRT_121, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt121 ) },	// ＴＣ誤装着・イエロー（ＡＴＴＥＮＴＩＯＮレベル）.
		{ ATTENTION_TCMissetK					,     TYPE_NTF_DataID_WRN_PRT_122, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt122 ) },	// ＴＣ誤装着・ブラック（ＡＴＴＥＮＴＩＯＮレベル）.
		{ ATTENTION_IUbeforeNearLifeC			,     TYPE_NTF_DataID_WRN_PRT_123, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt123 ) },	// ＩＵライフあり（事前警告）・シアン.
		{ ATTENTION_IUbeforeNearLifeM			,     TYPE_NTF_DataID_WRN_PRT_124, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt124 ) },	// ＩＵライフあり（事前警告）・マゼンダ.
		{ ATTENTION_IUbeforeNearLifeY			,     TYPE_NTF_DataID_WRN_PRT_125, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt125 ) },	// ＩＵライフあり（事前警告）・イエロー.
		{ ATTENTION_IUbeforeNearLifeK			,     TYPE_NTF_DataID_WRN_PRT_126, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt126 ) },	// ＩＵライフあり（事前警告）・ブラック.
		{ ATTENTION_DevCMaterialNearLifeK		,     TYPE_NTF_DataID_WRN_PRT_127, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt127 ) },	// 現像剤ニアライフ・ブラック.
		{ ERROR_DevCMaterialLifeK				,     TYPE_NTF_DataID_WRN_PRT_128, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt128 ) },	// 現像剤ライフ・ブラック.
// ADS坪井 → 2011/12/01 【パフォーマンス改善】isWarning対応 Start.
		{ ERROR_ThinPaperError					,     TYPE_NTF_DataID_WRN_PRT_129, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt129 ) },	// 薄紙小サイズ用紙セット禁止警告.
		{ ERROR_PaperSizeChangedManual			,     TYPE_NTF_DataID_WRN_PRT_130, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt130 ) },	// 手差しサイズエラー警告.
		{ ERROR_PaperSizeChangedForceStop		,     TYPE_NTF_DataID_WRN_PRT_131, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt131 ) },	// 強制停止サイズエラー警告.
		{ ERROR_FuserPlugComesOff				,     TYPE_NTF_DataID_WRN_PRT_132, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt132 ) },	// 定着プラグ抜け警告.
// ADS坪井 → 2011/12/01 【パフォーマンス改善】isWarning対応 End.

// 2016. 5.13 [ZeusSBK/Icarus] MZV-171 消耗品のライフ警告非表示対応 Start.
//		{ ATTENTION_PaperDustRemovalCleanerNearLife,  TYPE_NTF_DataID_WRN_PRT_164, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt164 ) },	// 紙粉除去クリーナニアライフ.
//		{ ERROR_PaperDustRemovalCleanerLife		,     TYPE_NTF_DataID_WRN_PRT_165, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt165 ) },	// 紙粉除去クリーナライフ.
// 2016. 5.13 [ZeusSBK/Icarus] MZV-171 消耗品のライフ警告非表示対応 End.

#ifdef DEF_A3A4COMM
//Str【A3A4共通化FUM  ユニット装着情報１共通化: M.Saitou : 2014.12.22】.
		{ ERROR_TCNewTroubleC					,     TYPE_NTF_DataID_WRN_PRT_173, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt173 ) },	// ＴＣ新品解除不良・シアン.
		{ ERROR_TCNewTroubleM					,     TYPE_NTF_DataID_WRN_PRT_174, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt174 ) },	// ＴＣ新品解除不良・マゼンダ.
		{ ERROR_TCNewTroubleY					,     TYPE_NTF_DataID_WRN_PRT_175, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt175 ) },	// ＴＣ新品解除不良・イエロー.
		{ ERROR_TCNewTroubleK					,     TYPE_NTF_DataID_WRN_PRT_176, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt176 ) },	// ＴＣ新品解除不良・ブラック.
		{ ATTENTION_TCNewTroubleC				,     TYPE_NTF_DataID_WRN_PRT_177, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt177 ) },	// ＴＣ新品解除不良・シアン（ＡＴＴＥＮＴＩＯＮレベル）.
		{ ATTENTION_TCNewTroubleM				,     TYPE_NTF_DataID_WRN_PRT_178, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt178 ) },	// ＴＣ新品解除不良・マゼンダ（ＡＴＴＥＮＴＩＯＮレベル）.
		{ ATTENTION_TCNewTroubleY				,     TYPE_NTF_DataID_WRN_PRT_179, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt179 ) },	// ＴＣ新品解除不良・イエロー（ＡＴＴＥＮＴＩＯＮレベル）.
		{ ATTENTION_TCNewTroubleK				,     TYPE_NTF_DataID_WRN_PRT_180, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt180 ) },	// ＴＣ新品解除不良・ブラック（ＡＴＴＥＮＴＩＯＮレベル）.
//End【A3A4共通化FUM  ユニット装着情報１共通化: M.Saitou : 2014.12.22】.
#endif // DEF_A3A4COMM

#ifdef DEF_A4_Cus
		{ ERROR_IUNewTroubleC					,     TYPE_NTF_DataID_WRN_PRT_181, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt181 ) },	// ＩＵ新品解除不良・シアン.
		{ ERROR_IUNewTroubleM					,     TYPE_NTF_DataID_WRN_PRT_182, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt182 ) },	// ＩＵ新品解除不良・マゼンダ.
		{ ERROR_IUNewTroubleY					,     TYPE_NTF_DataID_WRN_PRT_183, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt183 ) },	// ＩＵ新品解除不良・イエロー.
		{ ERROR_IUNewTroubleK					,     TYPE_NTF_DataID_WRN_PRT_184, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt184 ) },	// ＩＵ新品解除不良・ブラック.
		{ ATTENTION_IUNewTroubleC				,     TYPE_NTF_DataID_WRN_PRT_185, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt185 ) },	// ＩＵ新品解除不良・シアン（ＡＴＴＥＮＴＩＯＮレベル）.
		{ ATTENTION_IUNewTroubleM				,     TYPE_NTF_DataID_WRN_PRT_186, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt186 ) },	// ＩＵ新品解除不良・マゼンダ（ＡＴＴＥＮＴＩＯＮレベル）.
		{ ATTENTION_IUNewTroubleY				,     TYPE_NTF_DataID_WRN_PRT_187, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt187 ) },	// ＩＵ新品解除不良・イエロー（ＡＴＴＥＮＴＩＯＮレベル）.
		{ ATTENTION_IUNewTroubleK				,     TYPE_NTF_DataID_WRN_PRT_188, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt188 ) },	// ＩＵ新品解除不良・ブラック（ＡＴＴＥＮＴＩＯＮレベル）.
//AR1062444 EagleL_トナーボトルエラー対応 ST.
#endif	/* DEF_A4_Cus */
		{ ERROR_TCUnsetC						,     TYPE_NTF_DataID_WRN_PRT_189, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt189 ) },	// トナーカートリッジセット不良エラー・シアン.
		{ ERROR_TCUnsetM						,     TYPE_NTF_DataID_WRN_PRT_190, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt190 ) },	// トナーカートリッジセット不良エラー・マゼンタ.
		{ ERROR_TCUnsetY						,     TYPE_NTF_DataID_WRN_PRT_191, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt191 ) },	// トナーカートリッジセット不良エラー・イエロー.
		{ ERROR_TCUnsetK						,     TYPE_NTF_DataID_WRN_PRT_192, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt192 ) },	// トナーカートリッジセット不良エラー・ブラック.
//#endif	/* DEF_A4_Cus */
//AR1062444 EagleL_トナーボトルエラー対応 ED.
//Str 【KronosS WBS_16_26 KronosS警告追加対応 : S.Itoh 2016.10.31】.
#if defined(DEF_A4_Cus) && defined(DEF_KRONOSS_Cus)
		{ ERROR_MaintenanceKitLife				,     TYPE_NTF_DataID_WRN_PRT_193 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt193 )  },	// メンテナンスキットライフ.
		{ ATTENTION_MaintenanceKitNearLife		,     TYPE_NTF_DataID_WRN_PRT_194 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt194 )  },	// メンテナンスキットニアライフ.
// 2017/05/16 APF AR.1018782NG Start.
////Str 【KronosS AR1012883_メンテナンスキットライフパネル表示用 : S.Itoh 2017.02.20】.
//		{ ERROR_MaintenanceKitLifeForPNL		,     TYPE_NTF_DataID_WRN_PRT_195 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt195 )  },	// メンテナンスキットライフ.
//		{ ATTENTION_MaintenanceKitNearLifeForPNL,     TYPE_NTF_DataID_WRN_PRT_196 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt196 )  },	// メンテナンスキットニアライフ.
////End 【KronosS AR1012883_メンテナンスキットライフパネル表示用 : S.Itoh 2017.02.20】.
// 2017/05/16 APF AR.1018782NG End.
#endif    // #if defined(DEF_A4_Cus) && defined(DEF_KRONOSS_Cus)
//End 【KronosS WBS_16_26 KronosS警告追加対応 : S.Itoh 2016.10.31】.
// 2017/08/17 APF 機器構成変更No2 LCTシフト異常の検出 Start.
		{ ATTENTION_LCCPaperAbnormalSet			,     TYPE_NTF_DataID_WRN_PRT_197,	&( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt197 )	},	// 移載LCT用紙異常セット警告（ＡＴＴＥＮＴＩＯＮレベル）.
// 2017/08/17 APF 機器構成変更No2 LCTシフト異常の検出 End.
// 2017/08/17 APF RQ-693 JAM発生時の処理方法改善 Start.
		{ ERROR_TrayAbnormalSet					,     TYPE_NTF_DataID_WRN_PRT_198,	&( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt198 )	},	// トレイ異常セット状態.
// 2017/08/17 APF RQ-693 JAM発生時の処理方法改善 End.
// 2017/09/01 APF 制御構成変更No34 トレイオープン時の警告表示改善 Start.
#if defined(DEF_A4_Cus) && defined(DEF_KRONOSS_Cus)
		{ ERROR_FeedTrayUnset2					,     TYPE_NTF_DataID_WRN_PRT_199,	&( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt199 )	},	// トレイセット不良２.
#endif // #if defined(DEF_A4_Cus) && defined(DEF_KRONOSS_Cus).
// 2017/09/01 APF 制御構成変更No34 トレイオープン時の警告表示改善 End.

// 【RQ-747 VirusScanFUM for FVT2】 2018.04.23 Sky.A.Tamura Add Start ->.
// 【RQ-747 VirusScanFUM for 機種】 start.
//		{ ATTENTION_VirusDetection				,     TYPE_NTF_DataID_WRN_PRT_200,	&( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt200 )	},	// ウイルス検知警告（Attention）.
// 【RQ-747 VirusScanFUM for 機種】 end.
// 【RQ-747 VirusScanFUM for FVT2】 2018.04.23 Sky.A.Tamura Add End   <-.

// 【RQ-747 VirusScanFUM for FVT2】 2018.05.23 Sky.K.Ogawa Add Start ->.
// 【RQ-747 VirusScanFUM for 機種】 start.
//		{ ATTENTION_PatternFileAvailability		,     TYPE_NTF_DataID_WRN_PRT_201,	&( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt201 )	},	// ウイルス検知警告（Attention）.
// 【RQ-747 VirusScanFUM for 機種】 end.
// 【RQ-747 VirusScanFUM for FVT2】 2018.05.23 Sky.K.Ogawa Add End   <-.
// 2018/09/28 APF 機器構成変更_No7_FS-540_ステープルユニット変更 start.
		{ ATTENTION_StapleScrapsFullUser			,     TYPE_NTF_DataID_WRN_PRT_200,	&( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt200 )	},	// ステープルくずフル（ユーザー交換時）
		{ ATTENTION_StapleScrapsFullService			,     TYPE_NTF_DataID_WRN_PRT_201,	&( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt201 )	},	// ステープルくずフル（サーッビス交換時）
		{ ATTENTION_StapleNearEmpty					,     TYPE_NTF_DataID_WRN_PRT_202,	&( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt202 )	},	// ステープルニアエンプティ
// 2018/09/28 APF 機器構成変更_No7_FS-540_ステープルユニット変更 end.
// 2019/03/22 APF RQ-720_Add Start.
		{ ATTENTION_NotSupportMediaTypeManual		,     TYPE_NTF_DataID_WRN_PRT_203,	&( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt203 )	},	// 手差トレイ非サポート紙種セット警告.
		{ ATTENTION_NotSupportMediaTypeTray1		,     TYPE_NTF_DataID_WRN_PRT_204,	&( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt204 )	},	// 給紙トレイ1非サポート紙種セット警告.
		{ ATTENTION_NotSupportMediaTypeTray2		,     TYPE_NTF_DataID_WRN_PRT_205,	&( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt205 )	},	// 給紙トレイ2非サポート紙種セット警告.
		{ ATTENTION_NotSupportMediaTypeTray3		,     TYPE_NTF_DataID_WRN_PRT_206,	&( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt206 )	},	// 給紙トレイ3非サポート紙種セット警告.
		{ ATTENTION_NotSupportMediaTypeTray4		,     TYPE_NTF_DataID_WRN_PRT_207,	&( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt207 )	},	// 給紙トレイ4非サポート紙種セット警告.
		{ ATTENTION_NotSupportMediaTypeLCT			,     TYPE_NTF_DataID_WRN_PRT_208,	&( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt208 )	},	// 外付けLCT非サポート紙種セット警告.
// 2019/03/22 APF RQ-720_Add End.

		{ (TYPE_PrinterWarning)0				,     TYPE_NTF_DataID_End        , NULL						   												}	// end.
	};
	SSTS_ChangedInfo stChangedInfo;
	Bool b_Val;
	
	for( ST_CHK_WARNING *pst_WarTbl = &st_CheckPrtWarningTable[ 0 ]; pst_WarTbl->e_DataID != TYPE_NTF_DataID_End; pst_WarTbl++ ){
		b_Val = APIC_PrinterWarning::newInstance()->isWarning( pst_WarTbl->e_Prt );
		if( *( pst_WarTbl->pb_Val ) != b_Val ){
			*( pst_WarTbl->pb_Val ) = b_Val;
			stChangedInfo.e_DataID = pst_WarTbl->e_DataID;
			stChangedInfo.p_Value  = pst_WarTbl->pb_Val;
			// 2021/10/22 APF OP_BTS-35403 start
			//m_vChangedInfo.push_back( stChangedInfo );
			AddChangedInfo( &stChangedInfo );
			// 2021/10/22 APF OP_BTS-35403 end
		}
	}

}

//************************************************************************************************************************.
//概要　　：Prt警告情報取得処理 (ＤＣユーザー交換超過警告).
//戻り値　：無し.
//備考　　：特に無し.
//作成者　：高橋.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
// 2012. 5.31 [CQ363516] パフォーマンス改善 高周期のポーリング処理で、各条件に該当する場合に変化点検出処理を実行する.
//************************************************************************************************************************.
void SSTC_SystemStatusService::getPrtWarningStatus_Opt_DCUserExchange()
{
	SST_LOG(MFPE_LogLevelAll);
	
	typedef struct st_CheckWarning {
		TYPE_PrinterWarning		e_Prt;					// PRT警告情報.
		TYPE_NTF_DataID				e_DataID;				// データID.
		Bool 				   	   *pb_Val;					// 警告の有無.
	}ST_CHK_WARNING;
	static ST_CHK_WARNING st_CheckPrtWarningTable[] = {
		{ ATTENTION_DCUserExchangeExcessC		,     TYPE_NTF_DataID_WRN_PRT_36 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt36 )  },	// ＤＣユーザー交換超過警告・シアン（Amurのみ).
		{ ATTENTION_DCUserExchangeExcessM		,     TYPE_NTF_DataID_WRN_PRT_37 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt37 )  },	// ＤＣユーザー交換超過警告・マゼンダ（Amurのみ).
		{ ATTENTION_DCUserExchangeExcessY		,     TYPE_NTF_DataID_WRN_PRT_38 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt38 )  },	// ＤＣユーザー交換超過警告・イエロー（Amurのみ).
		{ ATTENTION_DCUserExchangeExcessK		,     TYPE_NTF_DataID_WRN_PRT_39 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt39 )  },	// ＤＣユーザー交換超過警告・ブラック（Amurのみ).
		{ (TYPE_PrinterWarning)0				,     TYPE_NTF_DataID_End        , NULL						   												}	// end.
	};
	SSTS_ChangedInfo stChangedInfo;
	Bool b_Val;
	
	for( ST_CHK_WARNING *pst_WarTbl = &st_CheckPrtWarningTable[ 0 ]; pst_WarTbl->e_DataID != TYPE_NTF_DataID_End; pst_WarTbl++ ){
		b_Val = APIC_PrinterWarning::newInstance()->isWarning( pst_WarTbl->e_Prt );
		if( *( pst_WarTbl->pb_Val ) != b_Val ){
			*( pst_WarTbl->pb_Val ) = b_Val;
			stChangedInfo.e_DataID = pst_WarTbl->e_DataID;
			stChangedInfo.p_Value  = pst_WarTbl->pb_Val;
			// 2021/10/22 APF OP_BTS-35403 start
			//m_vChangedInfo.push_back( stChangedInfo );
			AddChangedInfo( &stChangedInfo );
			// 2021/10/22 APF OP_BTS-35403 end
		}
	}

}

//************************************************************************************************************************.
//概要　　：Prt警告情報取得処理 (パネル警告表示用:IU/DCユニット).
//戻り値　：無し.
//備考　　：特に無し.
//作成者　：高橋.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
// 2012. 5.31 [CQ363516] パフォーマンス改善 高周期のポーリング処理で、各条件に該当する場合に変化点検出処理を実行する.
//************************************************************************************************************************.
void SSTC_SystemStatusService::getPrtWarningStatus_Opt_pnl_IcDc()
{
	SST_LOG(MFPE_LogLevelAll);
	
	typedef struct st_CheckWarning {
		TYPE_PrinterWarning		e_Prt;					// PRT警告情報.
		TYPE_NTF_DataID				e_DataID;				// データID.
		Bool 				   	   *pb_Val;					// 警告の有無.
	}ST_CHK_WARNING;
	static ST_CHK_WARNING st_CheckPrtWarningTable[] = {
		{ ATTENTION_IUNearLifeForPNLC			,     TYPE_NTF_DataID_WRN_PRT_133, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt133 ) },	// Ｃ ＩＵニアライフ警告検出（パネル確認用）.
		{ ATTENTION_IUNearLifeForPNLM			,     TYPE_NTF_DataID_WRN_PRT_134, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt134 ) },	// Ｍ ＩＵニアライフ警告検出（パネル確認用）.
		{ ATTENTION_IUNearLifeForPNLY			,     TYPE_NTF_DataID_WRN_PRT_135, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt135 ) },	// Ｙ ＩＵニアライフ警告検出（パネル確認用）.
		{ ATTENTION_IUNearLifeForPNLK			,     TYPE_NTF_DataID_WRN_PRT_136, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt136 ) },	// Ｋ ＩＵニアライフ警告検出（パネル確認用）.
		{ ERROR_IULifeForPNLC					,     TYPE_NTF_DataID_WRN_PRT_137, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt137 ) },	// Ｃ ＩＵライフストップ警告検出（パネル確認用）.
		{ ERROR_IULifeForPNLM					,     TYPE_NTF_DataID_WRN_PRT_138, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt138 ) },	// Ｍ ＩＵライフストップ警告検出（パネル確認用）.
		{ ERROR_IULifeForPNLY					,     TYPE_NTF_DataID_WRN_PRT_139, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt139 ) },	// Ｙ ＩＵライフストップ警告検出（パネル確認用）.
		{ ERROR_IULifeForPNLK					,     TYPE_NTF_DataID_WRN_PRT_140, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt140 ) },	// Ｋ ＩＵライフストップ警告検出（パネル確認用）.
		{ ATTENTION_DCNearLifeForPNLC			,     TYPE_NTF_DataID_WRN_PRT_141, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt141 ) },	// Ｃ ＤＣニアライフ警告検出（パネル確認用）.
		{ ATTENTION_DCNearLifeForPNLM			,     TYPE_NTF_DataID_WRN_PRT_142, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt142 ) },	// Ｍ ＤＣニアライフ警告検出（パネル確認用）.
		{ ATTENTION_DCNearLifeForPNLY			,     TYPE_NTF_DataID_WRN_PRT_143, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt143 ) },	// Ｙ ＤＣニアライフ警告検出（パネル確認用）.
		{ ATTENTION_DCNearLifeForPNLK			,     TYPE_NTF_DataID_WRN_PRT_144, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt144 ) },	// Ｋ ＤＣニアライフ警告検出（パネル確認用）.
		{ ERROR_DCLifeForPNLC					,     TYPE_NTF_DataID_WRN_PRT_145, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt145 ) },	// Ｃ ＤＣライフストップ警告検出（パネル確認用）.
		{ ERROR_DCLifeForPNLM					,     TYPE_NTF_DataID_WRN_PRT_146, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt146 ) },	// Ｍ ＤＣライフストップ警告検出（パネル確認用）.
		{ ERROR_DCLifeForPNLY					,     TYPE_NTF_DataID_WRN_PRT_147, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt147 ) },	// Ｙ ＤＣライフストップ警告検出（パネル確認用）.
		{ ERROR_DCLifeForPNLK					,     TYPE_NTF_DataID_WRN_PRT_148, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt148 ) },	// Ｋ ＤＣライフストップ警告検出（パネル確認用）.
// 2014.12.23 ZeusS 機器構成変更No40 s.
		{ ATTENTION_IUPreNearLifeForPNLC		,     TYPE_NTF_DataID_WRN_PRT_166, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt166 ) },	// Ｃ ＩＵプレニアライフ警告検出（パネル確認用）.
		{ ATTENTION_IUPreNearLifeForPNLM		,     TYPE_NTF_DataID_WRN_PRT_167, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt167 ) },	// Ｍ ＩＵプレニアライフ警告検出（パネル確認用）.
		{ ATTENTION_IUPreNearLifeForPNLY		,     TYPE_NTF_DataID_WRN_PRT_168, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt168 ) },	// Ｙ ＩＵプレニアライフ警告検出（パネル確認用）.
		{ ATTENTION_DCPreNearLifeForPNLC		,     TYPE_NTF_DataID_WRN_PRT_169, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt169 ) },	// Ｃ ＤＣプレニアライフ警告検出（パネル確認用）.
		{ ATTENTION_DCPreNearLifeForPNLM		,     TYPE_NTF_DataID_WRN_PRT_170, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt170 ) },	// Ｍ ＤＣプレニアライフ警告検出（パネル確認用）.
		{ ATTENTION_DCPreNearLifeForPNLY		,     TYPE_NTF_DataID_WRN_PRT_171, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt171 ) },	// Ｙ ＤＣプレニアライフ警告検出（パネル確認用）.
		{ ATTENTION_DCPreNearLifeForPNLK		,     TYPE_NTF_DataID_WRN_PRT_172, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt172 ) },	// Ｋ ＤＣプレニアライフ警告検出（パネル確認用）.
// 2014.12.23 ZeusS 機器構成変更No40 e.
		{ (TYPE_PrinterWarning)0				,     TYPE_NTF_DataID_End        , NULL						   												}	// end.
	};
	SSTS_ChangedInfo stChangedInfo;
	Bool b_Val;
	
	for( ST_CHK_WARNING *pst_WarTbl = &st_CheckPrtWarningTable[ 0 ]; pst_WarTbl->e_DataID != TYPE_NTF_DataID_End; pst_WarTbl++ ){
		b_Val = APIC_PrinterWarning::newInstance()->isWarning( pst_WarTbl->e_Prt );
		if( *( pst_WarTbl->pb_Val ) != b_Val ){
			*( pst_WarTbl->pb_Val ) = b_Val;
			stChangedInfo.e_DataID = pst_WarTbl->e_DataID;
			stChangedInfo.p_Value  = pst_WarTbl->pb_Val;
			// 2021/10/22 APF OP_BTS-35403 start
			//m_vChangedInfo.push_back( stChangedInfo );
			AddChangedInfo( &stChangedInfo );
			// 2021/10/22 APF OP_BTS-35403 end
		}
	}

}

//************************************************************************************************************************.
//概要　　：Prt警告情報取得処理 (パネル警告表示用:現像ユニット).
//戻り値　：無し.
//備考　　：特に無し.
//作成者　：高橋.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
// 2012. 5.31 [CQ363516] パフォーマンス改善 高周期のポーリング処理で、各条件に該当する場合に変化点検出処理を実行する.
//************************************************************************************************************************.
void SSTC_SystemStatusService::getPrtWarningStatus_Opt_pnl_DevC()
{
	SST_LOG(MFPE_LogLevelAll);
	
	typedef struct st_CheckWarning {
		TYPE_PrinterWarning		e_Prt;					// PRT警告情報.
		TYPE_NTF_DataID				e_DataID;				// データID.
		Bool 				   	   *pb_Val;					// 警告の有無.
	}ST_CHK_WARNING;
	static ST_CHK_WARNING st_CheckPrtWarningTable[] = {
		{ ATTENTION_DevCNearLifeForPNLC			,     TYPE_NTF_DataID_WRN_PRT_149, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt149 ) },	// Ｃ 現像ユニットニアライフ警告検出（パネル確認用）.
		{ ATTENTION_DevCNearLifeForPNLM			,     TYPE_NTF_DataID_WRN_PRT_150, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt150 ) },	// Ｍ 現像ユニットニアライフ警告検出（パネル確認用）.
		{ ATTENTION_DevCNearLifeForPNLY			,     TYPE_NTF_DataID_WRN_PRT_151, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt151 ) },	// Ｙ 現像ユニットニアライフ警告検出（パネル確認用）.
		{ ATTENTION_DevCNearLifeForPNLK			,     TYPE_NTF_DataID_WRN_PRT_152, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt152 ) },	// Ｋ 現像ユニットニアライフ警告検出（パネル確認用）.
		{ ERROR_DevCLifeForPNLC					,     TYPE_NTF_DataID_WRN_PRT_153, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt153 ) },	// Ｃ 現像ユニットライフストップ警告検出（パネル確認用）.
		{ ERROR_DevCLifeForPNLM					,     TYPE_NTF_DataID_WRN_PRT_154, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt154 ) },	// Ｍ 現像ユニットライフストップ警告検出（パネル確認用）.
		{ ERROR_DevCLifeForPNLY					,     TYPE_NTF_DataID_WRN_PRT_155, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt155 ) },	// Ｙ 現像ユニットライフストップ警告検出（パネル確認用）.
		{ ERROR_DevCLifeForPNLK					,     TYPE_NTF_DataID_WRN_PRT_156, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt156 ) },	// Ｋ 現像ユニットライフストップ警告検出（パネル確認用）.
		{ (TYPE_PrinterWarning)0				,     TYPE_NTF_DataID_End        , NULL						   												}	// end.
	};
	SSTS_ChangedInfo stChangedInfo;
	Bool b_Val;
	
	for( ST_CHK_WARNING *pst_WarTbl = &st_CheckPrtWarningTable[ 0 ]; pst_WarTbl->e_DataID != TYPE_NTF_DataID_End; pst_WarTbl++ ){
		b_Val = APIC_PrinterWarning::newInstance()->isWarning( pst_WarTbl->e_Prt );
		if( *( pst_WarTbl->pb_Val ) != b_Val ){
			*( pst_WarTbl->pb_Val ) = b_Val;
			stChangedInfo.e_DataID = pst_WarTbl->e_DataID;
			stChangedInfo.p_Value  = pst_WarTbl->pb_Val;
			// 2021/10/22 APF OP_BTS-35403 start
			//m_vChangedInfo.push_back( stChangedInfo );
			AddChangedInfo( &stChangedInfo );
			// 2021/10/22 APF OP_BTS-35403 end
		}
	}

}

//************************************************************************************************************************.
//概要　　：Prt警告情報取得処理 (パネル警告表示用:転写ベルト（現像剤）).
//戻り値　：無し.
//備考　　：特に無し.
//作成者　：高橋.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
// 2012. 5.31 [CQ363516] パフォーマンス改善 高周期のポーリング処理で、各条件に該当する場合に変化点検出処理を実行する.
//************************************************************************************************************************.
void SSTC_SystemStatusService::getPrtWarningStatus_Opt_pnl_TransBelt()
{
	SST_LOG(MFPE_LogLevelAll);
	
	typedef struct st_CheckWarning {
		TYPE_PrinterWarning		e_Prt;					// PRT警告情報.
		TYPE_NTF_DataID				e_DataID;				// データID.
		Bool 				   	   *pb_Val;					// 警告の有無.
	}ST_CHK_WARNING;
	static ST_CHK_WARNING st_CheckPrtWarningTable[] = {
		{ ATTENTION_DevCMaterialNearLifeForPNLK	,     TYPE_NTF_DataID_WRN_PRT_157, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt157 ) },	// Ｋ 現像剤ニアライフ警告検出（パネル確認用）.
		{ ERROR_DevCMaterialLifeForPNLK			,     TYPE_NTF_DataID_WRN_PRT_158, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt158 ) },	// Ｋ 現像剤ライフストップ警告検出（パネル確認用）.
		{ ATTENTION_TransBeltUnitNearLifeForPNL	,     TYPE_NTF_DataID_WRN_PRT_161, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt161 ) },	// 転写ベルトニアライフ警告検出（パネル確認用）.
		{ ERROR_TransBeltUnitLifeForPNL			,     TYPE_NTF_DataID_WRN_PRT_162, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt162 ) },	// 転写ベルトライフストップ警告検出（パネル確認用）.
		{ (TYPE_PrinterWarning)0				,     TYPE_NTF_DataID_End        , NULL						   												}	// end.
	};
	SSTS_ChangedInfo stChangedInfo;
	Bool b_Val;
	
	for( ST_CHK_WARNING *pst_WarTbl = &st_CheckPrtWarningTable[ 0 ]; pst_WarTbl->e_DataID != TYPE_NTF_DataID_End; pst_WarTbl++ ){
		b_Val = APIC_PrinterWarning::newInstance()->isWarning( pst_WarTbl->e_Prt );
		if( *( pst_WarTbl->pb_Val ) != b_Val ){
			*( pst_WarTbl->pb_Val ) = b_Val;
			stChangedInfo.e_DataID = pst_WarTbl->e_DataID;
			stChangedInfo.p_Value  = pst_WarTbl->pb_Val;
			// 2021/10/22 APF OP_BTS-35403 start
			//m_vChangedInfo.push_back( stChangedInfo );
			AddChangedInfo( &stChangedInfo );
			// 2021/10/22 APF OP_BTS-35403 end
		}
	}

}

//************************************************************************************************************************.
//概要　　：Prt警告情報取得処理 (パネル警告表示用:定着ユニット).
//戻り値　：無し.
//備考　　：特に無し.
//作成者　：高橋.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
// 2012. 5.31 [CQ363516] パフォーマンス改善 高周期のポーリング処理で、各条件に該当する場合に変化点検出処理を実行する.
//************************************************************************************************************************.
void SSTC_SystemStatusService::getPrtWarningStatus_Opt_pnl_Fuser()
{
	SST_LOG(MFPE_LogLevelAll);
	
	typedef struct st_CheckWarning {
		TYPE_PrinterWarning		e_Prt;					// PRT警告情報.
		TYPE_NTF_DataID				e_DataID;				// データID.
		Bool 				   	   *pb_Val;					// 警告の有無.
	}ST_CHK_WARNING;
	static ST_CHK_WARNING st_CheckPrtWarningTable[] = {
		{ ATTENTION_FuserUnitNearLifeForPNL		,     TYPE_NTF_DataID_WRN_PRT_159, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt159 ) },	// 定着ユニットニアライフ警告検出（パネル確認用）.
		{ ERROR_FuserUnitLifeForPNL				,     TYPE_NTF_DataID_WRN_PRT_160, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt160 ) },	// 定着ユニットライフストップ警告検出（パネル確認用）.
		{ (TYPE_PrinterWarning)0				,     TYPE_NTF_DataID_End        , NULL						   												}	// end.
	};
	SSTS_ChangedInfo stChangedInfo;
	Bool b_Val;
	
	for( ST_CHK_WARNING *pst_WarTbl = &st_CheckPrtWarningTable[ 0 ]; pst_WarTbl->e_DataID != TYPE_NTF_DataID_End; pst_WarTbl++ ){
		b_Val = APIC_PrinterWarning::newInstance()->isWarning( pst_WarTbl->e_Prt );
		if( *( pst_WarTbl->pb_Val ) != b_Val ){
			*( pst_WarTbl->pb_Val ) = b_Val;
			stChangedInfo.e_DataID = pst_WarTbl->e_DataID;
			stChangedInfo.p_Value  = pst_WarTbl->pb_Val;
			// 2021/10/22 APF OP_BTS-35403 start
			//m_vChangedInfo.push_back( stChangedInfo );
			AddChangedInfo( &stChangedInfo );
			// 2021/10/22 APF OP_BTS-35403 end
		}
	}

}

//************************************************************************************************************************.
//概要　　：Prt警告情報取得処理 (パネル警告表示用:オゾンフィルター).
//戻り値　：無し.
//備考　　：特に無し.
//作成者　：西村.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
// 2016. 5.13 [ZeusSBK/Icarus] MZV-171 消耗品のライフ警告非表示対応.
//************************************************************************************************************************.
void SSTC_SystemStatusService::getPrtWarningStatus_Opt_pnl_OzonFilter()
{
	SST_LOG(MFPE_LogLevelAll);
	
	typedef struct st_CheckWarning {
		TYPE_PrinterWarning		e_Prt;					// PRT警告情報.
		TYPE_NTF_DataID				e_DataID;				// データID.
		Bool 				   	   *pb_Val;					// 警告の有無.
	}ST_CHK_WARNING;
	static ST_CHK_WARNING st_CheckPrtWarningTable[] = {
		{ ATTENTION_OzonFilterNearLife			,     TYPE_NTF_DataID_WRN_PRT_31 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt31 )  },	// 紙紛除去クリーナー／オゾンフィルタニアライフ.
		{ ERROR_OzonFilterLifeUser				,     TYPE_NTF_DataID_WRN_PRT_99 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt99 )  },	// 紙紛除去クリーナー／オゾンフィルタライフ(ユーザー交換時).
		{ ERROR_OzonFilterLifeService			,     TYPE_NTF_DataID_WRN_PRT_100, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt100 ) },	// 紙紛除去クリーナー／オゾンフィルタライフ(サービス交換時).
		{ (TYPE_PrinterWarning)0				,     TYPE_NTF_DataID_End        , NULL						   												}	// end.
	};
	SSTS_ChangedInfo stChangedInfo;
	Bool b_Val;
	
	for( ST_CHK_WARNING *pst_WarTbl = &st_CheckPrtWarningTable[ 0 ]; pst_WarTbl->e_DataID != TYPE_NTF_DataID_End; pst_WarTbl++ ){
		b_Val = APIC_PrinterWarning::newInstance()->isWarning( pst_WarTbl->e_Prt );
		if( *( pst_WarTbl->pb_Val ) != b_Val ){
			*( pst_WarTbl->pb_Val ) = b_Val;
			stChangedInfo.e_DataID = pst_WarTbl->e_DataID;
			stChangedInfo.p_Value  = pst_WarTbl->pb_Val;
			// 2021/10/22 APF OP_BTS-35403 start
			//m_vChangedInfo.push_back( stChangedInfo );
			AddChangedInfo( &stChangedInfo );
			// 2021/10/22 APF OP_BTS-35403 end
		}
	}
}

//************************************************************************************************************************.
//概要　　：Prt警告情報取得処理 (パネル警告表示用:転写ローラーユニット).
//戻り値　：無し.
//備考　　：特に無し.
//作成者　：西村.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
// 2016. 5.13 [ZeusSBK/Icarus] MZV-171 消耗品のライフ警告非表示対応.
//************************************************************************************************************************.
void SSTC_SystemStatusService::getPrtWarningStatus_Opt_pnl_TransRoller()
{
	SST_LOG(MFPE_LogLevelAll);
	
	typedef struct st_CheckWarning {
		TYPE_PrinterWarning		e_Prt;					// PRT警告情報.
		TYPE_NTF_DataID				e_DataID;				// データID.
		Bool 				   	   *pb_Val;					// 警告の有無.
	}ST_CHK_WARNING;
	static ST_CHK_WARNING st_CheckPrtWarningTable[] = {
		{ ATTENTION_TransRollerUnitNearLife		,     TYPE_NTF_DataID_WRN_PRT_30 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt30 )  },	// 転写ローラーユニットニアライフ.
		{ ERROR_TransRollerUnitLifeUser			,     TYPE_NTF_DataID_WRN_PRT_97 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt97 )  },	// 転写ローラーユニットライフ(ユーザー交換時).
		{ ERROR_TransRollerUnitLifeService		,     TYPE_NTF_DataID_WRN_PRT_98 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt98 )  },	// 転写ローラーユニットライフ(サービス交換時).
		{ (TYPE_PrinterWarning)0				,     TYPE_NTF_DataID_End        , NULL						   												}	// end.
	};
	SSTS_ChangedInfo stChangedInfo;
	Bool b_Val;
	
	for( ST_CHK_WARNING *pst_WarTbl = &st_CheckPrtWarningTable[ 0 ]; pst_WarTbl->e_DataID != TYPE_NTF_DataID_End; pst_WarTbl++ ){
		b_Val = APIC_PrinterWarning::newInstance()->isWarning( pst_WarTbl->e_Prt );
		if( *( pst_WarTbl->pb_Val ) != b_Val ){
			*( pst_WarTbl->pb_Val ) = b_Val;
			stChangedInfo.e_DataID = pst_WarTbl->e_DataID;
			stChangedInfo.p_Value  = pst_WarTbl->pb_Val;
			// 2021/10/22 APF OP_BTS-35403 start
			//m_vChangedInfo.push_back( stChangedInfo );
			AddChangedInfo( &stChangedInfo );
			// 2021/10/22 APF OP_BTS-35403 end
		}
	}
}

//************************************************************************************************************************.
//概要　　：Prt警告情報取得処理 (パネル警告表示用:トナー(噴煙)フィルター).
//戻り値　：無し.
//備考　　：特に無し.
//作成者　：西村.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
// 2016. 5.13 [ZeusSBK/Icarus] MZV-171 消耗品のライフ警告非表示対応.
//************************************************************************************************************************.
void SSTC_SystemStatusService::getPrtWarningStatus_Opt_pnl_SmokeFilter()
{
	SST_LOG(MFPE_LogLevelAll);
	
	typedef struct st_CheckWarning {
		TYPE_PrinterWarning		e_Prt;					// PRT警告情報.
		TYPE_NTF_DataID				e_DataID;				// データID.
		Bool 				   	   *pb_Val;					// 警告の有無.
	}ST_CHK_WARNING;
	static ST_CHK_WARNING st_CheckPrtWarningTable[] = {
		{ ATTENTION_SmokeFilterUnitLife			,     TYPE_NTF_DataID_WRN_PRT_32 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt32 )  },	// 噴煙フィルタニアライフ.
		{ ERROR_SmokeFilterUnitLife				,     TYPE_NTF_DataID_WRN_PRT_101, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt101 ) },	// 噴煙フィルタライフ.
		{ (TYPE_PrinterWarning)0				,     TYPE_NTF_DataID_End        , NULL						   												}	// end.
	};
	SSTS_ChangedInfo stChangedInfo;
	Bool b_Val;
	
	for( ST_CHK_WARNING *pst_WarTbl = &st_CheckPrtWarningTable[ 0 ]; pst_WarTbl->e_DataID != TYPE_NTF_DataID_End; pst_WarTbl++ ){
		b_Val = APIC_PrinterWarning::newInstance()->isWarning( pst_WarTbl->e_Prt );
		if( *( pst_WarTbl->pb_Val ) != b_Val ){
			*( pst_WarTbl->pb_Val ) = b_Val;
			stChangedInfo.e_DataID = pst_WarTbl->e_DataID;
			stChangedInfo.p_Value  = pst_WarTbl->pb_Val;
			// 2021/10/22 APF OP_BTS-35403 start
			//m_vChangedInfo.push_back( stChangedInfo );
			AddChangedInfo( &stChangedInfo );
			// 2021/10/22 APF OP_BTS-35403 end
		}
	}
}

//************************************************************************************************************************.
//概要　　：Prt警告情報取得処理 (パネル警告表示用:紙粉除去クリーナー).
//戻り値　：無し.
//備考　　：特に無し.
//作成者　：西村.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
// 2016. 5.13 [ZeusSBK/Icarus] MZV-171 消耗品のライフ警告非表示対応.
//************************************************************************************************************************.
void SSTC_SystemStatusService::getPrtWarningStatus_Opt_pnl_PaperDust()
{
	SST_LOG(MFPE_LogLevelAll);
	
	typedef struct st_CheckWarning {
		TYPE_PrinterWarning		e_Prt;					// PRT警告情報.
		TYPE_NTF_DataID				e_DataID;				// データID.
		Bool 				   	   *pb_Val;					// 警告の有無.
	}ST_CHK_WARNING;
	static ST_CHK_WARNING st_CheckPrtWarningTable[] = {
		{ ATTENTION_PaperDustRemovalCleanerNearLife,  TYPE_NTF_DataID_WRN_PRT_164, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt164 ) },	// 紙粉除去クリーナニアライフ.
		{ ERROR_PaperDustRemovalCleanerLife		,     TYPE_NTF_DataID_WRN_PRT_165, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt165 ) },	// 紙粉除去クリーナライフ.
		{ (TYPE_PrinterWarning)0				,     TYPE_NTF_DataID_End        , NULL						   												}	// end.
	};
	SSTS_ChangedInfo stChangedInfo;
	Bool b_Val;
	
	for( ST_CHK_WARNING *pst_WarTbl = &st_CheckPrtWarningTable[ 0 ]; pst_WarTbl->e_DataID != TYPE_NTF_DataID_End; pst_WarTbl++ ){
		b_Val = APIC_PrinterWarning::newInstance()->isWarning( pst_WarTbl->e_Prt );
		if( *( pst_WarTbl->pb_Val ) != b_Val ){
			*( pst_WarTbl->pb_Val ) = b_Val;
			stChangedInfo.e_DataID = pst_WarTbl->e_DataID;
			stChangedInfo.p_Value  = pst_WarTbl->pb_Val;
			// 2021/10/22 APF OP_BTS-35403 start
			//m_vChangedInfo.push_back( stChangedInfo );
			AddChangedInfo( &stChangedInfo );
			// 2021/10/22 APF OP_BTS-35403 end
		}
	}
}

// 2017/05/16 APF AR.1018782NG Start.
#if defined(DEF_A4_Cus) && defined(DEF_KRONOSS_Cus)
//************************************************************************************************************************.
//概要　　：Prt警告情報取得処理 (パネル警告表示用:メンテナンスキット).
//戻り値　：無し.
//備考　　：特に無し.
//作成者　：門野.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
// 2016. 5.13 AR.1018782NG対応.
//************************************************************************************************************************.
void SSTC_SystemStatusService::getPrtWarningStatus_Opt_pnl_MaintenanceKit()
{
	SST_LOG(MFPE_LogLevelAll);
	
	typedef struct st_CheckWarning {
		TYPE_PrinterWarning		e_Prt;					// PRT警告情報.
		TYPE_NTF_DataID			e_DataID;				// データID.
		Bool 					*pb_Val;				// 警告の有無.
	}ST_CHK_WARNING;
	static ST_CHK_WARNING st_CheckPrtWarningTable[] = {
//Str 【KronosS WBS_16_26 KronosS警告追加対応 : S.Itoh 2016.10.31】.
//Str 【KronosS AR1012883_メンテナンスキットライフパネル表示用 : S.Itoh 2017.02.20】.
		{ ERROR_MaintenanceKitLifeForPNL		,     TYPE_NTF_DataID_WRN_PRT_195 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt195 )  },	// メンテナンスキットライフ.
		{ ATTENTION_MaintenanceKitNearLifeForPNL,     TYPE_NTF_DataID_WRN_PRT_196 , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt196 )  },	// メンテナンスキットニアライフ.
//End 【KronosS AR1012883_メンテナンスキットライフパネル表示用 : S.Itoh 2017.02.20】.
//End 【KronosS WBS_16_26 KronosS警告追加対応 : S.Itoh 2016.10.31】.

		{ (TYPE_PrinterWarning)0				,     TYPE_NTF_DataID_End        , NULL						   												}	// end.
	};
	SSTS_ChangedInfo stChangedInfo;
	Bool b_Val;
	
	for( ST_CHK_WARNING *pst_WarTbl = &st_CheckPrtWarningTable[ 0 ]; pst_WarTbl->e_DataID != TYPE_NTF_DataID_End; pst_WarTbl++ ){
		b_Val = APIC_PrinterWarning::newInstance()->isWarning( pst_WarTbl->e_Prt );
		if( *( pst_WarTbl->pb_Val ) != b_Val ){
			*( pst_WarTbl->pb_Val ) = b_Val;
			stChangedInfo.e_DataID = pst_WarTbl->e_DataID;
			stChangedInfo.p_Value  = pst_WarTbl->pb_Val;
			// 2021/10/22 APF OP_BTS-35403 start
			//m_vChangedInfo.push_back( stChangedInfo );
			AddChangedInfo( &stChangedInfo );
			// 2021/10/22 APF OP_BTS-35403 end
		}
	}
}
#endif    // #if defined(DEF_A4_Cus) && defined(DEF_KRONOSS_Cus)
// 2017/05/16 APF AR.1018782NG End.

//************************************************************************************************************************.
//概要　　：Prt警告情報取得処理 (パネル警告表示用:その他).
//戻り値　：無し.
//備考　　：特に無し.
//作成者　：高橋.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
// 2012. 5.31 [CQ363516] パフォーマンス改善 高周期のポーリング処理で、各条件に該当する場合に変化点検出処理を実行する.
//************************************************************************************************************************.
void SSTC_SystemStatusService::getPrtWarningStatus_Opt_pnl_Other()
{
	SST_LOG(MFPE_LogLevelAll);
	
	typedef struct st_CheckWarning {
		TYPE_PrinterWarning		e_Prt;					// PRT警告情報.
		TYPE_NTF_DataID				e_DataID;				// データID.
		Bool 				   	   *pb_Val;					// 警告の有無.
	}ST_CHK_WARNING;
	static ST_CHK_WARNING st_CheckPrtWarningTable[] = {
		{ ATTENTION_ExcessOfLifeStopForPNL		,     TYPE_NTF_DataID_WRN_PRT_163, &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt163 ) },	// ライフストップ超過警告検出（オイル指しマーク表示用）.
		{ (TYPE_PrinterWarning)0				,     TYPE_NTF_DataID_End        , NULL						   												}	// end.
	};
	SSTS_ChangedInfo stChangedInfo;
	Bool b_Val;
	
	for( ST_CHK_WARNING *pst_WarTbl = &st_CheckPrtWarningTable[ 0 ]; pst_WarTbl->e_DataID != TYPE_NTF_DataID_End; pst_WarTbl++ ){
		b_Val = APIC_PrinterWarning::newInstance()->isWarning( pst_WarTbl->e_Prt );
		if( *( pst_WarTbl->pb_Val ) != b_Val ){
			*( pst_WarTbl->pb_Val ) = b_Val;
			stChangedInfo.e_DataID = pst_WarTbl->e_DataID;
			stChangedInfo.p_Value  = pst_WarTbl->pb_Val;
			// 2021/10/22 APF OP_BTS-35403 start
			//m_vChangedInfo.push_back( stChangedInfo );
			AddChangedInfo( &stChangedInfo );
			// 2021/10/22 APF OP_BTS-35403 end
		}
	}

}

//************************************************************************************************************************.
//概要　　：Scn警告情報取得処理.
//戻り値　：無し.
//備考　　：特に無し.
//作成者　：高橋.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
// 2012. 5.15 [CQ363516] パフォーマンス改善 ポーリング周期を500msと2sに変更.
//************************************************************************************************************************.
void SSTC_SystemStatusService::getScnWarningStatus_High()
{
	SST_LOG(MFPE_LogLevelAll);
	
	typedef struct st_CheckWarning {
		TYPE_ScannerWarning		e_Scn;					// SCN警告情報.
		TYPE_NTF_DataID				e_DataID;				// データID.
		Bool 				   	   *pb_Val;					// 警告の有無.
	}ST_CHK_WARNING;
	static ST_CHK_WARNING st_CheckScannerWarningTable[] = {
		{ CAUTION_ForgotToRemovePlatenDoc, TYPE_NTF_DataID_WRN_SCN_2 , &( m_stSysStatInfo.st_WarningInfoParam.st_ScnWarningInfoParam.b_WrnScn2  ) },	// プラテン上原稿取り忘れ.
		{ ERROR_ADFDoorOpen              , TYPE_NTF_DataID_WRN_SCN_3 , &( m_stSysStatInfo.st_WarningInfoParam.st_ScnWarningInfoParam.b_WrnScn3  ) },	// ＡＤＦドアオープン.
		{ ERROR_ADFJam                   , TYPE_NTF_DataID_WRN_SCN_4 , &( m_stSysStatInfo.st_WarningInfoParam.st_ScnWarningInfoParam.b_WrnScn4  ) },	// ＡＤＦＪＡＭ.
		{ ERROR_ScnNoFeedAdfDoc          , TYPE_NTF_DataID_WRN_SCN_5 , &( m_stSysStatInfo.st_WarningInfoParam.st_ScnWarningInfoParam.b_WrnScn5  ) },	// 原稿ノーフィードエラー(LaPlata固有).
		{ ERROR_ScnMixDocSizeDisable     , TYPE_NTF_DataID_WRN_SCN_6 , &( m_stSysStatInfo.st_WarningInfoParam.st_ScnWarningInfoParam.b_WrnScn6  ) },	// 混載原稿禁止サイズエラー(LaPlata固有).
		{ ERROR_DocSizeMixError          , TYPE_NTF_DataID_WRN_SCN_7 , &( m_stSysStatInfo.st_WarningInfoParam.st_ScnWarningInfoParam.b_WrnScn7  ) },	// 原稿サイズ混載エラー(EDH機固有).
		{ ERROR_NextDocError             , TYPE_NTF_DataID_WRN_SCN_8 , &( m_stSysStatInfo.st_WarningInfoParam.st_ScnWarningInfoParam.b_WrnScn8  ) },	// 次原稿エラー(LaPlata固有).
		{ CAUTION_DirtyEDHGrass1         , TYPE_NTF_DataID_WRN_SCN_9 , &( m_stSysStatInfo.st_WarningInfoParam.st_ScnWarningInfoParam.b_WrnScn9  ) },	// ＥＤＨ原稿ガラス汚れ警告(CAUTION1).
		{ CAUTION_DirtyEDHGrass2         , TYPE_NTF_DataID_WRN_SCN_10, &( m_stSysStatInfo.st_WarningInfoParam.st_ScnWarningInfoParam.b_WrnScn10 ) },	// ＥＤＨ原稿ガラス汚れ警告(CAUTION2).
		{ CAUTION_DirtyEDHBackground     , TYPE_NTF_DataID_WRN_SCN_12, &( m_stSysStatInfo.st_WarningInfoParam.st_ScnWarningInfoParam.b_WrnScn12 ) },	// ＥＤＨ背景板汚れ警告.
		{ CAUTION_EDHTrayOver            , TYPE_NTF_DataID_WRN_SCN_13, &( m_stSysStatInfo.st_WarningInfoParam.st_ScnWarningInfoParam.b_WrnScn13 ) },	// ＥＤＨ原稿容量オーバー警告.
		{ ERROR_SkyShotPreScanError      , TYPE_NTF_DataID_WRN_SCN_14, &( m_stSysStatInfo.st_WarningInfoParam.st_ScnWarningInfoParam.b_WrnScn14 ) },	// フリーイレースプレスキャンエラー.
		{ CAUTION_EDHDocSetError         , TYPE_NTF_DataID_WRN_SCN_15, &( m_stSysStatInfo.st_WarningInfoParam.st_ScnWarningInfoParam.b_WrnScn15 ) },	// ＥＤＨ原稿セット不良警告.
		{ CAUTION_DirtyCISGrass1         , TYPE_NTF_DataID_WRN_SCN_16, &( m_stSysStatInfo.st_WarningInfoParam.st_ScnWarningInfoParam.b_WrnScn16 ) },	// ＣＩＳ読取りガラス汚れ警告(CAUTION1).
		{ CAUTION_DirtyCISGrass2         , TYPE_NTF_DataID_WRN_SCN_17, &( m_stSysStatInfo.st_WarningInfoParam.st_ScnWarningInfoParam.b_WrnScn17 ) },	// ＣＩＳ読取りガラス汚れ警告(CAUTION2).
#if defined( DEF_A4_Cus )
		{ ERROR_ScannerLock              , TYPE_NTF_DataID_WRN_SCN_19, &( m_stSysStatInfo.st_WarningInfoParam.st_ScnWarningInfoParam.b_WrnScn19 ) },	// スライダーロック異常.
#endif // defined( DEF_A4_Cus ).
		{ (TYPE_ScannerWarning)NULL                           , TYPE_NTF_DataID_End       , 0                                                                          }		// end.
	};
	SSTS_ChangedInfo stChangedInfo;
	Bool b_Val;
	
	for( ST_CHK_WARNING *pst_WarTbl = &st_CheckScannerWarningTable[ 0 ]; pst_WarTbl->e_DataID != TYPE_NTF_DataID_End; pst_WarTbl++ ){
		b_Val = APIC_ScannerWarning::newInstance()->isWarning( pst_WarTbl->e_Scn );
		if( *( pst_WarTbl->pb_Val ) != b_Val ){
			*( pst_WarTbl->pb_Val ) = b_Val;
			stChangedInfo.e_DataID = pst_WarTbl->e_DataID;
			stChangedInfo.p_Value  = pst_WarTbl->pb_Val;
			// 2021/10/22 APF OP_BTS-35403 start
			//m_vChangedInfo.push_back( stChangedInfo );
			AddChangedInfo( &stChangedInfo );
			// 2021/10/22 APF OP_BTS-35403 end
		}
	}

}

//************************************************************************************************************************.
//概要　　：Scn警告情報取得処理.
//戻り値　：無し.
//備考　　：特に無し.
//作成者　：高橋.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
// 2012. 5.15 [CQ363516] パフォーマンス改善 ポーリング周期を500msと2sに変更.
//************************************************************************************************************************.
void SSTC_SystemStatusService::getScnWarningStatus_Low()
{
	SST_LOG(MFPE_LogLevelAll);
	
	typedef struct st_CheckWarning {
		TYPE_ScannerWarning		e_Scn;					// SCN警告情報.
		TYPE_NTF_DataID				e_DataID;				// データID.
		Bool 				   	   *pb_Val;					// 警告の有無.
	}ST_CHK_WARNING;
	static ST_CHK_WARNING st_CheckScannerWarningTable[] = {
		{ ATTENTION_IRWarning            , TYPE_NTF_DataID_WRN_SCN_1 , &( m_stSysStatInfo.st_WarningInfoParam.st_ScnWarningInfoParam.b_WrnScn1  ) },	// ＩＲ関連異常警告.
		{ ATTENTION_DirtyEDHGrass        , TYPE_NTF_DataID_WRN_SCN_11, &( m_stSysStatInfo.st_WarningInfoParam.st_ScnWarningInfoParam.b_WrnScn11 ) },	// ＥＤＨ原稿ガラス汚れ警告(ATTENTION).
		{ ATTENTION_DirtyCISGrass        , TYPE_NTF_DataID_WRN_SCN_18, &( m_stSysStatInfo.st_WarningInfoParam.st_ScnWarningInfoParam.b_WrnScn18 ) },	// ＣＩＳ読取りガラス汚れ警告(ATTENTION).
		{ (TYPE_ScannerWarning)NULL                           , TYPE_NTF_DataID_End       , 0                                                                          }		// end.
	};
	SSTS_ChangedInfo stChangedInfo;
	Bool b_Val;
	
	for( ST_CHK_WARNING *pst_WarTbl = &st_CheckScannerWarningTable[ 0 ]; pst_WarTbl->e_DataID != TYPE_NTF_DataID_End; pst_WarTbl++ ){
		b_Val = APIC_ScannerWarning::newInstance()->isWarning( pst_WarTbl->e_Scn );
		if( *( pst_WarTbl->pb_Val ) != b_Val ){
			*( pst_WarTbl->pb_Val ) = b_Val;
			stChangedInfo.e_DataID = pst_WarTbl->e_DataID;
			stChangedInfo.p_Value  = pst_WarTbl->pb_Val;
			// 2021/10/22 APF OP_BTS-35403 start
			//m_vChangedInfo.push_back( stChangedInfo );
			AddChangedInfo( &stChangedInfo );
			// 2021/10/22 APF OP_BTS-35403 end
		}
	}

}

// 2014.01.09-STA  パネルHW情報の取得.
//************************************************************************************************************************.
//概要　　：パネルHW情報の取得処理.
//戻り値　：無し.
//備考　　：特に無し.
//作成者　：高橋.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//************************************************************************************************************************.
void SSTC_SystemStatusService::getPanelHardResolution(TYPS_PanelHardResolution* ps_PanelHardResolution)
{
	SST_LOG(MFPE_LogLevelAll);

	if(ps_PanelHardResolution == 0) {
		return;
	}
//	CNVC_PanelHard* pcPanelHard = CNVC_PanelHard::newInstance();

	memset(ps_PanelHardResolution,0,sizeof(TYPS_PanelHardResolution));

#if 1 
	// CNVC_PanelHard 実装まちのため暫定値.
	// Makefile中の DEF_LCD_7INCH も暫定対応なのでCNVC_PanelHard側リリースで削除すること.
	#ifdef DEF_LCD_7INCH
	ps_PanelHardResolution->sf_PanelSize   = 7.0f;
	ps_PanelHardResolution->us_ResolutionX = 800;
	ps_PanelHardResolution->us_ResolutionY = 480;
	#else
	ps_PanelHardResolution->sf_PanelSize   = 9.0f;
	ps_PanelHardResolution->us_ResolutionX = 800;
	ps_PanelHardResolution->us_ResolutionY = 480;
	#endif
#endif

}

//************************************************************************************************************************.
//概要　　：パネルHW情報の初期値設定処理.
//戻り値　：無し.
//備考　　：特に無し.
//作成者　：高橋.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//************************************************************************************************************************.
void SSTC_SystemStatusService::initPanelHardResolution()
{
	SST_LOG(MFPE_LogLevelAll);

	getPanelHardResolution(&m_stSysStatInfo.st_PanelInfoParam.st_PanelHardResolution);

	// グループ情報を取得.
	TYPE_NTF_GroupID  e_GroupID    = TYPE_NTF_GroupID_End;
	void 			 *pv_GroupData = NULL;
			
	if( getGroupInfo( TYPE_NTF_DataID_PNL_5, &e_GroupID, &pv_GroupData ) == False ){
		//APIC_ThrowSystemError(SYSE_SysErrGeneral, LOGF_DEBUG_EVENT_KIND_SYS_STATUS, "SSTC_SystemStatusService getGroupInfo()");
		SST_ERR_LOG("getGroupInfo() Failed!.\n");
		return;
	}

	// 共有メモリ更新.
	m_pcInstance->lock();
	m_pcShMemMan->writeData( e_GroupID, pv_GroupData );
	m_pcInstance->unlock();
}

//************************************************************************************************************************.
//概要　　：パネルHW情報更新取得処理.
//戻り値　：無し.
//備考　　：特に無し.
//作成者　：高橋.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//************************************************************************************************************************.
void SSTC_SystemStatusService::getPanelInfoStatus_PanelHardResolution_Low()
{
	SST_LOG(MFPE_LogLevelAll);
	
	SSTS_ChangedInfo stChangedInfo;

	// パネルサイズ情報.
	TYPS_PanelHardResolution s_PanelHardResolution;
	getPanelHardResolution(&s_PanelHardResolution);
	if(0 != memcmp(&m_stSysStatInfo.st_PanelInfoParam.st_PanelHardResolution,&s_PanelHardResolution,sizeof(TYPS_PanelHardResolution))) {
		m_stSysStatInfo.st_PanelInfoParam.st_PanelHardResolution = s_PanelHardResolution;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_PNL_5;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PanelInfoParam.st_PanelHardResolution);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
}
// 2014.01.09-END  パネルHW情報の取得.

//************************************************************************************************************************.
//概要　　：System Status 情報ファイルの書込み処理.
//引数　　：<in>　psc_FilePath		ファイルPATH.
//　　　　：<in>　pst_SysStatInfo	System Status情報先頭アドレス.
//　　　　：<in>　ul_dataSize		System Status情報サイズ.
//戻り値　：処理結果 正常終了時：TYPD_SSTS_StatusSuccess　異常終了時：TYPD_SSTS_STATUS_F****_ERROR.
//備考　　：特に無し.
//作成者　：高橋.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
// 2011.12.08, ADS坪井, CIT不具合対応[マージ後不具合管理票 No.164].
//  - APIC_FaxObserver::xxx()通知情報     st_DevXX->st_DevXX[TYPE_CommLine2].
//  - アクセス状態変化通知（アプリ単位）  st_Dev8->st_Dev8[TYPE_UsbMemAppIdAll].
// 2011.12.15. ADS データなしObserverのデータ追加対応.
// 2012/09/13, ADS坪井, CQ369866:Android接続中通知追加.
//************************************************************************************************************************.
TYPD_SSTS_STATUS SSTC_SystemStatusService::writeSystemStatusInfo( char *psc_FilePath, TYPS_SystemStatusInfo *pst_SysStatInfo, size_t ul_dataSize )
{
	SST_LOG2(MFPE_LogLevelTrace, "psc_FilePath=(%s) ul_dataSize=(%ld).\n", psc_FilePath, ul_dataSize);
	
	TYPD_SSTS_STATUS sc_Result = TYPD_SSTS_StatusSuccess;
	
	struct st_WriteDataInfo {
		TYPE_NTF_DataID e_DataID;			// データID.
		uint32_t        ul_DataSize;		// 可変長データ部の長さ.
		void            *pv_Data;			// 可変長データ部.
	} static st_WriteDataInfoTable[] = {
		{ TYPE_NTF_DataID_WRN_RDS_1  , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_RdsWarningInfoParam.b_WrnRds1 )          },	// モデム異常.
		{ TYPE_NTF_DataID_WRN_RDS_2  , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_RdsWarningInfoParam.b_WrnRds2 )          },	// コントローラ異常.
		{ TYPE_NTF_DataID_WRN_BOX_1  , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_BoxWarningInfoParam.b_WrnBox1 )          },	// HDD容量がFULLになったか.
		{ TYPE_NTF_DataID_WRN_BOX_2  , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_BoxWarningInfoParam.b_WrnBox2 )          },	// Box登録数Over(1000個に達したか).
		{ TYPE_NTF_DataID_WRN_BOX_3  , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_BoxWarningInfoParam.b_WrnBox3 )          },	// Box内Document数Over(200数に達したか).
		{ TYPE_NTF_DataID_WRN_BOX_4  , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_BoxWarningInfoParam.b_WrnBox4 )          },	// Documentページ数Over(9000ページに達したか).
		{ TYPE_NTF_DataID_WRN_FAX_1  , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_FaxWarningInfoParam.st_WrnFax1.b_Line1 ) },	// ダイアルエラー(回線1).
		{ TYPE_NTF_DataID_WRN_FAX_2  , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_FaxWarningInfoParam.st_WrnFax1.b_Line2 ) },	// ダイアルエラー(回線2).
		{ TYPE_NTF_DataID_WRN_FAX_3  , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_FaxWarningInfoParam.st_WrnFax2.b_Line1 ) },	// 送信中エラー(回線1).
		{ TYPE_NTF_DataID_WRN_FAX_4  , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_FaxWarningInfoParam.st_WrnFax2.b_Line2 ) },	// 送信中エラー(回線2).
		{ TYPE_NTF_DataID_WRN_FAX_5  , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_FaxWarningInfoParam.st_WrnFax3.b_Line1 ) },	// 受信中エラー(回線1).
		{ TYPE_NTF_DataID_WRN_FAX_6  , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_FaxWarningInfoParam.st_WrnFax3.b_Line2 ) },	// 受信中エラー(回線2).
		{ TYPE_NTF_DataID_WRN_FAX_7  , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_FaxWarningInfoParam.st_WrnFax4.b_Line1 ) },	// テストのエラー(回線1).
		{ TYPE_NTF_DataID_WRN_FAX_8  , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_FaxWarningInfoParam.st_WrnFax4.b_Line2 ) },	// テストのエラー(回線2).
		{ TYPE_NTF_DataID_WRN_FAX_9  , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_FaxWarningInfoParam.st_WrnFax5.b_Line1 ) },	// Iswのエラー(回線1).
		{ TYPE_NTF_DataID_WRN_FAX_10 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_FaxWarningInfoParam.st_WrnFax5.b_Line2 ) },	// Iswのエラー(回線2).
		{ TYPE_NTF_DataID_WRN_FAX_11 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_FaxWarningInfoParam.st_WrnFax6.b_Line1 ) },	// NvRam書き込み中のエラー(回線1).
		{ TYPE_NTF_DataID_WRN_FAX_12 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_FaxWarningInfoParam.st_WrnFax6.b_Line2 ) },	// NvRam書き込み中のエラー(回線2).
		{ TYPE_NTF_DataID_WRN_FAX_13 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_FaxWarningInfoParam.st_WrnFax7.b_Line1 ) },	// 初期化中のエラー(回線1).
		{ TYPE_NTF_DataID_WRN_FAX_14 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_FaxWarningInfoParam.st_WrnFax7.b_Line2 ) },	// 初期化中のエラー(回線2).
		// UV-00877 help add start.
		{ TYPE_NTF_DataID_WRN_FAX_15 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_FaxWarningInfoParam.st_WrnFax1.b_Line3 ) },	// ダイアルエラー(回線3).
		{ TYPE_NTF_DataID_WRN_FAX_16 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_FaxWarningInfoParam.st_WrnFax1.b_Line4 ) },	// ダイアルエラー(回線4).
		{ TYPE_NTF_DataID_WRN_FAX_17 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_FaxWarningInfoParam.st_WrnFax2.b_Line3 ) },	// 送信中エラー(回線3).
		{ TYPE_NTF_DataID_WRN_FAX_18 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_FaxWarningInfoParam.st_WrnFax2.b_Line4 ) },	// 送信中エラー(回線4).
		{ TYPE_NTF_DataID_WRN_FAX_19 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_FaxWarningInfoParam.st_WrnFax3.b_Line3 ) },	// 受信中エラー(回線3).
		{ TYPE_NTF_DataID_WRN_FAX_20 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_FaxWarningInfoParam.st_WrnFax3.b_Line4 ) },	// 受信中エラー(回線4).
		{ TYPE_NTF_DataID_WRN_FAX_21 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_FaxWarningInfoParam.st_WrnFax4.b_Line3 ) },	// テストのエラー(回線3).
		{ TYPE_NTF_DataID_WRN_FAX_22 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_FaxWarningInfoParam.st_WrnFax4.b_Line4 ) },	// テストのエラー(回線4).
		{ TYPE_NTF_DataID_WRN_FAX_23 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_FaxWarningInfoParam.st_WrnFax5.b_Line3 ) },	// Iswのエラー(回線3).
		{ TYPE_NTF_DataID_WRN_FAX_24 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_FaxWarningInfoParam.st_WrnFax5.b_Line4 ) },	// Iswのエラー(回線4).
		{ TYPE_NTF_DataID_WRN_FAX_25 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_FaxWarningInfoParam.st_WrnFax6.b_Line3 ) },	// NvRam書き込み中のエラー(回線3).
		{ TYPE_NTF_DataID_WRN_FAX_26 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_FaxWarningInfoParam.st_WrnFax6.b_Line4 ) },	// NvRam書き込み中のエラー(回線4).
		{ TYPE_NTF_DataID_WRN_FAX_27 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_FaxWarningInfoParam.st_WrnFax7.b_Line3 ) },	// 初期化中のエラー(回線3).
		{ TYPE_NTF_DataID_WRN_FAX_28 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_FaxWarningInfoParam.st_WrnFax7.b_Line4 ) },	// 初期化中のエラー(回線4).
		// UV-00877 help add end.
// 2018/02/27 APF 制御構成変更No.82 リソースフル状態の表示対応 s.
// 2018/04/18 APF AR.1044038 Start.
//		{ TYPE_NTF_DataID_WRN_FAX_29 , (uint32_t)sizeof(FAXE_ResourceWarning)                  , &( m_stSysStatInfo.st_WarningInfoParam.st_FaxWarningInfoParam.e_WrnFax8 )          },	// FAX通信不可警告.
		{ TYPE_NTF_DataID_WRN_FAX_29 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_FaxWarningInfoParam.b_WrnFax8 )          },	// FAX通信不可警告.
// 2018/04/18 APF AR.1044038 End.
// 2018/02/27 APF 制御構成変更No.82 リソースフル状態の表示対応 e.
		{ TYPE_NTF_DataID_WRN_PRT_1  , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt1 )          },	// シアン色トナーニアエンプティ.
		{ TYPE_NTF_DataID_WRN_PRT_2  , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt2 )          },	// マゼンダ色トナーニアエンプティ.
		{ TYPE_NTF_DataID_WRN_PRT_3  , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt3 )          },	// イエロー色トナーニアエンプティ.
		{ TYPE_NTF_DataID_WRN_PRT_4  , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt4 )          },	// ブラック色トナーニアエンプティ.
		{ TYPE_NTF_DataID_WRN_PRT_5  , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt5 )          },	// シアン色トナーエンプティストップ.
		{ TYPE_NTF_DataID_WRN_PRT_6  , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt6 )          },	// マゼンダ色トナーエンプティストップ.
		{ TYPE_NTF_DataID_WRN_PRT_7  , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt7 )          },	// イエロー色トナーエンプティストップ.
		{ TYPE_NTF_DataID_WRN_PRT_8  , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt8 )          },	// ブラック色トナーエンプティストップ.
		{ TYPE_NTF_DataID_WRN_PRT_9  , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt9 )          },	// 廃トナーニアフル.
		{ TYPE_NTF_DataID_WRN_PRT_10 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt10 )         },	// ＰＲＴ関連異常警告.
		{ TYPE_NTF_DataID_WRN_PRT_11 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt11 )         },	// ＩＵニアライフ・シアン.
		{ TYPE_NTF_DataID_WRN_PRT_12 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt12 )         },	// ＩＵニアライフ・マゼンダ.
		{ TYPE_NTF_DataID_WRN_PRT_13 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt13 )         },	// ＩＵニアライフ・イエロー.
		{ TYPE_NTF_DataID_WRN_PRT_14 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt14 )         },	// ＩＵニアライフ・ブラック.
		{ TYPE_NTF_DataID_WRN_PRT_15 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt15 )         },	// ＤＣニアライフ・シアン.
		{ TYPE_NTF_DataID_WRN_PRT_16 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt16 )         },	// ＤＣニアライフ・マゼンダ.
		{ TYPE_NTF_DataID_WRN_PRT_17 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt17 )         },	// ＤＣニアライフ・イエロー.
		{ TYPE_NTF_DataID_WRN_PRT_18 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt18 )         },	// ＤＣニアライフ・ブラック.
		{ TYPE_NTF_DataID_WRN_PRT_19 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt19 )         },	// ＤｅｖＣニアライフ・シアン.
		{ TYPE_NTF_DataID_WRN_PRT_20 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt20 )         },	// ＤｅｖＣニアライフ・マゼンダ.
		{ TYPE_NTF_DataID_WRN_PRT_21 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt21 )         },	// ＤｅｖＣニアライフ・イエロー.
		{ TYPE_NTF_DataID_WRN_PRT_22 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt22 )         },	// ＤｅｖＣニアライフ・ブラック.
		{ TYPE_NTF_DataID_WRN_PRT_23 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt23 )         },	// 定着ユニットニアライフ.
		{ TYPE_NTF_DataID_WRN_PRT_24 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt24 )         },	// ステープルエンプティ1(奥).
		{ TYPE_NTF_DataID_WRN_PRT_25 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt25 )         },	// ステープルエンプティ2(手前).
		{ TYPE_NTF_DataID_WRN_PRT_26 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt26 )         },	// ステープルエンプティ3.
		{ TYPE_NTF_DataID_WRN_PRT_27 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt27 )         },	// エレベートトレイ上昇未完.
		{ TYPE_NTF_DataID_WRN_PRT_28 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt28 )         },	// パンチくずフル(ユーザー交換時).
		{ TYPE_NTF_DataID_WRN_PRT_29 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt29 )         },	// パンチくずフル(サービス交換時).
		{ TYPE_NTF_DataID_WRN_PRT_30 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt30 )         },	// 転写ローラーユニットニアライフ.
		{ TYPE_NTF_DataID_WRN_PRT_31 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt31 )         },	// 紙紛除去クリーナー／オゾンフィルタニアライフ.
		{ TYPE_NTF_DataID_WRN_PRT_32 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt32 )         },	// 噴煙フィルタニアライフ.
		{ TYPE_NTF_DataID_WRN_PRT_33 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt33 )         },	// 転写ベルトユニットニアライフ.
		{ TYPE_NTF_DataID_WRN_PRT_34 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt34 )         },	// ＰＭカウンタオーバー(ATTENTION).
		{ TYPE_NTF_DataID_WRN_PRT_35 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt35 )         },	// プリンタセット不良(ATTENTION).
		{ TYPE_NTF_DataID_WRN_PRT_36 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt36 )         },	// ＤＣユーザー交換超過警告・シアン（Amurのみ).
		{ TYPE_NTF_DataID_WRN_PRT_37 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt37 )         },	// ＤＣユーザー交換超過警告・マゼンダ（Amurのみ).
		{ TYPE_NTF_DataID_WRN_PRT_38 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt38 )         },	// ＤＣユーザー交換超過警告・イエロー（Amurのみ).
		{ TYPE_NTF_DataID_WRN_PRT_39 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt39 )         },	// ＤＣユーザー交換超過警告・ブラック（Amurのみ).
		{ TYPE_NTF_DataID_WRN_PRT_40 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt40 )         },	// ＯＨＰ縦用紙セット禁止警告.
		{ TYPE_NTF_DataID_WRN_PRT_41 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt41 )         },	// プリンタジャム検出.
		{ TYPE_NTF_DataID_WRN_PRT_42 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt42 )         },	// シアン色トナーエンプティストップ.
		{ TYPE_NTF_DataID_WRN_PRT_43 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt43 )         },	// マゼンダ色トナーエンプティストップ.
		{ TYPE_NTF_DataID_WRN_PRT_44 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt44 )         },	// イエロー色トナーエンプティストップ.
		{ TYPE_NTF_DataID_WRN_PRT_45 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt45 )         },	// ブラック色トナーエンプティストップ.
		{ TYPE_NTF_DataID_WRN_PRT_46 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt46 )         },	// 用紙サイズ不一致.
		{ TYPE_NTF_DataID_WRN_PRT_47 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt47 )         },	// 用紙種類不一致.
		{ TYPE_NTF_DataID_WRN_PRT_48 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt48 )         },	// 給紙トレイセット不良.
		{ TYPE_NTF_DataID_WRN_PRT_49 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt49 )         },	// プリンタセット不良.
		{ TYPE_NTF_DataID_WRN_PRT_50 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt50 )         },	// 廃トナーボトル（廃棄トナーボトル）セット不良.
		{ TYPE_NTF_DataID_WRN_PRT_51 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt51 )         },	// トナーボトルセット不良・シアン.
		{ TYPE_NTF_DataID_WRN_PRT_52 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt52 )         },	// トナーボトルセット不良・マゼンダ.
		{ TYPE_NTF_DataID_WRN_PRT_53 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt53 )         },	// トナーボトルセット不良・イエロー.
		{ TYPE_NTF_DataID_WRN_PRT_54 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt54 )         },	// トナーボトルセット不良・ブラック.
		{ TYPE_NTF_DataID_WRN_PRT_55 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt55 )         },	// ＩＵライフ・シアン.
		{ TYPE_NTF_DataID_WRN_PRT_56 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt56 )         },	// ＩＵライフ・マゼンダ.
		{ TYPE_NTF_DataID_WRN_PRT_57 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt57 )         },	// ＩＵライフ・イエロー.
		{ TYPE_NTF_DataID_WRN_PRT_58 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt58 )         },	// ＩＵライフ・ブラック.
		{ TYPE_NTF_DataID_WRN_PRT_59 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt59 )         },	// ＤＣライフ・シアン.
		{ TYPE_NTF_DataID_WRN_PRT_60 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt60 )         },	// ＤＣライフ・マゼンダ.
		{ TYPE_NTF_DataID_WRN_PRT_61 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt61 )         },	// ＤＣライフ・イエロー.
		{ TYPE_NTF_DataID_WRN_PRT_62 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt62 )         },	// ＤＣライフ・ブラック.
		{ TYPE_NTF_DataID_WRN_PRT_63 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt63 )         },	// ＤｅｖＣライフ・シアン.
		{ TYPE_NTF_DataID_WRN_PRT_64 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt64 )         },	// ＤｅｖＣライフ・マゼンダ.
		{ TYPE_NTF_DataID_WRN_PRT_65 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt65 )         },	// ＤｅｖＣライフ・イエロー.
		{ TYPE_NTF_DataID_WRN_PRT_66 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt66 )         },	// ＤｅｖＣライフ・ブラック.
		{ TYPE_NTF_DataID_WRN_PRT_67 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt67 )         },	// 転写クリーナーユニットライフ.
		{ TYPE_NTF_DataID_WRN_PRT_68 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt68 )         },	// 定着ユニットライフ.
		{ TYPE_NTF_DataID_WRN_PRT_69 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt69 )         },	// ＩＵセット不良・シアン.
		{ TYPE_NTF_DataID_WRN_PRT_70 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt70 )         },	// ＩＵセット不良・マゼンダ.
		{ TYPE_NTF_DataID_WRN_PRT_71 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt71 )         },	// ＩＵセット不良・イエロー.
		{ TYPE_NTF_DataID_WRN_PRT_72 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt72 )         },	// ＩＵセット不良・ブラック.
		{ TYPE_NTF_DataID_WRN_PRT_73 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt73 )         },	// ＤＣセット不良・シアン.
		{ TYPE_NTF_DataID_WRN_PRT_74 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt74 )         },	// ＤＣセット不良・マゼンダ.
		{ TYPE_NTF_DataID_WRN_PRT_75 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt75 )         },	// ＤＣセット不良・イエロー.
		{ TYPE_NTF_DataID_WRN_PRT_76 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt76 )         },	// ＤＣセット不良・ブラック.
		{ TYPE_NTF_DataID_WRN_PRT_77 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt77 )         },	// ＤｅｖＣセット不良・シアン.
		{ TYPE_NTF_DataID_WRN_PRT_78 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt78 )         },	// ＤｅｖＣセット不良・マゼンダ.
		{ TYPE_NTF_DataID_WRN_PRT_79 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt79 )         },	// ＤｅｖＣセット不良・イエロー.
		{ TYPE_NTF_DataID_WRN_PRT_80 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt80 )         },	// ＤｅｖＣセット不良・ブラック.
		{ TYPE_NTF_DataID_WRN_PRT_81 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt81 )         },	// ＩＵ誤装着・シアン.
		{ TYPE_NTF_DataID_WRN_PRT_82 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt82 )         },	// ＩＵ誤装着・マゼンダ.
		{ TYPE_NTF_DataID_WRN_PRT_83 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt83 )         },	// ＩＵ誤装着・イエロー.
		{ TYPE_NTF_DataID_WRN_PRT_84 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt84 )         },	// ＩＵ誤装着・ブラック.
		{ TYPE_NTF_DataID_WRN_PRT_85 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt85 )         },	// ＤＣ誤装着・シアン.
		{ TYPE_NTF_DataID_WRN_PRT_86 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt86 )         },	// ＤＣ誤装着・マゼンダ.
		{ TYPE_NTF_DataID_WRN_PRT_87 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt87 )         },	// ＤＣ誤装着・イエロー.
		{ TYPE_NTF_DataID_WRN_PRT_88 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt88 )         },	// ＤＣ誤装着・ブラック.
		{ TYPE_NTF_DataID_WRN_PRT_89 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt89 )         },	// ＤｅｖＣ誤装着・シアン.
		{ TYPE_NTF_DataID_WRN_PRT_90 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt90 )         },	// ＤｅｖＣ誤装着・マゼンダ.
		{ TYPE_NTF_DataID_WRN_PRT_91 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt91 )         },	// ＤｅｖＣ誤装着・イエロー.
		{ TYPE_NTF_DataID_WRN_PRT_92 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt92 )         },	// ＤｅｖＣ誤装着・ブラック.
		{ TYPE_NTF_DataID_WRN_PRT_93 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt93 )         },	// 転写ベルトユニットセット不良.
		{ TYPE_NTF_DataID_WRN_PRT_94 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt94 )         },	// 定着ユニットセット不良.
		{ TYPE_NTF_DataID_WRN_PRT_95 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt95 )         },	// 廃トナーフル.
		{ TYPE_NTF_DataID_WRN_PRT_96 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt96 )         },	// 処理トレイ用紙取り忘れ.
		{ TYPE_NTF_DataID_WRN_PRT_97 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt97 )         },	// 転写ローラーユニットライフ(ユーザー交換時).
		{ TYPE_NTF_DataID_WRN_PRT_98 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt98 )         },	// 転写ローラーユニットライフ(サービス交換時).
		{ TYPE_NTF_DataID_WRN_PRT_99 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt99 )         },	// 紙紛除去クリーナー／オゾンフィルタライフ(ユーザー交換時).
		{ TYPE_NTF_DataID_WRN_PRT_100, (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt100 )        },	// 紙紛除去クリーナー／オゾンフィルタライフ(サービス交換時).
		{ TYPE_NTF_DataID_WRN_PRT_101, (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt101 )        },	// 噴煙フィルタライフ.
		{ TYPE_NTF_DataID_WRN_PRT_102, (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt102 )        },	// 転写ベルトユニットライフ.
		{ TYPE_NTF_DataID_WRN_PRT_103, (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt103 )        },	// ＰＭカウンタオーバー.
		{ TYPE_NTF_DataID_WRN_PRT_104, (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt104 )        },	// 手差しトレイのサイズ検出出来ず.
		{ TYPE_NTF_DataID_WRN_PRT_105, (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt105 )        },	// 特定原稿検出.
		{ TYPE_NTF_DataID_WRN_PRT_106, (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt106 )        },	// トータルカウンタセット不良.
		{ TYPE_NTF_DataID_WRN_PRT_107, (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt107 )        },	// 起動時ビン／トレイ容量オーバー.
		{ TYPE_NTF_DataID_WRN_PRT_108, (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt108 )        },	// ＴＣ誤装着・シアン.
		{ TYPE_NTF_DataID_WRN_PRT_109, (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt109 )        },	// ＴＣ誤装着・マゼンダ.
		{ TYPE_NTF_DataID_WRN_PRT_110, (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt110 )        },	// ＴＣ誤装着・イエロー.
		{ TYPE_NTF_DataID_WRN_PRT_111, (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt111 )        },	// ＴＣ誤装着・ブラック.
		{ TYPE_NTF_DataID_WRN_PRT_112, (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt112 )        },	// レターヘッド紙小サイズ禁止警告.
		{ TYPE_NTF_DataID_WRN_PRT_113, (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt113 )        },	// トナーカートリッジセット不良・シアン.
		{ TYPE_NTF_DataID_WRN_PRT_114, (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt114 )        },	// トナーカートリッジセット不良・マゼンダ.
		{ TYPE_NTF_DataID_WRN_PRT_115, (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt115 )        },	// トナーカートリッジセット不良・イエロー.
		{ TYPE_NTF_DataID_WRN_PRT_116, (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt116 )        },	// トナーカートリッジセット不良・ブラック.
		{ TYPE_NTF_DataID_WRN_PRT_117, (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt117 )        },	// Z折りユニットパンチくずフル(ユーザー交換時).
		{ TYPE_NTF_DataID_WRN_PRT_118, (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt118 )        },	// Z折りユニットパンチくずフル(サービス交換時).
		{ TYPE_NTF_DataID_WRN_PRT_119, (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt119 )        },	// ＴＣ誤装着・シアン（ＡＴＴＥＮＴＩＯＮレベル）.
		{ TYPE_NTF_DataID_WRN_PRT_120, (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt120 )        },	// ＴＣ誤装着・マゼンダ（ＡＴＴＥＮＴＩＯＮレベル）.
		{ TYPE_NTF_DataID_WRN_PRT_121, (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt121 )        },	// ＴＣ誤装着・イエロー（ＡＴＴＥＮＴＩＯＮレベル）.
		{ TYPE_NTF_DataID_WRN_PRT_122, (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt122 )        },	// ＴＣ誤装着・ブラック（ＡＴＴＥＮＴＩＯＮレベル）.
		{ TYPE_NTF_DataID_WRN_PRT_123, (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt123 )        },	// ＩＵライフあり（事前警告）・シアン.
		{ TYPE_NTF_DataID_WRN_PRT_124, (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt124 )        },	// ＩＵライフあり（事前警告）・マゼンダ.
		{ TYPE_NTF_DataID_WRN_PRT_125, (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt125 )        },	// ＩＵライフあり（事前警告）・イエロー.
		{ TYPE_NTF_DataID_WRN_PRT_126, (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt126 )        },	// ＩＵライフあり（事前警告）・ブラック.
		{ TYPE_NTF_DataID_WRN_PRT_127, (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt127 )        },	// 現像剤ニアライフ・ブラック.
		{ TYPE_NTF_DataID_WRN_PRT_128, (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt128 )        },	// 現像剤ライフ・ブラック.
// ADS坪井 → 2011/12/01 【パフォーマンス改善】isWarning対応 Start.
		{ TYPE_NTF_DataID_WRN_PRT_129, (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt129 )        },	// 薄紙小サイズ用紙セット禁止警告.
		{ TYPE_NTF_DataID_WRN_PRT_130, (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt130 )        },	// 手差しサイズエラー警告.
		{ TYPE_NTF_DataID_WRN_PRT_131, (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt131 )        },	// 強制停止サイズエラー警告.
		{ TYPE_NTF_DataID_WRN_PRT_132, (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt132 )        },	// 定着プラグ抜け警告.
		{ TYPE_NTF_DataID_WRN_PRT_133, (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt133 )        },	// Ｃ ＩＵニアライフ警告検出（パネル確認用）.
		{ TYPE_NTF_DataID_WRN_PRT_134, (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt134 )        },	// Ｍ ＩＵニアライフ警告検出（パネル確認用）.
		{ TYPE_NTF_DataID_WRN_PRT_135, (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt135 )        },	// Ｙ ＩＵニアライフ警告検出（パネル確認用）.
		{ TYPE_NTF_DataID_WRN_PRT_136, (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt136 )        },	// Ｋ ＩＵニアライフ警告検出（パネル確認用）.
		{ TYPE_NTF_DataID_WRN_PRT_137, (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt137 )        },	// Ｃ ＩＵライフストップ警告検出（パネル確認用）.
		{ TYPE_NTF_DataID_WRN_PRT_138, (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt138 )        },	// Ｍ ＩＵライフストップ警告検出（パネル確認用）.
		{ TYPE_NTF_DataID_WRN_PRT_139, (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt139 )        },	// Ｙ ＩＵライフストップ警告検出（パネル確認用）.
		{ TYPE_NTF_DataID_WRN_PRT_140, (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt140 )        },	// Ｋ ＩＵライフストップ警告検出（パネル確認用）.
		{ TYPE_NTF_DataID_WRN_PRT_141, (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt141 )        },	// Ｃ ＤＣニアライフ警告検出（パネル確認用）.
		{ TYPE_NTF_DataID_WRN_PRT_142, (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt142 )        },	// Ｍ ＤＣニアライフ警告検出（パネル確認用）.
		{ TYPE_NTF_DataID_WRN_PRT_143, (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt143 )        },	// Ｙ ＤＣニアライフ警告検出（パネル確認用）.
		{ TYPE_NTF_DataID_WRN_PRT_144, (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt144 )        },	// Ｋ ＤＣニアライフ警告検出（パネル確認用）.
		{ TYPE_NTF_DataID_WRN_PRT_145, (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt145 )        },	// Ｃ ＤＣライフストップ警告検出（パネル確認用）.
		{ TYPE_NTF_DataID_WRN_PRT_146, (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt146 )        },	// Ｍ ＤＣライフストップ警告検出（パネル確認用）.
		{ TYPE_NTF_DataID_WRN_PRT_147, (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt147 )        },	// Ｙ ＤＣライフストップ警告検出（パネル確認用）.
		{ TYPE_NTF_DataID_WRN_PRT_148, (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt148 )        },	// Ｋ ＤＣライフストップ警告検出（パネル確認用）.
		{ TYPE_NTF_DataID_WRN_PRT_149, (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt149 )        },	// Ｃ 現像ユニットニアライフ警告検出（パネル確認用）.
		{ TYPE_NTF_DataID_WRN_PRT_150, (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt150 )        },	// Ｍ 現像ユニットニアライフ警告検出（パネル確認用）.
		{ TYPE_NTF_DataID_WRN_PRT_151, (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt151 )        },	// Ｙ 現像ユニットニアライフ警告検出（パネル確認用）.
		{ TYPE_NTF_DataID_WRN_PRT_152, (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt152 )        },	// Ｋ 現像ユニットニアライフ警告検出（パネル確認用）.
		{ TYPE_NTF_DataID_WRN_PRT_153, (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt153 )        },	// Ｃ 現像ユニットライフストップ警告検出（パネル確認用）.
		{ TYPE_NTF_DataID_WRN_PRT_154, (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt154 )        },	// Ｍ 現像ユニットライフストップ警告検出（パネル確認用）.
		{ TYPE_NTF_DataID_WRN_PRT_155, (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt155 )        },	// Ｙ 現像ユニットライフストップ警告検出（パネル確認用）.
		{ TYPE_NTF_DataID_WRN_PRT_156, (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt156 )        },	// Ｋ 現像ユニットライフストップ警告検出（パネル確認用）.
		{ TYPE_NTF_DataID_WRN_PRT_157, (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt157 )        },	// Ｋ 現像剤ニアライフ警告検出（パネル確認用）.
		{ TYPE_NTF_DataID_WRN_PRT_158, (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt158 )        },	// Ｋ 現像剤ライフストップ警告検出（パネル確認用）.
		{ TYPE_NTF_DataID_WRN_PRT_159, (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt159 )        },	// 定着ユニットニアライフ警告検出（パネル確認用）.
		{ TYPE_NTF_DataID_WRN_PRT_160, (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt160 )        },	// 定着ユニットライフストップ警告検出（パネル確認用）.
		{ TYPE_NTF_DataID_WRN_PRT_161, (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt161 )        },	// 転写ベルトニアライフ警告検出（パネル確認用）.
		{ TYPE_NTF_DataID_WRN_PRT_162, (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt162 )        },	// 転写ベルトライフストップ警告検出（パネル確認用）.
		{ TYPE_NTF_DataID_WRN_PRT_163, (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt163 )        },	// ライフストップ超過警告検出（オイル指しマーク表示用）.
// ADS坪井 → 2011/12/01 【パフォーマンス改善】isWarning対応 End.
		{ TYPE_NTF_DataID_WRN_SCN_1  , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_ScnWarningInfoParam.b_WrnScn1  )         },	// スキャナー関連:ＩＲ関連異常警告.
		{ TYPE_NTF_DataID_WRN_SCN_2  , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_ScnWarningInfoParam.b_WrnScn2  )         },	// スキャナー関連:プラテン上原稿取り忘れ.
		{ TYPE_NTF_DataID_WRN_SCN_3  , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_ScnWarningInfoParam.b_WrnScn3  )         },	// スキャナー関連:ＡＤＦドアオープン.
		{ TYPE_NTF_DataID_WRN_SCN_4  , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_ScnWarningInfoParam.b_WrnScn4  )         },	// スキャナー関連:ＡＤＦＪＡＭ.
		{ TYPE_NTF_DataID_WRN_SCN_5  , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_ScnWarningInfoParam.b_WrnScn5  )         },	// スキャナー関連:原稿ノーフィードエラー(LaPlata固有).
		{ TYPE_NTF_DataID_WRN_SCN_6  , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_ScnWarningInfoParam.b_WrnScn6  )         },	// スキャナー関連:混載原稿禁止サイズエラー(LaPlata固有).
		{ TYPE_NTF_DataID_WRN_SCN_7  , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_ScnWarningInfoParam.b_WrnScn7  )         },	// スキャナー関連:原稿サイズ混載エラー(EDH機固有).
		{ TYPE_NTF_DataID_WRN_SCN_8  , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_ScnWarningInfoParam.b_WrnScn8  )         },	// スキャナー関連:次原稿エラー(LaPlata固有).
		{ TYPE_NTF_DataID_WRN_SCN_9  , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_ScnWarningInfoParam.b_WrnScn9  )         },	// スキャナー関連:ＥＤＨ原稿ガラス汚れ警告(CAUTION1).
		{ TYPE_NTF_DataID_WRN_SCN_10 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_ScnWarningInfoParam.b_WrnScn10 )         },	// スキャナー関連:ＥＤＨ原稿ガラス汚れ警告(CAUTION2).
		{ TYPE_NTF_DataID_WRN_SCN_11 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_ScnWarningInfoParam.b_WrnScn11 )         },	// スキャナー関連:ＥＤＨ原稿ガラス汚れ警告(ATTENTION).
		{ TYPE_NTF_DataID_WRN_SCN_12 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_ScnWarningInfoParam.b_WrnScn12 )         },	// スキャナー関連:ＥＤＨ背景板汚れ警告.
		{ TYPE_NTF_DataID_WRN_SCN_13 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_ScnWarningInfoParam.b_WrnScn13 )         },	// スキャナー関連:ＥＤＨ原稿容量オーバー警告.
		{ TYPE_NTF_DataID_WRN_SCN_14 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_ScnWarningInfoParam.b_WrnScn14 )         },	// スキャナー関連:フリーイレースプレスキャンエラー.
		{ TYPE_NTF_DataID_WRN_SCN_15 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_ScnWarningInfoParam.b_WrnScn15 )         },	// スキャナー関連:ＥＤＨ原稿セット不良警告.
		{ TYPE_NTF_DataID_WRN_SCN_16 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_ScnWarningInfoParam.b_WrnScn16 )         },	// スキャナー関連:ＣＩＳ読取りガラス汚れ警告(CAUTION1).
		{ TYPE_NTF_DataID_WRN_SCN_17 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_ScnWarningInfoParam.b_WrnScn17 )         },	// スキャナー関連:ＣＩＳ読取りガラス汚れ警告(CAUTION2).
		{ TYPE_NTF_DataID_WRN_SCN_18 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_ScnWarningInfoParam.b_WrnScn18 )         },	// スキャナー関連:ＣＩＳ読取りガラス汚れ警告(ATTENTION).
		{ TYPE_NTF_DataID_WRN_USB_1  , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_UsbWarningInfoParam.b_WrnScnUsb1 )       },	// ファイル数オーバー.
		{ TYPE_NTF_DataID_WRN_USB_2  , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_UsbWarningInfoParam.b_WrnScnUsb2 )       },	// メモリフル.
		{ TYPE_NTF_DataID_WRN_USB_3  , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_UsbWarningInfoParam.b_WrnScnUsb3 )       },	// USB２つ挿し.
		{ TYPE_NTF_DataID_WRN_USB_4  , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_UsbWarningInfoParam.b_WrnScnUsb4 )       },	// 書き込み中のUSBメモリ引き抜き.
		{ TYPE_NTF_DataID_WRN_USB_5  , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_UsbWarningInfoParam.b_WrnScnUsb5 )       },	// 全て.
// ADS坪井 → 2011/12/01 【パフォーマンス改善】isWarning対応 Start.
		{ TYPE_NTF_DataID_WRN_USB_6  , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_UsbWarningInfoParam.b_WrnScnUsb6 )       },	// USBメモリ引き抜き要求状態.
		{ TYPE_NTF_DataID_WRN_USB_7  , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_UsbWarningInfoParam.b_WrnScnUsb7 )       },	// デバッグログ蓄積不可状態.
// ADS坪井 → 2011/12/01 【パフォーマンス改善】isWarning対応 End.
// 2016/07/01 2Sys-Sky西村 RQ-436対応 start.
		{ TYPE_NTF_DataID_WRN_IWS_1  , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_IwsWarningInfoParam.b_WrnIws1 )          },	// IWSバックグラウンドアプリ使用不可.
// 2016/07/01 2Sys-Sky西村 RQ-436対応 end.
// 【RQ-747 VirusScanFUM for 機種】 start.
		{ TYPE_NTF_DataID_WRN_VRS_1  , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_VrsWarningInfoParam.b_WrnVrs1 )          },	// ウイルス検知警告（Attention）.
		{ TYPE_NTF_DataID_WRN_VRS_2  , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_VrsWarningInfoParam.b_WrnVrs2 )          },	// パターンファイル有無（Attention）.
// 【RQ-747 VirusScanFUM for 機種】 end.
// 2018/08/30 [RQ-674] SCD2-motohashi ログ遅延警告を追加.
		{ TYPE_NTF_DataID_WRN_JLG_1  , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_WarningInfoParam.st_JlgWarningInfoParan.b_WrnJlg1 )          },	// ログ遅延警告.
// 2020/12/07 APF OP_BTS-23544_Add NFCデバイス異常警告追加対応 Start.
		{ TYPE_NTF_DataID_WRN_NFC_1  , (uint32_t)sizeof(Bool)                                   , &( m_stSysStatInfo.st_WarningInfoParam.st_NfcWarningInfoParam.b_WrnNfc1 )         },	// NFCデバイス異常警告（Attention）.
// 2020/12/07 APF OP_BTS-23544_Add NFCデバイス異常警告追加対応 End.
		// [SystemStatus] データなしObserverのデータ追加対応：2011.12.15 <ADD> Start.
		{ TYPE_NTF_DataID_RPL_1		 , (uint32_t)sizeof(TYPS_SysStatusPrinterSystem)					, &( m_stSysStatInfo.st_RemotePanelInfoParam.st_Rpl1 )						},	// プリンタシステム変化情報.
		{ TYPE_NTF_DataID_RPL_2		 , (uint32_t)sizeof(TYPS_SysStatusRemotePanelStatusFiery)			, &( m_stSysStatInfo.st_RemotePanelInfoParam.st_Rpl2 )						},	// リモートパネル状態変化情報.
		{ TYPE_NTF_DataID_RPL_3		 , (uint32_t)sizeof(TYPS_SysStatusRemotePanelStatusRequestDisplay)	, &( m_stSysStatInfo.st_RemotePanelInfoParam.st_Rpl3 )						},	// リモートパネル状態変化情報.
		{ TYPE_NTF_DataID_RPL_4		 , (uint32_t)sizeof(TYPS_SysStatusRemotePanelStatusCommunication)	, &( m_stSysStatInfo.st_RemotePanelInfoParam.st_Rpl4 )						},	// リモートパネル状態変化情報.
		{ TYPE_NTF_DataID_RPL_5		 , (uint32_t)sizeof(TYPS_SysStatusRemotePanelStatusFunctionNotice)	, &( m_stSysStatInfo.st_RemotePanelInfoParam.st_Rpl5 )						},	// リモートパネル状態変化情報.
		{ TYPE_NTF_DataID_RPL_6		 , (uint32_t)sizeof(TYPS_SysStatusRemotePanelStatusStatusNotice)	, &( m_stSysStatInfo.st_RemotePanelInfoParam.st_Rpl6 )						},	// リモートパネル状態変化情報.
		{ TYPE_NTF_DataID_RPL_8		 , (uint32_t)sizeof(TYPS_SysStatusRemotePanelStatusRMP_Message)		, &( m_stSysStatInfo.st_RemotePanelInfoParam.st_Rpl8 )						},	// リモートパネル状態変化情報.
		{ TYPE_NTF_DataID_RPL_9		 , (uint32_t)sizeof(TYPS_SysStatusRemotePanelStatusRMP_Message)		, &( m_stSysStatInfo.st_RemotePanelInfoParam.st_Rpl9 )						},	// リモートパネル状態変化情報(KeyOff).
		{ TYPE_NTF_DataID_RPL_10	 , (uint32_t)sizeof(Bool)									 		, &( m_stSysStatInfo.st_RemotePanelInfoParam.b_Rpl10 )						},	// 遠隔操作Androidパネルとの接続有無 (True:接続状態 , False:未接続).

		// [SystemStatus] データなしObserverのデータ追加対応：2011.12.15 <ADD> End.
		{ TYPE_NTF_DataID_ADR_1      , (uint32_t)sizeof(TYPE_COMMON_Result)                    , &( m_stSysStatInfo.st_AddressInfoParam.e_Adr1 )                                    },	// 宛先設定結果通知 受付結果.
		{ TYPE_NTF_DataID_ADR_2      , (uint32_t)sizeof(TYPS_SysStatusAddressNumber)           , &( m_stSysStatInfo.st_AddressInfoParam.st_Adr2 )                                   },	// アドレス情報：宛先登録番号変更通知用パラメータ.
		{ TYPE_NTF_DataID_ADR_3      , (uint32_t)sizeof(TYPE_COMMON_Result)                    , &( m_stSysStatInfo.st_AddressInfoParam.e_Adr3 )                                    },	// 自動転送BOX - 宛先設定結果.
		{ TYPE_NTF_DataID_SRV_1      , (uint32_t)sizeof(short)                                 , &( m_stSysStatInfo.st_ServiceInfoParam.ss_Srv1 )                                   },	// ジョブログファイル作成状況通知用ファイル状態.
		{ TYPE_NTF_DataID_SRV_2      , (uint32_t)sizeof(JLGE_Observer_FileSts)                 , &( m_stSysStatInfo.st_ServiceInfoParam.e_Srv2 )                                    },	// ジョブログファイル状態変化通知用ファイル状態.
		{ TYPE_NTF_DataID_SRV_3      , (uint32_t)sizeof(JLGE_Observer_DataSts)                 , &( m_stSysStatInfo.st_ServiceInfoParam.e_Srv3 )                                    },	// ジョブログフル通知用データ状態.
		{ TYPE_NTF_DataID_SRV_4      , (uint32_t)sizeof(JLGE_Observer_LogSts)                  , &( m_stSysStatInfo.st_ServiceInfoParam.e_Srv4 )                                    },	// ジョブログ一時ログ状態変化通知用ログ状態.
		{ TYPE_NTF_DataID_SRV_5      , (uint32_t)sizeof(JLGE_Observer_JobLogSts)               , &( m_stSysStatInfo.st_ServiceInfoParam.e_Srv5 )                                    },	// ジョブログ状態変化通知用ジョブログ状態.
		
		// ATS/AMS対応：2011.05.10 <ADD> Start T.Takahashi.
		{ TYPE_NTF_DataID_SRV_10     , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_ServiceInfoParam.b_Srv10 )                                   },	// 拡大ローテーション使用情報設定.
		{ TYPE_NTF_DataID_SRV_11     , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_ServiceInfoParam.b_Srv11 )                                   },	// 拡大ローテーション時AMS倍率方向不可警告設定.
		// ATS/AMS対応：2011.05.10 <ADD> End T.Takahashi.
		
		// 新API対応：2011.05.26 <ADD> Start T.Takahashi.
		{ TYPE_NTF_DataID_SRV_12     , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_ServiceInfoParam.b_Srv12 )                                   },	// 中とじ/中折り時の小冊子選択.
		{ TYPE_NTF_DataID_SRV_13     , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_ServiceInfoParam.b_Srv13 )                                   },	// Ｎｉｎ１優先倍率設定.
		// 新API対応：2011.05.26 <ADD> End T.Takahashi.
		// Job実行可否対応 2011.11.22 ADS K.Tsuboi Start.
		{ TYPE_NTF_DataID_SRV_14     , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_ServiceInfoParam.b_Srv14 )                                   },	// パスワードコピー印字効果設定.
		{ TYPE_NTF_DataID_SRV_15     , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_ServiceInfoParam.b_Srv15 )                                   },	// コピーガード印字効果設定.
		// Job実行可否対応 2011.11.22 ADS K.Tsuboi End.
		
		{ TYPE_NTF_DataID_SYS_3      , (uint32_t)sizeof(TYPS_SysStatusFatal)                   , &( m_stSysStatInfo.st_SystemInfoParam.st_Sys3 )									},	// FATAL変化情報 [SystemStatus] データなしObserverのデータ追加対応：2011.12.15 <ADD>.
		{ TYPE_NTF_DataID_SYS_4      , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_SystemInfoParam.b_Sys4 )										},	// 管理者モード変更通知用管理者モード判定フラグ.
		{ TYPE_NTF_DataID_SYS_8      , (uint32_t)sizeof(TYPS_SysStatusSystem)                  , &( m_stSysStatInfo.st_SystemInfoParam.st_Sys8 )									},	// システム状態変化情報 [SystemStatus] データなしObserverのデータ追加対応：2011.12.15 <ADD>.
		{ TYPE_NTF_DataID_SYS_9      , (uint32_t)sizeof(TYPE_BmlPrintStatus)                   , &( m_stSysStatInfo.st_SystemInfoParam.e_Sys9 )										},	// BMLinkS状態のデータ変化通知用状態.
		{ TYPE_NTF_DataID_SYS_10     , (uint32_t)sizeof(TYPE_BmlinksParam)                     , &( m_stSysStatInfo.st_SystemInfoParam.e_Sys10 )									},	// BMLinkS設定のデータ変化通知用状態.
		{ TYPE_NTF_DataID_SYS_13     , (uint32_t)sizeof(TYPS_SysStatusHddInfo)                 , &( m_stSysStatInfo.st_SystemInfoParam.st_Sys13 )									},	// ＨＤＤ情報のデータ変化情報 [SystemStatus] データなしObserverのデータ追加対応：2011.12.15 <ADD>.
		
		// STEP2：2010.12.10 <ADD> Start T.Takahashi.
		{ TYPE_NTF_DataID_SYS_14     , (uint32_t)SYSD_MAX_SysDescr                             , &( m_stSysStatInfo.st_SystemInfoParam.sc_Sys14 )                                   },	// 製品名情報.
		{ TYPE_NTF_DataID_SYS_15     , (uint32_t)TYPE_RomMax * TYPD_SSTS_RomVersionLength      , &( m_stSysStatInfo.st_SystemInfoParam.sc_Sys15 )                                   },	// RomVersion情報.
		{ TYPE_NTF_DataID_SYS_16     , (uint32_t)TYPE_RomMax * sizeof(ushort)                  , &( m_stSysStatInfo.st_SystemInfoParam.us_Sys16 )                                   },	// Romチェックサム.
		{ TYPE_NTF_DataID_SYS_17     , (uint32_t)TYPE_MFPControllerRomMax * TYPD_SSTS_RomVersionLength, &( m_stSysStatInfo.st_SystemInfoParam.sc_Sys17 )                            },	// ＭＦＰコントローラ内のサブＲＯＭバージョン.
		{ TYPE_NTF_DataID_SYS_18     , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_SystemInfoParam.b_Sys18 )                                    },	// 製品タイプ情報.
		{ TYPE_NTF_DataID_SYS_19     , (uint32_t)sizeof(TYPE_VersionOfModel)                   , &( m_stSysStatInfo.st_SystemInfoParam.e_Sys19 )                                    },	// モデル( Low / Standard ).
		{ TYPE_NTF_DataID_SYS_20     , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_SystemInfoParam.b_Sys20 )                                    },	// カラー機／モノクロ機.
		{ TYPE_NTF_DataID_SYS_21     , (uint32_t)sizeof(short)                                 , &( m_stSysStatInfo.st_SystemInfoParam.ss_Sys21 )                                   },	// エンジン解像度.
		{ TYPE_NTF_DataID_SYS_22     , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_SystemInfoParam.b_Sys22 )                                    },	// スキャナー有り.
		{ TYPE_NTF_DataID_SYS_23     , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_SystemInfoParam.b_Sys23 )                                    },	// ＰＣＩ拡張基板有無.
		{ TYPE_NTF_DataID_SYS_24     , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_SystemInfoParam.b_Sys24 )                                    },	// ローカルＩ／Ｆ有無.
		{ TYPE_NTF_DataID_SYS_25     , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_SystemInfoParam.b_Sys25 )                                    },	// JpegAsic有り 判定.
		{ TYPE_NTF_DataID_SYS_26     , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_SystemInfoParam.b_Sys26 )                                    },	// ＳＩＰアダプタ有無(バックアップ必要) .
		{ TYPE_NTF_DataID_SYS_27     , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_SystemInfoParam.b_Sys27 )                                    },	// 暗号化基板有無.
		{ TYPE_NTF_DataID_SYS_28     , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_SystemInfoParam.b_Sys28 )                                    },	// USBホスト基板有り.
		{ TYPE_NTF_DataID_SYS_29     , (uint32_t)TYPD_SSTS_SerialNumberLength                  , &( m_stSysStatInfo.st_SystemInfoParam.sc_Sys29 )                                   },	// シリアルナンバー・プリンタ.
		{ TYPE_NTF_DataID_SYS_30     , (uint32_t)TYPD_SSTS_SerialNumberLength                  , &( m_stSysStatInfo.st_SystemInfoParam.sc_Sys30 )                                   },	// シリアルナンバー・スキャナ.
		{ TYPE_NTF_DataID_SYS_31     , (uint32_t)TYPD_SSTS_SerialNumberLength                  , &( m_stSysStatInfo.st_SystemInfoParam.sc_Sys31 )                                   },	// シリアルナンバー・ＡＤＦ.
		{ TYPE_NTF_DataID_SYS_32     , (uint32_t)TYPD_SSTS_SerialNumberLength                  , &( m_stSysStatInfo.st_SystemInfoParam.sc_Sys32 )                                   },	// シリアルナンバー・ＬＣＣ.
		{ TYPE_NTF_DataID_SYS_33     , (uint32_t)TYPD_SSTS_SerialNumberLength                  , &( m_stSysStatInfo.st_SystemInfoParam.sc_Sys33 )                                   },	// シリアルナンバー・排紙ＯＰ.
		{ TYPE_NTF_DataID_SYS_34     , (uint32_t)TYPD_SSTS_SerialNumberLength                  , &( m_stSysStatInfo.st_SystemInfoParam.sc_Sys34 )                                   },	// シリアルナンバー・Ｚ折りユニット.
		{ TYPE_NTF_DataID_SYS_35     , (uint32_t)TYPD_SSTS_SerialNumberLength                  , &( m_stSysStatInfo.st_SystemInfoParam.sc_Sys35 )                                   },	// シリアルナンバー・リレーユニット.
		{ TYPE_NTF_DataID_SYS_36     , (uint32_t)TYPD_SSTS_SerialNumberLength                  , &( m_stSysStatInfo.st_SystemInfoParam.sc_Sys36 )                                   },	// シリアルナンバー・再給紙.
		{ TYPE_NTF_DataID_SYS_37     , (uint32_t)TYPD_SSTS_SerialNumberLength                  , &( m_stSysStatInfo.st_SystemInfoParam.sc_Sys37 )                                   },	// シリアルナンバー・ピーベンダー.
		{ TYPE_NTF_DataID_SYS_38     , (uint32_t)TYPD_SSTS_SerialNumberLength                  , &( m_stSysStatInfo.st_SystemInfoParam.sc_Sys38 )                                   },	// シリアルナンバー・ファクス１.
		{ TYPE_NTF_DataID_SYS_39     , (uint32_t)TYPD_SSTS_SerialNumberLength                  , &( m_stSysStatInfo.st_SystemInfoParam.sc_Sys39 )                                   },	// シリアルナンバー・ファクス２.
		{ TYPE_NTF_DataID_SYS_40     , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_SystemInfoParam.b_Sys40 )                                    },	// Etherインストール.
		{ TYPE_NTF_DataID_SYS_41     , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_SystemInfoParam.b_Sys41 )                                    },	// Wirelessインストール.
		{ TYPE_NTF_DataID_SYS_42     , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_SystemInfoParam.b_Sys42 )                                    },	// TokenRingインストール.
		{ TYPE_NTF_DataID_SYS_43     , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_SystemInfoParam.b_Sys43 )                                    },	// USBインストール.
		{ TYPE_NTF_DataID_SYS_44     , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_SystemInfoParam.b_Sys44 )                                    },	// IEEE1394インストール.
		{ TYPE_NTF_DataID_SYS_45     , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_SystemInfoParam.b_Sys45 )                                    },	// プロテクションチップ情報.
		{ TYPE_NTF_DataID_SYS_46     , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_SystemInfoParam.b_Sys46 )                                    },	// 起動時に特別なモード(サービスマンモード)で起動したか.
		{ TYPE_NTF_DataID_SYS_47     , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_SystemInfoParam.b_Sys47 )                                    },	// 起動時に特別なモード(カードラインモード)で起動したか.
		{ TYPE_NTF_DataID_SYS_48     , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_SystemInfoParam.b_Sys48 )                                    },	// 特定原稿検出エラー.
		{ TYPE_NTF_DataID_SYS_49     , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_SystemInfoParam.b_Sys49 )                                    },	// エンジン出荷仕向け.
		{ TYPE_NTF_DataID_SYS_50     , (uint32_t)TYPD_SSTS_MakerNameLength                     , &( m_stSysStatInfo.st_SystemInfoParam.sc_Sys50 )                                   },	// メーカー名.
		{ TYPE_NTF_DataID_SYS_51     , (uint32_t)TYPD_SSTS_ShortMakerNameLength                , &( m_stSysStatInfo.st_SystemInfoParam.sc_Sys51 )                                   },	// 省略メーカー名.
		{ TYPE_NTF_DataID_SYS_52     , (uint32_t)TYPD_SSTS_BrandNameLength                     , &( m_stSysStatInfo.st_SystemInfoParam.sc_Sys52 )                                   },	// ブランド名.
		{ TYPE_NTF_DataID_SYS_53     , (uint32_t)TYPD_SSTS_ModelNameLength                     , &( m_stSysStatInfo.st_SystemInfoParam.sc_Sys53 )                                   },	// 製品名(モデル名).
		{ TYPE_NTF_DataID_SYS_54     , (uint32_t)TYPD_MSK_PreRomVersion                        , &( m_stSysStatInfo.st_SystemInfoParam.sc_Sys54 )                                   },	// Pre Rom Version 設定.
		{ TYPE_NTF_DataID_SYS_55     , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_SystemInfoParam.b_Sys55 )                                    },	// トラブルリセットモード.
		{ TYPE_NTF_DataID_SYS_56     , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_SystemInfoParam.b_Sys56 )                                    },	// FirmWare ダウンロード 設定.
		{ TYPE_NTF_DataID_SYS_57     , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_SystemInfoParam.b_Sys57 )                                    },	// パネル起動完了済みの設定 取得.
		{ TYPE_NTF_DataID_SYS_58     , (uint32_t)sizeof(TYPE_PrinterModelKind)                 , &( m_stSysStatInfo.st_SystemInfoParam.e_Sys58 )                                    },	// Thames1P モデル.
		{ TYPE_NTF_DataID_SYS_59     , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_SystemInfoParam.b_Sys59 )                                    },	// Thames1P 強制切替モード設定.
		{ TYPE_NTF_DataID_SYS_60     , (uint32_t)sizeof(TYPE_PrinterModelKind)                 , &( m_stSysStatInfo.st_SystemInfoParam.e_Sys60 )                                    },	// Thames1P 強制モデル設定.
		{ TYPE_NTF_DataID_SYS_61     , (uint32_t)sizeof(TYPS_IFunctionOn_InfoTubeOn)           , &( m_stSysStatInfo.st_SystemInfoParam.st_Sys61 )                                   },	// 起動時にiモデルかどうか判定.
		{ TYPE_NTF_DataID_SYS_62     , (uint32_t)sizeof(TYPE_FirmWareVersion)                  , &( m_stSysStatInfo.st_SystemInfoParam.e_Sys62 )                                    },	// Ｆ／Ｗバージョン.
		{ TYPE_NTF_DataID_SYS_63     , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_SystemInfoParam.b_Sys63 )                                    },	// Emu機判定.
		{ TYPE_NTF_DataID_SYS_64     , (uint32_t)sizeof(SYSE_FuncVer)                          , &( m_stSysStatInfo.st_SystemInfoParam.e_Sys64 )                                    },	// MFPの機能バージョン.
		{ TYPE_NTF_DataID_SYS_65     , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_SystemInfoParam.b_Sys65 )                                    },	// i-option kit装着有無取得.
		{ TYPE_NTF_DataID_SYS_66     , (uint32_t)sizeof(uint32_t)                              , &( m_stSysStatInfo.st_SystemInfoParam.ul_Sys66 )                                   },	// 指定された機能を実行するのに必要なH/Wリソースが存在するか.
		{ TYPE_NTF_DataID_SYS_67     , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_SystemInfoParam.b_Sys67 )                                    },	// DSC1基板装着状態.
		{ TYPE_NTF_DataID_SYS_68     , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_SystemInfoParam.b_Sys68 )                                    },	// DSC2基板装着状態.
		{ TYPE_NTF_DataID_SYS_69     , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_SystemInfoParam.b_Sys69 )                                    },	// MFPが外部メモリから起動されたかどうかを判別する.
		{ TYPE_NTF_DataID_SYS_70     , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_SystemInfoParam.b_Sys70 )                                    },	// 製品機判定.
		{ TYPE_NTF_DataID_SYS_71     , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_SystemInfoParam.b_Sys71 )                                    },	// Bluetooth機能設定.
		{ TYPE_NTF_DataID_SYS_72     , (uint32_t)sizeof(ushort)                                , &( m_stSysStatInfo.st_SystemInfoParam.us_Sys72 )                                   },	// アウトラインフォントデータのチェックサム値.
		{ TYPE_NTF_DataID_SYS_73     , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_SystemInfoParam.b_Sys73 )                                    },	// 1パス両面 裏面用Asic200基板 装着状態取得.
		{ TYPE_NTF_DataID_SYS_74     , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_SystemInfoParam.b_Sys74 )                                    },	// DSモデルであるか否かの判断.
		// JobService関連追加：2011.02.09 <ADD> START T.Takahashi.
		{ TYPE_NTF_DataID_SYS_75     , (uint32_t)sizeof(TYPE_ProgJobInStatus)                  , &( m_stSysStatInfo.st_SystemInfoParam.e_Sys75 )                                    },	// プログラムジョブ用Inputステータス.
		{ TYPE_NTF_DataID_SYS_76     , (uint32_t)sizeof(TYPE_ProgJobOutStatus)                 , &( m_stSysStatInfo.st_SystemInfoParam.e_Sys76 )                                    },	// プログラムジョブ用Outputステータス.
		{ TYPE_NTF_DataID_SYS_77     , (uint32_t)sizeof(int32_t)                               , &( m_stSysStatInfo.st_SystemInfoParam.sl_Sys77 )                                   },	// プログラムジョブの最後に投入されたDocID.
		{ TYPE_NTF_DataID_SYS_78     , (uint32_t)sizeof(int32_t)                               , &( m_stSysStatInfo.st_SystemInfoParam.sl_Sys78 )                                   },	// 部数（プログラムジョブ用）.
		// JobService関連追加：2011.02.09 <ADD> End   T.Takahashi.
		
		// ATS/AMS対応：2011.05.10 <ADD> Start T.Takahashi.
		{ TYPE_NTF_DataID_SYS_79     , (uint32_t)sizeof(char)                                  , &( m_stSysStatInfo.st_SystemInfoParam.sc_Sys79 )                                   },	// 認証不成功時に操作禁止とする回数.
		{ TYPE_NTF_DataID_SYS_80     , (uint32_t)sizeof(int32_t)                               , &( m_stSysStatInfo.st_SystemInfoParam.sl_Sys80 )                                   },	// プログラムジョブDocIDの登録数.
		// ATS/AMS対応：2011.05.10 <ADD> End T.Takahashi.
		
		// 新API対応：2011.05.26 <ADD> Start T.Takahashi.
		{ TYPE_NTF_DataID_SYS_81     , (uint32_t)sizeof(char) * TYPD_SSTS_NotifyEmailAdminIdMax * TYPD_SSTS_MAX_StatusNoticeMailAdrs, &( m_stSysStatInfo.st_SystemInfoParam.uc_Sys81 )},	// 機械状態通知管理者.
		{ TYPE_NTF_DataID_SYS_82     , (uint32_t)sizeof(char) * TYPD_SSTS_AdminMailAddress     , &( m_stSysStatInfo.st_SystemInfoParam.uc_Sys82 )                                   },	// 管理者のメールアドレス.
		{ TYPE_NTF_DataID_SYS_83     , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_SystemInfoParam.b_Sys83 )                                    },	// ＦＡＸ１有り.
		{ TYPE_NTF_DataID_SYS_84     , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_SystemInfoParam.b_Sys84 )                                    },	// ＦＡＸ２有り.
		{ TYPE_NTF_DataID_SYS_85     , (uint32_t)sizeof(TYPE_CategoryState) * TYPD_SSTS_MaxFEKType * TYPD_SSTS_TruthMax, &( m_stSysStatInfo.st_SystemInfoParam.e_Sys85 )            },	// FEK有効無効状態.
		{ TYPE_NTF_DataID_SYS_86     , (uint32_t)sizeof(TYPS_SysStatusPMInfo)                  , &( m_stSysStatInfo.st_SystemInfoParam.st_Sys86 )                                   },	// 電源管理変化情報.
		{ TYPE_NTF_DataID_SYS_92     , (uint32_t)sizeof(TYPE_ExternalIFMode)                   , &( m_stSysStatInfo.st_SystemInfoParam.e_Sys92 )                                    },	// 外部Ｉ／Ｆ選択.
		// 新API対応：2011.05.26 <ADD> End   T.Takahashi.
		
		// HDD関連通知対応 2011.06.30 Start T.Takahashi.
		{ TYPE_NTF_DataID_SYS_93     , (uint32_t)TYPD_MAX_LogEncryptionPassword                , &( m_stSysStatInfo.st_SystemInfoParam.uc_Sys93 )                                   },	// HDD蓄積ログ暗号化パスワード取得.
		// HDD関連通知対応 2011.06.30 Start T.Takahashi.
// CQ376793 HDDフォーマット中状態の共有メモリへの追加 2013/1/7 Soma Start.
		{ TYPE_NTF_DataID_SYS_94     , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_SystemInfoParam.b_Sys94 )                                    },	// HDDフォーマット中状態.
// CQ376793 HDDフォーマット中状態の共有メモリへの追加 2013/1/7 Soma End.
		// Job実行可否対応 2011.11.22 ADS K.Tsuboi Start.
		{ TYPE_NTF_DataID_SYS_96     , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_SystemInfoParam.b_Sys96 )                                    },	// サービスモード.
		{ TYPE_NTF_DataID_SYS_97     , (uint32_t)sizeof(TYPE_AdminModeApp)                     , &( m_stSysStatInfo.st_SystemInfoParam.e_Sys97 )                                    },	// 管理者モード中（使用アプリ）.
		{ TYPE_NTF_DataID_SYS_98     , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_SystemInfoParam.b_Sys98 )                                    },	// キーオペレータモード中.
		{ TYPE_NTF_DataID_SYS_99     , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_SystemInfoParam.b_Sys99 )                                    },	// ユーザチョイス変更設定中 .
		{ TYPE_NTF_DataID_SYS_100    , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_SystemInfoParam.b_Sys100 )                                   },	// 割込コピー中.
		{ TYPE_NTF_DataID_SYS_101    , (uint32_t)sizeof(Bool) * TYPE_TemporaryRescueUnitMax    , &( m_stSysStatInfo.st_SystemInfoParam.b_Sys101 )                                   },	// 一時切り離しON/OFF（一時切り離し対象指定）.
		{ TYPE_NTF_DataID_SYS_102    , (uint32_t)sizeof(Bool) * TYPE_TemporaryRescueUnitMax    , &( m_stSysStatInfo.st_SystemInfoParam.b_Sys102 )                                   },	// 定常切り離しON/OFF（一時切り離し対象指定）.
		{ TYPE_NTF_DataID_SYS_103    , (uint32_t)sizeof(Bool) * TYPE_JobEnableCheckStatusMax   , &( m_stSysStatInfo.st_SystemInfoParam.b_Sys103 )                                   },	// ジョブ起動チェック.
		{ TYPE_NTF_DataID_SYS_104    , (uint32_t)sizeof(TYPE_TwainLock)                        , &( m_stSysStatInfo.st_SystemInfoParam.e_Sys104 )                                   },	// Ｔｗａｉｎロック状態 .
		{ TYPE_NTF_DataID_SYS_105    , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_SystemInfoParam.b_Sys105 )                                   },	// デバイスロック中.
		{ TYPE_NTF_DataID_SYS_106    , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_SystemInfoParam.b_Sys106 )                                   },	// FATALが発生しているか.
		{ TYPE_NTF_DataID_SYS_107    , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_SystemInfoParam.b_Sys107 )                                   },	// 長尺プリントのON/OFF設定.
		// Job実行可否対応 2011.11.22 ADS K.Tsuboi End.
		{ TYPE_NTF_DataID_SYS_108    , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_SystemInfoParam.b_Sys108 )                                   },	// 通信履歴表示設定.
		{ TYPE_NTF_DataID_SYS_109    , (uint32_t)sizeof(TYPE_UserAuthentication)               , &( m_stSysStatInfo.st_SystemInfoParam.e_Sys109 )                                   },	// ユーザー認証設定.
//2014.03.13 OPS1(MSE) 野口 ジョブ実行中の管理者ログイン 新API化対応 ADD ST.
		{ TYPE_NTF_DataID_SYS_112    , (uint32_t)sizeof(TYPE_AdminLoginMode)                   , &( m_stSysStatInfo.st_SystemInfoParam.e_Sys112 )                                   },	// 管理者ログイン先情報.
//2014.03.13 OPS1(MSE) 野口 ジョブ実行中の管理者ログイン 新API化対応 ADD ED.
		{ TYPE_NTF_DataID_SYS_113    , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_SystemInfoParam.b_Sys113 )                                   },	// ZeusSZX0 CQ517818 s.
// Eagle:Sparrow:制御構成変更_No.91_ハードキーレス対応 start.
		{ TYPE_NTF_DataID_SYS_115    , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_SystemInfoParam.b_Sys115 )                                   },	// デバッグ情報の取出し設定.
// Eagle:Sparrow:制御構成変更_No.91_ハードキーレス対応 end.
// Eagle:制御構成変更No.104_テストパターン印刷のターミナルコマンド対応(生産対応）start.
		{ TYPE_NTF_DataID_SYS_116    , (uint32_t)sizeof(TYPS_TestPatternPrintInfo)             , &( m_stSysStatInfo.st_SystemInfoParam.st_Sys116 )                                  },	// テストパターン印刷要求.
// Eagle:制御構成変更No.104_テストパターン印刷のターミナルコマンド対応(生産対応）end.
		{ TYPE_NTF_DataID_SYS_117    , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_SystemInfoParam.b_Sys117 )                                   },	// HDD物理フォーマット中状態.
// comment [特ROM対応][IT5-148]【SOLIMP3様】音声操作によるジョブ実行(IWS連携)要望_最終版リリース ->.
		{ TYPE_NTF_DataID_SYS_118    , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_SystemInfoParam.b_Sys118 )                                   },	// IWS_WebAPIからのジョブ投入.
// comment [特ROM対応][IT5-148]【SOLIMP3様】音声操作によるジョブ実行(IWS連携)要望_最終版リリース <-.
		
		// [SystemStatus] データなしObserverのデータ追加対応：2011.12.15 <ADD> Start.
		{ TYPE_NTF_DataID_SCN_4		 , (uint32_t)sizeof(TYPS_SysStatusScannerServiceInfo)	   , &( m_stSysStatInfo.st_ScannerInfoParam.st_Scn4 )									},	// スキャナサービス情報状態変化.
		{ TYPE_NTF_DataID_SCN_5		 , (uint32_t)sizeof(TYPS_SysStatusScannerServiceSetting)   , &( m_stSysStatInfo.st_ScannerInfoParam.st_Scn5 )									},	// スキャナサービス設定状態変化.
		{ TYPE_NTF_DataID_SCN_6		 , (uint32_t)sizeof(TYPS_SysStatusScannerStatus)		   , &( m_stSysStatInfo.st_ScannerInfoParam.st_Scn6 )									},	// スキャナ状態変化.
		{ TYPE_NTF_DataID_SCN_7		 , (uint32_t)sizeof(TYPS_SysStatusScannerSystem)		   , &( m_stSysStatInfo.st_ScannerInfoParam.st_Scn7 )									},	// スキャナシステム構成状態変化.
		// [SystemStatus] データなしObserverのデータ追加対応：2011.12.15 <ADD> End.
		{ TYPE_NTF_DataID_SCN_9      , (uint32_t)sizeof(TYPE_ADFType)                          , &( m_stSysStatInfo.st_ScannerInfoParam.e_Scn9 )                                    },	// ＡＤＦ装着＆装着タイプ.
		{ TYPE_NTF_DataID_SCN_10     , (uint32_t)sizeof(TYPE_ADFMechanism)                     , &( m_stSysStatInfo.st_ScannerInfoParam.e_Scn10 )                                   },	// ADF読取機構取得.
		// STEP2：2010.12.10 <ADD> End   T.Takahashi.
		// スキャナ警告追加：2011.2.18 <ADD> Start.
		{ TYPE_NTF_DataID_SCN_11     , (uint32_t)sizeof(ushort)                                , &( m_stSysStatInfo.st_ScannerInfoParam.us_Scn11 )                                  },	// ＦＡＴＡＬコードの取得.
		{ TYPE_NTF_DataID_SCN_12     , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_ScannerInfoParam.b_Scn12 )                                   },	// ＦＡＴＡＬが発生しているか.
		{ TYPE_NTF_DataID_SCN_13     , (uint32_t)sizeof(TYPE_FatalLevel)                       , &( m_stSysStatInfo.st_ScannerInfoParam.e_Scn13 )                                   },	// ＦＡＴＡＬ Ｌｅｖｅｌ確認処理.
		{ TYPE_NTF_DataID_SCN_14     , (uint32_t)sizeof(ushort)                                , &( m_stSysStatInfo.st_ScannerInfoParam.us_Scn14 )                                  },	// 周辺機器コミュニケーションエラーコードの取得.
		{ TYPE_NTF_DataID_SCN_15     , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_ScannerInfoParam.b_Scn15 )                                   },	// 周辺機器コミュニケーションエラーが発生しているか.
		{ TYPE_NTF_DataID_SCN_16     , (uint32_t)sizeof(ushort)                                , &( m_stSysStatInfo.st_ScannerInfoParam.us_Scn16 )                                  },	// ＡＤＦジャムコードの取得.
		{ TYPE_NTF_DataID_SCN_17     , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_ScannerInfoParam.b_Scn17 )                                   },	// ＡＤＦジャムが発生しているか.
		{ TYPE_NTF_DataID_SCN_18     , (uint32_t)sizeof(Bool) * TYPD_SSTS_AdfJamPaperItemNum   , &( m_stSysStatInfo.st_ScannerInfoParam.b_Scn18 )                                   },	// ジャム残留用紙が存在するか.
		{ TYPE_NTF_DataID_SCN_19     , (uint32_t)sizeof(Bool) * TYPD_SSTS_AdfDoorOpenItemNum   , &( m_stSysStatInfo.st_ScannerInfoParam.b_Scn19 )                                   },	// ドアが開いているか（個々のドア）.
		{ TYPE_NTF_DataID_SCN_20     , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_ScannerInfoParam.b_Scn20 )                                   },	// 何れかのドアが開いているか.
		{ TYPE_NTF_DataID_SCN_21     , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_ScannerInfoParam.b_Scn21 )                                   },	// プラテンカバーが開いているか.
		
		// ATS/AMS対応：2011.05.10 <ADD> Start T.Takahashi.
		{ TYPE_NTF_DataID_SCN_22     , (uint32_t)sizeof(TYPS_PaperSize)                        , &( m_stSysStatInfo.st_ScannerInfoParam.st_Scn22 )                                  },	// ＡＤＦ・原稿サイズ.
		{ TYPE_NTF_DataID_SCN_23     , (uint32_t)sizeof(TYPS_PaperSize)                        , &( m_stSysStatInfo.st_ScannerInfoParam.st_Scn23 )                                  },	// 手置き・原稿サイズ.
		{ TYPE_NTF_DataID_SCN_24     , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_ScannerInfoParam.b_Scn24 )                                   },	// ＡＤＦ・ガラス上に原稿あり.判定.
		{ TYPE_NTF_DataID_SCN_25     , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_ScannerInfoParam.b_Scn25 )                                   },	// ＡＤＦ・原稿有り 判定.
		{ TYPE_NTF_DataID_SCN_26     , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_ScannerInfoParam.b_Scn26 )                                   },	// ＡＤＦ・原稿サイズ確定 判定.
		{ TYPE_NTF_DataID_SCN_27     , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_ScannerInfoParam.b_Scn27 )                                   },	// 手置き・原稿有り 判定.
		{ TYPE_NTF_DataID_SCN_28     , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_ScannerInfoParam.b_Scn28 )                                   },	// 手置き・原稿サイズ確定 判定.
		// ATS/AMS対応：2011.05.10 <ADD> End T.Takahashi.
		
		// 新API対応：2011.05.26 <ADD> Start T.Takahashi.
		{ TYPE_NTF_DataID_SCN_29     , (uint32_t)sizeof(char)                                  , &( m_stSysStatInfo.st_ScannerInfoParam.sc_Scn29 )                                  },	// デフォルトMail/I-Faxタイトル番号.
		{ TYPE_NTF_DataID_SCN_30     , (uint32_t)sizeof(TYPS_MailSubject) * TYPD_SSTS_MailSubject,&( m_stSysStatInfo.st_ScannerInfoParam.st_Scn30 )                                 },	// Mail/I-Fax用タイトル.
		// 新API対応：2011.05.26 <ADD> End T.Takahashi.

//Str MinervaSBK 機器構成変更_No.13 DFのFCOT短縮対応 H.Ochi 15.02.26.
		{ TYPE_NTF_DataID_SCN_31     , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_ScannerInfoParam.b_Scn31 )                                   },	// 原稿トレイ事前上昇機能有無判定.
//End MinervaSBK 機器構成変更_No.13 DFのFCOT短縮対応 H.Ochi 15.02.26.
//Str【機器構成変更No.11 DFスキュー量パネル表示】M.Koyanagi 2015/04/27.
		{ TYPE_NTF_DataID_SCN_32     , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_ScannerInfoParam.b_Scn32 )                                   },	// DF調整値バックアップ機能有無判定.
//End【機器構成変更No.11 DFスキュー量パネル表示】M.Koyanagi 2015/04/27.
//Str【CQ517055対応 KRQ-49 不定形用紙サイズ自動検出機能の搭載】M.Koyanagi 2015/11/12.
		{ TYPE_NTF_DataID_SCN_33     , (uint32_t)sizeof(TYPS_PaperSize)                        , &( m_stSysStatInfo.st_ScannerInfoParam.st_Scn33 )                                  },	// ＡＤＦ・原稿サイズ.
//End【CQ517055対応 KRQ-49 不定形用紙サイズ自動検出機能の搭載】M.Koyanagi 2015/11/12.
//Str【CQ521390:getEDHProductTypeのBlackAPI化対応】M.Koyanagi 2015/11/12.
		{ TYPE_NTF_DataID_SCN_34     , (uint32_t)sizeof(TYPE_EDHProductType)                   , &( m_stSysStatInfo.st_ScannerInfoParam.e_Scn34 )                                   },	// DF製品タイプを取得.
//End【CQ521390:getEDHProductTypeのBlackAPI化対応】M.Koyanagi 2015/11/12.
// 2016/09/02 APF 制御構成変更No51 制御トラブル検出時のHW自己診断自動化 s.
		{ TYPE_NTF_DataID_SCN_35     , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_ScannerInfoParam.b_Scn35 )                                   },	// 自動自己診断　実施判定.
// 2016/09/02 APF 制御構成変更No51 制御トラブル検出時のHW自己診断自動化 e.
//Str【Eagle 機器構成変更No.86:重送検出機能追加対応】M.Koyanagi 2017/09/05.
		{ TYPE_NTF_DataID_SCN_36     , (uint32_t)sizeof(Bool)                                   , &( m_stSysStatInfo.st_ScannerInfoParam.b_Scn36 )                                   }, // 重送検知機能有無判定.
//End【Eagle 機器構成変更No.86:重送検出機能追加対応】M.Koyanagi 2017/09/05.
// 【追加】2017.12.22 鈴木希 [start] Eagle 機器構成変更_No.86 重送検出センサ調整対応. 
		{ TYPE_NTF_DataID_SCN_37     , (uint32_t)sizeof(TYPE_EDHAdjustTypeMultiFeedDetect)      , &( m_stSysStatInfo.st_ScannerInfoParam.e_Scn37 )                                   }, // 重送検知調整の方式を取得. 
// 【追加】2017.12.22 鈴木希 [ end ] Eagle 機器構成変更_No.86 重送検出センサ調整対応. 
//Str【DenebMLK 制御構成変更No.20 DF不要機能削除】M.Koyanagi 2018/05/14.
		{ TYPE_NTF_DataID_SCN_38     , (uint32_t)sizeof(Bool)                                   , &( m_stSysStatInfo.st_ScannerInfoParam.b_Scn35 )                                   }, // ガラス清掃機能有無判定.
//End【DenebMLK 制御構成変更No.20 DF不要機能削除】M.Koyanagi 2018/05/14.
//Str【Sparrow 制御構成変更No.254 【AIO1ラインCIS】CIS変更 CCD/CIS搭載判定】T.Kurihara 2018/06/22.
#ifdef DEF_A4_Cus
		{ TYPE_NTF_DataID_SCN_39     , (uint32_t)sizeof(TYPE_PlatenScannerType)                 , &( m_stSysStatInfo.st_ScannerInfoParam.e_Scn39 )                                   },	// プラテンのスキャナセンサー種類を取得.
#endif	// DEF_A4_Cus
//End【Sparrow 制御構成変更No.254 【AIO1ラインCIS】CIS変更 CCD/CIS搭載判定】T.Kurihara 2018/06/22.
#ifdef DEF_A4_Cus
//Str. Sparrow 機器構成変更_No.285_AR1062416課題追加対応 読取可能最大サイズ取得対応 S.Kagawa 2018/10/31.
		{ TYPE_NTF_DataID_SCN_40,		(uint32_t)sizeof(TYPS_PaperSize)                        , &( m_stSysStatInfo.st_ScannerInfoParam.st_Scn40 )									},	// プラテン読取可能最大サイズ取得.
		{ TYPE_NTF_DataID_SCN_41,		(uint32_t)sizeof(TYPS_PaperSize)                        , &( m_stSysStatInfo.st_ScannerInfoParam.st_Scn41 )									},	// ADF読取可能最大サイズ取得.
//End. Sparrow 機器構成変更_No.285_AR1062416課題追加対応 読取可能最大サイズ取得対応 S.Kagawa 2018/10/31.
#endif	// def DEF_A4_Cus
// 【追加】2022.07.05 鈴木希 [start] IT6_2.2MR3 機器構成変更No.1 DF-713 副走査方向の調整マージン不足対応. 
		{ TYPE_NTF_DataID_SCN_42     , (uint32_t)sizeof(Bool)                                   , &( m_stSysStatInfo.st_ScannerInfoParam.b_Scn42 )                                   }, // ADFフィード位置調整範囲の拡大機能. 
// 【追加】2022.07.05 鈴木希 [ end ] IT6_2.2MR3 機器構成変更No.1 DF-713 副走査方向の調整マージン不足対応. 

		
		// スキャナ警告追加：2011.2.18 <ADD> End.
		{ TYPE_NTF_DataID_COM_1      , (uint32_t)sizeof(uchar)                                 , &( m_stSysStatInfo.st_CommunicateInfoParam.uc_Com1 )                               },	// スケジュール更新通知用更新番号.
		{ TYPE_NTF_DataID_COM_5		 , (uint32_t)sizeof(TYPS_SysStatusRds)					   , &( m_stSysStatInfo.st_CommunicateInfoParam.st_Com5 )								},	// ＲＤＳ・ＡＰＩオブザーバー状態変化情報 [SystemStatus] データなしObserverのデータ追加対応：2011.12.15 <ADD>.
		{ TYPE_NTF_DataID_COM_7      , (uint32_t)sizeof(TYPE_RdsTrgKind)                       , &( m_stSysStatInfo.st_CommunicateInfoParam.e_Com7 )                                },	// 発信トリガ通知種別.
		{ TYPE_NTF_DataID_DEV_PRT_1  , (uint32_t)sizeof(TYPS_SysStatusEnvironmentalHistoryInfo ), &( m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt1 )                         },	// 環境履歴情報パラメータ.
		{ TYPE_NTF_DataID_DEV_PRT_2  , (uint32_t)sizeof(ushort) * TYPD_SSTS_PaperReachTimeItemNum, &( m_stSysStatInfo.st_PrinterDeviceInfoParam.us_DevPrt2 )                        },	// 「用紙到達時間」情報取得処理.
		{ TYPE_NTF_DataID_DEV_PRT_3  , (uint32_t)sizeof(ushort) * TYPD_SSTS_StabilizationCounterItemNum, &( m_stSysStatInfo.st_PrinterDeviceInfoParam.us_DevPrt3 )                  },	// 「安定化系カウンタ」情報取得処理.
		{ TYPE_NTF_DataID_DEV_PRT_4  , (uint32_t)sizeof(TYPS_UnitLife)                         , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt4 )                          },	// ２次転写ローラーユニットライフ情報.
		{ TYPE_NTF_DataID_DEV_PRT_5  , (uint32_t)sizeof(char)                                  , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt5 )                          },	// Ｋ色ＤＣ消耗率情報.
		{ TYPE_NTF_DataID_DEV_PRT_6  , (uint32_t)sizeof(TYPS_UnitLife)                         , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt6 )                          },	// Ｋ色ＤＣライフ情報.
		{ TYPE_NTF_DataID_DEV_PRT_7  , (uint32_t)sizeof(char)                                  , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt7 )                          },	// Ｋ色ＤｅｖＣ消耗率情報.
		{ TYPE_NTF_DataID_DEV_PRT_8  , (uint32_t)sizeof(TYPS_UnitLife)                         , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt8 )                          },	// Ｋ色ＤｅｖＣライフ情報.
		{ TYPE_NTF_DataID_DEV_PRT_9  , (uint32_t)sizeof(char)                                  , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt9 )                          },	// K現像剤消耗率情報.
		{ TYPE_NTF_DataID_DEV_PRT_10 , (uint32_t)sizeof(TYPS_UnitLife)                         , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt10 )                         },	// K現像剤情報.
		{ TYPE_NTF_DataID_DEV_PRT_11 , (uint32_t)sizeof(TYPS_UnitLife)                         , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt11 )                         },	// Ｋ色ＩＵライフ情報.
		{ TYPE_NTF_DataID_DEV_PRT_12 , (uint32_t)sizeof(char)                                  , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt12 )                         },	// Ｋトナー状態.
		{ TYPE_NTF_DataID_DEV_PRT_13 , (uint32_t)sizeof(char)                                  , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt13 )                         },	// Ｋトナー状態【★CSRC用】.
		{ TYPE_NTF_DataID_DEV_PRT_14 , (uint32_t)sizeof(char)                                  , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt14 )                         },	// 中綴じステープラ動作状態.
		{ TYPE_NTF_DataID_DEV_PRT_15 , (uint32_t)sizeof(ushort)                                , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.us_DevPrt15 )                         },	// 周辺機器コミュニケーションエラーコード.
		{ TYPE_NTF_DataID_DEV_PRT_16 , (uint32_t)sizeof(char)                                  , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt16 )                         },	// Ｃトナー残量情報.
		{ TYPE_NTF_DataID_DEV_PRT_17 , (uint32_t)sizeof(char)                                  , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt17 )                         },	// Ｃ色ＤＣ消耗率情報.
		{ TYPE_NTF_DataID_DEV_PRT_18 , (uint32_t)sizeof(TYPS_UnitLife)                         , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt18 )                         },	// Ｃ色ＤＣライフ情報.
		{ TYPE_NTF_DataID_DEV_PRT_19 , (uint32_t)sizeof(char)                                  , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt19 )                         },	// Ｃ色ＤｅｖＣ消耗率情報.
		{ TYPE_NTF_DataID_DEV_PRT_20 , (uint32_t)sizeof(TYPS_UnitLife)                         , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt20 )                         },	// Ｃ色ＤｅｖＣライフ情報.
		{ TYPE_NTF_DataID_DEV_PRT_21 , (uint32_t)sizeof(char)                                  , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt21 )                         },	// Ｃ色ＩＵ消耗率情報.
		{ TYPE_NTF_DataID_DEV_PRT_22 , (uint32_t)sizeof(TYPS_UnitLife)                         , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt22 )                         },	// Ｃ色ＩＵライフ情報.
		{ TYPE_NTF_DataID_DEV_PRT_23 , (uint32_t)sizeof(char)                                  , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt23 )                         },	// Ｃトナー状態.
		{ TYPE_NTF_DataID_DEV_PRT_24 , (uint32_t)sizeof(char)                                  , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt24 )                         },	// Ｃトナー状態【★CSRC用】.
		{ TYPE_NTF_DataID_DEV_PRT_25 , (uint32_t)sizeof(ushort)                                , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.us_DevPrt25 )                         },	// プリンタ警告（ＦＡＴＡＬコード）.
		{ TYPE_NTF_DataID_DEV_PRT_26 , (uint32_t)sizeof(char)                                  , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt26 )                         },	// ＦＡＴＡＬ Ｌｅｖｅｌ確認.
		{ TYPE_NTF_DataID_DEV_PRT_27 , (uint32_t)sizeof(char)                                  , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt27 )                         },	// 定着ユニット消耗率情報.
		{ TYPE_NTF_DataID_DEV_PRT_28 , (uint32_t)sizeof(TYPS_UnitLife)                         , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt28 )                         },	// 定着ユニットライフ情報.
		{ TYPE_NTF_DataID_DEV_PRT_29 , (uint32_t)sizeof(char)                                  , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt29 )                         },	// Ｋトナー残量情報.
		{ TYPE_NTF_DataID_DEV_PRT_30 , (uint32_t)sizeof(char)                                  , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt30 )                         },	// Ｍ色ＤＣ消耗率情報.
		{ TYPE_NTF_DataID_DEV_PRT_31 , (uint32_t)sizeof(TYPS_UnitLife)                         , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt31 )                         },	// Ｍ色ＤＣライフ情報.
		{ TYPE_NTF_DataID_DEV_PRT_32 , (uint32_t)sizeof(char)                                  , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt32 )                         },	// Ｍ色ＤｅｖＣ消耗率情報.
		{ TYPE_NTF_DataID_DEV_PRT_33 , (uint32_t)sizeof(TYPS_UnitLife)                         , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt33 )                         },	// Ｍ色ＤｅｖＣライフ情報.
		{ TYPE_NTF_DataID_DEV_PRT_34 , (uint32_t)sizeof(char)                                  , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt34 )                         },	// Ｍ色ＩＵ消耗率情報.
		{ TYPE_NTF_DataID_DEV_PRT_35 , (uint32_t)sizeof(TYPS_UnitLife)                         , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt35 )                         },	// Ｍ色ＩＵライフ情報.
		{ TYPE_NTF_DataID_DEV_PRT_36 , (uint32_t)sizeof(char)                                  , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt36 )                         },	// Ｍトナー状態.
		{ TYPE_NTF_DataID_DEV_PRT_37 , (uint32_t)sizeof(char)                                  , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt37 )                         },	// Ｍトナー状態【★CSRC用】.
		{ TYPE_NTF_DataID_DEV_PRT_38 , (uint32_t)sizeof(char)                                  , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt38 )                         },	// メディアエラー.
		{ TYPE_NTF_DataID_DEV_PRT_39 , (uint32_t)sizeof(char)                                  , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt39 )                         },	// Ｍトナー残量情報.
		{ TYPE_NTF_DataID_DEV_PRT_40 , (uint32_t)sizeof(char)                                  , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt40 )                         },	// オゾンフィルタ消耗率情報.
		{ TYPE_NTF_DataID_DEV_PRT_41 , (uint32_t)sizeof(TYPS_UnitLife)                         , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt41 )                         },	// 紙粉除去クリーナー／オゾンフィルタ情報.
		{ TYPE_NTF_DataID_DEV_PRT_42 , (uint32_t)sizeof(char)                                  , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt42 )                         },	// 紙紛除去クリーナ消耗率情報.
		{ TYPE_NTF_DataID_DEV_PRT_43 , (uint32_t)sizeof(ushort)                                , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.us_DevPrt43 )                         },	// ジャムコード.
		{ TYPE_NTF_DataID_DEV_PRT_44 , (uint32_t)sizeof(char)                                  , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt44 )                         },	// サイズエラー.
		{ TYPE_NTF_DataID_DEV_PRT_45 , (uint32_t)sizeof(TYPS_PaperSize) * TYPD_SSTS_FeedTrayMax, &( m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt45 )                         },	// サイズ／メディアエラー用紙サイズ情報.
		{ TYPE_NTF_DataID_DEV_PRT_46 , (uint32_t)sizeof(TYPS_PaperType) * TYPD_SSTS_FeedTrayMax, &( m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt46 )                         },	// サイズ／メディアエラー用紙種類情報.
		{ TYPE_NTF_DataID_DEV_PRT_47 , (uint32_t)sizeof(TYPS_UnitLife)                         , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt47 )                         },	// 噴煙フィルタ情報.
		{ TYPE_NTF_DataID_DEV_PRT_48 , (uint32_t)sizeof(char)                                  , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt48 )                         },	// 平綴じステープラ動作状態.
		{ TYPE_NTF_DataID_DEV_PRT_49 , (uint32_t)sizeof(char)                                  , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt49 )                         },	// トナー回収ボトル状態.
		{ TYPE_NTF_DataID_DEV_PRT_50 , (uint32_t)sizeof(char)                                  , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt50 )                         },	// トナー回収ボトル状態【★CSRC用】.
		{ TYPE_NTF_DataID_DEV_PRT_51 , (uint32_t)sizeof(char)                                  , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt51 )                         },	// トナーフィルタ消耗率情報.
		{ TYPE_NTF_DataID_DEV_PRT_52 , (uint32_t)sizeof(TYPS_UnitLife)                         , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt52 )                         },	// 転写クリーナーユニットライフ情報.
		{ TYPE_NTF_DataID_DEV_PRT_53 , (uint32_t)sizeof(char)                                  , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt53 )                         },	// 転写ベルトユニット消耗率情報.
		{ TYPE_NTF_DataID_DEV_PRT_54 , (uint32_t)sizeof(TYPS_UnitLife)                         , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt54 )                         },	// 転写ベルトユニットライフ情報.
		{ TYPE_NTF_DataID_DEV_PRT_55 , (uint32_t)sizeof(char)                                  , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt55 )                         },	// 転写ローラ消耗率情報.
		{ TYPE_NTF_DataID_DEV_PRT_56 , (uint32_t)sizeof(char)                                  , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt56 )                         },	// Ｙ色ＤＣ消耗率情報.
		{ TYPE_NTF_DataID_DEV_PRT_57 , (uint32_t)sizeof(TYPS_UnitLife)                         , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt57 )                         },	// Ｙ色ＤＣライフ情報.
		{ TYPE_NTF_DataID_DEV_PRT_58 , (uint32_t)sizeof(char)                                  , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt58 )                         },	// Ｙ色ＤｅｖＣ消耗率情報.
		{ TYPE_NTF_DataID_DEV_PRT_59 , (uint32_t)sizeof(TYPS_UnitLife)                         , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt59 )                         },	// Ｙ色ＤｅｖＣライフ情報.
		{ TYPE_NTF_DataID_DEV_PRT_60 , (uint32_t)sizeof(char)                                  , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt60 )                         },	// Ｙ色ＩＵ消耗率情報.
		{ TYPE_NTF_DataID_DEV_PRT_61 , (uint32_t)sizeof(TYPS_UnitLife)                         , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt61 )                         },	// Ｙ色ＩＵライフ情報.
		{ TYPE_NTF_DataID_DEV_PRT_62 , (uint32_t)sizeof(char)                                  , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt62 )                         },	// Ｙトナー状態.
		{ TYPE_NTF_DataID_DEV_PRT_63 , (uint32_t)sizeof(char)                                  , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt63 )                         },	// Ｙトナー状態【★CSRC用】 .
		{ TYPE_NTF_DataID_DEV_PRT_64 , (uint32_t)sizeof(char)                                  , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt64 )                         },	// Ｙトナー残量情報.
		{ TYPE_NTF_DataID_DEV_PRT_66 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt66 )                          },	// クリーニング中.
		{ TYPE_NTF_DataID_DEV_PRT_67 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt67 )                          },	// エンジン環境通知情報_フルカラー禁止状態送信処理.
		{ TYPE_NTF_DataID_DEV_PRT_68 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt68 )                          },	// ドラムドライ中.
		{ TYPE_NTF_DataID_DEV_PRT_69 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt69 )                          },	// 強制トナー補給中.
		{ TYPE_NTF_DataID_DEV_PRT_70 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt70 )                          },	// 画像安定化中.
		{ TYPE_NTF_DataID_DEV_PRT_71 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt71 )                          },	// 初期化中.
		{ TYPE_NTF_DataID_DEV_PRT_72 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt72 )                          },	// PIマニュアル動作中.
		{ TYPE_NTF_DataID_DEV_PRT_73 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt73 )                          },	// 装置構成確定.
		{ TYPE_NTF_DataID_DEV_PRT_74 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt74 )                          },	// ウォームアップ中.
		{ TYPE_NTF_DataID_DEV_PRT_75 , (uint32_t)sizeof(TYPS_SysStatusIULifeStatus)            , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt75 )                         },	// ＩＵライフ状態確認処理パラメータ.
		{ TYPE_NTF_DataID_DEV_PRT_76 , (uint32_t)sizeof(TYPS_SysStatusScanStart)               , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt76 )                         },	// ＣＭＹトナーエンプティ状態パラメータ.
		{ TYPE_NTF_DataID_DEV_PRT_77 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt77 )                          },	// ステープルエンプティ2.
		{ TYPE_NTF_DataID_DEV_PRT_78 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt78 )                          },	// ステープルエンプティ3.
		{ TYPE_NTF_DataID_DEV_PRT_79 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt79 )                         },	// Ｋトナー誤装着.
		{ TYPE_NTF_DataID_DEV_PRT_80 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt80 )                          },	// Ｋトナー未装着.
		{ TYPE_NTF_DataID_DEV_PRT_81 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt81 )                          },	// Ｃトナー誤装着.
		{ TYPE_NTF_DataID_DEV_PRT_82 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt82 )                          },	// Ｃトナー未装着.
		{ TYPE_NTF_DataID_DEV_PRT_83 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt83 )                          },	// エレベートトレイ上昇未完.
		{ TYPE_NTF_DataID_DEV_PRT_84 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt84 )                          },	// 長尺紙可能モード判定処理.
		{ TYPE_NTF_DataID_DEV_PRT_85 , (uint32_t)sizeof(Bool) * TYPD_SSTS_PaperTypeMax         , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt85 )                          },	// 長尺紙可能用紙種類判定処理.
		{ TYPE_NTF_DataID_DEV_PRT_86 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt86 )                          },	// ERRORが発生しているか(ATTENTION/CAUTIONは含まない).
		{ TYPE_NTF_DataID_DEV_PRT_87 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt87 )                          },	// ＨＤＤファン異常が発生しているか.
		{ TYPE_NTF_DataID_DEV_PRT_88 , (uint32_t)sizeof(Bool) * TYPD_SSTS_JamPaperItemNum      , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt88 )                          },	// ジャム残留用紙が存在するか.
		{ TYPE_NTF_DataID_DEV_PRT_89 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt89 )                          },	// LCT装着状態.
		{ TYPE_NTF_DataID_DEV_PRT_90 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt90 )                          },	// Ｍトナー誤装着.
		{ TYPE_NTF_DataID_DEV_PRT_91 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt91 )                          },	// Ｍトナー未装着.
		{ TYPE_NTF_DataID_DEV_PRT_92 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt92 )                          },	// 周辺機器コミュニケーションエラーが発生しているか.
		{ TYPE_NTF_DataID_DEV_PRT_93 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt93 )                          },	// ＦＡＴＡＬが発生しているか.
		{ TYPE_NTF_DataID_DEV_PRT_94 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt94 )                          },	// ジャムが発生しているか.
		{ TYPE_NTF_DataID_DEV_PRT_95 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt95 )                          },	// コピー除去要求.
		{ TYPE_NTF_DataID_DEV_PRT_96 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt96 )                          },	// ＰＩプレ給紙用紙残留検知処理.
		{ TYPE_NTF_DataID_DEV_PRT_97 , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt97 )                          },	// 何れかのドアが開いているか .
		{ TYPE_NTF_DataID_DEV_PRT_98 , (uint32_t)sizeof(Bool) * TYPD_SSTS_DoorOpenItemNum      , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt98 )                          },	// ドアが開いているか（個々のドア）.
		{ TYPE_NTF_DataID_DEV_PRT_99 , (uint32_t)sizeof(Bool) * TYPD_SSTS_PrtWarningItemNum    , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt99 )                          },	// ＰＲＴ関連異常警告が発生しているか(指定Pコードは存在するか) .
		{ TYPE_NTF_DataID_DEV_PRT_100, (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt100 )                         },	// パンチくずフル.
		{ TYPE_NTF_DataID_DEV_PRT_101, (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt101 )                         },	// ステープル屑箱セット.
		{ TYPE_NTF_DataID_DEV_PRT_102, (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt102 )                         },	// ステープルエンプティ1.
		{ TYPE_NTF_DataID_DEV_PRT_103, (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt103 )                         },	// Ｙトナー誤装着.
		{ TYPE_NTF_DataID_DEV_PRT_104, (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt104 )                         },	// Ｙトナー未装着.
		{ TYPE_NTF_DataID_DEV_PRT_105, (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt105 )                         },	// Z折りユニットパンチくずフル.
		{ TYPE_NTF_DataID_DEV_PRT_106, (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt106 )                         },	// プリンタ状態（ＡＰＩ）・CSRC警告情報参照許可/不許可状態.
		
		// [SystemStatus] データなしObserverのデータ追加対応：2011.12.15 <ADD> Start.
		{ TYPE_NTF_DataID_DEV_PRT_109, (uint32_t)sizeof(TYPS_SysStatusPrinterServiceInfo)	   , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt109 )						},	// プリンタサービス情報状態変化.
		{ TYPE_NTF_DataID_DEV_PRT_110, (uint32_t)sizeof(TYPS_SysStatusPrinterServiceSetting)   , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt110 )						},	// プリンタサービス設定状態変化.
		// [SystemStatus] データなしObserverのデータ追加対応：2011.12.15 <ADD> End.
		// STEP2：2010.12.10 <ADD> Start   T.Takahashi.
		{ TYPE_NTF_DataID_DEV_PRT_113, (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt113 )                         },	// 両面ユニット有り.
		{ TYPE_NTF_DataID_DEV_PRT_114, (uint32_t)sizeof(TYPE_LCTType)                          , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.e_DevPrt114 )                         },	// ＬＣＴタイプ.
		{ TYPE_NTF_DataID_DEV_PRT_115, (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt115 )                         },	// 手差しトレイの用紙サイズ自動検出.
		{ TYPE_NTF_DataID_DEV_PRT_116, (uint32_t)sizeof(TYPE_EngineType)                       , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.e_DevPrt116 )                         },	// Ｅｎｇｉｎｅ－Ｔｙｐｅ取得.
		{ TYPE_NTF_DataID_DEV_PRT_117, (uint32_t)sizeof(float)                                 , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.sf_DevPrt117 )                        },	// 長尺用紙サイズの最小値を取得.
		{ TYPE_NTF_DataID_DEV_PRT_118, (uint32_t)sizeof(TYPE_OutputTray)                       , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.e_DevPrt118 )                         },	// リストプリント用の排紙トレイを取得.
		{ TYPE_NTF_DataID_DEV_PRT_119, (uint32_t)sizeof(TYPE_OutputTray) * TYPD_SSTS_OutputTrayMax, &( m_stSysStatInfo.st_PrinterDeviceInfoParam.e_DevPrt119 )                      },	// PCプリント用の排紙トレイを取得.
		{ TYPE_NTF_DataID_DEV_PRT_120, (uint32_t)sizeof(Bool) * TYPD_SSTS_PaperTypeMax         , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt120 )                         },	// 長尺印刷可能な用紙種類か判定する.
		{ TYPE_NTF_DataID_DEV_PRT_121, (uint32_t)sizeof(TYPS_Point_f)                          , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt121 )                        },	// 最大通紙可能用紙サイズ.
		{ TYPE_NTF_DataID_DEV_PRT_122, (uint32_t)sizeof(TYPS_Point_f)                          , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt122 )                        },	// 最小通紙可能用紙サイズ.
		{ TYPE_NTF_DataID_DEV_PRT_123, (uint32_t)sizeof(float)                                 , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.sf_DevPrt123 )                        },	// 後端画像欠損幅（ｍｍ単位）.
		{ TYPE_NTF_DataID_DEV_PRT_124, (uint32_t)sizeof(float)                                 , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.sf_DevPrt124 )                        },	// 左端画像欠損幅（ｍｍ単位）.
		{ TYPE_NTF_DataID_DEV_PRT_125, (uint32_t)sizeof(float)                                 , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.sf_DevPrt125 )                        },	// 右端画像欠損幅（ｍｍ単位）.
		{ TYPE_NTF_DataID_DEV_PRT_126, (uint32_t)sizeof(float)                                 , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.sf_DevPrt126 )                        },	// 印刷領域の最大幅.
		{ TYPE_NTF_DataID_DEV_PRT_127, (uint32_t)sizeof(float)                                 , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.sf_DevPrt127 )                        },	// 印刷領域の最大長.
		{ TYPE_NTF_DataID_DEV_PRT_128, (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt128 )                         },	// サドルユニット有り.
		{ TYPE_NTF_DataID_DEV_PRT_129, (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt129 )                         },	// フィニッシャ有り.
		{ TYPE_NTF_DataID_DEV_PRT_130, (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt130 )                         },	// リレーユニット（ＲＵ）有り.
		{ TYPE_NTF_DataID_DEV_PRT_131, (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt131 )                         },	// Ｚ折りユニット（ＺＵ）有り.
		{ TYPE_NTF_DataID_DEV_PRT_132, (uint32_t)sizeof(TYPE_FinisherType)                     , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.e_DevPrt132 )                         },	// フィニッシャタイプ.
		{ TYPE_NTF_DataID_DEV_PRT_133, (uint32_t)sizeof(TYPS_StapleFunction)                   , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt133 )                        },	// ステープル機能.
		{ TYPE_NTF_DataID_DEV_PRT_134, (uint32_t)sizeof(TYPS_PunchFunction)                    , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt134 )                        },	// パンチ機能.
		{ TYPE_NTF_DataID_DEV_PRT_135, (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt135 )                         },	// PK-514装着時のパンチモード制約有り.
		{ TYPE_NTF_DataID_DEV_PRT_136, (uint32_t)sizeof(TYPS_FoldFunction)                     , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt136 )                        },	// 紙折り機能.
		{ TYPE_NTF_DataID_DEV_PRT_137, (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt137 )                         },	// シフト機能.
		{ TYPE_NTF_DataID_DEV_PRT_138, (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt138 )                         },	// カバーシートフィーダ.
		{ TYPE_NTF_DataID_DEV_PRT_139, (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt139 )                         },	// 第３トレイ装着情報.
		{ TYPE_NTF_DataID_DEV_PRT_140, (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt140 )                         },	// メールビン装着情報.
		{ TYPE_NTF_DataID_DEV_PRT_141, (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt141 )                         },	// Ｚ折り内）パンチユニット（ＰＵ）有り.
		
		// ATS/AMS対応：2011.05.10 <CHG/ADD> Start T.Takahashi.
		{ TYPE_NTF_DataID_DEV_PRT_142, (uint32_t)sizeof(TYPS_PaperSize) * TYPD_SSTS_FeedTrayMax, &( m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt142 )                        },	// 用紙サイズ.
		{ TYPE_NTF_DataID_DEV_PRT_143, (uint32_t)sizeof(TYPS_PaperType) * TYPD_SSTS_FeedTrayMax, &( m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt143 )                        },	// 用紙種類.
		{ TYPE_NTF_DataID_DEV_PRT_144, (uint32_t)sizeof(TYPE_TrayType)  * TYPD_SSTS_FeedTrayMax, &( m_stSysStatInfo.st_PrinterDeviceInfoParam.e_DevPrt144 )                         },	// トレイタイプ.
		{ TYPE_NTF_DataID_DEV_PRT_145, (uint32_t)sizeof(TYPE_PaperStatus) * TYPD_SSTS_FeedTrayMax, &( m_stSysStatInfo.st_PrinterDeviceInfoParam.e_DevPrt145 )                       },	// 用紙状態.
		{ TYPE_NTF_DataID_DEV_PRT_146, (uint32_t)sizeof(TYPE_TrayStatus) * TYPD_SSTS_FeedTrayMax, &( m_stSysStatInfo.st_PrinterDeviceInfoParam.e_DevPrt146 )                        },	// 給紙トレイ状態.
// ADS坪井 → 2011/12/01 【パフォーマンス改善】isWarning対応 Start.
//		{ TYPE_NTF_DataID_DEV_PRT_147, (uint32_t)sizeof(float)                                 , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.sf_DevPrt147 )                        },	// 先端画像欠損幅（ｍｍ単位）.
		{ TYPE_NTF_DataID_DEV_PRT_147, (uint32_t)sizeof(float) * TYPD_SSTS_PaperTypeMax        , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.sf_DevPrt147 )                        },	// 先端画像欠損幅（ｍｍ単位）.
// ADS坪井 → 2011/12/01 【パフォーマンス改善】isWarning対応 Start.
		{ TYPE_NTF_DataID_DEV_PRT_148, (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt148 )                         },	// パンチキット（ＰＫ）有無.
		{ TYPE_NTF_DataID_DEV_PRT_149, (uint32_t)sizeof(ENGE_PunchKind) * TYPD_SSTS_PunchModeMax, &( m_stSysStatInfo.st_PrinterDeviceInfoParam.e_DevPrt149 )                        },	// パンチ種別.
//Str【MZ_3rd】【UV-01273 手差しアンマッチ状態対応（共有メモリ化） : 奥本(ICS) : 2013/04/30.】
		{ TYPE_NTF_DataID_DEV_PRT_150, (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt150 )                         },	// 手差しアンマッチ状態.
//End【MZ_3rd】【UV-01273 手差しアンマッチ状態対応（共有メモリ化） : 奥本(ICS) : 2013/04/30.】
		// ATS/AMS対応：2011.05.10 <CHG/ADD> End T.Takahashi.
		
		// STEP2：2010.12.10 <ADD> End     T.Takahashi.
//Str HeliosL 機器構成変更No.1 定着手動離間対応 : 谷 : 2014/12/09.
		{ TYPE_NTF_DataID_DEV_PRT_151, (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt151 )                         },	// 定着離間状態.
//End HeliosL 機器構成変更No.1 定着手動離間対応 : 谷 : 2014/12/09.
//Str MinervaSBK KRQ-140 手差しトレイ開閉検知センサの搭載 : 谷 : 2015/06/08.
		{ TYPE_NTF_DataID_DEV_PRT_152, (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt152 )                         },	// 手差しトレイセット不良状態取得.
//End MinervaSBK KRQ-140 手差しトレイ開閉検知センサの搭載 : 谷 : 2015/06/08.
//Str【MinervaSBK : CQ520910(定着ウェイト対応) : D.Ujiie : 2015.11.04】.
		{ TYPE_NTF_DataID_DEV_PRT_153, (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt153 )                         },	// 定着ウェイト中.
//End【MinervaSBK : CQ520910(定着ウェイト対応) : D.Ujiie : 2015.11.04】.
		
//Str【ZeusSZX0 RU上排紙対応 : K.Natsume : 2015.07.06】.
		{ TYPE_NTF_DataID_DEV_PRT_156, (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt156 )                         },	// 第４（ＲＵ）トレイ装着情報.
//End【ZeusSZX0 RU上排紙対応 : K.Natsume : 2015.07.06】.

// CQ375788対応 12/12/12 ->.
//		{ TYPE_NTF_DataID_DEV_1      , (uint32_t)sizeof(TYPS_CallerID) * TYPE_CommLine2        , &( m_stSysStatInfo.st_DeviceInfoParam.st_Dev1 )                                    },	// CallerID通知情報.
//		{ TYPE_NTF_DataID_DEV_2      , (uint32_t)sizeof(TYPS_CallerName) * TYPE_CommLine2      , &( m_stSysStatInfo.st_DeviceInfoParam.st_Dev2 )                                    },	// CallerName(発信者名情報)通知用情報.
		{ TYPE_NTF_DataID_DEV_1      , (uint32_t)sizeof(TYPS_CallerID) * TYPE_CommLine4        , &( m_stSysStatInfo.st_DeviceInfoParam.st_Dev1 )                                    },	// CallerID通知情報.
		{ TYPE_NTF_DataID_DEV_2      , (uint32_t)sizeof(TYPS_CallerName) * TYPE_CommLine4      , &( m_stSysStatInfo.st_DeviceInfoParam.st_Dev2 )                                    },	// CallerName(発信者名情報)通知用情報.
// CQ375788対応 12/12/12 <-.
		{ TYPE_NTF_DataID_DEV_3      , (uint32_t)sizeof(TYPS_SysStatusIGCdeviceList)           , &( m_stSysStatInfo.st_DeviceInfoParam.st_Dev3 )                                    },	// IGCdeviceList処理完了通知用パラメータ.
		{ TYPE_NTF_DataID_DEV_4      , (uint32_t)sizeof(TYPE_ChangePowerSave)                  , &( m_stSysStatInfo.st_DeviceInfoParam.e_Dev4 )                                     },	// 節電モード移行開始前通知用種別.
		{ TYPE_NTF_DataID_DEV_6      , (uint32_t)sizeof(TYPE_PmCnt)                            , &( m_stSysStatInfo.st_DeviceInfoParam.e_Dev6 )                                     },	// ＰＭカウンタクリア通知種別.
		{ TYPE_NTF_DataID_DEV_7      , (uint32_t)sizeof(uint32_t)                              , &( m_stSysStatInfo.st_DeviceInfoParam.ul_Dev7 )                                    },	// ＵＳＢデバイス状態変化通知状態.
		{ TYPE_NTF_DataID_DEV_8      , (uint32_t)sizeof(TYPS_SysStatusUsbMemDeviceAppAccess) * TYPE_UsbMemAppIdAll, &( m_stSysStatInfo.st_DeviceInfoParam.st_Dev8 )                 },	// アクセス状態変化通知用パラメータ.
		{ TYPE_NTF_DataID_DEV_9      , (uint32_t)sizeof(uint32_t)                              , &( m_stSysStatInfo.st_DeviceInfoParam.ul_Dev9 )                                    },	// アクセス状態変化通知（アプリ単位）状態.
		{ TYPE_NTF_DataID_DEV_10     , (uint32_t)sizeof(TYPE_CountDown)                        , &( m_stSysStatInfo.st_DeviceInfoParam.c_Dev10 )                                    },	// ウィークリータイマー事前通知(5分前、3分前、1分前)(対パネルタスク専用)カウント.
		{ TYPE_NTF_DataID_DEV_11     , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_DeviceInfoParam.b_Dev11 )                                    },	// ウィークリータイマ一時起動通知(対パネルタスク専用)起動判定フラグ.
// CQ375788対応 12/12/12 ->.
//		{ TYPE_NTF_DataID_DEV_12     , (uint32_t)sizeof(TYPE_FaxErrKind) * TYPE_CommLine2      , &( m_stSysStatInfo.st_DeviceInfoParam.e_Dev12 )                                    },	// Faxエラー通知用エラー種別.
		{ TYPE_NTF_DataID_DEV_12     , (uint32_t)sizeof(TYPE_FaxErrKind) * TYPE_CommLine4      , &( m_stSysStatInfo.st_DeviceInfoParam.e_Dev12 )                                    },	// Faxエラー通知用エラー種別.
// CQ375788対応 12/12/12 <-.
		{ TYPE_NTF_DataID_DEV_14     , (uint32_t)sizeof(TYPE_ExchangeLifeCnt)                  , &( m_stSysStatInfo.st_DeviceInfoParam.e_Dev14 )                                    },	// クリア前発信用保存領域変更通知カウント.
		{ TYPE_NTF_DataID_DEV_15     , (uint32_t)sizeof(uint32_t)                              , &( m_stSysStatInfo.st_DeviceInfoParam.ul_Dev15 )                                   },	// 警告状態変化通知用警告種別.
// CQ375788対応 12/12/12 ->.
//		{ TYPE_NTF_DataID_DEV_16     , (uint32_t)sizeof(FAXE_DetectRs) * TYPE_CommLine2        , &( m_stSysStatInfo.st_DeviceInfoParam.e_Dev16 )                                    },	// 検知レスポンス上位通知用ID.
//		{ TYPE_NTF_DataID_DEV_17     , (uint32_t)sizeof(FAXS_CmdRs) * TYPE_CommLine2           , &( m_stSysStatInfo.st_DeviceInfoParam.st_Dev17 )                                   },	// コマンドレスポンス上位通知（OK／ERROR）用情報.
		{ TYPE_NTF_DataID_DEV_16     , (uint32_t)sizeof(FAXE_DetectRs) * TYPE_CommLine4        , &( m_stSysStatInfo.st_DeviceInfoParam.e_Dev16 )                                    },	// 検知レスポンス上位通知用ID.
		{ TYPE_NTF_DataID_DEV_17     , (uint32_t)sizeof(FAXS_CmdRs) * TYPE_CommLine4           , &( m_stSysStatInfo.st_DeviceInfoParam.st_Dev17 )                                   },	// コマンドレスポンス上位通知（OK／ERROR）用情報.
// CQ375788対応 12/12/12 <-.
		{ TYPE_NTF_DataID_DEV_20     , (uint32_t)sizeof(TYPS_SysStatusAuthDeviceCommon)        , &( m_stSysStatInfo.st_DeviceInfoParam.st_Dev20 )                                   },	// 認証デバイス共通変化通知フラグ.
		{ TYPE_NTF_DataID_DEV_21     , (uint32_t)sizeof(uint32_t)                              , &( m_stSysStatInfo.st_DeviceInfoParam.ul_Dev21 )                                   },	// 認証デバイス状態変化通知状態.
		{ TYPE_NTF_DataID_DEV_38     , (uint32_t)sizeof(TYPE_ExchangeLifeCnt)                  , &( m_stSysStatInfo.st_DeviceInfoParam.e_Dev38 )                                    },	// 発信用保存領域変化通知用カウント.
		{ TYPE_NTF_DataID_DEV_39	 , (uint32_t)sizeof(TYPS_SysStatusFeedTray)				   , &( m_stSysStatInfo.st_DeviceInfoParam.st_Dev39 )									},	// 給紙トレイ情報 [SystemStatus] データなしObserverのデータ追加対応：2011.12.15 <ADD>.
// CQ375788対応 12/12/12 ->.
//		{ TYPE_NTF_DataID_DEV_46     , (uint32_t)sizeof(Bool) * TYPE_CommLine2                 , &( m_stSysStatInfo.st_DeviceInfoParam.b_Dev46 )                                    },	// ＦＡＸ初期化完了通知フラグ.
		{ TYPE_NTF_DataID_DEV_46     , (uint32_t)sizeof(Bool) * TYPE_CommLine4                 , &( m_stSysStatInfo.st_DeviceInfoParam.b_Dev46 )                                    },	// ＦＡＸ初期化完了通知フラグ.
// CQ375788対応 12/12/12 <-.
		// [SystemStatus] データなしObserverのデータ追加対応：2011.12.15 <ADD> Start.
		{ TYPE_NTF_DataID_DEV_53	 , (uint32_t)sizeof(TYPS_SysStatusMemoryInfo)			   , &( m_stSysStatInfo.st_DeviceInfoParam.st_Dev53 )									},	// メモリ情報.
		{ TYPE_NTF_DataID_DEV_56	 , (uint32_t)sizeof(TYPS_SysStatusOriginal)				   , &( m_stSysStatInfo.st_DeviceInfoParam.st_Dev56 )									},	// 原稿情報.
		{ TYPE_NTF_DataID_DEV_57	 , (uint32_t)sizeof(TYPS_SysStatusOutputTray)			   , &( m_stSysStatInfo.st_DeviceInfoParam.st_Dev57 )									},	// 排紙トレイ情報.
		// [SystemStatus] データなしObserverのデータ追加対応：2011.12.15 <ADD> End.
		{ TYPE_NTF_DataID_DEV_62     , (uint32_t)sizeof(TYPS_SysStatusNewPowerStatusChanged)   , &( m_stSysStatInfo.st_DeviceInfoParam.st_Dev62 )                                   },	// パワー状態変化通知(新規)用パラメータ.
// CQ375788対応 12/12/12 ->.
//		{ TYPE_NTF_DataID_DEV_76     , (uint32_t)sizeof(TYPE_TestResult) * TYPE_CommLine2      , &( m_stSysStatInfo.st_DeviceInfoParam.e_Dev76 )                                    },	// テスト結果通知用結果.
		{ TYPE_NTF_DataID_DEV_76     , (uint32_t)sizeof(TYPE_TestResult) * TYPE_CommLine4      , &( m_stSysStatInfo.st_DeviceInfoParam.e_Dev76 )                                    },	// テスト結果通知用結果.
// CQ375788対応 12/12/12 <-.
		{ TYPE_NTF_DataID_DEV_77     , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_DeviceInfoParam.b_Dev77 )                                    },	// パネルバックライト処理フラグ.
// CQ375788対応 12/12/12 ->.
//		{ TYPE_NTF_DataID_DEV_79     , (uint32_t)sizeof(FAXS_CommInfoKPM) * TYPE_CommLine2     , &( m_stSysStatInfo.st_DeviceInfoParam.st_Dev79 )                                   },	// プロトコルモニター信号通知用情報.
//		{ TYPE_NTF_DataID_DEV_80     , (uint32_t)sizeof(FAXE_InfoRs) * TYPE_CommLine2          , &( m_stSysStatInfo.st_DeviceInfoParam.e_Dev80 )                                    },	// 情報レスポンス上位通知用ID.
		{ TYPE_NTF_DataID_DEV_79     , (uint32_t)sizeof(FAXS_CommInfoKPM) * TYPE_CommLine4     , &( m_stSysStatInfo.st_DeviceInfoParam.st_Dev79 )                                   },	// プロトコルモニター信号通知用情報.
		{ TYPE_NTF_DataID_DEV_80     , (uint32_t)sizeof(FAXE_InfoRs) * TYPE_CommLine4          , &( m_stSysStatInfo.st_DeviceInfoParam.e_Dev80 )                                    },	// 情報レスポンス上位通知用ID.
// CQ375788対応 12/12/12 <-.
		
		// STEP2：2010.12.10 <ADD> Start   T.Takahashi.
// CQ375788対応 12/12/12 ->.
//		{ TYPE_NTF_DataID_DEV_82     , (uint32_t)sizeof(FAXS_RomVersion) * TYPE_CommLine2      , &( m_stSysStatInfo.st_DeviceInfoParam.st_Dev82 )                                   },	// Revision No.
//		{ TYPE_NTF_DataID_DEV_83     , (uint32_t)sizeof(FAXS_ModelNo) * TYPE_CommLine2         , &( m_stSysStatInfo.st_DeviceInfoParam.st_Dev83 )                                   },	// モデルNo.
		{ TYPE_NTF_DataID_DEV_82     , (uint32_t)sizeof(FAXS_RomVersion) * TYPE_CommLine4      , &( m_stSysStatInfo.st_DeviceInfoParam.st_Dev82 )                                   },	// Revision No.
		{ TYPE_NTF_DataID_DEV_83     , (uint32_t)sizeof(FAXS_ModelNo) * TYPE_CommLine4         , &( m_stSysStatInfo.st_DeviceInfoParam.st_Dev83 )                                   },	// モデルNo.
// CQ375788対応 12/12/12 <-.
		{ TYPE_NTF_DataID_DEV_84     , (uint32_t)sizeof(TYPE_KeyMode)                          , &( m_stSysStatInfo.st_DeviceInfoParam.e_Dev84 )                                    },	// ベンダータイプ.
		// STEP2：2010.12.10 <ADD> End     T.Takahashi.
		
		// ATS/AMS対応：2011.05.10 <ADD> Start T.Takahashi.
		{ TYPE_NTF_DataID_DEV_85     , (uint32_t)sizeof(TYPE_ApsA6orPostcard)                  , &( m_stSysStatInfo.st_DeviceInfoParam.e_Dev85 )                                    },	// Ａ６・はがきのＡＰＳ選択設定.
		{ TYPE_NTF_DataID_DEV_86     , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_DeviceInfoParam.b_Dev86 )                                    },	// トレイ１・ATS/APS許可.
		{ TYPE_NTF_DataID_DEV_87     , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_DeviceInfoParam.b_Dev87 )                                    },	// トレイ２・ATS/APS許可.
		{ TYPE_NTF_DataID_DEV_88     , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_DeviceInfoParam.b_Dev88 )                                    },	// トレイ３・ATS/APS許可.
		{ TYPE_NTF_DataID_DEV_89     , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_DeviceInfoParam.b_Dev89 )                                    },	// トレイ４・ATS/APS許可.
		{ TYPE_NTF_DataID_DEV_90     , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_DeviceInfoParam.b_Dev90 )                                    },	// トレイ５・ATS/APS許可.
		{ TYPE_NTF_DataID_DEV_91     , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_DeviceInfoParam.b_Dev91 )                                    },	// 手差し・ATS/APS許可.
		{ TYPE_NTF_DataID_DEV_92     , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_DeviceInfoParam.b_Dev92 )                                    },	// 給紙トレイ指定時のATS許可.
		{ TYPE_NTF_DataID_DEV_93     , (uint32_t)sizeof(TYPE_ImageRotateEnable)                , &( m_stSysStatInfo.st_DeviceInfoParam.e_Dev93 )                                    },	// 画像回転許可モード.
		{ TYPE_NTF_DataID_DEV_94     , (uint32_t)sizeof(TYPE_SmallSizeOriginal)                , &( m_stSysStatInfo.st_DeviceInfoParam.e_Dev94 )                                    },	// 小サイズ原稿.
		{ TYPE_NTF_DataID_DEV_95     , (uint32_t)sizeof(TYPS_TraySearchPriority)               , &( m_stSysStatInfo.st_DeviceInfoParam.st_Dev95 )                                   },	// 給紙トレイの優先順位指定.
		{ TYPE_NTF_DataID_DEV_96     , (uint32_t)sizeof(TYPS_PaperTypeOnAPS)                   , &( m_stSysStatInfo.st_DeviceInfoParam.st_Dev96 )                                   },	// ユーザ指定用紙ＡＰＳの紙種.
		{ TYPE_NTF_DataID_DEV_97     , (uint32_t)sizeof(uchar)                                 , &( m_stSysStatInfo.st_DeviceInfoParam.uc_Dev97 )                                   },	// 記録時の最小縮小率（87%～100%).
		{ TYPE_NTF_DataID_DEV_98     , (uint32_t)sizeof(TYPE_FaxPrintSet2Tray)                 , &( m_stSysStatInfo.st_DeviceInfoParam.e_Dev98 )                                    },	// 給紙ﾄﾚｲ固定.
		{ TYPE_NTF_DataID_DEV_99     , (uint32_t)sizeof(TYPE_8KType)                           , &( m_stSysStatInfo.st_DeviceInfoParam.e_Dev99 )                                    },	// ８Ｋタイプ.
		{ TYPE_NTF_DataID_DEV_100    , (uint32_t)sizeof(TYPE_16KType)                          , &( m_stSysStatInfo.st_DeviceInfoParam.e_Dev100 )                                   },	// １６Ｋタイプ.
		{ TYPE_NTF_DataID_DEV_101    , (uint32_t)sizeof(TYPE_FLSType)                          , &( m_stSysStatInfo.st_DeviceInfoParam.e_Dev101 )                                   },	// ＦＬＳタイプ.
		{ TYPE_NTF_DataID_DEV_102    , (uint32_t)sizeof(TYPE_PunchHoleType)                    , &( m_stSysStatInfo.st_DeviceInfoParam.e_Dev102 )                                   },	// パンチ穴数設定.
		{ TYPE_NTF_DataID_DEV_103    , (uint32_t)sizeof(TYPE_SystemType)                       , &( m_stSysStatInfo.st_DeviceInfoParam.e_Dev103 )                                   },	// マシン仕向.
		{ TYPE_NTF_DataID_DEV_104    , (uint32_t)sizeof(Bool) * TYPD_SSTS_DipSwMax * TYPD_SSTS_DipSwBitCount, &( m_stSysStatInfo.st_DeviceInfoParam.b_Dev104 )                      },	// SoftDipSw設定.
		// ATS/AMS対応：2011.05.10 <ADD> End T.Takahashi.
		
		// 新API対応：2011.05.26 <ADD> Start T.Takahashi.
		{ TYPE_NTF_DataID_DEV_105    , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_DeviceInfoParam.b_Dev105 )                                   },	// iFAX機能状態.
		// 新API対応：2011.05.26 <ADD> End T.Takahashi.
		// Job実行可否対応 2011.11.22 ADS K.Tsuboi Start.
		{ TYPE_NTF_DataID_DEV_106    , (uint32_t)sizeof(TYPE_KeyMode)                          , &( m_stSysStatInfo.st_DeviceInfoParam.e_Dev106 )                                   },	// キーモード.
		// Job実行可否対応 2011.11.22 ADS K.Tsuboi End.
// 2018/04/18 APF AR.1044038 Start.
		{ TYPE_NTF_DataID_DEV_107    , (uint32_t)sizeof(FAXE_ResourceWarning)                  , &( m_stSysStatInfo.st_DeviceInfoParam.e_Dev107 )                                   },	// リソースの警告種別.
// 2018/04/18 APF AR.1044038 End.
		
		{ TYPE_NTF_DataID_NET_3      , (uint32_t)sizeof(TYPE_SnmpTrapEvent)                    , &( m_stSysStatInfo.st_NetworkInfoParam.e_Net3 )                                    },	// SNMP TRAP通知設定（TYPE_SnmpTrapEventでSNMP TRAPイベント種類を指定）.
		{ TYPE_NTF_DataID_NET_6      , (uint32_t)sizeof(uchar)                                 , &( m_stSysStatInfo.st_NetworkInfoParam.uc_Net6 )                                   },	// スケジュール更新通知用ID.
		// [SystemStatus] データなしObserverのデータ追加対応：2011.12.15 <ADD> Start.
		{ TYPE_NTF_DataID_NET_14     , (uint32_t)sizeof(TYPS_SysStatusInternetISW)			   , &( m_stSysStatInfo.st_NetworkInfoParam.st_Net14 )									},	// インターネットＩＳＷ状態変化情報.
		{ TYPE_NTF_DataID_NET_25     , (uint32_t)sizeof(TYPS_SysStatusNetworkDevice)		   , &( m_stSysStatInfo.st_NetworkInfoParam.st_Net25 )									},	// ネットワークデバイス状態変化情報.
		// [SystemStatus] データなしObserverのデータ追加対応：2011.12.15 <ADD> End.
		{ TYPE_NTF_DataID_NET_32     , (uint32_t)sizeof(short)                                 , &( m_stSysStatInfo.st_NetworkInfoParam.ss_Net32 )                                  },	// S/MIME証明書DB状態変化通知用番号.
		{ TYPE_NTF_DataID_NET_39     , (uint32_t)sizeof(TYPE_SsdpParam)                        , &( m_stSysStatInfo.st_NetworkInfoParam.e_Net39 )                                   },	// パラメータ更新通知用パラメータ種別.
		
		// 新API対応：2011.05.26 <ADD> Start T.Takahashi.
		{ TYPE_NTF_DataID_NET_48     , (uint32_t)sizeof(ushort)                                , &( m_stSysStatInfo.st_NetworkInfoParam.us_Net48 )                                  },	// ポート番号（SMTP送信設定）.
		// 新API対応：2011.05.26 <ADD> End T.Takahashi.
		
		{ TYPE_NTF_DataID_OTH_1		 , (uint32_t)sizeof(TYPS_SysStatusSearchablePDFSetting)	   , &( m_stSysStatInfo.st_OtherInfoParam.st_Oth1 )										},	// サーチャブルPDF生成機能変化情報 [SystemStatus] データなしObserverのデータ追加対応：2011.12.15 <ADD>.
		{ TYPE_NTF_DataID_OTH_2      , (uint32_t)sizeof(TYPE_FaxListReportSettingObserver)     , &( m_stSysStatInfo.st_OtherInfoParam.e_Oth2 )                                      },	// Faxレポート設定データ変化通知用種別.
		{ TYPE_NTF_DataID_OTH_6      , (uint32_t)sizeof(TYPS_SysStatusDummyIFFaultNotify)      , &( m_stSysStatInfo.st_OtherInfoParam.st_Oth6 )                                     },	// フォールト通知用パラメータ.
		{ TYPE_NTF_DataID_OTH_7      , (uint32_t)sizeof(uint)                                  , &( m_stSysStatInfo.st_OtherInfoParam.e_Oth7 )                                      },	// タイムアウト通知用ハンドル.
		// Job実行可否対応 2011.11.22 ADS K.Tsuboi Start.
		{ TYPE_NTF_DataID_OTH_9      , (uint32_t)sizeof(Bool)                                  , &( m_stSysStatInfo.st_OtherInfoParam.b_Oth9 )                                      },	// 像ログ(簡易ログ)フル状態.
		// Job実行可否対応 2011.11.22 ADS K.Tsuboi End.

// 2014.01.09-STA  パネルHW情報の取得.
		{ TYPE_NTF_DataID_PNL_5     , (uint32_t)sizeof(TYPS_PanelHardResolution)               , &( m_stSysStatInfo.st_PanelInfoParam.st_PanelHardResolution )                      },	// パネルHWの情報.
// 2014.01.09-END  パネルHW情報の取得.

		{ TYPE_NTF_DataID_WRN_PRT_164   , (uint32_t)sizeof(Bool)                               , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt164 )        },	// 紙粉除去クリーナニアライフ.
		{ TYPE_NTF_DataID_WRN_PRT_165   , (uint32_t)sizeof(Bool)                               , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt165 )        },	// 紙粉除去クリーナライフ.
// 2014.12.23 ZeusS 機器構成変更No40 s.
		{ TYPE_NTF_DataID_WRN_PRT_166   , (uint32_t)sizeof(Bool)                               , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt166 )        },	// C IUプレニアライフ警告検出（パネル確認用）.
		{ TYPE_NTF_DataID_WRN_PRT_167   , (uint32_t)sizeof(Bool)                               , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt167 )        },	// M IUプレニアライフ警告検出（パネル確認用）.
		{ TYPE_NTF_DataID_WRN_PRT_168   , (uint32_t)sizeof(Bool)                               , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt168 )        },	// Y IUプレニアライフ警告検出（パネル確認用）.
		{ TYPE_NTF_DataID_WRN_PRT_169   , (uint32_t)sizeof(Bool)                               , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt169 )        },	// C DCプレニアライフ警告検出（パネル確認用）.
		{ TYPE_NTF_DataID_WRN_PRT_170   , (uint32_t)sizeof(Bool)                               , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt170 )        },	// M DCプレニアライフ警告検出（パネル確認用）.
		{ TYPE_NTF_DataID_WRN_PRT_171   , (uint32_t)sizeof(Bool)                               , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt171 )        },	// Y DCプレニアライフ警告検出（パネル確認用）.
//AR1062444 EagleL_トナーボトルエラー対応 ST.
		{ TYPE_NTF_DataID_WRN_PRT_172   , (uint32_t)sizeof(Bool)                               , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt172 )        },	// K DCプレニアライフ警告検出（パネル確認用）.
// 2014.12.23 ZeusS 機器構成変更No40 e.
		{ TYPE_NTF_DataID_WRN_PRT_189   , (uint32_t)sizeof(Bool)                               , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt189 )         },	// トナーカートリッジセット不良エラー・シアン.
		{ TYPE_NTF_DataID_WRN_PRT_190   , (uint32_t)sizeof(Bool)                               , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt190 )         },	// トナーカートリッジセット不良エラー・マゼンタ.
		{ TYPE_NTF_DataID_WRN_PRT_191   , (uint32_t)sizeof(Bool)                               , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt191 )         },	// トナーカートリッジセット不良エラー・イエロー.
		{ TYPE_NTF_DataID_WRN_PRT_192   , (uint32_t)sizeof(Bool)                               , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt192 )         }	// トナーカートリッジセット不良エラー・ブラック.
//AR1062444 EagleL_トナーボトルエラー対応 ED.
#ifdef DEF_A3A4COMM
//Str【A3A4共通化FUM  ユニット装着情報１共通化: M.Saitou : 2014.12.22】.
		,
		{ TYPE_NTF_DataID_WRN_PRT_173   , (uint32_t)sizeof(Bool)                               , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt173 )        },	// ＴＣ新品解除不良・シアン.
		{ TYPE_NTF_DataID_WRN_PRT_174   , (uint32_t)sizeof(Bool)                               , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt174 )        },	// ＴＣ新品解除不良・マゼンダ.
		{ TYPE_NTF_DataID_WRN_PRT_175   , (uint32_t)sizeof(Bool)                               , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt175 )        },	// ＴＣ新品解除不良・イエロー.
		{ TYPE_NTF_DataID_WRN_PRT_176   , (uint32_t)sizeof(Bool)                               , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt176 )        },	// ＴＣ新品解除不良・ブラック.
		{ TYPE_NTF_DataID_WRN_PRT_177   , (uint32_t)sizeof(Bool)                               , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt177 )        },	// ＴＣ新品解除不良・シアン（ＡＴＴＥＮＴＩＯＮレベル）.
		{ TYPE_NTF_DataID_WRN_PRT_178   , (uint32_t)sizeof(Bool)                               , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt178 )        },	// ＴＣ新品解除不良・マゼンダ（ＡＴＴＥＮＴＩＯＮレベル）.
		{ TYPE_NTF_DataID_WRN_PRT_179   , (uint32_t)sizeof(Bool)                               , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt179 )        },	// ＴＣ新品解除不良・イエロー（ＡＴＴＥＮＴＩＯＮレベル）.
		{ TYPE_NTF_DataID_WRN_PRT_180   , (uint32_t)sizeof(Bool)                               , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt180 )        },	// ＴＣ新品解除不良・ブラック（ＡＴＴＥＮＴＩＯＮレベル）.

#if defined(DEF_A4_Cus)
		{ TYPE_NTF_DataID_DEV_PRT_154   , (uint32_t)sizeof(Bool)                               , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt154 )                         },	// Ｍトナー新品解除不良.
		{ TYPE_NTF_DataID_DEV_PRT_155   , (uint32_t)sizeof(Bool)                               , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt155 )                         },	// Ｃトナー新品解除不良.
		{ TYPE_NTF_DataID_DEV_PRT_157   , (uint32_t)sizeof(Bool)                               , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt157 )                         },	// Ｋトナー新品解除不良.
		{ TYPE_NTF_DataID_DEV_PRT_158   , (uint32_t)sizeof(Bool)                               , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt158 )                         },	// Ｙトナー新品解除不良.
#else
		{ TYPE_NTF_DataID_DEV_PRT_152   , (uint32_t)sizeof(Bool)                               , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt152 )                         },	// Ｙトナー新品解除不良.
		{ TYPE_NTF_DataID_DEV_PRT_153   , (uint32_t)sizeof(Bool)                               , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt153 )                         },	// Ｍトナー新品解除不良.
		{ TYPE_NTF_DataID_DEV_PRT_154   , (uint32_t)sizeof(Bool)                               , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt154 )                         },	// Ｃトナー新品解除不良.
		{ TYPE_NTF_DataID_DEV_PRT_155   , (uint32_t)sizeof(Bool)                               , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt155 )                         },	// Ｋトナー新品解除不良.
#endif //defined(DEF_A4_Cus)
//End【A3A4共通化FUM  ユニット装着情報１共通化: M.Saitou : 2014.12.22】.
#endif // DEF_A3A4COMM
#ifdef	DEF_A4_Cus		// VenusMLK Ｋ色ＩＵ消耗率情報対応 2015.08.11 >>.
		{ TYPE_NTF_DataID_DEV_PRT_159   , (uint32_t)sizeof(char)                               , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.sc_DevPrt159 )                        },	// Ｋ色ＩＵ消耗率情報.
#endif /* DEF_A4_Cus */	// VenusMLK Ｋ色ＩＵ消耗率情報対応 2015.08.11 <<.
#if defined( DEF_A4_Cus )
		{ TYPE_NTF_DataID_WRN_SCN_19   , (uint32_t)sizeof(Bool)                                , &( m_stSysStatInfo.st_WarningInfoParam.st_ScnWarningInfoParam.b_WrnScn19 )         },	// スキャナー関連:スライダーロック異常.
#endif // defined( DEF_A4_Cus ).
#ifdef DEF_A4_Cus
		{ TYPE_NTF_DataID_WRN_PRT_181   , (uint32_t)sizeof(Bool)                               , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt181 )        },	// ＩＵ新品解除不良・シアン.
		{ TYPE_NTF_DataID_WRN_PRT_182   , (uint32_t)sizeof(Bool)                               , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt182 )        },	// ＩＵ新品解除不良・マゼンダ.
		{ TYPE_NTF_DataID_WRN_PRT_183   , (uint32_t)sizeof(Bool)                               , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt183 )        },	// ＩＵ新品解除不良・イエロー.
		{ TYPE_NTF_DataID_WRN_PRT_184   , (uint32_t)sizeof(Bool)                               , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt184 )        },	// ＩＵ新品解除不良・ブラック.
		{ TYPE_NTF_DataID_WRN_PRT_185   , (uint32_t)sizeof(Bool)                               , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt185 )        },	// ＩＵ新品解除不良・シアン（ＡＴＴＥＮＴＩＯＮレベル）.
		{ TYPE_NTF_DataID_WRN_PRT_186   , (uint32_t)sizeof(Bool)                               , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt186 )        },	// ＩＵ新品解除不良・マゼンダ（ＡＴＴＥＮＴＩＯＮレベル）.
		{ TYPE_NTF_DataID_WRN_PRT_187   , (uint32_t)sizeof(Bool)                               , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt187 )        },	// ＩＵ新品解除不良・イエロー（ＡＴＴＥＮＴＩＯＮレベル）.
		{ TYPE_NTF_DataID_WRN_PRT_188   , (uint32_t)sizeof(Bool)                               , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt188 )        },	// ＩＵ新品解除不良・ブラック（ＡＴＴＥＮＴＩＯＮレベル）.
		{ TYPE_NTF_DataID_DEV_PRT_160   , (uint32_t)sizeof(Bool)                               , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt160 )                         },	// ＹＩＵ新品解除不良.
		{ TYPE_NTF_DataID_DEV_PRT_161   , (uint32_t)sizeof(Bool)                               , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt161 )                         },	// ＭＩＵ新品解除不良.
		{ TYPE_NTF_DataID_DEV_PRT_162   , (uint32_t)sizeof(Bool)                               , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt162 )                         },	// ＣＩＵ新品解除不良.
		{ TYPE_NTF_DataID_DEV_PRT_163   , (uint32_t)sizeof(Bool)                               , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt163 )                         },	// ＫＩＵ新品解除不良.
#endif	/* DEF_A4_Cus */
//Str 【KronosS WBS_16_26 KronosS警告追加対応 : S.Itoh 2016.10.31】.
#if defined(DEF_A4_Cus) && defined(DEF_KRONOSS_Cus)
//Str 【KronosS WBS_16 LXK トラブルコード追加 S.Itoh 2016.10.13】.
		{ TYPE_NTF_DataID_DEV_PRT_165	, (uint32_t)sizeof(ushort)                             , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.us_DevPrt165 )						   },	// JAMLxkエンジンYYYコード.
		{ TYPE_NTF_DataID_DEV_PRT_166	, (uint32_t)sizeof(ushort)                             , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.us_DevPrt166 )						   },	// JAMLxkエンジンXXコード.
		{ TYPE_NTF_DataID_DEV_PRT_167	, (uint32_t)sizeof(ushort)                             , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.us_DevPrt167 )						   },	// FatalLxkエンジンYYYコード.
		{ TYPE_NTF_DataID_DEV_PRT_168	, (uint32_t)sizeof(ushort)                             , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.us_DevPrt168 )						   },	// FatalLxkエンジンXXコード.
//End 【KronosS WBS_16 LXK トラブルコード追加 S.Itoh 2016.10.13】.
		{ TYPE_NTF_DataID_DEV_PRT_169	, (uint32_t)sizeof(ushort)                             , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.us_DevPrt169 )						   },	// TCエラーコード(TYPE_PrinterWarning).
		{ TYPE_NTF_DataID_DEV_PRT_170	, (uint32_t)sizeof(ushort)                             , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.us_DevPrt170 )						   },	// TCエラーサブコードYYY.
		{ TYPE_NTF_DataID_DEV_PRT_171	, (uint32_t)sizeof(ushort)                             , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.us_DevPrt171 )						   },	// TCエラーサブコードXX.
		{ TYPE_NTF_DataID_DEV_PRT_172	, (uint32_t)sizeof(uchar)                              , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.uc_DevPrt172 )						   },	// TCエラーAlphaCharacter.
		{ TYPE_NTF_DataID_DEV_PRT_173	, (uint32_t)sizeof(ushort)                             , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.us_DevPrt173 )						   },	// IUエラーコード(TYPE_PrinterWarning).
		{ TYPE_NTF_DataID_DEV_PRT_174	, (uint32_t)sizeof(ushort)                             , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.us_DevPrt174 )						   },	// IUエラーサブコードYYY.
		{ TYPE_NTF_DataID_DEV_PRT_175	, (uint32_t)sizeof(ushort)                             , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.us_DevPrt175 )						   },	// IUエラーサブコードXX.
		{ TYPE_NTF_DataID_DEV_PRT_176	, (uint32_t)sizeof(uchar)                              , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.uc_DevPrt176 )						   },	// IUエラーAlphaCharacter.
		{ TYPE_NTF_DataID_WRN_PRT_193   , (uint32_t)sizeof(Bool)                               , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt193 )         },	// メンテナンスキットライフ.
		{ TYPE_NTF_DataID_WRN_PRT_194   , (uint32_t)sizeof(Bool)                               , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt194 )         },	// メンテナンスキットニアライフ.
//Str 【KronosS AR1012883_メンテナンスキットライフパネル表示用 : S.Itoh 2017.02.20】.
		{ TYPE_NTF_DataID_WRN_PRT_195   , (uint32_t)sizeof(Bool)                               , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt195 )         },	// メンテナンスキットライフ（パネル確認用）.
		{ TYPE_NTF_DataID_WRN_PRT_196   , (uint32_t)sizeof(Bool)                               , &( m_stSysStatInfo.st_WarningInfoParam.st_PrtWarningInfoParam.b_WrnPrt196 )         },	// メンテナンスキットニアライフ（パネル確認用）.
//End 【KronosS AR1012883_メンテナンスキットライフパネル表示用 : S.Itoh 2017.02.20】.
//Str 【KronosS AR1016161_JAM/FATAL_AlphaCharacter追加 : S.Itoh 2017.03.03】.
		{ TYPE_NTF_DataID_DEV_PRT_177	, (uint32_t)sizeof(uchar)                              , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.uc_DevPrt177 )						   },	// JAMLxkエンジンAlphaCharacter.
		{ TYPE_NTF_DataID_DEV_PRT_178	, (uint32_t)sizeof(uchar)                              , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.uc_DevPrt178 )						   },	// FatalLxkエンジンAlphaCharacter.
//End 【KronosS AR1016161_JAM/FATAL_AlphaCharacter追加 : S.Itoh 2017.03.03】.
// 2017/09/08 APF 制御構成変更No34 トレイオープン時の警告表示改善 Start.
		{ TYPE_NTF_DataID_DEV_PRT_179	, (uint32_t)sizeof(TYPE_FeedTrayUnset2Error)           , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.e_DevPrt179 )						   },	// 給紙トレイセット不良２エラー.
// 2017/09/08 APF 制御構成変更No34 トレイオープン時の警告表示改善 End.
#endif    // #if defined(DEF_A4_Cus) && defined(DEF_KRONOSS_Cus)
//End 【KronosS WBS_16_26 KronosS警告追加対応 : S.Itoh 2016.10.31】.
// 2016/09/02 APF 制御構成変更No51 制御トラブル検出時のHW自己診断自動化 s.
#if !defined( DEF_A3A4COMM ) && !defined( DEF_A4_Cus )
		,
#endif
		{ TYPE_NTF_DataID_DEV_PRT_164   , (uint32_t)sizeof(Bool)                               , &( m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt164 )                          },	// 自動自己診断　実施判定.
// 2016/09/02 APF 制御構成変更No51 制御トラブル検出時のHW自己診断自動化 e.

// 2022/9/22 RQ-912対応 start.
		{ TYPE_NTF_DataID_DEV_PRT_180, (uint32_t)sizeof(Bool) * TYPD_SSTS_FeedTrayMax			, &( m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt180 )                       },	// メディア自動検知.
		{ TYPE_NTF_DataID_DEV_PRT_181, (uint32_t)sizeof(Bool) * TYPD_SSTS_FeedTrayMax			, &( m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt181 )                       }	// メディア未確定確認.
// 2022/9/22 RQ-912対応 end.
};
	
	m_pcInstance->lock(m_semID);												// ローカルセマフォ ロック.
	
	// ***** データの書き込み *****.
	do {
		int  fd;
		
		//===== ファイルオープン =====.
		strncpy( psc_FilePath, SSTD_OUTPUT_FILEPATH, sizeof(SSTD_OUTPUT_FILEPATH) );
#ifdef LINUX
		fd = mkstemp( psc_FilePath );
		if( FILF_Close( fd ) == ERROR ){
			if ( sc_Result == TYPD_SSTS_StatusSuccess ){ sc_Result = TYPD_SSTS_StatusFcloseError; }
			break;
		}
#endif
		fd = FILF_Open( const_cast<char *>(psc_FilePath), O_BINARY | O_WRONLY | O_CREAT, 0666 );
		if( fd == ERROR ){
			sc_Result = TYPD_SSTS_StatusFopenError;							// オープンエラー.
			break;
		}
		
		//===== １レコード×データ分書込み =====.
		size_t ul_InfoSize = offsetof(struct st_WriteDataInfo, pv_Data);
		for( ushort us_WriteDataIdx = 0; us_WriteDataIdx < sizeof( st_WriteDataInfoTable ) / sizeof( struct st_WriteDataInfo ); us_WriteDataIdx++ ){
			
			//----- １レコード情報書き込み -----.
			size_t ul_Temp = (size_t)FILF_Write( fd, ( char * )&st_WriteDataInfoTable[ us_WriteDataIdx ],  ul_InfoSize);
			if( ul_Temp != ul_InfoSize ){
				sc_Result = TYPD_SSTS_StatusFwriteError;					// ライトエラー.
				break;
			}
			
			//----- １レコード：データ書込み -----.
			ul_Temp = FILF_Write( fd, ( char * )( st_WriteDataInfoTable[ us_WriteDataIdx ].pv_Data ), st_WriteDataInfoTable[ us_WriteDataIdx ].ul_DataSize );
			if( ul_Temp != st_WriteDataInfoTable[ us_WriteDataIdx ].ul_DataSize ){
				sc_Result = TYPD_SSTS_StatusFwriteError;					// ライトエラー.
				break;
			}
		}
		//===== ファイルクローズ =====.
		if( FILF_Close( fd ) == ERROR ){
			if ( sc_Result == TYPD_SSTS_StatusSuccess ){ sc_Result = TYPD_SSTS_StatusFcloseError; }
			break;
		}
	} while( 0 );
	
	m_pcInstance->unlock(m_semID); 											// ローカルセマフォ アンロック.
	
	SST_LOG2(MFPE_LogLevelTrace, "End. sc_Result =(%d).\n", (int)sc_Result);
	
	return sc_Result;
}

// STEP2：2010.12.10 <ADD> Start T.Takahashi.
//************************************************************************************************************************.
//概要　　：システムステータス更新要求処理.
//引数　　：<in>　e_DataID		データID.
//　　　　：<in>　puc_Data		更新データ.
//戻り値　：なし.
//備考　　：エンディアンに注意すること.
//作成者　：高橋.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//************************************************************************************************************************.
void SSTC_SystemStatusService::setSystemStatusData( TYPE_NTF_DataID e_DataID, uchar *puc_Data )
{
	SST_LOG2(MFPE_LogLevelTrace, "e_DataID=(0x%04x).\n", e_DataID);
	
	APIC_SystemInfo* pc_SystemInfo = APIC_SystemInfo::newInstance();
	
	// APICのメソッドコール.
	switch( e_DataID ){
		case TYPE_NTF_DataID_SYS_14:	// 製品名情報.
			pc_SystemInfo->setSysDescr( (char *)puc_Data ); break;
		case TYPE_NTF_DataID_SYS_15:	// RomVersion情報.
			pc_SystemInfo->setRomVersion( (TYPE_Rom)*puc_Data, (char*)(puc_Data + sizeof(TYPE_Rom)) ); break;
		case TYPE_NTF_DataID_SYS_16:	// Romチェックサム.
			pc_SystemInfo->setRomCheckSum( (TYPE_Rom)*puc_Data, (ushort)*(puc_Data + sizeof(TYPE_Rom)) ); break;
		case TYPE_NTF_DataID_SYS_17:	// ＭＦＰコントローラ内のサブＲＯＭバージョン.
			pc_SystemInfo->setMFPControllerRomVersion( (TYPE_MFPControllerRom)*puc_Data, (const char*)(puc_Data + sizeof(TYPE_MFPControllerRom)) ); break;
		case TYPE_NTF_DataID_SYS_18:	// 製品タイプ情報.
			pc_SystemInfo->setMachineTypeInfo( (Bool)*puc_Data ); break;
		case TYPE_NTF_DataID_SYS_23:	// ＰＣＩ拡張基板有無.
			pc_SystemInfo->setPciExpandBoardExist( (Bool)*puc_Data ); break;
		case TYPE_NTF_DataID_SYS_24:	// ローカルＩ／Ｆ有無.
			pc_SystemInfo->setLocalIfExist( (Bool)*puc_Data ); break;
		case TYPE_NTF_DataID_SYS_27:	// 暗号化基板有無.
			pc_SystemInfo->setEncryptionBoardExist( (Bool)*puc_Data ); break;
		case TYPE_NTF_DataID_SYS_29:	// シリアルナンバー・プリンタ.
			pc_SystemInfo->setSerialNoPrinter( (const char*)puc_Data ); break;
		case TYPE_NTF_DataID_SYS_30:	// シリアルナンバー・スキャナ.
			pc_SystemInfo->setSerialNoScaner( (const char*)puc_Data ); break;
		case TYPE_NTF_DataID_SYS_31:	// シリアルナンバー・ＡＤＦ.
			pc_SystemInfo->setSerialNoAdf( (const char*)puc_Data ); break;
		case TYPE_NTF_DataID_SYS_32:	// シリアルナンバー・ＬＣＣ.
			pc_SystemInfo->setSerialNoLcc( (const char*)puc_Data ); break;
		case TYPE_NTF_DataID_SYS_33:	// シリアルナンバー・排紙ＯＰ.
			pc_SystemInfo->setSerialNoEjectOP( (const char*)puc_Data ); break;
		case TYPE_NTF_DataID_SYS_34:	// シリアルナンバー・Ｚ折りユニット.
			pc_SystemInfo->setSerialNoZU( (const char*)puc_Data ); break;
		case TYPE_NTF_DataID_SYS_35:	// シリアルナンバー・リレーユニット.
			pc_SystemInfo->setSerialNoRU( (const char*)puc_Data ); break;
		case TYPE_NTF_DataID_SYS_36:	// シリアルナンバー・再給紙.
			pc_SystemInfo->setSerialNoDuplex( (const char*)puc_Data ); break;
		case TYPE_NTF_DataID_SYS_37:	// シリアルナンバー・コピーベンダー.
			pc_SystemInfo->setSerialNoVendor( (const char*)puc_Data ); break;
		case TYPE_NTF_DataID_SYS_38:	// シリアルナンバー・ファクス１.
			pc_SystemInfo->setSerialNoFax1( (const char*)puc_Data ); break;
		case TYPE_NTF_DataID_SYS_39:	// シリアルナンバー・ファクス２.
			pc_SystemInfo->setSerialNoFax2( (const char*)puc_Data ); break;
		case TYPE_NTF_DataID_SYS_48:	// 特定原稿検出エラー.
			pc_SystemInfo->setSODError(); break;
		case TYPE_NTF_DataID_SYS_54:	// Pre Rom Version 設定.
			pc_SystemInfo->setPreRomVersion( (const char*)puc_Data ); break;
		case TYPE_NTF_DataID_SYS_55:	// トラブルリセットモード.
			pc_SystemInfo->setTroubleResetMode( (Bool)*puc_Data ); break;
		case TYPE_NTF_DataID_SYS_56:	// FirmWare ダウンロード 設定 .
			pc_SystemInfo->setFirmWareDownLoad( (Bool)*puc_Data ); break;
		case TYPE_NTF_DataID_SYS_58:	// Thames1P モデル.
			pc_SystemInfo->setPrinterModelKind( (TYPE_PrinterModelKind)*puc_Data ); break;
		case TYPE_NTF_DataID_SYS_59:	// Thames1P 強制切替モード設定.
			pc_SystemInfo->setPrinterModelForceChange( (Bool)*puc_Data ); break;
		case TYPE_NTF_DataID_SYS_60:	// Thames1P 強制モデル設定.
			pc_SystemInfo->setPrinterModelForceKind( (TYPE_PrinterModelKind)*puc_Data ); break;
		case TYPE_NTF_DataID_SYS_62:	// Ｆ／Ｗバージョン.
			pc_SystemInfo->setFirmWareVersion( (TYPE_FirmWareVersion)*puc_Data ); break;
		case TYPE_NTF_DataID_SYS_64:	// MFPの機能バージョン.
			pc_SystemInfo->setFuncVer( (uchar)*puc_Data ); break;
		case TYPE_NTF_DataID_SYS_67:	// DSC1基板装着状態.
			pc_SystemInfo->setDSC1Exist( (Bool)*puc_Data ); break;
		case TYPE_NTF_DataID_SYS_68:	// DSC2基板装着状態.
			pc_SystemInfo->setDSC2Exist( (Bool)*puc_Data ); break;
		case TYPE_NTF_DataID_SYS_71:	// Bluetooth機能設定.
			pc_SystemInfo->setBluetoothFunc( (Bool)*puc_Data ); break;
		case TYPE_NTF_DataID_SYS_72:	// アウトラインフォントデータのチェックサム値.
			pc_SystemInfo->setOutLineFontRomCheckSum( (ushort)*puc_Data ); break;
		default:
			SST_LOG2(MFPE_LogLevelError, "e_DataID =(%d) error：Out of DataID.\n", (int)e_DataID);
			break;
	}
}

//************************************************************************************************************************.
//概要　　：変化点通知設定要求処理.
//　　　　：<in>　e_GroupID		GroupID.
//　　　　：<in>　us_DataSize	設定データ長.
//　　　　：<in>　puc_Data		設定データ.
//戻り値　：なし.
//備考　　：特に無し.
//作成者　：高橋.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//************************************************************************************************************************.
void SSTC_SystemStatusService::setNotificationSetting( TYPE_NTF_GroupID e_GroupID, size_t ul_DataSize, uchar *puc_Data )
{
	SST_LOG2(MFPE_LogLevelTrace, "e_GroupID=(0x%02x) ul_DataSize=(%ld) puc_Data=(%p).\n", e_GroupID, ul_DataSize, puc_Data);
	
	// 登録先の取得
	uchar *puc_Top = ( m_pulNotificationMap[ e_GroupID ] == NULL ) ? new uchar [ ul_DataSize ] : (uchar *)m_pulNotificationMap[ e_GroupID ];
	
	do {
		if( puc_Top == NULL ){
// 2015.10.16 ZeusSZX0 制御76-1 新詳細コード s.
//			APIC_ThrowSystemError(SYSE_SysErrGeneral, LOGF_DEBUG_EVENT_KIND_SYS_STATUS, "SSTC_SystemStatusService setNotificationSetting() new error.");
			APIC_ThrowSystemErrorNewDetailCode(SYSE_SysErrGeneral, LOGF_DEBUG_EVENT_KIND_SYS_STATUS, "SSTC_SystemStatusService setNotificationSetting() new error.", 0, createDetailCode(0x3053));
// 2015.10.16 ZeusSZX0 制御76-1 新詳細コード e.
			break;
		}
		
		// 書き込み.
		memcpy( puc_Top, puc_Data, ul_DataSize );
		
		// 該当するGroupの変化点設定情報を登録.
		m_pulNotificationMap[ e_GroupID ] = (uint32_t *)puc_Top;
		
		uint32_t *pul_Top = m_pulNotificationMap[ e_GroupID ];
		for( ushort us_Idx = 0; us_Idx < 16; us_Idx++ ){
			SST_LOG2(MFPE_LogLevelTrace, " [0x%8.8x]\n", *pul_Top);
			pul_Top++;
			if( (us_Idx + 1) == 16){ SST_LOG2(MFPE_LogLevelTrace, "\n", NULL); }
		}
	} while( 0 );
}

//************************************************************************************************************************.
//概要　　：変化点通知設定要求処理.
//　　　　：<in>　e_DataID	データID.
//　　　　：<in>　ul_Size	パラメータ長.
//戻り値　：なし.
//備考　　：特に無し.
//作成者　：高橋.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//************************************************************************************************************************.
Bool SSTC_SystemStatusService::chkSetSystemStatusDataParam( TYPE_NTF_DataID e_DataID, uint32_t ul_Size )
{
	SST_LOG2(MFPE_LogLevelTrace, "e_DataID=(0x%04x) ul_Size=(%ld).\n", e_DataID, ul_Size);
	
	struct st_CheckParam {							// データIDとデータサイズのチェックテーブル.
		TYPE_NTF_DataID	e_DataID;					// データID.
		uint32_t		ul_Min;						// 取りうる最小値.
		uint32_t		ul_Max;						// 取りうる最大値.
	} static st_CheckParamTable[] = {
		{ TYPE_NTF_DataID_SYS_14, 0                            , SYSD_MAX_SysDescr                                          },	// 製品名情報.
		{ TYPE_NTF_DataID_SYS_15, sizeof(TYPE_Rom)             , sizeof(TYPE_Rom) + TYPD_SSTS_RomVersionLength              },	// RomVersion情報.
		{ TYPE_NTF_DataID_SYS_16, sizeof(TYPE_Rom)             , sizeof(TYPE_Rom) + sizeof(ushort)                          },	// Romチェックサム.
		{ TYPE_NTF_DataID_SYS_17, sizeof(TYPE_MFPControllerRom), sizeof(TYPE_MFPControllerRom) + TYPD_SSTS_RomVersionLength },	// ＭＦＰコントローラ内のサブＲＯＭバージョン.
		{ TYPE_NTF_DataID_SYS_18, sizeof(Bool)                 , sizeof(Bool)                                               },	// 製品タイプ情報.
		{ TYPE_NTF_DataID_SYS_23, sizeof(Bool)                 , sizeof(Bool)                                               },	// ＰＣＩ拡張基板有無.
		{ TYPE_NTF_DataID_SYS_24, sizeof(Bool)                 , sizeof(Bool)                                               },	// ローカルＩ／Ｆ有無.
		{ TYPE_NTF_DataID_SYS_26, sizeof(Bool)                 , sizeof(Bool)                                               },	// ＳＩＰアダプタ有無(バックアップ必要) .
		{ TYPE_NTF_DataID_SYS_27, sizeof(Bool)                 , sizeof(Bool)                                               },	// 暗号化基板有無.
		{ TYPE_NTF_DataID_SYS_29, 0                            , TYPD_SSTS_SerialNumberLength                               },	// シリアルナンバー・プリンタ.
		{ TYPE_NTF_DataID_SYS_30, 0                            , TYPD_SSTS_SerialNumberLength                               },	// シリアルナンバー・スキャナ.
		{ TYPE_NTF_DataID_SYS_31, 0                            , TYPD_SSTS_SerialNumberLength                               },	// シリアルナンバー・ＡＤＦ.
		{ TYPE_NTF_DataID_SYS_32, 0                            , TYPD_SSTS_SerialNumberLength                               },	// シリアルナンバー・ＬＣＣ.
		{ TYPE_NTF_DataID_SYS_33, 0                            , TYPD_SSTS_SerialNumberLength                               },	// シリアルナンバー・排紙ＯＰ.
		{ TYPE_NTF_DataID_SYS_34, 0                            , TYPD_SSTS_SerialNumberLength                               },	// シリアルナンバー・Ｚ折りユニット.
		{ TYPE_NTF_DataID_SYS_35, 0                            , TYPD_SSTS_SerialNumberLength                               },	// シリアルナンバー・リレーユニット.
		{ TYPE_NTF_DataID_SYS_36, 0                            , TYPD_SSTS_SerialNumberLength                               },	// シリアルナンバー・再給紙.
		{ TYPE_NTF_DataID_SYS_37, 0                            , TYPD_SSTS_SerialNumberLength                               },	// シリアルナンバー・コピーベンダー.
		{ TYPE_NTF_DataID_SYS_38, 0                            , TYPD_SSTS_SerialNumberLength                               },	// シリアルナンバー・ファクス１.
		{ TYPE_NTF_DataID_SYS_39, 0                            , TYPD_SSTS_SerialNumberLength                               },	// シリアルナンバー・ファクス２.
		{ TYPE_NTF_DataID_SYS_48, 0                            , 0                                                          },	// 特定原稿検出エラー.
		{ TYPE_NTF_DataID_SYS_54, 0                            , TYPD_SSTS_RomVersionLength                                 },	// Pre Rom Version 設定.
		{ TYPE_NTF_DataID_SYS_55, sizeof(Bool)                 , sizeof(Bool)                                               },	// トラブルリセットモード.
		{ TYPE_NTF_DataID_SYS_56, sizeof(Bool)                 , sizeof(Bool)                                               },	// FirmWare ダウンロード 設定 .
		{ TYPE_NTF_DataID_SYS_58, sizeof(TYPE_PrinterModelKind), sizeof(TYPE_PrinterModelKind)                              },	// Thames1P モデル.
		{ TYPE_NTF_DataID_SYS_59, sizeof(Bool)                 , sizeof(Bool)                                               },	// Thames1P 強制切替モード設定.
		{ TYPE_NTF_DataID_SYS_60, sizeof(TYPE_PrinterModelKind), sizeof(TYPE_PrinterModelKind)                              },	// Thames1P 強制モデル設定.
		{ TYPE_NTF_DataID_SYS_62, sizeof(TYPE_FirmWareVersion) , sizeof(TYPE_FirmWareVersion)                               },	// Ｆ／Ｗバージョン.
		{ TYPE_NTF_DataID_SYS_64, sizeof(uchar)                , sizeof(uchar)                                              },	// MFPの機能バージョン.
		{ TYPE_NTF_DataID_SYS_67, sizeof(Bool)                 , sizeof(Bool)                                               },	// DSC1基板装着状態.
		{ TYPE_NTF_DataID_SYS_68, sizeof(Bool)                 , sizeof(Bool)                                               },	// DSC2基板装着状態.
		{ TYPE_NTF_DataID_SYS_71, sizeof(Bool)                 , sizeof(Bool)                                               },	// Bluetooth機能設定.
		{ TYPE_NTF_DataID_SYS_72, sizeof(ushort)               , sizeof(ushort)                                             }	// アウトラインフォントデータのチェックサム値.
	};
	Bool b_Ret = False;
	for( ushort us_Idx = 0; us_Idx < sizeof(st_CheckParamTable) / sizeof(struct st_CheckParam); ++us_Idx ){
		if( e_DataID == st_CheckParamTable[ us_Idx ].e_DataID ){
			if( st_CheckParamTable[ us_Idx ].ul_Min <= ul_Size && st_CheckParamTable[ us_Idx ].ul_Max >= ul_Size ){
				b_Ret = True;
			}
			break;
		}
	}
	SST_LOG2(MFPE_LogLevelTrace, "End. b_Ret=(%d).\n", b_Ret);
	return b_Ret;
}

//************************************************************************************************************************.
//概要　　：GroupID妥当性チェック処理.
//　　　　：<in>　e_GroupID		GroupID.
//戻り値　：True：GroupID or False：GroupIDでない.
//備考　　：特に無し.
//作成者　：高橋.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//************************************************************************************************************************.
Bool SSTC_SystemStatusService::checkGroupID( TYPE_NTF_GroupID e_GroupID )
{
	SST_LOG2(MFPE_LogLevelAll, "e_GroupID=(%d).\n", e_GroupID);
	
	static const TYPE_NTF_GroupID e_SSTD_GroupIDTbl[TYPE_NTF_GroupID_Max] = {
		TYPE_NTF_GroupID_Warning, TYPE_NTF_GroupID_RemotePanel, TYPE_NTF_GroupID_Address,       TYPE_NTF_GroupID_Service,
		TYPE_NTF_GroupID_System,  TYPE_NTF_GroupID_Scanner,     TYPE_NTF_GroupID_Commutication, TYPE_NTF_GroupID_PrinterDevice,
		TYPE_NTF_GroupID_Device,  TYPE_NTF_GroupID_Network,     TYPE_NTF_GroupID_Panel,         TYPE_NTF_GroupID_Other
	};
	Bool b_Ret = False;
	for( ushort us_Idx = 0; us_Idx < TYPE_NTF_GroupID_Max; ++us_Idx ){
		if( e_GroupID == e_SSTD_GroupIDTbl[ us_Idx ] ){
			b_Ret = True;
			break;
		}
	}
	SST_LOG2(MFPE_LogLevelAll, "End. b_Ret=(%d).\n", b_Ret);
	return b_Ret;
}

// STEP2：2010.12.10 <ADD> End   T.Takahashi.

//************************************************************************************************************************.
//概要　　：全オブザーバー登録.
//引数　　：なし.
//戻り値　：なし.
//備考　　：特に無し.
//作成者　：高橋.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
// 2011.12.08 CIT不具合対応[マージ後不具合管理票 No.164].
//  - APIC_FaxObserver::xxx()通知情報     st_DevXX->st_DevXX[TYPE_CommLine2].
// 2012. 5.15 [CQ363516] パフォーマンス改善 Observerで検出可能な情報をポーリング→Observerに変更した対応で追加した関数.
// 2012/09/13, ADS坪井, CQ369866:Android接続中通知追加.
// 2013/02/25 日立JTE 古川 【MP-IDB-01】USBサムネイル削除対応.SSTC_ConcreteImageDecoderObserverクラスインスタンス生成処理コメントアウト.
// 2013/10/16 【DS-IGL】日立JTE 古川 画像ログオプション化 APIC_ImageLogControllへのアタッチを別処理へ移動し削除.
//************************************************************************************************************************.
void SSTC_SystemStatusService::attachAllObserver()
{
	SST_LOG(MFPE_LogLevelTrace);
	
	// オブザーバ登録(リモートパネル).
	m_pcPrinterSystemObserver = SSTC_ConcretePrinterSystemObserver::newInstance( );
	APIC_PrinterSystem::newInstance()->attachPrinterSystemObserver( m_pcPrinterSystemObserver );
	
	m_pcRemotePanelObserver = SSTC_ConcreteRemotePanelObserver::newInstance( );
	APIC_RemotePanel::newInstance()->attachObserver( m_pcRemotePanelObserver );
	
	m_pcRemotePanelRMP_Observer = SSTC_ConcreteRemotePanelRMP_Observer::newInstance( );
	APIC_RMP::newInstance()->attachObserver( m_pcRemotePanelRMP_Observer );

	m_pcRemotePanelRMP_KeyOff_Observer = SSTC_ConcreteRemotePanelRMP_KeyOff_Observer::newInstance( );
	APIC_RMP_KeyOff::newInstance()->attachObserver( m_pcRemotePanelRMP_KeyOff_Observer );

	m_pcRemotePanelEXC_APPDispReqObserver = SSTC_ConcreteRemotePanelEXC_APPDispReqObserver::newInstance( );
	APIC_EXC_APPDispReq::newInstance()->attachObserver( m_pcRemotePanelEXC_APPDispReqObserver );

	m_pcRemotePanelEXC_RPnlCloseRequest = SSTC_ConcreteRemotePanelEXC_RPnlCloseRequest::newInstance( );
	APIC_EXC_RPnlCloseRequest::newInstance()->attachObserver( m_pcRemotePanelEXC_RPnlCloseRequest );

	m_pcRemotePanelEXC_RPnlCommunicationNoticeObserver = SSTC_ConcreteRemotePanelEXC_RPnlCommunicationNoticeObserver::newInstance( );
	APIC_EXC_RPnlCommunicationNotice::newInstance()->attachObserver( m_pcRemotePanelEXC_RPnlCommunicationNoticeObserver );

	m_pcRemotePanelEXC_RPnlFunctionNoticeObserver = SSTC_ConcreteRemotePanelEXC_RPnlFunctionNoticeObserver::newInstance( );
	APIC_EXC_RPnlFunctionNotice::newInstance()->attachObserver( m_pcRemotePanelEXC_RPnlFunctionNoticeObserver );

	m_pcRemotePanelEXC_RPnlStatusNoticeObserver = SSTC_ConcreteRemotePanelEXC_RPnlStatusNoticeObserver::newInstance( );
	APIC_EXC_RPnlStatusNotice::newInstance()->attachObserver( m_pcRemotePanelEXC_RPnlStatusNoticeObserver );

	m_pcTabletIFObserver = SSTC_ConcreteTabletIFObserver::newInstance( );
	APIC_TabletIF::newInstance()->attachObserver( m_pcTabletIFObserver );

	// オブザーバ登録(アドレス).
	m_pcFwdAutoInfoObserver = SSTC_ConcreteFwdAutoInfoObserver::newInstance( );
	
	m_pcAddressNumberObserver = SSTC_ConcreteAddressNumberInfoObserver::newInstance( );
	APIC_MyAddress::newInstance()->attachObserver( m_pcAddressNumberObserver );


	m_pcFaxTsiRoutingObserver = SSTC_ConcreteFaxTsiRoutingObserver::newInstance( );

	// オブザーバ登録(サービス).
	m_pcExternalScannerControllerObserver = SSTC_ConcreteExternalScannerControllerObserver::newInstance( );
	APIC_ScannerExternalIF::newInstance()->attachScannerExternalIFObserver( m_pcExternalScannerControllerObserver );
	
	m_pcJLGObserver = SSTC_ConcreteJLGObserver::newInstance( );
	APIC_JLG::newInstance()->attachObserver( m_pcJLGObserver );
	
	m_pcApplicationFileIDObserver = SSTC_ConcreteApplicationFileIDObserver::newInstance( );
	APIC_ApplicationFileIo::newInstance()->attachObserver( m_pcApplicationFileIDObserver );
	
	m_pcOperationSettingObserver = SSTC_ConcreteOperationSettingObserver::newInstance( );
	APIC_OperationSetting::newInstance()->attachObserver( m_pcOperationSettingObserver );
	
	m_pcPrintJobSettingObserver = SSTC_ConcretePrintJobSettingObserver::newInstance( );
	APIC_PrintJobSetting::newInstance()->attachObserver( m_pcPrintJobSettingObserver );
	
	// 新API対応：2011.05.26 <ADD> Start T.Takahashi.
#ifdef PLUGIN_OBS_ENABLE

	m_pcPowerServiceObserver = SSTC_ConcretePowerServiceObserver::newInstance( );
	APIC_PowerServiceIF::newInstance()->attachObserver( m_pcPowerServiceObserver );
	
	m_pcImageFormatterObserver = SSTC_ConcreteImageFormatterObserver::newInstance( );
	CNVC_ImageFormatterIf::newInstance()->attachObserver( m_pcImageFormatterObserver );
	
	m_pcNetPrintRxPluginAdapterObserver = SSTC_ConcreteNetPrintRxPluginAdapterObserver::newInstance( );
	NETC_NetPrintRxPluginAdapter::newInstance()->attachObserver( m_pcNetPrintRxPluginAdapterObserver );

	m_pcNetTxPluginAdapterObserver = SSTC_ConcreteNetTxPluginAdapterObserver::newInstance( );
	NETC_NetTxPluginAdapter::newInstance()->attachObserver( m_pcNetTxPluginAdapterObserver );

	m_pcNetFaxRxPluginAdapterObserver = SSTC_ConcreteNetFaxRxPluginAdapterObserver::newInstance( );
	NETC_NetFaxRxPluginAdapter::newInstance()->attachObserver( m_pcNetFaxRxPluginAdapterObserver );

#endif
	// 新API対応：2011.05.26 <ADD> End T.Takahashi.
// 認証プラグイン対応_Start. ADS 2011.11.01.
	m_pcAuthPluginAdapterObserver = SSTC_ConcreteAuthPluginAdapterObserver::newInstance( );
	AUTC_AuthPluginIF::newInstance()->attachObserver( m_pcAuthPluginAdapterObserver );
// 認証プラグイン対応_End.   ADS 2011.11.01.
	
	m_pcDebugLogSystemSettingObserver = SSTC_ConcreteDebugLogSystemSettingObserver::newInstance( );
	APIC_DebugLogSystemSetting::newInstance()->attachObserver( m_pcDebugLogSystemSettingObserver );
	m_pcUserAuthenticationObserver = SSTC_ConcreteUserAuthenticationObserver::newInstance( );
	APIC_UserAuthentication::newInstance()->attachObserver( m_pcUserAuthenticationObserver );
	
// 2016/07/01 2Sys-Sky西村 RQ-436対応 start.
	m_pcIwsObserver = SSTC_ConcreteIwsObserver::newInstance( );
	APIC_Iws::newInstance()->attachObserver( m_pcIwsObserver );
// 2016/07/01 2Sys-Sky西村 RQ-436対応 end.

	// オブザーバ登録(システム).
	m_pcSystemStatusObserver = SSTC_ConcreteSystemStatusObserver::newInstance( );
	APIC_SystemStatus::newInstance()->attachObserver( m_pcSystemStatusObserver );
	APIC_HddInfo::newInstance()->attachObserver( m_pcSystemStatusObserver );
	
	m_pcEnablerSettingObserver = SSTC_ConcreteEnablerSettingObserver::newInstance( );
	APIC_EnablerSetting::newInstance()->attachObserver( m_pcEnablerSettingObserver );
	
	m_pcSystemInfoObserver = SSTC_ConcreteSystemInfoObserver::newInstance( );
	APIC_SystemInfo::newInstance()->attachObserver( m_pcSystemInfoObserver );
	
	m_pcSystemSettingObserver = SSTC_ConcreteSystemSettingObserver::newInstance( );
	APIC_SystemSetting::newInstance()->attachObserver( m_pcSystemSettingObserver );
	
	m_pcBMLinkSStatusObserver = SSTC_ConcreteBMLinkSStatusObserver::newInstance( );
	APIC_BMLinkSStatus::newInstance()->attachObserver( m_pcBMLinkSStatusObserver );
	
	m_pcBMLinkSSettingsObserver = SSTC_ConcreteBMLinkSSettingsObserver::newInstance( );
	APIC_BMLinkSSettings::newInstance()->attachObserver( m_pcBMLinkSSettingsObserver );
	
	m_pcCopyProtectInfoSettingObserver = SSTC_ConcreteCopyProtectInfoSettingObserver::newInstance( );
	APIC_CopyProtectInfoSetting::newInstance()->attachObserver( m_pcCopyProtectInfoSettingObserver );
	
	m_pcDatabaseStatusObserver = SSTC_ConcreteDatabaseStatusObserver::newInstance( );
	APIC_DatabaseStatus::newInstance()->attachObserver( m_pcDatabaseStatusObserver );
	
	m_pcHddInfoObserver = SSTC_ConcreteHddInfoObserver::newInstance( );
	APIC_HddInfo::newInstance()->attachObserver( m_pcHddInfoObserver );
	
// Eagle:制御構成変更No.104_テストパターン印刷のターミナルコマンド対応(生産対応）start.
	m_pcSystemInfoObserver3 = SSTC_ConcreteSystemInfoObserver3::newInstance( );
	APIC_SystemInfo::newInstance()->attachObserver( m_pcSystemInfoObserver3 );
// Eagle:制御構成変更No.104_テストパターン印刷のターミナルコマンド対応(生産対応）end.
	
	// オブザーバ登録(スキャナー).
	m_pcWSDSettingObserver = SSTC_ConcreteWSDSettingObserver::newInstance( );
	APIC_WSDSetting::newInstance()->attachErrorNoticeObserver( m_pcWSDSettingObserver );
	
	m_pcOpticalAdjustObserver = SSTC_ConcreteOpticalAdjustObserver::newInstance( );
	APIC_ScanerSetting::newInstance()->attachObserver( m_pcOpticalAdjustObserver );
	
	m_pcScanJobSettingObserver = SSTC_ConcreteScanJobSettingObserver::newInstance( );
	APIC_ScanJobSetting::newInstance()->attachObserver( m_pcScanJobSettingObserver );
	
	m_pcScannerServiceInfoObserver = SSTC_ConcreteScannerServiceInfoObserver::newInstance( );
	APIC_ScannerServiceInfo::newInstance()->attachScannerServiceInfoObserver( m_pcScannerServiceInfoObserver );
	
	m_pcScannerServiceSettingObserver = SSTC_ConcreteScannerServiceSettingObserver::newInstance( );
	APIC_ScannerServiceSetting::newInstance()->attachScannerServiceSettingObserver( m_pcScannerServiceSettingObserver );
	
	m_pcScannerStatusObserver = SSTC_ConcreteScannerStatusObserver::newInstance( );
	APIC_ScannerStatus::newInstance()->attachScannerStatusObserver( m_pcScannerStatusObserver );
	
	m_pcScannerSystemObserver = SSTC_ConcreteScannerSystemObserver::newInstance( );
	APIC_ScannerSystem::newInstance()->attachScannerSystemObserver( m_pcScannerSystemObserver );
	
	// オブザーバ登録(通信).
	m_pcRdsSettingObserver = SSTC_ConcreteRdsSettingObserver::newInstance( );
	APIC_RdsSetting::newInstance()->attachObserver( m_pcRdsSettingObserver );
	
	m_pcRdsObserver = SSTC_ConcreteRdsObserver::newInstance( );
	APIC_Rds::newInstance()->attachObserver( m_pcRdsObserver );
	
	m_pcLogRdsJnlDataModemObserver = SSTC_ConcreteLogRdsJnlDataModemObserver::newInstance( );
	APIC_LogRdsJnlDataModem::newInstance()->attachObserver( m_pcLogRdsJnlDataModemObserver );
	
	m_pcLogRdsJnlEmailObserver = SSTC_ConcreteLogRdsJnlEmailObserver::newInstance( );
	APIC_LogRdsJnlEmail::newInstance()->attachObserver( m_pcLogRdsJnlEmailObserver );
	
	// オブザーバ登録(デバイス_プリンタ).
	m_pcPrinterInfoObserver = SSTC_ConcretePrinterInfoObserver::newInstance( );
	APIC_PrinterInfo::newInstance()->attachPrinterInfoObserver( m_pcPrinterInfoObserver );
	
	m_pcPrinterPortSettingObserver = SSTC_ConcretePrinterPortSettingObserver::newInstance( );
	APIC_PrinterPortSetting::newInstance()->attachObserver( m_pcPrinterPortSettingObserver );
	
	m_pcPrinterServiceInfoObserver = SSTC_ConcretePrinterServiceInfoObserver::newInstance( );
	APIC_PrinterServiceInfo::newInstance()->attachPrinterServiceInfoObserver( m_pcPrinterServiceInfoObserver );
	
	m_pcPrinterServiceSettingObserver = SSTC_ConcretePrinterServiceSettingObserver::newInstance( );
	APIC_PrinterServiceSetting::newInstance()->attachPrinterServiceSettingObserver( m_pcPrinterServiceSettingObserver );
	
	m_pcPrinterRegulationObserver = SSTC_ConcretePrinterRegulationObserver::newInstance( );
	APIC_PrinterSetting::newInstance()->attachObserver( m_pcPrinterRegulationObserver );
	
	m_pcPrinterStatusObserver = SSTC_ConcretePrinterStatusChangedObserver::newInstance( );
	APIC_PrinterStatus::newInstance()->attachPrinterStatusObserver( m_pcPrinterStatusObserver );

	m_pcPrinterWarningObserver = SSTC_ConcretePrinterWarningObserver::newInstance( );
	APIC_PrinterWarning::newInstance()->attachPrinterWarningObserver( m_pcPrinterWarningObserver );
	
	// オブザーバ登録(デバイス).
	for ( TYPE_CommLine i = (TYPE_CommLine)0, e_Line = TYPE_CommLine1; i < TYPE_CommLine2; i++, e_Line++ ) {
		m_pcFaxObserver[(ushort)i] = SSTC_ConcreteFaxObserver::newInstance(e_Line);
		APIC_ObsvrMgr::instance( e_Line )->attachObserver( m_pcFaxObserver[(ushort)i] );
	}

	m_pcIGCdevListFinishObserver = SSTC_ConcreteIGCdevListFinishObserver::newInstance( );

	m_pcPowerObserver = SSTC_ConcretePowerObserver::newInstance( );
	APIC_Power::newInstance()->attachObserver( m_pcPowerObserver );

	m_pcPmCntObserver = SSTC_ConcretePmCntObserver::newInstance( );
	APIC_PmCnt::newInstance()->attachObserver( m_pcPmCntObserver );

	m_pcUsbMemDeviceInfoObserver = SSTC_ConcreteUsbMemDeviceInfoObserver::newInstance( );
	APIC_UsbMemDeviceInfo::newInstance()->attachObserver( m_pcUsbMemDeviceInfoObserver );

	m_pcLifeCntObserver = SSTC_ConcreteLifeCntObserver::newInstance( );
	APIC_LifeCnt::newInstance()->attachObserver( m_pcLifeCntObserver );

	m_pcAccumulateCoverageCntObserver = SSTC_ConcreteAccumulateCoverageCntObserver::newInstance( );
	APIC_AccumulateCoverageCnt::newInstance()->attachObserver( m_pcAccumulateCoverageCntObserver );

	m_pcAdfCntObserver = SSTC_ConcreteAdfCntObserver::newInstance( );
	APIC_AdfCnt::newInstance()->attachObserver( m_pcAdfCntObserver );

	m_pcCompressRateCntObserver = SSTC_ConcreteCompressRateCntObserver::newInstance( );
	APIC_CompressRateCnt::newInstance()->attachObserver( m_pcCompressRateCntObserver );

	m_pcConsumableInfoObserver = SSTC_ConcreteConsumableInfoObserver::newInstance( );
	APIC_ConsumableInfo::newInstance()->attachObserver( m_pcConsumableInfoObserver );

	m_pcCopyJobSettingObserver = SSTC_ConcreteCopyJobSettingObserver::newInstance( );
	APIC_CopyJobSetting::newInstance()->attachObserver( m_pcCopyJobSettingObserver );

	m_pcCopyKitCounterObserver = SSTC_ConcreteCopyKitCounterObserver::newInstance( );
	APIC_CopyKitCounter::newInstance()->attachObserver( m_pcCopyKitCounterObserver );

	m_pcCopyModeCntObserver = SSTC_ConcreteCopyModeCntObserver::newInstance( );
	APIC_CopyModeCnt::newInstance()->attachObserver( m_pcCopyModeCntObserver );

	m_pcCoverageCntObserver = SSTC_ConcreteCoverageCntObserver::newInstance( );
	APIC_CoverageCnt::newInstance()->attachObserver( m_pcCoverageCntObserver );

	m_pcCSRCCoverageCntObserver = SSTC_ConcreteCSRCCoverageCntObserver::newInstance( );
	APIC_CSRCCoverageCnt::newInstance()->attachObserver( m_pcCSRCCoverageCntObserver );

	m_pcDustCntObserver = SSTC_ConcreteDustCntObserver::newInstance( );
	APIC_DustCnt::newInstance()->attachObserver( m_pcDustCntObserver );

	m_pcFaxBoxSettingObserver = SSTC_ConcreteFaxBoxSettingObserver::newInstance( );
	APIC_FaxBoxSetting::newInstance()->attachObserver( m_pcFaxBoxSettingObserver );

	m_pcFaxCntObserver = SSTC_ConcreteFaxCntObserver::newInstance( );
	APIC_FaxCnt::newInstance()->attachObserver( m_pcFaxCntObserver );

	m_pcFaxErrorCntObserver = SSTC_ConcreteFaxErrorCntObserver::newInstance( );
	APIC_FaxErrorCnt::newInstance()->attachObserver( m_pcFaxErrorCntObserver );

	m_pcFaxGfpSettingObserver = SSTC_ConcreteFaxGfpSettingObserver::newInstance( );
	APIC_FaxGfpSetting::newInstance()->attachObserver( m_pcFaxGfpSettingObserver );

	m_pcFaxGhpSettingObserver = SSTC_ConcreteFaxGhpSettingObserver::newInstance( );
	APIC_FaxGhpSetting::newInstance()->attachObserver( m_pcFaxGhpSettingObserver );

	m_pcFaxGswSettingObserver = SSTC_ConcreteFaxGswSettingObserver::newInstance( );
	APIC_FaxGswSetting::newInstance()->attachObserver( m_pcFaxGswSettingObserver );

	m_pcFaxJobSettingObserver = SSTC_ConcreteFaxJobSettingObserver::newInstance( );
	APIC_FaxJobSetting::newInstance()->attachObserver( m_pcFaxJobSettingObserver );

	m_pcFaxJobSetting2Observer = SSTC_ConcreteFaxJobSetting2Observer::newInstance( );
	APIC_FaxJobSetting2::newInstance()->attachObserver( m_pcFaxJobSetting2Observer );

	m_pcFeedTrayObserver = SSTC_ConcreteFeedTrayObserver::newInstance( );
	APIC_FeedTray::newInstance()->attachFeedTrayObserver( m_pcFeedTrayObserver );

	m_pcHighRankCoverageCntObserver = SSTC_ConcreteHighRankCoverageCntObserver::newInstance( );
	APIC_HighRankCoverageCnt::newInstance()->attachObserver( m_pcHighRankCoverageCntObserver );

	m_pcIGCdeviceListObserver = SSTC_ConcreteIGCdeviceListObserver::newInstance( );
	APIC_IGCdeviceList::newInstance()->attachObserver( m_pcIGCdeviceListObserver );

	m_pcJamCntObserver = SSTC_ConcreteJamCntObserver::newInstance( );
	APIC_JamCnt::newInstance()->attachObserver( m_pcJamCntObserver );

	m_pcJamDailyCntObserver = SSTC_ConcreteJamDailyCntObserver::newInstance( );
	APIC_JamDailyCnt::newInstance()->attachObserver( m_pcJamDailyCntObserver );

	m_pcJamTotalCntObserver = SSTC_ConcreteJamTotalCntObserver::newInstance( );
	APIC_JamTotalCnt::newInstance()->attachObserver( m_pcJamTotalCntObserver );

	m_pcJimonPatternSettingObserver = SSTC_ConcreteJimonPatternSettingObserver::newInstance( );
	APIC_JimonPatternSetting::newInstance()->attachObserver( m_pcJimonPatternSettingObserver );

	m_pcLEDStatusSettingObserver = SSTC_ConcreteLEDStatusSettingObserver::newInstance( );
	APIC_LEDStatusSetting::newInstance()->attachObserver( m_pcLEDStatusSettingObserver );

	m_pcLogJamObserver = SSTC_ConcreteLogJamObserver::newInstance( );
	APIC_LogJam::newInstance()->attachObserver( m_pcLogJamObserver );

	m_pcLogTroubleObserver = SSTC_ConcreteLogTroubleObserver::newInstance( );
	APIC_LogTrouble::newInstance()->attachObserver( m_pcLogTroubleObserver );

	m_pcMachineSettingObserver = SSTC_ConcreteMachineSettingObserver::newInstance( );
	APIC_MachineSetting::newInstance()->attachObserver( m_pcMachineSettingObserver );

	m_pcMechaCounterObserver = SSTC_ConcreteMechaCounterObserver::newInstance( );
	APIC_MechaCounter::newInstance()->attachObserver( m_pcMechaCounterObserver );

	m_pcMemoryInfoObserver = SSTC_ConcreteMemoryInfoObserver::newInstance( );
	APIC_MemoryInfo::newInstance()->attachObserver( m_pcMemoryInfoObserver );

	m_pcMobileSettingObserver = SSTC_ConcreteMobileSettingObserver::newInstance( );
	APIC_MobileSetting::newInstance()->attachObserver( m_pcMobileSettingObserver );

	m_pcOptionPartsCntObserver = SSTC_ConcreteOptionPartsCntObserver::newInstance( );
	APIC_OptionPartsCnt::newInstance()->attachObserver( m_pcOptionPartsCntObserver );

	m_pcOriginalObserver = SSTC_ConcreteOriginalObserver::newInstance( );
	APIC_Original::newInstance()->attachOriginalObserver( m_pcOriginalObserver );

	m_pcOutputTrayObserver = SSTC_ConcreteOutputTrayObserver::newInstance( );
	APIC_OutputTray::newInstance()->attachOutputTrayObserver( m_pcOutputTrayObserver );

	m_pcPaperSizeCntCSRCObserver = SSTC_ConcretePaperSizeCntCSRCObserver::newInstance( );
	APIC_PaperSizeCntCSRC::newInstance()->attachObserver( m_pcPaperSizeCntCSRCObserver );

	m_pcPaperSizeCntObserver = SSTC_ConcretePaperSizeCntObserver::newInstance( );
	APIC_PaperSizeCnt::newInstance()->attachObserver( m_pcPaperSizeCntObserver );

	m_pcPartsCntObserver = SSTC_ConcretePartsCntObserver::newInstance( );
	APIC_PartsCnt::newInstance()->attachObserver( m_pcPartsCntObserver );

	m_pcPrefixObserver = SSTC_ConcretePrefixObserver::newInstance( );
	APIC_Prefix::newInstance()->attachObserver( m_pcPrefixObserver );

	m_pcPreviewSettingObserver = SSTC_ConcretePreviewSettingObserver::newInstance( );
	APIC_PreviewSetting::newInstance()->attachObserver( m_pcPreviewSettingObserver );

	m_pcReuseCntObserver = SSTC_ConcreteReuseCntObserver::newInstance( );
	APIC_ReuseCnt::newInstance()->attachObserver( m_pcReuseCntObserver );

	m_pcSoftDipSwObserver = SSTC_ConcreteSoftDipSwObserver::newInstance( );
	APIC_SoftDipSw::newInstance()->attachObserver( m_pcSoftDipSwObserver );

	m_pcStampSettingObserver = SSTC_ConcreteStampSettingObserver::newInstance( );
	APIC_StampSetting::newInstance()->attachObserver( m_pcStampSettingObserver );

	m_pcStatisticsPageObserver = SSTC_ConcreteStatisticsPageObserver::newInstance( );
	APIC_StatisticsPage::newInstance()->attachObserver( m_pcStatisticsPageObserver );

	m_pcTonerChangeCntObserver = SSTC_ConcreteTonerChangeCntObserver::newInstance( );
	APIC_TonerChangeCnt::newInstance()->attachObserver( m_pcTonerChangeCntObserver );

	m_pcTotalCntObserver = SSTC_ConcreteTotalCntObserver::newInstance( );
	APIC_TotalCnt::newInstance()->attachObserver( m_pcTotalCntObserver );

	m_pcTroubleCntObserver = SSTC_ConcreteTroubleCntObserver::newInstance( );
	APIC_TroubleCnt::newInstance()->attachObserver( m_pcTroubleCntObserver );

	m_pcTroubleTotalCntObserver = SSTC_ConcreteTroubleTotalCntObserver::newInstance( );
	APIC_TroubleTotalCnt::newInstance()->attachObserver( m_pcTroubleTotalCntObserver );

	m_pcUsbMemSettingObserver = SSTC_ConcreteUsbMemSettingObserver::newInstance( );
	APIC_UsbMemSetting::newInstance()->attachObserver( m_pcUsbMemSettingObserver );

	m_pcWarningCntObserver = SSTC_ConcreteWarningCntObserver::newInstance( );
	APIC_WarningCnt::newInstance()->attachObserver( m_pcWarningCntObserver );

	// オブザーバ登録(ネットワーク).
	m_pcWebDAVTimerResetObserver = SSTC_ConcreteWebDAVTimerResetObserver::newInstance( );
	APIC_WebDAVdSetting::newInstance()->attachObserver( m_pcWebDAVTimerResetObserver );
	
	m_pcSnmpTrapSetObserver = SSTC_ConcreteSnmpTrapSettingObserver::newInstance( );
	APIC_SnmpTrapSetting::newInstance()->attachObserver( m_pcSnmpTrapSetObserver );
	
	m_pcWebDAVSvrCtrlObserver = SSTC_ConcreteWebDAVServerControlObserver::newInstance( );
	APIC_WebDAVServerControl::newInstance()->attachObserver( m_pcWebDAVSvrCtrlObserver );
	
	m_pcCntMailSndSetObserver = SSTC_ConcreteCounterMailSendSettingObserver::newInstance( );
	APIC_CounterMailSendSetting::newInstance()->attachObserver( m_pcCntMailSndSetObserver );
	
	m_pcAppleTalkSetObserver = SSTC_ConcreteAppleTalkSettingObserver::newInstance( );
	APIC_AppleTalkSetting::newInstance()->attachObserver( m_pcAppleTalkSetObserver );
	
	m_pcBonjourSetObserver = SSTC_ConcreteBonjourSettingObserver::newInstance( );
	APIC_BonjourSetting::newInstance()->attachObserver( m_pcBonjourSetObserver );
	
	m_pcBrowserSetObserver = SSTC_ConcreteBrowserSettingObserver::newInstance( );
	APIC_BrowserSetting::newInstance()->attachObserver( m_pcBrowserSetObserver );
	
	m_pcCertInfoSetObserver = SSTC_ConcreteCertInfoSettingObserver::newInstance( );
	APIC_CertInfoSetting::newInstance()->attachObserver( m_pcCertInfoSetObserver );
	
	m_pcFtpClientSetObserver = SSTC_ConcreteFtpClientSettingObserver::newInstance( );
	APIC_FtpClientSetting::newInstance()->attachObserver( m_pcFtpClientSetObserver );
	
	m_pcIEEE802SetObserver = SSTC_ConcreteIEEE802SettingObserver::newInstance( );
	APIC_IEEE802Setting::newInstance()->attachObserver( m_pcIEEE802SetObserver );
	
	m_pcInternetISWObserver = SSTC_ConcreteInternetISWObserver::newInstance( );
	APIC_InternetISW::newInstance()->attachObserver( m_pcInternetISWObserver );
	
	m_pcIpFilteringSetObserver = SSTC_ConcreteIpFilteringSettingObserver::newInstance( );
	APIC_IpFilteringSetting::newInstance()->attachObserver( m_pcIpFilteringSetObserver );
	
	m_pcIppSetObserver = SSTC_ConcreteIppSettingObserver::newInstance( );
	APIC_IppSetting::newInstance()->attachObserver( m_pcIppSetObserver );
	
	m_pcIPsecSetObserver = SSTC_ConcreteIPsecSettingObserver::newInstance( );
	APIC_IPsecSetting::newInstance()->attachObserver( m_pcIPsecSetObserver );
	
	m_pcIPv6SetObserver = SSTC_ConcreteIPv6SettingObserver::newInstance( );
	APIC_IPv6Setting::newInstance()->attachObserver( m_pcIPv6SetObserver );
	
	m_pcLdapSetObserver = SSTC_ConcreteLdapSettingObserver::newInstance( );
	APIC_LdapSetting::newInstance()->attachObserver( m_pcLdapSetObserver );
	
	m_pcMySpoolSvrSetObserver = SSTC_ConcreteMySpoolServerSettingObserver::newInstance( );
	APIC_MySpoolServerSetting::newInstance()->attachObserver( m_pcMySpoolSvrSetObserver );
	
	m_pcNetWareSetObserver = SSTC_ConcreteNetWareSettingObserver::newInstance( );
	APIC_NetWareSetting::newInstance()->attachObserver( m_pcNetWareSetObserver );
	
	m_pcNWDevObserver = SSTC_ConcreteNetworkDeviceObserver::newInstance( );
	APIC_NetworkDevice::newInstance()->attachObserver( m_pcNWDevObserver );
	
	m_pcNWFaxSetObserver = SSTC_ConcreteNetworkFaxSettingObserver::newInstance( );
	APIC_NetworkFaxSetting::newInstance()->attachObserver( m_pcNWFaxSetObserver );
	
	m_pcNtpSetObserver = SSTC_ConcreteNtpSettingObserver::newInstance( );
	APIC_NtpSetting::newInstance()->attachObserver( m_pcNtpSetObserver );
	
	m_pcPswcMibSetObserver = SSTC_ConcretePswcMibSettingObserver::newInstance( );
	APIC_PswcMibSetting::newInstance()->attachObserver( m_pcPswcMibSetObserver );
	
	m_pcSlpSetObserver = SSTC_ConcreteSlpSettingObserver::newInstance( );
	APIC_SlpSetting::newInstance()->attachObserver( m_pcSlpSetObserver );
	
	m_pcSmbPrtSetObserver = SSTC_ConcreteSmbPrintSettingObserver::newInstance( );
	APIC_SmbPrintSetting::newInstance()->attachObserver( m_pcSmbPrtSetObserver );
	
	m_pcSmbSetObserver = SSTC_ConcreteSmbSettingObserver::newInstance( );
	APIC_SmbSetting::newInstance()->attachObserver( m_pcSmbSetObserver );
	
	m_pcSmimeCertObserver = SSTC_ConcreteSmimeCertInfoObserver::newInstance( );
	APIC_SmimeCertInfo::newInstance()->attachObserver( m_pcSmimeCertObserver );
	
	m_pcSmimeSetObserver = SSTC_ConcreteSmimeSettingObserver::newInstance( );
	APIC_SmimeSetting::newInstance()->attachObserver( m_pcSmimeSetObserver );
	
	m_pcSnmpSetObserver = SSTC_ConcreteSnmpSettingObserver::newInstance( );
	APIC_SnmpSetting::newInstance()->attachObserver( m_pcSnmpSetObserver );
	
	m_pcSNMPv3PenaltyCntObserver = SSTC_ConcreteSNMPv3PenaltyCntObserver::newInstance( );
	APIC_SNMPv3PenaltyCnt::newInstance()->attachObserver( m_pcSNMPv3PenaltyCntObserver );
	
	m_pcSSDPSetObserver = SSTC_ConcreteSSDPSettingsObserver::newInstance( );
	APIC_SSDPSettings::newInstance()->attachObserver( m_pcSSDPSetObserver );
	
	m_pcSslSetObserver = SSTC_ConcreteSslSettingObserver::newInstance( );
	APIC_SslSetting::newInstance()->attachObserver( m_pcSslSetObserver );
	
	m_pcTcpIpSetObserver = SSTC_ConcreteTcpIpSettingObserver::newInstance( );
	APIC_TcpIpSetting::newInstance()->attachObserver( m_pcTcpIpSetObserver );
	
	m_pcTcpSockSetObserver = SSTC_ConcreteTcpSocketSettingObserver::newInstance( );
	APIC_TcpSocketSetting::newInstance()->attachObserver( m_pcTcpSockSetObserver );
	
	m_pcWebDAVPenaltyCntObserver = SSTC_ConcreteWebDAVdPenaltyCntObserver::newInstance( );
	APIC_WebDAVdPenaltyCnt::newInstance()->attachObserver( m_pcWebDAVPenaltyCntObserver );
	
	m_pcWebDAVSvrSetObserver = SSTC_ConcreteWebDAVdServerSettingStateChangedObserver::newInstance( );
	APIC_WebDAVdSetting::newInstance()->attachObserver( m_pcWebDAVSvrSetObserver );
	
	m_pcWebDAVSetObserver = SSTC_ConcreteWebDavSettingStateChangedObserver::newInstance( );
	APIC_WebDavSetting::newInstance()->attachObserver( m_pcWebDAVSetObserver );
	
	m_pcWirelessSetObserver = SSTC_ConcreteWirelessSettingObserver::newInstance( );
	APIC_WirelessSetting::newInstance()->attachObserver( m_pcWirelessSetObserver );
	
	m_pcMailPOPRcvSetObserver = SSTC_ConcreteMailPOPReceiveSettingObserver::newInstance( );
	APIC_MailReceiveSetting::newInstance()->attachObserver( m_pcMailPOPRcvSetObserver );
	
	m_pcMailRecSmtpSetObserver = SSTC_ConcreteMailRecSmtpSettingObserver::newInstance( );
	APIC_MailRecSmtpSetting::newInstance()->attachObserver( m_pcMailRecSmtpSetObserver );
	
	m_pcMailSndSetObserver = SSTC_ConcreteMailSendSettingObserver::newInstance( );
	APIC_MailSendSetting::newInstance()->attachObserver( m_pcMailSndSetObserver );
	
	m_pcSmtpRcvSetObserver = SSTC_ConcreteSmtpRecvSettingObserver::newInstance( );
	APIC_SmtpRecvSetting::newInstance()->attachObserver( m_pcSmtpRcvSetObserver );
	
	m_pcSmtpSndSetObserver = SSTC_ConcreteSmtpSendSettingObserver::newInstance( );
	APIC_SmtpSendSetting::newInstance()->attachObserver( m_pcSmtpSndSetObserver );
	
	// オブザーバ登録(パネル情報).
	m_pcWatermarkSetObserver = SSTC_ConcreteWatermarkSettingObserver::newInstance( );
	APIC_WatermarkSetting::newInstance()->attachObserver( m_pcWatermarkSetObserver );

	m_pcAppKeySetObserver = SSTC_ConcreteApplicationKeySettingObserver::newInstance( );
	APIC_ApplicationKeySetting::newInstance()->attachObserver( m_pcAppKeySetObserver );
	
	m_pcDefPanelSetObserver = SSTC_ConcreteDefaultPanelSettingObserver::newInstance( );
	APIC_DefaultPanelSetting::newInstance()->attachObserver( m_pcDefPanelSetObserver );
	
	m_pcFaxPanelSetObserver = SSTC_ConcreteFaxPanelSettingObserver::newInstance( );
	APIC_FaxPanelSetting::newInstance()->attachObserver( m_pcFaxPanelSetObserver );
	
	// オブザーバ登録(その他).
	m_pcPDFSettingObserver = SSTC_ConcreteSearchablePDFSettingObserver::newInstance( );
	APIC_SearchablePDFSetting::newInstance()->attachObserver( m_pcPDFSettingObserver );
	
//2013/10/16 【DS-IGL】日立JTE 古川 画像ログオプション化によるシーケンス変更.
//	m_pcImageLogObserver = SSTC_ConcreteImageLogObserver::newInstance( );
//	APIC_ImageLogControll::newInstance()->attachObserver( m_pcImageLogObserver );
	
	m_pcFaxReportObserver = SSTC_ConcreteFaxListReportSettingObserver::newInstance( );
	APIC_FaxListReportSetting::newInstance()->attachObserver( m_pcFaxReportObserver );
	
	m_pcLicenseInfoObserver = SSTC_ConcreteLicenseInfoSettingObserver::newInstance( );
	APIC_LicenseInfoSetting::newInstance()->attachObserver( m_pcLicenseInfoObserver );
	
	m_pcOpenApiSettingObserver = SSTC_ConcreteOpenApiSettingObserver::newInstance( );
	APIC_OpenApiSetting::newInstance()->attachObserver( m_pcOpenApiSettingObserver );

// 【RQ-747 VirusScanFUM for FVT2】 2018.04.23 Sky.A.Tamura Add Start ->.
	m_pcVirusScanSettingObserver = SSTC_ConcreteVirusScanSettingObserver::newInstance( );
	APIC_VirusScanSetting::newInstance()->attachObserver( m_pcVirusScanSettingObserver );
// 【RQ-747 VirusScanFUM for FVT2】 2018.04.23 Sky.A.Tamura Add End   <-.

// 【RQ-747 VirusScanFUM for FVT2】 2018.05.23 Sky.K.Ogawaa Add Start ->.
	m_pcVirusScanObserver = SSTC_ConcreteVirusScanObserver::newInstance( );
	APIC_VirusScan::newInstance()->attachObserver( m_pcVirusScanObserver );
// 【RQ-747 VirusScanFUM for FVT2】 2018.05.23 Sky.K.Ogawa Add End   <-.
}

//************************************************************************************************************************.
//概要　　：全オブザーバー登録解除.
//引数　　：なし.
//戻り値　：なし.
//備考　　：特に無し.
//作成者　：高橋.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
// 2011.12.08 CIT不具合対応[マージ後不具合管理票 No.164].
//  - APIC_FaxObserver::xxx()通知情報     st_DevXX->st_DevXX[TYPE_CommLine2].
// 2012. 5.15 [CQ363516] パフォーマンス改善 Observerで検出可能な情報をポーリング→Observerに変更した対応で追加した関数.
// 2012/09/13, ADS坪井, CQ369866:Android接続中通知追加.
// 2013.10.16 日立JTE 古川 【DS-IGL】 画像ログオプション化 IGLモジュールロード時のみAPIC_ImageLogからのオブザーバ登録解除を行う.
//************************************************************************************************************************.
void SSTC_SystemStatusService::detachAllObserver()
{
	SST_LOG(MFPE_LogLevelTrace);
	
	// オブザーバ登録解除(リモートパネル).
	APIC_PrinterSystem::newInstance()->detachPrinterSystemObserver( m_pcPrinterSystemObserver );
	APIC_RemotePanel::newInstance()->detachObserver( m_pcRemotePanelObserver );
	
	APIC_RMP::newInstance()->detachObserver( m_pcRemotePanelRMP_Observer );
	APIC_RMP_KeyOff::newInstance()->detachObserver( m_pcRemotePanelRMP_KeyOff_Observer );
	APIC_EXC_APPDispReq::newInstance()->detachObserver( m_pcRemotePanelEXC_APPDispReqObserver );
	APIC_EXC_RPnlCloseRequest::newInstance()->detachObserver( m_pcRemotePanelEXC_RPnlCloseRequest );
	APIC_EXC_RPnlCommunicationNotice::newInstance()->detachObserver( m_pcRemotePanelEXC_RPnlCommunicationNoticeObserver );
	APIC_EXC_RPnlFunctionNotice::newInstance()->detachObserver( m_pcRemotePanelEXC_RPnlFunctionNoticeObserver );
	APIC_EXC_RPnlStatusNotice::newInstance()->detachObserver( m_pcRemotePanelEXC_RPnlStatusNoticeObserver );
	APIC_TabletIF::newInstance()->detachObserver( m_pcTabletIFObserver );

	// オブザーバ登録解除(アドレス).
	APIC_MyAddress::newInstance()->detachObserver( m_pcAddressNumberObserver );
	
	// オブザーバ登録解除(サービス).
	APIC_ScannerExternalIF::newInstance()->detachScannerExternalIFObserver();
	APIC_JLG::newInstance()->detachObserver( m_pcJLGObserver );
	APIC_ApplicationFileIo::newInstance()->detachObserver( m_pcApplicationFileIDObserver );
	APIC_OperationSetting::newInstance()->detachObserver( m_pcOperationSettingObserver );
	APIC_PrintJobSetting::newInstance()->detachObserver( m_pcPrintJobSettingObserver );
	
	// オブザーバ登録解除(システム).
	APIC_SystemStatus::newInstance()->detachObserver( m_pcSystemStatusObserver );
	APIC_EnablerSetting::newInstance()->detachObserver( m_pcEnablerSettingObserver );
	APIC_SystemInfo::newInstance()->detachObserver( m_pcSystemInfoObserver );
	APIC_SystemSetting::newInstance()->detachObserver( m_pcSystemSettingObserver );
	APIC_BMLinkSStatus::newInstance()->detachObserver( m_pcBMLinkSStatusObserver );
	APIC_BMLinkSSettings::newInstance()->detachObserver( m_pcBMLinkSSettingsObserver );
	APIC_CopyProtectInfoSetting::newInstance()->detachObserver( m_pcCopyProtectInfoSettingObserver );
	APIC_DatabaseStatus::newInstance()->detachObserver( m_pcDatabaseStatusObserver );
	APIC_HddInfo::newInstance()->detachObserver( m_pcHddInfoObserver );
	
	// 新API対応：2011.05.26 <ADD> Start T.Takahashi.
#ifdef PLUGIN_OBS_ENABLE
	APIC_PowerServiceIF::newInstance()->detachObserver( m_pcPowerServiceObserver );
	CNVC_ImageFormatterIf::newInstance()->detachObserver( m_pcImageFormatterObserver );
	NETC_NetPrintRxPluginAdapter::newInstance()->detachObserver( m_pcNetPrintRxPluginAdapterObserver );
	NETC_NetTxPluginAdapter::newInstance()->detachObserver( m_pcNetTxPluginAdapterObserver );
	NETC_NetFaxRxPluginAdapter::newInstance()->detachObserver( m_pcNetFaxRxPluginAdapterObserver );
#endif
	// 新API対応：2011.05.26 <ADD> End T.Takahashi.
// 認証プラグイン対応_Start. ADS 2011.11.01.
	AUTC_AuthPluginIF::newInstance()->detachObserver( m_pcAuthPluginAdapterObserver );
// 認証プラグイン対応_End.   ADS 2011.11.01.
	
	APIC_DebugLogSystemSetting::newInstance()->detachObserver( m_pcDebugLogSystemSettingObserver );
	APIC_UserAuthentication::newInstance()->detachObserver( m_pcUserAuthenticationObserver );

// Eagle:制御構成変更No.104_テストパターン印刷のターミナルコマンド対応(生産対応）start.
	APIC_SystemInfo::newInstance()->detachObserver( m_pcSystemInfoObserver3 );
// Eagle:制御構成変更No.104_テストパターン印刷のターミナルコマンド対応(生産対応）end.

	// オブザーバ登録解除(スキャナー).
	APIC_WSDSetting::newInstance()->detachErrorNoticeObserver( m_pcWSDSettingObserver );
	APIC_ScanerSetting::newInstance()->detachObserver( m_pcOpticalAdjustObserver );
	APIC_ScanJobSetting::newInstance()->detachObserver( m_pcScanJobSettingObserver );
	APIC_ScannerServiceInfo::newInstance()->detachScannerServiceInfoObserver( m_pcScannerServiceInfoObserver );
	APIC_ScannerServiceSetting::newInstance()->detachScannerServiceSettingObserver( m_pcScannerServiceSettingObserver );
	APIC_ScannerStatus::newInstance()->detachScannerStatusObserver( m_pcScannerStatusObserver );
	APIC_ScannerSystem::newInstance()->detachScannerSystemObserver( m_pcScannerSystemObserver );
	
	// オブザーバ登録解除(通信).
	APIC_RdsSetting::newInstance()->detachObserver( m_pcRdsSettingObserver );
	APIC_Rds::newInstance()->detachObserver( m_pcRdsObserver );
	APIC_LogRdsJnlDataModem::newInstance()->detachObserver( m_pcLogRdsJnlDataModemObserver );
	APIC_LogRdsJnlEmail::newInstance()->detachObserver( m_pcLogRdsJnlEmailObserver );
	
	// オブザーバ登録解除(デバイス_プリンタ).
	APIC_PrinterInfo::newInstance()->detachPrinterInfoObserver( m_pcPrinterInfoObserver );
	APIC_PrinterPortSetting::newInstance()->detachObserver( m_pcPrinterPortSettingObserver );
	APIC_PrinterServiceInfo::newInstance()->detachPrinterServiceInfoObserver( m_pcPrinterServiceInfoObserver );
	APIC_PrinterServiceSetting::newInstance()->detachPrinterServiceSettingObserver( m_pcPrinterServiceSettingObserver );
	APIC_PrinterSetting::newInstance()->detachObserver( m_pcPrinterRegulationObserver );
	APIC_PrinterStatus::newInstance()->detachPrinterStatusObserver( m_pcPrinterStatusObserver );
	APIC_PrinterWarning::newInstance()->detachPrinterWarningObserver( m_pcPrinterWarningObserver );
	
	// オブザーバ登録解除(デバイス).
	for ( TYPE_CommLine i = (TYPE_CommLine)0, e_Line = TYPE_CommLine1; i < TYPE_CommLine2; i++, e_Line++ ) {
		APIC_ObsvrMgr::instance( e_Line )->detachObserver( m_pcFaxObserver[(ushort)i] );
	}
	APIC_Power::newInstance()->detachObserver( m_pcPowerObserver );
	APIC_PmCnt::newInstance()->detachObserver( m_pcPmCntObserver );
	APIC_UsbMemDeviceInfo::newInstance()->detachObserver( m_pcUsbMemDeviceInfoObserver );
	APIC_LifeCnt::newInstance()->detachObserver( m_pcLifeCntObserver );
	APIC_AccumulateCoverageCnt::newInstance()->detachObserver( m_pcAccumulateCoverageCntObserver );
	APIC_AdfCnt::newInstance()->detachObserver( m_pcAdfCntObserver );
	APIC_CompressRateCnt::newInstance()->detachObserver( m_pcCompressRateCntObserver );
	APIC_ConsumableInfo::newInstance()->detachObserver( m_pcConsumableInfoObserver );
	APIC_CopyJobSetting::newInstance()->detachObserver( m_pcCopyJobSettingObserver );
	APIC_CopyKitCounter::newInstance()->detachObserver( m_pcCopyKitCounterObserver );
	APIC_CopyModeCnt::newInstance()->detachObserver( m_pcCopyModeCntObserver );
	APIC_CoverageCnt::newInstance()->detachObserver( m_pcCoverageCntObserver );
	APIC_CSRCCoverageCnt::newInstance()->detachObserver( m_pcCSRCCoverageCntObserver );
	APIC_DustCnt::newInstance()->detachObserver( m_pcDustCntObserver );
	APIC_FaxBoxSetting::newInstance()->detachObserver( m_pcFaxBoxSettingObserver );
	APIC_FaxCnt::newInstance()->detachObserver( m_pcFaxCntObserver );
	APIC_FaxErrorCnt::newInstance()->detachObserver( m_pcFaxErrorCntObserver );
	APIC_FaxGfpSetting::newInstance()->detachObserver( m_pcFaxGfpSettingObserver );
	APIC_FaxGhpSetting::newInstance()->detachObserver( m_pcFaxGhpSettingObserver );
	APIC_FaxGswSetting::newInstance()->detachObserver( m_pcFaxGswSettingObserver );
	APIC_FaxJobSetting::newInstance()->detachObserver( m_pcFaxJobSettingObserver );
	APIC_FaxJobSetting2::newInstance()->detachObserver( m_pcFaxJobSetting2Observer );
	APIC_FeedTray::newInstance()->detachFeedTrayObserver( m_pcFeedTrayObserver );
	APIC_HighRankCoverageCnt::newInstance()->detachObserver( m_pcHighRankCoverageCntObserver );
	APIC_IGCdeviceList::newInstance()->detachObserver( m_pcIGCdeviceListObserver );
	APIC_JamCnt::newInstance()->detachObserver( m_pcJamCntObserver );
	APIC_JamDailyCnt::newInstance()->detachObserver( m_pcJamDailyCntObserver );
	APIC_JamTotalCnt::newInstance()->detachObserver( m_pcJamTotalCntObserver );
	APIC_JimonPatternSetting::newInstance()->detachObserver( m_pcJimonPatternSettingObserver );
	APIC_LEDStatusSetting::newInstance()->detachObserver( m_pcLEDStatusSettingObserver );
	APIC_LogJam::newInstance()->detachObserver( m_pcLogJamObserver );
	APIC_LogTrouble::newInstance()->detachObserver( m_pcLogTroubleObserver );
	APIC_MachineSetting::newInstance()->detachObserver( m_pcMachineSettingObserver );
	APIC_MechaCounter::newInstance()->detachObserver( m_pcMechaCounterObserver );
	APIC_MemoryInfo::newInstance()->detachObserver( m_pcMemoryInfoObserver );
	APIC_MobileSetting::newInstance()->detachObserver( m_pcMobileSettingObserver );
	APIC_OptionPartsCnt::newInstance()->detachObserver( m_pcOptionPartsCntObserver );
	APIC_Original::newInstance()->detachOriginalObserver( m_pcOriginalObserver );
	APIC_OutputTray::newInstance()->detachOutputTrayObserver( m_pcOutputTrayObserver );
	APIC_PaperSizeCntCSRC::newInstance()->detachObserver( m_pcPaperSizeCntCSRCObserver );
	APIC_PaperSizeCnt::newInstance()->detachObserver( m_pcPaperSizeCntObserver );
	APIC_PartsCnt::newInstance()->detachObserver( m_pcPartsCntObserver );
	APIC_Prefix::newInstance()->detachObserver( m_pcPrefixObserver );
	APIC_PreviewSetting::newInstance()->detachObserver( m_pcPreviewSettingObserver );
	APIC_ReuseCnt::newInstance()->detachObserver( m_pcReuseCntObserver );
	APIC_SoftDipSw::newInstance()->detachObserver( m_pcSoftDipSwObserver );
	APIC_StampSetting::newInstance()->detachObserver( m_pcStampSettingObserver );
	APIC_StatisticsPage::newInstance()->detachObserver( m_pcStatisticsPageObserver );
	APIC_TonerChangeCnt::newInstance()->detachObserver( m_pcTonerChangeCntObserver );
	APIC_TotalCnt::newInstance()->detachObserver( m_pcTotalCntObserver );
	APIC_TroubleCnt::newInstance()->detachObserver( m_pcTroubleCntObserver );
	APIC_TroubleTotalCnt::newInstance()->detachObserver( m_pcTroubleTotalCntObserver );
	APIC_UsbMemSetting::newInstance()->detachObserver( m_pcUsbMemSettingObserver );
	APIC_WarningCnt::newInstance()->detachObserver( m_pcWarningCntObserver );

	// オブザーバ登録解除(ネットワーク).
	APIC_WebDAVdSetting::newInstance()->detachObserver( m_pcWebDAVTimerResetObserver );
	APIC_SnmpTrapSetting::newInstance()->detachObserver( m_pcSnmpTrapSetObserver );
	APIC_WebDAVServerControl::newInstance()->detachObserver( m_pcWebDAVSvrCtrlObserver );
	APIC_CounterMailSendSetting::newInstance()->detachObserver( m_pcCntMailSndSetObserver );
	APIC_AppleTalkSetting::newInstance()->detachObserver( m_pcAppleTalkSetObserver );
	APIC_BonjourSetting::newInstance()->detachObserver( m_pcBonjourSetObserver );
	APIC_BrowserSetting::newInstance()->detachObserver( m_pcBrowserSetObserver );
	APIC_CertInfoSetting::newInstance()->detachObserver( m_pcCertInfoSetObserver );
	APIC_FtpClientSetting::newInstance()->detachObserver( m_pcFtpClientSetObserver );
	APIC_IEEE802Setting::newInstance()->detachObserver( m_pcIEEE802SetObserver );
	APIC_InternetISW::newInstance()->detachObserver( m_pcInternetISWObserver );
	APIC_IpFilteringSetting::newInstance()->detachObserver( m_pcIpFilteringSetObserver );
	APIC_IppSetting::newInstance()->detachObserver( m_pcIppSetObserver );
	APIC_IPsecSetting::newInstance()->detachObserver( m_pcIPsecSetObserver );
	APIC_IPv6Setting::newInstance()->detachObserver( m_pcIPv6SetObserver );
	APIC_LdapSetting::newInstance()->detachObserver( m_pcLdapSetObserver );
	APIC_MySpoolServerSetting::newInstance()->detachObserver( m_pcMySpoolSvrSetObserver );
	APIC_NetWareSetting::newInstance()->detachObserver( m_pcNetWareSetObserver );
	APIC_NetworkDevice::newInstance()->detachObserver( m_pcNWDevObserver );
	APIC_NetworkFaxSetting::newInstance()->detachObserver( m_pcNWFaxSetObserver );
	APIC_NtpSetting::newInstance()->detachObserver( m_pcNtpSetObserver );
	APIC_PswcMibSetting::newInstance()->detachObserver( m_pcPswcMibSetObserver );
	APIC_SlpSetting::newInstance()->detachObserver( m_pcSlpSetObserver );
	APIC_SmbPrintSetting::newInstance()->detachObserver( m_pcSmbPrtSetObserver );
	APIC_SmbSetting::newInstance()->detachObserver( m_pcSmbSetObserver );
	APIC_SmimeCertInfo::newInstance()->detachObserver( m_pcSmimeCertObserver );
	APIC_SmimeSetting::newInstance()->detachObserver( m_pcSmimeSetObserver );
	APIC_SnmpSetting::newInstance()->detachObserver( m_pcSnmpSetObserver );
	APIC_SNMPv3PenaltyCnt::newInstance()->detachObserver( m_pcSNMPv3PenaltyCntObserver );
	APIC_SSDPSettings::newInstance()->detachObserver( m_pcSSDPSetObserver );
	APIC_SslSetting::newInstance()->detachObserver( m_pcSslSetObserver );
	APIC_TcpIpSetting::newInstance()->detachObserver( m_pcTcpIpSetObserver );
	APIC_TcpSocketSetting::newInstance()->detachObserver( m_pcTcpSockSetObserver );
	APIC_WebDAVdPenaltyCnt::newInstance()->detachObserver( m_pcWebDAVPenaltyCntObserver );
	APIC_WebDAVdSetting::newInstance()->detachObserver( m_pcWebDAVSvrSetObserver );
	APIC_WebDavSetting::newInstance()->detachObserver( m_pcWebDAVSetObserver );
	APIC_WirelessSetting::newInstance()->detachObserver( m_pcWirelessSetObserver );
	APIC_MailReceiveSetting::newInstance()->detachObserver( m_pcMailPOPRcvSetObserver );
	APIC_MailRecSmtpSetting::newInstance()->detachObserver( m_pcMailRecSmtpSetObserver );
	APIC_MailSendSetting::newInstance()->detachObserver( m_pcMailSndSetObserver );
	APIC_SmtpRecvSetting::newInstance()->detachObserver( m_pcSmtpRcvSetObserver );
	APIC_SmtpSendSetting::newInstance()->detachObserver( m_pcSmtpSndSetObserver );
	
	// オブザーバ登録解除(パネル情報).
	APIC_ApplicationKeySetting::newInstance()->detachObserver( m_pcAppKeySetObserver );
	APIC_DefaultPanelSetting::newInstance()->detachObserver( m_pcDefPanelSetObserver );
	APIC_FaxPanelSetting::newInstance()->detachObserver( m_pcFaxPanelSetObserver );

	// オブザーバ登録解除(その他).
	APIC_SearchablePDFSetting::newInstance()->detachObserver( m_pcPDFSettingObserver );
	//2013.10.16 日立JTE 古川 【DS-IGL】 画像ログオプション化 IGLモジュールロード時のみデタッチを行う.
	if( True == SYSF_getAppOptionStatus( TYPE_AppOptionIGL ) ){
		APIC_ImageLogControll::newInstance()->detachObserver( m_pcImageLogObserver );
	}
	APIC_FaxListReportSetting::newInstance()->detachObserver( m_pcFaxReportObserver );
	APIC_LicenseInfoSetting::newInstance()->detachObserver( m_pcLicenseInfoObserver );
	APIC_OpenApiSetting::newInstance()->detachObserver( m_pcOpenApiSettingObserver );
}

/**
* \note オブザーバで取得するデータの初期化処理.
*/
//*****************************************************************************.
//概要　　：オブザーバで取得するデータの初期化処理.
//引数　　：なし.
//戻り値　：なし.
//作成者　：ADS.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
// 2012. 1.31 [CQ359780] getProgJobXXXStatusをポーリング→Observerに変更.
//*****************************************************************************.
void SSTC_SystemStatusService::initObserverData()
{
	m_vChangedInfo.clear();	// 変化情報リスト初期化.

	procGetObserverData();	// 情報取得.

	TYPE_NTF_GroupID  e_writeGroupID = TYPE_NTF_GroupID_End;	// 共有メモリ書き込み済みGrpID.
	for( std::vector<SSTS_ChangedInfo>::iterator it = m_vChangedInfo.begin(); it != m_vChangedInfo.end(); ++it ){
		TYPE_NTF_GroupID  e_GroupID    = TYPE_NTF_GroupID_End;
		void 			 *pv_GroupData = NULL;
		if( SSTC_SystemStatusService::getGroupInfo( (it)->e_DataID, &e_GroupID, &pv_GroupData ) == False ){
			SST_ERR_LOG("getGroupInfo() == False.\n");	// グループ情報取得 該当なしの場合はエラーログ出力.
			continue;
		}
		if ( e_GroupID != e_writeGroupID ){
			m_pcInstance->lock();
			m_pcShMemMan->writeData( e_GroupID, pv_GroupData );	// 共有メモリ更新.
			m_pcInstance->unlock();
			e_writeGroupID = e_GroupID;	// 共有メモリ書き込み済みGrpID設定.
		}
	}

	m_vChangedInfo.clear();	// 変化情報リストをクリア.
}

//*****************************************************************************.
//概要　　：オブザーバで取得するデータを更新する.
//引数　　：なし.
//戻り値　：なし.
//作成者　：ADS.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//*****************************************************************************.
void SSTC_SystemStatusService::updateObserverData()
{
	m_vChangedInfo.clear();	// 変化情報リスト初期化.

	procGetObserverData();	// 情報取得.

	TYPE_NTF_GroupID  e_writeGroupID    = TYPE_NTF_GroupID_End;
	for( std::vector<SSTS_ChangedInfo>::iterator it = m_vChangedInfo.begin(); it != m_vChangedInfo.end(); ++it ){
		
		// グループ情報取得 該当なしの場合はエラーログ出力.
		TYPE_NTF_GroupID  e_GroupID    = TYPE_NTF_GroupID_End;
		void 			 *pv_GroupData = NULL;
		
		if( SSTC_SystemStatusService::getGroupInfo( (it)->e_DataID, &e_GroupID, &pv_GroupData ) == False ){
			// エラーログ出力.
			SST_ERR_LOG("getGroupInfo() == False.\n");
			continue;
		}
		if( e_GroupID != e_writeGroupID ){
			// 共有メモリ更新.
			m_pcInstance->lock();
			m_pcShMemMan->writeData( e_GroupID, pv_GroupData );
			m_pcInstance->unlock();
			e_writeGroupID = e_GroupID;
		}
	}

	if( m_bStartNotifyFlag == True ){
		for( std::vector<SSTS_ChangedInfo>::iterator it = m_vChangedInfo.begin(); it != m_vChangedInfo.end(); ++it ){
			// データ変化通知.
			notifyChangedData( (it)->e_DataID );
		}
	}

	m_vChangedInfo.clear();	// 変化情報リストをクリア.
}

//*****************************************************************************.
//概要　　：オブザーバで取得するデータの取得処理.
//引数　　：なし.
//戻り値　：なし.
//作成者　：ADS.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
// 2011.12.15  データなしObserverのデータ追加対応.
// 2012. 5.15 [CQ363516] パフォーマンス改善 Observerで検出可能な情報をポーリング→Observerに変更した対応で追加した関数.
//*****************************************************************************.
void SSTC_SystemStatusService::procGetObserverData()
{
	SST_LOG(MFPE_LogLevelAll);

	/**
	* \note GrpIDの順番に登録必要.
	*/
	// リモートパネル情報:TYPE_NTF_GroupID_RemotePanel(0x0001).
	getRemotePanelObserverData();

	// サービス情報:TYPE_NTF_GroupID_Service(0x0003).
	getOperationSettingObserverData();		// APIC_OperationSettingObserver::dataChanged().

	// システム情報:TYPE_NTF_GroupID_System(0x0004).
	getSystemObserverData();
	getEnablerSettingObserverData();		// APIC_EnablerSettingObserver::dataChanged().
	getSystemInfoObserverData();			// APIC_SystemInfoObserver::dataChanged().
	getSystemSettingObserverData();			// APIC_SystemSettingObserver::dataChanged().
	getSystemStatusObserverData();			// APIC_SystemStatusObserver::dataChanged().
	getDebugLogSystemSettingObserverData();	// APIC_DebugLogSystemSettingObserver::dataChanged().
	getUserAuthenticationObserverData();	// APIC_UserAuthenticationObserver::dataChanged().
// 2016/07/01 2Sys-Sky西村 RQ-436対応 start.
	getIwsObserverData();					// APIC_IwsObserver::dataChanged().
// 2016/07/01 2Sys-Sky西村 RQ-436対応 end.
// 2018/09/29 [RQ-674] SCD2-motohashi ジョブログ関連の警告取得処理は定期起床処理で行うように変更.
//// 2018/08/30 [RQ-674] SCD2-motohashi ログ遅延状態を追加.
//	getJlgObserverData();					// APIC_JLGObserver::JobLogDataFULL().

	// スキャナ情報:TYPE_NTF_GroupID_Scanner(0x0005).
	getScannerObserverData();
	getScanJobSettingObserverData();		// APIC_ScanJobSettingObserver::dataChanged().

	// 通信情報:TYPE_NTF_GroupID_Commutication(0x0006).
	getCommunicateObserverData();
	
	// プリンタデバイス情報:TYPE_NTF_GroupID_PrinterDevice(0x0007).
	getPrinterDeviceObserverData();
	getPrinterSettingObserverData();		// APIC_PrinterSettingObserver::dataChanged().
	getPrinterStatusObserverData();			// APIC_PrinterStatusObserver::dataChanged().

	// デバイス情報:TYPE_NTF_GroupID_Device(0x0008).
	getDeviceObserverData();
	getUsbWarningObserverData();			// APIC_UsbMemDeviceInfoObserver::warningChanged().
	getCopyJobSettingObserverData();		// APIC_CopyJobSettingObserver::dataChanged().
	getFaxJobSettingObserverData();			// APIC_FaxJobSettingObserver::dataChanged().
	getMachineSettingObserverData();		// APIC_MachineSettingObserver::dataChanged().
	getSoftDipSwObserverData();				// APIC_SoftDipSwObserver::dataChanged().
	getSystemInfoObserverDeviceInfo();		// APIC_SystemInfoObserver::dataChanged() デバイス情報.

	// ネットワーク情報:TYPE_NTF_GroupID_Network(0x0009).
	getNetworkObserverData();
	getSmtpSendSettingObserverData();		// APIC_SmtpSendSettingObserver::dataChanged().

	// その他情報:TYPE_NTF_GroupID_Other(0x000B).
	getOtherObserverData();
	getImageLogObserverData();				// APIC_ImageLogObserver::notifyImageLogWarning().

}

//*****************************************************************************.
//概要　　：オブザーバで取得するデータの取得処理.
//引数　　：なし.
//戻り値　：なし.
//作成者　：ADS.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//*****************************************************************************.
void SSTC_SystemStatusService::getRemotePanelObserverData()
{
	SST_LOG(MFPE_LogLevelAll);
	SSTS_ChangedInfo st_ChangedInfo;
	SYSF_bzero((char*)&st_ChangedInfo, sizeof(st_ChangedInfo));
	
	// TYPE_NTF_DataID_RPL_1	プリンタシステム情報.
	SSTC_ConcretePrinterSystemObserver::getPrinterSystemData(m_stSysStatInfo.st_RemotePanelInfoParam.st_Rpl1);
	st_ChangedInfo.e_DataID = TYPE_NTF_DataID_RPL_1;
	st_ChangedInfo.p_Value  = &(m_stSysStatInfo.st_RemotePanelInfoParam.st_Rpl1);
	// 2021/10/22 APF OP_BTS-35403 start
	//m_vChangedInfo.push_back(st_ChangedInfo);
	AddChangedInfo(&st_ChangedInfo);
	// 2021/10/22 APF OP_BTS-35403 end

	// TYPE_NTF_DataID_RPL_2	リモートパネル情報.
	SSTC_ConcreteRemotePanelObserver::getRemotePanelStatusFiery(m_stSysStatInfo.st_RemotePanelInfoParam.st_Rpl2);
	st_ChangedInfo.e_DataID = TYPE_NTF_DataID_RPL_2;
	st_ChangedInfo.p_Value  = &(m_stSysStatInfo.st_RemotePanelInfoParam.st_Rpl2);
	// 2021/10/22 APF OP_BTS-35403 start
	//m_vChangedInfo.push_back(st_ChangedInfo);
	AddChangedInfo(&st_ChangedInfo);
	// 2021/10/22 APF OP_BTS-35403 end

	// TYPE_NTF_DataID_RPL_3	リモートパネル情報.
	SSTC_ConcreteRemotePanelEXC_APPDispReqObserver::getRemotePanelStatusEXC_APPDispReq(m_stSysStatInfo.st_RemotePanelInfoParam.st_Rpl3);
	st_ChangedInfo.e_DataID = TYPE_NTF_DataID_RPL_3;
	st_ChangedInfo.p_Value  = &(m_stSysStatInfo.st_RemotePanelInfoParam.st_Rpl3);
	// 2021/10/22 APF OP_BTS-35403 start
	//m_vChangedInfo.push_back(st_ChangedInfo);
	AddChangedInfo(&st_ChangedInfo);
	// 2021/10/22 APF OP_BTS-35403 end

	// TYPE_NTF_DataID_RPL_4	リモートパネル情報.
	SSTC_ConcreteRemotePanelEXC_RPnlCommunicationNoticeObserver::getRemotePanelStatusEXC_RPnlCommunicationNotice(m_stSysStatInfo.st_RemotePanelInfoParam.st_Rpl4);
	st_ChangedInfo.e_DataID = TYPE_NTF_DataID_RPL_4;
	st_ChangedInfo.p_Value  = &(m_stSysStatInfo.st_RemotePanelInfoParam.st_Rpl4);
	// 2021/10/22 APF OP_BTS-35403 start
	//m_vChangedInfo.push_back(st_ChangedInfo);
	AddChangedInfo(&st_ChangedInfo);
	// 2021/10/22 APF OP_BTS-35403 end

	// TYPE_NTF_DataID_RPL_5	リモートパネル情報.
	SSTC_ConcreteRemotePanelEXC_RPnlFunctionNoticeObserver::getRemotePanelStatusEXC_RPnlFunctionNotice(m_stSysStatInfo.st_RemotePanelInfoParam.st_Rpl5);
	st_ChangedInfo.e_DataID = TYPE_NTF_DataID_RPL_5;
	st_ChangedInfo.p_Value  = &(m_stSysStatInfo.st_RemotePanelInfoParam.st_Rpl5);
	// 2021/10/22 APF OP_BTS-35403 start
	//m_vChangedInfo.push_back(st_ChangedInfo);
	AddChangedInfo(&st_ChangedInfo);
	// 2021/10/22 APF OP_BTS-35403 end

	// TYPE_NTF_DataID_RPL_6	リモートパネル情報.
	SSTC_ConcreteRemotePanelEXC_RPnlStatusNoticeObserver::getRemotePanelStatusEXC_RPnlStatusNotice(m_stSysStatInfo.st_RemotePanelInfoParam.st_Rpl6);
	st_ChangedInfo.e_DataID = TYPE_NTF_DataID_RPL_6;
	st_ChangedInfo.p_Value  = &(m_stSysStatInfo.st_RemotePanelInfoParam.st_Rpl6);
	// 2021/10/22 APF OP_BTS-35403 start
	//m_vChangedInfo.push_back(st_ChangedInfo);
	AddChangedInfo(&st_ChangedInfo);
	// 2021/10/22 APF OP_BTS-35403 end

	// TYPE_NTF_DataID_RPL_8	リモートパネル情報.
	SSTC_ConcreteRemotePanelRMP_Observer::getRemotePanelStatusRMP_Message(m_stSysStatInfo.st_RemotePanelInfoParam.st_Rpl8);
	st_ChangedInfo.e_DataID = TYPE_NTF_DataID_RPL_8;
	st_ChangedInfo.p_Value  = &(m_stSysStatInfo.st_RemotePanelInfoParam.st_Rpl8);
	// 2021/10/22 APF OP_BTS-35403 start
	//m_vChangedInfo.push_back(st_ChangedInfo);
	AddChangedInfo(&st_ChangedInfo);
	// 2021/10/22 APF OP_BTS-35403 end

	// TYPE_NTF_DataID_RPL_9	リモートパネル情報.
	SSTC_ConcreteRemotePanelRMP_KeyOff_Observer::getRemotePanelStatusRMP_KeyOffMessage(m_stSysStatInfo.st_RemotePanelInfoParam.st_Rpl9);
	st_ChangedInfo.e_DataID = TYPE_NTF_DataID_RPL_9;
	st_ChangedInfo.p_Value  = &(m_stSysStatInfo.st_RemotePanelInfoParam.st_Rpl9);
	// 2021/10/22 APF OP_BTS-35403 start
	//m_vChangedInfo.push_back(st_ChangedInfo);
	AddChangedInfo(&st_ChangedInfo);
	// 2021/10/22 APF OP_BTS-35403 end

}


//*****************************************************************************.
//概要　　：オブザーバで取得するデータの取得処理.
//引数　　：なし.
//戻り値　：なし.
//作成者　：ADS.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//*****************************************************************************.
void SSTC_SystemStatusService::getSystemObserverData()
{
	SST_LOG(MFPE_LogLevelAll);
	SSTS_ChangedInfo st_ChangedInfo;
	SYSF_bzero((char*)&st_ChangedInfo, sizeof(st_ChangedInfo));
	
	// TYPE_NTF_DataID_SYS_3	FATAL情報.
	SSTC_ConcreteSystemStatusObserver::getFatalData(m_stSysStatInfo.st_SystemInfoParam.st_Sys3);
	st_ChangedInfo.e_DataID = TYPE_NTF_DataID_SYS_3;
	st_ChangedInfo.p_Value  = &(m_stSysStatInfo.st_SystemInfoParam.st_Sys3);
	// 2021/10/22 APF OP_BTS-35403 start
	//m_vChangedInfo.push_back(st_ChangedInfo);
	AddChangedInfo(&st_ChangedInfo);
	// 2021/10/22 APF OP_BTS-35403 end

	// APIC_BMLinkSStatusObserver関連.
	// TYPE_NTF_DataID_SYS_9	BMLinkS状態のデータ変化通知.
	SSTC_ConcreteBMLinkSStatusObserver::getPrintServiceStatus(m_stSysStatInfo.st_SystemInfoParam.e_Sys9);
	st_ChangedInfo.e_DataID = TYPE_NTF_DataID_SYS_9;
	st_ChangedInfo.p_Value  = &(m_stSysStatInfo.st_SystemInfoParam.e_Sys9);
	// 2021/10/22 APF OP_BTS-35403 start
	//m_vChangedInfo.push_back(st_ChangedInfo);
	AddChangedInfo(&st_ChangedInfo);
	// 2021/10/22 APF OP_BTS-35403 end

	// TYPE_NTF_DataID_SYS_13	HDD情報.
	SSTC_ConcreteHddInfoObserver::getHddInfoData(m_stSysStatInfo.st_SystemInfoParam.st_Sys13);
	st_ChangedInfo.e_DataID = TYPE_NTF_DataID_SYS_13;
	st_ChangedInfo.p_Value  = &(m_stSysStatInfo.st_SystemInfoParam.st_Sys13);
	// 2021/10/22 APF OP_BTS-35403 start
	//m_vChangedInfo.push_back(st_ChangedInfo);
	AddChangedInfo(&st_ChangedInfo);
	// 2021/10/22 APF OP_BTS-35403 end
}

//*****************************************************************************.
//概要　　：オブザーバで取得するデータの取得処理.
//引数　　：なし.
//戻り値　：なし.
//作成者　：ADS.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//*****************************************************************************.
void SSTC_SystemStatusService::getScannerObserverData()
{
	SST_LOG(MFPE_LogLevelAll);
	SSTS_ChangedInfo st_ChangedInfo;
	SYSF_bzero((char*)&st_ChangedInfo, sizeof(st_ChangedInfo));
	
	// TYPE_NTF_DataID_SCN_4	ScannerServiceInfo.
	SSTC_ConcreteScannerServiceInfoObserver::getScannerServiceInfoData(m_stSysStatInfo.st_ScannerInfoParam.st_Scn4);
	st_ChangedInfo.e_DataID = TYPE_NTF_DataID_SCN_4;
	st_ChangedInfo.p_Value  = &(m_stSysStatInfo.st_ScannerInfoParam.st_Scn4);
	// 2021/10/22 APF OP_BTS-35403 start
	//m_vChangedInfo.push_back(st_ChangedInfo);
	AddChangedInfo(&st_ChangedInfo);
	// 2021/10/22 APF OP_BTS-35403 end

	// TYPE_NTF_DataID_SCN_5	ScannerServiceSetting.
	SSTC_ConcreteScannerServiceSettingObserver::getScannerServiceSettingData(m_stSysStatInfo.st_ScannerInfoParam.st_Scn5);
	st_ChangedInfo.e_DataID = TYPE_NTF_DataID_SCN_5;
	st_ChangedInfo.p_Value  = &(m_stSysStatInfo.st_ScannerInfoParam.st_Scn5);
	// 2021/10/22 APF OP_BTS-35403 start
	//m_vChangedInfo.push_back(st_ChangedInfo);
	AddChangedInfo(&st_ChangedInfo);
	// 2021/10/22 APF OP_BTS-35403 end

	// TYPE_NTF_DataID_SCN_6	ScannerStatus.
	SSTC_ConcreteScannerStatusObserver::getScannerStatusData(m_stSysStatInfo.st_ScannerInfoParam.st_Scn6);
	st_ChangedInfo.e_DataID = TYPE_NTF_DataID_SCN_6;
	st_ChangedInfo.p_Value  = &(m_stSysStatInfo.st_ScannerInfoParam.st_Scn6);
	// 2021/10/22 APF OP_BTS-35403 start
	//m_vChangedInfo.push_back(st_ChangedInfo);
	AddChangedInfo(&st_ChangedInfo);
	// 2021/10/22 APF OP_BTS-35403 end

	// TYPE_NTF_DataID_SCN_7	ScannerSystem.
	SSTC_ConcreteScannerSystemObserver::getScannerSystemData(m_stSysStatInfo.st_ScannerInfoParam.st_Scn7);
	st_ChangedInfo.e_DataID = TYPE_NTF_DataID_SCN_7;
	st_ChangedInfo.p_Value  = &(m_stSysStatInfo.st_ScannerInfoParam.st_Scn7);
	// 2021/10/22 APF OP_BTS-35403 start
	//m_vChangedInfo.push_back(st_ChangedInfo);
	AddChangedInfo(&st_ChangedInfo);
	// 2021/10/22 APF OP_BTS-35403 end
}

//*****************************************************************************.
//概要　　：オブザーバで取得するデータの取得処理.
//引数　　：なし.
//戻り値　：なし.
//作成者　：ADS.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//*****************************************************************************.
void SSTC_SystemStatusService::getCommunicateObserverData()
{
	SST_LOG(MFPE_LogLevelAll);
	SSTS_ChangedInfo st_ChangedInfo;
	SYSF_bzero((char*)&st_ChangedInfo, sizeof(st_ChangedInfo));
	
	// TYPE_NTF_DataID_COM_5	RDS情報.
	SSTC_ConcreteRdsObserver::getRdsData(m_stSysStatInfo.st_CommunicateInfoParam.st_Com5);
	st_ChangedInfo.e_DataID = TYPE_NTF_DataID_COM_5;
	st_ChangedInfo.p_Value  = &(m_stSysStatInfo.st_CommunicateInfoParam.st_Com5);
	// 2021/10/22 APF OP_BTS-35403 start
	//m_vChangedInfo.push_back(st_ChangedInfo);
	AddChangedInfo(&st_ChangedInfo);
	// 2021/10/22 APF OP_BTS-35403 end
}

//*****************************************************************************.
//概要　　：オブザーバで取得するデータの取得処理.
//引数　　：なし.
//戻り値　：なし.
//作成者　：ADS.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//*****************************************************************************.
void SSTC_SystemStatusService::getPrinterDeviceObserverData()
{
	SST_LOG(MFPE_LogLevelAll);
	SSTS_ChangedInfo st_ChangedInfo;
	SYSF_bzero((char*)&st_ChangedInfo, sizeof(st_ChangedInfo));
	
	// TYPE_NTF_DataID_DEV_PRT_109	PrinterServiceInfo.
	SSTC_ConcretePrinterServiceInfoObserver::getPrinterServiceInfoData(m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt109);
	st_ChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_109;
	st_ChangedInfo.p_Value  = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt109);
	// 2021/10/22 APF OP_BTS-35403 start
	//m_vChangedInfo.push_back(st_ChangedInfo);
	AddChangedInfo(&st_ChangedInfo);
	// 2021/10/22 APF OP_BTS-35403 end

	// TYPE_NTF_DataID_DEV_PRT_110	PrinterServiceSetting.
	SSTC_ConcretePrinterServiceSettingObserver::getPrinterServiceSettingData(m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt110);
	st_ChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_110;
	st_ChangedInfo.p_Value  = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt110);
	// 2021/10/22 APF OP_BTS-35403 start
	//m_vChangedInfo.push_back(st_ChangedInfo);
	AddChangedInfo(&st_ChangedInfo);
	// 2021/10/22 APF OP_BTS-35403 end

}

//*****************************************************************************.
//概要　　：オブザーバで取得するデータの取得処理.
//引数　　：なし.
//戻り値　：なし.
//作成者　：ADS.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//*****************************************************************************.
void SSTC_SystemStatusService::getDeviceObserverData()
{
	SST_LOG(MFPE_LogLevelAll);
	SSTS_ChangedInfo st_ChangedInfo;
	SYSF_bzero((char*)&st_ChangedInfo, sizeof(st_ChangedInfo));
	
	//.
	// APIC_FaxObserver関連.
	//.
	// TYPE_NTF_DataID_DEV_1	CallerID通知.
	SSTC_ConcreteFaxObserver::getCallerID(m_stSysStatInfo.st_DeviceInfoParam.st_Dev1);
	st_ChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_1;
	st_ChangedInfo.p_Value  = &(m_stSysStatInfo.st_DeviceInfoParam.st_Dev1[0]);
	// 2021/10/22 APF OP_BTS-35403 start
	//m_vChangedInfo.push_back(st_ChangedInfo);
	AddChangedInfo(&st_ChangedInfo);
	// 2021/10/22 APF OP_BTS-35403 end

	// TYPE_NTF_DataID_DEV_2	CallerName(発信者名情報)通知.
	SSTC_ConcreteFaxObserver::getCallerName(m_stSysStatInfo.st_DeviceInfoParam.st_Dev2);
	st_ChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_2;
	st_ChangedInfo.p_Value  = &(m_stSysStatInfo.st_DeviceInfoParam.st_Dev2[0]);
	// 2021/10/22 APF OP_BTS-35403 start
	//m_vChangedInfo.push_back(st_ChangedInfo);
	AddChangedInfo(&st_ChangedInfo);
	// 2021/10/22 APF OP_BTS-35403 end

	// TYPE_NTF_DataID_DEV_12	FAX関連エラー通知.
	SSTC_ConcreteFaxObserver::getError(m_stSysStatInfo.st_DeviceInfoParam.e_Dev12);
	st_ChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_12;
	st_ChangedInfo.p_Value  = &(m_stSysStatInfo.st_DeviceInfoParam.e_Dev12[0]);
	// 2021/10/22 APF OP_BTS-35403 start
	//m_vChangedInfo.push_back(st_ChangedInfo);
	AddChangedInfo(&st_ChangedInfo);
	// 2021/10/22 APF OP_BTS-35403 end

	// TYPE_NTF_DataID_DEV_17	コマンドレスポンス通知.
	SSTC_ConcreteFaxObserver::getCmdRs(m_stSysStatInfo.st_DeviceInfoParam.st_Dev17);
	st_ChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_17;
	st_ChangedInfo.p_Value  = &(m_stSysStatInfo.st_DeviceInfoParam.st_Dev17[0]);
	// 2021/10/22 APF OP_BTS-35403 start
	//m_vChangedInfo.push_back(st_ChangedInfo);
	AddChangedInfo(&st_ChangedInfo);
	// 2021/10/22 APF OP_BTS-35403 end

	// TYPE_NTF_DataID_DEV_76	テスト結果通知.
	SSTC_ConcreteFaxObserver::getTestResult(m_stSysStatInfo.st_DeviceInfoParam.e_Dev76);
	st_ChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_76;
	st_ChangedInfo.p_Value  = &(m_stSysStatInfo.st_DeviceInfoParam.e_Dev76[0]);
	// 2021/10/22 APF OP_BTS-35403 start
	//m_vChangedInfo.push_back(st_ChangedInfo);
	AddChangedInfo(&st_ChangedInfo);
	// 2021/10/22 APF OP_BTS-35403 end

	// TYPE_NTF_DataID_DEV_79	プロトコルモニター信号通知.
	SSTC_ConcreteFaxObserver::getProtocolSignal(m_stSysStatInfo.st_DeviceInfoParam.st_Dev79);
	st_ChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_79;
	st_ChangedInfo.p_Value  = &(m_stSysStatInfo.st_DeviceInfoParam.st_Dev79[0]);
	// 2021/10/22 APF OP_BTS-35403 start
	//m_vChangedInfo.push_back(st_ChangedInfo);
	AddChangedInfo(&st_ChangedInfo);
	// 2021/10/22 APF OP_BTS-35403 end

	//.
	// APIC_UsbMemDeviceInfoObserver関連.
	//.
	// TYPE_NTF_DataID_DEV_7	ＵＳＢデバイス状態変化通知.
	SSTC_ConcreteUsbMemDeviceInfoObserver::getStatus(m_stSysStatInfo.st_DeviceInfoParam.ul_Dev7);
	st_ChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_7;
	st_ChangedInfo.p_Value  = &(m_stSysStatInfo.st_DeviceInfoParam.ul_Dev7);
	// 2021/10/22 APF OP_BTS-35403 start
	//m_vChangedInfo.push_back(st_ChangedInfo);
	AddChangedInfo(&st_ChangedInfo);
	// 2021/10/22 APF OP_BTS-35403 end

	// TYPE_NTF_DataID_DEV_8	アクセス状態変化通知（アプリ単位）.
	SSTC_ConcreteUsbMemDeviceInfoObserver::getAccess(m_stSysStatInfo.st_DeviceInfoParam.st_Dev8);
	st_ChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_8;
	st_ChangedInfo.p_Value  = &(m_stSysStatInfo.st_DeviceInfoParam.st_Dev8[0]);
	// 2021/10/22 APF OP_BTS-35403 start
	//m_vChangedInfo.push_back(st_ChangedInfo);
	AddChangedInfo(&st_ChangedInfo);
	// 2021/10/22 APF OP_BTS-35403 end

	// TYPE_NTF_DataID_DEV_9	アクセス状態変化通知.
	SSTC_ConcreteUsbMemDeviceInfoObserver::getAccess(m_stSysStatInfo.st_DeviceInfoParam.ul_Dev9);
	st_ChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_9;
	st_ChangedInfo.p_Value  = &(m_stSysStatInfo.st_DeviceInfoParam.ul_Dev9);
	// 2021/10/22 APF OP_BTS-35403 start
	//m_vChangedInfo.push_back(st_ChangedInfo);
	AddChangedInfo(&st_ChangedInfo);
	// 2021/10/22 APF OP_BTS-35403 end

	// TYPE_NTF_DataID_DEV_15	警告状態変化通知.
	SSTC_ConcreteUsbMemDeviceInfoObserver::getWarning(m_stSysStatInfo.st_DeviceInfoParam.ul_Dev15);
	st_ChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_15;
	st_ChangedInfo.p_Value  = &(m_stSysStatInfo.st_DeviceInfoParam.ul_Dev15);
	// 2021/10/22 APF OP_BTS-35403 start
	//m_vChangedInfo.push_back(st_ChangedInfo);
	AddChangedInfo(&st_ChangedInfo);
	// 2021/10/22 APF OP_BTS-35403 end

	//.
	// APIC_PowerObserver関連.
	//.
	// TYPE_NTF_DataID_DEV_62	パワー状態変化通知(新規).
	SSTC_ConcretePowerObserver::getPowerStatus(m_stSysStatInfo.st_DeviceInfoParam.st_Dev62);
	st_ChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_62;
	st_ChangedInfo.p_Value  = &(m_stSysStatInfo.st_DeviceInfoParam.st_Dev62);
	// 2021/10/22 APF OP_BTS-35403 start
	//m_vChangedInfo.push_back(st_ChangedInfo);
	AddChangedInfo(&st_ChangedInfo);
	// 2021/10/22 APF OP_BTS-35403 end


	// TYPE_NTF_DataID_DEV_39	給紙トレイ情報.
	SSTC_ConcreteFeedTrayObserver::getFeedTrayData(m_stSysStatInfo.st_DeviceInfoParam.st_Dev39);
	st_ChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_39;
	st_ChangedInfo.p_Value  = &(m_stSysStatInfo.st_DeviceInfoParam.st_Dev39);
	// 2021/10/22 APF OP_BTS-35403 start
	//m_vChangedInfo.push_back(st_ChangedInfo);
	AddChangedInfo(&st_ChangedInfo);
	// 2021/10/22 APF OP_BTS-35403 end

	// TYPE_NTF_DataID_DEV_53	メモリ情報.
	SSTC_ConcreteMemoryInfoObserver::getMemoryInfoData(m_stSysStatInfo.st_DeviceInfoParam.st_Dev53);
	st_ChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_53;
	st_ChangedInfo.p_Value  = &(m_stSysStatInfo.st_DeviceInfoParam.st_Dev53);
	// 2021/10/22 APF OP_BTS-35403 start
	//m_vChangedInfo.push_back(st_ChangedInfo);
	AddChangedInfo(&st_ChangedInfo);
	// 2021/10/22 APF OP_BTS-35403 end

	// TYPE_NTF_DataID_DEV_56	原稿情報.
	SSTC_ConcreteOriginalObserver::getOriginalData(m_stSysStatInfo.st_DeviceInfoParam.st_Dev56);
	st_ChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_56;
	st_ChangedInfo.p_Value  = &(m_stSysStatInfo.st_DeviceInfoParam.st_Dev56);
	// 2021/10/22 APF OP_BTS-35403 start
	//m_vChangedInfo.push_back(st_ChangedInfo);
	AddChangedInfo(&st_ChangedInfo);
	// 2021/10/22 APF OP_BTS-35403 end

	// TYPE_NTF_DataID_DEV_57	排紙トレイ情報.
	SSTC_ConcreteOutputTrayObserver::getOutputTrayData(m_stSysStatInfo.st_DeviceInfoParam.st_Dev57);
	st_ChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_57;
	st_ChangedInfo.p_Value  = &(m_stSysStatInfo.st_DeviceInfoParam.st_Dev57);
	// 2021/10/22 APF OP_BTS-35403 start
	//m_vChangedInfo.push_back(st_ChangedInfo);
	AddChangedInfo(&st_ChangedInfo);
	// 2021/10/22 APF OP_BTS-35403 end

}

//*****************************************************************************.
//概要　　：オブザーバで取得するデータの取得処理.
//引数　　：なし.
//戻り値　：なし.
//作成者　：ADS.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//*****************************************************************************.
void SSTC_SystemStatusService::getNetworkObserverData()
{
	SST_LOG(MFPE_LogLevelAll);
	SSTS_ChangedInfo st_ChangedInfo;
	SYSF_bzero((char*)&st_ChangedInfo, sizeof(st_ChangedInfo));
	
	// TYPE_NTF_DataID_NET_14	インターネットISW情報.
	SSTC_ConcreteInternetISWObserver::getInternetISWData(m_stSysStatInfo.st_NetworkInfoParam.st_Net14);
	st_ChangedInfo.e_DataID = TYPE_NTF_DataID_NET_14;
	st_ChangedInfo.p_Value  = &(m_stSysStatInfo.st_NetworkInfoParam.st_Net14);
	// 2021/10/22 APF OP_BTS-35403 start
	//m_vChangedInfo.push_back(st_ChangedInfo);
	AddChangedInfo(&st_ChangedInfo);
	// 2021/10/22 APF OP_BTS-35403 end

	// TYPE_NTF_DataID_NET_25	ネットワークデバイス情報.
	SSTC_ConcreteNetworkDeviceObserver::getNetworkDeviceData(m_stSysStatInfo.st_NetworkInfoParam.st_Net25);
	st_ChangedInfo.e_DataID = TYPE_NTF_DataID_NET_25;
	st_ChangedInfo.p_Value  = &(m_stSysStatInfo.st_NetworkInfoParam.st_Net25);
	// 2021/10/22 APF OP_BTS-35403 start
	//m_vChangedInfo.push_back(st_ChangedInfo);
	AddChangedInfo(&st_ChangedInfo);
	// 2021/10/22 APF OP_BTS-35403 end

}

//*****************************************************************************.
//概要　　：オブザーバで取得するデータの取得処理.
//引数　　：なし.
//戻り値　：なし.
//作成者　：ADS.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//*****************************************************************************.
void SSTC_SystemStatusService::getOtherObserverData()
{
	SST_LOG(MFPE_LogLevelAll);
	SSTS_ChangedInfo st_ChangedInfo;
	SYSF_bzero((char*)&st_ChangedInfo, sizeof(st_ChangedInfo));
	
	// TYPE_NTF_DataID_OTH_1	サーチャブルPDF生成機能情報.
	SSTC_ConcreteSearchablePDFSettingObserver::getSearchablePDFSettingData(m_stSysStatInfo.st_OtherInfoParam.st_Oth1);
	st_ChangedInfo.e_DataID = TYPE_NTF_DataID_OTH_1;
	st_ChangedInfo.p_Value  = &(m_stSysStatInfo.st_OtherInfoParam.st_Oth1);
	// 2021/10/22 APF OP_BTS-35403 start
	//m_vChangedInfo.push_back(st_ChangedInfo);
	AddChangedInfo(&st_ChangedInfo);
	// 2021/10/22 APF OP_BTS-35403 end

}


// ログ出力レベル設定用関数.
#if defined(__cplusplus)
extern "C" {
#endif
	extern void SSTF_setLogLevel(MFPE_LogLevel lv) { printf("setLogLv=%d", lv);SSTC_SystemStatusService::setLogLevel(lv);}
#if defined(__cplusplus)
}
#endif

/**
* \note ステータス情報取得(オブザーバ).
   2012. 5.15 [CQ363516] パフォーマンス改善 Observerで検出可能な情報をポーリング→Observerに変更した対応で追加した関数.
*/

//************************************************************************************************************************.
//概要　　：データ変化通知受信時処理.
//引数　　：<in>　e_DataID			データID.
//戻り値　：通知必要(True)または通知不要(False).
//備考　　：特に無し.
//作成者　：ADS.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//************************************************************************************************************************.
Bool SSTC_SystemStatusService::notifyObserverData(TYPE_NTF_DataID e_DataID)
{
	SST_LOG2(MFPE_LogLevelTrace, "e_DataID=(0x%x).\n", e_DataID);

	Bool b_Ret = True;

	// 変化情報初期化.
	m_vChangedInfo.clear();

	switch (e_DataID) {
	///< リモートパネル情報.
	case TYPE_NTF_DataID_RPL_1:	// APIC_PrinterSystemObserver::dataChanged().
		getPrinterSystemObserverRemotePanelInfo(); // リモートパネル情報.
		b_Ret = False;	// TYPE_NTF_DataID_RPL_1の変化点通知は検出用関数で検地した場合に本関数内で通知する為不要.
		break;

	///< サービス情報.
	case TYPE_NTF_DataID_SRV_7:	// APIC_OperationSettingObserver::dataChanged().
		getOperationSettingObserverData();
		break;

	///< システム情報.
	case TYPE_NTF_DataID_SYS_5:	// APIC_EnablerSettingObserver::dataChanged().
		getEnablerSettingObserverData();
		break;
	case TYPE_NTF_DataID_SYS_6:	// APIC_SystemInfoObserver::dataChanged().
		m_ulCounterSystemInfoObserver++;	// 発生回数インクリメント.
		getSystemInfoObserverData();
		getSystemInfoObserverDeviceInfo(); // デバイス情報.
		b_Ret = False;	// 変化点通知は不要.
		break;
	case TYPE_NTF_DataID_SYS_7:	// APIC_SystemSettingObserver::dataChanged().
		getSystemSettingObserverData();
		break;
	case TYPE_NTF_DataID_SYS_8:	// APIC_SystemStatusObserver::dataChanged().
		getSystemStatusObserverData();
		getSystemStatusObserverPrinterDeviceInfo(); // デバイス情報_プリンタ.
		b_Ret = False;	// TYPE_NTF_DataID_SYS_8の変化点通知は検出用関数で検地した場合に本関数内で通知する為不要.
		break;
	case TYPE_NTF_DataID_SYS_93:	// APIC_DebugLogSystemSettingObserver::dataChanged().
		getDebugLogSystemSettingObserverData();
		b_Ret = False;	// TYPE_NTF_DataID_SYS_93の変化点通知は検出用関数で検地した場合に本関数内で通知する為不要.
		break;
	case TYPE_NTF_DataID_SYS_109:	// APIC_UserAuthenticationObserver::dataChanged().
		getUserAuthenticationObserverData();
		b_Ret = False;	// TYPE_NTF_DataID_SYS_109の変化点通知は検出用関数で検地した場合に本関数内で通知する為不要.
		break;
// 2016/07/01 2Sys-Sky西村 RQ-436対応 start.
	case TYPE_NTF_DataID_SYS_114:	// SSTC_ConcreteIwsObserver::dataChanged().
		getIwsObserverData();
		b_Ret = False;	// TYPE_NTF_DataID_SYS_114の変化点通知は検出用関数で検知した場合に本関数内で通知する為不要.
		break;
// 2016/07/01 2Sys-Sky西村 RQ-436対応 end.

	///< スキャナー情報.
	case TYPE_NTF_DataID_SCN_3:			// APIC_ScanJobSettingObserver::dataChanged().
		getScanJobSettingObserverData();
		break;

	///< デバイス情報_プリンタ.
	case TYPE_NTF_DataID_DEV_PRT_86:	// APIC_PrinterWarningObserver::dataChanged().
		m_ulCounterPrinterWarningObserver++;	// 発生回数インクリメント.
		b_Ret = False;	// TYPE_NTF_DataID_DEV_PRT_86の変化点通知はPolling処理で通知する為不要.
		break;
	case TYPE_NTF_DataID_DEV_PRT_111:	// APIC_PrinterSettingObserver::dataChanged().
		getPrinterSettingObserverData();
		break;
	case TYPE_NTF_DataID_DEV_PRT_112:	// APIC_PrinterStatusObserver::dataChanged().
		m_ulCounterPrinterStatusObserver++;	// 発生回数インクリメント.
		getPrinterStatusObserverData();
		break;

	///< デバイス情報.
	case TYPE_NTF_DataID_DEV_15:	// APIC_UsbMemDeviceInfoObserver::warningChanged().
		getUsbWarningObserverData();
		break;
	case TYPE_NTF_DataID_DEV_25:	// APIC_CopyJobSettingObserver::dataChanged().
		getCopyJobSettingObserverData();
		break;
	case TYPE_NTF_DataID_DEV_37:	// APIC_FaxJobSettingObserver::dataChanged().
		getFaxJobSettingObserverData();
		break;
	case TYPE_NTF_DataID_DEV_51:	// APIC_MachineSettingObserver::dataChanged().
		getMachineSettingObserverPrinterDeviceInfo(); // デバイス情報_プリンタ.
		getMachineSettingObserverData();
		b_Ret = False;	/* TYPE_NTF_DataID_DEV_51の変化点通知は検出用関数で検地した場合に本関数内で通知する為不要 */
		break;
	case TYPE_NTF_DataID_DEV_62:	// APIC_PowerObserver::powerStatusChanged().
		m_ulCounterPowerStatusChanged++;	// 発生回数インクリメント.
		break;
	case TYPE_NTF_DataID_DEV_67:	// APIC_SoftDipSwObserver::dataChanged().
		m_ulCounterSoftDipSwObserver++;	// 発生回数インクリメント.
		getSoftDipSwObserverData();
		break;
	case TYPE_NTF_DataID_DEV_105:	// APIC_FaxJobSettingObserver::dataChanged().
		getFaxJobSettingObserverData();
		b_Ret = False;	/* TYPE_NTF_DataID_DEV_105の変化点通知は検出用関数で検地した場合に本関数内で通知する為不要 */
		break;

	///< ネットワーク情報.
	case TYPE_NTF_DataID_NET_35:	// APIC_SmtpSendSettingObserver::dataChanged().
		getSmtpSendSettingObserverData();
		break;

	///< その他.
	case TYPE_NTF_DataID_OTH_5:		// APIC_ImageLogObserver::notifyImageLogWarning().
		getImageLogObserverData();
		break;

// 【RQ-747 VirusScanFUM for 機種】 start.
	case TYPE_NTF_DataID_OTH_15:	// APIC_VirusScanSettingObserver::dataChanged().
	case TYPE_NTF_DataID_OTH_16:	// APIC_VirusScanObserver::NotifyEvent_PatternFile()
		getVirusScanObserverData();
		b_Ret = False;	// TYPE_NTF_DataID_OTH_15.16の変化点通知は検出用関数で検知した場合に本関数内で通知する為不要.
		break;
// 【RQ-747 VirusScanFUM for 機種】 end.

// 2018/09/29 [RQ-674] SCD2-motohashi ジョブログ関連の警告取得処理は定期起床処理で行うように変更.
//// 2018/08/30 [RQ-674] SCD2-motohashi ログ遅延状態を追加.
//	case TYPE_NTF_DataID_SRV_3:	// APIC_JLGObserver::JobLogDataFULL().
//		getJlgObserverData();
//		break;

	default:
		break;
	}

	TYPE_NTF_GroupID  e_writeGroupID    = TYPE_NTF_GroupID_End;
	for( std::vector<SSTS_ChangedInfo>::iterator it = m_vChangedInfo.begin(); it != m_vChangedInfo.end(); ++it ){
		
		// グループ情報取得 該当なしの場合はエラーログ出力.
		TYPE_NTF_GroupID  e_GroupID    = TYPE_NTF_GroupID_End;
		void 			 *pv_GroupData = NULL;
		
		if( SSTC_SystemStatusService::getGroupInfo( (it)->e_DataID, &e_GroupID, &pv_GroupData ) == False ){
			// エラーログ出力.
			SST_ERR_LOG("getGroupInfo() == False.\n");
			continue;
		}
		if( e_GroupID != e_writeGroupID ){
			// 共有メモリ更新.
			m_pcInstance->lock();
			m_pcShMemMan->writeData( e_GroupID, pv_GroupData );
			m_pcInstance->unlock();
			e_writeGroupID = e_GroupID;
		}
	}

	if( m_bStartNotifyFlag == True ){
		for( std::vector<SSTS_ChangedInfo>::iterator it = m_vChangedInfo.begin(); it != m_vChangedInfo.end(); ++it ){
			// データ変化通知.
			notifyChangedData( (it)->e_DataID );
		}
	}

	// 変化データをクリア.
	m_vChangedInfo.clear();

	SST_LOG2(MFPE_LogLevelTrace, "End. b_Ret=(%d).\n", (int)b_Ret );
	return b_Ret;

}

//************************************************************************************************************************.
//概要　　：APIC_OperationSettingObserver::dataChanged情報取得処理.
//戻り値　：無し.
//備考　　：特に無し.
//作成者　：ADS.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
// 2012. 8.31 [CQ371105] 静的解析 APIC_OperationSetting::newInstance()はNULLリターンしないため、NULLチェックを外す.
//************************************************************************************************************************.
void SSTC_SystemStatusService::getOperationSettingObserverData()
{
	SST_LOG(MFPE_LogLevelAll);
	SSTS_ChangedInfo stChangedInfo;
	Bool             b_Ret = False;
	
	APIC_OperationSetting* pc_OperationSetting = APIC_OperationSetting::newInstance();
	
	// 拡大ローテーション使用情報設定.
	b_Ret = pc_OperationSetting->getEnlargedRotateEnable();
	if( b_Ret != m_stSysStatInfo.st_ServiceInfoParam.b_Srv10 ){
		m_stSysStatInfo.st_ServiceInfoParam.b_Srv10 = b_Ret;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SRV_10;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_ServiceInfoParam.b_Srv10 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// 拡大ローテーション時AMS倍率方向不可警告設定.
	b_Ret = pc_OperationSetting->getAMSWarningInEnlargedRotate();
	if( b_Ret != m_stSysStatInfo.st_ServiceInfoParam.b_Srv11 ){
		m_stSysStatInfo.st_ServiceInfoParam.b_Srv11 = b_Ret;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SRV_11;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_ServiceInfoParam.b_Srv11 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// 新API対応：2011.05.26 <ADD> Start T.Takahashi.
	
	// 中とじ/中折り時の小冊子選択.
	b_Ret = pc_OperationSetting->getBookletOnAtInnerFoldSelect();
	if( b_Ret != m_stSysStatInfo.st_ServiceInfoParam.b_Srv12 ){
		m_stSysStatInfo.st_ServiceInfoParam.b_Srv12 = b_Ret;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SRV_12;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_ServiceInfoParam.b_Srv12 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// Ｎｉｎ１優先倍率設定.
	b_Ret = pc_OperationSetting->getEnableNin1Mag();
	if( b_Ret != m_stSysStatInfo.st_ServiceInfoParam.b_Srv13 ){
		m_stSysStatInfo.st_ServiceInfoParam.b_Srv13 = b_Ret;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SRV_13;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_ServiceInfoParam.b_Srv13 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// 新API対応：2011.05.26 <ADD> End T.Takahashi.

	// Job実行可否対応 2011.11.22 ADS K.Tsuboi Start.
	// パスワードコピー印字効果設定.
	b_Ret = pc_OperationSetting->getPasswordCopyEnable();
	if( b_Ret != m_stSysStatInfo.st_ServiceInfoParam.b_Srv14 ){
		m_stSysStatInfo.st_ServiceInfoParam.b_Srv14 = b_Ret;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SRV_14;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_ServiceInfoParam.b_Srv14 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}

	// コピーガード印字効果設定.
	b_Ret = pc_OperationSetting->getCopyGuardEnable();
	if( b_Ret != m_stSysStatInfo.st_ServiceInfoParam.b_Srv15 ){
		m_stSysStatInfo.st_ServiceInfoParam.b_Srv15 = b_Ret;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SRV_15;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_ServiceInfoParam.b_Srv15 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	// Job実行可否対応 2011.11.22 ADS K.Tsuboi End.

	SST_LOG2(MFPE_LogLevelAll, "End.\n", NULL);
}

//************************************************************************************************************************.
//概要　　：APIC_EnablerSettingObserver::dataChanged情報取得処理.
//戻り値　：無し.
//備考　　：特に無し.
//作成者　：ADS.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//************************************************************************************************************************.
void SSTC_SystemStatusService::getEnablerSettingObserverData()
{
	SST_LOG(MFPE_LogLevelAll);
	SSTS_ChangedInfo stChangedInfo;

	// FEK有効無効状態.
	APIC_EnablerSetting* pc_EnablerSetting = APIC_EnablerSetting::newInstance();
	Bool b_UpdFlg = False;
	for( ushort us_Idx = 0; us_Idx < TYPD_SSTS_MaxFEKType; ++us_Idx ){	/* 静的解析 */
		TYPE_CategoryState e_CategoryStateT = pc_EnablerSetting->getFEK_CotegoryState( (TYPE_FEKType)us_Idx, True );
		TYPE_CategoryState e_CategoryStateF = pc_EnablerSetting->getFEK_CotegoryState( (TYPE_FEKType)us_Idx, False );
		if( m_stSysStatInfo.st_SystemInfoParam.e_Sys85[ us_Idx ][ TYPD_SSTS_TrueIdx ] != e_CategoryStateT ){
			m_stSysStatInfo.st_SystemInfoParam.e_Sys85[ us_Idx ][ TYPD_SSTS_TrueIdx ]  = e_CategoryStateT;
			b_UpdFlg = True;
		}
		if( m_stSysStatInfo.st_SystemInfoParam.e_Sys85[ us_Idx ][ TYPD_SSTS_FalseIdx ] != e_CategoryStateF ){
			m_stSysStatInfo.st_SystemInfoParam.e_Sys85[ us_Idx ][ TYPD_SSTS_FalseIdx ]  = e_CategoryStateF;
			b_UpdFlg = True;
		}
	}
	if( b_UpdFlg ){
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SYS_85;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_SystemInfoParam.e_Sys85 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}

	SST_LOG2(MFPE_LogLevelAll, "End.\n", NULL);
}

//************************************************************************************************************************.
//概要　　：APIC_SystemInfoObserver::dataChanged情報取得処理.
//戻り値　：無し.
//備考　　：特に無し.
//作成者　：ADS.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//************************************************************************************************************************.
void SSTC_SystemStatusService::getSystemInfoObserverData()
{
	SST_LOG(MFPE_LogLevelAll);
	SSTS_ChangedInfo stChangedInfo;

	// 概要：製品名情報.
	char sc_SysDescr[ SYSD_MAX_SysDescr ];
	memset( sc_SysDescr, (char)SSTD_ZERO_OUT, SYSD_MAX_SysDescr );
	APIC_SystemInfo* pc_SystemInfo = APIC_SystemInfo::newInstance();
	pc_SystemInfo->getSysDescr( sc_SysDescr );
	if( strncmp( m_stSysStatInfo.st_SystemInfoParam.sc_Sys14, sc_SysDescr, SYSD_MAX_SysDescr ) != 0 ){
		memset( m_stSysStatInfo.st_SystemInfoParam.sc_Sys14, (char)SSTD_ZERO_OUT, SYSD_MAX_SysDescr );
		strncpy( m_stSysStatInfo.st_SystemInfoParam.sc_Sys14, sc_SysDescr, SYSD_MAX_SysDescr );
//Str【 静的解析対応 : Sky田村 : 2012.12.11 : CID-435211】.
		m_stSysStatInfo.st_SystemInfoParam.sc_Sys14[SYSD_MAX_SysDescr-1] = '\0';
//End【 静的解析対応 : Sky田村 : 2012.12.11 : CID-435211】.
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SYS_14;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_SystemInfoParam.sc_Sys14 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	Bool b_UpdFlg = False;
	// RomVersion情報.
	char sc_RomVersion[ TYPD_SSTS_RomVersionLength ];
	memset( sc_RomVersion, (char)SSTD_ZERO_OUT, TYPD_SSTS_RomVersionLength );
	for( ushort us_Idx = 0; us_Idx < (ushort)TYPE_RomMax; ++us_Idx ){
		pc_SystemInfo->getRomVersion( (TYPE_Rom)us_Idx, sc_RomVersion );
		if( strncmp( m_stSysStatInfo.st_SystemInfoParam.sc_Sys15[ us_Idx ], sc_RomVersion, TYPD_SSTS_RomVersionLength ) != 0 ){
			memset( m_stSysStatInfo.st_SystemInfoParam.sc_Sys15[ us_Idx ], (char)SSTD_ZERO_OUT, TYPD_SSTS_RomVersionLength );
			strncpy( m_stSysStatInfo.st_SystemInfoParam.sc_Sys15[ us_Idx ], sc_RomVersion, TYPD_SSTS_RomVersionLength );
//Str【 静的解析対応 : Sky田村 : 2012.12.11 : CID-435211】.
			m_stSysStatInfo.st_SystemInfoParam.sc_Sys15[ us_Idx ][TYPD_SSTS_RomVersionLength-1] = '\0';
//End【 静的解析対応 : Sky田村 : 2012.12.11 : CID-435211】.
			b_UpdFlg = True;
		}
	}
	if( b_UpdFlg ){
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SYS_15;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_SystemInfoParam.sc_Sys15[ 0 ] );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// Romチェックサム.
	ushort us_Val = 0;
	b_UpdFlg      = False;
	for( us_Idx = 0; us_Idx < (ushort)TYPE_RomMax; ++us_Idx ){
		us_Val = pc_SystemInfo->getRomVersion( (TYPE_Rom)us_Idx );
		if( m_stSysStatInfo.st_SystemInfoParam.us_Sys16[ us_Idx ] != us_Val ){
			m_stSysStatInfo.st_SystemInfoParam.us_Sys16[ us_Idx ] = us_Val;
			b_UpdFlg = True;
		}
	}
	if( b_UpdFlg ){
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SYS_16;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_SystemInfoParam.us_Sys16[ 0 ] );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// ＭＦＰコントローラ内のサブＲＯＭバージョン.
	char sc_CtlRom[ TYPD_SSTS_RomVersionLength ];
	b_UpdFlg = False;
	memset( sc_CtlRom, (char)SSTD_ZERO_OUT, TYPD_SSTS_RomVersionLength );
	for( us_Idx = 0; us_Idx < (ushort)TYPE_MFPControllerRomMax; ++us_Idx ){
		pc_SystemInfo->getMFPControllerRomVersion( (TYPE_Rom)us_Idx, sc_CtlRom );
		if( strncmp( m_stSysStatInfo.st_SystemInfoParam.sc_Sys17[ us_Idx ], sc_CtlRom, TYPD_SSTS_RomVersionLength ) != 0 ){
			memset( m_stSysStatInfo.st_SystemInfoParam.sc_Sys17[ us_Idx ], (char)SSTD_ZERO_OUT, TYPD_SSTS_RomVersionLength );
			strncpy( m_stSysStatInfo.st_SystemInfoParam.sc_Sys17[ us_Idx ], sc_CtlRom, TYPD_SSTS_RomVersionLength );
//Str【 静的解析対応 : Sky田村 : 2012.12.11 : CID-435211】.
			m_stSysStatInfo.st_SystemInfoParam.sc_Sys17[ us_Idx ][TYPD_SSTS_RomVersionLength-1] = '\0';
//End【 静的解析対応 : Sky田村 : 2012.12.11 : CID-435211】.
			b_UpdFlg = True;
		}
	}
	if( b_UpdFlg ){
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SYS_17;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_SystemInfoParam.sc_Sys17[ 0 ] );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// 製品タイプ情報.
	Bool b_Val = pc_SystemInfo->getMachineTypeInfo();
	if( m_stSysStatInfo.st_SystemInfoParam.b_Sys18 != b_Val ){
		m_stSysStatInfo.st_SystemInfoParam.b_Sys18 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SYS_18;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_SystemInfoParam.b_Sys18 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// ＰＣＩ拡張基板有無.
	b_Val = pc_SystemInfo->getPciExpandBoardExist();
	if( m_stSysStatInfo.st_SystemInfoParam.b_Sys23 != b_Val ){
		m_stSysStatInfo.st_SystemInfoParam.b_Sys23 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SYS_23;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_SystemInfoParam.b_Sys23 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// ローカルＩ／Ｆ有無.
	b_Val = pc_SystemInfo->getLocalIfExist();
	if( m_stSysStatInfo.st_SystemInfoParam.b_Sys24 != b_Val ){
		m_stSysStatInfo.st_SystemInfoParam.b_Sys24 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SYS_24;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_SystemInfoParam.b_Sys24 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// 暗号化基板有無.
	b_Val = pc_SystemInfo->getEncryptionBoardExist();
	if( m_stSysStatInfo.st_SystemInfoParam.b_Sys27 != b_Val ){
		m_stSysStatInfo.st_SystemInfoParam.b_Sys27 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SYS_27;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_SystemInfoParam.b_Sys27 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// シリアルナンバー・プリンタ.
	char sc_SerialNo[ TYPD_SSTS_SerialNumberLength ];
	memset( sc_SerialNo, (char)SSTD_ZERO_OUT, TYPD_SSTS_SerialNumberLength );
	pc_SystemInfo->getSerialNoPrinter( sc_SerialNo );
	if( strncmp( m_stSysStatInfo.st_SystemInfoParam.sc_Sys29, sc_SerialNo, TYPD_SSTS_SerialNumberLength ) != 0 ){
		memset( m_stSysStatInfo.st_SystemInfoParam.sc_Sys29, (char)SSTD_ZERO_OUT, TYPD_SSTS_SerialNumberLength );
		strncpy( m_stSysStatInfo.st_SystemInfoParam.sc_Sys29, sc_SerialNo, TYPD_SSTS_SerialNumberLength );
//Str【 静的解析対応 : Sky田村 : 2012.12.11 : CID-435211】.
		m_stSysStatInfo.st_SystemInfoParam.sc_Sys29[TYPD_SSTS_SerialNumberLength-1] = '\0';
//End【 静的解析対応 : Sky田村 : 2012.12.11 : CID-435211】.
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SYS_29;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_SystemInfoParam.sc_Sys29 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// シリアルナンバー・スキャナ.
	memset( sc_SerialNo, (char)SSTD_ZERO_OUT, TYPD_SSTS_SerialNumberLength );
	pc_SystemInfo->getSerialNoScaner( sc_SerialNo );
	if( strncmp( m_stSysStatInfo.st_SystemInfoParam.sc_Sys30, sc_SerialNo, TYPD_SSTS_SerialNumberLength ) != 0 ){
		memset( m_stSysStatInfo.st_SystemInfoParam.sc_Sys30, (char)SSTD_ZERO_OUT, TYPD_SSTS_SerialNumberLength );
		strncpy( m_stSysStatInfo.st_SystemInfoParam.sc_Sys30, sc_SerialNo, TYPD_SSTS_SerialNumberLength );
//Str【 静的解析対応 : Sky田村 : 2012.12.11 : CID-435211】.
		m_stSysStatInfo.st_SystemInfoParam.sc_Sys30[TYPD_SSTS_SerialNumberLength-1] = '\0';
//End【 静的解析対応 : Sky田村 : 2012.12.11 : CID-435211】.
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SYS_30;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_SystemInfoParam.sc_Sys30 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// シリアルナンバー・ＡＤＦ.
	memset( sc_SerialNo, (char)SSTD_ZERO_OUT, TYPD_SSTS_SerialNumberLength );
	pc_SystemInfo->getSerialNoAdf( sc_SerialNo );
	if( strncmp( m_stSysStatInfo.st_SystemInfoParam.sc_Sys31, sc_SerialNo, TYPD_SSTS_SerialNumberLength ) != 0 ){
		memset( m_stSysStatInfo.st_SystemInfoParam.sc_Sys31, (char)SSTD_ZERO_OUT, TYPD_SSTS_SerialNumberLength );
		strncpy( m_stSysStatInfo.st_SystemInfoParam.sc_Sys31, sc_SerialNo, TYPD_SSTS_SerialNumberLength );
//Str【 静的解析対応 : Sky田村 : 2012.12.11 : CID-435211】.
		m_stSysStatInfo.st_SystemInfoParam.sc_Sys31[TYPD_SSTS_SerialNumberLength-1] = '\0';
//End【 静的解析対応 : Sky田村 : 2012.12.11 : CID-435211】.
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SYS_31;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_SystemInfoParam.sc_Sys31 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// シリアルナンバー・ＬＣＣ.
	memset( sc_SerialNo, (char)SSTD_ZERO_OUT, TYPD_SSTS_SerialNumberLength );
	pc_SystemInfo->getSerialNoLcc( sc_SerialNo );
	if( strncmp( m_stSysStatInfo.st_SystemInfoParam.sc_Sys32, sc_SerialNo, TYPD_SSTS_SerialNumberLength ) != 0 ){
		memset( m_stSysStatInfo.st_SystemInfoParam.sc_Sys32, (char)SSTD_ZERO_OUT, TYPD_SSTS_SerialNumberLength );
		strncpy( m_stSysStatInfo.st_SystemInfoParam.sc_Sys32, sc_SerialNo, TYPD_SSTS_SerialNumberLength );
//Str【 静的解析対応 : Sky田村 : 2012.12.11 : CID-435211】.
		m_stSysStatInfo.st_SystemInfoParam.sc_Sys32[TYPD_SSTS_SerialNumberLength-1] = '\0';
//End【 静的解析対応 : Sky田村 : 2012.12.11 : CID-435211】.
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SYS_32;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_SystemInfoParam.sc_Sys32 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// シリアルナンバー・排紙ＯＰ.
	memset( sc_SerialNo, (char)SSTD_ZERO_OUT, TYPD_SSTS_SerialNumberLength );
	pc_SystemInfo->getSerialNoEjectOP( sc_SerialNo );
	if( strncmp( m_stSysStatInfo.st_SystemInfoParam.sc_Sys33, sc_SerialNo, TYPD_SSTS_SerialNumberLength ) != 0 ){
		memset( m_stSysStatInfo.st_SystemInfoParam.sc_Sys33, (char)SSTD_ZERO_OUT, TYPD_SSTS_SerialNumberLength );
		strncpy( m_stSysStatInfo.st_SystemInfoParam.sc_Sys33, sc_SerialNo, TYPD_SSTS_SerialNumberLength );
//Str【 静的解析対応 : Sky田村 : 2012.12.11 : CID-435211】.
		m_stSysStatInfo.st_SystemInfoParam.sc_Sys33[TYPD_SSTS_SerialNumberLength-1] = '\0';
//End【 静的解析対応 : Sky田村 : 2012.12.11 : CID-435211】.
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SYS_33;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_SystemInfoParam.sc_Sys33 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// シリアルナンバー・Ｚ折りユニット.
	memset( sc_SerialNo, (char)SSTD_ZERO_OUT, TYPD_SSTS_SerialNumberLength );
	pc_SystemInfo->getSerialNoZU( sc_SerialNo );
	if( strncmp( m_stSysStatInfo.st_SystemInfoParam.sc_Sys34, sc_SerialNo, TYPD_SSTS_SerialNumberLength ) != 0 ){
		memset( m_stSysStatInfo.st_SystemInfoParam.sc_Sys34, (char)SSTD_ZERO_OUT, TYPD_SSTS_SerialNumberLength );
		strncpy( m_stSysStatInfo.st_SystemInfoParam.sc_Sys34, sc_SerialNo, TYPD_SSTS_SerialNumberLength );
//Str【 静的解析対応 : Sky田村 : 2012.12.11 : CID-435211】.
		m_stSysStatInfo.st_SystemInfoParam.sc_Sys34[TYPD_SSTS_SerialNumberLength-1] = '\0';
//End【 静的解析対応 : Sky田村 : 2012.12.11 : CID-435211】.
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SYS_34;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_SystemInfoParam.sc_Sys34 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// シリアルナンバー・リレーユニット.
	memset( sc_SerialNo, (char)SSTD_ZERO_OUT, TYPD_SSTS_SerialNumberLength );
	pc_SystemInfo->getSerialNoRU( sc_SerialNo );
	if( strncmp( m_stSysStatInfo.st_SystemInfoParam.sc_Sys35, sc_SerialNo, TYPD_SSTS_SerialNumberLength ) != 0 ){
		memset( m_stSysStatInfo.st_SystemInfoParam.sc_Sys35, (char)SSTD_ZERO_OUT, TYPD_SSTS_SerialNumberLength );
		strncpy( m_stSysStatInfo.st_SystemInfoParam.sc_Sys35, sc_SerialNo, TYPD_SSTS_SerialNumberLength );
//Str【 静的解析対応 : Sky田村 : 2012.12.11 : CID-435211】.
		m_stSysStatInfo.st_SystemInfoParam.sc_Sys35[TYPD_SSTS_SerialNumberLength-1] = '\0';
//End【 静的解析対応 : Sky田村 : 2012.12.11 : CID-435211】.
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SYS_35;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_SystemInfoParam.sc_Sys35 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// シリアルナンバー・再給紙.
	memset( sc_SerialNo, (char)SSTD_ZERO_OUT, TYPD_SSTS_SerialNumberLength );
	pc_SystemInfo->getSerialNoDuplex( sc_SerialNo );
	if( strncmp( m_stSysStatInfo.st_SystemInfoParam.sc_Sys36, sc_SerialNo, TYPD_SSTS_SerialNumberLength ) != 0 ){
		memset( m_stSysStatInfo.st_SystemInfoParam.sc_Sys36, (char)SSTD_ZERO_OUT, TYPD_SSTS_SerialNumberLength );
		strncpy( m_stSysStatInfo.st_SystemInfoParam.sc_Sys36, sc_SerialNo, TYPD_SSTS_SerialNumberLength );
//Str【 静的解析対応 : Sky田村 : 2012.12.11 : CID-435211】.
		m_stSysStatInfo.st_SystemInfoParam.sc_Sys36[TYPD_SSTS_SerialNumberLength-1] = '\0';
//End【 静的解析対応 : Sky田村 : 2012.12.11 : CID-435211】.
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SYS_36;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_SystemInfoParam.sc_Sys36 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// シリアルナンバー・コピーベンダー.
	memset( sc_SerialNo, (char)SSTD_ZERO_OUT, TYPD_SSTS_SerialNumberLength );
	pc_SystemInfo->getSerialNoVendor( sc_SerialNo );
	if( strncmp( m_stSysStatInfo.st_SystemInfoParam.sc_Sys37, sc_SerialNo, TYPD_SSTS_SerialNumberLength ) != 0 ){
		memset( m_stSysStatInfo.st_SystemInfoParam.sc_Sys37, (char)SSTD_ZERO_OUT, TYPD_SSTS_SerialNumberLength );
		strncpy( m_stSysStatInfo.st_SystemInfoParam.sc_Sys37, sc_SerialNo, TYPD_SSTS_SerialNumberLength );
//Str【 静的解析対応 : Sky田村 : 2012.12.11 : CID-435211】.
		m_stSysStatInfo.st_SystemInfoParam.sc_Sys37[TYPD_SSTS_SerialNumberLength-1] = '\0';
//End【 静的解析対応 : Sky田村 : 2012.12.11 : CID-435211】.
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SYS_37;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_SystemInfoParam.sc_Sys37 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// シリアルナンバー・ファクス１.
	memset( sc_SerialNo, (char)SSTD_ZERO_OUT, TYPD_SSTS_SerialNumberLength );
	pc_SystemInfo->getSerialNoFax1( sc_SerialNo );
	if( strncmp( m_stSysStatInfo.st_SystemInfoParam.sc_Sys38, sc_SerialNo, TYPD_SSTS_SerialNumberLength ) != 0 ){
		memset( m_stSysStatInfo.st_SystemInfoParam.sc_Sys38, (char)SSTD_ZERO_OUT, TYPD_SSTS_SerialNumberLength );
		strncpy( m_stSysStatInfo.st_SystemInfoParam.sc_Sys38, sc_SerialNo, TYPD_SSTS_SerialNumberLength );
//Str【 静的解析対応 : Sky田村 : 2012.12.11 : CID-435211】.
		m_stSysStatInfo.st_SystemInfoParam.sc_Sys38[TYPD_SSTS_SerialNumberLength-1] = '\0';
//End【 静的解析対応 : Sky田村 : 2012.12.11 : CID-435211】.
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SYS_38;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_SystemInfoParam.sc_Sys38 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// シリアルナンバー・ファクス２.
	memset( sc_SerialNo, (char)SSTD_ZERO_OUT, TYPD_SSTS_SerialNumberLength );
	pc_SystemInfo->getSerialNoFax2( sc_SerialNo );
	if( strncmp( m_stSysStatInfo.st_SystemInfoParam.sc_Sys39, sc_SerialNo, TYPD_SSTS_SerialNumberLength ) != 0 ){
		strncpy( m_stSysStatInfo.st_SystemInfoParam.sc_Sys39, sc_SerialNo, TYPD_SSTS_SerialNumberLength );
//Str【 静的解析対応 : Sky田村 : 2012.12.11 : CID-435211】.
		m_stSysStatInfo.st_SystemInfoParam.sc_Sys39[TYPD_SSTS_SerialNumberLength-1] = '\0';
//End【 静的解析対応 : Sky田村 : 2012.12.11 : CID-435211】.
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SYS_39;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_SystemInfoParam.sc_Sys39 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	//  Pre Rom Version 設定.
	char sc_PreRomVer[ TYPD_MSK_PreRomVersion ];
	memset( sc_PreRomVer, (char)SSTD_ZERO_OUT, TYPD_MSK_PreRomVersion );
	pc_SystemInfo->getPreRomVersion( sc_PreRomVer );
	if( strncmp( m_stSysStatInfo.st_SystemInfoParam.sc_Sys54, sc_PreRomVer, TYPD_MSK_PreRomVersion ) != 0 ){
		memset( m_stSysStatInfo.st_SystemInfoParam.sc_Sys54, (char)SSTD_ZERO_OUT, TYPD_MSK_PreRomVersion );
		strncpy( m_stSysStatInfo.st_SystemInfoParam.sc_Sys54, sc_PreRomVer, TYPD_MSK_PreRomVersion );
//Str【 静的解析対応 : Sky田村 : 2012.12.11 : CID-435211】.
		m_stSysStatInfo.st_SystemInfoParam.sc_Sys54[TYPD_MSK_PreRomVersion-1] = '\0';
//End【 静的解析対応 : Sky田村 : 2012.12.11 : CID-435211】.
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SYS_54;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_SystemInfoParam.sc_Sys54 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// トラブルリセットモード.
	b_Val = pc_SystemInfo->getTroubleResetMode();
	if( m_stSysStatInfo.st_SystemInfoParam.b_Sys55 != b_Val ){
		m_stSysStatInfo.st_SystemInfoParam.b_Sys55 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SYS_55;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_SystemInfoParam.b_Sys55 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// FirmWare ダウンロード 設定.
	b_Val = pc_SystemInfo->getFirmWareDownLoad();
	if( m_stSysStatInfo.st_SystemInfoParam.b_Sys56 != b_Val ){
		m_stSysStatInfo.st_SystemInfoParam.b_Sys56 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SYS_56;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_SystemInfoParam.b_Sys56 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	

	// Thames1P 強制切替モード設定.
	b_Val = pc_SystemInfo->getPrinterModelForceChange();
	if( m_stSysStatInfo.st_SystemInfoParam.b_Sys59 != b_Val ){
		m_stSysStatInfo.st_SystemInfoParam.b_Sys59 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SYS_59;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_SystemInfoParam.b_Sys59 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// Thames1P 強制モデル設定.
	TYPE_PrinterModelKind e_PrtMdlKind = pc_SystemInfo->getPrinterModelForceKind();
	if( m_stSysStatInfo.st_SystemInfoParam.e_Sys60 != e_PrtMdlKind ){
		m_stSysStatInfo.st_SystemInfoParam.e_Sys60 = e_PrtMdlKind;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SYS_60;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_SystemInfoParam.e_Sys60 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}

	// Ｆ／Ｗバージョン.
	TYPE_FirmWareVersion e_FirmWareVer = pc_SystemInfo->getFirmWareVersion();
	if( m_stSysStatInfo.st_SystemInfoParam.e_Sys62 != e_FirmWareVer ){
		m_stSysStatInfo.st_SystemInfoParam.e_Sys62 = e_FirmWareVer;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SYS_62;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_SystemInfoParam.e_Sys62 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}

	// DSC1基板装着状態.
	b_Val = pc_SystemInfo->getDSC1Exist();
	if( m_stSysStatInfo.st_SystemInfoParam.b_Sys67 != b_Val ){
		m_stSysStatInfo.st_SystemInfoParam.b_Sys67 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SYS_67;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_SystemInfoParam.b_Sys67 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// DSC2基板装着状態.
	b_Val = pc_SystemInfo->getDSC2Exist();
	if( m_stSysStatInfo.st_SystemInfoParam.b_Sys68 != b_Val ){
		m_stSysStatInfo.st_SystemInfoParam.b_Sys68 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SYS_68;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_SystemInfoParam.b_Sys68 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}

	// Bluetooth機能設定.
	b_Val = pc_SystemInfo->getBluetoothFunc();
	if( m_stSysStatInfo.st_SystemInfoParam.b_Sys71 != b_Val ){
		m_stSysStatInfo.st_SystemInfoParam.b_Sys71 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SYS_71;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_SystemInfoParam.b_Sys71 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}

	// ＦＡＸ１有り.
	b_Val = pc_SystemInfo->getFax1Exist();
	if( m_stSysStatInfo.st_SystemInfoParam.b_Sys83 != b_Val ){
		m_stSysStatInfo.st_SystemInfoParam.b_Sys83 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SYS_83;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_SystemInfoParam.b_Sys83 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// ＦＡＸ２有り.
	b_Val = pc_SystemInfo->getFax2Exist();
	if( m_stSysStatInfo.st_SystemInfoParam.b_Sys84 != b_Val ){
		m_stSysStatInfo.st_SystemInfoParam.b_Sys84 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SYS_84;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_SystemInfoParam.b_Sys84 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}

	// 外部Ｉ／Ｆ選択 取得.
	TYPE_ExternalIFMode e_ExternalIFMode = pc_SystemInfo->getExternalIFMode();
	if( m_stSysStatInfo.st_SystemInfoParam.e_Sys92 != e_ExternalIFMode ){
		m_stSysStatInfo.st_SystemInfoParam.e_Sys92  = e_ExternalIFMode;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SYS_92;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_SystemInfoParam.e_Sys92 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}

	SST_LOG2(MFPE_LogLevelAll, "End.\n", NULL);
}

//************************************************************************************************************************.
//概要　　：APIC_SystemInfoObserver::dataChanged情報取得処理 (デバイス情報).
//戻り値　：無し.
//備考　　：特に無し.
//作成者　：高橋.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
// 2012. 5.15 [CQ363516] パフォーマンス改善 Observerで検出可能な情報をポーリング→Observerに変更.
// 2012. 5.15 [CQ363516] パフォーマンス改善 ポーリング周期を500msと2sに変更.
//************************************************************************************************************************.
void SSTC_SystemStatusService::getSystemInfoObserverDeviceInfo()
{
	SST_LOG(MFPE_LogLevelAll);
	SSTS_ChangedInfo stChangedInfo;
	ushort us_CommIdx;
	
	APIC_FaxDevice* pc_FaxDevice = APIC_FaxDevice::newInstance();
	
	// ボードのRomVersion.
	static TYPE_CommLine e_CommLineTable[] = {
		TYPE_CommLine1,							// 回線指定（１宛先時の回線指定）：回線１(主回線）.
		TYPE_CommLine2,							// 回線指定（１宛先時の回線指定）：回線２(副回線）.
		// UV-00877 help add start.
		TYPE_CommLine3,							// 回線指定（１宛先時の回線指定）：回線３.
		TYPE_CommLine4,							// 回線指定（１宛先時の回線指定）：回線４.
		// UV-00877 help add end.
	};
	struct FAXS_RomVersion st_FaxRomVer;
	Bool b_UpdFlg = False;
	Bool b_Val    = False;
	for( us_CommIdx = 0; us_CommIdx < sizeof( e_CommLineTable ) / sizeof( TYPE_CommLine ) ; ++us_CommIdx ){
		b_Val = pc_FaxDevice->getRomVersion( &st_FaxRomVer, e_CommLineTable[ us_CommIdx ] );
		if( b_Val && memcmp( &(m_stSysStatInfo.st_DeviceInfoParam.st_Dev82[ us_CommIdx ]), &st_FaxRomVer, sizeof(struct FAXS_RomVersion) ) != 0 ){
			m_stSysStatInfo.st_DeviceInfoParam.st_Dev82[ us_CommIdx ] = st_FaxRomVer;
			if( b_UpdFlg == False ){
				b_UpdFlg = True;
			}
		}
	}
	if( b_UpdFlg ){
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_82;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_DeviceInfoParam.st_Dev82[ 0 ]);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// ボードのモデル名.
	struct FAXS_ModelNo st_FaxModelNo;
	b_UpdFlg = False;
	for( us_CommIdx = 0; us_CommIdx < sizeof( e_CommLineTable ) / sizeof( TYPE_CommLine ) ; ++us_CommIdx ){
		b_Val = pc_FaxDevice->getModelNumber( &st_FaxModelNo, e_CommLineTable[ us_CommIdx ] );
		if( b_Val && memcmp( &(m_stSysStatInfo.st_DeviceInfoParam.st_Dev83[ us_CommIdx ]), &st_FaxModelNo, sizeof(struct FAXS_ModelNo) ) != 0 ){
			m_stSysStatInfo.st_DeviceInfoParam.st_Dev83[ us_CommIdx ] = st_FaxModelNo;
			if( b_UpdFlg == False ){
				b_UpdFlg = True;
			}
		}
	}
	if( b_UpdFlg ){
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_83;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_DeviceInfoParam.st_Dev83[ 0 ]);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	SST_LOG2(MFPE_LogLevelAll, "End.\n", NULL);
}

//************************************************************************************************************************.
//概要　　：APIC_SystemSettingObserver::dataChanged情報取得処理.
//戻り値　：無し.
//備考　　：特に無し.
//作成者　：ADS.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
// 2012. 8.31 [CQ371105] 静的解析 APIC_SystemSetting::newInstance()はNULLリターンしないため、NULLチェックを外す.
//************************************************************************************************************************.
void SSTC_SystemStatusService::getSystemSettingObserverData()
{
	SST_LOG(MFPE_LogLevelAll);
	SSTS_ChangedInfo stChangedInfo;

	// 認証不成功時に操作禁止とする回数.
	APIC_SystemSetting* pc_SystemSetting = APIC_SystemSetting::newInstance();
	char sc_Val = pc_SystemSetting->getDisableUserLoginCnt();
	if( m_stSysStatInfo.st_SystemInfoParam.sc_Sys79 != sc_Val ){
		m_stSysStatInfo.st_SystemInfoParam.sc_Sys79  = sc_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SYS_79;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_SystemInfoParam.sc_Sys79 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}

	// 機械状態通知管理者.
	char sc_MacInfo[ TYPD_SSTS_MAX_StatusNoticeMailAdrs ];
	Bool b_UpdFlg = False;
	for( short ss_IdCnt = 1; ss_IdCnt <= TYPD_SSTS_NotifyEmailAdminIdMax; ++ss_IdCnt ){
		SYSF_bzero( sc_MacInfo, sizeof( sc_MacInfo ) );
		pc_SystemSetting->getStatusNoticeMailAdrs( sc_MacInfo, ss_IdCnt );
		if( memcmp( sc_MacInfo, m_stSysStatInfo.st_SystemInfoParam.uc_Sys81[ ss_IdCnt - 1 ], sizeof( sc_MacInfo ) ) != 0 ){
			memcpy( m_stSysStatInfo.st_SystemInfoParam.uc_Sys81[ ss_IdCnt - 1 ], sc_MacInfo, sizeof( sc_MacInfo ) );
			b_UpdFlg = True;
		}
	}
	if( b_UpdFlg ){
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SYS_81;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_SystemInfoParam.uc_Sys81[ 0 ] );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}

	// 管理者のメールアドレス.
	char sc_MailAdr[ TYPD_SSTS_AdminMailAddress ];
	SYSF_bzero( sc_MailAdr, sizeof( sc_MailAdr ) );
	pc_SystemSetting->getAdminMailAddress( sc_MailAdr );
	if( memcmp( sc_MailAdr, m_stSysStatInfo.st_SystemInfoParam.uc_Sys82, sizeof( sc_MailAdr ) ) != 0 ){
		memcpy( m_stSysStatInfo.st_SystemInfoParam.uc_Sys82, sc_MailAdr, sizeof( sc_MailAdr ) );
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SYS_82;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_SystemInfoParam.uc_Sys82 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}

	// 長尺プリントのON/OFF設定 取得.
	Bool b_Val = pc_SystemSetting->getLongPicturePrint();
	if( m_stSysStatInfo.st_SystemInfoParam.b_Sys107 != b_Val ){
		m_stSysStatInfo.st_SystemInfoParam.b_Sys107  = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SYS_107;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_SystemInfoParam.b_Sys107 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}

	// 通信履歴表示設定 取得.
	b_Val = pc_SystemSetting->getEnableCommunicationsRecord();
	if( m_stSysStatInfo.st_SystemInfoParam.b_Sys108 != b_Val ){
		m_stSysStatInfo.st_SystemInfoParam.b_Sys108  = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SYS_108;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_SystemInfoParam.b_Sys108 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	// Job実行可否対応 2011.11.22 ADS K.Tsuboi End.

// ZeusSZX0 CQ517818 s.
	b_Val = pc_SystemSetting->getEnableUSBConnectAllService();
	if( m_stSysStatInfo.st_SystemInfoParam.b_Sys113 != b_Val ){
		m_stSysStatInfo.st_SystemInfoParam.b_Sys113  = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SYS_113;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_SystemInfoParam.b_Sys113 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
// ZeusSZX0 CQ517818 e.
	SST_LOG2(MFPE_LogLevelAll, "End.\n", NULL);
}

//************************************************************************************************************************.
//概要　　：APIC_SystemStatusObserver::dataChanged情報取得処理.
//戻り値　：無し.
//備考　　：特に無し.
//作成者　：ADS.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
// 2012. 8.31 [CQ371105] 静的解析 APIC_SystemStatus::newInstance()はNULLリターンしないため、NULLチェックを外す.
//************************************************************************************************************************.
void SSTC_SystemStatusService::getSystemStatusObserverData()
{
	SST_LOG(MFPE_LogLevelAll);
	SSTS_ChangedInfo stChangedInfo;

	// TYPE_NTF_DataID_SYS_8.
	static TYPS_SysStatusSystem st_Info;
	SSTC_ConcreteSystemStatusObserver::getSystemData(st_Info);
	if (memcmp( &m_stSysStatInfo.st_SystemInfoParam.st_Sys8, &st_Info, sizeof( st_Info ) ) != 0 ) {
		memcpy(&m_stSysStatInfo.st_SystemInfoParam.st_Sys8, &st_Info, sizeof(st_Info));
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SYS_8;
		stChangedInfo.p_Value  = &(m_stSysStatInfo.st_SystemInfoParam.st_Sys8);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}

	APIC_SystemStatus *pc_SystemStatus = APIC_SystemStatus::newInstance();
	// プログラムジョブ用Inputステータス.

	TYPE_ProgJobInStatus e_ISts  = pc_SystemStatus->getProgJobInStatus();
	if( m_stSysStatInfo.st_SystemInfoParam.e_Sys75 != e_ISts ){
		m_stSysStatInfo.st_SystemInfoParam.e_Sys75 = e_ISts;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SYS_75;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_SystemInfoParam.e_Sys75 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}

	// プログラムジョブ用Outputステータス.
	TYPE_ProgJobOutStatus e_OSts = pc_SystemStatus->getProgJobOutStatus();
	if( m_stSysStatInfo.st_SystemInfoParam.e_Sys76 != e_OSts ){
		m_stSysStatInfo.st_SystemInfoParam.e_Sys76 = e_OSts;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SYS_76;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_SystemInfoParam.e_Sys76 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}

	// Job実行可否対応 2011.11.22 ADS K.Tsuboi Start.
	// サービスモード.
	Bool b_Val = pc_SystemStatus->getServiceModeOn();
	if( m_stSysStatInfo.st_SystemInfoParam.b_Sys96 != b_Val ){
		m_stSysStatInfo.st_SystemInfoParam.b_Sys96  = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SYS_96;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_SystemInfoParam.b_Sys96 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	// 管理者モード中（使用アプリ）.
	TYPE_AdminModeApp e_Sys97 = pc_SystemStatus->getAdminModeOnApp();
	if( m_stSysStatInfo.st_SystemInfoParam.e_Sys97 != e_Sys97 ){
		m_stSysStatInfo.st_SystemInfoParam.e_Sys97  = e_Sys97;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SYS_97;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_SystemInfoParam.e_Sys97 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	// キーオペレータモード中.
	b_Val = pc_SystemStatus->getKeyOperatorModeOn();
	if( m_stSysStatInfo.st_SystemInfoParam.b_Sys98 != b_Val ){
		m_stSysStatInfo.st_SystemInfoParam.b_Sys98  = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SYS_98;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_SystemInfoParam.b_Sys98 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	// ユーザチョイス変更設定中.
	b_Val = pc_SystemStatus->getUserChoiceChanging();
	if( m_stSysStatInfo.st_SystemInfoParam.b_Sys99 != b_Val ){
		m_stSysStatInfo.st_SystemInfoParam.b_Sys99  = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SYS_99;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_SystemInfoParam.b_Sys99 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	// 割込コピー中.
	b_Val = pc_SystemStatus->getIntCopy();
	if( m_stSysStatInfo.st_SystemInfoParam.b_Sys100 != b_Val ){
		m_stSysStatInfo.st_SystemInfoParam.b_Sys100  = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SYS_100;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_SystemInfoParam.b_Sys100 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}

	// 一時切り離しON/OFF.
	Bool b_Sys101 = False;
	Bool b_Sys102 = False;
	for( ushort us_Idx = 0; us_Idx < (ushort)TYPE_TemporaryRescueUnitMax; ++us_Idx ){
		// 一時切り離しON/OFF（一時切り離し対象指定）.
		b_Val = pc_SystemStatus->getTemporaryRescueUserOn((TYPE_TemporaryRescueUnit)us_Idx);
		if( m_stSysStatInfo.st_SystemInfoParam.b_Sys101[us_Idx] != b_Val ){
			m_stSysStatInfo.st_SystemInfoParam.b_Sys101[us_Idx]  = b_Val;
			b_Sys101 = True;
		}

		// 定常切り離しON/OFF（一時切り離し対象指定）.
		b_Val = pc_SystemStatus->getTemporaryRescueServiceOn((TYPE_TemporaryRescueUnit)us_Idx);
		if( m_stSysStatInfo.st_SystemInfoParam.b_Sys102[us_Idx] != b_Val ){
			m_stSysStatInfo.st_SystemInfoParam.b_Sys102[us_Idx]  = b_Val;
			b_Sys102 = True;
		}
	}
	if( b_Sys101 ){
		// 一時切り離しON/OFF（一時切り離し対象指定）.
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SYS_101;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_SystemInfoParam.b_Sys101[ 0 ] );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	if( b_Sys102 ){
		// 定常切り離しON/OFF（一時切り離し対象指定）.
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SYS_102;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_SystemInfoParam.b_Sys102[ 0 ] );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}

	// Ｔｗａｉｎロック状態.
	TYPE_TwainLock e_Sys104 = pc_SystemStatus->getTwainLock();
	if( m_stSysStatInfo.st_SystemInfoParam.e_Sys104 != e_Sys104 ){
		m_stSysStatInfo.st_SystemInfoParam.e_Sys104  = e_Sys104;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SYS_104;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_SystemInfoParam.e_Sys104 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	// デバイスロック中.
	b_Val = pc_SystemStatus->getDeviceLockOn();
	if( m_stSysStatInfo.st_SystemInfoParam.b_Sys105 != b_Val ){
		m_stSysStatInfo.st_SystemInfoParam.b_Sys105  = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SYS_105;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_SystemInfoParam.b_Sys105 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
//	2017/03/17 IT5_4.0 1017494 Sky嵯峨山 ST.
//	Bootシーケンス診断または、自動自己診断時には、LocalUIへ通知しないように検知を抑制.
	Bool b_BootSelfCheckMode = APIC_SystemInfo::newInstance()->isBootSelfCheckMode();
	Bool b_AutoSelfCheckOn = APIC_SystemStatus::newInstance()->getAutoSelfCheckOn();
	if ( False == b_BootSelfCheckMode && ( False == b_AutoSelfCheckOn ) ) {
		// FATALが発生しているか.
		b_Val = pc_SystemStatus->isOccurFatal();
		if( m_stSysStatInfo.st_SystemInfoParam.b_Sys106 != b_Val ){
			m_stSysStatInfo.st_SystemInfoParam.b_Sys106  = b_Val;
			stChangedInfo.e_DataID = TYPE_NTF_DataID_SYS_106;
			stChangedInfo.p_Value  = &( m_stSysStatInfo.st_SystemInfoParam.b_Sys106 );
			// 2021/10/22 APF OP_BTS-35403 start
			//m_vChangedInfo.push_back(stChangedInfo);
			AddChangedInfo(&stChangedInfo);
			// 2021/10/22 APF OP_BTS-35403 end
// 2016/09/02 APF 制御構成変更No51 制御トラブル検出時のHW自己診断自動化 s.
		// 共有メモリ上の情報(FATALコード、Fatalレベル、自動自己診断実施判定結果)を更新しておく.
			m_stSysStatInfo.st_SystemInfoParam.st_Sys3.us_FatalCode = ((TYPC_FatalCommErrorJamCode)pc_SystemStatus->getFatal()).us_Code;
			m_stSysStatInfo.st_SystemInfoParam.st_Sys3.e_FatalLevel = pc_SystemStatus->getFatalLevel();
			m_stSysStatInfo.st_SystemInfoParam.st_Sys3.b_FatalCancellation = pc_SystemStatus->isFatalCancellation();
			TYPS_FatalInfoForAutoSelfCheck st_FatalInfoForAutoSelfCheck = {
				TYPE_FatalOccurredKindSys,
				m_stSysStatInfo.st_SystemInfoParam.st_Sys3.us_FatalCode,
				m_stSysStatInfo.st_SystemInfoParam.st_Sys3.e_FatalLevel
			};
			m_stSysStatInfo.st_SystemInfoParam.st_Sys3.b_AutoSelfCheck = pc_SystemStatus->judgeAutoSelfCheck( &st_FatalInfoForAutoSelfCheck );
		// 共有メモリの更新は、TYPE_NTF_DataID_SYS_106の通知により、procNotifyChangedData()でGroup04の情報がまとめて処理されるため、以下データ変化通知はしない.
		// stChangedInfo.e_DataID = TYPE_NTF_DataID_SYS_3;
		// stChangedInfo.p_Value  = &( m_stSysStatInfo.st_SystemInfoParam.st_Sys3 );
		// m_vChangedInfo.push_back(stChangedInfo);
// 2016/09/02 APF 制御構成変更No51 制御トラブル検出時のHW自己診断自動化 e.
#if defined(DEF_A4_Cus) && defined(DEF_KRONOSS_Cus)
			call_NPAG_setChagneSysFatal = True;
#endif // defined(DEF_A4_Cus) && defined(DEF_KRONOSS_Cus).
		}
#if defined(DEF_A4_Cus) && defined(DEF_KRONOSS_Cus)
		if( True == call_NPAG_setChagneSysFatal )
		{
			// for sys:NPA.
			call_NPAG_setChagneSysFatal = False;
			NPAG_setChagneSysFatal( m_stSysStatInfo.st_SystemInfoParam.st_Sys3 );
		}
#endif // defined(DEF_A4_Cus) && defined(DEF_KRONOSS_Cus).
	}
//	2017/03/17 IT5_4.0 1017494 Sky嵯峨山 ED.

	// Job実行可否対応 2011.11.22 ADS K.Tsuboi End.
//2014.03.13 OPS1(MSE) 野口 ジョブ実行中の管理者ログイン 新API化対応 ADD ST.
	// 管理者ログイン先情報.
	TYPE_AdminLoginMode e_Sys112 = pc_SystemStatus->getAdminLoginMode();
	if( m_stSysStatInfo.st_SystemInfoParam.e_Sys112 != e_Sys112 ){
		m_stSysStatInfo.st_SystemInfoParam.e_Sys112  = e_Sys112;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SYS_112;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_SystemInfoParam.e_Sys112 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
//2014.03.13 OPS1(MSE) 野口 ジョブ実行中の管理者ログイン 新API化対応 ADD ED.

// comment [特ROM対応][IT5-148]【SOLIMP3様】音声操作によるジョブ実行(IWS連携)要望_最終版リリース ->.
	// IWS_WebAPIからのジョブ投入.
	b_Val = pc_SystemStatus->getIWSRemoteJobEntry();
	if( m_stSysStatInfo.st_SystemInfoParam.b_Sys118 != b_Val ){
		m_stSysStatInfo.st_SystemInfoParam.b_Sys118  = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SYS_118;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_SystemInfoParam.b_Sys118 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
// comment [特ROM対応][IT5-148]【SOLIMP3様】音声操作によるジョブ実行(IWS連携)要望_最終版リリース <-.

// 2020/12/07 APF OP_BTS-23544_Add NFCデバイス異常警告追加対応 Start.
	TYPE_NFCDeviceStatus b_WrnNfc1 = pc_SystemStatus->getNFCDeviceErrorStatus();
	if( m_stSysStatInfo.st_WarningInfoParam.st_NfcWarningInfoParam.b_WrnNfc1 != b_WrnNfc1 ){
		m_stSysStatInfo.st_WarningInfoParam.st_NfcWarningInfoParam.b_WrnNfc1  = b_WrnNfc1;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_WRN_NFC_1;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_WarningInfoParam.st_NfcWarningInfoParam.b_WrnNfc1 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
// 2020/12/07 APF OP_BTS-23544_Add NFCデバイス異常警告追加対応 End.

	SST_LOG2(MFPE_LogLevelAll, "End.\n", NULL);
}

//************************************************************************************************************************.
//概要　　：APIC_SystemStatusObserver::dataChanged情報取得処理 (デバイス情報_プリンタ).
//戻り値　：無し.
//備考　　：特に無し.
//作成者　：ADS.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//************************************************************************************************************************.
void SSTC_SystemStatusService::getSystemStatusObserverPrinterDeviceInfo()
{
	SST_LOG(MFPE_LogLevelAll);
	SSTS_ChangedInfo stChangedInfo;
	
	// シフト機能.
	APIC_PrinterSystem *pc_PrinterSystem   = APIC_PrinterSystem::newInstance();
	Bool b_Val = pc_PrinterSystem->isShiftFunction();
	if( m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt137 != b_Val){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt137 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_137;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt137);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}

	SST_LOG2(MFPE_LogLevelAll, "End.\n", NULL);
}

//************************************************************************************************************************.
//概要　　：APIC_DebugLogSystemSettingObserver::dataChanged情報取得処理.
//戻り値　：無し.
//備考　　：特に無し.
//作成者　：ADS.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//************************************************************************************************************************.
void SSTC_SystemStatusService::getDebugLogSystemSettingObserverData()
{
	SST_LOG(MFPE_LogLevelAll);
	SSTS_ChangedInfo stChangedInfo;

	// HDD関連通知対応 2011.06.30 Start T.Takahashi.
	// HDD蓄積ログ暗号化パスワード取得.
	uchar uc_Password[ TYPD_MAX_LogEncryptionPassword ];
	memset( uc_Password, (char)SSTD_ZERO_OUT, TYPD_MAX_LogEncryptionPassword );
	APIC_DebugLogSystemSetting* pc_DebugLogSystemSetting = APIC_DebugLogSystemSetting::newInstance();
	pc_DebugLogSystemSetting->getHDDLogEncryptionPassword( reinterpret_cast<char *>( uc_Password ) );
	if( memcmp( m_stSysStatInfo.st_SystemInfoParam.uc_Sys93, uc_Password, TYPD_MAX_LogEncryptionPassword ) != 0 ){
		memcpy( m_stSysStatInfo.st_SystemInfoParam.uc_Sys93, uc_Password, TYPD_MAX_LogEncryptionPassword );
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SYS_93;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_SystemInfoParam.uc_Sys93 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	// HDD関連通知対応 2011.06.30 End   T.Takahashi.
	
// Eagle:Sparrow:制御構成変更_No.91_ハードキーレス対応 start.
	Bool b_TakeLogPermissionSetting = pc_DebugLogSystemSetting->getTakeLogPermissionSetting();
	if( m_stSysStatInfo.st_SystemInfoParam.b_Sys115 != b_TakeLogPermissionSetting ) {
		m_stSysStatInfo.st_SystemInfoParam.b_Sys115 = b_TakeLogPermissionSetting;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SYS_115;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_SystemInfoParam.b_Sys115 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
// Eagle:Sparrow:制御構成変更_No.91_ハードキーレス対応 end.

	SST_LOG2(MFPE_LogLevelAll, "End.\n", NULL);
}

//************************************************************************************************************************.
//概要　　：APIC_UserAuthenticationObserver::dataChanged情報取得処理.
//戻り値　：無し.
//備考　　：特に無し.
//作成者　：ADS.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//************************************************************************************************************************.
void SSTC_SystemStatusService::getUserAuthenticationObserverData()
{
	SST_LOG(MFPE_LogLevelAll);
	SSTS_ChangedInfo stChangedInfo;

	APIC_UserAuthentication *pc_UserAuthentication = APIC_UserAuthentication::newInstance();
	if( pc_UserAuthentication != NULL ){
		// ユーザー認証設定 取得.
		TYPE_UserAuthentication e_Val = pc_UserAuthentication->getUserAuthentication();
		if( m_stSysStatInfo.st_SystemInfoParam.e_Sys109 != e_Val ){
			m_stSysStatInfo.st_SystemInfoParam.e_Sys109  = e_Val;
			stChangedInfo.e_DataID = TYPE_NTF_DataID_SYS_109;
			stChangedInfo.p_Value  = &( m_stSysStatInfo.st_SystemInfoParam.e_Sys109 );
			// 2021/10/22 APF OP_BTS-35403 start
			//m_vChangedInfo.push_back(stChangedInfo);
			AddChangedInfo(&stChangedInfo);
			// 2021/10/22 APF OP_BTS-35403 end
		}
	}

	SST_LOG2(MFPE_LogLevelAll, "End.\n", NULL);
}

//************************************************************************************************************************.
//概要　　：APIC_ScanJobSettingObserver::dataChanged情報取得処理.
//戻り値　：無し.
//備考　　：特に無し.
//作成者　：ADS.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//************************************************************************************************************************.
void SSTC_SystemStatusService::getScanJobSettingObserverData()
{
	SST_LOG(MFPE_LogLevelAll);
	SSTS_ChangedInfo stChangedInfo;

	// 新API対応：2011.05.26 <ADD> Start T.Takahashi.
	
	// デフォルトMail/I-Faxタイトル番号.
	APIC_ScanJobSetting *pScanJobSetting  = APIC_ScanJobSetting::newInstance();
	char sc_Val = pScanJobSetting->getDefaultMailSubjectNo();
	if( m_stSysStatInfo.st_ScannerInfoParam.sc_Scn29 != sc_Val ){
		m_stSysStatInfo.st_ScannerInfoParam.sc_Scn29  = sc_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SCN_29;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_ScannerInfoParam.sc_Scn29 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// Mail/I-Fax用タイトル.
	
	TYPC_MailSubject c_MailSubject;
	Bool b_UpdFlg = False;
	for( short ss_Index = 1; ss_Index <= TYPD_SSTS_MailSubject; ++ss_Index ){
		c_MailSubject = pScanJobSetting->getMailSubject( ss_Index );
		if( memcmp( m_stSysStatInfo.st_ScannerInfoParam.st_Scn30[ ss_Index - 1 ].sc_NameUTF8,    c_MailSubject.sc_NameUTF8,    TYPD_SSTS_MailNameLength ) != 0 ||
			memcmp( m_stSysStatInfo.st_ScannerInfoParam.st_Scn30[ ss_Index - 1 ].sc_SubjectUTF8, c_MailSubject.sc_SubjectUTF8, TYPD_SSTS_MailSubjectLength ) != 0 ){
			memcpy( m_stSysStatInfo.st_ScannerInfoParam.st_Scn30[ ss_Index - 1 ].sc_NameUTF8,    c_MailSubject.sc_NameUTF8,    TYPD_SSTS_MailNameLength );
			memcpy( m_stSysStatInfo.st_ScannerInfoParam.st_Scn30[ ss_Index - 1 ].sc_SubjectUTF8, c_MailSubject.sc_SubjectUTF8, TYPD_SSTS_MailSubjectLength );
			b_UpdFlg = True;
		}
	}
	if( b_UpdFlg ){
		stChangedInfo.e_DataID = TYPE_NTF_DataID_SCN_30;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_ScannerInfoParam.st_Scn30);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// 新API対応：2011.05.26 <ADD> End T.Takahashi.

	SST_LOG2(MFPE_LogLevelAll, "End.\n", NULL);
}

//************************************************************************************************************************.
//概要　　：APIC_PrinterSettingObserver::dataChanged情報取得処理.
//戻り値　：無し.
//備考　　：特に無し.
//作成者　：ADS.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//************************************************************************************************************************.
void SSTC_SystemStatusService::getPrinterSettingObserverData()
{
	SST_LOG(MFPE_LogLevelAll);
	SSTS_ChangedInfo stChangedInfo;

	APIC_PrinterInfo* pc_PrinterInfo = APIC_PrinterInfo::newInstance();
	Bool b_DevPrt147 = False;
	for( ushort us_Idx = 0; us_Idx < sizeof( e_PaperTypeTable ) / sizeof( TYPE_PaperType ) ; ++us_Idx ){
		// 先端画像欠損幅（ｍｍ単位）.
		float sf_Val = pc_PrinterInfo->getTopMargine( e_PaperTypeTable[ us_Idx ] );
		if( m_stSysStatInfo.st_PrinterDeviceInfoParam.sf_DevPrt147[ (ushort)e_PaperTypeTable[ us_Idx ] ] != sf_Val ){
			m_stSysStatInfo.st_PrinterDeviceInfoParam.sf_DevPrt147[ (ushort)e_PaperTypeTable[ us_Idx ] ] = sf_Val;
			b_DevPrt147 = True;
		}
	}

	if( b_DevPrt147 ){
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_147;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.sf_DevPrt147[ 0 ]);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}

	SST_LOG2(MFPE_LogLevelAll, "End.\n", NULL);
}

//************************************************************************************************************************.
//概要　　：APIC_PrinterStatusObserver::dataChanged情報取得処理.
//戻り値　：無し.
//備考　　：特に無し.
//作成者　：ADS.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//************************************************************************************************************************.
void SSTC_SystemStatusService::getPrinterStatusObserverData()
{
	SST_LOG(MFPE_LogLevelAll);
	SSTS_ChangedInfo stChangedInfo;

	static Bool b_PrevInitializing = True;	// 前回の初期化中状態 [初期値:True].
	static Bool b_PrevWarmingUp = True;	// 前回のウォームアップ中状態 [初期値:True].
	static Bool b_Check = True;	// 検出実行フラグ [初期値:True].

	APIC_PrinterStatus* pc_APIPrinterStatus = APIC_PrinterStatus::newInstance();

	// クリーニング中.
	Bool b_Val = pc_APIPrinterStatus->isCleaning();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt66 != b_Val){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt66 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_66;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt66);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// エンジン環境通知情報_フルカラー禁止状態送信処理.
	b_Val = pc_APIPrinterStatus->isColorPrintDisable();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt67 != b_Val){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt67 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_67;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt67);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// ドラムドライ中.
	b_Val = pc_APIPrinterStatus->isDrumDryExec();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt68 != b_Val){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt68 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_68;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt68);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// ：強制トナー補給中.
	b_Val = pc_APIPrinterStatus->isForceSupply();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt69 != b_Val){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt69 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_69;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt69);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// 画像安定化中.
	b_Val = pc_APIPrinterStatus->isImageKeeping();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt70 != b_Val){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt70 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_70;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt70);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// 初期化中.
	Bool b_Initializing = pc_APIPrinterStatus->isInitializing();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt71 != b_Initializing){
		// 初期化完了で、FKE値を更新する.
		if ( b_Initializing == False ) {
			getEnablerSettingObserverData();
		}

		m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt71 = b_Initializing;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_71;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt71);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// PIマニュアル動作中.
	b_Val = pc_APIPrinterStatus->isPIActive();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt72 != b_Val){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt72 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_72;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt72);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}

	// ウォームアップ中.
	Bool b_WarmingUp = pc_APIPrinterStatus->isWarmingUp();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt74 != b_WarmingUp){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt74 = b_WarmingUp;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_74;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt74);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	SST_LOG2(MFPE_LogLevelTrace, "isInitializing=(%d->%d), isWarmingUp=(%d->%d).\n", b_PrevInitializing, b_Initializing, b_PrevWarmingUp, b_WarmingUp);

//Str【MinervaSBK : CQ520910(定着ウェイト対応) : D.Ujiie : 2015.11.04】.
	// 定着ウェイト中.
	Bool b_FuserWaiting = pc_APIPrinterStatus->isFuserWaiting();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt153 != b_FuserWaiting){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt153 = b_FuserWaiting;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_153;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt153);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
//End【MinervaSBK : CQ520910(定着ウェイト対応) : D.Ujiie : 2015.11.04】.

	if ( ( (b_PrevInitializing == True) && (b_Initializing == False) )	// 初期化完了時.
			|| ( (b_PrevWarmingUp == True) && (b_WarmingUp == False) ) ) {	// ウォームアップ完了時.
		b_Check = True;	// 検出実行フラグを設定.
	}
	b_PrevInitializing = b_Initializing;	// 初期化中状態退避.
	b_PrevWarmingUp = b_WarmingUp;	// ウォームアップ中状態退避.

	if (b_Check == True) {
		b_Check = False;	// 検出実行フラグをクリア.

		// デバイス情報_プリンタの変化点検出を行う.
		getPrinterStatusObserverPrinterDeviceInfo();
		getMachineSettingObserverPrinterDeviceInfo();
		getSystemStatusObserverPrinterDeviceInfo();

	}

	SST_LOG2(MFPE_LogLevelAll, "End.\n", NULL);
}

//************************************************************************************************************************.
//概要　　：APIC_PrinterStatusObserver::dataChanged情報取得処理 (デバイス情報_プリンタ).
//戻り値　：無し.
//備考　　：特に無し.
//作成者　：ADS.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//************************************************************************************************************************.
void SSTC_SystemStatusService::getPrinterStatusObserverPrinterDeviceInfo()
{
	SST_LOG(MFPE_LogLevelAll);
	SSTS_ChangedInfo stChangedInfo;

	// 長長尺紙可能モード判定処理.
	Bool b_Val = APIC_PrinterWarning::newInstance()->isEnableLongPrintMode();
	if(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt84 != b_Val){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt84 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_84;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt84);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// STEP2：2010.12.10 <CHG> Start   T.Takahashi.
	//   同一のテーブルをパラメータに使用しているＡＰＩを一箇所に纏めた.
	//   STEP2での追加されたDataIDは 120.
	
	APIC_PrinterSystem *pc_PrinterSystem   = APIC_PrinterSystem::newInstance();
	APIC_PrinterWarning *pc_PrinterWarning = APIC_PrinterWarning::newInstance();
	Bool b_EnableLongPrintPaperTypeFlg     = False;
	Bool b_LongPicturePaperTypeFlg         = False;
	for( ushort us_Idx = 0; us_Idx < sizeof( e_PaperTypeTable ) / sizeof( TYPE_PaperType ) ; ++us_Idx ){
		// 長尺紙可能用紙種類判定処理.
		Bool b_Val = pc_PrinterWarning->isEnableLongPrintPaperType( e_PaperTypeTable[ us_Idx ] );
		if( m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt85[ (ushort)e_PaperTypeTable[ us_Idx ] ] != b_Val ){
			m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt85[ (ushort)e_PaperTypeTable[ us_Idx ] ] = b_Val;
			if( b_EnableLongPrintPaperTypeFlg == False ){
				b_EnableLongPrintPaperTypeFlg = True;
			}
		}
		// 長尺印刷可能用紙種類判定処理.
		b_Val = pc_PrinterSystem->isLongPicturePaperType( e_PaperTypeTable[ us_Idx ] );
		if( m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt120[ (ushort)e_PaperTypeTable[ us_Idx ] ] != b_Val ){
			m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt120[ (ushort)e_PaperTypeTable[ us_Idx ] ] = b_Val;
			if( b_LongPicturePaperTypeFlg == False ){
				b_LongPicturePaperTypeFlg = True;
			}
		}
	}
	if( b_EnableLongPrintPaperTypeFlg ){
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_85;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt85[ 0 ]);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}

	if( b_LongPicturePaperTypeFlg ){
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_120;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt120[ 0 ]);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}

	// 両面ユニット有り.
	b_Val = pc_PrinterSystem->isExistDuplex();
	if( m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt113 != b_Val ){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt113 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_113;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt113);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// ＬＣＴタイプ.
	TYPE_LCTType e_Lct = pc_PrinterSystem->getLCTType();
	if( m_stSysStatInfo.st_PrinterDeviceInfoParam.e_DevPrt114 != e_Lct ){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.e_DevPrt114 = e_Lct;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_114;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.e_DevPrt114);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// 手差しトレイの用紙サイズ自動検出.
	b_Val = pc_PrinterSystem->isEnableAutoSizeDetectAtManualTray();
	if( m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt115 != b_Val ) {;
		m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt115 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_115;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt115);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// Ｅｎｇｉｎｅ－Ｔｙｐｅ取得.
	TYPE_EngineType e_Eng = pc_PrinterSystem->getEngineType();
	if( m_stSysStatInfo.st_PrinterDeviceInfoParam.e_DevPrt116 != e_Eng ){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.e_DevPrt116 = e_Eng;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_116;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.e_DevPrt116);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// 長尺用紙サイズの最小値を取得.
	float sf_Val = pc_PrinterSystem->getLongPictureMinSize();
	if( m_stSysStatInfo.st_PrinterDeviceInfoParam.sf_DevPrt117 != sf_Val ){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.sf_DevPrt117 = sf_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_117;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.sf_DevPrt117);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// リストプリント用の排紙トレイを取得.
	TYPE_OutputTray e_OutTray = pc_PrinterSystem->getListPrintOutputTray();
	if( m_stSysStatInfo.st_PrinterDeviceInfoParam.e_DevPrt118 != e_OutTray ){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.e_DevPrt118 = e_OutTray;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_118;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.e_DevPrt118);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}

	// 最大通紙可能用紙サイズ.
	APIC_OutputTray *pc_OutputTray = APIC_OutputTray::newInstance();
	TYPC_Point<float> c_Point = pc_OutputTray->getMaxOutputPaperSize();
	TYPS_Point_f st_Point = { c_Point.x, c_Point.y };
	if( m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt121.sf_x != st_Point.sf_x ||
		m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt121.sf_y != st_Point.sf_y ){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt121 = st_Point;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_121;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt121);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// 最小通紙可能用紙サイズ.
	c_Point = pc_OutputTray->getMinOutputPaperSize();
	st_Point.sf_x = c_Point.x;
	st_Point.sf_y = c_Point.y;
	if( m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt122.sf_x != st_Point.sf_x || 
		m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt122.sf_y != st_Point.sf_y ){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt122 = st_Point;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_122;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt122);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}

	// 後端画像欠損幅（ｍｍ単位）.
	APIC_PrinterInfo* pc_PrinterInfo = APIC_PrinterInfo::newInstance();
	sf_Val = pc_PrinterInfo->getBottomMargine();
	if( m_stSysStatInfo.st_PrinterDeviceInfoParam.sf_DevPrt123 != sf_Val ){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.sf_DevPrt123 = sf_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_123;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.sf_DevPrt123);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// 左端画像欠損幅（ｍｍ単位）.
	sf_Val = pc_PrinterInfo->getLeftMargine();
	if( m_stSysStatInfo.st_PrinterDeviceInfoParam.sf_DevPrt124 != sf_Val ){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.sf_DevPrt124 = sf_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_124;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.sf_DevPrt124);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// 右端画像欠損幅（ｍｍ単位）.
	sf_Val = pc_PrinterInfo->getRightMargine();
	if( m_stSysStatInfo.st_PrinterDeviceInfoParam.sf_DevPrt125 != sf_Val ){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.sf_DevPrt125 = sf_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_125;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.sf_DevPrt125);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// 印刷領域の最大幅.
	sf_Val = pc_PrinterInfo->getMaxPrintableAreaWidth();
	if( m_stSysStatInfo.st_PrinterDeviceInfoParam.sf_DevPrt126 != sf_Val ){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.sf_DevPrt126 = sf_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_126;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.sf_DevPrt126);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// 印刷領域の最大長.
	sf_Val = pc_PrinterInfo->getMaxPrintableAreaHeight();
	if( m_stSysStatInfo.st_PrinterDeviceInfoParam.sf_DevPrt127 != sf_Val ){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.sf_DevPrt127 = sf_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_127;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.sf_DevPrt127);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// フィニッシャ有り.
	b_Val = pc_PrinterSystem->isExistFinisher();
	if( m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt129 != b_Val ){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt129 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_129;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt129);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// リレーユニット（ＲＵ）有り.
	b_Val = pc_PrinterSystem->isExistRUOption();
	if( m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt130 != b_Val ){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt130 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_130;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt130);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// Ｚ折りユニット（ＺＵ）有り.
	b_Val = pc_PrinterSystem->isExistZUOption();
	if( m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt131 != b_Val ){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt131 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_131;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt131);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// フィニッシャタイプ.
	TYPE_FinisherType e_FinType = pc_PrinterSystem->getFinisherType();
	if( m_stSysStatInfo.st_PrinterDeviceInfoParam.e_DevPrt132 != e_FinType ){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.e_DevPrt132 = e_FinType;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_132;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.e_DevPrt132);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// ステープル機能.
	TYPS_StapleFunction st_Steple = pc_PrinterSystem->getStapleFunction();
	if( m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt133.b_Staple1Corner     != st_Steple.b_Staple1Corner     ||
		m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt133.b_Staple2Side       != st_Steple.b_Staple2Side       ||
		m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt133.b_Staple2SideCenter != st_Steple.b_Staple2SideCenter ||
		m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt133.uc_StapleMaxNum     != st_Steple.uc_StapleMaxNum ){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt133 = st_Steple;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_133;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt133);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}

	// 紙折り機能.
	TYPS_FoldFunction st_Fold = pc_PrinterSystem->getFoldFunction();
	if( m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt136.b_FoldZshaped     != st_Fold.b_FoldZshaped     ||
		m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt136.b_FoldCenterInner != st_Fold.b_FoldCenterInner ||
		m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt136.b_FoldLetter      != st_Fold.b_FoldLetter ){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt136 = st_Fold;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_136;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt136);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}

	// メールビン装着情報.
	b_Val = pc_PrinterSystem->isOptionMailBin();
	if( m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt140 != b_Val){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt140 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_140;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt140);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// Ｚ折り内）パンチユニット（ＰＵ）有り.
	b_Val = pc_PrinterSystem->isExistPunchUnitOption();
	if( m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt141 != b_Val){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt141 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_141;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt141);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}

	// ATS/AMS対応：2011.05.10 <ADD> Start T.Takahashi.
	// パンチキット（ＰＫ）有無.
	b_Val = pc_PrinterSystem->isExistPunchKitOption();
	if( m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt148 != b_Val){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt148 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_148;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt148 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// パンチ種別.
	static TYPE_PunchMode e_PunchModeTbl[] = {
		TYPE_PunchModeNone,		// パンチモード：なし 
		TYPE_PunchMode2,		// パンチモード：2穴 
		TYPE_PunchMode3,		// パンチモード：3穴 
		TYPE_PunchMode4			// パンチモード：4穴 
	};
	ENGC_PrinterImpColor *pc_PrinterImpColor = ENGC_PrinterImpColor::newInstance();
	Bool b_UpdFlg = False;
	for( us_Idx = 0; us_Idx < sizeof( e_PunchModeTbl ) / sizeof( TYPE_PunchMode ); ++us_Idx ){
		ENGE_PunchKind e_PunchKind = pc_PrinterImpColor->getPunchKind( e_PunchModeTbl[ us_Idx ] );
		if( m_stSysStatInfo.st_PrinterDeviceInfoParam.e_DevPrt149[ (ushort)e_PunchModeTbl[ us_Idx ] ] != e_PunchKind ){
			m_stSysStatInfo.st_PrinterDeviceInfoParam.e_DevPrt149[ (ushort)e_PunchModeTbl[ us_Idx ] ]   = e_PunchKind;
			if( b_UpdFlg == False ){
				b_UpdFlg = True;
			}
		}
	}
	if( b_UpdFlg ){
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_149;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_PrinterDeviceInfoParam.e_DevPrt149[ 0 ] );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	// ATS/AMS対応：2011.05.10 <ADD> End T.Takahashi.

	SST_LOG2(MFPE_LogLevelAll, "End.\n", NULL);
}

//************************************************************************************************************************.
//概要　　：APIC_PrinterSystemObserver::dataChanged情報取得処理 (リモートパネル情報).
//戻り値　：無し.
//備考　　：特に無し.
//作成者　：ADS.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//************************************************************************************************************************.
void SSTC_SystemStatusService::getPrinterSystemObserverRemotePanelInfo()
{
	SST_LOG(MFPE_LogLevelAll);
	SSTS_ChangedInfo stChangedInfo;

	// TYPE_NTF_DataID_RPL_1.
	TYPS_SysStatusPrinterSystem st_Info;
	memset( &st_Info, SSTD_ZERO_OUT, sizeof( st_Info ));
	SSTC_ConcretePrinterSystemObserver::getPrinterSystemData(st_Info);
	if (memcmp( &m_stSysStatInfo.st_RemotePanelInfoParam.st_Rpl1, &st_Info, sizeof( st_Info ) ) != 0 ) {
		memcpy(&m_stSysStatInfo.st_RemotePanelInfoParam.st_Rpl1, &st_Info, sizeof(st_Info));
		stChangedInfo.e_DataID = TYPE_NTF_DataID_RPL_1;
		stChangedInfo.p_Value  = &(m_stSysStatInfo.st_RemotePanelInfoParam.st_Rpl1);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}

	SST_LOG2(MFPE_LogLevelAll, "End.\n", NULL);
}

//************************************************************************************************************************.
//概要　　：APIC_UsbMemDeviceInfoObserver::warningChanged情報取得処理.
//戻り値　：無し.
//備考　　：特に無し.
//作成者　：高橋.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
// 2012. 5.15 [CQ363516] パフォーマンス改善 Observerで検出可能な情報をポーリング→Observerに変更した対応で追加した関数.
//************************************************************************************************************************.
void SSTC_SystemStatusService::getUsbWarningObserverData()
{
	SST_LOG(MFPE_LogLevelAll);
	
	typedef struct st_CheckWarning {
		TYPE_UsbMemWarning			e_Usb;					// USB警告情報.
		TYPE_NTF_DataID				e_DataID;				// データID.
		Bool 				   	   *pb_Val;					// 警告の有無.
	}ST_CHK_WARNING;
	static ST_CHK_WARNING st_CheckUsbMemWarningTable[] = {
		{ TYPE_UsbMemWarningFileNumOver	, TYPE_NTF_DataID_WRN_USB_1, &( m_stSysStatInfo.st_WarningInfoParam.st_UsbWarningInfoParam.b_WrnScnUsb1 ) },	// ファイル数オーバー.
		{ TYPE_UsbMemWarningMemoryFull	, TYPE_NTF_DataID_WRN_USB_2, &( m_stSysStatInfo.st_WarningInfoParam.st_UsbWarningInfoParam.b_WrnScnUsb2 ) },	// メモリフル.
		{ TYPE_UsbMemWarningTwoInsert	, TYPE_NTF_DataID_WRN_USB_3, &( m_stSysStatInfo.st_WarningInfoParam.st_UsbWarningInfoParam.b_WrnScnUsb3 ) },	// USB２つ挿し.
		{ TYPE_UsbMemWarningPullOut		, TYPE_NTF_DataID_WRN_USB_4, &( m_stSysStatInfo.st_WarningInfoParam.st_UsbWarningInfoParam.b_WrnScnUsb4 ) },	// 書き込み中のUSBメモリ引き抜き.
		{ TYPE_UsbMemWarningAll			, TYPE_NTF_DataID_WRN_USB_5, &( m_stSysStatInfo.st_WarningInfoParam.st_UsbWarningInfoParam.b_WrnScnUsb5 ) },	// 全て.
// ADS坪井 → 2011/12/01 【パフォーマンス改善】isWarning対応 Start.
		{ TYPE_UsbMemWarningNeedPullOut	, TYPE_NTF_DataID_WRN_USB_6, &( m_stSysStatInfo.st_WarningInfoParam.st_UsbWarningInfoParam.b_WrnScnUsb6 ) },	// USBメモリ引き抜き要求状態.
		{ TYPE_UsbMemWarningDebugLog	, TYPE_NTF_DataID_WRN_USB_7, &( m_stSysStatInfo.st_WarningInfoParam.st_UsbWarningInfoParam.b_WrnScnUsb7 ) },	// デバッグログ蓄積不可状態.
// ADS坪井 → 2011/12/01 【パフォーマンス改善】isWarning対応 End.
		{ (TYPE_UsbMemWarning)0			, TYPE_NTF_DataID_End		, NULL						   												   }	// end.
	};
	SSTS_ChangedInfo stChangedInfo;
	Bool b_Val;
	
	for( ST_CHK_WARNING *pst_WarTbl = &st_CheckUsbMemWarningTable[ 0 ];  pst_WarTbl->e_DataID != TYPE_NTF_DataID_End; pst_WarTbl++ ){
		b_Val = APIC_UsbMemDeviceInfo::newInstance()->isWarning( pst_WarTbl->e_Usb );
		if( *( pst_WarTbl->pb_Val ) != b_Val ){
			*( pst_WarTbl->pb_Val ) = b_Val;
			stChangedInfo.e_DataID = pst_WarTbl->e_DataID;
			stChangedInfo.p_Value  = pst_WarTbl->pb_Val;
			// 2021/10/22 APF OP_BTS-35403 start
			//m_vChangedInfo.push_back( stChangedInfo );
			AddChangedInfo( &stChangedInfo );
			// 2021/10/22 APF OP_BTS-35403 end
		}
	}

}

// 2016/07/01 2Sys-Sky西村 RQ-436対応 start.
//************************************************************************************************************************.
//概要　　：APIC_IwsObserver::dataChanged情報取得処理.
//戻り値　：無し.
//備考　　：特に無し.
//作成者　：APF 西村.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//************************************************************************************************************************.
void SSTC_SystemStatusService::getIwsObserverData()
{
	SST_LOG(MFPE_LogLevelAll);

	typedef struct st_CheckWarning {
		TYPE_WarningType			e_Iws;					// IWS警告情報.
		TYPE_NTF_DataID				e_DataID;				// データID.
		Bool 						*pb_Val;				// 警告の有無.
	}ST_CHK_WARNING;
	static ST_CHK_WARNING st_CheckPrtWarningTable[] = {
		{ TYPE_WarningType_BackgroundDisable	,	TYPE_NTF_DataID_WRN_IWS_1	, &( m_stSysStatInfo.st_WarningInfoParam.st_IwsWarningInfoParam.b_WrnIws1 ) },	// IWSバックグラウンドアプリ使用不可.
		{ (TYPE_WarningType)0					,	TYPE_NTF_DataID_End			, NULL																		}	// end.
	};
	SSTS_ChangedInfo stChangedInfo;
	Bool b_Val;

	for( ST_CHK_WARNING *pst_WarTbl = &st_CheckPrtWarningTable[ 0 ]; pst_WarTbl->e_DataID != TYPE_NTF_DataID_End; pst_WarTbl++ ){
		b_Val = APIC_IWSAppInfo::newInstance()->isWarning( pst_WarTbl->e_Iws );
		if( *( pst_WarTbl->pb_Val ) != b_Val ){
			*( pst_WarTbl->pb_Val ) = b_Val;
			stChangedInfo.e_DataID = pst_WarTbl->e_DataID;
			stChangedInfo.p_Value  = pst_WarTbl->pb_Val;
			// 2021/10/22 APF OP_BTS-35403 start
			//m_vChangedInfo.push_back( stChangedInfo );
			AddChangedInfo( &stChangedInfo );
			// 2021/10/22 APF OP_BTS-35403 end
		}
	}
}
// 2016/07/01 2Sys-Sky西村 RQ-436対応 end.

//************************************************************************************************************************.
//概要　　：APIC_CopyJobSettingObserver::dataChanged情報取得処理.
//戻り値　：無し.
//備考　　：特に無し.
//作成者　：ADS.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//************************************************************************************************************************.
void SSTC_SystemStatusService::getCopyJobSettingObserverData()
{
	SST_LOG(MFPE_LogLevelAll);
	SSTS_ChangedInfo stChangedInfo;

	// ATS/AMS対応：2011.05.10 <ADD> Start T.Takahashi.
	
	APIC_CopyJobSetting* pc_PaperTypeOnAPS = APIC_CopyJobSetting::newInstance();
	
	// Ａ６・はがきのＡＰＳ選択設定.
	TYPE_ApsA6orPostcard e_ApsA6orPostcard = pc_PaperTypeOnAPS->getApsA6orPostcard();
	if( e_ApsA6orPostcard != m_stSysStatInfo.st_DeviceInfoParam.e_Dev85 ){
		m_stSysStatInfo.st_DeviceInfoParam.e_Dev85 = e_ApsA6orPostcard;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_85;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_DeviceInfoParam.e_Dev85 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// トレイ１・ATS/APS許可.
	Bool b_Val = pc_PaperTypeOnAPS->getAtsApsEnable_Tray_1();
	if( b_Val != m_stSysStatInfo.st_DeviceInfoParam.b_Dev86 ){
		m_stSysStatInfo.st_DeviceInfoParam.b_Dev86 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_86;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_DeviceInfoParam.b_Dev86 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// トレイ２・ATS/APS許可.
	b_Val = pc_PaperTypeOnAPS->getAtsApsEnable_Tray_2();
	if( b_Val != m_stSysStatInfo.st_DeviceInfoParam.b_Dev87 ){
		m_stSysStatInfo.st_DeviceInfoParam.b_Dev87 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_87;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_DeviceInfoParam.b_Dev87 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// トレイ３・ATS/APS許可.
	b_Val = pc_PaperTypeOnAPS->getAtsApsEnable_Tray_3();
	if( b_Val != m_stSysStatInfo.st_DeviceInfoParam.b_Dev88 ){
		m_stSysStatInfo.st_DeviceInfoParam.b_Dev88 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_88;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_DeviceInfoParam.b_Dev88 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back( stChangedInfo );
		AddChangedInfo( &stChangedInfo );
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// トレイ４・ATS/APS許可.
	b_Val = pc_PaperTypeOnAPS->getAtsApsEnable_Tray_4();
	if( b_Val != m_stSysStatInfo.st_DeviceInfoParam.b_Dev89 ){
		m_stSysStatInfo.st_DeviceInfoParam.b_Dev89 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_89;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_DeviceInfoParam.b_Dev89 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// トレイ５・ATS/APS許可.
	b_Val = pc_PaperTypeOnAPS->getAtsApsEnable_Tray_5();
	if( b_Val != m_stSysStatInfo.st_DeviceInfoParam.b_Dev90 ){
		m_stSysStatInfo.st_DeviceInfoParam.b_Dev90 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_90;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_DeviceInfoParam.b_Dev90 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// 手差し・ATS/APS許可.
	b_Val = pc_PaperTypeOnAPS->getAtsApsEnable_TrayManual();
	if( b_Val != m_stSysStatInfo.st_DeviceInfoParam.b_Dev91 ){
		m_stSysStatInfo.st_DeviceInfoParam.b_Dev91 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_91;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_DeviceInfoParam.b_Dev91 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// 給紙トレイ指定時のATS許可.
	b_Val = pc_PaperTypeOnAPS->getAtsEnableAtUserSelectTray();
	if( b_Val != m_stSysStatInfo.st_DeviceInfoParam.b_Dev92 ){
		m_stSysStatInfo.st_DeviceInfoParam.b_Dev92 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_92;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_DeviceInfoParam.b_Dev92 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// 画像回転許可モード.
	TYPE_ImageRotateEnable e_ImageRotateEnable = pc_PaperTypeOnAPS->getImageRotate();
	if( e_ImageRotateEnable != m_stSysStatInfo.st_DeviceInfoParam.e_Dev93 ){
		m_stSysStatInfo.st_DeviceInfoParam.e_Dev93 = e_ImageRotateEnable;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_93;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_DeviceInfoParam.e_Dev93 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// 小サイズ原稿.
	TYPE_SmallSizeOriginal e_SmallSizeOriginal = pc_PaperTypeOnAPS->getSmallSizeOriginal();
	if( e_SmallSizeOriginal != m_stSysStatInfo.st_DeviceInfoParam.e_Dev94 ){
		m_stSysStatInfo.st_DeviceInfoParam.e_Dev94 = e_SmallSizeOriginal;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_94;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_DeviceInfoParam.e_Dev94 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// 給紙トレイの優先順位指定.
	TYPS_TraySearchPriority st_TraySearchPriority = pc_PaperTypeOnAPS->getTraySearchPriority();
	if( 0 != memcmp( &m_stSysStatInfo.st_DeviceInfoParam.st_Dev95, &st_TraySearchPriority, sizeof( TYPS_TraySearchPriority ) ) ){
		m_stSysStatInfo.st_DeviceInfoParam.st_Dev95 = st_TraySearchPriority;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_95;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_DeviceInfoParam.st_Dev95 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// ユーザ指定用紙ＡＰＳの紙種.
	TYPS_PaperTypeOnAPS* ps_PaperTypeOnAPS = pc_PaperTypeOnAPS->getPaperTypeOnAPS();
	
	if( ps_PaperTypeOnAPS->b_TypeUser1 != m_stSysStatInfo.st_DeviceInfoParam.st_Dev96.b_TypeUser1 ||
		 ps_PaperTypeOnAPS->b_TypeUser2 != m_stSysStatInfo.st_DeviceInfoParam.st_Dev96.b_TypeUser2 ){
		m_stSysStatInfo.st_DeviceInfoParam.st_Dev96 = *ps_PaperTypeOnAPS;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_96;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_DeviceInfoParam.st_Dev96 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// ATS/AMS対応：2011.05.10 <ADD> End T.Takahashi.

	SST_LOG2(MFPE_LogLevelAll, "End.\n", NULL);
}

//************************************************************************************************************************.
//概要　　：APIC_FaxJobSettingObserver::dataChanged情報取得処理.
//戻り値　：無し.
//備考　　：特に無し.
//作成者　：ADS.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//************************************************************************************************************************.
void SSTC_SystemStatusService::getFaxJobSettingObserverData()
{
	SST_LOG(MFPE_LogLevelAll);
	SSTS_ChangedInfo stChangedInfo;

	APIC_FaxJobSetting* pc_FaxJobSetting = APIC_FaxJobSetting::newInstance();
	
	// 記録時の最小縮小率（87%～100%).
	uchar uc_Val = pc_FaxJobSetting->getApsMsg2();
	if( uc_Val != m_stSysStatInfo.st_DeviceInfoParam.uc_Dev97 ){
		m_stSysStatInfo.st_DeviceInfoParam.uc_Dev97 = uc_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_97;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_DeviceInfoParam.uc_Dev97 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// 給紙ﾄﾚｲ固定.
	TYPE_FaxPrintSet2Tray e_FaxPrintSet2Tray = pc_FaxJobSetting->getFaxPrintSet2Tray();
	if( e_FaxPrintSet2Tray != m_stSysStatInfo.st_DeviceInfoParam.e_Dev98 ){
		m_stSysStatInfo.st_DeviceInfoParam.e_Dev98 = e_FaxPrintSet2Tray;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_98;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_DeviceInfoParam.e_Dev98 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}

	// 新API対応：2011.05.26 <ADD> Start T.Takahashi.
	// iFAX機能状態.
	APIC_FaxJobSetting2 *pc_FaxJobSetting2 = APIC_FaxJobSetting2::newInstance();
	Bool b_Val = pc_FaxJobSetting2->getiFaxFuncOn();
	if( b_Val != m_stSysStatInfo.st_DeviceInfoParam.b_Dev105 ){
		m_stSysStatInfo.st_DeviceInfoParam.b_Dev105 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_105;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_DeviceInfoParam.b_Dev105 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// 新API対応：2011.05.26 <ADD> End T.Takahashi.

	SST_LOG2(MFPE_LogLevelAll, "End.\n", NULL);
}

//************************************************************************************************************************.
//概要　　：APIC_MachineSettingObserver::dataChanged情報取得処理.
//戻り値　：無し.
//備考　　：特に無し.
//作成者　：ADS.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//************************************************************************************************************************.
void SSTC_SystemStatusService::getMachineSettingObserverData()
{
	SST_LOG(MFPE_LogLevelAll);
	SSTS_ChangedInfo stChangedInfo;

	// TYPE_NTF_DataID_DEV_51.
	static bool b_first = true;
	static TYPS_Group1Param m_stG1Param;
	static TYPS_Group3Param m_stG3Param;

	if ( b_first ) {
		// 初回はコピーとフラグ変更だけして終わり.
		m_stG1Param = NVDC_MachineSetting6::newInstance()->getGroup1Param();
		m_stG3Param = NVDC_MachineSetting6::newInstance()->getGroup3Param();
		b_first = false;
	} else {
		TYPS_Group1Param st_NewParam1 = NVDC_MachineSetting6::newInstance()->getGroup1Param();
		TYPS_Group3Param st_NewParam3 = NVDC_MachineSetting6::newInstance()->getGroup3Param();

		if ( memcmp( &m_stG1Param, &st_NewParam1, sizeof(TYPS_Group1Param) ) != 0 
		  || memcmp( &m_stG3Param, &st_NewParam3, sizeof(TYPS_Group3Param) ) != 0 ) {
			// いずれかに変化があれば保持データを更新して変化通知.
			memcpy( &m_stG1Param, &st_NewParam1, sizeof(TYPS_Group1Param) );
			memcpy( &m_stG3Param, &st_NewParam3, sizeof(TYPS_Group3Param) );

			// デバイス情報_プリンタの変化点検出を行う.
			getPrinterStatusObserverPrinterDeviceInfo();
			getSystemStatusObserverPrinterDeviceInfo();

			stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_51;
			stChangedInfo.p_Value = NULL;
			// 2021/10/22 APF OP_BTS-35403 start
			//m_vChangedInfo.push_back(stChangedInfo);
			AddChangedInfo(&stChangedInfo);
			// 2021/10/22 APF OP_BTS-35403 end
		}
	}

	// 現在選択ベンダータイプ：キーデバイス.
	TYPE_KeyMode e_KeyMode = APIC_MachineSetting::newInstance()->getKeyMode();
	if( e_KeyMode != m_stSysStatInfo.st_DeviceInfoParam.e_Dev84 ){
		m_stSysStatInfo.st_DeviceInfoParam.e_Dev84 = e_KeyMode;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_84;
		stChangedInfo.p_Value = NULL;
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}

	APIC_MachineSetting* pc_MachineSetting = APIC_MachineSetting::newInstance();

	// ８Ｋタイプ.
	TYPE_8KType e_8KType = pc_MachineSetting->get8KType();
	if( e_8KType != m_stSysStatInfo.st_DeviceInfoParam.e_Dev99 ){
		m_stSysStatInfo.st_DeviceInfoParam.e_Dev99 = e_8KType;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_99;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_DeviceInfoParam.e_Dev99 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}

	// １６Ｋタイプ.
	TYPE_16KType e_16KType = pc_MachineSetting->get16KType();
	if( e_16KType != m_stSysStatInfo.st_DeviceInfoParam.e_Dev100 ){
		m_stSysStatInfo.st_DeviceInfoParam.e_Dev100 = e_16KType;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_100;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_DeviceInfoParam.e_Dev100 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// ＦＬＳタイプ.
	TYPE_FLSType e_FLSType = pc_MachineSetting->getFLSType();
	if( e_FLSType != m_stSysStatInfo.st_DeviceInfoParam.e_Dev101 ){
		m_stSysStatInfo.st_DeviceInfoParam.e_Dev101 = e_FLSType;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_101;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_DeviceInfoParam.e_Dev101 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// パンチ穴数設定.
	TYPE_PunchHoleType e_PunchHoleType = pc_MachineSetting->getPunchHoleType();
	if( e_PunchHoleType != m_stSysStatInfo.st_DeviceInfoParam.e_Dev102 ){
		m_stSysStatInfo.st_DeviceInfoParam.e_Dev102 = e_PunchHoleType;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_102;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_DeviceInfoParam.e_Dev102 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// マシン仕向.
	TYPE_SystemType e_SystemType = pc_MachineSetting->getSystemType();
	if( e_SystemType != m_stSysStatInfo.st_DeviceInfoParam.e_Dev103 ){
		m_stSysStatInfo.st_DeviceInfoParam.e_Dev103 = e_SystemType;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_103;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_DeviceInfoParam.e_Dev103 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}

	SST_LOG2(MFPE_LogLevelAll, "End.\n", NULL);
}

//************************************************************************************************************************.
//概要　　：APIC_MachineSettingObserver::dataChanged情報取得処理 (デバイス情報_プリンタ).
//戻り値　：無し.
//備考　　：特に無し.
//作成者　：ADS.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//************************************************************************************************************************.
void SSTC_SystemStatusService::getMachineSettingObserverPrinterDeviceInfo()
{
	SST_LOG(MFPE_LogLevelAll);
	SSTS_ChangedInfo stChangedInfo;

	//// PCプリント用の排紙トレイを取得.
	static TYPE_OutputTray e_OutputTrayTbl[] = {
		TYPE_OutputTrayMain,             // 排紙トレイ：メイントレイ（Ｍ：エレベートトレイ） 基本定義として基本出力モード（ステイプル・パンチ・仕分け）で出力可能なトレイ.
		TYPE_OutputTraySub1,             // 排紙トレイ：サブトレイ（Ｍ：第１トレイ） メイントレイ以外のトレイ.
		TYPE_OutputTraySub2,             // 排紙トレイ：サブトレイ（Ｍ：第３トレイ） メイントレイ以外のトレイ.
		TYPE_OutputTrayBooklet,          // 排紙トレイ：ブックレットトレイ（Ｍ：サドルトレイ）   折った用紙を出力するトレイ（中折り/中綴じ、Z折り、袋折り、３つ折り）.
		TYPE_OutputTrayProcess,          // 排紙トレイ：フィニッシャ処理トレイ.
		TYPE_OutputTrayMailBin1,         // 排紙トレイ：メールビン１.
		TYPE_OutputTrayMailBin2,         // 排紙トレイ：メールビン２.
		TYPE_OutputTrayMailBin3,         // 排紙トレイ：メールビン３.
		TYPE_OutputTrayMailBin4,         // 排紙トレイ：メールビン４.
		TYPE_OutputTrayMailBin5,         // 排紙トレイ：メールビン５.
		TYPE_OutputTrayAuto,             // 排紙トレイ：自動判別.
// comment 2015/06/26 OPS1-Sky西村 ZeusS_ZX0 機器構成変更No.3.
//		TYPE_OutputTrayProcess2          // 排紙トレイ：フィニッシャ処理トレイ２.
		TYPE_OutputTrayProcess2,         // 排紙トレイ：フィニッシャ処理トレイ２.
		TYPE_OutputTraySub3,             // 排紙トレイ：サブトレイ３(RU増設トレイ).
	};
	APIC_PrinterSystem *pc_PrinterSystem   = APIC_PrinterSystem::newInstance();
	Bool b_UpdFlg = False;
	for( ushort us_Idx = 0; us_Idx < sizeof( e_OutputTrayTbl ) / sizeof( TYPE_OutputTray ); ++us_Idx ){ 
		TYPE_OutputTray e_OutTray = pc_PrinterSystem->getPcPrintOutputTray( e_OutputTrayTbl[ us_Idx ] );
		if( m_stSysStatInfo.st_PrinterDeviceInfoParam.e_DevPrt119[ (ushort)e_OutputTrayTbl[ us_Idx ] ] != e_OutTray ){
			m_stSysStatInfo.st_PrinterDeviceInfoParam.e_DevPrt119[ (ushort)e_OutputTrayTbl[ us_Idx ] ] = e_OutTray;
			if( b_UpdFlg == False ){
				b_UpdFlg = True;
			}
		}
	}
	if( b_UpdFlg ){
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_119;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.e_DevPrt119[ 0 ]);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}

	// サドルユニット有り.
	Bool b_Val = pc_PrinterSystem->isExistSaddleUnitOption();
	if( m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt128 != b_Val){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt128 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_128;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt128);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}

	// パンチ機能.
	TYPS_PunchFunction st_Punch = pc_PrinterSystem->getPunchFunction();
	if( m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt134.b_Punch2Hole    != st_Punch.b_Punch2Hole ||
		m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt134.b_Punch3Hole    != st_Punch.b_Punch3Hole ||
		m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt134.b_Punch4Hole    != st_Punch.b_Punch4Hole ||
		m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt134.b_Punch4HoleSWE != st_Punch.b_Punch4HoleSWE ){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt134 =  st_Punch;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_134;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.st_DevPrt134);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// PK-514装着時のパンチモード制約有り.
	b_Val = pc_PrinterSystem->isPunchModeRestricted();
	if( m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt135 != b_Val){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt135 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_135;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt135);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	

	// カバーシートフィーダ.
	b_Val = pc_PrinterSystem->isCoverSheetFeeder();
	if( m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt138 != b_Val){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt138 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_138;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt138);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
	
	// 第３トレイ装着情報.
	b_Val = pc_PrinterSystem->isOptionTray3();
	if( m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt139 != b_Val){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt139 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_139;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt139);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}

//Str【ZeusSZX0 RU上排紙対応 : K.Natsume : 2015.07.06】.
	// 第４（ＲＵ）トレイ装着情報.
	b_Val = pc_PrinterSystem->isOptionTray4();
	if( m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt156 != b_Val){
		m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt156 = b_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_PRT_156;
		stChangedInfo.p_Value = &(m_stSysStatInfo.st_PrinterDeviceInfoParam.b_DevPrt156);
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}
//End【ZeusSZX0 RU上排紙対応 : K.Natsume : 2015.07.06】.

	SST_LOG2(MFPE_LogLevelAll, "End.\n", NULL);
}

//************************************************************************************************************************.
//概要　　：APIC_SoftDipSwObserver::dataChanged情報取得処理.
//戻り値　：無し.
//備考　　：特に無し.
//作成者　：ADS.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//************************************************************************************************************************.
void SSTC_SystemStatusService::getSoftDipSwObserverData()
{
	SST_LOG(MFPE_LogLevelAll);
	SSTS_ChangedInfo stChangedInfo;

	// SoftDipSw設定.
	APIC_SoftDipSw* pc_SoftDipSw = APIC_SoftDipSw::newInstance(); 
	Bool b_UpdFlg = False;
	ushort us_SwNo( 0 );
	ushort us_BitNo( 0 );
	for( us_SwNo  = 1; us_SwNo <= ( ushort )TYPD_SSTS_DipSwMax; us_SwNo++ ){
		for( us_BitNo = 1; us_BitNo <= ( ushort )TYPD_SSTS_DipSwBitCount; us_BitNo++ ){
			Bool b_Val = pc_SoftDipSw->getSoftDipSwBit( us_SwNo, (uchar)us_BitNo );
			if( b_Val != m_stSysStatInfo.st_DeviceInfoParam.b_Dev104[ us_SwNo - 1 ][ us_BitNo - 1 ] ){
				m_stSysStatInfo.st_DeviceInfoParam.b_Dev104[ us_SwNo - 1 ][ us_BitNo - 1 ] = b_Val;
				if( b_UpdFlg == False ){
					b_UpdFlg = True;
				}
			}
		}
	}
	if( b_UpdFlg ){
		stChangedInfo.e_DataID = TYPE_NTF_DataID_DEV_104;
		stChangedInfo.p_Value = &( m_stSysStatInfo.st_DeviceInfoParam.b_Dev104 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}

	SST_LOG2(MFPE_LogLevelAll, "End.\n", NULL);
}

//************************************************************************************************************************.
//概要　　：APIC_SmtpSendSettingObserver::dataChanged情報取得処理.
//戻り値　：無し.
//備考　　：特に無し.
//作成者　：ADS.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//************************************************************************************************************************.
void SSTC_SystemStatusService::getSmtpSendSettingObserverData()
{
	SST_LOG(MFPE_LogLevelAll);
	SSTS_ChangedInfo stChangedInfo;
	
	APIC_SmtpSendSetting *pc_SmtpSendSetting = APIC_SmtpSendSetting::newInstance();
	
	// ポート番号（SMTP送信設定）.
	ushort us_Val = pc_SmtpSendSetting->getPortNumber();
	if( us_Val != m_stSysStatInfo.st_NetworkInfoParam.us_Net48 ){
		m_stSysStatInfo.st_NetworkInfoParam.us_Net48 = us_Val;
		stChangedInfo.e_DataID = TYPE_NTF_DataID_NET_48;
		stChangedInfo.p_Value  = &( m_stSysStatInfo.st_NetworkInfoParam.us_Net48 );
		// 2021/10/22 APF OP_BTS-35403 start
		//m_vChangedInfo.push_back(stChangedInfo);
		AddChangedInfo(&stChangedInfo);
		// 2021/10/22 APF OP_BTS-35403 end
	}

	SST_LOG2(MFPE_LogLevelAll, "End.\n", NULL);
}

//************************************************************************************************************************.
//概要　　：APIC_ImageLogObserver::notifyImageLogWarning情報取得処理.
//戻り値　：無し.
//備考　　：特に無し.
//作成者　：ADS.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//************************************************************************************************************************.
void SSTC_SystemStatusService::getImageLogObserverData()
{
	SST_LOG(MFPE_LogLevelAll);
	SSTS_ChangedInfo stChangedInfo;

	Bool b_Ret = False;
	
	APIC_ImageLogControll *pc_ImageLogControll = APIC_ImageLogControll::newInstance();
	
	if (pc_ImageLogControll != NULL) {
		// 画像ログ(簡易ログ)フル状態.
		b_Ret = pc_ImageLogControll->isImageLogFull();
		if( b_Ret != m_stSysStatInfo.st_OtherInfoParam.b_Oth9 ){
			m_stSysStatInfo.st_OtherInfoParam.b_Oth9 = b_Ret;
			stChangedInfo.e_DataID = TYPE_NTF_DataID_OTH_9;
			stChangedInfo.p_Value  = &( m_stSysStatInfo.st_OtherInfoParam.b_Oth9 );
			// 2021/10/22 APF OP_BTS-35403 start
			//m_vChangedInfo.push_back(stChangedInfo);
			AddChangedInfo(&stChangedInfo);
			// 2021/10/22 APF OP_BTS-35403 end
		}
	}

	SST_LOG2(MFPE_LogLevelAll, "End.\n", NULL);
}

//************************************************************************************************************************.
//概要　　：SSTC_SystemStatusService::attachImageLogObserver情報取得処理.
//戻り値　：無し.
//備考　　：特に無し.
//作成者　：日立JTE 古川.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//        ：2013/10/16 【DS-IGL】日立JTE 古川 画像ログオプション化 新規作成.
//************************************************************************************************************************.
void SSTC_SystemStatusService::attachImageLogObserver()
{
    APIC_ImageLogControll *pc_ImageLogControll = APIC_ImageLogControll::newInstance();
    if(pc_ImageLogControll == NULL){
        errorLog_sysServiceSsts("pc_ImageLogCntroll is Null\n");
        return ;
    }
    
    m_pcImageLogObserver = SSTC_ConcreteImageLogObserver::newInstance();
    pc_ImageLogControll->attachObserver( m_pcImageLogObserver );
}

// 【RQ-747 VirusScanFUM for 機種】 start.
//************************************************************************************************************************.
//概要　　：APIC_VirusScanObserver::dataChanged情報取得処理.
//戻り値　：無し.
//備考　　：特に無し.
//作成者　：Sky新大阪.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//************************************************************************************************************************.
//Bool SSTC_SystemStatusService::getVirusScanObserverData()
void SSTC_SystemStatusService::getVirusScanObserverData()
{
	SST_LOG(MFPE_LogLevelAll);

	typedef struct st_CheckWarning {
		TYPE_WarningType			e_Vrs;					// VirusScan警告情報.
		TYPE_NTF_DataID				e_DataID;				// データID.
		Bool 						*pb_Val;				// 警告の有無.
	}ST_CHK_WARNING;
	static ST_CHK_WARNING st_CheckWarningTable[] = {
		{ ATTENTION_VirusDetection				,	TYPE_NTF_DataID_WRN_VRS_1	, &( m_stSysStatInfo.st_WarningInfoParam.st_VrsWarningInfoParam.b_WrnVrs1 ) },	// ウイルス検知警告（Attention）.
		{ ATTENTION_PatternFileAvailability		,	TYPE_NTF_DataID_WRN_VRS_2	, &( m_stSysStatInfo.st_WarningInfoParam.st_VrsWarningInfoParam.b_WrnVrs2 )	},	// パターンファイル有無（Attention）.
		{ (ATTENTION_VirusScan)0				,	TYPE_NTF_DataID_End			, NULL																		}	// end.
	};
	SSTS_ChangedInfo stChangedInfo;
	Bool b_Val;

	for( ST_CHK_WARNING *pst_WarTbl = &st_CheckWarningTable[ 0 ]; pst_WarTbl->e_DataID != TYPE_NTF_DataID_End; pst_WarTbl++ ){
		b_Val = APIC_VirusScan::newInstance()->isWarning( pst_WarTbl->e_Vrs );
		if( *( pst_WarTbl->pb_Val ) != b_Val ){
			*( pst_WarTbl->pb_Val ) = b_Val;
			stChangedInfo.e_DataID = pst_WarTbl->e_DataID;
			stChangedInfo.p_Value  = pst_WarTbl->pb_Val;
			// 2021/10/22 APF OP_BTS-35403 start
			//m_vChangedInfo.push_back( stChangedInfo );
			AddChangedInfo( &stChangedInfo );
			// 2021/10/22 APF OP_BTS-35403 end
		}
	}
}
// 【RQ-747 VirusScanFUM for 機種】 end.

//************************************************************************************************************************.
//概要　　：APIC_JLGObserver::JobLogDataFULL情報取得処理.
//戻り値　：無し.
//備考　　：特に無し.
//作成者　：SCD2-本橋.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//			2018/08/30 [RQ-674] SCD2-motohashi ログ遅延警告を追加.
//			2018/09/29 [RQ-674] SCD2-motohashi ジョブログ関連の警告取得処理は定期起床処理で行うように変更の為.
//											   メソッド名を「getJlgObserverData」→「getJlgWarningStatus_Low」へ変更.
//************************************************************************************************************************.
void SSTC_SystemStatusService::getJlgWarningStatus_Low()
{
	SST_LOG(MFPE_LogLevelAll);

	typedef struct st_CheckWarning {
		TYPE_JlgWarning				e_Jlg;					// JLG警告情報.
		TYPE_NTF_DataID				e_DataID;				// データID.
		Bool 						*pb_Val;				// 警告の有無.
	}ST_CHK_WARNING;
	static ST_CHK_WARNING st_CheckWarningTable[] = {
		{ TYPE_JlgWarning_LogDelayed	,	TYPE_NTF_DataID_WRN_JLG_1	, &( m_stSysStatInfo.st_WarningInfoParam.st_JlgWarningInfoParan.b_WrnJlg1 ) },	// ログ遅延警告.
		{ (TYPE_WarningType)0			,	TYPE_NTF_DataID_End			, NULL																		}	// end.
	};
	SSTS_ChangedInfo stChangedInfo;
	Bool b_Val;

	for( ST_CHK_WARNING *pst_WarTbl = &st_CheckWarningTable[ 0 ]; pst_WarTbl->e_DataID != TYPE_NTF_DataID_End; pst_WarTbl++ ){
		b_Val = APIC_JLG::newInstance()->isWarning( pst_WarTbl->e_Jlg );
		if( *( pst_WarTbl->pb_Val ) != b_Val ){
			*( pst_WarTbl->pb_Val ) = b_Val;
			stChangedInfo.e_DataID = pst_WarTbl->e_DataID;
			stChangedInfo.p_Value  = pst_WarTbl->pb_Val;
			// 2021/10/22 APF OP_BTS-35403 start
			//m_vChangedInfo.push_back( stChangedInfo );
			AddChangedInfo( &stChangedInfo );
			// 2021/10/22 APF OP_BTS-35403 end
		}
	}
}

//************************************************************************************************************************.
//概要　　：警告情報更新要求.
//戻り値　：正常(MFPD_OK)または異常(MFPD_NG).
//備考　　：特に無し.
//作成者　：Sky新大阪.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//************************************************************************************************************************.
MFPD_Bool SSTC_SystemStatusService::reqUpdateWarning( )
{
	SST_LOG(MFPE_LogLevelTrace);

	msgQSend( SSTE_SysStatusMsgType_UpdateWarning );

	return MFPD_OK;
}

//*****************************************************************************.
//概要　　：警告情報を更新する.
//引数　　：なし.
//戻り値　：なし.
//作成者　：Sky新大阪.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//*****************************************************************************.
void SSTC_SystemStatusService::updateWarning( )
{
	SST_LOG(MFPE_LogLevelTrace);

	m_vChangedInfo.clear();	// 変化情報リスト初期化.

	procUpdateWarning();	// 情報更新.

	TYPE_NTF_GroupID  e_writeGroupID    = TYPE_NTF_GroupID_End;
	for( std::vector<SSTS_ChangedInfo>::iterator it = m_vChangedInfo.begin(); it != m_vChangedInfo.end(); ++it ){
		
		// グループ情報取得 該当なしの場合はエラーログ出力.
		TYPE_NTF_GroupID  e_GroupID    = TYPE_NTF_GroupID_End;
		void 			 *pv_GroupData = NULL;
		
		if( SSTC_SystemStatusService::getGroupInfo( (it)->e_DataID, &e_GroupID, &pv_GroupData ) == False ){
			// エラーログ出力.
			SST_ERR_LOG("getGroupInfo() == False.\n");
			continue;
		}
		if( e_GroupID != e_writeGroupID ){
			// 共有メモリ更新.
			m_pcInstance->lock();
			m_pcShMemMan->writeData( e_GroupID, pv_GroupData );
			m_pcInstance->unlock();
			e_writeGroupID = e_GroupID;
		}
	}

	if( m_bStartNotifyFlag == True ){
		for( std::vector<SSTS_ChangedInfo>::iterator it = m_vChangedInfo.begin(); it != m_vChangedInfo.end(); ++it ){
			// データ変化通知.
			notifyChangedData( (it)->e_DataID );
		}
	}

	m_vChangedInfo.clear();	// 変化情報リストをクリア.
}

//*****************************************************************************.
//概要　　：警告情報を更新する.
//引数　　：なし.
//戻り値　：なし.
//作成者　：Sky新大阪.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//*****************************************************************************.
void SSTC_SystemStatusService::procUpdateWarning()
{
	SST_LOG(MFPE_LogLevelAll);

	// カウンタを見て、警告確認を抑制している以下のカウンタのみ、カウントを強制的にアップさせる.
	m_ulCounterPrinterStatusObserver++;				///< APIC_PrinterStatusObserver::dataChanged()発生回数.
	m_ulCounterPrinterWarningObserver++;			///< APIC_PrinterWarningObserver::dataChanged()発生回数.

	// 警告情報取得.
	getRdsWarningStatus_Low();	// RDS警告情報取得処理.
	getBoxWarningStatus_High();	// BOX警告情報取得処理.
	getFaxWarningStatus_High();	// FAX警告情報取得処理.
	getPrtWarningStatus_High();	// PRT警告情報取得処理.
	getScnWarningStatus_High();	// SCN警告情報取得処理.
	getScnWarningStatus_Low();

	// スキャナ情報取得.
	getScannerInfoStatus_High();
	
	// デバイス情報プリンタ取得.
	getPrinterDeviceStatus_High();
	getPrinterDeviceStatus_Low();

	getJlgWarningStatus_Low();	// JLG警告情報取得処理.

	getSystemStatusObserverData();
}

// 2021/10/22 APF OP_BTS-35403 start
//*****************************************************************************.
//概要　　：変化情報追加.
//引数　　：pstChangedInfo		変化点情報.
//戻り値　：なし.
//作成者　：Sky新大阪.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//*****************************************************************************.
void SSTC_SystemStatusService::AddChangedInfo( SSTS_ChangedInfo* pstChangedInfo )
{
	if( NULL == pstChangedInfo ){
		errorLog_sysServiceSsts( "AddChangedInfo() pstChangedInfo is NULL.\n" );
		return;
	}

	m_vChangedInfo.push_back( *pstChangedInfo );

	// 通知開始フラグが無効の場合.
	if( m_bStartNotifyFlag == False ){
		// 通知開始時に変化点通知処理ができるよう警告データIDを記憶.
		if( ( 0x00000000 == ( 0xFFFF0000 & pstChangedInfo->e_DataID ) ) ){
			if( m_vStartNotifyDataID.end() == std::find( m_vStartNotifyDataID.begin(), m_vStartNotifyDataID.end(), pstChangedInfo->e_DataID ) ){
				m_vStartNotifyDataID.push_back( pstChangedInfo->e_DataID );
			}
		}
	}
}
// 2021/10/22 APF OP_BTS-35403 end
