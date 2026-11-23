//
//ファイル名：SYSC_.SystemInit.cpp
//
//概要：システム初期化用関数
//
//インクルード・ファイル-------------------------------------------------------------------------------------------------
#include <stdio.h>

#include <panel/plibl/plibl_CLCD/tskpnldraw_CLCD.h>

#ifdef SIMULATOR
#include "../../simulator/SimShared/SimCnvImg/SIMC_ScanAttrOutput.h"
#include "../../simulator/SimShared/SimCnvImg/SIMC_ActiveJobInfo.h"	// SimLin WBS(SIM345)関連対応
#endif

#include <stdlib.h>
#include <string.h>
#include "SYSC_SystemInit.h"
#include "SYSC_Enabler.h"
#include "system/sys/Logging/SYSC_ApiLogging.h"
#include "system/sys/Backup/SYSC_Backup.h"
#include "system/prt/APIC_Printer.h"
#include "system/scn/APIC_Scanner.h"
#include "system/sys/APIC_System.h"
#include "system/job/APIC_Job.h"
#include "system/job/APIC_JobList.h"
#include "SYSC_SwapControl.h"	// EagleH_制御構成変更_No.49 swap有効化対応(swap有効化遅延実行対応).

// FWリファクタリング APIC_Setting.hの分割.
#include "system/nvd/APIC_Setting_EnablerSetting.h"
#include "system/nvd/APIC_Setting_MachineSetting.h"
#include "system/nvd/APIC_Setting_Network.h"
#include "system/nvd/APIC_Setting_RdsSetting.h"
#include "system/nvd/APIC_Setting_SoftDipSw.h"
#include "system/nvd/APIC_Setting_SystemSetting.h"
#include "system/nvd/APIC_Setting_Other.h"
#include <sys/time.h>

#include "system/nvd/APIC_Counter.h"
#include "system/nvd/APIC_AddressBook.h"
#include "system/uac/APIC_UserAcount.h"
#include "system/doc/DOCG_InitModuleIf.h"
#include "system/dbg/DBGG_InitModuleIf.h"
#include "system/igl/IGLG_InitModuleIf.h"
#include "system/opl/OPLG_InitModuleIf.h"
#include "system/thr/PanelHW/THRC_PanelHardCom.h"
#include "system/sys/sysg/SYSC_AppOptionFuncInit.h"
#include "Linux/Platform/Mediaif.h"
#include "system/rss/APIC_ResourceScheduler.h"
#if !defined(SIMULATOR) || defined(DBG_IRMSIM)
#include "system/irc/IRMG_InitModuleIf.h"
#endif

#if defined(SIMULATOR) && defined(FUM_LINUX)
#include "../LocalUI/browser/browser/browserg/BRWC_Browser.h"
#endif

#include "system/doc/docg/DOCC_EncFileBox.h"

#include "system/doc/docg/DOCC_EncPrintCommon.h"

#include "sgw/SGWG_InitModuleIf.h"
#include "xmppm/APIC_XmppmIF.h"
#include "system/sys/APIC_Roim.h"
#include "system/sys/APIC_BackupRestore.h"
#include "system/sys/APIC_Fctm.h"

//2019/10/08 Sky 【自動測定】起動イベントの通知 start.
#include "system/thr/thrInclude/THRC_BSPif.h"
// 2023/04/12 [IT6_2.3] [OP_BTS-42639] FPT_KhanhNQ32 Start.
#include "system/sys/APIC_SystemInfo.h"
// 2023/04/12 [IT6_2.3] [OP_BTS-42639] FPT_KhanhNQ32 End.
//2019/10/08 Sky 【自動測定】起動イベントの通知 end.
// 2017/10/12 AR1030522 装置情報の取得が出来ない 日立 村上(幸) Start.
// 2017/12/18 リンクエラー対応 HIENG Start.
// #if defined( DEF_EagleCus ) || (defined(DEF_IT5EMU3) && defined(DEF_Helios_PF))
// 2018/02/22 IT6Com参照判定の変更 日立 中井 MOD Start.
//#if defined( DEF_EagleCus ) || (defined(DEF_IT5EMU3) && defined(DEF_Helios_PF)) || defined(DEF_SparrowCus)

// 2019/05/06 EagleZ+/EagleBKZ+  【Control No.1】 HIENG START.
// #if !defined( COMMON_REF_IT5COM ) || defined( DEF_EagleBKCus ) // 2019/03/22 【EagleBK】 【Control_No.1】 ROM version HIENG.
// 2019/06/17 SparrowBK [Control No.1] HIENG Start.
// #if !defined( COMMON_REF_IT5COM ) || defined( DEF_EagleBKCus ) || defined( DEF_EagleZpCus ) // EagleZ Plus.
// 2021/03/19 MinervaSSBK  【Control No.1】 HIENG START.
//#if !defined( COMMON_REF_IT5COM ) || defined( DEF_EagleBKCus ) || defined( DEF_EagleZpCus ) || defined( DEF_SparrowBKCus ) // SparrowBK.
#if !defined( COMMON_REF_IT5COM ) || defined( DEF_EagleBKCus ) || defined( DEF_EagleZpCus ) || defined( DEF_SparrowBKCus ) || defined( DEF_MinervaSSBKCus ) // MinervaSSBK.
// 2021/03/19 MinervaSSBK  【Control No.1】 HIENG END.
// 2019/06/17 SparrowBK [Control No.1] HIENG End.
// 2019/05/06 EagleZ+/EagleBKZ+  【Control No.1】 HIENG END.

// 2018/02/22 IT6Com参照判定の変更 日立 中井 MOD End..
// 2017/12/18 リンクエラー対応 HIENG End.
// 2017/10/02 IT6 Eagle 制御構成変更_No.1 日立米川 Start.
#include "system/sys/SystemInfo/SystemInfoIT6Com/SYSC_SystemInfoIT6Com.h"
// 2017/10/02 IT6 Eagle 制御構成変更_No.1 日立米川 End.
#else
	#include "system/sys/SystemInfo/SystemInfoZeus5BkCom/SYSC_SystemInfoZeus5BkCom.h"
#endif
// 2017/10/12 AR1030522 装置情報の取得が出来ない 日立 村上(幸) End.
#include "system/opl/oplg/OPLC_OPL.h"
#include "system/jlg/JLGG_InitModuleIf.h"
#include "system/plg/PLGG_InitModuleIf.h"
#include "system/job/JOBG_InitModuleIf.h"
#include "service/jobs/JSVG_InitModuleIf.h"
#include "system/uac/UACG_InitModuleIf.h"
#include "system/uac/ASMG_MFP_InitModuleIf.h"
#include "system/prt/PRTG_InitModuleIf.h"
#include "system/scn/SCNG_InitModuleIf.h"
#include "system/cnv/CNVG_InitModuleIf.h"
#include "system/ats/ATSG_InitModuleIf.h"
#include "system/cpg/CPGG_InitModuleIf.h"
#include "printApp/PAPG_InitModuleIf.h"
#include "pdl/PDLG_InitModuleIf.h"
#include "system/pdi/PDIG_InitModuleIf.h"
#include "system/pdr/PDRG_InitModuleIf.h"
#include "system/lst/LSTG_InitModuleIf.h"
#include "system/rms/RMSG_InitModuleIf.h"
#include "system/pwr/PWRG_InitModuleIf.h"
#include "system/nvd/NVDG_InitModuleIf.h"
#include "system/img/IMGG_InitModuleIf.h"
#include "system/dbm/DBMG_InitModuleIf.h"
#include "netrx/netprintrx/NETG_InitNetPrintRxIf.h"
#include "netrx/netfaxrx/NETG_InitNetFaxRxIf.h"
#include "netrx/PrintRx/PRXG_InitPrintRxIf.h"
#include "netrx/WebDAV/WDAG_InitWebDavRxIf.h"
#include "netrx/fnt/FNTG_InitFontRxIf.h"
//#include "netrx/mbl/MBLG_InitMobilePhoneRxIf.h"
#include "system/fil/FILG_InitModuleIf.h"
#include "system/edt/EDTG_InitModuleIf.h"
#include "system/ntx/NTXG_InitModuleIf.h"
#include "system/ntfx/NTFXG_InitModuleIf.h"
#include "net/NETG_InitTCPSocketIf.h"
#include "lib/enabler/random_generate.h"
#include "lib/tpm/include/tpmMgrApi.h"
#include "system/fax/FAXG_InitModuleIf.h"
#include "system/edt/EDTG_InitModuleIf.h"
#include "system/sys/Timer/SYSC_Timer.h"
#include "system/sys/Timer/SYSC_RDSTimer.h"
#include "system/sys/APIC_AppLicense.h"
// 2017/12/19 【Eagle】【Simlin】 hitachi  Phat-Dang Modify Start.
// #if defined(SIMULATOR) && !defined(FUM_LINUX)
// #include "system/sys/Database2/SYSC_Database.h"
// #include "system/sys/Database2/SYSC_VDBM.h"
// #else
#include "system/sys/Database/SYSC_Database.h"
#include "system/sys/Database/SYSC_VDBM.h"
// #endif
// 2017/12/19 【Eagle】【Simlin】 hitachi  Phat-Dang Modify End.

#include <panel/pnlROMVer/PNLC_RomVersionDef.h>

#include "system/thr/thrInclude/THRC_PortInstance.h"
#include "system/thr/PanelHW/THRC_tskXPanelif.h"

#include "system/prv/PRVG_InitModuleIf.h"

#include "system/thr/thrInclude/THRC_Port.h"
#include "system/thr/thrInclude/THRC_TroubleCounter.h"

// 2018/04/16 Eagle 制御構成変更No.110 不具合解析強化対応 日立 岡部 Start.
//#include "system/sys/sysc/SYSC_SysLogALLToMedia.h"
#include "system/dbg/dbgg/DBGC_SysLogALLToMedia.h"
// 2018/04/16 Eagle 制御構成変更No.110 不具合解析強化対応 日立 岡部 End.
#include "rds/RDSG_InitModuleIf.h"
#include "lca/LCA_InitModuleIf.h"
#include "system/exc/EXCG_InitModuleIf.h"
#include "system/sys/sysc/SYSC_LogToCF.h"
#include "system/sys/sysg/SYSC_System.h"


#ifndef DEF_MoselCom	// 構成別・機種別の立ち上げ用に必要なコンパイルＳＷ 
#include "system/cnt/CNTC_CntIf.h"
#endif
#include "system/sys/KeyDevice/SYSC_VndCcmagcard.h" 
#include "system/sys/FsWrapper/SYSC_FsUSBWrapperC.h"
#include "system/sys/APIC_Music.h"		// MTUIv1 反応音導入.

#include "rmp/RMP_WebDavClient.h"
#include "rmp/RMP_WebDavServer.h"

#include "system/thr/PanelHW/THRC_PanelHardCom.h"

#include "system/thr/thrInclude/THRC_Threadif.h"
// 2016/03/24 OPS1-Sky Kawanishi [SecV2][SecV2-9] セキュリティー一括設定.
#include "system/sys/sysg/SYSC_SecurityAutoSetting.h"

// 2017/12/19 【Eagle】【Simlin】 hitachi  Phat-Dang Modify Start.
// #if defined(SIMULATOR) && !defined(FUM_LINUX)
// #warning 暫定処置 ブラウザが居ないのでフラグだけ更新するためインクルードを追加
// //2016/03/23【MinervaMLK】ブラウザincludeパス変更対応 日立 田澤 MOD Start.
#include "browser/browserg/BRWC_Browser.h"
// #include "../LocalUI/browser/browser/browserg/BRWC_Browser.h"
// //2016/03/23【MinervaMLK】ブラウザincludeパス変更対応 日立 田澤 MOD End.
// #endif
// 2017/12/19 【Eagle】【Simlin】 hitachi  Phat-Dang Modify End.
#ifndef SIMULATOR
#include "../LocalUI/browser/browser/browserg/BRWC_Browser.h"
#include "../../../ftp/FTP_Server.h"
#include "../../../ftp/RCD_Account.h"
#endif //#ifdef SIMULATOR
#ifdef TORNADO
#ifdef DEF_Donau
#ifndef LINUX
#include "Mediaif.h"
#endif	// #ifndef LINUX 
#else
#include "sysFlashrom.h"
#endif
#endif
// 2017/12/19 【Eagle】【Simlin】 hitachi  Phat-Dang Modify Start.
// #ifdef SIMULATOR
// extern void dbgSetImageLog(uint32_t ulSet);
// #endif
// 2017/12/19 【Eagle】【Simlin】 hitachi  Phat-Dang Modify End.

// CQ.378292(Act_63656) MS-EVP-94 起動パフォーマンス計測処理埋め込み対応.
extern void SetLapTime(char flag, unsigned int seqno);

// サービス全体の初期化用.
#include "service/base/MFPC_ServiceBase.h"
// SystemStatusサービス.
#include "service/APIC_SystemStatusServiceIF.h"
// Maintenanceサービス.
#include "service/APIC_MaintenanceServiceIF.h"
// DataCacheモジュール.
#include "system/dcm/DCMC_CacheDataIF.h"
#include "service/APIC_DataCacheModuleIF.h"
// Authenticationサービス.
#include "service/auts/AUTC_AuthService.h"
// Externalサービス.
#include "service/exts/EXTC_ExternalService.h"
// Storageサービス.
#include "service/strs/STRS_StorageService.h"
// SystemStatusサービス.
#include "service/ssts/SSTC_SystemStatusService.h"

// Chargingサービス.
#include "service/chrs/CHRC_ChargingServiceBase.h"
// Deviceサービス.
#include "service/APIC_DeviceServiceIF.h"
// Powerサービス.
#include "service/APIC_PowerServiceIF.h"

// FaxDialサービス.
#include "service/faxs/FXDC_FaxDialService.h"

// AppDirectPrintサービス.
#include "service/adps/ADPC_AppDirectPrintService.h"
#include "netrx/adp/ADPC_InitAppDirectPrintRX.h"

#include	"system/sys/APIC_VendorAppManage.h"

#include "system/thr/AndroidCtrl/THRG_AndroidInstance.h"	// 遠隔操作Androidパネル対応.

#if defined(DEF_A4_Cus) && defined(DEF_KRONOSS_Cus)
#include "system/thr/thrcom/THRC_LxNvControl.h"
#include "system/thr/thrcom/THRC_LxCrashMon.h"
//2017/05/09 日立井口 新規追加 KronosS 管理No.16 トラブルコード仕様変更(ESD試験対応)start.
#include "system/sys/sysg/TroubleReboot/SYSC_TroubleReboot.h"
//2017/05/09 日立井口 新規追加 KronosS 管理No.16 トラブルコード仕様変更(ESD試験対応)end.
#endif // defined(DEF_A4_Cus) && defined(DEF_KRONOSS_Cus).

#ifdef SIMULATOR
#include "../../simulator/SimShared/include/SIMC_SoundObserverIf.h"
#endif

#include "system/sys/APIC_FatalSignalDetailCode.h"

#include "system/sys/Dbms/DAPG_ModuleIf.h"
#include "system/sys/webapi/SYSC_WebAPI.h"

#include "system/sys/VirusScan/SYSC_VirusScan.h"
#include "system/sys/VirusScan/SYSC_VirusScanMem.h"
#include "system/sys/VirusScan/SYSC_VirusScanLog.h"
#include "system/sys/VirusScan/SYSC_VirusScanDirTimer.h"
#include "system/sys/VirusScan/SYSC_VirusScanUpdateTimer.h"
#include "system/sys/VirusScan/SYSC_VirusScanVersion.h"

#include "SYSC_SystemNotifyUsbInfo.h"

#if defined( TORNADO ) && defined( ROM_DIV_3 )
extern char *pt_mioSetFuncTblToGetName;
extern char *pt_mioKMVersionString;
extern char *pt_MIO_taskSpawn;
extern char *pt_mioSetDeviceInfo;
#endif

extern void mioSetModelName();
extern void sysc_SetSecurityLevelUpOnToMio();
extern void sysc_SetDeviceInfoToMio();
extern SYMTAB_ID	sysSymTbl;
extern STATUS symFindByName(SYMTAB_ID symtblId, char* name, char** pValue, SYM_TYPE* pType);
#ifdef DEF_SupportSSD
extern void initOSdriverSSD();
#endif	/*#ifdef DEF_SupportSSD*/

extern void PNL_setVersionForHDD( );

extern int copy_InstallConfig();	// インストール管理ファイルのコピー.

#include "system/thr/thrInclude/THRC_Threadif.h"
#ifdef TORNADO
#if !defined(DEF_Helios_PF)
#define INIT_REC_START()	initDebugRunningFlagSet( initDebugModule_THR, 1, 0 )
#define INIT_REC( uc_data )	{ int recLine = __LINE__; initDebugWrite( initDebugModule_THR, 20, &recLine, sizeof(int) ); }
#define INIT_REC_END()		initDebugRunningFlagReset( initDebugModule_THR, 1, 0 )
#else	// #if !defined(DEF_Helios_PF).
extern void refresh_SoftDog( void );
#define INIT_REC_START()		initDebugRunningFlagSet( THRD_OFFSET_INITDEBUG_INIT_SEC1, THRD_OFFSET_INITDEBUG_INIT_SEC2 )
#define INIT_REC( uc_data )		{ initDebugWrite( 0, 0, NULL, 0, uc_data ); refresh_SoftDog(); } 
#define INIT_REC_END()			initDebugRunningFlagReset()
#endif	// #if !defined(DEF_Helios_PF).
#else // #ifdef TORNADO
#define INIT_REC_START()	initDebugRunningFlagSet( initDebugModule_THR, 1, 0 )
#define INIT_REC()			{ int recLine = __LINE__; initDebugWrite( initDebugModule_THR, 20, &recLine, sizeof(int) ); }
#define INIT_REC_END()		initDebugRunningFlagReset( initDebugModule_THR, 1, 0 )
#endif // #ifdef TORNADO

// 2018/09/07 MFPソリューション分離対応Step2 (Add-Start).
#include "../../shlib/RecoveryLib/LIBC_RecoveryLib_IF.h"

extern pid_t child_pid[];
extern int child_cnt;
extern pid_t mainThread_pid;
// 2018/09/07 MFPソリューション分離対応Step2 (Add-End).

#ifdef INCLUDE_MIO
extern "C" {
	void taskAPI_MIB();
	void taskAPI_IPP();

	STATUS mioDrv(void);
	STATUS mioInstallDevices(void);
	void mioInit(void);
}
#endif	// INCLUDE_MIO

#ifdef LINUX
// JScribe初期化処理追加.
#if !defined(COSIM)
extern "C"{
extern int jscf_init();
}
#endif
#endif

#include "netrx/smbd/smbdg/SMBC_SMBAuth.h"
#include "netrx/smbd/smbdg/SMBC_SMBShareClean.h"

#include "system/sys/InternetISW/SYSC_InternetISW.h"

#include "system/sys/sysg/SYSC_FuncPatternSetting.h"

#include	"system/thr/thrInclude/ThrCommonDrvrIf.h"
#include	"system/thr/thrInclude/THRC_Threadif.h"

#include "system/sys/InternetISW/SYSC_IISWClient.h"

#include "system/sys/InternetISW/SYSC_FWPreserve.h"

#include "system/nvd/APIC_Func.h"

#include "system/sys/APIC_OptionSetting.h"

#include "system/sys/sysg/SYSC_IwsUserAccount.h"

#include "system/sys/Music/SYSC_BGM.h"

#include "system/sys/SystemStatus/SYSC_SystemStatus.h"

// 2017/02/20 Sky酒井（純）ユーザーイベント記録(APIコール).
#include "system/sys/OsWrapper/SYSC_OSWrapperTaskMonitorLinux.h"
// 2021/08/04 MinervaSSBK [DEVALL_OP-965] HIENG Start.
#include "system/nvd/nvdg/NVDC_FlashMemory.h"
// 2021/08/04 MinervaSSBK [DEVALL_OP-965] HIENG End.
extern void setLoopBootCnt(unsigned char);	// 起動回数カウンタ設定.
extern void setLoopBootCntBoot(unsigned char);	// 起動回数カウンタ(U-Boot)設定.

extern void setLoopBootErrDetailCode(void);	// 詳細コード記録.
#include "system/sys/APIC_MediaMapFolder.h"

extern int32_t NVDC_MediaAccess_Init(void);		/* メディアマップファイル読み込み */

extern void NVDC_MediaAccess_checkFatalError(void);		/* メディアマップファイル読み込みエラーチェック */
#include "system/nvd/APIC_FWUpdateSetting.h"
#include "system/nvd/APIC_JobDJKeyInfo.h"
#include "system/nvd/nvdg/elapsed_time_log.h"

#include "system/sys/AndroidPanel/SYSC_AndroidPanel.h"

//【CQ 516434】【ZeusS-ZXO】【RQ-383bizhubMarketPlaceのダウンローダーの搭載】【HCL_Sagar】2015/09/02 Add Start.
#include "system/sys/sysg/SYSC_IWSAppInfo.h"
//【CQ 516434】【ZeusS-ZXO】【RQ-383bizhubMarketPlaceのダウンローダーの搭載】【HCL_Sagar】2015/09/02 Add End.
extern "C" {
extern void DBGF_PrintNoOperationPath(char* psc_FileName, int32_t sl_LineNo); 
}

#include "system/nvd/aci/ACIG_InitModuleIf.h"

// 2016/2/3【RQ-503】WW指示サーバ運用の拡張 日立 谷 Start.
#include "system/sys/sysg/SYSC_Distributer.h"
// 2016/2/3【RQ-503】WW指示サーバ運用の拡張 日立 谷 End.

// 2016/09/27 IT5_4.0 RQ-421 装置自動更新機能の機能拡張 日立 江上 ADD Start.
#include "system/sys/sysg/SYSC_RDTManager.h"
// 2016/09/27 IT5_4.0 RQ-421 装置自動更新機能の機能拡張 日立 江上 ADD End.

#include "system/sys/sysg/SYSC_DeviceReport.h"		// 2018/04/09 DenebMLK [Control_No_67] HIENG ADD.
// 2018/05/07【MAv6】汎用WebDAVクライアント対応 日立 西原 Start.
#include "system/sys/APIC_WebDAVTx.h"
// 2018/05/07【MAv6】汎用WebDAVクライアント対応 日立 西原 End.
// 2022/03/01 RQ-931 Sky Start.
#include "system/sys/APIC_WebDAVc.h"
// 2022/03/01 RQ-931 Sky End.
#include "system/nvd/APIC_NVBackup.h"				// 2019/01/08 EagleH [Control_No.60] HIENG Add.

#include "system/sys/sysg/SYSC_EditData.h"

#include "system/sys/sysg/SYSC_MarketPlaceData.h"
// 2022/03/01 RQ-931 Sky Start.
#include "system/sys/sysg/SYSC_RmtControlOperation.h"
// 2022/03/01 RQ-931 Sky End.

//定義-------------------------------------------------------------------------------------------------------------------
// ROMバージョンの定義 
#include "system/sys/RomVersion/SYSC_RomVersion.h"
#include "system/sys/APIC_Timer.h"
// ROMバージョンの設定.


#if defined(DEF_MinervaBKCus) || defined(DEF_MinervaBKEmu) //MinervaBK.
#include "system/thr/thrDonau/THRC_IPRomParaDonauData.h"
#define FN_PIC_IPROM_SO		"/km/fw/lib/libPicMinervaBKIprom0.so.1"
extern unsigned char* IPROM_CPSPIC_INT_Ver;

#elif	defined(DEF_ZeusMLKEmu) || \
	defined(DEF_ZeusMLKCus) && !defined(DEF_HELIOS_EMU2) && !defined(DEF_Asic777_PMS) && !defined(DEF_S800) && !defined(DEF_ARMADAXP) || \
	defined(DEF_Zeus5BkCus) && !defined(DEF_Helios_PF) && !defined(DEF_HELIOS_EMU2)

#if defined(DEF_Asic777_PMS) || defined(DEF_S800)  // ZeusS.
#include "system/thr/Asic777/THRC_IPRomParaData.h"
#else
#include "system/thr/thrSoC/THRC_IPRomParaSoCData.h"
#endif

#define FN_PIC_IPROM_SO		"/km/fw/lib/libPicZeusMLKIprom0Beam.so.1"
extern unsigned char* IPROM_CPSPIC_INT_Ver;

#endif
#if defined(DEF_ZEUSS_ZX0_CUS) && !defined(DEF_IT5EMU3) && !defined(DEF_S800)	//制御構成変更No44対応 OPS1-sky_simogaki.
#include "system/thr/Asic777/THRC_IPRomParaData.h"
extern unsigned char* IPROM_CPSPIC_INT_Ver;
static const char* const FN_PIC_IPROM_SO =		"/km/fw/lib/libPicZeusSZIprom0Beam.so.1";
#endif // ZeusSZX0.

//↓ 2016/08/31 Sky合屋 [IT5_4.0]制御構成変更No.48.
#if defined( DEF_CCD_CHG_ENABLE )
#include "system/thr_ZeusS/Asic777/THRC_IPRomParaData.h"
extern unsigned char* IPROM_CPSPIC_INT_Ver;
extern unsigned char* p_irc_ShLib_ver;
#if defined( DEF_Zeus5BkCus ) // Poseidon.
#define FN_PIC_IPROM_SO  "/km/fw/lib/libPic_bh287_Iprom.so"
#define FN_IRC_IPROM_SO  "/km/fw/lib/libIrc_bh287.so"
#elif defined( DEF_ZEUSS_CUS ) // ZeusS.
#define FN_PIC_IPROM_SO  "/km/fw/lib/libPic_bhC368_Iprom0.so"
#define FN_IRC_IPROM_SO  "/km/fw/lib/libIrc_bhC368.so"
#elif defined( DEF_HeliosCus ) // Helios.
#define FN_PIC_IPROM_SO  "/km/fw/lib/libPic_bhC287_Iprom0.so"
#define FN_IRC_IPROM_SO  "/km/fw/lib/libIrc_bhC287.so"
#elif (defined(DEF_ZEUSSBK12_CUS) && !defined(DEF_ZEUSS_BASE_EMU))	// ZeusSBK 1/2 2016.10.25【IT5_4.0】制御構成変更No48「CCD切り替え対応」(ZeusSBK追加).
#define FN_PIC_IPROM_SO  "/km/fw/lib/libPic_bh368_Iprom0.so"
#define FN_IRC_IPROM_SO  "/km/fw/lib/libIrc_bh368.so"
#elif defined(DEF_ZEUSSBKX0_CUS)	// ZeusSBK X/0 2016.10.25【IT5_4.0】制御構成変更No48「CCD切り替え対応」(ZeusSBK追加).
#define FN_PIC_IPROM_SO  "/km/fw/lib/libPic_bh558_Iprom0Beam0.so"
#define FN_IRC_IPROM_SO  "/km/fw/lib/libIrc_bh558.so"
#endif
#endif //defined( DEF_CCD_CHG_ENABLE ).
//↑ 2016/08/31 Sky合屋 [IT5_4.0]制御構成変更No.48.

// 2018/10/19 Sky松井	[Eagle_High]制御構成変更No.47.
#ifndef SIMULATOR
#if ( defined( DEF_EagleCus ) && !defined( DEF_S800Emu ) && defined( DEF_S800 ) && !defined( DEF_A800 ) && !defined( DEF_HeliosMLKCus ) ) // Eagle Low(1/2/2.5)Eagle High(0).
#define FN_PIC_IPROM_SO "/km/fw/lib/libIrc_bhC.so"
#define FN_THR_ICC_SO	"/km/fw/lib/libThr_Icc.so"
extern unsigned char* IPROM_CPSPIC_INT_Ver;
extern char* IP_Ver;
extern unsigned char* CPS_ICC_Ver;
#endif // ( defined( DEF_EagleCus ) && !defined( DEF_S800Emu ) && defined( DEF_S800 ) && !defined( DEF_A800 ) && !defined( DEF_HeliosMLKCus ) ).
#endif // SIMULATOR.

#ifndef SIMULATOR	// 2016/02/24 maeda	[ZeusSBK][制御構成変更No27]プリント画像パラメータ設計.
#if (defined(DEF_ZEUSSBK12_CUS) && !defined(DEF_ZEUSS_BASE_EMU)) || defined(DEF_ZEUSSBKX0_CUS)
#include "system/thr_ZeusS/Asic777/THRC_IPRomParaData.h"
extern unsigned char* IPROM_CPSPIC_INT_Ver;
#define FN_PIC_IPROM_SO		"/km/fw/lib/libPicZeusSBKIprom0Beam0.so.1"
#endif // ZeusSBK.
#endif // #ifndef SIMULATOR.

// 2016/12/06 Sky鈴木	[IT5_4.1]制御構成変更No.05 画像設計 画パラ分離対応(ZeusSBK2nd Z/X/0).
#if defined(DEF_ZEUSSBK2ND_ZX0_CUS)
#include "system/thr_ZeusS/Asic777/THRC_IPRomParaData.h"
extern unsigned char* IPROM_CPSPIC_INT_Ver;
#define FN_PIC_IPROM_SO		"/km/fw/lib/libPic_bh658e_Iprom0Beam.so"
#endif // defined(DEF_ZEUSSBK2ND_ZX0_CUS)

#if defined(DEF_A3A4COMM)
#include "system/sys/ConfigInfoManage/SYSC_ConfigInfoManage.h"
#endif // defined(DEF_A3A4COMM)

// 2022/02/01 APF OP_BTS-37216 BlackAPIログ強化対応 start.
#include "../../../../divlib/Common/DIV_BaseLibrary/DebugUtility.h"
// 2022/02/01 APF OP_BTS-37216 BlackAPIログ強化対応 end.

// 2023/11/07 [IT6_2.3][OP_BTS-45304] FPT-TUANDA33 START.
#include "system/nvd/nvdg/NVDC_Setting_CopyScanJobSetting.h"
// 2023/11/07 [IT6_2.3][OP_BTS-45304] FPT-TUANDA33 END.

#if defined(LINUX)
extern char* GetSysRomVersion(char* psc_buf);
#endif

#ifdef TORNADO
#ifdef ROM_DIV_2
extern void appInit();
#endif	/* #ifdef ROM_DIV_2 */

extern "C"{
extern int32_t FtpServerInit();
extern int32_t WebDAVdDeviceAppInit();// i-Option WebDAV Device App.
extern int32_t WebDAVdOapDeviceAppInit();		// i-option特ROM WebDAV Device App(OAP連携) 
extern int32_t WebDAVdBackupRestoreDeviceAppInit(); // 2016/01/20【RQ-505】 バックアップサーバ化対応 日立 喜多. 
extern int32_t AccountInit();
extern Bool initAccountTask();
extern void checkLineModeInTar();
extern void checkBootSelfCheckMode();
extern void checkSpeedVariableMode();		// 速度可変起動モード.
#ifndef ROM_DIV_2
extern void appInit();
#endif	/* #ifdef ROM_DIV_2 */
}
#endif

#ifdef TORNADO          // 実機で有効 
#ifdef INCLUDE_MIO
extern "C" {
extern void MIO_taskSpawn();
}
#endif
#endif

#if !defined( TORNADO )
extern "C" {
	void InitIGC();
}
#endif

extern ushort us_tAppInitProgress;	// 2020/10/27 詳細コードC311対応.

#ifdef DEF_A4_Cus
#include "lib/LIBC_SimpleLog.h"															// シンプルログクラスヘッダー。
LIBC_SimpleLog*	gpSimpleLogForSys = new LIBC_SimpleLog("system/sys/",128,128);			// sys用シンプルログインスタンス128行× (128バイト(+1バイト))。

extern "C" void ShowSimpleLogForSys()
{
	if( gpSimpleLogForSys!=0 ) gpSimpleLogForSys->out();
}
#endif
// 2019/06/07 EagleH [OP_BTS-5581] HIENG Start.
uint64_t createCPUNewDetailCode( char *pc_SerialNo );
uint64_t createPhisonSSDDetailCode( char *pc_SerialNo );
uint64_t createToShiBaSSDDetailCode( char *pc_SerialNo );
uint64_t createOtherSSDDetailCode( char *pc_SerialNo );
uint64_t createPhisonAdataMicroSDDetailCode( char *pc_SerialNo );
// 2019/06/07 EagleH [OP_BTS-5581] HIENG End.
// 2020/01/23 日立 馬場 add Start.
uint64_t createMicronSSDDetailCode( char *pc_SerialNo );
// 2020/01/23 日立 馬場 add End.
extern void usbInitFunc();		// Thread USBキーボード初期化関数.
void WaitSystemConfigurationFixed( void );
t_sMioFuncTblToGetName MioFuncTbl,*pMioFuncTbl;	
// 製品名取得用関数 
STATUS nmGetModelName(UCHAR *pModelName)
{
 	char* sc_DeviceName = NULL;
  	APIC_SystemInfo* pc_SystemInfo = APIC_SystemInfo::newInstance();
  	pc_SystemInfo->lock();
  	sc_DeviceName = pc_SystemInfo->getModelName();		// 製品名を取得する 
	TYPE_MachineType e_Type = pc_SystemInfo->getMachineType();	// 2023/04/25 [IT6_2.3] [OP_BTS-42639] FPT_KhanhNQ32 Added.
  	pc_SystemInfo->unlock();
  	short ss_Size = static_cast<short>(strlen( sc_DeviceName ));
// 2023/04/25 [IT6_2.3] [OP_BTS-42639] FPT_KhanhNQ32 Start.
// 2023/04/12 [IT6_2.3] [OP_BTS-42639] FPT_KhanhNQ32 Start.
//	if(APIC_SystemInfo::newInstance()->getMachineType() == TYPE_MachineTypeSparrow2AIO){
	if( ( TYPE_MachineTypeSparrow1SFP   == e_Type )	||
		( TYPE_MachineTypeSparrow2SFP   == e_Type )	||
		( TYPE_MachineTypeSparrowBK0SFP == e_Type )	||
		( TYPE_MachineTypeSparrow2AIO   == e_Type ) )
	{
// 2023/04/25 [IT6_2.3] [OP_BTS-42639] FPT_KhanhNQ32 End.
		memset(pModelName, '\0', ss_Size);
		pModelName[0] = '-';
	} else {
		if( sc_DeviceName[ss_Size-1] == 'P' ) {
			memcpy( reinterpret_cast<char*>(pModelName), sc_DeviceName, ss_Size );
			pModelName[ss_Size-1] = '\0';	/* (*1) */
		} else {
			memcpy( reinterpret_cast<char*>(pModelName), sc_DeviceName, ss_Size+1 );
		}
	}	
// 2023/04/12 [IT6_2.3] [OP_BTS-42639] FPT_KhanhNQ32 End.
	return OK;
}
// OEM名取得用関数 
STATUS nmGetCompanyName(UCHAR *pCompanyName)
{
 	char* sc_Manufacturer = NULL;
  	APIC_SystemInfo* pc_SystemInfo = APIC_SystemInfo::newInstance();
  	pc_SystemInfo->lock();
  	sc_Manufacturer = pc_SystemInfo->getManufacturer();		// メーカー名を取得する 
  	pc_SystemInfo->unlock();
	strcpy( reinterpret_cast<char*>(pCompanyName), sc_Manufacturer );
	return OK;
}
// OEM短縮名取得用 
STATUS nmGetCompanyAbbrevName(UCHAR *pCompanyAbbrevName)
{
 	char* sc_ShortMfg = NULL;
 	APIC_SystemInfo* pc_SystemInfo = APIC_SystemInfo::newInstance();
 	pc_SystemInfo->lock();
  	sc_ShortMfg = pc_SystemInfo->getShortMfg();	// 省略メーカー名を取得する 
  	pc_SystemInfo->unlock();
	strcpy( reinterpret_cast<char*>(pCompanyAbbrevName), sc_ShortMfg );
	return OK;
}
static const int32_t TYPD_SetModelNameTimeout = 3*60*200L;	// タイムアウト値: 3分 
void mioSetModelName()
{
	// NICFUMに製品名/OEM名/OEM短縮名の取得関数を通知する 
	memset( &MioFuncTbl, 0, sizeof(t_sMioFuncTblToGetName) );
	pMioFuncTbl = &MioFuncTbl;
	pMioFuncTbl->mioGetModelName 		= nmGetModelName;		// テーブルに関数の登録を行う 
	pMioFuncTbl->mioGetOEMName 			= nmGetCompanyName;
	pMioFuncTbl->mioGetOEMAbbrevName 	= nmGetCompanyAbbrevName;
#ifdef TORNADO		// 実機版 
#ifdef ROM_DIV_3
	if( pt_mioSetFuncTblToGetName != NULL )
	{
		(reinterpret_cast<STATUS(*)(t_sMioFuncTblToGetName *)>(pt_mioSetFuncTblToGetName))( pMioFuncTbl );						// 関数テーブルをNICFUMに通知する 
	}
	else
	{
#ifdef TORNADO
		SYSF_logMsg("■□mioSetModelName() pt_mioSetFuncTblToGetName NULL pointer.□■\n",1,2,3,4,5,6);
#endif
	}
#else
	mioSetFuncTblToGetName( pMioFuncTbl );						// 関数テーブルをNICFUMに通知する 
#endif
#endif
}

extern void NVDF_setSecurityLevelUpOn( Bool b_SecurityLevelUpOn );
void sysc_SetSecurityLevelUpOnToMio()
{
	Bool b_Security = False;
	APIC_SystemSetting* pc_SystemSetting = NULL;
	pc_SystemSetting = APIC_SystemSetting::newInstance();
	b_Security = pc_SystemSetting->getSecurityLevelUpOn();
	NVDF_setSecurityLevelUpOn( b_Security );
}
void sysc_SetDeviceInfoToMio()
{
	APIC_SystemInfo* pc_SystemInfo = NULL;
	pc_SystemInfo = APIC_SystemInfo::newInstance();
	pc_SystemInfo->SetDeviceInfoToMio( TYPE_MioSetDeviceSelect_TIMEZONE );
	pc_SystemInfo->SetDeviceInfoToMio( TYPE_MioSetDeviceSelect_CODESET );
	pc_SystemInfo->SetDeviceInfoToMio( TYPE_MioSetDeviceSelect_HDD_ENABLE );
	pc_SystemInfo->SetDeviceInfoToMio( TYPE_MioSetDeviceSelect_SUMMERTIME );
	pc_SystemInfo->SetDeviceInfoToMio( TYPE_MioSetDeviceSelect_FIPS );
	pc_SystemInfo->SetDeviceInfoToMio( TYPE_MioSetDeviceSelect_RemotePanel );
	pc_SystemInfo->SetDeviceInfoToMio( TYPE_MioSetDeviceSelect_RemotePanel_MsgTimeout );
#if defined(DEF_ZeusMLKBoard)
#if !defined(SIMULATOR)
	if(( pc_SystemInfo->getExNetworkAdapter() == TYPE_ExNetworkAdapter_Wireless ) &&
	   ( THRF_isExistWLAN() == True )){
		pc_SystemInfo->SetDeviceInfoToMio( TYPE_MioSetDeviceSelect_WLAN_INITIALIZE );
	}
#endif // SIMULATOR.
#endif	// #if defined(DEF_ZeusMLKBoard).
	pc_SystemInfo->SetDeviceInfoToMio( TYPE_MioSetDeviceSelect_DHCP_Option60 );
	pc_SystemInfo->SetDeviceInfoToMio( TYPE_MioSetDeviceSelect_WSD_WSPRINT_V20 );
	pc_SystemInfo->SetDeviceInfoToMio( TYPE_MioSetDeviceSelect_SMBClientEnable_SMB2 );
	pc_SystemInfo->SetDeviceInfoToMio( TYPE_MioSetDeviceSelect_FAX_INFO );
	pc_SystemInfo->SetDeviceInfoToMio( TYPE_MioSetDeviceSelect_RESEARCH_INFO );
	pc_SystemInfo->SetDeviceInfoToMio( TYPE_MioSetDeviceSelect_USB_PRINT_ENABLE );
	pc_SystemInfo->SetDeviceInfoToMio( TYPE_MioSetDeviceSelect_SYSCTL_INFO );	// 2016/08/16【制御構成変更No.16】_NICFUMのリサーチ機能 日立 柳澤 add.
	// 2018/10/15 EagleH [RQ-272] HIENG Start.
	// 2018/11/14 EagleH [RQ-272-2] HIENG Start.
//	pc_SystemInfo->SetDeviceInfoToMio( TYPE_MioSetDeviceSelect_IPFAXLIB_PATH );	
	// 2018/11/14 EagleH [RQ-272-2] HIENG End.
	// 2018/10/15 EagleH [RQ-272] HIENG End.
// 2019/03/04 EagleH [RQ-537-4] HIENG Start.
	pc_SystemInfo->SetDeviceInfoToMio( TYPE_MioSetDeviceSelect_CER_PATH );	
// 2019/03/04 EagleH [RQ-537-4] HIENG End.
}


//関数-------------------------------------------------------------------------------------------------------------------
//概要　　：Exception発生時のハンドラー関数
//作成者　：
//レビュー：未実施（レビュー実施日と名前を記述）
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
#ifdef TORNADO
void exceptionHandler( int tid, int vec, ESFPPC *psf )
{
	SYSF_logMsg("exceptionHandler call tid=0x%x\n",tid,2,3,4,5,6);
	if( semID_CommonData != 0 )
	{
// SEM_IDのメンバ変数の参照を削除（SEM_IDはvoid*と定義されている）
	}
}
#endif

//ROM_DIV_3対応
#ifdef ROM_DIV_3
extern "C"{
extern BOOL nicFumLink(void);
}

#endif
extern "C"{
int32_t lWarpTimeLogFlg = 0;
}

#if 1 /* 暫定対応：QuadCore検討 */
#include <stdio.h>
#include <unistd.h>
#include <sched.h>
#include <sys/syscall.h>
#include <sys/stat.h>

extern "C" {
	static int getFile( void );
}


static int getFile( void )
{
	char buf[10];
	int ret = -1;
	size_t num = 0;
	const char boot_fileName[] = "/ata0a/QuadCore/getApfFile.txt";
	const char km_fileName[] = "/km/cgscript/getApfFile.txt";
	struct stat st;
	FILE* fp = NULL;


	if (stat(boot_fileName, &st) == 0) {
		fp= fopen(boot_fileName, "rb");
	}
	else {
		fp= fopen(km_fileName, "rb");
	}


	if ( fp == NULL ) {
		return ret;
	}
	
	num = fread(buf, 1, 10, fp);
	fclose(fp);

	if (num > 0) {
		ret = buf[0] - '0';	/* atoiの変わり*/
	}
	
	if (ret < 0 || ret > 3) {
		ret = -1;
	}
	
	return ret;
}

extern int setAffinity(int cpuMsk, pid_t tid);/* OSWrapperで定義しているから、そっちを使う。 */

#endif /* 暫定対応：QuadCore検討 */

//関数-------------------------------------------------------------------------------------------------------------------
//概要　　：Application P/F起動
//作成者　：
//レビュー：未実施（レビュー実施日と名前を記述）
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
int do_forkexec_apf( char* path, char* arg )
{
// 2017/12/19 【Eagle】【Simlin】 hitachi  Phat-Dang Modify Start.
// #if defined(SIMULATOR) && !defined(FUM_LINUX)
	// return 0;
// #else

    int   ret = 0;
#ifdef DEF_TASK_MANAGER
    pid_t pid = 0;
#else
    int pid = 0;
#endif // DEF_TASK_MANAGER.

	struct stat st = {};
    char* args[2];

    if( path == NULL ){
        SYSF_logMsg( "[%s] path is NULL. Application P/F fork&exec failed.\n", __PRETTY_FUNCTION__ );
        return -1;
    }

    ret = stat( path, &st );
    if( ret == -1 ){
        perror( "stat" );
        SYSF_logMsg( "[%s] path:[%s] Application P/F fork&exec failed.\n", __PRETTY_FUNCTION__, path );
        return -1;
    }else{
        if( !(st.st_mode & S_IXUSR) ){
            SYSF_logMsg( "[%s] path:[%s] permittion NG. Application P/F fork&exec failed.\n", __PRETTY_FUNCTION__, path );
            return -1;
        }
    }

    args[0] = path;
    if( arg ){
        args[1] = arg;
    }else{
        args[1] = NULL;
    }

    // インストール管理ファイルのコピー.
    copy_InstallConfig();

#ifdef DEF_TASK_MANAGER
    pid = SYSF_procFork("apf000");
#else
    pid = fork();
#endif // DEF_TASK_MANAGER.
    if( pid == 0 ){
#if 1 /* 暫定対応：QuadCore検討 *//* apf000のコア割を設定する .*/
	/* MFPの/ata0a/QuadCore/getApfFile.txtでコア設定 */
	int cpu_mode = getFile();

	if( cpu_mode != -1 ){
#ifdef DEF_64BIT_PF		// EagleH_制御構成変更_No.49 swap有効化対応.
		setAffinity(cpu_mode, (pid_t)syscall(SYS_gettid),"apf000");	// apf000をnoswapグループへ登録する.
#else	// DEF_64BIT_PF.
		setAffinity(cpu_mode, (pid_t)syscall(SYS_gettid));
#endif	// DEF_64BIT_PF.
		printf("[QuadCore][%s] path=[%s] core=[%d]\n", __FUNCTION__, path, cpu_mode);
	}
	else{
		printf("[QuadCore][%s] path=[%s] not set\n", __FUNCTION__, path);
	}


#endif /* 暫定対応：QuadCore検討 */
        ret = execv( path, args );
        if( ret == -1 ){
            perror( "execv" );
            SYSF_logMsg( "[%s] exec NG [%s] %d. Application P/F fork&exec failed.\n", __PRETTY_FUNCTION__, path, SYSF_errnoGet() );
        }

    }else if( pid == -1 ){
        perror( "fork" );
        SYSF_logMsg( "[%s] fork NG [%s] %d. Application P/F fork&exec failed.\n", __PRETTY_FUNCTION__, path, SYSF_errnoGet() );
    }
    else {
    	// no operation.
    	DBGF_PrintNoOperationPath(__FILE__, __LINE__);
    }

    return pid;
// #endif
// 2017/12/19 【Eagle】【Simlin】 hitachi  Phat-Dang Modify End.
}

extern "C" int forkexec_apf()
{
    return do_forkexec_apf( "/km/fw/bin/apf000", NULL );
}




int do_forkexec_pos( char* path, char* arg )
{
#ifdef DEF_TASK_MANAGER
	pid_t pid = 0;
	int   ret = 0;
#else
	int pid = 0;
	int ret = 0;
#endif // DEF_TASK_MANAGER.
	char *args[2];
	struct stat st;
	
	args[0] = path;
	if( arg ){
		args[1] = arg;
	}else{
		args[1] = NULL;
	}
	
	ret = stat( path, &st );
	printf("[Parent] ret = %d\n",ret);
	if( ret == -1 ){
		printf("[Parent] stat error = [%d]\n",errno);
	}
#ifdef DEF_TASK_MANAGER
	pid = SYSF_procFork("pos000");
#else
	pid = fork();
#endif // DEF_TASK_MANAGER.
	if( pid == 0 ){
		//子プロセス
		ret = execv( path, args );
		if( ret == -1 ){
			printf("[%s]execv Error\n",__FILE__); // 2015/10/27 静的解析対応 日立 中井 MOD.
		}
	}else if( pid == -1 ){
		//生成に失敗した場合は親プロセスに-1が返る
		printf("[%s] fork Error\n",__FILE__);
	}
	
	printf("[Parent]done\n");
	return 0;	
}

extern "C" int forkexec_pos()
{
	return do_forkexec_pos("/km/fw/bin/pos000", NULL );
}


#ifdef TORNADO

#if !(defined(DEF_ARMADAXP) || defined(DEF_HELIOS_EMU2) || defined(DEF_Helios_PF))
#if defined(DEF_Soc1200)
#ifndef FUM_LINUX
#include "eipCmemApi.h"
#endif
#include "ThrCmemDrvrIf.h"
#endif
#endif /* !(defined(DEF_ARMADAXP) || defined(DEF_HELIOS_EMU2)) */

extern void IRDebugPrint(int32_t) ;
//関数-------------------------------------------------------------------------------------------------------------------
//概要　　：システム初期化用関数
//作成者　：
//レビュー：未実施（レビュー実施日と名前を記述）
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
//        ：2013/03/28		ADS吉田			CQ.378292(Act_63656) MS-EVP-94 起動パフォーマンス計測処理埋め込み対応.
//        ：2015/02/03		Sky古市			【Poseidon】CQ509420 速度可変モードに遷移しない対応.
//											checkSpeedVariableMode()の実行位置をSYSC_SystemInit1からSYSC_SystemInit0に変更.
void SYSC_SystemInit0()
{	
	/* 起動シーケンス：起動時間測定用 */
	printf("%s : START %lu\n",__func__,SYSF_tickGet());
//2019/10/08 Sky 【自動測定】起動イベントの通知 start.
	/* SystemInit0 開始を通知 */
	THRF_setEventRecord(THRD_EventRecord_SystemInit0Start);
//2019/10/08 Sky 【自動測定】起動イベントの通知 end.
	// 2017/02/20 Sky酒井（純）ユーザーイベント記録(APIコール).
	SYSF_ReqTaskMonitorUserEvent(18000, 2, 0, 0);

#ifdef DEF_A4_Cus// Performance測定用に追加 。
	if( gpSimpleLogForSys != 0 )
	{
		gpSimpleLogForSys->set("Performance_StartSysInit0 %010lu\n",SYSF_tickGet());
	}
#endif
	
#if !defined( SIMULATOR ) || ( defined ( SIMULATOR ) && defined( DBG_IRMSIM ) )
#if defined (DEF_IT5EMU3) || ( defined(DEF_Helios_PF) || defined(DEF_Asic777_PMS) || defined(DEF_S800) )
	InitIRC();	//nara 2013.12.18 暫定.
#endif
#endif
	APIC_DebugLogSystem* pc_DbgLogSystem = APIC_DebugLogSystem::newInstance();
	// ここにモジュール初期化関数をリストする。ここでpanel, sys以外のinitClass()はしないこと。



    SYSF_DBInit();
    SYSF_DBVdbmInit();
    FILG_InitModule( 1 );   // FILC_OverWriteEraseIf::initClass(); INIT_REC();
    NVDG_InitModule( 1 );   // NV Backup Task起動
#ifdef ROM_DIV_3
    nicFumLink();           //nicFumLink()はNVDSInit()より前に呼び出さなければならない　ROM_DIV_3対応
#endif
#ifdef DEF_SupportSSD
//2019/12/02 Sky 【自動測定】起動イベントの通知対応(Step2) start.
	/* 優先パーティションマウント 開始を通知 */
	THRF_setEventRecord(THRD_EventRecord_PreferredPartitionMountStart);
//2019/12/02 Sky 【自動測定】起動イベントの通知対応(Step2) end.
	initOSdriverSSD();
//2019/12/02 Sky 【自動測定】起動イベントの通知対応(Step2) start.
	/* 優先パーティションマウント 終了を通知 */
	THRF_setEventRecord(THRD_EventRecord_PreferredPartitionMountEnd);
//2019/12/02 Sky 【自動測定】起動イベントの通知対応(Step2) end.
#endif	/*#ifdef DEF_SupportSSD*/
// 2020/04/14 OP_BTS-21885 FW書き換え後、電源ONで放置させていたところ、C-D3B1が発生 Start.
//	DAPG_InitModule();	//.
	DAPG_InitModule(0);		// DBMSアプリ用Task起動.
// 2020/04/14 OP_BTS-21885 FW書き換え後、電源ONで放置させていたところ、C-D3B1が発生 End.
	if(lWarpTimeLogFlg == 1)
		printf("@@@NV Start\n");
    NVDG_InitModule( 2 );   // NVDSInit(); INIT_REC();
    if(lWarpTimeLogFlg == 1)
	    printf("@@@NV End\n");
	
	DBMG_InitModule();

#if defined(DEF_A3A4COMM)
	SYSC_ConfigInfoManage *p_SYSC_ConfigInfoManage = NULL;
	// コンフィグ情報管理モジュールが有効の場合.
	if( (p_SYSC_ConfigInfoManage = SYSC_ConfigInfoManage::newInstance()) != NULL ) {
		// コンフィグ情報FATALチェックAPIをコールする.
		p_SYSC_ConfigInfoManage->checkConfigInfoFatal();
	}
#endif // defined(DEF_A3A4COMM)

	// ソフトDipSWが取得できないため、NVDSInit()よりも後で実行する必要がある
	INIT_REC_START();

	// SaaSGatewayDB初期化.
	SGWG_InitModule(0); INIT_REC( 0x01 );

// 2016/03/18【CQ.525980】起動時のタイムスタンプ更新問題対応 日立 田澤 DEL Start.
//    APIC_OptionSetting *pc_OptionSetting = APIC_OptionSetting::newInstance();
//    if (NULL != pc_OptionSetting)
//    {
//    	pc_OptionSetting->SetupOption();
//    }
//    INIT_REC( 0x02 );
// 2016/03/18【CQ.525980】起動時のタイムスタンプ更新問題対応 日立 田澤 DEL End.

	t_eTPM_TPMDATA_STATUS e_tpmstatus = TPM_MGR_TPMDATA_COMPLETION;
	TPM_MGR_GetStatus(&e_tpmstatus); INIT_REC( 0x03 );
	if( ( APIC_TPMSetting::newInstance()->getEnableTPM() == True ) || ( e_tpmstatus != TPM_MGR_TPMDATA_COMPLETION ) ){
		TPM_MGR_Start();
	}
	INIT_REC( 0x04 );
	pc_DbgLogSystem->req_PrintDateSerialNo(); INIT_REC( 0x05 );
	APIC_AppLicense::newInstance()->initRCAutoAdjuster(); INIT_REC( 0x06 );
    APIC_ApiLogging::newInstance()->startApiLoggingTask(); INIT_REC( 0x07 );
// 2018/04/16 Eagle 制御構成変更No.110 不具合解析強化対応 日立 岡部 Start.
//    SYSC_SysLogALLToMedia::newInstance()->startUsbLogWriteTask(); INIT_REC( 0x08 );
	DBGC_SysLogALLToMedia::newInstance()->startUsbLogWriteTask(); INIT_REC( 0x08 );
// 2018/04/16 Eagle 制御構成変更No.110 不具合解析強化対応 日立 岡部 End.
	APIC_EnablerSetting::newInstance()->setDBWriteStatus( False );
    APIC_EnablerSetting::newInstance()->initClass(); INIT_REC( 0x09 );
    SYSC_Enabler::initClass(); INIT_REC( 0x0a );
	_ElapsedTimeLog_(">>> EnablerSetting ->setDBWriteStatus(True).\n");
	APIC_EnablerSetting::newInstance()->setDBWriteStatus( True|NVDE_DBWriteDelay );
	_ElapsedTimeLog_("<<< EnablerSetting ->setDBWriteStatus(True).\n");
    DBGG_InitModule( 1 ); INIT_REC( 0x0b );

	// load_moduleが完了してからspawnSioRecieveThreadを実行させる.
	{
		int sem_ret = 0;
		sem_t* p_sem_load_module = sem_open( "/sem_load_module", O_RDWR );
		if( p_sem_load_module == SEM_FAILED ) {
			printf( "[mfp] /p_sem_load_module open() ERROR!! errno = [%d].\n", errno );
		}
		if( NULL != p_sem_load_module ) {
			do {
				sem_ret = sem_wait( p_sem_load_module );
			} while ((sem_ret == -1) && (errno == EINTR));
			sem_close( p_sem_load_module );
		}
	}
	INIT_REC( 0x0c );

    // NVDSInit()よりも後で実行する必要がある
    spawnSioRecieveThread(); INIT_REC( 0x0d );
#if defined(DEF_A4_Cus) && defined(DEF_KRONOSS_Cus)
    THRC_LxNvControl::newInstance()->init();
    THRC_LxCrashMon::newInstance()->init();
#endif // defined(DEF_A4_Cus) && defined(DEF_KRONOSS_Cus).
    FILG_InitModule( 2 ); INIT_REC( 0x0e );   
    SYSC_Timer::initClass(); INIT_REC( 0x0f );
    SYSC_AjustTime::initClass(); INIT_REC( 0x10 );
    SYSC_RDSTimer::initClass(); INIT_REC( 0x11 );
    APIC_SummerTimerObserver::newInstance()->initTimer(); INIT_REC( 0x12 );

// 2016/03/18【CQ.525980】起動時のタイムスタンプ更新問題対応 日立 田澤 ADD Start.
    APIC_OptionSetting *pc_OptionSetting = APIC_OptionSetting::newInstance();
    if (NULL != pc_OptionSetting)
    {
    	pc_OptionSetting->SetupOption();
    }
    INIT_REC( 0x02 );
// 2016/03/18【CQ.525980】起動時のタイムスタンプ更新問題対応 日立 田澤 ADD End.

    ATSG_InitModule( 1 ); INIT_REC( 0x13 );
    CPGG_InitModule( 1 ); INIT_REC( 0x14 );
    IMGG_InitModule( 1 ); INIT_REC( 0x15 );
    DOCG_InitModule( 1 ); INIT_REC( 0x16 );
	
    DOCG_InitModule( 2 ); INIT_REC( 0x17 );
    JOBG_InitModule( 1 ); INIT_REC( 0x18 );
	APIC_JobDJKeyInfo::newInstance()->setDBWriteStatus( False );
    JLGG_InitModule( 1 ); INIT_REC( 0x19 );
    PLGG_InitModule( 1 );
	_ElapsedTimeLog_(">>> JobDJKeyInfo ->setDBWriteStatus(True).\n");
	APIC_JobDJKeyInfo::newInstance()->setDBWriteStatus( True|NVDE_DBWriteDelay );
	_ElapsedTimeLog_("<<< JobDJKeyInfo ->setDBWriteStatus(True).\n");
    {
    	struct timeval tv = {};
        gettimeofday(&tv, NULL);
        double d_time = (static_cast<double>(tv.tv_sec) + static_cast<double>(tv.tv_usec) * 0.001 * 0.001);
        printf("###[BootPerform] start service initialize (%.06f)\n", d_time );
        SetLapTime(0, 0);
    }
    INIT_REC( 0x1a );

    JSVG_InitModule( 2 ); INIT_REC( 0x1b );
	// ストレージサービス初期化.
	STRC_StorageService::newInstance()->service_own_initialize(0, NULL); INIT_REC( 0x1c );	// 起動シーケンス変更
#if !defined(COSIM)
    NVDG_InitModule( 3 ); INIT_REC( 0x1d );
#endif //#if !defined(COSIM)
    UACG_InitModule( 1 ); INIT_REC( 0x1e );
    ACIG_InitModule( 1 ); INIT_REC( 0xb9 );
    ASMG_MFP_InitModule( 1 ); INIT_REC( 0xba );
#if !defined(COSIM)
    NVDG_InitModule( 4 ); INIT_REC( 0x1f );
#endif //#if !defined(COSIM)
#if !defined(COSIM)
	DOCC_EncFileBox::initClass(); INIT_REC( 0x20 );
#endif //#if !defined(COSIM)

	DOCC_EncPrintManager::initClass(); INIT_REC( 0x21 );

	NVDG_InitModule( 5 ); INIT_REC( 0x22 );

	checkSpeedVariableMode(); INIT_REC( 0x69 );

    PWRG_InitModule( 1 ); INIT_REC( 0x23 );
	APIC_PowerServiceIF::service_initialize(); INIT_REC( 0x24 );
	OPLG_InitModule( 1 ); INIT_REC( 0x25 );
	
    SYSC_Backup::initClass(); INIT_REC( 0x26 );
#ifdef TORNADO
#ifndef LINUX
	excHookAdd( (FUNCPTR)exceptionHandler );
#endif	/* LINUX */
	SYSF_logMsg("add exceptionHandler\n",1,2,3,4,5,6);
#endif
#if !defined(COSIM)
    NVDG_InitModule( 6 ); INIT_REC( 0x27 );
#endif //#if !defined(COSIM)
    SYSC_SystemInfo::initClass(); INIT_REC( 0x28 );
	SYSC_SystemStatus::initClass(); INIT_REC( 0x29 );

// 2017/10/12 AR1030522 装置情報の取得が出来ない 日立 村上(幸) Start.
// 2017/12/18 リンクエラー対応 HIENG Start.
// #if defined( DEF_EagleCus ) || (defined(DEF_IT5EMU3) && defined(DEF_Helios_PF))
// 2018/02/22 IT6Com参照判定の変更 日立 中井 MOD Start.
//#if defined( DEF_EagleCus ) || (defined(DEF_IT5EMU3) && defined(DEF_Helios_PF)) || defined(DEF_SparrowCus)

// 2019/05/06 EagleZ+/EagleBKZ+  【Control No.1】 HIENG START.
// #if !defined( COMMON_REF_IT5COM ) || defined( DEF_EagleBKCus ) // 2019/03/22 【EagleBK】 【Control_No.1】 ROM version HIENG.
// 2019/06/17 SparrowBK [Control No.1] HIENG Start.
// #if !defined( COMMON_REF_IT5COM ) || defined( DEF_EagleBKCus ) || defined( DEF_EagleZpCus ) // EagleZ Plus.
// 2021/03/19 MinervaSSBK  【Control No.1】 HIENG START.
//#if !defined( COMMON_REF_IT5COM ) || defined( DEF_EagleBKCus ) || defined( DEF_EagleZpCus ) || defined( DEF_SparrowBKCus ) // SparrowBK.
#if !defined( COMMON_REF_IT5COM ) || defined( DEF_EagleBKCus ) || defined( DEF_EagleZpCus ) || defined( DEF_SparrowBKCus ) || defined( DEF_MinervaSSBKCus ) // MinervaSSBK.
// 2021/03/19 MinervaSSBK  【Control No.1】 HIENG END.
// 2019/06/17 SparrowBK [Control No.1] HIENG End.
// 2019/05/06 EagleZ+/EagleBKZ+  【Control No.1】 HIENG END.

// 2018/02/22 IT6Com参照判定の変更 日立 中井 MOD End.
// 2017/12/18 リンクエラー対応 HIENG End.
// 2017/10/02 IT6 Eagle 制御構成変更_No.1 日立米川 Start.
	SYSC_SystemInfoIT6Com::newInstance()->init(); INIT_REC( 0x2a );
// 2017/10/02 IT6 Eagle 制御構成変更_No.1 日立米川 End.
#else
	SYSC_SystemInfoZeus5BkCom::newInstance()->init(); INIT_REC( 0x2a );
#endif
// 2017/10/12 AR1030522 装置情報の取得が出来ない 日立 村上(幸) End.
	SYSC_AndroidPanel::initClass(); INIT_REC( 0x2b );

#if !defined(COSIM)
#ifdef TORNADO	// for 実機
	int ss_checksum      = 0;
#if defined(DEF_Donau)
	SYSC_SystemInfoColor::newInstance()->setOutLineFontRomCheckSum(static_cast<ushort>(ss_checksum)); INIT_REC( 0x2c );
#endif	// #if defined(DEF_Donau).
#endif	// TORNADO	// for 実機.
#endif	// !defined(COSIM).
#if !defined(COSIM)
    NVDG_InitModule( 7 ); INIT_REC( 0x2d );
	// 認証サービス初期化.
	AUTC_AuthService::newInstance()->service_own_initialize(0, NULL); INIT_REC( 0x2e );	// 起動シーケンス変更
	// Deviceサービス初期化.
	APIC_DeviceServiceIF::service_initialize(); INIT_REC( 0x2f );	// ads_yoshida 起動シーケンス変更
	APIC_VendorAppManage::initClass(); INIT_REC( 0x30 );
    RDSG_InitModule( 1 ); INIT_REC( 0x31 );
    LCA_InitModule( 1 ); INIT_REC( 0x32 );
    SYSC_KeyDevice::initClass(); INIT_REC( 0x33 );
	// 課金サービス初期化.
	CHRC_ChargingServiceBase::newInstance()->service_own_initialize(0, NULL); INIT_REC( 0x34 );	// 起動シーケンス変更
#endif //#if !defined(COSIM)
    PRTG_InitModule( 1 ); INIT_REC( 0x35 );
    SCNG_InitModule( 1 ); INIT_REC( 0x36 );
	APIC_SystemStatusServiceIF::newInstance()->service_own_initialize(0, NULL); INIT_REC( 0x37 );	// 起動シーケンス変更

// 2018/02/14【Task6】MFPソリューション分離FUM 日立 柳澤 Start.
	APIC_SystemStatus::newInstance()->init_EngReboot();	//ENG/IR単体リブートフラグ初期化処理.
// 2018/02/14【Task6】MFPソリューション分離FUM 日立 柳澤 End.
	
	APIC_MachineSetting *pc_machineSetting = APIC_MachineSetting::newInstance();
	if( pc_machineSetting && ( pc_machineSetting->getTroubleCancellationForBRankFATAL() == TYPE_TroubleCancellationTypeCancel ||
							   pc_machineSetting->getRebootSettingForCRankFATAL()       == TYPE_RebootSettingTypeReboot ) ) {
		/* C/Bランクトラブルリブートがどちらか設定されている場合にOops/kernel panicはリブートする */
		system( "/sbin/sysctl -w kernel.panic=1;/sbin/sysctl -w kernel.panic_on_oops=1" );
		printf( "%s : kernel panic / Oops の発生でリブートを行うよう設定しました\n", __FUNCTION__ );
	}
	
#ifdef LINUX
    // タスクの実行を開始する
    fire_bootSem2();
	/* 起動シーケンス：起動時間測定用 */
	printf("%s : FIRE_BOOTSEM2 START %lu\n",__func__,SYSF_tickGet());
//2019/10/08 Sky 【自動測定】起動イベントの通知 start.
	/* タスク動作解放 開始を通知 */
	THRF_setEventRecord(THRD_EventRecord_TaskReleaseStart);
//2019/10/08 Sky 【自動測定】起動イベントの通知 end.
    INIT_REC( 0x38 );
#endif

#if !defined(DEF_Helios_PF)
	INIT_REC_END();
#endif	// #if !defined(DEF_Helios_PF).

	/* 起動シーケンス：起動時間測定用 */
	printf("%s : END %lu\n",__func__,SYSF_tickGet());
//2019/10/08 Sky 【自動測定】起動イベントの通知 start.
	/* SystemInit0 終了を通知 */
	THRF_setEventRecord(THRD_EventRecord_SystemInit0End);
//2019/10/08 Sky 【自動測定】起動イベントの通知 end.
	// 2017/02/20 Sky酒井（純）ユーザーイベント記録(APIコール).
	SYSF_ReqTaskMonitorUserEvent(18000, 3, 0, 0);
	
}

//関数-------------------------------------------------------------------------------------------------------------------
//概要　　：システム初期化用関数
//作成者　：
//レビュー：未実施（レビュー実施日と名前を記述）
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
//	08/09/25 Sky-horiuchi CQ309332 パネル関数を別タスク起動に変更対応.
//	09/02/09 h.wachi	  CQ315257 中韓台　ダウンロード時のアウトラインフォントのチェックサム値計算方法の変更.
//	10/08/11 s.ohba		  Minerva1st基板立上げ対応.
//  11/07/21 ichiyama     CQ340984 システム初期化関数を2つに分割
//  11/12/26 koshimo      静的解析指摘修正.
//  12/08/08 N.Suzuki     CCD変更対応. // 2012/10/02 福原 マージ.
//  12/09/28 koshimo      速度可変起動モード状態設定関数コールを追加.
//	12/12/20 酒井(日立JTE)OP-OAP-01,OP-OAP-05,OP-BML-01,OP-CSR-01,OP-PRV-01
//						  応用機能オプション化初期化実行タスク起動処理を追加.
//	13/07/16 fukuhara     【ZeusCD】1ビーム/2ビーム画パラファイル切り替え対応.
//	13/10/18 yamauchi     CQ383164 画パラCheckSum算出方法変更(起動時間短縮の為、Ver文字列からCheckSumを作成).
//	14/01/10 toda         制御変更No.1329 新CIS対応.
//	14/02/07 ADS坪井 CQ.381188(Act_70106)	新SIMコンパイルエラー対応.
//						  DEF_Asic777_PMSが定義されている場合は画パラを無効化.
//	2014/05/21 Sky鈴木理	制御構成変更_No.92 画パラファイルの分離化対応.
//							Zeus5BKPreEmuの追加とSoCZeusの判定見直し.
//	2015/02/03 Sky古市		【Poseidon】CQ509420 速度可変モードに遷移しない対応.
//							checkSpeedVariableMode()の実行位置をSYSC_SystemInit1からSYSC_SystemInit0に変更.
//  2016/02/16 maeda		[ZeusSBK][制御構成変更No27]プリント画像パラメータ設計.
//        ：2016/12/06		Sky鈴木		[IT5_4.1]制御構成変更No.05 画像設計 画パラ分離対応(ZeusSBK2nd Z/X/0).
//  2016/05/11 simogaki		AR1021487 スキャナ切り離し状態においてIR画像パラメータの表示が異常になる不具合対応.
//  2018/10/23 Sky松井		[Eagle_High]制御構成変更No.47 EagleL/Eagle0 画パラ切り替え対応.
//  2019/09/12 SCD2-Sky Kawanishi [SecV5][RQ-827] JobLog SMB送信追加のため初期化処理(InitOrder(2))を追加.
//  2022/11/29:SKY		制御構成変更_No.52 USB接続の無線LAN、FAXが繋がっていたらSystemInit1で装着状態をOnにする。
void SYSC_SystemInit1()
{	
	/* 起動シーケンス：起動時間測定用 */
	printf("%s : START %lu\n",__func__,SYSF_tickGet());
//2019/10/08 Sky 【自動測定】起動イベントの通知 start.
	/* SystemInit1 開始を通知 */
	THRF_setEventRecord(THRD_EventRecord_SystemInit1Start);
//2019/10/08 Sky 【自動測定】起動イベントの通知 end.
	// 2017/02/20 Sky酒井（純）ユーザーイベント記録(APIコール).
	SYSF_ReqTaskMonitorUserEvent(18000, 4, 0, 0);

#if !defined(DEF_Helios_PF)
	INIT_REC_START();
#endif	// #if !defined(DEF_Helios_PF).

#if !defined(COSIM)
    CC_MagCard::initClass(); INIT_REC( 0x39 );

// 2017/12/19 【Eagle】【Simlin】 hitachi  Phat-Dang Modify Start.
// //SimWin起動時のエラー対策
// #if defined(SIMULATOR) && !defined(FUM_LINUX)
    // PLBC_PanelCpuIF::initClass();
    // PLBC_PanelLcdDraw::initClass();
    // PMGC_PanelManager::initClass();
    // PCGC_PanelMain::initClass();
// #else
// #endif
// 2017/12/19 【Eagle】【Simlin】 hitachi  Phat-Dang Modify End.
    SYSC_AuthLoadableDriver::initClass(); INIT_REC( 0x3a );
    
	usbInitFunc(); INIT_REC( 0x3b );		// ThreadのUSBキーボード初期化関数.
#endif //#if !defined(COSIM)
	APIC_ResourceScheduler::newInstance()->initClass(); INIT_REC( 0x3c );// EmpFmtメモリ共通化 oikawa.
#if !defined(COSIM)
    CNVG_InitModule( 1 ); INIT_REC( 0x3d );
    NTXG_InitModule( 1 ); INIT_REC( 0x3e );
    NTFXG_InitModule( 1 ); INIT_REC( 0x3f );
#endif //#if !defined(COSIM)
    CNVG_InitModule( 2 ); INIT_REC( 0x40 );
//    CNVG_InitModule( 4 ); INIT_REC( 0x41 );
    PAPG_InitModule( 1 ); INIT_REC( 0x42 );
    PDLG_InitModule( 1 ); INIT_REC( 0x43 );
    PDIG_InitModule( 1 ); INIT_REC( 0x44 );
#if !defined(COSIM)
    LSTG_InitModule( 1 ); INIT_REC( 0x45 );
	RMSG_InitModule( 1 ); INIT_REC( 0x46 );
    EXCG_InitModule( 1 ); INIT_REC( 0x47 );
    NETG_InitNetPrintRx( 1 ); INIT_REC( 0x48 );
    EDTG_InitModule( 1 ); INIT_REC( 0x49 );
    NETG_InitNetFaxRx( 1 ); INIT_REC( 0x4a );
    WDAG_InitWebDavRx( 1 ); INIT_REC( 0x4b );
    PRXG_InitPrintRx( 1 ); INIT_REC( 0x4c );
    FNTG_InitFontRx( 1 ); INIT_REC( 0x4d );
//    MBLG_InitMobilePhoneRx( 1 ); INIT_REC( 0x4e );
    PDRG_InitModule( 1 ); INIT_REC( 0x4f );
    PDIG_InitModule( 2 ); INIT_REC( 0x50 );
    EXCG_InitModule( 2 ); INIT_REC( 0x51 );
    NETG_InitTCPSocket( 1 ); INIT_REC( 0x52 );
    init_random_generate(); INIT_REC( 0x53 );
    FAXG_InitModule( 1 ); INIT_REC( 0x54 );
    RDSG_InitModule( 2 ); INIT_REC( 0x55 );
#endif //#if !defined(COSIM)
	DCMC_InitModule(); INIT_REC( 0x56 );

	/* SMBユーザ認証(SMBAuthTask)タスクを生成する. */
	SMBC_SMBAuth::initClass(); INIT_REC( 0x57 );
	/* /ata01/SMB/BShare/配下を掃除する(SMBShareCleanTask)タスクを生成する. */
	SMBC_SMBShareClean::initClass(); INIT_REC( 0x58 );

    SYSC_InternetISW::initClass(); INIT_REC( 0x59 );

    SYSC_IISWClient::initClass(); INIT_REC( 0x5a );

    SYSC_FWPreserve::initClass(); INIT_REC( 0x5b );

    ADPC_InitAppDirectPrintRx( 1 ); INIT_REC( 0x5c );
	SYSC_TPM::newInstance()->initClass(); INIT_REC( 0x5d );

	SYSC_BGM::newInstance()->initClass(); INIT_REC( 0x5e );
// 2016/03/23 OPS1-Sky Kawanishi [SecV2][SecV2-9] セキュリティー一括設定.
	SYSC_SecurityAutoSetting::initClass(); INIT_REC( 0x5f );
	SYSC_UsbImportExportOperation::initClass(); INIT_REC( 0x60 );
// 2022/03/01 RQ-931 Sky Start.
	SYSC_RmtControlOperation::initClass();
// 2022/03/01 RQ-931 Sky End.
#if !(defined(DEF_ARMADAXP) || defined(DEF_HELIOS_EMU2) || defined(DEF_Helios_PF))
#if defined(DEF_Soc1200) && !defined(FUM_LINUX)
	extern PART_ID memID_HWEncryption;
	EIP_SetMemPartID_HWEncryption(memID_HWEncryption); INIT_REC( 0x61 );
	EIP_SetFunc_SYSF_memPartAlignedAlloc((void*)SYSF_memPartAlignedAlloc); INIT_REC( 0x62 );
	EIP_SetFunc_SYSF_memPartAlloc((void*)SYSF_memPartAlloc); INIT_REC( 0x63 );
	EIP_SetFunc_SYSF_memPartFree((void*)SYSF_memPartFree); INIT_REC( 0x64 );
	EIP_SetThrlFunc_GetKAddr((void*)THRL_Drvrif_Cmemapi_GetKAddr); INIT_REC( 0x65 );
#endif
#endif /* !(defined(DEF_ARMADAXP) || defined(DEF_HELIOS_EMU2)) */

#ifdef TORNADO          // 実機で有効 
#if !defined(COSIM)
#ifndef ROM_DIV // 【崎山】ROM分割時無効 
#ifdef INCLUDE_MIO
    taskAPI_MIB();
    taskAPI_IPP();
#endif
#endif

#if defined(DEF_ZeusMLKBoard)
#if !defined(SIMULATOR)

	// 2022/11/29:USB接続の無線LAN、FAXが繋がっていたらSystemInit1で装着状態をOnにする.
	//      APIC_SystemSetting::getSetupWizardStep()が1以上、かつDipSW No.176 bit3がONのとき.
	if ( True == APIC_SoftDipSw::newInstance()->getSoftDipSwBitForPanelSpecNo(176, 3)
		&& 1 <= APIC_SystemSetting::newInstance()->getSetupWizardStep() ){
		APIC_SystemInfo* pc_SystemInfo = APIC_SystemInfo::newInstance();
		
		// ※無線LANは仕様確認中につき削除.
		// 2022/11/29:拡張ネットワークアダプタ=TYPE_ExNetworkAdapter_Wireless(無線アダプタ).
		// 2022/12/14:無線LANが取り付けられている場合のみ装着状態をOnにする.
		Bool b_WlanBoard = THRF_isExistWLAN() ;
		SYSF_logMsg(" THRF_isExistWLAN() = %d\n",b_WlanBoard) ;
		if ( b_WlanBoard == True ) {
			pc_SystemInfo->setExNetworkAdapter(TYPE_ExNetworkAdapter_Wireless) ;
		}
		
		// 2022/11/29:FAXがPCIバスに接続されているか否か判断.
		// 2022/12/14:FAXの非接続は制御しない.
		//           :主回線が無い状態では副回線は確認しない.
		Bool b_Fax1Exist = THRF_fixFAXDevice( 1 );	// ファックスポート1 接続確認.
		SYSF_logMsg(" THRF_fixFAXDevice( 1 ) = %d\n",b_Fax1Exist) ;
		Bool b_Fax2Exist = THRF_fixFAXDevice( 2 ); 	// ファックスポート2 接続確認.
		SYSF_logMsg(" THRF_fixFAXDevice( 2 ) = %d\n",b_Fax2Exist) ;
		Bool b_Fax3Exist = THRF_fixFAXDevice( 3 );	// ファックスポート3 接続確認.
		SYSF_logMsg(" THRF_fixFAXDevice( 3 ) = %d\n",b_Fax3Exist) ;
		Bool b_Fax4Exist = THRF_fixFAXDevice( 4 );	// ファックスポート4 接続確認.
		SYSF_logMsg(" THRF_fixFAXDevice( 4 ) = %d\n",b_Fax4Exist) ;
		// 基板がいくつ接続されているか.
		unsigned char usConnectDev = 0;
		if( True == b_Fax1Exist ){
			usConnectDev++;
		}
		if( True == b_Fax2Exist ){
			usConnectDev++;
		}
		if( True == b_Fax3Exist ){
			usConnectDev++;
		}
		if( True == b_Fax4Exist ){
			usConnectDev++;
		}
		SYSF_logMsg(" usConnectDev = %d\n",usConnectDev) ;
		// 設定ループカウンタ.
		unsigned char ucSetFaxExistCnt	= 0;
		// 設定回線配列.
		TYPE_CommLine conectLine[4]	= {
									TYPE_CommLine1,
									TYPE_CommLine2,
									TYPE_CommLine3,
									TYPE_CommLine4,
		};

		if( 0 != usConnectDev)
		{
			for( ucSetFaxExistCnt = 0;  ucSetFaxExistCnt < usConnectDev; ucSetFaxExistCnt++ )
			{
				SYSF_logMsg(" setFaxExistAll( TYPE_CommLine%d ) \n",ucSetFaxExistCnt + 1 ) ;
				pc_SystemInfo->setFaxExistAll( conectLine[ucSetFaxExistCnt], True );
			}
		}
	}

	// b_WlanFaxAutoSetUp == Trueのとき、setExNetworkAdapter(TYPE_ExNetworkAdapter_Wireless)を実行すると、.
	// getExNetworkAdapter() == TYPE_ExNetworkAdapter_Wirelessとなると仮定している.
	if ( APIC_SystemInfo::newInstance()->getExNetworkAdapter() == TYPE_ExNetworkAdapter_Wireless ) {
		// 2016/04/15 Harada CQ.527019、CQ.527061 THRF_isExistWLAN()でWLANデバイスが初期化(未確定)の場合は確定させるように変更のため、ここでのデバイス確定は不要.
//    	THRF_fixWLANDevice(); // 2016/02/26 河村 - [ZeusSBK]RQ-340 新WLANデバイス対応 デバイス確定.
#if defined( DEF_A4_Cus ) && defined( DEF_KRONOSS_Cus )
		extern Bool THRG_isOrionMode();
		if( THRF_isExistWLAN() == False &&
			False == THRG_isOrionMode()		// Orion起動時はD262パスする.
		) {
#else // defined( DEF_A4_Cus ) && defined( DEF_KRONOSS_Cus ).
    	if( THRF_isExistWLAN() == False ) {
#endif // defined( DEF_A4_Cus ) && defined( DEF_KRONOSS_Cus ).
			CNTC_CounterIf::newInstance()->countTrouble( TYPD_TroubleCode_FATAL_WLAN_NONE );
			APIC_SystemStatus::newInstance()->setFatal( TYPD_TroubleCode_FATAL_WLAN_NONE );
// 2015/11/13 ZeusS_ZX0 新詳細コード追加対応v1 日立米川 Start.
			APIC_setNewDetailCode( 0,TYPD_NewDetailCode_Cmn_D262_0001 );
// 2015/11/13 ZeusS_ZX0 新詳細コード追加対応v1 日立米川 End.
    	}
	}
#endif // SIMULATOR.
	INIT_REC( 0x66 );
#endif	// #if defined(DEF_ZeusMLKBoard).

	mioSetModelName(); INIT_REC( 0x67 );
	sysc_SetDeviceInfoToMio(); INIT_REC( 0x68 );
	sysc_SetSecurityLevelUpOnToMio(); INIT_REC( 0x69 );

	checkLineModeInTar(); INIT_REC( 0x6a );

	checkBootSelfCheckMode(); INIT_REC( 0x6b );

#ifndef INCLUDE_MIO
#ifdef ROM_DIV_3
	if( pt_MIO_taskSpawn != NULL ) {
		SYSF_logMsg( "NicFum Active1... \n",1,2,3,4,5,6);
		((void(*)())pt_MIO_taskSpawn)();	// MIOタスク起動 
	} else {
		SYM_TYPE	pt_EMP_taskSpawnType;
		if( ( symFindByName( sysSymTbl, "MIO_taskSpawn", &pt_MIO_taskSpawn, &pt_EMP_taskSpawnType ) ) == ERROR ) {
			SYSF_logMsg( "symbol not find(MIO_taskSpawn)...  \n",1,2,3,4,5,6);
		} else {
			SYSF_logMsg( "NicFum Active2... \n",1,2,3,4,5,6);
			((void(*)())pt_MIO_taskSpawn)();	// MIOタスク起動 
		}
	}
#else
    MIO_taskSpawn(); INIT_REC( 0x6c );
#endif
#else
	mioDrv(); INIT_REC( 0x6d );
	mioInstallDevices(); INIT_REC( 0x6e );

//  2017/11/09	SCD2-Sky Shindo [WP.53-54] 設定値見直し対応.
//// 2016/05/09 OPS1-Sky Shibahara [CQ529002] ファームウェア更新検証設定が無効の場合も、検証に失敗してしまう問題の修正.
//	if( APIC_SystemSetting::newInstance()->getIISWFWUpdateError() )
//	{
//		APIC_InternetISW::newInstance()->setInternetIswResult( TYPE_InternetIswResultFailure , JLGE_InspectLogOperateIDL_Admin, JLGE_InspectLogIfOperationPanel, False );
//		APIC_InternetISW::newInstance()->setInternetIswErrorCode( 0x000000A9 );
//		APIC_RdsSetting::newInstance()->setRdsISWMode_ResultErrCode( 0xA9 );
//		APIC_SystemSetting::newInstance()->setIISWFWUpdateError( False );
//	}

	APIC_LdapSetting::newInstance()->setDBWriteStatus( False );
	APIC_SystemStatus::newInstance()->setDBWriteStatus( False );
	APIC_SystemSetting::newInstance()->setDBWriteStatus( False );
	APIC_InternetISW::newInstance()->setDBWriteStatus( False );
	APIC_FWUpdateSetting::newInstance()->setDBWriteStatus( False );
	APIC_RdsSetting::newInstance()->setDBWriteStatus( False );
	mioInit(); INIT_REC( 0x6f );
	_ElapsedTimeLog_(">>> LdapSetting - SystemStatus ->setDBWriteStatus(True).\n");
	APIC_LdapSetting::newInstance()->setDBWriteStatus( True|NVDE_DBWriteDelay );
	APIC_RdsSetting::newInstance()->setDBWriteStatus( True|NVDE_DBWriteDelay );
	APIC_FWUpdateSetting::newInstance()->setDBWriteStatus( True|NVDE_DBWriteDelay );
	APIC_InternetISW::newInstance()->setDBWriteStatus( True|NVDE_DBWriteDelay );
	APIC_SystemSetting::newInstance()->setDBWriteStatus( True|NVDE_DBWriteDelay );
	APIC_SystemStatus::newInstance()->setDBWriteStatus( True|NVDE_DBWriteDelay );
	_ElapsedTimeLog_("<<< LdapSetting - SystemStatus ->setDBWriteStatus(True).\n");

#if defined(DEF_ZeusMLKBoard)
#if !defined(SIMULATOR)
	if(( APIC_SystemInfo::newInstance()->getExNetworkAdapter() == TYPE_ExNetworkAdapter_Wireless ) &&
	   ( THRF_isExistWLAN() == True )){
		if( mioGetWlanInstallStatus() == ERROR ){
			CNTC_CounterIf::newInstance()->countTrouble( TYPD_TroubleCode_D110 );
			APIC_SystemStatus::newInstance()->setFatal( TYPD_TroubleCode_D110 );
// 2015/11/13 ZeusS_ZX0 新詳細コード追加対応v1 日立米川 Start.
			APIC_setNewDetailCode( 0,TYPD_NewDetailCode_Cmn_D110_0001 );
// 2015/11/13 ZeusS_ZX0 新詳細コード追加対応v1 日立米川 End.
		}
	}
#endif // SIMULATOR.
	INIT_REC( 0x70 );
#endif	// #if defined(DEF_ZeusMLKBoard).

#endif
	
	THRC_PanelSound::newInstance()->init(); INIT_REC( 0x71 );
	
    APIC_OptionSetting *pc_OptionSetting = APIC_OptionSetting::newInstance();
    if (NULL != pc_OptionSetting)
    {
    	pc_OptionSetting->SetupNicOption();
    }
    INIT_REC( 0x72 );
	
	// 応用機能オプション化初期化実行タスク起動
	initAppOptFunc(); INIT_REC( 0x73 );
    appInit(); INIT_REC( 0x74 );  // 動的リンク 
    
#endif //!defined(COSIM)
#endif  // #ifdef TORNADO

// 2021/03/28 APF OP_BTS-31670 Start.
#ifdef XWINDOW_ENABLE
// 2018/09/07 MFPソリューション分離対応Step2 (Modify-Start).
	if ( LIBE_RebootStatus_Reboot != LIBG_GetRebootStatus() ) {	// 初期起動時のみ実行する
		printf("/usr/local/bin/xauthadd.sh call.\n");
		system("/usr/local/bin/xauthadd.sh &");
	}
// 2018/09/07 MFPソリューション分離対応Step2 (Modify-Start).
#endif
// 2021/03/28 APF OP_BTS-31670 end.
    	
#if defined(COSIM)
	APIC_SystemInfo* pc_SystemInfo = APIC_SystemInfo::newInstance();
#endif
	
#if !defined(COSIM)

    NVDG_InitModule( 8 ); INIT_REC( 0x75 );
    
    // ROMバージョンの設定 
    APIC_SystemInfo* pc_SystemInfo = APIC_SystemInfo::newInstance(); INIT_REC( 0x76 );
// 2018/10/11【1061143】詳細コードC1AA 日立 中井 ADD Start.
	if( NULL == pc_SystemInfo ){
		printf("%s : pc_SystemInfo==NULL \n",__func__);
	}
// 2018/10/11【1061143】詳細コードC1AA 日立 中井 ADD End.

#if defined(LINUX)
	{
		char    sc_SysRomVersion[40];
		char*    psc_Ret = NULL;
		unsigned char pcCardVersion[DEF_CARD_VERSION_LENGTH_MAX+1];

		psc_Ret = GetSysRomVersion(sc_SysRomVersion); INIT_REC( 0x77 );

		if(NULL == psc_Ret) {
			// WebROM有無に対応したROMバージョンを設定.
			char sc_SysRomversionDef[40] = {0};
			APIC_SystemInfo::newInstance()->getSysRomversionDef(sc_SysRomversionDef); INIT_REC( 0x78 );
			strncpy(sc_SysRomVersion, sc_SysRomversionDef, sizeof(sc_SysRomVersion));
			sc_SysRomVersion[39]='\0';
		}

		pc_SystemInfo->setRomVersion( TYPE_RomMFPController, sc_SysRomVersion ); INIT_REC( 0x79 );
		char    sc_getPreRomVer[40] = {};
		SYSC_SystemInfo3::newInstance()->getPreRomVersion( sc_getPreRomVer );

		// 設定済みPreRomVersionと一致しない場合にのみSPI-Flashへ書き込む.
		if ( strcmp( sc_SysRomVersion, sc_getPreRomVer ) != 0 ) {
			// 未設定バージョンのためSPI-Flashへ書き込む.
			SYSC_SystemInfo3::newInstance()->setPreRomVersion( sc_SysRomVersion );
		}
		else {
			// 設定済みバージョンのためSPI-Flashへ書き込まない.
		}
		INIT_REC( 0x7a );
		
		// MinervaZeus カードバージョンの設定.	
		if( THRF_getCardVersion( pcCardVersion ) == True ){
    		pc_SystemInfo->setRomVersion( TYPE_RomCardVersion, reinterpret_cast<const char*>(pcCardVersion) ); INIT_REC( 0x7b );
		}
		
	}
#else
	// WebROM有無に対応したROMバージョンを設定.
	char sc_SysRomversionDef[40] = {0};
	APIC_SystemInfo::newInstance()->getSysRomversionDef(sc_SysRomversionDef);
	pc_SystemInfo->setRomVersion( TYPE_RomMFPController, sc_SysRomversionDef );
	char    sc_getPreRomVer[40] = {};
	SYSC_SystemInfo3::newInstance()->getPreRomVersion( sc_getPreRomVer );

	// 設定済みPreRomVersionと一致しない場合にのみSPI-Flashへ書き込む.
	if ( strcmp( sc_SysRomversionDef, sc_getPreRomVer ) != 0 ) {
		// 未設定バージョンのためSPI-Flashへ書き込む.
		SYSC_SystemInfo3::newInstance()->setPreRomVersion( sc_SysRomversionDef );
	}
	else {
		// 設定済みバージョンのためSPI-Flashへ書き込まない.
	}

#endif   /* #if defined(LINUX) */
#ifndef LINUX
#ifdef TORNADO	/* 実機で有効 */
	// Gaia先取り カードバージョンの設定.
	unsigned char pcCardVersion[DEF_CARD_VERSION_LENGTH_MAX+1];
	
	if( THRF_getCardVersion( pcCardVersion ) == True ){
    	pc_SystemInfo->setRomVersion( TYPE_RomCardVersion, (const char*)pcCardVersion );
	}
#endif /* #ifdef TORNADO */
#endif /* #ifndef LINUX */
	ushort us_totalChecksum = 0;
#ifdef TORNADO	// for 実機
	getChkSumFromIndex( DEF_IMGTYPE_ALL , &us_totalChecksum ); INIT_REC( 0x7c );
#else			// for SimWin
	us_totalChecksum = 0xabcd;
#endif
	pc_SystemInfo->setRomCheckSum( TYPE_RomMFPController, us_totalChecksum ); INIT_REC( 0x7d );

    //概要　　：ＭＦＰコントローラ内のサブＲＯＭバージョン 
	// WebROM有無に対応したROMバージョンを設定.
	char sc_MFPSystemControllerRomversionDef[40] = {0};
	APIC_SystemInfo::newInstance()->getMFPSystemControllerRomversionDef(sc_MFPSystemControllerRomversionDef); INIT_REC( 0x7e );
	pc_SystemInfo->setMFPControllerRomVersion( TYPE_MFPControllerRomSystem, sc_MFPSystemControllerRomversionDef ); INIT_REC( 0x7f );
	// オペレーションパネル.
    pc_SystemInfo->setMFPControllerRomVersion( TYPE_MFPControllerRomPanel, TYPD_MFPControllerRomPanel ); INIT_REC( 0x80 );

    //概要　　：ＭＦＰコントローラ内のサブＲＯＭバージョン 
#ifdef ROM_DIV_3
	if( pt_mioKMVersionString != NULL ) {
		pc_SystemInfo->setMFPControllerRomVersion( TYPE_MFPControllerRomMio, static_cast<char *>(pt_mioKMVersionString) ); INIT_REC( 0x81 );
	} else {
#ifdef TORNADO
		SYSF_logMsg("■□SYSC_SystemInit() pt_mioKMVersionString is NULL pointer.□■\n",1,2,3,4,5,6);
#endif
	}
#else
    pc_SystemInfo->setMFPControllerRomVersion( TYPE_MFPControllerRomMio, TYPD_MioRomversion ); INIT_REC( 0x82 );
#endif
#endif
#if !defined(DEF_MinervaPreEmu) && !defined(DEF_ZeusRevCus)
	{
		char			linux_ver[33];
		char			kernel_ver[32];
		unsigned int	linux_checksum = 0;
		unsigned int	kernel_checksum = 0;

		memset(linux_ver, 0, 33);
		memset(kernel_ver, 0, 32);

		THRL_DrvrIf_GetKernelVer(kernel_ver, 32); INIT_REC( 0x83 );					// kernelバージョン取得.
		pc_SystemInfo->setRomVersion(TYPE_RomKernel, kernel_ver); INIT_REC( 0x84 );	// ROM設定.

		if(THRF_getLinuxVer(linux_ver) != False)								// Linuxバージョン取得.
		{
			pc_SystemInfo->setRomVersion(TYPE_RomLinux, linux_ver); INIT_REC( 0x85 );	// ROM設定.
		}
		if(THRF_getKernelChecksum(&linux_checksum, &kernel_checksum) != False)	// チェックサム取得.
		{
			pc_SystemInfo->setRomCheckSum(TYPE_RomLinux, static_cast<ushort>(linux_checksum)); INIT_REC( 0x86 );	// ROM設定.
			pc_SystemInfo->setRomCheckSum(TYPE_RomKernel, static_cast<ushort>(kernel_checksum)); INIT_REC( 0x87 );	// ROM設定.
		}
		printf("@@ Linux  Version = %31s Checksum = 0x%x\n", linux_ver, static_cast<ushort>(linux_checksum));
		printf("@@ Kernel Version = %s Checksum = 0x%x\n", kernel_ver, static_cast<ushort>(kernel_checksum));
	}
#endif
#if defined(DEF_CCD_CHG_ENABLE)
#if !defined(FUM_LINUX)
	{
		const char *soname = NULL;
		char* pc_ver = NULL;
		// PIC画パラのチェックサムとバージョンの設定.
		soname = FN_PIC_IPROM_SO;
		pc_SystemInfo->setRomVersion( TYPE_ImageParamPIC, (const char *)IPROM_CPSPIC_INT_Ver );
		printf("@@ GAPARA PIC %s Version = [%s]\n",soname,IPROM_CPSPIC_INT_Ver); 
		// THR画パラのチェックサムとバージョンの設定.
		pc_SystemInfo->setRomVersion( TYPE_ImageParamThread, IP_Ver );
		printf("@@ GAPARA THR Version = [%s]\n",IP_Ver); 
		// IR画パラのチェックサムとバージョンの設定.
		soname = FN_IRC_IPROM_SO;
// 2017/02/09 【1013105】IT5_4.0 warning除去対応 日立 平方 Start.
//		pc_SystemInfo->setRomVersion( TYPE_IRImageParameter, p_irc_ShLib_ver );
//		pc_SystemInfo->setRomVersion( TYPE_IRImageParameter, (char *)p_irc_ShLib_ver );
		char sc_ir_ShLib_ver_tmp[ ROM_VERSION_NEW_SIZE + 1 ]={0};	// バージョン設定用文字列バッファ(バージョン情報文字数+終端コード(1)).
		size_t size_ir_verlen = 0;									// IR内部のバージョン情報文字列サイズ.

		size_ir_verlen = strlen( (char *)p_irc_ShLib_ver );
		// IRの内部保持バージョンをバージョン設定用データにコピー.
		if ( size_ir_verlen < ( sizeof( sc_ir_ShLib_ver_tmp ) ) ) { 	//終端コードのサイズは含まないので未満で判定する.
			strcpy( (char *)comSystem.cc_ROMVersionIPIRC.uc_Data_New, (char *)p_irc_ShLib_ver );
		} else {														// IR内部のバージョン情報文字列が規定サイズ以上の場合のフェール.
			strncpy( (char *)comSystem.cc_ROMVersionIPIRC.uc_Data_New, (char *)p_irc_ShLib_ver, ( sizeof( sc_ir_ShLib_ver_tmp ) - 1 ) );	//	終端は含まないので-1する.
		}
		// ハイフン付きのバージョンに変換する.
		// getROMVersion_New2()はuc_Data_New[]に設定されている文字列に"-"を付加したデータを引数バッファに設定する.
		comSystem.cc_ROMVersionIPIRC.getROMVersion_New2( (uchar *)sc_ir_ShLib_ver_tmp );
		// ハイフン付きのバージョンを設定する.
		pc_SystemInfo->setRomVersion( TYPE_IRImageParameter, sc_ir_ShLib_ver_tmp );
// 2017/02/09 【1013105】IT5_4.0 warning除去対応 日立 平方 Start.
		printf("@@ GAPARA IR %s Version = org:[%s] set:[%s]\n",soname,p_irc_ShLib_ver, sc_ir_ShLib_ver_tmp ); 
	}
#endif //!defined(FUM_LINUX)
#else //defined(DEF_CCD_CHG_ENABLE).
// 2016/12/06 Sky鈴木	[IT5_4.1]制御構成変更No.05 画像設計 画パラ分離対応(ZeusSBK2nd Z/X/0).
// 2018/10/19 Sky松井	[Eagle_High]制御構成変更No.47 EagleL/Eagle0 ROM一本化対応.
// SIMでなく、Emuでなく.
// 画パラ切り替え機種（ZeusS ZX0、Eagle Low(1/2/2.5)、Eagle High(0) ）を対象とする.
#if	( !defined(SIMULATOR) && !defined(DEF_S800Emu) && !defined(DEF_IT5EMU3) ) && \
	( ( defined( DEF_ZEUSS_ZX0_CUS ) && !defined(DEF_S800) ) || \
	  ( defined( DEF_EagleCus ) && !defined(DEF_A800) ) ) && \
    ( !defined( DEF_HeliosMLKCus ) )
	{	// 画パラのバージョンとチェックサムの設定.
		const char *soname = NULL;
		int32_t sum = 0L;
		char* pc_ver = NULL;

		// PIC画パラのチェックサムとバージョンの設定.
		soname = FN_PIC_IPROM_SO;
		for(sum = 0, pc_ver = reinterpret_cast<char*>(IPROM_CPSPIC_INT_Ver); *pc_ver != '\0'; pc_ver++)
		{
			// バージョン文字列からチェックサムを算出.
			sum += *pc_ver;
		}
		pc_SystemInfo->setRomCheckSum(TYPE_ImageParamPIC, static_cast<ushort>(sum) );					   INIT_REC( 0x88 );
		pc_SystemInfo->setRomVersion( TYPE_ImageParamPIC, const_cast<const char *>(reinterpret_cast<const char *>(IPROM_CPSPIC_INT_Ver)) );INIT_REC( 0x89 );
		printf("@@ GAPARA PIC %s Version = [%s] sum = 0x%08X\n",soname,IPROM_CPSPIC_INT_Ver,sum); 

		// THR画パラのチェックサムとバージョンの設定.
		for(sum = 0, pc_ver = static_cast<char*>(IP_Ver); *pc_ver != '\0'; pc_ver++)
		{
			// バージョン文字列からチェックサムを算出.
			sum += *pc_ver;
		}
		pc_SystemInfo->setRomCheckSum(TYPE_ImageParamThread, static_cast<ushort>(sum) );				INIT_REC( 0x8a );
		pc_SystemInfo->setRomVersion( TYPE_ImageParamThread, IP_Ver );					INIT_REC( 0x8b );
		printf("@@ GAPARA THR Version = [%s] sum = 0x%08X\n",IP_Ver,sum);
	}
#endif	// SoCZeusBKEmu || SoCZeus || Zeus5BKPreEmu.
#endif // defined(DEF_CCD_CHG_ENABLE).

	char sc_RomVerInt[40] = {};
	// 統合FWバージョンの生成.
	pc_SystemInfo->setRomIntFWversion( sc_RomVerInt );

#if !defined(COSIM)

	if( comSystem.bl_FactoryAdjustMode == True ) { // カードラインモード 
		comIRIndi.setIRI_Cmode( 0x003D ); INIT_REC( 0x8c );				// C-modeを設定する 
		comSystem.cc_Cmode.setCmode( 0x003D ); INIT_REC( 0x8d );			// C-modeを設定する.
#ifdef TORNADO
		SYSF_logMsg("★★【ｶｰﾄﾞﾗｲﾝ】起動時ﾊﾞｯｸｱｯﾌﾟｾｯﾄ %04x 起動時ﾌﾗｸﾞｾｯﾄ %04x\n",comIRIndi.getIRI_Cmode(),comSystem.cc_Cmode.ss_Cmode,3,4,5,6);
#endif
	} else {
		if( comIRIndi.getIRI_Cmode() == 0x003D )
		{
			comSystem.cc_Cmode.setCmode( 0x003D ); INIT_REC( 0x8e );		// C-modeを設定する.
#ifdef TORNADO
		SYSF_logMsg("★★【通常ﾓｰﾄﾞ(Mode On)】起動時ﾊﾞｯｸｱｯﾌﾟｾｯﾄ %04x 起動時ﾌﾗｸﾞｾｯﾄ %04x\n",comIRIndi.getIRI_Cmode(),comSystem.cc_Cmode.ss_Cmode,3,4,5,6);
#endif
		} else {
			comSystem.cc_Cmode.setCmode( 0x0000 ); INIT_REC( 0x8f );		// C-modeを解除する.
			comIRIndi.setIRI_Cmode( 0x0000 ); INIT_REC( 0x90 );			// C-modeを解除する 
		}
	}

#endif


#if defined(TORNADO)
#if !defined(COSIM)
#ifndef FUM_LINUX
    FtpServerInit(); INIT_REC( 0x91 );
#endif // #ifndef FUM_LINUX
    WebDAVdDeviceAppInit(); INIT_REC( 0x92 );      // i-Option WebDAV Device App.
	WebDAVdOapDeviceAppInit(); INIT_REC( 0x93 );		// i-option特ROM WebDAV Device App(OAP連携) 
#endif
#endif

#ifdef SIMULATOR
    FtpServerInit();
    AccountInit(); 
	if (SIMC_ScanAttrOutput	*pScanAttrOutput = SIMC_ScanAttrOutput::newInstance()) {
		pScanAttrOutput->initClass();
	}
	// SimLin WBS(SIM345)関連対応
    if (SIMC_ActiveJobInfo *pActiveJobInfo = SIMC_ActiveJobInfo::newInstance()) {
		pActiveJobInfo->initClass();
	}
#endif

    PDLG_InitModule( 2 ); INIT_REC( 0x94 );

	// Maintenanceサービス初期化.
	APIC_MaintenanceServiceIF::newInstance()->service_own_initialize(0, NULL); INIT_REC( 0x95 );
	// APIC_DataCacheモジュール初期化.
	APIC_DataCacheModuleIF::newInstance()->service_own_initialize(0, NULL); INIT_REC( 0x96 );
	// FaxDialサービス初期化.
	FXDC_FaxDialService::newInstance()->service_own_initialize(0, NULL); INIT_REC( 0x97 );
	// AppDirectPrintサービス初期化.
	ADPC_AppDirectPrintService::newInstance()->service_own_initialize( 0, NULL ); INIT_REC( 0x98 );

	// SystemStatusデータ更新.
	SSTC_SystemStatusService::newInstance()->reqUpdateSystemStatusData(); INIT_REC( 0x99 );

#if defined(COSIM)
#ifdef LINUX
    // タスクの実行を開始する
    fire_bootSem();
#endif
	
#else	// COSIM でないとき 

    NVDG_InitModule( 9 ); INIT_REC( 0x9a );

	// 管理者認証失敗ロック解除スタート 
	// ペナルティカウントがMAXの場合、ロック解除タイマー起動を行う 
	APIC_UserAuthentication::newInstance()->startAutoResetseServiceTimer(TYPD_IndexUserAdmin); INIT_REC( 0x9b );

	// ログ管理者認証失敗ロック解除スタート 
	// ペナルティカウントがMAXの場合、ロック解除タイマー起動を行う 
	APIC_UserAuthentication::newInstance()->startAutoResetseServiceTimer(TYPD_IndexUserLMU); INIT_REC( 0x9c );

	TYPE_GDPElectrostaticTPAdjust e_GDPParam = 0;
	e_GDPParam = APIC_MachineSetting::newInstance()->getGDPElectrostaticTPAdjust(); INIT_REC( 0x9d );

#if defined(TORNADO) && !defined(FUM_LINUX)
#if !defined(DEF_MinervaPreEmu) && !defined(DEF_ZeusRevCus)
	sysBspIoctlPanelSio( &e_GDPParam, EXDB_PanelSetSensitivity ); INIT_REC( 0x9e );
#endif
#endif /* #if defined(TORNADO) && !defined(FUM_LINUX) */

#ifdef LINUX
    // タスクの実行を開始する
    fire_bootSem();
	/* 起動シーケンス：起動時間測定用 */
	printf("%s : FIRE_BOOTSEM START %lu\n",__func__,SYSF_tickGet());
	// 2017/02/20 Sky酒井（純）ユーザーイベント記録(APIコール).
	SYSF_ReqTaskMonitorUserEvent(18000, 8, 0, 0);
	
    INIT_REC( 0x9f );

    SetLapTime(0, 30);
#endif

	// 遠隔操作Androidパネル対応.
	THRG_InitAndroidTask(); INIT_REC( 0xa0 );
	
	SYSC_IwsAuthScreen::newInstance()->initClass(); INIT_REC( 0xa1 );
	
//【CQ 516434】【ZeusS-ZXO】【RQ-383bizhubMarketPlaceのダウンローダーの搭載】【HCL_Sagar】2015/09/02 Add End.
	SYSC_IWSAppInfo::newInstance()->initClass(); INIT_REC( 0xa2 );
//【CQ 516434】【ZeusS-ZXO】【RQ-383bizhubMarketPlaceのダウンローダーの搭載】【HCL_Sagar】2015/09/02 Add End.

// 2018/04/03【RQ-675】性能に合わせてすべてのMFPにMarketPlaceをダウンロード・インストールする 日立 米川 Start.
// 2018/08/20 1056850 【RQ-675】「MarketPlaceSetUP」と表示されるの場合で、IWS=使用するを設定し、MFPが自動起動して、WebUI画面で、二つ「MarketPlace」ボタンを表示されること Start.
// SystemInit1時点ではDB情報(HDD)が参照できないため、SYSC_IWSAppInfo::newIWSInitialize実行位置(NVDG_FuncZeus.cpp)よりも後に移動.
//	/* MarketPlace 条件判定結果を取得 */	//.
//	if ( True == APIC_IWSAppInfo::newInstance()->isMarketPlaceSetupCondition() ){	//.
//		/*  IWS使用設定を取得する */	//.
//		if ( False == APIC_IWSSetting::newInstance()->getEnable() ){	//.
//			/* MarketPlaceセットアップキーを割り当てる*/	//.
//			APIC_StandardPanelSetting::newInstance()->setShortCutSetting_MarketPlaceSetup();	//.
//		}else{	//.
//			/* MarketPlaceをインストールする */	//.
//			APIC_IWSAppInfo::newInstance()->executeIWSAppDownloadStartRequest( NULL,0 );	//.
//		}	//.
//	}	//.
// 2018/08/20 1056850 【RQ-675】「MarketPlaceSetUP」と表示されるの場合で、IWS=使用するを設定し、MFPが自動起動して、WebUI画面で、二つ「MarketPlace」ボタンを表示されること End.
// 2018/04/03【RQ-675】性能に合わせてすべてのMFPにMarketPlaceをダウンロード・インストールする 日立 米川 End.

//2017/05/09 日立井口 新規追加 KronosS 管理No.16 トラブルコード仕様変更(ESD試験対応)start.
#if defined(DEF_A4_Cus) && defined(DEF_KRONOSS_Cus)		// Kronos.
	SYSC_TroubleReboot c_TroubleReboot;
	c_TroubleReboot.count();
	printf("%s :TroubleReboot checked.\n", __func__);
#endif	// Kronos.
//2017/05/09 日立井口 新規追加 KronosS 管理No.16 トラブルコード仕様変更(ESD試験対応)end.

#if !defined(DEF_Helios_PF)
	INIT_REC_END();
#endif	// #if !defined(DEF_Helios_PF).

	/* 起動シーケンス：起動時間測定用 */
	printf("%s : END %lu\n",__func__,SYSF_tickGet());
//2019/10/08 Sky 【自動測定】起動イベントの通知 start.
	/* SystemInit1 終了を通知 */
	THRF_setEventRecord(THRD_EventRecord_SystemInit1End);
//2019/10/08 Sky 【自動測定】起動イベントの通知 end.
	// 2017/02/20 Sky酒井（純）ユーザーイベント記録(APIコール).
	SYSF_ReqTaskMonitorUserEvent(18000, 5, 0, 0);
}

void SYSC_SystemInit2()
{
	/* 起動シーケンス：起動時間測定用 */
	printf("%s : START %lu\n",__func__,SYSF_tickGet());
//2019/10/08 Sky 【自動測定】起動イベントの通知 start.
	/* SystemInit2 開始を通知 */
	THRF_setEventRecord(THRD_EventRecord_SystemInit2Start);
//2019/10/08 Sky 【自動測定】起動イベントの通知 end.
	// 2017/02/20 Sky酒井（純）ユーザーイベント記録(APIコール).
	SYSF_ReqTaskMonitorUserEvent(18000, 6, 0, 0);

	APIC_SystemInfo* pc_SystemInfo = APIC_SystemInfo::newInstance();
#if !defined(DEF_Helios_PF)
	INIT_REC_START();
#endif	// #if !defined(DEF_Helios_PF).
#ifdef TORNADO
	CNVC_PanelHard::newInstance()->init2(); INIT_REC( 0xa1 );
#endif
	APIC_EnablerSetting::newInstance()->setDBWriteStatus( False );
	APIC_MachineSetting::newInstance()->setDBWriteStatus( False );
	APIC_SystemSetting::newInstance()->setDBWriteStatus( False );
	APIC_SystemInfo::newInstance()->setDBWriteStatus( False );
	APIC_RdsSetting::newInstance()->setDBWriteStatus( False );
	APIC_LifeCnt::newInstance()->setDBWriteStatus( False );
	APIC_PartsCnt::newInstance()->setDBWriteStatus( False );
	comWarning.SetWriteDBStatus( False );
	if ( True != SYSC_SystemStatus::newInstance()->isTroubleReset() ) {
		// トラブルリセット実行中でない場合、フラグを設定する.
		comIRIndi.setDBWriteStatus( False );
	}
	comFeed.setDBWriteStatus( False );
	// プリントランプ制御の前に機種構成が確定する必要がある。 
	WaitSystemConfigurationFixed(); INIT_REC( 0xa2 );
	_ElapsedTimeLog_(">>> comFeed - EnablerSetting ->setDBWriteStatus(True).\n");
	comFeed.setDBWriteStatus( True|NVDE_DBWriteDelay );
	comIRIndi.setDBWriteStatus( True|NVDE_DBWriteDelay );
	comWarning.SetWriteDBStatus( True|NVDE_DBWriteDelay );
	APIC_PartsCnt::newInstance()->setDBWriteStatus( True|NVDE_DBWriteDelay );
	APIC_LifeCnt::newInstance()->setDBWriteStatus( True|NVDE_DBWriteDelay );
	APIC_RdsSetting::newInstance()->setDBWriteStatus( True|NVDE_DBWriteDelay );
	APIC_SystemInfo::newInstance()->setDBWriteStatus( True|NVDE_DBWriteDelay );
	APIC_SystemSetting::newInstance()->setDBWriteStatus( True|NVDE_DBWriteDelay );
	APIC_MachineSetting::newInstance()->setDBWriteStatus( True|NVDE_DBWriteDelay );
	APIC_EnablerSetting::newInstance()->setDBWriteStatus( True|NVDE_DBWriteDelay );
	_ElapsedTimeLog_("<<< comFeed - EnablerSetting ->setDBWriteStatus(True).\n");

	// NGINX起動
	SYSC_WebAPI::newInstance()->initClass();

	if( APIC_SystemStatus::newInstance()->getMioExecResetSystemNetwork() == True )
	{
		// WebService Scanner情報設定.
		TYPS_WSDDeviceDescription s_Scanner;
		APIC_WSDSetting::newInstance()->getScannerDescription( &s_Scanner ); INIT_REC( 0xa3 );
		memset( s_Scanner.uc_Infomation, 0x00, sizeof( s_Scanner.uc_Infomation ) );
		APIC_WSDSetting::newInstance()->getDefaultScannerInfo( s_Scanner.uc_Infomation ); INIT_REC( 0xa4 );
// 2016/02/01 CQ521285 監査ログ HCD-PP対応 Sky大嶋 MOD Start.
		APIC_WSDSetting::newInstance()->setScannerDescription( &s_Scanner, JLGE_InspectLogOperateIDL_System, JLGE_InspectLogIfSystem, False ); INIT_REC( 0xa5 );
// 2016/02/01 CQ521285 監査ログ HCD-PP対応 Sky大嶋 MOD End.

		// MIOからのネットワークリセットの実行情報をクリアする.
		APIC_SystemStatus::newInstance()->setMioExecResetSystemNetwork( False ); INIT_REC( 0xa6 );
	}
// 2017/12/19 【Eagle】【Simlin】 hitachi  Phat-Dang Modify Start.
// #if defined(SIMULATOR) && !defined(FUM_LINUX)
	// BRWC_Browser::initClass();			// ブラウザタスクの初期化.
// #endif
// 2017/12/19 【Eagle】【Simlin】 hitachi  Phat-Dang Modify End.


    PAPG_InitModule( 2 ); INIT_REC( 0xa7 );

#ifdef LINUX
	SYSC_ChangeDevice::initClass(); INIT_REC( 0xa8 );
#endif	// #ifdef LINUX.
	APIC_Music::newInstance()->initClass(); INIT_REC( 0xa9 );		// MTUIv1 反応音導入.
	
	pc_SystemInfo->chkInterchangeCFType(); INIT_REC( 0xaa );	// カード誤挿入検出 
	SYSF_taskSpawn( "PNL_setVersionForHDD", 251, VX_FP_TASK, 24576, reinterpret_cast<FUNCPTR>(PNL_setVersionForHDD), 0,0,0,0,0,0,0,0,0,0 );

#endif // #if !defined(COSIM)
	STATUS status = OK;
	BOOL b_scannerEnable = False;

	// Fiery接続状態状態の場合、
    if( pc_SystemInfo->getExternalIFMode() == TYPE_ExternalIFMode1 ||
		pc_SystemInfo->isScanerExist() == False ){
		b_scannerEnable = False;
	}else{
		b_scannerEnable = True;
	}
	INIT_REC( 0xab );
#ifdef TORNADO
#ifdef ROM_DIV_3
	if( pt_mioSetDeviceInfo != NULL ){
		status = (reinterpret_cast<STATUS(*)(t_eDeviceSelect,void *)>(pt_mioSetDeviceInfo))( MIO_SETDEV_SCANNER_ENABLE, reinterpret_cast<void*>(&b_scannerEnable) ); 
	}else{
		SYSF_logMsg("MIO_SETDEV_SCANNER_ENABLE pt_mioSetDeviceInfo NULL pointer.\n",1,2,3,4,5,6 );
	}
	INIT_REC( 0xac );
#else
	status = mioSetDeviceInfo( MIO_SETDEV_SCANNER_ENABLE, (void*)&b_scannerEnable ); INIT_REC( 0xad );
#endif // #ifdef ROM_DIV_3
	SYSF_logMsg("MIO_SETDEV_SCANNER_ENABLE pt_mioSetDeviceInfo b_scannerEnable:[%d] status:[%d] \n", b_scannerEnable, status,0,0,0,0 );
#endif // #ifdef TORNADO

#if !defined( TORNADO )
#ifdef _USE_IGC
	if( True == APIC_SystemInfo::newInstance()->isIFunctionOn() ){
		InitIGC();   // IGC 起動関数
	}
#endif
#endif

	RemotePanelInit(); INIT_REC( 0xae );					//RemotePanel起動.
	// 初期タイマー起動チェック.
	APIC_FatalWatchTimerObserver::newInstance()->initTimer(); INIT_REC( 0xaf );

	SYSC_CustomPatternSetting::initClass(); INIT_REC( 0xb0 );

    APIC_InternetISW::newInstance()->checkFWDownload(); INIT_REC( 0xb1 );

	// 外部接続サービス初期化.
	EXTC_ExternalService::newInstance()->service_own_initialize(0, NULL); INIT_REC( 0xb2 );

	// オブザーバデータ更新要求.
	SSTC_SystemStatusService::newInstance()->reqUpdateObserverData(); INIT_REC( 0xb3 );

	// UV-01347 SaaSGateway対応.
	// SaaSGateway初期化.
	SGWG_InitModule(1); INIT_REC( 0xb4 );

	sleep(3);
#ifdef DEF_TASK_MANAGER
	pid_t pid = SYSF_procFork("lgp000");
#else
	pid_t pid = fork();
#endif // DEF_TASK_MANAGER.
	if( pid < 0 )
	{
		/* fork失敗 */
		printf("Log Process fork NG!!!\n");
	}
	else if( pid == 0 )
	{
		chdir( "/km" );
		/* 子プロセス（LogProcess）*/
// 2015/04/03 Warning除去対応 Bランク指摘 CommonAPI 太田 UPDATE FROM.
//		execlp( "/km/fw/bin/lgp000", NULL );
// 2017/07/04 [AR1025412] LogProcessが起動しない問題 start.
//		execlp( "/km/fw/bin/lgp000", "" );
		execlp( "/km/fw/bin/lgp000", "/km/fw/bin/lgp000", (char*)NULL );
// 2017/07/04 [AR1025412] LogProcessが起動しない問題 end.
// 2015/04/03 Warning除去対応 Bランク指摘 CommonAPI 太田 UPDATE TO.
		/* 失敗	*/
		printf("Log Process Start NG!!!\n");

		_exit(1);
	}
	else {
		// no operation.
		DBGF_PrintNoOperationPath(__FILE__, __LINE__);
// 2018/09/07 MFPソリューション分離対応Step2 (Add-Start).
		child_pid[child_cnt++]=pid;
// 2018/09/07 MFPソリューション分離対応Step2 (Add-End).
	}
	INIT_REC( 0xb5 );

	// 起動回数カウンタクリア.
	setLoopBootCnt( 0 ); INIT_REC( 0xb6 );
	// 起動回数カウンタ(U-Boot)クリア.
	setLoopBootCntBoot( 0 ); INIT_REC( 0xb7 );

	// 詳細コード記録.
	setLoopBootErrDetailCode(); INIT_REC( 0xb8 );

	APIC_XmppmIF::newInstance()->StartUp(TYPE_XmppmIF_StartMode_Normal);
	INIT_REC( 0xb9 );
	APIC_Roim::newInstance()->StartUp(TYPE_Roim_StartMode_Normal);
	INIT_REC( 0xba );
	APIC_BackupRestore::newInstance()->StartUp(APIE_BACKUPRESTORE_StartMode_Normal);
	INIT_REC( 0xbb );
	APIC_Fctm::newInstance()->StartUp(TYPE_Fctm_StartMode_Normal);
	INIT_REC( 0xbc );
// 2015/12/24【RQ-502】機能カウンタ追加 日立 谷 Start.
	SYSC_WideUse::newInstance()->initClass(); INIT_REC( 0xbd );
	TYPC_Time c_TimeTemp;
	APIC_SystemSetting::newInstance()->getCountHddFreeSizeDate(&c_TimeTemp);
	if ( 0 == c_TimeTemp.ss_Year && 0 == c_TimeTemp.sc_Month && 0 == c_TimeTemp.sc_Day ) {
		APIC_SystemSetting::newInstance()->setCountHddFreeSizeDate();
	}
// 2015/12/24【RQ-502】機能カウンタ追加 日立 谷 End.
// 2016/01/20【RQ-505】 バックアップサーバ化対応 日立 喜多 Start.
	WebDAVdBackupRestoreDeviceAppInit(); INIT_REC( 0xbe );		// バックアップリストア専用WebDAVスレッド起動.
// 2016/01/20【RQ-505】 バックアップサーバ化対応 日立 喜多 End.
// 2016/2/3【RQ-503】WW指示サーバ運用の拡張 日立 谷 Start.
	SYSC_Distributer::newInstance()->initClass(); INIT_REC( 0xbf );
// 2016/2/3【RQ-503】WW指示サーバ運用の拡張 日立 谷 End.

// 2016/09/27 IT5_4.0 RQ-421 装置自動更新機能の機能拡張 日立 江上 ADD Start.
	SYSC_RDTManager::newInstance()->initClass(); INIT_REC( 0xc0 );
// 2016/09/27 IT5_4.0 RQ-421 装置自動更新機能の機能拡張 日立 江上 ADD End.

// 2016/12/16【RQ-643】SuperWarp対応 日立 松井 Start.
	SYSC_SuperWarpOperation::newInstance()->initClass(); INIT_REC( 0xc1 );
// 2016/12/16【RQ-643】SuperWarp対応 日立 松井 End.

	SYSC_VirusScan_DirTimer::initClass(); INIT_REC(0xc2);
	SYSC_VirusScan_UpdateTimer::initClass(); INIT_REC(0xc3);
	SYSC_VirusScan::initClass(); INIT_REC(0xc4); // 2018/3/13 SCD2-Sky Kawanishi [RQ-747].
	SYSC_VirusScan_Mem::initClass(); INIT_REC(0xc5);
	SYSC_VirusScan_Log::initClass(); INIT_REC(0xc6);
	SYSC_VirusScan_Version::initClass(); INIT_REC(0xc7);

	// 起動シーケンス中にSYSC_SecurityAutoSettingからrebootしないよう、初期化にSYSC_SecurityAutoSettingの処理を起動..
	SYSC_SecurityAutoSetting::newInstance()->wakeup();

	// JobLog (MIO関連) 初期化.
	JLGG_InitModule(2);
	
	// EditData初期化.
	SYSC_EditData::initClass();

	// MarketPlace初期化.
	SYSC_MarketPlaceData::initClass();
	
// 2018/04/09 DenebMLK [Control_No_67] HIENG Start.
//#if defined(DEF_DenebMLKCus)	// 2018/06/25 DenebMLK [Compile_Switch] HIENG Add.
#if defined(DEF_DenebMLKCus) || defined(DEF_HeliosMLKCus)  // 2019/08/02 HeliosMLK [Compile_Switch] HIENG .	
	SYSC_DeviceReport::initClass();
	INIT_REC( 0xc8 );
//#endif	// DEF_DenebMLKCus.	// 2018/06/25 DenebMLK [Compile_Switch] HIENG Add.
#endif	// DEF_DenebMLKCus/DEF_HeliosMLKCus // 2019/08/02 HeliosMLK [Compile_Switch] HIENG .
// 2018/04/09 DenebMLK [Control_No_67] HIENG End.

// 2018/05/07【MAv6】汎用WebDAVクライアント対応 日立 西原 Start.
	APIC_WebDAVTx::newInstance()->Init(); INIT_REC(0xc9);
// 2018/05/07【MAv6】汎用WebDAVクライアント対応 日立 西原 End.
// 2022/03/01 RQ-931 Sky Start.
	APIC_WebDAVc::newInstance()->Init();
// 2022/03/01 RQ-931 Sky End.
// 2019/01/29 EagleH [Bug_OP_BTS-2277] HIENG Start.
    NVDC_IPFaxT38MessageDispatcher::initClass(); INIT_REC(0xca);
// 2019/01/29 EagleH [Bug_OP_BTS-2277] HIENG End.

#ifdef DEF_64BIT_PF		// EagleH_制御構成変更_No.49 swap有効化対応(swap有効化遅延実行対応).
// Sanitizer有効時はメモリ不足回避の為本体FW起動前にスワップを有効化しているので、swap遅延スタートを行わない.
#ifndef __SANITIZE_ADDRESS__
	SYSC_SwapDelayedStart(); INIT_REC(0xcb);	// swap遅延スタート処理.
#endif	// __SANITIZE_ADDRESS__
#endif	// DEF_64BIT_PF

	SYSC_SystemNotifyUsbInfo::initClass(); INIT_REC(0xcc);

	INIT_REC_END();

#if !defined(DEF_Helios_PF)
#if (defined TORNADO) && (!defined FUM_LINUX)
	/* ※起動完了時に確定したデバッグ情報(INIT_REC_END())をNVRAMからSSDへコピーするため、INIT_REC_END()の後に実行 */
	/* ※通常、処理を追加する場合は、INIT_REC_END()の前に追加すること                                             */
	initDebugDataCopyToSSD();
#endif // #if (defined TORNADO) && (!defined FUM_LINUX)
#endif	// #if !defined(DEF_Helios_PF).

#ifdef SIMULATOR
	SIMC_SoundObserverIf::newInstance()->startSimInit();
#endif

	// D390とC164が同時発生した場合、C164がパネルに表示されてしまい、D390の復旧ができないため、.
	// D390が発生している場合はパネルに表示させるために最後にFATAL登録するよう変更.
	NVDG_InitModule( 10 ); // INIT_REC();

	NVDG_InitModule( 11 ); // INIT_REC();	// 2016/02/16 伊藤史. UACE_JMA. Ldap Multi対応.

	// 2022/11/16 [IT6_2.2MR4] [OP_BTS-40197] HIENG START.
	SYSC_SystemInfoTimeDiff::newInstance()->initClass(); INIT_REC( 0xcc );
	// 2022/11/16 [IT6_2.2MR4] [OP_BTS-40197] HIENG END.
	
// 2018/10/30 【EagleH】 【RQ-771】 HIENG Start.
	// No manual stapling function or finisher installation information is not installed.
	if( ( False ==APIC_PrinterSystem::newInstance()->isManualStapleFunction() ) ||
		( False == APIC_PrinterSystem::newInstance()->isExistFinisher() ) )
	{
		//Manual staple priority: not priority.
		APIC_SystemSetting::newInstance()->setManualStaplePriority(False);
	}
// 2018/10/30 【EagleH】 【RQ-771】 HIENG End.

	printf("MFPシステム起動\n");

// 2018/09/07 MFPソリューション分離対応Step2 (Add-Start).
	pid_t nic_pid;
	char BUF[128];
	snprintf( BUF, 128, "ps --ppid %d --no-heading", mainThread_pid );
	system(BUF);
	snprintf( BUF, 128, "ps --ppid %d --no-heading | grep SPS_RSHA | awk '{ print $1 }'", mainThread_pid );
	if( FILE *stream = popen(BUF, "r") ){
		fscanf(stream, "%d", &nic_pid);
		pclose(stream);
	}
	printf("Step2-Apply: SPS_RSHA pid is %d\n", nic_pid );
	if( nic_pid > 0 ){
		child_pid[child_cnt++]=nic_pid;
	}
// 2018/09/07 MFPソリューション分離対応Step2 (Add-End).

#ifdef DEF_A4_Cus// Performance測定用に追加 。
	if( gpSimpleLogForSys != 0 )
	{
		gpSimpleLogForSys->set("ShowCanCopyBasicGamen %010lu\n",SYSF_tickGet());
	}
#endif

	DBGG_InitModule( 3 ); INIT_REC( 0xc2 );
// 2019/01/23 EagleLMR [AR1071137] HIENG Start.
	SYSC_CheckSSDState();
// 2019/01/23 EagleLMR [AR1071137] HIENG End.

// 2018/11/08 EagleH [RQ-272-2] HIENG Start.
	APIC_SystemInfo::newInstance()->SetDeviceInfoToMio( TYPE_MioSetDeviceSelect_IPFAX_ENABLE );	
	if( TYPE_FuncVer_Expansion1 <= APIC_SystemInfo::newInstance()->getFuncVer2() )
	{
		APIC_SystemInfo::newInstance()->SetDeviceInfoToMio( TYPE_MioSetDeviceSelect_IPFAXLIB_PATH );	
	}
// 2018/11/08 EagleH [RQ-272-2] HIENG End.
// 2019/01/07 EagleH [Control_No.60] HIENG Start.
	SYSC_checkCPUandSSDBoardExchange();
// 2019/01/07 EagleH [Control_No.60] HIENG End.
// 2019/10/31 EagleZPlus[RQ-845] HIENG START.
// 2023/02/15 [IT6_2.3] [OP_BTS_41502] FPT_KhanhNQ32 Start.
// 	SYSC_SystemStatus::newInstance()->checkAdministratorDefaultPassword();
	SYSC_SystemStatus::newInstance()->checkAdministratorDefaultPassword( False );
// 2023/02/15 [IT6_2.3] [OP_BTS_41502] FPT_KhanhNQ32 End.
// 2019/10/31 EagleZPlus[RQ-845] HIENG END.

// 2018/09/07 MFPソリューション分離対応Step2 (Add-Start).
	// MFP監視プロセスへ起動完了通知を送信.
	SYSS_MfpWatchTroubleMsgInfo  stMfpWatchTroubleMsgInfo = {};
	stMfpWatchTroubleMsgInfo.us_TrData = SYSD_MfpWatchInitComplete;
	if ( ERROR == SYSF_msgQSend( SYSC_SystemStatus::newInstance()->getMsgQIdMfpWatch(), ( char* )&stMfpWatchTroubleMsgInfo, sizeof( SYSS_MfpWatchTroubleMsgInfo ), NO_WAIT, MSG_PRI_NORMAL ) ) {
		// イベント送信失敗.
		SYSF_logMsg( "SYSC_SystemInit2::%s():Message queue send error.\n", __FUNCTION__ );
	}
// 2018/09/07 MFPソリューション分離対応Step2 (Add-End).

	/* 起動シーケンス：起動時間測定用 */
	printf("%s : END %lu\n",__func__,SYSF_tickGet());
//2019/10/08 Sky 【自動測定】起動イベントの通知 start.
	/* SystemInit2 終了を通知 */
	THRF_setEventRecord(THRD_EventRecord_SystemInit2End);
//2019/10/08 Sky 【自動測定】起動イベントの通知 end.
	// 2017/02/20 Sky酒井（純）ユーザーイベント記録(APIコール).
	SYSF_ReqTaskMonitorUserEvent(18000, 7, 0, 0);

	// 2023/09/01 APF IT6_2.3_MIF_3rd OP_BTS-44230 BlackAPIログ記録の変更対応.
	Bool bDipSW = APIC_SoftDipSw::newInstance()->getSoftDipSwBitForPanelSpecNo( 104, 4 );
	DIVC_LogMessage::OnSystemInit2End( bDipSW ); // 2022/02/01 APF OP_BTS-37216 BlackAPIログ強化対応.

// 2023/11/07 [IT6_2.3][OP_BTS-45304] FPT-TUANDA33 START.
	// Execute setting only once
	NVDC_CopyJobSetting3* pc_CopyJobSetting3 = NVDC_CopyJobSetting3::newInstance();
	if( NULL == pc_CopyJobSetting3 ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
		return;
	}
	pc_CopyJobSetting3->exeOnlyOnceSetting();
// 2023/11/07 [IT6_2.3][OP_BTS-45304] FPT-TUANDA33 END.
}
#else	//#ifdef TORNADO
void SYSC_SystemInit()
{
	INIT_REC_START();
	// ここにモジュール初期化関数をリストする。ここでpanel, sys以外のinitClass()はしないこと。
    SYSF_DBInit(); INIT_REC();
    SYSF_DBVdbmInit(); INIT_REC();
    FILG_InitModule( 1 ); INIT_REC();   // FILC_OverWriteEraseIf::initClass(); INIT_REC();
    NVDG_InitModule( 1 ); INIT_REC();   // NV Backup Task起動
#ifdef ROM_DIV_3
    nicFumLink();           //nicFumLink()はNVDSInit()より前に呼び出さなければならない　ROM_DIV_3対応
#endif
#ifdef DEF_SupportSSD
	initOSdriverSSD(); INIT_REC();
#endif	/*#ifdef DEF_SupportSSD*/
	if(lWarpTimeLogFlg == 1)
		printf("@@@NV Start\n");
    NVDG_InitModule( 2 ); INIT_REC();   // NVDSInit(); INIT_REC();
    if(lWarpTimeLogFlg == 1)
	    printf("@@@NV End\n");
	APIC_AppLicense::newInstance()->initRCAutoAdjuster(); INIT_REC();
    APIC_ApiLogging::newInstance()->startApiLoggingTask(); INIT_REC();
// 2018/04/16 Eagle 制御構成変更No.110 不具合解析強化対応 日立 岡部 Start.
//    SYSC_SysLogALLToMedia::newInstance()->startUsbLogWriteTask(); INIT_REC();
    DBGC_SysLogALLToMedia::newInstance()->startUsbLogWriteTask(); INIT_REC();
// 2018/04/16 Eagle 制御構成変更No.110 不具合解析強化対応 日立 岡部 End.	
    APIC_EnablerSetting::newInstance()->initClass(); INIT_REC();
    SYSC_Enabler::initClass(); INIT_REC();
    DBGG_InitModule( 1 ); INIT_REC();
    // NVDSInit()よりも後で実行する必要がある
    spawnSioRecieveThread(); INIT_REC();
    FILG_InitModule( 2 ); INIT_REC();   
    SYSC_Timer::initClass(); INIT_REC();
    SYSC_AjustTime::initClass(); INIT_REC();
    SYSC_RDSTimer::initClass(); INIT_REC();
    APIC_SummerTimerObserver::newInstance()->initTimer(); INIT_REC();
    ATSG_InitModule( 1 ); INIT_REC();
    CPGG_InitModule( 1 ); INIT_REC();
    IMGG_InitModule( 1 ); INIT_REC();
    DOCG_InitModule( 1 ); INIT_REC();
    DOCG_InitModule( 2 ); INIT_REC();
    JOBG_InitModule( 1 ); INIT_REC();
    JLGG_InitModule( 1 ); INIT_REC();
    PLGG_InitModule( 1 ); INIT_REC();
#if !defined(COSIM)
    NVDG_InitModule( 3 ); INIT_REC();
#endif //#if !defined(COSIM)
    UACG_InitModule( 1 ); INIT_REC();
#if !defined(COSIM)
    NVDG_InitModule( 4 ); INIT_REC();
#endif //#if !defined(COSIM)
#if !defined(COSIM)
	DOCC_EncFileBox::initClass(); INIT_REC();
#endif //#if !defined(COSIM)

	DOCC_EncPrintManager::initClass(); INIT_REC();

	NVDG_InitModule( 5 ); INIT_REC();

	OPLG_InitModule( 1 ); INIT_REC();
	
    SYSC_Backup::initClass(); INIT_REC();
#ifdef TORNADO
#ifndef LINUX
	excHookAdd( (FUNCPTR)exceptionHandler );
#endif	/* LINUX */
	SYSF_logMsg("add exceptionHandler\n",1,2,3,4,5,6);
#endif
#if !defined(COSIM)
    NVDG_InitModule( 6 ); INIT_REC();
#endif //#if !defined(COSIM)
    SYSC_SystemInfo::initClass(); INIT_REC();
	SYSC_SystemStatus::initClass(); INIT_REC();

    ADPC_InitAppDirectPrintRx( 1 );
	SYSC_TPM::newInstance()->initClass(); INIT_REC();

	SYSC_BGM::newInstance()->initClass(); INIT_REC();
// 2017/10/12 AR1030522 装置情報の取得が出来ない 日立 村上(幸) Start.
// 2018/02/22 IT6Com参照判定の変更 日立 中井 MOD Start.
// 2018/03/26 Eagle Warning除去 Start.
//#if defined( DEF_EagleCus ) || (defined(DEF_IT5EMU3) && defined(DEF_Helios_PF)) ||
// 2018/03/26 Eagle Warning除去 End.
//	defined( DEF_SparrowCus ) // 2017/12/08 Sparrow [No.01] HIENG.

// 2019/05/06 EagleZ+/EagleBKZ+  【Control No.1】 HIENG START.
// #if !defined( COMMON_REF_IT5COM )  || defined( DEF_EagleBKCus ) // 2019/03/22 【EagleBK】 【Control_No.1】 ROM version HIENG.
// 2019/06/17 SparrowBK [Control No.1] HIENG Start.
// #if !defined( COMMON_REF_IT5COM )  || defined( DEF_EagleBKCus ) || defined( DEF_EagleZpCus ) // EagleZ Plus.
// 2021/03/19 MinervaSSBK  【Control No.1】 HIENG START.
//#if !defined( COMMON_REF_IT5COM )  || defined( DEF_EagleBKCus ) || defined( DEF_EagleZpCus ) || defined( DEF_SparrowBKCus ) // SparrowBK.
#if !defined( COMMON_REF_IT5COM )  || defined( DEF_EagleBKCus ) || defined( DEF_EagleZpCus ) || defined( DEF_SparrowBKCus ) || defined( DEF_MinervaSSBKCus ) // MinervaSSBK.
// 2021/03/19 MinervaSSBK  【Control No.1】 HIENG END.
// 2019/06/17 SparrowBK [Control No.1] HIENG End.
// 2019/05/06 EagleZ+/EagleBKZ+  【Control No.1】 HIENG END.

// 2018/02/22 IT6Com参照判定の変更 日立 中井 MOD End.
// 2017/10/02 IT6 Eagle 制御構成変更_No.1 日立米川 Start.
	SYSC_SystemInfoIT6Com::newInstance()->init(); INIT_REC();
// 2017/10/02 IT6 Eagle 制御構成変更_No.1 日立米川 End.
#else
	SYSC_SystemInfoZeus5BkCom::newInstance()->init(); INIT_REC();
#endif
// 2017/10/12 AR1030522 装置情報の取得が出来ない 日立 村上(幸) End.

	SYSC_AndroidPanel::initClass(); INIT_REC();

#if !defined(COSIM)
#ifdef TORNADO	// for 実機
	int ss_checksum      = 0;
#if defined(DEF_Donau)
	SYSC_SystemInfoColor::newInstance()->setOutLineFontRomCheckSum((ushort)ss_checksum); INIT_REC();
#endif	// #if defined(DEF_Donau).
#endif	// TORNADO	// for 実機.
#endif	// !defined(COSIM).
#if !defined(COSIM)
    NVDG_InitModule( 7 ); INIT_REC();
	APIC_VendorAppManage::initClass();
    RDSG_InitModule( 1 ); INIT_REC();
#ifndef SIMULATOR
    LCA_InitModule( 1 ); INIT_REC();
#endif
    SYSC_KeyDevice::initClass(); INIT_REC();
#endif //#if !defined(COSIM)
    PRTG_InitModule( 1 ); INIT_REC();
    SCNG_InitModule( 1 ); INIT_REC();
#if !defined(COSIM)
    CC_MagCard::initClass(); INIT_REC();
    
    SYSC_AuthLoadableDriver::initClass(); INIT_REC();
    
    PLBC_PanelCpuIF::initClass(); INIT_REC();
	usbInitFunc(); INIT_REC();		// ThreadのUSBキーボード初期化関数.
    PLBC_PanelLcdDraw::initClass(); INIT_REC();
    PMGC_PanelManager::initClass(); INIT_REC();
    PCGC_PanelMain::initClass(); INIT_REC();
#endif //#if !defined(COSIM)
    
    PWRG_InitModule( 1 ); INIT_REC();
#if !defined(COSIM)
    CNVG_InitModule( 1 ); INIT_REC();
    NTXG_InitModule( 1 ); INIT_REC();
    NTFXG_InitModule( 1 ); INIT_REC();
#endif //#if !defined(COSIM)
    CNVG_InitModule( 2 ); INIT_REC();
    PAPG_InitModule( 1 ); INIT_REC();
    PDLG_InitModule( 1 ); INIT_REC();
    PDIG_InitModule( 1 ); INIT_REC();
#if !defined(COSIM)
    LSTG_InitModule( 1 ); INIT_REC();
	RMSG_InitModule( 1 ); INIT_REC();
    EXCG_InitModule( 1 ); INIT_REC();
    NETG_InitNetPrintRx( 1 ); INIT_REC();
    EDTG_InitModule( 1 ); INIT_REC();
    NETG_InitNetFaxRx( 1 ); INIT_REC();
    WDAG_InitWebDavRx( 1 ); INIT_REC();
    PRXG_InitPrintRx( 1 ); INIT_REC();
    FNTG_InitFontRx( 1 ); INIT_REC();
//    MBLG_InitMobilePhoneRx( 1 ); INIT_REC();
    PDRG_InitModule( 1 ); INIT_REC();
    PDIG_InitModule( 2 ); INIT_REC();
    EXCG_InitModule( 2 ); INIT_REC();
    NETG_InitTCPSocket( 1 ); INIT_REC();
    init_random_generate(); INIT_REC();
    FAXG_InitModule( 1 ); INIT_REC();
    RDSG_InitModule( 2 ); INIT_REC();
#endif //#if !defined(COSIM)

	/* SMBユーザ認証(SMBAuthTask)タスクを生成する. */
	SMBC_SMBAuth::initClass(); INIT_REC();
	/* /ata01/SMB/BShare/配下を掃除する(SMBShareCleanTask)タスクを生成する. */
	SMBC_SMBShareClean::initClass(); INIT_REC();

    SYSC_InternetISW::initClass(); INIT_REC();

    SYSC_IISWClient::initClass(); INIT_REC();

    SYSC_FWPreserve::initClass(); INIT_REC();
// 2016/03/23 OPS1-Sky Kawanishi [SecV2][SecV2-9] セキュリティー一括設定追加.
	SYSC_SecurityAutoSetting::initClass(); INIT_REC();
	SYSC_UsbImportExportOperation::initClass(); INIT_REC();

    ADPC_InitAppDirectPrintRx( 1 );

#ifdef TORNADO          // 実機で有効 
#if !defined(COSIM)
#ifndef ROM_DIV // ROM分割時無効.
#ifdef INCLUDE_MIO
    taskAPI_MIB();
    taskAPI_IPP();
#endif
#endif

	mioSetModelName(); INIT_REC();
	sysc_SetDeviceInfoToMio(); INIT_REC();
	sysc_SetSecurityLevelUpOnToMio(); INIT_REC();

#ifndef INCLUDE_MIO	// MIO8ではMIO_taskSpawn()する必要がない.
#ifdef ROM_DIV_3
	if( pt_MIO_taskSpawn != NULL ) {
		SYSF_logMsg( "NicFum Active1... \n",1,2,3,4,5,6);
		((void(*)())pt_MIO_taskSpawn)();	// MIOタスク起動 
	} else {
		SYM_TYPE	pt_EMP_taskSpawnType;
		if( ( symFindByName( sysSymTbl, "MIO_taskSpawn", &pt_MIO_taskSpawn, &pt_EMP_taskSpawnType ) ) == ERROR ) {
			SYSF_logMsg( "symbol not find(MIO_taskSpawn)...  \n",1,2,3,4,5,6);
		} else {
			SYSF_logMsg( "NicFum Active2... \n",1,2,3,4,5,6);
			((void(*)())pt_MIO_taskSpawn)();	// MIOタスク起動 
		}
	}
#else
    MIO_taskSpawn(); INIT_REC();
#endif
#else
	mioDrv();
	mioInstallDevices();
	mioInit();
#endif

    appInit(); INIT_REC();  // 動的リンク 
    
#endif //!defined(COSIM)
#endif  // #ifdef TORNADO

#if defined(COSIM)
	APIC_SystemInfo* pc_SystemInfo = APIC_SystemInfo::newInstance();
#endif
	
#if !defined(COSIM)

    NVDG_InitModule( 8 ); INIT_REC();
    
    // ROMバージョンの設定 
    APIC_SystemInfo* pc_SystemInfo = APIC_SystemInfo::newInstance(); INIT_REC();
#if defined(LINUX)
	{
		char    sc_SysRomVersion[40];
		char*    psc_Ret = NULL;
		unsigned char pcCardVersion[DEF_CARD_VERSION_LENGTH_MAX+1];

		psc_Ret = GetSysRomVersion(sc_SysRomVersion); INIT_REC();

		if(NULL == psc_Ret) {
			// WebROM有無に対応したROMバージョンを設定.
			char sc_SysRomversionDef[40] = {0};
			APIC_SystemInfo::newInstance()->getSysRomversionDef(sc_SysRomversionDef);
			strncpy(sc_SysRomVersion, sc_SysRomversionDef, sizeof(sc_SysRomVersion));
			sc_SysRomVersion[39]='\0';
		}

		pc_SystemInfo->setRomVersion( TYPE_RomMFPController, sc_SysRomVersion ); INIT_REC();
		char    sc_getPreRomVer[40] = {};
		SYSC_SystemInfo3::newInstance()->getPreRomVersion( sc_getPreRomVer );

		// 設定済みPreRomVersionと一致しない場合にのみSPI-Flashへ書き込む.
		if ( strcmp( sc_SysRomVersion, sc_getPreRomVer ) != 0 ) {
			// 未設定バージョンのためSPI-Flashへ書き込む.
			SYSC_SystemInfo3::newInstance()->setPreRomVersion( sc_SysRomVersion );
		}
		else {
			// 設定済みバージョンのためSPI-Flashへ書き込まない.
		}
		INIT_REC();
		
		// MinervaZeus カードバージョンの設定.	
		if( THRF_getCardVersion( pcCardVersion ) == True ){
    		pc_SystemInfo->setRomVersion( TYPE_RomCardVersion, (const char*)pcCardVersion ); INIT_REC();
		}
		
	}
#else
	// WebROM有無に対応したROMバージョンを設定.
	char sc_SysRomversionDef[40] = {0};
	APIC_SystemInfo::newInstance()->getSysRomversionDef(sc_SysRomversionDef);
	pc_SystemInfo->setRomVersion( TYPE_RomMFPController, sc_SysRomversionDef );
	char    sc_getPreRomVer[40] = {};
	SYSC_SystemInfo3::newInstance()->getPreRomVersion( sc_getPreRomVer );

	// 設定済みPreRomVersionと一致しない場合にのみSPI-Flashへ書き込む.
	if ( strcmp( sc_SysRomversionDef, sc_getPreRomVer ) != 0 ) {
		// 未設定バージョンのためSPI-Flashへ書き込む.
		SYSC_SystemInfo3::newInstance()->setPreRomVersion( sc_SysRomversionDef );
	}
	else {
		// 設定済みバージョンのためSPI-Flashへ書き込まない.
	}
#endif   /* #if defined(LINUX) */
#ifndef LINUX
#ifdef TORNADO	/* 実機で有効 */
	// Gaia先取り カードバージョンの設定.
	unsigned char pcCardVersion[DEF_CARD_VERSION_LENGTH_MAX+1];
	
	if( THRF_getCardVersion( pcCardVersion ) == True ){
    	pc_SystemInfo->setRomVersion( TYPE_RomCardVersion, (const char*)pcCardVersion );
	}
#endif /* #ifdef TORNADO */
#endif /* #ifndef LINUX */
	ushort us_totalChecksum = 0;
#ifdef TORNADO	// for 実機
	getChkSumFromIndex( DEF_IMGTYPE_ALL , &us_totalChecksum ); INIT_REC();
#else			// for SimWin
	us_totalChecksum = 0xabcd;
#endif
	pc_SystemInfo->setRomCheckSum( TYPE_RomMFPController, us_totalChecksum ); INIT_REC();

    //概要　　：ＭＦＰコントローラ内のサブＲＯＭバージョン 
	// WebROM有無に対応したROMバージョンを設定.
	char sc_MFPSystemControllerRomversionDef[40] = {0};
	APIC_SystemInfo::newInstance()->getMFPSystemControllerRomversionDef(sc_MFPSystemControllerRomversionDef);
	pc_SystemInfo->setMFPControllerRomVersion( TYPE_MFPControllerRomSystem, sc_MFPSystemControllerRomversionDef ); INIT_REC();
	// オペレーションパネル.
    pc_SystemInfo->setMFPControllerRomVersion( TYPE_MFPControllerRomPanel, TYPD_MFPControllerRomPanel ); INIT_REC();

    //概要　　：ＭＦＰコントローラ内のサブＲＯＭバージョン 
#ifdef ROM_DIV_3
	if( pt_mioKMVersionString != NULL ) {
		pc_SystemInfo->setMFPControllerRomVersion( TYPE_MFPControllerRomMio, (char *)pt_mioKMVersionString );
	} else {
#ifdef TORNADO
		SYSF_logMsg("■□SYSC_SystemInit() pt_mioKMVersionString is NULL pointer.□■\n",1,2,3,4,5,6);
#endif
	}
#else
    pc_SystemInfo->setMFPControllerRomVersion( TYPE_MFPControllerRomMio, TYPD_MioRomversion ); INIT_REC();
#endif
#endif
#if !defined(DEF_MinervaPreEmu) && !defined(DEF_ZeusRevCus)
	{
		char			linux_ver[32];
		char			kernel_ver[32];
		unsigned int	linux_checksum = 0;
		unsigned int	kernel_checksum = 0;

		memset(linux_ver, 0, 32);
		memset(kernel_ver, 0, 32);

		THRL_DrvrIf_GetKernelVer(kernel_ver, 32); INIT_REC();					// kernelバージョン取得.
		pc_SystemInfo->setRomVersion(TYPE_RomKernel, kernel_ver); INIT_REC();	// ROM設定.

		if(THRF_getLinuxVer(linux_ver) != False)								// Linuxバージョン取得.
		{
			pc_SystemInfo->setRomVersion(TYPE_RomLinux, linux_ver); INIT_REC();	// ROM設定.
		}
		if(THRF_getKernelChecksum(&linux_checksum, &kernel_checksum) != False)	// チェックサム取得.
		{
			pc_SystemInfo->setRomCheckSum(TYPE_RomLinux, (ushort)linux_checksum); INIT_REC();	// ROM設定.
			pc_SystemInfo->setRomCheckSum(TYPE_RomKernel, (ushort)kernel_checksum); INIT_REC();	// ROM設定.
		}
		printf("@@ Linux  Version = %s Checksum = 0x%x\n", linux_ver, (ushort)linux_checksum);
		printf("@@ Kernel Version = %s Checksum = 0x%x\n", kernel_ver, (ushort)kernel_checksum);
	}
#endif
	char sc_RomVerInt[40] = {};
	// 統合FWバージョンの生成.
	pc_SystemInfo->setRomIntFWversion( sc_RomVerInt );

#if !defined(COSIM)

	if( comSystem.bl_FactoryAdjustMode == True ) { // カードラインモード 
		comIRIndi.setIRI_Cmode( 0x003D ); INIT_REC();				// C-modeを設定する 
		comSystem.cc_Cmode.setCmode( 0x003D ); INIT_REC();			// C-modeを設定する.
#ifdef TORNADO
		SYSF_logMsg("★★【ｶｰﾄﾞﾗｲﾝ】起動時ﾊﾞｯｸｱｯﾌﾟｾｯﾄ %04x 起動時ﾌﾗｸﾞｾｯﾄ %04x\n",comIRIndi.getIRI_Cmode(),comSystem.cc_Cmode.ss_Cmode,3,4,5,6);
#endif
	} else {
		if( comIRIndi.getIRI_Cmode() == 0x003D )
		{
			comSystem.cc_Cmode.setCmode( 0x003D ); INIT_REC();		// C-modeを設定する.
#ifdef TORNADO
		SYSF_logMsg("★★【通常ﾓｰﾄﾞ(Mode On)】起動時ﾊﾞｯｸｱｯﾌﾟｾｯﾄ %04x 起動時ﾌﾗｸﾞｾｯﾄ %04x\n",comIRIndi.getIRI_Cmode(),comSystem.cc_Cmode.ss_Cmode,3,4,5,6);
#endif
		} else {
			comSystem.cc_Cmode.setCmode( 0x0000 ); INIT_REC();		// C-modeを解除する.
			comIRIndi.setIRI_Cmode( 0x0000 ); INIT_REC();			// C-modeを解除する 
		}
	}

#endif


#if defined(TORNADO)
#if !defined(COSIM)
    FtpServerInit(); INIT_REC();
    WebDAVdDeviceAppInit(); INIT_REC();      // i-Option WebDAV Device App.
	WebDAVdOapDeviceAppInit(); INIT_REC();		// i-option特ROM WebDAV Device App(OAP連携) 
#endif
#endif

#ifdef SIMULATOR
// 2017/12/19 【Eagle】【Simlin】 hitachi  Phat-Dang Modify Start.
// #ifndef FUM_LINUX
    // FtpServerInit();
// #endif // #ifndef FUM_LINUX
// 2017/12/19 【Eagle】【Simlin】 hitachi  Phat-Dang Modify End.
    AccountInit(); 
#endif

    PDLG_InitModule( 2 ); INIT_REC();

#if defined(COSIM)
#ifdef LINUX
    // タスクの実行を開始する
    fire_bootSem();
#endif
	
#else	// COSIM でないとき 

    NVDG_InitModule( 9 ); INIT_REC();

	// 管理者認証失敗ロック解除スタート 
	// ペナルティカウントがMAXの場合、ロック解除タイマー起動を行う 
	APIC_UserAuthentication::newInstance()->startAutoResetseServiceTimer(TYPD_IndexUserAdmin); INIT_REC();

	// ログ管理者認証失敗ロック解除スタート 
	// ペナルティカウントがMAXの場合、ロック解除タイマー起動を行う 
	APIC_UserAuthentication::newInstance()->startAutoResetseServiceTimer(TYPD_IndexUserLMU); INIT_REC();

#ifdef LINUX
    // タスクの実行を開始する
    fire_bootSem(); INIT_REC();
#endif


	SYSC_IwsAuthScreen::newInstance()->initClass(); INIT_REC();

//【CQ 516434】【ZeusS-ZXO】【RQ-383bizhubMarketPlaceのダウンローダーの搭載】【HCL_Sagar】2015/09/02 Add End.
	SYSC_IWSAppInfo::newInstance()->initClass(); INIT_REC();
//【CQ 516434】【ZeusS-ZXO】【RQ-383bizhubMarketPlaceのダウンローダーの搭載】【HCL_Sagar】2015/09/02 Add End.
	WaitSystemConfigurationFixed();
	// プリントランプ制御の前に機種構成が確定する必要がある。 
	WaitSystemConfigurationFixed(); INIT_REC();

	if( APIC_SystemStatus::newInstance()->getMioExecResetSystemNetwork() == True )
	{
		// WebService Scanner情報設定.
		TYPS_WSDDeviceDescription s_Scanner;
		APIC_WSDSetting::newInstance()->getScannerDescription( &s_Scanner ); INIT_REC();
		memset( s_Scanner.uc_Infomation, 0x00, sizeof( s_Scanner.uc_Infomation ) );
		APIC_WSDSetting::newInstance()->getDefaultScannerInfo( s_Scanner.uc_Infomation ); INIT_REC();
		APIC_WSDSetting::newInstance()->setScannerDescription( &s_Scanner ); INIT_REC();

		// MIOからのネットワークリセットの実行情報をクリアする.
		APIC_SystemStatus::newInstance()->setMioExecResetSystemNetwork( False ); INIT_REC();
	}

// 2017/12/19 【Eagle】【Simlin】 hitachi  Phat-Dang Modify Start.
// #if defined(SIMULATOR) && !defined(FUM_LINUX)
	// BRWC_Browser::initClass();			// ブラウザタスクの初期化.
// #endif
// 2017/12/19 【Eagle】【Simlin】 hitachi  Phat-Dang Modify End.

    PAPG_InitModule( 2 ); INIT_REC();
#ifdef LINUX
	SYSC_ChangeDevice::initClass(); INIT_REC();
#endif	// #ifdef LINUX.

	APIC_Music::newInstance()->initClass();	INIT_REC();

	pc_SystemInfo->chkInterchangeCFType(); INIT_REC();	// カード誤挿入検出 
	SYSF_taskSpawn( "PNL_setVersionForHDD", 251, VX_FP_TASK, 24576, (FUNCPTR)PNL_setVersionForHDD, 0,0,0,0,0,0,0,0,0,0 );

#endif // #if !defined(COSIM)
	STATUS status = OK;
	BOOL b_scannerEnable = False;

	// Fiery接続状態状態の場合、
    if( pc_SystemInfo->getExternalIFMode() == TYPE_ExternalIFMode1 ||
		pc_SystemInfo->isScanerExist() == False ){
		b_scannerEnable = False;
	}else{
		b_scannerEnable = True;
	}

#ifdef TORNADO
#ifdef ROM_DIV_3
	if( pt_mioSetDeviceInfo != NULL ){
		status = ((STATUS(*)(t_eDeviceSelect,void *))pt_mioSetDeviceInfo)( MIO_SETDEV_SCANNER_ENABLE, (void*)&b_scannerEnable ); 
	}else{
		SYSF_logMsg("MIO_SETDEV_SCANNER_ENABLE pt_mioSetDeviceInfo NULL pointer.\n",1,2,3,4,5,6 );
	}
#else
	status = mioSetDeviceInfo( MIO_SETDEV_SCANNER_ENABLE, (void*)&b_scannerEnable ); INIT_REC();
#endif // #ifdef ROM_DIV_3
	SYSF_logMsg("MIO_SETDEV_SCANNER_ENABLE pt_mioSetDeviceInfo b_scannerEnable:[%d] status:[%d] \n", b_scannerEnable, status,0,0,0,0 );
#endif // #ifdef TORNADO

#if !defined( TORNADO )
#ifdef _USE_IGC
	if( True == APIC_SystemInfo::newInstance()->isIFunctionOn() ){
		InitIGC();   // IGC 起動関数
	}
#endif
#endif

	RemotePanelInit(); INIT_REC();					//RemotePanel起動.
	// 初期タイマー起動チェック.
	APIC_FatalWatchTimerObserver::newInstance()->initTimer(); INIT_REC();

	SYSC_CustomPatternSetting::initClass(); INIT_REC();

    APIC_InternetISW::newInstance()->checkFWDownload(); INIT_REC();

	// 外部接続サービス初期化.
	EXTC_ExternalService::newInstance()->service_own_initialize(0, NULL); INIT_REC();

	// オブザーバデータ更新要求.
	SSTC_SystemStatusService::newInstance()->reqUpdateObserverData();

	SYSC_VirusScan_DirTimer::initClass(); INIT_REC();
	SYSC_VirusScan_UpdateTimer::initClass(); INIT_REC();
	SYSC_VirusScan::initClass(); INIT_REC(); // 2018/3/13 SCD2-Sky Kawanishi [RQ-747].
	SYSC_VirusScan_Mem::initClass(); INIT_REC();
	SYSC_VirusScan_Log::initClass(); INIT_REC();
	SYSC_VirusScan_Version::initClass(); INIT_REC();

// 2018/04/09 DenebMLK [Control_No_67] HIENG Start.
//#if defined(DEF_DenebMLKCus)	// 2018/06/25 DenebMLK [Compile_Switch] HIENG Add.
#if defined(DEF_DenebMLKCus) || defined(DEF_HeliosMLKCus)  // 2019/08/02 HeliosMLK [Compile_Switch] HIENG .	
	SYSC_DeviceReport::initClass();
	INIT_REC();
//#endif	// DEF_DenebMLKCus.	// 2018/06/25 DenebMLK [Compile_Switch] HIENG Add.
#endif	// DEF_DenebMLKCus/DEF_HeliosMLKCus // 2019/08/02 HeliosMLK [Compile_Switch] HIENG .
// 2018/04/09 DenebMLK [Control_No_67] HIENG End.

	SYSC_SystemNotifyUsbInfo::initClass(); INIT_REC();
	
	// 起動シーケンス中にSYSC_SecurityAutoSettingからrebootしないよう、初期化にSYSC_SecurityAutoSettingの処理を起動..
	SYSC_SecurityAutoSetting::newInstance()->wakeup();

	INIT_REC_END();
	// DipSW49-bit6（NVRAM33-5）がONの場合.
	if( True == ( APIC_SoftDipSw::newInstance()->getSoftDipSwBit(33, 5) ) )
	{
		TYPC_PaperType c_PaperType;
		APIC_SystemSetting* pc_SystemSetting = APIC_SystemSetting::newInstance();
		APIC_FeedTray* pc_FeedTray = APIC_FeedTray::newInstance();

		if( ( NULL == pc_SystemSetting ) || ( NULL == pc_FeedTray ) )
		{
			LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
			return;
		}

		// トレイ1.
		c_PaperType = pc_FeedTray->getPaperType( TYPE_FeedTray1 );
		c_PaperType.setPaperType( pc_SystemSetting->getPaperSizeCnt_PaperType( TYPE_PaperSizeCnt1 ) );
		pc_FeedTray->setPaperType( TYPE_FeedTray1, c_PaperType );
		// トレイ2.
		c_PaperType = pc_FeedTray->getPaperType( TYPE_FeedTray2 );
		c_PaperType.setPaperType( pc_SystemSetting->getPaperSizeCnt_PaperType( TYPE_PaperSizeCnt2 ) );
		pc_FeedTray->setPaperType( TYPE_FeedTray2, c_PaperType );
		// トレイ3.
		c_PaperType = pc_FeedTray->getPaperType( TYPE_FeedTray3 );
		c_PaperType.setPaperType( pc_SystemSetting->getPaperSizeCnt_PaperType( TYPE_PaperSizeCnt3 ) );
		pc_FeedTray->setPaperType( TYPE_FeedTray3, c_PaperType );
		// トレイ4.
		c_PaperType = pc_FeedTray->getPaperType( TYPE_FeedTray4 );
		c_PaperType.setPaperType( pc_SystemSetting->getPaperSizeCnt_PaperType( TYPE_PaperSizeCnt4 ) );
		pc_FeedTray->setPaperType( TYPE_FeedTray4, c_PaperType );
		// 手差し.
		c_PaperType = pc_FeedTray->getPaperType( TYPE_FeedTrayManual );
		c_PaperType.setPaperType( pc_SystemSetting->getPaperSizeCnt_PaperType( TYPE_PaperSizeCnt5 ) );
		pc_FeedTray->setPaperType( TYPE_FeedTrayManual, c_PaperType );
		// LCT.
		// DenebMLKではトレイ5として使用.
		c_PaperType = pc_FeedTray->getPaperType( TYPE_FeedTray5 );
		c_PaperType.setPaperType( pc_SystemSetting->getPaperSizeCnt_PaperType( TYPE_PaperSizeCnt6 ) );
		pc_FeedTray->setPaperType( TYPE_FeedTray5, c_PaperType );
	}
	// D390とC164が同時発生した場合、C164がパネルに表示されてしまい、D390の復旧ができないため、.
	// D390が発生している場合はパネルに表示させるために最後にFATAL登録するよう変更.
	NVDG_InitModule( 10 ); INIT_REC();

// 2023/11/07 [IT6_2.3][OP_BTS-45304] FPT-TUANDA33 START.
	// Execute setting only once
	NVDC_CopyJobSetting3* pc_CopyJobSetting3 = NVDC_CopyJobSetting3::newInstance();
	if( NULL == pc_CopyJobSetting3 ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
		return;
	}
	pc_CopyJobSetting3->exeOnlyOnceSetting();
// 2023/11/07 [IT6_2.3][OP_BTS-45304] FPT-TUANDA33 END.
}
#endif	//#ifdef TORNADO

void WaitSystemConfigurationFixed( void )
{
#ifdef TORNADO
	us_tAppInitProgress = 392;	// 2020/10/27 詳細コードC311対応.
	APIC_PrinterStatus* pc_PrinterStatus = APIC_PrinterStatus::newInstance();
	Bool isPrinterFixed = False;
	// 装置構成確定情報(Printer) を待つ 
	for (;;) {
		us_tAppInitProgress = 393;	// 2020/10/27 詳細コードC311対応.
		pc_PrinterStatus->lock();
		us_tAppInitProgress = 394;	// 2020/10/27 詳細コードC311対応.
		isPrinterFixed = pc_PrinterStatus->isSystemConfigurationFixed();
		us_tAppInitProgress = 395;	// 2020/10/27 詳細コードC311対応.
		pc_PrinterStatus->unlock();
#if defined( DEF_SimAutoTest )		// 自動評価で有効. 
		us_tAppInitProgress = 396;	// 2020/10/27 詳細コードC311対応.
		if( ddInPort_SimAutoTest_isActive() == true )
		{
			us_tAppInitProgress = 397;	// 2020/10/27 詳細コードC311対応.
			break;
		}
		else
		{
			us_tAppInitProgress = 398;	// 2020/10/27 詳細コードC311対応.
			if ( isPrinterFixed ) {
				us_tAppInitProgress = 399;	// 2020/10/27 詳細コードC311対応.
				break;
			}
			us_tAppInitProgress = 400;	// 2020/10/27 詳細コードC311対応.
		}
#else	// #if defined( DEF_SimAutoTest )		// 自動評価で有効. 
		us_tAppInitProgress = 401;	// 2020/10/27 詳細コードC311対応.
		if ( isPrinterFixed ) {
			us_tAppInitProgress = 402;	// 2020/10/27 詳細コードC311対応.
			break;
		}
		us_tAppInitProgress = 403;	// 2020/10/27 詳細コードC311対応.
#endif	// #if defined( DEF_SimAutoTest )		// 自動評価で有効. 
		SYSF_taskDelay( 40 );	/* 200ms */
		us_tAppInitProgress = 404;	// 2020/10/27 詳細コードC311対応.
	}
	us_tAppInitProgress = 405;	// 2020/10/27 詳細コードC311対応.
#endif // #ifdef TORNADO
}

//関数-------------------------------------------------------------------------------------------------------------------
//概要　　：サービス生成(サービス共通の初期化)
//作成者　：
//レビュー：未実施（レビュー実施日と名前を記述）
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
void SYSC_CreateService( void )
{
	// サービス全体の初期化.
	MFPC_ServiceBase::mfpcom_initialize();

	// JobService.
	JSVG_InitModule( 1 );

	// PowerService.
	APIC_PowerServiceIF::newInstance();

	// StorageService.
	STRC_StorageService::newInstance()->mfp_initialize(0, NULL);
// 2015/10/14 CQ.513519対応 Start.
	STRC_StorageService::newInstance()->mfp_initialize_for_dataChanged(0, NULL);
// 2015/10/14 CQ.513519対応 End.

	// MaintenanceService.
	APIC_MaintenanceServiceIF::newInstance()->mfp_initialize(0, NULL);

	// DataCacheModule.
	APIC_DataCacheModuleIF::newInstance()->mfp_initialize(0, NULL);

	// AuthenticationService.
	AUTC_AuthService::newInstance()->mfp_initialize(0, NULL);

	// FaxDialService.
	FXDC_FaxDialService::newInstance()->mfp_initialize(0, NULL);

	// AppDirectPrintService.
	ADPC_AppDirectPrintService::newInstance()->mfp_initialize( 0, NULL );

	// DeviceService.
	APIC_DeviceServiceIF::newInstance();

	// SystemStatusService.
	APIC_SystemStatusServiceIF::newInstance()->mfp_initialize(0, NULL);

	// ExternalService.
	EXTC_ExternalService::newInstance()->mfp_initialize(0, NULL);

	// ChargingService.
	CHRC_ChargingServiceBase::newInstance()->mfp_initialize(0, NULL);
}

//関数-------------------------------------------------------------------------------------------------------------------.
//概要　　：パネルHW情報初期化.
//作成者　：MSE 諸岡.
//レビュー：.
//変更履歴：.
void SYSC_InitPanelHardResolution( void )
{
	// パネルHW情報初期化.
	SYSC_SystemInfo::InitPanelHardResolution();
}

// 2016/11/15【RQ-643 SuperWarp対応】日立 松井 Start.
//関数-------------------------------------------------------------------------------------------------------------------.
// 概要		：SuperWarp起動時の初期化処理.
// 作成者	：日立 松井.
Bool SYSC_SystemInitOfSuperWarp()
{
	Bool b_InitOk = True;

	// 本API内には同期型の制御のみを実装すること.			 //.
	// 各モジュール責務の元、初期化処理でNGとなった場合は	 //.
	// 本APIの戻り値：b_InitOkに「False」をセットし			 //.
	// APIの戻り値が初期化NGとなるよう考慮して実装すること	 //.
// 2021/08/04 MinervaSSBK [DEVALL_OP-965] HIENG Start.
	// 電断時に保存するデータを削除する.
	NVDC_FlashMemory* pFlashMemory = NVDC_FlashMemory::newInstance();
	if( pFlashMemory != NULL ){
		pFlashMemory->eraseInPowerOff();
	}else{
		SYSF_logMsg( "[%s] pFlashMemory is NULL.\n", __PRETTY_FUNCTION__ );
	}
// 2021/08/04 MinervaSSBK [DEVALL_OP-965] HIENG End.
// 2016/12/16【RQ-643】SuperWarp対応 日立 松井 Start.
	// CommonAPI 初期化処理 (同期型).
	if ( False == SYSC_SuperWarpOperation::newInstance()->initializeOfCommonAPI() ) {
		b_InitOk = False;
	}
// 2016/12/16【RQ-643】SuperWarp対応 日立 松井 End.

	// TCPSocketの初期化を実行する(戻り値参照:不要)
	NETG_SPWInitTCPSocke();

	// CSRCの初期化処理を行う.
	// CSRCの初期化処理においては、エラーを返すことが無いため.
	// 戻り値の参照は不要となる.
	RDSG_InitModuleOfSuperWarp();

	// デバイスカウンタ収集タスク(FTP)を初期化.
	if( False == initAccountTask() ) {
		b_InitOk = False;
	}

	// Roimの初期化処理を行う(戻り値の参照は不要).
	APIC_Roim::newInstance()->StartUp(TYPE_Roim_StartMode_Superwarp);

	// KeyDevice(課金装置関連)の初期化を行う(戻り値の参照は不要).
	APIC_KeyDevice::initOfSuperWarp();

// 2022/09/20 IT6_2.2MR4 制御構成変更_No.38_メディア検知機能OFFONの対応 小林 str.
//	PRTG_InitModule(3);			// OP_BTS-44126対応で削除 2023/07/11 kobayashi.
// 2022/09/20 IT6_2.2MR4 制御構成変更_No.38_メディア検知機能OFFONの対応 小林 end.

	// DBGの初期化 SCD2 y.kojima.
	if( False == DBGG_InitForSuperwarp() ) {
		b_InitOk = False;
	}

	// DOCの初期化(PrintKey用EXTRA領域を初期化).
	DOCG_InitModule(3);

	// IGLの初期化(画像ログ転送の起動時リトライ(10分)タイマーを設定).
	IGLG_InitModule(2);

	// UACの初期化.
	UACG_InitModule(3);

// 2020/04/14 OP_BTS-21885 FW書き換え後、電源ONで放置させていたところ、C-D3B1が発生 Start.
	// DBMSアプリ起動.
	DAPG_InitModule(1);
// 2020/04/14 OP_BTS-21885 FW書き換え後、電源ONで放置させていたところ、C-D3B1が発生 End.
// 2022/11/16 [IT6_2.2MR4] [OP_BTS-40197] HIENG START.
	SYSC_SystemInfoTimeDiff::newInstance()->initClass();
// 2022/11/16 [IT6_2.2MR4] [OP_BTS-40197] HIENG END.
	return b_InitOk;
}
// 2016/11/15【RQ-643 SuperWarp対応】日立 松井 End.

// 2019/01/23 EagleLMR [AR1071137] HIENG Start.
//関数-------------------------------------------------------------------------------------------------------------------.
// 概要		： Check SSD state.
// 作成者	：.
void SYSC_CheckSSDState()
{
	TYPE_StorageConfig e_StorageConfig = TYPE_StorageConfig_None;	// SSD hardware configuration.
	Bool b_SSDState = False;										// SSD state.
//	Bool b_LineMode = False;										// Line mode.  ///< 2022/06/22 [IT6_2.2MR3] [Control_No.41] HIENG Remove.
	ushort us_Code = 0;												// Trouble code.

	e_StorageConfig = APIC_SystemInfo::newInstance()->isStorageConfiguration();
	b_SSDState = APIC_SharedSystemDataSetting::newInstance()->getHddExist();
	if( (b_SSDState == False && e_StorageConfig != TYPE_StorageConfig_MicroSD_8GB && e_StorageConfig != TYPE_StorageConfig_MicroSD_16GB) ||
		(b_SSDState == True && e_StorageConfig != TYPE_StorageConfig_SSD_256GB && e_StorageConfig != TYPE_StorageConfig_SSD_1TB && e_StorageConfig != TYPE_StorageConfig_MicroSD_8GB_SSD_256GB) ) { // SSD state does not match with SSD hardware configuration.
//		b_LineMode = APIC_SystemInfo::newInstance()->isCardLineMode();  ///< 2022/06/22 [IT6_2.2MR3] [Control_No.41] HIENG Remove.
		us_Code = APIC_SystemStatus::newInstance()->getFatal().us_Code;
// 2022/06/22 [IT6_2.2MR3] [Control_No.41] HIENG Start.
//		if( (b_LineMode != True) && (us_Code < 0xD300 || us_Code > 0xD3FF) ) {
		if( (us_Code < 0xD300 || us_Code > 0xD3FF) ) {
		// Notify C-D073 on both normal and line modes.
// 2022/06/22 [IT6_2.2MR3] [Control_No.41] HIENG End.
			APIC_SystemStatus::newInstance()->setFatal( TYPD_TroubleCode_D073 );
		}
	}
}
// 2019/01/23 EagleLMR [AR1071137] HIENG End.

// 2019/01/10 EagleH [Control_No.60] HIENG Start.
//関数-------------------------------------------------------------------------------------------------------------------.
// 概要		： Check CPU and Storage serial number change.
// 作成者	：.
void SYSC_checkCPUandSSDBoardExchange()
{
#if !defined(COMMON_REF_IT5COM)
	char sc_CPUSerialNoFromThread[TYPD_MAX_SerialNoCPU+1];			// CPU serial number acquired from thread module.
	char sc_StorageSerialNoFromThread[TYPD_MAX_SerialNoStorage+1];	// Storage serial number acquired from thread module.
	char sc_CPUSerialNoOnSpiFlash[TYPD_MAX_SerialNoCPU+1];			// CPU serial number acquired from SPI-Flash.
	char sc_StorageSerialNoOnSpiFlash[TYPD_MAX_SerialNoStorage+1];	// Storage serial number acquired from SPI-Flash.
	char sc_CPUSerialNoOnSSD[TYPD_MAX_SerialNoCPU+1];				// CPU serial number acquired from SSD.
	char sc_StorageSerialNoOnSSD[TYPD_MAX_SerialNoStorage+1];		// Storage serial number acquired from SSD.
// 2019/04/19 [EagleZX] [FixWarning_2] HIENG START.
//	Bool b_FirstTimeCheck = True;    
// 2019/04/19 [EagleZX] [FixWarning_2] HIENG END.
	Bool b_CPUSerialNo = False;    
	Bool b_StorageSerialNo = False;    
	Bool b_getSPI_Flash  = False;
	Bool b_RecoveryFatal = False;
// 2019/06/07 EagleH [OP_BTS-5581] HIENG Start.
	static const int64_t ToShiBaSSD_ID = 0x1179;
	static const int64_t PhiSonSSD_ID = 0x1987;
// 2020/01/23 日立 馬場 add Start.
	static const int64_t ToShiBaNewSSD_ID = 0x1e0f;
	static const int64_t MicronSSD_ID = 0x1344;
// 2020/01/23 日立 馬場 add End.
// 2019/06/07 EagleH [OP_BTS-5581] HIENG End.
	memset( sc_CPUSerialNoFromThread, '\0', sizeof(sc_CPUSerialNoFromThread) );
	memset( sc_StorageSerialNoFromThread, '\0', sizeof(sc_StorageSerialNoFromThread) );
	memset( sc_CPUSerialNoOnSpiFlash, '\0', sizeof(sc_CPUSerialNoOnSpiFlash) );
	memset( sc_StorageSerialNoOnSpiFlash, '\0', sizeof(sc_StorageSerialNoOnSpiFlash) );
	memset( sc_CPUSerialNoOnSSD, '\0', sizeof(sc_CPUSerialNoOnSSD) );
	memset( sc_StorageSerialNoOnSSD, '\0', sizeof(sc_StorageSerialNoOnSSD) );

	b_CPUSerialNo = THRG_getMFPBoardSerialNoFactory(sc_CPUSerialNoFromThread);
	if( b_CPUSerialNo == False) {
		memset(&sc_CPUSerialNoFromThread, '*', sizeof(sc_CPUSerialNoFromThread));
	}
	
	b_StorageSerialNo = GetStorageSerialNo(sc_StorageSerialNoFromThread);
	if( b_StorageSerialNo == False) {
		memset(&sc_StorageSerialNoFromThread, '*', sizeof(sc_StorageSerialNoFromThread));
	}
	
	// 不揮発異常トラブル 発生状態チェック.
	b_RecoveryFatal = APIC_NVBackup::newInstance()->isRecoveryFatal( SYSC_SystemStatus::newInstance()->getFatal().us_Code );

	// Acquired CPU serial number of SPI-Flash (BackUp area).
	b_getSPI_Flash = APIC_NVBackup::newInstance()->getCPUSerialNo( sc_CPUSerialNoOnSpiFlash );
	
	// In the case where nonvolatile abnormality trouble has not occurred and both thread module/SPI-Flash and CPU serial number are not empty (NULL).
	if ( (False == b_RecoveryFatal) && (0 != strcmp(sc_CPUSerialNoFromThread, "")) && ((True == b_getSPI_Flash) && (0 != strcmp(sc_CPUSerialNoOnSpiFlash, ""))) ) {
		// When the CPU serial number from thread module and SPI-Flash does not match.
		if ( 0 != strncmp( sc_CPUSerialNoFromThread, sc_CPUSerialNoOnSpiFlash, sizeof(sc_CPUSerialNoFromThread) ) ) {
			APIC_setNewDetailCode( 0, createCPUNewDetailCode(sc_CPUSerialNoFromThread) );// 2019/06/07 EagleH [OP_BTS-5581] HIENG.
		}
	}

	// Acquired Storage serial number of SPI-Flash (BackUp area).
	b_getSPI_Flash = APIC_NVBackup::newInstance()->getStorageSerialNo( sc_StorageSerialNoOnSpiFlash );
	
	// In the case where nonvolatile abnormality trouble has not occurred and both thread module/SPI-Flash and Storage serial number are not empty (NULL).
	if ( (False == b_RecoveryFatal) && (0 != strcmp(sc_StorageSerialNoFromThread, "")) && ((True == b_getSPI_Flash) && (0 != strcmp(sc_StorageSerialNoOnSpiFlash, ""))) ) {
		// When the Storage serial number from thread module and SPI-Flash does not match.
		if ( 0 != strncmp( sc_StorageSerialNoFromThread, sc_StorageSerialNoOnSpiFlash, sizeof(sc_StorageSerialNoFromThread) ) ) {
// 2019/06/07 EagleH [OP_BTS-5581] HIENG Start.
			//using SSD
			if( True == APIC_HddInfo::newInstance()->getHddExist()) {
				switch (GetStorageVenderID())
				{
					case ToShiBaSSD_ID:
// 2020/01/23 日立 馬場 add Start.
					case ToShiBaNewSSD_ID: //東芝製SSD(新).
// 2020/01/23 日立 馬場 add End.
						APIC_setNewDetailCode( 0, createToShiBaSSDDetailCode(sc_StorageSerialNoFromThread) );
						break;
					case PhiSonSSD_ID:
						APIC_setNewDetailCode( 0, createPhisonSSDDetailCode(sc_StorageSerialNoFromThread) );
						break;
// 2020/01/23 日立 馬場 add Start.
					case MicronSSD_ID: //Micron製SSD.
						APIC_setNewDetailCode( 0, createMicronSSDDetailCode(sc_StorageSerialNoFromThread) );
						break;
// 2020/01/23 日立 馬場 add End.
					default:
						APIC_setNewDetailCode( 0, createOtherSSDDetailCode(sc_StorageSerialNoFromThread));
						break;
				}
			}
			else {//using microSD
				APIC_setNewDetailCode( 0, createPhisonAdataMicroSDDetailCode(sc_StorageSerialNoFromThread));
			}
// 2019/06/07 EagleH [OP_BTS-5581] HIENG End.
		}
	}

	APIC_SerialNo::newInstance()->getCPUSerialNo( sc_CPUSerialNoOnSSD );
	if ( 0 != strncmp( sc_CPUSerialNoFromThread, sc_CPUSerialNoOnSSD, sizeof(sc_CPUSerialNoFromThread) ) ) {
		APIC_SerialNo::newInstance()->setCPUSerialNo( sc_CPUSerialNoFromThread );
	}
	APIC_SerialNo::newInstance()->getStorageSerialNo( sc_StorageSerialNoOnSSD );
	if ( 0 != strncmp( sc_StorageSerialNoFromThread, sc_StorageSerialNoOnSSD, sizeof(sc_StorageSerialNoFromThread) ) ) {
		APIC_SerialNo::newInstance()->setStorageSerialNo( sc_StorageSerialNoFromThread );
	}
#endif
}
// 2019/01/10 EagleH [Control_No.60] HIENG End.

// 2019/06/07 EagleH [OP_BTS-5581] HIENG Start.
//関数-------------------------------------------------------------------------------------------------------------------.
// 概要		： create a CPU new detail code from CPU serial number got from Thread.
// 作成者	：.
uint64_t createCPUNewDetailCode( char *pc_SerialNo )
{
	// CPU board Serial code(Example).
	// | 基板ASSYコード					| 固定 	| 識別	| シリアルナンバー.
	// |								| 文字	| コード|.
	// ----------------------------------------------------------------------------------.
	// |A	A	2	J	H	0	2	N	| M		| 0		| 0	0	0	0	0	1.
	// 		~	~	~					  ~				  ~	~	~	~	~	~.
	// 上記の~の個所を新詳細コードに変換.
	// (注)シリアルナンバーは数値として使用.
	
	const uint64_t CPU_2nd_Mask		 = 0x00FF000000000000ULL;								///< シリアルコード2byteのマスク.
	const uint64_t CPU_3rd_Mask		 = 0x0000FF0000000000ULL;								///< シリアルコード3byteのマスク.
	const uint64_t CPU_4th_Mask		 = 0x000000FF00000000ULL;								///< シリアルコード4byteのマスク.
	const uint64_t CPU_5th_Mask		 = 0x00000000FF000000ULL;								///< シリアルコード5byteのマスク.
	const uint64_t CPU_6th_Mask		 = 0x0000000000FFFFFFULL;								///< シリアルコード6-8byteのマスク.
	
	const uint32_t CPU_2nd_BitShift	 = 48;													// 12桁 x 4bitシフト.
	const uint32_t CPU_3rd_BitShift	 = 40;													// 10桁 x 4bitシフト.
	const uint32_t CPU_4th_BitShift	 = 32;													//  8桁 x 4bitシフト.
	const uint32_t CPU_5th_BitShift	 = 24;													//  6桁 x 4bitシフト.
	
	uint64_t ui64_NewDetailCode = 0;														///< New detailed code for CPU serial number change.
	uint64_t ui64_SerialNo 		= 0;                                        ///< シリアルナンバー.
	// 文字列をASCIIコードとして新詳細コードに設定.
	ui64_NewDetailCode  = (uint64_t)TYPD_NewDetailCode_CF_CPU_ID;							// 先頭2byteを設定(0XCF00000000000000).
	ui64_NewDetailCode |= (uint64_t)( pc_SerialNo[1] ) << CPU_2nd_BitShift & CPU_2nd_Mask;	// 3～ 4byteを設定(0XCFXX000000000000).
	ui64_NewDetailCode |= (uint64_t)( pc_SerialNo[2] ) << CPU_3rd_BitShift & CPU_3rd_Mask;	// 5～ 6byteを設定(0XCFXXXX0000000000).
	ui64_NewDetailCode |= (uint64_t)( pc_SerialNo[3] ) << CPU_4th_BitShift & CPU_4th_Mask;	// 7～ 8byteを設定(0XCFXXXXXX00000000).
	ui64_NewDetailCode |= (uint64_t)( pc_SerialNo[8] ) << CPU_5th_BitShift & CPU_5th_Mask;	// 9～10byteを設定(0XCFXXXXXXXX000000).
	
	pc_SerialNo = pc_SerialNo + 10;	                                                        //the begining index of シリアルナンバー which will be converted to detail code.
	// 取得したシリアルコードを16進数変換.
	ui64_SerialNo = strtol (pc_SerialNo,NULL,16);
	
	ui64_NewDetailCode |= ui64_SerialNo & CPU_6th_Mask;

	return ui64_NewDetailCode;
}
//関数-------------------------------------------------------------------------------------------------------------------.
// 概要		：create a PhiSon SSD new detail code from Storage serial number got from Thread.
// 作成者	：.
uint64_t createPhisonSSDDetailCode( char *pc_SerialNo )
{
	// Phison SSD Serial code(Example).
	// |PCネットワーク	| カスタマー| 製造年月日	| 工場PC| シリアルナンバー.
	// |MACアドレス		| コード	|				| 番号	|.
	// ----------------------------------------------------------------------------------.
	// |0	3	8	5	| 0	7		| 7	A	1	B	| 5	D	| 0	1	0	0	0	8	4	4.
	// 								  ~	~	~	~	  ~	~	  ~	~	~	~	~	~	~	~.
	// 上記の~の個所を新詳細コードに変換.
	// (注)全ては数値として使用.

	const uint64_t SerialNo_Mask	 = 0x00FFFFFFFFFFFFFFULL;								// シリアルコードのマスク(計14byte).
	
	uint64_t ui64_NewDetailCode = 0;														///< New detailed code for Phison SSD.
	uint64_t ui64_SerialNo 		= 0;
	pc_SerialNo = pc_SerialNo + 6;															//  the begining index of Phison SSD serial which will be converted to detail code.
	// 6byte目以降を16進数変換.
	ui64_SerialNo = strtol (pc_SerialNo,NULL,16);
	ui64_NewDetailCode  = (uint64_t)TYPD_NewDetailCode_CF_Storage_ID;						// 先頭2byteを設定(0XCF00000000000000).
	ui64_NewDetailCode |= (uint64_t)( ui64_SerialNo ) & SerialNo_Mask;						// 3～16byteを設定(0XCFAAAAAAAAAAAAAA).

	return ui64_NewDetailCode;
}
//関数-------------------------------------------------------------------------------------------------------------------.
// 概要		：create a ToShiBa SSD new detail code from Storage serial number got from Thread.
// 作成者	：.
uint64_t createToShiBaSSDDetailCode( char *pc_SerialNo )
{
	// Toshiba SSD Serial code(Example).
	// |年月日(MYD)	|社内管理コード			|製品情報（固定）
	// ----------------------------------------------------------------------------------.
	// | Z	7	7	| L	L	0	0	0	F	| S	Y	S.
	// 				  ~	~	~	~	~	~.
	// 上記の~の個所を新詳細コードに変換.
	// (注)文字列を数値変換して使用.
	
	const uint64_t ToShiBaSSD_2nd_Mask			= 0x00FF000000000000ULL;	// シリアルコード4byteのマスク.
	const uint64_t ToShiBaSSD_3rd_Mask			= 0x0000FF0000000000ULL;	// シリアルコード5byteのマスク.
	const uint64_t ToShiBaSSD_4th_Mask			= 0x000000FF00000000ULL;	// シリアルコード6byteのマスク.
	const uint64_t ToShiBaSSD_5th_Mask			= 0x00000000FF000000ULL;	// シリアルコード7byteのマスク.
	const uint64_t ToShiBaSSD_6th_Mask			= 0x0000000000FF0000ULL;	// シリアルコード8byteのマスク.
	const uint64_t ToShiBaSSD_7th_Mask			= 0x000000000000FF00ULL;	// シリアルコード9byteのマスク.
	
	const uint32_t ToShiBaSSD_2nd_BitShift		= 48;						// 12桁 x 4bitシフト.
	const uint32_t ToShiBaSSD_3rd_BitShift		= 40;						// 10桁 x 4bitシフト.
	const uint32_t ToShiBaSSD_4th_BitShift		= 32;						//  8桁 x 4bitシフト.
	const uint32_t ToShiBaSSD_5th_BitShift		= 24;						//  6桁 x 4bitシフト.
	const uint32_t ToShiBaSSD_6th_BitShift		= 16;						//  4桁 x 4bitシフト.
	const uint32_t ToShiBaSSD_7th_BitShift		= 8;						//  2桁 x 4bitシフト.
	uint64_t ui64_NewDetailCode = 0;										///< New detailed code for CPU serial number change.
	
	ui64_NewDetailCode = (uint64_t)TYPD_NewDetailCode_CF_Storage_ID;										//  先頭2byteを設定(0XCF00000000000000).
	ui64_NewDetailCode |= (uint64_t)( pc_SerialNo[3] ) << ToShiBaSSD_2nd_BitShift & ToShiBaSSD_2nd_Mask;	//  3～4 byteを設定(0XCFXX000000000000).
	ui64_NewDetailCode |= (uint64_t)( pc_SerialNo[4] ) << ToShiBaSSD_3rd_BitShift & ToShiBaSSD_3rd_Mask;	//  5～6 byteを設定(0XCFXXXX0000000000).
	ui64_NewDetailCode |= (uint64_t)( pc_SerialNo[5] ) << ToShiBaSSD_4th_BitShift & ToShiBaSSD_4th_Mask;	//  7～8 byteを設定(0XCFXXXXXX00000000).
	ui64_NewDetailCode |= (uint64_t)( pc_SerialNo[6] ) << ToShiBaSSD_5th_BitShift & ToShiBaSSD_5th_Mask;	//  9～10byteを設定(0XCFXXXXXXXX000000).
	ui64_NewDetailCode |= (uint64_t)( pc_SerialNo[7] ) << ToShiBaSSD_6th_BitShift & ToShiBaSSD_6th_Mask;	// 11～12byteを設定(0XCFXXXXXXXXXX0000).
	ui64_NewDetailCode |= (uint64_t)( pc_SerialNo[8] ) << ToShiBaSSD_7th_BitShift & ToShiBaSSD_7th_Mask;	// 13～14byteを設定(0XCFXXXXXXXXXXXX00).
	
	return ui64_NewDetailCode;
}

// 2020/01/23 日立 馬場 add Start.
//関数-------------------------------------------------------------------------------------------------------------------.
// 概要		：create a Micron SSD new detail code from Storage serial number got from Thread.
// 作成者	：.
uint64_t createMicronSSDDetailCode( char *pc_SerialNo )
{
	// Micron SSD Serial code(Example).
	// |年 / 週				|LotNo.			|シリアルナンバー
	// ----------------------------------------------------------------------------------.
	// | 1	9	4	9	2	|	5	4	3	|	D	F	E	4	
	// 				  			~	~	~		~	~	~	~	.
	// 上記の~の個所を新詳細コードに変換.
	// (注)文字列を数値変換して使用.
	
	const uint64_t MicronSSD_2nd_Mask			= 0x00FF000000000000ULL;	// シリアルコード4byteのマスク.
	const uint64_t MicronSSD_3rd_Mask			= 0x0000FF0000000000ULL;	// シリアルコード5byteのマスク.
	const uint64_t MicronSSD_4th_Mask			= 0x000000FF00000000ULL;	// シリアルコード6byteのマスク.
	const uint64_t MicronSSD_5th_Mask			= 0x00000000FF000000ULL;	// シリアルコード7byteのマスク.
	const uint64_t MicronSSD_6th_Mask			= 0x0000000000FF0000ULL;	// シリアルコード8byteのマスク.
	const uint64_t MicronSSD_7th_Mask			= 0x000000000000FF00ULL;	// シリアルコード9byteのマスク.
	const uint64_t MicronSSD_8th_Mask			= 0x00000000000000FFULL;	// シリアルコード10byteのマスク.

	
	const uint32_t MicronSSD_2nd_BitShift		= 48;						// 12桁 x 4bitシフト.
	const uint32_t MicronSSD_3rd_BitShift		= 40;						// 10桁 x 4bitシフト.
	const uint32_t MicronSSD_4th_BitShift		= 32;						//  8桁 x 4bitシフト.
	const uint32_t MicronSSD_5th_BitShift		= 24;						//  6桁 x 4bitシフト.
	const uint32_t MicronSSD_6th_BitShift		= 16;						//  4桁 x 4bitシフト.
	const uint32_t MicronSSD_7th_BitShift		= 8;						//  2桁 x 4bitシフト.
	const uint32_t MicronSSD_8th_BitShift		= 0;						//  0桁 x 4bitシフト.
	
	uint64_t ui64_NewDetailCode = 0;										///< New detailed code for CPU serial number change.
	
	ui64_NewDetailCode = (uint64_t)TYPD_NewDetailCode_CF_Storage_ID;										//  先頭2byteを設定(0XCF00000000000000).
	ui64_NewDetailCode |= (uint64_t)( pc_SerialNo[5] ) << MicronSSD_2nd_BitShift & MicronSSD_2nd_Mask;	//  3～4 byteを設定(0XCFXX000000000000).
	ui64_NewDetailCode |= (uint64_t)( pc_SerialNo[6] ) << MicronSSD_3rd_BitShift & MicronSSD_3rd_Mask;	//  5～6 byteを設定(0XCFXXXX0000000000).
	ui64_NewDetailCode |= (uint64_t)( pc_SerialNo[7] ) << MicronSSD_4th_BitShift & MicronSSD_4th_Mask;	//  7～8 byteを設定(0XCFXXXXXX00000000).
	ui64_NewDetailCode |= (uint64_t)( pc_SerialNo[8] ) << MicronSSD_5th_BitShift & MicronSSD_5th_Mask;	//  9～10byteを設定(0XCFXXXXXXXX000000).
	ui64_NewDetailCode |= (uint64_t)( pc_SerialNo[9] ) << MicronSSD_6th_BitShift & MicronSSD_6th_Mask;	// 11～12byteを設定(0XCFXXXXXXXXXX0000).
	ui64_NewDetailCode |= (uint64_t)( pc_SerialNo[10] ) << MicronSSD_7th_BitShift & MicronSSD_7th_Mask;	// 13～14byteを設定(0XCFXXXXXXXXXXXX00).
	ui64_NewDetailCode |= (uint64_t)( pc_SerialNo[11] ) << MicronSSD_8th_BitShift & MicronSSD_8th_Mask;	// 15～16byteを設定(0XCFXXXXXXXXXXXXXX).
	
	
	
	return ui64_NewDetailCode;
}
// 2020/01/23 日立 馬場 add End.
//関数-------------------------------------------------------------------------------------------------------------------.
// 概要		： create a Other SSD new detail code from Storage serial number got from Thread.
// 作成者	：.
uint64_t createOtherSSDDetailCode( char *pc_SerialNo )
{
	// unknown SSD Serial code.
	// 取得できる情報を設定.
	// 先頭12文字を取得し、数値変換した値を新詳細コードに設定.
	const uint64_t SerialNo_Mask					= 0x0000FFFFFFFFFFFFULL;
	const uint32_t TYPD_MAX_OtherSerialNoSSD		= 12;						// 取得するシリアルコードの文字列数.
	
	uint64_t ui64_NewDetailCode				 = 0;								///< New detailed code for Other SSD.
	uint64_t ui64_SerialNo 					 = 0;

	char OtherSerialSSD[TYPD_MAX_OtherSerialNoSSD + 1];
	memset(OtherSerialSSD, '\0', sizeof(OtherSerialSSD));
	strncpy(OtherSerialSSD, pc_SerialNo, TYPD_MAX_OtherSerialNoSSD);
	// 取得したシリアルコードを16進数変換.
	ui64_SerialNo = strtol (OtherSerialSSD,NULL,16);
	
	ui64_NewDetailCode = (uint64_t)TYPD_NewDetailCode_CFFF_OtherSSD_ID;			//  先頭4byteを設定(0XCFFF000000000000).
	ui64_NewDetailCode |= (uint64_t)( ui64_SerialNo ) & SerialNo_Mask;			//  4byte以降を設定(0XCFFFXXXXXXXXXXXX).
	return ui64_NewDetailCode;
}

//関数-------------------------------------------------------------------------------------------------------------------.
// 概要		： create a PhiSon microSD new detail code from Storage serial number got from Thread.
// 作成者	：.
uint64_t createPhisonAdataMicroSDDetailCode( char *pc_SerialNo )
{
	// Micro SD Serial code.
	// 数値変換可能な文字列のみ.
	// 数値変換した値を新詳細コードに設定.
	
	const uint64_t SerialNo_Mask			= 0x00FFFFFFFF000000ULL;
	const uint32_t SerialNo_BitShift			 = 24;					//  6桁 x 4bitシフト.
	const uint32_t TYPD_MAX_OtherSerialNoMicroSD = 8;					// μSDの取得文字列数.

	uint64_t ui64_NewDetailCode = 0;									///< New detailed code for Phison ADATA.
	uint64_t ui64_SerialNo		= 0;
	char OtherSerialSSD[TYPD_MAX_OtherSerialNoMicroSD + 1];
	memset(OtherSerialSSD, '\0', sizeof(OtherSerialSSD));
	strncpy(OtherSerialSSD, pc_SerialNo, TYPD_MAX_OtherSerialNoMicroSD);
	ui64_SerialNo = strtol (OtherSerialSSD,NULL,16);
	
	ui64_NewDetailCode = (uint64_t)TYPD_NewDetailCode_CF_Storage_ID;							//  先頭2byteを設定(0XCF00000000000000).
	ui64_NewDetailCode |= (uint64_t)( ui64_SerialNo ) << SerialNo_BitShift & SerialNo_Mask;		//  3～16byteを設定(0XCFXXXXXXXX000000).
	return ui64_NewDetailCode;
}
// 2019/06/07 EagleH [OP_BTS-5581] HIENG End.