// ================================================================================================.
//! \file THRC_PowerManager.cpp スレッド 電源管理クラス定義実装 機種共通.
// ================================================================================================.

#include "system/cnt/CNTC_CntIf.h"
#include "system/thr/thrcom/THRC_PowerManager.h"
#include  "THRC_Vxworks.h"
#include "system/thr/thrcom/THRC_Threaddef.h"
#include  "THRC_Multithread.h"
#include "system/thr/thrInclude/THRC_PortInstance.h"
#include  "THRC_BSPif.h"
#include  "system/thr/thrInclude/THRC_ThreadInstance.h"
#include  "system/prt/prtc/prtcImp/ENGC_PrinterImpColor.h"		// 05/12/25 moirkawa 追加.
#include  "system/thr/thrInclude/THRC_Threadif.h"
#include  "system/fil/FILC_FilIf.h"
#include  "system/prt/prtc/prtcImp/ccpic.h"				// 2006.02.16 Katougi PIC2007省電力解除シーケンス追加対応.
#include  "system/img/imgc/IMGC_MemMan.h"	// 06/03/24 morikawa 電源ONによる起動状態識別追加.
#include  "system/sys/APIC_SystemServiceFunction.h"
#include  "system/thr/thrInclude/THRC_BSPifCom.h"
#include "system/nvd/nvdg/NVDG_NVWrite.h"
#include "system/sys/APIC_KeyDevice.h"
#include "system/thr/PanelHW/THRC_PanelHardSoC.h"
#ifdef TORNADO
#include "Linux/Platform/InitDeviceIt6.h"
#endif
#include "system/sys/APIC_OptionSetting.h"	//2014/08/05 ADS八木 CQ.500724(Act_501780) 機能判定.
#include <usb.h>							// 2021/08/16 OP_BTS-35243 山口 SuperWarp起動で認証装置を認識しない.

int32_t wdTestCode = 0;		// [forTest] 2012/10/04 河村 - CQ363535 wd実装テスト.
Bool g_SubPowOff15ToSubPowOffDelay= False;	// 2016/02/10 Sky_OGAWA CQ.525104 副電源遅延後、Fiery初期化不良.

#include "THRC_PowerManagerDevice.h"
#include "system/sys/OsWrapper/SYSC_OSWrapper.h"
#include "system/sys/FsWrapper/SYSC_FSWrapper.h"			/* 10/12/03 浦崎 追加 */
#include "system/thr/thrcom/ThrCommonDrvrIf.h"
// BSPのダミー関数定義.
#include  "system/thr/thrcom/Dummy_BSP.h"

#include "system/sys/APIC_VirusScan.h"

#ifdef TORNADO
#include <nicfum/mioApi.h>
#endif

//D030対応.
#include "system/thr/thr/thrCrypto.h"

// for Debug.
#define THRC_PM_DEBUG_WAIT

#include <stdio.h>			//perror.
#include <stdlib.h>			//system.

// Str 2010/09/13 ft_kondo 8536環境Sim対応.
#ifdef TORNADO			// 実機で有効.
#include <linux/rtc.h>		//rtc.
#include <sys/ioctl.h>		//ioctl.
#include <sys/time.h>
#include <unistd.h>
#include <signal.h>			//sigaction,siginfo_t.
#include <sys/syscall.h>	//gettid 2013/02/14 堀井 CQ.377663.

#include <sys/mman.h>
#include <sys/stat.h>
#endif // #ifndef TORNADO.
// End 2010/09/13 ft_kondo 8536環境Sim対応.

#include <sys/types.h>
#include <fcntl.h>			// O_RDONLY.
#include <linux/fb.h>

#ifdef DEF_Helios_PF
#include "Linux/Platform/Mediaif.h"
#define DEBUG_USE_TAIKI_COM_BUF
#include "Linux/taiki_com_chrdev_api.h"
#include "Linux/ogma_chrdev_api.h"
#endif
#include "THRC_McCpuIf.h"				// 2014/12/22 小林 CQ.508148/508299 省電力移行時にC-5370を誤検出する問題対応.

#include "system/job/APIC_JobList.h"	// 2015/03/18 荒木  CQ.510572.

#include "system/sys/APIC_System.h"		// 2015/07/24 sky_okuma CQ516040対応.

#ifdef	DEF_A4_Cus
#include "lib/LIBC_SimpleLog.h"								// 2015/10/09 eSOL石田 パフォーマンスログ対応.
#include "system/thr/thrInclude/THRC_PerformanceLog.h"		// 2015/10/09 eSOL石田 パフォーマンスログ対応.
#endif	// DEF_A4_Cus

#include "system/sys/APIC_FatalSignalDetailCode.h"	// 2015/10/21 Harada 新詳細コード対応 .

// FWリファクタリング APIC_Setting.hの分割.
#include "../../shlib/LogLib/LIBC_LogLibDefine.h"

#include "Linux/Platform/km_warpDef.h"				// 2016/11/09 sky_ishiga [MinervaMLK][起動時間短縮] SuperWarp Snapshot取得対応.
// 2017/02/20 Sky酒井（純）ユーザーイベント記録(APIコール).
#include "system/sys/OsWrapper/SYSC_OSWrapperTaskMonitorLinux.h"

// 2017/05/22 Sky-ishiga [AR1023389] トラブル、エラーシグナル検出時のSuperWarp不可対応.
#include "../../../shlib/LogLib/LIBC_LogLib.h"

// 2018/03/28 Sky酒井(純) [Eagle] [制御構成変更_No.92-1] 振動パネル対応(追加).
#include "system/nvd/APIC_Setting_OperationSetting.h"

// comment 2018/09/10 SCD2-太田 [Eagle H][制御構成変更No.50]待ち時間測定の自動化.
#include "system/dbg/PFMG_PerformAutoLogging.h"

#include "system/prt/APIC_Printer.h"			// 2020/11/20 Sky CPU高温異常に対するフェールセーフ対応

// グループテーブル.
// グループとデバイスの関係を表す.
// 2014/04/10 toda Helios対応 Heliosで不要なものを削除.
// 2014/07/07 takas Helios対応 Sleep1でPEX-PDを切る対応.
static struct  {
	THRC_PowerDeviceID e_newGroup;
	THRE_PowerSave_Device e_oldGroup;
} s_groupTable[] = {
	{ THRC_PowerDeviceID_FileSystem,	THRE_PowerSave_ImageTransReceiveGroup },
	{ THRC_PowerDeviceID_HDD_DRIVER,	THRE_PowerSave_ImageTransReceiveGroup },
	{ THRC_PowerDeviceID_PCIe_BUS,		THRE_PowerSave_ImageTransReceiveGroup },
	{ THRC_PowerDeviceID_CPU_FAN,		THRE_PowerSave_ImageTransReceiveGroup },
	{ THRC_PowerDeviceID_SOUND,			THRE_PowerSave_ImageTransReceiveGroup },	// 音声IC対応 2014/9/5 小林 サウンドデバイス追加.
	{ THRC_PowerDeviceID_USB,			THRE_PowerSave_USB_H },
	{ THRC_PowerDeviceID_IR,			THRE_PowerSave_IR },
	{ THRC_PowerDeviceID_Fiery_PT_T,	THRE_PowerSave_IR },
	{ THRC_PowerDeviceID_Fiery,			THRE_PowerSave_FieryIF },
	{ THRC_PowerDeviceID_Panel,			THRE_PowerSave_PanelGroup },
	{ THRC_PowerDeviceID_IMGSYS,		THRE_PowerSave_IMGSYS },
	{ THRC_PowerDeviceID_PEXPD,			THRE_PowerSave_ImageTransReceiveGroup },
	{ THRC_PowerDeviceID(-1),			THRE_PowerSave_Device(-1) },
};

//++ 2019/04/24 Sky		【EagleH】Warning除去対応.
// Str 2010/09/13 ft_kondo 8536環境Sim対応.
#if defined(TORNADO) && !defined(SIMULATOR)			// 実機で有効.
/****************************************
  static 関数プロトタイプ宣言			// 10/07/13 内野 追加.
****************************************/
static void    sig_handler( int32_t sn , siginfo_t *si , void *sc );
#endif // #ifndef TORNADO.
//-- 2019/04/24 Sky		【EagleH】Warning除去対応.


/*================================================================================================
	モジュール内関数の宣言 
================================================================================================*/
// 定期起床用.
static void cyclicLoop_taskPowerManager();

// パワーダウンPCIクロック、画像クロック、ASIC1セルフリフレッシュ制御関数.
static void powerDown_ChkError();
void cps2300_InputSel( uchar uc_sel );

// 2021/07/27 ft_kondo OP_BTS-34942 RamDisk内のログ削除用監視処理追加. 
void checkRamDisk_Messages(void);

#define		WAIT_10US(x)	{	wait_10us(x); THRL_DbgPowerLogCT(481,x);	}
#define		DELAY_TSK(x)	{	delayTask(x); THRL_DbgPowerLogCT(482,x);	}

#define time_after(a,b)         ((int32_t)(b) - (int32_t)(a) < 0)

// 2015/07/24 sky_okuma CQ516040対応.
///	\brief	機能：デバッグログ保存可否設定.
///	\par	備考：.
///	\param	bl_flg
///	\result	なし.
static void setDebugLogSaveEnable( Bool bl_flg );
// 2015/07/24 sky_okuma CQ516040対応.

// 2017/02/10 sky_hatono [MinervaMLK][起動時間短縮] AR1012820 メモリパーティション管理領域の保存/復帰.
// 2018/09/10 Sky酒井(純) [Eagle] AR1055313 メモリパーティションのバックアップ、リストアの関数名を変更.	
extern STATUS MemPartAreaBackup( void );
extern STATUS MemPartAreaRestore( void );

// 2020/04/28 河村 -  OP_BTS-22858 CA72WDTを省電力制御以外も動作させる対応.
extern void ca72wdt_ext_refresh( uint32_t point, uint32_t timeout, uint32_t debugInfo1, char *debugInfo2 );

/*================================================================================================
	モジュール内変数の宣言　
================================================================================================*/
static WDOG_ID wdog_ID_PowerManager = NULL;
// 2022/09/22 OP_BTS-39386 山口 RTC時刻初期化【暫定】.
static WDOG_ID wdog_ID_rtc_Time_BackUpLoop = NULL;	
static WDOG_ID wdog_ID_rtc_Time_BackUpStartUp = NULL;

Bool bl_PowerOnStart = True;	// 電源ONによる起動状態.
// 2014/04/30 ft_kondo 省電力復帰時間短縮対応. 
Bool b_doPowerChangeSequence_Hdd = False;	// HDD復帰シーケンス実行中フラグ. 
Bool b_rec_HDD = False;
THR_ID si_taskID_PM_Panel = 0;
THR_ID si_taskID_PM_PCIe_PCIDev = 0;
THR_ID si_taskID_PM_Fiery = 0;
THR_ID si_taskID_PM_USB = 0;
THR_ID si_taskID_PM_HDD = 0;
THR_ID si_taskID_PM_IR = 0;
THR_ID si_taskID_PM_Watch_HDD = 0;
WDOG_ID wdID_Watch_HDD = 0;
static uint32_t ul_PM_HDD_tickGetTime1 = 0;	// HDDの復帰処理開始の時間(tick). 
static uint32_t ul_PM_HDD_tickGetTime2 = 0;
// 2015/07/24 sky_okuma CQ516040対応.
static Bool bl_debugLogSaveEnable = True;	// デバッグログ保存可否設定値
extern Bool isDeepSleepShift;				// DeepSleep/ErP 移行/復帰中か？実態はTHRC_PowerManagerDevice.cpp.

Bool bl_PowerOffTimerUpdate = False;		//2019/04/04 Sky[IT6_1.0MR2][AR1074753]電源OFFタイマー更新中フラグ.

//省電力復帰要因用
#define THRD_PmuEventMaxNum	100
// 2014/10/08 FT_yamaguchi CQ.503322.
static Bool bl_MODEM_MaskSetting = True;

#ifdef TORNADO
static class PmuEvent {
	int32_t code[THRD_PmuEventMaxNum];
	int32_t rp, wp;
	void sig_block( Bool is_block ) {
		sigset_t sigset;
		// 2011/12/19 ft_kondo 静的解析 CID:13063. 
		if( sigemptyset( &sigset ) < 0 ){
			printf( "(%010u)(BSP) error PmuEvent() sigemptyset()\n", SYSF_tickGet() );
			return;
		}
		// 2011/12/19 ft_kondo 静的解析 CID:13062. 
		if( sigaddset( &sigset, SIGRTMIN + 6 ) < 0 ){
			printf( "(%010u)(BSP) error PmuEvent() sigaddset()\n", SYSF_tickGet() );
			return;
		}
		// 2014/12/04 ikawa CQ503192 対応.
		pthread_sigmask( is_block == TRUE ? SIG_BLOCK : SIG_UNBLOCK, &sigset, NULL );
	}
public:
	PmuEvent() {
		rp = 0;
		wp = 0;
		// 2012/08/31 ft_kobayashi 静的解析 CID:35735.
		// 2011/12/19 ft_kondo 静的解析 CID:35735. 
		memset( code, 0x0, sizeof(code) );
	}
	~PmuEvent() {};
	void add( int32_t event ) {
		sig_block( True );
		code[wp] = event;
		wp = (wp+1) % THRD_PmuEventMaxNum;
		sig_block( False );
	}
	int32_t get(void) {
		int32_t ret;
		sig_block( True );
		if( rp == wp ) {
			sig_block( False );
			return 0;
		}
		ret = code[rp];
		rp = (rp+1) % THRD_PmuEventMaxNum;
		sig_block( False );
		return ret;
	}
} pmuEvent;
#endif

// 2015/12/18 Sky-Araki CQ520828 Lot26対応.
// 2017/10/11 Sky佐藤 S800省電力Lot26対応.
Bool Lot26HwType= True;

// 17/05/08 Sky-Araki AR1021331 xhci-hcd Timeout.
#define	PM_TIME_Watch		(100)
#define PM_TIME_WaitCntr	(10*1000/PM_TIME_Watch)

/*================================================================================================
	人体近接検出ioctl用定義. 10/11/30 静電センサー対応 13/02/28 Sky 機種共通化対応 TRHC_PowerManagerSoC.cppから移植.
================================================================================================*/
#define CAP1166_SET_CTSC        1   /*人体近接検出センサ操作(CapacitiveTouchSensorControl)*/
/* 人体近接検出センサ操作(CapacitiveTouchSensorControl)制御用ioctl構造体 */
struct ctsc_ioctl_info_struct
{
	uchar state;        /*状態*/
	uchar mode;         /*モード*//*2011/10/13 第1OSW浦崎 「近接センサーの感度設定対応」対応。*/
};
static struct ctsc_ioctl_info_struct ctsc_ctrl_info;

/*================================================================================================
	スレッド電源制御部専用log 13/02/28 Sky 機種共通化対応 TRHC_PowerManagerSoC.cppから移植.
 ================================================================================================*/
#define THRC_PM_DBGMSG_MAX 250	// 2008/04/15 河村 CQ.302130 - ログ出量の調整.
#define THRC_PM_DBGMSG_LENGTH 128
// 2011/08/19 Harada デバッグログ収集システム対応 >>.
// 上記定義を参考に \shlib\LogLib\LIBC_LogLibDefine.h で LIBD_PM_DBGMSG_MAX、LIBD_PM_DBGMSG_LENGTH を定義している.
// THRC_PM_DBGMSG_MAX、THRC_PM_DBGMSG_LENGTH の変更時は、LIB側の値も変更すること.
// 2011/08/19 Harada デバッグログ収集システム対応 <<.
static int32_t msgNum = 0;
static char msgStr[THRC_PM_DBGMSG_MAX][THRC_PM_DBGMSG_LENGTH];
static FILE *pm_fp = NULL;
static Bool isLooped=False;
int32_t pmLogOn = 0;	// 2008/04/15 河村 CQ.302130 - デバッグ関数名称統一.
// 2011/08/19 Harada デバッグログ収集システム対応 >>.
extern LIBS_ST_ThrPwrMngLog st_ThrPwrMngLog;
// 2011/08/19 Harada デバッグログ収集システム対応 <<.

// スレッド電源制御部専用log.
#include <stdarg.h>
void THRC_PM_LOG( const char *formatString, ... )
{
#ifdef TORNADO
	char buf[512];
	va_list ap;

	va_start( ap, formatString );
	vsprintf( buf, formatString, ap );
	va_end( ap );

	strncpy( msgStr[msgNum], buf, THRC_PM_DBGMSG_LENGTH-1 );
	//for Debug.
	if( pmLogOn )
		printf( "(%010u)(BSP)[PM_LOG] %s\n", SYSF_tickGet() , buf);
	msgStr[msgNum][THRC_PM_DBGMSG_LENGTH-1] = '\0';

	msgNum++;
	if( msgNum >= THRC_PM_DBGMSG_MAX ) {
		msgNum = 0;
		isLooped = True;
	}
#else	// TORNADO.
	// simの場合はそのまま出力.
	char buf[512];
	va_list ap;

	va_start( ap, formatString );
	vsprintf( buf, formatString, ap );
	va_end( ap );

	if( pm_fp == NULL ) {
		TRACE( "## %s\n", buf, 0,0,0 );
	} else {
		fprintf( pm_fp, "%s\n", buf );
	}
#endif
}

// 2008/04/15 河村 CQ.302130 - デバッグ関数名称統一/ログ出力の調整.
extern "C" void pmLogPrint_File(FILE *p_File);
extern "C" void pmLogPrint(void)
{
	pmLogPrint_File( stdout ); // 既存動作は標準出力.
}
extern "C" void pmLogPrint_File(FILE *p_File)
{
	if( NULL==p_File )
	{
		p_File = stdout; // ファイルポインタがNULLであれば標準出力に切り替える.
	}

	// 2018/04/10 ft_kondo 制御構成変更No110 ログリファイン. 
	fprintf(p_File, "========== BSP ==========\n" );
	fprintf(p_File, "Output power saving sequence log\n" );
	fprintf(p_File, "========== BSP ==========\n" );
	THRC_PM_LOG( "-- last" );
// 2011/08/19 Harada デバッグログ収集システム対応 >>.
	memset( st_ThrPwrMngLog.c_msgStr_StrinData_Command, 0, sizeof(st_ThrPwrMngLog.c_msgStr_StrinData_Command) );
	sprintf( st_ThrPwrMngLog.c_msgStr_StrinData_Command, "\n\n-> pmLogPrint\n" );
// 2011/08/19 Harada デバッグログ収集システム対応 <<.
	for( int32_t i=0; i< (isLooped == True ? THRC_PM_DBGMSG_MAX : msgNum); i++ )
	{
		fprintf(p_File, "(%010u)(BSP) %s\n", SYSF_tickGet() , msgStr[i] );
// 2011/08/19 Harada デバッグログ収集システム対応 >>.
		memset( st_ThrPwrMngLog.c_msgStr_StrinData[i], 0, sizeof(st_ThrPwrMngLog.c_msgStr_StrinData[i]) );
		sprintf( st_ThrPwrMngLog.c_msgStr_StrinData[i], "%s\n", msgStr[i] );
// 2011/08/19 Harada デバッグログ収集システム対応 <<.
	}
}

extern "C" void THRC_PM_LOG_CLEAR(void)
{
	msgNum = 0;
}

void THRC_PM_MSG( const char *formatString, ... )
{
	char buf[512];
	va_list ap;

	va_start( ap, formatString );
	vsprintf( buf, formatString, ap );
	va_end( ap );

#ifdef TORNADO
	if( pm_fp == NULL ) {
		logMsg( "%s", buf, 0,0,0,0,0 );
	} else {
		fprintf( pm_fp, "%s", buf );
	}
#else	// TORNADO.
	if( pm_fp == NULL ) {
		TRACE( "%s", buf, 0,0,0 );
	} else {
		fprintf( pm_fp, "%s", buf );
	}
#endif
}

extern "C" void setPmLogOn( int32_t si_OnOff )
{
	if ( 0 == si_OnOff ) {
		pmLogOn = 0;
	} else {
		pmLogOn = 1;
	}
}

extern "C" void THRF_MainSwOFFEvent( void )
{
	/* 主電源OFFイベントを追加 */
	pmuEvent.add( 0x126A ); 
}

enum THRE_SWDebugCmd {
	THRE_SWNormal,			/*=0*/
	THRE_SWSequenceStop,	/*=1*/
};
static int32_t s_debugCmd = THRE_SWNormal;

extern "C" void pmSWDebugCmd(enum THRE_SWDebugCmd para)
{
	printf( "(%010u)(BSP) @@@ pmSWDebugCmd call para=%d @@@\n", SYSF_tickGet() , para);
	s_debugCmd = (int32_t)para;
}

extern "C" void pmResettime(int32_t setval)
{
// 2017/10/17 ishiga [Eagle][起動時間短縮(再設計)] インターフェース見直し.
    printf( "(%010u)(BSP) @@@ pmResettime call setval=%d @@@\n", SYSF_tickGet() , setval);
	if( 0 <= setval && setval <= 65535 ) {
		THRF_setPowerOffTimer( (uint16_t)setval );
	} else {
		printf( "(%010u)(BSP) Invalid Parameter!\n", SYSF_tickGet() );	
	}
}

// 省電力復帰時間計測対応.
static int pmMeasurementMode = 0;	// 測定モード. 0:測定なし, 1:省電力復帰.
// Printkのログレベルを下げる.
#define PROCFS_PRINTK_LOG_LEVEL		"/proc/sys/kernel/printk"
#define DEF_PRINTK_LOG_LEVEL_HIGH	"1 4 1 7"
static char sc_PrintkLogLevel_backup[256] = {0};
static short ss_PrintkLogLevel_backupSize = 0;
static void changePrintkLogLevel()
{
	int fd, ret;

	fd = SYSF_open( PROCFS_PRINTK_LOG_LEVEL, O_RDWR, 0666 );
	if ( 0 > fd ) {
		printf( "(%010u)(BSP) Cannot open file " PROCFS_PRINTK_LOG_LEVEL "\n", SYSF_tickGet() );
		return;
	}

	// 現在の設定値をバックアップ.
	ret = SYSF_read( fd, sc_PrintkLogLevel_backup, sizeof(sc_PrintkLogLevel_backup) );
	if ( -1 == ret ) {
		printf( "(%010u)(BSP) Cannot read file " PROCFS_PRINTK_LOG_LEVEL "\n", SYSF_tickGet() );
		SYSF_close(fd);
		return;
	}
	ss_PrintkLogLevel_backupSize = ret;

	ret = SYSF_lseek( fd, 0, SEEK_SET );
	if ( -1 == ret ) {
		printf( "(%010u)(BSP) Cannot seek file " PROCFS_PRINTK_LOG_LEVEL "\n", SYSF_tickGet() );
		SYSF_close(fd);
		return;
	}

	ret = SYSF_write( fd, DEF_PRINTK_LOG_LEVEL_HIGH, sizeof(DEF_PRINTK_LOG_LEVEL_HIGH)-1 );
	if ( -1 == ret ) {
		printf( "(%010u)(BSP) Cannot write file " PROCFS_PRINTK_LOG_LEVEL "\n", SYSF_tickGet() );
	}

	SYSF_close(fd);
}
// Printkのログレベルを元に戻す.
static void restorePrintkLogLevel()
{
	int fd, ret;

	// バックアップがない場合は、終了.
	if ( 0 == ss_PrintkLogLevel_backupSize ) {
		printf( "(%010u)(BSP) [restorePrintkLogLevel] no backup data -> return\n", SYSF_tickGet() );
		return;
	}
	// ファイルオープン.
	fd = SYSF_open( PROCFS_PRINTK_LOG_LEVEL, O_RDWR, 0666 );
	if ( 0 > fd ) {
		printf( "(%010u)(BSP) Cannot open file " PROCFS_PRINTK_LOG_LEVEL "\n", SYSF_tickGet() );
		return;
	}

	// 設定値をバックアップ.
	ret = SYSF_write( fd, sc_PrintkLogLevel_backup, ss_PrintkLogLevel_backupSize );
	if ( -1 == ret ) {
		printf( "(%010u)(BSP) Cannot write file " PROCFS_PRINTK_LOG_LEVEL "\n", SYSF_tickGet() );
	}
	
	// ファイルクローズ.
	SYSF_close(fd);

	// バックアップをクリアする.
	memset( sc_PrintkLogLevel_backup, 0, sizeof(sc_PrintkLogLevel_backup) );
	ss_PrintkLogLevel_backupSize = 0;
}

// 2019/11/09 松永 kernelログレベル低下復旧対応.
// DeepSleep移行中にログレベル確認.
#define SLEEP_CHECK_DEF_LOG_LEVEL	"6 4 1 7"
static char sc_SleepLogLevel[256] = {0};
static void checkSleepLogLevel()
{
	int fd,ret,str;
	
	// PROCFS_PRINTK_LOG_LEVELファイルをオープン.
	fd = SYSF_open( PROCFS_PRINTK_LOG_LEVEL, O_RDWR, 0666 );
	if ( 0 > fd ) {
		printf( "(%010u)(BSP) [%s] Cannot open file " PROCFS_PRINTK_LOG_LEVEL "\n", SYSF_tickGet() , __FUNCTION__ );
		return;
	}
	
	// ファイルの中身を読み込む.
	ret = SYSF_read( fd, sc_SleepLogLevel, sizeof(sc_SleepLogLevel) );
	if ( -1 == ret ) {
		printf( "(%010u)(BSP) [%s] Cannot read file " PROCFS_PRINTK_LOG_LEVEL "\n", SYSF_tickGet() , __FUNCTION__ );
		SYSF_close(fd);
		return;
	}
	
	// ファイルの中身の一番初めのログレベルを比較.
	str = strncmp(SLEEP_CHECK_DEF_LOG_LEVEL ,sc_SleepLogLevel ,1 );
	
	// もしファイルのログレベルが不一致なら.
	// ログレベルをデフォルト値に書き換える.
	if( 0 != str){
		// ファイルの先頭に移動.
		ret = SYSF_lseek( fd, 0, SEEK_SET );
		if ( -1 == ret ) {
			printf( "(%010u)(BSP) [%s] Cannot seek file " PROCFS_PRINTK_LOG_LEVEL "\n", SYSF_tickGet() , __FUNCTION__ );
			SYSF_close(fd);
			return;
		}
		
		//ファイルのログレベルをデフォルト値に書き換える.
		ret = SYSF_write( fd,SLEEP_CHECK_DEF_LOG_LEVEL , sizeof(SLEEP_CHECK_DEF_LOG_LEVEL));
		if ( -1 == ret ) {
			printf( "(%010u)(BSP) [%s] Cannot write file " PROCFS_PRINTK_LOG_LEVEL "\n", SYSF_tickGet() , __FUNCTION__ );
		}
		
		printf("(%010u)(BSP) [%s] change_Log_Level\n",SYSF_tickGet() , __FUNCTION__ );
	}
	
	//ファイルをクローズ.
	SYSF_close(fd);
	// バッファをクリアする.
	memset( sc_SleepLogLevel, 0, sizeof(sc_SleepLogLevel) );
}

/* 2020/11/18 山口 OP_BTS-27099 WLANのログレベルが上がりWLANのWarningログ大量発生 */
#define PROCFS_WLAN_LOG_LEVEL		"/proc/net/rtl8812bu/log_level2"
#define WLAN_CHECK_DEF_LOG_LEVEL	"2"

static void checkWlanLogLevel(void){
	
	int fd,ret,str;
	char sc_WlanLogLevel[256] = {0};
	
	// 無線LAN無効の場合はチェックしない.
	if((THRF_isExistWLAN() == False)
	 ||( False == APIC_SystemInfo::newInstance()->getExNetworkAdapter() )
	 ||( TYPE_NetworkIFMode_SingleEthernet == APIC_ExNetworkSetting::newInstance()->getNetworkIFMode() )
	 ||( TYPE_NetworkIFMode_DualEthernet == APIC_ExNetworkSetting::newInstance()->getNetworkIFMode() ) ) {
		return;
	}
	
	
	// PROCFS_WLAN_LOG_LEVELファイルをオープン.
	fd = SYSF_open( PROCFS_WLAN_LOG_LEVEL, O_RDWR, 0666 );
	if ( 0 > fd ) {
		printf( "(%010u)(BSP) [%s] Cannot open file " PROCFS_WLAN_LOG_LEVEL "\n", SYSF_tickGet() , __FUNCTION__ );
		return;
	}
	
	// ファイルの中身を読み込む.
	ret = SYSF_read( fd, sc_WlanLogLevel, sizeof(sc_WlanLogLevel));
	if ( -1 == ret ) {
		printf( "(%010u)(BSP) [%s] Cannot read file " PROCFS_WLAN_LOG_LEVEL "\n", SYSF_tickGet() , __FUNCTION__ );
		SYSF_close(fd);
		return;
	}
	
	// ファイルの中身の一番初めのログレベルを比較.
	str = strncmp(WLAN_CHECK_DEF_LOG_LEVEL ,sc_WlanLogLevel ,1 );
	
	// もしファイルのログレベルが不一致なら.
	// ログレベルをデフォルト値に書き換える.
	if( 0 != str){
		// ファイルの先頭に移動.
		ret = SYSF_lseek( fd, 0, SEEK_SET );
		if ( -1 == ret ) {
			printf( "(%010u)(BSP) [%s] Cannot seek file " PROCFS_WLAN_LOG_LEVEL "\n", SYSF_tickGet() , __FUNCTION__ );
			SYSF_close(fd);
			return;
		}
		
		//ファイルのログレベルをデフォルト値に書き換える.
		ret = SYSF_write( fd,WLAN_CHECK_DEF_LOG_LEVEL , sizeof(WLAN_CHECK_DEF_LOG_LEVEL));
		if ( -1 == ret ) {
			printf( "(%010u)(BSP) [%s] Cannot write file " PROCFS_WLAN_LOG_LEVEL "\n", SYSF_tickGet() , __FUNCTION__ );
		}
		
		printf("(%010u)(BSP) [%s] change_Log_Level\n",SYSF_tickGet() , __FUNCTION__ );
	}
	
	//ファイルをクローズ.
	SYSF_close(fd);
	
}

// 時間測定モード.
extern "C" void setPmMeasureMode( int si_mode )
{
	if ( 0 == si_mode ) {
		restorePrintkLogLevel();	// Printkのログレベルを戻す.
		pmMeasurementMode = 0;
	} else {
		pmMeasurementMode = 1;
	}
}

// パネルログ出力(測定時用).
extern "C" {
	extern void LIBF_ShowPanelLog();
}
extern "C" void pmShowPanelLog()
{
	LIBF_ShowPanelLog();
}

/* 2011/10/01 第1OSW浦崎 CQ.355237「省電力中のプリントで省電力解除されてしまう」対応。調査用デバッグコマンド追加 */
struct led_ioctl_info_struct
{
	uchar id;
	uchar state;
};
struct led_ioctl_info_struct ctsc_dbg_ioctl_info;

#define CAP1166_SET_DBG_INFO    		2	/* CAP1166レジスタ書き換え。2011/10/01 第1OSW浦崎 CQ.355237調査用に追加 */
#define CAP1166_SET_DBG_LED_ON_OFF		3	/* LED制御on/off切り替え。  2011/10/01 第1OSW浦崎 CQ.355237調査用に追加 */

/* [for debug.]CAP1166のレジスタにターミナルから任意の値を設定する。 */
/*    「dcap x y」x=レジスタ, y=書き込みたい値.                */
extern "C" void dcap( int32_t addr, int32_t data )
{
#ifdef TORNADO			// 実機で有効.
	static int32_t dbg_cap1166_fd = -1;
	if( -1 == dbg_cap1166_fd ) {
		dbg_cap1166_fd = SYSF_open( THRD_DeviceFile_Cap1166, O_RDWR, 0666 );
		if( -1 == dbg_cap1166_fd ) {
            THRC_PM_LOG( "%"PRId32" : cap1166 open error.", SYSF_tickGet() );
        }
	}

    ctsc_dbg_ioctl_info.id = (uchar)addr;
    ctsc_dbg_ioctl_info.state = (uchar)data;

    SYSF_ioctl( dbg_cap1166_fd, CAP1166_SET_DBG_INFO, (intptr_t)&ctsc_dbg_ioctl_info );
#endif
}

/* [for debug.]CAP1166のLED制御だけをしないようにする。         */
/*    「cap_led x」x=0 LED制御する(default)、x=1 LED制御しない。*/
extern "C" void cap_led( int32_t value )
{
#ifdef TORNADO			// 実機で有効.
	static int32_t dbg_cap1166_fd = -1;
	if( -1 == dbg_cap1166_fd ) {
		dbg_cap1166_fd = SYSF_open( THRD_DeviceFile_Cap1166, O_RDWR, 0666 );
		if( -1 == dbg_cap1166_fd ) {
            THRC_PM_LOG( "%"PRId32" : cap1166 open error.", SYSF_tickGet() );
        }
	}

    ctsc_dbg_ioctl_info.id = (uchar)value;
    ctsc_dbg_ioctl_info.state = 0;

    SYSF_ioctl( dbg_cap1166_fd, CAP1166_SET_DBG_LED_ON_OFF, (intptr_t)&ctsc_dbg_ioctl_info );
#endif
}


/* 2014/04/22 Harada [Zeus5BK]NV-RAMレス 起動/省電力シーケンスデバッグ対応 */
/* 2020/12/08 OP-BTS_28357 PowerManagerでSegF時の詳細コード改善 */
unsigned char uc_seqCodeGlobal = 0;

#define PM_SEQ_REC_START()		initDebugRunningFlagSet( THRD_OFFSET_INITDEBUG_PM_SEC1, THRD_OFFSET_INITDEBUG_PM_SEC2 )
//#define PM_SEQ_REC( NVdata )	initDebugWrite( 0, 0, NULL, 0, NVdata )
//#define PM_SEQ_REC_END()		initDebugRunningFlagReset()
#define PM_SEQ_REC( NVdata )	uc_seqCodeGlobal = NVdata
#define PM_SEQ_REC_END()		uc_seqCodeGlobal = 0
// ================================================================================================.
//! <!--機能--> 電源管理 制御部  コンストラクタ.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param THRE_CtrID_0.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! \nF.
//! <!--履歴-->.
// 2005/012/07 忠政	新規作成.
// 13/02/28 Sky 機種共通化対応 THRC_PowerManagerSoC::THRC_PowerManagerSoC()から処理を移植.
// 2015/12/18 Sky-Araki CQ520828 Lot26対応.
// 2015/12/25 山口 静的解析 DEF_IT5EMU3のコンパイルSWを削除.
// 2016/11/21 sky_ishiga [MinervaMLK][起動時間短縮] 電源SubCpuタイマー設定対応.
// 2017/4/26 Sky-NAKANISHI AR1019633 主電源OFF後に前ドア開閉するとMFPパネルが起動する問題の対応.
// 2017/10/11 Sky佐藤 S800省電力Lot26対応.
// 2017/10/18 Sky-ishiga [Eagle] [起動時間短縮(再設計)] インターフェース見直し(固有名称(電源SubCPU)を削除).
// ================================================================================================.
THRC_PowerManager::THRC_PowerManager()
{
	static char *table[DEF_MAX_PowerSaveStatusNum] = {
	//   ABCDEFG,      A:HDD&PCIe_BUS, B:PIC, C:USB, D:IR&CPS, E:Fiery F:Panel G:IMGSYS.
		"1111111",	// THRE_PowerSaveStatusFirstBootConfigration			 0: 起動中.
		"1111111",	// THRE_PowerSaveStatusIdle								 1: 待機中.
		"1010101",	// THRE_PowerSaveStatusLowPowerDelay					 2: ローパワー : 画像受信可能.
		"0010001",	// THRE_PowerSaveStatusLowPower							 3: ローパワー : 画像受信不可.
		"1010101",	// THRE_PowerSaveStatusSleepDelay						 4: スリープ : 画像受信可能 / メインリレーON.
		"0000001",	// THRE_PowerSaveStatusSleep							 5: スリープ : 画像受信不可 / メインリレーON.
		"1010001",	// THRE_PowerSaveStatusSubPowerOffDelay					 6: 副電源オフ : 画像受信可能.
		"0000001",	// THRE_PowerSaveStatusSubPowerOff						 7: 副電源オフ : 画像受信不可.
		"1110101",	// THRE_PowerSaveStatusPrtEngOnlyLowPowerCancel			 8: ローパワー : 画像受信可能 / プリント可能.
		"1110101",	// THRE_PowerSaveStatusPrtEngOnlySleepCancel			 9: スリープ : 画像受信可能 / プリント可能.
		"1110111",	// THRE_PowerSaveStatusPrtEngOnlyLowPowerCancelPanelOn	10: ローパワー : 画像受信可能 / プリント可能 / パネルON.
		"1110111",	// THRE_PowerSaveStatusPrtEngOnlySleepCancelPanelOn		11: スリープ : 画像受信可能 / プリント可能 / パネルON.
		"1010101",	// THRE_PowerSaveStatusSleepDelayRelayOff				12: スリープ : 画像受信可能 / メインリレーOFF.
		"0010001",	// THRE_PowerSaveStatusSleepRelayOff					13: スリープ : 画像受信不可 / メインリレーOFF.
		"0000000",	// THRE_PowerSaveStatusSleep15							14: スリープ1.5.
		"1111101",	// THRE_PowerSaveStatusSubPowerOffDelayCpsOn			15: 副電源OFF遅延 CPS動作可能 2011/08/01 CQ:351511 河村 - 追加.
		"0000000",	// THRE_PowerSaveStatusSubPowerOff15					16: 副電源オフ1.5.
	};
//↑USB部分Taiga相当。ローバワー/スリープ遅延でもUSBは使用可能にしている.
	
	ushort i =0 ,j = 0;
	for( i=0; i< DEF_MAX_PowerSaveStatusNum; ++i ) {
		for( j=0; j< DEF_MAX_DeviceNum; ++j ) {
			switch( *(table[i]+j) )
			{
				case '0':
					uc_MFPBoard_PowerSaveTableStruct[i][j] = DEF_PowerSave_Inactive;
					break;
				case '1':
					uc_MFPBoard_PowerSaveTableStruct[i][j] = DEF_PowerSave_Active;
					break;
				default:
					THRC_PM_ASSERT("THRC_PowerManager() tableが異常です", 0,0,0,0,CFA14_DetailCode0x08E4 ); // 2010/03/04 ft_kondo CQ324295.
				break;
			}
		}
	}

	// 初期値設定.
	pc_powerDevice = &THRC_PowerManagerDeviceSingletonIf::newInstance();
	bl_DirActive = False;

	uc_ChangeStatusBefore = (uchar)THRE_PowerSaveStatusNone;
	uc_ChangeStatusAfter  = (uchar)THRE_PowerSaveStatusNone;
	uc_ExcludeUSBHost     = 0;
	msgQID_wdPowerManager = NULL;	// 2012/10/04 河村 - CQ363535 for Watchdog.
	/* 2014/10/4 小林 静的解析 ID:19729 */
	semID_PanelPower = NULL;
	bl_MainSwOFF_Flag = False;		//2017/04/25 Sky-NAKANISI AR1019633 主電源OFF検知用フラグ.
	isStorageUnlock = False;		// 2020/02/14 河村 -  OP_BTS-20183 ストレージのロク解除必要フラグ.
	isResetContDelayFlag = False;	// reset_controller遅延フラグ.	// 2020/04/17 Harada OP_BTS-21547 DeepSleepから主電源OFFでSnapshot取得しない問題対策 ロックパスワード設定時、Shutdownをアンロック後に行う対応.
	
	// 2014/02/20 ft_kondo 省電力復帰制御をパラレルで動かす対応. 
	msgQID_PowerManager_USB = SYSF_msgQCreate( MAX_MSGS, sizeof(THRS_PowerManagerMsg), MSG_Q_FIFO );
	msgQID_PowerManager_Panel = SYSF_msgQCreate( MAX_MSGS, sizeof(THRS_PowerManagerMsg), MSG_Q_FIFO );
	msgQID_PowerManager_Hdd = SYSF_msgQCreate( MAX_MSGS, sizeof(THRS_PowerManagerMsg), MSG_Q_FIFO );
	msgQID_PowerManager_IR = SYSF_msgQCreate( MAX_MSGS, sizeof(THRS_PowerManagerMsg), MSG_Q_FIFO );
	msgQID_PowerManager_Fiery = SYSF_msgQCreate( MAX_MSGS, sizeof(THRS_PowerManagerMsg), MSG_Q_FIFO );
	msgQID_PowerManager_PCIePCIDev = SYSF_msgQCreate( MAX_MSGS, sizeof(THRS_PowerManagerMsg), MSG_Q_FIFO );
	msgQID_PowerManager_Watch_Hdd = SYSF_msgQCreate( MAX_MSGS, sizeof(THRS_PowerManagerMsg), MSG_Q_FIFO );

	e_PM_DeviceStatus = 0x00000000;

	e_PM_SeqReqEndComplete = THRE_PM_SeqReqEndComplete_Complete;	// 省電力シーケンス記録フラグ.	// 2015/01/16 Harada CQ.509010 省電力制御パラレル対応に合わせた省電力シーケンス記録フラグ対応.

	// 2013/08/06 Harada MinervaZeus3rd 静的解析対応(ID:21665). 
	p_PowerCallBack = NULL;

	// for DelayedPowerChangeSequence().
	for( i=0; i<DELAYED_POWER_CHANGE_SEQUENCE_QUE_MAX; i++ ) {
		st_delayedParam[i].ul_actionTick = 0;
	}
	isEngOnlyWakeup = False;
	isSleepDelay = False;
	bl_reqPanelOn = False;
	bl_PowerSavePanel = False;
//Str【VenusMLK CQ.529803 スリープ中動作が失敗する問題】2016.05.23.
	uc_CsetModeExec = 0;
//End【VenusMLK CQ.529803 スリープ中動作が失敗する問題】2016.05.23.
	Lot26HwType= True;
	#if defined(FUM_LINUX)
	/* SIMはLot26非対応 */
	Lot26HwType= False;
	#endif

	/* THRC_PM_LOG()だと出力されないのでprintf()にする */
	// 2018/04/10 ft_kondo 制御構成変更No110 ログリファイン 常にLot26であるため、不要なログと判断(削除). 
//	printf( "(%010u)(BSP) ### Set Lot26HwType to %d ###\n", SYSF_tickGet() , Lot26HwType);
	
// 2016/11/21 sky_ishiga [MinervaMLK][起動時間短縮] 電源OFFタイマー設定対応.
	msgQID_setPowerOffTimer = 0;
	si_taskID_setPowerOffTimer = 0;
	wdogID_SetPowerOffTimer = NULL;

	// 17/05/08 Sky-Araki AR1021331 xhci-hcd Timeout.
	bWakeupUSB_Busy= False;
	
	// 2017/09/26 河村 - Eagle省電力復帰時間短縮対応 .
	bl_isPanelOnTimer = False;
	ul_PanelOnTimer = 0;

	be_TaskMessage = 0;					// メッセージ種別.	// 2017/11/02 Harada 新規作成 AR.1029174 新詳細コード改善 SegF付加情報対応.
	sc_Pcie_Wa_flg = THRE_PCIE_WA_CONDUCT;					// 2022/10/17 OP_BTS-37178 山口 EagleⅢではPCIeのWA処理を行わない.
	return;
}

// ================================================================================================.
//! <!--機能--> 電源管理 制御部  デストラクタ.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
// 2005/012/07 忠政	新規作成.
// 2016/11/21 sky_ishiga [MinervaMLK][起動時間短縮] 電源SubCpuタイマー設定対応.
// ================================================================================================.
THRC_PowerManager::~THRC_PowerManager()
{
	/* 2014/10/4 小林 静的解析 ID:19728 */
	SYSF_msgQDelete( msgQID_PowerManager_USB );
	SYSF_msgQDelete( msgQID_PowerManager_Panel );
	SYSF_msgQDelete( msgQID_PowerManager_Hdd );
	SYSF_msgQDelete( msgQID_PowerManager_IR );
	SYSF_msgQDelete( msgQID_PowerManager_Fiery );
	SYSF_msgQDelete( msgQID_PowerManager_PCIePCIDev );
	SYSF_msgQDelete( msgQID_PowerManager_Watch_Hdd );
	if ( 0 != msgQID_setPowerOffTimer ) {
		SYSF_msgQDelete( msgQID_setPowerOffTimer );
	}
}

// ================================================================================================.
//! <!--機能--> 電源管理 定期起床ハンドラ関数.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! 電源管理タスクの定期起床ハンドラ処理を行う \n.
//! <!--履歴-->.
// 2007/03/26 yokoi	新規作成.
// ================================================================================================.
static void cyclicLoop_taskPowerManager()
{
	// 割り込みバンドラなのでNoWaitで送信.
	int_sendMessage( TID_PowerManeger, TM_TimerWakeUp_Sys, 0L );		// メッセージ「定期起床」.
}

// ================================================================================================.
//! <!--機能--> 電源管理 定期起床開始関数.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! 電源管理タスクの定期起床処理開始を行う \n.
//! <!--履歴-->.
// 2007/03/26 yokoi	新規作成.
// ================================================================================================.
void start_cyclicLoop_taskPowerManager()
{
	if( wdog_ID_PowerManager == NULL ){
		// 初回起動なので、ID生成するのみ.
		wdog_ID_PowerManager		= SYSF_wdCreate();							// ウォッチドッグＩＤ.
	}
	// 再度、定期起床タイマーを起動.
	int32_t sl_Tick  = (int32_t)SYSF_sysClkRateGet() * 100 / 1000;	// 100ｍｓ→ティック数に変換（タイマーを動作させる為、計算結果が０の場合は１ティックとする）.
	if( sl_Tick == 0L ) { sl_Tick = 1L; }
	SYSF_wdStart( wdog_ID_PowerManager, sl_Tick, (FUNCPTR)cyclicLoop_taskPowerManager, 0L );	// 定期起床タイマー起動.
}

// 2022/09/22 OP_BTS-39386 山口 RTC時刻初期化【暫定】.
#define TIME_REGULAR_CHECK 600000
#define TIME_REGULAR_CHECK_STARTUP 60000
// RTCのバックアップ(定期起床用).
static void cyclicLoop_rtc_Time_BackUp()
{
	
	sendMessage( TID_PowerManeger, TM_RTC_Time_BackUpLoop, 0 );
	
}
void rtc_Time_BackUpLoop(void)
{
	if( wdog_ID_rtc_Time_BackUpLoop == NULL ){
		// 初回起動なので、ID生成するのみ.
		wdog_ID_rtc_Time_BackUpLoop		= SYSF_wdCreate();							// ウォッチドッグＩＤ.
	}
	// RTCバックアップ時刻更新.
	CB_RTC::newInstance().setBackUpTime(False , True);
	SYSF_wdStart( wdog_ID_rtc_Time_BackUpLoop, SYSF_sysClkRateGet() * TIME_REGULAR_CHECK/1000, (FUNCPTR)cyclicLoop_rtc_Time_BackUp, 0 );	// 定期起床タイマ起動.

}
// RTCのバックアップ(外部提供用).
void THRC_PowerManager::rtc_Time_BackUp(void){
	/* 2022/11/25 山口 OP_BTS-40732 RTC FAXロールバック正式対応 */
	CB_RTC::newInstance().getRtcFaxBackupData( );
	
}
// RTCのバックアップ(SuperWarp起動用).
// SuperWarp起動時にRTCのバックアップを行うと起動時間に影響がある.
// 1分後にバックアップを行うように対応.
static void rtc_Time_BackUpStart()
{
	
	sendMessage( TID_PowerManeger, TM_RTC_Time_BackUp, 0 );
	
}

void rtc_Time_BackUpStartUp(void)
{
	if( wdog_ID_rtc_Time_BackUpStartUp == NULL ){
		// 初回起動なので、ID生成するのみ.
		wdog_ID_rtc_Time_BackUpStartUp		= SYSF_wdCreate();							// ウォッチドッグＩＤ.
	}
	// RTCバックアップ時刻更新.
	SYSF_wdStart( wdog_ID_rtc_Time_BackUpStartUp, SYSF_sysClkRateGet() * TIME_REGULAR_CHECK_STARTUP/1000, (FUNCPTR)rtc_Time_BackUpStart, 0 );	// 定期起床タイマ起動.

}

// 2022/11/25 山口 OP_BTS-40732 RTC FAXロールバック正式対応 .
// FAX送受信時のRTC時刻をkernelへ通知.
void THRC_PowerManager::setRtcFaxBackupDataRegion( unsigned char *puc_timeBackupFax )
{
	
	int32_t fd = 0, ret = 0;
	
	if(NULL != puc_timeBackupFax ){
	
		fd = SYSF_devopen( "/dev/PMU0", O_RDWR, 0666 );
		if( fd < 0 ){
			printf("%s: SYSF_devopen ERR errno:%d\n", __FUNCTION__, SYSF_errnoGet());
			return;
		}
		ret = SYSF_ioctl(fd, THRD_PMU_IOCSSET_RTCFAXBACKUPDATAAREA, (intptr_t)puc_timeBackupFax);
		if( ret < 0 ){
			printf("%s: SYSF_ioctl ERR errno:%d\n", __FUNCTION__, SYSF_errnoGet());
		}
		SYSF_close(fd);
		
	}

}

/* 2019/01/31 AR1072285 山口 CA72 WDT対応. */
Bool isWdOn_CA72 = False;	// CA72WDT有効フラグ.

// ================================================================================================.
//! <!--機能--> 電源管理 制御部 メッセージループ.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
//  09/08/07 越野	 Donau環境から分離して作成.
//  11/03/29 佐々木 CQ:341055 TM_DecisionEngineTypeNoticeに対応するcase文追加。処理はなにもしない.
// 2011/07/25 CQ:352236 山下 - CQ:344079のコメントアウトマージを解除.
// 13/02/28 Sky 機種共通化対応 THRC_PowerManagerSoC.cppから移植.
// 14/09/18 sky_tksk Fiery省電力対応.
// 15/02/24 ft_kondo initDebugDataDump 記録されない事がある進捗コード修正. 
// 15/04/10 ft_kondo CQ.511976 スリープ遅延、副電源OFF遅延に遷移するとENGが動作している問題. 
// 19/04/04 Sky[IT6_1.0MR2][AR1074753] 電源OFFタイマータスクの常駐化
// 19/06/13 Sky[EagleH][OP_BTS-9198] シグナル番号登録より先ににシグナルハンドラ登録を行う.
// 19/10/08 ft_kondo OP_BTS-14443 CA72 WDTを常時動作させる. 
// 20/04/07 Sky OP_BTS-22077 C-D030対応.
// 20/09/09 Tsukiji OP_BTS-26829 2.0MR3 D030発生問題対応.
// ================================================================================================.
void THRC_PowerManager::messageLoop()
{
	BE_TaskMessage message = 0; uintptr_t option = 0;
#if !defined(FUM_LINUX) /*実機*/
	APIC_SoftDipSw *pc_SoftDipSw = APIC_SoftDipSw::newInstance();
#endif	/*実機*/

#ifdef TORNADO		// ＰＣシミュレータで有効.
	// 2019/08/26 山口 OP_BTS-11998 DeepSleep中にフリーズ.
	int32_t si_Fd = 0;
	int32_t si_Ret2 = 0;
	si_Fd = SYSF_open(THRD_DeviceFile_Pmu, O_RDWR, 0);
	// 2012/10/04 河村 - CQ363535 watchdog初期化.
	// TASK_PowerManagerとタスクプライオリティを同じにすること.
	// 2014/03/24 H.Takahashi スタックサイズ調整対応.
//	SYSF_taskSpawn( "TASK_WD_PowerManager",50, VX_FP_TASK,	25000, (FUNCPTR)THRC_PowerManager::wdMessageLoop, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);	
	SYSF_taskSpawn( "TASK_WD_PowerManager",50, VX_FP_TASK,	50000, (FUNCPTR)THRC_PowerManager::wdMessageLoop, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);	

	// 2021/07/27 ft_kondo OP_BTS-34942 RamDisk内のログ削除用監視処理追加. 
	SYSF_taskSpawn( "TASK_Check_RamDisk",50, VX_FP_TASK,	50000, (FUNCPTR)checkRamDisk_Messages, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);	

	// 2019/11/20 山口 OP_BTS-10123 無線LANデバイスのエラー発生時にリセット処理を行う.
#if !defined(FUM_LINUX) /*実機*/
	SYSF_taskSpawn( "TASK_WLAN_Reset",50, VX_FP_TASK,	50000, (FUNCPTR)THRC_PowerManager::wlan_reset_Loop, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);	
	// 2020/10/28 河村 - OP_BTS-28747 無線LAN切断問題対応 No.3/4.
	if( pc_SoftDipSw != NULL && pc_SoftDipSw->getSoftDipSwBit( 147, 6 ) == True ) { // DipSWNo. 173 bit5(NVRAM：147-6).
		printf( "(%010u)(BSP) SW-DipSW 173 bit5=1 / Enable WLAN reset task2.\n", SYSF_tickGet() );
		SYSF_taskSpawn( "TASK_WLAN_Reset2",150, VX_FP_TASK,	10000, (FUNCPTR)THRC_PowerManager::wlan_reset_Loop2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);	
	} else {
		printf( "(%010u)(BSP) SW-DipSW 173 bit5=0 / Disable WLAN reset task2.\n", SYSF_tickGet() );
	}
#endif	/*実機*/
	/* 2019/01/31 AR1072285 山口 CA72 WDT対応. */
	// 2019/10/08 ft_kondo OP_BTS-14443 CA72 WDTを常時動作させる. 
	isWdOn_CA72 = True;	// CA72WDT有効.
	// プライオリティ1。他のタスクに動作を阻害されないよう高優先度の設定。cuShellと同じ.
	SYSF_taskSpawn( "TASK_WD_CA72",1, VX_FP_TASK,	10000, (FUNCPTR)THRC_PowerManager::wdCA72Loop, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	// 2019/08/26 山口 OP_BTS-11998 DeepSleep中にフリーズ.
	si_Ret2 = ioctl( si_Fd, THRD_PMU_IOCSSET_REBOOT_CRANK, True );
	// 2022/10/17 OP_BTS-37178 山口 EagleⅢではPCIeのWA処理を行わない.
	// PCIe WA 実行判定,結果をkernelにも通知.
	sc_Pcie_Wa_flg = THRG_PcieWaExecution();
	si_Ret2 = ioctl( si_Fd, THRD_PMU_IOCSSET_PCIE_WA_FLG, sc_Pcie_Wa_flg );
	SYSF_close(si_Fd);
#endif	// #ifdef TORNADO.
	
	//電源OFFタイマータスク生成
	CreateTask_SetPowerOffTimer();
	
	// 2022/09/22 OP_BTS-39386 山口 RTC時刻初期化【暫定】.
	cyclicLoop_rtc_Time_BackUp();
	unsigned char uc_timeBackupFaxRegion[7];
	unsigned char uc_timebuf_FF[7];
	memset( uc_timeBackupFaxRegion, 0, sizeof(uc_timeBackupFaxRegion) );
	memset( uc_timebuf_FF, 0xFF, sizeof(uc_timebuf_FF) );
	if ( 0 == THRG_getBspThrRegion( 0x4F, (void*)&uc_timeBackupFaxRegion, sizeof(uc_timeBackupFaxRegion) ) ) {
		if(0 != memcmp(uc_timeBackupFaxRegion , uc_timebuf_FF , sizeof(uc_timeBackupFaxRegion))){
			THRG_setBspThrRegion( 0x4F, uc_timebuf_FF, sizeof(uc_timebuf_FF) );
		}
		
	}

	// 2013/02/14 堀井 CQ.377663 シグナルハンドラ登録をTASK_PowerManagerで行う.
	/*********** 2010/07/14 内野 省電力復帰要因の通知対応. start ***********/
	
//++ 2019/04/24 Sky		【EagleH】Warning除去対応.
#if defined(TORNADO) && !defined(SIMULATOR)			// 実機で有効.
	int32_t fd;
#endif
//-- 2019/04/24 Sky		【EagleH】Warning除去対応.

// Str 2010/09/13 ft_kondo 8536環境Sim対応.
#ifdef TORNADO			// 実機で有効.
	struct sigaction mysig_act;
	// 2012/08/31 ft_kobayashi 静的解析 CID:446938.
	memset( &mysig_act, 0, sizeof(struct sigaction) );

#if defined(TORNADO) && !defined(SIMULATOR)	// 実機で有効.
	// set signal handler.
	mysig_act.sa_flags = SA_SIGINFO;
	mysig_act.sa_sigaction = sig_handler;								//ハンドラの登録.
	sigaction ( SIGRTMIN + 6, &mysig_act, (struct sigaction *)NULL);	//シグナルが通知されたらハンドラが呼ばれる。SIGRTMIN + 6が発生したらここで登録したハンドラがよばれる。
#endif // #if defined(TORNADO) && !defined(SIMULATOR).

#endif // #ifndef TORNADO.
// End 2010/09/13 ft_kondo 8536環境Sim対応.
	int32_t errno;

//++ 2019/04/24 Sky		【EagleH】Warning除去対応.
#if defined(TORNADO) && !defined(SIMULATOR)			// 実機で有効.
	fd = SYSF_open( THRD_DeviceFile_Pmu, O_RDWR, 0  );
#endif
//-- 2019/04/24 Sky		【EagleH】Warning除去対応.

	// 2012/12/05 ft_kobayashi 静的解析 CID-454653.
	// 2011/12/19 ft_kondo 静的解析 CID:10555. 
// Str 2010/09/13 ft_kondo 8536環境Sim対応.
#if defined(TORNADO) && !defined(SIMULATOR)			// 実機で有効.
	//++ 2019/04/24 Sky		【EagleH】Warning除去対応.
	int32_t res;
	struct pmu_ioc_regist reginfo;
	//-- 2019/04/24 Sky		【EagleH】Warning除去対応.

	if( fd < 0 ){
		printf( "(%010u)(BSP) Cannot open file THRC_PowerManager::messageLoop() THRD_DeviceFile_Pmu\n", SYSF_tickGet() );
		return;
	}

	reginfo.pid = syscall(SYS_gettid);									//自分自身のprocessIDの取得 CQ.377663 getpid->gettidに変更.
	reginfo.signo = SIGRTMIN + 6;										//通知で使ってほしいシグナル.
	res = ioctl( fd, THRD_PMU_IOCSPIDREGIST, &reginfo );				//PIDとシグナル番号を指定する。ドライバにシグナルを出してくれと要求.
																		//THRD_PMU_IOCSPIDREGISTはどこにあるか？.hを追加しないといけない？.
	if( res < 0 ){
		printf( "(%010u)(BSP) ioctl error!! THRC_PowerManager::messageLoop() THRD_PMU_IOCSPIDREGIST\n", SYSF_tickGet() );
	}
	// 2012/08/31 ft_kobayashi 静的解析 CID:440998.
	// 2011/12/19 ft_kondo 静的解析 CID:10555. 
	SYSF_close(fd);
	
#endif // #ifndef TORNADO.
// End 2010/09/13 ft_kondo 8536環境Sim対応.
	/*********** 2010/07/14 内野 省電力復帰要因の通知対応. end ***********/

	// 2017/11/02 Harada 新規作成 AR.1029174 新詳細コード改善 SegF付加情報対応.
	// コールバック関数登録API.
	TYPE_FatalCallBackResult e_FatalCallBackResult = TYPE_FatalCallBackResult_OK;					// Result.
	TYPE_TaskDetailCode e_TaskDetailCode = TYPE_TaskDetailCode_TASK_PowerManager;					// タスク番号.
	TYPD_CallBackFunc pf_CallBackFunc = (TYPD_CallBackFunc)THRC_PowerManager::fatalCallBack;		// コールバック関数のポインタ.
	TYPE_FatalCallBackPriority e_Priority = TYPE_FatalCallBackPriority_Owner;						// プライオリティ.
	e_FatalCallBackResult = APIC_setFatalCallBack( e_TaskDetailCode, pf_CallBackFunc, e_Priority );
	if (TYPE_FatalCallBackResult_OK != e_FatalCallBackResult) {
		printf("## %s %d : APIC_setFatalCallBack NG  e_FatalCallBackResult=%d\n", __FUNCTION__, __LINE__, e_FatalCallBackResult );
	}
	
	while( 1 ) {
		if( receiveMessage( TID_PowerManeger, message, option ) == False ) {	// タスク間メッセージの受信.
#ifndef TORNADO		// ＰＣシミュレータで有効.
			return;
#endif	// #ifdef TORNADO.
		}
#if 1 // OP_BTS-22077 ストレージ暗号化が有効の場合に、正常にリカバリできないため処理無効化.
		ST_MsgPowerSaveStatusPara* pst_MsgPowerSaveStatusPara = (ST_MsgPowerSaveStatusPara*)option;
#endif

		be_TaskMessage = message;		// メッセージ種別.	// 2017/11/02 Harada 新規作成 AR.1029174 新詳細コード改善 SegF付加情報対応.
		char c;
		switch( message )
		{
			case TM_TimerWakeUp_Sys:				// 定期起床, 100ms.
				powerDown_ChkError();					// パワーダウン異常検出.
				checkFanLock();							// ファンロック検出.
				DoDelayedPowerChangeSequence();			// 遅延電源ステート変更シーケンス実行.
				SendPmuEvent();							// PMUイベントの通知.
#ifdef THRC_PM_POWER_SWITCH_LOGGING // 2015/10/09 CQ.519888 河村 - 未使用デバッグ機能の無効化.
				batchTest();
#endif
				if( ( uc_ChangeStatusBefore == THRE_PowerSaveStatusNone && uc_ChangeStatusAfter == THRE_PowerSaveStatusNone ) &&
					( e_PM_DeviceStatus == THRE_PM_DeviceStatus_None ) ) {
					// シーケンスが動作していない場合にリフレッシュする.
					// 2015/10/13 CQ.519716 河村 - サブタスクが動作していないことも条件に追加.
					wdRefresh();
				}
				start_cyclicLoop_taskPowerManager();	// 次の定期起床起動.
				break;
			case TM_ReqStartChangePowerSaveStatus:	// 省電力制御要求通知.
#if 1 // OP_BTS-22077 ストレージ暗号化が有効の場合に、正常にリカバリできないため処理無効化.調査用に復活.
/*Str【対応 :省電力移行前にSPIFlashとSSDの値が正常確認、異常ならリカバリする】2020.03.27 Tsukiji */
				switch( pst_MsgPowerSaveStatusPara->uc_ChangeStatusAfter ) {
//LOW POWER除外 OP_BTS-26829 2.0MR3 D030発生問題対応.
//					case THRE_PowerSaveStatusLowPowerDelay:
//					case THRE_PowerSaveStatusLowPower:
					case THRE_PowerSaveStatusSleepDelay:
					case THRE_PowerSaveStatusSleep:
					case THRE_PowerSaveStatusSubPowerOffDelay:
					case THRE_PowerSaveStatusSubPowerOff:
					case THRE_PowerSaveStatusSleep15:
					case THRE_PowerSaveStatusSubPowerOff15:
						//JOBが無い場合に異常チェックとリカバリ処理.
//						THRC_Crypto::newInstance()->checkRecoveryCryptData();
//						printf( "TM_ReqStartChangePowerSaveStatus Recover (%d)\n",pst_MsgPowerSaveStatusPara->uc_ChangeStatusAfter );
						// 20/09/09 Tsukiji OP_BTS-26829 2.0MR3 D030発生問題対応(リカバリはしない/暗号化キー設定フラグクリア).
						//暗号化キー設定フラグクリア.
						THRC_AesTransSingletonIf::newInstance().ResetKeyFlag();
						break;
					default:
						break;
				}
				printf( "TM_ReqStartChangePowerSaveStatus (%d)\n",pst_MsgPowerSaveStatusPara->uc_ChangeStatusAfter );
/*End【対応 :省電力移行前にSPIFlashとSSDの値が正常確認、異常ならリカバリする】2020.03.27 Tsukiji */
#endif
// Str 2010/09/13 ft_kondo 8536環境Sim対応.
#if defined(TORNADO) && !defined(SIMULATOR)			// 実機で有効.
				PowerChangeStart( *((ST_MsgPowerSaveStatusPara*)option) );
#else
                //IT6 SimLinで省電力復帰時に不足しているメッセージを送信する。（OP_BTS-3623)
				if ( THRE_PowerSaveStatusIdle != uc_ChangeStatusBefore)	//変更前ステータスがIdle以外(アクティブでないとき)
				{	// Sim環境で復帰時に送られていない応答メッセージを送信する。
					// IRを起こしたメッセージ. 
					sendMessage( TID_MachineMan, TM_IRDevicePowerSaveCancel, 0 );
					// PICを起こしたメッセージ.
					sendMessage( TID_MachineMan, TM_PicDevicePowerSaveCancel, 0 );
					// CPSを起こしたメッセージ.
					sendMessage( TID_MachineMan, TM_CpsDevicePowerSaveCancel, 0 );
				}
				uc_ChangeStatusBefore = THRE_PowerSaveStatusIdle; // 2重チェックで弾かれないように、None以外のパラメータをセットする.
				SendSequenceEndToPWR();	// PWRへシーケンス完了を通知.
#endif // #ifndef TORNADO.
// End 2010/09/13 ft_kondo 8536環境Sim対応.
				break;
			case TM_FanLockErr:	// FanLockエラー割り込み発生 // 2007/12/20 yokoi add.
				procFanLockErr();
				break;
			case TM_ImageClearEndNotice:			// 2009/07/15 河村 CQ319244 - クリアDMA完了通知.
				THRC_PM_LOG( "TM_ImageClearEndNotice" );
				THRC_BufMemorySingletonIf::newInstance().clearComplete();
				c=2; initDebugWrite( initDebugModule_THR, 2, &c, 1);	/*ASIC200 SDRAMクリア完了*/
				if( THRC_BufMemorySingletonIf::newInstance().isClearSDRAM() == True ){
					THRC_PM_LOG( "Full clear DMA completion" );
					// 2015/01/12 Harada CQ.506130、CQ.508744、CQ.508749 SDRAMクリア完了も待つ対象とする.
					sendMessage( TID_PowerManeger, TM_AnsWakeUpComplete, (uintptr_t)THRE_PM_DeviceStatus_SDRAM ); // TASK_PowerManagerにメッセージを投げる. 
					c=3; initDebugWrite( initDebugModule_THR, 2, &c, 1);	/*ASIC200 SDRAMクリア完了*/
					PowerChangeSequence( THRE_PowerSequenceID_LowToNormal, 3, 0 );
				}
				break;
			case TM_DecisionEngineTypeNotice:				// 2011/03/29 佐々木 CQ:341055.
				//TM_DecisionEngineTypeNoticeに対応する処理が無いため何もしない.
				break;
			// 2014/02/21 ft_kondo 省電力復帰制御をパラレルで動かす対応. 
			case TM_AnsWakeUpComplete:
				if( THRE_PM_DeviceStatus_Hdd != option ){					// HDDの復帰完了は待たないので、処理しない. 
					e_PM_DeviceStatus &= ~option;							// 復帰完了したデバイスのフラグを落とす. 
					if( THRE_PM_DeviceStatus_None == e_PM_DeviceStatus ){ // 各タスクでデバイスの復帰が完了しているかどうか. 
						// 復帰完了していたら、電源部に応答を返す. 
						// 2014/08/19 CQ.501018 sky_tksk PC-Printの子ジョブがHDD認識を待たないようにする対応 start.
						// 省電力復帰制御がパラレル化したことで「HDDを除く省電力解除完了通知」を出すタイミングが無いため修正.
						SendSequencePCPrintToPWR();
						// 2014/08/19 CQ.501018 sky_tksk PC-Printの子ジョブがHDD認識を待たないようにする対応 end.
						SendSequenceEndToPWR();	// PWRへシーケンス完了を通知.
						// 2014/10/16 ft_kondo 省電力シーケンスの最後の処理かどうか. 
						// 2015/01/16 Harada CQ.509010 省電力制御パラレル対応に合わせた省電力シーケンス記録フラグ対応.
						e_PM_SeqReqEndComplete &= ~THRE_PM_SeqReqEndComplete_AllDevExceptHDD;	// 復帰完了したデバイスのフラグを落とす.
						if (THRE_PM_SeqReqEndComplete_Complete == e_PM_SeqReqEndComplete) {		// 省電力シーケンスの最後の処理かどうか.
							PM_SEQ_REC_END();	// 2014/10/16 ft_kondo ここが省電力シーケンスの最後の処理となった場合. 
						}
					} else {
						// まだ制御中との判断の場合は、処理無し. 
						THRC_PM_LOG( "taskPowerManager e_PM_DeviceStatus = [0x%"PRIx32"]\n", e_PM_DeviceStatus );
					}
				}
				// 以下、タスク削除処理. 
				switch( option ){
					case THRE_PM_DeviceStatus_USB:
						SYSF_taskDelete( si_taskID_PM_USB );
						PM_SEQ_REC( 0x47 );		/* CFA14_DetailCode0x0AEE */
						break;
					case THRE_PM_DeviceStatus_PCIePCIDev:
						SYSF_taskDelete( si_taskID_PM_PCIe_PCIDev );
						PM_SEQ_REC( 0x42 );		/* CFA14_DetailCode0x0AE9 */
						break;
					case THRE_PM_DeviceStatus_IR:
						SYSF_taskDelete( si_taskID_PM_IR );
						PM_SEQ_REC( 0x2d );		/* CFA14_DetailCode0x0AD4 */
						break;
					case THRE_PM_DeviceStatus_Fiery:
						SYSF_taskDelete( si_taskID_PM_Fiery );
						PM_SEQ_REC( 0x46 );		/* CFA14_DetailCode0x0AED */
						break;
					case THRE_PM_DeviceStatus_Hdd:
						SYSF_taskDelete( si_taskID_PM_HDD );
						break;
					default:
						break;
				}
				break;
			// 2016/10/31 sky_ishiga [MinervaMLK][起動時間短縮] スレッド省電力制御の並列制御対応.
			case TM_AnsPowerSaveComplete:
				e_PM_DeviceStatus &= ~option;			// 制御完了したデバイスのフラグを落とす.
				switch( option ){
					case THRE_PM_DeviceStatus_USB:
						SYSF_taskDelete( si_taskID_PM_USB );
						break;
					case THRE_PM_DeviceStatus_PCIePCIDev:
						SYSF_taskDelete( si_taskID_PM_PCIe_PCIDev );
						break;
					case THRE_PM_DeviceStatus_IR:
						SYSF_taskDelete( si_taskID_PM_IR );
						break;
					case THRE_PM_DeviceStatus_Fiery:
						SYSF_taskDelete( si_taskID_PM_Fiery );
						break;
					case THRE_PM_DeviceStatus_Hdd:
						SYSF_taskDelete( si_taskID_PM_HDD );
						break;
					default:	// Panelは、終了させない.
						break;
				}
				break;
/* 2014/09/18 sky_tksk for Fiery省電力 start. */
			case TM_NotifyWakeupFromFiery:
				NotifyWakeupFromFiery();
				break;
/* 2014/09/18 sky_tksk for Fiery省電力 end. */
			case TM_HubPortStatusFailed:	//!< Hub Port Status Failed通知.	/* 2021/01/05 Harada IT6 2.1 OP_BTS-30215 認証装置を認証しない *//* 同件として対応 *//* 市場問題 TAS3758 E301 */
				procHubPortStatusFailed((uintptr_t)option);
				break;
			// 2022/09/22 OP_BTS-39386 山口 RTC時刻初期化【暫定】.
			case TM_RTC_Time_BackUpLoop:
				rtc_Time_BackUpLoop();
				break;
			case TM_RTC_Time_BackUp:
				/* 2023/04/06 山口 OP_BTS-44083 起動時もバックアップ時刻の更新は1時間に1度に変更 */
				CB_RTC::newInstance().setBackUpTime(False , True);
				break;
			case TM_NewDetailCode:	// 2022/12/12 河村 - OP_BTS-41054 alignment faultの新詳細コード記録.
				{
					Bool bl_ret = True;

					SYSF_logMsg( "(%010u)(BSP) ## %s %d : setNewDetailCode from kernel.\n", SYSF_tickGet() , __FUNCTION__, __LINE__, 3,4,5,6 );
					bl_ret = APIC_setNewDetailCode( TYPE_FatalDetailCode_undefined, getNewDetailCode() );		// 詳細コード登録.
					if (False == bl_ret) {
						SYSF_logMsg( "(%010u)(BSP) ## %s %d : APIC_setNewDetailCode NG\n", SYSF_tickGet() , __FUNCTION__, __LINE__, 3,4,5,6 );
					}
					if( getNewDetailCodeIsReboot()) {	// リブートする.
						APIC_SystemServiceFunction* p_SystemServiceFunction = APIC_SystemServiceFunction::newInstance();
						if( p_SystemServiceFunction ) {
							{ // 制御構成変更_No.40 リブート回数を数えるカウンタを追加する対応.
								APIC_OperateCnt *pAPIC_OperateCnt = APIC_OperateCnt::newInstance();
								if( NULL!=pAPIC_OperateCnt )
								{
									pAPIC_OperateCnt->inc(TYPE_OperateCnt_Reboot);
								}
							}
							p_SystemServiceFunction->reboot();
						}
					}				
				}
				break;
			default:
				THRC_PM_MSG( "taskPowerManager unknown msg:%d\n", message );
//				ThAssert( False, "taskPowerManager UnKnownMsg %d", message );.
				break;
		}
	}
}

// 2012/10/04 河村 - CQ363535 省電力復帰NG時リブート制御対応.
#define DEF_WDPM_MESSAGE_TIMER		1
#define DEF_WDPM_MESSAGE_REFRESH	2

#define DEF_WDPM_TIMEUP_TIMER		1000	//定期起床間隔.
#define DEF_WDPM_TIMEUP_COUNT		90		//タイムアップとする定期起床回数.

// 2020/11/19 ft_kondo OP_BTS-29116 新詳細コード細分化(22014～). 
#define DEF_WDPM_CPU_OCCUPY_COUNT	30		//CPU占有傾向ありと判断する定期起床回数.
Bool	bl_THRC_CPUOccup = False;			//CPU占有傾向有無フラグ(True:あり、False：なし) 何度もQSPIにアクセスしないようフラグ制御にする. 

#define DEF_WDPM_ISDISABLE_NVRAM_OFFSET (0x07fecd+2)	//オフセットはNVRAM使用領域資料.xls参照.
#define DEF_SLEEP_NG_NVRAM_OFFSET (0x07fecd+3)			// 2012.10.15 内野 CQ373869 FA14詳細コードの書き込み機能追加.

/* 2014/04/22 Harada [Zeus5BK]NV-RAMレス 起動/省電力シーケンスデバッグ対応 */
#define THRD_OFFSET_DETAIL_CODE_REGISTERED	0x000A	/* FA14詳細コード登録済みフラグ */
#define THRD_DETAIL_CODE_REGISTERED			1		/* FA14詳細コード登録済み */
#define THRD_DETAIL_CODE_UNREGISTERED		0		/* FA14詳細コード未登録 */
// 2015/10/13 ft_kondo CQ.519866 B013レジスタ FA14詳細コード対応. 
#define DEF_LOOPCNT_MAX 9
// 2016/11/29 RQ-643 起動時間短縮 電Subタイマーによる電源断確認ログ対応.
// 2017/10/18 Sky-ishiga [Eagle] [起動時間短縮(再設計)] インターフェース見直し(固有名称(電源SubCPU)を削除).
#define THRD_POWEROFFTIMER_LOG_WDOG		(1 << 0)	// bit0: 2秒タイマ(ウォッチドック).
#define THRD_POWEROFFTIMER_LOG_LIMIT	(1 << 1)	// bit1: 1分タイマ(OFF時間上限).
#define THRD_POWEROFFTIMER_LOG_MASK		\
		( THRD_POWEROFFTIMER_LOG_WDOG | THRD_POWEROFFTIMER_LOG_LIMIT )

// 2020/07/29 OP_BTS-25722 PowerManagerのWDでリブート.
Bool isWdStart_CA72 = True;	// CA72WDT Startフラグ.											
void THRC_PowerManager::wdMessageLoop( void )
{
	ushort message;
	int32_t si_res;
	WDOG_ID		wdID_local;
	short count = 0;
	int32_t si_Fd = 0;
	//int32_t si_Ret = 0;		// 2020/07/29 OP_BTS-25722 PowerManagerのWDでリブート.
	uchar uc_code = 0;
	size_t ul_Offset = 0;

#if defined(TORNADO) && !defined(FUM_LINUX) /*実機*/
/* 2014/04/22 Harada [Zeus5BK]NV-RAMレス 起動/省電力シーケンスデバッグ対応 */
#if 0
	uint32_t ul_offsetCurrent = 0;
	uint32_t ul_offsetNext = 0;
	uint32_t ul_pos = 0;
	unsigned char uc_seqCode = 0;
	unsigned short us_FA14DetailCode = 0x0000;
	unsigned char uc_data = THRD_DETAIL_CODE_UNREGISTERED;	/* FA14詳細コード未登録 */
#endif
	// 2020/02/26 山口 OP_BTS-20843 PowerManagerのウォッチドッグのリブート処理発生後フリーズ.
	si_Fd = SYSF_open(THRD_DeviceFile_Pmu, O_RDWR, 0);
	if (0 > si_Fd) {
		printf( "(%010u)(BSP) [%s] open failed. file=%s\n", SYSF_tickGet() , __func__, THRD_DeviceFile_Pmu);
		return;
	}
// 2022/12/06 OP_BTS-41198 山口 SPI-FLASH書き込み低減対応.
// 下記処理はinitDebugDataDumpを無効化しているため現状必要ない.
#if 0
	/* FA14詳細コード登録済みフラグ取得 */
	if ( THRD_GET_SPI_OK != THRG_getBspThrRegion( THRD_OFFSET_DETAIL_CODE_REGISTERED, (void*)&uc_data, sizeof(uc_data) ) ) {
		/* エラー */
		SYSF_logMsg( "## %s %d : THRG_getBspThrRegion ERR\n", __FUNCTION__, __LINE__, 3,4,5,6 );
	}
	if (uc_data == THRD_DETAIL_CODE_UNREGISTERED) {	/* FA14詳細コード未登録 */
		/* 使用領域を決める */
		initDebugGetCurrentSectorOffset( THRD_OFFSET_INITDEBUG_PM_SEC1, THRD_OFFSET_INITDEBUG_PM_SEC2, &ul_offsetCurrent, &ul_offsetNext );
		/* 領域の最後の位置から0xffでない値を検索 */
		ul_pos = initDebugGetLastPos( ul_offsetCurrent );
		/* 値取得 */
		if ( False == THRF_readFlash( ul_offsetCurrent + ul_pos, (void*)&uc_seqCode, sizeof(uc_seqCode) ) ) {
			/* エラー */
			SYSF_logMsg( "## %s %d : THRF_readFlash ERR\n", __FUNCTION__, __LINE__, 3,4,5,6 );
		}
		if ( uc_seqCode != THRD_INITDEBUG_SEQ_POWER_DOWN ) {	/* 電断の記録がない場合 */
			/* 省電力シーケンス進捗コードからFA14詳細コードへの変換 */
			us_FA14DetailCode = initDebugGetFA14DetailCodeFromSeqCode( uc_seqCode );
			if ( us_FA14DetailCode != 0x00 ) {	/* 省電力移行復帰進捗状況(FA14詳細コード)に値が設定されていた場合に登録 */
				SYSF_logMsg( "## 省電力移行復帰停止箇所 : 0x0%x\n", us_FA14DetailCode );
				//THRL_ThAssertLog( us_FA14DetailCode );
				uc_data = THRD_DETAIL_CODE_REGISTERED;	/* FA14詳細コード登録済み */
				if ( THRD_SET_SPI_OK != THRG_setBspThrRegion( THRD_OFFSET_DETAIL_CODE_REGISTERED, (void*)&uc_data, sizeof(uc_data) ) ) {
					/* エラー */
					SYSF_logMsg( "## %s %d : THRG_setBspThrRegion ERR\n", __FUNCTION__, __LINE__, 3,4,5,6 );
				}
			}
		}
	}
#endif
	// 2015/03/23 CQ.511418 ft_kondo 前回のDeepSleep復帰で異常が発生していたかどうかをチェック. 
	// 2015/09/09 ft_kondo CQ.518620 B013/B014レジスタを参照してB013/B014レジスタのBit15が立っていると、正しくwrite出来ない問題対策. 
	FILE *fp;
	char str[10] = {};
	
	if ((fp = popen("cat /sys/bus/platform/devices/PMU.0/devices/B014", "r")) != NULL) {
		fgets(str, sizeof(str), fp);
		pclose(fp);
	}
	
	// Bレジスタを参照し、必要があればFA14詳細コードを記録する. 
	// bit6/bit7はチェックしない. 
	// bit0/bit1は、復帰要因によっては立ったままとなっている. 
	
	// 型変換. 
	int32_t si_B014 = atoi( str );
	
	// bit2のチェック. 
	if( 0 != ( si_B014 & ( 1 << 2 )) ){
		// Bit2が立っていたら、Bit0とBit1だけチェックする. 
		switch( si_B014 & 0x00FF ){
			case 0x00FE:			// Bit0だけ「0」. 
				THRL_ThAssertLog( CFA14_DetailCode0x3001 );									// FA14詳細コードを記録する. 
				SYSF_logMsg( "## Record detailed code : %d\n", CFA14_DetailCode0x3001 );
				break;
			case 0x00FC:			// Bit0とBit1が「0」. 
				THRL_ThAssertLog( CFA14_DetailCode0x3002 );									// FA14詳細コードを記録する. 
				SYSF_logMsg( "## Record detailed code : %d\n", CFA14_DetailCode0x3002 );
				break;
			default:
				break;
		}
	} else {
		// Bit2が立っていなければ、Bit2以降のBitをチェックして、適切なFA14詳細コードを記録する. 
		switch( si_B014 & 0x00F8 ){
			case 0x00F8:			// Bit2だけ「0」. 
				THRL_ThAssertLog( CFA14_DetailCode0x3003 );									// FA14詳細コードを記録する. 
				SYSF_logMsg( "## Record detailed code : %d\n", CFA14_DetailCode0x3003 );
				break;
			case 0x00F0:			// Bit2とBit3が「0」. 
				THRL_ThAssertLog( CFA14_DetailCode0x3004 );									// FA14詳細コードを記録する. 
				SYSF_logMsg( "## Record detailed code : %d\n", CFA14_DetailCode0x3004 );
				break;
			case 0x00E0:			// Bit2とBit3とBit4が「0」. 
				THRL_ThAssertLog( CFA14_DetailCode0x3005 );									// FA14詳細コードを記録する. 
				SYSF_logMsg( "## Record detailed code : %d\n", CFA14_DetailCode0x3005 );
				break;
			default:
				break;
		}
	}
	// RB014に0x00を記録する(初期値を設定する). 
	system( "echo 0 > /sys/bus/platform/devices/PMU.0/devices/B014" );
	
    memset( str, 0, sizeof(str) );
	
	if ((fp = popen("cat /sys/bus/platform/devices/PMU.0/devices/B013", "r")) != NULL) {
		fgets(str, sizeof(str), fp);
		pclose(fp);
	}
	
	// Bレジスタを参照し、必要があればFA14詳細コードを記録する. 
	// bit9/bit10はチェックしない. 
	
	// 型変換. 
	int32_t si_B013 = atoi( str );
	
	// 2015/10/13 ft_kondo CQ.519866 B013レジスタ FA14詳細コード対応. 
	// Bit0から1Bitずつチェックして、1が立っているBitを見つけた時、FA14詳細コードを記録する. 
	// Bレジスタの値が0、もしくはALL1の場合は、何も記録しない. 
	char sc_LoopCnt = 0;
	for( sc_LoopCnt = 0; sc_LoopCnt < DEF_LOOPCNT_MAX; sc_LoopCnt++ ){
		if( si_B013 & 1 << sc_LoopCnt ){
			break;						// 1が立っていたら、その時点でループを抜ける. 
		}
	}
	switch( sc_LoopCnt ){
		case 1:			// Bit0だけ「0」. 
				THRL_ThAssertLog( CFA14_DetailCode0x3006 );									// FA14詳細コードを記録する. 
			SYSF_logMsg( "## Record detailed code : %x\n", CFA14_DetailCode0x3006 );
				break;
		case 2:			// Bit0とBit1が「0」. 
				THRL_ThAssertLog( CFA14_DetailCode0x3007 );									// FA14詳細コードを記録する. 
			SYSF_logMsg( "## Record detailed code : %x\n", CFA14_DetailCode0x3007 );
				break;
		case 3:			// Bit0とBit1とBit2が「0」. 
				THRL_ThAssertLog( CFA14_DetailCode0x3008 );									// FA14詳細コードを記録する. 
			SYSF_logMsg( "## Record detailed code : %x\n", CFA14_DetailCode0x3008 );
				break;
		case 4:			// Bit0とBit1とBit2とBit3が「0」. 
				THRL_ThAssertLog( CFA14_DetailCode0x3009 );									// FA14詳細コードを記録する. 
			SYSF_logMsg( "## Record detailed code : %x\n", CFA14_DetailCode0x3009 );
				break;
		case 5:			// Bit0とBit1とBit2とBit3とBit4が「0」. 
				THRL_ThAssertLog( CFA14_DetailCode0x300A );									// FA14詳細コードを記録する. 
			SYSF_logMsg( "## Record detailed code : %x\n", CFA14_DetailCode0x300A );
				break;
		case 6:			// Bit0とBit1とBit2とBit3とBit4とBit5が「0」. 
				THRL_ThAssertLog( CFA14_DetailCode0x300B );									// FA14詳細コードを記録する. 
			SYSF_logMsg( "## Record detailed code : %x\n", CFA14_DetailCode0x300B );
				break;
		case 7:			// Bit0とBit1とBit2とBit3とBit4とBit5とBit6が「0」. 
				THRL_ThAssertLog( CFA14_DetailCode0x300C );									// FA14詳細コードを記録する. 
			SYSF_logMsg( "## Record detailed code : %x\n", CFA14_DetailCode0x300C );
				break;
		case 8:			// Bit0とBit1とBit2とBit3とBit4とBit5とBit6とBit7が「0」. 
				THRL_ThAssertLog( CFA14_DetailCode0x300D );									// FA14詳細コードを記録する. 
			SYSF_logMsg( "## Record detailed code : %x\n", CFA14_DetailCode0x300D );
				break;
			default:
				break;
	}
                      
	// RB013に0x00を記録する(初期値を設定する). 
	system( "echo 0 > /sys/bus/platform/devices/PMU.0/devices/B013" );

	// 2016/11/29 RQ-643 起動時間短縮 電Subタイマーによる電源断確認ログ対応.
	// 2017/06/06 ishiga 新規作成 [AR1023490] レジスタアクセスを関数化.
	// 2017/10/18 Sky-ishiga [Eagle] [起動時間短縮(再設計)] インターフェース見直し(固有名称(電源SubCPU)を削除).
	// 2018/11/27 Sky-ishiga PS-CPU側が主電源ON時にリセットされる為、ログ取得は不要 .
	// 2018/12/17 Sky-ishiga 通電OFF要因をログ出力する対応.
	uint32_t u32_PowerOffFactor = THRF_getPowerOffFactor();
	THRF_printPowerOffFactor(u32_PowerOffFactor);
	if( 0 != u32_PowerOffFactor) {
		THRF_clrPowerOffFactor();
	}
#endif

	thePowerManagerTask.msgQID_wdPowerManager = SYSF_msgQCreate( MAX_MSGS, MAX_MSG_LEN, MSG_Q_FIFO );
	wdID_local = SYSF_wdCreate();
	SYSF_wdStart( wdID_local, SYSF_sysClkRateGet() * DEF_WDPM_TIMEUP_TIMER/1000, (FUNCPTR)THRC_PowerManager::wdTimer, 0 );	// 定期起床タイマ起動.

	THRC_PM_MSG( "%s start wd.\n", "THRC_PowerManager::wdMessageLoop" );
	while( 1 ) {
		si_res = SYSF_msgQReceive( thePowerManagerTask.msgQID_wdPowerManager, (char*)&message, sizeof(message), WAIT_FOREVER );
		if( si_res == ERROR ) {
#ifdef TORNADO
			continue;
#else
			SYSF_close(si_Fd);		// 2020/02/26 山口 OP_BTS-20843 PowerManagerのウォッチドッグのリブート処理発生後フリーズ.
			return;
#endif
		} else {
			switch( message ) {
			case DEF_WDPM_MESSAGE_TIMER:
				if( count >= 0 ) {	//マイナスの時はカウントしない.
					count++;
				}

				// 2020/11/19 ft_kondo OP_BTS-29116 新詳細コード細分化(22014～). 
				if( count == DEF_WDPM_CPU_OCCUPY_COUNT ){
					uc_code = 1;
					ul_Offset = 0x003f;
					THRG_setBspThrRegion( ul_Offset, &uc_code, sizeof(uc_code) );	// 30s間、動作出来ていない状況を検知したら、CPU占有している傾向有りと判断する. 
					bl_THRC_CPUOccup = True;	// CPU占有傾向あり. 
				}
				if( count >= DEF_WDPM_TIMEUP_COUNT ) {
					// 2020/02/26 山口 OP_BTS-20843 PowerManagerのウォッチドッグのリブート処理発生後フリーズ.
					//uint64_t ull_NewDetailCode = 0x0;
					THRC_PM_MSG( "%s wd expire.\n", "THRC_PowerManager::wdMessageLoop" );
					// 2020/07/29 OP_BTS-25722 PowerManagerのWDでリブート.
					// デバッグログ取得中にフリーズするケースを考慮しCA72のリフレッシュを停止させる.
					isWdStart_CA72 = False;	// CA72WDT Startフラグ.
					count=-1;	//カウント停止.
					// PowerManagerのウォッチドッグのリブート処理はkernel側で実行するように修正.
					//ull_NewDetailCode = (((uint64_t)0x22                  << 56) & 0xff00000000000000)		// モジュール番号.
					//				  | (((uint64_t)0x021                 << 44) & 0x00fff00000000000)		// 通し番号.
					//				  | (((uint64_t)thePowerManagerTask.e_PM_DeviceStatus     << 36) & 0x00000ff000000000)		// e_PM_DeviceStatus.
					//				  | (((uint64_t)thePowerManagerTask.uc_ChangeStatusBefore << 28) & 0x0000000ff0000000)		// 遷移前電源ステータス.
					//				  | (((uint64_t)thePowerManagerTask.uc_ChangeStatusAfter  << 20) & 0x000000000ff00000);		// 遷移後電源ステータス.
					//si_Ret = ioctl( si_Fd, THRD_PMU_IOCSSET_WD_EXPIRE, ull_NewDetailCode );
					thePowerManagerTask.wdExpire();
				}
				SYSF_wdStart( wdID_local, SYSF_sysClkRateGet() * DEF_WDPM_TIMEUP_TIMER/1000, (FUNCPTR)THRC_PowerManager::wdTimer, 0 );	// 定期起床タイマ起動.
				break;
			case DEF_WDPM_MESSAGE_REFRESH:
				count = 0;
				// 2020/11/19 ft_kondo OP_BTS-29116 新詳細コード細分化(22014～). 
				if( bl_THRC_CPUOccup ){			// タイマーリフレッシュ時点でCPU占有傾向ありだった場合、フラグを落とす. 
					uc_code = 0;
					ul_Offset = 0x003f;
					THRG_setBspThrRegion( ul_Offset, &uc_code, sizeof(uc_code) );	// QSPIの特定領域を0で初期化. 
					bl_THRC_CPUOccup = False;	// CPU占有傾向なし. 
				}
				break;
			default:
				THRC_PM_MSG( "%s unknown msg:%d\n", "THRC_PowerManager::wdMessageLoop", message );
				break;
			}
		}
	}
	SYSF_close(si_Fd);			// 2020/02/26 山口 OP_BTS-20843 PowerManagerのウォッチドッグのリブート処理発生後フリーズ.
}

void THRC_PowerManager::wdRefresh( void )
{
	ushort message = DEF_WDPM_MESSAGE_REFRESH;
	SYSF_msgQSend( msgQID_wdPowerManager, (char*)&message, sizeof(message), WAIT_FOREVER, MSG_PRI_NORMAL );
}

uchar guc_isWdDisable = 0;

void wdPowerManagerDisable( uchar isWdDisable )
{
// 2012.10.17 内野 CQ373869 Cランクトラブルリブート設定をコマンドによる切り替え方法に変更.
	isWdDisable = (uchar)(isWdDisable ? 1:0);
#if defined(TORNADO)
	guc_isWdDisable = isWdDisable;
#endif
	if( isWdDisable == 0 ) {	// 無効化解除の場合はカウントを(refreshを使用して)リスタートする.
		thePowerManagerTask.wdRefresh();
		printf( "(%010u)(BSP) Power save sequence error automatic reboot：ON\n", SYSF_tickGet()  );
	} else {
		printf( "(%010u)(BSP) Power save sequence error automatic reboot：OFF\n", SYSF_tickGet()  );
	}
}

void THRC_PowerManager::wdTimer( void )
{
	ushort message = DEF_WDPM_MESSAGE_TIMER;
	SYSF_msgQSend( thePowerManagerTask.msgQID_wdPowerManager, (char*)&message, sizeof(message), WAIT_FOREVER, MSG_PRI_NORMAL );
}

/* 2019/01/31 AR1072285 山口 CA72 WDT対応. */
#define DEF_WDT_CA72_START 		0
#define DEF_WDT_CA72_END 		1
#define DEF_WDT_CA72_REFRESH	2

void THRC_PowerManager::wdCA72Loop( void ){
	
	int32_t si_Fd = 0;
	int32_t si_Ret2 = 0;
	si_Fd = SYSF_open(THRD_DeviceFile_Pmu, O_RDWR, 0);
	if (0 > si_Fd) {
		printf( "(%010u)(BSP) [%s] open failed. file=%s\n", SYSF_tickGet() , __func__, THRD_DeviceFile_Pmu);
	}
	// 2020/07/29 OP_BTS-25722 PowerManagerのWDでリブート.
	// OP_BTS-22858の追加対応.リフレッシュタスク開始前にCA72のウォッチドッグの監視を開始.	
	if(isWdStart_CA72){		// CA72WDT Start後の場合.
		si_Ret2 = ioctl( si_Fd, THRD_PMU_IOCSSET_WDT_CA72, DEF_WDT_CA72_START );	// START.
	}
	if( si_Ret2 < 0 ) {
		printf( "(%010u)(BSP) %s: pmu ioctl(THRD_PMU_IOCSSET_WDT_CA72) Error!!! \n", SYSF_tickGet() , __FUNCTION__ );
	}
	// CA72WDT Start後、定期的にREFRESHの設定を行う.
	while( 1 ) {
		if(isWdStart_CA72){		// CA72WDT Start後の場合.
			si_Ret2 = ioctl( si_Fd, THRD_PMU_IOCSSET_WDT_CA72, DEF_WDT_CA72_REFRESH );	// REFRESH.
		}
		if( si_Ret2 < 0 ) {
			printf( "(%010u)(BSP) %s: pmu ioctl(THRD_PMU_IOCSSET_WDT_CA72) Error!!! \n", SYSF_tickGet() , __FUNCTION__ );
		}
		SYSF_taskDelay(SYSF_sysClkRateGet() * 10000 / 1000);	// 10s wait.
		if(  wdTestCode==10 ) {	// リブートテスト用.
			while(1) {
				SYSF_taskDelay( SYSF_sysClkRateGet() );
			}
		}
	}
	SYSF_close(si_Fd);
}

THRE_ReqShiftType e_ReqShiftType_tmp = THRE_ReqShiftType_DeepSleep;

// 2019/11/20 山口 OP_BTS-10123 無線LANデバイスのエラー発生時にリセット処理を行う.
#define THRD_WLAN_RESET_CNT_MAX		5		// 5*200ms 待つ.
#define THRD_WLAN_RESET_CNT_MAX_2	10		// wait回数.	/* 2020/05/26 Harada OP_BTS-23584 DeepSleep復帰後、Wlanリセット時エラー不具合対応 リセット処理にwait追加 */
extern "C" STATUS mioWlanReset(void);
Bool bl_Wlan_reset_disable_SuperWarp = False;

void THRC_PowerManager::wlan_reset_Loop( void ){
	
	int32_t si_Fd = 0;
	int32_t si_Ret2 = 0;
	int32_t wlan_reset_usbport_flg = 0;	
	uint64_t ull_NewDetailCode = 0x0;
	Bool bl_Wlan_reset_success;
	int32_t wlan_reset_loopcnt;
	int32_t wlan_reset_loopcnt2;
	int32_t bl_ret;
	APIC_Power* pc_ApiPower = APIC_Power::newInstance();
	APIC_SystemServiceFunction* p_SystemServiceFunction = APIC_SystemServiceFunction::newInstance();
    APIC_ActiveJobList *pc_ActiveJobList = APIC_ActiveJobList::newInstance();
	LIBC_List<APIC_Job>     c_List;
	// 2021/01/07 山口 OP_BTS-29944 管理者画面中はWLANリセット行わない.
	APIC_SystemStatus* pc_SystemStatus = APIC_SystemStatus::newInstance();
	si_Fd = SYSF_open(THRD_DeviceFile_Pmu, O_RDWR, 0);
	if (0 > si_Fd) {
		printf( "(%010u)(BSP) [%s] open failed. file=%s\n", SYSF_tickGet() , __func__, THRD_DeviceFile_Pmu);
		return;
	}
	
	// 無線LANのエラーを検知するとリセット処理を行う.
	while( 1 ) {
		bl_Wlan_reset_success = 0;
		si_Ret2 = ioctl( si_Fd, THRD_PMU_IOCSGET_WLANRESET, &wlan_reset_usbport_flg );	
		if( si_Ret2 < 0 ) {
			//2019/12/11 ft-matsunaga 静的解析対応[230100,230101,230431,230432].
			SYSF_close(si_Fd);
			printf( "(%010u)(BSP) %s: pmu ioctl(THRD_PMU_IOCSGET_WLANRESET) Error!!! \n", SYSF_tickGet() , __FUNCTION__ );
			return;
		}
		// 2021/01/07 山口 OP_BTS-29944 管理者画面中はWLANリセット行わない.
		// 無線LANでエラーを検知した場合.
		if( (wlan_reset_usbport_flg != 0)
		  &&(THRE_ReqShiftType_GetSnapShot != e_ReqShiftType_tmp )
		  &&( NULL != pc_SystemStatus )
		  &&( pc_SystemStatus->getAdminModeOnApp() == TYPE_AdminModeAppNon) ){	// 管理者モード中でない.
			pc_ApiPower->setDeepSleepDelay( __FILE__, __LINE__ );		// DeepSleepに移行しないように抑止フラグを立てる.
			// USB3.0がdisconectされてからUSB2.0として認識されるまでに時間がかかることがあるため、1秒間無線LANデバイスが装着されているかチェックする.
			for ( wlan_reset_loopcnt= 0; wlan_reset_loopcnt <  THRD_WLAN_RESET_CNT_MAX; wlan_reset_loopcnt++ ) {
				// 2020/04/22 山口 OP_BTS-21605 無線LANのリセット処理の条件から装着設定を削除.
				if( THRF_isExistWLAN() == True ){											// 無線LAN装着.
					printf( "(%010u)(BSP) %s:  Wlan Reset Execution\n", SYSF_tickGet() , __FUNCTION__ );
				 	bl_Wlan_reset_disable_SuperWarp = True;
					
					if(( False == APIC_SystemInfo::newInstance()->getExNetworkAdapter() )
					 ||( TYPE_NetworkIFMode_SingleEthernet == APIC_ExNetworkSetting::newInstance()->getNetworkIFMode() )
					 ||( TYPE_NetworkIFMode_DualEthernet == APIC_ExNetworkSetting::newInstance()->getNetworkIFMode() ) ) {
						bl_Wlan_reset_success = 1;
						// 2020/11/05 OP_BTS-28999 山口 WLANリセット後DeepSleepへ移行できなくなる.
					 	pc_ApiPower->resetDeepSleepDelay( __FILE__, __LINE__ );		// 抑止フラグクリア.
			 			break;
					}
					
					// 2021/06/08 OP_BTS-32392 山口 Wlan reset失敗時にUSBのunbind/bind処理からやり直す.
				 	for ( wlan_reset_loopcnt2= 0; wlan_reset_loopcnt2 <  THRD_WLAN_RESET_CNT_MAX_2; wlan_reset_loopcnt2++ ) {

						// USB3.0がdisconectされた場合のみ無線LANのポートに対してunbind/bindは行わない.
						if(wlan_reset_usbport_flg != 1 ){
#if ( defined( DEF_SparrowCus ) || defined( DEF_DenebMLKCus ) || defined( DEF_SparrowBKCus ) || defined( DEF_HeliosMLKCus ) )  && !defined( DEF_S800Emu )
							printf( "(%010u)(BSP) %s: #### before unbind\n", SYSF_tickGet() , __FUNCTION__ );
							// 2019/12/18 山口 OP_BTS-18110 無線LANデバイスのUSB2.0ポートに対してもバスリセットを行う.
							system("echo -n 4-1.3 > /sys/bus/usb/drivers/usb/unbind");	// 無線LANに対してunbind.
							system("echo -n 4-0:1.0 > /sys/bus/usb/drivers/hub/unbind");	// オンボードHUB(HUB 4-1)に対してunbind.	//2021/04/14 OP_BTS-32392 山口 オンボードHUB(HUB 4-1)に対してもリセットを行う.
							system("echo -n 3-1.3 > /sys/bus/usb/drivers/usb/unbind");	// 無線LANに対してunbind.
							printf( "(%010u)(BSP) %s: #### after  unbind\n", SYSF_tickGet() , __FUNCTION__ );
							SYSF_taskDelay(SYSF_sysClkRateGet() * 1000 / 1000);	// 1s wait.	/* 2020/05/26 Harada OP_BTS-23584 DeepSleep復帰後、Wlanリセット時エラー不具合対応 リセット処理にwait追加 */
							printf( "(%010u)(BSP) %s: #### after  wait\n", SYSF_tickGet() , __FUNCTION__ );
							system("echo -n 4-0:1.0 > /sys/bus/usb/drivers/hub/bind");		// オンボードHUB(HUB 4-1)に対してbind.		//2021/04/14 OP_BTS-32392 山口 オンボードHUB(HUB 4-1)に対してもリセットを行う.
							system("echo -n 4-1.3 > /sys/bus/usb/drivers/usb/bind");	// 無線LANに対してbind.
							system("echo -n 3-1.3 > /sys/bus/usb/drivers/usb/bind");	// 無線LANに対してbind.
							printf( "(%010u)(BSP) %s: #### after  bind\n", SYSF_tickGet() , __FUNCTION__ );
#else 
							printf( "(%010u)(BSP) %s: #### before unbind\n", SYSF_tickGet() , __FUNCTION__ );
							system("echo -n 4-1.2 > /sys/bus/usb/drivers/usb/unbind");	// 無線LANに対してunbind.
							system("echo -n 4-0:1.0 > /sys/bus/usb/drivers/hub/unbind");	// オンボードHUB(HUB 4-1)に対してunbind.	//2021/04/14 OP_BTS-32392 山口 オンボードHUB(HUB 4-1)に対してもリセットを行う.
							system("echo -n 3-1.2 > /sys/bus/usb/drivers/usb/unbind");	// 無線LANに対してunbind.
							printf( "(%010u)(BSP) %s: #### after  unbind\n", SYSF_tickGet() , __FUNCTION__ );
							SYSF_taskDelay(SYSF_sysClkRateGet() * 1000 / 1000);	// 1s wait.	/* 2020/05/26 Harada OP_BTS-23584 DeepSleep復帰後、Wlanリセット時エラー不具合対応 リセット処理にwait追加 */
							printf( "(%010u)(BSP) %s: #### after  wait\n", SYSF_tickGet() , __FUNCTION__ );
							system("echo -n 4-0:1.0 > /sys/bus/usb/drivers/hub/bind");		// オンボードHUB(HUB 4-1)に対してbind.		//2021/04/14 OP_BTS-32392 山口 オンボードHUB(HUB 4-1)に対してもリセットを行う.
							system("echo -n 4-1.2 > /sys/bus/usb/drivers/usb/bind");	// 無線LANに対してbind.
							system("echo -n 3-1.2 > /sys/bus/usb/drivers/usb/bind");	// 無線LANに対してbind.
							printf( "(%010u)(BSP) %s: #### after  bind\n", SYSF_tickGet() , __FUNCTION__ );
#endif
						}
						SYSF_taskDelay(SYSF_sysClkRateGet() * 1000 / 1000);	// 1s wait.	/* 2020/05/26 Harada OP_BTS-23584 DeepSleep復帰後、Wlanリセット時エラー不具合対応 リセット処理にwait追加 */
						printf( "(%010u)(BSP) %s: #### after  wait\n", SYSF_tickGet() , __FUNCTION__ );
						// 無線LANモジュールのリセット.
						printf( "(%010u)(BSP) %s: #### before mioWlanReset  wlan_reset_loopcnt2=%d\n", SYSF_tickGet() , __FUNCTION__, wlan_reset_loopcnt2 );
					 	si_Ret2 = mioWlanReset();
						printf( "(%010u)(BSP) %s: #### after  mioWlanReset  si_Ret2=%d\n", SYSF_tickGet() , __FUNCTION__, si_Ret2 );
				 		if(si_Ret2 == OK){
							printf( "(%010u)(BSP) %s: #### after  mioWlanReset  if(si_Ret2 == OK)\n", SYSF_tickGet() , __FUNCTION__ );
							bl_Wlan_reset_success = 1;
				 			break;
				 		}
						printf( "(%010u)(BSP) %s: #### after  mioWlanReset Error!!!  si_Ret2=%d\n", SYSF_tickGet() , __FUNCTION__, si_Ret2 );
				 		SYSF_taskDelay(SYSF_sysClkRateGet() * 5);	// 1s wait → 5s wait.	/* 2020/05/26 Harada OP_BTS-23584 DeepSleep復帰後、Wlanリセット時エラー不具合対応 リセット処理にwait追加 */
				 	}
					// 2023/04/11 TAS000000014292 山口 Wait中に無線LANのエラーが発生するケースがあるため、Wait処理を削除.
					//SYSF_taskDelay(SYSF_sysClkRateGet() * 60);// 60s wait.
					THRF_fixWLANDevice();
					if( THRF_isExistWLAN() == False ){												// 無線LAN装着.
						bl_Wlan_reset_success = 0;
						break;
					}
					pc_ApiPower->resetDeepSleepDelay( __FILE__, __LINE__ );		// 抑止フラグクリア.
					break;
				}
				SYSF_taskDelay(SYSF_sysClkRateGet() * 200 / 1000);	// 200ms wait.
			}
			// 無線LANを認識していなかった場合詳細コードを登録しリブート.
			if(bl_Wlan_reset_success == 0 ){
				ull_NewDetailCode = (((uint64_t)0x22			<< 56) & 0xff00000000000000)		// モジュール番号.
								  | (((uint64_t)0x013			<< 44) & 0x00fff00000000000)		// 通し番号.
								  | (((uint64_t)3				<< 40) & 0x00000f0000000000);		// IOエラーを起こした箇所の通し番号.
				
				bl_ret = APIC_setNewDetailCode( TYPE_FatalDetailCode_undefined, ull_NewDetailCode );		// 詳細コード登録.
				if (False == bl_ret) {
					SYSF_logMsg( "## %s %d : APIC_setNewDetailCode NG\n", __FUNCTION__, __LINE__, 3,4,5,6 );
				}	
				if (p_SystemServiceFunction){
					
					while(1){
						if( pc_ActiveJobList->getList(TYPE_ActiveJobListPrint, 1, TYPD_MaxActiveJob, &c_List) < 1 ) {
							{ // 制御構成変更_No.40 リブート回数を数えるカウンタを追加する対応.
								APIC_OperateCnt *pAPIC_OperateCnt = APIC_OperateCnt::newInstance();
								if( NULL!=pAPIC_OperateCnt )
								{
									pAPIC_OperateCnt->inc(TYPE_OperateCnt_Reboot);
								}
							}
							p_SystemServiceFunction->reboot();
						}
						SYSF_taskDelay(SYSF_sysClkRateGet() * 5);	// 5s wait.	
					}
				}
			}
		}
	}
	SYSF_close(si_Fd);
}

UINT32	getNetworkIFType( void )
{
	MioDataDesc dataDesc;
	UINT32 uiValue;
	dataDesc.objCls = MIO_OBJCLS_DEVICE;
	dataDesc.objId = MIO_OBJID_NWIF_CONFIG;
	dataDesc.objType = MIO_OBJTYP_UINT32;
	dataDesc.pDataItm = (void*)&uiValue;
	dataDesc.Len     = sizeof(UINT32);
	mioGetDataObj(&dataDesc);
	return uiValue;
}

// 2020/10/28 河村 - OP_BTS-28747 無線LAN切断問題対応 No.3/4 PING確認を行い送信NGの場合リブート.
extern char *pt_mioPing;
#include "system/nvd/APIC_Setting_Network.h"
void THRC_PowerManager::wlan_reset_Loop2( void ){
	
	int32_t si_Ping_NGcnt = 0;
	Bool bl_link_check = False;
	APIC_SystemServiceFunction* p_SystemServiceFunction = APIC_SystemServiceFunction::newInstance();
    APIC_ActiveJobList *pc_ActiveJobList = APIC_ActiveJobList::newInstance();
	LIBC_List<APIC_Job>     c_List;
	APIC_TcpIpSetting			*pc_TcpIpSetting	= APIC_TcpIpSetting::newInstance();
	APIC_LdapSetting *pc_ldapSetting = APIC_LdapSetting::newInstance(4);	// 2020/12/25 河村 - OP_BTS-30331 LDAP5番目から4番目に変更.
	char	sc_Host[20];
	uint64_t ull_NewDetailCode = 0x0;
	int32_t bl_ret;
	static char *ip0="0.0.0.0";
	
	while( 1 ) {
		switch( getNetworkIFType() ) {
		case MIO_NWIF_SINGLE_STA:        //シングルIF 構成無線子機モード.
			// デフォルトゲートウェイのアドレスを取得.
			if( pc_TcpIpSetting ) {
				pc_TcpIpSetting->getGatewayAddress( sc_Host );
			} else {
				strncpy( sc_Host, ip0 , sizeof(sc_Host)-1 );
				sc_Host[sizeof(sc_Host)-1]='\0';
			}
			break;
		case MIO_NWIF_DUAL_ETHERNET_STA: //デュアルIF 構成有線+無線子機モード.
			// LDAPサーバー5番目のIPアドレスを参照.
			if( pc_ldapSetting ) {
				pc_ldapSetting->getServerAddressUTF8( sc_Host );
			} else {
				strncpy( sc_Host, ip0 , sizeof(sc_Host)-1 );
				sc_Host[sizeof(sc_Host)-1]='\0';
			}
			break;
		case MIO_NWIF_SINGLE_ETHERNET:   //シングルIF 構成有線モード.
		case MIO_NWIF_DUAL_ETHERNET_AP:  //デュアルIF 構成有線+無線APモード.
		case MIO_NWIF_DUAL_ETHERNET_WFD: //デュアルIF 構成有線+Wi-Fi Direct.
		default:
			strncpy( sc_Host, ip0 , sizeof(sc_Host)-1 );
			sc_Host[sizeof(sc_Host)-1]='\0';
			break;
		}
		printf("(%010u)(BSP) ## %s : netWorkType:0x%x host:%s\n", SYSF_tickGet(), __FUNCTION__, getNetworkIFType(), sc_Host );
		// デフォルトゲートウェイが未設定(0.0.0.0)の場合はPING送信しない.
		if( !strncmp(sc_Host , ip0 ,sizeof(sc_Host) )){
			bl_link_check = False;
			si_Ping_NGcnt = 0;
		}
		// デフォルトゲートウェイへPING送信.
		else if(( pt_mioPing != NULL )
		 &&( (reinterpret_cast<STATUS(*)(char *,int,uint32_t)>(pt_mioPing))( sc_Host, 1, 0x9 ) == OK ))
		{
			// PING送信OKなら監視開始.
			bl_link_check = True;
			si_Ping_NGcnt = 0;
		}
		else if(bl_link_check == True){
			// 監視開始後、PING送信NG.
			si_Ping_NGcnt++;
		}
		//PING送信NG後、5分経過.
		if(si_Ping_NGcnt > 10){
			while(1){
				//アクティブジョブがなくなるまで待機.
				if( pc_ActiveJobList->getList(TYPE_ActiveJobListPrint, 1, TYPD_MaxActiveJob, &c_List) < 1 ) {
					//詳細コード登録.
					ull_NewDetailCode = (((uint64_t)0x22			<< 56) & 0xff00000000000000)		// モジュール番号.
									  | (((uint64_t)0x013			<< 44) & 0x00fff00000000000)		// 通し番号.
									  | (((uint64_t)4				<< 40) & 0x00000f0000000000);		// IOエラーを起こした箇所の通し番号.
					
					bl_ret = APIC_setNewDetailCode( TYPE_FatalDetailCode_undefined, ull_NewDetailCode );		// 詳細コード登録.
					if (False == bl_ret) {
						SYSF_logMsg( "## %s %d : APIC_setNewDetailCode NG\n", __FUNCTION__, __LINE__, 3,4,5,6 );
					}
					{ // 制御構成変更_No.40 リブート回数を数えるカウンタを追加する対応.
						APIC_OperateCnt *pAPIC_OperateCnt = APIC_OperateCnt::newInstance();
						if( NULL!=pAPIC_OperateCnt )
						{
							pAPIC_OperateCnt->inc(TYPE_OperateCnt_Reboot);
						}
					}
					p_SystemServiceFunction->reboot();
				}
				SYSF_taskDelay(SYSF_sysClkRateGet() * 5);	// 5s wait.			
			}
		}
		SYSF_taskDelay(SYSF_sysClkRateGet() * 30);	// 30s wait.			
	}
}

/* 2020/04/27 山口 OP_BTS-22870 スリープ復帰ランニング中リブート発生(Phison製SSDスクリーニング品). */
int32_t si_Fd_Dleay = -1;
extern int32_t THRF_taskBind_Quad( THRE_IDTYPE e_IdType, uint32_t ul_TaskId, int cpuMsk );
void THRC_PowerManager::MC_PWR_EN_DelayCore1( void ){
	
	if(si_Fd_Dleay == -1){
		si_Fd_Dleay = SYSF_open(THRD_DeviceFile_Pmu, O_RDWR, 0);
	}
	int32_t i_Ret = 0;
	/* Core1に割り当て */
	i_Ret = THRF_taskBind_Quad( THRE_IDTYPE_THREAD, 0, THRE_CPUID_1 );
	if(i_Ret == -1){		/* 2020/05/09 山口 OP_BTS-23434 タスク情報取得失敗でSegF */
		return;
	}

	/* Core1のDelay処理 */
	i_Ret = ioctl( si_Fd_Dleay, THRD_PMU_IOCSSET_MC_PWR_EN_DELAY, 0 );
	
}

void THRC_PowerManager::MC_PWR_EN_DelayCore2( void ){

	int32_t i_Ret = 0;
	if(si_Fd_Dleay == -1){
		si_Fd_Dleay = SYSF_open(THRD_DeviceFile_Pmu, O_RDWR, 0);
	}
	/* Core2に割り当て */
	i_Ret = THRF_taskBind_Quad( THRE_IDTYPE_THREAD, 0, THRE_CPUID_2 );
	if(i_Ret == -1){		/* 2020/05/09 山口 OP_BTS-23434 タスク情報取得失敗でSegF */
		return;
	}

	/* Core2のDelay処理 */
	i_Ret = ioctl( si_Fd_Dleay, THRD_PMU_IOCSSET_MC_PWR_EN_DELAY, 0 );
	
}

void THRC_PowerManager::MC_PWR_EN_DelayCore3( void ){
	
	int32_t i_Ret = 0;
	if(si_Fd_Dleay == -1){
		si_Fd_Dleay = SYSF_open(THRD_DeviceFile_Pmu, O_RDWR, 0);
	}
	/* Core3に割り当て */
	i_Ret = THRF_taskBind_Quad( THRE_IDTYPE_THREAD, 0, THRE_CPUID_3 );
	if(i_Ret == -1){		/* 2020/05/09 山口 OP_BTS-23434 タスク情報取得失敗でSegF */
		return;
	}
	
	/* Core3のDelay処理 */
	i_Ret = ioctl( si_Fd_Dleay, THRD_PMU_IOCSSET_MC_PWR_EN_DELAY, 0 );
	
}
/* Delay用のタスク生成処理 */
void THRC_PowerManager::MC_PWR_EN_DelayTaskSpawn( int e_CpuId ){
	
	
	switch(e_CpuId){
		
		case 1:
			SYSF_taskSpawn( "MC_PWR_EN_DelayCore1",1, VX_FP_TASK,	50000, (FUNCPTR)THRC_PowerManager::MC_PWR_EN_DelayCore1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);	
			break;
		case 2:
			SYSF_taskSpawn( "MC_PWR_EN_DelayCore2",1, VX_FP_TASK,	50000, (FUNCPTR)THRC_PowerManager::MC_PWR_EN_DelayCore2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);	
			break;
		case 3:
			SYSF_taskSpawn( "MC_PWR_EN_DelayCore3",1, VX_FP_TASK,	50000, (FUNCPTR)THRC_PowerManager::MC_PWR_EN_DelayCore3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);	
			break;
		default:
			break;
	}
	
}

void THRC_PowerManager::linkCheck(void)
{
	int32_t i_Ret = 0;
	if(si_Fd_Dleay == -1){
		si_Fd_Dleay = SYSF_open(THRD_DeviceFile_Pmu, O_RDWR, 0);
	}
	/* PCIe リンクチェックの実行 */
	i_Ret = ioctl( si_Fd_Dleay, THRD_PMU_IOCSLINKCHECK, 0 );
}

// 2015/09/11 Sky_Araki CQ518543 FAX受信後、本体フリーズ.
/********************************************************************************/
/* 機能: ｐｓ情報関連処理.														*/
/********************************************************************************/
#define CMD_PS_TO_CONSOLE	"ps H -eo comm:20,pid,tid,pgid,class,rtprio,ni,pri,psr,pcpu,state,stackp,esp,eip,wchan:15,psr | sort -rk 8"
#define CMD_CAT_SSD_FILE	"cat "SSD_FILE
// 2018/04/17 制御構成変更No.99 ストレージ構成変更 山口.
#define THRD_DEBUG_PS_uSD				"/ata0a/DebugLog/PsInfo.dat"
#define THRD_DEBUG_PMLOG_uSD			"/ata0a/DebugLog/PM_WdDebug.dat"
#define THRD_DEBUG_DMESG_uSD			"/ata0a/DebugLog/PM_Wd_dmesg.gz"
#define THRD_DEBUG_TASKMONITOR_uSD	"/ata0a/DebugLog/PM_WdTask.gz"
#define THRD_DEBUG_PS_SSD				"/ata01/DebugLog/PsInfo.dat"
#define THRD_DEBUG_PMLOG_SSD			"/ata01/DebugLog/PM_WdDebug.dat"
#define THRD_DEBUG_DMESG_SSD			"/ata01/DebugLog/PM_Wd_dmesg.gz"
#define THRD_DEBUG_TASKMONITOR_SSD	"/ata01/DebugLog/PM_WdTask.gz"
#define CMD_PS_TO_SSD_FILE_uSD	"ps H -eo comm:20,pid,tid,pgid,class,rtprio,ni,pri,psr,pcpu,state,stackp,esp,eip,wchan:15,psr | sort -rk 8 > "THRD_DEBUG_PS_uSD
#define CMD_PS_TO_SSD_FILE_SSD	"ps H -eo comm:20,pid,tid,pgid,class,rtprio,ni,pri,psr,pcpu,state,stackp,esp,eip,wchan:15,psr | sort -rk 8 > "THRD_DEBUG_PS_SSD

extern void StopTm();
extern void StartTmB_SMP();
extern void tt(char* taskname);		// 2019/07/10 OP_BTS-3440 山口 wdExpire時にタスクトレース出力.

// 2018/04/17 制御構成変更No.99 ストレージ構成変更 山口.
// SSD/uSD装着によって保存するパーティションを変更.
void Save_PsInfo(void)
{
	
	FILE *fp;
	Bool  bl_ConnectSSD= APIC_HddInfo::newInstance()->getHddExist();		// SSD装着設定.
	//++ 2019/04/24 Sky		【EagleH】Warning除去対応.
	int32_t		si_Result = 0;
	//-- 2019/04/24 Sky		【EagleH】Warning除去対応.
	
	// SSD/uSDへデバッグログを保存する.
	if(bl_ConnectSSD){
		//++ 2019/04/24 Sky		【EagleH】Warning除去対応.
		si_Result = system(CMD_PS_TO_SSD_FILE_SSD);
		if (si_Result != 0){
			printf("(%010u)(BSP) ## %s[%d] %s\n", SYSF_tickGet(), __FILE__, __LINE__, __FUNCTION__);
		}
		si_Result = system("dmesg | tail -c 512k | gzip > "THRD_DEBUG_DMESG_SSD);
		if (si_Result != 0){
			printf("(%010u)(BSP) ## %s[%d] %s\n", SYSF_tickGet(), __FILE__, __LINE__, __FUNCTION__);
		}
		si_Result = system("cat /proc/kmtm/info | gzip > "THRD_DEBUG_TASKMONITOR_SSD);
		if (si_Result != 0){
			printf("(%010u)(BSP) ## %s[%d] %s\n", SYSF_tickGet(), __FILE__, __LINE__, __FUNCTION__);
		}
		//-- 2019/04/24 Sky		【EagleH】Warning除去対応.
	} else {
		//++ 2019/04/24 Sky		【EagleH】Warning除去対応.
		si_Result = system(CMD_PS_TO_SSD_FILE_uSD);
		if (si_Result != 0){
			printf("(%010u)(BSP) ## %s[%d] %s\n", SYSF_tickGet(), __FILE__, __LINE__, __FUNCTION__);
		}
		si_Result = system("dmesg | tail -c 512k | gzip > "THRD_DEBUG_DMESG_uSD);
		if (si_Result != 0){
			printf("(%010u)(BSP) ## %s[%d] %s\n", SYSF_tickGet(), __FILE__, __LINE__, __FUNCTION__);
		}
		si_Result = system("cat /proc/kmtm/info | gzip > "THRD_DEBUG_TASKMONITOR_uSD);
		if (si_Result != 0){
			printf("(%010u)(BSP) ## %s[%d] %s\n", SYSF_tickGet(), __FILE__, __LINE__, __FUNCTION__);
		}
		//-- 2019/04/24 Sky		【EagleH】Warning除去対応.
	}
	
	// 2017/02/09 AR.1012381 山口 ウォッチドックのリブート時に調査用のログをeMMCに保存する.
	if(bl_ConnectSSD){
		fp = fopen( THRD_DEBUG_PMLOG_SSD, "w" );
	} else {
		fp = fopen( THRD_DEBUG_PMLOG_uSD, "w" );
	}
		
	if( NULL != fp ){	// 2017/3/1 堀井 静的解析対応.
		for( int i=0; i< (isLooped == True ? THRC_PM_DBGMSG_MAX : msgNum); i++ )
		{
			fprintf( fp, "%s\n", msgStr[i] );
		}
		fclose( fp );
	}else{
		if(bl_ConnectSSD){
			printf("%s : %s fopen error\n", __FUNCTION__, THRD_DEBUG_PMLOG_SSD);
		} else {
			printf("%s : %s fopen error\n", __FUNCTION__, THRD_DEBUG_PMLOG_uSD);
		}
	}
	
	// 同期させる.
	sync();
	
	// トラブルコード発生.
	// 2015/10/21 Harada 新詳細コード対応.
//	THRL_ThAssertLog(CFA14_DetailCode0x200A);

	return;	
}

void Print_PsInfo(void)
{
	// catで内容吐き出し.
	//system(CMD_CAT_SSD_FILE);
	
	return;
}
// 2015/09/11 Sky_Araki CQ518543 FAX受信後、本体フリーズ.


// 2017/02/09 AR.1012381 山口 ウォッチドックのリブート時に調査用のログをUSBメモリにコピー.
// 2018/04/17 制御構成変更No.99 ストレージ構成変更 山口.
// SSD/uSD装着によって取得するパーティションを変更.
void Print_PM_WdDebug(void)
{
	//++ 2019/04/24 Sky		【EagleH】Warning除去対応.
	int32_t		si_Result = 0;
	//-- 2019/04/24 Sky		【EagleH】Warning除去対応.

	Bool  bl_ConnectSSD= APIC_HddInfo::newInstance()->getHddExist();		// SSD装着設定.

	if(bl_ConnectSSD){
		//++ 2019/04/24 Sky		【EagleH】Warning除去対応.
		si_Result = system("cp "THRD_DEBUG_PS_SSD" /bd/sdc1/");
		if (si_Result != 0){
			printf("(%010u)(BSP) ## %s[%d] %s\n", SYSF_tickGet(), __FILE__, __LINE__, __FUNCTION__);
		}
		si_Result = system("cp "THRD_DEBUG_PMLOG_SSD" /bd/sdc1/");
		if (si_Result != 0){
			printf("(%010u)(BSP) ## %s[%d] %s\n", SYSF_tickGet(), __FILE__, __LINE__, __FUNCTION__);
		}
		si_Result = system("cp "THRD_DEBUG_DMESG_SSD" /bd/sdc1/");
		if (si_Result != 0){
			printf("(%010u)(BSP) ## %s[%d] %s\n", SYSF_tickGet(), __FILE__, __LINE__, __FUNCTION__);
		}
		si_Result = system("cp "THRD_DEBUG_TASKMONITOR_SSD" /bd/sdc1/");
		if (si_Result != 0){
			printf("(%010u)(BSP) ## %s[%d] %s\n", SYSF_tickGet(), __FILE__, __LINE__, __FUNCTION__);
		}
		//-- 2019/04/24 Sky		【EagleH】Warning除去対応.
	} else {
		//++ 2019/04/24 Sky		【EagleH】Warning除去対応.
		si_Result = system("cp "THRD_DEBUG_PS_uSD" /bd/sdc1/");
		if (si_Result != 0){
			printf("(%010u)(BSP) ## %s[%d] %s\n", SYSF_tickGet(), __FILE__, __LINE__, __FUNCTION__);
		}
		si_Result = system("cp "THRD_DEBUG_PMLOG_uSD" /bd/sdc1/");
		if (si_Result != 0){
			printf("(%010u)(BSP) ## %s[%d] %s\n", SYSF_tickGet(), __FILE__, __LINE__, __FUNCTION__);
		}
		si_Result = system("cp "THRD_DEBUG_DMESG_uSD" /bd/sdc1/");
		if (si_Result != 0){
			printf("(%010u)(BSP) ## %s[%d] %s\n", SYSF_tickGet(), __FILE__, __LINE__, __FUNCTION__);
		}
		si_Result = system("cp "THRD_DEBUG_TASKMONITOR_uSD" /bd/sdc1/");
		if (si_Result != 0){
			printf("(%010u)(BSP) ## %s[%d] %s\n", SYSF_tickGet(), __FILE__, __LINE__, __FUNCTION__);
		}
		//-- 2019/04/24 Sky		【EagleH】Warning除去対応.
	}
	// 同期させる.
	sync();
}

// 2020/12/25 ft_kondo OP_BTS-28469 型を修正. 
extern uint32_t pm_delay;
extern "C" void pmDevicesState(void);

// FWリファクタリング APIC_Setting.hの分割.
#include "system/nvd/APIC_Setting_Network.h"
#include "system/nvd/APIC_Setting_SoftDipSw.h"
#include "system/nvd/APIC_Setting_SystemSetting.h"


void THRC_PowerManager::wdExpire( void )
{
	uchar isWdDisable = 0;
	//++ 2019/04/24 Sky		【EagleH】Warning除去対応.
	int32_t		si_Result = 0;
	//-- 2019/04/24 Sky		【EagleH】Warning除去対応.

	// 2020/07/29 OP_BTS-25722 PowerManagerのWDでリブート.
	// デバッグログを収集する前に詳細コードを登録する.
	// 2016/06/14 PS1-ft_kobayashi CQ.529697 【詳細コード対応】200A.
	// 2016/02/22 堀井 CQ.524679 新詳細コード無効化.
	// 2015/10/21 Harada 新詳細コード対応 >>.
#if defined(TORNADO) && !defined(FUM_LINUX) /*実機*/
	unsigned short us_DetailCode = CFA14_DetailCode0x200A;
	uint64_t ull_NewDetailCode = 0x0;
	Bool bl_ret = True;
	THRC_PowerManagerDevice *pc_powerDevice = &THRC_PowerManagerDeviceSingletonIf::newInstance();	// デバイス制御部インスタンス.
	ull_NewDetailCode = (((uint64_t)0x22                  << 56) & 0xff00000000000000)		// モジュール番号.
					// 2019/07/10 ft_kondo OP_BTS-7750 新詳細コード強化. 
					  | (((uint64_t)0x01A                 << 44) & 0x00fff00000000000)		// 通し番号.
					  | (((uint64_t)e_PM_DeviceStatus     << 36) & 0x00000ff000000000)		// e_PM_DeviceStatus.
					  | (((uint64_t)uc_ChangeStatusBefore << 31) & 0x0000000f80000000)		// 遷移前電源ステータス.
					  | (((uint64_t)uc_ChangeStatusAfter  << 26) & 0x000000007c000000)		// 遷移後電源ステータス.
					  | (((uint64_t)(THRC_PowerStateID_NORMAL == pc_powerDevice->GetPowerState( THRC_PowerDeviceID_IR ) ? 1 : 0)                     << 25) & 0x0000000002000000)		// 現在の電源ステート取得.
					  | (((uint64_t)(THRC_PowerStateID_NORMAL == pc_powerDevice->GetPowerState( THRC_PowerDeviceID_IR_RY ) ? 1 : 0)                  << 24) & 0x0000000001000000)		// 現在の電源ステート取得.
					  | (((uint64_t)(THRC_PowerStateID_NORMAL == pc_powerDevice->GetPowerState( THRC_PowerDeviceID_ENG_IRC_MORTION_ENABLE ) ? 1 : 0) << 23) & 0x0000000000800000)		// 現在の電源ステート取得.
					  | (((uint64_t)(THRC_PowerStateID_NORMAL == pc_powerDevice->GetPowerState( THRC_PowerDeviceID_Fiery ) ? 1 : 0)                  << 22) & 0x0000000000400000)		// 現在の電源ステート取得.
					  | (((uint64_t)(THRC_PowerStateID_NORMAL == pc_powerDevice->GetPowerState( THRC_PowerDeviceID_Fiery_PT_R ) ? 1 : 0)             << 21) & 0x0000000000200000)		// 現在の電源ステート取得.
					  | (((uint64_t)(THRC_PowerStateID_NORMAL == pc_powerDevice->GetPowerState( THRC_PowerDeviceID_Panel ) ? 1 : 0)                  << 20) & 0x0000000000100000)		// 現在の電源ステート取得.
					  | (((uint64_t)(THRC_PowerStateID_NORMAL == pc_powerDevice->GetPowerState( THRC_PowerDeviceID_LCDC_LVDS ) ? 1 : 0)              << 19) & 0x0000000000080000)		// 現在の電源ステート取得.
					  | (((uint64_t)(THRC_PowerStateID_NORMAL == pc_powerDevice->GetPowerState( THRC_PowerDeviceID_PANEL_POWER ) ? 1 : 0)            << 18) & 0x0000000000040000)		// 現在の電源ステート取得.
					  | (((uint64_t)(THRC_PowerStateID_NORMAL == pc_powerDevice->GetPowerState( THRC_PowerDeviceID_ENG ) ? 1 : 0)                    << 17) & 0x0000000000020000)		// 現在の電源ステート取得.
					  | (((uint64_t)(THRC_PowerStateID_NORMAL == pc_powerDevice->GetPowerState( THRC_PowerDeviceID_ENG_POWER ) ? 1 : 0)              << 16) & 0x0000000000010000)		// 現在の電源ステート取得.
					  | (((uint64_t)(THRC_PowerStateID_NORMAL == pc_powerDevice->GetPowerState( THRC_PowerDeviceID_ENG_MORTION_ENABLE ) ? 1 : 0)     << 15) & 0x0000000000008000)		// 現在の電源ステート取得.
					  | (((uint64_t)(THRC_PowerStateID_NORMAL == pc_powerDevice->GetPowerState( THRC_PowerDeviceID_PCIe_BUS ) ? 1 : 0)               << 14) & 0x0000000000004000)		// 現在の電源ステート取得.
					  | (((uint64_t)(THRC_PowerStateID_NORMAL == pc_powerDevice->GetPowerState( THRC_PowerDeviceID_PEXPD ) ? 1 : 0)                  << 13) & 0x0000000000002000)		// 現在の電源ステート取得.
					  | (((uint64_t)(THRC_PowerStateID_NORMAL == pc_powerDevice->GetPowerState( THRC_PowerDeviceID_WAKE_FAXMFPC ) ? 1 : 0)           << 12) & 0x0000000000001000)		// 現在の電源ステート取得.
					  | (((uint64_t)(THRC_PowerStateID_NORMAL == pc_powerDevice->GetPowerState( THRC_PowerDeviceID_MC_PWR_EN ) ? 1 : 0)              << 11) & 0x0000000000000800);		// 現在の電源ステート取得.
	printf( "(%010u)(BSP) ## %s us_DetailCode=%#x, ull_NewDetailCode=%#llx\n", SYSF_tickGet() , __FUNCTION__, us_DetailCode, ull_NewDetailCode);
	bl_ret = APIC_setNewDetailCode( us_DetailCode, ull_NewDetailCode );		// 詳細コード登録.
	if (False == bl_ret) {
		SYSF_logMsg( "## %s %d : APIC_setNewDetailCode NG\n", __FUNCTION__, __LINE__, 3,4,5,6 );
	}
#endif	// #if defined(TORNADO) && !defined(FUM_LINUX).

	ca72wdt_ext_refresh( 0x4200, 120, 0x0000, "wdExpire.\n" );
	
#if defined(TORNADO) && !defined(FUM_LINUX) /*実機*/
	// 2012.10.15 内野 CQ373869 wdPowerManagerDisable()にてセットしたNVRAMを見て、isWdDisableにNVRAMの値を入れる.
	isWdDisable = guc_isWdDisable;
#else /*sim*/
	isWdDisable = 1;
#endif

	StartTmB_SMP();
	SYSF_taskDelay(SYSF_sysClkRateGet() * 50 / 1000);	// 30ms wait.
	StopTm();

	// 2015/09/11 Sky_Araki CQ518543 FAX受信後、本体フリーズ.	
	Save_PsInfo();
	
	// 2017/04/20 山口 AR.1017893 SDRAMの復帰待ち.
	THRG_DspChoiceLog(THRD_ChoiceDspLogKind_SDRAMClear);
	
	// 2015/10/21 Harada 新詳細コード対応 <<.
	// 2016/02/22 堀井 CQ.524679 新詳細コード無効化.
	// 2016/06/14 PS1-ft_kobayashi CQ.529697 【詳細コード対応】200A.

	if( isWdDisable == 1 || pm_delay || pmLogOn ) {	// wdが無効化されている？.
		THRC_PM_MSG( "wd disabled.\n" );
		return;
	}
	
	// ログ出力.
	printf( "(%010u)(BSP) wdExpire log start ---------------------------\n", SYSF_tickGet()  );
	pmLogPrint();
	pmDevicesState();
	//++ 2019/04/24 Sky		【EagleH】Warning除去対応.
	si_Result = system( "dmesg" );
	if (si_Result != 0){
			printf("(%010u)(BSP) ## %s[%d] %s\n", SYSF_tickGet(), __FILE__, __LINE__, __FUNCTION__);
	}
	si_Result = system(CMD_PS_TO_CONSOLE);		// 2015/09/11 Sky_Araki CQ518543 FAX受信後、本体フリーズ.
	if (si_Result != 0){
			printf("(%010u)(BSP) ## %s[%d] %s\n", SYSF_tickGet(), __FILE__, __LINE__, __FUNCTION__);
	}
	// 2020/12/21 ft_kondo OP_BTS-28906 PowerManagerのバックトレース出力処理追加. 
	printf("(%010u)(BSP) taskTrace TASK_PowerManager\n", SYSF_tickGet());
	tt("TASK_PowerManager");

	// 2019/07/10 OP_BTS-3440 山口 wdExpire時にタスクトレース出力.
	printf("(%010u)(BSP) taskTrace TASK_PM_PCIe_PCIDev\n", SYSF_tickGet());
	tt("TASK_PM_PCIe_PCIDev");
	printf("(%010u)(BSP) taskTrace TASK_PM_Fiery\n", SYSF_tickGet());
	tt("TASK_PM_Fiery");
	printf("(%010u)(BSP) taskTrace TASK_PM_USB\n", SYSF_tickGet());
	tt("TASK_PM_USB");
	printf("(%010u)(BSP) taskTrace TASK_PM_HDD\n", SYSF_tickGet());
	tt("TASK_PM_HDD");
	printf("(%010u)(BSP) taskTrace TASK_PM_IR\n", SYSF_tickGet());
	tt("TASK_PM_IR");
	
	//-- 2019/04/24 Sky		【EagleH】Warning除去対応.
	printf( "(%010u)(BSP) wdExpire log end ---------------------------\n", SYSF_tickGet()  );
	
	// リブート.
	APIC_SystemServiceFunction* p_SystemServiceFunction = APIC_SystemServiceFunction::newInstance();
	if (p_SystemServiceFunction){
		{ // 制御構成変更_No.40 リブート回数を数えるカウンタを追加する対応.
			APIC_OperateCnt *pAPIC_OperateCnt = APIC_OperateCnt::newInstance();
			if( NULL!=pAPIC_OperateCnt )
			{
				pAPIC_OperateCnt->inc(TYPE_OperateCnt_Reboot);
			}
		}
		p_SystemServiceFunction->reboot();
	}
}

// ================================================================================================.
//! <!--機能--> 電源管理 制御部 デバッグログをeMMCに保存する.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
//  2017/04/19 山口 1016622 BSPのデバッグログをeMMCに保存するラッパー関数.
// ================================================================================================.
void THRC_PowerManager::PowerSave_DebugLog( void ){
	
	StartTmB_SMP();
	SYSF_taskDelay(SYSF_sysClkRateGet() * 50 / 1000);	// 50ms wait.
	StopTm();
	
	Save_PsInfo();
	
}


// ================================================================================================.
//! <!--機能--> 電源管理 制御部 パワーオンスタート その１.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
//  09/08/07 越野	Donau環境から分離して作成.
//  10/07/14 内野	省電力復帰要因の通知対応.
// 13/02/28 Sky 機種共通化対応 THRC_PowerManagerSoC.cppから移植.
// 14/04/10 toda Helios対応 Heliosで制御対象としない処理を削除.
// ================================================================================================.
void THRC_PowerManager::PowerOnStart1()
{
//++ 2019/04/24 Sky		【EagleH】Warning除去対応.
// test code.
// 2013/07/02 KM(Morikawa) SoC LinuxSim JFM Merge.
#if defined(TORNADO) && !defined(FUM_LINUX)	// [新SIM対応]MinervaZeusInt立ち上げ.	
	THRC_PM_LOG( "PowerOnStart1()" );
//-- 2019/04/24 Sky		【EagleH】Warning除去対応.

	static THRC_PowerDeviceID e_devices[] = {
		THRC_PowerDeviceID_FileSystem,	THRC_PowerDeviceID_HDD_DRIVER,	THRC_PowerDeviceID_PCIe_BUS,
		/*THRC_PowerDeviceID_PIC,*/			THRC_PowerDeviceID_USB,			THRC_PowerDeviceID_IR,
		/*THRC_PowerDeviceID_CPS,*/			/*THRC_PowerDeviceID_Fiery,*/	THRC_PowerDeviceID_Panel,
		/*THRC_PowerDeviceID_USB_DOORSENSOR_EN,*/ THRC_PowerDeviceID_IMGSYS,
		THRC_PowerDeviceID(-1),
	};

	// 管理下デバイスを通常状態に設定.
	ushort i = 0;
	while( e_devices[i] != THRC_PowerDeviceID(-1) ) {
		pc_powerDevice->SwitchPowerState( e_devices[i], THRC_PowerStateID_NORMAL );
		i++;
	}

	// 2014/07/23 ft_kondo Poseidon 機器構成変更No.59 ファン制御変更. 
	// Poseidonは、アプリ起動中は全速に変更(電源ステータスが待機中となったら、電源部が半速に変更する). 
	// 2014/10/24 FT_yamaguchi Heliosコンパイル対応 HeliosLはPoseidonと同様なので、Poseidon側に倒すために、HeliosLのコンパイルSWを追加.
#if ( defined(DEF_Zeus5BkCus) || defined(DEF_HeliosCus) ) && !defined(DEF_S800Emu) && !defined(DEF_EagleCus) && !defined(DEF_SparrowCus)
	pc_powerDevice->SwitchPowerState( THRC_PowerDeviceID_BOX_FAN, THRC_PowerStateID_NORMAL );
#else 
	// ZeusSやEmu3は、MLKの仕様のまま. 
	// 2008/04/30 河村 CQ.302645 - 起動時の機内ファン制御を全速から半速に変更.
	pc_powerDevice->SwitchPowerState( THRC_PowerDeviceID_BOX_FAN, THRC_PowerStateID_LOW0 );
#endif // #if ( defined(DEF_Zeus5BkCus) || defined(DEF_HeliosCus) ) && !defined(DEF_S800Emu)

	// 2008/03/26 河村 CQ.301465 - ENG_UARTをENG_POWER(ENGリレー状態)と同じ値に設定する. //戸田 元に戻す.
	pc_powerDevice->SwitchPowerState( THRC_PowerDeviceID_ENG_UART, pc_powerDevice->GetPowerState( THRC_PowerDeviceID_ENG_POWER ) );

	THRC_PM_LOG( "PowerOnStart1() complete" );
#endif	// #if defined(TORNADO) && !defined(FUM_LINUX)	// [新SIM対応]MinervaZeusInt立ち上げ.	
	
	/*********** 2010/07/14 内野 省電力復帰要因の通知対応. start ***********/
	

	
	//電源オブザーバ登録.
    APIC_Power::newInstance()->attachObserver( THRC_PowerObserver::newInstance() );
	// 2011/08/18 CQ:339394 河村 - 定期起床開始タイミングを変更.
	start_cyclicLoop_taskPowerManager();

	// パネル電源制御タスク生成 省電力中のブザー対応 堀井 2014/5/13.
	si_taskID_PM_Panel			= SYSF_taskSpawn( "TASK_PM_Panel",50, VX_FP_TASK,		10000, (FUNCPTR)THRC_PowerManager::messageLoop_Panel,		0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

}

// Str 2010/09/13 ft_kondo 8536環境Sim対応.
//++ 2019/04/24 Sky		【EagleH】Warning除去対応.
#if defined(TORNADO) && !defined(SIMULATOR)			// 実機で有効.
//-- 2019/04/24 Sky		【EagleH】Warning除去対応.
// ================================================================================================.
//! <!--機能-->シグナルハンドラ.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param  int			：未使用.
//! \param  siginfo_t *	：シグナル情報.
//! \param  void *		：未使用.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! シグナルに対するハンドラ\n.
//! <!--履歴-->.
// 2010/11/09 佐々木	SoC対応 シグナル値を修正.
// 2011/05/09 CQ:344079 河村 - シグナルセーフでない関数の呼び出しを削除.
// 2011/07/25 CQ:352236 山下 - CQ:344079のコメントアウトマージを解除.
// 13/02/28 Sky 機種共通化対応 THRC_PowerManagerSoC.cppから移植.
// ================================================================================================.
#define PMU_EVENT_STATE_NEW_DETAIL_CODE		(0xf<<12)	/* newDetailCode通知 */

static void sig_handler( int32_t sn , siginfo_t *si , void *sc  )		//siginfo_tの構造体はsignal.hにある.
{
	if( ( si->si_code & (0xf<<12) ) == PMU_EVENT_STATE_NEW_DETAIL_CODE ) {
		sendMessage( TID_PowerManeger, TM_NewDetailCode, 0 );	// 2022/12/12 河村 - OP_BTS-41054 新詳細コード記録通知.
	} else {
	pmuEvent.add( si->si_code );
}
}

#endif // #ifndef TORNADO.
// End 2010/09/13 ft_kondo 8536環境Sim対応.


// ================================================================================================.
//! <!--機能--> スレッド電源制御部 デバイス制御部 省電力復帰コールバック.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return true/成功　False/失敗.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
// 2010/07/20 内野 初版.
// 13/02/28 Sky 機種共通化対応 THRC_PowerManagerSoC.cppから移植.
// ================================================================================================.
Bool THRC_PowerManager::SetPowerCallBack( void (*pl_PowerCallBack)( ushort ) )		//power部が呼ぶ.
{
	p_PowerCallBack = pl_PowerCallBack;
	return true;
}

Bool bl_pm_batchTest = False;

#ifdef THRC_PM_POWER_SWITCH_LOGGING // 2015/10/09 CQ.519888 河村 - 未使用デバッグ機能の無効化.
void THRC_PowerManager::batchTest(void)
{
	static ushort us_stateBefore = 1;
	static ushort us_stateAfter = 1;
	static Bool bl_isFirstProcess = True;
	static uint32_t ul_testTimer = 0;

	if( bl_pm_batchTest == False ) {
		return;
	}

#ifndef TORNADO
	if( pm_fp == NULL ) {
		pm_fp = SYSF_fopen( "simwin_log.txt","w" );
	}
#else
	if( pm_fp == NULL ) {
		pm_fp = SYSF_fopen( "/bd0/simwin_log.txt","w" );
		if( pm_fp == NULL ) {
			logMsg( "@@ %"PRId32":file open error.\n", SYSF_tickGet(), 0,0,0,0,0 );
			// 2013/07/18 Harada MinervaZeus3rd 静的解析対応(ID:21662、216756). 
			return;
		}
	}
#endif

	ul_testTimer++;
	if( ul_testTimer < 100 ) {
		return;
	}
	ul_testTimer = 0;
#ifdef TORNADO	// 実機.
	logMsg( "@@ %"PRId32":batch test %d-%d\n", SYSF_tickGet(), us_stateBefore,us_stateAfter,0,0,0 );
#else			// SIM.
	TRACE ( "@@ %"PRId32":\n", SYSF_tickGet(), 0,0,0 );
#endif

	static ST_MsgPowerSaveStatusPara para = { THRE_PowerSaveStatusIdle, THRE_PowerSaveStatusIdle };

	extern void pmd_list(void);

	if( bl_isFirstProcess ) {
		// まず、遷移前のステートに移行。.
		pmd_list();
		THRC_PM_MSG( "@@ test 準備\n" );
		para.uc_ChangeStatusBefore = para.uc_ChangeStatusAfter;
		para.uc_ChangeStatusAfter  = (uchar)us_stateBefore;
		PowerChangeStart( para );
		bl_isFirstProcess = False;
	} else {
		pmd_list();
		THRC_PM_MSG( "@@ test %d to %d\n", us_stateBefore, us_stateAfter );
		para.uc_ChangeStatusBefore = para.uc_ChangeStatusAfter;
		para.uc_ChangeStatusAfter  = (uchar)us_stateAfter;
		PowerChangeStart( para );
		bl_isFirstProcess = True;
		us_stateAfter++;
		if( us_stateAfter > 11 ) {
			us_stateAfter = 1;
			us_stateBefore ++;
			if( us_stateBefore > 11 ) {
				bl_pm_batchTest = False;
                // HCL Warning Fix - Starts.
                // Warning No : 34315.
                if( pm_fp != NULL ){
				SYSF_fclose( pm_fp );
				pm_fp = NULL;
			}
                // HCL Warning Fix - Ends.
			}
		}
	}
}
#endif

// ================================================================================================.
//! <!--機能--> 電源管理 制御部 パワーオンスタート その２.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
//  09/08/07 越野	 Donau環境から分離して作成.
// 13/02/28 Sky 機種共通化対応 THRC_PowerManagerSoC.cppから移植.
// 14/04/10 toda Helios対応 Fieryの制御は現状未確定であるため、暫定でコメントアウト.
// ================================================================================================.
struct TX_taskPic_IFMsg
{
// 各自が自由に定義可能です。.
	BE_TaskMessage	e_type;		// メッセージ種別.
	int32_t			sl_data;	// メッセージデータ.
};

void THRC_PowerManager::PowerOnStart2()
{
	extern void InitPIC();

#ifdef	TORNADO		 
		logMsg( "PowerOnStart2 portSCANVD_EN H->L Before InitPIC() \n",0,0,0,0,0,0);
#endif	
	THRC_PM_LOG( "%"PRId32" : PowerOnStart2()", SYSF_tickGet() );

	if( bl_PowerOnStart == True  ) {
		THRC_PM_LOG( "InitPIC()" );
		InitPIC();
		TX_taskPic_IFMsg msg = { TM_PICEnableComNotice, 0 };		// メッセージ送信用構造体.
		SYSF_msgQSend( ENGC_PrinterImpColor::newInstance()->getMsgQID_FortaskPicIF(), (char*)&msg, sizeof(TX_taskPic_IFMsg), NO_WAIT, MSG_PRI_NORMAL );		// メッセージ送信.
		// 電源ON時の立ち上げ処理終了セット.
		bl_PowerOnStart = False;
	} else {
#ifdef	TORNADO		 
		logMsg( "PowerOnStart2 Sleep End \n",0,0,0,0,0,0);
#endif	
		THRC_PM_LOG( "Sleep End" );
	}

	pc_powerDevice->SwitchPowerState( THRC_PowerDeviceID_Fiery, THRC_PowerStateID_NORMAL );
}

// ================================================================================================.
//! <!--機能--> 電源管理 制御部 電源ステータス変更開始.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param ST_MsgPowerSaveStatusPara st_MsgPowerSaveStatusPara.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
//  09/08/07 越野	 Donau環境から分離して作成.
//  2011/04/29 浦崎 CQ.342154対応。ENG限定解除スリープからの復帰時に、復帰要因割込みが有効のままになっていたため修正.
//  2011/06/21 近藤 CQ.346317 副電源OFF/ONで特別なモードで起動する問題対策.
//  2011/10/07 佐々木	ZeusSoCEMU対応 USB_HOSTの電源制御対応.
//  2011/10/05 浦崎 CQ.355237対応。近接センサー有効無効タイミングも合わせて修正.
//  2011/11/25 河村 CQ.358450 上位から副電源OFF指定の場合に統合管理装置あり/なしで副電源OFF/副電源OFF1.5を切り替える.
//  2012/11/26 河村 CQ.375284 kernelからのリブート要求で省電力復帰時にリブートする対応.
// 13/02/28 Sky 機種共通化対応 THRC_PowerManagerSoC.cppから移植.
// ================================================================================================.
#if defined(TORNADO) && !defined(FUM_LINUX) /*実機*/
#define PMU_REQUEST_REBOOT_CODE_PATH	"/sys/bus/platform/devices/PMU.0/request_reboot"
#define min(a, b) ((a) < (b) ? (a) : (b))
int32_t pmRequestRebootDisable = 0;
uint32_t getRequestRebootCode(void)
{
	FILE *fp;
	char buf[11];	/*uint32_t最大桁数(10)+1*/
	size_t size = 0;
	
	fp = SYSF_fopen( PMU_REQUEST_REBOOT_CODE_PATH, "r" );
	if( fp == NULL ) { // request reboot not support?
		return 0;
	}
	size = fread( buf, 1, 10, fp );
	SYSF_fclose( fp );
	size = min( size, 10 );
	buf[size] = '\0';
	return strtoul( buf, NULL, 10 );
}
#endif	// #if defined(TORNADO) && !defined(FUM_LINUX).

// 2015/mm/dd Sky_Araki CQ518419 副電源オフ時にスリープしない不具合を修正.
static Bool bl_SubPowOffMode= False;
static Bool bl_ErpMode= False;
void THRC_PowerManager::PowerChangeStart( ST_MsgPowerSaveStatusPara st_MsgPowerSaveStatusPara )
{
	const char *psc_stringFromStatus[] = {
		"0: 起動中",
		"1: 待機中",
		"2: ローパワー : 画像受信可",
		"3: ローパワー : 画像受信不",
		"4: スリープ : 画像受信可 / メインリレーON",
		"5: スリープ : 画像受信不 / メインリレーON",
		"6: 副電源オフ : 画像受信可",
		"7: 副電源オフ : 画像受信不",
		"8: ローパワー : 画像受信可 / プリント可",
		"9: スリープ : 画像受信可 / プリント可",
		"10: ローパワー : 画像受信可 / プリント可 / パネルON",
		"11: スリープ : 画像受信可 / プリント可 / パネルON",
		"12: スリープ : 画像受信可 / メインリレーOFF",
		"13: スリープ : 画像受信不 / メインリレーOFF",
		"14: スリープ1.5",
		"15: 副電源オフ : CPS On",
		"16: 副電源オフ1.5",
	};
	if( THRE_PowerSaveStatusNone != uc_ChangeStatusBefore || THRE_PowerSaveStatusNone != uc_ChangeStatusAfter ) {
		THRC_PM_ASSERT("PowerChangeStart() : Sequence Error! uc_ChangeStatusBefore[%d] uc_ChangeStatusAfter = [%d]", uc_ChangeStatusBefore,uc_ChangeStatusAfter,0,0,CFA14_DetailCode0x08E5 );
	}

	// 2014/05/01 ft_kondo 省電力復帰時間短縮対応. 
	// まだHDDの復帰シーケンス途中だったら、完了するまでシーケンス開始を遅らせる. 
	// 2014/07/25 CQ.501828 ft_kondo HDDの接続完了まで、次の省電力シーケンスは動かさない対応. 
	// 2014/09/26 ft_kondo スリープ移行出来無くなる問題対応. 
	static ST_MsgPowerSaveStatusPara st_localMsgPowerSaveStatusPara;
	if( True == b_doPowerChangeSequence_Hdd ){
		st_localMsgPowerSaveStatusPara.uc_ChangeStatusBefore = st_MsgPowerSaveStatusPara.uc_ChangeStatusBefore;
		st_localMsgPowerSaveStatusPara.uc_ChangeStatusAfter  = st_MsgPowerSaveStatusPara.uc_ChangeStatusAfter;
		st_localMsgPowerSaveStatusPara.uc_ExcludeUSBHost = st_MsgPowerSaveStatusPara.uc_ExcludeUSBHost;
		SYSF_taskDelay((int32_t)(SYSF_sysClkRateGet() * 2000 / 1000 ));		// 2000msec遅延.
		THRC_PM_LOG( "%s HDD WakeUp Waiting...", __FUNCTION__ );
		// 自タスクに再度要求を投げて、待たされている他の処理を行う. 
		sendMessage( TID_PowerManeger, TM_ReqStartChangePowerSaveStatus, (uintptr_t)&st_localMsgPowerSaveStatusPara ); // TASK_PowerManagerにメッセージを投げる. 
		// 2017/02/20 Sky酒井（純）ユーザーイベント記録(APIコール).
		SYSF_ReqTaskMonitorUserEvent(18000, 29, 0, 0);
		
		return;
	}

//Str【VenusMLK CQ.529803 スリープ中動作が失敗する問題】2016.05.23.
	//節電状態復帰時のCセットモード状態を電源から受け取る.
	uc_CsetModeExec = st_MsgPowerSaveStatusPara.uc_CsetMode;
//End【VenusMLK CQ.529803 スリープ中動作が失敗する問題】2016.05.23.
	
	b_rec_HDD = False;

	//CT用ログ.
	THRC_PM_LOG( "PowerChangeStart : ");
	// 2017/02/20 Sky酒井（純）ユーザーイベント記録(APIコール).
	SYSF_ReqTaskMonitorUserEvent(18000, 18, 0, 0);

	// 2011/11/25 CQ:358450 河村 - 統合管理装置あり/なしで副電源OFFのモードを切り替える.
	// 2016/10/31 sky_ishiga [MinervaMLK][起動時間短縮] 主電源OFFモード時は、副電源オフ1.5で動作させる.
	if(( APIC_KeyDevice::getKeyMode() != TYPE_KeyModeNone )
	&& ( 0 == st_MsgPowerSaveStatusPara.uc_MainPowerOff )) {
		// サービス設定「管理装置装着」か「統合ベンダー」か「キーカウンタ」か「PagaAccces」が設定ありの場合.
		// かつ、主電源OFFモードでない場合.
		// 特に何もしない.
	} else {
		// サービス設定「管理装置装着」か「統合ベンダー」か「キーカウンタ」か「PagaAccces」が設定なしの場合.
		// または、主電源OFFモードの場合
		// 副電源OFFは副電源OFF1.5として扱う.
		if( st_MsgPowerSaveStatusPara.uc_ChangeStatusBefore == THRE_PowerSaveStatusSubPowerOff ) {
			st_MsgPowerSaveStatusPara.uc_ChangeStatusBefore = THRE_PowerSaveStatusSubPowerOff15;
			THRC_PM_LOG( "from : No integrated management device attached THRE_PowerSaveStatusSubPowerOff15" );
		}
		if( st_MsgPowerSaveStatusPara.uc_ChangeStatusAfter == THRE_PowerSaveStatusSubPowerOff ) {
			st_MsgPowerSaveStatusPara.uc_ChangeStatusAfter = THRE_PowerSaveStatusSubPowerOff15;
			THRC_PM_LOG( "to : No integrated management device attached THRE_PowerSaveStatusSubPowerOff15 " );
		}
	}
	
	THRC_PM_LOG( "from : %s", psc_stringFromStatus[st_MsgPowerSaveStatusPara.uc_ChangeStatusBefore] );
	THRC_PM_LOG( "to   : %s", psc_stringFromStatus[st_MsgPowerSaveStatusPara.uc_ChangeStatusAfter] );
	THRC_PM_LOG( "ExcludeUSBHost : %d, uc_CsetMode : %d", st_MsgPowerSaveStatusPara.uc_ExcludeUSBHost, st_MsgPowerSaveStatusPara.uc_CsetMode );

	uc_ChangeStatusBefore = st_MsgPowerSaveStatusPara.uc_ChangeStatusBefore;
	uc_ChangeStatusAfter  = st_MsgPowerSaveStatusPara.uc_ChangeStatusAfter;
	uc_ExcludeUSBHost = st_MsgPowerSaveStatusPara.uc_ExcludeUSBHost;

	THRC_PM_LOG( "%"PRId32" : THRC_PowerManager::PowerChangeStart() befor:%d after:%d ", SYSF_tickGet(), uc_ChangeStatusBefore, uc_ChangeStatusAfter );

	// 2015/mm/dd Sky_Araki CQ518419 副電源オフ時にスリープしない不具合を修正.
	switch(uc_ChangeStatusAfter) {
		case THRE_PowerSaveStatusSubPowerOffDelay:			//!< 副電源を伴うSleepモードでアクセス可能.
		case THRE_PowerSaveStatusSubPowerOff:				//!< 副電源を伴うSleepモード.
		case THRE_PowerSaveStatusSubPowerOffDelayCpsOn:		//!< 副電源OFF遅延 CPS動作可能.
		case THRE_PowerSaveStatusSubPowerOff15:				//!< 副電源OFF1.5.
			bl_SubPowOffMode= True;
			break;
		default:
			bl_SubPowOffMode= False;
			break;
	}
	THRC_PM_LOG("%s SubPowerOffMode=%d", __FUNCTION__, bl_SubPowOffMode );

	// 2011/06/01 CQ:346018 河村 - 起動/省電力デバッグシステム対応.
	{	/*領域のクリア*/
		char clr[20];
		memset( clr, 0, sizeof(clr));
		initDebugWrite( initDebugModule_THR, 0, (void*)clr, sizeof(clr) );
	}
	char c=1; initDebugWrite( initDebugModule_THR, 3, &c, 1);	/*PWRから開始要求を受けた*/
	initDebugWrite( initDebugModule_THR, 4, &uc_ChangeStatusBefore, 1 );
	initDebugWrite( initDebugModule_THR, 5, &uc_ChangeStatusAfter, 1 );
	/* 2014/04/22 Harada [Zeus5BK]NV-RAMレス 起動/省電力シーケンスデバッグ対応 */
// 2022/12/06 OP_BTS-41198 山口 SPI-FLASH書き込み低減対応.
// 下記処理はinitDebugDataDumpを無効化しているため現状必要ない.
#if 0
	unsigned char uc_data = THRD_DETAIL_CODE_UNREGISTERED;	/* FA14詳細コード未登録 */
	/* FA14詳細コード登録済みフラグ取得 */
	if ( THRD_GET_SPI_OK != THRG_getBspThrRegion( THRD_OFFSET_DETAIL_CODE_REGISTERED, (void*)&uc_data, sizeof(uc_data) ) ) {
		/* エラー */
		SYSF_logMsg( "## %s %d : THRG_getBspThrRegion ERR\n", __FUNCTION__, __LINE__, 3,4,5,6 );
	}
	if (uc_data == THRD_DETAIL_CODE_REGISTERED) {	/* FA14詳細コード登録済み */
		/* FA14詳細コード登録済みフラグクリア */
		uc_data = THRD_DETAIL_CODE_UNREGISTERED;	/* FA14詳細コード未登録 */
		if ( THRD_SET_SPI_OK != THRG_setBspThrRegion( THRD_OFFSET_DETAIL_CODE_REGISTERED, (void*)&uc_data, sizeof(uc_data) ) ) {
			/* エラー */
			SYSF_logMsg( "## %s %d : THRG_setBspThrRegion ERR\n", __FUNCTION__, __LINE__, 3,4,5,6 );
		}
	}
#endif
	PM_SEQ_REC_START();
	PM_SEQ_REC( uc_ChangeStatusBefore );	/* From */
	PM_SEQ_REC( uc_ChangeStatusAfter );		/* To */

	bl_DirActive = False;
	for( uchar j=0; j< DEF_MAX_DeviceNum; ++j ) {
		if( uc_MFPBoard_PowerSaveTableStruct[uc_ChangeStatusBefore][j] != uc_MFPBoard_PowerSaveTableStruct[uc_ChangeStatusAfter][j] ) {
			if( uc_MFPBoard_PowerSaveTableStruct[uc_ChangeStatusAfter][j] == DEF_PowerSave_Active ) {
				bl_DirActive = True;
				break;
			}
		}
	}

#if defined(TORNADO) && !defined(FUM_LINUX) /*実機*/
	if( True == bl_DirActive ) {
		Bool isReboot = False;
		uint32_t requestRebootCode = getRequestRebootCode();
		switch( requestRebootCode ) {
		case 0:
			break;
		case 1:
			DebugForeverFA14( CFA14_DebugFatal, CFA14_DetailCode0x0B2B );	//cic_free_func:Oops detected.
			isReboot = True;
			break;
		default:
			DebugForeverFA14( CFA14_DebugFatal, CFA14_DetailCode0x0B2C );	//未定義コード.
			isReboot = True;
			break;
		}
		
		if( True == isReboot ) {
			if( pmRequestRebootDisable == 0 ) {
				printf( "(%010u)(BSP) Reboot request from Kernel. code:%"PRId32"\n", SYSF_tickGet() , requestRebootCode );
				APIC_SystemServiceFunction*	pc_SystemServiceFunction = APIC_SystemServiceFunction::newInstance();
				if( pc_SystemServiceFunction ) {
					pc_SystemServiceFunction->reboot(); // リブートさせる.
				}
			} else {
				printf( "(%010u)(BSP) Reboot request from Kernel.(reboot Ban) code:%"PRId32"\n", SYSF_tickGet() , requestRebootCode );
			}
		}
	}
#endif	// #if defined(TORNADO) && !defined(FUM_LINUX).
	
	isEngOnlyWakeup = False;
	isSleepDelay = False;
	if( True == bl_DirActive ) {
		// 2014/02/19 ft_kondo Zeus5BK 省電力復帰処理をパラレルで動かす対応. 
		// 各デバイスの復帰処理を制御するタスクを生成. 
		si_taskID_PM_PCIe_PCIDev	= SYSF_taskSpawn( "TASK_PM_PCIe_PCIDev",50, VX_FP_TASK,	10000, (FUNCPTR)THRC_PowerManager::messageLoop_PCIePCIDev,	0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
		si_taskID_PM_Fiery			= SYSF_taskSpawn( "TASK_PM_Fiery",50, VX_FP_TASK,		10000, (FUNCPTR)THRC_PowerManager::messageLoop_Fiery,		0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
		si_taskID_PM_USB			= SYSF_taskSpawn( "TASK_PM_USB",50, VX_FP_TASK,			10000, (FUNCPTR)THRC_PowerManager::messageLoop_USB,			0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
		si_taskID_PM_HDD			= SYSF_taskSpawn( "TASK_PM_HDD",50, VX_FP_TASK,			10000, (FUNCPTR)THRC_PowerManager::messageLoop_HDD,			0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
		si_taskID_PM_IR				= SYSF_taskSpawn( "TASK_PM_IR",50, VX_FP_TASK,			10000, (FUNCPTR)THRC_PowerManager::messageLoop_IR,			0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
// 2019/04/03 AR.1075382 山口 TASK_PM_Watch_HDDはIT6では不要なのでタスク生成しない.
#if 0
		si_taskID_PM_Watch_HDD		= SYSF_taskSpawn( "TASK_PM_Watch_HDD", 50, VX_FP_TASK, 	10000, (FUNCPTR)THRC_PowerManager::messageLoop_Watch_HDD, 	0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
#endif
		// 2016/02/10 Sky_OGAWA CQ.525104 副電源遅延後、Fiery初期化不良.
		// 2019/06/28 Sky [EagleH][OP_BTS-7775] スリープ移行復帰後Fieryアイコンがアクティブにならない.
        // CQ.525104も移動させた.
		if( uc_ChangeStatusAfter == THRE_PowerSaveStatusSubPowerOffDelay ) {
			g_SubPowOff15ToSubPowOffDelay= True;
			THRC_PM_LOG("### g_SubPowOff15ToSubPowOffDelay=%d ###\n",g_SubPowOff15ToSubPowOffDelay);	
		}

		// 2015/05/21 ft_kondo ASICの電源を入れるとFaxの割り込みが入るため、復帰前に割り込みは無効にしておく. 
		THRC_PowerManagerDeviceSingletonIf::newInstance().SwitchPowerState( THRC_PowerDeviceID_SET_KR1,	  THRC_PowerStateID_NORMAL );// FAXからの起動.

		switch( uc_ChangeStatusAfter ) {
		case THRE_PowerSaveStatusPrtEngOnlyLowPowerCancel:		// エンジン限定解除系.
		case THRE_PowerSaveStatusPrtEngOnlySleepCancel:
		case THRE_PowerSaveStatusPrtEngOnlyLowPowerCancelPanelOn:
		case THRE_PowerSaveStatusPrtEngOnlySleepCancelPanelOn:
			isEngOnlyWakeup=True;
			// 2015/11/09 CQ.520955 河村 - スキャン時定着OFF制御によりMC_MOTION_ENは禁止状態の可能性があるので解除する.
			pc_powerDevice->SwitchPowerState( THRC_PowerDeviceID_ENG_MORTION_ENABLE_RESUME, THRC_PowerStateID_NORMAL );
			break;
			
		case THRE_PowerSaveStatusSleepDelay:					// スリープ遅延系.
		case THRE_PowerSaveStatusSubPowerOffDelay:
			isSleepDelay = True;
			break;

		case THRE_PowerSaveStatusIdle:							// アイドル.
			// 2015/11/09 CQ.520955 河村 - スリープ遅延によりMC_MOTION_ENは禁止状態の可能性があるので解除する.
			// スキャン時定着OFFの場合には解除してはいけない.
			if( isMotionEnableOn() ) {
				pc_powerDevice->SwitchPowerState( THRC_PowerDeviceID_ENG_MORTION_ENABLE_RESUME, THRC_PowerStateID_NORMAL );
			}
			break;
		}
		// 省電力解除方向.
		PowerChangeSequence( THRE_PowerSequenceID_LowToNormal, 0, 0 );
		// 2014/02/21 ft_kondo 省電力制御をパラレルで動かす対応. 
//		e_PM_DeviceStatus = 0x1F; // デバイス復帰完了待ちステータスを設定. 
		e_PM_DeviceStatus = 0x5F; // デバイス復帰完了待ちステータスを設定. 	// 2015/01/12 Harada CQ.506130、CQ.508744、CQ.508749 SDRAMクリア完了も待つ対象とする.

		// 2014/10/16 ft_kondo 省電力シーケンスデバッグ対応. 
		// 2015/01/16 Harada CQ.509010 省電力制御パラレル対応に合わせた省電力シーケンス記録フラグ対応.
		// 省電力シーケンス記録フラグ.
		e_PM_SeqReqEndComplete = THRE_PM_SeqReqEndComplete_AllDevExceptHDD;		// 全デバイス(HDD以外).
		APIC_HddInfo* pc_HddInfo = APIC_HddInfo::newInstance();
		if (NULL != pc_HddInfo) {
			if (True == pc_HddInfo->getHddExist()) {	// HDD装着.
				e_PM_SeqReqEndComplete |= THRE_PM_SeqReqEndComplete_HDD;		// HDD.
			}
		}
		Bool b_TemporaryRescueUnitIR = False;	// IR暫定救済.
		APIC_SystemStatus* pc_SystemStatus = APIC_SystemStatus::newInstance();
		if (NULL != pc_SystemStatus) {
			if ( (TYPE_TemporaryRescueStatusSet == pc_SystemStatus->getTemporaryRescueServiceOn(TYPE_TemporaryRescueUnitIR))	// IR暫定救済.
			  || (TYPE_TemporaryRescueStatusSet == pc_SystemStatus->getTemporaryRescueUserOn(TYPE_TemporaryRescueUnitIR)) ) {	// IR暫定救済.
				b_TemporaryRescueUnitIR = True;
			}
		}
		if ( (1 == uc_ChangeStatusAfter)				// 1: 待機中へ遷移.
		  && (False == b_TemporaryRescueUnitIR) ) {		// IR暫定救済でない.
			e_PM_SeqReqEndComplete |= THRE_PM_SeqReqEndComplete_IR;				// IR.
		}
	} else {
		// 省電力移行方向.
		// 2016/10/31 sky_ishiga [MinervaMLK][起動時間短縮] スレッド省電力制御の並列制御対応.
		if( 0 == st_MsgPowerSaveStatusPara.uc_MainPowerOff ) {
			/* 2019/01/31 AR1072285 山口 CA72 WDT対応. */
			// 省電力移行中の区間を対象とする.
			// SuperWarpは対象外とする.
			if (True == isWdOn_CA72) {	// CA72WDT有効の場合.
				if(uc_ChangeStatusBefore == THRE_PowerSaveStatusIdle){	// 遷移前電源ステータスが待機中.
#if 0	// 2020/04/28 河村 -  OP_BTS-22858 CA72WDTを省電力制御以外も動作させる対応.
					int32_t si_Fd = 0;
					int32_t si_Ret2 = 0;
					si_Fd = SYSF_open(THRD_DeviceFile_Pmu, O_RDWR, 0);
					if (0 > si_Fd) {
						printf( "(%010u)(BSP) [%s] open failed. file=%s\n", SYSF_tickGet() , __func__, THRD_DeviceFile_Pmu);
					}
					si_Ret2 = ioctl( si_Fd, THRD_PMU_IOCSSET_WDT_CA72, DEF_WDT_CA72_START );	// START.
					if( si_Ret2 < 0 ) {
						printf( "(%010u)(BSP) %s: pmu ioctl(THRD_PMU_IOCSSET_WDT_CA72) Error!!! \n", SYSF_tickGet() , __FUNCTION__ );
					}
					SYSF_close(si_Fd);
					isWdStart_CA72 = True;	// CA72WDT Startセット.
#endif
				ca72wdt_ext_refresh( 0x4100, 90, 0x0000, "Sleep sequence.\n" );
				}
			}
			PowerChangeSequence( THRE_PowerSequenceID_NormalToLow, 0, 0 );
		} else {
			PowerChangeSequence( THRE_PowerSequenceID_NormalToPowerOff, 0, 0 );
		}
		APIC_SystemInfo::newInstance()->setExtraModeInPowerSave( True );
	}
}

// ================================================================================================.
//! <!--機能--> 電源管理 リブート前処理(リブート用の省電力復帰).
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! なし\n.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
//  09/08/07 越野	Donau環境から分離して作成.
//  10/08/27 河村	GaiaPF8536対応。画処理系のリセットで同様のことが出来るので、それに修正.
// 13/02/28 Sky 機種共通化対応 THRC_PowerManagerSoC.cppから移植.
// ================================================================================================.
void THRC_PowerManager::RunRebootSequence()
{
	/*GaiaPF8536 2010/10/26 河村 - 画処理系リセットをKernelで行うのでここは削除 */
}

// ================================================================================================.
//! <!--機能--> 電源管理 制御部 テーブルから電源ステート取得.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param ushort us_deviceID : 電源ステートを取得したいデバイス.
//! \param uchar uc_status : 電源ステータス.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
//  09/08/07 越野	 Donau環境から分離して作成.
// 13/02/28 Sky 機種共通化対応 THRC_PowerManagerSoC.cppから移植.
// ================================================================================================.
THRC_PowerStateID THRC_PowerManager::PowerStateID( ushort us_deviceID, uchar uc_status )
{
	ushort i = 0;

	// HCL Warning Fix - Starts.
    // Warning No : 34432.
    THRL_ThAssert( (uc_status < DEF_MAX_PowerSaveStatusNum ), "PowerStateID() uc_MFPBoard_PowerSaveTableStruct index error", 0 , 0, 0, 0, CFA14_DetailCode0x0CCF );
    // HCL Warning Fix - Ends.

	while( s_groupTable[i].e_newGroup != THRC_PowerDeviceID(-1) ) {
		if( us_deviceID == s_groupTable[i].e_newGroup ) {
			if( uc_MFPBoard_PowerSaveTableStruct[uc_status][s_groupTable[i].e_oldGroup] == DEF_PowerSave_Active ) {
				return THRC_PowerStateID_NORMAL;
			} else {
				return THRC_PowerStateID_LOW;
			}
		}
		i++;
	}
	return THRC_PowerStateID_UNKNOWN;	// 指定デバイスが存在しなかった.
}

// ================================================================================================.
//! <!--機能--> 電源管理 制御部 FileSystemの省電力移行カウント可能.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return Bool True:カウント可能, False:カウント不可.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
//  09/08/07 越野	 Donau環境から分離して作成.
// 13/02/28 Sky 機種共通化対応 THRC_PowerManagerSoC.cppから移植.
// ================================================================================================.
// HDDスリープ遅延処理用。DOCの初期化が完了しているか？等の条件でウエイトカウント可能状態であるかどうかをチェック.
static Bool canFileSystemCount( void )
{
	FILS_InitStat st_initStat;
	st_initStat = FILC_FsControlIf::getInitStatus() ;

	if( THRF_IsCpsGreatExist() == True && st_initStat.Doc != FILE_InitStatDocInitComplete ) {
		// CPSが存在、Docが初期化完了していない.
		return False;
	}
#ifdef TORNADO						
	if ( SYSF_taskIsSuspended(SYSF_taskNameToId((char*)"FsControl")) == True ) {
		return False;
	}
#endif	// #ifdef TORNADO.
	return True;
}

// ================================================================================================.
//! <!--機能--> スレッド電源制御部 シーケンス制御部 遅延制御付き電源ステート変更シーケンス要求受け付け.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param 　ushort e_sequenceID    : シーケンスID.
//! \param 　ushort us_sequenceState : シーケンスState.
//! \param 　uint32_t  ul_param         : パラメータ.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
//  09/08/07 越野	 Donau環境から分離して作成.
// 13/02/28 Sky 機種共通化対応 THRC_PowerManagerSoC.cppから移植.
// 17/02/13 Sky-Araki [起動時間短縮] 省電力時に主電源OFFするとフリーズ.
// 17/06/23 河村 AR.1024778/1024779 DelayedPowerChangeSequenceが動作しないことがある問題修正(HDD復帰シーケンス停止でC-D050が発生する).
// ================================================================================================.
static Bool s_DelayedPowChgSeq= False;
void THRC_PowerManager::DelayedPowerChangeSequence( uint32_t ul_delayTimeMS, THRE_PowerSequenceID e_sequenceID, ushort us_sequenceState, uint32_t ul_param )
{
	ushort us_num = 0;
	uint32_t ticks;

	THRC_PM_LOG( "%"PRId32" : DelayedPowerChangeSequence( %"PRId32", %d, %d, %"PRId32")", SYSF_tickGet(), ul_delayTimeMS, e_sequenceID, us_sequenceState, ul_param );

	//パラメータを格納可能な領域を探す.
	for( us_num=0; us_num<DELAYED_POWER_CHANGE_SEQUENCE_QUE_MAX; us_num++ ) {
		if( st_delayedParam[us_num].ul_actionTick == 0 ) {
			break;
		}
	}
	if( us_num == DELAYED_POWER_CHANGE_SEQUENCE_QUE_MAX ) {
		// 空きがない場合.
		THRC_PM_ASSERT("DelayedPowerChangeSequence() :: Sequence error! us_num = [%d]", us_num,0,0,0,CFA14_DetailCode0x08E6 ); // 2010/03/04 ft_kondo CQ324295.
	}
	//パラメータを保存.
	ticks = SYSF_tickGet();
	st_delayedParam[us_num].ul_actionTick = (uint32_t)SYSF_sysClkRateGet() * ul_delayTimeMS / 1000 + (uint32_t)ticks;
	// 2017/06/23 河村 AR.1024778/1024779 DelayedPowerChangeSequenceが動作しないことがある問題修正(HDD復帰シーケンス停止でC-D050が発生する).
	if( st_delayedParam[us_num].ul_actionTick == 0 ) { // 計算の結果0の場合には1にする.
		st_delayedParam[us_num].ul_actionTick = 1;
	}
	st_delayedParam[us_num].e_sequenceID    = e_sequenceID;
	st_delayedParam[us_num].us_sequenceState = us_sequenceState;
	st_delayedParam[us_num].ul_param         = ul_param;
	s_DelayedPowChgSeq= True;

	THRC_PM_LOG( "@@ now:%d timeout:%"PRId32"", ticks, st_delayedParam[us_num].ul_actionTick );
}

// ================================================================================================.
//! <!--機能--> スレッド電源制御部 シーケンス制御部 遅延電源ステート変更シーケンス実行.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
//  09/08/07 越野	 Donau環境から分離して作成.
// 13/02/28 Sky 機種共通化対応 THRC_PowerManagerSoC.cppから移植.
// ================================================================================================.
void THRC_PowerManager::DoDelayedPowerChangeSequence(void)
{
	ushort us_num = 0;
	static uint32_t ul_prevTick = 0;
	uint32_t ul_nowTick = SYSF_tickGet();

	//タイムアップした要求があったらそれを実行.
	for( us_num=0; us_num<DELAYED_POWER_CHANGE_SEQUENCE_QUE_MAX; us_num++ ) {
		if( st_delayedParam[us_num].ul_actionTick > 0 && time_after( ul_nowTick, st_delayedParam[us_num].ul_actionTick ) ) {
			THRC_PM_LOG( "Sequence execution by time up." );
			PowerChangeSequence( st_delayedParam[us_num].e_sequenceID, st_delayedParam[us_num].us_sequenceState, st_delayedParam[us_num].ul_param );
			st_delayedParam[us_num].ul_actionTick = 0;
			break;
		}
	}
	if( ul_prevTick > ul_nowTick ) {
		THRC_PM_LOG( "@@ tick Overflow detection prev:%"PRId32" , now:%"PRId32"", ul_prevTick, ul_nowTick );
	}
	ul_prevTick = ul_nowTick;
}

// ================================================================================================.
//! <!--機能--> スレッド電源制御部 シーケンス制御部 電源ステート変更シーケンス.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param 　ushort e_sequenceID    : シーケンスID.
//! \param 　ushort us_sequenceState : シーケンスState.
//! \param 　ulong  ul_param         : パラメータ.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
//  09/08/07 越野	Donau環境から分離して作成.
//  09/07/15 河村	CQ.319244 - ASIC200 SDRAMクリア制御の追加.
//  2010/09/24 伊藤史 CQ.333291 電源制御方向(uc_ChangeStatusAfter)がセマフォ排他の外にあるので、値を保全するためローカルへコピーする.
//  2011/04/12 Sky越野 CQ340449のSoC化対応.
//  2011/04/12 Sky越野 CQ331205のSoC化対応.
//  2011/03/25 河村 CQ340449 DeepSleep復帰NGリカバリ対応.
//  2011/04/29 浦崎 CQ.342154対応。ENG限定解除スリープからの復帰時に、復帰要因割込みが有効のままになっていたため修正.
//                  本関数ではなく、PowerChangeStart()にて待機中の指示がされたら無効にするようにした。
//  2011/06/28 kurachi MinervaZeus CQ:349562対策（CQ:348620対応を削除＋Panel通信再開の1s Delayを削除）.
//  2011/10/15 MinervaZeus_int リベース DeepSleep復帰NGリカバリ対応有効化.
//  2011/10/05 浦崎 CQ.355237対応。近接センサー有効無効タイミングも合わせて修正.
// 13/02/28 Sky 機種共通化対応 THRC_PowerManagerSoC.cppから移植.
//  2013/10/03 河村 CQ.382962 - 省電力中のFAX受信音対応.
// 14/04/10 toda Helios対応 Heliosで制御対象としない処理を削除.
// 14/10/28 toda CQ504323/504728/505515 HDDの接続完了まで、次の省電力シーケンスは動かさない対応.
// 17/10/10 Yoneda  S800 ストレージ構成変更対応.
// ================================================================================================.
// HDD確認.
// 15/09/18 Sky井柳 CQ519407 失敗時のエラー番号をアウトパラメタとして出力するように追加.
static Bool isLinkedHdd( int32_t *psi_Errno = NULL )
{
	struct stat buf;
	if( psi_Errno != NULL ){
		*psi_Errno = 0;		// エラー番号のリセット(失敗時のみ値が入るようにするため).
	}

// Str 2010/09/13 ft_kondo 8536環境Sim対応.
#ifdef TORNADO			// 実機で有効.
	if( stat( theHDDTrans.pc_strDeviceSSD, &buf ) == 0 && S_ISBLK( buf.st_mode ) ) {
#else 
	if( stat( theHDDTrans.pc_strDeviceSSD, &buf ) ) {
#endif // #ifndef TORNADO.
// End 2010/09/13 ft_kondo 8536環境Sim対応.
		return True;
	} else {
		if( psi_Errno != NULL ){
			*psi_Errno = errno;	// エラー番号の設定.
		}
		return False;
	}
}

#ifdef TORNADO
// 2011/03/25 CQ340449 河村 DeepSleep復帰NGリカバリ対応.
extern void change_cntSoftDog( int32_t cnt );
extern int32_t DEF_WDT_CNT_MAX;
static Bool bl_deepsleep = False;	// DeepSleep判定用フラグ.
#endif // TORNADO.

extern SEM_ID semWaitPEXPD;							// 2021/08/25 OP_BTS-35342 山口 PEXPDの遅延フラグの改善.
static volatile Bool bl_isHddConnectComplete = True;	// 他のタスクから変更される.
static Bool bl_isLowToNormalComplete = False;
Bool bl_isDelayedSequence = False;

void THRC_PowerManager::PowerChangeSequence( THRE_PowerSequenceID e_sequenceID, ushort us_sequenceState, uint32_t ul_param )
{
	static SEM_ID sem = NULL;
	// 2015/10/21 Harada 新詳細コード対応 >>.
	unsigned short us_DetailCode = 0x0000;		// 詳細コード.

//++ 2019/04/24 Sky		【EagleH】Warning除去対応.
#if defined(TORNADO) && !defined(FUM_LINUX) /*実機*/
	SEM_ID sem_pre = NULL;						// 前回のsem.
	int32_t si_errno = 0;							// errno.
	uint64_t ull_NewDetailCode = 0x0;				// 新詳細コード.
	Bool bl_ret = True;
#endif	// #if defined(TORNADO) && !defined(FUM_LINUX).
//-- 2019/04/24 Sky		【EagleH】Warning除去対応.
	// 2015/10/21 Harada 新詳細コード対応 <<.

	if( sem == NULL ) {
		sem = SYSF_semMCreate( SEM_Q_PRIORITY | SEM_INVERSION_SAFE );
		// 2015/10/21 Harada 新詳細コード対応 >>.
		if ( sem == NULL ) {
//++ 2019/04/24 Sky		【EagleH】Warning除去対応.
#if defined(TORNADO) && !defined(FUM_LINUX) /*実機*/
			si_errno = SYSF_errnoGet();
#endif	// #if defined(TORNADO) && !defined(FUM_LINUX).
//-- 2019/04/24 Sky		【EagleH】Warning除去対応.
			us_DetailCode = CFA14_DetailCode0x0FD1;
			THRC_PM_ASSERT("PowerChangeSequence(): SYSF_semMCreate error! sem = NULL", 0,0,0,0,us_DetailCode );
#if defined(TORNADO) && !defined(FUM_LINUX) /*実機*/
			//-- 2019/04/24 Sky		【EagleH】Warning除去対応.
			ull_NewDetailCode = (((uint64_t)0x22     << 56) & 0xff00000000000000)		// モジュール番号.
							  | (((uint64_t)si_errno << 24) & 0x00ffffffff000000);	// errno.
			printf( "(%010u)(BSP) ## %s us_DetailCode=%#x, ull_NewDetailCode=%#llx\n", SYSF_tickGet() , __FUNCTION__, us_DetailCode, ull_NewDetailCode);
			bl_ret = APIC_setNewDetailCode( us_DetailCode, ull_NewDetailCode );		// 詳細コード登録.
			if (False == bl_ret) {
				SYSF_logMsg( "## %s %d : APIC_setNewDetailCode NG\n", __FUNCTION__, __LINE__, 3,4,5,6 );
			}
#endif	// #if defined(TORNADO) && !defined(FUM_LINUX).
		}
		// 2015/10/21 Harada 新詳細コード対応 <<.
	}
//++ 2019/04/24 Sky		【EagleH】Warning除去対応.
#if defined(TORNADO) && !defined(FUM_LINUX) /*実機*/
	sem_pre = sem;	// 前回のsem.		// 2015/10/21 Harada 新詳細コード対応.
#endif	// #if defined(TORNADO) && !defined(FUM_LINUX).
//-- 2019/04/24 Sky		【EagleH】Warning除去対応.

	STATUS e_Status = OK; 
	e_Status = SYSF_semTake( sem, WAIT_FOREVER );
	if( e_Status != OK ) {
		//++ 2019/04/24 Sky		【EagleH】Warning除去対応.
#if defined(TORNADO) && !defined(FUM_LINUX) /*実機*/
		si_errno = SYSF_errnoGet();		// 2015/10/21 Harada 新詳細コード対応.
#endif	// #if defined(TORNADO) && !defined(FUM_LINUX).
		//-- 2019/04/24 Sky		【EagleH】Warning除去対応.
		PM_SEQ_REC( 0x02 );		/* CFA14_DetailCode0x0AA9 */
		// 2015/10/21 Harada 新詳細コード対応 >>.
//		THRC_PM_ASSERT("PowerChangeSequence(): semTake error! e_Status = [%d]", e_Status,0,0,0,CFA14_DetailCode0x08E7 ); // 2010/03/04 ft_kondo CQ324295.
		us_DetailCode = CFA14_DetailCode0x08E7;
		THRC_PM_ASSERT("PowerChangeSequence(): semTake error! e_Status = [%d]", e_Status,0,0,0,us_DetailCode ); // 2010/03/04 ft_kondo CQ324295.
		// 2015/10/21 Harada 新詳細コード対応 >>.
#if defined(TORNADO) && !defined(FUM_LINUX) /*実機*/
		ull_NewDetailCode = (((uint64_t)0x22     << 56) & 0xff00000000000000)		// モジュール番号.
						  | (((uint64_t)si_errno << 24) & 0x00ffffffff000000);	// errno.
		printf( "(%010u)(BSP) ## %s us_DetailCode=%#x, ull_NewDetailCode=%#llx\n", SYSF_tickGet() , __FUNCTION__, us_DetailCode, ull_NewDetailCode);
		bl_ret = APIC_setNewDetailCode( us_DetailCode, ull_NewDetailCode );		// 詳細コード登録.
		if (False == bl_ret) {
			SYSF_logMsg( "## %s %d : APIC_setNewDetailCode NG\n", __FUNCTION__, __LINE__, 3,4,5,6 );
		}
#endif	// #if defined(TORNADO) && !defined(FUM_LINUX).
		// 2015/10/21 Harada 新詳細コード対応 <<.
	}
	// 2011/02/23 ft_kondo 第6回手動マージ. 
	// 2010/09/24 伊藤史 CQ333291：電源制御方向の保全.
	uchar	uc_localChangeStatusAfter = uc_ChangeStatusAfter;
//	uchar	uc_localChangeStatusBefore = uc_ChangeStatusBefore;

	static Bool bl_isSdramClearComplete = False;				// 2009/07/15 河村 CQ319244.
	static Bool bl_isLowToNormalCompleteExceptHDD = False;/* S73-BSP_DD-28 4.4. */
	uint32_t ul_nextParam = ul_param;		// デフォルトは前のパラメータを継承する.
	bl_isDelayedSequence = False;
#if 0
	uint32_t ul_time = 0;
#endif // #if 0.
	
	THRC_PM_LOG( "%"PRId32" : PowerChangeSequence( %d, %d, %"PRId32")", SYSF_tickGet(), e_sequenceID, us_sequenceState, ul_param );
	// 2014/02/20 ft_kondo 省電力復帰制御をパラレルで動かす対応. 
	THRS_PowerManagerMsg st_Msg;
	st_Msg.uc_localChangeStatusBefore = uc_ChangeStatusBefore;  // 2015/10/09 CQ.519888 河村 - Before追加.
	st_Msg.uc_localChangeStatusAfter = uc_ChangeStatusAfter;

	switch ( e_sequenceID ) {
	case THRE_PowerSequenceID_NormalToLow:					// 省電力移行.
		switch ( us_sequenceState ) {
		case 0:
			// 2011/04/29 浦崎 CQ.342154対応。
			//	system("echo 1 > /sys/bus/platform/devices/PMU.0/power/wakeupInterruptEnable");	/* 復帰要因割り込みを有効にする */
			//	THRC_PM_LOG("Resume factor interrupt ON");
			PM_SEQ_REC( 0x03 );		/* CFA14_DetailCode0x0AAA */
			if( THRC_PowerStateID_NOT_EXIST_DEVICE != pc_powerDevice->GetPowerState( THRC_PowerDeviceID_HDD )
			 && isNormalToLowOfDevice( THRC_PowerDeviceID_FileSystem ) ) {
				// HDDが存在しており、省電力へ移行の場合, HDD_DRIVERの省電力は遅延して行う必要がある.
				// FileSystemの遅延制御はDocの初期化が完了している状態で5秒待ち.
				PM_SEQ_REC( 0x04 );		/* CFA14_DetailCode0x0AAB */
				Bool bl_canCount = canFileSystemCount();
				THRC_PM_LOG( "Power saving transition of HDD canFileSystemCount() = %d", bl_canCount );
				if( bl_canCount ) {
					// 時間計測できる状態.
					ul_nextParam = SYSF_tickGet();
#if 0
					ul_time = THRD_HDD_SUSPEND_WAIT;		// GaiaPF8536対応.
#endif // #if 0.
					// 2010/02/23 ft_kondo 第6回手動マージ. 
					PM_SEQ_REC( 0x05 );		/* CFA14_DetailCode0x0AAC */
					pc_powerDevice->SwitchPowerState( THRC_PowerDeviceID_FileSystem,	PowerStateID( THRC_PowerDeviceID_FileSystem,	uc_localChangeStatusAfter ) );
					PM_SEQ_REC( 0x06 );		/* CFA14_DetailCode0x0AAD */
//					pc_powerDevice->SwitchPowerState( THRC_PowerDeviceID_FileSystem,	PowerStateID( THRC_PowerDeviceID_FileSystem,	uc_ChangeStatusAfter ) );
				} else {
					// 時間計測出来ない状態。ポーリングで計測可能になるまで待つ.
					ul_nextParam = 0;
#if 0
					ul_time = 0;
#endif // #if 0.
				}
#if 0 /* ↓S73-BSP_DD-28 4.7. */
				// 次の定期起床で次のステートを実行.
				DelayedPowerChangeSequence( ul_time, e_sequenceID, (ushort)(us_sequenceState+1), ul_nextParam );
				bl_isDelayedSequence = True;
#endif /* ↑S73-BSP_DD-28 4.7. */
			} else {
				bl_isDelayedSequence = False;
			}
			// 2010/02/23 ft_kondo 第6回手動マージ. 
			PM_SEQ_REC( 0x07 );		/* CFA14_DetailCode0x0AAE */
			// 2014/05/13 ft_horii 省電力中のブザー対応.
			st_Msg.us_message = TM_ReqPowerSavePanel;
			SYSF_msgQSend( thePowerManagerTask.msgQID_PowerManager_Panel, (char*)&st_Msg, sizeof(st_Msg), WAIT_FOREVER, MSG_PRI_NORMAL );
#if 0 // Helios対応 暫定対応.
			// PANEL_POWERの制御先であるPNL_P_OFFはHeliosでは存在しないため、PANEL_POWERの処理は、LCDC_LVDSに移植している.
			// そのため、THRC_PowerDeviceID_PanelをLowにするとパネル通信がsuspendされるため、FAX受信音は鳴らなくなる.
			// Heliosとしては、省電力中のFAX受信音が必要であれば、別途対応が必要.
			// 現状では、暫定でコメントアウトしておく.
			SYSF_semTake( semID_PanelPower, WAIT_FOREVER );	// セマフォ取得.
			// 2013/10/03 河村 CQ.382962 - 省電力中のFAX受信音対応.
			if( panelPowerMode() == THRE_PanelPowerMode_Normal ) {
				// IT4仕様.
				pc_powerDevice->SwitchPowerState( THRC_PowerDeviceID_PANEL_POWER,	PowerStateID( THRC_PowerDeviceID_Panel,			uc_localChangeStatusAfter ) );
			} else {
				// IT3仕様(IMGSYSと同じ電源ステートを使用する).
				pc_powerDevice->SwitchPowerState( THRC_PowerDeviceID_PANEL_POWER,	PowerStateID( THRC_PowerDeviceID_IMGSYS,		uc_localChangeStatusAfter ) );
			}
			SYSF_semGive( semID_PanelPower );	// セマフォ開放.
			PM_SEQ_REC( 0x09 );		/* CFA14_DetailCode0x0AB0 */
#endif
//			pc_powerDevice->SwitchPowerState( THRC_PowerDeviceID_Panel,			PowerStateID( THRC_PowerDeviceID_Panel,			uc_ChangeStatusAfter ) );
//			pc_powerDevice->SwitchPowerState( THRC_PowerDeviceID_PANEL_POWER,	PowerStateID( THRC_PowerDeviceID_Panel,			uc_ChangeStatusAfter ) );
			if( False == bl_isDelayedSequence && isNormalToLowOfDevice( THRC_PowerDeviceID_Panel ) && isNormalToLowOfDevice( THRC_PowerDeviceID_PCIe_BUS ) ) {
				// 2008/05/09 河村 CQ.303235 - HDD遅延制御が発生しない & パネルの省電力移行を行う & PCIeの省電力移行を行う場合は2秒の遅延制御.
				PM_SEQ_REC( 0x0a );		/* CFA14_DetailCode0x0AB1 */
				DelayedPowerChangeSequence( 2000, e_sequenceID, 2, ul_nextParam );
				bl_isDelayedSequence = True;
			}
			
			// 2010/02/23 ft_kondo 第6回手動マージ. 
			PM_SEQ_REC( 0x0b );		/* CFA14_DetailCode0x0AB2 */
			pc_powerDevice->SwitchPowerState( THRC_PowerDeviceID_IR,			PowerStateID( THRC_PowerDeviceID_IR,			uc_localChangeStatusAfter ) );
			PM_SEQ_REC( 0x0c );		/* CFA14_DetailCode0x0AB3 */
//			pc_powerDevice->SwitchPowerState( THRC_PowerDeviceID_IR,			PowerStateID( THRC_PowerDeviceID_IR,			uc_ChangeStatusAfter ) );
			// 2010/02/23 ft_kondo 第6回手動マージ. 
			// 2008/04/09 河村 CQ.302130 - Fiery_PT_T制御変更.
			pc_powerDevice->SwitchPowerState( THRC_PowerDeviceID_Fiery_PT_T,	PowerStateID( THRC_PowerDeviceID_Fiery_PT_T,	uc_localChangeStatusAfter ) );
			PM_SEQ_REC( 0x0d );		/* CFA14_DetailCode0x0AB4 */
//			pc_powerDevice->SwitchPowerState( THRC_PowerDeviceID_Fiery_PT_T,	PowerStateID( THRC_PowerDeviceID_Fiery_PT_T,	uc_ChangeStatusAfter ) );
			// -- IR_RY OFFをPWRへ通知.
			THRC_PM_LOG( "sendMessage( TID_MachineMan, TM_IRPowerSaveEndNotice, 0 );" );
			sendMessage( TID_MachineMan, TM_IRPowerSaveEndNotice, 0 );
			// 2010/02/23 ft_kondo 第6回手動マージ. 
			pc_powerDevice->SwitchPowerState( THRC_PowerDeviceID_Fiery,			PowerStateID( THRC_PowerDeviceID_Fiery,			uc_localChangeStatusAfter ) );
			PM_SEQ_REC( 0x0e );		/* CFA14_DetailCode0x0AB5 */
//			pc_powerDevice->SwitchPowerState( THRC_PowerDeviceID_Fiery,			PowerStateID( THRC_PowerDeviceID_Fiery,			uc_ChangeStatusAfter ) );

			// GaiaPFにはUSB HUBの電源制御は存在しないのでDonau相当.
#ifdef DEF_TaigaCus
			// USB省電力対応.
			if ( uc_ExcludeUSBHost == 0 ) {			// 電源制御部からのUSBホストローパワー指示があったら.
				// 2010/02/23 ft_kondo 第6回手動マージ. 
				if ( PowerStateID( THRC_PowerDeviceID_USB,	uc_localChangeStatusAfter ) == THRC_PowerStateID_LOW ) {		// USBホストの遷移後電源ステータス時の状態がLowだったら.
//				if ( PowerStateID( THRC_PowerDeviceID_USB,	uc_ChangeStatusAfter ) == THRC_PowerStateID_LOW ) {		// USBホストの遷移後電源ステータス時の状態がLowだったら.
#ifdef TORNADO
					if (( USBPortInfoIsPluggedAnyDevice( USBPORTINFO_PORTID_SERVICE0 ) == False ) ||																	// ダウンロードポートの蓋が閉まっていたら.
						( getUsbBulkMountName( USBPortInfoHubPortNum( USBPORTINFO_PORTID_SERVICE0 ), USBPortInfoHubID( USBPORTINFO_PORTID_SERVICE0 )) == NULL )) {		// ダウンロードポートに何も挿されてなかったら.
						portRST3_USBHUB = 0;			// USB-HUBの電源をOFF.
					}
#endif // TORNADO.
				}
			}
#else
//			pc_powerDevice->SwitchPowerState( THRC_PowerDeviceID_USB,	PowerStateID( THRC_PowerDeviceID_USB,	uc_ChangeStatusAfter ));
#endif // DEF_TaigaCus.

//Str【MinervaZeus CQNo.361285対応】: A.Watanabe(ICS) : 2012.02.17.
			if( ( uc_ChangeStatusAfter == THRE_PowerSaveStatusSubPowerOffDelay )
			 || ( uc_ChangeStatusAfter == THRE_PowerSaveStatusSubPowerOff )
			 || ( uc_ChangeStatusAfter == THRE_PowerSaveStatusSubPowerOff15 ) )
			{
				SYSF_taskDelay((int32_t)(SYSF_sysClkRateGet() * 200 / 1000 ));		// 200msec遅延.
			}
//End【MinervaZeus CQNo.361285対応】: A.Watanabe(ICS) : 2012.02.17.

			// -- ↓ここからPCI_BUS.
			// 2010/02/23 ft_kondo 第6回手動マージ. 
			// PIC/CPSはAsic777に統合されており、Asic777のSuspend/Resumeは行わない方針（DevOff-modeSTRによって行われる）であるため、.
			// ここでのPIC/CPSのリセットは行わない.
			PM_SEQ_REC( 0x13 );		/* CFA14_DetailCode0x0ABA */
			// 遅延制御が不要な場合.
			if( bl_isDelayedSequence == False) {
				/*setCTSensor( CTSensor_ON );*/     /* 10/11/25 浦崎 静電センサー対応。 *//* 2011/10/05 第1OSW浦崎 CQ.355237対応。近接センサー有効無効タイミングも合わせて修正 */
				THRC_PM_LOG( "Panel/HDD No delay control required. Continue sequence");
				PM_SEQ_REC( 0x15 );		/* CFA14_DetailCode0x0ABC */
				PowerChangeSequence( e_sequenceID, 2, 0 );
			}
			break;
		case 1:	// FileSystem ウエイトポーリング.
			if( canFileSystemCount() ) {
				// Doc初期化完了している.
				if( ul_param == 0 ) {
					// まだ計測開始していない場合は計測開始tick値記録.
					ul_nextParam = SYSF_tickGet();
					PM_SEQ_REC( 0x16 );		/* CFA14_DetailCode0x0ABD */
					pc_powerDevice->SwitchPowerState( THRC_PowerDeviceID_FileSystem,	PowerStateID( THRC_PowerDeviceID_FileSystem,	uc_localChangeStatusAfter ) );
					PM_SEQ_REC( 0x17 );		/* CFA14_DetailCode0x0ABE */
//					pc_powerDevice->SwitchPowerState( THRC_PowerDeviceID_FileSystem,	PowerStateID( THRC_PowerDeviceID_FileSystem,	uc_ChangeStatusAfter ) );
				}
			} else {
				// Doc初期化中.
				ul_nextParam = 0;
				// 次の定期起床でこのステートを再度実行.
				PM_SEQ_REC( 0x18 );		/* CFA14_DetailCode0x0ABF */
				DelayedPowerChangeSequence( 0, e_sequenceID, us_sequenceState, ul_nextParam );
				break;
			}
			
			if( time_after( SYSF_tickGet(), ul_nextParam + (uint32_t)SYSF_sysClkRateGet() * THRD_HDD_SUSPEND_WAIT / 1000 ) ) {
				/*setCTSensor( CTSensor_ON );*/     /* 10/11/25 浦崎 静電センサー対応。 *//* 2011/10/05 第1OSW浦崎 CQ.355237対応。近接センサー有効無効タイミングも合わせて修正 */
				// 計測開始からPM_HDD_SUSPEND_WAIT[ms]の経過→遅延制御後の処理.
				THRC_PM_LOG( "FileSystem stop %d[ms] Passage. Continue sequence", THRD_HDD_SUSPEND_WAIT );
				PM_SEQ_REC( 0x19 );		/* CFA14_DetailCode0x0AC0 */
				PowerChangeSequence( e_sequenceID, 2, 0 );	// FileSystem待ち以降のシーケンスへ.
			} else {
				// 次の定期起床でこのステートを再度実行.
				PM_SEQ_REC( 0x1a );		/* CFA14_DetailCode0x0AC1 */
				DelayedPowerChangeSequence( 100, e_sequenceID, us_sequenceState, ul_nextParam );	// 0→100.
			}
//			PM_SEQ_REC(CFA14_DetailCode0x0AC2);			// 2012.10.22 CQ.373869 内野 PM_SEQ_REC(CFA14_DetailCode0x0ACC),PM_SEQ_REC(0x0000)をコール後本箇所へ戻る.FA14詳細コードが0x0000にならなくなるため削除.
			break;

		case 2:
#ifdef DEF_TaigaCus
			// USB省電力対応.
			// 2010/02/23 ft_kondo 第6回手動マージ. 
			if ( pc_powerDevice->SwitchPowerState( THRC_PowerDeviceID_USB,	PowerStateID( THRC_PowerDeviceID_USB,	uc_localChangeStatusAfter )) == False ) {		// USBホストの電源断が失敗したら.
//			if ( pc_powerDevice->SwitchPowerState( THRC_PowerDeviceID_USB,	PowerStateID( THRC_PowerDeviceID_USB,	uc_ChangeStatusAfter )) == False ) {		// USBホストの電源断が失敗したら.
				portRST3_USBHUB = 1;		// USB-HUBの電源をON.
			}
#else
			// 2010/08/24 GaiaPF8536対応 河村 - 電源制御部から自邸があったときのみ制御する.
			if ( uc_ExcludeUSBHost == 0 ) {			// 電源制御部からのUSBホストローパワー指示があったら.
				// 2010/02/23 ft_kondo 第6回手動マージ. 
				PM_SEQ_REC( 0x1c );		/* CFA14_DetailCode0x0AC3 */
				pc_powerDevice->SwitchPowerState( THRC_PowerDeviceID_USB,	PowerStateID( THRC_PowerDeviceID_USB,	uc_localChangeStatusAfter ));
				PM_SEQ_REC( 0x1d );		/* CFA14_DetailCode0x0AC4 */
//				pc_powerDevice->SwitchPowerState( THRC_PowerDeviceID_USB,	PowerStateID( THRC_PowerDeviceID_USB,	uc_ChangeStatusAfter ));
			}
#endif // DEF_TaigaCus.

			// 2010/02/23 ft_kondo 第6回手動マージ. 
			// 遅延制御完了後のシーケンス.
			PM_SEQ_REC( 0x1e );		/* CFA14_DetailCode0x0AC5 */
			pc_powerDevice->SwitchPowerState( THRC_PowerDeviceID_HDD_DRIVER,	PowerStateID( THRC_PowerDeviceID_HDD_DRIVER,	uc_localChangeStatusAfter ) );
			PM_SEQ_REC( 0x1f );		/* CFA14_DetailCode0x0AC6 */
			pc_powerDevice->SwitchPowerState( THRC_PowerDeviceID_PCIe_BUS,		PowerStateID( THRC_PowerDeviceID_PCIe_BUS,		uc_localChangeStatusAfter ) );
			PM_SEQ_REC( 0x20 );		/* CFA14_DetailCode0x0AC7 */
			// 音声IC対応 2014/9/5 小林 サウンドデバイス追加.
			pc_powerDevice->SwitchPowerState( THRC_PowerDeviceID_SOUND,			PowerStateID( THRC_PowerDeviceID_SOUND,			uc_localChangeStatusAfter ) );
			PM_SEQ_REC( 0x21 );		/* CFA14_DetailCode0x0AC8 */
			pc_powerDevice->SwitchPowerState( THRC_PowerDeviceID_CPU_FAN,		PowerStateID( THRC_PowerDeviceID_CPU_FAN,		uc_localChangeStatusAfter ) );
			PM_SEQ_REC( 0x22 );		/* CFA14_DetailCode0x0AC9 */
			pc_powerDevice->SwitchPowerState( THRC_PowerDeviceID_IMGSYS,		PowerStateID( THRC_PowerDeviceID_IMGSYS,		uc_localChangeStatusAfter ) );
			PM_SEQ_REC( 0x23 );		/* CFA14_DetailCode0x0ACA */
//			pc_powerDevice->SwitchPowerState( THRC_PowerDeviceID_HDD_DRIVER,	PowerStateID( THRC_PowerDeviceID_HDD_DRIVER,	uc_ChangeStatusAfter ) );
//			pc_powerDevice->SwitchPowerState( THRC_PowerDeviceID_PCIe_BUS,		PowerStateID( THRC_PowerDeviceID_PCIe_BUS,		uc_ChangeStatusAfter ) );
//			pc_powerDevice->SwitchPowerState( THRC_PowerDeviceID_USB_DOORSENSOR_EN,	PowerStateID( THRC_PowerDeviceID_USB_DOORSENSOR_EN,		uc_ChangeStatusAfter ) );	// 2008/05/29 河村 CQ.304589 - USBフタセンサ対応.
//			pc_powerDevice->SwitchPowerState( THRC_PowerDeviceID_CPU_FAN,		PowerStateID( THRC_PowerDeviceID_CPU_FAN,		uc_ChangeStatusAfter ) );
//			pc_powerDevice->SwitchPowerState( THRC_PowerDeviceID_IMGSYS,		PowerStateID( THRC_PowerDeviceID_IMGSYS,		uc_ChangeStatusAfter ) );

//			PM_SEQ_REC( 0x24 );		/* CFA14_DetailCode0x0ACB */
			pc_powerDevice->SwitchPowerState( THRC_PowerDeviceID_PEXPD,			PowerStateID( THRC_PowerDeviceID_PEXPD,			uc_localChangeStatusAfter ) ); /* 2014/07/07 takas Helios対応 Sleep1でPEX-PDを切る対応 */
			/*setCTSensor( CTSensor_ON );*/     /* 10/11/25 浦崎 静電センサー対応。 *//* 2011/10/05 第1OSW浦崎 CQ.355237対応。近接センサー有効無効タイミングも合わせて修正 */
			PM_SEQ_REC( 0x25 );		/* CFA14_DetailCode0x0ACC */
			SendSequenceEndToPWR();	// PWRへシーケンス完了を通知.
/*			PM_SEQ_REC(0x0000);*/
			PM_SEQ_REC_END();
			sync();/* S73-BSP_DD-28 4.8. */
			break;

		default:
			PM_SEQ_REC( 0x26 );		/* CFA14_DetailCode0x0ACD */
			THRC_PM_MSG( "PowerChangeSequence() : THRC_PowerSequenceID_NormalToLowで無効ステートが指定された\n" );
			THRC_PM_ASSERT("PowerChangeSequence() : Irregular State! e_sequenceID = [%d]", e_sequenceID,0,0,0,CFA14_DetailCode0x08E8 ); // 2010/03/04 ft_kondo CQ324295.
			break;
		}
		break;

	// 2016/10/31 sky_ishiga [MinervaMLK][起動時間短縮] スレッド省電力制御の並列制御対応.
	case THRE_PowerSequenceID_NormalToPowerOff:				// 電源OFF移行.

		// 各デバイスの省電力処理を制御するタスクを生成. 
		si_taskID_PM_PCIe_PCIDev	= SYSF_taskSpawn( "TASK_PM_PCIe_PCIDev",50, VX_FP_TASK,	10000, (FUNCPTR)THRC_PowerManager::messageLoop_PCIePCIDev,	0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
		si_taskID_PM_Fiery			= SYSF_taskSpawn( "TASK_PM_Fiery",50, VX_FP_TASK,		10000, (FUNCPTR)THRC_PowerManager::messageLoop_Fiery,		0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
		si_taskID_PM_USB			= SYSF_taskSpawn( "TASK_PM_USB",50, VX_FP_TASK,			10000, (FUNCPTR)THRC_PowerManager::messageLoop_USB,			0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
		si_taskID_PM_HDD			= SYSF_taskSpawn( "TASK_PM_HDD",50, VX_FP_TASK,			10000, (FUNCPTR)THRC_PowerManager::messageLoop_HDD,			0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
		si_taskID_PM_IR				= SYSF_taskSpawn( "TASK_PM_IR",50, VX_FP_TASK,			10000, (FUNCPTR)THRC_PowerManager::messageLoop_IR,			0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

		// デバイス復帰完了待ちステータスを設定.
		e_PM_DeviceStatus = ( THRE_PM_DeviceStatus_USB			// USB.
							| THRE_PM_DeviceStatus_Panel		// Panel.
							| THRE_PM_DeviceStatus_PCIePCIDev	// PCIe、PCIデバイスなど.
							| THRE_PM_DeviceStatus_IR			// IR.
							| THRE_PM_DeviceStatus_Fiery		// Fiery.
							| THRE_PM_DeviceStatus_Hdd );		// HDD.

		// Hdd.
		st_Msg.us_message = TM_ReqPowerSaveHdd;
		SYSF_msgQSend( thePowerManagerTask.msgQID_PowerManager_Hdd, (char*)&st_Msg, sizeof(st_Msg), WAIT_FOREVER, MSG_PRI_NORMAL );
		// IR.
		st_Msg.us_message = TM_ReqPowerSaveIR;
		SYSF_msgQSend( thePowerManagerTask.msgQID_PowerManager_IR, (char*)&st_Msg, sizeof(st_Msg), WAIT_FOREVER, MSG_PRI_NORMAL );
		// Fiery.
		st_Msg.us_message = TM_ReqPowerSaveFiery;
		SYSF_msgQSend( thePowerManagerTask.msgQID_PowerManager_Fiery, (char*)&st_Msg, sizeof(st_Msg), WAIT_FOREVER, MSG_PRI_NORMAL );
		// Panel.
		st_Msg.us_message = TM_ReqPowerSavePanel;
		SYSF_msgQSend( thePowerManagerTask.msgQID_PowerManager_Panel, (char*)&st_Msg, sizeof(st_Msg), WAIT_FOREVER, MSG_PRI_NORMAL );
		// USB
		st_Msg.us_message = TM_ReqPowerSaveUsb;
		SYSF_msgQSend( thePowerManagerTask.msgQID_PowerManager_USB, (char*)&st_Msg, sizeof(st_Msg), WAIT_FOREVER, MSG_PRI_NORMAL );
		// PCIePCIDev.
		st_Msg.us_message = TM_ReqPowerSavePCIe_PCIDev;
		SYSF_msgQSend( thePowerManagerTask.msgQID_PowerManager_PCIePCIDev, (char*)&st_Msg, sizeof(st_Msg), WAIT_FOREVER, MSG_PRI_NORMAL );

		// PWRへシーケンス完了を通知.
		SendSequenceEndToPWR();
		break;

	case THRE_PowerSequenceID_LowToNormal:					// 省電力解除.
		switch ( us_sequenceState ) {
		case 0:
			// 2012/01/20 ft_kondo CQ.360494 umountが発生していた場合、C-FA14を発生させてリブートさせる. 
			if( theHDDTrans.isUmountError() == True ) {	// umountエラー時Reboot対応 2012/01/16 河村 umountエラーが発生している場合はFA14でリブートする.
#ifdef TORNADO
				SYSF_logMsg( "@@ %"PRId32" : %s - umountでエラーが検出されているFA14(Dcode:%X)\n", SYSF_tickGet(), __FUNCTION__, (uintptr_t)theHDDTrans.getFA14DetailCode(), (uintptr_t)4, (uintptr_t)5, (uintptr_t)6 );
#endif// #ifdef TORNADO
				DebugForeverFA14( CFA14_DebugFatal, (ushort)(theHDDTrans.getFA14DetailCode() + 0x6000) ); //暗号化設定していない場合は、0x6001～の詳細コード。CQ374156 森田.
				APIC_SystemServiceFunction*	pc_SystemServiceFunction = APIC_SystemServiceFunction::newInstance();
				pc_SystemServiceFunction->reboot(); // リブートさせる. 
			}
			// 2011/04/29 浦崎 CQ.342154対応。
			//	system("echo 0 > /sys/bus/platform/devices/PMU.0/power/wakeupInterruptEnable");	/* 復帰要因割り込みを無効にする */
			//	THRC_PM_LOG("Resume factor interrupt OFF");
			// 2010/02/23 ft_kondo 第6回手動マージ. 
			PM_SEQ_REC( 0x27 );		/* CFA14_DetailCode0x0ACE */
			// 2014/04/15 ft_kondo Zeus5BK 省電力復帰時間短縮対応. 
#ifdef DEF_Helios_PF // Emu3以降では、パネル→IMGSYSの順番で復帰させる. 
			st_Msg.us_message = TM_ReqWakeUpPanel;
			SYSF_msgQSend( thePowerManagerTask.msgQID_PowerManager_Panel, (char*)&st_Msg, sizeof(st_Msg), WAIT_FOREVER, MSG_PRI_NORMAL );
			pc_powerDevice->SwitchPowerState( THRC_PowerDeviceID_IMGSYS,		PowerStateID( THRC_PowerDeviceID_IMGSYS,		uc_localChangeStatusAfter ) );
			PM_SEQ_REC( 0x28 );		/* CFA14_DetailCode0x0ACF */
#else		// PreEmuとEmu2では、従来通りIMGSYSから復帰させる. 
			SYSF_semTake( semID_PanelPower, WAIT_FOREVER );	// セマフォ取得.
			pc_powerDevice->SwitchPowerState( THRC_PowerDeviceID_IMGSYS,		PowerStateID( THRC_PowerDeviceID_IMGSYS,		uc_localChangeStatusAfter ) );
			SYSF_semGive( semID_PanelPower );	// セマフォ開放.
			PM_SEQ_REC(CFA14_DetailCode0x0ACF);

			st_Msg.us_message = TM_ReqWakeUpPanel;
			SYSF_msgQSend( thePowerManagerTask.msgQID_PowerManager_Panel, (char*)&st_Msg, sizeof(st_Msg), WAIT_FOREVER, MSG_PRI_NORMAL );
#endif // #ifdef DEF_Helios_PF. 
#if 0 // Helios対応 暫定対応.
			// PANEL_POWERの制御先であるPNL_P_OFFはHeliosでは存在しないため、PANEL_POWERの処理は、LCDC_LVDSに移植している.
			// そのため、THRC_PowerDeviceID_PanelをLowにするとパネル通信がsuspendされるため、FAX受信音は鳴らなくなる.
			// Heliosとしては、省電力中のFAX受信音が必要であれば、別途対応が必要.
			// 現状では、暫定でコメントアウトしておく.
			SYSF_semTake( semID_PanelPower, WAIT_FOREVER );	// セマフォ取得.
			// 2013/10/03 河村 CQ.382962 - 省電力中のFAX受信音対応.
			if( panelPowerMode() == THRE_PanelPowerMode_Normal ) {
				// IT4仕様.
				pc_powerDevice->SwitchPowerState( THRC_PowerDeviceID_PANEL_POWER,	PowerStateID( THRC_PowerDeviceID_Panel,			uc_localChangeStatusAfter ) );
			} else {
				// IT3仕様(IMGSYSと同じ電源ステートを使用する).
				pc_powerDevice->SwitchPowerState( THRC_PowerDeviceID_PANEL_POWER,	PowerStateID( THRC_PowerDeviceID_IMGSYS,		uc_localChangeStatusAfter ) );
			}
			SYSF_semGive( semID_PanelPower );	// セマフォ開放.
			PM_SEQ_REC( 0x29 );		/* CFA14_DetailCode0x0AD0 */
#endif
//			pc_powerDevice->SwitchPowerState( THRC_PowerDeviceID_IMGSYS,		PowerStateID( THRC_PowerDeviceID_IMGSYS,		uc_ChangeStatusAfter ) );
//			pc_powerDevice->SwitchPowerState( THRC_PowerDeviceID_PANEL_POWER,	PowerStateID( THRC_PowerDeviceID_Panel,			uc_ChangeStatusAfter ) );
			// Str 2010/09/14 ft_kondo DeepSleep中のFax受信でエラーが発生する問題.
//			if( isLowToNormalOfDevice( THRC_PowerDeviceID_IMGSYS ) ) {

			// 2017/02/22 SuperWarp起動中の場合はAsic復帰通知の前にパネル電源ONを通知する(Panel点灯遅くなる問題対応).
			// 2018/08/06 ishiga SuperWarp起動中判断について適切なAPIを使用するように修正.
			if (APIC_Power::newInstance()->isSuperWarpFirstBootConfiguration() == True) {
				THRC_PM_LOG("sendMessage( TID_MachineMan, TM_PanelPowerOn, 0 );");
				sendMessage( TID_MachineMan, TM_PanelPowerOn, 0 );
			}
			// 2021/08/25 OP_BTS-35342 山口 PEXPDの遅延フラグの改善.
			if( semWaitPEXPD == NULL ) {
				semWaitPEXPD = SYSF_semMCreate( SEM_Q_PRIORITY | SEM_INVERSION_SAFE );
			}
			SYSF_semTake( semWaitPEXPD, WAIT_FOREVER );
			// 2014/10/27 ft_kondo DeepSleep復帰時間が仕様値未達問題. 
			pc_powerDevice->SwitchPowerState( THRC_PowerDeviceID_PEXPD,			PowerStateID( THRC_PowerDeviceID_PEXPD,			uc_localChangeStatusAfter ) ); /* 2014/07/07 takas Helios対応 Sleep1でPEX-PDを切る対応 */
			SYSF_semGive( semWaitPEXPD );		// セマフォ開放.	
			// ASIC22を起こしたメッセージ.
			THRC_PM_LOG( "sendMessage( TID_MachineMan, TM_Asic22PowerSaveCancel, 0 );" );
			PM_SEQ_REC( 0x2a );		/* CFA14_DetailCode0x0AD1 */
			sendMessage( TID_MachineMan, TM_Asic22PowerSaveCancel, 0 );
//			}
			// End 2010/09/14 ft_kondo DeepSleep中のFax受信でエラーが発生する問題.
			// 2010/02/23 ft_kondo 第6回手動マージ. 
			PM_SEQ_REC( 0x2b );		/* CFA14_DetailCode0x0AD2 */
			// IRの復帰制御. 
			st_Msg.us_message = TM_ReqWakeUpIR;
			SYSF_msgQSend( thePowerManagerTask.msgQID_PowerManager_IR, (char*)&st_Msg, sizeof(st_Msg), WAIT_FOREVER, MSG_PRI_NORMAL );

			// HDDの復帰制御. 
			st_Msg.us_message = TM_ReqWakeUpHdd;
			st_Msg.e_sequenceID = e_sequenceID;
			st_Msg.ul_nextParam = ul_param;
			st_Msg.b_NormalShift = isLowToNormalOfDevice( THRC_PowerDeviceID_FileSystem );	// FileSystemの復帰が必要かどうか、予め判断しておく. 
			
			SYSF_msgQSend( thePowerManagerTask.msgQID_PowerManager_Hdd, (char*)&st_Msg, sizeof(st_Msg), WAIT_FOREVER, MSG_PRI_NORMAL );

			// PCIe、PCIデバイスの復帰制御. 
			st_Msg.bl_isSdramClearComplete = bl_isSdramClearComplete;
			st_Msg.us_message = TM_ReqWakeUpPCIe_PCIDev;
			SYSF_msgQSend( thePowerManagerTask.msgQID_PowerManager_PCIePCIDev, (char*)&st_Msg, sizeof(st_Msg), WAIT_FOREVER, MSG_PRI_NORMAL );

			// Fieryの復帰制御. 
			st_Msg.us_message = TM_ReqWakeUpFiery;
			SYSF_msgQSend( thePowerManagerTask.msgQID_PowerManager_Fiery, (char*)&st_Msg, sizeof(st_Msg), WAIT_FOREVER, MSG_PRI_NORMAL );

			// USBの復帰制御. 
			st_Msg.us_message = TM_ReqWakeUpUsb;
			SYSF_msgQSend( thePowerManagerTask.msgQID_PowerManager_USB, (char*)&st_Msg, sizeof(st_Msg), WAIT_FOREVER, MSG_PRI_NORMAL );

			// HDDが存在していない or HDD接続が完了している or HDD_DRIVERの省電力解除無し.
			// 2009/07/15 河村 CQ319244 - クリアDMA完了条件追加.
			if( ( pc_powerDevice->GetPowerState( THRC_PowerDeviceID_HDD ) == THRC_PowerStateID_NOT_EXIST_DEVICE
			   || bl_isHddConnectComplete == True
			   || !isLowToNormalOfDevice( THRC_PowerDeviceID_HDD_DRIVER ) )
			&& bl_isSdramClearComplete == True ) {
				bl_isLowToNormalCompleteExceptHDD = True;/* S73-BSP_DD-28 4.4. */
#ifdef TORNADO
				// 2011/03/25 CQ340449 河村 DeepSleep復帰NGリカバリ対応.
				PM_SEQ_REC( 0x4a );		/* CFA14_DetailCode0x0AF1 */
				if( bl_deepsleep == True ){	// DeepSleepからの復帰.
					change_cntSoftDog( DEF_WDT_CNT_MAX );	// ウォッチドッグタイマ終了.
					bl_deepsleep = False;	// DeepSleepフラグクリア.
					PM_SEQ_REC( 0x4b );		/* CFA14_DetailCode0x0AF2 */
				}
#endif // TORNADO.
				PM_SEQ_REC( 0x4c );		/* CFA14_DetailCode0x0AF3 */
				// 2014/03/03 ft_kondo このタイミングでは電源部に応答を返さない. 
//				SendSequenceEndToPWR();	// PWRへシーケンス完了を通知.
/*				PM_SEQ_REC(0x0000);*/                // 2012.10.15 内野 正常に省電力移行復帰完了時はNVRAMを0に戻す.
			} else {
				bl_isLowToNormalComplete = True;	// 省電力のシーケンスを最後まで行った.
				PM_SEQ_REC( 0x4d );		/* CFA14_DetailCode0x0AF4 */
			}
			break;

		case 4:
			// 2010/08/20 GaiaPF8536対応 河村 - HDD制御 遅延時間経過 FileSystem制御.
			if( False == b_rec_HDD ){ // 1回だけ、0x4eを記録する. 
				PM_SEQ_REC( 0x4e );		/* CFA14_DetailCode0x0AF5 */
				b_rec_HDD = True;
			}
			if( isLinkedHdd() == False || wdTestCode == 6 ) {	// [forTest] 2012/10/04 河村 - CQ363535 wd実装テスト.
				DelayedPowerChangeSequence( 100, e_sequenceID, us_sequenceState, ul_nextParam );
//				PM_SEQ_REC( 0x4f );		/* CFA14_DetailCode0x0AF6 */
				break;
			}
			THRC_PM_LOG( "HDD recognition" );
			// 2010/02/23 ft_kondo 第6回手動マージ. 
			PM_SEQ_REC( 0x50 );		/* CFA14_DetailCode0x0AF7 */
			// 2014/07/25 CQ.501828 ft_kondo タイミングによっては、PowerStateIDの戻り値がLowとなるので、NORMAL固定で処理する. 
			pc_powerDevice->SwitchPowerState( THRC_PowerDeviceID_FileSystem,	THRC_PowerStateID_NORMAL );
			PM_SEQ_REC( 0x51 );		/* CFA14_DetailCode0x0AF8 */
//			pc_powerDevice->SwitchPowerState( THRC_PowerDeviceID_FileSystem,	PowerStateID( THRC_PowerDeviceID_FileSystem,	uc_ChangeStatusAfter ) );
			// HDD接続完了後case 1が実行される.
			// 2014/07/25 CQ.501828 ft_kondo HDDの接続完了まで、次の省電力シーケンスは動かさない対応. 
			// 2014/08/19 CQ.501018 sky_tksk PC-Printの子ジョブがHDD認識を待たないようにする対応 start.
			// HDD認識するとミラーリングを許可する.
			{
				IMGC_ImageMemoryPoolImpColorFileMemoryInstance& comMemoryManager = IMGC_ImageMemoryPoolImpColorFileMemorySingletonIf::newInstance();
				if ( comMemoryManager.isIgnoreHddAccess() == True ) {
					THRC_PM_LOG( "Allow HDD mirroring Seq 4" );
					PM_SEQ_REC( 0x6a );		/* CFA14_DetailCode0x0B11 */ // kondo_新規. 
					comMemoryManager.resetIgnoreHddAccess() ;
				}
			}
			PM_SEQ_REC( 0x6b );		/* CFA14_DetailCode0x0B12 */ // kondo_新規. 
			// 2014/08/19 CQ.501018 sky_tksk PC-Printの子ジョブがHDD認識を待たないようにする対応 end.
			
#if 0		// 2014/10/28 toda CQ504323/504728/505515 HDDの接続完了まで、次の省電力シーケンスは動かさない対応.
			// ここはまだHDDを認識したところなので、ここでフラグを落としてしまうと、この後動作するHDD接続処理中に、.
			// HDDの切断処理が動いてしまいFA14が発生してしまう.
			b_doPowerChangeSequence_Hdd = False;
#endif
			
			break;
			
		case 1:
			// 2011/02/23 ft_kondo 第6回手動マージ. 
			// HDD接続完了の通知を受けた.
			PM_SEQ_REC( 0x52 );		/* CFA14_DetailCode0x0AF9 */
			
			// 2014/10/28 toda CQ504323/504728/505515 HDDの接続完了まで、次の省電力シーケンスは動かさない対応.
			// フラグを落とすタイミングを「HDD認識」ではなく、「HDD接続完了」とする.
			// 2015/02/04 河村 CQ.509407 - C-D050誤検出問題対応.
			THRC_PM_LOG( "%d : HDD connection completion time %d[tick]", SYSF_tickGet(), SYSF_tickGet() - ul_PM_HDD_tickGetTime1 );
			/* 2016/04/19 CQ527820 Altantic省電力問題No6. 既に削除されているWdタイマは削除しない */
			if (si_taskID_PM_Watch_HDD) {
				SYSF_taskDelete( si_taskID_PM_Watch_HDD );
				si_taskID_PM_Watch_HDD = 0;
				// 2016/06/06 CQ530712 unknown wdId問題対応、再Delete/Cancelできないようにゼロクリアする。
				if (wdID_Watch_HDD) {
					SYSF_wdCancel( wdID_Watch_HDD );			// タイマーの停止. 
					SYSF_wdDelete( wdID_Watch_HDD );			// タイマーの削除. 
					wdID_Watch_HDD = 0;
				}
			}
			b_doPowerChangeSequence_Hdd = False;

			// 2014/03/03 ft_kondo Zeus5BKでは、省電力シーケンス中で無い時にHDD接続完了が来る事があるため、アサート処理を削除. 
//			if( uc_localChangeStatusBefore == THRE_PowerSaveStatusNone ) {
//			if( uc_ChangeStatusBefore == THRE_PowerSaveStatusNone ) {
//				THRC_PM_MSG( "PowerChangeSequence() : 省電力シーケンス中でないのにHDD接続完了が通知された\n" );
//				PM_SEQ_REC( 0x53 );		/* CFA14_DetailCode0x0AFA */
//				THRC_PM_ASSERT("PowerChangeSequence() : Irregular State Change HDD Activate1! uc_localChangeStatusBefore = [%d]", uc_localChangeStatusBefore,0,0,0,CFA14_DetailCode0x08E9 ); // 2010/03/04 ft_kondo CQ324295. 
//				THRC_PM_ASSERT("PowerChangeSequence() : Irregular State Change HDD Activate1! uc_ChangeStatusBefore = [%d]", uc_ChangeStatusBefore,0,0,0 ); // 2010/03/04 ft_kondo CQ324295. 
//				THRC_PM_ASSERT("");
//			}
			if( bl_isHddConnectComplete == True ) {
//				THRC_PM_MSG( "PowerChangeSequence() : HDD接続要求していないのに完了が通知された\n" );
				PM_SEQ_REC( 0x54 );		/* CFA14_DetailCode0x0AFB */
				THRC_PM_ASSERT("PowerChangeSequence() : Irregular State Change HDD Activate2! bl_isHddConnectComplete = [%d]", bl_isHddConnectComplete,0,0,0,CFA14_DetailCode0x08EA ); // 2010/03/04 ft_kondo CQ324295. 
//				THRC_PM_ASSERT("");
			}
			bl_isHddConnectComplete = True;
			// 2009/07/15 河村 CQ319244 - クリアDMA完了条件追加.
			if( bl_isLowToNormalComplete == True && bl_isSdramClearComplete == True ) {
				bl_isLowToNormalCompleteExceptHDD = True;/* S73-BSP_DD-28 4.4. */
#ifdef TORNADO
				// 2011/03/25 CQ340449 河村 DeepSleep復帰NGリカバリ対応.
				if( bl_deepsleep == True ){	// DeepSleepからの復帰.
					PM_SEQ_REC( 0x55 );		/* CFA14_DetailCode0x0AFC */
					change_cntSoftDog( DEF_WDT_CNT_MAX );	// ウォッチドッグタイマ終了.
					bl_deepsleep = False;	// DeepSleepフラグクリア.
				}
#endif // TORNADO.
				// 省電力のシーケンスを最後まで行っている場合、ここで通知する必要がある.
				PM_SEQ_REC( 0x56 );		/* CFA14_DetailCode0x0AFD */
				// 2014/03/03 ft_kondo このタイミングでは電源部に応答を返さない. 
//				SendSequenceEndToPWR();	// PWRへシーケンス完了を通知.
/*				PM_SEQ_REC(0x0000);*/                // 2012.10.15 内野 正常に省電力移行復帰完了時はNVRAMを0に戻す.
			}
			// 2015/01/16 Harada CQ.509010 省電力制御パラレル対応に合わせた省電力シーケンス記録フラグ対応.
			e_PM_SeqReqEndComplete &= ~THRE_PM_SeqReqEndComplete_HDD;				// 復帰完了したデバイスのフラグを落とす.
			if (THRE_PM_SeqReqEndComplete_Complete == e_PM_SeqReqEndComplete) {		// 省電力シーケンスの最後の処理かどうか.
				PM_SEQ_REC_END();	// 2014/10/16 ft_kondo ここが省電力シーケンスの最後の処理となった場合. 
			}
			break;

		case 2:
			// 2008/04/09 河村 CQ.302130 - IR初期接完了通知受信(省電力解除)を受けた.
			PM_SEQ_REC( 0x57 );		/* CFA14_DetailCode0x0AFE */
			pc_powerDevice->SwitchPowerState( THRC_PowerDeviceID_Fiery_PT_T,	THRC_PowerStateID_NORMAL );
			PM_SEQ_REC( 0x58 );		/* CFA14_DetailCode0x0AFF */
			// 2015/01/16 Harada CQ.509010 省電力制御パラレル対応に合わせた省電力シーケンス記録フラグ対応.
			e_PM_SeqReqEndComplete &= ~THRE_PM_SeqReqEndComplete_IR;				// 復帰完了したデバイスのフラグを落とす.
			if (THRE_PM_SeqReqEndComplete_Complete == e_PM_SeqReqEndComplete) {		// 省電力シーケンスの最後の処理かどうか.
				PM_SEQ_REC_END();	// 2014/10/16 ft_kondo ここが省電力シーケンスの最後の処理となった場合. 
			}
			break;	

		case 3:
			PM_SEQ_REC( 0x59 );		/* CFA14_DetailCode0x0B00 */
			// 2009/07/15 河村 CQ319244 - クリアDMA完了通知.
			bl_isSdramClearComplete = True;
			if( bl_isLowToNormalComplete == True && bl_isHddConnectComplete == True ) {
				bl_isLowToNormalCompleteExceptHDD = True;/* S73-BSP_DD-28 4.4. */
#ifdef TORNADO
				// 2011/03/25 CQ340449 河村 DeepSleep復帰NGリカバリ対応.
				PM_SEQ_REC( 0x5a );		/* CFA14_DetailCode0x0B01 */
				if( bl_deepsleep == True ){	// DeepSleepからの復帰.
					PM_SEQ_REC( 0x5b );		/* CFA14_DetailCode0x0B02 */
					change_cntSoftDog( DEF_WDT_CNT_MAX );	// ウォッチドッグタイマ終了.
					bl_deepsleep = False;	// DeepSleepフラグクリア.
				}
#endif // TORNADO.
				PM_SEQ_REC( 0x5c );		/* CFA14_DetailCode0x0B03 */
				// 2014/03/03 ft_kondo このタイミングでは電源部に応答を返さない. 
//				SendSequenceEndToPWR();	// PWRへシーケンス完了を通知.
/*				PM_SEQ_REC(0x0000);*/                // 2012.10.15 内野 正常に省電力移行復帰完了時はNVRAMを0に戻す.
			}
			/* ↓S73-BSP_DD-28 4.4. */
			else if( ( True == bl_isLowToNormalComplete )&&( False == bl_isLowToNormalCompleteExceptHDD ) ) {
				// 2014/08/19 CQ.501018 sky_tksk PC-Printの子ジョブがHDD認識を待たないようにする対応 start.
				// 省電力復帰制御がパラレル化したことで「HDDを除く省電力解除完了通知」を出すタイミングを変えたため削除.
//				SendSequencePCPrintToPWR();
				// 2014/08/19 CQ.501018 sky_tksk PC-Printの子ジョブがHDD認識を待たないようにする対応 end.
				bl_isLowToNormalCompleteExceptHDD = True;
			}
			/* ↑S73-BSP_DD-28 4.4. */
			break;

		default:
			PM_SEQ_REC( 0x5d );		/* CFA14_DetailCode0x0B04 */
			THRC_PM_ASSERT("PowerChangeSequence() : Irregular State Change HDD Activate3! us_sequenceState = [%d]", us_sequenceState,0,0,0,CFA14_DetailCode0x08EB ); // 2010/03/04 ft_kondo CQ324295.
			break;
		}
		break;
	default:
		PM_SEQ_REC( 0x5e );		/* CFA14_DetailCode0x0B05 */
		break;
	}
	e_Status = SYSF_semGive( sem );
	if( e_Status != OK ) {
		//++ 2019/04/24 Sky		【EagleH】Warning除去対応.
#if defined(TORNADO) && !defined(FUM_LINUX) /*実機*/
		si_errno = SYSF_errnoGet();		// 2015/10/21 Harada 新詳細コード対応.
#endif	// #if defined(TORNADO) && !defined(FUM_LINUX).
		//-- 2019/04/24 Sky		【EagleH】Warning除去対応.
		PM_SEQ_REC( 0x5f );		/* CFA14_DetailCode0x0B06 */
		// 2015/10/21 Harada 新詳細コード対応 >>.
//		THRC_PM_ASSERT("::PowerChangeSequence(): semGive error! e_Status = [%d]", e_Status,0,0,0,CFA14_DetailCode0x08EC ); // 2010/03/04 ft_kondo CQ324295.
		us_DetailCode = CFA14_DetailCode0x08EC;
		THRC_PM_ASSERT("::PowerChangeSequence(): semGive error! e_Status = [%d]", e_Status,0,0,0,us_DetailCode ); // 2010/03/04 ft_kondo CQ324295.
		// 2015/10/21 Harada 新詳細コード対応 >>.
#if defined(TORNADO) && !defined(FUM_LINUX) /*実機*/
		ull_NewDetailCode = (((uint64_t)0x22                     << 56) & 0xff00000000000000)		// モジュール番号.
						  | (((uint64_t)si_errno                 << 24) & 0x00ffffffff000000)		// errno.
						  | (((uint64_t)(sem != sem_pre ? 1 : 0) << 23) & 0x0000000000800000);	// semが変更されているか否か.
		printf( "(%010u)(BSP) ## %s us_DetailCode=%#x, ull_NewDetailCode=%#llx\n", SYSF_tickGet() , __FUNCTION__, us_DetailCode, ull_NewDetailCode);
		bl_ret = APIC_setNewDetailCode( us_DetailCode, ull_NewDetailCode );		// 詳細コード登録.
		if (False == bl_ret) {
			SYSF_logMsg( "## %s %d : APIC_setNewDetailCode NG\n", __FUNCTION__, __LINE__, 3,4,5,6 );
		}
#endif	// #if defined(TORNADO) && !defined(FUM_LINUX).
		// 2015/10/21 Harada 新詳細コード対応 <<.
	}
}

// ================================================================================================.
//! <!--機能--> 電源管理 制御部 　IR電源制御：IR初期化完了通知後省電力復帰処理.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param 　なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
//  09/08/07 越野	 Donau環境から分離して作成.
//  2008/04/09 河村 CQ.302130 - Fiery_PT_Tの制御をIR初期通信完了後に行うためここへ制御を追加.
// 13/02/28 Sky 機種共通化対応 THRC_PowerManagerSoC.cppから移植.
// ================================================================================================.
void THRC_PowerManager::PowerIRpowerCtl2()
{
	// 2008/04/09 河村 CQ.302130 - Fiery_PT_Tの制御をIR初期通信完了後に行う.
	THRC_PM_LOG( "IR initial connection completion notification reception (power saving release)" );
	DelayedPowerChangeSequence( 0, THRE_PowerSequenceID_LowToNormal, 2, 0 );	// スレッド電源制御タスクで動作させるためこのサービスを使う.
}

// ================================================================================================.
//! <!--機能--> 電源管理 制御部  電源状態差分チェッカ.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! THRE_PowerSave_Device e_PowerSaveDevice     ：チェック対象デバイス（DEF_MAX_DeviceNumで全デバイス指定）.
//! THRE_PowerSaveStatus e_PowerSaveStatusBefore：遷移前の電源ステータス.
//! THRE_PowerSaveStatus e_PowerSaveStatusAfte r：遷移後の電源ステータス.
//! <!--戻値-->.
//! デバイス電源状態差分チェック結果.
//! <!--詳細-->.
//! 各電源ステータス遷移において、デバイス省電力状態の差分の有無を返す.
//! <!--履歴-->.
//  09/08/07 越野	 Donau環境から分離して作成.
// 13/02/28 Sky 機種共通化対応 THRC_PowerManagerSoC.cppから移植.
// ================================================================================================.
THRE_DevicePowerSaveStatusDiffAns THRC_PowerManager::getDevicePowerSaveStatusDiff( THRE_PowerSave_Device e_PowerSaveDevice, THRE_PowerSaveStatus e_PowerSaveStatusBefore, THRE_PowerSaveStatus e_PowerSaveStatusAfter )
{
	THRE_DevicePowerSaveStatusDiffAns e_AnsPara = THRE_DiffNone;

	// 遷移前と遷移後のデバイス省電力状態を比較.
	if( uc_MFPBoard_PowerSaveTableStruct[e_PowerSaveStatusBefore][e_PowerSaveDevice] != uc_MFPBoard_PowerSaveTableStruct[e_PowerSaveStatusAfter][e_PowerSaveDevice] )
	{
		// 状態遷移後のデバイスの省電力状態で分岐.
		if( uc_MFPBoard_PowerSaveTableStruct[e_PowerSaveStatusAfter][e_PowerSaveDevice] == DEF_PowerSave_Active )
		{
			// 遷移後のデバイスの省電力状態がActive → 戻り値に「差分有り・遷移後のデバイス省電力状態はActive」を設定.
			e_AnsPara = THRE_DiffExistToActive;
		}
		else
		{
			// 遷移後のデバイスの省電力状態がINActive → 戻り値に「差分有り・遷移後のデバイス省電力状態はInActive」を設定.
			e_AnsPara = THRE_DiffExistToInActive;
		}
	}
	return( e_AnsPara );
}

// ================================================================================================.
//! <!--機能--> 電源管理 制御部  HDD接続完了受け付け.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! <!--戻値-->.
//! <!--詳細-->.
//! <!--履歴-->.
//  09/08/07 越野	 Donau環境から分離して作成.
// 13/02/28 Sky 機種共通化対応 THRC_PowerManagerSoC.cppから移植.
// ================================================================================================.
void THRC_PowerManager::PowerChangeEnd()
{
	// IMGC_MemMan.cppよりHDD接続完了時に呼び出される.
	// Moselと異なりIMGC_MemManからの完了を受け付けるだけになっているので注意.
	THRC_PM_LOG( "Accept HDD connection completion" );
	char c=2; initDebugWrite( initDebugModule_THR, 6, &c, 1);	/*HDD接続完了通知を受けた*/
	PowerChangeSequence( THRE_PowerSequenceID_LowToNormal, 1, 0 );
}


// ================================================================================================.
//! <!--機能--> 電源管理 制御部  Powerへの完了メッセージ送信.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! <!--戻値-->.
//! <!--詳細-->.
//! <!--履歴-->.
//  09/08/07 越野	 Donau環境から分離して作成.
// 13/02/28 Sky 機種共通化対応 THRC_PowerManagerSoC.cppから移植.
// 17/02/13 Sky-Araki [起動時間短縮] 省電力時に主電源OFFするとフリーズ.
// ================================================================================================.
void THRC_PowerManager::SendSequenceEndToPWR()
{
	static EC_ChangePowerSaveStatusAnsID ec_ChangePowerSaveStatusAnsID;

	char c=2; initDebugWrite( initDebugModule_THR, 3, &c, 1);	/*PWRへ応答1*/
	// 二重通知チェック.
	if( uc_ChangeStatusBefore == THRE_PowerSaveStatusNone ) {
		THRC_PM_LOG( "PowerChangeEnd() : Sequence completion already notified to PWR" );
		return;
	}

	/* 2019/01/31 AR1072285 山口 CA72 WDT対応. */
	// 省電力復帰中の区間を対象とする.
	if (True == isWdOn_CA72) {	// CA72WDT有効の場合.
		if(uc_ChangeStatusAfter == THRE_PowerSaveStatusIdle){	// 遷移後電源ステータスが待機中.
#if 0	// 2020/04/28 河村 -  OP_BTS-22858 CA72WDTを省電力制御以外も動作させる対応.
			int32_t si_Fd = 0;
			int32_t si_Ret2 = 0;
			isWdStart_CA72 = False;	// CA72WDT Startクリア.
		
			si_Fd = SYSF_open(THRD_DeviceFile_Pmu, O_RDWR, 0);
			if (0 > si_Fd) {
				printf( "(%010u)(BSP) [%s] open failed. file=%s\n", SYSF_tickGet() , __func__, THRD_DeviceFile_Pmu);
			}
			si_Ret2 = ioctl( si_Fd, THRD_PMU_IOCSSET_WDT_CA72, DEF_WDT_CA72_END );	// END.
			if( si_Ret2 < 0 ) {
				printf( "(%010u)(BSP) %s: pmu ioctl(THRD_PMU_IOCSSET_WDT_CA72) Error!!! \n", SYSF_tickGet() , __FUNCTION__ );
			}
			SYSF_close(si_Fd);
#endif
			ca72wdt_ext_refresh( 0x4000, 300, 0x0000, "Not sleep sequence\n" );
		}
	}
	
	// PWRへ完了を通知(メッセージ).
	ec_ChangePowerSaveStatusAnsID = DEF_AnsAck;
	THRC_PM_LOG( "PowerChangeEnd() : Notified completion of sequence to PWR" );
	THRC_PM_LOG( "sendMessage( TID_MachineMan, TM_AnsEndChangePowerSaveStatus, xx );" );
	sendMessage( TID_MachineMan, TM_AnsEndChangePowerSaveStatus, (uintptr_t)ec_ChangePowerSaveStatusAnsID );
	c=3; initDebugWrite( initDebugModule_THR, 3, &c, 1);	/*PWRへ応答2*/
	// デバッグログ　省電力制御終了.
	setDebugLowSpeedSeq( 722, (uintptr_t)0 );

	//実行中シーケンス情報のクリア.
	uc_ChangeStatusBefore = THRE_PowerSaveStatusNone;
	uc_ChangeStatusAfter  = THRE_PowerSaveStatusNone;
	
	if( True== s_DelayedPowChgSeq ) s_DelayedPowChgSeq= False;
}

// ================================================================================================.
//! <!--機能--> 電源管理 制御部  PowerへのHDDを除く省電力制御完了応答メッセージ送信.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! <!--戻値-->.
//! <!--詳細-->.
//! <!--履歴-->.
//  13/09/21 Sky藤井 ディープスリープ省電力パフォーマンス対応 S73-BSP_DD-28 4.4. 
// ================================================================================================.
void THRC_PowerManager::SendSequencePCPrintToPWR()
{
	sendMessage( TID_MachineMan, TM_AnsEndChangePowerSaveStatusExceptHDD, (uintptr_t)DEF_AnsAck );
}

// ================================================================================================.
//! <!--機能--> 電源管理 制御部 　スリープ終了後のPCIコンフィグ実行中確認.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param 　なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
//  09/08/07 越野	 Donau環境から分離して作成.
// 13/02/28 Sky 機種共通化対応 THRC_PowerManagerSoC.cppから移植.
// ================================================================================================.
Bool THRC_PowerManager::isSleepEndConfigBusy()
{
	// PCI_CFGが省電力解除中であればTrueを返す → PCIに変更している.
	// [課題I/F]呼び元を新I/Fを使用に変更したいところ.
	if( THRC_PowerStateID_NORMAL == pc_powerDevice->GetToPowerState( THRC_PowerDeviceID_PCI_BUS) ) {
		return True;
	} else {
		return False;
	}
}
// ================================================================================================.
//! <!--機能--> 電源管理 制御部 　PIC2007スリープ状態確認.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param 　なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
//  09/08/07 越野	 Donau環境から分離して作成.
// 13/02/28 Sky 機種共通化対応 THRC_PowerManagerSoC.cppから移植.
// 14/04/10 toda Helios対応 Heliosで不要なためFalseを返すようにする.
// ================================================================================================.
Bool THRC_PowerManager::isPIC2007SleepBusy()
{
	// PICのスリープ移行中 or スリープ中の場合にTrueを返す.
	// [課題I/F]呼び元を新I/Fを使用に変更したいところ.
	// 本関数は現状未使用.Heliosでは取りあえずFalseを返しておく.
	return False;
}

// ================================================================================================.
//! <!--機能--> 電源管理 制御部 　パネルスリープ終了後のLCD初期化状態確認.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param 　なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
//  09/08/07 越野	 Donau環境から分離して作成.
// 13/02/28 Sky 機種共通化対応 THRC_PowerManagerSoC.cppから移植.
// ================================================================================================.
Bool THRC_PowerManager::isPanelSleepEndLCDInitBusy()
{
	// 08/02/05 sysAsicRegInit2() 河村.
	// ×Panel→○LCDCが省電力解除であればTrueを返す.
	// [課題I/F]呼び元を新I/Fを使用に変更したいところ.
	return isJpgHddAsic1200TRInitBusy();
}

// ================================================================================================.
//! <!--機能--> 電源管理 制御部 　JPEG,HDD,ASIC200スリープ終了後の初期化状態確認.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param 　なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
//  09/08/07 越野	 Donau環境から分離して作成.
// 13/02/28 Sky 機種共通化対応 THRC_PowerManagerSoC.cppから移植.
// ================================================================================================.
Bool THRC_PowerManager::isJpgHddAsic1200TRInitBusy()
{
	// THRE_PowerSave_ImageTransReceiveGroupが省電力解除中シーケンス中だったらTrueを返す.
	// THRC_ThreadBiosDonau::AsicRegInit()でASIC200の初期化が必要かどうかの判断に使用している.
	// [課題I/F]呼び元を新I/Fを使用に変更したいところ.
	if( THRE_PowerSaveStatusNone != uc_ChangeStatusBefore && THRE_PowerSaveStatusNone != uc_ChangeStatusAfter ) {
		if( THRE_DiffExistToActive == getDevicePowerSaveStatusDiff( THRE_PowerSave_ImageTransReceiveGroup, (THRE_PowerSaveStatus)uc_ChangeStatusBefore, (THRE_PowerSaveStatus)uc_ChangeStatusAfter ) ) {
			return True;
		} else {
			return False;
		}
		
	} else {
		return False;
	}
}

// ================================================================================================.
//! <!--機能--> 電源管理 制御部 　Deep Sleep移行時のRTC への復帰時間(設定)要求対応.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param 　int32_t return_time	:RTCへ設定する復帰時間.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
// 2010/07/12 内野 Minerva初版.
//  2011/10/15 MinervaZeus_int リベース DeepSleep復帰NGリカバリ対応有効化、2nd暫定対応削除.
// 13/02/28 Sky 機種共通化対応 THRC_PowerManagerSoC.cppから移植.
// ================================================================================================.
void THRC_PowerManager::setTimerNearestWakeupEvent(int32_t return_time)
{
	CB_RTC cb_RTC = CB_RTC::newInstance();

	/* アラーム設定 */
	cb_RTC.setAlarm( return_time );

	/* アラーム割り込み有効 */
	cb_RTC.setAlarmInterrupt( True );
}

// 20160203 USB印刷ジョブの隙間問題対応
// Return True:ディープスリープ遷移する False:ディープスリープ遷移しない(一応戻り値として残す)
Bool checkTransitionDeepSleep()
{
    APIC_ActiveJobList      *pc_ActiveJobList = APIC_ActiveJobList::newInstance();
    LIBC_List<APIC_Job>     c_List;

	/* Erpか副電源OFFモードならActiveJob有無のチェックは要らない */
	if ( True == bl_ErpMode || True == bl_SubPowOffMode ) {
		return True;
	}
    //-------------------------------.
    // アクティブリストがある場合.
    //-------------------------------.
    if( pc_ActiveJobList->getList(TYPE_ActiveJobListPrint, 1, TYPD_MaxActiveJob, &c_List) >= 1 ) {
		/* Lot26対応端末のLot26モードスリープ遷移ならUSBの復帰要因を仕込んで一旦はディープスリープへ */
		if( True == Lot26HwType && True == THRC_PowerManager::getUSBHostConnect() ) {
			THRC_PowerManagerDeviceSingletonIf::newInstance().SwitchPowerState( THRC_PowerDeviceID_SET_KR6,	THRC_PowerStateID_LOW );	// USBホスト通電.
			THRC_PM_LOG( "checkTransitionDeepSleep(): SwitchPowerState" );
			return True;
		}
	}
    return True;
}

#ifdef TORNADO
/* ↓F_TAIKIをNICFUMの代わりに操作 #2481 */
extern STATUS sysFTaikiRetentionSequence( Bool b_Close );
extern STATUS sysFTaikiSuependSequence( Bool b_Close );
extern STATUS sysFTaikiResumeSequence( Bool b_Close );
Bool b_close = True;
/* ↑F_TAIKIをNICFUMの代わりに操作 #2481 */
extern void exec_str();
// 2011/03/25 CQ340449 河村 DeepSleep復帰NGリカバリ対応.
extern int32_t wdtFd;
extern void start_cyclicLoop_taskSoftDog();
#endif

// ================================================================================================.
//! <!--機能--> 電源管理 制御部  ディープスリープ実行後の処理.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! <!--戻値-->.
//! <!--詳細-->.
//! <!--履歴-->.
//  13/09/21 Sky藤井 ディープスリープ省電力パフォーマンス対応 S73-BSP_DD-28 4.2. 
//  14/04/14 Sky藤井 ErP(リテンション)モード対応 S73-BSP_DD-29
//  2015/12/18 Sky-Araki CQ520828 Lot26対応.
//	2017/05/08 Sky-Araki AR1021331 xhci-hcd Timeout.
//  2021/08/16 OP_BTS-35243 山口 SuperWarp起動で認証装置を認識しない.
//  2024/01/04 OP_BTS-46374 Sky USB Resumeエラー対応.
// ================================================================================================.
void THRC_PowerManager::resumeAfterDeepSleep( 
	Bool b_ErP,
	t_eMioWkupMode MioWakeupMode,
	THRE_ReqShiftType e_ReqShiftType)/* ←S73-BSP_DD-29 */
{
#ifdef TORNADO			// 実機で有効.
	APIC_Power* pc_ApiPower = APIC_Power::newInstance();
	thePowerManagerTask.bWakeupUSB_Busy=True;	//	17/05/08 Sky-Araki AR1021331 xhci-hcd Timeout.
	//++ 2019/04/24 Sky		【EagleH】Warning除去対応.
	int32_t		si_Result = 0;
	//-- 2019/04/24 Sky		【EagleH】Warning除去対応.
	int32_t si_Fd = 0;
	int32_t si_Ret = 0;

	// WLANリセット処理をUSBのResume処理が完了させるまで待たせる.
	si_Fd = SYSF_open( THRD_DeviceFile_Pmu, O_RDWR, 0 );
	si_Ret = ioctl( si_Fd, THRD_PMU_IOCSSET_REQ_WLANRESET, (int64_t)1 );
	// 2023/09/12 Sky OP_BTS-44703 新詳細(0x2202804130000000)対応 End.
	
	int32_t i_Ret = THRF_taskBind( THRE_IDTYPE_THREAD, 0, THRE_CPUID_1 );
	if( 0 != i_Ret ) {
		THRC_PM_LOG( "THRF_taskBind returns %"PRId32" on %s line:%d", i_Ret, __FUNCTION__, __LINE__ );
	}
	// 2018/07/27 K.Sato Change USB SleepSequence [STR]suspend/resume [SuperWarp]insmod/rmmod
	//++ 2019/04/24 Sky		【EagleH】Warning除去対応.
	// 2021/08/16 OP_BTS-35243 山口 SuperWarp起動で認証装置を認識しない.
	if(THRE_ReqShiftType_GetSnapShot == e_ReqShiftType){
		// SuperWarpでは前回起動時のUSBデバイスの情報がライブラリ側に残ってしまう場合がある.
		// ライブラリに情報が残っているとUSBモジュールのロードで新しく認識されたUSBデバイスの情報を無視してしまう.
		// 下記関数を実行することでライブラリの持っているUSBデバイスの情報をクリアする.
		usb_find_busses();		// 全てのUSBバスを検索. 
		usb_find_devices();		// 全てのUSBデバイスを検索. 
		/* 2022/02/22 山口 OP_BTS-37466 DeepSleepではloadUSBModuleを実行しないように修正 */
		si_Result = system("/usr/local/bin/loadUSBModule.sh");
		if (si_Result != 0){
			printf("(%010u)(BSP) ## %s[%d] %s\n", SYSF_tickGet(), __FILE__, __LINE__, __FUNCTION__);
		}
	}

	//-- 2019/04/24 Sky		【EagleH】Warning除去対応.

	// 2018/11/07 K.Sato Check finish usb_resume
	int ret = 0;
	int cnt = 0;
	// 2022/02/04 山口 OP_BTS-37242 新詳細コード22028細分化.
#if defined(TORNADO) && !defined(FUM_LINUX) /*実機*/		
	uint32_t i = 0;
	uint32_t j = 0;
	char buf[20] = {0};
	char buf_num[4] = {0};
#endif	// #if defined(TORNADO) && !defined(FUM_LINUX).
	char buf_usb[4] = {0};
	FILE *fp_usb;
	const int32_t s32_usb_timeout = 200;//USB Resume完了のタイムアウト時間 [x100ms].
	while( cnt++ < s32_usb_timeout ){	//Timeout 20s  2024/01/04 10sec⇒20sec
		/* 2022/02/22 山口 OP_BTS-37441 usbのresume完了のチェックで使用していたsystem関数の廃止*/
		fp_usb = SYSF_fopen( "/sys/module/usbcore/parameters/usb_resume_fin", "rb" );
		memset( buf_usb, 0, sizeof(buf_usb) );	
		if( fp_usb != NULL ) {
			SYSF_Fseek( fp_usb, 0, SEEK_SET );
	        SYSF_Fread(buf_usb, sizeof(buf_usb[0]), (sizeof(buf_usb) / sizeof(buf_usb[0])) - 1, fp_usb);
    		SYSF_fclose(fp_usb);	        
			if(buf_usb[0] == 'Y'){
				break;	//finish USB Resume.
			}
			
		}
		DELAY_TSK(100);
	}

	// 2023/09/12 Sky OP_BTS-44703 新詳細(0x2202804130000000)対応 Start.
	// 2024/01/04 Sky OP_BTS-46374 USB Resumeエラー対応見直し Start.
	// WLANのUSBポートのチェックを削除、timeout 10s ⇒ 20s.
	if( cnt >= s32_usb_timeout ){	//Timeout 10s ⇒ 20s.
		printf("[TIMEOUT] Don't finish usb_resume\n");
		/* 2021/05/24 OP_BTS-33777 USB resume完了しない */
#if defined(TORNADO) && !defined(FUM_LINUX) /*実機*/		
			// 2023/09/12 Sky OP_BTS-44703 新詳細(0x2202804130000000)対応を削除.
			// 2022/02/04 山口 OP_BTS-37242 新詳細コード22028細分化.
			// エラーを起こしているqueueの状態を取得.
			FILE *fp_num = SYSF_fopen( "/sys/module/phy_usb3_mv/drivers/platform:mv-usb3-phy/8d00d8000.usb3h0p0_phy/power/queue", "rb" );

			uint64_t u64_queue = 0;
			if( fp_num != NULL ) {
		        fread(buf_num, sizeof(buf_num[0]), (sizeof(buf_num) / sizeof(buf_num[0])) - 1, fp_num);
		  	    fclose(fp_num);
				u64_queue = static_cast<uint64_t>(atoi(buf_num));
			}
			// エラーを起こしたUSBデバイスのパスを取得.
			FILE *fp = SYSF_fopen( "/sys/module/phy_usb3_mv/drivers/platform:mv-usb3-phy/8d00d8000.usb3h0p0_phy/power/pass", "rb" );
			if( fp != NULL ) {
				fread( buf, sizeof(buf[0]), (sizeof(buf) / sizeof(buf[0])) - 1, fp );
				SYSF_fclose(fp);
			}
			StopTm();
			Save_PsInfo();						/* 必要なログをSSD(uSD)へ保存 */
			system("dmesg");
			uint64_t ull_NewDetailCode = ((((uint64_t)0x22              << 56) & 0xff00000000000000)	// モジュール番号.
									   |  (((uint64_t)0x028             << 44) & 0x00fff00000000000)	// 通し番号.
									   |  (((uint64_t)u64_queue         << 40) & 0x00000f0000000000));	// queueの状態.
			// USBデバイスのパスを格納,残せる情報に限りがあるので'-'や'.'は省略する.
			for(i=0;i<sizeof(buf);i++){
				if(buf[i] == '\n'){
					break;
				}
				if( buf[i] != '-' && buf[i] != '.' ){
					ull_NewDetailCode |= ( (uint64_t)(buf[i] & 0xf) << (36 - j*4) );
					j++;
					if(j > 9){
						break;
					}
				}
			}
			
			printf( "(%010u)(BSP) ## %s us_DetailCode=0, ull_NewDetailCode=%#llx\n", SYSF_tickGet() , __FUNCTION__, ull_NewDetailCode);
			Bool bl_ret = True;
			bl_ret = APIC_setNewDetailCode( TYPE_FatalDetailCode_undefined, ull_NewDetailCode );		// 詳細コード登録.
			if (False == bl_ret) {
				SYSF_logMsg( "## %s %d : APIC_setNewDetailCode NG\n", __FUNCTION__, __LINE__, 3,4,5,6 );
			}
			SYSF_taskDelay(SYSF_sysClkRateGet() * 5);	// ログ出力待ち 5s wait.
			/* 本件発生時リブートコマンド発行でリブートしないため直接HRESET_REQを発行する */
			system("echo on > /sys/bus/platform/devices/PMU.0/devices/HRESET_REQ");
#endif	// #if defined(TORNADO) && !defined(FUM_LINUX).
	}
	// 2024/01/04 Sky OP_BTS-46374 USB Resumeエラー対応見直し Start.
	// ERROR Transfer event TRB DMA ptr not part 発生チェック.
	const uint32_t u32_usbResumeError = 0x02;		// B1 1:ERROR Transfer event TRB DMA 発生.
	uint32_t u32_usbError = 0;						// USB Resume実行状態.
	// USB Resumeの実行状態を取得.
	si_Result = ioctl( si_Fd, THRD_PMU_IOCSSET_RESET_USB_VAR, (int64_t)&u32_usbError );
	if( si_Result != 0 ){
		printf( "(%010u)(BSP) ## %s[%d] %s\n", SYSF_tickGet(), __FILE__, __LINE__, __FUNCTION__ );
	}
	// エラー発生チェック.
	if( ( u32_usbError & u32_usbResumeError ) != 0 ){
#if defined(TORNADO) && !defined(FUM_LINUX) /*実機*/
		// ERROR Transfer event TRB DMAが発生したため再インストールを実施.
		printf( "[USB Resume] Found ERROR Transfer event TRB DMA!!\n" );
		// STEP1.新詳細コードの登録.
		// queue状態を取得.
		FILE *fp_num = SYSF_fopen( "/sys/module/phy_usb3_mv/drivers/platform:mv-usb3-phy/8d00d8000.usb3h0p0_phy/power/queue", "rb" );
		uint64_t u64_queue = 0;
		if( fp_num != NULL ){
			fread( buf_num, sizeof( buf_num[0] ), ( sizeof( buf_num ) / sizeof( buf_num[0] ) ) - 1, fp_num );
			fclose( fp_num );
			u64_queue = static_cast<uint64_t>( atoi( buf_num ) );
		}
		// エラーを起こしたUSBデバイスのパスを取得.
		memset( buf, 0, sizeof( buf ) );
		FILE *fp = SYSF_fopen( "/sys/module/phy_usb3_mv/drivers/platform:mv-usb3-phy/8d00d8000.usb3h0p0_phy/power/pass", "rb" );
		if( fp != NULL ){
			fread( buf, sizeof( buf[0] ), ( sizeof( buf ) / sizeof( buf[0] ) ) - 1, fp );
			SYSF_fclose( fp );
		}
		uint64_t ull_NewDetailCode = ((((uint64_t)0x22      << 56) & 0xff00000000000000)	// モジュール番号.
								   |  (((uint64_t)0x030     << 44) & 0x00fff00000000000)	// 通し番号.
								   |  (((uint64_t)u64_queue << 40) & 0x00000f0000000000));	// queueの状態.
		// USBデバイスのパスを格納,残せる情報に限りがあるので'-'や'.'は省略する.
		for( i = 0; i < sizeof( buf ); i++ ){
			if( buf[i] == '\n' ){
				break;
			}
			if( buf[i] != '-' && buf[i] != '.' ){
				ull_NewDetailCode |= ( ( uint64_t )( buf[i] & 0xf) << ( 36 - j * 4 ) );
				j++;
				if( j > 9 ){
					break;
				}
			}
		}
		printf( "(%010u)(BSP) ## %s us_DetailCode=0, ull_NewDetailCode=%#llx\n", SYSF_tickGet() , __FUNCTION__, ull_NewDetailCode);
		Bool bl_ret = True;
		bl_ret = APIC_setNewDetailCode( TYPE_FatalDetailCode_undefined, ull_NewDetailCode );		// 詳細コード登録.
		if( False == bl_ret ){
			SYSF_logMsg( "## %s %d : APIC_setNewDetailCode NG\n", __FUNCTION__, __LINE__, 3, 4, 5, 6 );
		}
		// STEP2.USBドライバのアンインストール.
		// unloadUSBModule.shを実行するとWLANのドライバがアンインストールされるため、個別にrmmodを実施.
		printf( "[USB Resume] unloadUSBModule.sh\n" );
		si_Result = system( "rmmod -w /km/module/usb-storage.ko" );
		if( si_Result != 0 ){
			printf( "(%010u)(BSP) ## %s[%d] %s\n", SYSF_tickGet(), __FILE__, __LINE__, __FUNCTION__ );
		}
		si_Result = system( "rmmod -w /km/module/xhci-plat-hcd.ko" );
		if( si_Result != 0 ){
			printf( "(%010u)(BSP) ## %s[%d] %s\n", SYSF_tickGet(), __FILE__, __LINE__, __FUNCTION__ );
		}
		si_Result = system( "rmmod -w /km/module/xhci-pci.ko" );
		if( si_Result != 0 ){
			printf( "(%010u)(BSP) ## %s[%d] %s\n", SYSF_tickGet(), __FILE__, __LINE__, __FUNCTION__ );
		}
		si_Result = system( "rmmod -w /km/module/xhci-hcd.ko" );
		if( si_Result != 0 ){
			printf( "(%010u)(BSP) ## %s[%d] %s\n", SYSF_tickGet(), __FILE__, __LINE__, __FUNCTION__ );
		}
		si_Result = system( "rmmod -w /km/module/phy-usb3-mv.ko" );
		if( si_Result != 0 ){
			printf( "(%010u)(BSP) ## %s[%d] %s\n", SYSF_tickGet(), __FILE__, __LINE__, __FUNCTION__ );
		}
		DELAY_TSK( 1000 );
		// STEP3.USBドライバのインストール.
		printf( "[USB Resume] loadUSBModule.sh\n" );
		si_Result = system( "/usr/local/bin/loadUSBModule.sh" );
		if( si_Result != 0 ){
			printf( "(%010u)(BSP) ## %s[%d] %s\n", SYSF_tickGet(), __FILE__, __LINE__, __FUNCTION__ );
		}
		if( THRF_isExistWLAN() == True ){
			// 無線LANデバイスのエラー発生時にリセット処理を実施.
			// 2:USB3.0のunbind/bindを実行する.
			si_Result = ioctl( si_Fd, THRD_PMU_IOCSSET_APP_REQ_WLANRESET, 2 );
			if( si_Result != 0 ){
				printf( "(%010u)(BSP) ## %s[%d] %s\n", SYSF_tickGet(), __FILE__, __LINE__, __FUNCTION__ );
			}
		}
		DELAY_TSK( 1000 );
#endif /* defined(TORNADO) && !defined(FUM_LINUX) */
	}
	// 2024/01/04 Sky OP_BTS-46374 USB Resumeエラー対応見直し End.

	// 2018/08/02 K.Sato If setting Ethernet mode (don't use wlan), don't check WLAN status
	if( THRF_isExistWLAN() == True && 
	    TYPE_NetworkIFMode_SingleEthernet != APIC_ExNetworkSetting::newInstance()->getNetworkIFMode() &&
	    TYPE_NetworkIFMode_DualEthernet != APIC_ExNetworkSetting::newInstance()->getNetworkIFMode() ){
		ret = 0;
		cnt = 0;
		while( cnt++ < 100 ){	//Timeout 10s
			//Check WLAN stasus by polling
			ret = system( "test `cat /proc/net/rtl8812bu/wlan0/in_suspend` -eq 0" );
			if( !WEXITSTATUS( ret ) ){
				break;	//finish WLAN Resume
			}
			DELAY_TSK(100);
		}
	}

#if !defined(DEF_64BIT_PF)
	DELAY_TSK(1000);										//wakeupへアクセスできない対策のため、insmodが完了するまで1s待つ.
	/* 2014/07/08 USBデバイス名を変更 */
	system("echo disabled > /sys/bus/usb/devices/usb1/power/wakeup");
	system("echo disabled > /sys/bus/usb/devices/usb2/power/wakeup");
	system("echo disabled > /sys/bus/usb/devices/usb3/power/wakeup");
#if defined(DEF_MinervaS_PF)	/* 2015/05/04 Emu72 USBデバイス名を変更. */
	system("echo disabled > /sys/bus/usb/devices/usb3/3-1/power/wakeup");
#endif	/* defined(DEF_MinervaS_PF) */
	system("echo disabled > /sys/bus/usb/devices/usb4/power/wakeup");
	system("echo disabled > /sys/bus/usb/devices/usb5/power/wakeup");
#if !defined(DEF_MinervaS_PF)	/* 2015/05/04 Emu72 USBデバイス名を変更. */
	system("echo disabled > /sys/bus/usb/devices/usb5/5-1/power/wakeup");
#endif	/* !defined(DEF_MinervaS_PF) */
#if defined(DEF_MinervaS_PF)	/* 2015/05/29 Emu72 USBデバイス名を変更. */
	system("echo disabled > /sys/bus/usb/devices/usb6/power/wakeup");
#endif	/* defined(DEF_MinervaS_PF) */
#endif /* !defined(DEF_64BIT_PF) */

	/* 2014/07/11 NICFUM F_TAIKI対応 USBのinsmod/open処理 */
	i_Ret = mioPostChangeModeToNormal(MioWakeupMode);
	thePowerManagerTask.bWakeupUSB_Busy=False;		//	17/05/08 Sky-Araki AR1021331 xhci-hcd Timeout.
	THRL_ThAssert( (i_Ret == OK), "resumeAfterDeepSleep(), mioPostChangeModeToNormal() return ERROR", 0 , 0, 0, 0, CFA14_DetailCode0x0961 );
	// 2019/10/03 OP_BTS-11242 山口 詳細コード2008.
	pc_ApiPower->resetDeepSleepDelay( __FILE__, __LINE__ );	
	// 2023/09/12 Sky OP_BTS-44703 新詳細(0x2202804130000000)対応 Start.
	// WLANドライバからのリセット要求を実行する.
	si_Ret = ioctl( si_Fd, THRD_PMU_IOCSSET_REQ_WLANRESET, (int64_t)2 );
	SYSF_close( si_Fd );
	// 2023/09/12 Sky OP_BTS-44703 新詳細(0x2202804130000000)対応 End.
#endif // #ifdef TORNADO. 
}
// ================================================================================================.
//! <!--機能--> 電源管理 制御部  復帰要因割り込み設定処理.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.b_ErP Erpへ移行フラグ us_level DeepSleepのレベル.
//! <!--戻値-->.
//! <!--詳細-->.
//! <!--履歴-->.
// 2015/2/18 山口 CQ.509733 IT4と比較してDeepSleep移行時間が長くなっている.
//							そのため電サブへの割り込み設定処理を並列化し処理時間を短縮させる.
// 2015/12/18 Sky-Araki CQ520828 Lot26対応.
// 2019/11/06 ft_kondo  OP_BTS-16624 DeepSleep後にErP移行すると電力が高い. 
// ================================================================================================.
#define PMU_AP_CHECK_CNT 2 /* 無線LANによる復帰のための前処理用(値はTHRC_PowerManagerSoC::req_ErPShift()(THRC_PowerManagerSoC.cpp)から踏襲) */
static int32_t si_SubCpuIntEnd = 0;
void THRC_PowerManager::Task_pre_DeepSleepShift( 
	Bool 	b_ErP, 
	ushort 	us_level,
	Bool	bLot26Active	)
{
	
#ifdef TORNADO			// 実機で有効.
	
	THRC_PowerManagerDeviceSingletonIf::newInstance().SwitchPowerState( THRC_PowerDeviceID_SET_INTP4,	THRC_PowerStateID_LOW );	// 副電源SW.
	
	// 割り込み無効にするもの.
	if( True == b_ErP ) {	/* Erpモードの場合*/
		// ErP個別で割り込み設定
		// 割り込み有効にするもの.
		// 無線LANからの起動の割り込みはreq_DeepSleepShiftで行う.理由はmioWlanSetErPMode前に行う必要があるため.
		//THRC_PowerManagerDeviceSingletonIf::newInstance().SwitchPowerState( THRC_PowerDeviceID_SET_KR0,	  THRC_PowerStateID_LOW );	// 無線LANからの起動.
		// 割り込み無効にするもの.
		THRC_PowerManagerDeviceSingletonIf::newInstance().SwitchPowerState( THRC_PowerDeviceID_SET_INTP5, THRC_PowerStateID_NORMAL );// 原稿セット(EDH). 2014/11/13 CQ.504087.
		THRC_PowerManagerDeviceSingletonIf::newInstance().SwitchPowerState( THRC_PowerDeviceID_SET_INTP6, THRC_PowerStateID_NORMAL );// 原稿セット(DSET).2014/11/13 CQ.504087.
		THRC_PowerManagerDeviceSingletonIf::newInstance().SwitchPowerState( THRC_PowerDeviceID_SET_INTP7, THRC_PowerStateID_NORMAL );// 前カバーの開閉.  2014/11/13 CQ.504087.
		THRC_PowerManagerDeviceSingletonIf::newInstance().SwitchPowerState( THRC_PowerDeviceID_SET_KR1,	  THRC_PowerStateID_NORMAL );// FAXからの起動.
		THRC_PowerManagerDeviceSingletonIf::newInstance().SwitchPowerState( THRC_PowerDeviceID_SET_KR3,	  THRC_PowerStateID_NORMAL );// PHYからの起動.
		THRC_PowerManagerDeviceSingletonIf::newInstance().SwitchPowerState( THRC_PowerDeviceID_SET_KR4,   THRC_PowerStateID_NORMAL );// Fieryからの起動. /* 14.11.28 / sky_nkia / CQ.507406 */
		THRC_PowerManagerDeviceSingletonIf::newInstance().SwitchPowerState( THRC_PowerDeviceID_SET_KR5,	  THRC_PowerStateID_NORMAL );// 人体検出センサ.  2014/10/3 CQ.504507 堀井.
		THRC_PowerManagerDeviceSingletonIf::newInstance().SwitchPowerState( THRC_PowerDeviceID_SET_KR2,	  THRC_PowerStateID_NORMAL );// MODEM RI信号.  //2014/10/08 FT_yamaguchi CQ.503322. 
		THRC_PowerManagerDeviceSingletonIf::newInstance().SwitchPowerState( THRC_PowerDeviceID_SET_KR7,	  THRC_PowerStateID_NORMAL );// SMNP(Ⅱ).        2014/11/13 CQ.504087.
	} else {
		// DeepSleep個別で割り込み設定
		// 割り込み有効にするもの.
		THRC_PowerManagerDeviceSingletonIf::newInstance().SwitchPowerState( THRC_PowerDeviceID_SET_KR6,	THRC_PowerStateID_LOW );	// USBホスト通電.
		THRC_PowerManagerDeviceSingletonIf::newInstance().SwitchPowerState( THRC_PowerDeviceID_SET_KR5,	THRC_PowerStateID_LOW );	// 人体検出センサ. 2014/10/3 CQ.504507 堀井.
		THRC_PowerManagerDeviceSingletonIf::newInstance().SwitchPowerState( THRC_PowerDeviceID_SET_KR7,	THRC_PowerStateID_LOW );	// SMNP(Ⅱ).
#if defined(DEF_S800) /* S800はDeepSleepからFieryで復帰可能 */
		THRC_PowerManagerDeviceSingletonIf::newInstance().SwitchPowerState( THRC_PowerDeviceID_SET_KR4,	THRC_PowerStateID_LOW );	// Fieryからの起動.
#endif /* defined(DEF_S800) */
		// 2014/10/08 FT_yamaguchi CQ.503322. 
		// HWのバグでCSRCの割り込みが常に飛んできてしまう.マスクの設定のフラグを追加し、コマンドよりHW改修済みの場合は割り込み受付可、非改修の場合は受付不可に設定する.
		if( True == bl_MODEM_MaskSetting ){
			THRC_PowerManagerDeviceSingletonIf::newInstance().SwitchPowerState( THRC_PowerDeviceID_SET_KR2,		THRC_PowerStateID_LOW );	// MODEM RI信号.
		} else {
			THRC_PowerManagerDeviceSingletonIf::newInstance().SwitchPowerState( THRC_PowerDeviceID_SET_KR2,		THRC_PowerStateID_NORMAL );	// MODEM RI信号.
		}
		// 割り込み無効にするもの.
		THRC_PowerManagerDeviceSingletonIf::newInstance().SwitchPowerState( THRC_PowerDeviceID_SET_KR3,	THRC_PowerStateID_NORMAL );	// PHYからの起動.
		// 2014/10/27 CQ.505932 / CQ.505934 河村 - 拡張ディープスリープ復帰時に原稿セットや前ドア開を検出してしまう問題の対策.
		switch( us_level ) {
		case 1:		// 0.5w.
			// 拡張ディープスリープモード(0.5w)の場合は使用できないセンサの割り込みは無効にする.
			// 有効はTHRC_PowerObserver::powerStatusChanged()にて行われる.
			THRC_PowerManagerDeviceSingletonIf::newInstance().SwitchPowerState( THRC_PowerDeviceID_SET_INTP5,	THRC_PowerStateID_NORMAL );	// 原稿セット(EDH).
			THRC_PowerManagerDeviceSingletonIf::newInstance().SwitchPowerState( THRC_PowerDeviceID_SET_INTP6,	THRC_PowerStateID_NORMAL );	// 原稿セット(DSET).
			THRC_PowerManagerDeviceSingletonIf::newInstance().SwitchPowerState( THRC_PowerDeviceID_SET_INTP7,	THRC_PowerStateID_NORMAL );	// 前カバーの開閉.
			break;
		default:
			break;
		}
	}
	
/* 2014/11/13 M.Handa CQ.507316対応 end */


	if( False == b_ErP ) {
		/* DeepSleep 0.5W/0.7Wを指定 */
		switch( us_level ) {
		case 0:
			THRC_PowerManagerDeviceSingletonIf::newInstance().SwitchPowerState( THRC_PowerDeviceID_ENG_SENSOR,	THRC_PowerStateID_NORMAL );
			break;
		case 1:
			THRC_PowerManagerDeviceSingletonIf::newInstance().SwitchPowerState( THRC_PowerDeviceID_ENG_SENSOR,	THRC_PowerStateID_LOW );
			break;
		default:
			printf( "(%010u)(BSP) %s : Param error. b_ErP=%"PRId32", us_level=%"PRId32"\n", SYSF_tickGet() , __FUNCTION__, (int32_t)b_ErP, (int32_t)us_level );
			THRC_PowerManagerDeviceSingletonIf::newInstance().SwitchPowerState( THRC_PowerDeviceID_ENG_SENSOR,	THRC_PowerStateID_NORMAL );
			break;
		}
	// 2019/11/06 ft_kondo  OP_BTS-16624 DeepSleep後にErP移行すると電力が高い. 
	} else {		// ErPの場合は常にOFFにする. 
		THRC_PowerManagerDeviceSingletonIf::newInstance().SwitchPowerState( THRC_PowerDeviceID_ENG_SENSOR,	THRC_PowerStateID_LOW );
	}
	si_SubCpuIntEnd = 0;
#endif // #ifdef TORNADO. 
}


// ================================================================================================.
//! <!--機能--> 電源管理 制御部 　DeepSleep復帰要因の解析.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return OK / ERROR.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
// 2014/10/21 半田  SNMP以外のLANパケットによる復帰時にイベント0x1305を送信する。
// 2014/12/10 半田  CQ.507291対応 SNMPパケット受信の識別を追加する。
// 2015/12/18 Sky-Araki CQ520828 Lot26対応.
// 2017/08/24 Sky佐藤 S800省電力MFPアプリ対応.
// 2017/10/11 Sky佐藤 S800省電力Lot26対応.
// ================================================================================================.
static STATUS checkFTaikiWakeupFactor(
	Bool Lot26HwType,
	Bool USBHostConnect		)
{
	struct pmu_wake_info st_Info = {};
	int32_t si_Ret = 0;
	int32_t si_Fd = 0;

	/* PMUドライバから起床情報取得 */
	si_Fd = SYSF_open(THRD_DeviceFile_Pmu, O_RDWR, 0);
	if (0 > si_Fd) {
		printf( "(%010u)(BSP) [%s] open failed. file=%s\n", SYSF_tickGet() , __func__, THRD_DeviceFile_Pmu);
		return ERROR;
	}
	si_Ret = SYSF_ioctl(si_Fd, THRD_PMU_IOCSGETWAKEINFO, (intptr_t)&st_Info);
	if (0 != si_Ret) {
		printf( "(%010u)(BSP) [%s] ioctl failed. cmd=%d\n", SYSF_tickGet() , __func__, THRD_PMU_IOCSGETWAKEINFO);
		SYSF_close(si_Fd);
		return ERROR;
	}
	THRC_PM_LOG("[wake info]:0x%hhx\n", st_Info.flags);
	/* 起床状態の処理 */
	if (st_Info.flags & THRD_PMU_WAKE_INFO_USB) {
		THRC_PM_LOG("usb.\n");
		pmuEvent.add(0x1305);		/* USB起床 */
	} else if (st_Info.flags & THRD_PMU_WAKE_INFO_PROXY) {
		if (st_Info.flags & THRD_PMU_WAKE_INFO_SNMP2) {
			THRC_PM_LOG("snmp2.\n");
			pmuEvent.add(0x1308);	/* SNMPⅡ受信 */
		} else {
			THRC_PM_LOG("proxy.\n");
			pmuEvent.add(0x1305);	/* 代理応答起床 */
		}
	} else if (st_Info.flags & THRD_PMU_WAKE_INFO_WOL) {
		THRC_PM_LOG("wol.\n");
		pmuEvent.add(0x1264);	/* WOL起床 */
	}
	SYSF_close(si_Fd);
	return OK;
}
// ================================================================================================.
//! <!--機能--> 電源管理 制御部 　スリープ前にＪＯＢが積まれていないかをチェックする.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
// 2015/03/18 荒木  CQ.510572.
// ================================================================================================.
Bool THRC_PowerManager::check_ActiveJobList( void )
{
    APIC_ActiveJobList      *pc_ActiveJobList = APIC_ActiveJobList::newInstance();
    LIBC_List<APIC_Job>     c_List;
 	Bool bRet= False;
 	 
    //-------------------------------.
    // アクティブリストがある場合.
    //-------------------------------.
    if( pc_ActiveJobList->getList(TYPE_ActiveJobListPrint,1,TYPD_MaxActiveJob,&c_List) >= 1 ) {
		bRet = True;        
    }
    
    //-------------------------------.
    // アクティブリストがない場合.  
    //-------------------------------.
    else {
        // 何もしない.
    }

    return bRet;
}

// ================================================================================================.
//! <!--機能--> 電源管理 制御部 　起動時間短縮 電源OFF移行制御完了チェック.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! 主電源OFF時の省電力移行制御(タスク)未完了チェックAPI.
//! \n.
//! <!--履歴-->.
// 2016/10/31 sky_ishiga [MinervaMLK][起動時間短縮] スレッド省電力制御の並列制御対応.
// 2017/02/13 Sky-Araki [起動時間短縮] 省電力時に主電源OFFするとフリーズ.
// 2017/10/18 Sky-ishiga [Eagle] [起動時間短縮(再設計)] インターフェース見直し(固有名称(電源SubCPU)を削除).
// 2018/11/05 Sky-yoshimura [AR1062169] 新詳細コード削除対応. [IT5_4.2][AR1038475]からの横展開
// 2018/12/14 Sky-佐藤(高) [AR1068104] USBメモリの挿抜を繰り返すと誤認識する問題対応.
// 2019/11/20 山口 OP_BTS-10123 無線LANデバイスのエラー発生時にリセット処理を行う.
// ================================================================================================.
#define CNTR_WAIT_SEQDONE	500		// 10*500ms 待つ.
void THRC_PowerManager::check_PowerOffSequenceComplete()
{
	// 10ms*500=5秒ウェイトする.
	int32_t WaitCntr;
	int32_t si_SdcExist;

	for ( WaitCntr= 0; WaitCntr< CNTR_WAIT_SEQDONE; WaitCntr++ ) {
		if( THRE_PM_DeviceStatus_None == e_PM_DeviceStatus	&&
			False== s_DelayedPowChgSeq	&&
			0 != access("/dev/sdc", F_OK )					) {
			break;
		}
		SYSF_taskDelay(SYSF_sysClkRateGet() * 10 / 1000);	// 10ms wait.
	}
	si_SdcExist = access("/dev/sdc", F_OK );
	
	// タイムアウト発生時は、エラー出力し、即電源断する.
	if ( (WaitCntr >= CNTR_WAIT_SEQDONE) ||
		 (bl_Wlan_reset_disable_SuperWarp == True) ){
		printf( "(%010u)(BSP) %s: ERROR TimeOut! PowerSaveSequence is not Complete DeviceStatus=[0x%08x] /dev/sdc=[%d] WlanRset[%d]\n", SYSF_tickGet() , __FUNCTION__, e_PM_DeviceStatus, si_SdcExist ,bl_Wlan_reset_disable_SuperWarp);
		SYSF_taskDelay(SYSF_sysClkRateGet() * 1000 / 1000);	// 1sec wait.

		// 電源断.
		THRF_setPowerOffDelay( False );
	}
	THRC_PM_LOG("### %s(L%d)/Cntr=%d Limit=%d DevS=%d DelayedPowChgSeq=%d /dev/sdc=%d ###\n",
		__FUNCTION__,
		__LINE__,
		WaitCntr,
		CNTR_WAIT_SEQDONE,
		e_PM_DeviceStatus,
		s_DelayedPowChgSeq,
		si_SdcExist);

	return;
}

// ================================================================================================.
//! <!--機能--> 電源管理 制御部 　SuperWarpSnapShot削除処理.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
// 2016/11/22 yamada 	新規作成　起動時間短縮対応.
// 2018/05/25 Sky酒井(純） [Eagle][AR1046576]システムコールを使用した処理方法に変更.
// 2019/03/05 Sky大喜	[EagleH]静的解析対応.
// ================================================================================================.
#define THRD_SUPERWARP_DEVICEPATH	"/dev/kmsda10"
void THRC_PowerManager::del_SuperWarpSnapShot()
{
	// ddコマンドよりシステムコールで書き込みを行う方が早いため.
	// システムコールでSnapshotの削除を行う.

	//kmsda22は、SuperWarpのSnapShot保存場所.
	//system("dd if=/dev/zero of=/dev/kmsda22 bs=512 seek=0 count=2");

	int si_Fd = -1;									
	ssize_t size = 0;
	static char sc_ClearBuffer[512 * 2] = { 0 };	// ２セクタ分を確保.

	// デバイスオープン.
	si_Fd = SYSF_devopen( THRD_SUPERWARP_DEVICEPATH, O_RDWR, 0666 );
	if( -1 == si_Fd ){
		printf( "%s Cannot create file file=%d\n", __FUNCTION__, errno );
		return;
	}
	
	size = SYSF_write(si_Fd, sc_ClearBuffer, sizeof(sc_ClearBuffer));
	if (sizeof(sc_ClearBuffer) != size ) {
		printf("Cannot write file file=%s Result=%zd\n", THRD_SUPERWARP_DEVICEPATH, size);
	}
	
	// デバイスクローズ.
	SYSF_devclose(si_Fd);
	
	sync();
	sync();
	sync();
	return;
}

#include "system/sys/sysg/SYSC_SwapControl.h"	// EagleH_制御構成変更_No.49 swap有効化対応(swap有効化遅延実行対応)↓.
extern Bool bl_isRebooting;			// 2020/11/27 OP_BTS-27365 山口 詳細コード対応_09C1.
// ================================================================================================.
//! <!--機能--> 電源管理 制御部 　起動時間短縮用SuperWarpSnapShot取得API.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! 主電源OFF時のSnapShot取得用のAPI。必要な処理をここにまとめている。
//! \n.
//! <!--履歴-->.
// 2016/05/30 tadamasa 	新規作成　起動時間短縮対応.
// 2016/11/09 sky_ishiga [MinervaMLK][起動時間短縮] SuperWarp Snapshot取得対応.
// 2016/11/21 sky_ishiga [MinervaMLK][起動時間短縮] 電源SubCpuタイマー設定対応.
// 2017/01/05 sky_shasegawa [MinervaMLK][起動時間短縮] SuperWarp起動時のログを追加.
// 2017/02/10 sky_hatono [MinervaMLK][起動時間短縮] AR1012820 メモリパーティション管理領域の保存/復帰.
// 2017/02/13 Sky-Araki [起動時間短縮] 省電力時に主電源OFFするとフリーズ.
// 2017/03/09 Sky-ishiga [AR1013403] 電源の移行が不可の場合は、通常起動/通常Warpさせる.
// 2017/05/12 Sky-nakanisi [AR1021721] 【IT5_4.1】 【開発提起】【RQ-643】SuperWarp可否判定ログの強化対応.
// 2017/05/22 Sky-ishiga [AR1023389] トラブル、エラーシグナル検出時のSuperWarp不可対応.
// 2017/06/06 Sky-ishiga [AR1023490] 電源SubCPUにてタイムアウト発生時はリブートし、通常Warp起動させる.
// 2017/06/27 Sky-ishiga [AR1024990] Snapshot取得前に1秒Wait. -> Eagle で削除.
// 2017/10/18 Sky-ishiga [Eagle] [起動時間短縮(再設計)] インターフェース見直し(固有名称(電源SubCPU)を削除).
// 2017/12/19 Sky-Araki 起動音制御変更.
// 2017/05/25 Sky酒井(純） [Eagle][AR1046576] SuperWarp起動後のSnapshot削除を行う対応.（暫定対応）
// 2017/06/27 Sky酒井(純） [Eagle][AR1050555] SuperWarp起動後、パネルの入力デバイスを有効にする.
// 2017/06/29 Sky酒井(純） [Eagle][AR1051521] AR1046576に対する暫定対応を解除する.
// 2018/07/19 Sky酒井(純） [Eagle][AR1046564] SuperWarp起動時にファイルメモリのメモリプール初期化を行う.
// 2018/09/10 Sky酒井(純)  [Eagle][AR1055313] メモリパーティションのバックアップ、リストアの関数名を変更.
// 2018/09/20 Sky酒井(純） [Eagle][AR1062953] メモリプール初期化処理追加(SuperWarp向け) AR1046564の対応も移動.	
// 2018/10/11 Sky-ohshima [Eagle][AR1063557] 省電力中のウィルススキャン継続、電源OFF時にレジュームスキャン用ファイナライズ実施.
// 2019/01/28 Sky          [EagleH]制御構成変更No.60 自己診断結果収集機能API追加対応.
// 2019/04/04 Sky[IT6_1.0MR2][AR1074753] 電源OFFタイマータスクの常駐化.
// ================================================================================================.
void THRC_PowerManager::req_SuperWarpSnapShot()
{
	int32_t si_SnapShotResult = 0;
	int32_t si_WarpError = 0;
	uint32_t ui32_dramtblnum = 0;
	uint32_t ui32_dramtblmax = 0;
	uint64_t u64_dt = 0;
	uchar   uc_ngPoint = 0;
	STATUS	ret = ERROR;
	//++ 2019/04/24 Sky		【EagleH】Warning除去対応.
	int32_t		si_Result = 0;
	//-- 2019/04/24 Sky		【EagleH】Warning除去対応.
	
	THRC_PM_LOG( "[PowOffPerf][%s] START : %lu\n", __FUNCTION__, SYSF_tickGet() ); 
	//++ 2019/04/24 Sky		【EagleH】Warning除去対応.
	si_Result = system("sync");//キャッシュクリア.
	if (si_Result != 0){
		printf("(%010u)(BSP) ## %s[%d] %s\n", SYSF_tickGet(), __FILE__, __LINE__, __FUNCTION__);
	}
	si_Result = system("sync");//キャッシュクリア.
	if (si_Result != 0){
		printf("(%010u)(BSP) ## %s[%d] %s\n", SYSF_tickGet(), __FILE__, __LINE__, __FUNCTION__);
	}
	si_Result = system("sync");//キャッシュクリア.
	if (si_Result != 0){
		printf("(%010u)(BSP) ## %s[%d] %s\n", SYSF_tickGet(), __FILE__, __LINE__, __FUNCTION__);
	}
	si_Result = system("sysctl -w vm.drop_caches=3");//キャッシュクリア.
	if (si_Result != 0){
		printf("(%010u)(BSP) ## %s[%d] %s\n", SYSF_tickGet(), __FILE__, __LINE__, __FUNCTION__);
	}
	//-- 2019/04/24 Sky		【EagleH】Warning除去対応.
	//system("free");//キャッシュの状態チェック用API.
	THRC_PM_LOG( "[PowOffPerf][%s] drop_caches END : %lu\n", __FUNCTION__, SYSF_tickGet() ); 

	if( 0 != si_taskID_setPowerOffTimer ) {
		// SnapShot取得前にタイマー設定要求.
		ushort us_Msg = TM_SetPowerOffTimerSS;
		SYSF_msgQSend( msgQID_setPowerOffTimer, (char*)&us_Msg, sizeof(us_Msg), WAIT_FOREVER, MSG_PRI_NORMAL );
		// タイマー設定要求が完了まで、ポーリングする.
		while( bl_PowerOffTimerUpdate ){
			SYSF_taskDelay(SYSF_sysClkRateGet() * 1 / 500);	// 1ms wait.
		}
	} else {
		printf( "(%010u)(BSP) req_SuperWarpSnapShot: si_taskID_setPowerOffTimer == 0\n", SYSF_tickGet() );	// タスク未起動.
	}

	// SnapShot取得前にメモリパーティション管理領域のバックアップ.
	THRC_PM_LOG( "[PowOffPerf][%s] MemPartAreaBackup START : %lu", __FUNCTION__, SYSF_tickGet() ); 
	ret = MemPartAreaBackup();
	THRC_PM_LOG( "[PowOffPerf][%s] MemPartAreaBackup END : %lu", __FUNCTION__, SYSF_tickGet() ); 

	// ASIC777の電源がOFFであればONにする.
	if( THRC_PowerStateID_LOW== pc_powerDevice->GetPowerState(THRC_PowerDeviceID_PEXPD) ) {
		THRC_PM_LOG("### %s(L%d)/Change Asic777 status LOW -> NORMAL to prevent freezing ###\n",__FUNCTION__,__LINE__);
		pc_powerDevice->SwitchPowerState(THRC_PowerDeviceID_PEXPD,THRC_PowerStateID_NORMAL);
	}
	
	// パネルの電源がOFFであればONにする  AR1016416 Sky-Araki SuperWarp起動後に黒画面表示.
	if( THRC_PowerStateID_LOW== pc_powerDevice->GetPowerState(THRC_PowerDeviceID_PANEL_POWER) ) {
		uint32_t TicksS= SYSF_tickGet();
		pc_powerDevice->SwitchPowerState(THRC_PowerDeviceID_PANEL_POWER,THRC_PowerStateID_NORMAL);		
		THRC_PM_LOG("### %s(L%d)/Change PanelPower LOW-> NORMAL to prevent black draw/ETime=%dms ###\n",
			__FUNCTION__,
			__LINE__,
			(SYSF_tickGet()-TicksS)* 5	);
	}
#ifdef DEF_64BIT_PF		// EagleH_制御構成変更_No.49 swap有効化対応.	
	if( ret == OK ){
		ret = SYSF_checkSwapoutLimit();	// swapアウト状況を確認してスナップショット取得可能ならOKを返す.
		THRC_PM_LOG( "%s: checkSwapoutLimit result[%d]",__FUNCTION__, ret );
	}
#endif	// DEF_64BIT_PF

	if ( ret == OK ) {
// 2020/07/14 伊藤史. OP_BTS-21031 起動時割込抑制.
		// 前扉,副電源SWを禁止.
		THRL_SetThLog( 745, THRE_DebugLogType_ThCtrLog, 0, True, 0 );
		pc_powerDevice->SwitchSubPowerSWState(THRC_PowerStateID_NORMAL);		
		pc_powerDevice->SwitchPowerState(THRC_PowerDeviceID_SET_INTP4,THRC_PowerStateID_NORMAL);		
		pc_powerDevice->SwitchPowerState(THRC_PowerDeviceID_SET_INTP7,THRC_PowerStateID_NORMAL);		

		THRC_PM_LOG( "[PowOffPerf][%s] getSnapShot START : %lu\n", __FUNCTION__, SYSF_tickGet() ); 
		si_SnapShotResult = getSnapShot_SuperWarp();	//SnapShotの取得.
		si_WarpError = getWarpError();					//SnapShot取得結果.

		THRC_PM_LOG( "%s: getSnapShot result[%d] error[%d]",__FUNCTION__, si_SnapShotResult, si_WarpError );
//2019/10/08 Sky 【自動測定】起動イベントの通知 start.
		/* Warp起動 完了を通知 */
		THRF_setEventRecord(THRD_EventRecord_WarpComplete);
//2019/10/08 Sky 【自動測定】起動イベントの通知 end.
	}
	// 2020/03/06 山口 OP-BTS_21240 ロックパスワード中にSuperWarp起動でD391発生.
	unlockStorage( True );
	
	ca72wdt_ext_refresh( 0x4002, 300, 0x0000, "SuperWarp End.\n" );	
	     /* /sys/kernel/warp_kobj/dramtbl_num  snapshotの取得サイズ */
	   	FILE* fp_num = fopen("/sys/kernel/warp_kobj/dramtbl_num", "rt");
	
		char buf_num[12] = {0};
   	    assert(fp_num != NULL);
        fread(buf_num, sizeof(buf_num[0]), (sizeof(buf_num) / sizeof(buf_num[0])) - 1, fp_num);
   	    fclose(fp_num);
		u64_dt = static_cast<uint64_t>(atoi(buf_num)/16);
		ui32_dramtblnum = static_cast<uint32_t>( u64_dt);
	
	     /* /sys/kernel/warp_kobj/dramtbl_max  snapshotのMAXサイズ */
	   	FILE* fp_max = fopen("/sys/kernel/warp_kobj/dramtbl_max", "rt");
	
		char buf_max[12] = {0};
   	    assert(fp_max != NULL);
        fread(buf_max, sizeof(buf_max[0]), (sizeof(buf_max) / sizeof(buf_max[0])) - 1, fp_max);
   	    fclose(fp_max);
		u64_dt = static_cast<uint64_t>(atoi(buf_max)/16);
		ui32_dramtblmax = static_cast<uint32_t>( u64_dt);
	
		/* /sys/kernel/warp_kobj/dramtbl_max  ngPoint */
	   	FILE* fp_ng = fopen("/sys/kernel/warp_kobj/ngPoint", "rt");
	
		char buf_ng[12] = {0};
   	    assert(fp_ng != NULL);
        fread(buf_ng, sizeof(buf_ng[0]), (sizeof(buf_ng) / sizeof(buf_ng[0])) - 1, fp_ng);
   	    fclose(fp_ng);
		uc_ngPoint = static_cast<uchar>(atoi(buf_ng));
	
	// スナップショット取得エラー.
	if( SYSE_Snapshot_Failure == si_SnapShotResult || ret == ERROR ) {
		printf( "(%010u)(BSP) %s: MemPartAreaBackup() Result=[%d]\n", SYSF_tickGet() , __FUNCTION__, ret );
		printf( "(%010u)(BSP) %s: getSnapShot_SuperWarp() Error!!! \n", SYSF_tickGet() , __FUNCTION__ );

		// 2021/04/13 ft_kondo OP_BTS-31367 新詳細コード登録不要のため削除. 
		// 新詳細コードに残していた情報は垂れ流しログに残す. 
		printf( "(%010u)(BSP) %s: WarpError[%d] ngPoint[%d] num[%d] max[%d]\n", SYSF_tickGet() , __FUNCTION__, si_WarpError, uc_ngPoint, ui32_dramtblnum, ui32_dramtblmax );
		// 新詳細コード登録追加.
		// OP_BTS-26062 山口 uc_ngPointが3の場合は0x00Aの詳細コードは登録しない.
//		if(uc_ngPoint != 3){
//			uint64_t ull_NewDetailCode = ((((uint64_t)0x22              << 56) & 0xff00000000000000)	// モジュール番号.
//									   |  (((uint64_t)0x00A             << 44) & 0x00fff00000000000)	// 通し番号.
//									   |  (((uint64_t)si_WarpError      << 36) & 0x00000ff000000000)	// SnapShot取得エラーコード.
//									   |  (((uint64_t)uc_ngPoint        << 32) & 0x0000000f00000000)	// ngPoint.		
//									   |  (((uint64_t)ui32_dramtblnum     << 16) & 0x00000000ffff0000)	// snapshotの取得サイズ.
//									   |  (((uint64_t)ui32_dramtblmax     <<  0) & 0x000000000000ffff));	// snapshotのMAXサイズ.
//			printf( "(%010u)(BSP) ## %s us_DetailCode=0, ull_NewDetailCode=%#llx\n", SYSF_tickGet() , __FUNCTION__, ull_NewDetailCode);
//			APIC_setNewDetailCode( 0, ull_NewDetailCode );		// 詳細コード登録.
//		}
		// 次回電源ON時にレジュームスキャン可能とするため、ウィルススキャン終了処理をコール.
		APIC_VirusScan::newInstance()->finalize();
		//SnapShotを削除.
		del_SuperWarpSnapShot();

		SYSF_taskDelay(SYSF_sysClkRateGet() * 200 / 1000);	// 200ms wait(これが無いとログ出力前に電源が落ちるため).

		//電源断を行う.
		THRF_setPowerOffDelay( False );
	} else if ( SYSE_Snapshot_Read == si_SnapShotResult ) {
		printf("\nSuperWarp起動だった。\n" );
//2019/10/08 Sky 【自動測定】起動イベントの通知 start.
		/* BSP復帰処理 開始を通知 */
		THRF_setEventRecord(THRD_EventRecord_BSPReturnStart);
//2019/10/08 Sky 【自動測定】起動イベントの通知 end.
		
		rtc_Time_BackUpStartUp();
		unsigned char uc_timeBackupFaxRegion[7];
		unsigned char uc_timebuf_FF[7];
		memset( uc_timeBackupFaxRegion, 0, sizeof(uc_timeBackupFaxRegion) );
		memset( uc_timebuf_FF, 0xFF, sizeof(uc_timebuf_FF) );
		if ( 0 == THRG_getBspThrRegion( 0x4F, (void*)&uc_timeBackupFaxRegion, sizeof(uc_timeBackupFaxRegion) ) ) {
			if(0 != memcmp(uc_timeBackupFaxRegion , uc_timebuf_FF , sizeof(uc_timeBackupFaxRegion))){
				THRG_setBspThrRegion( 0x4F, uc_timebuf_FF, sizeof(uc_timebuf_FF) );
			}
			
		}
#ifdef DEF_64BIT_PF		// EagleH_制御構成変更_No.49 swap有効化対応(swap有効化遅延実行対応).
		SYSC_SwapDelayedStart();	// swap遅延スタート処理.
#endif	// DEF_64BIT_PF

#if defined(TORNADO) && !defined(FUM_LINUX)
		// パネルの入力デバイス設定を有効にする.
		sysBspIoctlPanelSio(NULL, EXDB_Panelinit);
#endif /* #if defined(TORNADO) && !defined(FUM_LINUX) */

		// メモリプール初期化処理(SuperWarp向け). 
		IMGC_ImageMemoryPoolImpColorFileMemoryInstance& comMemoryManager = IMGC_ImageMemoryPoolImpColorFileMemorySingletonIf::newInstance();
		comMemoryManager.init_MemoryPoolFileMemoryForSuperWarp();
		
		// SuperWarp起動成功したので、起動要因をOK(0)で初期化する.
		gl_ucWarpNgType = 0;	// 2017/10/31 M.Adachi [EagleL]uboot不要処理削除対応.
		
		// comment 2018/09/10 SCD2-太田 [Eagle H][制御構成変更No.50]待ち時間測定の自動化.
		PFMG_JudgeWaitTimePerformAutoLoggingStart();
		PFMG_WaitTimePerformLogging(PFMD_LogFormatNormal, PFMD_LogTypeSuperWarpBooted, PFMD_LogStatusBooted);

		// SnapShot展開後にメモリパーティション管理領域のリストア.
		THRC_PM_LOG( "[PowOffPerf][%s] MemPartAreaRestore START : %lu", __FUNCTION__, SYSF_tickGet() ); 
		ret = MemPartAreaRestore();
		THRC_PM_LOG( "[PowOffPerf][%s] MemPartAreaRestore END : %lu", __FUNCTION__, SYSF_tickGet() ); 
		if ( ret == ERROR ) {
			printf( "(%010u)(BSP) %s: MemPartAreaRestore() Error!!! Result=[%d]\n", SYSF_tickGet() , __FUNCTION__, ret );
		}
	}

	if( SYSE_Snapshot_Write == si_SnapShotResult ) {
		// スナップショット取得.
		//Jobが存在するかどうかチェックする.
		Bool bl_ActiveJob = check_ActiveJobList();
		Bool bl_isDeepSleepDelayExist = False;
		Bool bl_isPowerSaveDelayExist = False;
		Bool bl_isDataAccessExist = False;
		// トラブルの発生チェック.
		Bool bl_isPrinterFatal = APIC_PrinterWarning::newInstance()->isOccurFatal();
		Bool bl_isScannerFatal = APIC_ScannerWarning::newInstance()->isOccurFatal();
		Bool bl_isSystemFatal = APIC_SystemStatus::newInstance()->isOccurFatal();
		// mfp以外のアプリでErrorSignal検出.
		LIBS_ApiSignalInfo* pst_ApiSignalInfo = LIBF_GetApiSignalInfoAdr();

		//PwrがSuperWarp不可の状態かチェック.
		APIC_Power* pc_Power = APIC_Power::newInstance();
		if ( NULL != pc_Power ) {
			bl_isDeepSleepDelayExist = pc_Power->isDeepSleepDelayExist();	// ディープスリープモード移行抑止要求中.
			bl_isPowerSaveDelayExist = pc_Power->isPowerSaveDelayExist();	// 節電モード移行遅延要求中.
			bl_isDataAccessExist = pc_Power->isDataAccessExist();			// データアクセス要求中.
		}
		if ( ( True == bl_ActiveJob )				// Jobが存在する.
		  || ( True == bl_isDeepSleepDelayExist )	// ディープスリープモード移行抑止要求中.
		  || ( True == bl_isPowerSaveDelayExist ) 	// 節電モード移行遅延要求中.
		  || ( True == bl_isDataAccessExist )		// データアクセス要求中.
		  || ( True == bl_isPrinterFatal )			// Printerトラブル.
		  || ( True == bl_isScannerFatal )			// Scannerトラブル.
		  || ( True == bl_isSystemFatal )			// Systemトラブル.
		  || ( (NULL != pst_ApiSignalInfo) && (0 != pst_ApiSignalInfo->si_signo) ) ) {	// mfp以外のアプリでErrorSignal検出.
			printf( "(%010u)(BSP) [%s] Delete SuperWarp Snapshot! \n", SYSF_tickGet() , __FUNCTION__ );
			printf( "(%010u)(BSP) \t\tJob[%d] DeepSleep[%d] PowerSave[%d] DataAccess[%d]\n", SYSF_tickGet() , bl_ActiveJob, bl_isDeepSleepDelayExist, bl_isPowerSaveDelayExist, bl_isDataAccessExist );
			printf( "(%010u)(BSP) \t\tPrinterFatal[%d] ScannerFatal[%d] SystemFatal[%d]\n", SYSF_tickGet() , bl_isPrinterFatal, bl_isScannerFatal, bl_isSystemFatal );
			if ( NULL != pst_ApiSignalInfo ) {
				printf( "(%010u)(BSP) \t\tHandle Error Signal Pid[%d] SigNo[%d]\n", SYSF_tickGet() , pst_ApiSignalInfo->si_ApiID, pst_ApiSignalInfo->si_signo);
			}
			// 次回電源ON時にレジュームスキャン可能とするため、ウィルススキャン終了処理をコール.
			APIC_VirusScan::newInstance()->finalize();
			// SnapShotを削除する(次回SuperWarpさせない).
			del_SuperWarpSnapShot();
		}
		// 2017/12/27 SCD2-Sky_Shindo [WP.83-84] SuperWarp対応.
		else if ( APIC_SystemSetting::newInstance()->getSecureBootMode() ) {
			printf( "(%010u)(BSP) [%s] Delete SuperWarp Snapshot! \n\t\tSecureBootModeOn\n", SYSF_tickGet() , __FUNCTION__);
			// 次回電源ON時にレジュームスキャン可能とするため、ウィルススキャン終了処理をコール.
			APIC_VirusScan::newInstance()->finalize();
			// SnapShotを削除する(次回SuperWarpさせない).
			del_SuperWarpSnapShot();
		}

		//電源断を行う.
		THRF_setPowerOffDelay( False );

		volatile int32_t boot = 0;	// 最適化防止用カウント.
		while(true) {
			// 無限Loopで電源断を待つ.
			boot++;
			if (1==boot) {
				printf( "(%010u)(BSP) PowerOff Command execute!!!\n", SYSF_tickGet() );
			}
			SYSF_taskDelay(SYSF_sysClkRateGet() * 1 / 1000);	// 1ms wait.
		}
	}

	// 17/10/31 Sky-Araki OneShotモードを適用したため、SnapShotを削除するのを廃止.
//	del_SuperWarpSnapShot();

	//電源OFFタイマーによる電源断をチェックする.
	// 2018/11/27 Sky-ishiga PS-CPU側が主電源ON時にリセットされる為、ログ取得は不要 .
	// 2018/12/17 Sky-ishiga 通電OFF要因をログ出力する対応.
	uint32_t u32_PowerOffFactor = THRF_getPowerOffFactor();
	THRF_printPowerOffFactor(u32_PowerOffFactor);
	if( 0 != u32_PowerOffFactor ) {
		THRF_clrPowerOffFactor();
	}
//	// タイマーログをチェックする.
//	if ( u32_PowerOffTimerLog & THRD_POWEROFFTIMER_LOG_MASK ) {
//		printf( "(%010u)(BSP) PowerOffTimer reset occurred[%d] => Reboot ... \n", SYSF_tickGet() , u32_PowerOffTimerLog);
//		// 電源OFFタイマーログクリア.
//		THRF_clrPowerOffTimerLog();
//		// リブート.
//		APIC_SystemServiceFunction::newInstance()->reboot();
//		while( 1 ){	// Reboot無限ループ待ち.
//			SYSF_taskDelay(1);
//		}
//	}
	
// 2019.01.28 [EagleH]制御構成変更No.60 自己診断結果収集機能API追加対応 ->.
	// ストレージタイプとメーカ名・シリアルNoをターミナル出力する.
	THRG_DspStrgMakerNameAndSerialNo();
// 2019.01.28 [EagleH]制御構成変更No.60 自己診断結果収集機能API追加対応 <-.

	//パワーへ復帰イベント送信.
	pmuEvent.add( 0xFFF );
	bl_isRebooting = False;		// FAX詳細コード登録抑止のフラグを落とす. // 2020/11/27 OP_BTS-27365 山口 詳細コード対応_09C1.
	SendPmuEvent();
}

// ================================================================================================.
//! <!--機能--> 電源管理 制御部 　Deep Sleep移行要求対応.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
// 2010/07/12 内野 Minerva初版.
// 2011/02/24 kurachi Minerva/Zeus CQ:336567対策（STR実行を専用子プロセスにて行うように修正）.
// 2011/03/25 河村 CQ340449 DeepSleep復帰NGリカバリ対応.
// 2011/04/12 Sky越野 CQ340449のSoC化対応.
// 2011/10/21 佐々木 DeepSleep、PCプリント対応、2nd暫定対応削除、USBHostドライバ制御変更対応.
// 2011/12/07 佐々木 DeepSleep復帰時、USBHostドライバの組み込み後に1sの待ち処理を追加(wakeupへアクセスできない対策).
// 2012/02/03 佐々木 DeepSleep中断シーケンス - NIC_APIによる"D3Cold移行要求"でERRORが返る場合はSTRせず復帰する対応.
// 13/02/28 Sky 機種共通化対応 THRC_PowerManagerSoC.cppから移植.
// 2012/12/04 半田   ZeusS対応 (DeepSleepを行わない)
// 2013/09/21 Sky藤井 ディープスリープ省電力パフォーマンス対応 S73-BSP_DD-28 4.2.
// 2014/04/10 toda Helios対応 STR突入/復帰時に、必要な割り込み設定を追加.
//                            電源サブCPUにPMUドライバ経由で復帰要因を問い合わせる処理を追加.
// 2014/04/14 Sky藤井 ErP(リテンション)モード対応 S73-BSP_DD-29
// 2014/06/13 河村 0.5W対応(拡張ディープスリープ).
// 2014/10/27 河村 CQ.505932 / CQ.505934 拡張ディープスリープ復帰時に原稿セットや前ドア開を検出してしまう問題の対策.
// 2014/11/13 半田 CQ.507316 Erp状態 無線LAN復帰対応.
// 2015/2/18 山口 CQ.509733 req_DeepSleepShift中にPCプリント受信すると印刷されない、処理時間短縮のため電サブへの割り込みを並列化.
// 2015/03/18 荒木	CQ.510572. スリープ移行中にジョブ受信しても印刷しない.
// 2015/04/30 半田 Emu72無線LAN対応 USBのアンロード順序を変更する.
// 2015/mm/dd Sky_Araki CQ518419 副電源オフでスリープしない不具合修正.
// 2015/12/18 Sky-Araki CQ520828 Lot26対応.
// 2016/05/30 tadamasa 	起動時間短縮対応.
// 2016/10/31 sky_ishiga [MinervaMLK][起動時間短縮] スレッド省電力制御の並列制御対応.
// 2016/10/31 sky_ishiga [MinervaMLK][起動時間短縮] 電源OFF移行(Snapshot取得)時の不要制御対応.
// 2016/11/29 yamada RQ-643 主電源OFF時異常シーケンス新詳細コード対応.
// 2017/01/26 Matsuoka SuperWarp後にネットワーク使用できなくなる問題対応.
// 2017/4/26 Sky-NAKANISHI AR1019633 主電源OFF後に前ドア開閉するとMFPパネルが起動する問題の対応.
// 2017/05/08 Sky-Araki AR1021331 xhci-hcd Timeout.
// 2017/05/02 Sky井柳 [IT5_4.1][AR1021451]主電源スイッチ割り込み(OFF検出)時のイベント通知追加(電源部にOFFを検出したことを知らせる).
// 2017/06/06 sky_ishiga [IT5_4.1][AR1023492] ERP移行直前に主電源スイッチと別復帰要因が同時に入った場合に主電源OFFが処理されない.
// 2017/10/11 Sky佐藤 S800省電力Lot26対応.
// 2017/12/26 波多野 IT6_SuperWarp標準化対応(コンパイルスイッチ<DEF_MINERVAMLK_CUS>削除).
// 2018/10/11 Sky-ohshima [Eagle][AR1063557] 省電力中のウィルススキャン継続、電源OFF時にレジュームスキャン用ファイナライズ実施.
// 2018/11/05 Sky-yoshimura [AR1062169] 新詳細コード削除対応. [IT5_4.2][AR1038475]からの横展開
// 2019/02/19 Sky [IT6_1.0MR2][AR1073329]省電力移行直前に電源SWOFFすると次回SuperWarpしない.
// 2019/11/09 BSP-matsunaga Kernelログレベル低下復旧対策
// 2020/02/07 Sky OP_BTS-19517 DeepSleep前のsync対応.
// 2021/10/15 Sky MinervaSSBK 制御構成変更_No.40 リブート回数を数えるカウンタを追加する対応.
// 24/03/22 FPT		Fix build error GccVersionUp.
// ================================================================================================.

/* 2014/11/13 M.Handa CQ.507316対応 start */
static int32_t si_Fd = -1;			/* PMUファイルハンドラ. 2014/11/13 CQ.504087. */
/* 2014/11/13 M.Handa CQ.507316対応 end */
#if 1 /* 暫定対応:Sleep2復帰中にQSPIのアクセスでR4が固まる */
bool b_in_exec_str = false;
#endif
void THRC_PowerManager::req_DeepSleepShift( THRE_ReqShiftType e_ReqShiftType, ushort us_level )/* ←S73-BSP_DD-29 */
{
#if !defined(DEF_ARMAGAXP)
#ifdef TORNADO			// 実機で有効.
	STATUS si_Ret = ERROR;					//戻り値用.
#endif // #ifdef TORNADO. 
#endif
	int32_t Wait = 0;
	static Bool b_isErr_mioChangeModeToSleep = False;		// 2019/12/03 河村 -  OP_BTS-15604.
	static Bool b_isErr_mioChangeModeToNormal = False;		// 2019/12/03 河村 -  OP_BTS-15604.

	e_ReqShiftType_tmp = e_ReqShiftType;
	
	// 2019/05/20 BSP 山口 AR1076953 req_DeepSleepShiftの開始/終了にログを追加.
	printf( "(%010u)(BSP) : req_DeepSleepShift() Start\n", SYSF_tickGet());
	APIC_Power* pc_ApiPower = APIC_Power::newInstance();
	
	THRC_PM_LOG( "[PowOffPerf][%s] Start : %lu\n", __FUNCTION__, SYSF_tickGet() ); 
	Bool bLot26Active= False;	
	//++ 2019/04/24 Sky		【EagleH】Warning除去対応.
	int32_t		si_Result = 0;
	//-- 2019/04/24 Sky		【EagleH】Warning除去対応.
	// 2020/12/25 山口 OP-BTS_30056 FAXジョブあり状態でDeepSleep中に再起動.
	void* pv_addr = NULL;
	APIC_ActiveJobList *pc_ActiveJobList   = APIC_ActiveJobList::newInstance();
	Bool bl_setPowerDownRegion = pc_ActiveJobList->isJobIdle();

	//既存処理に影響を修正しないようにする為、enumをTrue、Falseに丸め込む.
	Bool b_ErP = ((e_ReqShiftType > 0) ? True : False);
	// 2016/7/04 山口 CQ.532124 Erp移行途中に割り込み発生でリブートする.
	uint32_t u32_INT_SOURCE = 0;
//	ushort us_setNVData = CFA14_DetailCode0x0AA8;	// 2012.10.17 内野 CQ373869.
//	ushort us_setNVClear = 0x0000;	// 2012.10.17 内野 CQ373869.
/* 2014/11/13 M.Handa CQ.507316対応 start */
	int32_t si_Ret2 = 0;
	int32_t irq_disable_flg = 0;	// 2018/12/25 ft_kondo AR1070568調査仕込み. 
	//Provisional avoidance(In case of never linking up ethernet, stall LPPP before system resume)
	static Bool b_EtherLinkUpFlag = False;
	// 2019/07/08 OP_BTS-9953 山口 DeepSleep復帰時間改善.
	char sc_Pass[256] ;
	NVDC_SystemSetting *pc_Instance = NVDC_SystemSetting::newInstance();
	if (NULL == pc_Instance) {
		return ;
	}

	if(THRE_ReqShiftType_PowerOFF == e_ReqShiftType )
	{
		// 次回電源ON時にレジュームスキャン可能とするため、ウィルススキャン終了処理をコール.
		APIC_VirusScan::newInstance()->finalize();
		//電源を即断する.
		THRF_setPowerOffDelay( False );
	}

	// CQ.504087 Erp状態 無線LAN復帰対応.
	// 割り込みを取りこぼさないようにするため、Erp/DeepSleep移行前に.
	// 復帰要因割り込みを禁止にしてから、割り込みマスク設定を行う.
	if( si_Fd < 0 ) {
		si_Fd = SYSF_open( THRD_DeviceFile_Pmu, O_RDWR, 0 );
		if( si_Fd < 0 ) {
			//起動時間短縮処理の場合には主電源OFFをする.
			if(e_ReqShiftType == THRE_ReqShiftType_GetSnapShot){
				printf( "(%010u)(BSP) %s: SYSF_open(%s) Error!!! \n", SYSF_tickGet() , __FUNCTION__, THRD_DeviceFile_Pmu );
				THRF_setPowerOffDelay( False );
			}else{
				THRC_PM_ASSERT("Cannot open file THRC_PowerManager::resumeAfterDeepSleep() THRD_DeviceFile_Pmu", 0,0,0,0,CFA14_DetailCode0x0CEA );
			}
		}
	}
	
	// 17/05/08 Sky-Araki AR1021331 xhci-hcd Timeout.
	// SuperWarp起動であれば、USBロード処理が完了するのを待つ.
	// 2017/08/09 山口 AR1027412 KronosS水平展開(AR1025017) AR1021331の対応のSuperWarp限定の条件を外せばAR1025017と同様になる.
	if( True== bWakeupUSB_Busy ) {		
		logMsg( "(%010u)(BSP) : req_DeepSleepShift() bWakeupUSB_Busy Wait Start\n", SYSF_tickGet(), 0, 0, 0, 0, 0 );
		for( Wait= 0; Wait< PM_TIME_WaitCntr; Wait++ ) {
			if( False== bWakeupUSB_Busy ) break;
			DELAY_TSK(PM_TIME_Watch);
		}//for().
	
		// 保険をかけておく.
		if( Wait>= PM_TIME_WaitCntr ) {
			printf( "(%010u)(BSP) ### %s()/Timeout occured for waiting Wakeup USB ###\n", SYSF_tickGet() ,__FUNCTION__);
		}
		logMsg( "(%010u)(BSP) : req_DeepSleepShift() bWakeupUSB_Busy Wait End\n", SYSF_tickGet(), 0, 0, 0, 0, 0 );
	}

	// AR.1007037 2016/12/08 山口 DeepSleep移行中の電源キー長押し問題.
	// kernelの電源キー押下の割り込み無効.
	si_Ret2 = ioctl( si_Fd, THRD_PMU_IOCSSUBSWINT, 0 );
	if( si_Ret2 < 0 ) {
		setFA14_PmuDriverIoError(0x01,errno);		// 2017/2/21 山口 AR.1013578 詳細コード0CEB調査.
		THRC_PM_ASSERT("pmu ioctl(THRD_PMU_IOCSSUBSWINT) error.", 0,0,0,0,CFA14_DetailCode0x0CEB );
	}

	// 2019/11/09 松永 kernelログレベル低下復旧対応.
	// ログレベルを確認.
	checkSleepLogLevel();
	checkWlanLogLevel();		/* 2020/11/18 山口 OP_BTS-27099 WLANのログレベルが上がりWLANのWarningログ大量発生 */

	if(THRE_ReqShiftType_GetSnapShot != e_ReqShiftType){	//電源OFF移行(Snapshot取得)ではない場合.
		// 復帰要因割り込み禁止.
		si_Ret2 = ioctl( si_Fd, THRD_PMU_IOCSPWRSUBINT, 0 );
		if( si_Ret2 < 0 ) {
			setFA14_PmuDriverIoError(0x02,errno);		// 2017/2/21 山口 AR.1013578 詳細コード0CEB調査.
			THRC_PM_ASSERT("pmu ioctl(THRD_PMU_IOCSPWRSUBINT) error.", 0,0,0,0,CFA14_DetailCode0x0CEB );
		}
	}
	// 2019/07/08 OP_BTS-9953 山口 DeepSleep復帰時間改善.
	memset( sc_Pass, 0, sizeof(sc_Pass) );	
	pc_Instance->getHddLockPassword_SPIFlash( sc_Pass ) ;	// SSDロックパスワード設定取得.
	if ( sc_Pass[0] != '\x0' ){				// SSDロックパスワードが有効の場合.
		isStorageUnlock = True;
		theHDDTrans.setSecurityLocked(1);
	} else {
		isStorageUnlock = False;
		theHDDTrans.setSecurityLocked(0);
	}
	// 2020/12/25 山口 OP-BTS_30056 FAXジョブあり状態でDeepSleep中に再起動.
	// JOBがあればDeepSleep移行時に電断領域に共有メモリの情報を書き込む.
	if( bl_setPowerDownRegion == False){
		pv_addr = THRF_getPowerDownRegion();
		// 2021/01/14 山口 静的解析ID:24664
		if(pv_addr != NULL){
			THRF_writePowerDownRegion(pv_addr);
		}
	}
#if 0
	// 電源サブCPUの割り込み有効.
	// 2014/05/02 takas 割り込み条件暫定見直し.
	// 割り込み有効にするもの.
	// 2014/11/13 M.Handa 割り込み設定位置を移動.ERP復帰で不要な割り込みを無効にする.
	THRC_PowerManagerDeviceSingletonIf::newInstance().SwitchPowerState( THRC_PowerDeviceID_SET_INTP4,	THRC_PowerStateID_LOW );	// 副電源SW.
	// 割り込み無効にするもの.
	if( True == b_ErP ) {	/* Erpモードの場合*/
		// ErP個別で割り込み設定
		// 割り込み有効にするもの.
		THRC_PowerManagerDeviceSingletonIf::newInstance().SwitchPowerState( THRC_PowerDeviceID_SET_KR0,	  THRC_PowerStateID_LOW );	// 無線LANからの起動.
		// 割り込み無効にするもの.
		THRC_PowerManagerDeviceSingletonIf::newInstance().SwitchPowerState( THRC_PowerDeviceID_SET_INTP5, THRC_PowerStateID_NORMAL );// 原稿セット(EDH). 2014/11/13 CQ.504087.
		THRC_PowerManagerDeviceSingletonIf::newInstance().SwitchPowerState( THRC_PowerDeviceID_SET_INTP6, THRC_PowerStateID_NORMAL );// 原稿セット(DSET).2014/11/13 CQ.504087.
		THRC_PowerManagerDeviceSingletonIf::newInstance().SwitchPowerState( THRC_PowerDeviceID_SET_INTP7, THRC_PowerStateID_NORMAL );// 前カバーの開閉.  2014/11/13 CQ.504087.
		THRC_PowerManagerDeviceSingletonIf::newInstance().SwitchPowerState( THRC_PowerDeviceID_SET_KR1,	  THRC_PowerStateID_NORMAL );// FAXからの起動.
		THRC_PowerManagerDeviceSingletonIf::newInstance().SwitchPowerState( THRC_PowerDeviceID_SET_KR3,	  THRC_PowerStateID_NORMAL );// PHYからの起動.
		THRC_PowerManagerDeviceSingletonIf::newInstance().SwitchPowerState( THRC_PowerDeviceID_SET_KR4,   THRC_PowerStateID_NORMAL );// Fieryからの起動. /* 14.11.28 / sky_nkia / CQ.507406 */
		THRC_PowerManagerDeviceSingletonIf::newInstance().SwitchPowerState( THRC_PowerDeviceID_SET_KR5,	  THRC_PowerStateID_NORMAL );// 人体検出センサ.  2014/10/3 CQ.504507 堀井.
		THRC_PowerManagerDeviceSingletonIf::newInstance().SwitchPowerState( THRC_PowerDeviceID_SET_KR2,	  THRC_PowerStateID_NORMAL );// MODEM RI信号.  //2014/10/08 FT_yamaguchi CQ.503322. 
		THRC_PowerManagerDeviceSingletonIf::newInstance().SwitchPowerState( THRC_PowerDeviceID_SET_KR7,	  THRC_PowerStateID_NORMAL );// SMNP(Ⅱ).        2014/11/13 CQ.504087.
	} else {
		// DeepSleep個別で割り込み設定
		// 割り込み有効にするもの.
		THRC_PowerManagerDeviceSingletonIf::newInstance().SwitchPowerState( THRC_PowerDeviceID_SET_KR6,	THRC_PowerStateID_LOW );	// USBホスト通電.
		THRC_PowerManagerDeviceSingletonIf::newInstance().SwitchPowerState( THRC_PowerDeviceID_SET_KR5,	THRC_PowerStateID_LOW );	// 人体検出センサ. 2014/10/3 CQ.504507 堀井.
		THRC_PowerManagerDeviceSingletonIf::newInstance().SwitchPowerState( THRC_PowerDeviceID_SET_KR7,	THRC_PowerStateID_LOW );	// SMNP(Ⅱ).
		// 2014/10/08 FT_yamaguchi CQ.503322. 
		// HWのバグでCSRCの割り込みが常に飛んできてしまう.マスクの設定のフラグを追加し、コマンドよりHW改修済みの場合は割り込み受付可、非改修の場合は受付不可に設定する.
		if( True == bl_MODEM_MaskSetting ){
			THRC_PowerManagerDeviceSingletonIf::newInstance().SwitchPowerState( THRC_PowerDeviceID_SET_KR2,		THRC_PowerStateID_LOW );	// MODEM RI信号.
		} else {
			THRC_PowerManagerDeviceSingletonIf::newInstance().SwitchPowerState( THRC_PowerDeviceID_SET_KR2,		THRC_PowerStateID_NORMAL );	// MODEM RI信号.
		}
		// 割り込み無効にするもの.
		THRC_PowerManagerDeviceSingletonIf::newInstance().SwitchPowerState( THRC_PowerDeviceID_SET_KR3,	THRC_PowerStateID_NORMAL );	// PHYからの起動.

		// 2014/10/27 CQ.505932 / CQ.505934 河村 - 拡張ディープスリープ復帰時に原稿セットや前ドア開を検出してしまう問題の対策.
		switch( us_level ) {
		case 1:		// 0.5w.
			// 拡張ディープスリープモード(0.5w)の場合は使用できないセンサの割り込みは無効にする.
			// 有効はTHRC_PowerObserver::powerStatusChanged()にて行われる.
			THRC_PowerManagerDeviceSingletonIf::newInstance().SwitchPowerState( THRC_PowerDeviceID_SET_INTP5,	THRC_PowerStateID_NORMAL );	// 原稿セット(EDH).
			THRC_PowerManagerDeviceSingletonIf::newInstance().SwitchPowerState( THRC_PowerDeviceID_SET_INTP6,	THRC_PowerStateID_NORMAL );	// 原稿セット(DSET).
			THRC_PowerManagerDeviceSingletonIf::newInstance().SwitchPowerState( THRC_PowerDeviceID_SET_INTP7,	THRC_PowerStateID_NORMAL );	// 前カバーの開閉.
			break;
		default:
			break;
		}
	}
/* 2014/11/13 M.Handa CQ.507316対応 end */

	/* ↓S73-BSP_DD-29 */
	if( True == b_ErP ) {
		/* 無線LANによる復帰のための前処理をTHRC_PowerManagerSoC::req_ErPShift()(THRC_PowerManagerSoC.cpp)から引用 */
		STATUS statRet = MIO_WLAN_WOL_INTERNAL_ERROR;
		short cnt = 0;
		if( THRF_isExistWLAN() == True ){
			switch( APIC_ExNetworkSetting::newInstance()->getReturnFromErp() ){
				case TYPE_ReturnFromErp_None:
					// ErP設定しない.
					break;
				case TYPE_ReturnFromErp_MagicPacket:
					for( cnt = 0; cnt < PMU_AP_CHECK_CNT; cnt++ ){
						statRet = mioWlanSetErPMode( 16, 56 );	// マジックパケットによるErP復帰用のErP設定.
						if( statRet == MIO_WLAN_WOL_SUCCESSFUL_OK ){
							logMsg( "%d : req_DeepSleepShift() mioWlanSetErPMode_MagicPacket OK.\n", SYSF_tickGet(), 0, 0, 0, 0, 0 );
							break;
						}
						logMsg( "%d : req_DeepSleepShift() mioWlanSetErPMode_MagicPacket NG[%d].\n", SYSF_tickGet(), statRet, 0, 0, 0, 0 );
						DELAY_TSK(500);
					}
					break;
				case TYPE_ReturnFromErp_ARP_Unicast:
					for( cnt = 0; cnt < PMU_AP_CHECK_CNT; cnt++ ){
						statRet = mioWlanSetErPMode( 0, 8 );	// ARP+ユニキャストによるErP復帰用のErP設定.
						if( statRet == MIO_WLAN_WOL_SUCCESSFUL_OK ){
							logMsg( "%d : req_DeepSleepShift() mioWlanSetErPMode_ARPUnicast OK.\n", SYSF_tickGet(), 0, 0, 0, 0, 0 );
							break;
						}
						logMsg( "%d : req_DeepSleepShift() mioWlanSetErPMode_ARPUnicast NG[%d].\n", SYSF_tickGet(), statRet, 0, 0, 0, 0 );
						DELAY_TSK(500);
					}
					break;
				default:
					// ErP設定しない.
					break;
			}
			statRet = mioWlanSuspend();	// WLANを省電力状態へ.
			if( statRet == MIO_WLAN_WOL_SUCCESSFUL_OK ){
				logMsg( "%d : req_DeepSleepShift() mioWlanSuspend OK.\n", SYSF_tickGet(), 0, 0, 0, 0, 0 );
			}else{
				logMsg( "%d : req_DeepSleepShift() mioWlanSuspend NG[%d].\n", SYSF_tickGet(), statRet, 0, 0, 0, 0 );
			}
		}
	}
	/* ↑S73-BSP_DD-29 */

	if( False == b_ErP ) {
		/* DeepSleep 0.5W/0.7Wを指定 */
		switch( us_level ) {
		case 0:
			THRC_PowerManagerDeviceSingletonIf::newInstance().SwitchPowerState( THRC_PowerDeviceID_ENG_SENSOR,	THRC_PowerStateID_NORMAL );
			break;
		case 1:
			THRC_PowerManagerDeviceSingletonIf::newInstance().SwitchPowerState( THRC_PowerDeviceID_ENG_SENSOR,	THRC_PowerStateID_LOW );
			break;
		default:
			printf( "(%010u)(BSP) %s : Param error. b_ErP=%d, us_level=%d\n", SYSF_tickGet() , __FUNCTION__, (int32_t)b_ErP, (int32_t)us_level );
			THRC_PowerManagerDeviceSingletonIf::newInstance().SwitchPowerState( THRC_PowerDeviceID_ENG_SENSOR,	THRC_PowerStateID_NORMAL );
			break;
		}
	}
#endif
	isDeepSleepShift = True;	// 2015/11/26 CQ.521133 河村 - DeepSlee/ErP 移行/復帰フラグ セット.
	/* ↓S73-BSP_DD-29 */
// [ADD]SuperWarp for Nicfum Wi-Fi start.
	if(THRE_ReqShiftType_GetSnapShot != e_ReqShiftType){
// [ADD]SuperWarp for Nicfum Wi-Fi end.
		//Provisional avoidance(In case of never linking up ethernet, stall LPPP before system resume)
		if( b_EtherLinkUpFlag == True &&
		    THRF_isExistWLAN() == True &&
		    TYPE_NetworkIFMode_SingleSta == APIC_ExNetworkSetting::newInstance()->getNetworkIFMode() ){
			//++ 2019/04/24 Sky		【EagleH】Warning除去対応.
			si_Result = system( "ifconfig eth0 up" );
			if (si_Result != 0){
				printf("(%010u)(BSP) ## %s[%d] %s\n", SYSF_tickGet(), __FILE__, __LINE__, __FUNCTION__);
			}
			si_Result = system( "ifconfig eth0 down" );
			if (si_Result != 0){
				printf("(%010u)(BSP) ## %s[%d] %s\n", SYSF_tickGet(), __FILE__, __LINE__, __FUNCTION__);
			}
			//-- 2019/04/24 Sky		【EagleH】Warning除去対応.
			b_EtherLinkUpFlag = False;
		}

		if( True == b_ErP ) {
			// 無線LANからの起動の割り込み設定はmioWlanSetErPMode前に行う必要があるため本関数にて処理する.
			// ErP個別で割り込み設定
			/* 無線LANによる復帰のための前処理をTHRC_PowerManagerSoC::req_ErPShift()(THRC_PowerManagerSoC.cpp)から引用 */
			STATUS statRet = MIO_WLAN_WOL_INTERNAL_ERROR;
			short cnt = 0;
			// 2016/09/26 1003625 山口 NicFUMのAPIを実行する条件に拡張ネットワークアダプタ装着設定の条件追加.
			if( ( THRF_isExistWLAN() == True )
			 && ( True == APIC_SystemInfo::newInstance()->getExNetworkAdapter() ) ){		// 拡張ネットワークアダプタ装着設定時.
				// 2015/03/24 ft_kondo CQ,511211 ErP移行できない問題対策. 
				// 割り込み有効にするもの.
				THRC_PowerManagerDeviceSingletonIf::newInstance().SwitchPowerState( THRC_PowerDeviceID_SET_KR0,	  THRC_PowerStateID_LOW );	// 無線LANからの起動.

				switch( APIC_ExNetworkSetting::newInstance()->getReturnFromErp() ){
					case TYPE_ReturnFromErp_None:
						// ErP設定しない.
						break;
					case TYPE_ReturnFromErp_MagicPacket:
						for( cnt = 0; cnt < PMU_AP_CHECK_CNT; cnt++ ){
							statRet = mioWlanSetErPMode( 16, 56 );	// マジックパケットによるErP復帰用のErP設定.
							if( statRet == MIO_WLAN_WOL_SUCCESSFUL_OK ){
								logMsg( "%"PRId32" : req_DeepSleepShift() mioWlanSetErPMode_MagicPacket OK.\n", SYSF_tickGet(), 0, 0, 0, 0, 0 );
								break;
							}
							logMsg( "%"PRId32" : req_DeepSleepShift() mioWlanSetErPMode_MagicPacket NG[%d].\n", SYSF_tickGet(), statRet, 0, 0, 0, 0 );
							DELAY_TSK(500);
						}
						break;
					case TYPE_ReturnFromErp_ARP_Unicast:
						for( cnt = 0; cnt < PMU_AP_CHECK_CNT; cnt++ ){
							statRet = mioWlanSetErPMode( 0, 8 );	// ARP+ユニキャストによるErP復帰用のErP設定.
							if( statRet == MIO_WLAN_WOL_SUCCESSFUL_OK ){
								logMsg( "%"PRId32" : req_DeepSleepShift() mioWlanSetErPMode_ARPUnicast OK.\n", SYSF_tickGet(), 0, 0, 0, 0, 0 );
								break;
							}
							logMsg( "%"PRId32" : req_DeepSleepShift() mioWlanSetErPMode_ARPUnicast NG[%d].\n", SYSF_tickGet(), statRet, 0, 0, 0, 0 );
							DELAY_TSK(500);
						}
						break;
					default:
						// ErP設定しない.
						break;
				}
				statRet = mioWlanSuspend();	// WLANを省電力状態へ.
				if( statRet == MIO_WLAN_WOL_SUCCESSFUL_OK ){
					logMsg( "%"PRId32" : req_DeepSleepShift() mioWlanSuspend OK.\n", SYSF_tickGet(), 0, 0, 0, 0, 0 );
				}else{
					logMsg( "%"PRId32" : req_DeepSleepShift() mioWlanSuspend NG[%d].\n", SYSF_tickGet(), statRet, 0, 0, 0, 0 );
				}
			}
		}
// [ADD]SuperWarp for Nicfum Wi-Fi start.
	}
// [ADD]SuperWarp for Nicfum Wi-Fi end.
	
	/* USBホスト・Lot26対応・リテンション状態を判定する */
	t_eMioWkupMode MioWakeupMode= MIO_DEEP_SLEEP_MODE;
	if( b_ErP== True ) MioWakeupMode= MIO_RETENTION_MODE;
	if(THRE_ReqShiftType_GetSnapShot==e_ReqShiftType) MioWakeupMode = MIO_SUPERWARP_MODE; /* ADD 2017.04.07 Y.Hagura */
	if( getUSBHostConnect()== True	&&
		IsLot26HWType()== True		&&
		b_ErP== False				&&
		// 2017/01/26 Matsuoka SuperWarp後にネットワーク使用できなくなる問題対応
		THRE_ReqShiftType_GetSnapShot != e_ReqShiftType ) {
		THRC_PM_LOG("### Lot26 mode is active ###");
		MioWakeupMode= MIO_LOT26_MODE;
		bLot26Active= True;
		PM_SEQ_REC( 0x3d );		/* ID不足のため0x3dを再利用 */
	}
	THRC_PM_LOG( "[PowOffPerf][%s] PowerSubCPU IntMask. USB rmmod. f_taiki proc : %lu\n", __FUNCTION__, SYSF_tickGet() ); 

	/* ↑S73-BSP_DD-29 */
	if(THRE_ReqShiftType_GetSnapShot != e_ReqShiftType){	//電源OFF移行(Snapshot取得)ではない場合.
		// Task_pre_DeepSleepShiftに処理を移動.
		si_SubCpuIntEnd = 1;
		SYSF_taskSpawn( "TASK_SUBCPUSINGL", 50, VX_FP_TASK, 8192, (FUNCPTR)THRC_PowerManager::Task_pre_DeepSleepShift,(intptr_t)b_ErP,(intptr_t)us_level,(intptr_t)bLot26Active,0,0,0,0,0,0,0 );
		// 2015/09/09 ft_kondo CQ.518620 B013/B014レジスタのBit15が立っていると、正しくwrite出来ない問題対策. 
		// 2015/03/23 CQ.511418 ft_kondo ErPはRB013に0x87FFを、DeepSleepはRB014に0x80FFを記録する. 
		if( True == b_ErP ) {
			//++ 2019/04/24 Sky		【EagleH】Warning除去対応.
			si_Result = system( "echo 0 > /sys/bus/platform/devices/PMU.0/devices/B013" ); // 0をwriteしておく. 
			if (si_Result != 0){
				printf("(%010u)(BSP) ## %s[%d] %s\n", SYSF_tickGet(), __FILE__, __LINE__, __FUNCTION__);
			}
			//-- 2019/04/24 Sky		【EagleH】Warning除去対応.
			SYSF_taskDelay(SYSF_sysClkRateGet() * 30 / 1000);	// 30ms wait.
			// 2015/10/13 ft_kondo CQ.519866 B013レジスタ FA14詳細コード対応. 
#if defined(DEF_MinervaS_PF) 
			//++ 2019/04/24 Sky		【EagleH】Warning除去対応.
			si_Result = system( "echo 34791 > /sys/bus/platform/devices/PMU.0/devices/B013" ); // ErPの場合は、0x87E7をwriteする. 
			if (si_Result != 0){
				printf("(%010u)(BSP) ## %s[%d] %s\n", SYSF_tickGet(), __FILE__, __LINE__, __FUNCTION__);
			}
#else 
			si_Result = system( "echo 34815 > /sys/bus/platform/devices/PMU.0/devices/B013" ); // ErPの場合は、0x87FFをwriteする. 
			if (si_Result != 0){
				printf("(%010u)(BSP) ## %s[%d] %s\n", SYSF_tickGet(), __FILE__, __LINE__, __FUNCTION__);
			}
			//-- 2019/04/24 Sky		【EagleH】Warning除去対応.
#endif // #if defined(DEF_MinervaS_PF) 
		} else {
			//++ 2019/04/24 Sky		【EagleH】Warning除去対応.
			si_Result = system( "echo 0 > /sys/bus/platform/devices/PMU.0/devices/B014" ); // 0をwriteしておく. 
			if (si_Result != 0){
				printf("(%010u)(BSP) ## %s[%d] %s\n", SYSF_tickGet(), __FILE__, __LINE__, __FUNCTION__);
			}
			SYSF_taskDelay(SYSF_sysClkRateGet() * 30 / 1000);	// 30ms wait.
			si_Result = system( "echo 33023 > /sys/bus/platform/devices/PMU.0/devices/B014" ); // DeepSleepの場合は、0x80FFをwriteする. 
			if (si_Result != 0){
				printf("(%010u)(BSP) ## %s[%d] %s\n", SYSF_tickGet(), __FILE__, __LINE__, __FUNCTION__);
			}
			//-- 2019/04/24 Sky		【EagleH】Warning除去対応.
		}
	}

	/************************************************************
	DeepSleepに遷移。
	************************************************************/
// Str 2010/09/13 ft_kondo 8536環境Sim対応.
// 2012/12/04 ZeusS対応.
#if !defined(DEF_ARMAGAXP)
#ifdef TORNADO			// 実機で有効.
	SYSF_semTake( semID_PanelPower , WAIT_FOREVER );	// 省電力中にブザー対応 堀井 2014/05/13.
#if 0 /* ↓S73-BSP_DD-28 4.2. */
	SYSF_taskDelay( (int32_t)(SYSF_sysClkRateGet() * 500 / 1000) );
#endif /* ↑S73-BSP_DD-28 4.2. */

// 2013/07/02 KM(Morikawa) SoC LinuxSim JFM Merge.
#if defined(TORNADO) && !defined(FUM_LINUX)
	/* 2014/07/11 NICFUM F_TAIKI対応 USBのclose/rmmod処理 */
	si_Ret = mioPreChangeModeToSleep(MioWakeupMode);
	THRC_PM_LOG( "[PowOffPerf][%s] mioChangeModeToSleep End : %lu\n", __FUNCTION__, SYSF_tickGet() ); 
#endif
#if 0 /* ↓S73-BSP_DD-28 4.2. mioRequestDeepSleepStart()で既に実施済みのため削除. */
	// 2012/07/10 ft_kondo CQ.367218 FAX受信出来ない問題. 
	system("rmmod -w /km/module/g_kmgadget.ko");			//suspend→g_kmgadget.koをrmmod.
	system("rmmod -w /km/module/net2280.ko");				//suspend→net2280.koをrmmod.
#endif /* ↑S73-BSP_DD-28 4.2. */

	if(si_Ret == OK){												//mioRequestDeepSleepStart()がOKを返した場合は、STRへ移行する.
		// 2011/03/25 CQ340449 河村 DeepSleep復帰NGリカバリ対応.
		if( wdtFd != -1 ){	// ウォッチドッグタイマ動作中.
			change_cntSoftDog( 0 );	// ウォッチドッグタイマ継続.
			bl_deepsleep = True;	// DeepSleepフラグセット.
		}else{	// ウォッチドッグタイマ未動作.
			wdtFd = open( "/dev/watchdog", O_RDWR | O_CLOEXEC );	// ウォッチドッグタイマ開始.
			if( wdtFd == -1 ){
				THRC_PM_LOG( "/dev/watchdog open() ERROR!! errno = [%d].", errno );
			}else{
				start_cyclicLoop_taskSoftDog();	// 定期起床タスクTASK_SoftDogにてシステム監視開始.
				bl_deepsleep = True;	// DeepSleepフラグセット.
				// 2011/05/11 CQ:344902 河村 - 異常検出リブート時のFA14詳細コード発生対応.
#if !defined(DEF_Helios_PF)
				char rebootCause;
				rebootCause = *((volatile unsigned char *)(ul_CS1_BaseAdr + 0xf6));
				rebootCause |= (1<<3);
				NVDF_memcpyNvRam( (void*)(ul_CS1_BaseAdr + 0xf6), &rebootCause, sizeof(rebootCause) );
#else	// #if !defined(DEF_Helios_PF).		// 2014/10/15 Harada NV-RAMレス Watchdogリブート対応.
				//++ 2019/04/24 Sky		【EagleH】Warning除去対応.
				ssize_t si_wtRet = write( wdtFd, "D", 1 );	// DeepSleep.	// 2014/10/15 Harada NV-RAMレス Watchdogリブート対応.
				if (si_wtRet == -1){
					THRC_PM_LOG( "/dev/watchdog write() ERROR!! errno = [%d].", errno );
				}
				//-- 2019/04/24 Sky		【EagleH】Warning除去対応.
#endif	// #if !defined(DEF_Helios_PF).
			}
		}
		// 2012/07/10 ft_kondo CQ.367218 FAX受信出来ない問題. 
//		system("rmmod /mfp/module/ehci-hcd.ko");				//suspend→USBHostドライバをrmmod.
//		system("rmmod /km/module/ehci-hcd.ko");				//suspend→USBHostドライバをrmmod.
/* 2015/04/30 半田 Emu72 無線LAN対応 */
// 2016/02/26 河村 - CQ.525415 ZeusSBKもZeusSと同様の処理を通すよう修正.
		// 2018/07/27 K.Sato Change USB SleepSequence [STR]suspend/resume [SuperWarp]insmod/rmmod
		if(THRE_ReqShiftType_GetSnapShot == e_ReqShiftType){
#if ( ( ( defined(DEF_S800Emu) || defined(DEF_ZEUSS_CUS) )  && !defined(DEF_A4_Cus) ) || defined(DEF_ZEUSSBK12_CUS) || defined(DEF_ZEUSSBK2ND_12_CUS) || defined(DEF_ZEUSSBKX0_CUS) && !defined(DEF_MinervaS_PF) )
			system("/usr/local/bin/unloadUSBModule.sh 1");  // 2014/11/06 sky_hnis 引数がある場合はXHCIを先にrmmodする.
#else	/* (defined(DEF_S800Emu) || defined(DEF_ZEUSS_CUS)) && !defined(DEF_MinervaS_PF) */
			// 2017/04/14 AR.1019598 山口 無線LANが装着している場合USB2.0-USB3.0ホストのrmmodの間に600msのWaitを入れる.
			if( THRF_isExistWLAN() == True ){
			//++ 2019/04/24 Sky		【EagleH】Warning除去対応.
				si_Result = system("/usr/local/bin/unloadUSBModule.sh 2");  
				if (si_Result != 0){
					printf("(%010u)(BSP) ## %s[%d] %s\n", SYSF_tickGet(), __FILE__, __LINE__, __FUNCTION__);
				}
			} else {
				si_Result = system("/usr/local/bin/unloadUSBModule.sh");
				if (si_Result != 0){
					printf("(%010u)(BSP) ## %s[%d] %s\n", SYSF_tickGet(), __FILE__, __LINE__, __FUNCTION__);
				}
			//-- 2019/04/24 Sky		【EagleH】Warning除去対応.
			}
#endif	/* (defined(DEF_S800Emu) || defined(DEF_ZEUSS_CUS)) && !defined(DEF_MinervaS_PF) */
		}
		int32_t cnt = 0;
#ifndef DEF_S800	// 今後不要になった場合は削除する.　S800ではHDDの省電力制御は不要のため削除.
		// 2015/1/13 CQ.508221 堀井 HDDのデバイスファイルチェック.
		for(cnt=0;cnt<100;cnt++){
			if( isLinkedHdd() == False ){
				break;
			}
			SYSF_taskDelay(SYSF_sysClkRateGet() * 10 / 1000);	// 10ms wait.
		}
		THRC_PM_LOG( "[PowOffPerf][%s] isLinkedHdd cnt[%d] : %lu\n", __FUNCTION__, cnt, SYSF_tickGet() ); 
		if(cnt >= 100){
			// HDD切断できていない場合はログ出力し、FA14を出す.
			printf( "(%010u)(BSP) THRC_PowerManager::req_DeepSleepShift: /dev/kmsdb still exists\n", SYSF_tickGet() );
			printf( "(%010u)(BSP) -> !dmesg\n", SYSF_tickGet() );
			system("dmesg");
			printf( "(%010u)(BSP) -> !ls /dev/ -l\n", SYSF_tickGet() );
			system("ls /dev/ -l");
			printf( "(%010u)(BSP) -> !lsof\n", SYSF_tickGet() );
			system("lsof");
			printf( "(%010u)(BSP) -> !mount\n", SYSF_tickGet() );
			system("mount");
			printf( "(%010u)(BSP) -> !lsusb -v\n", SYSF_tickGet() );
			system("lsusb -v");
			printf( "(%010u)(BSP) -> pmLogPrint\n", SYSF_tickGet() );
			pmLogPrint();
			printf( "(%010u)(BSP) -> pmDevicesState\n", SYSF_tickGet() );
			pmDevicesState();
			printf( "(%010u)(BSP) -> !fdisk -l /dev/kmsdb\n", SYSF_tickGet() );
			system("fdisk -l /dev/kmsdb");
			SYSF_taskDelay(SYSF_sysClkRateGet() * 1);	// ログ出力待ち 1s wait.
			// 2016/7/20 堀井 新詳細コード登録追加 CQ.532934 .
			uint64_t ull_NewDetailCode = (((uint64_t)0x22                          << 56) & 0xff00000000000000)		// モジュール番号.
									   | (((uint64_t)0x008                         << 44) & 0x00fff00000000000);	// 通し番号.
			int fd = SYSF_open("/proc/modules", O_RDWR, 0);
			if(fd > 0){
				char c_buf[4096] = {};
				if( SYSF_read(fd, c_buf, sizeof(c_buf) - 1) ){
					printf( "(%010u)(BSP) -> !cat /proc/modules\n%s\n", SYSF_tickGet() ,c_buf);
					ull_NewDetailCode |=(((uint64_t)(strstr(c_buf, "xhci_hcd") != NULL)      << 43) & 0x0000080000000000)	// xhci-hcd.ko.
									  | (((uint64_t)(strstr(c_buf, "dwc3 ") != NULL)         << 42) & 0x0000040000000000)	// dwc3.ko.
									  | (((uint64_t)(strstr(c_buf, "dwc3_mb86s70") != NULL)  << 41) & 0x0000020000000000)	// dwc3-mb86s70.ko.
									  | (((uint64_t)(strstr(c_buf, "ehci_hcd") != NULL)      << 40) & 0x0000010000000000)	// ehci-hcd.ko.
									  | (((uint64_t)(strstr(c_buf, "ehci_platform") != NULL) << 39) & 0x0000008000000000)	// ehci-platform.ko.
									  | (((uint64_t)(strstr(c_buf, "f_usb20ho_hcd") != NULL) << 38) & 0x0000004000000000);// f_usb20ho_hcd.ko.
				}else{
					ull_NewDetailCode |=(((uint64_t)(1) <<  0) & 0x0000000000000001);	// エラー記録.
				}
				SYSF_close(fd);
			}else{
				ull_NewDetailCode |=(((uint64_t)(1) <<  0) & 0x0000000000000001);	// エラー記録.
			}
			fd = SYSF_open("/etc/mtab", O_RDWR, 0);
			if(fd > 0){
				char c_buf[4096] = {};
				if( SYSF_read(fd, c_buf, sizeof(c_buf) - 1) ){
					printf( "(%010u)(BSP) -> !cat /etc/mtab\n%s\n", SYSF_tickGet() ,c_buf);
					ull_NewDetailCode |=(((uint64_t)(strstr(c_buf, "kmsdb4") != NULL)      << 37) & 0x0000002000000000)	// kmsdb4.
									  | (((uint64_t)(strstr(c_buf, "kmsdb5") != NULL)      << 36) & 0x0000001000000000)	// kmsdb5.
									  | (((uint64_t)(strstr(c_buf, "kmsdb6") != NULL)      << 35) & 0x0000000800000000)	// kmsdb6.
									  | (((uint64_t)(strstr(c_buf, "kmsdb7") != NULL)      << 34) & 0x0000000400000000)	// kmsdb7.
									  | (((uint64_t)(strstr(c_buf, "kmsdb8") != NULL)      << 33) & 0x0000000200000000)	// kmsdb8.
									  | (((uint64_t)(strstr(c_buf, "kmsdb9") != NULL)      << 32) & 0x0000000100000000)	// kmsdb9.
									  | (((uint64_t)(strstr(c_buf, "kmsdb10") != NULL)     << 31) & 0x0000000080000000)	// kmsdb10.
									  | (((uint64_t)(strstr(c_buf, "kmsdb11") != NULL)     << 30) & 0x0000000040000000);	// kmsdb11.
				}else{
					ull_NewDetailCode |=(((uint64_t)(1) <<  1) & 0x0000000000000002);	// エラー記録.
				}
				SYSF_close(fd);
			}else{
				ull_NewDetailCode |=(((uint64_t)(1) <<  1) & 0x0000000000000002);	// エラー記録.
			}
			// 2017/06/20 堀井 AR.1022784 詳細コード0D71を1回だけ記録する.
			static Bool uc_isFirstFlag_0D71 = True;
			if( uc_isFirstFlag_0D71 ){
				uc_isFirstFlag_0D71 = False;
				printf( "(%010u)(BSP) ## %s us_DetailCode=%#x, ull_NewDetailCode=%#llx\n", SYSF_tickGet() , __FUNCTION__, TYPE_FatalDetailCode_undefined, ull_NewDetailCode);
				APIC_setNewDetailCode( CFA14_DetailCode0x0D71, ull_NewDetailCode );		// 詳細コード登録.
			}
#if 0	// 2017/06/20 堀井 AR.1022784 0D71検出してもrebootしない.
			//起動時間短縮処理の場合には主電源OFFをする.
			if(e_ReqShiftType == THRE_ReqShiftType_GetSnapShot){
				THRF_setPowerOffDelay( False );
			} else if( TYPE_RebootSettingTypeReboot == APIC_MachineSetting::newInstance()->getRebootSettingForCRankFATAL() ){	//Cランクトラブルリセットでのみrebootする.
				APIC_SystemServiceFunction::newInstance()->reboot();
			}
			while( 1 ){	// Reboot,主電源OFFしない場合は無限ループ待ち.
				SYSF_taskDelay(1);
			}
#endif
		}
#endif

		if(THRE_ReqShiftType_GetSnapShot != e_ReqShiftType){	//電源OFF移行(Snapshot取得)ではない場合.
			for(cnt=0;cnt<100;cnt++){
				if( si_SubCpuIntEnd == 0 ){
					break;
				}
				SYSF_taskDelay(SYSF_sysClkRateGet() * 10 / 1000);	// 10ms wait.
			}
			if(cnt >= 100){
				THRC_PM_LOG( "Task_pre_DeepSleepShift(): Time Over " );
			}
		}
		// 2016/7/04 山口 CQ.532124 Erp移行途中に割り込み発生でリブートする.
		// Erp移行の場合割り込み要因を取得する.
		if(True == b_ErP){
			u32_INT_SOURCE = THRF_getIntSource();
		}
		// 2019/07/20 Sky OP_BTS-10168：スナップショット取得時、割り込み要因不要の為、クリアする. 
		if( THRE_ReqShiftType_GetSnapShot == e_ReqShiftType ) {
			printf("(%010u)(BSP) INT_SOURCE_CLEAR u32_INT_SOURCE = %X\n", SYSF_tickGet(), u32_INT_SOURCE);
			u32_INT_SOURCE = 0;
		}
// 2017/12/26 Hatano コンパイルスイッチ<DEF_MINERVAMLK_CUS>削除 Start.
/* #if defined(DEF_MINERVAMLK_CUS) */
// 2017/12/26 Hatano コンパイルスイッチ<DEF_MINERVAMLK_CUS>削除  End.
		// 省電力移行中に主電源OFFされた場合、復帰処理⇒主電源OFFシーケンスを行う必要がある為.
		// ここで主電源OFF割り込みが発生していたかチェックを行い、後段の処理にて復帰を行う.
		Bool bl_CheckMainPowerOff = False;
// Erp移行に関係なく、割り込みソースが無い場合、主電源OFFSWをチェックする.
        if( ( THRE_ReqShiftType_GetSnapShot != e_ReqShiftType ) && ( 0 == (u32_INT_SOURCE & 0x4000) ) ) {
			uint32_t u32_IntPinStatus = THRF_getIntPinStatus();
			if( 0 != (u32_IntPinStatus & 0x0004) ) {
				bl_CheckMainPowerOff = True;
			}
		}
// 2017/12/26 Hatano コンパイルスイッチ<DEF_MINERVAMLK_CUS>削除 Start.
/* #endif */
// 2017/12/26 Hatano コンパイルスイッチ<DEF_MINERVAMLK_CUS>削除  End.

       	// 2015/03/18 荒木 CQ.510572 DeepSleepでPCプリントJOBがあればスリープしない.
       	Bool bl_ActiveJob = check_ActiveJobList();
		THRC_PM_LOG("%s ActiveJob=%d, ErP=%d, SubPowerOffMode=%d", __FUNCTION__, bl_ActiveJob, b_ErP, bl_SubPowOffMode );
      	if( ( True == bl_ActiveJob ) &&	( False == b_ErP ) && 
      		( False == bl_SubPowOffMode ) ) {  // CQ518419対応「副電源オフであれば処理しない」条件追加.
        	pmuEvent.add( 0x1305 ); 
        	THRC_PM_LOG( "Skip deep sleep for active job list" ); 
       	}
		// 2016/03/10 山口 CQ.525992 Deep移行直前にPCプリント実行すると、自動で復帰しない 
		else if( ( True == APIC_Power::newInstance()->isDeepSleepDelayExist() )&&( False == b_ErP ) ){
			// DeepSleep移行禁止の場合、Sleepに復帰させる.
        	pmuEvent.add( 0x1305 ); 
        	THRC_PM_LOG( "Skip deep sleep for DeepSleep delay" ); 
		}
// 2017/12/26 Hatano コンパイルスイッチ<DEF_MINERVAMLK_CUS>削除 Start.
/* #if defined(DEF_MINERVAMLK_CUS) */
// 2017/12/26 Hatano コンパイルスイッチ<DEF_MINERVAMLK_CUS>削除  End.
		else if( True == bl_CheckMainPowerOff ) {	// 主電源スイッチOFF.
			THRC_PM_LOG( "Skip DeepSleep/Erp for MainPowerSwitch Off interrupt" );
		}
		else if( u32_INT_SOURCE & 0x4000 ){	// 主電源スイッチ割り込み(OFF検出).
			pmuEvent.add(0x126A);						// 主電源OFF検出の通知.
			THRC_PM_LOG( "Skip DeepSleep/Erp for MainPowerSwitch Off interrupt and SendPmuEvent" );
		}
// 2017/12/26 Hatano コンパイルスイッチ<DEF_MINERVAMLK_CUS>削除 Start.
/* #endif */
// 2017/12/26 Hatano コンパイルスイッチ<DEF_MINERVAMLK_CUS>削除  End.
		// 2016/7/04 山口 CQ.532124 Erp移行途中に割り込み発生でリブートする.
		// Erpから復帰する割り込みの場合「exec_str」をCALLしない.
		else if( u32_INT_SOURCE & 0x0022 ){		// 電源キー/無線LAN割り込み.
			pmuEvent.add(0x1262);		/* [副]電源SW押下開始 */
			pmuEvent.add(0x1306);		/* [副]電源ON短押し   */
        	THRC_PM_LOG( "Skip Erp for PowerSW interrupt" ); 
        	// 2018/11/09 AR.1064027 河村 - STRに入らない場合はここで電源キー割り込みを再度有効にする【暫定】.
        	si_Ret2 = ioctl( si_Fd, THRD_PMU_IOCSPWRSUBINT, 1 );
    		if( si_Ret2 < 0 ) {
	        	THRC_PM_LOG( "PMU_IOCSPWRSUBINT error:Skip Erp for PowerSW interrupt" ); // 暫定対応のため、エラーログ出力のみとする. 
    		}
   		}
		else if( u32_INT_SOURCE & 0x2000 ){		// RTC割り込み.
			pmuEvent.add(0x1263);		/* タイマからの復帰 */
        	THRC_PM_LOG( "Skip Erp for RTC interrupt" ); 
		}
		else {
			/* 2014/11/13 M.Handa CQ.507316対応 start */
			// 復帰要因割り込み許可.
			// 2014/05/02 takas 割り込み条件暫定見直し.
			/* この時点の設定では遅すぎるため、関数の先頭に移動する(CQ.507316) */
			/* 2014/11/13 M.Handa CQ.507316対応 end */
			/* 2014/07/11 NICFUM F_TAIKI対応 スリープモード/リテンションモードに移行 */
			si_Ret = mioChangeModeToSleep(MioWakeupMode);

			/* 2016/01/29 デバイスの電源OFF/ON制御LOG出力 Start.  */
#ifdef	DEF_A4_Cus
			if( True == b_ErP ) {
				printf( "@@@@@_PD#3 = [OFF]\n" );
				printf( "@@@@@_PD#2 = [OFF]\n" );
				printf( "@@@@@_PD#1 = [OFF]\n" );
        	} 
        	else {
				printf( "@@@@@_PD#3 = [OFF]\n" );
				printf( "@@@@@_PD#2 = [OFF]\n" );
			}
#endif	// DEF_A4_Cus
			/* 2016/01/29 デバイスの電源OFF/ON制御LOG出力 End.  */

			if( OK != si_Ret ) {
				b_isErr_mioChangeModeToSleep =True;	// 2019/12/03 河村 -  OP_BTS-15604.
				/* 2014/11/13 M.Handa CQ.507316対応 start */
				// CQ.504087対応 復帰要因割り込み許可.
				si_Ret2 = ioctl( si_Fd, THRD_PMU_IOCSPWRSUBINT, 1 );
				if( si_Ret2 < 0 ) {
					//起動時間短縮処理の場合には主電源OFFをする.
					if(e_ReqShiftType == THRE_ReqShiftType_GetSnapShot){
						printf( "(%010u)(BSP) %s: ioctl(THRD_PMU_IOCSPWRSUBINT) Error!!! \n", SYSF_tickGet() , __FUNCTION__ );
						THRF_setPowerOffDelay( False );
					}else{
						setFA14_PmuDriverIoError(0x03,errno);		// 2017/2/21 山口 AR.1013578 詳細コード0CEB調査.
						THRC_PM_ASSERT("pmu ioctl(THRD_PMU_IOCSPWRSUBINT) error.", 0,0,0,0,CFA14_DetailCode0x0CEB );
					}
				}
				/* 2014/11/13 M.Handa CQ.507316対応 end */
				THRC_PM_LOG( "req_DeepSleepShift(): mioChangeModeToSleep() return ERROR!!", (True == b_ErP ? "Retention" : "Suspend") );
				pmuEvent.add( 0x1305 );
			} 
			else {
				// 2015/07/24 sky_okuma CQ516746対応.
				/* デバッグログ保存不可設定 */
				setDebugLogSaveEnable( False );
				//[Debug]  アクセス禁止.
				theHDDTrans.setDisableSSDAccess(1);	// OP_BTS-22176:SSDアクセス禁止状態設定(1:禁止).
				// 2015/07/24 sky_okuma CQ516746対応.
				
				/* 2019/01/31 AR1072285 山口 CA72 WDT対応. */
				// 省電力移行中の区間を対象とする.
#if 0	// 2020/04/28 河村 -  OP_BTS-22858 CA72WDTを省電力制御以外も動作させる対応.
				if (True == isWdOn_CA72) {	// CA72WDT有効の場合.
					isWdStart_CA72 = False;	// CA72WDT Startクリア.
				}
#endif
				if(THRE_ReqShiftType_GetSnapShot == e_ReqShiftType){
					// 2017/01/27 デバックコマンドでシーケンスをSTOPさせる.
					// 2019/08/26 山口 OP_BTS-11998 DeepSleep中にフリーズ.
					if (True == isWdOn_CA72) {	// CA72WDT有効の場合.
#if 0	// 2020/04/28 河村 -  OP_BTS-22858 CA72WDTを省電力制御以外も動作させる対応.
						isWdStart_CA72 = False;	// CA72WDT Startクリア.
						si_Ret2 = ioctl( si_Fd, THRD_PMU_IOCSSET_WDT_CA72, DEF_WDT_CA72_END );	// END.
#else
						ca72wdt_ext_refresh( 0x4101, 600, 0x0000, "SuperWarp get snapshot.\n" );
#endif
					}

					if (THRE_SWSequenceStop==s_debugCmd) {
						printf( "(%010u)(BSP) @@@ SuperWarp Sequence Stop !! @@@\n", SYSF_tickGet() );
						volatile int32_t sequenceStop = 0;	// 最適化防止用カウント.
						while(true) {
							// 無限Loopで電源断を待つ.
							sequenceStop++;
							SYSF_taskDelay(SYSF_sysClkRateGet() * 1 / 1000);	// 1ms wait.
						}
					}
					check_PowerOffSequenceComplete();	// スレッド省電力制御の並列制御の完了チェック.
					req_SuperWarpSnapShot();
					set_MainSwOFF_Flag(False);	//2017/4/26 Sky-NAKANISHI AR1019633 主電源OFF後に前ドア開閉するとMFPパネルが起動する問題の対応.
					//Provisional avoidance(In case of never linking up ethernet, stall LPPP before system resume)
					b_EtherLinkUpFlag = True;
					/* 2020/02/26 Harada OP_BTS-20942 LPDDR4パラメータ化け LPDDR4パラメータ領域の多重化対応 */
					THRG_recoverLpddr4Param();	/* LPDDR4パラメータ領域復旧 */
					// 2019/08/26 山口 OP_BTS-11998 DeepSleep中にフリーズ.
#if 0	// 2020/04/28 河村 -  OP_BTS-22858 CA72WDTを省電力制御以外も動作させる対応.
					if (True == isWdOn_CA72) {	// CA72WDT有効の場合.
						si_Ret2 = ioctl( si_Fd, THRD_PMU_IOCSSET_WDT_CA72, DEF_WDT_CA72_START );	// START.
						isWdStart_CA72 = True;	// CA72WDT Startセット.
					}
#endif

				}else{
					/* DeepSleep/Erp移行 */
#if 1 /* 暫定対応:Sleep2復帰中にQSPIのアクセスでR4が固まる */
        			b_in_exec_str = true;
#endif
					/* 2019/01/17 AR1071618 山口 無線LANが接続できなくなる. */
					si_Ret2 = ioctl( si_Fd, THRD_PMU_IOCSGET_DETAILFLG, 1 );

					// 2018/12/25 ft_kondo AR1070568調査仕込み. 
					si_Ret2 = ioctl( si_Fd, THRD_PMU_IOCSGET_IRQ_DISABLE_FLG, &irq_disable_flg );
					THRC_PM_LOG("(%010u)(BSP) si_Ret2[%d] , irq_disable_flg[%d]\n" , SYSF_tickGet(), si_Ret2 , irq_disable_flg );
					if( si_Ret2 < 0 || irq_disable_flg == 1) {
						//[Debug] リブート前に許可.
						theHDDTrans.setDisableSSDAccess(0);	// OP_BTS-22176:SSDアクセス禁止状態設定(1:禁止).
						// 新詳細コード登録追加.
						uint64_t ull_NewDetailCode = ((((uint64_t)0x22              << 56) & 0xff00000000000000)	// モジュール番号.
												   |  (((uint64_t)0x011             << 44) & 0x00fff00000000000));	// 通し番号.
						Bool bl_ret = True;
						bl_ret = APIC_setNewDetailCode( TYPE_FatalDetailCode_undefined, ull_NewDetailCode );		// 詳細コード登録.
						if (False == bl_ret) {
							SYSF_logMsg( "(%010u)(BSP) ## %s %d : APIC_setNewDetailCode NG.  0x%016llx\n", SYSF_tickGet() , __FUNCTION__, __LINE__, ull_NewDetailCode,4,5,6 );
						} else {
							SYSF_logMsg( "(%010u)(BSP) ## %s %d : new detail code. 0x%016llx\n", SYSF_tickGet() , __FUNCTION__, __LINE__, ull_NewDetailCode,4,5,6 );
						}
						// 新詳細コード登録後、rebootさせる. 
						{ // 制御構成変更_No.40 リブート回数を数えるカウンタを追加する対応.
							APIC_OperateCnt *pAPIC_OperateCnt = APIC_OperateCnt::newInstance();
							if( NULL!=pAPIC_OperateCnt )
							{
								pAPIC_OperateCnt->inc(TYPE_OperateCnt_Reboot);
							}
						}
						APIC_SystemServiceFunction::newInstance()->reboot();
					}
					THRF_DoSync();
					exec_str();		/* ★ここに入ると省電力復帰まで戻ってこない */
//2019/12/17 Sky 【自動測定】起動イベントの通知対応(Step3) start.
					/* BSP復帰処理 開始を通知 */
					THRF_setEventRecord(THRD_EventRecord_BSPReturnStart);
//2019/12/17 Sky 【自動測定】起動イベントの通知対応(Step3) end.
				}
				/* 2019/01/31 AR1072285 山口 CA72 WDT対応. */
				// 省電力復帰中の区間を対象とする.
#if 0	// 2020/04/28 河村 -  OP_BTS-22858 CA72WDTを省電力制御以外も動作させる対応.
				if (True == isWdOn_CA72) {	// CA72WDT有効の場合.
					isWdStart_CA72 = True;	// CA72WDT Startセット.
				}
#endif

				// 瞬断を有効にする.
				THRF_setAPStatus(True);
				
				if ( 1 == pmMeasurementMode ) {
					// 省電力復帰時にKernelログにより、ログが崩れることがある為、printkのログレベルを上げる.
					changePrintkLogLevel();
				}
#ifdef	DEF_A4_Cus
				// 2015/10/09 eSOL石田 パフォーマンスログ対応.
				THRC_PerformanceLog::getInstance().getLogInstance(THRC_PerformanceLog::e_performanceLogTypeSleep)->set("%10d : Performance_RecoveryFromDeepSleep\n", SYSF_tickGet());
#endif	// DEF_A4_Cus
				
				/* 2014/07/11 NICFUM F_TAIKI対応 稼働モードに移行 */
				si_Ret =  mioChangeModeToNormal(MioWakeupMode);
					
				if( OK != si_Ret ) {
					b_isErr_mioChangeModeToNormal = True;		// 2019/12/03 河村 -  OP_BTS-15604.
					THRC_PM_LOG( "req_DeepSleepShift(): mioChangeModeToNormal() return ERROR!!", (True == b_ErP ? "Retention" : "Suspend") );
				}
				/* 2014/10/21 DeepSleep復帰時、SNMP以外のLANからの復帰の場合、LANパケット受信での復帰(0x1305)イベントを送信 */
//				if ( False == b_ErP ) {
					if ( checkFTaikiWakeupFactor(IsLot26HWType(),getUSBHostConnect()) != OK ) {
						THRC_PM_LOG( "req_DeepSleepShift(): checkFTaikiWakeupFactor() return ERROR!!" );
					}
//				}
				/* 2016/01/29 デバイスの電源OFF/ON制御LOG出力 Start.  */
#ifdef	DEF_A4_Cus
				if( True == b_ErP ) {
					
					THRC_PM_LOG( "@@@@@_PD#1 = [ON]\n" );
					THRC_PM_LOG( "@@@@@_PD#2 = [ON]\n" );
					THRC_PM_LOG( "@@@@@_PD#3 = [ON]\n" );
				}
				else {
					THRC_PM_LOG( "@@@@@_PD#2 = [ON]\n" );
					THRC_PM_LOG( "@@@@@_PD#3 = [ON]\n" );
				}
#endif	// DEF_A4_Cus
				/* 2016/01/29 デバイスの電源OFF/ON制御LOG出力 End.  */
			}
		} // *** True == check_ActiveJobList() && False == b_ErP ***.
		
		// 復帰要因割り込みマスク解除.
		THRC_PowerManagerDeviceSingletonIf::newInstance().SwitchPowerState( THRC_PowerDeviceID_SET_INTP4,	THRC_PowerStateID_NORMAL );	// 副電源SW.
		THRC_PowerManagerDeviceSingletonIf::newInstance().SwitchPowerState( THRC_PowerDeviceID_SET_KR7,	THRC_PowerStateID_NORMAL );		// SMNP(Ⅱ).
		if( True == b_ErP ) {	/* Erpモードの場合*/
			THRC_PowerManagerDeviceSingletonIf::newInstance().SwitchPowerState( THRC_PowerDeviceID_SET_KR0,	THRC_PowerStateID_NORMAL );	// 無線LANからの起動.
			THRC_PowerManagerDeviceSingletonIf::newInstance().SwitchPowerState( THRC_PowerDeviceID_SET_KR3,	THRC_PowerStateID_NORMAL );	// PHYからの起動.
		} else {
			THRC_PowerManagerDeviceSingletonIf::newInstance().SwitchPowerState( THRC_PowerDeviceID_SET_KR6,	THRC_PowerStateID_NORMAL );	// USBホスト通電.
		}
#if 1 /* ↓S73-BSP_DD-28 4.2. resumeAfterDeepSleepへ移動. */
#ifdef TORNADO			// 実機で有効.
		bWakeupUSB_Busy=True;		// 2019/06/27 Harada OP_BTS-4583 USBのresume前にsuspend処理が実行される問題対応、タスク生成前にフラグ設定する.
		// 2019/10/03 OP_BTS-11242 山口 詳細コード2008.	
		pc_ApiPower->setDeepSleepDelay( __FILE__, __LINE__ );
#endif // #ifdef TORNADO.

		// 2022/02/17 OP_BTS-37399 DeepSleepからSleepへ復帰したら、DeepSleep中のwd監視を明示的に止める. 
		if( True == bl_deepsleep ){
			change_cntSoftDog( DEF_WDT_CNT_MAX );	// ウォッチドッグタイマ終了.
			bl_deepsleep = False;					// DeepSleepフラグリセット.
		}

		// 2021/08/16 OP_BTS-35243 山口 SuperWarp起動で認証装置を認識しない.
		SYSF_taskSpawn( "TASK_WakeupUSB", 255, VX_FP_TASK, 16384, (FUNCPTR)THRC_PowerManager::resumeAfterDeepSleep, (intptr_t)b_ErP, (intptr_t)MioWakeupMode,(intptr_t)e_ReqShiftType ,0,0,0,0,0,0,0 );/* ←S73-BSP_DD-28 4.2. *//* ←S73-BSP_DD-29 */
		
/* 2014/11/13 M.Handa CQ.507316対応 start */
		// CQ.504087対応 (PMUドライバはOPENしっぱなしとする).
		// 復帰要因取得要求.
		// PMUドライバから復帰要因を取得したら、シグナルを飛ばしてくれる.
		si_Ret2 = ioctl( si_Fd, THRD_PMU_IOCSREQESTFACTOR, 0 );
		if( si_Ret2 < 0 ) {
			setFA14_PmuDriverIoError(0x04,errno);		// 2017/2/21 山口 AR.1013578 詳細コード0CEB調査.
			THRC_PM_ASSERT("pmu ioctl(PMU_IOCSREQESTFACTOR) error.", 0,0,0,0,CFA14_DetailCode0x0CEB );
		}
		// AR.1007037 2016/12/08 山口 DeepSleep移行中の電源キー長押し問題.
		// kernelの電源キー押下の割り込み有効.
		si_Ret2 = ioctl( si_Fd, THRD_PMU_IOCSSUBSWINT, 1 );
		if( si_Ret2 < 0 ) {
			setFA14_PmuDriverIoError(0x05,errno);		// 2017/2/21 山口 AR.1013578 詳細コード0CEB調査.
			THRC_PM_ASSERT("pmu ioctl(THRD_PMU_IOCSSUBSWINT) error.", 0,0,0,0,CFA14_DetailCode0x0CEB );
		}
/* 2014/11/13 M.Handa CQ.507316対応 end */
#else
		// 2012/07/10 ft_kondo CQ.367218 FAX受信出来ない問題. 
//		system("insmod /mfp/module/ehci-hcd.ko");				//resume→USBHostドライバをinsmod.
		system("insmod /km/module/ehci-hcd.ko");				//resume→USBHostドライバをinsmod.
		DELAY_TSK(1000);										//wakeupへアクセスできない対策のため、insmodが完了するまで1s待つ.

		system("echo disabled > /sys/bus/usb/devices/usb1/power/wakeup");	// 2011/10/21 佐々木 CQ:351293のZeusMLK対応 wakeupをdisableへ設定.
		system("echo disabled > /sys/bus/usb/devices/1-1/power/wakeup");
		system("echo disabled > /sys/bus/usb/devices/1-2/power/wakeup");
#endif /* ↑S73-BSP_DD-28 4.2. resumeAfterDeepSleepへ移動. */
	}else{															//mioRequestDeepSleepStart()がエラーを返した場合、STRには移行せず、DeepSleep移行シーケンスを中断する.
#if 0	// 2015/01/27 CQ.509330 河村 - エラーが返ってきた場合にFA14詳細コードを記録してリブートするよう変更.
/* 2014/11/13 M.Handa CQ.507316対応 start */
		// 復帰要因割り込み許可.
		si_Ret2 = ioctl( si_Fd, THRD_PMU_IOCSPWRSUBINT, 1 );
		if( si_Ret2 < 0 ) {
			THRC_PM_ASSERT("pmu ioctl(THRD_PMU_IOCSPWRSUBINT) error.", 0,0,0,0,CFA14_DetailCode0x0CEB );
		}
/* 2014/11/13 M.Handa CQ.507316対応 end */
		THRC_PM_LOG( "req_DeepSleepShift(): mioPreChangeModeToSleep() return ERROR!!");
		pmuEvent.add( 0x1305 );										//"LANパケット受信"のシグナルを受信させる.
#else
		// 2015/01/27 CQ.509330 河村 - エラーが返ってきた場合にFA14詳細コードを記録してリブートするよう変更.
		printf( "(%010u)(BSP) %s : mioRequestDeepSleepStart()にてエラー検出。FA14詳細コード0x2008の記録とリブート\n", SYSF_tickGet() , __FUNCTION__ );
		// 2019/12/03 河村 -  OP_BTS-15604 新詳細コード登録追加.
		uint64_t ull_NewDetailCode = ((((uint64_t)0x22                                                  << 56) & 0xff00000000000000)	// モジュール番号.
								   |  (((uint64_t)0x020                                                 << 44) & 0x00fff00000000000)	// 通し番号.
								   |  (((uint64_t)(b_isErr_mioChangeModeToSleep  == True)               <<  8) & 0x0000000000000100)   // 1bit.
								   |  (((uint64_t)(b_isErr_mioChangeModeToNormal == True)               <<  7) & 0x0000000000000080)   // 1bit.
								   |  (((uint64_t)APIC_SystemInfo::newInstance()->getExNetworkAdapter() <<  5) & 0x0000000000000060)   // 2bit getExNetworkAdapter()は0～2.
								   |  (((uint64_t)(THRF_isExistWLAN() == True)                          <<  4) & 0x0000000000000010)   // 1bit.
								   |  (((uint64_t)MioWakeupMode	                                        <<  1) & 0x000000000000000e)   // 3bit MioWakeupModeは0～3。1bitは予備.
								   |  (((uint64_t)(bWakeupUSB_Busy == True)                             <<  0) & 0x0000000000000001)   // 1bit.
		);
		Bool bl_ret = True;
		bl_ret = APIC_setNewDetailCode( CFA14_DetailCode0x2008, ull_NewDetailCode );		// 詳細コード登録.
		if (False == bl_ret) {
			SYSF_logMsg( "(%010u)(BSP) ## %s %d : APIC_setNewDetailCode NG.  0x%016llx\n", SYSF_tickGet() , __FUNCTION__, __LINE__, ull_NewDetailCode,4,5,6 );
		} else {
			SYSF_logMsg( "(%010u)(BSP) ## %s %d : new detail code. 0x%016llx\n", SYSF_tickGet() , __FUNCTION__, __LINE__, ull_NewDetailCode,4,5,6 );
		}
		{ // 制御構成変更_No.40 リブート回数を数えるカウンタを追加する対応.
			APIC_OperateCnt *pAPIC_OperateCnt = APIC_OperateCnt::newInstance();
			if( NULL!=pAPIC_OperateCnt )
			{
				pAPIC_OperateCnt->inc(TYPE_OperateCnt_Reboot);
			}
		}
		APIC_SystemServiceFunction::newInstance()->reboot();
#endif
	}
#if 0 /* ↓S73-BSP_DD-28 4.2. resumeAfterDeepSleepへ移動,このinsmodはmioRequestDeepSleepEnd()で既に実施済みのため削除. */
// 2013/07/02 KM(Morikawa) SoC LinuxSim JFM Merge.
#if defined(TORNADO) && !defined(FUM_LINUX)
	si_Ret = mioRequestDeepSleepEnd();						//D3Cold復帰後に"USBデバイスドライバopen用API"をコール.
#endif
	// 2012/07/10 ft_kondo CQ.367218 FAX受信出来ない問題. 
	system("insmod /km/module/net2280.ko");				//resume→net2280.koをinsmod.
	system("insmod /km/module/g_kmgadget.ko");				//resume→g_kmgadget.koをinsmod.

	THRL_ThAssert( (si_Ret == OK), "req_DeepSleepShift(), mioRequestDeepSleepEnd() return ERROR", 0 , 0, 0, 0, CFA14_DetailCode0x0961 );
#endif /* ↑S73-BSP_DD-28 4.2. resumeAfterDeepSleepへ移動,このinsmodはmioRequestDeepSleepEnd()で既に実施済みのため削除. */

	SYSF_semGive( semID_PanelPower );	// 省電力中にブザー対応 堀井 2014/05/14.
#endif // #ifndef TORNADO.
#endif
// End 2010/09/13 ft_kondo 8536環境Sim対応.

	/* アラーム割り込み無効 *//* SuperWarp復帰時は不要 */
	if ( THRE_ReqShiftType_GetSnapShot != e_ReqShiftType ) {
		CB_RTC::newInstance().setAlarmInterrupt( False );
	}
	// 2020/12/25 山口 OP-BTS_30056 FAXジョブあり状態でDeepSleep中に再起動.
	if( bl_setPowerDownRegion == False){
		THRF_erasePowerDownRegion();		
	}
	// 2021/03/18 山口 OP_BTS-31858 Erp移行シーケンス中断で電源キー効かなくなる.
	if( True == b_ErP ) {	/* Erpモードの場合*/
		si_Ret2 = ioctl( si_Fd, THRD_PMU_IOCSPWRSUBINT, 1 );
		if( si_Ret2 < 0 ) {
			setFA14_PmuDriverIoError(0x07,errno);		// 2017/2/21 山口 AR.1013578 詳細コード0CEB調査.
			THRC_PM_ASSERT("pmu ioctl(THRD_PMU_IOCSPWRSUBINT) error.", 0,0,0,0,CFA14_DetailCode0x0CEB );
		}
	}
	
	
// 2015/07/24 sky_okuma CQ516040対応 Start. 
	/* デバッグログ保存可能設定 */
	setDebugLogSaveEnable( True );
// 2015/07/24 sky_okuma CQ516040対応 End. 
	
	e_ReqShiftType_tmp = THRE_ReqShiftType_DeepSleep;
	
	// 2019/05/20 BSP 山口 AR1076953 req_DeepSleepShiftの開始/終了にログを追加.
	printf( "(%010u)(BSP) : req_DeepSleepShift() End\n", SYSF_tickGet());

}
			
// 2014/10/08 FT_yamaguchi CQ.503322. 
extern "C" void setMODEM_MaskSetting(Bool bl_flag){
	bl_MODEM_MaskSetting = bl_flag;
}
// ================================================================================================.
//! <!--機能--> 電源管理 制御部 　ErP移行要求.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
// 2011/04/07 浦崎 MinervaZeus初版.
// 2012/04/05 kurachi ZeusMLK機種 CQ:362220（Nic2枚ざし_WLAN省電力制御対応）.
// 13/02/28 Sky 機種共通化対応 THRC_PowerManagerSoC.cppから移植.
// ================================================================================================.
void THRC_PowerManager::req_ErPShift()
{
#if defined(TORNADO) && !defined(DEF_Helios_PF) /* ↓S73-BSP_DD-29 */
	SYSF_semTake( semID_PanelPower , WAIT_FOREVER );	// 省電力中にブザー対応 堀井 2014/05/13.
	// 2012/9/19 堀井 スライドレス_イルミネーション対応.
	CNVC_PanelHard*	ppc_PanelHard = CNVC_PanelHard::newInstance();
	if(ppc_PanelHard){
		ppc_PanelHard->setIllumiLED(THRD_IllumiLEDStatus_SLEEP);
	}
	// 2012/05/28 ft_kondo ZeusMLK：HWチェックモード対応. 
	// NVRAMの固定領域にフラグを立てて、HWチェックモードとErPからの復帰との棲み分けを行う. 
	uchar uc_setValue = 1;
#if !defined(DEF_Helios_PF)
	NVDF_memcpyNvRam( (void*)(ul_CS1_BaseAdr + 0x07FECD), &uc_setValue, sizeof(uc_setValue) );
#else
	THRG_setBspThrRegion( 0x0000, &uc_setValue, sizeof(uc_setValue) );
#endif
	system("poweroff -f");
	SYSF_semGive( semID_PanelPower );	// 省電力中にブザー対応 堀井 2014/05/13.
#endif/* ↑S73-BSP_DD-29 */
}

// ================================================================================================.
//! <!--機能--> 電源管理 制御部 　指定デバイスの省電力復帰確認.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param THRC_PowerDeviceID e_deviceID : デバイスID, 注意:テーブル(s_groupTable[])に登録されているデバイスのみ対象.
//! <!--戻値-->.
//! \return True:省電力復帰対象 / False:省電力復帰対象外.
//! <!--詳細-->.
//! ASIC初期化処理及びPowerChangeSequence()から呼び出す事を前提のサービス。指定デバイスが省電力復帰する場合にTRUEを返す\n.
//! <!--履歴-->.
//  09/08/07 越野	 Donau環境から分離して作成.
// 13/02/28 Sky 機種共通化対応 THRC_PowerManagerSoC.cppから移植.
// ================================================================================================.
Bool THRC_PowerManager::isLowToNormalOfDevice( THRC_PowerDeviceID e_deviceID )
{
	// 電源ステート移行中で無い場合はFalse.
	if( THRE_PowerSaveStatusNone == uc_ChangeStatusBefore || THRE_PowerSaveStatusNone == uc_ChangeStatusAfter ) {
		return False;
	}

	ushort i = 0;
	while( s_groupTable[i].e_newGroup != THRC_PowerDeviceID(-1) ) {
		if( e_deviceID == s_groupTable[i].e_newGroup ) {
			if( THRE_DiffExistToActive == getDevicePowerSaveStatusDiff( s_groupTable[i].e_oldGroup, (THRE_PowerSaveStatus)uc_ChangeStatusBefore, (THRE_PowerSaveStatus)uc_ChangeStatusAfter ) ) {
				return True;
			} else {
				return False;
			}
		}
		i++;
	}
	return False;
}

// ================================================================================================.
//! <!--機能--> 電源管理 制御部 　指定デバイスの省電力移行確認.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param THRC_PowerDeviceID e_deviceID : デバイスID, 注意:テーブル(s_groupTable[])に登録されているデバイスのみ対象.
//! <!--戻値-->.
//! \return True:省電力移行対象 / False:省電力移行対象外.
//! <!--詳細-->.
//! ASIC初期化処理及びPowerChangeSequence()から呼び出す事を前提のサービス。指定デバイスが省電力移行するする場合にTrueを返す\n.
//! <!--履歴-->.
//  09/08/07 越野	 Donau環境から分離して作成.
// 13/02/28 Sky 機種共通化対応 THRC_PowerManagerSoC.cppから移植.
// ================================================================================================.
Bool THRC_PowerManager::isNormalToLowOfDevice( THRC_PowerDeviceID e_deviceID )
{
	// 電源ステート移行中で無い場合はFalse.
	if( THRE_PowerSaveStatusNone == uc_ChangeStatusBefore || THRE_PowerSaveStatusNone == uc_ChangeStatusAfter ) {
		return False;
	}

	ushort i = 0;
	while( s_groupTable[i].e_newGroup != THRC_PowerDeviceID(-1) ) {
		if( e_deviceID == s_groupTable[i].e_newGroup ) {
			if( THRE_DiffExistToInActive == getDevicePowerSaveStatusDiff( s_groupTable[i].e_oldGroup, (THRE_PowerSaveStatus)uc_ChangeStatusBefore, (THRE_PowerSaveStatus)uc_ChangeStatusAfter ) ) {
				return True;
			} else {
				return False;
			}
		}
		i++;
	}
	return False;
}

// ================================================================================================.
//! <!--機能--> ファンロック状態のチェック.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--履歴-->.
//  09/08/07 越野	 Donau環境から分離して作成.
//! 2009/12/03 小霜 CQ.322442 - Soft Dip SWを変更.
//! 2011/07/12 河村 CQ.350760 - 画処理系動作中もファンロック検出条件にする.
// 2011/10/15 MinervaZeus_int リベース CQ.350760有効化.
// 13/02/28 Sky 機種共通化対応 THRC_PowerManagerSoC.cppから移植.
// 14/12/10 小林 CQ507444対応.
// 14/12/12 小林 CQ507966、CQ507967対応.
// 2014/12/22 小林 初版 CQ.508148/508299 省電力移行時にC-5370を誤検出する問題対応. 
// 15/1/16 小林 CQ508886 バスエラーが発生してしまう問題対応.
// 15/10/06	Sky	システム統合SoC(S800)対応.
// 18/04/16	Sky内藤 【Eagle】ファンロックのトラブルが発生しない対応.
// ================================================================================================.
#define DEF_BOX_FAN_LOCK_COUNT	20	// 100ms x 20.

void THRC_PowerManager::checkFanLock()
{
// 14/12/12 小林 CQ507444、CQ507966、CQ507967対応.
// Emu3は検出を無効にしている.
#if (defined(DEF_Helios_PF) && !defined(DEF_SparrowCus)) || (defined(DEF_A4_Cus) && !defined(DEF_SparrowCus))
	//BOXファンロック検出.
	static	 unsigned short	us_BoxFanLockCount = 0;
	APIC_SoftDipSw *pc_SoftDipSw = APIC_SoftDipSw::newInstance();
	ushort us_boxFanPowerState = pc_powerDevice->GetPowerState( THRC_PowerDeviceID_BOX_FAN );
	// S800のステータスがサスペンド状態の時はS800へのアクセスを禁止する.
	ushort us_S800PowerState = pc_powerDevice->GetPowerState( THRC_PowerDeviceID_PCIe_BUS );

	if( CB_BitPort<THRD_Port_DataType>::newInstance( CBBPTN_portFUN_LOCK_1 ) == 0									// ファンロックしているか？.
	 && (THRC_PowerStateID_NORMAL == us_boxFanPowerState || THRC_PowerStateID_LOW0 == us_boxFanPowerState)	// BOXファンの状態が有効であるか？.
	 && THRC_PowerStateID_NORMAL == us_S800PowerState															// S800動作中.
	 && THRC_McCpuIf::newInstance()->getReset() == False													// MCCPUがリセットチェック. 
	) {
		us_BoxFanLockCount++;
	} else {
		us_BoxFanLockCount = 0;
	}

	if( us_BoxFanLockCount >= DEF_BOX_FAN_LOCK_COUNT	// ファンロックが規定回数維持された.
	   && pc_SoftDipSw != NULL							// SoftDipSwの取得が可能か？.
	   && pc_SoftDipSw->getSoftDipSwBit( 28, 2 ) == 0 	// SoftDipSw 91が0x00(CPUファンロック検出を無効化しない)の場合、検出する.
	) {
#ifdef TORNADO				// VXWORKSで有効.
		//2009/03/30 newInstance対応 comMemoryManagerのインスタンス生成.
		IMGC_ImageMemoryPoolImpColorFileMemoryInstance& comMemoryManager = IMGC_ImageMemoryPoolImpColorFileMemorySingletonIf::newInstance();


#ifdef DEF_A4_Cus
		// 2016/03/02 石田 電源冷却ファンロックコード(0x5351)対応
		comMemoryManager.setFatalCodeToIJC( FATAL_POWER_FAN_LOCK );		// FATALコードの表示（電源冷却ＦＡＮロック）.
#else
		// 2008/03/18 河村 CQ.301499 - FATAL_FAN_LOCK2→FATAL_FAN_LOCKに修正(FATALコード間違い).
		comMemoryManager.setFatalCodeToIJC( FATAL_FAN_LOCK );		// FATALコードの表示（ＦＡＮロック）.
#endif // DEF_A4_Cus
#endif	// #ifdef TORNADO	// VXWORKSで有効.
	}	
#endif
}

// ================================================================================================.
//! <!--機能--> ファンロックエラー処理.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! ファンロックの種類に応じてFatalを発行 \n.
//! CPUファンロックの場合、発生から1秒経過したらCPUをロックさせる \n.
//! CPUをロックさせる理由は、ファンロック状態のままCPUが動作し続けると、故障の恐れがあるため \n.
//! t-kasahara追記.
//! DonauではCPUのファンロックエラーのみを本関数で処理するため、ファンコントローラのエラー状態を.
//! 確認するchkFanCtrErr()を実行する必要はない。.
//! <!--履歴-->.
//  09/08/07 越野	 Donau環境から分離して作成.
// 13/02/28 Sky 機種共通化対応 THRC_PowerManagerSoC.cppから移植.
// 20/11/20 Sky CPU高温異常に対するフェールセーフ対応.
// 20/12/15 Sky CPU高温異常に対するフェールセーフ対応：温度取得し続けない対応. 
// ================================================================================================.
void THRC_PowerManager::procFanLockErr()
{
		//CPUファンのみエラー処理のため、chkFanCtrErr()を実行しない.
		//CPUのFANロック.
		THRL_ThLogMsgErr( "CPU_FAN_LOCK FATAL OCCUR! DISP ONLY ONCE\n" );
	
		static Bool CPUTempErrorFlg = False;	// 2020/12/15 CPU高温異常に対するフェールセーフ対応：温度取得し続けない対応. 
		
		// 2020/12/15 CPU高温異常に対するフェールセーフ対応：温度取得し続けない対応 start. 
		if( CPUTempErrorFlg == False)
		{
		// 2020/11/20 Sky CPU高温異常に対するフェールセーフ対応.
		APIC_PrinterServiceInfo::newInstance()->savePaperTemp( TYPE_SavePaperTempType_CPUTempError );
			CPUTempErrorFlg = True;
			// 電源OFF/ONしない限り、温度情報を再度取得することはないため、フラグをFalseに戻す必要はない.
		}
		// 2020/12/15 CPU高温異常に対するフェールセーフ対応：温度取得し続けない対応 end. 
	
		DebugForever( FATAL_CPU_TEMP_ERROR );
//		//エラー割り込みから、パネルに Fatalコード表示指示を出しているはず。表示した後、CPUをスリープ状態にする.
//		//1秒後、CPUをスリープ状態に遷移 パネル定期起床が 500msなので2倍で1秒必要と判断.
		//Sleepモードを実行すると動作保障が無くなることから、実行しない.
		SYSF_taskDelay( (int32_t)(SYSF_sysClkRateGet() * 1000 / 1000) );
}

// ================================================================================================.
//! <!--機能--> 電源管理 制御部 　0W設定要求.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
// 2012/01/11 笠原 Android初版.
// 2012/01/20 Gonsho CQ360633 暗号化有効APでのErp復帰対応。パスフレーズ設定処理追加.
//                   iwconfig コマンドに失敗した時リトライするように修正.
//                   各種コマンドの設定の前にAndroid接続を切断するように修正.
// 2012/03/07 笠原 CQ361732 NICFUM代替処理の削除。.
//                 ErP移行指示を設定するように修正.
// 13/02/28 Sky 機種共通化対応 THRC_PowerManagerSoC.cppから移植.
// ================================================================================================.
void THRC_PowerManager::req_0WSetUp()
{
}
// ================================================================================================.
//! <!--機能--> 電源管理 制御部 　0W設定解除要求.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
// 2012/01/11 笠原 Android初版.
// 2012/03/07 笠原 ErP復帰指示を設定するように修正.
// 13/02/28 Sky 機種共通化対応 THRC_PowerManagerSoC.cppから移植.
// ================================================================================================.
void THRC_PowerManager::req_0WUnSetUp()
{
}

// ================================================================================================.
//! <!--機能--> 電源管理 制御部 　パネル電源状態操作.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param Bool bl_OnOff： Trueでパネルの電源オン、Falseでパネルの電源を元の状態に戻す.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//!  要求に応じてパネルの電源を操作する.
//!  Trueを要求した場合は必ずFalseで元に戻す.
//!  Falseで元に戻さないと、セマフォをロックする.
//! <!--履歴-->.
// 2013/09/24 堀井 次世代パネル対応_cap1166パネルマイコン制御化.
// 2014/04/10 toda Helios対応 PANEL_POWERはIMGSYS_PNLに統合したため削除.
// ================================================================================================.
void THRC_PowerManager::req_PanelPower(Bool bl_OnOff)
{
	static THRC_PowerStateID e_nowStateIMGSYS = THRC_PowerStateID_NONE;
	
	printf( "(%010u)(BSP) THRC_PowerManager::req_PanelPower bl_OnOff %d\n", SYSF_tickGet() ,bl_OnOff);

	if( semID_PanelPower == NULL ) {
		semID_PanelPower = SYSF_semMCreate( SEM_Q_PRIORITY | SEM_INVERSION_SAFE );
	}

	if(bl_OnOff){
		// 省電力解除.
		SYSF_semTake( semID_PanelPower, WAIT_FOREVER );	// セマフォ取得.
		e_nowStateIMGSYS = pc_powerDevice->GetPowerState( THRC_PowerDeviceID_IMGSYS );
		if( e_nowStateIMGSYS == THRC_PowerStateID_LOW ){
			pc_powerDevice->SwitchPowerState( THRC_PowerDeviceID_IMGSYS,	THRC_PowerStateID_NORMAL );
		}
	}else{
		// 省電力移行.
		if( e_nowStateIMGSYS == THRC_PowerStateID_LOW ){
			pc_powerDevice->SwitchPowerState( THRC_PowerDeviceID_IMGSYS,	e_nowStateIMGSYS );
		}
		SYSF_semGive( semID_PanelPower );	// セマフォ開放.
	}
	return;
}


/*================================================================================================
［機能］	【割り込み】パワーダウンの割り込み
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	無し
［履歴］	09/08/07 越野	 Donau環境から分離して作成 
			13/02/28 Sky 機種共通化対応 THRC_PowerManagerSoC.cppから移植.
================================================================================================*/
extern "C" {							// 2004/05/06 澤柳 Cから呼ぶので"C"で囲んだ.
void powerDown_Interrupt();
}
Bool bl_PowerDown_Occur = False;	// パワーダウン発生フラグ初期化.
static uchar uc_PowerDown_Count = 0;
static Bool bl_ReqResetReboot = False;	// エンジンリセットリブート要求初期化.

extern void wait_10us( uint32_t );
static Bool bl_MoniterChk = True;	// 型試機以降は正式対応で、ノイズ除去有効、パワーモニタチェック発生フラグ初期化.
void powerMoniter( int32_t set ) 
{
	if( set != 0 ) {
		bl_MoniterChk = True;
	} else {
		bl_MoniterChk = False;
	}
}
void powerDown_Interrupt()
{
	if( bl_PowerDown_Occur == True ){
		return;	// 一度SMIが入ってきている.
	}
	WAIT_10US(10);							// １００μＳのウエイト　ノイズ除去処理.

	bl_PowerDown_Occur = True;
	uc_PowerDown_Count = 2;		
}

/*================================================================================================
［機能］	パワーダウンの割り込み異常検出　
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	無し
［履歴］	09/08/07 越野	 Donau環境から分離して作成 
			13/02/28 Sky 機種共通化対応 THRC_PowerManagerSoC.cppから移植.
================================================================================================*/
void powerDown_ChkError()
{
	if( bl_PowerDown_Occur == True || bl_ReqResetReboot == True ) {
		APIC_SystemServiceFunction* p_SystemServiceFunction = APIC_SystemServiceFunction::newInstance();
		if (p_SystemServiceFunction){
			p_SystemServiceFunction->reboot();
		}
	} 
}

/*================================================================================================
［機能］	エンジンリセットリブート要求処理　
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	無し
［詳細］	エンジンリセットリブートフラグをセットすることにより、パワーダウンの割り込み異常検出と同じ処理でリブートする
［履歴］	09/08/07 越野	 Donau環境から分離して作成 
			13/02/28 Sky 機種共通化対応 THRC_PowerManagerSoC.cppから移植.
================================================================================================*/
void req_resetReboot()
{
	bl_ReqResetReboot = True;
	uc_PowerDown_Count = 2;		
}

/*================================================================================================
［機能］CPS2300入力切替
--------------------------------------------------------------------------------------------------
［引数］	uchar	uc_sel;		0:Scanner側  1:ASIC10・ASIC110側
［戻値］	無し
［履歴］	09/08/07 越野	 Donau環境から分離して作成 
			13/02/28 Sky 機種共通化対応 THRC_PowerManagerSoC.cppから移植.
================================================================================================*/
// Rhein2リブート問題対策 2005/08/08 伊藤(史)：Rhein1,2,3共通.
void cps2300_InputSel( uchar uc_sel )
{
	if( uc_sel == 0 ){
		// Scanner.
		CB_BitPortN<THRD_Port_DataType>::newInstance( CBBPTN_portLED_3 ) = 0;		// 確認のため LED 制御する。.
	}
	else{
		CB_BitPortN<THRD_Port_DataType>::newInstance( CBBPTN_portLED_3 ) = 1;		// 確認のため LED 制御する。.
	}
}

//**********************************************************************************************************.
// Donauにおいて、THRC_MemMan.cpp から 本モジュールへ移動 END !.
//**********************************************************************************************************.

void	debug_sleep( int32_t mode )
{
	BOOL								b_Chk = True;
	static ST_MsgPowerSaveStatusPara	st_MsgPowerSaveStatusPara;

	switch( mode ){
		case 0:		//スリープモードへの遷移.
			st_MsgPowerSaveStatusPara.uc_ChangeStatusBefore = THRE_PowerSaveStatusIdle;
			st_MsgPowerSaveStatusPara.uc_ChangeStatusAfter  = THRE_PowerSaveStatusSleep;
			break;
		case 1:		//スリープモードからの復帰.
			st_MsgPowerSaveStatusPara.uc_ChangeStatusBefore = THRE_PowerSaveStatusSleep;
			st_MsgPowerSaveStatusPara.uc_ChangeStatusAfter  = THRE_PowerSaveStatusIdle;
			break;
		default:	//モード設定エラー.
			THRL_ThLogMsgErr( "debug_sleep() illegal mode : mode=%"PRId32"\n"
				,mode
				,0,0,0,0,0);
			b_Chk = False;
			break;
	}

	if( b_Chk ){
		//メッセージ送信.
		sendMessage( TID_PowerManeger
			, TM_ReqStartChangePowerSaveStatus
			, (uintptr_t)&st_MsgPowerSaveStatusPara );
		THRL_ThLogMsgErr( "debug_sleep() -> sendMessage() : before=%d, after=%d\n"
			,st_MsgPowerSaveStatusPara.uc_ChangeStatusBefore
			,st_MsgPowerSaveStatusPara.uc_ChangeStatusAfter
			,0,0,0,0);
	}
}

extern "C" void pmChangeState( int32_t a, int32_t b )
{
	static ST_MsgPowerSaveStatusPara para = { THRE_PowerSaveStatusIdle, THRE_PowerSaveStatusIdle };

	para.uc_ChangeStatusBefore = (uchar)a;
	para.uc_ChangeStatusAfter  = (uchar)b;

	sendMessage( TID_PowerManeger
		, TM_ReqStartChangePowerSaveStatus
		, (uintptr_t)&para );
}


/*================================================================================================
［機能］	sysfs経由で現在のUSBホスト接続状態を参照できる関数
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	return True:接続されている / False:接続されていない
［履歴］	10/07/29 内野 初版
			2011/04/21 浦崎 CQ.338283、338232対応。SimWin時は、USBのデバイスファイルが
							存在しないためアクセスせず、常にホストと接続していないと判断する。
		2012/12/21 河村 CQ.376620 PCIeが無効の場合にIF番号がずれることにより正常動作しなくなる問題の対応.
			13/02/28 Sky 機種共通化対応 THRC_PowerManagerSoC.cppから移植.
================================================================================================*/

static const char *SS_STATEID_SUSPEND	= "suspend";
#define SS_STATEID_STRING_MAX_LEN 8					//len:11⇒8に変更.
#define	THRD_DeviceFile_UsbHost				"/sys/bus/platform/drivers/ci_hdrc/ci_hdrc.0/power/level"

Bool THRC_PowerManager::getUSBHostConnect()
{
#ifndef TORNADO		// 2011/04/21 浦崎 CQ.338283、338232対応。SimWin時は、USBのデバイスファイルが存在しないためアクセスせず、常にホストと接続していないと判断する。.
	return False;
#else
	// 2012/08/31 ft_kobayashi 静的解析 CID:441100.
	// 2011/12/19 ft_kondo 静的解析 CID:29038. 
	char buf[ SS_STATEID_STRING_MAX_LEN + 1];
	char* ed;

	FILE *fp = SYSF_fopen( THRD_DeviceFile_UsbHost, "rb" );
// 2011/08/26 Harada 静的解析対応 >>.
	if( fp == NULL ) {
		logMsg( "@@ %"PRId32": THRC_PowerManager::getUSBHostConnect() file open error.\n", SYSF_tickGet(), 0,0,0,0,0 );
		return False;
	}
// 2011/08/26 Harada 静的解析対応 <<.
	size_t size = fread( buf, 1, SS_STATEID_STRING_MAX_LEN, fp );	//fpから1バイトのデータをSS_STATEID_STRING_MAX_LEN個読込bufに格納.
	SYSF_fclose( fp );
	// 2012/08/31 ft_kobayashi 静的解析 CID:441100.
	// 2011/12/19 ft_kondo 静的解析 CID:29038. 
    buf[size] = '\0';
	ed = strchr( buf, '\n' );	// 改行が含まれていたら削除.
	if( ed ) {
		*ed = '\0';
		size--;
	}
	
	if( !strcmp( buf, SS_STATEID_SUSPEND ) ) {		//suspend：ホストPCが接続されていない.
		return False;
	}
	return True;									//on：ホストPCと接続されている.
#endif
}

/*================================================================================================
［機能］	スレッド->Power部へDeepSleep移行確認
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	return True:DeepSleep可能 / False:DeepSleep不可能
［履歴］	10/08/09 内野 初版
			13/02/28 Sky 機種共通化対応 THRC_PowerManagerSoC.cppから移植.
================================================================================================*/
Bool THRC_PowerManager::isShiftDeepSleep()
{
	if( getUSBHostConnect() == True ){
		return False;
	} else {
		return True;
	}
}


/*================================================================================================.
［機能］	PMUイベントの通知.
--------------------------------------------------------------------------------------------------.
［引数］	無し.
［戻値］	無し.
［履歴］	2011/05/09 河村 初版.
			2011/07/25 山下 CQ:344079のコメントアウトマージを解除 Soc用にsig_handlerの内容をコピー.
			2011/10/18 佐々木 ZeusMLK対応(各si_codeのSoC化、副電源S/W削除対応).
			2011/09/27 近藤 CQ.355900 LANケーブル挿抜でDeepSleepから復帰後、1分経過後にDeepSleepに遷移しない問題.
			2012/10/23 河村 CQ373991 パワーダウンによる復帰を追加.
			13/02/28 Sky 機種共通化対応 THRC_PowerManagerSoC.cppから移植.
			2013/09/21 Sky藤井 ディープスリープ省電力パフォーマンス対応 S73-BSP_DD-28 4.1.
			2013/09/24 堀井 次世代パネル対応_cap1166パネルマイコン制御化.
			2014/05/14 堀井 省電力中のブザー対応.
			2014/09/18 sky_tksk Fiery省電力対応.
			2014/12/10 半田 CQ.507291対応 DeepSleepの復帰不具合を修正
			2016/11/21 sky_ishiga [MinervaMLK] 起動時間短縮 電源SubCPUタイマー設定.
			2017/02/24 sky_hatono [MinervaMLK] 起動時間短縮 AR1013397 主電源をOFFしてもすぐにパネルが消灯しない.
			2017/4/26 Sky-NAKANISHI AR1019633 主電源OFF後に前ドア開閉するとMFPパネルが起動する問題の対応.
			2017/06/12 Sky-Araki AR1023861 DeepSleep移行中の主電源SW-OFFで3秒後に主電源OFFされる.
			2017/6/15 sky-ishiga AR1024990 主電源OFFのシーケンスログを記録.
			2017/12/13 Sky-ishitake [Eagle][起動時間短縮(再設計)] スイッチOFF～通電OFFの区間の割り込みを全マスク対応
			2018/03/28 Sky酒井(純) [Eagle] [制御構成変更_No.92-1] 振動パネル対応(追加).
			2018/07/11 Sky酒井(純) [Eagle] [AR.1058545] 電源OFFシーケンス実行中時間の設定を行う.
			2019/04/04 Sky[IT6_1.0MR2][AR1074753] 電源OFFタイマータスクの常駐化.
			2019/05/07 Sky 主電源SW-OFF時にsyncおよびnvmeコントローラ(SSD)のリセットを実施.
			2022/09/27 Sky 生産RINNEログ保存対応.
================================================================================================*/
void THRC_PowerManager::SendPmuEvent()
{
#ifdef TORNADO
	static Bool isPowerNtfyDeter = False;

	THRC_PowerReqID e_reqID = THRC_PowerReqID(-1);
	int32_t si_code = 0;
	THRS_PowerManagerMsg st_Msg = { 0 };
	Bool isPanelPowerOn = False;
	
	while( (si_code = pmuEvent.get()) != 0 ) {
		// 電源Mイベント通知抑止中.
		// かつ、SuperWarp起動イベント以外の場合.
		if ( (True == isPowerNtfyDeter) && (0xFFF != si_code) ) {
			// イベント処理は実施しない.
			THRC_PM_LOG( "pm event throw away [%x]\n", si_code );
			continue;
		}

		//シグナルsi->si_codeについてswitch文で分ける.
		e_reqID = THRC_PowerReqID(-1); // 2012/04/10 ft_kondo e_reqIDの初期化が抜けていたため追加. 
	    THRC_PM_LOG( "pm event 0x%"PRIx32"", si_code );
		// 2017/02/20 Sky酒井（純）ユーザーイベント記録(APIコール).
		SYSF_ReqTaskMonitorUserEvent(18000, 17, (uint32_t)si_code, 0);
	    switch( si_code ) {
		case 0xFFF:	//SuperWarp起動.
			isPowerNtfyDeter = False; // 電源Mイベント通知抑止解除.
	        e_reqID = THRC_PowerReqID_WakeSuperWarp;
	        isPanelPowerOn = False;
//2019/10/08 Sky 【自動測定】起動イベントの通知 start.
			/* BSP復帰処理 完了を通知 */
			THRF_setEventRecord(THRD_EventRecord_BSPReturnComplete);
//2019/10/08 Sky 【自動測定】起動イベントの通知 end.
	        break;
	    case 0x126A:	//主電源OFF検出.
	    	// 多重イベントをブロックする.
	    	if( get_MainSwOFF_Flag()== False ) {
				isPowerNtfyDeter = True; // 電源Mイベント通知抑止する
	    		bl_isRebooting = True;	// FAX詳細コード登録抑止のフラグを立てる. // 2020/11/27 OP_BTS-27365 山口 詳細コード対応_09C1.
				set_MainSwOFF_Flag(True);		//2017/4/26 Sky-NAKANISHI AR1019633 主電源OFF検知用フラグ設定.
	            e_reqID = THRC_PowerReqID_MainSwOFF;
	    		SendStart_PowerOffTimer();		// 電源OFFタイマー更新開始.
				setPowerOffSeqTime();			// 2018/07/11 Sky酒井(純) [Eagle] [AR.1058545] 電源OFFシーケンス実行中時間の設定を行う.
				if (True == isStorageUnlock) {	// ストレージのロク解除必要フラグが有効な場合.
					isResetContDelayFlag = True;	// reset_controller遅延フラグセット.	// 2020/04/17 Harada OP_BTS-21547 DeepSleepから主電源OFFでSnapshot取得しない問題対策 ロックパスワード設定時、Shutdownをアンロック後に行う対応.
				} else {
					sync();							// sync.
					if( THRE_StartupDeviceType_SSD == THRF_GetStartupDeviceType() ){ // SSDの場合.
						system("echo 1 > /sys/devices/platform/quartz-iris-sb-0/8d0098000.pcie0/pci0000:00/0000:00:00.0/0000:01:00.0/nvme/nvme0/reset_controller");
					}
				}
				THRL_createMeminfoLogFile(True);	// 2022/05/18 Sky メモリ測定強化対応.
	            PM_SEQ_REC( THRD_INITDEBUG_SEQ_POWER_DOWN );	//2017/6/15 sky-ishiga 主電源OFFシーケンスログを記録.
	            /* 2017/02/24 sky_hatono AR1013397 主電源をOFFを検出したらすぐにパネルを消灯させる. */
	            st_Msg.uc_localChangeStatusBefore = THRE_PowerSaveStatusIdle;
	        	st_Msg.uc_localChangeStatusAfter = THRE_PowerSaveStatusSubPowerOff15;
//	            st_Msg.us_message = TM_ReqPowerSavePanel;
	            st_Msg.us_message = TM_ReqMainSwOFFPanelSeq;	// 主電源OFFパネルシーケンス通知.	/* 2018/11/30 Harada AR.1068362 SuperWarp起動後、黒画面一瞬表示不具合対応 */
	            SYSF_msgQSend( thePowerManagerTask.msgQID_PowerManager_Panel, (char*)&st_Msg, sizeof(st_Msg), WAIT_FOREVER, MSG_PRI_NORMAL );
				if( ( True == APIC_SystemInfo::newInstance()->isCardLineMode() ) && ( TYPE_LineModeVer_2nd == APIC_SystemInfo::newInstance()->getCardLineModeVer() ) ) {
					// Rinneログ保存.
					printf("###rinne log stop\n");
					THRL_ReqRinneLog( THRD_COM_DBG_STOP_RINNE_LOG, NULL );
					THRL_ReqRinneLog( THRD_COM_DBG_SAVE_RINNE_LOG, NULL );
				}
	    	}
	    	else {
				THRC_PM_LOG("### %s(L%d)/Block multiple MainSW OFF event ###\n",__FUNCTION__,__LINE__);
			}
	        isPanelPowerOn = False;
	        break;
	    case 0x126D:	//前ドア開検出.
	        e_reqID = THRC_PowerReqID_DoorOpen;
	        isPanelPowerOn = True;
	        break;
	    case 0x1265:	//FAXからの復帰.
	        e_reqID = THRC_PowerReqID_WakeFax;
	        isPanelPowerOn = True;
	        break;
	    case 0x189:		//パネルワンキー復帰信号.
	        e_reqID = THRC_PowerReqID_WakePanel;
	        isPanelPowerOn = True;
	        break;
	    case 0x1103:	//副電源ON(IRQ[3])で復帰.
	    case 0x1262:	//副電源ON(WAKE_SUBSW)で復帰.
	        e_reqID = THRC_PowerReqID_WakeSubSW;
	        isPanelPowerOn = True;
	        break;
	    case 0x1269:	//IRからの復帰.
	        e_reqID = THRC_PowerReqID_WakeIR;
	        isPanelPowerOn = True;
	        break;
	    case 0x1263:	//タイマからの復帰、仮にIRQ番号を12として記載.
	        e_reqID = THRC_PowerReqID_WakeTimer;
	        isPanelPowerOn = True;
	        break;
	    case 0x1303:	//USBドライバからのホスト接続状態変化通知.
	        e_reqID = THRC_PowerReqID_USBDeviceAttach;
	        isPanelPowerOn = True;
	        break;
	    case 0x1304:	//USBドライバからのホスト非接続状態変化通知.
	        e_reqID = THRC_PowerReqID_USBDeviceDetach;
	        isPanelPowerOn = True;
	        break;
	    case 0x1305:	//LANパケット受信での復帰.
	        e_reqID = THRC_PowerReqID_WakeLANPacket;
	        isPanelPowerOn = True;
	        break;
/* 2014/12/10 M.Handa CQ.507291対応 start */
	    case 0x1264:    //PHY割込み(リンクアップ発生)での復帰.
			// 2018/08/09 AR.1054296 DeepSleep中のWOL割り込みで待機状態に復帰してしまう.
    		 e_reqID = THRC_PowerReqID_WakeOnLAN; 
			// 電源キーのイベントを削除.
	        isPanelPowerOn = True;
	    	break;
/* 2014/12/10 M.Handa CQ.507291対応 end */
	    case 0x1266:	//CSRC Ring受信での復帰.
	        e_reqID = THRC_PowerReqID_CSRCRingRcv;
	        isPanelPowerOn = True;
	        break;
	    case 0x1261:	//近接検知通知. 静電センサー対応.
	        e_reqID = THRC_PowerReqID_CTSensor;
	        // パネル電源管理タスクで割り込みを解除する 堀井 省電力中にブザー対応 2014/05/14.
			st_Msg.us_message = TM_ReqPanelCTSensorOff;
			SYSF_msgQSend( thePowerManagerTask.msgQID_PowerManager_Panel, (char*)&st_Msg, sizeof(st_Msg), WAIT_FOREVER, MSG_PRI_NORMAL );
	        isPanelPowerOn = True;
	        break;
	    case 0x1306:	//PowerSaveキー押下通知「短押し」. 副電源S/W削除対応.
	        e_reqID = THRC_PowerReqID_PSKeyON_Short;
	        isPanelPowerOn = True;
//2019/12/17 Sky 【自動測定】起動イベントの通知対応(Step3) start.
			/* BSP復帰処理 完了を通知 */
			THRF_setEventRecord(THRD_EventRecord_BSPReturnComplete);
//2019/12/17 Sky 【自動測定】起動イベントの通知対応(Step3) end.
	        break;
	    case 0x1307:	//PowerSaveキー押下通知「長押し」. 副電源S/W削除対応.
	        e_reqID = THRC_PowerReqID_PSKeyON_Long;
	        isPanelPowerOn = True;
	    	setVibrationLongTapPowerKey();	// 電源キー長押し時のパネル振動.
	        break;
	    case 0x126e:	//PowerDownによる復帰.
	        e_reqID = THRC_PowerReqID_PowerDown;
	        isPanelPowerOn = True;
	        break;
		/* ↓S73-BSP_DD-28 4.1. */
	    case 0x1308:	//プリンタドライバからの「プリントする」通知による復帰(1308は仮).
	        e_reqID = THRC_PowerReqID_ReceiveDataNeedPrint;
	        isPanelPowerOn = True;
	        break;
		/* ↑S73-BSP_DD-28 4.1. */
	    // 2014/07/03 FT_yamaguchi NFC対応.
	    case 0x1309:	//パネルハードキーによる復帰.
	    	e_reqID = THRC_PowerReqID_Panel_Hardkey;
	        isPanelPowerOn = True;
	    	break;
	    case 0x130a:	//タッチパネルによる復帰.
	    	e_reqID = THRC_PowerReqID_Touch_Panel;
	        isPanelPowerOn = True;
	    	break;
	    case 0x130b:	//NFC通信による復帰.
	    	e_reqID = THRC_PowerReqID_NFC_Trans;
			// 2016/06/09 山口 CQ.531483  省電力移行途中にNFC認証を行うと、電源キー押下でログインしてしまうことがある.
			APIC_Power::newInstance()->changePowerMode(TYPE_PowerModeCardDeviceAccess);		// 電源部に認証データが送られてきたことを通知する.
	        isPanelPowerOn = True;
	    	break;
	    case 0x130c:	//Bluetooth通信による復帰.
	    	e_reqID = THRC_PowerReqID_BLE_Trans;
			// 2016/06/09 山口 CQ.531483  省電力移行途中にNFC認証を行うと、電源キー押下でログインしてしまうことがある.
			APIC_Power::newInstance()->changePowerMode(TYPE_PowerModeCardDeviceAccess);		// 電源部に認証データが送られてきたことを通知する.
	        isPanelPowerOn = True;
	    	break;
/* 2014/09/18 sky_tksk for Fiery省電力 start. */
		case 0x1267:	//Fieryによる起動信号.
	    	e_reqID = THRC_PowerReqID_WakeFiery;
			sendMessage( TID_PowerManeger, TM_NotifyWakeupFromFiery, 0 ); // TASK_PowerManagerにメッセージを投げる. 
	        isPanelPowerOn = True;
			break;
/* 2014/09/18 sky_tksk for Fiery省電力 end. */
		// 2018/05/11 ft_kondo 機器構成変更No87 メカコンWDT対応. 
		case 0x1279:	//メカコンWDT割込み. 
			// 上位(PWR)への通知は行わず、PrintPのトラブル発行関数にC-5620をセットしてコールする.
			comMCPF.getCcFatalMcpf().setFatalCode( TYPD_TroubleCode_MCPF_WDT );
			THRC_PM_LOG( "pm event MC WDT Interrupt!!\n" );
			break;

	    default:		//何もしない.
	        break;
	    }

	    // 2017/09/22 河村 - Eagle省電力復帰時間短縮対応 パネル電源ONするイベント.
	    if( isPanelPowerOn ) {
	    	thePowerManagerTask.panelPowerOn();
	    }
		// Str 2010/10/19 ft_kondo 特にイベントが無ければコールバックは通知しないように修正.
	    if( thePowerManagerTask.p_PowerCallBack != NULL){	//コールバック登録チェック.
			if( THRC_PowerReqID(-1) != e_reqID ){
				if( e_reqID == THRC_PowerReqID_MainSwOFF ) {
					printf( "(%010u)(BSP) SendPmuEvent::MainSwOFF CallBack\n", SYSF_tickGet() );
				}
				thePowerManagerTask.p_PowerCallBack( e_reqID );
			}
		} else {
	        THRL_ThAssert( False, "SendPmuEvent() コールバック未登録 \n" , 0, 0, 0, 0, CFA14_DetailCode0x088E );
	    }
		// End 2010/10/19 ft_kondo 特にイベントが無ければコールバックは通知しないように修正.
	}
#endif
}

/*================================================================================================
［機能］	パネル省電力モード
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	THRE_PanelPowerMode_Normal:IT4仕様設定 / THRE_PanelPowerMode_Compatible:IT3仕様設定
［履歴］	2013/10/03 河村 初版 CQ.382962 - 省電力中のFAX受信音対応.
================================================================================================*/
int32_t pmPanelPowerMode = 0;	//デバッグ用 ターミナルから値を変更する.
THRE_PanelPowerMode THRC_PowerManager::panelPowerMode(void)
{
	APIC_SoftDipSw *pc_SoftDipSw = APIC_SoftDipSw::newInstance();
	if( pc_SoftDipSw == NULL ) {
		return THRE_PanelPowerMode_Normal;
	}
	//DipSW No157(bit2) NVRAM 78-2 (bit2).
	if( pc_SoftDipSw->getSoftDipSwBit( 78, 2 ) == True || pmPanelPowerMode ) {
		return THRE_PanelPowerMode_Compatible;
	} else {
		return THRE_PanelPowerMode_Normal;
	}
}

/*================================================================================================
［機能］	cap1166のi2cアクセスを禁止する。
--------------------------------------------------------------------------------------------------
［引数］	int value：  1：アクセス禁止   0：アクセス許可
［戻値］	無し
［履歴］	第1OSW浦崎 新規作成。CQ.358025対応2。省電力シーケンス中はi2cアクセスさせない。
			13/02/28 Sky 機種共通化対応 THRC_PowerManagerSoC.cppから移植.
================================================================================================*/
static int32_t cap1166_fd = -1;
#define CAP1166_SET_I2C_AC_ON_OFF       4   /*i2cアクセス許可/不許可。 11/11/18 浦崎 CQ.358025対応2 */
struct led_ioctl_info_struct ctsc_i2c_access_off;
void cap_i2c_access_off( int32_t value )
{
#ifdef TORNADO			// 実機で有効.
	if( -1 == cap1166_fd ) {
		cap1166_fd = SYSF_open( THRD_DeviceFile_Cap1166, O_RDWR, 0666 );
		if( -1 == cap1166_fd ) {
            THRC_PM_LOG( "%"PRId32" : cap1166 open error.", SYSF_tickGet() );
            return;
        }
	}

    ctsc_i2c_access_off.id = (uchar)value;
    ctsc_i2c_access_off.state = 0;

	// 2012/06/29 ft_kobayashi 静的解析 CID:432401.
	int32_t ret = 0;
    ret = SYSF_ioctl(cap1166_fd, CAP1166_SET_I2C_AC_ON_OFF, (intptr_t)&ctsc_i2c_access_off);
    if(ret < 0){
		THRC_PM_LOG("%"PRId32" : cap1166 ioctl error.", SYSF_tickGet());
    	return;
    }

#endif
}
/*================================================================================================
［機能］	イルミユニットのi2cアクセスを禁止する。
--------------------------------------------------------------------------------------------------
［引数］	uchar value：  0以外：アクセス禁止   0：アクセス許可
［戻値］	無し
［履歴］	2012/9/14 堀井 新規作成 スライドレス_イルミネーション対応.
			13/02/28 Sky 機種共通化対応 THRC_PowerManagerSoC.cppから移植.
================================================================================================*/
#define ILLUMI_SET_I2C_AC_ON_OFF	1   /*i2cアクセス許可/禁止*/
void illumi_i2c_access_off( uchar value )
{
#ifdef TORNADO			// 実機で有効.
	ushort option = CNVC_PanelHard::newInstance()->getSlidePanelOptionInfo();
	static int32_t illumi_fd = -1;
	uchar setValue = value;

	if((option & THRD_PanelOP_IllumiLED) == 0){
		return;
	}
	if( -1 == illumi_fd ) {
		illumi_fd = SYSF_open( "/dev/illumi_unit", O_RDWR, 0666 );
		if( -1 == illumi_fd ) {
            THRC_PM_LOG( "%"PRId32" : illumi_unit open error.", SYSF_tickGet() );
            return;
        }
	}

	// 2012/12/05 ft_kobayashi 静的解析 CID-454504.
	int32_t ret = 0;
	ret = SYSF_ioctl( illumi_fd, ILLUMI_SET_I2C_AC_ON_OFF, (intptr_t)&setValue );
    if(ret < 0){
		THRC_PM_LOG("%"PRId32" : illumi_unit ioctl error.", SYSF_tickGet());
    	return;
    }

#endif
}

THRC_PowerObserver* THRC_PowerObserver::pc_powerObserver = NULL;

THRC_PowerObserver* THRC_PowerObserver::newInstance()
{
	if( pc_powerObserver == NULL ) {
		pc_powerObserver = new THRC_PowerObserver();
	}
	return pc_powerObserver;
}

SEM_ID semPowerStatusChange = NULL;
Bool isNowMainPowerStatusChange = False;	// メインの省電力シーケンス中.
extern Bool isNowEngPowerStatusChange;		// ENGが省電力移行中(Low移行限定).

/*================================================================================================
［機能］	PowerPからのステータス変化オブザーバ通知.
--------------------------------------------------------------------------------------------------
［引数］	TYPE_PowerSaveStatus e_PowerStatus：				変化後パワーステータス.
			TYPE_PowerWeeklyTimerStatus e_WeeklyTimerStatus：	変化後ウィークリータイマーステータス.
［戻値］	無し.
［履歴］	新規作成.
			2014/09/24 sky_tksk	Fiery省電力対応 ポート制御対象変更 ASIC777⇒CPU GPIO.
================================================================================================*/
void THRC_PowerObserver::powerStatusChanged(TYPE_PowerSaveStatus e_PowerStatus, TYPE_PowerWeeklyTimerStatus e_WeeklyTimerStatus)
{
	if( semPowerStatusChange == NULL ) {
		semPowerStatusChange = SYSF_semMCreate( SEM_Q_PRIORITY | SEM_INVERSION_SAFE );
	}
#if defined(DEF_Helios_PF) /* ↓S73-BSP_DD-29 */
	IMGC_ImageMemoryPoolImpColorFileMemoryInstance& comMemoryManager = IMGC_ImageMemoryPoolImpColorFileMemorySingletonIf::newInstance();
	// 2014/08/19 CQ.501018 sky_tksk PC-Printの子ジョブがHDD認識を待たないようにする対応 start.
	// ミラーリング許可の判断文を追加.
	if ( comMemoryManager.isIgnoreHddAccess() == True && isLinkedHdd() == True ) {
		THRC_PM_LOG( "Allow HDD mirroring PowerObserver" );
		comMemoryManager.resetIgnoreHddAccess() ;
	}
	// 2014/08/19 CQ.501018 sky_tksk PC-Printの子ジョブがHDD認識を待たないようにする対応 end.
#endif/* ↑S73-BSP_DD-29 */
	switch( e_PowerStatus ) {
		case TYPE_PowerSaveStatusIdle:									// 待機中. 
		case TYPE_PowerSaveStatusLowPowerDelay:							// ローパワー遅延モード. 
		case TYPE_PowerSaveStatusLowPower:								// ローパワーモード. 
		case TYPE_PowerSaveStatusPrtEngOnlyLowPowerCancel:				// ローパワープリントエンジン限定解除. 
		case TYPE_PowerSaveStatusPrtEngOnlyLowPowerCancelPanelOn:		// ローパワープリントエンジン限定解除(パネルOn). 
		case TYPE_PowerSaveStatusSleepDelay:							// スリープ遅延モード. 
		case TYPE_PowerSaveStatusSleep:									// スリープモード. 
		case TYPE_PowerSaveStatusPrtEngOnlySleepCancel:					// スリーププリントエンジン限定解除. 
		case TYPE_PowerSaveStatusPrtEngOnlySleepCancelPanelOn:			// スリーププリントエンジン限定解除(パネルOn). 
		case TYPE_PowerSaveStatusSubPowerOffDelay:						// 副電源OFF遅延. 
		case TYPE_PowerSaveStatusSubPowerOff:							// 副電源OFF. 
			//電源シーケンス完了.
			SYSF_semTake( semPowerStatusChange, WAIT_FOREVER );
			isNowMainPowerStatusChange = False;
			if( isNowEngPowerStatusChange == False ) {
				// ENG制御中は有効にしない.
				THRC_PM_LOG( "CAP1166 Effectiveness" );
#ifdef TORNADO
#if 0	// 2013/09/24 堀井 次世代パネル対応_cap1166パネルマイコン制御化.
				cap_i2c_access_off( 0 );
#endif
				illumi_i2c_access_off( 0 );	// 2012/9/18 堀井 スライドレス_イルミネーション対応.
#endif
			}
			SYSF_semGive( semPowerStatusChange );
			// 2012/04/10 ft_kondo CQ363368 PWR部からオブザーバで電源ステータス変化通知を受けた時、復帰要因割り込みを有効にする. 
			if( TYPE_PowerSaveStatusIdle != e_PowerStatus ){ // 待機中以外だった場合に、割り込みを許可にする. 
				// 2014/04/10 toda Helios対応 電源サブCPUの割り込みを許可する.
				THRC_PowerManagerDeviceSingletonIf::newInstance().SwitchPowerState( THRC_PowerDeviceID_SET_INTP5,	THRC_PowerStateID_LOW );	// 原稿セット(EDH).
				THRC_PowerManagerDeviceSingletonIf::newInstance().SwitchPowerState( THRC_PowerDeviceID_SET_INTP6,	THRC_PowerStateID_LOW );	// 原稿セット(DSET).
#if !defined( DEF_DenebMLKCus )	// 2018/05/25 河村- DenebMLK 機器構成変更_No.34 - DenebMLK固有 前ドア開割り込みは有効にしない.
				THRC_PowerManagerDeviceSingletonIf::newInstance().SwitchPowerState( THRC_PowerDeviceID_SET_INTP7,	THRC_PowerStateID_LOW );	// 前カバーの開閉.
#endif // #if !defined( DEF_DenebMLKCus )
				// 2015/05/19 ft_kondo Fax割り込み問題. 
//				THRC_PowerManagerDeviceSingletonIf::newInstance().SwitchPowerState( THRC_PowerDeviceID_SET_KR1,	THRC_PowerStateID_LOW );		// FAXからの起動.
				THRC_PowerManagerDeviceSingletonIf::newInstance().SwitchPowerState( THRC_PowerDeviceID_SET_KR2,	THRC_PowerStateID_LOW );		// MODEM RI信号.
				THRC_PowerManagerDeviceSingletonIf::newInstance().SwitchPowerState( THRC_PowerDeviceID_SET_KR3,	THRC_PowerStateID_LOW );		// PHYからの起動.
/* 2014/09/18 sky_tksk for Fiery省電力 start. */
				if ( TYPE_PowerSaveStatusSubPowerOffDelay == e_PowerStatus || TYPE_PowerSaveStatusSubPowerOff == e_PowerStatus ) {
					THRC_PowerManagerDeviceSingletonIf::newInstance().SwitchPowerState( THRC_PowerDeviceID_SET_KR4,	THRC_PowerStateID_NORMAL );	// Fieryからの起動.
				} else {
					// 2018/07/18 KM(Morikawa) Fiery復帰デバッグ
					if( TYPE_PowerSaveStatusSleepDelay != e_PowerStatus ) {
						THRC_PM_LOG("Fiery復帰要因割り込みON スリープ遅延状態ではない\n");
						THRC_PowerManagerDeviceSingletonIf::newInstance().SwitchPowerState( THRC_PowerDeviceID_SET_KR4,	THRC_PowerStateID_LOW );		// Fieryからの起動.
					} else {
						THRC_PM_LOG("Fiery復帰要因割り込み禁止 スリープ遅延状態\n");
					}	
				}
/* 2014/09/18 sky_tksk for Fiery省電力 end. */
				THRC_PowerManagerDeviceSingletonIf::newInstance().SwitchPowerState( THRC_PowerDeviceID_SET_KR5,	THRC_PowerStateID_LOW );		// 人体検出センサ.
#if 0	// 2013/09/24 堀井 次世代パネル対応_cap1166パネルマイコン制御化.
				setCTSensor( CTSensor_ON );     /* 10/11/25 浦崎 静電センサー対応。 *//* 2011/10/05 第1OSW浦崎 CQ.355237対応。近接センサー有効無効タイミングも合わせて修正 */
#endif
				// 2016/1/14 山口 CQ:523721.
				thePowerManagerTask.set_PowerSavePanel(False);		// 省電力移行完了でパネル電源省電力移行フラグを落とす.
				THRC_PM_LOG("Resume factor interrupt ON");
			} else { // 待機状態への移行の場合は、割り込みを無効にする. 
				// 2014/04/10 toda Helios対応 電源サブCPUの割り込みを無効にする.
				THRC_PowerManagerDeviceSingletonIf::newInstance().SwitchPowerState( THRC_PowerDeviceID_SET_INTP5,	THRC_PowerStateID_NORMAL );	// 原稿セット(EDH).
				THRC_PowerManagerDeviceSingletonIf::newInstance().SwitchPowerState( THRC_PowerDeviceID_SET_INTP6,	THRC_PowerStateID_NORMAL );	// 原稿セット(DSET).
				THRC_PowerManagerDeviceSingletonIf::newInstance().SwitchPowerState( THRC_PowerDeviceID_SET_INTP7,	THRC_PowerStateID_NORMAL );	// 前カバーの開閉.
				THRC_PowerManagerDeviceSingletonIf::newInstance().SwitchPowerState( THRC_PowerDeviceID_SET_KR1,	THRC_PowerStateID_NORMAL );		// FAXからの起動.
				THRC_PowerManagerDeviceSingletonIf::newInstance().SwitchPowerState( THRC_PowerDeviceID_SET_KR2,	THRC_PowerStateID_NORMAL );		// MODEM RI信号.
				THRC_PowerManagerDeviceSingletonIf::newInstance().SwitchPowerState( THRC_PowerDeviceID_SET_KR3,	THRC_PowerStateID_NORMAL );		// PHYからの起動.
/* 2014/09/18 sky_tksk for Fiery省電力 start. */
				THRC_PowerManagerDeviceSingletonIf::newInstance().SwitchPowerState( THRC_PowerDeviceID_SET_KR4,	THRC_PowerStateID_NORMAL );		// Fieryからの起動.
/* 2014/09/18 sky_tksk for Fiery省電力 end. */
				THRC_PowerManagerDeviceSingletonIf::newInstance().SwitchPowerState( THRC_PowerDeviceID_SET_KR5,	THRC_PowerStateID_NORMAL );		// 人体検出センサ.
#if 0	// 2013/09/24 堀井 次世代パネル対応_cap1166パネルマイコン制御化.
				setCTSensor( CTSensor_OFF );     /* 10/11/25 浦崎 静電センサー対応。 *//* 2011/10/05 第1OSW浦崎 CQ.355237対応。近接センサー有効無効タイミングも合わせて修正 */
#endif
				THRC_PM_LOG("Resume factor interrupt OFF");
			}
			break;
		case TYPE_PowerSaveStatusLowPowerIn:							// ローパワー移行中. 
		case TYPE_PowerSaveStatusLowPowerOut:							// ローパワー解除中. 
		case TYPE_PowerSaveStatusSleepIn:								// スリープ移行中. 
		case TYPE_PowerSaveStatusSleepOut:								// スリープ解除中. 
		case TYPE_PowerSaveStatusSubPowerOffProc:						// 副電源OFF処理中. 
		case TYPE_PowerSaveStatusSubPowerOnProc:						// 副電源ON処理中. 
		case TYPE_PowerSaveStatusLowPowerDelayIn:						// ローパワー遅延移行中. 
		case TYPE_PowerSaveStatusLowPowerDelayOut:						// ローパワー遅延解除中. 
		case TYPE_PowerSaveStatusSleepDelayIn:							// スリープ遅延移行中. 
		case TYPE_PowerSaveStatusSleepDelayOut:							// スリープ遅延解除中. 
		case TYPE_PowerSaveStatusSubPowerOffDelayIn:					// 副電源OFF遅延移行中. 
		case TYPE_PowerSaveStatusSubPowerOffDelayOut:					// 副電源OFF遅延解除中. 
		case TYPE_PowerSaveStatusPrtEngOnlyLowPowerCancelProc:			// ローパワープリントエンジン限定解除処理中. 
		case TYPE_PowerSaveStatusPrtEngOnlySleepCancelProc:				// スリーププリントエンジン限定解除処理中. 
		case TYPE_PowerSaveStatusPrtEngOnlyLowPowerCancelPanelOnProc:	// ローパワープリントエンジン限定解除パネルON制御中. 
		case TYPE_PowerSaveStatusPrtEngOnlySleepCancelPanelOnProc:		// スリーププリントエンジン限定解除パネルON制御中. 
			//電源シーケンス開始.
			SYSF_semTake( semPowerStatusChange, WAIT_FOREVER );
			isNowMainPowerStatusChange = True;
			THRC_PM_LOG( "CAP1166 Disabled" );
#ifdef TORNADO
#if 0	// 2013/09/24 堀井 次世代パネル対応_cap1166パネルマイコン制御化.
			cap_i2c_access_off( 1 );
#endif
			// 2012/9/18 堀井 スライドレス_イルミネーション対応.
			if( (e_PowerStatus == TYPE_PowerSaveStatusLowPowerIn) 
				|| (e_PowerStatus == TYPE_PowerSaveStatusSleepIn)
				|| (e_PowerStatus == TYPE_PowerSaveStatusSubPowerOffProc)
				|| (e_PowerStatus == TYPE_PowerSaveStatusLowPowerDelayIn)
				|| (e_PowerStatus == TYPE_PowerSaveStatusSleepDelayIn)
				|| (e_PowerStatus == TYPE_PowerSaveStatusSubPowerOffDelayIn)){
				illumi_i2c_access_off( 1 );	// 省電力移行のみイルミアクセス禁止.
			}
#endif
			SYSF_semGive( semPowerStatusChange );
			break;
		default:
			//制御対象外電源ステータス or 未定義電源ステータス.
			THRC_PM_LOG( "CAP1166 do nothing. e_PowerStatus = [%d]", e_PowerStatus );
			break;
	}
}
#if defined(DEF_Helios_PF) /* ↓S73-BSP_DD-29 */
void THRC_PowerObserver::notifyDeepSleepPCPrintExceptHDD(TYPE_PowerSaveStatus e_PowerStatus, TYPE_PowerWeeklyTimerStatus e_WeeklyTimerStatus)
{
	IMGC_ImageMemoryPoolImpColorFileMemoryInstance& comMemoryManager = IMGC_ImageMemoryPoolImpColorFileMemorySingletonIf::newInstance();
	int32_t si_Errno = 0;	// isLinkedHdd失敗時のerrno取得用.
	// 2015/02/20 ft_kondo CQ.509948 電源キー押下直後にPC FAX投入するとFAX送信しない問題. 
	THRC_PM_LOG( "%"PRId32" : %s call.", SYSF_tickGet(), __FUNCTION__ );
	if( False == isLinkedHdd( &si_Errno ) ){		// HDD認識出来ていれば、SWAPは行わない. 
		// 2015/09/24 CQ519407 sk_iya HDD認識済みではないならミラーリング禁止処理はパスする.
		if ( THRC_PowerStateID_NORMAL !=THRC_PowerManagerDeviceSingletonIf::newInstance().GetPowerState( THRC_PowerDeviceID_FileSystem )) {
			// 2014/08/19 CQ.501018 sky_tksk PC-Printの子ジョブがHDD認識を待たないようにする対応 start.
			// ログを追加.
			THRC_PM_LOG( "HDD mirroring is prohibited" );
			// 2014/08/19 CQ.501018 sky_tksk PC-Printの子ジョブがHDD認識を待たないようにする対応 end.
			comMemoryManager.setIgnoreHddAccess() ;
		} else {
			// isLinkedHdd内のstatエラールート(HDD認識されているのに失敗する).
			THRC_PM_LOG( "No HDD mirroring prohibited stat errno[%"PRId32"]", si_Errno );
		}
	} else {
		// HDD認識.
		THRC_PM_LOG( "HDD recognition OK No mirroring prohibited" );
	}
}
#endif/* ↑S73-BSP_DD-29 */
// ================================================================================================.
//! <!--機能--> スレッド電源制御部 人体検知センサー(静電センサー)のOFF/ON制御.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param int mode ･･･ CTSensor_XXX.
//! <!--戻値-->.
//! \return int ･･･ -1:エラー / -1以外：正常.
//! <!--詳細-->.
//! ioctlをコールし、人体検知センサーをOFForONする。\n.
//! <!--履歴-->.
//  2010/12/01 浦崎 MinervaSoC初版。静電センサー対応.
//  2011/10/13 第1OSW浦崎 「近接センサーの感度設定対応」対応。
//  2012/04/13 第1OSW近藤 CQ.363368 オブザーバクラスに移動. 
//	2013/02/28 Sky 機種共通化対応 THRC_PowerManagerSoC.cppから移植.
// ================================================================================================.
int32_t THRC_PowerObserver::setCTSensor( int32_t mode )
{
    int32_t ret = 0;

    THRC_PM_LOG( "%"PRId32" : setCTSensor. mode[%"PRId32"]", SYSF_tickGet(), mode );

#ifdef TORNADO			// 実機で有効.
	if( -1 == cap1166_fd ) {
		cap1166_fd = SYSF_open( THRD_DeviceFile_Cap1166, O_RDWR, 0666 );
		if( -1 == cap1166_fd ) {
            ret = -1;
            THRC_PM_LOG( "%"PRId32" : cap1166 open error.", SYSF_tickGet() );
            return( ret );
        }
	}
	
    ctsc_ctrl_info.state = 0xff;
	switch(mode)
	{
        case CTSensor_OFF:          /*静電センサーOFF*/
			if( THRC_PowerStateID_LOW != THRC_PowerManagerDeviceSingletonIf::newInstance().GetPowerState( THRC_PowerDeviceID_Panel ) )  {
                ctsc_ctrl_info.state = 0;
            }
            break;
        case CTSensor_ON:           /*静電センサーON*/
			ctsc_ctrl_info.state = 0;
			if( TYPD_ElectrostaticSensorON == APIC_SystemSetting::newInstance()->getElectrostaticSensor() ) {
                ctsc_ctrl_info.state = 1;
            }
            break;
        case CTSensor_ForceOFF:     /*人体検知センサー強制OFF*/
			ctsc_ctrl_info.state = 0;
            break;
        case CTSensor_ForceON:      /*人体検知センサー強制ON*/
			ctsc_ctrl_info.state = 1;
            break;
        default:
        break;
    }
    if( 0xff != ctsc_ctrl_info.state ) {
    	/*2011/10/13 第1OSW浦崎 「近接センサーの感度設定対応」対応。*/
		TYPE_ElectrostaticSensorAdjust e_tmpSensorData = 2;
		switch( APIC_SystemSetting::newInstance()->getElectrostaticSensorAdjust() ) {
            case TYPE_ElectrostaticSensorAdjust_M2 : e_tmpSensorData = 0; break;       //-2.
            case TYPE_ElectrostaticSensorAdjust_M1 : e_tmpSensorData = 1; break;       //-1.
            case TYPE_ElectrostaticSensorAdjust_N  : e_tmpSensorData = 2; break;       // 0.
            case TYPE_ElectrostaticSensorAdjust_P1 : e_tmpSensorData = 3; break;       //+1.
            case TYPE_ElectrostaticSensorAdjust_P2 : e_tmpSensorData = 4; break;       //+2.
            default : e_tmpSensorData = 2;                                break;       //default:0.
        }
		ctsc_ctrl_info.mode = e_tmpSensorData;
		// 2012/12/05 ft_kobayashi 静的解析 CID-436101.
		// 2011/12/19 ft_kondo 静的解析 CID:13064. 
        ret = SYSF_ioctl( cap1166_fd, CAP1166_SET_CTSC, (intptr_t)&ctsc_ctrl_info );
        if( -1 == ret ){
			printf( "(%010u)(BSP) error!! THRC_PowerManager::setCTSensor() SYSF_ioctl() == -1\n", SYSF_tickGet() );
			return -1;
		}
    }
#endif
    return( ret );
}

// ================================================================================================.
//! <!--機能--> 電源管理 制御部 メッセージループ(USB).
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
// 2014/02/19 ft_kondo 新規作成.
// 2016/10/31 sky_ishiga [MinervaMLK][起動時間短縮] スレッド省電力制御の並列制御対応.
// ================================================================================================.
void THRC_PowerManager::messageLoop_USB()
{
	THRS_PowerManagerMsg st_Msg;
	int32_t si_res;

//	thePowerManagerTask.msgQID_PowerManager_USB = SYSF_msgQCreate( MAX_MSGS, sizeof(THRS_PowerManagerMsg), MSG_Q_FIFO );

	while( 1 ) {
		si_res = SYSF_msgQReceive( thePowerManagerTask.msgQID_PowerManager_USB, (char*)&st_Msg, sizeof(st_Msg), WAIT_FOREVER );
		if( si_res == ERROR ) {
#ifdef TORNADO
			continue;
#else
			return;
#endif
		} else {
			switch( st_Msg.us_message )
			{
				case TM_ReqWakeUpUsb:	// USBの復帰. 
					THRC_PM_LOG( "%s [%d]", __FUNCTION__, st_Msg.uc_localChangeStatusAfter );
					thePowerManagerTask.PowerChangeSequence_WakeUpUSB( st_Msg.uc_localChangeStatusAfter );
					break;
				case TM_ReqPowerSaveUsb:	// USBの省電力.
					THRC_PM_LOG( "%s [%d]", __FUNCTION__, st_Msg.uc_localChangeStatusAfter );
					thePowerManagerTask.PowerChangeSequence_PowerSaveUSB( st_Msg.uc_localChangeStatusAfter );
					break;
				default:
					THRC_PM_MSG( "%s unknown msg:%d\n", __FUNCTION__, st_Msg.us_message );
					break;
			}
		}
	}
}

// USBデバイスの復帰処理 PowerChangeSequence()から、処理の一部を移植. 
// 2016/10/31 sky_ishiga [MinervaMLK][起動時間短縮] スレッド省電力制御の並列制御対応 メソッド名を変更.
void THRC_PowerManager::PowerChangeSequence_WakeUpUSB( uchar uc_localChangeStatusAfter )
{
	THRC_PM_LOG( "%s [%d]", __FUNCTION__, uc_localChangeStatusAfter );
	pc_powerDevice->SwitchPowerState( THRC_PowerDeviceID_USB,	PowerStateID( THRC_PowerDeviceID_USB,	uc_localChangeStatusAfter ) );
	// 2015/02/24 ft_kondo initDebugDataDump 記録されない事がある進捗コード修正. 
//	PM_SEQ_REC( 0x47 );		/* CFA14_DetailCode0x0AEE */
	
//	pc_powerDevice->SwitchPowerState( THRC_PowerDeviceID_USB_DOORSENSOR_EN,	PowerStateID( THRC_PowerDeviceID_USB_DOORSENSOR_EN,		uc_localChangeStatusAfter ) );	// 2008/05/29 河村 CQ.304589 - USBフタセンサ対応.
//	PM_SEQ_REC( 0x48 );		/* CFA14_DetailCode0x0AEF */

	sendMessage( TID_PowerManeger, TM_AnsWakeUpComplete, (uintptr_t)THRE_PM_DeviceStatus_USB ); // TASK_PowerManagerにメッセージを投げる. 
}

// ================================================================================================.
//! <!--機能--> 電源管理 制御部 USBデバイスの省電力移行制御.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param uc_localChangeStatusAfter 遷移先ステータス.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! USBデバイスの省電力移行制御\n.
//! <!--履歴-->.
// 2016/10/31 sky_ishiga [MinervaMLK][起動時間短縮] スレッド省電力制御の並列制御対応 新規作成.
// ================================================================================================.
void THRC_PowerManager::PowerChangeSequence_PowerSaveUSB( uchar uc_localChangeStatusAfter )
{
	THRC_PM_LOG( "%s [%d]", __FUNCTION__, uc_localChangeStatusAfter );
	pc_powerDevice->SwitchPowerState( THRC_PowerDeviceID_USB,	PowerStateID( THRC_PowerDeviceID_USB,	uc_localChangeStatusAfter ));

	sendMessage( TID_PowerManeger, TM_AnsPowerSaveComplete, THRE_PM_DeviceStatus_USB ); // TASK_PowerManagerにメッセージを投げる. 
}

// ================================================================================================.
//! <!--機能--> 電源管理 制御部 ウォッチドッグ定期起床(Panel).
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
// 2014/05/13 ft_horii 新規作成.
// ================================================================================================.
void THRC_PowerManager::Watch_PanelTimer( void )
{
	THRS_PowerManagerMsg st_Msg;
	st_Msg.us_message = TM_TimerWakeUp_Panel;
	SYSF_msgQSend( thePowerManagerTask.msgQID_PowerManager_Panel, (char*)&st_Msg, sizeof(st_Msg), WAIT_FOREVER, MSG_PRI_NORMAL );
}

	
#define BUZZER_TIMEOUT 12000	// ブザーのメッセージ送信のタイムアウト時間.
// ================================================================================================.
//! <!--機能--> 電源管理 制御部 Buzzer指示.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
// 2014/05/14 ft_horii 新規作成.
// ================================================================================================.
void THRC_PowerManager::sendSetPanelBuzzer( BUZZER_SETTING str_Buzzer )
{
	THRS_PowerManagerMsg st_Msg;
	st_Msg.us_message = TM_ReqPanelBuzzerOnOff;
	st_Msg.str_buzzerSetting = str_Buzzer;
	// 2019/01/15 AR.1071400 山口 メッセージキューにブザーのメッセージ送信のタイムアウト時間を設定.
	SYSF_msgQSend( thePowerManagerTask.msgQID_PowerManager_Panel, (char*)&st_Msg, sizeof(st_Msg), BUZZER_TIMEOUT, MSG_PRI_NORMAL );
}
// ================================================================================================.
//! <!--機能--> 電源管理 制御部 LED指示.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
// 2014/05/14 ft_horii 新規作成.
// ================================================================================================.
void THRC_PowerManager::sendSetPanelLed( LED_SETTING str_Led )
{
	THRS_PowerManagerMsg st_Msg;
	st_Msg.us_message = TM_ReqPanelLEDOnOff;
	st_Msg.str_ledSetting = str_Led;
	SYSF_msgQSend( thePowerManagerTask.msgQID_PowerManager_Panel, (char*)&st_Msg, sizeof(st_Msg), WAIT_FOREVER, MSG_PRI_NORMAL );
}

// ================================================================================================.
//! <!--機能--> 電源管理 制御部 ペアリングデータ転送指示.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
// 2014/09/8 ft_yamaguchi 新規作成.
// ================================================================================================.
void THRC_PowerManager::sendSetPairingData( uintptr_t args_resultCallback )
{
	THRS_PowerManagerMsg st_Msg;
	st_Msg.us_message = TM_ReqPairingDataTrans;
	st_Msg.resultCallback = args_resultCallback;
	SYSF_msgQSend( thePowerManagerTask.msgQID_PowerManager_Panel, (char*)&st_Msg, sizeof(st_Msg), WAIT_FOREVER, MSG_PRI_NORMAL );
}

// ================================================================================================.
//! <!--機能--> 電源管理 制御部 パネル電源ON指示.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
// 2017/09/26 河村 新規作成.
// ================================================================================================.
void THRC_PowerManager::panelPowerOn( void )
{
	THRS_PowerManagerMsg st_Msg;
	st_Msg.us_message = TM_ReqPanelPowerOn;
	SYSF_msgQSend( thePowerManagerTask.msgQID_PowerManager_Panel, (char*)&st_Msg, sizeof(st_Msg), WAIT_FOREVER, MSG_PRI_NORMAL );
}

// ================================================================================================.
//! <!--機能--> 電源管理 制御部 省電力中にパネルOnフラグ取得.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return 省電力中にパネルOnフラグ.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
// 2015/11/9 山口 新規作成.
// ================================================================================================.
Bool THRC_PowerManager::get_reqPanelOn(void)
{
	return bl_reqPanelOn;
}

// ================================================================================================.
//! <!--機能--> 電源管理 制御部 パネル電源省電力移行フラグ設定.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param True/False 移行中/移行中でない.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
// 2016/1/14 山口 新規作成 CQ:523721.
// ================================================================================================.
void THRC_PowerManager::set_PowerSavePanel(Bool flg){
	
	bl_PowerSavePanel = flg;
	
}
// ================================================================================================.
//! <!--機能--> 電源管理 制御部 パネル電源省電力移行フラグ取得.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return True/False 移行中/移行中でない.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
// 2016/1/14 山口 新規作成 CQ:523721.
// ================================================================================================.
Bool THRC_PowerManager::get_PowerSavePanel(void){
	
	return bl_PowerSavePanel;
	
}

// ================================================================================================.
//! <!--機能--> 主電源OFF検知用フラグ設定.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param True/False 主電源OFF/主電源OFFでない.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
// 2017/4/26 Sky-NAKANISI 新規作成 AR1019633 主電源OFF後に前ドア開閉するとMFPパネルが起動する問題の対応.
// ================================================================================================.
void THRC_PowerManager::set_MainSwOFF_Flag(Bool flg){
	
	bl_MainSwOFF_Flag = flg;
	
}
// ================================================================================================.
//! <!--機能--> 主電源OFF検知用フラグ取得.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return True/False 主電源OFF/主電源OFFでない.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
// 2017/4/26 Sky-NAKANISI 新規作成 AR1019633 主電源OFF後に前ドア開閉するとMFPパネルが起動する問題の対応.
// ================================================================================================.
Bool THRC_PowerManager::get_MainSwOFF_Flag(void){
	
	return bl_MainSwOFF_Flag;
	
}

// ================================================================================================.
//! <!--機能--> 電源管理 制御部 スナップショット取得可能通知.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! スナップショット取得可能状態を通知する\n.
//! <!--履歴-->.
// 2016/11/28 Sky-Matsuoka 新規作成 起動時間短縮対応.
// 2016/11/30 tadamasa HW検証で、SuperWarp起動後、主電源OFFの割り込みが入って来ない問題対応.
// 2016/12/14 AccuRev.1009544 ioctrlの戻り値の扱い問題.
// 2017/10/18 Sky-ishiga [Eagle] [起動時間短縮(再設計)] インターフェース見直し(固有名称(電源SubCPU)を削除).
// ================================================================================================.
void THRC_PowerManager::SnapshotEnabled(void)
{
	Bool bl_res = False;
	if(si_Fd < 0){
		si_Fd = SYSF_open( THRD_DeviceFile_Pmu, O_RDWR, 0 );
		if( si_Fd < 0 ) {
			THRC_PM_ASSERT("Cannot open file pmuWakeupEventInterruptEnable THRD_DeviceFile_Pmu", 0,0,0,0,CFA14_DetailCode0x1091 );
		}
	}
	//復帰要因割り込みを許可にする.
	//2016/12/14 AccuRev.1009544 ioctl戻り値扱い問題.
	bl_res = ioctl( si_Fd, THRD_PMU_IOCSPWRSUBINT, 1 );
	if( bl_res < 0 ) {
		THRC_PM_ASSERT("pmu ioctl(THRD_PMU_IOCSPWRSUBINT) error.", 0,0,0,0,CFA14_DetailCode0x1092 );
	}
	bl_res = THRC_PowerManagerDeviceSingletonIf::newInstance().SwitchPowerState( THRC_PowerDeviceID_SET_MAINSW,	THRC_PowerStateID_LOW );	// 主電源割り込みを許可する.
	if (!bl_res) {
		THRC_PM_LOG( "%s THRC_PowerDeviceID_SET_MAINSW NG return\n", __FUNCTION__ );
	}
	if (bl_res) {
		bl_res = THRF_setPowerOffDelay( True );	// ディレイスイッチをオンにする(電源通電状態).
		if (!bl_res) {
			THRC_PM_LOG( "%s THRF_setPowerOffDelay(True) NG return\n", __FUNCTION__ );
		}
	}
}

// ================================================================================================.
//! <!--機能--> 電源管理 制御部 主電源リレー(ディレイ)スイッチオフ要求
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//!	電源OFF遅延をオフにする指示を出す\n.
//! 本APIをCallした場合そのままリターンしてこない\n.
//! <!--履歴-->.
// 2016/11/28 Sky-Matsuoka 新規作成 起動時間短縮対応.
// 2017/10/18 Sky-ishiga [Eagle] [起動時間短縮(再設計)] インターフェース見直し(固有名称(電源SubCPU)を削除).
// 2017/11/08 ishiga [Eagle][起動時間短縮(再設計)] ターミナルログが途切れる問題の対応.
// 2017/11/08 Sky 通電OFF前にSyncを実行する対応.
// ================================================================================================.
void THRC_PowerManager::DelaySwitchOFF(void)
{
	sync();		/* sync */
	printf( "(%010u)(BSP) %s sync exec\n", SYSF_tickGet() , __FUNCTION__ );

	THRF_waitForTermLogOutput();	// ターミナルログ出力を待つ.

	volatile Bool bl_res = False;
	bl_res = THRF_setPowerOffDelay( False );	// 電源OFF遅延をオフにする.
	if (!bl_res) {
		THRC_PM_LOG( "%s THRF_setPowerOffDelay(False) NG return\n", __FUNCTION__ );
	}
	while (True) {
		bl_res = False;
	}
}

// ================================================================================================.
//! <!--機能--> 電源管理 制御部 メッセージループ(Panel).
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
// 2014/02/19 ft_kondo 新規作成.
// 2014/05/14 ft_horii 省電力中のブザー対応 省電力復帰だけでなく、移行やブザー、LEDの制御を行う.
// ================================================================================================.
#define DEF_PM_PANEL_TIMER 100	// 定期起床時間.
void THRC_PowerManager::messageLoop_Panel()
{
	THRS_PowerManagerMsg st_Msg;
	int32_t si_res;

//	thePowerManagerTask.msgQID_PowerManager_Panel = SYSF_msgQCreate( MAX_MSGS, sizeof(THRS_PowerManagerMsg), MSG_Q_FIFO );

	while( 1 ) {
		si_res = SYSF_msgQReceive( thePowerManagerTask.msgQID_PowerManager_Panel, (char*)&st_Msg, sizeof(st_Msg), WAIT_FOREVER );
		if( si_res == ERROR ) {
#ifdef TORNADO
			continue;
#else
			return;
#endif
		} else {
			thePowerManagerTask.PowerChangeSequence_Panel( st_Msg );
		}
	}
}

#define LCDC100_MEMCPYPCITOVRAM			(10001)		// LCDデータの読み込み（PCI→VRAM）. 

#define DEF_PM_SHM_OPEN_NAME "/panel_snapshotimgbuf"
// 2015/1/15 UI2スケーラブル対応 MSE 諸岡 Add Start.
#define DEF_PM_SNAPSHOT_IMGSIZE(w,h) (w)*(h)*2
// 2015/1/15 UI2スケーラブル対応 MSE 諸岡 Add End.
// パネルデバイスの復帰処理 PowerChangeSequence()から、処理の一部を移植. 
// 2014/10/9 堀井 CQ.504507 IMGSYS→PANEL_POWERに変更.
#define DEF_PM_PANEL_ON_TIMER_TICK		(TICKS_PER_SEC/2)		// PWRから電源ステート変更が来るまでのタイムアウト:1/2秒.

void THRC_PowerManager::PowerChangeSequence_Panel( THRS_PowerManagerMsg st_Msg )
{
	static uchar uc_NextStatus =  THRE_PowerSaveStatusNone;
	static ushort us_dirActive = 0;
//	static Bool bl_reqPanelOn = False;
	static WDOG_ID		wdID_local = NULL;
	PNL_OUTSTATE st_pnlstate;
//	LED_SETTING st_led;
	CNVC_PanelHard*	ppc_PanelHard;
	// 2014/07/10 ft_kondo 制御構成変更No142 省電力復帰時間短縮対応. 
	// 2016/07/08 Sky-Araki 省電力復帰時間短縮.
	THRC_PowerStateID e_nowStatePanel = THRC_PowerStateID_NONE;
// 2014/10/29 ADS成田 CQ.506206 SIM大量デバッグ不具合対応の為
#ifdef SIMULATOR
	memset(&st_pnlstate, 0, sizeof(PNL_OUTSTATE));
#endif

	if( semID_PanelPower == NULL ) {
		semID_PanelPower = SYSF_semMCreate( SEM_Q_PRIORITY | SEM_INVERSION_SAFE );
		SYSF_semTake(semID_PanelPower, WAIT_FOREVER );
	}
	
	if( wdID_local == NULL ){
		wdID_local = SYSF_wdCreate();
	}
	
	switch( st_Msg.us_message )
	{
		case TM_ReqWakeUpPanel:	// Panelの復帰. 
		case TM_ReqPowerSavePanel:	// Panelの移行. 
			THRC_PM_LOG( "%s [%d]", __FUNCTION__, st_Msg.uc_localChangeStatusAfter );
			// 2017/02/20 Sky酒井（純）ユーザーイベント記録(APIコール).
			SYSF_ReqTaskMonitorUserEvent(18000, 20, (uint32_t)st_Msg.uc_localChangeStatusAfter, 0);
			// 省電力移行先と省電力方向を保存する.
			uc_NextStatus = st_Msg.uc_localChangeStatusAfter;
			us_dirActive = st_Msg.us_message;
			// 定期起床で移行復帰を行う.
			st_Msg.us_message = TM_TimerWakeUp_Panel;
			// 2016/1/14 山口 CQ:523721.
			// 省電力移行指示の場合省電力移行中フラグを立てる.
			if(us_dirActive == TM_ReqPowerSavePanel ){
				bl_PowerSavePanel = True;
			}
			SYSF_msgQSend( thePowerManagerTask.msgQID_PowerManager_Panel, (char*)&st_Msg, sizeof(st_Msg), WAIT_FOREVER, MSG_PRI_NORMAL );
			bl_isPanelOnTimer = False;	// 電源ステート変更が来たのでタイマは停止する.
			break;
		case TM_ReqPanelLEDOnOff:		// cap1166のLED制御.
		case TM_ReqPanelBuzzerOnOff:	// ブザー制御.
		case TM_ReqPanelCTSensorOff:	// センサーオフ.
		case TM_ReqPairingDataTrans:	// ペアリングデータ転送.		2014/09/08 FT_yamaguchi NFC対応(ZeusS).
		case TM_ReqPanelPowerOn:		// 2017/09/25 河村 - Eagle省電力復帰時間短縮対応 パネル電源ON要求.
			if( THRC_PowerStateID_NORMAL != pc_powerDevice->GetPowerState( THRC_PowerDeviceID_PANEL_POWER ) ){
				// パネルの電源が落ちている場合には電源を入れる.
				bl_reqPanelOn = True;
				SYSF_semTake(semID_PanelPower, WAIT_FOREVER );
				pc_powerDevice->SwitchPowerState( THRC_PowerDeviceID_PANEL_POWER,	THRC_PowerStateID_NORMAL );
				// 2022/08/24 OP_BTS-39242 山口 省電力中にネットワークの初期化完了でNFCデバイスエラー.	
				if(st_Msg.us_message == TM_ReqPairingDataTrans){
					SYSF_taskDelay(SYSF_sysClkRateGet() * 20 / 1000);	// 20ms wait.
				}
				
			}
			if( TM_ReqPanelBuzzerOnOff == st_Msg.us_message ){
				// ブザー制御を行う.
				sysBspIoctlPanelSio(&st_Msg.str_buzzerSetting, EXDB_PanelKindSetBuzzer);
			}else if( TM_ReqPanelLEDOnOff == st_Msg.us_message ){
				// LED制御を行う.
				sysBspIoctlPanelSio(&st_Msg.str_ledSetting, EXDB_PanelKindSetLedExtend);
			}else if( TM_ReqPairingDataTrans == st_Msg.us_message ){
				// ペアリングデータ転送を行う.	2014/09/08 FT_yamaguchi NFC対応(ZeusS).
				CNVC_PanelHard::TASK_transNFCPairingData(st_Msg.resultCallback);
			}else if( TM_ReqPanelPowerOn == st_Msg.us_message ){
				// 一定期間パネル電源ONする(タイマーをスタートする).
				bl_isPanelOnTimer = True;
				ul_PanelOnTimer = SYSF_tickGet();	// パネル電源ONのtickを記録.
				THRC_PM_LOG( "%"PRId32" : %s Resume factor generation panel Power on.", SYSF_tickGet(), __FUNCTION__ );
			}else{
				// 割込み解除のため、パネルを一度起こしセンサーを無効にする 2013/08/08 堀井 次世代パネル対応_cap1166パネルマイコン制御化.
				ppc_PanelHard = CNVC_PanelHard::newInstance();	// パネルハード取得.
				if( ppc_PanelHard ) {
					ppc_PanelHard->setCTSensor( False );	// センサー無効.
				}
			}
			// 定期起床で電源操作を行う.
			st_Msg.us_message = TM_TimerWakeUp_Panel;
			SYSF_msgQSend( thePowerManagerTask.msgQID_PowerManager_Panel, (char*)&st_Msg, sizeof(st_Msg), WAIT_FOREVER, MSG_PRI_NORMAL );
			break;
		case TM_ReqMainSwOFFPanelSeq:	// 主電源OFFパネルシーケンス通知.	/* 2018/11/30 Harada AR.1068362 SuperWarp起動後、黒画面一瞬表示不具合対応 */
			if( THRC_PowerStateID_NORMAL != pc_powerDevice->GetPowerState( THRC_PowerDeviceID_Panel ) ){
				// パネルの電源が落ちている場合には電源を入れる.
				// 省電力状態からの主電源OFFではステートがLOW→LOWで電源ステート遷移指示処理が行われないため、.
				// 電源ステート遷移指示処理を行うことができるように一度電源を入れる.
				SYSF_semTake(semID_PanelPower, WAIT_FOREVER );
				pc_powerDevice->SwitchPowerState( THRC_PowerDeviceID_PANEL_POWER,	THRC_PowerStateID_NORMAL );
				pc_powerDevice->SwitchPowerState( THRC_PowerDeviceID_Panel,			THRC_PowerStateID_NORMAL );
			}
			st_Msg.us_message = TM_ReqPowerSavePanel;	// Panelの移行. 
			SYSF_msgQSend( thePowerManagerTask.msgQID_PowerManager_Panel, (char*)&st_Msg, sizeof(st_Msg), WAIT_FOREVER, MSG_PRI_NORMAL );
			break;
		case TM_TimerWakeUp_Panel:	// 定期起床.
			if( us_dirActive == TM_ReqWakeUpPanel ){
				// 省電力復帰する.
				e_nowStatePanel = pc_powerDevice->GetPowerState( THRC_PowerDeviceID_Panel );	// 現在の状態を取得.				
				if ( THRC_PowerStateID_LOW == pc_powerDevice->GetPowerState( THRC_PowerDeviceID_PANEL_POWER )
				&& ( THRC_PowerStateID_NORMAL == PowerStateID( THRC_PowerDeviceID_Panel, uc_NextStatus ) ) ){
					SYSF_semTake(semID_PanelPower, WAIT_FOREVER );
					PM_SEQ_REC( 0x6c );		/* CFA14_DetailCode0x0B13 */ // kondo_新規. 
					pc_powerDevice->SwitchPowerState( THRC_PowerDeviceID_PANEL_POWER, PowerStateID( THRC_PowerDeviceID_Panel,			uc_NextStatus ) );
					PM_SEQ_REC( 0x6d );		/* CFA14_DetailCode0x0B14 */ // kondo_新規. 
				}
				PM_SEQ_REC( 0x43 );		/* CFA14_DetailCode0x0AEA */
				pc_powerDevice->SwitchPowerState( THRC_PowerDeviceID_Panel,			PowerStateID( THRC_PowerDeviceID_Panel,			uc_NextStatus ) );
				PM_SEQ_REC( 0x44 );		/* CFA14_DetailCode0x0AEB */

				// パネルが復帰し、最初の画面表示した時のみ、お待ちください画面を表示させる.
				if( (THRC_PowerStateID_LOW == e_nowStatePanel) 												&& 
				    (THRC_PowerStateID_NORMAL == pc_powerDevice->GetPowerState(THRC_PowerDeviceID_Panel))	) {

#if defined(TORNADO) && !defined(FUM_LINUX) 
					// お待ちください画面があるかどうかを判断する. 
					// パネルの用意した「お待ちください画面」を表示する処理. 
					if( True == APIC_SystemSetting::newInstance()->getWaitScreenExist() ){ // お待ちください画面があるかチェック. 
					// 2015/1/15 UI2スケーラブル対応 MSE 諸岡 Add Start.
						// 2015/3/20 UI2スケーラブル対応 MSE 諸岡 Add Start.
#ifdef SIMULATOR
						// Simでの/dev/fb0の値は、VirtualBoxの設定値であり、参照しないのが方針となる.
						// その為以下の/dev/fb0を参照するのは実機のみとなる.
						// ただ、zse3(ZeusS)環境にて、Simは[defined(TORNADO) && !defined(FUM_LINUX) ]は有効ではない為問題ないが.
						// 環境によってはSimでも有効なルートとなりうる為、その場合に検出できるようログを残す.
						printf("%s /dev/fb0 読み込み制限 [%d]\n", __FUNCTION__, errno );
#else
						//++ 2019/04/24 Sky		【EagleH】Warning除去対応.
						int32_t i32_Ret;
						void* addr = NULL;
						int	si_fb0_fd;
						char* psc_fb_addr = NULL;
						//-- 2019/04/24 Sky		【EagleH】Warning除去対応.

						// 2015/1/15 UI2スケーラブル対応 MSE 諸岡 Add Start.
						// 2014/10/22 ft_kondo CQ.502417 お待ちください画面が表示されない問題対策. 
						// FrameBuffer open. 
						si_fb0_fd = SYSF_devopen( "/dev/fb0", O_RDWR, 0 );
						if( si_fb0_fd == -1 ){
							printf( "%s FrameBuffer devicefile open error [%d]\n", __FUNCTION__, errno );
							return;
						}
						PM_SEQ_REC( 0x6e );		/* CFA14_DetailCode0x0B15 */ // kondo_新規. 

						// 解像度取得.
						struct fb_var_screeninfo vscr_info;
						if (ioctl(si_fb0_fd, FBIOGET_VSCREENINFO, &vscr_info) != 0 ){
							SYSF_close( (int)si_fb0_fd );
							printf( "(%010u)(BSP) %s FrameBuffer ioctl error [%d]\n", SYSF_tickGet() , __FUNCTION__, errno );
							return;
						}
						ushort resx = vscr_info.xres;
						ushort resy = vscr_info.yres;
						uint32_t	imagesize = DEF_PM_SNAPSHOT_IMGSIZE(resx,resy);
						printf( "(%010u)(BSP) [PowerManager] x:%d y:%d\n", SYSF_tickGet() ,resx,resy);
						// 2015/1/15 UI2スケーラブル対応 MSE 諸岡 Add End.
						
						// 共有メモリからお待ちください画面を取得. 
						i32_Ret = shm_open( DEF_PM_SHM_OPEN_NAME, O_RDONLY, 0666 );
						THRL_ThAssert( ( -1 != i32_Ret ), "[%s] 共有メモリ[%s]open失敗!!", (uintptr_t)__FUNCTION__ , (uintptr_t)DEF_PM_SHM_OPEN_NAME, 0, 0, CFA14_DetailCode0x0B0F ); 
						// 2015/1/15 UI2スケーラブル対応 MSE 諸岡 Add Start.
						addr = mmap(0, imagesize, PROT_READ, MAP_SHARED, i32_Ret, 0);
						// 2015/1/15 UI2スケーラブル対応 MSE 諸岡 Add End.
						
						THRL_ThAssert( ( addr != MAP_FAILED ), "[%s] 共有メモリ[%s]mmap失敗!!", (uintptr_t)__FUNCTION__ , (uintptr_t)DEF_PM_SHM_OPEN_NAME, 0, 0, CFA14_DetailCode0x0B10 ); 
						PM_SEQ_REC( 0x6f );		/* CFA14_DetailCode0x0B16 */ // kondo_新規. 

						// FrameBufferのメモリマッピング.
						// 2015/1/15 UI2スケーラブル対応 MSE 諸岡 Add Start.
						psc_fb_addr = (char*)mmap( NULL, imagesize, PROT_READ | PROT_WRITE, MAP_SHARED, si_fb0_fd, 0);
						// 2015/1/15 UI2スケーラブル対応 MSE 諸岡 Add End.
						
						if(MAP_FAILED == psc_fb_addr) {
							// 2015/1/15 UI2スケーラブル対応 MSE 諸岡 Add Start.
							munmap( addr, imagesize );
							// 2015/1/15 UI2スケーラブル対応 MSE 諸岡 Add End.
							
							SYSF_close( i32_Ret );
							SYSF_close( (int)si_fb0_fd );
							printf( "(%010u)(BSP) %s FrameBuffer mmap error [%d]\n", SYSF_tickGet() , __FUNCTION__, errno );
							return;
						}
						PM_SEQ_REC( 0x70 );		/* CFA14_DetailCode0x0B17 */ // kondo_新規. 

						// 共有メモリ内のお待ちください画面データを、FrameBufferへコピーする. 
						// 2015/1/15 UI2スケーラブル対応 MSE 諸岡 Add Start.
						memcpy( psc_fb_addr, addr, imagesize);
						// 2015/1/15 UI2スケーラブル対応 MSE 諸岡 Add End.
						
						msync( psc_fb_addr, imagesize, MS_SYNC);
						
						// 2015/1/15 UI2スケーラブル対応 MSE 諸岡 Add Start.
						munmap( addr, imagesize );
						// 2015/1/15 UI2スケーラブル対応 MSE 諸岡 Add End.
						SYSF_close( i32_Ret );

						// FrameBufferからVRAMへ転送. 
						if ( ioctl( si_fb0_fd, LCDC100_MEMCPYPCITOVRAM , NULL ) ){
							printf( "(%010u)(BSP) %s FrameBuffer LCDC100_MEMCPYPCITOVRAM error [%d]\n", SYSF_tickGet() , __FUNCTION__, errno );
							// 2015/1/15 UI2スケーラブル対応 MSE 諸岡 Add Start.
							munmap(psc_fb_addr, imagesize);
							// 2015/1/15 UI2スケーラブル対応 MSE 諸岡 Add End.
							SYSF_close( (int)si_fb0_fd );
							return;
						}
						PM_SEQ_REC( 0x71 );		/* CFA14_DetailCode0x0B18 */ // kondo_新規. 

						// 2015/1/15 UI2スケーラブル対応 MSE 諸岡 Add Start.
						munmap(psc_fb_addr, imagesize);
						// 2015/1/15 UI2スケーラブル対応 MSE 諸岡 Add End.
						
						SYSF_close( (int)si_fb0_fd );
#endif // #if !SIMULATOR.
					// 2015/3/20 UI2スケーラブル対応 MSE 諸岡 Add End.
					}
#endif // #if defined(TORNADO) && !defined(FUM_LINUX). 

					// 2017/02/22 SuperWarp起動中の場合はAsic復帰通知の前にパネル電源ONを通知する(Panel点灯遅くなる問題対応).
					// 2018/08/06 ishiga SuperWarp起動中判断について適切なAPIを使用するように修正.
					if (APIC_Power::newInstance()->isSuperWarpFirstBootConfiguration() == False) {
						// 2016/07/8 Sky-Araki 省電力復帰時間短縮. 
						THRC_PM_LOG("sendMessage( TID_MachineMan, TM_PanelPowerOn, 0 );");
						sendMessage( TID_MachineMan, TM_PanelPowerOn, 0 );
					}
				}

				sendMessage( TID_PowerManeger, TM_AnsWakeUpComplete, THRE_PM_DeviceStatus_Panel ); // TASK_PowerManagerにメッセージを投げる. 					break;
				uc_NextStatus = THRE_PowerSaveStatusNone;
				us_dirActive = 0;
				bl_reqPanelOn = False;
			}
			sysBspIoctlPanelSio(&st_pnlstate, EXDB_PanelDataGet);
			if( st_pnlstate.BuzzerOnOffState != DEF_BUZ_CMD_BUZZERCTL_OFF ){
				// ブザー鳴動中であるため、100ms後にチェックする.
				SYSF_wdStart( wdID_local, SYSF_sysClkRateGet() * DEF_PM_PANEL_TIMER/1000, (FUNCPTR)THRC_PowerManager::Watch_PanelTimer, 0 );	// 定期起床タイマ起動.
				break;
			}
			if( st_pnlstate.AuthDataTrans == True ){
				// 省電力移行中に認証情報が転送された場合100ms後にチェックする.
				SYSF_wdStart( wdID_local, SYSF_sysClkRateGet() * DEF_PM_PANEL_TIMER/1000, (FUNCPTR)THRC_PowerManager::Watch_PanelTimer, 0 );	// 定期起床タイマ起動.
				break;
			}
			if( bl_isPanelOnTimer == True ) {
				if( !time_after(  SYSF_tickGet(), ul_PanelOnTimer+DEF_PM_PANEL_ON_TIMER_TICK ) ) {
					// タイマー動作中でまだ規定時間に達していない場合100msにチェックする.
					SYSF_wdStart( wdID_local, SYSF_sysClkRateGet() * DEF_PM_PANEL_TIMER/1000, (FUNCPTR)THRC_PowerManager::Watch_PanelTimer, 0 );	// 定期起床タイマ起動.
					break;
				} else {
					// 規定時間でタイマーを停止する.
					bl_isPanelOnTimer = False;
				}
			}
			if( us_dirActive == TM_ReqPowerSavePanel ){
				// 省電力移行させる.
				pc_powerDevice->SwitchPowerState( THRC_PowerDeviceID_Panel,			PowerStateID( THRC_PowerDeviceID_Panel,			uc_NextStatus ) );
				PM_SEQ_REC( 0x08 );		/* CFA14_DetailCode0x0AAF */
				// 2017/5/29 sky_Nakanisi AR1022867【システム品証PreFC評価】【SYSQA-IT5_4.1-003】主電源OFFでパネル消灯後しばらく電源ボタン青点灯.
				if ( False == get_MainSwOFF_Flag() ) {
					if ( THRC_PowerStateID_NORMAL == pc_powerDevice->GetPowerState( THRC_PowerDeviceID_PANEL_POWER ) 
					   && ( THRC_PowerStateID_LOW == PowerStateID( THRC_PowerDeviceID_Panel, uc_NextStatus ) ) ) {
						PM_SEQ_REC( 0x72 );		/* CFA14_DetailCode0x0B19 */ // kondo_新規. 
						pc_powerDevice->SwitchPowerState( THRC_PowerDeviceID_PANEL_POWER,	PowerStateID( THRC_PowerDeviceID_Panel,			uc_NextStatus ) );
						PM_SEQ_REC( 0x73 );		/* CFA14_DetailCode0x0B1A */ // kondo_新規. 
						SYSF_semGive( semID_PanelPower );
					} 
				}
				else {
					SYSF_semGive( semID_PanelPower );
				}
				PM_SEQ_REC( 0x09 );		/* CFA14_DetailCode0x0AB0 */
				uc_NextStatus = THRE_PowerSaveStatusNone;
				us_dirActive = 0;
				bl_reqPanelOn = False;
				// 2016/10/31 sky_ishiga [MinervaMLK][起動時間短縮] スレッド省電力制御の並列制御対応.
				sendMessage( TID_PowerManeger, TM_AnsPowerSaveComplete, THRE_PM_DeviceStatus_Panel ); // TASK_PowerManagerにメッセージを投げる. 
			}
			if( bl_reqPanelOn ){
				// 省電力中にパネルOnしたため、パネルOffする.
				if( THRC_PowerStateID_LOW != pc_powerDevice->GetPowerState( THRC_PowerDeviceID_PANEL_POWER ) ){
					// 2016/03/07 CQ.525843 山口. 
					// 省電力中のパネル電源ON/OFFは省電力シーケンスと関係ない処理のため進捗コードを残す処理を削除する(0x74,0x75).
					// 2018/07/03 AR.1043719 省電力中に静電センサーで復帰時ない.
					pc_powerDevice->SwitchPowerState( THRC_PowerDeviceID_PANEL_POWER,	THRC_PowerStateID_LOW );
					SYSF_semGive( semID_PanelPower );
				}
				bl_reqPanelOn = False;
			}
			break;
		default:
			THRC_PM_MSG( "%s unknown msg:%d\n", __FUNCTION__, st_Msg.us_message );
			break;
	}
}
// ================================================================================================.
//! <!--機能--> 電源管理 制御部 メッセージループ(HDD).
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
// 2014/02/19 ft_kondo 新規作成.
// 2015/02/04 河村 CQ.509407 - C-D050誤検出問題対応/タイムアウト時間を10秒から90秒に変更.
// 2016/10/31 sky_ishiga [MinervaMLK][起動時間短縮] スレッド省電力制御の並列制御対応.
// ================================================================================================.
uint32_t ul_PM_HDD_TimeOut = 90*200;				// タイムアウト時間 90秒. 
void THRC_PowerManager::messageLoop_HDD()
	{
	THRS_PowerManagerMsg st_Msg;
	int32_t si_res;

	while( 1 ) {
		si_res = SYSF_msgQReceive( thePowerManagerTask.msgQID_PowerManager_Hdd, (char*)&st_Msg, sizeof(st_Msg), WAIT_FOREVER );
		if( si_res == ERROR ) {
#ifdef TORNADO
			continue;
#else
			return;
#endif
		} else {
			switch( st_Msg.us_message )
			{
				case TM_ReqWakeUpHdd:	// HDDの復帰. 
					ul_PM_HDD_tickGetTime1 = SYSF_tickGet();	// HDD復帰処理開始の時間を取得. 
					THRC_PM_LOG( "%"PRId32" : HDD connection completion timeout %"PRId32"[tick]", SYSF_tickGet(), ul_PM_HDD_tickGetTime1 + ul_PM_HDD_TimeOut );
					// 2014/09/26 ft_kondo CQ.504373 省電力移行復帰を繰り返していると、省電力移行出来無くなる問題. 
					if( st_Msg.b_NormalShift ){						// FileSystemの制御が必要な場合のみ、HDD処理中フラグを立てる. 
						b_doPowerChangeSequence_Hdd = True;			// HDD復帰処理中フラグを立てる. 
					}
					THRC_PM_LOG( "%s [%d] [%d] [%"PRId32"]", __FUNCTION__, st_Msg.uc_localChangeStatusAfter, st_Msg.e_sequenceID, st_Msg.ul_nextParam );
					thePowerManagerTask.PowerChangeSequence_WakeUpHdd( st_Msg.uc_localChangeStatusAfter, st_Msg.e_sequenceID, st_Msg.ul_nextParam, st_Msg.b_NormalShift );
//					b_doPowerChangeSequence_Hdd = False;
					break;
				case TM_ReqPowerSaveHdd:	// HDDの省電力.
					THRC_PM_LOG( "%s [%d]", __FUNCTION__, st_Msg.uc_localChangeStatusAfter );
					thePowerManagerTask.PowerChangeSequence_PowerSaveHdd( st_Msg.uc_localChangeStatusAfter );
					break;
				default:
					THRC_PM_MSG( "%s unknown msg:%d\n", __FUNCTION__, st_Msg.us_message );
					break;
			}
		}
	}
}

// 2016/10/31 sky_ishiga [MinervaMLK][起動時間短縮] スレッド省電力制御の並列制御対応 メソッド名を変更.
void THRC_PowerManager::PowerChangeSequence_WakeUpHdd( uchar uc_localChangeStatusAfter, THRE_PowerSequenceID e_sequenceID, uint32_t ul_nextParam, Bool b_NormalShift )
{
	THRC_PM_LOG( "%s [%d] [%d] [%"PRId32"]", __FUNCTION__, uc_localChangeStatusAfter, e_sequenceID, ul_nextParam );
	// 2010/08/20 GaiaPF8536対応 河村 - 復帰時間のかかるHDDを優先.
	bl_isDelayedSequence = False;

	if( pc_powerDevice->GetPowerState( THRC_PowerDeviceID_HDD ) != THRC_PowerStateID_NOT_EXIST_DEVICE ) {
		// HDDが存在する場合はHDD接続要求が発生する.
		bl_isHddConnectComplete = False;
		bl_isLowToNormalComplete = False;
		// 2014/04/11 ft_kondo Zeus5BK 省電力復帰時間短縮対応. 
		// 2010/02/23 ft_kondo 第6回手動マージ. 
		PM_SEQ_REC( 0x2e );		/* CFA14_DetailCode0x0AD5 */
		pc_powerDevice->SwitchPowerState( THRC_PowerDeviceID_HDD_DRIVER,	PowerStateID( THRC_PowerDeviceID_HDD_DRIVER,	uc_localChangeStatusAfter ) );
		PM_SEQ_REC( 0x2f );		/* CFA14_DetailCode0x0AD6 */

		// HDDが存在しておりLowからNormalへの遷移が発生する場合.
		// HDD_DRIVER制御後、PM_HDD_RESUME_WAIT[ms]後にFileSystemを制御する.
		if( b_NormalShift ) {
			DelayedPowerChangeSequence( (uint32_t)THRD_HDD_RESUME_WAIT, e_sequenceID, 4, ul_nextParam );
			bl_isDelayedSequence = True;
			PM_SEQ_REC( 0x30 );		/* CFA14_DetailCode0x0AD7 */
			char c=1; initDebugWrite( initDebugModule_THR, 6, &c, 1);	/*HDD接続要求*/
		} else {
			/* 2016/04/19 CQ527820 Altantic省電力問題No6. 既に削除されているWdタイマは削除しない */
			if (si_taskID_PM_Watch_HDD) {
				// 2015/03/04 ft_kondo TASK_PM_Watch_HDDタスクが生成されたまま残る事がある問題. 
				SYSF_taskDelete( si_taskID_PM_Watch_HDD );
				si_taskID_PM_Watch_HDD = 0;
				// 2016/06/06 CQ530712 unknown wdId問題対応、再Delete/Cancelできないようにゼロクリアする。
				if (wdID_Watch_HDD) {
					SYSF_wdCancel( wdID_Watch_HDD );			// タイマーの停止. 
					SYSF_wdDelete( wdID_Watch_HDD );			// タイマーの削除. 
					wdID_Watch_HDD = 0;
				}
			}
			// 2015/01/16 Harada CQ.509010 省電力制御パラレル対応に合わせた省電力シーケンス記録フラグ対応.
			e_PM_SeqReqEndComplete &= ~THRE_PM_SeqReqEndComplete_HDD;				// 復帰完了したデバイスのフラグを落とす.
			if (THRE_PM_SeqReqEndComplete_Complete == e_PM_SeqReqEndComplete) {		// 省電力シーケンスの最後の処理かどうか.
				PM_SEQ_REC_END();	// 2014/10/16 ft_kondo ここが省電力シーケンスの最後の処理となった場合. 
			}
		}
	} else {
		bl_isHddConnectComplete = True;
		PM_SEQ_REC( 0x31 );		/* CFA14_DetailCode0x0AD8 */
		/* 2016/04/19 CQ527820 Atlantic省電力問題No6. 既に削除されているWdタイマは削除しない */
		if (si_taskID_PM_Watch_HDD) {
			// 2014/07/25 ft_kondo CQ.501828 HDDの接続完了まで、次の省電力シーケンスは動かさない対応. 
			SYSF_taskDelete( si_taskID_PM_Watch_HDD );
			si_taskID_PM_Watch_HDD = 0;
			// 2016/06/06 CQ530712 unknown wdId問題対応、再Delete/Cancelできないようにゼロクリアする。
			if (wdID_Watch_HDD) {
				SYSF_wdCancel( wdID_Watch_HDD );			// タイマーの停止. 
				SYSF_wdDelete( wdID_Watch_HDD );			// タイマーの削除. 
				wdID_Watch_HDD = 0;
			}
		}
		b_doPowerChangeSequence_Hdd = False; // もしここが呼ばれた場合は、ココでフラグを落とす. 
	}
//	b_doPowerChangeSequence_Hdd = False;
	sendMessage( TID_PowerManeger, TM_AnsWakeUpComplete, (uintptr_t)THRE_PM_DeviceStatus_Hdd ); // TASK_PowerManagerにメッセージを投げる. 
}

// ================================================================================================.
//! <!--機能--> 電源管理 制御部 HDDの省電力移行制御.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param uc_localChangeStatusAfter 遷移先ステータス.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! HDDの省電力移行制御\n.
//! <!--履歴-->.
// 2016/10/31 sky_ishiga [MinervaMLK][起動時間短縮] スレッド省電力制御の並列制御対応 新規作成.
// ================================================================================================.
void THRC_PowerManager::PowerChangeSequence_PowerSaveHdd( uchar uc_localChangeStatusAfter )
{
	THRC_PM_LOG( "%s [%d]", __FUNCTION__, uc_localChangeStatusAfter );
	pc_powerDevice->SwitchPowerState( THRC_PowerDeviceID_FileSystem, PowerStateID( THRC_PowerDeviceID_FileSystem, uc_localChangeStatusAfter ) );
	pc_powerDevice->SwitchPowerState( THRC_PowerDeviceID_HDD_DRIVER, PowerStateID( THRC_PowerDeviceID_HDD_DRIVER, uc_localChangeStatusAfter ) );
	sendMessage( TID_PowerManeger, TM_AnsPowerSaveComplete, THRE_PM_DeviceStatus_Hdd ); // TASK_PowerManagerにメッセージを投げる. 
}

// ================================================================================================.
//! <!--機能--> 電源管理 制御部 メッセージループ(IR).
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
// 2014/02/19 ft_kondo 新規作成.
// 2016/10/31 sky_ishiga [MinervaMLK][起動時間短縮] スレッド省電力制御の並列制御対応.
// ================================================================================================.
void THRC_PowerManager::messageLoop_IR()
{
	THRS_PowerManagerMsg st_Msg;
	int32_t si_res;

//	thePowerManagerTask.msgQID_PowerManager_IR = SYSF_msgQCreate( MAX_MSGS, sizeof(THRS_PowerManagerMsg), MSG_Q_FIFO );

	while( 1 ) {
		si_res = SYSF_msgQReceive( thePowerManagerTask.msgQID_PowerManager_IR, (char*)&st_Msg, sizeof(st_Msg), WAIT_FOREVER );
		if( si_res == ERROR ) {
#ifdef TORNADO
			continue;
#else
			return;
#endif
		} else {
			switch( st_Msg.us_message )
			{
				case TM_ReqWakeUpIR:	// IRの復帰. 
					THRC_PM_LOG( "%s [%d]", __FUNCTION__, st_Msg.uc_localChangeStatusAfter );
					thePowerManagerTask.PowerChangeSequence_WakeUpIR( st_Msg.uc_localChangeStatusBefore, st_Msg.uc_localChangeStatusAfter ); // 2015/10/09 CQ.519888 河村 - Before追加.
					break;
				case TM_ReqPowerSaveIR:	// IRの省電力移行.
					THRC_PM_LOG( "%s [%d]", __FUNCTION__, st_Msg.uc_localChangeStatusAfter );
					thePowerManagerTask.PowerChangeSequence_PowerSaveIR( st_Msg.uc_localChangeStatusAfter );
					break;
				default:
					THRC_PM_MSG( "%s unknown msg:%d\n", __FUNCTION__, st_Msg.us_message );
					break;
			}
		}
	}
}

// 2015/09/16 CQ.519888 河村 - IR制御タイミングをずらしてENG制御と被らないようにする対応(MinervaSBK/ZeusS_ZX0のDeepSleep復帰限定).
#define IrWait (400/5)

// 2016/06/09 井柳 CQ.530875（元CQ.530313マージ） ネガポジ問題. 
uint32_t ul_PM_ENG_POWER_MON_WAIT = 1000;			// タイムアウト時間 10s. 
uint32_t ul_Delay_TM_IRDevicePowerSaveCancel = 0;	// wait時間 0s. 

// 2016/10/31 sky_ishiga [MinervaMLK][起動時間短縮] スレッド省電力制御の並列制御対応 メソッド名の変更.
void THRC_PowerManager::PowerChangeSequence_WakeUpIR( uchar uc_localChangeStatusBefore, uchar uc_localChangeStatusAfter )
{
	THRC_PM_LOG( "%s [%PRId32]", __FUNCTION__, uc_localChangeStatusAfter );
	// 2015/09/16 CQ.519888 河村 - IR制御タイミングをずらしてENG制御と被らないようにする対応(MinervaSBK/ZeusS_ZX0のDeepSleep復帰限定) 「Sleep1.5->待機中」 に限定.
	if( uc_localChangeStatusBefore == THRE_PowerSaveStatusSleep15 && uc_localChangeStatusAfter == THRE_PowerSaveStatusIdle ) {
		THRC_PM_LOG( "%"PRId32" : %s wait start.", SYSF_tickGet(), __FUNCTION__ );
		SYSF_taskDelay( IrWait );
		THRC_PM_LOG( "%"PRId32" : %s wait end.", SYSF_tickGet(), __FUNCTION__ );
	}
	pc_powerDevice->SwitchPowerState( THRC_PowerDeviceID_CPU_FAN,		PowerStateID( THRC_PowerDeviceID_CPU_FAN,		uc_localChangeStatusAfter ) );
	PM_SEQ_REC( 0x2c );		/* CFA14_DetailCode0x0AD3 */
	pc_powerDevice->SwitchPowerState( THRC_PowerDeviceID_IR,			PowerStateID( THRC_PowerDeviceID_IR,			uc_localChangeStatusAfter ) );
	// 2015/02/24 ft_kondo initDebugDataDump 記録されない事がある進捗コード修正. 
//	PM_SEQ_REC( 0x2d );		/* CFA14_DetailCode0x0AD4 */
	// 2016/06/09 井柳 CQ.530875（元CQ.530313マージ） ネガポジ問題. 
	if( isLowToNormalOfDevice( THRC_PowerDeviceID_IR ) ) { 
		uint32_t ul_LoopCnt = 0;
		while( pc_powerDevice->GetPowerState( THRC_PowerDeviceID_ENG_POWER_MON ) == THRC_PowerStateID_LOW ){ // 24V11をモニタをチェックして、LowならHiになるまで待つ. 
			SYSF_taskDelay(SYSF_sysClkRateGet() * 10 / 1000);	// 10ms wait.
			if( ul_LoopCnt > ul_PM_ENG_POWER_MON_WAIT ){ // 10s待っても終わらなかったら、処理を抜ける. 
				THRC_PM_LOG( "%d : %s ENG_POWER_MON(Low→High) wait timeout.", SYSF_tickGet(), __FUNCTION__ );
				break;
			}
			ul_LoopCnt++;
		}
		// IRを起こしたメッセージ. 
		SYSF_taskDelay( ul_Delay_TM_IRDevicePowerSaveCancel ); // IR_P_ON、24V12がHiになった後、3.3V-1がOnになるまで時間がかかるため、将来的にwaitを入れる可能性有りのため、waitを追加しておく(現状は0s). 
		THRC_PM_LOG( "%d : %s sendMessage( TID_MachineMan, TM_IRDevicePowerSaveCancel, 0 ); Loop[%lu]", SYSF_tickGet(), __FUNCTION__, ul_LoopCnt );
		sendMessage( TID_MachineMan, TM_IRDevicePowerSaveCancel, 0 );
		PM_SEQ_REC( 0x48 );		/* CFA14_DetailCode0x0AEF */
	}

	sendMessage( TID_PowerManeger, TM_AnsWakeUpComplete, (uintptr_t)THRE_PM_DeviceStatus_IR ); // TASK_PowerManagerにメッセージを投げる. 
}

// ================================================================================================.
//! <!--機能--> 電源管理 制御部 IRの省電力移行制御.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param uc_localChangeStatusAfter 遷移先ステータス.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! IRの省電力移行制御\n.
//! <!--履歴-->.
// 2016/10/31 sky_ishiga [MinervaMLK][起動時間短縮] スレッド省電力制御の並列制御対応 新規作成.
// ================================================================================================.
void THRC_PowerManager::PowerChangeSequence_PowerSaveIR( uchar uc_localChangeStatusAfter )
{
	THRC_PM_LOG( "%s [%d]", __FUNCTION__, uc_localChangeStatusAfter );
	pc_powerDevice->SwitchPowerState( THRC_PowerDeviceID_IR, PowerStateID( THRC_PowerDeviceID_IR, uc_localChangeStatusAfter ) );

	// 電源管理にIR省電力終了通知を送信.
	THRC_PM_LOG( "sendMessage( TID_MachineMan, TM_IRPowerSaveEndNotice, 0 );" );
	sendMessage( TID_MachineMan, TM_IRPowerSaveEndNotice, 0 );

	sendMessage( TID_PowerManeger, TM_AnsPowerSaveComplete, THRE_PM_DeviceStatus_IR ); // TASK_PowerManagerにメッセージを投げる.
}

// ================================================================================================.
//! <!--機能--> 電源管理 制御部 メッセージループ(Fiery).
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
// 2014/02/19 ft_kondo 新規作成.
// 2016/10/31 sky_ishiga [MinervaMLK][起動時間短縮] スレッド省電力制御の並列制御対応.
// ================================================================================================.
void THRC_PowerManager::messageLoop_Fiery()
{
	THRS_PowerManagerMsg st_Msg;
	int32_t si_res;

//	thePowerManagerTask.msgQID_PowerManager_Fiery = SYSF_msgQCreate( MAX_MSGS, sizeof(THRS_PowerManagerMsg), MSG_Q_FIFO );

	while( 1 ) {
		si_res = SYSF_msgQReceive( thePowerManagerTask.msgQID_PowerManager_Fiery, (char*)&st_Msg, sizeof(st_Msg), WAIT_FOREVER );
		if( si_res == ERROR ) {
#ifdef TORNADO
			continue;
#else
			return;
#endif
		} else {
			switch( st_Msg.us_message )
			{
				case TM_ReqWakeUpFiery:	// Fieryの復帰. 
					THRC_PM_LOG( "%s [%d]", __FUNCTION__, st_Msg.uc_localChangeStatusAfter );
					thePowerManagerTask.PowerChangeSequence_WakeUpFiery( st_Msg.uc_localChangeStatusAfter );
					break;
				case TM_ReqPowerSaveFiery:	// Fieryの省電力移行.
					THRC_PM_LOG( "%s [%d]", __FUNCTION__, st_Msg.uc_localChangeStatusAfter );
					thePowerManagerTask.PowerChangeSequence_PowerSaveFiery( st_Msg.uc_localChangeStatusAfter );
					break;
				default:
					THRC_PM_MSG( "%s unknown msg:%d\n", __FUNCTION__, st_Msg.us_message );
					break;
			}
		}
	}
}

// 2016/10/31 sky_ishiga [MinervaMLK][起動時間短縮] スレッド省電力制御の並列制御対応 メソッド名変更.
void THRC_PowerManager::PowerChangeSequence_WakeUpFiery( uchar uc_localChangeStatusAfter )
{
	THRC_PM_LOG( "%s [%d]", __FUNCTION__, uc_localChangeStatusAfter );
	pc_powerDevice->SwitchPowerState( THRC_PowerDeviceID_Fiery,			PowerStateID( THRC_PowerDeviceID_Fiery,			uc_localChangeStatusAfter ) );
	// 2015/02/24 ft_kondo initDebugDataDump 記録されない事がある進捗コード修正. 
//	PM_SEQ_REC( 0x46 );		/* CFA14_DetailCode0x0AED */

	sendMessage( TID_PowerManeger, TM_AnsWakeUpComplete, (uintptr_t)THRE_PM_DeviceStatus_Fiery ); // TASK_PowerManagerにメッセージを投げる. 

}

// ================================================================================================.
//! <!--機能--> 電源管理 制御部 Fieryの省電力移行制御.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param uc_localChangeStatusAfter 遷移先ステータス.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! Fieryの省電力移行制御\n.
//! <!--履歴-->.
// 2016/10/31 sky_ishiga [MinervaMLK][起動時間短縮] スレッド省電力制御の並列制御対応 新規作成.
// ================================================================================================.
void THRC_PowerManager::PowerChangeSequence_PowerSaveFiery( uchar uc_localChangeStatusAfter )
{
	THRC_PM_LOG( "%s [%d]", __FUNCTION__, uc_localChangeStatusAfter );
	pc_powerDevice->SwitchPowerState( THRC_PowerDeviceID_Fiery_PT_T,	PowerStateID( THRC_PowerDeviceID_Fiery_PT_T,	uc_localChangeStatusAfter ) );
	pc_powerDevice->SwitchPowerState( THRC_PowerDeviceID_Fiery,			PowerStateID( THRC_PowerDeviceID_Fiery,			uc_localChangeStatusAfter ) );

	sendMessage( TID_PowerManeger, TM_AnsPowerSaveComplete, THRE_PM_DeviceStatus_Fiery ); // TASK_PowerManagerにメッセージを投げる. 
}

// ================================================================================================.
//! <!--機能--> 電源管理 制御部 メッセージループ(PCIePCIDev).
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
// 2014/02/19 ft_kondo 新規作成.
//	2014/08/05 ADS八木 CQ.500724(Act_501780) 副電源OFF時PCPrint受信異常画像対応.
//  2014/12/05 ADS吉田 CQ.506801(Act_504828) AS-FSC_14 A3A4共通化(機能スケーラビリティ拡張).
//                     ⇒機能オプション「TYPE_AppOptionSubMemSplitSave」を以下機能別に分離.
//                       ・TYPE_AppOptionSubMemSplitSave ：サブメモリ可変化[1ページ確保動作]、スワップ分割保存・分割読出.
//                       ・TYPE_AppOptionSubMemPartition ：サブメモリ割り当て切り替え対応.
//                       ・TYPE_AppOptionSubFileMem      ：サブメモリのファイルメモリ化.
//                       ・TYPE_AppOptionWorstSize       ：最悪圧縮率対応.
// 2016/10/31 sky_ishiga [MinervaMLK][起動時間短縮] スレッド省電力制御の並列制御対応.
// ================================================================================================.
void THRC_PowerManager::messageLoop_PCIePCIDev()
{
	THRS_PowerManagerMsg st_Msg;
	int32_t si_res;

//	thePowerManagerTask.msgQID_PowerManager_PCIePCIDev = SYSF_msgQCreate( MAX_MSGS, sizeof(THRS_PowerManagerMsg), MSG_Q_FIFO );

	// 2017/11/02 Harada 新規作成 AR.1029174 新詳細コード改善 SegF付加情報対応.
	// コールバック関数登録API.
	TYPE_FatalCallBackResult e_FatalCallBackResult = TYPE_FatalCallBackResult_OK;							// Result.
	TYPE_TaskDetailCode e_TaskDetailCode = TYPE_TaskDetailCode_TASK_PM_PCIe_PCIDev;							// タスク番号.
	TYPD_CallBackFunc pf_CallBackFunc = (TYPD_CallBackFunc)THRC_PowerManager::fatalCallBack_PCIePCIDev;		// コールバック関数のポインタ.
	TYPE_FatalCallBackPriority e_Priority = TYPE_FatalCallBackPriority_Owner;								// プライオリティ.
	e_FatalCallBackResult = APIC_setFatalCallBack( e_TaskDetailCode, pf_CallBackFunc, e_Priority );
	if (TYPE_FatalCallBackResult_OK != e_FatalCallBackResult) {
		printf("## %s %d : APIC_setFatalCallBack NG  e_FatalCallBackResult=%d\n", __FUNCTION__, __LINE__, e_FatalCallBackResult );
	}

	while( 1 ) {
		si_res = SYSF_msgQReceive( thePowerManagerTask.msgQID_PowerManager_PCIePCIDev, (char*)&st_Msg, sizeof(st_Msg), WAIT_FOREVER );
		if( si_res == ERROR ) {
#ifdef TORNADO
			continue;
#else
			return;
#endif
		} else {
			switch( st_Msg.us_message )
			{
				case TM_ReqWakeUpPCIe_PCIDev:	// PCIデバイスの復帰. 
					THRC_PM_LOG( "%s [%d] [%d]", __FUNCTION__, st_Msg.uc_localChangeStatusAfter, st_Msg.bl_isSdramClearComplete );
					thePowerManagerTask.PowerChangeSequence_WakeUpPCIePCIDev( st_Msg.uc_localChangeStatusAfter, st_Msg.bl_isSdramClearComplete );
					break;
				case TM_ReqPowerSavePCIe_PCIDev:	// PCIデバイスの省電力移行. 
					THRC_PM_LOG( "%s [%d]", __FUNCTION__, st_Msg.uc_localChangeStatusAfter );
					thePowerManagerTask.PowerChangeSequence_PowerSavePCIePCIDev( st_Msg.uc_localChangeStatusAfter );
					break;
				default:
					THRC_PM_MSG( "%s unknown msg:%d\n", __FUNCTION__, st_Msg.us_message );
					break;
			}
		}
	}
}

// 2016/10/31 sky_ishiga [MinervaMLK][起動時間短縮] スレッド省電力制御の並列制御対応 メソッド名変更.
void THRC_PowerManager::PowerChangeSequence_WakeUpPCIePCIDev( uchar uc_localChangeStatusAfter, Bool bl_isSdramClearComplete )
{
	THRC_PM_LOG( "%s [%d] [%d]", __FUNCTION__, uc_localChangeStatusAfter, bl_isSdramClearComplete );
	// 2010/02/23 ft_kondo 第6回手動マージ. 
	// 2008/06/17 河村 CQ.305256 - 復帰時間のかかるIRC(CPSグループに含まれる)を優先する.
	PM_SEQ_REC( 0x32 );		/* CFA14_DetailCode0x0AD9 */
//	pc_powerDevice->SwitchPowerState( THRC_PowerDeviceID_CPS,			PowerStateID( THRC_PowerDeviceID_CPS,			uc_localChangeStatusAfter ) );
	PM_SEQ_REC( 0x38 );		/* CFA14_DetailCode0x0ADF */
	pc_powerDevice->SwitchPowerState( THRC_PowerDeviceID_PCIe_BUS,		PowerStateID( THRC_PowerDeviceID_PCIe_BUS,		uc_localChangeStatusAfter ) );
	PM_SEQ_REC( 0x39 );		/* CFA14_DetailCode0x0AE0 */
	// 音声IC対応 2014/9/5 小林 サウンドデバイス追加.
	pc_powerDevice->SwitchPowerState( THRC_PowerDeviceID_SOUND,			PowerStateID( THRC_PowerDeviceID_SOUND,			uc_localChangeStatusAfter ) );
	// 2017/4/24 堀井 AR.1020061 省電力復帰時に音が鳴らない問題.
	if( pc_powerDevice->GetPowerState( THRC_PowerDeviceID_SOUND ) == THRC_PowerStateID_NORMAL ){
		THRC_PM_LOG( "sendMessage( TID_MachineMan, TM_SoundICPowerOn, 0 );" );
		sendMessage( TID_MachineMan, TM_SoundICPowerOn, 0 ); // 音声IC復帰通知. 
	}
	PM_SEQ_REC( 0x3a );		/* CFA14_DetailCode0x0AE1 */
//	pc_powerDevice->SwitchPowerState( THRC_PowerDeviceID_PIC,			PowerStateID( THRC_PowerDeviceID_PIC,			uc_localChangeStatusAfter ) );
	// -- ↑ここまでPCI_BUS, CPS/PIC OKをPWRへ通知 
	PM_SEQ_REC( 0x3b );		/* CFA14_DetailCode0x0AE2 */

	// 2014/12/18 ft_kondo CQ.508228 PIC省電力解除要求を常に投げるように修正. 
	// BSPからは常にPIC省電力解除通知を送信し、電源部にてPICリセットを行うかどうか、必要に応じて判断する. 
	// PICを起こしたメッセージ.
	THRC_PM_LOG( "sendMessage( TID_MachineMan, TM_PicDevicePowerSaveCancel, 0 );" );
	sendMessage( TID_MachineMan, TM_PicDevicePowerSaveCancel, 0 );
	PM_SEQ_REC( 0x3c );		/* CFA14_DetailCode0x0AE3 */
//	PM_SEQ_REC( 0x3d );		/* CFA14_DetailCode0x0AE4 */

	// 2014/11/27 ft_kondo CQ.507269 ENG限定解除でプリント中に、待機状態へ遷移するとPIC/CPSの復帰メッセージを投げてしまう件. 
	// CPSはASIC777に統合されているため、IRの復帰が完了したタイミングで、CPS省電力解除通知を電源部に投げる. 
	if( isLowToNormalOfDevice( THRC_PowerDeviceID_IR ) ) { 
		// CPSを起こしたメッセージ.
		THRC_PM_LOG( "sendMessage( TID_MachineMan, TM_CpsDevicePowerSaveCancel, 0 );" );
		sendMessage( TID_MachineMan, TM_CpsDevicePowerSaveCancel, 0 );
		PM_SEQ_REC( 0x3e );		/* CFA14_DetailCode0x0AE5 */
	}

//	PM_SEQ_REC( 0x3f );		/* CFA14_DetailCode0x0AE6 */
	// 2014/10/08 ft_kondo CQ.500724 副電源OFF中にPCプリントすると、異常画像となる問題の修正. 
	if( isLowToNormalOfDevice( THRC_PowerDeviceID_PCIe_BUS ) ) {
		// 2009/07/15 河村 CQ319244 - PCIe復帰の場合はASIC200 SDRAMのクリアDMAを起動する.
		bl_isSdramClearComplete = False;
		PM_SEQ_REC( 0x40 );		/* CFA14_DetailCode0x0AE7 */
		// 2014/08/05 ADS八木 CQ.500724(Act_501780) メモリ上の画像を削除する.
#ifdef DEF_A3A4COMM  // 2015/02/16 ADS吉田 CQ.510156(Act_507195) 【A3A4共通化】コンパイルスイッチ対応(機能/メモリスケーラビリティ拡張).
		if( True == SYSF_getAppOptionStatus(TYPE_AppOptionSubFileMem) ) {	// 機能判定.
#else
		if( True == SYSF_getAppOptionStatus(TYPE_AppOptionSubMemSplitSave) ) {	// 機能判定.
#endif
			IMGC_ImageMemoryPoolImpColorFileMemoryInstance& comMemoryManager = IMGC_ImageMemoryPoolImpColorFileMemorySingletonIf::newInstance();
			comMemoryManager.sweepMemoryAll();
		}
#if defined(DEF_S800)
		THRC_BufMemorySingletonIf::newInstance().AllClearMemory();
#else /* defined(DEF_S800) */
		THRC_BufMemorySingletonIf::newInstance().AllClearSDRAM();
#endif /* defined(DEF_S800) */
		char c=1; initDebugWrite( initDebugModule_THR, 2, &c, 1);	/*ASIC200 SDRAMクリア開始*/
		PM_SEQ_REC( 0x41 );		/* CFA14_DetailCode0x0AE8 */
	} else {
		bl_isSdramClearComplete = True;
		// 2015/01/12 Harada CQ.506130、CQ.508744、CQ.508749 SDRAMクリア完了も待つ対象とする.
		sendMessage( TID_PowerManeger, TM_AnsWakeUpComplete, (uintptr_t)THRE_PM_DeviceStatus_SDRAM ); // TASK_PowerManagerにメッセージを投げる. 
	}

	// 2010/02/23 ft_kondo 第6回手動マージ. 
//	PM_SEQ_REC( 0x43 );		/* CFA14_DetailCode0x0AEA */

	sendMessage( TID_PowerManeger, TM_AnsWakeUpComplete, (uintptr_t)THRE_PM_DeviceStatus_PCIePCIDev ); // TASK_PowerManagerにメッセージを投げる. 

}

// ================================================================================================.
//! <!--機能--> 電源管理 制御部 PCIe PCIDevの省電力移行制御.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param uc_localChangeStatusAfter 遷移先ステータス.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! PCIe PCIDevryの省電力移行制御\n.
//! <!--履歴-->.
// 2016/10/31 sky_ishiga [MinervaMLK][起動時間短縮] スレッド省電力制御の並列制御対応 新規作成.
// ================================================================================================.
void THRC_PowerManager::PowerChangeSequence_PowerSavePCIePCIDev( uchar uc_localChangeStatusAfter )
{
	THRC_PM_LOG( "%s [%d]", __FUNCTION__, uc_localChangeStatusAfter );
	pc_powerDevice->SwitchPowerState( THRC_PowerDeviceID_PCIe_BUS,		PowerStateID( THRC_PowerDeviceID_PCIe_BUS, uc_localChangeStatusAfter ) );
	pc_powerDevice->SwitchPowerState( THRC_PowerDeviceID_SOUND,			PowerStateID( THRC_PowerDeviceID_SOUND, uc_localChangeStatusAfter ) );
	pc_powerDevice->SwitchPowerState( THRC_PowerDeviceID_CPU_FAN,		PowerStateID( THRC_PowerDeviceID_CPU_FAN, uc_localChangeStatusAfter ) );
	pc_powerDevice->SwitchPowerState( THRC_PowerDeviceID_IMGSYS,		PowerStateID( THRC_PowerDeviceID_IMGSYS, uc_localChangeStatusAfter ) );

// 電源OFF移行(Snapshot取得)時は、ASICの電源を落とさない.
//		pc_powerDevice->SwitchPowerState( THRC_PowerDeviceID_PEXPD,			PowerStateID( THRC_PowerDeviceID_PEXPD, uc_localChangeStatusAfter ) );

	sendMessage( TID_PowerManeger, TM_AnsPowerSaveComplete, THRE_PM_DeviceStatus_PCIePCIDev ); // TASK_PowerManagerにメッセージを投げる. 
}

// ================================================================================================.
//! <!--機能--> 電源管理 制御部 HDD復帰時間監視処理.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
// 2014/04/14 ft_kondo 新規作成.
// 2015/02/04 河村 CQ.509407 - C-D050誤検出問題対応.
// 2015/04/02 山口 CQ 511642 D050の詳細コード登録.
// 2017/03/28 PS1-ft_kobayashi AR1010867 C-D050発生時調査用ログを取得しeMMCに保存する.
// ================================================================================================.
#define DEF_PM_WATCH_HDD_TIMER 500
#define DEF_PM_MESSAGE_WATCH_HDD_TIMER 1
void THRC_PowerManager::messageLoop_Watch_HDD()
{
	THRS_PowerManagerMsg st_Msg;
	int32_t si_res;
	wdID_Watch_HDD = SYSF_wdCreate();
	SYSF_wdStart( wdID_Watch_HDD, SYSF_sysClkRateGet() * DEF_PM_WATCH_HDD_TIMER/1000, (FUNCPTR)THRC_PowerManager::Watch_HDDTimer, 0 );	// 定期起床タイマ起動.

//	thePowerManagerTask.msgQID_PowerManager_Watch_Hdd = SYSF_msgQCreate( MAX_MSGS, sizeof(THRS_PowerManagerMsg), MSG_Q_FIFO );

	while( 1 ) {
		si_res = SYSF_msgQReceive( thePowerManagerTask.msgQID_PowerManager_Watch_Hdd, (char*)&st_Msg, sizeof(st_Msg), WAIT_FOREVER );
		if( si_res == ERROR ) {
#ifdef TORNADO
			continue;
#else
			return;
#endif
		} else {
			switch( st_Msg.us_message )
			{
				case DEF_PM_MESSAGE_WATCH_HDD_TIMER:				// 定期起床. 
					if( True == b_doPowerChangeSequence_Hdd ){		// HDD復帰処理中かどうか. 
						ul_PM_HDD_tickGetTime2 = SYSF_tickGet();	// 現在の時間を取得. 
						if( time_after( ul_PM_HDD_tickGetTime2, ul_PM_HDD_tickGetTime1 + ul_PM_HDD_TimeOut ) ){
							// 10s以上経過しても処理が完了していなかったら、トラブルコード：C-D050を発生させる. 
							THRC_PM_MSG( "%s timeout発生!!", __FUNCTION__ );
							APIC_SystemStatus* pc_SystemStatus = APIC_SystemStatus::newInstance();
							pc_SystemStatus->setFatal( TYPD_TroubleCode_D050 );	// C-D050発生. 
							// 2015/10/21 Harada 新詳細コード対応 >>.
#if defined(TORNADO) && !defined(FUM_LINUX) /*実機*/
//							THRL_ThAssertLog( CFA14_DetailCode0x4006 );			// C-D050詳細コード登録.
							StartTmB_SMP();	//タスクモニタ取得.
							SYSF_taskDelay(SYSF_sysClkRateGet() * 50 / 1000);	// 50ms wait.
							StopTm();
							system("dmesg");
							pmLogPrint();
							Save_PsInfo();	// 調査用ログを取得.
extern Bool bl_HDDendSleepOK;		// HDDスリープ解除OK.
extern Bool bl_endSleepHDDInit;		// スリープ解除時のHDD初期化実行処理状態.
							unsigned short us_DetailCode = CFA14_DetailCode0x4006;
							uint64_t ull_NewDetailCode = 0x0;
							Bool bl_ret = True;
							THRC_PowerManagerDevice *pc_powerDevice = &THRC_PowerManagerDeviceSingletonIf::newInstance();	// デバイス制御部インスタンス.
							THRC_PowerStateID uc_PowerState_HDD = pc_powerDevice->GetPowerState( THRC_PowerDeviceID_HDD );	// 現在の電源ステート取得 HDD.
							IMGC_ImageMemoryPoolImpColorFileMemoryInstance& comMemoryManager = IMGC_ImageMemoryPoolImpColorFileMemorySingletonIf::newInstance();
							ull_NewDetailCode = (((uint64_t)0x22                                               << 56) & 0xff00000000000000)		// モジュール番号.
											  | (((uint64_t)(True == isLinkedHdd() ? 1 : 0)                    << 55) & 0x0080000000000000)		// HDD認識.
											  | (((uint64_t)(True == bl_isHddConnectComplete ? 1 : 0)          << 54) & 0x0040000000000000)		// HDD接続完了.
											  | (((uint64_t)(True == bl_isLowToNormalComplete ? 1 : 0)         << 53) & 0x0020000000000000)		// 省電力シーケンス完了.
											  | (((uint64_t)(True == bl_isDelayedSequence ? 1 : 0)             << 52) & 0x0010000000000000)		// 遅延制御要否.
											  | (((uint64_t)(True == bl_HDDendSleepOK ? 1 : 0)                 << 51) & 0x0008000000000000)		// HDDスリープ解除OK.
											  | (((uint64_t)(True == bl_endSleepHDDInit ? 1 : 0)               << 50) & 0x0004000000000000)		// スリープ解除時のHDD初期化実行処理状態.
											  | (((uint64_t)uc_PowerState_HDD                                  << 46) & 0x0003C00000000000)		// 現在の電源ステート取得 HDD.
											  | (((uint64_t)theHDDTrans.getHDDSeq()                            << 39) & 0x00003f8000000000)		// ＨＤＤ転送シーケンス取得.
											  | (((uint64_t)(True == theHDDTrans.isCheckFormatError() ? 1 : 0) << 38) & 0x0000004000000000)		// フォーマット異常.
											  | (((uint64_t)(True == theHDDTrans.isHddFwAreaError() ? 1 : 0)   << 37) & 0x0000002000000000)		// HDDのFW領域の異常状態取得.
											  | (((uint64_t)(True == theHDDTrans.isNotFormatted() ? 1 : 0)     << 36) & 0x0000001000000000)		// 未フォーマットチェック.
											  | (((uint64_t)(True == theHDDTrans.isCommandExec() ? 1 : 0)      << 35) & 0x0000000800000000)		// コマンド実行中.
											  | (((uint64_t)(True == theHDDTrans.isUmountError() ? 1 : 0)      << 34) & 0x0000000400000000)		// umountエラーが発生しているかどうかを判定.
											  | (((uint64_t)comMemoryManager.getHDDStatus( THRE_StorageDeviceTypeHDD )                 << 31) & 0x0000000380000000)		// ＨＤＤのステータス.
											  | (((uint64_t)(True == comMemoryManager.isHDDReady( THRE_StorageDeviceTypeHDD ) ? 1 : 0) << 30) & 0x0000000040000000)		// ＨＤＤは使用可能か？状態変化.
											  | (((uint64_t)(True == FILC_FsControlIf::isFileSystemInitComp() ? 1 : 0)                 << 29) & 0x0000000020000000)		// FsControlの終了を返す（HDDのマウント、Docの初期化、DataBaseの初期化）.
											  | (((uint64_t)(True == FILC_FsControlIf::getExistHddDevFile() ? 1 : 0)                   << 28) & 0x0000000010000000);		// HDD dev file 存在確認.
							printf( "(%010u)(BSP) ## %s us_DetailCode=%#x, ull_NewDetailCode=%#llx\n", SYSF_tickGet() , __FUNCTION__, us_DetailCode, ull_NewDetailCode);
							bl_ret = APIC_setNewDetailCode( us_DetailCode, ull_NewDetailCode );		// 詳細コード登録.
							if (False == bl_ret) {
								SYSF_logMsg( "## %s %d : APIC_setNewDetailCode NG\n", __FUNCTION__, __LINE__, 3,4,5,6 );
							}
#endif	// #if defined(TORNADO) && !defined(FUM_LINUX).
							// 2015/10/21 Harada 新詳細コード対応 <<.
							CNTC_CounterIf::newInstance()->countTrouble( TYPD_TroubleCode_D050 );	// C-D050のトラブルをカウント. 
							b_doPowerChangeSequence_Hdd = False;	// HDD復帰処理中フラグを落としておく(500ms毎に、FATALをセットしてしまう事を防ぐため). 
						}
					}
					// 定期起床は、500ms. 
					SYSF_wdStart( wdID_Watch_HDD, SYSF_sysClkRateGet() * DEF_PM_WATCH_HDD_TIMER/1000, (FUNCPTR)THRC_PowerManager::Watch_HDDTimer, 0 );	// 定期起床タイマ起動.
					break;
				default:
					THRC_PM_MSG( "%s unknown msg:%d\n", __FUNCTION__, st_Msg.us_message );
					break;
			}
		}
	}
}
void THRC_PowerManager::Watch_HDDTimer( void )
{
	ushort message = DEF_PM_MESSAGE_WATCH_HDD_TIMER;
	SYSF_msgQSend( thePowerManagerTask.msgQID_PowerManager_Watch_Hdd, (char*)&message, sizeof(message), WAIT_FOREVER, MSG_PRI_NORMAL );
}

/* 2014/09/18 sky_tksk for Fiery省電力 start. */
// ================================================================================================.
//! <!--機能--> 電源管理 制御部 Fieryによる起動復帰処理.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
// 2014/09/18 sky_tksk 新規作成.
// ================================================================================================.
void THRC_PowerManager::NotifyWakeupFromFiery( void )
{
	pc_powerDevice->SwitchPowerState( THRC_PowerDeviceID_SET_KR4,THRC_PowerStateID_NORMAL );
}
/* 2014/09/18 sky_tksk for Fiery省電力 end. */

// ================================================================================================.
//! <!--機能--> 電源管理 制御部 ファックス/スキャン省電力設定が有効かどうかを返す.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return True:ファックス/スキャン省電力設定が有効 False:無効.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
// 2015/10/27 河村 新規作成 CQ.520852 スキャン時定着OFF対応 その2.
// ================================================================================================.
Bool THRC_PowerManager::isFaxScanPowerSave(void)
{
#if (!defined(DEF_S800Emu) && !defined(DEF_EagleCus) && !defined(DEF_SparrowCus) && defined(DEF_Helios_PF)) || (defined(DEF_A4_Cus) && !defined(DEF_SparrowCus))	// PreEmu/Emu3対象外.
	Bool isFaxScanPowerSave = False;
	APIC_SystemSetting*     pc_SystemSetting = APIC_SystemSetting::newInstance();
	if( pc_SystemSetting != NULL && pc_SystemSetting->getFaxScanPowerSaveSetting() == TYPE_FaxScanPowerSaveSettingON ) {
		isFaxScanPowerSave = True;	// ファックス/スキャン省電力設定が有効.
	}

	return isFaxScanPowerSave;
#else
	return False;	// 制御対象外.
#endif // #if (!defined(DEF_S800Emu) && defined(DEF_Helios_PF)) || defined(DEF_A4_Cus). 
}

// ================================================================================================.
//! <!--機能--> 電源管理 制御部 ENG_MOTION_ENABLEをONすべきかどうかを返す.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! ENG_MOTION_ENABLEをON制御するかどうかの判断に使用する。省電力復帰シーケンス中のみ有効(それ以外のタイミングでは\n.
//! 正しい値を返さない)。THRC_PowerManagerDevice.cppより呼び出し\n.
//! <!--履歴-->.
// 2015/04/01 河村 新規作成.
// 2015/11/08 河村 CQ.520955 スリープ/副電源OFF→スリープ遅延/副電源OFF遅延の場合にはMC_MOTION_ENはOffにする.
// 2015/12/07 河村 CQ.523409 MinervaSBK/ZeusS_ZX0はスリープ遅延でもOnにする.
// ================================================================================================.
Bool THRC_PowerManager::isMotionEnableOn(void)
{
#if (!defined(DEF_S800Emu) && !defined(DEF_EagleCus) && !defined(DEF_SparrowCus) && defined(DEF_Helios_PF)) || (defined(DEF_A4_Cus) && !defined(DEF_SparrowCus))	// PreEmu/Emu3対象外.
	Bool isPowerSave = isFaxScanPowerSave();

//Str【VenusMLK CQ.529803 スリープ中動作が失敗する問題】2016.05.23.
	//Csetﾓｰﾄﾞ[実行中]である場合.
	if( uc_CsetModeExec == 1  )
	{
		//Printpの初期通信が完了するまでMotionEnableの制御をガードする.
		return False;
	}
//End【VenusMLK CQ.529803 スリープ中動作が失敗する問題】2016.05.23.

	THRC_PM_LOG( "%"PRId32" : isEngOnlyWakeup=%d, isFaxScanPowerSave=%d isSleepDelay=%d", SYSF_tickGet(), isEngOnlyWakeup, isPowerSave, isSleepDelay );
#if defined( DEF_MinervaS_PF )
	return isEngOnlyWakeup || (!isPowerSave);
#else	// defined( DEF_MinervaS_PF )
	return (isEngOnlyWakeup || (!isPowerSave)) && !isSleepDelay;
#endif	// defined( DEF_MinervaS_PF )
#else
	return True;	// 制御対象外は常にONすべきとする.
#endif // #if (!defined(DEF_S800Emu) && defined(DEF_Helios_PF)) || defined(DEF_A4_Cus). 
}
			
// 2015/07/24 sky_okuma CQ516040対応 Start.
// ================================================================================================.
//! <!--機能--> デバッグログ保存可否設定.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param bl_flg デバッグログ保存可否設定値.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! デバッグログ保存の可否を設定する\n.
//! <!--履歴-->.
// 2015/07/24 sky_okuma 新規作成.
// ================================================================================================.
static void setDebugLogSaveEnable( Bool bl_flg )
{
	bl_debugLogSaveEnable = bl_flg;
}
// ================================================================================================.
//! <!--機能--> デバッグログ保存可否取得.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return True:ログ保存可能 / False:ログ保存不可.
//! <!--詳細-->.
//! デバッグログ保存の可否を取得する\n.
//! <!--履歴-->.
// 2015/07/24 sky_okuma 新規作成.
// ================================================================================================.
Bool getDebugLogSaveEnable( void )
{
	return bl_debugLogSaveEnable;
}
// 2015/07/24 sky_okuma CQ516040対応 End.

// ================================================================================================.
//! <!--機能--> 電源管理 制御部 PMUからの復帰割り込みの許可/禁止設定.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数--> isEnable True:割り込みを有効 False:割り込みを無効.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! PMUで処理している復帰割り込みを許可/禁止する\n.
//! PWRが復帰イベントを処理不可能な期間、復帰割り込みを禁止にするために用意している\n.
//! <!--履歴-->.
// 2015/09/02 河村 新規作成 CQ.518298対応.
// 2017/06/12 Sky-Araki AR1023861 DeepSleep移行中の主電源SW-OFFで3秒後に主電源OFFされる.
// ================================================================================================.
void THRC_PowerManager::pmuWakeupInterruptEnable( Bool isEnable )
{
	int32_t ret = 0, si_Ret;
	
	THRC_PM_LOG("### %s(L%d)/Enable=%d ###\n",__FUNCTION__,__LINE__,isEnable);	
	
	// まだオープンされていない場合はオープンする.
	if( si_Fd < 0 ) {
		si_Fd = SYSF_open( THRD_DeviceFile_Pmu, O_RDWR, 0 );
		if( si_Fd < 0 ) {
			THRC_PM_ASSERT("Cannot open file pmuWakeupEventInterruptEnable THRD_DeviceFile_Pmu", 0,0,0,0,CFA14_DetailCode0x0CEA );
		}
	}
	// 復帰要因割り込み許可/禁止.
	ret = ioctl( si_Fd, THRD_PMU_IOCSPWRSUBINT, isEnable ? 1 : 0 );
	if( ret < 0 ) {
		setFA14_PmuDriverIoError(0x06,errno);		// 2017/2/21 山口 AR.1013578 詳細コード0CEB調査.
		THRC_PM_ASSERT("pmu ioctl(THRD_PMU_IOCSPWRSUBINT) error.", 0,0,0,0,CFA14_DetailCode0x0CEB );
	}
	else {
		// 電SUB-CPUの割込みを許可する場合、割込み禁止区間で発生した割込みを取得する.
		if( True== isEnable ) {
			si_Ret= ioctl(si_Fd,THRD_PMU_IOCSREQESTFACTOR,0);
			if( si_Ret< 0 ) {
				printf( "(%010u)(BSP) ### %s(L%d)/Failed to read PwrSub-CPU IntFacter Ret=%d ###\n", SYSF_tickGet() ,__FUNCTION__,__LINE__,si_Ret);
			}
		}
	}
}

// ================================================================================================.
//! <!--機能--> 電源管理 制御部 Lot26対応HWかどうかを返す.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数--> 
//! \param なし.
//! <!--戻値-->.
//! \return  True= Lot26対応 False= Lot26非対応.
//! <!--詳細-->.
//! Lot26HwTypeはPowerManagerコンストラクト内で設定される.
//! <!--履歴-->.
// 2015/12/18 Sky-Araki 新規作成 CQ520828 Lot26対応.
// 2017/10/11 Sky佐藤 S800省電力Lot26対応. 
// ================================================================================================.
Bool THRC_PowerManager::IsLot26HWType(void)
{
	return Lot26HwType;
}

// ================================================================================================.
//! <!--機能--> 電源管理 制御部  タスク生成：電源OFFタイマー設定.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
// 2016/11/21 sky_ishiga [MinervaMLK][起動時間短縮] 電源SubCpuタイマー設定対応.
// 2017/10/18 Sky-ishiga [Eagle] [起動時間短縮(再設計)] インターフェース見直し(固有名称(電源SubCPU)を削除).
// 2018/11/05 Sky-yoshimura [AR1062169] 新詳細コード削除対応. [IT5_4.2][AR1038475]からの横展開
// ================================================================================================.
#define THRD_PowerOffTimerTaskName	"TASK_PwrOffTimr"			// タスク名称.
#define THRD_PowerOffTimerPriority	(8)							// プライオリティ.
#define THRD_PowerOffTimerStackSize	(0x4000)					// スタックサイズ.
void THRC_PowerManager::CreateTask_SetPowerOffTimer( void )
{
	// タスク起動.
	THR_ID tid = SYSF_taskSpawn_Join(
			THRD_PowerOffTimerTaskName,				// タスク名称.
			THRD_PowerOffTimerPriority,				// プライオリティ.
			VX_FP_TASK,
			THRD_PowerOffTimerStackSize,			// スタックサイズ.
			(FUNCPTR)THRC_PowerManager::taskSetPowerOffTimer,	// タスク制御.
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

	if ( ERROR == tid ) {
		printf( "(%010u)(BSP) CreateTask_SetPowerOffTimer: ERROR!!!\n", SYSF_tickGet() );
		// 電源断.
		THRF_setPowerOffDelay( False );
		return;
	}

	// タスクIDの保持.
	si_taskID_setPowerOffTimer = tid;
	THRC_PM_LOG("CreateTask_SetPowerOffTimer: Create OK [tid:%d]", tid);
}

// ================================================================================================.
//! <!--機能--> 電源管理 制御部  タスク制御：電源OFFタイマー開始.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
//! 2019/04/04 Sky[IT6_1.0MR2][AR1074753] 電源OFFタイマータスクの常駐化.
// ================================================================================================.
void THRC_PowerManager::SendStart_PowerOffTimer( void )
{
	// 電源OFFタイマー更新要求開始.
	ushort us_Msg = DEF_TM_SetPowerOffTimerStart;
	SYSF_msgQSend( thePowerManagerTask.msgQID_setPowerOffTimer, (char*)&us_Msg, sizeof(us_Msg), WAIT_FOREVER, MSG_PRI_NORMAL );
}
// ================================================================================================.
//! <!--機能--> 電源管理 制御部  タスク制御：電源OFFタイマー設定.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
// 2016/11/21 sky_ishiga [MinervaMLK][起動時間短縮] 電源SubCpuタイマー設定対応.
// 2017/01/27 sky_matsuoka デバックコマンド実行時にはタイマ処理を解除する.
// 2017/06/01 sky_ishiga [AR1023387] OFF時に電Subでタイムアウト..
// 2017/10/18 Sky-ishiga [Eagle] [起動時間短縮(再設計)] インターフェース見直し(固有名称(電源SubCPU)を削除).
// 2018/05/31 Sky酒井(純) [Eagle] [AR.1048114] 電源SubCPUのSnapShot取得前タイマーの設定値ををSDとSSDで切り替える.
// 2019/04/04 Sky[IT6_1.0MR2][AR1074753] 電源OFFタイマータスクの常駐化.
// ================================================================================================.
void THRC_PowerManager::taskSetPowerOffTimer( void )
{
	ushort us_Msg;
	int32_t si_res;
	ushort us_SSTimer = 0;	// 電源SubCPUのSnapShot取得前タイマー設定値.

	THRC_PM_LOG("taskSetPowerOffTimer: Enter.");
	
	// メッセージキュー生成.
	thePowerManagerTask.msgQID_setPowerOffTimer = SYSF_msgQCreate( MAX_MSGS, sizeof(ushort), MSG_Q_FIFO );

	while( 1 ) {
		si_res = SYSF_msgQReceive( thePowerManagerTask.msgQID_setPowerOffTimer, (char*)&us_Msg, sizeof(us_Msg), WAIT_FOREVER );
		// デバックコマンド実行時にはタイマ処理を解除する.
		if (THRE_SWSequenceStop==s_debugCmd) {
			break;
		}
		if( si_res == ERROR ) {
#ifdef TORNADO
			continue;
#else
			return;
#endif
		} else {
			if ( TM_SetPowerOffTimer == us_Msg ) {
				// 定期起床.
// comment 2017/06/01 ishiga [AR1023387] 処理順を変更.
//				// タイマー更新.
//				THRF_setPowerOffTimer( THRD_PowerOffTimerNum );
				// 
				if( True == bl_PowerOffTimerUpdate){
					// watchDogタイマー起動.
					thePowerManagerTask.start_cyclicLoop_setPowerOffTimer();
					// タイマー更新.
					THRF_setPowerOffTimer( THRD_PowerOffTimerNum );
				}
			} else if ( TM_SetPowerOffTimerSS == us_Msg ) {
				// SnapShot取得前タイマー設定.
				// WatchDogタイマ削除.
				if ( NULL != thePowerManagerTask.wdogID_SetPowerOffTimer ) {
					SYSF_wdDelete( thePowerManagerTask.wdogID_SetPowerOffTimer );
					thePowerManagerTask.wdogID_SetPowerOffTimer = NULL;
				}
				// タイマー更新.
				if( THRE_StartupDeviceType_SD == THRF_GetStartupDeviceType() ){
					// SDの場合.
					us_SSTimer = THRD_PowerOffTimerSSNum_SD;
				}else{
					// SSDの場合.
					us_SSTimer = THRD_PowerOffTimerSSNum;
				}
				THRF_setPowerOffTimer( us_SSTimer );
				THRC_PM_LOG("taskSetPowerOffTimer: Set PowerOffTimer %d Just before get SuperWarp Snapshot.", us_SSTimer);
				bl_PowerOffTimerUpdate = False;
			} else if ( TM_SetPowerOffTimerStart == us_Msg ) {
				bl_PowerOffTimerUpdate = True;
				// 電源OFFタイマー更新要求開始.
				// watchDogタイマー起動.
				thePowerManagerTask.start_cyclicLoop_setPowerOffTimer();
				// タイマー更新.
				THRF_setPowerOffTimer( THRD_PowerOffTimerNum );
			} else {
				// 不明なメッセージ.
				printf( "(%010u)(BSP) taskSetPowerOffTimer: unknown message :%d\n", SYSF_tickGet() , us_Msg );
				// 電源断.
				THRF_setPowerOffDelay( False );
				break;
			}
		}
	}

	THRC_PM_LOG("taskSetPowerOffTimer: Leave.");
}

// ================================================================================================.
//! <!--機能--> 電源管理 制御部  定期起床開始：電源OFFタイマー設定.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
// 2016/11/21 sky_ishiga [MinervaMLK][起動時間短縮] 電源SubCpuタイマー設定対応.
// ================================================================================================.
#define THRD_SetPowerOffTimer_WakeupNum		(1000)	//. 電源OFFタイマ更新定期起床時間.
void THRC_PowerManager::start_cyclicLoop_setPowerOffTimer( void )
{
	// 初回.
	if ( NULL == wdogID_SetPowerOffTimer ) {
		wdogID_SetPowerOffTimer = SYSF_wdCreate();
	}

	// タイマー起動.
	SYSF_wdStart( wdogID_SetPowerOffTimer,
			SYSF_sysClkRateGet() * THRD_SetPowerOffTimer_WakeupNum/1000,
			(FUNCPTR)THRC_PowerManager::cyclicLoop_setPowerOffTimer, 0 );	// 定期起床タイマ起動.
}

// ================================================================================================.
//! <!--機能--> 電源管理 制御部  定期起床ハンドラ処理：電源OFFタイマー設定.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
// 2016/11/21 sky_ishiga [MinervaMLK][起動時間短縮] 電源SubCpuタイマー設定対応.
// 2017/10/18 Sky-ishiga [Eagle] [起動時間短縮(再設計)] インターフェース見直し(固有名称(電源SubCPU)を削除).
// ================================================================================================.
void THRC_PowerManager::cyclicLoop_setPowerOffTimer( void )
{
	ushort us_Msg = TM_SetPowerOffTimer;
	SYSF_msgQSend( thePowerManagerTask.msgQID_setPowerOffTimer, (char*)&us_Msg, sizeof(us_Msg), WAIT_FOREVER, MSG_PRI_NORMAL );
}

// ================================================================================================.
//! <!--機能--> 電源管理 制御部 PMUドライバーのIOCTLエラー新詳細コード登録.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数--> 
//! \param ushort us_num:エラー箇所を判定する通し番号.
//		   int si_error:errno errnoはint型なのでint型で宣言、PMUドライバーのIOCTLエラーは1byteのみなのでFFで丸めこむ.
//! <!--戻値-->.
//! \return  なし.
//! <!--詳細-->.
//! PMUドライバーのIOCTLエラー新詳細コード登録する.
//! <!--履歴-->.
// 2017/2/21 山口 新規作成 AR.1013578 詳細コード0CEB調査.
// ================================================================================================.
void THRC_PowerManager::setFA14_PmuDriverIoError( ushort us_num , int si_error){
	
	printf( "(%010u)(BSP) us_num[%d],si_error[%d]\n", SYSF_tickGet() ,us_num,si_error);
#if defined(TORNADO) && !defined(FUM_LINUX) /*実機*/
	//++ 2019/04/24 Sky		【EagleH】Warning除去対応.
	uint64_t ull_NewDetailCode = 0x0;
	Bool bl_ret;
	//-- 2019/04/24 Sky		【EagleH】Warning除去対応.

	ull_NewDetailCode = (((uint64_t)0x22			<< 56) & 0xff00000000000000)		// モジュール番号.
					  | (((uint64_t)0x00B			<< 44) & 0x00fff00000000000)		// 通し番号.
					  | (((uint64_t)us_num			<< 36) & 0x00000ff000000000)		// IOエラーを起こした箇所の通し番号.
					  | (((uint64_t)si_error		<< 28) & 0x0000000ff0000000);		// エラー内容.
	
	bl_ret = APIC_setNewDetailCode( TYPE_FatalDetailCode_undefined, ull_NewDetailCode );		// 詳細コード登録.
	if (False == bl_ret) {
		SYSF_logMsg( "## %s %d : APIC_setNewDetailCode NG\n", __FUNCTION__, __LINE__, 3,4,5,6 );
	}
#endif	// #if defined(TORNADO) && !defined(FUM_LINUX).

}

// ================================================================================================.
//! <!--機能--> 電源管理 制御部  SegF発生時の新詳細コード登録コールバック.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数--> 
//! \param ushort us_TaskDetailCode：詳細コード(タスク番号).
//! <!--戻値-->.
//! \return  なし.
//! <!--詳細-->.
//! SegF発生時コールバック関数コールされ、新詳細コード登録を行う.
//! <!--履歴-->.
// 2017/11/02 Harada 新規作成 AR.1029174 新詳細コード改善 SegF付加情報対応.
// 2020/12/08 OP-BTS_28357 PowerManagerでSegF時の詳細コード改善 IT6用にデバイスを変更、省電力シーケンスの記録を変更.
// ================================================================================================.
void THRC_PowerManager::execFatalCallBack( ushort us_TaskDetailCode )
{
#if defined(TORNADO) && !defined(FUM_LINUX) /*実機*/
	printf("## %s us_TaskDetailCode=%#x\n", __FUNCTION__, us_TaskDetailCode);
	unsigned short us_DetailCode = 0x0000;
	uint64_t ull_NewDetailCode = 0x0;
	Bool bl_ret = True;
	us_DetailCode = us_TaskDetailCode;
	THRC_PowerManagerDevice *pc_powerDevice = &THRC_PowerManagerDeviceSingletonIf::newInstance();	// デバイス制御部インスタンス.
	uint32_t ul_offsetCurrent = 0;
	uint32_t ul_offsetNext = 0;
	uint32_t ul_pos = 0;
	unsigned char uc_seqCode = 0;
	/* 使用領域を決める */
	initDebugGetCurrentSectorOffset( THRD_OFFSET_INITDEBUG_PM_SEC1, THRD_OFFSET_INITDEBUG_PM_SEC2, &ul_offsetCurrent, &ul_offsetNext );
	/* 領域の最後の位置から0xffでない値を検索 */
	ul_pos = initDebugGetLastPos( ul_offsetCurrent );
	/* 値取得 */
	if ( False == THRF_readFlash( ul_offsetCurrent + ul_pos, (void*)&uc_seqCode, sizeof(uc_seqCode) ) ) {
		/* エラー */
		SYSF_logMsg( "## %s %d : THRF_readFlash ERR\n", __FUNCTION__, __LINE__, 3,4,5,6 );
	}
	printf("## ul_offsetCurrent=%#x, ul_pos=%d, uc_seqCode=%#x\n", ul_offsetCurrent, ul_pos, uc_seqCode);
	printf("## be_TaskMessage=%d\n", be_TaskMessage);
	ull_NewDetailCode = (((uint64_t)0x22                  << 56) & 0xff00000000000000)		// モジュール番号.
					  | (((uint64_t)0x025                 << 44) & 0x00fff00000000000)		// 通し番号.
					  | (((uint64_t)uc_ChangeStatusBefore << 39) & 0x00000f8000000000)		// 遷移前電源ステータス.
					  | (((uint64_t)uc_ChangeStatusAfter  << 34) & 0x0000007c00000000)		// 遷移後電源ステータス.
					  | (((uint64_t)(THRC_PowerStateID_NORMAL == pc_powerDevice->GetPowerState( THRC_PowerDeviceID_IR ) ? 1 : 0)                     << 33) & 0x0000000200000000)		// 現在の電源ステート取得.
					  | (((uint64_t)(THRC_PowerStateID_NORMAL == pc_powerDevice->GetPowerState( THRC_PowerDeviceID_IR_RY ) ? 1 : 0)                  << 32) & 0x0000000100000000)		// 現在の電源ステート取得.
					  | (((uint64_t)(THRC_PowerStateID_NORMAL == pc_powerDevice->GetPowerState( THRC_PowerDeviceID_ENG_IRC_MORTION_ENABLE ) ? 1 : 0) << 31) & 0x0000000080000000)		// 現在の電源ステート取得.
					  | (((uint64_t)(THRC_PowerStateID_NORMAL == pc_powerDevice->GetPowerState( THRC_PowerDeviceID_Fiery ) ? 1 : 0)                  << 30) & 0x0000000040000000)		// 現在の電源ステート取得.
					  | (((uint64_t)(THRC_PowerStateID_NORMAL == pc_powerDevice->GetPowerState( THRC_PowerDeviceID_Fiery_PT_R ) ? 1 : 0)             << 29) & 0x0000000020000000)		// 現在の電源ステート取得.
					  | (((uint64_t)(THRC_PowerStateID_NORMAL == pc_powerDevice->GetPowerState( THRC_PowerDeviceID_Panel ) ? 1 : 0)                  << 28) & 0x0000000010000000)		// 現在の電源ステート取得.
					  | (((uint64_t)(THRC_PowerStateID_NORMAL == pc_powerDevice->GetPowerState( THRC_PowerDeviceID_LCDC_LVDS ) ? 1 : 0)              << 27) & 0x0000000008000000)		// 現在の電源ステート取得.
					  | (((uint64_t)(THRC_PowerStateID_NORMAL == pc_powerDevice->GetPowerState( THRC_PowerDeviceID_PANEL_POWER ) ? 1 : 0)            << 26) & 0x0000000004000000)		// 現在の電源ステート取得.
					  | (((uint64_t)(THRC_PowerStateID_NORMAL == pc_powerDevice->GetPowerState( THRC_PowerDeviceID_ENG ) ? 1 : 0)                    << 25) & 0x0000000002000000)		// 現在の電源ステート取得.
					  | (((uint64_t)(THRC_PowerStateID_NORMAL == pc_powerDevice->GetPowerState( THRC_PowerDeviceID_ENG_POWER ) ? 1 : 0)              << 24) & 0x0000000001000000)		// 現在の電源ステート取得.
					  | (((uint64_t)(THRC_PowerStateID_NORMAL == pc_powerDevice->GetPowerState( THRC_PowerDeviceID_ENG_MORTION_ENABLE ) ? 1 : 0)     << 23) & 0x0000000000800000)		// 現在の電源ステート取得.
					  | (((uint64_t)(THRC_PowerStateID_NORMAL == pc_powerDevice->GetPowerState( THRC_PowerDeviceID_PCIe_BUS ) ? 1 : 0)               << 22) & 0x0000000000400000)		// 現在の電源ステート取得.
					  | (((uint64_t)(THRC_PowerStateID_NORMAL == pc_powerDevice->GetPowerState( THRC_PowerDeviceID_PEXPD ) ? 1 : 0)                  << 21) & 0x0000000000200000)		// 現在の電源ステート取得.
					  | (((uint64_t)(THRC_PowerStateID_NORMAL == pc_powerDevice->GetPowerState( THRC_PowerDeviceID_WAKE_FAXMFPC ) ? 1 : 0)           << 20) & 0x0000000000100000)		// 現在の電源ステート取得.
					  | (((uint64_t)(THRC_PowerStateID_NORMAL == pc_powerDevice->GetPowerState( THRC_PowerDeviceID_MC_PWR_EN ) ? 1 : 0)              << 19) & 0x0000000000080000)		// 現在の電源ステート取得.
					  | (((uint64_t)be_TaskMessage                                                                                                   <<  8) & 0x000000000007ff00)		// メッセージ種別.
					  | (((uint64_t)uc_seqCodeGlobal                                                                                                 <<  0) & 0x00000000000000ff);	// 起動/省電力シーケンスデバッグ 省電力移行復帰進捗状況.
	printf("## %s us_DetailCode=%#x, ull_NewDetailCode=%#llx\n", __FUNCTION__, us_DetailCode, ull_NewDetailCode);
	bl_ret = APIC_setNewDetailCode( us_DetailCode, ull_NewDetailCode );		// 詳細コード登録.
	if (False == bl_ret) {
		SYSF_logMsg( "## %s %d : APIC_setNewDetailCode NG\n", __FUNCTION__, __LINE__, 3,4,5,6 );
	}
#endif	// #if defined(TORNADO) && !defined(FUM_LINUX).
}

// ================================================================================================.
//! <!--機能--> 電源管理 制御部  SegF発生時の新詳細コード登録コールバック.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数--> 
//! \param ushort us_TaskDetailCode：詳細コード(タスク番号).
//! <!--戻値-->.
//! \return  なし.
//! <!--詳細-->.
//! SegF発生時コールバック関数コールされ、新詳細コード登録を行う.
//! <!--履歴-->.
// 2017/11/02 Harada 新規作成 AR.1029174 新詳細コード改善 SegF付加情報対応.
// ================================================================================================.
void THRC_PowerManager::fatalCallBack( ushort us_TaskDetailCode )
{
#if defined(TORNADO) && !defined(FUM_LINUX) /*実機*/
	thePowerManagerTask.execFatalCallBack( us_TaskDetailCode );
#endif	// #if defined(TORNADO) && !defined(FUM_LINUX).
}

// ================================================================================================.
//! <!--機能--> 電源管理 制御部  SegF発生時の新詳細コード登録コールバック PCIePCIDev.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数--> 
//! \param ushort us_TaskDetailCode：詳細コード(タスク番号).
//! <!--戻値-->.
//! \return  なし.
//! <!--詳細-->.
//! SegF発生時コールバック関数コールされ、新詳細コード登録を行う PCIePCIDev.
//! <!--履歴-->.
// 2017/11/02 Harada 新規作成 AR.1029174 新詳細コード改善 SegF付加情報対応.
// ================================================================================================.
void THRC_PowerManager::execFatalCallBack_PCIePCIDev( ushort us_TaskDetailCode )
{
#if defined(TORNADO) && !defined(FUM_LINUX) /*実機*/
	printf("## %s us_TaskDetailCode=%#x\n", __FUNCTION__, us_TaskDetailCode);
	unsigned short us_DetailCode = 0x0000;
	uint64_t ull_NewDetailCode = 0x0;
	Bool bl_ret = True;
	us_DetailCode = us_TaskDetailCode;
	THRC_PowerManagerDevice *pc_powerDevice = &THRC_PowerManagerDeviceSingletonIf::newInstance();	// デバイス制御部インスタンス.
	uint32_t ul_offsetCurrent = 0;
	uint32_t ul_offsetNext = 0;
	uint32_t ul_pos = 0;
	unsigned char uc_seqCode = 0;
	/* 使用領域を決める */
	initDebugGetCurrentSectorOffset( THRD_OFFSET_INITDEBUG_PM_SEC1, THRD_OFFSET_INITDEBUG_PM_SEC2, &ul_offsetCurrent, &ul_offsetNext );
	/* 領域の最後の位置から0xffでない値を検索 */
	ul_pos = initDebugGetLastPos( ul_offsetCurrent );
	/* 値取得 */
	if ( False == THRF_readFlash( ul_offsetCurrent + ul_pos, (void*)&uc_seqCode, sizeof(uc_seqCode) ) ) {
		/* エラー */
		SYSF_logMsg( "## %s %d : THRF_readFlash ERR\n", __FUNCTION__, __LINE__, 3,4,5,6 );
	}
	printf("## ul_offsetCurrent=%#x, ul_pos=%d, uc_seqCode=%#x\n", ul_offsetCurrent, ul_pos, uc_seqCode);
	ull_NewDetailCode = (((uint64_t)0x22                  << 56) & 0xff00000000000000)		// モジュール番号.
					  | (((uint64_t)uc_ChangeStatusBefore << 51) & 0x00f8000000000000)		// 遷移前電源ステータス.
					  | (((uint64_t)uc_ChangeStatusAfter  << 46) & 0x0007c00000000000)		// 遷移後電源ステータス.
//					  | (((uint64_t)(THRC_PowerStateID_NORMAL == pc_powerDevice->GetPowerState( THRC_PowerDeviceID_IR ) ? 1 : 0)                     << 45) & 0x0000200000000000)		// 現在の電源ステート取得.
//					  | (((uint64_t)(THRC_PowerStateID_NORMAL == pc_powerDevice->GetPowerState( THRC_PowerDeviceID_IR_SERIES_UART ) ? 1 : 0)         << 44) & 0x0000100000000000)		// 現在の電源ステート取得.
//					  | (((uint64_t)(THRC_PowerStateID_NORMAL == pc_powerDevice->GetPowerState( THRC_PowerDeviceID_IR_RY ) ? 1 : 0)                  << 43) & 0x0000080000000000)		// 現在の電源ステート取得.
//					  | (((uint64_t)(THRC_PowerStateID_NORMAL == pc_powerDevice->GetPowerState( THRC_PowerDeviceID_EDH_UART ) ? 1 : 0)               << 42) & 0x0000040000000000)		// 現在の電源ステート取得.
//					  | (((uint64_t)(THRC_PowerStateID_NORMAL == pc_powerDevice->GetPowerState( THRC_PowerDeviceID_ENG_IRC_MORTION_ENABLE ) ? 1 : 0) << 41) & 0x0000020000000000)		// 現在の電源ステート取得.
//					  | (((uint64_t)(THRC_PowerStateID_NORMAL == pc_powerDevice->GetPowerState( THRC_PowerDeviceID_HDD ) ? 1 : 0)                    << 40) & 0x0000010000000000)		// 現在の電源ステート取得.
//					  | (((uint64_t)(THRC_PowerStateID_NORMAL == pc_powerDevice->GetPowerState( THRC_PowerDeviceID_FileSystem ) ? 1 : 0)             << 39) & 0x0000008000000000)		// 現在の電源ステート取得.
//					  | (((uint64_t)(THRC_PowerStateID_NORMAL == pc_powerDevice->GetPowerState( THRC_PowerDeviceID_HDD_DRIVER ) ? 1 : 0)             << 38) & 0x0000004000000000)		// 現在の電源ステート取得.
//					  | (((uint64_t)(THRC_PowerStateID_NORMAL == pc_powerDevice->GetPowerState( THRC_PowerDeviceID_POWER_OFF_HDD ) ? 1 : 0)          << 37) & 0x0000002000000000)		// 現在の電源ステート取得.
//					  | (((uint64_t)(THRC_PowerStateID_NORMAL == pc_powerDevice->GetPowerState( THRC_PowerDeviceID_USB ) ? 1 : 0)                    << 36) & 0x0000001000000000)		// 現在の電源ステート取得.
//					  | (((uint64_t)(THRC_PowerStateID_NORMAL == pc_powerDevice->GetPowerState( THRC_PowerDeviceID_USB_HOST ) ? 1 : 0)               << 35) & 0x0000000800000000)		// 現在の電源ステート取得.
//					  | (((uint64_t)(THRC_PowerStateID_NORMAL == pc_powerDevice->GetPowerState( THRC_PowerDeviceID_USB_DEVICE ) ? 1 : 0)             << 34) & 0x0000000400000000)		// 現在の電源ステート取得.
//					  | (((uint64_t)(THRC_PowerStateID_NORMAL == pc_powerDevice->GetPowerState( THRC_PowerDeviceID_Fiery ) ? 1 : 0)                  << 33) & 0x0000000200000000)		// 現在の電源ステート取得.
//					  | (((uint64_t)(THRC_PowerStateID_NORMAL == pc_powerDevice->GetPowerState( THRC_PowerDeviceID_Fiery_PT_R ) ? 1 : 0)             << 32) & 0x0000000100000000)		// 現在の電源ステート取得.
//					  | (((uint64_t)(THRC_PowerStateID_NORMAL == pc_powerDevice->GetPowerState( THRC_PowerDeviceID_Panel ) ? 1 : 0)                  << 31) & 0x0000000080000000)		// 現在の電源ステート取得.
//					  | (((uint64_t)(THRC_PowerStateID_NORMAL == pc_powerDevice->GetPowerState( THRC_PowerDeviceID_LCDC_LVDS ) ? 1 : 0)              << 30) & 0x0000000040000000)		// 現在の電源ステート取得.
//					  | (((uint64_t)(THRC_PowerStateID_NORMAL == pc_powerDevice->GetPowerState( THRC_PowerDeviceID_PANEL_POWER ) ? 1 : 0)            << 29) & 0x0000000020000000)		// 現在の電源ステート取得.
//					  | (((uint64_t)(THRC_PowerStateID_NORMAL == pc_powerDevice->GetPowerState( THRC_PowerDeviceID_ENG ) ? 1 : 0)                    << 28) & 0x0000000010000000)		// 現在の電源ステート取得.
//					  | (((uint64_t)(THRC_PowerStateID_NORMAL == pc_powerDevice->GetPowerState( THRC_PowerDeviceID_ENG_UART ) ? 1 : 0)               << 27) & 0x0000000008000000)		// 現在の電源ステート取得.
//					  | (((uint64_t)(THRC_PowerStateID_NORMAL == pc_powerDevice->GetPowerState( THRC_PowerDeviceID_ENG_POWER ) ? 1 : 0)              << 26) & 0x0000000004000000)		// 現在の電源ステート取得.
//					  | (((uint64_t)(THRC_PowerStateID_NORMAL == pc_powerDevice->GetPowerState( THRC_PowerDeviceID_ENG_MORTION_ENABLE ) ? 1 : 0)     << 25) & 0x0000000002000000)		// 現在の電源ステート取得.
					  | (((uint64_t)(THRC_PowerStateID_NORMAL == pc_powerDevice->GetPowerState( THRC_PowerDeviceID_PCIe_BUS ) ? 1 : 0)               << 24) & 0x0000000001000000)		// 現在の電源ステート取得.
					  | (((uint64_t)(THRC_PowerStateID_NORMAL == pc_powerDevice->GetPowerState( THRC_PowerDeviceID_ASIC ) ? 1 : 0)                   << 23) & 0x0000000000800000)		// 現在の電源ステート取得.
					  | (((uint64_t)(THRC_PowerStateID_NORMAL == pc_powerDevice->GetPowerState( THRC_PowerDeviceID_PCIe_CLK ) ? 1 : 0)               << 22) & 0x0000000000400000)		// 現在の電源ステート取得.
					  | (((uint64_t)(THRC_PowerStateID_NORMAL == pc_powerDevice->GetPowerState( THRC_PowerDeviceID_IMGSYS ) ? 1 : 0)                 << 21) & 0x0000000000200000)		// 現在の電源ステート取得.
					  | (((uint64_t)(THRC_PowerStateID_NORMAL == pc_powerDevice->GetPowerState( THRC_PowerDeviceID_PEXPD ) ? 1 : 0)                  << 20) & 0x0000000000100000)		// 現在の電源ステート取得.
					  | (((uint64_t)(THRC_PowerStateID_NORMAL == pc_powerDevice->GetPowerState( THRC_PowerDeviceID_WAKE_FAXMFPC ) ? 1 : 0)           << 19) & 0x0000000000080000)		// 現在の電源ステート取得.
					  | (((uint64_t)uc_seqCode                                                                                                       <<  0) & 0x00000000000000ff);	// 起動/省電力シーケンスデバッグ 省電力移行復帰進捗状況.
	printf("## %s us_DetailCode=%#x, ull_NewDetailCode=%#llx\n", __FUNCTION__, us_DetailCode, ull_NewDetailCode);
	bl_ret = APIC_setNewDetailCode( us_DetailCode, ull_NewDetailCode );		// 詳細コード登録.
	if (False == bl_ret) {
		SYSF_logMsg( "## %s %d : APIC_setNewDetailCode NG\n", __FUNCTION__, __LINE__, 3,4,5,6 );
	}
#endif	// #if defined(TORNADO) && !defined(FUM_LINUX).
}

// ================================================================================================.
//! <!--機能--> 電源管理 制御部  SegF発生時の新詳細コード登録コールバック PCIePCIDev.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数--> 
//! \param ushort us_TaskDetailCode：詳細コード(タスク番号).
//! <!--戻値-->.
//! \return  なし.
//! <!--詳細-->.
//! SegF発生時コールバック関数コールされ、新詳細コード登録を行う PCIePCIDev.
//! <!--履歴-->.
// 2017/11/02 Harada 新規作成 AR.1029174 新詳細コード改善 SegF付加情報対応.
// ================================================================================================.
void THRC_PowerManager::fatalCallBack_PCIePCIDev( ushort us_TaskDetailCode )
{
#if defined(TORNADO) && !defined(FUM_LINUX) /*実機*/
	thePowerManagerTask.execFatalCallBack_PCIePCIDev( us_TaskDetailCode );
#endif	// #if defined(TORNADO) && !defined(FUM_LINUX).
}

// ================================================================================================.
//! <!--機能--> 電源管理 制御部 電源キー長押し時のパネル振動.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数--> 
//! \param なし.
//! <!--戻値-->.
//! \return  なし.
//! <!--詳細-->.
//! 電源キー長押し検知時のパネル振動を行う.
//! <!--履歴-->.
// 	2018/03/28 Sky酒井(純）新規作成.
// ================================================================================================.
void THRC_PowerManager::setVibrationLongTapPowerKey( void )
{
	APIC_OperationSetting* pc_OperationSetting = APIC_OperationSetting::newInstance();
	CNVC_PanelHard*	pc_PanelHard = CNVC_PanelHard::newInstance();	

	// NULLチェック.
	if( NULL == pc_OperationSetting ){
		printf( "(%010u)(BSP) %s pc_OperationSetting is NULL\n", SYSF_tickGet() , __FUNCTION__);
		return;
	}

	if( NULL == pc_PanelHard ){
		printf( "(%010u)(BSP) %s ppc_PanelHard is NULL\n", SYSF_tickGet() , __FUNCTION__);
		return;
	}
	
	// 振動応答の設定値を取得.
	if( TYPE_VibrationResponse_On == pc_OperationSetting->getVibrationLongTapPowerKey()){
		THRC_PM_LOG( "%s Vibration Setting On\n", __FUNCTION__);
		// パネルを振動させる.
		pc_PanelHard->setVibrator( THRD_PANEL_VIB_PATTERN_E, THRD_PANEL_VIB_AMOUNT_MID );
	}else{
		THRC_PM_LOG( "%s Vibration Setting Off\n", __FUNCTION__);
	}
	
}

/*================================================================================================
［機能］	電源OFFシーケンス実行中時間の設定を行う.
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	無し
［履歴］	2018/07/11 Sky酒井(純) [Eagle] [AR.1058545] 電源OFFシーケンス実行中時間の設定を行う.
================================================================================================*/
void THRC_PowerManager::setPowerOffSeqTime( void )
{
	if( THRE_StartupDeviceType_SD == THRF_GetStartupDeviceType() ){
		// mSDの場合.
		THRF_setPowerOffSeqTime(THRD_PWROFF_SEQTIME_SD);
	}else{
		// SSDの場合.
		THRF_setPowerOffSeqTime(THRD_PWROFF_SEQTIME_SSD);
	}
}

// ================================================================================================.
//! <!--機能--> 電源管理 制御部 パネル電源制御.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数--> 
//! \param b_isSave True:省電力状態にする False:通常状態にする.
//! <!--戻値-->.
//! \return  なし.
//! <!--詳細-->.
//! パネル電源制御を行う。電源制御との整合は取られない。それを考慮して実行できる電源部(PWR)専用.
//! <!--履歴-->.
// 	2018/11/20 河村 初版.
// ================================================================================================.
void THRC_PowerManager::req_PanelPowerSave( Bool b_isSave )
{
	THRS_PowerManagerMsg st_Msg ;

	if( b_isSave ) {
		st_Msg.uc_localChangeStatusBefore = THRE_PowerSaveStatusIdle;
		st_Msg.uc_localChangeStatusAfter = THRE_PowerSaveStatusSubPowerOff15;
		st_Msg.us_message = TM_ReqPowerSavePanel;
	} else {
		st_Msg.uc_localChangeStatusBefore = THRE_PowerSaveStatusSubPowerOff15;
		st_Msg.uc_localChangeStatusAfter = THRE_PowerSaveStatusIdle;
		st_Msg.us_message = TM_ReqWakeUpPanel;
	}
	THRC_PM_LOG( "%s b_isSave=%d ", __FUNCTION__, b_isSave );
	SYSF_msgQSend( thePowerManagerTask.msgQID_PowerManager_Panel, (char*)&st_Msg, sizeof(st_Msg), WAIT_FOREVER, MSG_PRI_NORMAL );
}

// ================================================================================================.
//! <!--機能--> 電源管理 制御部 ストレージアンロック.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数--> 
//! \param なし.
//! <!--戻値-->.
//! \return  なし.
//! <!--詳細-->.
//! ストレージがロックされている場合にロック解除を行う。DeepSleep復帰後最速のタイミングで呼び出す必要あり.
//! <!--履歴-->.
// 	2020/02/17 河村 初版.
// ================================================================================================.
void THRC_PowerManager::unlockStorage( Bool isSuperWarp )
{
	if( isStorageUnlock ) {
		/* 2020/07/01 OP_BTS-24973 河村 - OPALアンロック修正:kernelで行うのでここでは行わない */
#if 0
/*		theHDDTrans.unlockStorage(); */		// 2019/05/31 ft_kondo AR1077537 SSDロックパスワード有効時にトラブルコードC-D391問題対策.
#else
/*		theHDDTrans.forceReMount(); */			// 2020/06/17 OP_BTS-24547 河村 - 強制リマウントする.
#endif
		theHDDTrans.setSecurityLocked(0);
		THRC_PM_LOG("%s : Storage unlock(disabled).", __FUNCTION__ );
		// 2020/04/17 Harada OP_BTS-21547 DeepSleepから主電源OFFでSnapshot取得しない問題対策 ロックパスワード設定時、Shutdownをアンロック後に行う対応.
		if (True == isResetContDelayFlag) {	// reset_controller遅延フラグが有効な場合.
			sync();							// sync.
			if( THRE_StartupDeviceType_SSD == THRF_GetStartupDeviceType() ){ // SSDの場合.
				system("echo 1 > /sys/devices/platform/quartz-iris-sb-0/8d0098000.pcie0/pci0000:00/0000:00:00.0/0000:01:00.0/nvme/nvme0/reset_controller");
			}
			isResetContDelayFlag = False;
		}
		isStorageUnlock = False;
	} else {
		THRC_PM_LOG("%s : Storage no unlock required.", __FUNCTION__ );
	}
	theHDDTrans.setDisableSSDAccess(0);	// OP_BTS-22176:SSDアクセス禁止状態設定(0:許可).
}


// ================================================================================================.
//! <!--機能--> CA72WDT EXTのリフレッシュ
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数--> 
//! \param uint32_t point		リフレッシュポイント.
//! \param uint32_t timeout		タイムアウト.
//! \param uint32_t debugInfo1	デバッグ情報1.
//! \param char *debugInfo2		デバッグ情報2.
//! <!--戻値-->.
//! \return  なし.
//! <!--詳細-->.
//! CA72WDT EXTのリフレッシュを行う.
//! <!--履歴-->.
// 2019/10/08 河村 初版.
// ================================================================================================.
void ca72wdt_ext_refresh( uint32_t point, uint32_t timeout, uint32_t debugInfo1, char *debugInfo2 )
{
	struct ca72_wdt_ext_data data;
	data.event = DEF_WDT_CA72_REFRESH;
	data.point = point;
	data.timeout = timeout;
	data.debugInfo1 = debugInfo1;
	if( debugInfo2 == NULL ) {
		data.debugInfo2[0]='\0';
	} else {
		strncpy( data.debugInfo2, debugInfo2, 240 );
		data.debugInfo2[239]='\0';
	}
	
	int32_t si_Ret2 = 0;
	if( si_Fd < 0 ) {
		si_Fd = SYSF_open(THRD_DeviceFile_Pmu, O_RDWR, 0);
		if (0 > si_Fd) {
			printf( "(%010u)(BSP) [%s] open failed. file=%s\n", SYSF_tickGet() , __func__, THRD_DeviceFile_Pmu);
		}
	}
	
	si_Ret2 = ioctl( si_Fd, THRD_PMU_IOCSSET_WDT_CA72_EXT, &data );	// REFRESH.
}

// ================================================================================================.
//! <!--機能--> CA72WDT EXTのリフレッシュ
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数--> 
//! \param なし.
//! <!--戻値-->.
//! \return  なし.
//! <!--詳細-->.
//! CA72WDT EXTの終了.
//! <!--履歴-->.
// 2019/10/08 河村 初版.
// ================================================================================================.
void ca72wdt_ext_end( void )
{
	int32_t si_Ret2 = 0;
	struct ca72_wdt_ext_data data;

	data.event = DEF_WDT_CA72_END;
	data.point = 0;
	data.timeout = 0;
	data.debugInfo1 = 0;
	data.debugInfo2[0]='\0';
	
	if( si_Fd < 0 ) {
		si_Fd = SYSF_open(THRD_DeviceFile_Pmu, O_RDWR, 0);
		if (0 > si_Fd) {
			printf( "(%010u)(BSP) [%s] open failed. file=%s\n", SYSF_tickGet() , __func__, THRD_DeviceFile_Pmu);
		}
	}
	
	si_Ret2 = ioctl( si_Fd, THRD_PMU_IOCSSET_WDT_CA72_EXT, &data );	// END.
}
// ================================================================================================.
//! <!--機能--> CA72WDT EXTの開始
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数--> 
//! \param uint32_t point		リフレッシュポイント.
//! \param uint32_t timeout		タイムアウト.
//! \param uint32_t debugInfo1	デバッグ情報1.
//! \param char *debugInfo2		デバッグ情報2.
//! <!--戻値-->.
//! \return  なし.
//! <!--詳細-->.
//! CA72WDT EXTの開始を行う.
//! <!--履歴-->.
// 2019/12/15 畔柳 初版.
// 					EagleZ+ OP_BTS-16479 CA72 Watchdog開始処理の呼び出しのため追加.
// ================================================================================================.
void ca72wdt_ext_start( uint32_t point, uint32_t timeout, uint32_t debugInfo1, char *debugInfo2 )
{
	struct ca72_wdt_ext_data data;
	data.event = DEF_WDT_CA72_START;
	data.point = point;
	data.timeout = timeout;
	data.debugInfo1 = debugInfo1;
	if( debugInfo2 == NULL ) {
		data.debugInfo2[0]='\0';
	} else {
		strncpy( data.debugInfo2, debugInfo2, 240 );
		data.debugInfo2[239]='\0';
	}
	
	int32_t si_Ret2 = 0;
	if( si_Fd < 0 ) {
		si_Fd = SYSF_open(THRD_DeviceFile_Pmu, O_RDWR, 0);
		if (0 > si_Fd) {
			printf( "(%010u)(BSP) [%s] open failed. file=%s\n", SYSF_tickGet() , __func__, THRD_DeviceFile_Pmu);
		}
	}
	
	si_Ret2 = ioctl( si_Fd, THRD_PMU_IOCSSET_WDT_CA72_EXT, &data );	// REFRESH.
}

// ================================================================================================.
//! <!--機能--> 電源管理 制御部 温度取得.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数--> 
//! \param int32_t *psi_Temperature:温度.
//! <!--戻値-->.
//! \return True：取得成功、False：取得失敗.
//! <!--詳細-->.
//! 温度取得用I/F.
//! <!--履歴-->.
// 	2020/11/02 Sky 新規作成 OP_BTS-28675 CPU高温異常対するフェールセーフ対応.
// ================================================================================================.
Bool THRC_PowerManager::getTemperature( int32_t *psi_Temperature )
{
	int32_t	si_Fd = 0;
	int32_t si_ret = 0;

	if( psi_Temperature == NULL )
	{
		printf( "(%010u)(BSP) [getTemperature] param err.\n", SYSF_tickGet());
		return False;
	}

	si_Fd = SYSF_open(THRD_DeviceFile_Pmu, O_RDWR, 0);
	if( si_Fd < 0 )
	{
		printf( "(%010u)(BSP) [getTemperature] open err. errno:%d\n", SYSF_tickGet() , SYSF_errnoGet());
		return False;
	}

	// 温度取得.
	si_ret = SYSF_ioctl(si_Fd, THRD_PMU_IOCSGET_TEMPERATURE, (intptr_t)psi_Temperature);
	if( si_ret < 0 )
	{
		printf( "(%010u)(BSP) [getTemperature] ioctl err. errno:%d\n", SYSF_tickGet() , SYSF_errnoGet());
		SYSF_close(si_Fd);
		return False;
	}

	SYSF_close(si_Fd);

	return True;
}

/* 2021/01/05 Harada IT6 2.1 OP_BTS-30215 認証装置を認証しない *//* 同件として対応 *//* 市場問題 TAS3758 E301 */
// ================================================================================================.
//! <!--機能--> 電源管理 制御部 Hub Port Status Failed制御.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数--> 
//! \param uintptr_t ul_Option：Hub Port Info.
//! <!--戻値-->.
//! \return  なし.
//! <!--詳細-->.
//! Hub Port Status Failed通知受信時の制御を行う。.
//! USB Hubをリセットする。.
//! <!--履歴-->.
// 	2021/01/05 Harada 初版.
// ================================================================================================.
void THRC_PowerManager::procHubPortStatusFailed(uintptr_t ul_Option)
{
	THRS_HubPortInfo s_HubPortInfo;
	char c_PortNum[24];

	s_HubPortInfo = *((THRS_HubPortInfo*)ul_Option);

	memset(c_PortNum, 0, sizeof(c_PortNum));
	snprintf(c_PortNum, sizeof(c_PortNum)-1 ,"%d-%s", s_HubPortInfo.i_busnum, s_HubPortInfo.c_devpath );

	THRF_USBHubReset(c_PortNum);
}
// ================================================================================================.
//! <!--機能--> 電源管理 SuspendPrepareで停止箇所特定.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数--> 
//! \なし.
//! <!--戻値-->.
//! \なし.
//! <!--詳細-->.
//! SuspendPrepareで停止箇所を特定し新詳細コード化する.
//! <!--履歴-->.
// 	2021/01/20 山口 OP_BTS-30528 Suspending console直前で停止.
// 	2021/04/13 武田 OP_BTS-29385 静的解析対応
// ================================================================================================.
void THRC_PowerManager::ResearchSuspendPrepare(void){

	
	int32_t	si_Fd = 0;
	int32_t si_Ret2 = 0;
	si_Fd = SYSF_open(THRD_DeviceFile_Pmu, O_RDWR, 0);
	if( si_Fd < 0 )
	{
		printf( "(%010u)(BSP) [%s] open failed. file=%s\n", SYSF_tickGet() , __func__, THRD_DeviceFile_Pmu);
		return ;
	}
	// PMUドライバーで停止箇所を特定し新詳細コード化.
	si_Ret2 = ioctl( si_Fd, THRD_PMU_IOCRESEARCHSUSPENDPREPARE, 0 );
	if( si_Ret2 < 0 )
	{
		printf( "(%010u)(BSP) [%s] ioctl err. errno:%d\n", SYSF_tickGet() , __func__, SYSF_errnoGet());
		SYSF_close(si_Fd);
		return ;
	}

	SYSF_close(si_Fd);
	// タスクモニタを取得(他必要そうなログも取得).
	PowerSave_DebugLog();
	
}

// 2021/07/27 ft_kondo OP_BTS-34942 RamDisk内のログ削除用監視処理追加. 
#define THRC_RAMDISK_MESSAGES_SIZE		10000000				// 10MB. 
#define THRC_RAMDISK_MESSAGES_LOGFILE	"/var/log/messages"		// logファイル. 
void checkRamDisk_Messages(void)
{
	STATUS st_status = 0;
	struct stat st_stat = {};
	
	while( 1 ){
		
		SYSF_taskDelay(SYSF_sysClkRateGet() * 180000 / 1000);	// 180s wait.

		// サイズ取得. 
		st_status = SYSF_stat( THRC_RAMDISK_MESSAGES_LOGFILE, &st_stat );

		if(st_status == 0){
			// サイズチェックしてサイズオーバーしていたら、ファイルを削除する. 
			if( THRC_RAMDISK_MESSAGES_SIZE < st_stat.st_size ){	// 10MB以上だったら削除する. 
				printf( "(%010u)(BSP) Messages Delete [%d]\n", SYSF_tickGet(), st_stat.st_size );
				SYSF_xdelete( THRC_RAMDISK_MESSAGES_LOGFILE );
			}
		}
	}
}

// ================================================================================================.
//! <!--機能-->kernelで発生した新詳細コードの取得.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \なし
//! <!--戻値-->.
//! \return uint64_t	新詳細コード.
//! <!--詳細-->.
//! シグナルハンドラにて通知を受けた後にこの関数で新詳細コードを読み出す\n.
//! <!--履歴-->.
// 2022/12/12 河村 初版
// ================================================================================================.
uint64_t THRC_PowerManager::getNewDetailCode(void)
{
	FILE *fp = NULL;
	char str[21] = {};	/* uint64_tは最大20桁 */
	
	if ((fp = popen("cat /sys/bus/platform/devices/PMU.0/devices/NEW_DETAIL_CODE", "r")) != NULL) {
		fgets(str, sizeof(str), fp);
		pclose(fp);

		int64_t si_code = 0;
		sscanf( str, "%llu", &si_code );
		return si_code;
	} else {
		return 0;
	}
}

// ================================================================================================.
//! <!--機能-->kernelで発生した新詳細コードはリブートするのかの取得.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \なし
//! <!--戻値-->.
//! \return Bool	True=リブート必要あり False=リブートしない.
//! <!--詳細-->.
//! シグナルハンドラにて通知を受けた後にこの関数でリブートの必要があるのかを読み出す\n.
//! <!--履歴-->.
// 2022/12/12 河村 初版
// ================================================================================================.
Bool THRC_PowerManager::getNewDetailCodeIsReboot(void)
{
	FILE *fp = NULL;
	char str[10] = {};
	
	if ((fp = popen("cat /sys/bus/platform/devices/PMU.0/devices/NEW_DETAIL_CODE_ISREBOOT", "r")) != NULL) {
		fgets(str, sizeof(str), fp);
		pclose(fp);

		int32_t si_isReboot = atoi( str );
		return si_isReboot==0?False:True;
	} else {
		return False;
	}
}
