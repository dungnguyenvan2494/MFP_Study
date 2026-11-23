/******************************************************************************
* KonicaMinolta Modificate History
* [ 2017/10/13 Sky S800 QSPI Flash対応 ]
******************************************************************************/
// ================================================================================================.
//! \file THRC_BSPif.cpp スレッド-BSP間インタフェースの実装.
// ================================================================================================.

#ifdef TORNADO		/* VXWORKSで有効 */
//#include "vxWorks.h".
//#include "pciConfigLib.h".
//#include "comCoreTypes.h".
//#include "drv/pci/pciIntLib.h".
#include <Linux/Platform/InitDevice.h>
#include <sys/types.h>
#include <scsi/sg.h>

#endif
#include "system/thr/thrcom/THRC_Threaddef.h"
#include "THRC_BSPif.h"
#include "system/sys/SYSC_SysIf.h"
#include "system/thr/thrInclude/THRC_BSPifCom.h"
#include "system/thr/thrInclude/THRC_ThreadInstance.h"
#include "system/thr/thrInclude/THRC_NVRAMdef.h"
#include "system/thr/thrInclude/THRC_ThreadInstance.h"	// 05/12/25 morikawa 電力制御対応.
#include "system/thr/thrcom/THRC_ThreadDebug.h"
#include "system/thr/PanelHW/THRC_PanelHardCom.h"
#include "system/thr/thrInclude/THRC_PortInstance.h" /* 20060403 ikeda */ /* 入出力ポート インスタンス定義 */
#ifdef COSIM
#include "system/thr/COSIM/THRC_CosimDebug.h"
#include "system/thr/COSIM/THRC_CosimMain.h"
#include "system/thr/COSIM/THRC_CosimSeamlessMacroExt.h"
#endif

#include "system/nvd/nvdg/NVDG_NVWrite.h"	// 08/07/01 Hamada CQ:305796 ダウンロードフラグ修正.
#include "system/thr/thrcom/ThrCommonDrvrIf.h"

#include "ThrPic2007DrvrIf.h"
#if !defined(DEF_MinervaPreEmu) && !defined(DEF_ZeusRevCus)
#include "system/fax/APIC_FaxDevice.h"
#endif
//	2014/04/25  FT_yamaguchi EEPROMデータコピーコマンド 制御構成変更_No.210 EEPROMレス対応.
#include<linux/i2c.h>
#include<linux/i2c-dev.h>
#include "Linux/Platform/Mediaif.h"		// 2014.04.10.	Sky井柳	【Zeus5BK】制御構成変更_No.210 SPI-Flashへのアクセス先変更対応.
#include <sys/mount.h>		// 2018.08.27 SCD2-Sky Kawanishi [EagleH][SecV4][RQ-676] OSS単独更新機能.

/* ↓S73-BSP_DD-28 4.5. */
#include <sys/sysinfo.h> /* for get_nprocs() */
#if defined(DEF_ARMADAXP) || defined(DEF_HELIOS_EMU2) || defined(DEF_Helios_PF)
#include <sys/syscall.h> /* for syscall() */
/* following function prototypes for affinity in SYSC_OSWrapperTaskLinux.cpp */
extern int SYSF_taskNameToPid(char* name);
#ifdef DEF_64BIT_PF		// EagleH_制御構成変更_No.49 swap有効化対応.
extern int32_t setTaskAffinityCgroup(pid_t tid, const char* path, const char *taskname);
#else	// DEF_64BIT_PF.
extern int32_t setTaskAffinityCgroup(pid_t tid, const char* path);
#endif	// DEF_64BIT_PF.
#define	DEF_CGROUP_PATHSIZE	(32)
#endif/* defined(DEF_ARMADAXP) || defined(DEF_HELIOS_EMU2) || defined(DEF_Helios_PF) */
/* ↑S73-BSP_DD-28 4.5. */
#if defined(DEF_Soc1200) && defined(FUM_LINUX)
#include	<Linux/Platform/InitMemoryIt6.h>		//! 2013/11/21 ADS坪井 CQ.381188(Act_68930) サブメモリのファイルメモリ化 新SIM(1G)対応.
#endif	// #if defined(DEF_Soc1200) && defined(FUM_LINUX).

// BSPのダミー関数定義.
#include "Dummy_BSP.h"

#include "system/thr/thrInclude/THRC_IPRomPara.h"
#include "system/sys/APIC_OptionSetting.h"	// 2014/04/10 ADS滕 CQ.384987(Act_72902) ソリューションモデル切替対応.

#include "system/sys/APIC_FatalSignalDetailCode.h"	/// 2016/03/23 山口 CQ.525702.

#include "system/thr/thrInclude/THRC_PowerManager.h"	// 2016/05/17 PS1-ft_kobayashi ZeusSBK 制御構成変更_No.57 電源SubCPUモニタ対応.

//2019/10/08 Sky 【自動測定】起動イベントと時間の取得 start.
#include "system/thr/thrcom/THRC_CoreDumpSetting.h"
//2019/10/08 Sky 【自動測定】起動イベントと時間の取得 end.
// 2022/03/30 生産RINNE対応 Sky start.
#include "iws/include/json/json.h"
#include "iws/include/json/reader.h"
#include "iws/include/json/writer.h"
#include "iws/include/json/value.h"

#include <arpa/inet.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <netinet/in.h>
#include <stdio.h>      
#include <stdlib.h>
#include <string.h> 
#include <sys/types.h>
#include <sys/ioctl.h>
#include <unistd.h>
// 2022/03/30 生産RINNE対応 Sky end.

extern Bool getKernelDeviceStatusDirect( THRE_StoreDirectDeviceIX e_DevIx, char *pc_State );	// 2016/05/17 PS1-ft_kobayashi ZeusSBK 制御構成変更_No.57 電源SubCPUモニタ対応.

//	//	Emu3もSPI-Flashに書き込むのでEEPROM用の定義は消しておく.
//#define		THRD_MachineTypeDevice_EEPROM

// 2018/02/13 Sky内藤 [Eagle][制御構成変更 No.2] ROMバージョン対応.
#define THRD_PMU_PsCpuVersion	"/sys/bus/platform/devices/PMU.0/devices/PWRSUBCPU_VERSION"		// PS-CPUのバージョンを取得するsysfsファイルパス.
#define THRD_PMU_PsCpuCheckSum 	"/sys/bus/platform/devices/PMU.0/devices/PWRSUBCPU_CHECKSUM"	// PS-CPUのチェックサムを取得するsysfsファイルパス.
#define THRD_PsCpuVersionSize	23																// PS-CPUのバージョン文字数(4->23).
#define THRD_PsCpuCheckSumSize	4																// PS-CPUのチェックサム文字数.

#define THRD_LPPPOffset_Version			0x1FFFD0					// LPPP先頭からのオフセット(バージョン情報).
#define THRD_LPPPOffset_CheckSum		0x1FFFFE					// LPPP先頭からのオフセット(チェックサム値).
//#define THRD_LPPP_VersionSize			24							// バージョン情報のサイズ(Byte).
#define THRD_LPPP_VersionSize			23							// バージョン情報のサイズ(Byte).
#define THRD_LPPP_CheckSumSize			2							// チェックサム値のサイズ(Byte).
#define THRD_LPPPName_CopyDevice		"/dev/mtd0"					// LPPP格納デバイス名(SPI-Flash).

extern char *IP_Ver;
extern char *IP_Ver_A800;
extern unsigned char* IPROM_CPSPIC_INT_Ver;
#undef NULL
#define NULL	(0)

#if defined(TORNADO)	// 実機で有効.
	#ifdef ROM_DIV_3	/* NICFUM ROM_DIV */
		extern char *psc_LineModeIPAddr;		// \common\system\sys\sysg\appInit2.cpp に定義されている.
		extern char *psc_LineModeMask;		// \common\system\sys\sysg\appInit2.cpp に定義されている.
		extern char *psc_LineModeGateway;		// \common\system\sys\sysg\appInit2.cpp に定義されている.
	#else
#if defined(COSIM)
#else
		extern char uc_LineModeIPAddr[16];		// \common\nicfum\src\mio_init.c に定義されている.
		extern char uc_LineModeMask[16];		// \common\nicfum\src\mio_init.c に定義されている.
		extern char uc_LineModeGateway[16];		// \common\nicfum\src\mio_init.c に定義されている.
		static char *psc_LineModeIPAddr				= uc_LineModeIPAddr;
		static char *psc_LineModeMask				= uc_LineModeMask;
		static char *psc_LineModeGateway			= uc_LineModeGateway;
#endif
	#endif	/* #ifdef ROM_DIV_3 NICFUM ROM_DIV */
#endif	// #ifdef TORNADO.


#ifdef TORNADO		// VXWORKSで有効.
extern char* psc_ALLOC_BufImage;				/* ＡＳＩＣ１０の画像バッファ */
extern char* psc_ALLOC_ArrayTableSize;		/* ８００ＫＢｙｔｅ アレイ管理テーブル 					*/
extern char* psc_ALLOC_Asic10ArrayTableSize;		/* ８００ＫＢｙｔｅ アレイ管理テーブル 					*/
extern char* psc_ALLOC_FileMemoryPool;				/* メモリの残り全部 符号メモリ領域(ファイルメモリ) */
#ifdef DEF_A3A4COMM
extern char* psc_ALLOC_EncodeTmpBuffer;				/* 暗号化/復号化用のメモリパーティション */
#endif	// #ifdef DEF_A3A4COMM.
extern uint32_t gtMemoryGetMemoryBankSize(THRE_MEMORY_BANK bank);

//s_SYS_ASIC_PCI_RSRC pAsicPciRsrcTbl[PCI_MAX_SLOTS];.
#endif

#if 0	// AsicSim対応.
#ifdef SIMULATOR // 2006/06/22 Fujii(Sky) シミュレーション用のメモリ確保対応.
// シミュレーション用のメインバッファ領域.
static uchar* puc_SimMainBufTop = NULL;
#endif
#else	// AsicSim対応.
#if defined(SIMULATOR)
// ドライバシミュレーションを行う場合.
extern uchar* puc_SimMainBufTop;
#endif
#endif	// AsicSim対応.


static ushort us_DIMMSize[THRE_SPD_DEVICE_MAX] = { 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF };

// 2012/04/17 N.Kato 速度変更対応後のライセンス管理異常を検出しない機能実現対応.
// 製品タイプ検索テーブル.
struct MachineTypeFind {
	THRE_MACHINENO		eMachineNo;
	TYPE_MachineType	eMachineType;
};

// 2014/02/25 yamada 制御構成変更43:NVRAMレス対応.
uchar uc_FlashDownloadingBuf[THRD_FlashSize_SubsetArea];	// ダウンロード実行状態の一括書き込みに使用するバッファ.
static uchar uc_FlashWorkBuf[THRD_FlashSize_Sector];		// SPIフラッシュ制御に使用するワークバッファ(1セクタサイズ).
static uchar uc_FlashVerifyBuf[THRD_FlashSize_Sector];		// べリファイで使用するワークバッファ(1セクタサイズ).

static Bool verifyFlashSubsetData(const void *p_CheckData, uint32_t ul_Offset, uint32_t ul_Size);	// SPI-Flash への書き込みべリファイ関数.
static Bool setFlashDownloadingStatus(uchar uc_Data, uint32_t ul_Offset);					// ダウンロード情報をSPI-Flashへ書き込む関数.
#if defined(TORNADO) && !defined(FUM_LINUX)	/* 実機で有効 */
static double gettimeofday_sec();														// 経過秒取得関数
#endif // defined(TORNADO) && !defined(FUM_LINUX)	/* 実機で有効 */.

// 2018/05/11 橋本裕 [Eagle][制御構成変更 No.33] Subset OtherFW-DL対応 SSDコントローラFW書き換え追加対応.
// 2020/01/29 Sky井柳 [IT6_2.0MR(新SSD対応MR)] SSD追加対応.
// 装着SSDのベンダや種類の定義.
typedef uchar THRE_SSD_KIND;
	#define THRE_SSD_KIND_PHISON	1	// PhisonSSD.
	#define THRE_SSD_KIND_TOSHIBA	2	// ToshibaSSD.
	#define THRE_SSD_KIND_KIOXIA	3	// Kioxia(Toshiba)SSD.
	#define THRE_SSD_KIND_MICRON	4	// MicronSSD.
	#define THRE_SSD_KIND_PHISON96	5	// PhisonSSD 96層.
	#define THRE_SSD_KIND_PHISON112	6	// PhisonSSD 112層.
	#define THRE_SSD_KIND_KIOXIA112 7	// KioxiaSSD 112層.
	#define THRE_SSD_KIND_NONE		0	// 該当なし.

typedef short THRE_SSD_VENDID;
	#define THRE_SSD_VENDID_PHISON	0x1987 	// PhisonSSDベンダID.
	#define THRE_SSD_VENDID_TOSHIBA	0x1179	// ToshibaSSDベンダID.
	#define THRE_SSD_VENDID_KIOXIA	0x1e0f	// Kioxia(Toshiba)SSDベンダID.
	#define THRE_SSD_VENDID_MICRON	0x1344	// MicronSSDベンダID.


typedef short THRE_SSD_DEVID;
	#define 	THRE_SSD_DEVICEID_PHISON1	0x5007	// PhisonSSD 1.
	#define 	THRE_SSD_DEVICEID_PHISON2	0x5008	// PhisonSSD 2.
	#define 	THRE_SSD_DEVICEID_PHISON96	0x5013	// PhisonSSD 96層.
	#define 	THRE_SSD_DEVICEID_KIOXIA	0x0001	// KIOXIA 96層.
	#define 	THRE_SSD_DEVICEID_KIOXIA112	0x000c	// KIOXIA 112層.
	#define 	THRE_SSD_DEVICEID_NONE		0x0000	// 該当なし.

#define THRD_VENDID_SIZE			(4)		// ベンダIDのサイズ.
#define THRD_PREIDSIZE				(2) 	//ベンダIDの接頭語文字列数.
#define THRD_SSD_CONTROLER_VERSION 	"AA2J0Y0-6500-G00-00(00)"
#define THRD_SSDCTLFW_INFO			"nvme -id-ctrl /dev/kmsda | grep 'fr '"	// SSDコントローラFWバージョン取得コマンド.
#define THRD_SSDCTL_VERSIZE			(4) 	// SSDコントローラFWバージョンサイズ.
#define THRD_SSDCTL_HWMAXSIZE		(64)	// SSDコントローラバージョンサイズ.
#define THRD_SSDCTL_KEY_PHISON		"E8FT"	// バージョン情報取得のための文字列(Phison用) 64層.
#define THRD_SSDCTL_KEY_PHISON96	"EDFMQ"	// バージョン情報取得のための文字列(Phison用) 96層.
#define THRD_SSDCTL_KEY_PHISON112	"EDFQG"	// バージョン情報取得のための文字列(Phison用) 112層.
											//  Phisonの場合、本文字列以降の数字(**.*)がバージョンになる.
#define THRD_SSDCTL_KEY_TOSHIBA		"ADGA1"	// バージョン情報取得のための文字列(Toshiba用).
#define THRD_SSDCTL_KEY_KIOXIA		"AEGA1"	// バージョン情報取得のための文字列(Kioxia用).
#define THRD_SSDCTL_KEY_KIOXIA112	"ANGA1"	// バージョン情報取得のための文字列(Kioxia用) 112層.
#define THRD_SSDCTL_KEY_MICRON		"P4MU"	// バージョン情報取得のための文字列(Micron用).
											// Toshiba/Kioxia/Micronの場合、本文字列以降の数字(***)がバージョンになる.
#define THRD_VENDERID_FILE  "/sys/class/nvme/nvme0/device/vendor"   //ベンダID格納先ファイルパス.

#define THRD_DEVICEID_SIZE			(4)		// デバイスIDのサイズ.
#define THRD_DEVICEID_FILE  "/sys/class/nvme/nvme0/device/device"   //デバイスID格納先ファイルパス.

static Bool			bl_TypeChangeFlg = True ;						// マシンタイプ情報取得フラグ.

// OP_BTS-41577 J99-02対応 日立PrintP 2023/7/12 start.
extern void setTOD_Interrupt();		// TOD受信.
// OP_BTS-41577 J99-02対応 日立PrintP 2023/7/12 end.
// ================================================================================================.
//! <!--機能--> パネル初期化.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! パネル初期化部分をsysAsicRegInitから分離 シリアル初期化後に行うため.
//! <!--履歴-->.
//  05/12/13 k_ozawa	 新規作成.
// ================================================================================================.
void THRL_SysAsicRegInit2()
{
#ifndef COSIM
	THRL_ThLogMsgErr( "THRL_SysAsicRegInit2  Exec Start Panel Hard Init\n" );
// PanelChk Start morikawa 05/11/21.
	CNVC_PanelHard*	pc_PanelHard = CNVC_PanelHard::newInstance();	// パネルハード取得.
	pc_PanelHard->init();		// 	パネルハードウェア 初期化.
	THRL_ThLogMsgErr( "CNVC_PanelHard::init Exec \n" );
// PanelChk End.

#if 0
	// スリープ復帰後のパネル初期化でない時のみＵＡＲＴの初期化を行う.
	if( thePowerManagerTask.isPanelSleepEndLCDInitBusy() == False ) {
		THRL_ThLogMsgErr( "Panel UART RTS Low Set\n" );
#ifdef TORNADO	
#endif	// #ifdef TORNADO.
	}
#endif // #if 0.
#endif
}

// ================================================================================================.
//! <!--機能--> Asic 1個目 がPCIバスに接続されれいるか否か判断.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return True: PCI接続 , False: 未検出.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
// 2005/06/27 橋本昌	新規作成.
// 2010/10/28 小倉		シミュレータの時もドライバに問い合わせるように戻す.
// ================================================================================================.
Bool THRF_IsAsic_0Exist()
{
	Bool b_Exist = THRL_Drvrif_IsDeviceExist( THRE_ASIC_0 );
	return b_Exist;
}

// ================================================================================================.
//! <!--機能--> Asic 2個目 がPCIバスに接続されれいるか否か判断.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return True: PCI接続 , False: 未検出.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
// 2005/06/27 橋本昌	新規作成.
// 2010/10/28 小倉		シミュレータの時もドライバに問い合わせるように戻す.
// ================================================================================================.
Bool THRF_IsAsic_1Exist()
{
	Bool b_Exist = THRL_Drvrif_IsDeviceExist( THRE_ASIC_1 );
	return b_Exist;
}

// ================================================================================================.
//! <!--機能--> Asic 3個目 がPCIバスに接続されれいるか否か判断.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return True: PCI接続 , False: 未検出.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
// 2007/04/05 井川	新規作成.
// 2010/10/28 小倉		シミュレータの時もドライバに問い合わせるように戻す.
// ================================================================================================.
Bool THRF_IsAsic_2Exist()
{
	Bool b_Exist = THRL_Drvrif_IsDeviceExist( THRE_ASIC_2 );
	return b_Exist;
}

// ================================================================================================.
//! <!--機能--> Asic 4個目 がPCIバスに接続されているか否か判断.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return True: PCI接続 , False: 未検出.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
// 2013/03/06 Sky	新規作成.
// ================================================================================================.
Bool THRF_IsAsic_3Exist()
{
	Bool b_Exist = THRL_Drvrif_IsDeviceExist( THRE_ASIC_3 );
	return b_Exist;
}

// ================================================================================================.
//! <!--機能--> Asic 5個目 がPCIバスに接続されているか否か判断.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return True: PCI接続 , False: 未検出.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
// 2013/03/06 Sky	新規作成.
// ================================================================================================.
Bool THRF_IsAsic_4Exist()
{
	Bool b_Exist = THRL_Drvrif_IsDeviceExist( THRE_ASIC_4 );
	return b_Exist;
}

#ifdef DEF_PP_Cus
// ================================================================================================.
//! <!--機能--> Asic 6個目 がSPIバスに接続されているか否か判断.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return True: SPI接続 , False: 未検出.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
// 2017/06/06 Sky	新規作成. OP/PP共通化.
// ================================================================================================.
Bool THRF_IsAsic_5Exist()
{
	Bool b_Exist = THRL_Drvrif_IsDeviceExist( THRE_ASIC_5 );
	return b_Exist;
}
// ================================================================================================.
//! <!--機能--> Asic 7個目 が接続されているか否か判断.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return True: 接続 , False: 未検出.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
// 2017/06/06 Sky	新規作成 .接続方式はTBDとなるため、空関数で対応. OP/PP共通化.
// ================================================================================================.
Bool THRF_IsAsic_6Exist()
{
	Bool b_Exist = False;
	return b_Exist;
}
#endif // DEF_PP_Cus.

// ================================================================================================.
//! <!--機能--> AsicPMS がPCIバスに接続されているか否か判断.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return True: PCI接続 , False: 未検出.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
// 2013/03/06 Sky	新規作成.
// 2015/10/06 Sky	システム統合SoC(S800)対応.
// ================================================================================================.
Bool THRF_IsPMSExist()
{
	return False;
}

// ================================================================================================.
//! <!--機能--> A800(画像処理統合Asic) がPCIバスに接続されているか否か判断.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return True: PCI接続 , False: 未検出.
//! <!--詳細-->.
//! A800画像処理統合Asicが装着されている場合にTrueを返す.\n.
//! A800Fieryが装着されている場合は、Falseを返す.\n.
//! <!--履歴-->.
//  2017/04/07 Sky		ZeusSMLKPF対応.
// ================================================================================================.
Bool THRF_IsA800IntegrationExist()
{
	Bool b_Exist = False;

#ifdef DEF_A800
	if( THRE_AsicKind_A800 == THRL_Drvrif_GetAsicKind( THRE_AsicUID_Asic_2 )) {
		b_Exist = True;
	}
#endif

	return b_Exist;
}

// ================================================================================================.		
//! <!--機能--> A800(画像処理統合Asic) がPCIバスに接続されているか否か判断.		
//! <!--------------------------------------------------------------------------------------------->.		
//! <!--引数-->.		
//! \param uchar uc_AsicUID ：ASIC 種別.		
//! <!--戻値-->.		
//! \return True: PCI接続 , False: 未検出.		
//! <!--詳細-->.		
//! A800画像処理統合Asicが装着されている場合にTrueを返す.\n.		
//! A800Fieryが装着されている場合は、Falseを返す.\n.		
//! <!--履歴-->.		
//  2017/04/07 Sky		ZeusSMLKPF対応.
// ================================================================================================.		
Bool THRF_IsA800IntegrationExist(unsigned char uc_asicNo)
{
	Bool b_Exist = False;
#ifdef DEF_PP_Cus
#ifdef DEF_A800
	switch( uc_asicNo){
	case THRE_ASIC_1:
		b_Exist =THRF_IsA800IntegrationExist();
	break;

	//A800 3枚目、4枚目はSPI接続、接続確認仕様はTBD.

	case THRE_ASIC_3: ///A800　2枚目
	if( THRE_AsicKind_A800 == THRL_Drvrif_GetAsicKind( THRE_AsicUID_Asic_4 )) {
		b_Exist = True;
	}
	break;
	}
#endif
#endif // DEF_PP_Cus.
	return b_Exist;
}

// ================================================================================================.
//! <!--機能--> CPSGreat がPCIバスに接続されれいるか否か判断.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return True: PCI接続 , False: 未検出.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
// 2005/06/27 橋本昌	新規作成.
// 2011/08/24 世古		環境構築(リベース_8/2):2011/08 MinervaZeus_int リベースによるSoC化.
// 2010/10/28 小倉		シミュレータの時もドライバに問い合わせるように戻す.
// ================================================================================================.
Bool THRF_IsCpsGreatExist()
{
	Bool b_Exist = THRL_Drvrif_IsDeviceExist( THRE_CPS_0 );
	return b_Exist;
}

// ================================================================================================.
//! <!--機能--> CPSGreat がPCIバスに接続されれいるか否か判断.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return True: PCI接続 , False: 未検出.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
// 2005/06/27 橋本昌	新規作成.
// 2011/08/24 世古		環境構築(リベース_8/2):2011/08 MinervaZeus_int リベースによるSoC化.
// 2010/10/28 小倉		シミュレータの時もドライバに問い合わせるように戻す.
// ================================================================================================.
Bool THRF_IsCpsGreat2Exist()
{
	Bool b_Exist = THRL_Drvrif_IsDeviceExist( THRE_CPS_1 );
	return b_Exist;
}

// ================================================================================================.
//! <!--機能--> PICGreat がPCIバスに接続されれいるか否か判断.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return True: PCI接続 , False: 未検出.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
// 2005/06/27 橋本昌	新規作成.
// 2014/01/31 Sky		ES基板対応 ASIC非装着起動対応.
// ================================================================================================.
Bool THRF_IsPicGreatExist()
{
	Bool b_Exist = False;

	// ES基板検証 ASIC非装着起動対応.
	// ASIC1枚目がない場合は、必ずFalseを返す.
	if( THRL_Drvrif_IsDeviceExist( THRE_ASIC_0 ) == False ) {
		b_Exist = False;
	}
	else {
		b_Exist = THRL_Drvrif_IsDeviceExist( THRE_PIC );
	}

	return b_Exist;
}

// ================================================================================================.
//! <!--機能--> HDD制御チップがPCIバスに接続されれいるか否か判断.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return True: PCI接続 , False: 未検出.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
// 2006/01/23 morikawa 新規作成.
// ================================================================================================.
Bool THRF_IsHDDControlExist()
{
	Bool b_Exist = THRL_Drvrif_IsDeviceExist( THRE_SATA_Controller );
	return b_Exist;
}

// ================================================================================================.
//! <!--機能--> Asic7 がPCIバスに接続されれいるか否か判断.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return True: PCI接続 , False: 未検出.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
// 2005/06/27 橋本昌	新規作成.
// 2015/11/24 Sky  		システム統合SoC(S800)対応.
// 2016/01/22 Sky古市	[ZeusSBK]制御構成変更No.18 機種コード対応.
// 2017/04/07 Sky		ZeusSMLKPF対応.
// ================================================================================================.
Bool THRF_IsAsic7Exist()
{
	// S800、A800どちらもHW JPEGのため固定でTrueを返却.
	Bool b_Exist = True;

	return b_Exist;
}

// 2011/10/13 Gonsho DS基板JPEGコーダ対応 Start.
// ================================================================================================.
//! <!--機能--> DS基板JPEGコーダ がPCIバスに接続されれいるか否か判断.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return True: PCI接続 , False: 未検出.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
// 2011/10/13 Gonsho	新規作成.
// ================================================================================================.
Bool THRF_IsAsic7_1Exist()
{
	Bool b_Exist = THRL_Drvrif_IsDeviceExist( THRE_ASIC7_1 );
	return b_Exist;
}
// 2011/10/13 Gonsho DS基板JPEGコーダ対応 End.

// ================================================================================================.
//! <!--機能--> USB deviceがPCIバスに接続されれいるか否か判断.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return True: PCI接続 , False: 未検出.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
// 2006/05/15 井川 新規作成.
// ================================================================================================.
Bool THRF_IsUSBExist()
{
	Bool b_Exist = THRL_Drvrif_IsDeviceExist( THRE_USB_HOST0 );
	return b_Exist;
}

// ================================================================================================.
//! <!--機能--> USBHOSTがPCIバスに接続されれいるか否か判断.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return True: PCI接続 , False: 未検出.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
// 2006/05/15 井川 新規作成.
// 2010/08/10 飯塚 外部メモリ(USBメモリ) が認識しないのトラブル対策 (GaiaPF 8536).
// ================================================================================================.
Bool THRF_IsUSBHOSTExist()
{
#if !defined(DEF_MinervaPreEmu) && !defined(DEF_ZeusRevCus)
	//	2010.8.10	飯塚	外部メモリ(USBメモリ) が認識しないのトラブル対策.
	//	GaiaPF 8536では、常に有効なのでTrueを返す (USBHOSTはPCIバスには非接続).
	return True ;
#else
	Bool b_Exist = THRL_Drvrif_IsDeviceExist( THRE_USB_HOST0 );
	return b_Exist;
#endif // #if !defined(DEF_MinervaPreEmu) && !defined(DEF_ZeusRevCus).
}

// ================================================================================================.
//! <!--機能--> FAX1がPCIバスに接続されれいるか否か判断.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return True: PCI接続 , False: 未検出.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
// 2006/05/15 井川 新規作成.
// 2010/05/08 小霜 CQ.325288 マージ漏れ対応.
// ================================================================================================.
Bool THRF_IsFAX1Exist()
{
#if !defined(DEF_MinervaPreEmu) && !defined(DEF_ZeusRevCus)

	Bool b_Exist = False;
	FAXE_FaxLineState faxStatus = getFaxLineStatusC(TYPE_CommLine1);
	if (faxStatus == FAXE_LineStateNoDevice ||
	    faxStatus == FAXE_LineStateNotInitialize){
		b_Exist = False;
	} else {
		b_Exist = True;
	}
#else
	Bool b_Exist = THRL_Drvrif_IsDeviceExist( THRE_FAX0 );
#endif // #if !defined(DEF_MinervaPreEmu) && !defined(DEF_ZeusRevCus).
	return b_Exist;
}

// ================================================================================================.
//! <!--機能--> FAX2がPCIバスに接続されれいるか否か判断.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return True: PCI接続 , False: 未検出.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
// 2006/05/15 井川 新規作成.
// 2010/05/08 小霜 CQ.325288 マージ漏れ対応.
// ================================================================================================.
Bool THRF_IsFAX2Exist()
{
#if !defined(DEF_MinervaPreEmu) && !defined(DEF_ZeusRevCus)

	Bool b_Exist = False;
	FAXE_FaxLineState faxStatus = getFaxLineStatusC(TYPE_CommLine2);
	if (faxStatus == FAXE_LineStateNoDevice ||
	    faxStatus == FAXE_LineStateNotInitialize){
		b_Exist = False;
	} else {
		b_Exist = True;
	}
#else
	Bool b_Exist = THRL_Drvrif_IsDeviceExist( THRE_FAX1 );
#endif // #if !defined(DEF_MinervaPreEmu) && !defined(DEF_ZeusRevCus).
	return b_Exist;
}

// ================================================================================================.
//! <!--機能--> FAX3が接続されれいるか否か判断.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return True: 接続 , False: 未検出.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
// 2011/12/19 小霜 新規作成 UV-00877 Fax3or4回線対応.
// ================================================================================================.
Bool THRF_IsFAX3Exist()
{
	// USB FAXボードのみ対応（PCI接続で3、4回線となる場合は別途対応が必要）.
	Bool b_Exist = False;
	FAXE_FaxLineState faxStatus = getFaxLineStatusC(TYPE_CommLine3);
	if (faxStatus == FAXE_LineStateNoDevice ||
	    faxStatus == FAXE_LineStateNotInitialize){
		b_Exist = False;
	} else {
		b_Exist = True;
	}

	return b_Exist;
}
// ================================================================================================.
//! <!--機能--> FAX4が接続されれいるか否か判断.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return True: 接続 , False: 未検出.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
// 2011/12/19 小霜 新規作成 UV-00877 Fax3or4回線対応.
// ================================================================================================.
Bool THRF_IsFAX4Exist()
{
	// USB FAXボードのみ対応（PCI接続で3、4回線となる場合は別途対応が必要）.
	Bool b_Exist = False;
	FAXE_FaxLineState faxStatus = getFaxLineStatusC(TYPE_CommLine4);
	if (faxStatus == FAXE_LineStateNoDevice ||
	    faxStatus == FAXE_LineStateNotInitialize){
		b_Exist = False;
	} else {
		b_Exist = True;
	}

	return b_Exist;
}

// ================================================================================================.
//! <!--機能--> Fieryオプションの有無.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return True あり, False なし.
//! <!--詳細-->.
//! 機種共通で、Asic2枚目が装着 or Asic2枚目非装着かつAsic3枚目がAsic2014なら、FieryはOKとする \n.
//! <!--履歴-->.
//  07/07/17 yokoi	 新規作成.
//  14/09/30 Sky	 Asic2014対応.
//  17/04/07 Sky	ZeusSMLKPF対応.
// ================================================================================================.
Bool THRF_IsFieryOption()
{
	// Asic2枚目がA800装着 or Asic3枚目がAsic2014.
	if( (THRE_AsicKind_A800     == THRL_Drvrif_GetAsicKind( THRE_AsicUID_Asic_2 )) ||
		(THRE_AsicKind_Asic2014 == THRL_Drvrif_GetAsicKind( THRE_AsicUID_Asic_3 )) ){
		return True;
	}

	return False;
}

// ================================================================================================.
//! <!--機能--> DSC1(地紋セキュリティ1)基板の有無.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return True あり, False なし.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
//  2008/07/07 河村 新規作成.
// ================================================================================================.
Bool THRF_IsDsc1Exist()
{
	return CB_BitPort<THRD_Port_DataType>::newInstance( CBBPTN_portSEC_SET1 ) == 1 ? True:False;
}

// ================================================================================================.
//! <!--機能--> DSC2(地紋セキュリティ2)基板の有無.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return True あり, False なし.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
//  2008/07/07 河村 新規作成.
// ================================================================================================.
Bool THRF_IsDsc2Exist()
{
// ポートが仕様削除となったため呼び出し元のモジュール(SCN)が対応するまでの仮対応.
//	return CB_BitPort<THRD_Port_DataType>::newInstance( CBBPTN_portSEC_SET2 ) == 1 ? True:False;
#if defined(DEF_S800)
	return CB_BitPort<THRD_Port_DataType>::newInstance( CBBPTN_portSEC_SET1 ) == 1 ? True:False;
#else
	return False;
#endif
}

// ================================================================================================.
//! <!--機能--> 両面読み込みオプションの有無.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return True あり, False なし.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
//  07/07/17 yokoi	 新規作成.
// ================================================================================================.
Bool THRF_IsExtInDup()
{
	// 現状は仕様外。機種開発において、有効にする？.
	return False;
}

// ================================================================================================.
//! <!--機能--> AsicがFPGAであるか否かのチェック.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return True FPGAである, False FPGAではない.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
//  07/11/22 morikawa	 Donau新規作成.
//  10/09/24 世古		 FPGA基板装着状態は、ioctl経由でドライバから取得する.
//  13/03/06 Sky		 ZeusS対応.
// ================================================================================================.
Bool THRF_IsAsicFpga()
{
	// ZeusSではFPGA基板を開発しないため、デフォルトでFalseを返す.
	return False;
}

// ================================================================================================.
//! <!--機能--> メモリマッピング関連 メインメモリ上のスレッド作業領域取得.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
// 2005/06/30 橋本昌	新規作成.
// 2013/11/21 ADS坪井	CQ.381188(Act_68930)	サブメモリのファイルメモリ化 新SIM(1G)対応.
//												DEF_Soc1200が定義されている場合の1G環境での処理を追加.
// 2014/04/10 ADS滕		CQ.384987(Act_72902)	ソリューションモデル切替対応.
// 2014/04/25 ADS竹内   CQ.389967(Act_73361)    モデル切替の判定条件を変更.
// 2014/12/05 ADS吉田   CQ.506801(Act_504828) AS-FSC_14 A3A4共通化(機能スケーラビリティ拡張).
//                      ⇒機能オプション「TYPE_AppOptionSubMemSplitSave」を以下機能別に分離.
//                        ・TYPE_AppOptionSubMemSplitSave ：サブメモリ可変化[1ページ確保動作]、スワップ分割保存・分割読出.
//                        ・TYPE_AppOptionSubMemPartition ：サブメモリ割り当て切り替え対応.
//                        ・TYPE_AppOptionSubFileMem      ：サブメモリのファイルメモリ化.
//                        ・TYPE_AppOptionWorstSize       ：最悪圧縮率対応.
// ================================================================================================.
char* THRF_GetThreadBufferTopAdrOnMainMemory()
{
#if defined(TORNADO) && !defined(FUM_LINUX)	/* 実機で有効 */
	return (char*)(_roundup( (uintptr_t)psc_ALLOC_BufImage, 64 ) * 64);	/* スレッド作業領域先頭アドレス */
#else	/* #if defined(TORNADO) && !defined(FUM_LINUX) */// シミュレーション環境.
#if defined(SIMULATOR)
#if defined(DEF_Soc1200) && defined(FUM_LINUX)
#if defined(DEF_A3A4COMM)  // 2015/02/16 ADS吉田 CQ.510156(Act_507195) 【A3A4共通化】コンパイルスイッチ対応(機能/メモリスケーラビリティ拡張).
	if( True == SYSF_getAppOptionStatus(TYPE_AppOptionSubFileMem) ){ // 機能判定.
#else
	if( True == SYSF_getAppOptionStatus(TYPE_AppOptionSubMemSplitSave) ){ // 機能判定.
#endif
		char *pc_Ret = THRF_GetFileMemoryPoolTopAdrOnMainMemory();
		pc_Ret += THRL_GetFileMemoryPool();
		return pc_Ret;
	} else {
		return (char*)puc_SimMainBufTop;	// 2006/06/22 Fujii(Sky) シミュレーション用のメモリ確保対応.
	}
#else	// #if defined(DEF_Soc1200) && defined(FUM_LINUX).
	return (char*)puc_SimMainBufTop;	// 2006/06/22 Fujii(Sky) シミュレーション用のメモリ確保対応.
#endif	// #if defined(DEF_Soc1200) && defined(FUM_LINUX).
#else	// #if defined(SIMULATOR).
	return 0;
#endif	// #if defined(SIMULATOR).
#endif /* #if defined(TORNADO) && !defined(FUM_LINUX) */
}

// ================================================================================================.
//! <!--機能--> メモリマッピング関連 ファイルメモリのアレイ管理テーブル先頭アドレス取得.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
// 2005/06/30 橋本昌	新規作成.
// 2013/11/21 ADS坪井	CQ.381188(Act_68930)	サブメモリのファイルメモリ化 新SIM(1G)対応.
//												新SIMの場合この関数内で領域を取得するように変更.
// ================================================================================================.
char* THRF_GetArrayTableAdrForFileMemory()
{
#if defined(TORNADO) && !defined(FUM_LINUX)	/* 実機で有効 */
	return psc_ALLOC_ArrayTableSize;		/* ８００ＫＢｙｔｅ アレイ管理テーブル 					*/
#else	/* #if defined(TORNADO) && !defined(FUM_LINUX) */// シミュレーション環境.
#if defined(DEF_Soc1200) && defined(FUM_LINUX)
	static char* p_TablePtr = NULL;
	if( p_TablePtr == NULL ){
#if defined(DEF_64BIT_PF)
		p_TablePtr = (char *)malloc( 2048 * 1024 );	// 1MB Fileアレイ管理Table・MemoryPartition.
#else
		p_TablePtr = (char *)malloc( 1024 * 1024 ); // 1MB Fileアレイ管理Table・MemoryPartition
#endif
	}
	return p_TablePtr;
#else	// #if defined(DEF_Soc1200) && defined(FUM_LINUX).
	// シミュレーションは使ってはだめ.
	THRL_ThAssert( False, "THRC_BSPif.cpp : getArrayTableAdrForFileMemory Simulator Error!!" , 0, 0, 0, 0, CFA14_DetailCode0x0265 );
	return 0;
#endif	// #if defined(DEF_Soc1200) && defined(FUM_LINUX).
#endif /* #if defined(TORNADO) && !defined(FUM_LINUX) */
}

// ================================================================================================.
//! <!--機能--> メモリマッピング関連 メインメモリ上の符号メモリ(メモリープール)の領域の先頭アドレス取得.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
// 2005/07/01 橋本昌	新規作成.
// 2007/02/04 tsukiji アレイサイズ単位でないと切り下げが発生して若いアドレスへはみ出してしまう不具合対策(memPartAlloc破壊).
//						【対策】先頭メモリアドレスをアレイサイズでアドレスを切り上げておく.
// ================================================================================================.
char* THRF_GetFileMemoryPoolTopAdrOnMainMemory()
{
#if defined(TORNADO) && !defined(FUM_LINUX)	/* 実機で有効 */
	// 07/02/04 tsukiji アレイサイズ単位でないと切り下げが発生して若いアドレスへはみ出してまう不具合対策(memPartAlloc破壊).
	return (char*)(_roundup( (uintptr_t)psc_ALLOC_FileMemoryPool, THRD_ONEARRAYSIZE ) * THRD_ONEARRAYSIZE);				/* メモリの残り全部 符号メモリ領域(ファイルメモリ) */
#else	/* シミュレーション環境 */
	
	return (char*)0x12000000;	/* シミュレーション環境はアドレス 0x12000000 (32MBの位置)を返す データの書き込みは禁止 */
#endif /* #ifdef TORNADO */
}

#ifdef DEF_A3A4COMM
// ================================================================================================.
//! <!--機能--> A3A4共通化FUM 暗号化DMA対応.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return EncodeTmpBufferの先頭アドレス.
//! <!--詳細-->.
//! 暗号化、復号化用の一時バッファ(EncodeTmpBuffer)の先頭アドレスを取得する.
//! <!--履歴-->.
// 2015/02/16 岡部朋(日立JTE)	暗号化DMA対応 新規作成.
// ================================================================================================.
char* THRF_GetEncodeTmpBufferAddr()
{
	return (char*)(_roundup( (uintptr_t)psc_ALLOC_EncodeTmpBuffer, THRD_ONEARRAYSIZE ) * THRD_ONEARRAYSIZE);
}
#endif	// #ifdef DEF_A3A4COMM.

// ================================================================================================.
//! <!--機能--> MFPが外部メモリから起動されたか判定する.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return True: 外部メモリ起動 , False: FlashROM起動.
//! <!--詳細-->.
//! 外部メモリ(USB・CFCard)から起動された場合はTrueを返す.
//! <!--履歴-->.
// 2008/08/20 f-horiuchi CQ307968 新規作成.
// ================================================================================================.
Bool THRL_IsMFPBootType()
{
//	return isMFPBootType_unique();.
	return True;	// 常にUSB起動するように修正.
}	

// ================================================================================================.
// ユーティリティ 関数群.

// PENDING: 以下、とりあえずijc_asic_config.hから移植した。.
//			適切な挿入先を決めるまで、ここに置いておく。(2005/06/10　松永).
// ここから -->.

/*================================================================================================
	外部参照変数の定義
--------------------------------------------------------------------------------------------------
［履歴］2002.2.07		澤柳		参照位置を変更（NVRAMの後半→NVRAM前半・VxWorks予約領域内）
［履歴］2002.3.06		夏目		シミュレータ用に領域定義
		2008.7.01		Hamada		CQ:305796 ダウンロードフラグ修正
================================================================================================*/
/*	ｂｏｏｔＲＯＭ内でも判断する為、ここに定義した	*/
/*ifdef TORNADO */
#if defined(TORNADO) && !defined(COSIM)
/*	VXWORKSで有効	*/
char& bl_MSCDownloading			= *(char*)((uintptr_t)&NVRAM_BASE+0xF8);	// ＭＳＣダウンロード実行状態.
char& bl_ENGDownloading			= *(char*)((uintptr_t)&NVRAM_BASE+0xF9);	// エンジンダウンロード実行状態.
char& bl_IRCDownloading			= *(char*)((uintptr_t)&NVRAM_BASE+0xFA);	// ＩＲＣダウンロード実行状態.
char& bl_EDHDownloading			= *(char*)((uintptr_t)&NVRAM_BASE+0xFB);	// EDHダウンロード実行状態.
char& bl_FAXDownloading			= *(char*)((uintptr_t)&NVRAM_BASE+0xFC);	// ＦＡＸダウンロード実行状態.
char& bl_MSCSubsetDownloading	= *(char*)((uintptr_t)&NVRAM_BASE+0xFD);	// ＭＳＣサブセットダウンロード実行状態.
char& bl_NWDLExec				= *(char*)((uintptr_t)&NVRAM_BASE+0xFF);	// ＮＷＤＬ実行状態.
static char sbl_MCPFDownloading;										// ＭＣＰＦダウンロード実行状態.
char& bl_MCPFDownloading		= sbl_MCPFDownloading;					// ＭＣＰＦダウンロード実行状態.

char& uc_BackupFlag				= *(char*)((uintptr_t)&NVRAM_BASE+0xFFFFF);	// バックアップフラグ	//20060324 k_ozawa NVRAMサイズ変更のため.
#else				/*	ＰＣシミュレータで有効	*/
static char sbl_MSCDownloading;										// ＭＳＣダウンロード実行状態.
static char sbl_ENGDownloading;										// エンジンダウンロード実行状態.
static char sbl_IRCDownloading;										// ＩＲＣダウンロード実行状態.
static char sbl_EDHDownloading;										// EDHダウンロード実行状態.
static char sbl_FAXDownloading;										// ＩＦＡＸフォントダウンロード実行状態.
static char sbl_MSCSubsetDownloading;								// ＭＳＣサブセットダウンロード実行状態.
static char sbl_MCPFDownloading;									// ＭＣＰＦダウンロード実行状態.
static char sbl_NWDLExec;											// ＮＷＤＬ実行状態.
static char sbl_BackupFlag;

char& bl_MSCDownloading	      = sbl_MSCDownloading;					// ＭＳＣダウンロード実行状態.
char& bl_ENGDownloading       = sbl_ENGDownloading;					// エンジンダウンロード実行状態.
char& bl_IRCDownloading       = sbl_IRCDownloading;					// ＩＲＣダウンロード実行状態.
char& bl_EDHDownloading       = sbl_EDHDownloading;					// EDHダウンロード実行状態.
char& bl_FAXDownloading       = sbl_FAXDownloading;					// ＩＦＡＸフォントダウンロード実行状態.
char& bl_MSCSubsetDownloading = sbl_MSCSubsetDownloading;			// ＭＳＣサブセットダウンロード実行状態.
char& bl_MCPFDownloading	  = sbl_MCPFDownloading;
char& bl_NWDLExec             = sbl_NWDLExec;						// ＮＷＤＬ実行状態.
char& uc_BackupFlag			  = sbl_BackupFlag;
#endif/* TORNADO && !COSIM */


#if defined(DEF_Helios_PF)
void clearDownloadingFlag()
{
	// 2014/02/25 yamada 制御構成変更43:NVRAMレス対応.
	/* 注意：下記フラグプリフィックスは"bl_"ですが、型はcharです */
	(void)THRG_SetMSCDownloading(0x00);			// MSCダウンロード実行状態.
	(void)THRG_SetMSCSubsetDownloading(0x00);	// MS1ダウンロード実行状態.
	(void)THRG_SetENGDownloading(0x00);			// ENGダウンロード実行状態.
	(void)THRG_SetEDHDownloading(0x00);			// EDHダウンロード実行状態.
	(void)THRG_SetIRCDownloading(0x00);			// IRCダウンロード実行状態.
	(void)THRG_SetFAXDownloading(0x00);			// FAXダウンロード実行状態.
	(void)THRG_SetMCPFDownloading(0x00);		// MCPダウンロード実行状態.
	(void)THRG_SetNWDLExec(0x00);				// NWDL実行状態.
}
#else	 // defined(DEF_Helios_PF).
void clearDownloadingFlag()
{
	// 08/07/01 Hamada CQ:305796 ダウンロードフラグ修正.
	/* 注意：下記フラグプリフィックスは"bl_"ですが、型はcharです */
	NVDF_memsetNvRam( (void *)&bl_MSCDownloading,       0x00, sizeof( bl_MSCDownloading ) );		// MSCダウンロード実行状態.
	NVDF_memsetNvRam( (void *)&bl_MSCSubsetDownloading, 0x00, sizeof( bl_MSCSubsetDownloading ) );	// MS1ダウンロード実行状態.
	NVDF_memsetNvRam( (void *)&bl_ENGDownloading,       0x00, sizeof( bl_ENGDownloading ) );		// ENGダウンロード実行状態.
	NVDF_memsetNvRam( (void *)&bl_EDHDownloading,       0x00, sizeof( bl_EDHDownloading ) );		// EDHダウンロード実行状態.
	NVDF_memsetNvRam( (void *)&bl_IRCDownloading,       0x00, sizeof( bl_IRCDownloading ) );		// IRCダウンロード実行状態.
	NVDF_memsetNvRam( (void *)&bl_FAXDownloading,       0x00, sizeof( bl_FAXDownloading ) );		// FAXダウンロード実行状態.
	NVDF_memsetNvRam( (void *)&bl_NWDLExec,             0x00, sizeof( bl_NWDLExec));				// NWDL実行状態.
}
#endif // defined(DEF_Helios_PF).

/*================================================================================================
［機能］	ASICのDIMMチェックＳＰＤレジスタ(SPDI)読み出し
--------------------------------------------------------------------------------------------------
［引数］	THRE_SPD_DEVICE_NO SPDに接続されるデバイス番号
［戻値］	ushort 検出容量　
			Bool DIMMタイプ判別結果　True:仕様どおり、False：仕様外　
［ｳｫｰｸｽﾙｰ］	未実施
［履歴］	2007/08/01 yokoi サイズが0の場合、存在しないと言う事なのでFalseを設定するように修正 
			2007/11/08 yokoi Asicが存在しないときもFalseを設定するように修正 
			2013/03/06 Sky ZeusS対応　新規Asicを追加.
================================================================================================*/
ushort THRF_GetDIMMSizeBySPD( THRE_SPD_DEVICE_NO e_SPD_DEVICE_NO, Bool& bl_TypeOK  ) 
{
	bl_TypeOK = False;	// 取得失敗で初期化.
	ushort us_Ans = 0;

	switch( e_SPD_DEVICE_NO ){
		// Asic1枚目.
		case THRE_SPD_DEVICE_0:
		case THRE_SPD_DEVICE_1:
		// Asic2枚目.
		case THRE_SPD_DEVICE_2:
		case THRE_SPD_DEVICE_3:
		// Asic3枚目.
		case THRE_SPD_DEVICE_4:
		case THRE_SPD_DEVICE_5:
		// Asic4枚目.
		case THRE_SPD_DEVICE_6:
		case THRE_SPD_DEVICE_7:
		// Asic5枚目.
		case THRE_SPD_DEVICE_8:
		case THRE_SPD_DEVICE_9:
			if( us_DIMMSize[e_SPD_DEVICE_NO] == 0xFFFF ){
				bl_TypeOK = THRL_Drvrif_GetDIMMSizeBySPD( e_SPD_DEVICE_NO, &us_DIMMSize[e_SPD_DEVICE_NO] );
			}else{
				if( 0 < us_DIMMSize[e_SPD_DEVICE_NO] ){
					bl_TypeOK = True;
				}
			}
			us_Ans = us_DIMMSize[e_SPD_DEVICE_NO];
			break;
		default:
			THRL_ThAssert( False, "THRF_GetDIMMSizeBySPD DevNoErr %d", e_SPD_DEVICE_NO , 0, 0, 0, CFA14_DetailCode0x0266 );
			break;
	}

	return( us_Ans );
}

// ================================================================================================.
//! <!--機能--> 【BSPIF】メインメモリのメモリバンクサイズの取得.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param THRE_MEMORY_BANK bank.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
// 2005/07/04 橋本昌	新規作成.
// ================================================================================================.
uint64_t THRF_GetMainMemoryBankSize( THRE_MEMORY_BANK e_Bank )
{
	register uint64_t ul_BankSize = 0;
	// ドライバ側からメモリサイズを取得.
	ul_BankSize = THRL_Drvrif_GetMainMemoryBankSize( e_Bank );
	return ul_BankSize;
}
// <-- ここまで.

/*================================================================================================
［機能］	【割り込み】PICのTOD割り込み
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	無し
［履歴］	03/04/23 tsukiji 【TSS2】PICのTOD割り込み処理を呼ぶ（統合基板処理）
			03/06/02 morikawa【TSS1】TSS2とPICソフト共通化のため追加
			03/12/03 morikawa【TSS1】実機対応　
			03/12/27 morikawa【TSS1】デバッグ情報設定仕様追加
			04/06/09 ikawa	不要な処理を削除
			08/01/29 井川 TOD Disable処理をMosel Doanu固有処理へ移動
================================================================================================*/
Bool b_IntTODFlg = False;		/* 06/06/06 Hamada プリンタバスチェック対応 */

// ================================================================================================.
//! <!--機能--> ファイルメモリサイズ設定.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param unsigned long memsize	ファイルメモリサイズ.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
// 2006/04/04 太田(慎)	新規作成.
// ================================================================================================.
uint32_t ul_FileMemSize = 0;
void THRL_SetFileMemoryPool( uint32_t memsize )
{
	ul_FileMemSize = memsize;
}

// ================================================================================================.
//! <!--機能--> ファイルメモリサイズ取得.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param EN_SPD_DEVICE_NO en_SPD_DEVICE_NO ...
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
// 2006/04/04 太田(慎)	新規作成.
// 2013/11/21 ADS坪井	CQ.381188(Act_68930)	サブメモリのファイルメモリ化 新SIM(1G)対応.
//												新SIMの場合もDEF_Soc1200が定義されている場合はul_FileMemSizeを返却するように変更.
// ================================================================================================.
uint32_t THRL_GetFileMemoryPool( void )
{
#if defined(TORNADO) && !defined(FUM_LINUX)		// VXWORKSで有効.
	return( ul_FileMemSize );
#else
#if defined(DEF_Soc1200) && defined(FUM_LINUX)
	return( ul_FileMemSize );
#else	// #if defined(DEF_Soc1200) && defined(FUM_LINUX).
	return( 441L*1024*1024 );	/* RAMサイズ 1024MByte */
#endif	// #if defined(DEF_Soc1200) && defined(FUM_LINUX).
#endif
}

// ================================================================================================.
//! <!--機能--> ラインカード検出処理.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! ラインカードを検出し、共有情報(comSystem)にカードラインモードを設定する.\n.
//! chkCFIndex 【BSPのIF】 compactflash.h のinclude 必要 \n.
//! comSystem 共有データ「システム情報」.
//! <!--履歴-->.
// 2006/04/20 橋本昌	新規作成.
// 2006/06/28 R.Suzuki	統合パターンのA4Y/LetterY対応.
// 2007/11/02 R.Suzuki	Donau 改修.
// 2016/04/27 itakura	[CQ528449]A4機向け固定IPアドレス追加対応.
// ================================================================================================.
void THRF_CheckLineCardMode()
{
#if !defined(SIMULATOR) && defined(TORNADO)	&& !defined(COSIM)	// 実機で有効.
	
	if( THRL_Drvrif_IsLineMode() == 1 ){
		if( psc_LineModeIPAddr == NULL || psc_LineModeMask == NULL || psc_LineModeGateway == NULL ){
			THRL_ThLogMsgErr( "## THRF_CheckLineCardMode Error !! ##\n" );
			THRL_ThLogMsgErr( "psc_LineModeIPAddr = %08"PRIxPTR"\n",psc_LineModeIPAddr );
			THRL_ThLogMsgErr( "psc_LineModeMask = %08"PRIxPTR"\n",psc_LineModeMask );
			THRL_ThLogMsgErr( "psc_LineModeGateway = %08"PRIxPTR"\n",psc_LineModeGateway );
			THRL_ThAssert( False, "BSPif THRF_CheckLineCardMode IP %"PRIdPTR", Msk %"PRIdPTR", Gw %"PRIdPTR"", (uintptr_t)psc_LineModeIPAddr, (uintptr_t)psc_LineModeMask, (uintptr_t)psc_LineModeGateway , 0, CFA14_DetailCode0x0268 );
		}

// 2022/03/30 生産RINNE対応 Sky start.
//		strcpy(psc_LineModeIPAddr, "192.168.50.100");
//		strcpy(psc_LineModeMask,    "255.255.255.0");
//		strcpy(psc_LineModeGateway, "192.168.50.1");

		if( TYPE_LineModeVer_2nd == APIC_SystemInfo::newInstance()->getCardLineModeVer() ) {
			TYPS_LineModeInfo	s_LineModeInfo = {};
			APIC_SystemInfo::newInstance()->getCardLineModeInfo( &s_LineModeInfo );
			//有線LANより無線LANを優先する.
			if( ( 0 == strlen(s_LineModeInfo.sc_Wlan_SSID) ) &&
				( 0 != strlen(s_LineModeInfo.sc_Static_IPAddress) ) && 
			    ( 0 != strlen(s_LineModeInfo.sc_Static_Subnetmask) ) && 
				( 0 != strlen(s_LineModeInfo.sc_Static_Gateway) ) ) {
				strncpy(psc_LineModeIPAddr, s_LineModeInfo.sc_Static_IPAddress, 16-1);
				strncpy(psc_LineModeMask,   s_LineModeInfo.sc_Static_Subnetmask, 16-1);
				strncpy(psc_LineModeGateway,s_LineModeInfo.sc_Static_Gateway, 16-1);
			}else if(0 != strlen(s_LineModeInfo.sc_Wlan_SSID)){
				// [Rinneのwlan0の通信異常対策].
				//デフォルトゲートウェイの設定が後勝ちとなるため、一旦先に無効なIPでeth0を設定し、eth0の設定が完了してからwlan0の設定を行う.
				strncpy(psc_LineModeIPAddr, "169.254.0.2", 16-1);
				strncpy(psc_LineModeMask,    "255.255.0.0", 16-1);
				strncpy(psc_LineModeGateway, "169.254.0.1", 16-1);
			}
		} else {
			strncpy(psc_LineModeIPAddr, "192.168.50.100", 16-1);
			strncpy(psc_LineModeMask,    "255.255.255.0", 16-1);
			strncpy(psc_LineModeGateway, "192.168.50.1", 16-1);
		}
// 2022/03/30 生産RINNE対応 Sky end.
	}
#endif	// #if !defined(SIMULATOR) && defined(TORNADO)	&& !defined(COSIM)	// 実機で有効.
}

/*================================================================================================
［機能］	【割り込み】PICのTOD割り込み
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	無し
［履歴］	03/04/23 tsukiji 【TSS2】PICのTOD割り込み処理を呼ぶ（統合基板処理）
			03/06/02 morikawa【TSS1】TSS2とPICソフト共通化のため追加
			03/12/03 morikawa【TSS1】実機対応　
			03/12/27 morikawa【TSS1】デバッグ情報設定仕様追加
			04/06/09 ikawa	不要な処理を削除
			08/01/29 井川 TOD Disable処理をMosel Doanu固有処理へ移動
================================================================================================*/
void	TOD_Interrupt(void)
{
#if defined(COSIM) && defined(THR_SIMWIN) // 【COSIM+ｽﾚｯﾄﾞSIMWIN対応】.
	ddOutPort_IREQ_PIC_setActive();
#else
	ddPort_intTOD();
#endif
	// OP_BTS-41577 J99-02対応 日立PrintP 2023/7/12 start.
	// PrintPへTOD信号受信を通知する.
	setTOD_Interrupt();
	// OP_BTS-41577 J99-02対応 日立PrintP 2023/7/12 end.
	THRL_SetThLog( 237, THRE_DebugLogType_ThCtrLog, 100, True );
}

/*================================================================================================
［機能］	【PICから呼ばれる】PICのTOD割り込み許可
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	無し
［履歴］	03/04/23 tsukiji 【TSS2】PICのTOD割り込み許可を行なう（統合基板処理）
			03/06/02 morikawa【TSS1】TSS2とPICソフト共通化のため追加
			04/06/09 ikawa	不要な処理を削除
			04/09/15 morikawa デバッグ情報設定仕様追加
			08/01/29 井川 TOD Enable処理をMosel Doanu固有処理へ移動
			13/10/22 tadamasa CQ383221のTOD追い越しログ仕込み対応.
================================================================================================*/
extern ushort us_Debug_output_disp;		// 04/09/15 morikawa　debugFlag 5,1の時のみログを出すように修正.
extern THRE_TOD_STATE e_TODState;//BufToExtのTOD追い越し確認用.
void	enableTODInterrupt(void)
{
#if defined(SIMULATOR) && defined(TORNADO)	// 2003/06/27 橋本昌.
	// 評価基板のときは vxGppIntEnable: Invalid cause -1 (0-31) となるので処理しない.
	return;
#endif

#ifdef TORNADO		// VXWORKSで有効.
    THRL_Drvrif_EnableTODInterrupt();

	if( us_Debug_output_disp > 0 ) {
		THRL_ThLogMsgErr("##enableTODInterrupt call\n");
	}	
#endif				// #ifdef TORNADO.
	
	if(e_TODState == THRE_TOD_BufToExtFirst){
		e_TODState = THRE_TOD_None;			//TODの追い越しは起きていない.
	}else{
		THRL_SetTODLog(e_TODState);			//TODの追い越し発生.
		e_TODState = THRE_TOD_None;
	}
	THRL_SetThLog( 238, THRE_DebugLogType_ThCtrLog, 101, True );

#ifdef COSIM
#if defined(COSIM) && defined(THR_SIMWIN) // 【COSIM+ｽﾚｯﾄﾞSIMWIN対応】.
// 【COSIM+ｽﾚｯﾄﾞSIMWIN対応】.
#else
	dbgCosimLog( "$$$$ enableTODInt" );
//	このタイミングでTODを出すと早すぎるので エンジン出力DMA03 起動後(taskCosim_NoticeEngOutDMAStart())に移動.
	extern void taskCosimEnableTodInt();
	taskCosimEnableTodInt();
#endif // #if defined(COSIM) && defined(THR_SIMWIN) // 【COSIM+ｽﾚｯﾄﾞSIMWIN対応】.
#endif

#if defined(SIMULATOR) // SIMWIN, 評価基板.
	// ＳＩＭＷＩＮ､評価基板 ではここで割り込みルーチンをコール.
//	TOD_Interrupt();	// 【割り込み】PICのTOD割り込み.
#endif
}

/*================================================================================================
［機能］	【割り込み】PICのTOD割り込み判定
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	無し
［履歴］	06/06/05 Hamada 新規作成
================================================================================================*/
Bool	checkEnableTODInterrupt(void)
{
    return THRL_Drvrif_CheckEnableTODInterrupt();
}

#if defined( DEF_A3A4ENGSTUB )
// C164 Skip
Bool isSkipC164(){
	int32_t testFd;
	static Bool ret = True;
	static char flg = 0;
	
	if( flg == 1 ){
		return ret;
	}
	
	testFd = open("/boot/NotSkipC164.dat", O_RDONLY );	
	if (-1 != testFd){	
		close(testFd);
		ret = False;
	}else{	
		ret = True;
	}
	flg = 1;
	return ret;
}
#elif defined( DEF_A4_Cus )
// C164 Skip(SparrowEda向け一時対応).
// 2019/01/07 Sky井柳 [Sparrow]AR.1071558 C164 Skip一時対応の解除（C164を検出するように修正）.
//            FC時期であるため、Emu800は、スキップのままとする.
Bool isSkipC164()
{
	//Edaでは機種No.と機種情報を設定できないため、
	//製品タイプとエンジンタイプの整合性チェックでアンマッチが発生し、
	//C164が発生してしまう可能性がある.
	//一時的にTrue(Skip判定)を返すようにする.
#if defined( DEF_S800Emu )
	return True;	// Emu800は、Skipのままとする.
#else // defined( DEF_S800Emu ).
	return False;// C164を検出するように修正.
#endif // defined( DEF_S800Emu ).
}
#else
	//A4機以外.
#endif // defined( DEF_A3A4ENGSTUB )

/*================================================================================================
［機能］	再転送要求.
--------------------------------------------------------------------------------------------------
［引数］	b_surface(表面/裏面).
［戻値］	無し.
［履歴］	16/05/16 Sky		新規作成.
================================================================================================*/	
void reqScanRetransmission( Bool b_surface )
{
	THRL_SetThLog( 680, THRE_DebugLogType_ThCtrLog, 0, False, b_surface );
	//スレッドタスクに再転送要求メッセージを送る.
	sendMessage(TID_BufToFile, TM_ImageReStart, b_surface);
}
		
/*================================================================================================
［機能］	オリジナル製品タイプ取得.
--------------------------------------------------------------------------------------------------
［引数］	無し.
［戻値］	TYPE_MachineType	オリジナルの製品タイプ.
［履歴］	12/04/17 N.Kato 速度変更対応後のライセンス管理異常を検出しない機能実現対応.
			12/04/26 N.Kato ZeusMLK起動時のエラー発生対応.
			12/08/09 t.fukuhara ZeusBK対応.
			12/08/30 Gonsho	静的解析対応 eMachineInfo, eMachineNoの初期化を追加.
			13/04/02 fukuhara MinervaBK機種 速度変更正式対応(コメントの「暫定」を削除).
			14/04/08 下垣 Zeus5BK 制御構成No84 エンジン/マシンタイプ対応.
			14/07/14 山田 ZeusS   制御構成変更No8 マシンタイプ追加対応.
			14/11/04 井口 HeliosL 制御構成変更No5 マシンタイプ追加対応.
			15/01/07 古市 Zeus5BK CQ508775 取得失敗時に再度取得を行うように変更する対応.
			15/02/19 井口 MinervaSBK 制御構成変更_No.17 マシンタイプ追加対応.
			15/06/01 Miyazaki ZeusS_ZX0 制御構成変更_No.32 アンマッチ検出対応.
			15/08/19 h.hashimoto ZeusS_ZX0 制御構成変更_No.7 速度変更対応.
			15/10/01 es_oka VenusMLK 制御構成変更No.5 マシンタイプ変更対応.
			15/10/22 大岡	[ZeusSZX0]制御構成変更No.76-1 新詳細コードデータ収集機能.
			16/01/19 Sky鈴木 ZeusSBK 制御構成変更No.3 機種コード対応.
			16/11/21 Sky鈴木 IT5_4.1 制御構成変更No.3 機種コード対応.
			17/09/14 Sky松井 [Eagle] 制御構成変更No.3 機種コード対応.
			17/11/06 田川	 [Sparrow] 制御構成変更No.3 機種コード対応.
			17/11/20 田川 Sparrow 画出し用_C164回避対応.
			18/01/30 Sky薗﨑 [Eagle] 制御構成変更No.6 マシンタイプ対応 (Emu800対応).
			18/05/08 Sky山岸 [DenebMLK] 制御構成変更No.3 機種コード対応.
			18/06/07 Sky山岸 [DenebMLK] 制御構成変更No.3 機種コード対応 (Emu800対応).
			18/09/06 Sky松井 [Eagle_High] 制御構成変更No.3 Eagle_High_Emu800をテーブルに追加.
			19/04/23 Sky井柳 [EagleBK] 制御構成変更No.3 機種コード対応.
			19/05/13 Sky     [EagleZ+/BKZ+] 制御構成変更No.3 機種コード対応.
			19/07/09 Sky井柳 [SparrowBK/HeliosMLK] 制御構成変更No.3 機種コード対応.
			19/08/26 日立　足立 [HeliosMLK] 制御構成変更_No.48 WW／中国仕向け切り分け対応.
			19/09/09 日立畔柳[SparrowBK]OP_BTS-10868 【開発提起】Emu機のマシンタイプ設定不可.
			19/11/15 Sky井柳 [HeliosMLK] 制御構成変更No.52 製品コンフィグ変更対応.
			21/02/15 Sky [MinervaSSBK] 制御構成変更No.3 機種コード対応.
			22/06/01 Sky [IT6_2.3]制御構成変更_No.6マシンタイプ対応.
================================================================================================*/
#if !defined(DEF_MinervaPreEmu) && !defined(DEF_ZeusRevCus)
TYPE_MachineType	THRF_GetOriginalMachineTypeInfo()
{
	THRE_MACHINEINFO	eMachineInfo = THRE_MACHINEINFO_INVALID;	// 機種情報.
	THRE_MACHINENO		eMachineNo   = THRE_MACHINENO_INVALID;		// 機種No.
	static TYPE_MachineType	eOrgMachineType = TYPE_MachineTypeZeus0;	// 製品タイプ defaultはZeus0.
	static Bool	bGetFlg = False;									// 取得済みフラグ.
	uchar	ucIdx;													// Index.

	// 製品タイプ検索テーブル.
	static const MachineTypeFind	s_EagleFindType[] = {			// Eagle_Low/Hight製品タイプ検索テーブル.
#if defined(DEF_S800Emu) && defined(DEF_EagleCus)
#if defined(DEF_EagleZpCus)
		{ THRE_MACHINENO_Z,			TYPE_MachineTypeEagleZplusEmu800 },		// Eagle_Z+_Emu800.
#else
#if defined(DEF_A800)
		{ THRE_MACHINENO_Z,			TYPE_MachineTypeEagleHEmu800 },		// Eagle_High_Emu800.
#else	// defined(DEF_A800).
		{ THRE_MACHINENO_1,			TYPE_MachineTypeEagleEmu800 },		// Eagle_Low_Emu800.
#endif	// defined(DEF_A800).
#endif	// defined(DEF_EagleZpCus).
#endif	// defined(DEF_S800Emu) && defined(DEF_EagleCus).
		{ THRE_MACHINENO_1,			TYPE_MachineTypeEagle1 },			// Eagle_1.
		{ THRE_MACHINENO_2,			TYPE_MachineTypeEagle2 },			// Eagle_2.
		{ THRE_MACHINENO_2_5,		TYPE_MachineTypeEagle2_5 },			// Eagle_2_5.
		{ THRE_MACHINENO_Z,			TYPE_MachineTypeEagleZ },			// Eagle_Z.
		{ THRE_MACHINENO_X,			TYPE_MachineTypeEagleX },			// Eagle_X.
		{ THRE_MACHINENO_0,			TYPE_MachineTypeEagle0 },			// Eagle_0.
		{ THRE_MACHINENO_ZP,		TYPE_MachineTypeEagleZplus },		// Eagle_Z+.
	};

	static const MachineTypeFind	s_SparrowFindType[] = {			// Sparrow製品タイプ検索テーブル.
		{ THRE_MACHINENO_1,			TYPE_MachineTypeSparrow1MFP },		// SparrowMFP1.
		{ THRE_MACHINENO_2,			TYPE_MachineTypeSparrow2MFP },		// SparrowMFP2.
		{ THRE_MACHINENO_2AIO,		TYPE_MachineTypeSparrow2AIO },		// SparrowAIO2.
		{ THRE_MACHINENO_1SFP,		TYPE_MachineTypeSparrow1SFP },		// SparrowSFP1.
		{ THRE_MACHINENO_2SFP,		TYPE_MachineTypeSparrow2SFP },		// SparrowSFP2.
	};

	static const MachineTypeFind	s_DenebMLKFindType[] = {			// DenebMLK製品タイプ検索テーブル.
#if defined(DEF_S800Emu) && defined(DEF_DenebMLKCus)
		{ THRE_MACHINENO_1,			TYPE_MachineTypeDenebMLKEmu800 },	// Emu800.
#endif
		{ THRE_MACHINENO_1,			TYPE_MachineTypeDenebMLK1 },		// DenebMLK1.
		{ THRE_MACHINENO_2,			TYPE_MachineTypeDenebMLK2 },		// DenebMLK2.
		{ THRE_MACHINENO_3,			TYPE_MachineTypeDenebMLK3 },		// DenebMLK3.
		{ THRE_MACHINENO_4,			TYPE_MachineTypeDenebMLK4 },		// DenebMLK4.
	};

	static const MachineTypeFind	s_EagleBKFindType[] = {			// EagleBK_Low/High製品タイプ検索テーブル.
#if defined(DEF_S800Emu) && defined(DEF_EagleCus)
#if defined(DEF_EagleZpCus)
		{ THRE_MACHINENO_Z,		TYPE_MachineTypeEagleBKZplusEmu800 },		// EagleBK_Z+_Emu800.
#else
#if defined(DEF_A800)
		{ THRE_MACHINENO_Z,		TYPE_MachineTypeEagleBKH_Emu800 },		// EagleBK_High_Emu800.
#else	// defined(DEF_A800).
		{ THRE_MACHINENO_1,		TYPE_MachineTypeEagleBKL_Emu800 },		// EagleBK_Low_Emu800.
#endif	// defined(DEF_A800).
#endif	// defined(DEF_EagleZpCus).
#endif	// defined(DEF_S800Emu) && defined(DEF_EagleCus).
		{ THRE_MACHINENO_Z,			TYPE_MachineTypeEagleBKZ },			// EagleBK_Z.
		{ THRE_MACHINENO_X,			TYPE_MachineTypeEagleBKX },			// EagleBK_X.
		{ THRE_MACHINENO_0,			TYPE_MachineTypeEagleBK0 },			// EagleBK_0.
		{ THRE_MACHINENO_1,			TYPE_MachineTypeEagleBK1 },			// EagleBK_1.
		{ THRE_MACHINENO_2,			TYPE_MachineTypeEagleBK2 },			// EagleBK_2.
		{ THRE_MACHINENO_ZP,		TYPE_MachineTypeEagleBKZplus },		// EagleBK_Z+.
	};

	static const MachineTypeFind	s_SparrowBKFindTypeSpeed[] = {	// 速度変更有効 SparrowBK製品タイプ検索テーブル.
#if defined(DEF_S800Emu) && defined(DEF_SparrowCus)
		{ THRE_MACHINENO_1,			TYPE_MachineTypeSparrowBKEmu800 },	// Emu800.
#endif
#if defined(DEF_SparrowEmu)
		{ THRE_MACHINENO_1,			TYPE_MachineTypeSparrowBKMFPEmu1 },		// SparrowBK 1 MFP Emu.
		{ THRE_MACHINENO_2,			TYPE_MachineTypeSparrowBKMFPEmu2 },		// SparrowBK 2 MFP Emu.
		{ THRE_MACHINENO_1SFP,		TYPE_MachineTypeSparrowBKSFPEmu1 },		// SparrowBK 1 SFP Emu.
		{ THRE_MACHINENO_2SFP,		TYPE_MachineTypeSparrowBKSFPEmu2 },		// SparrowBK 2 SFP Emu.
#endif
		{ THRE_MACHINENO_0,			TYPE_MachineTypeSparrowBK0MFP },	// SparrowBK 0 MFP.
		{ THRE_MACHINENO_1,			TYPE_MachineTypeSparrowBK1MFP },	// SparrowBK 1 MFP.
		{ THRE_MACHINENO_0SFP,		TYPE_MachineTypeSparrowBK0SFP },	// SparrowBK 0 SFP.
	};

	static const MachineTypeFind	s_HeliosMLKFindTypeSpeed[] = {		// 速度変更有効 HeliosMLK製品タイプ検索テーブル.
#if defined(DEF_S800Emu) && defined(DEF_EagleCus)
		{ THRE_MACHINENO_1,			TYPE_MachineTypeHeliosMLKEmu800 },			// Emu800.
#endif
		{ THRE_MACHINENO_2,			TYPE_MachineTypeHeliosMLK2 },				// HeliosMLK 2.
		{ THRE_MACHINENO_2_5,		TYPE_MachineTypeHeliosMLK2_5 },				// HeliosMLK 2.5.
		{ THRE_MACHINENO_3,			TYPE_MachineTypeHeliosMLK3 },				// HeliosMLK 3.
		{ THRE_MACHINENO_2_5_CN,	TYPE_MachineTypeHeliosMLK2_5_CN },			// HeliosMLK 2.5　[中国向け].
		{ THRE_MACHINENO_2_5_Other,	TYPE_MachineTypeHeliosMLK2_5_CNDF },		// HeliosMLK 2.5　[その他向け].
		{ THRE_MACHINENO_3_CN,		TYPE_MachineTypeHeliosMLK3_CN },			// HeliosMLK 3　[中国向け].
		{ THRE_MACHINENO_2_CN,		TYPE_MachineTypeHeliosMLK2_CNDF },			// HeliosMLK 2　[中国向け]　DF標準.
	};

	static const MachineTypeFind	s_MinervaSSBKFindTypeSpeed[] = { // 速度変更有効 MinervaSSBK製品タイプ検索テーブル.
		{ THRE_MACHINENO_X,    TYPE_MachineTypeMinervaSSBKX    },       // MinervaSSBK X.
		{ THRE_MACHINENO_0_US, TYPE_MachineTypeMinervaSSBK0_US },       // MinervaSSBK 0 [US向け].
		{ THRE_MACHINENO_0_EU, TYPE_MachineTypeMinervaSSBK0_EU },       // MinervaSSBK 0 [EU向け].
	};

	const MachineTypeFind	*pFindType = NULL;								// 製品タイプ検索テーブル.
	uchar uc_FindTypeCnt = 0;											// 製品タイプ検索テーブル数.

	if( bGetFlg == True ){
		return eOrgMachineType;
	}

	THRF_getMachineType( (char*)&eMachineInfo, (char*)&eMachineNo );

	// 機種情報ごとに、検索する製品タイプテーブルを切り替える.
	// 2022/06/01 Sky [IT6_2.3]制御構成変更_No.6マシンタイプ対応.
	if( ( eMachineInfo == THRE_MACHINEINFO_EAGLE ) ||
		( eMachineInfo == THRE_MACHINEINFO_EAGLE3 ) ) {				// Eagle/Eagle3.
		pFindType = s_EagleFindType;
		uc_FindTypeCnt = sizeof( s_EagleFindType ) / sizeof( MachineTypeFind );
	} else if( eMachineInfo == THRE_MACHINEINFO_DENEBMLK ) {	// DenebMLK.
		pFindType = s_DenebMLKFindType;
		uc_FindTypeCnt = sizeof( s_DenebMLKFindType ) / sizeof( MachineTypeFind );
	} else if( ( eMachineInfo == THRE_MACHINEINFO_EAGLEBK ) ||
			   ( eMachineInfo == THRE_MACHINEINFO_EAGLEBK3 ) ) {	// EagleBK/EagleBK3.
		pFindType = s_EagleBKFindType;
		uc_FindTypeCnt = sizeof( s_EagleBKFindType ) / sizeof( MachineTypeFind );
	} else if( ( eMachineInfo == THRE_MACHINEINFO_SPARROW ) ||
			   ( eMachineInfo == THRE_MACHINEINFO_SPARROW3 ) ) {	// Sparrow/Sparrow3.
		pFindType = s_SparrowFindType;
		uc_FindTypeCnt = sizeof( s_SparrowFindType ) / sizeof( MachineTypeFind );
	} else if( ( eMachineInfo == THRE_MACHINEINFO_SPARROWBK ) ||
			   ( eMachineInfo == THRE_MACHINEINFO_SPARROWBK3 ) ) {	// SparrowBK/SparrowBK3.
		pFindType = s_SparrowBKFindTypeSpeed;
		uc_FindTypeCnt = sizeof( s_SparrowBKFindTypeSpeed ) / sizeof( MachineTypeFind );
	} else if( eMachineInfo == THRE_MACHINEINFO_HELIOSMLK ) {	// HeliosMLK.
		pFindType = s_HeliosMLKFindTypeSpeed;
		uc_FindTypeCnt = sizeof( s_HeliosMLKFindTypeSpeed ) / sizeof( MachineTypeFind );
#if defined( DEF_A3A4ENGSTUB ) && !defined( DEF_SparrowCus )
	// C164 Skip
	} else if( isSkipC164() == True ) {
		THRL_SetThLog( 239, THRE_DebugLogType_ThCtrLog, 255, False, eMachineInfo, eMachineNo );
		eOrgMachineType = TYPE_MachineTypeZeusS1;
		printf("\n\n######################## C164 Check Skip!!! set MachineType << ZeusS1 >> ########################################\n\n");
//		DebugForever( TYPD_TroubleCode_MSCFwMismatchError );
		return eOrgMachineType;
#endif // defined( DEF_A3A4ENGSTUB )				// 未知の機種情報の場合はFatalを返す.
	} else if( eMachineInfo == THRE_MACHINEINFO_MINERVASSBK ) {	// MinervaSSBK.
		pFindType = s_MinervaSSBKFindTypeSpeed;
		uc_FindTypeCnt = sizeof( s_MinervaSSBKFindTypeSpeed ) / sizeof( MachineTypeFind );
	} else {											// 未知の機種情報の場合はFatalを返す.
		THRL_SetThLog( 239, THRE_DebugLogType_ThCtrLog, 255, False, eMachineInfo, eMachineNo );
		// 新詳細コード保存.
		uint64_t		ull_NewDetailData = THRD_NewDetail_ModuleId_Thread;				// 1,2桁：スレッド.
		ull_NewDetailData |= (uint64_t)( TYPD_TroubleCode_MSCFwMismatchError ) << THRD_NewDetail_TroubleCode_Shift;	// 3-6桁：トラブルコード.
		ull_NewDetailData |= (uint64_t)( eMachineInfo ) << 32;						// 7,8桁：機種情報.
		ull_NewDetailData |= (uint64_t)( eMachineNo ) << 24;							// 9,10桁：機種No.
//		ull_NewDetailData |= (uint64_t)( eMachineSpeed ) << 16;						// 11,12桁：機種Speed.
		ull_NewDetailData |= (uint64_t)( eOrgMachineType ) << 8;						// 13,14桁：機種Type.
		ull_NewDetailData |= (uint64_t)( 0x00 );										// 15,16桁：番号.
		THRL_ThAssertLog2( 0, ull_NewDetailData );
		DebugForever( TYPD_TroubleCode_MSCFwMismatchError );
		return eOrgMachineType;
	}

	// 製品タイプ検索.
	for( ucIdx = 0; ucIdx < uc_FindTypeCnt; ucIdx++ ){
		if( pFindType[ucIdx].eMachineNo == eMachineNo ){
			eOrgMachineType =  pFindType[ucIdx].eMachineType;
			break;
		}
	}
#if defined( DEF_A3A4ENGSTUB ) && !defined( DEF_SparrowCus )
	// C164 Skip
	if( isSkipC164() == False && ucIdx >= uc_FindTypeCnt ){
#else // defined( DEF_A3A4ENGSTUB )
	if( ucIdx >= uc_FindTypeCnt ){
#endif // defined( DEF_A3A4ENGSTUB )
		THRL_SetThLog( 240, THRE_DebugLogType_ThCtrLog, 255, False, eMachineInfo, eMachineNo );
		// 新詳細コード保存.
		uint64_t		ull_NewDetailData = THRD_NewDetail_ModuleId_Thread;				// 1,2桁：スレッド.
		ull_NewDetailData |= (uint64_t)( TYPD_TroubleCode_MSCFwMismatchError ) << THRD_NewDetail_TroubleCode_Shift;	// 3-6桁：トラブルコード.
		ull_NewDetailData |= (uint64_t)( eMachineInfo ) << 32;						// 7,8桁：機種情報.
		ull_NewDetailData |= (uint64_t)( eMachineNo ) << 24;							// 9,10桁：機種No.
//		ull_NewDetailData |= (uint64_t)( eMachineSpeed ) << 16;						// 11,12桁：機種Speed.
		ull_NewDetailData |= (uint64_t)( eOrgMachineType ) << 8;						// 13,14桁：機種Type.
		ull_NewDetailData |= (uint64_t)( 0x01 );										// 15,16桁：番号.
		THRL_ThAssertLog2( 0, ull_NewDetailData );
		// 機種情報に対応する機種Noが見つからなければFatalを返す.
		DebugForever( TYPD_TroubleCode_MSCFwMismatchError );
		return eOrgMachineType; // CQ508775対応 GetFlgのTrue化位置を変更する対応
	}
	
#if defined( DEF_A3A4ENGSTUB ) && !defined( DEF_SparrowCus )
	// C164 Skip
	if( isSkipC164() ){
		eOrgMachineType = TYPE_MachineTypeZeusS1;
		printf("\n\n######################## C164 Check Skip!!! set MachineType << ZeusS1 >> ########################################\n\n");
	}
#endif // defined( DEF_A3A4ENGSTUB )

	bGetFlg = True; // CQ508775対応 GetFlgのTrue化位置を変更する対応

#if defined( DEF_A3A4ENGSTUB ) && !defined( DEF_SparrowCus )
	// C164 Skip
	if( isSkipC164() ){
		eOrgMachineType = TYPE_MachineTypeZeusS1;
		printf("\n\n######################## C164 Check Skip!!! set MachineType << ZeusS1 >> ########################################\n\n");
	}
#endif // defined( DEF_A3A4ENGSTUB )

	return eOrgMachineType;
}
#endif // #if !defined(DEF_MinervaPreEmu) && !defined(DEF_ZeusRevCus).

/*================================================================================================
［機能］	製品タイプ取得.
--------------------------------------------------------------------------------------------------
［引数］	無し.
［戻値］	TYPE_MachineType	製品タイプ.
 [備考]		THRE_MACHINENO_0は指定なし扱いになる.
［履歴］	10/08/11 s-ohba Minerva1st基板対応 新規作成.
			12/03/07 N.Kato ZeusMLK速度変更対応.
			12/04/17 N.Kato 速度変更対応後のライセンス管理異常を検出しない機能実現対応.
			12/08/09 takatsuji ZeusBK対応.
			12/08/30 Gonsho	静的解析対応 eMachineNoの初期化と不要コード削除.
			13/04/02 fukuhara MinervaBK機種 速度変更対応.
			13/04/03 K.Morikawa(VSN) 製品タイプ検索テーブルにZeusXを追加(ARM環境の暫定対応).
			14/04/08 下垣 Zeus5BK 制御構成No84 エンジン/マシンタイプ対応.
			14/07/14 山田 ZeusS   制御構成変更No8 マシンタイプ追加対応.
			14/11/04 井口 HeliosL 制御構成変更No5 マシンタイプ追加対応.
			15/01/07 古市 Zeus5BK CQ508775 取得失敗時に再度取得を行うように変更する対応.
			15/01/13 小倉 ZeusS   CQ.508983 ZeusS 2.5に速度変更するとC164が発生する問題対応.
			15/08/19 h.hashimoto ZeusSZX0 制御構成No7 速度変更対応.
			15/10/01 es_oka VenusMLK 制御構成変更No.5 マシンタイプ変更対応.
			15/10/22 大岡	[ZeusSZX0]制御構成変更No.76-1 新詳細コードデータ収集機能.
			16/02/09 Sky鈴木 [ZeusSBK]制御構成変更No.29 速度変更対応.
			16/11/21 Sky鈴木 IT5_4.1 制御構成変更No.3 機種コード対応.
			17/02/27 Sky鈴木 IT5_4.1 制御構成変更No.11 MinervaMLKで速度可変できない対応.
			17/09/14 Sky松井 [Eagle] 制御構成変更No.3 機種コード対応.
			17/11/06 田川 [Sparrow] 制御構成変更No.3 機種コード対応.
			17/11/20 田川 Sparrow 画出し用_C164回避対応.
			18/01/30 Sky薗﨑 [Eagle] 制御構成変更No.6 マシンタイプ対応 (Emu800対応).
			18/03/29 Sky内藤 [Eagle] AR1042209 Eagle2.5へ速度変更を行うと「C164」が発生する対応.
			                         Eagle2_5の値をTHRE_MACHINENO_2_5→THRE_MACHINENO_3に変更.
			19/04/23 Sky井柳 [EagleBK] 制御構成変更No.3 機種コード対応.
			19/06/12 Sky     [SparrowAIO] OP_BTS-7110【課題1077160別件提起】SparrowAIOにてC-40C5発生.
			                         THRF_getMachineSpeedType()の結果がTHRE_MACHINENO_0の場合も.
			                         bGetFlgをOnにするよう、修正.
			22/06/01 Sky     [IT6_2.3]制御構成変更_No.6マシンタイプ対応.
================================================================================================*/
TYPE_MachineType	THRF_GetMachineTypeInfo()
{
	THRE_MACHINEINFO	eMachineInfo = THRE_MACHINEINFO_INVALID;	// 機種情報.
	THRE_MACHINENO		eMachineNo   = THRE_MACHINENO_INVALID;		// 機種No.
	static TYPE_MachineType	eMachineType = TYPE_MachineTypeZeus0;	// 製品タイプ defaultはZeus0.
	static Bool	bGetFlg = False;									// 取得済みフラグ.
	uchar	ucIdx;													// Index.
// 2012/03/07 N.Kato ZeusMLK速度変更対応.
	THRE_MACHINENO		eSerchNo;									// 検索No.
	THRE_MACHINENO		eMachineSpeed;								// 速度情報.


	// 製品タイプ検索テーブル.
	static const MachineTypeFind	s_EagleFindTypeSpeed[] = {		// 速度変更有効 Eagle_Low製品タイプ検索テーブル.
#if defined(DEF_S800Emu) && defined(DEF_EagleCus)
		{ THRE_MACHINENO_1,			TYPE_MachineTypeEagleEmu800 },			// Emu800.
#endif
		{ THRE_MACHINENO_1,			TYPE_MachineTypeEagle1 },				// Eagle1.
		{ THRE_MACHINENO_2,			TYPE_MachineTypeEagle2 },				// Eagle2.
		{ THRE_MACHINENO_3,			TYPE_MachineTypeEagle2_5 },				// Eagle2_5.
	};
// 2015/10/01 VenusMLK 制御構成変更No.5 マシンタイプ変更対応 #start
#ifdef DEF_A4_Cus
	static const MachineTypeFind	s_SparrowFindTypeSpeed[] = {		// 速度変更有効 Sparrow製品タイプ検索テーブル.
		{ THRE_MACHINENO_1,			TYPE_MachineTypeSparrow1MFP },		// SparrowMFP1.
		{ THRE_MACHINENO_2,			TYPE_MachineTypeSparrow2MFP },		// SparrowMFP2.
		{ THRE_MACHINENO_2AIO,		TYPE_MachineTypeSparrow2AIO },		// SparrowAIO2.
		{ THRE_MACHINENO_1SFP,		TYPE_MachineTypeSparrow1SFP },		// SparrowSFP1.
		{ THRE_MACHINENO_2SFP,		TYPE_MachineTypeSparrow2SFP },		// SparrowSFP2.
	};
#endif
// 2015/10/01 VenusMLK 制御構成変更No.5 マシンタイプ変更対応 #end

	static const MachineTypeFind	s_EagleBKFindTypeSpeed[] = {		// 速度変更有効 EagleBK_Low製品タイプ検索テーブル.
#if defined(DEF_S800Emu) && defined(DEF_EagleCus)
		{ THRE_MACHINENO_1,			TYPE_MachineTypeEagleBKL_Emu800 },			// Emu800.
#endif
		{ THRE_MACHINENO_1,			TYPE_MachineTypeEagleBK1 },				// EagleBK1.
		{ THRE_MACHINENO_2,			TYPE_MachineTypeEagleBK2 },				// EagleBK2.
	};

	const MachineTypeFind	*pFindType = NULL;						// 製品タイプ検索テーブル.
	uchar uc_FindTypeCnt = 0;										// 製品タイプ検索テーブル数.
	
	if( bGetFlg == True ){
		return eMachineType;
	}

	THRF_getMachineType( (char*)&eMachineInfo, (char*)&eMachineNo );

	// ZeusMLK速度変更対応 EEPROMの速度情報を読み込む.
	eMachineSpeed = THRF_getMachineSpeedType();

	// 機種情報ごとに、検索する製品タイプテーブルを切り替える.
	if( eMachineSpeed == THRE_MACHINENO_0 ) { // ZeusMLK速度変更対応. 
		// 速度変更領域の速度情報が無効の場合.

		// 12/04/17 N.Kato 速度変更対応後のライセンス管理異常を検出しない機能実現対応.
		eMachineType = THRF_GetOriginalMachineTypeInfo();

		bGetFlg = True; // OP_BTS-7110 eMachineSpeedがTHRE_MACHINENO_0の場合もTrueにする.

		return eMachineType;

	} else {
		// 2022/06/01 Sky [IT6_2.3]制御構成変更_No.6マシンタイプ対応.
		if ( ( eMachineInfo == THRE_MACHINEINFO_EAGLE ) ||
			 ( eMachineInfo == THRE_MACHINEINFO_EAGLE3 ) ) {			//Eagle/Eagle3.
		pFindType = s_EagleFindTypeSpeed;
		uc_FindTypeCnt = sizeof( s_EagleFindTypeSpeed ) / sizeof( MachineTypeFind );
		} else if( ( eMachineInfo == THRE_MACHINEINFO_EAGLEBK ) ||
				   ( eMachineInfo == THRE_MACHINEINFO_EAGLEBK3 ) ) {	// EagleBK/EagleBK3.
			pFindType = s_EagleBKFindTypeSpeed;
			uc_FindTypeCnt = sizeof( s_EagleBKFindTypeSpeed ) / sizeof( MachineTypeFind );
// 2015/10/01 VenusMLK 制御構成変更No.5 マシンタイプ変更対応 #start
#ifdef DEF_A4_Cus
		} else if( ( eMachineInfo == THRE_MACHINEINFO_SPARROW ) ||
				   ( eMachineInfo == THRE_MACHINEINFO_SPARROW3 ) ) {	// Sparrow/Sparrow3.
			pFindType = s_SparrowFindTypeSpeed;
			uc_FindTypeCnt = sizeof( s_SparrowFindTypeSpeed ) / sizeof( MachineTypeFind );
			eMachineSpeed = eMachineNo;
#endif
// 2015/10/01 VenusMLK 制御構成変更No.5 マシンタイプ変更対応 #end
#if defined( DEF_A3A4ENGSTUB ) && !defined( DEF_SparrowCus )
		// C164 Skip
		} else if( isSkipC164() == True ) {
			THRL_SetThLog( 241, THRE_DebugLogType_ThCtrLog, 255, False, eMachineSpeed, eMachineInfo, eMachineNo );
			eMachineType = TYPE_MachineTypeZeusS1;
			printf("\n\n######################## C164 Check Skip!!! set MachineType << ZeusS1 >> ########################################\n\n");
//			DebugForever( TYPD_TroubleCode_MSCFwMismatchError );
			return eMachineType;
#endif // defined( DEF_A3A4ENGSTUB )
		} else {											// 未知の機種情報の場合はFatalを返す.
			THRL_SetThLog( 241, THRE_DebugLogType_ThCtrLog, 255, False, eMachineSpeed, eMachineInfo, eMachineNo );
			// 新詳細コード保存.
			uint64_t		ull_NewDetailData = THRD_NewDetail_ModuleId_Thread;				// 1,2桁：スレッド.
			ull_NewDetailData |= (uint64_t)( TYPD_TroubleCode_MSCFwMismatchError ) << THRD_NewDetail_TroubleCode_Shift;	// 3-6桁：トラブルコード.
			ull_NewDetailData |= (uint64_t)( eMachineInfo ) << 32;						// 7,8桁：機種情報.
			ull_NewDetailData |= (uint64_t)( eMachineNo ) << 24;							// 9,10桁：機種No.
			ull_NewDetailData |= (uint64_t)( eMachineSpeed ) << 16;						// 11,12桁：機種Speed.
			ull_NewDetailData |= (uint64_t)( eMachineType ) << 8;							// 13,14桁：機種Type.
			ull_NewDetailData |= (uint64_t)( 0x00 );										// 15,16桁：番号.
			THRL_ThAssertLog2( 0, ull_NewDetailData );
			DebugForever( TYPD_TroubleCode_MSCFwMismatchError );
			return eMachineType;
		}
		// 製品タイプ検索を速度情報で行う.
		eSerchNo = eMachineSpeed;
	} // ZeusMLK速度変更対応. 

	
	// 製品タイプ検索.
	for( ucIdx = 0; ucIdx < uc_FindTypeCnt; ucIdx++ ){
		if( pFindType[ucIdx].eMachineNo == eSerchNo ){
			eMachineType =  pFindType[ucIdx].eMachineType;
			break;
		}
	}
#if defined( DEF_A3A4ENGSTUB ) && !defined( DEF_SparrowCus )
	// C164 Skip
	if( isSkipC164() == False && ucIdx >= uc_FindTypeCnt ){
#else // defined( DEF_A3A4ENGSTUB )
	if( ucIdx >= uc_FindTypeCnt ){
#endif // defined( DEF_A3A4ENGSTUB )
		THRL_SetThLog( 242, THRE_DebugLogType_ThCtrLog, 255, False, eMachineSpeed, eMachineInfo, eMachineNo );
		// 新詳細コード保存.
		uint64_t		ull_NewDetailData = THRD_NewDetail_ModuleId_Thread;				// 1,2桁：スレッド.
		ull_NewDetailData |= (uint64_t)( TYPD_TroubleCode_MSCFwMismatchError ) << THRD_NewDetail_TroubleCode_Shift;	// 3-6桁：トラブルコード.
		ull_NewDetailData |= (uint64_t)( eMachineInfo ) << 32;						// 7,8桁：機種情報.
		ull_NewDetailData |= (uint64_t)( eMachineNo ) << 24;							// 9,10桁：機種No.
		ull_NewDetailData |= (uint64_t)( eMachineSpeed ) << 16;						// 11,12桁：機種Speed.
		ull_NewDetailData |= (uint64_t)( eMachineType ) << 8;							// 13,14桁：機種Type.
		ull_NewDetailData |= (uint64_t)( 0x01 );										// 15,16桁：番号.
		THRL_ThAssertLog2( 0, ull_NewDetailData );
		// 機種情報に対応する機種Noが見つからなければFatalを返す.
		DebugForever( TYPD_TroubleCode_MSCFwMismatchError );
		return eMachineType; // CQ508775対応 GetFlgのTrue化位置を変更する対応
	}
#if defined( DEF_A3A4ENGSTUB ) && !defined( DEF_SparrowCus )
	// C164 Skip
	if( isSkipC164() ){
		eMachineType = TYPE_MachineTypeZeusS1;
		printf("\n\n######################## C164 Check Skip!!! set MachineType << ZeusS1 >> ########################################\n\n");
	}
#endif // defined( DEF_A3A4ENGSTUB )
	
	bGetFlg = True; // CCQ508775対応 GetFlgのTrue化位置を変更する対応
#if defined( DEF_A3A4ENGSTUB ) && !defined( DEF_SparrowCus )
	// C164 Skip
	if( isSkipC164() ){
		eMachineType = TYPE_MachineTypeZeusS1;
		printf("\n\n######################## C164 Check Skip!!! set MachineType << ZeusS1 >> ########################################\n\n");
	}
#endif // defined( DEF_A3A4ENGSTUB )

	return eMachineType;
}

/*================================================================================================
［機能］	マシンタイプ情報取得フラグ設定.
--------------------------------------------------------------------------------------------------
［引数］	Bool		True： SPI-FlashからMachineTypeを取得する.
						Fals： e_ModelTypeTempから取得する.
［戻値］	無し.
［履歴］	23/07/11 Sky EagleⅢー1でパフォーマンス(WUT)未達対応.
================================================================================================*/
void THRG_SetModelTypeChangeFlg( Bool bl_ChangeFlg )
{
	bl_TypeChangeFlg = bl_ChangeFlg;
}
/*================================================================================================
［機能］	マシンタイプ情報取得フラグ取得.
--------------------------------------------------------------------------------------------------
［引数］	無し.
［戻値］	Bool		True： SPI-FlashからMachineTypeを取得する.
						Fals： e_ModelTypeTempから取得する.
［履歴］	23/07/11 Sky EagleⅢー1でパフォーマンス(WUT)未達対応.
================================================================================================*/
Bool THRG_GetModelTypeChangeFlg( void )
{
	return bl_TypeChangeFlg;
}
/*================================================================================================
［機能］	識別情報（Model Type）取得.
--------------------------------------------------------------------------------------------------
［引数］	無し.
［戻値］	TYPE_ModelType	識別情報(Model Type).
						TYPE_ModelTypeI_II(0)	Model Type： I/II.
						TYPE_ModelTypeIII(1)	Model Type： III.
［履歴］	22/06/01 Sky [IT6_2.3]制御構成変更_No.6マシンタイプ対応.
			23/07/11 Sky EagleⅢー1でパフォーマンス(WUT)未達対応.
================================================================================================*/
TYPE_ModelType	THRG_GetModelType(void)
{
	THRE_MACHINEINFO		eMachineInfo	= THRE_MACHINEINFO_INVALID;		// 機種情報.
	THRE_MACHINENO			eMachineNo		= THRE_MACHINENO_INVALID;		// 機種No.
	static TYPE_ModelType	eModelType 		= TYPE_ModelTypeI_II	;		// 識別情報(Model Type).

	if( THRG_GetModelTypeChangeFlg() == True )
	{
		// 機種情報取得.
		THRF_getMachineType( (char*)&eMachineInfo, (char*)&eMachineNo );

		// 機種情報を基に識別情報をセット.
		switch(eMachineInfo){
			// 機種情報がModel TypeIIIの対象機種の場合.
			case THRE_MACHINEINFO_EAGLE3:									// Eagle3.
			case THRE_MACHINEINFO_SPARROW3:									// Sparrow3.
			case THRE_MACHINEINFO_EAGLEBK3:									// EagleBK3.
			case THRE_MACHINEINFO_SPARROWBK3:								// SparrowBK3.
				// 識別情報(Model Type)にIII セット.
				eModelType = TYPE_ModelTypeIII;
				break;
			// 機種情報がその他の場合.
			default:
				// 識別情報(Model Type)にI/IIセット.
				eModelType = TYPE_ModelTypeI_II;
				break;
		};
		THRG_SetModelTypeChangeFlg( False );
	}
	// 識別情報返却.
	return(eModelType);
}
// 2022/10/17 OP_BTS-37178 山口 EagleⅢではPCIeのWA処理を行わない.
// PCIe WA処理実行の判定処理.
char THRG_PcieWaExecution(void){
	
	static char sc_pcie_ret = THRE_PCIE_WA_DEFAULT;
	
	if(THRE_PCIE_WA_DEFAULT != sc_pcie_ret){
		return sc_pcie_ret;
	}
	// EagleⅢ/SparrowⅢ/uSD起動の場合はPCIe WA処理を行わない.
	if( TYPE_ModelTypeI_II == THRG_GetModelType() && True == APIC_HddInfo::newInstance()->getHddExist()){
		sc_pcie_ret = THRE_PCIE_WA_CONDUCT;
	} else {
		sc_pcie_ret = THRE_PCIE_WA_NO_CONDUCT;
	}
	return sc_pcie_ret;
}
	
// ---> 2011/07/01 Gonsho 生産前ライン対応のため、追加.
/*================================================================================================
［機能］	機種情報設定関数
			機種情報型名と機種No型名を設定する
--------------------------------------------------------------------------------------------------
［引数］	*psc_Machine	: 機種情報型名
			*psc_Type		: 機種No型名
［戻値］	なし
［履歴］	11/04/11 f.karasaki 機種情報書き込み 新規作成.
			11/05/16 浦崎 中身実装。
			12/03/27 N.Kato Sim起動不具合対応.
			12/07/24 N.Kato 新Sim SW25有効時の起動不具合対応.
			12/08/09 t.fukuhara ZeusBK対応.
			13/04/03 K.Morikawa(VSN)   固定値の機種NoをZeusXに変更(ARM環境の暫定対応).
			13/04/26 K.Morikawa(VSN)   [ArmadaXP エンジン対応]ARM環境の暫定対応を削除し、機種情報と機種番号の取得処理を修正
			14/02/17 K.Morikawa(VSN)   S73_u-boot ターミナルコマンド対応
										S73向けのdefine定義を追加
			14/04/08 下垣 Zeus5BK 制御構成No84 エンジン/マシンタイプ対応.
			2014.04.10.	Sky井柳	【Zeus5BK】制御構成変更_No.210 SPI-Flashへのアクセス先変更対応.
								S73向けのdefine定義を追加.
			2014/06/29	Ogura	制御構成No84 エンジン/マシンタイプ対応.
			2017/09/01	Sky井柳 [Eagle][制御構成変更No10] EEPROM処理削除.
================================================================================================*/
void THRF_getMachineType( char* psc_Machine, char* psc_Type )
{
	if( NULL == psc_Machine ){
		printf( "THRF_getMachineType:machine is NULL NG.\n" );
        return;
	}
	if( NULL == psc_Type ){
		printf( "THRF_getMachineType:type is NULL NG.\n" );
        return;
	}
	
	char sc_TmpBuf[2] = { 0 };
	//	シミュレータでの初期値は、SPIFlashクラス（THRC_SPI_FlashCtrlSim）のコンストラクタで設定されている.
	uchar uc_Ret = THRG_getMachineInfo( 0x50, sc_TmpBuf, 2 );
	
	switch( uc_Ret ) {
		case THRD_GET_SPI_OK:
		case THRD_GET_SPI_OK_AREA1_ERR:
		case THRD_GET_SPI_OK_AREA2_ERR:
			*psc_Machine	= sc_TmpBuf[ 0 ];
			*psc_Type		= sc_TmpBuf[ 1 ];
			break;
		default:
			printf( "THRF_getMachineType:THRG_getMachineInfo NG.\n" );
			break;
	}
}

/*================================================================================================
［機能］	機種情報取得関数.
			機種情報型名と機種No型名の情報を取得する
--------------------------------------------------------------------------------------------------
［引数］	sc_Machine	: 機種情報型名
			sc_Type		: 機種No型名
			bl_LogOut	: ログ表示の有効・無効の指定(デフォルトで有効).
						: True (有効).
						: False(無効).
［戻値］	Bool	True:正常終了、	False:異常終了.
［詳細］	ラインモードで使用するコマンドでは、表示されるログを読み取って処理を行う為、.
　　　　　　指定外のログが表示されると、期待通りの処理がされない場合がある.
　　　　　　ログ表示の有効・無効を指定する引数を追加し、デフォルトを有効に設定する.
［履歴］	11/04/11 f.karasaki 機種情報書き込み 新規作成.
			11/05/16 浦崎 中身実装。
			12/03/27 N.Kato Sim起動不具合対応.
			12/07/24 N.Kato 新Sim SW25有効時の起動不具合対応.
			2014.04.10.	Sky井柳	【Zeus5BK】制御構成変更_No.210 SPI-Flashへのアクセス先変更対応.
			2014/06/29	Ogura	制御構成No84 エンジン/マシンタイプ対応.
			2014/12/17	Iguchi	[HeliosL][制御構成変更_No.29]生産ライン対応 ログ表示の有効・無効を指定する引数を追加.
			2017/09/01	Sky井柳 [Eagle][制御構成変更No10] EEPROM処理削除.
			2020/01/06	Sky吉村(魁)	[HeliosMLK][制御構成変更_No48] 設定成功時、Snapshot再取得処理追加.
			23/07/11 Sky EagleⅢー1でパフォーマンス(WUT)未達対応.
================================================================================================*/
Bool THRF_setMachineType( char sc_Machine, char sc_Type, Bool bl_LogOut )
{
	char sc_TmpBuf[ 2 ] = { 0 };
	sc_TmpBuf[ 0 ] = sc_Machine;
	sc_TmpBuf[ 1 ] = sc_Type;
	
	APIC_SystemInfo* pc_SystemInfo = APIC_SystemInfo::newInstance();
	if( NULL == pc_SystemInfo ){
		if( bl_LogOut == True ){
        	printf( "THRF_setMachineType:pc_SystemInfo is NULL.\n" );
		}
		return False;
	}
	
	// 制御構成変更_No48対応 SPI-Flashの内容を反映させるため、次回起動時にsnapshotを取得する.
	pc_SystemInfo->setWarpInfoForMfpData( True );
	
	uchar uc_Ret = THRG_setMachineInfo( 0x50, sc_TmpBuf, 2 );
	if( uc_Ret != THRD_SET_SPI_OK ){	// 正常終了以外は、FALSE(失敗).
		if( bl_LogOut == True ){
        	printf( "THRF_setMachineType:THRG_setMachineInfo NG.\n" );
		}
		return False;
	}
	// マシンタイプ情報取得フラグをTrueに設定する.
	THRG_SetModelTypeChangeFlg( True );
	return True;
}
// <--- 2011/07/01 Gonsho.

// ================================================================================================.
//! <!--機能--> COSIM 最適化対応版 Ｍｅｍｓｅｔ.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param	void* 	pv_Dst.
//! \param 	int		si_Src.
//! \param 	uint32_t	size.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//!  なし\n.
//! <!--履歴-->.
//  09/10/22 八木  新規作成.
// ================================================================================================.
void* THRF_Memset_Optimization ( void* pv_Dst , int32_t si_Src ,size_t ul_Size )
{
	void* pv_Relust = 0;

// COSIM時のみ、実行時間短縮の為メモリ最適化処理を行う.

#ifdef COSIM
	THRF_SetCOSIM_MemoryOptimize( True );	//メモリ最適化ON.
#endif

	//通常のMemset.
	pv_Relust = memset(pv_Dst, si_Src ,ul_Size);

#ifdef COSIM
	THRF_SetCOSIM_MemoryOptimize( False );	//メモリ最適化OFF.
#endif
	return pv_Relust;
}

// ================================================================================================.
//! <!--機能--> COSIM 最適化対応版 Ｍｅｍｃｐｙ.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param	void* 	pv_Dst.
//! \param 	const void* pv_Src.
//! \param 	uint32_t	size.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//!  なし\n.
//! <!--履歴-->.
//  09/10/22	八木  新規作成.
// ================================================================================================.
void* THRF_Memcpy_Optimization ( void* pv_Dst , const void* pv_Src ,size_t ul_Size )
{

	void* pv_Relust = 0;

// COSIM時のみ、実行時間短縮の為メモリ最適化処理を行う.
#ifdef COSIM
	THRF_SetCOSIM_MemoryOptimize( True );	//メモリ最適化ON.
#endif

	//通常のMemset.
	pv_Relust = memcpy(pv_Dst, pv_Src ,ul_Size);

#ifdef COSIM
	THRF_SetCOSIM_MemoryOptimize( False );	//メモリ最適化OFF.
#endif
	return pv_Relust;
}

// ================================================================================================.
//! <!--機能--> アクセラレータDSの接続状態取得.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param void.
//! <!--戻値-->.
//! \return Bool アクセラレータDS接続時 True / 未接続時 False.
//! <!--詳細-->.
//!  なし\n.
//! <!--履歴-->.
//! 2011/09/05	山下	新規作成 アクセラレータAsicオプション化対応.
//  2014/09/30	Sky		Asic2014対応.
//  2015/10/06	Sky		システム統合SoC(S800)対応.
//  2017/04/07	Sky		ZeusSMLKPF対応.
// ================================================================================================.
Bool THRF_IsAcceleratorDS( void ){

    // A800画像処理統合装着時のみアクセラレータDSとして動作.
	Bool b_Exist = THRF_IsA800IntegrationExist();

	return b_Exist;
}

/*================================================================================================
［機能］	指定された速度情報を速度変更領域へ書込む.
--------------------------------------------------------------------------------------------------
［引数］	THRE_MACHINENO e_Speed
［戻値］	Bool	書き込み結果
［履歴］	2012/03/07 N.Kato ZeusMLK速度変更対応 新規作成.
［履歴］	2012/03/27 N.Kato Sim起動不具合対応.
			2012/07/24 N.Kato 新Sim SW25有効時の起動不具合対応.
			2012/10/02 N.Kato CQ.370932 次回の起動時にsnapshotの取得を行う対応.
			2013/11/22 toda   DipSw25削除対応.
			2014/03/24 K.Morikawa(VSN)  S73_EEPROM読み込みの暫定対応対応
										EEPROMドライバが対応されるまで暫定的に無条件に正常(True)を返す
			2014/05/08 K.Morikawa(VSN)  S73:EEPROM_Kernelドライバ対応
										EEPROMドライバが対応されたので、S73用の暫定対応を外す
			2014.04.10.	Sky井柳	【Zeus5BK】制御構成変更_No.210 SPI-Flashへのアクセス先変更対応.
			2014.07.22	Sky小倉	マシンタイプ対応.
			2014/03/14	Sky鈴木	[CQ525502] 速度変更するとトラブルC-C164が発生することがある.
			2017/09/01	Sky井柳 [Eagle][制御構成変更No10] EEPROM処理削除.
================================================================================================*/
Bool THRF_setMachineSpeedType( THRE_MACHINENO e_Speed )
{
	char sc_TmpBuf = e_Speed;
	Bool b_Ret = True;
	
	if( e_Speed > THRE_MACHINENO_3 ){
		THRL_SetThLog( 244, THRE_DebugLogType_ThCtrLog, 255, False,e_Speed );
		return( False );
	}

	uchar uc_Ret = THRG_setMachineInfo( 0x52, &sc_TmpBuf, 1 );
	if( uc_Ret != THRD_SET_SPI_OK ){	// 正常終了以外は、FALSE(失敗).
        printf( "THRF_setMachineSpeedType:THRG_setMachineInfo NG.\n" );
		return( False );
	}
	// Warp起動時にSPI-Flashの更新が反映されない対策。次回の起動時にsnapshotを取得する.
	APIC_SystemInfo::newInstance()->setWarpInfoForMfpData( True );
	return b_Ret;
}

/*================================================================================================
［機能］	速度変更領域の速度情報を取得する.
--------------------------------------------------------------------------------------------------
［引数］	void
［戻値］	THRE_MACHINENO	速度情報
［履歴］	2012/03/07 N.Kato ZeusMLK速度変更対応 新規作成.
［履歴］	2012/03/27 N.Kato Sim起動不具合対応.
			2012/07/24 N.Kato 新Sim SW25有効時の起動不具合対応.
			2013/11/22 toda   DipSw25削除対応.
			2014/02/17 K.Morikawa(VSN)  S73_u-boot ターミナルコマンド対応
										マシンNoをIoctl経由でKernelから取得する処理に変更
			2014/03/24 K.Morikawa(VSN)  S73_EEPROM読み込みの暫定対応対応
										EEPROMドライバが対応されるまで暫定対応として固定値を返す
			2014/05/08 K.Morikawa(VSN)  S73:EEPROM_Kernelドライバ対応
										EEPROMドライバが対応されたので、S73用の暫定対応を外す
			2014.04.10.	Sky井柳	【Zeus5BK】制御構成変更_No.210 SPI-Flashへのアクセス先変更対応.
			2014.07.22	Sky小倉	マシンタイプ対応.
			2014.10.29	Sky小倉	EEPROMレス対応.
			2015/01/07 古市 Zeus5BK CQ508775 取得失敗時に再度取得を行うように変更する対応.
			2017/09/01 Sky井柳 [Eagle][制御構成変更No10] EEPROM処理削除.
================================================================================================*/
THRE_MACHINENO THRF_getMachineSpeedType( void )
{
	THRE_MACHINENO e_Type = THRE_MACHINENO_0;
	uchar uc_Ret = THRG_getMachineInfo( 0x52, &e_Type, 1 );
	if( ( uc_Ret != THRD_GET_SPI_OK )					// 取得OK.
		 &&( uc_Ret != THRD_GET_SPI_OK_AREA1_ERR )		// バックアップデータ取得OK：1面目にメモリ不整合.
		  &&( uc_Ret != THRD_GET_SPI_OK_AREA2_ERR )	){	// バックアップデータ取得OK：2面目にメモリ不整合.
		printf( "THRF_getMachineSpeedType:THRG_getMachineInfo NG.\n" );
	}
	
	// CQ508775 取得失敗時に再度取得を行うように変更する対応.
	// 速度情報値が範囲外の場合は速度無効を設定
	if ( e_Type > THRE_MACHINENO_3 ){
		THRL_SetThLog( 257, THRE_DebugLogType_ThCtrLog, 255, False,e_Type );
		e_Type = THRE_MACHINENO_0;
	}
    return e_Type;
}

#if defined(DEF_ZeusMLKBoard) || defined(DEF_S800Emu)
// 2016/02/26 河村 - [ZeusSBK]RQ-340 新WLANデバイス対応.
// 2016/06/06 板倉 - [VenusMLK]CQ526976 無線アダプタ UK-212非サポート対応.
#define SYSFS_USB	"/sys/bus/usb/devices"
static struct {
	THRE_WLAN_DEVICE_TYPE  e_device;
	ushort us_vendorId;
	ushort us_productId;
} wlan_support_deivices[] = {
#if !defined(DEF_A4_Cus)
	{ THRE_WLAN_DEVICE_TYPE_UK212, 0x1286, 0x2040 },	// 1286:2040 UK-212 Marvell 99W8782.
#endif
	{ THRE_WLAN_DEVICE_TYPE_UK215, 0x0a5c, 0xbd27 },	// 0a5c:bd27 UK-215 Broadcom BCM43566.
	{ THRE_WLAN_DEVICE_TYPE_UK221, 0x0bda, 0xb812 },	// 0bda:b812 UK-221 Realtek RTL8812.  //2018/05/23 ft_kondo Eagle 制御構成変更No.77-1 無線モジュール対応 UK-221正式名称対応. 
	{ THRE_WLAN_DEVICE_TYPE(0),    0,      0      },	// 終端.
};
// 2016/04/06 Harada ZeusSBK 静的解析対応(ID:32175).
#define DEF_ID_VENDOR	"idVendor"
#define DEF_ID_PRODUCT	"idProduct"

static Bool checkUsbDeviceId( const char *pc_path, ushort us_vendorId, ushort us_productId  )
{
	char c_attrPath[PATH_MAX] = {};	// 2016/04/06 Harada ZeusSBK 静的解析対応(ID:32175).
	char val[5];
	FILE *fp;

	// 2016/04/06 Harada ZeusSBK 静的解析対応(ID:32175).
	if( strlen( pc_path ) + strlen( DEF_ID_VENDOR ) >= sizeof( c_attrPath ) ){
		printf("%s: %d Buffer Over Flow  sizeof( c_attrPath )=%d, strlen( pc_path )=%d, strlen( ID_VENDOR )=%d\n",
			__FUNCTION__, __LINE__, sizeof( c_attrPath ), strlen( pc_path ), strlen( DEF_ID_VENDOR ));
		return False;
	}
	if( strlen( pc_path ) + strlen( DEF_ID_PRODUCT ) >= sizeof( c_attrPath ) ){
		printf("%s: %d Buffer Over Flow  sizeof( c_attrPath )=%d, strlen( pc_path )=%d, strlen( ID_PRODUCT )=%d\n",
			__FUNCTION__, __LINE__, sizeof( c_attrPath ), strlen( pc_path ), strlen( DEF_ID_PRODUCT ));
		return False;
	}

	// idVendorのチェック.
	strcpy( c_attrPath, pc_path );
	strcat( c_attrPath, DEF_ID_VENDOR );	// 2016/04/06 Harada ZeusSBK 静的解析対応(ID:32175).
	
	fp = SYSF_fopen( c_attrPath, "r" );
	if( fp == NULL ) {
		printf( "Unknown attribute path : [%s].\n", c_attrPath );
		return False;
	}
	fread( val, 1, 5, fp );
	SYSF_fclose( fp );
	val[4]='\0';
	if( strtoul( val, NULL, 16) != us_vendorId ) {
		return False;	// Vendor ID不一致.
	}

	// idProductのチェック.
	memset( c_attrPath, '\0', sizeof( c_attrPath ) );	// 2016/04/06 Harada ZeusSBK 静的解析対応(ID:32175).
	strcpy( c_attrPath, pc_path );
	strcat( c_attrPath, DEF_ID_PRODUCT );	// 2016/04/06 Harada ZeusSBK 静的解析対応(ID:32175).

	fp = SYSF_fopen( c_attrPath, "r" );
	if( fp == NULL ) {
		printf( "Unknown attribute path : [%s].\n", c_attrPath );
		return False;
	}
	fread( val, 1, 5, fp );
	SYSF_fclose( fp );
	val[4]='\0';
	if( strtoul( val, NULL, 16) != us_productId ) {
		return False;	// Product ID不一致.
	}
	return True;	// 指定デバイスである.
}

static THRE_WLAN_DEVICE_TYPE e_wlan_device = THRE_WLAN_DEVICE_TYPE_INITIALIZATION;

THRE_WLAN_DEVICE_TYPE THRF_getWLANDeviceType( void )
{
	// 2016/04/15 Harada CQ.527019、CQ.527061 WLANデバイスが初期化(未確定)の場合は確定させる.
	if( e_wlan_device == THRE_WLAN_DEVICE_TYPE_INITIALIZATION ) {
		THRF_fixWLANDevice();
	}
	return e_wlan_device;
}

#define FAX_HUB_KIT_PATH	"/sys/bus/usb/devices/3-1.1.1/"
#define DEF_DEVICE_CLASS	"bDeviceClass"
#define FAXHUB_IDVENDOR		0x0409
#define FAXHUB_IDPRODUCT	0x005a
#define FAX_IDVENDOR		0x132b
#define FAX_IDPRODUCT		0x20b7
#define HUB_DEVICECLASS		"09"
/*================================================================================================
［機能］	FAX拡張基板の検出状態確認.
--------------------------------------------------------------------------------------------------
［引数］	なし.
［戻値］	Bool True：検出できた、False：検出無し.
［詳細］	FAX拡張基板の接続を確認する.
［履歴］	2023/01/19 Sky 初版.
================================================================================================*/
#if defined(SIMULATOR) || !defined(DEF_S800)	// Sim/Emu機 対象外.
#elif ( defined(DEF_EagleCus) && !defined(DEF_HeliosMLKCus) ) || defined(DEF_MinervaSSBKCus)
// EagleL, EagleH, EagleLBK, EagleHBK  又は  MinervaSSBK.
static Bool isFAXHubExist()
{
	char buf[PATH_MAX] = {0} ;
	char cp_attrPath[PATH_MAX] = {0};
	char val[3] = {0};
	Bool bl_SearchResult = False;
	static Bool bl_First = True;	// 初回アクセスフラグ初期化（True:初回アクセス False:2回目以降アクセス）.
	static Bool bl_Result = False;	// FAX拡張基板接続判定結果初期化（True:接続 False:未接続）.

	if( !bl_First ){	// 2回目以降は、検索せずにFAX拡張基板接続判定結果を返す.
		return bl_Result;
	}

	snprintf( buf, sizeof(buf), "%s", FAX_HUB_KIT_PATH );
	
	// 指定デバイスファイルがFAX拡張基板かチェック.
	if ( checkUsbDeviceId( buf, FAXHUB_IDVENDOR, FAXHUB_IDPRODUCT ) == True ) {
		snprintf( cp_attrPath, sizeof(cp_attrPath), "%s%s", buf, DEF_DEVICE_CLASS );
		FILE *fp = SYSF_fopen( cp_attrPath, "r" );
		if( fp == NULL ) {
			printf( "(%010u)(BSP) unknown attribute pass : [%s].\n", SYSF_tickGet() , cp_attrPath );
			return False;
		}
		fread( val, 1, 3, fp );
		SYSF_fclose( fp );
		val[2]='\0';
		if( strncmp( val, HUB_DEVICECLASS, 2 ) == 0 ){
			bl_SearchResult = True;	// FAX拡張基板であるのでTrue.
		}
	}
	// FAX拡張基板の検索結果チェック.
	if( bl_SearchResult ){
		bl_Result = True;	// FAX拡張基板接続と判断しTrue（接続）を返す.
	}else{
		bl_Result = False;	// False（未接続）を返す.
	}

	bl_First = False;	// 初回アクセスフラグクリア.
	return bl_Result;
}
#endif // ( defined(DEF_EagleCus) && !defined(DEF_HeliosMLKCus) ) || defined(DEF_MinervaSSBKCus).

#if defined(SIMULATOR) || !defined(DEF_S800)	// Sim/Emu機 対象外.
#elif ( defined(DEF_EagleCus) && !defined(DEF_HeliosMLKCus) ) || defined(DEF_MinervaSSBKCus)
// EagleL, EagleH, EagleLBK, EagleHBK  又は  MinervaSSBK.
#define SYSFS_USB_FAX_HUB_PATH	"/sys/bus/usb/devices/3-1.1.1.%d/"
#define SYSFS_USB_FAX_PATH		"/sys/bus/usb/devices/3-1.1.%d/"
#elif defined(DEF_SparrowCus) || defined(DEF_HeliosMLKCus) || defined(DEF_DenebMLKCus)
// Sparrow, HeliosMLK, DenebMLK.
#define SYSFS_USB_FAX_PATH		"/sys/bus/usb/devices/3-1.4/"
#endif
/*================================================================================================
［機能］	FAXボードの検出状態確認.
--------------------------------------------------------------------------------------------------
［引数］	unsigned char chNo FAX番号.
［戻値］	Bool True：検出できた、False：検出無し.
［詳細］	起動時FAXデバイスの認識に十分な時間を置いて呼び出す。どのデバイスが接続されているか？もしくはデバイスが.
			接続されていないか？を確定する。SYSC_SystemInitから呼び出される.
［履歴］	2023/01/12 Sky 初版.
================================================================================================*/
Bool THRF_fixFAXDevice( unsigned char chNo )
{
#if defined(SIMULATOR) || !defined(DEF_S800)	// Sim/Emu機 対象外.
// 非サポート.
	SYSF_logMsg( "%ld : %s Unsupported Machine Type.\n", SYSF_tickGet(), __FUNCTION__ );
	return False;
#elif ( defined(DEF_EagleCus) && !defined(DEF_HeliosMLKCus) ) || defined(DEF_MinervaSSBKCus)
// EagleL, EagleH, EagleLBK, EagleHBK  又は  MinervaSSBK.
	char buf[PATH_MAX] = {0} ;
	// 接続する FAX ボードは拡張ボードがある場合は4枚、無い場合は2枚.
	// FAX HUB（拡張ボード）の有無チェック.
	if( isFAXHubExist() == True ){
		// 拡張ボードがある場合は 4th バス.
		switch ( chNo ){
			case 1:
				snprintf( buf, sizeof(buf), SYSFS_USB_FAX_HUB_PATH, 3 ) ;
				break;
			case 2:
				snprintf( buf, sizeof(buf), SYSFS_USB_FAX_HUB_PATH, 4 ) ;
				break;
			case 3:
				snprintf( buf, sizeof(buf), SYSFS_USB_FAX_HUB_PATH, 1 ) ;
				break;
			case 4:
				snprintf( buf, sizeof(buf), SYSFS_USB_FAX_HUB_PATH, 2 ) ;
				break;
			default:
				SYSF_logMsg( "%ld : %s Unsupported Fax device :%d\n", SYSF_tickGet(), __FUNCTION__, (int32_t)chNo );
				return False ;
				break;
		}
	}else{
		// chNo が 2 より大きい場合は無効.
		if( chNo > 2 ){
			SYSF_logMsg( "%ld : %s Unsupported Fax device :%d\n", SYSF_tickGet(), __FUNCTION__, (int32_t)chNo );
			return False ;
		}
		// 拡張ボードではない場合は 3rd バス.
		snprintf( buf, sizeof(buf), SYSFS_USB_FAX_PATH, (int32_t)chNo ) ;
	}
	// 指定デバイスファイルがFAX基板かチェック.
	if( checkUsbDeviceId( buf, FAX_IDVENDOR, FAX_IDPRODUCT ) ) {
		SYSF_logMsg( "%ld : %s Detection Fax device, path:%s\n", SYSF_tickGet(), __FUNCTION__, buf );
		return True;
	}
	SYSF_logMsg( "%ld : %s Fax device was not detected.\n", SYSF_tickGet(), __FUNCTION__ );
	return False ;
#elif defined(DEF_SparrowCus) || defined(DEF_HeliosMLKCus) || defined(DEF_DenebMLKCus)
// Sparrow, HeliosMLK, DenebMLK.
	char buf[PATH_MAX] = {0} ;
	// 接続する FAX ボードは1枚のみ.
	if( chNo == 1 ){
		snprintf( buf, sizeof(buf), "%s", SYSFS_USB_FAX_PATH ) ;
		// 指定デバイスファイルがFAX基板かチェック.
		if( checkUsbDeviceId( buf, FAX_IDVENDOR, FAX_IDPRODUCT ) ) {
			SYSF_logMsg( "%ld : %s Detection Fax device, path:%s\n", SYSF_tickGet(), __FUNCTION__, buf );
			return True;
		}
		SYSF_logMsg( "%ld : %s Fax device was not detected.\n", SYSF_tickGet(), __FUNCTION__ );
		return False ;
	}
	SYSF_logMsg( "%ld : %s Unsupported Fax device :%d\n", SYSF_tickGet(), __FUNCTION__, (int32_t)chNo );
	return False ;
#else
// 未定義機種.
	SYSF_logMsg( "%ld : %s Unknown Machine Type.\n", SYSF_tickGet(), __FUNCTION__ );
	return False;
#endif
}

/*================================================================================================
［機能］	WLANデバイスの検出状態確定.
--------------------------------------------------------------------------------------------------
［引数］	なし.
［戻値］	なし.
［詳細］	起動時WLANデバイスの認識に十分な時間を置いて呼び出す。どのデバイスが接続されているか？もしくはデバイスが.
			接続されていないか？を確定する。SYSC_SystemInitから呼び出される.
［履歴］	2016/02/26 河村 初版
================================================================================================*/
void THRF_fixWLANDevice( void )
{
	DIR *dp;
	struct dirent dir;
	struct dirent *dir_Result;
	char *name;
	char buf[PATH_MAX];
	int i_Result = 0;
	int i;
	
	if(( dp = SYSF_opendir( SYSFS_USB )) == NULL ){	// /sys/bus/usb/devicesをオープン.
		printf( "can`t open sysfs usb.\n" );
		return;	// sysfsが存在しない？.
	}
	// sysfsをスキャン.
	while((( i_Result = readdir_r( dp, &dir, &dir_Result )) == 0) && dir_Result != NULL){
		name = &dir.d_name[0];
		if( strchr( name, ':') != NULL ){	// コロンを含む項目はスキップ.
			continue;
		}
		strcpy( buf, SYSFS_USB );
		strcat( buf, "/" );
		strcat( buf, name );
		strcat( buf, "/" );

		// 指定デバイスファイルがWLANかチェック.
		i=0;
		while( wlan_support_deivices[i].e_device != THRE_WLAN_DEVICE_TYPE(0) ) {
			if( checkUsbDeviceId( buf, wlan_support_deivices[i].us_vendorId, wlan_support_deivices[i].us_productId ) ) {
				e_wlan_device = wlan_support_deivices[i].e_device;	// 一致したデバイスを保存.
				SYSF_closedir( dp );	// /sys/bus/usb/devicesをクローズ.
				printf( "%ld : %s detection wlan device :%d , path:%s\n", SYSF_tickGet(), __FUNCTION__, (int32_t)e_wlan_device, buf );	// forTest.
				return;
			}
			i++;
		}
	}
	e_wlan_device = THRE_WLAN_DEVICE_TYPE_NOT_EXIST;	// WLANデバイスが見つからなかった.
	printf( "%ld : %s wlan device was not detected.\n", SYSF_tickGet(), __FUNCTION__ );	// 2016/04/06 Harada ZeusSBK 静的解析対応(ID:32177).
	SYSF_closedir( dp );	// /sys/bus/usb/devicesをクローズ.
}

int32_t sl_WLAN_Timer = 5000;
/*================================================================================================
［機能］	WLANデバイスの物理的なUSBポート接続状態を返す.
--------------------------------------------------------------------------------------------------
［引数］	なし.
［戻値］	True:接続 False:未接続.
［履歴］	2012/03/30 kurachi ZeusMLK機種（Nic2枚ざし対応-WLAN装着状態判定I/F）.
			2012/07/26 kurachi ZeusMLK CQ:369287対策（ポート固定制御をはずす）.
			2013/09/17 fukuhara スレッドセーフでない関数の対応 readdir -> readdir_r.
			2014/20/27 FT_yamaguchi CQ.501467 バス番号の指定処理を削除する
			2016/02/26 河村 RQ-340 IEEE802.11ac/ad対応。保持されているWLANデバイス情報を使用する用変更。中身はTHRF_fixWLANDevice()へ移植。
			2019/12/17 山口 OP_BTS-17874_test_BSP_起動時にC-D262 起動時に無線LANを認識できない場合は親ポートをバスリセットする.
================================================================================================*/
Bool THRF_isExistWLAN()
{
	// 2016/04/15 Harada CQ.527019、CQ.527061 WLANデバイスが初期化(未確定)の場合は確定させる.
	if( e_wlan_device == THRE_WLAN_DEVICE_TYPE_INITIALIZATION ) {
		THRF_fixWLANDevice();
		if(e_wlan_device == THRE_WLAN_DEVICE_TYPE_NOT_EXIST){
			system("echo -n 4-1 > /sys/bus/usb/drivers/usb/unbind");	// 無線LANに対してunbind.
			system("echo -n 4-1 > /sys/bus/usb/drivers/usb/bind");		// 無線LANに対してbind.
			SYSF_taskDelay( (int)(SYSF_sysClkRateGet() *  sl_WLAN_Timer / 1000 ) );	// 5s wait.
			THRF_fixWLANDevice();
		}
	}
	if( e_wlan_device == THRE_WLAN_DEVICE_TYPE_INITIALIZATION || e_wlan_device == THRE_WLAN_DEVICE_TYPE_NOT_EXIST ) {
		return False;
	} else {
		return True;
	}
}
#endif	// #if defined(DEF_ZeusMLKBoard) || defined(DEF_S800Emu).

/* 2020/04/27 山口 OP_BTS-22870 スリープ復帰ランニング中リブート発生(Phison製SSDスクリーニング品). */
extern SYSS_TaskInfo* SYSF_taskInfoGet (int32_t type, void* param, int32_t* count);
/*================================================================================================
［機能］	CPUマスクによってCPUコアにタスクを割り当てる.
--------------------------------------------------------------------------------------------------
［引数］	e_IdType  :タスクID指定
			ul_TaskId :プロセスID、又は、スレッドID.
			ul_CpuMask:CPUマスク(THRD_CPUMASK_…).
［戻値］	0:正常終了 0以外:エラー(errno).
［履歴］	2013/09/24 Sky藤井 ディープスリープ省電力パフォーマンス対応 S73-BSP_DD-28 4.5.
================================================================================================*/
int32_t THRF_taskBindByMask( THRE_IDTYPE e_IdType, uint32_t ul_TaskId, uint32_t ul_CpuMask )
{
#if defined(DEF_ARMADAXP) || defined(DEF_HELIOS_EMU2) || defined(DEF_Helios_PF)
	uintptr_t i_Pid;
	int32_t i_Ret;
	int32_t i_Bit;
	static int32_t i_Cpus = 0;

	if( 0 == i_Cpus ) {
		i_Cpus  = get_nprocs_conf();
	}

	if( 1 >= i_Cpus ) {
		i_Ret = 0;/* なにもせず正常値を返す. */
	} else {
#if 1
		const char* pathfmt = "/cgroup/%s/tasks";
		char path[DEF_CGROUP_PATHSIZE];
#endif
		if( THRE_IDTYPE_THREAD == e_IdType) {/* スレッドID指定. */
			if( 0 == ul_TaskId ) {
				i_Pid = syscall(SYS_gettid);
			} else {
				i_Pid = SYSF_taskNameToPid(SYSF_taskName(ul_TaskId));
			}
		} else {
			if( 0 == ul_TaskId ) {
				i_Pid = syscall(SYS_gettid);
			} else {
				i_Pid = (uintptr_t)ul_TaskId;
			}
		}

		i_Ret = EINVAL;
		/* CPU数以上はループしない(有効でないビットは無視). */
		for( i_Bit=0; i_Cpus>i_Bit; ++i_Bit ) {
			if( 0 != ( (ul_CpuMask >> i_Bit) & 1) ) {
#if 1
				snprintf( path, DEF_CGROUP_PATHSIZE, pathfmt, (0<i_Bit?"sub":"main") );
#ifdef DEF_64BIT_PF		// EagleH_制御構成変更_No.49 swap有効化対応.
				i_Ret = setTaskAffinityCgroup( (pid_t)i_Pid, path, NULL );
#else	// DEF_64BIT_PF.
				i_Ret = setTaskAffinityCgroup( (pid_t)i_Pid, path );
#endif	// DEF_64BIT_PF.
#else
				i_Ret = SYSF_taskGroupSet( (THR_ID)i_Pid, (0<i_Bit?"sub":"main") );
#endif
			}
		}
	}

	return i_Ret;
#else/* defined(DEF_ARMADAXP) || defined(DEF_HELIOS_EMU2) || defined(DEF_Helios_PF) */
	cpu_set_t cpuSet;
	THR_ID thrId;
	int i_Ret;
	int i_Bit;
	static int i_Cpus = 0;

	if( 0 == i_Cpus ) {
		i_Cpus  = get_nprocs();
	}

	if( 1 >= i_Cpus ) {
		i_Ret = 0;/* なにもせず正常値を返す. */
	} else {
		CPU_ZERO(&cpuSet);

		/* CPU数以上はループしない(有効でないビットは無視). */
		for( i_Bit=0; i_Cpus>i_Bit; ++i_Bit ) {
			if( 0 != ( (ul_CpuMask >> i_Bit) & 1) ) {
				CPU_SET( i_Bit, &cpuSet );
			}
		}

		/* ul_CpuMaskが0でなくても有効でないCPUのみ指定されている場合を想定してcpu_set_tを作ってからチェックする. */
		i_Bit = CPU_COUNT( &cpuSet );
		if( 0 != i_Bit ) {
			if( THRE_IDTYPE_THREAD == e_IdType) {/* スレッドID指定. */
				if( 0 == ul_TaskId ) {
					thrId = (THR_ID)SYSF_taskIdSelf();
				} else {
					thrId = (THR_ID)ul_TaskId;
				}
				i_Ret = pthread_setaffinity_np( thrId, sizeof(cpu_set_t), &cpuSet );
			} else {/* プロセスID指定 */
				i_Ret = sched_setaffinity( (pid_t)ul_TaskId, sizeof(cpu_set_t), &cpuSet );
			}
		} else {
			i_Ret = EINVAL;
		}
	}

	return i_Ret;
#endif/* !(defined(DEF_ARMADAXP) || defined(DEF_HELIOS_EMU2) || defined(DEF_Helios_PF)) */
}

/*================================================================================================
［機能］	CPUマスクによってCPUコアにタスクを割り当てる.
--------------------------------------------------------------------------------------------------
［引数］	e_IdType  :タスクID指定
			ul_TaskId :プロセスID、又は、スレッドID.
			e_CpuId   :CPUID(コア番号).
［戻値］	0:正常終了 0以外:エラー(errno).
［履歴］	2013/09/24 Sky藤井 ディープスリープ省電力パフォーマンス対応 S73-BSP_DD-28 4.5.
================================================================================================*/
int32_t THRF_taskBind( THRE_IDTYPE e_IdType, uint32_t ul_TaskId, THRE_CPUID e_CpuId )
{
	uint32_t ul_CpuMask = ( (THRE_CPUID_0 == e_CpuId) ? THRD_CPUMASK_0 : ( (THRE_CPUID_1 == e_CpuId) ? THRD_CPUMASK_1 : 0 ) );

	return THRF_taskBindByMask( e_IdType, ul_TaskId, ul_CpuMask );
}

/* 2020/04/27 山口 OP_BTS-22870 スリープ復帰ランニング中リブート発生(Phison製SSDスクリーニング品). */
//extern int setAffinity(int cpuMsk, pid_t tid);
static const char* const	DEF_ENV_GRPFILE =	"GRPFILE";
static const char* const	DEF_ENV_CGPATH =	"CGPATH";
static const int32_t	DEF_BUFSIZE =			(64);
static const int32_t	DEF_COMMLEN =			(16);
#define	DEF_ENV_CORE0PATH	"CORE0PATH"
#define	DEF_ENV_CORE1PATH	"CORE1PATH"
#define	DEF_ENV_CORE2PATH	"CORE2PATH"
#define	DEF_ENV_CORE3PATH	"CORE3PATH"

	
int32_t THRF_taskBind_Quad( THRE_IDTYPE e_IdType, uint32_t ul_TaskId, int cpuMsk ){
	
	uintptr_t i_Pid;

	if( THRE_IDTYPE_THREAD == e_IdType) {/* スレッドID指定. */
		if( 0 == ul_TaskId ) {
			i_Pid = syscall(SYS_gettid);
		} else {
			i_Pid = SYSF_taskNameToPid(SYSF_taskName(ul_TaskId));
		}
	} else {
		if( 0 == ul_TaskId ) {
			i_Pid = syscall(SYS_gettid);
		} else {
			i_Pid = (uintptr_t)ul_TaskId;
		}
	}
	
//	setAffinity(cpuMsk, i_Pid);
	
	
	
	const char* pathfmt = "%s/%s/tasks";
	const char* cgpath = getenv(DEF_ENV_CGPATH);
	char path[DEF_CGROUP_PATHSIZE];
	char* group = NULL;
	SYSS_TaskInfo *taskInfo=NULL;
	if(SYSF_getTaskInfo((THR_ID)pthread_self(), &taskInfo) == ERROR){
		printf("SYSF_getTaskInfo Error\n");
		return -1;		/* 2020/05/09 山口 OP_BTS-23434 タスク情報取得失敗でSegF */
	}

	switch(cpuMsk) {
		case 0x0: /* コア0指定 */
			group = getenv(DEF_ENV_CORE0PATH);
			break;

		case 0x1: /* コア1指定 */
			group = getenv(DEF_ENV_CORE1PATH);
			break;

		case 0x2: /* コア2指定 */
			group = getenv(DEF_ENV_CORE2PATH);
			break;

		case 0x3: /* コア3指定 */
			group = getenv(DEF_ENV_CORE3PATH);
			break;

		default:
			/* cpuMsk=0や上記の値以外は、何もしない。（コア割り当ては従来どおりmainグループ(コア0)とになる。) */
			return 0;	/* success. */
	}

	if( group != NULL ){
		if( cgpath != NULL && strlen(cgpath) + strlen(group) + strlen(pathfmt) < DEF_CGROUP_PATHSIZE ){
			if( sprintf(path, pathfmt, cgpath, group) > 0 )
				return setTaskAffinityCgroup((pid_t)i_Pid, path ,taskInfo->taskName);
		}
	}
	
	return 0;
}
/*================================================================================================
［機能］	MACアドレスの取得.
--------------------------------------------------------------------------------------------------
［引数］	char* psc_MacAddress.
［戻値］	True:取得成功 False:取得失敗.
［履歴］	2013/03/28 horii 新規 UV-01062 TPMの搭載.
			2014.04.10.	Sky井柳	【Zeus5BK】制御構成変更_No.210 SPI-Flashへのアクセス先変更対応.
			2014.10.29	Sky小倉	EEPROMレス対応.
			2017.09.01	Sky井柳 [Eagle][制御構成変更No10] EEPROM処理削除.
================================================================================================*/
Bool THRF_getMacAddress(char* psc_MacAddress)
{
	uchar uc_TmpBuf[6];

	memset(uc_TmpBuf, 0x00, sizeof(uc_TmpBuf));
	
	if(psc_MacAddress == NULL){
		printf( "THRF_getMacAddress: psc_MacAddress == NULL\n" );
		return False;
	}
	
	uchar uc_Ret = THRG_getMachineInfo( 0x63, uc_TmpBuf, 6 );
	if( ( uc_Ret != THRD_GET_SPI_OK )					// 取得OK.
		 &&( uc_Ret != THRD_GET_SPI_OK_AREA1_ERR )		// バックアップデータ取得OK：1面目にメモリ不整合.
		  &&( uc_Ret != THRD_GET_SPI_OK_AREA2_ERR )	){	// バックアップデータ取得OK：2面目にメモリ不整合.
		printf( "THRF_getMacAddress:THRG_getMachineInfo NG.\n" );
        return False;
	}

	snprintf(psc_MacAddress, 32,"%02X:%02X:%02X:%02X:%02X:%02X"
		,uc_TmpBuf[0],uc_TmpBuf[1],uc_TmpBuf[2],uc_TmpBuf[3],uc_TmpBuf[4],uc_TmpBuf[5]);
	
	return True;
}

/*================================================================================================
［機能］	EEPROMのPH構成情報を取得する.
--------------------------------------------------------------------------------------------------
［引数］	char* sc_PhInfo	EEPROMから取得した値(OUT).
［戻値］	Bool 読み出し成功/失敗.
［履歴］	2013/07/08 itakura 緊急CD向け1ビーム/2ビーム切替対応.
			2013/08/08 itakura CQ382291 コンパイルスイッチ対応.
			2014.04.10.	Sky井柳	【Zeus5BK】制御構成変更_No.210 SPI-Flashへのアクセス先変更対応.
			2014.10.29	Sky小倉	EEPROMレス対応.
			2017.09.01	Sky井柳 [Eagle][制御構成変更No10] EEPROM処理削除.
================================================================================================*/
Bool THRG_getEepromIPROMInfo( char* sc_PhInfo )
{
	if( sc_PhInfo == NULL ){
		printf( "THRG_getEepromIPROMInfo:sc_PhInfo is NULL NG.\n" );
        return False;
	}
	uchar uc_Ret = THRG_getMachineInfo( 0x53, sc_PhInfo, 1 );
	if( ( uc_Ret != THRD_GET_SPI_OK )					// 取得OK.
		 &&( uc_Ret != THRD_GET_SPI_OK_AREA1_ERR )		// バックアップデータ取得OK：1面目にメモリ不整合.
		  &&( uc_Ret != THRD_GET_SPI_OK_AREA2_ERR )	){	// バックアップデータ取得OK：2面目にメモリ不整合.
		printf( "THRG_getEepromIPROMInfo:THRG_getMachineInfo NG.\n" );
        return False;
	}
	printf(" THRG_getEepromIPROMInfo read value %d \n", *sc_PhInfo );
	return True;
}

/*================================================================================================
［機能］	EEPROMのPH構成情報を書き込む.
--------------------------------------------------------------------------------------------------
［引数］	uchar uc_PhInfo	EEPROMに書き込む値.
［戻値］	Bool 	書き込み成功/失敗.
［履歴］	2013/07/08 itakura 緊急CD向け1ビーム/2ビーム切替対応.
			2013/08/08 itakura CQ382291 コンパイルスイッチ対応.
			2014.04.10.	Sky井柳	【Zeus5BK】制御構成変更_No.210 SPI-Flashへのアクセス先変更対応.
			2014.10.29	Sky小倉	EEPROMレス対応.
			2017.09.01	Sky井柳 [Eagle][制御構成変更No10] EEPROM処理削除.
================================================================================================*/
Bool THRG_setEepromIPROMInfo( char sc_PhInfo )
{
	uchar uc_Ret = THRG_setMachineInfo( 0x53, &sc_PhInfo, 1 );
	if( uc_Ret != THRD_SET_SPI_OK ){	// 正常終了以外は、FALSE(失敗).
        printf( "THRG_setEepromIPROMInfo:THRG_setMachineInfo NG.\n" );
		return False ;
	}
	printf(" THRG_setEepromIPROMInfo write value %d ", sc_PhInfo );
	return True;
}


/*================================================================================================
［機能］	本体起動時に参照されている画パラファイルのバージョン情報から、対応するPH構成情報を取得する.
--------------------------------------------------------------------------------------------------
［引数］	void.
［戻値］	char 	-1:画パラファイル異常.
					0:2ビーム向け画パラファイル.
					1:1ビーム向け画パラファイル.
［履歴］	2013/07/08 itakura 緊急CD向け1ビーム/2ビーム切替対応.
			2013/08/08 itakura CQ382291 コンパイルスイッチ対応.
			2017/04/07 Sky		ZeusSMLKPF対応.
			2017/09/14 Sky井柳 [Eagle][制御構成変更No5] コンパイルスイッチ変更.
================================================================================================*/
char THRG_getSSDIPROMInfo()
{
#if (defined TORNADO) && (!defined FUM_LINUX)		// 実機で有効.
#if !defined( DEF_S800Emu )
	char sc_Ret = THRD_SsdIpromVersionInvalid;
	char sc_PicIpVerNum = 0;
	char sc_ThrIpVerNum = 0;
	// PIC・スレッド画パラファイルバージョンから1ビーム/2ビーム判定箇所(1文字)を格納.
	char* sc_PicIpPHInfo = (char*)(IPROM_CPSPIC_INT_Ver)+THRD_PhInfoOffset;
	char* sc_IpVer = (THRF_IsA800IntegrationExist() == True) ? IP_Ver_A800 : IP_Ver;
	char* sc_ThrIpPHInfo = sc_IpVer+THRD_PhInfoOffset;

	// 取得した文字を数値変換.
	sc_PicIpVerNum = *sc_PicIpPHInfo - '0';
	sc_ThrIpVerNum = *sc_ThrIpPHInfo - '0';

	// 読み込んだバージョンからいずれも1ビーム向けであれば、1ビーム向け画パラファイルと判定する.
	if ( sc_PicIpVerNum == THRD_SSDPh1Beam && sc_ThrIpVerNum == THRD_SSDPh1Beam ) {
		sc_Ret = THRD_SsdIpromVersion1Beam;
	} else if ( sc_PicIpVerNum == THRD_SSDPh2Beam && sc_ThrIpVerNum == THRD_SSDPh2Beam ) {
		// 読み込んだバージョンからいずれも2ビーム向けであれば、2ビーム向け画パラファイルと判定する.
		sc_Ret = THRD_SsdIpromVersion2Beam;
	} else {
		// 以下のケースの場合、この分岐に入り、画パラファイル異常を返す.
		// ・読み込んだバージョンが1ビーム/2ビームいずれでもない場合.
		// ・PIC用とスレッド用のPH情報の整合性が取れていない場合.
		printf( "THRG_getSSDIPROMInfo : IPROM Version Invalid PIC=%s, Thread=%s\n", sc_IpVer, IPROM_CPSPIC_INT_Ver );
	}
#else	// !defined( DEF_S800Emu ).
		char sc_Ret = THRD_SsdIpromVersion2Beam;
#endif	// !defined( DEF_S800Emu ).
#else		// 実機で有効.
	char sc_Ret = THRD_SsdIpromVersion2Beam;
#endif
	return sc_Ret;
}

/*================================================================================================
［機能］	PH構成情報一致・不一致チェック.
--------------------------------------------------------------------------------------------------
［引数］	TYPE_EngineType	エンジンのPH構成情報.
［戻値］	char 	-2:構成不一致(画パラ異常).
					-1:構成不一致(エンジンと画パラ不一致).
					0:構成一致(エンジンとEEPROMが異なる場合も含む).
［履歴］	2013/07/08 itakura 緊急CD向け1ビーム/2ビーム切替対応.
			2013/08/08 itakura CQ382291 コンパイルスイッチ対応.
			2017/09/14 Sky井柳 [Eagle][制御構成変更No5] コンパイルスイッチ変更.
================================================================================================*/
char THRG_CheckPHBeamInfo( TYPE_PHType sc_EngPH )
{
#if (defined TORNADO) && (!defined FUM_LINUX)		// 実機で有効.
#if !defined( DEF_S800Emu )
	char sc_Ret = THRD_SsdIpromInvalid;
	uchar uc_ChkFlg = 0;				// エンジン・EEPROM・画パラファイルのPH構成情報パターンフラグ.
	Bool b_SymLinkResult = True;		// シンボリックリンク作成結果.
	Bool b_EepromReadResult = False;		// EEPROMへの情報書き込み結果.
	Bool b_EepromWriteResult = True;	// EEPROMへの情報書き込み結果.
	char sc_EepromInfo = -1;					// EEPROMの情報取得値.
	char sc_SSDPhInfo = THRD_SsdIpromVersionInvalid;			// 画パラファイルに対応するPH構成情報取得結果.
	
	// 引数チェック.
	if ( sc_EngPH != TYPE_PHType1Beam && sc_EngPH != TYPE_PHType2Beam ) {
		// 1ビーム/2ビーム以外の値が引数に指定されたらFA14を発生させる.
		THRL_ThAssert( False, "THRG_CheckPHBeamInfo Param Error %d", sc_EngPH, 0, 0, 0, CFA14_DetailCode0x0BF4 );
	}

	// エンジン・EEPROM・画パラファイルのPH情報が1ビームであれば、フラグをセットする.
	// 画パラファイルに対応するPH構成情報取得.
	sc_SSDPhInfo = THRG_getSSDIPROMInfo();
	switch ( sc_SSDPhInfo ) {
		case THRD_SsdIpromVersion1Beam:
			// 1ビーム向け画パラファイルの場合はフラグをセットする.
			uc_ChkFlg |= THRD_Ssd1BeamFlg
			break;
		case THRD_SsdIpromVersion2Beam:
			// 2ビーム向け画パラファイルの場合はフラグをセットしない.
			break;
		case THRD_SsdIpromVersionInvalid:
		default:
			// 画パラファイル異常の場合.
			return  THRD_SsdIpromInvalid;
			break;
	}

	// エンジンのPH情報取得.
	if ( sc_EngPH == TYPE_PHType1Beam ) {
		uc_ChkFlg |= THRD_Eng1BeamFlg;
	}
	
	// EEPROMのPH構成情報取得.
	b_EepromReadResult = THRG_getEepromIPROMInfo( &sc_EepromInfo );
	if ( b_EepromReadResult == False ) {
		// EEPROMの読み出しに失敗した場合は画パラファイル異常としてAssetする.
		THRL_ThAssert( False, "THRG_CheckPHBeamInfo Error %d %d", b_EepromWriteResult, b_SymLinkResult, 0, 0, CFA14_DetailCode0x0BF5 );
	}
	if ( sc_EepromInfo == THRD_SSDPh1Beam ) {
		uc_ChkFlg |= THRD_Eeprom1BeamFlg;
	}


	// エンジン・EEPROM・画パラファイルのPH構成一致・不一致チェック.
	// PH情報が異なる場合は、EEPROM・画パラファイルのリンクエンジンの情報に合わせる.
	// また、スナップショット取得フラグをセットする.
	printf( "THRG_CheckPHBeamInfo uc_ChkFlg = %d \n", uc_ChkFlg );
	switch ( uc_ChkFlg ) {
		case 0x0:		// エンジン:2ビーム/EEPROM:2ビーム/画パラファイル:2ビーム.
			printf(" PH Info Eng:2Beam EEPROM:2Beam SSD:2Beam \n");
			sc_Ret = THRD_PhInfoMatch;
			break;
		case 0x1:		// エンジン:2ビーム/EEPROM:2ビーム/画パラファイル:1ビーム.
			printf(" PH Info Eng:2Beam EEPROM:2Beam SSD:1Beam \n");
			b_SymLinkResult = THRG_changeIPROMSymLink( sc_EngPH );
			APIC_SystemInfo::newInstance()->setWarpInfoForMfpData( True );
			sc_Ret = THRD_EngSsdPhInfoNotMatch;
			break;
		case 0x2:		// エンジン:2ビーム/EEPROM:1ビーム/画パラファイル:2ビーム.
			printf(" PH Info Eng:2Beam EEPROM:1Beam SSD:2Beam \n");
			b_EepromWriteResult = THRG_setEepromIPROMInfo( THRD_SetEepromPhInfo2Beam );
			APIC_SystemInfo::newInstance()->setWarpInfoForMfpData( True );
			sc_Ret = THRD_PhInfoMatch;
			break;
		case 0x3:		// エンジン:2ビーム/EEPROM:1ビーム/画パラファイル:1ビーム.
			printf(" PH Info Eng:2Beam EEPROM:1Beam SSD:1Beam \n");
			b_EepromWriteResult = THRG_setEepromIPROMInfo( THRD_SetEepromPhInfo2Beam );
			b_SymLinkResult = THRG_changeIPROMSymLink( sc_EngPH );
			APIC_SystemInfo::newInstance()->setWarpInfoForMfpData( True );
			sc_Ret = THRD_EngSsdPhInfoNotMatch;
			break;
		case 0x4:		// エンジン:1ビーム/EEPROM:2ビーム/画パラファイル:2ビーム.
			printf(" PH Info Eng:1Beam EEPROM:2Beam SSD:2Beam \n");
			b_EepromWriteResult = THRG_setEepromIPROMInfo( THRD_SetEepromPhInfo1Beam );
			b_SymLinkResult = THRG_changeIPROMSymLink( sc_EngPH );
			APIC_SystemInfo::newInstance()->setWarpInfoForMfpData( True );
			sc_Ret = THRD_EngSsdPhInfoNotMatch;
			break;
		case 0x5:		// エンジン:1ビーム/EEPROM:2ビーム/画パラファイル:1ビーム.
			printf(" PH Info Eng:1Beam EEPROM:2Beam SSD:1Beam \n");
			b_EepromWriteResult = THRG_setEepromIPROMInfo( THRD_SetEepromPhInfo1Beam );
			APIC_SystemInfo::newInstance()->setWarpInfoForMfpData( True );
			sc_Ret = THRD_PhInfoMatch;
			break;
		case 0x6:		// エンジン:1ビーム/EEPROM:1ビーム/画パラファイル:2ビーム.
			printf(" PH Info Eng:1Beam EEPROM:1Beam SSD:2Beam \n");
			b_SymLinkResult = THRG_changeIPROMSymLink( sc_EngPH );
			APIC_SystemInfo::newInstance()->setWarpInfoForMfpData( True );
			sc_Ret = THRD_EngSsdPhInfoNotMatch;
			break;
		case 0x7:		// エンジン:1ビーム/EEPROM:1ビーム/画パラファイル:1ビーム.
			printf(" PH Info Eng:1Beam EEPROM:1Beam SSD:1Beam \n");
			sc_Ret = THRD_PhInfoMatch;
			break;
		default:
			THRL_ThAssert( False, "THRG_CheckPHBeamInfo  Param Error %d", sc_EngPH, 0, 0, 0, CFA14_DetailCode0x0BF6 );
			break;
	}

	// EEPROMへの書き込みまたはシンボリックリンクの切り替えに失敗した場合はFatalを出す.
	if ( b_EepromWriteResult == False || b_SymLinkResult == False ) {
		THRL_ThAssert( False, "THRG_CheckPHBeamInfo Error %d %d", b_EepromWriteResult, b_SymLinkResult, 0, 0, CFA14_DetailCode0x0BF7 );
	}
#else // !defined( DEF_S800Emu ).
	char sc_Ret = THRD_PhInfoMatch;		// Emuは、構成一致とする.
#endif // !defined( DEF_S800Emu ).
#else	// (defined TORNADO) && (!defined FUM_LINUX).
	char sc_Ret = THRD_PhInfoMatch;
#endif
	return sc_Ret;
}

Bool THRF_isAral65()
{
	return True;
}

// ================================================================================================.
//! <!--機能--> 10usecウェイト関数.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param uint32_t count ... タイマカウント値（10usec単位）.
//! <!--戻値-->.
//! \return.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
//  09/08/07 越野	 Donau環境から分離して作成.
//  10/04/09 大庭 CQ.324581 DonauBK用にWait時間調整.
//  13/02/28 Sky 機種共通化 THRC_BSPifSoC.cppから移植.
//  13/03/06 Sky AmurCusの処理を削除.
// ================================================================================================.
#ifdef COSIM
 #define	CT_10US	160
#else
// #define	CT_10US	2000	// 計測して設定値を決定すること　この数値だと 4.66 us相当.
// #define	CT_10US	4300	// Mosel計測値.
#define		CT_10US	5900	// Donau計測値.

// 2011/02/02 ft_kondo CQ.324581 第6回手動マージ. 
#ifdef DEF_DonauBKCus
#define		CT_AMUR1000M_10US	1110	// DonauBK 1GHz計測値.
#else
#define		CT_AMUR1000M_10US	1065	// Amur 1GHz計測値.
#endif

#define		CT_AMUR667M_10US	704		// Amur 667MHz計測値.
#define		DEF_CLOCK_AMUR_1000M		999999990
#define		DEF_CLOCK_AMUR_667M			666666660
#endif  // #ifdef COSIM.

#if defined(TORNADO) 
#ifdef DEF_AmurCus
volatile int32_t wait_dummy1=0, wait_dummy2=0;
extern uint32_t  coreFreq;		// CPUクロックを保持しているグローバル変数.
#endif
#endif

void wait_10us( uint32_t count )
{
#if defined(TORNADO) 

	// for Donau.
	for( register uint32_t i = 0; i < count; i++ ) {
		for( register ushort us_count = 0; us_count < CT_10US; us_count++ ) {}	//10us相当ウェイト.
	}
#endif // #if defined(TORNADO).
}

// wait_10us()の定数テスト用.
// sp waitTestをターミナルから実行して定数を合わせる.
#ifdef TORNADO
void wait_10us_Test( int32_t sec )
{
	if( sec==0 )
		sec=10;

	uint32_t ul = SYSF_tickGet();
	THRL_ThLogMsgErr( "Start Time[tick] : %"PRId32"\n", ul,2,3,4,5,6 );
	wait_10us( 100*1000*sec );
	uint32_t ul2 = SYSF_tickGet();
	THRL_ThLogMsgErr( "End Time[tick] : %"PRId32"\n", ul2,2,3,4,5,6 );
	THRL_ThLogMsgErr( "Start - End Time[ms] : %"PRId32" (%"PRId32")\n", (ul2-ul)*5,sec*1000,3,4,5,6 );
}
#endif

// ================================================================================================.
//! <!--機能--> 接続しているアクセラレータDSのASIC種類を返す.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return THRE_AsicKindDS e_DSAsicKind.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
//!  2014/09/30	Sky	新規作成.
//!	 2015/10/06 Sky システム統合SoC(S800)対応.
//!  2017/04/07 Sky	ZeusSMLKPF対応.
// ================================================================================================.
THRE_AsicKindDS THRF_getAcceleratorDS_AsicKind()
{
	THRE_AsicKindDS e_DSAsicKind = THRE_AsicKindDS_AsicNon;

	if( True == THRF_IsA800IntegrationExist() ) { 
	   // A800画像処理統合装着時のみアクセラレータDSとして動作.
		e_DSAsicKind = THRE_AsicKindDS_A800;
	}

	return e_DSAsicKind;
}
// from mtd-abi.h
struct erase_info_user {
    uint32_t start;
    uint32_t length;
};

// [SecV2-3] Flash Write Protect Control.
struct otp_info {
	__u32 start;
	__u32 length;
	__u32 locked;
};

#define MEMERASE _IOW('M', 2, struct erase_info_user)
#define MEMWRITEPROTECT	_IOWR('M', 25, struct otp_info)
#define POWERDOWN_REGION 0x76000	// 電断時領域(4KB).
static SEM_ID semID_Flash = NULL;

int32_t getFlashFd(void)
{
	static int32_t fd = -1;
	if( fd < 0){
		fd = SYSF_devopen( "/dev/mtd2", O_RDWR | O_SYNC, 0666 );
		if( fd  < 0 )
			return -1;
	}
	return fd;
}

/*================================================================================================
［機能］	Flashにライトする.
--------------------------------------------------------------------------------------------------
［引数］	uint32_t offset	:書き込み位置のオフセット
			void *buf		:書き込みデータのポインタ
			uint32_t size		:書き込みデータのサイズ
［戻値］	True:正常終了 False:エラー
［履歴］	2014/02/17 horii NVRAMレス対応 SPI-Flashアクセス関数新規作成.
================================================================================================*/
#if 1 /* 暫定対応:Sleep2復帰中にQSPIのアクセスでR4が固まる */
extern bool b_in_exec_str;
static bool b_writeFlash_enable = true;
void writeFlash_enable( bool ena ){
	b_writeFlash_enable = ena;
	printf("b_writeFlash_enable:%d \n",b_writeFlash_enable);
}
#endif

#include <pthread.h>

Bool THRF_writeFlash( uint32_t offset, void *buf, uint32_t size )
{
	int32_t fd = -1;
	off_t   lseek_ret = 0;
	ssize_t write_ret = 0;
#if defined(TORNADO) && !defined(FUM_LINUX) /*実機*/
	uint64_t ull_NewDetailCode = 0x0;
	Bool bl_ret = True;
#endif
	struct sched_param save_sched;
	int save_policy;
	
	// エラーチェック.
	if( ( offset >= FLASH_SIZE ) || ( buf == NULL ) || (size <= 0) || ( offset + size > FLASH_SIZE ) ){
		printf("%s: ERR offset %"PRId32" buf %p size %"PRId32"\n", __FUNCTION__, offset, buf, size);
		return False;
	}

#if 1 /* 暫定対応:Sleep2復帰中にQSPIのアクセスでR4が固まる */
	if((b_writeFlash_enable == false) && (b_in_exec_str == true)){
		return True;
	}
#endif
	
	// セマフォ取得.
	if( semID_Flash == NULL ){
		semID_Flash = SYSF_semMCreate(SEM_Q_PRIORITY | SEM_DELETE_SAFE | SEM_INVERSION_SAFE);
	}
	SYSF_semTake( semID_Flash, WAIT_FOREVER );
	// 2019/12/19 河村 -  OP_BTS-18210 一時的にタスクプライオリティを上げる.
	if (0 != pthread_getschedparam( pthread_self(), &save_policy, &save_sched ) ) {
		SYSF_semGive( semID_Flash );
		return False;
	}
	struct sched_param sched = {};
	sched.sched_priority = 207;		/* 通信系タスクと同等 */
	if ( save_sched.sched_priority < sched.sched_priority ) {
//		printf( "(%010u)(BSP)[%s] %s : prio %d -> %d\n", SYSF_tickGet(), __FUNCTION__, SYSF_taskName(SYSF_taskIdSelf()), save_sched.sched_priority, sched.sched_priority );
		if( 0 != pthread_setschedparam(pthread_self(), SCHED_RR, &sched) ) {
			SYSF_semGive( semID_Flash );
			return False;
		}
	}
	
	fd = getFlashFd();
	if( fd < 0 ){
		// 2019/12/19 河村 -  OP_BTS-18210 タスクプライオリティを戻す.
		if ( save_sched.sched_priority < sched.sched_priority ) {
//			printf( "(%010u)(BSP)[%s] %s : prio %d -> %d\n", SYSF_tickGet(), __FUNCTION__, SYSF_taskName(SYSF_taskIdSelf()), sched.sched_priority, save_sched.sched_priority );
			pthread_setschedparam( pthread_self(), save_policy, &save_sched );
		}
		SYSF_semGive( semID_Flash );
		return False;
	}
	
	// 指定の場所までシークする.
	lseek_ret = SYSF_lseek( fd, (off_t)offset, SEEK_SET);
	if( lseek_ret < 0 ){
		printf("%s: SYSF_lseek ERR errno:%d\n", __FUNCTION__, SYSF_errnoGet());
	// 2016/03/23 山口 CQ.525702 SPI-Flashへの書き込みに失敗した場合、新詳細コードを残す.
#if defined(TORNADO) && !defined(FUM_LINUX) /*実機*/
		ull_NewDetailCode = (((uint64_t)0x22                                  << 56) & 0xff00000000000000)	// モジュール番号.
						  | (((uint64_t)0x001                                 << 44) & 0x00fff00000000000)	// 通し番号.
						  | (((uint64_t)((SYSF_errnoGet() < 0) ?  -SYSF_errnoGet() :  SYSF_errnoGet())  << 36) & 0x00000ff000000000)	// errno.
						  | (((uint64_t)offset                                << 16) & 0x0000000fffff0000)	// 書き込み位置のオフセット.
						  | (((uint64_t)((size < 0xffff) ?  size :  0xffff)   <<  0) & 0x000000000000ffff);	// サイズ.

		bl_ret = APIC_setNewDetailCode( TYPE_FatalDetailCode_undefined, ull_NewDetailCode );		// 詳細コード登録.
		if (False == bl_ret) {
			SYSF_logMsg( "## %s %d : APIC_setNewDetailCode NG\n", __FUNCTION__, __LINE__, 3,4,5,6 );
		}
#endif	// #if defined(TORNADO) && !defined(FUM_LINUX).

		// 2019/12/19 河村 -  OP_BTS-18210 タスクプライオリティを戻す.
		if ( save_sched.sched_priority < sched.sched_priority ) {
			printf( "(%010u)(BSP)[%s] %s : prio %d -> %d\n", SYSF_tickGet(), __FUNCTION__, SYSF_taskName(SYSF_taskIdSelf()), sched.sched_priority, save_sched.sched_priority );
			pthread_setschedparam( pthread_self(), save_policy, &save_sched );
		}
		SYSF_semGive( semID_Flash );
		return False;
	}
	
	// Flashにライトする.
	// 2015/08/26 ft_kobayashi MinervaSBK Warning除去対応. 
	write_ret = SYSF_write( fd, (char*)buf, size);
	if( write_ret < 0 ){
		printf("%s: SYSF_write ERR errno:%d\n", __FUNCTION__, SYSF_errnoGet());
	// 2016/03/23 山口 CQ.525702 SPI-Flashへの書き込みに失敗した場合、新詳細コードを残す.
#if defined(TORNADO) && !defined(FUM_LINUX) /*実機*/
		ull_NewDetailCode = (((uint64_t)0x22                                  << 56) & 0xff00000000000000)	// モジュール番号.
						  | (((uint64_t)0x002                                 << 44) & 0x00fff00000000000)	// 通し番号.
						  | (((uint64_t)((SYSF_errnoGet() < 0) ?  -SYSF_errnoGet() :  SYSF_errnoGet())  << 36) & 0x00000ff000000000)	// errno.
						  | (((uint64_t)offset                                << 16) & 0x0000000fffff0000)	// 書き込み位置のオフセット.
						  | (((uint64_t)((size < 0xffff) ?  size :  0xffff)   <<  0) & 0x000000000000ffff);	// サイズ.

		bl_ret = APIC_setNewDetailCode( TYPE_FatalDetailCode_undefined, ull_NewDetailCode );		// 詳細コード登録.
		if (False == bl_ret) {
			SYSF_logMsg( "## %s %d : APIC_setNewDetailCode NG\n", __FUNCTION__, __LINE__, 3,4,5,6 );
		}
#endif	// #if defined(TORNADO) && !defined(FUM_LINUX).
		// 2019/12/19 河村 -  OP_BTS-18210 タスクプライオリティを戻す.
		if ( save_sched.sched_priority < sched.sched_priority ) {
			printf( "(%010u)(BSP)[%s] %s : prio %d -> %d\n", SYSF_tickGet(), __FUNCTION__, SYSF_taskName(SYSF_taskIdSelf()), sched.sched_priority, save_sched.sched_priority );
			pthread_setschedparam( pthread_self(), save_policy, &save_sched );
		}
		SYSF_semGive( semID_Flash );
		return False;
	}
	
	// 2019/12/19 河村 -  OP_BTS-18210 タスクプライオリティを戻す.
	if ( save_sched.sched_priority < sched.sched_priority ) {
//		printf( "(%010u)(BSP)[%s] %s : prio %d -> %d\n", SYSF_tickGet(), __FUNCTION__, SYSF_taskName(SYSF_taskIdSelf()), sched.sched_priority, save_sched.sched_priority );
		pthread_setschedparam( pthread_self(), save_policy, &save_sched );
	}

	// セマフォ開放.
	SYSF_semGive( semID_Flash );
	return True;
}

/*================================================================================================
［機能］	Flashからリードする.
--------------------------------------------------------------------------------------------------
［引数］	uint32_t offset	:読み込み位置のオフセット
			void *buf		:読み込みデータのポインタ
			uint32_t size		:読み込みデータのサイズ
［戻値］	True:正常終了 False:エラー
［履歴］	2014/02/17 horii NVRAMレス対応 SPI-Flashアクセス関数新規作成.
================================================================================================*/
Bool THRF_readFlash( uint32_t offset, void *buf, uint32_t size )
{
	int32_t fd = 0;
	off_t   lseek_ret = 0;
	ssize_t read_ret  = 0;
	
	// エラーチェック.
	if( ( offset >= FLASH_SIZE ) || ( buf == NULL ) || (size <= 0) || ( offset + size > FLASH_SIZE ) ){
		printf("%s: ERR offset %"PRId32" buf %p size %"PRId32"\n", __FUNCTION__, offset, buf, size);
		return False;
	}
	
	// セマフォ取得.
	if( semID_Flash == NULL ){
		semID_Flash = SYSF_semMCreate(SEM_Q_PRIORITY | SEM_DELETE_SAFE | SEM_INVERSION_SAFE);
	}
	SYSF_semTake( semID_Flash, WAIT_FOREVER );
	
	fd = getFlashFd();
	if( fd < 0 ){
		SYSF_semGive( semID_Flash );
		return False;
	}
	
	// 指定の場所までシークする.
	lseek_ret = SYSF_lseek( fd, (off_t)offset, SEEK_SET);
	if( lseek_ret < 0 ){
		printf("%s: SYSF_lseek ERR errno:%d\n", __FUNCTION__, SYSF_errnoGet());
		SYSF_semGive( semID_Flash );
		return False;
	}
	
	// Flashからリードする.
	// 2015/08/26 ft_kobayashi MinervaSBK Warning除去対応. 
	read_ret = SYSF_read( fd, (char*)buf, size);
	if( read_ret < 0 ){
		printf("%s: SYSF_read ERR errno:%d\n", __FUNCTION__, SYSF_errnoGet());
		SYSF_semGive( semID_Flash );
		return False;
	}
	
	// セマフォ開放.
	SYSF_semGive( semID_Flash );
	return True;
}

#define DEF_ERASE_RETRY_MAXNUM 10	//Flashイレースのリトライ回数.

/*================================================================================================
［機能］	Flashをイレースする.
--------------------------------------------------------------------------------------------------
［引数］	uint32_t start		:消去を行うアドレス(4KB単位)
			uint32_t size		:消去するサイズ(4KB単位)
［戻値］	True:正常終了 False:エラー
［履歴］	2014/02/17 horii NVRAMレス対応 SPI-Flashアクセス関数新規作成.
			2017/03/25 PS1-ft_kobayashi AR1015411 【新詳細コード対応】220070550000FFFF対応.
================================================================================================*/
Bool THRF_eraseFlash( uint32_t start, uint32_t size )
{
	int fd = -1;
	uchar uc_RetryCnt = 0;
	struct sched_param save_sched;
	int save_policy;

	
	//エラーチェック.
	if( ( start & 0xFFF ) || ( size & 0xFFF ) || (start >= FLASH_SIZE) || (size <= 0) || (start + size > FLASH_SIZE) ){
		printf("%s: start %"PRId32" size %"PRId32"\n", __FUNCTION__, start, size);
		return False;
	}
	
	// セマフォ取得.
	if( semID_Flash == NULL ){
		semID_Flash = SYSF_semMCreate(SEM_Q_PRIORITY | SEM_DELETE_SAFE | SEM_INVERSION_SAFE);
	}
	SYSF_semTake( semID_Flash, WAIT_FOREVER );
	// 2019/12/19 河村 -  OP_BTS-18210 一時的にタスクプライオリティを上げる.
	if (0 != pthread_getschedparam( pthread_self(), &save_policy, &save_sched ) ) {
		SYSF_semGive( semID_Flash );
		return False;
	}
	struct sched_param sched = {};
	sched.sched_priority = 207;		/* 通信系タスクと同等 */
	if ( save_sched.sched_priority < sched.sched_priority ) {
//		printf( "(%010u)(BSP)[%s] %s : prio %d -> %d\n", SYSF_tickGet(), __FUNCTION__, SYSF_taskName(SYSF_taskIdSelf()), save_sched.sched_priority, sched.sched_priority );
		if( 0 != pthread_setschedparam(pthread_self(), SCHED_RR, &sched) ) {
			SYSF_semGive( semID_Flash );
			return False;
		}
	}
	
	fd = getFlashFd();
	if( fd < 0 ){
		// 2019/12/19 河村 -  OP_BTS-18210 タスクプライオリティを戻す.
		if ( save_sched.sched_priority < sched.sched_priority ) {
//			printf( "(%010u)(BSP)[%s] %s : prio %d -> %d\n", SYSF_tickGet(), __FUNCTION__, SYSF_taskName(SYSF_taskIdSelf()), sched.sched_priority, save_sched.sched_priority );
			pthread_setschedparam( pthread_self(), save_policy, &save_sched );
		}
		SYSF_semGive( semID_Flash );
		return False;
	}
	
	// イレース.
	int ret = 0;
	struct	erase_info_user	info;	/* イレース用構造体 */

	info.start	= start;
	info.length	= size;
	// 2017/03/25 PS1-ft_kobayashi AR1015411 リトライ処理を追加.
	for(uc_RetryCnt = 0; uc_RetryCnt < DEF_ERASE_RETRY_MAXNUM; uc_RetryCnt++){
		ret = SYSF_ioctl( fd, MEMERASE, (intptr_t)&info );
		if(ret >= 0){
			break;
		}
	}
	if( ret < 0 ){
	// 2016/06/20 山口 CQ.530652 SPI-Flashへのイレースに失敗した場合、新詳細コードを残す.
#if defined(TORNADO) && !defined(FUM_LINUX) /*実機*/
		uint64_t ull_NewDetailCode = 0x0;
		Bool bl_ret = True;

		ull_NewDetailCode = (((uint64_t)0x22                                  << 56) & 0xff00000000000000)	// モジュール番号.
						  | (((uint64_t)0x007                                 << 44) & 0x00fff00000000000)	// 通し番号.
						  | (((uint64_t)((SYSF_errnoGet() < 0) ?  -SYSF_errnoGet() :  SYSF_errnoGet())  << 36) & 0x00000ff000000000)	// errno.
						  | (((uint64_t)start                                 << 16) & 0x0000000fffff0000)	// 書き込み位置のオフセット.
						  | (((uint64_t)((size < 0xffff) ?  size :  0xffff)   <<  0) & 0x000000000000ffff);	// サイズ.

		bl_ret = APIC_setNewDetailCode( TYPE_FatalDetailCode_undefined, ull_NewDetailCode );		// 詳細コード登録.
		if (False == bl_ret) {
			SYSF_logMsg( "## %s %d : APIC_setNewDetailCode NG\n", __FUNCTION__, __LINE__, 3,4,5,6 );
		}
#endif	// #if defined(TORNADO) && !defined(FUM_LINUX).

		printf("%s: SYSF_ioctl ERR errno:%d\n", __FUNCTION__, SYSF_errnoGet());
		// 2019/12/19 河村 -  OP_BTS-18210 タスクプライオリティを戻す.
		if ( save_sched.sched_priority < sched.sched_priority ) {
			printf( "(%010u)(BSP)[%s] %s : prio %d -> %d\n", SYSF_tickGet(), __FUNCTION__, SYSF_taskName(SYSF_taskIdSelf()), sched.sched_priority, save_sched.sched_priority );
			pthread_setschedparam( pthread_self(), save_policy, &save_sched );
		}
		SYSF_semGive( semID_Flash );
		return False;
	}
	// 2019/12/19 河村 -  OP_BTS-18210 タスクプライオリティを戻す.
	if ( save_sched.sched_priority < sched.sched_priority ) {
//		printf( "(%010u)(BSP)[%s] %s : prio %d -> %d\n", SYSF_tickGet(), __FUNCTION__, SYSF_taskName(SYSF_taskIdSelf()), sched.sched_priority, save_sched.sched_priority );
		pthread_setschedparam( pthread_self(), save_policy, &save_sched );
	}
	// セマフォ開放.
	SYSF_semGive( semID_Flash );
	
	return True;
}

// ================================================================================================.
//! <!--機能--> ダウンロード実行状態フラグの表示を行う.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param void 	なし.
//! <!--戻値-->.
//! \return void	なし.
//! <!--詳細-->.
//!  ダウンロード実行状態の、フラグ・読出し・書出し、の状態を表示する.\n
//! <!--履歴-->.
//! 2015/07/13 Sky O.Suzuki	新規作成 IRC/ENGのDLフラグ対応.
// ================================================================================================.
void THRG_GetDLStat( void )
{
	printf( "bl_MSCDownloading=[%d] WorkBuf[%d] DownloadingBuf[%d]\n", bl_MSCDownloading, uc_FlashWorkBuf[THRD_FlashOffset_MSCDownloading], uc_FlashDownloadingBuf[THRD_FlashOffset_MSCDownloading] );
	printf( "bl_ENGDownloading=[%d] WorkBuf[%d] DownloadingBuf[%d]\n", bl_ENGDownloading, uc_FlashWorkBuf[THRD_FlashOffset_ENGDownloading], uc_FlashDownloadingBuf[THRD_FlashOffset_ENGDownloading] );
	printf( "bl_IRCDownloading=[%d] WorkBuf[%d] DownloadingBuf[%d]\n", bl_IRCDownloading, uc_FlashWorkBuf[THRD_FlashOffset_IRCDownloading], uc_FlashDownloadingBuf[THRD_FlashOffset_IRCDownloading] );
	printf( "bl_EDHDownloading=[%d] WorkBuf[%d] DownloadingBuf[%d]\n", bl_EDHDownloading, uc_FlashWorkBuf[THRD_FlashOffset_EDHDownloading], uc_FlashDownloadingBuf[THRD_FlashOffset_EDHDownloading] );
	printf( "bl_FAXDownloading=[%d] WorkBuf[%d] DownloadingBuf[%d]\n", bl_FAXDownloading, uc_FlashWorkBuf[THRD_FlashOffset_FAXDownloading], uc_FlashDownloadingBuf[THRD_FlashOffset_FAXDownloading] );
	printf( "bl_MSCSubsetDownloading=[%d] WorkBuf[%d] DownloadingBuf[%d]\n", bl_MSCSubsetDownloading, uc_FlashWorkBuf[THRD_FlashOffset_MSCSubsetDownloading], uc_FlashDownloadingBuf[THRD_FlashOffset_MSCSubsetDownloading] );
	printf( "bl_MCPFDownloading=[%d] WorkBuf[%d] DownloadingBuf[%d]\n", bl_MCPFDownloading, uc_FlashWorkBuf[THRD_FlashOffset_MCPFDownloading], uc_FlashDownloadingBuf[THRD_FlashOffset_MCPFDownloading] );
}

// ================================================================================================.
//! <!--機能--> SPIフラッシュへ書き込みを行った際のべリファイを行う.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param const void *	べリファイ対象の書き込みデータ配列(I).
//! \param uint32_t	SPIフラッシュの読み込み位置オフセット(I).
//! \param uint32_t	SPIフラッシュからの読み込みサイズ(I).
//! <!--戻値-->.
//! \return Bool True：成功 / False：失敗.
//! <!--詳細-->.
//!  SPIフラッシュ搭載機種のみ使用可能。それ以外の機種では常にFalseを戻す.\n
//!  本API の用途は、SPIフラッシュに書き込みを行った際のべリファイである.\n
//!  従い、本APIを実行する前にSPIフラッシュへの書き込みを行っている必要がある.\n
//! <!--履歴-->.
//! 2014/02/25 yamada	新規作成 制御構成変更No.43 : NVRAMレス対応.
// ================================================================================================.
static Bool verifyFlashSubsetData(const void *p_CheckData, uint32_t ul_Offset, uint32_t ul_Size)
{
	if ((p_CheckData == NULL) || (ul_Size == 0) || (ul_Size > THRD_FlashSize_Sector)) {
		printf( "[%s] Parameter error.\n", __FUNCTION__);
		return False;
	}

	Bool	bl_Ret = False;
	Bool	bl_VerResult = False;
	int32_t		si_Ret = 0;
	uchar	uc_RetryCnt = 0;

	memset(uc_FlashVerifyBuf, 0x00, THRD_FlashSize_Sector);

	// SPI-Flash からデータを読み出し、書き込みデータの整合性チェックを行う.
	for (uc_RetryCnt = 0; uc_RetryCnt < THRD_VerifyRetryMaxNum; uc_RetryCnt++) {
		// 指定オフセットから指定サイズ読み出す.
		bl_Ret = THRF_readFlash(ul_Offset, (void *)uc_FlashVerifyBuf, ul_Size);
		if (bl_Ret == False) {
			printf( "[%s] SPI-Flash read error. offset = 0x%"PRIX32", size = %"PRId32"\n", __FUNCTION__, ul_Offset, ul_Size);
			break;
		}

		// 書き込みデータとの比較.
		si_Ret = memcmp(p_CheckData, (void *)uc_FlashVerifyBuf, ul_Size);
		if (si_Ret == 0) {
			// 一致している場合は成功を設定して抜ける.
			bl_VerResult = True;
			break;
		}

		// 100ms Delay(Rhein系では 5ms 単位で指定する).
		SYSF_taskDelay(20);
	}

	return bl_VerResult;
}

// ================================================================================================.
//! <!--機能--> ダウンロード実行状態をSPIフラッシュに書き込む.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param uchar	ダウンロード実行情報(I).
//! \param uint32_t	SPIフラッシュの書き込み位置オフセット(I).
//! <!--戻値-->.
//! \return Bool True：成功 / False：失敗.
//! <!--詳細-->.
//!  SPIフラッシュ搭載機種のみ使用可能。それ以外の機種では常にFalseを戻す.\n
//! <!--履歴-->.
//! 2014/02/25 yamada	新規作成 制御構成変更No.43 : NVRAMレス対応.
// ================================================================================================.
static Bool setFlashDownloadingStatus(uchar uc_Data, uint32_t ul_Offset)
{
	Bool	bl_Ret = False;

	memset(uc_FlashWorkBuf, 0x00, THRD_FlashSize_Sector);

	// 引数範囲チェック.
	if (ul_Offset >= THRD_FlashSize_Sector) {
		printf( "[%s] Parameter error. offset(%"PRId32").\n", __FUNCTION__, ul_Offset);
		return False;
	}

	// Subset情報を保存してあるセクタの読み出し.
	bl_Ret = THRF_readFlash(THRD_FlashOffset_SubsetSector, (void *)uc_FlashWorkBuf, THRD_FlashSize_Sector);
	if (bl_Ret == False) {
		printf( "[%s] SPI-Flash sector read error. offset = 0x%lX, size = %ld\n", __FUNCTION__, THRD_FlashOffset_SubsetSector, THRD_FlashSize_Sector);
		return False;
	}

	// 書き込みデータと現在のデータを比較し、異なる場合のみ書き込み処理を行う.
	if (uc_Data != uc_FlashWorkBuf[ul_Offset]) {
		// セクタイレースが必要か(1を書き込むか)のチェックを行う.
		if ((uc_Data & (uc_Data ^ uc_FlashWorkBuf[ul_Offset])) != 0) {
			// 読み出した配列の情報を書き込みデータで更新.
			uc_FlashWorkBuf[ul_Offset] = uc_Data;

			// セクタイレースで 1 を書き込む.
			bl_Ret = THRF_eraseFlash(THRD_FlashOffset_SubsetSector, THRD_FlashSize_Sector);
			if (bl_Ret == False) {
				printf( "[%s] SPI-Flash sector erase error. offset = 0x%lX, size = %ld\n", __FUNCTION__, THRD_FlashOffset_SubsetSector, THRD_FlashSize_Sector);
				return False;
			}

			// 更新データと元データの書き戻し.
			bl_Ret = THRF_writeFlash(THRD_FlashOffset_SubsetSector, (void *)uc_FlashWorkBuf, THRD_FlashSize_Sector);
			if (bl_Ret == False) {
				printf( "[%s] SPI-Flash write error. offset = 0x%lX, size = %ld\n", __FUNCTION__, THRD_FlashOffset_SubsetSector, THRD_FlashSize_Sector);
				return False;
			}

			// べリファイの実行.
			bl_Ret = verifyFlashSubsetData((void *)uc_FlashWorkBuf, THRD_FlashOffset_SubsetSector, THRD_FlashSize_Sector);
			if (bl_Ret == False) {
				printf( "[%s] SPI-Flash sector verify error. offset = 0x%lX, size = %ld\n", __FUNCTION__, THRD_FlashOffset_SubsetSector, THRD_FlashSize_Sector);
				return False;
			}
		} else {
			// SPI-Flash 書き込みAPIにて 0 を書き込む.
			uint32_t ul_WriteOffset = THRD_FlashOffset_SubsetSector+ul_Offset;

			bl_Ret = THRF_writeFlash(ul_WriteOffset, (void *)&uc_Data, sizeof(uc_Data));
			if (bl_Ret == False) {
				printf( "[%s] SPI-Flash write error. offset = 0x%"PRIX32", data = %d, size = %ld\n", __FUNCTION__, ul_WriteOffset, uc_Data, sizeof(uc_Data));
				return False;
			}

			// べリファイの実行.
			bl_Ret = verifyFlashSubsetData((void *)&uc_Data, ul_WriteOffset, sizeof(uc_Data));
			if (bl_Ret == False) {
				printf( "[%s] SPI-Flash verify error. offset = 0x%"PRIX32", data = %d, size = %ld\n", __FUNCTION__, ul_WriteOffset, uc_Data, sizeof(uc_Data));
				return False;
			}
		}
	}

	return True;
}

// ================================================================================================.
//! <!--機能--> MSCダウンロード実行状態をSPIフラッシュに書き込み、グローバル変数を更新する.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param uchar	ダウンロード実行情報(I).
//! <!--戻値-->.
//! \return Bool True：成功 / False：失敗.
//! <!--詳細-->.
//!  SPIフラッシュ搭載機種のみ使用可能。それ以外の機種では常にFalseを戻す.\n
//! <!--履歴-->.
//! 2014/02/25 yamada	新規作成 制御構成変更No.43 : NVRAMレス対応.
// ================================================================================================.
Bool THRG_SetMSCDownloading(uchar uc_Data)
{
	Bool bl_Ret = setFlashDownloadingStatus(uc_Data, THRD_FlashOffset_MSCDownloading);
	if (bl_Ret != False) {
		// グローバル変数の更新.
		bl_MSCDownloading = uc_Data;
	}

	return bl_Ret;
}

// ================================================================================================.
//! <!--機能--> MSCダウンロード実行状態をSPIフラッシュから読み出す.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param uchar*	ダウンロード実行情報(O).
//! <!--戻値-->.
//! \return Bool True：成功 / False：失敗.
//! <!--詳細-->.
//!  SPIフラッシュ搭載機種のみ使用可能。それ以外の機種では常にFalseを戻す.\n
//! <!--履歴-->.
//! 2014/02/25 yamada	新規作成 制御構成変更No.43 : NVRAMレス対応.
//!	2014/10/21 Sky O.Suzuki [CQ503465]静的解析対応.
//!	2017/05/25 AR1022537 Sky simogaki 【システム品証PreFC評価】【SYQA-IT5_4.1-001】SuperWarp起動しない.
// ================================================================================================.
Bool THRG_GetMSCDownloading(uchar *uc_Data)
{
	if (uc_Data == NULL) {
		printf( "[%s] Parameter error.\n", __FUNCTION__);
		return False;
	}

	uint32_t ul_ReadOffset = THRD_FlashOffset_SubsetSector+THRD_FlashOffset_MSCDownloading;
	Bool bl_Ret = THRF_readFlash(ul_ReadOffset, (void *)uc_Data, 1);
	if (bl_Ret == False) {
		//	14/10/21 Sky O.Suzuki [CQ503465]静的解析対応.
		printf( "[%s] SPI-Flash read error. offset = 0x%"PRIX32", size = %d\n", __FUNCTION__, ul_ReadOffset, THRD_FlashSize_Sector);
	} else {
		// MSCダウンロード実行状態としては0か1しか取りえないが、SPIFlashの状態によって0、1以外が返る可能性があるので、0x01でBitANDを取る.
		*uc_Data &= 0x01;
	}
	return bl_Ret;
}

// ================================================================================================.
//! <!--機能--> ENGダウンロード実行状態をSPIフラッシュに書き込み、グローバル変数を更新する.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param uchar	ダウンロード実行情報(I).
//! <!--戻値-->.
//! \return Bool True：成功 / False：失敗.
//! <!--詳細-->.
//!  SPIフラッシュ搭載機種のみ使用可能。それ以外の機種では常にFalseを戻す.\n
//! <!--履歴-->.
//! 2014/02/25 yamada	新規作成 制御構成変更No.43 : NVRAMレス対応.
//!	2015/07/13 Sky O.Suzuki IRC/ENGのDLフラグ対応.
// ================================================================================================.
Bool THRG_SetENGDownloading(uchar uc_Data)
{
	uc_Data = 0x00;
	Bool bl_Ret = setFlashDownloadingStatus(uc_Data, THRD_FlashOffset_ENGDownloading);
	if (bl_Ret != False) {
		// グローバル変数の更新.
		bl_ENGDownloading = uc_Data;
	}

	return bl_Ret;
}

// ================================================================================================.
//! <!--機能--> ENGダウンロード実行状態をSPIフラッシュから読み出す.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param uchar*	ダウンロード実行情報(O).
//! <!--戻値-->.
//! \return Bool True：成功 / False：失敗.
//! <!--詳細-->.
//!  SPIフラッシュ搭載機種のみ使用可能。それ以外の機種では常にFalseを戻す.\n
//! <!--履歴-->.
//! 2014/02/25 yamada	新規作成 制御構成変更No.43 : NVRAMレス対応.
//!	2014/10/21 Sky O.Suzuki [CQ503465]静的解析対応.
//!	2015/07/13 Sky O.Suzuki IRC/ENGのDLフラグ対応.
// ================================================================================================.
Bool THRG_GetENGDownloading(uchar *uc_Data)
{
	*uc_Data = 0x00;
	return True;
}

// ================================================================================================.
//! <!--機能--> IRCダウンロード実行状態をSPIフラッシュに書き込み、グローバル変数を更新する.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param uchar	ダウンロード実行情報(I).
//! <!--戻値-->.
//! \return Bool True：成功 / False：失敗.
//! <!--詳細-->.
//!  SPIフラッシュ搭載機種のみ使用可能。それ以外の機種では常にFalseを戻す.\n
//! <!--履歴-->.
//! 2014/02/25 yamada	新規作成 制御構成変更No.43 : NVRAMレス対応.
//!	2015/07/13 Sky O.Suzuki IRC/ENGのDLフラグ対応.
// ================================================================================================.
Bool THRG_SetIRCDownloading(uchar uc_Data)
{
	uc_Data = 0x00;
	Bool bl_Ret = setFlashDownloadingStatus(uc_Data, THRD_FlashOffset_IRCDownloading);
	if (bl_Ret != False) {
		// グローバル変数の更新.
		bl_IRCDownloading = uc_Data;
	}

	return bl_Ret;
}

// ================================================================================================.
//! <!--機能--> IRCダウンロード実行状態をSPIフラッシュから読み出す.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param uchar*	ダウンロード実行情報(O).
//! <!--戻値-->.
//! \return Bool True：成功 / False：失敗.
//! <!--詳細-->.
//!  SPIフラッシュ搭載機種のみ使用可能。それ以外の機種では常にFalseを戻す.\n
//! <!--履歴-->.
//! 2014/02/25 yamada	新規作成 制御構成変更No.43 : NVRAMレス対応.
//!	2014/10/21 Sky O.Suzuki [CQ503465]静的解析対応.
//!	2015/07/13 Sky O.Suzuki IRC/ENGのDLフラグ対応.
// ================================================================================================.
Bool THRG_GetIRCDownloading(uchar *uc_Data)
{
	*uc_Data = 0x00;
	return True;
}

// ================================================================================================.
//! <!--機能--> EDHダウンロード実行状態をSPIフラッシュに書き込み、グローバル変数を更新する.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param uchar	ダウンロード実行情報(I).
//! <!--戻値-->.
//! \return Bool True：成功 / False：失敗.
//! <!--詳細-->.
//!  SPIフラッシュ搭載機種のみ使用可能。それ以外の機種では常にFalseを戻す.\n
//! <!--履歴-->.
//! 2014/02/25 yamada	新規作成 制御構成変更No.43 : NVRAMレス対応.
// ================================================================================================.
Bool THRG_SetEDHDownloading(uchar uc_Data)
{
	Bool bl_Ret = setFlashDownloadingStatus(uc_Data, THRD_FlashOffset_EDHDownloading);
	if (bl_Ret != False) {
		// グローバル変数の更新.
		bl_EDHDownloading = uc_Data;
	}

	return bl_Ret;
}

// ================================================================================================.
//! <!--機能--> EDHダウンロード実行状態をSPIフラッシュから読み出す.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param uchar*	ダウンロード実行情報(O).
//! <!--戻値-->.
//! \return Bool True：成功 / False：失敗.
//! <!--詳細-->.
//!  SPIフラッシュ搭載機種のみ使用可能。それ以外の機種では常にFalseを戻す.\n
//! <!--履歴-->.
//! 2014/02/25 yamada	新規作成 制御構成変更No.43 : NVRAMレス対応.
//!	2014/10/21 Sky O.Suzuki [CQ503465]静的解析対応.
//!	2017/05/25 AR1022537 Sky simogaki 【システム品証PreFC評価】【SYQA-IT5_4.1-001】SuperWarp起動しない.
// ================================================================================================.
Bool THRG_GetEDHDownloading(uchar *uc_Data)
{
	if (uc_Data == NULL) {
		printf( "[%s] Parameter error.\n", __FUNCTION__);
		return False;
	}

	uint32_t ul_ReadOffset = THRD_FlashOffset_SubsetSector+THRD_FlashOffset_EDHDownloading;
	Bool bl_Ret = THRF_readFlash(ul_ReadOffset, (void *)uc_Data, 1);
	if (bl_Ret == False) {
		//	14/10/21 Sky O.Suzuki [CQ503465]静的解析対応.
		printf( "[%s] SPI-Flash read error. offset = 0x%"PRIX32", size = %d\n", __FUNCTION__, ul_ReadOffset, THRD_FlashSize_Sector);
	} else {
		// EDHダウンロード実行状態としては0か1しか取りえないが、SPIFlashの状態によって0、1以外が返る可能性があるので、0x01でBitANDを取る.
		*uc_Data &= 0x01;
	}

	return bl_Ret;
}

// ================================================================================================.
//! <!--機能--> FAXダウンロード実行状態をSPIフラッシュに書き込み、グローバル変数を更新する.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param uchar	ダウンロード実行情報(I).
//! <!--戻値-->.
//! \return Bool True：成功 / False：失敗.
//! <!--詳細-->.
//!  SPIフラッシュ搭載機種のみ使用可能。それ以外の機種では常にFalseを戻す.\n
//! <!--履歴-->.
//! 2014/02/25 yamada	新規作成 制御構成変更No.43 : NVRAMレス対応.
// ================================================================================================.
Bool THRG_SetFAXDownloading(uchar uc_Data)
{
	Bool bl_Ret = setFlashDownloadingStatus(uc_Data, THRD_FlashOffset_FAXDownloading);
	if (bl_Ret != False) {
		// グローバル変数の更新.
		bl_FAXDownloading = uc_Data;
	}

	return bl_Ret;
}

// ================================================================================================.
//! <!--機能--> FAXダウンロード実行状態をSPIフラッシュから読み出す.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param uchar*	ダウンロード実行情報(O).
//! <!--戻値-->.
//! \return Bool True：成功 / False：失敗.
//! <!--詳細-->.
//!  SPIフラッシュ搭載機種のみ使用可能。それ以外の機種では常にFalseを戻す.\n
//! <!--履歴-->.
//! 2014/02/25 yamada	新規作成 制御構成変更No.43 : NVRAMレス対応.
//!	2014/10/21 Sky O.Suzuki [CQ503465]静的解析対応.
//!	2017/05/25 AR1022537 Sky simogaki 【システム品証PreFC評価】【SYQA-IT5_4.1-001】SuperWarp起動しない.
// ================================================================================================.
Bool THRG_GetFAXDownloading(uchar *uc_Data)
{
	if (uc_Data == NULL) {
		printf( "[%s] Parameter error.\n", __FUNCTION__);
		return False;
	}

	uint32_t ul_ReadOffset = THRD_FlashOffset_SubsetSector+THRD_FlashOffset_FAXDownloading;
	Bool bl_Ret = THRF_readFlash(ul_ReadOffset, (void *)uc_Data, 1);
	if (bl_Ret == False) {
		//	14/10/21 Sky O.Suzuki [CQ503465]静的解析対応.
		printf( "[%s] SPI-Flash read error. offset = 0x%"PRIX32", size = %d\n", __FUNCTION__, ul_ReadOffset, THRD_FlashSize_Sector);
	} else {
		// FaxはFax1-Fax4を一つのフラグでBit制御しているので、すべてのBitを確認する.
		*uc_Data &= THRD_DownloadingStatus_FAX_ALL;
	}

	return bl_Ret;
}

// ================================================================================================.
//! <!--機能--> MSCSubsetダウンロード実行状態をSPIフラッシュに書き込み、グローバル変数を更新する.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param uchar	ダウンロード実行情報(I).
//! <!--戻値-->.
//! \return Bool True：成功 / False：失敗.
//! <!--詳細-->.
//!  SPIフラッシュ搭載機種のみ使用可能。それ以外の機種では常にFalseを戻す.\n
//! <!--履歴-->.
//! 2014/02/25 yamada	新規作成 制御構成変更No.43 : NVRAMレス対応.
// ================================================================================================.
Bool THRG_SetMSCSubsetDownloading(uchar uc_Data)
{
	Bool bl_Ret = setFlashDownloadingStatus(uc_Data, THRD_FlashOffset_MSCSubsetDownloading);
	if (bl_Ret != False) {
		// グローバル変数の更新.
		bl_MSCSubsetDownloading = uc_Data;
	}

	return bl_Ret;
}

// ================================================================================================.
//! <!--機能--> MSCSubsetダウンロード実行状態をSPIフラッシュから読み出す.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param uchar*	ダウンロード実行情報(O).
//! <!--戻値-->.
//! \return Bool True：成功 / False：失敗.
//! <!--詳細-->.
//!  SPIフラッシュ搭載機種のみ使用可能。それ以外の機種では常にFalseを戻す.\n
//! <!--履歴-->.
//! 2014/02/25 yamada	新規作成 制御構成変更No.43 : NVRAMレス対応.
//!	2014/10/21 Sky O.Suzuki [CQ503465]静的解析対応.
//!	2017/05/25 AR1022537 Sky simogaki 【システム品証PreFC評価】【SYQA-IT5_4.1-001】SuperWarp起動しない.
// ================================================================================================.
Bool THRG_GetMSCSubsetDownloading(uchar *uc_Data)
{
	if (uc_Data == NULL) {
		printf( "[%s] Parameter error.\n", __FUNCTION__);
		return False;
	}

	uint32_t ul_ReadOffset = THRD_FlashOffset_SubsetSector+THRD_FlashOffset_MSCSubsetDownloading;
	Bool bl_Ret = THRF_readFlash(ul_ReadOffset, (void *)uc_Data, 1);
	if (bl_Ret == False) {
		//	14/10/21 Sky O.Suzuki [CQ503465]静的解析対応.
		printf( "[%s] SPI-Flash read error. offset = 0x%"PRIX32", size = %d\n", __FUNCTION__, ul_ReadOffset, THRD_FlashSize_Sector);
	} else {
		// MSCSubSetダウンロード実行状態としては0か1しか取りえないが、SPIFlashの状態によって0、1以外が返る可能性があるので、0x01でBitANDを取る.
		*uc_Data &= 0x01;
	}

	return bl_Ret;
}

// ================================================================================================.
//! <!--機能--> MCPFダウンロード実行状態をSPIフラッシュに書き込み、グローバル変数を更新する.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param uchar	ダウンロード実行情報(I).
//! <!--戻値-->.
//! \return Bool True：成功 / False：失敗.
//! <!--詳細-->.
//!  SPIフラッシュ搭載機種のみ使用可能。それ以外の機種では常にFalseを戻す.\n
//! <!--履歴-->.
//! 2014/02/25 yamada	新規作成 制御構成変更No.43 : NVRAMレス対応.
// ================================================================================================.
Bool THRG_SetMCPFDownloading(uchar uc_Data)
{
	Bool bl_Ret = setFlashDownloadingStatus(uc_Data, THRD_FlashOffset_MCPFDownloading);
	if (bl_Ret != False) {
		// グローバル変数の更新.
		bl_MCPFDownloading = uc_Data;
	}

	return bl_Ret;
}

// ================================================================================================.
//! <!--機能--> MCPFダウンロード実行状態をSPIフラッシュから読み出す.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param uchar*	ダウンロード実行情報(O).
//! <!--戻値-->.
//! \return Bool True：成功 / False：失敗.
//! <!--詳細-->.
//!  SPIフラッシュ搭載機種のみ使用可能。それ以外の機種では常にFalseを戻す.\n
//! <!--履歴-->.
//! 2014/02/25 yamada	新規作成 制御構成変更No.43 : NVRAMレス対応.
//!	2014/10/21 Sky O.Suzuki [CQ503465]静的解析対応.
//!	2017/05/25 AR1022537 Sky simogaki 【システム品証PreFC評価】【SYQA-IT5_4.1-001】SuperWarp起動しない.
// ================================================================================================.
Bool THRG_GetMCPFDownloading(uchar *uc_Data)
{
	if (uc_Data == NULL) {
		printf( "[%s] Parameter error.\n", __FUNCTION__);
		return False;
	}

	uint32_t ul_ReadOffset = THRD_FlashOffset_SubsetSector+THRD_FlashOffset_MCPFDownloading;
	Bool bl_Ret = THRF_readFlash(ul_ReadOffset, (void *)uc_Data, 1);
	if (bl_Ret == False) {
		//	14/10/21 Sky O.Suzuki [CQ503465]静的解析対応.
		printf( "[%s] SPI-Flash read error. offset = 0x%"PRIX32", size = %d\n", __FUNCTION__, ul_ReadOffset, THRD_FlashSize_Sector);
	} else {
		// MCPFダウンロード実行状態としては0か1しか取りえないが、SPIFlashの状態によって0、1以外が返る可能性があるので、0x01でBitANDを取る.
		*uc_Data &= 0x01;
	}

	return bl_Ret;
}

// ================================================================================================.
//! <!--機能--> 起動種別情報をSPIフラッシュに書き込み、グローバル変数を更新する.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param uchar	ダウンロード実行情報(I).
//! <!--戻値-->.
//! \return Bool True：成功 / False：失敗.
//! <!--詳細-->.
//!  SPIフラッシュ搭載機種のみ使用可能。それ以外の機種では常にFalseを戻す.\n
//! <!--履歴-->.
//! 2014/02/25 yamada	新規作成 制御構成変更No.43 : NVRAMレス対応.
// ================================================================================================.
Bool THRG_SetNWDLExec(uchar uc_Data)
{
	uchar uc_Ret = THRG_setSoftDipSw(THRD_FlashOffset_NWDLExec, (void *)&uc_Data, 1);
	if (uc_Ret != THRD_SET_SPI_OK) {
		printf( "[%s] SPI-Flash NWDLExec write error(%d). write data(%d).\n", __FUNCTION__, uc_Ret, uc_Data);
		return False;
	}

	return True;
}

// ================================================================================================.
//! <!--機能--> 起動種別情報をSPIフラッシュから読み出す.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param uchar*	ダウンロード実行情報(O).
//! <!--戻値-->.
//! \return Bool True：成功 / False：失敗.
//! <!--詳細-->.
//!  SPIフラッシュ搭載機種のみ使用可能。それ以外の機種では常にFalseを戻す.\n
//! <!--履歴-->.
//! 2014/02/25 yamada	新規作成 制御構成変更No.43 : NVRAMレス対応.
//  2014/08/13 yamada	CQ502406 IISW出来ない問題対応.
// ================================================================================================.
Bool THRG_GetNWDLExec(uchar *uc_Data)
{
	if (uc_Data == NULL) {
		printf( "[%s] Parameter error.\n", __FUNCTION__);
		return False;
	}

	uchar uc_Ret = THRG_getSoftDipSw(THRD_FlashOffset_NWDLExec, (void *)uc_Data, 1);
	if ((uc_Ret != THRD_GET_SPI_OK) &&
		(uc_Ret != THRD_GET_SPI_OK_AREA1_ERR) &&
		(uc_Ret != THRD_GET_SPI_OK_AREA2_ERR)) {
		printf( "[%s] SPI-Flash NWDLExec read error(%d).\n", __FUNCTION__, uc_Ret);
		return False;
	}

	return True;
}

// ================================================================================================.
//! <!--機能--> SPIフラッシュのダウンロードステータスフラグ情報を、グローバル変数に設定する.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return Bool True：成功 / False：失敗.
//! <!--詳細-->.
//!  本API は、ダウンロードステータス情報を参照する前に実行しておく必要がある.\n
//!  SPIフラッシュ搭載機種のみ使用可能。それ以外の機種では常にFalseを戻す.\n
//! <!--履歴-->.
//! 2014/02/25 yamada	新規作成 制御構成変更No.43 : NVRAMレス対応.
//!	2015/07/13 Sky O.Suzuki IRC/ENGのDLフラグ対応.
// ================================================================================================.
Bool THRG_InitDownloadVariable()
{
	// サブセット領域セクタを読み出し、変数に設定を行う.
	memset(uc_FlashWorkBuf, 0x00, THRD_FlashSize_Sector);

	Bool bl_Ret = THRF_readFlash(THRD_FlashOffset_SubsetSector, (void *)uc_FlashWorkBuf, THRD_FlashSize_SubsetArea);
	if (bl_Ret == False) {
		printf( "[%s] SPI-Flash read error. offset = 0x%lX, size = %ld\n", __FUNCTION__, THRD_FlashOffset_SubsetSector, THRD_FlashSize_SubsetArea);
		return False;
	}

	uc_FlashWorkBuf[THRD_FlashOffset_ENGDownloading] = 0x00;
	uc_FlashWorkBuf[THRD_FlashOffset_IRCDownloading] = 0x00;


	// ダウンロードフラグの設定.
	bl_MSCDownloading		= uc_FlashWorkBuf[THRD_FlashOffset_MSCDownloading];
	bl_ENGDownloading		= uc_FlashWorkBuf[THRD_FlashOffset_ENGDownloading];
	bl_IRCDownloading		= uc_FlashWorkBuf[THRD_FlashOffset_IRCDownloading];
	bl_EDHDownloading		= uc_FlashWorkBuf[THRD_FlashOffset_EDHDownloading];
	bl_FAXDownloading		= uc_FlashWorkBuf[THRD_FlashOffset_FAXDownloading];
	bl_MSCSubsetDownloading	= uc_FlashWorkBuf[THRD_FlashOffset_MSCSubsetDownloading];
	bl_MCPFDownloading		= uc_FlashWorkBuf[THRD_FlashOffset_MCPFDownloading];

	return True;
}

// ================================================================================================.
//! <!--機能--> SPIフラッシュに設定されているダウンロードステータス情報を取得し、セクタイレースを行う.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return Bool True：成功 / False：失敗.
//! <!--詳細-->.
//!  同セクタ内のサブセット使用領域以外のデータについては、即時書き戻しを行う.\n
//!  但し、イレース中(書き戻し前)の電源断が発生した場合については、データの保障は出来ないものとする.\n
//!  SPIフラッシュ搭載機種のみ使用可能。それ以外の機種では常にFalseを戻す.\n
//! <!--履歴-->.
//! 2014/02/25 yamada	新規作成 制御構成変更No.43 : NVRAMレス対応.
// ================================================================================================.
Bool THRG_InitDownloadStatus()
{
	memset(uc_FlashWorkBuf, 0x00, THRD_FlashSize_Sector);
	memset(uc_FlashDownloadingBuf, 0x00, THRD_FlashSize_SubsetArea);

	// サブセット領域セクタをSPI-Flashから読み出し.
	Bool bl_Ret = THRF_readFlash(THRD_FlashOffset_SubsetSector, (void *)uc_FlashWorkBuf, THRD_FlashSize_Sector);
	if (bl_Ret == False) {
		printf( "[%s] SPI-Flash read sector error. offset = 0x%lX, size = %ld\n", __FUNCTION__, THRD_FlashOffset_SubsetSector, THRD_FlashSize_Sector);
		return False;
	}

	// 読み出したダウンロード情報を保持.
	memcpy(uc_FlashDownloadingBuf, uc_FlashWorkBuf, THRD_FlashSize_SubsetArea);

	// サブセット領域セクタをセクタイレース.
	bl_Ret = THRF_eraseFlash(THRD_FlashOffset_SubsetSector, THRD_FlashSize_Sector);
	if (bl_Ret == False) {
		printf( "[%s] SPI-Flash sector erase error. offset = 0x%lX, size = %ld\n", __FUNCTION__, THRD_FlashOffset_SubsetSector, THRD_FlashSize_Sector);
		return False;
	}

	// サブセット領域(先頭512byte)以外のデータを書き戻し.
	uint32_t ul_WriteOffset = THRD_FlashOffset_SubsetSector + THRD_FlashSize_SubsetArea;
	uint32_t ul_WriteSize = THRD_FlashSize_Sector - THRD_FlashSize_SubsetArea;

	bl_Ret = THRF_writeFlash(ul_WriteOffset, (void *)&uc_FlashWorkBuf[THRD_FlashSize_SubsetArea], ul_WriteSize);
	if (bl_Ret == False) {
		printf( "[%s] SPI-Flash write error. offset = 0x%"PRIX32", size = %"PRId32"\n", __FUNCTION__, ul_WriteOffset, ul_WriteSize);
		return False;
	}

	// べリファイの実行.
	bl_Ret = verifyFlashSubsetData((void *)&uc_FlashWorkBuf[THRD_FlashSize_SubsetArea], ul_WriteOffset, ul_WriteSize);
	if (bl_Ret == False) {
		printf( "[%s] SPI-Flash verify error. offset = 0x%"PRIX32", size = %"PRId32"\n", __FUNCTION__, ul_WriteOffset, ul_WriteSize);
		return False;
	}

	return True;
}

// ================================================================================================.
//! <!--機能--> SPIフラッシュのダウンロードステータスフラグ情報領域にダウンロード実行状態を書き込む.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return Bool True：成功 / False：失敗.
//! <!--詳細-->.
//!  本API を使用する前に、THRG_InitDownloadStatus()にて、ダウンロードステータス情報領域を初期化しておく必要がある.\n
//!  初期化せずに使用した場合の動作は保障されないものとする.\n
//!  SPIフラッシュ搭載機種のみ使用可能。それ以外の機種では常にFalseを戻す.\n
//! <!--履歴-->.
//! 2014/02/25 yamada	新規作成 制御構成変更No.43 : NVRAMレス対応.
//!	2015/07/13 Sky O.Suzuki IRC/ENGのDLフラグ対応.
// ================================================================================================.
Bool THRG_WriteDownloadStatus()
{
	// サブセット領域を書き込み.
	Bool bl_Ret = THRF_writeFlash(THRD_FlashOffset_SubsetSector, (void *)uc_FlashDownloadingBuf, THRD_FlashSize_SubsetArea);
	if (bl_Ret == False) {
		printf( "[%s] SPI-Flash write error. offset = 0x%lX, size = %ld\n", __FUNCTION__, THRD_FlashOffset_SubsetSector, THRD_FlashSize_SubsetArea);
		return False;
	}

	// べリファイの実行.
	bl_Ret = verifyFlashSubsetData((void *)uc_FlashDownloadingBuf, THRD_FlashOffset_SubsetSector, THRD_FlashSize_SubsetArea);
	if (bl_Ret == False) {
		printf( "[%s] SPI-Flash verify error. offset = 0x%lX, size = %ld\n", __FUNCTION__, THRD_FlashOffset_SubsetSector, THRD_FlashSize_SubsetArea);
		return False;
	}

	uc_FlashDownloadingBuf[THRD_FlashOffset_ENGDownloading] = 0x00;
	uc_FlashDownloadingBuf[THRD_FlashOffset_IRCDownloading] = 0x00;

	// ダウンロードフラグの設定.
	bl_MSCDownloading		= uc_FlashDownloadingBuf[THRD_FlashOffset_MSCDownloading];
	bl_ENGDownloading		= uc_FlashDownloadingBuf[THRD_FlashOffset_ENGDownloading];
	bl_IRCDownloading		= uc_FlashDownloadingBuf[THRD_FlashOffset_IRCDownloading];
	bl_EDHDownloading		= uc_FlashDownloadingBuf[THRD_FlashOffset_EDHDownloading];
	bl_FAXDownloading		= uc_FlashDownloadingBuf[THRD_FlashOffset_FAXDownloading];
	bl_MSCSubsetDownloading	= uc_FlashDownloadingBuf[THRD_FlashOffset_MSCSubsetDownloading];
	bl_MCPFDownloading		= uc_FlashDownloadingBuf[THRD_FlashOffset_MCPFDownloading];

	return True;
}

// ================================================================================================.
//! <!--機能--> SPIフラッシュデータの整合性チェックを行う.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return uchar	THRD_FlashCheck_OK(0)		 : SPI-Flashのデータ正常.
//! 				THRD_FlashCheck_NG(1)		 : SPI-Flashのデータ異常.
//! 				THRD_FlashCheck_AccessNG(2)	 : SPI-Flashのアクセスエラー.
//! <!--詳細-->.
//!  SPIフラッシュ搭載機種のみ使用可能。それ以外の機種では常にFalseを戻す.
//! <!--履歴-->.
//! 2014/02/25 yamada	新規作成 制御構成変更No.43 : NVRAMレス対応.
// ================================================================================================.
uchar THRG_CheckFlashSubsetData()
{
	// サブセット領域の読み出し.
	Bool bl_Ret = THRF_readFlash(THRD_FlashOffset_SubsetSector, (void *)uc_FlashWorkBuf, THRD_FlashSize_SubsetArea);
	if (bl_Ret == False) {
		printf( "[%s] SPI-Flash read error. offset = 0x%lX, size = %ld\n", __FUNCTION__, THRD_FlashOffset_SubsetSector, THRD_FlashSize_SubsetArea);
		return THRD_FlashCheck_AccessNG;
	}

	// SPI-Flashのデータが不正でないか、未使用領域をチェックする.
	for (uint32_t ul_Cnt = THRD_FlashOffset_UnuseArea; ul_Cnt < THRD_FlashSize_SubsetArea; ul_Cnt++) {
		// 未使用領域はセクタイレース後、値を設定しないため 0xFF が設定されている.
		// 従い、0xFF でない場合を不正データと判断する.
		if (uc_FlashWorkBuf[ul_Cnt] != 0xFF) {
		printf( "[%s] SPI-Flash Check Subset area. abnormal data(%d). index(%"PRId32")\n", __FUNCTION__, uc_FlashWorkBuf[ul_Cnt], ul_Cnt);
			return THRD_FlashCheck_NG;
		}
	}

	return THRD_FlashCheck_OK;
}


/*================================================================================================
［機能］	電源断保存領域を取得する.
--------------------------------------------------------------------------------------------------
［引数］	なし
［戻値］	Kernelで確保した4KBの領域へのポインタ
［履歴］	2014/02/17 horii NVRAMレス対応 SPI-Flashアクセス関数新規作成.
			2014/11/04 ft_kondo Poseidon 静的解析対応 ID:19456. 
================================================================================================*/
#define PMU_IOC_MAGIC 0x82
#define PMU_IOCSGET_POWERDOWN_DATAAREA		_IOW( PMU_IOC_MAGIC, 7, uint64_t )
void* THRF_getPowerDownRegion( void )
{
	static Bool bl_First = True;		// 初回アクセスフラグ初期化（True:初回アクセス False:2回目以降アクセス）.
	static void* vp_datav = NULL;		// データ格納先(仮想). 

	int32_t fd = 0, ret = 0;

	off_t ull_datap = 0;	// データ格納先(物理). 
	off_t ul_Offset = 0;
	uint32_t ul_Length = 0;
	off_t ui_Poff = 0;



	if( bl_First ){
		// 初回のみ、領域を確保する.
		bl_First = False;
		fd = SYSF_devopen( "/dev/PMU0", O_RDWR, 0666 );
		if( fd < 0 ){
			printf("%s: SYSF_devopen ERR errno:%d\n", __FUNCTION__, SYSF_errnoGet());
			return NULL;
		}
		ret = SYSF_ioctl(fd, PMU_IOCSGET_POWERDOWN_DATAAREA, (intptr_t)&ull_datap);
		if( ret < 0 ){
			printf("%s: SYSF_ioctl ERR errno:%d\n", __FUNCTION__, SYSF_errnoGet());
			SYSF_close(fd);
			return NULL;
		}
		SYSF_close(fd);
		
		fd = SYSF_open( "/dev/mem", O_RDWR, 0666 );
		if( fd < 0 ){
			printf("%s: SYSF_open ERR errno:%d\n", __FUNCTION__, SYSF_errnoGet());
			return NULL;
		}


		ui_Poff = ull_datap % (4 * 1024);
		ul_Offset = ull_datap - ui_Poff;
		ul_Length = ( 4 * 1024 ) + ui_Poff;


		vp_datav = mmap( 0, ul_Length, PROT_READ|PROT_WRITE, MAP_SHARED, fd, ul_Offset );
		if(vp_datav == MAP_FAILED){
			printf("%s: mmap ERR errno:%d\n", __FUNCTION__, SYSF_errnoGet());
			SYSF_close(fd);
			return NULL;
		}
		SYSF_close(fd);
	}
	return vp_datav;
}

/*================================================================================================
［機能］	電源断保存領域をリードする.
--------------------------------------------------------------------------------------------------
［引数］	void* data	:リードしたデータの格納先.
［戻値］	正常終了:読み込んだサイズ データ無し:0 エラー:負
［履歴］	2014/02/17 horii NVRAMレス対応 SPI-Flashアクセス関数新規作成.
			2015/04/13 okuma ワーニング対応
================================================================================================*/
int32_t THRF_readPowerDownRegion( void* data )
{
	Bool ret = False;
	struct readdata
	{
		union
		{
			uchar data[4];
			uint32_t size[1];
		};
	};
	
	readdata r_data;
	
	ret = THRF_readFlash( POWERDOWN_REGION, r_data.data, sizeof(r_data));
	
	if( ret == False ){
		printf("%s: THRF_readPowerDownRegion ERR\n", __FUNCTION__);
		return -1;
	}else if( ( *r_data.size == 0 ) || ( *r_data.size > ( 4 * 1024 ) ) ){
		return 0;
	}else{
		ret = THRF_readFlash( POWERDOWN_REGION, data, *r_data.size + 4);
		if( ret == False ){
			printf("%s: THRF_readPowerDownRegion ERR\n", __FUNCTION__);
			return -1;
		}
	}
	return *r_data.size;
}

/*================================================================================================
［機能］	電源断保存領域をイレースする.
--------------------------------------------------------------------------------------------------
［引数］	なし
［戻値］	True:正常終了 False:エラー
［履歴］	2014/02/17 horii NVRAMレス対応 SPI-Flashアクセス関数新規作成.
================================================================================================*/
Bool THRF_erasePowerDownRegion( void )
{
	Bool ret = 0;
	ret = THRF_eraseFlash( POWERDOWN_REGION, 4 * 1024);
	return ret;
}

/*================================================================================================
［機能］	電源断保存領域をライトする.
--------------------------------------------------------------------------------------------------
［引数］	なし
［戻値］	True:正常終了 False:エラー
［履歴］	2014/02/17 horii NVRAMレス対応 SPI-Flashアクセス関数新規作成(デバッグ用).
================================================================================================*/
Bool THRF_writePowerDownRegion( void *data )
{
	Bool ret = 0;
	uint32_t size = *(uint32_t *)data;
	if( ( size == 0 ) || ( size > (256 * 7) - 4 )){
		return False;
	}else {
		ret = THRF_writeFlash( POWERDOWN_REGION, data, size + 4);
		return ret;
	}
}
#define EEPROMDATASIZE 128
#define I2CSMBUSDATABLOCKSIZE 32
#ifdef DEF_Helios_PF
/*================================================================================================
［機能］	EEPROMの情報をSPI Flashにコピーする.
			EEPROMの情報、書き込み後のSPI Flashの情報を表示する.
--------------------------------------------------------------------------------------------------
［引数］	なし.
［戻値］	なし
［履歴］	2014/04/25  FT_yamaguchi EEPROMデータコピーコマンド新規作成 制御構成変更_No.210 EEPROMレス対応.
			2014/06/18  FT_yamaguchi EEPROMの情報をEEPROMドライバーを使用してアクセスするように変更.
			2014/10/29	Sky小倉		 EEPROMレス対応.
================================================================================================*/
extern "C" void EepromDataCopyToSpiFlash()
{

	FILE* m_fp;
	
	m_fp = SYSF_fopen("/sys/bus/i2c/drivers/eeprom/0-0050/eeprom", "r");
	if( m_fp == NULL ) {
		printf( "EepromDataCopyToSpiFlash: can`t open sysfs eeprom.\n" );
		return ;
	}
	
	uchar eepromdata[EEPROMDATASIZE];
	memset( eepromdata , 0 , sizeof(eepromdata) );
	
	fread(eepromdata, sizeof(char), EEPROMDATASIZE, m_fp);
	
	// 取り出したEEPROMの情報を表示.
	int32_t i;
	printf("eepromdate :\n");
	for( i=0; i<EEPROMDATASIZE; i++ ){
		printf("%02x  " ,eepromdata[i] );
		if(15 == i % 16){
			printf("\n");
		}
	}
	printf("\n");
	SYSF_fclose(m_fp);
	
	// SPI Flashに情報を書き込む.
	unsigned char uc_Ret;
	uc_Ret = THRG_setMachineInfo( 0, eepromdata, sizeof(eepromdata) );
	if(uc_Ret != 0){
		printf("%s: Copy NG ERROR Info %d\n" , __FUNCTION__ , uc_Ret );
	 	return ;
	}
	printf("%s: Copy OK\n" , __FUNCTION__ );
	
	// SPI Flashの情報を取得.
	uchar sc_SpiFlashData[EEPROMDATASIZE];
	memset(sc_SpiFlashData, 0 , sizeof(sc_SpiFlashData) );
	
	uc_Ret = THRG_getMachineInfo( 0, sc_SpiFlashData, sizeof(sc_SpiFlashData) );
	if( THRD_GET_SPI_NG_PARAM == uc_Ret 
	 && THRD_GET_SPI_NG_ALL_AREA == uc_Ret 
	 && THRD_GET_SPI_NG_READ_ERR == uc_Ret ){
	 	printf( " SPI-FLASH_read ERROR Info %d\n" , uc_Ret );
	 	return ;
	 }
	// 書き込んだSPI Flashの情報を表示.
	printf("sc_SpiFlashData :\n");
	for( i=0; i<EEPROMDATASIZE; i++ ){
		printf("%02x  " ,sc_SpiFlashData[i] );
		if(15 == i % 16){
			printf("\n");
		}
	}
}

extern "C" void readSpiFlashEeprom()
{
	int32_t i;
	// SPI Flashの情報を取得.
	uchar sc_SpiFlashData[EEPROMDATASIZE];
	memset(sc_SpiFlashData, 0 , sizeof(sc_SpiFlashData) );
	unsigned char uc_Ret;
	uc_Ret = THRG_getMachineInfo( 0, sc_SpiFlashData, sizeof(sc_SpiFlashData) );
	if( THRD_GET_SPI_NG_PARAM == uc_Ret 
	 && THRD_GET_SPI_NG_ALL_AREA == uc_Ret 
	 && THRD_GET_SPI_NG_READ_ERR == uc_Ret ){
	 	printf( " SPI-FLASH_read ERROR Info %d\n" , uc_Ret );
	 	return ;
	 }
	// 書き込んだSPI Flashの情報を表示.
	printf("sc_SpiFlashData :\n");
	for( i=0; i<EEPROMDATASIZE; i++ ){
		printf("%02x  " ,sc_SpiFlashData[i] );
		if(15 == i % 16){
			printf("\n");
		}
	}
	
}

#endif


#ifdef DEF_A4_Cus				//	[CQ521427]	スキャナ調整値保存先追加対応.
//#define THRD_SyncScannerAdjust_DebugLog

#define THRD_IriAdjust_Crc32Size					( sizeof(uint) )

#define THRD_SyncScannerAdjust_Size					( 256 )
#define THRD_SyncScannerAdjust_Offset				( EEPROMDATASIZE )

#define THRD_SyncIrFactoryAdjust_Size				( 1024 )
#define THRD_SyncIrFactoryAdjust_Offset				( THRD_SyncScannerAdjust_Offset + THRD_SyncScannerAdjust_Size )

#define THRD_SyncAdjustType_Scanner					( 0 )
#define THRD_SyncAdjustType_IrFactory				( 1 )

typedef int (*THRF_FuncGetEmmc)( ushort*, uint* );
typedef int (*THRF_FuncSetEmmc)( ushort* );
typedef void (*THRF_FuncEnaCrc32Change)();
typedef void (*THRF_FuncWriteCrc32)();

typedef struct {
	uint							ui_Offset;
	uint							ui_Size;
	THRF_FuncGetEmmc				fp_GetEmmc;
	THRF_FuncSetEmmc				fp_SetEmmc;
	THRF_FuncEnaCrc32Change			fp_EnaCrc32Chg;
	THRF_FuncWriteCrc32				fp_SetCrc32;
} THRS_SyncAdjust;

static int syncAdjustCommon( const THRS_SyncAdjust *pst_SyncAdjust );
static int getScannerAdjustFromEmmc( ushort *pus_Buf, uint *pui_DataSize );
static int setScannerAdjustToEmmc( ushort *pus_Buf );
static void enaScannerAdjustCrc32Change( void );
static void setScannerAdjustCrc32( void );
static int getIrFactoryAdjustFromEmmc( ushort *pus_Buf, uint *pui_DataSize );
static int setIrFactoryAdjustToEmmc( ushort *pus_Buf );
static void enaIrFactoryAdjustCrc32Change( void );
static void setIrFactoryAdjustCrc32( void );
static uint makeIriAdjustCrc32( ushort *pus_Buf, uint ui_Size );
static void printAdjustCommon( const THRS_SyncAdjust *pst_SyncAdjust );
static void dumpShortArea( ushort *pus_Buf, uint ui_Size );

const THRS_SyncAdjust st_SyncAdjust[] = {
	{
		THRD_SyncScannerAdjust_Offset,			// ui_Offset
		THRD_SyncScannerAdjust_Size,			// ui_Size
		getScannerAdjustFromEmmc,				// fp_GetEmmc
		setScannerAdjustToEmmc,					// fp_SetEmmc
		enaScannerAdjustCrc32Change,			// fp_EnaCrc32Chg
		setScannerAdjustCrc32,					// fp_SetCrc32
	},
	{
		THRD_SyncIrFactoryAdjust_Offset,		// ui_Offset
		THRD_SyncIrFactoryAdjust_Size,			// ui_Size
		getIrFactoryAdjustFromEmmc,				// fp_GetEmmc
		setIrFactoryAdjustToEmmc,				// fp_SetEmmc
		enaIrFactoryAdjustCrc32Change,			// fp_EnaCrc32Chg
		setIrFactoryAdjustCrc32,				// fp_SetCrc32
	}
};


/*================================================================================================
［機能］	スキャナ調整値の同期処理.
			eMMC(SSD)、SPI-Flashからデータを取得、比較し、異常の有無をチェックする。.
			片方のメディアに異常データが存在する場合は、他方の正常なデータに書き戻す。.
--------------------------------------------------------------------------------------------------
［引数］	なし
［戻値］	0		:	OK.
			-1		:	THRD_SyncScannerAdjust_MallocError.
			-2		:	THRD_SyncScannerAdjust_FlashReadError.
			-3		:	THRD_SyncScannerAdjust_FlashWriteError.
			-4		:	THRD_SyncScannerAdjust_DataError.
			-5		:	THRD_SyncScannerAdjust_ParameterError.
［履歴］	2016/01/08	es_igarashi	[CQ521427]	スキャナ調整値保存先追加対応.
================================================================================================*/
int THRG_SyncScannerAdjust( void )
{
	int		i_Ret = OK;

	i_Ret = syncAdjustCommon( &st_SyncAdjust[THRD_SyncAdjustType_Scanner] );
	if ( i_Ret ){
		printf( "$$$$ %s : i_Ret=%d.\n", __func__, i_Ret );
	}

	return ( i_Ret );
}


/*================================================================================================
［機能］	スキャナ調整値の同期処理.
			eMMC(SSD)、SPI-Flashからデータを取得、比較し、異常の有無をチェックする。.
			片方のメディアに異常データが存在する場合は、他方の正常なデータに書き戻す。.
--------------------------------------------------------------------------------------------------
［引数］	BOOL		b_WriteEmmc		eMMC書き込み要否(true：必要、false：不要).
［戻値］	0		:	OK.
			-1		:	THRD_SyncScannerAdjust_MallocError.
			-2		:	THRD_SyncScannerAdjust_FlashReadError.
			-3		:	THRD_SyncScannerAdjust_FlashWriteError.
			-4		:	THRD_SyncScannerAdjust_DataError.
			-5		:	THRD_SyncScannerAdjust_ParameterError.
［履歴］	2016/01/08	es_igarashi	[CQ521427]	スキャナ調整値保存先追加対応.
================================================================================================*/
int THRG_SyncScannerAdjust( Bool b_WriteEmmc )
{
	int		i_Ret = OK;

	i_Ret = syncAdjustCommon( &st_SyncAdjust[THRD_SyncAdjustType_Scanner] );
	if ( i_Ret ){
		printf( "$$$$ %s : i_Ret=%d.\n", __func__, i_Ret );
	}

	return ( i_Ret );
}


/*================================================================================================
［機能］	IR調整値の同期処理.
			eMMC(SSD)、SPI-Flashからデータを取得、比較し、異常の有無をチェックする。.
			片方のメディアに異常データが存在する場合は、他方の正常なデータに書き戻す。.
--------------------------------------------------------------------------------------------------
［引数］	なし
［戻値］	0		:	OK.
			-1		:	THRD_SyncScannerAdjust_MallocError.
			-2		:	THRD_SyncScannerAdjust_FlashReadError.
			-3		:	THRD_SyncScannerAdjust_FlashWriteError.
			-4		:	THRD_SyncScannerAdjust_DataError.
			-5		:	THRD_SyncScannerAdjust_ParameterError.
［履歴］	2016/01/08	es_igarashi	[CQ521427]	スキャナ調整値保存先追加対応.
================================================================================================*/
int THRG_SyncIrFactoryAdjust( void )
{
	int		i_Ret = OK;

	i_Ret = syncAdjustCommon( &st_SyncAdjust[THRD_SyncAdjustType_IrFactory] );
	if ( i_Ret ){
		printf( "$$$$ %s : i_Ret=%d.\n", __func__, i_Ret );
	}

	return ( i_Ret );
}


/*================================================================================================
［機能］	調整値の同期処理(共通).
			eMMC(SSD)、SPI-Flashからデータを取得、比較し、異常の有無をチェックする。.
			片方のメディアに異常データが存在する場合は、他方の正常なデータに書き戻す。.
--------------------------------------------------------------------------------------------------
［引数］	THRS_SyncAdjust*	pst_SyncAdjust		調整値判定用テーブルポインタ.
［戻値］	0		:	OK.
			-1		:	THRD_SyncScannerAdjust_MallocError.
			-2		:	THRD_SyncScannerAdjust_FlashReadError.
			-3		:	THRD_SyncScannerAdjust_FlashWriteError.
			-4		:	THRD_SyncScannerAdjust_DataError.
			-5		:	THRD_SyncScannerAdjust_ParameterError.
［履歴］	2016/01/08	es_igarashi	[CQ521427]	スキャナ調整値保存先追加対応.
================================================================================================*/
static int syncAdjustCommon( const THRS_SyncAdjust *pst_SyncAdjust )
{
	int			i_Ret			= OK;
	uint		ui_DataSize		= 0;
	uint		ui_EmmcCrc32	= 0;
	uint		ui_FlashCrc32	= 0;
	uint		ui_TempCrc32	= 0;
	bool		b_EmmcOK		= false;
	bool		b_FlashOK		= false;
	bool		b_EmmcWrite		= false;
	ushort		*pus_Emmc		= NULL;
	uchar		*puc_Flash		= NULL;
	ushort		*pus_EmmcData	= NULL;
	uchar		*puc_FlashData	= NULL;

	if ( pst_SyncAdjust == NULL ){
		printf( "$$$$ %s : parameter error.\n", __func__ );
		return ( THRD_SyncScannerAdjust_ParameterError );
	}

	pus_Emmc = (ushort*)malloc( pst_SyncAdjust->ui_Size );
	if ( pus_Emmc == NULL ){
		printf( "$$$$ %s : pus_Emmc malloc error.\n", __func__ );
		return ( THRD_SyncScannerAdjust_MallocError );
	}

	puc_Flash = (uchar*)malloc( pst_SyncAdjust->ui_Size );
	if ( puc_Flash == NULL ){
		printf( "$$$$ %s : puc_Flash malloc error.\n", __func__ );
		free( pus_Emmc );
		return ( THRD_SyncScannerAdjust_MallocError );
	}

	memset( pus_Emmc, 0x00, pst_SyncAdjust->ui_Size );
	memset( puc_Flash, 0x00, pst_SyncAdjust->ui_Size );

	// eMMCのデータ取得、CRC32取得・作成・比較を実行.
	(void)pst_SyncAdjust->fp_GetEmmc( pus_Emmc, &ui_DataSize );
	ui_EmmcCrc32 = *(uint*)pus_Emmc;											// eMMCから取得したデータの先頭にCRC32がセットされている.
	pus_EmmcData = pus_Emmc + ( THRD_IriAdjust_Crc32Size / sizeof(ushort) );	// eMMCから取得したデータのうち、CRC32を除いたデータ.
	ui_TempCrc32 = makeIriAdjustCrc32( pus_EmmcData, ui_DataSize );				// eMMCから取得したデータを元にCRC32を生成.
	if ( ui_EmmcCrc32 == ui_TempCrc32 ){
		b_EmmcOK = true;
#ifdef THRD_SyncScannerAdjust_DebugLog	// for Debug.
		printf( "$$$$ %s : eMMC CRC32 OK.\n", __func__ );
#endif	// for Debug.
	}
	else {
		b_EmmcOK = false;
		printf( "$$$$ %s : eMMC CRC32 error.\n", __func__ );
	}

	// SPI-Flashのデータ取得、CRC32取得・作成・比較を実行.
	if ( THRG_getMachineInfo( pst_SyncAdjust->ui_Offset, (void*)puc_Flash, pst_SyncAdjust->ui_Size ) == THRD_GET_SPI_OK ){
		ui_FlashCrc32 = *(uint*)puc_Flash;											// SPI-Flashから取得したデータの先頭にCRC32がセットされている.
		puc_FlashData = puc_Flash + ( THRD_IriAdjust_Crc32Size / sizeof(uchar) );	// SPI-Flashから取得したデータのうち、CRC32を除いたデータ.
		ui_TempCrc32 = makeIriAdjustCrc32( (ushort*)puc_FlashData, ui_DataSize );	// SPI-Flashから取得したデータを元にCRC32を生成.
		if ( ui_FlashCrc32 == ui_TempCrc32 ){
			b_FlashOK = true;
#ifdef THRD_SyncScannerAdjust_DebugLog	// for Debug.
			printf( "$$$$ %s : SPI-Flash CRC32 OK.\n", __func__ );
#endif	// for Debug.
		}
		else {
			b_FlashOK = false;
			printf( "$$$$ %s : SPI-Flash CRC32 error.\n", __func__ );
		}
	}
	else {
		b_FlashOK = false;
		printf( "$$$$ %s : THRG_getMachineInfo error.\n", __func__ );
	}

	// 書き込み先判断
	if ( ( b_EmmcOK == true ) && ( b_FlashOK == true ) ){
		// 正常時は、CRC32以外のデータ領域を比較.
		if ( memcmp( pus_EmmcData, puc_FlashData, ui_DataSize ) == 0 ){
			// 一致時は書き込み無し.
#ifdef THRD_SyncScannerAdjust_DebugLog	// for Debug.
			printf( "$$$$ %s : eMMC:OK SPI-Flash:OK. Data matched. No write.\n", __func__ );
#endif	// for Debug.
		}
		else {
			// 不一致時はeMMC側のデータ更新があったと判断し、eMMC→SPI-FlashのWriteを行う。.
			printf( "$$$$ %s : eMMC:OK SPI-Flash:OK. Data unmatched. Write SPI-Flash.\n", __func__ );
			if ( THRG_setMachineInfo( pst_SyncAdjust->ui_Offset, (void*)pus_Emmc, pst_SyncAdjust->ui_Size ) != THRD_SET_SPI_OK ){
				printf( "$$$$ %s : SPI-Flash write error.\n", __func__ );
				i_Ret = THRD_SyncScannerAdjust_FlashWriteError;
			}
		}
	}
	else if ( b_EmmcOK == true ){
		// eMMCのみ正常時は、eMMC→SPI-FlashのWriteを行う。.
		printf( "$$$$ %s : eMMC:OK SPI-Flash:NG. Write SPI-Flash.\n", __func__ );
		if ( THRG_setMachineInfo( pst_SyncAdjust->ui_Offset, (void*)pus_Emmc, pst_SyncAdjust->ui_Size ) != THRD_SET_SPI_OK ){
			printf( "$$$$ %s : SPI-Flash write error.\n", __func__ );
			i_Ret = THRD_SyncScannerAdjust_FlashWriteError;
		}
	}
	else if ( b_FlashOK == true ){
		// SPI-Flashのみ正常時は、SPI-Flash→eMMCのWriteを行う。.
		printf( "$$$$ %s : eMMC:NG SPI-Flash:OK. Write eMMC.\n", __func__ );
		(void)pst_SyncAdjust->fp_SetEmmc( (ushort*)puc_FlashData );
		b_EmmcWrite = true;
	}
	else {
		// 両方異常時は、書き込みを行わない
		printf( "$$$$ %s : eMMC:NG SPI-Flash:NG. No write.\n", __func__ );
		i_Ret = THRD_SyncScannerAdjust_DataError;
	}

	// CRC32設定許可.
	pst_SyncAdjust->fp_EnaCrc32Chg();

	// eMMCへの書き込みを実施した場合のみ、CRC32を設定する。.
	if ( b_EmmcWrite == true ){
		pst_SyncAdjust->fp_SetCrc32();
	}

	free( pus_Emmc );
	free( puc_Flash );

	return ( i_Ret );
}


/*================================================================================================
［機能］	スキャナ調整値のCRC32更新許可設定.
--------------------------------------------------------------------------------------------------
［引数］	なし.
［戻値］	なし.
［履歴］	2016/02/10	es_igarashi	[CQ521427]	スキャナ調整値保存先追加対応.
================================================================================================*/
static void enaScannerAdjustCrc32Change( void )
{
	comIRIndi.enaIRI_ScannerAdjustCrc32Change();

	return;
}


/*================================================================================================
［機能］	スキャナ調整値のCRC32設定.
--------------------------------------------------------------------------------------------------
［引数］	なし.
［戻値］	なし.
［履歴］	2016/02/16	es_igarashi	[CQ521427]	スキャナ調整値保存先追加対応.
================================================================================================*/
static void setScannerAdjustCrc32( void )
{
	comIRIndi.setIRI3_ScannerAdjustCrc32();

	return;
}


/*================================================================================================
［機能］	eMMCからスキャナ調整値を取得.
--------------------------------------------------------------------------------------------------
［引数］	uchar *		pus_Buf			読み出しデータ格納用バッファアドレス.
			uint *		pui_DataSize	CRC32を除くデータサイズ
［戻値］	0		:	OK.
			-5		:	THRD_SyncScannerAdjust_ParameterError.
［履歴］	2016/02/10	es_igarashi	[CQ521427]	スキャナ調整値保存先追加対応.
================================================================================================*/
static int getScannerAdjustFromEmmc( ushort *pus_Buf, uint *pui_DataSize )
{
	uint		*pui_Dat = (uint*)pus_Buf;

	if ( ( pus_Buf == NULL ) || ( pui_DataSize == NULL ) ){
		printf( "$$$$ %s : parameter error. pus_Buf=0x%08x pui_DataSize=0x%08x\n", __func__, (uintptr_t)pus_Buf, (uintptr_t)pui_DataSize );
		return ( THRD_SyncScannerAdjust_ParameterError );
	}
	*pui_Dat++ = comIRIndi.getIRI3_ScannerAdjustCrc32();
	*pui_DataSize = comIRIndi.getIRI_ScannerAdjustAll( (ushort*)pui_Dat );

	return ( OK );
}


/*================================================================================================
［機能］	eMMCにスキャナ調整値を設定.
--------------------------------------------------------------------------------------------------
［引数］	uchar *		pus_Buf		書き込みデータ格納用バッファアドレス.
［戻値］	0		:	OK.
			-5		:	THRD_SyncScannerAdjust_ParameterError.
［履歴］	2016/01/08	es_igarashi	[CQ521427]	スキャナ調整値保存先追加対応.
================================================================================================*/
static int setScannerAdjustToEmmc( ushort *pus_Buf )
{
	ushort		*pus_Dat = pus_Buf;

	if ( pus_Buf == NULL ){
		printf( "$$$$ %s : pus_Buf error.\n", __func__ );
		return ( THRD_SyncScannerAdjust_ParameterError );
	}

	comIRIndi.setIRI_LeftImage( *pus_Dat++ );
	comIRIndi.setIRI_TopImage( *pus_Dat++ );
	comIRIndi.setIRI_MainMag( *pus_Dat++ );
	comIRIndi.setIRI_SubMag( *pus_Dat++ );
	comIRIndi.setIRI_AEReference( *pus_Dat++ );
	comIRIndi.setIRI_ADFStopPositionCD( *pus_Dat++ );
	comIRIndi.setIRI_OrgDtctSensorSetUnset( *pus_Dat++ );
	comIRIndi.setIRI_1SideOriginalStopPosition( *pus_Dat++ );
	comIRIndi.setIRI_2SideOriginalStopPosition( *pus_Dat++ );
	comIRIndi.setIRI_RegistLoop( *pus_Dat++ );
	comIRIndi.setIRI_ClampROdd( *pus_Dat++ );
	comIRIndi.setIRI_ClampREven( *pus_Dat++ );
	comIRIndi.setIRI_ClampGOdd( *pus_Dat++ );
	comIRIndi.setIRI_ClampGEven( *pus_Dat++ );
	comIRIndi.setIRI_ClampBOdd( *pus_Dat++ );
	comIRIndi.setIRI_ClampBEven( *pus_Dat++ );
	comIRIndi.setIRI_GainROdd( *pus_Dat++ );
	comIRIndi.setIRI_GainREven( *pus_Dat++ );
	comIRIndi.setIRI_GainGOdd( *pus_Dat++ );
	comIRIndi.setIRI_GainGEven( *pus_Dat++ );
	comIRIndi.setIRI_GainBOdd( *pus_Dat++ );
	comIRIndi.setIRI_GainBEven( *pus_Dat++ );
	comIRIndi.setIRI_AdjustBeam1( *pus_Dat++ );
	comIRIndi.setIRI_AdjustBeam2( *pus_Dat++ );
	comIRIndi.setIRI_AdjustBeam3( *pus_Dat++ );
	comIRIndi.setIRI_AdjustBeam4( *pus_Dat++ );
	comIRIndi.setIRI_AdjustBeam5( *pus_Dat++ );
	comIRIndi.setIRI_AdjustBeam6( *pus_Dat++ );
	comIRIndi.setIRI_AdjustBeam7( *pus_Dat++ );
	comIRIndi.setIRI_AdjustBeam8( *pus_Dat++ );
	comIRIndi.setIRI_AdjustBeam9( *pus_Dat++ );
	comIRIndi.setIRI_AdjustBeam10( *pus_Dat++ );
	comIRIndi.setPapersizeDetectTable( *pus_Dat++ );
	comIRIndi.setTableSelectInfo( *pus_Dat++ );
	comIRIndi.setIRI_EDHPassAdjustMag( *pus_Dat++ );
	comIRIndi.setIRI_RegistLoopRev( *pus_Dat++ );
	comIRIndi.setIRI_IRSliderStopPosAdjust( *pus_Dat++ );
	comIRIndi.setIRI_RemoveNoiseLevel_Service( *pus_Dat++ );
	comIRIndi.setIRI_FinishStamp_Service( *pus_Dat++ );
	comIRIndi.setIRI_GlassTransmit_RedData( *pus_Dat++ );
	comIRIndi.setIRI_GlassTransmit_GreenData( *pus_Dat++ );
	comIRIndi.setIRI_GlassTransmit_BlueData( *pus_Dat++ );
	comIRIndi.setIRI_2SideADFStopPositionCD( *pus_Dat++ );
	comIRIndi.setIRI_EDHAdjustmentWidthMax( *pus_Dat++ );
	comIRIndi.setIRI_EDHAdjustmentWidthMin( *pus_Dat++ );
	comIRIndi.setIRI_AmbientLightLevel( *pus_Dat++ );
	comIRIndi.setIRI_MainMagBack( *pus_Dat++ );
	comIRIndi.setIRI_1PassDFOffsetBack( *pus_Dat++ );
	comIRIndi.setIRI_1PassDFGainROddBack( *pus_Dat++ );
	comIRIndi.setIRI_1PassDFGainREvenBack( *pus_Dat++ );
	comIRIndi.setIRI_1PassDFGainGOddBack( *pus_Dat++ );
	comIRIndi.setIRI_1PassDFGainGEvenBack( *pus_Dat++ );
	comIRIndi.setIRI_1PassDFGainBOddBack( *pus_Dat++ );
	comIRIndi.setIRI_1PassDFGainBEvenBack( *pus_Dat++ );
	comIRIndi.setIRI_1PassDFTableSelectInfoBack( *pus_Dat++ );
	comIRIndi.setIRI_CISOpposingBoardHomePosition( *pus_Dat++ );
	comIRIndi.setPIC_LightnessAdjustBack( *pus_Dat++ );
	comIRIndi.setPIC_HueAdjustBack( *pus_Dat++ );
	comIRIndi.setPIC_ChromaAdjustBack( *pus_Dat++ );
	comIRIndi.setIRI_SubMagBack( *pus_Dat++ );

	return ( OK );
}


/*================================================================================================
［機能］	IR調整値のCRC32更新許可設定.
--------------------------------------------------------------------------------------------------
［引数］	なし.
［戻値］	なし.
［履歴］	2016/02/10	es_igarashi	[CQ521427]	スキャナ調整値保存先追加対応.
================================================================================================*/
static void enaIrFactoryAdjustCrc32Change( void )
{
	comIRIndi.enaIRI_IrFactoryAdjustCrc32Change();

	return;
}


/*================================================================================================
［機能］	IR調整値のCRC32設定.
--------------------------------------------------------------------------------------------------
［引数］	なし.
［戻値］	なし.
［履歴］	2016/02/10	es_igarashi	[CQ521427]	スキャナ調整値保存先追加対応.
================================================================================================*/
static void setIrFactoryAdjustCrc32( void )
{
	comIRIndi.setIRI3_IrFactoryAdjustCrc32();

	return;
}


/*================================================================================================
［機能］	eMMCからIR調整値を取得.
--------------------------------------------------------------------------------------------------
［引数］	uchar *		pus_Buf			読み出しデータ格納用バッファアドレス.
			uint *		pui_DataSize	CRC32を除くデータサイズ
［戻値］	0		:	OK.
			-5		:	THRD_SyncScannerAdjust_ParameterError.
［履歴］	2016/02/10	es_igarashi	[CQ521427]	スキャナ調整値保存先追加対応.
================================================================================================*/
static int getIrFactoryAdjustFromEmmc( ushort *pus_Buf, uint *pui_DataSize )
{
	uint		*pui_Dat = (uint*)pus_Buf;

	if ( ( pus_Buf == NULL ) || ( pui_DataSize == NULL ) ){
		printf( "$$$$ %s : pus_Buf error.\n", __func__ );
		return ( THRD_SyncScannerAdjust_ParameterError );
	}
	*pui_Dat++ = comIRIndi.getIRI3_IrFactoryAdjustCrc32();
	*pui_DataSize = comIRIndi.getIRI_IrFactoryAdjustAll( (ushort*)pui_Dat );

	return ( OK );
}


/*================================================================================================
［機能］	eMMCにIR調整値を設定.
--------------------------------------------------------------------------------------------------
［引数］	ushort *	pus_Buf		書き込みデータ格納用バッファアドレス.
［戻値］	0		:	OK.
			-5		:	THRD_SyncScannerAdjust_ParameterError.
［履歴］	2016/01/08	es_igarashi	[CQ521427]	スキャナ調整値保存先追加対応.
================================================================================================*/
static int setIrFactoryAdjustToEmmc( ushort *pus_Buf )
{
	ushort		*pus_Dat = pus_Buf;

	if ( pus_Buf == NULL ){
		printf( "$$$$ %s : pus_Buf error.\n", __func__ );
		return ( THRD_SyncScannerAdjust_ParameterError );
	}

	comIRIndi.setIRI3_LeftImage( (short)(*pus_Dat++) );						// ushort -> short.
	comIRIndi.setIRI3_TopImage( (short)(*pus_Dat++) );						// ushort -> short.
	comIRIndi.setIRI3_MainMag( *pus_Dat++ );
	comIRIndi.setIRI3_SubMag( *pus_Dat++ );
	comIRIndi.setIRI3_GlassTransmit( *pus_Dat++ );
	comIRIndi.setIRI3_GlassTransmitWriteComplete( *pus_Dat++ );
	comIRIndi.setIRI3_IRSliderStopPosAdjust( (short)(*pus_Dat++) );			// ushort -> short.
	comIRIndi.setIRI3_CalibrationExecStatus( *pus_Dat++ );
	comIRIndi.setIRI3_LutCoefficient_R_A0( *pus_Dat++ );
	comIRIndi.setIRI3_LutCoefficient_R_A1( *pus_Dat++ );
	comIRIndi.setIRI3_LutCoefficient_R_A2( *pus_Dat++ );
	comIRIndi.setIRI3_LutCoefficient_R_A3( *pus_Dat++ );
	comIRIndi.setIRI3_LutCoefficient_G_A0( *pus_Dat++ );
	comIRIndi.setIRI3_LutCoefficient_G_A1( *pus_Dat++ );
	comIRIndi.setIRI3_LutCoefficient_G_A2( *pus_Dat++ );
	comIRIndi.setIRI3_LutCoefficient_G_A3( *pus_Dat++ );
	comIRIndi.setIRI3_LutCoefficient_B_A0( *pus_Dat++ );
	comIRIndi.setIRI3_LutCoefficient_B_A1( *pus_Dat++ );
	comIRIndi.setIRI3_LutCoefficient_B_A2( *pus_Dat++ );
	comIRIndi.setIRI3_LutCoefficient_B_A3( *pus_Dat++ );
	comIRIndi.setIRI3_3x3Matrix11( *pus_Dat++ );
	comIRIndi.setIRI3_3x3Matrix12( *pus_Dat++ );
	comIRIndi.setIRI3_3x3Matrix13( *pus_Dat++ );
	comIRIndi.setIRI3_3x3Matrix21( *pus_Dat++ );
	comIRIndi.setIRI3_3x3Matrix22( *pus_Dat++ );
	comIRIndi.setIRI3_3x3Matrix23( *pus_Dat++ );
	comIRIndi.setIRI3_3x3Matrix31( *pus_Dat++ );
	comIRIndi.setIRI3_3x3Matrix32( *pus_Dat++ );
	comIRIndi.setIRI3_3x3Matrix33( *pus_Dat++ );
	comIRIndi.setIRI3_ChromaticityCoordinates_R_X( (short)(*pus_Dat++) );	// ushort -> short.
	comIRIndi.setIRI3_ChromaticityCoordinates_G_X( (short)(*pus_Dat++) );	// ushort -> short.
	comIRIndi.setIRI3_ChromaticityCoordinates_B_X( (short)(*pus_Dat++) );	// ushort -> short.
	comIRIndi.setIRI3_ChromaticityCoordinates_R_Y( (short)(*pus_Dat++) );	// ushort -> short.
	comIRIndi.setIRI3_ChromaticityCoordinates_G_Y( (short)(*pus_Dat++) );	// ushort -> short.
	comIRIndi.setIRI3_ChromaticityCoordinates_B_Y( (short)(*pus_Dat++) );	// ushort -> short.
	comIRIndi.setIRI3_WhiteStandardPoint( *pus_Dat++ );
	comIRIndi.setIRI3_NewJigFlag( *pus_Dat++ );
	comIRIndi.setIRI3_PanelTopImage( (short)(*pus_Dat++) );					// ushort -> short.
	comIRIndi.setIRI3_TroubleStatusFlag( (short)(*pus_Dat++) );				// ushort -> short.
	// 2016/07/13	es_igarashi		[CQ532475]	IR調整値(キャリブレーション使用フラグ(表面))追加対応.
	comIRIndi.setIRI3_TableSelectInfo( (short)(*pus_Dat++) );				// ushort -> short.

//
// Str【Sparrow 】機器構成変更_No.283_1パス裏面のシェーディングリペア処理対応 2018/01/26 es_nagasaka
	comIRIndi.setIRI3_ShadingRepairInitialDataCheckCrc32( (uint32_t)(*pus_Dat) );	// 初期シェーディングデータチェックサム
	pus_Dat += sizeof( uint32_t ) / sizeof( ushort );
	comIRIndi.setIRI3_ShadingRepairEnable( *pus_Dat++ );					// シェーディングリペア処理
	comIRIndi.setIRI3_ShadingRepairThreshold( *pus_Dat++ );					// シェーディングデータ異常検知
	comIRIndi.setIRI3_ShadingRepairMovingAverage( *pus_Dat++ );				// シェーディングデータ移動平均
// End【Sparrow 】機器構成変更_No.283_1パス裏面のシェーディングリペア処理対応 2018/01/26 es_nagasaka
//

	return ( OK );
}


/*================================================================================================
［機能］	CRC32データ生成.
--------------------------------------------------------------------------------------------------
［引数］	ushort *	pus_Buf			データバッファアドレス.
			uint		ui_Size			サイズ.
［戻値］	なし.
［履歴］	2016/01/08	es_igarashi	[CQ521427]	スキャナ調整値保存先追加対応.
================================================================================================*/
static uint makeIriAdjustCrc32( ushort *pus_Buf, uint ui_Size )
{
	return ( CC_BackupIRIndi::makeIRI_IriAdjustCrc32( (uchar*)pus_Buf, ui_Size ) );
}


/*================================================================================================
［機能］	【DEBUG用】eMMC、SPI-Flash両方のスキャナ調整値の表示.
--------------------------------------------------------------------------------------------------
［引数］	なし.
［戻値］	なし.
［履歴］	2016/01/08	es_igarashi	[CQ521427]	スキャナ調整値保存先追加対応.
================================================================================================*/
void THRG_PrintScannerAdjustAll( void )
{
	printAdjustCommon( &st_SyncAdjust[THRD_SyncAdjustType_Scanner] );

	return;
}


/*================================================================================================
［機能］	【DEBUG用】eMMC、SPI-Flash両方のIR調整値の表示.
--------------------------------------------------------------------------------------------------
［引数］	なし.
［戻値］	なし.
［履歴］	2016/01/08	es_igarashi	[CQ521427]	スキャナ調整値保存先追加対応.
================================================================================================*/
void THRG_PrintIrFactoryAdjustAll( void )
{
	printAdjustCommon( &st_SyncAdjust[THRD_SyncAdjustType_IrFactory] );

	return;
}


/*================================================================================================
［機能］	【DEBUG用】eMMC、SPI-Flash両方のスキャナ調整値の表示.
--------------------------------------------------------------------------------------------------
［引数］	なし.
［戻値］	なし.
［履歴］	2016/01/08	es_igarashi	[CQ521427]	スキャナ調整値保存先追加対応.
================================================================================================*/
static void printAdjustCommon( const THRS_SyncAdjust *pst_SyncAdjust )
{
	int			i_Result = OK;
	uint		ui_DataSize = 0;
	ushort		*pus_Buf = NULL;

	if ( pst_SyncAdjust == NULL ){
		printf( "$$$$ %s : parameter error.\n", __func__ );
		return;
	}

	pus_Buf = (ushort*)malloc( pst_SyncAdjust->ui_Size );
	if ( pus_Buf == NULL ){
		printf( "$$$$ %s : pus_Buf malloc error.\n", __func__ );
		return;
	}

	// eMMC(SSD)上のスキャナ調整値データを表示.
	printf( "$$$$ %s : Print Scanner Adjust Data (eMMC)\n", __func__ );
	memset( pus_Buf, 0x00, pst_SyncAdjust->ui_Size );
	(void)pst_SyncAdjust->fp_GetEmmc( pus_Buf, &ui_DataSize );
	dumpShortArea( pus_Buf, pst_SyncAdjust->ui_Size );

	// SPI-Flash上のスキャナ調整値データを表示.
	printf( "$$$$ %s : Print Scanner Adjust Data (SPI-Flash)\n", __func__ );
	memset( pus_Buf, 0x00, pst_SyncAdjust->ui_Size );
	i_Result = THRG_getMachineInfo( pst_SyncAdjust->ui_Offset, (void*)pus_Buf, pst_SyncAdjust->ui_Size );
	if ( i_Result == THRD_GET_SPI_OK ){
		dumpShortArea( pus_Buf, pst_SyncAdjust->ui_Size );
	}
	else {
		printf( "$$$$ %s : THRG_getMachineInfo errored. i_Result=%d\n", __func__, i_Result );
	}

	free(pus_Buf);

	return;
}


/*================================================================================================
［機能］	【DEBUG用】eMMC(SSD)またはSPI-Flashスキャナ調整値の表示.
--------------------------------------------------------------------------------------------------
［引数］	char *		pus_Buf			データバッファアドレス.
			int			i_Size			サイズ.
［戻値］	なし.
［履歴］	2016/01/08	es_igarashi	[CQ521427]	スキャナ調整値保存先追加対応.
================================================================================================*/
static void dumpShortArea( ushort *pus_Buf, uint ui_Size )
{
	ushort		*pus_Dat = pus_Buf;
	uint		ui_CntMax = ( ( ui_Size - 1 ) / sizeof(ushort) ) + 1;
	uint		ui_Cnt = 0;

	if ( ui_Size <= 0 ){
		printf( "$$$$ %s : size error. ui_Size=%d\n", __func__, ui_Size );
		return;
	}

	printf( "\tdat[%04dbytes]=\n", ui_Size );

	while( 1 ){
		for( int i_Loop1 = 0; i_Loop1 < 4; i_Loop1++ ){
			printf( "\t" );
			for( int i_Loop2 = 0; i_Loop2 < 16; i_Loop2++  ){
				printf( "%04x ", *pus_Dat++ );
				ui_Cnt++;
				if ( ui_Cnt >= ui_CntMax ){
					goto loopEnd;
				}
			}
			printf( "\n" );
		}
		printf( "\n" );
	}

loopEnd :
	printf( "\n\n" );
	printf( "ui_Size=%d ui_Cnt=%d\n\n", ui_Size, ui_Cnt );

	return;
}
#endif	// #ifdef DEF_A4_Cus		[CQ521427]	スキャナ調整値保存先追加対応.


/*================================================================================================
［機能］	トラブルリセット起動の判定.
--------------------------------------------------------------------------------------------------
［引数］	なし
［戻値］	True:トラブルリセットを行う False:トラブルリセットを行わない.
［履歴］	2014/06/18 horii 新規 USB蓋センサー廃止対応.
			2014/10/26 horii CQ.505927 BOOT_TYPEからPANEL_KEYのキー情報での判断に変更.
			2018/07/04 Sky酒井(純) [Eagle][AR.1055339] トラブルリセット起動しない.
================================================================================================*/
#ifdef DEF_FUM_HDDLess_Enable
#define DEF_KM_WARP_SETTINGINFO_ADR         DEF_SNAPSHOTINFO_START_BLOCK     // 4161→DEF_SNAPSHOTINFO_START_BLOCK 13/8/27 ADS川口 CQ.381189(Act_67618) EE-HDL-5	HDDレス化対応.
#else
#define DEF_KM_WARP_SETTINGINFO_ADR         4159     // Warp用構成情報保存アドレス.【Warp!!起動シーケンス対応】10/03/03 第1OSW 浦崎./*4161→4159 13/11/06 ADS永井 CQ.383443(Act_68820)*/
#endif
#define DEF_KM_WARP_SI_INDEX_PANEL_KEY		373				// [in] 起動時のキー押下情報.
#define DEF_KM_WARP_SETTINGINFO_DISK        "/dev/kmsda"	// SSD.【Warp!!起動シーケンス対応】10/03/03 第1OSW 浦崎.
Bool THRF_isExtraMode(void)
{
	static Bool bl_First = True;	// 初回アクセスフラグ初期化（True:初回アクセス False:2回目以降アクセス）.
	static Bool bl_Result = False;
	
	if( bl_First == False){
		return bl_Result;
	}
	bl_First = False;
	
	int32_t i_ssdFd = SYSF_open( DEF_KM_WARP_SETTINGINFO_DISK, O_RDWR, 0 );
	if(i_ssdFd < 0) {
		printf("%s:open error \n",__FUNCTION__);
		return False;
	}
	if(lseek(i_ssdFd, DEF_KM_WARP_SETTINGINFO_ADR * 512 + DEF_KM_WARP_SI_INDEX_PANEL_KEY, SEEK_SET) < 0) {
		printf("%s:lseek error \n",__FUNCTION__);
		SYSF_close( i_ssdFd );
		return False;
	}
	uchar uc_data = 0;
	if( read(i_ssdFd, &uc_data, sizeof(uc_data))  < 0 ) {
		printf("%s:read error \n",__FUNCTION__);
		SYSF_close( i_ssdFd );
		return False;
	}
	
	SYSF_close( i_ssdFd );
	
	if(uc_data == 0x16){ // リセットキー押下であればTrueとする.
		bl_Result = True;
		return True;
	}else{
		return False;
	}
}

// ================================================================================================.
//! <!--機能--> 電源SubCPUバージョン、チェックサムを取得する.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! char *version	: 電源SubCPUバージョン文字列ポインタ.
//! int *chksum		: チェックサム数値ポインタ.
//! <!--戻値-->.
//! \return BOOL TRUE:取得成功 / FALSE:取得失敗.
//! <!--詳細-->.
//! ファイル先端にチェックサムとバージョンが格納されている。
//! <!--履歴-->.
//  2014.10.20	horii	新規作成.
//  2015.05.12	yamaguchi 機種コード変更対応 MinervaSBK.
//  2015.10.19	yamaguchi CQ.520452 MinervaSBKの機種コード変更(A796→A795).
//  2017.01.18  samura    AR1016286 ZeusSBK2nd_12(AA6V),ZeusSBK2nd_ZX0(AA6R),MinervaMLK(A8JE)の機種コード変更.
// ================================================================================================.
#if defined ( DEF_ZEUSSBK2ND_ZX0_CUS ) 
#define THRD_POWERSUBCPU_VERSION "AA6R0Y0-1900-G00-00(00)"
#elif defined ( DEF_MINERVAMLK_CUS ) 
#define THRD_POWERSUBCPU_VERSION "A8JE0Y0-1900-G00-00(00)"
#elif defined(DEF_MinervaS_PF)
#define THRD_POWERSUBCPU_VERSION "A7950Y0-1900-G00-00(00)"
#elif defined ( DEF_ZEUSSBK2ND_12_CUS ) 
#define THRD_POWERSUBCPU_VERSION "AA6V0Y0-1900-G00-00(00)"
#else
#define THRD_POWERSUBCPU_VERSION "A7AH0Y0-1900-G00-00(00)"
#endif	// DEF_ZEUSSBK2ND_ZX0_CUS).

Bool THRF_getPowerSubCPUVersion( char *version, ushort *chksum)
{
#if !defined(SIMULATOR)
	int32_t	 	bf_fd = 0;	// ファイルポインタ.
	char	buffer[5];	// 作業用.
	ssize_t		ret;
	
	if( (version == NULL) || (chksum == NULL) ){
		printf("THRF_getPowerSubCPUVersion() parameter NULL\n");
		return False;
	}
	memset( buffer, 0 , sizeof(buffer) );
	
	// 電源SubCPUのOPEN.
	bf_fd = SYSF_devopen( "/sys/bus/platform/devices/PMU.0/devices/PWRSUBCPU_VERSION" , O_RDWR | O_SYNC, 0644 );
	if ( bf_fd == -1 ){
		printf( "SYSF_open ERR errno:%d\n", SYSF_errnoGet() );
		return False;
	}

	// 電源SubCPUのバージョンをリードする(シークは不要).
	ret = SYSF_read( bf_fd, buffer, 4 );
	if( ret == -1 ){
		SYSF_devclose(bf_fd);
		printf( "read: ioctl ERR errno:%d\n",SYSF_errnoGet() );
		return False;
	}
	
	// 電源SubCPUのバージョンをセット.
	strncpy(version,THRD_POWERSUBCPU_VERSION,24);
	version[17] = buffer[0];
	version[18] = buffer[1];
	version[20] = buffer[2];
	version[21] = buffer[3];

	SYSF_devclose(bf_fd);

	// 電源SubCPUのOPEN.
	bf_fd = SYSF_devopen( "/sys/bus/platform/devices/PMU.0/devices/PWRSUBCPU_CHECKSUM" , O_RDWR | O_SYNC, 0644 );
	if ( bf_fd == -1 ){
		SYSF_devclose(bf_fd);
		return False;
	}
	// 電源SubCPUのチェックサムをリードする(シークは不要).
	ret = SYSF_read( bf_fd, buffer, 4 );
	if( ret == -1 ){
		SYSF_devclose(bf_fd);
		printf( "read: ioctl ERR errno:%d\n",SYSF_errnoGet() );
		return False;
	}
	
	// 電源SubCPUのチェックサムをセット.
	*chksum = (ushort)strtol(buffer, NULL, 16);

	SYSF_devclose(bf_fd);

	return True;
#else
	return False;
#endif
}

// ================================================================================================.
//! <!--機能--> PS-CPUバージョン、チェックサムを取得する.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! char *psc_Version	: PS-CPUバージョン文字列ポインタ.
//! int *pus_Chksum		: チェックサム数値ポインタ.
//! <!--戻値-->.
//! \return BOOL TRUE:取得成功 / FALSE:取得失敗.
//! <!--詳細-->.
//! なし
//! <!--履歴-->.
//  2018.02.13	Sky内藤 [Eagle][制御構成変更 No.2] ROMバージョン対応
// ================================================================================================.
Bool THRG_GetPsCpuVersion( char *psc_Version, ushort *pus_Chksum)
{
#if !defined(FUM_LINUX)
	int32_t	si_Fd = 0;												// ファイルポインタ.
	char	sc_BufferVersion[THRD_PsCpuVersionSize+1] = { 0 };		// Version取得作業用.
	char	sc_BufferCheckSum[THRD_PsCpuCheckSumSize+1] = { 0 };	// CheckSum取得作業用.
	ssize_t	si_Ret = 0;												// SYSF_readの戻り値.
	
	if( psc_Version == NULL) {
		printf("%s: psc_Version parameter NULL\n", __FUNCTION__);
		return False;
	}
	if( pus_Chksum == NULL ){
		printf("%s: pus_Chksum parameter NULL\n", __FUNCTION__);
		return False;
	}
	memset( sc_BufferVersion, 0 , sizeof(sc_BufferVersion) );
	
	// PS-CPUのsysfsファイル(バージョン取得)をオープン.
	si_Fd = SYSF_devopen( THRD_PMU_PsCpuVersion , O_RDONLY, 0644 );
	if ( si_Fd == -1 ){
		printf( "%s: SYSF_open PsCpuVersion sysfs ERR errno:%d\n", __FUNCTION__, SYSF_errnoGet() );
		return False;
	}

	// PS-CPUのバージョンをリードする.
	si_Ret = SYSF_read( si_Fd, sc_BufferVersion, THRD_PsCpuVersionSize );
	if( si_Ret == -1 ){
		SYSF_devclose(si_Fd);
		printf( "%s: SYSF_read PsCpuVersion sysfs ERR errno:%d\n", __FUNCTION__, SYSF_errnoGet() );
		return False;
	}
	
	// PS-CPUのバージョンをセット.
	memcpy( psc_Version, sc_BufferVersion, sizeof(sc_BufferVersion) );

	SYSF_devclose(si_Fd);

	memset( sc_BufferCheckSum, 0 , sizeof(sc_BufferCheckSum));
	
	// PS-CPUのsysfsファイル(チェックサム取得)をオープン.
	si_Fd = SYSF_devopen( THRD_PMU_PsCpuCheckSum , O_RDONLY, 0644 );
	if ( si_Fd == -1 ){
		printf( "%s: SYSF_open PsCpuCheckSum sysfs ERR errno:%d\n", __FUNCTION__, SYSF_errnoGet() );
		return False;
	}

	// PS-CPUのチェックサムをリードする.
	si_Ret = SYSF_read( si_Fd, sc_BufferCheckSum, THRD_PsCpuCheckSumSize );
	if( si_Ret == -1 ){
		SYSF_devclose(si_Fd);
		printf( "%s: SYSF_read PsCpuCheckSum sysfs ERR errno:%d\n", __FUNCTION__, SYSF_errnoGet() );
		return False;
	}

	// PS-CPUのチェックサムをセット.
	*pus_Chksum = (ushort)strtoul(sc_BufferCheckSum, NULL, 16);

	SYSF_devclose(si_Fd);

	return True;
#else
	return False;
#endif
}

// ================================================================================================.
//! <!--機能--> LPPPバージョン、チェックサムを取得する.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! char* psc_Version	: LPPPバージョン文字列ポインタ.
//! ushort* pus_Chksum	: チェックサム数値ポインタ.
//! <!--戻値-->.
//! \return BOOL TRUE:取得成功 / FALSE:取得失敗.
//! <!--詳細-->.
//! LPPP FWのバージョンとチェックサムを取得する.
//! <!--履歴-->.
//  2018.02.13	Sky内藤 [Eagle][制御構成変更 No.2] ROMバージョン対応
// ================================================================================================.
Bool THRG_GetLPPPVersion( char* psc_Version, ushort* pus_Chksum)
{
#if !defined(FUM_LINUX)
	int32_t	si_Fd = 0;											// ファイルポインタ.
	char 	sc_BufferVersion[THRD_LPPP_VersionSize+1] = { 0 };	// Version取得作業用.
	char	sc_BufferCheckSum[THRD_LPPP_CheckSumSize] = { 0 };	// CheckSum取得作業用.
	ssize_t	si_Ret = 0;											// SYSF_readの戻り値.
	off_t	si_lseekRet = 0;									// SYSF_lseekの戻り値.

	// パラメータチェック.
	if( psc_Version == NULL ){
		printf("%s: psc_Version parameter NULL\n", __FUNCTION__);
		return False;
	}
	if( pus_Chksum == NULL ){
		printf("%s: pus_Chksum parameter NULL\n", __FUNCTION__);
		return False;
	}
	memset( sc_BufferVersion, 0 , sizeof(sc_BufferVersion) );
	memset( sc_BufferCheckSum, 0 , sizeof(sc_BufferCheckSum) );

	// LPPP デバイスオープン.
	si_Fd = SYSF_devopen( THRD_LPPPName_CopyDevice, O_RDONLY, 0644 );
	if ( si_Fd == -1 ){
		printf( "%s: SYSF_open ERR errno:%d\n", __FUNCTION__, SYSF_errnoGet() );
		return False;
	}

	// バージョンの位置まで移動.
	si_lseekRet = SYSF_lseek( si_Fd, THRD_LPPPOffset_Version, SEEK_SET );
	if( -1 == si_lseekRet){
		SYSF_devclose(si_Fd);
		printf( "%s: SYSF_lseek Version seek ERR errno:%d\n", __FUNCTION__, SYSF_errnoGet() );
		return False;
	}

	// LPPPのバージョンをリードする.
	si_Ret = SYSF_read( si_Fd, sc_BufferVersion, THRD_LPPP_VersionSize );
	if( si_Ret == -1 ){
		SYSF_devclose(si_Fd);
		printf( "%s: SYSF_read Version read ERR errno:%d\n", __FUNCTION__, SYSF_errnoGet() );
		return False;
	}

	// LPPPのバージョンをセット.
	memcpy( psc_Version, sc_BufferVersion, sizeof(sc_BufferVersion) );

	// チェックサムの位置まで移動.
	si_lseekRet = SYSF_lseek( si_Fd, THRD_LPPPOffset_CheckSum, SEEK_SET );
	if( -1 == si_lseekRet){
		SYSF_devclose(si_Fd);
		printf( "%s: SYSF_lseek ChkSum seek ERR errno:%d\n", __FUNCTION__, SYSF_errnoGet() );
		return False;
	}

	// LPPPのチェックサムをリードする.
	si_Ret = SYSF_read( si_Fd, sc_BufferCheckSum, THRD_LPPP_CheckSumSize );
	if( si_Ret == -1 ){
		SYSF_devclose(si_Fd);
		printf( "%s: SYSF_read ChkSum ERR errno:%d\n", __FUNCTION__, SYSF_errnoGet() );
		return False;
	}

	// LPPPのチェックサムをセット.
	*pus_Chksum = (ushort)((((ushort)sc_BufferCheckSum[0]) << 8) & 0xFF00);				// チェックサム上位格納.
	*pus_Chksum = (ushort)( *pus_Chksum | (((ushort)sc_BufferCheckSum[1]) & 0x00FF));	// チェックサム下位格納.

	SYSF_devclose(si_Fd);

	return True;
#else
	return False;
#endif
}

// ================================================================================================.
//! <!--機能--> USB-SATA変換基板FWのバージョン、チェックサムを取得する.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! char *version	: USB-SATA変換基板FWバージョン文字列ポインタ.
//! int *chksum		: チェックサム数値ポインタ.
//! <!--戻値-->.
//! \return BOOL TRUE:取得成功 / FALSE:取得失敗.
//! <!--詳細-->.
//! ファイル先端にチェックサムとバージョンが格納されている。
//! <!--履歴-->.
//  2014.10.20	horii	新規作成.
//  2014.12.10	Matsuoka	BETA versionのフィールド書き込み追加
//	2015.04.13	okuma	ワーニング対応
//  2017/10/10	yoneda	S800 デバイス構成変更対応.
// ================================================================================================.
#define THRD_SATA_CONTROLER_VERSION "A7AH0Y0-6400-G00-00(00)"
Bool THRF_getUSBSATAControlerVersion( char *version, ushort *chksum)
{
#if !defined(SIMULATOR) || !defined(DEF_64BIT_PF)
	int32_t fd = 0;
	int32_t ret = 0;
	uchar sense[255] = { 0 };
	struct checksum
	{
		union
		{
			uchar data[10];
			ushort sum[5];
		};
	};
	checksum buffer;
	
	memset(&buffer.data, 0, sizeof(buffer));
	

	if( (version == NULL) || (chksum == NULL) ){
		printf("THRF_getUSBSATAControlerVersion() parameter NULL\n");
		return False;
	}

	// バージョン取得.
	uchar cdb1[10] = {
		0x3C,
		0x01,
		0x52,
	};

	sg_io_hdr_t sg1 = {
		.interface_id     = 'S',
		.dxfer_direction  = SG_DXFER_FROM_DEV,
		.cmd_len          = 10,
		.mx_sb_len        = sizeof(sense),
		.iovec_count      = 0,
		.dxfer_len        = 8,
		.dxferp           = buffer.data,
		.cmdp             = cdb1,
		.sbp              = sense,
		.timeout          = (uint32_t)-1,
		.flags            = SG_FLAG_DIRECT_IO,
		.pack_id          = 0,
		.usr_ptr          = NULL
    };

	fd = SYSF_open(theHDDTrans.pc_strDeviceSSD, O_RDWR | O_NONBLOCK, 0777);
	if (fd < 0) {
		printf( "SYSF_open ERR errno:%d\n", SYSF_errnoGet() );
		return False;
	}
	ret = SYSF_ioctl(fd, SG_IO, (intptr_t)&sg1);
    if ((ret != 0) || (sg1.status != 0)) {
		SYSF_close(fd);
		printf( "SYSF_ioctl ERR errno:%d\n",SYSF_errnoGet() );
		return False;
	}
	// 2014.12.10 ベースのバージョン情報文字列に「IDENTIFIER, MINOR VERSION, BETA VERSION」を上書き
	strncpy(version,THRD_SATA_CONTROLER_VERSION,24);
	sprintf(&version[10],"%02X-G00-%02X(%02X)",buffer.data[0], buffer.data[6], buffer.data[7]);	// IDENTIFIER, MINOR VERSION, BETA VERSION.

#if 0
	sprintf(buffer,"%02x%02x%02x%02x%02x%02x%02x%02x",buffer[0],buffer[1],buffer[2],buffer[3],buffer[4],buffer[5],buffer[6],buffer[7]);
	printf("buffer : %s\n",buffer);
#endif

	// チェックサム取得.
	uint32_t addr = 0x1fffe;	// 2015/2/6 CQ.509868 堀井 チェックサム位置変更.
	uint32_t size = 2;
	uchar cdb2[10] = {
		0x3C,
		0x01,
		0x22,
		0x00,
		(uchar)(addr >> 0),
		(uchar)(addr >> 8),
		(uchar)(addr >> 16),
		(uchar)(size >> 0),
		(uchar)(size >> 8),
	};

	sg_io_hdr_t sg2 = {
		.interface_id     = 'S',
		.dxfer_direction  = SG_DXFER_FROM_DEV,
		.cmd_len          = 10,
		.mx_sb_len        = sizeof(sense),
		.iovec_count      = 0,
		.dxfer_len        = 2,
		.dxferp           = buffer.data,
		.cmdp             = cdb2,
		.sbp              = sense,
		.timeout          = (uint32_t)-1,
		.flags            = SG_FLAG_DIRECT_IO,
		.pack_id          = 0,
		.usr_ptr          = NULL
    };

	ret = SYSF_ioctl(fd, SG_IO, (intptr_t)&sg2);
    if ((ret != 0) || (sg2.status != 0)) {
		SYSF_close(fd);
		printf( "SYSF_ioctl ERR errno:%d\n",SYSF_errnoGet() );
		return False;
	}
	SYSF_close(fd);
	
	*chksum = *buffer.sum;

	return True;

#else
	return False;
#endif
}
// ================================================================================================.
//! <!--機能--> SSDコントローラデバイスID取得関数.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return THRE_SSD_DEVID SSDのデバイスID.  取得失敗時：THRE_SSD_DEVICEID_NONE. 成功時：SSDのデバイスID.
//! <!--詳細-->.
//! 装着SSDのデバイスIDを取得する.
//! <!--履歴-->.
//  2021/02/25 Sky		[IT6_2.1(新SSD対応)] SSD追加対応.
//  2022/02/07 Sky		112層SSD 対応（KIOXIAバージョン表示）.
// ================================================================================================.
THRE_SSD_DEVID THRF_getSsdDeviceIDInfo()
{
	size_t si_Ret = 0;

	// SSDデバイスID格納ファイルをオープン.
	FILE *pF_File = fopen( THRD_DEVICEID_FILE, "r" );
	if(pF_File == NULL){
		printf("%s:FW file open failed.\n", __FUNCTION__);
		return THRE_SSD_KIND_NONE;
	}
	// 読み出したバージョン情報(0x****)から****を取り出すため、読み出し位置を移動.
	si_Ret = fseek(pF_File,THRD_PREIDSIZE,SEEK_SET);
	if(si_Ret < 0){
		fclose(pF_File);
		printf("%s:fseek error(%d).\n", __FUNCTION__,si_Ret);
		return THRE_SSD_DEVICEID_NONE;
	}	
	// SSDデバイスID格納ファイルから取得した値からSSDデバイスIDの値(****)を取得.
	char sc_DeviceID[THRD_DEVICEID_SIZE + 1] = {0};
	si_Ret = fread(sc_DeviceID, sizeof(char), THRD_DEVICEID_SIZE, pF_File);
	if( si_Ret <= 0  ){
		fclose(pF_File);
		printf("%s:fread failed(%d).\n", __FUNCTION__,si_Ret);
		return THRE_SSD_DEVICEID_NONE;
	}
	fclose(pF_File); // SSDデバイスID格納ファイルをClose.
	
	uint32_t ui32_DevID = 0; // 文字列から数値変換後の値格納変数.
	int si_ScanRet = sscanf(sc_DeviceID,"%x",&ui32_DevID); // 文字列から数値へ変換.
	if(si_ScanRet == EOF){
		printf("Change string to Hex error(%s).\n",sc_DeviceID);
		return THRE_SSD_DEVICEID_NONE;
	}
	return ui32_DevID;
}
// ================================================================================================.
//! <!--機能--> SSD Phison112層チェック関数.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return Bool True:Phison112層 / False:Phison112層以外.
//! <!--詳細-->.
//! SSD FWバージョンの情報文字列を取得(fr行)を取得し、Phison112層かチェックする.
//! <!--履歴-->.
//  2022/01/24 Sky		[IT6_2.2MR2 Phison112層対応].
// ================================================================================================.
Bool THRF_checkSsdDInfoPhison112()
{
#define	PHISON_FWVERSION_SIZE	(18)
	char sc_VerInfo[PHISON_FWVERSION_SIZE + 1] = { 0 };
	Bool b_Ret = False;
	FILE *pF_File;
	size_t st_Ret;

	// SSD FWバージョンの情報文字列を取得(fr行).
	//バージョン情報を読み出す.
	pF_File = popen(THRD_SSDCTLFW_INFO, "r");
	if (pF_File == NULL) {
		printf("%s:popen error.\n", __FUNCTION__);
		return False;
	}
	st_Ret = fread(sc_VerInfo, sizeof(char), PHISON_FWVERSION_SIZE, pF_File);
	pclose(pF_File);
	if (st_Ret < PHISON_FWVERSION_SIZE ) {
		printf("%s:fread error.(GetLen:%lu)\n", __FUNCTION__, st_Ret );
		return False;
	}
	//Phison112層の固有文字列をチェック.
	if ( strstr(sc_VerInfo, THRD_SSDCTL_KEY_PHISON112) != NULL ) {
		b_Ret = True;
	}
	return b_Ret;
}
// ================================================================================================.
//! <!--機能--> SSDコントローラ種別取得関数.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return THRE_SSD_TYPE SSDの種別.  取得失敗時：THRE_SSD_KIND_NONE. 成功時：SSDの種別.
//! <!--詳細-->.
//! 装着SSDの種別(種類)を取得する.
//! <!--履歴-->.
//  2018/05/11 橋本裕	[Eagle][制御構成変更 No.33] Subset OtherFW-DL対応 SSDコントローラFW書き換え追加対応.
//  2020/01/29 Sky井柳	[IT6_2.0MR(新SSD対応MR)] SSD追加対応.
//  2021/02/25 Sky		[IT6_2.1(新SSD対応)] SSD追加対応.
//  2022/01/24 Sky		[IT6_2.2MR2 Phison112層対応].
//  2022/02/07 Sky		112層SSD 対応（KIOXIAバージョン表示）.
// ================================================================================================.
THRE_SSD_KIND THRF_getSsdKindInfo()
{
	static THRE_SSD_KIND  e_SsdType = THRE_SSD_KIND_NONE; // ベンダID格納変数.
	static Bool bl_Flag = False; // 本関数を1回でもコールされたかを管理する変数.
							  	 // True:1回コールされた、False:1度もコールされていない.
	size_t si_Ret = 0;
	
	// すでに本関数コール済みであればベンダID格納変数に格納された結果を返す.
	if(bl_Flag == True){
		return e_SsdType;
	}
	bl_Flag = True; // 本関数コールされたため、フラグをセットする.
	
	// SSDベンダID格納ファイルをオープン.
	FILE *pF_File = fopen( THRD_VENDERID_FILE, "r" );
	if(pF_File == NULL){
		printf("%s:FW file open failed.\n", __FUNCTION__);
		return THRE_SSD_KIND_NONE;
	}
	// 読み出したバージョン情報(0x****)から****を取り出すため、読み出し位置を移動.
	si_Ret = fseek(pF_File,THRD_PREIDSIZE,SEEK_SET);
	if(si_Ret < 0){
		fclose(pF_File);
		printf("%s:fseek error(%d).\n", __FUNCTION__,si_Ret);
		return THRE_SSD_KIND_NONE;
	}	
	// SSDベンダID格納ファイルから取得した値からSSDベンダIDの値(****)を取得.
	char sc_VenderID[THRD_VENDID_SIZE + 1] = {0};
	si_Ret = fread(sc_VenderID, sizeof(char), THRD_VENDID_SIZE, pF_File);
	if( si_Ret <= 0  ){
		fclose(pF_File);
		printf("%s:fread failed(%d).\n", __FUNCTION__,si_Ret);
		return THRE_SSD_KIND_NONE;
	}
	fclose(pF_File); // SSDベンダID格納ファイルをClose.
	
	// 読み出したデータは文字列であるため、数値に変換.
	uint32_t ui32_VendID = 0; // 文字列から数値変換後の値格納変数.
	int si_ScanRet = sscanf(sc_VenderID,"%x",&ui32_VendID); // 文字列から数値へ変換.
	if(si_ScanRet == EOF){
		printf("Change string to Hex error(%d).\n",sc_VenderID);
		return THRE_SSD_KIND_NONE;
	}
	// SSDベンダIDチェック.
	switch(ui32_VendID){
		case THRE_SSD_VENDID_PHISON: // 装着SSDがPhisonの場合.
			if ( THRE_SSD_DEVICEID_PHISON96 == THRF_getSsdDeviceIDInfo() ) {
				// Phison製SSD 96層と112層のDeviceIDは同じ(0x5013).
				// FWバージョンの固定部をチェックし、96層と112層を判別する.
				//   EDFQG:112層.
				//   上記以外(EDFMQ):96層.
				if (THRF_checkSsdDInfoPhison112() == True) {
					e_SsdType = THRE_SSD_KIND_PHISON112;// Phison112層であることを変数にセット.
					printf("%s:SSD Kind is Phison112(%d).\n", __FUNCTION__,e_SsdType);
				} else {
					e_SsdType = THRE_SSD_KIND_PHISON96; // Phison96層であることを変数にセット.
					printf("%s:SSD Kind is Phison96(%d).\n", __FUNCTION__,e_SsdType);
				}
			}else {
				e_SsdType = THRE_SSD_KIND_PHISON; // Phisonであることを変数にセット.
				printf("%s:SSD Kind is Phison(%d).\n", __FUNCTION__,e_SsdType);
			}
			break;
		case THRE_SSD_VENDID_TOSHIBA: // 装着SSDがToshibaの場合.
			e_SsdType = THRE_SSD_KIND_TOSHIBA; // Toshibaであることを変数にセット.
			printf("%s:SSD Kind is Toshiba(%d).\n", __FUNCTION__,e_SsdType);
			break;
		case THRE_SSD_VENDID_KIOXIA: // 装着SSDがKioxia(Toshiba)の場合.
			if (THRE_SSD_DEVICEID_KIOXIA112 == THRF_getSsdDeviceIDInfo() ) {
				e_SsdType = THRE_SSD_KIND_KIOXIA112; // Kioxia112層であることを変数にセット.
				printf("%s:SSD Kind is Kioxia112(Toshiba)(%d).\n", __FUNCTION__,e_SsdType);
			} else {
				e_SsdType = THRE_SSD_KIND_KIOXIA; // Kioxiaであることを変数にセット.
				printf("%s:SSD Kind is Kioxia(Toshiba)(%d).\n", __FUNCTION__,e_SsdType);
			}
			break;
		case THRE_SSD_VENDID_MICRON: // 装着SSDがMicronの場合.
			e_SsdType = THRE_SSD_KIND_MICRON; // Micronであることを変数にセット.
			printf("%s:SSD Kind is Micron(%d).\n", __FUNCTION__,e_SsdType);
			break;
		default: // 装着SSDが不明である場合.
			printf("%s:SSD Kind is None(Vender ID:%d).\n", __FUNCTION__,ui32_VendID);
	}
	return e_SsdType;
}
	
// ================================================================================================.
//! <!--機能--> SSDからのバージョン番号文字列の取得.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param THRE_SSD_KIND e_SsdInfo [IN] SSD種別.
//! \param char* psc_Version [OUT] SSDコントローラFWバージョン番号文字列ポインタ.
//! <!--戻値-->.
//! \return Bool True:正常終了 / False:エラー.
//! <!--詳細-->.
//! SSDコントローラHWのFWバージョンは、以下のコマンドにより.
//! "nvme -id-ctrl /dev/kmsda".
//! 得られる情報群の"fr"行の情報となる.
//! "fr"行の検索文字列（メーカーごとに異なる）以降の文字列が、バージョン番号文字列となる.
//! 取得される番号文字列は、使用する形式"***"(*は数値)の形式と異なる場合は.
//! 使用する形式"***"の形に成型して、バージョン番号文字列として出力する.
//! <!--履歴-->.
//  2020/01/29 Sky井柳	[IT6_2.0MR(新SSD対応MR)] SSD追加対応 新規作成.
//  2021/03/04 Sky	[IT6_2.1MR1 PhisonSSD 96層追加対応.
//  2022/01/24 Sky		[IT6_2.2MR2 Phison112層対応].
//  2022/02/07 Sky		112層SSD 対応（KIOXIAバージョン表示）.
// ================================================================================================.
Bool THRL_getSSDCtrlVerStr( THRE_SSD_KIND e_SsdInfo, char * psc_VerStr )
{
	// パラメタチェック.
	if( psc_VerStr == NULL ){
		printf("%s:psc_VerStr NULL.\n", __FUNCTION__);
		return False;
	}
	
	char *psc_SrchKey = NULL;	// 検索文字列:SSD種別によって違う.
	switch(e_SsdInfo){
		case THRE_SSD_KIND_PHISON:
			psc_SrchKey = THRD_SSDCTL_KEY_PHISON;	// "E8FT".
			break;
		case THRE_SSD_KIND_TOSHIBA:
			psc_SrchKey = THRD_SSDCTL_KEY_TOSHIBA;	// "ADGA1".
			break;
		case THRE_SSD_KIND_KIOXIA:
			psc_SrchKey = THRD_SSDCTL_KEY_KIOXIA;	// "AEGA1".
			break;
		case THRE_SSD_KIND_MICRON:
			psc_SrchKey = THRD_SSDCTL_KEY_MICRON;	// "P4MU".
			break;
		case THRE_SSD_KIND_PHISON96:
			psc_SrchKey = THRD_SSDCTL_KEY_PHISON96;	// "EDFMQ".
			break;
		case THRE_SSD_KIND_PHISON112:
			psc_SrchKey = THRD_SSDCTL_KEY_PHISON112;// "EDFQG".
			break;
		case THRE_SSD_KIND_KIOXIA112:
			psc_SrchKey = THRD_SSDCTL_KEY_KIOXIA112;// "ANGA1".
			break;
		default:
			printf( "%s:SSD Info none (e_SsdInfo:%d).\n", __FUNCTION__,e_SsdInfo);
		return False;
	}
	
	//バージョン情報を読み出す.
	FILE* pF_File = popen(THRD_SSDCTLFW_INFO,"r");
	if(pF_File == NULL){ 
		printf("%s:popen error.\n", __FUNCTION__);
		return False;
	}
	// SSD FWバージョンの情報文字列を取得(fr行).
	char sc_VerInfo[THRD_SSDCTL_HWMAXSIZE + 1] = {0}; // バージョン情報格納変数.
	size_t st_VerLenMin = strlen(psc_SrchKey) + THRD_SSDCTL_VERSIZE;	// 最低バージョンの情報文字列長.
		// (検索対象文字列長＋バージョン番号文字列長) 以上なければ、検索しても意味がないため確認する長さとする.
	size_t st_Ret = fread(sc_VerInfo,sizeof(char),THRD_SSDCTL_HWMAXSIZE,pF_File);
	Bool b_Phison96up = False;
	if ( (e_SsdInfo == THRE_SSD_KIND_PHISON96) || (e_SsdInfo == THRE_SSD_KIND_PHISON112) ) {
		b_Phison96up = True;
	}
	if( ((b_Phison96up == False) && ( st_Ret < st_VerLenMin )) ||
		((b_Phison96up == True) && ( st_Ret < (st_VerLenMin-1)) ))	{
		printf("%s:fread error.(e_SsdInfo:%d)(GetLen:%lu)\n", __FUNCTION__,e_SsdInfo, st_Ret );
		pclose(pF_File);
		return False;
	}
	pclose(pF_File); // popenしたファイルをクローズ.
	
	// 取得した情報文字列から検索文字列の位置(ポインタ)を取得.
	char* psc_SsdVer = (char *)memmem(sc_VerInfo,THRD_SSDCTL_HWMAXSIZE,psc_SrchKey,strlen(psc_SrchKey));
	if(psc_SsdVer == NULL){
		printf("%s:get SSDCTL version error.[%s]\n", __FUNCTION__, sc_VerInfo);
		return False;
	}
	
	// バージョン情報から検索文字列以降のバージョン番号を取得.
	char sc_VerBuffer[THRD_SSDCTL_VERSIZE + 1] = {0};
	strncpy(sc_VerBuffer,&psc_SsdVer[strlen(psc_SrchKey)],THRD_SSDCTL_VERSIZE);
	
	// 検索文字列以降のバージョン文字列の形式による成型.
	switch(e_SsdInfo){
	case THRE_SSD_KIND_PHISON:
		// Phisonは、取得された形式が"**.*"であるため、"."の確認と、"***"への成型を行う.
		if( sc_VerBuffer[2] != '.' ){
			printf("%s:version format error[%s].\n", __FUNCTION__, sc_VerBuffer );
			return False;
		}
		sc_VerBuffer[2] = sc_VerBuffer[3];	// 一つ詰める.
		sc_VerBuffer[3] = NULL;
		break;
	case THRE_SSD_KIND_PHISON96:
	case THRE_SSD_KIND_PHISON112:
		// Phisonは、取得された形式が"**.*","*.*"また区切りコードが'.',"T","A","K"があるため判定する.
		// ('.'が2文字目に存在するかチェックすることにより判定('.'は'T''A''K'も可能とする)).
		if( ( sc_VerBuffer[1] == '.' ) ||
			( sc_VerBuffer[1] == 'T' ) ||
			( sc_VerBuffer[1] == 'A' ) ||
			( sc_VerBuffer[1] == 'K' ) ) {
			// Phisonは、取得された形式が"*.*"であるため、"."の確認と、"0**"への成型を行う.
			sc_VerBuffer[1] = sc_VerBuffer[0];	// 一つ下げる.
			sc_VerBuffer[0] = '0';
			sc_VerBuffer[3] = NULL;
			break;
		}
		if ( st_Ret < (st_VerLenMin) )	{
			printf("%s:fread error.(e_SsdInfo:%d)(GetLen:%lu)\n", __FUNCTION__,e_SsdInfo, st_Ret );
			return False;
		}
		if( ( sc_VerBuffer[2] == '.' ) ||
			( sc_VerBuffer[2] == 'T' ) ||
			( sc_VerBuffer[2] == 'A' ) ||
			( sc_VerBuffer[2] == 'K' ) ) {
			// Phisonは、取得された形式が"**.*"であるため、"."の確認と、"***"への成型を行う.
			sc_VerBuffer[2] = sc_VerBuffer[3];	// 一つ詰める.
			sc_VerBuffer[3] = NULL;
		} else {
			printf("%s:version format error[%s].\n", __FUNCTION__, sc_VerBuffer );
			return False;
		}
		break;
	case THRE_SSD_KIND_TOSHIBA:
	case THRE_SSD_KIND_KIOXIA:
	case THRE_SSD_KIND_KIOXIA112:
	case THRE_SSD_KIND_MICRON:
		// 取得された形式は"***"であるため、変更なし.
		break;
	default:
		printf( "%s:SSD Info none (e_SsdInfo:%d).\n", __FUNCTION__,e_SsdInfo);			
		return False;
	}

	// 戻り設定
	strncpy( psc_VerStr,sc_VerBuffer,THRD_SSDCTL_VERSIZE );
	
	return True;
}

// ================================================================================================.
//! <!--機能--> SSDコントローラFWのバージョン取得.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param char* psc_Version [OUT] SSDコントローラFWバージョン文字列ポインタ.
//! <!--戻値-->.
//! \return Bool True:正常終了 / False:エラー.
//! <!--詳細-->.
//! 各ベンダ用のバージョン情報取得関数をコールし、SSDコントローラからFWバージョンの取得を行う.
//! ベースバージョンに対して下記を変更する.
//! AA2J0Y0-650*-G0*-**(00).
//! 650*	：装着されるSSDのベンダ情報.
//! G0*-**	：装着されるSSDのバージョン情報.
//! <!--履歴-->.
//  2018/05/11 橋本裕　[Eagle][制御構成変更 No.33] Subset OtherFW-DL対応 SSDコントローラFW書き換え追加対応.
//  2020/01/29 Sky井柳	[IT6_2.0MR(新SSD対応MR)] SSD追加対応 処理共通化による変更.
// ================================================================================================.
Bool THRF_getSSDControlerVersion(char* psc_Version)
{
#if !defined(SIMULATOR)
	// 引数チェックを行う.
	if( psc_Version == NULL ){
		printf("%s:parameter NULL.\n", __FUNCTION__);
		return False;
	}

	// 装着SSDの種類(種別)取得.
	THRE_SSD_KIND e_SsdInfo = THRF_getSsdKindInfo();
	
	// SSDのバージョン番号文字列の取得.
	char sc_VerNoStr[THRD_SSDCTL_VERSIZE + 1] = {0};  // バージョン番号文字列.
	Bool bl_Ret = THRL_getSSDCtrlVerStr( e_SsdInfo, sc_VerNoStr );
	if(bl_Ret == False){
		printf("%s:THRL_getSSDCtrlVerStr error.\n", __FUNCTION__);
		return False;
	}

	// バージョン情報設定.
		// ベースバージョンの文字列をセット.
	strncpy(psc_Version,THRD_SSD_CONTROLER_VERSION,strlen(THRD_SSD_CONTROLER_VERSION));

		// SSD種別.
	char sc_SsdKind = (char)(e_SsdInfo + '0'); // 数字から数字文字列へ変換.
	psc_Version[11] = sc_SsdKind;		// AA2J0Y0-650*-G00-00(00).
										//            ↑上記にSSD種別をセット.

		// バージョン番号文字列の"***"(*は数値)は、それぞれ順に①②③として、以下のように設定.
	psc_Version[15] = sc_VerNoStr[0];	// AA2J0Y0-650?-G0*-**(00).
										//                ↑上記に①をセット.
	psc_Version[17] = sc_VerNoStr[1];	// AA2J0Y0-650?-G0*-**(00).
										//                 ↑上記に②をセット.
	psc_Version[18] = sc_VerNoStr[2];	// AA2J0Y0-650?-G0*-**(00).
										//                   ↑上記に③をセット.

	return True;
#else // !defined(SIMULATOR).
	return False;
#endif // !defined(SIMULATOR).
}

// 20150427 Realtek PHY問題対応.
// ================================================================================================.
//! <!--機能--> Phyレジスタ取得.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数--> c_devicePath	ファイルパス.
//! 			sc_buf			設定値.
//!				offset			オフセット値.
//! <!--戻値--> なし.
//! <!--詳細-->.
//! Phyのレジスタ値を取得.
//! <!--履歴-->.
//  2015.04.27	山口	新規作成.
// ================================================================================================.
#if defined(TORNADO) && !defined(FUM_LINUX)	/* 実機で有効 */
static void THRF_getPhysetting(char *c_devicePath, ushort* sc_buf , uint32_t offset)
{
	FILE* fp_phy;
	size_t ret = 0;	/*	2017/01/20 uint32_tからsize_tに変更 */
	// エラーチェック.
	if( c_devicePath == NULL || sc_buf == NULL || offset > (31*2) ){
		printf("%s: ERR \n", __FUNCTION__);
		return;
	}
	fp_phy = SYSF_fopen( c_devicePath, "r");
	if( fp_phy == NULL ) {
        printf("%s: SYSF_fopen ERR errno:%d\n", __FUNCTION__, SYSF_errnoGet());
        return;
    }
	ret = fseek( fp_phy, offset, SEEK_SET);
	if( ret < 0 ){
		printf("%s: SYSF_lseek ERR errno:%d\n", __FUNCTION__, SYSF_errnoGet());
		fclose(fp_phy);
		return;
	}
	ret = fread(sc_buf, 2, 1, fp_phy);
	if( ret < 0 ){
		printf("%s: SYSF_write ERR errno:%d\n", __FUNCTION__, SYSF_errnoGet());
		fclose(fp_phy);
		return;
	}
	fclose(fp_phy);
	return;
}
#endif // defined(TORNADO) && !defined(FUM_LINUX)	/* 実機で有効 */.
// ================================================================================================.
//! <!--機能--> Phyレジスタ書き込み.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数--> c_devicePath	ファイルパス.
//! 			us_buf			設定値.
//!				offset			オフセット値.
//! <!--戻値--> なし.
//! <!--詳細-->.
//! Phyのレジスタ値へ書き込み.
//! <!--履歴-->.
//  2015.04.27	山口	新規作成.
// ================================================================================================.
#if defined(TORNADO) && !defined(FUM_LINUX)	/* 実機で有効 */
static void THRF_setPhysetting(char *c_devicePath, ushort us_buf , uint32_t offset)
{
	FILE* fp_phy;
	size_t ret = 0;
	
	// エラーチェック.
	if( c_devicePath == NULL || offset > (31*2) ){
		printf("%s: ERR \n", __FUNCTION__);
		return;
	}
	fp_phy = SYSF_fopen( c_devicePath, "w");
	if( fp_phy == NULL ) {
        printf("%s: SYSF_fopen ERR errno:%d\n", __FUNCTION__, SYSF_errnoGet());
        return;
    }
	ret = fseek( fp_phy, offset, SEEK_SET);
	if( ret < 0 ){
		printf("%s: SYSF_lseek ERR errno:%d\n", __FUNCTION__, SYSF_errnoGet());
		fclose(fp_phy);
		return;
	}
	ret = fwrite(&us_buf, 2, 1, fp_phy);
	if( ret < 0 ){
		printf("%s: SYSF_write ERR errno:%d\n", __FUNCTION__, SYSF_errnoGet());
		fclose(fp_phy);
		return;
	}
	fclose(fp_phy);
	return;
}
#endif // defined(TORNADO) && !defined(FUM_LINUX)	/* 実機で有効 */.
// ================================================================================================.
//! <!--機能--> Phyレジスタ書き込み要求.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数--> なし.
//! <!--戻値--> なし.
//! <!--詳細-->.
//! Phyのレジスタ値を書き換える（リンクダウンが発生する問題の修正）
//! Realtekから提供されたパッチコードを実行する
//! <!--履歴-->.
//  2015.04.27	山口	新規作成.
// ================================================================================================.
void THRF_setLinkDownRecoverPhysetting(){
#if defined(TORNADO) && !defined(FUM_LINUX)	/* 実機で有効 */
	ushort us_regs_val = 0;
	ushort us_cnt = 0;
	// 2018/10/12 BSP 山口 1047494 Realtekから提供されたパッチコードの廃止.
#if 1
	return;
#endif
	// Realtekから提供されたパッチコード(STEP1).
	THRF_setPhysetting("/proc/ndebug/page", 0x0B82 , 0*2 );
	THRF_setPhysetting("/proc/ndebug/regs", 0x0010 , 16*2 );
	THRF_setPhysetting("/proc/ndebug/page", 0x0B80 , 0*2 );
	
	// STEP1が正常に設定できたかの確認.
	while(1){ 
		THRF_getPhysetting("/proc/ndebug/regs", &us_regs_val , 16*2 );
		if( us_regs_val & 0x40 ){
			break;
		}
		SYSF_taskDelay( (int32_t)(SYSF_sysClkRateGet() * 10 / 1000 ) );	// 10ms wait.
		us_cnt++;
		if( us_cnt > 100 ){		// 1s以上waitでエラー
			printf("Phy Register Setting Time over\n");
			THRL_ThAssertLog( CFA14_DetailCode0x2009 );			// C-D050詳細コード登録.
			return;
		}
	}
	// Realtekから提供されたパッチコード(STEP2).
	THRF_setPhysetting("/proc/ndebug/page", 0x0A43 , 0*2 );
	THRF_setPhysetting("/proc/ndebug/regs", 0x8146 , 27*2 );
	THRF_setPhysetting("/proc/ndebug/regs", 0x9500 , 28*2 );
	THRF_setPhysetting("/proc/ndebug/page", 0x0B82 , 0*2 );
	THRF_setPhysetting("/proc/ndebug/regs", 0x0001 , 23*2 );
	THRF_setPhysetting("/proc/ndebug/page", 0x0A43 , 0*2 );
	THRF_setPhysetting("/proc/ndebug/regs", 0xB820 , 27*2 );
	THRF_setPhysetting("/proc/ndebug/regs", 0x0290 , 28*2 );
	THRF_setPhysetting("/proc/ndebug/regs", 0xA012 , 27*2 );
	THRF_setPhysetting("/proc/ndebug/regs", 0x0000 , 28*2 );
	THRF_setPhysetting("/proc/ndebug/regs", 0xA014 , 27*2 );
	THRF_setPhysetting("/proc/ndebug/regs", 0x2C04 , 28*2 );
	THRF_setPhysetting("/proc/ndebug/regs", 0x2C0B , 28*2 );
	THRF_setPhysetting("/proc/ndebug/regs", 0x2C0B , 28*2 );
	THRF_setPhysetting("/proc/ndebug/regs", 0x2C0B , 28*2 );
	THRF_setPhysetting("/proc/ndebug/regs", 0x0000 , 28*2 );
	THRF_setPhysetting("/proc/ndebug/regs", 0x0000 , 28*2 );
	THRF_setPhysetting("/proc/ndebug/regs", 0x0000 , 28*2 );
	THRF_setPhysetting("/proc/ndebug/regs", 0x0000 , 28*2 );

	THRF_setPhysetting("/proc/ndebug/regs", 0x0000 , 28*2 );
	THRF_setPhysetting("/proc/ndebug/regs", 0x8804 , 28*2 );
	THRF_setPhysetting("/proc/ndebug/regs", 0x234B , 28*2 );
	THRF_setPhysetting("/proc/ndebug/regs", 0xA01A , 27*2 );
	THRF_setPhysetting("/proc/ndebug/regs", 0x0000 , 28*2 );
	THRF_setPhysetting("/proc/ndebug/regs", 0xA006 , 27*2 );
	THRF_setPhysetting("/proc/ndebug/regs", 0x0FFF , 28*2 );
	THRF_setPhysetting("/proc/ndebug/regs", 0xA004 , 27*2 );
	THRF_setPhysetting("/proc/ndebug/regs", 0x0FFF , 28*2 );
	THRF_setPhysetting("/proc/ndebug/regs", 0xA002 , 27*2 );
	THRF_setPhysetting("/proc/ndebug/regs", 0x0FFF , 28*2 );
	THRF_setPhysetting("/proc/ndebug/regs", 0xA000 , 27*2 );
	THRF_setPhysetting("/proc/ndebug/regs", 0x134A , 28*2 );
	THRF_setPhysetting("/proc/ndebug/regs", 0xB820 , 27*2 );
	THRF_setPhysetting("/proc/ndebug/regs", 0x0210 , 28*2 );
	THRF_setPhysetting("/proc/ndebug/regs", 0x0000 , 27*2 );
	THRF_setPhysetting("/proc/ndebug/regs", 0x0000 , 28*2 );
	THRF_setPhysetting("/proc/ndebug/page", 0x0B82 , 0*2 );
	THRF_setPhysetting("/proc/ndebug/regs", 0x0000 , 23*2 );
	THRF_setPhysetting("/proc/ndebug/page", 0x0A43 , 0*2 );
	THRF_setPhysetting("/proc/ndebug/regs", 0x8146 , 27*2 );
	THRF_setPhysetting("/proc/ndebug/regs", 0x9500 , 28*2 );
	THRF_setPhysetting("/proc/ndebug/page", 0x0B82 , 0*2 );
	THRF_setPhysetting("/proc/ndebug/regs", 0x0000 , 16*2 );
#endif // defined(TORNADO) && !defined(FUM_LINUX)	/* 実機で有効 */.
}

// ================================================================================================.
//! <!--機能--> USBFaxボードのリセット処理
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数--> なし.
//! <!--戻値--> なし.
//! <!--詳細-->.
//! USBFAXが認識されない問題対応
//! USB-HUBをリセットする事により認識しないUSBFaxボードを再接続させる。
//! <!--履歴-->.
//  2015.11.11	松岡	新規作成.
//  2015.12.08	近藤	CQ.523223 MinervaSBK/ZeusS_ZX0でもバスリセットが動作するように修正.
//  2016/03/11	Harada	CQ.526133 USBデバイスファイル有無チェックを削除、常にバスリセット実行するように修正.
//	2017/08/30	山口	Eagle_Faxバスリセット_電源入れ直し	FAXのポート指定を Eagle/Emu800ように修正.
//	2018/06/07	佐藤	DenebMLKに対応.
//	2019/04/18	Sky大喜	Faxバスリセット電源入れ直し制御対応_EagleZ+/EagleZ+BK/SparrowBK/HeliosMLK対応.
// ================================================================================================.
void THRF_USBFaxBoardBusReset()
{
#if defined(TORNADO) && !defined(FUM_LINUX)	/* 実機で有効 */

	static SEM_ID semID_BusReset = NULL;
	static double s_wTime = 0;
	// セマフォ取得.
	if( semID_BusReset == NULL ){
		semID_BusReset = SYSF_semMCreate(SEM_Q_PRIORITY | SEM_DELETE_SAFE | SEM_INVERSION_SAFE);
	}
	SYSF_semTake( semID_BusReset, WAIT_FOREVER );
	if (!s_wTime) {
		printf("THRF_USBFaxBoardBusReset first\n");
		s_wTime = gettimeofday_sec();
	}
	else {
		/* 4回線分コールされる可能性があるので1分内にコールされた場合はリターンする */
		if ( 60 > (gettimeofday_sec() - s_wTime) ) {
			printf("THRF_USBFaxBoardBusReset return\n");
			SYSF_semGive( semID_BusReset );
			return;
		}
		// 60秒経過した後は一度取り直す
		printf("THRF_USBFaxBoardBusReset get\n");
		s_wTime = gettimeofday_sec();
	}
// Sparrow(MFP/AIO/SFP)、DenebMLK、SparrowBK、HeliosMLK各実機の場合(SparrowEmu/DenebMLKEmuはここに入らない).
#if ( defined( DEF_SparrowCus ) || defined( DEF_DenebMLKCus ) || defined( DEF_SparrowBKCus ) || defined( DEF_HeliosMLKCus ) )  && !defined( DEF_S800Emu )
	// Sparrow、DenebMLK、SparrowBK、HeliosMLK.
	system("echo -n 3-1.4 > /sys/bus/usb/drivers/usb/unbind");	// FAX1、FAX拡張基板に対してunbind.
	system("echo -n 3-1.4 > /sys/bus/usb/drivers/usb/bind");	// FAX1、FAX拡張基板に対してbind.

	// SparrowEmu機、DenebMLKEmu機は↓を通す.
#else  // #if ( defined( DEF_SparrowCus ) || defined( DEF_DenebMLKCus ) || defined( DEF_SparrowBKCus ) || defined( DEF_HeliosMLKCus ) )  && !defined( DEF_S800Emu ).
	// EagleL/EagleH/EagleLBK/EagleHBK/EagleZ+/EagleZ+BK/Emu800.
	// 2018/12/06 BSP 山口 AR.1068776 B195問題 FAXのポートを修正.
	system("echo -n 3-1.1.1 > /sys/bus/usb/drivers/usb/unbind");	// FAX1、FAX拡張基板に対してunbind.
	system("echo -n 3-1.1.1 > /sys/bus/usb/drivers/usb/bind");	// FAX1、FAX拡張基板に対してbind.
	system("echo -n 3-1.1.2 > /sys/bus/usb/drivers/usb/unbind");	// FAX2に対してunbind.
	system("echo -n 3-1.1.2 > /sys/bus/usb/drivers/usb/bind");	// FAX2に対してbind.
#endif // #if ( defined( DEF_SparrowCus ) || defined( DEF_DenebMLKCus ) || defined( DEF_SparrowBKCus ) || defined( DEF_HeliosMLKCus ) )  && !defined( DEF_S800Emu ).
	SYSF_semGive( semID_BusReset );

#endif // defined(TORNADO) && !defined(FUM_LINUX)	/* 実機で有効 */.
}

// ================================================================================================.
//! <!--機能--> 経過秒取得関数
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数--> なし.
//! <!--戻値--> なし.
//! <!--詳細-->.
//! USBFAXが認識されない問題対応
//! 経過秒を取得する関数
//! <!--履歴-->.
//  2015.11.11	松岡	新規作成.
// ================================================================================================.
#if defined(TORNADO) && !defined(FUM_LINUX)	/* 実機で有効 */
double gettimeofday_sec()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec * 1e-6;
}
#endif // defined(TORNADO) && !defined(FUM_LINUX)	/* 実機で有効 */.
// ================================================================================================.
//! <!--機能--> USB-FAXボード電源OFF/ON制御. 
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数--> なし.
//! <!--戻値--> なし.
//! <!--詳細-->.
//! /WAKE_FAXMFPC信号をLoからHi、HiからLoに制御して 
//! FAXボードの電源を入れ直す. 
//! <!--履歴-->.
//  2016.02.03	近藤	新規作成.
// ================================================================================================.
void THRF_FaxBoardPowerOffOn()
{
#if defined(TORNADO) && !defined(FUM_LINUX)	/* 実機で有効 */
	static SEM_ID semID_FaxBoardPowerOffOn = NULL;
	static double s_wTime = 0;
	// セマフォ取得.
	if( semID_FaxBoardPowerOffOn == NULL ){
		semID_FaxBoardPowerOffOn = SYSF_semMCreate(SEM_Q_PRIORITY | SEM_DELETE_SAFE | SEM_INVERSION_SAFE);
	}
	SYSF_semTake( semID_FaxBoardPowerOffOn, WAIT_FOREVER );
	if (!s_wTime) {
		printf("THRF_FaxBoardPowerOffOn first\n");
		s_wTime = gettimeofday_sec();
	}
	else {
		/* 4回線分コールされる可能性があるので1分内にコールされた場合はリターンする */
		if ( 60 > (gettimeofday_sec() - s_wTime) ) {
			printf("THRF_FaxBoardPowerOffOn return\n");
			SYSF_semGive( semID_FaxBoardPowerOffOn );
			return;
		}
		// 60秒経過した後は一度取り直す
		printf("THRF_FaxBoardPowerOffOn get\n");
		s_wTime = gettimeofday_sec();
	}

	// 2016/02/04 ft_kondo CQ.524086. 
	// /WAKE_FAXMFPCの信号をLo状態から、Lo→Hi→Loと変化させる事で、FAXの電源をOFF/ONする. 
	THRC_PowerManagerDeviceSingletonIf::newInstance().SwitchPowerState( THRC_PowerDeviceID_WAKE_FAXMFPC, THRC_PowerStateID_LOW );
	SYSF_taskDelay( (int)(SYSF_sysClkRateGet() * 10 / 1000 ) );	// 10ms wait.
	THRC_PowerManagerDeviceSingletonIf::newInstance().SwitchPowerState( THRC_PowerDeviceID_WAKE_FAXMFPC, THRC_PowerStateID_NORMAL );

	SYSF_semGive( semID_FaxBoardPowerOffOn );
#endif // defined(TORNADO) && !defined(FUM_LINUX)	/* 実機で有効 */.
}
// ================================================================================================.
//! <!--機能--> USBFaxポートのリセット処理
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数--> ポート番号.
//! <!--戻値--> なし.
//! <!--詳細-->.
//! 	指定されたポートに対して「unbind」処理を行う.
//! <!--履歴-->.
//  2016.05.17	山口	新規作成.
// ================================================================================================.
void THRF_USBFaxPortLineBusReset(char *c_PortLine)
{
#if defined(TORNADO) && !defined(FUM_LINUX)	/* 実機で有効 */

	static SEM_ID semID_BusResetPort = NULL;
	char data[55] = {};
	// エラーチェック.
	if( c_PortLine == NULL ){
		printf("%s: NULL ERR \n", __FUNCTION__);
		return;
	}
	// セマフォ取得.
	if( semID_BusResetPort == NULL ){
		semID_BusResetPort = SYSF_semMCreate(SEM_Q_PRIORITY | SEM_DELETE_SAFE | SEM_INVERSION_SAFE);
	}
	SYSF_semTake( semID_BusResetPort, WAIT_FOREVER );

	snprintf(data, sizeof(data) ,"echo -n %s > /sys/bus/usb/drivers/usb/unbind", c_PortLine );	// 2016/06/02 Harada ZeusSBK 静的解析対応(ID:32276).
	printf(data);
	system(data);	// 指定ポートに対してunbind
	SYSF_semGive( semID_BusResetPort );

#endif // defined(TORNADO) && !defined(FUM_LINUX)	/* 実機で有効 */.
}

// ================================================================================================.
//! <!--機能--> USBFaxポートのリセット解除処理
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数--> ポート番号.
//! <!--戻値--> なし.
//! <!--詳細-->.
//! 	指定されたポートに対して「bind」処理を行う.
//! <!--履歴-->.
//  2021.07.20	ft_kondo OP_BTS-34852	リセット解除用API新設.
// ================================================================================================.
void THRF_USBFaxPortLineBusResetRelease(char *c_PortLine)
{
#if defined(TORNADO) && !defined(FUM_LINUX)	/* 実機で有効 */

	static SEM_ID semID_BusResetReleasePort = NULL;
	char data[55] = {};
	// エラーチェック.
	if( c_PortLine == NULL ){
		printf("%s: NULL ERR \n", __FUNCTION__);
		return;
	}
	// セマフォ取得.
	if( semID_BusResetReleasePort == NULL ){
		semID_BusResetReleasePort = SYSF_semMCreate(SEM_Q_PRIORITY | SEM_DELETE_SAFE | SEM_INVERSION_SAFE);
	}
	SYSF_semTake( semID_BusResetReleasePort, WAIT_FOREVER );

	snprintf(data, sizeof(data) ,"echo -n %s > /sys/bus/usb/drivers/usb/bind", c_PortLine );
	printf(data);
	system(data);	// 指定ポートに対してbind
	SYSF_semGive( semID_BusResetReleasePort );

#endif // defined(TORNADO) && !defined(FUM_LINUX)	/* 実機で有効 */.
}

/* 2021/01/05 Harada IT6 2.1 OP_BTS-30215 認証装置を認証しない *//* 同件として対応 *//* 市場問題 TAS3758 E301 */
// ================================================================================================.
//! <!--機能--> USBFaxポートのリセット処理
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数--> ポート番号.
//! <!--戻値--> なし.
//! <!--詳細-->.
//! 	指定されたポートに対して「unbind/bind」処理を行う.
//! <!--履歴-->.
//  2021/01/14	Harada	新規作成.
// ================================================================================================.
void THRF_USBFaxSelectPortLineBusReset(char *c_PortLine)
{
#if defined(TORNADO) && !defined(FUM_LINUX)	/* 実機で有効 */

	static SEM_ID semID_SelectBusResetPort = NULL;
	char data[65] = {};
	// エラーチェック.
	if( c_PortLine == NULL ){
		printf("%s: NULL ERR \n", __FUNCTION__);
		return;
	}
	// セマフォ取得.
	if( semID_SelectBusResetPort == NULL ){
		semID_SelectBusResetPort = SYSF_semMCreate(SEM_Q_PRIORITY | SEM_DELETE_SAFE | SEM_INVERSION_SAFE);
	}
	SYSF_semTake( semID_SelectBusResetPort, WAIT_FOREVER );

	memset(data, 0, sizeof(data));
	snprintf(data, sizeof(data)-1 ,"echo -n %s > /sys/bus/usb/drivers/usb/unbind", c_PortLine );
	system(data);	// 指定ポートに対してunbind
	memset(data, 0, sizeof(data));
	snprintf(data, sizeof(data)-1 ,"echo -n %s > /sys/bus/usb/drivers/usb/bind", c_PortLine );
	system(data);	// 指定ポートに対してbind

	memset(data, 0, sizeof(data));
	snprintf(data, sizeof(data)-1 ,"echo -n %s > /sys/bus/usb/drivers/usb/unbind & bind", c_PortLine );
	SYSF_logMsg( "(%010u)(BSP) %s : %s\n", SYSF_tickGet() , __FUNCTION__, data, 3,4,5,6 );

	SYSF_semGive( semID_SelectBusResetPort );

#endif // defined(TORNADO) && !defined(FUM_LINUX)	/* 実機で有効 */.
}
// ================================================================================================.
//! <!--機能--> カーネルメッセージバッファへ指定文字列を書き込む.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! char *string	: 文字列.
//! <!--戻値-->.
//! int 	0：エラーなし / 0未満：エラー.
//! <!--詳細-->.
//! ZeusSBK 制御構成変更_No.57 電源SubCPUモニタ対応
//! カーネルメッセージバッファへ指定文字列を書き込む.
//! <!--履歴-->.
//  2016.5.17	PS1-ft_kobayashi	新規作成.
// ================================================================================================.
int THRF_writeKernelMessage( char *string )
{
#if defined(TORNADO) && !defined(FUM_LINUX)	/* 実機で有効 */
	int fd = 0, ret = 0;
	
	if( string == NULL ){
		printf("THRF_writeKernelMessage: null argument\n");
		return -1;
	}
	
	fd = SYSF_devopen( "/dev/kmsg", O_RDWR, 0666 );
	
	if( fd < 0 ){
		printf("Failed to open kmsg!\n");
		return fd;
	}
	
	ret = SYSF_write( fd, string, strlen(string) + 1 );
	
	if( ret < 0 ){
		printf("Failed to write message to kmsg!\n");
		SYSF_devclose( fd );
		return ret;
	}
	
	printf("Writing message to kmsg finished!\n");
	SYSF_devclose( fd );
	return 0;
#else
	return -1;
#endif // defined(TORNADO) && !defined(FUM_LINUX)	/* 実機で有効 */.
}

/*================================================================================================
［機能］	CA72の状態をLPPPに設定する.(LPPPとのハンドシェイク)
--------------------------------------------------------------------------------------------------
［引数］	Bool	isStart			True：起動状態、False：未起動状態.
［戻値］	なし
［履歴］	2017/8/7 horii 瞬断対応.
================================================================================================*/
#define PMU_IOCSSET_APSTATUS		_IOW( PMU_IOC_MAGIC, 10, uint64_t )
Bool THRF_setAPStatus( Bool isStart )
{
	int fd = 0, ret = 0;
	
	fd = SYSF_devopen( "/dev/PMU0", O_RDWR, 0666 );
	if( fd < 0 ){
		printf("%s: SYSF_devopen ERR errno:%d\n", __FUNCTION__, SYSF_errnoGet());
		return False;
	}
	ret = SYSF_ioctl(fd, PMU_IOCSSET_APSTATUS, (intptr_t)&isStart);
	if( ret < 0 ){
		printf("%s: SYSF_ioctl ERR errno:%d\n", __FUNCTION__, SYSF_errnoGet());
		SYSF_close(fd);
		return False;
	}
	SYSF_close(fd);
	return True;
}
// ================================================================================================.
//! <!--機能--> 通電OFF要因ログ取得.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! void.
//! <!--戻値-->.
//! uint32_t 
//! <!--詳細-->.
//!  通電OFF要因ログ取得.
//! <!--履歴-->.
// ================================================================================================.
uint32_t THRF_getPowerOffFactor(void)
{
	char pc_state[THRD_StateID_MaxLen] = {'\0'};
	Bool bl_res = False;
	bl_res = getKernelDeviceStatusDirect( THRE_StoreDirectDeviceIX_PowerOffFactor, pc_state );
	if( bl_res == False ) {
		return 0;
	}
	return (uint32_t)atoi( pc_state );
}

// ================================================================================================.
//! <!--機能--> 通電OFF要因ログクリア.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! void.
//! <!--戻値-->.
//! Bool		True: 成功,  False: 失敗.
//! <!--詳細-->.
//!  通電OFF要因ログクリア.
//! <!--履歴-->.
// ================================================================================================.
Bool THRF_clrPowerOffFactor(void)
{
	return setKernelDeviceStatusDirect( THRE_StoreDirectDeviceIX_PowerOffFactor, "0" );
}

// ================================================================================================.
//! <!--機能--> 通電OFF要因ログクリア.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! uint32_t 通電OFF要因.
//! <!--戻値-->.
//! void.
//! <!--詳細-->.
//!  通電OFF要因ログを垂れ流しログに出力する。
//! <!--履歴-->.
// ================================================================================================.
void THRF_printPowerOffFactor(uint32_t ul_factor)
{
	const char* sc_factor_str = "no factor ( or pscpu is old version )";
	uchar uc_cnt = 0;
	uint32_t ul_bit = 1;
	// 2019/06/16 山口 AR1077356 DeepSleep復帰に3分程度かかる事がある問題.
#if defined(TORNADO) && !defined(FUM_LINUX) /*実機*/
	uint64_t ull_NewDetailCode = 0x0;
	Bool bl_ret = True;
#endif	// #if defined(TORNADO) && !defined(FUM_LINUX).
	// 2020/06/05 ft_kondo OP_BTS-24920 22019新詳細コード細分化対応 S. 
#if defined(TORNADO) && !defined(FUM_LINUX) /*実機*/
	ushort us_code = 0;
	uchar uc_ret = 0;
	// 2020/10/02 山口 OP_BTS-28170 Oops発生でLPPPがフリーズ.
	uchar uc_kernel_panic_occer = 0;
	uchar uc_kernel_panic_occer_tmp = 0;		// 2020/10/17 山口 OP_BTS-28396 SError発生時に22019詳細コード細分化.
	uchar uc_kernel_panic_occer_Info[5];		// 2022/01/18 OP_BTS-37270 山口 Oops発生時の詳細コード細分化.
	uchar uc_kernel_panic_occer_default[5];	
#endif	// #if defined(TORNADO) && !defined(FUM_LINUX).
	// 要因がなし.
	if ( 0 == ul_factor ) {
		printf( "(%010u)(BSP)[%s] %s\n", SYSF_tickGet(), __FUNCTION__, sc_factor_str );
		return;
	}
#if defined(TORNADO) && !defined(FUM_LINUX) /*実機*/
	// 2020/10/02 山口 OP_BTS-28170 Oops発生でLPPPがフリーズ.
	// Oops発生でLPPPがフリーズした場合はreboot要因識別子を3(oops発生)を登録する.
	uc_ret = THRG_getBspThrRegion( 0x003E, &uc_kernel_panic_occer, sizeof(uc_kernel_panic_occer) );
	if( ( uc_ret == THRD_GET_SPI_OK ) || ( uc_ret == THRD_GET_SPI_OK_AREA1_ERR ) || ( uc_ret == THRD_GET_SPI_OK_AREA2_ERR ) ) {
		if ( 0xFF != uc_kernel_panic_occer ) {
			uc_kernel_panic_occer_tmp = uc_kernel_panic_occer;		// 2020/10/17 山口 OP_BTS-28396 SError発生時に22019詳細コード細分化.
			uc_kernel_panic_occer = 0xFF;
			THRG_setBspThrRegion( 0x003E, &uc_kernel_panic_occer, sizeof(uc_kernel_panic_occer) );
		}
	}
	// 2022/01/18 OP_BTS-37270 山口 Oops発生時の詳細コード細分化.			
	memset(&uc_kernel_panic_occer_Info, 0x00, sizeof(uc_kernel_panic_occer_Info));
	memset(&uc_kernel_panic_occer_default, 0xff, sizeof(uc_kernel_panic_occer_default));
	
	uc_ret = THRG_getBspThrRegion( 0x0040, &uc_kernel_panic_occer_Info, sizeof(uc_kernel_panic_occer_Info) );
	if( ( uc_ret == THRD_GET_SPI_OK ) || ( uc_ret == THRD_GET_SPI_OK_AREA1_ERR ) || ( uc_ret == THRD_GET_SPI_OK_AREA2_ERR ) ) {
		for( uc_cnt = 0; uc_cnt < 5; uc_cnt++ ) {
			if(uc_kernel_panic_occer_Info[uc_cnt] != 0xFF){
				THRG_setBspThrRegion( 0x0040, &uc_kernel_panic_occer_default, sizeof(uc_kernel_panic_occer_default) );
				break;
			}
		}

	}	
#endif	// #if defined(TORNADO) && !defined(FUM_LINUX).	
	
	for( uc_cnt = 0, ul_bit = 1; uc_cnt < 16; uc_cnt++, ul_bit <<= 1 ) {
		if( ul_factor & ul_bit ) {
			switch( ul_bit ) {
				case THRD_PWROFF_FACTOR_TMOUT_LIMIT:		/* bit_0 最大監視時間 */
					sc_factor_str = "power-off timeout ( limit )";
					break;
				case THRD_PWROFF_FACTOR_TMOUT_WDG:			/* bit_1 WDG時間 */
					sc_factor_str = "power-off Timeout ( watchdog )";
					break;
				case THRD_PWROFF_FACTOR_HRESET_REQ:			/* bit_2 /HRESET_REQ=0 */
					sc_factor_str = "/HRESET_REQ = 0 ( normal power off )";
					break;
				case THRD_PWROFF_FACTOR_OFFSEQ_ON:			/* bit_3 OFFシーケンス中の主電源ON */
					sc_factor_str = "MSW ON during power-off Sequence";
					break;
				case THRD_PWROFF_FACTOR_BOOTING_OFF:		/* bit_4 ブートアップ中の主電源OFF */
					sc_factor_str = "MSW OFF during boot up";
					break;
				case THRD_PWROFF_FACTOR_BACKUP_ON:			/* bit_5 瞬断バックアップ中の主電源ON */
					sc_factor_str = "MSW ON during power-off backup";
					break;
				case THRD_PWROFF_FACTOR_FLICKER:			/* bit_6 瞬断リブート */
					sc_factor_str = "power-flicker detect";
					break;
				case THRD_PWROFF_FACTOR_FLICKER_FAILSAFE:	/* bit_7 瞬断リブート(フェールセーフ) */
					sc_factor_str = "power-flicker detect ( failsafe )";
					break;
				case THRD_PWROFF_FACTOR_BOOT_SEQ_ERROR:		/* bit_8 Bootシーケンスエラー */
					sc_factor_str = "boot sequence error";
#if defined(TORNADO) && !defined(FUM_LINUX) /*実機*/

					// 2020/06/05 ft_kondo OP_BTS-24920 22019新詳細コード細分化対応 S. 
					// reboot要因識別子をQSPIからReadする. 
					us_code = 0;
					// 2020/10/02 山口 OP_BTS-28170 Oops発生でLPPPがフリーズ.
					// 2020/10/17 山口 OP_BTS-28396 SError発生時に22019詳細コード細分化.
					// QSPIから読みだした値が0x1ならOops、0x2ならSEoor.
					if( 0x01 == uc_kernel_panic_occer_tmp){
						us_code = 3;
					} else if( 0x02 == uc_kernel_panic_occer_tmp ){
						us_code = 4;
					}
					// 2022/01/18 OP_BTS-37270 山口 Oops発生時の詳細コード細分化.
					// Oops発生で22019が発生した場合は別の新詳細コードを登録する.
					if(us_code != 3 ){
						// 2019/06/16 山口 AR1077356 DeepSleep復帰に3分程度かかる事がある問題.
						// 2020/5/28 伊藤史. OP_BTS-24010 ログ強化. ul_factor を追加.
						ull_NewDetailCode = ((((uint64_t)0x22			<< 56) & 0xff00000000000000)		// モジュール番号.
										   | (((uint64_t)0x019			<< 44) & 0x00fff00000000000)		// 通し番号.
											// 2020/06/05 ft_kondo OP_BTS-24920 22019新詳細コード細分化対応. 
										   | (((uint64_t)us_code		<< 40) & 0x00000f0000000000)		// reboot要因識別子. 
										   | (((uint64_t)ul_factor		<<  0) & 0x00000000ffffffff));		// 通電OFF要因.
						bl_ret = APIC_setNewDetailCode( TYPE_FatalDetailCode_undefined, ull_NewDetailCode );		// 詳細コード登録.
						if (False == bl_ret) {
							SYSF_logMsg( "(%010u)(BSP) ## %s %d : APIC_setNewDetailCode NG\n", SYSF_tickGet() , __FUNCTION__, __LINE__, 3,4,5,6 );
						}
					} else {
						ull_NewDetailCode = ((((uint64_t)0x22														<< 56) & 0xff00000000000000)		// モジュール番号.
										  | (((uint64_t)0x029                                            			<< 44) & 0x00fff00000000000)		// 通し番号.
										  | (((uint64_t)uc_kernel_panic_occer_Info[0]                               << 36) & 0x00000ff000000000)		// デバイス名 1byte目、サイズに応じた記録位置.
										  | (((uint64_t)uc_kernel_panic_occer_Info[1]                             	<< 28) & 0x0000000ff0000000)		// デバイス名 2byte目、サイズに応じた記録位置.
										  | (((uint64_t)uc_kernel_panic_occer_Info[2]                             	<< 20) & 0x000000000ff00000)		// デバイス名 3byte目、サイズに応じた記録位置.
										  | (((uint64_t)uc_kernel_panic_occer_Info[3]                             	<< 12) & 0x00000000000ff000)		// デバイス名 4byte目、サイズに応じた記録位置.
										  | (((uint64_t)uc_kernel_panic_occer_Info[4]                             	<<  4) & 0x0000000000000ff0));		// データサイズ.
						bl_ret = APIC_setNewDetailCode( TYPE_FatalDetailCode_undefined, ull_NewDetailCode );		// 詳細コード登録.
						if (False == bl_ret) {
							SYSF_logMsg( "(%010u)(BSP) ## %s %d : APIC_setNewDetailCode NG\n", SYSF_tickGet() , __FUNCTION__, __LINE__, 3,4,5,6 );
						}
						
					}
#endif	// #if defined(TORNADO) && !defined(FUM_LINUX).
					break;
				default:
					sc_factor_str = "unknown factor";
					break;
			}
			printf( "(%010u)(BSP)[%s] (bit%d) %s\n", SYSF_tickGet(), __FUNCTION__, uc_cnt, sc_factor_str );
		}
	}
}
// ================================================================================================.
//! <!--機能--> 電源OFFタイマー設定.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! uint16_t	タイマー値(0-65535) ミリ秒.
//! <!--戻値-->.
//! Bool		True: 成功,  False: 失敗.
//! <!--詳細-->.
//!  電源OFFタイマー設定.
//! <!--履歴-->.
//! 2017/10/17 ishiga [Eagle][起動時間短縮(再設計)] インターフェース見直し.
// ================================================================================================.
Bool THRF_setPowerOffTimer(uint16_t us_timer)
{
	char pc_setval[THRD_StateID_MaxLen] = {'\0'};
	snprintf( pc_setval, sizeof(pc_setval), "%d", us_timer );
	return setKernelDeviceStatusDirect( THRE_StoreDirectDeviceIX_PowerOffTimer, pc_setval );
}

// ================================================================================================.
//! <!--機能--> 電源OFF遅延指示.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! Bool		True:電源OFF遅延する.		False:電源OFF遅延しない.
//! <!--戻値-->.
//! Bool		True: 成功,  False: 失敗.
//! <!--詳細-->.
//!  電源OFF遅延指示.
//! <!--履歴-->.
//! 2017/10/17 ishiga [Eagle][起動時間短縮(再設計)] インターフェース見直し.
// ================================================================================================.
Bool THRF_setPowerOffDelay(Bool bl_delay)
{
	const char* pc_setval = (bl_delay)? "1": "0";
	return setKernelDeviceStatusDirect( THRE_StoreDirectDeviceIX_PowerOffDelay, pc_setval );
}

// ================================================================================================.
//! <!--機能--> 割り込み要因入力状態取得.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! なし.
//! <!--戻値-->.
//! uint32_t
//!  旧電源SubCPU L007レジスタ互換.
//!    bit0:	RTC割込み.
//!    bit1:	未使用.
//!    bit2:	[主]電源スイッチ割込み.
//!    bit3:	未使用.
//!    bit4:	[副]電源スイッチ割込み.
//!    bit5:	原稿セット割込み.
//!    bit6:	原稿台クローズ割込み.
//!    bit7:	カバーオープン割込み.
//!    bit8:	無線LAN 割込み.
//!    bit9:	FAX 割込み.
//!    bit10:	CSRC 割込み.
//!    bit11:	PHY 割込み.
//!    bit12:	FIERY 起動割込み.
//!    bit13:	ALERT(パネルセンサ)  割込み.
//!    bit14:	USB-B VBUS 割込み.
//!    bit15:	SNMP(Ⅱ) 割込み   (IT5まで).
//! <!--詳細-->.
//!  割り込み要因入力状態取得.
//! <!--履歴-->.
//! 2017/10/17 ishiga [Eagle][起動時間短縮(再設計)] インターフェース見直し.
// ================================================================================================.
uint32_t THRF_getIntPinStatus(void)
{
	char pc_state[THRD_StateID_MaxLen] = {'\0'};
	Bool bl_res = False;
	bl_res = getKernelDeviceStatusDirect( THRE_StoreDirectDeviceIX_INT_PIN_STATUS, pc_state );
	if( bl_res == False ) {
		return 0;
	}
	return (uint32_t)atoi( pc_state );
}

// ================================================================================================.
//! <!--機能--> 割り込み要因取得.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! なし.
//! <!--戻値-->.
//! uint32_t	割り込み復帰要因.
//!    bit0:	一次電源瞬停.
//!    bit1:	[副]電源SW押.
//!    bit2:	原稿 EDHセット.
//!    bit3:	原稿セット.
//!    bit4:	前カバー開閉.
//!    bit5:	無線LANからの起動.
//!    bit6:	FAXからの起動信号.
//!    bit7:	MODEMのRI信号(CSRC).
//!    bit8:	PHYからの起動要求(PME).
//!    bit9:	Fieryからの起動信号.
//!    bit10:	人体検出センサ(パネル).
//!    bit11:	USBホストからの給電検出.
//!    bit12:	SMNP(Ⅱ)エンジン起動.
//!    bit13:	RTCアラーム.
//!    bit14:	主電源SW(OFF).
//! <!--詳細-->.
//!  割り込み要因取得.
//! <!--履歴-->.
//! 2017/10/17 ishiga [Eagle][起動時間短縮(再設計)] インターフェース見直し.
// ================================================================================================.
uint32_t THRF_getIntSource(void)
{
	char pc_state[THRD_StateID_MaxLen] = {'\0'};
	Bool bl_res = False;
	bl_res = getKernelDeviceStatusDirect( THRE_StoreDirectDeviceIX_INT_SOURCE, pc_state );
	if( bl_res == False ) {
		return 0;
	}
	return (uint32_t)atoi( pc_state );
}

#define THRD_TERMOUT_DELAY_100MS	(SYSF_sysClkRateGet() * 100 / 1000)		// 100ms.
#define THRD_TERMOUT_DELAY_MAX		(2000/100)								// 最大待ち時間(2秒).
/*================================================================================================
［機能］	ターミナルログの出力完了をWaitする.
--------------------------------------------------------------------------------------------------
［引数］	なし.
［戻値］	なし.
［履歴］	2017/11/08 ishiga [Eagle][起動時間短縮(再設計)] ターミナルログが途切れる問題の対応.
================================================================================================*/
void THRF_waitForTermLogOutput( void )
{
	int fp = 0, i_delay_cnt = 0, i_outbuf_cnt = 0;

	// デバッグターミナル無効の場合は、即終了.
	if ( False == THRF_isEnable_DebugTerm() ) {
		return;
	}

	fp = SYSF_devopen( "/dev/ttyS5", O_RDWR, 0 );
	if ( -1 != fp ) {
		i_delay_cnt = 0;
		do {
			i_outbuf_cnt = 0;
			// 出力待ちバッファデータ数取得.
			if ( 0 > ioctl( fp, TIOCOUTQ, &i_outbuf_cnt ) ) {
				printf("[%s] Error: ioctl( fp, TIOCOUTQ )\n", __FUNCTION__);
				break;
			}
			// 終了条件.
			if ( ( i_outbuf_cnt <= 0 )							// 出力待ちデータなし.
			  || ( i_delay_cnt >= THRD_TERMOUT_DELAY_MAX ) ) {	// 最大待ち時間経過.
				break;	// 待ち終了.
			}
			// 一定時間待つ.
			SYSF_taskDelay( THRD_TERMOUT_DELAY_100MS );	// 100ms待ち.
			i_delay_cnt++;
		} while ( True );
		// クローズ.
		SYSF_close( fp );
	} else {
		printf("[%s] Error: SYSF_devopen( /dev/ttyS5, O_RDWR, 0 )\n", __FUNCTION__);
	}
}

// 電源サブCPUより指定レジスタの値を取得する.
uint32_t THRF_getPowerSubCPURegister( THRE_PowerSubCPU_REG reg ){
	return THRF_getIntPinStatus();
}

// ================================================================================================.
//! <!--機能--> 電源OFFシーケンス実行中時間を設定（電源OFF/ON時）.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! uint16_t	即リブート実行を許容する閾値(msec単位).
//! <!--戻値-->.
//! Bool		True: 成功,  False: 失敗.
//! <!--詳細-->.
//! 電源OFF/ON時、PS-CPUの電源OFFシーケンス実行中時間を設定する.
//! <!--履歴-->.
//! 2018/07/11 Sky酒井(純)[Eagle][AR.1058545] 電源OFFシーケンス実行中時間を設定するインターフェイスの作成.
// ================================================================================================.
Bool THRF_setPowerOffSeqTime(uint16_t us_Timer)
{
	char pc_setval[THRD_StateID_MaxLen] = {'\0'};
	snprintf( pc_setval, sizeof(pc_setval), "%d", us_Timer );
	return setKernelDeviceStatusDirect( THRE_StoreDirectDeviceIX_PowerOffSeqTime, pc_setval );
}
	
// ================================================================================================.
//! <!--機能--> USBデバイスの強制切断タイミング確認.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! なし.
//! <!--戻値-->.
//! Bool		True: USBデバイスが矯正切断される可能性のあるタイミング,  False: ないタイミング.
//! <!--詳細-->.
//! リブート時にシステムはUSBデバイスの切断を行う。その場合にFaxにて異常な切断と判断してしまうため
//! この関数で問題のない切断であるかの確認を行う(Faxから呼び出されるAPI).
//! <!--履歴-->.
//! 2018/10/18 河村 AR.1051005 新規.
// ================================================================================================.
Bool bl_isRebooting = False;	// リブートの際THRF_reboot()にて値を設定する(Trueを維持したままリブートする).
Bool THRF_isUsbDeviceForceDisconnectTiming(void)
{
	return bl_isRebooting;
}
//2019/10/08 Sky 【自動測定】起動イベントと時間の記録 start.
#if defined(TORNADO) && !defined(FUM_LINUX) /*実機*/
// イベント名一覧.
char cEventName[THRD_EventRecord_Max][THRD_EVENT_NAME_MAX_NUM] = {
	"Total",
	"SwitchOn",
	"LPPPStart",
	"ATFStart",
	"WarpStart",
	"WarpComplete",
	"BSPReturnStart",
	"BSPReturnComplete",
	"PowerReturnStart",
	"PowerReturnComplete",
	"KernelStart",
	"MemoryClearStart",
	"MemoryClearEnd",
	"MFPAppStart",
	"SnapShotStart",
	"SnapShotEnd",
	"SystemInit0Start",
	"PreferredPartitionMountStart",
	"PreferredPartitionMountEnd",
	"AnnShellStart",
	"TaskReleaseStart",
	"SystemInit0End",
	"LocalUIStart",
	"NewAPIConnection",
	"LocalUImainStart",
	"SystemInit1Start",
	"SystemInit1End",
	"SystemInit2Start",
	"SystemInit2End",
	"PertitionMountStart",
	"PertitionMountEnd",
	"SwitchingScreenStartup",
	"BrowserStart",
	"BrowserEnd",
	"NginxEnabled",
	"EventMEnabled",
	"PowerWaitTransition",
	"BrowserAvailable",
	"RebootRequestAccept",
	"DisplaySwitchingWait",
	"V3Request",
	"V3LoadRequestStart",
	"V3LoadStart",
	"V3LoadComplete",
	"V3LoadRequestComplete",
	"V2Request",
	"V2LoadRequestStart",
	"V2LoadStart",
	"V2LoadComplete",
	"V2LoadRequestComplete",
	"V3EnabledNotice",
	"V3DisplayRequest",
	"V2CopykeyEnabledNotice",
	"V2CopykeyEnabledUpdate",
	"PowerNotifyDefalutPanel",
	"PressCopyKey",
//2019/12/17 Sky 【自動測定】起動イベントの通知対応(Step3) start.
	"PressPanelPowerKeyDeepSleep",
	"PressPanelPowerKeySleep",
	"BackLightOn",
//2019/12/17 Sky 【自動測定】起動イベントの通知対応(Step3) end.
	"V3InitReadyComplete",				// 2020/03/13 Sky 自動測定ツール対応(Step3)により追加.
	"LocalUI_V3BaseScreenStart",		// 2020/03/13 Sky 自動測定ツール対応(Step3)により追加.
	"DefaultApplicationNotice",			// 2020/03/13 Sky 自動測定ツール対応(Step3)により追加.
	"V3ReadyDisplayNotice",				// 2020/03/13 Sky 自動測定ツール対応(Step3)により追加.
	"V3EnabledNotice-Run",				// 2020/03/13 Sky 自動測定ツール対応(Step3)により追加.
	"DefaultApplicationNotice-Run",		// 2020/03/13 Sky 自動測定ツール対応(Step3)により追加.
	"V3ReadyDisplayNotice-Run",			// 2020/03/13 Sky 自動測定ツール対応(Step3)により追加.
	"V3WebSocketOpenComplete",			// 2020/03/19 Sky 自動測定ツール対応(Step3)により追加.
	"V3MFPDataComplete",				// 2020/03/19 Sky 自動測定ツール対応(Step3)により追加.
//2020/08/21 Sky 【自動測定】追加イベントの通知対応 start.
	"MC_PWR_EN_ON",
	"MC_RESET_OFF",
	"MC_P_ON",
	"MOTION_ENABLE",
	"SB_PWR_ON",
	"CopyEnabledScreen",
	"McInitialCommunication",
	"PrintReadySend",		
	"ImagePrintSend",		
	"SystemEjectEndReceive",
	"V2CopyJobReady",		
	"V2CopyJobRun",			
//2020/08/21 Sky 【自動測定】追加イベントの通知対応 end.
//2022/06/17 Sky EagleⅢ 計測イベント対応 start.
//2023/01/30 Sky Takasaki EagleIII PS-CPU一本化対応 start.
	"AP_PWR_EN=L-Detect",
	"AP_PWR_EN=H-Detect",
	"AP_PWR_EN=L->REM=L",
	"AP_PWR_EN=H->REM=H",
	"SB_PWR_EN=L->REM=L",
	"SB_PWR_EN=H->REM=H",
	"LPPP->REM=L",
	"LPPP->REM=H",
	"LPPP-Resume-Request",
//2023/01/30 Sky Takasaki EagleIII PS-CPU一本化対応 end.
	"McPOnSkipLow",
//2022/06/17 Sky EagleⅢ 計測イベント対応 end.
};
#endif	// #if defined(TORNADO) && !defined(FUM_LINUX).

// ================================================================================================.
//! <!--機能--> 起動イベント番号と時間を取得するデバッグ用コマンド.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! void* pvEventBuff	: メモリバッファ.
//! <!--戻値-->.
//! なし.
//! <!--詳細-->.
//! 【自動測定】起動イベントと時間の取得対応.
//! <!--履歴-->.
//  2019.10.08	Sky		新規作成.
// ================================================================================================.
void DebugGetEventData()
{
#if defined(TORNADO) && !defined(FUM_LINUX) /*実機*/
	int32_t ret = 0;
	THRS_EventBuff *pBuff = NULL;
	int i = 0;
	
	pBuff = (THRS_EventBuff*)malloc( sizeof(THRS_EventBuff) * THRD_EVENT_BUFFER_MAX_NUM );
	if(pBuff == NULL){
		printf( "(%010u)(BSP)[%s] pBuff is NULL.\n", SYSF_tickGet(), __FUNCTION__ );
		return;
	}
	memset(pBuff, 0, sizeof(THRS_EventBuff) * THRD_EVENT_BUFFER_MAX_NUM);

	ret = THRF_getEventRecord((void*)pBuff);
	printf( "(%010u)(BSP)[%s] THRF_getEventRecord ret=[%d]\n", SYSF_tickGet(), __FUNCTION__, ret );
	
	for(i = 0;i < THRD_EVENT_BUFFER_MAX_NUM;i++){
		if(i > 0 && (pBuff+i)->ui_Num == 0){
			break;
		}
		printf( "(%010u)(BSP)[%s] Name=[%s] Num=[%lu] Time=[%lu]\n", SYSF_tickGet(), __FUNCTION__, (pBuff+i)->sc_Name, (pBuff+i)->ui_Num, (pBuff+i)->ui_Time );
	}
	
	free(pBuff);
	pBuff = NULL;
#endif	// #if defined(TORNADO) && !defined(FUM_LINUX).
}

#define THRD_setEventRecord_MaxLen (20)
// ================================================================================================.
//! <!--機能--> 起動イベント番号を記録する.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! uint32_t ui_Num	: イベント番号.
//! <!--戻値-->.
//! int32_t 0:イベント記録OK、-1：イベント記録NG.
//! <!--詳細-->.
//! 【自動測定】起動イベントの記録対応.
//! <!--履歴-->.
//  2019.10.08	Sky		新規作成.
// ================================================================================================.
int32_t THRF_setEventRecord( uint32_t ui_Num )
{
#if defined(TORNADO) && !defined(FUM_LINUX) /*実機*/
	Bool bl_res = False;

	// DipSWの有効判定.
	APIC_SoftDipSw* pc_SoftDipSw = APIC_SoftDipSw::newInstance();	// DipSw（API）.
	if( pc_SoftDipSw->getSoftDipSwBit( 144, 1 ) == False ){
		printf( "(%010u)(BSP)[%s] DipSW 199-0x08 OFF.\n", SYSF_tickGet(), __FUNCTION__ );
		return -1;
	}

	if(ui_Num == NULL){
		printf( "(%010u)(BSP)[%s] THRF_setEventRecord EventNumber NULL.\n", SYSF_tickGet(), __FUNCTION__ );
		return -1;
	}

	// イベントNoと時間を記録.
	char sc_SetVal[THRD_setEventRecord_MaxLen] = {'\0'};
	snprintf( sc_SetVal, sizeof(sc_SetVal), "%d", ui_Num );
	bl_res = setKernelDeviceStatusDirect( THRE_StoreDirectDeviceIX_EVENT_RECORD, sc_SetVal );
	if( bl_res == False ) {	// Falseがエラー.
		printf( "(%010u)(BSP)[%s] setKernelDeviceStatusDirect error(%d).\n", SYSF_tickGet(), __FUNCTION__, bl_res );
		return -1;
	}
#endif	// #if defined(TORNADO) && !defined(FUM_LINUX).

	return 0;
}

// ================================================================================================.
//! <!--機能--> 起動イベント番号と時間を取得する.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! void* pvEventBuff	: メモリバッファ.
//! <!--戻値-->.
//! int32_t 0：バッファ書込OK、-1：バッファ書込NG.
//! <!--詳細-->.
//! 【自動測定】起動イベントと時間の取得対応.
//! <!--履歴-->.
//  2019.10.08	Sky		新規作成.
// ================================================================================================.
int32_t THRF_getEventRecord( void* pvEventBuff )
{
#if defined(TORNADO) && !defined(FUM_LINUX) /*実機*/
//2019/11/27 Sky 【自動測定】起動時間取得修正 start.
	static THRS_EventBuff_Internal stEventBuffer[THRD_EVENT_BUFFER_MAX_NUM - 1] = {0};		// 取得するデータはTotalを含まないため、Totalを含むサイズのTHRD_EVENT_BUFFER_MAX_NUMは-1する.
	THRS_EventBuff *pstBuff = NULL;
	int i = 0;
	int32_t	si_Fd = 0;												// ファイルポインタ.
	ssize_t	si_Ret = 0;												// SYSF_readの戻り値.

	// DipSWの有効判定.
	APIC_SoftDipSw* pc_SoftDipSw = APIC_SoftDipSw::newInstance();	// DipSw（API）.
	if( pc_SoftDipSw->getSoftDipSwBit( 144, 1 ) == False ){
		printf( "(%010u)(BSP)[%s] DipSW 199-0x08 OFF.\n", SYSF_tickGet(), __FUNCTION__ );
		return -1;
	}
	
	if(pvEventBuff == NULL){
		printf( "(%010u)(BSP)[%s] THRF_getEventRecord Buffer NULL.\n", SYSF_tickGet(), __FUNCTION__ );
		return -1;
	}

	// バッファクリア.
	memset(pvEventBuff, 0, sizeof(THRS_EventBuff) * THRD_EVENT_BUFFER_MAX_NUM);
	memset(stEventBuffer, 0, sizeof(THRS_EventBuff_Internal) * (THRD_EVENT_BUFFER_MAX_NUM - 1));

	// 起動時間を取得するsysfsファイルをオープン.
	si_Fd = SYSF_devopen( "/sys/bus/platform/devices/PMU.0/devices/EVENT_RECORD" , O_RDONLY, 0644 );
	if ( si_Fd == -1 ){
		printf( "%s: SYSF_open EventRecord sysfs ERR errno:%d\n", __FUNCTION__, SYSF_errnoGet() );
		return -1;
	}

	// EventRecordをリードする.
	si_Ret = SYSF_read( si_Fd, (char*)&stEventBuffer, (sizeof(THRS_EventBuff_Internal) * (THRD_EVENT_BUFFER_MAX_NUM - 1)) );
	if( si_Ret == -1 ){
		SYSF_devclose(si_Fd);
		printf( "%s: SYSF_read EventRecord sysfs ERR errno:%d\n", __FUNCTION__, SYSF_errnoGet() );
		return -1;
	}

	// クローズ.
	SYSF_devclose(si_Fd);

	// PS-CPUから取得したデータを出力バッファへコピー(先頭のデータはTotal時間に使用するのでデータは2番目以降にコピーする).
	pstBuff = (THRS_EventBuff*)pvEventBuff;
	for(i = 0;i < THRD_EVENT_BUFFER_MAX_NUM - 1;i++){	// 取得したデータはTotalを含まないため、Totalを含むサイズのTHRD_EVENT_BUFFER_MAX_NUMは-1した値を使用する.
		if(stEventBuffer[i].ui_Num == 0){
			break;
		}
		(pstBuff+(i+1))->ui_Num = stEventBuffer[i].ui_Num;
		(pstBuff+(i+1))->ui_Time = stEventBuffer[i].ui_Time;
	}

	// Total起動時間を計算.
	strncpy(pstBuff->sc_Name, cEventName[THRD_EventRecord_Total], THRD_EVENT_NAME_MAX_NUM);
	pstBuff->ui_Num = THRD_EventRecord_Total;
	pstBuff->ui_Time = 0;
	for(i = 1;i < THRD_EVENT_BUFFER_MAX_NUM;i++){
		if((pstBuff+i)->ui_Num == THRD_EventRecord_PressCopyKey){
			pstBuff->ui_Time = (pstBuff+i)->ui_Time;
			break;
		}
	}

	// イベントNoをイベント名に変換.
	for(i = 1;i < THRD_EVENT_BUFFER_MAX_NUM;i++){
		// イベント番号が無ければ変換処理を終了する.
		if((pstBuff+i)->ui_Num == 0){
			break;
		}
		if((pstBuff+i)->ui_Num < THRD_EventRecord_Max){
			strncpy((pstBuff+i)->sc_Name, cEventName[(pstBuff+i)->ui_Num], THRD_EVENT_NAME_MAX_NUM);
		} else {
			memset((pstBuff+i)->sc_Name, 0, THRD_EVENT_NAME_MAX_NUM);
		}
	}
//2019/11/27 Sky 【自動測定】起動時間取得修正 end.
#endif	// #if defined(TORNADO) && !defined(FUM_LINUX).

	return 0;
}

// ================================================================================================.
//! <!--機能--> 起動イベント番号と時間を取得してUSBメモリに保存する.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! なし.
//! <!--戻値-->.
//! int32_t 0：USBメモリ出力OK、-1：USBメモリ出力NG.
//! <!--詳細-->.
//! 【自動測定】起動イベントと時間の取得対応.
//! <!--履歴-->.
//  2019.10.08	Sky		新規作成.
// ================================================================================================.
int32_t THRF_getEventRecordToUSB(void)
{
#if defined(TORNADO) && !defined(FUM_LINUX) /*実機*/
	int32_t ret = 0;
	THRS_EventBuff *pBuff = NULL;
	int32_t ul_Size = 0;
	char cWriteBuffer[256] = {0};
	int i = 0;
	char c_FileName[256] = {""};
	
	pBuff = (THRS_EventBuff*)malloc( sizeof(THRS_EventBuff) * THRD_EVENT_BUFFER_MAX_NUM );
	if(pBuff == NULL){
		printf( "(%010u)(BSP)[%s] pBuff is NULL.\n", SYSF_tickGet(), __FUNCTION__ );
		return -1;
	}
	memset(pBuff, 0, sizeof(THRS_EventBuff) * THRD_EVENT_BUFFER_MAX_NUM);

	ret = THRF_getEventRecord((void*)pBuff);
	printf( "(%010u)(BSP)[%s] THRF_getEventRecord ret=[%d]\n", SYSF_tickGet(), __FUNCTION__, ret );
	if(ret == -1){
		printf( "(%010u)(BSP)[%s] Failed Get Event Record.\n", SYSF_tickGet(), __FUNCTION__ );
		free(pBuff);
		pBuff = NULL;
		return -1;
	}
	
	// ROMバージョン(MFPコントローラ)を取得する.
	char sc_RomVersion[THRD_ROMVersionLength+1] = {0};                         // ROMバージョン.
	APIC_SystemInfo* pc_SystemInfo = APIC_SystemInfo::newInstance();
	pc_SystemInfo->getRomVersion(TYPE_RomMFPController, sc_RomVersion);

	// システム時刻を取得する.
	time_t sys_time = 0;
	struct tm sys_tm = {};
	sys_time = time( NULL );
	SYSF_localtime_r( &sys_time, &sys_tm );

	// ディレクトリ存在チェック.
	if ( access(THRD_EVENT_DATA_PATH, F_OK) != 0 ) {
		SYSF_Mkdir(THRD_EVENT_DATA_PATH);
	}

	// USBメモリに出力.
	sprintf(c_FileName, "%s/%04d%02d%02d_%02d%02d%02d_%s.log",
					THRD_EVENT_DATA_PATH,
					sys_tm.tm_year + 1900, sys_tm.tm_mon + 1, sys_tm.tm_mday,
					sys_tm.tm_hour, sys_tm.tm_min, sys_tm.tm_sec,
					sc_RomVersion);
	FILE* fp = SYSF_Fopen(c_FileName, "w+");
	if ( fp == NULL ) {
		printf( "(%010u)(BSP)[%s] EventRecordToUSB File Open error.\n", SYSF_tickGet(), __FUNCTION__ );
		free(pBuff);
		pBuff = NULL;
		return -1;
	}

	ret = (int32_t)SYSF_Fwrite("Name,Num,Time\n", 1, strlen("Name,Num,Time\n"), fp);
	for(i = 0;i < THRD_EVENT_BUFFER_MAX_NUM;i++){
		if(i > 0 && (pBuff+i)->ui_Num == 0){
			break;
		}
		sprintf(cWriteBuffer, "%s,%lu,%lu\n", (pBuff+i)->sc_Name, (pBuff+i)->ui_Num, (pBuff+i)->ui_Time);
		ul_Size = (int32_t)strlen(cWriteBuffer);
		ret = (int32_t)SYSF_Fwrite(cWriteBuffer, 1, ul_Size, fp);
		if (ret != ul_Size) {
			SYSF_Fclose(fp);
			free(pBuff);
			pBuff = NULL;
			printf( "(%010u)(BSP)[%s] EventRecordToUSB File Write error. size = %"PRIuPTR", write = %"PRIu32"\n", SYSF_tickGet(), __FUNCTION__, ul_Size, ret );
			return -1;
		}
	}

	SYSF_Fclose(fp);
	free(pBuff);
	pBuff = NULL;
	// Syncする.
	THRF_DoSync();
	printf( "(%010u)(BSP)[%s] Output event data to USB memory.\n", SYSF_tickGet(), __FUNCTION__ );
#endif	// #if defined(TORNADO) && !defined(FUM_LINUX).

	return 0;
}
//2019/10/08 Sky 【自動測定】起動イベントと時間の記録 end.


//FUNCTION-------------------------------------------------------------------------------------------------------------------.
// \brief	概要：CPU使用率 (CPU utilization) 取得.
// \par		備考：OP_BTS-19313/OP_BTS-18895の高難易度問題仕込み.
// \param	uc_GetCpuUtlzID ：CPU使用率取得方法.
// \param	sc_nice         ：CPU使用率取得優先度変更. 0:変更なし, -20:最高優先度.
// \result	なし.
// \par	変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//		2020/1/28 BSP伊藤史. 新規作成. OP_BTS-19313 ログ強化. 高難易度問題仕込み.
//---------------------------------------------------------------------------------------------------------------------------.
static const char THRD_GetCpuUtlzID_topCmd[]		= "top -i -H -n 2 -d 3";	///< topコマンド文字列.
static const char THRD_GetCpuUtlzID_pidstatCmd[]	= "pidstat -t -T ALL -u";	///< pidstatコマンド文字列.

void THRF_GetCpuUtilization( THRD_GetCpuUtlzID uc_GetCpuUtlzID, char sc_nice )
{
	char sc_Command[ 256 + 20 ];	// コマンド文字列格納領域.
	int  si_Ret = OK;	// コマンド実行戻り値.

#ifdef FUM_LINUX	// Sim対応.
	uc_GetCpuUtlzID = 1;	// Simはpidstat使用不可.
	sc_nice = 0;
#endif

	if( THRD_GetCpuUtlzID_top == uc_GetCpuUtlzID ){
		memset( sc_Command, 0x00, sizeof( sc_Command ) );  
		if( 0 == sc_nice ){
			strncpy( sc_Command, THRD_GetCpuUtlzID_topCmd, sizeof(sc_Command)-1 );
		} else {
			sprintf( sc_Command, "nice -n %d %s", sc_nice, THRD_GetCpuUtlzID_topCmd );
		}
	}
	if( THRD_GetCpuUtlzID_pidstat == uc_GetCpuUtlzID ){
		memset( sc_Command, 0x00, sizeof( sc_Command ) );  
		if( 0 == sc_nice ){
			strncpy( sc_Command, THRD_GetCpuUtlzID_pidstatCmd, sizeof(sc_Command)-1 );
		} else {
			sprintf( sc_Command, "nice -n %d %s", sc_nice, THRD_GetCpuUtlzID_pidstatCmd );
		}
	}
	si_Ret = system( sc_Command );
}


// ターミナルコマンド.
// CPU使用率 (CPU utilization) 取得.
void dbg_GetCpuUtilization( int si_GetCpuUtlzID, int si_nice )
{
	if( 0 == si_GetCpuUtlzID ){
		// HELP.
		printf("dbg_GetCpuUtilization : Disp CPU utilization\n");
		printf("dbg_GetCpuUtilization <get method>, <nice>n");
		printf(" [ method ] 1:top cmd.\n");
		printf("            2:pidstat cmd.\n");
		printf(" [ nice ] 0 : not change.\n");
		printf("          1 to 20 : priority down. 20 is down max.\n");
		printf("         -1 to -20 : priority up. -20 is up max.\n");
		return;
	}
	THRF_GetCpuUtilization( static_cast<THRD_GetCpuUtlzID>(si_GetCpuUtlzID), static_cast<char>(si_nice) );
}

static ulong ul_dbgLoopMax = 0;
static ushort us_priority = 0;
static uchar uc_silent = 0;

#include <math.h>

// ダミータスク.
void dbgTaskLoop()
{
	ulong	ul_p1 = 0;
	ulong	ul_p2 = 0;
	ulong	ul_p3 = 0;
	ulong	ul_p4 = 0;
	double	sd_dt1 = 0.0;
	double	sd_dt2 = 0.0;

	printf("TASK_dbgTaskLoop <loop rate=%ld>,<priority=%d>,<silent=%d>\n",
		ul_dbgLoopMax, us_priority, uc_silent );

	sd_dt1 = (double)ul_dbgLoopMax;
	while( 0 != ul_dbgLoopMax ){
		if( 0 == uc_silent ) printf("[%d] dbgTaskLoop!!\n", SYSF_tickGet() );

		for( ul_p1=0; ul_p1<ul_dbgLoopMax; ul_p1++ ){
			for( ul_p2=0; ul_p2<ul_dbgLoopMax; ul_p2++ ){
				sd_dt1 = (double)ul_dbgLoopMax;
				for( ul_p3=0; ul_p3<ul_dbgLoopMax; ul_p3++ ){
					for( ul_p4=0; ul_p4<ul_dbgLoopMax; ul_p4++ ){
						sd_dt2 = log( sd_dt1 );
						sd_dt1 = exp( sd_dt2 );
						if( 0 == ul_dbgLoopMax ) break;
					}
					if( 0 == ul_dbgLoopMax ) break;
				}
				if( 0 == ul_dbgLoopMax ) break;
			}
			if( 0 == ul_dbgLoopMax ) break;
		}
	}
	printf("[%d] dbgTaskLoop END.\n", SYSF_tickGet() );
}

// ターミナルコマンド.
// ダミータスク生成.
void dbgTaskLoopRun( int dt1, int dt2 , int dt3)
{
	if( (0 == dt1) && (0 == dt2) ){
		printf("Usage : dbgTaskLoopRun <loop rate>,<priority>,<silent>\n");
		return;
	}
	if( 0 == dt2 ){
		printf("TASK_dbgTaskLoop priority = 45\n");
		us_priority = 45;
	} else {
		us_priority = static_cast<ushort>(dt2);
	}
	uc_silent = ( 0 == dt3 )? 0:1;

	if( 0 == ul_dbgLoopMax ){
		ul_dbgLoopMax = dt1;
		 SYSF_taskSpawn( "TASK_dbgTaskLoop", dt2, VX_FP_TASK, 2048, (FUNCPTR)dbgTaskLoop, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	} else {
		ul_dbgLoopMax = 0;
	}
}

/* 2020/02/26 Harada OP_BTS-20942 LPDDR4パラメータ化け LPDDR4パラメータ領域の多重化対応 >> */
/*
 * Name:     calculateChecksumQspi
 * Desc:     QSPI内データ 指定Readサイズ分のチェックサムを取得する
 * Args:     readAddr  - 読み込む先頭アドレス
 *           readSize  - 読み込みデータサイズ
 * Notes:
 * Returns:  指定リードサイズのチェックサム.
 */
uint32_t THRL_calculateChecksumQspi(uint64_t ull_readAddr, uint64_t ull_readSize)
{
	uint32_t ul_checksum = 0;
	uint32_t ul_retVal = 0;

	ul_checksum = 0;

	for(uint64_t ull_count = 0; ull_count < ull_readSize; ull_count+=4)
	{
		ul_retVal = *(uint32_t*)((uintptr_t)(ull_readAddr + ull_count));
		ul_checksum += ul_retVal;
/*		SYSF_logMsg( "(%010u)(BSP) ## %s %d : addr=0x%08lx, ul_retVal=0x%08x, ul_checksum = 0x%08x\n", SYSF_tickGet() , __FUNCTION__, __LINE__, (uintptr_t)(ull_readAddr + ull_count), ul_retVal, ul_checksum, 6 );*/
	}
	
	if (ull_count != 0) {
/*		SYSF_logMsg( "(%010u)(BSP) ## %s %d : ul_checksum = 0x%08x\n", SYSF_tickGet() , __FUNCTION__, __LINE__, ul_checksum, 4,5,6 );*/
	}
	
	return ul_checksum;
}

/*
 * Name:     judgeQspiParam
 * Desc:     引数のチェックサムとチェックサム格納先から取得したデータの一致判定を行う
 * Args:     checksum1          - 比較するチェックサム1
 *           readAddrChecksum2  - 比較するチェックサム2が格納されているアドレス
 * Notes:
 * Returns:  checksum1とchecksum2の一致判定結果
 *             0 : 不一致
 *             1 : 一致
 */
Bool THRL_judgeQspiParam(uint32_t ul_checksum1, uint64_t ull_readAddrChecksum2)
{
	uint32_t ul_retVal = 0;
	Bool bl_ret = False;
	
	ul_retVal = *(uint32_t*)((uintptr_t)ull_readAddrChecksum2);
	
/*	SYSF_logMsg( "(%010u)(BSP) ## %s %d : Calculated checksum = 0x%08x, Expected checksum = 0x%08x\n", SYSF_tickGet() , __FUNCTION__, __LINE__, ul_checksum1, ul_retVal, 5,6 );*/
	if(ul_checksum1 == ul_retVal)
	{
		bl_ret = True;
	}
	else
	{
		bl_ret = False;
	}
	
	return bl_ret;
}

/*** Add for LPDDDR4 chip detect ***/
#define CHIP_DETECT_BASE_ADDR			0x00081800	/* LPDDR4パラメータ領域のアドレス              ※/dev/mtd2のアドレス0xF4300000からのオフセット */
#define CHIP_DETECT_BACKUP1_BASE_ADDR	0x000FE800	/* LPDDR4パラメータ領域バックアップ1のアドレス ※/dev/mtd2のアドレス0xF4300000からのオフセット */
#define QSPI_CHECK_SUM_READ_SIZE 2044

typedef struct _Lpddr4Param_t {
	uintptr_t ul_chipInfoReg;		/* LPDDR4パラメータ領域のアドレス */
	uint32_t ul_checksumQspi;		/* チェックサム */
	Bool bl_paramQspiFlag;			/* チェックサム判定結果 */
}Lpddr4Param_t;

Lpddr4Param_t Lpddr4Param[] = {
	{ (uintptr_t)CHIP_DETECT_BASE_ADDR,			0x00000000,	False },	/* LPDDR4パラメータ領域のアドレス */
	{ (uintptr_t)CHIP_DETECT_BACKUP1_BASE_ADDR,	0x00000000,	False },	/* LPDDR4パラメータ領域バックアップ1のアドレス */
};

#define THRD_SECTOR_SIZE	4096		/* セクタサイズ(4 * 1024) */
static unsigned char uc_data[THRD_SECTOR_SIZE];		/* 取得領域 */

/* 2020/05/01 Harada OP_BTS-22200 LPDDR4パラメータ化け LPDDR4パラメータ領域の多重化対応 B対応 ライトプロテクト */
#define CHIP_DETECT_WRITE_PROTECT_START_ADDR	0x00081000	/* LPDDR4パラメータ領域のライトプロテクト開始アドレス ※/dev/mtd2のアドレス0xF4300000からのオフセット */
#define CHIP_DETECT_WRITE_PROTECT_END_ADDR		0x00081fff	/* LPDDR4パラメータ領域のライトプロテクト終了アドレス ※/dev/mtd2のアドレス0xF4300000からのオフセット */

// ================================================================================================.
//! <!--機能--> LPDDR4パラメータ領域 ライトプロテクト設定.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! uintptr_t ul_addr			: アドレス.
//! uint32_t ul_writeProtect	: '0':読み書き可能/'1':読み込みのみ可能.
//! <!--戻値-->.
//! なし.
//! <!--詳細-->.
//! LPDDR4パラメータ領域の書き込みに手順が必要(ライトプロテクトを一時的に外す)にする対応
//! 関連としてC対応がLPDDR4パラメータ領域を書き換えるので、同制御にライトプロテクトを一時的に外す対応も追加の必要がある。
//! ※ライトプロテクトの範囲はmtd2の0x81000～0x81fff
//! <!--履歴-->.
//! 2020/05/01 Harada OP_BTS-22200 LPDDR4パラメータ化け LPDDR4パラメータ領域の多重化対応 B対応 ライトプロテクト.
// ================================================================================================.
void THRL_setWriteProtectLpddr4Param(uintptr_t ul_addr, uint32_t ul_writeProtect)
{
	if ( (CHIP_DETECT_WRITE_PROTECT_START_ADDR <= ul_addr) && (ul_addr <= CHIP_DETECT_WRITE_PROTECT_END_ADDR)
	  && (CHIP_DETECT_WRITE_PROTECT_START_ADDR <= (ul_addr + THRD_SECTOR_SIZE -1)) && ((ul_addr + THRD_SECTOR_SIZE -1) <= CHIP_DETECT_WRITE_PROTECT_END_ADDR) ) {	/* LPDDR4パラメータ領域のライトプロテクト対象範囲の場合 */
		if (0 == ul_writeProtect) {
		  	system("echo 0 > /sys/devices/platform/quartz-sb/8e8273000.spi0/write_protect_lpddr4param");	/* ライトプロテクト '0':読み書き可能 */
		} else if (1 == ul_writeProtect) {
			system("echo 1 > /sys/devices/platform/quartz-sb/8e8273000.spi0/write_protect_lpddr4param");	/* ライトプロテクト '1':読み込みのみ可能 */
		}
	}
}

Bool THRF_writeFlashExt( uint32_t offset, void *buf, uint32_t size );	/* Flashにライトする. 拡張版. */
Bool THRF_readFlashExt( uint32_t offset, void *buf, uint32_t size );	/* Flashからリードする. 拡張版. */
Bool THRF_eraseFlashExt( uint32_t start, uint32_t size );				/* Flashをイレースする. 拡張版. */

// ================================================================================================.
//! <!--機能--> LPDDR4パラメータ領域復旧.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! なし.
//! <!--戻値-->.
//! なし.
//! <!--詳細-->.
//! 概要.
//! LPDDR4パラメータ領域に加えてバックアップ領域を用意する。.
//! バックアップはそれぞれSPI-Flash上の離れた場所に配置する。.
//! 領域のうち、問題のないものを参照して起動する。.
//! 制御.
//! <本体FW>.
//! 1. 起動後各領域のチェックサムを確認してOK領域とNG領域を判断。.
//! 2. NG領域はOK領域の内容に置き換える。.
//! <!--履歴-->.
//! 2020/02/26 Harada OP_BTS-20942 LPDDR4パラメータ化け LPDDR4パラメータ領域の多重化対応.
// ================================================================================================.
void THRL_execRecoverLpddr4Param(void);

void THRG_recoverLpddr4Param(void)
{
#if defined(TORNADO) && !defined(FUM_LINUX) /*実機*/
	THR_ID taskID;
	taskID = SYSF_taskSpawn( "TASK_recoverLpddr4Param", 253, VX_FP_TASK, 0x4000,
							(FUNCPTR)THRL_execRecoverLpddr4Param,
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0 );
	if ( taskID == ERROR ) {
		printf("(%010u)(BSP) %s SYSF_taskSpawn() Error\n", SYSF_tickGet(), __FUNCTION__);
	}
#endif	// #if defined(TORNADO) && !defined(FUM_LINUX).
}

void THRL_execRecoverLpddr4Param(void)
{
	uint32_t ul_checksumQspi = 0;
	Bool bl_paramQspiFlag = 0;
	uintptr_t ul_addr = 0;

	/* wait */
	SYSF_taskDelay( (int)(SYSF_sysClkRateGet() * 60 * 1000 / 1000 ) );	// 60 * 1000ms wait.

	/* LPDDR4パラメータ領域のチェックサムチェック */
	for(uint32_t ul_count = 0; ul_count < sizeof(Lpddr4Param)/sizeof(Lpddr4Param_t); ul_count++) {
		memset(uc_data, 0, sizeof(uc_data));
		/* 領域のデータ取得 */
		ul_addr = Lpddr4Param[ul_count].ul_chipInfoReg - 0x0800;		/* 4KB単位でアクセスするので、開始位置はLPDDR4パラメータ領域のアドレスから2KB引いたアドレス */
		if ( False == THRF_readFlashExt( (uint32_t)ul_addr, (void*)uc_data, sizeof(uc_data) ) ) {
			/* エラー */
			SYSF_logMsg( "(%010u)(BSP) ## %s %d : THRF_readFlashExt ERR\n", SYSF_tickGet() , __FUNCTION__, __LINE__, 3,4,5,6 );
			return;
		}
		ul_addr = (uintptr_t)uc_data + 0x0800;		/* 4KB単位でアクセスしたので、LPDDR4パラメータ領域のアドレスは2KB足したアドレス */
		ul_checksumQspi = THRL_calculateChecksumQspi((uint64_t)ul_addr, QSPI_CHECK_SUM_READ_SIZE);
		bl_paramQspiFlag = THRL_judgeQspiParam(ul_checksumQspi, (uint64_t)ul_addr+QSPI_CHECK_SUM_READ_SIZE);
		Lpddr4Param[ul_count].ul_checksumQspi = ul_checksumQspi;		/* チェックサム */
		Lpddr4Param[ul_count].bl_paramQspiFlag = bl_paramQspiFlag;		/* チェックサム判定結果 */
	}

	/* チェックサムが異なる領域は、チェックサム判定結果が一致の領域の内容に置き換える */
	for(ul_count = 0; ul_count < sizeof(Lpddr4Param)/sizeof(Lpddr4Param_t); ul_count++) {
		if (True == Lpddr4Param[ul_count].bl_paramQspiFlag) {		/* チェックサム判定結果が一致の場合 */
			memset(uc_data, 0, sizeof(uc_data));
			/* 領域のデータ取得 */
			ul_addr = Lpddr4Param[ul_count].ul_chipInfoReg - 0x0800;		/* 4KB単位でアクセスするので、開始位置はLPDDR4パラメータ領域のアドレスから2KB引いたアドレス */
			if ( False == THRF_readFlashExt( (uint32_t)ul_addr, (void*)uc_data, sizeof(uc_data) ) ) {
				/* エラー */
				SYSF_logMsg( "(%010u)(BSP) ## %s %d : THRF_readFlashExt ERR\n", SYSF_tickGet() , __FUNCTION__, __LINE__, 3,4,5,6 );
				return;
			}
			
			for(uint32_t ul_count2 = 0; ul_count2 < sizeof(Lpddr4Param)/sizeof(Lpddr4Param_t); ul_count2++) {
				if (Lpddr4Param[ul_count].ul_checksumQspi != Lpddr4Param[ul_count2].ul_checksumQspi) {	/* チェックサム判定結果が一致の領域のチェックサムと、対象の領域のチェックサムが異なる場合 */
					SYSF_logMsg( "(%010u)(BSP) ## %s %d : ul_count=%d, ul_checksumQspi=0x%08x, ul_count2=%d, ul_checksumQspi=0x%08x\n", SYSF_tickGet() , __FUNCTION__, __LINE__, ul_count, Lpddr4Param[ul_count].ul_checksumQspi, ul_count2, Lpddr4Param[ul_count2].ul_checksumQspi );
					ul_addr = Lpddr4Param[ul_count2].ul_chipInfoReg - 0x0800;		/* 4KB単位でアクセスするので、開始位置はLPDDR4パラメータ領域のアドレスから2KB引いたアドレス */
					THRL_setWriteProtectLpddr4Param(ul_addr, 0);	/* ライトプロテクト '0':読み書き可能 *//* 2020/05/01 Harada OP_BTS-22200 LPDDR4パラメータ化け LPDDR4パラメータ領域の多重化対応 B対応 ライトプロテクト */
					/* erase */
					if ( False == THRF_eraseFlashExt( (uint32_t)ul_addr, sizeof(uc_data) ) ) {
						/* エラー */
						SYSF_logMsg( "(%010u)(BSP) ## %s %d : THRF_eraseFlashExt ERR\n", SYSF_tickGet() , __FUNCTION__, __LINE__, 3,4,5,6 );
						THRL_setWriteProtectLpddr4Param(ul_addr, 1);	/* ライトプロテクト '1':読み込みのみ可能 *//* 2020/05/01 Harada OP_BTS-22200 LPDDR4パラメータ化け LPDDR4パラメータ領域の多重化対応 B対応 ライトプロテクト */
						return;
					}
					/* write */
					if ( False == THRF_writeFlashExt( (uint32_t)ul_addr, (void*)uc_data, sizeof(uc_data) ) ) {
						/* エラー */
						SYSF_logMsg( "(%010u)(BSP) ## %s %d : THRF_writeFlashExt ERR\n", SYSF_tickGet() , __FUNCTION__, __LINE__, 3,4,5,6 );
						THRL_setWriteProtectLpddr4Param(ul_addr, 1);	/* ライトプロテクト '1':読み込みのみ可能 *//* 2020/05/01 Harada OP_BTS-22200 LPDDR4パラメータ化け LPDDR4パラメータ領域の多重化対応 B対応 ライトプロテクト */
						return;
					}
					THRL_setWriteProtectLpddr4Param(ul_addr, 1);	/* ライトプロテクト '1':読み込みのみ可能 *//* 2020/05/01 Harada OP_BTS-22200 LPDDR4パラメータ化け LPDDR4パラメータ領域の多重化対応 B対応 ライトプロテクト */
					Lpddr4Param[ul_count2].ul_checksumQspi = Lpddr4Param[ul_count].ul_checksumQspi;			/* チェックサム */
					Lpddr4Param[ul_count2].bl_paramQspiFlag = Lpddr4Param[ul_count].bl_paramQspiFlag;		/* チェックサム判定結果 */
				}
			}
			break;
		}
	}
}

#define FLASH_SIZE_EXT	( 1024 * 1024 )		// SPI-Flashのサイズ 拡張版.

/*================================================================================================
［機能］	Flashにライトする. 拡張版.
--------------------------------------------------------------------------------------------------
［引数］	uint32_t offset	:書き込み位置のオフセット
			void *buf		:書き込みデータのポインタ
			uint32_t size		:書き込みデータのサイズ
［戻値］	True:正常終了 False:エラー
［履歴］	2020/02/26 Harada OP_BTS-20942 LPDDR4パラメータ化け LPDDR4パラメータ領域の多重化対応
			THRF_writeFlashをベースにコピーして作成。アクセス範囲をUser Data以降の範囲へ拡張.
			処理はそのまま採用する。新詳細コード登録もそのまま採用し、通し番号は変更しない。
================================================================================================*/
Bool THRF_writeFlashExt( uint32_t offset, void *buf, uint32_t size )
{
	int32_t fd = -1;
	off_t   lseek_ret = 0;
	ssize_t write_ret = 0;
#if defined(TORNADO) && !defined(FUM_LINUX) /*実機*/
	uint64_t ull_NewDetailCode = 0x0;
	Bool bl_ret = True;
#endif
	struct sched_param save_sched;
	int save_policy;
	
	// エラーチェック.
	if( ( offset >= FLASH_SIZE_EXT ) || ( buf == NULL ) || (size <= 0) || ( offset + size > FLASH_SIZE_EXT ) ){	/* 2020/02/26 Harada OP_BTS-20942 LPDDR4パラメータ化け LPDDR4パラメータ領域の多重化対応 */
		printf("%s: ERR offset %"PRId32" buf %p size %"PRId32"\n", __FUNCTION__, offset, buf, size);
		return False;
	}

#if 1 /* 暫定対応:Sleep2復帰中にQSPIのアクセスでR4が固まる */
	if((b_writeFlash_enable == false) && (b_in_exec_str == true)){
		return True;
	}
#endif
	
	// セマフォ取得.
	if( semID_Flash == NULL ){
		semID_Flash = SYSF_semMCreate(SEM_Q_PRIORITY | SEM_DELETE_SAFE | SEM_INVERSION_SAFE);
	}
	SYSF_semTake( semID_Flash, WAIT_FOREVER );
	// 2019/12/19 河村 -  OP_BTS-18210 一時的にタスクプライオリティを上げる.
	if (0 != pthread_getschedparam( pthread_self(), &save_policy, &save_sched ) ) {
		SYSF_semGive( semID_Flash );
		return False;
	}
	struct sched_param sched = {};
	sched.sched_priority = 207;		/* 通信系タスクと同等 */
	if ( save_sched.sched_priority < sched.sched_priority ) {
		printf( "(%010u)(BSP)[%s] %s : prio %d -> %d\n", SYSF_tickGet(), __FUNCTION__, SYSF_taskName(SYSF_taskIdSelf()), save_sched.sched_priority, sched.sched_priority );
		if( 0 != pthread_setschedparam(pthread_self(), SCHED_RR, &sched) ) {
			SYSF_semGive( semID_Flash );
			return False;
		}
	}
	
	fd = getFlashFd();
	if( fd < 0 ){
		// 2019/12/19 河村 -  OP_BTS-18210 タスクプライオリティを戻す.
		if ( save_sched.sched_priority < sched.sched_priority ) {
			printf( "(%010u)(BSP)[%s] %s : prio %d -> %d\n", SYSF_tickGet(), __FUNCTION__, SYSF_taskName(SYSF_taskIdSelf()), sched.sched_priority, save_sched.sched_priority );
			pthread_setschedparam( pthread_self(), save_policy, &save_sched );
		}
		SYSF_semGive( semID_Flash );
		return False;
	}
	
	// 指定の場所までシークする.
	lseek_ret = SYSF_lseek( fd, (off_t)offset, SEEK_SET);
	if( lseek_ret < 0 ){
		printf("%s: SYSF_lseek ERR errno:%d\n", __FUNCTION__, SYSF_errnoGet());
	// 2016/03/23 山口 CQ.525702 SPI-Flashへの書き込みに失敗した場合、新詳細コードを残す.
#if defined(TORNADO) && !defined(FUM_LINUX) /*実機*/
		ull_NewDetailCode = (((uint64_t)0x22                                  << 56) & 0xff00000000000000)	// モジュール番号.
						  | (((uint64_t)0x001                                 << 44) & 0x00fff00000000000)	// 通し番号.
						  | (((uint64_t)((SYSF_errnoGet() < 0) ?  -SYSF_errnoGet() :  SYSF_errnoGet())  << 36) & 0x00000ff000000000)	// errno.
						  | (((uint64_t)offset                                << 16) & 0x0000000fffff0000)	// 書き込み位置のオフセット.
						  | (((uint64_t)((size < 0xffff) ?  size :  0xffff)   <<  0) & 0x000000000000ffff);	// サイズ.

		bl_ret = APIC_setNewDetailCode( TYPE_FatalDetailCode_undefined, ull_NewDetailCode );		// 詳細コード登録.
		if (False == bl_ret) {
			SYSF_logMsg( "## %s %d : APIC_setNewDetailCode NG\n", __FUNCTION__, __LINE__, 3,4,5,6 );
		}
#endif	// #if defined(TORNADO) && !defined(FUM_LINUX).

		// 2019/12/19 河村 -  OP_BTS-18210 タスクプライオリティを戻す.
		if ( save_sched.sched_priority < sched.sched_priority ) {
			printf( "(%010u)(BSP)[%s] %s : prio %d -> %d\n", SYSF_tickGet(), __FUNCTION__, SYSF_taskName(SYSF_taskIdSelf()), sched.sched_priority, save_sched.sched_priority );
			pthread_setschedparam( pthread_self(), save_policy, &save_sched );
		}
		SYSF_semGive( semID_Flash );
		return False;
	}
	
	// Flashにライトする.
	// 2015/08/26 ft_kobayashi MinervaSBK Warning除去対応. 
	write_ret = SYSF_write( fd, (char*)buf, size);
	if( write_ret < 0 ){
		printf("%s: SYSF_write ERR errno:%d\n", __FUNCTION__, SYSF_errnoGet());
	// 2016/03/23 山口 CQ.525702 SPI-Flashへの書き込みに失敗した場合、新詳細コードを残す.
#if defined(TORNADO) && !defined(FUM_LINUX) /*実機*/
		ull_NewDetailCode = (((uint64_t)0x22                                  << 56) & 0xff00000000000000)	// モジュール番号.
						  | (((uint64_t)0x002                                 << 44) & 0x00fff00000000000)	// 通し番号.
						  | (((uint64_t)((SYSF_errnoGet() < 0) ?  -SYSF_errnoGet() :  SYSF_errnoGet())  << 36) & 0x00000ff000000000)	// errno.
						  | (((uint64_t)offset                                << 16) & 0x0000000fffff0000)	// 書き込み位置のオフセット.
						  | (((uint64_t)((size < 0xffff) ?  size :  0xffff)   <<  0) & 0x000000000000ffff);	// サイズ.

		bl_ret = APIC_setNewDetailCode( TYPE_FatalDetailCode_undefined, ull_NewDetailCode );		// 詳細コード登録.
		if (False == bl_ret) {
			SYSF_logMsg( "## %s %d : APIC_setNewDetailCode NG\n", __FUNCTION__, __LINE__, 3,4,5,6 );
		}
#endif	// #if defined(TORNADO) && !defined(FUM_LINUX).
		// 2019/12/19 河村 -  OP_BTS-18210 タスクプライオリティを戻す.
		if ( save_sched.sched_priority < sched.sched_priority ) {
			printf( "(%010u)(BSP)[%s] %s : prio %d -> %d\n", SYSF_tickGet(), __FUNCTION__, SYSF_taskName(SYSF_taskIdSelf()), sched.sched_priority, save_sched.sched_priority );
			pthread_setschedparam( pthread_self(), save_policy, &save_sched );
		}
		SYSF_semGive( semID_Flash );
		return False;
	}
	
	// 2019/12/19 河村 -  OP_BTS-18210 タスクプライオリティを戻す.
	if ( save_sched.sched_priority < sched.sched_priority ) {
		printf( "(%010u)(BSP)[%s] %s : prio %d -> %d\n", SYSF_tickGet(), __FUNCTION__, SYSF_taskName(SYSF_taskIdSelf()), sched.sched_priority, save_sched.sched_priority );
		pthread_setschedparam( pthread_self(), save_policy, &save_sched );
	}
	// セマフォ開放.
	SYSF_semGive( semID_Flash );
	return True;
}

/*================================================================================================
［機能］	Flashからリードする. 拡張版.
--------------------------------------------------------------------------------------------------
［引数］	uint32_t offset	:読み込み位置のオフセット
			void *buf		:読み込みデータのポインタ
			uint32_t size		:読み込みデータのサイズ
［戻値］	True:正常終了 False:エラー
［履歴］	2020/02/26 Harada OP_BTS-20942 LPDDR4パラメータ化け LPDDR4パラメータ領域の多重化対応
			THRF_readFlashをベースにコピーして作成。アクセス範囲をUser Data以降の範囲へ拡張.
			処理はそのまま採用する。新詳細コード登録もそのまま採用し、通し番号は変更しない。
================================================================================================*/
Bool THRF_readFlashExt( uint32_t offset, void *buf, uint32_t size )
{
	int32_t fd = 0;
	off_t   lseek_ret = 0;
	ssize_t read_ret  = 0;
	
	// エラーチェック.
	if( ( offset >= FLASH_SIZE_EXT ) || ( buf == NULL ) || (size <= 0) || ( offset + size > FLASH_SIZE_EXT ) ){	/* 2020/02/26 Harada OP_BTS-20942 LPDDR4パラメータ化け LPDDR4パラメータ領域の多重化対応 */
		printf("%s: ERR offset %"PRId32" buf %p size %"PRId32"\n", __FUNCTION__, offset, buf, size);
		return False;
	}
	
	// セマフォ取得.
	if( semID_Flash == NULL ){
		semID_Flash = SYSF_semMCreate(SEM_Q_PRIORITY | SEM_DELETE_SAFE | SEM_INVERSION_SAFE);
	}
	SYSF_semTake( semID_Flash, WAIT_FOREVER );
	
	fd = getFlashFd();
	if( fd < 0 ){
		SYSF_semGive( semID_Flash );
		return False;
	}
	
	// 指定の場所までシークする.
	lseek_ret = SYSF_lseek( fd, (off_t)offset, SEEK_SET);
	if( lseek_ret < 0 ){
		printf("%s: SYSF_lseek ERR errno:%d\n", __FUNCTION__, SYSF_errnoGet());
		SYSF_semGive( semID_Flash );
		return False;
	}
	
	// Flashからリードする.
	// 2015/08/26 ft_kobayashi MinervaSBK Warning除去対応. 
	read_ret = SYSF_read( fd, (char*)buf, size);
	if( read_ret < 0 ){
		printf("%s: SYSF_read ERR errno:%d\n", __FUNCTION__, SYSF_errnoGet());
		SYSF_semGive( semID_Flash );
		return False;
	}
	
	// セマフォ開放.
	SYSF_semGive( semID_Flash );
	return True;
}

/*================================================================================================
［機能］	Flashをイレースする. 拡張版.
--------------------------------------------------------------------------------------------------
［引数］	uint32_t start		:消去を行うアドレス(4KB単位)
			uint32_t size		:消去するサイズ(4KB単位)
［戻値］	True:正常終了 False:エラー
［履歴］	2020/02/26 Harada OP_BTS-20942 LPDDR4パラメータ化け LPDDR4パラメータ領域の多重化対応
			THRF_eraseFlashをベースにコピーして作成。アクセス範囲をUser Data以降の範囲へ拡張.
			処理はそのまま採用する。新詳細コード登録もそのまま採用し、通し番号は変更しない。
================================================================================================*/
Bool THRF_eraseFlashExt( uint32_t start, uint32_t size )
{
	int fd = -1;
	uchar uc_RetryCnt = 0;
	struct sched_param save_sched;
	int save_policy;
	
	//エラーチェック.
	if( ( start & 0xFFF ) || ( size & 0xFFF ) || (start >= FLASH_SIZE_EXT) || (size <= 0) || (start + size > FLASH_SIZE_EXT) ){	/* 2020/02/26 Harada OP_BTS-20942 LPDDR4パラメータ化け LPDDR4パラメータ領域の多重化対応 */
		printf("%s: start %"PRId32" size %"PRId32"\n", __FUNCTION__, start, size);
		return False;
	}
	
	// セマフォ取得.
	if( semID_Flash == NULL ){
		semID_Flash = SYSF_semMCreate(SEM_Q_PRIORITY | SEM_DELETE_SAFE | SEM_INVERSION_SAFE);
	}
	SYSF_semTake( semID_Flash, WAIT_FOREVER );
	// 2019/12/19 河村 -  OP_BTS-18210 一時的にタスクプライオリティを上げる.
	if (0 != pthread_getschedparam( pthread_self(), &save_policy, &save_sched ) ) {
		SYSF_semGive( semID_Flash );
		return False;
	}
	struct sched_param sched = {};
	sched.sched_priority = 207;		/* 通信系タスクと同等 */
	if ( save_sched.sched_priority < sched.sched_priority ) {
		printf( "(%010u)(BSP)[%s] %s : prio %d -> %d\n", SYSF_tickGet(), __FUNCTION__, SYSF_taskName(SYSF_taskIdSelf()), save_sched.sched_priority, sched.sched_priority );
		if( 0 != pthread_setschedparam(pthread_self(), SCHED_RR, &sched) ) {
			SYSF_semGive( semID_Flash );
			return False;
		}
	}
	
	fd = getFlashFd();
	if( fd < 0 ){
		// 2019/12/19 河村 -  OP_BTS-18210 タスクプライオリティを戻す.
		if ( save_sched.sched_priority < sched.sched_priority ) {
			printf( "(%010u)(BSP)[%s] %s : prio %d -> %d\n", SYSF_tickGet(), __FUNCTION__, SYSF_taskName(SYSF_taskIdSelf()), sched.sched_priority, save_sched.sched_priority );
			pthread_setschedparam( pthread_self(), save_policy, &save_sched );
		}
		SYSF_semGive( semID_Flash );
		return False;
	}
	
	// イレース.
	int ret = 0;
	struct	erase_info_user	info;	/* イレース用構造体 */

	info.start	= start;
	info.length	= size;
	// 2017/03/25 PS1-ft_kobayashi AR1015411 リトライ処理を追加.
	for(uc_RetryCnt = 0; uc_RetryCnt < DEF_ERASE_RETRY_MAXNUM; uc_RetryCnt++){
		ret = SYSF_ioctl( fd, MEMERASE, (intptr_t)&info );
		if(ret >= 0){
			break;
		}
	}
	if( ret < 0 ){
	// 2016/06/20 山口 CQ.530652 SPI-Flashへのイレースに失敗した場合、新詳細コードを残す.
#if defined(TORNADO) && !defined(FUM_LINUX) /*実機*/
		uint64_t ull_NewDetailCode = 0x0;
		Bool bl_ret = True;

		ull_NewDetailCode = (((uint64_t)0x22                                  << 56) & 0xff00000000000000)	// モジュール番号.
						  | (((uint64_t)0x007                                 << 44) & 0x00fff00000000000)	// 通し番号.
						  | (((uint64_t)((SYSF_errnoGet() < 0) ?  -SYSF_errnoGet() :  SYSF_errnoGet())  << 36) & 0x00000ff000000000)	// errno.
						  | (((uint64_t)start                                 << 16) & 0x0000000fffff0000)	// 書き込み位置のオフセット.
						  | (((uint64_t)((size < 0xffff) ?  size :  0xffff)   <<  0) & 0x000000000000ffff);	// サイズ.

		bl_ret = APIC_setNewDetailCode( TYPE_FatalDetailCode_undefined, ull_NewDetailCode );		// 詳細コード登録.
		if (False == bl_ret) {
			SYSF_logMsg( "## %s %d : APIC_setNewDetailCode NG\n", __FUNCTION__, __LINE__, 3,4,5,6 );
		}
#endif	// #if defined(TORNADO) && !defined(FUM_LINUX).

		printf("%s: SYSF_ioctl ERR errno:%d\n", __FUNCTION__, SYSF_errnoGet());
		// 2019/12/19 河村 -  OP_BTS-18210 タスクプライオリティを戻す.
		if ( save_sched.sched_priority < sched.sched_priority ) {
			printf( "(%010u)(BSP)[%s] %s : prio %d -> %d\n", SYSF_tickGet(), __FUNCTION__, SYSF_taskName(SYSF_taskIdSelf()), sched.sched_priority, save_sched.sched_priority );
			pthread_setschedparam( pthread_self(), save_policy, &save_sched );
		}
		SYSF_semGive( semID_Flash );
		return False;
	}
	// 2019/12/19 河村 -  OP_BTS-18210 タスクプライオリティを戻す.
	if ( save_sched.sched_priority < sched.sched_priority ) {
		printf( "(%010u)(BSP)[%s] %s : prio %d -> %d\n", SYSF_tickGet(), __FUNCTION__, SYSF_taskName(SYSF_taskIdSelf()), sched.sched_priority, save_sched.sched_priority );
		pthread_setschedparam( pthread_self(), save_policy, &save_sched );
	}
	// セマフォ開放.
	SYSF_semGive( semID_Flash );
	
	return True;
}
/* 2020/02/26 Harada OP_BTS-20942 LPDDR4パラメータ化け LPDDR4パラメータ領域の多重化対応 << */


/* 2021/01/05 Harada IT6 2.1 OP_BTS-30215 認証装置を認証しない *//* 同件として対応 *//* 市場問題 TAS3758 E301 */
// ================================================================================================.
//! <!--機能--> USB Hubのリセット処理
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数--> なし.
//! <!--戻値--> なし.
//! <!--詳細-->.
//! FAX＋認証装置装着で省電力移行時、Hubでエラー、または認証装置でエラーが発生する対応
//! USB Hubをリセットする。
//! USB HubをリセットすることによりUSBデバイスを再接続させる。
//! ※Cソースモジュールからコール可能。
//! <!--履歴-->.
//  2021/01/05	Harada	新規作成.
// ================================================================================================.
void THRF_USBHubReset(char *c_PortNum)
{
#if defined(TORNADO) && !defined(FUM_LINUX)	/* 実機で有効 */

	static SEM_ID semID_HubReset = NULL;
	char data[65] = {};
	// エラーチェック.
	if( c_PortNum == NULL ){
		printf("%s: NULL ERR \n", __FUNCTION__);
		return;
	}
	// セマフォ取得.
	if( semID_HubReset == NULL ){
		semID_HubReset = SYSF_semMCreate(SEM_Q_PRIORITY | SEM_DELETE_SAFE | SEM_INVERSION_SAFE);
	}
	SYSF_semTake( semID_HubReset, WAIT_FOREVER );

	if ( '3' == c_PortNum[0] ) {	/* busnumが3の場合のみ実行する *//* 2021/01/18 Harada IT6 2.1 OP_BTS-30504 親デバイスからリセット 【暫定】 */
		memset(data, 0, sizeof(data));
/*		snprintf(data, sizeof(data)-1 ,"echo -n %s > /sys/bus/usb/drivers/usb/unbind", c_PortNum );*/
		snprintf(data, sizeof(data)-1 ,"echo -n 3-0:1.0 > /sys/bus/usb/drivers/hub/unbind");	/* 2021/01/18 Harada IT6 2.1 OP_BTS-30504 親デバイスからリセット 【暫定】 */
		system(data);	// 指定ポートに対してunbind
		memset(data, 0, sizeof(data));
/*		snprintf(data, sizeof(data)-1 ,"echo -n %s > /sys/bus/usb/drivers/usb/bind", c_PortNum );*/
		snprintf(data, sizeof(data)-1 ,"echo -n 3-0:1.0 > /sys/bus/usb/drivers/hub/bind");		/* 2021/01/18 Harada IT6 2.1 OP_BTS-30504 親デバイスからリセット 【暫定】 */
		system(data);	// 指定ポートに対してbind

		memset(data, 0, sizeof(data));
/*		snprintf(data, sizeof(data)-1 ,"echo -n %s > /sys/bus/usb/drivers/usb/unbind & bind", c_PortNum );*/
		snprintf(data, sizeof(data)-1 ,"echo -n 3-0:1.0 > /sys/bus/usb/drivers/hub/unbind & bind");	/* 2021/01/18 Harada IT6 2.1 OP_BTS-30504 親デバイスからリセット 【暫定】 */
		SYSF_logMsg( "(%010u)(BSP) %s : %s\n", SYSF_tickGet() , __FUNCTION__, data, 3,4,5,6 );
	}
	/* 2021/04/28 Harada IT6 2.2 OP_BTS-32570 マシン放置状態でリブート発生 新詳細コード0x2201E 無線LANエラー */
	else if ( '4' == c_PortNum[0] ) {	/* busnumが4の場合実行する */
		if(True == THRF_isExistWLAN()) {	/* 無線LAN装着*/
			if ( (TYPE_ExNetworkAdapter_Disable == APIC_SystemInfo::newInstance()->getExNetworkAdapter())					/* 拡張ネットワークアダプタ設定：無効 */
				||
				 ( (TYPE_ExNetworkAdapter_Wireless == APIC_SystemInfo::newInstance()->getExNetworkAdapter())				/* 拡張ネットワークアダプタ設定：無線アダプタ装着 */
				&& ( (TYPE_NetworkIFMode_SingleEthernet == APIC_ExNetworkSetting::newInstance()->getNetworkIFMode())		/* ネットワークインタフェースモード：シングルIF構成 有線モード */
				  || (TYPE_NetworkIFMode_DualEthernet == APIC_ExNetworkSetting::newInstance()->getNetworkIFMode()) ) ) )	/* ネットワークインタフェースモード：デュアルIF構成 有線モード */
			{
				memset(data, 0, sizeof(data));
/*				snprintf(data, sizeof(data)-1 ,"echo -n %s > /sys/bus/usb/drivers/usb/unbind", c_PortNum );*/
				snprintf(data, sizeof(data)-1 ,"echo -n 4-0:1.0 > /sys/bus/usb/drivers/hub/unbind");	/* 2021/01/18 Harada IT6 2.1 OP_BTS-30504 親デバイスからリセット 【暫定】 */
				system(data);	// 指定ポートに対してunbind
				memset(data, 0, sizeof(data));
/*				snprintf(data, sizeof(data)-1 ,"echo -n %s > /sys/bus/usb/drivers/usb/bind", c_PortNum );*/
				snprintf(data, sizeof(data)-1 ,"echo -n 4-0:1.0 > /sys/bus/usb/drivers/hub/bind");		/* 2021/01/18 Harada IT6 2.1 OP_BTS-30504 親デバイスからリセット 【暫定】 */
				system(data);	// 指定ポートに対してbind

				memset(data, 0, sizeof(data));
/*				snprintf(data, sizeof(data)-1 ,"echo -n %s > /sys/bus/usb/drivers/usb/unbind & bind", c_PortNum );*/
				snprintf(data, sizeof(data)-1 ,"echo -n 4-0:1.0 > /sys/bus/usb/drivers/hub/unbind & bind");	/* 2021/01/18 Harada IT6 2.1 OP_BTS-30504 親デバイスからリセット 【暫定】 */
				SYSF_logMsg( "(%010u)(BSP) %s : %s\n", SYSF_tickGet() , __FUNCTION__, data, 3,4,5,6 );
			}
		}
	}

	SYSF_semGive( semID_HubReset );

#endif // defined(TORNADO) && !defined(FUM_LINUX)	/* 実機で有効 */.
}

// ================================================================================================.
//! <!--機能--> 認証装置のリセット処理
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数--> なし.
//! <!--戻値--> なし.
//! <!--詳細-->.
//! FAX＋認証装置装着で省電力移行時、Hubでエラー、または認証装置でエラーが発生する対応
//! 認証装置をリセットする。
//! 認証装置をリセットすることにより認証装置を再接続させる。
//! ※Cソースモジュールからコール可能。
//! ※Cソースモジュールからコールされる。
//! <!--履歴-->.
//  2021/01/05	Harada	新規作成.
// ================================================================================================.
void THRF_USBAuthDeviceReset(void)
{
#if defined(TORNADO) && !defined(FUM_LINUX)	/* 実機で有効 */

	static SEM_ID semID_AuthDeviceReset = NULL;
	// セマフォ取得.
	if( semID_AuthDeviceReset == NULL ){
		semID_AuthDeviceReset = SYSF_semMCreate(SEM_Q_PRIORITY | SEM_DELETE_SAFE | SEM_INVERSION_SAFE);
	}
	SYSF_semTake( semID_AuthDeviceReset, WAIT_FOREVER );

	// Sparrow実機(MFP/AIO/SFP)又はDenebMLK実機の場合(SparrowEmuはここに入らない).
#if ( defined( DEF_SparrowCus ) || defined( DEF_DenebMLKCus ) ) && !defined(DEF_S800Emu)
	// Sparrow実機、又はDenebMLK実機.
	system("echo -n 3-1.2 > /sys/bus/usb/drivers/usb/unbind");		// AuthDeviceに対してunbind.
	system("echo -n 3-1.2 > /sys/bus/usb/drivers/usb/bind");		// AuthDeviceに対してbind.
	SYSF_logMsg( "(%010u)(BSP) %s : echo -n 3-1.2 > /sys/bus/usb/drivers/usb/unbind & bind\n", SYSF_tickGet() , __FUNCTION__, 2,3,4,5,6 );

	// SparrowEmu機、DenebMLKEmu機は↓を通す.
#else  // #if defined( DEF_SparrowCus ) && !defined(DEF_S800Emu).
	// Eagle/Emu800. 
	system("echo -n 3-1.1.3 > /sys/bus/usb/drivers/usb/unbind");	// AuthDeviceに対してunbind.
	system("echo -n 3-1.1.3 > /sys/bus/usb/drivers/usb/bind");		// AuthDeviceに対してbind.
	SYSF_logMsg( "(%010u)(BSP) %s : echo -n 3-1.1.3 > /sys/bus/usb/drivers/usb/unbind & bind\n", SYSF_tickGet() , __FUNCTION__, 2,3,4,5,6 );
#endif // #if defined( DEF_SparrowCus ) && !defined(DEF_S800Emu).

	SYSF_semGive( semID_AuthDeviceReset );

#endif // defined(TORNADO) && !defined(FUM_LINUX)	/* 実機で有効 */.
}

// 2022/03/30 生産RINNE対応 Sky start.
#ifdef SIMULATOR
const char* 	THRD_RINNE_SETTING_FILE_PATH = "/root/work/linemodeinfo";
#else
const char*		THRD_RINNE_SETTING_FILE_PATH = "/bd/sdc1/linemodeinfo";
#endif
const char*		THRD_RINNE_AES_PW	= "rinneennir";
const char* 	THRD_RINNE_AES_SALT	= "2D45205F17DB4265";
const uint32_t	THRD_RINNE_SETTING_FILE_SIZE	= 2048;

// ================================================================================================.
//! <!--機能--> 設定ファイルをReadして復号化する.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数--> 暗号化対象の設定値(文字列).
//! <!--戻値--> なし.
//! <!--詳細-->.
//! <!--履歴-->.
// ================================================================================================.
void THRF_DecodeLineModeSettingFile( char* pc_output, uint32_t ui_outputsize )
{
	if( NULL == pc_output ) {
		return;
	}	
	
	char sc_cmd[THRD_RINNE_SETTING_FILE_SIZE] = {};
	snprintf(sc_cmd, sizeof(sc_cmd), "openssl enc -aes-256-cbc -d -pass pass:%s -S %s -in %s", THRD_RINNE_AES_PW, THRD_RINNE_AES_SALT, THRD_RINNE_SETTING_FILE_PATH);
	FILE*	p_Fp = NULL;
	if ( NULL != ( p_Fp = popen(sc_cmd,"r") ) ) {
		fgets( pc_output, ui_outputsize, p_Fp );
		pclose(p_Fp);
		p_Fp = NULL;
	}
}

// ================================================================================================.
//! <!--機能--> 設定ファイルを暗号化してWriteする.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数--> 暗号化対象の設定値(文字列).
//! <!--戻値--> なし.
//! <!--詳細-->.
//! <!--履歴-->.
// ================================================================================================.
void THRF_EncodeLineModeSettingFile( const char* pc_input )
{
	if( NULL == pc_input ) {
		return;
	}

	char sc_cmd[THRD_RINNE_SETTING_FILE_SIZE] = {};
	snprintf(sc_cmd, sizeof(sc_cmd), "echo '%s' | openssl enc -aes-256-cbc -e -pass pass:%s -e -S %s -out %s",pc_input, THRD_RINNE_AES_PW, THRD_RINNE_AES_SALT, THRD_RINNE_SETTING_FILE_PATH);
	system(sc_cmd);
}

// ================================================================================================.
//! <!--機能--> ラインカードから設定値をReadする.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数--> なし.
//! <!--戻値--> Bool Read結果.
//! <!--詳細-->.
//! <!--履歴-->.
// ================================================================================================.
Bool THRF_ReadLineModeSettingFile(void)
{
	printf( "LineModeInfo ==========================================================\n" );

	Bool b_NoError = True; 
	TYPS_LineModeInfo s_LineModeInfo = {};

	struct LINEMODE_KEYWORD_STRUCT {
		char*			pc_Keyword;
		Bool			b_IsNeed;
		void*			pc_Buffer;
		size_t			st_BufferSize;
		Json::ValueType	e_ValueType;
	};

	struct LINEMODE_KEYWORD_STRUCT LINEMODE_KEYWORD_TABLE[]= {
		{	"rinneUrl",					True,	s_LineModeInfo.sc_RinneSeverContentsUrl,	sizeof(s_LineModeInfo.sc_RinneSeverContentsUrl)	,Json::stringValue	},
		{	"staticipAddress",			False,	s_LineModeInfo.sc_Static_IPAddress,			sizeof(s_LineModeInfo.sc_Static_IPAddress)		,Json::stringValue	},
		{	"staticipSubnetmask",		False,	s_LineModeInfo.sc_Static_Subnetmask,		sizeof(s_LineModeInfo.sc_Static_Subnetmask)		,Json::stringValue	},
		{	"staticipGateway",			False,	s_LineModeInfo.sc_Static_Gateway,			sizeof(s_LineModeInfo.sc_Static_Gateway)		,Json::stringValue	},
		{	"wlanSsid",					False,	s_LineModeInfo.sc_Wlan_SSID,				sizeof(s_LineModeInfo.sc_Wlan_SSID)				,Json::stringValue	},
		{	"wlanKey",					False,	s_LineModeInfo.sc_Wlan_Key,					sizeof(s_LineModeInfo.sc_Wlan_Key)				,Json::stringValue	},
		{	"wlanDrivername",			False,	s_LineModeInfo.sc_DriverName,				sizeof(s_LineModeInfo.sc_DriverName)			,Json::stringValue	},
		{	"stationId",				False,	s_LineModeInfo.sc_StationId,				sizeof(s_LineModeInfo.sc_StationId)				,Json::stringValue	},
		{	"lineId",					False,	s_LineModeInfo.sc_LineId,					sizeof(s_LineModeInfo.sc_LineId)				,Json::stringValue	},
		{	"appId",					False,	s_LineModeInfo.sc_AppId,					sizeof(s_LineModeInfo.sc_AppId)					,Json::stringValue	},
		{	"subhopperCheck",			False,	&s_LineModeInfo.b_SubhopperCheck,			0												,Json::booleanValue	},
		{	"tonerbottleRotationcount",	False,	&s_LineModeInfo.ss_tonerbottleRotationcount,0												,Json::uintValue	},
		{	"webdavUrl",				False,	s_LineModeInfo.sc_WebdavUrl,				sizeof(s_LineModeInfo.sc_WebdavUrl)				,Json::stringValue	},
	};

	size_t LINEMODE_KEYWORD_TABLE_SIZE = sizeof(LINEMODE_KEYWORD_TABLE) / sizeof(LINEMODE_KEYWORD_STRUCT); 

	string jsonfile( THRD_RINNE_SETTING_FILE_SIZE, '\0' );
	THRF_DecodeLineModeSettingFile(&jsonfile[0],THRD_RINNE_SETTING_FILE_SIZE);

	Json::Reader reader;
	Json::Value value;
	reader.parse(jsonfile, value);

	for( uint32_t i = 0; i < (uint32_t)LINEMODE_KEYWORD_TABLE_SIZE; i++ ) {
		const char*	pc_Keyword = LINEMODE_KEYWORD_TABLE[i].pc_Keyword;
		if(value.isMember(pc_Keyword)){
			switch(LINEMODE_KEYWORD_TABLE[i].e_ValueType) {
				case Json::stringValue:
				{
					strncpy(reinterpret_cast <char*>(LINEMODE_KEYWORD_TABLE[i].pc_Buffer), value[pc_Keyword].asString().c_str(), LINEMODE_KEYWORD_TABLE[i].st_BufferSize - 1);
					break;
				}
				case Json::booleanValue:
				{
					BOOL* temp = reinterpret_cast <BOOL*>(LINEMODE_KEYWORD_TABLE[i].pc_Buffer);
					if(true == value[pc_Keyword].asBool()){
						*temp = TRUE;
					}else{
						*temp = FALSE;
					}
					break;
				}
				case Json::uintValue:
				{
					unsigned int* temp = reinterpret_cast <unsigned int*>(LINEMODE_KEYWORD_TABLE[i].pc_Buffer);
					*temp = static_cast<uint32_t>(value[pc_Keyword].asUInt());
					break;
				}
				default:
					printf( "[Table error] key[%s] type[%d]\n",pc_Keyword,LINEMODE_KEYWORD_TABLE[i].e_ValueType);
					break;
			}
		}else{
			if(LINEMODE_KEYWORD_TABLE[i].b_IsNeed){
				b_NoError = False;
				printf( "[Format error] need[%s]\n",pc_Keyword);
				break;
			}
		}
	}

	printf( "RinneSeverContentsUrl = %s\n" , s_LineModeInfo.sc_RinneSeverContentsUrl );
	printf( "Static_IPAddress      = %s\n" , s_LineModeInfo.sc_Static_IPAddress );
	printf( "Static_Subnetmask     = %s\n" , s_LineModeInfo.sc_Static_Subnetmask );
	printf( "Static_Gateway        = %s\n" , s_LineModeInfo.sc_Static_Gateway );
	printf( "wlan_ssid             = %s\n" , s_LineModeInfo.sc_Wlan_SSID );
//		printf( "wlan_key              = %s\n" , s_LineModeInfo.sc_Wlan_Key );	パスワードのため出力しない.
	printf( "DriverName            = %s\n" , s_LineModeInfo.sc_DriverName );
	printf( "StationId             = %s\n" , s_LineModeInfo.sc_StationId );
	printf( "LineId                = %s\n" , s_LineModeInfo.sc_LineId );
	printf( "AppId                 = %s\n" , s_LineModeInfo.sc_AppId );
	printf( "SubhopperCheck        = %d\n" , s_LineModeInfo.b_SubhopperCheck );
	printf( "tonerbottleRotation   = %d\n" , s_LineModeInfo.ss_tonerbottleRotationcount );
	printf( "WebdavUrl             = %s\n" , s_LineModeInfo.sc_WebdavUrl );
				
	if( b_NoError ) {
		APIC_SystemInfo::newInstance()->setCardLineModeInfo(&s_LineModeInfo);
	}

	printf( "=======================================================================\n" );
	return b_NoError;
}

// ================================================================================================
//! <!--機能--> 無線LAN接続待ち. 
//! <!---------------------------------------------------------------------------------------------
//! <!--引数--> Wlan番号.
//! <!--戻値--> Bool True:接続OK、False:タイムアウト.
//! <!--詳細--> 無線LANが接続させるのを待つ.
//! <!--履歴--> 2022/11/16 Sky 新規作成.
// ================================================================================================
Bool THRF_WaitWlanConnected(uchar *uc_WlanNo)
{
	Bool bl_wlan0Connected = False;
	Bool bl_wlan1Connected = False;
	uint32_t ui32_ErrCnt = 0;
	static Bool bl_Timeout = False;
	static uchar uc_oldWlanNo = 0;
	if(bl_Timeout == True){
		*uc_WlanNo = uc_oldWlanNo;
		return False;
	}
	for(uint32_t i = 0; i < 60; i++){
		// IF名取得.
		struct ifaddrs *ifa_list = NULL;
		struct ifaddrs *ifa;
		uint32_t ui32_Ret = getifaddrs(&ifa_list);
		if(ui32_Ret != 0){
			ui32_ErrCnt++;
			sleep(1);
			continue;
		}
		for(ifa = ifa_list; ifa != NULL; ifa = ifa->ifa_next){
			printf("###ifa_name=%s\n", ifa->ifa_name);
			if(strcmp(ifa->ifa_name, "wlan0") == 0){
				bl_wlan0Connected = True;
			}
			if(strcmp(ifa->ifa_name, "wlan1") == 0){
				bl_wlan1Connected = True;
			}
		}
		if(NULL != ifa_list){
			freeifaddrs(ifa_list);
		}
		if(bl_wlan0Connected == True && bl_wlan1Connected == True){
			printf("[THRF_WaitWlanConnected] connected.\n");
			*uc_WlanNo = 2;
			return True;
		}
		sleep(1);
	}
	printf("[THRF_WaitWlanConnected] time out! getifaddrs err count=%d\n", ui32_ErrCnt);
	
	// Wlan番号設定.
	if(bl_wlan0Connected == False){
		*uc_WlanNo = 0;
	}else{
		*uc_WlanNo = 1;
	}
	bl_Timeout = True;
	uc_oldWlanNo = *uc_WlanNo;

	return False;
}

// wlan名.
static char sc_wlanName[6] = {0};

// ================================================================================================
//! <!--機能--> ラインモード時の無線LAN設定. 
//! <!---------------------------------------------------------------------------------------------
//! <!--引数--> TYPS_LineModeInfo構造体.
//! <!--戻値--> なし
//! <!--詳細--> ラインモード時の無線LAN設定を行う.
//! <!--履歴--> 2021/7/19 Sky 新規作成.
//!             2022/11/16 Sky 無線LAＮの装着設定を有効化すると、Rinne画面が表示されない不具合対応.
// ================================================================================================
void THRF_SetLineModeWifiParam(const TYPS_LineModeInfo& s_LineModeInfo)
{
	APIC_ExNetworkSetting* pc_NetworkSetting = APIC_ExNetworkSetting::newInstance();
	if(NULL == pc_NetworkSetting){
		printf("[THRF_SetLineModeWifiParam] SystemErr\n");
		return;
	}
	TYPE_NetworkIFMode e_NwIfMode = pc_NetworkSetting->getNetworkIFMode();
	printf("[THRF_SetLineModeWifiParam] NetworkIFMode=%d\n", e_NwIfMode);
	switch(e_NwIfMode){
	// 無線LAN有効.
	case TYPE_NetworkIFMode_SingleSta:         // シングルIF構成 無線子機モード.
	case TYPE_NetworkIFMode_DualEthernetSta:   // デュアルIF構成 有線＋無線子機モード.
	case TYPE_NetworkIFMode_DualEthernetAp:    // デュアルIF構成 有線＋無線APモード.
	case TYPE_NetworkIFMode_DualEthernetWfd:   // デュアルIF構成 有線+Wi-Fi Directモード.
	{
		uchar uc_WlanNo = 0;
		Bool bl_Ret = THRF_WaitWlanConnected(&uc_WlanNo);
		// Wlan番号設定.
		if(bl_Ret == True){
			strncpy(sc_wlanName, "wlan2", sizeof(sc_wlanName));
		}else{
			// タイムアウト時の設定.
			char tempwlanno[16] ={};
			snprintf(tempwlanno,16,"%s%d","wlan",uc_WlanNo);
			strncpy(sc_wlanName, tempwlanno, sizeof(sc_wlanName));
		}
	}
		break;
	// 無線LAN無効.
	default:
		strncpy(sc_wlanName, "wlan0", sizeof(sc_wlanName));
		break;
	}
	
	system("insmod /km/module/cfg80211.ko");
	
	char tempDriverName[256] ={};
	if( 0 != strlen(s_LineModeInfo.sc_DriverName )) {
		snprintf(tempDriverName,256,"%s%s%s","insmod /bd/sdc1/",s_LineModeInfo.sc_DriverName,".ko");
	}else{
		snprintf(tempDriverName,256,"%s%s%s","insmod /km/module/","8812bu",".ko");
	}
	system(tempDriverName);

	char tempssid[256] ={};
	snprintf(tempssid,256,"%s%s%s","echo \" ssid=\\\"",s_LineModeInfo.sc_Wlan_SSID,"\\\"\" >> /tmp/wpa_supplicant_sta.conf");
	char temppsk[256] ={};
	snprintf(temppsk,256,"%s%s%s","echo \" psk=\\\"",s_LineModeInfo.sc_Wlan_Key,"\\\"\" >> /tmp/wpa_supplicant_sta.conf");
							
	system("echo \"ctrl_interface=/var/run/wpa_supplicant\" > /tmp/wpa_supplicant_sta.conf");
	system("echo \"network={\" >> /tmp/wpa_supplicant_sta.conf");
	system(tempssid);
	system("echo \" proto=RSN\" >> /tmp/wpa_supplicant_sta.conf");
	system("echo \" pairwise=CCMP\" >> /tmp/wpa_supplicant_sta.conf");
	system("echo \" group=CCMP\" >> /tmp/wpa_supplicant_sta.conf");
	system(temppsk);
	system("echo \" scan_ssid=1\" >> /tmp/wpa_supplicant_sta.conf");
	system("echo \"}\" >> /tmp/wpa_supplicant_sta.conf");
	
	if( 0 != strlen(s_LineModeInfo.sc_Static_IPAddress) &&
		0 != strlen(s_LineModeInfo.sc_Static_Subnetmask) &&
		0 != strlen(s_LineModeInfo.sc_Static_Gateway)){
		char tempifconfigup[256] ={};
		snprintf(tempifconfigup,256,"%s%s%s","ifconfig ",sc_wlanName," up");
		system(tempifconfigup);
		char tempStaticIp[256] ={};
		snprintf(tempStaticIp,256,"%s%s%s%s%s%s","ifconfig ",sc_wlanName," inet ", s_LineModeInfo.sc_Static_IPAddress," netmask ",s_LineModeInfo.sc_Static_Subnetmask);
		system(tempStaticIp);
		char tempStaticGateway[256] ={};
		snprintf(tempStaticGateway,256,"%s%s%s%s","route add default gw ",s_LineModeInfo.sc_Static_Gateway," ",sc_wlanName);
		system(tempStaticGateway);
	}else{
		char tempudhcpc[256] ={};
		snprintf(tempudhcpc,256,"%s%s%s","udhcpc -i ",sc_wlanName," -b");
		system(tempudhcpc);
	}
	
	char temprouteadd[256] ={};
	snprintf(temprouteadd,256,"%s%s","route add -net 224.0.0.0 netmask 240.0.0.0  ",sc_wlanName);
	system(temprouteadd);
	char tempsupplicant[256] ={};
	snprintf(tempsupplicant,256,"%s%s","/km/fw/lib/wlan/wpa_supplicant -B -Dnl80211 -c /tmp/wpa_supplicant_sta.conf -i ",sc_wlanName);
	system(tempsupplicant);
}

// ================================================================================================
//! <!--機能--> ラインモード時の無線LAN有効化待ち. 
//! <!---------------------------------------------------------------------------------------------
//! <!--引数--> なし
//! <!--戻値--> なし
//! <!--詳細--> ラインモード時の無線LAN設定を行い有効化させるのを待つ.
//! <!--履歴--> 2021/7/19 Sky 新規作成.
//!             2022/11/16 Sky 無線LAＮの装着設定を有効化すると、Rinne画面が表示されない不具合対応.
// ================================================================================================
Bool THRF_IsWlanConnected()
{
	if(sc_wlanName[0] == 0){
		return False;
	}
	int status = 0;
	int fd = 0;
	struct ifreq ifreq = {};
	strncpy(ifreq.ifr_name, sc_wlanName, sizeof(ifreq.ifr_name));
	fd = socket(PF_INET, SOCK_DGRAM, 0);
	status = ioctl(fd, SIOCGIFADDR, &ifreq);
	close(fd);
	if (status == -1)
	{
		return False;
	}

	FILE *fp = NULL;
	char buf[256] = {0};
	char sc_command[256 + 1] = {0};
	snprintf(sc_command, 256, "iw dev %s link", sc_wlanName);
	if (NULL == (fp = popen(sc_command, "r")))
	{
		return False;
	}
	else
	{
		Bool hitflg = False;
		while (fgets(buf, sizeof(buf), fp))
		{
			if (NULL != strstr(buf, "SSID"))
			{
				hitflg = True;
				break;
			}
		}
		pclose(fp);
		if (False == hitflg)
		{
			return False;
		}
	}

	return True;
}

// 2022/03/30 生産RINNE対応 Sky end.

/* 2022/11/17 Harada OP_BTS-40197 RTCの時刻ずれ補正対応 */
// ================================================================================================.
//! <!--機能--> RTC時刻をリストアしたのか取得する.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数--> なし.
//! <!--戻値--> .
//! Bool		True：リストアを実行した、False：リストア実行していない.
//! <!--詳細--> RTC時刻をリストアしたのか取得する.
//! <!--履歴-->.
//  2022/11/17	Harada	新規作成.
// ================================================================================================.
Bool get_Rtc_Time_Restored(void)
{
#if defined(TORNADO) && !defined(FUM_LINUX)	/* 実機で有効 */
	uchar uc_code = 0;
	uchar uc_ret = 0;
	Bool bl_ret = False;
	// RTC時刻初期化発生フラグ(CommonAPIへ通知用)をQSPIからReadする. 
	uc_ret = THRG_getBspThrRegion( 0x004E, (void*)&uc_code, sizeof(uc_code) );
	if( ( uc_ret == THRD_GET_SPI_OK ) || ( uc_ret == THRD_GET_SPI_OK_AREA1_ERR ) || ( uc_ret == THRD_GET_SPI_OK_AREA2_ERR ) ){
		// 取得成功.
	} else {
		// 取得失敗.
		SYSF_logMsg( "(%010u)(BSP) ## %s %d : THRG_getBspThrRegion NG\n", SYSF_tickGet() , __FUNCTION__, __LINE__, 3,4,5,6 );
		bl_ret = False;
	}

	if (0x01 == uc_code) {		// RTC時刻初期化発生フラグ(CommonAPIへ通知用)が立っている場合.
		bl_ret = True;
	}
	return bl_ret;
#else // defined(TORNADO) && !defined(FUM_LINUX)	/* 実機で有効 */.
	return False;
#endif // defined(TORNADO) && !defined(FUM_LINUX)	/* 実機で有効 */.
}
// ================================================================================================.
//! <!--機能--> RTC時刻をリストアしたことをクリアする.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数--> なし.
//! <!--戻値--> なし.
//! <!--詳細--> RTC時刻をリストアしたことをクリアする.
//! <!--履歴-->.
//  2022/11/17	Harada	新規作成.
// ================================================================================================.
void clear_Rtc_Time_Restored(void)
{
#if defined(TORNADO) && !defined(FUM_LINUX)	/* 実機で有効 */
	uchar uc_code = 0;
	uchar uc_ret = 0;
	Bool bl_ret = False;
	// RTC時刻をリストアしたのか取得する.
	bl_ret = get_Rtc_Time_Restored();

	if (True == bl_ret) {		// リストアを実行した場合.
		uc_code = 0;
		// RTC時刻初期化発生フラグ(CommonAPIへ通知用)をクリア.
		uc_ret = THRG_setBspThrRegion( 0x004E, (void*)&uc_code, sizeof(uc_code) );
		if ( THRD_SET_SPI_OK != uc_ret ) {
			/* エラー */
			SYSF_logMsg( "(%010u)(BSP) ## %s %d : THRG_setBspThrRegion NG\n", SYSF_tickGet() , __FUNCTION__, __LINE__, 3,4,5,6 );
		}
	}
#endif // defined(TORNADO) && !defined(FUM_LINUX)	/* 実機で有効 */.
	return;
}

// ================================================================================================.
//! <!--機能--> Reboot用のwatchdogを開始する.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数--> uchar	moduleId	(I) モジュール番号を設定する(BSPは0x22など)																										
//! 			ushort	secTime		(I) リブートまでの時間設定(秒単位)																										
//! <!--戻値--> void
//! <!--詳細-->.
//! <!--履歴--> 2022/11/16 河村 - OP_BTS-40590 初版.
// ================================================================================================.
extern Bool isWdStart_CA72;	// CA72WDT Startフラグ.											
extern void ca72wdt_ext_refresh( uint32_t point, uint32_t timeout, uint32_t debugInfo1, char *debugInfo2 );
#define DEF_WDT_CA72_START 		0

void THRF_startRebootWatchdog( uchar moduleId, ushort secTime )
{
	int32_t si_Fd = 0;
	int32_t si_ret = 0;

	isWdStart_CA72 = False;		// リフレッシュの停止.
	
	// 起動後すぐのタイミングではWatchdog startしていない可能性があるので開始する(多重スタートはOK).
	si_Fd = SYSF_open(THRD_DeviceFile_Pmu, O_RDWR, 0);
	if( si_Fd >= 0 ) {
		si_ret = ioctl( si_Fd, THRD_PMU_IOCSSET_WDT_CA72, DEF_WDT_CA72_START );	// START.
		if( si_ret < 0 ) {
			printf( "(%010u)(BSP)[%s] ioctl error. errno:%d\n", SYSF_tickGet(), __FUNCTION__, SYSF_errnoGet() );
		}
		SYSF_close(si_Fd);
	} else {
		printf( "(%010u)(BSP)[%s] open error. errno:%d\n", SYSF_tickGet(), __FUNCTION__, SYSF_errnoGet() );
	}
	ca72wdt_ext_refresh( 0x7000 | ((ushort)moduleId), secTime, 0x0000, "THRF_startRebootWatchdog\n" );	// Timer Set.
}

/*================================================================================================
 * ［機能］ Threadのメモリ使用領域設定.
 * -----------------------------------------------------------------------------------------------
 * ［引数］	THRE_MemKind 	e_MemKind	：メモリ種別.
 *			uintptr_t		ul_addr		：開始アドレス.
 *			size_t			st_size		：サイズ.
 * ［戻値］	なし
 * ［詳細］ ファイルメモリやメインメモリのスレッドバッファの使用領域を設定する.
 * ［作成］ Sky.
 * ［履歴］	2023/01/10	Threadメモリ使用領域チェック対応.
 *================================================================================================*/
void THRF_SetUsageMemRange( THRE_MemKind e_MemKind, uintptr_t ul_addr, size_t st_size )
{
	switch(e_MemKind)
	{
		case THRE_MemFileMemory:		// ファイルメモリ.
		case THRE_MemBufImage_Main:		// Thredバッファメインメモリ.
			THRL_Drvrif_SetUsageMemRange(e_MemKind, ul_addr, st_size);
			break;

		case THRE_MemBufImage_Sub:		// Thredバッファサブメモリ.
		default:
			// 設定なし.
			break;
	}
}

// ================================================================================================.
//! <!--機能--> 共有メモリ通信タイムアウトの新詳細コード記録.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数--> なし.																										
//! <!--戻値--> void.
//! <!--詳細-->.
//! <!--履歴--> 2023/11/20 OP_BTS-42228 河村 初版.
// ================================================================================================.
extern int32_t mcGetInterrupt(void);
extern int32_t mcGetMcCpuInterrupt(void);

void THRF_setNewDetailCodeForSharedMememoryTimeout(void)
{
	uint64_t ull_NewDetailCode = 0x0;
	Bool bl_ret = True;
	uint32_t si_mc   = mcGetMcCpuInterrupt() & 0xffff;		// CTRL->MCのパルス割り込み要因.
	uint32_t si_ctrl = mcGetInterrupt()      & 0xffff;		// MC->CTRLのパルス割り込み要因.
	printf( "(%010u)(BSP)[%s] interrupt cause  MC->CTRL:0x%04x CTRL->MC:0x%04x :\n", SYSF_tickGet(), __FUNCTION__, si_ctrl, si_mc );

	ull_NewDetailCode = 0x2202e00000000000	// 共有メモリ通信タイムアウト.
			| ((uint64_t)si_mc)   << 16		// 31-16 CTRL->MC.
			| ((uint64_t)si_ctrl);			// 15- 0 MC->CTRL. 

	bl_ret = APIC_setNewDetailCode( TYPE_FatalDetailCode_undefined, ull_NewDetailCode );		// 詳細コード登録.
	if (False == bl_ret) {
		SYSF_logMsg( "## %s %d : APIC_setNewDetailCode NG\n", __FUNCTION__, __LINE__, 3,4,5,6 );
	}
}
