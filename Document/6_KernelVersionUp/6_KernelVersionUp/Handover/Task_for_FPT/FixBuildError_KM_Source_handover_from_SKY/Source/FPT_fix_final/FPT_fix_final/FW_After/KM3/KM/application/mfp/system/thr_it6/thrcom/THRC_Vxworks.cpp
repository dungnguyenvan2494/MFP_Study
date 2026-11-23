/*================================================================================================
［タイトル］	ＶｘＷＯＲＫＳ
================================================================================================*/

/*================================================================================================
	インクルード・ファイル
================================================================================================*/
// 2010/11/15 【LocalUIプロセス化】ライブラリ化（ライブラリ対応）.
#ifndef DIV_MFP_LIB2
#include <errno.h>
#include <dirent.h>
#include <algorithm>


#ifndef TORNADO			// ＰＣシミュレータで有効.
#ifdef SIMULATOR		//! OSヘッダ対応.
#include  "simdefall.h"
#endif
#endif	// #ifdef TORNADO.
#endif // DIV_MFP_LIB2
// 2010/11/15 【LocalUIプロセス化】ライブラリ化（ライブラリ対応）.
#include  "system/thr/thrcom/THRC_Vxworks.h"
// 2010/11/15 【LocalUIプロセス化】ライブラリ化（ライブラリ対応）.
#ifndef DIV_MFP_LIB2
#include  "system/thr/thrcom/THRC_Multithread.h"
#include "system/sys/OsWrapper/SYSC_OSWrapper.h"	// これはインクルードを許可する.
#include  "system/cnv/cmp/CNVC_LocalCompressorImpColorBand.h"
#include  "system/cnv/cmp/CNVC_LocalCompressorImpColorPage.h"
#include  "system/cnv/cmp/CNVC_LocalDecompressorImp.h"
#include  "system/cnv/cmp/CNVC_LocalAsicToolImp.h"
#include  "system/cnv/CNVC_CnvIf.h" 
#include  "system/pwr/pwrc/PWRC_PowerImpColor.h"
#include  "system/thr/thrInclude/THRC_ThreadInstance.h"
#include  "system/thr/thrInclude/THRC_Vxworks.h"
#include  "system/thr/thrInclude/THRC_BSPif.h"
#ifdef COSIM
#include "system/thr/COSIM/THRC_CosimDebug.h"
#endif
#include "THRC_ThreadSimWin.h"
// 2010/11/24 UV-00794 デバッグログ収集システム対応 M.Koshimo start.
#include "../../../../../shlib/LogLib/LIBC_LogLib.h"
// 2010/11/24 UV-00794 デバッグログ収集システム対応 M.Koshimo end.
#include "system/sys/APIC_SystemInfo.h"

#include "Linux/Platform/Mediaif.h"

#include "system/sys/APIC_System_DEF.h" // 2018/04/20 Sky K.Yamagishi [DenebMLK]制御構成変更No.66 横流れ対応.

#ifdef TORNADO
// VXWORKSで有効.
//#include  "cacheLib.h".
//#include  "drv/hdisk/ataDrv.h".
extern "C" BOOL EraseNvramBackupArea( short ss_area1 , int32_t si_area2 );	// フラッシュバックアップエリアの削除.
#endif// #ifdef TORNADO.

// 2022/03/30 生産RINNE対応 Sky start.
#ifdef SIMULATOR
// 07/08/17 moirkawa Donau CT検証用にdebugLogの出力をOutputに可能にするようにする.
//#define printf	TRACE
#define SYSD_WEBAPI_LINEMODE_PRINTTRACE
#endif	// #ifdef SIMULATOR.
// 生産コマンドをWebAPI化するためprintfをマクロで置き換え.
#include "system/sys/webapi/SYSC_WebAPILineMacro.h"
// 2022/03/30 生産RINNE対応 Sky end.

#include  "system/doc/docg/DOCC_DocInstanceTbl.h"

// FWリファクタリング APIC_Setting.hの分割.
#include "system/nvd/APIC_Setting_MachineSetting.h"
#include "system/nvd/APIC_Setting_MailSendSetting.h"
#include "system/nvd/APIC_Setting_Network.h"
#include "system/nvd/APIC_Setting_PrintJobSetting.h"
#include "system/nvd/APIC_Setting_RdsSetting.h"
#include "system/nvd/APIC_Setting_SoftDipSw.h"
#include "system/nvd/APIC_Setting_SystemSetting.h"
#include "system/nvd/APIC_Setting_TcpIpSetting.h"

//FWリファクタリング NVDC_Setting.hを分割.
#include "system/nvd/nvdg/NVDC_Setting_MachineSetting.h"


#include "system/sys/Timer/SYSC_RDSTimer.h"
#include "system/sys/APIC_Enabler.h"

// STBD_StubEnable.
#ifdef STBD_StubEnable
#include "STB/thr/STB_Thread.h"
#endif // STBD_StubEnable.

// 【追加】自動評価対応 [start] 2008.08.11 鈴木希 自動評価システム環境では、メッセージキューのメッセージ数を増加する.
#if defined( DEF_SimAutoTest )		// 自動評価で有効.
#include "system/cnv/CNVC_CnvIf.h"
#endif	// #if defined( DEF_SimAutoTest )		// 自動評価で有効.
// 【追加】自動評価対応 [ end ] 2008.08.11 鈴木希 自動評価システム環境では、メッセージキューのメッセージ数を増加する.

#if defined(DEF_TaigaCus) || defined(DEF_DonauBKCus) // 2011/02/22 ft_kondo 第6回手動マージ. 
// CQ.324008 FC21の保存.
ushort us_waitDebugForeverCode = 0x0000;
#endif

// add itakura 工場調整モード追加対応 -->
#include "system/thr/SATA2/THRC_HddTrans.h"
#include "system/sys/APIC_HddInfo.h"
#include "system/thr/thrcom/THRC_MEMFunc.h" 
extern void CFStart( short ss_FunctionCode );
// add itakura 工場調整モード追加対応 <--

// 2011.05.17 horishita: 静電TPキャリブレーション対応.
#include "system/nvd/nvdg/NVDC_ClassIF.h"

#include "system/thr/AndroidCtrl/THRC_AndroidControl.h"
#include "system/nvd/APIC_NVBackup.h"
#include "system/thr/thrcom/THRC_Threadif.h"

// 2017/02/20 Sky酒井（純）ユーザーイベント記録(APIコール).
#include "system/sys/OsWrapper/SYSC_OSWrapperTaskMonitorLinux.h"

// 2018/06/20 Sky小久保 .
#include "system/scn/SCNC_ScnIf.h"

// 2018/10/19 橋本裕 AR.1065702 SetOwnGenericの後に証明証発行できるようにする対応.
#include "system/fil/FILC_FilIf.h"
#include "system/fil/filg/FILC_BaseDir.h"

// 2019/11/22 Sky吉村(魁) OP_BTS-14959 制御構成変更No.52 SetMachineType_ToRam対応.
#include <map>

// 2014/06/02 CQ390530対応.
#define THRD_TYPE_GENERIC	1
#define THRD_TYPE_OWN		2
#define THRD_TYPE_DEVELOP	4


// 2014/08/22 CQ:391523 Sky O.Suzuki 出荷時インストール済の自己証明書でアプリ登録ができない対応.
// 自己証明書インストール ターミナルコマンド関連.
#define TERM_OF_VALIDITY					1825		// 自己証明書の有効期間.
#define MAX_MacAddressForSelfCertificate	6 			// MACアドレス取得桁数.
#define THRD_CertCityNameStr				"CHIYODA"	// L(市区町村名).
#define THRD_CertStateNameStr				"TOKYO"		// S(都道府県名).
#define THRD_CertCountryCodeStr				"JP"		// C(国別記号).

#define THRD_TERM_VALIDITY_YEAR				2037		// 自己証明書の有効期間(年).
#define THRD_TERM_VALIDITY_MONTH			12		// 自己証明書の有効期間(月).
#define THRD_TERM_VALIDITY_DAY				31		// 自己証明書の有効期間(日).
#define THRD_TERM_VALIDITY_HOUR				23		// 自己証明書の有効期間(時).
#define THRD_TERM_VALIDITY_MINUTE			59		// 自己証明書の有効期間(分).
#define THRD_TERM_VALIDITY_SECOND			59		// 自己証明書の有効期間(秒).
#define THRD_SECONDS_DAY				(60*60*24)	// 1日の秒数.

// 2015/03/05 Sky M.Furuichi	Macアドレス取得ターミナルコマンド追加対応.
#define THRD_MacAddress_Disp_Size			32		// Macアドレスをターミナルに表示する文字列のサイズ.
#define THRD_Get_MacAddress_Size			6		// SPI-Flashから読み出すMacアドレスのサイズ.
#define THRD_Get_MacAddress_Offset			0x63	// SPI-FlashからMacアドレスを取得する際に用いるoffset.

// 2015/03/12 Sky M.Furuichi	[Helios]制御構成変更No.43 Cコード、Pコール、JAM履歴取得ターミナルコマンド追加対応.
// 警告カウンタ種別名称.
// 2017/10/30 Sky 松井	[Eagle]制御構成変更No.52 サービスモードのマシン状態/管理にて気圧を表示できるようにする対応.
#define		THRD_WarningName_WP5					"P-5 "		// ＷＰ－　５：ＡＩＤＣセンサ異常（手前）.
#define		THRD_WarningName_WP6					"P-6 "		// ＷＰ－　６：シアンＩＵ異常.
#define		THRD_WarningName_WP7					"P-7 "		// ＷＰ－　７：マゼンタＩＵ異常.
#define		THRD_WarningName_WP8					"P-8 "		// ＷＰ－　８：イエローＩＵ異常.
#define		THRD_WarningName_WP9					"P-9 "		// ＷＰ－　９：ブラックＩＵ異常.
#define		THRD_WarningName_WP21					"P-21"		// ＷＰ－２１：色ずれ補正異常（テストパターン異常）.
#define		THRD_WarningName_WP22					"P-22"		// ＷＰ－２２：色ずれ補正異常（補正量異常）.
#define		THRD_WarningName_WP23					"P-23"		// ＷＰ－２３：Ｃ１次転写ＡＴＶＣ異常.
#define		THRD_WarningName_WP24					"P-24"		// ＷＰ－２４：Ｍ１次転写ＡＴＶＣ異常.
#define		THRD_WarningName_WP25					"P-25"		// ＷＰ－２５：Ｙ１次転写ＡＴＶＣ異常.
#define		THRD_WarningName_WP26					"P-26"		// ＷＰ－２６：Ｋ１次転写ＡＴＶＣ異常.
#define		THRD_WarningName_WP27					"P-27"		// ＷＰ－２７：２次転写ＡＴＶＣ異常.
#define		THRD_WarningName_WP28					"P-28"		// ＷＰ－２８：ＡＩＤＣセンサ異常（奥）.
#define		THRD_WarningName_WI1					"S-1 "		// ＷＩ－　１：ＣＣＤクランプゲイン調整異常.
#define		THRD_WarningName_PCHomeSensorErrorCMY	"P-30"		// ＰＣホームセンサー不良（カラー）.
#define		THRD_WarningName_PCHomeSensorErrorK		"P-31"		// ＰＣホームセンサー不良（ブラック）.
#define		THRD_WarningName_1stTransferATVCCMY		"P-29"		// カラー１次転写ＡＴＶＣ異常.
#define		THRD_WarningName_PriorDetectionDust		"D-1 "		// ノイズ検出.
#define		THRD_WarningName_ReadGuideWarning		"D-2 "		// Ｄ－２：読取ガイド異常 .
#define		THRD_WarningName_SkewReviseAbnormality	"P-14"		// ＷＰ－１４：スキュー補正異常 .
#define		THRD_WarningName_ElectricCHCleaning1	"P-16"		// ＷＰ－１６：帯電CH清掃１ .
#define		THRD_WarningName_ElectricCHCleaning2	"P-18"		// ＷＰ－１８：帯電CH清掃２ .
#define		THRD_WarningName_FusingSensorNCDir		"P-32"		// 定着センサ温度検出不良. 
#define		THRD_WarningName_CISGain				"S-2 "		// Ｓ－２：CISゲイン調整不良. 
#define		THRD_WarningName_PriorDetectionDustBack	"D-3 "		// ノイズ検出(裏面用).
#define		THRD_WarningName_AbnormalLD				"P-33"		// LD異常. 
#define		THRD_WarningName_AbnormalcyAirPressure	"P-34"		// 気圧センサ異常.
#define		THRD_WarningName_MediaSensorError		"P-35"		// メディアセンサ異常.  2017/11/02 Sky小久保 紙種自動検知(メディアセンサ異常追加).
#define		THRD_WarningName_MediaSensorError2		"P-36"		// メディアセンサ異常2. 2018/11/08 橋本裕 [EagleH] RQ-720 紙種自動検知(メディアセンサ異常2追加).
#define		THRD_WarningName_PaperWidthError		"P-38"		// 用紙幅センサ異常.	2019/02/07 Sky千葉 [EagleH] 機器構成変更No.30 用紙幅センサ異常定義追加.
#define		THRD_WarningName_OpticalSystemDirtP37	"P-37"		// PH光学系汚れ.		2019/02/07 Sky千葉 [EagleH] 機器構成変更No.31 PH光学系汚れ定義追加.
#define		THRD_WarningName_TackingFanError		"P-39"		// タッキングFAN異常.	2019/03/04 Sky合屋 [EagleH] 機器構成変更No.29 タッキングFAN異常定義追加.
#define		THRD_WarningName_MotorError				"P-40"		// 定着モータトルク異常.2019/07/02 Sky鳥居 [EagleZ+]機器構成変更No.2 定着モータトルク異常追加.

#define		THRD_WarningCnt_InternalTemperatureSensorError	"P-41"	// P-41：機内温度センサ異常 [IT6_2.0MR3] 機器構成変更No.1 温度センサPコール追加.
#define		THRD_WarningCnt_PHTemperatureSensorError		"P-42"	// P-42：PH温度センサ異常 [IT6_2.0MR3] 機器構成変更No.1 温度センサPコール追加.
#define		THRD_WarningCnt_PaperTemperatureSensorError		"P-43"	// P-43：P-43：用紙温度センサ異常 [IT6_2.0MR3] 機器構成変更No.1 温度センサPコール追加.

// 給紙トレイ名称.
#define		THRD_TrayName_Manual					"Manual"	// 手差しトレイ.
#define		THRD_TrayName_Tray1						"Tray1 "	// トレイ１.
#define		THRD_TrayName_Tray2						"Tray2 "	// トレイ２.
#define		THRD_TrayName_Tray3						"Tray3 "	// トレイ３.
#define		THRD_TrayName_Tray4						"Tray4 "	// トレイ４.
// 2018/11/02 [DenebMLK][AR1055247]トレイ5追加対応 Sky新大阪 ->.
#if  defined(DEF_DenebMLKCus)
#define		THRD_TrayName_Tray5						"Tray5 "	// トレイ５.
#else
#define		THRD_TrayName_Tray5						"LCT   "	// LCT.
#endif
// 2018/11/02 [DenebMLK][AR1055247]トレイ5追加対応 Sky新大阪 <-.


// 2019.01.28 [EagleH]制御構成変更No.60 自己診断結果収集機能API追加対応 ->.
#define		THRD_CIDDeviceFileName					"/sys/block/kmsda/device/cid"			// CIDデバイスファイル名.
#define		THRD_CIDStringBufferLength				50										// CID文字列の取得Buffer長.
#define		THRD_uSDSeriarlLength					 8										// maicroSDのシリアルNo文字列長.
#define		THRD_PSNStartPositionInCID				18										// CIDにおけるPSN開始位置.
#define		THRD_CharBlank							' '										// 空白文字.
// 2019.01.28 [EagleH]制御構成変更No.60 自己診断結果収集機能API追加対応 <-.

// 2019.06.30 [EagleH]制御構成変更No.60 OP_BTS-5581_メーカーID取得API追加対応. <-.
#define		THRD_VenderID_DeviceFileName			"/sys/class/nvme/nvme0/device/vendor"	// ベンダID格納先デバイスファイル名.
#define		THRD_VenderID_StrSize					 4										// ベンダIDのサイズ.
#define		THRD_VenderPreID_StrSize				 2 										// ベンダIDの接頭語文字列数.
// 2019.06.30 [EagleH]制御構成変更No.60 OP_BTS-5581_メーカーID取得API追加対応 <-.

// 2021.03.02 PhisonSSD 96層対応 ->.
#define		THRD_DeviceID_DeviceFileName			"/sys/class/nvme/nvme0/device/device"	// デバイスID格納先デバイスファイル名.
#define		THRD_DeviceID_StrSize					 4										// デバイスIDのサイズ.
#define		THRD_DevicePreID_StrSize				 2 										// デバイスIDの接頭語文字列数.
// 2021.03.02 PhisonSSD 96層対応 <-.

// 警告カウンタ種別データ.
typedef struct {
	TYPE_WarningCnt		e_WarningCntID;		// 警告カウンタ種別ID.
	char*				sc_WarningName;		// 警告カウンタ種別名称.
}THRS_WariningCnt;							// 2016/04/12	Warning対応

// トレイ名称データ.
typedef struct {
	TYPE_FeedTray		e_FeedTrayID;		// トレイ選択ID.
	char*				sc_FeedTrayName;	// トレイ名称.
}THRS_FeedTray;								// 2016/04/12	Warning対応

// totalclear実行指定状態設定値	[Poseidon量産問題]No7 TotalClearの実行有無判定の状態設定値.
#define THRE_ExecTotalClearState_NotReady				(0)		// 判定準備中　　　：　"Not Ready ExecTotalClear".
#define THRE_ExecTotalClearState_NeedTotalClear			(1)		// totalclear必要　：　"Need TotalClear".
#define THRE_ExecTotalClearState_UnnecessaryTotalClear	(2)		// totalclear不要　：　"Unnecessary TotalClear".

#define THRD_DispUnknown						"Unknown"	// 機種情報、機種No.データ不正時表示.

// MinervaSBK制御構成変更No.70 OEMデータのダウンロード対応.
#define THRD_DevName_USB					"/bd/sdc1"								// USBメモリデバイス名. 
#define	THRD_SearchFileOemData				"*_oem.bin"								// OEMドライバ名称カスタマイズ データファイル検索用.
#define	THRD_SearchFileCustomImage			"customimage*.tar"						// OEMマシン図データファイル検索用.
#define	THRD_SearchFileBootUpScreen			"BootUpScreen*.zip"						// BootUpScreenファイル検索用.
#define	THRD_MachineSsdDir					"/ata0a/"								// OEMマシン図データのSSD格納場所.
#define	THRD_MachineSsdDirWithFilename		"/ata0a/image/machine/customimage.bin"	// OEMマシン図データのSSD格納場所フルパス.
#define	THRD_ExtensionPeriodZip				".zip"									// .zipファイルの拡張子.
#define	THRD_ZipFileExtension				"zip"									// BootUp Screen zipファイルの拡張子.
#define	THRD_RecFileExtension				"bin"									// BootUp Screen zip解凍後の拡張子.
#define	THRD_VerFileName					"Version.txt"							// BootUp Screen バージョンテキスト名称.

#define	THRD_CallbackSuccess		(0x00)
#define	THRD_CallbackError			(0x01)
#define	THRD_CallbackTimeout		(0x02)
#define	THRD_CallbackUnknown		(0xFF)

#define	THRD_OEMTaskDelayTick		(20)		//100ms delayする.
#define	THRD_OEMTimeOutCount		(600)		//100ms*600で60秒でタイムアウト.

#define	THRD_CmdNameLen				(256+10)


// 2016/01/22	itakura	[CQ521426] VenusMLK WBS-1457 生産コマンド追加対応.
// 2016/06/06	itakura	[VenusMLK]CQ526976 無線アダプタ UK-212非サポート対応.
//						          無線LANのベンダVID, PIDをUK-215のものに修正.
#if defined(DEF_A4_Cus)
#define THRD_UsbConnectCheck_Error		-1
static char sc_FaxDevIdtbl[][10] = {
	"132b:20b7",				// VID:132b		PID:20b7	ベンダ名: Konica Minolta.
};

static char sc_WlanDevIdtbl[][10] = {
	"0a5c:bd27",				// VID:0a5c		PID:bd27	ベンダ名: Broadcom.
};
#endif	//defined(DEF_A4_Cus).

// 2018/04/20 Sky K.Yamagishi [DenebMLK]制御構成変更No.66 横流れ対応.
#define THRD_MacAddress_DispStr				"MacAddress %c%c-%c%c-%c%c-%c%c-%c%c-%c%c\n"	// 工場出荷時Macアドレス表示用文字列定義.

#define THRD_PrinterAdjust_LDLight_Max	(10)					//設定値の最大値 LD光量バランス補正値.
#define THRD_PrinterAdjust_LDLight_Min	(-10)					//設定値の最小値 LD光量バランス補正値.

// 2019/11/22 Sky吉村(魁) OP_BTS-14959 制御構成変更No.52 SetMachineType_ToRam対応.
typedef char THRE_MACHINETYPESELECT_RESULT;
#define THRE_MACHINETYPESELECT_SUCCESS		(0)
#define THRE_MACHINETYPESELECT_INFO_ERROR	(1)
#define THRE_MACHINETYPESELECT_NUM_ERROR	(2)
#define THRE_MACHINETYPESELECT_ARGS_ERROR	(3)

//	2021/03/03	Sky	OP_BTS-31220 生産ライン用 APPコマンド追加.
typedef char THRE_NwIfMode;
#define THRE_NwIfMode_SingleEthernet			(0)		// シングルIF構成 有線モード.
#define THRE_NwIfMode_DualEthernetSta			(1)		// デュアルIF構成 有線＋無線子機モード.
#define THRE_NwIfMode_SingleSta					(2)		// シングルIF構成 無線子機モード.
#define THRE_NwIfMode_DualEthernetAp			(3)		// デュアルIF構成 有線＋無線APモード.
#define THRE_NwIfMode_DualEthernetWfd			(4)		// デュアルIF構成 有線＋Wi-Fi Directモード.
#define THRE_NwIfMode_DualEthernet				(5)		// デュアルIF構成 有線モード.
#define THRE_NwIfMode_DualEthernetWolLimited	(6)		// デュアルIF構成 有線＋無線子機（ErP復帰限定）モード.

//	2021/04/15	Sky	OP_BTS-32090 生産ライン用設定コマンド追加対応.
#define THRE_2ndNwAdapter_OFF  (0) // 2ndネットワークアダプタ非装着.
#define THRE_2ndNwAdapter_ON   (1) // 2ndネットワークアダプタ装着.

/*================================================================================================
	モジュール内変数の宣言
================================================================================================*/
extern	MSG_Q_ID msgQID_ExtToBuf;		// 外部→バッファメモリ.
extern	MSG_Q_ID msgQID_BufToFile;		// バッファメモリ→ファイルメモリ.
extern	MSG_Q_ID msgQID_AppToBuf;		// アプリ→バッファメモリ.
extern	MSG_Q_ID msgQID_AppToFile;		// アプリ→ファイルメモリ.
extern	MSG_Q_ID msgQID_BufToExt;		// バッファメモリ→外部.
extern	MSG_Q_ID msgQID_FileToBuf;		// バッファメモリ→バッファメモリ.
extern	MSG_Q_ID msgQID_BufToBuf;		// バッファメモリ→バッファメモリ.
extern	MSG_Q_ID msgQID_BufToApp;		// バッファメモリ→アプリ.
extern	MSG_Q_ID msgQID_FileToApp;		// ファイル→アプリ.
extern	MSG_Q_ID msgQID_AppToApp;		// アプリ→アプリ.
extern	MSG_Q_ID msgQID_SaveMedia;		// 保存メディアドライバ.
extern	MSG_Q_ID msgQID_SpecialJobCont;	// 特殊ジョブ制御 [追加] 04/03/21 kuroda.

extern	MSG_Q_ID msgQID_ImagePattern;

// サブタスク（画像データ転送）.
extern	MSG_Q_ID msgQID_Input;
extern	MSG_Q_ID msgQID_Expand;
extern	MSG_Q_ID msgQID_ImgBufEngOut;
extern	MSG_Q_ID msgQID_Compress;
extern	MSG_Q_ID msgQID_HDDTrans;
extern	MSG_Q_ID msgQID_WorkInput;
extern	MSG_Q_ID msgQID_WorkOutput;
#ifdef DEF_Soc1200
extern	MSG_Q_ID msgQID_MainToSubF;		// 2013/07/12 ADS岡村 CQ.381171(Act_66426) SC-SFM-4 サブメモリのファイルメモリ化.
extern	MSG_Q_ID msgQID_SubFToMain;		// 2013/07/12 ADS岡村 CQ.381171(Act_66426) SC-SFM-4 サブメモリのファイルメモリ化.
#endif	// DEF_Soc1200.
// メインタスク-- 優先順位変更.
extern	MSG_Q_ID msgQID_Image;
extern	MSG_Q_ID msgQID_ThreadManager;
// スレッド層　画像制御層（高速タスク）タスク.
extern  MSG_Q_ID msgQID_ClearImage;
extern  MSG_Q_ID msgQID_JpegImage;
extern  MSG_Q_ID msgQID_FaxCmpExp;
extern  MSG_Q_ID msgQID_ImageConv;
extern  MSG_Q_ID msgQID_ImgSynthesis;
extern  MSG_Q_ID msgQID_AppInConvDiv;	// 11/09/01 山下		アクセラレータAsicオプション化対応.
extern  MSG_Q_ID msgQID_MultiTransData;			// 13/03/19 Sky ZeusS対応.
extern  MSG_Q_ID msgQID_InputImageConv;

#if (defined(COSIM) && defined(THR_SIMWIN)) || (defined(SIMULATOR) && defined(THR_SIMWIN)) // 【COSIM+ｽﾚｯﾄﾞSIMWIN対応】.
// スレッドSimWinの仮想上位タスク.
extern	MSG_Q_ID msgQID_TestInputIR;	// 仮想　1ページIR入力シーケンス.
extern	MSG_Q_ID msgQID_TestInputIRDup;	// 仮想　1ページIR入力裏面シーケンス 2008/08/28 R.Suzuki CQ.309145 1Pass対応.
extern	MSG_Q_ID msgQID_TestInputExtIf;	// 仮想　1ページ外部IF入力シーケンス.
extern	MSG_Q_ID msgQID_TestInputApp;	// 仮想　1ページアプリ入力シーケンス.
extern	MSG_Q_ID msgQID_TestOutputEng;	// 仮想　1ページエンジン出力シーケンス.
extern	MSG_Q_ID msgQID_TestOutputApp;	// 仮想　1ページアプリ出力シーケンス.
extern	MSG_Q_ID msgQID_TestJpegTool;	// 仮想　JPEGツールシーケンスタスク.
#endif

extern  MSG_Q_ID msgQID_HDDOneTrans;
extern  MSG_Q_ID msgQID_PowerManager;	// 電源管理.
extern  MSG_Q_ID msgQID_LEDStatus;		// LED状態表示.

extern  MSG_Q_ID msgQID_CancelDebug;	// キャンセルテストツール用.

extern	MSG_Q_ID msgQID_PIC_IF;			// ＰＩＣ・インターフェース★★.
extern	MSG_Q_ID msgQID_Client;			// ソケット通信クライアント.
extern	MSG_Q_ID msgQID_TestDrv;		// シミュレーション専用タスク「CoSim検証用タスク」.

extern	WDOG_ID wdID_Client;			// ソケット通信クライアント.
extern	WDOG_ID wdID_TestDrv;			// シミュレーション専用タスク「CoSim検証用タスク」.
extern	WDOG_ID wdID_PIC_IF;			// ＰＩＣ・インターフェース★★.

extern	MSG_Q_ID msgQID_InitPIC;		// Rhine1 PIC初期化タスク.
extern	MSG_Q_ID msgQID_XPanel_IF;		// Rhine1P 外部パネルタスク.
extern	WDOG_ID wdID_XPanel_IF;			// Rhine1P 外部パネルタスク.

extern	MSG_Q_ID msgQID_AesTrans;

extern	MSG_Q_ID msgQID_ThDebugLog;		// CQ.361271 デバッグログ収集システムタスク.
extern  MSG_Q_ID msgQID_JpegAndroid;
extern	MSG_Q_ID msgQID_AndroidControl;	//!< Android制御.
extern	MSG_Q_ID msgQID_AndroidJpegControl;	//!< AndroidJpeg制御.
extern	MSG_Q_ID msgQID_AsicComIPRomPara;	//!< IPRomイベント受信タスク.
extern	MSG_Q_ID msgQID_SSDKeepData;		//!< 88 SSDデータ消失防止 定期Readタスク.
extern	MSG_Q_ID msgQID_AndroidConnection;	//!< Android接続制御.
#ifdef FUM_LINUX
extern  MSG_Q_ID msgQID_SimThread;		// HCL- Thread added for tss file processing queue. 03-14-2013
#endif
extern  MSG_Q_ID msgQID_AesDmaMonitor;
extern	MSG_Q_ID msgQID_S800jControl;		// S800J制御管理タスク.
extern	MSG_Q_ID msgQID_S800jControl0;		// S800J制御タスク(Scan用Core0).
extern	MSG_Q_ID msgQID_S800jControl1;		// S800J制御タスク(Android用Core0).
extern	MSG_Q_ID msgQID_JPEGProxy0;			// JPEGプロキシタスク(Scan用Core0).
extern	MSG_Q_ID msgQID_JPEGProxy1;			// JPEGプロキシタスク(Android用Core1).
extern	MSG_Q_ID msgQID_ThMeminfo;			// メモリ使用量監視タスク.
// OP_BTS-40890_タスクモニタ新詳細データ化対応_S.
extern	MSG_Q_ID msgQID_TotalingNewDetail;	// タスクモニタ新詳細コード化タスク.
// OP_BTS-40890_タスクモニタ新詳細データ化対応_E.

// 2019/08/30【EagleH】OP_BTS-12474 RTC時刻を設定する箇所をcb_RTC.writeからSYSF_setRealTimeClockへ変更 ->.
extern void SYSF_setRealTimeClock(SYSS_AbsTime*);	// RTC設定.
// 2019/08/30【EagleH】OP_BTS-12474 RTC時刻を設定する箇所をcb_RTC.writeからSYSF_setRealTimeClockへ変更 <-.

// 【注意】06/01/05 松永.
//	TIDを修正した場合は、getTaskName( ushort us_TaskId ) 関数内の.
//	タスク名定義も合わせて修正すること。.

#ifdef TORNADO		// VXWORKSで有効.
struct ST_Task {
	MSG_Q_ID*	msgQID;		// メッセージキューIＤテーブル.
	WDOG_ID*	wdID;		// ウォッチドッグＩＤテーブル.
	int32_t	sl_Tick;	// 定期起床用カウント数.
};
#else	//TORNADO.
struct ST_Task {
	MSG_Q_ID*	msgQID;		// メッセージキューIＤテーブル.
	WDOG_ID*	wdID;		// ウォッチドッグＩＤテーブル.
	int32_t		sl_Tick;	// 定期起床用カウント数.
};
#endif	//TORNADO.

//	2015/03/17	Ogura	Poseidon 生産ラインAPP開始できない問題対応.
static Bool bl_AppStartUpFlag = False;
//	2021/01/12	Sky		生産コマンドの投入可能タイミング変更対応.
static Bool bl_ConfigConfirmed = False;

static struct ST_Task st_Task[] = {	// // 01/07/11 TT ↓bcVxWorks.hのTID_の番号と順番を合わせないとだめ！！.
	// サブタスク（画像データ転送）↓ 01/07/11 TT 番号を変更する場合はbcVxWorks.cppのテーブル順と合わせた変更が必要！！.
	{ &msgQID_Input,		NULL,			    0L },	// 0 TID_Input,      	// 画像入力.
	{ &msgQID_Expand,	 	NULL,			    0L },	// 1 TID_Expand,     	// 画像伸張.
	{ &msgQID_ImgBufEngOut,	NULL,			    0L },	// 2 TID_Output,     	// 画像出力.
	{ &msgQID_Compress,	 	NULL,			    0L },	// 3 TID_Compress,   	// 画像圧縮.
	{ &msgQID_HDDTrans,	 	NULL,			    0L },	// 4 TID_HDDTrans,   	// ＨＤＤデータ転送.
	{ &msgQID_WorkInput,	NULL,			    0L },	// 5 TID_WorkInput,		// ワーク画像入力.
	{ &msgQID_WorkOutput, 	NULL,			    0L },	// 6 TID_WorkOutput, 	// ワーク画像出力.
	// メインタスク.
	{ NULL,		 			NULL,		    	0L },	// 7 TID_Read,       	// 原稿読み取り部.
	{ NULL,					NULL,			    0L },	// 8 TID_FileMemEntry,	// ファイルメモリ登録.
	{ NULL,					NULL,			    0L },	// 9 TID_FileMemRead,	// ファイルメモリ読み出し.
	{ NULL,					NULL,	    		0L },	// 10 TID_Print,      	// 用紙プリント部.
	{ NULL,					NULL,	    		0L },	// 11 TID_Panel,      	// パネル部.
	{ NULL,	 				NULL,	    		0L },	// 12 TID_JobMan,     	// ジョブ管理部.
	{ NULL,					NULL,   			0L },	// 13 TID_MachineMan, 	// マシン管理部.
	{ &msgQID_Image,		NULL,				0L },	// 14 TID_Image,      	// 画像・メモリ管理部.
	{ &msgQID_InitPIC,	 	NULL,		    	0L },	// 15 TID_InitPIC,      // Rhine1 PIC初期化タスク.
	{ &msgQID_SaveMedia, 	NULL,		    	0L },	// 16 TID_SaveMedia,    // Rhine1 保存メディアドライバ.
	{ &msgQID_SpecialJobCont,NULL, 		    	0L },	// 17 TID_SpecialJobCont // 特殊ジョブ制御部 [修正] 04/03/21 kuroda.
	{ NULL,					NULL,			    0L },	// 18 TID_NTF_Convert	// 外部I/F変換部.
	// サブタスク（定期起床）.
	{ NULL,	 				NULL,	    		0L },	// 19 TID_IRC_IF,     	// ＩＲＣ・インターフェース.
	{ NULL,					NULL,    			0L },	// 20 TID_Engine_IF,  	// エンジン・インターフェース.
#ifdef DEF_Soc1200
	{ &msgQID_MainToSubF,	NULL,			    0L },	// 21 TID_MainToSubF,	// メインメモリtoサブファイルメモリ DMA01使用[Main→Sub/Main]  2013/07/12 ADS岡村 CQ.381171(Act_66426) SC-SFM-4 サブメモリのファイルメモリ化.
	{ &msgQID_SubFToMain, 	NULL,			    0L },	// 22 TID_SubFToMain, 	// サブファイルメモリtoメインメモリ DMA10使用[Sub/Main→Main]  2013/07/12 ADS岡村 CQ.381171(Act_66426) SC-SFM-4 サブメモリのファイルメモリ化.
#else
	{ NULL,					NULL,	    		0L },	// 21 TID_NTF_IF,		// 外部I/F・インターフェース.
	{ NULL,	 				NULL,     			0L },	// 22 TID_EPNet_IF,   	// ＥＰＮｅｔ・インターフェース.
#endif	// DEF_Soc1200.
	{ NULL,					NULL, 				0L },	// 23 TID_EPNet_Event,	// EPNet Event.
	{ NULL,					NULL, 				0L },	// 24 TID_EPNet_DTBIOS,	// EPNet DTBIOS.
	{ NULL,					NULL,    			0L },	// 25 TID_MinetA_IF,  	// ＭＩＮＥＴＡ・インターフェース.
	{ NULL,	 				NULL,     			0L },	// 26 TID_PanelCPU,   	// パネルＣＰＵ・インターフェース.
	// サブタスク（処理時間長い）.
	{ NULL,					NULL,			    0L },	// 27 TID_PanelDraw,  	// パネルＬＣＤ描画.
//	{ NULL,					NULL,	    		0L },	// 28 TID_FlashROM,  	// フラッシュＲＯＭ.
	// ◆Start_2006/03/31_kurachi_文字画像合成.
	//{ &msgQID_JimonPattern,	NULL,	    		0L },	// 28 TID_JimonPattern, // 地紋パターン.
	{ &msgQID_ImagePattern,	NULL,				0L },	// 28 TID_ImagePattern, // 文字画像合成パターン.
	// ◆End_2006/03/31_kurachi_文字画像合成.
	{ NULL,					NULL,			    0L },	// 29 TID_EPNet_Data	// EPNet Data.
	{ NULL,					NULL,			    0L },	// 30 TID_EPNet_DT		// EPNet DT.
	{ NULL,					NULL,			    0L },	// 31 TID_EPNet_ExIF	// EPNet ExIF.
	{ NULL,					NULL,			    0L },	// 32 TID_EPNet_MIO1	// EPNet MIO1.
	{ NULL,					NULL,			    0L },	// 33 TID_EPNet_MIO2	// EPNet MIO2.
	// その他.
	{ &msgQID_Client,	 	&wdID_Client,	    0L },	// 34 TID_Client,		// ソケット通信クライアント.
#ifdef COSIM
	{ &msgQID_TestDrv,	 	&wdID_TestDrv,	    0L },	// 35 TID_TestDrv,		// シミュレーション専用「CoSim検証用」.
#else
	{ NULL,					NULL,			    0L },	// 35 TID_TestDrv,		// シミュレーション専用「CoSim検証用」.
#endif
	{ NULL,		 	NULL,						0L },	// 36 TID_TestAsic,		// シミュレーション専用「ASICシミュレータ」.
// タスク層の１ページシーケンスタスク.
	{ NULL,		 			NULL,			    0L },	// 37 TID_InputIR.
	{ NULL,					NULL,				0L },	// 38 TID_InputExtIf.
	{ NULL		,		 	NULL,			    0L },	// 39 TID_InputApp.
	{ NULL,					NULL,				0L },	// 40 TID_OutputEng.
	{ NULL,					NULL,				0L },	// 41 TID_OutputApp.
	{ NULL,					NULL,				0L },	// 42 TID_OutputExtIf.
// スレッド層のタスク.
	{ &msgQID_ExtToBuf,		NULL,				0L },	// 43 TID_ExtToBuf.
	{ NULL,					NULL,				0L },	// 44 TID_ExtToFile.
	{ NULL,					NULL,				0L },	// 45 TID_ExtToBufAndFile.
	{ &msgQID_BufToFile,	NULL,				0L },	// 46 TID_BufToFile.
	{ NULL,					NULL,				0L },	// 47 TID_IRToExtMem.
	{ &msgQID_AppToBuf,		NULL,				0L },	// 48 TID_AppToBuf.
	{ &msgQID_AppToFile,	NULL,				0L },	// 49 TID_AppToFile.
	{ NULL,					NULL,				0L },	// 50 TID_AppToBufAndFile.
	{ &msgQID_FileToBuf,	NULL,				0L },	// 51 TID_FileToBuf.
	{ &msgQID_BufToExt,		NULL,				0L },	// 52 TID_BufToExt.
	{ &msgQID_BufToBuf,		NULL,				0L },	// 53 TID_BufToBuf.
	{ &msgQID_BufToApp,		NULL,				0L },	// 54 TID_BufToApp.
	{ &msgQID_FileToApp,	NULL,				0L },	// 55 TID_FileToApp.
	{ &msgQID_AppToApp,		NULL,				0L },	// 56 TID_AppToApp.
	{ NULL,					NULL,				0L },	// 57 TID_ExtMemToExt.
// スレッド層　画像制御層（高速タスク）タスク.
	{ &msgQID_ClearImage,	NULL,				0L },	// 58 TID_ClearImage.
	{ &msgQID_JpegImage,	NULL,				0L },	// 59 TID_JpegImage.
	{ &msgQID_FaxCmpExp,	NULL,				0L },	// 60 TID_FaxCmpExp.
	{ &msgQID_ImageConv,	NULL,				0L },	// 61 TID_ImageConv.
// PIC　タスク.
	{ &msgQID_PIC_IF,		&wdID_PIC_IF,	    0L },	// 62 TID_PIC_IF,	 	// ＰＩＣ・インターフェース★★.
// タスク層の１ページシーケンスタスク.
	{ NULL,			NULL,						0L },	// 63 TID_AsicTool Asicツールタスク(JPEGツールから名称変更).
	{ &msgQID_XPanel_IF,	&wdID_XPanel_IF,    0L },	// 64 TID_XPanel_IF,  	// Rhein1P　外部パネル・インターフェース.
	{ &msgQID_HDDOneTrans,	NULL,			    0L },	// 65 TID_HDDOneTrans.
	{ &msgQID_ImgSynthesis,	NULL,				0L },	// 66 TID_ImageSynthesis.
#if (defined(COSIM) && defined(THR_SIMWIN)) || (defined(SIMULATOR) && defined(THR_SIMWIN)) // 【COSIM+ｽﾚｯﾄﾞSIMWIN対応】.
	// スレッドSimWinの仮想上位タスク.
	{ &msgQID_TestInputIR,	NULL,				0L },	// 67 TID_TestInputIR // 仮想　1ページIR入力シーケンス.
	{ &msgQID_TestInputExtIf,	NULL,			0L },	// 68 TID_TestInputExtIf // 仮想　1ページ外部IF入力シーケンス.
	{ &msgQID_TestInputApp,	NULL,				0L },	// 69 TID_TestInputApp // 仮想　1ページアプリ入力シーケンス.
	{ &msgQID_TestOutputEng,NULL,				0L },	// 70 TID_TestOutputEng	// 仮想　1ページエンジン出力シーケンス.
	{ &msgQID_TestOutputApp,NULL,				0L },	// 71 TID_TestOutputApp	// 仮想　1ページアプリ出力シーケンス.
	{ &msgQID_TestJpegTool,NULL,				0L },	// 72 TID_TestJpegTool	// 仮想　JPEGツールタスク.
#else
	{ NULL,					NULL,				0L },	// 67 TID_TestInputIR.
	{ NULL,					NULL,				0L },	// 68 TID_TestInputExtIf.
	{ NULL,					NULL,				0L },	// 69 TID_TestInputApp.
	{ NULL,					NULL,				0L },	// 70 TID_TestOutputEng.
	{ NULL,					NULL,				0L },	// 71 TID_TestOutputApp.
	{ NULL,					NULL,				0L },	// 72 TID_TestJpegTool.
#endif	// #ifdef SIMULATOR.
#if defined(DEF_EDH_COLOR)	//【EDH固有部】.
	// サブタスク（定期起床）.
	{ NULL,	 				NULL,	    		0L },	// --/66 TID_EDH_IF,     	// ＥＤＨ・インターフェース.
#endif		// #if defined(DEF_EDH_COLOR).
	{ &msgQID_PowerManager,NULL,				0L },	// 74.
	{ &msgQID_LEDStatus,NULL,					0L },	// 75 TID_LEDStatus.
	{ &msgQID_CancelDebug,NULL,					0L },	// 76 TID_CancelDebug.
	{ &msgQID_ThreadManager,NULL,				0L },	// 77 TID_ThreadManager.
	{ &msgQID_AesTrans,NULL,					0L },	// 78 TID_AesTrans.
	{ NULL, 		NULL,						0L },	// 79 TID_InputIRDup.
#if (defined(SIMULATOR) && defined(THR_SIMWIN)) // スレッドSimWin.
	{ &msgQID_TestInputIRDup,	NULL,			0L },	// 80 TID_TestInputIRDup // 仮想　1ページIR入力裏面シーケンス 2008/08/28 R.Suzuki CQ.309145 1Pass対応.
#else
	{ NULL,			NULL,						0L },
#endif
	{ NULL, 					NULL,			0L },	// 81 Dummy.
	{ &msgQID_AppInConvDiv, 	NULL,			0L },	//! 82 TID_AppInConvDiv.
	{ &msgQID_ThDebugLog, 		NULL,			0L },	// 83 TID_ThDebugLog CQ.361271.
	{ NULL, 		NULL,						0L },	//!< 84 TID_JpegImageAndroid.
	{ &msgQID_AndroidControl, NULL,				0L },	//!< 85 TID_AndroidControl.
	{ &msgQID_AndroidJpegControl, NULL,			0L },	//!< 86 TID_AndroidJpegControl.
	{ NULL,			 NULL,						0L },	//!< 87 TID_InInput.
	{ &msgQID_AsicComIPRomPara, NULL,			0L },	//!< 88 IPRomイベント受信タスク.
	{ &msgQID_SSDKeepData,		NULL,			0L },	//!< 89 SSDデータ消失防止 定期Readタスク.
	{ &msgQID_MultiTransData,	NULL,			0L },	//!< 90 マルチ画像転送タスク.
	{ &msgQID_AndroidConnection,NULL,			0L },	//!< 91 TID_AndroidConnection.
	{ &msgQID_InputImageConv,	NULL,			0L },	//!< 92 InputImgCnv.
#ifdef FUM_LINUX
	{ &msgQID_SimThread,        NULL,			0L },   //!< 92 TID_SimThread. HCL added 03-14-2013
#endif
	{ &msgQID_AesDmaMonitor,	NULL,			0L },	//!< 92 or 93 TID_AesDmaMonitor.
	{ &msgQID_S800jControl,		NULL,			0L },	//!< 92 or 93 or 94 TID_S800jControl.
	{ &msgQID_S800jControl0,	NULL,			0L },	//!< 93 or 94 or 95 TID_S800jControl0.
	{ &msgQID_S800jControl1,	NULL,			0L },	//!< 94 or 95 or 96 TID_S800jControl1.
	{ &msgQID_JPEGProxy0,		NULL,			0L },	//!< 95 or 96 or 97 TID_JPEGProxy0.
	{ &msgQID_JPEGProxy1,		NULL,			0L },	//!< 96 or 97 or 98 TID_JPEGProxy1.
	{ &msgQID_ThMeminfo,		NULL,			0L },	//!< 97 or 98 or 99 TID_Meminfo.
	{ &msgQID_TotalingNewDetail,NULL,			0L },	//!< 98 or 99 or 100 TID_TotalingNewDetail.
	{ NULL, 					NULL,			0L },	//!< 92 or 93 or 94 or 97 or 98 or 99 or 100 or 101 TID_MAX.
};
static Bool bl_SemphoreLock = False;		// セマフォのロック状態.
static Bool bl_Callback = False;			// コールバック状態.

// totalclear実行指定状態	[Poseidon量産問題]No7 TotalClearの実行有無判定の状態.
static uchar uc_ExecTotalClearState = THRE_ExecTotalClearState_NotReady;
//.

// MinervaSBK制御構成変更No.70 OEMデータのダウンロード対応.
static uchar uc_OemDataCallbackRet = THRD_CallbackUnknown;
///*================================================================================================.
//	外部参照関数の定義.
//================================================================================================*/.
//// ターミナルから呼ぶ物はここで記述する.
extern "C" {
void debugF(int32_t mode,int32_t on);
void IsExecTotalClear();
#if ( defined( DEF_A4_Cus ) && defined( DEF_SparrowCus ) )
//	2018/12/07	AR:1069261 eSOL 田川【Sparrow】【生産対応】ラインカード起動時、ADF非装着問題対応.
//	2019/11/22 Sky吉村(魁) OP_BTS-14959 制御構成変更No.52 SetMachineType_ToRam対応 引数の型を変更.
void SetMachineType_ToRam( THRE_MACHINEINFO e_MachineInfo, THRE_MACHINENO e_MachineNo );
//	2018/12/14	AR:1070278 eSOL 田川【Sparrow】【生産対応】主走査方向倍率パターン印刷用コマンド作成 新規作成.
void ExecADFMainZoomPatternPrint();
#endif //#if ( defined( DEF_A4_Cus ) && defined( DEF_SparrowCus ) ).

// 2019/03/07  AR:1076046 Sky【生産対応】SPI_Flash_DataRestoreダミーコマンド.
int SPI_Flash_DataRestore(void);
// 2019/05/31 Sky【生産対応】SPI_Flash_DataDeleteコマンド作成.
int SPI_Flash_DataDelete(void);
}
/*================================================================================================
	モジュール内関数の定義
================================================================================================*/
static void _wdTimerFunc( EV_TaskID myTaskID );							// 定期起床用処理関数.
static ushort THRL_RemainingDays( short ss_Year, char sc_Month, char sc_Day, char sc_Hour, char sc_Minute, char sc_Second );	// 証明書有効期限を返す.

// 2015/09/04	yamashita MinervaSBK制御構成変更No.70 OEMデータのダウンロード対応.
static void THRL_OemDataCallback( Bool bl_Status );

// 2019/11/22 Sky吉村(魁) OP_BTS-14959 制御構成変更No.52 SetMachineType_ToRam対応.
THRE_MACHINETYPESELECT_RESULT THRF_MachineTypeSelect(THRE_MACHINEINFO e_MachineInfo, THRE_MACHINENO e_MachineNo, TYPE_MachineType* pe_MachineType);

/*================================================================================================
	デバッグ用領域定義
================================================================================================*/
static Bool bl_DebugDetectTask[TID_MAX] = {	// 検出するタスクＩＤ.
	True , True , True , True , True , 		// 画像入力          / 画像伸張          / 画像出力          / 画像圧縮          / ＨＤＤデータ転送.
	True , True , True , True , True , 		// ワーク画像入力    / ワーク画像出力    / 原稿読み取り部    / ファイルメモリ登録/ ファイルメモリ読み出し.
	True , False, True , True , True , 		// 用紙プリント部    / パネル部          / ジョブ管理部      / マシン管理部      / 画像・メモリ管理部.
	True , True , True , True , True , True , True , 	// ＡＰＩ変換部      / ＡＰＩ ＩＦ部     / 特殊ジョブ制御部  / @外部I/F変換部    / ＩＲＣ・ＩＦ      / エンジン・ＩＦ / @外部I/F・ＩＦ.
	True , True , True , True , False,		// ＥＰＮｅｔ・ＩＦ  / EPNet Event       / EPNet DTBIOS      / ＭＩＮＥＴＡ・ＩＦ/ パネルＣＰＵ・ＩＦ.
	False, True , True , True , True ,		// パネルＬＣＤ描画  / フラッシュＲＯＭ  / EPNet Data        / EPNet DT          / EPNet ExIF.
	False, True , True , True , True , True,	// EPNet MIO1        / EPNet MIO2        / ソケット通信Client/ テスト用ドライバ  / ASICシミュレータ  / 評価用:ＦＴＰ転送.
	False
};
static struct ST_LogMessageTbl {
	uintptr_t	ul_Option;		// オプション.
	Bool	bl_IsEnrol;		// 登録状態.
	ushort	us_TaskMessage;	// メッセージ.
	uchar	uc_TaskID;		// タスクＩＤ.
} st_Tbl[100];
static short ss_TblINdex = 0;
Bool getDebugSequenceSendMessage( short ss_No, uchar& uc_TaskID, ushort& us_TaskMessage, uintptr_t& ul_Option );
Bool getDebugSequenceSendMessage( short ss_No, uchar& uc_TaskID, ushort& us_TaskMessage, uintptr_t& ul_Option )
{
	uc_TaskID		= st_Tbl[ss_No].uc_TaskID;
	us_TaskMessage	= st_Tbl[ss_No].us_TaskMessage;
	ul_Option		= st_Tbl[ss_No].ul_Option;
	return( st_Tbl[ss_No].bl_IsEnrol );
}
short getDebugSequenceIndexSendMessage();
short getDebugSequenceIndexSendMessage()
{
	return( ss_TblINdex );
}
void clearDebugSendMessage();
void clearDebugSendMessage()
{
	ss_TblINdex = 0;
	memset( &st_Tbl, 0x0, sizeof( st_Tbl ) );
}
void setDebugDetectTask( uchar uc_TaskID, Bool bl_Detect )
{
	if( /*0 <= uc_TaskID &&*/ uc_TaskID < TID_MAX ) {	// 個別指定.
		bl_DebugDetectTask[uc_TaskID] = bl_Detect;
	} else if( uc_TaskID == 99 ) {					// 全指定.
		for( register short i=0; i<TID_MAX; i++ ) {	bl_DebugDetectTask[i] = bl_Detect;	}
	}
}

static Bool bl_MessageQueOverflow = False;		// メッセージキューオーバーフロー発生.
static Bool bl_CF023_Occur = False;				// メッセージキューオーバーフロー発生(intr時).
static Bool bl_ExceptionOccur = False;			// 例外発生.

ushort us_debug_dismessage = 0;		// debufF 5 :Message Data Display Mode.
ushort us_debug_CancelTrace = 0;

// OP_BTS-40890_タスクモニタ新詳細データ化対応_S.
#define THRD_NEWDETAIL_TASKNAME_LENGTH		4
#define THRD_NEWDETAIL_TASKNAME_MAXLENGTH	16

struct st_ND_TaskNameCompress {
	uchar uc_Byte_One;		// 1Byte目.
	uchar uc_Byte_Two;		// 2Byte目.
	uchar uc_Byte_Three;	// 3Byte目.
	uchar uc_Byte_Four;		// 4Byte目.
};

// ゼロオリジンの為、資料から1引く.
static struct st_ND_TaskNameCompress st_ND_TaskNameCompressTbl[] = {
	{ 0,  0,   0,   0 },// タスク名長[ 1].
	{ 0,  0,   0,   0 },// タスク名長[ 2].
	{ 0,  0,   0,   0 },// タスク名長[ 3].
	{ 0,  0,   0,   0 },// タスク名長[ 4].
	{ 0,  2,   3,   4 },// タスク名長[ 5].
	{ 0,  3,   4,   5 },// タスク名長[ 6].
	{ 0,  4,   5,   6 },// タスク名長[ 7].
	{ 0,  5,   6,   7 },// タスク名長[ 8].
	{ 0,  5,   6,   8 },// タスク名長[ 9].
	{ 0,  5,   6,   9 },// タスク名長[10].
	{ 0,  5,   7,  10 },// タスク名長[11].
	{ 0,  5,   8,  11 },// タスク名長[12].
	{ 0,  5,   9,  12 },// タスク名長[13].
	{ 0,  5,  10,  13 },// タスク名長[14].
	{ 0,  5,  11,  14 },// タスク名長[15].
};
#define THRD_CPU_USE_RATE_BORDER 95
// OP_BTS-40890_タスクモニタ新詳細データ化対応_E.

// ================================================================================================.
//!［機能］タスク名取得関数.
// ------------------------------------------------------------------------------------------------.
// ［引数］.
//! \param ushort us_TaskId ... タスクのID.
// ［戻値］.
//! \return const char* タスク名へのポインタ.
// ［詳細］タスク名を取得する.
// ［履歴］2005/08/08 yokoi 新規作成.
//		   2008/11/07 R.Suzuki CQ.309145 1Pass対応 裏面用TaskIDの追加.
//		   2012/08/07 世古	CQ:370087 sc_Taskテーブルのタスク名称の並びをタスクIDであるEV_TaskID定義のものと一致するよう修正.
//		   2013/01/24 福原	CQ:377654 [スレッドリファイン]EV_TaskID定義のものと一致するよう修正(SSDKeepDataの追加).
//		   2015/11/24 Sky	システム統合SoC(S800)対応.
//		   2019/09/03 Sky井柳 【EagleZ+】ID不正値によるFA14(CFA14_DetailCode0x0626)を、不明時のタスク名設定へ変更.
//										（FA14発生によるログ出力中断の回避）.
// ================================================================================================.
const char* getTaskName( ushort us_TaskId )
{
	static char sc_Task[TID_MAX][20] = {
		"=Input=",           // 0.
		"=Expand=",          // 1.
		"=Output=",          // 2.
		"=Compress=",        // 3.
		"=HDDTrans=",        // 4.
		"=WorkInput=",       // 5.
		"=WorkOutput=",      // 6.
		"=Read=",            // 7.
		"=FileMemEntry=",    // 8.
		"=FileMemRead=",     // 9.
		"=Print=",           // 10.
		"=Panel=",           // 11.
		"=JobMan=",          // 12.
		"=MachineMan=",      // 13.
		"=Image=",           // 14.
		"=InitPIC=",         // 15.
		"=SaveMedia=",       // 16.
		"=SpJobCont=",       // 17.
		"=NTF_Convert=",     // 18.
		"=IRC_IF=",          // 19.
		"=Engine_IF=",       // 20.
#ifdef DEF_Soc1200
		"=MainToSubF=",      // 21  2013/07/12 ADS岡村 CQ.381171(Act_66426) SC-SFM-4 サブメモリのファイルメモリ化.
		"=SubFToMain=",      // 22  2013/07/12 ADS岡村 CQ.381171(Act_66426) SC-SFM-4 サブメモリのファイルメモリ化.
#else
		"=NTF_IF=",          // 21.
		"=EPNet_IF=",        // 22.
#endif	// DEF_Soc1200.
		"=EPNet_Event=",     // 23.
		"=EPNet_DTBIOS=",    // 24.
		"=MinetA_IF=",       // 25.
		"=PanelCPU=",        // 26.
		"=PanelDraw=",       // 27.
		"=ImagePattern=",    // 28.	// JimonPattern から ImagePattern に名称を変更. 2013/01/24 fukuhara.
		"=EPNet_Data=",      // 29.
		"=EPNet_DT=",        // 30.
		"=EPNet_ExIF=",      // 31.
		"=EPNet_MIO1=",      // 32.
		"=EPNet_MIO2=",      // 33.
		"=Client=",          // 34.
		"=TestDrv=",         // 35.
		"=TestAsic=",        // 36.
		"=InputIR=",         // 37.
		"=InputExIf=",       // 38.
		"=InputApp=",        // 39.
		"=OutputEng=",       // 40.
		"=OutputApp=",       // 41.
		"=OutputExIf=",      // 42.
		"=ExtToBuf=",        // 43.
		"=ExtToFile=",       // 44.
		"=ExtToBufAndFile=", // 45.
		"=BufToFile=",       // 46.
		"=IRToExtMem=",      // 47.
		"=AppToBuf=",        // 48.
		"=AppToFile=",       // 49.
		"=AppToBufAndFile=", // 50.
		"=FileToBuf=",       // 51.
		"=BufToExt=",        // 52.
		"=BufToBuf=",        // 53.
		"=BufToApp=",        // 54.
		"=FileToApp=",       // 55.
		"=AppToApp=",        // 56.
		"=ExtMemToExt=",     // 57.
		"=ClearImage=",      // 58.
		"=JpegImage=",       // 59.
		"=FaxCmpExp=",       // 60.
		"=ImageConv=",       // 61.
		"=PIC_IF=",          // 62.
		"=AsicTool=",        // 63.	// JpegTool から AsicTool に名称を変更. 2013/01/24 fukuhara.
		"=XPanel_IF=",       // 64.
		"=HDDOneTrans=",     // 65.
		"=ImageSynthesis=",  // 66.
		"=TestInputIR=",     // 67.
		"=TestInputExtIf=",  // 68.
		"=TestInputApp=",    // 69.
		"=TestOutputEng=",   // 70.
		"=TestOutputApp=",   // 71.
		"=TestJpegTool=",    // 72.
		"=EDH_IF=",          // 73.
		"=PowerManeger=",    // 74.
		"=LEDStatus=",       // 75.
		"=CancelDebug=",     // 76.
		"=ThreadManager=",   // 77.
		"=AesTrans=", 		 // 78.
		"=InputIRDup=",		 // 79.
		"=TestInputIRDup=",	 // 80.
		"",					 // 81 ソフトでのがたつき補正なしのときはID=81は空文字とする.
		"=AppInCovDiv=", 	 // 82.
		"=ThDebugLog=",      // 83.
		"=JpegImageAndroid=",// 84.
		"=AndroidControl=",  // 85.
		"=AndroidJpegCtrl=", // 86.
		"=InInpu=", 		 // 87.
		"=AsicComIPRomPara=",  // 88.
		"=SSDkeepData=",	 // 89.
		"=MultiTransData=",	 // 90.
		"=AndroidConnection=",	 // 91.
		"=InputImageConv=",	 // 92.
#ifdef FUM_LINUX
		"=SimThread=",		 // 93.
#endif // FUM_LINUX.
		"=AesDmaMonitor=",   // 94 or 93.
		"=S800jControl=",    // 95 or 94.
		"=JPEGTransport=",   // 96 or 95.
		"=JPEGConvCntl=",    // 97 or 96.
		"=JPEGConv0=",       // 98 or 97.
		"=JPEGConv1=",       // 99 or 98.
		"=Meminfo=",         // 100 or 99.
		"=TotalingNewDetail=",// 101 or 100.
	};
	static char sc_UnknownTaskName[20]={0};// 不明時のタスク名称保持領域.
	const char* psc_RetTaskName = NULL;
	if( us_TaskId >= TID_MAX ){ // ID不正(最大値以上).
		snprintf( sc_UnknownTaskName, sizeof(sc_UnknownTaskName), "=Unknown(%u)=", us_TaskId );
		psc_RetTaskName = sc_UnknownTaskName;
	}else{
		psc_RetTaskName = sc_Task[us_TaskId];
	}
	return psc_RetTaskName;
};
/*================================================================================================
［機能］	割り込み処理のLockクラス 
--------------------------------------------------------------------------------------------------
 2006/07/03 yokoi  新規作成 
 2006/12/09 yokoi  ISRからのクラス生成対策のため、lock/unlock処理を別API に分けた 
================================================================================================*/
#ifdef LINUX
SEM_ID THRC_IntLock::replaceSEM = NULL;
#else
int32_t THRC_IntLock::i_LockKey = 0;
#endif
uint32_t THRC_IntLock::ul_Counter = 0;
THRC_IntLock::THRC_IntLock()
{
	if( SYSF_intContext() == TRUE ){
		THRL_ThAssert( False, "::THRC_IntLock() ISR Error!\n" , 0, 0, 0, 0, CFA14_DetailCode0x0627 );
		return;
	}
	lock();
}

THRC_IntLock::~THRC_IntLock()
{
	if( SYSF_intContext() == TRUE ){
		THRL_ThAssert( False, "::~THRC_IntLock() ISR Error!\n" , 0, 0, 0, 0, CFA14_DetailCode0x0628 );
		return;
	}
	unlock();
}

void THRC_IntLock::lock()
{
	if( SYSF_intContext() == TRUE ){
		return;
	}

#ifdef LINUX
	if(replaceSEM == NULL) {
		replaceSEM = SYSF_semMCreate( SEM_Q_PRIORITY | SEM_DELETE_SAFE | SEM_INVERSION_SAFE );
	}
	SYSF_semTake(replaceSEM, WAIT_FOREVER);	// honda.
#else
	// 最初のロックなら、タスクと割り込み処理をロックする.
	if( ul_Counter == 0 ){
		SYSF_taskLock();
		i_LockKey = SYSF_intLock();
	}
#endif
	ul_Counter++;
}

void THRC_IntLock::unlock()
{
	if( SYSF_intContext() == TRUE ){
		return;
	}

	THRL_ThAssert( ul_Counter>0 , "THRC_IntLock::unlock() unlock over.\n" , 0, 0, 0, 0, CFA14_DetailCode0x0629 );	// 06/12/15 アンロックしすぎチェック 河村.
	ul_Counter--;

#ifdef LINUX
	SYSF_semGive(replaceSEM);
#else
	// 最後のアンロックなら、タスクと割り込み処理のロックを解除する.
	if( ul_Counter == 0 ){
		SYSF_intUnlock( i_LockKey );
		SYSF_taskUnlock();
	}
#endif
}
void THRC_IntLock::ForceUnlock()
{
#ifndef LINUX
	SYSF_intUnlock( i_LockKey );
	SYSF_taskUnlock();
#endif
}
uint32_t THRC_IntLock::getCounter()
{
	return (ul_Counter);
}

/*================================================================================================
［機能］	THRC_LocalSemaphore : コンストラクタ
--------------------------------------------------------------------------------------------------
［引数］	SEM_ID& semID		：使用するセマフォID指定
［戻値］	なし
［詳細］	クラス生成時にセマフォ獲得するクラス( セマフォIDを指定 )
			初期値は(SEM_ID)0で初期化しておくこと 
［ｳｫｰｸｽﾙｰ］	実施未  ：2007/06/14	Tsukiji
［履歴］	2007/06/14	Tsukiji
================================================================================================*/
THRC_LocalSemaphore::THRC_LocalSemaphore( SEM_ID& semID )
{
	// 2007/06/14	Tsukiji	蓄積復帰時のセマフォを作成.
	if( semID == (SEM_ID)0 ) {
		// セマフォ生成されていない場合、一番最初であればセマフォ作成.
		semID =  SYSF_semMCreate( SEM_Q_PRIORITY | SEM_DELETE_SAFE | SEM_INVERSION_SAFE ) ;
	}
	sem_ID_Bak = semID;		// デストラクタ用にローカルに保存する変数初期化.
	// セマフォID生成されているのを確認後、セマフォ取得.
	STATUS e_Status = SYSF_semTake( semID, WAIT_FOREVER);
	if( e_Status == ERROR ) {
		// 生成に失敗、及び獲得に失敗した場合エラー表示.
		THRL_ThLogMsgErr( "semTake ERROR tick(%"PRId32") SEMID=0x%08lX\n",SYSF_tickGet(),(uintptr_t)semID);
	}
}

/*================================================================================================
［機能］	THRC_LocalSemaphore : デストラクタ
--------------------------------------------------------------------------------------------------
［戻値］	なし
［詳細］	クラスが破棄される場合にコンストラクタ時のセマフォをGiveする
［ｳｫｰｸｽﾙｰ］	実施未  ：2007/06/14	Tsukiji
［履歴］	2007/06/14	Tsukiji
================================================================================================*/
THRC_LocalSemaphore::~THRC_LocalSemaphore()
{
	// セマフォID生成されているのを確認後、セマフォ解放.
	// コンストラクタ生成時のセマフォIDを使用.
	STATUS e_Status = SYSF_semGive( sem_ID_Bak );
	if( e_Status == ERROR ) {
		// 解放に失敗した場合エラー表示.
		THRL_ThLogMsgErr( "semGive ERROR tick(%"PRId32") SEMID=0x%08lX\n",SYSF_tickGet(),(uintptr_t)sem_ID_Bak );
	}
}


/*================================================================================================
［機能］	定期起床時間の設定
--------------------------------------------------------------------------------------------------
［引数］	EV_TaskID myTaskID   : 定期起床するタスクＩＤ
			int32_t	  sl_MSTimer : 定期起床時間（ｍＳ）
［戻値］	無し
［ｳｫｰｸｽﾙｰ］	実施済み：2000/8/17：出山
================================================================================================*/
void setWakeUpTimer( EV_TaskID myTaskID, int32_t sl_MSTimer )
{
#if		defined(TORNADO)||defined(__SIMWIN_MULTI_THREAD__)	// VXWORKS,SimWinマルチで有効.
	register int32_t sl_Tick = (int32_t)SYSF_sysClkRateGet() * sl_MSTimer / 1000;
	if( sl_Tick == 0L ) { sl_Tick = 1L; }
	st_Task[myTaskID].sl_Tick = sl_Tick;	// 定期起床時間をテーブルに設定.
	SYSF_wdStart( *st_Task[myTaskID].wdID, st_Task[myTaskID].sl_Tick, (FUNCPTR)_wdTimerFunc, myTaskID );
#else	// シミュレータで有効.
	m_hWndTam->SimSetWakeupTimer( myTaskID, sl_MSTimer );
#endif	// #if defined(TORNADO)||defined(__SIMWIN_MULTI_THREAD__).
}
/*================================================================================================
［機能］	一定時間後にメッセージ発行
--------------------------------------------------------------------------------------------------
［引数］	EV_TaskID	   ev_TaskID      : タスクＩＤ
			BE_TaskMessage be_TaskMessage : タスク間メッセージ
			long		   sl_MSTimer	  : 起床時間（ｍＳ）
［戻値］	無し
［ｳｫｰｸｽﾙｰ］	実施済み：2000/8/17：出山
［履歴］	03/12/27：morikawa Rhein1　DMA_Dの４色分のタイムアウトを起動するとオーバーフローが発生するので
										12から16に増やす
			06/03/30：yokoi    Mosel2  DMAが26個くらいあり、かつAsic117が2枚になる事があるので、
										HDDタイムアウト用なども含めて60個に増加。 
			15/10/22  大岡		[ZeusSZX0]制御構成変更No.76-1 新詳細コードデータ収集機能.
================================================================================================*/
// DelayMessageの数を変更.
WDOG_ID wdID_DelayMessage[THRD_DelayMsgNum];	// 一定時間後のメッセージ発行.
THRS_DelayMessage st_DelayMessage[THRD_DelayMsgNum];

static void procDelayMessage( int32_t param )	// 一定時間後のメッセージ発行関数（複数の関数があるので、引数で分ける）.
{
	if( st_DelayMessage[param].ev_TaskID != EV_TaskID(-1) ) {	// キャンセル対応.
		int_sendMessage( st_DelayMessage[param].ev_TaskID, st_DelayMessage[param].be_TaskMessage, (uintptr_t)st_DelayMessage[param].ul_Option );
		st_DelayMessage[param].ev_TaskID = EV_TaskID(-1);		// タスクＩＤを未登録にする.
	}
}
void setDelayMessage( EV_TaskID ev_TaskID, BE_TaskMessage be_TaskMessage, uintptr_t ul_Option, uintptr_t sl_MSTimer )
{
#if		defined(TORNADO)||defined(__SIMWIN_MULTI_THREAD__)	// VXWORKS,SimWinマルチで有効.
	for( register ushort i = 0; i < sizeof(st_DelayMessage)/sizeof(THRS_DelayMessage); i++ ) {
		if( st_DelayMessage[i].ev_TaskID == EV_TaskID(-1) ) {
			// 未登録テーブルが見つかった時.
			st_DelayMessage[i].ev_TaskID      = ev_TaskID;		// タスクＩＤを登録.
			st_DelayMessage[i].be_TaskMessage = be_TaskMessage;	// タスク間メッセージ.
			st_DelayMessage[i].ul_Option      = ul_Option;		// タスク間メッセージ.
			int32_t sl_Tick = (int32_t)SYSF_sysClkRateGet() * sl_MSTimer / 1000;		// チック数を計算.
			if( sl_Tick == 0L ) { sl_Tick = 1L; }
			SYSF_wdStart( *st_DelayMessage[i].wdID, (int32_t)sl_Tick, (FUNCPTR)procDelayMessage, (int32_t)i );	// ウォッチドッグタイマをコールする.
			return;
		}
	}
	// 新詳細コード保存.
	uint64_t		ull_NewDetailData = THRD_NewDetail_ModuleId_Thread;		// 1,2桁：スレッド.
	ull_NewDetailData |= (uint64_t)( CF021_DebugFatal ) << THRD_NewDetail_TroubleCode_Shift;	// 3-6桁：トラブルコード.
	ull_NewDetailData |= (uint64_t)( ev_TaskID ) << 32;					// 6,7桁：タスクID.
	ull_NewDetailData |= (uint64_t)( be_TaskMessage ) << 24;				// 8,9桁：タスク間メッセージ.
	THRL_ThAssertLog2( 0, ull_NewDetailData );
	// 最大１２個まで登録可能・・・登録できない場合はバグとする。.
	DebugForever( CF021_DebugFatal );
#else	// シミュレータで有効.
	m_hWndTam->SimSetDelayTimer( ev_TaskID, be_TaskMessage, ul_Option, sl_MSTimer );
#endif	// #if defined(TORNADO)||defined(__SIMWIN_MULTI_THREAD__).
}
// タイマキャンセル.
void cancelDelayMessage( BE_TaskMessage be_ChkTaskMessage, uintptr_t ul_Option /*= 0*/ )
{
#if		defined(TORNADO)||defined(__SIMWIN_MULTI_THREAD__)	// VXWORKS,SimWinマルチで有効.
	for( register ushort i = 0; i < sizeof(st_DelayMessage)/sizeof(THRS_DelayMessage); i++ ) {
		if( st_DelayMessage[i].ev_TaskID != EV_TaskID(-1) ) {
			// 登録テーブルが見つかった時.
			if( st_DelayMessage[i].be_TaskMessage == be_ChkTaskMessage && st_DelayMessage[i].ul_Option == ul_Option ) {
				// 登録済みのタスク間メッセージからＷＤＩＤを検索.
				SYSF_wdCancel( *st_DelayMessage[i].wdID );	// ウォッチドッグタイマをキャンセルする.
				st_DelayMessage[i].ev_TaskID = EV_TaskID(-1);	// タスクＩＤを未登録にする.
				break;
			}
		}
	}
#else	// シミュレータで有効.
	m_hWndTam->SimCancelDelayTimer( be_ChkTaskMessage, ul_Option );
#endif	// #if defined(TORNADO)||defined(__SIMWIN_MULTI_THREAD__).
}
// C-5023(F023)メッセージキューオーバーフロー検出時の情報保持＆出力.
// デバッグ用.
static struct {
	uintptr_t ul_Option ;
	BE_TaskMessage be_TaskMessage ;
	EV_TaskID sendTaskID ;
} st_SMOF ;

void setSMOFInfo( 
	EV_TaskID sendTaskID,
	BE_TaskMessage be_TaskMessage,
	uintptr_t ul_Option )
{
	if ( (bl_MessageQueOverflow == False) && (bl_CF023_Occur == False) ) {
		st_SMOF.sendTaskID		= sendTaskID ;
		st_SMOF.be_TaskMessage	= be_TaskMessage ;
		st_SMOF.ul_Option		= ul_Option ;
	}
}
// 【追加】自動評価対応 [start] 2008.08.11 鈴木希 自動評価システム環境のデバッグ、メッセージオーバーフローのカウント数取得.
#if defined( DEF_SimAutoTest )		// 自動評価で有効.
// デバッグ用.
static struct {
	short	ssCount;	// 100over Count.
	short	ssNum;		// Max QueueCount.
} st_DebugQueCount ;

void setDebugQueCount( 
	short ssCount,
	short ssNum
)
{
	st_DebugQueCount.ssCount	= ssCount;
	if( st_DebugQueCount.ssNum < ssNum )
	{
		st_DebugQueCount.ssNum		= ssNum;
	}
}
#endif	// #if defined( DEF_SimAutoTest )		// 自動評価で有効.
// 【追加】自動評価対応 [ end ] 2008.08.11 鈴木希 自動評価システム環境のデバッグ、メッセージオーバーフローのカウント数取得.
extern "C" {
	void debugsmof() ;
// 【追加】自動評価対応 [start] 2008.08.11 鈴木希 自動評価システム環境のデバッグ、メッセージオーバーフローのカウント数取得.
#if defined( DEF_SimAutoTest )		// 自動評価で有効.
	void debugQueCount() ;
#endif	// #if defined( DEF_SimAutoTest )		// 自動評価で有効.
// 【追加】自動評価対応 [ end ] 2008.08.11 鈴木希 自動評価システム環境のデバッグ、メッセージオーバーフローのカウント数取得.
}
void debugsmof()
{
	printf("Message Queue Over Flow Infomation.\n") ;
	if ( (bl_MessageQueOverflow == False) && (bl_CF023_Occur == False) ) {
		printf("  Nothing.\n") ;
	} else {
		printf("  TaskID = [%d]\n  MessageID = [%d]\n  Option = [%"PRIdPTR"]\n", (ushort)st_SMOF.sendTaskID, st_SMOF.be_TaskMessage, st_SMOF.ul_Option ) ;
	}
}
// 【追加】自動評価対応 [start] 2008.08.11 鈴木希 自動評価システム環境のデバッグ、メッセージオーバーフローのカウント数取得.
#if defined( DEF_SimAutoTest )		// 自動評価で有効.
void debugQueCount()
{
	printf("Message Queue Over Flow Count.\n") ;
	printf("  (100over)Count = [%d] Max_Num = [%d]\n"
			, st_DebugQueCount.ssCount, st_DebugQueCount.ssNum ) ;
}
#endif	// #if defined( DEF_SimAutoTest )		// 自動評価で有効.
// 【追加】自動評価対応 [ end ] 2008.08.11 鈴木希 自動評価システム環境のデバッグ、メッセージオーバーフローのカウント数取得.


// 2020/6/5 伊藤史. OP_BTS-24114 ログ改善.
uint32_t ui32_GetCpuUtilization = 0;
void dbg_SetThDebugLogQueLimit( int dt )
{
	ui32_GetCpuUtilization = static_cast<uint32_t>(dt);
}

/*================================================================================================
［機能］	タスク間メッセージ送信
--------------------------------------------------------------------------------------------------
［引数］	EV_TaskID	   sendTaskID     : 送信先のタスクＩＤ
			BE_TaskMessage be_TaskMessage : 送信メッセージ
			uintptr_t	   ul_Option      : 送信オプション
［戻値］	無し
［ｳｫｰｸｽﾙｰ］	実施済み：2000/8/17：出山
［履歴］	2002/02/07：morikawa	メッセージＱの上限オーバープロテクト追加
［履歴］	2002/02/07：中田		メッセージＱの上限オーバープロテクト追加
			2002/02/14：morikawa	debug文追加
			2004/12/20：松田		CNVC_LocalCompressorImpColorとCNVC_LocalCompressorImpColorEx共存対応(外部IF)
			2008/08/18：R.Suzuki	CQ.309145 1Pass対応 TID_InputIRDupを追加.
			2010/02/24：M.Koshimo	CQ.324008 右ドア開けるとパネルロック対応.
			2011/11/29：越野		CQ.359261 アクセラレータFiery1200dpi対応.
			2013/03/26：Sky			ZeusS対応 Fiery 1200dpi JBIG圧縮データ入力の処理を削除.
			2013/06/03：鈴木		静的解析対応.
			2013/09/09：岡部(日立)  静的解析対応.
			2015/10/22：大岡		[ZeusSZX0]制御構成変更No.76-1 新詳細コードデータ収集機能.
			2015/10/29：sky古市		CQ.520849 メッセージキュー最大の場合にログ領域解放処理を追加.
================================================================================================*/
void sendMessage( EV_TaskID sendTaskID, BE_TaskMessage be_TaskMessage, uintptr_t ul_Option )
{
    // HCL Warning Fix - Starts.
    // Warning No : 34989.
    THRL_ThAssert( (sendTaskID < TID_MAX ), "sendMessage QID error %d", sendTaskID , 0, 0, 0, CFA14_DetailCode0x0CCC );
    // HCL Warning Fix - Ends.

// STBD_StubEnable.
#ifdef STBD_StubEnable
	STBC_Thread		*pc_ThreadStub;
	pc_ThreadStub = STBC_Thread::newInstance();
	if ( FALSE != pc_ThreadStub->sendMessage( sendTaskID, be_TaskMessage, (uintptr_t)ul_Option )) {
		return;
	}
#endif // STBD_StubEnable.

	static Bool	bl_GetCpuUtilization = False;

	if( 0 < us_debug_dismessage ){
		printf("[%"PRId32"] sendMessage TID(0x%"PRIxPTR") sendTaskID(%d) be_TaskMessage(%"PRId32")\n",SYSF_tickGet(),SYSF_taskIdSelf(),sendTaskID,(uint32_t)be_TaskMessage);
	}

	if( b_DebugCancelTestToolFlg==1 ){
		if ( 0 != THRL_CancelDebug_sendMessage( &sendTaskID, be_TaskMessage, (uintptr_t)ul_Option )) {
			// メッセージ送信を遅延させる場合は処理を抜ける.
			return;
		}
	}
	if( (0 < us_debug_CancelTrace) && (TM_ThCancelEnd == be_TaskMessage) ){
		printf("[%"PRId32"] sendMessage  TID(0x%"PRIxPTR") sendTaskID default(%d) be_TaskMessage(%"PRId32")\n",SYSF_tickGet(),SYSF_taskIdSelf(),sendTaskID,(uint32_t)be_TaskMessage);
	}

	// 2017/02/20 Sky酒井（純）ユーザーイベント記録(APIコール).
	SYSF_ReqTaskMonitorUserEvent(18004, (uint32_t)sendTaskID, (uint32_t)be_TaskMessage, (uint32_t)ul_Option);

	// 省電力復帰シーケンス測定.
	if ( (TID_PowerManeger ==sendTaskID ) && ( TM_AnsWakeUpComplete == be_TaskMessage ) ){
		printf("sendMessage( TID_PowerManeger, TM_AnsWakeUpComplete, %d ) [Tick:%lu]\n", (int)ul_Option, SYSF_tickGet());
	}
#if defined(SIMULATOR) || (defined(COSIM) && defined(THR_SIMWIN))
	// 上位へのメッセージ送信の場合、ログを出力する.
	ThOutputLogSendMessage( sendTaskID, be_TaskMessage, ul_Option );
#endif
#if defined(COSIM) && defined(THR_SIMWIN) // 【COSIM+ｽﾚｯﾄﾞSIMWIN対応】.
#else
	ENGC_PrinterImpColor*           pc_PrinterImpColor           = ENGC_PrinterImpColor::newInstance();				// プリンタデバイス（カラー機依存）.
	ENGC_ScannerImpColor*           pc_ScannerImpColor           = ENGC_ScannerImpColor::newInstance();				// スキャナデバイス（カラー機依存）.

	// 2004.12.20 change matsuda CNVC_LocalCompressorImpColorBandとCNVC_LocalCompressorImpColorPage共存対応(外部IF).
	CNVC_LocalCompressorImp*		pc_LocalCompressorImp = NULL;
	CNVC_LocalAsicToolImp*			pc_LocalAsicToolImpColor = CNVC_LocalAsicToolImp::newInstance();
	CNVC_LocalDecompressorImp*		pc_LocalDecompressorImpColor = CNVC_LocalDecompressorImp::newInstance();	//.
//#ifdef TORNADO // SimWinビルド対応.
#if 1 //HCL (Durai) Based on problem no.8 fixed - Based on ->2013/07/15 KM(Morikawa) denbug SimWin
	PWRC_PowerImpColor* 			pc_PowerImpColor			 = PWRC_PowerImpColor::newInstance();				// パワー（カラー機種依存部）// 04/01/27 小林追加.
#endif // TORNADO SimWinビルド対応.
#endif
	switch( sendTaskID )
	{
#if defined(COSIM) && defined(THR_SIMWIN) // 【COSIM+ｽﾚｯﾄﾞSIMWIN対応】.
		case TID_InputIR:			break;	// 1ページIR入力シーケンス.
		case TID_InputExtIf:		break;	// 1ページ外部IF入力シーケンス.
		case TID_InputApp:			break;	// 1ページアプリ入力シーケンス.
		case TID_AsicTool:			break;	// Asicツール(JPEGツールから名称変更).
		case TID_OutputEng:			break;	// 1ページエンジン出力シーケンス.
		case TID_OutputApp:			break;	// 1ページアプリ出力シーケンス.
		case TID_MachineMan:		break;	// パワー（カラー機種依存部）// 04/01/27 小林追加.
#else
		case TID_InputIR:			pc_ScannerImpColor          ->sendMessageFromThr( be_TaskMessage, ul_Option );		break;	// 1ページIR入力シーケンス.
		case TID_InputIRDup:				// 1ページIR入力シーケンス(裏面).
			pc_ScannerImpColor->sendMessageFromThr( be_TaskMessage, ul_Option );
			break;
		// 2004.12.20 change matsuda CNVC_LocalCompressorImpColorとCNVC_LocalCompressorImpColorEx共存対応(外部IF).
		case TID_InputExtIf:	
			pc_LocalCompressorImp = CNVC_LocalCompressorImpColorPage::_newInstance();
			((CNVC_LocalCompressorImpColorPage*)pc_LocalCompressorImp)->sendMessageFromThr( be_TaskMessage, ul_Option );
			break;

		// 2004.12.20 change matsuda CNVC_LocalCompressorImpColorとCNVC_LocalCompressorImpColorEx共存対応(外部IF).
		case TID_InputApp:			pc_LocalCompressorImp = CNVC_LocalCompressorImpColorBand::_newInstance();			((CNVC_LocalCompressorImpColorBand*)pc_LocalCompressorImp)->sendMessageFromThr( be_TaskMessage, ul_Option );		break;	// 1ページアプリ入力シーケンス.
		case TID_AsicTool:			pc_LocalAsicToolImpColor->sendMessageFromThr( be_TaskMessage, ul_Option );			break;	
#if defined(DEF_TaigaCus) || defined(DEF_DonauBKCus) // 2011/02/22 ft_kondo 第6回手動マージ. 
		case TID_OutputEng:
			// CQ.324008 FC21発生時にはPrintPに画像転送完了通知を行わない.
			if(( us_waitDebugForeverCode == 0xFC21 ) && ( be_TaskMessage == TM_ImageTransEnd )){
				return;
			}
			pc_PrinterImpColor->sendMessageFromThr( be_TaskMessage, ul_Option );
			break;	// 1ページエンジン出力シーケンス.
#else
		case TID_OutputEng:			pc_PrinterImpColor->sendMessageFromThr( be_TaskMessage, ul_Option );				break;	// 1ページエンジン出力シーケンス.
#endif
		case TID_OutputApp:			pc_LocalDecompressorImpColor->sendMessageFromThr( be_TaskMessage, ul_Option );		break;	// 1ページアプリ出力シーケンス.
#if 1 //HCL (Durai) Based on problem no.8 fixed - Based on ->2013/07/15 KM(Morikawa) denbug SimWin
//#ifdef TORNADO // SimWinビルド対応.
		case TID_MachineMan:		pc_PowerImpColor->sendMessageFromThr( be_TaskMessage, ul_Option ); 					break;	// パワー（カラー機種依存部）// 04/01/27 小林追加.
#endif // TORNADO SimWinビルド対応.
#endif
#ifdef THR_SIMWIN
		case TID_IRC_IF:
			// 上位が動かないようにするため、メッセージを送信しない.
			break;
#else
		case TID_IRC_IF:			pc_ScannerImpColor->sendMessageTotaskIRCIF( be_TaskMessage, ul_Option );			break;	// ＩＲＣ・インターフェース.
#endif
		case TID_OutputExtIf:
#ifdef TORNADO	
			SYSF_logMsg("sendMessage Error ! TID_OutputExtIf\n",0,0,0,0,0,0);
#endif
			break;
		case TID_Read:
#ifdef TORNADO	
			SYSF_logMsg("sendMessage Error ! TID_Read\n",0,0,0,0,0,0);
#endif
			break;
		case TID_FileMemEntry:
#ifdef TORNADO	
			SYSF_logMsg("sendMessage Error ! TID_FileMemEntry\n",0,0,0,0,0,0);
#endif
			break;
		case TID_FileMemRead:
#ifdef TORNADO	
			SYSF_logMsg("sendMessage Error ! TID_FileMemRead\n",0,0,0,0,0,0);
#endif
			break;
		case TID_Print:
#ifdef TORNADO	
			SYSF_logMsg("sendMessage Error ! TID_Print\n",0,0,0,0,0,0);
#endif
			break;
		case TID_JobMan:
#ifdef TORNADO	
			SYSF_logMsg("sendMessage Error ! TID_JobMan\n",0,0,0,0,0,0);
#endif
			break;
		case TID_Engine_IF:
#ifdef TORNADO	
			SYSF_logMsg("sendMessage Error ! TID_Engine_IF\n",0,0,0,0,0,0);
#endif
			break;
		case TID_EPNet_Event:
#ifdef TORNADO	
			SYSF_logMsg("sendMessage Error ! TID_EPNet_Event\n",0,0,0,0,0,0);
#endif
			break;
#if defined(DEF_EDH_COLOR)	//【EDH固有部】【Rhein2マージ撲滅】05/06/06 Div.12 k-kawa.
		case TID_EDH_IF:
#ifdef TORNADO	
			SYSF_logMsg("sendMessage Error ! TID_EDH_IF\n",0,0,0,0,0,0);
#endif
			break;
#endif		// #if defined(DEF_EDH_COLOR).
		default:
			if( 0 < us_debug_dismessage ){
				printf("[%"PRId32"] sendMessage  TID(0x%"PRIxPTR") sendTaskID default(%d) be_TaskMessage(%"PRId32")\n",SYSF_tickGet(),SYSF_taskIdSelf(),sendTaskID,(uint32_t)be_TaskMessage);
			}

#if		defined(TORNADO)||defined(__SIMWIN_MULTI_THREAD__)	// VXWORKS,SimWinマルチで有効.
// 【2001/12/20 : RXXX : Natsume : taskSpawn()していないのにメッセージ送信はダメ】.
			if( sendTaskID == TID_EPNet_Event ) { return; }
			uintptr_t messageBuf[2];
			messageBuf[0] = (uintptr_t)be_TaskMessage;
			messageBuf[1] = ul_Option;
			THRL_ThAssert( (st_Task[sendTaskID].msgQID != NULL), "sendMessage QID error %d", sendTaskID , 0, 0, 0, CFA14_DetailCode0x062A );
// 【追加】自動評価対応 [start] 2008.08.11 鈴木希 メッセージオーバーを100count毎に掃き出すログを出力する.
#if defined( DEF_SimAutoTest )		// 自動評価で有効.
			CNVC_PanelHardIf* pc_PanelHard = CNVC_PanelHardIf::newInstance();
			if( pc_PanelHard->getSimAutoTestConnected() == True
			 && SYSF_msgQNumMsgs( *st_Task[sendTaskID].msgQID ) % 100 == 0 && SYSF_msgQNumMsgs( *st_Task[sendTaskID].msgQID ) != 0 ) {
#ifdef TORNADO	
				SYSF_logMsg("[OverMsgQue!!] Num=[%d] T_ID=[%d]\n",SYSF_msgQNumMsgs( *st_Task[sendTaskID].msgQID ),sendTaskID,0,0,0,0);
#endif
			}
#endif	// #if defined( DEF_SimAutoTest )		// 自動評価で有効.
// 【追加】自動評価対応 [ end ] 2008.08.11 鈴木希 メッセージオーバーを100count毎に掃き出すログを出力する.
// 【変更】自動評価対応 [start] 2008.08.11 鈴木希 自動評価システム環境では、メッセージキューのメッセージ数を増加する.
#if defined( DEF_SimAutoTest )		// 自動評価で有効.
			if( ( pc_PanelHard->getSimAutoTestConnected() == True  && SYSF_msgQNumMsgs( *st_Task[sendTaskID].msgQID ) >= MAX_MSGS_AUTOTEST )
			 || ( pc_PanelHard->getSimAutoTestConnected() == False && SYSF_msgQNumMsgs( *st_Task[sendTaskID].msgQID ) >= MAX_MSGS ) ) {
#else	// #if defined( DEF_SimAutoTest )		// 自動評価で有効.
			if( SYSF_msgQNumMsgs( *st_Task[sendTaskID].msgQID ) >= MAX_MSGS) {
#endif	// #if defined( DEF_SimAutoTest )		// 自動評価で有効.
// 【変更】自動評価対応 [ end ] 2008.08.11 鈴木希 自動評価システム環境では、メッセージキューのメッセージ数を増加する.
				// デバッグログ収集システムの場合はエラーとしない。
				// この場合でもデバッグログに残らないだけなので動作に影響は出ない為。
				if( sendTaskID == TID_ThDebugLog ){
					static int32_t sl_QueMaxTick = 0;
					static uint32_t ul_QueMaxCnt = 0;
					Bool bl_QueMax = False;
					uint32_t sl_tick = (uint32_t)SYSF_tickGet();

					if( (0 != sl_QueMaxTick) && ( 2000 < (sl_tick - sl_QueMaxTick)) ){
						// 初回以外で 10s 間隔が空いたらリセット.
						ul_QueMaxCnt = 0;
					}
					sl_QueMaxTick = sl_tick;
					ul_QueMaxCnt++;
					if( 11 > ul_QueMaxCnt ){
						bl_QueMax = True;
					} else if( 101 > ul_QueMaxCnt ){
						if( 0 == (ul_QueMaxCnt % 10) ){
							bl_QueMax = True;
						}
					} else if( 1001 > ul_QueMaxCnt ){
						if( 0 == (ul_QueMaxCnt % 100) ){
							bl_QueMax = True;
						}
					} else if( 10001 > ul_QueMaxCnt ){
						if( 0 == (ul_QueMaxCnt % 1000) ){
							bl_QueMax = True;
						}
					} else {
						if( 0 == (ul_QueMaxCnt % 10000) ){
							bl_QueMax = True;
						}
					}

					if( True == bl_QueMax ){
// 【変更】2016.01.12 鈴木希 CQ523306 [start] QueueFullとなった時に垂れ流しログが埋まってしまう為、出力数を制限する. 
//						if( 10000 <= ul_QueMaxCnt ){
//							printf( "[%ld]TID_ThDebugLog Que Max!! (%ld)\n",SYSF_tickGet(),ul_QueMaxCnt );
//						} else {
						if( ul_QueMaxCnt < 100 ) {
							printf( "TID_ThDebugLog Que Max!! (%"PRId32")\n",ul_QueMaxCnt );
						}
						// 2020/6/5 伊藤史. OP_BTS-24114 ログ改善.
						if( 0 < ui32_GetCpuUtilization ){
							if( (ui32_GetCpuUtilization <= ul_QueMaxCnt) && (False == bl_GetCpuUtilization) ){
								bl_GetCpuUtilization = True;
								THRF_GetCpuUtilization( THRD_GetCpuUtlzID_pidstat, -10 );
							}
						}
					}
					return;
				}

				setSMOFInfo( sendTaskID, be_TaskMessage, ul_Option ) ;		// 最初の検出を残す 2004/11/15 added by t.sasaki.
				SYSF_logMsg( "SMOF!! -> TaskID %d Message %d Option %ld\n",sendTaskID, be_TaskMessage, ul_Option ,0,0,0);
				bl_MessageQueOverflow = True;		// メッセージキューオーバーフロー発生.
				debugF( 100, 0 );
				// 新詳細コード保存.
				uint64_t		ull_NewDetailData = THRD_NewDetail_ModuleId_Thread;							// 1,2桁：スレッド.
				ull_NewDetailData |= (uint64_t)( CF023_DebugFatal ) << THRD_NewDetail_TroubleCode_Shift;	// 3-6桁：トラブルコード.
				ull_NewDetailData |= (uint64_t)( SYSF_msgQNumMsgs( *st_Task[sendTaskID].msgQID ) ) << 8;	// 7-14桁：メッセージ数.
				THRL_ThAssertLog2( 0, ull_NewDetailData );
				DebugForever( CF023_DebugFatal );		// MsgQue OverFllow.
			}
			if ((TID_Image		== sendTaskID && TM_TimerWakeUp_Sys == be_TaskMessage)
			 || (TID_TestAsic	== sendTaskID && TM_TimerWakeUp_Sys == be_TaskMessage))
			{
#ifndef COSIM
				SYSF_msgQSend( *st_Task[sendTaskID].msgQID, (char*)messageBuf, sizeof(messageBuf), NO_WAIT, MSG_PRI_NORMAL );
#else
				STATUS status = SYSF_msgQSend( *st_Task[sendTaskID].msgQID
										, (char*)messageBuf
										, sizeof(messageBuf)
										, NO_WAIT, MSG_PRI_NORMAL );
				if( status == ERROR ){
					extern void tornadoErrorOccur( const char string[] );
					tornadoErrorOccur( "### THRC_msgQSend Error" );
					THRL_ThLogMsgErr( "TaskId %d, msgId %d\n", sendTaskID, be_TaskMessage );
				}
#endif
			}
			else
			{
#ifndef COSIM
				if( 0 < us_debug_dismessage ){
					printf("[%"PRId32"] sendMessage  TID(0x%"PRIxPTR") SYSF_msgQSend sendTaskID(%d) be_TaskMessage(%"PRId32")\n",SYSF_tickGet(),SYSF_taskIdSelf(),sendTaskID,(uint32_t)be_TaskMessage);
				}
				SYSF_msgQSend( *st_Task[sendTaskID].msgQID, (char*)messageBuf, sizeof(messageBuf), WAIT_FOREVER, MSG_PRI_NORMAL );
				if( 0 < us_debug_dismessage ){
					printf("[%"PRId32"] sendMessage  TID(0x%"PRIxPTR") SYSF_msgQSend Done sendTaskID(%d) be_TaskMessage(%"PRId32")\n",SYSF_tickGet(),SYSF_taskIdSelf(),sendTaskID,(uint32_t)be_TaskMessage);
				}
#else
				STATUS status = SYSF_msgQSend( *st_Task[sendTaskID].msgQID
										, (char*)messageBuf
										, sizeof(messageBuf)
										, WAIT_FOREVER
										, MSG_PRI_NORMAL );
				if( status == ERROR ){
					extern void tornadoErrorOccur( const char string[] );
					tornadoErrorOccur( "### THRC_msgQSend Error" );
				}
#endif
			}
#else	// シミュレータで有効.
			m_hWndTam->SimSendTaskMessage( sendTaskID, be_TaskMessage, ul_Option );
			if( sendTaskID == TID_Panel ) {
				TRACE( "TID_Panel <-- %d : %d\n", be_TaskMessage, ul_Option );
			}
#endif	// #if defined(TORNADO)||defined(__SIMWIN_MULTI_THREAD__).
			break;
	}
}
/*================================================================================================
［機能］	タスク間割り込みメッセージ送信
--------------------------------------------------------------------------------------------------
［引数］	EV_TaskID	   sendTaskID     : 送信先のタスクＩＤ
			BE_TaskMessage be_TaskMessage : 送信メッセージ
			uintptr_t	   ul_Option      : 送信オプション
［戻値］	Bool : True = 正常送信 / False = 送信エラー
［ｳｫｰｸｽﾙｰ］	実施済み：2000/8/17：出山
［履歴］	2004/11/11：t.sasaki	TID_MachineMan対応(カラー固有電源管理タスクへの送信とする).
			2004/11/15：t.sasaki	bl_CF023_Occurをファイルスタティックに変更.
			2008/06/04  yagi	  CQ304690 割り込み処理内でのインスタンス生成禁止(Amur画出し対応).
			2008/08/18：R.Suzuki	CQ.309145 1Pass対応 TID_InputIRDupを追加.
			2013/06/03：鈴木		静的解析対応.
			2015/02/19：yamada		CQ508109 : 調査用ログ出力対応.
			2015/10/22：大岡		[ZeusSZX0]制御構成変更No.76-1 新詳細コードデータ収集機能.
================================================================================================*/
void int_sendMessage( EV_TaskID sendTaskID, BE_TaskMessage be_TaskMessage, uintptr_t ul_Option )
{
	THRL_ThAssert( sendTaskID < TID_MAX	 , "sendTaskID error. sendTaskID=%d be_TaskMessage=%d ul_Option=%d", sendTaskID , be_TaskMessage, ul_Option, 0, CFA14_DetailCode0x0BBC );
	if( b_DebugCancelTestToolFlg==1 ){
		if ( 0 != THRL_CancelDebug_sendMessage( &sendTaskID, be_TaskMessage, (uintptr_t)ul_Option )) {
			// メッセージ送信を遅延させる場合は処理を抜ける.
			return;
		}
	}

	// 2017/02/20 Sky酒井（純）ユーザーイベント記録(APIコール).
	SYSF_ReqTaskMonitorUserEvent(18005, (uint32_t)sendTaskID, (uint32_t)be_TaskMessage, (uint32_t)ul_Option);
	
#if defined(SIMULATOR) || (defined(COSIM) && defined(THR_SIMWIN))
	// 上位へのメッセージ送信の場合、ログを出力する.
	ThOutputLogSendMessage( sendTaskID, be_TaskMessage, ul_Option );
#endif
#if defined(COSIM) && defined(THR_SIMWIN) // 【COSIM+ｽﾚｯﾄﾞSIMWIN対応】.
#else
	ENGC_ScannerImpColor*           pc_ScannerImpColor           = ENGC_ScannerImpColor::newInstance();				// スキャナデバイス（カラー機依存）.
#endif
	switch( sendTaskID )
	{
#if defined(COSIM) && defined(THR_SIMWIN) // 【COSIM+ｽﾚｯﾄﾞSIMWIN対応】.
		case TID_InputIR:			break;	// 1ページIR入力シーケンス.
		case TID_InputExtIf:		break;	// 1ページ外部IF入力シーケンス.
		case TID_InputApp:			break;	// 1ページアプリ入力シーケンス.
		case TID_OutputEng:			break;	// 1ページエンジン出力シーケンス.
		case TID_OutputApp:			break;	// 1ページアプリ出力シーケンス.
		case TID_MachineMan:		break;	// パワー（カラー機種依存部）.
		case TID_IRC_IF:			break;	// ＩＲＣ・インターフェース.
#else
		case TID_InputIR:					// 1ページIR入力シーケンス.
		case TID_InputIRDup:				// 1ページIR入力シーケンス(裏面).
		case TID_InputExtIf:				// 1ページ外部IF入力シーケンス.
		case TID_InputApp:					// 1ページアプリ入力シーケンス.
		case TID_OutputEng:					// 1ページエンジン出力シーケンス.
		case TID_OutputApp:					// 1ページアプリ出力シーケンス.
		case TID_MachineMan:				// パワー（カラー機種依存部）.
			// 上位タスクに対するメッセージは FA16 を出す。.
			THRL_ThUnknownPara( False, "int_sendMessage TID error %d", sendTaskID , 0, 0, 0, CFA14_DetailCode0x062B );
			break;
		case TID_IRC_IF:			pc_ScannerImpColor->sendMessageTotaskIRCIF( be_TaskMessage, ul_Option );			// ＩＲＣ・インターフェース 
			break;
#endif

		default:
#if		defined(TORNADO)||defined(__SIMWIN_MULTI_THREAD__)	// VXWORKS,SimWinマルチで有効.
			uintptr_t messageBuf[2];
			messageBuf[0] = be_TaskMessage;
			messageBuf[1] = ul_Option;
			THRL_ThAssert( (st_Task[sendTaskID].msgQID != NULL), "int_sendMessage QID error %d", sendTaskID , 0, 0, 0, CFA14_DetailCode0x062C );
// 【追加】自動評価対応 [start] 2008.08.11 鈴木希 自動評価システム環境のデバッグ、メッセージオーバーフローのカウント数取得.
#if defined( DEF_SimAutoTest )		// 自動評価で有効.
			CNVC_PanelHardIf* pc_PanelHard = CNVC_PanelHardIf::newInstance();
static uint32_t sl_QueNgCount = 0;
			if( pc_PanelHard->getSimAutoTestConnected() == True
			 && SYSF_msgQNumMsgs( *st_Task[sendTaskID].msgQID ) % 100 == 0 && SYSF_msgQNumMsgs( *st_Task[sendTaskID].msgQID ) != 0 ) {
#ifdef TORNADO	
				setDebugQueCount( ++sl_QueNgCount, SYSF_msgQNumMsgs( *st_Task[sendTaskID].msgQID ) );
#endif
			}
#endif	// #if defined( DEF_SimAutoTest )		// 自動評価で有効.
// 【追加】自動評価対応 [ end ] 2008.08.11 鈴木希 自動評価システム環境のデバッグ、メッセージオーバーフローのカウント数取得.
// 【変更】自動評価対応 [start] 2008.08.11 鈴木希 自動評価システム環境では、メッセージキューのメッセージ数を増加する.
#if defined( DEF_SimAutoTest )		// 自動評価で有効.
			if( ( pc_PanelHard->getSimAutoTestConnected() == True  && SYSF_msgQNumMsgs( *st_Task[sendTaskID].msgQID ) >= MAX_MSGS_AUTOTEST )
			 || ( pc_PanelHard->getSimAutoTestConnected() == False && SYSF_msgQNumMsgs( *st_Task[sendTaskID].msgQID ) >= MAX_MSGS ) ) {
#else	// #if defined( DEF_SimAutoTest )		// 自動評価で有効.
			if( SYSF_msgQNumMsgs( *st_Task[sendTaskID].msgQID ) >= MAX_MSGS) {
#endif	// #if defined( DEF_SimAutoTest )		// 自動評価で有効.
// 【変更】自動評価対応 [ end ] 2008.08.11 鈴木希 自動評価システム環境では、メッセージキューのメッセージ数を増加する.
				if( bl_CF023_Occur == False ) {
					setSMOFInfo( sendTaskID, be_TaskMessage, ul_Option ) ;		// 最初の検出を残す 2004/11/15 added by t.sasaki.
					bl_CF023_Occur = True;	// １回のみログに出すようにする.
					// 新詳細コード保存.
					uint64_t		ull_NewDetailData = THRD_NewDetail_ModuleId_Thread;							// 1,2桁：スレッド.
					ull_NewDetailData |= (uint64_t)( CF023_DebugFatal ) << THRD_NewDetail_TroubleCode_Shift;	// 3-6桁：トラブルコード.
					ull_NewDetailData |= (uint64_t)( SYSF_msgQNumMsgs( *st_Task[sendTaskID].msgQID ) ) << 8;	// 7-14桁：メッセージ数.
					THRL_ThAssertLog2( 0, ull_NewDetailData );
					DebugForever( CF023_DebugFatal );		// MsgQue OverFllow.
				}	
			}
#ifdef	TORNADO
			STATUS e_MsgRet = DEF_IJC_MSG_Q_SEND(( *st_Task[sendTaskID].msgQID, (char*)messageBuf, sizeof(messageBuf), NO_WAIT, MSG_PRI_NORMAL ));
			if (OK != e_MsgRet) {
				THRL_SetSigHandlerMsgLog( 9, (uintptr_t)e_MsgRet, (uintptr_t)(*st_Task[sendTaskID].msgQID), (uintptr_t)sizeof(messageBuf), 0, 0, 0 );
			}

#else	//TORNADO.
			SYSF_msgQSend( *st_Task[sendTaskID].msgQID, (char*)messageBuf, sizeof(messageBuf), NO_WAIT, MSG_PRI_NORMAL );
#endif	//TORNADO.
#else	// シミュレータで有効.
			m_hWndTam->SimSendTaskMessage( sendTaskID, be_TaskMessage, ul_Option );
#endif	// #if defined(TORNADO)||defined(__SIMWIN_MULTI_THREAD__).
			break;
	}
}
/*================================================================================================
［機能］	タスク間メッセージの受信
--------------------------------------------------------------------------------------------------
［引数］	EV_TaskID		myTaskID       : 受信するタスクＩＤ
			BE_TaskMessage& be_TaskMessage : 受信メッセージ
			uintptr_t& 		ul_Option      : 受信オプション
［戻値］	無し
［ｳｫｰｸｽﾙｰ］	実施済み：2000/8/17：出山
［履歴］	2012/02/14：esl_yamauchi 静的解析 CID:12597 SYSF_msgQReceive()の戻り値判定追加 .
================================================================================================*/
Bool receiveMessage( EV_TaskID myTaskID, BE_TaskMessage& be_TaskMessage, uintptr_t& ul_Option )
{
#if		defined(TORNADO)||defined(__SIMWIN_MULTI_THREAD__)	// VXWORKS,SimWinマルチで有効.
#ifndef COSIM
		uintptr_t messageBuf[2] = {0};
		int32_t i_Ret = 0;
		THRL_ThAssert( (st_Task[myTaskID].msgQID != NULL), "receiveMessage QID error %d", myTaskID , 0, 0, 0, CFA14_DetailCode0x062D );
		i_Ret = SYSF_msgQReceive( *st_Task[myTaskID].msgQID, (char*)messageBuf, sizeof(messageBuf), WAIT_FOREVER );
		be_TaskMessage = (BE_TaskMessage)messageBuf[0];
		ul_Option      = (uintptr_t)messageBuf[1];

		// 静的解析 CID:12597 SYSF_msgQReceive()の戻り値判定追加 .
		if( i_Ret == ERROR )
		{
			printf("### THRC_msgQReceive Error TaskId %d\n", myTaskID);
			THRL_SetThLog( 258, THRE_DebugLogType_MTCtrLog, 200, False, myTaskID);
			return False;
		}
#else
		uintptr_t messageBuf[2];
		THRL_ThAssert( (st_Task[myTaskID].msgQID != NULL), "receiveMessage QID error %d", myTaskID , 0, 0, 0, CFA14_DetailCode0x062E );
		int32_t i_Ret = SYSF_msgQReceive( *st_Task[myTaskID].msgQID, (char*)messageBuf, sizeof(messageBuf), WAIT_FOREVER );
		be_TaskMessage = (BE_TaskMessage)messageBuf[0];
		ul_Option      = (uintptr_t)messageBuf[1];

		if( i_Ret == ERROR ){
			extern void tornadoErrorOccur( const char string[] );
			tornadoErrorOccur( "### THRC_msgQReceive Error" );
			THRL_ThLogMsgErr( "TaskId %d\n", myTaskID );
		}
#endif
	// 2017/02/20 Sky酒井（純）ユーザーイベント記録(APIコール).
	SYSF_ReqTaskMonitorUserEvent(18003, (uint32_t)myTaskID, (uint32_t)be_TaskMessage, (uint32_t)ul_Option);
// 2013/04/04  morikawa debug
         ThOutputLogReceiveMessage( myTaskID, be_TaskMessage, ul_Option );
	return	True;
#else	// シミュレータで有効.
	register Bool ret = m_hWndTam->SimReceiveTaskMessage( myTaskID, (uintptr_t*)&be_TaskMessage, (uintptr_t*)&ul_Option ) == TRUE ? True : False;
	if( ret == True ){
		ThOutputLogReceiveMessage( myTaskID, be_TaskMessage, ul_Option );
	}
	return	(ret == True) ? True : False;
#endif	// #if defined(TORNADO)||defined(__SIMWIN_MULTI_THREAD__).
}
/*================================================================================================
［機能］	タスクの実行時間を延期する
--------------------------------------------------------------------------------------------------
［引数］	long sl_MSTimer : 遅延時間（ｍＳ）
［戻値］	無し
［ｳｫｰｸｽﾙｰ］	実施済み：2000/8/17：出山
［履歴］	2002/6/24：morikawa VxWorksシステムコールに関する不具合修正
				原因：以下、WindRiverのサポートからの回答抜粋
					taskDelay()はtickを使用した精度になりますので、お客様の環境ではシステム
					クロックレートを200Hzで使用しているので1tick　=　5msになります。この場合、
					5ms以下の誤差が発生します。具体的にはtaskDelay()はtickを見ていますので、
					taskDelay(1)を設定した直後にtickがインクリメントされた場合は1tick（5ms）前に
					復帰する事があります。taskDelay(1)としてもワーストケースでは1tick以内で復帰するのは正常な動作となります。
					お客様の環境での精度は0 < wait time <= 5ms となります。
				対策：システムコール実行前に引数を１加算する	
			2006/09/11: 松永 シミュレーション用の処理を追加 
================================================================================================*/
void delayTask( int32_t sl_MSTimer )
{
#ifdef COSIM
	return;
#endif
	register int32_t sl_Tick = (int32_t)SYSF_sysClkRateGet() * sl_MSTimer / 1000;
	if( sl_Tick == 0L ) { sl_Tick = 1L; }
	++sl_Tick;	// 02/06/24 morikawa履歴参照.
#if		defined(TORNADO)||defined(__SIMWIN_MULTI_THREAD__)	// VXWORKS,SimWinマルチで有効.
	SYSF_taskDelay( sl_Tick );
#else	// シミュレータで有効.
	m_hWndTam->SimTaskDelay( sl_Tick );
#endif	// #if defined(TORNADO)||defined(__SIMWIN_MULTI_THREAD__).
}
/*================================================================================================
［機能］	共有データのロック
--------------------------------------------------------------------------------------------------
［引数］	short option	: ID
［戻値］	無し
［ｳｫｰｸｽﾙｰ］	実施済み：2000/8/17：出山
--------------------------------------------------------------------------------------------------
 [履歴]
  	2002.7.13	中田	セマフォロックログデータ取得処理追加
  	2020.7.31	日立	OP_BTS-26050：グローバルセマフォ取得禁止.
================================================================================================*/
#define	DEF_SemphoreIDMax	100							// セマフォロックログ・バッファ最大サイズ.

static short ss_SemphoreCtr = 0;						// セマフォロックログ・バッファカウンタ.
static Bool  bl_SemphoreIndi[DEF_SemphoreIDMax];		// セマフォロックログ・ロック／アンロック指示.
static short ss_SemphoreID[DEF_SemphoreIDMax];			// セマフォロックログ・IDバッファ.
static int32_t  sl_SemphoreTick[DEF_SemphoreIDMax];		// セマフォロックログ・tickバッファ.
void semphoreLock( short option )
{
//Str【OP_BTS-26050 : 2020.07.31】.
//	SYSF_semTake( semID_CommonData, WAIT_FOREVER );		// 共有データのロック（永遠に待ちつづける）.
//End【OP_BTS-26050 : 2020.07.31】.
	bl_SemphoreLock = True;							// セマフォロック状態.
	if( bl_MessageQueOverflow == False && bl_ExceptionOccur == False ) {
		// メッセージキューオーバーフロー発生していないかつ、例外発生していない.
		bl_SemphoreIndi[ss_SemphoreCtr] = True;			// セマフォロックログ・ステータスバッファ.
		ss_SemphoreID  [ss_SemphoreCtr] = option;
//　PCプリントパフォーマンス対応.
//		sl_SemphoreTick[ss_SemphoreCtr] = SYSF_tickGet(); 
		if( ++ss_SemphoreCtr >= DEF_SemphoreIDMax ) {	ss_SemphoreCtr = 0;	}		// セマフォロックログバッファ.
	}
}
/*================================================================================================
［機能］	共有データのロック解除
--------------------------------------------------------------------------------------------------
［引数］	short option	: ID
［戻値］	無し
［ｳｫｰｸｽﾙｰ］	実施済み：2000/8/17：出山
--------------------------------------------------------------------------------------------------
 [履歴]
  	2002.6.21	中田	セマフォのアンロック状態の解除をsemGiveの前に行うようにした。
  	2002.7.13	中田	セマフォアンロックログデータ取得処理追加
  	2020.7.31	日立	OP_BTS-26050：グローバルセマフォ取得禁止.
================================================================================================*/
void semphoreUnLock( short option )
{
	bl_SemphoreLock = False;						// セマフォアンロック状態.
	if( bl_MessageQueOverflow == False && bl_ExceptionOccur == False ) {
		// メッセージキューオーバーフロー発生していないかつ、例外発生していない.
		bl_SemphoreIndi[ss_SemphoreCtr] = False;		// セマフォロックログ・ステータスバッファ.
		ss_SemphoreID  [ss_SemphoreCtr] = option;
//		sl_SemphoreTick[ss_SemphoreCtr] = SYSF_tickGet(); 
		if( ++ss_SemphoreCtr >= DEF_SemphoreIDMax ) {	ss_SemphoreCtr = 0;	}		// セマフォロックログバッファ.
	}
//Str【OP_BTS-26050 : 2020.07.31】.
//	SYSF_semGive( semID_CommonData );					// 共有データのロック解除（永遠に待ちつづける）.
//End【OP_BTS-26050 : 2020.07.31】.
}
/*================================================================================================
［機能］	共有データのロック状態チェック
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	Bool : True=セマフォロック中
［ｳｫｰｸｽﾙｰ］	実施済み：2000/8/17：出山
================================================================================================*/
Bool isSemphoreLock()
{
// 2004/11/19 澤柳 グローバルセマフォの開放・再取得(HDDへのアクセス時にはグローバルセマフォを持たない）・旧NTS用のチェック処理のため、無効とする.
//	return bl_SemphoreLock;						// セマフォロック状態.
	return False;
}
/*================================================================================================
［機能］	タスク生成
--------------------------------------------------------------------------------------------------
［引数］	char* pt_TaskName		: タスク名
			int si_Priority			: 優先順位
			int si_StackSize		: スタックサイズ（単位：kbyte）
			void* pt_EntryFunc		: エントリー関数ポインタ
［戻値］	THR_ID	: タスクＩＤ
［ｳｫｰｸｽﾙｰ］	未実施
================================================================================================*/
THR_ID procTaskSpawn( char* pt_TaskName, int si_Priority, int si_StackSize, void* pt_EntryFunc )
{
#ifdef TORNADO		// VXWORKSで有効.
	return SYSF_taskSpawn( pt_TaskName,	si_Priority, VX_FP_TASK, si_StackSize, (FUNCPTR)pt_EntryFunc,    0,0,0,0,0,0,0,0,0,0 );
#else				// ＰＣシミュレータで有効.
	return 0;
#endif				// #ifdef TORNADO.
}
/*================================================================================================
［機能］	タスク削除
--------------------------------------------------------------------------------------------------
［引数］	THR_ID si_TaskID	: タスクＩＤ
［戻値］	int status		: -1 ERROR, 0 OK
［ｳｫｰｸｽﾙｰ］	未実施
================================================================================================*/
int procTaskDelete( THR_ID si_TaskID )
{
#ifdef TORNADO		// VXWORKSで有効.
	return SYSF_taskDelete( si_TaskID );		// タスク削除.
#else				// ＰＣシミュレータで有効.
	return 0;
#endif	// #ifdef TORNADO		// VXWORKSで有効.
}
/*================================================================================================
［機能］	タスク遷移の禁止（この関数は基本的には使ってはいけない）
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	無し
［ｳｫｰｸｽﾙｰ］	実施済み：2000/8/17：出山
================================================================================================*/
void procTaskLock()
{
#ifdef TORNADO		// VXWORKSで有効.
	SYSF_taskLock();		// タスク遷移の禁止.
#endif	// #ifdef TORNADO		// VXWORKSで有効.
}
/*================================================================================================
［機能］	タスク遷移の許可（この関数は基本的には使ってはいけない）
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	無し
［ｳｫｰｸｽﾙｰ］	実施済み：2000/8/17：出山
================================================================================================*/
void procTaskUnlock()
{
#ifdef TORNADO		// VXWORKSで有効.
	SYSF_taskUnlock();	// タスク遷移の許可.
#endif	// #ifdef TORNADO		// VXWORKSで有効.
}
/*================================================================================================
［機能］	割り込みの禁止
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	無し
［ｳｫｰｸｽﾙｰ］	未実施
================================================================================================*/
int32_t procIntLock()
{
#ifdef TORNADO		// VXWORKSで有効.
	return(SYSF_intLock ());
#else
	return(1);
#endif	// #ifdef TORNADO		// VXWORKSで有効.
}
/*================================================================================================
［機能］	割り込みの許可
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	無し
［ｳｫｰｸｽﾙｰ］	未実施
================================================================================================*/
void procIntUnlock(int32_t lockkey)
{
#ifdef TORNADO		// VXWORKSで有効.
	SYSF_intUnlock (lockkey);
#endif	// #ifdef TORNADO		// VXWORKSで有効.
}
///*================================================================================================.
//	内部制御関数.
//================================================================================================*/.
// 定期起床用処理関数.
static void _wdTimerFunc( EV_TaskID myTaskID )
{
	uintptr_t messageBuf[2];
	messageBuf[0] = (uintptr_t)TM_TimerWakeUp_Sys;
	messageBuf[1] = 0;
	THRL_ThAssert( (st_Task[myTaskID].msgQID != NULL), "_wdTimerFunc QID error %d", myTaskID , 0, 0, 0, CFA14_DetailCode0x062F );
	SYSF_msgQSend( *st_Task[myTaskID].msgQID, (char*)messageBuf, sizeof(messageBuf), NO_WAIT, MSG_PRI_NORMAL );
	register int32_t sl_Tick = st_Task[myTaskID].sl_Tick;
	SYSF_wdStart( *st_Task[myTaskID].wdID, sl_Tick, (FUNCPTR)_wdTimerFunc, myTaskID );	// 次回起動用にウォッチドッグタイマをセット.
}
///*================================================================================================.
//［機能］	DMA終了割り込み発生モニタ（割り込み発生メッセージ表示＆割り込み発生回数カウント）.
//--------------------------------------------------------------------------------------------------.
//［引数］	int	count：割り込み番号.
//［戻値］	無し.
//［ｳｫｰｸｽﾙｰ］	未実施.
//================================================================================================*/.
///*================================================================================================.
//	デバッグ用グローバル変数.
//================================================================================================*/.
ushort us_DebugPrintLineMonitor;		// プリントライン数モニタ.
ushort us_DebugPrintNum;				// プリント回数モニタ.
ushort us_DebugACS;						// COSIMにてACSモードで伸張開始しないデバッグ用。１の時、圧縮完了後ab成分メモリ開放しない。チェック終了後３にする。.
uint32_t ul_DebugTime[TIMER_DEBUG_MAX];	// IJC処理時間計測用.
uint32_t ul_DebugAGCODRYO;					// 圧縮後データサイズ（2101変更が容易なようにグローバル変数とする）.
ushort us_DebugSeqIdx;
uint32_t  ul_DebugLowSeqIdx;
uint32_t  ul_DebugLowLogSeqIdx;
Bool   bl_DebugLowSeqFlag = False;
Bool   bl_DebugLowLogSeqFlag = False;
///*================================================================================================.
//	関数のプロトタイプ宣言.
//================================================================================================*/.
ushort getDebugHighSpeedSeqNo();
ushort getDebugHighSpeedSeq( ushort, uint32_t& ,uint32_t& );
void setDebugHighError( uint32_t );
void clearDebugHighSpeedSeqNo();
uint32_t  getDebugLowSpeedSeqNo();
Bool   getDebugLowSpeedSeqFlag();
ushort getDebugLowSpeedSeq( uint32_t, uint32_t& ,uint32_t& );
void setDebugLowError( uintptr_t );						//2017/01/24 uint32_tからuintptr_tに変更.
void clearDebugLowSpeedSeqNo();
void clearDisableDebugLow( ushort );
void setDisableDebugLow( ushort );
Bool isDisableDebugLow( ushort );
ushort getDebugLowLogSpeedSeqNo();
Bool   getDebugLowLogSpeedSeqFlag();
ushort getDebugLowLogSpeedSeq( ushort, uint32_t& ,uint32_t& );
void setDebugLowLogError( uintptr_t );
void clearDebugLowLogSpeedSeqNo();
void clearDisableDebugLowLog( ushort );
void setDisableDebugLowLog( ushort );
Bool isDisableDebugLowLog( ushort );
// HDDシーケンスログ用(debugLog 12).
void setDebugHddSeq( ushort us_State, uint32_t ul_Data ) ;
ushort getDebugHddSeq( ushort us_No, uint32_t& ul_Data, uint32_t& ul_Tick ) ;
ushort getDebugHddSeqNo() ;
uint32_t SetulongdecToField( uchar, uchar, uint32_t );	// 02/06/06 tsukiji 関数追加.
uint32_t GetulongdecToField( uchar, uchar, uint32_t );	// 02/06/06 tsukiji 関数追加.
uint32_t _tenpow( uchar );								// 02/06/06 tsukiji 関数追加.

// ターミナルから呼ぶ物はここで記述する.
extern "C" {
void mem();
void fill(uint32_t* buf, uchar mode, uint32_t val );
void dump(uint32_t* buf, uchar mode );
void LogSendMessage();
void clearLogSendMessage();
//void CompDebugEnable();.
void LogCompImageTask();
//void ExpDebugEnable();.
void LogExpImageTask();
void HddDebugEnable();
void LogHddTask();
void GetComIRC();
void GetComFeed();
// EMI評価用メモリRead/Writeチェック機能追加.
void memTrans(uint32_t ul_Dst, uint32_t ul_Src, uint32_t ul_Size, Bool bl_EndlessFlag);
void memTransStart(uint32_t ul_Dst, uint32_t ul_Src, uint32_t ul_Size, Bool bl_EndlessFlag);
void memTransStop();
void memTransCancel();
}
///*================================================================================================.
//	モジュール内関数のプロトタイプ宣言.
//================================================================================================*/.

//.
///*================================================================================================.
//	ＨＤＤ検証用グローバル変数の宣言.
//================================================================================================*/.
uint32_t	ul_Test_LBA = 0;
uint32_t	ul_DebugLogErrCount_Low  = 0;
uint32_t	ul_DebugLogErrCount_High = 0;
uint32_t ul_DebugLogDisableCount_Low = 0;
uint32_t	ul_DebugLogMemChangeTick = 0;	// Memory Status Change Tick(latest) 2005/06/06 added by t.sasaki.
Bool	bl_HDDtestMode = False;		// ＨＤＤテストモード.
Bool	bl_HDDtestExec = False;		// ＨＤＤコマンド実行中.
static ST_DEBUGSEQ st_DebugSeq[DEF_DebugSeq_MAX];
static ST_DEBUGSEQ st_DebugLowSeq[DEF_DebugSeq_MAX];
static ST_DEBUGSEQ *pst_DebugLowSeq = st_DebugLowSeq;		// 2014/4/18 伊藤史. debugLog 90 拡張.
uint32_t	ul_DebugLowSeq_MAX = DEF_DebugSeq_MAX;
typedef struct {
	uint32_t ul_Error;
	uintptr_t ul_Time;
} ST_DEBUGERR;
ST_DEBUGERR	st_DebugHighErr[10];
ST_DEBUGERR	st_DebugLowErr[10];
uint32_t ul_DebugLogDisable_Low[10];
static ST_DEBUGSEQ st_DebugHddSeq[DEF_CONNECTLOG_NUM];
ushort	us_DebugHddSeqIndex = 0 ;
bool	bl_DebugHddSeqIndexLoop = false ;

ushort us_DebugLowLogSeqIdx;
uint32_t	ul_DebugLogErrCount_LowLog  = 0;
uint32_t ul_DebugLogDisableCount_LowLog = 0;
static ST_DEBUGSEQ st_DebugLowLogSeq[DEF_DebugLog92Seq_MAX];//CQ315824 tadamasa debugLog 92のログサイズを増やす.
static ST_DEBUGSEQ *pst_DebugLowLogSeq = st_DebugLowLogSeq;
uint32_t	ul_DebugLowLogSeq_MAX = DEF_DebugLog92Seq_MAX;
ST_DEBUGERR	st_DebugLowLogErr[10];
uint32_t ul_DebugLogDisable_LowLog[10];
// 2010/11/24 UV-00794 デバッグログ収集システム対応 M.Koshimo start.
static LIBS_ST_DEBUGSEQ_90_Index	st_debugLog90_Index;
static LIBS_ST_DEBUGSEQ				st_DebugLowSeq90[LIBD_DEF_DebugSeq_MAX];
static LIBS_ST_DEBUGSEQ_92_Index	st_debugLog92_Index;
static LIBS_ST_DEBUGSEQ				st_DebugLowSeq92[LIBD_DEF_DebugLog92Seq_MAX];
static LIBS_ST_DEBUGSEQ_12_Index	st_debugLog12_Index;
static LIBS_ST_DEBUGSEQ				st_debugLog12[LIBD_DEF_CONNECTLOG_NUM];
static LIBS_ST_HDDATBL_Index		st_HDDATBL_Index;
static LIBS_ST_HDDATBL				st_HddaTblFatal_debug[LIBD_HDDAFATALTBLMAX];

// 2012/12/20 ADS吉田  CQ.377858  MS-LCL_9 不要変数削除対応.
extern LIBU_ST_DebugDirectCommand union_DebugDirectCommand;
extern void THRL_DebugDirectCommandWriteSystemLog( LIBD_LogID e_LogID );
// 2010/11/24 UV-00794 デバッグログ収集システム対応 M.Koshimo end.

ST_mesDMAInterrupt st_mesDMAInterrupt[31] = {
	{ (char *)"" },
};

void printCosimDMAInterrupt( uint32_t ul_data )
{
}
///*================================================================================================.
//［機能］	DMA起動モニタ（割り込み発生メッセージ表示＆割り込み発生回数カウント）.
//--------------------------------------------------------------------------------------------------.
//［引数］	ulong ul_bit：割り込み識別ビット.
//［戻値］	無し.
//［ｳｫｰｸｽﾙｰ］	未実施.
//================================================================================================*/.
void printCosimDMAStart(uint32_t ul_bit)
{
}
// ================================================================================================.
//! <!--機能--> DebugForever()にてサスペンドさせるかどうかを設定.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param Bool b_isAssert ... これが真の場合、アサート処理を行う.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//!  \n.
//! <!--履歴-->.
//  2006/02/08 yokoi 新規作成.
// ================================================================================================.
Bool b_suspendDebugForever = False;
void suspendDF( int32_t para ){
	b_suspendDebugForever = (Bool)para;
	if( b_suspendDebugForever == True ){
		printf( "suspend at debugForever\n" );
	} else {
		printf( "not suspend at debugForever\n" );
	}
}
/*================================================================================================
［機能］	デバッグ用にトラブルコードをパネルに表示
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	無し
--------------------------------------------------------------------------------------------------
 [履歴]	2002/04/02	冨安	トラブルのセットから、ABORT情報の設定関数コールに変更。 
							現状の暫定処置として、トラブルのセットは、ABORT情報設定関数内でコールする。 
		03/12/27 morikawa Rhein1スレッドのログに残す				
		04/02/02 morikawa タスクをsuspend追加　				
		04/02/05 morikawa エンジンのFATALコードを上書きしていた不具合回避、エンジンがすでにFATAL発生しているときは
							上書きしない
						  タスクをsuspend削除　	
		04/02/09 morikawa パネルにメモリ関係のFATALが表示されるようになったのでエンジンの暫定FATAL削除　				  
		06/06/13 yokoi	  割り込みタスクからの処理の場合、メッセージ経由するように修正 
		06/07/05 yokoi	  Fatalが出続けてログがとりづらくなるので、一回Fatalがでれば以降はログを出さないように修正。 
		07/10/30 yokoi	  Simでも、実機と同じようにメッセージ経由でFatalを出すように修正 
  	    10/02/24 M.Koshimo	CQ.324008 右ドア開けるとパネルロック対応.
  	    10/03/18 S.Ohba	  CQ324460 トラブルコード5F07(パネル表示はF907)を検出したらrebootさせる.
		13/08/09 鈴木		静的解析対応3rd.klocwork専用コード追加.
		20/08/20 Sky	  FA14詳細コード対応.
================================================================================================*/
ushort us_DebugForeverCode = 0;
Bool bl_debugFatal = False;	// 【修正:橋本昌】2004/02/17 デバッグ用 Fatalが発生検出用フラグ False:正常, True:Fatal発生.
void DebugForever( ushort us_Code, ushort us_OldDetailCode )
{
#ifdef COSIM
		THRL_ThLogMsgErr( "###DebugForever %u", us_Code );
#if defined(COSIM) && defined(THR_SIMWIN) // 【COSIM+ｽﾚｯﾄﾞSIMWIN対応】.
#else
		dbgCOSIMBreakPoint();
#endif // #if defined(COSIM) && defined(THR_SIMWIN) // 【COSIM+ｽﾚｯﾄﾞSIMWIN対応】.
#endif
	if( bl_debugFatal == True ){
		// すでにFatalが発生していれば、後は無効とする.
		return;
	}
// CQ.324008 FC21発生後のトラブルは無効とする.
#if defined(DEF_TaigaCus) || defined(DEF_DonauBKCus) // 2011/02/22 ft_kondo 第6回手動マージ. 
	if(( us_waitDebugForeverCode == 0xFC21 ) && (us_Code != 0xFC21 )){
		return;
	}
#endif
	THRL_SetThLog( 259, THRE_DebugLogType_MTCtrLog, 200, False, us_Code );

	// メモリ管理部のFATALを使用する.
// 2006/06/13 yokoi mod 割り込みタスクからの場合は、メッセージ経由にするように変更.
// setFatal() の中で、new処理を行うものがあるため。.
	if( SYSF_intContext() == TRUE ) {
// CQ.324008 FC21発生時には状態を保持.
#if defined(DEF_TaigaCus) || defined(DEF_DonauBKCus) // 2011/02/22 ft_kondo 第6回手動マージ. 
		if(( us_waitDebugForeverCode == 0x0000 ) && ( us_Code == 0xFC21 )){
			us_waitDebugForeverCode = us_Code;
		}
#endif
		int_sendMessage( TID_ThreadManager, TM_ThAssert, (uintptr_t)us_Code );
		return;
	} else {
		//2009/03/30 newInstance対応 comMemoryManagerのインスタンス生成.
		IMGC_ImageMemoryPoolImpColorFileMemoryInstance& comMemoryManager = IMGC_ImageMemoryPoolImpColorFileMemorySingletonIf::newInstance();
	
	// 07/04/03 morikawa Donau C5023のグローバルセマフォロックタイムアウトログ.
	// setFatalCodeToIJC()をコールするとまたグローバルセマフォを獲得待ちになるので前もってログを出す.
	// 07/09/03 yokoi C5023では分かりづらいので、新規にFA15とした.
		if( us_Code == CFA15_DebugFatal ) {
#ifdef TORNADO
#ifndef COSIM	// ★COSIMエラー対策　松永.
//			if( semID_CommonData->semOwner != NULL ).
//			{.
//				extern SEM_ID semID_CommonData;			// グローバル（共有データ）セマフォID.
//				extern int debug_timeout_take_globalsem;	// イメージタスクでセットするタイムアウト時間.
				// タスクをサスペンドする.
//				taskSuspend( (int)semID_CommonData->semOwner );.
//				THRL_ThLogMsgErr( "●● GlobalSemphore TimeOut=%d(ms) ●●\n", debug_timeout_take_globalsem );.
//				THRL_ThLogMsgErr("semShow(semID_CommonData,1 )");.
//				SYSF_semShow( semID_CommonData,1 );.
				// タスクトレース実行.
//				tt( (int)semID_CommonData->semOwner );.
//			}.
#endif
#endif
		}	

		comMemoryManager.setFatalCodeToIJC( us_Code, us_OldDetailCode );		// 2004/07/20 伊藤(史) FATALカウンタ対応. // 2020/08/20 FA14詳細コード対応.
	}

	us_DebugForeverCode = us_Code;	// 03/06/10　morikawa　COSIM時EWS環境でもグローバル変数でチェック可能にする.

	THRL_ThLogMsgErr( "●● DebugForever C-%04X ●●\n", us_Code );
	THRL_ThLogMsgDebug( "●● DebugForever C-%04X ●●\n", us_Code );
	bl_debugFatal = True;	// 【修正:橋本昌】2004/02/17 デバッグ用 Fatalが発生検出用フラグ False:正常, True:Fatal発生.

	if( b_suspendDebugForever == True ){
#ifdef TORNADO
		// デバッグ用にサスペンドさせる場合、割り込み以外ならサスペンドさせる.
		if( SYSF_intContext() == FALSE ) {
			SYSF_taskSuspend( SYSF_taskIdSelf() );
		}
#endif
	}
#ifdef TORNADO
	// 2006/11/13 伊藤史：FATAL時HDDログ対応。FATAL発生時に停止。.
	// 常時HDDログの停止.
	// 2008/07/23 t.yokoyama：[CQ308783]常時HDDログ対応.
void stopHddaFatal(ushort us_Code);
	stopHddaFatal(us_Code);
	#define THRD_5F07_REBOOT_MAXCOUNT	(10)

	if( us_Code == 0x5F07 ){
		APIC_SoftDipSw* pc_SoftDipSw = APIC_SoftDipSw::newInstance();
		if( pc_SoftDipSw->getSoftDipSwBit( 39, 8 ) == True ){
			APIC_SystemStatus *pc_SystemStatus = APIC_SystemStatus::newInstance();
			if( pc_SystemStatus->getRebootCount() < THRD_5F07_REBOOT_MAXCOUNT ){
				pc_SystemStatus->incRebootCount();

				logMsg( "Detect F907 starting reboot.....\n", 0, 0, 0, 0, 0, 0 );
				APIC_SystemServiceFunction*	pc_SystemServiceFunction = APIC_SystemServiceFunction::newInstance();
				pc_SystemServiceFunction->reboot();
			}
		}
	}
#endif	// #ifdef TORNADO.

#ifdef __KLOCWORK__
  abort();  /* klockwokの解析を終わらせるためのダミーコード */
#endif
}

// トラブルコードがFA14の場合は、下記DebugForeverFA14関数を使用すること.
void DebugForeverFA14( ushort us_Code, ushort us_FA14DetailCode )
{
    // NVRAMにFA14詳細コードを書き込む.
    THRL_ThAssertLog( us_FA14DetailCode );


/* -fordebug- */
	THRL_ThLogMsgErr( "●● DebugForeverFA14 DetailCode=0x%04X ●●\n", us_FA14DetailCode );
/* -fordebug- */


	if( us_Code != 0xD403 )	// 2014/03/14 Harada CQ.387486 C-D403を検出しない、かつAndroid接続中でないときのみFA14詳細コードを記録する.
	    DebugForever( us_Code, us_FA14DetailCode );	// 2020/07/13 FA14詳細コード対応.
}

/*================================================================================================
［機能］	ターミナルコマンド（メモリライト）
--------------------------------------------------------------------------------------------------
［引数］	uint32_t*	buf
			uchar	mode
			uint32_t	val
［戻値］	無し
--------------------------------------------------------------------------------------------------
 [履歴]		2002.02.27	DEF_IJC_PRINTF　→　printfに変更（ハードウェア確認で必要なため）
================================================================================================*/
void fill(uint32_t* buf, uchar mode, uint32_t val )
{
	switch (mode) {
		case 1:			// BYTE Access.
			*(uchar*)buf	= (uchar)val;
			break;
		case 2:			// SHORT Access.
			*(ushort*)buf	= (ushort)val;
			break;
		case 4:			// LONG Access.
			*(uint32_t*)buf	= (uint32_t)val;
			break;
		default :
			printf("fill -- Mode Error (mode = 1,2,4)\n");
			break;
	}
}
/*================================================================================================
［機能］	ターミナルコマンド（メモリリード）
--------------------------------------------------------------------------------------------------
［引数］	ulong*	buf
			uchar	mode
［戻値］	無し
--------------------------------------------------------------------------------------------------
 [履歴]		2002.02.27	DEF_IJC_PRINTF　→　printfに変更（ハードウェア確認で必要なため）
================================================================================================*/
void dump(uint32_t* buf, uchar mode )
{
	switch (mode) {
		case 1:			// BYTE Access.
			printf("%pH:  0x%02X\n", buf, *(uchar*)buf);
			break;
		case 2:			// SHORT Access.
			printf("%pH:  0x%04X\n", buf, *(ushort*)buf);
			break;
		case 4:			// LONG Access.
			printf("%pH:  0x%08"PRIX32"\n", buf, *(uint32_t*)buf);
			break;
		default :
			printf("dump -- Mode Error (mode = 1,2,4)\n");
			break;
	}
}

/*==============================================================================
［機能］	メモリデータ転送スタート
--------------------------------------------------------------------------------------------------
［引数］	ulong ul_Dst		：転送先
ulong ul_Src		：転送元
ulong ul_Size		：転送するサイズ
Bool bl_EndlessFlag	：データ転送エンドレスで行うかどうか
［戻値］	無し
［詳細］	
［ｳｫｰｸｽﾙｰ］	未実施
==============================================================================*/
void memTrans(uint32_t ul_Dst, uint32_t ul_Src, uint32_t ul_Size, Bool bl_EndlessFlag){
	printf("チェック開始しました\n");
	memTransStart(ul_Dst, ul_Src, ul_Size, bl_EndlessFlag);
}
/*=============================================================================
［機能］	メモリデータ転送情報セット
--------------------------------------------------------------------------------------------------
［引数］	ulong ul_Dst		：転送先
ulong ul_Src		：転送元
ulong ul_Size		：転送するサイズ
Bool bl_EndlessFlag	：データ転送エンドレスで行うかどうか
［戻値］	無し
［詳細］	
［ｳｫｰｸｽﾙｰ］	未実施
==============================================================================*/
uintptr_t ul_DstHold;	// ターミナルからの設定値を保持しておく変数.
uintptr_t ul_SrcHold;	// ターミナルからの設定値を保持しておく変数.
uint32_t ul_SizeHold = 0;	//　ターミナルからの設定値を保持しておく変数.
uint32_t ul_LoopHold = 0;	//　データ転送ループ回数を保持しておく変数.
uint32_t ul_MaxLoopHold = 0;	//  データ転送ループ回数最大値を保持しておく変数.
uint32_t ul_memOffset = 0;	//  データ転送時のアドレスオフセットを保持.
Bool bl_EndlessFlagHold = False;	// ターミナルからの設定値を保持しておく変数.
void memTransStart(uint32_t ul_Dst, uint32_t ul_Src, uint32_t ul_Size, Bool bl_EndlessFlag){
	ul_DstHold = ul_Dst;
	ul_SrcHold = ul_Src;
	ul_SizeHold = ul_Size;
	bl_EndlessFlagHold = bl_EndlessFlag;	
	ul_MaxLoopHold = ul_Size/sizeof(uint32_t);	// 書き込み繰り返し回数.
}
/*==============================================================================
［機能］	メモリデータ転送ストップ
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	無し
［詳細］	
［ｳｫｰｸｽﾙｰ］	未実施
==============================================================================*/
void memTransStop(){
	ul_MaxLoopHold = 0;	//  データ転送ループ回数最大値を0.
	ul_LoopHold = 0;		//　データ転送ループ回数を0.
	ul_memOffset = 0;		//  データ転送時の0.
	printf("チェック途中で終了しました\n");
}
/*==============================================================================
［機能］	メモリデータ転送ストップ
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	無し
［詳細］	
［ｳｫｰｸｽﾙｰ］	未実施
==============================================================================*/
void memTransCansel(){
	ul_MaxLoopHold = 0;	//  データ転送ループ回数最大値を0.
	ul_LoopHold = 0;		//　データ転送ループ回数を0.
	ul_memOffset = 0;		//  データ転送時の0.
	printf("チェック途中で終了しました\n");
}
/*================================================================================================
［機能］	プリントタスクシーケンス確認
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	無し
［詳細］	
［ｳｫｰｸｽﾙｰ］	未実施
--------------------------------------------------------------------------------------------------
［履歴］	2002.2.20		中田	漢字が表示されないので変更した。
================================================================================================*/
extern short getDebugSequence( short ss_No );						//イベントNo取得.
extern ushort getDebugSequencePage( short ss_No );					//パラメータ取得.
extern uint32_t getDebugSequenceTime( short ss_No );					//時間取得.
extern short getDebugSequenceIndex();								//インデックス取得.

// LogPrintTask Engine IF(Map)復活.
extern short getEngDebugMapAddress( short ss_No );					//イベントNo取得.
extern short getEngDebugMapPara( short ss_No );						//パラメータ取得.
extern uint32_t getEngDebugMapTime( short ss_No );						//時間取得.
extern short getEngDebugMapIndex();									//インデックス取得.

#define PIC_LOG_TBL_SIZE 100		
#define ENG_LOG_TBL_SIZE 100		
extern short getPicDebugSequenceEvent( short ss_No );						//イベントNo取得.
extern short getPicDebugSequencePara( short ss_No );						//パラメータ取得.
extern uint32_t getPicDebugSequenceTime( short ss_No );						//時間取得.
extern short getPicDebugSequenceIndex();									//インデックス取得.
/*================================================================================================
［機能］	送信メッセージ確認
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	無し
［詳細］	
［ｳｫｰｸｽﾙｰ］	未実施
================================================================================================*/
extern Bool getDebugSequenceSendMessage( short, uchar&, ushort&, uintptr_t& );
extern short getDebugSequenceIndexSendMessage();
void LogSendMessage()
{
	register uchar	uc_TaskID;	register ushort	us_TaskMessage;	register uintptr_t ul_Option;
	register short ss_Index = getDebugSequenceIndexSendMessage();
	register short ss_SeqNo = 0;
	for( register short i = 0; i < 100; i++ ) {
		register short ss_TblNo = (short)(ss_Index + i);	if( ss_TblNo >= 100 ) { ss_TblNo = (short)(ss_TblNo - 100); }
		if( getDebugSequenceSendMessage( ss_TblNo, uc_TaskID, us_TaskMessage, ul_Option ) == True ) {
			printf("%03d : ", ++ss_SeqNo );
			switch( uc_TaskID ) { // タスクＩＤ.
				case TID_Input:			printf( "画像入力                      " );	break;
				case TID_Expand:		printf( "画像伸張                      " );	break;
				case TID_Output:		printf( "画像出力                      " );	break;
				case TID_Compress:		printf( "画像圧縮                      " );	break;
				case TID_HDDTrans:		printf( "ＨＤＤデータ転送              " );	break;
				case TID_WorkInput:		printf( "ワーク画像入力                " );	break;
				case TID_WorkOutput:	printf( "ワーク画像出力                " );	break;
				case TID_Image:			printf( "画像・メモリ管理部            " );	break;
#ifdef DEF_Soc1200
				case TID_MainToSubF:	printf( "MainToSubF                    " );	break;	// 2013/07/12 ADS岡村 CQ.381171(Act_66426) SC-SFM-4 サブメモリのファイルメモリ化.
				case TID_SubFToMain:	printf( "SubFToMain                    " );	break;	// 2013/07/12 ADS岡村 CQ.381171(Act_66426) SC-SFM-4 サブメモリのファイルメモリ化.
#endif	// DEF_Soc1200.
				default:				printf( "なし                          " );	break;
			}
			printf( "/ MessageID(%03d) / Option(0x%lx)\r\n", us_TaskMessage, ul_Option );
		}
	}
}
/*================================================================================================
［機能］	送信メッセージログを初期化する
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	無し
［詳細］	
［ｳｫｰｸｽﾙｰ］	未実施
================================================================================================*/
extern void clearDebugSendMessage();
void clearLogSendMessage()
{
	clearDebugSendMessage();
}
/*================================================================================================
［機能］	どのタスクＩＤの送信メッセージを検出するかをセットする 
--------------------------------------------------------------------------------------------------
［引数］	int taskID		: タスクＩＤ( 99 : 全てのタスク ) 
			int on			: 検出の有無( 1 : 検出する  その他 : 検出しない ) 
［戻値］	無し
［詳細］	
［ｳｫｰｸｽﾙｰ］	未実施
--------------------------------------------------------------------------------------------------
［履歴］	2015/10/29		Sky	システム統合SoC(S800)対応.
================================================================================================*/
void SetTaskID( int32_t taskID, int32_t on )
{
	if( ( 0 <= taskID && taskID < TID_MAX ) || taskID == 99 ) {
		setDebugDetectTask( (uchar)taskID, ( on == 1 ) ? True : False );
	} else {
		for( register short i = 0; i < TID_MAX; i++ ) {
			printf("%03"PRId32" : ", i );
			switch( (uchar)i ) { // タスクＩＤ.
				case TID_Input:			printf( "画像入力                      " );	break;
				case TID_Expand:		printf( "画像伸張                      " );	break;
				case TID_Output:		printf( "画像出力                      " );	break;
				case TID_Compress:		printf( "画像圧縮                      " );	break;
				case TID_HDDTrans:		printf( "ＨＤＤデータ転送              " );	break;
				case TID_WorkInput:		printf( "ワーク画像入力                " );	break;
				case TID_WorkOutput:	printf( "ワーク画像出力                " );	break;
				case TID_Image:			printf( "画像・メモリ管理部            " );	break;
#ifdef DEF_Soc1200
				case TID_MainToSubF:	printf( "MainToSubF                    " );	break;	// 2013/07/12 ADS岡村 CQ.381171(Act_66426) SC-SFM-4 サブメモリのファイルメモリ化.
				case TID_SubFToMain:	printf( "SubFToMain                    " );	break;	// 2013/07/12 ADS岡村 CQ.381171(Act_66426) SC-SFM-4 サブメモリのファイルメモリ化.
#endif	// DEF_Soc1200.
				default:	break;
			}
			if( ( i + 1 ) % 3 == 0 )	{	printf( "\r\n"); }
			else						{	printf( "\t" ); }
		}
		printf( "\r\n [Format]SetTaskID taskID, on\r\n" );
		printf( "\t taskID … タスクＩＤ( 99 : 全てのタスク )");
		printf( "\t on     … 検出有無( 1 : 有り    0 : 無し )\r\n");
	}
}


extern ushort us_debug_comparbi;
extern ushort us_debug_dispparam;
extern ushort us_debug_no_restore;
extern ushort us_debug_no_store;
extern ushort us_debug_memorymanager;
extern ushort us_debug_dummyHDD;
extern ushort us_debug_HDDdispparm;
extern ushort us_debug_HDDTime;
extern ushort us_debug_cancel;
extern uchar bl_dbg_FAX_RedZone_StopScan;		// 2006/02/22 伊藤(史)：FAX R40対策  debugFlag 29.
extern ushort us_debug_ActiveImgStblz;	// 毎回画像安定化設定.
extern ushort us_debug_ActiveCleaning;	// １０枚クリーニング実行設定.
extern ushort us_debug_StopCommunication;	// エンジン通信停止設定.
extern ushort us_debug_dispInputParam;	// 外部画像入力パラメータ表示.
extern ushort us_debug_dispPciInputParam;	// PCI入力パラメータ表示.
extern ushort us_debug_dispConvParam;	// 画像変換パラメータ表示.
extern ushort us_debug_dispOutputParam;	// 外部画像出力パラメータ表示.
ushort us_debug_dispDMA04 = 0;	// DMA04パラメータ表示.
extern ushort us_debug_dispFaxCompExpParam;	// Fax圧縮/伸張パラメータ表示.
extern ushort us_debug_dispPciOutputParam;	// PCI出力パラメータ表示.
extern ushort us_debug_ForceBlack;		// 強制モノクロ印字モード設定.
void msgQLog()
{
	printf( "100:MsgQ Num Display\n" );
#ifdef TORNADO			
	printf( "msgQID_Input=%d\n",SYSF_msgQNumMsgs(msgQID_Input)) ;				// サブタスク					画像入力.
	SYSF_msgQShow( *st_Task[TID_Input].msgQID, 0 );
	printf( "msgQID_Expand=%d\n",SYSF_msgQNumMsgs(msgQID_Expand)) ;				// サブタスク					画像伸張.
	SYSF_msgQShow( *st_Task[TID_Expand].msgQID, 0 );
	printf( "msgQID_ImgBufEngOut=%d\n",SYSF_msgQNumMsgs(msgQID_ImgBufEngOut)) ;				// サブタスク					画像出力.
	SYSF_msgQShow( *st_Task[TID_Output].msgQID, 0 );
	printf( "msgQID_Compress=%d\n",SYSF_msgQNumMsgs(msgQID_Compress)) ;			// サブタスク					画像圧縮.
	SYSF_msgQShow( *st_Task[TID_Compress].msgQID, 0 );
	printf( "msgQID_HDDTrans=%d\n",SYSF_msgQNumMsgs(msgQID_HDDTrans)) ;			// サブタスク					HDDデータ転送.
	SYSF_msgQShow( *st_Task[TID_HDDTrans].msgQID, 0 );
	printf( "msgQID_WorkInput=%d\n",SYSF_msgQNumMsgs(msgQID_WorkInput)) ;		// サブタスク					ワーク画像入力.
	SYSF_msgQShow( *st_Task[TID_WorkInput].msgQID, 0 );
	printf( "msgQID_WorkOutput=%d\n",SYSF_msgQNumMsgs(msgQID_WorkOutput)) ;		// サブタスク					ワーク画像出力.
	SYSF_msgQShow( *st_Task[TID_WorkOutput].msgQID, 0 );
	printf( "msgQID_Image=%d\n",SYSF_msgQNumMsgs(msgQID_Image)) ;				// メインタスク					画像・メモリ管理部.
	SYSF_msgQShow( *st_Task[TID_Image].msgQID, 0 );
	printf( "msgQID_InputCnv=%d\n",SYSF_msgQNumMsgs(msgQID_InputImageConv)) ;	// サブタスク					入力画像処理.
	SYSF_msgQShow( *st_Task[TID_InputImgCnv].msgQID, 0 );
#ifdef DEF_Soc1200
// 2013/07/12 ADS岡村 CQ.381171(Act_66426) SC-SFM-4 サブメモリのファイルメモリ化 Start.
	printf( "msgQID_MainToSubF=%d\n",SYSF_msgQNumMsgs(msgQID_MainToSubF)) ;		// サブタスク					メインメモリtoサブファイルメモリ DMA01使用[Main→Sub/Main].
	SYSF_msgQShow( *st_Task[TID_MainToSubF].msgQID, 0 );
	printf( "msgQID_SubFToMain=%d\n",SYSF_msgQNumMsgs(msgQID_SubFToMain)) ;		// サブタスク					サブファイルメモリtoメインメモリ DMA10使用[Sub/Main→Main].
	SYSF_msgQShow( *st_Task[TID_SubFToMain].msgQID, 0 );
// 2013/07/12 ADS岡村 CQ.381171(Act_66426) SC-SFM-4 サブメモリのファイルメモリ化 End.
#endif	// DEF_Soc1200.

#endif	// #ifdef TORNADO.
}
extern int32_t debugtaskImage;
extern ushort us_debug_compdisp;
extern ushort us_debug_compdispExp;
//091030 soota 倍率値表示.
extern ushort us_Debug_DispRatio;	// マルチスレッド 倍率算出結果 表示.
extern ushort us_Debug_AppInConvDiv_IpRomMode;	// アクセラレータIPROM各種画像変換モード表示状態。デフォルト値0とする.
extern ushort us_Debug_AppInConvDiv_IpRomData;	// アクセラレータIPROM各種テーブルデータ出力状態。デフォルト値0とする.
extern ushort us_Debug_CacheInvalidateInfo;	// キャッシュ無効化情報ログ出力状態。デフォルト値0とする.
#ifdef DEF_Soc1200	// 2013/07/24 ADS萩原 CQ.381171(Act_66426) SC-SFM-4 サブメモリのファイルメモリ化.
extern ushort us_debug_dispMainToSubFParam;
extern ushort us_debug_dispSubFToMainParam;
#endif	// DEF_Soc1200.

ushort us_debug_HDDcapture = 0;

ushort us_debug_cmpexpArrayinfo = 0;
ushort us_debug_dspArrayInfo = 0;

extern ushort us_debug_disableSplitWrite;	// 2013/08/06 伊藤史. Split禁止.
extern ushort us_debug_disableSplitRead;	// 2013/10/03 wakui 分割転送の対応.
extern uint32_t  ul_debug_isSplitTransThresholdSizeWrite;	// 2013/10/03 wakui 分割転送の対応.
extern uint32_t  ul_debug_isSplitTransThresholdSizeRead;	// 2013/10/03 wakui 分割転送の対応.
extern ushort us_debug_isSplitResult;	// 2013/12/04 wakui	分割転送の対応.
extern ushort us_debug_WorkingArrayLog;			// 2014/07/01 国吉(日立JTE) debugF 35.

ushort us_debug_chkDeadLock = 0;		//  2013/09/02 伊藤史. CQ382618 BufToFile停止問題対策.
ushort us_debug_chkEnableExpand = 0;	//  2013/09/20 伊藤史. マルチスレッド chkEnableExpand 確保量詳細.
extern ushort us_debug_TransLine;
ushort us_debug_forceE002 = 0;
extern ushort us_debug_getTransMode;	// 2014/03/11 伊藤史. getTransModeの結果表示.
extern ushort us_debug_2nd_CompressSize_Scalable;
extern void extendDebugLowSpeedSeq(ushort cnt);
extern void extendDebugLowLogSpeedSeq(ushort cnt);
extern void THRL_extendThLog( uint32_t ul_MaxDebugLogNum );
extern ushort us_debug_ImageInfoArraynumLog;		// debufF 31.
extern void dbg_InitImageInfoArraynumLog();
extern void debug_Init_workArrayDebugLog( uint32_t uldt );
extern uint32_t ul_MaxDMAResourceLogNum;
extern void dbg_extendDMAResourceLog( uint32_t ul_LogNum );
ushort us_debug_chkResource = 0;
ushort us_debug_WorkingArrayLogMode = 0;		// 2014/07/01 国吉(日立JTE) debugF 20.
ushort us_debug_2ndCompAllocPercent = 0;		// 2014/11/27 伊藤史. デバッグ機能：２次圧縮ファイルメモリを増減.
extern void dbg_NetScanIfaxTx_setFileNameDisp( short ssdt );	// 2014/11/29 伊藤史. NETC_NetScanIfaxTx::_setFileNameで送信ファイル名を表示する.
ushort us_debug_2ndLowDpi100Percent = 0;		// 2015/06/09 伊藤史. デバッグ機能：CQ514118 低解像度 90dpi 以下は最悪圧縮率にする対応を無効.
ushort us_debug_dispBlockSizeCnvInfo = 0;		// 2019/2/28 SCD2 伊藤史. EagleL MR1 制御構成変更No.1 BOX画像(KM形式)インポート対応（Eagle世代越リストア).
extern void debug_Init_S800JSeqLog(uint32_t uldt);	// 2019/11/6 伊藤史. OP_BTS14698 S800J制御ログ追加.
extern uint32_t	ui32_S800JSeqLogNum;

/*================================================================================================
［機能］	ターミナルコマンド（各種デバッグログ表示/非表示切替えコマンド）
--------------------------------------------------------------------------------------------------
［引数］	int	mode		デバッグログ種別指定モード.
			int	on			ログ表示状態.
								0：非表示、0以外：表示
［戻値］	無し
--------------------------------------------------------------------------------------------------
 [履歴]		2012.03.08	世古	アクセラレータIPROMカスタマイズ対応.IPROMデバッグログ出力設定追加.
			2013.10.03	wakui	分割転送の対応.
	2014.07.01	国吉(日立JTE) CQ.500968 debug_Dsp_workArrayLogのログを改善する.
							debug_Dsp_workArrayLogのログレベルを切り替える debugF35 を追加.
							us_debug_WorkingArrayLog=1 簡易ログ.
							us_debug_WorkingArrayLog=2 圧縮・伸張で使用する作業用アレイ間の制御ログ.
							us_debug_WorkingArrayLog=3 その他詳細ログ.
================================================================================================*/
void debugF(int32_t mode, int32_t on)
{
	THRS_ThDebugFArg	s_HiSpeedTskPara ;

	printf("========== Thread ==========\n");
	printf("Various debug log display / non-display switching command\n");
	printf("========== Thread ==========\n");

	switch( mode ) {
		case 1:
			break;
		case 2:
			break;
		case 3:
			printf( "3:Compress Data Display Mode[%d]<=[%"PRId32"]\n",us_debug_compdisp, on );
			us_debug_compdisp = (ushort)on;
			break;
		case 4:
			printf( "4:Expand Data Display Mode[%d]<=[%"PRId32"]\n",us_debug_dispparam, on );
			us_debug_dispparam = (ushort)on;
			break;
		case 5:
			printf( "5:Message Data Display Mode[%d]<=[%"PRId32"]\n",us_debug_dismessage, on );
			us_debug_dismessage = (ushort)on;
			break;
		case 6:
			printf( "6:ComParam Detail Display Mode  0/1/2 [%d]<=[%"PRId32"]\n",us_debug_compdispExp, on );
			us_debug_compdispExp = (ushort)on;
			break;
		case 7:
			printf( "7:pcImageImpColor ArrayInfo Display [%d]<=[%"PRId32"]\n",us_debug_dspArrayInfo, on );
			us_debug_dspArrayInfo = (ushort)on;
			break;
		case 8:
			printf( "8:HDD No Restore Mode[%d]<=[%"PRId32"]\n",us_debug_no_restore, on );
			us_debug_no_restore = (ushort)on;
			break;
		case 9:
			printf( "9:MemoryManager Mode[%d]<=[%"PRId32"]\n",us_debug_memorymanager, on );
			us_debug_memorymanager = (ushort)on;
			break;
		case 10:
			printf( "10:HDD Dummy Mode[%d]<=[%"PRId32"]\n",us_debug_dummyHDD, on );
			us_debug_dummyHDD = (ushort)on;
			break;
		case 11:
			printf( "11:HDD Ditail Disp Mode[%d]<=[%"PRId32"]\n",us_debug_HDDdispparm, on );
			us_debug_HDDdispparm = (ushort)on;
			break;
		case 12:
			printf( "12:HDD No Store Mode[%d]<=[%"PRId32"]\n",us_debug_no_store, on );
			us_debug_no_store = (ushort)on;
			break;
		case 13:	// 02/05/20 tsukiji 圧縮調停器デバッグ表示フラグ追加.
			printf( "13:Compress Arbitrator Data Display Mode[%d]<=[%"PRId32"]\n",us_debug_comparbi, on );
			us_debug_comparbi = (ushort)on;
			break;
		case 14:	// 02/05/20 tsukiji 圧縮調停器デバッグ表示フラグ追加.
			printf( "14:HDD Transport Time Display Mode[%d]<=[%"PRId32"]\n",us_debug_HDDTime, on );
			us_debug_HDDTime = (ushort)on;
			break;
		case 15:	// 02/05/29 tsukiji ポート出力portSEL_IR.
			break;
		case 16:	// 02/06/22 tsukiji 異常画像チェック.
			break;
		case 17:	// 02/06/22 tsukiji 異常画像チェック.
			break;
		case 18:
			printf( "18:Capture HDD image & info Mode[%d]<=[%"PRId32"]\n",us_debug_HDDcapture, on );
			us_debug_HDDcapture = (ushort)on;
			break;
		case 19:
			printf( "19:Disable Split Trans Mode Write[%d]<=[%"PRId32"]\n",us_debug_disableSplitWrite, on );
			us_debug_disableSplitWrite = (ushort)on;
			break;
		case 20:
			printf( "20:Image Working Array Log Mode[%d]<=[%"PRId32"]\n",us_debug_WorkingArrayLogMode, on );
			us_debug_WorkingArrayLogMode = (ushort)on;
			debug_Init_workArrayDebugLog( (uint32_t)us_debug_WorkingArrayLogMode );
			if( 0 == us_debug_WorkingArrayLogMode ){
				us_debug_WorkingArrayLog = 0;
			}else{
				us_debug_WorkingArrayLog = 1;
			}
			break;
		case 21:	//  2013/09/02 伊藤史. CQ382618 BufToFile停止問題対策.
			printf( "21:chkDeadLock Log Mode[%d]<=[%"PRId32"]\n",us_debug_chkDeadLock, on );
			us_debug_chkDeadLock = (ushort)on;
			break;
		case 22:	// 2013/09/20 伊藤史. マルチスレッド chkEnableExpand 確保量詳細.
			printf( "22:chkEnableExpand alloc size disp Mode[%d]<=[%"PRId32"]\n",us_debug_chkEnableExpand, on );
			us_debug_chkEnableExpand = (ushort)on;
			break;
		case 24:
			printf( "24:Split Trans Result[%d]<=[%"PRId32"]\n",us_debug_isSplitResult, on );
			us_debug_isSplitResult = (ushort)on;
			break;
		case 25:	// 2013/11/19 伊藤史. 
			printf( "25:disp TransLineMan Mode[%d]<=[%"PRId32"]\n",us_debug_TransLine, on );
			us_debug_TransLine = (ushort)on;
			break;
		case 26:
			printf( "26:force E002Mode[%d]<=[%"PRId32"]\n",us_debug_forceE002, on );
			us_debug_forceE002 = (ushort)on;
			break;
		case 27:
			printf( "27:disp getTrandMode selXXX[%d]<=[%"PRId32"]\n",us_debug_getTransMode, on );
			us_debug_getTransMode = (ushort)on;
			break;
		case 28:			
			printf( "28:Disable Split Trans Mode Read[%d]<=[%"PRId32"]\n",us_debug_disableSplitRead, on );
			us_debug_disableSplitRead = (ushort)on;
			break;
		case 29:	// 2006/02/17 伊藤(史)：R40対策.
			printf( "29:FAX RedZone ScanStop Enable[%d]=>[%"PRId32"]\n", bl_dbg_FAX_RedZone_StopScan, on );
			bl_dbg_FAX_RedZone_StopScan = (Bool)on;
			break;
		case 30:
			printf( "30:Active Image Stblz[%d]<=[%"PRId32"]\n", us_debug_ActiveImgStblz, on );
			us_debug_ActiveImgStblz = (ushort)on;
			break;
		case 31:
			printf( "31:ImageInfo CC_Array num Log [%d]<=[%"PRId32"]\n", us_debug_ImageInfoArraynumLog, on );
			if( (0 == us_debug_ImageInfoArraynumLog) && (0 < on) ){
				dbg_InitImageInfoArraynumLog();
				us_debug_ImageInfoArraynumLog = (ushort)on;
			}
			break;
		case 32:
			printf( "32: disp 2nd Compress Image Size Scalable Log [%d]<=[%"PRId32"]\n", us_debug_2nd_CompressSize_Scalable, on );
			us_debug_2nd_CompressSize_Scalable = (ushort)on;
			dbg_NetScanIfaxTx_setFileNameDisp( (ushort)on );	// 2014/11/29 伊藤史. NETC_NetScanIfaxTx::_setFileNameで送信ファイル名を表示する.
			break;
		case 33:
			printf( "33:Extend DMAResourceLog. Log cnt is 1024 * x = [%"PRId32"]\n",on );
			dbg_extendDMAResourceLog( (ushort)on );
			break;
		case 34:
			printf( "34:disp chkResource call log  [%d]<=[%"PRId32"]\n",us_debug_chkResource,on );
			us_debug_chkResource = (ushort)on;
			break;
		case 35:	// 2014/07/01 国吉(日立JTE) CQ.500968 ログ改善.
			printf( "35:Image Working Array Log Level[%d]<=[%"PRId32"]\n",us_debug_WorkingArrayLog, on );
			us_debug_WorkingArrayLog = (ushort)on;
			break;
		case 36:  // 2014/11/27 伊藤史. デバッグ機能：２次圧縮ファイルメモリを増減.
			printf( "36:2'nd Compress AllocMemory Rate (percent)  [%d]<=[%"PRId32"]\n",us_debug_2ndCompAllocPercent,on );
			us_debug_2ndCompAllocPercent = (ushort)on;
			break;
		case 37:  // 2015/4/4 伊藤史. キャンセルトレース.
			printf( "37:Cancel Trace Log Mode  [%u]<=[%"PRId32"]\n",us_debug_CancelTrace,on );
			us_debug_CancelTrace = (ushort)on;
			break;
		case 38:  // 2015/06/09 伊藤史. デバッグ機能：CQ514118 低解像度 90dpi 以下は最悪圧縮率にする対応を無効.
			printf( "38:Cancel [2'nd Compress Low dpi is worst size]  [%d]<=[%"PRId32"]\n",us_debug_2ndLowDpi100Percent,on );
			us_debug_2ndLowDpi100Percent = (ushort)on;
			break;
		case 40:
			printf( "40:Active Cleaning[%d]<=[%"PRId32"]\n", us_debug_ActiveCleaning, on );
			us_debug_ActiveCleaning = (ushort)on;
			break;
		case 41:	// 2019/11/6 伊藤史. OP_BTS14698 S800J制御ログ追加. 
			printf( "41:Extend debug_Dsp_S800JSeqLog. Log cnt is 1024 * x = [%"PRId32"]\n",on );
			debug_Init_S800JSeqLog( (ushort)on );
			break;
		case 50:
			printf( "50:Stop Communication[%d]<=[%"PRId32"]\n", us_debug_StopCommunication, on );
			us_debug_StopCommunication = (ushort)on;
			break;
		case 90: 
			printf( "90:Extend DebugLog 90. Log cnt is 1024 * x = [%"PRId32"]\n",on );
			extendDebugLowSpeedSeq( (ushort)on );
			break;
		case 92: 
			printf( "92:Extend DebugLog 92. Log cnt is 1024 * x = [%"PRId32"]\n",on );
			extendDebugLowLogSpeedSeq( (ushort)on );
			break;
		case 96: //  02/07/17 tsukiji デバッグ禁止解除.
			printf( "96:DebugLog 90 Disable Log Clear(0:ALL) = [%"PRId32"]\n",on );
			clearDisableDebugLow( (ushort)on );
			break;
		case 97: //  02/07/17 tsukiji デバッグ禁止設定.
			printf( "97:DebugLog 90 Disable Log Set = [%"PRId32"]\n",on );
			setDisableDebugLow( (ushort)on );
			break;
		case 100:	// 02/02/14 morikawa debug.
			msgQLog();
			break;
		// 高速タスクのパラメータ表示オプション↓.
		case 200:
			printf( "200:Input Data Display Mode : [%"PRId32"]\n", on );
			us_debug_dispInputParam = (ushort)on;
			s_HiSpeedTskPara.ul_Mode = mode;
			s_HiSpeedTskPara.ul_On = on;
			THRL_ReqDispHighSpeedTaskPara( &s_HiSpeedTskPara );
			break;
		case 201:
			printf( "201:PCI Input Data Display Mode : [%"PRId32"]\n", on );
			us_debug_dispPciInputParam = (ushort)on;
			s_HiSpeedTskPara.ul_Mode = mode;
			s_HiSpeedTskPara.ul_On = on;
			THRL_ReqDispHighSpeedTaskPara( &s_HiSpeedTskPara );
			break;
		case 202:
			printf( "202:Convert Data Display Mode : [%"PRId32"]\n", on );
			us_debug_dispConvParam = (ushort)on;
			s_HiSpeedTskPara.ul_Mode = mode;
			s_HiSpeedTskPara.ul_On = on;
			THRL_ReqDispHighSpeedTaskPara( &s_HiSpeedTskPara );
			break;
		case 203:
			printf( "203:Output Data Display Mode : [%"PRId32"]\n", on );
			us_debug_dispOutputParam = (ushort)on;
			s_HiSpeedTskPara.ul_Mode = mode;
			s_HiSpeedTskPara.ul_On = on;
			THRL_ReqDispHighSpeedTaskPara( &s_HiSpeedTskPara );
			break;
		case 204:
			printf( "204:DMA04 Data Display Mode : [%"PRId32"]\n", on );
			us_debug_dispDMA04 = (ushort)on;
			s_HiSpeedTskPara.ul_Mode = mode;
			s_HiSpeedTskPara.ul_On = on;
			THRL_ReqDispHighSpeedTaskPara( &s_HiSpeedTskPara );
			break;
		case 206:
			printf( "206:Fax Compress/Expand Data Display Mode : [%"PRId32"]\n", on );
			us_debug_dispFaxCompExpParam = (ushort)on;
			s_HiSpeedTskPara.ul_Mode = mode;
			s_HiSpeedTskPara.ul_On = on;
			THRL_ReqDispHighSpeedTaskPara( &s_HiSpeedTskPara );
			break;
		case 210:
			printf( "210:PCI Output Data Display Mode : [%"PRId32"]\n", on );
			us_debug_dispPciOutputParam = (ushort)on;
			s_HiSpeedTskPara.ul_Mode = mode;
			s_HiSpeedTskPara.ul_On = on;
			THRL_ReqDispHighSpeedTaskPara( &s_HiSpeedTskPara );
			break;
#ifdef DEF_Soc1200	// 2013/07/24 ADS萩原 CQ.381171(Act_66426) SC-SFM-4 サブメモリのファイルメモリ化.
		case 211:
			printf( "211:MainToSubF Data Display Mode : [%"PRId32"]\n", on );
			us_debug_dispMainToSubFParam = (ushort)on;
			s_HiSpeedTskPara.ul_Mode = mode;
			s_HiSpeedTskPara.ul_On = on;
			THRL_ReqDispHighSpeedTaskPara( &s_HiSpeedTskPara );
			break;
		case 212:
			printf( "212:SubFToMain Data Display Mode : [%"PRId32"]\n", on );
			us_debug_dispSubFToMainParam = (ushort)on;
			s_HiSpeedTskPara.ul_Mode = mode;
			s_HiSpeedTskPara.ul_On = on;
			THRL_ReqDispHighSpeedTaskPara( &s_HiSpeedTskPara );
			break;
#endif	// DEF_Soc1200.
		case 213:
			// 2019/2/28 SCD2 伊藤史. EagleL MR1 制御構成変更No.1 BOX画像(KM形式)インポート対応（Eagle世代越リストア).
			printf( "213:Disp KMIRestore BlockSizeConvert Info [%d]<=[%"PRId32"]\n",us_debug_dispBlockSizeCnvInfo, on );
			us_debug_dispBlockSizeCnvInfo = (ushort)on;
			break;

		case 300:
			printf( "300:ForceBlack Mode[%d]<=[%"PRId32"]\n",us_debug_ForceBlack, on );
			us_debug_ForceBlack = (ushort)on;
			break;
//091030 soota 倍率値表示.
		case 400:
			printf( "400:Multithread Ratio Display Mode[%d]<=[%"PRId32"]\n",us_Debug_DispRatio, on );
			us_Debug_DispRatio = (ushort)on;
			break;
		case 500:
			printf( "500: AppInConvDiv IPROM CmmMode/MSMode/ScreenMode Mode Output to ThLog[%d]<=[%"PRId32"]\n",us_Debug_AppInConvDiv_IpRomMode, on );
			us_Debug_AppInConvDiv_IpRomMode = (ushort)on;
			break;
		case 501:
			printf( "501: AppInConvDiv IPROM TableData Output to ThLog[%d]<=[%"PRId32"]\n",us_Debug_AppInConvDiv_IpRomData, on );
			us_Debug_AppInConvDiv_IpRomData = (ushort)on;
			THRL_Drvrif_ReqDumpAppInConvDivIpRomData( us_Debug_AppInConvDiv_IpRomData );
			break;
		case 600:
			printf( "600: CacheInvalidateInfo Output to ThLog[%d]<=[%"PRId32"]\n",us_Debug_CacheInvalidateInfo, on );
			us_Debug_CacheInvalidateInfo = (ushort)on;
			break;
		case 2000:	// 03/12/03 morikawa debug.
			printf("debugtaskImage=%d",debugtaskImage);
			break;
		default:	
			printf( "   3:Compress Data Display Mode            [%d]\n", us_debug_compdisp );
			printf( "   4:Expand Data Display Mode              [%d]\n", us_debug_dispparam );
			printf( "   5:Message Data Display Mode             [%d]\n", us_debug_dismessage );
			printf( "   6:ComParam Detail Display Mode  0/1/2   [%d]\n", us_debug_compdispExp );
			printf( "   7:pcImageImpColor ArrayInfo Display     [%d]\n", us_debug_dspArrayInfo );
			printf( "   8:HDD No Restore Mode                   [%d]\n", us_debug_no_restore);
			printf( "   9:MemoryManager Mode                    [%d]\n", us_debug_memorymanager );
			printf( "  10:HDD Dummy Mode                        [%d]\n", us_debug_dummyHDD );
			printf( "  11:HDD Ditail Disp Mode                  [%d]\n", us_debug_HDDdispparm );
			printf( "  12:HDD No Restore Mode                   [%d]\n", us_debug_no_store );
			printf( "  13:CompArbi Data Display Mode            [%d]\n", us_debug_comparbi );
			printf( "  14:HDD Transport Time Mode               [%d]\n", us_debug_HDDTime );
			printf( "  18:Capture HDD image & info Mode         [%d]\n", us_debug_HDDcapture );
			printf( "  19:Disable Split Trans Mode Write        [%d]\n", us_debug_disableSplitWrite );
			printf( "  20:disp Working Array Log                [%d]\n", us_debug_WorkingArrayLogMode );
			printf( "  21:chkDeadLock Log Mode                  [%d]\n", us_debug_chkDeadLock );
			printf( "  22:chkEnableExpand alloc size disp Mode  [%d]\n", us_debug_chkEnableExpand );
			printf( "  24:Split Trans Result                    [%d]\n", us_debug_isSplitResult );
			printf( "  25:disp TransLineMan Mode                [%d]\n", us_debug_TransLine );
			printf( "  26:force E002Mode                        [%d]\n", us_debug_forceE002 );
			printf( "  27:disp getTrandMode selXXX              [%d]\n", us_debug_getTransMode );
			printf( "  28:Disable Split Trans Mode Read         [%d]\n", us_debug_disableSplitRead );
			printf( "  29:FAX RedZone ScanStop Enable           [%d]\n", bl_dbg_FAX_RedZone_StopScan);
			printf( "  30:Active Image Stblz                    [%d]\n", us_debug_ActiveImgStblz );
			printf( "  31:ImageInfo CC_Array num Log            [%d]\n", us_debug_ImageInfoArraynumLog );
			printf( "  32:2nd Compress ImageSize Scalable Log   [%d]\n", us_debug_2nd_CompressSize_Scalable );
			printf( "  33:Extend DMAResourceLog. Log cnt is %"PRIu32" [%"PRIu32"]\n", ul_MaxDMAResourceLogNum,(ul_MaxDMAResourceLogNum/1024) );
			printf( "  34:disp chkResource call log             [%d]\n", us_debug_chkResource );
			printf( "  35:Working Array Log Level               [%d]\n", us_debug_WorkingArrayLog );
			printf( "  36:2'nd Compress AllocMemory Rate(per)   [%u]\n", us_debug_2ndCompAllocPercent );
			printf( "  37:Cancel Trace Log Mode                 [%u]\n", us_debug_CancelTrace );
			printf( "  38:Cancel [2'nd Compress Low dpi is worst size]  [%d]\n",us_debug_2ndLowDpi100Percent );
			printf( "  40:Active Cleaning                       [%d]\n", us_debug_ActiveCleaning );
			printf( "  41:Extend debug_Dsp_S800JSeqLog. Log cnt is %"PRIu32"   [%"PRIu32"]\n",ui32_S800JSeqLogNum,(ui32_S800JSeqLogNum/1024) );
			printf( "  50:Stop Communication                    [%d]\n", us_debug_StopCommunication );
			printf( "  90:Extend DebugLog 90. Log cnt is %"PRIu32"   [%"PRIu32"]\n",ul_DebugLowSeq_MAX,(ul_DebugLowSeq_MAX/1024) );
			printf( "  92:Extend DebugLog 92. Log cnt is %"PRIu32"   [%"PRIu32"]\n", ul_DebugLowLogSeq_MAX,(ul_DebugLowLogSeq_MAX/1024) );
			printf( "  96:DebugLog 90 Disable Log Clear  \n" );
			printf( "  97:DebugLog 90 Disable Log Set    \n" );
			printf( " 100:msgQLog\n" );
			printf( " 200:Input Data Display Mode               [%d]\n", us_debug_dispInputParam );
			printf( " 201:PCI Input Data Display Mode           [%d]\n", us_debug_dispPciInputParam );
			printf( " 202:Convert Data Display Mode             [%d]\n", us_debug_dispConvParam );
			printf( " 203:Output Data Display Mode              [%d]\n", us_debug_dispOutputParam );
			printf( " 204:DMA04 Data Display Mode               [%d]\n", us_debug_dispDMA04 );
			printf( " 206:Fax Compress/Expand Data Dsp Mode     [%d]\n", us_debug_dispFaxCompExpParam );
			printf( " 210:PCI Output Data Display Mode          [%d]\n", us_debug_dispPciOutputParam );
#ifdef DEF_Soc1200	// 2013/07/24 ADS萩原 CQ.381171(Act_66426) SC-SFM-4 サブメモリのファイルメモリ化.
			printf( " 211:MainToSubF Data Display Mode          [%d]\n", us_debug_dispMainToSubFParam );
			printf( " 212:SubFToMain Data Display Mode          [%d]\n", us_debug_dispSubFToMainParam );
#endif	// DEF_Soc1200.
			printf( " 213:Disp KMIRestore BlockSizeConvert Info [%d]\n", us_debug_dispBlockSizeCnvInfo );		// 2019/2/28 SCD2 伊藤史. EagleL MR1 制御構成変更No.1 BOX画像(KM形式)インポート対応（Eagle世代越リストア).
			printf( " 300:Force Black Mode                      [%d]\n", us_debug_ForceBlack );
			printf( " 400:Multithread Ratio Display Mode        [%d]\n", us_Debug_DispRatio );
			printf( " 500:AppInConvDiv IPROM Mode Output to ThLog      [%d]\n", us_Debug_AppInConvDiv_IpRomMode );
			printf( " 501:AppInConvDiv IPROM TableData Output to ThLog [%d]\n", us_Debug_AppInConvDiv_IpRomData );
			printf( " 600:Dump ChaceInvalidateLog               [%d]\n", us_Debug_CacheInvalidateInfo );
			printf( "2000:debugtaskImage                        [%d]\n", debugtaskImage );
			break;
	}
}
ushort	getDebugHighSpeedSeqNo()
{
	return us_DebugSeqIdx;
}
ushort getDebugHighSpeedSeq( ushort us_No, uint32_t& us_Data ,uint32_t& ul_TkTime )
{
	us_Data = st_DebugSeq[us_No%DEF_DebugSeq_MAX].us_data;
	ul_TkTime = st_DebugSeq[us_No%DEF_DebugSeq_MAX].ul_time;
	return st_DebugSeq[us_No%DEF_DebugSeq_MAX].us_mode;
}
void setDebugHighSpeedSeq( ushort us_Mode, uint32_t us_Data )
{
	st_DebugSeq[us_DebugSeqIdx%DEF_DebugSeq_MAX].us_mode = us_Mode;
	st_DebugSeq[us_DebugSeqIdx%DEF_DebugSeq_MAX].us_data = us_Data;
	st_DebugSeq[us_DebugSeqIdx%DEF_DebugSeq_MAX].ul_time = SYSF_tickGet();
	us_DebugSeqIdx++;
	if( us_Mode == 99 ) setDebugHighError(us_Data);	// エラーカウント.
}
void setDebugHighError( uint32_t us_Data )
{
	st_DebugHighErr[ ul_DebugLogErrCount_High % 10 ].ul_Error = us_Data;
#ifdef TORNADO	// VXWORKSで有効.
	st_DebugHighErr[ ul_DebugLogErrCount_High % 10 ].ul_Time = SYSF_tickGet();
#endif	// #ifdef TORNADO.
	ul_DebugLogErrCount_High++;
}
void clearDebugHighSpeedSeqNo()
{
	us_DebugSeqIdx = 0;
	for( int32_t i=0;i<DEF_DebugSeq_MAX; i++ ) {
		st_DebugSeq[i].us_mode = 0;
		st_DebugSeq[i].us_data = 0;
		st_DebugSeq[i].ul_time = 0;
	}
	ul_DebugLogErrCount_High = 0;	// エラーカウントクリア.
}

void extendDebugLowSpeedSeq(ushort us_cnt)
{
	if( DEF_DebugSeq_MAX != ul_DebugLowSeq_MAX ){
		free((void *)pst_DebugLowSeq);
	}
	if( 0 == us_cnt ){
		pst_DebugLowSeq = st_DebugLowSeq;		// 2014/4/18 伊藤史. debugLog 90 拡張.
		ul_DebugLowSeq_MAX = DEF_DebugSeq_MAX;
		ul_DebugLowSeqIdx = 0;
		bl_DebugLowSeqFlag = False;
	} else {
		ul_DebugLowSeq_MAX = (uint32_t)us_cnt * 1024;
		pst_DebugLowSeq = (ST_DEBUGSEQ *)malloc( ul_DebugLowSeq_MAX * sizeof(ST_DEBUGSEQ) );
		ul_DebugLowSeqIdx = 0;
		bl_DebugLowSeqFlag = False;
	}
}

uint32_t	getDebugLowSpeedSeqNo()
{
	return ul_DebugLowSeqIdx;
}
Bool	getDebugLowSpeedSeqFlag()
{
	return bl_DebugLowSeqFlag;
}
ushort getDebugLowSpeedSeq( uint32_t ul_No, uint32_t& us_Data , uint32_t& ul_TkTime)
{
	us_Data = pst_DebugLowSeq[ul_No%ul_DebugLowSeq_MAX].us_data;
	ul_TkTime = pst_DebugLowSeq[ul_No%ul_DebugLowSeq_MAX].ul_time;
	return pst_DebugLowSeq[ul_No%ul_DebugLowSeq_MAX].us_mode;
}
void setDebugLowSpeedSeq( ushort us_Mode, uintptr_t us_Data )
{
	short ss_Index_Data = (-1);
	if( isDisableDebugLow( us_Mode ) == False ) {
		// デバッグ登録禁止指示されていない場合に登録する.
		pst_DebugLowSeq[ul_DebugLowSeqIdx%ul_DebugLowSeq_MAX].us_mode = us_Mode;
		pst_DebugLowSeq[ul_DebugLowSeqIdx%ul_DebugLowSeq_MAX].us_data = us_Data;
		pst_DebugLowSeq[ul_DebugLowSeqIdx%ul_DebugLowSeq_MAX].ul_time = SYSF_tickGet();;
		ul_DebugLowSeqIdx++;
		if( ul_DebugLowSeqIdx >= ul_DebugLowSeq_MAX ){
			bl_DebugLowSeqFlag = True;
			ul_DebugLowSeqIdx = 0;
		}
// 2010/11/24 UV-00794 デバッグログ収集システム対応 M.Koshimo start.
		// 14/10/21 Sky O.Suzuki [CQ503465]静的解析対応.
		st_DebugLowSeq90[st_debugLog90_Index.us_DebugLowSeqIdx%LIBD_DEF_DebugSeq_MAX].us_mode = us_Mode;
		st_DebugLowSeq90[st_debugLog90_Index.us_DebugLowSeqIdx%LIBD_DEF_DebugSeq_MAX].us_Data = (ushort)us_Data;
		st_DebugLowSeq90[st_debugLog90_Index.us_DebugLowSeqIdx%LIBD_DEF_DebugSeq_MAX].ul_time = SYSF_tickGet();;
		ss_Index_Data = st_debugLog90_Index.us_DebugLowSeqIdx%LIBD_DEF_DebugSeq_MAX;
		st_debugLog90_Index.us_DebugLowSeqIdx++;
// 2010/11/24 UV-00794 デバッグログ収集システム対応 M.Koshimo end.
	}
	if( us_Mode == 999 ) setDebugLowError( us_Data );	// エラーカウント.
	if ( us_Mode == 503 ) ul_DebugLogMemChangeTick = SYSF_tickGet() ;	// Memory Status Change最新Tick 2005/06/06 added by t.sasaki.
// 2010/11/24 UV-00794 デバッグログ収集システム対応 M.Koshimo start.
	if( us_Mode == 999 ) st_debugLog90_Index.ul_DebugLogErrCount_LowLog++;
	if ( us_Mode == 503 ) st_debugLog90_Index.ul_DebugLogMemChangeTick = SYSF_tickGet();
	if( ss_Index_Data > -1 ){
		//インデックス指定.
		short ss_Index = (-1);
		//LogLibＡＰＩをコールし、共有メモリへ書き込み.
		LIBF_WriteSystemLog(LIBD_LogID_THRC_DebugLog90_Index,
							LIBD_LogOutPutLevel1,
							LIBD_LogCategoryCopy|LIBD_LogCategoryPrint|LIBD_LogCategoryBox|LIBD_LogCategoryFax|LIBD_LogCategoryNetFax,
							&st_debugLog90_Index,
							&ss_Index );

		LIBF_WriteSystemLog(LIBD_LogID_THRC_DebugLog90,
							LIBD_LogOutPutLevel1,
							LIBD_LogCategoryCopy|LIBD_LogCategoryPrint|LIBD_LogCategoryBox|LIBD_LogCategoryFax|LIBD_LogCategoryNetFax,
							&st_DebugLowSeq90[ss_Index_Data],
							&ss_Index_Data );
	}
// 2010/11/24 UV-00794 デバッグログ収集システム対応 M.Koshimo end.

}
void setDebugLowError( uintptr_t us_Data )	//2017/01/24 uint32_tからuintptr_tに変更.
{
	st_DebugLowErr[ ul_DebugLogErrCount_Low % 10 ].ul_Error = us_Data;
#ifdef TORNADO	// VXWORKSで有効.
	st_DebugLowErr[ ul_DebugLogErrCount_Low % 10 ].ul_Time = SYSF_tickGet();
#endif	// #ifdef TORNADO.
	ul_DebugLogErrCount_Low++;
}
void setDisableDebugLow( ushort us_Mode )
{
	if( us_Mode > 0 ) {
		// デバッグ登録禁止コード設定.
		ul_DebugLogDisable_Low[ul_DebugLogDisableCount_Low%10] = us_Mode;
		if( ul_DebugLogDisableCount_Low<(10-1)) ul_DebugLogDisableCount_Low++;
	}
}
void clearDisableDebugLow( ushort us_Mode )
{
	if( us_Mode > 0 ) {
		//	VC6 to VS2K8 Porting. HCLT Start.
//		for( int i=0;i<10;i++ ) {.
		int32_t i;
		for( i=0;i<10;i++ ) {
		//	VC6 to VS2K8 Porting. HCLT End.
			if( ul_DebugLogDisable_Low[i] == us_Mode ) {
				if( i==9 ) {
					ul_DebugLogDisable_Low[i] = 0; // クリアのみ.
				} else {
					// テーブル前詰め（一番最後のテーブルは詰めない）.
					//	VC6 to VS2K8 Porting. HCLT Start.
//					for( int j=i;j<9;j++ ) {.
					int32_t j;
					for( j=i;j<9;j++ ) {
					//	VC6 to VS2K8 Porting. HCLT End.
						ul_DebugLogDisable_Low[j] = ul_DebugLogDisable_Low[j+1];
					}
					ul_DebugLogDisable_Low[j] = 0; // 最後クリア.
				}
				if( ul_DebugLogDisableCount_Low>0 ) { ul_DebugLogDisableCount_Low--; }
			}
		}
	} else {
		 // テーブルクリア.
		for( int32_t i=0;i<10;i++ ) {
			ul_DebugLogDisable_Low[i] = 0;
		}
		ul_DebugLogDisableCount_Low = 0;
	}
}
Bool isDisableDebugLow( ushort us_Mode )
{
	Bool bl_Result = False;	// 初期値は禁止なし.
	for( uint32_t i=0;(i<(uint32_t)10)&&(i<ul_DebugLogDisableCount_Low);i++ ) {
		if( ul_DebugLogDisable_Low[i] == us_Mode ) {
			bl_Result = True;
			break;
		}
	}
	return bl_Result;
}
void clearDebugLowSpeedSeqNo()
{
	ul_DebugLowSeqIdx = 0;
	bl_DebugLowSeqFlag = False;
	for( uint32_t i=0;i<ul_DebugLowSeq_MAX; i++ ) {
		pst_DebugLowSeq[i].us_mode = 0;
		pst_DebugLowSeq[i].us_data = 0;
		pst_DebugLowSeq[i].ul_time = 0;
	}
	ul_DebugLogErrCount_Low = 0;	// エラーカウントクリア.
	// デバッグログ収集システム.
	st_debugLog90_Index.us_DebugLowSeqIdx = 0;
	// 14/10/21 Sky O.Suzuki [CQ503465]静的解析対応.
	for( int32_t j=0;j<LIBD_DEF_DebugSeq_MAX; j++ ) {
		st_DebugLowSeq90[j].us_mode = 0;
		st_DebugLowSeq90[j].us_Data = 0;
		st_DebugLowSeq90[j].ul_time = 0;
	}
	st_debugLog90_Index.ul_DebugLogErrCount_LowLog = 0;	// エラーカウントクリア.
}

		
void extendDebugLowLogSpeedSeq(ushort us_cnt)
{
	if( DEF_DebugLog92Seq_MAX != ul_DebugLowLogSeq_MAX ){
		free((void *)pst_DebugLowLogSeq);
	}
	if( 0 == us_cnt ){
		pst_DebugLowLogSeq = st_DebugLowLogSeq;		// 2014/4/18 伊藤史. debugLog 90 拡張.
		ul_DebugLowLogSeq_MAX = DEF_DebugLog92Seq_MAX;
		ul_DebugLowLogSeqIdx = 0;
		bl_DebugLowLogSeqFlag = False;
	} else {
		ul_DebugLowLogSeq_MAX = (uint32_t)us_cnt * 1024;
		pst_DebugLowLogSeq = (ST_DEBUGSEQ *)malloc( ul_DebugLowLogSeq_MAX * sizeof(ST_DEBUGSEQ) );
		ul_DebugLowLogSeqIdx = 0;
		bl_DebugLowLogSeqFlag = False;
	}
}

ushort	getDebugLowLogSpeedSeqNo()
{
	return us_DebugLowLogSeqIdx;
}
Bool	getDebugLowLogSpeedSeqFlag()
{
	return bl_DebugLowLogSeqFlag;
}
ushort getDebugLowLogSpeedSeq( ushort us_No, uint32_t& us_Data , uint32_t& ul_TkTime)
{
	us_Data = pst_DebugLowLogSeq[us_No%ul_DebugLowLogSeq_MAX].us_data;
	ul_TkTime = pst_DebugLowLogSeq[us_No%ul_DebugLowLogSeq_MAX].ul_time;
	return pst_DebugLowLogSeq[us_No%ul_DebugLowLogSeq_MAX].us_mode;
}
void setDebugLowLogSpeedSeq( ushort us_Mode, uintptr_t us_Data )
{
	short ss_Index_Data = (-1);

	if( isDisableDebugLowLog( us_Mode ) == False ) {
		// デバッグ登録禁止指示されていない場合に登録する.
		pst_DebugLowLogSeq[us_DebugLowLogSeqIdx%ul_DebugLowLogSeq_MAX].us_mode = us_Mode;
		pst_DebugLowLogSeq[us_DebugLowLogSeqIdx%ul_DebugLowLogSeq_MAX].us_data = us_Data;
		pst_DebugLowLogSeq[us_DebugLowLogSeqIdx%ul_DebugLowLogSeq_MAX].ul_time = SYSF_tickGet();
		us_DebugLowLogSeqIdx++;
		if( us_DebugLowLogSeqIdx >= ul_DebugLowLogSeq_MAX ){
			bl_DebugLowLogSeqFlag = True;
			ul_DebugLowLogSeqIdx = 0;
		}
// 2010/11/24 UV-00794 デバッグログ収集システム対応 M.Koshimo start.
		st_DebugLowSeq92[st_debugLog92_Index.us_DebugLowLogSeqIdx%DEF_DebugLog92Seq_MAX].us_mode = us_Mode;
		st_DebugLowSeq92[st_debugLog92_Index.us_DebugLowLogSeqIdx%DEF_DebugLog92Seq_MAX].us_Data = (ushort)us_Data;
		st_DebugLowSeq92[st_debugLog92_Index.us_DebugLowLogSeqIdx%DEF_DebugLog92Seq_MAX].ul_time = SYSF_tickGet();
		ss_Index_Data = st_debugLog92_Index.us_DebugLowLogSeqIdx%DEF_DebugLog92Seq_MAX;
		st_debugLog92_Index.us_DebugLowLogSeqIdx++;
// 2010/11/24 UV-00794 デバッグログ収集システム対応 M.Koshimo end.
	}
	if( us_Mode == 999 ) setDebugLowLogError( us_Data );	// エラーカウント.
// 2010/11/24 UV-00794 デバッグログ収集システム対応 M.Koshimo start.
	if( us_Mode == 999 ) st_debugLog92_Index.ul_DebugLogErrCount_LowLog++;
	if( ss_Index_Data > -1 ){
		short ss_Index = (-1);	//インデックス指定.
		//LogLibＡＰＩをコールし、共有メモリへ書き込み.
		LIBF_WriteSystemLog(LIBD_LogID_THRC_DebugLog92_Index, 
							LIBD_LogOutPutLevel1,
							LIBD_LogCategoryCopy|LIBD_LogCategoryPrint|LIBD_LogCategoryBox|LIBD_LogCategoryFax|LIBD_LogCategoryNetFax,
							&st_debugLog92_Index,
							&ss_Index );

		LIBF_WriteSystemLog(LIBD_LogID_THRC_DebugLog92, 
							LIBD_LogOutPutLevel1,
							LIBD_LogCategoryCopy|LIBD_LogCategoryPrint|LIBD_LogCategoryBox|LIBD_LogCategoryFax|LIBD_LogCategoryNetFax,
							&st_DebugLowSeq92[ss_Index_Data],
							&ss_Index_Data );
	}
// 2010/11/24 UV-00794 デバッグログ収集システム対応 M.Koshimo end.
}

/* 2016/01/20 uint32_tからuintptr_tに型変更 */
void setDebugLowLogError( uintptr_t us_Data )
{
	st_DebugLowLogErr[ ul_DebugLogErrCount_LowLog % 10 ].ul_Error = us_Data;
#ifdef TORNADO	// VXWORKSで有効.
	st_DebugLowLogErr[ ul_DebugLogErrCount_LowLog % 10 ].ul_Time = SYSF_tickGet();
#endif	// #ifdef TORNADO.
	ul_DebugLogErrCount_LowLog++;
}
void setDisableDebugLowLog( ushort us_Mode )
{
	if( us_Mode > 0 ) {
		// デバッグ登録禁止コード設定.
		ul_DebugLogDisable_LowLog[ul_DebugLogDisableCount_LowLog%10] = us_Mode;
		if( ul_DebugLogDisableCount_LowLog<(10-1)) ul_DebugLogDisableCount_LowLog++;
	}
}
void clearDisableDebugLowLog( ushort us_Mode )
{
	if( us_Mode > 0 ) {
		for( int32_t i=0;i<10;i++ ) {
			if( ul_DebugLogDisable_LowLog[i] == us_Mode ) {
				if( i==9 ) {
					ul_DebugLogDisable_LowLog[i] = 0; // クリアのみ.
				} else {
					// テーブル前詰め（一番最後のテーブルは詰めない）.
					//	VC6 to VS2K8 Porting. HCLT Start.
//					for( int j=i;j<9;j++ ) {.
					int32_t j;
					for(j=i;j<9;j++ ) {
					//	VC6 to VS2K8 Porting. HCLT Start.
						ul_DebugLogDisable_LowLog[j] = ul_DebugLogDisable_LowLog[j+1];
					}
					ul_DebugLogDisable_LowLog[j] = 0; // 最後クリア.
				}
				if( ul_DebugLogDisableCount_LowLog>0 ) { ul_DebugLogDisableCount_LowLog--; }
			}
		}
	} else {
		 // テーブルクリア.
		for( int32_t i=0;i<10;i++ ) {
			ul_DebugLogDisable_LowLog[i] = 0;
		}
		ul_DebugLogDisableCount_LowLog = 0;
	}
}
Bool isDisableDebugLowLog( ushort us_Mode )
{
	Bool bl_Result = False;	// 初期値は禁止なし.
	for( uint32_t i=0;(i<(uint32_t)10)&&(i<ul_DebugLogDisableCount_LowLog);i++ ) {
		if( ul_DebugLogDisable_LowLog[i] == us_Mode ) {
			bl_Result = True;
			break;
		}
	}
	return bl_Result;
}
void clearDebugLowLogSpeedSeqNo()
{
	us_DebugLowLogSeqIdx = 0;
	for( uint32_t i=0;i<ul_DebugLowLogSeq_MAX; i++ ) {
		pst_DebugLowLogSeq[i].us_mode = 0;
		pst_DebugLowLogSeq[i].us_data = 0;
		pst_DebugLowLogSeq[i].ul_time = 0;
	}
	ul_DebugLogErrCount_LowLog = 0;	// エラーカウントクリア.
	
	// デバッグログ収集システム.
	st_debugLog92_Index.us_DebugLowLogSeqIdx = 0;
	for( uint32_t i=0;i<DEF_DebugLog92Seq_MAX; i++ ) {
		st_DebugLowSeq92[i].us_mode = 0;
		st_DebugLowSeq92[i].us_Data = 0;
		st_DebugLowSeq92[i].ul_time = 0;
	}
	st_debugLog92_Index.ul_DebugLogErrCount_LowLog = 0;	// エラーカウントクリア.
}
// 開始桁数右から（０～８）、取得桁数（１～９）、設定値 02/06/06 tsukiji 関数追加.
uint32_t SetulongdecToField( uchar start, uchar num, uint32_t value )
{
	uint32_t ul_SetData = value % _tenpow( num );
	uint32_t ul_Pos     = 1;
	if( start>0 ) { ul_Pos = _tenpow( start ); }
	return ul_SetData * ul_Pos;
}
// 開始桁数右から（０～８）、取得桁数（１～９）、抜き出す値 02/06/06 tsukiji 関数追加.
uint32_t GetulongdecToField( uchar start, uchar num, uint32_t value )
{
	uint32_t ul_Last   = 0;
	uint32_t ul_Mask   = _tenpow( (uchar)(start+num) );
	if( ul_Mask > 0 ) {
		ul_Last  = value % ul_Mask;		// 上位桁数.
	}
	return ul_Last / _tenpow( start );
}
// １０＾ｎ乗計算(１０乗まで) 02/06/06 tsukiji 関数追加.
uint32_t _tenpow( uchar uc_jyo )
{
	uint32_t ul_Result = 1;
	for( uchar i=0; i<uc_jyo; i++ ) {
		ul_Result = ul_Result * 10;
	}
	return ul_Result;
}

/*================================================================================================
［機能］	評価ボード時間測定ルーチン
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	無し
［ｳｫｰｸｽﾙｰ］	未実施：2000/9/22：森川
================================================================================================*/
void testTime(int32_t step)
{
#ifdef TORNADO		// VXWORKSで有効.
	ul_DebugTime[step] = (uint32_t)SYSF_tickGet();
#endif	// #ifdef TORNADO.
	THRL_ThLogMsgDebug("TimeStep[%"PRId32"] = %"PRId32"\n",step,ul_DebugTime[step]);
}
/*================================================================================================
［機能］	デバッグルーチン
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	無し
［ｳｫｰｸｽﾙｰ］	未実施：2001/11/16：森川
================================================================================================*/
void LogCompImageTask()
{
	THRL_DrvrIf_LocalLogCompImageTask();
}
void LogExpImageTask()
{
	THRL_DrvrIf_LocalLogExpImageTask();
}

extern void HddDebugEnable();
void HddDebugEnable()
{
}
extern void LocalLogHddTask();
extern void LogHddTask();
void LogHddTask()
{
	LocalLogHddTask();
}

/*================================================================================================
［機能］	共有データ「給紙口」の情報取得 
--------------------------------------------------------------------------------------------------
［引数］	無し 
［戻値］	無し
［ｳｫｰｸｽﾙｰ］	未実施
--------------------------------------------------------------------------------------------------
 [履歴]
 '01/12/20 鈴木希 新規作成 
================================================================================================*/
void GetComFeed()
{
#if defined(COSIM) && defined(THR_SIMWIN) // 【COSIM+ｽﾚｯﾄﾞSIMWIN対応】.
#else
	printf( "【共有ﾃﾞｰﾀ「給紙口」ｸﾗｽの情報】\r\n" );
	printf( "\t1段目ｴﾝｼﾞﾝ検出用紙ｻｲｽﾞ x=[%f], y=[%f]\r\n", comFeed.getEnginePaperSize_1().c_SizeMM.x, comFeed.getEnginePaperSize_1().c_SizeMM.y );
	for( register unsigned char i = 0; i < EC_FeedCode_Size; i++ ) {
		printf( "\t=== comFeed[%"PRId32"] ===\r\n", i );
		printf( "\t\t用紙ｻｲｽﾞ x=[%f], y=[%f]\r\n", comFeed[i].getPaperSize().c_SizeMM.x, comFeed[i].getPaperSize().c_SizeMM.y );
		printf( "\t\t用紙種類【0:Normal 1:NotDuplex 2:Special 3:ThinPaper1 4:ThinPaper2 5:ThinPaper3 6:OHP 7:Envelope 8:HighQuality 9:PostCard 10:Label】= [%d]\r\n", comFeed[i].getPaperKind() );
		printf( "\t\tﾊﾟﾈﾙでのｻｲｽﾞ設定 = [%s]\r\n", (comFeed[i].getPanelSizeSet() == True) ? "True " : "False" );
		printf( "\t\t給紙ﾄﾚｲﾀｲﾌﾟ【0:Non 1:Fix 2:Universal 3:MPC 4:LCC 5:LCT 6:MultiMan 7:SingleMan】= [%d]\r\n", comFeed[i].getFeedTrayType() );
	}
#endif // #if defined(COSIM) && defined(THR_SIMWIN) // 【COSIM+ｽﾚｯﾄﾞSIMWIN対応】.
}
/*================================================================================================
［機能］	セマフォロック・アンロックのログを取得する
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	無し
［ｳｫｰｸｽﾙｰ］	未実施
--------------------------------------------------------------------------------------------------
 [履歴] 	'02.07.13	中田	新規追加
================================================================================================*/
#ifdef TORNADO		// VXWORKSで有効.
extern "C" {
	void LogSem();
}
void LogSem()
{
	for( short i = 0; i < DEF_SemphoreIDMax; i++ ) {
		printf( "%3"PRId32" : LockIndi = %d : ID = 0x%04x : tick = %"PRId32"\n", i, bl_SemphoreIndi[i], ss_SemphoreID[i], sl_SemphoreTick[i] );
	}
	printf( "LastIndex = %d\n", ss_SemphoreCtr );
}
#endif	// #ifdef TORNADO		// VXWORKSで有効.

// 07/03/07 morikawa SATA2.
/*================================================================================================
［機能］	ターミナルコマンド（IDE:SOFT RESET）
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	無し
［詳細］	
［ｳｫｰｸｽﾙｰ］	未実施
================================================================================================*/
extern "C" {
	void IDEset( int32_t );
}
void IDEset( int32_t mode )
{
	APIC_HddInfo*	pc_HddInfo = APIC_HddInfo::newInstance();

	if( mode == 0 ) {
		pc_HddInfo->setHddExist(False);
	} else {
		pc_HddInfo->setHddExist(True);
	}
}

/*================================================================================================
［機能］	ターミナルコマンド（IDE:Read DMA）
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	無し
［詳細］	
［ｳｫｰｸｽﾙｰ］	未実施
================================================================================================*/
extern "C" {
	void IDEconnect();
}
void IDEconnect()
{
	printf( "IDE connect\n" );
	sendMessage( TID_HDDTrans, TM_HDDConnectionReq, 0 );	// メッセージ送信「HDD接続要求」.
}

/*================================================================================================
［機能］	ターミナルコマンド（IDE:Read DMA）
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	無し
［詳細］	
［ｳｫｰｸｽﾙｰ］	未実施
================================================================================================*/
void IDEformat()
{
	THRL_ThLogMsgDebug(( "IDE format FUNC_HD_FORMAT_LOGICAL\n" ));
	sendMessage( TID_HDDTrans, TM_HDDFormatReq, (uintptr_t)FUNC_HD_FORMAT_LOGICAL );	// メッセージ送信「HDD接続要求」.
}

/*================================================================================================
	モジュール内の関数宣言(ラインモード)
================================================================================================*/
static Bool checkLineMode( Bool bl_LogOut = True );					//ラインモードチェック 2014/12/17 Iguchi [HeliosL][制御構成変更No.29]生産ライン対応 ログ表示の有効無効を指定する引数を追加.
static uchar convTYPE2LanguageNum( TYPE_Language e_LanguageSel );	//表示言語番号取得処理.
static TYPE_Language convNum2LanguageTYPE( uchar LanguageNum );		//表示言語番号取得処理.
static void RollBackFactoryTime(CT_Time ct_Time);					//SetFactoryTime()エラー時のロールバック.
static Bool CheckFactoryTime( int32_t i_year, int32_t i_mon, int32_t i_day, int32_t i_hour, int32_t i_min, int32_t i_sec );
																	//SetFactoryTime()の日時チェック.
static Bool CheckTimeZone( int32_t i_timeZoneH, int32_t i_timeZoneM );		//SetFactoryTime2()のタイムゾーン範囲チェック.
/*================================================================================================
	モジュール内の変数宣言(ラインモード) インスタンス取得
================================================================================================*/
#if defined(COSIM) && defined(THR_SIMWIN) // 【COSIM+ｽﾚｯﾄﾞSIMWIN対応】.
static APIC_MachineSetting* 		pc_MachineSetting 		= 0;
//static APIC_PrintJobSetting*		pc_PrintJobSetting		= 0;
//static APIC_SystemSetting*			pc_SystemSetting		= 0;
//static APIC_FeedTray*				pc_FeedTray				= 0;
static APIC_TotalCnt *				pc_TotalCnt				= 0;
static APIC_LifeCnt*				pc_LifeCnt				= 0;
static APIC_SystemInfo*				pc_SystemInfo			= 0;
static APIC_PrinterServiceSetting*	pc_PrinterServiceSetting = 0;
static APIC_HddInfo*				pc_HddInfo				= 0;
static APIC_ScannerServiceSetting*	pc_ScanSetting			= 0;
static APIC_SslSetting*				pc_SslSetting			= 0;
static APIC_BootupScreenSetting*	pc_BootupScreen			= 0;
#else
static APIC_MachineSetting* 		pc_MachineSetting 		= APIC_MachineSetting::newInstance();
//static APIC_PrintJobSetting*		pc_PrintJobSetting		= APIC_PrintJobSetting::newInstance();
static APIC_SystemSetting*			pc_SystemSetting		= APIC_SystemSetting::newInstance();
//static APIC_FeedTray*				pc_FeedTray				= APIC_FeedTray::newInstance();
static APIC_TotalCnt *				pc_TotalCnt				= APIC_TotalCnt::newInstance();
static APIC_LifeCnt*				pc_LifeCnt				= APIC_LifeCnt::newInstance();
static APIC_SystemInfo*				pc_SystemInfo			= APIC_SystemInfo::newInstance();
static APIC_PrinterServiceSetting*	pc_PrinterServiceSetting = APIC_PrinterServiceSetting::newInstance();
static APIC_HddInfo*				pc_HddInfo				= APIC_HddInfo::newInstance();
static APIC_ScannerServiceSetting*	pc_ScanSetting			= APIC_ScannerServiceSetting::newInstance();
static APIC_SslSetting*				pc_SslSetting			= APIC_SslSetting::newInstance();
static APIC_BootupScreenSetting*	pc_BootupScreen			= APIC_BootupScreenSetting::newInstance();
#endif // #if defined(COSIM) && defined(THR_SIMWIN) // 【COSIM+ｽﾚｯﾄﾞSIMWIN対応】.

// ターミナルから呼ぶ物はここで記述する.
extern "C" {
void SetMarketingArea( uchar uc_MarketingArea );
void SetShipmentType( int32_t mode, int32_t ShipmentType );
void SetOEMCode( int32_t mode, int32_t OEMCode );
#if defined( DEF_DonauBKCus )			
void SetModelType( ushort mode, ushort ModelType );	// 2010/06/29 hanaoka CQ325164 出荷前検査装置対応.
#endif
void SetLanguage( int32_t Language );
void SetTotalCounter( int32_t Counter, int32_t CounterNum );
void SetPMCounter( int32_t Counter, int32_t CounterNum );
void ClearCounter();
void GetROMVersion( uchar uc_mode );
void GetROMVersion_File(FILE *p_File, uchar uc_mode );
void SetImageStabilizerOFF( int32_t mode );
void GetMachineAdjustmentData();															//マシン調整データ表示.
void SetSerialNumber( char* psc_EngSerialNo, char* psc_IrSerialNo );						//シリアルNo.設定.
void SetSerialNumberENG( char* psc_SerialNo );												//ENGシリアルNo.設定.
void SetSerialNumberIR( char* psc_SerialNo );												//IRシリアルNo.設定.
void SetSerialNumberDF( char* psc_SerialNo );												//DFシリアルNo.設定.
void SetPHNativeData( int32_t mode, int32_t RelativePosition, int32_t PHSkewMotorInit);					//PH固有データ設定.
void GetPrinterAdjustSubRegist( short ss_Type );											//先端プリント位置調整値表示.
void SetPrinterAdjustSubRegist( short ss_Type, short ss_Value );							//先端プリント位置調整値設定.
void GetPrinterAdjustMainRegist( short ss_Tray );											//側端プリント位置調整値表示.
void SetPrinterAdjustMainRegist( short ss_Tray, short ss_Value );							//側端プリント位置調整値設定.
void GetPrinterAdjust2ndSubRegist( short ss_Type );											//両面先端プリント位置調整値表示.
void SetPrinterAdjust2ndSubRegist( short ss_Type, short ss_Value );							//両面先端プリント位置調整値設定.
void GetPrinterAdjust2ndMainRegist( short ss_Tray );										//両面側端プリント位置調整値表示.
void SetPrinterAdjust2ndMainRegist( short ss_Tray, short ss_Value );						//両面側端プリント位置調整値設定.
void SetFactoryTime( int32_t year, int32_t mon, int32_t day, int32_t hour, int32_t min, int32_t sec );				// パネル/RDSの時刻設定.
void SetFactoryTime2( int32_t year, int32_t mon, int32_t day, int32_t hour, int32_t min, int32_t sec , int32_t timeZoneH, int32_t timeZoneM);	// タイムゾーン付きパネル/RDSの時刻設定.
void GetFactoryTime();																		// パネルの時刻取得.
//Str 2011/02/22 ft_kondo 第6回手動マージ. 
void GetPHNativeDataLDK();																	//LD相対距離(K)表示 
void GetPHNativeDataLDC();																	//LD相対距離(C)表示.
void GetPHNativeDataLDM();																	//LD相対距離(M)表示.
void GetPHNativeDataLDY();																	//LD相対距離(Y)表示.
void GetPHNativeDataSKEWC();																//スキュー補正モータ初期設定位置(C)表示.
void GetPHNativeDataSKEWM();																//スキュー補正モータ初期設定位置(M)表示.
void GetPHNativeDataSKEWY();																//スキュー補正モータ初期設定位置(Y)表示.
void SetOwnGeneric( uchar mode );
void GetOwnGeneric();
void CreateMachineCode();
void ans_startGetRequestCode( TYPE_EnablerResult e_EnablerResult, uchar* puc_RequestCode );
void GetMachineCode();
void InstallHDD( uchar uc_mode );
void GetInstallHDDState( void );
void SetOriginalSizeDetection( uchar uc_mode );
void GetOriginalSizeDetection( void );
void CreateSelfCertificate( short ss_Year, char sc_Month, char sc_Day, char sc_Hour, char sc_Minute, char sc_Second );
void GetSelfCertificate( void );
void GetLDAdjustData( uchar uc_mode );
void getCEAuth( void );

void GetADFMainZoom();
void GetADFZoomBack();
void GetADFHomePosition();
void SetADFHomePosition( int32_t l_Mode );

void GetADFZoom();
void GetADF1SideStopPosition();
void GetADF2SideStopPosition();
void GetADFStopPositionCD();
void Get2SideADFStopPositionCD();
#if defined(DEF_ZeusCus) || defined(DEF_ZeusEmu)
short SetWUPCompleteTemperatureChoice(Bool bl_SystemType);
short GetWUPCompleteTemperatureChoice(Bool bl_SystemType);
short SetEstablishedHeatRoller();
short GetEstablishedHeatRoller();
void SetFusingTemperatureHeatRollerPlainpaper( short ss_Value );
void GetFusingTemperatureHeatRollerPlainpaper();
void GetTCRSensorC( short s_Mode );
void GetTCRSensorM( short s_Mode );
void GetTCRSensorY( short s_Mode );
void GetTCRSensorK( short s_Mode );
#elif defined(DEF_MINERVASBK_CUS)	//2015/3/5 MinervaSBK条件追加.
short SetEstablishedHeatRoller();
short GetEstablishedHeatRoller();
void SetFusingTemperatureHeatRollerPlainpaper( short ss_Value );
void GetFusingTemperatureHeatRollerPlainpaper();
void GetTCRSensorK();
#else
void GetTCRSensorC();
void GetTCRSensorM();
void GetTCRSensorY();
void GetTCRSensorK();
#endif // #if defined(DEF_ZeusCus) || defined(DEF_ZeusEmu).

void CheckMemoryRW();
void CheckJbigCmpExp();
void CheckMemoryBusIrToMem();
void CheckMemoryBusMemToPr();
void GetMachineType();
void GetTPMStatus();
void SetMachineType( const char sc_Machine, const char sc_Type );
void ExecuteTPCalibration();
void dispFunction( void );
void activateFunction( void );
void InitializeTPM( void );

#if defined( DEF_A4_Cus )
void Set2ndNICExist(short s_State, short s_Mode);	// CQ521426 Act_518744.
void SetFaxExist( short s_Fax, short s_OnOff);		// CQ521426 Act_518744.
void SetFaxCountryCode( char e_CountryCode);		// CQ531741 FAX仕向けの設定.
#endif // defined( DEF_A4_Cus )
#include	"system/nvd/nvdg/NVDC_JobDJKeyInfo.h"	// CQ531741 JobDJKeyInfoインプリメンテーションヘッダー.
static void deleteFaxScanJournal_Internal(void);	// CQ531741 FAX/SCANジャーナル削除.
//2018/10/23	AR:1051755 eSOL 田川【Sparrow】【生産対応】デザイナーズチョイス切り替えコマンド対応.
#if ( defined( DEF_A4_Cus ) && defined( DEF_SparrowCus ) )
void SetPSDesignersChoice( short s_DensityRevesionTable );
void GetPSDesignersChoice();
#endif // ( defined( DEF_A4_Cus ) && defined( DEF_SparrowCus ) )
void SetLDAdjustData( uchar uc_mode, short ss_Data );
}
/*================================================================================================
［機能］	ラインモードチェック：ラインモード用（各ラインモード用ターミナルコマンドからコール）
--------------------------------------------------------------------------------------------------
［引数］	bl_LogOut	: ログ表示の有効・無効の指定(デフォルトで有効).
						: True (有効).
						: False(無効).	　
［戻値］	Bool 
［詳細］	ラインモードで使用するコマンドでは、表示されるログを読み取って処理を行う為、.
　　　　　　指定外のログが表示されると、期待通りの処理がされない場合がある.
　　　　　　ログ表示の有効・無効を指定する引数を追加し、デフォルトを有効に設定する.
［ｳｫｰｸｽﾙｰ］	未実施
--------------------------------------------------------------------------------------------------
 [履歴]
 '02/02/21 冨安 新規作成 
 '04/07/23 米山 Rhein1移植.
  14/12/17 井口	[HeliosL][制御構成変更_No.29]生産ライン対応 ログ表示の有効・無効を指定する引数を追加.
================================================================================================*/
static Bool checkLineMode( Bool bl_LogOut )
{
#if defined(COSIM) && defined(THR_SIMWIN) // 【COSIM+ｽﾚｯﾄﾞSIMWIN対応】.
#else
	comSystem.bl_FactoryAdjustMode = THRL_Drvrif_GetFactoryAdjustMode();
	if( comSystem.bl_FactoryAdjustMode == False ){
		if( bl_LogOut == True ){
			printf( "\r\n Not Line Mode!!\r\n" );
		}
		return False;
	}
#endif // #if defined(COSIM) && defined(THR_SIMWIN) // 【COSIM+ｽﾚｯﾄﾞSIMWIN対応】.
	return True;
}
/*================================================================================================
［機能］	ターミナルコマンド(マーケティング仕向設定)
--------------------------------------------------------------------------------------------------
［引数］	uchar MarketingArea
［戻値］	無し
［詳細］	
［ｳｫｰｸｽﾙｰ］	未実施
--------------------------------------------------------------------------------------------------
 [履歴]		 
 '08/02/29 古野 新規作成 
 '08/07/25 鈴村 CQ307931_Others5の対応.
  23/07/07 Sky  OP_BTS-44075 制御構成変更_No77_北米仕向だけクラシック画面をマスク.
================================================================================================*/
void SetMarketingArea( uchar uc_MarketingArea )
{
	if( uc_MarketingArea > 7 ){
		printf( " [Format]SetMaketingArea MarketingArea\r\n" );
		printf( " MarketingArea … マーケティング仕向( 0: Japan  1: US  2: Europe  3: Others1  4: Others2  5: Others3  6: Others4  7: Others5 )\r\n" );
		printf( " Last Marketing Area = " );
		if( pc_MachineSetting->getSystemType() != TYPE_SystemType_EU ){
			printf( "%d\r\n", pc_MachineSetting->getSystemType() );
		}else{
			printf( "%d\r\n", TYPE_SystemType_EU + pc_MachineSetting->getSubSystemType());
		}
	} else {
		printf( " Last Marketing Area = " );
		if( pc_MachineSetting->getSystemType() != TYPE_SystemType_EU ){
			printf( "%d\r\n", pc_MachineSetting->getSystemType() );
		}else{
			printf( "%d\r\n", TYPE_SystemType_EU + pc_MachineSetting->getSubSystemType());
		}
		switch( uc_MarketingArea ) {					// マーケティング仕向の設定.
			case 0:	// MJキー.
				pc_MachineSetting->setDestination( TYPE_SystemType_JP );
				break;
			case 1:	// MCキー.
				pc_MachineSetting->setDestination( TYPE_SystemType_US );
				// OP_BTS-44075 制御構成変更_No77_北米仕向だけクラシック画面をマスク.
				{
					APIC_SoftDipSw* pc_SoftDipSw = APIC_SoftDipSw::newInstance();
					if( NULL == pc_SoftDipSw )
					{
						printf("SystemErr\n");
					}
					else
					{
						pc_SoftDipSw->setSoftDipSwBit( 158, 3, True );	// DipSW141-bit5.
					}
				}
				break;
			case 2:	// MHキー.
				pc_MachineSetting->setDestination( TYPE_SystemType_EU , TYPE_SubSystemType1 );
				break;
			case 3: // Others1キー.
				pc_MachineSetting->setDestination( TYPE_SystemType_EU , TYPE_SubSystemType2 );
				break;
			case 4: // Others2キー.
				pc_MachineSetting->setDestination( TYPE_SystemType_EU , TYPE_SubSystemType3 );
				break;
			case 5: // Others3キー.
				pc_MachineSetting->setDestination( TYPE_SystemType_EU , TYPE_SubSystemType4 );
				break;
			case 6: // Others4キー.
				pc_MachineSetting->setDestination( TYPE_SystemType_EU , TYPE_SubSystemType5 );
				break;
			case 7: // Others5キー.
				pc_MachineSetting->setDestination( TYPE_SystemType_EU , TYPE_SubSystemType6 );
				break;
			default:
				break;
		}
		printf( "  New Marketing Area = " );
		if( pc_MachineSetting->getSystemType() != TYPE_SystemType_EU ){
			printf( "%d\r\n", pc_MachineSetting->getSystemType() );
		}else{
			printf( "%d\r\n", TYPE_SystemType_EU + pc_MachineSetting->getSubSystemType());
		}
	}
}

/*================================================================================================
［機能］	ターミナルコマンド（出荷仕向設定）：ラインモード用
--------------------------------------------------------------------------------------------------
［引数］	mode		：モード（０：書き込み準備、１：書き込み指示）
			ShipmentType：出荷仕向（０～８１９１）
［戻値］	無し
［詳細］	
［ｳｫｰｸｽﾙｰ］	未実施
--------------------------------------------------------------------------------------------------
 [履歴]
 '02/02/21 冨安 新規作成 
 '02/03/01 冨安 設定結果受信時の処理用にターミナルフラグのセットを追加。 
 '02/03/12 冨安 出荷仕向の表示を常に３桁の数字で表示するように修正。 
 '03/11/04 松原 modeは0or1のみとなるように条件修正。
 '04/07/23 米山 Rhein1移植.
 '07/03/02 k-kawa Mosel/Thames(最大8191(0x1FFF)対応
================================================================================================*/
void SetShipmentType( int32_t mode, int32_t ShipmentType )
{
#if defined(COSIM) && defined(THR_SIMWIN) // 【COSIM+ｽﾚｯﾄﾞSIMWIN対応】.
#else
	if( checkLineMode() == False ){		// ラインモードかどうかのチェック.
		return;
	}
//	if( ( 0 > ShipmentType || ShipmentType > 999 ).
	if( ( 0 > ShipmentType || ShipmentType > 0x1FFF )
		|| ( 0 > mode || mode > 1 ) ){
		printf( " [Format]SetShipmentType mode,ShipmentType\r\n" );
		printf( " mode         … 準備:0／指示:1\r\n" );
//		printf( " ShipmentType … 出荷仕向( 0 ～ 999 )\r\n" );.
		printf( " ShipmentType … 出荷仕向( 0 ～ 8191 )\r\n" );
		printf( " Last Shipment Type = " );
		printf( "%d\r\n", comSystem.cc_ShipType.us_ResultCode );
	} else {
		if( comSystem.cc_ShipType.bl_Send == True ){
			printf( " Command Timing Error!!" );
		} else {
			if( mode == 1 ){
				printf( " Last Shipment Type = " );
				// 互換性維持のため、従来範囲(0～999)は従来の表記とする.
				if (comSystem.cc_ShipType.us_ResultCode < 1000) {
					printf( "%03d\r\n", comSystem.cc_ShipType.us_ResultCode );
				} else {
					printf( "%05d\r\n", comSystem.cc_ShipType.us_ResultCode );
				}
				comSystem.cc_ShipType.bl_Terminal = True;
			}
			comSystem.cc_ShipType.setShipmentType( ((mode==1)?True:False), (ushort)ShipmentType );
		}
	}
#endif
}
/*================================================================================================
［機能］	ターミナルコマンド（出荷仕向設定）：ラインモード用（エンジンからの結果受信時の処理）
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	無し
［詳細］	
［ｳｫｰｸｽﾙｰ］	未実施
--------------------------------------------------------------------------------------------------
 [履歴]
 '02/02/21 冨安 新規作成 
 '02/03/01 冨安 設定結果受信時の処理用にターミナルフラグのリセットを追加。 
 '02/03/12 冨安 出荷仕向の表示を常に３桁の数字で表示するように修正。 
 '04/07/23 米山 Rhein1移植.
 '06/08/16 宇田 出力内容を仕様書どおり変更 
 '07/03/02 k-kawa Mosel/Thames(最大8191(0x1FFF)対応
================================================================================================*/
void ReportShipmentType()
{
#if defined(COSIM) && defined(THR_SIMWIN) // 【COSIM+ｽﾚｯﾄﾞSIMWIN対応】.
#else
	comSystem.cc_ShipType.bl_Terminal = False;
	printf( "  New Shipment Type = " );
	if( comSystem.cc_ShipType.bl_Result == True ){
		// 互換性維持のため、従来範囲(0～999)は従来の表記とする.
		if (comSystem.cc_ShipType.us_ResultCode < 1000) {
			printf( "%03d\r\n", comSystem.cc_ShipType.us_ResultCode );
		} else {
			printf( "%05d\r\n", comSystem.cc_ShipType.us_ResultCode );
		}
	} else {
		printf( "No Good\r\n" );
	}
#endif
}

/*================================================================================================
［機能］	ターミナルコマンド（ＯＥＭコード設定）：ラインモード用
--------------------------------------------------------------------------------------------------
［引数］	mode		：モード（０：書き込み準備、１：書き込み指示）
			OEMCode：ＯＥＭコード（０～８１９１）
［戻値］	無し
［詳細］	
［ｳｫｰｸｽﾙｰ］	未実施
--------------------------------------------------------------------------------------------------
 [履歴]
 '02/02/21 冨安 新規作成 
 '02/03/01 冨安 設定結果受信時の処理用にターミナルフラグのセットを追加。 
 '02/03/12 冨安 ＯＥＭコードの表示を常に３桁の数字で表示するように修正。 
 '04/07/23 米山 Rhein1移植.
 '07/03/02 k-kawa Mosel/Thames(最大8191(0x1FFF)対応
================================================================================================*/
void SetOEMCode( int32_t mode, int32_t OEMCode )
{
#if defined(COSIM) && defined(THR_SIMWIN) // 【COSIM+ｽﾚｯﾄﾞSIMWIN対応】.
#else
	if( checkLineMode() == False ){		// ラインモードかどうかのチェック.
		return;
	}
	if( 0 > OEMCode || OEMCode > 0x1FFF
	 || ( mode != 0 && mode != 1 ) ){
		printf( " [Format]SetOEMCode mode,OEMCode\r\n" );
		printf( " mode    … 準備:0／指示:1\r\n" );
		printf( " OEMCode … ＯＥＭコード( 0 ～ 8191 )\r\n" );
		printf( " Last OEM Code = " );
		printf( "%d\r\n", comSystem.cc_OEMCode.us_ResultCode );
	} else {
		if( comSystem.cc_OEMCode.bl_Send == True ){
			printf( " Command Timing Error!!" );
		} else {
			if( mode == 1 ){
				printf( " Last OEM Code = " );
				// 互換性維持のため、従来範囲(0～999)は従来の表記とする.
				if (comSystem.cc_OEMCode.us_ResultCode < 1000) {
					printf( "%03d\r\n", comSystem.cc_OEMCode.us_ResultCode );
				} else {
					printf( "%05d\r\n", comSystem.cc_OEMCode.us_ResultCode );
				}
				comSystem.cc_OEMCode.bl_Terminal = True;
			}
			comSystem.cc_OEMCode.setShipmentType( ((mode!=0)?True:False), (ushort)OEMCode );
		}
	}
#endif
}
/*================================================================================================
［機能］	ターミナルコマンド（ＯＥＭコード設定）：ラインモード用（エンジンからの結果受信時の処理）
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	無し
［詳細］	
［ｳｫｰｸｽﾙｰ］	未実施
--------------------------------------------------------------------------------------------------
 [履歴]
 '02/02/21 冨安 新規作成 
 '02/03/01 冨安 設定結果受信時の処理用にターミナルフラグのリセットを追加。 
 '02/03/12 冨安 ＯＥＭコードの表示を常に３桁の数字で表示するように修正。 
 '04/07/23 米山 Rhein1移植.
 '07/03/02 k-kawa Mosel/Thames(最大8191(0x1FFF)対応
================================================================================================*/
void ReportOEMCode()
{
#if defined(COSIM) && defined(THR_SIMWIN) // 【COSIM+ｽﾚｯﾄﾞSIMWIN対応】.
#else
	comSystem.cc_OEMCode.bl_Terminal = False;
	printf( "  New OEM Code = " );
	if( comSystem.cc_OEMCode.bl_Result == True ){
		// 互換性維持のため、従来範囲(0～999)は従来の表記とする.
		if (comSystem.cc_OEMCode.us_ResultCode < 1000) {
			printf( "%03d\r\n", comSystem.cc_OEMCode.us_ResultCode );
		} else {
			printf( "%05d\r\n", comSystem.cc_OEMCode.us_ResultCode );
		}
	} else {
		printf( "No Good\r\n" );
	}
#endif
}
#if defined( DEF_DonauBKCus )
// =================================================================================================
//! <!--機能--> ターミナルコマンド（機種仕向設定）：ラインモード用.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param	mode	  ：モード（０：書き込み準備、１：書き込み指示）.
//! \param 	ModelType ：機種仕向（０～８１９１）.
//! <!--戻値-->
//! \return 無し.
//! <!--詳細-->
//! \n
//! <!--履歴-->
// 2010/06/29 k.hanaoka CQ:325164 新規作成.
// =================================================================================================
void SetModelType( ushort mode, ushort ModelType )
{
	if( checkLineMode() == False ){								// ラインモードかどうかのチェック.
		return;
	}
	if( 0x1FFF < ModelType || ( mode != 0 && mode != 1 ) ){		// 引数のチェック.
		printf( " [Format]SetModelType mode,ModelType\r\n" );
		printf( " mode      … 準備:0／指示:1\r\n" );
		printf( " ModelType … 機種仕向( 0 ～ 8191 )\r\n" );
		printf( " Last Model Type = " );
		printf( "%04d\r\n", comSystem.cc_ModelType.us_ResultCode );
		return;
	}
	if( comSystem.cc_ModelType.bl_Send == True ){				// 送信要求のチェック.
		printf( " Command Timing Error!!" );
		return;
	}
	if( mode == 1 ){											// 書き込みモードのチェック.
		printf( " Last Model Type = " );
		printf( "%04d\r\n", comSystem.cc_ModelType.us_ResultCode );
		comSystem.cc_ModelType.bl_Terminal = True;				// ターミナルフラグ設定.
	}
	comSystem.cc_ModelType.setShipmentType( ((mode!=0)?True:False), ModelType );	// 仕向設定通知関数コール.
}
// =================================================================================================
//! <!--機能--> ターミナルコマンド（機種仕向設定）：ラインモード用（エンジンからの結果受信時の処理）.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param	無し.
//! <!--戻値-->
//! \return 無し.
//! <!--詳細-->
//! \n
//! <!--履歴-->
// 2010/06/29 k.hanaoka CQ:325164 新規作成.
// =================================================================================================
void ReportModelType()
{
	comSystem.cc_ModelType.bl_Terminal = False;				// ターミナルフラグ設定.
	printf( "  New Model Type = " );
	if( comSystem.cc_ModelType.bl_Result == True ){			// 書き込み結果のチェック.
		printf( "%04d\r\n", comSystem.cc_ModelType.us_ResultCode );
	} else {
		printf( "No Good\r\n" );
	}
}
#endif	// #if defined( DEF_DonauBKCus ).
/*================================================================================================
［機能］	表示言語番号取得処理：ラインモード用（void SetLanguage()からcall
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	uchar
［詳細］	
［ｳｫｰｸｽﾙｰ］	未実施
--------------------------------------------------------------------------------------------------
 [履歴]
 '04/07/27 米山 新規作成 
================================================================================================*/
static uchar convTYPE2LanguageNum( TYPE_Language e_LanguageSel )
{
	switch( e_LanguageSel ) {
	case TYPE_LanguageJapanese  	:	return EC_Language_Japanese   ;	break;	// 日本語.
	case TYPE_LanguageAmerican  	:	return EC_Language_American   ;	break;	// 米語.
	case TYPE_LanguageGerman    	:	return EC_Language_German     ;	break;	// ドイツ語.
	case TYPE_LanguageFrench    	:	return EC_Language_French     ;	break;	// フランス語.
	case TYPE_LanguageItalian   	:	return EC_Language_Italian    ;	break;	// イタリア語.
	case TYPE_LanguageDanish    	:	return EC_Language_Danish     ;	break;	// デンマーク語.
	case TYPE_LanguageDutch     	:	return EC_Language_Dutch      ;	break;	// オランダ゛語.
	case TYPE_LanguagePortuguese	:	return EC_Language_Portuguese ;	break;	// ポルトガル語.
	case TYPE_LanguageSwedish   	:	return EC_Language_Swedish    ;	break;	// スウェーデン語.
	case TYPE_LanguageSpanish   	:	return EC_Language_Spanish    ;	break;	// スペイン語.
	case TYPE_LanguageNorwegian 	:	return EC_Language_Norwegian  ;	break;	// ノルウェー語.
	case TYPE_LanguageFinnish   	:	return EC_Language_Finnish    ;	break;	// フィンランド語.
	case TYPE_LanguageCzech     	:	return EC_Language_Czech      ;	break;	// チェコ語.
	case TYPE_LanguageHangarian 	:	return EC_Language_Hangarian  ;	break;	// ハンガリー語.
	case TYPE_LanguageGreek     	:	return EC_Language_Greek      ;	break;	// ギリシア語.
	case TYPE_LanguageTurkish   	:	return EC_Language_Turkish    ;	break;	// トルコ語.
	case TYPE_LanguageChinese1  	:	return EC_Language_Chinese1   ;	break;	// 広東語.
	case TYPE_LanguageChinese2  	:	return EC_Language_Chinese2   ;	break;	// 中国語.
	case TYPE_LanguageHangul    	:	return EC_Language_Hangul     ;	break;	// ハングル語.
	case TYPE_LanguageRussian   	:	return EC_Language_Russian    ;	break;	// ロシア語.
	case TYPE_LanguageCroatian  	:	return EC_Language_Croatian   ;	break;	// クロアチア語.
	case TYPE_LanguageUkrainian 	:	return EC_Language_Ukrainian  ;	break;	// ウクライナ語.
	case TYPE_LanguageRomanian  	:	return EC_Language_Romanian   ;	break;	// ルーマニア語.
	case TYPE_LanguageSlovak    	:	return EC_Language_Slovak     ;	break;	// スロベキア語.
	case TYPE_LanguageArabic    	:	return EC_Language_Arabic     ;	break;	// アラビア語.
	case TYPE_LanguageThai      	:	return EC_Language_Thai       ;	break;	// タイ語.
	case TYPE_LanguageMalaysia  	:	return EC_Language_Malaysia   ;	break;	// マレーシア語.
	case TYPE_LanguagePolish    	:	return EC_Language_Polish     ;	break;	// ポーランド語.
	default							:	return EC_Language_Japanese   ;	break;	// 日本語 デフォルトとする.
	}
}
static TYPE_Language convNum2LanguageTYPE( uchar LanguageNum )
{
	switch( LanguageNum ) {
	case EC_Language_Japanese   	:	return TYPE_LanguageJapanese  ;	break;	// 日本語.
	case EC_Language_American   	:	return TYPE_LanguageAmerican  ;	break;	// 米語.
	case EC_Language_German     	:	return TYPE_LanguageGerman    ;	break;	// ドイツ語.
	case EC_Language_French     	:	return TYPE_LanguageFrench    ;	break;	// フランス語.
	case EC_Language_Italian    	:	return TYPE_LanguageItalian   ;	break;	// イタリア語.
	case EC_Language_Danish     	:	return TYPE_LanguageDanish    ;	break;	// デンマーク語.
	case EC_Language_Dutch      	:	return TYPE_LanguageDutch     ;	break;	// オランダ゛語.
	case EC_Language_Portuguese 	:	return TYPE_LanguagePortuguese;	break;	// ポルトガル語.
	case EC_Language_Swedish    	:	return TYPE_LanguageSwedish   ;	break;	// スウェーデン語.
	case EC_Language_Spanish    	:	return TYPE_LanguageSpanish   ;	break;	// スペイン語.
	case EC_Language_Norwegian  	:	return TYPE_LanguageNorwegian ;	break;	// ノルウェー語.
	case EC_Language_Finnish    	:	return TYPE_LanguageFinnish   ;	break;	// フィンランド語.
	case EC_Language_Czech      	:	return TYPE_LanguageCzech     ;	break;	// チェコ語.
	case EC_Language_Hangarian  	:	return TYPE_LanguageHangarian ;	break;	// ハンガリー語.
	case EC_Language_Greek      	:	return TYPE_LanguageGreek     ;	break;	// ギリシア語.
	case EC_Language_Turkish    	:	return TYPE_LanguageTurkish   ;	break;	// トルコ語.
	case EC_Language_Chinese1   	:	return TYPE_LanguageChinese1  ;	break;	// 広東語.
	case EC_Language_Chinese2   	:	return TYPE_LanguageChinese2  ;	break;	// 中国語.
	case EC_Language_Hangul     	:	return TYPE_LanguageHangul    ;	break;	// ハングル語.
	case EC_Language_Russian    	:	return TYPE_LanguageRussian   ;	break;	// ロシア語.
	case EC_Language_Croatian   	:	return TYPE_LanguageCroatian  ;	break;	// クロアチア語.
	case EC_Language_Ukrainian  	:	return TYPE_LanguageUkrainian ;	break;	// ウクライナ語.
	case EC_Language_Romanian   	:	return TYPE_LanguageRomanian  ;	break;	// ルーマニア語.
	case EC_Language_Slovak     	:	return TYPE_LanguageSlovak    ;	break;	// スロベキア語.
	case EC_Language_Arabic     	:	return TYPE_LanguageArabic    ;	break;	// アラビア語.
	case EC_Language_Thai       	:	return TYPE_LanguageThai      ;	break;	// タイ語.
	case EC_Language_Malaysia   	:	return TYPE_LanguageMalaysia  ;	break;	// マレーシア語.
	case EC_Language_Polish     	:	return TYPE_LanguagePolish    ;	break;	// ポーランド語.
	default                     	:	return TYPE_LanguageJapanese  ;	break;	// 日本語 デフォルトとする.
	}
}
/*================================================================================================
［機能］	ターミナルコマンド（言語設定）：ラインモード用
--------------------------------------------------------------------------------------------------
［引数］	Language：言語
［戻値］	無し
［詳細］	
［ｳｫｰｸｽﾙｰ］	未実施
--------------------------------------------------------------------------------------------------
 [履歴]
 '02/02/21 冨安 新規作成 
 '02/03/07 冨安 設定内容、設定結果の表示をコード表示に変更。 
 '03/10/31 松原 Others1～4追加。 
 '04/07/23 米山 Rhein1移植.
================================================================================================*/
void SetLanguage( int32_t Language )
{
	Bool  SetTypeResult = False;
	TYPE_Language e_LastLanguage = pc_MachineSetting->getLanguage();	// 表示言語選択.

	if( 0 > Language || Language > 13 ){
		printf( " [Format]SetLanguage Language\r\n" );
		printf( " Language … 言語( 0 ～ 13 )\r\n" );
		printf( " Last Language = %d\r\n", convTYPE2LanguageNum(e_LastLanguage) );
		return;
	} else {
		switch( pc_MachineSetting->getSystemType() ){
			case TYPE_SystemType_JP:			// 日本仕向.
				if( Language == EC_Language_Japanese		// 日本語.
				 || Language == EC_Language_American ){		// 英語.
					pc_MachineSetting->setLanguage( convNum2LanguageTYPE((uchar)Language) );
					SetTypeResult = True;
				}
				break;
			case TYPE_SystemType_US:			// ＵＳ仕向.
				if( Language == EC_Language_Japanese		// 日本語.
				 || Language == EC_Language_American		// 英語.
				 || Language == EC_Language_French			// 仏語.
				 || Language == EC_Language_Spanish ){		// スペイン語.
					pc_MachineSetting->setLanguage( convNum2LanguageTYPE((uchar)Language) );
					SetTypeResult = True;
				}
				break;
			case TYPE_SystemType_EU:			// ＥＵ仕向.
				if( Language == EC_Language_Japanese		// 日本語.
				 || Language == EC_Language_American		// 英語.
				 || Language == EC_Language_German			// ドイツ語.
				 || Language == EC_Language_French			// フランス語.
				 || Language == EC_Language_Italian			// イタリア語.
				 || Language == EC_Language_Spanish			// スペイン語.
				  ) 
				{
					pc_MachineSetting->setLanguage( convNum2LanguageTYPE((uchar)Language) );
					SetTypeResult = True;
				}
				break;
			default:	break;
		}
			printf( " Last Language = %d\r\n", convTYPE2LanguageNum(e_LastLanguage) );
		if( SetTypeResult == True ) {
			printf( "  New Language = %d\r\n", convTYPE2LanguageNum(pc_MachineSetting->getLanguage()) );
		} else {
			printf( "  New Language = No Good\r\n" );
		}
	}
}
/*================================================================================================
［機能］	ターミナルコマンド（電子カウンタセット・クリア）：ラインモード用 
--------------------------------------------------------------------------------------------------
［引数］	Counter：カウンタ（０～２７） 
			CounterNum：カウンタ値（０～９、９９） 
［戻値］	無し 
［詳細］	
［ｳｫｰｸｽﾙｰ］	未実施 
--------------------------------------------------------------------------------------------------
 [履歴] 
 '02/02/21 冨安 新規作成 
 '03/11/04 松原 ２色カウンタ追加。 
 '04/07/26 米山 Rhein1移植（作り変え）.
 '06/08/28 N.UDA(REVSONIC) No.66994,No.67273,No.67301 左記の件に対応する修正 
 '08/03/04 鈴村 ↑の修正でトータルカウンタ値の更新が抜けてしまったので追加 
 '12/04/02 宮本 NVRAMからフラッシュメモリへのバックアップ追加
 '14/11/13 福原 CQ.505435対応（ZeusS UV-01178）
================================================================================================*/
	static struct ST_SetTotalCntTbl
	{
		TYPE_TotalCnt e_TotalCntKind;		// KM統合トータルカウンタ種別.
		int32_t           int_CntKind   ;		// コマンド入力種別.
	} st_SetTotalCntTbl[] = {
		{ TYPE_TotalCntCopyTotalBlack     	,	EC_CopyTotalSBlack   },		// コピー・トータル・ブラック.
		{ TYPE_TotalCntCopyTotalFullColor 	,	EC_CopyTotalSColor   },		// コピー・トータル・フルカラー.
		{ TYPE_TotalCntCopyTotal2Color    	,	EC_CopyTotal2Colors  },		// コピー・トータル・２色.
		{ TYPE_TotalCntCopyTotalMonoColor 	,	EC_CopyTotalSMono    },		// コピー・トータル・モノカラー.
		{ TYPE_TotalCntCopyLargeBlack     	,	EC_CopySizeSBlack    },		// コピー・大サイズ・ブラック.
		{ TYPE_TotalCntCopyLargeFullColor 	,	EC_CopySizeSColor    },		// コピー・大サイズ・フルカラー.
		{ TYPE_TotalCntCopyLarge2Color    	,	EC_CopySize2Colors   },		// コピー・大サイズ・２色.
		{ TYPE_TotalCntCopyLargeMonoColor 	,	EC_CopySizeSMono     },		// コピー・大サイズ・モノカラー.
		{ TYPE_TotalCntCopyDuplex         	,	EC_CopyTotalD        },		// コピー・両面.
		{ TYPE_TotalCntPrintTotalBlack    	,	EC_PrintTotalSBlack  },		// プリント・トータル・ブラック.
		{ TYPE_TotalCntPrintTotalFullColor	,	EC_PrintTotalSColor  },		// プリント・トータル・フルカラー.
		{ TYPE_TotalCntPrintTotal2Color   	,	EC_PrintTotal2Colors },		// プリント・トータル・２色.
		{ TYPE_TotalCntPrintLargeBlack    	,	EC_PrintSizeSBlack   },		// プリント・大サイズ・ブラック.
		{ TYPE_TotalCntPrintLargeFullColor	,	EC_PrintSizeSColor   },		// プリント・大サイズ・フルカラー.
		{ TYPE_TotalCntPrintLarge2Color   	,	EC_PrintSize2Colors  },		// プリント・大サイズ・２色.
		{ TYPE_TotalCntPrintDuplex        	,	EC_PrintTotalD       },		// プリント・両面.
		{ TYPE_TotalCntScanPrintTotalFullColor,		EC_ScanPrintTotalFullColor		},	// スキャン／ＦＡＸ・プリント・トータル（フルカラー）.
		{ TYPE_TotalCntScanPrintTotalBlack,			EC_ScanPrintTotalBlack			},	// スキャン／ＦＡＸ・プリント・トータル（ブラック）.
		{ TYPE_TotalCntScanPrintLargeFullColor,		EC_ScanPrintLargeFullColor		},	// スキャン／ＦＡＸ・プリント・大サイズ（フルカラー）.
		{ TYPE_TotalCntScanPrintLargeBlack,			EC_ScanPrintLargeBlack			},	// スキャン／ＦＡＸ・プリント・大サイズ（ブラック）.
		{ TYPE_TotalCntScanTotal,					EC_CntScanTotal					},	// スキャナ／ＦＡＸ・読取・トータル.
		{ TYPE_TotalCntScanLarge,					EC_CntScanLarge					},	// スキャナ／ＦＡＸ・読取・大サイズ.
		{ TYPE_TotalCntFaxSendTotal,				EC_FaxSendTotal					},	// ＦＡＸ・送信枚数.
		{ TYPE_TotalCntFaxReceiveTotal,				EC_FaxReceiveTotal				},	// ＦＡＸ・受信枚数.
		{ TYPE_TotalCntPage,						EC_CntPageTotal					},	// 原稿枚数.
		{ TYPE_TotalCntPaper,						EC_CntPaperTotal				},	// 用紙枚数.
		{ TYPE_TotalCntServiceTotal,				EC_ServiceTotal					},	// サービストータルカウンタ.
		{ TYPE_TotalCntServiceDuplex,				EC_ServiceDuplex				},	// サービストータル両面カウンタ.
		{ TYPE_TotalCntLayout_2in1,					EC_Layout_2in1					},	// Nin1カウンタ(4種類)  2in1.
		{ TYPE_TotalCntLayout_4in1,					EC_Layout_4in1					},	// Nin1カウンタ(4種類)  4in1.
		{ TYPE_TotalCntLayout_Other,				EC_Layout_Other					},	// Nin1カウンタ(4種類)  その他.
		{ TYPE_TotalCntPaperType_Normal,			EC_PaperType_Normal				},	// 用紙種類カウンタ(4種類)　普通紙.
		{ TYPE_TotalCntPaperType_NonDuplex,			EC_PaperType_NonDuplex			},	// 用紙種類カウンタ(4種類)　両面不可紙.
		{ TYPE_TotalCntPaperType_UserSpecify1,		EC_PaperType_UserSpecify1		},	// 用紙種類カウンタ(4種類)　ユーザー指定１.
		{ TYPE_TotalCntPaperType_Other,				EC_PaperType_Other				},	// 用紙種類カウンタ(4種類)　その他.
		{ TYPE_TotalCntLongPaper_Black,				EC_LongPaper_Black				},	// 長尺カウンタ(ブラック).
		{ TYPE_TotalCntLongPaper_FullColor,			EC_LongPaper_FullColor			},	// 長尺カウンタ(フルカラー).
		{ TYPE_TotalCntLongPaper_2Color,			EC_LongPaper_2Color				},	// 長尺カウンタ(単色カラー+2色).
		{ TYPE_TotalCntPaperSizeFullColor_A3,		EC_PaperSizeFullColor_A3		},	// 用紙サイズカウンタ(フルカラー)(10種類) A3.
		{ TYPE_TotalCntPaperSizeFullColor_A4,		EC_PaperSizeFullColor_A4		},	// 用紙サイズカウンタ(フルカラー)(10種類) A4.
		{ TYPE_TotalCntPaperSizeFullColor_B4,		EC_PaperSizeFullColor_B4		},	// 用紙サイズカウンタ(フルカラー)(10種類) B4.
		{ TYPE_TotalCntPaperSizeFullColor_B5,		EC_PaperSizeFullColor_B5		},	// 用紙サイズカウンタ(フルカラー)(10種類) B5.
		{ TYPE_TotalCntPaperSizeFullColor_5_5x8_5,	EC_PaperSizeFullColor_5_5x8_5	},	// 用紙サイズカウンタ(フルカラー)(10種類) 5.5*8.5.
		{ TYPE_TotalCntPaperSizeFullColor_8_5x11,	EC_PaperSizeFullColor_8_5x11	},	// 用紙サイズカウンタ(フルカラー)(10種類) 8.5*11.
		{ TYPE_TotalCntPaperSizeFullColor_8_5x14,	EC_PaperSizeFullColor_8_5x14	},	// 用紙サイズカウンタ(フルカラー)(10種類) 8.5*14.
		{ TYPE_TotalCntPaperSizeFullColor_11x17,	EC_PaperSizeFullColor_11x17		},	// 用紙サイズカウンタ(フルカラー)(10種類) 11*17.
		{ TYPE_TotalCntPaperSizeFullColor_Long,		EC_PaperSizeFullColor_Long		},	// 用紙サイズカウンタ(フルカラー)(10種類) 長尺.
		{ TYPE_TotalCntPaperSizeFullColor_Other,	EC_PaperSizeFullColor_Other		},	// 用紙サイズカウンタ(フルカラー)(10種類) その他.
		{ TYPE_TotalCntPaperSizeBlack_A3,			EC_PaperSizeBlack_A3			},	// 用紙サイズカウンタ(ブラック)(10種類)　A3.
		{ TYPE_TotalCntPaperSizeBlack_A4,			EC_PaperSizeBlack_A4			},	// 用紙サイズカウンタ(ブラック)(10種類)　A4.
		{ TYPE_TotalCntPaperSizeBlack_B4,			EC_PaperSizeBlack_B4			},	// 用紙サイズカウンタ(ブラック)(10種類)　B4.
		{ TYPE_TotalCntPaperSizeBlack_B5,			EC_PaperSizeBlack_B5			},	// 用紙サイズカウンタ(ブラック)(10種類)　B5.
		{ TYPE_TotalCntPaperSizeBlack_5_5x8_5,		EC_PaperSizeBlack_5_5x8_5		},	// 用紙サイズカウンタ(ブラック)(10種類)　5.5*8.5.
		{ TYPE_TotalCntPaperSizeBlack_8x5_11,		EC_PaperSizeBlack_8x5_11		},	// 用紙サイズカウンタ(ブラック)(10種類)　8.5*11.
		{ TYPE_TotalCntPaperSizeBlack_8_5x14,		EC_PaperSizeBlack_8_5x14		},	// 用紙サイズカウンタ(ブラック)(10種類)　8.5*14.
		{ TYPE_TotalCntPaperSizeBlack_11x17,		EC_PaperSizeBlack_11x17			},	// 用紙サイズカウンタ(ブラック)(10種類)　11*17.
		{ TYPE_TotalCntPaperSizeBlack_Long,			EC_PaperSizeBlack_Long			},	// 用紙サイズカウンタ(ブラック)(10種類)　長尺.
		{ TYPE_TotalCntPaperSizeBlack_Other,		EC_PaperSizeBlack_Other			},	// 用紙サイズカウンタ(ブラック)(10種類)　その他.
		{ TYPE_TotalCntPaperSize2Color_A3,			EC_PaperSize2Color_A3			},	// 用紙サイズカウンタ(単色カラー+２色)(10種類)　A3.
		{ TYPE_TotalCntPaperSize2Color_A4,			EC_PaperSize2Color_A4			},	// 用紙サイズカウンタ(単色カラー+２色)(10種類)　A4.
		{ TYPE_TotalCntPaperSize2Color_B4,			EC_PaperSize2Color_B4			},	// 用紙サイズカウンタ(単色カラー+２色)(10種類)　B4.
		{ TYPE_TotalCntPaperSize2Color_B5,			EC_PaperSize2Color_B5			},	// 用紙サイズカウンタ(単色カラー+２色)(10種類)　B5.
		{ TYPE_TotalCntPaperSize2Color_5_5x8_5,		EC_PaperSize2Color_5_5x8_5		},	// 用紙サイズカウンタ(単色カラー+２色)(10種類)　5.5*8.5.
		{ TYPE_TotalCntPaperSize2Color_8_5x11,		EC_PaperSize2Color_8_5x11		},	// 用紙サイズカウンタ(単色カラー+２色)(10種類)　8.5*11.
		{ TYPE_TotalCntPaperSize2Color_8_5x14,		EC_PaperSize2Color_8_5x14		},	// 用紙サイズカウンタ(単色カラー+２色)(10種類)　8.5*14.
		{ TYPE_TotalCntPaperSize2Color_11x17,		EC_PaperSize2Color_11x17		},	// 用紙サイズカウンタ(単色カラー+２色)(10種類)　11*17.
		{ TYPE_TotalCntPaperSize2Color_Long,		EC_PaperSize2Color_Long			},	// 用紙サイズカウンタ(単色カラー+２色)(10種類)　長尺.
		{ TYPE_TotalCntPaperSize2Color_Other,		EC_PaperSize2Color_Other		},	// 用紙サイズカウンタ(単色カラー+２色)(10種類)　その他.
		{ TYPE_TotalCntNetFaxSend_Black,			EC_NetFaxSend_Black				},	// ネットワークＦAX送信枚数カウンタ（ブラック）.
		{ TYPE_TotalCntNetFaxSend_Color,			EC_NetFaxSend_Color				},	// ネットワークFAX送信枚数カウンタ（カラー）.
		{ TYPE_TotalCntScanSend_Black,				EC_ScanSend_Black				},	// スキャナ送信枚数カウンタ（ブラック）.
		{ TYPE_TotalCntScanSend_Color,				EC_ScanSend_Color				},	// スキャナ送信枚数カウンタ（カラー）.
		{ TYPE_TotalCntPrintPage,					EC_PrintNum						},	// 印刷枚数カウンタ. (AR1054221: add)
		{ TYPE_TotalCntTotalDuplex,					EC_BothSideTotal				},	// 両面トータル. (AR1054221: add)
	};

void ans_SetTotalCounter()
{
	bl_Callback = False;
}

void SetTotalCounter( int32_t Counter, int32_t CounterNum )
{
	uint32_t ul_TotalCnt = 0;
	uchar uc_i;

	if( ( 0 > Counter || Counter > 73 )
	|| ( ( 0 > CounterNum || CounterNum > 9 ) && ( CounterNum != 99 ) ) ){
		printf( " [Format]SetTotalCounter Counter,CounterNum\r\n" );
		printf( " Counter    … カウンタ  ( 0 ～ 73 )\r\n" );
		printf( " CounterNum … カウンタ値( 0 ～ 9,99 )\r\n" );
		return;
	}
	
	for( uc_i=0; uc_i<sizeof(st_SetTotalCntTbl)/sizeof(ST_SetTotalCntTbl); uc_i++ ) {
		if( st_SetTotalCntTbl[uc_i].int_CntKind == Counter ) {
			printf( " Last Counter Num = " );
			printf( "%ld\r\n", pc_TotalCnt->get( st_SetTotalCntTbl[uc_i].e_TotalCntKind ) );
			if( CounterNum != 99 ){
				pc_TotalCnt->set( st_SetTotalCntTbl[uc_i].e_TotalCntKind, (uint32_t)CounterNum );
				printf( "  New Counter Num = " );
				printf( "%ld\r\n", pc_TotalCnt->get( st_SetTotalCntTbl[uc_i].e_TotalCntKind ) );
				//0クリア処理がいるか判定する.
				switch ( Counter ) {
					case 17:							//スキャン／ＦＡＸ・プリント・トータル（ブラック）.
						//ＦＡＸ・プリント・トータルを0クリアする.
						pc_TotalCnt->set( TYPE_TotalCntFaxPrintTotal, 0 );
						break;
					case 19:							//スキャン／ＦＡＸ・プリント・大サイズ（ブラック）.
						//ＦＡＸ・プリント・大サイズを0クリアする.
						pc_TotalCnt->set( TYPE_TotalCntFaxPrintLarge, 0 );
						break;
					case 20:							//スキャナ／ＦＡＸ・読取・トータル.
						//ＦＡＸ・読取・トータルを0クリアする.
						pc_TotalCnt->set( TYPE_TotalCntFaxScanTotal, 0 );
						break;
					case 21:							//スキャナ／ＦＡＸ・読取・大サイズ.
						//ＦＡＸ・読取・大サイズを0クリアする.
						pc_TotalCnt->set( TYPE_TotalCntFaxScanLarge, 0 );
						break;
					default:							//対象外.
						break;
				}
			}
			//パネル(12開発 柳川氏より指示)とトータルカウンターの計算式を統一.
			ul_TotalCnt  = pc_TotalCnt->get( TYPE_TotalCntCopyTotalFullColor );			//コピー・トータル・フルカラー.
			ul_TotalCnt += pc_TotalCnt->get( TYPE_TotalCntPrintTotalFullColor );		//プリント・トータル・フルカラー.
			ul_TotalCnt += pc_TotalCnt->get( TYPE_TotalCntCopyTotal2Color );			//コピー・トータル・２色.
			ul_TotalCnt += pc_TotalCnt->get( TYPE_TotalCntCopyTotalMonoColor );			//コピー・トータル・モノカラー.
			ul_TotalCnt += pc_TotalCnt->get( TYPE_TotalCntPrintTotal2Color );			//プリント・トータル・２色.
			ul_TotalCnt += pc_TotalCnt->get( TYPE_TotalCntCopyTotalBlack );				//コピー・トータル・ブラック.
			ul_TotalCnt += pc_TotalCnt->get( TYPE_TotalCntPrintTotalBlack );			//プリント・トータル・ブラック.
			ul_TotalCnt += pc_TotalCnt->get( TYPE_TotalCntFaxPrintTotal );				//ＦＡＸ・プリント・トータル.
			ul_TotalCnt += pc_TotalCnt->get( TYPE_TotalCntScanPrintTotalBlack );		//スキャンプリント・トータル・ブラック.
			ul_TotalCnt += pc_TotalCnt->get( TYPE_TotalCntScanPrintTotalFullColor );	//スキャンプリント・トータル・フルカラー.
			pc_TotalCnt->set( TYPE_TotalCntTotal, ul_TotalCnt );						//トータルカウント値を更新する_20080304鈴村.
			
							// フラッシュメモリへバックアップ.
#ifdef TORNADO	/* VXWORKSで有効 */ 
			EraseNvramBackupArea(0, 0);					// バックアップエリア削除(グループ1).
			EraseNvramBackupArea(0, 1);					// バックアップエリア削除(グループ2).
			bl_Callback = True;
			APIC_NVBackup::newInstance()->NVDataBackup( (void *)ans_SetTotalCounter );
			while( bl_Callback == True ) {
				sleep(1);
			}
#endif	/* #ifdef TORNADO */ 				
			printf( "Total Counter Num = %"PRId32"\r\n", ul_TotalCnt );
			break;		// 指定のカウンタを処理すれば終了！.
		}
	}
}
#if defined(DEF_A4_Cus)
/*================================================================================================
 * ［機能］ Sparrow2AIO確認.
 * -----------------------------------------------------------------------------------------------
 * ［引数］	なし.
 * ［戻値］	Bool	True	：Sparrow2AIO.
 *					False	：Sparrow2AIOではない.
 * ［詳細］ マシンタイプを取得しSparrow2AIOかどうかを返す.
 * ［作成］ Sky.
 * ［履歴］	2023/06/29	OP_BTS-43745 IT6_2.3 Sparrow 給紙トレイ4段対応.
 *================================================================================================*/
static Bool THRF_isSparrow2AIO( void )
{
	// マシンタイプ取得.
	TYPE_MachineType e_MachineType = THRF_GetOriginalMachineTypeInfo();

	switch( e_MachineType )
	{
		case TYPE_MachineTypeSparrow2AIO:
			return True;

		default:
			return False;
	}
}
#endif // defined(DEF_A4_Cus).
/*================================================================================================
［機能］	ターミナルコマンド（ＰＭカウンタセット・クリア）：ラインモード用.
--------------------------------------------------------------------------------------------------
［引数］	Counter：カウンタ（０～４２）.
			CounterNum：カウンタ値（０～９、９９）.
［戻値］	無し.
［詳細］	
［ｳｫｰｸｽﾙｰ］	未実施.
--------------------------------------------------------------------------------------------------
 [履歴]
 '02/02/21 冨安 新規作成.
 '03/11/04 松原 設定可能項目変更。.
 '04/07/26 米山 Rhein1移植（作り変え）.
 '06/09/04 N.UDA(REVSONIC) No.68101 一部対応していないカウンタがある件に対応する修正.
 '08/10/07 鈴村 CQ310200_Donauから設定できるカウンタが変更になった.
 '09/12/10 柄崎 CQ322906 Taiga用のカウンタを追加、エラーチェックを仕向けで切り分けるよう修正.
 '09/12/25 手塚 CQ322906 現像剤回転時間カウンターの種別をTYPE_LifeCntDevC_K_Materialに修正.
 '10/01/15 柄崎 CQ322906 現像ユニット回転時間、現像剤交換回数追加.
 '11/06/13 柄崎 MinervaZeus対応 項目見直しとコメントにIDを付加.
 '16/01/15 馬場 CQ521426 VenusMLK対応 新規項目追加(53)、Read-Only属性追加.
 '18/07/17 開發 【DenebMLK 生産コマンド対応（コマンド追加対応）:機器構成変更_No.15 オゾンフィルタ非搭載対応】.
 '18/07/17 開發 【DenebMLK 生産コマンド対応（コマンド追加対応）:機器構成変更_No.2 各ユニットの新品検出情報の削除】.
 '23/06/29 Sky      OP_BTS-43745 IT6_2.3 Sparrow 給紙トレイ4段対応.
================================================================================================*/
#ifdef DEF_TaigaCus
#define  EC_PMC_MAX				48
#elif  defined(DEF_A4_Cus) // // CQ521426 Act_518744.
#define  EC_PMC_MAX				53
// 2018/11/02 [DenebMLK][AR1055247]トレイ5追加対応 Sky新大阪 DenebMLKの場合のMax値追加 ->.
#elif  defined(DEF_DenebMLKCus)
#define  EC_PMC_MAX				54
// 2018/11/02 [DenebMLK][AR1055247]トレイ5追加対応 Sky新大阪 DenebMLKの場合のMax値追加 <-.
#else
#define  EC_PMC_MAX				42
#endif /*DEF_TaigaCus */
	static struct ST_SetLifeCntTbl
	{
		TYPE_LifeCnt e_LifeCntKind;		// KM統合ライフカウンタ種別.
		int32_t          int_PMCKind  ;		// コマンド入力種別.
	} st_SetLifeCntTbl[] = {
		{ TYPE_LifeCntRollerTray1     	,	EC_PMC_FDRoller1      },	//  9 １段目給紙ローラ.
		{ TYPE_LifeCntRollerTray2     	,	EC_PMC_FDRoller2      },	// 10 ２段目給紙ローラ.
		{ TYPE_LifeCntRollerTray3     	,	EC_PMC_FDRoller3      },	// 11 ３段目給紙ローラ.
		{ TYPE_LifeCntRollerTray4     	,	EC_PMC_FDRoller4      },	// 12 ４段目給紙ローラ.
#if !defined(DEF_DenebMLKCus)	// !DenebMLK.
		{ TYPE_LifeCntRollerTrayLCC1  	,	EC_PMC_LCCParts1      },	// 13 ＬＣＣ部品１.
#endif // !DenebMLK.
		{ TYPE_LifeCntRollerTrayManual 	,	EC_PMC_LCCParts2      },	// 14 ＬＣＣ部品２(<-NG)	// 手差し(変更されている).
		{ TYPE_LifeCntIU_C            	,	EC_PMC_ICC            },	//  0 ＩＵ＿Ｃ.
		{ TYPE_LifeCntIU_M            	,	EC_PMC_ICM            },	//  1 ＩＵ＿Ｍ.
		{ TYPE_LifeCntIU_Y            	,	EC_PMC_ICY            },	//  2 ＩＵ＿Ｙ.
		{ TYPE_LifeCntIU_K            	,	EC_PMC_ICK            },	//  3 ＩＵ＿Ｂｋ.
#if !defined(DEF_DenebMLKCus)	// !DenebMLK.
		{ TYPE_LifeCntFinisher        	,	EC_PMC_Sorter         },	// 17 ソータ・フィニッシャ（排紙ＯＰ）.
		{ TYPE_LifeCntHeatRoller      	,	EC_PMC_FuserUnit      },	// 15 定着ユニット.
#endif // !DenebMLK.
#ifdef DEF_TaigaCus
		{ TYPE_LifeCntOzoneFilter   	,	EC_PMC_OZONFilterMosel},	// 25 オゾンフィルタ(Taiga固有) .
#else
#if !defined(DEF_DenebMLKCus)	// !DenebMLK.
		{ TYPE_LifeCntCleanerFilter   	,	EC_PMC_OZONFilterMosel},	// 25 オゾンフィルタ .
#endif // !DenebMLK.
#endif /* DEF_TaigaCus */
		{ TYPE_LifeCntTtransferRoller 	,	EC_PMC_2ndImgTransfer },	// 23 転写ローラーユニット.
#if !defined(DEF_DenebMLKCus)	// !DenebMLK.
		{ TYPE_LifeCntTtransferBelt   	,	EC_PMC_TransferBelt   },	// 16 転写ベルトユニット(転写ｸﾘｰﾅｰﾕﾆｯﾄ含む).
#endif // !DenebMLK.
		{ TYPE_LifeCntAdfFeed         	,	EC_PMC_ADFFeed        },	// 18 ＡＤＦ給紙.
		{ TYPE_LifeCntAdfReverse      	,	EC_PMC_ADFReverse     },	// 19 ＡＤＦ反転.
		{ TYPE_LifeCntSmokeFilter     	,	EC_PMC_FunenFilter    },	// 26 カラー用噴煙フィルタ.
#if !defined(DEF_DenebMLKCus)	// !DenebMLK.
		{ TYPE_LifeCntIU_C            	,	EC_PMC_IU_C           },	// 31 IU_C回転時間カウンタ／DC_C回転時間カウンタ.
		{ TYPE_LifeCntIU_M            	,	EC_PMC_IU_M           },	// 32 IU_M回転時間カウンタ／DC_M回転時間カウンタ.
		{ TYPE_LifeCntIU_Y            	,	EC_PMC_IU_Y           },	// 33 IU_Y回転時間カウンタ／DC_Y回転時間カウンタ.
#endif // !DenebMLK.
		{ TYPE_LifeCntIU_K            	,	EC_PMC_IU_K           },	// 34 DC_K回転時間カウンタ(ドラムユニット回転時間).
#if !defined(DEF_DenebMLKCus)	// !DenebMLK.
		{ TYPE_LifeCntIU_C_PrintNum   	,	EC_PMC_IU_C_PrintNum  },	// 35 IU_C印字枚数カウンタ／DC_C印字枚数カウンタ.
		{ TYPE_LifeCntIU_M_PrintNum   	,	EC_PMC_IU_M_PrintNum  },	// 36 IU_C印字枚数カウンタ／DC_M印字枚数カウンタ.
		{ TYPE_LifeCntIU_Y_PrintNum   	,	EC_PMC_IU_Y_PrintNum  },	// 37 IU_C印字枚数カウンタ／DC_Y印字枚数カウンタ.
#endif // !DenebMLK.
		{ TYPE_LifeCntDevC_K_PrintNum 	,	EC_PMC_DevC_K_PrintNum},	// 38 DevC_K 印字枚数カウンタ.
		{ TYPE_LifeCntHeatRollerTime  	,	EC_PMC_HeatRollerTime },	// 39 定着回転時間.
#if !defined(DEF_DenebMLKCus)	// !DenebMLK.
		{ TYPE_LifeCntHeatRoller      	,	EC_PMC_HeatRoller     },	// 40 定着印字枚数.
		{ TYPE_LifeCntTtransferBeltTime	,	EC_PMC_BeltTime       },	// 41 転写ベルト回転時間.
		{ TYPE_LifeCntTtransferBelt   	,	EC_PMC_Belt           },	// 42 転写ベルト印字枚数.
#endif // !DenebMLK.
#if defined(DEF_DenebMLKCus)	// DenebMLK.
		{ TYPE_LifeCntPaperPowderCleaner,	EC_PMC_PaperPowderCl  },	// 46 紙粉除去クリーナー.
#else
#ifdef DEF_TaigaCus
		{ TYPE_LifeCntDevC_K_Material	,	EC_PMC_DeveloperTurn  },	// 43 現像剤回転時間カウンター.
		{ TYPE_LifeCntPaperPowderCleaner,	EC_PMC_PaperPowderCl  },	// 46 紙粉除去クリーナー.
		{ TYPE_LifeCntDeveloper_K_Change,	EC_PMC_Developer_K_Ch },	// 47 現像剤交換回数.
		{ TYPE_LifeCntDevelopUnit_K,		EC_PMC_DevelopUnitK   },	// 48 現像ユニット回転時間.
#endif /* DEF_TaigaCus */
#endif // DenebMLK.
#if defined(DEF_A4_Cus) // CQ521426 Act_518744.
        { TYPE_LifeCntIUK_Time          ,	EC_PMC_IUK_Time       },	// 53 イメージングユニット_K回転時間カウンタ.
#endif // defined( DEF_A4_Cus )
// 2018/11/02 [DenebMLK][AR1055247]トレイ5追加対応 Sky新大阪 ->.
#if defined(DEF_DenebMLKCus)	// DenebMLK.
		{ TYPE_LifeCntRollerTray5     	,	EC_PMC_FDRoller5      },	// 54 ５段目給紙ローラ.
#endif // DenebMLK.
// 2018/11/02 [DenebMLK][AR1055247]トレイ5追加対応 Sky新大阪 <-.
	};
#if !defined(DEF_DenebMLKCus)	// !DenebMLK.
	static struct ST_SetTonerChangeCntTbl
	{
		TYPE_TonerChangeCnt e_TonerChangeCntKind;		//KM統合トナー新品カウンタ種別.
		int32_t          		int_PMCKind  ;				//コマンド入力種別.
	} st_SetTonerChangeCntTbl[] = {
		{ TYPE_TonerChangeCntCyan   	, 	EC_PMC_TonerChangeCntC},	// 27 TC新品検出カウンタ(C).
		{ TYPE_TonerChangeCntMagenta   	, 	EC_PMC_TonerChangeCntM},	// 28 TC新品検出カウンタ(M).
		{ TYPE_TonerChangeCntYellow   	, 	EC_PMC_TonerChangeCntY},	// 29 TC新品検出カウンタ(Y).
		{ TYPE_TonerChangeCntBlack   	, 	EC_PMC_TonerChangeCntK},	// 30 TC新品検出カウンタ(K).
	};
#endif // !DenebMLK.
void SetPMCounter( int32_t Counter, int32_t CounterNum )
{
	uchar uc_i;
	if( checkLineMode() == False ){		// ラインモードかどうかのチェック.
		return;
	}
#if defined( DEF_A4_Cus ) // CQ521426 Act_518744.
    // 引数 Counterで指定したものがテーブルに登録されているかを確認する.
    // 既存処理では、引数の正誤判定を範囲チェックで行っているが .
    // DEF_A4_Cusでは今後の拡張性を考慮し、テーブルを検索することで引数の正誤を判定する.
    
    Bool b_ParamError = True;                   // 引数エラーフラグ.
    // ライフカウンタ関連のコマンドか検索する.
    for( uc_i=0; uc_i<sizeof(st_SetLifeCntTbl)/sizeof(ST_SetLifeCntTbl); uc_i++) 
    {
        if( st_SetLifeCntTbl[uc_i].int_PMCKind == Counter ) 
        {
            // カウンターあり.
            b_ParamError = False;    // 引数エラーフラグをクリアする .
            break;
        }else{
            // Nothing to do.
        }
    }
    // トナー新品カウンタ関連のコマンドか検索する.
    for ( uc_i = 0; uc_i < sizeof( st_SetTonerChangeCntTbl ) / sizeof( ST_SetTonerChangeCntTbl ); uc_i++ ) {
			//対応カウンタあり?.
			if ( st_SetTonerChangeCntTbl[uc_i].int_PMCKind == Counter )
            {
                // カウンターあり.
                b_ParamError = False; // 引数エラーフラグをクリアする .
                break;
            }else{
                // Nothing to do.
            }
    }
    
    // カウンター更新値をチェックする  0-9 or 99.
    if( b_ParamError == False && (( 0 > CounterNum || CounterNum > 9 ) && ( CounterNum != 99 )) )
    {
        // 範囲外.
        b_ParamError = True;    // 引数エラーフラグを立てる.
    }
    
    // パラメーター異常の場合　ヘルプを表示する.
    if ( b_ParamError == True ) {
		printf( " [Format]SetPMCounter Counter,CounterNum\r\n" );
		printf( " Counter    … カウンタ  ( 0 ～ 53 )\r\n" );
		printf( " CounterNum … カウンタ値( 0 ～ 9,99 )\r\n" );
	}
#else // defined( DEF_A4_Cus )  // CQ521426 Act_518744.
#if defined(DEF_DenebMLKCus)	// DenebMLK.
    Bool b_ParamError = True;                   // 引数エラーフラグ.
    // ライフカウンタ関連のコマンドか検索する.
    for( uc_i=0; uc_i<sizeof(st_SetLifeCntTbl)/sizeof(ST_SetLifeCntTbl); uc_i++) 
    {
        if( st_SetLifeCntTbl[uc_i].int_PMCKind == Counter ) 
        {
            // カウンターあり.
            b_ParamError = False;    // 引数エラーフラグをクリアする .
            break;
        }else{
            // Nothing to do.
        }
    }
// 2018/11/02 [DenebMLK][AR1055247]トレイ5追加対応 (カウンタ値 46 -> 54に修正) Sky新大阪 ->.
	//カウンタ指定が0～54でないか、カウンタ値が0～9,99以外?.
	if ( ( b_ParamError == True ) || ( ( 0 > Counter || Counter > 54 ) || ( ( 0 > CounterNum || CounterNum > 9 ) && ( CounterNum != 99 ) ) ) ) {
		printf( " [Format]SetPMCounter Counter,CounterNum\r\n" );
		printf( " Counter    … カウンタ  ( 0 ～ 54 )\r\n" );
		printf( " CounterNum … カウンタ値( 0 ～ 9,99 )\r\n" );
	}
// 2018/11/02 [DenebMLK][AR1055247]トレイ5追加対応 (カウンタ値 46 -> 54に修正) Sky新大阪 <-.
#else
	//カウンタ指定が0～42でないか、カウンタ値が0～9,99以外?.
	if ( ( 0 > Counter || Counter > 42 ) || ( ( 0 > CounterNum || CounterNum > 9 ) && ( CounterNum != 99 ) ) ) {
		printf( " [Format]SetPMCounter Counter,CounterNum\r\n" );
		printf( " Counter    … カウンタ  ( 0 ～ 42 )\r\n" );
		printf( " CounterNum … カウンタ値( 0 ～ 9,99 )\r\n" );
	}
#endif // !DenebMLK.
#endif // defined( DEF_A4_Cus )
    else{
		pc_LifeCnt->setDBWriteStatus( False ); // 2013/11/08 次世代アーキテクト 負荷軽減対応 日立 江上 ADD.
		for( uc_i=0; uc_i<sizeof(st_SetLifeCntTbl)/sizeof(ST_SetLifeCntTbl); uc_i++ ) {
			if( st_SetLifeCntTbl[uc_i].int_PMCKind == Counter ) {
				printf( " Last Counter Num = " );
				printf( "%ld\r\n", pc_LifeCnt->get( st_SetLifeCntTbl[uc_i].e_LifeCntKind ) );
				if( CounterNum != 99 ){
					pc_LifeCnt->set( st_SetLifeCntTbl[uc_i].e_LifeCntKind, (uint32_t)CounterNum );
					printf( "  New Counter Num = " );
					printf( "%ld\r\n", pc_LifeCnt->get( st_SetLifeCntTbl[uc_i].e_LifeCntKind ) );
				}
			}
		}
		pc_LifeCnt->setDBWriteStatus( True ); // 2013/11/08 次世代アーキテクト 負荷軽減対応 日立 江上 ADD.
#if !defined(DEF_DenebMLKCus)	// !DenebMLK.
		//トナー新品カウンタ（ＡＰＩ）・インスタンス取得.
		APIC_TonerChangeCnt*	pc_TonerChangeCnt = APIC_TonerChangeCnt::newInstance();
		//トナー新品カウンタ関連のコマンドか検索する.
		for ( uc_i = 0; uc_i < sizeof( st_SetTonerChangeCntTbl ) / sizeof( ST_SetTonerChangeCntTbl ); uc_i++ ) {
			//対応カウンタあり?.
			if ( st_SetTonerChangeCntTbl[uc_i].int_PMCKind == Counter ) {
				printf( " Last Counter Num = " );
				printf( "%ld\r\n", pc_TonerChangeCnt->get( st_SetTonerChangeCntTbl[uc_i].e_TonerChangeCntKind ) );
				//参照モードでない?.
				if ( CounterNum != 99 ) {
					pc_TonerChangeCnt->set( st_SetTonerChangeCntTbl[uc_i].e_TonerChangeCntKind, (uint32_t)CounterNum );
					printf( "  New Counter Num = " );
					printf( "%ld\r\n", pc_TonerChangeCnt->get( st_SetTonerChangeCntTbl[uc_i].e_TonerChangeCntKind ) );
				}
			}
		}
#endif // !DenebMLK.
	}
}
/*================================================================================================
［機能］	ターミナルコマンド（各種カウンタクリア）：ラインモード用
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	無し
［詳細］	
［ｳｫｰｸｽﾙｰ］	未実施
--------------------------------------------------------------------------------------------------
 [履歴]
 '02/02/21 冨安 新規作成 
 '02/08/20 冨安 出荷前検査装置で、rebootのカウンタがクリアされない不具合対策の為、ABORTカウンタのクリアを追加した。
 '03/11/04 松原 カバレッジカウンタのクリア追加。 
 '04/07/26 米山 Rhein1移植（作り変え）.
 '06/09/04 山本 サービスモード時に参照される用紙サイズ別カウンタのクリア処理追加 
 '06/11/02 杉本 トラブルトータルカウンタとジャムトータルカウンタは未使用のため、削除 
 '09/09/15 柄崎 CQ:320521 ゴミカウンタのクリア処理追加.
 '12/09/11 福原 CQ:371845 時系列電源瞬断データのクリア処理追加.
================================================================================================*/
void ClearCounter()
{
#if defined(COSIM) && defined(THR_SIMWIN) // 【COSIM+ｽﾚｯﾄﾞSIMWIN対応】.
#else

	APIC_PaperSizeCnt::newInstance()->clearAll();			//概要：用紙サイズ別カウンタ（ＡＰＩ）.
	APIC_TroubleCnt::newInstance()->clearAll();				//概要：トラブルカウンタ（ＡＰＩ）.
//	APIC_TroubleTotalCnt::newInstance()->clearAll();		//概要：トラブルトータルカウンタ（ＡＰＩ）.
	APIC_JamCnt::newInstance()->clearAll();					//概要：ジャムカウンタ（ＡＰＩ）.
//	APIC_JamTotalCnt::newInstance()->clearAll();			//概要：ジャムトータルカウンタ（ＡＰＩ）.
	APIC_WarningCnt::newInstance()->clearAll();				//概要：警告カウンタ（ＡＰＩ）.
	APIC_PmCnt::newInstance()->clearAll();					//概要：ＰＭカウンタ（ＡＰＩ）.
	APIC_FaxErrorCnt::newInstance()->clearAll();			//概要：ＦＡＸ通信エラーカウンタ（ＡＰＩ）.
	APIC_CopyModeCnt::newInstance()->clearAll();			//概要：コピーモード別カウンタ（ＡＰＩ）.
	APIC_PartsCnt::newInstance()->clearAll();				//概要：部品別カウンタ（ＡＰＩ）.
	APIC_ReuseCnt::newInstance()->clearAll();				//概要：リユースカウンタ（ＡＰＩ）.
	APIC_AdfCnt::newInstance()->clearAll();					//概要：ＡＤＦ原稿通紙枚数カウンタ（ＡＰＩ）.
	APIC_FaxCnt::newInstance()->clearAll();					//概要：ＦＡＸカウンタ（ＡＰＩ）.
	APIC_CompressRateCnt::newInstance()->clearAll();		//概要：圧縮率カウンタ（ＡＰＩ）.
	APIC_CoverageCnt::newInstance()->clearAll();			//概要：カバレッジカウンタ（ＡＰＩ）.
	APIC_LogJam::newInstance()->clearAll();					//概要：時系列ＪＡＭデータ（ＡＰＩ）.
	APIC_LogTrouble::newInstance()->clearAll();				//概要：時系列トラブルデータ（ＡＰＩ）.
	APIC_LogRdsJnlDataModem::newInstance()->clearAll();		//概要：ＲＤＳジャーナル・データモデム（ＡＰＩ）.
	APIC_LogRdsJnlEmail::newInstance()->clearAll();			//概要：ＲＤＳジャーナル・Ｅ－ｍａｉｌ（ＡＰＩ）.
	APIC_PaperSizeCntCSRC::newInstance()->clearAll();		//概要：用紙サイズ別カウンタ（ＡＰＩ）.
	if( pc_SystemInfo->isDSModel() == True ){
		APIC_DustCnt::newInstance()->clearAll();			//概要：ゴミカウンタ（ＡＰＩ）.
	}
	APIC_LogMomentaryPowerOff::newInstance()->clearAll();	//概要：時系列電源瞬断データ（ＡＰＩ）.
	printf( " Counter Clear OK!\r\n" );
#endif // #if defined(COSIM) && defined(THR_SIMWIN) // 【COSIM+ｽﾚｯﾄﾞSIMWIN対応】.
}
/*================================================================================================
［機能］	ターミナルコマンド（ＲＯＭバージョン表示）：ラインモード用 
--------------------------------------------------------------------------------------------------
［引数］	uc_mode		：モード（１：CheckSum値表示） 
［戻値］	無し 
［詳細］	
［ｳｫｰｸｽﾙｰ］	未実施 
--------------------------------------------------------------------------------------------------
 [履歴]
 '02/02/21 冨安 新規作成 
 '04/07/26 米山 Rhein1移植（作り変え）.
 '06/09/01 N.UDA(REVSONIC) No.68121 GetROMVersionの一部のバージョン表示がおかしい件に対応する修正 
 '06/10/20 杉本 GetROMVersionのMFPControllerRomPrinterFontのバージョン表示がおかしい件に対応する修正 
 '07/12/20 砂見 CheckSum表示対応.
 '08/05/20 R.Suzuki CQ.303569 i-Optionのマージ漏れ分の追加(LoadableDriver).
 '08/09/02 古野 CQ309029 Amur新規対応.DSC、SD-509、ドライバ名称カスタマイズの追加.
 '08/09/22 鈴村 CQ309549音声データ、動画データのVer追加.
 '08/10/18 古野 CQ310452 OEMドライバ名称カスタマイズの表示を仕様書にあわせる修正.
 '08/10/30 古野 CQ308403 SerchablePDF,OCRLibのVer追加.
 '10/04/21 柄崎 Gaia先行カードバージョン表示対応 st_GetRomVerTbl[]にMFP Card Version を追加。
 '10/11/05 R.Suzuki CQ.334955 MFPControllerRomPrinterFontのバージョンの書き換えをやめる.
 '11/02/23 高山 MinervaZeus 不要モジュールのコメントアウト.
 '11/06/13 柄崎 MinervaZeus Engine parameterをMinervaのみ表示.
 '11/07/29 小霜 CQ.352143 GetROMVersionのチェックサムが4桁でなく3桁で表示される。.
 '12/03/13 小霜 エンジンBOOT追加対応.
 '12/05/17 福原 音源データバージョン表示追加対応.
 '12/06/15 福原 ZeusMLK対応 音源データバージョン表示追加対応.
 '12/06/15 福原 ZeusMLK Engine parameter 表示されない対応.
 '12/08/08 鈴木 CCD変更対応.2012/10/02 福原 マージ.
 '13/02/12 北澤 CQ377711対応(ZeusBK用表示項目追加).
 '13/06/03 大橋 MinBK2nd 制御構成変更_No.52 PKI用ローダブルドライバ－のバージョンを表示する対応.
 '13/07/16 福原 【ZeusCD】1ビーム/2ビーム画パラファイル切り替え対応.
 '14/10/17 有澤 Poseidon 制御構成変更＿No.244 ROMバージョン項目追加削除対応　「SCB」追加.
 '14/10/31 小倉 CQ.503538 表示項目の整理.
 '14/10/31 有澤 Poseidon 機器構成変更_No.92 ROMバージョン表示対応　「Power Sub CPU」追加.
 '14/12/10 井柳 ZeusS 機器構成変更_No.53 ROMバージョン表示対応　「Panel CPU」追加.
 '14/12/11 井柳 ZeusS 機器構成変更_No.54 ROMバージョン表示対応　「SATA Controller」追加.
 '14/12/23 井柳 HeliosL 機器構成変更_No.30 ROMバージョン表示対応　以下の11項目追加.
 '			「PDF/Aフォント」「BootUp Screen」「マシン画像」「SD」「RU」「ZU」「OCRフォント」.
 '			「Unicodeフォント 1」「Unicodeフォント 2」「Unicodeフォント 3」「Unicodeフォント 4」.
 '15/04/23 山下 MinervaSBK 機器構成変更_No.47 ドライバ一本化による、統合FWバージョン表示対応　「MFP Integration Version」追加.
 '15/04/30 大岡 MinervaSBK 機器構成変更_No.20 【FS-537】PKPIのROMバージョン表示の追加.
 '15/08/25 大岡 ZeusSZX0 RQ-257 認証関連の市場要望対応への即応化.
 '15/09/28 古市 MinervaSBK CQ.517448 統合FWバージョン表示不具合対応.
 '16/09/08 Sky合屋 IT5_4.0 制御構成変更No.48 新旧CCD切り替え対応.
 '17/03/08 Sky合屋 IT5_4.0 AR1016652 IRのROMバージョンが表示されない問題の対応.
 '18/02/08 Sky内藤 IT6 Eagle 制御構成変更No.2 ROMバージョン対応.
 '18/06/25 Sky小久保 IT6 Eagle 制御構成変更_No.89 表示順序変更/メッセージ名変更.
 '18/10/31 Sky松井 [Eagle_High]RQ-272 T.38Fax LibraryとSIP Libraryの項目を追加.
 '18/11/12 Sky松井 IT6_DenebMLK	AR.1064408「MFP Controller Image Reader Controller」「CCID Driver」「PC/SC Lite Driver」.
											の仕様修正に伴う修正.
 '18/12/17 Sky [Eagle_High]「Power Control CPU1」「Power Control CPU2」「Loadable Driver1」「Loadable Driver2」に表示名変更.
 '19/04/18 Sky鳥居 [Eagle_High]OP_BTS-3030_GetROMVersionでPIが表示されない問題対応.
 '19/05/23 Sky鳥居 [EagleBK]EAGLEBK-154 コンパイルSW削除、機種固有処理削除対応.
 '19/07/11 Sky [Eagle_High]「Power Control CPU 1」「Power Control CPU 2」に表示名変更.
 '22/11/11 Sky 制御構成変更No.51_サービスレスメンテナンス対応(Serviceless maintenance guide files追加)
 '23/02/15 Sky OP_BTS-41804 Content Data修正対応.
================================================================================================*/


#if defined(COSIM) && defined(THR_SIMWIN) // 【COSIM+ｽﾚｯﾄﾞSIMWIN対応】.
#else
	static struct ST_GetRomVerTbl
	{
		char                  *c_message;
		TYPE_Rom              e_Rom;					// KM統合ＲＯＭ種類.
		TYPE_MFPControllerRom e_MFPControllerRom;		// KM統合ＭＦＰコントローラ内のサブＲＯＭ種類.
	} st_GetRomVerTbl[] = {
		{ "MFP Card Version                           ", TYPE_RomCardVersion              , (TYPE_MFPControllerRom)-1            },
		{ "MFP Controller BOOT Program                ", TYPE_RomMFPControllerBoot        , (TYPE_MFPControllerRom)-1            },	
		{ "MFP Controller Linux                       ", TYPE_RomLinux                    , (TYPE_MFPControllerRom)-1            },	
		{ "MFP Controller Kernel Program              ", TYPE_RomKernel                   , (TYPE_MFPControllerRom)-1            },	
		{ "MFP Controller SubSet                      ", TYPE_RomMFPControllerSubset      , (TYPE_MFPControllerRom)-1            },	
		{ "MFP Controller                             ", TYPE_RomMFPController            , (TYPE_MFPControllerRom)-1            },	
		{ "MFP Controller System                      ", (TYPE_Rom)-1                     , TYPE_MFPControllerRomSystem          },	
		{ "MFP Controller Print Controller            ", (TYPE_Rom)-1                     , TYPE_MFPControllerRomPrinter         },	
		{ "MFP Controller Print Controller Font       ", (TYPE_Rom)-1                     , TYPE_MFPControllerRomPrinterFont     },	
		{ "MFP Controller Print Controller Demo Page  ", (TYPE_Rom)-1                     , TYPE_MFPControllerRomPrinterDemoPage },	
		{ "MFP Controller Image Reader Controller     ", TYPE_RomImageReaderController    , (TYPE_MFPControllerRom)-1            },	// comment 2014/04/08 OPS1-Sky鈴木理 [Zeus5BK] 制御構成変更_No.97 CPS分離対応.
		{ "MFP Controller MIO                         ", (TYPE_Rom)-1                     , TYPE_MFPControllerRomMio             },	
		{ "MFP Controller PIC                         ", (TYPE_Rom)-1                     , TYPE_MFPControllerRomPIC             },	
		{ "MFP Controller PIC Image Parameter         ", TYPE_ImageParamPIC               , (TYPE_MFPControllerRom)-1            },
		{ "MFP Controller Thread Image Parameter      ", TYPE_ImageParamThread            , (TYPE_MFPControllerRom)-1            },
		{ "MFP Controller IR Image Parameter          ", TYPE_IRImageParameter            , (TYPE_MFPControllerRom)-1            },
		{ "MFP Controller Operation Panel Message Data", TYPE_RomMFPControllerPanelMessage, (TYPE_MFPControllerRom)-1            },	
		{ "MFP Controller OSL                         ", TYPE_RomOSS_OSL                  , (TYPE_MFPControllerRom)-1            },// 2019/01/30 SCD2-motohashi [OP_BTS-1285][RQ-676] OSSバージョンの追加.
		{ "MFP Controller CHR                         ", TYPE_RomOSS_CHR                  , (TYPE_MFPControllerRom)-1            },// 2019/01/30 SCD2-motohashi [OP_BTS-1285][RQ-676] OSSバージョンの追加.
		{ "MFP Controller NDJ                         ", TYPE_RomOSS_NDJ                  , (TYPE_MFPControllerRom)-1            },// 2019/01/30 SCD2-motohashi [OP_BTS-1285][RQ-676] OSSバージョンの追加.
		{ "MFP Controller UPL                         ", TYPE_RomOSS_UPL                  , (TYPE_MFPControllerRom)-1            },
		{ "Panel CPU                                  ", TYPE_RomLCDV                     , (TYPE_MFPControllerRom)-1            },
		{ "Power Control CPU 1                        ", TYPE_RomPowerControlCPU1         , (TYPE_MFPControllerRom)-1            },	// 2018/02/08 SCD2-Sky内藤 []制御構成変更_No.2 ROMバージョン対応.
		{ "Power Control CPU 2                        ", TYPE_RomPowerControlCPU2         , (TYPE_MFPControllerRom)-1            },	// 2018/02/08 SCD2-Sky内藤 []制御構成変更_No.2 ROMバージョン対応.
		{ "SSD Controller                             ", TYPE_RomSSDController            , (TYPE_MFPControllerRom)-1            },	// 2018/02/08 SCD2-Sky内藤 []制御構成変更_No.2 ROMバージョン対応.
		{ "Machine Controller                         ", TYPE_RomMachineController        , (TYPE_MFPControllerRom)-1            },	// comment 2014/04/08 OPS1-Sky鈴木理 [Zeus5BK] 制御構成変更_No.97 CPS分離対応.
		{ "Machine Controller BOOT                    ", TYPE_RomMachineControllerBoot    , (TYPE_MFPControllerRom)-1            },	// comment 2014/04/08 OPS1-Sky鈴木理 [Zeus5BK] 制御構成変更_No.97 CPS分離対応.
		{ "Machine Controller Printer                 ", TYPE_RomMachineControllerPrinter , (TYPE_MFPControllerRom)-1            },	// comment 2014/04/08 OPS1-Sky鈴木理 [Zeus5BK] 制御構成変更_No.97 CPS分離対応.
		{ "Machine Controller Media                   ", TYPE_RomMachineControllerMedia   , (TYPE_MFPControllerRom)-1            },	// comment 2014/04/08 OPS1-Sky鈴木理 [Zeus5BK] 制御構成変更_No.97 CPS分離対応.
		{ "Machine Controller Scanner                 ", TYPE_RomMachineControllerScanner , (TYPE_MFPControllerRom)-1            },	// comment 2014/04/08 OPS1-Sky鈴木理 [Zeus5BK] 制御構成変更_No.97 CPS分離対応.
		{ "ADF                                        ", TYPE_RomADF                      , (TYPE_MFPControllerRom)-1            },	
		{ "Finisher                                   ", TYPE_RomFinisher                 , (TYPE_MFPControllerRom)-1            },	
		{ "SD                                         ", TYPE_RomSaddleUnit               , (TYPE_MFPControllerRom)-1            },	
		{ "PK                                         ", TYPE_RomPK                       , (TYPE_MFPControllerRom)-1            },	
		{ "PI                                         ", TYPE_RomPI                       , (TYPE_MFPControllerRom)-1            },	
		{ "Fax Board Controller1                      ", TYPE_RomFax                      , (TYPE_MFPControllerRom)-1            },	
		{ "Fax Board Controller2                      ", TYPE_RomFax2                     , (TYPE_MFPControllerRom)-1            },	
		{ "Fax Board Controller3                      ", TYPE_RomFax3                     , (TYPE_MFPControllerRom)-1            },	
		{ "Fax Board Controller4                      ", TYPE_RomFax4                     , (TYPE_MFPControllerRom)-1            },	
		{ "DSC1                                       ", TYPE_RomDSC                      , (TYPE_MFPControllerRom)-1            },	
		{ "DSC2                                       ", TYPE_RomDSC2                     , (TYPE_MFPControllerRom)-1            },	
		{ "Loadable Driver1                           ", TYPE_RomDownloadableDriver1      , (TYPE_MFPControllerRom)-1            },	
		{ "Loadable Driver2                           ", TYPE_RomDownloadableDriver2      , (TYPE_MFPControllerRom)-1            },	
		{ "CCID Driver                                ", TYPE_RomDownloadableReaderDriver , (TYPE_MFPControllerRom)-1            },	
		{ "PC/SC Lite Driver                          ", TYPE_RomDownloadablePcscLite     , (TYPE_MFPControllerRom)-1            },	
		{ "PKCS#11 Library                            ", TYPE_RomDownloadablePkcs11Lib    , (TYPE_MFPControllerRom)-1            },	
		{ "PKCS#11 Library Config                     ", TYPE_RomDownloadablePkcs11Cfg    , (TYPE_MFPControllerRom)-1            },	
		{ "Integrated Printer Driver Version          ", TYPE_RomIntegrationFW            , (TYPE_MFPControllerRom)-1            },	// comment 2015/04/23 OPS1-Sky山下 [MinervaSBK] 制御構成変更_No.47 統合FWバージョン表示対応.
		{ "Movie Data                                 ", TYPE_RomAnimation                , (TYPE_MFPControllerRom)-1            },	
		{ "Voice Data                                 ", TYPE_RomVoice                    , (TYPE_MFPControllerRom)-1            },	
		{ "PDF/A Font                                 ", TYPE_RomPDFAFont                 , (TYPE_MFPControllerRom)-1            },
		{ "OCR Library                                ", TYPE_RomOCRLib                   , (TYPE_MFPControllerRom)-1            },
		{ "OCR Dictionary                             ", TYPE_RomSearchablePDFDictionaryData, (TYPE_MFPControllerRom)-1          },
		{ "OCR Font                                   ", TYPE_RomOCRFont                  , (TYPE_MFPControllerRom)-1            },
		{ "Unicode Font1                              ", TYPE_RomUnicodeFont1             , (TYPE_MFPControllerRom)-1            },
		{ "Unicode Font2                              ", TYPE_RomUnicodeFont2             , (TYPE_MFPControllerRom)-1            },
		{ "Unicode Font3                              ", TYPE_RomUnicodeFont3             , (TYPE_MFPControllerRom)-1            },
		{ "Unicode Font4                              ", TYPE_RomUnicodeFont4             , (TYPE_MFPControllerRom)-1            },
		{ "Panel Sound Data                           ", TYPE_RomPanelSoundData           , (TYPE_MFPControllerRom)-1            },
		{ "Machine Image                              ", TYPE_RomOEMMachineFigData        , (TYPE_MFPControllerRom)-1            },
		{ "BootUp Screen                              ", TYPE_RomBootupScreen             , (TYPE_MFPControllerRom)-1            },
		{ "Custom Auth Setting                        ", TYPE_RomMasterConfigSetting      , (TYPE_MFPControllerRom)-1            },
		{ "Conversion File                            ", TYPE_RomOEMNameCustomize         , (TYPE_MFPControllerRom)-1            },
		{ "T.38 Library                               ", TYPE_RomT38Library               , (TYPE_MFPControllerRom)-1            },
		{ "SIP Library                                ", TYPE_RomSipLibrary               , (TYPE_MFPControllerRom)-1            },
		{ "Content Data                               ", TYPE_RomGuideFiles               , (TYPE_MFPControllerRom)-1            }	// comment 2022/11/11 Sky 制御構成変更No.51_サービスレスメンテナンス対応(Serviceless maintenance guide files追加).
		// 表示項目数が71を超える場合はKM\shlib\LogLib\LIBC_LogLibDefine.hの構造体LIBS_ST_DebugDirectCommandのメンバc_ROMVertion_StrinData[71][90+1]の配列数を増やすこと.
	};
#endif

#define	EC_GetROMVersion_Kokunai	"001"	//出荷仕向けが国内の場合に仕向けコードにセットする文字列.
#define	EC_GetROMVersion_Kokugai	"0Y0"	//出荷仕向けが国外の場合に仕向けコードにセットする文字列.

void GetRFSVersion();

//関数	:	GetROMVersion()
//履歴	:	2012/12/20	ADS吉田		CQ.377858  MS-LCL_9 不要変数削除対応.
//									・st_DebugDirectCommand.c_ROMVertion_StrinData ⇒ union_DebugDirectCommand.st_ROMVertion.c_ROMVertion_StrinDataへ変更.
//									・構造体のメンバ初期化処理を共用体の初期化処理に変更.
//									・THRL_DebugDirectCommandWriteSystemLog()の追加.
//  		2013/04/19	ADS坪井		CQ.379397(Act_64292)  MS-EVP-69 <静的解析対応>
//									 ・strcatをstrncatに修正.
//			2018/06/05	Sky小久保	Eagle 御構成変更_No.89 生産コマンド対応 表示を別処理として追加/イコールの追加 "MFP RFS"項は削除.
//			2019/07/16 Sky OP_BTS-4631 ログ出力タイミング修正の再検討.
//			2021/05/26 Sky OP_BTS-32644 バージョンに改行がある場合は、改行を削除するように修正.
void GetROMVersion( uchar uc_mode )
{
	GetROMVersion_File(stdout, uc_mode); // 既存動作は標準出力.
}
void GetROMVersion_File(FILE *p_File, uchar uc_mode )
{
	if( NULL==p_File )
	{
		p_File = stdout; // ファイルポインタがNULLであれば標準出力に切り替える.
	}

#if defined(COSIM) && defined(THR_SIMWIN) // 【COSIM+ｽﾚｯﾄﾞSIMWIN対応】.
#else
	uchar uc_i;
	char* pch_find;					// 削除文字列発見ポインタ.
	char ch_Delstr[] = "(00)";		// 削除する文字列.
	char ch_Delstr_Cr[] = "\n";		// 削除する文字列.
	char ch_ROMstr[40+1];
	ushort us_CheckSum = 0;
	char sc_machine = -1;			// マシンNo.
	char sc_type = -1;				// エンジンタイプ.
// 2010/11/24 UV-00794 デバッグログ収集システム対応 M.Koshimo start.
	char ch_strdata[45+1];
	memset(ch_strdata,0,sizeof(ch_strdata));
	memset(&union_DebugDirectCommand, 0, sizeof(union_DebugDirectCommand));
// 2010/11/24 UV-00794 デバッグログ収集システム対応 M.Koshimo end.
//	→ 静的解析対応(CIM283091) 2011/05/23 saka.
	memset( ch_ROMstr, 0, sizeof(ch_ROMstr) );
//	← 静的解析対応.
	THRF_getMachineType(&sc_machine, &sc_type);

	for( uc_i=0; uc_i<sizeof(st_GetRomVerTbl)/sizeof(ST_GetRomVerTbl); uc_i++ ) {
// 2022/03/30 生産RINNE対応 Sky start.
		SYSD_WEBAPI_LINEMODE_PUSH(" %s",st_GetRomVerTbl[uc_i].c_message );
// 2022/03/30 生産RINNE対応 Sky end.

		fprintf(p_File, " %s",st_GetRomVerTbl[uc_i].c_message );
		// デバッグログ収集システム.
		sprintf( union_DebugDirectCommand.st_ROMVertion.c_ROMVertion_StrinData[uc_i], " %s",st_GetRomVerTbl[uc_i].c_message );
		if( st_GetRomVerTbl[uc_i].e_Rom != (-1) ) {
			pc_SystemInfo->getRomVersion( st_GetRomVerTbl[uc_i].e_Rom , ch_ROMstr );
			if( ( TYPE_RomDownloadableReaderDriver != st_GetRomVerTbl[uc_i].e_Rom )
			 && ( TYPE_RomDownloadablePcscLite     != st_GetRomVerTbl[uc_i].e_Rom )
			 && ( TYPE_RomDownloadablePkcs11Lib    != st_GetRomVerTbl[uc_i].e_Rom )
			 && ( TYPE_RomDownloadablePkcs11Cfg    != st_GetRomVerTbl[uc_i].e_Rom )
			 && ( TYPE_RomIntegrationFW            != st_GetRomVerTbl[uc_i].e_Rom )		// comment 2015/04/23 OPS1-Sky山下 [MinervaSBK] 制御構成変更_No.47 統合FWバージョン表示対応.
			 && ( TYPE_RomMasterConfigSetting	   != st_GetRomVerTbl[uc_i].e_Rom ) 	// comment 2015/08/21 OPS1-Sky大岡 [ZeusSZX0] RQ-257 認証関連の市場要望対応への即応化.
			 && ( TYPE_RomSSDController	  		   != st_GetRomVerTbl[uc_i].e_Rom ) ) {
				us_CheckSum = pc_SystemInfo->getRomVersion( st_GetRomVerTbl[uc_i].e_Rom );
			}
		} 
		else if( st_GetRomVerTbl[uc_i].e_MFPControllerRom != (-1) ) {
			pc_SystemInfo->getMFPControllerRomVersion( st_GetRomVerTbl[uc_i].e_MFPControllerRom , ch_ROMstr );
		}

		pch_find = strstr( ch_ROMstr, ch_Delstr_Cr );	// ROMVer文字列から削除文字列(改行)に該当する先頭のポインタ取得.
		if( pch_find != NULL ) {
			*pch_find = '\0';		// 発見したポインタに'\0'を埋め込む.
		}
		pch_find = strstr( ch_ROMstr, ch_Delstr );	// ROMVer文字列から削除文字列に該当する先頭のポインタ取得.
		if( pch_find != NULL ) {
			*pch_find = '\0';		// 発見したポインタに'\0'を埋め込む.
		}

		if( ( uc_mode == 1 &&  st_GetRomVerTbl[uc_i].e_Rom != (-1) ) && ch_ROMstr[0] != 0 ){
			if( ( TYPE_RomDownloadableReaderDriver != st_GetRomVerTbl[uc_i].e_Rom )
			 && ( TYPE_RomDownloadablePcscLite     != st_GetRomVerTbl[uc_i].e_Rom )
			 && ( TYPE_RomDownloadablePkcs11Lib    != st_GetRomVerTbl[uc_i].e_Rom )
			 && ( TYPE_RomDownloadablePkcs11Cfg    != st_GetRomVerTbl[uc_i].e_Rom )
			 && ( TYPE_RomIntegrationFW            != st_GetRomVerTbl[uc_i].e_Rom )		// comment 2015/04/23 OPS1-Sky山下 [MinervaSBK] 制御構成変更_No.47 統合FWバージョン表示対応.
			 && ( TYPE_RomMasterConfigSetting	   != st_GetRomVerTbl[uc_i].e_Rom )	// comment 2015/08/21 OPS1-Sky大岡 [ZeusSZX0] RQ-257 認証関連の市場要望対応への即応化.
			 && ( TYPE_RomSSDController	  		   != st_GetRomVerTbl[uc_i].e_Rom ) ) {
				// デバッグログ収集システム.
				sprintf( ch_strdata,"%-28s %4X\n", ch_ROMstr, us_CheckSum );
			}
			else{
				// デバッグログ収集システム.
				sprintf( ch_strdata,"%s\r\n", ch_ROMstr );
			}
		}
		else{
			// デバッグログ収集システム.
			sprintf( ch_strdata,"%s\r\n", ch_ROMstr );
		}
		if( ch_ROMstr[0] != 0 ){							// ROMVersionなしの場合イコール不要.
// 2022/03/30 生産RINNE対応 Sky start.
			SYSD_WEBAPI_LINEMODE_PUSH("= %-28s\n", ch_ROMstr );
// 2022/03/30 生産RINNE対応 Sky end.
			fprintf(p_File, "= %-28s\n", ch_ROMstr );
		} else {
// 2022/03/30 生産RINNE対応 Sky start.
			SYSD_WEBAPI_LINEMODE_PUSH("\r\n" );
// 2022/03/30 生産RINNE対応 Sky end.
			fprintf(p_File, "\r\n" );
		}
		size_t t_BuffSize = sizeof( union_DebugDirectCommand.st_ROMVertion.c_ROMVertion_StrinData[uc_i] );
		size_t t_strLen = strnlen( union_DebugDirectCommand.st_ROMVertion.c_ROMVertion_StrinData[uc_i], t_BuffSize );
		strncat( union_DebugDirectCommand.st_ROMVertion.c_ROMVertion_StrinData[uc_i], ch_strdata, t_BuffSize - t_strLen );
		union_DebugDirectCommand.st_ROMVertion.c_ROMVertion_StrinData[uc_i][t_BuffSize-1] = '\0'; // 終端を設定する.

		us_CheckSum = 0;
	}
	

	// 共有メモリへの書き込み.
	THRL_DebugDirectCommandWriteSystemLog( LIBD_LogID_THRC_DebugDirectCommand_ROMVertion );
#endif
}

void GetRFSVersion()
{
	FILE	*pF_File = 0;
	char	sc_Buff[DEF_CMD_MAX_CNT];
	int32_t		i = 0;	size_t si_Size = 0;	//201701/24 si_Sizeをint32_tからsize_t

	pF_File = SYSF_fopen( "/usr/version", "rb" );
	memset( sc_Buff , 0 , sizeof( sc_Buff ) );
	if( NULL != pF_File ){
		//indexファイル読み込み.
		for (i = 0;i < DEF_CARD_VERSION_LENGTH_MAX;i++) {
			si_Size = fread( &sc_Buff[i] , 1, 1 , pF_File );// １文字読込.
			// 読込文字数がなくなれば終了.
			if( si_Size == 0 ){
				break;
			}
			if( ( sc_Buff[ i ] == ( char )DEF_INDEX_CR )		// CR
			||( sc_Buff[ i ] == ( char )DEF_INDEX_LF ) ){		// LF
				break;
			}
		}
		SYSF_fclose(pF_File);
	}
	printf( " MFP RFS                                    %s\n",sc_Buff ); // 先頭中間スペース追加 GetROMVersionの書式にあわせる 鈴木 '12/08/08.
}

/*================================================================================================
［機能］	ターミナルコマンド（画像安定化処理ＯＦＦ設定）：ラインモード用 
--------------------------------------------------------------------------------------------------
［引数］	mode：ＯＮ／ＯＦＦ設定（　０：ＯＦＦ　１：ＯＮ　） 
［戻値］	無し 
［詳細］	
［ｳｫｰｸｽﾙｰ］	未実施 
--------------------------------------------------------------------------------------------------
 [履歴]
 '02/02/21 冨安 新規作成 
 '04/07/26 米山 Rhein1移植.
 '06/08/21 N.UDA(REVSONIC) No.66734 設定内容を仕様書どおり変更 
================================================================================================*/
void SetImageStabilizerOFF( int32_t mode )
{
#if defined(COSIM) && defined(THR_SIMWIN) // 【COSIM+ｽﾚｯﾄﾞSIMWIN対応】.
#else
	if( 0 > mode || mode > 1 ){
		printf( " [Format]SetImageStabilizerOFF mode\r\n" );
		printf( " mode … OFF: 0 / ON: 1\r\n" );
		printf( " Last ImageStabilizerOFF = " );
		printf( "%d\r\n", comSystem.bl_NulliImgStblz );
	} else {
		comSystem.bl_NulliImgStblz = (Bool)mode;						// 画像安定化処理ＯＦＦ設定 (True=安定化無効(安定化OFF)).
	}
#endif
}
/*================================================================================================ 
［機能］	ターミナルコマンド（マシン調整データ表示）：ラインモード用 
-------------------------------------------------------------------------------------------------- 
［引数］	無し 
［戻値］	無し 
［詳細］	
［ｳｫｰｸｽﾙｰ］	未実施 
-------------------------------------------------------------------------------------------------- 
 [履歴] 
 '06/05/29 山本(F) 新規作成 
  '06/08/22 山本(F) 「厚紙1」「厚紙1+」と表示していたのを「厚紙1/1+」に修正 
 '08/09/03 furuno CQ309029 Amur対応.「給紙口毎のトップマージン調整」に対応.パラメータ8～11を追加.
                              「LD相対位置補正」はAmurでは非表示.
                               Donau:厚紙1/1+,Amur:厚紙1.
 11/06/13 Karasaki MinervaZeus対応 DEF_AmurCusを削除。
 					厚紙1/1+統一、薄紙追加、トレイをZeus、LDHをMinervaで表示.
 18/01/30 Tagawa AR:1033007 Sparrow 制御構成変更No.335 マシン調整データ表示内容変更. 
 18/07/17 Kaihotsu 【DenebMLK 生産コマンド対応（コマンド追加対応）:制御構成変更_No.45 給紙カセット対応】.
 19/06/26 Sky鳥居	[EagleH]OP_BTS-9105	TopMargin1stTrayLの文字列を修正.
 23/06/29 Sky		OP_BTS-43745 IT6_2.3 Sparrow 給紙トレイ4段対応.
================================================================================================*/ 
#define	SS_PaperTypeNameLength	(20+1)		//全角５文字まで⇒Amur対応にて10文字に変更.
//Str[Sparrow].
#if defined(DEF_A4_Cus)
#define	SS_PaperTypeKind3		12
#define	SS_TrayTypeKind3		5
#if !defined( DEF_SparrowAIOSFPCus )// Sparrow MFP.
#define	SS_PaperTypeKind		12
#define	SS_TrayTypeKind			4
#else					// Sparrow AIO/SFP.
#define	SS_PaperTypeKind		11
#define	SS_TrayTypeKind			3
#endif	// #if !defined( DEF_SparrowAIOSFPCus ) .

//プリント位置・先端に使用.
typedef struct {
	TYPE_PrinterAdjust 	e_PaperType;							//用紙種類.
	char				sc_DispStr[SS_PaperTypeNameLength];		//表示文字列.
} ST_ADJUSTDATADISPLAY_PAPER;
ST_ADJUSTDATADISPLAY_PAPER st_DisplayTable[SS_PaperTypeKind]
							 = {	{TYPE_PrinterAdjustSubRegistNormal,		"普通紙"},
									{TYPE_PrinterAdjustSubRegistThin,		"薄紙"},
									{TYPE_PrinterAdjustSubRegistOHP, 		"OHP"},
									{TYPE_PrinterAdjustSubRegistHeavy1, 	"厚紙1/1+"},
									{TYPE_PrinterAdjustSubRegistHeavy2, 	"厚紙2"},
									{TYPE_PrinterAdjustSubRegistHeavy3, 	"厚紙3"},
									{TYPE_PrinterAdjustSubRegistHeavy4, 	"厚紙4"},
									{TYPE_PrinterAdjustSubRegistEnvelope, 	"封筒"}	,
									{TYPE_PrinterAdjustTopMargin1stTrayS, 	"トレイ1(short)"},
									{TYPE_PrinterAdjustTopMargin1stTrayL, 	"トレイ1(int32_t)"},
#if !defined( DEF_SparrowAIOSFPCus ) //Sparrow AIO/Sparrow2 SFPは対象外.
									{TYPE_PrinterAdjustTopMargin2nd3rd4thLCTTray, 	"トレイ2/3/4"},
#endif // #if !define() //Sparrow AIO/Sparrow2 SFPは対象外.
									{TYPE_PrinterAdjustTopMarginManualTray, 	"手差し"}
								};
ST_ADJUSTDATADISPLAY_PAPER st_DisplayTable3[SS_PaperTypeKind3]
							 = {	{TYPE_PrinterAdjustSubRegistNormal,		"普通紙"},
									{TYPE_PrinterAdjustSubRegistThin,		"薄紙"},
									{TYPE_PrinterAdjustSubRegistOHP, 		"OHP"},
									{TYPE_PrinterAdjustSubRegistHeavy1, 	"厚紙1/1+"},
									{TYPE_PrinterAdjustSubRegistHeavy2, 	"厚紙2"},
									{TYPE_PrinterAdjustSubRegistHeavy3, 	"厚紙3"},
									{TYPE_PrinterAdjustSubRegistHeavy4, 	"厚紙4"},
									{TYPE_PrinterAdjustSubRegistEnvelope, 	"封筒"}	,
									{TYPE_PrinterAdjustTopMargin1stTrayS, 	"トレイ1(short)"},
									{TYPE_PrinterAdjustTopMargin1stTrayL, 	"トレイ1(int32_t)"},
									{TYPE_PrinterAdjustTopMargin2nd3rd4thLCTTray, 	"トレイ2/3/4"},
									{TYPE_PrinterAdjustTopMarginManualTray, 	"手差し"}
								};
//プリント位置・側端に使用.
typedef struct {
	TYPE_PrinterAdjust 	e_FeedTrayType;							//給紙口種類.
	char				sc_DispStr[SS_PaperTypeNameLength];		//表示文字列.
} ST_ADJUSTDATADISPLAY_TRAY;
//プリント位置側面用テーブル.
ST_ADJUSTDATADISPLAY_TRAY st_DisplayTableTrayS[SS_TrayTypeKind]
							 = {	{TYPE_PrinterAdjustMainRegist1st,		"1段目"},
									{TYPE_PrinterAdjustMainRegist2nd, 		"2段目"},
#if !defined( DEF_SparrowAIOSFPCus ) //Sparrow AIO/Sparrow2 SFPは対象外.

									{TYPE_PrinterAdjustMainRegist3rd, 		"3段目"},
#endif // #if !defined() //Sparrow AIO/Sparrow2 SFPは対象外.
//									{TYPE_PrinterAdjustMainRegist4th,		"4段目"},
									{TYPE_PrinterAdjustMainRegistHand, 		"手差し"}  };
ST_ADJUSTDATADISPLAY_TRAY st_DisplayTableTrayS3[SS_TrayTypeKind3]
							 = {	{TYPE_PrinterAdjustMainRegist1st,		"1段目"},
									{TYPE_PrinterAdjustMainRegist2nd, 		"2段目"},
									{TYPE_PrinterAdjustMainRegist3rd, 		"3段目"},
									{TYPE_PrinterAdjustMainRegist4th,		"4段目"},
									{TYPE_PrinterAdjustMainRegistHand, 		"手差し"}  };

//両面プリント位置側面用テーブル.
ST_ADJUSTDATADISPLAY_TRAY st_DisplayTableTrayD[SS_TrayTypeKind]
							 = {	{TYPE_PrinterAdjustMainRegist1st2,		"1段目"},
									{TYPE_PrinterAdjustMainRegist2nd2, 		"2段目"},
#if !defined( DEF_SparrowAIOSFPCus ) //Sparrow AIO/Sparrow2 SFPは対象外.
									{TYPE_PrinterAdjustMainRegist3rd2, 		"3段目"},
#endif // #if !defined() //Sparrow AIO/Sparrow2 SFPは対象外.
//									{TYPE_PrinterAdjustMainRegist4th2,		"4段目"},
									{TYPE_PrinterAdjustMainRegistHand2, 	"手差し"}  };
ST_ADJUSTDATADISPLAY_TRAY st_DisplayTableTrayD3[SS_TrayTypeKind3]
							 = {	{TYPE_PrinterAdjustMainRegist1st2,		"1段目"},
									{TYPE_PrinterAdjustMainRegist2nd2, 		"2段目"},
									{TYPE_PrinterAdjustMainRegist3rd2, 		"3段目"},
									{TYPE_PrinterAdjustMainRegist4th2,		"4段目"},
									{TYPE_PrinterAdjustMainRegistHand2, 	"手差し"}  };
#else // defined(DEF_A4_Cus).
//End[Sparrow].
#if defined(DEF_ZeusCus) || defined(DEF_ZeusEmu) 		// Zeus用.
#if !defined(DEF_DenebMLKCus)	// !DenebMLK.
#define	SS_PaperTypeKind		12			// Zeus 12
#else
// 2018/11/26 [DenebMLK][AR1055247] 再提起問題：統合仕様との食い違いを修正 ( 8 -> 11 ) Sky新大阪 ->.
#define	SS_PaperTypeKind		11			// DenebMLK用.
// 2018/11/26 [DenebMLK][AR1055247] 再提起問題：統合仕様との食い違いを修正 ( 8 -> 11 ) Sky新大阪 <-.
#endif // !DenebMLK.
#else					// Minerva用.
#define	SS_PaperTypeKind		8			// Minerva 8
#endif	// #if !defined(DEF_ZeusCus) && !defined(DEF_ZeusEmu) .
#if !defined(DEF_DenebMLKCus)	// !DenebMLK.
#define	SS_TrayTypeKind			5
#else
#define	SS_TrayTypeKind			6
#endif // !DenebMLK.
//プリント位置・先端に使用.
typedef struct {
	TYPE_PrinterAdjust 	e_PaperType;							//用紙種類.
	char				sc_DispStr[SS_PaperTypeNameLength];		//表示文字列.
} ST_ADJUSTDATADISPLAY_PAPER;
ST_ADJUSTDATADISPLAY_PAPER st_DisplayTable[SS_PaperTypeKind]
// 2018/11/26 [DenebMLK][AR1055247] 再提起問題：統合仕様との食い違いを修正 Sky新大阪 ->.
#if !defined(DEF_DenebMLKCus)	// DenebMLK以外.
							 = {	{TYPE_PrinterAdjustSubRegistNormal,		"普通紙"},
									{TYPE_PrinterAdjustSubRegistThin,		"薄紙"},
									{TYPE_PrinterAdjustSubRegistOHP, 		"OHP"},
									{TYPE_PrinterAdjustSubRegistHeavy1, 	"厚紙1/1+"},
									{TYPE_PrinterAdjustSubRegistHeavy2, 	"厚紙2"},
									{TYPE_PrinterAdjustSubRegistHeavy3, 	"厚紙3"},
									{TYPE_PrinterAdjustSubRegistHeavy4, 	"厚紙4"},
									{TYPE_PrinterAdjustSubRegistEnvelope, 	"封筒"}
#if defined(DEF_ZeusCus) || defined(DEF_ZeusEmu) 	//	Zeus表示.
									,
									{TYPE_PrinterAdjustTopMargin1stTrayS, 	"トレイ1(short)"},
									{TYPE_PrinterAdjustTopMargin1stTrayL, 	"トレイ1(int32_t)"},
									{TYPE_PrinterAdjustTopMargin2nd3rd4thLCTTray, 	"トレイ2/3/4"},
									{TYPE_PrinterAdjustTopMarginManualTray, 	"手差し"}
#endif	// #ifdef DEF_ZeusCus	//	Zeus表示.
#else							// DenebMLK.
							 = {	{TYPE_PrinterAdjustSubRegistNormal,					"トレイ1_普通紙"},
									{TYPE_PrinterAdjustSubRegistHeavy1,					"トレイ1_厚紙1/1+"},
									{TYPE_PrinterAdjustSubRegistEnvelope,				"トレイ1_封筒"},
									{TYPE_PrinterAdjustTopMargin2ndTray, 				"トレイ2"},
									{TYPE_PrinterAdjustTopMargin3rdTray, 				"トレイ3"},
									{TYPE_PrinterAdjustTopMargin4thTray, 				"トレイ4"},
									{TYPE_PrinterAdjustTopMargin5thTray, 				"トレイ5"},
									{TYPE_PrinterAdjustTopMarginManualTray,				"手差し_普通紙"},
									{TYPE_PrinterAdjustSubRegistHeavy1ManualTray, 		"手差し_厚紙1/1+"},
									{TYPE_PrinterAdjustSubRegistOHPManualTray, 			"手差し_OHP"},
									{TYPE_PrinterAdjustSubRegistEnvelopeManualTray, 	"手差し_封筒"}
#endif	// DenebMLK.
// 2018/11/26 [DenebMLK][AR1055247] 再提起問題：統合仕様との食い違いを修正 Sky新大阪 <-.
								};
//プリント位置・側端に使用.
typedef struct {
	TYPE_PrinterAdjust 	e_FeedTrayType;							//給紙口種類.
	char				sc_DispStr[SS_PaperTypeNameLength];		//表示文字列.
} ST_ADJUSTDATADISPLAY_TRAY;
//プリント位置側面用テーブル.
ST_ADJUSTDATADISPLAY_TRAY st_DisplayTableTrayS[SS_TrayTypeKind]
							 = {	{TYPE_PrinterAdjustMainRegist1st,		"1段目"},
									{TYPE_PrinterAdjustMainRegist2nd, 		"2段目"},
									{TYPE_PrinterAdjustMainRegist3rd, 		"3段目"},
									{TYPE_PrinterAdjustMainRegist4th,		"4段目"},
#if defined(DEF_DenebMLKCus)	// DenebMLK.
									{TYPE_PrinterAdjustMainRegistLCT,		"5段目"},
#endif // DenebMLK.
									{TYPE_PrinterAdjustMainRegistHand, 		"手差し"}  };
//両面プリント位置側面用テーブル.
ST_ADJUSTDATADISPLAY_TRAY st_DisplayTableTrayD[SS_TrayTypeKind]
							 = {	{TYPE_PrinterAdjustMainRegist1st2,		"1段目"},
									{TYPE_PrinterAdjustMainRegist2nd2, 		"2段目"},
									{TYPE_PrinterAdjustMainRegist3rd2, 		"3段目"},
									{TYPE_PrinterAdjustMainRegist4th2,		"4段目"},
#if defined(DEF_DenebMLKCus)	// DenebMLK.
									{TYPE_PrinterAdjustMainRegistLCT2,		"5段目"},
#endif // DenebMLK.
									{TYPE_PrinterAdjustMainRegistHand2, 	"手差し"}  };
#endif // define(DEF_A4_Cus).

void GetMachineAdjustmentData()
{
	float sf_Margine;
#if !defined(DEF_ZeusCus) && !defined(DEF_ZeusEmu) 	// Zeusでは表示不要.
	short ss_AdjustData;
#endif	// #if !defined(DEF_ZeusCus) && !defined(DEF_ZeusEmu)	// Zeusでは表示不要.
	short   i;	
	
	//プリント位置・先端.
	printf( " プリント位置・先端\r\n" );
#if defined(DEF_A4_Cus)
	if ( TYPE_ModelTypeIII == THRG_GetModelType() )	{
		for(i=0; i<SS_PaperTypeKind3; i++)
		{
			sf_Margine = (float)pc_PrinterServiceSetting->getPrinterAdjust(st_DisplayTable3[i].e_PaperType)/10;
			if ( (TYPE_PrinterAdjustTopMargin2nd3rd4thLCTTray == st_DisplayTable3[i].e_PaperType) &&
				(True == THRF_isSparrow2AIO()) ){
					// 3rdのSparrow2AIOのみトレイ名を トレイ2/3 とする.
					printf( " %s = %3.1f\r\n", "トレイ2/3" , sf_Margine );
			}else{
				printf( " %s = %3.1f\r\n", st_DisplayTable3[i].sc_DispStr , sf_Margine );
			}
		}
	}else{
#endif // defined(DEF_A4_Cus).
	for(i=0; i<SS_PaperTypeKind; i++)
	{
		sf_Margine = (float)pc_PrinterServiceSetting->getPrinterAdjust(st_DisplayTable[i].e_PaperType)/10;
		printf( " %s = %3.1f\r\n", st_DisplayTable[i].sc_DispStr , sf_Margine );
	}
#if defined(DEF_A4_Cus)
	}
#endif // defined(DEF_A4_Cus).
	printf( "\r\n");
	//プリント位置・側端.
	printf( " プリント位置・側端\r\n" );
#if defined(DEF_A4_Cus)
	if ( TYPE_ModelTypeIII == THRG_GetModelType() )	{
		for(i=0; i<SS_TrayTypeKind3; i++)
		{
			if ( (TYPE_PrinterAdjustMainRegist4th == st_DisplayTableTrayS3[i].e_FeedTrayType) &&
				(True == THRF_isSparrow2AIO()) ){
				continue;
			}			
			sf_Margine = (float)pc_PrinterServiceSetting->getPrinterAdjust(st_DisplayTableTrayS3[i].e_FeedTrayType)/10;
			printf( " %s = %3.1f\r\n", st_DisplayTableTrayS3[i].sc_DispStr , sf_Margine );
		}
	}else{
#endif // defined(DEF_A4_Cus).
	for(i=0; i<SS_TrayTypeKind; i++)
	{
		sf_Margine = (float)pc_PrinterServiceSetting->getPrinterAdjust(st_DisplayTableTrayS[i].e_FeedTrayType)/10;
		printf( " %s = %3.1f\r\n", st_DisplayTableTrayS[i].sc_DispStr , sf_Margine );
	}
#if defined(DEF_A4_Cus)
	}
#endif // defined(DEF_A4_Cus).
	printf( "\r\n");
	//両面プリント位置・側端.
	printf( " 両面プリント位置・側端\r\n" );
#if defined(DEF_A4_Cus)
	if ( TYPE_ModelTypeIII == THRG_GetModelType() )	{
		for(i=0; i<SS_TrayTypeKind3; i++)
		{
			if ( (TYPE_PrinterAdjustMainRegist4th2 == st_DisplayTableTrayD3[i].e_FeedTrayType) &&
				(True == THRF_isSparrow2AIO()) ){
				continue;
			}			
			sf_Margine = (float)pc_PrinterServiceSetting->getPrinterAdjust(st_DisplayTableTrayD3[i].e_FeedTrayType)/10;
			printf( " %s = %3.1f\r\n", st_DisplayTableTrayD3[i].sc_DispStr , sf_Margine );
		}
	}else{
#endif // defined(DEF_A4_Cus).
	for(i=0; i<SS_TrayTypeKind; i++)
	{
		sf_Margine = (float)pc_PrinterServiceSetting->getPrinterAdjust(st_DisplayTableTrayD[i].e_FeedTrayType)/10;
		printf( " %s = %3.1f\r\n", st_DisplayTableTrayD[i].sc_DispStr , sf_Margine );
	}
#if defined(DEF_A4_Cus)
	}
#endif // defined(DEF_A4_Cus).
#if !defined(DEF_ZeusCus) && !defined(DEF_ZeusEmu) 	// Zeusでは表示不要.
	printf( "\r\n");
	printf( " LD相対位置補正\r\n" );
#if !defined(DEF_MonoCus)	// 2015/3/5 MinervaSBK条件追加.
	//LD1及びLD2相対位置補正(Y).
	ss_AdjustData = pc_PrinterServiceSetting->getPrinterAdjust(TYPE_PrinterAdjustLDRelativePositionY);
	printf( " LD相対補正値(Y) = %d\r\n", ss_AdjustData );
	//LD1及びLD2相対位置補正(M).
	ss_AdjustData = pc_PrinterServiceSetting->getPrinterAdjust(TYPE_PrinterAdjustLDRelativePositionM);
	printf( " LD相対補正値(M) = %d\r\n", ss_AdjustData );
	//LD1及びLD2相対位置補正(C).
	ss_AdjustData = pc_PrinterServiceSetting->getPrinterAdjust(TYPE_PrinterAdjustLDRelativePositionC);
	printf( " LD相対補正値(C) = %d\r\n", ss_AdjustData );
#endif
	//LD1及びLD2相対位置補正(K).
	ss_AdjustData = pc_PrinterServiceSetting->getPrinterAdjust( TYPE_PrinterAdjustLDRelativePositionK);
	printf( " LD相対補正値(K) = %d\r\n", ss_AdjustData );
#endif	// #if !defined(DEF_ZeusCus) && defined(DEF_ZeusEmu)// Zeusでは表示不要.
}
/*================================================================================================
［機能］	ターミナルコマンド（ENG & IR & DFシリアル番号設定）：ラインモード用 
--------------------------------------------------------------------------------------------------
［引数］	char* EngSerialNo		：エンジンシリアルNo. 
			char* IrSerialNo		：IRシリアルNo. 
			char* DfSerialNo		：DFシリアルNo. 
［戻値］	無し 
［詳細］	
［ｳｫｰｸｽﾙｰ］	未実施 
--------------------------------------------------------------------------------------------------
 [履歴]
 '06/05/29 山本(F) 新規作成 
 '08/03/24 古野 Donau対応 ENGシリアル11桁許容(11桁の場合は数値のみでもOK) & DFシリアル設定追加.
 '08/04/08 古野 既存のシリアル設定処理は残すようにする.
                ENGシリアルは7桁以上13桁以下を許容する & ENG,IR,DF各々個別に設定できるようにする.
 '16/10/18 Sky鈴木 [IT5_4.1] 制御構成変更No.10 コンパイル環境作成 ZeusSBK2nd DFシリアル13桁対応.
 2017/09/01 Sky井柳 [Eagle][制御構成変更No10] DFシリアル番号桁数（ EagleH側（DF713)：16桁・EagleL側（DF714/DF632）：13桁）対応.
 2018/11/07 Sky松井 [DenebMLK] AR.1064565 DenebMLKのDFシリアル番号桁数定義を13桁となるよう修正.
 2019/07/19 Sky [EagleH] EagleH OP_BTS-9105 【生産事前流し】APP1登録時の不具合.
================================================================================================*/
//シリアル番号型定義 桁数.
#define DEF_SNLeng_ENG_MIN		7		//ENGシリアル番号最小桁数.
#define DEF_SNLeng_ENG_MAX		13		//ENGシリアル番号最大桁数.
#define DEF_SNLeng_IR			16		//IRシリアル番号桁数.

//DFシリアル番号桁数.
#define DEF_SNLeng_DF_13        13       // EagleL側（DF714/DF632） DenebMLK時(DF633).
#define DEF_SNLeng_DF_16        16       // EagleH側（DF713).

//2020/01/27 Sky DFのシリアルNoの文字数を、常に16桁以下でチェックする対応 ->.
size_t si_SNLeng_DF = DEF_SNLeng_DF_16;
//2020/01/27 Sky DFのシリアルNoの文字数を、常に16桁以下でチェックする対応 <-.

//数値のみかチェック.
Bool CheckDetail_Number( char* psc_SerialNo )
{
	short ss_chkLength;
	short ss_NoLength;

	ss_NoLength = (short)strlen( psc_SerialNo );
	//数値チェック.
	for( ss_chkLength = 0; ss_chkLength < ss_NoLength; ss_chkLength++ ) {
		if( !( isdigit( psc_SerialNo[ss_chkLength] ))) {
			break;
		}
	}
	return ( ss_NoLength != ss_chkLength ? True : False );
}

//文字のみかチェック.
Bool CheckDetail_Character( char* psc_SerialNo )
{
	short ss_chkLength;
	short ss_NoLength;

	ss_NoLength = (short)strlen( psc_SerialNo );
	//文字列チェック.
	for( ss_chkLength = 0; ss_chkLength < ss_NoLength; ss_chkLength++ ) {
		if( isdigit( psc_SerialNo[ss_chkLength] ) ){
			break;
		}
	}
	return ( ss_NoLength != ss_chkLength ? True : False );
}

void SetSerialNumber( char* psc_EngSerialNo, char* psc_IrSerialNo )
{
	if( checkLineMode() == False ){					// ラインモードかどうかのチェック.
		return;
	}

	if( psc_EngSerialNo == NULL || psc_IrSerialNo == NULL		//引数NULLチェック
	||  DEF_SNLeng_ENG_MAX != strlen( psc_EngSerialNo )			//文字数チェック ENG.
	||  False == CheckDetail_Number( psc_EngSerialNo )			//数字のみかチェック ENG.
	||  False == CheckDetail_Character( psc_EngSerialNo )		//英字のみかチェック ENG.
	||  DEF_SNLeng_IR != strlen( psc_IrSerialNo )				//文字数チェック IR.
	||  False == CheckDetail_Number( psc_IrSerialNo )			//数字のみかチェック IR.
	||  False == CheckDetail_Character( psc_IrSerialNo )){		//英字のみかチェック IR.
		//NG.
		char sc_EngSerialNostr[DEF_SNLeng_ENG_MAX+1];
		char sc_IrSerialNostr[DEF_SNLeng_IR+1];
		pc_SystemInfo->getSerialNoPrinter( sc_EngSerialNostr );
		pc_SystemInfo->getSerialNoScaner( sc_IrSerialNostr );
		printf( " [Format]SetSerialNumber Engne SerialNo., IR SerialNo.\r\n" );
		printf( " Engne SerialNo.  … 13桁ENGシリアルNo\r\n" );
		printf( " IR SerialNo.     … 16桁IRシリアルNo\r\n" );
		printf( " Last ENG SerialNo. = " );
		printf( "%s\r\n", sc_EngSerialNostr );
		printf( " Last IR  SerialNo. = " );
		printf( "%s\r\n", sc_IrSerialNostr );
	} else {	//OK.
		pc_SystemInfo->setSerialNoPrinter( psc_EngSerialNo );	//ENGシリアルNo.設定.
		printf( " Set ENG SerialNo. = " );
		printf( "%s\r\n", psc_EngSerialNo );
		pc_SystemInfo->setSerialNoScaner( psc_IrSerialNo );		//IRシリアルNo.設定.
		printf( " Set IR  SerialNo. = " );
		printf( "%s\r\n", psc_IrSerialNo );
	}
}

//ENGシリアルナンバー設定.
void SetSerialNumberENG( char* psc_SerialNo )
{
	if( checkLineMode() == False ){				// ラインモードかどうかのチェック.
		return;
	}

	if( psc_SerialNo == NULL
	||  DEF_SNLeng_ENG_MIN > strlen( psc_SerialNo )			//最小文字数チェック.
	||  DEF_SNLeng_ENG_MAX < strlen( psc_SerialNo )			//最大文字数チェック.
	||  False == CheckDetail_Character( psc_SerialNo )){	//英字のみかチェック.
		//NG.
		char sc_SerialNostr[DEF_SNLeng_ENG_MAX+1];
		pc_SystemInfo->getSerialNoPrinter( sc_SerialNostr );
		printf( " [Format]SetSerialNumber Engine SerialNo.\r\n" );
		printf( " Engine SerialNo.  … 7桁から13桁 ENGシリアルNo\r\n" );
		printf( " Last ENG SerialNo. = " );
		printf( "%s\r\n", sc_SerialNostr );
	} else {	//OK.
		pc_SystemInfo->setSerialNoPrinter( psc_SerialNo );	//ENGシリアルNo.設定.
		printf( " Set ENG SerialNo. = " );
		printf( "%s\r\n", psc_SerialNo );
	}
}

//IRシリアルナンバー設定.
void SetSerialNumberIR( char* psc_SerialNo )
{
	if( checkLineMode() == False ){				// ラインモードかどうかのチェック.
		return;
	}

	if( psc_SerialNo == NULL								//引数NULLチェック
	||  DEF_SNLeng_IR != strlen( psc_SerialNo )				//文字数チェック.
	||  False == CheckDetail_Number( psc_SerialNo )			//数字のみかチェック.
	||  False == CheckDetail_Character( psc_SerialNo )){	//英字のみかチェック.
		//NG.
		char sc_SerialNostr[DEF_SNLeng_IR+1];
		pc_SystemInfo->getSerialNoScaner( sc_SerialNostr );
		printf( " [Format]SetSerialNumberIR IR SerialNo.\r\n" );
		printf( " IR SerialNo.     … 16桁IRシリアルNo\r\n" );
		printf( " Last IR SerialNo. = " );
		printf( "%s\r\n", sc_SerialNostr );
	} else {	//OK.
		pc_SystemInfo->setSerialNoScaner( psc_SerialNo );	//IRシリアルNo.設定.
		printf( " Set IR SerialNo. = " );
		printf( "%s\r\n", psc_SerialNo );
	}
}

//DFシリアルナンバー設定.
//2016/07/12 Sky千葉 AR1001758	【IT_3.3】【CC-AccuRev手動マージ】DFシリアル番号桁数 [ZeusSBK] Sky合屋 16桁→13桁に修正. 
//2020/01/27 Sky DFのシリアルNoの文字数を、常に16桁以下でチェックする対応.
void SetSerialNumberDF( char* psc_SerialNo )
{
	if( checkLineMode() == False ){				// ラインモードかどうかのチェック.
		return;
	}

	if( psc_SerialNo == NULL								//引数NULLチェック
	||  si_SNLeng_DF < strlen( psc_SerialNo )				//文字数チェック.
	||  False == CheckDetail_Number( psc_SerialNo )			//数字のみかチェック.
	||  False == CheckDetail_Character( psc_SerialNo )){	//英字のみかチェック.
		//NG.
		char sc_SerialNostr[DEF_SNLeng_DF_16+1];
		pc_SystemInfo->getSerialNoAdf( sc_SerialNostr );
		printf( " [Format]SetSerialNumberDF DF SerialNo.\r\n" );
		printf( " DF SerialNo.     … %d桁DFシリアルNo\r\n", si_SNLeng_DF );
		printf( " Last DF SerialNo. = " );
		printf( "%s\r\n", sc_SerialNostr );
	} else {	//OK.
		pc_SystemInfo->setSerialNoAdf( psc_SerialNo );		//DFシリアルNo.設定.
		printf( " Set DF SerialNo. = " );
		printf( "%s\r\n", psc_SerialNo );
	}
}
/*================================================================================================
［機能］	ターミナルコマンド（PH固有データ設定）：ラインモード用 
--------------------------------------------------------------------------------------------------
［引数］	short ss_mode				：モード（0:C 1:M 2:Y 3:K）
			short ss_RelativePosition	：LD相対距離（1～27）
			short ss_PHSkewMotorInit	：スキュー補正モータ初期設定位置（0～480）
［戻値］	無し 
［詳細］	
［ｳｫｰｸｽﾙｰ］	未実施 
--------------------------------------------------------------------------------------------------
 [履歴]
 '06/05/29 山本(F) 新規作成 
 '08/10/01 鈴村 CQ310066_仕様変更_Donau/Amurではスキュー補正モータの設定範囲変更.
 '09/09/15 越野		Soc対応.
 '17/03/28 Sky千葉	AR1016349ターミナルからのスキュー補正モータ初期設定位置設定で引数の範囲が仕様と異なる.
================================================================================================*/
//定数定義.
//設定モード型定義.
typedef short EC_LDRelativePositionMode;
#define EC_LDRelativePositionMode_C     	0	//C.
#ifndef DEF_DonauBKCus // 10/04/23 hanaoka CQ325164出荷前検査装置対応. // 2011/02/23 ft_kondo 第6回手動マージ. 
#define EC_LDRelativePositionMode_M     	1	//M.
#define EC_LDRelativePositionMode_Y     	2	//Y.
#endif // #ifndef DEF_DonauBKCus.
#define EC_LDRelativePositionMode_K     	3	//K.
//ＬＤ相対位置調整型定義.
typedef short EC_LDRelativePositionRange;
#define EC_LDRelativePositionRange_MIN     1	//ＬＤ相対位置調整最小値.
#define EC_LDRelativePositionRange_MAX     27	//ＬＤ相対位置調整最大値.
//スキュー補正モータ初期設定型定義.
typedef short EC_PHSkewMotorInitRange;
#ifdef DEF_MINERVAMLK_CUS
//MinervaMLKスキュー補正モータ初期設定定義.
#define EC_PHSkewMotorInitRange_MIN     	0	//MMLK用スキュー補正モータ初期設定最小値.
#define EC_PHSkewMotorInitRange_MAX     	480	//MMLK用スキュー補正モータ初期設定最大値.
#else
#define EC_PHSkewMotorInitRange_MIN     	150	//スキュー補正モータ初期設定最小値.
#define EC_PHSkewMotorInitRange_MAX     	250	//スキュー補正モータ初期設定最大値.
#endif	//#ifdef DEF_MINERVAMLK_CUS.

//パラメータチェック（ＬＤ相対位置調整）.
Bool CheckPHNativeDataLD(short ss_mode, short ss_LDRelativePosition)
{
	if( (EC_LDRelativePositionMode_C > ss_mode) || (EC_LDRelativePositionMode_K < ss_mode) ) {	//モードパラメータ異常.
		return False;
	}
	if( (EC_LDRelativePositionRange_MIN > ss_LDRelativePosition) || (EC_LDRelativePositionRange_MAX < ss_LDRelativePosition) ) {	//LD相対距離パラメータ異常.
		return False;
	}
	return True;
}
//パラメータチェック（スキュー補正モータ初期設定）.
Bool CheckPHNativeDataSKEW(short ss_mode, short ss_PHSkewMotorInit)
{
	if( (EC_LDRelativePositionMode_C > ss_mode) || (EC_LDRelativePositionMode_Y < ss_mode) ) {	//モードパラメータ異常.
		return False;
	}
	if( (EC_PHSkewMotorInitRange_MIN > ss_PHSkewMotorInit) || (EC_PHSkewMotorInitRange_MAX < ss_PHSkewMotorInit) ) {	//スキュー補正モータ初期設定位置パラメータ異常.
		return False;
	}
	return True;
}
//PH固有データ設定：LD相対距離(Y).
void SetPHNativeDataLDY( short ss_LDRelativePosition )
{
	if( checkLineMode() == False ){		// ラインモードかどうかのチェック.
		return;
	}
	if(False == CheckPHNativeDataLD(EC_LDRelativePositionMode_Y, ss_LDRelativePosition)) {	//パラメータチェック.
		printf( " [Format]SetPHNativeDataLDY LDRelativePosition\r\n" );
		printf( " LDRelativePosition … LD相対距離(Y)（1～27）\r\n" );
	} else {	
		//EC_LDRelativePositionMode_Y:LD相対距離(Y)設定.
		pc_PrinterServiceSetting->setPrinterAdjust(TYPE_PrinterAdjustLDRelativePositionY, ss_LDRelativePosition );  // LD相対位置調整(Y).
	}
}
//PH固有データ設定：LD相対距離(M).
void SetPHNativeDataLDM( short ss_LDRelativePosition )
{
	if( checkLineMode() == False ){		// ラインモードかどうかのチェック.
		return;
	}
	if(False == CheckPHNativeDataLD(EC_LDRelativePositionMode_M, ss_LDRelativePosition)) {	//パラメータチェック.
		printf( " [Format]SetPHNativeDataLDM LDRelativePosition\r\n" );
		printf( " LDRelativePosition … LD相対距離(M)（1～27）\r\n" );
	} else {
		//EC_LDRelativePositionMode_M:LD相対距離(M).
		pc_PrinterServiceSetting->setPrinterAdjust(TYPE_PrinterAdjustLDRelativePositionM, ss_LDRelativePosition );  // LD相対位置調整(M).
	}
}
//PH固有データ設定：LD相対距離(C).
void SetPHNativeDataLDC( short ss_LDRelativePosition )
{
	if( checkLineMode() == False ){		// ラインモードかどうかのチェック.
		return;
	}
	if(False == CheckPHNativeDataLD(EC_LDRelativePositionMode_C, ss_LDRelativePosition)) {	//パラメータチェック.
		printf( " [Format]SetPHNativeDataLDC LDRelativePosition\r\n" );
		printf( " LDRelativePosition … LD相対距離(C)（1～27）\r\n" );
	} else {	
		//EC_LDRelativePositionMode_C:LD相対距離(C).
		pc_PrinterServiceSetting->setPrinterAdjust(TYPE_PrinterAdjustLDRelativePositionC, ss_LDRelativePosition );  // LD相対位置調整(C).
	}
}
//PH固有データ設定：LD相対距離(K).
void SetPHNativeDataLDK( short ss_LDRelativePosition )
{
	if( checkLineMode() == False ){		// ラインモードかどうかのチェック.
		return;
	}
	if(False == CheckPHNativeDataLD(EC_LDRelativePositionMode_K, ss_LDRelativePosition)) {	//パラメータチェック.
		printf( " [Format]SetPHNativeDataLDK LDRelativePosition\r\n" );
		printf( " LDRelativePosition … LD相対距離(K)（1～27）\r\n" );
	} else {
		//EC_LDRelativePositionMode_K:LD相対距離(K).
		pc_PrinterServiceSetting->setPrinterAdjust(TYPE_PrinterAdjustLDRelativePositionK, ss_LDRelativePosition );  // LD相対位置調整(K).
	}
}
//PH固有データ設定：スキュー補正モータ初期設定位置(Y).
void SetPHNativeDataSKEWY( short ss_PHSkewMotorInit )
{
	if( checkLineMode() == False ){		// ラインモードかどうかのチェック.
		return;
	}
	if(False == CheckPHNativeDataSKEW(EC_LDRelativePositionMode_Y, ss_PHSkewMotorInit)) {	//パラメータチェック.
		printf( " [Format]SetPHNativeDataSKEWY PHSkewMotorInit\r\n" );
		printf( " PHSkewMotorInit    … スキュー補正モータ初期設定位置(Y)（%d～%d）\r\n", EC_PHSkewMotorInitRange_MIN, EC_PHSkewMotorInitRange_MAX );
	} else {	
		//EC_LDRelativePositionMode_Y:スキュー補正モータ初期設定位置(Y).
		pc_PrinterServiceSetting->setPrinterAdjust(TYPE_PrinterAdjustPHSkewMotorInitY, ss_PHSkewMotorInit );  		// スキュー補正モータYの初期設定位置.
	}
}
//PH固有データ設定：スキュー補正モータ初期設定位置(M).
void SetPHNativeDataSKEWM( short ss_PHSkewMotorInit )
{
	if( checkLineMode() == False ){		// ラインモードかどうかのチェック.
		return;
	}
	if(False == CheckPHNativeDataSKEW(EC_LDRelativePositionMode_M, ss_PHSkewMotorInit)) {	//パラメータチェック.
		printf( " [Format]SetPHNativeDataSKEWM PHSkewMotorInit\r\n" );
		printf( " PHSkewMotorInit    … スキュー補正モータ初期設定位置(M)（%d～%d）\r\n", EC_PHSkewMotorInitRange_MIN, EC_PHSkewMotorInitRange_MAX );
	} else {	
		//EC_LDRelativePositionMode_M:スキュー補正モータ初期設定位置(M).
		pc_PrinterServiceSetting->setPrinterAdjust(TYPE_PrinterAdjustPHSkewMotorInitM, ss_PHSkewMotorInit );  		// スキュー補正モータMの初期設定位置.
	}
}
//PH固有データ設定：スキュー補正モータ初期設定位置(C).
void SetPHNativeDataSKEWC( short ss_PHSkewMotorInit )
{
	if( checkLineMode() == False ){		// ラインモードかどうかのチェック.
		return;
	}
	if(False == CheckPHNativeDataSKEW(EC_LDRelativePositionMode_C, ss_PHSkewMotorInit)) {	//パラメータチェック.
		printf( " [Format]SetPHNativeDataSKEWC PHSkewMotorInit\r\n" );
		printf( " PHSkewMotorInit    … スキュー補正モータ初期設定位置(C)（%d～%d）\r\n", EC_PHSkewMotorInitRange_MIN, EC_PHSkewMotorInitRange_MAX );
	} else {	
		//EC_LDRelativePositionMode_C:			//C.
		pc_PrinterServiceSetting->setPrinterAdjust(TYPE_PrinterAdjustPHSkewMotorInitC, ss_PHSkewMotorInit );  		// スキュー補正モータCの初期設定位置.
	}
}
/*================================================================================================
［機能］	ターミナルコマンド（先端プリント位置調整値表示） 
--------------------------------------------------------------------------------------------------
［引数］	short ss_Type				：用紙種類 
										： Eagle(1～12).
										：  1:Plain Paper				(普通紙).
										：  2:Thick Paper1/1+			(厚紙1/1+).
										：  3:Thick Paper2				(厚紙2).
										：  4:Thick Paper3				(厚紙3).
										：  5:Thick Paper4				(厚紙4).
										：  6:OHP Film					(OHP).
										：  7:Envelope					(封筒).
										：  8:Tray1 short				(トレイ1 short).
										：  9:Tray1 long				(トレイ1 long).
										：  10:Tray2_3_4T				(トレイ2/3/4).
										：  11:Manual					(手差し).
										：  12:Thin Paper				(薄紙).
										： DenebMLK(13～23).
										：  13:Tray1_Plain Paper		(トレイ1+普通紙).
										：  14:Tray1_Thick Paper1/1+	(トレイ1+厚紙1/1+).
										：  15:Tray1_Envelope			(トレイ1+封筒).
										：  16:Tray2					(トレイ2).
										：  17:Tray3					(トレイ3).
										：  18:Tray4					(トレイ4).
										：  19:Tray5					(トレイ5).
										：  20:Manual_Plain Paper		(手差し+普通紙).
										：  21:Manual_Thick Paper1/1+	(手差し+厚紙1/1+).
										：  22:Manual_OHP Film			(手差し+OHP).
										：  23:Manual_Envelope			(手差し+封筒).
［戻値］	無し 
［詳細］ 
［ｳｫｰｸｽﾙｰ］	未実施 
--------------------------------------------------------------------------------------------------
 [履歴] 
 '06/08/04 N.UDA(REVSONIC) 新規作成 
 '08/09/03 furuno CQ309029 Amur対応.「給紙口毎のトップマージン調整」に対応.パラメータ8～11を追加.
 '12/09/19 福原		CQ371998 対応.
 '18/07/17 Kaihotsu 【DenebMLK 生産コマンド対応（コマンド追加対応）:機器構成変更_No_13-2 チョイス調整機能変更対応】.
 '18/11/08 Sky松井	[DenebMLK]AR.1065528 引数に13～23を受けた際の出力文字列定義を追加.
										 機器構成変更No.13での出力文字列のDenebMLKとの切り分け部分を削除.
										 用紙種別名の格納バッファサイズを15→30へ変更.
 '19/06/26 Sky鳥居	[EagleH]OP_BTS-9105	TopMargin1stTrayS,TopMargin1stTrayLの文字列を修正.
================================================================================================*/
//定数定義.
#define EC_PrinterAdjust_PaperNameLenMax	(30)				//用紙種類名の格納バッファサイズ.
#define EC_PrinterAdjust_TrayNameLenMax		(6)					//トレイ種類名の格納バッファサイズ.
//#define EC_PrinterAdjust_RegistValue_Max	(100)				//設定値の最大値 MinervaZeus  30⇒100に変更.
//#define EC_PrinterAdjust_RegistValue_Min	(-100)				//設定値の最小値 MinervaZeus -30⇒100に変更.
// [changelog] 2016/07/05 AccuRev環境マージ VenusMLK >>.
#if !defined(DEF_A4_Cus)	//-> CQ525664 2016.03.16.
#define EC_PrinterAdjust_MainRegistValue_Max	(30)			//設定値の最大値.
#define EC_PrinterAdjust_MainRegistValue_Min	(-30)			//設定値の最小値.
#else	// defined(DEF_A4_Cus).
#define EC_PrinterAdjust_MainRegistValue_Max	(50)			//設定値の最大値.
#define EC_PrinterAdjust_MainRegistValue_Min	(-50)			//設定値の最小値.
#endif	//<- !defined(DEF_A4_Cus) CQ525664 2016.03.16.
// [changelog] 2016/07/05 AccuRev環境マージ VenusMLK <<.
#if defined(DEF_DenebMLKCus)	// DenebMLK.
#define EC_PrinterAdjust_SubRegistValue_Max	(40)				//設定値の最大値 MinervaZeus  30⇒ 40に変更 機器構成変更_No_13-2対応.
#else
#define EC_PrinterAdjust_SubRegistValue_Max	(30)				//設定値の最大値 MinervaZeus  100⇒ 30に変更 CQ371998対応.
#endif // DenebMLK.
// [changelog] 2016/07/05 AccuRev環境マージ VenusMLK >>.
#if defined(DEF_A4_Cus)	//-> 封筒の最大値を60とする 2016.04.04.
#define EC_PrinterAdjust_SubRegistValue_Envelope_Max	(60)	//封筒設定値の最大値 VenusMLK 30⇒ 60に変更 2016.04.04.
#endif	//<-defined(DEF_A4_Cus) 2016.04.04.
// [changelog] 2016/07/05 AccuRev環境マージ VenusMLK <<.
#if defined(DEF_DenebMLKCus)	// DenebMLK.
#define EC_PrinterAdjust_SubRegistValue_Min	(-40)				//設定値の最小値 MinervaZeus -30⇒-40に変更 機器構成変更_No_13-2対応.
#else
#define EC_PrinterAdjust_SubRegistValue_Min	(-30)				//設定値の最小値 MinervaZeus -100⇒-30に変更 CQ371998対応.
#endif // DenebMLK.
#define EC_PrinterAdjust_RegistValue_Step	(2)					//設定値のSTEP値.
#if !defined(DEF_A4_Cus)	//-> 2015.12.14 Change Start.
#define EC_PrinterAdjust_PaperName1			"Plain Paper"		//用紙種類1の名前.
#define EC_PrinterAdjust_PaperName2			"Thick Paper1/1+"	//用紙種類2の名前.
#define EC_PrinterAdjust_PaperName3			"Thick Paper2"		//用紙種類3の名前.
#define EC_PrinterAdjust_PaperName4			"Thick Paper3"		//用紙種類4の名前.
#define EC_PrinterAdjust_PaperName5			"Thick Paper4"		//用紙種類5の名前.
#define EC_PrinterAdjust_PaperName6			"OHP Film"			//用紙種類6の名前.
#define EC_PrinterAdjust_PaperName7			"Envelope"			//用紙種類7の名前.
#if defined(DEF_DenebMLKCus)	// DenebMLK.
#define EC_PrinterAdjust_PaperName13		"Tray1_Plain Paper"		//用紙種類13:トレイ1+普通紙.
#define EC_PrinterAdjust_PaperName14		"Tray1_Thick Paper1/1+"	//用紙種類14:トレイ1+厚紙1/1+.
#define EC_PrinterAdjust_PaperName15		"Tray1_Envelope"		//用紙種類15:トレイ1+封筒.
#define EC_PrinterAdjust_PaperName16		"Tray2"					//用紙種類16:トレイ2.
#define EC_PrinterAdjust_PaperName17		"Tray3"					//用紙種類17:トレイ3.
#define EC_PrinterAdjust_PaperName18		"Tray4"					//用紙種類18:トレイ4.
#define EC_PrinterAdjust_PaperName19		"Tray5"					//用紙種類19:トレイ5.
#define EC_PrinterAdjust_PaperName20		"Manual_Plain Paper"	//用紙種類20:手差し+普通紙.
#define EC_PrinterAdjust_PaperName21		"Manual_Thick Paper1/1+"//用紙種類21:手差し+厚紙1/1+.
#define EC_PrinterAdjust_PaperName22		"Manual_OHP Film"		//用紙種類22:手差し+OHP.
#define EC_PrinterAdjust_PaperName23		"Manual_Envelope"		//用紙種類23:手差し+封筒.
/* AR1072240 DenebMLK 「普通紙＋」追加 Start */
#define EC_PrinterAdjust_PaperName24		"Tray1_Plain Paper+"	//用紙種類24:トレイ１_普通紙＋.
#define EC_PrinterAdjust_PaperName25		"Manual_Plain Paper+"	//用紙種類25:手差し_普通紙＋.
/* AR1072240 DenebMLK 「普通紙＋」追加 End */
#endif	// defined(DEF_DenebMLKCus).
#else	// defined(DEF_A4_Cus) 2015.12.14 Add Start.
#define EC_PrinterAdjust_PaperName1			"Plain Paper Gr"	//用紙種類1の名前.
#define EC_PrinterAdjust_PaperName2			"Glossy Mode"		//用紙種類2の名前.
#define EC_PrinterAdjust_PaperName3			"Thick Paper Gr"	//用紙種類3の名前.
#define EC_PrinterAdjust_PaperName4			"Envelope"			//用紙種類4の名前.
#endif	// <- !defined(DEF_A4_Cus) Add End.
// [changelog] 2016/07/05 AccuRev環境マージ VenusMLK >>.
#if !defined(DEF_A4_Cus)	//-> VenusMLK Tray先端余白補正値追加 2016.04.04.
#define EC_PrinterAdjust_TopMargin1stTrayS				"Tray1 short"	//トップマージン トレイ1 short.
#define EC_PrinterAdjust_TopMargin1stTrayL				"Tray1 int32_t"	//トップマージン トレイ1 long.
#define EC_PrinterAdjust_TopMargin2nd3rd4thLCTTray		"Tray2_3_4"		//トップマージン トレイ2/3/4.
#define EC_PrinterAdjust_TopMarginManualTray			"Manual"		//トップマージン 手差し.
#define EC_PrinterAdjust_TopMarginManualThin			"Thin Paper"	//上端余白補正値【薄紙】(1pass).
#else // defined(DEF_A4_Cus)	//-> VenusMLK Tray先端余白補正値追加 2016.04.04.
// Str【VenusMLK CQ526598 2016.04.18 カセット1long未使用】.
#define EC_PrinterAdjust_TopMargin1stTrayS			"Tray1"	//トップマージン トレイ1
//#define EC_PrinterAdjust_TopMargin1stTrayL			"Tray1 long"	//トップマージン トレイ1 long.
// End【VenusMLK CQ526598 2016.04.18 カセット1long未使用】.
#define EC_PrinterAdjust_TopMargin2nd3rdTray			"Tray2_3"		//トップマージン トレイ2/3.
#define EC_PrinterAdjust_TopMargin2nd3rd4thLCTTray		"Tray2_3_4"		//トップマージン トレイ2/3/4.
#define EC_PrinterAdjust_TopMarginManualTray			"Manual"		//トップマージン 手差し.
#endif	//<- defined(DEF_A4_Cus) 2016.04.04.
// [changelog] 2016/07/05 AccuRev環境マージ VenusMLK <<.

#define EC_PrinterAdjust_PaperThin			"Thin Paper"		//薄紙の名前.
//Str【EagleZ+ OP_BTS-15006対応 : Y.Okumoto : 2019.10.21】.
#define EC_PrinterAdjust_PaperPlainPlus		"Plain Paper+"		//普通紙+の名前.
//End【EagleZ+ OP_BTS-15006対応 : Y.Okumoto : 2019.10.21】.

#define EC_PrinterAdjust_TrayName1			"1st"				//トレイ種類1の名前.
#define EC_PrinterAdjust_TrayName2			"2nd"				//トレイ種類2の名前.
#define EC_PrinterAdjust_TrayName3			"3rd"				//トレイ種類3の名前.
#define EC_PrinterAdjust_TrayName4			"4th"				//トレイ種類4の名前.
#if defined(DEF_DenebMLKCus)	// DenebMLK.
#define EC_PrinterAdjust_TrayName5			"5th"				//トレイ種類5の名前.
#define EC_PrinterAdjust_TrayName6			"Manual"			//トレイ種類6の名前.
#else
#define EC_PrinterAdjust_TrayName5			"Manual"			//トレイ種類5の名前.
#define EC_PrinterAdjust_TrayName6			"LCT"				//トレイ種類6の名前.
#endif // DenebMLK.

/*================================================================================================.
［機能］	ターミナルコマンド（先端プリント位置調整値取得）.
--------------------------------------------------------------------------------------------------.
［引数］	short ss_Type				：用紙種類.
										： Eagle(1～12).
										：  1:Plain Paper				(普通紙).
										：  2:Thick Paper1/1+			(厚紙1/1+).
										：  3:Thick Paper2				(厚紙2).
										：  4:Thick Paper3				(厚紙3).
										：  5:Thick Paper4				(厚紙4).
										：  6:OHP Film					(OHP).
										：  7:Envelope					(封筒).
										：  8:Tray1 short				(トレイ1 short).
										：  9:Tray1 long				(トレイ1 long).
										：  10:Tray2_3_4T				(トレイ2/3/4).
										：  11:Manual					(手差し).
										：  12:Thin Paper				(薄紙).
										： DenebMLKのみ(13～23).
										：  13:Tray1_Plain Paper		(トレイ1+普通紙).
										：  14:Tray1_Thick Paper1/1+	(トレイ1+厚紙1/1+).
										：  15:Tray1_Envelope			(トレイ1+封筒).
										：  16:Tray2					(トレイ2).
										：  17:Tray3					(トレイ3).
										：  18:Tray4					(トレイ4).
										：  19:Tray5					(トレイ5).
										：  20:Manual_Plain Paper		(手差し+普通紙).
										：  21:Manual_Thick Paper1/1+	(手差し+厚紙1/1+).
										：  22:Manual_OHP Film			(手差し+OHP).
										：  23:Manual_Envelope			(手差し+封筒).
［戻値］	無し.
［詳細］ 引数で指定された用紙種類が仕様値かを判定し、その値にあった先端プリント位置調整値表示.
		 仕様値以外の値が設定された場合は指定する値の一覧をターミナルに出力する.
［ｳｫｰｸｽﾙｰ］	未実施.
--------------------------------------------------------------------------------------------------.
 [履歴].
 '18/07/17 Kaihotsu 【DenebMLK 生産コマンド対応（コマンド追加対応）:機器構成変更_No_13-2 チョイス調整機能変更対応】.
 '18/11/08 Sky松井	[DenebMLK]AR.1065528 DenebMLK時に引数が13～23のケースを追加.
							  上記と共にAR.1065532 DenebMLK時の引数エラー出力内容を追加.
 '19/01/28 Sky		[DenebMLK]AR.1072240 DenebMLK時に引数が24、25のケースを追加.
 '23/06/29 Sky		OP_BTS-43745 IT6_2.3 Sparrow 給紙トレイ4段対応.
================================================================================================*/
void GetPrinterAdjustSubRegist( short ss_Type )
{
	TYPE_PrinterAdjust 	e_Type;								//用紙種類.
	short ss_Value;											//取得した設定値.
	char sc_Name[ EC_PrinterAdjust_PaperNameLenMax + 1 ];	//用紙種類名(最後はNull分).
	//パラメータチェック.
	switch ( ss_Type ) {
#if !defined(DEF_A4_Cus)	//-> 2015.12.14 Change Start.
#if !defined(DEF_DenebMLKCus)	// DenebMLK以外.
	case 1:									//1:Plain Paper     (普通紙).
		e_Type = TYPE_PrinterAdjustSubRegistNormal;
		sprintf( sc_Name, "%s", EC_PrinterAdjust_PaperName1 );
		break;
	case 2:									//2:Thick Paper1/1+ (厚紙1/1+).
		e_Type = TYPE_PrinterAdjustSubRegistHeavy1;
		sprintf( sc_Name, "%s", EC_PrinterAdjust_PaperName2 );
		break;
	case 3:									//3:Thick Paper2    (厚紙2).
		e_Type = TYPE_PrinterAdjustSubRegistHeavy2;
		sprintf( sc_Name, "%s", EC_PrinterAdjust_PaperName3 );
		break;
	case 4:									//4:Thick Paper3    (厚紙3).
		e_Type = TYPE_PrinterAdjustSubRegistHeavy3;
		sprintf( sc_Name, "%s", EC_PrinterAdjust_PaperName4 );
		break;
	case 5:									//5:Thick Paper4    (厚紙4).
		e_Type = TYPE_PrinterAdjustSubRegistHeavy4;
		sprintf( sc_Name, "%s", EC_PrinterAdjust_PaperName5 );
		break;
	case 6:									//6:OHP Film        (OHP).
		e_Type = TYPE_PrinterAdjustSubRegistOHP;
		sprintf( sc_Name, "%s", EC_PrinterAdjust_PaperName6 );
		break;
	case 7:									//7:Envelope        (封筒).
		e_Type = TYPE_PrinterAdjustSubRegistEnvelope;
		sprintf( sc_Name, "%s", EC_PrinterAdjust_PaperName7 );
		break;
	case 8:									//8:Tray1 short     (トレイ1 short).
		e_Type = TYPE_PrinterAdjustTopMargin1stTrayS;
		sprintf( sc_Name, "%s", EC_PrinterAdjust_TopMargin1stTrayS );
		break;
	case 9:									//9:Tray1 long      (トレイ1 long).
		e_Type = TYPE_PrinterAdjustTopMargin1stTrayL;
		sprintf( sc_Name, "%s", EC_PrinterAdjust_TopMargin1stTrayL );
		break;
	case 10:								//10:Tray2_3_4      (トレイ2/3/4).
		e_Type = TYPE_PrinterAdjustTopMargin2nd3rd4thLCTTray;
		sprintf( sc_Name, "%s", EC_PrinterAdjust_TopMargin2nd3rd4thLCTTray );
		break;
	case 11:								//11:Manual         (手差し).
		e_Type = TYPE_PrinterAdjustTopMarginManualTray;
		sprintf( sc_Name, "%s", EC_PrinterAdjust_TopMarginManualTray );
		break;
	case 12:								//11:薄紙.
		e_Type = TYPE_PrinterAdjustSubRegistThin;	// 副走査倍率微調 薄紙.
		sprintf( sc_Name, "%s", EC_PrinterAdjust_TopMarginManualThin );
		break;
#else	// !defined(DEF_DenebMLKCus) DenebMLK機種.
	case 13:								// 13：Tray1_Plain Paper(トレイ1+普通紙).
		e_Type = TYPE_PrinterAdjustSubRegistNormal;
		sprintf( sc_Name, "%s", EC_PrinterAdjust_PaperName13 );
		break;
	case 14:								// 14：Tray1_Thick Paper1/1+(トレイ1+厚紙1/1+).
		e_Type = TYPE_PrinterAdjustSubRegistHeavy1;
		sprintf( sc_Name, "%s", EC_PrinterAdjust_PaperName14 );
		break;
	case 15:								// 15：Tray1_Envelope(トレイ１+封筒).
		e_Type = TYPE_PrinterAdjustSubRegistEnvelope;
		sprintf( sc_Name, "%s", EC_PrinterAdjust_PaperName15 );
		break;
	case 16:								// 16：Tray2(トレイ2).
		e_Type = TYPE_PrinterAdjustTopMargin2ndTray;
		sprintf( sc_Name, "%s", EC_PrinterAdjust_PaperName16 );
		break;
	case 17:								// 17：Tray3(トレイ3).
		e_Type = TYPE_PrinterAdjustTopMargin3rdTray;
		sprintf( sc_Name, "%s", EC_PrinterAdjust_PaperName17 );
		break;
	case 18:								// 18：Tray4(トレイ4).
		e_Type = TYPE_PrinterAdjustTopMargin4thTray;
		sprintf( sc_Name, "%s", EC_PrinterAdjust_PaperName18 );
		break;
	case 19:								// 19：Tray5(トレイ5).
		e_Type = TYPE_PrinterAdjustTopMargin5thTray;
		sprintf( sc_Name, "%s", EC_PrinterAdjust_PaperName19 );
		break;
	case 20:								// 20：Manual_Plain Paper(手差し+普通紙).
		e_Type = TYPE_PrinterAdjustTopMarginManualTray;
		sprintf( sc_Name, "%s", EC_PrinterAdjust_PaperName20 );
		break;
	case 21:								// 21：Manual_Thick Paper1/1+(手差し+厚紙1/1+).
		e_Type = TYPE_PrinterAdjustSubRegistHeavy1ManualTray;
		sprintf( sc_Name, "%s", EC_PrinterAdjust_PaperName21 );
		break;
	case 22:								// 22：Manual_OHP Film(手差し+OHP).
		e_Type = TYPE_PrinterAdjustSubRegistOHPManualTray;
		sprintf( sc_Name, "%s", EC_PrinterAdjust_PaperName22 );
		break;
	case 23:								// 23：Manual_Envelope(手差し+封筒).
		e_Type = TYPE_PrinterAdjustSubRegistEnvelopeManualTray;
		sprintf( sc_Name, "%s", EC_PrinterAdjust_PaperName23 );
		break;
	case 24:								// 24：Tray1_Plain Paper+(トレイ１_普通紙＋).
		e_Type = TYPE_PrinterAdjustSubRegistNormalPlus;
		sprintf( sc_Name, "%s", EC_PrinterAdjust_PaperName24 );
		break;
	case 25:								// 25：Manual_Plain Paper+(手差し_普通紙＋).
		e_Type = TYPE_PrinterAdjustSubRegistNormalPlusManualTray;
		sprintf( sc_Name, "%s", EC_PrinterAdjust_PaperName25 );
		break;
#endif // !defined(DEF_DenebMLKCus).
#else 	// defined(DEF_A4_Cus)	2015.12.14 Add Start.
	case 1:									//1:Plain Paper     (普通紙).
		e_Type = TYPE_PrinterAdjustSubRegistNormal;
		sprintf( sc_Name, "%s", EC_PrinterAdjust_PaperName1 );
		break;
	case 2:									// 2:光沢モード/1200dpi	(光沢モード）
		e_Type = TYPE_PrinterAdjustSubRegistGlossy;
		sprintf( sc_Name, "%s", EC_PrinterAdjust_PaperName2 );
		break;
	case 3:									// 3:厚紙Gr			（厚紙Gr)
		e_Type = TYPE_PrinterAdjustSubRegistHeavyGr;
		sprintf( sc_Name, "%s", EC_PrinterAdjust_PaperName3 );
		break;
	case 4:									//4:Envelope        (封筒).
		e_Type = TYPE_PrinterAdjustSubRegistEnvelope;
		sprintf( sc_Name, "%s", EC_PrinterAdjust_PaperName4 );
		break;
// [changelog] 2016/07/05 AccuRev環境マージ VenusMLK >>.
//-> VenusMLK トレイ先端余白補正値追加 2016.04.04.
	case 8:									//8:Tray1 short     (トレイ1 short).
		e_Type = TYPE_PrinterAdjustTopMargin1stTrayS;
		sprintf( sc_Name, "%s", EC_PrinterAdjust_TopMargin1stTrayS );
		break;
// Str【VenusMLK CQ526598 2016.04.18 カセット1long未使用】.
//	case 9:									//9:Tray1 long      (トレイ1 long).
//		e_Type = TYPE_PrinterAdjustTopMargin1stTrayL;
//		sprintf( sc_Name, "%s", EC_PrinterAdjust_TopMargin1stTrayL );
//		break;
// End【VenusMLK CQ526598 2016.04.18 カセット1long未使用】.
	case 10:								//10:Tray2_3      (トレイ2/3).
		e_Type = TYPE_PrinterAdjustTopMargin2nd3rd4thLCTTray;
		if( (TYPE_ModelTypeIII == THRG_GetModelType()) &&
			(False == THRF_isSparrow2AIO()) ){
			sprintf( sc_Name, "%s", EC_PrinterAdjust_TopMargin2nd3rd4thLCTTray );
		}else{
			sprintf( sc_Name, "%s", EC_PrinterAdjust_TopMargin2nd3rdTray );
		}
		break;
	case 11:								//11:Manual         (手差し).
		e_Type = TYPE_PrinterAdjustTopMarginManualTray;
		sprintf( sc_Name, "%s", EC_PrinterAdjust_TopMarginManualTray );
		break;
//<- 2016.04.04.
// [changelog] 2016/07/05 AccuRev環境マージ VenusMLK <<.
#endif	//<- !defined(DEF_A4_Cus) Add End.
//Str【EagleZ+ OP_BTS-15006対応 : Y.Okumoto : 2019.10.21】.
	case 26:								//26:Plain Paper+   (普通紙+).
		e_Type = TYPE_PrinterAdjustSubRegistNormalPlus;
		sprintf( sc_Name, "%s", EC_PrinterAdjust_PaperPlainPlus );
		break;
//End【EagleZ+ OP_BTS-15006対応 : Y.Okumoto : 2019.10.21】.
	default:								//用紙種類エラー.
		printf( " [Format]GetPrinterAdjustSubRegist Type\r\n" );
		printf( " Type ... Paper Kind\r\n" );
#if !defined(DEF_A4_Cus)	//-> 2015.12.14 Change Start.
#if !defined(DEF_DenebMLKCus)	// DenebMLK以外.
		printf( "          1:%s\r\n", EC_PrinterAdjust_PaperName1 );
		printf( "          2:%s\r\n", EC_PrinterAdjust_PaperName2 );
		printf( "          3:%s\r\n", EC_PrinterAdjust_PaperName3 );
		printf( "          4:%s\r\n", EC_PrinterAdjust_PaperName4 );
		printf( "          5:%s\r\n", EC_PrinterAdjust_PaperName5 );
		printf( "          6:%s\r\n", EC_PrinterAdjust_PaperName6 );
		printf( "          7:%s\r\n", EC_PrinterAdjust_PaperName7 );
		printf( "          8:%s\r\n", EC_PrinterAdjust_TopMargin1stTrayS );
		printf( "          9:%s\r\n", EC_PrinterAdjust_TopMargin1stTrayL );
		printf( "         10:%s\r\n", EC_PrinterAdjust_TopMargin2nd3rd4thLCTTray );
		printf( "         11:%s\r\n", EC_PrinterAdjust_TopMarginManualTray );
		printf( "         12:%s\r\n", EC_PrinterAdjust_TopMarginManualThin );
#else	// !defined(DEF_DenebMLKCus) DenebMLK機種.
		printf( "         13:%s\r\n", EC_PrinterAdjust_PaperName13 );
		printf( "         14:%s\r\n", EC_PrinterAdjust_PaperName14 );
		printf( "         15:%s\r\n", EC_PrinterAdjust_PaperName15 );
		printf( "         16:%s\r\n", EC_PrinterAdjust_PaperName16 );
		printf( "         17:%s\r\n", EC_PrinterAdjust_PaperName17 );
		printf( "         18:%s\r\n", EC_PrinterAdjust_PaperName18 );
		printf( "         19:%s\r\n", EC_PrinterAdjust_PaperName19 );
		printf( "         20:%s\r\n", EC_PrinterAdjust_PaperName20 );
		printf( "         21:%s\r\n", EC_PrinterAdjust_PaperName21 );
		printf( "         22:%s\r\n", EC_PrinterAdjust_PaperName22 );
		printf( "         23:%s\r\n", EC_PrinterAdjust_PaperName23 );
		printf( "         24:%s\r\n", EC_PrinterAdjust_PaperName24 );
		printf( "         25:%s\r\n", EC_PrinterAdjust_PaperName25 );
#endif // !defined(DEF_DenebMLKCus).
#else	// defined(DEF_A4_Cus) Add Start.
		printf( "          1:%s\r\n", EC_PrinterAdjust_PaperName1 );
		printf( "          2:%s\r\n", EC_PrinterAdjust_PaperName2 );
		printf( "          3:%s\r\n", EC_PrinterAdjust_PaperName3 );
		printf( "          4:%s\r\n", EC_PrinterAdjust_PaperName4 );
// [changelog] 2016/07/05 AccuRev環境マージ VenusMLK >>.
//-> VenusMLK トレイ先端余白補正値追加 2016.04.04.
		printf( "          8:%s\r\n", EC_PrinterAdjust_TopMargin1stTrayS );
// Str【VenusMLK CQ526598 2016.04.18 カセット1long未使用】.
//		printf( "          9:%s\r\n", EC_PrinterAdjust_TopMargin1stTrayL );
// End【VenusMLK CQ526598 2016.04.18 カセット1long未使用】.
		if( (TYPE_ModelTypeIII == THRG_GetModelType())	&&
			(False == THRF_isSparrow2AIO()) ){
			printf( "         10:%s\r\n", EC_PrinterAdjust_TopMargin2nd3rd4thLCTTray );
		}else{
			printf( "         10:%s\r\n", EC_PrinterAdjust_TopMargin2nd3rdTray );
		}
		printf( "         11:%s\r\n", EC_PrinterAdjust_TopMarginManualTray );
//<- 2016.04.04.
// [changelog] 2016/07/05 AccuRev環境マージ VenusMLK <<.
#endif	//<- !defined(DEF_A4_Cus) Change End.
//Str【EagleZ+ OP_BTS-15006対応 : Y.Okumoto : 2019.10.21】.
		printf( "         26:%s\r\n", EC_PrinterAdjust_PaperPlainPlus );
//End【EagleZ+ OP_BTS-15006対応 : Y.Okumoto : 2019.10.21】.
		//エラー終了.
		return;
		break;
	}
	//共有データから値を取得.
	ss_Value = pc_PrinterServiceSetting->getPrinterAdjust( e_Type );
	printf( " PrinterAdjustSubRegist %s = %d\r\n", sc_Name, ss_Value );
}
/*================================================================================================
［機能］	ターミナルコマンド（先端プリント位置調整値設定） 
--------------------------------------------------------------------------------------------------
［引数］	short ss_Type				：用紙種類 
										： Eagle(1～12).
										：  1:Plain Paper				(普通紙).
										：  2:Thick Paper1/1+			(厚紙1/1+).
										：  3:Thick Paper2				(厚紙2).
										：  4:Thick Paper3				(厚紙3).
										：  5:Thick Paper4				(厚紙4).
										：  6:OHP Film					(OHP).
										：  7:Envelope					(封筒).
										：  8:Tray1 short				(トレイ1 short).
										：  9:Tray1 long				(トレイ1 long).
										：  10:Tray2_3_4T				(トレイ2/3/4).
										：  11:Manual					(手差し).
										：  12:Thin Paper				(薄紙).
										： DenebMLKのみ(13～23).
										：  13:Tray1_Plain Paper		(トレイ1+普通紙).
										：  14:Tray1_Thick Paper1/1+	(トレイ1+厚紙1/1+).
										：  15:Tray1_Envelope			(トレイ1+封筒).
										：  16:Tray2					(トレイ2).
										：  17:Tray3					(トレイ3).
										：  18:Tray4					(トレイ4).
										：  19:Tray5					(トレイ5).
										：  20:Manual_Plain Paper		(手差し+普通紙).
										：  21:Manual_Thick Paper1/1+	(手差し+厚紙1/1+).
										：  22:Manual_OHP Film			(手差し+OHP).
										：  23:Manual_Envelope			(手差し+封筒).
			short ss_Value				：設定値 
										：  -30～30 
										：  DenebMLKのみ.
										：  -40～40.
										：  設定する値を10倍値で指定する。さらにステップは2の倍数 
										：  になるように設定する。 
										：  ※実際の範囲は、-3.0mm～3.0mm 
										：  ※実際のステップは、0.2mm単位 
										：  ※11(1.1mm)や-5(-0.5mm)はエラー 
［戻値］	無し 
［詳細］ 引数で受け取った用紙種別と設定値が仕様内かを判定し、仕様内の場合は受け取った設定値を設定する.
		 仕様外の値が引数で指定された場合は指定する値の一覧と設定値の範囲をターミナルに出力する.
［ｳｫｰｸｽﾙｰ］	未実施 
--------------------------------------------------------------------------------------------------
 [履歴] 
 '06/08/04 N.UDA(REVSONIC) 新規作成 
 '08/09/03 furuno CQ309029 Amur対応.「給紙口毎のトップマージン調整」に対応.パラメータ8～11を追加.
 '18/11/08 Sky松井	[DenebMLK]AR.1065528 DenebMLK時に引数が13～23のケースを追加.
							  上記と共にAR.1065532 DenebMLK時の引数エラー出力内容を追加.
 '19/01/28 Sky		[DenebMLK]AR.1072240 DenebMLK時に引数が24、25のケースを追加.
 '23/06/29 Sky		OP_BTS-43745 IT6_2.3 Sparrow 給紙トレイ4段対応.
================================================================================================*/
void SetPrinterAdjustSubRegist( short ss_Type, short ss_Value )
{
	TYPE_PrinterAdjust 	e_Type = TYPE_PrinterAdjustSubRegistNormal;				//用紙種類.
	short ss_OldValue;						//変更前の設定値.
	bool bl_error = False;					//エラー検出フラグ(True:検出,False:未検出).
	//パラメータチェック.
	switch ( ss_Type ) {
#if !defined(DEF_A4_Cus)	//-> 2015.12.14 Change Start.
#if !defined(DEF_DenebMLKCus)	// DenebMLK以外.
	case 1:									//1:Plain Paper     (普通紙).
		e_Type = TYPE_PrinterAdjustSubRegistNormal;
		break;
	case 2:									//2:Thick Paper1/1+ (厚紙1/1+).
		e_Type = TYPE_PrinterAdjustSubRegistHeavy1;
		break;
	case 3:									//3:Thick Paper2    (厚紙2).
		e_Type = TYPE_PrinterAdjustSubRegistHeavy2;
		break;
	case 4:									//4:Thick Paper3    (厚紙3).
		e_Type = TYPE_PrinterAdjustSubRegistHeavy3;
		break;
	case 5:									//5:Thick Paper4    (厚紙4).
		e_Type = TYPE_PrinterAdjustSubRegistHeavy4;
		break;
	case 6:									//6:OHP Film        (OHP).
		e_Type = TYPE_PrinterAdjustSubRegistOHP;
		break;
	case 7:									//7:Envelope        (封筒).
		e_Type = TYPE_PrinterAdjustSubRegistEnvelope;
		break;
	case 8:									//8:Tray1 short     (トレイ1 short).
		e_Type = TYPE_PrinterAdjustTopMargin1stTrayS;
		break;
	case 9:									//9:Tray1 long      (トレイ1 long).
		e_Type = TYPE_PrinterAdjustTopMargin1stTrayL;
		break;
	case 10:								//10:Tray2_3_4      (トレイ2/3/4).
		e_Type = TYPE_PrinterAdjustTopMargin2nd3rd4thLCTTray;
		break;
	case 11:								//11:Manual         (手差し).
		e_Type = TYPE_PrinterAdjustTopMarginManualTray;
		break;
	case 12:								//12:副走査倍率微調 薄紙.
		e_Type = TYPE_PrinterAdjustSubRegistThin;
		break;
#else	// !defined(DEF_DenebMLKCus) DenebMLK機種.
	case 13:								// 13：Tray1_Plain Paper(トレイ1+普通紙).
		e_Type = TYPE_PrinterAdjustSubRegistNormal;
		break;
	case 14:								// 14：Tray1_Thick Paper1/1+(トレイ1+厚紙1/1+).
		e_Type = TYPE_PrinterAdjustSubRegistHeavy1;
		break;
	case 15:								// 15：Tray1_Envelope(トレイ１+封筒).
		e_Type = TYPE_PrinterAdjustSubRegistEnvelope;
		break;
	case 16:								// 16：Tray2(トレイ2).
		e_Type = TYPE_PrinterAdjustTopMargin2ndTray;
		break;
	case 17:								// 17：Tray3(トレイ3).
		e_Type = TYPE_PrinterAdjustTopMargin3rdTray;
		break;
	case 18:								// 18：Tray4(トレイ4).
		e_Type = TYPE_PrinterAdjustTopMargin4thTray;
		break;
	case 19:								// 19：Tray5(トレイ5).
		e_Type = TYPE_PrinterAdjustTopMargin5thTray;
		break;
	case 20:								// 20：Manual_Plain Paper(手差し+普通紙).
		e_Type = TYPE_PrinterAdjustTopMarginManualTray;
		break;
	case 21:								// 21：Manual_Thick Paper1/1+(手差し+厚紙1/1+).
		e_Type = TYPE_PrinterAdjustSubRegistHeavy1ManualTray;
		break;
	case 22:								// 22：Manual_OHP Film(手差し+OHP).
		e_Type = TYPE_PrinterAdjustSubRegistOHPManualTray;
		break;
	case 23:								// 23：Manual_Envelope(手差し+封筒).
		e_Type = TYPE_PrinterAdjustSubRegistEnvelopeManualTray;
		break;
	case 24:								// 24：Tray1_Plain Paper+(トレイ１_普通紙＋).
		e_Type = TYPE_PrinterAdjustSubRegistNormalPlus;
		break;
	case 25:								// 25：Manual_Plain Paper+(手差し_普通紙＋).
		e_Type = TYPE_PrinterAdjustSubRegistNormalPlusManualTray;
		break;
#endif // !defined(DEF_DenebMLKCus).
#else // defined(DEF_A4_Cus) 2015.12.14 Add Start.
	case 1:									//1:Plain Paper     (普通紙).
		e_Type = TYPE_PrinterAdjustSubRegistNormal;
		break;
	case 2:									//2:光沢モード.
		e_Type = TYPE_PrinterAdjustSubRegistGlossy;
		break;
	case 3:									//3:厚紙グループ.
		e_Type = TYPE_PrinterAdjustSubRegistHeavyGr;
		break;
	case 4:									//4:Envelope        (封筒).
		e_Type = TYPE_PrinterAdjustSubRegistEnvelope;
		break;
// [changelog] 2016/07/05 AccuRev環境マージ VenusMLK >>.
//-> VenusMLK トレイ先端余白補正値追加 2016.04.04.
	case 8:									//8:Tray1 short     (トレイ1 short).
		e_Type = TYPE_PrinterAdjustTopMargin1stTrayS;
		break;
// Str【VenusMLK CQ526598 2016.04.18 カセット1long未使用】.
//	case 9:									//9:Tray1 long      (トレイ1 long).
//		e_Type = TYPE_PrinterAdjustTopMargin1stTrayL;
//		break;
// End【VenusMLK CQ526598 2016.04.18 カセット1long未使用】.
	case 10:								//10:Tray2_3        (トレイ2/3).
		e_Type = TYPE_PrinterAdjustTopMargin2nd3rd4thLCTTray;
		break;
	case 11:								//11:Manual         (手差し).
		e_Type = TYPE_PrinterAdjustTopMarginManualTray;
		break;
//<- 2016.04.04.
// [changelog] 2016/07/05 AccuRev環境マージ VenusMLK <<.
#endif	//<- !defined(DEF_A4_Cus) 2015.12.14 Change End.
//Str【EagleZ+ OP_BTS-15006対応 : Y.Okumoto : 2019.10.21】.
	case 26:								//26:Plain Paper+   (普通紙+).
		e_Type = TYPE_PrinterAdjustSubRegistNormalPlus;
		break;
//End【EagleZ+ OP_BTS-15006対応 : Y.Okumoto : 2019.10.21】.
	default:								//用紙種類エラー.
		//エラー検出!!.
		bl_error = True;
		break;
	}
	//エラー未検出?.
	if ( bl_error == False ) {
		//設定値の範囲チェックエラー?.
//		if ( ss_Value > EC_PrinterAdjust_RegistValue_Max || ss_Value < EC_PrinterAdjust_RegistValue_Min ) {
// [changelog] 2016/07/05 AccuRev環境マージ VenusMLK >>.
#if defined(DEF_A4_Cus) //-> VenusMLK 封筒の補正値最大値の変更(+30=>+60) 2016.04.04.
		if ( ( ss_Type != 4 && ( ss_Value > EC_PrinterAdjust_SubRegistValue_Max || ss_Value < EC_PrinterAdjust_SubRegistValue_Min ) )
		  || ( ss_Type == 4 && ( ss_Value > EC_PrinterAdjust_SubRegistValue_Envelope_Max || ss_Value < EC_PrinterAdjust_SubRegistValue_Min ) ) )
		{
#else // !defined(DEF_A4_Cus).
		if ( ss_Value > EC_PrinterAdjust_SubRegistValue_Max || ss_Value < EC_PrinterAdjust_SubRegistValue_Min ) {
#endif //<- defined(DEF_A4_Cus) 2016.04.04.
// [changelog] 2016/07/05 AccuRev環境マージ VenusMLK <<.
			//エラー検出!!.
			bl_error = True;
		}
		//設定値のSTEPチェックエラー?(余りがある?).
		else if ( ss_Value % EC_PrinterAdjust_RegistValue_Step ) {
			//エラー検出!!.
			bl_error = True;
		}
	}
	//エラー検出した?.
	if ( bl_error == True ) {
		printf( " [Format]SetPrinterAdjustSubRegist Type, Value\r\n" );
		printf( "  Type ... Paper Kind\r\n" );
#if !defined(DEF_A4_Cus)	//-> 2015.12.14 change Start.
#if !defined(DEF_DenebMLKCus)	// DenebMLK以外.
		printf( "           1:%s\r\n", EC_PrinterAdjust_PaperName1 );
		printf( "           2:%s\r\n", EC_PrinterAdjust_PaperName2 );
		printf( "           3:%s\r\n", EC_PrinterAdjust_PaperName3 );
		printf( "           4:%s\r\n", EC_PrinterAdjust_PaperName4 );
		printf( "           5:%s\r\n", EC_PrinterAdjust_PaperName5 );
		printf( "           6:%s\r\n", EC_PrinterAdjust_PaperName6 );
		printf( "           7:%s\r\n", EC_PrinterAdjust_PaperName7 );
		printf( "           8:%s\r\n", EC_PrinterAdjust_TopMargin1stTrayS );
		printf( "           9:%s\r\n", EC_PrinterAdjust_TopMargin1stTrayL );
		printf( "          10:%s\r\n", EC_PrinterAdjust_TopMargin2nd3rd4thLCTTray );
		printf( "          11:%s\r\n", EC_PrinterAdjust_TopMarginManualTray );
		printf( "          12:%s\r\n", EC_PrinterAdjust_TopMarginManualThin );
#else	// !defined(DEF_DenebMLKCus) DenebMLK機種.
		printf( "          13:%s\r\n", EC_PrinterAdjust_PaperName13 );
		printf( "          14:%s\r\n", EC_PrinterAdjust_PaperName14 );
		printf( "          15:%s\r\n", EC_PrinterAdjust_PaperName15 );
		printf( "          16:%s\r\n", EC_PrinterAdjust_PaperName16 );
		printf( "          17:%s\r\n", EC_PrinterAdjust_PaperName17 );
		printf( "          18:%s\r\n", EC_PrinterAdjust_PaperName18 );
		printf( "          19:%s\r\n", EC_PrinterAdjust_PaperName19 );
		printf( "          20:%s\r\n", EC_PrinterAdjust_PaperName20 );
		printf( "          21:%s\r\n", EC_PrinterAdjust_PaperName21 );
		printf( "          22:%s\r\n", EC_PrinterAdjust_PaperName22 );
		printf( "          23:%s\r\n", EC_PrinterAdjust_PaperName23 );
		printf( "          24:%s\r\n", EC_PrinterAdjust_PaperName24 );
		printf( "          25:%s\r\n", EC_PrinterAdjust_PaperName25 );
#endif // !defined(DEF_DenebMLKCus).
#else	// defined(DEF_A4_Cus) 2015.12.15 Add Start.
		printf( "           1:%s\r\n", EC_PrinterAdjust_PaperName1 );
		printf( "           2:%s\r\n", EC_PrinterAdjust_PaperName2 );
		printf( "           3:%s\r\n", EC_PrinterAdjust_PaperName3 );
		printf( "           4:%s\r\n", EC_PrinterAdjust_PaperName4 );
// [changelog] 2016/07/05 AccuRev環境マージ VenusMLK >>.
//-> VenusMLK トレイ先端余白補正値追加 2016.04.04.
		printf( "           8:%s\r\n", EC_PrinterAdjust_TopMargin1stTrayS );
// Str【VenusMLK CQ526598 2016.04.18 カセット1long未使用】.
//		printf( "           9:%s\r\n", EC_PrinterAdjust_TopMargin1stTrayL );
// End【VenusMLK CQ526598 2016.04.18 カセット1long未使用】.
		if( (TYPE_ModelTypeIII == THRG_GetModelType())	&&
			(False == THRF_isSparrow2AIO()) ){
			printf( "          10:%s\r\n", EC_PrinterAdjust_TopMargin2nd3rd4thLCTTray );
		}
		else
		{
			printf( "          10:%s\r\n", EC_PrinterAdjust_TopMargin2nd3rdTray );
		}
		printf( "          11:%s\r\n", EC_PrinterAdjust_TopMarginManualTray );
//<- 2016.04.04.
// [changelog] 2016/07/05 AccuRev環境マージ VenusMLK <<.
#endif	//<- !defined(DEF_A4_Cus) Change End.
//Str【EagleZ+ OP_BTS-15006対応 : Y.Okumoto : 2019.10.21】.
		printf( "          26:%s\r\n", EC_PrinterAdjust_PaperPlainPlus );
//End【EagleZ+ OP_BTS-15006対応 : Y.Okumoto : 2019.10.21】.
		printf( " Value ... Setting Value\r\n" );
//		printf( "           From -100 up to 100.\r\n" );
		printf( "           From %d up to %d.\r\n" , EC_PrinterAdjust_SubRegistValue_Min, EC_PrinterAdjust_SubRegistValue_Max);
// [changelog] 2016/07/05 AccuRev環境マージ VenusMLK >>.
#if defined(DEF_A4_Cus) //-> VenusMLK トレイ先端余白補正値追加 2016.04.04.
		printf( "           Envelope only %d up to %d.\r\n", EC_PrinterAdjust_SubRegistValue_Min, EC_PrinterAdjust_SubRegistValue_Envelope_Max);
#endif //<- defined(DEF_A4_Cus) 2016.04.04.
// [changelog] 2016/07/05 AccuRev環境マージ VenusMLK <<.
		printf( "           The internal scale (10 times value) with please appoint.\r\n" );
		printf( "           Step please makes multiple of 2.\r\n" );
		//エラー終了.
		return;
	}
	//共有データから現在値を取得.
	ss_OldValue = pc_PrinterServiceSetting->getPrinterAdjust( e_Type );
	printf( " Last Value = %d\r\n", ss_OldValue );
	//共有データに値を設定.
	pc_PrinterServiceSetting->setPrinterAdjust( e_Type, ss_Value );
	printf( "  New Value = %d\r\n", ss_Value );
}
/*================================================================================================
［機能］	ターミナルコマンド（側端プリント位置調整値表示） 
--------------------------------------------------------------------------------------------------
［引数］	short ss_Tray				：トレイ種類 
										：　1:1st     (1段目) 
										：　2:2nd     (2段目) 
										：　3:3rd     (3段目) 
										：　4:4th     (4段目) 
										：　5:Manual  (手差し)          DenebMLKの場合、5th (5段目).
										：　6:LCT     (LCT) ⇒ Amurなし DenebMLKの場合、Manual  (手差し).
［戻値］	無し 
［詳細］ 
［ｳｫｰｸｽﾙｰ］	未実施 
--------------------------------------------------------------------------------------------------
 [履歴] 
 '06/08/04 N.UDA(REVSONIC) 新規作成 
 '08/09/03 furuno CQ309029 Amur対応.AmurにはLCTがつかないのではずす.
 '18/07/17 Kaihotsu 【DenebMLK 生産コマンド対応（コマンド追加対応）:機器構成変更_No_13-2 チョイス調整機能変更対応】.
 '23/06/29 Sky		OP_BTS-43745 IT6_2.3 Sparrow 給紙トレイ4段対応.
================================================================================================*/
void GetPrinterAdjustMainRegist( short ss_Tray )
{
	TYPE_PrinterAdjust 	e_Tray;								//トレイ種類.
	short ss_Value;											//取得した設定値.
	char sc_Name[ EC_PrinterAdjust_TrayNameLenMax + 1 ];	//トレイ種類名(最後はNull分).
	//パラメータチェック.
	switch ( ss_Tray ) {
	case 1:									//1:1st    (1段目).
		e_Tray = TYPE_PrinterAdjustMainRegist1st;
		sprintf( sc_Name, "%s", EC_PrinterAdjust_TrayName1 );
		break;
	case 2:									//2:2nd    (2段目).
		e_Tray = TYPE_PrinterAdjustMainRegist2nd;
		sprintf( sc_Name, "%s", EC_PrinterAdjust_TrayName2 );
		break;
	case 3:									//3:3rd    (3段目).
		e_Tray = TYPE_PrinterAdjustMainRegist3rd;
		sprintf( sc_Name, "%s", EC_PrinterAdjust_TrayName3 );
		break;
#if !defined(DEF_A4_Cus)	//-> 2016.01.28 Start.
	case 4:									//4:4th    (4段目).
		e_Tray = TYPE_PrinterAdjustMainRegist4th;
		sprintf( sc_Name, "%s", EC_PrinterAdjust_TrayName4 );
		break;
#if defined(DEF_DenebMLKCus)	// DenebMLK.
	case 5:									//5:5th    (5段目).
		e_Tray = TYPE_PrinterAdjustMainRegistLCT;
		sprintf( sc_Name, "%s", EC_PrinterAdjust_TrayName5 );
		break;
	case 6:									//6:Manual (手差し).
		e_Tray = TYPE_PrinterAdjustMainRegistHand;
		sprintf( sc_Name, "%s", EC_PrinterAdjust_TrayName6 );
		break;
#else
	case 5:									//5:Manual (手差し).
		e_Tray = TYPE_PrinterAdjustMainRegistHand;
		sprintf( sc_Name, "%s", EC_PrinterAdjust_TrayName5 );
		break;
	case 6:									//6:LCT    (LCT).
		e_Tray = TYPE_PrinterAdjustMainRegistLCT;
		sprintf( sc_Name, "%s", EC_PrinterAdjust_TrayName6 );
		break;
#endif // DenebMLK.
#else	// defined(DEF_A4_Cus).
	case 4:									// 4:Manual (手差し).
		if( (TYPE_ModelTypeIII == THRG_GetModelType())	&&
			(False == THRF_isSparrow2AIO()) ){
			e_Tray = TYPE_PrinterAdjustMainRegist4th;
			sprintf( sc_Name, "%s", EC_PrinterAdjust_TrayName4 );

		}else{
			e_Tray = TYPE_PrinterAdjustMainRegistHand;
			sprintf( sc_Name, "%s", EC_PrinterAdjust_TrayName5 );
		}
		break;
	case 5:
        if( (TYPE_ModelTypeIII == THRG_GetModelType()) &&
            (False == THRF_isSparrow2AIO()) ){
			e_Tray = TYPE_PrinterAdjustMainRegistHand;
			sprintf( sc_Name, "%s", EC_PrinterAdjust_TrayName5 );
			break;
		}else{
            // 5がないため、breakせずdefaultに流す.
		}		
#endif	//<- !defined(DEF_A4_Cus) 2016.01.28 End.
	default:								//トレイ種類エラー.
		printf( " [Format]GetPrinterAdjustMainRegist Tray\r\n" );
		printf( " Tray ... Tray Kind\r\n" );
		printf( "          1:%s\r\n", EC_PrinterAdjust_TrayName1 );
		printf( "          2:%s\r\n", EC_PrinterAdjust_TrayName2 );
		printf( "          3:%s\r\n", EC_PrinterAdjust_TrayName3 );
#if !defined(DEF_A4_Cus)	//-> 2016.01.28 Start.
		printf( "          4:%s\r\n", EC_PrinterAdjust_TrayName4 );
		printf( "          5:%s\r\n", EC_PrinterAdjust_TrayName5 );
		printf( "          6:%s\r\n", EC_PrinterAdjust_TrayName6 );
#else	// defined(DEF_A4_Cus).
		if( (TYPE_ModelTypeIII == THRG_GetModelType()) &&
			(False == THRF_isSparrow2AIO()) ){
			printf( "          4:%s\r\n", EC_PrinterAdjust_TrayName4 );
			printf( "          5:%s\r\n", EC_PrinterAdjust_TrayName5 );
		}else{
			printf( "          4:%s\r\n", EC_PrinterAdjust_TrayName5 );
		}
#endif	//<- !defined(DEF_A4_Cus) 2016.01.28 End.
		//エラー終了.
		return;
		break;
	}
	//共有データから値を取得.
	ss_Value = pc_PrinterServiceSetting->getPrinterAdjust( e_Tray );
	printf( " PrinterAdjustMainRegist %s = %d\r\n", sc_Name, ss_Value );
}
/*================================================================================================
［機能］	ターミナルコマンド（側端プリント位置調整値設定） 
--------------------------------------------------------------------------------------------------
［引数］	short ss_Tray				：トレイ種類 
										：　1:1st     (1段目) 
										：　2:2nd     (2段目) 
										：　3:3rd     (3段目) 
										：　4:4th     (4段目) 
										：　5:Manual  (手差し)          DenebMLKの場合、5th (5段目).
										：　6:LCT     (LCT) ⇒ Amurなし DenebMLKの場合、Manual  (手差し).
			short ss_Value				：設定値 
										：  -30～30 
										：  設定する値を10倍値で指定する。さらにステップは2の倍数 
										：  になるように設定する。 
										：  ※実際の範囲は、-3.0mm～3.0mm 
										：  ※実際のステップは、0.2mm単位 
										：  ※11(1.1mm)や-5(-0.5mm)はエラー 
［戻値］	無し 
［詳細］ 
［ｳｫｰｸｽﾙｰ］	未実施 
--------------------------------------------------------------------------------------------------
 [履歴] 
 '06/08/04 N.UDA(REVSONIC) 新規作成 
 '08/09/03 furuno CQ309029 Amur対応.AmurにはLCTがつかないのではずす.
 '18/07/17 Kaihotsu 【DenebMLK 生産コマンド対応（コマンド追加対応）:機器構成変更_No_13-2 チョイス調整機能変更対応】.
 '23/06/29 Sky		OP_BTS-43745 IT6_2.3 Sparrow 給紙トレイ4段対応.
================================================================================================*/
void SetPrinterAdjustMainRegist( short ss_Tray, short ss_Value )
{
	TYPE_PrinterAdjust 	e_Tray = TYPE_PrinterAdjustMainRegist1st;				//トレイ種類.
	short ss_OldValue;						//変更前の設定値.
	bool bl_error = False;					//エラー検出フラグ(True:検出,False:未検出).
	//パラメータチェック.
	switch ( ss_Tray ) {
	case 1:									//1:1st    (1段目).
		e_Tray = TYPE_PrinterAdjustMainRegist1st;
		break;
	case 2:									//2:2nd    (2段目).
		e_Tray = TYPE_PrinterAdjustMainRegist2nd;
		break;
	case 3:									//3:3rd    (3段目).
		e_Tray = TYPE_PrinterAdjustMainRegist3rd;
		break;
#if !defined(DEF_A4_Cus)	//-> 2016.01.28 Start.
	case 4:									//4:4th    (4段目).
		e_Tray = TYPE_PrinterAdjustMainRegist4th;
		break;
#if defined(DEF_DenebMLKCus)	// DenebMLK.
	case 5:									//5:5th    (5段目).
		e_Tray = TYPE_PrinterAdjustMainRegistLCT;
		break;
	case 6:									//6:Manual (手差し).
		e_Tray = TYPE_PrinterAdjustMainRegistHand;
		break;
#else
	case 5:									//5:Manual (手差し).
		e_Tray = TYPE_PrinterAdjustMainRegistHand;
		break;
	case 6:									//6:LCT    (LCT).
		e_Tray = TYPE_PrinterAdjustMainRegistLCT;
		break;
#endif // DenebMLK.
#else	// defined(DEF_A4_Cus).
	case 4:									//5:Manual (手差し).
		if( (TYPE_ModelTypeIII == THRG_GetModelType()) &&
			(False == THRF_isSparrow2AIO()) ){
			e_Tray = TYPE_PrinterAdjustMainRegist4th;
		}else{
			e_Tray = TYPE_PrinterAdjustMainRegistHand;
		}
		break;
	case 5:
		if( (TYPE_ModelTypeIII == THRG_GetModelType()) &&
			(False == THRF_isSparrow2AIO()) ){
			e_Tray = TYPE_PrinterAdjustMainRegistHand;
			break;
		}else{
			// 5がないため、breakせずdefaultに流す.
		}
#endif	//<- !defined(DEF_A4_Cus) 2016.01.28 End.
	default:								//トレイ種類エラー.
		//エラー検出!!.
		bl_error = True;
		break;
	}
	//エラー未検出?.
	if ( bl_error == False ) {
		//設定値の範囲チェックエラー?.
//		if ( ss_Value > EC_PrinterAdjust_RegistValue_Max || ss_Value < EC_PrinterAdjust_RegistValue_Min ) {
		if ( ss_Value > EC_PrinterAdjust_MainRegistValue_Max || ss_Value < EC_PrinterAdjust_MainRegistValue_Min ) {
			//エラー検出!!.
			bl_error = True;
		}
		//設定値のSTEPチェックエラー?(余りがある?).
		else if ( ss_Value % EC_PrinterAdjust_RegistValue_Step ) {
			//エラー検出!!.
			bl_error = True;
		}
	}
	//エラー検出した?.
	if ( bl_error == True ) {
		printf( " [Format]SetPrinterAdjustMainRegist Tray, Value\r\n" );
		printf( "  Tray ... Tray Kind\r\n" );
		printf( "           1:%s\r\n", EC_PrinterAdjust_TrayName1 );
		printf( "           2:%s\r\n", EC_PrinterAdjust_TrayName2 );
		printf( "           3:%s\r\n", EC_PrinterAdjust_TrayName3 );
#if !defined(DEF_A4_Cus)	//-> 2016.01.28 Start.
		printf( "           4:%s\r\n", EC_PrinterAdjust_TrayName4 );
		printf( "           5:%s\r\n", EC_PrinterAdjust_TrayName5 );
		printf( "           6:%s\r\n", EC_PrinterAdjust_TrayName6 );
#else	// defined(DEF_A4_Cus).
		if( (TYPE_ModelTypeIII == THRG_GetModelType())	&&
			(False == THRF_isSparrow2AIO()) ){
			printf( "           4:%s\r\n", EC_PrinterAdjust_TrayName4 );
			printf( "           5:%s\r\n", EC_PrinterAdjust_TrayName5 );
		}else{
			printf( "           4:%s\r\n", EC_PrinterAdjust_TrayName5 );
		}
#endif	//<- !defined(DEF_A4_Cus) 2016.01.28 End.
		printf( " Value ... Setting Value\r\n" );
//		printf( "           From -100 up to 100.\r\n" );
		printf( "           From %d up to %d.\r\n", EC_PrinterAdjust_MainRegistValue_Min, EC_PrinterAdjust_MainRegistValue_Max );
		printf( "           The internal scale (10 times value) with please appoint.\r\n" );
		printf( "           Step please makes multiple of 2.\r\n" );
		//エラー終了.
		return;
	}
	//共有データから現在値を取得.
	ss_OldValue = pc_PrinterServiceSetting->getPrinterAdjust( e_Tray );
	printf( " Last Value = %d\r\n", ss_OldValue );
	//共有データに値を設定.
	pc_PrinterServiceSetting->setPrinterAdjust( e_Tray, ss_Value );
	printf( "  New Value = %d\r\n", ss_Value );
}
/*================================================================================================
［機能］	ターミナルコマンド（両面先端プリント位置調整値表示） 
--------------------------------------------------------------------------------------------------
［引数］	short ss_Type				：用紙種類 
										：　1:Plain Paper     (普通紙) 
										：　2:Thick Paper1/1+ (厚紙1/1+) 
										：　3:Thick Paper2    (厚紙2)    DenebMLKは削除.
										：　4:Thick Paper3    (厚紙3)    DenebMLKは削除.
［戻値］	無し 
［詳細］ 
［ｳｫｰｸｽﾙｰ］	未実施 
--------------------------------------------------------------------------------------------------
 [履歴] 
 '06/08/04 N.UDA(REVSONIC) 新規作成 
 '18/07/17 Kaihotsu 【DenebMLK 生産コマンド対応（コマンド追加対応）:機器構成変更_No_13-2 チョイス調整機能変更対応】.
================================================================================================*/
void GetPrinterAdjust2ndSubRegist( short ss_Type )
{
	TYPE_PrinterAdjust 	e_Type;								//用紙種類.
	short ss_Value;											//取得した設定値.
	char sc_Name[ EC_PrinterAdjust_PaperNameLenMax + 1 ];	//用紙種類名(最後はNull分).
	//パラメータチェック.
	switch ( ss_Type ) {
#if !defined(DEF_A4_Cus) //- 2015.12.15 Change Start.
	case 1:									//1:Plain Paper     (普通紙).
		e_Type = TYPE_PrinterAdjust2ndSubRegistNormal;
		sprintf( sc_Name, "%s", EC_PrinterAdjust_PaperName1 );
		break;
	case 2:									//2:Thick Paper1/1+ (厚紙1/1+).
		e_Type = TYPE_PrinterAdjust2ndSubRegistHeavy1;
		sprintf( sc_Name, "%s", EC_PrinterAdjust_PaperName2 );
		break;
#if !defined(DEF_DenebMLKCus)	// !DenebMLK.
	case 3:									//3:Thick Paper2    (厚紙2).
		e_Type = TYPE_PrinterAdjust2ndSubRegistHeavy2;
		sprintf( sc_Name, "%s", EC_PrinterAdjust_PaperName3 );
		break;
	case 4:									//4:Thick Paper3    (厚紙3).
		e_Type = TYPE_PrinterAdjust2ndSubRegistHeavy3;
		sprintf( sc_Name, "%s", EC_PrinterAdjust_PaperName4 );
		break;
	case 5:									//5：Thin Paper     (薄紙).
		e_Type = TYPE_PrinterAdjust2ndSubRegistThin;
		sprintf( sc_Name, "%s", EC_PrinterAdjust_PaperThin );
		break;
#endif // !DenebMLK.
#else // defined(DEF_A4_Cus) 2015.12.15 Add Start.
	case 1:									//1:Plain Paper     (普通紙).
		e_Type = TYPE_PrinterAdjust2ndSubRegistNormal;
		sprintf( sc_Name, "%s", EC_PrinterAdjust_PaperName1 );
		break;
	case 2:									//2: Glossy Mode	(光沢モード）.
		e_Type = TYPE_PrinterAdjust2ndSubRegistGlossy;
		sprintf( sc_Name, "%s", EC_PrinterAdjust_PaperName2 );
		break;
	case 3:									//3:Thick Paper1/1+ (厚紙1/1+).
		e_Type = TYPE_PrinterAdjust2ndSubRegistHeavy1;
		sprintf( sc_Name, "%s", EC_PrinterAdjust_PaperName3 );
		break;
#endif	//<- !defined(DEF_A4_Cus) 2015.12.15 Change End.
//Str【EagleZ+ OP_BTS-15006対応 : Y.Okumoto : 2019.10.21】.
	case 6:									//6:Plain Paper+    (普通紙+).
		e_Type = TYPE_PrinterAdjust2ndSubRegistNormalPlus;
		sprintf( sc_Name, "%s", EC_PrinterAdjust_PaperPlainPlus );
		break;
//End【EagleZ+ OP_BTS-15006対応 : Y.Okumoto : 2019.10.21】.
	default:								//用紙種類エラー.
		printf( " [Format]GetPrinterAdjust2ndSubRegist Type\r\n" );
		printf( " Type ... Paper Kind\r\n" );
		printf( "          1:%s\r\n", EC_PrinterAdjust_PaperName1 );
		printf( "          2:%s\r\n", EC_PrinterAdjust_PaperName2 );
#if !defined(DEF_DenebMLKCus)	// !DenebMLK.
		printf( "          3:%s\r\n", EC_PrinterAdjust_PaperName3 );
#endif // !DenebMLK.
#if !defined(DEF_A4_Cus)	//-> 2015.12.15 Change Start.
#if !defined(DEF_DenebMLKCus)	// !DenebMLK.
		printf( "          4:%s\r\n", EC_PrinterAdjust_PaperName4 );
		printf( "          5:%s\r\n", EC_PrinterAdjust_PaperThin );
#endif // !DenebMLK.
#endif	//<- !defined(DEF_A4_Cus) 2015.12.15 Change End.
//Str【EagleZ+ OP_BTS-15006対応 : Y.Okumoto : 2019.10.21】.
		printf( "          6:%s\r\n", EC_PrinterAdjust_PaperPlainPlus );
//End【EagleZ+ OP_BTS-15006対応 : Y.Okumoto : 2019.10.21】.
		//エラー終了.
		return;
		break;
	}
	//共有データから値を取得.
	ss_Value = pc_PrinterServiceSetting->getPrinterAdjust( e_Type );
	printf( " PrinterAdjust2ndSubRegist %s = %d\r\n", sc_Name, ss_Value );
}
/*================================================================================================
［機能］	ターミナルコマンド（両面先端プリント位置調整値設定） 
--------------------------------------------------------------------------------------------------
［引数］	short ss_Type				：用紙種類 
										：  1:Plain Paper     (普通紙) 
										：  2:Thick Paper1/1+ (厚紙1/1+) 
										：  3:Thick Paper2    (厚紙2)      DenebMLKは削除.
										：  4:Thick Paper3    (厚紙3)      DenebMLKは削除.
			short ss_Value				：設定値 
										：  -30～30 
										：  設定する値を10倍値で指定する。さらにステップは2の倍数 
										：  になるように設定する。 
										：  ※実際の範囲は、-3.0mm～3.0mm 
										：  ※実際のステップは、0.2mm単位 
										：  ※11(1.1mm)や-5(-0.5mm)はエラー 
［戻値］	無し 
［詳細］ 
［ｳｫｰｸｽﾙｰ］	未実施 
--------------------------------------------------------------------------------------------------
 [履歴] 
 '06/08/04 N.UDA(REVSONIC) 新規作成 
 '18/07/17 Kaihotsu 【DenebMLK 生産コマンド対応（コマンド追加対応）:機器構成変更_No_13-2 チョイス調整機能変更対応】.
================================================================================================*/
void SetPrinterAdjust2ndSubRegist( short ss_Type, short ss_Value )
{
	TYPE_PrinterAdjust 	e_Type = TYPE_PrinterAdjust2ndSubRegistNormal;				//用紙種類.
	short ss_OldValue;						//変更前の設定値.
	bool bl_error = False;					//エラー検出フラグ(True:検出,False:未検出).
	//パラメータチェック.
	switch ( ss_Type ) {
#if !defined(DEF_A4_Cus)	//-> 2015.12.15 Change Start.
	case 1:									//1:Plain Paper     (普通紙).
		e_Type = TYPE_PrinterAdjust2ndSubRegistNormal;
		break;
	case 2:									//2:Thick Paper1/1+ (厚紙1/1+).
		e_Type = TYPE_PrinterAdjust2ndSubRegistHeavy1;
		break;
#if !defined(DEF_DenebMLKCus)	// !DenebMLK.
	case 3:									//3:Thick Paper2    (厚紙2).
		e_Type = TYPE_PrinterAdjust2ndSubRegistHeavy2;
		break;
	case 4:									//4:Thick Paper3    (厚紙3).
		e_Type = TYPE_PrinterAdjust2ndSubRegistHeavy3;
		break;
	case 5:									//5：Thin Paper     (薄紙).
		e_Type = TYPE_PrinterAdjust2ndSubRegistThin;
		break;
#endif // !DenebMLK.
#else	// defined(DEF_A4_Cus) 2015.12.15 Add Start.
	case 1:									//1:Plain Paper     (普通紙).
		e_Type = TYPE_PrinterAdjust2ndSubRegistNormal;
		break;
	case 2:									//2: Glossy Mode	（光沢モード）.
		e_Type = TYPE_PrinterAdjust2ndSubRegistGlossy;
		break;
	case 3:									//3:Thick Paper1/1+ (厚紙1/1+).
		e_Type = TYPE_PrinterAdjust2ndSubRegistHeavy1;
		break;
#endif	//<- !defined(DEF_A4_Cus) 2015.12.15 Change End.
//Str【EagleZ+ OP_BTS-15006対応 : Y.Okumoto : 2019.10.21】.
	case 6:									//6:Plain Paper+    (普通紙+).
		e_Type = TYPE_PrinterAdjust2ndSubRegistNormalPlus;
		break;
//End【EagleZ+ OP_BTS-15006対応 : Y.Okumoto : 2019.10.21】.
	default:								//用紙種類エラー.
		//エラー検出!!.
		bl_error = True;
		break;
	}
	//エラー未検出?.
	if ( bl_error == False ) {
		//設定値の範囲チェックエラー?.
//		if ( ss_Value > EC_PrinterAdjust_RegistValue_Max || ss_Value < EC_PrinterAdjust_RegistValue_Min ) {
		if ( ss_Value > EC_PrinterAdjust_SubRegistValue_Max || ss_Value < EC_PrinterAdjust_SubRegistValue_Min ) {
			//エラー検出!!.
			bl_error = True;
		}
		//設定値のSTEPチェックエラー?(余りがある?).
		else if ( ss_Value % EC_PrinterAdjust_RegistValue_Step ) {
			//エラー検出!!.
			bl_error = True;
		}
	}
	//エラー検出した?.
	if ( bl_error == True ) {
		printf( " [Format]SetPrinterAdjust2ndSubRegist Type, Value\r\n" );
		printf( "  Type ... Paper Kind\r\n" );
		printf( "           1:%s\r\n", EC_PrinterAdjust_PaperName1 );
		printf( "           2:%s\r\n", EC_PrinterAdjust_PaperName2 );
#if !defined(DEF_DenebMLKCus)	// !DenebMLK.
		printf( "           3:%s\r\n", EC_PrinterAdjust_PaperName3 );
#endif // !DenebMLK.
#if !defined(DEF_A4_Cus)	//-> 2015.12.15 Change Start.
#if !defined(DEF_DenebMLKCus)	// !DenebMLK.
		printf( "           4:%s\r\n", EC_PrinterAdjust_PaperName4 );
		printf( "           5:%s\r\n", EC_PrinterAdjust_PaperThin );	// 薄紙.
#endif // !DenebMLK.
#endif	//<- !defined(DEF_A4_Cus) 2015.12.15 Change End.
//Str【EagleZ+ OP_BTS-15006対応 : Y.Okumoto : 2019.10.21】.
		printf( "           6:%s\r\n", EC_PrinterAdjust_PaperPlainPlus );	// 普通紙+.
//End【EagleZ+ OP_BTS-15006対応 : Y.Okumoto : 2019.10.21】.
		printf( " Value ... Setting Value\r\n" );
//		printf( "           From -100 up to 100.\r\n" );
		printf( "           From %d up to %d.\r\n", EC_PrinterAdjust_SubRegistValue_Min, EC_PrinterAdjust_SubRegistValue_Max );
		printf( "           The internal scale (10 times value) with please appoint.\r\n" );
		printf( "           Step please makes multiple of 2.\r\n" );
		//エラー終了.
		return;
	}
	//共有データから現在値を取得.
	ss_OldValue = pc_PrinterServiceSetting->getPrinterAdjust( e_Type );
	printf( " Last Value = %d\r\n", ss_OldValue );
	//共有データに値を設定.
	pc_PrinterServiceSetting->setPrinterAdjust( e_Type, ss_Value );
	printf( "  New Value = %d\r\n", ss_Value );
}
/*================================================================================================
［機能］	ターミナルコマンド（両面側端プリント位置調整値表示） 
--------------------------------------------------------------------------------------------------
［引数］	short ss_Tray				：トレイ種類 
										：　1:1st     (1段目) 
										：　2:2nd     (2段目) 
										：　3:3rd     (3段目) 
										：　4:4th     (4段目) 
										：　5:Manual  (手差し)          DenebMLKの場合、5th (5段目).
										：　6:LCT     (LCT) ⇒ Amurなし DenebMLKの場合、Manual  (手差し).
［戻値］	無し 
［詳細］ 
［ｳｫｰｸｽﾙｰ］	未実施 
--------------------------------------------------------------------------------------------------
 [履歴] 
 '06/08/04 N.UDA(REVSONIC) 新規作成 
 '08/09/03 furuno CQ309029 Amur対応.AmurにはLCTがつかないのではずす.
 '18/07/17 Kaihotsu 【DenebMLK 生産コマンド対応（コマンド追加対応）:機器構成変更_No_13-2 チョイス調整機能変更対応】.
 '23/06/29 Sky		OP_BTS-43745 IT6_2.3 Sparrow 給紙トレイ4段対応.
================================================================================================*/
void GetPrinterAdjust2ndMainRegist( short ss_Tray )
{
	TYPE_PrinterAdjust 	e_Tray;								//トレイ種類.
	short ss_Value;											//取得した設定値.
	char sc_Name[ EC_PrinterAdjust_TrayNameLenMax + 1 ];	//トレイ種類名(最後はNull分).
	//パラメータチェック.
	switch ( ss_Tray ) {
	case 1:									//1:1st    (1段目).
		e_Tray = TYPE_PrinterAdjustMainRegist1st2;
		sprintf( sc_Name, "%s", EC_PrinterAdjust_TrayName1 );
		break;
	case 2:									//2:2nd    (2段目).
		e_Tray = TYPE_PrinterAdjustMainRegist2nd2;
		sprintf( sc_Name, "%s", EC_PrinterAdjust_TrayName2 );
		break;
	case 3:									//3:3rd    (3段目).
		e_Tray = TYPE_PrinterAdjustMainRegist3rd2;
		sprintf( sc_Name, "%s", EC_PrinterAdjust_TrayName3 );
		break;
#if !defined(DEF_A4_Cus)	//-> 2016.01.28 Start.
	case 4:									//4:4th    (4段目).
		e_Tray = TYPE_PrinterAdjustMainRegist4th2;
		sprintf( sc_Name, "%s", EC_PrinterAdjust_TrayName4 );
		break;
#if defined(DEF_DenebMLKCus)	// DenebMLK.
	case 5:									//5:5th    (5段目).
		e_Tray = TYPE_PrinterAdjustMainRegistLCT2;
		sprintf( sc_Name, "%s", EC_PrinterAdjust_TrayName5 );
		break;
	case 6:									//6:Manual (手差し).
		e_Tray = TYPE_PrinterAdjustMainRegistHand2;
		sprintf( sc_Name, "%s", EC_PrinterAdjust_TrayName6 );
		break;
#else
	case 5:									//5:Manual (手差し).
		e_Tray = TYPE_PrinterAdjustMainRegistHand2;
		sprintf( sc_Name, "%s", EC_PrinterAdjust_TrayName5 );
		break;
	case 6:									//6:LCT    (LCT).
		e_Tray = TYPE_PrinterAdjustMainRegistLCT2;
		sprintf( sc_Name, "%s", EC_PrinterAdjust_TrayName6 );
		break;
#endif // DenebMLK.
#else	// defined(DEF_A4_Cus).
	case 4:									//5:Manual (手差し).
		if( (TYPE_ModelTypeIII == THRG_GetModelType()) &&
			(False == THRF_isSparrow2AIO()) ){
			e_Tray = TYPE_PrinterAdjustMainRegist4th2;
			sprintf( sc_Name, "%s", EC_PrinterAdjust_TrayName4 );
		}else{
			e_Tray = TYPE_PrinterAdjustMainRegistHand2;
			sprintf( sc_Name, "%s", EC_PrinterAdjust_TrayName5 );
		}
		break;
	case 5:
		if( (TYPE_ModelTypeIII == THRG_GetModelType() ) &&
			(False == THRF_isSparrow2AIO()) ){
			e_Tray = TYPE_PrinterAdjustMainRegistHand2;
			sprintf( sc_Name, "%s", EC_PrinterAdjust_TrayName5 );
			break;
		}else{
			// 5がないため、breakせずdefaultに流す.
		}
#endif	//<- !defined(DEF_A4_Cus) 2016.01.28 End.
	default:								//トレイ種類エラー.
		printf( " [Format]GetPrinterAdjust2ndMainRegist Tray\r\n" );
		printf( " Tray ... Tray Kind\r\n" );
		printf( "          1:%s\r\n", EC_PrinterAdjust_TrayName1 );
		printf( "          2:%s\r\n", EC_PrinterAdjust_TrayName2 );
		printf( "          3:%s\r\n", EC_PrinterAdjust_TrayName3 );
#if !defined(DEF_A4_Cus)	//-> 2016.01.28 Start.
		printf( "          4:%s\r\n", EC_PrinterAdjust_TrayName4 );
		printf( "          5:%s\r\n", EC_PrinterAdjust_TrayName5 );
		printf( "          6:%s\r\n", EC_PrinterAdjust_TrayName6 );
#else	// defined(DEF_A4_Cus).
		if( (TYPE_ModelTypeIII == THRG_GetModelType() ) &&
			(False == THRF_isSparrow2AIO()) )
		{
			printf( "          4:%s\r\n", EC_PrinterAdjust_TrayName4 );
			printf( "          5:%s\r\n", EC_PrinterAdjust_TrayName5 );
		}
		else
		{
			printf( "          4:%s\r\n", EC_PrinterAdjust_TrayName5 );
		}
#endif	//<- !defined(DEF_A4_Cus) End.
		//エラー終了.
		return;
		break;
	}
	//共有データから値を取得.
	ss_Value = pc_PrinterServiceSetting->getPrinterAdjust( e_Tray );
	printf( " PrinterAdjust2ndMainRegist %s = %d\r\n", sc_Name, ss_Value );
}
/*================================================================================================
［機能］	ターミナルコマンド（両面側端プリント位置調整値設定） 
--------------------------------------------------------------------------------------------------
［引数］	short ss_Tray				：トレイ種類 
										：　1:1st     (1段目) 
										：　2:2nd     (2段目) 
										：　3:3rd     (3段目) 
										：　4:4th     (4段目) 
										：　5:Manual  (手差し)          DenebMLKの場合、5th (5段目).
										：　6:LCT     (LCT) ⇒ Amurなし DenebMLKの場合、Manual  (手差し).
			short ss_Value				：設定値 
										：  -30～30 
										：  設定する値を10倍値で指定する。さらにステップは2の倍数 
										：  になるように設定する。 
										：  ※実際の範囲は、-3.0mm～3.0mm 
										：  ※実際のステップは、0.2mm単位 
										：  ※11(1.1mm)や-5(-0.5mm)はエラー 
［戻値］	無し 
［詳細］ 
［ｳｫｰｸｽﾙｰ］	未実施 
--------------------------------------------------------------------------------------------------
 [履歴] 
 '06/08/04 N.UDA(REVSONIC) 新規作成 
 '08/09/03 furuno CQ309029 Amur対応.AmurにはLCTがつかないのではずす.
 '18/07/17 Kaihotsu 【DenebMLK 生産コマンド対応（コマンド追加対応）:機器構成変更_No_13-2 チョイス調整機能変更対応】.
 '23/06/29 Sky		OP_BTS-43745 IT6_2.3 Sparrow 給紙トレイ4段対応.
================================================================================================*/
void SetPrinterAdjust2ndMainRegist( short ss_Tray, short ss_Value )
{
	TYPE_PrinterAdjust 	e_Tray = TYPE_PrinterAdjustMainRegist1st2;				//トレイ種類.
	short ss_OldValue;						//変更前の設定値.
	bool bl_error = False;					//エラー検出フラグ(True:検出,False:未検出).
	//パラメータチェック.
	switch ( ss_Tray ) {
	case 1:									//1:1st    (1段目).
		e_Tray = TYPE_PrinterAdjustMainRegist1st2;
		break;
	case 2:									//2:2nd    (2段目).
		e_Tray = TYPE_PrinterAdjustMainRegist2nd2;
		break;
	case 3:									//3:3rd    (3段目).
		e_Tray = TYPE_PrinterAdjustMainRegist3rd2;
		break;
#if !defined(DEF_A4_Cus)	//-> 2016.01.28 Start.
	case 4:									//4:4th    (4段目).
		e_Tray = TYPE_PrinterAdjustMainRegist4th2;
		break;
#if defined(DEF_DenebMLKCus)	// DenebMLK.
	case 5:									//5:5th    (5段目).
		e_Tray = TYPE_PrinterAdjustMainRegistLCT2;
		break;
	case 6:									//6:Manual (手差し).
		e_Tray = TYPE_PrinterAdjustMainRegistHand2;
		break;
#else
	case 5:									//5:Manual (手差し).
		e_Tray = TYPE_PrinterAdjustMainRegistHand2;
		break;
	case 6:									//6:LCT    (LCT).
		e_Tray = TYPE_PrinterAdjustMainRegistLCT2;
		break;
#endif // DenebMLK.
#else	// defined(DEF_A4_Cus).
	case 4:									//4:Manual (手差し).
		if( (TYPE_ModelTypeIII == THRG_GetModelType() ) &&
			(False == THRF_isSparrow2AIO()) )
		{
			e_Tray = TYPE_PrinterAdjustMainRegist4th2;
		}
		else
		{
			e_Tray = TYPE_PrinterAdjustMainRegistHand2;
		}
		break;
	case 5:
		if( (TYPE_ModelTypeIII == THRG_GetModelType() ) &&
			(False == THRF_isSparrow2AIO()) )
		{
			e_Tray = TYPE_PrinterAdjustMainRegistHand2;
			break;
		}
		else
		{
			// 5がないため、breakせずdefaultに流す.
		}
#endif	//<- !defined(DEF_A4_Cus) 2016.01.28 End.
	default:								//トレイ種類エラー.
		//エラー検出!!.
		bl_error = True;
		break;
	}
	//エラー未検出?.
	if ( bl_error == False ) {
		//設定値の範囲チェックエラー?.
//		if ( ss_Value > EC_PrinterAdjust_RegistValue_Max || ss_Value < EC_PrinterAdjust_RegistValue_Min ) {
		if ( ss_Value > EC_PrinterAdjust_MainRegistValue_Max || ss_Value < EC_PrinterAdjust_MainRegistValue_Min ) {
			//エラー検出!!.
			bl_error = True;
		}
		//設定値のSTEPチェックエラー?(余りがある?).
		else if ( ss_Value % EC_PrinterAdjust_RegistValue_Step ) {
			//エラー検出!!.
			bl_error = True;
		}
	}
	//エラー検出した?.
	if ( bl_error == True ) {
		printf( " [Format]SetPrinterAdjust2ndMainRegist Tray, Value\r\n" );
		printf( "  Tray ... Tray Kind\r\n" );
		printf( "           1:%s\r\n", EC_PrinterAdjust_TrayName1 );
		printf( "           2:%s\r\n", EC_PrinterAdjust_TrayName2 );
		printf( "           3:%s\r\n", EC_PrinterAdjust_TrayName3 );
#if !defined(DEF_A4_Cus)	//-> 2016.01.28 Start.
		printf( "           4:%s\r\n", EC_PrinterAdjust_TrayName4 );
		printf( "           5:%s\r\n", EC_PrinterAdjust_TrayName5 );
		printf( "           6:%s\r\n", EC_PrinterAdjust_TrayName6 );
#else	// defined(DEF_A4_Cus).
		if( (TYPE_ModelTypeIII == THRG_GetModelType() ) &&
			(False == THRF_isSparrow2AIO()) )
		{
			printf( "           4:%s\r\n", EC_PrinterAdjust_TrayName4 );
			printf( "           5:%s\r\n", EC_PrinterAdjust_TrayName5 );

		}
		else
		{
			printf( "           4:%s\r\n", EC_PrinterAdjust_TrayName5 );
		}
#endif	// !defined(DEF_A4_Cus) 2016.01.28 End.
		printf( " Value ... Setting Value\r\n" );
//		printf( "           From -100 up to 100.\r\n" );
		printf( "           From %d up to %d.\r\n", EC_PrinterAdjust_MainRegistValue_Min, EC_PrinterAdjust_MainRegistValue_Max );
		printf( "           The internal scale (10 times value) with please appoint.\r\n" );
		printf( "           Step please makes multiple of 2.\r\n" );
		//エラー終了.
		return;
	}
	//共有データから現在値を取得.
	ss_OldValue = pc_PrinterServiceSetting->getPrinterAdjust( e_Tray );
	printf( " Last Value = %d\r\n", ss_OldValue );
	//共有データに値を設定.
	pc_PrinterServiceSetting->setPrinterAdjust( e_Tray, ss_Value );
	printf( "  New Value = %d\r\n", ss_Value );
}
/*================================================================================================
［機能］	ターミナルコマンド（日付時刻設定/取得）：ラインモード用 
--------------------------------------------------------------------------------------------------
［引数］	年、月、日、時、分、秒 
［戻値］	無し 
［詳細］	SYSTimerとRDSTimer両方の時刻を設定する 
			RDSを変更するため、ラインモード限定処理とする 
［ｳｫｰｸｽﾙｰ］	未実施 
--------------------------------------------------------------------------------------------------
 [履歴]
 '06/08/08 R.Suzuki 新規作成 ( 65819 出荷前検査装置 日付・時刻設定/取得コマンドの追加 ) 
 '06/11/15 Inisi No72443対応 
 '09/09/14 S.Ohba CQ320622対応 USR時刻補正値を0クリアするように修正.
 '13/05/20 R.Ohashi 制御構成変更_No.1203_日時設定の設定方法改善.
                    AdjustFactoryTimeForPRC, RollBackFactoryTime, CheckFactoryModeを新規作成.
 '13/07/01 fukuhara 制御構成変更_No.1203_日時設定の設定方法改善 DipSW25対象外対応.
================================================================================================*/
// 閏年の計算関数 パネルからそのまま移植.
// 月ごとの日数						 1	 2	 3	 4	 5	 6	 7	 8	 9	10	11	12.
const char DaysOfMonth[] = {	0,	31,	28,	31,	30,	31,	30,	31,	31,	30,	31,	30,	31 };
char FactorydaysOfMonth(
	uchar	uc_month,		// 月（１～１２）.
	short	ss_year			// 年.
	)
{
	char sc_days = DaysOfMonth[ uc_month ];	// 日数を設定する.

	// うるう年の計算.
	register Bool bl_year = False;
	if ( ss_year % 4 == 0 )
	{
		// 基本 ：4で割り切れる年はうるう年.
		bl_year = True;
		if ( ss_year % 100 == 0 ) {
			// 例外1：100で割り切れる年はうるう年でない.
			bl_year = False;
			if ( ss_year % 400 == 0 ) {
				// 例外2：400で割り切れる年はうるう年.
				bl_year = True;
			}
		}
	}
	if ( bl_year == True && uc_month == 2 ) {
		// うるう年の2月の場合.
		sc_days = 29;		// 日数を29日にする.
	}
	return sc_days;			// 日数を返す.
}

#define THRD_AdjustFactoryTimeToGMT    -8	// 中国工場向けグリニッジ標準時算出用調整時間固定値(=(-1)*タイムゾーン調整時間).
#define THRD_AdjustFactoryTimeToLocal   8	// 中国工場向けタイムゾーン調整時間固定値.
#define THRD_Min_TimeZone_H			  -12	// 指定タイムゾーン(時)最小値.
#define THRD_Max_TimeZone_H			   13	// 指定タイムゾーン(時)最大値.
#define THRD_Min_TimeZone_M			  	0	// 指定タイムゾーン(分)最小値.
#define THRD_Max_TimeZone_M			   30	// 指定タイムゾーン(分)最大値.

Bool AdjustFactoryTimeForPRC( int32_t *pi_year, int32_t *pi_mon, int32_t *pi_day, int32_t *pi_hour, int32_t *pi_min, int32_t *pi_sec, int32_t i_AdjH, int32_t i_AdjM=0 )
{
	time_t timer;
	struct tm st_time;
	int32_t i_Ret = 0;

	if(   pi_year == NULL || pi_mon == NULL || pi_day == NULL
	   || pi_hour == NULL || pi_min == NULL || pi_sec == NULL ){
		return False;
	}

	// CQ386605 タイムゾーン指定の日付・時刻設定コマンド追加対応.
	if( i_AdjH < 0 ){
		// タイムゾーン(時)がマイナス値の場合は.
		// タイムゾーン(分)もマイナス値に変更する必要がある.
		i_AdjM = -i_AdjM;
	}

	memset(&st_time, 0x00, sizeof(struct tm));

	st_time.tm_year = *pi_year - 1900;
	st_time.tm_mon  = *pi_mon  - 1;
	st_time.tm_mday = *pi_day;
	st_time.tm_hour = *pi_hour + i_AdjH;
	st_time.tm_min  = *pi_min  + i_AdjM;	// CQ386605 タイムゾーン指定の日付・時刻設定コマンド追加対応.
	st_time.tm_sec  = *pi_sec;
	st_time.tm_isdst= -1;      // 夏時間無効.

	// 日付・時刻の再計算.
	timer = mktime(&st_time);
	if(-1 == timer){
		return False; 
	}

	// 日付・時刻の再設定.
	i_Ret = SYSF_localtime_r(&timer, &st_time);
	if( -1 == i_Ret ){	// SYSF_localtime_rはエラー時に-1を返す.
		return False;
	}

	*pi_year = st_time.tm_year + 1900;
	*pi_mon  = st_time.tm_mon  + 1;
	*pi_day  = st_time.tm_mday;
	*pi_hour = st_time.tm_hour;
	*pi_min  = st_time.tm_min;
	*pi_sec  = st_time.tm_sec;

	return True;

}

static void RollBackFactoryTime(CT_Time ct_Time)
{
	CB_RTC cb_RTC = CB_RTC::newInstance();

// 2019/08/30【EagleH】OP_BTS-12474 RTC時刻を設定する箇所をcb_RTC.writeからSYSF_setRealTimeClockへ変更 ->.
	SYSS_AbsTime AbsTime;
	AbsTime.year  = ct_Time.year;
	AbsTime.month = ct_Time.month;
	AbsTime.day   = ct_Time.day;
	AbsTime.hour  = ct_Time.hour;
	AbsTime.min   = ct_Time.minute;
	AbsTime.sec   = ct_Time.second;
	SYSF_setRealTimeClock( &AbsTime );	// RTCに書き込み.
// 2019/08/30【EagleH】OP_BTS-12474 RTC時刻を設定する箇所をcb_RTC.writeからSYSF_setRealTimeClockへ変更 <-.

	cb_RTC.read( ct_Time );		// RTCを読み込み.
	TYPC_Time c_Time;
	c_Time.ss_Year   = ct_Time.year;
	c_Time.sc_Month  = ct_Time.month;
	c_Time.sc_Day    = ct_Time.day;
	c_Time.sc_Hour   = ct_Time.hour;
	c_Time.sc_Minute = ct_Time.minute;
	c_Time.sc_Second = ct_Time.second;
	APIC_SystemSetting::newInstance()->setSystemTime( c_Time ); // SYSTime設定.

	cb_RTC.read( ct_Time );		// RTCを読み込み.
// 2019/08/30【EagleH】OP_BTS-12474 RTC時刻を設定する箇所をcb_RTC.writeからSYSF_setRealTimeClockへ変更 ->.
//	SYSS_AbsTime AbsTime;
// 2019/08/30【EagleH】OP_BTS-12474 RTC時刻を設定する箇所をcb_RTC.writeからSYSF_setRealTimeClockへ変更 <-.
	AbsTime.year  = ct_Time.year;
	AbsTime.month = ct_Time.month;
	AbsTime.day   = ct_Time.day;
	AbsTime.hour  = ct_Time.hour;
	AbsTime.min   = ct_Time.minute;
	AbsTime.sec   = ct_Time.second;
	SYSC_RDSTimer::newInstance()->setCurrentTime( &AbsTime ); // RDS設定.
}

#define THRD_LimitFactoryTime2038    ((3 * 60 * 60) + (14 * 60) + 7)	// 2038年対策用 2038/1/19 3:14:7.

static Bool CheckFactoryTime( int32_t i_year, int32_t i_mon, int32_t i_day, int32_t i_hour, int32_t i_min, int32_t i_sec )
{
	Bool bl_Ans = True;

	if(	i_year < 2000 || 2038 < i_year
	||	i_mon  < 1    || 12   < i_mon
	||	i_day  < 1    || 31   < i_day
	||	i_hour < 0    || 23   < i_hour
	||	i_min  < 0    || 59   < i_min
	||	i_sec  < 0    || 59   < i_sec
	){
		bl_Ans = False;
	}else{
		// 2038年問題対策 2038/1/19 3:14:7 以降はエラーとする.
		if( 2038 == i_year ){
			if( 1 < i_mon ){	
				bl_Ans = False;
			}else{
				if( 19 < i_day ){
					bl_Ans = False;
				}else if( 19 == i_day ){
					int32_t i_time = (i_hour * 60 * 60) + (i_min * 60) + i_sec;
					if( THRD_LimitFactoryTime2038 < i_time ){
						bl_Ans = False;
					}
				}
			}
		}
	}

	return bl_Ans;
}

void SetFactoryTime( int32_t year, int32_t mon, int32_t day, int32_t hour, int32_t min, int32_t sec )
{
	if( checkLineMode() == False ){	// ラインモードであるかの確認.
		return;
	}
	Bool bl_Ans = True;
	int32_t  i_year, i_mon, i_day, i_hour, i_min, i_sec = 0;

	CT_Time ct_BackupTime;			// ロールバック用.

	// 指定日時が調整時にエラーとならないか事前にチェックしておく.
	// 調整前の日時チェック.
	bl_Ans = CheckFactoryTime(year, mon, day, hour, min, sec);
	if( True == bl_Ans ){
		// 確認用変数に格納.
		i_year = year;
		i_mon  = mon;
		i_day  = day;
		i_hour = hour;
		i_min  = min;
		i_sec  = sec;

		bl_Ans = AdjustFactoryTimeForPRC(&i_year, &i_mon, &i_day, &i_hour, &i_min, &i_sec, THRD_AdjustFactoryTimeToGMT);
		if( True == bl_Ans ){
			// 調整後の日時チェック.
			bl_Ans = CheckFactoryTime(i_year, i_mon, i_day, i_hour, i_min, i_sec);
			if( True == bl_Ans ){
				// 時間補正時にエラーが起きた際、元の時刻に戻すため、RTCをバックアップしておく.
				CB_RTC::newInstance().read( ct_BackupTime );		// RTCを読み込み、バックアップ.
			}
		}
	}

	if( True == bl_Ans ){
		if( False == CheckFactoryTime(year, mon, day, hour, min, sec) ){
			bl_Ans = False; // 引数値不正.
		}else{
			char sc_Days = FactorydaysOfMonth( (uchar)mon, (short)year );
			if( day > sc_Days ){ // 日の設定が月の最終日より大きい場合はエラー.
				bl_Ans = False;
			}else{
				// 2006/11/15 No72443対応 Inisi↓.
				CB_RTC cb_RTC = CB_RTC::newInstance();
				CT_Time ct_Time;

// 2019/08/30【EagleH】OP_BTS-12474 RTC時刻を設定する箇所をcb_RTC.writeからSYSF_setRealTimeClockへ変更 ->.
				SYSS_AbsTime AbsTime;
				AbsTime.year  = (int)year;
				AbsTime.month = (int)mon;
				AbsTime.day   = (int)day;
				AbsTime.hour  = (int)hour;
				AbsTime.min   = (int)min;
				AbsTime.sec   = (int)sec;
				SYSF_setRealTimeClock( &AbsTime );	// RTCに書き込み.
// 2019/08/30【EagleH】OP_BTS-12474 RTC時刻を設定する箇所をcb_RTC.writeからSYSF_setRealTimeClockへ変更 <-.

				cb_RTC.read( ct_Time );		// RTCを読み込み.

				TYPC_Time c_Time;
				c_Time.ss_Year   = ct_Time.year;
				c_Time.sc_Month  = ct_Time.month;
				c_Time.sc_Day    = ct_Time.day;
				c_Time.sc_Hour   = ct_Time.hour;
				c_Time.sc_Minute = ct_Time.minute;
				c_Time.sc_Second = ct_Time.second;
#if 0 //ソースが古い為、上位との整合が取れておらずエラーとなる.
				pc_SystemSetting->setSystemTime( c_Time ); // SYSTime設定.

				pc_SystemSetting->setUserTime( 0 );
				SYSC_Timer::setAdditionalTime( 0 ); 
#else
				APIC_SystemSetting::newInstance()->setSystemTime( c_Time ); // SYSTime設定.
#endif
				cb_RTC.read( ct_Time );		// RTCを読み込み.
// 2019/08/30【EagleH】OP_BTS-12474 RTC時刻を設定する箇所をcb_RTC.writeからSYSF_setRealTimeClockへ変更 ->.
//				SYSS_AbsTime AbsTime;
// 2019/08/30【EagleH】OP_BTS-12474 RTC時刻を設定する箇所をcb_RTC.writeからSYSF_setRealTimeClockへ変更 <-.
				AbsTime.year = ct_Time.year;
				AbsTime.month = ct_Time.month;
				AbsTime.day = ct_Time.day;
				AbsTime.hour = ct_Time.hour;
				AbsTime.min = ct_Time.minute;
				AbsTime.sec = ct_Time.second;
				SYSC_RDSTimer::newInstance()->setCurrentTime( &AbsTime ); // RDS設定.
			}
		}

		// 制御構成変更_No.1203　日時設定の設定方法改善.
		// PCから通知された時刻に対して、8時間前の時刻をMFP(RTC)に設定することで、.
		// グリニッジ標準時を設定する（中国で入力することが前提）.
		// RTC以外の時刻設定には調整時刻を設定しない.
		if ( True == bl_Ans ){
			CB_RTC cb_RTC = CB_RTC::newInstance();
			CT_Time ct_Time;
			cb_RTC.read( ct_Time );		// RTCを読み込み.

			i_year = (int32_t)ct_Time.year;
			i_mon  = (int32_t)ct_Time.month;
			i_day  = (int32_t)ct_Time.day;
			i_hour = (int32_t)ct_Time.hour;
			i_min  = (int32_t)ct_Time.minute;
			i_sec  = (int32_t)ct_Time.second;

			bl_Ans = AdjustFactoryTimeForPRC(&i_year, &i_mon, &i_day, &i_hour, &i_min, &i_sec, THRD_AdjustFactoryTimeToGMT);

			if( True == bl_Ans ){
// 2019/08/30【EagleH】OP_BTS-12474 RTC時刻を設定する箇所をcb_RTC.writeからSYSF_setRealTimeClockへ変更 ->.
				SYSS_AbsTime AbsTime2;
				AbsTime2.year  = (int)i_year;
				AbsTime2.month = (int)i_mon;
				AbsTime2.day   = (int)i_day;
				AbsTime2.hour  = (int)i_hour;
				AbsTime2.min   = (int)i_min;
				AbsTime2.sec   = (int)i_sec;
				SYSF_setRealTimeClock( &AbsTime2 );	// RTCに書き込み.
// 2019/08/30【EagleH】OP_BTS-12474 RTC時刻を設定する箇所をcb_RTC.writeからSYSF_setRealTimeClockへ変更 <-.
				
				// タイムゾーンの設定.
				// 全ての時刻設定に設定する(+8H).
				APIC_SystemInfo::newInstance()->setTimeZoneLineMode( TYPC_HourMinute( THRD_AdjustFactoryTimeToLocal, 0 ) );	// RTC設定.
// 2016/02/01 CQ521285 監査ログ HCD-PP対応 Sky大嶋 MOD Start.
				APIC_MailSendSetting::newInstance()->setTimeZone( TYPC_HourMinute( THRD_AdjustFactoryTimeToLocal, 0 ), JLGE_InspectLogOperateIDL_System, JLGE_InspectLogIfSystem, False );	// SYSTime設定.
// 2016/02/01 CQ521285 監査ログ HCD-PP対応 Sky大嶋 MOD End.
				APIC_RdsSetting::newInstance()->setRdsTimeZone(THRD_AdjustFactoryTimeToLocal);								// RDS設定.
			}else{	// 調整に失敗した場合は時刻をロールバック.
				RollBackFactoryTime(ct_BackupTime);
			}
		}
	}

	GetFactoryTime();
	if( False == bl_Ans ){
		printf( "SetFactoryTime NG\n" );
	}
}
void GetFactoryTime()
{
	Bool bl_Ans = True;

	// 2006/11/15 No72443対応 Inisi↓.
	CB_RTC cb_RTC = CB_RTC::newInstance();
	CT_Time c_Time;

	cb_RTC.read( c_Time );	// RCTを読み込み.

	// 制御構成変更_No.1203　日時設定の設定方法改善.
	// PCに通知する時刻として、RTCの時刻をタイムゾーンで調整した時間を通知する.

	// RTCのタイムゾーンを取得.
	TYPC_HourMinute c_TimeZone;
	c_TimeZone = APIC_SystemInfo::newInstance()->getTimeZoneLineMode();

	int32_t i_year = (int32_t)c_Time.year;
	int32_t i_mon  = (int32_t)c_Time.month;
	int32_t i_day  = (int32_t)c_Time.day;
	int32_t i_hour = (int32_t)c_Time.hour;
	int32_t i_min  = (int32_t)c_Time.minute;
	int32_t i_sec  = (int32_t)c_Time.second;

	bl_Ans = AdjustFactoryTimeForPRC(&i_year, &i_mon, &i_day, &i_hour, &i_min, &i_sec, c_TimeZone.sc_Hour, c_TimeZone.sc_Minute);

	if( True == bl_Ans ){
		c_Time.year   = (short)i_year;
		c_Time.month  = (char)i_mon;
		c_Time.day    = (char)i_day;
		c_Time.hour   = (char)i_hour;
		c_Time.minute = (char)i_min;
		c_Time.second = (char)i_sec;
	}

	printf( "Year %d\n", c_Time.year );
	printf( "Month %d\n", c_Time.month );
	printf( "Day %d\n", c_Time.day );
	printf( "Hour %d\n", c_Time.hour );
	printf( "Minute %d\n", c_Time.minute );
	printf( "Second %d\n", c_Time.second );
	// 2006/11/15 No72443対応 Inisi↑.
	
	if( False == bl_Ans ){
		printf( "GetFactoryTime NG\n" );
	}

}

/*================================================================================================
［機能］	ターミナルコマンド（日付・時刻_タイムゾーン設定）：ラインモード用 
--------------------------------------------------------------------------------------------------
［引数］	年、月、日、時、分、秒、タイムゾーン(時)、タイムゾーン(分)
［戻値］	無し 
［詳細］	タイムゾーンを指定して日時設定可能なコマンド
			※SetFactoryTime()は+8H固定の為、
			　これをベースにタイムゾーンの調整処理、ターミナル出力を変更
［ｳｫｰｸｽﾙｰ］	未実施 
--------------------------------------------------------------------------------------------------
 [履歴]
 '14/02/24 S.Yamauchi CQ386605 タイムゾーン指定の日付・時刻設定コマンドの追加対応.
 '15/05/13 Miyazaki   CQ512364 タイムゾーン指定の(時)がマイナス値の場合は(分)の値もマイナスにする対応.
================================================================================================*/
void SetFactoryTime2( int32_t year, int32_t mon, int32_t day, int32_t hour, int32_t min, int32_t sec , int32_t timeZoneH, int32_t timeZoneM)
{
	if( False == checkLineMode() ){	// ラインモードであるかの確認.
		return;
	}
	Bool bl_Ans = False;
	int32_t  i_year, i_mon, i_day, i_hour, i_min, i_sec = 0;

	CT_Time ct_BackupTime;			// ロールバック用.

	// 入力日時、タイムゾーンが有効値かチェックする.
	if( (False != CheckFactoryTime(year, mon, day, hour, min, sec))	// 2000～2038の範囲内かチェック.
	 && (False != CheckTimeZone( timeZoneH, timeZoneM ))			// タイムゾーンが時：-12～+13、分：0または30 かチェック.
	 && (day <= FactorydaysOfMonth( (uchar)mon, (short)year )) ){	// 入力された月の最終日チェック.
		// 指定日時が調整時にエラーとならないか事前にチェックしておく.
		// 確認用変数に格納.
		i_year = year;
		i_mon  = mon;
		i_day  = day;
		i_hour = hour;
		i_min  = min;
		i_sec  = sec;

		// 調整時間＝グリニッジ標準時を算出（指定日時±指定タイムゾーン(時の符号反転)）.
		bl_Ans = AdjustFactoryTimeForPRC(&i_year, &i_mon, &i_day, &i_hour, &i_min, &i_sec, -timeZoneH, timeZoneM);
		if( False != bl_Ans ){
			// 調整後の日時チェック.
			bl_Ans = CheckFactoryTime(i_year, i_mon, i_day, i_hour, i_min, i_sec);
			if( False != bl_Ans ){
				// 最終的な時間補正時にエラーが起きた際、元の時刻に戻すため、RTCをバックアップしておく.
				CB_RTC::newInstance().read( ct_BackupTime );		// RTCを読み出し、ct_BackupTimeにバックアップ.
			}
		}
	}

	// 事前チェックで問題無ければ、.
	// SYSTime、RDSタイマーの設定を、工場入力日時で、.
	// RTC の設定をグリニッジ標準時で行う.
	if( False != bl_Ans ){
		// SYSTime、RDSタイマーの設定を行う為に、一旦、工場入力日時でRTCを設定する.
		CB_RTC cb_RTC = CB_RTC::newInstance();
		CT_Time ct_Time;
// 2019/08/30【EagleH】OP_BTS-12474 RTC時刻を設定する箇所をcb_RTC.writeからSYSF_setRealTimeClockへ変更 ->.
		SYSS_AbsTime AbsTime;
		AbsTime.year  = (int)year;
		AbsTime.month = (int)mon;
		AbsTime.day   = (int)day;
		AbsTime.hour  = (int)hour;
		AbsTime.min   = (int)min;
		AbsTime.sec   = (int)sec;
		SYSF_setRealTimeClock( &AbsTime );	// RTCに書き込み.
// 2019/08/30【EagleH】OP_BTS-12474 RTC時刻を設定する箇所をcb_RTC.writeからSYSF_setRealTimeClockへ変更 <-.

		cb_RTC.read( ct_Time );		// RTCを読み出す.
		TYPC_Time c_Time;
		c_Time.ss_Year   = ct_Time.year;
		c_Time.sc_Month  = ct_Time.month;
		c_Time.sc_Day    = ct_Time.day;
		c_Time.sc_Hour   = ct_Time.hour;
		c_Time.sc_Minute = ct_Time.minute;
		c_Time.sc_Second = ct_Time.second;
		APIC_SystemSetting::newInstance()->setSystemTime( c_Time ); // SYSTimeを設定する(入力時間).

		cb_RTC.read( ct_Time );		// RTCを読み出す.
// 2019/08/30【EagleH】OP_BTS-12474 RTC時刻を設定する箇所をcb_RTC.writeからSYSF_setRealTimeClockへ変更 ->.
//		SYSS_AbsTime AbsTime;
// 2019/08/30【EagleH】OP_BTS-12474 RTC時刻を設定する箇所をcb_RTC.writeからSYSF_setRealTimeClockへ変更 <-.
		AbsTime.year = ct_Time.year;
		AbsTime.month = ct_Time.month;
		AbsTime.day = ct_Time.day;
		AbsTime.hour = ct_Time.hour;
		AbsTime.min = ct_Time.minute;
		AbsTime.sec = ct_Time.second;
		SYSC_RDSTimer::newInstance()->setCurrentTime( &AbsTime ); // RDSを設定する(入力時間).

		// 制御構成変更_No.1203　日時設定の設定方法改善.
		// 工場入力日時に対して、タイムゾーンを±した調整日時（＝グリニッジ標準時）をMFP(RTC)に設定する.
		// RTC以外の時刻設定には調整時刻を設定しない.
		cb_RTC.read( ct_Time );		// RTCを読み込み(ここでは工場入力時刻が入っている).

		i_year = (int32_t)ct_Time.year;
		i_mon  = (int32_t)ct_Time.month;
		i_day  = (int32_t)ct_Time.day;
		i_hour = (int32_t)ct_Time.hour;
		i_min  = (int32_t)ct_Time.minute;
		i_sec  = (int32_t)ct_Time.second;

		// グリニッジ標準時を算出（工場入力日時±タイムゾーン(時の符号反転)）.
		bl_Ans = AdjustFactoryTimeForPRC(&i_year, &i_mon, &i_day, &i_hour, &i_min, &i_sec, -timeZoneH, timeZoneM);
		if( False != bl_Ans ){
			// 調整後の日時チェック.
			bl_Ans = CheckFactoryTime(i_year, i_mon, i_day, i_hour, i_min, i_sec);
			if( False != bl_Ans ){
				int32_t i_setTimeZoneM = timeZoneM;
				
				if( timeZoneH < 0 ){
					// タイムゾーン(時)がマイナス値の場合は.
					// タイムゾーン設定APIにセットするタイムゾーン(分)もマイナス値に変更する必要がある.
					i_setTimeZoneM = -i_setTimeZoneM;
				}

// 2019/08/30【EagleH】OP_BTS-12474 RTC時刻を設定する箇所をcb_RTC.writeからSYSF_setRealTimeClockへ変更 ->.
				AbsTime.year  = (int)i_year;
				AbsTime.month = (int)i_mon;
				AbsTime.day   = (int)i_day;
				AbsTime.hour  = (int)i_hour;
				AbsTime.min   = (int)i_min;
				AbsTime.sec   = (int)i_sec;
				SYSF_setRealTimeClock( &AbsTime );	// グリニッジ標準時をRTCに書き込み.
// 2019/08/30【EagleH】OP_BTS-12474 RTC時刻を設定する箇所をcb_RTC.writeからSYSF_setRealTimeClockへ変更 <-.
				
				// タイムゾーンの設定.
				// 全ての時刻設定に設定する(タイムゾーン指定値をそのまま設定).
				APIC_SystemInfo::newInstance()->setTimeZoneLineMode( TYPC_HourMinute( timeZoneH, i_setTimeZoneM ) );	// RTCのタイムゾーン設定設定.
// 2016/02/01 CQ521285 監査ログ HCD-PP対応 Sky大嶋 MOD Start.
				APIC_MailSendSetting::newInstance()->setTimeZone( TYPC_HourMinute( timeZoneH, i_setTimeZoneM ), JLGE_InspectLogOperateIDL_System, JLGE_InspectLogIfSystem, False );	// SYSTimeのタイムゾーン設定設定.
// 2016/02/01 CQ521285 監査ログ HCD-PP対応 Sky大嶋 MOD End.
				APIC_RdsSetting::newInstance()->setRdsTimeZone(timeZoneH);										// RDSのタイムゾーン設定設定.
			}
		}
		if( False == bl_Ans ){
			// 調整に失敗した場合は、バックアップで取っておいたRTC時間から、RTC、SYSTime、RDSを設定し直す.
			// RTCは事前チェック時から進んでいる為、失敗する可能性がある為.
			RollBackFactoryTime(ct_BackupTime);
		}
	}

	// 設定結果をターミナルに出力.
	GetFactoryTime();

	// RTCのタイムゾーンを取得.
	TYPC_HourMinute c_TimeZone;
	c_TimeZone = APIC_SystemInfo::newInstance()->getTimeZoneLineMode();

	printf( "TimeZoneH %+d\n", c_TimeZone.sc_Hour );
	printf( "TimeZoneM %d\n", c_TimeZone.sc_Minute );
	
	if( False == bl_Ans ){
		printf( "SetFactoryTime2 NG\n" );
	}
}

/*================================================================================================
［機能］	タイムゾーン範囲チェック
--------------------------------------------------------------------------------------------------
［引数］	タイムゾーン(時)、タイムゾーン(分)
［戻値］	True:OK 、False:NG
［詳細］	指定されたタイムゾーンの値が 時:-12～13、分：0または30、
			の範囲内であるかをチェックする。
［ｳｫｰｸｽﾙｰ］	未実施 
--------------------------------------------------------------------------------------------------
 [履歴]
 '14/02/24 S.Yamauchi CQ386605 タイムゾーン指定の日付・時刻設定コマンドの追加対応 
================================================================================================*/
static Bool CheckTimeZone( int32_t i_timeZoneH, int32_t i_timeZoneM )
{
	Bool bl_Ans = True;
	
	if( (i_timeZoneH < THRD_Min_TimeZone_H) 
	 || (THRD_Max_TimeZone_H < i_timeZoneH) 
	 || ((THRD_Min_TimeZone_M != i_timeZoneM) && (THRD_Max_TimeZone_M != i_timeZoneM)) ){
		// タイムゾーンの指定値が、「時間」が -12H～+13H、「分」が 0m または 30m 以外の場合はNGとする.
		bl_Ans = False;
	}

	return bl_Ans;
}


#ifndef DEF_DonauBKCus // 10/04/23 hanaoka CQ325164出荷前検査装置対応.
/*================================================================================================
［機能］	ターミナルコマンド（LD相対距離(C)表示） 
--------------------------------------------------------------------------------------------------
［引数］	無し 
［戻値］	無し 
［詳細］ 
［ｳｫｰｸｽﾙｰ］	未実施 
--------------------------------------------------------------------------------------------------
 [履歴] 
 '06/08/18 N.UDA(REVSONIC) 新規作成 
================================================================================================*/
void GetPHNativeDataLDC()
{
	short ss_Value;											//取得した設定値.
	//ラインモードでない?.
	if ( checkLineMode() == False ) {
		//エラー終了.
		return;
	}
	//共有データから値を取得.
	ss_Value = pc_PrinterServiceSetting->getPrinterAdjust( TYPE_PrinterAdjustLDRelativePositionC );
	printf( " LDRelativePosition(C) = %d\r\n", ss_Value );
}
/*================================================================================================
［機能］	ターミナルコマンド（LD相対距離(M)表示） 
--------------------------------------------------------------------------------------------------
［引数］	無し 
［戻値］	無し 
［詳細］ 
［ｳｫｰｸｽﾙｰ］	未実施 
--------------------------------------------------------------------------------------------------
 [履歴] 
 '06/08/18 N.UDA(REVSONIC) 新規作成 
================================================================================================*/
void GetPHNativeDataLDM()
{
	short ss_Value;											//取得した設定値.
	//ラインモードでない?.
	if ( checkLineMode() == False ) {
		//エラー終了.
		return;
	}
	//共有データから値を取得.
	ss_Value = pc_PrinterServiceSetting->getPrinterAdjust( TYPE_PrinterAdjustLDRelativePositionM );
	printf( " LDRelativePosition(M) = %d\r\n", ss_Value );
}
/*================================================================================================
［機能］	ターミナルコマンド（LD相対距離(Y)表示） 
--------------------------------------------------------------------------------------------------
［引数］	無し 
［戻値］	無し 
［詳細］ 
［ｳｫｰｸｽﾙｰ］	未実施 
--------------------------------------------------------------------------------------------------
 [履歴] 
 '06/08/18 N.UDA(REVSONIC) 新規作成 
================================================================================================*/
void GetPHNativeDataLDY()
{
	short ss_Value;											//取得した設定値.
	//ラインモードでない?.
	if ( checkLineMode() == False ) {
		//エラー終了.
		return;
	}
	//共有データから値を取得.
	ss_Value = pc_PrinterServiceSetting->getPrinterAdjust( TYPE_PrinterAdjustLDRelativePositionY );
	printf( " LDRelativePosition(Y) = %d\r\n", ss_Value );
}
#endif //#ifndef DEF_DonauBKCus
/*================================================================================================
［機能］	ターミナルコマンド（LD相対距離(K)表示） 
--------------------------------------------------------------------------------------------------
［引数］	無し 
［戻値］	無し 
［詳細］ 
［ｳｫｰｸｽﾙｰ］	未実施 
--------------------------------------------------------------------------------------------------
 [履歴] 
 '06/08/18 N.UDA(REVSONIC) 新規作成 
================================================================================================*/
void GetPHNativeDataLDK()
{
	short ss_Value;											//取得した設定値.
	//ラインモードでない?.
	if ( checkLineMode() == False ) {
		//エラー終了.
		return;
	}
	//共有データから値を取得.
	ss_Value = pc_PrinterServiceSetting->getPrinterAdjust( TYPE_PrinterAdjustLDRelativePositionK );
	printf( " LDRelativePosition(K) = %d\r\n", ss_Value );
}
#ifndef DEF_DonauBKCus // 10/04/23 hanaoka CQ325164出荷前検査装置対応. // 2011/02/23 ft_kondo 第6回手動マージ. 
/*================================================================================================
［機能］	ターミナルコマンド（スキュー補正モータ初期設定位置(C)表示） 
--------------------------------------------------------------------------------------------------
［引数］	無し 
［戻値］	無し 
［詳細］ 
［ｳｫｰｸｽﾙｰ］	未実施 
--------------------------------------------------------------------------------------------------
 [履歴] 
 '06/08/18 N.UDA(REVSONIC) 新規作成 
================================================================================================*/
void GetPHNativeDataSKEWC()
{
	short ss_Value;											//取得した設定値.
	//ラインモードでない?.
	if ( checkLineMode() == False ) {
		//エラー終了.
		return;
	}
	//共有データから値を取得.
	ss_Value = pc_PrinterServiceSetting->getPrinterAdjust( TYPE_PrinterAdjustPHSkewMotorInitC );
	printf( " PHSkewMotorInit(C) = %d\r\n", ss_Value );
}
/*================================================================================================
［機能］	ターミナルコマンド（スキュー補正モータ初期設定位置(M)表示） 
--------------------------------------------------------------------------------------------------
［引数］	無し 
［戻値］	無し 
［詳細］ 
［ｳｫｰｸｽﾙｰ］	未実施 
--------------------------------------------------------------------------------------------------
 [履歴] 
 '06/08/18 N.UDA(REVSONIC) 新規作成 
================================================================================================*/
void GetPHNativeDataSKEWM()
{
	short ss_Value;											//取得した設定値.
	//ラインモードでない?.
	if ( checkLineMode() == False ) {
		//エラー終了.
		return;
	}
	//共有データから値を取得.
	ss_Value = pc_PrinterServiceSetting->getPrinterAdjust( TYPE_PrinterAdjustPHSkewMotorInitM );
	printf( " PHSkewMotorInit(M) = %d\r\n", ss_Value );
}
/*================================================================================================
［機能］	ターミナルコマンド（スキュー補正モータ初期設定位置(Y)表示） 
--------------------------------------------------------------------------------------------------
［引数］	無し 
［戻値］	無し 
［詳細］ 
［ｳｫｰｸｽﾙｰ］	未実施 
--------------------------------------------------------------------------------------------------
 [履歴] 
 '06/08/18 N.UDA(REVSONIC) 新規作成 
================================================================================================*/
void GetPHNativeDataSKEWY()
{
	short ss_Value;											//取得した設定値.
	//ラインモードでない?.
	if ( checkLineMode() == False ) {
		//エラー終了.
		return;
	}
	//共有データから値を取得.
	ss_Value = pc_PrinterServiceSetting->getPrinterAdjust( TYPE_PrinterAdjustPHSkewMotorInitY );
	printf( " PHSkewMotorInit(Y) = %d\r\n", ss_Value );
}
#endif //#ifndef DEF_DonauBKCus.
/*================================================================================================
［機能］	ターミナルコマンド（Own/Generic設定）：ラインモード用 
--------------------------------------------------------------------------------------------------
［引数］	1: Generic 2: Own 4:Develop 8: Oce.
［戻値］	無し 
［詳細］	SoftDipSW10の設定と同じ 
［ｳｫｰｸｽﾙｰ］	未実施 
--------------------------------------------------------------------------------------------------
 [履歴]
 '06/08/30 R.Suzuki 新規作成 ( 67858 出荷前検査装置 Own/Generic設定、取得コマンドの追加 ) 
 09/07/23 kurachi Donau2nd/Amur CQ:319344対策（取得コマンドに対してのDevelop/Oce仕向け対応）.
 09/09/14 karasaki CQ:320625対策 Own、Generic、Develop、Oceの４種類を設定できるように対応.
 '16/02/15 Y.Samura CQ525120 マーケティング仕向けが""US""かつ、変更後の値が"OWN"の場合,DipSW172を0x00に設定、以外はDipSW172を0x01に設定.
 '18/07/09 Sky山岸 [Eagle]RQ-675_性能に合わせてすべてのMFPにMarketPlaceをダウンロード・インストールする_追加対応.
 18/08/09 Sky忠政　AR1049080 再提起対応.
 18/09/06 Sky忠政　AR1049080 再再提起対応.
 18/12/20 Sky井柳　[Eagle]AR1070720 仕様変更「MarketPlaceの全仕向け有効化対応」対応(Own設定時のDipSW172の値設定条件変更).
 19/04/02 Sky合屋　[IT6_1.1GM] AR1075689 仕様変更マーケティング仕向けが日本の場合、DipSW172を0x01に設定する.
 19/05/16 Sky鳥居　【EagleH】静的解析対応.
 19/09/10 Sky　　　[Eagle]OP_BTS-13279 仕様変更「MarketPlaceの日本仕向けのデフォルト値変更対応」.
 22/03/10 Sky　　　OP_BTS-37764 エッジ補正APP対応.
 23/07/07 Sky      OP_BTS-44076 制御構成変更No52-3 ユーザーセットアップ販社要望.
================================================================================================*/
void SetOwnGeneric( uchar mode )
{
	if( checkLineMode() == False ){	// ラインモードであるかの確認.
		return;
	}
	GetOwnGeneric(); // 現在値表示.

	APIC_SystemInfo* pc_SystemInfo = APIC_SystemInfo::newInstance();
	APIC_SoftDipSw* pc_SoftDipSw = APIC_SoftDipSw::newInstance();
	NVDC_SystemSetting_ColorCorrect* pc_SystemSetting_ColorCorrect = NVDC_SystemSetting_ColorCorrect::newInstance();
	APIC_MachineSetting* pc_MachineSetting = APIC_MachineSetting::newInstance();
	
	if ( pc_SystemInfo == NULL ) {
		return;
	}
	if ( pc_SoftDipSw == NULL ) {
		return;
	}
	if ( pc_SystemSetting_ColorCorrect == NULL ) {
		return;
	}
	if ( pc_MachineSetting == NULL ) {
		return;
	}
	
	switch( mode ){
		case 1:		// Generic.
			pc_SoftDipSw->setSoftDipSwBit( 1, 1, False  );	// Generic.
			pc_SoftDipSw->setSoftDipSwBit( 3, 2, False );	// Genericに書き換える.
			pc_SoftDipSw->setSoftDipSwBit( 4, 1, False );	// Develop.
			pc_SoftDipSw->setSoftDipSwBit( 4, 2, False );	// Oce.
			break;
		case 2:		// Own.
			pc_SoftDipSw->setSoftDipSwBit( 1, 1, False );	// Own.
			pc_SoftDipSw->setSoftDipSwBit( 3, 2, True  );	// Ownに書き換える.
			pc_SoftDipSw->setSoftDipSwBit( 4, 1, False );	// Develop.
			pc_SoftDipSw->setSoftDipSwBit( 4, 2, False );	// Oce.
			pc_SoftDipSw->setSoftDipSwBit( 152, 5, True );	// DipSW176-bit3 OP_BTS-44076 ユーザーセットアップ.
			break;
		case 4:		// Develop.
			pc_SoftDipSw->setSoftDipSwBit( 1, 1, False  );
			pc_SoftDipSw->setSoftDipSwBit( 3, 2, False );	// Genericに書き換える.
			pc_SoftDipSw->setSoftDipSwBit( 4, 1, True );	// Develop.
			pc_SoftDipSw->setSoftDipSwBit( 4, 2, False );	// Oce.
			break;
		case 8:		// Oce.
			pc_SoftDipSw->setSoftDipSwBit( 1, 1, False  );
			pc_SoftDipSw->setSoftDipSwBit( 3, 2, False );	// Genericに書き換える.
			pc_SoftDipSw->setSoftDipSwBit( 4, 1, False );	// Develop.
			pc_SoftDipSw->setSoftDipSwBit( 4, 2, True );	// Oce.
			break;
		default:
			printf( " [Format]SetOwnGeneric OWN or GenericType\n" );
			printf( " 1:Generic 2:OWN 4:Develop 8:Oce\n" );
			return;
	}

	pc_SystemInfo->chkAndSetOwnOrGeneric(); // NVRAMにOwnorGenericを書き込む.
	if( True == pc_SystemInfo->isOwnOrGeneric() ){
		printf( " New  Value = 2\n" );		// Own.
	}else{
		// Generic種別を取得してそれに応じたログを表示.
		TYPE_GenericType e_GenericType = pc_SystemInfo->getGenericKind();

		switch( e_GenericType ){
			case TYPE_GenericTypeNormal:	// Generic.
				printf( " New  Value = 1\n" );
				break;
			case TYPE_GenericTypeDevelop:	// Develop.
				printf( " New  Value = 4\n" );
				break;
			case TYPE_GenericTypeOce:		// Oce.
				printf( " New  Value = 8\n" );
				break;
			default:
				printf( " New  Value = 1\n" );
				break;
		}
	}

	if ( pc_MachineSetting == NULL ) {
		// ソフトDipSW172の値を0x01（無効（デフォルト））にする.
		pc_SoftDipSw->setSoftDipSwBit( 101, 5, True  );		/*bit0*/
		pc_SoftDipSw->setSoftDipSwBit( 101, 6, False  );	/*bit1*/
		pc_SoftDipSw->setSoftDipSwBit( 136, 5, False  );	/*bit2*/
	}
	else if( TYPE_SystemType_JP == pc_MachineSetting->getSystemType() ) {	//日本仕向けの場合.
		// ソフトDipSW172の値を0x01（無効（デフォルト））にする.
		pc_SoftDipSw->setSoftDipSwBit( 101, 5, True  );		/*bit0*/
		pc_SoftDipSw->setSoftDipSwBit( 101, 6, False  );	/*bit1*/
		pc_SoftDipSw->setSoftDipSwBit( 136, 5, False  );	/*bit2*/
	}
	else if( True == pc_SystemInfo->isOwnOrGeneric() ) {	// 変更後の値が"OWN"の場合.
		// ソフトDipSW172の値を0x05（MarketPlaceセットアップ機能使用可）にする.
		pc_SoftDipSw->setSoftDipSwBit( 101, 5, True  );		/*bit0*/
		pc_SoftDipSw->setSoftDipSwBit( 101, 6, False  );	/*bit1*/
		pc_SoftDipSw->setSoftDipSwBit( 136, 5, True  );		/*bit2*/
	}
	else {
		// ソフトDipSW172の値を0x01（無効（デフォルト））にする.
		pc_SoftDipSw->setSoftDipSwBit( 101, 5, True  );		/*bit0*/
		pc_SoftDipSw->setSoftDipSwBit( 101, 6, False  );	/*bit1*/
		pc_SoftDipSw->setSoftDipSwBit( 136, 5, False  );	/*bit2*/
	}
	
	pc_SystemSetting_ColorCorrect->FactorySetup( pc_MachineSetting->getSystemType() );

}
void GetOwnGeneric()
{
	if( checkLineMode() == False ){	// ラインモードであるかの確認.
		return;
	}
	if( True == pc_SystemInfo->isOwnOrGeneric() ){
		printf( " Last Value = 2\n" );
	}else{
		// Generic種別を取得してそれに応じたログを表示.
		TYPE_GenericType e_GenericType = pc_SystemInfo->getGenericKind();

		switch( e_GenericType ){
			case TYPE_GenericTypeNormal:	// Generic（一般）.
				printf( " Last Value = 1\n" );
				break;
			case TYPE_GenericTypeDevelop:	// Generic/Develop.
				printf( " Last Value = 4\n" );
				break;
			case TYPE_GenericTypeOce:		// Generic/Oce.
				printf( " Last Value = 8\n" );
				break;
			default:
				printf( " Last Value = 1\n" );
				break;
		}
	}
}
/*================================================================================================
［機能］	ターミナルコマンド（装置固有識別子の生成）：ラインモード用 
--------------------------------------------------------------------------------------------------
［引数］	なし 
［戻値］	なし 
［詳細］	出荷前検査装置 装置固有識別子の生成コマンド追加 
［ｳｫｰｸｽﾙｰ］	未実施 
--------------------------------------------------------------------------------------------------
 [履歴]
 '07/06/11 鈴村 新規作成 出荷前検査装置 装置固有識別子の生成コマンド追加 
 '07/11/12 鈴村 出荷前検査装置 ライセンス異常状態であれば処理せずに抜ける 
  09/01/20 tadamasa  Warning対応	
  18/06/05 Sky小久保 Eagle 制御構成変更_No.89 生産コマンド対応 角括弧の削除.
================================================================================================*/
Bool bl_CreateRequestCode;		// リクエストコード生成判定用グローバル変数.
uchar uc_BeforeRequestCode[TYPD_MAX_RequestCode+1];	// 生成前のリクエストコード保存用.

void CreateMachineCode()
{
	if( checkLineMode() == False ){		// ラインモードであるかの確認.
		return;
	}
	
	short ss_RequestCodeCount = 0;	// ループ回数カウント用.
	
	bl_CreateRequestCode = False;	// リクエストコード生成判定を初期化.
	
	APIC_Enabler* pc_Enabler = APIC_Enabler::newInstance();
	
	if( True == pc_Enabler->isLicenseManagementAbnormal() ){		// ライセンス管理異常状態か.
		printf( "The Request Code was not able to be made.\n" );	// 失敗の旨を出力.
		return;		// ライセンス管理異常状態であれば終了.
	}
	
	memset( &(uc_BeforeRequestCode), 0, sizeof(uc_BeforeRequestCode) );	// 配列初期化.
	pc_Enabler->getRequestCode( (uchar*)uc_BeforeRequestCode );			// 生成する前に装置固有識別子を取得しておく.
	pc_Enabler->req_startGetRequestCode( ans_startGetRequestCode );		// コールバック関数を呼ぶ.
	
	while( False == bl_CreateRequestCode ){		// リクエストコード生成完了までループ.
#if defined(TORNADO)	// TORNADO環境のみ.
		SYSF_taskDelay( 100 );	// 500ms delayする.
#endif // #if defined(TORNADO).
		ss_RequestCodeCount++;	// ループ回数をカウントアップ.
		if( ss_RequestCodeCount > 10 ){	// 10回以上ループした.
			printf( "Timeout.\n" );		// タイムアウトの旨を出力.
			printf( "The Request Code was not able to be made.\n" );	// 失敗の旨を出力.
			break;
		}
	}
}
void ans_startGetRequestCode( TYPE_EnablerResult e_EnablerResult, uchar* puc_RequestCode )
{
	if( e_EnablerResult == TYPE_EnablerResult_OK ){		// リクエストコード生成成功か.
		if ( uc_BeforeRequestCode[0] == (uchar)NULL ) {		// 生成する前に生成してあったか判定.
			printf( "Request Code New = %s\n", puc_RequestCode );		// 装置固有識別子を出力.
		} else {
			printf( "Request Code Already = %s\n", puc_RequestCode );	// 装置固有識別子を出力.
		}
	} else {
		printf( "The Request Code was not able to be made.\n" );	// 失敗の旨を出力.
	}
	bl_CreateRequestCode = True;	// リクエストコード生成完了と設定.
}

/*================================================================================================
［機能］	ターミナルコマンド（装置固有識別子の取得）：ラインモード用 
--------------------------------------------------------------------------------------------------
［引数］	なし 
［戻値］	なし 
［詳細］	出荷前検査装置 装置固有識別子の取得コマンド追加 
［ｳｫｰｸｽﾙｰ］	未実施 
--------------------------------------------------------------------------------------------------
 [履歴]
 '07/06/11 鈴村 新規作成 出荷前検査装置 装置固有識別子の取得コマンド追加 
 '08/02/06 鈴村 リクエストコード取得用領域を確保する 
 '08/03/25 古野 関数SetSerialNumberでdefine定義名変更を行ったための修正.
 '08/04/08 古野 関数SetSerialNumberでdefine定義名変更を行ったための再修正.
 '18/06/05 Sky小久保 Eagle 御構成変更_No.89 生産コマンド対応 角括弧の削除.
================================================================================================*/
void GetMachineCode()
{
	if( checkLineMode() == False ){		// ラインモードであるかの確認.
		return;
	}	
	uchar uc_RequestCode[TYPD_MAX_RequestCode+1];	// リクエストコード取得用.
	memset( &(uc_RequestCode), 0, sizeof(uc_RequestCode) );	// 配列初期化.
	char ch_EngSerialNostr[DEF_SNLeng_ENG_MAX+1];
	
	APIC_Enabler* pc_Enabler = APIC_Enabler::newInstance();
	pc_Enabler->getRequestCode( (uchar*)uc_RequestCode );	// 装置固有識別子を取得.
	printf( "Request Code = %s\n", uc_RequestCode );		// 装置固有識別子を出力.
	pc_SystemInfo->getSerialNoPrinter (ch_EngSerialNostr );	// ENGシリアルNo取得.
	printf( "Eng SerialNo = %s\n", ch_EngSerialNostr );		// ENGシリアルNoを出力.
}


/*================================================================================================
［機能］	HDD装着/非装着設定 ターミナルコマンド
--------------------------------------------------------------------------------------------------
［引数］	0:非装着 / 1:装着
［戻値］	無し
［詳細］	出荷前検査装置 HDD装着/非装着設定切り替え
［ｳｫｰｸｽﾙｰ］	未実施
--------------------------------------------------------------------------------------------------
 [履歴]
 '08/02/29 古野 新規作成 
 '13/08/07 福原 CQ.382261 スプーラ機能OFF/ON追加. 
================================================================================================*/
void InstallHDD( uchar uc_mode )
{
	switch( uc_mode ){
		case 0:
			pc_HddInfo->setHddExist( False );
			APIC_PrintJobSetting::newInstance()->setSpoolMode(TYPE_SpoolModeOff, NULL);
			printf( "  HDD Not Installed \n" );
			break;
		case 1:
			pc_HddInfo->setHddExist( True );
			APIC_PrintJobSetting::newInstance()->setSpoolMode(TYPE_SpoolModeOn, NULL);
			printf( "  HDD Installed \n" );
			break;
		default:
			printf( "  incorrect value \n" );
			printf( "   setup is not complete! \n" );
			break;
	}
}

/*================================================================================================
［機能］	HDD装着/非装着設定 ターミナルコマンド.
--------------------------------------------------------------------------------------------------.
［引数］	無し.
［戻値］	無し.
［詳細］	.
［ｳｫｰｸｽﾙｰ］	未実施.
--------------------------------------------------------------------------------------------------.
 [履歴]		.
 '15/10/26	大岡	[ZeusSZX0]制御構成変更No.80 Setコマンドに対応するGetコマンド追加対応. 
================================================================================================*/
void GetInstallHDDState( void )
{

	// HDD装着/非装着状態取得.
	if( pc_HddInfo->getHddExist() == True ){
		// HDD装着.
		printf("HDD Installed\n");
	} else {
		// HDD非装着.
		printf("HDD Not Installed\n");
	}
}

/*================================================================================================
［機能］	原稿サイズ検知テーブル切り替え ターミナルコマンド
--------------------------------------------------------------------------------------------------
［引数］	1:テーブル1 / 2:テーブル2
［戻値］	無し
［詳細］	出荷前検査装置
［ｳｫｰｸｽﾙｰ］	未実施
--------------------------------------------------------------------------------------------------
 [履歴]
 '08/02/29 古野 新規作成 
 '18/06/05 Sky小久保 Eagle 御構成変更_No.89 生産コマンド対応 イコールを追加.
================================================================================================*/
void SetOriginalSizeDetection( uchar uc_mode )
{
	switch( uc_mode ){
		case 1:
			pc_ScanSetting->setScannerAdjust( TYPE_ScannerAdjustPapersizeDetectTable, 0 );
			printf( "  Changed Table = 1 \n" );
			break;
		case 2:
			pc_ScanSetting->setScannerAdjust( TYPE_ScannerAdjustPapersizeDetectTable, 1 );
			printf( "  Changed Table = 2 \n" );
			break;
		default:
			printf( "  incorrect value \n" );
			printf( "   setup is not complete! \n" );
			break;
	}
}

/*================================================================================================
［機能］	原稿サイズ検知テーブル状態表示 ターミナルコマンド.
--------------------------------------------------------------------------------------------------.
［引数］	無し.
［戻値］	無し.
［詳細］	.
［ｳｫｰｸｽﾙｰ］	未実施.
--------------------------------------------------------------------------------------------------.
 [履歴]		.
 '15/10/26	大岡	[ZeusSZX0]制御構成変更No.80 Setコマンドに対応するGetコマンド追加対応. 
================================================================================================*/
void GetOriginalSizeDetection( void )
{

	// 原稿サイズ検知テーブル状態取得(戻り値が0の時のみテーブル1。その他はテーブル2).
	if( pc_ScanSetting->getScannerAdjust(TYPE_ScannerAdjustPapersizeDetectTable) == 0 ){
		// テーブル1.
		printf( "Table1\n" );
	} else {
		// テーブル2.
		printf( "Table2\n" );
	}
}

/*================================================================================================
［機能］	デバイス証明書の削除／一覧表示 ターミナルコマンド.
--------------------------------------------------------------------------------------------------
［引数］	s_mode:動作モード　　０：一覧表示　１：削除.
［戻値］	無し.
［詳細］	出荷前検査装置向けコマンド。デバイス証明書の削除／一覧表示。.
［ｳｫｰｸｽﾙｰ］	未実施.
--------------------------------------------------------------------------------------------------
 [履歴]
 '09/04/16 CQ:317367 伊藤史 新規作成. 
 '12/09/03 Gonsho	 静的解析対応 一覧表示のインデックス修正.
 '12/12/05 ADS川口  CQ.377860  MS-LCL_13 getSslCertList()に渡す証明書取得用の領域をグローバル変数から動的確保に変更.
================================================================================================*/
/* getSslCertList関数に使用する証明書Max数 */
//#define PCM_CERT_NUM_MAX						100.
#define	SSLCERTLISTNUM		128

extern "C" {
	void AllClearSslCertificate( int32_t s_mode );
}

void AllClearSslCertificate( int32_t s_mode )
{
	uint32_t		ulCertNum = SSLCERTLISTNUM;		/* 証明書登録数 */

	/* デバイス証明書リスト取得用領域を確保 */
	TYPC_SslCertList* pst_SslCertList = NULL;
	pst_SslCertList = new TYPC_SslCertList[SSLCERTLISTNUM];
	THRL_ThAssert( (pst_SslCertList != NULL), "can't allocate pst_SslCertList!!", 0, 0, 0, 0, CFA14_DetailCode0x0B7A);

	/* インスタンスのロック */
	pc_SslSetting->lock();

	/* デバイス証明書リストの取得 */
	TYPE_SslStatus sslStatus = 0;
	sslStatus = pc_SslSetting->getSslCertList(0,TYPD_CertType_Device,&ulCertNum,&pst_SslCertList[0]);

	/* インスタンスのアンロック */
	pc_SslSetting->unlock();

	if(sslStatus == TYPE_SslStatus_SslSuccess){
		if(ulCertNum > 0){
			if( s_mode == 0 ){
				printf("ulCertNum = %"PRId32"\n", ulCertNum);
			}
			/* ループカウンタ */
			uint32_t		ulCertCnt = 0;
			for(ulCertCnt = 0; ulCertCnt < ulCertNum; ulCertCnt++){
				if( pst_SslCertList[ulCertCnt].ul_CertID == 0 ){
					break;
				}
			}
			for( ; ulCertCnt > 0; ulCertCnt--){

				if( s_mode == 0 ){
					///	\brief	概要：デジタル証明書ID。.
					printf("デジタル証明書ID = %"PRId32"\n",pst_SslCertList[ulCertCnt-1].ul_CertID);

					///	\brief	概要：デバイス証明書の発行者名。64バイト以上ある場合は切り詰める。.
					///	\par	備考：長さが0の文字列は、CSRで証明書を要求していることを示す。.
					///			設定範囲：最大64文字.
					printf("デバイス証明書の発行者名 = %s\n",pst_SslCertList[ulCertCnt-1].uc_Issuer);

					///	\brief	概要：デバイス証明書の所有者名。.
					///	\par	備考：64バイト以上ある場合は切り詰める。.
					///			設定範囲：最大64文字.
					printf("デバイス証明書の所有者名 = %s\n",pst_SslCertList[ulCertCnt-1].uc_Subject);

				} else {
					//証明書削除.
					pc_SslSetting->lock();
					pc_SslSetting->removeSslCertInfo( pst_SslCertList[ulCertCnt-1].ul_CertID );
					pc_SslSetting->unlock();
					if( s_mode == 0 ){
						printf("削除しました！！\n");
					}
				}
			}
		}
	} else {
		printf("sslStatus - Error  %ld", sslStatus);
	}
	
	/* デバイス証明書リスト取得用領域を解放 */
	if( pst_SslCertList != NULL )
	{
		delete [] pst_SslCertList;
		pst_SslCertList = NULL;
	}
	
	return;
}

/*================================================================================================
［機能］	自己証明書インストール ターミナルコマンド
--------------------------------------------------------------------------------------------------
［引数］	ss_Year		自己証明書作成時の日時(年)
			sc_Month	自己証明書作成時の日時(月)
			sc_Day		自己証明書作成時の日時(日)
			sc_Hour		自己証明書作成時の日時(時)
			sc_Minute	自己証明書作成時の日時(分)
			sc_Second	自己証明書作成時の日時(秒)
［戻値］	無し
［詳細］	出荷前検査装置
［ｳｫｰｸｽﾙｰ］	未実施
--------------------------------------------------------------------------------------------------
 [履歴]
 '08/02/29 古野 新規作成 
 '09/04/16 CQ:317367 伊藤史 現在登録されているデバイス証明書を一括削除しする処理を追加.
 '09/05/20 CQ:317738 t-yokoyama Generic/Ownに応じたCN情報を設定する処理を追加.
 '09/05/20 CQ:317213 伊藤史 デフォルトデバイス証明書ID取得処理を追加.
 '13/03/04 CQ:378165  板倉 社名変更対応(KMBT→KM).
 '13/06/07 UV-00958/UV-01257 山内 MACアドレス取得API変更対応.
 '14/07/22 sky O.Suzuki [Poseidon] 制御構成変更_No.234 FIPS140-2変更対応.
 '14/07/22 sky O.Suzuki SHA256-RSA-2048bitを指定するよう修正.
 '14/08/22 CQ:391523 Sky O.Suzuki 出荷時インストール済の自己証明書でアプリ登録ができない対応.
 '15/01/28 sky O.Yamashita	 ターミナルコマンドの証明書有効期限の変更対応.
================================================================================================*/
void CreateSelfCertificate( short ss_Year, char sc_Month, char sc_Day, char sc_Hour, char sc_Minute, char sc_Second )
{

	//パラメータチェック.
	if(	ss_Year   < 2000 || 2039 < ss_Year
  	||	sc_Month  < 1    || 12   < sc_Month
	||	sc_Day    < 1    || 31   < sc_Day
	||	sc_Hour   < 0    || 23   < sc_Hour
	||	sc_Minute < 0    || 59   < sc_Minute
	||	sc_Second < 0    || 59   < sc_Second
	||	sc_Day > FactorydaysOfMonth( sc_Month, ss_Year )	// 日の設定が月の最終日より大きい場合はエラー.
	){	//日付のフォーマットが異なっている場合、パラメータが間違っている場合.
		printf( "CreateSelfCertificate Failed.\n" );
		printf( "\n" );
		printf( "Value  ... Setting Value\n" );
		printf( "    Please input collect ValidStart date.    \n" );
		return;
	}

	//証明書情報設定.
	TYPC_SslCertInfo c_SslCertInfo;

	// 証明書一括削除.
	AllClearSslCertificate( 1 );

	//CQ:317738 t-yokoyama Own/Genericに応じたCN情報を設定する処理を追加.
	//MACアドレス取得 xx:xx:xx:xx:xx:xx.
	char sc_MacAddress[32];
	Bool b_Ret = False; // MACアドレス取得結果.
	memset(sc_MacAddress, 0x00, sizeof(sc_MacAddress));

	APIC_TcpIpSetting* pc_TcpIpSetting = APIC_TcpIpSetting::newInstance();
	// MACアドレス取得関数を有線MACアドレス取得関数に変更.
	b_Ret = pc_TcpIpSetting->getUniqueMacAddress((uchar*)&sc_MacAddress[0]);
	if( False == b_Ret )
	{
		// MACアドレス取得失敗.
		printf( "CreateSelfCertificate Failed.\n" );
		printf( "\n" );
		printf( "    getMacAddress Error\n" );
		return;
	}

	//MACアドレス下6桁.
	char sc_TmpLowMacAddress[MAX_MacAddressForSelfCertificate+1];
	memset( &sc_TmpLowMacAddress[0], 0x00, MAX_MacAddressForSelfCertificate+1 );
	// MACアドレス取り出し.
	for( int32_t i = 0; i < (MAX_MacAddressForSelfCertificate/2); i++ )
	{
		memcpy( &sc_TmpLowMacAddress[i*2], &sc_MacAddress[9+i*3], 2 );		// 下位.
	}

	//O:組織名の設定とCN:コモンネームの編集＆設定.
	if( True == pc_SystemInfo->isOwnOrGeneric() ){	//Own.
		strncpy( c_SslCertInfo.sc_OrganizationUTF8, "KONICA MINOLTA", (sizeof(c_SslCertInfo.sc_OrganizationUTF8)-1) );
		snprintf( c_SslCertInfo.sc_CommonNameUTF8, sizeof(c_SslCertInfo.sc_CommonNameUTF8), "%s%s", "KM", &sc_TmpLowMacAddress[0] );
	}else{	//Generic.
		strncpy( c_SslCertInfo.sc_OrganizationUTF8, "Some Organization", (sizeof(c_SslCertInfo.sc_OrganizationUTF8)-1) );
		snprintf( c_SslCertInfo.sc_CommonNameUTF8, sizeof(c_SslCertInfo.sc_CommonNameUTF8), "%s%s", "GEN", &sc_TmpLowMacAddress[0] );
	}

	// C(国別記号)/S(都道府県名)/L(市区町村名)の設定（固定値）.
	strncpy( c_SslCertInfo.sc_CityNameUTF8, THRD_CertCityNameStr, (sizeof(c_SslCertInfo.sc_CityNameUTF8)-1) );
	strncpy( c_SslCertInfo.sc_StateNameUTF8, THRD_CertStateNameStr, (sizeof(c_SslCertInfo.sc_StateNameUTF8)-1) );
	strncpy( c_SslCertInfo.sc_CountryCode, THRD_CertCountryCodeStr, (sizeof(c_SslCertInfo.sc_CountryCode)-1) );

	//時刻の設定.
	c_SslCertInfo.c_ValidStart.ss_Year   = ss_Year;
	c_SslCertInfo.c_ValidStart.sc_Month  = sc_Month;
	c_SslCertInfo.c_ValidStart.sc_Day    = sc_Day;
	c_SslCertInfo.c_ValidStart.sc_Hour   = sc_Hour;
	c_SslCertInfo.c_ValidStart.sc_Minute = sc_Minute;
	c_SslCertInfo.c_ValidStart.sc_Second = sc_Second;

	//有効期間の設定.
	c_SslCertInfo.us_Days = THRL_RemainingDays(ss_Year, sc_Month, sc_Day, sc_Hour, sc_Minute, sc_Second);

	//証明書作成開始.
	pc_SslSetting->lock();
	pc_SslSetting->setSslSelfCertInfo( c_SslCertInfo, TYPE_SslEncryptionL2, TYPE_SslPrivKeyType_Rsa_2048_SHA_256 );
	pc_SslSetting->unlock();

	//作成完了待ち.
	while ( TYPE_SslCertStatusInstalling == pc_SslSetting->getSslCertSelfStatus() ) {
#if defined(TORNADO)	// TORNADO環境のみ.
		SYSF_taskDelay(1);
#endif // #if defined(TORNADO).
	}

	if( TYPE_SslCertStatusOk != pc_SslSetting->getSslCertSelfStatus() ){
		//作成失敗.
		printf( "CreateSelfCertificate Failed.\n" );
		printf( "\n" );
		printf( "    Internal Error\n" );
		return;
	}

	//09/05/20 CQ:317747 伊藤史 デフォルトデバイス証明書ID取得処理を追加.
	uint32_t			ul_CertId = 0;			//デフォルト証明書ID.

	//SSL設定インスタンスを設定.
	APIC_CertInfoSetting*	pc_CertInfoSetting;
	pc_CertInfoSetting	= APIC_CertInfoSetting::newInstance();

	pc_CertInfoSetting->lock();
	//デフォルトデバイス証明書ID取得.
	ul_CertId = pc_CertInfoSetting->getDefaultCert();
	pc_CertInfoSetting->unlock();

	if( ul_CertId == 0 ){
		//取得失敗.
		printf( "CreateSelfCertificate Failed.\n" );
		printf( "\n" );
		printf( "    getDefaultCert Error\n" );
		return;
	}

	//作成成功,情報取得.
	pc_SslSetting->lock();
	//デフォルト証明書情報取得.
	TYPC_SslCertInfo c_SslCertInfo_Result = pc_SslSetting->getSslCertInfo( ul_CertId );
	pc_SslSetting->unlock();
	printf( "  CN：%s\n", c_SslCertInfo_Result.sc_CommonNameUTF8 );
	printf( "  O：%s\n", c_SslCertInfo_Result.sc_OrganizationUTF8 );
	printf( "  ValidStart：%d/%d/%d %d:%d:%d\n", c_SslCertInfo_Result.c_ValidStart.ss_Year,
												 c_SslCertInfo_Result.c_ValidStart.sc_Month,
												 c_SslCertInfo_Result.c_ValidStart.sc_Day,
												 c_SslCertInfo_Result.c_ValidStart.sc_Hour,
												 c_SslCertInfo_Result.c_ValidStart.sc_Minute,
												 c_SslCertInfo_Result.c_ValidStart.sc_Second );
	printf( "  ValidDay：%d Days\n", c_SslCertInfo_Result.us_Days );
	return;
}

/*================================================================================================
［機能］	自己証明書状態確認 ターミナルコマンド.
--------------------------------------------------------------------------------------------------.
［引数］	無し.
［戻値］	無し.
［詳細］	.
［ｳｫｰｸｽﾙｰ］	未実施.
--------------------------------------------------------------------------------------------------.
 [履歴]		.
 '15/10/26	大岡	[ZeusSZX0]制御構成変更No.80 Setコマンドに対応するGetコマンド追加対応. 
================================================================================================*/
void GetSelfCertificate( void )
{

	// 自己証明書ID取得(ID=0の場合、自己証明書なし).
	if( pc_SslSetting->getSslCertId( TYPE_SslCertUsage_Default ) != 0 ){
		// 自己証明あり.
		printf("SelfCertificate exists\n");
	} else {
		// 自己証明なし.
		printf("SelfCertificate is none\n");
	}
}

/*================================================================================================
［機能］	ターミナルコマンド（ LD光量バランス補正値表示 ) 
--------------------------------------------------------------------------------------------------
［引数］	uc_mode ( 1:C, 2:M, 3:Y, 4:K ). 
［戻値］	なし. 
［詳細］	出荷前検査装置  LD光量バランス補正値. 
［ｳｫｰｸｽﾙｰ］	未実施. 
--------------------------------------------------------------------------------------------------
 [履歴]
 '08/03/21 sunami CQ:301776		新規作成 出荷前検査装置  LD光量バランス補正値. 
 '08/08/21 鈴村   CQ:307931と一緒に直す		printfの()を半角に統一.
================================================================================================*/
void GetLDAdjustData( uchar uc_mode )
{
	if( checkLineMode() == False ){		// ラインモードであるかの確認.
		return;
	}

	short ss_LDAdjustmentLightBalanceValue;

	switch(uc_mode) {
	case 1:
		ss_LDAdjustmentLightBalanceValue = pc_PrinterServiceSetting->getPrinterAdjust( TYPE_PrinterAdjustLDLightC );
		printf( "LDLightBalanceValue(C) = %d\n", ss_LDAdjustmentLightBalanceValue );
		break;
	case 2:
		ss_LDAdjustmentLightBalanceValue = pc_PrinterServiceSetting->getPrinterAdjust( TYPE_PrinterAdjustLDLightM );
		printf( "LDLightBalanceValue(M) = %d\n", ss_LDAdjustmentLightBalanceValue );
		break;
	case 3:
		ss_LDAdjustmentLightBalanceValue = pc_PrinterServiceSetting->getPrinterAdjust( TYPE_PrinterAdjustLDLightY );
		printf( "LDLightBalanceValue(Y) = %d\n", ss_LDAdjustmentLightBalanceValue );
		break;
	case 4:
		ss_LDAdjustmentLightBalanceValue = pc_PrinterServiceSetting->getPrinterAdjust( TYPE_PrinterAdjustLDLightK );
		printf( "LDLightBalanceValue(K) = %d\n", ss_LDAdjustmentLightBalanceValue );
		break;
	default:
		printf( "incorrect value \n" );
		break;
	}
}

/*================================================================================================
［機能］	ターミナルコマンド（ LD光量バランス補正値を設定する ) 
--------------------------------------------------------------------------------------------------
［引数］	uc_mode ( 1:C, 2:M, 3:Y, 4:K ). 
			ss_Data LD光量バランス補正値( -10～10  (ステップ1)). 
［戻値］	なし. 
［詳細］	出荷前検査装置  LD光量バランス補正値を設定する. 
［ｳｫｰｸｽﾙｰ］	未実施. 
--------------------------------------------------------------------------------------------------
 [履歴]
 '19/10/28 Sky鳥居	新規作成 OP_BTS-13704[SparrowBK]LD光量バランス補正値を設定するコマンド追加. 
================================================================================================*/
void SetLDAdjustData( uchar uc_mode, short ss_Data )
{
	if( checkLineMode() == False ){		// ラインモードであるかの確認.
		return;
	}

	short ss_LDAdjustmentLightBalanceValue;

	// LD光量バランス補正値範囲チェック.
	if(( THRD_PrinterAdjust_LDLight_Max < ss_Data )||( THRD_PrinterAdjust_LDLight_Min > ss_Data )){
		printf( "incorrect value\n" );
		return;
	}

	switch(uc_mode) {
	case 1:
		ss_LDAdjustmentLightBalanceValue = pc_PrinterServiceSetting->getPrinterAdjust( TYPE_PrinterAdjustLDLightC );
		printf( "Last LDLightBalanceValue(C) = %d\n", ss_LDAdjustmentLightBalanceValue );
		pc_PrinterServiceSetting->setPrinterAdjust( TYPE_PrinterAdjustLDLightC, ss_Data );
		printf( "New LDLightBalanceValue(C) = %d\n", ss_Data );
		break;
	case 2:
		ss_LDAdjustmentLightBalanceValue = pc_PrinterServiceSetting->getPrinterAdjust( TYPE_PrinterAdjustLDLightM );
		printf( "Last LDLightBalanceValue(M) = %d\n", ss_LDAdjustmentLightBalanceValue );
		pc_PrinterServiceSetting->setPrinterAdjust( TYPE_PrinterAdjustLDLightM, ss_Data );
		printf( "New LDLightBalanceValue(M) = %d\n", ss_Data );
		break;
	case 3:
		ss_LDAdjustmentLightBalanceValue = pc_PrinterServiceSetting->getPrinterAdjust( TYPE_PrinterAdjustLDLightY );
		printf( "Last LDLightBalanceValue(Y) = %d\n", ss_LDAdjustmentLightBalanceValue );
		pc_PrinterServiceSetting->setPrinterAdjust( TYPE_PrinterAdjustLDLightY, ss_Data );
		printf( "New LDLightBalanceValue(Y) = %d\n", ss_Data );
		break;
	case 4:
		ss_LDAdjustmentLightBalanceValue = pc_PrinterServiceSetting->getPrinterAdjust( TYPE_PrinterAdjustLDLightK );
		printf( "Last LDLightBalanceValue(K) = %d\n", ss_LDAdjustmentLightBalanceValue );
		pc_PrinterServiceSetting->setPrinterAdjust( TYPE_PrinterAdjustLDLightK, ss_Data );
		printf( "New LDLightBalanceValue(K) = %d\n", ss_Data );
		break;
	default:
		printf( "incorrect value\n" );
		break;
	}
}

/*================================================================================================
［機能］	ターミナルコマンド( ADF 通紙方向倍率 表示 ) 
--------------------------------------------------------------------------------------------------
［引数］	なし. 
［戻値］	なし. 
［詳細］	出荷前検査装置  ADF 通紙方向倍率. 
［ｳｫｰｸｽﾙｰ］	未実施. 
--------------------------------------------------------------------------------------------------
 [履歴]
 '08/03/21 sunami CQ:301776	 新規作成 出荷前検査装置  ADF 通紙方向倍率. 
================================================================================================*/
void GetADFZoom()
{

	if( checkLineMode() == False ){		// ラインモードであるかの確認.
		return;
	}

	short ss_ADFFeedDirectionZoom = (short)(pc_ScanSetting->getScannerAdjust( TYPE_ScannerAdjustEDHPassAdjustMag ) - 1000);
	printf( "ADFFeedDirectionZoom = %d\n", ss_ADFFeedDirectionZoom );
}

#ifndef DEF_DonauBKCus // 10/04/23 hanaoka CQ325164出荷前検査装置対応. // 2011/02/23 ft_kondo 第6回手動マージ. 
// ================================================================================================.
//! <!--機能--> ターミナルコマンド( ADF調整値表示(主走査方向倍率)  )  .
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! 出荷前検査装置  ADF調整値表示(主走査方向倍率) \n.
//! <!--履歴-->.
//  09/09/13 F.Karasaki CQ:316540 新規作成 DonauDS 出荷前検査装置対応.
// ================================================================================================.
void GetADFMainZoom()
{
	if( checkLineMode() == False ){		// ラインモードであるかの確認.
		return;
	}

	short s_Data = (short)(pc_ScanSetting->getScannerAdjust( TYPE_ScannerAdjustMainMagBack ) - 1000);
	printf( "ADFMainScanningDirectionZoom = %d\n", s_Data );
}

// ================================================================================================.
//! <!--機能--> ターミナルコマンド( ADF調整値表示(裏面通紙方向倍率) ) .
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! 出荷前検査装置  ADF調整値表示(裏面通紙方向倍率)\n.
//! <!--履歴-->.
//  09/09/13 F.Karasaki CQ:316540 新規作成 DonauDS 出荷前検査装置対応.
// ================================================================================================.
void GetADFZoomBack()
{
	if( checkLineMode() == False ){		// ラインモードであるかの確認.
		return;
	}

	short s_Data = (short)(pc_ScanSetting->getScannerAdjust( TYPE_ScannerAdjustSubMagBack ) - 1000);
	printf( "ADFFeedDirectionZoomBack = %d\n", s_Data );
}

// ================================================================================================.
//! <!--機能--> ターミナルコマンド( ADF調整値表示(ホーム位置（シェーディング軸）) ) .
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! 出荷前検査装置  ADF調整値表示(ホーム位置（シェーディング軸）)\n.
//! <!--履歴-->.
//  09/09/13 F.Karasaki CQ:316540 新規作成 DonauDS 出荷前検査装置対応.
//  09/09/30 tadamasa   CQ:なし 仕様変更の為、修正.
// ================================================================================================.
void GetADFHomePosition()
{
	if( checkLineMode() == False ){		// ラインモードであるかの確認.
		return;
	}

	short s_Data = pc_ScanSetting->getScannerAdjust( TYPE_ScannerAdjustCISOpposingBoardHomePosition );
	printf( "ADFHomeReadPosition = %d\n", s_Data );
}
// ================================================================================================.
//! <!--機能--> ターミナルコマンド( ADF調整値設定(ホーム位置（シェーディング軸）) ) .
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param int32_t l_mode ADF調整値.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! 出荷前検査装置  ADF調整値設定(ホーム位置（シェーディング軸）)\n.
//! <!--履歴-->.
//  09/09/26 tadamasa CQ:321014 新規作成 DonauDS 出荷前検査装置対応.
// ================================================================================================.
#define	DEF_ADFHomePosRangeMax  4 													    // ADF調整の最大値.
#define	DEF_ADFHomePosRangeMin -4														// ADF調整の最小値.

void SetADFHomePosition( int32_t l_Mode )
{
	if( checkLineMode() == False ){														// ラインモードであるかの確認.
		return;
	}

	if( ( l_Mode < DEF_ADFHomePosRangeMin ) || ( DEF_ADFHomePosRangeMax < l_Mode ) ){	// 正常な範囲の値かどうかを判断.
		printf( "incorrect value  \n" );
		return;
	}

	pc_ScanSetting->setScannerAdjust( TYPE_ScannerAdjustCISOpposingBoardHomePosition, (short)l_Mode );
	printf( "Set ADFHomeReadPosition = %"PRId32"\n", l_Mode );
}

#endif //DEF_DonauBKCus.
// ================================================================================================.
//! <!--機能--> ＴＣＲセンサを表示する.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param const char           *c_TCRSensorName 表示文字列.
//! \param TYPE_TCRSensorColor　e_TCRSensorColor 表示カラー.
//! \param short                s_mode           (0:高速　1：低速).
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! 出荷前検査装置  ＴＣＲセンサを表示する\n.
//! <!--履歴-->.
//  09/09/26 tadamasa CQ:321014 新規作成 出荷前検査装置対応.
// ================================================================================================.
typedef short TYPE_TCRSensorColor; // カラー選択用.
 #define	TYPE_TCRSensorColorC 0 // シアンのオフセット値.
 #define	TYPE_TCRSensorColorM 1 // マゼンダのオフセット値.
 #define	TYPE_TCRSensorColorY 2 // イエローのオフセット値.
 #define	TYPE_TCRSensorColorK 3 // ブラックのオフセット値.

void GetTCRSensorCommon( const char *c_TCRSensorName,  TYPE_TCRSensorColor e_TCRSensorColor, short s_Mode = 0 );

void GetTCRSensorCommon( const char *c_TCRSensorName,  TYPE_TCRSensorColor e_TCRSensorColor, short s_Mode )
{

	if( checkLineMode() == False ){	// ラインモードであるかの確認.
		return;
	}

	APIC_PrinterServiceSetting* pc_PrinterServiceSetting = APIC_PrinterServiceSetting::newInstance();

	switch( s_Mode ){
	case 0: 						// 高速モード.
		printf( "%s = %d\n", c_TCRSensorName, pc_PrinterServiceSetting->getATDCManual( (TYPE_PrinterAdjust)(TYPE_PrinterAdjustATDCManualC + e_TCRSensorColor) ) );
		break;
	case 1: 						// 低速モード.
		printf( "%s = %d\n", c_TCRSensorName, pc_PrinterServiceSetting->getATDCManual( (TYPE_PrinterAdjust)(TYPE_PrinterAdjustATDCManualCL + e_TCRSensorColor) ) );
		break;
	default:
		printf( "incorrect value \n" );
		break;
	}
}

#if defined(DEF_MINERVASBK_CUS)	//2015/3/5 MinervaSBK条件追加.
// モノクロ機の場合は定義しない.
#elif defined(DEF_ZeusCus) || defined(DEF_ZeusEmu)	// Zeusは引数あり.	
// ================================================================================================
//! <!--機能--> ＴＣＲセンサ（C）を表示する.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param short s_mode (0:高速　1：低速).
//! <!--戻値-->
//! \return なし.
//! <!--詳細-->
//! 出荷前検査装置  ＴＣＲセンサ（C）を表示する Amur用\n
//! <!--履歴-->
//  09/09/26 tadamasa CQ:321014 新規作成  出荷前検査装置対応.
// ================================================================================================
void GetTCRSensorC( short s_Mode )
{
	GetTCRSensorCommon( "TCRSensorC", TYPE_TCRSensorColorC, s_Mode );
}

// ================================================================================================
//! <!--機能--> ＴＣＲセンサ（M）を表示する.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param short s_mode (0:高速　1：低速).
//! <!--戻値-->
//! \return なし.
//! <!--詳細-->
//! 出荷前検査装置  ＴＣＲセンサ（M）を表示する Amur用\n
//! <!--履歴-->
//  09/09/26 tadamasa CQ:321014 新規作成 出荷前検査装置対応.
// ================================================================================================
void GetTCRSensorM( short s_Mode )
{
	GetTCRSensorCommon( "TCRSensorM", TYPE_TCRSensorColorM, s_Mode );
}
// ================================================================================================
//! <!--機能--> ＴＣＲセンサ（Y）を表示する.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param short s_mode (0:高速　1：低速).
//! <!--戻値-->
//! \return なし.
//! <!--詳細-->
//! 出荷前検査装置  ＴＣＲセンサ（Y）を表示する Amur用\n
//! <!--履歴-->
//  09/09/26 tadamasa CQ:321014 新規作成 出荷前検査装置対応.
// ================================================================================================
void GetTCRSensorY( short s_Mode )
{
	GetTCRSensorCommon( "TCRSensorY", TYPE_TCRSensorColorY, s_Mode );
}
#else	// Minerva引数なし.
// ================================================================================================.
//! <!--機能--> ＴＣＲセンサ（C）を表示する.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! 出荷前検査装置  ＴＣＲセンサ（C）を表示する Donau用\n.
//! <!--履歴-->.
//  09/09/26 tadamasa CQ:321014 新規作成  出荷前検査装置対応.
// ================================================================================================.
void GetTCRSensorC()
{
	GetTCRSensorCommon( "TCRSensorC", TYPE_TCRSensorColorC );
}

// ================================================================================================.
//! <!--機能--> ＴＣＲセンサ（M）を表示する.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! 出荷前検査装置  ＴＣＲセンサ（M）を表示する Donau用\n.
//! <!--履歴-->.
//  09/09/26 tadamasa CQ:321014 新規作成 出荷前検査装置対応.
// ================================================================================================.
void GetTCRSensorM()
{
	GetTCRSensorCommon( "TCRSensorM", TYPE_TCRSensorColorM );
}

// ================================================================================================.
//! <!--機能--> ＴＣＲセンサ（Y）を表示する.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! 出荷前検査装置  ＴＣＲセンサ（Y）を表示する Donau用\n.
//! <!--履歴-->.
//  09/09/26 tadamasa CQ:321014 新規作成 出荷前検査装置対応.
// ================================================================================================.
void GetTCRSensorY()
{
	GetTCRSensorCommon( "TCRSensorY", TYPE_TCRSensorColorY );
}
#endif // #if defined(DEF_ZeusCus) || defined(DEF_ZeusEmu).

#if defined(DEF_ZeusCus) || defined(DEF_ZeusEmu)
// ================================================================================================.
//! <!--機能--> ＴＣＲセンサ（K）を表示する.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param short s_mode (0:高速　1：低速).
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! 出荷前検査装置  ＴＣＲセンサ（K）を表示する Amur用\n.
//! <!--履歴-->.
//  09/09/26 tadamasa CQ:321014 新規作成 出荷前検査装置対応.
// ================================================================================================.
void GetTCRSensorK( short s_Mode )
{
	GetTCRSensorCommon( "TCRSensorK", TYPE_TCRSensorColorK, s_Mode );
}
#else
// ================================================================================================.
//! <!--機能--> ＴＣＲセンサ（K）を表示する.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! 出荷前検査装置  ＴＣＲセンサ（K）を表示する Donau用\n.
//! <!--履歴-->.
//  09/09/26 tadamasa CQ:321014 新規作成 出荷前検査装置対応.
// ================================================================================================.
void GetTCRSensorK()
{
	GetTCRSensorCommon( "TCRSensorK", TYPE_TCRSensorColorK );
}
#endif // #if defined(DEF_ZeusCus) || defined(DEF_ZeusEmu).

/* IT3からマージ */
#if defined(DEF_ZeusCus) || defined(DEF_ZeusEmu) || defined(DEF_MINERVASBK_CUS) //Zeus固有 10/04/23 hanaoka CQ325164出荷前検査装置対応. // 2015/3/5 MinervaSBK条件追加.
//定数定義.
#define EngineChoice1_Bit			(0x10)	//エンジン個別対応チョイス5Bit目.
#define FuserTempUser1Upper			(-10)	//定着加熱ローラ温度調整：欧州以外の仕向け.
#define FuserTempUser1Upper_Europe 	(0)		//定着加熱ローラ温度調整：欧州仕向け.
#define SysType_ExcludingEurope		(0)		//欧州以外.
#define SysType_Europe				(1)		//欧州.
#endif //#if defined(DEF_ZeusCus) || defined(DEF_ZeusEmu) || defined(DEF_MINERVASBK_CUS).

#if defined(DEF_ZeusCus) || defined(DEF_ZeusEmu) //Zeus固有 10/04/23 hanaoka CQ325164出荷前検査装置対応.
/*================================================================================================
［機能］	ターミナルコマンド（WUP完了温度チョイス設定).
--------------------------------------------------------------------------------------------------
［引数］	bl_SystemType　仕向け値(0:欧州以外,1,欧州).
［戻値］	設定した仕向け別エンジン個別対応チョイス値.
［詳細］	出荷前検査装置  WUP完了温度チョイスのデフォルト値を仕向け別に設定.
［ｳｫｰｸｽﾙｰ］	未実施.
--------------------------------------------------------------------------------------------------
 [履歴].
 '09/02/19 花岡 CQ:315869	 新規作成 出荷前検査装置  出荷前検査装置書き込み追加.
================================================================================================*/
short SetWUPCompleteTemperatureChoice(Bool bl_SystemType = 0)
{
	short ss_Data = 0x00;
	short ss_EngineChoice1Bit = EngineChoice1_Bit;
	
	ss_Data = pc_PrinterServiceSetting->getPrinterAdjust(TYPE_PrinterAdjustEngineChoice1);		//エンジン個別対応チョイス現在値取得.
	
	//引数判定.
	if(bl_SystemType == SysType_Europe){
		ss_Data = (short)(ss_Data | ss_EngineChoice1Bit);										//欧州向けデフォルト値設定.
		pc_PrinterServiceSetting->setPrinterAdjust(TYPE_PrinterAdjustEngineChoice1,ss_Data);	//エンジン個別対応チョイス仕向け別デフォルト値設定.
	}else if(bl_SystemType == SysType_ExcludingEurope){
		ss_Data = (short)(ss_Data  & ss_EngineChoice1Bit);										//欧州向け以外デフォルト値設定.
		pc_PrinterServiceSetting->setPrinterAdjust(TYPE_PrinterAdjustEngineChoice1,ss_Data);	//エンジン個別対応チョイス仕向け別デフォルト値設定.
	}else{
		printf( "Argument error \n");
		printf("Set Argument (0:Excluding Europe,1:Europe)\n");
	}
	
	return pc_PrinterServiceSetting->getPrinterAdjust(TYPE_PrinterAdjustEngineChoice1);
}

/*================================================================================================
［機能］	ターミナルコマンド（WUP完了温度チョイス取得).
--------------------------------------------------------------------------------------------------
［引数］	なし.
［戻値］	エンジン個別対応チョイス値.
［詳細］	出荷前検査装置  WUP完了温度チョイスの現在値取得.
［ｳｫｰｸｽﾙｰ］	未実施.
--------------------------------------------------------------------------------------------------
 [履歴].
 '09/02/19 花岡 CQ:315869	 新規作成 出荷前検査装置  出荷前検査装置書き込み追加.
================================================================================================*/
short GetWUPCompleteTemperatureChoice()
{
	return pc_PrinterServiceSetting->getPrinterAdjust(TYPE_PrinterAdjustEngineChoice1);
}
#endif //#if defined(DEF_ZeusCus) || defined(DEF_ZeusEmu).

#if defined(DEF_ZeusCus) || defined(DEF_ZeusEmu) || defined(DEF_MINERVASBK_CUS) // 2015/3/5 MinervaSBK条件追加.
/*================================================================================================
［機能］	ターミナルコマンド（定着加熱ローラ温度調整設定).
--------------------------------------------------------------------------------------------------
［引数］	bl_SystemType　仕向け値(0:欧州以外,1,欧州).
［戻値］	設定した仕向け別定着加熱ローラ温度調整値.
［詳細］	出荷前検査装置  定着加熱ローラ温度調整のデフォルト値を仕向け別に設定.
［ｳｫｰｸｽﾙｰ］	未実施.
--------------------------------------------------------------------------------------------------
 [履歴].
 '09/02/19 花岡 CQ:315869	 新規作成 出荷前検査装置  出荷前検査装置書き込み追加.
================================================================================================*/
short SetEstablishedHeatRoller(Bool bl_SystemType = 0)
{
	short ss_Data = 0x00;
	
	ss_Data = pc_PrinterServiceSetting->getPrinterAdjust(TYPE_PrinterAdjustFuserTempUser1Upper);	//定着加熱ローラ温度調整：現在値取得.
	
	//引数判定.
	if(bl_SystemType == SysType_Europe){
		ss_Data = FuserTempUser1Upper_Europe;  														//定着加熱ローラ温度調整値：欧州の仕向け.
		pc_PrinterServiceSetting->setPrinterAdjust(TYPE_PrinterAdjustFuserTempUser1Upper,ss_Data);	//定着加熱ローラ温度調整デフォルト値設定.
	}else if(bl_SystemType == SysType_ExcludingEurope){
		ss_Data = FuserTempUser1Upper;																//定着加熱ローラ温度調整：欧州以外の仕向け.
		pc_PrinterServiceSetting->setPrinterAdjust(TYPE_PrinterAdjustFuserTempUser1Upper,ss_Data);	//定着加熱ローラ温度調整デフォルト値設定.
	}else{
		printf( "Argument error \n");
		printf("Set Argument (0:Excluding Europe,1:Europe)\n");
	}
	
	return pc_PrinterServiceSetting->getPrinterAdjust(TYPE_PrinterAdjustFuserTempUser1Upper);
}

/*================================================================================================
［機能］	ターミナルコマンド（定着加熱ローラ温度調整取得).
--------------------------------------------------------------------------------------------------
［引数］	なし.
［戻値］	定着加熱ローラ温度調整.
［詳細］	出荷前検査装置  定着加熱ローラ温度調整値取得.
［ｳｫｰｸｽﾙｰ］	未実施.
--------------------------------------------------------------------------------------------------
 [履歴].
 '09/02/19 花岡 CQ:315869	 新規作成 出荷前検査装置  出荷前検査装置書き込み追加.
================================================================================================*/
short GetEstablishedHeatRoller()
{
	return pc_PrinterServiceSetting->getPrinterAdjust(TYPE_PrinterAdjustFuserTempUser1Upper);
}

 // =================================================================================================
//! <!--機能--> ターミナルコマンド｛定着温度微調（加熱側：普通紙）値を設定する｝.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param ss_Value 定着温度微調（加熱側：普通紙）値 ※範囲-20～+5 5単位で設定する ※DenebMLKは範囲-5～20.
//! <!--戻値-->
//! \return なし.
//! <!--詳細-->
//! 出荷前検査装置 定着温度微調（加熱側：普通紙）値を設定する\n
//! <!--履歴-->
// 09/09/28 t-yokoyama CQ:321014	新規作成 Amur 出荷前検査装置対応.
// 18/07/17 Kaihotsu 【DenebMLK 生産コマンド対応（コマンド追加対応）:制御構成変更_No.45 給紙カセット対応】.
// =================================================================================================
#if defined(DEF_DenebMLKCus)	// DenebMLK.
#define THRD_FuserTempNormalHUpperMAX	20																	// 定着温度微調_加熱側：普通紙の最大値.
#define THRD_FuserTempNormalHUpperMIN	-5																	// 定着温度微調_加熱側：普通紙の最小値.
#else
#define THRD_FuserTempNormalHUpperMAX	5																	// 定着温度微調_加熱側：普通紙の最大値.
#define THRD_FuserTempNormalHUpperMIN	-20																	// 定着温度微調_加熱側：普通紙の最小値.
#endif // DenebMLK.
void SetFusingTemperatureHeatRollerPlainpaper( short ss_Value )
{
	if( ( THRD_FuserTempNormalHUpperMIN <= ss_Value )
		&& ( ss_Value <= THRD_FuserTempNormalHUpperMAX )
		&& ( 0 == ss_Value % 5 ) ){																		// 5単位で設定する為.
		pc_PrinterServiceSetting->setPrinterAdjust( TYPE_PrinterAdjustFuserTempNormalHUpper,ss_Value );	// 定着加熱ヒータ温度調整デフォルト値設定.
		printf("Set FusingTemperatureHeatRollerPlainpaper = %d\n", 
				pc_PrinterServiceSetting->getPrinterAdjust( TYPE_PrinterAdjustFuserTempNormalHUpper ) );
	}else{
		printf("incorrect value\n");																	// 引数の値が範囲外の場合.
	}
}

// =================================================================================================
//! <!--機能--> ターミナルコマンド｛定着温度微調（加熱側：普通紙）値を表示する｝.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param なし.
//! <!--戻値-->
//! \return なし.
//! <!--詳細-->
//! 出荷前検査装置 定着温度微調（加熱側：普通紙）値を表示する\n
//! <!--履歴-->
// 09/09/28 t-yokoyama CQ:321014	新規作成 Amur 出荷前検査装置対応.
// =================================================================================================
void GetFusingTemperatureHeatRollerPlainpaper()
{
	printf("FusingTemperatureHeatRollerPlainpaper = %d\n", 
			pc_PrinterServiceSetting->getPrinterAdjust( TYPE_PrinterAdjustFuserTempNormalHUpper ) );
}
#endif //#if defined(DEF_ZeusCus) || defined(DEF_ZeusEmu) || defined(DEF_MINERVASBK_CUS).	IT3からマージ.


/*================================================================================================
［機能］	ターミナルコマンド( ADF 原稿停止位置調整：副走査１面目 表示 ) 
--------------------------------------------------------------------------------------------------
［引数］	なし. 
［戻値］	なし. 
［詳細］	出荷前検査装置  ADF 原稿停止位置調整：副走査１面目. 
［ｳｫｰｸｽﾙｰ］	未実施. 
--------------------------------------------------------------------------------------------------
 [履歴]
 '08/03/21 sunami CQ:301776	 新規作成 出荷前検査装置  ADF 原稿停止位置調整：副走査１面目. 
================================================================================================*/
void GetADF1SideStopPosition()
{

	if( checkLineMode() == False ){		// ラインモードであるかの確認.
		return;
	}

	short ss_ADF1SideStopPosition;

	ss_ADF1SideStopPosition = pc_ScanSetting->getScannerAdjust( TYPE_ScannerAdjust1SideOriginalStopPositon );
	printf( "ADF1SideStopPosition = %d\n", ss_ADF1SideStopPosition );
}

/*================================================================================================
［機能］	ターミナルコマンド( ADF 原稿停止位置調整：副走査２面目 表示 ) 
--------------------------------------------------------------------------------------------------
［引数］	なし. 
［戻値］	なし. 
［詳細］	出荷前検査装置  ADF 原稿停止位置調整：副走査２面目. 
［ｳｫｰｸｽﾙｰ］	未実施. 
--------------------------------------------------------------------------------------------------
 [履歴]
 '08/03/21 sunami CQ:301776	 新規作成 出荷前検査装置  ADF 原稿停止位置調整：副走査２面目. 
================================================================================================*/
void GetADF2SideStopPosition()
{

	if( checkLineMode() == False ){		// ラインモードであるかの確認.
		return;
	}

	short ss_ADF2SideStopPosition;

	ss_ADF2SideStopPosition = pc_ScanSetting->getScannerAdjust( TYPE_ScannerAdjust2SideOriginalStopPosition );
	printf( "ADF2SideStopPosition = %d\n", ss_ADF2SideStopPosition );
}

/*================================================================================================
［機能］	ターミナルコマンド( ADF 原稿停止位置調整：主走査 表示 ) 
--------------------------------------------------------------------------------------------------
［引数］	なし. 
［戻値］	なし. 
［詳細］	出荷前検査装置  ADF 原稿停止位置調整：主走査. 
［ｳｫｰｸｽﾙｰ］	未実施. 
--------------------------------------------------------------------------------------------------
 [履歴]
 '08/03/21 sunami CQ:301776	 新規作成 出荷前検査装置  ADF 原稿停止位置調整：主走査. 
================================================================================================*/
void GetADFStopPositionCD()
{

	if( checkLineMode() == False ){		// ラインモードであるかの確認.
		return;
	}

	short ss_ADFStopPositionCD;

	ss_ADFStopPositionCD = pc_ScanSetting->getScannerAdjust( TYPE_ScannerAdjustADFStopPositionCD );
	printf( "ADFStopPositionCD = %d\n", ss_ADFStopPositionCD );
}

/*================================================================================================
［機能］	ターミナルコマンド( ADF 原稿停止位置調整：主走査2面目 表示 ).
--------------------------------------------------------------------------------------------------
［引数］	なし.
［戻値］	なし.
［詳細］	出荷前検査装置  ADF 原稿停止位置調整：主走査2面目.
［ｳｫｰｸｽﾙｰ］	未実施.
--------------------------------------------------------------------------------------------------
 [履歴].
 '08/08/26 鈴村 CQ:308566	 新規作成 出荷前検査装置  ADF 原稿停止位置調整：主走査2面目.
================================================================================================*/
void Get2SideADFStopPositionCD()
{

	if( checkLineMode() == False ){		// ラインモードであるかの確認.
		return;
	}

	short ss_2SideADFStopPositionCD = 0;

	ss_2SideADFStopPositionCD = pc_ScanSetting->getScannerAdjust( TYPE_ScannerAdjust2SideADFStopPositionCD );
	printf( "2SideADFStopPositionCD = %d\n", ss_2SideADFStopPositionCD );
}


//【課題】暫定対応.
/*================================================================================================
［機能］	【内部】生産性評価ログ
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	無し
 [作成者]	R.Suzuki
［ｳｫｰｸｽﾙｰ］	未実施
 [変更履歴]	05/05/19 新規追加
================================================================================================*/
// プロトタイプ宣言.
void _ResetPPM();
void _ChkPPM(int32_t avg);

// ターミナルから使用.
extern "C"
{
void ResetPPM();
void CheckPPM(int32_t avg);
}

void ResetPPM()
{
	_ResetPPM();
}

void CheckPPM(int32_t avg)
{
	_ChkPPM( avg );
}

#define	ST_THROUGHPUTTBL_SIZE 100
static	uint32_t  ul_time[ ST_THROUGHPUTTBL_SIZE ] = { 0 };
static	ushort us_PPMTblIdx = 0;
static	ushort us_PPMTblFlg = 0;

void _SavePPM()
{
	ul_time[ us_PPMTblIdx % ST_THROUGHPUTTBL_SIZE ] = SYSF_tickGet();

	us_PPMTblIdx++;
	if( us_PPMTblIdx >= ST_THROUGHPUTTBL_SIZE ){
		us_PPMTblIdx = 0;
		us_PPMTblFlg = 1;
	}
}
void _ResetPPM() // 計測しなおし.
{
	uchar i;
	us_PPMTblIdx = 0;
	us_PPMTblFlg = 0;
	for( i=0; i<ST_THROUGHPUTTBL_SIZE; i++){
		ul_time[ i ] = 0;
	}
	printf("PPM Log reset\n");
}
void _ChkPPM(int32_t avg)
{
	register int32_t i, No, idx = 0;
	int32_t    AvgNum = 10;
	double sd_PPM = 0.0, sd_TotalSec = 0.0, sd_MSec = 0.0;
	int32_t   sl_Tick = 0;

	if( us_PPMTblFlg == 0 ){
		if( us_PPMTblIdx == 0 ){
			return;
		}
		No = us_PPMTblIdx;
	}else{
		No = ST_THROUGHPUTTBL_SIZE;
	}
	for( i=0; i<No; i++ ) {
		printf( "No.[%03"PRId32"]%s ",i,(us_PPMTblIdx==i)?"*":" " );
		printf( "%7"PRId32" ", ul_time[ i ]);
		if( i == 0 ){
			if( us_PPMTblFlg == 0 ){
				sd_PPM = 0.0;
			}else{
				sl_Tick = ( (int32_t)ul_time[ i % ST_THROUGHPUTTBL_SIZE ] - 
							(uint32_t)ul_time[ ST_THROUGHPUTTBL_SIZE - 1 ] );
				if( sl_Tick <= 0 ){
					sd_MSec = 0.0;
					sd_PPM = 0.0;
				}else{
					sd_MSec = (int32_t)((double)sl_Tick / (double)SYSF_sysClkRateGet() * 1000);
					sd_PPM  = 60.0 / ( (double)sl_Tick / (double)SYSF_sysClkRateGet() );
				}
			}
		}else{
			sl_Tick = ( (int32_t)ul_time[ i % ST_THROUGHPUTTBL_SIZE ] - 
						(uint32_t)ul_time[ ( i - 1 ) % ST_THROUGHPUTTBL_SIZE ] );
			if( sl_Tick <= 0 ){
				sd_MSec = 0.0;
				sd_PPM  = 0.0;
			}else{
				sd_MSec = (double)sl_Tick / (double)SYSF_sysClkRateGet() * 1000.0;
				sd_PPM  = 60.0 / ( (double)sl_Tick / (double)SYSF_sysClkRateGet() );
			}
		}
		printf("%7.0f  ", sd_MSec);
		printf("%5.2f\n", sd_PPM );
	}
	// 平均の算出.
	if( 0 < avg && avg <= ( ST_THROUGHPUTTBL_SIZE - 1 ) ){
		AvgNum = avg;
	}else{
		printf("The average of 10 pieces is calculated. \n");
	}
	for( i=0; i<AvgNum; i++ ){
		if( us_PPMTblIdx <= i ){
			idx = ST_THROUGHPUTTBL_SIZE  - ( i - ( us_PPMTblIdx - 1 ) );
			sd_TotalSec += ul_time[ idx % ST_THROUGHPUTTBL_SIZE ] -	ul_time[ ( idx - 1 ) % ST_THROUGHPUTTBL_SIZE ];
		}else{
			idx = us_PPMTblIdx - 1 - i;
			if( idx == 0 ){
				sd_TotalSec += ul_time[ 0 ] - ul_time[ ST_THROUGHPUTTBL_SIZE - 1 ];
			}else{
				sd_TotalSec += ul_time[ idx % ST_THROUGHPUTTBL_SIZE ] - ul_time[ ( idx - 1 ) % ST_THROUGHPUTTBL_SIZE ];
			}
		}
	}
	if( sd_TotalSec != 0.0 ){
		printf("Average:  %5.2f ppm\n", 60.0 / ( ( sd_TotalSec / (double)SYSF_sysClkRateGet() ) / (double)AvgNum ) );
		printf("Total  : %6.2f sec\n", sd_TotalSec / (double)SYSF_sysClkRateGet());
	}
	_ResetPPM();
}

// MoselのHDDの60GByte対応でdosFsのチェックに追加.
#ifdef TORNADO

#define uint32 uint32_t
#define OLM_SIM_OK 1

int32_t FreeTest();

int32_t WriteTest(int32_t count, uint32_t size)
{
	char filename[256];
	int32_t fd = -1;
	int32_t x, y;
	uint32_t size32 = size;
	int32_t objId=0;
	char *pBuf, *buf;
	ssize_t ret=0;	//2017/01/24 int32_tからssize_tに変更.

	// 04/06/23 morikawa debug.
	// 06/01/29 morikawa debug.
	// 8MB×4000個で32GByteなので.
	uint32_t size32_max = 8*1024*1024; 

	if(size32 == 0)
		size32 = size32_max;
	if(size32 > size32_max){
		printf("Error Size : Change size size32_max\n");
		size32 = size32_max;
	}

	if(count == 0)
		count = 100;
	if(count > 4000){
		printf( "Error Size : Change count 4000\n");
		count = 4000;
	}

	if ((pBuf = (char *)malloc(size32)) == NULL)
		return(ERROR);

	int32_t x_max = size32/1024;	// 04/06/23 morikawa debug.
	
	buf = pBuf;
	for(y=0; y<1024; y++){
		for(x=0; x<x_max; x++){
			*buf = (char)x;
			buf++;
		}
	}
	printf("WriteTest\n");

	for(objId=0; objId<count; objId++){

	sprintf(filename, "%s%s/%6.6"PRId32".olm", "/ata01/sys/","olm_dir",objId);

{
		char dirname[256];
		int32_t dirnamestatret;
		struct stat dirnamestat;

		strcpy(dirname, "/ata01/sys/olm_dir");

		dirnamestatret = stat(dirname, &dirnamestat);
		if (dirnamestatret != 0) {
			if( objId != 0 ) {
				printf("dir no Exist Error!\n");
				break;
			}	
			
			if (mkdir(dirname, 0777) != 0) {
				printf( "WriteTest: init error\n");
			}
			else
				printf( "WriteTest: make dir\n");
		}
}
		if ((fd = SYSF_open(filename, (O_CREAT | O_WRONLY ), 0666)) >= 0) {
			ret = (int32_t)write(fd, (char *)(&size32), sizeof(uint32_t));
			if(ret != sizeof(uint32_t)) {
				printf( "    WriteError hdr : %"PRId32" != %zd\n", ret, sizeof(uint32_t));
				SYSF_close( fd ) ;
				break;	// 04/06/16　morikawa.
			}
			ret = write(fd, (char *)pBuf, size32);
			if((uint32_t)ret != size32) {  
				printf( "    WriteError Buf: %"PRId32" != %"PRIu32"\n", ret, size32); 
				SYSF_close( fd ) ;
				break;	// 04/06/16　morikawa.
			}
			
			SYSF_close( fd ) ;
			printf( "  StoreHDD: %s\n", filename );
		}
		else
			printf( "    Cannot open file\n");
	}
	free(pBuf);
	return OLM_SIM_OK;
}
int32_t ReadTest(int32_t count)
{
	char filename[256];
	int32_t fd = -1;
	int32_t objId=0;
	char *pBuf;
	ssize_t ret=0;
	
	if(count == 0)
		count = 100;
	if(count > 4000){
		printf( "Error Size : Change count 4000\n");
		count = 4000;
	}

	printf("ReadTest\n");
	pBuf = NULL;	// 初期化.

	for(objId=0; objId<count; objId++){

		sprintf(filename, "%s%s/%6.6"PRId32".olm", "/ata01/sys/","olm_dir",objId);

		{
			char dirname[256];
			int32_t dirnamestatret;
			struct stat dirnamestat;

			strcpy(dirname, "/ata01/sys/olm_dir");

			dirnamestatret = stat(dirname, &dirnamestat);
			if (dirnamestatret != 0) {
				if (mkdir(dirname, 0777) != 0) {
					printf( "ReadTest: init error\n");
				}
				else
					printf( "ReadTest: make dir\n");
			}
		}
		if ((fd = SYSF_open(filename, O_RDONLY, 0666)) >= 0) {
			uint32_t size32 = 0;

			/* acquire data length */
			ret =  (int32_t)read(fd, (char *)(&size32), sizeof(uint32_t));
			if(ret != sizeof(uint32_t))
				printf( "    ReadError hdr : %"PRId32" != %zd\n", ret, sizeof(uint32_t));
			
			if( pBuf == NULL ) {
				// 最初の１回のみ確保する.
				if ((pBuf = (char *)malloc(size32)) == NULL) {
					SYSF_close( fd );
					return(ERROR);
				}
			}
			/* restore to pBuf */
			ret = read(fd, (char *)pBuf, size32);
			if((uint32_t)ret != size32) 
				printf( "    ReadError Buf: %"PRId32" != %"PRIu32"\n", ret, size32);	
			SYSF_close( fd ) ;
			printf( "RetrieveHDD: %s\n", filename );
			if( objId + 1 >= count ) {
				free(pBuf);
				pBuf = NULL; // 2011.12.14 s.yamauchi 静的解析対応：CID28615 free漏れ対応.
				printf( "free pBuf\n" );
			}	
		}
		else
			printf( "    Cannot open file\n");
	}
	
	if( pBuf != NULL ) // 2011.12.14 s.yamauchi 静的解析対応：CID28615 free漏れ対応.
	{
		free(pBuf);
		pBuf = NULL;
	}
	
	return OLM_SIM_OK;
}
int32_t DeleteTest(int32_t count)
{
	char filename[256];
	int32_t objId=0;

	if(count == 0)
		count = 100;
	if(count > 4000){
		printf( "Error Size : Change count 4000\n");
		count = 4000;
	}
	printf("DeleteTest\n");
	for(objId=0; objId<count; objId++){

	sprintf(filename, "%s%s/%6.6"PRId32".olm", "/ata01/sys/","olm_dir",objId);

{
		char dirname[256];
		int32_t dirnamestatret;
		struct stat dirnamestat;

		strcpy(dirname, "/ata01/sys/olm_dir");

		dirnamestatret = stat(dirname, &dirnamestat);
		if (dirnamestatret != 0) {
			if (mkdir(dirname, 0777) != 0) {
				printf( "DeleteTest: init error\n");
			}
			else
				printf( "DeleteTest: make dir\n");
		}
}
		unlink(filename);
		printf( "DeleteHDD: %s\n", filename );
	}
	return OLM_SIM_OK;
}
int32_t FreeTest()
{
	int32_t fd = -1;
	int32_t status;
	int32_t freeCount = 0;
	
	fd = SYSF_open( "/ata01", O_RDONLY, 0444 );
	if( fd == ERROR ) {
		printf("open : Can't open /ata01\n");
	} else {
		if( ( status = ioctl(fd, FIONFREE, (uintptr_t)&freeCount) ) == ERROR ) {
			printf("dosFS ioctl : ERROR OCCUR /ata01 ERROR_CODE=%"PRId32"[DEC]\n",status); 
		} else {
			printf("dosFS ioctl =%"PRId32"[byte]\n",freeCount);	// 02/04/09 morikawa debug.
		}
		SYSF_close( fd );	// 02/04/09 morikawa.
	}
	return OLM_SIM_OK;
}
#endif	// #ifdef TORNADO.

// ============================================================================.
// usecTimer.
// MV6446xのタイマー(ch1)を使用する.
// ----------------------------------------------------------------------------.
#ifndef TORNADO	// SimWinで有効.
extern "C" uint32_t usecTimerGet()
{
	return 0 ;
}
#else			// 実機で有効.

#ifdef LINUX
#include <sys/time.h>
extern "C" void startusecTimer( void )
{
	// 何もしない.
}

extern "C" uint32_t usecTimerGet( void )
{
	struct timeval	s_Tv;
	uint32_t			ul_USec;

	gettimeofday(&s_Tv, NULL);
	ul_USec = (uint32_t)(s_Tv.tv_sec * 1000000);
	ul_USec += s_Tv.tv_usec;
	
	return( ul_USec );
}

extern "C" Bool getTimerStatus( void )
{
	return( False );
}

#else  // LINUX.

//#include "vxCntmrIntCtrl.h".
#define DEF_USTMR_COUT_NUM    1									//使用するMV64460のタイマ番号.
#define DEF_USTMR_COUT_CAUSE  1									//MV64460から発生する割り込みの番号.
#define DEF_USTMR_INIT_VALUE  (133000000/100000)				// MV64460に渡すカウント値：10usec(1uだと負荷が高い).
#define DEF_USTMR_INT_PRIO    10

#if 1
/* includes */
//#include "mvCntmrRegs.h".
//#include "mvCtrlEnvLib.h".
//#include "mvCtrlEnvAddrDec.h".

/* This enumerator describes counter/timer operation mode                   */
typedef enum _mvCntmrMode
{
    COUNTER,            /* Counts down to terminal count and stops          */
    TIMER               /* Reloads itself to the programmed value           */

}MV_CNTMR_MODE;

/* Each timer/counter has a TC_NTn output pin which is asserted when the    */
/* counter reaches zero. The following enumerator determine if TcTcnt is    */
/* asserted for one or two TCLK cycles.                                     */
typedef enum _mvCntmrTcntClocks
{
    ONE_TCLK_CYC, 
    TWO_TCLK_CYC

}MV_CNTMR_TCNT_CLOCKS;

/* Counter / Timer control structure */

typedef struct _mvCntmrCtrl
{
	MV_BOOL 				enable;		/* enable                           */
	MV_BOOL 				extTrig;	/* External trigger                 */
	MV_CNTMR_MODE 			mode;		/* counter/Timer                    */
	MV_CNTMR_TCNT_CLOCKS 	clocks;		/* No of clocks TC_NTn is asserted  */

}MV_CNTMR_CTRL;

#endif
/* Load an init Value to a given counter/timer */
extern  "C" MV_STATUS mvCntmrLoad(MV_U32 countNum, MV_U32 value);

/* Get a timer/counter clock frequancy */
extern  "C" MV_U32 mvCntmrClkFreqGet(MV_U32 countNum);

/* Returns the value of the given Counter/Timer */
extern  "C" MV_U32 mvCntmrRead(MV_U32 countNum);

/* Set the Control to a given counter/timer */
extern  "C" MV_STATUS mvCntmrCtrlSet(MV_U32 countNum, MV_CNTMR_CTRL *pCtrl);

/* Get the Control value of a given counter/timer */
extern  "C" MV_STATUS mvCntmrCtrlGet(MV_U32 countNum, MV_CNTMR_CTRL *pCtrl);

/* Set the Enable-Bit to logic '1' ==> starting the counter. */
extern  "C" MV_STATUS mvCntmrEnable(MV_U32 countNum);

/* Stop the counter/timer running, and returns its Value. */
extern  "C" MV_STATUS mvCntmrDisable(MV_U32 countNum);

/* Combined all the sub-operations above to one function: Load,setMode,Enable */
extern  "C" MV_STATUS mvCntmrStart(MV_U32 countNum, MV_U32 value, MV_CNTMR_CTRL *pCtrl);


static uint32_t ul_usec_Counter = 0 ;								//時間（10μsec単位）.
static bool vxusecTimerRunning = false ;						//タイマ稼動・停止フラグ.
//**************************************************.
// 10μS（マイクロセック）の割り込み.
//		割り込み発生時のカウントアップ関数.
//		「MV64460」で発生した割り込みをVxWorks側で登録しておき、.
//		この関数をコールすることでカウントアップする.
extern "C" void usecTimerInc(int32_t arg)
{
	ul_usec_Counter+=10 ;	// 10usecサイクルで呼び出される.カウンタ.
}
//***************************************************.
//タイマの登録.
extern "C" void startusecTimer()
{
	_mvCntmrCtrl *mv_setdt;
	static _mvCntmrCtrl M_mv_setdt;
	mv_setdt = &M_mv_setdt;
	
	/* ---------------------------- */
	/* カウンタードライバーの初期化 */
	/* 複数回呼び出し可             */
	/* 最低一回は実行する           */
	/* ---------------------------- */
// 【検討必要】.
// 以下のコメント化している処理に関してはLinux化での対応が必要です.
//	vxCntmrIntCtrlInit() ;.

	// 割り込み発生時のコール関数の登録.
	// 2008/04/17 Ueda (CQ:301878) DonauにもvxCntmrIntConnect関数を追加したので実行するように変更.
//	if ( vxCntmrIntConnect(DEF_USTMR_COUT_CAUSE, (VOIDFUNCPTR)usecTimerInc, 0, DEF_USTMR_INT_PRIO) == OK ) {.
//		;//成功.
//	} else {.
//		;//失敗.
//	}.

//	vxCntmrIntEnable(DEF_USTMR_COUT_CAUSE);						//割り込みを許可する.
//	mvCntmrDisable(DEF_USTMR_COUT_NUM);							//MV64460のカウンターを無効にする.
	
	if(!vxusecTimerRunning){
		//タイマーが未使用の場合.
		mv_setdt->enable  = DEF_USTMR_COUT_NUM;					//TIMER No １番のタイマーを使用する.
		mv_setdt->extTrig = True;
		mv_setdt->mode    = TIMER;								//TIMER/COUNTERをTIMERで設定.
		mv_setdt->clocks  = ONE_TCLK_CYC;						//TC_NTn asserted for one Tclk ?.
		mvCntmrStart(DEF_USTMR_COUT_NUM,DEF_USTMR_INT_PRIO, mv_setdt);				//MV64460のTIMER/COUNTERモードをTIMERでセットする.
	}
	mvCntmrLoad(DEF_USTMR_COUT_NUM, DEF_USTMR_INIT_VALUE);		//カウント値のをMV64460に設定.
	mvCntmrEnable(DEF_USTMR_COUT_NUM);							//MV64460のカウンターを有効にする.
	vxusecTimerRunning = true;									//タイマ使用中にする.
}
//カウンタを無効にする.
extern "C" void stopusecTimer()
{
	if(vxusecTimerRunning) {
		mvCntmrDisable((MV_U32)DEF_USTMR_COUT_NUM);							//MV64460のカウンターを無効にする.
		vxusecTimerRunning = false;									//タイマ停止中にする.
	}
}
//カウンタの読み込み.
extern "C" uint32_t usecTimerGet()
{
	return ul_usec_Counter ;
}
//VxWorksのチックカウンタとMV6446xのカウンタの継続時間を表示.
extern "C" void checkusectimer()
{
	if ( vxusecTimerRunning == false ) {
		printf( "do start timer.\n" ) ;
		return ;
	}
	uint32_t ul_tick ;
	uint32_t ul_usec ;

	ul_tick = SYSF_tickGet() ;
	ul_usec = usecTimerGet() ;
	SYSF_taskDelay( 200 ) ;
	ul_tick = SYSF_tickGet() - ul_tick ;
	ul_usec = usecTimerGet() - ul_usec ;
	
	printf( "tick=%"PRIu32"[tick] %12.3f[msec]\n", ul_tick, ((double)(ul_tick*1000))/((double)SYSF_sysClkRateGet()) ) ;
	printf( "usec=%"PRIu32"[usec] %12.3f[msec]\n", ul_usec, ((double)ul_usec)/1000.0 ) ;
}
//VxWorksのチックカウンタとMV6446xのカウンタの継続時間を表示.
extern "C" void chkutimer()
{
	if ( vxusecTimerRunning == false ) {
		printf( "do start timer.\n" ) ;
		return ;
	}
	uint32_t ul_tick ;
	uint32_t ul_usec ;

	ul_tick = SYSF_tickGet() ;
	ul_usec = usecTimerGet() ;
	
	printf( "tick=%"PRIu32"[tick] %12.3f[msec]\n", ul_tick, ((double)(ul_tick*1000))/((double)SYSF_sysClkRateGet()) ) ;
	printf( "usec=%"PRIu32"[usec] %12.3f[msec]\n", ul_usec, ((double)ul_usec)/1000.0 ) ;
}

#define getTimerStatus() (vxusecTimerRunning)

#endif  // LINUX.
#endif  // TORNADO.
/*================================================================================================
［機能］	time_tをローカルタイム表示.
--------------------------------------------------------------------------------------------------
［戻値］	無し
［詳細］	
［ｳｫｰｸｽﾙｰ］	未実施
 [履歴]		2005/09/20	t.sasaki	Rhein1/2/3初版
			2019/07/16 Sky OP_BTS-4631 ログ出力タイミング修正の再検討.
================================================================================================*/
#ifdef TORNADO
extern "C" {
void dspLocalTime( time_t t );
void dspLocalTime_File( FILE *p_File, time_t t );
}
void dspLocalTime( time_t t )
{
	dspLocalTime_File( stdout, t );  // 既存動作は標準出力.
}

void dspLocalTime_File( FILE *p_File, time_t t )
{
	struct tm st_time ;
	struct tm* pst_time = &st_time ;
	
	if( NULL==p_File )
	{
		p_File = stdout; // ファイルポインタがNULLであれば標準出力に切り替える.
	}
	if ( t == 0 ) {	// 時間指定なしは現在時刻.
		t = time( NULL ) ;
		fprintf(p_File, "time_t=%zu\n", (uintptr_t)t) ;
	}
	SYSF_localtime_r( &t, pst_time );
	fprintf(p_File, "  D:%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d \n",
				pst_time->tm_year + 1900,
				pst_time->tm_mon+1,
				pst_time->tm_mday,
				pst_time->tm_hour,
				pst_time->tm_min,
				pst_time->tm_sec);
}
#endif //#ifdef TORNADO.

// ============================================================================.
// タスクモニター.
// ----------------------------------------------------------------------------.
#ifdef TORNADO

//!< タスクモニタ用プロトタイプ宣言.
static void THRL_DispTaskMonitor( uint32_t ul_ReqType );

//!< sysで管理しているタスク情報のリスト.
extern SYSS_TaskInfo* p_taskInfoList;

void StartTmB_SMP_Next(uint32_t ui_JobID);
void EndTmNest(uint32_t ui_JobID, char *uc_JobName);
void StopTm(void);

#define THRD_COM_DBG_INIT_JOB					(90040)		/* Job記録の初期化	*/
#define THRD_COM_DBG_START_JOB					(90041)		/* Job記録の開始	*/
#define THRD_COM_DBG_END_JOB					(90042)		/* Job記録の終了	*/

#define	THRL_JobNameMax							64			/* Job名の最大値	*/
static int32_t i_TmDepth;

typedef struct _THRL_JobArgs {
	uint32_t	ui_JobID;									/* JobID			*/
	char	sc_JobName[THRL_JobNameMax];					/* Jobネーム		*/
}THRL_JobArgs;



// ================================================================================================.
//! <!--機能--> タスクモニタ開始.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
// 2009/07/10	小倉	新規作成.
// ================================================================================================.
void StartTm()
{
	i_TmDepth=0;
	THRL_ReqTaskMonitor( THRD_COM_DBG_INIT_JOB, NULL);
	THRL_ReqTaskMonitor( THRD_COM_DBG_START_TM, NULL );
}
 	
// ================================================================================================.
//! <!--機能--> タスクモニタ開始 ２.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//!				タスクモニター改善.
//! 			タスク名記録型のタスクモニターを起動. StartTm と併用はできない.
//! <!--履歴-->.
// 2012/08/28	伊藤史	新規作成.
// ================================================================================================.
void StartTmB()
{
	i_TmDepth=0;
	THRL_ReqTaskMonitor( THRD_COM_DBG_INIT_JOB, NULL);
	THRL_ReqTaskMonitor( THRD_COM_DBG_START_TMB, NULL );
}

// ================================================================================================.
//! <!--機能--> タスクモニタ開始 3.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//!				タスクモニター改善.
//! 			SMP用のタスクモニターを起動. 
//!			 	他のタスクモニタと併用はできない.
//! <!--履歴-->.
// 2014/8/21 	稲垣大樹        新規作成.
// ================================================================================================.
void StartTm_SMP()
{
	i_TmDepth=0;
	THRL_ReqTaskMonitor( THRD_COM_DBG_INIT_JOB, NULL);
	THRL_ReqTaskMonitor( THRD_COM_DBG_START_TM_SMP, NULL );
}

// ================================================================================================.
//! <!--機能--> タスクモニタ開始 4.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//!				タスクモニター改善.
//! 			SMP用のタスク名記録型タスクモニターを起動. 
//!				他のタスクモニタと併用はできない.
//! <!--履歴-->.
// 2014/8/21 	稲垣大樹        新規作成.
// ================================================================================================.
void StartTmB_SMP()
{
	i_TmDepth=0;
	THRL_ReqTaskMonitor( THRD_COM_DBG_INIT_JOB, NULL);
	THRL_ReqTaskMonitor( THRD_COM_DBG_START_TMB_SMP, NULL );
}

// ================================================================================================.
//! <!--機能--> タスクモニタ開始 4（Jobネスト対応版).
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param uint32_t ui_JobID		JobID.\n.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
// 2016/04/27	Ads長谷川	新規作成.
// ================================================================================================.
void StartTmB_SMP_Nest(uint32_t ui_JobID)
{
	ulong ul_JobID = ui_JobID;
	if( i_TmDepth == 0 ){
		THRL_ReqTaskMonitor( THRD_COM_DBG_INIT_JOB, NULL);
		THRL_ReqTaskMonitor( THRD_COM_DBG_START_TMB_SMP, NULL );
	}
	THRL_ReqTaskMonitor( THRD_COM_DBG_START_JOB, (ulong*)ul_JobID );
	i_TmDepth++;
}

// ================================================================================================.
//! <!--機能--> タスクモニタ終了 4（Jobネスト対応版).
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param uint32_t  ui_JobID		JobID.\n.
//! \param char* uc_JobName		Jobネーム.\n.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
// 2016/04/27	Ads長谷川	新規作成.
// ================================================================================================.
void EndTmNest(uint32_t ui_JobID, char *uc_JobName)
{
	THRL_JobArgs st_param;

	if( i_TmDepth > 0 ){
		--i_TmDepth;
		/* JOB名を設定する */
		st_param.ui_JobID = ui_JobID;
		if( uc_JobName == NULL ){			/* Job名が無い場合はJob+JobIDとする(cushell対応) */
			sprintf(st_param.sc_JobName, "Job%d", ui_JobID );
		}else{
			strncpy((char*)st_param.sc_JobName, (const char*)uc_JobName, THRL_JobNameMax );
		}
		THRL_ReqTaskMonitor( THRD_COM_DBG_END_JOB, (void*)&st_param );
	}
	if( i_TmDepth == 0 ){
		StopTm();
	}
}

// ================================================================================================.
//! <!--機能--> タスクモニタ開始 5.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//!				タスクモニター改善(イベント拡張).
//! 			タスク名記録型のタスクモニターを起動. StartTm/StartTmB と併用はできない.
//! <!--履歴-->.
// 2016/09/21	ishiga 	新規作成.
// ================================================================================================.
void StartTmE()
{
	i_TmDepth=0;
	THRL_ReqTaskMonitor( THRD_COM_DBG_INIT_JOB, NULL);
	THRL_ReqTaskMonitor( THRD_COM_DBG_START_TME, NULL );
}

// ================================================================================================.
//! <!--機能--> タスクモニタ開始 6.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//!				タスクモニター改善(イベント拡張).
//!             SMP用のタスク名記録型のタスクモニターを起動
//! 			StartTm/StartTmB と併用はできない.
//! <!--履歴-->.
// 2016/10/16	酒井純 	新規作成.
// ================================================================================================.
void StartTmE_SMP()
{
	i_TmDepth=0;
	THRL_ReqTaskMonitor( THRD_COM_DBG_INIT_JOB, NULL);
	THRL_ReqTaskMonitor( THRD_COM_DBG_START_TME_SMP, NULL );
}

// ================================================================================================.
//! <!--機能--> タスクモニタ停止.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
// 2009/07/10	小倉	新規作成.
// 2016/09/21	Sky千葉 【Icarus水平展開】StartTmおよびStartTm_SMPを現在は使用しないため、処理を削除.
// ================================================================================================.
void StopTm()
{
#if 1
	i_TmDepth=0;
	THRL_ReqTaskMonitor( THRD_COM_DBG_STOP_TM, NULL );
#else
	//! PID/タスク名のリストを生成する.
	THRS_TaskNameList s_TaskNameList[ THRD_MaxTaskNumProcess ];
	memset( s_TaskNameList, 0, sizeof( s_TaskNameList ) );

	int32_t list_cnt = 0;
	int32_t task_cnt;
	THR_ID  task_pid = SYSF_taskIdSelf();
	SYSS_TaskInfo* taskinfo = SYSF_taskInfoGet(DEF_OSWTM_INFOTYP_PID, (void*)task_pid, &task_cnt);
	if( taskinfo != NULL ){
		for( long l_Cnt = 0; l_Cnt < task_cnt; l_Cnt++ ){
			if( list_cnt >= THRD_MaxTaskNumProcess )
				break;
			if( taskinfo[l_Cnt].taskStatus != DEF_OSWTM_TS_NOWAKE ){
				s_TaskNameList[list_cnt].si_PID = taskinfo[l_Cnt].pid;
				memcpy( s_TaskNameList[list_cnt].sc_TaskName, taskinfo[l_Cnt].taskName,  THRD_TaskNameLen );
				list_cnt++;
			}
		}
		SYSF_taskInfoRelease(taskinfo);
	}

	i_TmDepth=0;
	THRL_ReqTaskMonitor( THRD_COM_DBG_STOP_TM, s_TaskNameList );
#endif
}

// ================================================================================================.
//! <!--機能--> タスクモニタ初期化.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
// 2009/07/10	小倉	新規作成.
// ================================================================================================.
void InitTm()
{
	i_TmDepth=0;
	THRL_ReqTaskMonitor( THRD_COM_DBG_INIT_JOB, NULL);
	THRL_ReqTaskMonitor( THRD_COM_DBG_INIT_TM, NULL );
}

// ================================================================================================.
//! <!--機能--> スナップショット.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
// 2009/07/10	小倉	新規作成.
// ================================================================================================.
void SnapTm()
{
	THRL_ReqTaskMonitor( THRD_COM_DBG_SNAP_TM, NULL );
}

// ================================================================================================.
//! <!--機能--> スナップショット.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param uint32_t ul_DspType		表示種別.\n.
//! 							1:タスクモニタ一覧表示.\n.
//! 							2:タスクモニタ累積表示.\n.
//! 							3:スナップショット一覧表示.\n.
//! 							4:スナップショット累積表示.\n.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
// 2009/07/10	小倉	新規作成.
// ================================================================================================.
void DspTm( uint32_t ul_DspType )
{
	uint32_t ul_ReqType = 0;
	
	printf("========== Thread ==========\n");
	printf("snap shot\n");
	printf("========== Thread ==========\n");

	switch( ul_DspType ) {
	case 1:
		ul_ReqType = THRD_COM_DBG_DISP_TM;
		break;
	case 2:
		ul_ReqType = THRD_COM_DBG_DISP_EX_TM;
		break;
	case 3:
		ul_ReqType = THRD_COM_DBG_DISP_SNAP_TM;
		break;
	case 4:
		ul_ReqType = THRD_COM_DBG_DISP_EX_SNAP_TM;
		break;
	default:
		return;
	}
	
	THRL_DispTaskMonitor( ul_ReqType );
}


// ================================================================================================.
//! <!--機能--> 表示要求（ローカル）.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param uint32_t ul_ReqType		要求種別.\n.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
// 2009/07/10	小倉	新規作成.
// ================================================================================================.
void THRL_DispTaskMonitor( uint32_t ul_ReqType )
{
	i_TmDepth=0;
	THRL_ReqTaskMonitor( ul_ReqType, NULL );
}
	
// ================================================================================================.
//! <!--機能--> タスクモニタファイル化コマンド.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.  psc_filenm : 保存ファイル名
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! 			HDDに簡易にタスクモニターログを作成するためのターミナルコマンド.
//! <!--履歴-->.
// 2012/09/12	伊藤史	新規作成.
// 2018/03/22 Sky井柳 [Eagle]制御構成変更No97「ストレージ装着設定変更対応」 ata01(HDD)→ata0a(SSDorμSD)に変更.
// ================================================================================================.
void SaveTm( char *psc_filenm )
{
	char	sc_cmd[300];
	memset(sc_cmd,0,sizeof(sc_cmd));
	strcpy(sc_cmd,"cat /proc/kmtm/info >/ata0a/Capture/");
	if( NULL == psc_filenm || 0 == *psc_filenm ){
		strcat(sc_cmd,"taskmonitor_dsptm1.log");
	} else {
		strncat(sc_cmd,psc_filenm,256);
	}
	printf("%s\n",sc_cmd);
	system(sc_cmd);
}

// ================================================================================================.
//! <!--機能--> タスクモニタ取得コマンド.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.  なし.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! ストレージにタスクモニターログを保存し、読みだしてログ出力する.
//! ※カーネル側のタスクモニターログが出力されない場合に使用する想定.
//! <!--履歴-->.
//! 2020/06/04 Sky 新規作成 .
// ================================================================================================.
void ShowTmApp()
{
	#define THRD_SaveTaskmonitorDir			"/ata0a/Capture"										/* タスクモニタを保存するパス */
	#define THRD_SaveTaskmonitorDsptmTMP	THRD_SaveTaskmonitorDir"/taskmonitor_dsptm1_tmp.log"	/* 保存するタスクモニタログ */
	#define THRD_TaskmonitorStrMAX			256														/* タスクモニタの1行の文字列最大長 */
	
	FILE *pf_TmFile = NULL;
	const char *psc_LogPath = THRD_SaveTaskmonitorDsptmTMP;	
	char sc_Readline[THRD_TaskmonitorStrMAX] = {0};
	char *psc_Result = NULL;
	char sc_cmd[100] = {0};

	/* ディレクトリがない場合は作成しておく */
	if ( 0 != access( THRD_SaveTaskmonitorDir, F_OK ) ) {
		SYSF_mkdir( THRD_SaveTaskmonitorDir );
	}
	
	/* タスクモニタをストレージに保存 */
	printf("cat /proc/kmtm/info > %s\n", psc_LogPath);
	sprintf( sc_cmd, "cat /proc/kmtm/info > %s", psc_LogPath );
	system(sc_cmd);

	/* 保存したタスクモニタを読み出し */
	pf_TmFile = SYSF_fopen( psc_LogPath, "r" );
	if( NULL == pf_TmFile ) {
		printf("[%s]Dose not exist %s(errno = %d)\n", __FUNCTION__,  psc_LogPath, errno);
	
		/* 保存したタスクモニタを削除 */
		sprintf( sc_cmd, "rm %s", THRD_SaveTaskmonitorDsptmTMP );
		system( sc_cmd );

		sync();
		sync();
		sync();
		return;
	}

	/* 1行ずつログ出力 */
	while ( 1 ) {
		psc_Result = SYSF_Fgets( sc_Readline, THRD_TaskmonitorStrMAX, pf_TmFile);
		if( NULL == psc_Result ) {
			break;
		}
		printf("%s", sc_Readline);
	}

	SYSF_fclose(pf_TmFile);

	/* 保存したタスクモニタを削除 */
	sprintf( sc_cmd, "rm %s", THRD_SaveTaskmonitorDsptmTMP );
	system( sc_cmd );

	sync();
	sync();
	sync();
}

#define startTimer() startusecTimer()
#define U2M(a) (((double)(a))/1000.0)		// usec→msec.

// ============================================ //.
// HDDモニター(簡易版)							//.
// BIOS内でのHDD転送開始～完了をモニターする	//.
// ============================================ //.
// 2008/07/23 t.yokoyama [CQ308783]ログテーブルにR/Wを追加.
struct ST_HDDATBL{
	int32_t		nTaskID;	// タスクID.
	uint32_t	ul_SecNo;	// セクターNo.
	uint32_t	ul_Size;	// サイズ.
	uint32_t	ul_STick;	// 開始時Tick.
	uint32_t	ul_Time;	// 処理時間(usec).
	uchar	uc_Flags;	// b0:0=read,1:write.
//	→ 静的解析対応(CIM23887) 2011/05/23 saka.
	ST_HDDATBL():nTaskID(0),ul_SecNo(0),ul_Size(0),ul_STick(0),ul_Time(0),uc_Flags(0){}
//	← 静的解析対応.
} ;
#ifdef DEF_A3A4COMM
struct ST_HDDATBLEX{
	int32_t		nTaskID;	// タスクID.
	uint32_t	ul_SecNo;	// セクターNo.
	uint32_t	ul_Size;	// サイズ.
	uint32_t	ul_STick;	// 開始時Tick.
	struct timeval st_TimeVal;		// 開始.
	uchar	uc_Flags;	// b0:0=read,1:write.
	uchar	uc_type;	// ログ種別.
//	→ 静的解析対応(CIM23887) 2011/05/23 saka.
	ST_HDDATBLEX():nTaskID(0),ul_SecNo(0),ul_Size(0),ul_STick(0),uc_Flags(0),uc_type(0){
		// 2016/06/23 Klocwork #58648 #58654 対応.
		st_TimeVal.tv_sec	= 0;
		st_TimeVal.tv_usec	= 0;
	}
//	← 静的解析対応.

	// 2016/06/23 Klocwork #58631 対応.
	void copy(ST_HDDATBLEX &data) {
		nTaskID		= data.nTaskID;
		ul_SecNo	= data.ul_SecNo;
		ul_Size		= data.ul_Size;
		ul_STick	= data.ul_STick;
		st_TimeVal.tv_sec	= data.st_TimeVal.tv_sec;
		st_TimeVal.tv_usec	= data.st_TimeVal.tv_usec;
		uc_Flags	= data.uc_Flags;
		uc_type		= data.uc_type;
	}
} ;

ST_HDDATBLEX* pst_HddATbl = NULL ;
#else
ST_HDDATBL* pst_HddATbl = NULL ;
#endif	// #ifdef DEF_A3A4COMM.

static bool  bl_HddA_Running = false ;
static bool  bl_HddA_Loop    = false ;
static uint32_t ul_HddA_Index   = 0 ;
static uint32_t ul_HddA_MaxNum   = 0 ;
static ST_HDDATBL sst_hddatmp ;			// 一時保存用.

// 2006/11/13 伊藤史：FATAL時HDDログ対応。FATAL発生時に停止.
// 2008/07/23 t.yokoyama [CQ308783]ログを取り込む件数を拡大.
#define		HDDAFATALTBLMAX		512
static ST_HDDATBL	st_HddaTblFatal[HDDAFATALTBLMAX];
static bool  bl_HddAFatal_Stop    = false;
static bool  bl_HddAFatal_Loop    = false ;
static uint32_t ul_HddAFatal_Index   = 0 ;
static short ss_HddAFatal_StartIndex   = -1 ;	// 多重Startチェック.

/*================================================================================================
［機能］	HDD転送の遅延時間ログ取得関連宣言.
--------------------------------------------------------------------------------------------------
 [履歴]
	2013/10/23 s.iyanagi	 新規作成. 
================================================================================================*/
#define DEF_HDDTRANSDELAYTBL_MAX (100)		// HDD転送遅延情報数.
#define DEF_HDDTRANSAVERAGECOUNT (100)		// 転送平均対象数.
struct ST_HDDTRANSDELAYTBL{					// HDD転送遅延情報.
	int32_t		nTaskID;	// タスクID.
	uint32_t	ul_SecNo;	// セクターNo.
	uint32_t	ul_SectNum;	// セクター数.
	uint32_t	ul_STick;	// 開始時Tick.
	uint32_t	ul_Time;	// 処理時間(usec).
	uchar	uc_Flags;	// b0:0=read,1:write.
	double	dl_Speed;	// 転送速度(KB/msec).
	double	dl_SpeedAve;// 平均転送速度(KB/msec).
	ST_HDDTRANSDELAYTBL():nTaskID(0),ul_SecNo(0),ul_SectNum(0),ul_STick(0),ul_Time(0),uc_Flags(0),dl_Speed(0.0),dl_SpeedAve(0.0){}
};
static ST_HDDTRANSDELAYTBL	st_HddaTblTransDelay[ DEF_HDDTRANSDELAYTBL_MAX ];	// 転送遅延情報保存領域.
static uint32_t				ul_HddDelay_Index   = 0 ;							// 転送遅延情報インデックス.
static bool					bl_HddDelay_Loop    = false ;						// 転送遅延情報保存領域のループチェックフラグ.
static ST_HDDTRANSDELAYTBL	st_HddDelayTmp ;									// 転送遅延情報一時保存用.
double						dl_TransDelayCheckSpeed = 30.0;						// 転送遅延判定スピード(KB/msec).
double						dl_TransDelayAveCheckSpeed = 35.0;					// 平均転送遅延判定スピード(KB/msec).
static double	dl_HddReadTransSpeedSum = 0.0;									// 読み込み転送平均対象数あたりの転送速度合計値.
static double	dl_HddReadTransSpeedData[ DEF_HDDTRANSAVERAGECOUNT];			// 読み込み転送平均対象数分の転送速度値.
static uchar	uc_HddReadTransSpeedDataNo = 0;									// 読み込み転送平均対象データ番号.
static bool		bl_HddReadTransSpeedDataMaxUp = false;							// 読み込み転送平均対象データループチェックフラグ.
static double	dl_HddWriteTransSpeedSum = 0.0;									// 書き込み転送平均対象数あたりの転送速度合計値.
static double	dl_HddWriteTransSpeedData[ DEF_HDDTRANSAVERAGECOUNT];			// 書き込み転送平均対象数分の転送速度値.
static uchar	uc_HddWriteTransSpeedDataNo = 0;								// 書き込み転送平均対象データ番号.
static bool		bl_HddWriteTransSpeedDataMaxUp = false;							// 書き込み転送平均対象データループチェックフラグ.

/*================================================================================================
［機能］	HDD転送遅延情報設定関数.
--------------------------------------------------------------------------------------------------
［引数］	ST_HDDTRANSDELAYTBL	*pst_HddaTblTmp	：遅延時のHDDデータ.
［戻値］	なし.
［詳細］	HDDの転送時間が遅延している場合にその転送時間を設定する.
［ｳｫｰｸｽﾙｰ］	未実施.
--------------------------------------------------------------------------------------------------
 [履歴]
	2013/10/23 s.iyanagi	 新規作成. 
================================================================================================*/
void setHddTransDelay( ST_HDDTRANSDELAYTBL *pst_HddaTblTmp )
{
	st_HddaTblTransDelay[ul_HddDelay_Index].nTaskID		= pst_HddaTblTmp->nTaskID ;		// タスクID.
	st_HddaTblTransDelay[ul_HddDelay_Index].ul_SecNo	= pst_HddaTblTmp->ul_SecNo ;	// セクターNo.
	st_HddaTblTransDelay[ul_HddDelay_Index].ul_SectNum	= pst_HddaTblTmp->ul_SectNum ;	// セクター数.
	st_HddaTblTransDelay[ul_HddDelay_Index].ul_STick	= pst_HddaTblTmp->ul_STick ;	// 開始時Tick.
	st_HddaTblTransDelay[ul_HddDelay_Index].ul_Time		= pst_HddaTblTmp->ul_Time ;		// 終了時Tick.
	st_HddaTblTransDelay[ul_HddDelay_Index].uc_Flags	= pst_HddaTblTmp->uc_Flags;		// 0=read,1=write.
	st_HddaTblTransDelay[ul_HddDelay_Index].dl_Speed	= pst_HddaTblTmp->dl_Speed;		// 転送速度(KB/msec).
	st_HddaTblTransDelay[ul_HddDelay_Index].dl_SpeedAve	= pst_HddaTblTmp->dl_SpeedAve;	// 平均転送速度(KB/msec).
	ul_HddDelay_Index++;

	if( ul_HddDelay_Index >= DEF_HDDTRANSDELAYTBL_MAX){
		ul_HddDelay_Index = 0;
		bl_HddDelay_Loop = true;
	}
}

/*================================================================================================
［機能］	HDD転送の遅延確認開始処理関数.
--------------------------------------------------------------------------------------------------
［引数］	uint32_t ul_SecNo		：セクターNo.
			uint32_t ul_SectorNum	：セクター数.
			uchar uc_Flags		：0=read,1=write.
［戻値］	なし.
［詳細］	HDDの転送時間が遅延確認のための開始処理(開始時間登録)をする.
［ｳｫｰｸｽﾙｰ］	未実施.
--------------------------------------------------------------------------------------------------
 [履歴]
	2013/10/23 s.iyanagi	 新規作成. 
================================================================================================*/
void setHddTransDelayStart(
	uint32_t ul_SecNo,		// セクターNo.
	uint32_t ul_SectorNum,	// セクター数.
	uchar uc_Flags )	// 0=read,1=write.
{
	// 一時記憶させる.
	st_HddDelayTmp.nTaskID		= SYSF_taskIdSelf() ;
	st_HddDelayTmp.ul_STick		= SYSF_tickGet() ;
	st_HddDelayTmp.ul_SecNo		= ul_SecNo ;
	st_HddDelayTmp.ul_SectNum	= ul_SectorNum ;
	st_HddDelayTmp.ul_Time		= usecTimerGet() ;	// 現在のusecタイマー値保持.
	st_HddDelayTmp.uc_Flags		= uc_Flags;			// 0=read,1=write.
	st_HddDelayTmp.dl_Speed		= 0.0;				// 転送速度(KB/msec).
	st_HddDelayTmp.dl_SpeedAve	= 0.0;				// 平均転送速度(KB/msec).
}

/*================================================================================================
［機能］	HDD転送速度平均計算処理関数.
--------------------------------------------------------------------------------------------------
［引数］	uchar	uc_Flags	：0=read,1=write.
			double	dl_Speed	：転送速度(KB/msec).
［戻値］	なし.
［詳細］	近々のHDD転送平均対象数分の平均転送時間を種別ごと(読み込み・書き込み)に算出する.
［ｳｫｰｸｽﾙｰ］	未実施.
--------------------------------------------------------------------------------------------------
 [履歴]
	2013/10/23 s.iyanagi	 新規作成. 
================================================================================================*/
double calcHddTransSpeed( uchar	uc_Flags, 	double dl_Speed ){
	
	uchar uc_Idx;
	double dl_SpeedAve = 0.0;
	double dl_SpeedSum = 1.0;
	uchar uc_DataCnt = 1;
	
	if( uc_Flags == 0 ){	// Read.
		uc_Idx = uc_HddReadTransSpeedDataNo;
		if ( bl_HddReadTransSpeedDataMaxUp == false ) {
			dl_HddReadTransSpeedData[ uc_Idx ] = dl_Speed;						// データ設定.
			dl_HddReadTransSpeedSum += dl_Speed;								// 加算.
			uc_HddReadTransSpeedDataNo++;
			uc_DataCnt = uc_HddReadTransSpeedDataNo;
			if( uc_HddReadTransSpeedDataNo >= DEF_HDDTRANSAVERAGECOUNT ){
				bl_HddReadTransSpeedDataMaxUp = true;
				uc_HddReadTransSpeedDataNo = 0;
			}
		} else {
			dl_HddReadTransSpeedSum -= dl_HddReadTransSpeedData[ uc_Idx ];		// 減算.
			dl_HddReadTransSpeedData[ uc_Idx ] = dl_Speed;						// データ設定.
			dl_HddReadTransSpeedSum += dl_Speed;								// 加算.
			uc_DataCnt = DEF_HDDTRANSAVERAGECOUNT;
		}
		dl_SpeedSum = dl_HddReadTransSpeedSum;
	} else {							// Write.
		uc_Idx = uc_HddWriteTransSpeedDataNo;
		if ( bl_HddWriteTransSpeedDataMaxUp == false ) {
			dl_HddWriteTransSpeedData[ uc_Idx ] = dl_Speed;						// データ設定.
			dl_HddWriteTransSpeedSum += dl_Speed;								// 加算.
			uc_HddWriteTransSpeedDataNo++;
			uc_DataCnt = uc_HddWriteTransSpeedDataNo;
			if( uc_HddWriteTransSpeedDataNo >= DEF_HDDTRANSAVERAGECOUNT ){
				bl_HddWriteTransSpeedDataMaxUp = true;
				uc_HddWriteTransSpeedDataNo = 0;
			}
		} else {
			dl_HddWriteTransSpeedSum -= dl_HddWriteTransSpeedData[ uc_Idx ];	// 減算.
			dl_HddWriteTransSpeedData[ uc_Idx ] = dl_Speed;						// データ設定.
			dl_HddWriteTransSpeedSum += dl_Speed;								// 加算.
			uc_DataCnt = DEF_HDDTRANSAVERAGECOUNT;
		}
		dl_SpeedSum = dl_HddWriteTransSpeedSum;
	}
	dl_SpeedAve = dl_SpeedSum / (double)uc_DataCnt;
	return dl_SpeedAve;
}


/*================================================================================================
［機能］	HDD転送の遅延確認終了処理関数.
--------------------------------------------------------------------------------------------------
［引数］	なし.
［戻値］	なし.
［詳細］	HDDの転送時間を算出し、一定速度以下の場合、転送遅延として処理を登録する.
［ｳｫｰｸｽﾙｰ］	未実施.
--------------------------------------------------------------------------------------------------
 [履歴]
	2013/10/23 s.iyanagi	 新規作成. 
================================================================================================*/
void setHddTransDelayEnd()
{
	uint32_t ul_ProcTime = usecTimerGet() - st_HddDelayTmp.ul_Time;							// 処理時間(usec).
	// 0除算チェック.
	if( ul_ProcTime != 0 ){
		double dl_ProcSize = (double)DEF_SECTOR_SIZE * (double)st_HddDelayTmp.ul_SectNum;	// 処理サイズ(Byte).
		double dl_ProcSpeed = dl_ProcSize / 1024.0 / (double)ul_ProcTime * 1000.0;			// 転送速度(KB/msec).
		double di_ProcSpeedAve = calcHddTransSpeed( st_HddDelayTmp.uc_Flags, dl_ProcSpeed );

		if( ( dl_ProcSpeed < dl_TransDelayCheckSpeed )&&				// 転送遅延判定スピード以下.
			( di_ProcSpeedAve < dl_TransDelayAveCheckSpeed )			// 平均転送遅延判定スピード以下.
			){
			ST_HDDTRANSDELAYTBL st_ResistData;
			st_ResistData.nTaskID		= st_HddDelayTmp.nTaskID ;		// タスクID.
			st_ResistData.ul_SecNo		= st_HddDelayTmp.ul_SecNo ;		// セクターNo.
			st_ResistData.ul_SectNum	= st_HddDelayTmp.ul_SectNum;	// セクター数.
			st_ResistData.ul_STick		= st_HddDelayTmp.ul_STick ;		// 開始時Tick.
			st_ResistData.ul_Time		= ul_ProcTime ;					// 処理時間(usec).
			st_ResistData.uc_Flags		= st_HddDelayTmp.uc_Flags;		// 0=read,1=write.
			st_ResistData.dl_Speed		= dl_ProcSpeed;					// 転送速度(KB/msec).
			st_ResistData.dl_SpeedAve	= di_ProcSpeedAve;				// 平均転送速度(KB/msec).
			setHddTransDelay( &st_ResistData );
		}
	}
}

/*================================================================================================
［機能］	HDD転送遅延チェック速度設定関数.
--------------------------------------------------------------------------------------------------
［引数］	uint32_t	ul_Speed	：転送速度(KB/msec).
			uint32_t	ul_AveSpeed	：平均転送速度(KB/msec).
［戻値］	なし.
［詳細］	HDD転送遅延のチェック速度を設定する.
［ｳｫｰｸｽﾙｰ］	未実施.
--------------------------------------------------------------------------------------------------
 [履歴]
	2013/10/23 s.iyanagi	 新規作成. 
================================================================================================*/
void setTransDlyChkSpd( uint32_t ul_Speed , uint32_t ul_AveSpeed ){
	dl_TransDelayCheckSpeed = (double)ul_Speed;
	dl_TransDelayAveCheckSpeed = (double)ul_AveSpeed;
	printf( "set Trans Delay Value = %5.1f(Ave[%5.1f]) KB/ms\n", dl_TransDelayCheckSpeed, dl_TransDelayAveCheckSpeed ) ;
}

/*================================================================================================
［機能］	HDD転送の遅延時ログを出力表示関数.
--------------------------------------------------------------------------------------------------
［引数］	なし.
［戻値］	なし.
［詳細］	HDDの転送時間が遅延している場合に取得した時間を出力する.
［ｳｫｰｸｽﾙｰ］	未実施.
--------------------------------------------------------------------------------------------------
 [履歴]
	2013/10/23 s.iyanagi	 新規作成. 
================================================================================================*/
void dspHddTransDelay()
{
	register uint32_t ul_Max = ul_HddDelay_Index ;		// 登録最終位置を設定.
	register uint32_t ul_Idx = 0;

	
	if ( bl_HddDelay_Loop == true ) {
		ul_Max = DEF_HDDTRANSDELAYTBL_MAX ;			// 最大値を設定.
		ul_Idx = ul_HddDelay_Index;
	}

	printf( "HDD Trans Delay( %5.1f(Ave[%5.1f]) KB/ms Over ) Log \n", dl_TransDelayCheckSpeed, dl_TransDelayAveCheckSpeed ) ;
	printf( "(  ST[tick]) No.   TID        R/W   SecNo    Size[KB] Prc[usec] Spd[KB/ms]  Ave[KB/ms] Name \n" ) ;
	printf( "---------- ----- ----------  -  ---------- -------- --------- ---------- ---------- ---------------\n" ) ;
	//       uuuuu 0xXXXXXXXX uuuuuuuu  c  uuuuuuuuuu fffff.f  uuuuuuuu   fffff.f    fffff.f    NNN...NNN.
	for ( register uint32_t idx=0; idx < ul_Max; idx++ ) {
#ifdef DEF_A3A4COMM
		printf( "(%8lu) %5lu 0x%.8x  %c  %10lu %6.1f  %8lu   %5.1f    %5.1f    %s\n",
#else	// #ifdef DEF_A3A4COMM.
		printf( "(%8"PRIu32") %5"PRIu32" 0x%.8"PRIx32"  %c  %10"PRIu32" %5.1f  %8"PRIu32"   %5.1f    %5.1f    %s\n",
#endif	// #ifdef DEF_A3A4COMM.
		st_HddaTblTransDelay[ul_Idx].ul_STick, idx +1, st_HddaTblTransDelay[ul_Idx].nTaskID 
		,	( st_HddaTblTransDelay[ul_Idx].uc_Flags == HDDA_WRITE )? 'W':'R'
		,	st_HddaTblTransDelay[ul_Idx].ul_SecNo
		,	( (double)DEF_SECTOR_SIZE * (double)st_HddaTblTransDelay[ul_Idx].ul_SectNum / 1024.0) 
		,	st_HddaTblTransDelay[ul_Idx].ul_Time
		,	st_HddaTblTransDelay[ul_Idx].dl_Speed
		,	st_HddaTblTransDelay[ul_Idx].dl_SpeedAve
		,	SYSF_taskName(st_HddaTblTransDelay[ul_Idx].nTaskID)
			);
		//アドレスのバッファオーバー処理.
		ul_Idx = ((ul_Idx + 1) % DEF_HDDTRANSDELAYTBL_MAX);				//位置の算出（カウントアップ）.
	}
	printf( "---------------------------------------------------------------------------------------\n" ) ;
}
// ============================================ //.
//ＨＤＤモニタ開始.
// ============================================ //.
extern "C" void starthdda( int32_t nNum )
{
#ifdef DEF_A3A4COMM
	int32_t	si_num = 0;
#endif	// #ifdef DEF_A3A4COMM.
	if ( nNum == 0 ) {
#ifdef DEF_A3A4COMM
		nNum = 40;										// デフォルトサンプリング数(sizeof(ST_HDDSTBL)×nNum分のメモリ確保).
#else	// #ifdef DEF_A3A4COMM.
		nNum = 40000 ;										// デフォルトサンプリング数(sizeof(ST_HDDSTBL)×nNum分のメモリ確保).
#endif	// #ifdef DEF_A3A4COMM.
	}
#ifdef DEF_A3A4COMM
	si_num = nNum * 1024;
#endif	// #ifdef DEF_A3A4COMM.
	if ( pst_HddATbl != NULL ) {
		// 確保済みリソースの解放.
		ul_HddA_Index = 0 ;
		ul_HddA_MaxNum = 0 ;
		bl_HddA_Loop = false ;
		delete[] pst_HddATbl ;
	}
#ifdef DEF_A3A4COMM
	pst_HddATbl = new ST_HDDATBLEX[si_num] ;					//領域の確保.
#else	// #ifdef DEF_A3A4COMM.
	pst_HddATbl = new ST_HDDATBL[nNum] ;					//領域の確保.
#endif	// #ifdef DEF_A3A4COMM.
	if ( pst_HddATbl == NULL ) {
		//領域が確保できなかった場合.
		printf("error(malloc)\n");
		return ;
	}
#ifdef DEF_A3A4COMM
	ul_HddA_MaxNum = (uint32_t)si_num ;							//最大モニタ数の設定.

	printf("starthdda ul_HddA_MaxNum=%lu size=%lu\n",si_num, si_num*sizeof(ST_HDDATBLEX) );
#else	// #ifdef DEF_A3A4COMM.
	ul_HddA_MaxNum = (uint32_t)nNum ;							//最大モニタ数の設定.
#endif	// #ifdef DEF_A3A4COMM.
	// usecタイマー開始.
	if ( getTimerStatus() == false ) {
		//μsecタイマが開始していない場合.
		startTimer() ;										//μsecタイマー開始.
	}
	bl_HddA_Running = true ;								// HDDモニタ開始.
}
// ============================================ //.
//ＨＤＤモニタ停止.
// ============================================ //.
extern "C" void stophdda( int32_t delflg )
{
	bl_HddA_Running = false ;								// HDDモニタ停止.
	if ( delflg == 1 ) {
		// 確保済みリソースの解放.
		if ( pst_HddATbl != NULL ) {
			ul_HddA_Index = 0 ;
			ul_HddA_MaxNum = 0 ;
			bl_HddA_Loop = false ;
			delete[] pst_HddATbl ;
		}
	}
}
// ============================================ //.
//モニタデータ表示.
// ============================================ //.
extern "C" void dsphdda()
{
	if ( pst_HddATbl == NULL ) {
		//領域が確保してない場合.
#ifdef DEF_A3A4COMM
		printf("\nnot start hdda\n");
#else	// #ifdef DEF_A3A4COMM.
		printf("error(NULL)\n");
#endif	// #ifdef DEF_A3A4COMM.
		return ;
	}
#ifdef DEF_A3A4COMM
	ST_HDDATBLEX* pst = NULL ;
#else	// #ifdef DEF_A3A4COMM.
	ST_HDDATBL* pst = NULL ;
#endif	// #ifdef DEF_A3A4COMM.
	uint32_t ul_NowIndex = 0 ;
	bool bl_Loop = false ;										// ??意味不明.
	
	if ( bl_HddA_Running == true ) {
		// モニターが停止していない場合はduplicateする.
#ifdef DEF_A3A4COMM
		pst = new ST_HDDATBLEX[ul_HddA_MaxNum] ;					// 領域の確保.
#else	// #ifdef DEF_A3A4COMM.
		pst = new ST_HDDATBL[ul_HddA_MaxNum] ;					// 領域の確保.
#endif	// #ifdef DEF_A3A4COMM.

		if ( pst == NULL ) {
			//領域が確保できなかった場合.
			printf("error(malloc)\n");
			return ;
		}
		// Start タスクロック ----------------------.
		SYSF_taskLock();
#ifdef DEF_A3A4COMM
		/* 2016/06/23 Klocwork #58631 対応 */
		for (int loop=0; loop<ul_HddA_MaxNum; loop++) {
			pst[loop].copy(pst_HddATbl[loop]);									// 領域へデータの設定.
		}
/*
		memcpy( pst, pst_HddATbl, sizeof(ST_HDDATBLEX)*ul_HddA_MaxNum ) ;		// 領域へデータの設定.
*/
#else	// #ifdef DEF_A3A4COMM.
		memcpy( pst, pst_HddATbl, sizeof(ST_HDDATBL)*ul_HddA_MaxNum ) ;		// 領域へデータの設定.
#endif	// #ifdef DEF_A3A4COMM.
		ul_NowIndex = ul_HddA_Index ;							// 登録最終位置の設定.
		bl_Loop = bl_HddA_Loop ;								// ??.
		SYSF_taskUnlock();
		// End   タスクロック ----------------------.
	} else {
		//タスクモニタが停止している場合.
		pst = pst_HddATbl ;
		ul_NowIndex = ul_HddA_Index ;
		bl_Loop = bl_HddA_Loop ;								// ??.
	}

#ifdef DEF_A3A4COMM
	ST_HDDATBLEX* p = &pst[0] ;
#else	// #ifdef DEF_A3A4COMM.
	ST_HDDATBL* p = &pst[0] ;
#endif	// #ifdef DEF_A3A4COMM.
	uint32_t ul_Max = ul_NowIndex ;								// 登録最終位置を設定.
	if ( bl_HddA_Loop == true ) {
		p = &pst[ul_NowIndex] ;									// 現在のインデックス部分が一番古い.
		ul_Max = ul_HddA_MaxNum ;								// 最大値を設定.
	}
#ifdef DEF_A3A4COMM
	printf( "\n@@@ HDD MONITOR @@@\n" ) ;
	printf( "sampling num=%lu[p] now=%lu[tick] loop=%d\n", ul_Max, SYSF_tickGet(),bl_HddA_Loop ) ;
	// 2008/07/23 t.yokoyama [CQ308783]ログフォーマットにR/Wを追加.
#else	// #ifdef DEF_A3A4COMM.
	printf( "@@@ HDD MONITOR @@@\n" ) ;
	printf( "sampling num=%"PRIu32"[p] now=%"PRIu32"[tick]\n", ul_Max, SYSF_tickGet() ) ;
#endif	// #ifdef DEF_A3A4COMM.
	// 2008/07/23 t.yokoyama [CQ308783]ログフォーマットにR/Wを追加.
#ifdef DEF_A3A4COMM
	printf( "No.   TID        Type   ST[tick] R/W  SecNo     Size[KB]  ST[timeval us]   Name \n" ) ;
	printf( "----- ---------- ------ --------  -  ---------- -------- ----------.------ ---------------\n" ) ;
#else	// #ifdef DEF_A3A4COMM.
	printf( "No.   TID        ST[tick] R/W  SecNo     Size[KB] Time[msec] Name \n" ) ;
	printf( "----- ---------- --------  -  ---------- -------- ---------- ---------------\n" ) ;
#endif	// #ifdef DEF_A3A4COMM.
	//       uuuuu 0xXXXXXXXX uuuuuuuu uuuuuuuuuu fff.f    uuuuuu.uuu NNN...NNN.
	for ( register uint32_t ul_Idx = 0; ul_Idx < ul_Max; ul_Idx++ ) {
#ifdef DEF_A3A4COMM
		printf( "%5.5lu 0x%.8x ",ul_Idx, p->nTaskID );
		switch( p->uc_type ){
			case 0:  printf("0:RWed"); break;
			case 1:  printf("1:RWst"); break;
			case 2:  printf("2:AEst"); break;
			case 3:  printf("3:AEed"); break;
			case 4:  printf("4:TPst"); break;
			case 5:  printf("5:TPed"); break;
			case 6:  printf("6:HDst"); break;
			case 7:  printf("7:HDed"); break;
			default: printf(" ---- "); break;
		}

		printf( " %8lu ", p->ul_STick );
		if( (uchar)-1 == p->uc_Flags ){
			printf(" - ");
		} else {
			printf(" %c ", ( p->uc_Flags == HDDA_WRITE )? 'W':'R' );
		}
		if( (uint32_t)-1 == p->ul_SecNo ){
			printf(" -------- ");
		} else {
			printf( "%10lu",p->ul_SecNo);
		}
		printf( " %7.1f %10lu.%06lu  %s\n",
			((double)p->ul_Size/1024.0),
			p->st_TimeVal.tv_sec, p->st_TimeVal.tv_usec,
			SYSF_taskName(p->nTaskID) ) ;
#else	// #ifdef DEF_A3A4COMM.
		printf( "%5.5"PRIu32" 0x%.8"PRIx32" %8"PRIu32"  %c  %10"PRIu32" %5.1f    %10.3f %s\n",
			ul_Idx, p->nTaskID, p->ul_STick, 
			( p->uc_Flags == HDDA_WRITE )? 'W':'R',
			p->ul_SecNo, ((double)p->ul_Size/1024.0),
			U2M(p->ul_Time), SYSF_taskName(p->nTaskID) ) ;
#endif	// #ifdef DEF_A3A4COMM.
		//アドレスのバッファオーバー処理.
		if ( (++p) >= &pst[ul_HddA_MaxNum] ) {
			p = &pst[0] ;
		}
	}

	// duplicateした場合はこのメモリ解放.
	if ( pst != pst_HddATbl ) {
		delete [] pst; // 2011.12.14 s.yamauchi 静的解析対応：CID14062 配列のdelete対応.
	}
}

// ============================================ //.
// 2008/07/23 t.yokoyama [CQ308783]HDDモニタ開始関数へ第三引数を追加.
#ifdef DEF_A3A4COMM
void setHddAStart(
	uint32_t ul_SecNo,	    // セクターNo.
	uint32_t ul_Size, 	    // サイズ.
	uchar uc_Flags,		// 0=read,1=write.
	uchar uc_type )		// ログ種別.
#else	// #ifdef DEF_A3A4COMM.
void setHddAStart(
	uint32_t ul_SecNo,	    // セクターNo.
	uint32_t ul_Size, 	    // サイズ.
	uchar uc_Flags )	// 0=read,1=write.
#endif	// #if !defined DEF_A3A4COMM.
{
#ifdef DEF_A3A4COMM
	if( THD_hdda_OSRW_start == uc_type ){
#endif	// #ifdef DEF_A3A4COMM.
		setHddTransDelayStart(ul_SecNo,ul_Size,uc_Flags);	// 2013/10/23 s.iyanagi [CQ383221] HDD転送遅延ログ出力機能追加.
#ifdef DEF_A3A4COMM
	}
#endif	// #ifdef DEF_A3A4COMM.
	// 2008/05/29 t.yokoyama HDD常時ログ.
	// 2006/11/13 伊藤史：FATAL時HDDログ対応。FATAL発生時に停止。.
	if ( bl_HddA_Running == false && bl_HddAFatal_Stop == true ){
		//モニターが停止ししている場合.
		return ;
	}

#ifdef DEF_A3A4COMM
	if( THD_hdda_OSRW_start == uc_type ){
#endif	// #if !defined DEF_A3A4COMM.
		// 従来の hdda ログのみ.
		// ここでは一時記憶させるだけ、構造上ネストには対応不可.
		sst_hddatmp.nTaskID  = SYSF_taskIdSelf() ;
		sst_hddatmp.ul_STick = SYSF_tickGet() ;
		sst_hddatmp.ul_SecNo = ul_SecNo ;
		sst_hddatmp.ul_Size  = ul_Size ;
		sst_hddatmp.ul_Time  = usecTimerGet() ;	// 現在のusecタイマー値保持.
		sst_hddatmp.uc_Flags = uc_Flags;	    // 0=read,1=write.
	
// 2008/05/29 t.yokoyama HDD常時ログ.
		short ss_Index_Data = (-1);	//インデックス指定.
		// 2006/11/13 伊藤史：FATAL時HDDログ対応。FATAL発生時に停止。.
		if( bl_HddAFatal_Stop == false ){
			if( ss_HddAFatal_StartIndex == -1 ){
				// 初回アクセス：前回ログ表示後の初めてのアクセスの場合.
				ss_HddAFatal_StartIndex = 0;
				st_HddaTblFatal[ul_HddAFatal_Index].nTaskID  = 0xffffffff;
				st_HddaTblFatal[ul_HddAFatal_Index].ul_STick = 0xffffffff;
				st_HddaTblFatal[ul_HddAFatal_Index].ul_SecNo = 0xffffffff;
				st_HddaTblFatal[ul_HddAFatal_Index].ul_Size  = 0xffffffff;
				st_HddaTblFatal[ul_HddAFatal_Index].ul_Time  = 0xffffffff;
				st_HddaTblFatal[ul_HddAFatal_Index].uc_Flags = 0xff;
				ul_HddAFatal_Index = ((ul_HddAFatal_Index + 1) % HDDAFATALTBLMAX);				//位置の算出（カウントアップ）.
				if ( ul_HddAFatal_Index == 0 ){
					bl_HddAFatal_Loop = true ;
				}
// 2010/11/24 UV-00794 デバッグログ収集システム対応 M.Koshimo start.
				st_HDDATBL_Index.ss_HddAFatal_StartIndex = 0;
				st_HddaTblFatal_debug[st_HDDATBL_Index.ul_HddAFatal_Index].nTaskID  = 0xffffffff;
				st_HddaTblFatal_debug[st_HDDATBL_Index.ul_HddAFatal_Index].ul_STick = 0xffffffff;
				st_HddaTblFatal_debug[st_HDDATBL_Index.ul_HddAFatal_Index].ul_SecNo = 0xffffffff;
				st_HddaTblFatal_debug[st_HDDATBL_Index.ul_HddAFatal_Index].ul_Size  = 0xffffffff;
				st_HddaTblFatal_debug[st_HDDATBL_Index.ul_HddAFatal_Index].ul_Time  = 0xffffffff;
				st_HddaTblFatal_debug[st_HDDATBL_Index.ul_HddAFatal_Index].uc_Flags = 0xff;
				ss_Index_Data = (short)st_HDDATBL_Index.ul_HddAFatal_Index;	//インデックス指定.
				st_HDDATBL_Index.ul_HddAFatal_Index = ((st_HDDATBL_Index.ul_HddAFatal_Index + 1) % HDDAFATALTBLMAX);				//位置の算出（カウントアップ）.
				if ( st_HDDATBL_Index.ul_HddAFatal_Index == 0 ){
					st_HDDATBL_Index.bl_HddAFatal_Loop = true ;
				}
// 2010/11/24 UV-00794 デバッグログ収集システム対応 M.Koshimo start.
				short ss_Index = (-1);	//インデックス指定.
			//LogLibＡＰＩをコールし、共有メモリへ書き込み.
				LIBF_WriteSystemLog(LIBD_LogID_THRC_HddaFATAL_Index, 
								LIBD_LogOutPutLevel1,
								LIBD_LogCategoryCopy|LIBD_LogCategoryPrint|LIBD_LogCategoryBox|LIBD_LogCategoryFax|LIBD_LogCategoryNetFax,
								&st_HDDATBL_Index,
								&ss_Index );

				LIBF_WriteSystemLog(LIBD_LogID_THRC_HddaFATAL, 
								LIBD_LogOutPutLevel1,
								LIBD_LogCategoryCopy|LIBD_LogCategoryPrint|LIBD_LogCategoryBox|LIBD_LogCategoryFax|LIBD_LogCategoryNetFax,
								&st_HddaTblFatal_debug[ss_Index_Data],
								&ss_Index_Data );
// 2010/11/24 UV-00794 デバッグログ収集システム対応 M.Koshimo end.
			}
			
		// 多重Startチェック.
			ss_HddAFatal_StartIndex++;
// 2010/11/24 UV-00794 デバッグログ収集システム対応 M.Koshimo start.
			st_HDDATBL_Index.ss_HddAFatal_StartIndex++;
// 2010/11/24 UV-00794 デバッグログ収集システム対応 M.Koshimo end.

			if( bl_HddAFatal_Stop == false && ss_HddAFatal_StartIndex > 1 ){
				// 多重StartチェックNG.
				st_HddaTblFatal[ul_HddAFatal_Index].nTaskID  = SYSF_taskIdSelf() ;
				st_HddaTblFatal[ul_HddAFatal_Index].ul_STick = SYSF_tickGet() ;
				st_HddaTblFatal[ul_HddAFatal_Index].ul_SecNo = ul_SecNo ;
				st_HddaTblFatal[ul_HddAFatal_Index].ul_Size  = ul_Size ;
				st_HddaTblFatal[ul_HddAFatal_Index].ul_Time  = 0xffffffff ;	// 多重Start NG.
				st_HddaTblFatal[ul_HddAFatal_Index].uc_Flags = uc_Flags;	// 0=read,1=write.

				ul_HddAFatal_Index = ((ul_HddAFatal_Index + 1) % HDDAFATALTBLMAX);				//位置の算出（カウントアップ）.
				if ( ul_HddAFatal_Index == 0 ){
					bl_HddAFatal_Loop = true ;
				}
// 2010/11/24 UV-00794 デバッグログ収集システム対応 M.Koshimo start.
				st_HddaTblFatal_debug[st_HDDATBL_Index.ul_HddAFatal_Index].nTaskID  = SYSF_taskIdSelf() ;
				st_HddaTblFatal_debug[st_HDDATBL_Index.ul_HddAFatal_Index].ul_STick = SYSF_tickGet() ;
				st_HddaTblFatal_debug[st_HDDATBL_Index.ul_HddAFatal_Index].ul_SecNo = ul_SecNo ;
				st_HddaTblFatal_debug[st_HDDATBL_Index.ul_HddAFatal_Index].ul_Size  = ul_Size ;
				st_HddaTblFatal_debug[st_HDDATBL_Index.ul_HddAFatal_Index].ul_Time  = 0xffffffff ;	// 多重Start NG.
				st_HddaTblFatal_debug[st_HDDATBL_Index.ul_HddAFatal_Index].uc_Flags = uc_Flags;	// 0=read,1=write.

				ss_Index_Data = (short)st_HDDATBL_Index.ul_HddAFatal_Index;	//インデックス指定.
				st_HDDATBL_Index.ul_HddAFatal_Index = ((st_HDDATBL_Index.ul_HddAFatal_Index + 1) % HDDAFATALTBLMAX);				//位置の算出（カウントアップ）.
				if ( st_HDDATBL_Index.ul_HddAFatal_Index == 0 ){
					st_HDDATBL_Index.bl_HddAFatal_Loop = true ;
				}
// 2010/11/24 UV-00794 デバッグログ収集システム対応 M.Koshimo end.
			}
// 2010/11/24 UV-00794 デバッグログ収集システム対応 M.Koshimo start.
//Str【EagleZ+ AddressSanitizer対応 : Y.Okumoto : 2019.12.11】.
			if( ss_Index_Data != (-1) ){
				short ss_Index = (-1);	//インデックス指定.
				//LogLibＡＰＩをコールし、共有メモリへ書き込み.
				LIBF_WriteSystemLog(LIBD_LogID_THRC_HddaFATAL_Index, 
								LIBD_LogOutPutLevel1,
								LIBD_LogCategoryCopy|LIBD_LogCategoryPrint|LIBD_LogCategoryBox|LIBD_LogCategoryFax|LIBD_LogCategoryNetFax,
								&st_HDDATBL_Index,
								&ss_Index );

				LIBF_WriteSystemLog(LIBD_LogID_THRC_HddaFATAL, 
								LIBD_LogOutPutLevel1,
								LIBD_LogCategoryCopy|LIBD_LogCategoryPrint|LIBD_LogCategoryBox|LIBD_LogCategoryFax|LIBD_LogCategoryNetFax,
								&st_HddaTblFatal_debug[ss_Index_Data],
								&ss_Index_Data );
			}
//End【EagleZ+ AddressSanitizer対応 : Y.Okumoto : 2019.12.11】.
// 2010/11/24 UV-00794 デバッグログ収集システム対応 M.Koshimo end.
		}
#ifdef DEF_A3A4COMM
	}
#endif	// #ifdef DEF_A3A4COMM.	

#ifdef DEF_A3A4COMM
	if ( bl_HddA_Running == false ) {
		//モニターが停止ししている場合.
		return ;
	}
	if ( pst_HddATbl == NULL ) {
		//テーブル領域がない場合.
		return ;
	}
	//タスク情報アドレス位置の設定.
	register ST_HDDATBLEX* p = &pst_HddATbl[ul_HddA_Index] ;

	p->nTaskID	= SYSF_taskIdSelf() ;			// タスクID.
	p->ul_SecNo	= ul_SecNo ;		// セクターNo.
	p->ul_Size	= ul_Size ;			// サイズ.
	p->ul_STick	= SYSF_tickGet() ;		// 開始時Tick.
	p->uc_Flags	= uc_Flags;			// 0=read,1=write.
	p->uc_type  = uc_type;			// ログ種別..
	gettimeofday( &p->st_TimeVal, NULL );
	ul_HddA_Index = ((ul_HddA_Index + 1) % ul_HddA_MaxNum);			//位置の算出（カウントアップ）.
	if ( ul_HddA_Index == 0 ){
		bl_HddA_Loop = true ;
	}
#endif	// #ifdef DEF_A3A4COMM.
}

#ifdef DEF_A3A4COMM
//  2019/11/26 Sky井柳 【HeliosMLK】OP_BTS-13978 静的解析対応 ID51140.
//  					[引数の型変更 ushort -> uchar].
void setHddAEnd(uchar uc_type)
#else	// #ifdef DEF_A3A4COMM.
void setHddAEnd()
#endif	// #ifdef DEF_A3A4COMM.
{
// 2008/05/29 t.yokoyama HDD常時ログ.
	short ss_Index_Data = (-1);

#ifdef DEF_A3A4COMM
	if( THD_hdda_OSRW_end == uc_type ){
#endif	// #ifdef DEF_A3A4COMM.
		setHddTransDelayEnd();	// 2013/10/23 s.iyanagi [CQ383221] HDD転送遅延ログ出力機能追加.
		// 2006/11/13 伊藤史：FATAL時HDDログ対応。FATAL発生時に停止。.
		if( bl_HddAFatal_Stop == false && ss_HddAFatal_StartIndex > 0 ){
			ss_HddAFatal_StartIndex--;

			st_HddaTblFatal[ul_HddAFatal_Index].nTaskID		= sst_hddatmp.nTaskID ;			// タスクID.
			st_HddaTblFatal[ul_HddAFatal_Index].ul_SecNo	= sst_hddatmp.ul_SecNo ;		// セクターNo.
			st_HddaTblFatal[ul_HddAFatal_Index].ul_Size		= sst_hddatmp.ul_Size ;			// サイズ.
			st_HddaTblFatal[ul_HddAFatal_Index].ul_STick	= sst_hddatmp.ul_STick ;		// 開始時Tick.
			st_HddaTblFatal[ul_HddAFatal_Index].ul_Time		= SYSF_tickGet() ;					// 終了時Tick.
			st_HddaTblFatal[ul_HddAFatal_Index].uc_Flags	= sst_hddatmp.uc_Flags;			// 0=read,1=write.

			ss_Index_Data = (short)ul_HddAFatal_Index;

			ul_HddAFatal_Index = ((ul_HddAFatal_Index + 1) % HDDAFATALTBLMAX);				//位置の算出（カウントアップ）.
			if ( ul_HddAFatal_Index == 0 ){
				bl_HddAFatal_Loop = true ;
			}
// 2010/11/24 UV-00794 デバッグログ収集システム対応 M.Koshimo start.
			st_HDDATBL_Index.ss_HddAFatal_StartIndex--;

			st_HddaTblFatal_debug[ss_Index_Data].nTaskID		= sst_hddatmp.nTaskID ;			// タスクID.
			st_HddaTblFatal_debug[ss_Index_Data].ul_SecNo	= sst_hddatmp.ul_SecNo ;		// セクターNo.
			st_HddaTblFatal_debug[ss_Index_Data].ul_Size		= sst_hddatmp.ul_Size ;			// サイズ.
			st_HddaTblFatal_debug[ss_Index_Data].ul_STick	= sst_hddatmp.ul_STick ;		// 開始時Tick.
			st_HddaTblFatal_debug[ss_Index_Data].ul_Time		= SYSF_tickGet() ;					// 終了時Tick.
			st_HddaTblFatal_debug[ss_Index_Data].uc_Flags	= sst_hddatmp.uc_Flags;			// 0=read,1=write.

			st_HDDATBL_Index.ul_HddAFatal_Index = ((st_HDDATBL_Index.ul_HddAFatal_Index + 1) % HDDAFATALTBLMAX);				//位置の算出（カウントアップ）.
			if ( st_HDDATBL_Index.ul_HddAFatal_Index == 0 ){
				st_HDDATBL_Index.bl_HddAFatal_Loop = true ;
			}
			short ss_Index = (-1);	//インデックス指定.
			//LogLibＡＰＩをコールし、共有メモリへ書き込み.
			LIBF_WriteSystemLog(LIBD_LogID_THRC_HddaFATAL_Index, 
							LIBD_LogOutPutLevel1,
							LIBD_LogCategoryCopy|LIBD_LogCategoryPrint|LIBD_LogCategoryBox|LIBD_LogCategoryFax|LIBD_LogCategoryNetFax,
							&st_HDDATBL_Index,
							&ss_Index );

			LIBF_WriteSystemLog(LIBD_LogID_THRC_HddaFATAL, 
							LIBD_LogOutPutLevel1,
							LIBD_LogCategoryCopy|LIBD_LogCategoryPrint|LIBD_LogCategoryBox|LIBD_LogCategoryFax|LIBD_LogCategoryNetFax,
							&st_HddaTblFatal_debug[ss_Index_Data],
							&ss_Index_Data );
// 2010/11/24 UV-00794 デバッグログ収集システム対応 M.Koshimo end.
		}
#ifdef DEF_A3A4COMM
	}
#endif	// #ifdef DEF_A3A4COMM.

	if ( bl_HddA_Running == false ) {
		//モニターが停止ししている場合.
		return ;
	}
	if ( pst_HddATbl == NULL ) {
		//テーブル領域がない場合.
		return ;
	}
	
	//タスク情報アドレス位置の設定.
#ifdef DEF_A3A4COMM
	register ST_HDDATBLEX* p = &pst_HddATbl[ul_HddA_Index];
#else	// #ifdef DEF_A3A4COMM.
	register ST_HDDATBL* p = &pst_HddATbl[ul_HddA_Index] ;
#endif	// #ifdef DEF_A3A4COMM.
	p->nTaskID	= sst_hddatmp.nTaskID ;			// タスクID.
	p->ul_SecNo	= sst_hddatmp.ul_SecNo ;		// セクターNo.
	p->ul_Size	= sst_hddatmp.ul_Size ;			// サイズ.
#ifdef DEF_A3A4COMM
	p->ul_STick	= SYSF_tickGet();		// 開始時Tick.
#else	// #ifdef DEF_A3A4COMM.
	p->ul_STick	= sst_hddatmp.ul_STick ;		// 開始時Tick.
	p->ul_Time	= (usecTimerGet() - sst_hddatmp.ul_Time) ;			// 処理時間(usec).
#endif	// #ifdef DEF_A3A4COMM.

	p->uc_Flags	= sst_hddatmp.uc_Flags;			// 0=read,1=write.
#ifdef DEF_A3A4COMM
	p->uc_type  = uc_type;			// ログ種別..
	gettimeofday( &p->st_TimeVal, NULL );
#endif	// #ifdef DEF_A3A4COMM.
	ul_HddA_Index = ((ul_HddA_Index + 1) % ul_HddA_MaxNum);			//位置の算出（カウントアップ）.
	if ( ul_HddA_Index == 0 ){
		bl_HddA_Loop = true ;
	}
}


// ============================================ //.
//モニタデータ表示.
// 2006/11/13 伊藤史：FATAL時HDDログ対応。FATAL発生時に停止。.
// ============================================ //.
void _dsphddaFATAL()
{

	bl_HddAFatal_Stop = true;		// ログを表示している間は、ログを停止.

	if( ss_HddAFatal_StartIndex > 0 ){
		// 未終了のログを取り出す.
		st_HddaTblFatal[ul_HddAFatal_Index].nTaskID		= sst_hddatmp.nTaskID ;			// タスクID.
		st_HddaTblFatal[ul_HddAFatal_Index].ul_SecNo	= sst_hddatmp.ul_SecNo ;		// セクターNo.
		st_HddaTblFatal[ul_HddAFatal_Index].ul_Size		= sst_hddatmp.ul_Size ;			// サイズ.
		st_HddaTblFatal[ul_HddAFatal_Index].ul_STick	= sst_hddatmp.ul_STick ;		// 開始時Tick.
		st_HddaTblFatal[ul_HddAFatal_Index].ul_Time		= 0xffffffff;					// 未終了.
		st_HddaTblFatal[ul_HddAFatal_Index].uc_Flags	= sst_hddatmp.uc_Flags;			// 0=read,1=write.

		ul_HddAFatal_Index = ((ul_HddAFatal_Index + 1) % HDDAFATALTBLMAX);				//位置の算出（カウントアップ）.
		if ( ul_HddAFatal_Index == 0 ){
			bl_HddAFatal_Loop = true ;
		}
	}

	register uint32_t ul_Max = ul_HddAFatal_Index ;								// 登録最終位置を設定.
	register uint32_t ul_Idx = 0;
	if ( bl_HddAFatal_Loop == true ) {
		ul_Max = HDDAFATALTBLMAX ;								// 最大値を設定.
		ul_Idx = ul_HddAFatal_Index;
	}

	// 08/07/23 t.yokoyama [CQ308783]ログ出力フォーマットにR/Wを追加.  SizeをMBにした.
#ifdef DEF_A3A4COMM
	printf( "(   ST[tick])No.   TID        R/W   SecNo    Size[KB] Ed[tick] Name \n" ) ;
#else	// #ifdef DEF_A3A4COMM.
	printf( "(   ST[tick])No.   TID        R/W   SecNo    Size[MB] Ed[tick] Name \n" ) ;
#endif	// #ifdef DEF_A3A4COMM.
	printf( "------------ ----- ----------  -  ---------- -------- -------- ---------------\n" ) ;
	//       uuuuu 0xXXXXXXXX uuuuuuuu uuuuuuuuuu fffff.f  uuuuuuuu NNN...NNN.
	for ( register uint32_t idx=0; idx < ul_Max; idx++ ) {
		if( (uint32_t)st_HddaTblFatal[ul_Idx].nTaskID == 0xffffffff ){
			// 初回アクセス.
			printf( "============ ===== ==========  =  ========== ======== ======== ===============\n" ) ;
		}
		else{
#ifdef DEF_A3A4COMM
			printf( "(%10lu) %5.5lu 0x%.8x  %c  %10lu %6.1f  ",
#else	// #ifdef DEF_A3A4COMM.
			printf( "(%10"PRIu32") %5.5"PRIu32" 0x%.8"PRIx32"  %c  %10"PRIu32" %5.1f  ",
#endif	// #ifdef DEF_A3A4COMM.
				st_HddaTblFatal[ul_Idx].ul_STick, ul_Idx, st_HddaTblFatal[ul_Idx].nTaskID, 
				( st_HddaTblFatal[ul_Idx].uc_Flags == HDDA_WRITE )? 'W':'R',
				st_HddaTblFatal[ul_Idx].ul_SecNo,    ((double)st_HddaTblFatal[ul_Idx].ul_Size/1024.0) );

			if( st_HddaTblFatal[ul_Idx].ul_Time == 0xffffffff ){
				printf( "(%10"PRIx32") %s\n", st_HddaTblFatal[ul_Idx].ul_Time, SYSF_taskName(st_HddaTblFatal[ul_Idx].nTaskID) ) ;
			}
			else{
				printf( "(%10"PRIu32") %s\n", st_HddaTblFatal[ul_Idx].ul_Time, SYSF_taskName(st_HddaTblFatal[ul_Idx].nTaskID) ) ;
			}
		}

		//アドレスのバッファオーバー処理.
		ul_Idx = ((ul_Idx + 1) % HDDAFATALTBLMAX);				//位置の算出（カウントアップ）.
	}

	// ログを再開.
	ss_HddAFatal_StartIndex = -1;	// 初回アクセス準備.
	bl_HddAFatal_Stop = false;

#ifdef DEF_A3A4COMM
	dsphdda();
#endif	// #ifdef DEF_A3A4COMM.
}

extern "C" void dsphddaFATAL()
{
	_dsphddaFATAL();
}

// DebugForeverから停止をさせる。.
void stopHddaFatal(ushort us_Code)
{
	bl_HddAFatal_Stop    = true;
	
	if( us_Code >= HDD_CONNECT_ERROR && us_Code <= HDD_OUT_OF_SPEC ){
		if( ss_HddAFatal_StartIndex > 0 ){
			// HDDエラーの場合、未終了のアクセスがあればタスクを停止する。.
			SYSF_taskSuspend( sst_hddatmp.nTaskID );
		}
	}
}

/*================================================================================================
［機能］	【ターミナルコマンド】ファイル作成速度調査 
--------------------------------------------------------------------------------------------------
［戻値］	なし
 [詳細] 	デバッグ用にNVRAMバックアップ領域を削除する 
 			市場環境再現用 
［ｳｫｰｸｽﾙｰ］	実施未 
 [履歴]		2005/12/02	R.Suzuki	新規作成 
 			2006/03/29	ct_osa		変更：複数フォルダにわけファイル作成時間を計測する
 			2011.12.14 s.yamauchi 静的解析対応：CID14061 配列のdelete漏れ修正
 			2013.06.14 suzuki		メモリ削減対応でTHRD_ENABLE_FM_LOで有効/無効を切り替え.
================================================================================================*/
//#include "ioLib.h".


//メモリ削減のためにログ格納をコンパイルスイッチ化.
//#define THRD_ENABLE_FM_LOG	//メモリ削減対応.

#ifdef THRD_ENABLE_FM_LOG 
#define FM_LOG_NUM 1000
uint32_t ul_FMTick[ FM_LOG_NUM ] = {0};
#endif	//THRD_ENABLE_FM_LOG.

/////////////////////////////////////////////////////////////////////////////////////////////////.
//ファイルの実作成.
//----------------------------------------------------------------------------------------------.
//引数：		pscPath	:ファイル作成フォルダ.
//				nSize	:作成ファイル容量.
//				nNum	:ファイル作成数.
/////////////////////////////////////////////////////////////////////////////////////////////////.
void makedummyf( const char* pscPath, int32_t nSize, int32_t nNum )
{
#ifdef THRD_ENABLE_FM_LOG
	char strPath[PATH_MAX];
	char* nPtn = new char[nSize];
	
	//ダミーデータ作成.
	memset(nPtn,0xcd,nSize);
	
	for ( register uint32_t i=0; i < (uint32_t)nNum; i++ ) {
		//ファイル名の作成.
		sprintf(strPath, "%sdummy%"PRIu32"", pscPath, i);
		register int32_t fd = SYSF_open( strPath, O_RDWR | O_CREAT | O_BINARY, 0666 ) ;
		if ( fd == ERROR ){
			printf("File Open error%s\n",pscPath);
			break ;
		}
		//データの書き込み.
		if ( write( fd, nPtn, nSize ) != nSize ) {
			printf("error(write)\n");
			SYSF_close(fd);
			break ;
		}
		SYSF_close(fd);
	}
	delete [] nPtn ; // 2011.12.14 s.yamauchi 静的解析対応：CID14061 配列のdelete漏れ修正.
#else
	printf("\n---- notdefine THRD_ENABLE_FM_LOG ----\n");
#endif	//THRD_ENABLE_FM_LOG.
}

/////////////////////////////////////////////////////////////////////////////////////////////////.
// 機能：上記関数をループで回す.
//----------------------------------------------------------------------------------------------.
// 引数：	nSize		:作成ファイル容量.
//			nFileNum	:ファイル数（時間計測単位）.
//			nNum		:ループ回数（１フォルダ計測回数）.
//			dir_num		:作成フォルダ数.
/////////////////////////////////////////////////////////////////////////////////////////////////.
extern "C" void startFM( int32_t nSize, int32_t nFileNum, int32_t nNum ,int32_t dir_num)
{
#ifdef THRD_ENABLE_FM_LOG
	char strPath[PATH_MAX];
	char setPath[PATH_MAX];
	char makPath[10];
	int32_t Ind,dir_c;
	uint32_t ul_TotalTime;
	int32_t set_point;
	
	set_point=0;												//配列設定位置クリア.
	ul_TotalTime = SYSF_tickGet();									//測定開始時間覚え.
	
	//計測開始.
	for( dir_c = 0; dir_c < dir_num; dir_c++ ){
		//ディレクトリ作成.
		sprintf( strPath, "gomi%"PRIu32"", dir_c );
		if(SYSF_mkdir(strPath)==ERROR){
			//ディレクトリが作成できない場合　エラー.
			printf("error(mkdir)\n");
			return;
		}
		sprintf( strPath, "/ata01/gomi%"PRIu32"/", dir_c );
		//sprintf( strPath, "gomi%u\\", dir_c );.
		for( Ind = 0; Ind < nNum; Ind++ ){
			sprintf( makPath, "%"PRIu32"", Ind );
			strcpy(setPath,(const char *)strPath);				//パスの設定.
			strcat(setPath,makPath);							//パスの設定.
			makedummyf( setPath, nSize, nFileNum );				//ファイル作成ルーチンコール.
			ul_FMTick[ set_point % FM_LOG_NUM ] = SYSF_tickGet();
			set_point++;
		}
	}
	ul_TotalTime = SYSF_tickGet() - ul_TotalTime;				//書き込みにかかった時間.
	printf( "TotalTick : %"PRIu32" TotalSec : %"PRIu32"", ul_TotalTime, ul_TotalTime * 5 / 1000 );
#else
	printf("\n---- notdefine THRD_ENABLE_FM_LOG ----\n");
#endif	//THRD_ENABLE_FM_LOG.
}

/////////////////////////////////////////////////////////////////////////////////////////////////.
//	ログ表示ルーチン.
//----------------------------------------------------------------------------------------------.
//引数：		NONE.
/////////////////////////////////////////////////////////////////////////////////////////////////.
extern "C" void dbgLogFM()
{
#ifdef THRD_ENABLE_FM_LOG
	int32_t Ind;
	for( Ind = 1; Ind < FM_LOG_NUM; Ind++ ){
		printf( "%03"PRId32" %"PRIu32"\n", Ind, ul_FMTick[ Ind % FM_LOG_NUM ] - ul_FMTick[ ( Ind - 1 ) % FM_LOG_NUM ] );
		if( ul_FMTick[ Ind % FM_LOG_NUM ] == 0 ){ // 必要なとこまで出力.
			break;
		}
	}
#else
	printf("\n---- notdefine THRD_ENABLE_FM_LOG ----\n");
#endif	//THRD_ENABLE_FM_LOG.
}

#endif // #ifdef TORNADO.
//HDDシーケンスログ(debugLog 12) 2005/10/07 added by t.sasaki +06/04/17 yoshii Moselマージのため追加.
void setDebugHddSeq( ushort us_State, uint32_t ul_Data )
{
	ST_DEBUGSEQ* p = &st_DebugHddSeq[us_DebugHddSeqIndex%DEF_CONNECTLOG_NUM] ;
// 2010/11/24 UV-00794 デバッグログ収集システム対応 M.Koshimo start.
	// 通常はインデックス定義時にクリアしており、デバッグログ収集システムはタイミングがないのでここで通常のインデックス値を入れる。.
	st_debugLog12_Index.us_DebugHddSeqIndex=us_DebugHddSeqIndex;
// 2010/11/24 UV-00794 デバッグログ収集システム対応 M.Koshimo end.
	short ss_Index_Data = (-1);

	p->us_mode	= us_State ;
	p->us_data	= ul_Data ;	// us_dataの型はuint32_t.
	p->ul_time	= SYSF_tickGet() ;
	us_DebugHddSeqIndex++ ;
	if ( us_DebugHddSeqIndex >= (USHRT_MAX-(USHRT_MAX%DEF_CONNECTLOG_NUM)) ) {
		us_DebugHddSeqIndex = 0 ;
		bl_DebugHddSeqIndexLoop = true ;
	}
// 2010/11/24 UV-00794 デバッグログ収集システム対応 M.Koshimo start.
	st_debugLog12[st_debugLog12_Index.us_DebugHddSeqIndex%DEF_CONNECTLOG_NUM].us_mode = us_State;
	st_debugLog12[st_debugLog12_Index.us_DebugHddSeqIndex%DEF_CONNECTLOG_NUM].us_Data = (ushort)ul_Data;
	st_debugLog12[st_debugLog12_Index.us_DebugHddSeqIndex%DEF_CONNECTLOG_NUM].ul_time = SYSF_tickGet();
	ss_Index_Data = st_debugLog12_Index.us_DebugHddSeqIndex%DEF_CONNECTLOG_NUM;
	st_debugLog12_Index.us_DebugHddSeqIndex++;
	if ( st_debugLog12_Index.us_DebugHddSeqIndex >= (USHRT_MAX-(USHRT_MAX%DEF_CONNECTLOG_NUM)) ) {
		st_debugLog12_Index.us_DebugHddSeqIndex = 0 ;
		st_debugLog12_Index.bl_DebugHddSeqIndexLoop = true ;
	}
	short ss_Index = (-1);	//インデックス指定.
	//LogLibＡＰＩをコールし、共有メモリへ書き込み.
	LIBF_WriteSystemLog(LIBD_LogID_THRC_DebugLog12_Index, 
						LIBD_LogOutPutLevel1,
						LIBD_LogCategoryCopy|LIBD_LogCategoryPrint|LIBD_LogCategoryBox|LIBD_LogCategoryFax|LIBD_LogCategoryNetFax,
						&st_debugLog12_Index,
						&ss_Index );

	LIBF_WriteSystemLog(LIBD_LogID_THRC_DebugLog12, 
						LIBD_LogOutPutLevel1,
						LIBD_LogCategoryCopy|LIBD_LogCategoryPrint|LIBD_LogCategoryBox|LIBD_LogCategoryFax|LIBD_LogCategoryNetFax,
						&st_debugLog12[ss_Index_Data],
						&ss_Index_Data );
// 2010/11/24 UV-00794 デバッグログ収集システム対応 M.Koshimo end.

}
ushort getDebugHddSeq( ushort us_No, uint32_t& ul_Data, uint32_t& ul_Tick )
{
	ST_DEBUGSEQ* p = &st_DebugHddSeq[us_No%DEF_CONNECTLOG_NUM] ;
	ul_Data	= p->us_data ;	// us_dataの型はuint32_t.
	ul_Tick	= p->ul_time ;
	return p->us_mode ;
}
ushort getDebugHddSeqNo()
{
	return us_DebugHddSeqIndex ;
}


// ================================================================================================.
//!［機能］Emperor初期化完了判定.
// ------------------------------------------------------------------------------------------------.
// ［引数］なし.
// ［戻値］True：Emperor初期化完了。  False：Emperor未初期化.
// ［詳細］GetROMVersion が成功した場合、初期化完了と判定してよい。.
//.
// ［履歴］2007/05/22 伊藤史 新規作成.
//         2012/07/12 小霜   CQ.368361 Soc暫定対応を削除
// ================================================================================================.
Bool isEmperorInitTaskDone()
{
#ifdef SIMULATOR		// シミュレータで有効.
	return True;		// 問題点82163対応 2007/06/12 12仁枝.
#endif	// #ifdef SIMULATOR.

	Bool  bl_FlgDone = False;
    char  ch_ROMstr[40+1];
    APIC_SystemInfo*  pc_SystemInfo = APIC_SystemInfo::newInstance();

	if( pc_SystemInfo == NULL ) return bl_FlgDone;

	SYSF_taskLock();

	// Emperorのバージョン取得.
    pc_SystemInfo->getMFPControllerRomVersion( TYPE_MFPControllerRomPrinter, ch_ROMstr );
	ch_ROMstr[40] = 0;
	if( strlen(ch_ROMstr) > 16 ){
		// Emperor Fontのバージョン取得.
	    pc_SystemInfo->getMFPControllerRomVersion( TYPE_MFPControllerRomPrinterFont, ch_ROMstr );
		ch_ROMstr[40] = 0;
		if( strlen(ch_ROMstr) > 16 ){
			bl_FlgDone = True;
		}
	}

	SYSF_taskUnlock();

	return bl_FlgDone;
}

//! 【自動評価用】start 2009.02.13 tsukiji Own/Genericデバッグコマンド(出荷前検査装置のコマンドのまね).
#if defined( DEF_SimAutoTest ) && defined( TORNADO )		// 自動評価かつVXWORKSで有効.
// 【自動評価用】2009.02.13 tsukiji Own/Generic状態取得.
extern "C" void GetOwnGenericAuto()
{
	if( True == pc_SystemInfo->isOwnOrGeneric() ){
		printf( " Last Value = 2\n" );
	}else{
		printf( " Last Value = 1\n" );
	}
}
// 【自動評価用】2009.02.13 tsukiji Own/Generic切り替え.
extern "C" void SetOwnGenericAuto( uchar mode )
{
	GetOwnGenericAuto(); // 現在値表示.

	if( 2 < mode || 0 == mode ){
		printf( " [Format]SetOwnGeneric Generic or OWN\n" );
		printf( " Generic/OWN … 1:Generic 2:OWN\n" );
		return;
	}
	APIC_SoftDipSw* pc_SoftDipSw = APIC_SoftDipSw::newInstance();
	if( 2 == mode ){ // SoftDipSW10に値をセットする.
		pc_SoftDipSw->setSoftDipSwBit( 3, 2, True  ); // Own.
		pc_SoftDipSw->setSoftDipSwBit( 1, 1, False );
	}else{
		pc_SoftDipSw->setSoftDipSwBit( 3, 2, False ); // Generic.
		pc_SoftDipSw->setSoftDipSwBit( 1, 1, True  );
	}
	pc_SystemInfo->chkAndSetOwnOrGeneric(); // NVRAMにOwnorGenericを書き込む.
	if( True == pc_SystemInfo->isOwnOrGeneric() ){
		printf( " New  Value = 2\n" );
	}else{
		printf( " New  Value = 1\n" );
	}
}
#endif //defined( DEF_SimAutoTest ) && defined( TORNADO )		// 自動評価かつVXWORKSで有効.
//! 【自動評価用】 end 2009.02.13 tsukiji Own/Genericデバッグコマンド(出荷前検査装置のコマンドのまね).

/*================================================================================================
［機能］	ターミナルコマンド（ MFPコントローラ基板シリアル番号取得 ）
--------------------------------------------------------------------------------------------------
［引数］	なし. 
［戻値］	なし. 
［詳細］	MFPコントローラ基板シリアル番号取得 I/F関数.
［ｳｫｰｸｽﾙｰ］	未実施. 
--------------------------------------------------------------------------------------------------
 [履歴]
 '08/03/21 sunami CQ:301776	 新規作成 出荷前検査装置  MFPコントローラ基板シリアル番号取得. 
================================================================================================*/
void GetBoardSerialNo( void )
{
#ifdef TORNADO
	THRL_Drvrif_SysBspBoardSerialNoPrint();
#endif // #if defined(TORNADO).
}

/*================================================================================================.
［機能］	生産ロガー（ MFPコントローラ基板シリアル番号取得 ）.
--------------------------------------------------------------------------------------------------.
［引数］	シリアルNo. 
［戻値］	True:OK False:NG. 
［詳細］	MFPコントローラ基板シリアル番号取得 I/F関数.
［ｳｫｰｸｽﾙｰ］	未実施. 
--------------------------------------------------------------------------------------------------.
 [履歴].
 '12/10/22 M.koshimo 新規作成 生産ロガー  MFPコントローラ基板シリアル番号取得. 
 '15/05/20 miyazaki  [MinervaSBK][CQ508955] 制御構成変更 No.51 OEMカスタマイズデータ名称変更.
 '18/06/12 eSOL長坂  [Sparrow] 制御構成変更 No.18 VenusMLK OEMカスタマイズデータ名称変更.
 '18/07/12 Sky薗﨑  [Eagle] AR1048759_【EagleL/Sparrow】OEMカスタマイズデータのダウンロードに失敗する(バイナリのファイル名が異なる)不具合対応.
================================================================================================*/
Bool GetBoardSerialNo_Data( char *pc_BoardSerialNo )
{
#ifdef TORNADO
	return THRL_Drvrif_SysBspBoardSerialNoPrint_Data( pc_BoardSerialNo );
#else // #if defined(TORNADO).
	return False;
#endif
}

/* USBダウンロード用マウントポイント */
static const int32_t THRD_NameLenMax = (64);	// ファイル名(絶対パス)の最大長.
static const int32_t THRD_UsbNum	 = (1);		// USBデバイスのパスの取得数.
static const int32_t THRD_UsbPathLen = (20+1);	// Sys側の文字列長(20)+終端\0.
char *cp_OemFilePath=NULL;

/* OEMファイル名 */
#if defined(DEF_A4_Cus)				// A4:VenusMLK, Sparrow MFP, Sparrow AIO/SFP.
									// 2018/07/02 [Sparrow] eSOL長坂 AR1046933 代表機種コード変更対応
	#if defined(DEF_SparrowAIOSFPCus)		// 2018/06/12 eSOL長坂 [Sparrow AIO/SFP] 制御構成変更No.18 OEM名称カスタマイズデータのファイル名称変更.
		#if defined( DEF_SparrowBKCus )
			char *cp_oemDefaultFileName = "bh4700iSeries_OEM.bin";		// SparrowBK SFP.
		#else
			char *cp_oemDefaultFileName = "bhC3300iSeries_OEM.bin";
		#endif
	#elif defined(DEF_SparrowCus)			// 2018/06/12 eSOL長坂 [Sparrow MFP] 制御構成変更No.18 OEM名称カスタマイズデータのファイル名称変更.
		#if defined( DEF_SparrowBKCus )
			char *cp_oemDefaultFileName = "bh4050iSeries_OEM.bin";		// SparrowBK MFP.
		#else
			char *cp_oemDefaultFileName = "bhC3350iSeries_OEM.bin";
		#endif
	#else
		char *cp_oemDefaultFileName = "bhC3851Series_OEM.bin";		// CQ522423 / CQ524346 eSOL) K.Shiranami [VenusMLK] OEMドライバ名称カスタマイズファイル名定義対応.
	#endif
#elif defined(DEF_MinervaSSBKCus)	// MinervaSSBK.
char *cp_oemDefaultFileName = "bh950iSeries_OEM.bin";		// MinervaSSBK.
#elif defined(DEF_HeliosMLKCus)	// HeliosMLK.
char *cp_oemDefaultFileName = "bhC287iSeries_OEM.bin";		// HeliosMLK.
#elif defined(DEF_DenebMLKCus)	// DenebMLK.
char *cp_oemDefaultFileName = "bh306iSeries_OEM.bin";		// DenebMLK Sky井谷 制御構成変更No.3 OEM名称カスタマイズデータのファイル名称変更.
#elif defined(DEF_EagleCus)	// 2017/09/11 Sky薗﨑 [Eagle] 制御構成変更No.3 OEM名称カスタマイズデータのファイル名称変更.
	#if defined(DEF_EagleZpCus )	// EagleZ+.
		#if defined(DEF_EagleZBKpCus)		// EagleBkZ+.
			char *cp_oemDefaultFileName = "bh750iSeries_OEM.bin";
		#else // defined(DEF_EagleZBKpCus).
			char *cp_oemDefaultFileName = "bhC750iSeries_OEM.bin";
		#endif // defined(DEF_EagleZBKpCus).
	#else // defined(DEF_EagleZpCus).
		#if defined(DEF_EagleBKCus)	// EagleBK.
			#if defined(DEF_A800)	// Eagle_High.
				char *cp_oemDefaultFileName = "bh650iSeries_OEM.bin";
			#else // defined(DEF_A800) Eagle_Low.
				char *cp_oemDefaultFileName = "bh360iSeries_OEM.bin";
			#endif // defined(DEF_A800).
		#else // defined(DEF_EagleBKCus).
			#if defined(DEF_A800)	// Eagle_High.
				char *cp_oemDefaultFileName = "bhC650iSeries_OEM.bin";
			#else // defined(DEF_A800) Eagle_Low.
				char *cp_oemDefaultFileName = "bhC360iSeries_OEM.bin";
			#endif // defined(DEF_A800).
		#endif // defined(DEF_EagleBKCus).
	#endif // defined(DEF_EagleZpCus).
#else	// ZeusMLK.
char *cp_oemDefaultFileName = "ZEUSMLK_OEM.bin";
#endif	// DEF_Zeus5BkCus or DEF_ZEUSS_CUS or DEF_HeliosCus or DEF_MonoCus or DEF_ZEUSS_ZX0_CUS.
/*================================================================================================
［機能］	OEMファイル名更新.
--------------------------------------------------------------------------------------------------
［引数］	なし.
［戻値］	なし.
［詳細］	A800レス（EagleX/EagleBKX/EagleBKZ）用は、egl/eglbビルドとなるため
			OEMバイナリのファイル名を参照前に切り替える.
 [履歴].
			2021/07/09 SKy		A800レス FWダウンロード切替対応.
			2022/12/20 Sky		A800レス A800装着判定処理追加対応(SPI-Flashから取得).
			2023/04/28 Sky		[IT6_2.3] 制御構成変更No.18 OEM名称カスタマイズデータのファイル名称変更.
================================================================================================*/
void THRF_UpdateOEMDefaultFileName( void )
{
	if (TYPE_ModelTypeIII == THRG_GetModelType() )
	{
#if defined(DEF_SparrowAIOSFPCus)	// Sparrow AIO/SFP.
	#if defined( DEF_SparrowBKCus )
		cp_oemDefaultFileName = "bh4701iSeries_OEM.bin";		// SparrowBK SFP.
	#else
		cp_oemDefaultFileName = "bhC3321iSeries_OEM.bin";
	#endif // defined( DEF_SparrowBKCus ).
#elif defined(DEF_SparrowCus)	// Sparrow MFP.	
	#if defined( DEF_SparrowBKCus )
		cp_oemDefaultFileName = "bh4751iSeries_OEM.bin";		// SparrowBK MFP.
	#else
		cp_oemDefaultFileName = "bhC4051iSeries_OEM.bin";
	#endif // defined( DEF_SparrowBKCus ).
#elif defined(DEF_EagleCus)	&& !defined(DEF_HeliosMLKCus) && !defined(DEF_MinervaSSBKCus)
	#if defined(DEF_EagleZpCus )	// EagleZ+.
		#if defined(DEF_EagleZBKpCus)		// EagleBkZ+.
			cp_oemDefaultFileName = "bh751iSeries_OEM.bin";
		#else // defined(DEF_EagleZBKpCus).
			cp_oemDefaultFileName = "bhC751iSeries_OEM.bin";
		#endif // defined(DEF_EagleZBKpCus).
	#else // defined(DEF_EagleZpCus).
		#if !defined(DEF_A800)
			#if defined(DEF_EagleBKCus)	// EagleBK.
				if( THRF_IsA800LessModel_fromSPIFlash() == True )
				{
					cp_oemDefaultFileName = "bh651iSeries_OEM.bin";     // EagleBKX/Z用定義.
				}
				else
				{
					cp_oemDefaultFileName = "bh361iSeries_OEM.bin";
				}
			#else // defined(DEF_EagleBKCus).
				if( THRF_IsA800LessModel_fromSPIFlash() == True )
				{
					cp_oemDefaultFileName = "bhC651iSeries_OEM.bin";     // EagleBKX/Z用定義.
				}
				else
				{
					cp_oemDefaultFileName = "bhC361iSeries_OEM.bin";
				}
			#endif // defined(DEF_EagleBKCus).
		#else 
			#if defined(DEF_EagleBKCus)	// EagleBK.
				cp_oemDefaultFileName = "bh651iSeries_OEM.bin";
			#else
				cp_oemDefaultFileName = "bhC651iSeries_OEM.bin";
			#endif // defined(DEF_EagleBKCus).
		#endif // !defined(DEF_A800).	
	#endif // defined(DEF_EagleZpCus).
#endif	// defined(DEF_SparrowAIOSFPCus).		
	}
	else
	{
#if defined(DEF_EagleCus) && !defined(DEF_HeliosMLKCus) && !defined(DEF_MinervaSSBKCus)
  #if !defined(DEF_EagleZpCus)
    #if !defined(DEF_A800)	// Eagle_L/EagleBK_L.
      #if defined(DEF_EagleBKCus)	// EagleBK.
		if( THRF_IsA800LessModel_fromSPIFlash() == True )
		{
			cp_oemDefaultFileName = "bh650iSeries_OEM.bin";     // EagleBKX/Z用定義.
		}
      #else // defined(DEF_EagleBKCus).
		if( THRF_IsA800LessModel_fromSPIFlash() == True )
		{
			cp_oemDefaultFileName = "bhC650iSeries_OEM.bin";    // EagleX用定義
		}
      #endif // defined(DEF_EagleBKCus).
    #endif // defined(DEF_A800).
  #endif // !defined(DEF_EagleZpCus).
#endif // defined(DEF_EagleCus) && !defined(DEF_HeliosMLKCus)
	}
}

#ifdef TORNADO
#define	DEF_PATH_MAX	_PARM_PATH_MAX
#else
#define	DEF_PATH_MAX	1023
#endif
#define DEF_OEMDRIVER_PATHMAX	(54)			// OEMドライバ名称カスタマイズ インストール元パス最大値.

#define DEF_OEM_DRIVER_CUSTOMIZE_DATA_BIN_SIZE	(256 * 1024)	// OEMドライバ名称カスタマイズbinファイルサイズ.
/*================================================================================================
［機能］	USBメモリにOEMドライバ名称カスタマイズデータがあるかの確認
--------------------------------------------------------------------------------------------------
［引数］	char *cp_path：OEMドライバ名称カスタマイズデータへのフルパス
［戻値］	ファイル存在の有無    True：ファイル有り   False：ファイル無し
［詳細］	2009/04/23 CQ317413 河村・伊藤史 - OEMドライバ名称カスタマイズ対応.
			2011/07/18 CQ 柄崎 Linux化、Flashリードからファイルリードに.
			2013/05/10 fukuhara 静的解析対応.
			2021/02/04 sky SYSC_GetBulkDeviceNames()関数を用いて外部USBメモリのパス取得するように変更.
			2021/07/09 SKy		A800レス FWダウンロード切替対応.
［ｳｫｰｸｽﾙｰ］	未実施
================================================================================================*/
BOOL isExistOemData( char *cp_path )
{
#ifdef TORNADO		// 実機.
	FILE*		fp							= NULL;
	uint64_t	l_OemBinSize				= 0;		// 2017/01/23 uint32_tからuint64_tに変更.
	char		c_OemPath[THRD_NameLenMax]	= { 0 };
	char*		psc_UsbPath[THRD_UsbNum ]	= { NULL };	// 外部USBメモリのパス取得用.
	ushort		us_DevNum;								// デバイス数.

	if( cp_path ) {
		if( strlen( cp_path ) > DEF_OEMDRIVER_PATHMAX ){	// 上限チェック.
			printf("isExistOemData:cp_path size Over\n");
			return FALSE;
		}
	}
	if( cp_OemFilePath == NULL ){
		printf("cp_OemFilePath:malloc\n");
		cp_OemFilePath = (char *)malloc( THRD_UsbPathLen );
		if (cp_OemFilePath == NULL) {
			printf("isExistOemData cp_OemFilePath NULL!!\n");
			return FALSE;
		}
	}
	// 外部USBメモリのパスを取得.
	/* downloadOemDataで使用 */
	psc_UsbPath[0] = cp_OemFilePath;
	us_DevNum = ::SYSC_GetBulkDeviceNames( psc_UsbPath, THRD_UsbNum );	// 最初のデバイス名のみ取得.
	if( us_DevNum < 1 ) {									// デバイスが取得できない場合はエラー.
		printf( "isExistOemData:USB device doesn't exist\n" );
		free( cp_OemFilePath );
		cp_OemFilePath = NULL;
		return FALSE;
	}
	strncat( cp_OemFilePath, "/", THRD_UsbPathLen );
	// BINファイルの確認.
	strcpy( c_OemPath, cp_OemFilePath );
	if( cp_path ) {
		strcat( c_OemPath, cp_path );
	} else {
		THRF_UpdateOEMDefaultFileName();	// OEMファイル名更新.
		strcat( c_OemPath, cp_oemDefaultFileName );
	}
	fp = SYSF_fopen( c_OemPath, "rb" );
	if( fp == NULL ) {
		printf("isExistOemData:Can't open file=%s\n", c_OemPath );
		free( cp_OemFilePath );
		cp_OemFilePath = NULL;
		return FALSE;		// ファイルオープン不可.
	}

	// バイナリーサイズ計算 終端へ移動.
	if( fseek( fp, 0L, SEEK_END ) != OK ){
		printf("isExistOemData:Can't seek End\n");
		SYSF_fclose( fp );
		free( cp_OemFilePath );
		cp_OemFilePath = NULL;
		return FALSE;
	}
	// 現在地を求める(=サイズ).
	l_OemBinSize = ftell(fp);
	if( l_OemBinSize != DEF_OEM_DRIVER_CUSTOMIZE_DATA_BIN_SIZE ){
		printf("isExistOemData:file size NG %"PRId32"\n", l_OemBinSize);
		SYSF_fclose( fp );
		free( cp_OemFilePath );
		cp_OemFilePath = NULL;
		return FALSE;
	}
	// ファイルクローズ.
	SYSF_fclose( fp );
	return TRUE;
#else
	return FALSE;
#endif	// #ifdef TORNADO.
}

#endif // DIV_MFP_LIB2
// 2010/11/15 【LocalUIプロセス化】ライブラリ化（ライブラリ対応）.
/*================================================================================================
［機能］	USBメモリからOEMドライバ名称カスタマイズデータをインストール
--------------------------------------------------------------------------------------------------
［引数］	char *cp_path：OEMドライバ名称カスタマイズデータへのUSBデバイスからの相対パス
［戻値］	インストール結果    True：成功   False：失敗
［詳細］	2009/04/23 CQ317413 河村・伊藤史 - OEMドライバ名称カスタマイズ対応.
			2015/09/17 yamashita MinervaSBK制御構成変更No.70 OEMデータのダウンロード対応.
			2021/03/16 Sky       引数のファイル名がデフォルト名と同一だった場合にエラーとなる不具合解消.
			2021/07/09 SKy		A800レス FWダウンロード切替対応.
			2021/10/11 SKy		MinervaSSBK FW静的解析対応.
［ｳｫｰｸｽﾙｰ］	未実施
================================================================================================*/
BOOL downloadOemData( char *cp_path )
{
#ifdef TORNADO		// 実機.
	char	c_cmdName[THRD_CmdNameLen] = {0};
	int32_t		ret=0;
	
	if( cp_path ) {
		if( strlen( cp_path ) > DEF_OEMDRIVER_PATHMAX ){	// 上限チェック.
			printf("isExistOemData:cp_path size Over\n" );
			return FALSE;
		}
	}
	if( cp_OemFilePath == NULL ){
		printf("downloadOemData:Not exit isExistOemData()\n");
		return FALSE;
	}

	THRF_UpdateOEMDefaultFileName();	// OEMファイル名更新.

	/* /sdirのファイルを削除 */
	sprintf( c_cmdName, "rm /sdir/%s", cp_oemDefaultFileName );
	system( c_cmdName );
	// ファイルパス作成.
	if( cp_path ) {
		/* /sdirへコピー */
		sprintf( c_cmdName, "cp %s%s /sdir", cp_OemFilePath, cp_path );
	} else {
		/* /sdirへコピー */
		sprintf( c_cmdName, "cp %s%s /sdir", cp_OemFilePath, cp_oemDefaultFileName );
	}
	ret = system( c_cmdName );
	if( WIFEXITED(ret) ){	// 子プロセスが正常終了したか確認.
		if( WEXITSTATUS(ret) != 0 ){	
			SYSF_logMsg("downloadOemData cp error(%"PRId32")\n", WEXITSTATUS(ret));
			return FALSE;
		}
		// ディスクキャッシュをフラッシュ.
		THRF_DoSync();
	}else{
		SYSF_logMsg("downloadOemData Child process cp error(%"PRId32")\n", WIFEXITED(ret));
		return FALSE;
	}
	if( cp_path ) {
		char	*psc_Search = NULL;
		//ファイル名のみを指定する為、文字列内から'/'を後ろから検索する.
		psc_Search = strrchr(cp_path, '/');
		if(psc_Search == NULL){
			psc_Search = cp_path;
		}else{
			psc_Search++;	//'/'の位置から1文字分ずらす.
		}
		snprintf( c_cmdName, THRD_CmdNameLen ,"mv /sdir/%s /sdir/%s", psc_Search, cp_oemDefaultFileName );

		// USBメモリ内のOEM名称カスタマイズデータのファイル名がデフォルト名称の場合実施しないようにする.
		if( strncmp( psc_Search, cp_oemDefaultFileName, strlen(cp_oemDefaultFileName)+1 ) != 0 ){
			ret = system( c_cmdName );
			if( WIFEXITED(ret) ){	// 子プロセスが正常終了したか確認.
				if( WEXITSTATUS(ret) != 0 ){	
					SYSF_logMsg("downloadOemData mv error(%"PRId32")\n", WEXITSTATUS(ret));
					return FALSE;
				}
				// ディスクキャッシュをフラッシュ.
				THRF_DoSync();
			}else{
				SYSF_logMsg("downloadOemData Child process mv error(%"PRId32")\n", WIFEXITED(ret));
				return FALSE;
			}
		}
	}
	return TRUE;
#else
	return FALSE;
#endif	// #ifdef TORNADO.
}

/*================================================================================================
［機能］	OEMドライバ名称カスタマイズデータを削除
--------------------------------------------------------------------------------------------------
［引数］	void
［戻値］	削除結果    TRUE：成功   FALSE：失敗
［詳細］	2012/09/20 堀下		OEMドライバ名称カスタマイズデータ削除対応.
			2021/07/09 SKy		A800レス FWダウンロード切替対応.
［ｳｫｰｸｽﾙｰ］	
================================================================================================*/
BOOL removeOemData( void )
{
#ifdef TORNADO		// 実機.

	char c_cmdName[ 64 + 1 ] = { 0 };
	THRF_UpdateOEMDefaultFileName();	// OEMファイル名更新.

	/* /sdirのファイルを削除 */
	sprintf( c_cmdName, "rm -f /sdir/%s", cp_oemDefaultFileName );
	int32_t ret = system( c_cmdName );
	if( ret != 0 ) {
		printf( "remove OEM data: FALSE\n" );
		return FALSE;
	}
	ret = system( "sync" );
	if( ret != 0 ) {
		printf( "sync: FALSE\n" );
		return FALSE;
	}
	return TRUE;

#else

	return FALSE;
	
#endif	// #ifdef TORNADO.
}


/*================================================================================================
［機能］	HDD論理フォーマット
--------------------------------------------------------------------------------------------------
［引数］	なし
［戻値］	なし
［詳細］	ラインモードであればHDD論理フォーマットを行う。
［ｳｫｰｸｽﾙｰ］	
================================================================================================*/
#include "system/thr/thrcom/THRC_MEMFunc.h"
#include "system/sys/SYSC_SysIf.h"
void logicformatHDD()
{
	IMGC_ImageMemoryPoolImpColorFileMemoryInstance& comMemoryManager = IMGC_ImageMemoryPoolImpColorFileMemorySingletonIf::newInstance();
	TYPE_HddStatus status;

#ifdef TORNADO		// 実機
	struct timeval now, tvdeadline;
	now.tv_sec = 0;
#endif	// #ifdef TORNADO.
	uint32_t delay;
	SYSC_HddInfoIf::newInstance( )->hddFormatNotice( );
	CFStart(FUNC_HD_FORMAT_LOGICAL);
#ifdef TORNADO		// 実機.
	if (gettimeofday(&now, NULL) < 0 ) {
		return;
	}
#endif	// #ifdef TORNADO.
	SYSF_taskDelay(10000);
	for ( delay = 0; ; delay +=100 ) {
#ifdef TORNADO		// 実機.
		if(gettimeofday(&tvdeadline, NULL) < 0) {
			return;
		}
		// 実行状態が100%かつフォーマットが300秒以内で完了
		if((CC_MEMFunctionSingletonIf::newInstance().ss_ExecPercent == 100) && (tvdeadline.tv_sec - now.tv_sec <= 1200)) {
			break;
		}
#else
		if((CC_MEMFunctionSingletonIf::newInstance().ss_ExecPercent == 100)) {
			break;
		}
#endif	// #ifdef TORNADO.
		if(delay > 800000) {
			break;
		}
		SYSF_taskDelay(20);		//20ms delay
	}

	status = comMemoryManager.getHDDStatus();
	if(status == TYPE_HddStatusFormated) {	// HDD使用可能状態
		printf("HDDformat OK.\n");
	} else {								// HDD未フォーマット状態
		printf("HDDformat NG.\n");
	}

	return;
}


/*================================================================================================
［機能］	リブート
--------------------------------------------------------------------------------------------------
［引数］	なし
［戻値］	なし
［詳細］	ラインモードであればリブート処理を行う。
［ｳｫｰｸｽﾙｰ］	
================================================================================================*/
#include "system/sys/SystemServiceFunction/SystemServiceFunctionC/SYSC_SystemServiceFunctionColor.h"
void reboot()
{
	SYSC_SystemServiceFunctionColor* sys_serviceFuncColor = SYSC_SystemServiceFunctionColor::newInstance();
	
	sys_serviceFuncColor->reboot();

	return;
}

void ans_totalclear(Bool b_Ok);

/*================================================================================================
［機能］	トータルクリア
--------------------------------------------------------------------------------------------------
［引数］	なし
［戻値］	なし
［詳細］	
［ｳｫｰｸｽﾙｰ］	
================================================================================================*/
#include "system/sys/SystemInfo/SystemInfoC/SYSC_SystemInfoColor.h"
void totalclear()
{
	SYSC_SystemInfo* sysc_systeminfo = SYSC_SystemInfo::newInstance();
	if( checkLineMode() == False ){		// ラインモードであるかの確認.
		return;
	} else {
		sysc_systeminfo->req_totalClear(ans_totalclear);
	}
	return;
}

void ans_totalclear(Bool b_Ok)
{
	if(b_Ok == True) {
		printf("totalClear OK.\n");
	} else {
		printf("totalClear NG.\n");
	}
	return;
}

/*================================================================================================
［機能］	トータルクリア実行判定コマンド.
--------------------------------------------------------------------------------------------------.
［引数］	なし.
［戻値］	なし.
［詳細］	トータルクリア実行有無について、ターミナルに出力する.
			－－－－－－－－－－－－－－－－－－－－－－－－－－－－－.
			状態　　　　　　：　戻り文字列.
			－－－－－－－－：－－－－－－－－－－－－－－－－－－－－.
			判定準備中　　　：　"Not Ready ExecTotalClear".
			totalclear必要　：　"Need TotalClear".
			totalclear不要　：　"Unnecessary TotalClear".
			－－－－－－－－－－－－－－－－－－－－－－－－－－－－－.
［履歴］	.
			2015/04/28	井柳	[Poseidon量産問題]No7 TotalClearの実行有無判定対応(新規作成).
================================================================================================*/
void IsExecTotalClear()
{
	switch( uc_ExecTotalClearState ){
	case THRE_ExecTotalClearState_NeedTotalClear:			// totalclear必要.
		printf( "\n\nNeed TotalClear\n\n" );
		break;
	case THRE_ExecTotalClearState_UnnecessaryTotalClear:	// totalclear不要.
		printf( "\n\nUnnecessary TotalClear\n\n" );
		break;
	case THRE_ExecTotalClearState_NotReady:					// 判定準備中.
	default:
		printf( "\n\nNot Ready ExecTotalClear\n\n" );
		break;
	}
}

/*===============================================================================================.
［機能］	totalclear実行指定設定.
-------------------------------------------------------------------------------------------------.
［引数］	Bool	bl_Exec		totalclear実行指定（True：必要／False：不要）.
［戻値］	なし.
［詳細］	totalclear実行指定の設定.
［履歴］	.
			2015/04/28	井柳	[Poseidon量産問題]No7 TotalClearの実行有無判定対応(新規作成).
================================================================================================*/
void THRG_setIsExecTotalClear( Bool bl_Exec )
{
	if( bl_Exec == True ){
		uc_ExecTotalClearState = THRE_ExecTotalClearState_NeedTotalClear;			// totalclear必要.
	} else {
		uc_ExecTotalClearState = THRE_ExecTotalClearState_UnnecessaryTotalClear;	// totalclear不要.
	}
}

void ans_totalCounterClear(Bool b_Ok);

/*================================================================================================
［機能］	トータルカウンタクリア.
--------------------------------------------------------------------------------------------------
［引数］	なし
［戻値］	0:成功、-1:失敗.
［詳細］	温度/湿度画面の情報を表示する.
［ｳｫｰｸｽﾙｰ］	未実施
--------------------------------------------------------------------------------------------------
 [履歴]
 '11/07/01 karasaki	 新規作成
 '16/12/20 SKy井柳	 【IT5_4.0】制御構成変更No42「totalcounterclearコマンドの改善」A4制限解除.
================================================================================================*/
void totalCounterClear( void ){

	SYSC_SystemInfo* sysc_systeminfo = SYSC_SystemInfo::newInstance();
	if( checkLineMode() == False ){		// ラインモードであるかの確認.
		return;
	} else {
		// Faxジャーナル・Scanジャーナルの削除.
		deleteFaxScanJournal_Internal();

		sysc_systeminfo->req_totalCounterClear(ans_totalCounterClear);
// 2022/03/30 生産RINNE対応 Sky start.
		SYSD_WEBAPI_LINEMODE_WAIT
// 2022/03/30 生産RINNE対応 Sky end.
	}
	return;
}

void ans_totalCounterClear(Bool b_Ok)
{
	if(b_Ok == True) {
		printf("totalCounterClear OK.\n");
	} else {
		printf("totalCounterClear NG.\n");
	}

// 2022/03/30 生産RINNE対応 Sky start.
	SYSD_WEBAPI_LINEMODE_WAKEUP
// 2022/03/30 生産RINNE対応 Sky end.

	return;
}

/*================================================================================================
［機能］	SW11設定
--------------------------------------------------------------------------------------------------
［引数］	uc_disp: 1 Generic
                     2 OWN
			uc_set : 0 解除
                     1 設定
［戻値］	なし
［詳細］	コマンド実行により、DipSW11の設定を実行する。
			setDipsw11 1 1	起動画面を強制的にGenericに切り替える。
			setDipsw11 1 0	起動画面を強制的にGenericに解除する。
			setDipsw11 1 1	起動画面を強制的にOWNに切り替える。
			setDipsw11 1 1	起動画面を強制的にOWNに解除する。
［ｳｫｰｸｽﾙｰ］	
================================================================================================*/
void setDipsw11( uchar uc_disp, uchar uc_set )
{
	APIC_SoftDipSw* pc_SoftDipSw = APIC_SoftDipSw::newInstance();
	
	if( (uc_disp < 0x01 || uc_disp > 0x02) || (uc_set < 0 || uc_set > 1) ){	// パラメータチェック
		printf("setDipsw11 parameter NG.\n");
		return;
	}

	if(uc_disp == 0x01) {	// Generic
		pc_SoftDipSw->setSoftDipSwBit( 2, 5, uc_set );
		if( pc_SoftDipSw->getSoftDipSwBit( 2, 5 ) != uc_set ) {	// setSoftDipSwBit()で期待結果が得られているかどうかのチェック
			printf("setDipsw11 NG.\n");
			return;
		}
	} else if(uc_disp == 0x02) {	// OWN
		pc_SoftDipSw->setSoftDipSwBit( 3, 3, uc_set );
		if( pc_SoftDipSw->getSoftDipSwBit( 3, 3 ) != uc_set ) {	// setSoftDipSwBit()で期待結果が得られているかどうかのチェック
			printf("setDipsw11 NG.\n");
			return;
		}
	}

	printf("setDipsw11 OK.\n");

	return;
}

/*================================================================================================
［機能］	SW11表示
--------------------------------------------------------------------------------------------------
［引数］	なし
［戻値］	なし
［詳細］	ラインモードかチェックし、ラインモードであればDIPSW11の値を表示する。
［ｳｫｰｸｽﾙｰ］	
================================================================================================*/
void dspDipsw11()
{
	APIC_SoftDipSw* pc_SoftDipSw = APIC_SoftDipSw::newInstance();

	if ((pc_SoftDipSw->getSoftDipSwBit(2, 5) == True) && (pc_SoftDipSw->getSoftDipSwBit(3, 3) == True)) {
		printf( "DIPSW11 : 0x03\n");
	} else if (pc_SoftDipSw->getSoftDipSwBit(2, 5) == True) {
		printf( "DIPSW11 : 0x01\n");
	} else if (pc_SoftDipSw->getSoftDipSwBit(3, 3) == True){
		printf( "DIPSW11 : 0x02\n");
	} else {
		printf( "DIPSW11 : 0x00\n");
	}

	return;
}

/*================================================================================================
［機能］	SW11設定
--------------------------------------------------------------------------------------------------
［引数］	uc_set : 0 解除
                     1 Generic
                     2 OWN
［戻値］	なし
［詳細］	コマンド実行により、DipSW11の設定を実行する。
			setDipsw11_Ver2 0	Generic,OWNを解除する。
			setDipsw11_Ver2 1	起動画面を強制的にGenericに切り替える。
			setDipsw11_Ver2 2	起動画面を強制的にOWNに切り替える。
［ｳｫｰｸｽﾙｰ］	
================================================================================================*/
void setDipsw11_Ver2( uchar uc_set )
{
	APIC_SoftDipSw* pc_SoftDipSw = APIC_SoftDipSw::newInstance();
	int	LastValue = 0;

	if( uc_set > 2 ){	// パラメータチェック.
		printf(" setDipsw11_Ver2 parameterNG\n");
		return;
	}
	if ((pc_SoftDipSw->getSoftDipSwBit(2, 5) == True) && (pc_SoftDipSw->getSoftDipSwBit(3, 3) == True)) {
		LastValue = 3;
	} else if (pc_SoftDipSw->getSoftDipSwBit(2, 5) == True) {
		LastValue = 1;
	} else if (pc_SoftDipSw->getSoftDipSwBit(3, 3) == True){
		LastValue = 2;
	} else {
		LastValue = 0;
	}

	if(uc_set == 0x00) {	// OFF.
		pc_SoftDipSw->setSoftDipSwBit( 2, 5, 0 );			//Generic off.
		if( pc_SoftDipSw->getSoftDipSwBit( 2, 5 ) != 0 ) {	// setSoftDipSwBit()で期待結果が得られているかどうかのチェック.
			printf(" setDipsw11_Ver2 NG\n");
			return;
		}
		pc_SoftDipSw->setSoftDipSwBit( 3, 3, 0 );			//OWN off.
		if( pc_SoftDipSw->getSoftDipSwBit( 3, 3 ) != 0 ) {	// setSoftDipSwBit()で期待結果が得られているかどうかのチェック.
			printf(" setDipsw11_Ver2 NG\n");
			return;
		}
	} else if(uc_set == 0x01) {	// Generic.
		pc_SoftDipSw->setSoftDipSwBit( 2, 5, 1 );
		if( pc_SoftDipSw->getSoftDipSwBit( 2, 5 ) != 1 ) {	// setSoftDipSwBit()で期待結果が得られているかどうかのチェック.
			printf(" setDipsw11_Ver2 NG\n");
			return;
		}
		pc_SoftDipSw->setSoftDipSwBit( 3, 3, 0 );			//OWN off.
		if( pc_SoftDipSw->getSoftDipSwBit( 3, 3 ) != 0 ) {	// setSoftDipSwBit()で期待結果が得られているかどうかのチェック.
			printf(" setDipsw11_Ver2 NG\n");
			return;
		}
	} else if(uc_set == 0x02) {	// OWN.
		pc_SoftDipSw->setSoftDipSwBit( 3, 3, 1 );
		if( pc_SoftDipSw->getSoftDipSwBit( 3, 3 ) != 1 ) {	// setSoftDipSwBit()で期待結果が得られているかどうかのチェック.
			printf(" setDipsw11_Ver2 NG\n");
			return;
		}
		pc_SoftDipSw->setSoftDipSwBit( 2, 5, 0 );			//Generic off.
		if( pc_SoftDipSw->getSoftDipSwBit( 2, 5 ) != 0 ) {	// setSoftDipSwBit()で期待結果が得られているかどうかのチェック.
			printf(" setDipsw11_Ver2 NG\n");
			return;
		}
	}

	printf(" Last Value = %d\n",LastValue);
	printf(" New  Value = %d\n",uc_set);
	return;
}

/*================================================================================================
［機能］	SW11表示
--------------------------------------------------------------------------------------------------
［引数］	なし
［戻値］	なし
［詳細］	ラインモードかチェックし、ラインモードであればDIPSW11の値を表示する。
［ｳｫｰｸｽﾙｰ］	
================================================================================================*/
void dspDipsw11_Ver2()
{
	APIC_SoftDipSw* pc_SoftDipSw = APIC_SoftDipSw::newInstance();

	if ((pc_SoftDipSw->getSoftDipSwBit(2, 5) == True) && (pc_SoftDipSw->getSoftDipSwBit(3, 3) == True)) {
		printf( " Last Value = 3\n");
	} else if (pc_SoftDipSw->getSoftDipSwBit(2, 5) == True) {
		printf( " Last Value = 1\n");
	} else if (pc_SoftDipSw->getSoftDipSwBit(3, 3) == True){
		printf( " Last Value = 2\n");
	} else {
		printf( " Last Value = 0\n");
	}

	return;
}


/*================================================================================================
［機能］	SW25取得
--------------------------------------------------------------------------------------------------
［引数］	なし
［戻値］	なし
［詳細］	
［ｳｫｰｸｽﾙｰ］	
================================================================================================*/
void dspDipsw25( void )
{
	APIC_SoftDipSw* pc_SoftDipSw = APIC_SoftDipSw::newInstance();

	if( NULL == pc_SoftDipSw ){
		printf("SystemErr\n");
		return;	
	}
	printf("Last Value = 0x%02x\n",pc_SoftDipSw->getDipSw25());

}
/*================================================================================================
［機能］	SW25設定
--------------------------------------------------------------------------------------------------
［引数］	uc_set	パラメータ
［戻値］	なし
［詳細］	
［ｳｫｰｸｽﾙｰ］	
================================================================================================*/
void setDipsw25( uchar uc_set )
{

	static	bool	Chg_Table[11][4] = {
	/* 0x01*/	{False,False,False,False},	// 0x00に設定.
	/* 0x02*/	{True, False,False,False},	// 0x10に設定.
	/* 0x03*/	{False,True, False,False},	// 0x20に設定.
	/* 0x04*/	{True ,True, False,False},	// 0x30に設定.
	/* 0x05*/	{False,False,True ,False},	// 0x40に設定.
	/* 0x06*/	{True ,False,True ,False},	// 0x50に設定.
	/* 0x07*/	{False,True ,True ,False},	// 0x60に設定.
	/* 0x08*/	{True ,True ,True ,False},	// 0x70に設定.
	/* 0x09*/	{False,False,False,True },	// 0x80に設定.
	/* 0x0A*/	{True ,False,False,True },	// 0x90に設定.
	/* 0x0B*/	{False,True ,False,True }	// 0xA0に設定.
	};
	unsigned char i;
	uchar	ucLastValue = 0;
	APIC_SoftDipSw* pc_SoftDipSw = APIC_SoftDipSw::newInstance();

	if( NULL == pc_SoftDipSw ){
		printf("SystemErr\n");
		return;	
	}

	if( uc_set > 0x0B ){		// パラメータチェック.
		printf("setDipsw25 parameter NG\n");
		return;
	}
	ucLastValue = pc_SoftDipSw->getDipSw25();
	
	if (0 == uc_set) {
		pc_SoftDipSw->setDipSw25();
	} else {
		for ( i = 0; i < 4; i++ ) {
			pc_SoftDipSw->setSoftDipSwBit( 19, i+1, Chg_Table[uc_set-1][i] );
			if( pc_SoftDipSw->getSoftDipSwBit( 19, i+1 ) != Chg_Table[uc_set-1][i] )
			{
				printf("setDipsw25 NG\n");
				return;							// エラー発生のため途中で処理中止.
			}
		}
	}
	printf("setDipsw25\n");
	printf(" Last Value = 0x%02x\n",ucLastValue);
	printf(" New Value = 0x%02x\n",pc_SoftDipSw->getDipSw25());
}

/*================================================================================================
［機能］	ターミナルコマンド（ 静電TPキャリブレーション実施状態取得 ）
--------------------------------------------------------------------------------------------------
［引数］	なし
［戻値］	なし 
［詳細］	静電TPキャリブレーション実施状態をNVRAMから読み出し、ログ出力取得
［ｳｫｰｸｽﾙｰ］	未実施
--------------------------------------------------------------------------------------------------
 [履歴]
 '11/05/17 horishita	 新規作成
================================================================================================*/
void GetCalibStatus( void )
{
	NVDE_CalibrationExecStatus ret = NVDF_getStaticElectricCalibrationExecStatus();		// NVRAMから静電TPキャリブレーション実施状態を取得.
	if( NVDE_CalibrationExecStatusOn  == ret )
	{
		printf( "Electric calibration has been executed. \n" );							// 実施済み.
	}
	else
	{
		printf( "Electric calibration has not been executed yet. \n" );					// 未実施.
	}
}

/*================================================================================================
［機能］	[内部] CFStart呼び出し関数
--------------------------------------------------------------------------------------------------
［引数］	ss_FunctionCode: CFStartのファンクションコード
			waitSec : タイムアウト値（秒数）
［戻値］	なし 
［詳細］	CFStartを呼び出し、結果が分かる or タイムアウト までdelayする
［ｳｫｰｸｽﾙｰ］	未実施
--------------------------------------------------------------------------------------------------
 [履歴]
 '11/07/01 Gonsho    新規作成
================================================================================================*/
static Bool callCFStart(short ss_FunctionCode, int32_t waitSec)
{
	// メモリR/Wチェック 開始.
	CFStart(ss_FunctionCode);
	
	// CFが終了するまで待つ.
	uint32_t delay = 0;
	int32_t waitCnt = waitSec * 10;	// waitSec -> ミリ秒、ミリ秒 / delay : waitSec * 1000 / 100
	
	while ( ( CC_MEMFunctionSingletonIf::newInstance().ss_ExecPercent != 100 ) 
			&& ( CC_MEMFunctionSingletonIf::newInstance().ec_MEMFuncStatus == TYPE_MemFuncStatusNOWExec ) 
			&& ( delay < (uint32_t)waitCnt ) )
	{
		SYSF_taskDelay(20);		//20ms delay -> 実際100ms程delayしている
		delay++;
	}
	
	Bool b_Rslt = False;
	if ( ( CC_MEMFunctionSingletonIf::newInstance().ec_MEMFuncStatus == TYPE_MemFuncStatusOKEnd ) && ( delay < (uint32_t)waitCnt ) )
	{
		b_Rslt = True;
	}
	
	return b_Rslt;
}

#define PrintCFRslt(s,rslt)	printf("%s : %s\n", s, ( rslt == True ? "OK":"NG" ))

/*================================================================================================
［機能］	ターミナルコマンド （ファンクションチェック：メモリR/Wチェック）
--------------------------------------------------------------------------------------------------
［引数］	なし 
［戻値］	なし 
［詳細］	ファンクションチェック：メモリR/Wチェックの実施、ログ出力
［ｳｫｰｸｽﾙｰ］	未実施
--------------------------------------------------------------------------------------------------
 [履歴]
 '11/07/01 Gonsho    新規作成
================================================================================================*/
void CheckMemoryRW()
{
	// メモリR/Wチェック 開始.
	Bool b_Rslt = callCFStart(FUNC_MEMORY_RW_CHECK1_MODE, 120);
	PrintCFRslt("Memory R/W Check", b_Rslt);
}

/*================================================================================================
［機能］	ターミナルコマンド （ファンクションチェック：JBIG圧縮・伸張）
--------------------------------------------------------------------------------------------------
［引数］	なし 
［戻値］	なし 
［詳細］	ファンクションチェック：JBIG圧縮・伸張の実施、ログ出力
［ｳｫｰｸｽﾙｰ］	未実施
--------------------------------------------------------------------------------------------------
 [履歴]
 '11/07/01 Gonsho    新規作成
================================================================================================*/
void CheckJbigCmpExp()
{
	// JBIG 圧縮・伸張チェック 開始.
	Bool b_Rslt = callCFStart(FUNC_JBIG_CMPEXP_CHECK_MODE, 120);
	PrintCFRslt("JBIG compress & expand Check", b_Rslt);
}

/*================================================================================================
［機能］	ターミナルコマンド （ファンクションチェック：メモリバスチェック（スキャナ->メモリ））
--------------------------------------------------------------------------------------------------
［引数］	なし 
［戻値］	なし 
［詳細］	ファンクションチェック：メモリバスチェック（スキャナ->メモリ）、ログ出力
［ｳｫｰｸｽﾙｰ］	未実施
--------------------------------------------------------------------------------------------------
 [履歴]
 '11/07/01 Gonsho    新規作成
 '12/03/15 Gonsho    CQ:361725 遠隔操作AndroidPF DS機判定変更。DS基板装着判定からCommoAPIのDS機判定APIに変更.
================================================================================================*/
void CheckMemoryBusIrToMem()
{	
	// バスチェック（スキャナ->メモリ） 開始.
	Bool b_Rslt = callCFStart(FUNC_IR_MEM_BUS_CHECK_IR_TO_MEM, 120);
	if(CC_MEMFunctionSingletonIf::newInstance().s_IRMEMFuncStatus.ec_IRMEMFuncMSCStatus == TYPE_MemFuncStatusNGEnd || 
	   CC_MEMFunctionSingletonIf::newInstance().s_IRMEMFuncStatus.ec_IRMEMFuncIRStatus == TYPE_MemFuncStatusNGEnd) {
		b_Rslt = False;
	}
	if ( APIC_SystemInfo::newInstance()->isDSModel() == True ) {
		if(CC_MEMFunctionSingletonIf::newInstance().s_IRMEMFuncStatus.ec_IRDupMEMFuncMSCStatus == TYPE_MemFuncStatusNGEnd || 
		   CC_MEMFunctionSingletonIf::newInstance().s_IRMEMFuncStatus.ec_IRDupMEMFuncIRStatus == TYPE_MemFuncStatusNGEnd) {
			b_Rslt = False;
		}
	}
	PrintCFRslt("Memory Bus Check (Scanner -> Memory)", b_Rslt);
}

/*================================================================================================
［機能］	ターミナルコマンド （ファンクションチェック：メモリバスチェック（メモリ->プリンタ））
--------------------------------------------------------------------------------------------------
［引数］	なし 
［戻値］	なし 
［詳細］	ファンクションチェック：メモリバスチェック（メモリ->プリンタ）、ログ出力
［ｳｫｰｸｽﾙｰ］	未実施
--------------------------------------------------------------------------------------------------
 [履歴]
 '11/07/01 Gonsho    新規作成
================================================================================================*/
void CheckMemoryBusMemToPr()
{	
	// バスチェック（メモリ->プリンタ） 開始.
	Bool b_Rslt = callCFStart(FUNC_MEM_PR_BUS_CHECK_MEM_TO_PR, 120);
	PrintCFRslt("Memory Bus Check (Memory -> Printer)", b_Rslt);
}

/*================================================================================================
［機能］	ターミナルコマンド （マシン情報取得）
--------------------------------------------------------------------------------------------------
［引数］	なし 
［戻値］	なし 
［詳細］	マシン情報取得、ログ出力
［ｳｫｰｸｽﾙｰ］	未実施
--------------------------------------------------------------------------------------------------
 [履歴]
 '11/07/01 Gonsho    新規作成
  12/03/16 N.Kato    機種情報MinervaBK/ZeusMLK対応.
  12/08/30 Takatsuji ZesuBK対応.
  14/04/10 K.Yasue   制御構成変更No.213 Zeus5BK マシン情報取得ターミナルコマンド用対応.
  14/07/14 yamada    ZeusS 制御構成変更No.2 機種コード追加対応(machine : ZeusS / type : 2.5).
  14/11/25 furuichi  HeliosL 制御構成変更No.2 マシンタイプ情報取得追加対応(machine : HeliosL).
  15/02/26 Ooka      MinervaSBK 制御構成変更No.2 機種コード対応
  15/04/09 Sky山下   CQ510921 ワーニング対応.
  15/04/09 Sky山下   ZeusSZX0 制御構成変更No.65 機種コード対応.
  15/08/20 h.hashimoto ZeusSZX0 制御構成変更No.7 速度変更対応.
  16/01/19 Sky鈴木   ZeusSBK 制御構成変更No.3 機種コード対応.
  16/04/01 eSOL) K.Shiranami VenusMLK固有対応.
  16/11/21 Sky鈴木   IT5_4.1 制御構成変更No.3 機種コード対応.
  17/01/27 Sky鈴木   AR1012782 ZeusSBK2ndのZとXのTypeがUnknownで表示される対応.
  17/09/14 Sky松井   [Eagle] 制御構成変更No.3 機種コード対応.
  17/11/06 田川		 [Sparrow] 制御構成変更No.3 機種コード対応.
  18/05/08 Sky山岸   [DenebMLK] 制御構成変更No.3 機種コード対応.
  18/06/05 Sky小久保 [Eagle] 制御構成変更_No.89 生産コマンド対応 イコールへ変更,(値表示)の削除.
  19/04/23 Sky井柳 [EagleBK] 制御構成変更No.3 機種コード対応.
  19/05/13 Sky     [EagleZ+/BKZ+] 制御構成変更No.3 機種コード対応.
  19/07/09 Sky井柳 [SparrowBK/HeliosMLK] 制御構成変更No.3 機種コード対応.
  19/09/03 日立足立 [HeliosMLK] 制御構成変更No.48 HeliosMLKの処理を独立化.	
  19/11/15 Sky井柳 [HeliosMLK] 制御構成変更No.52 製品コンフィグ変更対応:処理統合.
  21/02/15 Sky       [MinervaSSBK] 制御構成変更No.3 機種コード対応.
  22/06/01 Sky		 [IT6_2.3]制御構成変更_No.6マシンタイプ対応.
================================================================================================*/
void GetMachineType()
{
#ifndef SIMULATOR
	// マシン情報取得開始
	char machine = -1, type = -1;
	THRF_getMachineType(&machine, &type);	

	if ( machine == -1 || type == -1 )	// 取得後の値が初期値のまま -> 失敗.
	{
		// 失敗.
		PrintCFRslt(__func__, False);
	}
	else
	{
		typedef struct {
			uchar				uc_Code;
			const char*			psc_String;
		} THRS_MACHINE_KIND;
		
		// 機種情報.
		const THRS_MACHINE_KIND		machineKind[]	= {	{ THRE_MACHINEINFO_INVALID,		THRD_DispUnknown	},		// 先頭は不明とする.
			{	THRE_MACHINEINFO_EAGLE		,		"Eagle"			},	// Eagle(0x10).
			{	THRE_MACHINEINFO_SPARROW	,		"Sparrow"		},	// Sparrow(0x11).
			{	THRE_MACHINEINFO_DENEBMLK	,		"DenebMLK"		},	// DenebMLK(0x12).
			{	THRE_MACHINEINFO_EAGLEBK	,		"EagleBK"		},	// EagleBK(0x13).
			{	THRE_MACHINEINFO_SPARROWBK	,		"SparrowBK"		},	// SparrowBK(0x14).
			{	THRE_MACHINEINFO_HELIOSMLK	,		"HeliosMLK"		},	// HeliosMLK(0x15).
			{	THRE_MACHINEINFO_MINERVASSBK,		"MinervaSSBK"	},	// MinervaSSBK(0x16).
			{	THRE_MACHINEINFO_EAGLE3		,		"EagleIII"		},	// Eagle3(0x17).
			{	THRE_MACHINEINFO_SPARROW3	,		"SparrowIII"	},	// Sparrow3(0x18).
			{	THRE_MACHINEINFO_EAGLEBK3	,		"EagleBKIII"	},	// EagleBK3(0x19).
			{	THRE_MACHINEINFO_SPARROWBK3	,		"SparrowBKIII"	},	// SparrowBK3(0x1A).
		};
		
		// 機種No.
		const THRS_MACHINE_KIND		typeKind[]		= {	{ THRE_MACHINENO_INVALID,		THRD_DispUnknown	},		// 先頭は不明とする.
			{	THRE_MACHINENO_0			,		"0"			},	// 0(0x0).
			{	THRE_MACHINENO_0_US			,		"0US"		},	// 7(0x7).
			{	THRE_MACHINENO_0_EU			,		"0EU"		},	// 8(0x8).
			{	THRE_MACHINENO_1			,		"1"			},	// 1(0x1).
			{	THRE_MACHINENO_2			,		"2"			},	// 2(0x2).
			{	THRE_MACHINENO_3			,		"3"			},	// 3(0x3).
			{	THRE_MACHINENO_4			,		"4"			},	// 4(0x4).
			{	THRE_MACHINENO_X			,		"X"			},	// X(0xA).
			{	THRE_MACHINENO_Z			,		"Z"			},	// Z(0xC).
			{	THRE_MACHINENO_2_5			,		"2.5"		},	// 2.5(0xD).
			{	THRE_MACHINENO_ZP			,		"Z+"		},	// Z+(0xE).
			{	THRE_MACHINENO_2AIO			,		"2AIO"		},	// Sparrow2AIO(0xA2).
			{	THRE_MACHINENO_1SFP			,		"1SFP"		},	// Sparrow1SFP(0xB1).
			{	THRE_MACHINENO_2SFP			,		"2SFP"		},	// Sparrow2SFP(0xB2).	
			{	THRE_MACHINENO_0SFP			,		"0SFP"		},	// Sparrow0SFP(0xB3).
			{	THRE_MACHINENO_2_5_CN		,		"2.5CN"		},	// HeliosMLK 2.5 (中国向け)(0xCD).
			{	THRE_MACHINENO_2_5_Other	,		"2.5ROW"	},	// HeliosMLK 2.5 (その他向け)(0xDD).
			{	THRE_MACHINENO_3_CN			,		"3CN"		},	// HeliosMLK 3 (中国向け)(0xC3).
			{	THRE_MACHINENO_2_CN			,		"2CNDF"		},	// HeliosMLK 2 (中国向け)(0xD2).
		};

		// 出力フォーマット.
		const char printFormat[] = "  Machine Name = %s\n"
								   "  Type         = %s\n";
		const THRS_MACHINE_KIND		*pMachineKind, *pTypeKind;
		int							si_Index;
		Bool						b_Rslt;

		// 機種情報の値を検索
		si_Index = ( sizeof(machineKind) / sizeof(THRS_MACHINE_KIND) ) - 1;
		pMachineKind = &machineKind[ si_Index ];
		for( ; si_Index > 0; si_Index--, pMachineKind-- ) {
			if ( pMachineKind->uc_Code == (uchar)machine ) {
				break;
			}
		}
		// 機種No.の値を検索
		si_Index = ( sizeof(typeKind) / sizeof(THRS_MACHINE_KIND) ) - 1;
		pTypeKind = &typeKind[ si_Index ];
		for( ; si_Index > 0; si_Index--, pTypeKind-- ) {
			if ( pTypeKind->uc_Code == (uchar)type ) {
				break;
			}
		}
		// 機種情報及び機種No.が、THRE_MACHINENO_INVALID以外であれば、b_RsltをTrueに、そうでなければFalseにする.
		if ( ( pMachineKind->uc_Code != THRE_MACHINEINFO_INVALID ) &&
			 ( pTypeKind->uc_Code != THRE_MACHINEINFO_INVALID ) ) {
			b_Rslt = True;
		} else {
			b_Rslt = False;
		}

		// 結果をコンソール出力.
		printf(printFormat, pMachineKind->psc_String,pTypeKind->psc_String);
		PrintCFRslt(__func__, b_Rslt);
	}
#endif
}
/*================================================================================================
［機能］	メモリ、HDDの情報を表示する。
--------------------------------------------------------------------------------------------------
［引数］	なし
［戻値］	0:成功、-1:失敗 
［詳細］	メモリ/HDD画面に表示されている情報を表示する.
［ｳｫｰｸｽﾙｰ］	未実施
--------------------------------------------------------------------------------------------------
 [履歴]
 '11/07/01 karasaki	 新規作成
================================================================================================*/
int32_t showMemoryHDDState( void ){
	APIC_MemoryInfo* pc_MemoryInfo = APIC_MemoryInfo::newInstance();
	APIC_HddInfo* pc_HddInfo = APIC_HddInfo::newInstance();
	IMGC_ImageMemoryPoolImpColorFileMemoryInstance& comMemoryManager = IMGC_ImageMemoryPoolImpColorFileMemorySingletonIf::newInstance();

	if( (pc_MemoryInfo == NULL) || (pc_HddInfo == NULL) ){
		return -1;
	}
	
	/* メモリ */
	uint32_t usMemSize = pc_MemoryInfo->getStorageMemorySize();
	
	/*	"HDD・合計サイズ */
	int32_t usHddSize1 = pc_HddInfo->getStorageHddSize();
	
	/* HDD・空き容量 */
	int32_t usUsedSize1 = pc_HddInfo->getStorageHddUsedSize();
	
	printf(" Memory         %6"PRId32" M\n", usMemSize );
#ifdef DEF_SupportSSD
	if(comMemoryManager.getStorageDeviceType() == THRE_StorageDeviceTypeHDD)
	{
		printf(" HDD TotalSize  %6"PRId32" M\n", usHddSize1 );
		printf(" HDD VacantArea %6"PRId32" M\n", usHddSize1 - usUsedSize1 );
	} else {
		printf(" SSD TotalSize  %6"PRId32" M\n", usHddSize1 );
		printf(" SSD VacantArea %6"PRId32" M\n", usHddSize1 - usUsedSize1 );

	}
#else
		printf(" HDD TotalSize  %6"PRId32" M\n", usHddSize1 );
		printf(" HDD VacantArea %6"PRId32" M\n", usHddSize1 - usUsedSize1 );
#endif
	return 0;
}

/*================================================================================================
［機能］	CE認証機能のON/OFF切り替え.
--------------------------------------------------------------------------------------------------
［引数］	0:無効、1:有効、なし:無効.
［戻値］	0:成功、-1:失敗.
［詳細］	CE認証が必要か設定を切り替える.
［ｳｫｰｸｽﾙｰ］	未実施
--------------------------------------------------------------------------------------------------
 [履歴]
 '11/07/01 karasaki	 新規作成
================================================================================================*/
#include "system/uac/APIC_UserAcount.h"
int32_t setCEAuth( int32_t para ){
	APIC_UserAuthentication* pc_UserAuthentication = APIC_UserAuthentication::newInstance();

	if( pc_UserAuthentication == NULL ){
		return -1;
	}
	if(  0 == para  ){
		pc_UserAuthentication->setEnableCEAuthentication( False );
	} else if(  1 == para  ){
		pc_UserAuthentication->setEnableCEAuthentication( True );
	} else {
		printf( "setCEAuth Failed.\n" );
		printf( "0:OFF\n" );
		printf( "1:ON\n" );
	}
	return 0;
}

/*================================================================================================
［機能］	CE認証機能の表示 ターミナルコマンド.
--------------------------------------------------------------------------------------------------.
［引数］	無し.
［戻値］	無し.
［詳細］	.
［ｳｫｰｸｽﾙｰ］	未実施.
--------------------------------------------------------------------------------------------------.
 [履歴]		.
 '15/10/26	大岡	[ZeusSZX0]制御構成変更No.80 Setコマンドに対応するGetコマンド追加対応. 
================================================================================================*/
void getCEAuth( void )
{
	APIC_UserAuthentication* pc_UserAuthentication = APIC_UserAuthentication::newInstance();
	if( pc_UserAuthentication == NULL ){
		// 初期化失敗時は「SytemErr」表示.
		printf("SystemErr\n");
		return;
	}

	// CE認証状態取得.
	if( pc_UserAuthentication->getEnableCEAuthentication() == True ){
		// CE認証有効.
		printf( "CEAuth is effective\n" );
	} else {
		// CE認証無効.
		printf( "CEAuth is invalid\n" );
	}
}

/*================================================================================================
［機能］	温度/湿度表示.
--------------------------------------------------------------------------------------------------
［引数］	なし
［戻値］	0:成功、-1:失敗.
［詳細］	温度/湿度画面の情報を表示する.
［ｳｫｰｸｽﾙｰ］	未実施
--------------------------------------------------------------------------------------------------
 [履歴]
 '11/07/01 karasaki	 新規作成
 '13/08/09 suzuki    静的解析対応3rd.ID:22021.
 '18/06/05 Sky小久保 Eagle 制御構成変更_No.89 生産コマンド対応 イコールへ変更.
================================================================================================*/
int32_t showTempHumidity( void ){

	APIC_PrinterServiceInfo* pc_PrinterServiceInfo = APIC_PrinterServiceInfo::newInstance();
	if( pc_PrinterServiceInfo == NULL ){
		return -1;
	}
	ushort sTemper = pc_PrinterServiceInfo->getPrinterInfo( TYPE_PrinterInfoPCTemperature );		// 機内温度.
	ushort sHumidityK = pc_PrinterServiceInfo->getPrinterInfo( TYPE_PrinterInfoHumidity );			// 機内湿度.
#if !defined(DEF_A4_Cus)	//-> 2016.01.29 Start.
	ushort sHumidityA = pc_PrinterServiceInfo->getPrinterInfo( TYPE_PrinterInfoAbsoluteHumidity );	// 絶対湿度.
#else	// defined(DEF_A4_Cus).
	ushort sHumidityPH = pc_PrinterServiceInfo->getPrinterInfo( TYPE_PrinterInfoPHTemperature );		// PH温度.
#endif	// !defined(DEF_A4_Cus) 2016.01.29 End.

	printf(" Temp-Inside     =%4d C\n", sTemper);
	printf(" Humiity         =%4d %%\n", sHumidityK );
#if !defined(DEF_A4_Cus)	//-> 2016.01.29 Start.
	printf(" Absolute Humiity=%4d\n", sHumidityA );
#else	// defined(DEF_A4_Cus).
	printf(" PH-Temp         :%4d C\n", sHumidityPH );	// VenusMLKはPH温度
#endif	//<- !defined(DEF_A4_Cus) 2016.01.29 End.
	return 0;
}

/*================================================================================================
［機能］	PDF/Aのバージョン取得.
--------------------------------------------------------------------------------------------------
［引数］	なし
［戻値］	なし.
［詳細］	PDF/Aのバージョンを取得/表示する.
［ｳｫｰｸｽﾙｰ］	未実施
--------------------------------------------------------------------------------------------------
 [履歴]
 '11/07/13 takayama	 新規作成
================================================================================================*/
static char c_pdfa_ver[20];

void getPDFAVersion( void ){
	memset(c_pdfa_ver,0x00,sizeof(c_pdfa_ver));
	CNVC_ImageFormatterIf::newInstance()->getPdfaFontVer(c_pdfa_ver);
	printf("PDF/A　Verson：%s\n",c_pdfa_ver);
}
extern "C"
{
extern void callEngBackupStart();
}
/*================================================================================================
［機能］	ENGデータバックアップ.
--------------------------------------------------------------------------------------------------
［引数］	なし
［戻値］	なし.
［詳細］	ENGデータバックアップする.
［ｳｫｰｸｽﾙｰ］	未実施
--------------------------------------------------------------------------------------------------
 [履歴]
 '11/07/13 takayama	 新規作成
================================================================================================*/
void backupENGData(){
	callEngBackupStart();
}
 	
/*================================================================================================
［機能］	Unicodeフォントがダウンロード済みか否かを取得.
--------------------------------------------------------------------------------------------------
［引数］	なし
［戻値］	なし.
［詳細］	Unicodeフォントがダウンロード済みか否かをログ出力.
［ｳｫｰｸｽﾙｰ］	未実施
--------------------------------------------------------------------------------------------------
 [履歴]
 '11/11/08 itakura	 新規作成
================================================================================================*/
void GetFontUnicodeInstallInfo()
 {
	Bool ret = False;
	
	NVDC_MachineSetting* pc_MachineSetting = NVDC_MachineSetting::newInstance();
	ret = pc_MachineSetting->getFontUnicodeInstallFlgInfo();
	if ( ret == True ){
		printf("FontUnicode is installed.\n");
	}else{
		printf("FontUnicode is not installed.\n");
	}
}

 	
/*================================================================================================
［機能］	OCRフォントがダウンロード済みか否かを取得.
--------------------------------------------------------------------------------------------------
［引数］	なし
［戻値］	なし.
［詳細］	OCRフォントがダウンロード済みか否かをログ出力.
［ｳｫｰｸｽﾙｰ］	未実施
--------------------------------------------------------------------------------------------------
 [履歴]
 '11/11/08 itakura	 新規作成
================================================================================================*/
void GetFontOCRInstallInfo()
 {
	Bool ret = False;
	
	NVDC_MachineSetting* pc_MachineSetting = NVDC_MachineSetting::newInstance();
	ret = pc_MachineSetting->getFontOCRInstallFlgInfo();
	if ( ret == True ){
		printf("FontOCR is installed.\n");
	}else{
		printf("FontOCR is not installed.\n");
	}
}

/*================================================================================================
［機能］	トラブルコードが自己診断中に発生するラッチ対象のエラーかどうかを確認する.
--------------------------------------------------------------------------------------------------
［引数］	ushort us_FatalCode トラブルコード.
［戻値］	Bool True：ラッチ対象である / False：ラッチ対象でない.
--------------------------------------------------------------------------------------------------
［履歴］
 '12/07/05	Gonsho	関数追加. タイムアウトをNGで表示する対応
================================================================================================*/
Bool THRG_isLatchErrOnMemFunc(ushort us_FatalCode)
{
	return CC_MEMFunctionSingletonIf::newInstance().isLatchErrOnMemFunc( us_FatalCode );
}

/*================================================================================================
［機能］	TPMデバイスのステータスを取得.
--------------------------------------------------------------------------------------------------
［引数］	なし
［戻値］	なし.
［詳細］	TPMデバイスのステータスを取得しターミナルに表示する.
［ｳｫｰｸｽﾙｰ］	未実施
--------------------------------------------------------------------------------------------------
 [履歴]
 '12/10/02 N.Suzuki	 新規作成. 
 '13/03/14 堀井	     TPM装着状態取得方法変更. 
================================================================================================*/
void GetTPMStatus()
{
	uchar devstat = 0;

	// 13/03/14 堀井 CommonAPI経由でNicfumから装着状態取得.
	devstat = APIC_SystemInfo::newInstance()->isTPMExist();

	switch ( devstat ) {
		case 0x00:	// TPMデバイスが無い。又はエラーで使えない.
			printf("TPM device status = 0x%02X(Not found)\n",devstat);
			break;
		default:	// TPMデバイスが使用できる.
			printf("TPM device status = 0x%02X\n",devstat);
			break;
	}
}

/*===============================================================================================.
［機能］	認証機能有効化の結果通知用のコールバック.
-------------------------------------------------------------------------------------------------.
［引数］	TYPE_TPMResult e_TPMResult.実行結果.
［戻値］	なし.
［詳細］	TPMデバイスの初期化処理の実行結果を通知する.
［履歴］	.
 			2016/01/25 S.Samura	 新規作成. 
================================================================================================*/
void InitializeTPMResult( TYPE_TPMResult e_TPMResult )	{

	switch ( e_TPMResult ) {
		case TYPE_TPMResult_SUCCESS:	// 正常終了.
			printf("Initialization of TPM device is OK\n");
			break;
		default:	// 異常終了.
			printf("Initialization of TPM device is NG\n");
			break;
	}

}

/*================================================================================================
［機能］	TPMデバイスの初期化処理を実施.
--------------------------------------------------------------------------------------------------
［引数］	なし
［戻値］	なし.
［詳細］	TPMチップが搭載されているかどうかを確認し、搭載されている場合は、初期化を行う.
［ｳｫｰｸｽﾙｰ］	未実施
--------------------------------------------------------------------------------------------------
 [履歴]
 			2016/01/25 S.Samura	 新規作成. 
	 	    2016/02/02 Samura    NULLチャック条件修正. 
================================================================================================*/
void InitializeTPM( void )
{
	Bool bl_istpmexist	= False;

	APIC_SystemInfo* pc_Si = APIC_SystemInfo::newInstance();
	APIC_TPM* pc_TPM = APIC_TPM::newInstance();

	if( ( pc_Si != NULL ) && ( pc_TPM != NULL ) ) {  //  pc_TPMがNULLの場合は、初期化処理が出来ない為、ここで併せてチェックする.
		// 装着状態取得.
		bl_istpmexist = pc_Si->isTPMExist();
	}	

	if( bl_istpmexist == False ){	// TPMデバイスが無い.
		printf("TPM device is not attach\n");
	}
	else {							// TPMデバイスが使用できる.
		pc_TPM->req_InitTPM( InitializeTPMResult );
	}
}

/*================================================================================================
［機能］	ADF装着設定を設定.
--------------------------------------------------------------------------------------------------
［引数］	1:ADF非装着/2:循環/3:両面同時.
［戻値］	なし.
［詳細］	ADFの装着状態を設定しターミナルに表示する.
［ｳｫｰｸｽﾙｰ］	未実施.
--------------------------------------------------------------------------------------------------
 [履歴]
 '12/11/05 miyamoto	 新規作成. 
================================================================================================*/
void SetADFType( int32_t type )
{
	switch ( type ) {
		case 1:
			APIC_SystemInfo::newInstance()->setDFType( APIE_DFTypeNoDF );
			printf( "Set ADFType = 1\n" );
			break;
		case 2:
			APIC_SystemInfo::newInstance()->setDFType( APIE_DFTypeOneSideScan );
			printf( "Set ADFType = 2\n" );
			break;
		case 3:
			APIC_SystemInfo::newInstance()->setDFType( APIE_DFTypeDualScan );
			printf( "Set ADFType = 3\n" );
			break;
		default:
			printf( "Argument error \n");
			printf( "Set Argument (1:NoADF,2:OneSideScan,3:DualScan) \n");
			break;
		}
}

/*================================================================================================
［機能］	ADF装着設定を取得.
--------------------------------------------------------------------------------------------------
［引数］	なし.
［戻値］	なし.
［詳細］	ADFの装着状態を取得しターミナルに表示する.
［ｳｫｰｸｽﾙｰ］	未実施.
--------------------------------------------------------------------------------------------------
 [履歴]
 '12/11/05 miyamoto	 新規作成. 
================================================================================================*/
void GetADFType()
{
	switch ( APIC_SystemInfo::newInstance()->getDFType() ) {
		case APIE_DFTypeNoDF:
			printf( "Get ADFType = 1\n" );
			break;
		case APIE_DFTypeOneSideScan:
			printf( "Get ADFType = 2\n" );
			break;
		case APIE_DFTypeDualScan:
			printf( "Get ADFType = 3\n" );
			break;
		default:
			printf( "incorrect value \n" );
			break;
	}
}

/*================================================================================================
［機能］	HDD暗号化ターミナルコマンド.
--------------------------------------------------------------------------------------------------
［引数］	char *key1: 20文字の暗号種.
 			char *key2: 上記の確認用文字列.
［戻値］	なし.
［詳細］	引数1を用いてHDD暗号化の設定を行う.
［ｳｫｰｸｽﾙｰ］	未実施.
--------------------------------------------------------------------------------------------------
 [履歴]
 '12/12/12 horishita	 新規作成. 
================================================================================================*/
#define ENCRYPTHDD_KEY_LEN	20												// 暗号種の文字数.
void EncryptionHDD( char *arg1, char *arg2 )
{
	Bool bl_result = False;													// 引数無効に初期化.
	Bool bl_enable = False;													// 暗号化設定無効に初期化.
	char key[ ENCRYPTHDD_KEY_LEN + 1 ] = { 0 };								// 暗号種バッファを初期化.

	Bool bl_hddexist = APIC_HddInfo::newInstance()->getHddExist();

	if( bl_hddexist == False ) {											// HDD非装着の場合.
		printf( "Error: no HDD\n" );
	} else {																// HDD装着の場合.
		if( ( arg1 == NULL ) || ( arg2 == NULL ) ) {							// 引数1または引数2が無い場合.
			printf( "Error: no argument\n" );
		} else {																// 引数1と引数2が有る場合.
			int32_t len = strlen( arg1 );												// 引数1の文字数を取得.
			if( len == 0 ) {														// 引数1が空文字.
				bl_result = True;														// 引数有効.
			} else if( len == ENCRYPTHDD_KEY_LEN ) {							// 引数1の文字数が有効の場合.
				if( strcmp( arg1, arg2 ) == 0 ) {									// かつ、引数1と引数2が一致の場合は.
					bl_result = True;													// 引数有効.
					bl_enable = True;													// 暗号化設定有効.
					strcpy( key, arg1 );												// 暗号種バッファに引数1をCOPY.
				} else {															// 引数1と引数2が不一致の場合.
					printf( "Error: disagreement argument\n" );
				}
			} else {															// 引数1の文字数が不正.
				printf( "Error: NG length\n" );
			}
		}
	}
	if( bl_result == True ) {												// 引数有効の場合は.
// 2016/02/01 CQ521285 監査ログHCD-PP対応	Sky平田	Start.
		NVDC_SystemSetting::newInstance()->setEnableEncryption( bl_enable, JLGE_InspectLogOperateIDL_System, JLGE_InspectLogIfOTHER, False  );	// 暗号化有効/無効設定.
		NVDC_SystemSetting::newInstance()->setEncryptionPassword( key, JLGE_InspectLogOperateIDL_System, JLGE_InspectLogIfOTHER, False  );		// 暗号種登録.
// 2016/02/01 CQ521285 監査ログHCD-PP対応	Sky平田	End.
		printf( "EncryptionKeySet OK\n" );
	} else {																// 引数無効の場合は.
		printf( "EncryptionKeySet NG\n" );										// 暗号化設定NGログ.
	}
}


// 2016/04/11 CQ526818 ストレージ暗号化パスワードのハッシュ化対応 Sky大嶋 Start.
///*================================================================================================
//［機能］	HDD暗号鍵表示ターミナルコマンド.
//--------------------------------------------------------------------------------------------------
//［引数］	なし.
//［戻値］	なし.
//［詳細］	登録されている暗号鍵を表示する.
//［ｳｫｰｸｽﾙｰ］	未実施.
//--------------------------------------------------------------------------------------------------
// [履歴]
// '12/12/12 horishita	 ソースをTHRC_HddTrans.cppから移動。ラインカード時のみ表示に修正. 
//================================================================================================*/
//extern "C" unsigned char isLineMode();
//#define DEBUG_ENCDEC_SIZE 32
//void dspEncryptKey()
//{
//	if( isLineMode() != 1 ) {		// ラインカードモードでなければ即終了.
//		return;
//	}
//	char chKey[DEBUG_ENCDEC_SIZE] ;
//	memset( chKey, '\x0', sizeof(chKey) ) ;
//	NVDC_SystemSetting::newInstance()->getEncryptionPassword( chKey );
//	printf( "     +0+1+2+3+4 +5+6+7+8+9 +0+1+2+3+4 +5+6+7+8+9\n" ) ;
//	printf( "Key: " ) ;
//	char*p = &chKey[0] ;
//	for ( int32_t i=0; i<20; i++ ) {
//		printf( "%02X", (uchar)*p++ ) ;
//		if ( ((i+1)%5)==0 ) {
//			printf(" ");
//		}
//	}
//	printf( "\n" ) ;
//}

/*================================================================================================
［機能］	階調補正結果表示.
--------------------------------------------------------------------------------------------------
［引数］	プリンタ情報項目（TYPE_PrinterInfo型定義）.
［戻値］	なし.
［詳細］	階調補正を工場で実施するようにする対応.
［ｳｫｰｸｽﾙｰ］	未実施.
--------------------------------------------------------------------------------------------------
 [履歴]
 '13/12/06 tadamasa	 新規作成. 
  14/08/25 iguchi	 [ZueuS][機器構成変更No.13]表示内容の変更.
  18/04/18 Sky小久保 [DenebMLK][制御構成変更_No.10]（Thread）DEF_Zeus5BkCusをDEF_DenebMLKに変更.
================================================================================================*/
void getGradationAdjustment(){
	
	APIC_PrinterServiceInfo* pc_Psi = APIC_PrinterServiceInfo::newInstance();
	if( pc_Psi == NULL ){
		printf("getGradationAdjustment SystemError\n");
		return;	
	}
#ifndef DEF_MonoCus//カラーの場合.
	printf("Print600 / Dark: Adj , Conv / Highlight: Adj , Conv\n");	
	//プリンタ(600dpi)/階調優先.
	printf("Grad C: %d , %d / %d , %d\n"
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoGradationAutoAdjustCorrectCH)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoGradationAutoAdjustConvergeCH)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoGradationAutoAdjustCorrectCL)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoGradationAutoAdjustConvergeCL));		
	printf("Grad M: %d , %d / %d , %d\n"
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoGradationAutoAdjustCorrectMH)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoGradationAutoAdjustConvergeMH)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoGradationAutoAdjustCorrectML)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoGradationAutoAdjustConvergeML));
	printf("Grad Y: %d , %d / %d , %d\n"
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoGradationAutoAdjustCorrectYH)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoGradationAutoAdjustConvergeYH)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoGradationAutoAdjustCorrectYL)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoGradationAutoAdjustConvergeYL));
	printf("Grad K: %d , %d / %d , %d\n"
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoGradationAutoAdjustCorrectKH)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoGradationAutoAdjustConvergeKH)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoGradationAutoAdjustCorrectKL)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoGradationAutoAdjustConvergeKL));
	
	//プリンタ(600dpi)/解像度優先.
	printf("Res  C: %d , %d / %d , %d\n"
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoResolutionAutoAdjustCorrectCH)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoResolutionAutoAdjustConvergeCH)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoResolutionAutoAdjustCorrectCL)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoResolutionAutoAdjustConvergeCL));		
	printf("Res  M: %d , %d / %d , %d\n"
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoResolutionAutoAdjustCorrectMH)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoResolutionAutoAdjustConvergeMH)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoResolutionAutoAdjustCorrectML)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoResolutionAutoAdjustConvergeML));		
	printf("Res  Y: %d , %d / %d , %d\n"
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoResolutionAutoAdjustCorrectYH)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoResolutionAutoAdjustConvergeYH)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoResolutionAutoAdjustCorrectYL)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoResolutionAutoAdjustConvergeYL));		
	printf("Res  K: %d , %d / %d , %d\n"
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoResolutionAutoAdjustCorrectKH)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoResolutionAutoAdjustConvergeKH)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoResolutionAutoAdjustCorrectKL)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoResolutionAutoAdjustConvergeKL));		

	printf("Print1200 / Dark: Adj , Conv / Highlight: Adj , Conv\n");
	//プリンタ(1200dpi)/階調優先.
	printf("Grad C: %d , %d / %d , %d\n"
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoGradationAutoAdjust1200dpiCorrectCH)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoGradationAutoAdjust1200dpiConvergeCH)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoGradationAutoAdjust1200dpiCorrectCL)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoGradationAutoAdjust1200dpiConvergeCL));
	printf("Grad M: %d , %d / %d , %d\n"
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoGradationAutoAdjust1200dpiCorrectMH)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoGradationAutoAdjust1200dpiConvergeMH)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoGradationAutoAdjust1200dpiCorrectML)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoGradationAutoAdjust1200dpiConvergeML));
	printf("Grad Y: %d , %d / %d , %d\n"
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoGradationAutoAdjust1200dpiCorrectYH)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoGradationAutoAdjust1200dpiConvergeYH)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoGradationAutoAdjust1200dpiCorrectYL)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoGradationAutoAdjust1200dpiConvergeYL));
	printf("Grad K: %d , %d / %d , %d\n"
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoGradationAutoAdjust1200dpiCorrectKH)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoGradationAutoAdjust1200dpiConvergeKH)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoGradationAutoAdjust1200dpiCorrectKL)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoGradationAutoAdjust1200dpiConvergeKL));
	
	//プリンタ(1200dpi)/解像度優先.
	printf("Res  C: %d , %d / %d , %d\n"
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoResolutionAutoAdjust1200dpiCorrectCH)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoResolutionAutoAdjust1200dpiConvergeCH)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoResolutionAutoAdjust1200dpiCorrectCL)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoResolutionAutoAdjust1200dpiConvergeCL));
	printf("Res  M: %d , %d / %d , %d\n"
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoResolutionAutoAdjust1200dpiCorrectMH)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoResolutionAutoAdjust1200dpiConvergeMH)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoResolutionAutoAdjust1200dpiCorrectML)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoResolutionAutoAdjust1200dpiConvergeML));
	printf("Res  Y: %d , %d / %d , %d\n"
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoResolutionAutoAdjust1200dpiCorrectYH)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoResolutionAutoAdjust1200dpiConvergeYH)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoResolutionAutoAdjust1200dpiCorrectYL)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoResolutionAutoAdjust1200dpiConvergeYL));
	printf("Res  K: %d , %d / %d , %d\n"
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoResolutionAutoAdjust1200dpiCorrectKH)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoResolutionAutoAdjust1200dpiConvergeKH)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoResolutionAutoAdjust1200dpiCorrectKL)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoResolutionAutoAdjust1200dpiConvergeKL));

	printf("Copy / Dark: Adj , Conv / Highlight: Adj , Conv\n");	
	//コピー/圧縮優先.	
	printf("Comp C: %d , %d / %d , %d\n"
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoErrorDiffusionAutoAdjustCorrectCH)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoErrorDiffusionAutoAdjustConvergeCH)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoErrorDiffusionAutoAdjustCorrectCL)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoErrorDiffusionAutoAdjustConvergeCL));		
	printf("Comp M: %d , %d / %d , %d\n"
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoErrorDiffusionAutoAdjustCorrectMH)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoErrorDiffusionAutoAdjustConvergeMH)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoErrorDiffusionAutoAdjustCorrectML)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoErrorDiffusionAutoAdjustConvergeML));		
	printf("Comp Y: %d , %d / %d , %d\n"
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoErrorDiffusionAutoAdjustCorrectYH)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoErrorDiffusionAutoAdjustConvergeYH)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoErrorDiffusionAutoAdjustCorrectYL)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoErrorDiffusionAutoAdjustConvergeYL));		
	printf("Comp K: %d , %d / %d , %d\n"
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoErrorDiffusionAutoAdjustCorrectKH)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoErrorDiffusionAutoAdjustConvergeKH)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoErrorDiffusionAutoAdjustCorrectKL)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoErrorDiffusionAutoAdjustConvergeKL));		


	//コピー/FEET.
	printf("FEET C: %d , %d / %d , %d\n"
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoFeetAutoAdjustCorrectCH)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoFeetAutoAdjustConvergeCH)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoFeetAutoAdjustCorrectCL)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoFeetAutoAdjustConvergeCL));
	printf("FEET M: %d , %d / %d , %d\n"
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoFeetAutoAdjustCorrectMH)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoFeetAutoAdjustConvergeMH)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoFeetAutoAdjustCorrectML)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoFeetAutoAdjustConvergeML));
	printf("FEET Y: %d , %d / %d , %d\n"
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoFeetAutoAdjustCorrectYH)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoFeetAutoAdjustConvergeYH)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoFeetAutoAdjustCorrectYL)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoFeetAutoAdjustConvergeYL));
	printf("FEET K: %d , %d / %d , %d\n"
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoFeetAutoAdjustCorrectKH)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoFeetAutoAdjustConvergeKH)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoFeetAutoAdjustCorrectKL)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoFeetAutoAdjustConvergeKL));

#else//モノクロの場合.
// 2018/04/17【制御構成変更_No.10】コンパイル環境作成 Sky小久保. 
#ifdef DEF_DenebMLKCus // DEF_Zeus5BkCusから DEF_DenebMLKCusに変更.
	printf("Print / Dark: Adj , Conv / Highlight: Adj , Conv\n");	
	//プリンタ/FEET.
	printf("FEET K: %d , %d / %d , %d\n"
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoFeetAutoAdjustCorrectKH)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoFeetAutoAdjustConvergeKH)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoFeetAutoAdjustCorrectKL)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoFeetAutoAdjustConvergeKL));

	//プリンタ/解像度優先.
	printf("Res  K: %d , %d / %d , %d\n"
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoResolutionAutoAdjustCorrectKH)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoResolutionAutoAdjustConvergeKH)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoResolutionAutoAdjustCorrectKL)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoResolutionAutoAdjustConvergeKL));		

	printf("Copy / Dark: Adj , Conv / Highlight: Adj , Conv\n");
	//コピー/圧縮優先.	
	printf("Comp K: %d , %d / %d , %d\n"
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoErrorDiffusionAutoAdjustCorrectKH)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoErrorDiffusionAutoAdjustConvergeKH)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoErrorDiffusionAutoAdjustCorrectKL)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoErrorDiffusionAutoAdjustConvergeKL));		

	//コピー/諧調優先.
	printf("Grad K: %d , %d / %d , %d\n"
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoGradationAutoAdjustCorrectKH)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoGradationAutoAdjustConvergeKH)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoGradationAutoAdjustCorrectKL)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoGradationAutoAdjustConvergeKL));

#else //DEF_Zeus5BkCus.
	printf("Print600 / Dark: Adj , Conv / Highlight: Adj , Conv\n");	
	//プリンタ(600dpi)/階調優先.
	printf("Grad K: %d , %d / %d , %d\n"
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoGradationAutoAdjustCorrectKH)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoGradationAutoAdjustConvergeKH)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoGradationAutoAdjustCorrectKL)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoGradationAutoAdjustConvergeKL));

	//プリンタ(600dpi)/解像度優先.
	printf("Res  K: %d , %d / %d , %d\n"
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoResolutionAutoAdjustCorrectKH)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoResolutionAutoAdjustConvergeKH)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoResolutionAutoAdjustCorrectKL)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoResolutionAutoAdjustConvergeKL));		

	printf("Print1200 / Dark: Adj , Conv / Highlight: Adj , Conv\n");
	//プリンタ(1200dpi)/階調優先.
	printf("Grad K: %d , %d / %d , %d\n"
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoGradationAutoAdjust1200dpiCorrectKH)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoGradationAutoAdjust1200dpiConvergeKH)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoGradationAutoAdjust1200dpiCorrectKL)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoGradationAutoAdjust1200dpiConvergeKL));

	//プリンタ(1200dpi)/解像度優先.
	printf("Res  K: %d , %d / %d , %d\n"
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoResolutionAutoAdjust1200dpiCorrectKH)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoResolutionAutoAdjust1200dpiConvergeKH)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoResolutionAutoAdjust1200dpiCorrectKL)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoResolutionAutoAdjust1200dpiConvergeKL));

	printf("Copy / Dark: Adj , Conv / Highlight: Adj , Conv\n");
	//コピー/圧縮優先.	
	printf("Comp K: %d , %d / %d , %d\n"
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoErrorDiffusionAutoAdjustCorrectKH)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoErrorDiffusionAutoAdjustConvergeKH)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoErrorDiffusionAutoAdjustCorrectKL)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoErrorDiffusionAutoAdjustConvergeKL));		

	//コピー/FEET.
	printf("FEET K: %d , %d / %d , %d\n"
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoFeetAutoAdjustCorrectKH)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoFeetAutoAdjustConvergeKH)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoFeetAutoAdjustCorrectKL)
			,pc_Psi->getPrinterInfo(TYPE_PrinterInfoFeetAutoAdjustConvergeKL));

#endif //DEF_Zeus5BkCus.
#endif//DEF_MonoCus

}

/*================================================================================================
［機能］	ウィークリータイマー使用設定.
--------------------------------------------------------------------------------------------------
［引数］	ウィークリータイマーON/OFF設定.
			OFF時の節電モード設定.
［戻値］	なし.
［詳細］	ウィークリータイマー使用設定の設定値変更を行なう.
［ｳｫｰｸｽﾙｰ］	未実施.
--------------------------------------------------------------------------------------------------
 [履歴]
 '13/12/16 tadamasa	 新規作成. 
================================================================================================*/
void SetWeeklyTimer(short ss_num1 , short ss_num2){
        //APIのNULLチェック.
        if(pc_SystemSetting==NULL){printf("SetWeeklyTimer SystemErr\n");  return;}
        //範囲チェック.
        if( (1 < ss_num1) || ( ss_num1 < 0 ) ){ printf("Setting out range (0-1)\n");  return;}
        if( (1 < ss_num2) || ( ss_num2 < 0 ) ){ printf("Value out range (0-1)\n");  return;}

        //定義.
        TYPC_WeeklyTimer c_wt;
        memset(&c_wt, 0x00, sizeof(c_wt));

        //ウィークリータイマの情報取得.
        c_wt = pc_SystemSetting->getWeeklyTimer();

        //ウィークリータイマの設定.
		Bool b_IsWeeklyTimer = (ss_num1==1) ? True: False;
        c_wt.b_WeeklyTimerOn = b_IsWeeklyTimer;
        pc_SystemSetting->setWeeklyTimer(c_wt);

        //ログ表示.
        if(ss_num1 == 0){
            printf("Setting = OFF\n");
        }else{
            printf("Setting = ON\n");
            //OFF時の節電モード設定.
            if(ss_num2 == 0){
	            pc_SystemSetting->setOffPowerMode(TYPE_OffPowerModeErPAutoPowerOff);
                printf("Value = ErP Auto Power OFF\n");
            }else{
	            pc_SystemSetting->setOffPowerMode(TYPE_OffPowerModeSleep);
                printf("Value = Sleep\n");
            }
        }
}

/*================================================================================================
［機能］	ターミナルコマンドで分割保存の閾値を設定する.
--------------------------------------------------------------------------------------------------
［引数］	mmサイズx	10倍値.
			mmサイズy	10倍値.
			解像度x		10倍値.
			解像度y		10倍値.
［戻値］	なし.
［詳細］	ターミナルコマンドで分割保存の閾値を設定する.
［ｳｫｰｸｽﾙｰ］ 未実施.
--------------------------------------------------------------------------------------------------
 [履歴]
 '13/10/03 wakui	 新規作成. 
================================================================================================*/
void setThresholdAreaWrite( uint32_t ul_sizeMM_x, uint32_t ul_sizeMM_y, uint32_t ul_resolution_x, uint32_t ul_resolution_y )
{
	float f_sizeMM_x = (float)( ul_sizeMM_x / 10UL );
	float f_sizeMM_y = (float)( ul_sizeMM_y / 10UL );
	float f_resolution_x = (float)( ul_resolution_x / 10UL );
	float f_resolution_y = (float)( ul_resolution_y / 10UL );

	ul_debug_isSplitTransThresholdSizeWrite = _roundup( (uint32_t)(MMFtoDOTS(f_sizeMM_x, f_resolution_x) * MMFtoDOTS(f_sizeMM_y, f_resolution_y) * 13), 8L );
	printf( "ul_debug_isSplitTransThresholdSizeWrite %"PRIu32"\n", ul_debug_isSplitTransThresholdSizeWrite );
}

/*================================================================================================
［機能］	ターミナルコマンドで分割読み出しの閾値を設定する.
--------------------------------------------------------------------------------------------------
［引数］	mmサイズx	10倍値.
			mmサイズy	10倍値.
			解像度x		10倍値.
			解像度y		10倍値.
［戻値］	なし.
［詳細］	ターミナルコマンドで分割読み出しの閾値を設定する.
［ｳｫｰｸｽﾙｰ］ 未実施.
--------------------------------------------------------------------------------------------------
 [履歴]
 '13/10/03 wakui	 新規作成. 
================================================================================================*/
void setThresholdAreaRead( uint32_t ul_sizeMM_x, uint32_t ul_sizeMM_y, uint32_t ul_resolution_x, uint32_t ul_resolution_y )
{
	float f_sizeMM_x = (float)( ul_sizeMM_x / 10UL );
	float f_sizeMM_y = (float)( ul_sizeMM_y / 10UL );
	float f_resolution_x = (float)( ul_resolution_x / 10UL );
	float f_resolution_y = (float)( ul_resolution_y / 10UL );

	ul_debug_isSplitTransThresholdSizeRead = _roundup( (uint32_t)(MMFtoDOTS(f_sizeMM_x, f_resolution_x) * MMFtoDOTS(f_sizeMM_y, f_resolution_y) * 13), 8L );
	printf( "ul_debug_isSplitTransThresholdSizeRead %"PRIu32"\n", ul_debug_isSplitTransThresholdSizeRead );
}

/*================================================================================================
［機能］	マシン図データ有無を確認する(ターミナルコマンド).
--------------------------------------------------------------------------------------------------
［引数］	void.
［戻値］	Bool	True:マシン図データあり.
					False:マシン図データなし.
［履歴］	2014/04/30 fukuhara 新規作成.
［履歴］	2014/08/04 iyanagi [IT4_5th] CQ391330 マシン図データ削除対応：インストール確認方法を変更(ファイル存在ではなくて、インストールフラグ).
［履歴］	2014/08/07 iyanagi [IT4_5th] CQ391330 マシン図データ削除対応：バージョン表示出力の統合.
================================================================================================*/
Bool isExistMachineDiagramData(){
	Bool bl_ans = False;
	APIC_MachineSetting *pc_MachineSetting = APIC_MachineSetting::newInstance();
	if( pc_MachineSetting != NULL ){
		bl_ans = pc_MachineSetting->getOEMMachineFigDataInstallFlgInfo();
		if( bl_ans == True ){
			getMachineDiagramDataVersion();
		}
	}
	return bl_ans;
}

/*================================================================================================.
［機能］	ターミナルコマンド （マシン図データのバージョン情報表示）.
--------------------------------------------------------------------------------------------------.
［引数］	なし .
［戻値］	なし .
［詳細］	マシン図データのバージョン情報をログ出力.
--------------------------------------------------------------------------------------------------.
 [履歴]
 2014/07/24 iyanagi    [IT4_5th] CQ391330 マシン図データ削除対応 新規作成.
================================================================================================*/
void getMachineDiagramDataVersion()
{
	printf( "DataVersion = " );
	APIC_SystemInfo*  pc_SystemInfo = APIC_SystemInfo::newInstance();
	if( pc_SystemInfo != NULL ){
		char sc_RomVersion[40+1] = {0};
		pc_SystemInfo->getRomVersion( TYPE_RomOEMMachineFigData, sc_RomVersion );// マシン図データのＲＯＭバージョン取得.
		printf( "%s", sc_RomVersion );
	}
	printf( "\n" );
}

#ifdef DEF_Zeus5BkCus 
/*================================================================================================
［機能］	SW209設定.
--------------------------------------------------------------------------------------------------
［引数］	sl_disp: 8 OWN.
			sl_set : 0[OFF] AirPrint機能の使用可否設定 無効.
                     1[ON]  AirPrint機能の使用可否設定 有効.
［戻値］	なし.
［詳細］	コマンド実行により、DipSW209(bit3)の設定を実行する。.
			setDipsw209 8 0	OWNの場合、Airprint機能設定を無効にする。.
			setDipsw209 8 1	OWNの場合、Airprint機能設定を有効にする。.
［ｳｫｰｸｽﾙｰ］	
================================================================================================*/
void setDipsw209( uint32_t sl_disp, uint32_t sl_set )
{
	APIC_SoftDipSw* pc_SoftDipSw = APIC_SoftDipSw::newInstance();
	Bool bl_set = False;
	
	if( pc_SoftDipSw == NULL ){
		printf("SystemErr\n");
		return;	
	}
	
	if( (sl_disp != 0x08) || (sl_set < 0 || sl_set > 1) ){	// パラメータチェック.
		printf("setDipsw209 parameterNG\n");
		return;
	}

	bl_set = (1 == sl_set)?True:False;
	pc_SoftDipSw->setSoftDipSwBit( 89, 8, bl_set );
	if( pc_SoftDipSw->getSoftDipSwBit( 89, 8 ) != bl_set ){	// setSoftDipSwBit()で期待結果が得られているかどうかのチェック.
		printf("setDipsw209 NG\n");
		return;
	}

	printf("setDipsw209 OK\n");

	return;
}

/*================================================================================================
［機能］	SW209表示.
--------------------------------------------------------------------------------------------------
［引数］	なし.
［戻値］	なし.
［詳細］	設定されているDipSW No209(bit3)の状態を表示する。.
［ｳｫｰｸｽﾙｰ］	.
================================================================================================*/
void dspDipsw209()
{
	APIC_SoftDipSw* pc_SoftDipSw = APIC_SoftDipSw::newInstance();
	char sc_DIPSW209 = 0;

	if( pc_SoftDipSw == NULL ){
		printf("SystemErr\n");
		return;	
	}
	
	if( pc_SoftDipSw->getSoftDipSwBit(89, 8) == True ){
		sc_DIPSW209 |= 0x08;
	}

	printf( "DIPSW209:0x%02x\n", sc_DIPSW209);
	return;
}

/*================================================================================================
［機能］	SW106設定.
--------------------------------------------------------------------------------------------------
［引数］	sl_disp: 16 OWN.
			sl_set : 0[OFF] AirPrint機能の使用可否設定 無効.
                     1[ON]  AirPrint機能の使用可否設定 有効.
［戻値］	なし.
［詳細］	コマンド実行により、DipSW106(bit4)の設定を実行する。.
			setDipsw106 16 0	Genericの場合、Airprint機能設定を無効にする。.
			setDipsw106 16 1	Genericの場合、Airprint機能設定を有効にする。.
［ｳｫｰｸｽﾙｰ］	
================================================================================================*/
void setDipsw106( uint32_t sl_disp, uint32_t sl_set )
{
	APIC_SoftDipSw* pc_SoftDipSw = APIC_SoftDipSw::newInstance();
	Bool bl_set = False;
	
	if( pc_SoftDipSw == NULL ){
		printf("SystemErr\n");
		return;	
	}
	
	if( (sl_disp != 0x10) || (sl_set < 0 || sl_set > 1) ){	// パラメータチェック.
		printf("setDipsw106 parameterNG\n");
		return;
	}

	bl_set = (1 == sl_set)?True:False;
	pc_SoftDipSw->setSoftDipSwBit( 90, 1, bl_set );
	if( pc_SoftDipSw->getSoftDipSwBit( 90, 1 ) != bl_set ){	// setSoftDipSwBit()で期待結果が得られているかどうかのチェック.
		printf("setDipsw106 NG\n");
		return;
	}

	printf("setDipsw106 OK\n");

	return;
}

/*================================================================================================
［機能］	SW106表示.
--------------------------------------------------------------------------------------------------
［引数］	なし.
［戻値］	なし.
［詳細］	設定されているDipSW No106(bit4)の状態を表示する。.
［ｳｫｰｸｽﾙｰ］	.
================================================================================================*/
void dspDipsw106()
{
	APIC_SoftDipSw* pc_SoftDipSw = APIC_SoftDipSw::newInstance();
	char sc_DIPSW106 = 0;
	
	if( pc_SoftDipSw == NULL ){
		printf("SystemErr\n");
		return;	
	}
	
	if( pc_SoftDipSw->getSoftDipSwBit(90, 1) == True ){
		sc_DIPSW106 |= 0x10;
	}

	printf( "DIPSW106:0x%02x\n", sc_DIPSW106);
	return;
}


/*================================================================================================
［機能］	SW161設定.
--------------------------------------------------------------------------------------------------
［引数］	sl_disp: 2 Develop.
			sl_set : 0[OFF] AirPrint機能の使用可否設定 無効.
                     1[ON]  AirPrint機能の使用可否設定 有効.
［戻値］	なし.
［詳細］	コマンド実行により、DipSW161(bit1)の設定を実行する。.
			setDipsw161 2 0	Developの場合、Airprint機能設定を無効にする。.
			setDipsw161 2 1	Developの場合、Airprint機能設定を有効にする。.
［ｳｫｰｸｽﾙｰ］	
================================================================================================*/
void setDipsw161( uint32_t sl_disp, uint32_t sl_set )
{
	APIC_SoftDipSw* pc_SoftDipSw = APIC_SoftDipSw::newInstance();
	Bool bl_set = False;
	
	if( pc_SoftDipSw == NULL ){
		printf("SystemErr\n");
		return;	
	}
	
	if( (sl_disp != 0x02) || (sl_set < 0 || sl_set > 1) ){	// パラメータチェック.
		printf("setDipsw161 parameterNG\n");
		return;
	}

	bl_set = (1 == sl_set)?True:False;
	pc_SoftDipSw->setSoftDipSwBit( 90, 2, bl_set );
	if( pc_SoftDipSw->getSoftDipSwBit( 90, 2 ) != bl_set ){	// setSoftDipSwBit()で期待結果が得られているかどうかのチェック.
		printf("setDipsw161 NG\n");
		return;
	}

	printf("setDipsw161 OK\n");

	return;
}

/*================================================================================================
［機能］	SW161表示.
--------------------------------------------------------------------------------------------------
［引数］	なし.
［戻値］	なし.
［詳細］	設定されているDipSW No161(bit1)の状態を表示する。.
［ｳｫｰｸｽﾙｰ］	.
================================================================================================*/
void dspDipsw161()
{
	APIC_SoftDipSw* pc_SoftDipSw = APIC_SoftDipSw::newInstance();
	char sc_DIPSW161 = 0;

	if( pc_SoftDipSw == NULL ){
		printf("SystemErr\n");
		return;	
	}
	
	if( pc_SoftDipSw->getSoftDipSwBit(90, 2) == True ){
		sc_DIPSW161 |= 0x02;
	}
	
	printf( "DIPSW161:0x%02x\n", sc_DIPSW161 );

	return;
}
#endif //DEF_Zeus5BkCus.
#if defined(DEF_ZEUSS_CUS) || (defined DEF_ZEUSS_BASE_EMU)
/*================================================================================================
［機能］	SW125設定.
--------------------------------------------------------------------------------------------------
［引数］	sl_disp: 2 OWN.
			sl_set : 0[OFF] AirPrint機能の使用可否設定 無効.
                     1[ON]  AirPrint機能の使用可否設定 有効.
［戻値］	なし.
［詳細］	コマンド実行により、DipSW125(bit1)の設定を実行する。.
			setDipsw125 2 0	OWNの場合、Airprint機能設定を無効にする。.
			setDipsw125 2 1	OWNの場合、Airprint機能設定を有効にする。.
［ｳｫｰｸｽﾙｰ］	
================================================================================================*/
void setDipsw125( uint32_t sl_disp, uint32_t sl_set )
{
	APIC_SoftDipSw* pc_SoftDipSw = APIC_SoftDipSw::newInstance();
	Bool bl_set = False;
	
	if( pc_SoftDipSw == NULL ){
		printf("SystemErr\n");
		return;	
	}
	
	if( (sl_disp != 0x02) || (sl_set < 0 || sl_set > 1) ){	// パラメータチェック.
		printf("setDipsw125 parameterNG\n");
		return;
	}

	bl_set = (1 == sl_set)?True:False;
	pc_SoftDipSw->setSoftDipSwBit( 90, 3, bl_set );
	if( pc_SoftDipSw->getSoftDipSwBit( 90, 3 ) != bl_set ){	// setSoftDipSwBit()で期待結果が得られているかどうかのチェック.
		printf("setDipsw125 NG\n");
		return;
	}

	printf("setDipsw125 OK\n");

	return;
}

/*================================================================================================
［機能］	SW125表示.
--------------------------------------------------------------------------------------------------
［引数］	なし.
［戻値］	なし.
［詳細］	設定されているDipSW No125(bit1)の状態を表示する。.
［ｳｫｰｸｽﾙｰ］	.
================================================================================================*/
void dspDipsw125()
{
	APIC_SoftDipSw* pc_SoftDipSw = APIC_SoftDipSw::newInstance();
	char sc_DIPSW125 = 0;

	if( pc_SoftDipSw == NULL ){
		printf("SystemErr\n");
		return;	
	}
	
	if ( pc_SoftDipSw->getSoftDipSwBit(90, 3) == True ){
		sc_DIPSW125 |= 0x02;
	}
	
	printf( "DIPSW125:0x%02x\n", sc_DIPSW125 );
	
	return;
}

/*================================================================================================
［機能］	SW182設定.
--------------------------------------------------------------------------------------------------
［引数］	sl_disp: 1 Generic.
			sl_set : 0[OFF] AirPrint機能の使用可否設定 無効.
                     1[ON]  AirPrint機能の使用可否設定 有効.
［戻値］	なし.
［詳細］	コマンド実行により、DipSW182(bit0)の設定を実行する。.
			setDipsw182 1 0	OWNの場合、Airprint機能設定を無効にする。.
			setDipsw182 1 1	OWNの場合、Airprint機能設定を有効にする。.
［ｳｫｰｸｽﾙｰ］	
================================================================================================*/
void setDipsw182( uint32_t sl_disp, uint32_t sl_set )
{
	APIC_SoftDipSw* pc_SoftDipSw = APIC_SoftDipSw::newInstance();
	Bool bl_set = False;
	
	if( pc_SoftDipSw == NULL ){
		printf("SystemErr\n");
		return;	
	}
	
	if( (sl_disp != 0x01) || (sl_set < 0 || sl_set > 1) ){	// パラメータチェック.
		printf("setDipsw182 parameterNG\n");
		return;
	}

	bl_set = (1 == sl_set)?True:False;
	pc_SoftDipSw->setSoftDipSwBit( 90, 4, bl_set );
	if( pc_SoftDipSw->getSoftDipSwBit( 90, 4 ) != bl_set ){	// setSoftDipSwBit()で期待結果が得られているかどうかのチェック.
		printf("setDipsw182 NG\n");
		return;
	}

	printf("setDipsw182 OK\n");

	return;
}

/*================================================================================================
［機能］	SW182表示.
--------------------------------------------------------------------------------------------------
［引数］	なし.
［戻値］	なし.
［詳細］	設定されているDipSW No182(bit0)の状態を表示する。.
［ｳｫｰｸｽﾙｰ］	.
================================================================================================*/
void dspDipsw182()
{
	APIC_SoftDipSw* pc_SoftDipSw = APIC_SoftDipSw::newInstance();
	char sc_DIPSW182 = 0;

	if( pc_SoftDipSw == NULL ){
		printf("SystemErr\n");
		return;	
	}
	
	if( pc_SoftDipSw->getSoftDipSwBit(90, 4) == True ){
		sc_DIPSW182 |= 0x01;
	}
	
	printf( "DIPSW182:0x%02x\n", sc_DIPSW182 );
	
	return;
}


/*================================================================================================
［機能］	SW157設定.
--------------------------------------------------------------------------------------------------
［引数］	sl_disp: 32 Develop.
			sl_set : 0[OFF] AirPrint機能の使用可否設定 無効.
                     1[ON]  AirPrint機能の使用可否設定 有効.
［戻値］	なし.
［詳細］	コマンド実行により、DipSW157(bit5)の設定を実行する。.
			setDipsw157 32 0	OWNの場合、Airprint機能設定を無効にする。.
			setDipsw157 32 1	OWNの場合、Airprint機能設定を有効にする。.
［ｳｫｰｸｽﾙｰ］	
================================================================================================*/
void setDipsw157(uint32_t sl_disp, uint32_t sl_set )
{
	APIC_SoftDipSw* pc_SoftDipSw = APIC_SoftDipSw::newInstance();
	Bool bl_set = False;
	
	if( pc_SoftDipSw == NULL ){
		printf("SystemErr\n");
		return;	
	}
	
	if( (sl_disp != 0x20) || (sl_set < 0 || sl_set > 1) ){	// パラメータチェック.
		printf("setDipsw157 parameterNG\n");
		return;
	}

	bl_set = (1 == sl_set)?True:False;
	pc_SoftDipSw->setSoftDipSwBit( 90, 5, bl_set );
	if( pc_SoftDipSw->getSoftDipSwBit( 90, 5 ) != bl_set ){	// setSoftDipSwBit()で期待結果が得られているかどうかのチェック.
		printf("setDipsw157 NG\n");
		return;
	}

	printf("setDipsw157 OK\n");

	return;
}

/*================================================================================================
［機能］	SW157表示.
--------------------------------------------------------------------------------------------------
［引数］	なし.
［戻値］	なし.
［詳細］	設定されているDipSW No157(bit5)の状態を表示する。.
［ｳｫｰｸｽﾙｰ］	.
================================================================================================*/
void dspDipsw157()
{
	APIC_SoftDipSw* pc_SoftDipSw = APIC_SoftDipSw::newInstance();
	char sc_DIPSW157 = 0;

	if( pc_SoftDipSw == NULL ){
		printf("SystemErr\n");
		return;	
	}
	
	if( pc_SoftDipSw->getSoftDipSwBit(90, 5) == True ){
		sc_DIPSW157 |= 0x20;
	}
	
	printf( "DIPSW157:0x%02x\n", sc_DIPSW157 );
	
	return;
}
#endif //DEF_ZEUSS_CUS.

/*================================================================================================
［機能］	SW170設定.
--------------------------------------------------------------------------------------------------
［引数］	sl_disp: 1:Generic、2:OWN、4:Develop.
			sl_set : 0[OFF] Wi-Fi Direct機能の使用可否設定 無効.
                     1[ON]  Wi-Fi Direct機能の使用可否設定 有効.
［戻値］	なし.
［詳細］	コマンド実行により、DipSW170の設定を実行する。.
			setDipsw170 1 0	Genericの場合、Wi-Fi Direct機能設定を無効にする。.
			setDipsw170 1 1	Genericの場合、Wi-Fi Direct機能設定を有効にする。.
			setDipsw170 2 0	OWNの場合、Wi-Fi Direct機能設定を無効にする。.
			setDipsw170 2 1	OWNの場合、Wi-Fi Direct機能設定を有効にする。.
			setDipsw170 4 0	Developの場合、Wi-Fi Direct機能設定を無効にする。.
			setDipsw170 4 1	Developの場合、Wi-Fi Direct機能設定を有効にする。.
［ｳｫｰｸｽﾙｰ］.	
================================================================================================*/
void setDipsw170(uint32_t sl_disp, int32_t sl_set )
{
	APIC_SoftDipSw* pc_SoftDipSw = APIC_SoftDipSw::newInstance();
	Bool bl_set = False;
	uchar uc_bit = 0;
	
	if( pc_SoftDipSw == NULL ){
		printf("SystemErr\n");
		return;	
	}
	
	if( ((sl_disp != THRD_TYPE_GENERIC) && (sl_disp != THRD_TYPE_OWN) && (sl_disp != THRD_TYPE_DEVELOP))
	  ||(sl_set < 0 || sl_set > 1) ){	// パラメータチェック.
		printf("setDipsw170 parameterNG\n");
		return;
	}

	bl_set = (1 == sl_set)?True:False;
	
	switch( sl_disp ){
	case THRD_TYPE_GENERIC:
		uc_bit = 1;
		break;
	case THRD_TYPE_OWN:
		uc_bit = 2;
		break;
	case THRD_TYPE_DEVELOP:
		uc_bit = 3;
		break;
	default:
		break;
	}
	
	pc_SoftDipSw->setSoftDipSwBit( 91, uc_bit, bl_set );
	if( pc_SoftDipSw->getSoftDipSwBit( 91, uc_bit ) != bl_set ){	// setSoftDipSwBit()で期待結果が得られているかどうかのチェック.
		printf("setDipsw170 NG\n");
		return;
	}

	printf("setDipsw170 OK\n");

	return;
}

/*================================================================================================
［機能］	SW170表示.
--------------------------------------------------------------------------------------------------
［引数］	なし.
［戻値］	なし.
［詳細］	設定されているDipSW No170の状態を表示する。.
［ｳｫｰｸｽﾙｰ］	.
================================================================================================*/
void dspDipsw170()
{
	APIC_SoftDipSw* pc_SoftDipSw = APIC_SoftDipSw::newInstance();
	char sc_DIPSW170 = 0;

	if( pc_SoftDipSw == NULL ){
		printf("SystemErr\n");
		return;	
	}
	
	if( pc_SoftDipSw->getSoftDipSwBit(91, 1) == True ){
		sc_DIPSW170 |= THRD_TYPE_GENERIC;
	}
	if( pc_SoftDipSw->getSoftDipSwBit(91, 2) == True ){
		sc_DIPSW170 |= THRD_TYPE_OWN;
	}
	if( pc_SoftDipSw->getSoftDipSwBit(91, 3) == True ){
		sc_DIPSW170 |= THRD_TYPE_DEVELOP;
	}
	
	printf( "DIPSW170:0x%02x\n", sc_DIPSW170 );
	
	return;
}

/*================================================================================================
［機能］	SW172設定.
--------------------------------------------------------------------------------------------------
［引数］	sl_Set : MarketPlaceダウンローダー、セットアップ機能使用可否設定.
					 1 : ダウンロード有効(自動ダウンロード).
					 2 : ダウンロード無効（デフォルト）.
					 3 : ダウンロード有効(手動ダウンロード).
					 4 : セットアップ有効(ダウンロードは無効).
［戻値］	なし.
［詳細］	コマンド実行により、DipSW172の設定を実行する。.
			setDipsw172 1	ダウンロード有効(自動ダウンロード).
			setDipsw172 2	ダウンロード無効.
			setDipsw172 3	ダウンロード有効(手動ダウンロード).
			setDipsw172 4	セットアップ有効(ダウンロードは無効).
［履歴］	.
			2015/08/25	大岡 [ZeusSZX0]RQ-383_DipSWの新規追加.
			2018/02/28	河埜 [Eagle]RQ-675_性能に合わせてすべてのMFPにMarketPlaceをダウンロード・インストールする.
 18/08/09 Sky忠政　AR1049080 再提起対応.
 18/09/06 Sky忠政　AR1049080 再再提起対応.
================================================================================================*/
void setDipsw172( uint32_t sl_Set )
{
	APIC_SoftDipSw* pc_SoftDipSw = APIC_SoftDipSw::newInstance();
	
	// 初期化失敗時は「SytemErr」表示.
	if( pc_SoftDipSw == NULL ){
		printf("SystemErr\n");
		return;	
	}
	
	// NVRAM 101-5Bit,6Bit,136-5Bitに設定する値をセット.
	Bool bl_Set[3] = { False };
	switch( sl_Set ){
	case 2:								// 無効.
		bl_Set[0] = True;				// 5Bit : ON.
		bl_Set[1] = False;				// 6Bit : OFF.
		bl_Set[2] = False;				// 136-5Bit : OFF.
		break;
	case 4:								// ※セットアップ有効化時はダウンロードは無効.
		bl_Set[0] = True;				// 5Bit : ON.
		bl_Set[1] = False;				// 6Bit : OFF.
		bl_Set[2] = True;				// 136-5Bit : ON.
		break;
	default:
		// 引数が設定範囲外のため「parameterNG」表示.
		printf("setDipsw172 parameterNG\n");
		return;
	}

	// 5Bit設定後に読出を行い、不一致の場合は「NG」表示.
	pc_SoftDipSw->setSoftDipSwBit( 101, 5, bl_Set[0] );
	if( pc_SoftDipSw->getSoftDipSwBit( 101, 5 ) != bl_Set[0] ){
		printf("setDipsw172 NG\n");
		return;
	}
	// 6Bit設定後に読出を行い、不一致の場合は「NG」表示.
	pc_SoftDipSw->setSoftDipSwBit( 101, 6, bl_Set[1] );
	if( pc_SoftDipSw->getSoftDipSwBit( 101, 6 ) != bl_Set[1] ){
		printf("setDipsw172 NG\n");
		return;
	}

	// セットアップ機能を「有効」に設定.
	// 136-5Bit設定後に読出を行い、不一致の場合は「NG」表示.
	pc_SoftDipSw->setSoftDipSwBit( 136, 5, bl_Set[2] );
	if( pc_SoftDipSw->getSoftDipSwBit( 136, 5 ) != bl_Set[2] ){
		printf("setDipsw172 NG\n");
		return;
	}

	// 設定完了できた時は「OK」表示.
	printf("setDipsw172 OK\n");

	return;
}

/*================================================================================================
［機能］	SW172表示.
--------------------------------------------------------------------------------------------------
［引数］	なし.
［戻値］	なし.
［詳細］	設定されているDipSW No172の状態を表示する。.
［履歴］	.
			2015/08/25	大岡 [ZeusSZX0]RQ-383_DipSWの新規追加.
			2018/02/28	河埜 [Eagle]RQ-675_性能に合わせてすべてのMFPにMarketPlaceをダウンロード・インストールする.
 			2018/08/09 Sky忠政　AR1049080 再提起対応.
================================================================================================*/
void dspDipsw172( void )
{
	APIC_SoftDipSw* pc_SoftDipSw = APIC_SoftDipSw::newInstance();

	// 初期化失敗時は「SytemErr」表示.
	if( pc_SoftDipSw == NULL ){
		printf("SystemErr\n");
		return;	
	}
	
	char sc_DipSw172 = 0;
	// DipSW172の値が保存されている NVRAM:101の5Bit、6Bit、NVRAM:134の1Bitを読込み、表示用データにする.
	if( pc_SoftDipSw->getSoftDipSwBit(101, 5) == True ){
		sc_DipSw172 |= 0x01;
	}
	if( pc_SoftDipSw->getSoftDipSwBit(101, 6) == True ){
		sc_DipSw172 |= 0x02;
	}
	if( pc_SoftDipSw->getSoftDipSwBit(136, 5) == True ){
		sc_DipSw172 |= 0x04;
	}
	
	printf( "DIPSW172:0x%02x\n", sc_DipSw172 );
	
	return;
}

/*================================================================================================.
［機能］	ターミナルコマンド （ 機種情報と機種Noの登録 ）.
--------------------------------------------------------------------------------------------------.
［引数］	sc_Machine	機種情報.
			sc_Type		機種No ( 設定されていない場合は0を設定する ).
［戻値］	なし .
［詳細］	ラインモード起動時のみ、機種情報と機種Noを登録する.
--------------------------------------------------------------------------------------------------.
 [履歴]
 2014/12/11 iguchi    [HeliosL][制御構成変更_No.29]生産ライン対応 新規作成.
 2018/10/16 Sky松井   [Eagle_High][制御構成変更No.47]画パラ切り替え処理追加(EagleLowの時のみ).
================================================================================================*/
void SetMachineType( const char sc_Machine, const char sc_Type )
{
	// ラインモード起動かを判定する.
	Bool bl_LineMode = checkLineMode( False );
	if( bl_LineMode != True ){
		printf( "SetMachineType NG\n" );
		return;
	}

	// 機種情報、機種Noを登録する.
	Bool bl_SetMachineType = THRF_setMachineType( sc_Machine, sc_Type, False );
	if( bl_SetMachineType != True ){
		printf( "SetMachineType NG\n" );
		return;
	}
	
#if ( defined( DEF_EagleCus ) && !defined( DEF_S800Emu ) && defined( DEF_S800 ) && !defined( DEF_A800 ) ) // Eagle(1/2/2.5/0).
	// 画パラ切り替え実施.
	Bool bl_Ret = THRL_changeImageParamSymLinkMachine( sc_Type );
	if( bl_Ret != True ){
		printf( "ImageParamSymLink NG\n" );
	}
#endif // !defined( DEF_S800Emu ) && defined( DEF_EagleCus ) && defined( DEF_S800 ) && !defined( DEF_A800 ).

	printf( "SetMachineType OK\n" );

	return;
}

/*================================================================================================.
［機能］	ターミナルコマンド （ 静電TPのキャリブレーションを実行する ）.
--------------------------------------------------------------------------------------------------.
［引数］	なし .
［戻値］	なし .
［詳細］	静電TPのキャリブレーションを実行し、キャリブレーション実行済みフラグを更新する.
--------------------------------------------------------------------------------------------------.
 [履歴]
 2014/12/11 iguchi    [HeliosL][制御構成変更_No.29]生産ライン対応 新規作成.
 2015/11/16 furuichi  CQ.521884 パネルキャリブレーションNG対応.
================================================================================================*/
void ExecuteTPCalibration( void )
{
	// パネルハードウェア処理クラスのインスタンスのアドレスを取得する.
	CNVC_PanelHard* pc_PanelHard = CNVC_PanelHard::newInstance();
	if( pc_PanelHard == NULL ){
		printf( "ExecuteTPCalibration NG\n" );	
		return;
	}
		// パネルの種別を取得する.
	uint32_t ul_PanelType = pc_PanelHard->getTouchPanelType();
	if( ul_PanelType == PCGE_PanelType_MResistance ||
		ul_PanelType == PCGE_PanelType_SResistance ){ // 抵抗膜パネルのみ弾くように修正.
		printf( "ExecuteTPCalibration NG\n" );	
		return;
	}

	// キャリブレーションを実行する.
	Bool bl_CalibrationRequest = True;	// キャリブレーション実行要求用変数.
	int32_t si_CalibrationRet = sysBspIoctlPanelSio( &bl_CalibrationRequest, EXDB_PanelCalibration );
	if( si_CalibrationRet != 0 ){
		printf( "ExecuteTPCalibration NG\n" );	
		return;
	}

	SYSF_taskDelay((int32_t)(SYSF_sysClkRateGet() * 10000 / 1000 ));	// キャリブレーションが確実に完了する様に10秒間wait.

	// キャリブレーション実行済みフラグを、"実行済み"に設定する.
	NVDF_setStaticElectricCalibrationExecStatus( NVDE_CalibrationExecStatusOn );

	// キャリブレーション実行済みフラグに、"実行済み"が設定されているか確認する.
	char sc_CalibrationExecStatus = NVDF_getStaticElectricCalibrationExecStatus();
	if( sc_CalibrationExecStatus != NVDE_CalibrationExecStatusOn ){
		printf( "ExecuteTPCalibration NG\n" );	
		return;
	}

	printf( "ExecuteTPCalibration OK\n" );	

	return;
}
static struct	THRS_TrayPaperTypeTbl	// トレイ用紙種類のトレイと表示する文字列のテーブル.
{
	TYPE_FeedTray	uc_Tray;			// トレイ.
	char			*psc_TrayName;		// トレイ文字列.
} s_TrayPaperTypeTbl[] = {
	{	TYPE_FeedTrayManual,	"FeedTray Manual:" },		// 手差しトレイ.
	{	TYPE_FeedTray1,			"FeedTray Tray1 :" },		// トレイ１.
	{	TYPE_FeedTray2,			"FeedTray Tray2 :" },		// トレイ２.
	{	TYPE_FeedTray3,			"FeedTray Tray3 :" },		// トレイ３.
	{	TYPE_FeedTray4,			"FeedTray Tray4 :" },		// トレイ４.
// 2018/11/02 [DenebMLK][AR1055247]トレイ5追加対応 Sky新大阪 ->.
#if  defined(DEF_DenebMLKCus)
	{	TYPE_FeedTray5,			"FeedTray Tray5 :" },		// トレイ５.
#else
	{	TYPE_FeedTray5,			"FeedTray LCT   :" },		// ＬＣＴ.
#endif
// 2018/11/02 [DenebMLK][AR1055247]トレイ5追加対応 Sky新大阪 <-.
};
#define THRD_TrayPaperTypeSize	(sizeof(s_TrayPaperTypeTbl)/sizeof(THRS_TrayPaperTypeTbl))		// テーブルサイズ定義.
/*===============================================================================================.
［機能］	全トレイのデフォルトの用紙種類を設定する.
-------------------------------------------------------------------------------------------------.
［引数］	TYPE_FeedTray uc_setTray	指定トレイ.
［戻値］	設定成功：True / 設定失敗：False.
［詳細］	指定トレイにデフォルトの用紙種類を設定する.
［履歴］	.
			2015/01/21 鈴木理	[Poseidon] CQ507891 新規作成.
			2017/07/14 井柳		[IT5_4.1] [AR1026351] Eco設定のバックアップ更新処理の追加.
================================================================================================*/
static Bool THRL_setTrayPaperTypeDefault( void )
{
	// インスタンス取得とNULLチェック.
	APIC_FeedTray*	pc_FeedTray =	APIC_FeedTray::newInstance();
	if ( pc_FeedTray == NULL ) {
		return False;
	}
#if !defined(DEF_Zeus5BkCus)			//Poseidon機種以外.
	if ( pc_MachineSetting == NULL ) {
		return False;
	}

	// PrintJobSettingクラスインスタンスの取得とチェック.
	APIC_PrintJobSetting*	pc_PrintJobSetting =	APIC_PrintJobSetting::newInstance();
	if ( pc_PrintJobSetting == NULL ) {
		return False;
	}

	TYPS_PaperTypeNormalEx_Backup s_PaperTypeNormalEx_Backup;	// Eco設定(バックアップ)情報構造体.
	memset( &s_PaperTypeNormalEx_Backup, 0, sizeof(s_PaperTypeNormalEx_Backup) );
#endif	// defined(DEF_Zeus5BkCus).

	for( uchar uc_idx=0; uc_idx<THRD_TrayPaperTypeSize; uc_idx++ ) {
		// 指定トレイの用紙種類クラスの取得.
		TYPC_PaperType	c_PaperType =	pc_FeedTray->getPaperType( s_TrayPaperTypeTbl[uc_idx].uc_Tray );
		// 「普通紙」設定.
		c_PaperType.setPaperType( TYPE_PaperTypeNormal );
#if !defined(DEF_Zeus5BkCus)			//Poseidon機種以外.
		// 日本仕向けは「普通紙(Eco)」設定.
		if ( TYPE_SystemType_JP == pc_MachineSetting->getSystemType() ) {
			c_PaperType.setPaperTypeNormalEx( True );
		}
		else {	// 日本仕向け以外は「普通紙」のまま.
			c_PaperType.setPaperTypeNormalEx( False );
		}
		
		TYPE_FeedTray e_FeedTray = s_TrayPaperTypeTbl[uc_idx].uc_Tray;
		if(e_FeedTray < TYPE_FeedTrayMax) {
			s_PaperTypeNormalEx_Backup.b_PaperTypeNormalEx_Backup[ e_FeedTray ] 
				= c_PaperType.getPaperTypeNormalEx();	// Eco設定(バックアップ)情報構造体への保持.
		}else{
			printf( "\nsetTrayPaperTypeDefault TrayType Over[%d].\n", e_FeedTray );
			return False;
		}
#endif	// defined(DEF_Zeus5BkCus).
		// 指定給紙口に用紙種類を設定.
		pc_FeedTray->setPaperType( s_TrayPaperTypeTbl[uc_idx].uc_Tray, c_PaperType );
	}
#if !defined(DEF_Zeus5BkCus)		//Poseidon機種以外.
	// Eco設定(バックアップ)情報の設定.
	pc_PrintJobSetting->setPaperTypeNormalEx_Backup( s_PaperTypeNormalEx_Backup );
#endif	// defined(DEF_Zeus5BkCus).
	return True;
}

/*===============================================================================================.
［機能］	デフォルト用紙種類設定を行う.
-------------------------------------------------------------------------------------------------.
［引数］	なし.
［戻値］	なし.
［詳細］	全トレイにデフォルト用紙種類を設定する.
［履歴］	.
			2015/01/21 鈴木理	[Poseidon] CQ507891 新規作成.
================================================================================================*/
void setTrayPaperTypeDefault( void )
{
	Bool	bl_Result = THRL_setTrayPaperTypeDefault();		// トレイにデフォルト用紙を設定.

	// 終了ログを表示.
	if ( bl_Result == True ) {
		printf( "\nsetTrayPaperTypeDefault OK\n" );
	}
	else {
		printf( "\nsetTrayPaperTypeDefault NG\n" );
	}
}


/*===============================================================================================.
［機能］	全トレイのデフォルト用紙を表示する.
-------------------------------------------------------------------------------------------------.
［引数］	なし.
［戻値］	設定成功：True / 設定失敗：False.
［詳細］	設定された用紙種類を表示する.
［履歴］	.
			2015/01/21 鈴木理	[Poseidon] CQ507891 新規作成.
			2018/09/07 Sky門	【AR.1058539】【開発提起】【ラインモード】ターミナルコマンド「dspTrayPaperType」追加対応.
			2019/06/06 Sky		[EagleZ+/BKZ+] 機器構成変更No.1 コート紙対応(STEP1).
================================================================================================*/
static Bool THRL_dspTrayPaperType( void )
{
	// 用紙設定変換テーブル.
	static struct ST_GetPaperTypeTbl{
		TYPE_PaperType			e_PaperType_1st;
		TYPE_PaperType			e_PaperType_2nd;
		TYPE_CustomPaperType	e_CustomPaperType;
	}st_GetPaperTypeTbl[] = {
		{ TYPE_PaperTypeNormal,		TYPE_PaperTypeNormal_2nd,		TYPE_CustomPaperType_UnSelect	},	// [31]     ：普通紙(両面2面目).
		{ TYPE_PaperTypeNormalPlus,	TYPE_PaperTypeNormalPlus_2nd,	TYPE_CustomPaperType_UnSelect	},	// [92]     ：普通紙+(両面2面目)　(*A0162)(A0321).
		{ TYPE_PaperTypeThick1,		TYPE_PaperTypeThick1_2nd,		TYPE_CustomPaperType_UnSelect	},	// [32]     ：厚紙1(両面2面目).
		{ TYPE_PaperTypeThick1Plus,	TYPE_PaperTypeThick1Plus_2nd,	TYPE_CustomPaperType_UnSelect	},	// [33]     ：厚紙1+(両面2面目).
		{ TYPE_PaperTypeThick2,		TYPE_PaperTypeThick2_2nd,		TYPE_CustomPaperType_UnSelect	},	// [34]     ：厚紙2(両面2面目).
		{ TYPE_PaperTypeThick3,		TYPE_PaperTypeThick3_2nd,		TYPE_CustomPaperType_UnSelect	},	// [35]     ：厚紙3(両面2面目).
		{ TYPE_PaperTypeThick4,		TYPE_PaperTypeThick4_2nd,		TYPE_CustomPaperType_UnSelect	},	// [36]     ：厚紙4(両面2面目).
		{ TYPE_PaperTypeUser1,		TYPE_PaperTypeUser1_2nd,		TYPE_CustomPaperType_UnSelect	},	// [37]     ：ユーザー紙1(両面2面目).
		{ TYPE_PaperTypeUser2,		TYPE_PaperTypeUser2_2nd,		TYPE_CustomPaperType_UnSelect	},	// [38]     ：ユーザー紙2(両面2面目).
		{ TYPE_PaperTypeUser3,		TYPE_PaperTypeUser3_2nd,		TYPE_CustomPaperType_UnSelect	},	// [39]     ：ユーザー紙3(両面2面目).
		{ TYPE_PaperTypeUser4,		TYPE_PaperTypeUser4_2nd,		TYPE_CustomPaperType_UnSelect	},	// [40]     ：ユーザー紙4(両面2面目).
		{ TYPE_PaperTypeUser5,		TYPE_PaperTypeUser5_2nd,		TYPE_CustomPaperType_UnSelect	},	// [41]     ：ユーザー紙5(両面2面目).
		{ TYPE_PaperTypeUser6,		TYPE_PaperTypeUser6_2nd,		TYPE_CustomPaperType_UnSelect	},	// [42]     ：ユーザー紙6(両面2面目).
		{ TYPE_PaperTypeUser7,		TYPE_PaperTypeUser7_2nd,		TYPE_CustomPaperType_UnSelect	},	// [43]     ：ユーザー紙7（コート紙）(両面2面目).
		{ TYPE_PaperTypeCustom1,	TYPE_PaperTypeCustom1_2nd,		TYPE_CustomPaperType1			},	// [65]     ：カスタム1(両面2面目).
		{ TYPE_PaperTypeCustom2,	TYPE_PaperTypeCustom2_2nd,		TYPE_CustomPaperType2			},	// [66]     ：カスタム2(両面2面目).
		{ TYPE_PaperTypeCustom3,	TYPE_PaperTypeCustom3_2nd,		TYPE_CustomPaperType3			},	// [67]     ：カスタム3(両面2面目).
		{ TYPE_PaperTypeCustom4,	TYPE_PaperTypeCustom4_2nd,		TYPE_CustomPaperType4			},	// [68]     ：カスタム4(両面2面目).
		{ TYPE_PaperTypeCustom5,	TYPE_PaperTypeCustom5_2nd,		TYPE_CustomPaperType5			},	// [69]     ：カスタム5(両面2面目).
		{ TYPE_PaperTypeCustom6,	TYPE_PaperTypeCustom6_2nd,		TYPE_CustomPaperType6			},	// [70]     ：カスタム6(両面2面目).
		{ TYPE_PaperTypeCustom7,	TYPE_PaperTypeCustom7_2nd,		TYPE_CustomPaperType7			},	// [71]     ：カスタム7(両面2面目).
		{ TYPE_PaperTypeCustom8,	TYPE_PaperTypeCustom8_2nd,		TYPE_CustomPaperType8			},	// [72]     ：カスタム8(両面2面目).
		{ TYPE_PaperTypeCustom9,	TYPE_PaperTypeCustom9_2nd,		TYPE_CustomPaperType9			},	// [73]     ：カスタム9(両面2面目).
		{ TYPE_PaperTypeCustom10,	TYPE_PaperTypeCustom10_2nd,		TYPE_CustomPaperType10			},	// [74]     ：カスタム10(両面2面目).
		{ TYPE_PaperTypeCustom11,	TYPE_PaperTypeCustom11_2nd,		TYPE_CustomPaperType11			},	// [75]     ：カスタム11(両面2面目).
		{ TYPE_PaperTypeCustom12,	TYPE_PaperTypeCustom12_2nd,		TYPE_CustomPaperType12			},	// [76]     ：カスタム12(両面2面目).
		{ TYPE_PaperTypeCustom13,	TYPE_PaperTypeCustom13_2nd,		TYPE_CustomPaperType13			},	// [77]     ：カスタム13(両面2面目).
		{ TYPE_PaperTypeCustom14,	TYPE_PaperTypeCustom14_2nd,		TYPE_CustomPaperType14			},	// [78]     ：カスタム14(両面2面目).
		{ TYPE_PaperTypeCustom15,	TYPE_PaperTypeCustom15_2nd,		TYPE_CustomPaperType15			},	// [79]     ：カスタム15(両面2面目).
		{ TYPE_PaperTypeCustom16,	TYPE_PaperTypeCustom16_2nd,		TYPE_CustomPaperType16			},	// [80]     ：カスタム16(両面2面目).
		{ TYPE_PaperTypeCustom17,	TYPE_PaperTypeCustom17_2nd,		TYPE_CustomPaperType17			},	// [81]     ：カスタム17(両面2面目).
		{ TYPE_PaperTypeCustom18,	TYPE_PaperTypeCustom18_2nd,		TYPE_CustomPaperType18			},	// [82]     ：カスタム18(両面2面目).
		{ TYPE_PaperTypeCustom19,	TYPE_PaperTypeCustom19_2nd,		TYPE_CustomPaperType19			},	// [83]     ：カスタム19(両面2面目).
		{ TYPE_PaperTypeCustom20,	TYPE_PaperTypeCustom20_2nd,		TYPE_CustomPaperType20			},	// [84]     ：再生紙(両面2面目).
	};
	TYPE_PaperType			e_LogPaperType			= TYPE_PaperTypeNormal;
	TYPE_CustomPaperType	e_WorkCustumPaperType	= TYPE_CustomPaperType_UnSelect;
	Bool					bl_Feed2ndSidedPaper	= False;
	// インスタンス取得とNULLチェック.
	APIC_FeedTray*	pc_FeedTray =	APIC_FeedTray::newInstance();
	if ( pc_FeedTray == NULL ) {
		return False;
	}

	// 先頭に改行文字出力.
	printf( "\n" );
	for( uchar uc_idx=0; uc_idx<THRD_TrayPaperTypeSize; uc_idx++ ) {
		// 指定トレイの用紙種類クラスの取得.
		TYPC_PaperType	c_PaperType =	pc_FeedTray->getPaperType( s_TrayPaperTypeTbl[uc_idx].uc_Tray );
		e_LogPaperType 			= c_PaperType.getPaperType();			// 用紙タイプの取得.
		e_WorkCustumPaperType	= c_PaperType.getCustomPaperType();		// カスタム用紙タイプの取得.
		bl_Feed2ndSidedPaper	= c_PaperType.getFeed2ndSidedPaper();	// 両面2面目設定の取得.
		// ログ出力する用紙タイプを両面2面目の値に変換する.
		for( uchar uc_PaperTypeCnt = 0; uc_PaperTypeCnt < sizeof(st_GetPaperTypeTbl)/sizeof(ST_GetPaperTypeTbl); uc_PaperTypeCnt++ ){
			// カスタム用紙ではない場合.
			if( TYPE_CustomPaperType_UnSelect == e_WorkCustumPaperType ){
				// 両面2面目の場合.
				if( True == bl_Feed2ndSidedPaper ){
					// 出力値を両面2面目の設定値に変換.
					if( e_LogPaperType == st_GetPaperTypeTbl[uc_PaperTypeCnt].e_PaperType_1st ){
						e_LogPaperType = st_GetPaperTypeTbl[uc_PaperTypeCnt].e_PaperType_2nd;
						break;
					}
				}
			// カスタム用紙の場合.
			}else{
				// 用紙設定変換のカスタム用紙属性が一致する場合.
				if( e_WorkCustumPaperType == st_GetPaperTypeTbl[uc_PaperTypeCnt].e_CustomPaperType ){
					// 両面2面目の場合.
					if( True == bl_Feed2ndSidedPaper ){
						// 両面2面目の設定値に変換.
						e_LogPaperType = st_GetPaperTypeTbl[uc_PaperTypeCnt].e_PaperType_2nd;
						break;
					}else{
						// 通常の設定値に変換.
						e_LogPaperType = st_GetPaperTypeTbl[uc_PaperTypeCnt].e_PaperType_1st;
						break;
					}
				}
			}
		}
		// 用紙種類表示.
		printf( "%s PaperType [%d]", s_TrayPaperTypeTbl[uc_idx].psc_TrayName, e_LogPaperType );
		// (Eco)設定表示.
		if ( c_PaperType.getPaperTypeNormalEx() == True ) {
			printf( "(Eco)" );		// (Eco)設定：あり.
		}
		// 改行文字出力.
		printf( "\n" );
	}

	return True;
}

/*===============================================================================================.
［機能］	全トレイの用紙種類を表示する.
-------------------------------------------------------------------------------------------------.
［引数］	なし.
［戻値］	なし.
［詳細］	全トレイに設定された用紙種類を表示する.
［履歴］	.
			2015/01/21 鈴木理	[Poseidon] CQ507891 新規作成.
================================================================================================*/
void dspTrayPaperType( void )
{
	Bool	bl_Result = THRL_dspTrayPaperType();	// トレイのデフォルト用紙を表示.

	// エラーの場合に終了ログを表示.
	if ( bl_Result != True ) {
		printf( "\ndspTrayPaperType NG\n" );
	}
}
/*================================================================================================
［機能］	証明書有効期限を返す.
--------------------------------------------------------------------------------------------------
［引数］	short ss_Year	：作成日時(年)
		char sc_Month	：作成日時(月)
		char sc_Day	：作成日時(日)
		char sc_Hour	：作成日時(時)
		char sc_Minute	：作成日時(分)
		char sc_Second	：作成日時(秒)
［戻値］	証明書有効期限までの日数
［詳細］	証明書有効期限までの日数を返す
［ｳｫｰｸｽﾙｰ］	未実施
--------------------------------------------------------------------------------------------------
 [履歴]
 '15/01/28 sky O.Yamashita	 ターミナルコマンドの証明書有効期限の変更対応の為、新規作成.
================================================================================================*/
static ushort THRL_RemainingDays( short ss_Year, char sc_Month, char sc_Day, char sc_Hour, char sc_Minute, char sc_Second )
{
	// 作成日をtime_t型に変換.
	struct tm s_tmCreate;
	s_tmCreate.tm_year = ss_Year - 1900;		// 年（1900年から起算）.
	s_tmCreate.tm_mon  = sc_Month - 1;		// 月（ 1月＝0 ）.
	s_tmCreate.tm_mday = sc_Day;			// 日.
	s_tmCreate.tm_hour = sc_Hour;			// 時.
	s_tmCreate.tm_min  = sc_Minute;			// 分.
	s_tmCreate.tm_sec  = sc_Second;			// 秒.
	time_t timeCreate = mktime( &s_tmCreate );

	// 有効期限をtime_t型に変換.
	struct tm s_tmExpiration;
	s_tmExpiration.tm_year = THRD_TERM_VALIDITY_YEAR - 1900;	// 年（1900年から起算）.
	s_tmExpiration.tm_mon  = THRD_TERM_VALIDITY_MONTH - 1;		// 月（ 1月＝0 ）.
	s_tmExpiration.tm_mday = THRD_TERM_VALIDITY_DAY;		// 日.
	s_tmExpiration.tm_hour = THRD_TERM_VALIDITY_HOUR;		// 時.
	s_tmExpiration.tm_min  = THRD_TERM_VALIDITY_MINUTE;		// 分.
	s_tmExpiration.tm_sec  = THRD_TERM_VALIDITY_SECOND;		// 秒.
	time_t timeExpiration = mktime( &s_tmExpiration );

	// 時間差（秒）を算出.
	double sd_sec = difftime(timeExpiration,timeCreate);
	// 秒単位を日単位に変換.
	ushort us_daysleft = (ushort)(sd_sec / THRD_SECONDS_DAY);	// 1日の秒数で除算.

	// 0日の証明書は作成できないので補正する（有効期限当日の証明書が作成できない問題対応）.
	if(0 == us_daysleft) {
		us_daysleft = 1;
	}

	return	us_daysleft;
}

// ================================================================================================.
//! <!--機能--> 【ターミナルコマンド】Macアドレスを表示するコマンド.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param void.
//! <!--戻値-->.
//! \return void.
//! <!--詳細-->.
//!  Macアドレスを表示する\n.
//! <!--履歴-->.
//  2015/03/05	古市		[Helios]制御構成変更No.42 Macアドレス取得コマンド追加対応 新規作成.
//  2018/06/05	Sky 小久保	[Eagle]制御構成変更_No.89 生産コマンド対応 イコール追加、ハイフン削除.
// ================================================================================================.
void GetMacAddress()
{
	char	sc_MacAddress_Disp[THRD_MacAddress_Disp_Size] = { 0 };	// ターミナルへ表示する文字配列.
	uchar	uc_MacAddress[THRD_Get_MacAddress_Size] = { 0 };		// 取得したMacアドレスを格納する文字配列.
	
	// SPI-FlashからMacアドレスを取得する.
	uchar uc_Ret = THRG_getMachineInfo( THRD_Get_MacAddress_Offset, uc_MacAddress, THRD_Get_MacAddress_Size );
	if( ( uc_Ret != THRD_GET_SPI_OK )					// 取得OK.
		 &&( uc_Ret != THRD_GET_SPI_OK_AREA1_ERR )		// バックアップデータ取得OK：1面目にメモリ不整合.
		  &&( uc_Ret != THRD_GET_SPI_OK_AREA2_ERR )	){	// バックアップデータ取得OK：2面目にメモリ不整合.
		printf( "SystemErr\n" );						// GetMacAddressコマンドエラー.
        return;
	}	
	snprintf( sc_MacAddress_Disp, THRD_MacAddress_Disp_Size,"MacAddress = %02X%02X%02X%02X%02X%02X"
		, uc_MacAddress[0], uc_MacAddress[1], uc_MacAddress[2], uc_MacAddress[3], uc_MacAddress[4], uc_MacAddress[5]);
	printf("%s\n", sc_MacAddress_Disp );
	return;
}

// ================================================================================================.
//! <!--機能--> 【ターミナルコマンド】Macアドレス表示(無線LAN).
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! 基板に設定されている無線用のMacアドレスを表示する.
//! Macアドレスを大文字で表示する.
//! 無線アダプタ装着かつ、ネットワークI/F構成が以下の場合に、取得可能とする.
//!  有線+無線(子機モード)
//!  有線+無線(親機モード)
//!  有線+無線(Wi-Fi Direct)
//! <!--履歴-->.
//	2021/03/03	Sky	OP_BTS-31220 生産ライン用 APPコマンド追加.
// ================================================================================================.
void GetWlanMacAddress()
{
	APIC_ExNetworkSetting* pc_NetworkSetting = APIC_ExNetworkSetting::newInstance();
	if( NULL == pc_NetworkSetting )
	{
		printf("SystemErr\n");
		return;
	}

	char sc_MacAddress[32];
	memset( sc_MacAddress, 0, sizeof( sc_MacAddress ) );

	TYPE_NetworkIFMode e_NwIfMode = pc_NetworkSetting->getNetworkIFMode();
	printf("getNetworkIFMode=%d\n", e_NwIfMode);

 	switch ( e_NwIfMode )
	{
		case TYPE_NetworkIFMode_DualEthernetSta:	// デュアルIF構成 有線＋無線子機モード.
		case TYPE_NetworkIFMode_DualEthernetAp:		// デュアルIF構成 有線＋無線APモード.
		case TYPE_NetworkIFMode_DualEthernetWfd:	// デュアルIF構成 有線＋Wi-Fi Directモード.
			//MACアドレス取得.
			pc_NetworkSetting->getMacAddressEx( (uchar*)sc_MacAddress );
			printf("getMacAddressEx=%s\n", sc_MacAddress);
			break;

		default:
			printf("disconnected\n");
			return;
	}

	// MACアドレスからコロンを削除して出力.
	char sc_DispBuf[32] = "MacAddress = ";
	uint32_t i = 0;
	uint32_t pos = (uint32_t)strlen(sc_DispBuf);

	for( i=0; i < strlen(sc_MacAddress); i++ )
	{
		if( sc_MacAddress[i] == ':' )
		{
			continue;
		}
		else
		{
			sc_DispBuf[pos] = sc_MacAddress[i];
			pos++;
			if( pos >= sizeof(sc_DispBuf)-1 ){
				break;
			}
		}
	}
	sc_DispBuf[pos] = '\0';

    printf("%s\n", sc_DispBuf);
}

// ================================================================================================.
//! <!--機能--> 【ターミナルコマンド】Cコード、Pコール、JAM履歴の表示.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param void.
//! <!--戻値-->.
//! \return void.
//! <!--詳細-->.
//! Cコード、Pコール、JAM履歴を表示する.
//! <!--履歴-->.
//  2015/03/04	古市		[Helios]制御構成変更No.43 Cコード、Pコール、JAM履歴収集コマンド追加対応 新規作成.
//  2017/10/27	Sky松井		[Eagle]制御構成変更No.52 サービスモードのマシン状態／確認にて、気圧を表示できるようにする.
//  2017/11/09	Sky小久保	[Eagle]RQ-720 紙自動検知 ターミナルコマンド(GetTroubleJamHistory)受信、応答へ警告を追加.
//  2018/11/08	橋本裕		[EagleH]RQ-720 紙自動検知 メディア検知異常2(封筒異常検知追加).
//  2019/02/07  Sky千葉     [EagleH]機器構成変更No.30 テーブルに用紙幅センサ異常の要素追加.
//  2019/02/07  Sky千葉     [EagleH]機器構成変更No.31 テーブルにPH光学系汚れの要素追加.
//  2019/03/04  Sky合屋     [EagleH]機器構成変更No.29 警告カウンタ種別データテーブルにタッキングFAN異常を追加.
//  2019/07/02  Sky鳥居     [EagleZ+]機器構成変更No.2 警告カウンタ種別データテーブルに定着モータトルク異常追加.
// ================================================================================================.
int32_t GetTroubleJamHistory()
{
	ushort us_Index = 0;
	ushort us_Count = 0;
	uint32_t ul_WarningCnt = 0;
	
	TYPS_LogTrouble		st_LogTrouble;	// 時系列トラブルデータ.
	TYPS_LogJam 		st_LogJam;		// 時系列ＪＡＭデータ.
	// 警告カウンタ種別データを初期化.
	THRS_WariningCnt	st_WarningCntData[]= {
		{ TYPE_WarningCnt_WP5, 						THRD_WarningName_WP5 },
		{ TYPE_WarningCnt_WP6, 						THRD_WarningName_WP6 },
		{ TYPE_WarningCnt_WP7, 						THRD_WarningName_WP7 },
		{ TYPE_WarningCnt_WP8, 						THRD_WarningName_WP8 },
		{ TYPE_WarningCnt_WP9, 						THRD_WarningName_WP9 },
		{ TYPE_WarningCnt_WP21, 					THRD_WarningName_WP21 },
		{ TYPE_WarningCnt_WP22, 					THRD_WarningName_WP22 },
		{ TYPE_WarningCnt_WP23, 					THRD_WarningName_WP23 },
		{ TYPE_WarningCnt_WP24, 					THRD_WarningName_WP24 },
		{ TYPE_WarningCnt_WP25, 					THRD_WarningName_WP25 },
		{ TYPE_WarningCnt_WP26, 					THRD_WarningName_WP26 },
		{ TYPE_WarningCnt_WP27, 					THRD_WarningName_WP27 },
		{ TYPE_WarningCnt_WP28, 					THRD_WarningName_WP28 },
		{ TYPE_WarningCnt_WI1, 						THRD_WarningName_WI1 },
		{ TYPE_WarningCnt_PCHomeSensorErrorCMY, 	THRD_WarningName_PCHomeSensorErrorCMY },
		{ TYPE_WarningCnt_PCHomeSensorErrorK, 		THRD_WarningName_PCHomeSensorErrorK },
		{ TYPE_WarningCnt_1stTransferATVCCMY, 		THRD_WarningName_1stTransferATVCCMY },
		{ TYPE_WarningCnt_PriorDetectionDust, 		THRD_WarningName_PriorDetectionDust },
		{ TYPE_WarningCnt_ReadGuideWarning, 		THRD_WarningName_ReadGuideWarning },
		{ TYPE_WarningCnt_SkewReviseAbnormality, 	THRD_WarningName_SkewReviseAbnormality },
		{ TYPE_WarningCnt_ElectricCHCleaning1, 		THRD_WarningName_ElectricCHCleaning1 },
		{ TYPE_WarningCnt_ElectricCHCleaning2, 		THRD_WarningName_ElectricCHCleaning2 },
		{ TYPE_WarningCnt_FusingSensorNCDir, 		THRD_WarningName_FusingSensorNCDir },
		{ TYPE_WarningCnt_CISGain, 					THRD_WarningName_CISGain },
		{ TYPE_WarningCnt_PriorDetectionDustBack, 	THRD_WarningName_PriorDetectionDustBack },
		{ TYPE_WarningCnt_AbnormalLD, 				THRD_WarningName_AbnormalLD },
		{ TYPE_WarningCnt_AbnormalcyAirPressure,	THRD_WarningName_AbnormalcyAirPressure },
		// 2017/11/02 Sky小久保 紙種自動検知(メディアセンサ異常).
		{ TYPE_WarningCnt_MediaDetection, 			THRD_WarningName_MediaSensorError }, 
		{ TYPE_WarningCnt_MediaDetection2,			THRD_WarningName_MediaSensorError2 },
		{ TYPE_WarningCnt_PaperWidthError,			THRD_WarningName_PaperWidthError },
		{ TYPE_WarningCnt_OpticalSystemDirtP37,		THRD_WarningName_OpticalSystemDirtP37 },
		{ TYPE_WarningCnt_FanError,					THRD_WarningName_TackingFanError },
		{ TYPE_WarningCnt_MotorError,				THRD_WarningName_MotorError },

		{ TYPE_WarningCnt_InternalTemperatureSensorError,	THRD_WarningCnt_InternalTemperatureSensorError },
		{ TYPE_WarningCnt_PHTemperatureSensorError,			THRD_WarningCnt_PHTemperatureSensorError },
		{ TYPE_WarningCnt_PaperTemperatureSensorError,		THRD_WarningCnt_PaperTemperatureSensorError },
	};
	// トレイ名称データを初期化.
	THRS_FeedTray st_FeedTrayData[] = {
		{ TYPE_FeedTrayManual,	THRD_TrayName_Manual },
		{ TYPE_FeedTray1,		THRD_TrayName_Tray1 },
		{ TYPE_FeedTray2,		THRD_TrayName_Tray2 },
		{ TYPE_FeedTray3,		THRD_TrayName_Tray3 },
		{ TYPE_FeedTray4,		THRD_TrayName_Tray4 },
		{ TYPE_FeedTray5,		THRD_TrayName_Tray5 },
	};
	
	// 時系列トラブルデータ（ＡＰＩ）・インスタンス取得.
	APIC_LogTrouble*	pc_LogTrouble = APIC_LogTrouble::newInstance();
	if( pc_LogTrouble == NULL ){
		// エラー時ログを表示.
		printf( "SystemErr\n" );
		return 1;
	}
	// 警告カウンタ（ＡＰＩ）・インスタンス取得.
	APIC_WarningCnt*	pc_WarningCnt = APIC_WarningCnt::newInstance();
	if( pc_WarningCnt == NULL ){
		// エラー時ログを表示.
		printf( "SystemErr\n" );
		return 2;
	}
	// 時系列ＪＡＭデータ（ＡＰＩ）・インスタンス取得.
	APIC_LogJam*		pc_LogJam = APIC_LogJam::newInstance();
	if( pc_LogJam == NULL ){
		// エラー時ログを表示.
		printf( "SystemErr\n" );
		return 3;
	}
	
	// トラブルコード表示処理.
	printf("=== C-Code History ===\n");
	// リングバッファで制御されているので最新+1で最古インデックスを取得する.
	us_Index = (ushort)(pc_LogTrouble->newestDataPos() + 1); // インデックスとして用いるのでushortにキャストする.
	for( us_Count = 0; us_Count < TYPD_MaxLogTrouble; us_Count++){
		// インデックスが選択範囲を超えた場合は1に戻す.
		if( us_Index > TYPD_MaxLogTrouble ){
			us_Index = 1;
		}
		// トラブルコードを保持した構造体を取得.
		st_LogTrouble = pc_LogTrouble->getLogTrouble( us_Index );
		// トラブルコードが正常(0)以外かをチェック.
		if( st_LogTrouble.us_Code != 0 ){
			printf("%04X    %08"PRId32" %02d/%02d %02d:%02d\n", st_LogTrouble.us_Code
			                                   			, st_LogTrouble.ul_TotalCounter
			                                   			, st_LogTrouble.c_Date.sc_Month
			                                   			, st_LogTrouble.c_Date.sc_Day
			                                   			, st_LogTrouble.c_Date.sc_Hour
			                                   			, st_LogTrouble.c_Date.sc_Minute );
		}
		us_Index++;
	}
	
	// 警告カウント表示処理.
	printf("=== P-Call Counter ===\n");
	for( us_Count = 0; us_Count < sizeof(st_WarningCntData)/sizeof(THRS_WariningCnt); us_Count++ ){
		// 警告カウントを取得.
		ul_WarningCnt = pc_WarningCnt->get( st_WarningCntData[us_Count].e_WarningCntID );
		// 警告カウントが1以上なら警告カウンタ種別と警告カウントを表示.
		if( ul_WarningCnt >= 1 ){
			printf("%s    %"PRId32"\n", st_WarningCntData[us_Count].sc_WarningName, ul_WarningCnt );
		}
	}

	// JAM履歴表示処理.
	printf("=== Jam History ===\n");
	// リングバッファで制御されているので最新+1で最古インデックスを取得する.
	us_Index = (ushort)(pc_LogJam->newestDataPos() + 1); // インデックスとして用いるのでushortにキャストする.
	for( us_Count = 0; us_Count < TYPD_MaxLogJam; us_Count++){
		// インデックスが選択範囲を超えた場合は1に戻す.
		if( us_Index > TYPD_MaxLogJam ){
			us_Index = 1;
		}
		// JAM履歴を保持した構造体を取得.
		st_LogJam = pc_LogJam->getLogJam( us_Index );
		// JAMコードが正常(0)以外かをチェック.
		if( st_LogJam.us_Code != 0 ){
			if( st_LogJam.e_FeedTray < (sizeof(st_FeedTrayData)/sizeof(THRS_FeedTray)) ){
				printf("%04X    %"PRIu32" %02d/%02d %02d:%02d %s %d\n", st_LogJam.us_Code
				                                         , st_LogJam.ul_TotalCounter
				                                         , st_LogJam.c_Date.sc_Month
				                                         , st_LogJam.c_Date.sc_Day
				                                         , st_LogJam.c_Date.sc_Hour
				                                         , st_LogJam.c_Date.sc_Minute
				                                         , st_FeedTrayData[st_LogJam.e_FeedTray].sc_FeedTrayName
				                                         , st_LogJam.e_SizeCode );
			}
		}
		us_Index++;
	}
	
	return 0;
}


/*===============================================================================================.
［機能］	FWのセットアップ完了フラグの設定.
-------------------------------------------------------------------------------------------------.
［引数］	Bool	bl_Flag		セットアップ完了状態（True：完了／False：未完了）.
［戻値］	なし.
［詳細］	セットアップ完了状態の設定.
［履歴］	.
			2015/03/17	Ogura		Poseidon 生産ラインAPP開始できない問題対応(新規作成).
			2021/01/12	Sky			生産コマンドの投入可能タイミング変更対応.
================================================================================================*/
void THRF_SetApplicationStartUpFlag( Bool bl_Flag )
{
	bl_AppStartUpFlag = bl_Flag;

	// 両方TrueになったらApp1向けのログを出力する.
	if ( True == bl_AppStartUpFlag && True == bl_ConfigConfirmed )
	{
		SYSF_logMsg( "\nApplication StartUp End\n",1,2,3,4,5,6);
	}
}

/*===============================================================================================.
［機能］	構成確定状態を設定する.
-------------------------------------------------------------------------------------------------.
［引数］	Bool	bl_Flag		セットアップ完了状態（True：完了／False：未完了）.
［戻値］	なし.
［詳細］	PintP経由でメカコンからの構成確定をThreadに通知する.
			通常起動時、Warp起動時はクリアしておき、構成確定でTrueになる.
［履歴］	.
			2021/01/12	Sky		新規作成.生産コマンドの投入可能タイミング変更対応.
================================================================================================*/
void THRF_SetConfigurationConfirmed( Bool bl_Flag )
{
	bl_ConfigConfirmed = bl_Flag;
	SYSF_logMsg("(%010u)ConfigurationConfirmed(%d)\n", SYSF_tickGet(), bl_ConfigConfirmed);

	// 両方TrueになったらApp1向けのログを出力する.
	if ( True == bl_AppStartUpFlag && True == bl_ConfigConfirmed )
	{
		SYSF_logMsg( "\nApplication StartUp End\n",1,2,3,4,5,6);
	}
}

/*===============================================================================================.
［機能］	FWセットアップ完了状態の確認.
-------------------------------------------------------------------------------------------------.
［引数］	なし.
［戻値］	なし.
［詳細］	セットアップ完了状態の設定.
［履歴］	.
			2015/03/17	Ogura		Poseidon 生産ラインAPP開始できない問題対応(新規作成).
			2021/01/12	Sky			生産コマンドの投入可能タイミング変更対応.
================================================================================================*/
void isApplicationSetUp()
{
	if ( True == bl_AppStartUpFlag && True == bl_ConfigConfirmed ) {
		printf( "\nApplication StartUp Already Complete\n" );
	} else {
		printf( "\nApplication Initializing\n" );
		printf( "AppStartUp=%d, Configuration=%d\n", bl_AppStartUpFlag, bl_ConfigConfirmed );
	}
}
/*===============================================================================================.
［機能］	認証機能有効化表示.
-------------------------------------------------------------------------------------------------.
［引数］	なし.
［戻値］	なし.
［詳細］	AirPrint、Mopria、Wifi-Directの認証状態を表示する.
［履歴］	.
			2015/03/05	大岡	KRQ-101 認証機能有効化対応.
			2015/03/19	下垣	機能拡張判定を追加.
			2015/09/08	山下	MinervaSBK制御構成変更_No.71　Mopria1.3対応.
			2016/02/18	鈴木	[ZeusSBK]制御構成変更No.11 認証機能有効化.
================================================================================================*/
void dispFunction( void )
{
	APIC_SystemInfo*  pc_SystemInfo = APIC_SystemInfo::newInstance();
	if ( pc_SystemInfo == NULL ) {
		// 認証状態取得できないので、SystemErr表示して完了.
		printf("SystemErr\n");
		return;
	}
	// AirPrint 認証状態.
	char sc_AirPrintLicenceVer[TYPE_AirPrintLicenceVer_Size];
	memset( sc_AirPrintLicenceVer, 0x00, sizeof(sc_AirPrintLicenceVer) );
	TYPE_AirPrintLicence	e_AirPrint = pc_SystemInfo->getAirPrintLicenceVersion( sc_AirPrintLicenceVer );
	// 無効ではない場合にバージョン表示する.
	if ( e_AirPrint != TYPE_AirPrintLicence_None ) {
		printf("AirPrint%s\n", 	sc_AirPrintLicenceVer );
	}
	
	// Mopria 認証状態.
	char sc_MopriaLicenceVer[TYPE_MopriaLicenceVer_Size];
	memset( sc_MopriaLicenceVer, 0x00, sizeof(sc_MopriaLicenceVer) );
	TYPE_MopriaLicence		e_Mopria = pc_SystemInfo->getMopriaLicenceVersion( sc_MopriaLicenceVer );
	// 無効ではない場合にバージョン表示する.
	if ( e_Mopria != TYPE_MopriaLicence_None ) {
		printf( "Mopria%s\n", sc_MopriaLicenceVer );
	}
	
	// Wifi-Direct 認証状態.
	TYPE_WfdLicence			e_Wfd = pc_SystemInfo->getWfdLicenceStatus();
	if ( e_Wfd == TYPE_WfdLicence_Enable ) {
		printf("Wi-Fi Direct\n");
	}
}
/*===============================================================================================.
［機能］	認証機能有効化の結果通知用のコールバック.
-------------------------------------------------------------------------------------------------.
［引数］	TYPE_LicenceFunctionEnable e_Result.実行結果.
［戻値］	なし.
［詳細］	AirPrint、Mopria、Wifi-Directの認証機能を有効化の実行結果を通知する.
［履歴］	.
			2015/03/19	下垣	KRQ-101 認証機能有効化対応.
================================================================================================*/
void THRF_activateFunctionResult( TYPE_LicenceFunctionEnable e_Result )	{

	printf( "\n" );
	// コマンド成功.
	printf(" activateFunction OK\n");

}
/*===============================================================================================.
［機能］	認証機能有効化設定.
-------------------------------------------------------------------------------------------------.
［引数］	const char* pc_funccode.機能コード.
［戻値］	なし.
［詳細］	AirPrint、Mopria、Wifi-Directの認証機能を有効化する.
［履歴］	.
			2015/03/19	下垣	KRQ-101 認証機能有効化対応.
================================================================================================*/
void activateFunction(const char* pc_FuncCode){

	Bool bl_CommandChk = False;	//コマンド成否 初期値 失敗.
	char sc_FunCode_tmp[TYPD_FunctionCodeMaxSize] = {0};
	memset( sc_FunCode_tmp, 0 ,sizeof(sc_FunCode_tmp));

	if (pc_FuncCode != NULL){	// 未入力チェック.
		if (TYPD_FunctionCodeMaxSize > strlen(pc_FuncCode)){	//上限チェック(TYPD_FunctionCodeMaxSizeは終端(NULL)も含んだ値のため、未満でチェック).
			strncpy(sc_FunCode_tmp, pc_FuncCode, strlen(pc_FuncCode));
		} else {
			// 上限を超えている場合は32バイトまでを有効としてコピーする.
			strncpy(sc_FunCode_tmp, pc_FuncCode, TYPD_FunctionCodeMaxSize-1);
		}
		// 認証機能有効化.
		APIC_Enabler* pc_Enabler = APIC_Enabler::newInstanceAdmin();
		if( pc_Enabler == NULL ){
			return;
		}
		pc_Enabler->req_LicenceFunctionEnable((const uchar *)sc_FunCode_tmp, False, THRF_activateFunctionResult);
		bl_CommandChk = True;//コマンド成否 成功.
	}
	if ( True == bl_CommandChk ){
		// コマンド成功 (OK表示はコールバック関数内).
	} else {
		// 改行追加.
		printf( "\n" );
		// コマンド失敗.
		printf(" activateFunction NG\n");
	}
}

/******************************************************************************
* Function Name [ THRG_ATADebug ]
* Summary       [ SATAコマンド テスト処理(ターミナルから実行用) ]
* Input Param   [ int si_argc1 : デバイス番号 ]
*               [ int si_argc2 : デバイス数 ]
* OutPut Param  [ なし ]
* Return        [ 0 : 正常 
*                 -1: 異常終了 ]
* Make Day      [ 2014/09/18 ]
* Modificate History
* [ 2014/09/18 森川(義)   ZeusS 制御構成変更No.4 HDDミラーリング対応 ]
* [ 2015/05/09 山下  MinervaSBK 「sysBspHdd.c」から「THRC_Vxworks.cpp」に移動]
******************************************************************************/
extern "C" int32_t BspHddReadWrite(unsigned char uc_DevNo);

int32_t THRG_ATADebug(int32_t si_argc1, int32_t si_argc2)
{
	uint32_t ul_ret = 0;
	int32_t si_no;
	int32_t uc_devNo = 0; //2017/01/23 unsigned charからint32_tに変更.
	ST_HDD_DEVICE_INFO stDeviceInfo;
	unsigned char uc_pass[32] = "12345678901234567890";
	THRS_HDD_DEVICE_RAID_INFO st_RaidInfo = {0};	// RAID情報.
	THRS_HDD_DEVICE_RBLD_INFO st_RbldInfo = {0};	// Rebuild情報.
	unsigned char uc_deviceCount = 0;

	if ((si_argc1 > 25) || (si_argc1 < 1) )
	{
		printf("Usage: ATADebug {no(0-25), devno(0-2)}\n");
		printf("1: sysBspHddDeviceInfoGet\n");
		printf("2: sysBspHddPasswordSet\n");
		printf("3: sysBspHddUnlock\n");
		printf("4: sysBspHddDisablePassword\n");
		printf("5: sysBspHddPowerSleep\n");
		printf("6: sysBspHddPowerActive\n");
		printf("7: sysBspHddEraseUnit\n");
		printf("8: sysBspHddErasePrepare\n");
		printf("9: sysBspHddAPMEnable\n");
		printf("10: sysBspHddAPMDisable\n");
		printf("11: HDD READ WRITE\n");
		printf("12: sysBspHddRaidInfo\n");
		printf("13: sysBspHddChangeRaidMode(INDIVIDUAL)\n");
		printf("14: sysBspHddChangeRaidMode(RAID-1)\n");
		printf("15: sysBspChangeMasterChanel(Ch-A)\n");
		printf("16: sysBspChangeMasterChanel(Ch-B)\n");
		printf("17: sysBspGetRebuildInfo\n");
		printf("18: sysBspStartRebuild(Initial)\n");
		printf("19: sysBspStartRebuild(Continue)\n");
		printf("20: sysBspStopRebuild\n");
		printf("21: sysBspHddSelectChanel(Ch-A)\n");
		printf("22: sysBspHddSelectChanel(Ch-B)\n");
		printf("23: sysBspHddSetupRebuild(Initial Rebuild Enable)\n");
		printf("24: sysBspHddSetupRebuild(Initial Rebuild Disable)\n");
		printf("25: sysBspHddGetDriveInfo\n");
		return -1;
	}

	si_no = si_argc1;
	uc_devNo = si_argc2;
	if (uc_devNo >= HDD_MAX_DEVICE_NUM) {
		printf("DeviceNo is invalid.\n");
		return -1;
	}

	switch(si_no)
	{
	case 1:
		ul_ret = sysBspHddDeviceInfoGet(uc_devNo, 0, &stDeviceInfo);
		printf("VenderID          = %s\n", stDeviceInfo.ctHddVendorId);
		printf("ProdactID         = %s\n", stDeviceInfo.ctHddProductId);
		printf("cDeviceType       = %d\n", stDeviceInfo.cDeviceType);
		printf("iTotalBlocks      = %d\n", stDeviceInfo.iTotalBlocks);
		printf("iBlockSize        = %d\n", stDeviceInfo.iBlockSize);
		printf("us_Construct      = %d\n", stDeviceInfo.us_Construct);
		printf("us_CylinderNum    = %d\n", stDeviceInfo.us_CylinderNum);
		printf("us_HeadNum        = %d\n", stDeviceInfo.us_HeadNum);
		printf("us_SectorPerTrack = %d\n", stDeviceInfo.us_SectorPerTrack);
		printf("us_SecurityStatus = %d\n", stDeviceInfo.us_SecurityStatus);
		break;
	case 2:
		ul_ret = sysBspHddPasswordSet(uc_devNo, 0, uc_pass, 0);
		break;
	case 3:
		ul_ret = sysBspHddUnlock(uc_devNo, 0, uc_pass);
		break;
	case 4:
		ul_ret = sysBspHddDisablePassword(uc_devNo, 0, uc_pass);
		break;
	case 5:
		ul_ret = sysBspHddPowerSleep(uc_devNo, HDD_SLEEPMODE_STANDBY_IMMEDIATE);
		break;
	case 6:
		ul_ret = sysBspHddPowerActive(uc_devNo);
		break;
	case 7:
		ul_ret = sysBspHddEraseUnit(uc_devNo, 0, uc_pass);
		break;	
	case 8:
		ul_ret = sysBspHddErasePrepare(uc_devNo);
		break;
	case 9:
		ul_ret = sysBspHddAPMEnable(uc_devNo, 0xFE);
		break;
	case 10:
		ul_ret = sysBspHddAPMDisable(uc_devNo);
		break;
	case 11:
		ul_ret = BspHddReadWrite(uc_devNo);
		break;
	// 2014/09/08 ZeusS 制御構成変更No.4 HDDミラーリング対応.
	case 12:
		ul_ret = sysBspHddRaidInfo(uc_devNo, &st_RaidInfo);
		printf("HddStructure       = %d\n", st_RaidInfo.uc_HddStructure);
		printf("DeviceStatus       = %d\n", st_RaidInfo.uc_DeviceStatus);
		printf("MasterChanel       = %d\n", st_RaidInfo.uc_MasterChanel);
		printf("InitialRebuildFlag = %d\n", st_RaidInfo.uc_InitialRebuildFlag);
		printf("AutoRebuildFlag    = %d\n", st_RaidInfo.uc_AutoRebuildFlag);
		printf("UnitSize           = %"PRId32"\n", st_RaidInfo.ul_UnitSize);
		printf("Restart            = %d\n", st_RaidInfo.uc_Restart);
		printf("ErrRetryNum        = %d\n", st_RaidInfo.uc_ErrRetryNum);
		printf("DataSaveNum        = %d\n", st_RaidInfo.us_DataSaveNum);
		printf("WaitFlg            = %d\n", st_RaidInfo.uc_WaitFlg);
		printf("WaitParam          = %d\n", st_RaidInfo.us_WaitParam);
		printf("Devide2TB          = %d\n", st_RaidInfo.uc_Devide2TB);
		printf("HddStatusChA       = %d\n", st_RaidInfo.uc_HddStatusChA);
		printf("HddStatusChB       = %d\n", st_RaidInfo.uc_HddStatusChB);
		break;
	case 13:
		ul_ret = sysBspHddChangeRaidMode(uc_devNo, THRD_HDD_RAID_MODE_INDIVIDUAL);
		break;
	case 14:
		ul_ret = sysBspHddChangeRaidMode(uc_devNo, THRD_HDD_RAID_MODE_RAID1);
		break;
	case 15:
		ul_ret = sysBspChangeMasterChanel(uc_devNo, DEF_DEVICEID_HDD);
		break;
	case 16:
		ul_ret = sysBspChangeMasterChanel(uc_devNo, DEF_DEVICEID_HDD_CH_B);
		break;
	case 17:
		ul_ret = sysBspGetRebuildInfo(uc_devNo, &st_RbldInfo);
		printf("RebuildRunning       = %d\n", st_RbldInfo.uc_RebuildRunning);
		printf("RebuildLBAAddressH   = %d\n", st_RbldInfo.us_RebuildLBAAddressH);
		printf("RebuildLBAAddressM   = %d\n", st_RbldInfo.us_RebuildLBAAddressM);
		printf("RebuildLBAAddressL   = %d\n", st_RbldInfo.us_RebuildLBAAddressL);
		printf("RebuildResult        = %d\n", st_RbldInfo.uc_RebuildResult);
		printf("RebuildErrorTimes    = %d\n", st_RbldInfo.uc_RebuildErrorTimes);
		printf("RebuildStartError    = %d\n", st_RbldInfo.uc_RebuildStartError);
		printf("RebuildErrorLogCount = %d\n", st_RbldInfo.uc_RebuildErrorLogCount);
		printf("RaidBaseSizeH        = %d\n", st_RbldInfo.us_RaidBaseSizeH);
		printf("RaidBaseSizeM        = %d\n", st_RbldInfo.us_RaidBaseSizeM);
		printf("RaidBaseSizeL        = %d\n", st_RbldInfo.us_RaidBaseSizeL);
		break;
	case 18:
		ul_ret = sysBspStartRebuild(uc_devNo, 0);
		break;
	case 19:
		ul_ret = sysBspStartRebuild(uc_devNo, 1);
		break;
	case 20:
		ul_ret = sysBspStopRebuild(uc_devNo);
		break;
	case 21:
		ul_ret = sysBspHddSelectChanel(uc_devNo, DEF_DEVICEID_HDD);
		break;
	case 22:
		ul_ret = sysBspHddSelectChanel(uc_devNo, DEF_DEVICEID_HDD_CH_B);
		break;
	case 23:
		// case 12 のリビルド動作の情報、状態を取得を実行後に動作させる事.
		st_RaidInfo.uc_InitialRebuildFlag = 1;
		ul_ret = sysBspHddSetupRebuild(uc_devNo, &st_RaidInfo);
		break;
	case 24:
		// case 12 のリビルド動作の情報、状態を取得を実行後に動作させる事.
		st_RaidInfo.uc_InitialRebuildFlag = 0;
		ul_ret = sysBspHddSetupRebuild(uc_devNo, &st_RaidInfo);
		break;
	case 25:
		ul_ret = sysBspHddGetDriveInfo(uc_devNo, &uc_deviceCount);
		printf("DeviceCount       = %d\n", uc_deviceCount);
		break;
	}
	
	printf("ul_ret = 0x%08"PRIX32"\n",ul_ret);
	
	return 0;
}

/*===============================================================================================.
［機能］	【ターミナルコマンド】認定機能管理を無効化するコマンド.
-------------------------------------------------------------------------------------------------.
［引数］	なし.
［戻値］	なし.
［詳細］	AirPrintなど認証取得後に有効化する機能を無効（デフォルト機能のみ）に設定変更を行う.
［履歴］	.
			2015/06/05	古市	KRQ-101 認証機能無効化対応.
			                    拡張機能2版以降のみ本コマンドを受け付ける.
================================================================================================*/
void ClearAllFunctions()
{
	//ラインモードかチェックする.
	if( THRL_Drvrif_IsLineMode() == 1 ){
		//AirPrint/Mopria/Wi-Fi Directの各ライセンス 有効状態をデフォルトへリセットする.
		APIC_SystemInfo *pc_SystemInfo = APIC_SystemInfo::newInstance();
		if( pc_SystemInfo != NULL ){
			pc_SystemInfo->clearLicenceFuncEnable();
			printf( "ClearAllFunctions OK\r\n" );
		} else {
			printf( "SystemErr\r\n" );
		}
	} else {
		printf( "Not Line Mode!!\r\n" );
	}
}

/*===============================================================================================.
［機能］	debugLog 90の格納先バッファの取得.
-------------------------------------------------------------------------------------------------.
［引数］	なし.
［戻値］	ST_DEBUGSEQ*	debugLog 90の格納先バッファ.
［詳細］	debugLog 90の次に格納するログバッファを返す.
［履歴］	.
			2015/06/12	Ogura		ログのeMMC保存対応.
================================================================================================*/
ST_DEBUGSEQ* THRL_GetDebugLog90Buf()
{
	return pst_DebugLowSeq;
}
/*===============================================================================================.
［機能］	debugLog 92の格納先インデックスの取得.
-------------------------------------------------------------------------------------------------.
［引数］	なし.
［戻値］	uint32_t	debugLog 90のバッファインデックス.
［詳細］	debugLog 90の次に格納するログのインデックスを返す.
［履歴］	.
			2015/06/12	Ogura		ログのeMMC保存対応.
================================================================================================*/
uint32_t THRL_GetDebugLog92Index()
{
	return st_debugLog92_Index.us_DebugLowLogSeqIdx % DEF_DebugLog92Seq_MAX;
}

/*===============================================================================================.
［機能］	debugLog 92の格納先バッファの取得.
-------------------------------------------------------------------------------------------------.
［引数］	なし.
［戻値］	ST_DEBUGSEQ*	debugLog 90の格納先バッファ.
［詳細］	debugLog 90の次に格納するログバッファを返す.
［履歴］	.
			2015/06/12	Ogura		ログのeMMC保存対応.
================================================================================================*/
ST_DEBUGSEQ* THRL_GetDebugLog92Buf()
{
	return st_DebugLowLogSeq;
}

// ================================================================================================.
//! <!--機能--> USBメモリからOEM関連のデータをインストールする.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param  char	psc_path	サブフォルダ名.
//! <!--戻値-->.
//! \return.
//! <!--詳細-->.
//! 以下のOEMファイルをサブフォルダに格納し、インストールする.
//! OEMドライバ名称カスタマイズデータ、OEMマシン図データ、BootUpScreen.
//! <!--履歴-->.
//  2015/09/04	yamashita	新規作成(MinervaSBK制御構成変更No.70).
// ================================================================================================.
void installOemData( char *psc_Path )
{
	char	sc_FileName[DEF_PATH_MAX] = {0};
	char	sc_SearchPath[DEF_PATH_MAX] = {0};
	char	sc_InstallPath[DEF_PATH_MAX] = {0};
	Bool	bl_Ret = FALSE;
	uint32_t	ui_Delay = 0;

	if(psc_Path == NULL) {
		printf(" Parameter Error\n");
		return;
	}

	//ファイル検索用のパス作成.
	snprintf( sc_SearchPath, DEF_PATH_MAX, "%s/%s/", THRD_DevName_USB , psc_Path );
	//OEMドライバ名称カスタマイズデータをインストール.
	//ファイル検索.
	bl_Ret = THRL_searchFileFromDirectory( sc_FileName , DEF_PATH_MAX , sc_SearchPath , THRD_SearchFileOemData );
	if(bl_Ret){
		//USBデバイスからの相対パス作成.
		snprintf( sc_InstallPath, DEF_PATH_MAX, "%s/%s", psc_Path , sc_FileName );
		//ファイル確認.
		bl_Ret = (char)isExistOemData( sc_InstallPath );
		if(bl_Ret == TRUE){
			//ファイルインストール.
			bl_Ret = (char)downloadOemData( sc_InstallPath );
			if(bl_Ret == TRUE){
				printf(" OemString:%s/%s OK\n", psc_Path , sc_FileName );
			}else{
				printf(" OemString:%s/%s NG\n", psc_Path , sc_FileName );
			}
		}else{
			printf(" OemString:%s/%s NG\n", psc_Path , sc_FileName );
		}
	}else{
		printf(" OemString:%s NG\n", psc_Path );
	}

	//OEMマシン図データをインストール.
	//ファイル検索.
	bl_Ret = THRL_searchFileFromDirectory( sc_FileName , DEF_PATH_MAX , sc_SearchPath , THRD_SearchFileCustomImage );
	if(bl_Ret){
		//USBデバイスからの相対パス作成.
		snprintf( sc_InstallPath, DEF_PATH_MAX, "%s/%s", psc_Path , sc_FileName );

		//コールバック待ち用のステータス初期化.
		uc_OemDataCallbackRet = THRD_CallbackUnknown;
		ui_Delay = 0;

		//OEMマシン図データインストール（結果はコールバック）.
		pc_MachineSetting->saveHDDdata(TYPE_CharSetUTF8, sc_InstallPath, THRD_MachineSsdDir, THRL_OemDataCallback, True);
		//コールバック待ち.
		while(uc_OemDataCallbackRet == THRD_CallbackUnknown){
			SYSF_taskDelay(THRD_OEMTaskDelayTick);
			ui_Delay++;
			if(ui_Delay > THRD_OEMTimeOutCount){
				THRL_ThLogMsgErr( "installOemData saveHDDdata Timeout.\n" );
				uc_OemDataCallbackRet = THRD_CallbackTimeout;
			}
		}

		//インストール結果を表示.
		if(uc_OemDataCallbackRet == THRD_CallbackSuccess){
			//OEM用マシン図データインストール済フラグをTRUEに設定.
			pc_MachineSetting->setOEMMachineFigDataInstallFlgInfo(TRUE);
			printf(" CustomizeScreen:%s/%s OK\n",  psc_Path, sc_FileName );
		}else{
			printf(" CustomizeScreen:%s/%s NG\n",  psc_Path, sc_FileName );
		}
	}else{
		printf(" CustomizeScreen:%s NG\n", psc_Path );
	}

	//BootUpScreenをインストール.
	//ファイル検索.
	bl_Ret = THRL_searchFileFromDirectory( sc_FileName , DEF_PATH_MAX , sc_SearchPath , THRD_SearchFileBootUpScreen );
	if(bl_Ret){
		TYPS_BootUpScreenZipFileName	s_BootUpScreenZipFileName;
		TYPS_BootUpScreenBinFileName	s_BootUpScreenBinFileName;
		size_t ul_Len = 0;

		//文字列長を使用して、拡張子".zip"を削除する.
		ul_Len = strlen(sc_FileName);
		sc_FileName[(ul_Len - strlen(THRD_ExtensionPeriodZip) )] = '\0';

		//BootUp Screen Zipファイル用 構造体設定.
		s_BootUpScreenZipFileName.psc_ZipFileName		=	sc_FileName;				//	zipファイル名（拡張子なし）.
		s_BootUpScreenZipFileName.psc_ZipFileExtension	=	THRD_ZipFileExtension;		//	zipの拡張子.
		s_BootUpScreenZipFileName.psc_RecFileExtension	=	THRD_RecFileExtension;		//	zip解凍後の拡張子.
		//BootUp Screen Binファイル用 構造体設定.
		s_BootUpScreenBinFileName.psc_BinFileName		=	sc_FileName;				//	binファイル名（拡張子なし）.
		s_BootUpScreenBinFileName.psc_BinFileExtension	=	THRD_RecFileExtension;		//	binの拡張子.

		//コールバック待ち用のステータス初期化.
		uc_OemDataCallbackRet = THRD_CallbackUnknown;
		ui_Delay = 0;

		//BootUpScreenインストール（結果はコールバック）.
		pc_BootupScreen->writeBootupScreen( sc_SearchPath , s_BootUpScreenZipFileName , s_BootUpScreenBinFileName , THRD_VerFileName , THRL_OemDataCallback );

		//コールバック待ち.
		while(uc_OemDataCallbackRet == THRD_CallbackUnknown){
			SYSF_taskDelay(THRD_OEMTaskDelayTick);
			ui_Delay++;
			if(ui_Delay > THRD_OEMTimeOutCount){
				THRL_ThLogMsgErr( "installOemData writeBootupScreen Timeout.\n" );
				uc_OemDataCallbackRet = THRD_CallbackTimeout;
			}
		}

		//インストール結果を表示.
		if(uc_OemDataCallbackRet == THRD_CallbackSuccess){
			printf(" BootUpScreen:%s/%s OK\n", psc_Path, sc_FileName );
		}else{
			printf(" BootUpScreen:%s/%s NG\n", psc_Path, sc_FileName );
		}

	}else{
		printf(" BootUpScreen:%s NG\n", psc_Path );
	}
}

// ================================================================================================.
//! <!--機能--> OEM関連データーをMFPからアンインストールする.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param.
//! <!--戻値-->.
//! \return.
//! <!--詳細-->.
//! 以下のOEMファイルをアンインストールする.
//! OEMドライバ名称カスタマイズデータ、OEMマシン図データ、BootUpScreen.
//! <!--履歴-->.
//  2015/09/04	yamashita	新規作成(MinervaSBK制御構成変更No.70).
// ================================================================================================.
void uninstallOemData( void )
{
	Bool	bl_Ret = FALSE;
	uint32_t	ui_Delay = 0;

	//OEMドライバ名称カスタマイズデータをアンインストール.
	//OEMドライバ名称カスタマイズデータを削除.
	bl_Ret = (char)removeOemData();
	if(bl_Ret == TRUE){
		printf(" OemString OK\n");
	}else{
		printf(" OemString NG\n");
	}

	//OEMマシン図データをアンインストール.
	//OEMマシン図データのインストール済みかチェック.
	bl_Ret = pc_MachineSetting->getOEMMachineFigDataInstallFlgInfo();
	if(bl_Ret == TRUE){
		//コールバック待ち用のステータス初期化.
		uc_OemDataCallbackRet = THRD_CallbackUnknown;
		ui_Delay = 0;

		//OEMマシン図データを削除（結果はコールバック）.
		pc_MachineSetting->eraseHddArea( THRD_MachineSsdDirWithFilename, THRL_OemDataCallback, True );

		//コールバック待ち.
		while(uc_OemDataCallbackRet == THRD_CallbackUnknown){
			SYSF_taskDelay(THRD_OEMTaskDelayTick);
			ui_Delay++;
			if(ui_Delay > THRD_OEMTimeOutCount){
				THRL_ThLogMsgErr( "uninstallOemData eraseHddArea Timeout.\n" );
				uc_OemDataCallbackRet = THRD_CallbackTimeout;
			}
		}

		if(uc_OemDataCallbackRet == THRD_CallbackSuccess){
			//OEM用マシン図データインストール済フラグをFALSEに設定.
			pc_MachineSetting->setOEMMachineFigDataInstallFlgInfo(FALSE);
			printf(" CustomizeScreen  OK\n" );
		}else{
			printf(" CustomizeScreen  NG\n" );
		}
	}else{	//インストールされていない場合はOKを返す.
		printf(" CustomizeScreen  OK\n" );
	}

	//BootUpScreenをアンインストール.
	//コールバック待ち用のステータス初期化.
	uc_OemDataCallbackRet = THRD_CallbackUnknown;
	ui_Delay = 0;

	//BootUpScreenデータを削除（結果はコールバック）.
	pc_BootupScreen->eraseBootupScreenVersion( THRL_OemDataCallback);

	//コールバック待ち.
	while(uc_OemDataCallbackRet == THRD_CallbackUnknown){
		SYSF_taskDelay(THRD_OEMTaskDelayTick);
		ui_Delay++;
		if(ui_Delay > THRD_OEMTimeOutCount){
			THRL_ThLogMsgErr( "uninstallOemData eraseBootupScreenVersion Timeout.\n" );
			uc_OemDataCallbackRet = THRD_CallbackTimeout;
		}
	}

	if(uc_OemDataCallbackRet == THRD_CallbackSuccess){
		printf(" BootUpScreen OK\n" );
	}else{
		printf(" BootUpScreen NG\n" );
	}
}

// ================================================================================================.
//! <!--機能--> OEMデータインストール結果のコールバック関数.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param  Bool	bl_Status	インストール結果.
//! <!--戻値-->.
//! \return.
//! <!--詳細-->.
//! .
//! <!--履歴-->.
//  2015/09/04	yamashita	新規作成(MinervaSBK制御構成変更No.70).
// ================================================================================================.
static void THRL_OemDataCallback( Bool bl_Status )
{
	if(bl_Status == TRUE){
		uc_OemDataCallbackRet = THRD_CallbackSuccess;
	}else{
		uc_OemDataCallbackRet = THRD_CallbackError;
	}
}

// ================================================================================================.
//! <!--機能--> 指定したフォルダから条件に一致したファイル名を取得する.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param.
//! char *psc_findFileName,		(in/out)検索結果ファイル格納用バッファ.
//! uint32_t ul_findFileNameLen,	(in)	検索結果ファイル格納用バッファ長.
//! char *psc_searchPath,		(in)	検索するフォルダ.
//! char *psc_searchFileName,	(in)	検索するファイル名(ワイルドカード「*」使用可能).
//! <!--戻値-->.
//! \return Bool ファイル検索の成否(TRUE/FALSE).
//! <!--詳細-->.
//! 指定された条件に合致したファイル名を取得して、第一引数のバッファアドレスに格納する.
//! 第二引数に指定したファイル名長以上のファイル名は検索対象外とする.
//! 第三引数には、検索するフォルダを指定する.
//! 第四引数には、検索するファイル名を指定する.
//! 第四引数のファイル名はワイルドカードとして「*」が指定可能である.
//! 例)."customimage*.tar"、"*.tar"、"customimage*"等.
//! <!--履歴-->.
//  2015/09/04	yamashita	新規作成(MinervaSBK制御構成変更No.70).
// ================================================================================================.
Bool THRL_searchFileFromDirectory( char *psc_FindFileName,uint32_t ul_FindFileNameLen, char *psc_SearchPath,char *psc_SearchFileName )
{
	Bool bl_Ret = FALSE;
	DIR	*dir = NULL;
	struct dirent *dp = NULL;
	char	*psc_Search = NULL;
	char	*psc_RearSearchName = NULL;
	char	*psc_FolderFileName = NULL;
	uint32_t	ul_Wildcard = 0;	//ワイルドカードの位置.
	char	sc_SearchName[DEF_PATH_MAX] = {0};

	//引数チェック.
	if( (psc_FindFileName == NULL) ||
		(ul_FindFileNameLen == 0) ||
		(psc_SearchPath == NULL) ||
		(psc_SearchFileName == NULL) ){
		THRL_ThLogMsgErr( "THRL_searchFileFromDirectory para err %"PRIdPTR",%"PRId32",%"PRIdPTR",%"PRIdPTR".\n",(uintptr_t)psc_FindFileName,ul_FindFileNameLen,(uintptr_t)psc_SearchPath,(uintptr_t)psc_SearchFileName );
		return FALSE;
	}

	//フォルダオープン.
	dir=SYSF_opendir(psc_SearchPath);
	//フォルダがない.
	if(dir == NULL){
		THRL_ThLogMsgErr( "THRL_searchFileFromDirectory opendir non %s..\n",psc_SearchPath );
		return FALSE;
	}

	//検索ファイル名を小文字に変換.
	string	c_searchFileName = psc_SearchFileName;
	transform( c_searchFileName.begin(), c_searchFileName.end(), c_searchFileName.begin(), ::tolower);

	//ワイルドカード検索.
	ul_Wildcard = c_searchFileName.find("*");
	//ワイルドカード有の場合.
	if(ul_Wildcard != string::npos){
		//「*」より前のファイル名を取得.
		memcpy(sc_SearchName,c_searchFileName.c_str(),ul_Wildcard);
		sc_SearchName[ul_Wildcard] = '\0';

		//「*」より後の検索ファイル名取得.
		psc_RearSearchName = (char*)c_searchFileName.c_str();
		psc_RearSearchName += (ul_Wildcard+1);	//ワイルドカード分アドレスを進める.
	}else{	//ワイルドカード無の場合.
		psc_RearSearchName = (char*)c_searchFileName.c_str();
	}

	while(1){
		//フォルダ読み出し.
		dp = readdir(dir);

		//フォルダ内にファイルなし.
		if(dp == NULL){
			bl_Ret = FALSE;
			THRL_ThLogMsgErr( "THRL_searchFileFromDirectory readdir File non .\n" );
			break;
		}

		//検索したファイル名の長さチェック.
		if( ul_FindFileNameLen < strlen(dp->d_name) ){
			continue;
		}

		//フォルダから読み出したファイル名を小文字に変換.
		string	c_FolderFileName = dp->d_name;
		transform( c_FolderFileName.begin(), c_FolderFileName.end(), c_FolderFileName.begin(), ::tolower);
		psc_FolderFileName = (char*)c_FolderFileName.c_str();

		//「*」より前のファイル名検索.
		//ワイルドカードがあり、先頭が「*」以外の場合.
		if( (ul_Wildcard != string::npos) &&
			('*' != c_searchFileName.at(0)) ){
			//ファイル名検索.
			psc_Search = strstr(psc_FolderFileName,sc_SearchName);
			//検索文字列がない場合、次ファイル.
			if(psc_Search == NULL){
				continue;
			}
			//先頭アドレスが一致しない場合、次ファイル.
			if(psc_Search != psc_FolderFileName){
				continue;
			}
		}else{
			psc_Search = psc_FolderFileName;
		}

		//「*」より後のファイル名検索 or ワイルドカード無の場合.
		//終端ではない場合.
		if(*psc_RearSearchName != '\0'){
			//ファイル名検索.
			psc_Search = strstr(psc_Search,psc_RearSearchName);
			//検索文字列がない場合、次ファイル.
			if(psc_Search == NULL){
				continue;
			}

			//ファイル名の終端が一致するか.
			if(*(psc_Search+strlen(psc_RearSearchName)) != '\0'){
				continue;
			}
		}

		//検索したファイル名を取得する.
		strncpy( psc_FindFileName, dp->d_name, ul_FindFileNameLen );
		bl_Ret = TRUE;
		break;
	}
	//フォルダクローズ.
	closedir(dir);
	return	bl_Ret;
}

// ================================================================================================.
//! <!--機能--> シグナルハンドラ関連のログのバッファサイズを変更する.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param  uint32_t	ul_BufSize	バッファサイズ.
//! <!--戻値-->.
//! \return.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
//  2015/08/28	yamashita	新規作成(CQ514756調査用ログ出力対応).
// ================================================================================================.
void THRL_SetAsicHandlerLogBufSize(uint32_t ul_BufSize)
{
	uint32_t ul_AsicHandlerMsgLogMax;
	// 引数なし(0指定)の場合はデフォルト値で確保する.
	if (0 == ul_BufSize) {
		ul_AsicHandlerMsgLogMax = THRD_AsicHandlerMsgLogMax;
	} else {
		ul_AsicHandlerMsgLogMax = ul_BufSize;
	}

	THRL_Drvrif_SetAsicHandlerLogBufSize( ul_AsicHandlerMsgLogMax );
}

// ================================================================================================.
//! <!--機能--> 【ターミナルコマンド】エンジンデータクリアコマンド.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
// 2015/11/04	古市	ZeusSZX0 制御構成変更No.79 totalclear改善対応 新規作成.
// ================================================================================================.
void ClearEngineData()
{
	//ラインモードかチェックする.
	if( THRL_Drvrif_IsLineMode() == 1 ){
		// トラブルリセット指示.
		ENGC_PrinterImpColor::newInstance()->req_ResetOperation(
										ENGE_ResetIndication_TroubleReset,
										ENGE_TroubResetOperation_MemorySW,
										NULL );
		// サイズ/メディアエラーリセット指示.
		ENGC_PrinterImpColor::newInstance()->req_ResetOperation(
										ENGE_ResetIndication_SizeMediaErrReset,
										ENGE_TroubResetOperation_InitialSW,
										NULL );
		// イニシャライズ指示.
		ENGC_PrinterImpColor::newInstance()->req_ResetOperation(
										ENGE_ResetIndication_Initalize,
										ENGE_TroubResetOperation_InitialSW,
										NULL );
		// メモリクリア指示.
		ENGC_PrinterImpColor::newInstance()->req_ResetOperation(
										ENGE_ResetIndication_MemoryClear,
										ENGE_TroubResetOperation_InitialSW,
										NULL );
		// トータルクリア指示.
		ENGC_PrinterImpColor::newInstance()->req_ResetOperation(
										ENGE_ResetIndication_TotalClear,
										ENGE_TroubResetOperation_MemorySW,
										NULL );
		printf("EngineDataClear End\n");
	} else {
		printf("Not Line Mode!\n");
	}
}


#if defined( DEF_A4_Cus )   // CQ521426 Act_518744.
/*================================================================================================
［機能］	ターミナルコマンド（2ndネットワークカード装着状態）：ラインモード用.
--------------------------------------------------------------------------------------------------
［引数］	s_Mode : コマンドモード (0,1,99).
            s_State: 装着状態  ( 0 - 2 ).

［戻値］	無し.
［詳細］	
--------------------------------------------------------------------------------------------------
 [履歴]
 '16/01/15 馬場 CQ521426 新規作成.
================================================================================================*/
void Set2ndNICExist(short s_Mode ,short s_State)
{
    const short CONST_ACCEPTTBL_END = -1;                   // テーブル末尾.
    
    // 関数処理モードの列挙体.    
    enum ENUM_FuncMode : short
    {
        ShowHelp = 0,           // ヘルプ表示.
        Normal   = 1,           // 値設定.
        RefOnly  = 99,          // 参照モード.
        ParamError,             // 引数エラー.
        
    };
    ENUM_FuncMode e_FuncMode = ParamError;   // 関数処理モード 引数エラーで初期化.
    
    // s_Stateの許可する値テーブル.
    // 今後拡張される余地を考慮してテーブルに許可する値を入れる.
    // TYPE_ExNetworkAdapterはuchar.
    const short array_AcceptState[] = {
        (short)TYPE_ExNetworkAdapter_Disable,              // 無効.
        (short)TYPE_ExNetworkAdapter_Wireless,             // 無線LAN.
        (short)TYPE_ExNetworkAdapter_Wired,                // 有線LAN.
        CONST_ACCEPTTBL_END,
    };
    
    // ラインモード判定.
    if( checkLineMode() == False )
    {
        // ラインモードではない場合.
        // メッセージ出力は checkLineModeで行われているのでそのままreturnする.
		return;
	}
    
    // s_Modeによる関数処理モード振り分け.
    switch(s_Mode)
    {
    case (short)RefOnly:             // 参照モード.
        e_FuncMode = RefOnly;
        break;
        
    case (short)ShowHelp:            // ヘルプ表示モード.
        e_FuncMode = ShowHelp;
        break;
        
    case (short)Normal:              // 設定実行モード.
    {
        // パラメーターチェック s_State .
        short s_counter = 0;
        // array_AcceptStateから検索する
        while(array_AcceptState[s_counter] != CONST_ACCEPTTBL_END)
        {
            if(array_AcceptState[s_counter] == s_State )
            {
                // 許可する値なのでNormalを設定してbreakする.
                e_FuncMode = Normal;
                break;
            }
            
            // カウントアップ.
            s_counter++;
        }
    }
        break;
        
    default:                                                // その他は引数エラー.
        e_FuncMode = ParamError;
        break;
    }
    
    // 引数エラー、もしくはヘルプ表示の場合。ヘルプを表示してreturnする.
    if( ( e_FuncMode == ParamError ) || ( e_FuncMode == ShowHelp ))
    {
        printf("[Format]Set2ndNICExist Mode,State \r\n");
        printf(" Mode  ... コマンドモード. ( 0,1,99 ) \r\n");
        printf(" State ... 2ndネットワークカード装着状態.  ( 0 - 2 ) \r\n");
        printf(" \r\n");
        printf("[Ex.] \r\n");
        printf(" Set2ndNICExist 99    <= 現在の状態を表示. \r\n");
        printf(" Set2ndNICExist 1,1   <= 無線アダプタ装着を設定する. \r\n");
        printf(" \r\n");
        printf("[Mode] \r\n");
        printf(" 0: ヘルプ表示. \r\n");
        printf(" 1: 設定実行. \r\n");
        printf(" 99: 現在の状態を表示. \r\n");
        printf(" \r\n");
        printf("[State] \r\n");
        printf(" 0:  無効. \r\n");
        printf(" 1:  無線アダプタ装着. \r\n");
        printf(" 2:  有線アダプタ装着. \r\n");
        
        return;
    }
    
 

    // 設定クラスのインスタンスを取得.
    APIC_SystemInfo * pc_SystemInfo = APIC_SystemInfo::newInstance();
    if(pc_SystemInfo == NULL)
    {
        // エラー.
        printf("SystemErr \r\n");
        return;
    }
    
    // 更新前の状態を取得.
    TYPE_ExNetworkAdapter last_State = pc_SystemInfo->getExNetworkAdapter();
    
    printf("Last State = %d \r\n", (short)last_State);
    
    // 参照モードの場合、値を設定するまえにreturnする.
    if(e_FuncMode == RefOnly)
    {
        return;
    }
    
    // 状態を設定する.
    pc_SystemInfo->setExNetworkAdapter((TYPE_ExNetworkAdapter)s_State);
    
    // 最新の状態を取得する.
    last_State = pc_SystemInfo->getExNetworkAdapter();
    
    printf("New  State = %d \r\n", (short)last_State);

    
}

/*================================================================================================
［機能］	ターミナルコマンド（FAXボード装着設定）：ラインモード用.
--------------------------------------------------------------------------------------------------
［引数］	 s_Fax : 回線番号  ( 1 , 99 ).
           i_ONOFF : 有無状態  ( 0 ,1 ).
［戻値］	無し.
［詳細］	
--------------------------------------------------------------------------------------------------
 [履歴]
 '16/01/15 馬場 CQ521426 新規作成.
================================================================================================*/
void SetFaxExist( short s_Fax, short s_OnOff)
{
    const short CONST_ACCEPTTBL_END = -1;                   // テーブル末尾.
    
    // 関数処理モードの列挙体.    
    enum ENUM_FuncMode : short
    {
        //ShowHelp = 0,           // ヘルプ表示  処理上引数エラーと同じ扱い.
        RefOnly  = 99,          // 参照モード.
        Normal,                 // 値設定.
        ParamError,             // 引数エラー.
        
    };
    ENUM_FuncMode e_FuncMode = ParamError;   // 関数処理モード 引数エラーで初期化.
    
    // ラインモード判定.
    if( checkLineMode() == False )
    {
        // ラインモードではない場合.
        // メッセージ出力は checkLineModeで行われているのでそのままreturnする.
		return;
	}

    // s_Faxの許可する値テーブル.
    // 今後拡張される余地を考慮してテーブルに許可する値を入れる.
    // TYPE_CommLineは char .
    const short array_AcceptFax[] = {
        (short)TYPE_CommLine1,              // 回線１(主回線).        
//        TYPE_CommLine2,            // 回線２(副回線) VenusMLKでは未使用. 
//        TYPE_CommLine3,            // 回線３(副回線) VenusMLKでは未使用.
//        TYPE_CommLine4,            // 回線４(副回線) VenusMLKでは未使用.
        (short)RefOnly,      // 参照モード.
        CONST_ACCEPTTBL_END,
    };
    
    // パラメーターチェック s_Fax .
    short s_counter = 0;
    // array_AcceptFaxから検索する.
    while(array_AcceptFax[s_counter] != CONST_ACCEPTTBL_END)
    {
        if(array_AcceptFax[s_counter] == s_Fax )
        {
            // モードを設定してbreak.
            if(s_Fax != RefOnly)
            {
                // 設定実行モード.
                e_FuncMode = Normal;    
            }else{
                // 参照モード.
                e_FuncMode = RefOnly;
            }
            
            break;
        }
        
        // カウントアップ.
        s_counter++;
    }
    
    // パラメータチェック　s_OnOff.
    // 設定実行モード　かつs_OnOffがTrue/False以外の場合、引数エラー.
    if( ( e_FuncMode == Normal ) && 
        ( ( s_OnOff != True ) && ( s_OnOff != False ) ) ) 
    {
        // 引数エラー.
        e_FuncMode = ParamError;
    }
    
    // 引数エラーの場合、ヘルプを表示してreturn.
    if(e_FuncMode == ParamError)
    {
        printf("[Format]SetFaxExist FAX,ONOFF \r\n");
        printf(" FAX   ... 回線番号  ( 1 , 99 ) \r\n");
        printf(" ONOFF ... 有無状態  (0,1) \r\n");
        printf("[Ex.] \r\n");
        printf(" SetFaxExist 99    <= 現在の状態を表示. \r\n");
        printf(" SetFaxExist 1,1   <= FAX1を装着状態にする. \r\n");
        printf("[FAX] \r\n");
        printf(" 1: FAX1 \r\n");
//        printf(" 2: FAX2 \r\n");  VenusMLKでは未使用.
//        printf(" 3: FAX3 \r\n");  VenusMLKでは未使用.
//        printf(" 4: FAX4 \r\n");  VenusMLKでは未使用.
        printf(" 99: 現在の状態を表示. \r\n");
        printf("[ONOFF] \r\n");
        printf(" 0:非装着(OFF). \r\n");
        printf(" 1:装着  (ON). \r\n");
        
        return;
    }
    
    // 設定クラスのインスタンスを取得する.
    APIC_SystemInfo * pc_SystemInfo = APIC_SystemInfo::newInstance();
    if(pc_SystemInfo == NULL)
    {
        // エラー.
        printf("SystemErr  \r\n");
        return;
    }
    
    // モードによる振る舞い分岐.
    switch(e_FuncMode)
    {
    case RefOnly:        // 参照モードの振る舞い.
    {
        // array_AcceptFaxに登録されているFAX状態を表示する.
        s_counter = 0;
        while(array_AcceptFax[s_counter] != RefOnly)
        {
            Bool last_onoff = pc_SystemInfo->getFaxExistAll( (TYPE_CommLine)array_AcceptFax[s_counter] );
            printf("Last FAX%d = %d \r\n" ,(short)array_AcceptFax[s_counter] , (short)last_onoff );
            
            // カウントアップ.
            s_counter++;
        }
    }
        break;
        
    case Normal:         // 設定実行モードの振る舞い.
    {
        // 設定前の値を表示.
        Bool last_onoff = pc_SystemInfo->getFaxExistAll( (TYPE_CommLine)s_Fax );
        printf("Last FAX%d = %d \r\n" ,(short)s_Fax , (short)last_onoff );
        
        // 設定実行.
        pc_SystemInfo->setFaxExistAll( (TYPE_CommLine)s_Fax, (Bool)s_OnOff);
        
        // 設定後の値を表示.
        last_onoff = pc_SystemInfo->getFaxExistAll( (TYPE_CommLine)s_Fax );
        printf("New  FAX%d = %d \r\n" ,(short)s_Fax , (short)last_onoff );
    }
        break;
        
    default:
        // ここに来ることはない.
        printf("SystemErr \r\n");
    }
}
/*================================================================================================
［機能］	ターミナルコマンド（FAX仕向け設定）：ラインモード用.
--------------------------------------------------------------------------------------------------
［引数］	TYPE_CountryCodeに準ずる
			※TYPE_CountryCodeはsystem/nvd/nvdg/APIC_Setting.hに定義されている.

［戻値］	無し.
［詳細］	
--------------------------------------------------------------------------------------------------
 [履歴]
 '16/06/28 伊藤 CQ531741 新規作成.
================================================================================================*/
void SetFaxCountryCode( char e_CountryCode )
{

    // ラインモード判定.
    if( checkLineMode() == False )
    {
        // ラインモードではない場合.
        // メッセージ出力は checkLineModeで行われているのでそのままreturnする.
		return;
	}

	APIC_MachineSetting *pSetting = APIC_MachineSetting::newInstance();
	
	if(pSetting !=0 && e_CountryCode !=-1)	{
		pSetting->setCountryCode(e_CountryCode);
    }
    else{
		printf("[USAGE]:\n");
		printf("  引数n");
		printf("   -1:使用法説明と現在値取得\n");
		printf("   上記以外:FAX仕向け（TYPE_CountryCodeに定義された値\n");
		printf("             ※ /nvd/nvdg/APIC_Setting.hに定義されている\n");
		printf("             ※ 引数チェックは行っていない\n");
		printf("[現在値]:\n");
		if(pSetting !=0){
			printf("   countryCode=%d\n",pSetting->getCountryCode());
		}
		else{
			// 論理上あり得るが設計上はありえない.
			printf("   APIC_MachineSetting = NULL\n");
		}
	}
}
#endif // defined( DEF_A4_Cus )

/*================================================================================================
［機能］	Faxジャーナル・Scanジャーナルの削除処理：ラインモード用の内部関数.
--------------------------------------------------------------------------------------------------
［引数］	無し.
［戻値］	無し.
［詳細］	totalCounterClearから呼び出されるサブ関数.
			・ＦＡＸジャーナル・ＳＣＡＮジャーナルの削除.
			・送信・受信トランザクションＩＤのクリア.
--------------------------------------------------------------------------------------------------
 [履歴]
 '16/06/28 伊藤 CQ531741 新規作成。
================================================================================================*/

#define DEBUG_DELETE_JOURNAL_PRINT(x) { if( debug_delete_journal ) x; }					// ジャーナル削除計測用マクロ定義.
int debug_delete_journal = 0;															// ジャーナル削除計測用判断変数定義(1=表示).
static void deleteFaxScanJournal_Internal(void)
{
	APIC_DoneJobList* pDoneJobList = APIC_DoneJobList::newInstance();
	NVDC_JobDJKeyInfo* pc_JobDJKeyInfo = NVDC_JobDJKeyInfo::newInstance();

	if( pDoneJobList !=0 && pc_JobDJKeyInfo != 0)
	{
		// ---------------------------------------------------------------
		// Faxジャーナル・Scanジャーナルの削除.
		// ---------------------------------------------------------------
		DEBUG_DELETE_JOURNAL_PRINT( printf("[%010d]APIC_DoneJobList::deleteFaxJournal() start\n",  SYSF_tickGet()) );

		pDoneJobList->deleteFaxJournal();

		DEBUG_DELETE_JOURNAL_PRINT( printf("[%010d]APIC_DoneJobList::deleteFaxJournal() end\n",    SYSF_tickGet()) );
		DEBUG_DELETE_JOURNAL_PRINT( printf("[%010d]APIC_DoneJobList::deleteScanJournal() start\n", SYSF_tickGet()) );

		pDoneJobList->deleteScanJournal();

		DEBUG_DELETE_JOURNAL_PRINT( printf("[%010d]APIC_DoneJobList::deleteScanJournal() end\n",   SYSF_tickGet()) );

		// ---------------------------------------------------------------
		// 送信・受信トランザクションＩＤのクリア.
		// ---------------------------------------------------------------
	    uint32_t def_tranId = 0;//初期化値0.

		DEBUG_DELETE_JOURNAL_PRINT( printf("[%010d]NVDC_JobDJKeyInfo::settranidtx() start\n",      SYSF_tickGet()) );

		pc_JobDJKeyInfo->settranidtx( def_tranId );

		DEBUG_DELETE_JOURNAL_PRINT( printf("[%010d]NVDC_JobDJKeyInfo::settranidtx() end\n",        SYSF_tickGet()) );
		DEBUG_DELETE_JOURNAL_PRINT( printf("[%010d]NVDC_JobDJKeyInfo::settranidrx() start\n",      SYSF_tickGet()) );

		pc_JobDJKeyInfo->settranidrx( def_tranId );

		DEBUG_DELETE_JOURNAL_PRINT( printf("[%010d]NVDC_JobDJKeyInfo::settranidrx() end\n",       SYSF_tickGet()) );
	}
	else
	{
		// 設計的にはありえない.
		printf("deleteFaxScanJournal_Internal instance NULL. pDoneJobList(0x%08x)  pc_JobDJKeyInfo(0x%08x)\n",pDoneJobList, pc_JobDJKeyInfo);
	}
}

#if defined(DEF_A4_Cus)
// ================================================================================================.
//! <!--機能--> デバイスの物理的なUSB接続状態確認関数.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param const char 接続確認対象のデバイスのベンダID:プロダクトID文字列テーブル.
//! <!--戻値-->.
//! \return int 接続されているデバイスの数.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
// 2016/01/29	itakura	[CQ521426] VenusMLK WBS-1457 生産コマンド追加対応.
// ================================================================================================.
static int getUsbDevConnectNum(const char *psc_devtbl)
{
	int si_Ret			= 0;
	FILE* fp			= NULL;
	char* psc_Ret		= NULL;
	char sc_Cmdline[20]	= {0};		// lsusbコマンド文字列格納用.
	char sc_Buf[1024] 	= {0};		// lsusbコマンド実行結果格納用.
	
	// NULLが渡されることはないはずだが、静的解析での検出抑止のため、.
	// パラメータチェックを行なう.
	if (psc_devtbl == NULL) {
		printf("getUsbDevConnectNum: invalid parameter. \n");
		return THRD_UsbConnectCheck_Error;
	}

	snprintf(sc_Cmdline, sizeof(sc_Cmdline), "lsusb -d %s", psc_devtbl);
	fp = popen(sc_Cmdline, "r");
	if (fp == NULL) {
		printf("getUsbDevConnectNum: popen error. \n");
		return THRD_UsbConnectCheck_Error;
	}

	while (1) {
		psc_Ret = fgets(sc_Buf, sizeof(sc_Buf), fp);
		if (psc_Ret == NULL) {
			if (!feof(fp)) {
				// エラー.
				printf("getUsbDevConnectNum: fgets error. \n");
				pclose(fp);
				return THRD_UsbConnectCheck_Error;
			} else {
				// 全行読み込んだ.
				break;
			}
		} else {
			// 探索対象のベンダID：プロダクトID文字列を探索.
			psc_Ret = strstr(sc_Buf, psc_devtbl);
			if (psc_Ret != NULL) {
				si_Ret++;
			}
		}
	}
	pclose(fp);

	return si_Ret;
}


// ================================================================================================.
//! <!--機能--> 【ターミナルコマンド】FAXデバイス接続確認コマンド.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! FAX基板の物理的な接続/非接続状態をターミナルに出力する.
//! <!--履歴-->.
// 2016/01/29	itakura	[CQ521426] VenusMLK WBS-1457 生産コマンド追加対応.
// ================================================================================================.
void getFaxDeviceConnectStatus()
{
	int i;
	int si_Ret = THRD_UsbConnectCheck_Error;
	int si_Num = sizeof(sc_FaxDevIdtbl) / sizeof(sc_FaxDevIdtbl[0]);	// テーブルサイズ.
	int si_Cnt = 0;														// 接続を検出したデバイスのベンダ数用カウンタ.
	
	// テーブル定義されているデバイスIDの接続を確認する.
	for (i= 0; i < si_Num; i++) {
		si_Ret = getUsbDevConnectNum(sc_FaxDevIdtbl[i]);
		if (si_Ret > 0) {
			// デバイスが接続されていることを確認できたベンダ数をカウントする.
			si_Cnt++;
		} else if (si_Ret == THRD_UsbConnectCheck_Error) {
			printf("getFaxDeviceConnectStatus command error. \n");
			return;
		}
	}

	// 接続状態をターミナルに表示.
	if (si_Cnt > 0 ) {
		// デバイスの接続が一つでも確認できた場合.
		printf("Fax usb device is connected. \n");
	} else {
		// デバイスの接続が一つも確認できなかった場合.
		printf("Fax usb device is not connected. \n");
	}
}

// ================================================================================================.
//! <!--機能--> 【ターミナルコマンド】WLANデバイス接続確認コマンド.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! WLANの物理的な接続/非接続状態をターミナルに出力する.
//! <!--履歴-->.
// 2016/01/29	itakura	[CQ521426] VenusMLK WBS-1457 生産コマンド追加対応.
// ================================================================================================.
void getWlanDeviceConnectStatus()
{
	int i;
	int si_Ret = THRD_UsbConnectCheck_Error;
	int si_Num = sizeof(sc_WlanDevIdtbl) / sizeof(sc_WlanDevIdtbl[0]);	// テーブルサイズ.
	int si_Cnt = 0;														// 接続を検出したデバイスのベンダ数用カウンタ.
	
	// テーブル定義されているデバイスIDの接続を確認する.
	for (i= 0; i < si_Num; i++) {
		si_Ret = getUsbDevConnectNum(sc_WlanDevIdtbl[i]);
		if (si_Ret > 0) {
			// デバイスが接続されていることを確認できたベンダ数をカウントする.
			si_Cnt++;
		} else if (si_Ret == THRD_UsbConnectCheck_Error) {
			printf("getWlanDeviceConnectStatus command error. \n");
			return;
		}
	}

	// 接続状態をターミナルに表示.
	if (si_Cnt > 0 ) {
		// デバイスの接続が一つでも確認できた場合.
		printf("WLAN usb device is connected. \n");
	} else {
		// デバイスの接続が一つも確認できなかった場合.
		printf("WLAN usb device is not connected. \n");
	}
}

#endif	//defined(DEF_A4_Cus).

// ================================================================================================.
//! <!--機能--> コマンドの標準出力と実行結果を取得する関数.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param char *psc_Cmd	実行コマンド文字列.
//! \param char *psc_Out	標準出力格納領域.
//! \param ulong ul_OutSize	標準出力格納領域のサイズ.
//! \param int *pi_Ret		コマンド実行結果(0:成功、0以外:失敗).
//! <!--戻値-->.
//! \return Bool			処理の結果（True:成功、False:失敗）.
//! <!--詳細-->.
//! コマンドの標準出力と実行結果を取得する.
//! <!--履歴-->.
// 2016/08/25 SKY井柳	【IT5_4.0】制御構成変更No45「基板交換低減対応」：新規作成.
// ================================================================================================.
Bool THRL_getCommandData( char *psc_Cmd, char *psc_Out, uint32_t ul_OutSize ,int *pi_Ret )
{
	// パラメタチェック.
	if( psc_Cmd == NULL ){
		printf("[%s] Commond is NULL. \n",__FUNCTION__);
	    return False;// パラメタエラー.
	}
	if( psc_Out == NULL ){
		printf("[%s] Output Area is NULL. \n",__FUNCTION__);
	    return False;// パラメタエラー.
	}
	if( pi_Ret == NULL ){
		printf("[%s] Return Value is NULL. \n",__FUNCTION__);
	    return False;// パラメタエラー.
	}
	
	FILE *pf_File = NULL;
	int si_CmdRet = 0;			// コマンド実行結果.
	char sc_LineBuf[ 256 ];		// 1行文字列取得用バッファ.
	uint32_t  ul_LineSize = 0;		// 1行文字列サイズ.
	uint32_t  ul_ToralSize = 0;	// 取得総合サイズ.
	uint32_t  ul_CheckSize = 0;	// サイズチェック用.
	
	memset( sc_LineBuf, 0x0, sizeof( sc_LineBuf ) );

	if( ( pf_File=popen( psc_Cmd,"r") ) ){				// コマンドオープン.
		
		// 一行づつ標準出力を取得.
		while( fgets(sc_LineBuf,sizeof(sc_LineBuf),pf_File) ){
			if( ul_ToralSize >= ul_OutSize ){	// 取得サイズがフルの時は、処理を飛ばす.
				continue;
			}
			ul_LineSize = strlen( sc_LineBuf );			// 取得した1行文字列の長さを取得.
			ul_CheckSize = ul_ToralSize + ul_LineSize;	// チェック用に取り込んだときの長さを作成.

			if( ul_CheckSize < ul_OutSize ){	// チェックサイズが、出力領域をオーバーしてなければ、追加する.
				strcat( psc_Out, sc_LineBuf );
			}else{
				// オーバーしている時は、残りの長さ分だけ追加.
				ul_LineSize = ul_OutSize - ul_ToralSize;
				strncat( psc_Out, sc_LineBuf, ul_LineSize );
			}
			ul_ToralSize += ul_LineSize;
			memset( sc_LineBuf, 0x0, sizeof( sc_LineBuf ) );
		}
	    si_CmdRet = pclose( pf_File );
		*pi_Ret = 0;
		if( WIFEXITED(si_CmdRet) ){	// 子プロセスが正常終了したか確認.
			*pi_Ret = WEXITSTATUS(si_CmdRet);// 終了ステータス設定.
	    }else{
			*pi_Ret = -1;// 子プロセスの異常終了.
		}
	}else{
		printf("[%s] popen error(%d). \n",__FUNCTION__,errno);
	    return False;// コマンドオープン失敗.
	}
	
	return True;
}

// ================================================================================================.
//! <!--機能--> テキストの先頭から1行分の文字列を取得する関数.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param char *psc_Txt				テキスト文字列.
//! \param ulong ul_TxtSiz				テキスト文字列サイズ.
//! \param char *psc_OneLinStr			1行文字列.
//! \param ulong ul_OneLinStrMax		1行文字列最大サイズ.
//! \param ulong *pul_GetStrSiz		1行文字列サイズ(取得値）.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! テキストの1行目を取得する.
//! <!--履歴-->.
// 2016/09/05 SKY井柳	【IT5_4.0】制御構成変更No45「基板交換低減対応」：新規作成.
// ================================================================================================.
void THRL_getOneLineString( char *psc_Txt, uint32_t ul_TxtSiz, char *psc_OneLinStr, uint32_t ul_OneLinStrMax, uint32_t *pul_GetStrSiz )
{
	// パラメタチェック.
	if( psc_Txt == NULL ){	// パラメタ不正.
		printf("[%s] Text is NULL. \n",__FUNCTION__);
	    return ;
	}
	if( psc_OneLinStr == NULL ){// パラメタ不正.
		printf("[%s] One Line Area is NULL. \n",__FUNCTION__);
	    return ;
	}
	if( pul_GetStrSiz == NULL ){// パラメタ不正.
		printf("[%s] One Line Get Size is NULL. \n",__FUNCTION__);
	    return ;
	}
	
	uint32_t ul_GetStrSiz =0;
	while( ( ul_GetStrSiz < ul_TxtSiz )			// 元データオーバー.
		&&( psc_Txt[ul_GetStrSiz] != '\0')		// 出力終端.
		&&( ul_GetStrSiz < ul_OneLinStrMax )	// 取得データオーバー.
		 ){
		psc_OneLinStr[ ul_GetStrSiz ] = psc_Txt[ul_GetStrSiz];	// コピー.
		ul_GetStrSiz++;
		if( psc_Txt[ul_GetStrSiz-1] == 0x0A ){	// 追加文字が行終端.
			break;// 行終端は、ここで終了.
		}
	}
	*pul_GetStrSiz = ul_GetStrSiz;	// 取得1行データサイズ.
}

//! ================================================================================================.
//! <!--機能--> 文字列右端(後端)の空白文字を削除.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param char* psc_String	対象文字列.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//!    文字列右端(後端)の空白文字を削除する. \n.
//! <!--履歴-->.
//! 2016/09/07	SKY井柳	【IT5_4.0】制御構成変更No45「基板交換低減対応」：新規作成.
// ================================================================================================.
void THRL_TrimStringR( char* psc_String )
{
	// パラメタチェック.
	if ( NULL == psc_String ){	// パラメタ不正.
		return;
	}
	
	// 文字列長チェック.
	uint32_t ul_Len = strlen( psc_String );
	if( ul_Len == 0 ){	// 文字がなければここで終了.
		return;
	}
	
	uint32_t ul_Pos = 0;
	uint32_t ul_Cnt = 1;
	// 後端の処理.
	for( ;ul_Cnt <= ul_Len; ul_Cnt++ ){
		ul_Pos = ul_Len - ul_Cnt;
		if( isspace( psc_String[ ul_Pos ] ) == 0 ){	// 空白文字でない時.
			break;
		}else{
			// 空白文字の時は、NULL文字を入れる.
			psc_String[ ul_Pos ] = '\0';
		}
	}
}

//! ================================================================================================.
//! <!--機能--> 文字列左端(先端)の空白文字を削除.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param char* psc_String	対象文字列.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//!    文字列左端(先端)の空白文字を削除する. \n.
//! <!--履歴-->.
//! 2016/09/07	SKY井柳	【IT5_4.0】制御構成変更No45「基板交換低減対応」：新規作成.
// ================================================================================================.
void THRL_TrimStringL( char* psc_String )
{
	// パラメタチェック.
	if ( NULL == psc_String ){	// パラメタ不正.
		return;
	}
	
	// 文字列長チェック.
	uint32_t ul_Len = strlen( psc_String );
	if( ul_Len == 0 ){	// 文字がなければここで終了.
		return;
	}
	
	// 先端の空白文字以外になるまで検索.
	uint32_t ul_Pos = 0;
	for( ; ul_Pos < ul_Len; ul_Pos++ ){
		if( isspace( psc_String[ ul_Pos ] ) == 0 ){	// 空白文字でない時.
			break;
		}
	}
	if( ul_Pos == 0 ){// 先端の場合は、削除の必要なし.
		return;
	}
	// 空白ではない部分から、コピー(空白部分の削除).
	strcpy( psc_String, &psc_String[ ul_Pos ] );
}

//! ================================================================================================.
//! <!--機能--> 文字列前後端の空白文字を削除.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param char* psc_String	対象文字列.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//!    文字列のトリミング(空白文字削除)を行う. \n.
//! <!--履歴-->.
//! 2016/09/07	SKY井柳	【IT5_4.0】制御構成変更No45「基板交換低減対応」：新規作成.
// ================================================================================================.
void THRL_TrimString( char* psc_String )
{
	THRL_TrimStringR( psc_String );	// 文字列右端(後端)の空白文字を削除.
	THRL_TrimStringL( psc_String );	// 文字列左端(先端)の空白文字を削除.
}

// ================================================================================================.
//! <!--機能--> 差分時間取得.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param ulong ul_Start.
//! \param ulong ul_End.
//! <!--戻値-->.
//! \return ulong 差分時間.
//! <!--詳細-->.
//! Tick時間用の開始から終了までの差分時間を取得する.
//! ただし、値が逆転（開始のほうが大きい）している場合は、上限を超えたと判断して.
//! 計算する.
//! <!--履歴-->.
// 2016/09/30	SKY井柳	[CQ521426] 【IT5_4.0】 制御構成変更No39 「RA接続改善」新規作成.
// ================================================================================================.
uint32_t THRG_diffTime( uint32_t ul_Start, uint32_t ul_End )
{
	uint32_t ul_DiffTime = 0;
	if( ul_Start > ul_End ){	// 開始のほうが値が大きいときは、MAXを超えているとして計算する.
		ul_DiffTime = ( ULONG_MAX - ul_Start ) + ul_End;
	}else if( ul_Start < ul_End ){
		ul_DiffTime = ul_End - ul_Start;
	}
	// 等しいときは、0(初期値).
	
	return ul_DiffTime;
}

/*===============================================================================================.
［機能］	「不揮発データの初期化」"InitializeNVData"コマンド終了確認用コールバック.
-------------------------------------------------------------------------------------------------.
［引数］	「不揮発データの初期化」処理結果（True:成功、False失敗）.
［戻値］	なし.
［詳細］	CommonAPIの「不揮発データ初期化 実行API」処理の終了時にコールされるコールバック.
			引数に結果値(Bool)が設定されるため、その結果値に応じた内容を出力する.
［履歴］	.
			2016/11/07	Sky井柳 【IT5_4.0】制御構成変更No56「totalclearコマンドの改善」新規作成.
================================================================================================*/
void ans_InitializeNVData( Bool bl_Ret )
{
	if( bl_Ret == True) {
		printf("InitializeNVData OK\n");	// 「不揮発データの初期化」処理：成功時.
	} else {
		printf("InitializeNVData NG\n");	// 「不揮発データの初期化」処理：失敗時.
	}

// 2022/03/30 生産RINNE対応 Sky start.
	SYSD_WEBAPI_LINEMODE_WAKEUP
// 2022/03/30 生産RINNE対応 Sky end.

	return;
}

/*===============================================================================================.
［機能］	「不揮発データの初期化」"InitializeNVData"コマンド.
-------------------------------------------------------------------------------------------------.
［引数］	なし.
［戻値］	なし.
［詳細］	CommonAPIの「不揮発データ初期化 実行API」処理を実行するコマンド.
			上記の処理の引数に、「不揮発データの初期化」"InitializeNVData"コマンド終了確認用コールバックを.
			設定する.
［履歴］	.
			2016/11/07	Sky井柳 【IT5_4.0】制御構成変更No56「totalclearコマンドの改善」新規作成.
================================================================================================*/
void InitializeNVData()
{
	if( checkLineMode() == False ){		// ラインモードであるかの確認.
		// ラインモードではない場合.
		// メッセージ出力は checkLineModeで行われているのでそのままreturnする.
		return;
	}

	APIC_SystemInfo* pc_SystemInfo = APIC_SystemInfo::newInstance();
	if( pc_SystemInfo == NULL ){	// システム情報のインスタンス確認.
		printf("SystemErr\n");
		return;
	}

	// 「不揮発データの初期化」処理実行.
	pc_SystemInfo->req_InitializeNVData( ans_InitializeNVData );

// 2022/03/30 生産RINNE対応 Sky start.
	SYSD_WEBAPI_LINEMODE_WAIT
// 2022/03/30 生産RINNE対応 Sky end.
	return;
}

// 2018/04/20 Sky K.Yamagishi [DenebMLK]制御構成変更No.66 横流れ対応.
// ================================================================================================.
//! <!--機能--> 基板シリアル番号取得.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
// 2018/04/20	山岸	[DenebMLK]制御構成変更No.66 横流れ対応 新規作成.
// ================================================================================================.
Bool THRG_getMFPBoardSerialNoFactory( char *pc_MFPBoardSerialNoFactory )
{
	Bool bl_Result = False;
	if( pc_MFPBoardSerialNoFactory == NULL ){
		THRL_SetThLog( 683, THRE_DebugLogType_MTCtrLog, 0, True, 0 );
		return False;
	}
	memset( pc_MFPBoardSerialNoFactory, '\0', TYPD_MAX_MFPBoardSerialNoFactory+1 );
	
	bl_Result = THRL_Drvrif_SysBspBoardSerialNoPrint_Data( pc_MFPBoardSerialNoFactory );
	
	if( bl_Result == False){
		THRL_SetThLog( 684, THRE_DebugLogType_MTCtrLog, 0, True, 0 );
		return False;
	}
	return True;
}
/*===============================================================================================.
［機能］	IR原稿サイズ検出センサ状態取得のコールバック.
-------------------------------------------------------------------------------------------------.
［引数］	Bool   bl_Result：結果　True：正常終了/False：異常終了.
			ushort us_Data：IR原稿サイズ検出センサ状態.
［戻値］	なし.
［詳細］	Scan PIRMからセンサ状態を受信し実行結果を通知するコールバック.
			0：センサOFF/1：センサON.
［履歴］	.
			2018/06/05	Sky 小久保【Eagle】制御構成変更_No.89 生産コマンド対応 新規作成.
================================================================================================*/
static void THRL_ans_OriginalsizeSensor( Bool bl_Result, ushort us_Data )
{
	uchar uc_Count = 0;
	printf( "\n" );
	if( bl_Result == False) {
		printf("GetOriginalsizeSensor NG\n");		// 処理：失敗時.
		return;
	}
	for( uc_Count = 0; uc_Count < 8; uc_Count++){
		// センサ1～８の状態を出力する.
		if( ( us_Data & ( 1 << uc_Count ) ) == 0 ){
			printf("%d = 0\n", (uc_Count+1) );		// ON出力.
		}else{
			printf("%d = 1\n", (uc_Count+1) );		// OFF出力.
		}
	}
	return;
}
// ================================================================================================.
//! <!--機能--> 【ターミナルコマンド】IR原稿サイズ検出センサ表示.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! IR原稿サイズ検出センサ値を表示する.
//! <!--履歴-->.
//	2018/06/05	Sky 小久保【Eagle】制御構成変更_No.89 生産コマンド対応 新規作成.
// ================================================================================================.
void GetOriginalsizeSensor()
{
	printf( "\n" );
	if( checkLineMode( False ) == False ){		// ラインモード判定.
		printf("GetOriginalsizeSensor NG\n");	// ラインモードではない場合NG.
		return;
	}
	SCNC_ScannerIf* pc_ScannerIf = SCNC_ScannerIf::newInstance();
	if ( NULL == pc_ScannerIf ){	// インスタンス不正.
		printf("SystemErr\n");
		return;
	}
	// IR原稿サイズ検出センサ値取得要求.
	pc_ScannerIf->req_OriginalsizeSensor( THRL_ans_OriginalsizeSensor);
}
/*===============================================================================================.
［機能］	原稿押さえ検出センサ状態取得のコールバック.
-------------------------------------------------------------------------------------------------.
［引数］	Bool   bl_Result：結果　True：正常終了/False：異常終了.
			ushort us_Data：原稿押さえ検出センサ状態.
［戻値］	なし.
［詳細］	Scan 原稿押さえ検出センサ状態を受信し実行結果を通知するコールバック.
			0：センサOFF（OPEN）/1：センサON（CLOSE）.
［履歴］	.
			2018/06/05	Sky 小久保【Eagle】制御構成変更_No.89 生産コマンド対応 新規作成.
================================================================================================*/
static void THRL_ans_OriginalCoverSensor( Bool bl_Result, ushort us_Data )
{
	printf( "\n" );
	if( bl_Result == True) {
		printf("OriginalCoverSensor = %d\n", us_Data );	// 処理：正常時.
	} else {
		printf("OriginalCoverSensor NG\n");		// 処理：異常時.
	}
}
// ================================================================================================.
//! <!--機能--> 【ターミナルコマンド】原稿押さえ検出センサ表示.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! 原稿押さえ検出センサ値を表示する.
//! <!--履歴-->.
//	2018/06/05	Sky 小久保【Eagle】制御構成変更_No.89 生産コマンド対応 新規作成.
// ================================================================================================.
void OriginalCoverSensor()
{
	printf( "\n" );
	if( checkLineMode( False ) == False ){	// ラインモード判定.
		printf("OriginalCoverSensor NG\n");	// ラインモードではない場合NG.
		return;
	}
	SCNC_ScannerIf* pc_ScannerIf = SCNC_ScannerIf::newInstance();
	if ( NULL == pc_ScannerIf ){	// インスタンス不正.
		printf("SystemErr\n");
		return;
	}
	// 原稿押さえ検出センサ値取得要求.
	pc_ScannerIf->req_OriginalCoverSensor( THRL_ans_OriginalCoverSensor);
}
/*===============================================================================================.
［機能］	IR20度検出センサ状態取得のコールバック.
-------------------------------------------------------------------------------------------------.
［引数］	Bool   bl_Result：結果　True：正常終了/False：異常終了.
			ushort us_Data  ：IR20度検出センサ状態.
［戻値］	なし.
［詳細］	Scan IR20度検出センサ状態を受信し実行結果を通知するコールバック.
			0：センサOFF（OPEN）/1：センサON（CLOSE）.
［履歴］	.
			2018/06/05	Sky 小久保【Eagle】制御構成変更_No.89 生産コマンド対応 新規作成.
================================================================================================*/
static void THRL_ans_Degree20Sensor( Bool bl_Result, ushort us_Data )
{
	printf( "\n" );
	if( bl_Result == True) {
		printf("Degree20Sensor = %d\n", us_Data );
	} else {
		printf("Degree20Sensor NG\n");		// 処理：失敗時.
	}
}
// ================================================================================================.
//! <!--機能--> 【ターミナルコマンド】IR20度検出センサ表示.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! IR20度検出センサ値を表示する.
//! <!--履歴-->.
//	2018/06/05	Sky 小久保【Eagle】制御構成変更_No.89 生産コマンド対応 新規作成.
// ================================================================================================.
void Degree20Sensor()
{
	printf( "\n" );
	if( checkLineMode( False ) == False ){		// ラインモード判定.
		printf("Degree20Sensor NG\n");			// ラインモードではない場合NG.
		return;
	}
	SCNC_ScannerIf* pc_ScannerIf = SCNC_ScannerIf::newInstance();
	if ( NULL == pc_ScannerIf ){	// インスタンス不正.
		printf("SystemErr\n");
		return;
	}
	// IR20度検出センサ値取得要求.
	pc_ScannerIf->req_Degree20Sensor( THRL_ans_Degree20Sensor);
}
// ================================================================================================.
//! <!--機能--> IR15度検出センサ状態取得のコールバック.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param Bool   bl_Result：結果　True：正常終了/False：異常終了.
//! \param ushort us_Data  ：IR15度検出センサ状態.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! Scan IR15度検出センサ状態を受信し実行結果を通知するコールバック.
//! 0：センサOFF（OPEN）/1：センサON（CLOSE）.
//! <!--履歴-->.
//! 2018/08/21	Sky薗﨑	【DenebMLK】AR1058061_【開発提起】DenebMLKで「IR15度検出センサ表示」コマンドを実行しても結果が表示されないの不具合対応.
// ================================================================================================.
static void THRL_ans_Degree15Sensor( Bool bl_Result, ushort us_Data )
{
	printf( "\n" );
	if( bl_Result == True) {
		printf("Degree15Sensor = %d\n", us_Data );
	} else {
		printf("Degree15Sensor NG\n");		// 処理：失敗時.
	}
}
// ================================================================================================.
//! <!--機能--> 【ターミナルコマンド】IR15度検出センサ表示(IR20度検出センサと同じAPIをコールする).
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! IR15度検出センサ値を表示する(IR20度検出センサと同じAPIをコールする).
//! <!--履歴-->.
//! 2018/08/21	Sky薗﨑	【DenebMLK】AR1058061_【開発提起】DenebMLKで「IR15度検出センサ表示」コマンドを実行しても結果が表示されないの不具合対応.
// ================================================================================================.
void Degree15Sensor()
{
	printf( "\n" );
	if( checkLineMode( False ) == False ){		// ラインモード判定.
		printf("Degree15Sensor NG\n");			// ラインモードではない場合NG.
		return;
	}
	SCNC_ScannerIf* pc_ScannerIf = SCNC_ScannerIf::newInstance();
	if ( NULL == pc_ScannerIf ){	// インスタンス不正.
		printf("SystemErr\n");
		return;
	}
	// IR15度検出センサ値取得要求(IR20度検出センサと同じAPIをコールする).
	pc_ScannerIf->req_Degree20Sensor( THRL_ans_Degree15Sensor );
}

// ================================================================================================.
//! <!--機能-->【ターミナルコマンド】 工場出荷時の基板シリアル番号表示.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
// 2018/04/20	山岸	[DenebMLK]制御構成変更No.66 横流れ対応 新規作成.
// ================================================================================================.
void GetInitialBoardSerialNo()
{
	char sc_BoardSerialNum[TYPD_MAX_MFPBoardSerialNoFactory+1] = { '\0' };
	APIC_FactoryHistory* pc_FactoryHistory = APIC_FactoryHistory::newInstance();
	
	if( pc_FactoryHistory == NULL ){
		printf("SystemErr\n");
		return;
	}
	
	pc_FactoryHistory->getMFPBoardSerialNoFactory( &sc_BoardSerialNum[0] );
	
	// 工場出荷時基板シリアル番号の取得失敗はバッファに'*'が詰められる.
	if( sc_BoardSerialNum[0] == '*' ){
		printf("MFP Controller Serial No: No serial number\n");
		return;
	}
	sc_BoardSerialNum[TYPD_MAX_MFPBoardSerialNoFactory] = '\0'; // ターミナル表示前に終端文字を17byte目に付与する.
	printf("MFP Controller Serial No: %s\n", sc_BoardSerialNum );
	return;
}

// ================================================================================================.
//! <!--機能-->【ターミナルコマンド】 工場出荷時のMacアドレス表示.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
// 2018/04/20	山岸	[DenebMLK]制御構成変更No.66 横流れ対応 新規作成.
// ================================================================================================.
void GetInitialMacAddress()
{
	char sc_MacAddressFactoryNum[TYPD_MAX_MacAddressFactory] = { '\0' };
	APIC_FactoryHistory* pc_FactoryHistory = APIC_FactoryHistory::newInstance();
	
	if( pc_FactoryHistory == NULL ){
		printf("SystemErr\n");
		return;
	}
	
	// Macアドレスは取得成功・失敗に関わらずそのままターミナル表示する.
	pc_FactoryHistory->getMacAddressFactory( &sc_MacAddressFactoryNum[0] );
	printf( THRD_MacAddress_DispStr	, sc_MacAddressFactoryNum[0]
									, sc_MacAddressFactoryNum[1]
									, sc_MacAddressFactoryNum[2]
									, sc_MacAddressFactoryNum[3]
									, sc_MacAddressFactoryNum[4]
									, sc_MacAddressFactoryNum[5]
									, sc_MacAddressFactoryNum[6]
									, sc_MacAddressFactoryNum[7]
									, sc_MacAddressFactoryNum[8]
									, sc_MacAddressFactoryNum[9]
									, sc_MacAddressFactoryNum[10]
									, sc_MacAddressFactoryNum[11]
									);
	
	return;
}
// ================================================================================================.
//! <!--機能-->【ターミナルコマンド】 シリアル番号表示.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param TYPS_SerialNoChangeHistory	シリアル番号変更履歴情報.
//! 		ushort						出力番号.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! 引数で受け取ったシリアル番号変更履歴情報を元にターミナルへ履歴情報を出力する.
//! <!--履歴-->.
// 2018/08/08	Sky松井	[DenebMLK][AR.1050268]新規作成.
// ================================================================================================.
void THRL_AdjustDspSerialNumber( TYPS_SerialNoChangeHistory st_SerialNoChgHistory, ushort us_Index )
{
	// getSerialNoChangeHisory()で無効データを検知した場合はバッファを*で埋める.
	// * は正常時では絶対に入らないデータなので先頭データが*ならば無効データと判断する.
	if( ( st_SerialNoChgHistory.sc_date[ 0 ] == '*' ) ||			// 日付情報が無効データか判定.
			( st_SerialNoChgHistory.sc_TotalCnt[ 0 ] == '*' )){		// トータルカウンタが無効データか判定.
		printf("%02d;\n", us_Index+1);	// 無効データの場合は番号のみ出力する.
		return;
	}

	char sc_DateDisp[ TYPD_MAX_Date + 1 ] = { 0 };								// 日付情報格納用変数.
	char sc_TotalCntDisp[ TYPD_MAX_TotalCnt + 1 ] = { 0 };						// トータルカウンタ格納用変数.
	char sc_SerialNoOriginal[ TYPD_MAX_SerialNoChangeHistory + 1 ] = { 0 };		// 編集用シリアルナンバー格納変数.
	char sc_SerialNoPrinterDisp[ TYPD_MAX_SerialNoChangeHistory + 1 ] = { 0 };	// 出力用シリアルナンバー格納変数.

	// 出力用シリアルナンバー格納変数を'*'で埋めておく.
	memset( sc_SerialNoPrinterDisp, '*', sizeof( sc_SerialNoPrinterDisp ) );

	// 取得文字列には終端文字がないのでローカル変数にコピーする.
	strncpy( sc_DateDisp, st_SerialNoChgHistory.sc_date, TYPD_MAX_Date );										// 日付情報.
	strncpy( sc_TotalCntDisp, st_SerialNoChgHistory.sc_TotalCnt, TYPD_MAX_TotalCnt );							// トータルカウンタ情報.
	strncpy( sc_SerialNoOriginal, st_SerialNoChgHistory.sc_SerialNoPrinter, TYPD_MAX_SerialNoChangeHistory );	// シリアルナンバー情報.

	// 文字列の終端に終端文字列を格納する.
	sc_DateDisp[ TYPD_MAX_Date ] = '\0';
	sc_TotalCntDisp[ TYPD_MAX_TotalCnt ] = '\0';
	sc_SerialNoOriginal[ TYPD_MAX_SerialNoChangeHistory ] = '\0';

	uchar uc_StrRange = strlen( sc_SerialNoOriginal );					// 文字数を取得.
	uchar uc_StartIdx = 0;												// コピー開始位置定義.
	if( TYPD_MAX_SerialNoChangeHistory >= uc_StrRange ){				// コピー開始位置がマイナスにならないかを判定.
		uc_StartIdx = TYPD_MAX_SerialNoChangeHistory - uc_StrRange;		// コピー開始位置を計算.
	}

	// 事前に求めたコピー開始位置から文字数分出力用シリアルナンバー格納変数に格納する.
	strncpy( &sc_SerialNoPrinterDisp[ uc_StartIdx ], sc_SerialNoOriginal, uc_StrRange );
	sc_SerialNoPrinterDisp[ TYPD_MAX_SerialNoChangeHistory ] = '\0';		// 終端に終端文字列を格納.

	// 履歴を出力.
	printf("%02d; %s; %s; %s\n"	, us_Index + 1
								, sc_DateDisp
								, sc_TotalCntDisp
								, sc_SerialNoPrinterDisp );
}

// ================================================================================================.
//! <!--機能-->【ターミナルコマンド】 シリアル番号変更履歴表示.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
// 2018/04/20	山岸	[DenebMLK]制御構成変更No.66 横流れ対応 新規作成.
//!	2018/08/09	Sky松井	[DenebMLK][AR.1050268]シリアル番号の出力を別関数で実行するように修正.
// ================================================================================================.
void GetSerialNumberChangedHistory()
{
	ushort us_Index = 0;
	TYPS_SerialNoChangeHistoryList st_SerialNoChgHistoryList;
	
	memset( &st_SerialNoChgHistoryList, 0, sizeof(TYPS_SerialNoChangeHistoryList) );
	APIC_FactoryHistory* pc_FactoryHistory = APIC_FactoryHistory::newInstance();
	
	if( pc_FactoryHistory == NULL ){
		printf("SystemErr\n");
		return;
	}
	
	pc_FactoryHistory->getSerialNoChangeHisory( &st_SerialNoChgHistoryList );
	for( us_Index = 0; us_Index < TYPD_MAX_ChangeHistory; us_Index++ ){
		// シリアル番号履歴を出力.
		THRL_AdjustDspSerialNumber( st_SerialNoChgHistoryList.pc_SerialNoChangeHistory[us_Index], us_Index );
	}
}
#if defined( DEF_A4_Cus )
/*===============================================================================================.
［機能］	ADF開閉検知センササ状態取得のコールバック.
-------------------------------------------------------------------------------------------------.
［引数］	Bool   bl_Result：結果　True：正常終了/False：異常終了.
			ushort us_Data：ADF開閉検知センサ状態.
［戻値］	なし.
［詳細］	Scan ADF開閉検知センサ状態を受信し実行結果を通知するコールバック.
			0：センサOFF（CLOSE）/1：センサON（OPEN）.
［履歴］	.
			2018/07/12	AR:1051755 eSOL 田川【Sparrow】制御構成変更_No.253 生産コマンド対応 新規作成.
================================================================================================*/
static void THRL_ans_DFOpenSensor( Bool bl_Result, ushort us_Data )
{
	printf( "\n" );
	if( bl_Result == True) {
		printf("DFOpenSensor = %d\n", us_Data );	// 処理：正常時.
	} else {
		printf("DFOpenSensor NG\n");				// 処理：異常時.
	}
}
// ================================================================================================.
//! <!--機能--> 【ターミナルコマンド】ADF開閉検知センサ表示.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! ADF開閉検知センサの値を表示する.
//! <!--履歴-->.
//	2018/07/12	AR:1051755 eSOL 田川【Sparrow】制御構成変更_No.253 生産コマンド対応 新規作成.
//  2019/07/09 Sky井柳 [SparrowBK/HeliosMLK] 制御構成変更No.6 マシンタイプ対応.
//	2019/09/09 日立畔柳[SparrowBK]【開発提起】Emu機のマシンタイプ設定不可.
// ================================================================================================.
void DFOpenSensor()
{
	//マシンタイプの取得.
	switch( THRF_GetMachineTypeInfo() ) {
		//本情報取得は、SparrowのMFP,AIO対象(SFP対象外).
		case TYPE_MachineTypeSparrow1MFP:
		case TYPE_MachineTypeSparrow2MFP:
		case TYPE_MachineTypeSparrow2AIO:
		case TYPE_MachineTypeSparrowBK0MFP:		// SparrowBK 0MFP.
		case TYPE_MachineTypeSparrowBK1MFP:		// SparrowBK 1MFP.
		case TYPE_MachineTypeSparrowBKMFPEmu1:	// SparrowBK 1MFP Emu.
		case TYPE_MachineTypeSparrowBKMFPEmu2:	// SparrowBK 2MFP Emu.
		{
			printf( "\n" );
			if( checkLineMode( False ) == False ){	// ラインモード判定.
				printf( "DFOpenSensor NG\n" );		// ラインモードではない場合NG.
				return;
			}
			SCNC_ScannerIf* pc_ScannerIf = SCNC_ScannerIf::newInstance();
			if ( NULL == pc_ScannerIf ){	// インスタンス不正.
				printf( "SystemErr\n" );
				return;
			}

			// ADF開閉検知センサ値取得要求.
			pc_ScannerIf->req_DFOpenSensor( THRL_ans_DFOpenSensor );
		}
			break;
		default:
			printf( "Machine Type Unmatch [%d]\n", THRF_GetMachineTypeInfo() );
			break;
	}
}
#endif //#if defined( DEF_A4_Cus ).

// ================================================================================================.
//! <!--機能--> 【ターミナルコマンド】テストパターン印刷.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param TYPE_FNC_TestPatternPrint	ul_Pattern				パターン.
//! \param TYPE_JobSetting				ul_JobSetting			置数.
//! \param TYPE_TestPatternParam		uc_SingleMultiType		SINGLE/MULTI.
//! \param TYPE_TestPatternParam		uc_DuplexSet			片面/両面.
//! \param TYPE_TestPatternParam		uc_SurfaceSet			全面/前半分.
//! \param TYPE_TestPatternParam		uc_PaperPortSet			給紙口.
//! \param TYPE_TestPatternParam		uc_PaperSize			用紙サイズ.
//! \param TYPE_TestPatternParam		uc_PaperType			用紙種類.
//! \param TYPE_TestPatternParam		uc_LIMOSSelection		ＬＩＭＯＳ選択.
//! \param TYPE_TestPatternParam		uc_ScreenPatter			スクリーンパターン.
//! \param TYPE_TestPatternParam		uc_Gradation			グラデーション.
//! \param TYPE_TestPatternParam		uc_ColorType			色.
//! \param TYPE_TestPatternParam		uc_Orientation			縦/横.
//! \param TYPE_TestPatternParam		uc_AngleSet				角度.
//! \param TYPE_TestPatternParam		uc_RatioMode			ON/OFF比.
//! \param TYPE_TestPatternParam		uc_ReverseMode			リバース.
//! \param TYPE_TestPatternParam		uc_ColorMode			カラー.
//! \param TYPE_TestPatternParam		uc_Resolution			解像度(dpi).
//! \param TYPE_TestPatternParam		uc_NumberModeLine		線数モード.
//! \param TYPE_TestPatternParam		uc_Repetition			繰り返し幅.
//! \param TYPE_TestPatternParam		uc_Density				濃度.
//! \param TYPE_TestPatternParam		uc_BgDensity			背景濃度.
//! \param TYPE_LineWidthLandscape		us_LineWidthLandscape	線幅(CD/横).
//! \param TYPE_TestPatternParam		uc_LineWidthPortrait	線幅(FD/縦).
//! \param TYPE_TestPatternParam		uc_DotInterVal			ドット間隔.
//! \param TYPE_TestPatternParam		uc_DotGain				ドットゲイン.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! 上位(CommonAPI)へテストパターン印刷実行の要求を行う.
//! <!--履歴-->.
//!	2018/07/25	Sky 山岸【Eagle】制御構成変更No104_テストパターン印刷のターミナルコマンド対応(生産対応） 新規作成.
// ================================================================================================.
void TestPatternPrint(
	TYPE_FNC_TestPatternPrint	ul_Pattern,
	TYPE_JobSetting				ul_JobSetting,
	TYPE_TestPatternParam		uc_SingleMultiType,
	TYPE_TestPatternParam		uc_DuplexSet,
	TYPE_TestPatternParam		uc_SurfaceSet,
	TYPE_TestPatternParam		uc_PaperPortSet,
	TYPE_TestPatternParam		uc_PaperSize,
	TYPE_TestPatternParam		uc_PaperType,
	TYPE_TestPatternParam		uc_LIMOSSelection,
	TYPE_TestPatternParam		uc_ScreenPatter,
	TYPE_TestPatternParam		uc_Gradation,
	TYPE_TestPatternParam		uc_ColorType,
	TYPE_TestPatternParam		uc_Orientation,
	TYPE_TestPatternParam		uc_AngleSet,
	TYPE_TestPatternParam		uc_RatioMode,
	TYPE_TestPatternParam		uc_ReverseMode,
	TYPE_TestPatternParam		uc_ColorMode,
	TYPE_TestPatternParam		uc_Resolution,
	TYPE_TestPatternParam		uc_NumberModeLine,
	TYPE_TestPatternParam		uc_Repetition,
	TYPE_TestPatternParam		uc_Density,
	TYPE_TestPatternParam		uc_BgDensity,
	TYPE_LineWidthLandscape		us_LineWidthLandscape,
	TYPE_TestPatternParam		uc_LineWidthPortrait,
	TYPE_TestPatternParam		uc_DotInterVal,
	TYPE_TestPatternParam		uc_DotGain)
{
	TYPS_TestPatternPrintInfo st_TestPatternPrintInfo = {};
	st_TestPatternPrintInfo.ul_Pattern = ul_Pattern;						// パターン.
	st_TestPatternPrintInfo.ul_JobSetting = ul_JobSetting;					// 置数.
	st_TestPatternPrintInfo.uc_SingleMultiType = uc_SingleMultiType;		// SINGLE/MULTI.
	st_TestPatternPrintInfo.uc_DuplexSet = uc_DuplexSet;					// 片面/両面.
	st_TestPatternPrintInfo.uc_SurfaceSet = uc_SurfaceSet;					// 全面/前半分.
	st_TestPatternPrintInfo.uc_PaperPortSet = uc_PaperPortSet;				// 給紙口.
	st_TestPatternPrintInfo.uc_PaperSize = uc_PaperSize;					// 用紙サイズ.
	st_TestPatternPrintInfo.uc_PaperType = uc_PaperType;					// 用紙種類.
	st_TestPatternPrintInfo.uc_LIMOSSelection = uc_LIMOSSelection;			// ＬＩＭＯＳ選択.
	st_TestPatternPrintInfo.uc_ScreenPatter = uc_ScreenPatter;				// スクリーンパターン.
	st_TestPatternPrintInfo.uc_Gradation = uc_Gradation;					// グラデーション.
	st_TestPatternPrintInfo.uc_ColorType = uc_ColorType;					// 色.
	st_TestPatternPrintInfo.uc_Orientation = uc_Orientation;				// 縦/横.
	st_TestPatternPrintInfo.uc_AngleSet = uc_AngleSet;						// 角度.
	st_TestPatternPrintInfo.uc_RatioMode = uc_RatioMode;					// ON/OFF比.
	st_TestPatternPrintInfo.uc_ReverseMode = uc_ReverseMode;				// リバース.
	st_TestPatternPrintInfo.uc_ColorMode = uc_ColorMode;					// カラー.
	st_TestPatternPrintInfo.uc_Resolution = uc_Resolution;					// 解像度(dpi).
	st_TestPatternPrintInfo.uc_numberModeLine = uc_NumberModeLine;			// 線数モード.
	st_TestPatternPrintInfo.uc_Repetition = uc_Repetition;					// 繰り返し幅.
	st_TestPatternPrintInfo.uc_Density = uc_Density;						// 濃度.
	st_TestPatternPrintInfo.uc_BgDensity = uc_BgDensity;					// 背景濃度.
	st_TestPatternPrintInfo.us_LineWidthLandscape = us_LineWidthLandscape;	// 線幅(CD/横).
	st_TestPatternPrintInfo.uc_LineWidthPortrait = uc_LineWidthPortrait;	// 線幅(FD/縦).
	st_TestPatternPrintInfo.uc_DotInterVal = uc_DotInterVal;				// ドット間隔.
	st_TestPatternPrintInfo.uc_DotGain = uc_DotGain;						// ドットゲイン.

	APIC_SystemInfo*  pc_SystemInfo = APIC_SystemInfo::newInstance();
	if( pc_SystemInfo == NULL ){ // インスタンス取得失敗.
		printf("SystemErr\n");
		return;
	}

	pc_SystemInfo->req_TestPatternPrint(st_TestPatternPrintInfo);
}

// ================================================================================================.
//! <!--機能--> 【ターミナルコマンド】HDDステータス スリープ解除コマンド.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! HDDステータスのSleep状態を解除するためのコマンド.
//! <!--履歴-->.
//	2018/10/23	 橋本裕史 AR:1065702 自己証明書をOwn/Generic後、再起動無しで作成できない(生産対応) 新規作成.
//  2018/11/29   Sky合屋  【Eagle】[AR1068923]スペルミス修正.
// ================================================================================================.
void SetStorageSleepEnd()
{
	FILC_FsControlIf::setHddSleepEnd(); // Sleep状態を解除.
	
	printf("SetStorageSleepEnd OK \n"); 
}

// ================================================================================================.
//! <!--機能--> 【ターミナルコマンド】NFCユニットの装着有無確認コマンド.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! NFCユニットの装着有無を確認するためのコマンド.
//! <!--履歴-->.
//	2018/10/23	橋本裕史 AR:1064429 生産コマンド追加対応 新規作成.
// ================================================================================================.
void GetInstallNFCUnitStatus()
{
	CNVC_PanelHard* pc_PanelHard = CNVC_PanelHard::newInstance();
	
	// NULLチェック.
	if(pc_PanelHard == NULL) {
		printf("pc_PanelHard NULL error \n");
		printf("SystemErr \n");
		return;
	}
	
	Bool b_ret = pc_PanelHard->isPanelOptionNFC(); 	// NFC物理的に装着されているか確認.
												 	// True：装着. False：非装着.
	if(b_ret == False){
		printf("GetInstallNFCUnitStatus NG \n");
	}else{
		printf("GetInstallNFCUnitStatus OK \n");
	}
}
#if ( defined( DEF_A4_Cus ) && defined( DEF_SparrowCus ) )
// ================================================================================================.
//! <!--機能--> 【ターミナルコマンド】PSデザイナーズ値設定コマンド
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param short s_DensityRevesionTable 1:する/0:しない.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! PSデザイナーズ設定値を変更する.
//! <!--履歴-->.
//	2018/10/23	AR:1051755 eSOL 田川【Sparrow】【生産対応】デザイナーズチョイス切り替えコマンド対応 新規作成.
// ================================================================================================.
void SetPSDesignersChoice( short s_DensityRevesionTable )
{
	//PSデザイナーズ設定値を設定する.
	if( s_DensityRevesionTable == 1 || s_DensityRevesionTable == 0 ){
		APIC_PrintJobSetting::newInstance()->setEnableDensityRevesionTable( (Bool)s_DensityRevesionTable );
		printf("Setting = %s\n", ( (Bool)s_DensityRevesionTable == True ) ? "ON" : "OFF");
	}
	//範囲外.
	else{
		printf("incorrect value\n");
	}

}
// ================================================================================================.
//! <!--機能--> 【ターミナルコマンド】PSデザイナーズ値取得コマンド
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! PSデザイナーズ設定値を取得する.
//! <!--履歴-->.
//	2018/10/23	AR:1051755 eSOL 田川【Sparrow】【生産対応】デザイナーズチョイス切り替えコマンド対応 新規作成.
// ================================================================================================.
void GetPSDesignersChoice()
{
	//PSデザイナーズ設定値を取得する.
	if( APIC_PrintJobSetting::newInstance()->getEnableDensityRevesionTable() ){
		printf("Last Value = ON\n");
	}
	else{
		printf("Last Value = OFF\n");
	}

}
#endif //#if ( defined( DEF_A4_Cus ) && defined( DEF_SparrowCus ) ).

// 2018/11/15 Sky合屋 [Eagle]【AR1067938】【型試生産】【No31】SetSliderPositionコマンド　マージ対応.
#define THRD_SET_SLIDER_POSITION_TIMEOUT		60000	// タイムアウト時間(60秒).

// スライダーホーム位置移動のコールバック処理のステータス.
typedef uchar THRE_SET_SLIDER_POSITION_STATUS;
#define THRE_SET_SLIDER_POSITION_STATUS_INIT		0	// 初期状態.
#define THRE_SET_SLIDER_POSITION_STATUS_EXEC		1	// 実行中.
#define THRE_SET_SLIDER_POSITION_STATUS_END			2	// 完了.

THRE_SET_SLIDER_POSITION_STATUS		e_SetSliderPositionStatus = THRE_SET_SLIDER_POSITION_STATUS_INIT;

// ================================================================================================.
//! <!--機能--> ENGC_ScannerImpColorからのコールバック.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param ushort us_Result : 実行結果 正常終了(1:TYPD_FNC_OK).
//!                                    異常終了(5:TYPD_FNC_NG).
//! \param ushort us_Data   : 未使用のため、値は「0」固定で設定される.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! ENGC_ScannerImpColorからのコールバックの処理を行う.
//! <!--履歴-->.
//! 2018.11.15	Sky合屋 [Eagle]【AR1067938】【型試生産】【No31】SetSliderPositionコマンド　マージ対応.
// ================================================================================================.
void THRL_SetSliderPositionCheckCallBack( ushort us_Result, ushort us_Data ){
	us_Data = 0;	// 未使用変数のWarning対策.

	// スライダーホーム位置移動のコールバック処理のステータスが完了の場合.
	if( THRE_SET_SLIDER_POSITION_STATUS_END == e_SetSliderPositionStatus ){
		return;
	}

	if( TYPD_FNC_OK == us_Result ) {
		printf( "SetSliderPosition OK\n" );
	} else {
		printf( "SetSliderPosition NG\n" );
	}
	// スライダーホーム位置移動のコールバックを「完了」に設定.
	e_SetSliderPositionStatus = THRE_SET_SLIDER_POSITION_STATUS_END;

	// スライダーホーム位置移動チェックタイマーのキャンセル要求.
	cancelDelayMessage( TM_CheckSetSliderPosition, 0 );
// 2022/03/30 生産RINNE対応 Sky start.
	SYSD_WEBAPI_LINEMODE_WAKEUP
// 2022/03/30 生産RINNE対応 Sky end.
}


// ================================================================================================.
//! <!--機能--> 【ターミナルコマンド】スライダーホーム位置移動のファンクションを開始.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! スライダーホーム位置移動ファンクション処理を開始する.
//! <!--履歴-->.
//! 2018.11.15	Sky合屋 [Eagle]【AR1067938】【型試生産】【No31】SetSliderPositionコマンド　マージ対応.
// ================================================================================================.
extern "C" {
	void SetSliderPosition();
}
void SetSliderPosition(){
	// スライダーホーム位置移動のコールバック処理のステータスが実行中の場合.
	if( THRE_SET_SLIDER_POSITION_STATUS_EXEC == e_SetSliderPositionStatus ){
		return;
	}
	// ScanPにスライダーホーム位置移動ファンクションの開始要求を行う.
	ENGC_ScannerImpColor* pc_ScannerImpColor = ENGC_ScannerImpColor::newInstance();
	if( NULL == pc_ScannerImpColor ){
		printf( "SetSliderPosition NG\n" );
		return;
	}
	ushort us_Code = toCommandtCode( TYPD_FNC_IRHomePositionMove );			// ファンクションコード.
	ushort us_Parameter = toCommandtPara( TYPD_FNC_IRHomePositionMove );	// ファンクションパラメータ.
	// スライダーホーム位置移動のコールバックを「実行中」に設定.
	e_SetSliderPositionStatus = THRE_SET_SLIDER_POSITION_STATUS_EXEC;
	pc_ScannerImpColor->req_startFunction( us_Code, us_Parameter, THRL_SetSliderPositionCheckCallBack );

	// スライダーホーム位置移動チェックタイマーの通知要求.
	setDelayMessage( TID_ThreadManager, TM_CheckSetSliderPosition, 0, THRD_SET_SLIDER_POSITION_TIMEOUT );
// 2022/03/30 生産RINNE対応 Sky start.
	SYSD_WEBAPI_LINEMODE_WAIT
// 2022/03/30 生産RINNE対応 Sky end.
}

// ================================================================================================.
//! <!--機能--> 【ターミナルコマンド】ブラウザタスクの停止.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! フォーマット通知を実行する.
//! インスタンス生成に失敗したときは「SystemErr」を出力する.
//! ※フォーマット通知を送る事でブラウザタスクが停止する.
//! <!--履歴-->.
//! 2018/12/05	Sky松井 [EagleH]制御構成変更No.62 新規作成.
// ================================================================================================.
void BrowserTaskStop(){
	SYSC_HddInfoIf* pc_SYSC_HddInfoIf = SYSC_HddInfoIf::newInstance();

	if( pc_SYSC_HddInfoIf == NULL ){	// インスタンス取得失敗時.
		printf( "SystemErr\n" );		// インスタンス取得失敗時のログを出力.
		return;
	}
	pc_SYSC_HddInfoIf -> hddFormatNotice();	// フォーマット通知を実行する.
	printf( "BrowserTaskStop OK\n" );		// 実行OK時のログを出力.
}

extern "C" void MachineType_DebugON(char sc_MachineType);
// ================================================================================================.
//! <!--機能--> 【ターミナルコマンド】マシンタイプセット用コマンド(RAM用)
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param THRE_MACHINEINFO  e_MachineInfo	機種情報.
//!	       THRE_MACHINENO    e_MachineNo	機種番号.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! RAMへマシンタイプを設定する.
//! <!--履歴-->.
//	2018/12/07	AR:1069261 eSOL 田川【Sparrow】【生産対応】ラインカード起動時、ADF非装着問題対応 新規作成.
//	2019.10.04  			 Sky吉村(魁) SparrowBK制御構成変更No.3  生産コマンド対応 変更.
//	2019/11/22	OP_BTS-14959 Sky吉村(魁) HeliosMLK制御構成変更No.52 SetMachineType_ToRam対応 変更.
// ================================================================================================.
void SetMachineType_ToRam( THRE_MACHINEINFO e_MachineInfo, THRE_MACHINENO e_MachineNo )
{
	if( checkLineMode() == False ){	// ラインモードであるかの確認.
		return;
	}
	
	TYPE_MachineType e_SetMachineType = TYPE_MachineTypeEagle1;						// マシンタイプ取得用(IT6先頭マシンタイプで初期化).
	THRE_MACHINETYPESELECT_RESULT e_SelectResult = THRE_MACHINETYPESELECT_SUCCESS;	// マシンタイプ取得関数の結果.

	//2019.11.22 Sky吉村(魁) HeliosMLK制御構成変更No.52 SetMachineType_ToRam対応 マシンタイプ取得処理追加.
	e_SelectResult = THRF_MachineTypeSelect( e_MachineInfo, e_MachineNo, &e_SetMachineType );

	////2019.11.22 Sky吉村(魁) HeliosMLK制御構成変更No.52 SetMachineType_ToRam対応 分岐処理変更.
	switch ( e_SelectResult ){
	case THRE_MACHINETYPESELECT_SUCCESS:		// 正常にマシンタイプを取得.
		//2019.10.04 Sky吉村(魁) 制御構成変更_No.3_生産コマンド対応 仕様書に基づきログ追加.
		printf("SetMachineType_ToRam OK\n");
		printf("### Set to RAM　 MachineType : [%d] \n", e_SetMachineType );
		MachineType_DebugON( e_SetMachineType );
		break;
	case THRE_MACHINETYPESELECT_INFO_ERROR:		// 機種情報の指定値エラー.
		//2019.10.04 Sky吉村(魁) 制御構成変更_No.3_生産コマンド対応 仕様書に基づきログ追加.
		printf("SetMachineType_ToRam NG\n");
		printf("### MachineInfo NG [%x] \n", e_MachineInfo );
		break;
	case THRE_MACHINETYPESELECT_NUM_ERROR:		// 機種番号の指定値エラー.
		//2019.10.04 Sky吉村(魁) 制御構成変更_No.3_生産コマンド対応 仕様書に基づきログ追加.
		printf("SetMachineType_ToRam NG\n");
		printf("### MachineType NG [%x] \n", e_MachineNo );
		break;
	default:
		//Do Nothing.
		break;
	}
}
// ================================================================================================.
//! <!--機能--> マシンタイプ取得処理.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param THRE_MACHINEINFO  e_MachineInfo	機種情報.
//!	       THRE_MACHINENO    e_MachineNo	機種番号.
//!	       TYPE_MachineType* pe_MachineType	マシンタイプ.
//! <!--戻値-->.
//! \return THRE_MACHINETYPESELECT_RESULT 0:マシンタイプ取得成功, 1:機種情報エラー, 2:機種番号エラー, 3:引数NULLエラー.
//! <!--詳細-->.
//! 機種情報と機種番号を受け取り、一意なマシンタイプを特定後、引数のマシンタイプに設定する.
//! <!--履歴-->.
//	2019/11/20	OP_BTS-14959 Sky吉村(魁) HeliosMLK制御構成変更No.52 SetMachineType_ToRam対応 新規作成.
//	2021/08/23	OP_BTS-35195 Sky		 MinervaSSBKの機種構成を追加.
//	2022/09/21	OP_BTS-39462 Sky		 EagleⅢ/EagleBKⅢ/SparrowⅢ/SparrowBKⅢの機種情報を追加.
// ================================================================================================.	
THRE_MACHINETYPESELECT_RESULT THRF_MachineTypeSelect( THRE_MACHINEINFO e_MachineInfo, THRE_MACHINENO e_MachineNo, TYPE_MachineType* pe_MachineType )
{
	
	std::map< THRE_MACHINENO, TYPE_MachineType > m_Eagle_MachineTypeList;							// Eagle用リスト.
	std::map< THRE_MACHINENO, TYPE_MachineType > m_Sparrow_MachineTypeList;							// Sparrow用リスト.
	std::map< THRE_MACHINENO, TYPE_MachineType > m_DenebMLK_MachineTypeList;						// DenebMLK用リスト.
	std::map< THRE_MACHINENO, TYPE_MachineType > m_EagleBK_MachineTypeList;							// EagleBK用リスト.
	std::map< THRE_MACHINENO, TYPE_MachineType > m_SparrowBK_MachineTypeList;						// SparrowBK用リスト.
	std::map< THRE_MACHINENO, TYPE_MachineType > m_HeliosMLK_MachineTypeList;						// HeliosMLK用リスト.
	std::map< THRE_MACHINENO, TYPE_MachineType > m_MinervaSSBK_MachineTypeList;						// MinervaSSBK用リスト.
	std::map< THRE_MACHINEINFO, std::map< THRE_MACHINENO, TYPE_MachineType > > m_MachineInfoList;	// 機種情報リスト.
	
	if( pe_MachineType == NULL ) {
		// マシンタイプ格納場所が渡されていない.
		return THRE_MACHINETYPESELECT_ARGS_ERROR;
	}
	
	// Ealge用マシンタイプ検索リスト作成.
	m_Eagle_MachineTypeList[ THRE_MACHINENO_1 ]				=	TYPE_MachineTypeEagle1;				//	Eagle1.
	m_Eagle_MachineTypeList[ THRE_MACHINENO_2 ]				=	TYPE_MachineTypeEagle2;				//	Eagle2.
	m_Eagle_MachineTypeList[ THRE_MACHINENO_2_5 ]			=	TYPE_MachineTypeEagle2_5;			//	Eagle2.5.
	m_Eagle_MachineTypeList[ THRE_MACHINENO_Z ]				=	TYPE_MachineTypeEagleZ;				//	EagleZ.
	m_Eagle_MachineTypeList[ THRE_MACHINENO_X ]				=	TYPE_MachineTypeEagleX;				//	EagleX.
	m_Eagle_MachineTypeList[ THRE_MACHINENO_0 ]				=	TYPE_MachineTypeEagle0;				//	Eagle0.
	m_Eagle_MachineTypeList[ THRE_MACHINENO_ZP ]			=	TYPE_MachineTypeEagleZplus;			//	EagleZ+.
	
	// Sparrow用マシンタイプ検索リスト作成.
	m_Sparrow_MachineTypeList[ THRE_MACHINENO_1 ]			=	TYPE_MachineTypeSparrow1MFP;		//	Sparrow1 MFP.
	m_Sparrow_MachineTypeList[ THRE_MACHINENO_2 ]			=	TYPE_MachineTypeSparrow2MFP;		//	Sparrow2 MFP.
	m_Sparrow_MachineTypeList[ THRE_MACHINENO_2AIO ]		=	TYPE_MachineTypeSparrow2AIO;		//	Sparrow2 AIO.
	m_Sparrow_MachineTypeList[ THRE_MACHINENO_1SFP ]		=	TYPE_MachineTypeSparrow1SFP;		//	Sparrow1 SFP.
	m_Sparrow_MachineTypeList[ THRE_MACHINENO_2SFP ]		=	TYPE_MachineTypeSparrow2SFP;		//	Sparrow2 SFP.
	
	// DenebMLK用マシンタイプ検索リスト作成.
	m_DenebMLK_MachineTypeList[ THRE_MACHINENO_1 ]			=	TYPE_MachineTypeDenebMLK1;			//	DenebMLK1.
	m_DenebMLK_MachineTypeList[ THRE_MACHINENO_2 ]			=	TYPE_MachineTypeDenebMLK2;			//	DenebMLK2.
	m_DenebMLK_MachineTypeList[ THRE_MACHINENO_3 ]			=	TYPE_MachineTypeDenebMLK3;			//	DenebMLK3.
	m_DenebMLK_MachineTypeList[ THRE_MACHINENO_4 ]			=	TYPE_MachineTypeDenebMLK4;			//	DenebMLK4.
	
	// EagleBK用マシンタイプ検索リスト作成.
	m_EagleBK_MachineTypeList[ THRE_MACHINENO_Z ]			=	TYPE_MachineTypeEagleBKZ;			//	EagleBK Z.
	m_EagleBK_MachineTypeList[ THRE_MACHINENO_X ]			=	TYPE_MachineTypeEagleBKX;			//	EagleBK X.
	m_EagleBK_MachineTypeList[ THRE_MACHINENO_0 ]			=	TYPE_MachineTypeEagleBK0;			//	EagleBK 0.
	m_EagleBK_MachineTypeList[ THRE_MACHINENO_1 ]			=	TYPE_MachineTypeEagleBK1;			//	EagleBK 1.
	m_EagleBK_MachineTypeList[ THRE_MACHINENO_2 ]			=	TYPE_MachineTypeEagleBK2;			//	EagleBK 2.
	m_EagleBK_MachineTypeList[ THRE_MACHINENO_ZP ]			=	TYPE_MachineTypeEagleBKZplus;		//	EagleBKZ+.
	
	// SparrowBK用マシンタイプ検索リスト作成.
	m_SparrowBK_MachineTypeList[ THRE_MACHINENO_0 ]			=	TYPE_MachineTypeSparrowBK0MFP;		//	SparrowBK0 MFP.
	m_SparrowBK_MachineTypeList[ THRE_MACHINENO_1 ]			=	TYPE_MachineTypeSparrowBK1MFP;		//	SparrowBK1 MFP.
	m_SparrowBK_MachineTypeList[ THRE_MACHINENO_0SFP ]		=	TYPE_MachineTypeSparrowBK0SFP;		//	SparrowBK0 SFP.
	
	// HeliosMLK用マシンタイプ検索リスト作成.
	m_HeliosMLK_MachineTypeList[ THRE_MACHINENO_2 ]			=	TYPE_MachineTypeHeliosMLK2;			//	HeliosMLK2.
	m_HeliosMLK_MachineTypeList[ THRE_MACHINENO_2_5 ]		=	TYPE_MachineTypeHeliosMLK2_5;		//	HeliosMLK2.5.
	m_HeliosMLK_MachineTypeList[ THRE_MACHINENO_3 ]			=	TYPE_MachineTypeHeliosMLK3;			//	HeliosMLK3.
	m_HeliosMLK_MachineTypeList[ THRE_MACHINENO_2_CN ]		=	TYPE_MachineTypeHeliosMLK2_CNDF;	//	HeliosMLK 2		中国仕向DF標準.
	m_HeliosMLK_MachineTypeList[ THRE_MACHINENO_2_5_CN ]	=	TYPE_MachineTypeHeliosMLK2_5_CN;	//	HeliosMLK2.5 	中国仕向.
	m_HeliosMLK_MachineTypeList[ THRE_MACHINENO_2_5_Other ]	=	TYPE_MachineTypeHeliosMLK2_5_CNDF;	//	HeliosMLK2.5	中国仕向DF標準.
	m_HeliosMLK_MachineTypeList[ THRE_MACHINENO_3_CN ]		=	TYPE_MachineTypeHeliosMLK3_CN;		//	HeliosMLK3		中国仕向.
	
	// MinervaSSBK用マシンタイプ検索リスト作成.
	m_MinervaSSBK_MachineTypeList[ THRE_MACHINENO_X ]		=	TYPE_MachineTypeMinervaSSBKX;		//	MinervaSSBK X.
	m_MinervaSSBK_MachineTypeList[ THRE_MACHINENO_0_US ]	=	TYPE_MachineTypeMinervaSSBK0_US;	//	MinervaSSBK 0 [US向け].
	m_MinervaSSBK_MachineTypeList[ THRE_MACHINENO_0_EU ]	=	TYPE_MachineTypeMinervaSSBK0_EU;	//	MinervaSSBK 0 [EU向け].
	
	// 各機種毎のリストを機種情報で紐づける.
	m_MachineInfoList[ THRE_MACHINEINFO_EAGLE ]				=	m_Eagle_MachineTypeList;
	m_MachineInfoList[ THRE_MACHINEINFO_SPARROW ]			=	m_Sparrow_MachineTypeList;
	m_MachineInfoList[ THRE_MACHINEINFO_DENEBMLK ]			=	m_DenebMLK_MachineTypeList;
	m_MachineInfoList[ THRE_MACHINEINFO_EAGLEBK ]			=	m_EagleBK_MachineTypeList;
	m_MachineInfoList[ THRE_MACHINEINFO_SPARROWBK ]			=	m_SparrowBK_MachineTypeList;
	m_MachineInfoList[ THRE_MACHINEINFO_HELIOSMLK ]			=	m_HeliosMLK_MachineTypeList;
	m_MachineInfoList[ THRE_MACHINEINFO_MINERVASSBK ]		=	m_MinervaSSBK_MachineTypeList;
	m_MachineInfoList[ THRE_MACHINEINFO_EAGLE3 ]			=	m_Eagle_MachineTypeList;
	m_MachineInfoList[ THRE_MACHINEINFO_SPARROW3 ]			=	m_Sparrow_MachineTypeList;
	m_MachineInfoList[ THRE_MACHINEINFO_EAGLEBK3 ]			=	m_EagleBK_MachineTypeList;
	m_MachineInfoList[ THRE_MACHINEINFO_SPARROWBK3 ]		=	m_SparrowBK_MachineTypeList;
	
	// 機種情報の指定値が誤っている.
	if ( m_MachineInfoList.count( e_MachineInfo ) == 0 ){
		return THRE_MACHINETYPESELECT_INFO_ERROR;
	}
	
	// 機種情報は正しいが機種番号の指定値が誤っている.
	if ( m_MachineInfoList[ e_MachineInfo ].count( e_MachineNo ) == 0 ){
		return THRE_MACHINETYPESELECT_NUM_ERROR;
	}
	
	// 機種情報と機種番号の組み合わせに合致するマシンタイプの取得.
	*pe_MachineType = (m_MachineInfoList[ e_MachineInfo ])[ e_MachineNo ];
	
	return THRE_MACHINETYPESELECT_SUCCESS;
}
#if ( defined( DEF_A4_Cus ) && defined( DEF_SparrowCus ) )
// ================================================================================================.
//! <!--機能--> 【ターミナルコマンド】主走査方向倍率パターン印刷用コマンドのコールバック関数.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param ushort.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! 主走査方向倍率パターン印刷用APIの結果を返す.
//! <!--履歴-->.
//	2018/12/14	AR:1070278 eSOL 田川【Sparrow】【生産対応】主走査方向倍率パターン印刷用コマンド作成 新規作成.
// ================================================================================================.
void ans_startMainScanAdjustPrint( ushort us_result )
{
	//ExecADFMainZoomPatternPrint()は、req_startListPrint()の呼び出し結果に関わらずターミナル上にOKを返すため、.
	//本コールバック関数は空とする.

}
// ================================================================================================.
//! <!--機能--> 【ターミナルコマンド】主走査方向倍率パターン印刷用コマンド.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! 主走査方向倍率パターン印刷用APIを呼び出す.
//! <!--履歴-->.
//	2018/12/14	AR:1070278 eSOL 田川【Sparrow】【生産対応】主走査方向倍率パターン印刷用コマンド作成 新規作成.
// ================================================================================================.
void ExecADFMainZoomPatternPrint()
{
	//API呼び出し.
	APIC_ListPrint*	pc_ListPrint = APIC_ListPrint::newInstance();
	if( pc_ListPrint != NULL ) {
		pc_ListPrint->req_startListPrint(
			TYPE_ListPrintMainScanAdjust,		// リストプリント種別.
			( uint32_t )TYPE_FeedTray1,			// トレイ指定.
			ans_startMainScanAdjustPrint,		// コールバック.
			-1,									// ボックス番号.
			0,									// ドキュメント番号.
			NULL,								// 構造体へのポインタ.
			0,									// 片面(0)/両面(1).
			PDLE_JobEntryIF_OTHER );			// インタフェース番号(その他).
	}
	else
	{
		printf("APIC_ListPrint::newInstance NG \n");
	}
	
	printf("ExecADFMainZoomPatternPrint OK \n");
	
}
#endif //#if ( defined( DEF_A4_Cus ) && defined( DEF_SparrowCus ) ).


// 2019.01.28 [EagleH]制御構成変更No.60 自己診断結果収集機能API追加対応 ->.
// ================================================================================================.
//! <!--機能--> ストレージのシリアルNo取得API.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param char *pc_SerialNo : 取得シリアルNo(20+1Byte).
//! <!--戻値-->.
//! \return Bool True：取得成功  Flase：取得失敗.
//! <!--詳細-->.
//! 装着SSD、またはmicroSDのシリアル番号を返す.
//! <!--履歴-->.
//! 2019/01/28	Sky [EagleH]制御構成変更No.60 新規作成.
// ================================================================================================.
Bool GetStorageSerialNo( char *pc_SerialNo )
{
#ifdef FUM_LINUX				// 本関数は実機のみ有効.

	return True;

#else // FUM_LINUX.

	Bool bl_Result = False;

	if( pc_SerialNo == NULL ){
		THRL_SetThLog( 694, THRE_DebugLogType_MTCtrLog, 0, True, 0 );
		return bl_Result;
	}
	memset( pc_SerialNo, 0x00, HDD_BUFSIZE_SERIALNO+1 );

	uint32_t ui_ret = HDD_STATE_OK;
	ST_HDD_DEVICE_INFO	stDeviceInfo;
	memset( &stDeviceInfo, 0x00, sizeof( ST_HDD_DEVICE_INFO ) );

	// SSD起動のとき.
	if( THRE_StartupDeviceType_SD != THRF_GetStartupDeviceType() ){

		// ストレージデバイス情報を取得.
		// SSD起動の場合は、NVMeのIdentifyコマンドでシリアル番号を取得する.
		ui_ret = sysBspHddDeviceInfoGet( DEF_DEVICEID_HDD, 0, &stDeviceInfo );
		if( HDD_STATE_OK != ui_ret ){
			THRL_SetThLog( 695, THRE_DebugLogType_MTCtrLog, 0, True, ui_ret );
			return bl_Result;
		}

		strncpy( pc_SerialNo, stDeviceInfo.ctHddSerialNo, strlen(stDeviceInfo.ctHddSerialNo) );
		bl_Result = True;

	// microSD起動のとき.
	}else{

		// microSD起動の場合は、CID情報からシリアル番号を取得する.

		char pc_str_cid[THRD_CIDStringBufferLength+1] = {'\0'};						 /* CID文字列情報   */

		FILE *pf_fp = NULL;
		char *pc_path_cid = THRD_CIDDeviceFileName;  								 /* デバイスCID情報 */

		// CID情報を取得する.
		pf_fp = fopen(pc_path_cid, "r");
		if(pf_fp == NULL) {
			THRL_SetThLog( 696, THRE_DebugLogType_MTCtrLog, 0, True, errno );
			return bl_Result;
		}

		if(fgets(pc_str_cid, (sizeof(pc_str_cid)-1), pf_fp) == NULL){
			THRL_SetThLog( 697, THRE_DebugLogType_MTCtrLog, 0, True, errno );
			fclose(pf_fp);
			return bl_Result;
		}

		fclose(pf_fp);

		// CID情報の19Byte目から8Byte分がシリアルNo(PSN).
		// CID情報からシリアルNoを取得.
		int i_cnt1 = 0;
		int i_cnt2 = 0;
		int i_cnt3 = 0;

		for( i_cnt1 = 0; i_cnt1 < THRD_CIDStringBufferLength; i_cnt1++ ){
			if( pc_str_cid[i_cnt1] == NULL ) {						// NULL文字発見時はBreak.
				break;
			}

			if( pc_str_cid[i_cnt1] > THRD_CharBlank ) {				// 文字コードとして空白より大きい.
				if( i_cnt2 >= THRD_PSNStartPositionInCID ) {		// シリアルNo開始位置以降の文字のとき.
					pc_SerialNo[i_cnt3] = pc_str_cid[i_cnt1];
					i_cnt3 = i_cnt3 + 1;
				}
				i_cnt2 = i_cnt2 + 1;
			}

			// シリアルNoのサイズ分、文字を取得した？.
			if( i_cnt3 >= THRD_uSDSeriarlLength ) {
				break;
			}
		}

		bl_Result = True;
	}

	// シリアルNoが取得できた場合、文字列前後の空白を削除する.
	if( bl_Result == True ) {
		THRL_TrimString( pc_SerialNo );		// 文字列前後の空白を削除.
	}

	return bl_Result;

#endif // FUM_LINUX.
}
// 2019.01.28 [EagleH]制御構成変更No.60 自己診断結果収集機能API追加対応 <-.

// 2019.06.30 [EagleH]制御構成変更No.60 OP_BTS-5581_メーカーID取得API追加対応 ->.
// ================================================================================================.
//! <!--機能--> 装着ストレージのVenderID取得関数.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return uint32_t ベンダーID.
//!                  取得失敗時は0が入る.
//!                  EagleH(IT6_1.0)時点で接続の可能性があるストレージのVenderIDは以下.
//!                           SSD時   TOSHIBA :0x1179  (10進表記では4473).
//!                                   PHISON  :0x1987  (10進表記では6535).
//!
//!                           μSD時  ADATA   :0x1d  (10進表記では29).
//!                                   PHISON  :0x27  (10進表記では39).
//! <!--詳細-->.
//!  装着ストレージのVenderIDを返す.
//! <!--履歴-->.
//!  2019.06.30 [EagleH]制御構成変更No.60 OP_BTS-5581_メーカーID取得API追加対応 新規作成.
// ================================================================================================.
uint32_t GetStorageVenderID( void )
{
#ifdef FUM_LINUX				// 本関数は実機のみ有効.

	return 0;

#else // FUM_LINUX.

	uint32_t ui32_VendID = 0;		// ベンダID格納変数.
	size_t si_Ret = 0;

	// ストレージデバイス情報を取得.
	// SSD起動のとき.
	if( THRE_StartupDeviceType_SD != THRF_GetStartupDeviceType() ){

		// SSDベンダID格納ファイルをオープン.
		FILE *pF_File = fopen( THRD_VenderID_DeviceFileName, "r" );
		if( pF_File == NULL ){
			THRL_SetThLog( 701, THRE_DebugLogType_MTCtrLog, 0, True, errno );
			return 0;
		}

		// 読み出したバージョン情報(0x****)から****を取り出すため、読み出し位置を移動.
		si_Ret = fseek( pF_File, THRD_VenderPreID_StrSize, SEEK_SET );
		if( si_Ret < 0 ){
			fclose( pF_File );
			THRL_SetThLog( 702, THRE_DebugLogType_MTCtrLog, 0, True, errno );
			return 0;
		}

		// SSDベンダID格納ファイルから取得した値からSSDベンダIDの値(****)を取得.
		char sc_VenderID[THRD_VenderID_StrSize + 1] = {0};
		si_Ret = fread( sc_VenderID, sizeof(char), THRD_VenderID_StrSize, pF_File );
		if( si_Ret <= 0  ){
			fclose( pF_File );
			THRL_SetThLog( 703, THRE_DebugLogType_MTCtrLog, 0, True, errno );
			return 0;
		}

		fclose( pF_File ); // SSDベンダID格納ファイルをClose.

		// 読み出したデータは文字列であるため、数値に変換.
		ui32_VendID = 0; // 文字列から数値変換後の値格納変数.
		int si_ScanRet = sscanf( sc_VenderID, "%x", &ui32_VendID ); // 文字列から数値へ変換.
		if( si_ScanRet == EOF ){
			THRL_SetThLog( 704, THRE_DebugLogType_MTCtrLog, 0, True, (long)sc_VenderID );
			return 0;
		}

	// microSD起動のとき.
	} else {

		ui32_VendID = 0;
		Bool b_MakerIDGet = sysBspGetMicroSdMakerID( &ui32_VendID );
		if( b_MakerIDGet != True ) {
			THRL_SetThLog( 705, THRE_DebugLogType_MTCtrLog, 0, True );
			return 0;
		}

	}

	return ui32_VendID;

#endif // FUM_LINUX.
}
// 2019.06.30 [EagleH]制御構成変更No.60 OP_BTS-5581_メーカーID取得API追加対応 <-.

// 2021.03.02 PhisonSSD 96層対応 ->.
// ================================================================================================.
//! <!--機能--> 装着ストレージのVenderID取得関数.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return uint32_t デバイスID.
//!                  取得失敗時は0が入る.
//!                  接続の可能性があるストレージのデバイスIDは以下.
//!                           SSD時   PHISON 64層 :0x5007  .
//!                                   PHISON 64層 :0x5008  .
//!                                   PHISON 96層 :0x5013  .
//!
//!
//! <!--詳細-->.
//!  装着ストレージのデバイスIDを返す.
//! <!--履歴-->.
//!  2021.03.02 PhisonSSD 96層対応 新規作成.
// ================================================================================================.
uint32_t GetStorageDeviceID( void )
{
#ifdef FUM_LINUX				// 本関数は実機のみ有効.

	return 0;

#else // FUM_LINUX.

	uint32_t ui32_DeviceID = 0;		// デバイスID格納変数.
	size_t si_Ret = 0;

	// ストレージデバイス情報を取得.
	// SSD起動のとき.
	if( THRE_StartupDeviceType_SD != THRF_GetStartupDeviceType() ){

		// SSDカーネルID格納ファイルをオープン.
		FILE *pF_File = fopen( THRD_DeviceID_DeviceFileName, "r" );
		if( pF_File == NULL ){
			THRL_SetThLog( 747, THRE_DebugLogType_MTCtrLog, 0, True, errno );
			return 0;
		}

		// 読み出したバージョン情報(0x****)から****を取り出すため、読み出し位置を移動.
		si_Ret = fseek( pF_File, THRD_DevicePreID_StrSize, SEEK_SET );
		if( si_Ret < 0 ){
			fclose( pF_File );
			THRL_SetThLog( 748, THRE_DebugLogType_MTCtrLog, 0, True, errno );
			return 0;
		}

		// SSDベンダID格納ファイルから取得した値からSSDベンダIDの値(****)を取得.
		char sc_DeviceID[THRD_DeviceID_StrSize + 1] = {0};
		si_Ret = fread( sc_DeviceID, sizeof(char), THRD_DeviceID_StrSize, pF_File );
		if( si_Ret <= 0  ){
			fclose( pF_File );
			THRL_SetThLog( 749, THRE_DebugLogType_MTCtrLog, 0, True, errno );
			return 0;
		}

		fclose( pF_File ); // SSDベンダID格納ファイルをClose.

		// 読み出したデータは文字列であるため、数値に変換.
		ui32_DeviceID = 0; // 文字列から数値変換後の値格納変数.
		int si_ScanRet = sscanf( sc_DeviceID, "%x", &ui32_DeviceID ); // 文字列から数値へ変換.
		if( si_ScanRet == EOF ){
			THRL_SetThLog( 750, THRE_DebugLogType_MTCtrLog, 0, True, (long)ui32_DeviceID );
			return 0;
		}

	// microSD起動のとき.
	} else {
		// 対象外のため処理なし.
	}

	return ui32_DeviceID;

#endif // FUM_LINUX.
}
// 2021.03.02 PhisonSSD 96層対応 <-.

// ================================================================================================.
//! <!--機能--> 【ターミナルコマンド】SPI_Flash_DataRestoreダミーコマンド.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return 常に1.
//! <!--詳細-->.
//! APPI1工程以降にてラインアウトが発生時に.
//! 誤ってMFPアプリから、SPI-Flashリストアシェルを実行された場合、エラーにする.
//! <!--履歴-->.
//	2019/03/07	AR:1076046 Sky【生産改善】SPI_Flash_DataRestoreダミーコマンド作成 新規作成.
// ================================================================================================.
int SPI_Flash_DataRestore(void)
{
	printf("Restore Start\n");
	printf("[ERROR] Can not be executed here\n");
	printf("Error End\n");

	return 1;
}

// ================================================================================================.
//! <!--機能--> 【ターミナルコマンド】SPI_Flash_DataDeleteコマンド.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return 0.
//! <!--詳細-->.
//! MFPアプリからも、SPI-Flashデリートシェルを実行可能にする.
//! <!--履歴-->.
//	2019/05/31	Sky【生産改善】SPI_Flash_DataDeleteコマンド作成 新規作成.
// ================================================================================================.
int SPI_Flash_DataDelete(void)
{
	system( "SPI_Flash_DataDelete" );
	
	return 0;
}

// ================================================================================================.
//! <!--機能--> 【ターミナルコマンド】ネットワークインタフェース構成確認.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! ネットワークインタフェース構成の設定内容をターミナル上に表示する.
//! <!--履歴-->.
//	2021/03/03	Sky	OP_BTS-31220 生産ライン用 APPコマンド追加.
// ================================================================================================.
void GetNetworkIFSetting( void )
{
	APIC_ExNetworkSetting* pc_NetworkSetting = APIC_ExNetworkSetting::newInstance();
	if( NULL == pc_NetworkSetting )
	{
		printf("SystemErr\n");
		return;
	}

	TYPE_NetworkIFMode e_NwIfMode = pc_NetworkSetting->getNetworkIFMode();
	THRE_NwIfMode e_Result = THRE_NwIfMode_SingleEthernet;

 	switch ( e_NwIfMode )
	{
		case TYPE_NetworkIFMode_SingleEthernet:					// シングルIF構成 有線モード.
			e_Result = THRE_NwIfMode_SingleEthernet;
			break;
		case TYPE_NetworkIFMode_SingleSta:						// シングルIF構成 無線子機モード.
			e_Result = THRE_NwIfMode_SingleSta;
			break;
		case TYPE_NetworkIFMode_DualEthernetSta:				// デュアルIF構成 有線＋無線子機モード.
			e_Result = THRE_NwIfMode_DualEthernetSta;
			break;
		case TYPE_NetworkIFMode_DualEthernetAp:					// デュアルIF構成 有線＋無線APモード.
			e_Result = THRE_NwIfMode_DualEthernetAp;
			break;
		case TYPE_NetworkIFMode_DualEthernetWolLimited:			// デュアルIF構成 有線＋無線子機（ErP復帰限定）モード.
			e_Result = THRE_NwIfMode_DualEthernetWolLimited;
			break;
		case TYPE_NetworkIFMode_DualEthernet:					// デュアルIF構成 有線モード.
			e_Result = THRE_NwIfMode_DualEthernet;
			break;
		case TYPE_NetworkIFMode_DualEthernetWfd:				// デュアルIF構成 有線＋Wi-Fi Directモード.
			e_Result = THRE_NwIfMode_DualEthernetWfd;
			break;
		default:
			return;
	}

	printf("NetworkIFSetting = %d\n", e_Result);
}

// ================================================================================================.
//! <!--機能--> 【ターミナルコマンド】ネットワークインタフェース構成変更.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param THRE_NwIfMode e_SettingMode 0：有線のみ、1：有線+無線（子機モード）.
//! <!--戻値-->.
//! \return 0.
//! <!--詳細-->.
//! 引数に指定したモードを、ネットワークインターフェース構成に設定する.
//! 引数の指定が無い場合、引数0として動作する(ターミナルコマンドでは、引数なし=0となる).
//! 引数が範囲外の場合、エラーとする.
//! <!--履歴-->.
//	2021/03/03	Sky	OP_BTS-31220 生産ライン用 APPコマンド追加.
// ================================================================================================.
void SetNetworkIFSetting(THRE_NwIfMode e_SettingMode)
{
	APIC_ExNetworkSetting* pc_NetworkSetting = APIC_ExNetworkSetting::newInstance();
	if( NULL == pc_NetworkSetting )
	{
		printf("SystemErr\n");
		return;
	}

	TYPE_MioSettingResult e_Result = TYPE_MioSettingResultOk;

	switch( e_SettingMode )
	{
		case THRE_NwIfMode_SingleEthernet:		// 有線のみ.
			e_Result = pc_NetworkSetting->setNetworkIFModeOnly(TYPE_NetworkIFMode_SingleEthernet);
			break;

		case THRE_NwIfMode_DualEthernetSta:		// 有線+無線（子機モード）.
			e_Result = pc_NetworkSetting->setNetworkIFModeOnly(TYPE_NetworkIFMode_DualEthernetSta);
			break;

		default:								// 範囲外.
			printf("incorrected value\n");
			return;
	}
	printf("setNetworkIFModeOnly=%d\n", e_Result);

	if( TYPE_MioSettingResultOk == e_Result )
	{
		printf("NetworkIFSetting = %d\n", e_SettingMode);
	}
}

// ================================================================================================.
//! <!--機能--> 【ターミナルコマンド】2ndネットワークアダプタ設定の設定内容を確認する.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! 2ndネットワークアダプタ設定の設定内容をターミナル上に表示する.
//! <!--履歴-->.
//	2021/04/15	Sky	OP_BTS-32090 生産ライン用設定コマンド追加対応.
// ================================================================================================.
void Get2ndNetworkAdapterSetting( void )
{
	APIC_SystemInfo* pc_2ndNetworkAdapter = APIC_SystemInfo::newInstance();
	
	if(NULL == pc_2ndNetworkAdapter){
		
		printf("SystemErr\n");
		return;
	}
	
	TYPE_ExNetworkAdapter e_ExNetworkAdapter = pc_2ndNetworkAdapter->getExNetworkAdapter();
	
	printf("2ndNetworkAdapterSetting  = %d\n", e_ExNetworkAdapter );
	
}

// ================================================================================================.
//! <!--機能--> 【ターミナルコマンド】2ndネットワークアダプタ設定の設定内容を変更する.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param TYPE_ExNetworkAdapter e_ExNetworkAdapter 0: 非装着　1: 装着.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! 引数に指定したモードを、2ndネットワークアダプタ設定に設定する。.
//! 引数の指定が無い場合、引数0として動作する。.
//! 引数が範囲外の場合、エラーとする。.
//! <!--履歴-->.
//	2021/04/15	Sky	OP_BTS-32090 生産ライン用設定コマンド追加対応.
// ================================================================================================.
void Set2ndNetworkAdapterSetting( TYPE_ExNetworkAdapter e_set2ndNwAd )
{
	APIC_SystemInfo* pc_2ndNetworkAdapter = APIC_SystemInfo::newInstance();
	
	if(NULL == pc_2ndNetworkAdapter){
		
		printf("SystemErr\n");
		return;
	}
	
	Bool b_set2ndNwAdFlg = True; 
	
	switch( e_set2ndNwAd )
	{
		case THRE_2ndNwAdapter_OFF:		// 非装着(0).
			pc_2ndNetworkAdapter->setExNetworkAdapter(THRE_2ndNwAdapter_OFF);
			break;
		
		case THRE_2ndNwAdapter_ON:		// 装着(1).
			pc_2ndNetworkAdapter->setExNetworkAdapter(THRE_2ndNwAdapter_ON);
			break;

		default:						// 範囲外.
			printf("incorrected value\n");
			b_set2ndNwAdFlg = False;
			return;
	}
	
	if( True == b_set2ndNwAdFlg ){		
		printf("2ndNetworkAdapterSetting = %d\n", e_set2ndNwAd);	// 設定したときのみ、当ログ出力.
	}	
}

/*===============================================================================================.
［機能］	「エンジン側の不揮発データの初期化」"InitializeENGData"コマンド.
-------------------------------------------------------------------------------------------------.
［引数］	なし.
［戻値］	なし.
［詳細］	「エンジン側の不揮発データ初期化 実行API」処理を実行するコマンド.
［履歴］	.
	2019/04/11	AR:1076046 Sky SPI-Flashの一括データコピー(initializeENGDataコマンド作成)対応 新規作成.
================================================================================================*/
void InitializeENGData()
{
	if( checkLineMode() == False ){		// ラインモードであるかの確認.
		// ラインモードではない場合.
		printf("InitializeENGData NG\n");
		return;
	}

	ENGC_PrinterImpColor* pc_PrinterImpColor = ENGC_PrinterImpColor::newInstance();
	if( pc_PrinterImpColor == NULL ){
		printf("InitializeENGData NG : PrinterImpColorErr\n");
		return;
	}

	APIC_PrinterServiceSetting* pc_PrinterServiceSetting = APIC_PrinterServiceSetting::newInstance();
	if( pc_PrinterServiceSetting == NULL ){
		printf("InitializeENGData NG : PrinterServiceSettingErr\n");
		return;
	}

	// 「エンジン側の不揮発データの初期化」処理実行.
	pc_PrinterImpColor->req_ResetOperation( ENGE_ResetIndication_TroubleReset, ENGE_TroubResetOperation_MemorySW, NULL );
	pc_PrinterServiceSetting->setPrinterAdjust(TYPE_PrinterAdjustThinFuserChoice, False);
	pc_PrinterImpColor->req_ResetOperation( ENGE_ResetIndication_SizeMediaErrReset, ENGE_TroubResetOperation_InitialSW, NULL );
	pc_PrinterImpColor->req_ResetOperation( ENGE_ResetIndication_Initalize, ENGE_TroubResetOperation_InitialSW, NULL );
	pc_PrinterImpColor->req_ResetOperation( ENGE_ResetIndication_TotalClear, ENGE_TroubResetOperation_MemorySW, NULL );
	pc_PrinterServiceSetting->setPrinterAdjust( TYPE_PrinterAdjustFuserCSetModeChoice, TYPE_FuserCSetModeChoiceOff );
	
	SYSF_taskDelay( 200 );	//1秒間待ち.
	printf("InitializeENGData OK\n");
	
	return;
}


/*===============================================================================================.
［機能］	「Rinneログ保存停止・ログファイル削除」"StopRinneModeLog"コマンド.
-------------------------------------------------------------------------------------------------.
［引数］	なし.
［戻値］	なし.
［詳細］	「Rinneログ保存停止・ログファイル削除」処理を実行するコマンド.
［履歴］	.
	2022/09/27	Sky 新規作成.
	2022/11/15	Sky 【制御No.34】【Rinne】ラインモード時、「StopRinneModeLog」のコマンドを受け付ける.
================================================================================================*/
extern Bool THRL_rmRinneLogData();
void StopRinneModeLog( void )
{
	APIC_SystemInfo* pc_2ndNetworkAdapter = APIC_SystemInfo::newInstance();
	
	if(NULL == pc_2ndNetworkAdapter) {
		printf("StopRinneModeLog NG\n");
		printf("SystemErr\n");
		return;
	}
	
	// Rinneでない場合は受け付けない.
	if ( TYPE_LineModeVer_2nd != pc_SystemInfo->getCardLineModeVer() ) {
		printf("StopRinneModeLog NG\n");
		printf("Invalid Command \n");
		return;
	}
	
	Bool bl_ret = THRL_rmRinneLogData();
	if( bl_ret == False ){
		printf("StopRinneModeLog NG\n");
		printf("THRL_rmRinneLogData err!!\n");
		return;
	}
	printf("StopRinneModeLog OK\n");
}

// ================================================================================================.
//! <!--機能--> 【ターミナルコマンド】IRのPコールの発生有無を確認する.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! IRのPコールの発生有無の状態をターミナル上に表示する.
//! 0:未発生、 1:発生.
//! <!--履歴-->.
//	2022/10/31	Sky		[IT6_2.3]制御構成変更No.61 EagleでPコールの流出防止.
// ================================================================================================.
void GetIRPCallStatus( void )
{
	APIC_ScannerWarning* pc_ScannerWarning = APIC_ScannerWarning::newInstance();
	if(NULL == pc_ScannerWarning)
	{
		printf("SystemErr\n");
		return;
	}

	Bool b_IRPCallStatus = pc_ScannerWarning->isIRPCallStatus(); 

	if( False == b_IRPCallStatus )
	{
		printf("GetIRPCallStatus = 0\n");
	}
	else
	{
		printf("GetIRPCallStatus = 1\n");
	}
}

// ================================================================================================.
//! <!--機能--> 【ターミナルコマンド】生産コマンド(nvmeシャットダウン).
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! syncを３回実行する.
//! SSDの場合のみ、nvmeシャットダウンを実行する.
//! <!--履歴-->.
//	2023/04/03	Sky		新規作成.
// ================================================================================================.
void FlashStrage( void )
{
	Bool bl_Result = True;
	Bool bl_SSD = False;
	
	// syncを3回実行.
	sync();
	sync();
	sync();

	// ストレージデバイス構成を取得.
	TYPE_StorageConfig uc_StorageType = THRG_GetStorageType();

	switch( uc_StorageType ){
	case TYPE_StorageConfig_MicroSD_8GB:
		printf("**      Storage Type [ microSD 8GB ]\n");
		break;
	case TYPE_StorageConfig_SSD_256GB:
		printf("**      Storage Type [ SSD 256GB ]\n");
		bl_SSD = True;
		break;
	case TYPE_StorageConfig_SSD_1TB:
		printf("**      Storage Type [ SSD 1TB ]\n");
		bl_SSD = True;
		break;
	case TYPE_StorageConfig_MicroSD_16GB:
		printf("**      Storage Type [ microSD 16GB ]\n");
		break;
	default:
		printf("**      Storage Type [ Error(%d) ]\n", uc_StorageType );
		bl_Result = False;
		break;
	}
	
	// SSDの場合のみ、nvmeシャットダウンを実行.
	if( True == bl_SSD )
	{
		printf("FlashStrage reset_controller\n");
		int i_Ret = system("echo 1 > /sys/devices/platform/quartz-iris-sb-0/8d0098000.pcie0/pci0000:00/0000:00:00.0/0000:01:00.0/nvme/nvme0/reset_controller");
		if( i_Ret != 0 )
		{
			bl_Result = False;
		}
	}

	if( True == bl_Result )
	{
		printf("FlashStrage OK\n");
	}
	else
	{
		printf("FlashStrage NG\n");
	}
}

// OP_BTS-40890_タスクモニタ新詳細コード化対応_S.
/*================================================================================================
 * ［機能］ タスク名を圧縮.
 * -----------------------------------------------------------------------------------------------
 * ［引数］char* pc_taskName インプットタスク名
 * ［引数］char* pc_outputtaskName アウトプットタスク名
 * ［戻値］なし.
 * ［詳細］
 * ［作成］
 * ［履歴］2023/4/12 Sky 作成.
*================================================================================================*/
void THRL_Compression_NewDetai_TaskName( char* pc_taskName,  char* pc_outputtaskName )
{
	size_t	taskNamlen		= 0;
	bool	b_compressFlg	= False;
	
	if( (pc_taskName == NULL) || (pc_outputtaskName == NULL) ){
		printf("[%s] Param NULL. \n",__FUNCTION__);
		return;
	}
	
	taskNamlen = strlen( pc_taskName );
	// タスク名が4Byte以下であればそのまま返す.
	if( taskNamlen <= THRD_NEWDETAIL_TASKNAME_LENGTH ){
		b_compressFlg = False;
		strncpy(pc_outputtaskName, pc_taskName, taskNamlen );
	// タスク名が5Byte以上16Byte未満の場合はテーブルに従って戻り値を設定する.
	}else if( taskNamlen < THRD_NEWDETAIL_TASKNAME_MAXLENGTH ){
		b_compressFlg = True;
	}else{
		// タスク名が16Byte以上の場合は15文字として処理する.
		taskNamlen = (THRD_NEWDETAIL_TASKNAME_MAXLENGTH - 1);
		b_compressFlg = True;
	}
	if( b_compressFlg == True ){
		pc_outputtaskName[0] = pc_taskName[ st_ND_TaskNameCompressTbl[(taskNamlen - 1)].uc_Byte_One   ];
		pc_outputtaskName[1] = pc_taskName[ st_ND_TaskNameCompressTbl[(taskNamlen - 1)].uc_Byte_Two   ];
		pc_outputtaskName[2] = pc_taskName[ st_ND_TaskNameCompressTbl[(taskNamlen - 1)].uc_Byte_Three ];
		pc_outputtaskName[3] = pc_taskName[ st_ND_TaskNameCompressTbl[(taskNamlen - 1)].uc_Byte_Four  ];
	}
}
/*================================================================================================
 * ［機能］ CPU使用率、占有率の計算.
 * -----------------------------------------------------------------------------------------------
 * ［引数］
 * ［戻値］使用率/占有率.
 * ［詳細］
 * ［作成］
 * ［履歴］2023/4/12 Sky 作成.
*================================================================================================*/
uchar THRL_getTaskCPUUseRate( uint64_t u64_totalSec, uint64_t u64_useSec )
{
	double		dl_useRate		= 0.0	;//使用率(計算用).
	uchar		uc_retUseRate	= 0		;//使用率(戻り値) 小数点以下切り捨て.

	if( 0 != u64_totalSec){
		// (使用秒数/合計秒数) * 100 = 使用率.
		dl_useRate = ((double)u64_useSec / (double)u64_totalSec) ;
		dl_useRate = dl_useRate * 100 ;

		// 小数点以下切り捨て.
		uc_retUseRate = (uchar)floor(dl_useRate);
	} else {
        THRL_ThLogMsgErr( "THRL_getTaskCPUUseRate() u64_totalSec = 0\n" );
		uc_retUseRate = 100;
	}
	return uc_retUseRate;
}

/*================================================================================================
 * ［機能］ 新詳細コード用のデータを新詳細コードへ変換.
 * -----------------------------------------------------------------------------------------------
 * ［引数］uchar uc_Core.コア.
 * ［引数］THRS_SendDetalMakeInfo st_SendDetalMakeInfo.新詳細コード送信材料データ[CPU].
 * ［戻値］なし
 * ［詳細］全体使用率が95%以上であれば新詳細コードを保存.
 * ［作成］
 * ［履歴］2023/4/12 Sky 作成
*================================================================================================*/
static void THRL_EditInfoTaskMonitorNewDetailCode( uchar					uc_Core					// コア.
												   ,THRS_SendDetalMakeInfo	st_SendDetalMakeInfo	)	// 新詳細コード送信材料データ[CPU].
{
	uint64_t	u64_useSec		= 0;								// 使用秒数.
	ushort		us_NDcode		= st_SendDetalMakeInfo.us_Code;		// 詳細コード.
	uchar		uc_module		= st_SendDetalMakeInfo.uc_module;	// 詳細コード.	
	uchar		uc_AlluseRate	= 0;								// 使用率.
	uchar		uc_TASKuseRate	= 0;								// 使用率.
	ushort		us_PID			= 0;								// PID.
	char		outputTaskName[THRD_NEWDETAIL_TASKNAME_LENGTH + 1];	// タスク名.
    memset( outputTaskName, 0, sizeof(outputTaskName) );//初期化.
	
	// 使用秒数 = (合計秒数 - swapper).
	u64_useSec		= (st_SendDetalMakeInfo.u64_NewDetailTotalTime - st_SendDetalMakeInfo.u64_sw_totalSec) ;
	// 全体のCPU使用率取得.
	uc_AlluseRate	= THRL_getTaskCPUUseRate(	st_SendDetalMakeInfo.u64_NewDetailTotalTime,
												u64_useSec );
	// タスクのCPU使用率取得.
	uc_TASKuseRate	= THRL_getTaskCPUUseRate(	st_SendDetalMakeInfo.u64_NewDetailTotalTime,
												st_SendDetalMakeInfo.u64_tsk_totalSec );

	// 全体のCPU使用率が95%以上の場合のみ新詳細コードを残す.
	if( THRD_CPU_USE_RATE_BORDER <= uc_AlluseRate )
	{
		// タスク名を4Byteに圧縮.
		THRL_Compression_NewDetai_TaskName(  st_SendDetalMakeInfo.c_tsk_taskNam ,outputTaskName);
		// プロセスID.
		us_PID			= (ushort)st_SendDetalMakeInfo.u32_tsk_pID;
		uint64_t ul_NewDetailCode_1 = ( 
											  ( ((uint64_t) uc_module				<< 56)& 0xff00000000000000LL) // [1  ]新詳細コード用モジュール番号.
											| ( ((uint64_t) us_NDcode				<< 40)& 0x00ffff0000000000LL) // [2-3]詳細コード.
											| ( ((uint64_t) uc_AlluseRate			<< 32)& 0x000000ff00000000LL) // [4  ]タスク使用率.
											| ( ((uint64_t) outputTaskName[0]		<< 24)& 0x00000000ff000000LL) // [5  ]タスク名[1].
											| ( ((uint64_t) outputTaskName[1]		<< 16)& 0x0000000000ff0000LL) // [6  ]タスク名[2].
											| ( ((uint64_t) outputTaskName[2]		<<  8)& 0x000000000000ff00LL) // [7  ]タスク名[3].
											| ( ((uint64_t) outputTaskName[3]			 )& 0x00000000000000ffLL) // [8  ]タスク名[4].
									);
		THRL_ThAssertLog2( us_NDcode, ul_NewDetailCode_1 );
	} else {
		printf("[%s] Core[%d] RATE_BORDER > uc_AlluseRate[%d] \n", __FUNCTION__, uc_Core, uc_AlluseRate);
	}
}
/*================================================================================================
 * ［機能］ タスクモニタの新詳細コードを作成.
 * -----------------------------------------------------------------------------------------------
 * ［引数］THRS_SendDetalMakeInfo st_SendDetalMakeInfo.新詳細コード送信材料データ[CPU].
 * ［戻値］なし
 * ［詳細］
 * ［作成］
 * ［履歴］2023/4/12 Sky 作成.
*================================================================================================*/
static void THRL_MakeTaskMonitorNewDetailCode( THRS_SendDetalMakeInfo st_SendDetalMakeInfo[] )
{
	uchar	uc_CoreCnt	= 0;							// コアカウント.
	uchar	uc_CoreNum	= 4;							// コア数.
	// コア0～4 数回して新詳細コードを送信する.
	for( uc_CoreCnt = 0;  uc_CoreCnt < uc_CoreNum ; uc_CoreCnt++ )
	{
		THRL_EditInfoTaskMonitorNewDetailCode(	 uc_CoreCnt							// コア.
												,st_SendDetalMakeInfo[uc_CoreCnt]);	// 新詳細情報.
	}
}
// ================================================================================================.
//! <!--機能--> タスクモニタから新詳細コードの取得.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! タスクモニタから新詳細コードの取得を指示 \n.
//! <!--履歴-->.
//  2023/04/19 Sky	OP_BTS-40890_タスクモニタ新詳細コード化対応.
// ================================================================================================.
void THRL_GetNewDetailCodeTM( void )
{
	THRS_GetTotalingInfo  getTotalingInfo;
	getTotalingInfo.u32_CoreNo = 4;
	
	// 4core集計.
	THRL_ReqTaskMonitorOnSecEvent(THRD_COM_DBG_GET_NEW_DETAIL_CODE, &getTotalingInfo);

	// タスクモニタの新詳細コードを作成.
	THRL_MakeTaskMonitorNewDetailCode( getTotalingInfo.st_DetalMakeInfo );
	
	return;
}

// ================================================================================================.
//! <!--機能--> タスクモニタの集計.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! タスクモニタから新詳細コードの集計開始を指示 \n.
//! <!--履歴-->.
//  2023/04/19 Sky	OP_BTS-40890_タスクモニタ新詳細コード化対応.
// ================================================================================================.
void THRL_GetTotalingDataStart( void )
{
	THRS_GetTotalingInfo  getTotalingInfo;

	THRL_ReqTaskMonitorOnSecEvent(THRD_COM_DBG_GET_TOTALING_DATA_START, &getTotalingInfo);
	return;
}
// OP_BTS-40890_タスクモニタ新詳細コード化対応_E.
