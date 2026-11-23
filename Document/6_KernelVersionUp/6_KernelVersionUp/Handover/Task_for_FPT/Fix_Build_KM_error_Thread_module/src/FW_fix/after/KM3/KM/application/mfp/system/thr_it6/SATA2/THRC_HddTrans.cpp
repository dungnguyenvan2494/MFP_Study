// ================================================================================================
//! \file THRC_HddTrans.cpp 制御単位「ＨＤＤ転送」
// ================================================================================================
/*================================================================================================
［タイトル］	制御単位「ＨＤＤ転送」
================================================================================================*/
/*================================================================================================
［備考］
  17/10/13 Yoneda     S800 ストレージ構成変更対応
    ソースコード内でデバイスをSSD,HDDと表記しているが、実際に処理を行う対象デバイスは異なっている。
    +--------+---------------+
    |  表記  |  対象デバイス |
    +--------+---------------+
    |  SSD   |     SD        |
    |  HDD   |     SSD       |
    +--------+---------------+
    ただし、S800対応で新規に追加した物は対象デバイス表記としている。
================================================================================================*/

/*================================================================================================
	インクルード・ファイル
================================================================================================*/
#include <errno.h>

#include <scsi/sg.h>

#include <sys/uio.h>	//  24/03/22 FPT		Fix build error GccVersionUp.
#include "system/nvd/APIC_Setting_Management.h"
#include "system/thr/thrInclude/THRC_ThreadInstance.h"
#include "system/fil/FILC_FilIf.h"
#include "system/fil/filg/FILC_BaseDir.h"
#include "system/fil/filg/FILC_OverWriteErase.h"
#include "system/thr/thrInclude/THRC_Vxworks.h"
#include "system/thr/thrInclude/THRC_TroubleCounter.h" //アンチパターン対応.
#include "system/prt/APIC_Printer.h"                        //アンチパターン対応.
#include "THRC_Hdd.h"
#include "THRC_HddTrans.h"
#include "sysBspHdd.h"
#include "netrx/netprintrx/netprintrxg/NETC_NetPrintRx.h"
#include "system/thr/thrInclude/THRC_PortInstance.h"
#include "system/thr/thrInclude/THRC_ImgTransSim.h"	// 画像転送シミュレータ 
#ifdef TORNADO		// VxWorksで有効
#include <Linux/Platform/InitMemory.h>				// 13/10/31 ADS滕飛 CQ.381189(Act_68696) EE-HDL-33 HDDレス化対応_切替条件変更.
#include "Linux/Platform/HW_interim.h"
#include "time.h"	
#include <sys/mount.h>
#include <linux/hdreg.h>
#include <asm/byteorder.h>
#include <errno.h>
#include <unistd.h>	// symlink(2) 使用時に必要.
#endif	// #ifdef TORNADO
#include <usb.h>
#include "Linux/Platform/Mediaif.h"
#include "THRC_AesTrans.h"
#include "system/sys/OsWrapper/SYSC_OSWrapper.h"
#include "system/sys/FsWrapper/SYSC_FSWrapper.h"
#include "system/thr/thrInclude/THRC_HddMaster.h"
#include "system/thr/thrInclude/ThrSysFWrapper.h"
#include "system/thr/thrInclude/THRC_ThreadDebug.h" // UV-01385 ログ出力.
#include "system/nvd/nvdg/NVDC_ClassIF.h"
#include "system/thr/thrInclude/THRC_Aeslib.h"
#include "system/pwr/APIC_Power.h"
#include "system/sys/Timer/SYSC_Timer.h"			// RTC時刻取得に使用.
#include "system/sys/APIC_FatalSignalDetailCode.h"	//CQ374156 森田 umountエラー詳細コード.
#include "system/nvd/nvdg/NVDG_NVWrite.h"			// CQ378754 宮本 SSDデータ消失防止でFA14対応.
#include "system/sys/APIC_InternetISW.h"            // UV-01385 ファームウェア更新失敗時のロールバック機能対応.
#include "system/sys/sysg/SYSC_System.h"
#include "system/sys/APIC_SsdDataTransfer.h"	// 2018/09/21 Sky井柳 【DenebMLK】制御構成変更No78「データ移行」対応.

// FWリファクタリング APIC_Setting.hの分割.
#include "system/nvd/APIC_Setting_SystemSetting.h"

// HCL-Viru[01/10/2014] added support to clear memory during buf2hdd tranasfer 
#ifdef FUM_LINUX
#include "system/thr/thrcom/THRC_ImgTransSim.h"
#endif

#ifdef SIMULATOR
#include "../../simulator/SimShared/SimCnvImg/SIMC_OPSimCommon.h"
#endif	// SIMULATOR

#include "system/thr/thr/verify.h"
#include "Linux/Platform/Mediaif.h"
#include "../thr/thrCrypto.h"

// 2020/01/31 Sky FW検証モードで使用するハッシュ値をスレッド/BPS領域にバックアップする対応 ->.
#include "system/nvd/APIC_Setting_OSSIndividualUpdate.h"
// 2020/01/31 Sky FW検証モードで使用するハッシュ値をスレッド/BPS領域にバックアップする対応 <-.

//	2020/09/28 伊藤史. OP_BTS-27780,27963 INDEXの値を書き換えることで、改ざんしたFWがダウンロード可能となる脆弱性の対応.
#include "system/thr_it6/thr/fwupdate_DEF.h"

// OP_BTS-30791対応 単体リブート中を判定する為のヘッダー追加.
#include "../../../../../shlib/RecoveryLib/LIBC_RecoveryLib_IF.h"

// 2021/6/16 ads河井.
#include <fstream>
#include <iostream>
#include <sstream>
#include <pwd.h>
#include <algorithm>
#include <linux/fs.h>
#include <sys/ioctl.h>
/*================================================================================================
	ＨＤＤパーティション、デイレクトリー、ファイル名定義
================================================================================================*/
/* //【695新アーキFUM】20111111 FBSC渡辺 新アーキ論理フォーマット対応 THRC_Partition.hへ移動 start.
#define THRD_DevName_Hdd01		("/ata01")			// デバイス名(Root Dir)	: "/ata01"
// 08/01/16 morikawa DonauEmperor一時データ上書き消去対応 /a0/app1から/ata03に変更.
#define THRD_DevName_Hdd03		("/ata03")		// 
#define THRD_DevName_Hdd04		("/ata04")		// 
// 08/02/13 morikawa Donau機種対応、管理領域追加.
#define THRD_DevName_Hdd05		("/ata05")		// IWS Management
#define THRD_DevName_Hdd06		("/ata06")		// IWS User
// 10/09/14 Sky松尾 ブラウザ用パーティション追加.
#define THRD_DevName_Hdd07		("/ata07")		// WebKitBrowser用.
*/ //【695新アーキFUM】20111111 FBSC渡辺 新アーキ論理フォーマット対応 THRC_Partition.hへ移動 end.

#define THRD_DirName_IJC		("ijc")						// ディレクトリー名　
#define THRD_FileName_IJC		(DEF_THR_DIR_PATH "chk_ijc.data")		// スレッドフォーマットチェック用ファイル名　

// 2010/3/5 takayama SSD対応. 
#ifdef DEF_SupportSSD

/* //【695新アーキFUM】20111111 FBSC渡辺 新アーキ論理フォーマット対応 THRC_Partition.hへ移動 start.
#define THRD_DevName_Ssd0a		("/ata0a")		// 
#define THRD_DevName_NVDData	("/ata0b")		// 2010/12/10 MinervaZeus対応 河村 - NVD不揮発データ保存領域.
*/ //【695新アーキFUM】20111111 FBSC渡辺 新アーキ論理フォーマット対応 THRC_Partition.hへ移動 end.
#define THRD_FileName_IJC_SSD	(DEF_THR_DIR_PATH_SSD "chk_ijc.data")		// スレッドフォーマットチェック用ファイル名　


#endif

#define THRD_FileName_NOFSCK	(".no_fsck")

#define THRD_CRYPT_KEYSIZE		32					// 暗号化キーサイズ
#define THRD_ENCRYPT_DEVICENAME ("cryptvol1")		// 暗号化デバイス名称.

#define THRD_CRYPT_SHELLCOMLEN	170					// cryptsetupに渡すシェルコマンド長.
#define THRD_ENCRYPT_PASSLEN	20					// 暗号化パスワード長.

// 起動シーケンス変更 2012.04.26 start.
#define THRD_APPM_CONFIG_PATH		("/km/work_fw/apf")
#define THRD_APPM_RAM_PATH			("/ramdisk/apf")
#define THRD_APPM_DIR_NAME			("appm")
#define THRD_APPM_PREINST_CONFIG	("APFC_APPM_PreInstallConfig")
#define THRD_APPM_INST_CONFIG		("APFC_APPM_InstallConfig")
// 起動シーケンス変更 2012.04.26 end.

// 2013/02/19 ADS横山 CQ.378292 Act_63200 MS-EFS-16 SSD利用対応.
#define THRD_EMP_SWAP_PATH			("/ata0c/emp")

// 2014.07.03 Sky_yamada 制御構成変更No39 廃棄時消去レポート対応.
// 2018/10/30 Sky合屋    【Sparrow】[AR1065825]eMMCのパスからμSDのパスに修正.
#define THRD_EMMC_ModelDataPath		"/sys/block/kmsda/device/name"		// μSD型番取得ファイルパス.
#define THRD_EMMC_SerialDataPath	"/sys/block/kmsda/device/serial"	// μSDシリアルNoファイルパス.
#define THRD_EMMC_ModelSize			48	// eMMC型番サイズ.
#define THRD_EMMC_SerialSize		32	// eMMCシリアルNoサイズ.

//	2014.09.18 Ogura	HDDミラーリング対応.
#define	THRD_MirrorringBackupFile	"/sdir/MirroringBackup"		//	HDDミラーリングバックアップファイル.

#define THRD_KEYMATERIAL_RETRY		3

#ifdef SIMULATOR
#define THRD_TEMPIMAGE_ARRAY_FILE	"/root/array-%08X.dat"		// 画像一時ファイル
#endif // SIMULATOR.

int hddtrans_mkdir( const char* path );

#define THRD_CHK_FORMATID_MAX		(3)	// 2016.01.28 A.Gouya	[ZeusS_ZX0] CQ.524243 フォーマットID確認用最大リトライ回数.
#define THRD_DmcrypCmdtRetryMax		(10)	// 2017/06/05 Sky井柳	[IT5_4.0] AR1023986 暗号化デバイス作成コマンド実行のリトライ回数.
#define THRD_MaxClsNumRetryMax		(10)	// 2019/08/26 Sky鳥居	[EagleZ+] OP_BTS-11139対応 リトライ回数. OP_BTS-22077対応 3→10に変更.

// 2019/11/02 Sky TAS000000001236対応.
#define THRD_PartitionManageSize_MicroSD (16)
#define THRD_PartitionManageSize_SSD     (32)
#define THRD_PartitionManageSize_SIM     (63)

// 2020/01/31 Sky FW検証モードで使用するハッシュ値をスレッド/BPS領域にバックアップする対応 ->.
#define THRD_OSSINDEXHASH_UPDATE_COUNT	4		// OSSINDEXHash値の種類数.
size_t                st_OSSIndexSPIFlashOffset[THRD_OSSINDEXHASH_UPDATE_COUNT] = { 0xF2F, 0xF4F, 0xF6F, 0xF8F, };
TYPS_OSSIndexHashType e_OSSIndexHashType[THRD_OSSINDEXHASH_UPDATE_COUNT]        = { TYPS_OSSIndexHashType_UPL, TYPS_OSSIndexHashType_OSL, TYPS_OSSIndexHashType_CHR, TYPS_OSSIndexHashType_NDJ, };

#define THRD_SPI_OFFSET_INDEXHASH		(0xFAF)	// スレッド/BSP領域に保存するINDEXHashのオフセット.
#define THRD_SPI_OFFSET_BOOTROMHASH		(0xFCF)	// スレッド/BSP領域に保存するBootRomHashのオフセット.
#define THRD_OSS_Index_Hash_Size		32		// OSSINDEXハッシュ値 (32byte).
#define THRD_INDEX_Hash_Size			32		// INDEXハッシュ値 (32byte).
#define THRD_BOOT_Hash_Size				32		// BootROMハッシュ値 (32byte).
// 2020/01/31 Sky FW検証モードで使用するハッシュ値をスレッド/BPS領域にバックアップする対応 <-.

// 2021/06/16 ads河井.
#define THRD_GETPW_SIZE				(1024)									// getpwnam_r用バッファサイズ.NSS_BUFLEN_PASSWDと同値.
#define THRD_CtrlFile				"/boot/MfpAccessCtrlFile"				// 一般ユーザーアクセス変換管理ファイル.
#define THRD_CtrlTmpFile			"/boot/MfpAccessCtrlFile.tmp"			// 一時ファイル（一般ユーザーアクセス変換管理ファイル）.
#define THRD_MfpAccessTargetList	"/km/mfpaccesscng/MfpAccessTargetList"	// 一般ユーザーアクセス変換対象リストファイル.
#define THRD_MfpAccessNoRecurse	(0)											// 再帰なし
#define THRD_MfpAccessRecurseIn	(1)											// 再帰あり（targetpathを含む）
#define THRD_MfpAccessRecurseEx	(2)											// 再帰あり（targetpathを除く）
#define THRD_MfpAccessCmd_Replace	"sed -i 's/#*unix_socket_permissions\\s=\\s[0-9]*/unix_socket_permissions = 0700/' %s"
static const int32_t THRD_MFPACCESS_CMD = (21+1023+6+3+1);

/*================================================================================================
	外部参照定義  07/03/19 morikawa整理　
================================================================================================*/
#ifdef TORNADO		// VxWorksで有効
extern char* pt_ALLOC_HDDBufferArea;		/* １２８ＫＢｙｔｅ ＨＤＤ用バッファエリア				*/
// 04/07/08 morikawa warning対策 
// 18/07/20 Sky松井 [DenebMLK][AR.1053817]ラインカード起動フラグ設定関数追加.
extern "C" {
#ifndef TORNADO
extern  STATUS chkdsk(char *, u_int, u_int );
#endif	// TORNADO.
extern  void HddRWCheck( int32_t mode );
extern void checkLineModeInTar();
}
#endif	// #ifdef TORNADO
extern Bool setWarpInfo_ChgMFPData( Bool b_value );

extern Bool bl_endSleepHDDInit;
extern ushort us_debug_dspArrayInfo;

//2020/12/14 Tsukiji OP_BTS-29664 FWエラーが発生する問題対応(暫定)
void BackupSoftSW( uchar uc_SWvalue );
Bool RestoreSoftSW( uchar *uc_SWvalue );

/*================================================================================================
	変数定義  07/03/19 morikawa整理　
			  10/03/03 yashiro Gaia問題点リストNo286(CQ324193) : セマフォID dosImageMuteSem 関連処理の削除.
================================================================================================*/
#ifdef TORNADO
SEM_ID dosImageMuteSem;	// 01/10/05 morikawa
#else
// 07/08/17 morikawa 以前の機種ではHDD転送を遅くするためにデバッグ用にデフォルト1000としていた　
// Donauではマルチタスク対応のため現状このディレイは必要ないのでデフォルトでこのディレイタイマは0とする。　
static uint32_t	ul_ImageDelayTimer = 0;
ushort dosImageMuteSem = 0;	// セマフォの代わりを示す変数
#endif

#ifdef DEF_SupportSSD			// 090406 m.koshimo CQ.316761 SSD標準化対応（SSDアクセス制御）.
SEM_ID dosImageMuteSSDSem;		// SSD用セマフォ.
#endif // DEF_SupportSSD.

ST_MsgHDDBackupRestoreReq	st_MsgHDDBackupRestoreReq;
// 2016.05.23 【MAv4】RQ-505[バックアップ機能強化対応] Start.
ST_MsgHDDBackupRestoreCallbackFunc st_MsgHDDBackupRestoreCallbackFunc;
// 2016.05.23 【MAv4】RQ-505[バックアップ機能強化対応] End.

/*================================================================================================
	デバッグ用変数  07/03/19 morikawa整理　
================================================================================================*/
ushort us_debug_dummyHDD = 0;	// debug CP_HDDTrans::startHDDEraseData()で使用　

#if defined(TORNADO)	// 実機
ushort us_debug_HDDdispparm = 0;
#else
ushort us_debug_HDDdispparm = 0;	// 07/08/17 morikawa SimWin時はデフォルトでログを出さないようにする　
#endif	// #if defined(TORNADO)	

// debugF 14,1で有効にする　	
ushort us_debug_HDDTime = 0;	// 転送時間表示debug表示
// debugF 19,1で有効にするがtheLog.cppには入っているがTHRC_vxWorks.cppには入っていない　　	
extern ushort us_debug_hdderase ;	// 一時データ上書き削除 
// MEM_Func.cppのdebugMemCFで有効にする　
extern ushort us_debug_MemCF ;	// メモリチェックファンクションデバッグログ制御
// スリープ中にdosFSのアクセスがあった場合に異常ログを出すためのスリープ中判断フラグ
uchar uc_Indicator_HDDSleep = 0 ;	// スリープ中インジケータ 2005/10/07 added by t.sasaki 
// THRF_HddDebugChangeFsckLevel()で設定する.
uchar uc_debug_ChgFsckLevel = 0;	// 詳細版fsck実行有無フラグ(0:簡易版(Default),1:詳細版) 2010.05.17 soota Add.

// バッファはセクタサイズ単位で確保すること
// 管理Fat+アレイFat+チェックサムをアレイサイズで切り上げて転送するため、
// 旧の配列の大きさ（DEF_CLUSTER_INFO_SIZE）から領域を拡大する　
int32_t debug_FatTrans = 0;	// デバッグ用に追加　

// 時間計測用.
// 転送時間の最大値で更新される　
uint32_t ul_debug_Time_startHDDTrans = 0 ;		// HDD転送処理(startHDDTrans())最大処理時間.
uint32_t ul_debug_Time_startHDDEraseData = 0 ;	// HDD一時データ上書き消去(startHDDEraseData())最大処理時間.

extern uchar uc_umount2_retryCount_init;
extern uchar uc_umount2_retryCount_format;

//2013/08/06 伊藤史.
FILE *fp_debugHDDimage = NULL;
FILE *fp_debugHDDinfo = NULL;
extern ushort us_debug_HDDcapture;

/*Str【対応 :D030対応】2020.03.27 Tsukiji */
uint32_t 	ul_BackupMaxCluster1st = 0;						// 1stバックアップ値(最大クラスタ数).
uint32_t 	ul_BackupMaxCluster2nd = 0;						// 2ndバックアップ値(最大クラスタ数).
/*End【対応 :D030対応】2020.03.27 Tsukiji */

/*================================================================================================
	ログ出力用 define定義
================================================================================================*/
/* 要求コマンド一覧 */
#define		HDD_REQ_READ					(0x00000001)	/* 読み込み要求 */
#define		HDD_REQ_WRITE					(0x00000002)	/* 書き込み要求 */

/*================================================================================================
	パスワード関連定義
================================================================================================*/
#define		DEF_USER_PASSWORD_SUFFIX	"UQYCNFEURVA1"						// ユーザパスワードSUFFIX(12文字)
									  // 01234567890123456789012345678901
#define		DEF_USER_PASSWORD_SUFFIX_LEN	12
// 08/04/18 CQ.302140  kurachi 装置消去時間の定義をDonau_250GBとして修正（暫定）.
// 日立250GBでの実測値92分から1％あたりのフォーマット時間を算出.
// 式：5652480(ms) ÷ 100 ＝56524.8(ms), 切り上げで56525(ms).
// 【課題】新たなHDDが入荷されたら、再度測定して装置消去時間を確認する.
#define		DEF_FORMAT_PERCENT_TIME		56525L					// 250GBで1%あたりのフォーマット時間.

/*================================================================================================
	一時データ上書き消去用定義 2005/07/15 t.sasaki
================================================================================================*/
// 暗号化基板搭載時の一時データ上書き消去.
// HDD上に書き込まれる消去パターンを任意にするための消去データ作成.
uint32_t ul_ErasePattern_Index = 0 ;
#ifdef SIMULATOR    /* Linux(実機) AESのDMAに用いるため、使えない。また、Donauの場合、以下のバッファで直接HDDにwriteしないため、alignment不要 honda */
	uchar	uc_ErasePattern_00[DEF_SECTOR_SIZE];	// 消去パターン 00h
	uchar	uc_ErasePattern_FF[DEF_SECTOR_SIZE];	// 消去パターン FFh
	uchar	uc_ErasePattern_61[DEF_SECTOR_SIZE];	// 消去パターン 61h
#endif    /* #ifdef SIMULATOR */

ST_ErasePatternTbl st_ErasePatternTbl [] = {
#ifdef SIMULATOR
		{&uc_ErasePattern_00[0], 0x00, 0, sizeof(uc_ErasePattern_00), },
		{&uc_ErasePattern_FF[0], 0xFF, 0, sizeof(uc_ErasePattern_FF), },
		{&uc_ErasePattern_61[0], 0x61, 0, sizeof(uc_ErasePattern_61), }
#else    /* Linux(実機) puc_PtnBufメンバについては、CP_HDDTrans::connection()で初期化する honda */
		{NULL, 0x00, 0, DEF_SECTOR_SIZE},
		{NULL, 0xFF, 0, DEF_SECTOR_SIZE},
		{NULL, 0x61, 0, DEF_SECTOR_SIZE}
#endif    /* #ifdef SIMULATOR */
} ;

#define	DEF_ERASE_PATTERNNUM	3	// 上書き消去パターン数.

ST_ErasePatternTbl* getErasePatternTbl( uchar uc_Ptn )
{
	ST_ErasePatternTbl* pst = &st_ErasePatternTbl[0];
	ST_ErasePatternTbl* pRtn = NULL ;
	uint32_t ul_Index = 0;
	for ( ul_Index = 0; ul_Index < DEF_ERASE_PATTERNNUM; ul_Index++ ) {
		if ( pst->uc_PtnCode == uc_Ptn ) {
			pRtn = pst ;
			break ;
		}
		pst++ ;
	}
	return pRtn ;
}

// UV-01385 ファームウェア更新失敗時のロールバック機能対応.
#define THRD_FileName_UpdateFwTar   "UpdateFWPath"                      // 更新FWのファイルパス保存ファイル名.
#define THRD_FilePath_RollBackFw    "/ata01/FW_ROLLBACK/BackupFW.rbk"   // ロールバックファイルパス.
#define THRD_CardVersion_Size       32                                  // バックアップファイルのカードバージョンサイズ.
#define THRD_CardVersion_Offset     -32                                 // バックアップファイルに埋め込まれたカードバージョンの開始オフセット.

#if !defined(DEF_MinervaPreEmu) && !defined(DEF_ZeusRevCus)
#define PART_TYPE_PRIMARY    1
#define PART_TYPE_EXTENDED   2
#define PART_TYPE_LOGICAL    3
#define PART_TYPE_END        -1

#define PART_FSTYPE_FS       1
#define PART_FSTYPE_FAT      2
#define PART_FSTYPE_RAW      3
// 2010/3/5 takayama SSD対応. 
#ifdef DEF_SupportSSD
#define PART_FSTYPE_RAW_HIB  4	//システムID：Thinkpad hibernation
#define PART_FSTYPE_FAT16  5	//システムID：Thinkpad hibernation
#endif
#define PART_FSTYPE_FS_NF	6	// フォーマットしない領域 2010/12/27 MinervaZeus対応 河村 - NVRAM削減対応.
#define PART_FSTYPE_FS_NF2	7	// NV以外でフォーマットしない領域(ext2).
#define PART_FSTYPE_FS_NF3	8	// NV以外でフォーマットしない領域(ext3).
#define PART_FSTYPE_FS_NF4	9	// NV以外でフォーマットしない領域(ext3).
#define PART_FSTYPE_FS_SSD	10	// ext2:暗号化有り.
#define PART_FSTYPE_FS_SSD3	11	// ext3:暗号化有り 2018/09/21 Sky井柳 【DenebMLK】制御構成変更No78「データ移行」対応.
#define PART_FSTYPE_NONE     -1

#define CAPAB_0			49 
#define CMDS_SUPP_1		83
#define LBA_LSB			100
#define LBA_MID			101
#define LBA_48_MSB		102
#define LBA_64_MSB		103
#define LBA_SUP			0x0200
#define VALID			0xc000
#define VALID_VAL		0x4000
#define SUPPORT_48_BIT	0x0400  

// 2012/05/13 sky_kageyama CQ 364868 add.
//#define THRD_UMOUNT_RETRYMAX 10
#define THRD_UMOUNT_RETRYMAX 50

static char sbuf[1024];
#define FMT_PART_FDISK1		"/sbin/fdisk -ul %s | grep \"^/dev\" | sed 's/  */ /g' | cut -d\" \" -f1,3"
#define FMT_PART_FDISK2		"/sbin/fdisk -u %s"
#define FMT_PART_MKE2FS		"/sbin/mke2fs -j %s"
#define FMT_PART_MKE2FSEXT2	"/sbin/mke2fs %s"
#define FMT_PART_MKFSVFAT	"/sbin/mkfs.vfat %s"
#define FMT_PART_TUNE2FS0	"/sbin/tune2fs -c 0 -i 0 %s"
#define FMT_PART_TUNE2FS1	"/sbin/tune2fs -c 1 -i 0 %s"
#define FMT_PART_FSCK1ST	"/sbin/fsck.%s -a %s"
#define FMT_PART_FSCK2ND	"/sbin/fsck.%s -f -y %s"
//20170613 Y.Tanaka 64bit警告対応
//#define FMT_PART_DMCRYPT1	"/sbin/cryptsetup.sh -c aes -h plain -y -b `/sbin/blockdev --getsize %s` -d %s -s %d -p %ld create cryptvol%d %s"
#define FMT_PART_DMCRYPT1	"/sbin/cryptsetup.sh -c aes -h plain -y -b `/sbin/blockdev --getsize %s` -d %s -s %d -p %"PRId32" create cryptvol%d %s"
#define FMT_PART_DMCRYPT2	"/sbin/cryptsetup.sh -c aes -h plain -y -b `/sbin/blockdev --getsize %s` -d %s -s %d create cryptvol%d %s"
#define FMT_PART_DMCRYPT3	"/sbin/cryptsetup.sh -c aes-cbc-null  -h plain -y -b `/sbin/blockdev --getsize %s` -d %s -s %d create cryptvol%d %s"
#define FMT_PART_DMCRYPT_REMOVE	"/sbin/cryptsetup.sh -h plain remove cryptvol%d"		// 2010/10/18 takayama CQ333996 HDD暗号化時に省電力から復帰しない問題対応.
																						// 2012/09/05 miyamoto CQ371341 暗号化削除時にhashalotをチェックする問題対応.

/* cPP暴露保護対応： 暗号化マクロ */
#define THRD_DmcryptSsd1	"/sbin/cryptsetup.sh -c aes -h plain -y -b 2688 -d %s -s %d create crypt_1 /dev/kmsda1"
#define THRD_Dmcrypt		"/sbin/cryptsetup.sh -c aes -h plain -y -b `/sbin/blockdev --getsize %s` -d %s -s %d create crypt_%d %s"
#define THRD_DmcryptRemove	"/sbin/cryptsetup.sh -h plain remove crypt_%d"
#define THRD_EncryptSsd1	"dd if=/dev/kmsda1 of=/dev/mapper/crypt_1 count=2688 bs=512"
#define THRD_EncryptSsdX	"dd if=/dev/kmsda%d of=/dev/mapper/crypt_%d"
#define THRD_DecryptSsd1	"dd if=/dev/mapper/crypt_1 of=/dev/kmsda1 count=2688 bs=512"
#define THRD_DecryptSsdX	"dd if=/dev/mapper/crypt_%d of=/dev/kmsda%d"

/* cPP暴露保護対応： 暗号化フラグ */
#define THRD_CryptFlgNone		(0x0000)
#define THRD_CryptFlgConvert	(0x0001)	/* 暗号化データコンバート		*/

// 暗号化状態取得コマンド.
#define THRD_CMD_IS_MAINSTR_ENC		"check_sec 0"
#define THRD_IS_MAINSTR_ENC			(1)

// 2019/05/10 Sky井柳  【IT6_1.2MR】[AR.1076986]「暗号化とロックの併用対応」.
#define THRD_Encrypt_KeyDir						"/var/crypt"	// 暗号化キーファイル作成ディレクトリ.
#define THRD_Encrypt_KeyDirRamDiskMountCmd		("mount -n -t tmpfs -o noatime,size=1m tmpfs "THRD_Encrypt_KeyDir)	// 暗号化キーファイル作成RAMディスク作成コマンド.
#define THRD_Encrypt_EncDevMakeCmd				"exec /usr/bin/initencdev Setup Remake"	// 暗号化デバイス作成コマンド.
#define THRD_Encrypt_KeyDirRamDiskUnmountCmd	("umount "THRD_Encrypt_KeyDir)			// 暗号化キーファイル作成RAMディスク解放コマンド.
Bool THRL_makeMainStorageEncDevice(void);

// 10/06/3  takayama GaiaPF HDDレス時にata03をRamDiskで生成する.
#define FMT_PART_RAMDISK	"/bin/mount tmpfs %s -n -t tmpfs -o defaults,noatime,size=%s"

// 2018/10/29 SKY井柳 【Eagle】AR1062347 (追加実装)SMART情報ログ出力対応..
#define THRD_SmartInfoCommand		"nvme smart-log /dev/kmsda"		// SMART情報取得コマンド.

// TCG OPAL.
#define SED_OPAL_REVERT     "sed-opal sed-reverttper %s --password \"\""
#define SED_OPAL_OWNER      "sed-opal sed-ownership %s --password \"\""
#define SED_OPAL_ACTIVATE   "sed-opal sed-activatelsp %s --password \"\" --lr_str 1 --sum"
#define SED_OPAL_SETUP      "sed-opal sed-setuplr %s --lr 1  --password \"\"  --rangeStart 0 --rangeLength %ld --sum"

#define RAMDISK_NAME	"/ata03"
#define RAMDISK_SIZE	104857600

#define THRD_HddSCSICommandRetryMax		300		// HDDデバイスファイル(kmsdb)生成待ちでのSCSIコマンドRetry回数(1回 1秒).

// IISW更新準備処理-OSS単独更新.
// 2019/02/25 SCD2-Sky Kawanishi [SecV4][EagleH]機種レビュー指摘反映 直値指定を見直し.
#define THRD_TEMP_MOUNT_POINT_FOR_DEST		"/mnt"													// コピー先領域用マウントポイント.
#define THRD_TEMP_MOUNT_POINT_FOR_SOURCE	"/mnt2"													// コピー元領域用マウントポイント.
#define THRD_INDEX_FILE_NAME				"INDEX"													// INDEXファイル名.
#define THRD_SOURCE_INDEX_PATH				"/boot/"THRD_INDEX_FILE_NAME								// 作成元INDEXパス.
#define THRD_DEST_INDEX_PATH				THRD_TEMP_MOUNT_POINT_FOR_DEST"/"THRD_INDEX_FILE_NAME	// 作成先INDEXパス.

#define THRD_CMD_COPY_SUBSET_FILES			"exec cp -a "THRD_TEMP_MOUNT_POINT_FOR_SOURCE"/* "THRD_TEMP_MOUNT_POINT_FOR_DEST"/"
#define THRD_CMD_MKDIR_EXP					"mkdir "THRD_TEMP_MOUNT_POINT_FOR_DEST"/exp"
#define THRD_CMD_COPY_SUB_IMG_TO_EXP		"exec cp -a "THRD_TEMP_MOUNT_POINT_FOR_SOURCE"/SUB.img "THRD_TEMP_MOUNT_POINT_FOR_DEST"/exp/."
#define THRD_CMD_COPY_RFS_FILES				"exec cp -a "THRD_TEMP_MOUNT_POINT_FOR_SOURCE"/* "THRD_TEMP_MOUNT_POINT_FOR_DEST"/"
#define THRD_CMD_TAR_XFM_TO_DEST			"exec tar xfm %s --no-same-owner -C "THRD_TEMP_MOUNT_POINT_FOR_DEST

#define THRD_INDEX_TYPE_VER					"@VER"
#define THRD_INDEX_TYPE_DBG					"@DBG"
#define THRD_INDEX_DUMMYLINE_EXP			"@EXP\r\n"
#define THRD_INDEX_DUMMYLINE_FOR_SUBSET		"@SUB=1,ID=000008,NM=000000,OP=1,SUM=00000,IMS=0000000000000000000000000000000000000000000000000000000000000000,FN=exp/SUB.img\r\n"
#define THRD_INDEX_ONE_LINE_LENG			1024 // cf. grityinte.h DEF_ONE_LINE_LENG.

#define DEF_DATA_COUNT	15	// 文字列数.
uchar uc_NonAngo[ DEF_DATA_COUNT ] = "NONANGO";
uchar uc_Angoyusen[ DEF_DATA_COUNT ] = "ANGOYUSEN";
uchar uc_Uwagakiyusen[ DEF_DATA_COUNT ] = "UWAGAKIYUSEN";
uchar uc_ChkFormatHddData[ DEF_DATA_COUNT ];

//lost+foundディレクトリの指定用.
#define THRD_DirName_LostPlusFound 			"lost+found"

// 2010/3/5 takayama SSD対応.
#ifdef DEF_SupportSSD
typedef struct part_layout_ {
    int32_t type;
// Str 2010/09/13 koshimo 8536環境Sim対応.
#ifdef TORNADO
    int64_t end;
    int64_t nsec; /* -1: all */
#else // SIMではlongにする
    int32_t end;
    int32_t nsec; /* -1: all */
#endif
// End 2010/09/13 koshimo 8536環境Sim対応.
    int32_t fstype;
    char devpath[2][32];
    char mntpnt[16];
	int32_t crypt_Flg;
} part_layout; 

// 先頭パーティションとPART_TYPE_LOGICALにはパーティション情報が必要なため、サイズ+63セクタする必要がある.
// また4kセクタ対応でパーティション開始位置を4kbyte(8セクタ)に合わせる必要があるため、先頭パーティションの.
// パーティション情報を64セクタとして計算する.

//【695新アーキFUM】20111111 FBSC渡辺 新アーキ論理フォーマット対応 start.
#ifdef DEF_Soc1200
// 17/10/13 Yoneda     S800 ストレージ構成変更対応.
// SD+SSD構成でのSSDパーティションテーブル  hdd_part_info[].
// SSD構成でのSSDパーティションテーブル     sum_part_info[].
// 起動デバイスによりpart_infoに使用するパーティションテーブルのアドレスを設定する。.
// ※SDのパーティションテーブルに従来のssd_part_info[]を使用している為,従来構成のhdd名を使用している。.
static part_layout* part_info = NULL;

static part_layout hdd_part_info[] = {
	{ PART_TYPE_PRIMARY,	0,	81448552,	PART_FSTYPE_RAW,	{ THRM_MOUNT_DEV( THRD_DEVFILE_HDD, 1  ) , THRM_MOUNT_DEV( THRD_DEVFILE_HDD, 1 )},  THRD_DevName_NON	},
	{ PART_TYPE_EXTENDED,	0,	134430960,	PART_FSTYPE_NONE,	{ THRD_DevName_NON						 , THRD_DevName_NON 					},  THRD_DevName_NON	},
	{ PART_TYPE_PRIMARY,	0,	268281000,	PART_FSTYPE_RAW,	{ THRM_MOUNT_DEV( THRD_DEVFILE_HDD, 3  ) , THRM_MOUNT_DEV( THRD_DEVFILE_HDD, 3 )},  THRD_DevName_NON	},
	{ PART_TYPE_PRIMARY,	0,	4194304,	PART_FSTYPE_FS,		{ THRM_MOUNT_DEV( THRD_DEVFILE_HDD, 4  ) , THRM_MOUNT_DEV( THRD_DEVFILE_ENC, 4 )},  THRD_DevName_Hdd10	},	// km/work_app
	{ PART_TYPE_LOGICAL,	0,	97417872,	PART_FSTYPE_FS,		{ THRM_MOUNT_DEV( THRD_DEVFILE_HDD, 5  ) , THRM_MOUNT_DEV( THRD_DEVFILE_ENC, 5 )},  THRD_DevName_Hdd01	},
	{ PART_TYPE_LOGICAL,	0,	9703784,	PART_FSTYPE_FS,		{ THRM_MOUNT_DEV( THRD_DEVFILE_HDD, 6  ) , THRM_MOUNT_DEV( THRD_DEVFILE_ENC, 6 )},  THRD_DevName_Hdd03	},
	{ PART_TYPE_LOGICAL,	0,	20082824,	PART_FSTYPE_FS,		{ THRM_MOUNT_DEV( THRD_DEVFILE_HDD, 7  ) , THRM_MOUNT_DEV( THRD_DEVFILE_ENC, 7 )},  THRD_DevName_Hdd04	},
// CQ511217 C-D010問題 常時暗号化領域(kmsdb8)を平文に戻す対応.
#ifdef DEF_Zeus5BkCus
	{ PART_TYPE_LOGICAL,	0,	2097248,	PART_FSTYPE_FS,		{ THRM_MOUNT_DEV( THRD_DEVFILE_ENC, 8  ) , THRM_MOUNT_DEV( THRD_DEVFILE_ENC, 8 )},  THRD_DevName_Hdd11	},	// hdd_fw
#else
	{ PART_TYPE_LOGICAL,	0,	2097248,	PART_FSTYPE_FS,		{ THRM_MOUNT_DEV( THRD_DEVFILE_HDD, 8  ) , THRM_MOUNT_DEV( THRD_DEVFILE_ENC, 8 )},  THRD_DevName_Hdd11	},	// hdd_fw
#endif	// DEF_Zeus5BkCus.
	{ PART_TYPE_LOGICAL,	0,	277288,		PART_FSTYPE_FS,		{ THRM_MOUNT_DEV( THRD_DEVFILE_HDD, 9  ) , THRM_MOUNT_DEV( THRD_DEVFILE_ENC, 9 )},  THRD_DevName_Hdd05	},	// IWS Management
	{ PART_TYPE_LOGICAL,	0,	2218032,	PART_FSTYPE_FS,		{ THRM_MOUNT_DEV( THRD_DEVFILE_HDD, 10 ) , THRM_MOUNT_DEV( THRD_DEVFILE_ENC, 10)},  THRD_DevName_Hdd06	},	// IWS User
	{ PART_TYPE_LOGICAL,	0,	-1,			PART_FSTYPE_FS,		{ THRM_MOUNT_DEV( THRD_DEVFILE_HDD, 11 ) , THRM_MOUNT_DEV( THRD_DEVFILE_ENC, 11)},  THRD_DevName_Hdd07	},	// Browser
	{ PART_TYPE_END,		0,	0,			PART_FSTYPE_NONE,	{"",										""									},	""					},
};


static part_layout sum_part_info[] = {
    { PART_TYPE_PRIMARY,	0,	861320,		PART_FSTYPE_RAW,	{ THRM_MOUNT_DEV( THRD_DEVFILE_SSD, 1  ) , THRM_MOUNT_DEV( THRD_DEVFILE_SSD, 1 )}, THRD_DevName_NON		},// -
    { PART_TYPE_PRIMARY,	0,	307200,		PART_FSTYPE_FAT16,	{ THRM_MOUNT_DEV( THRD_DEVFILE_SSD, 2  ) , THRM_MOUNT_DEV( THRD_DEVFILE_SSD, 2 )}, THRD_DevName_NON		},// -
    { PART_TYPE_PRIMARY,	0,	1024000,	PART_FSTYPE_RAW_HIB,{ THRM_MOUNT_DEV( THRD_DEVFILE_SSD, 3  ) , THRM_MOUNT_DEV( THRD_DEVFILE_SSD, 3 )}, THRD_DevName_NON		},// -
//    { PART_TYPE_EXTENDED,	0,	-1,			PART_FSTYPE_NONE,	{ THRD_DevName_NON						 , THRD_DevName_NON 				 	}, THRD_DevName_NON		},// -
    { PART_TYPE_EXTENDED,	0,	490439000,	PART_FSTYPE_NONE,	{ THRD_DevName_NON						 , THRD_DevName_NON 				 	}, THRD_DevName_NON		},// -
    { PART_TYPE_LOGICAL,	0,	113368,		PART_FSTYPE_FAT16,	{ THRM_MOUNT_DEV( THRD_DEVFILE_SSD, 5  ) , THRM_MOUNT_DEV( THRD_DEVFILE_SSD, 5 )}, THRD_DevName_NON		},// /boot
    { PART_TYPE_LOGICAL,	0,	49168,		PART_FSTYPE_FS_NF3,	{ THRM_MOUNT_DEV( THRD_DEVFILE_SSD, 6  ) , THRM_MOUNT_DEV( THRD_DEVFILE_SSD, 6 )}, THRD_DevName_Ssd_SDIR	},// /sdir
    { PART_TYPE_LOGICAL,	0,	200000,		PART_FSTYPE_RAW,	{ THRM_MOUNT_DEV( THRD_DEVFILE_SSD, 7  ) , THRM_MOUNT_DEV( THRD_DEVFILE_SSD, 7 )}, THRD_DevName_NON		},// /
    { PART_TYPE_LOGICAL,	0,	2072,		PART_FSTYPE_FS_NF2,	{ THRM_MOUNT_DEV( THRD_DEVFILE_SSD, 8  ) , THRM_MOUNT_DEV( THRD_DEVFILE_SSD, 8 )}, THRD_DevName_Ssd_USR	},// /km/app/usr
    { PART_TYPE_LOGICAL,	0,	34840,		PART_FSTYPE_FS_NF3,	{ THRM_MOUNT_DEV( THRD_DEVFILE_SSD, 9  ) , THRM_MOUNT_DEV( THRD_DEVFILE_SSD, 9 )}, THRD_DevName_Ssd_FW	},// /km/work_fw
    { PART_TYPE_LOGICAL,	0,	14352,		PART_FSTYPE_FS_NF,	{ THRM_MOUNT_DEV( THRD_DEVFILE_SSD, 10 ) , THRM_MOUNT_DEV( THRD_DEVFILE_SSD, 10)}, THRD_DevName_NVDData	},// /ata0b
    { PART_TYPE_LOGICAL,	0,	61456,		PART_FSTYPE_RAW,	{ THRM_MOUNT_DEV( THRD_DEVFILE_SSD, 11 ) , THRM_MOUNT_DEV( THRD_DEVFILE_SSD, 11)}, THRD_DevName_NON		},// /res
    { PART_TYPE_LOGICAL,	0,	614416,		PART_FSTYPE_RAW,	{ THRM_MOUNT_DEV( THRD_DEVFILE_SSD, 12 ) , THRM_MOUNT_DEV( THRD_DEVFILE_SSD, 12)}, THRD_DevName_NON		},// /km
#ifndef FUM_LINUX
    { PART_TYPE_LOGICAL,	0,	20496,		PART_FSTYPE_FS_SSD,	{ THRM_MOUNT_DEV( THRD_DEVFILE_SSD, 13 ) , THRM_MOUNT_DEV( THRD_DEVFILE_ENC, 13)}, THRD_DevName_Ssd0c	},// /ata0c
    { PART_TYPE_LOGICAL,	0,	62480,		PART_FSTYPE_FS_SSD,	{ THRM_MOUNT_DEV( THRD_DEVFILE_SSD, 14 ) , THRM_MOUNT_DEV( THRD_DEVFILE_ENC, 14)}, THRD_DevName_Ssd_Ram1},// /ram1
    { PART_TYPE_LOGICAL,	0,	757776,		PART_FSTYPE_FS_SSD,	{ THRM_MOUNT_DEV( THRD_DEVFILE_SSD, 15 ) , THRM_MOUNT_DEV( THRD_DEVFILE_ENC, 15)}, THRD_DevName_Ssd0a	},// /ata0a
    { PART_TYPE_LOGICAL,	0,	20496,		PART_FSTYPE_FS_SSD,	{ THRM_MOUNT_DEV( THRD_DEVFILE_SSD, 16 ) , THRM_MOUNT_DEV( THRD_DEVFILE_ENC, 16)}, THRD_DevName_Ssd0d	},// /ata0d.
    { PART_TYPE_LOGICAL,	0,	20496,		PART_FSTYPE_FS_SSD,	{ THRM_MOUNT_DEV( THRD_DEVFILE_SSD, 17 ) , THRM_MOUNT_DEV( THRD_DEVFILE_ENC, 17)}, THRD_DevName_Ssd0e	},// /ata0e.
    { PART_TYPE_LOGICAL,	0,	51216,		PART_FSTYPE_FS_SSD,	{ THRM_MOUNT_DEV( THRD_DEVFILE_SSD, 18 ) , THRM_MOUNT_DEV( THRD_DEVFILE_ENC, 18)}, THRD_DevName_Ssd0f	},// /ata0f.
    { PART_TYPE_LOGICAL,	0,	20496,		PART_FSTYPE_FS_SSD,	{ THRM_MOUNT_DEV( THRD_DEVFILE_SSD, 19 ) , THRM_MOUNT_DEV( THRD_DEVFILE_ENC, 19)}, THRD_DevName_Ssd0g	},// /ata0g.
    { PART_TYPE_LOGICAL,	0,	20496,		PART_FSTYPE_FS_SSD,	{ THRM_MOUNT_DEV( THRD_DEVFILE_SSD, 20 ) , THRM_MOUNT_DEV( THRD_DEVFILE_ENC, 20)}, THRD_DevName_Ssd0h	},// /ata0h.
    { PART_TYPE_LOGICAL,	0,	20496,		PART_FSTYPE_FS_SSD,	{ THRM_MOUNT_DEV( THRD_DEVFILE_SSD, 21 ) , THRM_MOUNT_DEV( THRD_DEVFILE_ENC, 21)}, THRD_DevName_Ssd0i	},// /ata0i.
#else
    { PART_TYPE_LOGICAL,	0,	20496,		PART_FSTYPE_FS,		{ THRM_MOUNT_DEV( THRD_DEVFILE_SSD, 13 ) , THRM_MOUNT_DEV( THRD_DEVFILE_SSD, 13)}, THRD_DevName_Ssd0c	},// /ata0c
    { PART_TYPE_LOGICAL,	0,	62480,		PART_FSTYPE_FS,		{ THRM_MOUNT_DEV( THRD_DEVFILE_SSD, 14 ) , THRM_MOUNT_DEV( THRD_DEVFILE_SSD, 14)}, THRD_DevName_Ssd_Ram1},// /ram1
    { PART_TYPE_LOGICAL,	0,	757776,		PART_FSTYPE_FS,		{ THRM_MOUNT_DEV( THRD_DEVFILE_SSD, 15 ) , THRM_MOUNT_DEV( THRD_DEVFILE_SSD, 15)}, THRD_DevName_Ssd0a	},// /ata0a
    { PART_TYPE_LOGICAL,	0,	20496,		PART_FSTYPE_FS,		{ THRM_MOUNT_DEV( THRD_DEVFILE_SSD, 16 ) , THRM_MOUNT_DEV( THRD_DEVFILE_SSD, 16)}, THRD_DevName_Ssd0d	},// /ata0d.
    { PART_TYPE_LOGICAL,	0,	20496,		PART_FSTYPE_FS,		{ THRM_MOUNT_DEV( THRD_DEVFILE_SSD, 17 ) , THRM_MOUNT_DEV( THRD_DEVFILE_SSD, 17)}, THRD_DevName_Ssd0e	},// /ata0e.
    { PART_TYPE_LOGICAL,	0,	51216,		PART_FSTYPE_FS,		{ THRM_MOUNT_DEV( THRD_DEVFILE_SSD, 18 ) , THRM_MOUNT_DEV( THRD_DEVFILE_SSD, 18)}, THRD_DevName_Ssd0f	},// /ata0f.
    { PART_TYPE_LOGICAL,	0,	20496,		PART_FSTYPE_FS,		{ THRM_MOUNT_DEV( THRD_DEVFILE_SSD, 19 ) , THRM_MOUNT_DEV( THRD_DEVFILE_SSD, 19)}, THRD_DevName_Ssd0g	},// /ata0g.
    { PART_TYPE_LOGICAL,	0,	20496,		PART_FSTYPE_FS,		{ THRM_MOUNT_DEV( THRD_DEVFILE_SSD, 20 ) , THRM_MOUNT_DEV( THRD_DEVFILE_SSD, 20)}, THRD_DevName_Ssd0h	},// /ata0h.
    { PART_TYPE_LOGICAL,	0,	20496,		PART_FSTYPE_FS,		{ THRM_MOUNT_DEV( THRD_DEVFILE_SSD, 21 ) , THRM_MOUNT_DEV( THRD_DEVFILE_SSD, 21)}, THRD_DevName_Ssd0i	},// /ata0i.
#endif
    { PART_TYPE_LOGICAL,	0,	81448552,	PART_FSTYPE_RAW,	{ THRM_MOUNT_DEV( THRD_DEVFILE_SSD, 22 ) , THRM_MOUNT_DEV( THRD_DEVFILE_SSD, 22)},  THRD_DevName_NON	},
    { PART_TYPE_LOGICAL,	0,	97417872,	PART_FSTYPE_FS,		{ THRM_MOUNT_DEV( THRD_DEVFILE_SSD, 23 ) , THRM_MOUNT_DEV( THRD_DEVFILE_ENC, 23)},  THRD_DevName_Hdd01	},
    { PART_TYPE_LOGICAL,	0,	9703784,	PART_FSTYPE_FS,		{ THRM_MOUNT_DEV( THRD_DEVFILE_SSD, 24 ) , THRM_MOUNT_DEV( THRD_DEVFILE_ENC, 24)},  THRD_DevName_Hdd03	},
    { PART_TYPE_LOGICAL,	0,	20082824,	PART_FSTYPE_FS,		{ THRM_MOUNT_DEV( THRD_DEVFILE_SSD, 25 ) , THRM_MOUNT_DEV( THRD_DEVFILE_ENC, 25)},  THRD_DevName_Hdd04	},
#ifdef DEF_Zeus5BkCus
    { PART_TYPE_LOGICAL,	0,	2097248,	PART_FSTYPE_FS,		{ THRM_MOUNT_DEV( THRD_DEVFILE_ENC, 26 ) , THRM_MOUNT_DEV( THRD_DEVFILE_ENC, 26)},  THRD_DevName_Hdd11	},	// hdd_fw
#else
    { PART_TYPE_LOGICAL,	0,	2097248,	PART_FSTYPE_FS,		{ THRM_MOUNT_DEV( THRD_DEVFILE_SSD, 26 ) , THRM_MOUNT_DEV( THRD_DEVFILE_ENC, 26)},  THRD_DevName_Hdd11	},	// hdd_fw
#endif	// DEF_Zeus5BkCus.
    { PART_TYPE_LOGICAL,	0,	277288,		PART_FSTYPE_FS,		{ THRM_MOUNT_DEV( THRD_DEVFILE_SSD, 27 ) , THRM_MOUNT_DEV( THRD_DEVFILE_ENC, 27)},  THRD_DevName_Hdd05	},	// IWS Management
    { PART_TYPE_LOGICAL,	0,	2218032,	PART_FSTYPE_FS,		{ THRM_MOUNT_DEV( THRD_DEVFILE_SSD, 28 ) , THRM_MOUNT_DEV( THRD_DEVFILE_ENC, 28)},  THRD_DevName_Hdd06	},	// IWS User
    { PART_TYPE_LOGICAL,	0,	2633912,	PART_FSTYPE_FS,		{ THRM_MOUNT_DEV( THRD_DEVFILE_SSD, 29 ) , THRM_MOUNT_DEV( THRD_DEVFILE_ENC, 29)},  THRD_DevName_Hdd07	},
    { PART_TYPE_LOGICAL,	0,	268281032,	PART_FSTYPE_RAW,	{ THRM_MOUNT_DEV( THRD_DEVFILE_SSD, 30 ) , THRM_MOUNT_DEV( THRD_DEVFILE_SSD, 30)},  THRD_DevName_NON	},
    { PART_TYPE_LOGICAL,	0,	4194336,	PART_FSTYPE_FS,		{ THRM_MOUNT_DEV( THRD_DEVFILE_SSD, 31 ) , THRM_MOUNT_DEV( THRD_DEVFILE_ENC, 31)},  THRD_DevName_Hdd10	},	// km/work_app
    { PART_TYPE_END,		0,	0,			PART_FSTYPE_NONE,	{"",										""									},	""					},
};

#else // DEF_Soc1200
// 2013/07/19 yamada MIF機VerUP対応(MinervaBK 新アーキ4kセクタ対応).
#if defined(DEF_MinervaBKCus) && defined(DEF_MinervaZeusVerUp)
static part_layout part_info[] = {
    { PART_TYPE_PRIMARY,	0,	81448513,	PART_FSTYPE_RAW,	{ THRM_MOUNT_DEV( THRD_DEVFILE_HDD, 1  ) , THRM_MOUNT_DEV( THRD_DEVFILE_HDD, 1 )},  THRD_DevName_NON	},
    { PART_TYPE_EXTENDED,	0,	138625039,	PART_FSTYPE_NONE,	{ THRD_DevName_NON						 , THRD_DevName_NON 					},  THRD_DevName_NON	},
    { PART_TYPE_PRIMARY,	0,	-1,			PART_FSTYPE_RAW,	{ THRM_MOUNT_DEV( THRD_DEVFILE_HDD, 3  ) , THRM_MOUNT_DEV( THRD_DEVFILE_HDD, 3 )},  THRD_DevName_NON	},
    { PART_TYPE_PRIMARY,	0,	0,			PART_FSTYPE_NONE,	{ THRD_DevName_NON                       , THRD_DevName_NON                     },  THRD_DevName_NON	},
    { PART_TYPE_LOGICAL,	0,	101612144,	PART_FSTYPE_FS,		{ THRM_MOUNT_DEV( THRD_DEVFILE_HDD, 5  ) , THRM_MOUNT_DEV( THRD_DEVFILE_ENC, 5 )},  THRD_DevName_Hdd01	},
    { PART_TYPE_LOGICAL,	0,	9703752,	PART_FSTYPE_FS,		{ THRM_MOUNT_DEV( THRD_DEVFILE_HDD, 6  ) , THRM_MOUNT_DEV( THRD_DEVFILE_ENC, 6 )},  THRD_DevName_Hdd03	},
    { PART_TYPE_LOGICAL,	0,	15888424,	PART_FSTYPE_FS,		{ THRM_MOUNT_DEV( THRD_DEVFILE_HDD, 7  ) , THRM_MOUNT_DEV( THRD_DEVFILE_ENC, 7 )},  THRD_DevName_Hdd04	},
    { PART_TYPE_LOGICAL,	0,	4194368,	PART_FSTYPE_FS,		{ THRM_MOUNT_DEV( THRD_DEVFILE_HDD, 8  ) , THRM_MOUNT_DEV( THRD_DEVFILE_ENC, 8 )},  THRD_DevName_Hdd10	},	// km/work_app
    { PART_TYPE_LOGICAL,	0,	2097216,	PART_FSTYPE_FS,		{ THRM_MOUNT_DEV( THRD_DEVFILE_ENC, 9  ) , THRM_MOUNT_DEV( THRD_DEVFILE_ENC, 9 )},  THRD_DevName_Hdd11	},	// hdd_fw
    { PART_TYPE_LOGICAL,	0,	277256,		PART_FSTYPE_FS,		{ THRM_MOUNT_DEV( THRD_DEVFILE_HDD, 10 ) , THRM_MOUNT_DEV( THRD_DEVFILE_ENC, 10)},  THRD_DevName_Hdd05	},	// IWS Management
    { PART_TYPE_LOGICAL,	0,	2218000,	PART_FSTYPE_FS,		{ THRM_MOUNT_DEV( THRD_DEVFILE_HDD, 11 ) , THRM_MOUNT_DEV( THRD_DEVFILE_ENC, 11)},  THRD_DevName_Hdd06	},	// IWS User
    { PART_TYPE_LOGICAL,	0,	-1,			PART_FSTYPE_FS,		{ THRM_MOUNT_DEV( THRD_DEVFILE_HDD, 12 ) , THRM_MOUNT_DEV( THRD_DEVFILE_ENC, 12)},  THRD_DevName_Hdd07	},	// Browser
    { PART_TYPE_END,		0,	0,			PART_FSTYPE_NONE,	{"",										""									},	""					},
};
#else // defined(DEF_MinervaBKCus) && defined(DEF_MinervaZeusVerUp)
static part_layout part_info[] = {
	{ PART_TYPE_PRIMARY,	0,	81448513,	PART_FSTYPE_RAW,	{ THRM_MOUNT_DEV( THRD_DEVFILE_HDD, 1  ) , THRM_MOUNT_DEV( THRD_DEVFILE_HDD, 1 )},  THRD_DevName_NON	},
	{ PART_TYPE_EXTENDED,	0,	138625034,	PART_FSTYPE_NONE,	{ THRD_DevName_NON						 , THRD_DevName_NON 					},  THRD_DevName_NON	},
	{ PART_TYPE_PRIMARY,	0,	-1,			PART_FSTYPE_RAW,	{ THRM_MOUNT_DEV( THRD_DEVFILE_HDD, 3  ) , THRM_MOUNT_DEV( THRD_DEVFILE_HDD, 3 )},  THRD_DevName_NON	},
	{ PART_TYPE_PRIMARY,	0,	0,			PART_FSTYPE_NONE,	{ THRD_DevName_NON                       , THRD_DevName_NON                     },  THRD_DevName_NON	},
	{ PART_TYPE_LOGICAL,	0,	101612145,	PART_FSTYPE_FS,		{ THRM_MOUNT_DEV( THRD_DEVFILE_HDD, 5  ) , THRM_MOUNT_DEV( THRD_DEVFILE_ENC, 5 )},  THRD_DevName_Hdd01	},
	{ PART_TYPE_LOGICAL,	0,	9703752,	PART_FSTYPE_FS,		{ THRM_MOUNT_DEV( THRD_DEVFILE_HDD, 6  ) , THRM_MOUNT_DEV( THRD_DEVFILE_ENC, 6 )},  THRD_DevName_Hdd03	},
	{ PART_TYPE_LOGICAL,	0,	15888426,	PART_FSTYPE_FS,		{ THRM_MOUNT_DEV( THRD_DEVFILE_HDD, 7  ) , THRM_MOUNT_DEV( THRD_DEVFILE_ENC, 7 )},  THRD_DevName_Hdd04	},
	{ PART_TYPE_LOGICAL,	0,	4194367,	PART_FSTYPE_FS,		{ THRM_MOUNT_DEV( THRD_DEVFILE_HDD, 8  ) , THRM_MOUNT_DEV( THRD_DEVFILE_ENC, 8 )},  THRD_DevName_Hdd10	},	// km/work_app
	{ PART_TYPE_LOGICAL,	0,	2097215,	PART_FSTYPE_FS,		{ THRM_MOUNT_DEV( THRD_DEVFILE_ENC, 9  ) , THRM_MOUNT_DEV( THRD_DEVFILE_ENC, 9 )},  THRD_DevName_Hdd11	},	// hdd_fw
	{ PART_TYPE_LOGICAL,	0,	277250,		PART_FSTYPE_FS,		{ THRM_MOUNT_DEV( THRD_DEVFILE_HDD, 10 ) , THRM_MOUNT_DEV( THRD_DEVFILE_ENC, 10)},  THRD_DevName_Hdd05	},	// IWS Management
	{ PART_TYPE_LOGICAL,	0,	2218000,	PART_FSTYPE_FS,		{ THRM_MOUNT_DEV( THRD_DEVFILE_HDD, 11 ) , THRM_MOUNT_DEV( THRD_DEVFILE_ENC, 11)},  THRD_DevName_Hdd06	},	// IWS User
	{ PART_TYPE_LOGICAL,	0,	-1,			PART_FSTYPE_FS,		{ THRM_MOUNT_DEV( THRD_DEVFILE_HDD, 12 ) , THRM_MOUNT_DEV( THRD_DEVFILE_ENC, 12)},  THRD_DevName_Hdd07	},	// Browser
	{ PART_TYPE_END,		0,	0,			PART_FSTYPE_NONE,	{"",										""									},	""					},
};                              
#endif // defined(DEF_MinervaBKCus) && defined(DEF_MinervaZeusVerUp)
#endif // DEF_Soc1200

static part_layout* ssd_part_info = NULL;	// 2017/02/17 iyanagi 【IT5_4.0】AR1012448 NULL初期化.

#if defined(DEF_DenebMLKCus) || defined(DEF_HeliosMLKCus)    // DenebMLK又は、HeliosMLK.
// 2018/07/07 Sky松井 [DenebMLK] 制御構成変更No69 DnebMLK用のパーティションテーブルの追加.
static part_layout sd_part_info_8GB[] = {	// DenebMLK時μSD(8GB)用 蓄積領域有りパーティションテーブル.
	{ PART_TYPE_PRIMARY,	0,	8258,		PART_FSTYPE_RAW,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 1  ),	THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 1  )},	THRD_DevName_NON		},	// フォーマットID/スナップショット領域/FAT保存領.
	{ PART_TYPE_PRIMARY,	0,	176128,		PART_FSTYPE_FAT,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 2  ),	THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 2  )},	THRD_DevName_NON		},	// subset領域.
	{ PART_TYPE_PRIMARY,	0,	911360,		PART_FSTYPE_RAW_HIB,{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 3  ),	THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 3  )},		THRD_DevName_NON	},	// 通常ワープ領域.
	{ PART_TYPE_EXTENDED,	0,	14194094,	PART_FSTYPE_NONE,	{ THRD_DevName_NON						,	THRD_DevName_NON					},	THRD_DevName_NON		},	// 拡張パーティション.
	{ PART_TYPE_LOGICAL,	0,	41023,		PART_FSTYPE_FAT,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 5  ),	THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 5  )},	THRD_DevName_NON		},	// boot領域(書き込み可能領域) /boot.
	{ PART_TYPE_LOGICAL,	0,	49168,		PART_FSTYPE_FS_NF3,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 6  ),	THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 6  )},	THRD_DevName_Ssd_SDIR	},	// sdir領域 /sdir.
	{ PART_TYPE_LOGICAL,	0,	953407,		PART_FSTYPE_FAT,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 7  ),	THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 7  )},	THRD_DevName_NON		},	// RFS領域.
	{ PART_TYPE_LOGICAL,	0,	10303,		PART_FSTYPE_RAW,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 8  ),	THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 8  )},	THRD_DevName_NON		},	// RES領域 /res.
	{ PART_TYPE_LOGICAL,	0,	1271871,	PART_FSTYPE_RAW,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 9  ),	THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 9  )},		THRD_DevName_NON	},	// FW領域 /km.
	{ PART_TYPE_LOGICAL,	0,	2482296,	PART_FSTYPE_RAW,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 10 ),	THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 10 )},		THRD_DevName_NON	},	// SWAP/IISW/スーパーワープ領域.
	{ PART_TYPE_LOGICAL,	0,	698941,		PART_FSTYPE_RAW,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 11 ),	THRM_MOUNT_DEV(THRD_DEVFILE_ENCSSD, 11 )},	THRD_DevName_NON		},	// Backup領域/HDD蓄積データ.
	{ PART_TYPE_LOGICAL,	0,	163903,		PART_FSTYPE_FS_NF3,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 12 ),	THRM_MOUNT_DEV(THRD_DEVFILE_ENCSSD, 12 )},	THRD_DevName_Ssd0j		},	// データベース領域 /ata0j.
	{ PART_TYPE_LOGICAL,	0,	34833,		PART_FSTYPE_FS_NF3,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 13 ),	THRM_MOUNT_DEV(THRD_DEVFILE_ENCSSD, 13 )},	THRD_DevName_Ssd_FW		},	// アプリケーション領域 /km/work_fw.
	{ PART_TYPE_LOGICAL,	0,	15623,		PART_FSTYPE_FS_NF3,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 14 ),	THRM_MOUNT_DEV(THRD_DEVFILE_ENCSSD, 14 )},	THRD_DevName_NVDData	},	// NVD不揮発領域 /ata0b.
	{ PART_TYPE_LOGICAL,	0,	1397823,	PART_FSTYPE_FS_SSD,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 15 ),	THRM_MOUNT_DEV(THRD_DEVFILE_ENCSSD, 15 )},	THRD_DevName_Ssd0c		},	// Emperor スワップ画像領域 /ata0c.
	{ PART_TYPE_LOGICAL,	0,	62480,		PART_FSTYPE_FS_SSD,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 16 ),	THRM_MOUNT_DEV(THRD_DEVFILE_ENCSSD, 16 )},	THRD_DevName_Ssd_Ram1	},	// Emperor RAMDISK(Font) /ram1.
	{ PART_TYPE_LOGICAL,	0,	5873727,	PART_FSTYPE_FS_SSD,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 17 ),	THRM_MOUNT_DEV(THRD_DEVFILE_ENCSSD, 17 )},	THRD_DevName_Ssd0a		},	// 汎用領域 /ata0a.
	{ PART_TYPE_LOGICAL,	0,	327696,		PART_FSTYPE_FS_SSD,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 18 ),	THRM_MOUNT_DEV(THRD_DEVFILE_ENCSSD, 18 )},	THRD_DevName_Ssd0e		},	// Browser領域 /ata0e.
	{ PART_TYPE_LOGICAL,	0,	720865,		PART_FSTYPE_FS_SSD,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 19 ),	THRM_MOUNT_DEV(THRD_DEVFILE_ENCSSD, 19 )},	THRD_DevName_Ssd0g		},	// IWS データ領域 /ata0g.
	{ PART_TYPE_LOGICAL,	0,	90135,		PART_FSTYPE_FS_SSD,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 20 ),	THRM_MOUNT_DEV(THRD_DEVFILE_ENCSSD, 20 )},	THRD_DevName_Ssd0h		},	// IWS 汎用領域 /ata0h.
	{ PART_TYPE_END,		0,	0, 			PART_FSTYPE_NONE,	{ ""									,	""									},	""						}	// 終端.
};
#else // defined(DEF_DenebMLKCus) || defined(DEF_HeliosMLKCus).
// 2017/09/25 Sky松井 [Eagle] 制御構成変更No.XX パーティション構成変更.
// 2018/08/29 Sky井柳 【Eagle】制御構成変更No99 MicroSD暗号化対応  暗号化デバイス名指定.
// 2018/10/12 Sky新大阪 [AR.1061634]【制御_No.99】/ata07と/ata0eのfstypeを変更(NF2 -> FS,FS_SSD).
// 2018/12/13 Sky井柳 【Eagle】AR.1068816 /ata0eのfstypeを変更(FS -> FS_SSD).
static part_layout sd_part_info_8GB[] = {	// μSD(8GB)用パーティションテーブル.
	{ PART_TYPE_PRIMARY,	0,	66,			PART_FSTYPE_RAW,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 1  ),	THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 1 )},	THRD_DevName_NON		},	// フォーマットID/スナップショット領域/FAT保存領.
	{ PART_TYPE_PRIMARY,	0,	176128,		PART_FSTYPE_FAT,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 2  ),	THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 2 )},	THRD_DevName_NON		},	// subset領域.
	{ PART_TYPE_PRIMARY,	0,	911360,		PART_FSTYPE_RAW_HIB,{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 3  ),	THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 3 )},	THRD_DevName_NON		},	// 通常ワープ領域.
	{ PART_TYPE_EXTENDED,	0,	14192750,	PART_FSTYPE_NONE,	{ THRD_DevName_NON						,	THRD_DevName_NON					},	THRD_DevName_NON		},	// 拡張パーティション.
	{ PART_TYPE_LOGICAL,	0,	41023,		PART_FSTYPE_FAT,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 5  ),	THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 5 )},	THRD_DevName_NON		},	// boot領域(書き込み可能領域) /boot.
	{ PART_TYPE_LOGICAL,	0,	49168,		PART_FSTYPE_FS_NF3,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 6  ),	THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 6 )},	THRD_DevName_Ssd_SDIR	},	// sdir領域 /sdir.
	{ PART_TYPE_LOGICAL,	0,	953407,		PART_FSTYPE_FAT,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 7  ),	THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 7 )},	THRD_DevName_NON		},	// RFS領域.
	{ PART_TYPE_LOGICAL,	0,	10303,		PART_FSTYPE_RAW,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 8  ),	THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 8 )},	THRD_DevName_NON		},	// RES領域 /res.
	{ PART_TYPE_LOGICAL,	0,	1271871,	PART_FSTYPE_RAW,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 9  ),	THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 9 )},	THRD_DevName_NON		},	// FW領域 /km.
	{ PART_TYPE_LOGICAL,	0,	3416376,	PART_FSTYPE_RAW,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 10 ),	THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 10 )},	THRD_DevName_NON		},	// SWAP/IISW/スーパーワープ領域.
	{ PART_TYPE_LOGICAL,	0,	4157,		PART_FSTYPE_RAW,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 11 ),	THRM_MOUNT_DEV(THRD_DEVFILE_ENCSSD, 11 )},	THRD_DevName_NON		},	// Backup領域/HDD蓄積データ.
	{ PART_TYPE_LOGICAL,	0,	163903,		PART_FSTYPE_FS_NF3,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 12 ),	THRM_MOUNT_DEV(THRD_DEVFILE_ENCSSD, 12 )},	THRD_DevName_Ssd0j		},	// データベース領域 /ata0j.
	{ PART_TYPE_LOGICAL,	0,	34833,		PART_FSTYPE_FS_NF3,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 13 ),	THRM_MOUNT_DEV(THRD_DEVFILE_ENCSSD, 13 )},	THRD_DevName_Ssd_FW		},	// アプリケーション領域 /km/work_fw.
	{ PART_TYPE_LOGICAL,	0,	15623,		PART_FSTYPE_FS_NF3,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 14 ),	THRM_MOUNT_DEV(THRD_DEVFILE_ENCSSD, 14 )},	THRD_DevName_NVDData	},	// NVD不揮発領域 /ata0b.
	{ PART_TYPE_LOGICAL,	0,	1464383,	PART_FSTYPE_FS_SSD,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 15 ),	THRM_MOUNT_DEV(THRD_DEVFILE_ENCSSD, 15 )},	THRD_DevName_Ssd0c		},	// Emperor スワップ画像領域 /ata0c.
	{ PART_TYPE_LOGICAL,	0,	62480,		PART_FSTYPE_FS_SSD,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 16 ),	THRM_MOUNT_DEV(THRD_DEVFILE_ENCSSD, 16 )},	THRD_DevName_Ssd_Ram1	},	// Emperor RAMDISK(Font) /ram1.
	{ PART_TYPE_LOGICAL,	0,	5566527,	PART_FSTYPE_FS_SSD,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 17 ),	THRM_MOUNT_DEV(THRD_DEVFILE_ENCSSD, 17 )},	THRD_DevName_Ssd0a		},	// 汎用領域 /ata0a.
	{ PART_TYPE_LOGICAL,	0,	327696,		PART_FSTYPE_FS_SSD,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 18 ),	THRM_MOUNT_DEV(THRD_DEVFILE_ENCSSD, 18 )},	THRD_DevName_Ssd0e		},	// Browser領域 /ata0e.
	{ PART_TYPE_LOGICAL,	0,	720865,		PART_FSTYPE_FS_SSD,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 19 ),	THRM_MOUNT_DEV(THRD_DEVFILE_ENCSSD, 19 )},	THRD_DevName_Ssd0g		},	// IWS データ領域 /ata0g.
	{ PART_TYPE_LOGICAL,	0,	90135,		PART_FSTYPE_FS_SSD,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 20 ),	THRM_MOUNT_DEV(THRD_DEVFILE_ENCSSD, 20 )},	THRD_DevName_Ssd0h		},	// IWS 汎用領域 /ata0h.
	{ PART_TYPE_END,		0,	0, 			PART_FSTYPE_NONE,	{ ""									,	""									},	""						}	// 終端.
};
#endif // defined(DEF_DenebMLKCus) || defined(DEF_HeliosMLKCus).
// 2017/09/25 Sky松井 [Eagle] 制御構成変更No.XX パーティション構成変更.
// 2018/10/09 Sky新大阪 [AR.1061634]【制御_No.99】/ata01と/ata0aのパーティションサイズ仕様変更.
// 2018/10/12 Sky新大阪 [AR.1061634]【制御_No.99】/ata07と/ata0eのfstypeを変更(NF2 -> FS,FS_SSD).
// 2019/03/12 Sky井柳 【IT6_1.2MR】制御構成変更No10 SSD暗号化対応  暗号化デバイス名指定.
static part_layout ssd_part_info_256GB[] = {	// SSD(256GB)用パーティションテーブル.
	{ PART_TYPE_PRIMARY,	0,	8258,		PART_FSTYPE_RAW,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 1  ),	THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 1  )},		THRD_DevName_NON		},	// フォーマットID/スナップショット領域/FAT保存領.
	{ PART_TYPE_PRIMARY,	0,	176128,		PART_FSTYPE_FAT,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 2  ),	THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 2  )},		THRD_DevName_NON		},	// subset領域.
	{ PART_TYPE_PRIMARY,	0,	911360,		PART_FSTYPE_RAW_HIB,{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 3  ),	THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 3  )},		THRD_DevName_NON		},	// 通常ワープ領域.
	{ PART_TYPE_EXTENDED,	0,	494321662,	PART_FSTYPE_NONE,	{ THRD_DevName_NON,							THRD_DevName_NON					},		THRD_DevName_NON		},	// 拡張パーティション.
	{ PART_TYPE_LOGICAL,	0,	41023,		PART_FSTYPE_FAT,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 5  ),	THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 5  )},		THRD_DevName_NON		},	// boot領域(書き込み可能領域) /boot.
	{ PART_TYPE_LOGICAL,	0,	49168,		PART_FSTYPE_FS_NF3,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 6  ),	THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 6  )},		THRD_DevName_Ssd_SDIR	},	// sdir領域 /sdir.
	{ PART_TYPE_LOGICAL,	0,	953407,		PART_FSTYPE_FAT,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 7  ),	THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 7  )},		THRD_DevName_NON		},	// RFS領域.
	{ PART_TYPE_LOGICAL,	0,	10303,		PART_FSTYPE_RAW,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 8  ),	THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 8  )},		THRD_DevName_NON		},	// RES領域 /res.
	{ PART_TYPE_LOGICAL,	0,	1271871,	PART_FSTYPE_RAW,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 9  ),	THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 9  )},		THRD_DevName_NON		},	// FW領域 /km.
	{ PART_TYPE_LOGICAL,	0,	81457482,	PART_FSTYPE_RAW,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 10 ),	THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 10 )},		THRD_DevName_NON		},	// SWAP/IISW/スーパーワープ領域.
	{ PART_TYPE_LOGICAL,	0,	6979648,	PART_FSTYPE_FS_NF3,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 11 ),	THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 11 )},		THRD_DevName_VS			},	// ウィルススキャン領域 /km_vs.
	{ PART_TYPE_LOGICAL,	0,	268268605,	PART_FSTYPE_RAW,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 12 ),	THRM_MOUNT_DEV(THRD_DEVFILE_ENCSSD, 12 )},	THRD_DevName_NON		},	// Backup領域/HDD蓄積データ.
	{ PART_TYPE_LOGICAL,	0,	1024063,	PART_FSTYPE_FS_NF3,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 13 ),	THRM_MOUNT_DEV(THRD_DEVFILE_ENCSSD, 13 )},	THRD_DevName_ata09		},	// データベース領域 /ata09.
	{ PART_TYPE_LOGICAL,	0,	34833,		PART_FSTYPE_FS_NF3,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 14 ),	THRM_MOUNT_DEV(THRD_DEVFILE_ENCSSD, 14 )},	THRD_DevName_Ssd_FW		},	// アプリケーション領域 /km/work_fw.
	{ PART_TYPE_LOGICAL,	0,	15623,		PART_FSTYPE_FS_NF3,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 15 ),	THRM_MOUNT_DEV(THRD_DEVFILE_ENCSSD, 15 )},	THRD_DevName_ata02		},	// NVD不揮発領域 /ata02.
	{ PART_TYPE_LOGICAL,	0,	9703752,	PART_FSTYPE_FS_SSD,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 16 ),	THRM_MOUNT_DEV(THRD_DEVFILE_ENCSSD, 16 )},	THRD_DevName_Hdd03		},	// Emperor スワップ画像領域 /ata03.
	{ PART_TYPE_LOGICAL,	0,	62480,		PART_FSTYPE_FS_SSD,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 17 ),	THRM_MOUNT_DEV(THRD_DEVFILE_ENCSSD, 17 )},	THRD_DevName_Ssd_Ram1	},	// Emperor RAMDISK(Font) /ram1.
	{ PART_TYPE_LOGICAL,	0,	82014273,	PART_FSTYPE_FS_SSD,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 18 ),	THRM_MOUNT_DEV(THRD_DEVFILE_ENCSSD, 18 )},	THRD_DevName_Hdd01		},	// 汎用領域(/ata01).
	{ PART_TYPE_LOGICAL,	0,	4194367,	PART_FSTYPE_FS_SSD,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 19 ),	THRM_MOUNT_DEV(THRD_DEVFILE_ENCSSD, 19 )},	THRD_DevName_Ssd0a		},	// 汎用領域(/ata0a).
	{ PART_TYPE_LOGICAL,	0,	2633879,	PART_FSTYPE_FS_SSD,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 20 ),	THRM_MOUNT_DEV(THRD_DEVFILE_ENCSSD, 20 )},	THRD_DevName_Hdd07		},	// Browser領域 /ata07.
	{ PART_TYPE_LOGICAL,	0,	4757567,	PART_FSTYPE_FS_SSD,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 21 ),	THRM_MOUNT_DEV(THRD_DEVFILE_ENCSSD, 21 )},	THRD_DevName_Hdd06		},	// IWS データ領域　/ata06.
	{ PART_TYPE_LOGICAL,	0,	8603711,	PART_FSTYPE_FS_SSD,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 22 ),	THRM_MOUNT_DEV(THRD_DEVFILE_ENCSSD, 22 )},	THRD_DevName_ata08		},	// IWS 汎用領域　/ata08.
	{ PART_TYPE_LOGICAL,	0,	20082792,	PART_FSTYPE_FS,		{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 23 ),	THRM_MOUNT_DEV(THRD_DEVFILE_ENCSSD, 23 )},	THRD_DevName_Hdd04		},	// 音声・動画管理領域　/ata04.
	{ PART_TYPE_LOGICAL,	0,	2097216,	PART_FSTYPE_FS,		{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 24 ),	THRM_MOUNT_DEV(THRD_DEVFILE_ENCSSD, 24 )},	THRD_DevName_Hdd11		},	// HDD_FW領域/hdd_fw.
	{ PART_TYPE_LOGICAL,	0,	65599,		PART_FSTYPE_FS_SSD,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 25 ),	THRM_MOUNT_DEV(THRD_DEVFILE_ENCSSD, 25 )},	THRD_DevName_Hdd10		},	// HDD_アプリケーション&汎用領域　/km/work_app.
	{ PART_TYPE_END,		0,	0, 			PART_FSTYPE_NONE,	{ "",										""									},		""						}	// 終端.
};
// 2017/09/25 Sky松井 [Eagle] 制御構成変更No.XX パーティション構成変更.
// 2018/10/12 Sky新大阪 [AR.1061634]【制御_No.99】/ata07と/ata0eのfstypeを変更(NF2 -> FS,FS_SSD).
// 2019/07/12 Sky [RQ-560]1TB_SSD対応により再編集.
static part_layout ssd_part_info_1TB[] = {	// SSD(1TB)用パーティションテーブル.
	{ PART_TYPE_PRIMARY,	0,	41026,		PART_FSTYPE_RAW,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 1  ),	THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 1  )},		THRD_DevName_NON		},	// フォーマットID/スナップショット領域/FAT保存領.
	{ PART_TYPE_PRIMARY,	0,	176128,		PART_FSTYPE_FAT,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 2  ),	THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 2  )},		THRD_DevName_NON		},	// subset領域.
	{ PART_TYPE_PRIMARY,	0,	911360,		PART_FSTYPE_RAW_HIB,{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 3  ),	THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 3  )},		THRD_DevName_NON		},	// 通常ワープ領域.
	{ PART_TYPE_EXTENDED,	0,	1998053758,	PART_FSTYPE_NONE,	{ THRD_DevName_NON,							THRD_DevName_NON					},		THRD_DevName_NON		},	// 拡張パーティション.
	{ PART_TYPE_LOGICAL,	0,	41023,		PART_FSTYPE_FAT,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 5  ),	THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 5  )},		THRD_DevName_NON		},	// boot領域(書き込み可能領域) /boot.
	{ PART_TYPE_LOGICAL,	0,	49168,		PART_FSTYPE_FS_NF3,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 6  ),	THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 6  )},		THRD_DevName_Ssd_SDIR	},	// sdir領域 /sdir.
	{ PART_TYPE_LOGICAL,	0,	953407,		PART_FSTYPE_FAT,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 7  ),	THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 7  )},		THRD_DevName_NON		},	// RFS領域.
	{ PART_TYPE_LOGICAL,	0,	10303,		PART_FSTYPE_RAW,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 8  ),	THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 8  )},		THRD_DevName_NON		},	// RES領域 /res.
	{ PART_TYPE_LOGICAL,	0,	1271871,	PART_FSTYPE_RAW,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 9  ),	THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 9  )},		THRD_DevName_NON		},	// FW領域 /km.
	{ PART_TYPE_LOGICAL,	0,	81490250,	PART_FSTYPE_RAW,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 10 ),	THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 10 )},		THRD_DevName_NON		},	// SWAP/IISW/スーパーワープ領域.
	{ PART_TYPE_LOGICAL,	0,	6979648,	PART_FSTYPE_FS_NF3,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 11 ),	THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 11 )},		THRD_DevName_VS			},	// ウィルススキャン領域 /km_vs.
	{ PART_TYPE_LOGICAL,	0,	1341326397,	PART_FSTYPE_RAW,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 12 ),	THRM_MOUNT_DEV(THRD_DEVFILE_ENCSSD, 12 )},	THRD_DevName_NON		},	// Backup領域/HDD蓄積データ.
	{ PART_TYPE_LOGICAL,	0,	1024063,	PART_FSTYPE_FS_NF3,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 13 ),	THRM_MOUNT_DEV(THRD_DEVFILE_ENCSSD, 13 )},	THRD_DevName_ata09		},	// データベース領域 /ata09.
	{ PART_TYPE_LOGICAL,	0,	34833,		PART_FSTYPE_FS_NF3,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 14 ),	THRM_MOUNT_DEV(THRD_DEVFILE_ENCSSD, 14 )},	THRD_DevName_Ssd_FW		},	// アプリケーション領域 /km/work_fw.
	{ PART_TYPE_LOGICAL,	0,	15623,		PART_FSTYPE_FS_NF3,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 15 ),	THRM_MOUNT_DEV(THRD_DEVFILE_ENCSSD, 15 )},	THRD_DevName_ata02		},	// NVD不揮発領域 /ata02.
	{ PART_TYPE_LOGICAL,	0,	9703752,	PART_FSTYPE_FS_SSD,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 16 ),	THRM_MOUNT_DEV(THRD_DEVFILE_ENCSSD, 16 )},	THRD_DevName_Hdd03		},	// Emperor スワップ画像領域 /ata03.
	{ PART_TYPE_LOGICAL,	0,	62480,		PART_FSTYPE_FS_SSD,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 17 ),	THRM_MOUNT_DEV(THRD_DEVFILE_ENCSSD, 17 )},	THRD_DevName_Ssd_Ram1	},	// Emperor RAMDISK(Font) /ram1.
	{ PART_TYPE_LOGICAL,	0,	503193665,	PART_FSTYPE_FS_SSD,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 18 ),	THRM_MOUNT_DEV(THRD_DEVFILE_ENCSSD, 18 )},	THRD_DevName_Hdd01		},	// 汎用領域(/ata01).
	{ PART_TYPE_LOGICAL,	0,	8388671,	PART_FSTYPE_FS_SSD,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 19 ),	THRM_MOUNT_DEV(THRD_DEVFILE_ENCSSD, 19 )},	THRD_DevName_Ssd0a		},	// 汎用領域(/ata0a).
	{ PART_TYPE_LOGICAL,	0,	2633879,	PART_FSTYPE_FS_SSD,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 20 ),	THRM_MOUNT_DEV(THRD_DEVFILE_ENCSSD, 20 )},	THRD_DevName_Hdd07		},	// Browser領域 /ata07.
	{ PART_TYPE_LOGICAL,	0,	9017407,	PART_FSTYPE_FS_SSD,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 21 ),	THRM_MOUNT_DEV(THRD_DEVFILE_ENCSSD, 21 )},	THRD_DevName_Hdd06		},	// IWS データ領域　/ata06.
	{ PART_TYPE_LOGICAL,	0,	8603711,	PART_FSTYPE_FS_SSD,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 22 ),	THRM_MOUNT_DEV(THRD_DEVFILE_ENCSSD, 22 )},	THRD_DevName_ata08		},	// IWS 汎用領域　/ata08.
	{ PART_TYPE_LOGICAL,	0,	20082792,	PART_FSTYPE_FS,		{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 23 ),	THRM_MOUNT_DEV(THRD_DEVFILE_ENCSSD, 23 )},	THRD_DevName_Hdd04		},	// 音声・動画管理領域　/ata04.
	{ PART_TYPE_LOGICAL,	0,	3105216,	PART_FSTYPE_FS,		{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 24 ),	THRM_MOUNT_DEV(THRD_DEVFILE_ENCSSD, 24 )},	THRD_DevName_Hdd11		},	// HDD_FW領域/hdd_fw.
	{ PART_TYPE_LOGICAL,	0,	65599,		PART_FSTYPE_FS_SSD,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 25 ),	THRM_MOUNT_DEV(THRD_DEVFILE_ENCSSD, 25 )},	THRD_DevName_Hdd10		},	// HDD_アプリケーション&汎用領域　/km/work_app.
	{ PART_TYPE_END,		0,	0, 			PART_FSTYPE_NONE,	{ "",										""									},		""						}	// 終端.
};

#ifdef DEF_A3A4COMM
// 2018/04/13 [Sparrow] パーティション構成の変更対応.
static part_layout sd_part_info_16GB[] =
{
	{PART_TYPE_PRIMARY,		0,	8258,		PART_FSTYPE_RAW,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 1  )	,	THRM_MOUNT_DEV(	THRD_DEVFILE_SSD,    1  )},	THRD_DevName_NON		},	// フォーマットID/スナップショット領域/FAT保存領.
	{PART_TYPE_PRIMARY,		0,	176128,		PART_FSTYPE_FAT,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 2  )	,	THRM_MOUNT_DEV(	THRD_DEVFILE_SSD,    2  )},	THRD_DevName_NON		},	// subset領域.
	{PART_TYPE_PRIMARY,		0,	911360,		PART_FSTYPE_RAW_HIB,{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 3  )	,	THRM_MOUNT_DEV(	THRD_DEVFILE_SSD,    3  )},	THRD_DevName_NON		},	// 通常ワープ領域.
	{PART_TYPE_EXTENDED,	0,	28264621,	PART_FSTYPE_NONE,	{ THRD_DevName_NON						,	THRD_DevName_NON			 			 },	THRD_DevName_NON		},	// 拡張パーティション.
	{PART_TYPE_LOGICAL,		0,	41023,		PART_FSTYPE_FAT,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 5  )	,	THRM_MOUNT_DEV(	THRD_DEVFILE_SSD,    5  )},	THRD_DevName_NON		},	// boot領域(書き込み可能領域) /boot.
	{PART_TYPE_LOGICAL,		0,	49168,		PART_FSTYPE_FS_NF3,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 6  )	,	THRM_MOUNT_DEV(	THRD_DEVFILE_SSD,    6  )},	THRD_DevName_Ssd_SDIR	},	// sdir領域 /sdir.
	{PART_TYPE_LOGICAL,		0,	953407,		PART_FSTYPE_FAT,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 7  )	,	THRM_MOUNT_DEV(	THRD_DEVFILE_SSD,    7  )},	THRD_DevName_NON		},	// RFS領域.
	{PART_TYPE_LOGICAL,		0,	10303,		PART_FSTYPE_RAW,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 8  )	,	THRM_MOUNT_DEV(	THRD_DEVFILE_SSD,    8  )},	THRD_DevName_NON		},	// RES領域 /res.
	{PART_TYPE_LOGICAL,		0,	1271871,	PART_FSTYPE_RAW,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 9  )	,	THRM_MOUNT_DEV(	THRD_DEVFILE_SSD,    9  )},	THRD_DevName_NON		},	// FW領域 /km.
	{PART_TYPE_LOGICAL,		0,	3416376,	PART_FSTYPE_RAW_HIB,{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 10 )	,	THRM_MOUNT_DEV(	THRD_DEVFILE_SSD,    10 )},	THRD_DevName_NON		},	// SWAP/IISW/スーパーワープ領域.
	{PART_TYPE_LOGICAL,		0,	6979647,	PART_FSTYPE_FS_NF3,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 11 )	,	THRM_MOUNT_DEV(	THRD_DEVFILE_SSD,    11 )},	THRD_DevName_VS			},	// ウィルススキャン領域 /km_vs.
	{PART_TYPE_LOGICAL,		0,	4274237,	PART_FSTYPE_RAW,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 12 )	,	THRM_MOUNT_DEV(	THRD_DEVFILE_ENCSSD, 12 )},	THRD_DevName_NON		},	// Backup領域/HDD蓄積データ.
	{PART_TYPE_LOGICAL,		0,	163903,		PART_FSTYPE_FS_NF3,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 13 )	,	THRM_MOUNT_DEV(	THRD_DEVFILE_ENCSSD, 13 )},	THRD_DevName_Ssd0j		},	// データベース領域 /ata0j.
	{PART_TYPE_LOGICAL,		0,	34833,		PART_FSTYPE_FS_NF3,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 14 )	,	THRM_MOUNT_DEV(	THRD_DEVFILE_ENCSSD, 14 )},	THRD_DevName_Ssd_FW		},	// アプリケーション領域 /km/work_fw.
	{PART_TYPE_LOGICAL,		0,	15623,		PART_FSTYPE_FS_NF3,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 15 )	,	THRM_MOUNT_DEV(	THRD_DEVFILE_ENCSSD, 15 )},	THRD_DevName_NVDData	},	// NVD不揮発領域 /ata0b.
	{PART_TYPE_LOGICAL,		0,	1464383,	PART_FSTYPE_FS_SSD,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 16 )	,	THRM_MOUNT_DEV(	THRD_DEVFILE_ENCSSD, 16 )},	THRD_DevName_Ssd0c		},	// Emperor スワップ画像領域 /ata0c.
	{PART_TYPE_LOGICAL,		0,	8388671,	PART_FSTYPE_FS_SSD,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 17 )	,	THRM_MOUNT_DEV(	THRD_DEVFILE_ENCSSD, 17 )},	THRD_DevName_Ssd0a		},	// 汎用領域 /ata0a.
	{PART_TYPE_LOGICAL,		0,	62480,		PART_FSTYPE_FS_SSD,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 18 )	,	THRM_MOUNT_DEV(	THRD_DEVFILE_ENCSSD, 18 )},	THRD_DevName_Ssd_Ram1	},	// Emperor RAMDISK(Font) /ram1.
	{PART_TYPE_LOGICAL,		0,	327696,		PART_FSTYPE_FS_NF2,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 19 )	,	THRM_MOUNT_DEV(	THRD_DEVFILE_ENCSSD, 19	)},	THRD_DevName_Ssd0e		},	// Browser領域 /ata0e.
	{PART_TYPE_LOGICAL,		0,	720865,		PART_FSTYPE_FS_SSD,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 20 )	,	THRM_MOUNT_DEV(	THRD_DEVFILE_ENCSSD, 20	)},	THRD_DevName_Ssd0g		},	// IWS データ領域 /ata0g.
	{PART_TYPE_LOGICAL,		0,	90135,		PART_FSTYPE_FS_SSD,	{ THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 21 )	,	THRM_MOUNT_DEV(	THRD_DEVFILE_ENCSSD, 21 )},	THRD_DevName_Ssd0h		},	// IWS 汎用領域 /ata0h.
	{PART_TYPE_END,			0,	0,			PART_FSTYPE_NONE,	{ ""									,	""										 },	""						}
};
#endif // DEF_A3A4COMM.

// パーティション作成に必要なサイズを取得する関数.
static int64_t THRG_GetPartirionSize( part_layout* pst_PartInfo );

// 使用するパーティションテーブルを取得する関数.
static part_layout* THRG_GetPartitionType();

//【695新アーキFUM】20111111 FBSC渡辺 新アーキ論理フォーマット対応 end.

// 2017/10/25 Sky松井 [Eagle] 制御構成変更No.XX Swap管理情報最大ページ数ストレージサイズ判定用定義.
#define THRD_GET_STOR_ERROR_NUM		0				// 取得失敗値定義.

//  2018/09/21 Sky井柳 【DenebMLK】制御構成変更No78「データ移行」対応.
//  2018/12/04 Sky井柳 【DenebMLK】AR.1069398【IT問題】【制御構成変更_No.78】「μSDの一部がマウントされていない」.
//                       /ata0j、/ata0b、/ata0c、/ata0e、/ata0g、/ata0h 追加.
#define THRD_DataMove_DevicePath		"/dev/kmsdb"						// データ移行用 元データデバイスパス.
#define THRD_DataMove_ExistTypePath		"/sys/block/kmsdb/device/type"		// データ移行用 元データデバイスの確認用パス.
#define THRD_DataMove_MountDir_Boot		"/tmp/boot"			// データ移行用 元ディレクトリ:/boot マウント用.
#define THRD_DataMove_MountDir_Sdir		"/tmp/sdir"			// データ移行用 元ディレクトリ:/sdir マウント用.
#define THRD_DataMove_MountDir_Res		"/tmp/res"			// データ移行用 元ディレクトリ:/res マウント用.
#define THRD_DataMove_MountDir_WorkFw	"/tmp/km/work_fw"	// データ移行用 元ディレクトリ:/km/work_fw マウント用.
#define THRD_DataMove_MountDir_Ram1		"/tmp/ram1"			// データ移行用 元ディレクトリ:/ram1 マウント用.
#define THRD_DataMove_MountDir_Ata0a	"/tmp/ata0a"		// データ移行用 元ディレクトリ:/ata0a マウント用.
#define THRD_DataMove_MountDir_Ata0j	"/tmp/ata0j"		// データ移行用 元ディレクトリ:/ata0j マウント用.
#define THRD_DataMove_MountDir_Ata0b	"/tmp/ata0b"		// データ移行用 元ディレクトリ:/ata0b マウント用.
#define THRD_DataMove_MountDir_Ata0c	"/tmp/ata0c"		// データ移行用 元ディレクトリ:/ata0c マウント用.
#define THRD_DataMove_MountDir_Ata0e	"/tmp/ata0e"		// データ移行用 元ディレクトリ:/ata0e マウント用.
#define THRD_DataMove_MountDir_Ata0g	"/tmp/ata0g"		// データ移行用 元ディレクトリ:/ata0g マウント用.
#define THRD_DataMove_MountDir_Ata0h	"/tmp/ata0h"		// データ移行用 元ディレクトリ:/ata0h マウント用.

// データ移行対象 パーティションテーブル(構造体を流用).
part_layout st_part_info_DataMove[] = {
	{ PART_TYPE_LOGICAL, 0,	0,	PART_FSTYPE_FAT,	 { THRM_MOUNT_DEV(THRD_DataMove_DevicePath, 5  ), THRM_MOUNT_DEV(THRD_DataMove_DevicePath, 5 )	}, THRD_DataMove_MountDir_Boot	},	// boot領域(書き込み可能領域) /boot.
	{ PART_TYPE_LOGICAL, 0,	0,	PART_FSTYPE_FS_NF3,	 { THRM_MOUNT_DEV(THRD_DataMove_DevicePath, 6  ), THRM_MOUNT_DEV(THRD_DataMove_DevicePath, 6 )	}, THRD_DataMove_MountDir_Sdir	},	// sdir領域 /sdir.
	{ PART_TYPE_LOGICAL, 0,	0,	PART_FSTYPE_FS_NF3,	 { THRM_MOUNT_DEV(THRD_DataMove_DevicePath, 8  ), THRM_MOUNT_DEV(THRD_DataMove_DevicePath, 8 )	}, THRD_DataMove_MountDir_Res	},	// RES領域 /res.
	{ PART_TYPE_LOGICAL, 0,	0,	PART_FSTYPE_FS_NF3,	 { THRM_MOUNT_DEV(THRD_DataMove_DevicePath, 12 ), THRM_MOUNT_DEV(THRD_DEVFILE_ENC, 12 )			}, THRD_DataMove_MountDir_Ata0j	},	// データベース領域 /ata0j.
	{ PART_TYPE_LOGICAL, 0,	0,	PART_FSTYPE_FS_SSD3, { THRM_MOUNT_DEV(THRD_DataMove_DevicePath, 13 ), THRM_MOUNT_DEV(THRD_DEVFILE_ENC, 13 )			}, THRD_DataMove_MountDir_WorkFw},	// アプリケーション領域 /km/work_fw.
	{ PART_TYPE_LOGICAL, 0,	0,	PART_FSTYPE_FS_NF3,	 { THRM_MOUNT_DEV(THRD_DataMove_DevicePath, 14 ), THRM_MOUNT_DEV(THRD_DEVFILE_ENC, 14 )			}, THRD_DataMove_MountDir_Ata0b	},	// NVD不揮発領域 /ata0b.
	{ PART_TYPE_LOGICAL, 0,	0,	PART_FSTYPE_FS_SSD,	 { THRM_MOUNT_DEV(THRD_DataMove_DevicePath, 15 ), THRM_MOUNT_DEV(THRD_DEVFILE_ENC, 15 )			}, THRD_DataMove_MountDir_Ata0c	},	// Emperor スワップ画像領域 /ata0c.
	{ PART_TYPE_LOGICAL, 0,	0,	PART_FSTYPE_FS_SSD,	 { THRM_MOUNT_DEV(THRD_DataMove_DevicePath, 16 ), THRM_MOUNT_DEV(THRD_DEVFILE_ENC, 16 )			}, THRD_DataMove_MountDir_Ram1	},	// Emperor RAMDISK(Font) /ram1.
	{ PART_TYPE_LOGICAL, 0,	0,	PART_FSTYPE_FS_SSD,	 { THRM_MOUNT_DEV(THRD_DataMove_DevicePath, 17 ), THRM_MOUNT_DEV(THRD_DEVFILE_ENC, 17 )			}, THRD_DataMove_MountDir_Ata0a	},	// 汎用領域 /ata0a.
	{ PART_TYPE_LOGICAL, 0,	0,	PART_FSTYPE_FS_SSD,	 { THRM_MOUNT_DEV(THRD_DataMove_DevicePath, 18 ), THRM_MOUNT_DEV(THRD_DEVFILE_ENC, 18 )			}, THRD_DataMove_MountDir_Ata0e	},	// Browser領域 /ata0e.
	{ PART_TYPE_LOGICAL, 0,	0,	PART_FSTYPE_FS_SSD,	 { THRM_MOUNT_DEV(THRD_DataMove_DevicePath, 19 ), THRM_MOUNT_DEV(THRD_DEVFILE_ENC, 19 )			}, THRD_DataMove_MountDir_Ata0g	},	// IWS データ領域 /ata0g.
	{ PART_TYPE_LOGICAL, 0,	0,	PART_FSTYPE_FS_SSD,	 { THRM_MOUNT_DEV(THRD_DataMove_DevicePath, 20 ), THRM_MOUNT_DEV(THRD_DEVFILE_ENC, 20 )			}, THRD_DataMove_MountDir_Ata0h	},	// IWS 汎用領域 /ata0h.
	{ PART_TYPE_END,	 0,	0, 	PART_FSTYPE_NONE,	 { ""									,	""													},	""						}	// 終端.
};

#else // DEF_SupportSSD.
static struct part_layout {
    int32_t type;
// Str 2010/09/13 koshimo 8536環境Sim対応.
#ifdef TORNADO
    int64_t end;
    int64_t nsec; /* -1: all */
#else // SIMではlongにする
    int32_t end;
    int32_t nsec; /* -1: all */
#endif
// End 2010/09/13 koshimo 8536環境Sim対応.
    int32_t fstype;
    char devpath[2][32];
    char mntpnt[8];
} part_info[] = {
//【695新アーキFUM】20111111 FBSC渡辺 新アーキ論理フォーマット対応 start.
	{ PART_TYPE_PRIMARY,	0,	81448511,	PART_FSTYPE_RAW,	{ THRM_MOUNT_DEV( THRD_DEVFILE_HDD, 1  ) , THRM_MOUNT_DEV( THRD_DEVFILE_HDD, 1 )},  THRD_DevName_NON	},
	{ PART_TYPE_EXTENDED,	0,	138625024,	PART_FSTYPE_NONE,	{ THRD_DevName_NON						 , THRD_DevName_NON 					},  THRD_DevName_NON	},
	{ PART_TYPE_PRIMARY,	0,	-1,			PART_FSTYPE_RAW,	{ THRM_MOUNT_DEV( THRD_DEVFILE_HDD, 3  ) , THRM_MOUNT_DEV( THRD_DEVFILE_HDD, 3 )},  THRD_DevName_NON	},
	{ PART_TYPE_PRIMARY,	0,	0,			PART_FSTYPE_NONE,	{ THRD_DevName_NON						 , THRD_DevName_NON 					},  THRD_DevName_NON	},
	{ PART_TYPE_LOGICAL,	0,	101612143,	PART_FSTYPE_FS,		{ THRM_MOUNT_DEV( THRD_DEVFILE_HDD, 5  ) , THRM_MOUNT_DEV( THRD_DEVFILE_ENC, 5 )},  THRD_DevName_Hdd01	},
	{ PART_TYPE_LOGICAL,	0,	9703752,	PART_FSTYPE_FS,		{ THRM_MOUNT_DEV( THRD_DEVFILE_HDD, 6  ) , THRM_MOUNT_DEV( THRD_DEVFILE_ENC, 6 )},  THRD_DevName_Hdd03	},
	{ PART_TYPE_LOGICAL,	0,	22180004,	PART_FSTYPE_FS,		{ THRM_MOUNT_DEV( THRD_DEVFILE_HDD, 7  ) , THRM_MOUNT_DEV( THRD_DEVFILE_ENC, 7 )},  THRD_DevName_Hdd04	},
	{ PART_TYPE_LOGICAL,	0,	277250,		PART_FSTYPE_FS,		{ THRM_MOUNT_DEV( THRD_DEVFILE_HDD, 8  ) , THRM_MOUNT_DEV( THRD_DEVFILE_ENC, 8 )},  THRD_DevName_Hdd05	}, // IWS Management
	{ PART_TYPE_LOGICAL,	0,	2218000,	PART_FSTYPE_FS,		{ THRM_MOUNT_DEV( THRD_DEVFILE_HDD, 9  ) , THRM_MOUNT_DEV( THRD_DEVFILE_ENC, 9 )},  THRD_DevName_Hdd06	}, // IWS User
	{ PART_TYPE_LOGICAL,	0,	-1,			PART_FSTYPE_FS,		{ THRM_MOUNT_DEV( THRD_DEVFILE_HDD, 10 ) , THRM_MOUNT_DEV( THRD_DEVFILE_ENC, 10)},  THRD_DevName_Hdd07	}, // Browser
	{ PART_TYPE_END,		0,	0,			PART_FSTYPE_NONE,	{"",										""									},	"" 	},
//【695新アーキFUM】20111111 FBSC渡辺 新アーキ論理フォーマット対応 end.
};
#endif // DEF_SupportSSD.

#else // !defined(DEF_MinervaPreEmu) && !defined(DEF_ZeusRevCus).

#ifdef TORNADO
#define PART_TYPE_PRIMARY    1
#define PART_TYPE_EXTENDED   2
#define PART_TYPE_LOGICAL    3
#define PART_TYPE_END        -1

#define PART_FSTYPE_FS       1
#define PART_FSTYPE_FAT      2
#define PART_FSTYPE_RAW      3
#define PART_FSTYPE_NONE     -1

#define CAPAB_0			49 
#define CMDS_SUPP_1		83
#define LBA_LSB			100
#define LBA_MID			101
#define LBA_48_MSB		102
#define LBA_64_MSB		103
#define LBA_SUP			0x0200
#define VALID			0xc000
#define VALID_VAL		0x4000
#define SUPPORT_48_BIT	0x0400  

#define THRD_UMOUNT_RETRYMAX 10

static char sbuf[1024];
#define FMT_PART_FDISK1		"/sbin/fdisk -ul %s | grep \"^/dev\" | sed 's/  */ /g' | cut -d\" \" -f1,3"
#define FMT_PART_FDISK2		"/sbin/fdisk -u %s"
#define FMT_PART_MKE2FS		"/sbin/mke2fs -j %s"
#define FMT_PART_MKE2FSEXT2	"/sbin/mke2fs %s"
#define FMT_PART_MKFSVFAT	"/sbin/mkfs.vfat %s"
#define FMT_PART_TUNE2FS0	"/sbin/tune2fs -c 0 -i 0 %s"
#define FMT_PART_TUNE2FS1	"/sbin/tune2fs -c 1 -i 0 %s"
#define FMT_PART_FSCK1ST	"/sbin/fsck.%s -a %s"
#define FMT_PART_FSCK2ND	"/sbin/fsck.%s -f -y %s"
//20170613 Y.Tanaka 64bit警告対応
//#define FMT_PART_DMCRYPT1	"/sbin/cryptsetup.sh -c aes -h plain -y -b `/sbin/blockdev --getsize %s` -d %s -s %d -p %ld create cryptvol%d %s"
#define FMT_PART_DMCRYPT1	"/sbin/cryptsetup.sh -c aes -h plain -y -b `/sbin/blockdev --getsize %s` -d %s -s %d -p %"PRId32" create cryptvol%d %s"
#define FMT_PART_DMCRYPT2	"/sbin/cryptsetup.sh -c aes -h plain -y -b `/sbin/blockdev --getsize %s` -d %s -s %d create cryptvol%d %s"

#define RAMDISK_NAME	"/ata03"
#define RAMDISK_SIZE	104857600

static struct part_layout {
    int32_t type;
    int64_t end;
    int64_t nsec; /* -1: all */
    int32_t fstype;
    char devpath[2][32];
    char mntpnt[16];
} part_info[] = {
#if 0	//【695新アーキFUM】20111111 FBSC渡辺 新アーキ論理フォーマット対応 start.
    { PART_TYPE_PRIMARY,	0,	81448511,	PART_FSTYPE_RAW,	{"/dev/kmsda1",	"/dev/kmsda1"},				"" 	},
    { PART_TYPE_EXTENDED,	0,	138625024,	PART_FSTYPE_NONE,	{"",			""},						"" 	},
    { PART_TYPE_PRIMARY,	0,	-1,			PART_FSTYPE_RAW,	{"/dev/kmsda3",	"/dev/kmsda3"},				"" 	},
    { PART_TYPE_PRIMARY,	0,	0,			PART_FSTYPE_NONE,	{"",			""},						"" 	},
    { PART_TYPE_LOGICAL,	0,	101612143,	PART_FSTYPE_FS,		{"/dev/kmsda5",	"/dev/mapper/cryptvol5"},	THRD_DevName_Hdd01	},
    { PART_TYPE_LOGICAL,	0,	9703752,	PART_FSTYPE_FS,		{"/dev/kmsda6",	"/dev/mapper/cryptvol6"},	THRD_DevName_Hdd03	},
    { PART_TYPE_LOGICAL,	0,	22180004,	PART_FSTYPE_FS,		{"/dev/kmsda7",	"/dev/mapper/cryptvol7"},	THRD_DevName_Hdd04	},
    { PART_TYPE_LOGICAL,	0,	277250,		PART_FSTYPE_FS,		{"/dev/kmsda8",	"/dev/mapper/cryptvol8"},	THRD_DevName_Hdd05	},	// IWS Management
    { PART_TYPE_LOGICAL,	0,	2218000,	PART_FSTYPE_FS,		{"/dev/kmsda9",	"/dev/mapper/cryptvol9"},	THRD_DevName_Hdd06	},	// IWS User
    { PART_TYPE_LOGICAL,	0,	-1,			PART_FSTYPE_FS,		{"/dev/kmsda10","/dev/mapper/cryptvol10"},	THRD_DevName_Hdd07	},  // Browser
    { PART_TYPE_END,		0,	0,			PART_FSTYPE_NONE,	{"",			""},						"" 	},
#else	//【695新アーキFUM】20111111 FBSC渡辺 新アーキ論理フォーマット対応.
    { PART_TYPE_PRIMARY,	0,	81448511,	PART_FSTYPE_RAW,	{ THRM_MOUNT_DEV( THRD_DEVFILE_HDD, 1  ) , THRM_MOUNT_DEV( THRD_DEVFILE_HDD, 1 ) }, THRD_DevName_NON	},
	{ PART_TYPE_PRIMARY,	0,	138625024,	PART_FSTYPE_FS,		{ THRM_MOUNT_DEV( THRD_DEVFILE_HDD, 2  ) , THRM_MOUNT_DEV( THRD_DEVFILE_HDD, 2 ) }, THRD_DevName_NON	}, // km.
	{ PART_TYPE_PRIMARY,	0,	-1,			PART_FSTYPE_RAW,	{ THRM_MOUNT_DEV( THRD_DEVFILE_HDD, 3  ) , THRM_MOUNT_DEV( THRD_DEVFILE_HDD, 3 ) }, THRD_DevName_NON	},
	{ PART_TYPE_EXTENDED,	0,	419430400,	PART_FSTYPE_NONE,	{ THRD_DevName_NON						 , THRD_DevName_NON 					},  THRD_DevName_NON	},
	{ PART_TYPE_LOGICAL,	0,	101612143,	PART_FSTYPE_FS,		{ THRM_MOUNT_DEV( THRD_DEVFILE_HDD, 5  ) , THRM_MOUNT_DEV( THRD_DEVFILE_ENC, 5 ) }, THRD_DevName_Hdd01	},
	{ PART_TYPE_LOGICAL,	0,	9703752,	PART_FSTYPE_FS,		{ THRM_MOUNT_DEV( THRD_DEVFILE_HDD, 6  ) , THRM_MOUNT_DEV( THRD_DEVFILE_ENC, 6 ) }, THRD_DevName_Hdd03	},
	{ PART_TYPE_LOGICAL,	0,	22180004,	PART_FSTYPE_FS,		{ THRM_MOUNT_DEV( THRD_DEVFILE_HDD, 7  ) , THRM_MOUNT_DEV( THRD_DEVFILE_ENC, 7 ) }, THRD_DevName_Hdd04	},
	{ PART_TYPE_LOGICAL,	0,	277250,		PART_FSTYPE_FS,		{ THRM_MOUNT_DEV( THRD_DEVFILE_HDD, 8  ) , THRM_MOUNT_DEV( THRD_DEVFILE_ENC, 8 ) }, THRD_DevName_Hdd05	}, // IWS Management.
	{ PART_TYPE_LOGICAL,	0,	2097152,	PART_FSTYPE_FAT,	{ THRM_MOUNT_DEV( THRD_DEVFILE_HDD, 9  ) , THRM_MOUNT_DEV( THRD_DEVFILE_HDD, 9 ) }, THRD_DevName_NON	}, // boot.
	{ PART_TYPE_LOGICAL,	0,	2097152,	PART_FSTYPE_FS,		{ THRM_MOUNT_DEV( THRD_DEVFILE_HDD, 10 ) , THRM_MOUNT_DEV( THRD_DEVFILE_HDD, 10 )}, THRD_DevName_NON	}, // root.
	{ PART_TYPE_LOGICAL,	0,	2097152,	PART_FSTYPE_FS,		{ THRM_MOUNT_DEV( THRD_DEVFILE_HDD, 11 ) , THRM_MOUNT_DEV( THRD_DEVFILE_HDD, 11 )}, THRD_DevName_Hdd08	}, // /km/app/usr.
	{ PART_TYPE_LOGICAL,	0,	2097152,	PART_FSTYPE_FS,		{ THRM_MOUNT_DEV( THRD_DEVFILE_HDD, 12 ) , THRM_MOUNT_DEV( THRD_DEVFILE_HDD, 12 )}, THRD_DevName_Hdd09	}, // /km/work_fw.
//	{ PART_TYPE_LOGICAL,	0,	-1,			PART_FSTYPE_FS,		{ THRM_MOUNT_DEV( THRD_DEVFILE_HDD, 13 ) , THRM_MOUNT_DEV( THRD_DEVFILE_ENC, 13 )}, THRD_DevName_Hdd0a	}, // /ata0a(廃止).
//	{ PART_TYPE_LOGICAL,	0,	-1,			PART_FSTYPE_FS,		{ THRM_MOUNT_DEV( THRD_DEVFILE_HDD, 13 ) , THRM_MOUNT_DEV( THRD_DEVFILE_ENC, 13 )}, THRD_DevName_Hdd10	}, // /km/work_app.
	{ PART_TYPE_LOGICAL,	0,	20971520,	PART_FSTYPE_FS,		{ THRM_MOUNT_DEV( THRD_DEVFILE_HDD, 13 ) , THRM_MOUNT_DEV( THRD_DEVFILE_ENC, 13 )}, THRD_DevName_Hdd10	}, // /km/work_app.
	{ PART_TYPE_LOGICAL,	0,	2218000,	PART_FSTYPE_FS,		{ THRM_MOUNT_DEV( THRD_DEVFILE_HDD, 14 ) , THRM_MOUNT_DEV( THRD_DEVFILE_ENC, 14 )}, THRD_DevName_Hdd06	}, // IWS User.
	{ PART_TYPE_LOGICAL,	0,	2097152,	PART_FSTYPE_FS,		{ THRM_MOUNT_DEV( THRD_DEVFILE_HDD, 15 ) , THRM_MOUNT_DEV( THRD_DEVFILE_ENC, 15 )}, THRD_DevName_Hdd07	}, // Browser.
    { PART_TYPE_END,		0,	0,			PART_FSTYPE_NONE,	{"",			""},						"" 	},
#endif	//【695新アーキFUM】20111111 FBSC渡辺 新アーキ論理フォーマット対応 end.
};

#endif // TORNADO.

#endif // !defined(DEF_MinervaPreEmu) && !defined(DEF_ZeusRevCus).

static Bool verifySignature( const char *psc_FwPath, const char *psc_SigPath );
static Bool loadCertificate( const char *psc_CertPath );

// SATA FWバイナリ名.
#if ( defined DEF_MINERVASBK_CUS )
  #define THRD_SataFwBinName		"MINERVAS_SATA.bin"		// MinervaSBKのみ正しいファイル名を定義.
#elif ( defined DEF_A4_Cus )
  #define THRD_SataFwBinName		"bhC3851_SATA.bin"		// VenusMLKのみ正しいファイル名を定義.
#elif ( defined DEF_ZEUSS_CUS )
  #define THRD_SataFwBinName		"ZEUSS_SATA.bin"		// ZeusSのみ正しいファイル名を定義.
#elif ( defined DEF_HeliosCus )
  #define THRD_SataFwBinName		"HELIOSL23_SATA.bin"	// Heliosのみ正しいファイル名を定義.
#elif ( defined DEF_Zeus5BkCus )
  #define THRD_SataFwBinName		"ZEUS5BK_SATA.bin"		// Poseidon2ndのみ正しいファイル名を定義.
#elif ( defined DEF_ZEUSS_ZX0_CUS )
  #define THRD_SataFwBinName		"ZEUSSZX0_SATA.bin"		// ZeusS_ZX0のみ正しいファイル名を定義.
#elif ( defined DEF_ZEUSSBK12_CUS )
  #define THRD_SataFwBinName		"bh368_SATA.bin"		// ZeusSBK12のみ正しいファイル名を定義.
#elif ( defined DEF_ZEUSSBKX0_CUS )
  #define THRD_SataFwBinName		"bh558_SATA.bin"		// ZeusSBKX0のみ正しいファイル名を定義.
#elif (defined DEF_MINERVAMLK_CUS)							// MinervaMLK追加 Sky鈴木[IT5_4.1] 制御構成変更No.10 コンパイル環境作成.
  #define THRD_SataFwBinName		"bhC759_SATA.bin"   
#elif (defined DEF_ZEUSSBK2ND_12_CUS)						// ZeusSBK2nd 12追加 Sky鈴木[IT5_4.1] 制御構成変更No.10 コンパイル環境作成.
  #define THRD_SataFwBinName		"bh368e_SATA.bin"   
#elif (defined DEF_ZEUSSBK2ND_ZX0_CUS)						// ZeusSBK2nd ZX0追加 Sky鈴木[IT5_4.1] 制御構成変更No.10 コンパイル環境作成.
  #define THRD_SataFwBinName		"bh658e_SATA.bin"
#else
  #define THRD_SataFwBinName		"XXX.bin"				// ZeusS以外は動作させない.
#endif

#define THRD_Path_SataFwBin			"/km/fw/bin/"THRD_SataFwBinName
// MinervaMLK/ZeusSBK2nd Z/X/0追加 Sky鈴木[IT5_4.1] 制御構成変更No.10 コンパイル環境作成.
#if ( defined DEF_MINERVASBK_CUS ) || ( defined DEF_ZEUSS_ZX0_CUS ) || (defined DEF_MINERVAMLK_CUS) || (defined DEF_ZEUSSBK2ND_ZX0_CUS)
#define THRD_Path_WriteSataFwTool	"/km/tools/writeSATAFW_S72"	// SataFW更新ツールのファイルパス.
#elif ( defined DEF_HeliosCus ) || ( defined DEF_Zeus5BkCus ) || ( defined DEF_ZEUSS_CUS ) || ( defined DEF_ZEUSSBK12_CUS ) || ( defined DEF_ZEUSSBKX0_CUS ) || ( defined DEF_A4_Cus ) || ( defined DEF_ZEUSSBK2ND_12_CUS )
#define THRD_Path_WriteSataFwTool	"/km/tools/writeSATAFW_S73"	// SataFW更新ツールのファイルパス.
#else
#define THRD_Path_WriteSataFwTool	"/km/tools/writeSATAFW"	// SataFW更新ツールのファイルパス.
#endif

#define THRD_CMDErrorCode_sda	(uint64_t) 1	//新詳細コード下位4Bytes kmsda
#define THRD_CMDErrorCode_sdb	(uint64_t) 2	//新詳細コード下位4Bytes kmsdb

// 2011/05/30 takayama   CQ344683対応 connection処理変更.
extern Bool bl_endSleepHDDInit;
// AR1012671対応 タスクモニタ自動保存との調停 .
Bool gb_umount_exec = False;

Bool THRG_IsSsdEncrypt(void);

// 2018/09/14 Sky ソリューション分離V2対応（HDD/SSDのマウント＆fsck制御).
Bool THRG_IsMountPoint( const char *target );

/*================================================================================================
	デバッグ用関数定義  07/03/19 morikawa 整理
================================================================================================*/
void setDebugHddSeq( ushort us_State, uint32_t ul_Data ) ;	
void LocalHddDebugEnable(ushort on);
void LocalHddDebugSequence( short );
void LocalLogHddTask();

//2006/09/11 tadamasa Rhein　Moselデバックコマンドマージ
void dspEraseStatus();
void dspHddStatusAll();
extern "C" {
extern void dspStorageBusy();
}
// 01/11/08 morikawa デバッグ時のみ100個の配列を有効とすること
static short st_Tbl[100];
static uint32_t ul_Time[100];
static short ss_TblINdex = 0;
ushort us_DebugEnable = 0;
ushort us_DebugSplitTransEnable = 0;
static void _DebugSequence( short ss_No );
static void _DebugSequence( short ss_No )
{
	// 02/02/15 morikawa 常にログを取るように修正　if( bl_DebugEnable == False ) return;	// Debug指定ありの時のみ実行　
#ifdef TORNADO
	ul_Time[ss_TblINdex] = SYSF_tickGet();
#endif	
	st_Tbl[ss_TblINdex++] = ss_No;		// イベントＮｏ設定
	if( ss_TblINdex >= 100 ) { ss_TblINdex = 0; }
}

#if 0
static short getHddDebugSequenceIndex();
static short getHddDebugSequenceIndex()
{
	return( ss_TblINdex );
}
static uint32_t getHddDebugSequenceTime( short ss_No );
static uint32_t getHddDebugSequenceTime( short ss_No )
{
	return( ul_Time[ss_No] );
}
#endif

uint32_t getHddDebugSequence( short ss_No );
uint32_t getHddDebugSequence( short ss_No )
{
	return( st_Tbl[ss_No] );
}

#if defined(SIMULATOR) 	
// 画像転送SimのBox対応　
static void readWriteArraySim( EM_HDD_RWMode, uintptr_t, uint32_t, uint32_t );
#endif	/* defined(SIMULATOR) 	*/

//	リトライ回数の取得.
static uint32_t THRL_GetSCSICommandRetryCount();

// 2019/10/29 Sky TAS000000001236対応.
//  "/dev/kmsda" + オフセットを"/dev/kmsda#" + 調整後オフセットに変換する.
static char *THRL_GetPartitionPathFromOffset(char *device_path, uint32_t &sector_start, size_t size );

#if defined(DEF_EagleCus) && !defined(DEF_A800) && !defined(DEF_HeliosMLKCus)
static Bool isA800LessFW( const char *psc_FwPath );  // 2021/07/09 A800レス実機への旧ROMのNWDLガード対応.
#endif

/* ===============================================================================================
 [機能] ＨＤＤ 詳細版fsck実行有無フラグの設定処理(for デバッグ用). 
--------------------------------------------------------------------------------------------------
 [引数]		Bool b_isON(True：詳細版fsck実行,False：簡易版fsck実行).
 [戻値] 	無し.
 [詳細] 	.
 [履歴] 	.
  10/05/17  太田	新規作成(CQ327450). 
================================================================================================*/ 
void THRF_HddDebugChangeFsckLevel( Bool b_isON )
{
	uc_debug_ChgFsckLevel = (uchar)(b_isON == True ? 0x01 : 0x00);
}

/* ===============================================================================================
 [機能] ＨＤＤ マスタパスワード(システム固有値 32文字) Sim限定処理. 
--------------------------------------------------------------------------------------------------
 [引数]		無し 
 [戻値] 	マスタパスワード 
 [詳細] 	実機はパスワード隠蔽のため別オブジェクトから参照させる. 
 [履歴] 	
  08/03/10 sunami	新規作成. 
================================================================================================*/ 
#if !defined TORNADO || defined(FUM_LINUX)
char* GetMasterPassword( void )
{
	return	"DAMMYPASSWORD_000000000000000000";	// SimWin用パスワード.
}
#endif

/*================================================================================================
［機能］	デバッグ用
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	無し
［詳細］	
［ｳｫｰｸｽﾙｰ］	未実施
［履歴］	
  05/06/06 t.sasaki 無効にもできるように修正.
================================================================================================*/
void LocalHddDebugEnable(ushort on)
{
#if 1
	us_DebugEnable = on;
#else
	if ( on == 0 ) {
		bl_DebugEnable = False;
		printf( "LogHddTask Debug Log Disable Set\n");
	} else {
		bl_DebugEnable = True;
		printf( "LogHddTask Debug Log Enable Set\n");
	}
#endif
}

/*================================================================================================
［機能］	デバッグ用
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	無し
［詳細］	
［ｳｫｰｸｽﾙｰ］	未実施
［履歴］
　morikawa 01/11/29 デバッグ用に新規追加
================================================================================================*/
void LocalHddDebugSequence( short ss_No )
{
	_DebugSequence( ss_No );
}

/*================================================================================================
［機能］	デバッグ用
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	無し
［詳細］	
［ｳｫｰｸｽﾙｰ］	未実施
［履歴］	
　01/11/30 morikawa HDDデバッグ用チェック追加
　02/02/08 morikawa 表示内容変更　
　02/02/15 morikawa 表示内容変更　
　07/03/19 morikawa Donau初版　とりあえずコメントアウト
================================================================================================*/
void LocalLogHddTask()
{
#if 0
	printf("HDD access time,Calculating with (END-START)*5\n");
	short ss_Index = getHddDebugSequenceIndex();
	for( register short i = 0; i < 100; i++ ) {
		printf("%d : ", i );
		switch( getHddDebugSequence(i) ) {
			case EC_HDDTransMode_HDDtoSDRAM:
				printf("Image Array_Read_Start_Read_Start=%ld(ms/5)\r\n",getHddDebugSequenceTime(i));	break;
			case EC_HDDTransMode_SDRAMtoHDD:	
				printf("Image Array_Write_Start_Write_Start=%ld(ms/5)\r\n",getHddDebugSequenceTime(i));		break;
			case EC_HDDTransMode_HDDtoPCI:
				printf("Manage Info_Read_Start=%ld(ms/5)\r\n",getHddDebugSequenceTime(i));		break;
			case EC_HDDTransMode_PCItoHDD:
				printf("Manage Info_Write_Start=%ld(ms/5)\r\n",getHddDebugSequenceTime(i));		break;
			case (90+EC_HDDTransMode_HDDtoSDRAM):
				printf("Image Array_Read_End_Read_End=%ld(ms/5)\r\n",getHddDebugSequenceTime(i));		break;
			case (90+EC_HDDTransMode_SDRAMtoHDD):
				printf("Image Array_Write_Endy_Write_End=%ld(ms/5)\r\n",getHddDebugSequenceTime(i));		break;
			case (90+EC_HDDTransMode_HDDtoPCI):	
				printf("Manage Info_Read_End=%ld(ms/5)\r\n",getHddDebugSequenceTime(i));		break;
			case (90+EC_HDDTransMode_PCItoHDD):	
				printf("Manage Info_Write_End=%ld(ms/5)\r\n",getHddDebugSequenceTime(i));		break;
			case 5:		printf("format_Start=%ld(ms/5)\r\n",getHddDebugSequenceTime(i));	break;	
			case 6:		printf("DMA18_Start=%ld(ms/5)\r\n",getHddDebugSequenceTime(i));	break;	
			case 7:		printf("DMA18_End=%ld(ms/5)\r\n",getHddDebugSequenceTime(i));	break;	
			case 1000:	printf("OS_Request=%ld(ms/5)\r\n",getHddDebugSequenceTime(i));		break;	
			case 1001:	printf("OS_Write_START=%ld(ms/5)\r\n",getHddDebugSequenceTime(i));		break;	
			case 1002:	printf("OS_Write_END=%ld(ms/5)\r\n",getHddDebugSequenceTime(i));		break;	
			case 1003:	printf("OS_Read_START=%ld(ms/5)\r\n",getHddDebugSequenceTime(i));		break;	
			case 1004:	printf("OS_Read_END=%ld(ms/5)\r\n",getHddDebugSequenceTime(i));		break;	
			case 1005:	printf("OS_DMA18_START=%ld(ms/5)\r\n",getHddDebugSequenceTime(i));		break;	
			case 1006:	printf("OS_DMA18_END=%ld(ms/5)\r\n",getHddDebugSequenceTime(i));		break;	
			case 2000:	printf("FUM_Request=%ld(ms/5)\r\n",getHddDebugSequenceTime(i));		break;	
			case 2005:	printf("FUM_DMA18_START=%ld(ms/5)\r\n",getHddDebugSequenceTime(i));		break;	
			case 2006:	printf("FUM_DMA18_END=%ld(ms/5)\r\n",getHddDebugSequenceTime(i));		break;	
			default:
				if( getHddDebugSequence(i) >= 100 ) {
					printf("format_dmaStartNum=%ld\r\n", getHddDebugSequence(i) - 100);
				} else {
					printf("None\r\n");
				}
				break;
		}
	}
	printf("LastIndex=%d\r\n", ss_Index);
#endif
	
}

#ifdef TORNADO
/*================================================================================================
  ［機能］	デバッグ用（パーティション情報データ表示）
--------------------------------------------------------------------------------------------------
［引数］	device_path:デバイスパス
							SSD："/dev/kmsda"
							HDD："/dev/kmsdb"
［戻値］	無し
［詳細］	
［ｳｫｰｸｽﾙｰ］	未実施
［履歴］2010/3/5 takayama SSD対応. 	
        2017/10/10 yoneda S800 ストレージ構成変更対応.
================================================================================================*/
#ifdef DEF_SupportSSD
void dspPartInfo( char *device_path )
{
	part_layout*	device_info;
	if(strcmp(device_path,theHDDTrans.pc_strDeviceSSD) == 0){
		device_info = &part_info[0];
	} else {
		device_info = &ssd_part_info[0];
	}

    for( int32_t no = 0; device_info[no].type != PART_TYPE_END; no++ ){
        printf( "%"PRId32" %"PRId32" %"PRId64" %"PRId64"  %"PRId32" %s %s %s\n", no, device_info[no].type, device_info[no].end, device_info[no].nsec, device_info[no].fstype,
                device_info[no].devpath[0], device_info[no].devpath[1], device_info[no].mntpnt );
    }

}
#else
void dspPartInfo( void )
{
    for( int32_t no = 0; part_info[no].type != PART_TYPE_END; no++ ){
        printf( "%"PRId32" %"PRId32" %"PRId64" %"PRId64"  %"PRId32" %s %s %s\n", no, part_info[no].type, part_info[no].end, part_info[no].nsec, part_info[no].fstype,
                part_info[no].devpath[0], part_info[no].devpath[1], part_info[no].mntpnt );
    }
}
#endif
#endif

/*================================================================================================
［機能］	デバッグ用
--------------------------------------------------------------------------------------------------
［引数］	有効:1 無効:0
［戻値］	無し
［詳細］	
［ｳｫｰｸｽﾙｰ］	未実施
［履歴］	
  14/05/20 日立JTE岡部	スワップ分割読出対応 新規作成.
================================================================================================*/
void LocalHddDebugSplitTrans(ushort on)
{
	printf("LocalHddDebugSplitTrans [%d]->[%"PRId32"]\n", us_DebugSplitTransEnable, on);
	us_DebugSplitTransEnable = on;	
}

// ================================================================================================
//! <!--機能--> ＨＤＤ転送バッファ、コンストラクタ
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param なし
//! <!--戻値-->
//! \return なし
//! <!--詳細-->
//! バッファデータ初期化関数をコールしてメンバ変数を初期化する　
//! <!--履歴-->
//  07/03/07 morikawa	Donau初版
// ================================================================================================
THRC_HDDBuf::THRC_HDDBuf()
{
	initBufData( 0, 0 );
}
// ================================================================================================
//! <!--機能--> ＨＤＤ転送バッファ、デストラクタ
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param なし
//! <!--戻値-->
//! \return なし
//! <!--詳細-->
//! 
//! <!--履歴-->
//  07/10/29 morikawa	Donau初版
// ================================================================================================
THRC_HDDBuf::~THRC_HDDBuf()
{
}
// ================================================================================================
//! <!--機能--> ＨＤＤ転送バッファ、バッファデータ初期化
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param ulong 先頭バッファのアドレス
//! \param ulong １個のバッファのサイズ
//! <!--戻値-->
//! \return なし
//! <!--詳細-->
//! ＨＤＤ転送バッファのアドレスを初期化する。シーケンス初期化処理もコールする　
//! <!--履歴-->
//  07/03/07 morikawa	Donau初版
// ================================================================================================
void THRC_HDDBuf::initBufData( uintptr_t ul_Adr, uint32_t ul_Size )
{
	// １個のバッファサイズを設定　
	ul_OneBufSize = ul_Size;
	for( uchar i=0; i<THRD_HDDBUF_NUM; ++i ) {
		// バッファの先頭アドレスを設定　
		s_HDDBuf_Info[i].ul_BufAdr = ul_Adr + ul_OneBufSize * i;
	}
	initSeq();	
}
// ================================================================================================
//! <!--機能--> ＨＤＤ転送バッファ、バッファシーケンス初期化
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param なし
//! <!--戻値-->
//! \return なし
//! <!--詳細-->
//! バッファシーケンス初期化を行なう
//! <!--履歴-->
//  07/03/07 morikawa	Donau初版
// ================================================================================================
void THRC_HDDBuf::initSeq()
{
	uc_WriteIndex = 0;
	uc_ReadIndex = 0;
}
// ================================================================================================
//! <!--機能--> ＨＤＤ転送バッファ、書き込みアドレス取得
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param なし
//! <!--戻値-->
//! \return なし
//! <!--詳細-->
//! 
//! <!--履歴-->
//  07/03/07 morikawa Donau初版
//  07/05/14 morikawa 静的解析の不具合修正　
// ================================================================================================
uintptr_t THRC_HDDBuf::getWriteAdr()
{
	uintptr_t ul_Adr = s_HDDBuf_Info[uc_WriteIndex].ul_BufAdr;
	++uc_WriteIndex;
	if( uc_WriteIndex >= THRD_HDDBUF_NUM ) {
		uc_WriteIndex = 0;
	}
	return( ul_Adr );
}
// ================================================================================================
//! <!--機能--> ＨＤＤ転送バッファ、読み出しアドレス取得
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param なし
//! <!--戻値-->
//! \return なし
//! <!--詳細-->
//! 
//! <!--履歴-->
//  07/03/07 morikawa	Donau初版
//  07/05/14 morikawa 静的解析の不具合修正　
// ================================================================================================
uintptr_t THRC_HDDBuf::getReadAdr()
{
	uintptr_t ul_Adr = s_HDDBuf_Info[uc_ReadIndex].ul_BufAdr;
	++uc_ReadIndex;
	if( uc_ReadIndex >= THRD_HDDBUF_NUM ) {
		uc_ReadIndex = 0;
	}
	return( ul_Adr );
}

/*================================================================================================
［機能］	ＨＤＤ情報（ＡＰＩ）オブザーバー クラス インスタンス取得.
--------------------------------------------------------------------------------------------------
//! <!--引数-->
//! \param なし
//! <!--戻値-->
//! \return THRC_HddInfoObserver* オブザーバーのインスタンス.
//! <!--詳細-->
//! オブザーバークラスのインスタンスを取得する.
//! <!--履歴-->
//	2014/09/13 森川(義)	ZeusS 制御構成変更No.4 HDDミラーリング対応.
================================================================================================*/
THRC_HddInfoObserver* THRC_HddInfoObserver::newInstance()
{
	static THRC_HddInfoObserver c_THRC_HddInfoObserver;

	return &c_THRC_HddInfoObserver;
}

/*================================================================================================
［機能］	ＨＤＤ情報（ＡＰＩ）オブザーバー クラス データ変化通知.
--------------------------------------------------------------------------------------------------
//! <!--引数-->
//! \param なし
//! <!--戻値-->
//! \return なし.
//! <!--詳細-->
//! ミラーリング機能がONのときHDD非装着であればミラーリング機能をOFFに設定変更する.
//! <!--履歴-->
//	2014/09/13 森川(義)	ZeusS 制御構成変更No.4 HDDミラーリング対応.
//	2015/06/15 Ogura	getHddExist()の仕様変更対応.
================================================================================================*/
void THRC_HddInfoObserver::dataChanged()
{
	Bool bl_IsMirroring = theHDDTrans.isMirroringHDD();
	if (bl_IsMirroring == False){
		return;											// ミラーリングがOFFのときは終了する.
	}

	// 上位APIのインスタンス生成.
	APIC_SharedSystemDataSetting* pc_HddInfo = APIC_SharedSystemDataSetting::newInstance();
	if (pc_HddInfo == NULL) {
		setDebugLowLogSpeedSeq( 820, 0 );	
		return;
	}

	if (pc_HddInfo->getHddExist() == False) {			// HDD非装着の場合.
		theHDDTrans.setRebuildActive();				// HDDの動作構成を個別（RAID1を解除）へ変更する.
	}
}

// ================================================================================================
//! <!--機能--> ミラーリングバックアップデータの初期化.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! void.
//! \return void.
//! <!--詳細-->
//! HDDミラーリングのバックアップデータの初期化を行う.
//! バックアップデータがeMMC上になければ、バックアップファイル作成を行う.
//! <!--履歴-->
//	2014/09/18	Ogura		新規作成（HDDミラーリング対応）.
//	2014/10/14	Y.Morikawa	CQ.504646 ファイル生成を修正
//! ================================================================================================
void THRC_MirroringDataManage::initMirroringBackupData( void )
{
	int32_t si_File = SYSF_open( THRD_MirrorringBackupFile, O_CREAT | O_RDWR, 0666 );
	if ( -1 == si_File ) {
		setDebugLowLogSpeedSeq( 821, 0 );
		return;
	}
	
	//	不揮発領域からのデータ取得に成功した場合は、初期化フラグを立てて処理終了.
	THRS_MIRRORING_BACKUP_DATA s_BackupData;
	off_t si_Ret = SYSF_read( si_File, (char*)&s_BackupData, sizeof(s_BackupData) );
	SYSF_Close( si_File );
	if ( sizeof(s_BackupData) != si_Ret ) {
		//	不揮発領域からリードに失敗した場合は、初期値を取得する.
		getMirroringBackupData( s_BackupData );
	}
	setMirroringBackupData( s_BackupData );
}

// ================================================================================================
//! <!--機能--> ミラーリングバックアップデータの取得.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! THRS_MIRRORING_BACKUP_DATA& rs_MirrorData : データ格納先.
//! \return void.
//! <!--詳細-->
//! ミラーリングバックアップデータの取得.
//! <!--履歴-->
//	2014/09/18	Ogura		新規作成（HDDミラーリング対応）.
//! ================================================================================================
void THRC_MirroringDataManage::getMirroringBackupData( THRS_MIRRORING_BACKUP_DATA& rs_MirrorData )
{
	memcpy( &rs_MirrorData, &m_s_BackupData, sizeof(THRS_MIRRORING_BACKUP_DATA) );
}

// ================================================================================================
//! <!--機能--> ミラーリングバックアップデータの設定.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! THRS_MIRRORING_BACKUP_DATA& rs_MirrorData : バックアップデータ.
//! \return void.
//! <!--詳細-->
//! ミラーリングバックアップデータをeMMC上に保存する.
//! <!--履歴-->
//	2014/09/18	Ogura		新規作成（HDDミラーリング対応）.
//! ================================================================================================
void THRC_MirroringDataManage::setMirroringBackupData( THRS_MIRRORING_BACKUP_DATA& rs_MirrorData )
{
	m_s_BackupData = rs_MirrorData;
	
	int32_t si_File = SYSF_open( THRD_MirrorringBackupFile, O_RDWR, 0666 );
	if ( -1 == si_File ) {
		setDebugLowLogSpeedSeq( 822, 0 );
		return;
	}
	
	//	不揮発領域からのデータ取得に成功した場合は、初期化フラグを立てて処理終了.
	off_t si_Ret = SYSF_write( si_File, (char*)&m_s_BackupData, sizeof(m_s_BackupData) );
	if ( ERROR == si_Ret ) {
		setDebugLowLogSpeedSeq( 823, 0 );
	}
	
	SYSF_Close( si_File );
	
	sync();
	sync();
	sync();
}

// ================================================================================================
//! <!--機能--> マスタチャネルNoのバックアップ.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! THRE_HddMirroring_Chanel e_Chanel : バックアップデータ.
//! \return void.
//! <!--詳細-->
//! ミラーリングバックアップデータ（マスタチャネル）をeMMC上に保存する.
//! <!--履歴-->
//	2014/09/18	Ogura		新規作成（HDDミラーリング対応）.
//! ================================================================================================
void THRC_MirroringDataManage::setMirroringBackupDataMasterCh( THRE_HddMirroring_Chanel e_Chanel )
{
	m_s_BackupData.e_HddMirroringMasterChanel = e_Chanel;
	setMirroringBackupData( m_s_BackupData );
}

// ================================================================================================
//! <!--機能--> エラービットのバックアップ.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! THRE_MIRRORING_ERROR_DETAIL	e_ErrorBit	: エラービット.
//! Bool						bl_SetFlg	: セットフラグ（True：セット／False：クリア）.
//! \return void.
//! <!--詳細-->
//! ミラーリングバックアップデータ（エラー情報）をeMMC上に保存する.
//! <!--履歴-->
//	2014/09/18	Ogura		新規作成（HDDミラーリング対応）.
//! ================================================================================================
void THRC_MirroringDataManage::setMirroringBackupDataErrorBit( THRE_MIRRORING_ERROR_DETAIL e_ErrorBit, Bool bl_SetFlg )
{
	uint32_t ul_Err = 1 << e_ErrorBit;
	if ( True == bl_SetFlg ) {
		m_s_BackupData.ul_MirroringError |= ul_Err;
	} else {
		m_s_BackupData.ul_MirroringError &= ~ul_Err;
	}
	setMirroringBackupData( m_s_BackupData );
}

// ================================================================================================
//! <!--機能--> エラーが発生しているかの確認.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! THRE_MIRRORING_ERROR_DETAIL	e_ErrorBit	: エラービット.
//! \return Bool True:エラーあり／False:エラーなし.
//! <!--詳細-->
//! 指定のエラーが発生しているかの確認.
//! <!--履歴-->
//	2014/09/18	Ogura		新規作成（HDDミラーリング対応）.
//! ================================================================================================
Bool THRC_MirroringDataManage::isActiveError( THRE_MIRRORING_ERROR_DETAIL e_ErrorBit )
{
	Bool bl_Result = False;
	
	uint32_t ul_Err = 1 << e_ErrorBit;
	if ( 0 != ( ul_Err & m_s_BackupData.ul_MirroringError ) ) {
		bl_Result = True;
	}
	
	return bl_Result;
}

// ================================================================================================
//! <!--機能--> ミラーリング有効設定.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! Bool bl_On	True:有効／False:無効.
//! \return void.
//! <!--詳細-->
//! ミラーリング有効情報をeMMCに保存する.
//! <!--履歴-->
//	2014/09/18	Ogura		新規作成（HDDミラーリング対応）.
//! ================================================================================================
void THRC_MirroringDataManage::setMirroringOn( Bool bl_On )
{
	m_s_BackupData.bl_MirroringOn = bl_On;
	setMirroringBackupData( m_s_BackupData );
}

// ================================================================================================
//! <!--機能--> ミラーリング有効判定.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! void.
//! \return Bool	True:有効／False:無効.
//! <!--詳細-->
//! ミラーリングが有効かどうかを判定する.
//! <!--履歴-->
//	2014/09/18	Ogura		新規作成（HDDミラーリング対応）.
//! ================================================================================================
Bool THRC_MirroringDataManage::isMirroringOn()
{
	return m_s_BackupData.bl_MirroringOn;
}

// ================================================================================================
//! <!--機能--> HDDミラーリングバックアップデータクラスのコンストラクタ.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! .
//! \return.
//! <!--詳細-->
//! 
//! <!--履歴-->
//	2014/09/18	Ogura		新規作成（HDDミラーリング対応）.
//! ================================================================================================
THRS_MIRRORING_BACKUP_DATA::THRS_MIRRORING_BACKUP_DATA()
:bl_MirroringOn(False),
  ul_MirroringError(0),
  e_HddMirroringMasterChanel(THRE_HddMirroring_Chanel_Unknown)
{
}


// ================================================================================================
//! <!--機能--> エラー情報のクリア.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! void.
//! \return void.
//! <!--詳細-->
//! 
//! <!--履歴-->
//	2014/09/18	Ogura		新規作成（HDDミラーリング対応）.
//! ================================================================================================
void THRC_MirroringDataManage::clrMirroringBackupDataError()
{
	m_s_BackupData.ul_MirroringError = 0;
	setMirroringBackupData( m_s_BackupData );
}

/*================================================================================================
［機能］	サブタスク「ＨＤＤデータ転送」
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	無し
［ｳｫｰｸｽﾙｰ］	実施済み：2000/9/7：森川
［履歴］	07/03/06 morikawa Donau初版
			07/05/14 morikawa 静的解析不具合修正　
			08/06/19 kurachi Donau CQ:304895対策.
								論理フォーマット要求に対して、ata04をクリアするかしないかを判断する.
								対応を追加.
			11/11/11 新アーキ論理フォーマット対応(管理者からのフォーマット時 ata04 のみ判断 → テーブルのビットにて判断)
			13/06/17 ADS北村	CQ.381189(Act_66229) EE-SSC_4	HDDレス化対応_SSDへのスワップ対応.
			2014/03/06 toda		Zeus5BK HDDレス対応 HDD装着時にSSDへスワップする暫定対応を削除.
			15/05/09 iyanagi 【Helios】CQ.509096対応 イニシャライズフォーマット指定の追加.
			17/01/24 iyanagi AR1012856【IT5_4.0】クラスタ移動処理追加.
			18/09/05 Ogura	【EagleLow】 totalclearで/ata03、/ata04、/ata0cをフォーマットしない対応.
			18/09/21 Sky井柳 【DenebMLK】制御構成変更No78「データ移行」対応「【要求】データ移行用 元データ領域マウント指示」追加.
================================================================================================*/
extern Bool bl_HDDtestMode;
extern Bool bl_HDDtestExec;
extern void HDDtest();
void taskHDDTrans()
{
	BE_TaskMessage be_TaskMessage = 0;
	uintptr_t ul_Option = 0;
	while( 1 ) {
		receiveMessage( TID_HDDTrans, be_TaskMessage, ul_Option );	// タスク間メッセージの受信

		switch( be_TaskMessage ) {
			case TM_HDDConnectionReq:					// 【要求】初期化（接続）要求
			case TM_REQ_CHECK_HDD_DRDY:					// 【要求】DRDY再チェック要求
				theHDDTrans.connection();
				break;
			case TM_HDDWriteIDReq:						// 【要求】チェックID書込要求 2005/07/15 added by t.sasaki
				theHDDTrans.writeID();
				break;
			case TM_HDDFormatReq:						// 【要求】フォーマット要求
				{
					// (論理|物理)フォーマット開始時はフォーマット中フラグセット 2004/10/05 added by t.sasaki
					uchar	uc_stat = (uchar)NVDC_SystemSetting::newInstance()->getFsStat();
					uc_stat	|= (uchar)TYPD_SystemFsStatFormatting ;
					NVDC_SystemSetting::newInstance()->setFsStat(uc_stat);
				}
				if( ul_Option == FUNC_HD_FORMAT_PHYSICAL ) {
					theHDDTrans.format();	// 物理フォーマット実行
				} else if( ul_Option == FUNC_HD_FORMAT_WASTE) {	// 廃棄時フォーマット追加 05/08/05 Div.12 k-kawa
					theHDDTrans.setAllDataEraseMode( (ushort)(NVDC_SystemSetting::newInstance()->getHddOverWriteErase() + 1) );
					theHDDTrans.format();	// 物理フォーマット実行
				} else {
//【695新アーキFUM】20111111 FBSC渡辺 新アーキ論理フォーマット対応 start.
//					// ◇Start 2008/06/19 kurachi CQ:304895対策（ata04フォーマット切り替え対応）.
//					if( ul_Option == FUNC_HD_FORMAT_EXCEPT_ATA04 ){
//						theHDDTrans.bl_ATA04Format = False;	// ata04（音声・動画管理領域）はクリアしない.
//					}else{
//						theHDDTrans.bl_ATA04Format = True;	// ata04（音声・動画管理領域）もクリアする.
//					}
//					// ◇End 2008/06/19 kurachi CQ:304895対策（ata04フォーマット切り替え対応）.
					//	2018/9/5 Ogura	EagleLow ターミナルからのフォーマットとサービスからのフォーマットを分ける.
					if( ul_Option == FUNC_HD_FORMAT_LOGICAL_ADMIN ){
						theHDDTrans.m_usFormatTargetBit |= THRE_FORMATBIT_ADMIN;
					}else if( ul_Option == FUNC_HD_FORMAT_LOGICAL_INITIALIZE ){
						theHDDTrans.m_usFormatTargetBit |= (THRE_FORMATBIT_ADMIN | THRE_FORMATBIT_INITIALIZE);
					} else if ( ul_Option == FUNC_HD_FORMAT_LOGICAL_TERMINAL ) {
						theHDDTrans.m_usFormatTargetBit |= THRE_FORMATBIT_TERMINAL;
					}else{
						theHDDTrans.m_usFormatTargetBit |= (THRE_FORMATBIT_ADMIN | THRE_FORMATBIT_CE);
					}
//【695新アーキFUM】20111111 FBSC渡辺 新アーキ論理フォーマット対応 end.
					theHDDTrans.formatOS();	// 論理フォーマット実行
				}	
				break;
			case TM_HDDRWReq:							// 【要求】Ｒ／Ｗチェック要求
				theHDDTrans.rwCheckReq();
				break;
			case TM_HDDSetPasswordReq:					// 【要求】パスワード設定要求 03/04/17 tsukiji 【TSS2】
				theHDDTrans.password( ul_Option );
				break;
			case TM_HDDTransportStartReq:				// 【要求】転送開始要求
				if( theHDDTrans.isTransFat( ul_Option ) == True ) {
					theHDDTrans.startHDDTransFat( ul_Option );		// FAT
				} else if( theHDDTrans.isInfoData( ul_Option ) == True ) {
					theHDDTrans.startHDDTransInfo( ul_Option );		// 管理
				} else {
					if ( theHDDTrans.isReqEraseData( ul_Option ) == True ) {
						theHDDTrans.startHDDEraseData( ul_Option ) ;	// 一時データ上書き削除
					} else {
						theHDDTrans.startHDDTrans( ul_Option );		// 画像
					}
				}
				break;
			//タイムアウト
			case TM_TIMEOUT_HDD_DRDY:				//  02/01/08 morikawa HDDの接続不良　
				theHDDTrans.commandTimeOut();
				break;
			case TM_HDDRestoreReq:					// 【要求】リストア
				theHDDTrans.execRestore( ul_Option );
				break;
			case TM_HDDBackupReq:				// 【要求】バックアップ
				theHDDTrans.execBackup( ul_Option );
				break ;

			case TM_MoveClusterBackupReq:	// 【要求】クラスタ移動時バックアップ処理.
				theHDDTrans.setMoveClusterBackupStart( ul_Option );
				break ;
			case TM_MoveClusterRestoreReq:	// 【要求】クラスタ移動時リストア処理.
				theHDDTrans.setMoveClusterRestoreStart( ul_Option );
				break ;
			case TM_MoveClusterEndReq:		// 【要求】クラスタ移動時終了.
				theHDDTrans.setMoveClusterEnd( ul_Option );
				break ;
			case TM_DataMove_Mount_BaseDataReq:	// 【要求】データ移行用 元データ領域マウント指示.
				theHDDTrans.mountDataMoveBaseDataDevice( );
				break ;

			default:	
				THRL_ThAssert( False, "taskHDDTrans() : Unknown Msg! %d, para 0x%x", be_TaskMessage, ul_Option , 0, 0, CFA14_DetailCode0x01BA );
				break;	// For Warning!
		}
	}
}
/*================================================================================================
［クラス］	CB_IDEHard : 制御単位「ＨＤＤ関数」
//###CB_IDEHard
================================================================================================*/
/*================================================================================================
［機能］	コンストラクタ
--------------------------------------------------------------------------------------------------
［引数］	なし
［戻値］	
［詳細］	コンストラクタ処理
［ｳｫｰｸｽﾙｰ］	
［履歴］	17/10/10 Yoneda     S800 ストレージ構成変更対応：THRC_HDDTrans.hから移動
			18/03/30 Sky_松井	【Eagle】制御構成変更No.97【ストレージ構成変更】プログラム転送対応.
			18/11/26 Sky_松井	[DenebMLK]AR.1059379 DEF_SupportSSDの判定削除.
								初期化順を定義順となるようにbl_SwapFatOnlyDisableをm_us_RawAccessTargetの前に移動.
			19/01/28 Sky井柳	【IT6_1.0MR】[制御構成変更_No.8] initOSdriver成功状態の初期化追加.
			19/08/27 Sky		【EagleH】[RQ-560] Sim起動問題対応 Simの時のuc_HDDbuffer領域確保方法変更.
			19/09/11 Sky		[RQ-560]CommonAPIからcheckFatSize()が複数回呼ばれる不具合に対応.
			19/08/26 Sky鳥居	[EagleZ+] OP_BTS-11139対応.
            19/10/29 Sky        TAS000000001236対応.
================================================================================================*/
CP_HDDTrans::CP_HDDTrans()		// コンストラクタ
	:b_EnableEnc( False ),
	 b_IVFixMode( False ),
	 bl_SecurityErrorFlag( False ),
	 bl_CommandExecFlag( False ),
	 bl_LockPasswordFlag( False ),
	 puc_WorkMemAddr( NULL ),
	 us_AllDataEraseMode(0),
	 ul_HDDStatus( HDD_STATUS_OK ),
	 
#ifdef TORNADO
	 e_NvramInfoAreaState(TYPE_HddStatusConnecting),
#else
	 e_NvramInfoAreaState(TYPE_HddStatusFormated),
#endif
	ul_TransFatNum(0),
	bl_writeID_Direct( False ),
//【695新アーキFUM】20111111 FBSC渡辺 新アーキ論理フォーマット対応 start.
//	 bl_ATA04Format( True ),
	 m_usFormatTargetBit(0),
//【695新アーキFUM】20111111 FBSC渡辺 新アーキ論理フォーマット対応 end.
	 em_HDDSeq( EM_HDDSeq_Idle ),
	 ul_FormatSectorNo( 0 ),
	 uc_HDDbuffer( NULL ),
	 pst_MsgSetPassword( NULL ),
	 uc_CurnPos(0), 
	 bl_ReadWriteCheck( False ),
	 bl_ConnectFlag( False ),
	 bl_WarrnigErrorFlag( False ),
	 ul_Percent( 99 ), 
	 bl_CheckFormatError( False ),
	 bl_HddFwAreaError( False ),
	 bl_ExistCheckFile( False ),
	 bl_AccessSuspend( False ),
	 p_RestoreAreaBak(NULL),
	 ul_RestoreSizeBak(0),
	 e_BackupTypeBak(THRE_BackupTypeNum),
	 ul_CulsterSize( 0 ),
	 ul_CulsterinfoSize( 0 ),
	 ul_FatTotalSize( 0 ),
	 ul_SwapStartBlock( 0 ),
	 ul_SwapStartBlock2( 0 ),
	 ul_SwapClsNum( 0 ),
	 ul_SwapBlkNum( 0 ),
	 ul_SwapinfoStartBlock( 0 ),
	 ul_SwapinfoClsNum( 0 ),
	 ul_SwapinfoBlkNum(0),
	 ul_OsStartBlock( 0 ),
	 ul_OsBlockNum2( 0 ),
	 ul_OsStartBlock2( 0 ),
	 ul_OsBlockNum( 0 ),
	 ul_SectorFatSize( 0 ),
	 ul_SectorFatSwapFirst(0),
	 ul_SectorFatSwapSecond(0),
	 ul_SectorFatStoreFirst( 0 ),
	 ul_SectorFatStoreSecond( 0 ),
	 ul_StoreStartBlock( 0 ),
	 ul_StoreClsNum( 0 ),
	 ul_StoreBlkNum( 0 ),
// 2016.05.23 【MAv4】RQ-505[バックアップ機能強化対応] Start.
	 ul_StoreStartBlock_KM( 0 ),
	 ul_StoreClsNum_KM( 0 ),
	 ul_BackupStartBlock( 0 ),
	 ul_BackupClsNum( 0 ),
// 2016.05.23 【MAv4】RQ-505[バックアップ機能強化対応] End.
	 ul_StoreinfoStartBlock( 0 ),
	 ul_StoreinfoClsNum( 0 ),
	 ul_StoreinfoBlkNum( 0 ),
	 ul_StoreinfoEndBlock2( 0 ),
	 ul_FatMaxClsNum( 0 ),
	 ul_StorageMiniSize( 0 ),
	 ul_SysStartBlock( 0 ),
	 ul_EmmcSysStartBlock( 0 ),
	 ul_SysRwChkStartBlock( 0 ),
	 ul_SysRwChkStartBlock2( 0 ),
	 ul_SysRwChkBlockNum( 0 ),
	 ul_StorageEnd( 0 ),
	 ul_StorageEnd2( 0 ),
	 ul_PortNo( 0 ),
	 c_DevicePath( NULL ),
	 bl_SsdNgFlg( 0 ),
	 pc_HddInfo(NULL),
	 b_SsdKeepDataReadingAllFlg( False ),
	 bl_SwapFatOnlyDisable(False),	// SwapFatのみ無効フラグ.
	 m_bl_initOSdriverOK( False ),	// initOSdriver成功状態.
// 2013/06/17 ADS北村	CQ.381189(Act_66229) EE-SSC_4	HDDレス化対応_SSDへのスワップ対応.
	 m_us_RawAccessTarget(DEF_NONINITIALIZE_RAW_ACCESS_TARGET),
	 b_isSSDSwapTrans(False),
// 2014/09/08 ZeusS 制御構成変更No.4 HDDミラーリング対応.
	 m_bl_RebuildRunStatus(False),
	 m_e_HddMirroringMasterChanel(THRE_HddMirroring_Chanel_A),
	 m_uc_ProgressRatio(100),
	 dl_PhysicalHDDSize(0),
	 dl_PhysicalSubDeviceSize(0),
#ifdef DEF_SupportSSD
	 fd_ssd_map(),
#endif // DEF_SupportSSD
	 bl_OSaccessFlag( False ),
#ifndef DEF_SupportSSD
	 fd_hdd(-1),
#else
	 fd_hdd_map(),
#endif // DEF_SupportSSD
	 b_isUmountError(False), 	// 2012/01/20 ft_kondo CQ.360494 umountが発生していた場合、C-FA14を発生させてリブートさせる. 
	 us_FA14DetailCode(0),		// umount error発生時のFA14詳細コード.
	 semID_SataFwAutoRecover(0)	// SATA FW自動復旧セマフォ.
	 ,mb_FatRestoreStartF( False )		// [RQ-560]CommonAPIからcheckFatSize()が複数回呼ばれる不具合に対応.
{
	for( int32_t i=0; i<DEF_MAX_ONEREADWRITE_ARRAYNUM; i++ ) {
		ul_TransArrayAdr[ i ] = 0;
	}
	for( THRE_BackupType e_BackupType = THRE_BackupTypeImage; e_BackupType<THRE_BackupKindNum; e_BackupType++ )
	{
		pul_BackupClusterIdx[e_BackupType] = NULL;
	}

// RQ-560 2019/08/27 RQ-560 Sim起動問題対応 Simの時のuc_HDDbuffer領域確保方法変更 ->.
//#ifdef FUM_LINUX    /* Linux(実機) pt_ALLOC_HDDBufferAreaを使用するため、不要 honda */
//		uc_HDDbuffer = (uchar *)(((uintptr_t)__uc_HDDbuffer + 0x1FF) & ~0x1FF);
//#endif /* FUM_LINUX */
// RQ-560 2019/08/27 RQ-560 Sim起動問題対応 Simの時のuc_HDDbuffer領域確保方法変更 <-.

//	→ 静的解析対応(CIM444012) 2011/05/30 saka.
	memset( &s_MsgAESTransInfo, 0, sizeof(s_MsgAESTransInfo) );
	s_MsgAESTransInfo.e_EncHDDMode = THRE_EncHDDNone;		// 転送モード(Read/Write).
	s_MsgAESTransInfo.b_PassThroughMode = True;				// データスルーモード（暗号化/複合化せずにスルーで転送のみ）　

	// ＨＤＤ転送完了メッセージ.
	st_MsgHDDTransCompleteInfo.ev_TaskID = TID_MAX;				// 要求元タスク.
	st_MsgHDDTransCompleteInfo.pcc_OneInputImage = NULL;		// 入力ページ情報.
	st_MsgHDDTransCompleteInfo.uc_OneThreadId = 0;				//!< 転送識別情報.
	st_MsgHDDTransCompleteInfo.b_AnsStop = False;				// 転送停止応答.
	st_MsgHDDTransCompleteInfo.ul_EndArrayNum = 0;				// 転送完了したアレイ数.

	// ＨＤＤ転送完了メッセージパラメータ取得領域.
	for( int32_t j=0; j<DEF_HDD_TRANS_END_MSG_BUF_NUM; j++ ) {
		st_MsgHDDTransEndPara[j].ev_TaskID = TID_MAX;			// 要求元タスク.
		st_MsgHDDTransEndPara[j].pcc_OneInputImage = NULL;		// 入力ページ情報.
		st_MsgHDDTransEndPara[j].uc_OneThreadId = 0;			//!< 転送識別情報.
		st_MsgHDDTransEndPara[j].b_AnsStop = False;				// 転送停止応答.
		st_MsgHDDTransEndPara[j].ul_EndArrayNum = 0;			// 転送完了したアレイ数.
	}

	st_MsgHDDTransInfo.ev_TaskID = TID_MAX;							// 要求元タスク.
	st_MsgHDDTransInfo.em_TransDataKind = EM_TransDataKind_None;	// 転送データ種別(管理/画像/蓄積/背景/退避).
	st_MsgHDDTransInfo.em_HDD_RWMode = EM_HDD_RWMode_Read;			// 転送モード(Read/Write).
	st_MsgHDDTransInfo.pcc_OneInputImage = NULL;					// 入力ページ情報.
	st_MsgHDDTransInfo.ul_StartClusterNo = 0;						// 開始クラスタ番号.
	st_MsgHDDTransInfo.uc_OneThreadId = 0;							//!< 転送識別情報.
	st_MsgHDDTransInfo.ul_EndArrayNum = 0;							// 転送完了したアレイ数 初期値は０、再開時にこの値の次のアレイから開始する.

	st_MsgHDDEraseCompleteInfo.ev_TaskID = TID_MAX;							// 要求元タスク.
	st_MsgHDDEraseCompleteInfo.em_TransDataKind = EM_TransDataKind_None;	// 転送データ種別(管理/画像/蓄積/背景/退避).
	st_MsgHDDEraseCompleteInfo.ul_StartClusterNo = 0;						// 開始クラスタ番号.

	memset(  s_HDD_DEVICE_INFO, 0, sizeof(s_HDD_DEVICE_INFO) );
	memset( uc_WriteBuff, 0, sizeof(uc_WriteBuff) );
	memset( uc_ReadBuff, 0, sizeof(uc_ReadBuff) );
	memset( uc_PassBuff, 0, sizeof(uc_PassBuff) );
	memset( ul_TransIVNum, 0, sizeof(ul_TransIVNum) );
	memset( m_e_HddMirroringDeviceStatus, 0, sizeof(m_e_HddMirroringDeviceStatus) );
//	← 静的解析対応.
	m_ans_BackupFatChangeCallBackFunc = NULL;	// バックアップ用Fat変更コールバック関数.
	m_ans_BackupRemakeFatEndCallBackFunc = NULL;	// バックアップ用remakeFat終了コールバック関数.
	memset( &m_st_HddRestoreParam, 0, sizeof(m_st_HddRestoreParam) );		// 2017/01/05 ishiga [IT5_4.1][HDDデータバックアップ世代間互換].
		
	// クラスタ移動用.
	m_bl_MvClusterExec = False;				// 実行状態.
	m_ul_MvClusterFromStartSec = 0;		// 移動元クラスタの開始セクタ.
	m_ul_MvClusterFromEndSec = 0;		// 移動元クラスタの終了セクタ.
	m_ul_MvClusterToStartSec = 0;		// 移動先クラスタの開始セクタ.
	m_ul_MvClusterToEndSec = 0;			// 移動先クラスタの終了セクタ.
	m_ul_MvClusterExecFromStartSec = 0;	// 実行:移動元クラスタの開始セクタ.
	m_ul_MvClusterExecToStartSec = 0;	// 実行:移動先クラスタの開始セクタ.
	m_p_WorkArea = NULL;					// ワークエリアポインタ.
	m_ans_MvClusterEndCallBackFunc = NULL;	// 終了コールバック関数.

#ifdef FUM_LINUX
	pc_strDeviceSD = "/dev/sdc";
#else /* #ifdef FUM_LINUX */ 
	pc_strDeviceSD = "/dev/kmsda";
#endif /* #ifdef FUM_LINUX */ 
    // 起動デバイス判断.
	int32_t i_DviceId = THRE_SsdPortNo;//デフォルト：μSD.
	if(THRE_StartupDeviceType_SD == THRF_GetStartupDeviceType() ) {
		// SD.
		e_StartUpDeviceType = THRE_StartupDeviceType_SD;        // 起動デバイスタイプ.
	    part_info = hdd_part_info;                              // パーティションテーブル.
#ifdef FUM_LINUX
#ifdef DEF_64BIT_PF
		pc_strDeviceSSD = "/dev/sdd";
#else
		pc_strDeviceSSD = "/dev/sdb";
#endif
#else /* #ifdef FUM_LINUX */ 
		// AR1060044 対応.
		pc_strDeviceSSD = "/dev/kmsda";
#endif /* #ifdef FUM_LINUX */ 
	}else{
		// SSD.
		i_DviceId = THRE_HddPortNo;
		e_StartUpDeviceType = THRE_StartupDeviceType_SSD;       // 起動デバイスタイプ.
	    part_info = sum_part_info;                              // パーティションテーブル.
#ifdef FUM_LINUX
#ifdef DEF_64BIT_PF
		pc_strDeviceSSD = "/dev/sdc";
#else
		pc_strDeviceSSD = "/dev/sdb";
#endif
#else /* #ifdef FUM_LINUX */ 
		pc_strDeviceSSD = "/dev/kmsda";
#endif /* #ifdef FUM_LINUX */ 
	}
    
    // sysBspHdd.cpp内で使用しているデバイス配列.
    extern ST_HDD_DEVICE_FILE_INFO gst_HddDeviceFileInfo[];
#ifdef FUM_LINUX
	gst_HddDeviceFileInfo[1].puc_devName = pc_strDeviceSSD;
#else /* #ifdef FUM_LINUX */ 
	gst_HddDeviceFileInfo[1].puc_devName = pc_strDeviceSSD;
	gst_HddDeviceFileInfo[2].puc_devName = pc_strDeviceSSD;
#endif /* #ifdef FUM_LINUX */ 

	// 上記デバイス配列の値変更後でないとストレージデバイスサイズの取得はできない.
	// [Eagle] タイミングに依存せず、ストレージデバイスサイズを取得できるようにする対応.
	ST_HDD_DEVICE_INFO st_DeviceInfo;
	memset( &st_DeviceInfo, 0, sizeof(st_DeviceInfo) );
	uint32_t u32_ret = THRF_sysBspHddDeviceInfoGet(i_DviceId , 0, &st_DeviceInfo );
	if( HDD_STATE_OK == u32_ret ){	// ストレージデバイス情報を取得.
		dl_PhysicalSubDeviceSize = (DLONG)st_DeviceInfo.iTotalBlocks * (DLONG)DEF_SECTOR_SIZE;	// 取得OK時のみストレージデバイスサイズを初期値として格納.
	}
	memset( m_uc_FuncCallPointCash, 0, sizeof(m_uc_FuncCallPointCash) );
	memset( m_uc_FuncCallPointIntErr, 0, sizeof(m_uc_FuncCallPointIntErr) );
	memset( m_uc_FuncCallNestCnt, 0, sizeof(m_uc_FuncCallNestCnt) );
	memset( m_uc_FuncCallIntErrCnt, 0, sizeof(m_uc_FuncCallIntErrCnt) );

	uc_checkSecurityLocked = 0;
	uc_disableSSDAccess = 0;	// SSDアクセス許可状態.

// RQ-560 2019/08/27 RQ-560 Sim起動問題対応 Simの時のuc_HDDbuffer領域確保方法変更 ->.
#ifdef FUM_LINUX

	uint32_t		ul_FatTotalSize_local;						// FATトータルサイズ.
	part_layout*	s_DeviceInfo = THRG_GetPartitionType();		// PartitionType取得.
	if( ssd_part_info_1TB == s_DeviceInfo ) {
		ul_FatTotalSize_local = DEF_FAT_TOTAL_SIZE_1T;
	} else {
		ul_FatTotalSize_local = DEF_FAT_TOTAL_SIZE;
	}

//	uc_HDDbuffer = (uchar *)(((uintptr_t)__uc_HDDbuffer + 0x1FF) & ~0x1FF);
	uc_HDDbuffer = (uchar *)malloc( ul_FatTotalSize_local + 0x200 );
#endif /* FUM_LINUX */
// RQ-560 2019/08/27 RQ-560 Sim起動問題対応 Simの時のuc_HDDbuffer領域確保方法変更 <-.

}


/*================================================================================================
［機能］	【ＩＦ】物理HDDサイズを返す
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	DLONG	：物理HDDサイズ
［詳細］	ＦＵＭ側で使用可能な論理HDDサイズを返す。（物理HDDサイズをＯＳとＦＵＭで分割している）
［ｳｫｰｸｽﾙｰ］	再度必要：2001/1/10：森川
================================================================================================*/
DLONG CP_HDDTrans::getPhysicalHDDSize()
{
	return dl_PhysicalHDDSize;
}

/*================================================================================================
［機能］	【ＩＦ】物理SSDサイズを返す.
--------------------------------------------------------------------------------------------------
［引数］	無し.
［戻値］	DLONG	：物理SSDサイズ.
［詳細］	物理SSDサイズを返す.
［ｳｫｰｸｽﾙｰ］	09/04/22 t-kasahara CQ:317046 SSD廃棄消去_フォーマット対応.
================================================================================================*/
DLONG CP_HDDTrans::getPhysicalSubDeviceSize()
{

// 2013/10/10 ADS吉田 イテ3マージ 起動確認対応 パーティションサイズ変更対応.
//#ifdef DEF_ARMADAXP
//	dl_PhysicalSubDeviceSize = DEF_SSD_MINIMUM_SIZE*2*DEF_SECTOR_SIZE; /* 1GB */  //add sky_hnis
//#endif
	
	return dl_PhysicalSubDeviceSize;
}

// Debugコマンド
void CP_HDDTrans::setPhysicalSubDeviceSize(uint32_t ssdsize){
// 2013/10/10 ADS吉田 イテ3マージ 起動確認対応 パーティションサイズ変更対応.
//#ifdef DEF_ARMADAXP
//	dl_PhysicalSubDeviceSize = DEF_SSD_MINIMUM_SIZE*2*DEF_SECTOR_SIZE; /* 1GB */;  //add sky_hnis
//#else
	dl_PhysicalSubDeviceSize = ssdsize;
//#endif
}


/*================================================================================================
［機能］	【ＩＦ】SSD NVRAMエリアのステータスを返す.
--------------------------------------------------------------------------------------------------
［引数］	無し.
［戻値］	TYPE_HddStatus	：物理SSDサイズ.
［詳細］	SSD NVRAMエリアのステータスを返す.
［ｳｫｰｸｽﾙｰ］	09/04/22 t-takayama CQ344683対応.
================================================================================================*/
TYPE_HddStatus CP_HDDTrans::getNvramInfoAreaStateInSSD(){
	return e_NvramInfoAreaState;
}

/*================================================================================================
［機能］	データ読みだし＆チェック
--------------------------------------------------------------------------------------------------
［引数］	char*					：データバッファアドレス
［戻値］	ushort					：フォーマット済み(HDD_STATUS_OK)未初期化(HDD_NOT_INITIALIZED)
［詳細］	物理フォーマットの変わりに識別文字を読み出しフォーマットされているかを確認する
［ｳｫｰｸｽﾙｰ］	実施済み：2001/1/10：森川
［履歴］	07/05/14 morikawa 静的解析の不具合修正　
================================================================================================*/
ushort CP_HDDTrans::checkFormat( const char* pBuff)
{
	char System_Data[] = IDE_FORMAT_STRING;
	register ushort us_Result = HDD_STATUS_OK;	// フォーマット済みで初期化
	/* フォーマット情報('_MINOLTA')を読む */
	for( register ushort idx=0;idx < IDE_FORMAT_STRING_LEN;++idx) {
		if( *(pBuff + idx) != System_Data[idx] ) {
			// フォーマット情報が一致しない（未フォーマット）
			us_Result = HDD_NOT_INITIALIZED;
			break;	// 直ちに処理中断
		}
	}
	return us_Result;
}
/*================================================================================================
［機能］	IDENTIFY DEVICE情報チェック＆保存
--------------------------------------------------------------------------------------------------
［引数］	char*	：データバッファアドレス
［戻値］	ushort	：HDD_STATUS_OK 正常
					  HDD_OUT_OF_SPEC スペック外異常
［詳細］	IDENTIFY DEVICE情報を内部のバッファにコピーする
［ｳｫｰｸｽﾙｰ］	実施未：2001/10/09：森川
［履歴］	2002/01/30：morikawa フォーマットの総セクタ数が実際のHDDの使用可能容量よりも少なかった問題修正
				HDDの最大ＬＢＡNoの取得をDEF_HDD_IDENTIFY_CURRENT_TOTAL_SECTORからDEF_HDD_IDENTIFY_LBA__TOTAL_SECTORに変更　
			2002/02/12：morikawa HDDのFUM領域とOS（dosFS）領域サイズ設定仕様追加
			2002/02/13：morikawa モデル番号一致チェック仕様追加
			2002/02/18：中田	 モデル番号検出処理はＮＴＳの時のみ有効とした
			2002/02/18：morikawa モデル番号一致チェック修正
							「IC25N010ATCS04-0」だけから、-以下は何を表すかわからないので見ないで
							「IC25N010ATCS04」or「IC25N010ATDA04」に変更
			2004/01/19：t.sasaki Rhein1ではまだHDDが決定していないこともあり、一時的にメーカーモデルチェックを外した。いずれ見直す必要あり
			2004/07/20：t.sasaki HDDセキュリティ対応(TSS2からの移植)
			2007/03/23：morikawa Donau初版
			07/04/03 morikawa DonauのI/F仕様にあわせて修正
			2008/10/16 takayama   CQ:311209 SSD 領域割り当て動的変更対応.
			2009/04/05 takayama   CQ:316760 SSD常時接続対応.
			2009/04/22 t-kasahara CQ:317046 SSD廃棄消去_フォーマット対応.
			2009/05/13 takayama   CQ:317046 デバイスサイズ表示対応(FW領域除外処理を削除).
			2009/06/02 takayama   CQ:317995 HDD装着設定変更後にD047発生不具合対応.
								    		(デバイスタイプ設定処理を削除しメソッドconnectionに移動).
			2011/04/06 takayama MinervaZeus SSD512MB対応.
			2014/02/21 toda		Zeus5BK SSDパーティション構成変更 8GB用テーブル追加.
			2014/07/23 iguchi　 Poseidon CQ501496 eMMCパーティション構成変更.
			2016/03/14 yamada	D011詳細コード対応.
			2018/04/20 Sky_松井	[Eagle]ストレージデバイスサイズ設定タイミング変更対応
================================================================================================*/
#ifdef DEF_SupportSSD 
ushort CP_HDDTrans::chkSaveIdentifyDevice( int32_t port ,ST_HDD_DEVICE_INFO* ps_HDD_DEVICE_INFO )
{
	// 構成情報チェック
	if( (ps_HDD_DEVICE_INFO->us_Construct & DEF_ATA_HDD_MASK) != DEF_ATA_HDD ) {
		// 構成情報がATAでない時は仕様外　
		// C-D011(HDD_OUT_OF_SPEC)詳細コードセット.
		if (THRE_HddPortNo == port) {
			// Ch-A.
			us_FA14DetailCode = selectOutOfSpecDetailCode(THRE_D011Point_NotAta_ChA);
		} else if (THRE_HddChBPortNo == port) {
			// Ch-B.
			us_FA14DetailCode = selectOutOfSpecDetailCode(THRE_D011Point_NotAta_ChB);
		} else {
			// eMMCは残さない.
		}
		THRL_ThLogMsgErr("chkSaveIdentifyDevice:not ATA Error\n");
		return HDD_OUT_OF_SPEC;
	}

	THRE_StorageDeviceType uc_deviceType = THRE_StorageDeviceTypeSSD;
	if (port == THRE_SsdPortNo){
		// port1はSSD (SSD装着設定が標準).
		// LBAモードでの使用可能サイズを保存　
		dl_PhysicalHDDSize = dl_PhysicalSubDeviceSize;
		
		// HDDのデバイス情報を内部のデータに保持.
		// セキュリティステイタスも含む.
		memcpy( &s_HDD_DEVICE_INFO[port], ps_HDD_DEVICE_INFO, sizeof( ST_HDD_DEVICE_INFO ) );
		
		areaInitValue(uc_deviceType);
		
		if( (uint32_t)ps_HDD_DEVICE_INFO->iTotalBlocks < getStorageInfo(THRE_StgInfoStorageMiniSize) ) {
			// デバイスの最小構成よりも小さい時は仕様外.
			THRL_ThLogMsgErr("chkSaveIdentifyDevice:size small Error\n");
			return HDD_OUT_OF_SPEC;
		}
		// 2014/07/23 iguchi CQ501496 eMMCパーティション構成変更.
		ssd_part_info = THRG_GetPartitionType();
		// AR1060044 sdの場合にpc_strDeviceSSDをkmsdaに設定すると
		// checkPartitionLayout()でpart_infoが参照され、パーティションチェックがNG(FA14)となるので、part_infoにssd_part_info設定しておく.
		part_info = ssd_part_info;

	} else {
		// port0でコールされる場合、HDD装着設定
//		uc_deviceType = THRE_StorageDeviceTypeHDD;
		// 暫定張替え処置.
		uc_deviceType = THRE_StorageDeviceTypeSSD;
		// index0はSSD (SSD装着設定が標準).
		
		// LBAモードでの使用可能サイズを保存　
    	dl_PhysicalHDDSize = dl_PhysicalSubDeviceSize;

		memcpy( &s_HDD_DEVICE_INFO[port], ps_HDD_DEVICE_INFO, sizeof( ST_HDD_DEVICE_INFO ) );
		
		areaInitValue(uc_deviceType);

		if( (uint32_t)ps_HDD_DEVICE_INFO->iTotalBlocks < getStorageInfo(THRE_StgInfoStorageMiniSize) ) {
			// デバイスの最小構成よりも小さい時は仕様外.
			// C-D011(HDD_OUT_OF_SPEC)詳細コードセット.
			if (THRE_HddPortNo == port) {
				// Ch-A.
				us_FA14DetailCode = selectOutOfSpecDetailCode(THRE_D011Point_OutOfSpec_ChA);
			} else if (THRE_HddChBPortNo == port) {
				// Ch-B.
				us_FA14DetailCode = selectOutOfSpecDetailCode(THRE_D011Point_OutOfSpec_ChB);
			} else {;}
			THRL_ThLogMsgErr("chkSaveIdentifyDevice:size small Error\n");
			return HDD_OUT_OF_SPEC;
		}
		
		// 暫定張替え処置.
		ssd_part_info = THRG_GetPartitionType();
		part_info = ssd_part_info;

	}
	return HDD_STATUS_OK;
}

#else

ushort CP_HDDTrans::chkSaveIdentifyDevice( ST_HDD_DEVICE_INFO* ps_HDD_DEVICE_INFO )
{
	// 構成情報チェック
	if( (ps_HDD_DEVICE_INFO->us_Construct & DEF_ATA_HDD_MASK) != DEF_ATA_HDD ) {
		// 構成情報がATAでない時は仕様外　
		THRL_ThLogMsgErr("chkSaveIdentifyDevice:not ATA Error\n");
		return HDD_OUT_OF_SPEC;
	}
	if( ps_HDD_DEVICE_INFO->iTotalBlocks < DEF_HDD_MINIMUM_SIZE ) {
		// LBAサイズがHDDの最小構成よりも小さい時は仕様外　
		THRL_ThLogMsgErr("chkSaveIdentifyDevice:size small Error\n");
		return HDD_OUT_OF_SPEC;
	}
	// LBAモードでの使用可能サイズを保存　
	DLONG size = ps_HDD_DEVICE_INFO->iTotalBlocks;
	dl_PhysicalHDDSize = size * (uint32_t)DEF_SECTOR_SIZE;
	
	// HDDのデバイス情報を内部のデータに保持
	// セキュリティステイタスも含む　
	memcpy( &s_HDD_DEVICE_INFO, ps_HDD_DEVICE_INFO, sizeof( ST_HDD_DEVICE_INFO ) );

	return HDD_STATUS_OK;
}

#endif

/*================================================================================================.
［機能］	HDDの型番を取得.
--------------------------------------------------------------------------------------------------.
［引数］	char *psc_outbuf		： 型番を格納するバッファへのポインタ.
［戻値］	Bool					： True：型番取得成功 ／ False：型番取得失敗.
［詳細］	最大41バイト（HDDデバイス情報取得処理で取得した40バイト + NULL文字）の型番をpsc_outbufへ格納する.
			型番を格納するバッファは41バイト以上である事.
［履歴］	2013/05/08 堀下		新規作成(CQ.379013 HDDイレースレポート対応).
================================================================================================*/
Bool CP_HDDTrans::getHddModel( char *psc_outbuf )
{
	Bool b_ret = False;

	if( NULL != psc_outbuf ) {
		b_ret = True;
#ifdef DEF_SupportSSD
		strcpy( psc_outbuf, s_HDD_DEVICE_INFO[THRE_HddPortNo].ctHddProductId );	// 型番を出力バッファへCOPY.
#else
		strcpy( psc_outbuf, s_HDD_DEVICE_INFO.ctHddProductId );					// 型番を出力バッファへCOPY.
#endif
	}

	return b_ret;
}

// ================================================================================================
//! <!--機能--> HDDの型番を取得.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! char *psc_outbuf : HDDの型番保存領域.
//! THRE_HddMirroring_Chanel e_Chanel : チャネル情報.
//! <!--戻値-->
//! \return THRE_HddMirroring_Chanel マスターチャネル.
//! 									THRE_HddMirroring_Chanel_A : Ch-A.
//! 									THRE_HddMirroring_Chanel_B : Ch-B.
//! <!--詳細-->
//!	引数で指定するチャネルの型番を返す.
//! <!--履歴-->
//	2014/09/08 yamada	ZeusS 制御構成変更No.4 HDDミラーリング対応.
//! ================================================================================================
Bool CP_HDDTrans::getHddModel(char *psc_outbuf, THRE_HddMirroring_Chanel e_Chanel)
{
	// NULLチェック.
	if (psc_outbuf == NULL) {
		setDebugLowLogSpeedSeq(700, 0) ;
		return False;
	}

	// 引数を初期化(40byte).
	memset(psc_outbuf, 0x00, HDD_BUFSIZE_PRODUCT);

	// 指定チャネルのシリアル番号をバッファに設定する.
	switch(e_Chanel) {
	case THRE_HddMirroring_Chanel_A:
		memcpy(psc_outbuf, s_HDD_DEVICE_INFO[THRE_HddPortNo].ctHddProductId, HDD_BUFSIZE_PRODUCT);
		break;
	case THRE_HddMirroring_Chanel_B:
		memcpy(psc_outbuf, s_HDD_DEVICE_INFO[THRE_HddChBPortNo].ctHddProductId, HDD_BUFSIZE_PRODUCT);
		break;
	default:
		setDebugLowLogSpeedSeq(701, e_Chanel) ;
		return False;
		break;
	}

	return True;
}

/*================================================================================================.
［機能］	HDDのシリアル番号を取得.
--------------------------------------------------------------------------------------------------.
［引数］	char *psc_outbuf		： シリアル番号を格納するバッファへのポインタ.
［戻値］	Bool					： True：シリアル番号取得成功 ／ False：シリアル番号取得失敗.
［詳細］	最大21バイト（HDDデバイス情報取得処理で取得した20バイト + NULL文字）のシリアル番号をpsc_outbufへ格納する.
			シリアル番号を格納するバッファは21バイト以上である事.
［履歴］	2013/05/08 堀下		新規作成(CQ.379013 HDDイレースレポート対応).
================================================================================================*/
Bool CP_HDDTrans::getHddSerialNo( char *psc_outbuf )
{
	Bool b_ret = False;

	if( NULL != psc_outbuf ) {
		b_ret = True;
#ifdef DEF_SupportSSD
		strcpy( psc_outbuf, s_HDD_DEVICE_INFO[THRE_HddPortNo].ctHddSerialNo );	// シリアル番号を出力バッファへCOPY.
#else
		strcpy( psc_outbuf, s_HDD_DEVICE_INFO.ctHddSerialNo );					// シリアル番号を出力バッファへCOPY.
#endif
	}

	return b_ret;
}

// ================================================================================================
//! <!--機能--> シリアル番号取得.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! char *psc_outbuf : シリアル番号保存領域.
//! THRE_HddMirroring_Chanel e_Chanel : チャネル情報.
//! <!--戻値-->
//! \return Bool 取得結果(True : 正常終了 / False : 異常終了).
//! <!--詳細-->
//!	引数で指定するチャネルのシリアル番号を返す.
//! <!--履歴-->
//	2014/09/08 yamada	ZeusS 制御構成変更No.4 HDDミラーリング対応.
//! ================================================================================================
Bool CP_HDDTrans::getHddSerialNo(char *psc_outbuf, THRE_HddMirroring_Chanel e_Chanel)
{
	// NULLチェック.
	if (psc_outbuf == NULL) {
		setDebugLowLogSpeedSeq(705, 0) ;
		return False;
	}

	// 引数を初期化(20byte).
	memset(psc_outbuf, 0x00, HDD_BUFSIZE_SERIALNO);

	// 指定チャネルのシリアル番号をバッファに設定する.
	switch(e_Chanel) {
	case THRE_HddMirroring_Chanel_A:
		memcpy(psc_outbuf, s_HDD_DEVICE_INFO[THRE_HddPortNo].ctHddSerialNo, HDD_BUFSIZE_SERIALNO);
		break;
	case THRE_HddMirroring_Chanel_B:
		memcpy(psc_outbuf, s_HDD_DEVICE_INFO[THRE_HddChBPortNo].ctHddSerialNo, HDD_BUFSIZE_SERIALNO);
		break;
	default:
		setDebugLowLogSpeedSeq(706, e_Chanel) ;
		return False;
		break;
	}

	return True;
}

/*================================================================================================.
［機能］	eMMCの型番を取得.
--------------------------------------------------------------------------------------------------.
［引数］	char *psc_OutBuf		： 型番を格納するバッファへのポインタ.
［戻値］	Bool					： True：型番取得成功 ／ False：型番取得失敗.
［詳細］	型番の文字列(最大49バイト)をpsc_OutBufへ格納する.
			型番を格納するバッファは49バイト以上である事.
［履歴］	2014/07/03 山田		制御構成変更No39 廃棄時消去レポート対応.
================================================================================================*/
Bool CP_HDDTrans::GetEMMCModel(char *psc_OutBuf)
{
	char	sc_Cmd[128] = {0};
	FILE	*ps_File = NULL;

	if (psc_OutBuf == NULL) {
		THRL_ThLogMsgErr("GetEMMCModel() Parameter NULL.\n");
		return False;
	}

	memset(psc_OutBuf, 0, THRD_EMMC_ModelSize+1);

	// 型番ファイルをcatで読み出す.
	snprintf(sc_Cmd, sizeof(sc_Cmd), "exec cat %s", THRD_EMMC_ModelDataPath);
	ps_File = popen(sc_Cmd, "r");
	if (ps_File == NULL) {
		THRL_ThLogMsgErr("GetEMMCModel() popen failed. errno=%d.\n", SYSF_errnoGet());
		return False;
	}

	// 出力結果を取得.
	fgets(psc_OutBuf, THRD_EMMC_ModelSize, ps_File);
	if (psc_OutBuf[0] == '\0') {
		// 読み出しデータが可変である為、fgets()の戻り値ではエラーチェック出来ない.
		// 読み出し用バッファに何も読みだされていない場合のみ、エラーとする.
		THRL_ThLogMsgErr("GetEMMCModel() fgets failed. errno=%d.\n", SYSF_errnoGet());
		pclose(ps_File);
		return False;
	}
	pclose(ps_File);

	return True;
}

/*================================================================================================.
［機能］	eMMCのシリアル番号を取得.
--------------------------------------------------------------------------------------------------.
［引数］	char *psc_OutBuf		： シリアル番号を格納するバッファへのポインタ.
［戻値］	Bool					： True：シリアル番号取得成功 ／ False：シリアル番号取得失敗.
［詳細］	シリアル番号の文字列(最大32バイト)をpsc_OutBufへ格納する.
			シリアル番号を格納するバッファは33バイト以上である事.
［履歴］	2014/07/03 山田		制御構成変更No39 廃棄時消去レポート対応.
================================================================================================*/
Bool CP_HDDTrans::GetEMMCSerialNo(char *psc_OutBuf)
{
	char	sc_Cmd[128] = {0};
	FILE	*ps_File = NULL;

	if (psc_OutBuf == NULL) {
		THRL_ThLogMsgErr("GetEMMCSerialNo() Parameter NULL.\n");
		return False;
	}

	memset(psc_OutBuf, 0, THRD_EMMC_SerialSize+1);

	// 型番ファイルをcatで読み出す.
	snprintf(sc_Cmd, sizeof(sc_Cmd), "exec cat %s", THRD_EMMC_SerialDataPath);
	ps_File = popen(sc_Cmd, "r");
	if (ps_File == NULL) {
		THRL_ThLogMsgErr("GetEMMCSerialNo() popen failed. errno=%d.\n", SYSF_errnoGet());
		return False;
	}

	// 出力結果を取得.
	fgets(psc_OutBuf, THRD_EMMC_SerialSize, ps_File);
	if (psc_OutBuf[0] == '\0') {
		// 読み出しデータが可変である為、fgets()の戻り値ではエラーチェック出来ない.
		// 読み出し用バッファに何も読みだされていない場合のみ、エラーとする.
		THRL_ThLogMsgErr("GetEMMCSerialNo() fgets failed. errno=%d.\n", SYSF_errnoGet());
		pclose(ps_File);
		return False;
	}
	pclose(ps_File);

	return True;
}

/*================================================================================================
［タイトル］	ＯＳ
================================================================================================*/

/*================================================================================================
［機能］	セキュリティロック中(UNLOCKでOFF) 
--------------------------------------------------------------------------------------------------
［引数］	なし
［戻値］	Bool					：True:ロック中 False:ロック解除(ロックされていない)
［詳細］	※この関数を呼ぶ前にIdentify Deviceで情報を読み出しを行いus_SecurityStatusを設定すること 
［履歴] 	05/10/27 tsukiji 新規追加 
            09/04/05 takayama CQ:316760 SSD常時接続対応.
================================================================================================*/
// 2009/04/05   takayama CQ:316760 SSD常時接続対応.
#ifdef DEF_SupportSSD 
Bool  CP_HDDTrans::isSecurityLocked(int32_t portNo)	// 
{
	if( (s_HDD_DEVICE_INFO[portNo].us_SecurityStatus & DEF_SECURITY_SUPPORT)!=0 ) {
		return (Bool)((s_HDD_DEVICE_INFO[portNo].us_SecurityStatus & DEF_SECURITY_LOCKED)!=0);
	} else {
		return False;	// サポートされていないHDDは常にロックなしで返す 
	}
}
#else

Bool  CP_HDDTrans::isSecurityLocked()	// 
{
	if( (s_HDD_DEVICE_INFO.us_SecurityStatus & DEF_SECURITY_SUPPORT)!=0 ) {
		return (Bool)((s_HDD_DEVICE_INFO.us_SecurityStatus & DEF_SECURITY_LOCKED)!=0);
	} else {
		return False;	// サポートされていないHDDは常にロックなしで返す 
	}
}
#endif
/*================================================================================================
［機能］	セキュリティ設定中(PASSWORD設定でON) 
--------------------------------------------------------------------------------------------------
［引数］	なし
［戻値］	Bool					：True:パスワード設定されている False:設定されていない
［詳細］	※この関数を呼ぶ前にIdentify Deviceで情報を読み出しを行いus_SecurityStatusを設定すること 
［履歴] 	05/10/27 tsukiji 新規追加 
            09/04/05 takayama CQ:316760 SSD常時接続対応.
================================================================================================*/
// 09/04/05   takayama CQ:316760 SSD常時接続対応.
#ifdef DEF_SupportSSD
Bool  CP_HDDTrans::isSecurityEnable(int32_t portNo)
{
	if( (s_HDD_DEVICE_INFO[portNo].us_SecurityStatus & DEF_SECURITY_SUPPORT)!=0 ) {
		return (Bool)((s_HDD_DEVICE_INFO[portNo].us_SecurityStatus & DEF_SECURITY_ENABLE)!=0);
	} else {
		return False;	// サポートされていないHDDは常にセキュリティ設定なしで返す 
	}
}
#else
Bool  CP_HDDTrans::isSecurityEnable()
{
	if( (s_HDD_DEVICE_INFO.us_SecurityStatus & DEF_SECURITY_SUPPORT)!=0 ) {
		return (Bool)((s_HDD_DEVICE_INFO.us_SecurityStatus & DEF_SECURITY_ENABLE)!=0);
	} else {
		return False;	// サポートされていないHDDは常にセキュリティ設定なしで返す 
	}
}
#endif

#ifdef DEF_SupportSSD
/*================================================================================================
［機能］	APMコマンドサポート.
--------------------------------------------------------------------------------------------------
［引数］	なし.
［戻値］	Bool					：True:サポート False:非サポート.
［詳細］	※この関数を呼ぶ前にIdentify Deviceで情報を読み出しを行いus_SupportCommandを設定すること.
［履歴] 	08/11/21 takayama CQ:311209 Taiga APM非対応デバイス対応.
            09/04/05 takayama CQ:316760 SSD常時接続対応.
================================================================================================*/
Bool  CP_HDDTrans::isAPMSupport(int32_t portNo)
{
// 09/04/05   takayama CQ:316760 SSD常時接続対応.
	if( (s_HDD_DEVICE_INFO[portNo].us_SupportCommand & DEF_APM_SUPPORT)!=0 ) {
		return True;
	} else {
		return False; 
	}
}
#endif	// DEF_SupportSSD.

#ifdef DEF_SupportSSD
/*================================================================================================
［機能］	Securityコマンドサポート.
--------------------------------------------------------------------------------------------------
［引数］	なし.
［戻値］	Bool					：True:サポート False:非サポート.
［詳細］	※この関数を呼ぶ前にIdentify Deviceで情報を読み出しを行いus_SupportCommandを設定すること.
［履歴] 	08/12/12 takayama CQ:313055 Taiga セキュリティ非サポートSSD対応.
            09/04/05 takayama CQ:316760 SSD常時接続対応.
================================================================================================*/
Bool  CP_HDDTrans::isSecuritySupport(int32_t portNo)
{
// 09/04/05   takayama CQ:316760 SSD常時接続対応.
	if (s_HDD_DEVICE_INFO[portNo].us_SecurityStatus !=0 ) {
		return True;
	} else {
		return False; 
	}
}
#endif	// DEF_SupportSSD.

/*================================================================================================
［機能］	【ＩＦ】ＯＳ：指定されたセクタ数分読み込み、書き込みを行う（アクセス元によってオフセット分セクタアドレスをずらす)
--------------------------------------------------------------------------------------------------
［引数］	int						：ドライブ番号
			int						：シリンダ番号
			int						：ヘッド番号
			int						：セクタ番号
			void*					：バッファアドレス
			int						：セクタ数
			int						：転送方向( 
［戻値］	int						：正常(C_OK)それ以外異常
［詳細］
［ウォークスルー］	未実施
［履歴］	04/07/20 t.sasaki HDDセキュリティ対応(TSS2からの移植)
			04/08/23 morikawa 大量ファイルにてdosFSが異常となる問題修正
							cb_LBACHS.convertCHSToLBA()の第１引数(sylider)が誤ってucharにキャストされていたのをushortにキャストに修正
							第１引数(sector)が誤ってushortにキャストされていたのをucharにキャストに修正			
			04/11/23 morikawa HDDスリープ時の不正アクセスデバッグ処理追加　				
			05/06/13 t.sasaki DB.37599対策(グローバルセマフォが絡んだデッドロックが原因)
							デッドロックの主要因になっているグローバルセマフォ一時解放処理を削除
							グロバールセマフォを握ったままのHDD操作を検出(ログ or Suspend)
			07/05/14 morikawa 静的解析不具合修正　				
================================================================================================*/
static int32_t debug_sleep_iderw = 0;
static APIC_HddInfo* pHddInfo = NULL;
void debug_sleep_set();
void debug_sleep_set()
{
	debug_sleep_iderw = 1;	// スリープHDDデバッグ有効　
   	pHddInfo = APIC_HddInfo::newInstance();
#ifdef TORNADO
	THRL_ThLogMsgErr("Sub Power Off debug sleep enable\n");
#endif
}

// S 2005/06/13 t.sasaki SSSSSSSSSSSSSSSSSSSSSSSSSSSSSS
// グロバールセマフォを握ったままのHDD操作を検出(のログ出力) 2005/06/13 added by t.sasaki
static ushort us_debug_GSIO_Level = 0 ;	// 検出時の処理 0:ログ 1:ターミナル出力 2:サスペンド .
static ushort us_GSIOcheckIndex = 0 ;
static THR_ID sn_SuspendTaskID = 0 ;
static struct ST_GSIOTBL{
	THR_ID	nTaskID;		// Task ID
	uint32_t	ulCount;		// Counter
	char	strName[16];	// Task Name(with EOT='\x0')
	uint32_t	ulTick;			// Tick(Recent)
} taskGSIOTbl[10] ;
extern "C" {
	void debugGSIO(ushort on);	// デバッグレベル設定.
	void clearGSIO();			// ログ初期化.
	void dspGSIO(ushort on);	// ログ参照.
	void resumeGSIO();			// サスペンドタスク復帰.
	void debug_getssdsize();
	void debug_setssdsize();
	void sysBspSATADump(int32_t mode, uint32_t Secter, uint32_t RSize);
	// 14.07.07 yamada 制御構成変更No39 廃棄時消去レポート対応.
	void THRG_PrintEMMCInfo();	// eMMCの型番とシリアルNo出力.
}

/*================================================================================================
［機能］	eMMCの型番とシリアルNoを出力する.
--------------------------------------------------------------------------------------------------
［引数］	なし.
［戻値］	なし.
［詳細
［履歴] 	14/07/07	yamada	制御構成変更No39 廃棄時消去レポート対応.
================================================================================================*/
void THRG_PrintEMMCInfo() {
	char	sc_ModelInfo[THRD_EMMC_ModelSize+1] = {0};			// eMMC型番取得バッファ.

	Bool bl_Ret = theHDDTrans.GetEMMCModel(sc_ModelInfo);		// eMMCの型番取得.
	if (bl_Ret != False) {
		printf("eMMC Model    : %s\n", sc_ModelInfo);
	} else {
		printf("Get eMMC Model Failed.\n");
	}

	char	sc_SerialInfo[THRD_EMMC_SerialSize+1] = {0};		// eMMCシリアルNo取得バッファ.

	bl_Ret = theHDDTrans.GetEMMCSerialNo(sc_SerialInfo);		// eMMCのシリアルNo取得.
	if (bl_Ret != False) {
		printf("eMMC SerialNo : %s\n", sc_SerialInfo);
	} else {
		printf("Get eMMC SerialNo Failed.\n");
	}

}

void debug_getssdsize(){
	// 2017/02/20 Sky DLONG対応.
	printf("####SSD SIZE:%"PRIu64"\n",theHDDTrans.getPhysicalSubDeviceSize());
}

void debug_setssdsize(uint32_t ssdsize){
	theHDDTrans.setPhysicalSubDeviceSize(ssdsize);
}

// デバッグレベル設定.
void debugGSIO(ushort on)
{
	us_debug_GSIO_Level = on ;
}
// 初期化.
void clearGSIO()
{
	us_GSIOcheckIndex = 0 ;
}
// 参照.
void dspGSIO(ushort on)
{
#ifdef TORNADO
	printf( " Count=[%"PRId32"]\n", (int32_t)us_GSIOcheckIndex );
	printf( " (Tick(at least))No. ID       Name             Count\n" ) ;
	printf( " -----------------------------------------------------\n" ) ;
	for ( int32_t nIndex = 0; nIndex < us_GSIOcheckIndex; nIndex++ ) {
		ST_GSIOTBL* p = &taskGSIOTbl[nIndex] ;
		char* pName = p->strName ;
		char buf[16] ;
		if ( on == 1 ) {		// タスクIDからタスク名を求める(消滅していたり、別のタスクに代わっている場合あり)
			pName = &buf[0] ;
			char* pTaskName = SYSF_taskName(p->nTaskID);
			if(pTaskName != NULL){
//			strncpy( pName, SYSF_taskName(p->nTaskID), 15 ) ;
				strncpy( pName, pTaskName, 15 ) ;
				buf[15] = (char)'\x0' ;
			}
		}
		printf( "(%8"PRIu32") %3"PRId32" %08"PRIX32" %16s %8"PRIu32"\n", p->ulTick, nIndex, p->nTaskID, pName, p->ulCount ); 
	}
#endif	// #ifdef TORNADO
}
// サスペンドタスク復帰.
void resumeGSIO()
{
#ifdef TORNADO
	if ( sn_SuspendTaskID > 0 ) {
		printf( " Resume Task '%s'(ID=%08"PRIX32")\n", SYSF_taskName( sn_SuspendTaskID), sn_SuspendTaskID ) ;
		SYSF_taskResume( sn_SuspendTaskID ) ;
		sn_SuspendTaskID = 0 ;
	}
#endif	// #ifdef TORNADO
}
// 登録().
static void checkGSIO()
{
#ifdef TORNADO
	// グローバルセマフォを握ったままここに到達した.

	// ターミナルに警告出力(Level=1以上.
	if ( us_debug_GSIO_Level >= 1 ) {
		SYSF_logMsg("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n",0,0,0,0,0,0) ;
		SYSF_logMsg("@@@@@@ Semaphore Error(HDD BIOS:CB_IDEHard::ideRW()) @@@@@\n",0,0,0,0,0,0) ;
		SYSF_logMsg("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n",0,0,0,0,0,0) ;
	}

	// ログ(Level=0以上)
	if ( us_GSIOcheckIndex < 10 ) {		// 登録テーブルいっぱいなら何もしない.
		Bool bl_exist = False ;
		ST_GSIOTBL* pTbl = &taskGSIOTbl[0] ;
		for ( int32_t nIndex = 0; nIndex < (int32_t)us_GSIOcheckIndex; nIndex++ ) {
			// 重複登録しない(タスクID＆タスク名両方でチェックする).
			if ( ( pTbl->nTaskID == SYSF_taskIdSelf() )
			  && ( strncmp( pTbl->strName, SYSF_taskName(SYSF_taskIdSelf()), sizeof(pTbl->strName)-1 ) == 0 ) ) {
				bl_exist = True ;
				break ;
			}
			pTbl++ ;
		}
		if ( bl_exist == False ) {	// add.
			pTbl->nTaskID = SYSF_taskIdSelf() ;
			pTbl->ulCount = 1 ;
			strncpy( pTbl->strName, SYSF_taskName(SYSF_taskIdSelf()), sizeof(pTbl->strName)-1 ) ;
			pTbl->ulTick = SYSF_tickGet() ;
			us_GSIOcheckIndex++ ;
		} else {	// update.
			if ( pTbl->ulCount < UINT_MAX ) pTbl->ulCount++ ;
			pTbl->ulTick = SYSF_tickGet() ;
		}
	}

	// サスペンド(Level=2)
	if ( us_debug_GSIO_Level == 2 ) {
		SYSF_logMsg(" Suspend Task (ID=%08"PRIXPTR")\n", SYSF_taskIdSelf(), 0,0,0,0,0) ;
		sn_SuspendTaskID = SYSF_taskIdSelf() ;
		SYSF_taskSuspend(SYSF_taskIdSelf()) ;
	}
#endif

}

// S セクタログ 2005/10/04 added by t.sasaki
// DOSFSのIDE R/Wをセクタレベルでトレースする場合のみ下記DEFINEを有効にする.
//#define DEF_HDD_SECTORLOG
#ifdef DEF_HDD_SECTORLOG
int32_t nSecnotblidx = 0 ;
int32_t nSecnotblflg = 0 ;
typedef struct {
	int32_t nStartSecNo ;
	int32_t nSecNum ;
	uint32_t ulTick ;
} ST_secnotbl ;
ST_secnotbl st_secnotbl[100] ;
extern "C" {
void debugFlagsecnotbl(int32_t n) ;
void clearsecnotbl() ;
void dspsecnotbl() ;
}
void debugFlagsecnotbl(int32_t n)
{
	nSecnotblflg = n ;
}
void clearsecnotbl()
{
	nSecnotblidx = 0 ;
}
void setsecnotbl(int32_t nStartSecNo, int32_t nSecNum )
{
	ST_secnotbl* ptbl ;
	if ( nSecnotblflg != 1 ) {
		return ;
	}
	if ( nSecnotblidx < 100 ) {
		ptbl = &st_secnotbl[nSecnotblidx] ;
		ptbl->nStartSecNo = nStartSecNo ;
		ptbl->nSecNum     = nSecNum ;
		ptbl->ulTick      = SYSF_tickGet() ;
		nSecnotblidx++ ;
	}
}
void dspsecnotbl()
{
	int32_t i ;
	ST_secnotbl* ptbl ;
	printf( "Num=[%"PRId32"]\n", nSecnotblidx ) ;
	for ( i = 0; i < nSecnotblidx; i++ ) {
		ptbl = &st_secnotbl[i] ;
		printf( "  %6"PRIu32" No=%7"PRId32" Num=%"PRId32"\n", ptbl->ulTick, ptbl->nStartSecNo, ptbl->nSecNum ) ;
	}
}
#endif	//#ifdef DEF_HDD_SECTORLOG
// E セクタログ 2005/10/04 added by t.sasaki

// ================================================================================================
//! <!--機能--> dosFSのHDDへのRead/Write実行
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param int ドライブ番号
//! \param int シリンダ番号
//! \param int ヘッダ番号
//! \param int セクタ番号
//! \param void * アクセス用バッファ
//! \param int 転送セクタ数
//! \param int 転送方向
//! <!--戻値-->
//! \return int C_OK　成功のみ返す
//! <!--詳細-->
//! \n dosFSのHDDへのRead/Writeを実行する。暗号化ありの時は暗号化を実行する
//! <!--履歴-->
//  07/03/07 morikawa Donau初版
//  07/05/14 morikawa 静的解析不具合修正　
//  07/08/23 morikawa SATAの仕様制限対応
//							仕様：データバッファのアドレスは奇数アドレス始まらなければならない
//							対策：暗号化なしの時に
//									HDD書き込み時：先頭奇数要求バイトバッファであれば、転送前にデータバッファから暗号化バッファにmemcpy
//									HDD読み出し時：先頭奇数要求バイトバッファであれば、転送後に暗号化バッファからデータバッファにmemcpy
//  08/01/30 morikawa 暗号化ありの時にdosFSの転送異常となる不具合修正
//					原因：暗号化あり時のキャッシュ制御に問題があった。
//						Write時：書き込みデータバッファ→DMA11→暗号化後バッファ→HDDのDMA
//						Read時：HDDのDMA→暗号化前バッファ→DMA11→読み出データバッファ
//								上記各DMA転送において以下の３つの処理は行っていた
//								１．ソース側キャッシュフラッシュ
//								２．HDDのDMA転送、DMA11の転送
//								３．ディスティネーション側のキャッシュインバリデット
//							しかし、dosFSから要求のあるideRW関数内で書き込みデータバッファ
//							及び読み出しデータバッファのキャッシュフラッシュを行っていなかったため。問題が発生した。
//					対策：この関数にて要求を受けたバッファはDMA転送処理前にキャッシュフラッシュを行う
//  08/02/27 morikawa スキャン送信２０ページの繰り返しで異常画像となる不具合修正
//					原因：スキャン送信実行時は、ImageFormatterでファイル作成しnetTxScanでこのファイルを2000バイト単位で読み出してNICFUMへ送信する。
//						この、netTxScanのファイル読み出し時にちょうどdosFsの１クラスタサイズ32KBの境界の時にdosFSでは２クラスタ分連続してデータのReadが
//						実行される、このときに修正前のソースでは１回目の32KBの最終の16バイトのデータが他のデータに置き換わる。
//					対策：修正前はこの関数にてdosFSから引数で渡されたバッファに対して直接キャッシュフラッシュの処理を行っていたが
//						引数で渡されたバッファ領域に関しては、キャッシュフラッシュもキャッシュ無効化も行わないようにする。
//						暗号化有り時：
//							HDD書き込み時：１．転送前に引数で渡されたデータバッファからul_AESBufAddrにmemcpy、
//										　 ２．暗号化実行(ul_AESBufAddr→ul_WorkMemAddr）
//										　 ３．ＨＤＤ転送実行(ul_WorkMemAddr→HDD）
//							HDD読み出し時：１．ＨＤＤ転送実行(HDD→ul_WorkMemAddr）、
//										　 ２．暗号化実行(ul_WorkMemAddr→ul_AESBufAddr）
//										　 ３．転送後にul_AESBufAddrから引数で渡されたデータバッファにmemcpy
//						暗号化なし時：
//							HDD書き込み時：１．転送前に引数で渡されたデータバッファからul_WorkMemAddrにmemcpy
//										　 ２．ＨＤＤ転送実行(ul_WorkMemAddr→HDD）
//							HDD読み出し時：１．ＨＤＤ転送実行(HDD→ul_WorkMemAddr）
//										　 ２．転送後にul_WorkMemAddrから引数で渡されたデータバッファにmemcpy
//  08/09/29 morikawa CQ309555 ASICによるHDD暗号化で、dosFSについて、IV(イニシャルベクタ)固定となっていた不具合修正
//							上記対応するにあたり、暗号化あり時の暗号化実行関数への最後の引数でセクタ番号が抜けていた問題が発覚した。
//							最後の引数でセクタ番号を正しく渡すように修正。
//	08/11/25 horiuchi CQ311540 dosのAES&HDD転送と直接転送のセマフォ制御を変更.
//							encodeDecodeExec内で行っているセマフォ制御をideRWから行うことでメモリ書き換え中に参照されないように修正.
//  09/04/05 takayama CQ:316760 SSD常時接続対応.
//  09/04/07 koshimo  CQ.316761 SSD標準化対応（SSDアクセス制御）.
//	09/04/21 kurachi Donau3/Amur CQ:317353対策.
//							dosFSの多重アクセスが発生した場合にbl_OSaccessFlagフラグのAssert関数で必ずAssertしてしまうため、.
//							排他制御が働いていなかった。bl_OSaccessFlagフラグはデバッグ用フラグで、他で使用しているわけ.
//							ではないためAssert関数は不要。Assert関数をはずすことにより、多重アクセスが発生したとしても.
//							AES排他セマフォ＆DOS排他セマフォで排他がかかるため問題はない。.
//  10/03/03 yashiro  Gaia問題点リストNo286(CQ324193) : セマフォID dosImageMuteSem 関連処理の削除..
//	10/07/13 iizuka   ＤＯＳとの調停処理削除 (8536)
//  13/02/14 Takatsuji	FA14のアサート文字列のリファイン
//  17/10/10 Yoneda   S800 ストレージ構成変更対応.
//	19/08/26 Sky鳥居	[EagleZ+] OP_BTS-11139対応.
// ================================================================================================
//int  CB_IDEHard::ideRW( int drive, int cylinder, int head, int sector, void *buffer, int nSecs, int direction )
#ifdef DEF_SupportSSD
int32_t  CP_HDDTrans::ideRW( char *devicePath, int32_t cylinder, int32_t head, int32_t sector, void *buffer, int32_t nSecs, int32_t direction )
#else
int32_t  CP_HDDTrans::ideRW( int32_t drive, int32_t cylinder, int32_t head, int32_t sector, void *buffer, int32_t nSecs, int32_t direction )
#endif
{
// 【課題】07/05/07 morikawa 
// この関数は、dosFS領域のアクセスとDEF_HDD_RW_CHECK_BLOCKのアクセスでしか使用されないはずなので
// dosFS領域の開始セクタ番号をTHRC_TreadifDonau.hに定義してOS領域内かあるいはDEF_HDD_RW_CHECK_BLOCKでなければアサートする処理を追加予定　

	// スリープ中のHDDアクセスをトラップ 
	// HDDデバイスに対するSleep要求直前～スリープ中～HDD起動完了(起動シーケンス完了)の間のアクセスがNG対象.
#ifdef TORNADO
	if ( (uc_Indicator_HDDSleep == 1)
	  && (SYSF_taskIdSelf() != SYSF_taskNameToId((char*)"TASK_HDDTrans")) ) {
		THR_ID taskID = SYSF_taskIdSelf();
		uint32_t tick = SYSF_tickGet() ;
		SYSF_logMsg("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n",0,0,0,0,0,0) ;
		SYSF_logMsg("CP_HDDTrans::ideRW Error(under sleep) taskID=%"PRIx32" tick=%"PRIu32"\n",taskID,tick,0,0,0,0);
		SYSF_logMsg("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n",0,0,0,0,0,0) ;
		if( debug_sleep_iderw > 0 ) {
			SYSF_taskSuspend( taskID );
		}
		setDebugLowSpeedSeq(714,taskID) ;
	}
#endif

	// 2009/04/21 kurachi CQ:317353対策により、Assert関数をコメントアウト.
	//THRL_ThAssert( bl_OSaccessFlag == False,"CP_HDDTrans::ideRW() Error Double accsess!", 0, 0, 0, 0, CFA14_DetailCode0x01BB );
	bl_OSaccessFlag = True;
	if( isCommandExec() == True ) {
		setDebugHighSpeedSeq(99, 13);
	}

	checkGSIO() ;	// Debug グロバールセマフォを握ったままのHDD操作の検出処理 2005/06/13 added by t.sasaki

// CQ.316761 HDD、SSDのセマフォ取得処理の追加.
#ifdef DEF_SupportSSD
	THRE_StorageDeviceType devicetype  = 0;
	if(strcmp(devicePath,theHDDTrans.pc_strDeviceSSD) == 0){
		devicetype = THRE_StorageDeviceTypeHDD;
	} else {
		devicetype = THRE_StorageDeviceTypeSSD;
	}

	THRC_AesTransSingletonIf::newInstance().semTakeDosDirect(devicetype);

#else // DEF_SupportSSD.
	THRC_AesTransSingletonIf::newInstance().semTakeDosDirect();
#endif // DEF_SupportSSD.

	if( us_debug_HDDdispparm>0 ) {
		setDebugHddSeq(200, 0);	// 2005/10/07 modified by t.sasaki
	}
	bl_CommandExecFlag = True;

	EC_HDDTransDirection ec_HDDTransDirection = (EC_HDDTransDirection)direction;
	// HDD書き込み読み出し転送識別を書き込みで初期化　
	THRE_ReadWriteHDDMode e_ReadWriteHDDMode = THRE_ReadWriteBufToHDD;
	if( ec_HDDTransDirection == EC_HDDTransDirection_Read ) {
		e_ReadWriteHDDMode = THRE_ReadWriteHDDToBuf;	// 読み出し
	}

	// アドレス変換を行う（シリンダ、ヘッド、セクタ）から論理アドレス（ＬＢＡ）
	uint32_t ulTopSec = convertCHSToLBA( ushort( cylinder ), uchar( head ), uchar( sector ) );
	
	FuncCallstart( 31, __FUNCTION__, 1 );		// 割込みチェック用関数コールスタート.
	// バッファアドレス 
	uintptr_t ul_WorkMemAddr = getWorkMemAddr();
	uint32_t ul_Size = DEF_SECTOR_SIZE * nSecs;
	THRL_ThAssert( ul_Size <= THRD_HDD_WORKMEM_SIZE, "CP_HDDTrans::ideRW() THRD_HDD_WORKMEM_SIZE <= ideRW TransSize=%"PRIdPTR"[Byte]\n", ul_Size, 0, 0, 0, CFA14_DetailCode0x01BC );
	uintptr_t ul_AESBufAddr = ul_WorkMemAddr + THRD_HDD_WORKMEM_SIZE;

	if( ec_HDDTransDirection == EC_HDDTransDirection_Write ) {
		if( isEnableEnc() == True ) {
		// 暗号化ありの時
	 		// 先頭奇数要求バイトバッファから先頭偶数バイトAESバッファにコピー　
	 		memcpy( (void *)ul_AESBufAddr, buffer, ul_Size );
			// 暗号化実行
// 08/09/29 morikawa 履歴参照．
			THRC_AesTransSingletonIf::newInstance().encodeDecodeExec( THRE_EncHDDDosMemToBuf, 0, (uintptr_t)ul_AESBufAddr, ul_WorkMemAddr, ul_Size, ulTopSec );
		} else {
		// 暗号化なしの時
		 	// 先頭奇数要求バイトバッファから先頭偶数バイトAESバッファにコピー　
		 	memcpy( (void *)ul_WorkMemAddr, buffer, ul_Size );
		}	
	}			

#ifdef DEF_HDD_SECTORLOG
	setsecnotbl((int32_t)ulTopSec, nSecs ) ;	// セクタログ 2005/10/04 added by t.sasaki
#endif	//#ifdef DEF_HDD_SECTORLOG

	// HDD転送実行
	// 暗号化なしの時は、ul_BufAdrはbufferと同じ
	// 暗号化あり、HDDへの書き込み時はすでに暗号化されたデータをul_AESBufAdrから読み出す。これを今からHDDへ書き込む。			
	// 暗号化あり、HDDからの読み出し時はHDDから取り出したデータを今からul_AESBufAdrに書き込む。その後に複合化する。			
// 2009/04/05   takayama CQ:316760 SSD常時接続対応.
#ifdef DEF_SupportSSD
	readWriteExecHDD( devicePath, e_ReadWriteHDDMode, 0, ul_WorkMemAddr, ulTopSec, nSecs, TRUE );
#else
	readWriteExecHDD( e_ReadWriteHDDMode, 0, ul_WorkMemAddr, ulTopSec, nSecs, TRUE );
#endif
	
	if( ec_HDDTransDirection == EC_HDDTransDirection_Read ){
		if( isEnableEnc() == True ){
			// 暗号化ありの時 
			THRC_AesTransSingletonIf::newInstance().encodeDecodeExec( THRE_EncHDDDosBufToMem, 0, (uintptr_t)ul_AESBufAddr, ul_WorkMemAddr, ul_Size, ulTopSec );
			memcpy( buffer, (void *)ul_AESBufAddr, ul_Size );
		} else {
			memcpy( buffer, (void *)ul_WorkMemAddr, ul_Size );
		}
	}
	FuncCallend( 31, __FUNCTION__, 1 );			// 割込みチェック用関数コールエンド.

	bl_CommandExecFlag = False;
	bl_OSaccessFlag = False;
	if( us_debug_HDDdispparm>0 ) {
		setDebugHddSeq(201, 0);	// 2005/10/07 modified by t.sasaki
	}

// CQ.316761 HDD、SSDのセマフォ解放処理の追加.
#ifdef DEF_SupportSSD
	THRC_AesTransSingletonIf::newInstance().semGiveDosDirect( devicetype);
#else // DEF_SupportSSD.
	THRC_AesTransSingletonIf::newInstance().semGiveDosDirect();
#endif // DEF_SupportSSD.

	return	C_OK;
}

/*================================================================================================
［クラス］	CP_HDDTrans : 制御単位「HDD転送」
//###CP_HDDTrans
================================================================================================*/
// ================================================================================================
//! <!--機能--> ＨＤＤ　デストラクタ
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param なし 
//! <!--戻値-->
//! \return なし
//! <!--詳細-->
//! \n
//! <!--履歴-->
// 2007/10/29 morikawa	Donau初版 シミュレータの時のみ初期接続時にallocするのでここで開放する 
// 2012/11/21 ADS川口  CQ.377856  MS-LCL_3 pul_BackupClusterIdxの解放処理を追加. 
// ================================================================================================
CP_HDDTrans::~CP_HDDTrans()
{
#ifdef FUM_LINUX	
	free( puc_WorkMemAddr );
#endif	
	for( THRE_BackupType e_BackupType = THRE_BackupTypeImage; e_BackupType<THRE_BackupKindNum; e_BackupType++ )
	{
		if( pul_BackupClusterIdx[e_BackupType] != NULL )
		{
			delete [] pul_BackupClusterIdx[e_BackupType];
			pul_BackupClusterIdx[e_BackupType] = NULL;
		}
	}
}

#if !defined(DEF_MinervaPreEmu) && !defined(DEF_ZeusRevCus)
extern "C"{
extern int32_t lWarpTimeLogFlg;
}
#endif

// ================================================================================================
//! <!--機能--> ＨＤＤ接続要求受信に対する処理を起動
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param なし
//! <!--戻値-->
//! \return なし
//! <!--詳細-->
//! HDDの接続シーケンスを実行する
//! 	この関数の起動タイミングは、初期起動時とTM_REQ_CHECK_HDD_DRDY（実行可能判断のチェックタイマ）のタイムアウト時
//! <!--履歴-->
//  07/03/07 morikawa Donau初版
//  07/05/14 morikawa 静的解析の不具合修正  if 文の条件式で代入を行なっていた不具合修正　
//  07/08/07 morikawa 1st基板検証の不具合修正,HDD使用可能か否かの関数の戻り値判定修正　
//  07/08/07 morikawa 直接転送用の暗号化バッファは複数必要なので１バッファ当たりのサイズ×バッファ数分確保するように修正　
//  07/10/19 morikawa 次回、DRDYチェックまでのタイマ起動は異常が発生していない時のみに行う　
//  07/10/02 morikawa 暗号化用のバッファの先頭アドレス設定追加　
//						メモリパーティションのHDD領域としてセットされている先頭から128KBをdosFS等のワーク用に使用し
//						それ以降を暗号化用バッファとして使用する　
//						シミュレータの時のバッファ確保を修正　
// 	07/11/05 morikawa malloc領域を64バイト切り上げ（64バイト加算）して、デストラクタで64バイト余分に開放していたため異常になっていた
//					  不具合修正→64バイト切り上げ処理は不要なので削除する　
// 								 代わりにこのバッファは偶数アドレスである必要があるのでこれに対するプロテクトを追加　
//	08/02/25 morikawa HDDの容量増大（160GByte→250GByte）に伴いFatサイズが増えてたため
//						HDDのバッファのDEF_HDD_FORMATDATA_SIZEを除く部分が
//						暗号化用のバッファの１MByte×２個分では足りなくなったため、FAT領域サイズ分を確保する。
//						SimWinでのHDDのバッファ容量変更（実機の場合はconfig.hの変更になる）
//							修正前：DEF_HDD_FORMATDATA_SIZE + DEF_MAX_HDD_RW_SIZE * THRD_HDDBUF_NUM
//							修正後：DEF_HDD_FORMATDATA_SIZE + DEF_FAT_TOTAL_HDD_SIZE
//					  さらに、doosFS用のテンポラリ用のワークメモリのバッファとAES用のバッファも設ける。	
// 2008/06/26 morikawa CQ.305386 スリープ復帰後も暗号キーを再設定する必要があるので暗号キー未設定状態として、ASIC200に暗号キーを再設定する必要がある
//					   このために、AES転送タスクの初期化関数のコールをスリープ復帰時の接続時にもコールするように変更
// 08/07/01 morikawa CQ.305488 HDD未接続時にHDDあり設定で発生するFatalが間違っている問題修正
//						不具合：Mosel→Donauの変更にてHDDアクセスのインターフェースが変更になり、HDDが使用可能か否かのチェックを
//							BSPのHDDステータスチェック関数で行うことになった、スレッドのこの関数にてHDDの接続チェックを行う時には
//							HDDのステータスはすでに確定しているにも関わらず、前機種のタイムアウト発生まで定期的にチェック継続するという
//							処理が残っていた。
//						対策：上記不具合が残っていたとしても、この関数が最初に実行された時から35秒後にHDDがつながっていない時はHDD未接続状態のFATAL
//							となるが、無駄な処理でFATAL検出が遅れるために不要な処理として削除する。
//							１回のみHDDステータスチェック関数でチェックを行い、このときにHDDに接続できなければタイムアウト関数を直接コールしてFATALとする。
// 09/06/02 takayama CQ:317995 HDD装着設定変更後にD047発生不具合対応.
//							   (デバイスタイプ設定処理を追加).
// 14/09/08 y.morikawa	ZeusS 制御構成変更No.4 HDDミラーリング対応.
// 16/03/17	yamada		SATAコントローラ、メンテナンスモード起動時の自動復旧対応.
// 19/07/12 Sky			[RQ-560]1TB_SSD対応により、DEF_FAT_TOTAL_SIZEの切り替え処理を追加.
// 20/12/09 Sky			OP_BTS-28730 SIMのHDDバッファサイズを実機と同等に変更.
// ================================================================================================
void CP_HDDTrans::connection()
{
	// セマフォ生成.
	SemCreateAutoRecoverySataFw();

	uint32_t		ul_FatTotalSize_local;						// FATトータルサイズ.
	part_layout*	s_DeviceInfo = THRG_GetPartitionType();		// PartitionType取得.
	if( ssd_part_info_1TB == s_DeviceInfo ) {
		ul_FatTotalSize_local = DEF_FAT_TOTAL_SIZE_1T;
	} else {
		ul_FatTotalSize_local = DEF_FAT_TOTAL_SIZE;
	}

#if !defined(DEF_MinervaPreEmu) && !defined(DEF_ZeusRevCus)
	if(lWarpTimeLogFlg == 1)
		printf("@@@HDD Initialize START = %"PRIu32"@@@\n",SYSF_tickGet());
#endif
	// HDDロックフラグクリヤ　
	bl_LockPasswordFlag = False;
#ifndef TORNADO 
#ifdef DEF_SupportSSD
	initNvInfoSSD();
#endif	/* #ifdef DEF_SupportSSD */
#endif
	if( CP_HDDTrans::bl_ConnectFlag == False ) {
		// ワークRAMとHDD間のDMAは64バイト単位のため転送用のバッファ領域をあらかじめ確保する
		// puc_WorkMemAddrはdosFS転送用のAES暗号化のバッファとしても用いる
		// バッファの初期設定は電源投入時一度だけ行う。　
		if( NULL == puc_WorkMemAddr ) {
#if (defined TORNADO) && (!defined FUM_LINUX)
			puc_WorkMemAddr = (uchar *)(((uintptr_t)pt_ALLOC_HDDBufferArea+0x1FF) & ~0x1FF);    /* alignment honda */
			if( ( (uintptr_t)puc_WorkMemAddr % 2 ) != 0 ) {
				THRL_ThAssert( False," CP_HDDTrans::connection() pt_ALLOC_HDDBufferArea Adr Not Even Error! ", 0, 0, 0, 0, CFA14_DetailCode0x01BD );	
			}
#else	// ＰＣシミュレータで有効
			// pt_ALLOC_HDDBufferAreaに合わせてSIMでも1TBサイズで確保.
			puc_WorkMemAddr = (uchar *)malloc( THRD_HDD_WORKMEM_SIZE * THRD_HDD_WORKMEM_NUM + DEF_FAT_TOTAL_SIZE_1T );
#endif // #if (defined TORNADO) && (!defined FUM_LINUX)
			if( NULL == puc_WorkMemAddr ) {
				THRL_ThAssert( False," CP_HDDTrans::connection() puc_WorkMemAddr Alloc Error! ", 0, 0, 0, 0, CFA14_DetailCode0x01BE );	
			}
		}
			uintptr_t ul_HDDBufStartAdr = 0;
			// メモリパーティションのHDD領域としてセットされている先頭から128KBをdosFS等のワーク用に使用 
			ul_HDDBufStartAdr = (uintptr_t)puc_WorkMemAddr + THRD_HDD_WORKMEM_SIZE * THRD_HDD_WORKMEM_NUM;
			THRL_ThLogMsgErr("CP_HDDTrans::connection()ul_HDDBufStartAdr=0x%"PRIxPTR"\n",ul_HDDBufStartAdr);
	
			// 直接転送に使用する暗号化有効時のバッファアドレス初期設定　
			c_HDDBuf.initBufData( ul_HDDBufStartAdr, DEF_MAX_HDD_RW_SIZE );

#if (defined TORNADO) && (!defined FUM_LINUX)
				uc_HDDbuffer = (uchar *)(ul_HDDBufStartAdr + DEF_MAX_HDD_RW_SIZE * THRD_HDDBUF_NUM);
				st_ErasePatternTbl[0].puc_PtnBuf = (uchar *)((uintptr_t)uc_HDDbuffer + ul_FatTotalSize_local);
				st_ErasePatternTbl[1].puc_PtnBuf = (uchar *)((uintptr_t)st_ErasePatternTbl[0].puc_PtnBuf + DEF_SECTOR_SIZE);
				st_ErasePatternTbl[2].puc_PtnBuf = (uchar *)((uintptr_t)st_ErasePatternTbl[1].puc_PtnBuf + DEF_SECTOR_SIZE);
#endif // #if (defined TORNADO) && (!defined FUM_LINUX)
		// 08/06/26 morikawa 履歴参照.
		// AES転送タスク初期化.
		THRC_AesTransSingletonIf::newInstance().init();
		
		CP_HDDTrans::bl_ConnectFlag = True;		// 接続中
	}
// 2009/04/05   takayama CQ:316760 SSD常時接続対応.
#ifdef DEF_SupportSSD
	// 09/06/02 takayama CQ:317995.
	APIC_HddInfo* pc_HddInfo = APIC_HddInfo::newInstance();
	THRC_HddInfoObserver* pc_HddInfoObserver = THRC_HddInfoObserver::newInstance();		// 制御構成変更No.4 HDDミラーリング対応 ＨＤＤ情報（ＡＰＩ）オブザーバー クラスの初期化.
	THRL_ThAssert( (pc_HddInfo!=NULL && pc_HddInfoObserver!=NULL), "taskHDDTrans() : HddInfo Error pHddInfo=0x%08"PRIXPTR", HddInfoObserver=0x%08"PRIXPTR"",
		(uintptr_t)pc_HddInfo, (uintptr_t)pc_HddInfoObserver , 0, 0, CFA14_DetailCode0x0D1C );

	pc_HddInfo->attachObserver( pc_HddInfoObserver );

	uchar portNo = THRE_SsdPortNo;
	THRE_StorageDeviceType uc_deviceType = THRE_StorageDeviceTypeSSD;
	if( pc_HddInfo->getHddExist() == True ){
		uc_deviceType = THRE_StorageDeviceTypeHDD;
		portNo = THRE_HddPortNo;
	}
	//2009/03/30 newInstance対応 comMemoryManagerのインスタンス生成.
	IMGC_ImageMemoryPoolImpColorFileMemoryInstance& comMemoryManager = IMGC_ImageMemoryPoolImpColorFileMemorySingletonIf::newInstance();
	// デバイスタイプの設定.
	comMemoryManager.setStorageDeviceType(uc_deviceType);
	if( THRF_sysBspHddPowerStateGet(portNo) == HDD_POWER_STATE_READY ) {
#else
	// HDD使用可能か？
	if( THRF_sysBspHddPowerStateGet(0) == HDD_POWER_STATE_READY ) {
#endif
		// HDD使用可能になった
		setHDDSeq( EM_HDDSeq_CheckInformation );			// デバイス情報読み出し実行
		stateProcExecHDDSeq();						// 初期化シーケンスを連続実行する	
	} else {
		// 08/07/01 morikawa HDD使用不可時は即FATALとする 履歴参照.
		commandTimeOut();
	}
}
/*================================================================================================
［機能］	【ＩＦ】チェックID書込要求.
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	無し
［詳細］	チェックIDがNGだが、チェックファイルによるフォーマットチェックは正常と判定された場合に呼び出す.
			※廃止した自動フォーマットでは、チェックIDを書き込まない不具合があったための実装.
［ｳｫｰｸｽﾙｰ］	実施未：
			2005/07/15 t.sasaki Rhein1/2/3初版.
			2008/05/15 kurachi Donau CQ:302954対策.
								チェックID書き込みシーケンス（Write, Read, Check）が連続実行されない
								処理になっていたため連続実行するように修正.
================================================================================================*/
void CP_HDDTrans::writeID()
{
	// シーケンスを開始.
	setHDDSeq( EM_HDDSeq_CheckID_Write );

	// シーケンスに応じた処理を連続実行.
	stateProcExecHDDSeq();
}
/*================================================================================================
［機能］	【ＩＦ】チェックID直接書き込み デバッグ用.
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	Bool 書き込み結果.
［詳細］	デバッグ用 チェックID直接書き込み処理：外部コマンドでの使用のみとする.
 [履歴]		2018/01/01 Sky井柳 [Eagle]新規作成.
================================================================================================*/
Bool CP_HDDTrans::writeID_Direct()
{
	bl_writeID_Direct = False;// 失敗で初期化.
	func_HDDSeq_FormatID();
	return bl_writeID_Direct;
}

/*================================================================================================
［機能］	【ＩＦ】フォーマット要求
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	無し
［詳細］	サービスより指示されるはず
［ｳｫｰｸｽﾙｰ］	実施済み：2001/1/10：森川
［履歴］	04/07/20 t.sasaki HDDセキュリティ対応(TSS2からの移植)
			07/03/23 Donau初版 morikawa 
			19/08/26 Sky鳥居 [EagleZ+] OP_BTS-11139対応.
================================================================================================*/
void CP_HDDTrans::format()
{
	while( getHDDSeq() != EM_HDDSeq_Idle ) {}	// シーケンス実行中は待ち状態とする

	FuncCallstart( 20, __FUNCTION__, 1 );		// 割込みチェック用関数コールスタート.
	memset( puc_WorkMemAddr, 0, DEF_HDD_FORMATDATA_SIZE );
	FuncCallend( 20, __FUNCTION__, 1 );			// 割込みチェック用関数コールエンド.

	// シーケンスを開始
	setHDDSeq( EM_HDDSeq_FormatPre );

	// シーケンスに応じた処理を連続実行
	stateProcExecHDDSeq();
	//１．欠陥セクタ交代のための、ＨＤＤ使用全領域の０クリア処理
	//・ワークメモリに１２８Ｋバイトのエリアを確保する。
	//・確保された領域に０を書き込む
	//・ＨＤＤ・ＰＣＩバス間ＤＭＡを用いてＨＤＤにワークメモリのデータを転送する。
	//・転送の単位は５１２バイトセクタを２５６回連続の１２８Ｋバイトを１単位とする。
	//・セクタは０セクタ目から書き込む
	//２．ＨＤＤのシステム領域にフォーマット済み情報の書き込み
	//"_MINOLTA"（アスキーコード）を書き込む
	//３．ＨＤＤのシステム領域の書き込みチェック
	// "_MINOLTA"が書き込まれているかを読み出してチェックする
	//４．リードライトチェックチェック機能の呼び出し
}
/*================================================================================================
［機能］	【ＩＦ】論理フォーマット要求
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	無し
［詳細］	サービスより指示されるはず
［ｳｫｰｸｽﾙｰ］	実施未：
 [履歴]		04/07/20 t.sasaki	Rhein1初版(TSS2 より移植)
			07/03/23 morikawa  Donau初版
================================================================================================*/
void CP_HDDTrans::formatOS()
{
	// シーケンスを開始
	setHDDSeq( EM_HDDSeq_Format_Logical );
	// シーケンスに応じた処理を連続実行
	stateProcExecHDDSeq();
}
/*================================================================================================
［機能］	【ＩＦ】パスワード設定
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	無し
［詳細］	パスワードの設定､変更､削除を行なう
［ｳｫｰｸｽﾙｰ］	実施未：
 [履歴]		04/07/20 t.sasaki	Rhein1初版(TSS2 より移植)
			05/09/21 tsukiji ロックされていない場合はパスワード解除は行わない(東芝HDD対応)
			06/06/01 tsukiji パスワード設定前にOS側アクセスを禁止 
			07/09/26 morikawa シーケンスが連続実行できていなかった不具合修正　
			08/03/27 kurachi CQ:301533（C-D043問題対策）.
			14/02/06 y.morikawa CQ.509898 HDDミラーリング パスワードロック問題の修正.
			18/11/02 k.yamagishi 【管理No.564】【制御_No.64】全データクリアイニシャライズ実施できない setHddSleepStart()のコールを削除.
================================================================================================*/
void CP_HDDTrans::password( uintptr_t ulOption )
{
#ifdef TORNADO
	while( getHDDSeq() != EM_HDDSeq_Idle ) {}	// シーケンス実行中は待ち状態とする
#endif
	// メッセージ
	pst_MsgSetPassword = (ST_MsgSetPassword*)ulOption;
	if( pst_MsgSetPassword!=NULL ) {
		// シーケンスを開始（Disable=>Setを続けて行いパスワードの無いものは実行されない
		if( bl_LockPasswordFlag == True ) {
			// HDDにロックパスワードが設定されている.
			if( isSecurityError() == True ){
				if ( ( True == isMirroringHDD() )
					&& ( m_e_HddMirroringDeviceStatus[THRE_HddMirroring_Chanel_A] == THRE_HddMirroringStatus_Normal )
					&& ( m_e_HddMirroringDeviceStatus[THRE_HddMirroring_Chanel_B] == THRE_HddMirroringStatus_Normal ) ) {
					// セキュリティエラーが発生している場合は.
					// ミラーリングだと片側のみパスワード設定の場合があるのでHDD&NVRAMへパスワードを設定する.
					setHDDSeq( EM_HDDSeq_SetPassword );
				}
				else {
					// セキュリティエラーが発生している場合は.
					// HDDのパスワード解除及び設定は行わず、NVRAMにのみパスワードを設定する.
					setHDDSeq( EM_HDDSeq_PasswordEnd );
				}
			}else{
				// パスワード解除.
				setHDDSeq( EM_HDDSeq_DisablePassword );
			}
		} else {
			// HDDにロックパスワードが設定されていない場合は.
			// セキュリティエラー有無に関わらずHDD&NVRAMへパスワードを設定する.
			setHDDSeq( EM_HDDSeq_SetPassword );
		}
		// シーケンスに応じた処理を連続実行
		stateProcExecHDDSeq();
	}
}

// ================================================================================================ 
//! <!--機能--> ＨＤＤの管理情報転送要求受信 
//! <!---------------------------------------------------------------------------------------------> 
//! <!--引数--> 
//! \param  ulong メッセージオプション
//! <!--戻値--> 
//! \return なし
//! <!--詳細--> 
//! \n 	画像の管理情報のＨＤＤに書き込み／読み出しを実行する
//! \n 	１管理情報は１クラスタに収まるようにすること
//! <!--履歴--> 
//  07/03/15 morikawa 新規作成 
//  07/05/14 morikawa 静的解析の不具合修正
//  07/08/20 morikawa シミュレータの時に暗号化ありのときのDMA転送をmemcpyで行うように変更になったため、
//					１：シミュレータのファイルのRead/Write、２：直接転送のHDD転送とAES転送実行の順番で行うとHDDからの読み出しの場合にuc_HDDbufferのデータが
//					書き換わってしまうため以下の順番に変更する。
//					１：直接転送のHDD転送とAES転送実行、２：シミュレータのファイルのRead/Write
//					参考：FAT転送もシミュレータ時は以前から変更後の順番で行っているので問題ない。
//  08/07/23 t.yokoyama  [CQ308783]画像管理情報の読み書きに対する書き込み開始位置を追加.
//	12/01/20 takayama CQ360543 ファイルアクセスAPIをラッパー関数に置き換え.
//	13/06/27 Sky [HDD互換性対応] 色数により画像の管理情報の保存形式を切り替える.
//  19/3/22 SCD2 伊藤史. EagleL MR1 制御構成変更No.1 BOX画像(KM形式)インポート対応（Eagle世代越リストア).
//	19/08/26 Sky鳥居 [EagleZ+] OP_BTS-11139対応.
// ================================================================================================ 
void CP_HDDTrans::startHDDTransInfo( uintptr_t ulOption )
{
	FuncCallstart( 1, __FUNCTION__, 0 );		// 割込みチェック用関数コールスタート.

	// 受けたメッセージのオプションをアドレスとしてメッセージデータアドレスへ変換する
	ST_MsgHDDTransInfo*	pst_MsgHDDTransInfo = (ST_MsgHDDTransInfo*)ulOption;
	setHDDSeq( EM_HDDSeq_HDDtrans );	// 02/05/10 tsukiji シーケンスを転送中
	
	IMGC_Image* pcc_OneInputImage = NULL;	// 06/02/28 定義はメッセージパラメータのNULLチェックの前に移動する　
	IMGC_ImageMemoryPoolImpColorFileMemoryInstance& comMemoryManager = IMGC_ImageMemoryPoolImpColorFileMemorySingletonIf::newInstance();
	
	if( pst_MsgHDDTransInfo!=NULL ) {

		_memcpy( &st_MsgHDDTransInfo, pst_MsgHDDTransInfo, sizeof( ST_MsgHDDTransInfo ) );
		pcc_OneInputImage = st_MsgHDDTransInfo.pcc_OneInputImage;
	
		setDebugLowLogSpeedSeq( 302, st_MsgHDDTransInfo.em_HDD_RWMode );	// debugLog 92に残す.
		setDebugLowLogSpeedSeq( 141, (uintptr_t)pcc_OneInputImage );	// 06/03/03 morikawa Mosel Log Imageのポインタをログに残すように変更 
	
		if( (pcc_OneInputImage != NULL)
	  	&& (( ( pcc_OneInputImage->m_IMGC_ImageImp ) )->isExistImageInfo()==True)
	  	&& (( ( pcc_OneInputImage->m_IMGC_ImageImp ) )->c_ImageDataHDDBlock.pcc_Fat != NULL)
	  	&& (( ( pcc_OneInputImage->m_IMGC_ImageImp ) )->c_ImageDataHDDBlock.judgeClusterNo() == True)
		) {
			CC_OneImageInfoData* pcc_OneImageInfoData	= pcc_OneInputImage->m_IMGC_ImageImp->pc_OneImageInfoData ;
			CC_OneImageInfoData512* pc_OneImageInfoData512	= comMemoryManager.pc_OneImageInfoData512 ;
			// 画像の管理情報を構成する色数を取得.
			uchar uc_ColorNum	= theColorInformation.getColorNum( pcc_OneInputImage, True );
			uint32_t	ul_TotalSize = 0;			// 管理情報サイズ(HDD格納形式)

			// 2019/2/28 SCD2 伊藤史. EagleL MR1 制御構成変更No.1 BOX画像(KM形式)インポート対応（Eagle世代越リストア).
			if( (NULL != pc_OneImageInfoData512) && (EM_HDD_RWMode_Read == st_MsgHDDTransInfo.em_HDD_RWMode) ){
				ul_TotalSize	= pc_OneImageInfoData512->calcBufSize() ;
			} else {
			ul_TotalSize	= pcc_OneImageInfoData->calcBufSize() ;
			}

			// 転送モードから
			uint32_t ul_Cluster = st_MsgHDDTransInfo.ul_StartClusterNo;
			uint32_t ul_Sector = 0;
			ul_Sector = ( ( pcc_OneInputImage->m_IMGC_ImageImp ) )->c_ImageDataHDDBlock.calcClusterToSector( ul_Cluster );
			// 転送管理サイズを管理クラスタ単位で求める（切り上げ）
			uint32_t ul_TransTotalSize = _roundup( ul_TotalSize , (uint32_t)DEF_CLUSTER_INFO_SIZE ) * (uint32_t)DEF_CLUSTER_INFO_SIZE;
			// 転送セクタ数を求める
			uint32_t ul_SectorNum =  ul_TransTotalSize / DEF_SECTOR_SIZE;
			// HDD書き込み読み出し転送識別を書き込みで初期化　
			THRE_ReadWriteHDDMode e_ReadWriteHDDMode = THRE_ReadWriteBufToHDD;
			switch( st_MsgHDDTransInfo.em_HDD_RWMode ) {
				case EM_HDD_RWMode_Read:	// 読み込み
					e_ReadWriteHDDMode = THRE_ReadWriteHDDToBuf;	// 読み出し
					setDebugLowSpeedSeq( 861, ul_Sector );
					break;
				case EM_HDD_RWMode_Write:	// 書き込み
					// -------------------------------------------- //
					// バンド・ブロック・カラーごとのデータセット	//
					// -------------------------------------------- //
					if ( pcc_OneImageInfoData->setToBuf( &uc_HDDbuffer[0], uc_ColorNum ) == False ){
						// 変換時エラー
						ul_TotalSize	= 0 ;
					}
					setDebugLowSpeedSeq( 862, ul_Sector );
					break;
				default:	// モード不明
					ul_TotalSize	= 0 ;
					setDebugLowSpeedSeq( 863, ul_Sector );
					break;
			}

			if( 0 < us_DebugEnable ) {
				printf("[%ld]manage transfer start[%s] OIP[0x%08"PRIXPTR"]->[0x%08"PRIXPTR"] cluster[%"PRIu32"] sector[%"PRIu32"] size[%"PRIu32"]\n", 
					SYSF_tickGet(),
					(st_MsgHDDTransInfo.em_HDD_RWMode==EM_HDD_RWMode_Read)?"Read":"Write",
					(uintptr_t)pcc_OneInputImage, (uintptr_t)( ( pcc_OneInputImage->m_IMGC_ImageImp ) )->pc_OneImageInfoData, ul_Cluster, ul_Sector, 
					ul_TransTotalSize );
			}
			_DebugSequence( (short)(st_MsgHDDTransInfo.em_HDD_RWMode + 2));	//　02/02/15 morikawa 常にログをとるように修正　　
			
			// HDD転送と暗号化有効時のみAES転送を実行する　			
			transDirectHDDAndAES( e_ReadWriteHDDMode, (uintptr_t)uc_HDDbuffer, ul_Sector, ul_SectorNum );

			if( NULL != fp_debugHDDinfo ){
				fwrite((void *)uc_HDDbuffer,ul_TransTotalSize,1,fp_debugHDDinfo);
			}

			if( 0 < us_DebugEnable ) {
				printf("[%"PRId32"]manage transfer end_size[%"PRId32"] Valid size[%"PRId32"]\n", SYSF_tickGet(), ul_TransTotalSize, ul_TotalSize);
				// 2019/2/28 SCD2 伊藤史. EagleL MR1 制御構成変更No.1 BOX画像(KM形式)インポート対応（Eagle世代越リストア).
				if( (NULL == pc_OneImageInfoData512) && (1 < us_DebugEnable) ){
					uint32_t* pul_Buf = (uint32_t*)uc_HDDbuffer;
					printf("BandNum,BlockNum,ColorNum = %d, %d, %d\n",pcc_OneImageInfoData->uc_AllocBandNum,pcc_OneImageInfoData->uc_AllocBlockNum,pcc_OneImageInfoData->uc_AllocColorNum);
					printf("TotalSize=%"PRId32", ",*pul_Buf++);
					printf("sl_FileID=%"PRId32"\n",*pul_Buf++);
					printf("[Band:Block] Color0 ul_CompressSize, Color1 ul_CompressSize ...\n");
					for( uchar ucBandNo = 0; ucBandNo < pcc_OneImageInfoData->uc_AllocBandNum; ucBandNo++ ){
						for ( uchar ucBlockNo = 0; ucBlockNo < pcc_OneImageInfoData->uc_AllocBlockNum; ucBlockNo++ ){
							printf("[%d:%d] ",ucBandNo,ucBlockNo);
							for ( uchar ucColorNo = 0; ucColorNo < pcc_OneImageInfoData->uc_AllocColorNum; ucColorNo++ ){
								printf("%8"PRId32",",*pul_Buf++);
							}
							printf("\n");
						}
					}
					printf("\n");
				}
			} 
			_DebugSequence( (short)(st_MsgHDDTransInfo.em_HDD_RWMode + 90 + 2) );	// 02/02/15 morikawa 常にログをとるように修正　
			
			switch( st_MsgHDDTransInfo.em_HDD_RWMode ) {
				case EM_HDD_RWMode_Read:	// 読み込み(HDD->SDRAM)
					// -------------------------------------------- //
					// バンド・ブロック・カラーごとのデータセット	//
					// -------------------------------------------- //
					// 2019/2/28 SCD2 伊藤史. EagleL MR1 制御構成変更No.1 BOX画像(KM形式)インポート対応（Eagle世代越リストア).
					if( NULL != pc_OneImageInfoData512 ){
						if ( pc_OneImageInfoData512->setFromBuf( &uc_HDDbuffer[0], uc_ColorNum ) == False ){
							// 変換時エラー
							ul_TotalSize	= 0 ;
						}
					} else {
					if ( pcc_OneImageInfoData->setFromBuf( &uc_HDDbuffer[0], uc_ColorNum ) == False ){
						// 変換時エラー
						ul_TotalSize	= 0 ;
					}
					}
					break ;
				case EM_HDD_RWMode_Write:	// 書き込み(SDRAM->HDD)
					break;
				default:	// モード不明
					ul_TotalSize	= 0 ;
					break;
			}
		} else {
			setDebugHighSpeedSeq( 99, 7 );		// 02/06/18 tsukiji 管理ＨＤＤデータ異常
			if( 0 < us_DebugEnable ) {
				printf( "[%"PRId32"]manage HDD transfer data error number OIP,or No FAT info,or cluster number error\n", SYSF_tickGet());
			} 
		}
	} 
	
	// 転送完了メッセージ送信　	06/02/28 関数でまとめる
	sendMsgHDDTransEnd( &st_MsgHDDTransInfo );	

	setHDDSeq( EM_HDDSeq_Idle );	// シーケンスを停止（待機状態へ）

	setDebugLowLogSpeedSeq( 241, (uintptr_t)pcc_OneInputImage );	// 06/03/03 morikawa Mosel Log Imageのポインタをログに残すように変更 

	FuncCallend( 1, __FUNCTION__, 0 );			// 割込みチェック用関数コールエンド.
}
// ================================================================================================ 
//! <!--機能--> ＨＤＤのアレイ転送要求受信 
//! <!---------------------------------------------------------------------------------------------> 
//! <!--引数--> 
//! \param  ulong メッセージオプション
//! <!--戻値--> 
//! \return なし
//! <!--詳細--> 
//! \n 	実画像のアレイ情報のＨＤＤに書き込み／読み出しを実行する
//! \n 	暗号化なし時でも、ＡＥＳのＤＭＡをスルーで用いて以下の処理を行う　
//! \n 	○HDD書き込み時
//! \n 	ワークメモリ→（8KByte毎の不連続領域）AESスルー（最大１MBの連続領域）→BSPの関数でHDDへ書き込み
//! \n 	○HDD読み出し時
//! \n 	BSPの関数でHDDから読み出し→（最大１MBの連続領域）AESスルー（8KByte毎の不連続領域）→ワークメモリ
//! <!--履歴--> 
//  07/03/15 morikawa 新規作成 
//  07/05/14 morikawa 静的解析不具合修正　
//  07/08/08 morikawa 暗号化なし時でも不連続領域から連続領域の変換のためにＡＥＳのＤＭＡをスルー設定で用いる　
//  08/01/30 morikawa debugF 14,1で有効になるデバッグ用のHDDの転送時間表示不具合修正
//  08/03/17 morikawa 問題１(CQ301366)：アレイ転送実行中の中断処理の後の再開にてHDDのセクタ番号をページの先頭位置から行なっていていた
//					　対策１          ：再開時は転送完了終了したアレイ数を元に、HDDの開始セクタ番号を補正する。
//					　問題２(CQ301366)：HDD転送中断時に、1ページ当たりの転送すべきアレイ数を全て転送完了してしまったときにシーケンスが止まる
//					　対策２          ：転送中断実行時に全てのアレイを転送完了してしまった場合は停止完了フラグをセットせずに正常終了として結果を返す。
//					　		          ：転送中断実行時にAES転送タスクの処理がすでに全て終了してしまっていた場合は、停止処理を実行せずに最後まで処理を行う。
//  08/07/23 t.yokoyama  [CQ308783]画像データの読み書きに対する書き込み開始位置を追加.
//  09/03/11 takayama Taiga CQ316073 SSD接続でFAX送信中にFAX受信して受信完了後にFAX送信Jobを削除すると、FD17が発生する問題の対策.
//  09/04/05 takayama CQ:316760 SSD常時接続対応.
//	10/09/24 世古		 [CQxxxxxx]FPGA基板時、連続領域変換実行フラグを無効にする処理、１アレイ単位で転送する処理を削除.
//	12/02/23 戸田	  ファイルメモリ分割対応.
//	13/06/17 ADS北村	CQ.381189(Act_66229) EE-SSC_4	HDDレス化対応_SSDへのスワップ対応.
//	14/03/06 toda		Zeus5BK HDDレス対応 HDD装着時にSSDへスワップする暫定対応を削除.
//	14/05/09 岡部朋(日立JTE) 次世代アーキ スワップ分割読出対応.
//  17/10/13 Yoneda     S800 ストレージ構成変更対応.
//  19/3/22 SCD2 伊藤史. EagleL MR1 制御構成変更No.1 BOX画像(KM形式)インポート対応（Eagle世代越リストア).
// ================================================================================================ 
void CP_HDDTrans::startHDDTrans( uintptr_t ulOption )
{
	uint32_t ul_StartTime = SYSF_tickGet();
	uint32_t ul_TransTotalSize = 0;
#ifdef DEF_A3A4COMM
	uchar uc_Flags = -1;		// 0=read,1=write.
#endif	// #ifdef DEF_A3A4COMM.

// 09/03/11 takayama Taiga CQ316073 SSD接続でFAX送信中にFAX受信して受信完了後にFAX送信Jobを削除すると、FD17が発生する問題の対策.
#ifdef DEF_SupportSSD
	uint32_t ul_OneTransSectorNum = 0;
	uint32_t ul_MaxRwArrayNum = 0;
	
    if(((THRE_StorageDeviceTypeSSD & getNvRamDeviceType()) != 0)  && 
       (theHDDTrans.e_StartUpDeviceType == THRE_StartupDeviceType_SD)) {
		ul_OneTransSectorNum = DEF_MAX_SSD_RW_SECTOR_NUM;
		ul_MaxRwArrayNum = DEF_MAX_SSD_RW_ARRAY_NUM;
	} else {
		ul_OneTransSectorNum = DEF_MAX_HDD_RW_SECTOR_NUM;
		ul_MaxRwArrayNum = DEF_MAX_HDD_RW_ARRAY_NUM;
	}
#endif
	setHDDSeq( EM_HDDSeq_HDDtrans );	// シーケンスを転送中

	// 受けたメッセージのオプションをアドレスとしてメッセージデータアドレスへ変換する
	ST_MsgHDDTransInfo*	pst_MsgHDDTransInfo = (ST_MsgHDDTransInfo*)ulOption;
	IMGC_Image* pc_Image = NULL ;	// 06/02/28 定義はメッセージパラメータのNULLチェックの前に移動する　

	if( pst_MsgHDDTransInfo != NULL ) {
		// メッセージの内容をローカルへ保存
		_memcpy( &st_MsgHDDTransInfo, pst_MsgHDDTransInfo, sizeof( ST_MsgHDDTransInfo ) );
		// 
		pc_Image = st_MsgHDDTransInfo.pcc_OneInputImage;

		// debugLog 92に残す.
		setDebugLowLogSpeedSeq( 303, st_MsgHDDTransInfo.em_HDD_RWMode );	

		// Donau固有、連続領域変換実行フラグを有効に設定　
		Bool b_IsChangeContArea = True;

		if ( (pc_Image != NULL)
	  	 && (pc_Image->m_IMGC_ImageImp->c_HDDBlock.pcc_Fat != NULL)
	  	 && (pc_Image->m_IMGC_ImageImp->c_HDDBlock.judgeClusterNo()==True) ) {

	//2009/03/30 newInstance対応 comMemoryManagerのインスタンス生成.
	IMGC_ImageMemoryPoolImpColorFileMemoryInstance& comMemoryManager = IMGC_ImageMemoryPoolImpColorFileMemorySingletonIf::newInstance();

			setDebugLowLogSpeedSeq( 142, (uintptr_t)pc_Image );	
			setDebugLowLogSpeedSeq( 162, pst_MsgHDDTransInfo->ul_EndArrayNum );	
			
//////////////////////////////////////////////////////
// HDDに転送すべきモードを決定
//////////////////////////////////////////////////////
			THRE_ReadWriteHDDMode e_ReadWriteHDDMode = THRE_ReadWriteArrayToHDD;
			const uint32_t ul_TopCluster = st_MsgHDDTransInfo.ul_StartClusterNo;	// 固定値
			const uint32_t ul_TopSector = ( ( pc_Image->m_IMGC_ImageImp ) )->c_HDDBlock.calcClusterToSector( ul_TopCluster );
			uint32_t ul_EndArrayNum = pst_MsgHDDTransInfo->ul_EndArrayNum;	// 転送終了したアレイ数初期化 
			Bool  b_Stop = False;			// 転送中断フラグ初期化　
			THRE_EncHDDMode e_EncHDDMode = THRE_EncHDDNone;	// HDD転送暗号化識別初期化	
			
			_DebugSequence( st_MsgHDDTransInfo.em_HDD_RWMode );	
			switch( st_MsgHDDTransInfo.em_HDD_RWMode ) {
				case EM_HDD_RWMode_Read:
					if( b_IsChangeContArea == True ) {
						e_ReadWriteHDDMode = THRE_ReadWriteHDDToBuf;
						e_EncHDDMode = THRE_EncHDDBufToArray;
					} else {
						e_ReadWriteHDDMode = THRE_ReadWriteHDDToArray;
					}
					setDebugLowSpeedSeq( 864, ul_TopSector );
					break;
				case EM_HDD_RWMode_Write:
					if( b_IsChangeContArea == True ) {
						e_ReadWriteHDDMode = THRE_ReadWriteBufToHDD;
						e_EncHDDMode = THRE_EncHDDArrayToBuf;
					} else {
						e_ReadWriteHDDMode = THRE_ReadWriteArrayToHDD;
						
					}
					setDebugLowSpeedSeq( 865, ul_TopSector );
					break;
				default:
					setDebugLowSpeedSeq( 866, ul_TopSector );
					break;
			}
			if( 0 < us_DebugEnable ) {
				if( NULL == (uintptr_t)( comMemoryManager.pc_OneImageInfoData512 ) ){
				printf("[%ld]Transfer start[%s] OIP[0x%08"PRIXPTR"]->[0x%08"PRIXPTR"] cluster[%"PRIu32"] sector[%"PRIu32"] size[%"PRIu32"]\n", 
					SYSF_tickGet(),
					(st_MsgHDDTransInfo.em_HDD_RWMode==EM_HDD_RWMode_Read)?"Read":"Write",
					(uintptr_t)pc_Image, (uintptr_t)( ( pc_Image->m_IMGC_ImageImp ) )->pc_OneImageInfoData, 
					ul_TopCluster, ul_TopSector, comMemoryManager.getImageSize(pc_Image)  );
			}
			}

//////////////////////////////////////////////////////
// HDD転送前処理
//////////////////////////////////////////////////////
// 転送すべき、符号メモリ上のアレイアドレスをあらじめ獲得しておく。
			// 念のために初期化　　
			memset( ul_TransArrayAdr, 0, sizeof( ul_TransArrayAdr ) );
#ifdef DEF_Soc1200	// 2013/07/16 ADS萩原 CQ.381171(Act_66426) SC-SFM-26 サブメモリのファイルメモリ化.
			memset( e_ArrayType, 0, sizeof( e_ArrayType ) );
#endif	// DEF_Soc1200.
			// ページ情報からアレイアドレスと総アレイ個数を取得
			uint32_t ul_TransArrayNum = getAndSetHDDArrayInfo( pc_Image );
			uint32_t ul_Sector = 0;		// HDDアクセスセクタ番号　

			// 先頭セクタ番号からのオフセット初期値決定、再開を考慮して転送完了したアレイ数を元に計算.
			uint32_t ul_OffsetSector = ul_EndArrayNum * THRD_ONEARRAYSIZE / DEF_SECTOR_SIZE;	

			ul_TransTotalSize = ul_TransArrayNum * THRD_ONEARRAYSIZE;	// デバッグ用.

#ifdef DEF_A3A4COMM
			if( (THRE_EncHDDArrayToBuf == e_EncHDDMode) ||
				(THRE_EncHDDDirectMemToBuf == e_EncHDDMode) ||
				(THRE_EncHDDDosMemToBuf == e_EncHDDMode) ){
				uc_Flags = HDDA_WRITE;
			} else if( (THRE_EncHDDBufToArray == e_EncHDDMode) ||
				(THRE_EncHDDDirectBufToMem == e_EncHDDMode) ||
				(THRE_EncHDDDosBufToMem == e_EncHDDMode) ){
				uc_Flags = HDDA_READ;
			}
	  	 	setHddAStart( (uint32_t)-1, ul_TransTotalSize,uc_Flags, 6 );		//6:HDst.
#endif	// #ifdef DEF_A3A4COMM.
	
// 09/03/11 takayama Taiga CQ316073 SSD接続でFAX送信中にFAX受信して受信完了後にFAX送信Jobを削除すると、FD17が発生する問題の対策.
#ifdef DEF_SupportSSD
			// 最後の引数で転送再開時のアレイ個数を示す　
#ifdef DEF_Soc1200	// 2013/07/16 ADS萩原 CQ.381171(Act_66426) SC-SFM-26 サブメモリのファイルメモリ化.
			syncSendMsgExecHDD( e_EncHDDMode,
								ul_TransArrayAdr,
								False,		// アレイアドレスは複数からなる　
								ul_TransArrayNum,
								ul_MaxRwArrayNum,
								ul_EndArrayNum,
								False, NULL, 0,
								b_IsChangeContArea,e_ArrayType );
#else	// DEF_Soc1200.
			syncSendMsgExecHDD( e_EncHDDMode,
								ul_TransArrayAdr,
								False,		// アレイアドレスは複数からなる　
								ul_TransArrayNum,
								ul_MaxRwArrayNum,
								ul_EndArrayNum,
								False, NULL, 0,
								b_IsChangeContArea );
#endif	// DEF_Soc1200.
#else
							// 最後の引数で転送再開時のアレイ個数を示す　
			syncSendMsgExecHDD( e_EncHDDMode,
								ul_TransArrayAdr,
								False,		// アレイアドレスは複数からなる　
								ul_TransArrayNum,
								DEF_MAX_HDD_RW_ARRAY_NUM,
								ul_EndArrayNum,
								False, NULL, 0,
								b_IsChangeContArea );

#endif	
								
//////////////////////////////////////////////////////
// HDD転送実行開始
//////////////////////////////////////////////////////
			while( True ) {
				if( comMemoryManager.isStopHddTrans() == True ) {
					setDebugLowLogSpeedSeq( 182, (uintptr_t)pc_Image );	
					if( b_IsChangeContArea == True ) {
						// 停止要求とAES処理との同期のためにHDDからの読み出し時は読み出した分はAES実行するように停止個数を設定.
						Bool b_Ans = False;
						b_Ans = THRC_AesTransSingletonIf::newInstance().reqStopAes( ul_EndArrayNum );
						if( b_Ans == True ) {
							b_Stop = True;	// 中断発生　
							break;
						}
					}	
				}
// 09/03/11 takayama Taiga CQ316073 SSD接続でFAX送信中にFAX受信して受信完了後にFAX送信Jobを削除すると、FD17が発生する問題の対策.
#ifdef DEF_SupportSSD
				if( ul_TransArrayNum < ul_MaxRwArrayNum ) {
				 	// 転送すべきアレイ数が１回の最大個数より小さい時、転送セクタ数補正
					ul_OneTransSectorNum = ul_TransArrayNum * THRD_ONEARRAYSIZE / DEF_SECTOR_SIZE;
				} else if( (ul_TransArrayNum - ul_EndArrayNum) <  ul_MaxRwArrayNum ) {
				 	// 転送すべきアレイ数が最後の端数の時
					ul_OneTransSectorNum = (ul_TransArrayNum - ul_EndArrayNum) * THRD_ONEARRAYSIZE / DEF_SECTOR_SIZE;
				}
#else
				// １回のDMAで転送するセクタ数初期化
				uint32_t ul_OneTransSectorNum = DEF_MAX_HDD_RW_SECTOR_NUM;
				
				if( ul_TransArrayNum < DEF_MAX_HDD_RW_ARRAY_NUM ) {
				 	// 転送すべきアレイ数が１回の最大個数より小さい時、転送セクタ数補正
					ul_OneTransSectorNum = ul_TransArrayNum * THRD_ONEARRAYSIZE / DEF_SECTOR_SIZE;
				} else if( (ul_TransArrayNum - ul_EndArrayNum) <  DEF_MAX_HDD_RW_ARRAY_NUM ) {
				 	// 転送すべきアレイ数が最後の端数の時
					ul_OneTransSectorNum = (ul_TransArrayNum - ul_EndArrayNum) * THRD_ONEARRAYSIZE / DEF_SECTOR_SIZE;
				}
#endif
				// HDD転送の先頭セクタ番号取得
				ul_Sector = ( ( pc_Image->m_IMGC_ImageImp ) )->c_HDDBlock.calcRealSector( ul_OffsetSector );
				// 暗号化ありの時のバッファアドレス獲得
				uintptr_t ul_MemAdr = 0;
				switch( e_ReadWriteHDDMode ) {
					case THRE_ReadWriteBufToHDD:
						ul_MemAdr = c_HDDBuf.getReadAdr();	// 暗号化で書かれたものを読み出し　
						break;
					case THRE_ReadWriteHDDToBuf:
						ul_MemAdr = c_HDDBuf.getWriteAdr();	// HDDから読み出してまず書き込む　
						break;
					default:
						break;
				}	
				
				// 暗号化ありを考慮したスタート前処理実行
				syncStartOneExecHDD( b_IsChangeContArea ); 		
// 2009/04/05   takayama CQ:316760 SSD常時接続対応.
#ifdef DEF_SupportSSD
				readWriteExecHDD( c_DevicePath, e_ReadWriteHDDMode, &ul_TransArrayAdr[ul_EndArrayNum], ul_MemAdr, ul_Sector, ul_OneTransSectorNum );
				if( us_DebugSplitTransEnable > 0 ){
					if( st_MsgHDDTransInfo.em_HDD_RWMode == EM_HDD_RWMode_Read ){
						printf("[%"PRId32"] HDD Read ul_Sector[%"PRIu32"] ul_OneTransSectorNum[%"PRIu32"] pc_Image[%p] b_SplitTrans[%d]\n",SYSF_tickGet(),ul_Sector, ul_OneTransSectorNum, pc_Image, pc_Image->c_ImageInfo.b_SplitTrans);
					}else{
						printf("[%"PRId32"] HDD Write ul_Sector[%"PRIu32"] ul_OneTransSectorNum[%"PRIu32"] pc_Image[%p] b_SplitTrans[%d]\n",SYSF_tickGet(),ul_Sector, ul_OneTransSectorNum, pc_Image, pc_Image->c_ImageInfo.b_SplitTrans);
					}
				}
#else
				// HDD転送実行
				readWriteExecHDD( e_ReadWriteHDDMode, &ul_TransArrayAdr[ul_EndArrayNum], ul_MemAdr, ul_Sector, ul_OneTransSectorNum );
#endif

				if( ( EM_HDD_RWMode_Read == st_MsgHDDTransInfo.em_HDD_RWMode ) && (True == pc_Image->c_ImageInfo.b_SplitTrans) ) {
					// 読出時かつスワップ分割時.
					// 転送サイズとクラスタサイズから転送必要FAT数を求める.
					// ul_OneTransSectorNum * DEF_SECTOR_SIZE = 一回で転送する画像サイズ.
					ul_TransFatNum += _roundup( ul_OneTransSectorNum * DEF_SECTOR_SIZE, pc_Image->m_IMGC_ImageImp->c_HDDBlock.pcc_Fat->getClusterSize() );
				}

				// 暗号化ありを考慮した終了後処理実行
				syncEndOneExecHDD( b_IsChangeContArea ); 		
				
				// 転送完了したアレイ数
				ul_EndArrayNum += ul_OneTransSectorNum * DEF_SECTOR_SIZE / THRD_ONEARRAYSIZE;
				// セクタ相対番号加算　 
				ul_OffsetSector += ul_OneTransSectorNum;
				
				if( ul_EndArrayNum >= ul_TransArrayNum ) {
					if(us_DebugSplitTransEnable > 0 ){
						printf("[%"PRId32"] ul_TransArrayNum[%"PRIu32"] ul_EndArrayNum[%"PRIu32"] ul_TransFatNum[%"PRIu32"] pc_Image[%p]\n",SYSF_tickGet() ,ul_TransArrayNum, ul_EndArrayNum,ul_TransFatNum, pc_Image);
					}
					if( ( EM_HDD_RWMode_Read == st_MsgHDDTransInfo.em_HDD_RWMode ) && (True == pc_Image->c_ImageInfo.b_SplitTrans) ) {
						// 必要FAT数分、FATテーブルリンクを辿り、次回読み出し位置のFATを求める.
						for( uint32_t cnt = 0; cnt < ul_TransFatNum; cnt++ ){
							uint32_t ul_StartFat = pc_Image->m_IMGC_ImageImp->c_HDDBlock.pcc_Fat->next( pc_Image->m_IMGC_ImageImp->c_HDDBlock.getStartCluster() );
							// 読出開始FAT位置の更新.
							pc_Image->m_IMGC_ImageImp->c_HDDBlock.setStartCluster( ul_StartFat );
							if(us_DebugSplitTransEnable > 0 ){
								printf("[%"PRId32"] ul_StartFat[%"PRIu32"] pc_Image[%p] \n",SYSF_tickGet(),ul_StartFat, pc_Image);
							}
						}
					}
					// 転送予定数終了　
					if( EM_HDD_RWMode_Read == st_MsgHDDTransInfo.em_HDD_RWMode ){
						ul_TransFatNum = 0;
					}
					break;
				}
			}
	  	 	
			if( NULL != fp_debugHDDimage ){
				uint32_t  ul_lo_EndNum = pst_MsgHDDTransInfo->ul_EndArrayNum;	//ul_EndArrayNum;
				for( uint32_t ul_lono=0; ul_lono< ul_TransArrayNum; ul_lono++ ){
					fwrite((void *)ul_TransArrayAdr[ul_lo_EndNum++],THRD_ONEARRAYSIZE,1,fp_debugHDDimage);
				}
			}

//////////////////////////////////////////////////////
// HDD転送実行終了
//////////////////////////////////////////////////////

			// 暗号化ありを考慮した全終了後処理実行
			syncEndAllExecHDD( e_ReadWriteHDDMode, b_IsChangeContArea ); 		
			
			// 転送すべきアレイ数と転送完了したアレイ数が一致する時はたとえ停止要求が発生していても停止要求フラグは解除して応答を返す.
			if( ul_TransArrayNum == ul_EndArrayNum ) {
				b_Stop = False;
			}	
			// 転送完了メッセージ送信
			sendMsgHDDTransEnd( &st_MsgHDDTransInfo, b_Stop, ul_EndArrayNum );	

		} else {
			setDebugHighSpeedSeq( 99, 8 );		// 02/06/18 tsukiji 画像ＨＤＤデータ異常
			if( 0 < us_DebugEnable ) {
				printf( "[%"PRId32"]HDD transfer data error number OIP,or No FAT info,or cluster number error\n", SYSF_tickGet() );
			}	
		}
		if( 0 < us_DebugEnable ) {
			printf("[%"PRId32"]Transfer end_size[%"PRId32"] \n", SYSF_tickGet(), ul_TransTotalSize );
		} 
		_DebugSequence( (short)(st_MsgHDDTransInfo.em_HDD_RWMode + 90) );	// 02/02/15 morikawa 常にログをとるように修正　　
#ifdef DEF_A3A4COMM
	  	 setHddAStart( (uint32_t)-1, ul_TransTotalSize,uc_Flags, 7 );		//7:HDed.
#endif	// #ifdef DEF_A3A4COMM.
	}

	setDebugLowLogSpeedSeq( 242, (uintptr_t)pc_Image );	
	
	setHDDSeq( EM_HDDSeq_Idle );	// シーケンスを停止（待機状態へ）

	uint32_t ul_EndTime = SYSF_tickGet();		// 02/04/02 tsukiji 終了時間取得
	uint32_t ul_Time = ul_EndTime - ul_StartTime ;
	if ( ul_debug_Time_startHDDTrans < ul_Time ) {
		ul_debug_Time_startHDDTrans = ul_Time ;
	}
	if( us_debug_HDDTime > 0 ) {
		float fTransSpeed = ((float)ul_TransTotalSize / (float)(1024.0f*1024.0f)) / (((float) (ul_EndTime - ul_StartTime) * 5)/1000.0f);
		printf( "HDD Image %s Transport Time = [%"PRIu32"]ms [%"PRIu32"]byte [%f]MB/s\n",
			(st_MsgHDDTransInfo.em_HDD_RWMode==EM_HDD_RWMode_Read)?"Read":"Write",
			(ul_EndTime - ul_StartTime) * 5, ul_TransTotalSize, fTransSpeed );
	}
}
/*================================================================================================
［機能］	【受信】転送タイムアウト
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	無し
［ｳｫｰｸｽﾙｰ］	実施済み：2001/1/10：森川
［履歴］	07/10/19 morikawa DonauではBSPHDDアクセス関数内でタイムアウト検出するのでこの処理は不要　
						Mosel共通化のためにとりあえずおいておく
================================================================================================*/
void CP_HDDTrans::transTimeOut()
{
	THRL_ThAssert( False, "CP_HDDTrans::transTimeOut() Donau Error !!\n", 0, 0, 0, 0, CFA14_DetailCode0x01BF );
}

/*================================================================================================
［機能］	【受信】コマンドタイムアウト
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	無し
［詳細］	コマンドの完了で次のシーケンス起動
［ｳｫｰｸｽﾙｰ］	実施済み：2001/1/10：森川
［履歴］	02/01/08　morikawa　この関数は初期HDD接続時のDRDYチェックのタイムアウト発生時のみ使用するように修正
			04/07/20 t.sasaki HDDセキュリティ対応(TSS2からの移植)
================================================================================================*/
void CP_HDDTrans::commandTimeOut()
{
	/* 初期接続異常 */
	THRL_ThLogMsgErr( "<< HDD CONNECT_ERROR >>\n" );

	setDebugHddSeq(99, 0);	// 2005/10/07 modified by t.sasaki
	sendStatusNotice( (uint32_t)HDD_CONNECT_ERROR );	
}
/*================================================================================================
［機能］	【受信】カウントアップ
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	無し
［詳細］	
［ｳｫｰｸｽﾙｰ］	
［履歴］	04/07/20 t.sasaki HDDセキュリティ対応(TSS2より移植)
 			07/10/19 morikawa Donau初版
 								装置消去実行中はHDD転送タスクは動けないため、表示更新のためのメッセージはイメージタスクへ送信する。
================================================================================================*/
void CP_HDDTrans::countUp()
{
	if( ul_Percent < 99 ) {
		ul_Percent++;
		sendStatusNotice( ul_Percent );		// 進行パーセンテージ送信
		// 容量(40GB)に応じたカウントアップ速度
		setDelayMessage( TID_Image, TM_HDDtest, 1, DEF_FORMAT_PERCENT_TIME );
	}
}

// ================================================================================================
//! <!--機能--> シーケンスに応じた処理を連続実行
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param なし
//! <!--戻値-->
//! \return なし
//! <!--詳細-->
//! execHDDSeq()を連続して実行する
//! <!--履歴-->
//  07/03/11 morikawa	Donau初版
// ================================================================================================
void CP_HDDTrans::stateProcExecHDDSeq()
{
	while( True ) {	// 動作中の間ループ
		execHDDSeq();	// シーケンスのステート処理
		if( CP_HDDTrans::em_HDDSeq == EM_HDDSeq_Idle ) {
			break;	// ステート処理終了
		}
	}
}

// ================================================================================================
//! <!--機能--> データ表示.
//! <!--------------------------------------------------------------------------------------------->
//! <!--履歴-->
//  13/01/10 N.suzuki CQ.376010 FA14発生時のログ強化.
static void hdump(const char *title, const void *vp, int32_t siz)
{
	const unsigned char *ptr = (const unsigned char *)vp;
	int32_t i,lf;

	if ( title == NULL ) {
		title = "";
	}
	printf("%s %"PRId32" bytes = ",title,siz);
	if ( ptr == NULL ) {
		return;
	}
	for ( i = lf = 0 ; i < siz ; i++ ) {
		lf = 0;
		printf("%02X ",ptr[i]);
		if ( ((i+1) % 256) == 0 ) {
			printf("\n");
			lf = 1;
		}
	}
	if ( !lf )
		printf("\n");
}

// ================================================================================================
//! <!--機能--> データ表示.
//! <!--------------------------------------------------------------------------------------------->
//! <!--履歴-->
//  13/01/10 N.suzuki CQ.376010 FA14発生時のログ強化.
static void dumpArrayDBG(const CC_Array *pc_ArrayInfo)
{
	CC_Array *pc = (CC_Array*)pc_ArrayInfo; //メンバ関数がconstでないので無理cast
	printf("class CC_Array = %p\n",pc_ArrayInfo);
	if ( pc_ArrayInfo == NULL ) {
		return;
	}
	printf("getListNum()   = %ld\n",   pc->getListNum() );
	printf("getMemSize()   = %"PRId32"\n",   pc->getMemSize() );
	printf("getStartAddr() = %p\n" ,   pc->getStartAddr() );
	printf("getLastAddr()  = %p\n" ,   pc->getLastAddr() );
	printf("getLinkAddr()  = %p\n" ,   pc->getLinkAddr() );
	printf("class CC_Array end\n");
}

// ================================================================================================
//! <!--機能--> データ表示
//! <!--------------------------------------------------------------------------------------------->
//! <!--履歴-->
//  13/01/10 N.suzuki CQ.376010 FA14発生時のログ強化.
static void dumpParaSizeDBG(const char *title , const TYPC_PaperSize *pc)
{
	printf("class TYPC_PaperSize = %p\n",pc);
	if ( pc == NULL ) {
		return;
	}
	if ( title == NULL ) {
		title = "";
	}
    printf("%s e_SizeCode = %d c_SizeMM.x = %f c_SizeMM.y = %f b_WideOn = %d\n",
    	title,
		pc->e_SizeCode,
		pc->c_SizeMM.x,
		pc->c_SizeMM.y,
		pc->b_WideOn
    );
	printf("class TYPC_PaperSize end\n");
}


// ================================================================================================
//! <!--機能--> データ表示
//! <!--------------------------------------------------------------------------------------------->
//! <!--履歴-->
//  13/01/10 N.suzuki CQ.376010 FA14発生時のログ強化.
static void dumpImageInfoDBG(const IMGC_ImageInfo *pc_ImageInfo)
{
	printf("class IMGC_ImageInfo = %p\n",pc_ImageInfo);
	if ( pc_ImageInfo == NULL ) {
		return;
	}
	printf("ul_ImageProcessID               = %"PRId32"\n",	pc_ImageInfo->ul_ImageProcessID );
	printf("e_ImageType                     = %d\n" ,	pc_ImageInfo->e_ImageType			);
	dumpParaSizeDBG("ct_DocSize       ",&pc_ImageInfo->ct_DocSize);
	dumpParaSizeDBG("ct_ImageSize     ",&pc_ImageInfo->ct_ImageSize);
    printf("ct_ImageDotSize  c_SizeMM.x = %"PRId32" c_SizeMM.y = %"PRId32"\n" , pc_ImageInfo->ct_ImageDotSize.x , pc_ImageInfo->ct_ImageDotSize.y);
    printf("ct_TransSize     c_SizeMM.x = %d c_SizeMM.y = %d\n" , pc_ImageInfo->ct_TransSize.x    , pc_ImageInfo->ct_TransSize.y);
	printf("e_ImageDir                      = %d\n" ,	pc_ImageInfo->e_ImageDir			);
	printf("e_ImageDir_FAX                  = %d\n" ,	pc_ImageInfo->e_ImageDir_FAX		);
	printf("e_DocLP                         = %d\n" ,	pc_ImageInfo->e_DocLP				);
	printf("e_DocLP_FAX                     = %d\n" ,	pc_ImageInfo->e_DocLP_FAX			);
	//printf("c_Resolution                    = %d\n" ,   pc_ImageInfo->c_Resolution		  );//all private
	printf("ec_ColorMode                    = %d\n" ,	pc_ImageInfo->ec_ColorMode			);
	printf("b_Binary                        = %d\n" ,	pc_ImageInfo->b_Binary				);
	printf("ec_ImageColorOrg                = %d\n" ,	pc_ImageInfo->ec_ImageColorOrg		);
	printf("ec_ImageColorResult             = %d\n" ,	pc_ImageInfo->ec_ImageColorResult	);
	printf("ec_ImageTone                    = %d\n" ,	pc_ImageInfo->ec_ImageTone			);
	printf("ec_ImageCompressOrg             = %d\n" ,	pc_ImageInfo->ec_ImageCompressOrg	);
	printf("ec_ImageCompressResult          = %d\n" ,	pc_ImageInfo->ec_ImageCompressResult);
	printf("e_FileFormat                    = %d\n" ,	pc_ImageInfo->e_FileFormat			);
	printf("uc_BandDivNum                   = %d\n" ,	pc_ImageInfo->uc_BandDivNum 		);
	printf("ss_BandLineBasic                = %d\n" ,	pc_ImageInfo->ss_BandLineBasic		);
	printf("LinkCounter                     = %d\n" ,	pc_ImageInfo->LinkCounter			);
	printf("uc_ColorNum                     = %d\n" ,	pc_ImageInfo->uc_ColorNum			);
	hdump("uc_PlaneNum",pc_ImageInfo->uc_PlaneNum,sizeof(pc_ImageInfo->uc_PlaneNum));
	printf("bl_ImageLock                    = %d\n",	pc_ImageInfo->bl_ImageLock);
	printf("ec_ImagePlace                   = %d\n",	pc_ImageInfo->ec_ImagePlace);
    printf("ct_ReadMag     c_SizeMM.x = %f c_SizeMM.y = %f\n" , pc_ImageInfo->ct_ReadMag.x , pc_ImageInfo->ct_ReadMag.y);
	printf("ss_TopMargineLineNumber         = %d\n",	pc_ImageInfo->ss_TopMargineLineNumber);
	printf("ss_BottomMargineLineNumber      = %d\n",	pc_ImageInfo->ss_BottomMargineLineNumber);
	printf("ss_LeftMargineLineNumber        = %d\n",	pc_ImageInfo->ss_LeftMargineLineNumber );
	printf("ss_RightMargineLineNumber       = %d\n",	pc_ImageInfo->ss_RightMargineLineNumber);
	printf("bl_LossyCompress                = %d\n",	pc_ImageInfo->bl_LossyCompress);
	printf("bl_MemoryReserved               = %d\n",	pc_ImageInfo->bl_MemoryReserved );
	printf("bl_TransPixel                   = %d\n",	pc_ImageInfo->bl_TransPixel );
	printf("e_OriginalSetting_FAX           = %d\n",	pc_ImageInfo->e_OriginalSetting_FAX);
	printf("FileID                          = %"PRId32"\n",	pc_ImageInfo->FileID);
	printf("bl_TransPixelAtr                = %d\n",	pc_ImageInfo->bl_TransPixelAtr);
	printf("c_PageMemoryPointer             = %p\n",	pc_ImageInfo->c_PageMemoryPointer);
	printf("c_PageMemorySize                = %ld\n",	pc_ImageInfo->c_PageMemorySize );
	printf("e_CompressEndMode               = %d\n",	pc_ImageInfo->e_CompressEndMode);
    printf("ct_RotationTransSize      c_SizeMM.x = %d c_SizeMM.y = %d\n" , pc_ImageInfo->ct_RotationTransSize .x , pc_ImageInfo->ct_RotationTransSize .y);
	printf("PageMemoryPointerOffset         = %ld\n",	pc_ImageInfo->PageMemoryPointerOffset);
	printf("PageMemoryOffsetPlaceSize       = %ld\n",	pc_ImageInfo->PageMemoryOffsetPlaceSize);
	printf("uc_BandDivNumAfter              = %d\n",	pc_ImageInfo->uc_BandDivNumAfter);
	printf("uc_BandDivLineBasicAfter        = %d\n",	pc_ImageInfo->uc_BandDivLineBasicAfter );
    printf("ct_RotaionImageSize      c_SizeMM.x = %d c_SizeMM.y = %d\n" , pc_ImageInfo->ct_RotaionImageSize .x , pc_ImageInfo->ct_RotaionImageSize .y);
	printf("e_RotationImageDir              = %d\n",	pc_ImageInfo->e_RotationImageDir);
	printf("bl_NetDirOn                     = %d\n",	pc_ImageInfo->bl_NetDirOn);
	printf("e_RotationNetTxDir              = %d\n",	pc_ImageInfo->e_RotationNetTxDir);
	printf("bl_NetScanPageEnd               = %d\n",	pc_ImageInfo->bl_NetScanPageEnd);
	printf("us_NetScanImageID               = %d\n",	pc_ImageInfo->us_NetScanImageID);
	printf("ul_NetScanJobId                 = %d\n",	pc_ImageInfo->ul_NetScanJobId);
	printf("ss_FaxTopMargineLineNumber      = %d\n",	pc_ImageInfo->ss_FaxTopMargineLineNumber);
	printf("ss_FaxBottomMargineLineNumber   = %d\n",	pc_ImageInfo->ss_FaxBottomMargineLineNumber);
	printf("ss_FaxLeftMargineLineNumber     = %d\n",	pc_ImageInfo->ss_FaxLeftMargineLineNumber);
	printf("ss_FaxRightMargineLineNumber    = %d\n",	pc_ImageInfo->ss_FaxRightMargineLineNumber);
	//printf("c_FaxResolution                 = %d\n",	  pc_ImageInfo->c_FaxResolution);
	printf("e_OriginalImageType             = %d\n",	pc_ImageInfo->e_OriginalImageType);
	//printf("getImageMarginDot()             = %d\n",	  pc_ImageInfo->getImageMarginDot());
	//printf("getImageMarginMM()              = %f\n",	  pc_ImageInfo->getImageMarginMM());
	//printf("getImageSizeDot()               = %d\n",	  pc_ImageInfo->getImageSizeDot());
	//printf("getImageSizeMM()                = %f\n",	  pc_ImageInfo->getImageSizeMM());
	//printf("getFaxImageMarginDot()          = %d\n",	  pc_ImageInfo->getFaxImageMarginDot());
	//printf("getFaxImageMarginMM()           = %f\n",	  pc_ImageInfo->getFaxImageMarginMM());
	//printf("getFaxImageSizeDot()            = %d\n",	  pc_ImageInfo->getFaxImageSizeDot());
	//printf("getFaxImageSizeMM()             = %f\n",	  pc_ImageInfo->getFaxImageSizeMM());
	//printf("getFaxImageDirection()          = %d\n",	  pc_ImageInfo->getFaxImageDirection());
	//printf("getFaxOutputResolution()        = %d\n",	  pc_ImageInfo->getFaxOutputResolution());
	printf("class IMGC_ImageInfo end\n");
}

// ================================================================================================
//! <!--機能--> データ表示.
//! <!--------------------------------------------------------------------------------------------->
//! <!--履歴-->
//	13/01/10 N.suzuki CQ.376010 FA14発生時のログ強化.
static void dumpHDDBlockDB(const CC_HDDBlock *pc_hddb)
{
	printf("class CC_HDDBlock = %p\n",pc_hddb);
	if ( pc_hddb == NULL ) {
		return;
	}
	printf("ec_HDDRequestKind   = %d\n" 	,pc_hddb->ec_HDDRequestKind);
	printf("ec_IPEType          = %d\n" 	,pc_hddb->ec_IPEType);
	printf("ul_DataNum          = %"PRId32"\n"	,pc_hddb->ul_DataNum);
	printf("pcc_Fat             = %p\n" 	,pc_hddb->pcc_Fat);
	printf("class CC_HDDBlock end\n");
}

// ================================================================================================
//! <!--機能--> データ表示.
//! <!--------------------------------------------------------------------------------------------->
//! <!--履歴-->
//	13/01/10 N.suzuki CQ.376010 FA14発生時のログ強化.
static void dumpOneImageInfoDataDBG(const CC_OneImageInfoData *pc , uchar uc_Band , uchar  uc_Block , uchar  uc_Color)
{
	printf("class CC_OneImageInfoData = %p\n",pc);
	if ( pc == NULL ) {
		return;
	}
	printf("b_Exist          = %d\n" ,pc->b_Exist);
	printf("p_OIP            = %p\n" ,pc->p_OIP);
	printf("sl_FileID        = %"PRId32"\n",pc->sl_FileID);
	printf("cc_Band          = %p\n" ,pc->cc_Band);
	printf("uc_AllocBandNum  = %d\n" ,pc->uc_AllocBandNum);
	printf("uc_AllocBlockNum = %d\n" ,pc->uc_AllocBlockNum);
	printf("uc_AllocColorNum = %d\n" ,pc->uc_AllocColorNum);

	printf("cc_Band[%d].cc_Block[%d].cc_ColorBlock[%d].xxx\n",uc_Band, uc_Block, uc_Color);
	printf("getMemorySize() = %ld getCompressSize = %"PRId32" \n",
		pc->cc_Band[uc_Band].cc_Block[uc_Block].cc_ColorBlock[uc_Color].getMemorySize(),
		pc->cc_Band[uc_Band].cc_Block[uc_Block].cc_ColorBlock[uc_Color].getCompressSize()
	);
	dumpArrayDBG(&(pc->cc_Band[uc_Band].cc_Block[uc_Block].cc_ColorBlock[uc_Color].cc_ArrayInfo));
	{ // fileid
		uint32_t fileid = pc->sl_FileID;
		ST_ImageFileCtrlTbl ist;
		printf("sl_FileID = %"PRId32"\n",fileid);
		comImageFileCtrlTblStore.searchImageFileInfoByFileID(fileid,&ist);
		printf("comImageFileCtrlTblStore.sl_ImageFileID    = %"PRId32"\n",ist.sl_ImageFileID);
		printf("comImageFileCtrlTblStore.ul_InfoClusterNo  = %"PRId32"\n",ist.ul_InfoClusterNo);
		printf("comImageFileCtrlTblStore.ul_ArrayClusterNo = %"PRId32"\n",ist.ul_ArrayClusterNo);
	}
	printf("class CC_OneImageInfoData end\n");
}

// ================================================================================================
//! <!--機能--> データ表示.
//! <!--------------------------------------------------------------------------------------------->
//! <!--履歴-->
//	13/01/10 N.suzuki CQ.376010 FA14発生時のログ強化.
static void dumpImageImpDBG(const IMGC_ImageImp *pc_ImageImp, uchar uc_Band , uchar  uc_Block , uchar  uc_Color)
{
	printf("class IMGC_ImageImp = %p\n",pc_ImageImp);
	if ( pc_ImageImp == NULL ) {
		return;
	}
	printf("uc_IJCBandDivNum         = %d \n"	 ,pc_ImageImp->uc_IJCBandDivNum 					 );
	printf("ss_IJCBandDivLineBasic   = %d \n"	 ,pc_ImageImp->ss_IJCBandDivLineBasic				 );
	printf("ss_IJCBandDivLineLast    = %d \n"	 ,pc_ImageImp->ss_IJCBandDivLineLast				 );
	printf("uc_IJCBlockDivNum        = %d \n"	 ,pc_ImageImp->uc_IJCBlockDivNum					 );
	printf("ss_IJCBlockDivWidthBasic = %d \n"	 ,pc_ImageImp->ss_IJCBlockDivWidthBasic 			 );
	printf("ss_IJCBlockDivWidthLast  = %d \n"	 ,pc_ImageImp->ss_IJCBlockDivWidthLast				 );
	printf("uc_JbigColorNum          = %d \n"	 ,pc_ImageImp->uc_JbigColorNum						 );
	hdump("uc_JbigPlaneNum",pc_ImageImp->uc_JbigPlaneNum,sizeof(pc_ImageImp->uc_JbigPlaneNum));
	printf("ss_PageNo                = %d \n"	 ,pc_ImageImp->ss_PageNo							 );
	printf("ss_BandLineLast          = %d \n"	 ,pc_ImageImp->ss_BandLineLast						 );
	printf("b_CompressSeq            = %d \n"	 ,pc_ImageImp->b_CompressSeq						 );
	printf("b_PcfaxSendOn            = %d \n"	 ,pc_ImageImp->b_PcfaxSendOn						 );
	printf("e_HDDTransKind           = %d \n"	 ,pc_ImageImp->e_HDDTransKind						 );
	printf("uc_TransDataKindNum      = %d \n"	 ,pc_ImageImp->uc_TransDataKindNum					 );
	hdump("em_TransDataKind",pc_ImageImp->em_TransDataKind,sizeof(pc_ImageImp->em_TransDataKind));
	printf("b_isThreadLocal          = %d \n"	 ,pc_ImageImp->b_isThreadLocal						 );
	printf("e_InputPagePlace         = %d \n"	 ,pc_ImageImp->e_InputPagePlace 					 );

	IMGC_ImageMemoryPoolImpColorFileMemoryInstance& comMemoryManager = IMGC_ImageMemoryPoolImpColorFileMemorySingletonIf::newInstance();
	if( NULL == comMemoryManager.pc_OneImageInfoData512 ){
	dumpHDDBlockDB(&pc_ImageImp->c_ImageDataHDDBlock);
	dumpHDDBlockDB(&pc_ImageImp->c_HDDBlock);
	dumpOneImageInfoDataDBG(pc_ImageImp->pc_OneImageInfoData, uc_Band , uc_Block , uc_Color);
	dumpArrayDBG(&(pc_ImageImp->c_ArrayGetMemory));
	}
	printf("uc_OneBandCompress       = %d \n"	 ,pc_ImageImp->uc_OneBandCompress					 );
	printf("e_IPETypeInfo            = %d \n"	 ,pc_ImageImp->e_IPETypeInfo						 );
	printf("e_IPEType                = %d \n"	 ,pc_ImageImp->e_IPEType							 );
	printf("e_ImageMode              = %d \n"	 ,pc_ImageImp->e_ImageMode							 );
	printf("b_ColorSetSeq            = %d \n"	 ,pc_ImageImp->b_ColorSetSeq						 );
	printf("b_LockDelete             = %d \n"	 ,pc_ImageImp->b_LockDelete 						 );
	printf("b_ImageBufExist          = %d \n"	 ,pc_ImageImp->b_ImageBufExist						 );
	printf("b_RestoreBusy            = %d \n"	 ,pc_ImageImp->b_RestoreBusy						 );
	printf("b_ImageDelete            = %d \n"	 ,pc_ImageImp->b_ImageDelete						 );
	printf("e_DeleteMode             = %d \n"	 ,pc_ImageImp->e_DeleteMode 						 );
	printf("ul_CompressSize          = %"PRId32"\n"	 ,pc_ImageImp->ul_CompressSize						 );
	printf("e_OutputStatus           = %"PRId32" \n"	 ,pc_ImageImp->e_OutputStatus						 );
	printf("class IMGC_ImageImp end\n");
}

// ================================================================================================
//! <!--機能--> データ表示.
//! <!--------------------------------------------------------------------------------------------->
//! <!--履歴-->
//  13/01/10 N.suzuki CQ.376010 FA14発生時のログ強化.
static void dumpImageDBG(const IMGC_Image* pc_Image, uchar uc_Band , uchar uc_Block , uchar uc_Color)
{
	printf("\n");
	printf("class IMGC_Image = %p\n",pc_Image);
	if ( pc_Image == NULL ) {
		return;
	}
	dumpImageInfoDBG(&(pc_Image->c_ImageInfo));
	dumpImageImpDBG(pc_Image->m_IMGC_ImageImp,uc_Band , uc_Block , uc_Color);
	printf("class IMGC_Image end\n");
}


// ================================================================================================
//! <!--機能--> HDD転送のためのアレイ情報取得＆設定
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param IMGC_Image* HDD転送ページのImageへのポインタ　
//! <!--戻値-->
//! \return 転送アレイ個数
//! <!--詳細-->
//! HDD転送のためのアレイアドレスとIV（イニシャルベクタ）を１ページ分求める
//! <!--履歴-->
//  07/03/11 morikawa	Donau初版
//  07/05/14 morikawa 静的解析不具合修正　
//  07/07/02 morikawa 画SimHDDアレイ転送対応　
//  09/03/11 takayama Taiga CQ316073 SSD接続でFAX送信中にFAX受信して受信完了後にFAX送信Jobを削除すると.
//                                   FD17が発生する問題の対策.
//  13/02/14 Takatsuji	FA14のアサート文字列のリファイン
//	13/07/03 国吉(日立JTE) 次世代アーキテクト スワップ分割転送対応.
//	13/08/20 ADS坪井 CQ.381171(Act_66426) SC-SFM-26 サブメモリのファイルメモリ化(スワップ分割保存対応).
//	13/11/04 富田(日立JTE) 次世代アーキテクト スワップ分割読出対応. 
//	13/11/12 ADS坪井 CQ.381188(Act_69136) PS-SAC_36 シュリンク版統合ASIC対応.
//  20/12/13 HCL(Prabha) - Modified for Additional Requirement support on top of OND code changes. 
//           Based on the condition, when the system memory is greater than 1 GB.
//           Support for Read/Write operation is added in Simulator.
//	14/08/07 岡部(日立JTE) CQ500800 マルチ動作時、FA14が発生する問題の対策.
//	14/12/09 ADS萩原 CQ.507928(Act_505187) 分割保存/分割読出時の暗号化対応.
//  17/10/13 Yoneda     S800 ストレージ構成変更対応.
// ================================================================================================
uint32_t CP_HDDTrans::getAndSetHDDArrayInfo( IMGC_Image* pc_Image )
{
	uint32_t ul_ArrayCount = 0;	// アレイ個数初期化　
	uchar uc_BandMax 	= pc_Image->m_IMGC_ImageImp->uc_IJCBandDivNum;
	uchar uc_BlockMax 	= pc_Image->m_IMGC_ImageImp->uc_IJCBlockDivNum;
	uchar uc_ColorMax	= theColorInformation.getColorNum( pc_Image, True );
	CC_OneArray*	pcc_OneArray = 0;	// アレイリンクアドレス

	//分割転送の場合.
	if( (True == pc_Image->c_ImageInfo.b_SplitTrans) && (IMGE_SplitTransNone != pc_Image->c_ImageInfo.b_SplitTransStatus)
		&& (IMGE_SplitTransNg != pc_Image->c_ImageInfo.b_SplitTransStatus) ){
		// 2014/5/9 伊藤史. スワップ分割読出対応.
		// アレイを転送作業アレイからHDD転送用アレイにコピーする.
		IMGC_ImageImp*	pcImageImpColor = pc_Image->m_IMGC_ImageImp;
		ul_ArrayCount = pcImageImpColor->ul_TransWorkingArrayNum;
		
		uint32_t ul_MaxRwArrayNum = 0;
		if(((THRE_StorageDeviceTypeSSD & getNvRamDeviceType()) != 0) && 
            (theHDDTrans.e_StartUpDeviceType == THRE_StartupDeviceType_SD)) {
			ul_MaxRwArrayNum = DEF_MAX_SSD_RW_ARRAY_NUM;
		} else {
			ul_MaxRwArrayNum = DEF_MAX_HDD_RW_ARRAY_NUM;
		}

		//HDD転送が書き込みの場合.
		if(EM_HDD_RWMode_Write == st_MsgHDDTransInfo.em_HDD_RWMode){

#ifdef DEF_Soc1200	// 2013/08/20 ADS坪井 CQ.381171(Act_66426) SC-SFM-26 サブメモリのファイルメモリ化(スワップ分割保存対応).
			for( uint32_t i = 0; i < ul_ArrayCount; i++ ){
				if( i % ul_MaxRwArrayNum == 0 ) {
					// 書き込み開始からのオフセットセクタ数計算.
					uint32_t ul_OffsetSector = i * THRD_ONEARRAYSIZE / DEF_SECTOR_SIZE;
					// イニシャルベクタに使用するセクタ番号取得.
					uint32_t ul_Sector = (pc_Image->m_IMGC_ImageImp)->c_HDDBlock.calcRealSector( ul_OffsetSector );
					// イニシャルベクタに設定する入れる配列計算.
					uint32_t ul_IVCount = i / ul_MaxRwArrayNum;
					ul_TransIVNum[ ul_IVCount ] = ul_Sector;
				}
				ul_TransArrayAdr[i] = CC_OneArray::getArrayAddr( pcImageImpColor->pul_TransWorkingArray[i] );
				e_ArrayType[i] = CC_OneArray::getArrayMemoryType( pcImageImpColor->pul_TransWorkingArray[i] );
			}
#else	// #ifdef DEF_Soc1200.
			memcpy( ul_TransArrayAdr, pcImageImpColor->pul_TransWorkingArray, sizeof(uintptr_t) * ul_ArrayCount );
#endif	// #ifdef DEF_Soc1200.

		//HDD転送が読み出しの場合.
		}else if(EM_HDD_RWMode_Read == st_MsgHDDTransInfo.em_HDD_RWMode){	
			CC_OneArray*  pcc_TmpOneArray = pcImageImpColor->c_TransWorkingArray.getLinkAddr();
			for( uint32_t ul_i = 0; ul_i < ul_ArrayCount; ul_i++ ){
				if( ul_i % ul_MaxRwArrayNum == 0 ) {
					// 読み出し先頭からのオフセットセクタ数計算.
					uint32_t ul_OffsetSector = ul_i * THRD_ONEARRAYSIZE / DEF_SECTOR_SIZE;
					// イニシャルベクタに使用するセクタ番号取得.
					uint32_t ul_Sector = ( ( pc_Image->m_IMGC_ImageImp ) )->c_HDDBlock.calcRealSector( ul_OffsetSector );
					// イニシャルベクタに設定する入れる配列計算.
					uint32_t ul_IVCount = ul_i / ul_MaxRwArrayNum;
					ul_TransIVNum[ ul_IVCount ] = ul_Sector;
				}
				ul_TransArrayAdr[ul_i] = pcc_TmpOneArray->getArrayAddr();
#ifdef DEF_Soc1200			
				// アレイのメモリ確保先（メインメモリまたはサブメモリ）を取得し、HDD転送アレイのメモリ種別に設定する.
				e_ArrayType[ul_i] = pcc_TmpOneArray->getArrayMemoryType();
#endif	// DEF_Soc1200.
				pcc_TmpOneArray = pcc_TmpOneArray->next();
			}

		}

		return( ul_ArrayCount );
	}

#if defined(FUM_LINUX)
	int setSectorCount = 0; 
#endif

	IMGC_ImageMemoryPoolImpColorFileMemoryInstance& comMemoryManager = IMGC_ImageMemoryPoolImpColorFileMemorySingletonIf::newInstance();

	if( 0 < us_debug_dspArrayInfo ){
		dumpImageDBG(pc_Image, 0 , 0 , 0);
	}

	// アレイの配列とIVの配列に予め貯めておく　
	if( comMemoryManager.pc_OneImageInfoData512 != NULL ) {
		for( register uchar uc_Band = 0; uc_Band < uc_BandMax; uc_Band++ ) {
			for( register uchar uc_Block=0; uc_Block < uc_BlockMax; uc_Block++ ) {
				for( register uchar uc_Color=0; uc_Color < uc_ColorMax; uc_Color++ ) {
					pcc_OneArray = comMemoryManager.pc_OneImageInfoData512->cc_Band[uc_Band].cc_Block[uc_Block].cc_ColorBlock[uc_Color].cc_ArrayInfo.getStartAddr();
					if ( pcc_OneArray == NULL ) {	// CQ.376010 FA14ログ強化.
						dumpImageDBG(pc_Image, uc_Band ,  uc_Block ,  uc_Color);
					}
					THRL_ThAssert( pcc_OneArray != 0, "CP_HDDTrans::getAndSetHDDArrayInfo() pcc_OneArray==0 Error! Bnd(%d),Blk(%d),Col(%d)", uc_Band, uc_Block, uc_Color, 0, CFA14_DetailCode0x01C0 );

					// 画像転送Simのために必要な処理（１ブロック、１色単位でHDDのファイルを転送する）　
                    // アレイを転送単位個分用意する
                    while( pcc_OneArray != 0 ) {                       
#if defined(SIMULATOR)                    
                  		//Sub memory,Array Unit,CR Change Starts.
		          		//If Condition to verify Memory is greater than 1 GB & do read/write Operation.
                  		if( APIC_MemoryInfo::newInstance()->getWorkMemorySize() > DIMM_MEMSIZE_ALL2 ) {
					uint32_t ul_TotalSectorNum = ul_ArrayCount * THRD_ONEARRAYSIZE / DEF_SECTOR_SIZE;
					uint32_t ul_Sector = ( ( pc_Image->m_IMGC_ImageImp ) )->c_HDDBlock.calcRealSector( ul_TotalSectorNum );
					readWriteArraySim( st_MsgHDDTransInfo.em_HDD_RWMode, pcc_OneArray->getArrayAddr(), ul_Sector, ul_TotalSectorNum );
                        }
                        //Sub memory,Array Unit,CR Change Ends.
#endif  
#ifdef DEF_SupportSSD
						uint32_t ul_MaxRwArrayNum = 0;
                        if(((THRE_StorageDeviceTypeSSD & getNvRamDeviceType()) != 0) && 
                            (theHDDTrans.e_StartUpDeviceType == THRE_StartupDeviceType_SD)) {
							ul_MaxRwArrayNum = DEF_MAX_SSD_RW_ARRAY_NUM;
						} else {
							ul_MaxRwArrayNum = DEF_MAX_HDD_RW_ARRAY_NUM;
						}
						if( ul_ArrayCount % ul_MaxRwArrayNum == 0 ) {
							// ページ先頭からのオフセットセクタ数計算　
							uint32_t ul_OffsetSector = ul_ArrayCount * THRD_ONEARRAYSIZE / DEF_SECTOR_SIZE;
							// イニシャルベクタに使用するセクタ番号取得　
							uint32_t ul_Sector = ( ( pc_Image->m_IMGC_ImageImp ) )->c_HDDBlock.calcRealSector( ul_OffsetSector );
							// イニシャルベクタに設定する入れる配列計算　
							uint32_t ul_IVCount = ul_ArrayCount / ul_MaxRwArrayNum;
							THRL_ThAssert( ul_IVCount < DEF_MAX_IV_NUM,"CP_HDDTrans::getAndSetHDDArrayInfo() ul_IVCount[] Over Error!! (%"PRId32")", ul_IVCount, 0, 0, 0, CFA14_DetailCode0x01C1 );
							ul_TransIVNum[ ul_IVCount ] = ul_Sector;
						}
#else
						if( ul_ArrayCount % DEF_MAX_HDD_RW_ARRAY_NUM == 0 ) {
							// ページ先頭からのオフセットセクタ数計算　
							uint32_t ul_OffsetSector = ul_ArrayCount * THRD_ONEARRAYSIZE / DEF_SECTOR_SIZE;
							// イニシャルベクタに使用するセクタ番号取得　
							uint32_t ul_Sector = ( ( pc_Image->m_IMGC_ImageImp ) )->c_HDDBlock.calcRealSector( ul_OffsetSector );
							// イニシャルベクタに設定する入れる配列計算　
							uint32_t ul_IVCount = ul_ArrayCount / DEF_MAX_HDD_RW_ARRAY_NUM;
							THRL_ThAssert( ul_IVCount < DEF_MAX_IV_NUM,"CP_HDDTrans::getAndSetHDDArrayInfo() ul_IVCount[] Over Error!! (%"PRId32")", ul_IVCount, 0, 0, 0, CFA14_DetailCode0x01C2 );
							ul_TransIVNum[ ul_IVCount ] = ul_Sector;
						}
#endif
						if ( ul_ArrayCount >= DEF_MAX_ONEREADWRITE_ARRAYNUM ) {	// CQ.376010 FA14ログ強化.
							dumpImageDBG(pc_Image, uc_Band ,  uc_Block ,  uc_Color);
						}
						THRL_ThAssert( ul_ArrayCount < DEF_MAX_ONEREADWRITE_ARRAYNUM,"CP_HDDTrans::getAndSetHDDArrayInfo() ul_TransArrayAdr[] Over Error!!", 0, 0, 0, 0, CFA14_DetailCode0x01C3 );
#ifdef DEF_Soc1200	// 2013/07/16 ADS萩原 CQ.381171(Act_66426) SC-SFM-26 サブメモリのファイルメモリ化.
						e_ArrayType[ ul_ArrayCount ] = pcc_OneArray->getArrayMemoryType();	// 使用するアレイ種別を転送個数分用意する.
#endif	// DEF_Soc1200.
						// 使用するアレイアドレスを転送個数分用意する 
						ul_TransArrayAdr[ ul_ArrayCount++ ] = pcc_OneArray->getArrayAddr();
						pcc_OneArray = pcc_OneArray->next();
					}
				}
			}
		}

#if defined(SIMULATOR)
        //Sub memory,Array Unit,CR Change Starts.
		//If Condition to verify Memory is greater than 1 GB & do read/write Operation.
		 if( APIC_MemoryInfo::newInstance()->getWorkMemorySize() > DIMM_MEMSIZE_ALL2 ) {    
		// 画像転送Simのために必要な処理（ページクローズ処理のみ実行、アレイアドレス、セクタ番号はダミー）　
		readWriteArraySim( st_MsgHDDTransInfo.em_HDD_RWMode, 0, 0, 0xFFFFFFFF );
        }
        //Sub memory,Array Unit,CR Change Ends.
#endif        
		
	// アレイの配列とIVの配列に予め貯めておく　
	} else if( ( ( pc_Image->m_IMGC_ImageImp ) )->pc_OneImageInfoData != NULL ) {
		for( register uchar uc_Band = 0; uc_Band < uc_BandMax; uc_Band++ ) {
			for( register uchar uc_Block=0; uc_Block < uc_BlockMax; uc_Block++ ) {
				for( register uchar uc_Color=0; uc_Color < uc_ColorMax; uc_Color++ ) {
					pcc_OneArray = ( ( pc_Image->m_IMGC_ImageImp ) )->pc_OneImageInfoData->cc_Band[uc_Band].cc_Block[uc_Block].cc_ColorBlock[uc_Color].cc_ArrayInfo.getStartAddr();
					if ( pcc_OneArray == NULL ) {	// CQ.376010 FA14ログ強化.
						dumpImageDBG(pc_Image, uc_Band ,  uc_Block ,  uc_Color);
					}
					THRL_ThAssert( pcc_OneArray != 0, "CP_HDDTrans::getAndSetHDDArrayInfo() pcc_OneArray==0 Error! Bnd(%d),Blk(%d),Col(%d)", uc_Band, uc_Block, uc_Color, 0, CFA14_DetailCode0x01C0 );

					// 画像転送Simのために必要な処理（１ブロック、１色単位でHDDのファイルを転送する）　
                    // アレイを転送単位個分用意する
                    while( pcc_OneArray != 0 ) {                       
#if defined(SIMULATOR)                    
                  		//Sub memory,Array Unit,CR Change Starts.
		          		//If Condition to verify Memory is greater than 1 GB & do read/write Operation.
                  		if( APIC_MemoryInfo::newInstance()->getWorkMemorySize() > DIMM_MEMSIZE_ALL2 ) {
					uint32_t ul_TotalSectorNum = ul_ArrayCount * THRD_ONEARRAYSIZE / DEF_SECTOR_SIZE;
					uint32_t ul_Sector = ( ( pc_Image->m_IMGC_ImageImp ) )->c_HDDBlock.calcRealSector( ul_TotalSectorNum );
#if defined(FUM_LINUX) 
					if(0 == setSectorCount)
					{
						( ( pc_Image->m_IMGC_ImageImp ) )->c_HDDBlock.setArraySector( ul_Sector );	//hori debug
					}
#endif
					readWriteArraySim( st_MsgHDDTransInfo.em_HDD_RWMode, pcc_OneArray->getArrayAddr(), ul_Sector, ul_TotalSectorNum );
                        }
                        //Sub memory,Array Unit,CR Change Ends.
#endif  
#ifdef DEF_SupportSSD
						uint32_t ul_MaxRwArrayNum = 0;
                        if(((THRE_StorageDeviceTypeSSD & getNvRamDeviceType()) != 0) && 
                            (theHDDTrans.e_StartUpDeviceType == THRE_StartupDeviceType_SD)) {
							ul_MaxRwArrayNum = DEF_MAX_SSD_RW_ARRAY_NUM;
						} else {
							ul_MaxRwArrayNum = DEF_MAX_HDD_RW_ARRAY_NUM;
						}
						if( ul_ArrayCount % ul_MaxRwArrayNum == 0 ) {
							// ページ先頭からのオフセットセクタ数計算　
							uint32_t ul_OffsetSector = ul_ArrayCount * THRD_ONEARRAYSIZE / DEF_SECTOR_SIZE;
							// イニシャルベクタに使用するセクタ番号取得　
							uint32_t ul_Sector = ( ( pc_Image->m_IMGC_ImageImp ) )->c_HDDBlock.calcRealSector( ul_OffsetSector );
							// イニシャルベクタに設定する入れる配列計算　
							uint32_t ul_IVCount = ul_ArrayCount / ul_MaxRwArrayNum;
							THRL_ThAssert( ul_IVCount < DEF_MAX_IV_NUM,"CP_HDDTrans::getAndSetHDDArrayInfo() ul_IVCount[] Over Error!! (%"PRId32")", ul_IVCount, 0, 0, 0, CFA14_DetailCode0x01C1 );
							ul_TransIVNum[ ul_IVCount ] = ul_Sector;
						}
#else
						if( ul_ArrayCount % DEF_MAX_HDD_RW_ARRAY_NUM == 0 ) {
							// ページ先頭からのオフセットセクタ数計算　
							uint32_t ul_OffsetSector = ul_ArrayCount * THRD_ONEARRAYSIZE / DEF_SECTOR_SIZE;
							// イニシャルベクタに使用するセクタ番号取得　
							uint32_t ul_Sector = ( ( pc_Image->m_IMGC_ImageImp ) )->c_HDDBlock.calcRealSector( ul_OffsetSector );
							// イニシャルベクタに設定する入れる配列計算　
							uint32_t ul_IVCount = ul_ArrayCount / DEF_MAX_HDD_RW_ARRAY_NUM;
							THRL_ThAssert( ul_IVCount < DEF_MAX_IV_NUM,"CP_HDDTrans::getAndSetHDDArrayInfo() ul_IVCount[] Over Error!! (%"PRId32")", ul_IVCount, 0, 0, 0, CFA14_DetailCode0x01C2 );
							ul_TransIVNum[ ul_IVCount ] = ul_Sector;
						}
#endif
						if ( ul_ArrayCount >= DEF_MAX_ONEREADWRITE_ARRAYNUM ) {	// CQ.376010 FA14ログ強化.
							dumpImageDBG(pc_Image, uc_Band ,  uc_Block ,  uc_Color);
						}
						THRL_ThAssert( ul_ArrayCount < DEF_MAX_ONEREADWRITE_ARRAYNUM,"CP_HDDTrans::getAndSetHDDArrayInfo() ul_TransArrayAdr[] Over Error!!", 0, 0, 0, 0, CFA14_DetailCode0x01C3 );
#ifdef DEF_Soc1200	// 2013/07/16 ADS萩原 CQ.381171(Act_66426) SC-SFM-26 サブメモリのファイルメモリ化.
						e_ArrayType[ ul_ArrayCount ] = pcc_OneArray->getArrayMemoryType();	// 使用するアレイ種別を転送個数分用意する.
#endif	// DEF_Soc1200.
						// 使用するアレイアドレスを転送個数分用意する 
						ul_TransArrayAdr[ ul_ArrayCount++ ] = pcc_OneArray->getArrayAddr();
						pcc_OneArray = pcc_OneArray->next();
#if defined(FUM_LINUX)
						setSectorCount++; 
#endif
					}
				}
			}
		}

#if defined(SIMULATOR)
        //Sub memory,Array Unit,CR Change Starts.
		//If Condition to verify Memory is greater than 1 GB & do read/write Operation.
		 if( APIC_MemoryInfo::newInstance()->getWorkMemorySize() > DIMM_MEMSIZE_ALL2 ) {    
		// 画像転送Simのために必要な処理（ページクローズ処理のみ実行、アレイアドレス、セクタ番号はダミー）　
		readWriteArraySim( st_MsgHDDTransInfo.em_HDD_RWMode, 0, 0, 0xFFFFFFFF );
        }
        //Sub memory,Array Unit,CR Change Ends.
#endif        
		
	} else {
		THRL_ThAssert( False,"CP_HDDTrans::getAndSetHDDArrayInfo() pc_OneImageInfoData == NULL", 0, 0, 0, 0, CFA14_DetailCode0x01C4 );
	}

	return( ul_ArrayCount );
}

// ================================================================================================
//! <!--機能--> HDD転送、暗号化のためのIV設定
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param ulong 先頭セクタ番号
//! \param ulong 全転送アレイ個数
//! \param ulong １回の転送におけるアレイ個数
//! <!--戻値-->
//! \return なし
//! <!--詳細-->
//! HDD転送のためのIV（イニシャルベクタ）を１転送分設定する　
//! この関数は、画像のアレイ転送でない時のイニシャルベクタの設定を行う。
//! 具体的な用途は一時データ上書き消去のバッファとＨＤＤの転送
//! １．一時データ消去はＨＤＤ転送高速タスクに３MBで要求がきて、これを４アレイ毎にアクセスするので
//! ２．HDDバックアップユーティリティーで連続アドレスに対してRead/Writeするとき
//! <!--履歴-->
//  07/03/11 morikawa	Donau初版
//  13/02/14 Takatsuji	FA14のアサート文字列のリファイン
// ================================================================================================
void CP_HDDTrans::setIVNum( uint32_t ul_Sector, uint32_t ul_TransArrayNum, uint32_t ul_OneTransArrayNum )
{
	for( uint32_t ul_Num=0; ul_Num<ul_TransArrayNum; ++ul_Num ) {
		if( ul_Num % ul_OneTransArrayNum == 0 ) {
			uint32_t ul_IVCount = ul_Num / ul_OneTransArrayNum;
			THRL_ThAssert( ul_IVCount < DEF_MAX_IV_NUM,"CP_HDDTrans::setIVNum() ul_IVCount[] Over Error!!", 0, 0, 0, 0, CFA14_DetailCode0x01C5 );
			// １回の転送におけるアレイ個数をセクタ番号に換算して配列に代入　
			ul_TransIVNum[ ul_IVCount ] 
				= ul_Sector + ul_Num * THRD_ONEARRAYSIZE / DEF_SECTOR_SIZE;
		}
	}		
}

// ================================================================================================
//! <!--機能--> 直接転送のHDD転送とAES転送を実行
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param THRE_ReadWriteHDDMode HDD書き込み読み出し転送識別
//! \param ulong  バッファの先頭アドレス
//! \param ulong  転送する先頭のセクタ番号
//! \param ulong  転送セクタ数
//! <!--戻値-->
//! \return なし
//! <!--詳細-->
//! 直接転送モードにおけるHDD転送と暗号化モードに応じたAES転送を実行する
//! <!--履歴-->
//  07/03/11 morikawa	Donau初版
//  07/05/29 morikawa	UT検証の不具合修正(暗号化ありの時のメモリアドレスが間違っていた）
//  09/04/05 takayama   CQ:316760 SSD常時接続対応.
//	13/06/17 ADS北村	CQ.381189(Act_66229) EE-SSC_4	HDDレス化対応_SSDへのスワップ対応.
//	14/03/06 toda		Zeus5BK HDDレス対応 HDD装着時にSSDへスワップする暫定対応を削除.
// ================================================================================================
void CP_HDDTrans::transDirectHDDAndAES( THRE_ReadWriteHDDMode e_ReadWriteHDDMode, uintptr_t ul_MemAdr, uint32_t ul_Sector, uint32_t ul_SectorNum )
{
	// 暗号化なしのときの共通化処理のためにアドレス設定　
	uintptr_t ul_BufAdr = ul_MemAdr;
	// 転送トータルサイズを取得　
	uint32_t ul_TransTotalSize = ul_SectorNum * DEF_SECTOR_SIZE;
	// 暗号化ありの場合を考慮して、暗号化用のバッファの書き込み読み出しインデックスを初期化　
	c_HDDBuf.initSeq();
	
	if( b_EnableEnc == True ) {
		if( e_ReadWriteHDDMode == THRE_ReadWriteBufToHDD ) {
			// 暗号化実行
			// 第１引数：暗号化複合化モード
			// 第２引数：アレイ先頭アドレスへのポインタ、未使用なので０
			// 第３引数：メモリ先頭アドレス
			// 第４引数：バッファ先頭アドレス
			// 第５引数：転送サイズ
			// 第６引数：IVとして使用するセクタ番号
			THRC_AesTransSingletonIf::newInstance().encodeDecodeExec( THRE_EncHDDDirectMemToBuf, 0, ul_MemAdr, c_HDDBuf.getWriteAdr(), ul_TransTotalSize, ul_Sector );
			ul_BufAdr = c_HDDBuf.getReadAdr();	// バッファを読み出してHDDに書き込み
		} else {
			ul_BufAdr = c_HDDBuf.getWriteAdr();	// HDDから読み出して書き込むバッファアドレス獲得　
		}
	}	
	
	// HDD転送実行
	// 暗号化なしの時は、ul_BufAdrはul_BufAdrParaと同じ
	// 暗号化あり、HDDへの書き込み時はすでに暗号化されたデータをc_HDDBufから読み出す。これを今からHDDへ書き込む。			
	// 暗号化あり、HDDからの読み出し時はHDDから取り出したデータを今からc_HDDBufに書き込む。その後に複合化する。			
#ifdef DEF_SupportSSD
	readWriteExecHDD( c_DevicePath, e_ReadWriteHDDMode, 0, ul_BufAdr, ul_Sector, ul_SectorNum );
#else
	readWriteExecHDD( e_ReadWriteHDDMode, 0, ul_BufAdr, ul_Sector, ul_SectorNum );
#endif
	
	if( b_EnableEnc == True ) {
		if( e_ReadWriteHDDMode == THRE_ReadWriteHDDToBuf ) {
			// 複合化実行	
			// 第１引数：暗号化複合化モード
			// 第２引数：アレイ先頭アドレスへのポインタ、未使用なので０
			// 第３引数：メモリ先頭アドレス
			// 第４引数：バッファ先頭アドレス
			// 第５引数：転送サイズ
			// 第６引数：IVとして使用するセクタ番号
			THRC_AesTransSingletonIf::newInstance().encodeDecodeExec( THRE_EncHDDDirectBufToMem, 0, ul_MemAdr, c_HDDBuf.getReadAdr(), ul_TransTotalSize, ul_Sector );
		}
	}		
}

// ================================================================================================
//! <!--機能--> HDD転送実行
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param char * :デバイスパス
//! SSD："/dev/kmsda"
//! HDD："/dev/kmsdb"
//! \param THRE_ReadWriteHDDMode HDD書き込み読み出し転送識別
//! \param ulong* アレイの先頭アドレス（アレイ転送の時に使用）
//! \param ulong  メモリの先頭アドレス（メモリ先頭の時に使用）
//! \param ulong  転送する先頭のセクタ番号
//! \param ulong  転送セクタ数
//! \param bool   dosFSの転送と直接転送の識別用フラグ True:dosFs転送 False：直接転送.
//! <!--戻値-->
//! \return なし
//! <!--詳細-->
//! 直接転送、または、dosFSのHDD転送を実行する
//! 書き込み時は、転送開始前にデータのキャッシュフラッシュ
//! 読み出し時は、転送終了後にデータのキャッシュ無効化を行う
//! 直接転送のアレイの実体転送時は基本的にDMAで全てアクセスするのでキャッシュは不要であるが、一時データ上書き消去モード２はソフトでデータチェックが必要となる。
//! 従って、必ずキャッシュの制御は行う。
//! <!--履歴-->
//  07/03/11 morikawa	Donau初版
//  07/08/07 morikawa	1s基板検証不具合修正、デバイスIDと論理ユニット番号は0固定をセットする
//  07/10/19 morikawa	Donau HDDトラブルコード見直し対応
//  08/07/23 t.yokoyama [CQ308783]HDDモニタ開始の関数インターフェースに第三引数を追加.
//  08/11/25 horiuchi	dosのAES&HDD転送と直接転送のセマフォ制御を変更.
//  09/04/05 takayama CQ:316760 SSD常時接続対応.
//  09/04/07 koshimo	CQ.316761 SSD標準化対応（SSDアクセス制御）.
//  10/03/03 yashiro    Gaia問題点リストNo286(CQ324193) : セマフォID dosImageMuteSem 関連処理の削除.
//  10/03/03 Nakagawa	CQ324264 実機用デバッグログ整理.
// 2013/02/14 Takatsuji	FA14のアサート文字列のリファイン
//  2013/12/20 HCL (Prabha) Modified for Additional Requirement support on top of OND code changes. 
//             Based on the condition, when the system memory is less than 1 GB.
//             Support for Read/Write operation is added when Simulator enabled.
//	2017/03/15 Matsuoka FA14(0x01CA)対応 低レベルIOエラー時(ハード要因)はD00Fを出力する
//  2017/10/10 Yoneda   S800 ストレージ構成変更対応.
//  2019/10/29 Sky TAS000000001236対応.
// ================================================================================================
#ifdef DEF_SupportSSD
void CP_HDDTrans::readWriteExecHDD( char *devicePath, THRE_ReadWriteHDDMode e_ReadWriteHDDMode, uintptr_t* pul_ArrayTbl, uintptr_t ul_MemAdr, uint32_t ul_Sector, uint32_t ul_SectorNum, Bool b_DosTrans )

{
#ifdef TORNADO
	int64_t		l_syscall_Ret = 0;			// lseek,write,writev,read,readvの戻り値.
	// 2017/03/08 FA14(0x01CA)対応 エラー状況を一次保存領域へ退避しておく。
	int				tmp_errno = 0;

#if defined(TORNADO) && !defined(COSIM)
	struct iovec 	st_iovec[IOV_MAX];			// readv／writevシステムコールを利用するのでivec構造体(Linux標準構造体)の宣言.
#endif //defined(TORNADO) && !defined(COSIM).
	void*			pAdder = NULL;				// write,writev,read,readvのアドレス格納変数.
	uint32_t			ul_RwSize = 0;				// write,readの転送サイズ格納変数.
	uint32_t			ul_RwNum = 0;				// writev,readvの転送個数格納変数.
	int32_t				fd = -1;
	devicePath = THRL_GetPartitionPathFromOffset(devicePath, ul_Sector, ul_SectorNum);
	if( NULL!=devicePath )
	{
#if defined(TORNADO) && !defined(COSIM)		
		if(strcmp(devicePath,theHDDTrans.pc_strDeviceSSD) == 0){
			
			if( 0==fd_ssd_map.count( std::string(devicePath) ) )
			{
#ifdef FUM_LINUX
				/* 新SIMはRAWアクセス出来ない */
				fd = SYSF_Open( devicePath, O_RDWR | O_CLOEXEC, 0666 );
#else	/* #ifdef FUM_LINUX */
				fd = SYSF_Open( devicePath, ( O_RDWR | O_DIRECT ) | O_CLOEXEC, 0666 );
#endif	/* #ifdef FUM_LINUX */
				fd_ssd_map[ std::string(devicePath) ] = fd;
			} else {
				fd = fd_ssd_map[ std::string(devicePath) ];
			}
		} else {
			if( 0==fd_hdd_map.count( std::string(devicePath) ) )
			{
#ifdef FUM_LINUX
				/* 新SIMはRAWアクセス出来ない */
				fd = SYSF_devopen( devicePath, O_RDWR | O_CLOEXEC, 0666 );
#else	/* #ifdef FUM_LINUX */
				fd = SYSF_devopen( devicePath, ( O_RDWR | O_DIRECT ) | O_CLOEXEC, 0666 );
#endif	/* #ifdef FUM_LINUX */
				fd_hdd_map[ std::string(devicePath) ] = fd;
			} else {
				fd = fd_hdd_map[ std::string(devicePath) ];
			}
		}
	}
	
	if(fd < 0) {
		// open失敗.
		THRL_ThAssert( False, "[SATA2]CP_HDDTrans::readWriteExecHDD() fail open command devicePath=%s fd=%"PRId32" errno=%d\n" , (uintptr_t)devicePath, (uintptr_t)fd, (uintptr_t)SYSF_errnoGet(), 0, CFA14_DetailCode0x01C6 );
	}
	
	// open.
	// lseek.
	l_syscall_Ret = lseek( fd, (int64_t)ul_Sector * (int64_t)DEF_SECTOR_SIZE, SEEK_SET );
	if (l_syscall_Ret < 0) {
		// 失敗.
		THRL_ThAssert( False, "[SATA2]CP_HDDTrans::readWriteExecHDD () fail lseek command fd %"PRId32" ul_Sector %"PRId32" l_syscall_Ret %"PRIx64" errno %d\n",(uintptr_t)fd, (uintptr_t)ul_Sector, (uintptr_t)l_syscall_Ret , errno, CFA14_DetailCode0x01C7 );
	}
#endif //defined(TORNADO) && !defined(COSIM).
	if( e_ReadWriteHDDMode == THRE_ReadWriteArrayToHDD 
	 || e_ReadWriteHDDMode == THRE_ReadWriteHDDToArray ) {
		// アレイから直接転送する時
		// 転送アレイ数計算
		uint32_t ul_TransArrayNum = ul_SectorNum * DEF_SECTOR_SIZE / THRD_ONEARRAYSIZE;
		// エラープロテクト
		THRL_ThAssert( pul_ArrayTbl != NULL , "CP_HDDTrans::readWriteExecHDD() pul_ArrayTbl NULL Error!!", 0, 0, 0, 0, CFA14_DetailCode0x0A5C );
		THRL_ThAssert( pul_ArrayTbl[0] != 0,"CP_HDDTrans::readWriteExecHDD() ArrayAdr Error!! pul_ArrayTbl[0]=0", 0, 0, 0, 0, CFA14_DetailCode0x01C8 );
#if defined(TORNADO) && !defined(COSIM)
		// アレイの読み書き指定アドレス設定
		for( uint32_t ul_ArrayNum = 0; ul_ArrayNum < ul_TransArrayNum; ++ul_ArrayNum ) {
			st_iovec[ul_ArrayNum].iov_base = (void *)(pul_ArrayTbl[ul_ArrayNum]);
			st_iovec[ul_ArrayNum].iov_len = THRD_ONEARRAYSIZE;
		}
		pAdder = st_iovec;
#endif //defined(TORNADO) && !defined(COSIM).
		ul_RwNum = ul_TransArrayNum;
	} else {
		// バッファを介して転送する時
		// バッファの読み書き指定アドレス設定
		// エラープロテクト
		THRL_ThAssert( ul_MemAdr != 0,"CP_HDDTrans::readWriteExecHDD() ul_MemAdr Error!! ul_MemAdr=0", 0, 0, 0, 0, CFA14_DetailCode0x01C9 );
		ul_RwSize = ul_SectorNum * DEF_SECTOR_SIZE;
		pAdder = (void *)ul_MemAdr;
	}
	
	// HDDAccsess ログ出力.
	if ( us_debug_HDDdispparm != 0 ) {
		uint32_t ulReqCmd = HDD_REQ_READ;
		if ( e_ReadWriteHDDMode == THRE_ReadWriteArrayToHDD || e_ReadWriteHDDMode == THRE_ReadWriteBufToHDD ) {
			ulReqCmd = HDD_REQ_WRITE;
		}

		uintptr_t ulRWAdder = 0;
		if ( e_ReadWriteHDDMode == THRE_ReadWriteArrayToHDD || e_ReadWriteHDDMode == THRE_ReadWriteHDDToArray ) {
			ulRWAdder = (uintptr_t)(pul_ArrayTbl[0]);
			ul_RwSize = THRD_ONEARRAYSIZE;
		} else {
			ulRWAdder = ul_MemAdr;
		}

		THRL_ThLogMsgErr( "HDDAccsess Cmd=0x%"PRIx32",Adder[0]=0x%"PRIxPTR",Sector=%"PRId32"(%"PRId32"[MB]),Size[0]=%"PRId32"(%"PRId32"[KB])\n",
			ulReqCmd, ulRWAdder, 		// 要求コマンド、読み書き指定アドレス.
			ul_Sector, ul_Sector/2048,  // セクタ番号、セクタ番号からMBに変換.
			ul_RwSize, ul_RwSize/1024 ); // サイズ(バイト)、サイズ(バイトからKBに変換).
	}

#ifdef DEF_A3A4COMM
	setHddAStart( ul_Sector, (ul_SectorNum * DEF_SECTOR_SIZE),
		(uchar)( ( e_ReadWriteHDDMode == THRE_ReadWriteArrayToHDD || e_ReadWriteHDDMode == THRE_ReadWriteBufToHDD )? HDDA_WRITE:HDDA_READ ),
		THD_hdda_OSRW_start );		//HDDモニタ開始.
#else	// #ifdef DEF_A3A4COMM.
	setHddAStart( ul_Sector, ul_SectorNum,
		(uchar)( ( e_ReadWriteHDDMode == THRE_ReadWriteArrayToHDD || e_ReadWriteHDDMode == THRE_ReadWriteBufToHDD )? HDDA_WRITE:HDDA_READ ) );		//HDDモニタ開始.
#endif	// #ifdef DEF_A3A4COMM.

#if defined(TORNADO) && !defined(COSIM)

	uint32_t ul_SizeOrNum = 0;
	
	// HDD転送実行(Linux標準関数で転送実行).
	switch( e_ReadWriteHDDMode ) {
		case THRE_ReadWriteArrayToHDD:	
			l_syscall_Ret = writev( fd, (struct iovec *)pAdder ,ul_RwNum );
			ul_SizeOrNum = ul_RwNum;
			break;
		case THRE_ReadWriteHDDToArray:
			l_syscall_Ret = readv( fd, (struct iovec *)pAdder ,ul_RwNum );
			ul_SizeOrNum = ul_RwNum;
			break;
		case THRE_ReadWriteBufToHDD:
			l_syscall_Ret = write( fd, (char *)pAdder ,ul_RwSize );
			ul_SizeOrNum = ul_RwSize;
			break;
		case THRE_ReadWriteHDDToBuf:
			l_syscall_Ret = read( fd, (char *)pAdder ,ul_RwSize );
			ul_SizeOrNum = ul_RwSize;
			break;
		default:	
			// ありえないパターン.
			THRL_ThLogMsgErr("[SATA2]CP_HDDTrans::readWriteExecHDD() wrong THRE_ReadWriteHDDMode : %d\n", e_ReadWriteHDDMode );
			break;
	}		
	// 2017/03/08 FA14(0x01CA)対応 エラー状況を一次保存領域へ退避しておく.
	// 上記のwrite/readのシステムコールの間に他のシステムコールが割り込んではいけないので注意.
	tmp_errno = SYSF_errnoGet();

#endif //defined(TORNADO) && !defined(COSIM).
            
	//Sub memory,Array Unit,CR Change Starts.
    //When the system memory is less than 1 GB and also Simulator is enabled, Support for Read/Write operation is added.
#ifdef SIMULATOR
	if((pul_ArrayTbl != 0) && (APIC_MemoryInfo::newInstance()->getWorkMemorySize() <= DIMM_MEMSIZE_ALL2)){
			//Verification - Is ImageSim is active. 
              if( true == THRF_isActiveImageTransSim()){
                    uchar uc_Counter = (uchar)_roundup( ul_RwSize, (uint32_t)THRD_ONEARRAYSIZE );
                  
                    EM_HDD_RWMode em_HDD_RWMode = EM_HDD_RWMode_Erase;
                    switch( e_ReadWriteHDDMode ) {
                    case THRE_ReadWriteBufToHDD:
                        em_HDD_RWMode = EM_HDD_RWMode_Write;
                        break;
                    case THRE_ReadWriteHDDToBuf:
                        em_HDD_RWMode = EM_HDD_RWMode_Read;
                        break;
                    default:
                        break;
                    }

                    if(em_HDD_RWMode != EM_HDD_RWMode_Erase) {
                        uintptr_t ul_NewAdder = (uintptr_t)pAdder;
                        for (uchar uc_ArrayIndex = 0; uc_ArrayIndex < uc_Counter;uc_ArrayIndex++){
                                 readWriteArraySim( em_HDD_RWMode,  // read / wirte mode.
                                     ul_NewAdder,    // start address.
                                     ul_Sector,     //start sector number.
                                    uc_ArrayIndex );    // end sector.
                                  ul_NewAdder = ul_NewAdder + (uintptr_t)THRD_ONEARRAYSIZE;
                        }
						readWriteArraySim( em_HDD_RWMode, 0, 0, 0xFFFFFFFF );
                    }
              }
          }
#endif
	//Sub memory,Array Unit,CR Change Ends.
#ifdef DEF_A3A4COMM        
	setHddAEnd(THD_hdda_OSRW_end);	//HDDモニタ終了
#else
	setHddAEnd();	//HDDモニタ終了
#endif	// #ifdef DEF_A3A4COMM.
	if (l_syscall_Ret < 0) {		
		// 読み書き失敗.	
		uint64_t          ull_NewDetailData = THRD_NewDetail_ModuleId_Thread;	// 1,2桁：スレッド.
		// 2017/03/08 FA14(0x01CA)対応 低レベルIOエラー時はHDD_TRANS_ERRORトラブルコードを出力する
		if (EIO == tmp_errno || ENOSPC == tmp_errno) {
			ull_NewDetailData |= (uint64_t)( TYPD_TroubleCode_HDD_TRANS_ERROR ) << THRD_NewDetail_TroubleCode_Shift;	// 3-6桁：トラブルコード.
			ull_NewDetailData |= (uint64_t)( 1 ) << 32;							// 7-8桁：HDD_TRANS_ERRORの通し番号.
			ull_NewDetailData |= (uint64_t)( tmp_errno );							// 残り(INT幅)詳細エラーコード.
			THRL_ThAssertLog2( 0, ull_NewDetailData );
			DebugForever( HDD_TRANS_ERROR );
		} else {
			ull_NewDetailData |= (uint64_t)( CFA14_DetailCode0x01CA ) << THRD_NewDetail_TroubleCode_Shift;	// 3-6桁：トラブルコード.
			ull_NewDetailData |= (uint64_t)( tmp_errno );							// 7桁：e_ReadWriteHDDMode.
			THRL_ThAssertLog2( 0, ull_NewDetailData );
			THRL_ThAssert( False,"[SATA2]CP_HDDTrans::readWriteExecHDD() fail read/write command  Error!! e_ReadWriteHDDMode=%d pAdder=0x%08x ul_RwNum/ul_RwSize=%d errno=%d", 
			  (ulong)e_ReadWriteHDDMode, (ulong)pAdder, ul_SizeOrNum, (ulong)errno, CFA14_DetailCode0x01CA );
		}		
	}		
#endif	/* #ifdef TORNADO */
}

#else
void CP_HDDTrans::readWriteExecHDD( THRE_ReadWriteHDDMode e_ReadWriteHDDMode, uint32_t* pul_ArrayTbl, uintptr_t ul_MemAdr, uint32_t ul_Sector, uint32_t ul_SectorNum, Bool b_DosTrans )
{

#ifdef TORNADO
	int64_t		l_syscall_Ret = 0;			// lseek,write,writev,read,readvの戻り値.
#if defined(TORNADO) && !defined(COSIM)
	struct iovec 	st_iovec[IOV_MAX];			// readv／writevシステムコールを利用するのでivec構造体(Linux標準構造体)の宣言.
#endif //defined(TORNADO) && !defined(COSIM).
	void*			pAdder = NULL;				// write,writev,read,readvのアドレス格納変数.
	uint32_t			ul_RwSize = 0;				// write,readの転送サイズ格納変数.
	uint32_t			ul_RwNum = 0;				// writev,readvの転送個数格納変数.

#if defined(TORNADO) && !defined(COSIM)	
	// open.
	if(fd_hdd < 0) {
#ifdef FUM_LINUX
		/* 新SIMはRAWアクセス出来ない */
		fd_hdd = SYSF_Open( THRD_HDD_DEVICE, O_RDWR, 0666 );
#else	/* #ifdef FUM_LINUX */
		fd_hdd = SYSF_Open( THRD_HDD_DEVICE, O_RDWR | O_DIRECT, 0666 );
#endif	/* #ifdef FUM_LINUX */
		if(fd_hdd < 0) {
			// open失敗.
			THRL_ThAssert( False, "[SATA2]fail open command \n" , 0, 0, 0, 0, CFA14_DetailCode0x01CB );
		}
	}
	// lseek.
	l_syscall_Ret = lseek( fd_hdd, (int64_t)ul_Sector * (int64_t)DEF_SECTOR_SIZE, SEEK_SET );
	if (l_syscall_Ret < 0) {
		// 失敗.
		THRL_ThAssert( False, "[SATA2]fail lseek command fd_hdd %"PRId32" ul_Sector %"PRId32" l_syscall_Ret %"PRIx64"\n",(uintptr_t)fd_hdd,(uintptr_t)ul_Sector,(uintptr_t)l_syscall_Ret , 0, CFA14_DetailCode0x01CC );
	}
#endif //defined(TORNADO) && !defined(COSIM).

	if( e_ReadWriteHDDMode == THRE_ReadWriteArrayToHDD 
	 || e_ReadWriteHDDMode == THRE_ReadWriteHDDToArray ) {
		// アレイから直接転送する時
		// 転送アレイ数計算
		uint32_t ul_TransArrayNum = ul_SectorNum * DEF_SECTOR_SIZE / THRD_ONEARRAYSIZE;
		// エラープロテクト
		THRL_ThAssert( pul_ArrayTbl[0] != 0,"CP_HDDTrans::readWriteExecHDD ArrayAdr Error!!", 0, 0, 0, 0, CFA14_DetailCode0x01CD );
#if defined(TORNADO) && !defined(COSIM)
		// アレイの読み書き指定アドレス設定
		for( uint32_t ul_ArrayNum = 0; ul_ArrayNum < ul_TransArrayNum; ++ul_ArrayNum ) {
			st_iovec[ul_ArrayNum].iov_base = (void *)(pul_ArrayTbl[ul_ArrayNum]);
			st_iovec[ul_ArrayNum].iov_len = THRD_ONEARRAYSIZE;
		}
		pAdder = st_iovec;
#endif //defined(TORNADO) && !defined(COSIM).
		ul_RwNum = ul_TransArrayNum;
	} else {
		// バッファを介して転送する時
		// バッファの読み書き指定アドレス設定
		// エラープロテクト
		THRL_ThAssert( ul_MemAdr != 0,"CP_HDDTrans::readWriteExecHDD ul_MemAdr Error!!", 0, 0, 0, 0, CFA14_DetailCode0x01CE );
		ul_RwSize = ul_SectorNum * DEF_SECTOR_SIZE;
		pAdder = (void *)ul_MemAdr;
	}
	
	// HDDAccsess ログ出力.
	if ( us_debug_HDDdispparm != 0 ) {
		uint32_t ulReqCmd = HDD_REQ_READ;
		if ( e_ReadWriteHDDMode == THRE_ReadWriteArrayToHDD || e_ReadWriteHDDMode == THRE_ReadWriteBufToHDD ) {
			ulReqCmd = HDD_REQ_WRITE;
		}

		uintptr_t ulRWAdder = 0;
		if ( e_ReadWriteHDDMode == THRE_ReadWriteArrayToHDD || e_ReadWriteHDDMode == THRE_ReadWriteHDDToArray ) {
			ulRWAdder = (uintptr_t)(pul_ArrayTbl[0]);
			ul_RwSize = THRD_ONEARRAYSIZE;
		} else {
			ulRWAdder = ul_MemAdr;
		}

		THRL_ThLogMsgErr( "HDDAccsess Cmd=0x%"PRIx32",Adder[0]=0x%"PRIXPTR",Sector=%"PRIu32"(%"PRIu32"[MB]),Size[0]=%"PRIu32"(%"PRIu32"[KB])\n",
			ulReqCmd, ulRWAdder, 		// 要求コマンド、読み書き指定アドレス.
			ul_Sector, ul_Sector/2048,  // セクタ番号、セクタ番号からMBに変換.
			ul_RwSize, ul_RwSize/1024 ); // サイズ(バイト)、サイズ(バイトからKBに変換).
	}

#ifdef DEF_A3A4COMM
	setHddAStart( ul_Sector, ul_SectorNum,
		(uchar)(( e_ReadWriteHDDMode == THRE_ReadWriteArrayToHDD || e_ReadWriteHDDMode == THRE_ReadWriteBufToHDD )? HDDA_WRITE:HDDA_READ ), 
		THD_hddaOS_start );		//HDDモニタ開始.
#else	// #ifdef A3A4CO	MM.
	setHddAStart( ul_Sector, ul_SectorNum,
		(uchar)( ( e_ReadWriteHDDMode == THRE_ReadWriteArrayToHDD || e_ReadWriteHDDMode == THRE_ReadWriteBufToHDD )? HDDA_WRITE:HDDA_READ ) );		//HDDモニタ開始.
#endif	// #ifdef DEF_A3A4COMM.

#if defined(TORNADO) && !defined(COSIM)
	// HDD転送実行(Linux標準関数で転送実行).
	switch( e_ReadWriteHDDMode ) {
		case THRE_ReadWriteArrayToHDD:	
			l_syscall_Ret = writev( fd_hdd, (struct iovec *)pAdder ,ul_RwNum );
			break;
		case THRE_ReadWriteHDDToArray:
			l_syscall_Ret = readv( fd_hdd, (struct iovec *)pAdder ,ul_RwNum );
			break;
		case THRE_ReadWriteBufToHDD:
			l_syscall_Ret = write( fd_hdd, (char *)pAdder ,ul_RwSize );
			break;
		case THRE_ReadWriteHDDToBuf:
			l_syscall_Ret = read( fd_hdd, (char *)pAdder ,ul_RwSize );
			break;
		default:	
			// ありえないパターン.
			THRL_ThLogMsgErr("[SATA2]wrong THRE_ReadWriteHDDMode : %d\n", e_ReadWriteHDDMode );
			break;
	}		
#endif //defined(TORNADO) && !defined(COSIM).

#ifdef DEF_A3A4COMM
	setHddAEnd(THD_hdda_OSRW_end);	//HDDモニタ終了.
#else
	setHddAEnd();	//HDDモニタ終了.
#endif	// #ifdef DEF_A3A4COMM.

	if (l_syscall_Ret < 0) {		
		// 読み書き失敗.	
		THRL_ThAssert( False,"[SATA2]fail read/write command  Error!!", 0, 0, 0, 0, CFA14_DetailCode0x01CF );
	}		

#endif	/* #ifdef TORNADO */
}

#endif

// ================================================================================================
//! <!--機能--> HDD転送開始処理実行における暗号化を考慮メッセージ同期処理実行
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param THRE_EncHDDMode 	HDDの暗号化転送識別
//! \param ulong* pul_array	アレイ先頭アドレス
//! \param Bool b_OnlyOneTransArray	アレイアドレスのデータの大きさ識別
// 			True：アレイ領域が１回の転送サイズしか存在しない（一時データ消去用）、
// 			False：転送する総アレイ数分存在する（通常画像のアレイ転送用）
//! \param ulong ul_TransArrayNum	全転送アレイ個数
//! \param ulong ul_OneTransArrayNum１回の転送におけるアレイ個数
//! \param ulong ul_EndArrayNum		転送完了したアレイ個数（デフォルト引数で初期値は0）　
//! \param Bool b_ReqCompareData		一時データ消去におけるデータ比較要求（デフォルト引数で初期値はFalse）　
//! \param ST_ErasePatternTbl* pEPTbl	一時データ消去におけるデータ比較テーブルへのポインタ（デフォルト引数で初期値はNULL）
//! \param uchar	uc_Ptn				一時データ消去におけるデータ比較パターン（デフォルト引数で初期値は0）
//! \param Bool True:連続領域変更有効,False:無効
//! \param Bool b_BackupRestore True:バックアップ/リストアである. False:バックアップ/リストアではない.　
//! <!--戻値-->
//! \return なし
//! <!--詳細-->
//! HDD転送開始時の暗号化あり時を考慮した暗号化処理との同期制御を行う
//! 引数の最後の３つは一時データ消去モード２において、複合化されたデータが正しいかどうかをAES処理タスク側で行うために必要なもの
//! <!--履歴-->
//  07/03/11 morikawa	Donau初版
//  07/05/14 morikawa 静的解析不具合修正　
//  07/08/17 morikawa レビュー指摘事項修正　
//  07/08/20 morikawa AESタスクシーケンス実行変数の初期化をAESタスクからこの関数へ移動　
//  12/02/23 toda	  ファイルメモリ分割対応.
//	14/03/10 岡部朋(日立JTE) A3A4共通化 暗号化DMA対応 引数にバックアップ/リストアかどうかのパラメタを追加.
// ================================================================================================
#ifdef DEF_Soc1200	// 2013/07/16 ADS萩原 CQ.381171(Act_66426) SC-SFM-26 サブメモリのファイルメモリ化.
#ifdef DEF_A3A4COMM
void CP_HDDTrans::syncSendMsgExecHDD( THRE_EncHDDMode e_EncHDDModePara, uintptr_t* pul_array, Bool b_OnlyOneTransArray, 
									  uint32_t ul_TransArrayNum, uint32_t ul_OneTransArrayNum, uint32_t ul_EndArrayNum,
									  Bool b_ReqCompareData, const ST_ErasePatternTbl* pEPTbl, uchar uc_Ptn, Bool b_IsChangeContArea,
									  TYPE_ArrayMemoryType* pe_ArrayType, Bool b_BackupRestore )
#else // DEF_A3A4COMM.
void CP_HDDTrans::syncSendMsgExecHDD( THRE_EncHDDMode e_EncHDDModePara, uintptr_t* pul_array, Bool b_OnlyOneTransArray, 
									  uint32_t ul_TransArrayNum, uint32_t ul_OneTransArrayNum, uint32_t ul_EndArrayNum,
									  Bool b_ReqCompareData, const ST_ErasePatternTbl* pEPTbl, uchar uc_Ptn, Bool b_IsChangeContArea,
									  TYPE_ArrayMemoryType* pe_ArrayType )
#endif	// #ifdef DEF_A3A4COMM.
#else	// DEF_Soc1200.
void CP_HDDTrans::syncSendMsgExecHDD( THRE_EncHDDMode e_EncHDDModePara, uintptr_t* pul_array, Bool b_OnlyOneTransArray, 
									  uint32_t ul_TransArrayNum, uint32_t ul_OneTransArrayNum, uint32_t ul_EndArrayNum,
									  Bool b_ReqCompareData, const ST_ErasePatternTbl* pEPTbl, uchar uc_Ptn, Bool b_IsChangeContArea )
#endif	// DEF_Soc1200.
{
	if( b_EnableEnc == True || b_IsChangeContArea == True ) {
	
		// HDDタスクとAESタスクの同期の多重起動チェック　
		THRL_ThAssert( THRC_AesTransSingletonIf::newInstance().isBusySemCountHDDAESTrans() == False, "syncSendMsgExecHDD() Semphore Busy Double Exec Error!!", 0, 0, 0, 0, CFA14_DetailCode0x01D0 );
		
		// AESタスクのシーケンス実行変数を初期化する　
		THRC_AesTransSingletonIf::newInstance().initSeq();
		
		// 暗号化用のバッファの書き込み読み出しインデックスを初期化　
		c_HDDBuf.initSeq();
		
		// 暗号化ありのときのみ、以下の暗号化処理タスクを起動
		// AESとの同期のための計数セマフォ生成　
		EM_HDD_RWMode em_HDD_RWMode = EM_HDD_RWMode_Read; 
		if( e_EncHDDModePara == THRE_EncHDDArrayToBuf ) {
			em_HDD_RWMode = EM_HDD_RWMode_Write;
		}
		THRC_AesTransSingletonIf::newInstance().semCreateCountHDDAESTrans( em_HDD_RWMode );
		// 暗号化メッセージ送信　
		s_MsgAESTransInfo.e_EncHDDMode = e_EncHDDModePara;	// HDDの暗号化転送識別設定　
		s_MsgAESTransInfo.pul_ArrayAdr = pul_array;			// パラメータとしてアレイの先頭アドレスを渡す 
#ifdef DEF_Soc1200	// 2013/07/16 ADS萩原 CQ.381171(Act_66426) SC-SFM-26 サブメモリのファイルメモリ化.
		s_MsgAESTransInfo.pe_ArrayType       = pe_ArrayType;		// パラメータとしてアレイ種別の先頭アドレスを渡す.
#endif	// DEF_Soc1200.
		s_MsgAESTransInfo.pul_IVNum = ul_TransIVNum;		// IVに使用するセクタ番号の配列の先頭アドレスを渡す　
		s_MsgAESTransInfo.ul_EndArrayNum = ul_EndArrayNum;	// 転送済みアレイ個数を渡す　
		s_MsgAESTransInfo.b_OnlyOneTransArray = b_OnlyOneTransArray;	 
		s_MsgAESTransInfo.ul_TransArrayNum = ul_TransArrayNum;
		s_MsgAESTransInfo.ul_OneTransArrayNum = ul_OneTransArrayNum;

		s_MsgAESTransInfo.b_ReqCompareData = b_ReqCompareData;
		s_MsgAESTransInfo.pEPTbl = pEPTbl;
		s_MsgAESTransInfo.uc_Ptn = uc_Ptn;
		
		// 暗号化あり時は、スルーモードはFalseにする。
		// 暗号化なしでＡＥＳ転送タスクにメッセージを送信する時のみスルーモードをTrueにする。
		s_MsgAESTransInfo.b_PassThroughMode = !b_EnableEnc;
#ifdef DEF_A3A4COMM
		s_MsgAESTransInfo.b_BackupRestore = b_BackupRestore;
#endif	// #ifdef DEF_A3A4COMM.	
				
		sendMessage( TID_AesTrans, TM_AESTransStartReq, (uintptr_t)&s_MsgAESTransInfo );
	}
}

// ================================================================================================
//! <!--機能--> １つのHDD転送開始処理実行
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param Bool True:連続領域変更有効,False:無効　
//! <!--戻値-->
//! \return なし
//! <!--詳細-->
//! １つのHDD転送単位終了時の暗号化あり時を考慮した暗号化処理との同期制御を行う
//! <!--履歴-->
//  07/03/11 morikawa	Donau初版
// ================================================================================================
void CP_HDDTrans::syncStartOneExecHDD( Bool b_IsChangeContArea )
{
	if( b_EnableEnc == True || b_IsChangeContArea == True ) {
		// 暗号化ありor連続領域変更有効時、HDD転送カウンティングセマフォ取得
		THRC_AesTransSingletonIf::newInstance().semTakeCountHDDTrans();
	}
}

// ================================================================================================
//! <!--機能--> １つのHDD転送終了処理実行　
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param Bool True:連続領域変更有効,False:無効　
//! <!--戻値-->
//! \return なし
//! <!--詳細-->
//! １つのHDD転送単位終了時の暗号化あり時を考慮した暗号化処理との同期制御を行う
//! <!--履歴-->
//  07/03/11 morikawa	Donau初版
// ================================================================================================
void CP_HDDTrans::syncEndOneExecHDD( Bool b_IsChangeContArea )
{
	if( b_EnableEnc == True || b_IsChangeContArea == True ) {
		// 暗号化ありor連続領域変更有効時、HDD転送カウンティングセマフォ開放　
		THRC_AesTransSingletonIf::newInstance().semGiveCountAESTrans();
	}
}

// ================================================================================================
//! <!--機能--> 全てのHDD転送終了処理実行　
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param THRE_ReadWriteHDDToBuf HDD転送モード
//! \param Bool True:連続領域変更有効,False:無効　
//! <!--戻値-->
//! \return なし
//! <!--詳細-->
//! 全てのHDD転送終了時の暗号化あり時を考慮した暗号化処理との同期制御を行う
//! 同期の計数セマフォの削除も行う　
//! <!--履歴-->
//  07/03/13 morikawa	Donau初版
//  07/08/20 morikawa	アレイ転送の停止要求発生時にＡＥＳタスクが完全に停止処理が完了するまでセマフォ削除を待つ処理追加　
// ================================================================================================
void CP_HDDTrans::syncEndAllExecHDD( THRE_ReadWriteHDDMode e_ReadWriteHDDMode, Bool b_IsChangeContArea )
{
	if( b_EnableEnc == True || b_IsChangeContArea == True ) {
		if( e_ReadWriteHDDMode == THRE_ReadWriteHDDToBuf ) {
			// 暗号化ありor連続領域変更有効時のHDDからの読み出し時は複合化が全て終了したことをセマフォで確認する　
			THRC_AesTransSingletonIf::newInstance().semTakeCountHDDTrans();
			THRC_AesTransSingletonIf::newInstance().semTakeCountHDDTrans();
		}
		// 停止要求発生時はAESタスクが停止処理を完了するまで待つ　
		while( THRC_AesTransSingletonIf::newInstance().isBusyStopAes() == True  ) {
			delayTask( 10 );
		}
		// AESとの同期のための計数セマフォ削除　
		THRC_AesTransSingletonIf::newInstance().semDeleteCountHDDAESTrans();
	}	
}
			

/*================================================================================================
［機能］	シーケンスに応じた処理を実行
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	無し
［詳細］	現在のシーケンスに応じた処理を行う（※イベント発生時、通常nextHDDSeqと対）
			CASE文無いでは必ずHDDコマンドが発生する処理があること（シーケンスを進めるイベントはコマンド完了なので）
			シーケンスとして『接続』と『フォーマット』がありこの二つは同時に動作しないことが前提です
	◆ 接続シーケンス ◆
		ＨＤＤ情報読みだし→
		データ読みだし＆チェック→
		APM有効化→
		暗号鍵設定
			nextHddSeq()にてセキュリティチェック
		セキュリティーが有効モードの時のみ
			ロック解除
			ロック解除パスワード設定
		フォーマット情報読みだし→
		フォーマットチェック→（フォーマットされていない：未フォーマット）
		書き込みチェック１→
		読み出しチェック１(★)→
		書き込みチェック２→
		読み出しチェック２(★)→
		終了処理（メッセージ送信）（接続完了：フォーマット済み）
		
	◆ フォーマットシーケンス ◆
		フォーマット処理
		フォーマットＩＤ書き込み処理
		書き込みチェック１→
		読み出しチェック１→
		書き込みチェック２→
		読み出しチェック２→
		初期化終了処理
［ｳｫｰｸｽﾙｰ］	実施未
［履歴］	07/03/06 morikawa Donau初版
			07/09/27 morikawa 1st基板によるセキュリティーコマンド検証問題対応　
						パスワード不一致発生時の処理を整理　
						装置消去コマンドの実行処理はEM_HDDSeq_EraceUnit_SetPassで行うが、BSPの実行関数をコールするとその中で
						待ち状態となるため、startEraceStage()はBSPの関数実行前に行う。
						さらに、HDD転送タスクではメッセージが受け取れなくなるので、イメージタスク表示更新処理を行う　
			07/10/19 morikawa Donau HDDトラブルコード見直し対応に伴い
								ユーザーパスワード解除時のエラー発生処理は実行関数内で処理するように修正
			10/09/24 世古	  FPGA基板時も一時データ消去の上書き優先のための消去パターン作成処理を行う(CQ:xxxxxx).
			09/04/22 t-kasahara CQ:317046 SSD廃棄消去_フォーマット対応.
            13/02/14 Takatsuji	FA14のアサート文字列のリファイン
			14/07/03 yamada		制御構成変更No39 廃棄時消去レポート対応.
			14/09/17 Ogura		HDDミラーリング有効時にスレーブ側もベリファイを行う.
			15/02/20 y.morikawa CQ.507060 HDDミラリング再構筑を実行中にHDDフォーマットを実施できない。SlaveのR/Wチェックを削除.
			16/03/17 yamada		SATAコントローラ、メンテナンスモード起動時の自動復旧対応.
			19/08/26 Sky鳥居 [EagleZ+] OP_BTS-11139対応.
			20/04/16 Sky        [HeliosMLK] OP_BTS-22679対応.
================================================================================================*/
void CP_HDDTrans::execHDDSeq()
{
	static int32_t   n_RetryInitHDDCheck =  0; // 初期接続時のエラーリトライ回数.
	const  int32_t MAX_RetryInitHDDCheck = 10; // 初期接続時のエラーリトライの上限回数.

	// 06/07/18 moirkawa debug時のみログを出力するように修正　
	if( us_debug_HDDdispparm > 0 ) {
		THRL_ThLogMsgErr( "execHDDSeq=%d\n",CP_HDDTrans::em_HDDSeq,0,0,0);
	}	

	setDebugHddSeq((ushort)CP_HDDTrans::em_HDDSeq, 0);	// 2005/10/07 modified by t.sasaki
	
	// 現在のシーケンスに応じた処理を行う
	switch( CP_HDDTrans::em_HDDSeq ) {
		case EM_HDDSeq_Idle:						// アイドル時の処理は無し
			break;
// ここからＨＤＤの接続チェック開始　
		case EM_HDDSeq_CheckInformation:			// ＨＤＤ情報データ読みだし＆チェック
			if( func_HDDSeq_CheckInformation() != HDD_STATUS_OK ) {
				setHDDSeq( EM_HDDSeq_Idle );				// シーケンスを停止（待機状態へ）
				postProc_CheckInfomationNg();		// エラー時の後処理.
				return;	// 正常終了でない場合はこれ以上シーケンスを進めない　	
			} 
			// 01/10/04 morikawa ここまでで、コマンドを用いた接続確認完了　
			// checkInfomationを抜けたら、自動復旧フラグを落とす.
			if (True == THRG_getSataFwAutoRecoverExec()) {
				// フラグが立っているので復旧した事がわかる.
				setDebugLowLogSpeedSeq(881, 0);
				THRL_ThLogMsgErr("[connection] SATA FW AutoRecovery OK!!!\n");
				(void)THRG_setSataFwAutoRecoverNotExec();
			}
			bl_ConnectFlag = False;	// 以後はＤＭＡの完了検出はセマフォを用いる
			break;
//////////////////////////////////////////
// APM有効化
//////////////////////////////////////////
		case EM_HDDSeq_EnableAPM:				// APM有効化.
			func_HDDSeq_EnableAPM();
			break;
//////////////////////////////////////////
// 暗号化の有効／無効を判断する
//////////////////////////////////////////
		case EM_HDDSeq_SetAngouMode:			// 暗号モード設定 
			func_HDDSeq_SetAngouMode();
			break;
		case EM_HDDSeq_LockCheck:
			// 処理はなにもせずシーケンスを進めて、NEXT判定時にセキュリティをチェック 
			break;
//////////////////////////////////////////
// セキュリティチェック（セキュリティーが有効モードの時のみ）
//////////////////////////////////////////
		case EM_HDDSeq_UnLockMode:				
			func_HDDSeq_UnLockMode();
			break;
		case EM_HDDSeq_UnLockMode_SetPass:		
			// ユーザーパスワード解除を実行し、異常発生時の処理は実行関数内で行う　
			func_HDDSeq_UnLockMode_SetPass( uc_PassBuff );
			break;
//////////////////////////////////////////
// ＨＤＤアクセスチェック
//////////////////////////////////////////
		case EM_HDDSeq_ReadFormat:				// フォーマット情報読みだし
			FuncCallstart( 21, __FUNCTION__, 1 );		// 割込みチェック用関数コールスタート.
			func_HDDSeq_ReadFormat( puc_WorkMemAddr );
			FuncCallend( 21, __FUNCTION__, 1 );			// 割込みチェック用関数コールエンド.
			break;
		case EM_HDDSeq_CheckFormat:				// フォーマットチェック
			FuncCallstart( 22, __FUNCTION__, 1 );		// 割込みチェック用関数コールスタート.
			if( func_HDDSeq_CheckFormat( puc_WorkMemAddr )!= HDD_STATUS_OK ) {
// ここでFatalにしてしまうとファイルシステムが初期化されず、フォーマットできない.
				// Fatalにはしない(パネルによる警告表示).
				bl_CheckFormatError = True ;
			}
			FuncCallend( 22, __FUNCTION__, 1 );			// 割込みチェック用関数コールエンド.
			break;
//////////////////////////////////////////
// フォーマット処理起動
//////////////////////////////////////////
		case EM_HDDSeq_FormatPre:				// 03/04/09 tsukiji 【TSS2】フォーマット前処理セキュリティチェック
			func_HDDSeq_FormatPre();
			break;
		case EM_HDDSeq_Format:					// フォーマット処理
			// 0x00での上書き削除固定で動作する.// 2018/6/15 ishiga AR1047260.
			// 物理フォーマット or 廃棄フォーマット.
			func_HDDSeq_Format( THRE_StorageDeviceTypeSSD );
			break;
		case EM_HDDSeq_FormatDisablePassword:
			func_HDDSeq_DisablePassword();
			break;
		case EM_HDDSeq_FormatDisablePassword_SetPass:
			func_HDDSeq_FormatDisablePassword( uc_PassBuff );
			break;
		// 論理フォーマット
 		case EM_HDDSeq_Format_Logical:			// 03/07/28 tsukiji 【TSS2】論理フォーマット処理
 			func_HDDSeq_Format_Logical();
 			break;
		case EM_HDDSeq_FormatID:					// フォーマットＩＤ書き込み処理
			func_HDDSeq_FormatID();
			break;
// 以下の処理は初期接続時とフォーマット終了チェックと同じ
		// システム領域のあるエリアに対して行う。
		// ＨＤＤに対して、２回通り行う。
		// １回目と２回目のリードライトのデータの関係は以下のデータとする
		// １回目のデータ（X）、２回目のデータ（Y）
		// (X AND Y = 0x00000000) AND (X OR Y = 0xFFFFFFFF)
		case EM_HDDSeq_CheckWrite_1:				// 書き込みチェック１→読み出しチェック１
		case EM_HDDSeq_FormatWrite_1:				// フォーマット時も同じ処理
			// まず００Ｈを書き込み
			func_HDDSeq_CheckWrite( uc_WriteBuff, sizeof(uc_WriteBuff), DEF_HDD_READWRITE_CHECKDATA_1 );
			break;
		case EM_HDDSeq_CheckRead_1:				// 読み出しチェック１→書き込みチェック２
		case EM_HDDSeq_FormatRead_1:			// フォーマット時も同じ処理
			func_HDDSeq_CheckRead( uc_ReadBuff );
			break;
		case EM_HDDSeq_CheckWrite_2:				// 書き込みチェック２→読み出しチェック２
		case EM_HDDSeq_FormatWrite_2:				// フォーマット時も同じ処理
			if((CC_MEMFunctionSingletonIf::newInstance().getFuncCode() != FUNC_HD_CHECK_MODE)
			  || (CC_MEMFunctionSingletonIf::newInstance().enableFunction() != False)
			)
			{
				// 初期接続時はエラーリトライ回数を初期化する.
				n_RetryInitHDDCheck = 0;
			}

			bl_ReadWriteCheck = True;
			// データを書き込みデータと読み込みデータをORして０ならＯＫ
			if( bitData( uc_WriteBuff, uc_ReadBuff, DEF_SECTOR_SIZE, EC_LogicMode_Or, DEF_HDD_READWRITE_CHECKDATA_1 ) == False ) {
				THRL_ThLogMsgErr("HDD RWCheck Error(TestValue=0x00)\n");
				bl_ReadWriteCheck = False;	// 読み書き失敗
				// 読み書き異常で終了
				if( CP_HDDTrans::em_HDDSeq == EM_HDDSeq_CheckWrite_2 ) {
					func_HDDSeq_End();				// 初期接続失敗
				} else {
					func_HDDSeq_FormatEnd();		// フォーマット失敗
				}
				setHDDSeq( EM_HDDSeq_Idle );					// シーケンスを停止（待機状態へ）
				return;	// 正常終了でない場合はこれ以上シーケンスを進めない　	
			} else {
				// ＦＦＨを書き込み
				func_HDDSeq_CheckWrite( uc_WriteBuff, sizeof(uc_WriteBuff), DEF_HDD_READWRITE_CHECKDATA_2 );
			}
			break;
		case EM_HDDSeq_CheckRead_2:				// 読み出しチェック２→読み書きチェック
		case EM_HDDSeq_FormatRead_2:			// フォーマット時も同じ処理
			func_HDDSeq_CheckRead( uc_ReadBuff );
			break;
		case EM_HDDSeq_CheckCompare_2:
			bl_ReadWriteCheck = True;
			// データを書き込みデータと読み込みデータをANDしてＦＦならＯＫ
			if( bitData( uc_WriteBuff, uc_ReadBuff, DEF_SECTOR_SIZE, EC_LogicMode_And, DEF_HDD_READWRITE_CHECKDATA_2 ) == False ) {
				if((CC_MEMFunctionSingletonIf::newInstance().getFuncCode() != FUNC_HD_CHECK_MODE)
				  || (CC_MEMFunctionSingletonIf::newInstance().enableFunction() != False)
				)
				{
					// 初期接続時のエラーはリトライする.
					if( ++n_RetryInitHDDCheck<MAX_RetryInitHDDCheck )
					{
						THRL_ThLogMsgErr("HDD RWCheck Error(TestValue=0xFF) retry:%d\n", n_RetryInitHDDCheck);

						// 書込みチェック２のFFh書込みからやり直す.
						func_HDDSeq_CheckWrite( uc_WriteBuff, sizeof(uc_WriteBuff), DEF_HDD_READWRITE_CHECKDATA_2 );

						setHDDSeq( EM_HDDSeq_CheckRead_2 ); // 読み出しチェック２に移行.
						return;
					}
				}

				THRL_ThLogMsgErr("HDD RWCheck Error(TestValue=0xFF)\n");
				bl_ReadWriteCheck = False;
				func_HDDSeq_End();				// 初期接続失敗
				setHDDSeq( EM_HDDSeq_Idle );					// シーケンスを停止（待機状態へ）
				return;
			}

			n_RetryInitHDDCheck = 0; // チェックOKの時はリトライ回数を初期化する.
			break;
		case EM_HDDSeq_WriteErasePattern:		// 消去パターン作成		定義名は変更予定　
#ifdef TORNADO // SimWinビルド対応.
			if( THRF_IsAsic_0Exist() == True && THRF_IsAsicFpga() == False ) {
				// ASIC200が存在する時のみ実行　
				func_HDDSeq_MakeErasePattern();
			}	
#endif // TORNADO SimWinビルド対応.
			break;
			
		case EM_HDDSeq_MasterPassword:				// 03/04/10 tsukiji 【TSS2】マスタパスワード設定
			// 論理フォーマット時の後処理
			bl_ReadWriteCheck = True;
			// データを書き込みデータと読み込みデータをORしてＦＦならＯＫ
			if( bitData( uc_WriteBuff, uc_ReadBuff, DEF_SECTOR_SIZE, EC_LogicMode_And, DEF_HDD_READWRITE_CHECKDATA_2 ) == False ) {
				bl_ReadWriteCheck = False;
				setHDDSeq( EM_HDDSeq_Idle );					// シーケンスを停止（待機状態へ）
				// 読み書き異常で終了
				func_HDDSeq_FormatEnd();		// フォーマット失敗
				return;	// 正常終了でない場合はこれ以上シーケンスを進めない　	
			}
			func_HDDSeq_SetMasterPassword();
			break;
		case EM_HDDSeq_MasterPassword_SetPass:				// 03/04/10 tsukiji 【TSS2】マスタパスワード設定
			func_HDDSeq_SetMasterPassword_SetPass( uc_PassBuff );
			break;
 		case EM_HDDSeq_Format_OS:				// 03/07/28 tsukiji 【TSS2】論理フォーマット処理
 			func_HDDSeq_Format_OS();
 			break;
		case EM_HDDSeq_End:						// 初期化終了処理
			func_HDDSeq_End();				// 初期接続終了処理
			setHDDSeq( EM_HDDSeq_Idle );	// シーケンスを停止（待機状態へ）
			uc_Indicator_HDDSleep = 0 ;		// スリープ中インジケータ OFF 2005/10/07 added by t.sasaki
			break;
		case EM_HDDSeq_FormatEnd:				// フォーマット時も同じ処理
			bl_ReadWriteCheck = True;
			// データを書き込みデータと読み込みデータをANDしてＦＦならＯＫ
			if( bitData( uc_WriteBuff, uc_ReadBuff, DEF_SECTOR_SIZE, EC_LogicMode_And, DEF_HDD_READWRITE_CHECKDATA_2 ) == False ) {
				THRL_ThLogMsgErr("HDD RWCheck Error(TestValue=0xFF)\n");
				bl_ReadWriteCheck = False;
			}
			if( CP_HDDTrans::em_HDDSeq == EM_HDDSeq_End ) {
				func_HDDSeq_End();			// 初期接続終了処理
			} else {
				func_HDDSeq_FormatEnd();	// フォーマット終了処理
			}	
			setHDDSeq( EM_HDDSeq_Idle );	// シーケンスを停止（待機状態へ）
			break;
		case EM_HDDSeq_FormatEnd2:			// 03/07/29 tsukiji 【TSS2】論理フォーマット後の終了(前処理に読み書きチェック無し)
			func_HDDSeq_FormatEnd2();		// 通常のフォーマットはR/W行なうがここではしないので別処理 
			setHDDSeq( EM_HDDSeq_Idle );	// シーケンスを停止（待機状態へ）
			break;
//////////////////////////////////////////
// セキュリティイレース（フォーマット分岐処理）マスタパスワードは存在していて消えないのでそのまま
//////////////////////////////////////////
		case EM_HDDSeq_EracePrepare:			// 消去準備 03/03/24 tsukiji 【TSS2】
			func_HDDSeq_EracePrepare();
			break;
		case EM_HDDSeq_EraceUnit:				// 装置消去 03/03/24 tsukiji 【TSS2】
			func_HDDSeq_EraceUnit();
			break;
		case EM_HDDSeq_EraceUnit_SetPass:		// パスワード設定
			// 03/06/16 tsukiji 【TSS2】擬似カウントスタート ９９％まで
			startEraceStage();
			func_HDDSeq_EraceUnit_SetPass( uc_PassBuff );
			break;
		case EM_HDDSeq_EraceMaster:				// 03/04/10 tsukiji 【TSS2】マスタパスワード再設定
			// 03/06/16 tsukiji 【TSS2】擬似カウント停止
			endEraceStage();
			func_HDDSeq_SetMasterPassword();
			break;
		case EM_HDDSeq_EraceMaster_SetPass:		// 03/04/10 tsukiji 【TSS2】マスタパスワード再設定
			func_HDDSeq_SetMasterPassword_SetPass( uc_PassBuff );
			break;
		case EM_HDDSeq_EraceEnd:				// 03/04/10 tsukiji 【TSS2】消去完了後処理
			// 03/06/16 tsukiji 【TSS2】擬似カウント停止
			endEraceStage();
			func_HDDSeq_EraceEnd();
			setHDDSeq( EM_HDDSeq_Idle );	// シーケンスを停止（待機状態へ）
			break;
//////////////////////////////////////////
// パスワード設定
//////////////////////////////////////////
		case EM_HDDSeq_DisablePassword:			// パスワード消去 03/03/24 tsukiji 【TSS2】
			func_HDDSeq_DisablePassword();
			break;
		case EM_HDDSeq_DisablePassword_SetPass:			// パスワード消去 03/03/24 tsukiji 【TSS2】
			func_HDDSeq_DisablePassword_SetPass( uc_PassBuff, pst_MsgSetPassword->uc_OLD_Password );
			break;
		case EM_HDDSeq_SetPassword:				// パスワード設定 03/03/24 tsukiji 【TSS2】
			func_HDDSeq_SetPassword();
			break;
		case EM_HDDSeq_SetPassword_SetPass:				// パスワード設定 03/03/24 tsukiji 【TSS2】
			func_HDDSeq_SetPassword_SetPass( uc_PassBuff, pst_MsgSetPassword->uc_NEW_Password );
			break;
		case EM_HDDSeq_PasswordEnd:				// パスワード設定 03/03/24 tsukiji 【TSS2】
			func_HDDSeq_PasswordEnd();
			setHDDSeq( EM_HDDSeq_Idle );	// 03/10/08 tsukiji 【TSS2】設定し忘れ、シーケンスを停止（待機状態へ）
			break;
		case EM_HDDSeq_CheckID_Write:			// フォーマットＩＤ書き込み処理 2005/07/15 added by t.sasaki
			func_HDDSeq_FormatID();
			break;
		case EM_HDDSeq_CheckID_Read:			// フォーマットＩＤ読み出し処理 2005/07/15 added by t.sasaki
			FuncCallstart( 23, __FUNCTION__, 1 );		// 割込みチェック用関数コールスタート.
			func_HDDSeq_ReadFormat( puc_WorkMemAddr );
			FuncCallend( 23, __FUNCTION__, 1 );			// 割込みチェック用関数コールエンド.
			break;
		case EM_HDDSeq_CheckID_End:				// フォーマットＩＤ終了(検査)処理 2005/07/15 added by t.sasaki
			FuncCallstart( 24, __FUNCTION__, 1 );		// 割込みチェック用関数コールスタート.
			if( func_HDDSeq_CheckFormat( puc_WorkMemAddr )!= HDD_STATUS_OK ) {
				bl_CheckFormatError = True ;
			} else {
				bl_CheckFormatError = False ;
			}
			FuncCallend( 24, __FUNCTION__, 1 );			// 割込みチェック用関数コールエンド.
			sendMessage( TID_Image, TM_HDDWriteIDCompleteNotice, (uintptr_t)HDD_STATUS_OK );
			break;
		case EM_HDDSeq_ResetAngouMode:				// 暗号化ASIC 暗号モード解除 2005/08/10 Div.12 k-kawa 
			func_HDDSeq_ResetAngouMode();
			break;
		default:
			THRL_ThAssert( False, "CP_HDDTrans::execHDDSeq() default(%d) Error!!", CP_HDDTrans::em_HDDSeq, 0, 0, 0, CFA14_DetailCode0x01D1 );
			break;
	}
	
	// 実行処理終了で、nextHDDSeq()がコールされてステート遷移する。
	// 次のシーケンスへの移行を判断する　
	if( us_debug_HDDdispparm>0 ) {
		setDebugHddSeq(999, 0);	// 2005/10/07 modified by t.sasaki
	}

	// 次のシーケンス遷移
	nextHDDSeq();
}


/*================================================================================================
［機能］	イベントで次のシーケンスへ
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	無し
［詳細］	現在のシーケンスより次のシーケンスへ移行する（※イベント発生時）
			シーケンスとして『接続』と『フォーマット』がありこの二つは同時に動作しないことが前提です
［ｳｫｰｸｽﾙｰ］	実施済み：2001/1/10：森川
［履歴］	02/02/05：morikawa DIPSW1がONの時に強制的にフォーマット処理を行っていた暫定処置削除
			04/07/20：t.sasaki HDDセキュリティ対応(TSS2からの移植)
			04/08/31：k_ozawa  暗号化ASIC用の処理を追加
			04/09/10：t.sasaki HDDヘッド退避関連－APM有効化.
			04/09/24：k_ozawa PIO転送モード設定処理を追加
			05/06/20：t.sasaki 暗号化ASIC正式対応.
			05/07/15：t.sasaki 暗号化基板搭載時の一時データ上書き消去用上書きパターン作成シーケンス追加.
			05/07/15：t.sasaki チェックID書き込みシーケンス追加.
			05/09/21：tsukiji  ロック中にロックのかかっていないHDDに変えられた場合の対応 
			05/10/27：tsukiji ロック状態はIdentify Device Informationから参照するように変更 
			07/03/06 morikawa Donau初版
			07/09/27 morikawa 不要な処理を削除
			07/11/05 morikawa メモリファンクション HDD R/Wチェック時は上書き優先時の消去パターン作成を実施しないように変更 
			12/10/02 noda     CQ:383919 HDDロック状態の追加ログ出力 
			14/09/17 Ogura		スレーブ側もリードチェックをおこなう （HDDミラーリング対応）.
			14/11/21 y.morikawa CQ.504528 HDDパスワードロック中に本体の電源をOFFすると再起動後C-FA14が発生する.
			15/01/28 Ooka		CQ.509898 HDDミラーリング有効時に接続された側のHDDのみ処理を行う.
			15/02/20 y.morikawa CQ.507060 HDDミラリング再構筑を実行中にHDDフォーマットを実施できない。SlaveのR/Wチェックを削除.
			19/12/11 Sky		MFP初期セットアップ自動化対応.
================================================================================================*/
void CP_HDDTrans::nextHDDSeq()
{
	// 現在のシーケンスから移行
	switch( CP_HDDTrans::em_HDDSeq ) {
		case EM_HDDSeq_Idle:					// 待機状態時に発生したイベントは無視
			break;
		case EM_HDDSeq_CheckInformation:		// データ読みだし＆チェック→暗号化ASICステータス読み出し 
			setHDDSeq( EM_HDDSeq_EnableAPM );
			break;
		case EM_HDDSeq_EnableAPM:				// APM有効化.
			setHDDSeq( EM_HDDSeq_SetAngouMode );
			break;
		case EM_HDDSeq_SetAngouMode:			// 暗号モード設定 →暗号モード読み出し 
			setHDDSeq( EM_HDDSeq_LockCheck );	// UnLock前のロック確認
			break;

		case EM_HDDSeq_LockCheck:
#ifdef FUM_LINUX
			/* 新SIMはHDDロックパスワードを設定出来ないためスルー */
			setHDDSeq( EM_HDDSeq_ReadFormat );
#else	/* #ifdef FUM_LINUX */
//                          | 本体 PASSWORD  |
//                          |   ON   |  OFF  |
//      --------------------|--------|-------|-
//      HDD PASSWORD    ON  |   ○   |  ×   |	×  ...    セキュリティエラー
//                      ----|--------|-------|- ○  ...    不一致のみセキュリティエラー
//                      OFF |   ×   |  ◎   |  ◎  ...    セキュリティエラーなし
//      --------------------|--------|-------|-
			if( isLockPassword() == True ) {
				THRL_ThLogMsgErr("HDD LockPassword=true\n");
#ifdef DEF_SupportSSD
				if ( ( True == isMirroringHDD() ) && ( THRE_SsdPortNo != ul_PortNo )
					&& ( m_e_HddMirroringDeviceStatus[THRE_HddMirroring_Chanel_A] == THRE_HddMirroringStatus_Normal )
					&& ( m_e_HddMirroringDeviceStatus[THRE_HddMirroring_Chanel_B] == THRE_HddMirroringStatus_Normal ) ) {
					// ミラーリング有効時.
					if ( ( isSecurityEnable( THRE_HddPortNo ) == True ) && ( isSecurityEnable( THRE_HddChBPortNo ) == True ) ) {
						// 両方セキュリティはON.
						bl_LockPasswordFlag = True;			// ロックされているフラグ.
						// どちらかがロック中であれば解除.
						if( ( isSecurityLocked( THRE_HddPortNo ) == True ) || ( isSecurityLocked( THRE_HddChBPortNo ) == True ) ) {
							THRL_ThLogMsgErr("HDD Device Lock\n");
							// ロック状態にあるので解除を行う 
							setHDDSeq( EM_HDDSeq_UnLockMode );
						} else {
							THRL_ThLogMsgErr("HDD Device UnLock\n");
							// スリープ復帰後ですでにUNLOCK済みの場合
							setHDDSeq( EM_HDDSeq_ReadFormat );
						}
					} else {
						if( ( isSecurityLocked( THRE_HddPortNo ) == True ) || ( isSecurityLocked( THRE_HddChBPortNo ) == True ) ) {
							THRL_ThLogMsgErr("HDD Device Lock CH-A=[%d] CH-B=[%d]\n", isSecurityLocked( THRE_HddPortNo ), isSecurityLocked( THRE_HddChBPortNo ) );
						} else {
							THRL_ThLogMsgErr("HDD Device NotLock\n");
						}
						// セキュリティエラーとする(LOCKされていないＨＤＤ検出)
						bl_SecurityErrorFlag = True;
						setHDDSeq( EM_HDDSeq_Idle );			// セキュリティエラー停止
						// ★ パスワードが入っているのにロックがかかっていないHDD発見（あやしい）
						setDebugHighSpeedSeq(99, 15);
					}
				} else {
					// ミラーリング無効時 or SSD.
					// ミラーリング無効時(HDD)はm_e_HddMirroringDeviceStatus[THRE_HddMirroring_Chanel_A]がNormalとなる.
					uchar	uc_UnlockPortNo = THRE_SsdPortNo;
					if ( m_e_HddMirroringDeviceStatus[THRE_HddMirroring_Chanel_A] == THRE_HddMirroringStatus_Normal ) {
						uc_UnlockPortNo = THRE_HddPortNo;
					} else if ( m_e_HddMirroringDeviceStatus[THRE_HddMirroring_Chanel_B] == THRE_HddMirroringStatus_Normal ) {
						uc_UnlockPortNo = THRE_HddChBPortNo;
					}
					if( isSecurityEnable( uc_UnlockPortNo ) == True ) {
						// セキュリティはON 
						bl_LockPasswordFlag = True;			// ロックされているフラグ
// 2009/04/05   takayama CQ:316760 SSD常時接続対応.
						if( isSecurityLocked( uc_UnlockPortNo ) == True ) {
							THRL_ThLogMsgErr("HDD Device Lock\n");
							// ロック状態にあるので解除を行う 
							setHDDSeq( EM_HDDSeq_UnLockMode );	// 03/03/24 tsukiji 【TSS2】ロック解除
						} else {
							THRL_ThLogMsgErr("HDD Device UnLock\n");
							// スリープ復帰後ですでにUNLOCK済みの場合
							setHDDSeq( EM_HDDSeq_ReadFormat );
						}
					}
#else
					// ロック中かの判定をIdentify Device informationより取得する方法に変更 05/10/12 tsukiji
					if( isSecurityEnable() == True ) {
						// セキュリティはON 
						bl_LockPasswordFlag = True;			// ロックされているフラグ
						if( isSecurityLocked() == True ) {
							THRL_ThLogMsgErr("HDD Device Lock\n");
							// ロック状態にあるので解除を行う 
							setHDDSeq( EM_HDDSeq_UnLockMode );	// 03/03/24 tsukiji 【TSS2】ロック解除
						} else {
							THRL_ThLogMsgErr("HDD Device UnLock\n");
							// スリープ復帰後ですでにUNLOCK済みの場合
							setHDDSeq( EM_HDDSeq_ReadFormat );
						}
					}
#endif
					else {
						THRL_ThLogMsgErr("HDD Device NotLock\n");
						// セキュリティエラーとする(LOCKされていないＨＤＤ検出)
						bl_SecurityErrorFlag = True;
						setHDDSeq( EM_HDDSeq_Idle );			// セキュリティエラー停止
						// ★ パスワードが入っているのにロックがかかっていないHDD発見（あやしい）
						setDebugHighSpeedSeq(99, 15);
					}
				}
			} else {
				THRL_ThLogMsgErr("HDD LockPassword=False\n");
				if ( ( True == isMirroringHDD() ) && ( THRE_SsdPortNo != ul_PortNo )
					&& ( m_e_HddMirroringDeviceStatus[THRE_HddMirroring_Chanel_A] == THRE_HddMirroringStatus_Normal )
					&& ( m_e_HddMirroringDeviceStatus[THRE_HddMirroring_Chanel_B] == THRE_HddMirroringStatus_Normal ) ) {
					// ミラーリング有効時.
					if ( ( isSecurityEnable( THRE_HddPortNo ) == True ) || ( isSecurityEnable( THRE_HddChBPortNo ) == True ) ) {
						THRL_ThLogMsgErr("HDD Device Lock CH-A=[%d] CH-B=[%d]\n", isSecurityLocked( THRE_HddPortNo ), isSecurityLocked( THRE_HddChBPortNo ) );
						// セキュリティはON.
						bl_LockPasswordFlag = True;			// ロックされているフラグ.
						// HDD本体にパスワードが設定されている 
						// セキュリティエラーとする(LOCKされたＨＤＤ検出)
						bl_SecurityErrorFlag = True;
						setHDDSeq( EM_HDDSeq_Idle );			// セキュリティエラー停止
						// ★ パスワード設定されていないが、ロックがかかっているHDD発見
						setDebugHighSpeedSeq(99, 16);
						// 2012/05/16 sakanashi CQ364868 Start.
						// 接続シーケンス停止した場合、APFの初期処理で必要な/km/work_fwがマウントされない.
						initOSdriver(THRD_SSD_DEVICE);
#ifdef DCSW_DEVELOP_SETUPTOOL
						printf("[SetUPTool][%s] Mirror Enable THRF_DevelopSetUp Call \n",__FUNCTION__ );
						THRF_DevelopSetUp();
#endif	// DCSW_DEVELOP_SETUPTOOL.
						// 2012/05/16 sakanashi CQ364868 end.
					} else {
						THRL_ThLogMsgErr("HDD Device NotLock\n");
						// ★パスワードが設定されていない時UNLOCKしない(東芝とWDはパスワードなしでUNLOCKはエラーになるので )
						setHDDSeq( EM_HDDSeq_ReadFormat );
					}
				} else {
					// ミラーリング無効時 or SSD.
// 2009/04/05   takayama CQ:316760 SSD常時接続対応.
#ifdef DEF_SupportSSD
					uchar	uc_UnlockPortNo = THRE_SsdPortNo;
					if ( m_e_HddMirroringDeviceStatus[THRE_HddMirroring_Chanel_A] == THRE_HddMirroringStatus_Normal ) {
						uc_UnlockPortNo = THRE_HddPortNo;
					} else if ( m_e_HddMirroringDeviceStatus[THRE_HddMirroring_Chanel_B] == THRE_HddMirroringStatus_Normal ) {
						uc_UnlockPortNo = THRE_HddChBPortNo;
					}
					if( isSecurityEnable( uc_UnlockPortNo ) == True ) {
#else
					if( isSecurityEnable() == True ) {
#endif
						THRL_ThLogMsgErr("HDD Device Lock\n");
						// セキュリティはON.
						bl_LockPasswordFlag = True;			// ロックされているフラグ.
						// HDD本体にパスワードが設定されている 
						// セキュリティエラーとする(LOCKされたＨＤＤ検出)
						bl_SecurityErrorFlag = True;
						setHDDSeq( EM_HDDSeq_Idle );			// セキュリティエラー停止
						// ★ パスワード設定されていないが、ロックがかかっているHDD発見
						setDebugHighSpeedSeq(99, 16);
						// 2012/05/16 sakanashi CQ364868 Start.
						// 接続シーケンス停止した場合、APFの初期処理で必要な/km/work_fwがマウントされない.
						initOSdriver(THRD_SSD_DEVICE);
#ifdef DCSW_DEVELOP_SETUPTOOL
						printf("[SetUPTool][%s] Mirror Disable THRF_DevelopSetUp Call \n",__FUNCTION__ );
						THRF_DevelopSetUp();
#endif	// DCSW_DEVELOP_SETUPTOOL.
						// 2012/05/16 sakanashi CQ364868 end.
					} else {
						THRL_ThLogMsgErr("HDD Device NotLock\n");
						// ★パスワードが設定されていない時UNLOCKしない(東芝とWDはパスワードなしでUNLOCKはエラーになるので )
						setHDDSeq( EM_HDDSeq_ReadFormat );
					}
				}
			}
#endif	/* #ifdef FUM_LINUX */
			break;
// Rhein1/2/3 end 2005.09.21 tsukiji UnLock前のロック確認(セキュリティ強化：ロック時にロックされていないHDD装着)

// 初期接続時の動作チェック　
		case EM_HDDSeq_UnLockMode:				// 03/03/24 tsukiji 【TSS2】ロック解除
			// 実行後、ココでAbortedCommand発生するのはFrozenモード時のみだけどありえないのでチェックしない
			setHDDSeq( EM_HDDSeq_UnLockMode_SetPass );
			break;
		case EM_HDDSeq_UnLockMode_SetPass:			// 03/03/24 tsukiji 【TSS2】ロック解除
			setHDDSeq( EM_HDDSeq_ReadFormat );
			break;
		case EM_HDDSeq_ReadFormat:				// フォーマット情報読みだし→フォーマットチェック
			setHDDSeq( EM_HDDSeq_CheckFormat );
			break;
		case EM_HDDSeq_CheckFormat:				// フォーマットチェック→書き込みチェック
			setHDDSeq( EM_HDDSeq_CheckWrite_1 );	// フォーマット済みなら→書き込みチェック
			break;
		case EM_HDDSeq_CheckWrite_1:			// 書き込みチェック１→読み出しチェック１
			setHDDSeq( EM_HDDSeq_CheckRead_1 );
			break;
		case EM_HDDSeq_CheckRead_1:				// 読み出しチェック１(★)→書き込みチェック２
			setHDDSeq( EM_HDDSeq_CheckWrite_2 );
			break;
		case EM_HDDSeq_CheckWrite_2:			// 書き込みチェック２→読み出しチェック２
			setHDDSeq( EM_HDDSeq_CheckRead_2 );
			break;
		case EM_HDDSeq_CheckRead_2:				// 読み出しチェック２(★)→消去パターン書き込み(PIO)
			setHDDSeq( EM_HDDSeq_CheckCompare_2 );
			break;
		case EM_HDDSeq_CheckCompare_2:
#ifdef COSIM
			if( False ) {	// COSIM環境ではMemFuncがリンクされないので　
#else		
			if((CC_MEMFunctionSingletonIf::newInstance().getFuncCode() == FUNC_HD_CHECK_MODE) && (CC_MEMFunctionSingletonIf::newInstance().enableFunction() == False)) {
#endif	// #ifdef COSIM
				// メモリファンクション HDD R/Wチェック時は終了　
				setHDDSeq( EM_HDDSeq_End );
			} else {
				// 初期化シーケンス実行時は上書き優先時の消去パターン作成へ　
				ul_ErasePattern_Index = 0 ;
				setHDDSeq( EM_HDDSeq_WriteErasePattern );
			}
			break;
		case EM_HDDSeq_WriteErasePattern:		// 消去パターン作成終了　シーケンス終了　
			setHDDSeq( EM_HDDSeq_End );
			break;
		case EM_HDDSeq_End:						// 終了処理→待機状態
			setHDDSeq( EM_HDDSeq_Idle );
			break;
			
// フォーマットシーケンス　
		case EM_HDDSeq_FormatPre:				// フォーマット前処理→【分岐フォーマットまたは装置消去】
			// パスワード不一致状態かを判定 05/10/27 tsukiji パスワード設定されているHDDのみ 
// 2009/04/05   takayama CQ:316760 SSD常時接続対応.
#ifdef DEF_SupportSSD
			if( (isSecurityError()==True) && (isSecurityLocked(ul_PortNo) == True) ) {
#else
			if( (isSecurityError()==True) && (isSecurityLocked() == True) ) {
#endif
				setHDDSeq( EM_HDDSeq_EracePrepare );	// 03/04/10 tsukiji 【TSS2】セキュリティ異常の装置消去
			} else {
				if (isNeedResetAngouMode() == True) {
					setHDDSeq( EM_HDDSeq_ResetAngouMode );	// 05/08/10 Div.12 k-kawa
				} else {
					setHDDSeq( EM_HDDSeq_Format );	// 03/04/10 tsukiji 【TSS2】正常なフォーマット
				}
			}
			break;
		case EM_HDDSeq_Format:					// フォーマット処理→ＩＤ書き込み
			if ( isLockPassword() == True ){
				setHDDSeq( EM_HDDSeq_FormatDisablePassword );	// 03/08/29 tsukiji 【TSS2】物理フォーマット後のパスワード消去 
			} else {
                // ★パスワードが設定されていない時
				setHDDSeq( EM_HDDSeq_FormatEnd2 );
			}
			break;
		case EM_HDDSeq_FormatDisablePassword:					// フォーマット処理→ＩＤ書き込み
			setHDDSeq( EM_HDDSeq_FormatDisablePassword_SetPass );	// 03/08/29 tsukiji 【TSS2】物理フォーマット後のパスワード消去 
			break;
		case EM_HDDSeq_FormatDisablePassword_SetPass:					// フォーマット処理→ＩＤ書き込み
			setHDDSeq( EM_HDDSeq_FormatEnd2 );
			break;
			
// 論理フォーマットシーケンス　
		case EM_HDDSeq_Format_Logical:					// フォーマット処理→ＩＤ書き込み
			setHDDSeq( EM_HDDSeq_FormatID );
			break;
		case EM_HDDSeq_FormatID:					// ＩＤ書き込み→待機状態
			setHDDSeq( EM_HDDSeq_FormatWrite_1 );
			break;
		case EM_HDDSeq_FormatWrite_1:			// 書き込みチェック１
			setHDDSeq( EM_HDDSeq_FormatRead_1 );
			break;
		case EM_HDDSeq_FormatRead_1:				// 読み出しチェック１(★)
			setHDDSeq( EM_HDDSeq_FormatWrite_2 );
			break;
		case EM_HDDSeq_FormatWrite_2:				// 書き込みチェック２
			setHDDSeq( EM_HDDSeq_FormatRead_2 );
			break;
		case EM_HDDSeq_FormatRead_2:				// 読み出しチェック２(★)
			setHDDSeq( EM_HDDSeq_MasterPassword );
			break;
		case EM_HDDSeq_MasterPassword:				// 03/04/10 tsukiji 【TSS2】マスタパスワード設定
			// ココでAbortedCommand発生するのはLockモード＆Frozenモード時だがココまでOKならそれはない
#ifdef DEF_SupportSSD
// 08/12/12 takayama CQ:313055 Taiga セキュリティ非サポートSSD対応.
// 2009/04/05   takayama CQ:316760 SSD常時接続対応.
			if (isSecuritySupport(ul_PortNo) == TRUE) {
				setHDDSeq( EM_HDDSeq_MasterPassword_SetPass );
			} else {
				setHDDSeq( EM_HDDSeq_Format_OS );
			}
#else
			setHDDSeq( EM_HDDSeq_MasterPassword_SetPass );
#endif
			break;
		case EM_HDDSeq_MasterPassword_SetPass:		// 03/04/10 tsukiji 【TSS2】マスタパスワード設定
			setHDDSeq( EM_HDDSeq_Format_OS );
			break;
		case EM_HDDSeq_Format_OS:					// 最後にOS初期化
			setHDDSeq( EM_HDDSeq_FormatEnd2 );
			break;
		case EM_HDDSeq_FormatEnd:					// 終了処理
			setHDDSeq( EM_HDDSeq_Idle );
			break;
		case EM_HDDSeq_FormatEnd2:					// 03/07/28 tsukiji 【TSS2】フォーマット終了
			setHDDSeq( EM_HDDSeq_Idle );			// シーケンスを停止（待機状態へ）
			break;

// セキュリティイレースシーケンス　
		case EM_HDDSeq_EracePrepare:			// 03/04/10 tsukiji 【TSS2】消去準備
			// どのモードで実行できる
			setHDDSeq( EM_HDDSeq_EraceUnit );
			break;
		case EM_HDDSeq_EraceUnit:				// 03/04/10 tsukiji 【TSS2】消去
			// ココでAbortedCommand発生するのはFrozenモード時なのでありえない
			setHDDSeq( EM_HDDSeq_EraceUnit_SetPass );
			break;
		case EM_HDDSeq_EraceUnit_SetPass:				// 03/04/10 tsukiji 【TSS2】消去
			// 実行後、マスタパスワード比較結果
			if( isWarrning() == True ) {
				//（マスタパスワード不一致）
				// 03/04/11 tsukiji 装置消去でＮＧなら、マスタパスワード設定してみる。設定できたら新品
 				setHDDSeq( EM_HDDSeq_EraceMaster );
			} else {
				// 装置消去できたならマスタパスワードは既に設定済みなので終了する
				setHDDSeq( EM_HDDSeq_EraceEnd );
			}
			break;
		case EM_HDDSeq_EraceEnd:				// 03/04/10 tsukiji 【TSS2】消去完了後処理
			setHDDSeq( EM_HDDSeq_Idle );
			break;
		case EM_HDDSeq_EraceMaster:				// 03/04/10 tsukiji 【TSS2】マスタパスワード再設定
			// このステートでは異常発生しない　
			setHDDSeq( EM_HDDSeq_EraceMaster_SetPass );
			break;
		case EM_HDDSeq_EraceMaster_SetPass:				// 03/04/10 tsukiji 【TSS2】マスタパスワード再設定
			// 通常の物理フォーマットを行なう
			setHDDSeq( EM_HDDSeq_Format );
			// パスワードの設定ができたら通常の物理フォーマットを行なう
			break;
			
// パスワードシーケンス　
		case EM_HDDSeq_DisablePassword:			// パスワード消去 03/03/24 tsukiji 【TSS2】
			// このステートでは異常発生しない　
			setHDDSeq( EM_HDDSeq_DisablePassword_SetPass );
			break;
		case EM_HDDSeq_DisablePassword_SetPass:			// パスワード消去 03/03/24 tsukiji 【TSS2】
			setHDDSeq( EM_HDDSeq_SetPassword );		// 続けてパスワードの設定を行なう
			break;
		case EM_HDDSeq_SetPassword:				// パスワード設定 03/03/24 tsukiji 【TSS2】
			setHDDSeq( EM_HDDSeq_SetPassword_SetPass );			//  正常でも警告発生でも終了処理へ
			break;
		case EM_HDDSeq_SetPassword_SetPass:		// パスワード設定 03/03/24 tsukiji 【TSS2】
			// 実行後、パスワード比較結果
			// パスワード不一致（終了処理で異常を返す）
			setHDDSeq( EM_HDDSeq_PasswordEnd );
			break;
		case EM_HDDSeq_PasswordEnd:				// パスワード設定完了 03/03/24 tsukiji 【TSS2】
			setHDDSeq( EM_HDDSeq_Idle );
			break;
		case EM_HDDSeq_CheckID_Write:			// フォーマットＩＤ書き込み処理 2005/07/15 added by t.sasaki
			setHDDSeq( EM_HDDSeq_CheckID_Read );
			break;
		case EM_HDDSeq_CheckID_Read:			// フォーマットＩＤ読み出し処理 2005/07/15 added by t.sasaki
			setHDDSeq( EM_HDDSeq_CheckID_End );
			break;
		case EM_HDDSeq_CheckID_End:				// フォーマットＩＤ終了(検査)処理 2005/07/15 added by t.sasaki
			setHDDSeq( EM_HDDSeq_Idle );
			break;
		case EM_HDDSeq_ResetAngouMode:				// 暗号化ASIC 暗号モード解除 2005/08/10 Div.12 k-kawa 
			setHDDSeq( EM_HDDSeq_Format );
			break;
		default:
			THRL_ThAssert( False, "CP_HDDTrans::execHDDSeq() defatult Error!!", 0, 0, 0, 0, CFA14_DetailCode0x01D2 );
			break;
	}
}
/*================================================================================================
［機能］	【SEQ】ロック解除モード【TSS2】
--------------------------------------------------------------------------------------------------
［引数］	uchar*					：パスワード指定データ領域アドレス
［戻値］	無し
［詳細］	ロック解除
［ｳｫｰｸｽﾙｰ］	未実施
================================================================================================*/
void CP_HDDTrans::func_HDDSeq_UnLockMode()
{
	bl_SecurityErrorFlag = False;	// 実行前にフラグはクリアしておく
	// ロック解除
	//dmaIDEHard.unLockMode();
}
/*================================================================================================
［機能］	【SEQ】ロック解除モード【TSS2】
--------------------------------------------------------------------------------------------------
［引数］	uchar*：パスワード指定データ領域アドレス
［戻値］	ulong ul_HDDStatus パスワードロック解除実行結果　
［詳細］	ロック解除(ユーザパスワード使用）
 [履歴]  	07/05/07 morikawa Donau初版
 			07/10/19 morikawa	Donau HDDトラブルコード見直し対応
 			09/04/05 takayama CQ:316760 SSD常時接続対応.
            13/02/14 Takatsuji	FA14のアサート文字列のリファイン
            14/09/29 Ogura		HDDミラーリング対応.
			15/01/28 Ooka		CQ.509898 HDDミラーリング有効時にスレーブ接続時のみスレーブ側処理を行う.
			18/07/25 山岸     [Eagle][制御構成変更 No.99]【ストレージ構成変更】ストレージ管理(フォーマット_ロック_暗号化) パスワード取得API変更.
================================================================================================*/
uint32_t CP_HDDTrans::func_HDDSeq_UnLockMode_SetPass( uchar* puc_WriteBuff )
{
	uint32_t ul_Ans = HDD_STATE_OK;	// 戻り値初期化　
	char cPassword[DEF_IJC_HDD_USER_PASSWORD_LEN+1];	// 終端文字分も用意
	memset( cPassword, 0x20, DEF_IJC_HDD_USER_PASSWORD_LEN );	
	cPassword[DEF_IJC_HDD_USER_PASSWORD_LEN] = '\x0'; // 終端へNULL設定
	if( puc_WriteBuff != NULL ) {
	    ST_HDDPassword* pst_HDDPassword = (ST_HDDPassword*)puc_WriteBuff;
		// バックアップしているユーザパスワードを取得して使用する
		NVDC_SystemSetting *pc_Instance = NVDC_SystemSetting::newInstance();
		if (NULL == pc_Instance) {
			setDebugHddSeq(400, 1); // NULL Check NG.
			return HDD_STATE_NG;
		}
		pc_Instance->getHddLockPassword_SPIFlash( cPassword ) ;
		// パスワード設定(ユーザのパスワード、パスワード比較時はレベルなし）
		setParameterPassword( pst_HDDPassword, DEF_PASSWORD_USER, DEF_PASSWORD_LEVEL_NON, cPassword );
		
		THRE_HddMirroring_Chanel e_MasterCh	= getMirroringMasterChanel();
		THRE_HddMirroring_Chanel e_SlaveCh	= THRE_HddMirroring_Chanel_B;
		uchar uc_DevNoMaster	= THRE_HddPortNo;
		uchar uc_DevNoSlave		= THRE_HddChBPortNo;
		getDevNoAndChNo( uc_DevNoMaster, e_MasterCh, uc_DevNoSlave, e_SlaveCh );
		
		//	HDDミラーリングが有効な場合のみ、スレーブ側のパスワード設定処理を行う.
		uint32_t ul_Ans = HDD_STATE_OK;
		if ( True == isMirroringHDD() ) {
			if (m_e_HddMirroringDeviceStatus[e_SlaveCh] == THRE_HddMirroringStatus_Normal) {
				//	マスタ／スレーブの切り替え.
				ul_Ans = THRF_sysBspHddSelectChanel( uc_DevNoSlave, e_SlaveCh );
				if ( HDD_STATE_OK == ul_Ans ) {
					ul_Ans = THRF_sysBspHddUnlock( uc_DevNoSlave, HDD_PASSWORD_USER, (uchar*)pst_HDDPassword->uc_Password );
					
					//	パスワード設定時のATA PATH THROUGHでマスタに戻らないので、戻しておく.
					THRF_sysBspHddSelectChanel( uc_DevNoMaster, e_MasterCh );
					setDebugLowLogSpeedSeq( 847, (ushort)ul_Ans );
				} else {
					setDebugLowLogSpeedSeq( 848, e_SlaveCh );
				}
			}
		}
		
		//	ミラーリングが有効な場合は、スレーブの設定に成功した場合のみマスタも設定する.
		//	ミラーリングが無効な場合は、ul_Ansが必ずHDD_STATE_OKになるので、設定を行う.
		if ( HDD_STATE_OK == ul_Ans ) {
			// パスワードデータをPIO書き込みする
			// データ書き込み
			ul_Ans = THRF_sysBspHddUnlock( uc_DevNoMaster, HDD_PASSWORD_USER, (uchar*)pst_HDDPassword->uc_Password );
		}
		
		// HDDアクセス実行結果に応じた後処理　
		convertAndSetHDDStatus( ul_Ans );
	} else {
		THRL_ThAssert( False, "func_HDDSeq_UnLockMode_SetPass() puc_WriteBuff==NULL Error!!", 0, 0, 0, 0, CFA14_DetailCode0x01D3 );
	}
	return( ul_Ans );
}

/*================================================================================================
［機能］	【SEQ】データ読み出し＆チェック：『接続』
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	ushort					：ＨＤＤステイタス(HDD_STATUS_OK,HDD_OUT_OF_SPEC)
［詳細］	ＨＤＤの構成情報を読み出し、スペックに合致したものならOK,それ以外はスペック外とする
			注意：ＨＤＤはSHORT単位ですがここではCHARでデータを扱うためアドレスが＊２となっている
［ｳｫｰｸｽﾙｰ］	実施済み：2001/1/10：森川
［履歴］	2001/10/9：実処理はdmaIDEHardで行っていたがここで直接行うように修正 
 			07/10/19 morikawa	Donau HDDトラブルコード見直し対応
            08/10/16 takayama CQ:311209 SSD 領域割り当て動的変更対応.
            09/04/05 takayama CQ:316760 SSD常時接続対応.
            09/08/02 takayama CQ:318248  SSDステータス別管理対応.
            11/05/30 takayama CQ344683対応.
            14/09/08 yamada   ZeusS 制御構成変更No.4 HDDミラーリング対応.
            14/10/14 y.morikawa	CQ504660 ミラーリング機能OFF設定の追加.
            14/11/07 y.morikawa	ミラーリング機能OFF設定のログ追加.
            15/02/25 yamada     CQ509743 主電源OFF/ONでのD010発生に対する暫定処置.
            16/03/14 yamada		D011詳細コード対応.
            16/03/22 yamada		D011リトライ削除、検出条件変更対応.
            					トラブル3回目のみD011とし、他はFA14とする.
            17/10/10 Yoneda   S800 ストレージ構成変更対応.
			19/08/26 Sky鳥居 [EagleZ+] OP_BTS-11139対応.
================================================================================================*/
ushort CP_HDDTrans::func_HDDSeq_CheckInformation()
{
// 2009/04/05   takayama CQ:316760 SSD常時接続対応.
#ifdef DEF_SupportSSD

	ushort us_result = 0;

	FuncCallstart( 25, __FUNCTION__, 1 );		// 割込みチェック用関数コールスタート.

    if((bl_endSleepHDDInit == True) &&
       (theHDDTrans.e_StartUpDeviceType == THRE_StartupDeviceType_SD)) {
		// puc_WorkMemAddrにデバイス情報を設定
		uint32_t ul_HddState = THRF_sysBspHddDeviceInfoGet( THRE_SsdPortNo, 0, (ST_HDD_DEVICE_INFO *)puc_WorkMemAddr);
		
		// HDD接続時はSSDの初期エラーを無視する.
		if( getNvRamDeviceType() == THRE_StorageDeviceTypeSSD ) { 
			// HDDアクセス実行結果に応じた後処理.
			convertAndSetHDDStatus( ul_HddState );
		}
		us_result = chkSaveIdentifyDevice( THRE_SsdPortNo, (ST_HDD_DEVICE_INFO *)puc_WorkMemAddr );
	}

	m_c_MirroringBackupData.initMirroringBackupData();	// 制御構成変更No.4 HDDミラーリング対応 不揮発データの初期化を行う.

	// 万が一、HDDの接続要求と上位モジュールからの要求が競合したケースを想定してセマフォ管理する.
	SYSF_semTake(semID_SataFwAutoRecover, WAIT_FOREVER);

	// HDD接続時はSSDの初期エラーを無視する.
    if(( getNvRamDeviceType() == THRE_StorageDeviceTypeHDDAndSSD )  ||
        (theHDDTrans.e_StartUpDeviceType == THRE_StartupDeviceType_SSD)) {

		// RAID構成情報の取得を行う.
		Bool bl_Ret = initRaidInfo();
		if (bl_Ret == False) {
			setDebugLowLogSpeedSeq(790, 0) ;
			SYSF_semGive(semID_SataFwAutoRecover);

			FuncCallend( 25, __FUNCTION__, 1 );			// 割込みチェック用関数コールエンド.
			return HDD_OUT_OF_SPEC;
		}

		// Ch-Aが接続可能かチェック.
		if (m_e_HddMirroringDeviceStatus[THRE_HddMirroring_Chanel_A] == THRE_HddMirroringStatus_Normal) {
			uint32_t ul_Cnt = 0;
			uint32_t ul_HddState  = HDD_STATE_NG;
			// 主電源ON後の1回目のHDDアクセスは、HDDデバイスファイル生成完了まで待たせる.
			while(HDD_STATE_OK != ul_HddState) {
				// Ch-A の構成情報を保存.
				ul_HddState = THRF_sysBspHddDeviceInfoGet( THRE_HddPortNo, 0, (ST_HDD_DEVICE_INFO *)puc_WorkMemAddr);
				if (ul_HddState == HDD_STATE_OK){
					us_result = chkSaveIdentifyDevice( THRE_HddPortNo, (ST_HDD_DEVICE_INFO *)puc_WorkMemAddr );
					if (us_result != HDD_STATUS_OK) {
						setDebugLowLogSpeedSeq(791, us_result) ;
						SYSF_semGive(semID_SataFwAutoRecover);

						FuncCallend( 25, __FUNCTION__, 1 );			// 割込みチェック用関数コールエンド.
						return us_result;
					}
				} else {
					uint32_t ul_RetryCnt = THRL_GetSCSICommandRetryCount();
					if (ul_RetryCnt > ul_Cnt++) {
						setDebugLowLogSpeedSeq(851, (ushort)ul_HddState) ;
						THRL_ThLogMsgErr("[func_HDDSeq_CheckInformation] Chanel-A SCSI Command NG. Retry. ul_HddState(%"PRId32"), ul_Cnt(%"PRId32")\n", ul_HddState, ul_Cnt);
						SYSF_taskDelay(200);	// 1s.
					} else {
					// C-D011(HDD_OUT_OF_SPEC)詳細コードセット.
						us_FA14DetailCode = selectOutOfSpecDetailCode(THRE_D011Point_GetHddInfoRetryOver_ChA);
						setDebugLowLogSpeedSeq(852, (ushort)ul_HddState);
						THRL_ThLogMsgErr("[func_HDDSeq_CheckInformation] Chanel-A SCSI Command NG. ul_HddState(%ld)\n", ul_HddState);
						SYSF_semGive(semID_SataFwAutoRecover);

						FuncCallend( 25, __FUNCTION__, 1 );			// 割込みチェック用関数コールエンド.
						return ul_HddState;
					}
				}
			}
		}

		// Ch-Bが接続可能かチェック.
		if (m_e_HddMirroringDeviceStatus[THRE_HddMirroring_Chanel_B] == THRE_HddMirroringStatus_Normal) {
			uint32_t ul_Cnt = 0;
			uint32_t ul_HddState  = HDD_STATE_NG;
			// 主電源ON後の1回目のHDDアクセスは、HDDデバイスファイル生成完了まで待たせる.
			while(HDD_STATE_OK != ul_HddState) {
				// Ch-B の構成情報を保存.
				ul_HddState = THRF_sysBspHddDeviceInfoGet( THRE_HddChBPortNo, 0, (ST_HDD_DEVICE_INFO *)puc_WorkMemAddr);
				if (ul_HddState == HDD_STATE_OK){
					us_result = chkSaveIdentifyDevice( THRE_HddChBPortNo, (ST_HDD_DEVICE_INFO *)puc_WorkMemAddr );
					if (us_result != HDD_STATUS_OK) {
						setDebugLowLogSpeedSeq(792, us_result) ;
						SYSF_semGive(semID_SataFwAutoRecover);

						FuncCallend( 25, __FUNCTION__, 1 );			// 割込みチェック用関数コールエンド.
						return us_result;
					}
				} else {
					uint32_t ul_RetryCnt = THRL_GetSCSICommandRetryCount();
					if (ul_RetryCnt > ul_Cnt++) {
						setDebugLowLogSpeedSeq(853, (ushort)ul_HddState) ;
						THRL_ThLogMsgErr("[func_HDDSeq_CheckInformation] Chanel-B SCSI Command NG. Retry. ul_HddState(%"PRId32"), ul_Cnt(%"PRId32")\n", ul_HddState, ul_Cnt);
						SYSF_taskDelay(200);	// 1s.
					} else {
					// C-D011(HDD_OUT_OF_SPEC)詳細コードセット.
						us_FA14DetailCode = selectOutOfSpecDetailCode(THRE_D011Point_GetHddInfoRetryOver_ChB);
						setDebugLowLogSpeedSeq(854, (ushort)ul_HddState);
						THRL_ThLogMsgErr("[func_HDDSeq_CheckInformation] Chanel-B SCSI Command NG. ul_HddState(%ld)\n", ul_HddState);
						SYSF_semGive(semID_SataFwAutoRecover);

						FuncCallend( 25, __FUNCTION__, 1 );			// 割込みチェック用関数コールエンド.
						return ul_HddState;
					}
				}
			}
		}
	} 
	else {
		// NVRAMクリアの場合、HDD情報（API)オブザーバーからHDD非装着が通知されないため、.
		// 起動時にHDD非装着の場合は未装着の場合はミラーリングをOFFにする.
		setDebugLowLogSpeedSeq(793, 0) ;

		m_c_MirroringBackupData.setMirroringOn( False );
	}

	SYSF_semGive(semID_SataFwAutoRecover);

	FuncCallend( 25, __FUNCTION__, 1 );			// 割込みチェック用関数コールエンド.

	return us_result;
#else	//DEF_SupportSSD
	// puc_WorkMemAddrにデバイス情報を設定
	uint32_t ul_HddState = THRF_sysBspHddDeviceInfoGet( 0, 0, (ST_HDD_DEVICE_INFO *)puc_WorkMemAddr);
	
	// HDDアクセス実行結果に応じた後処理　
	convertAndSetHDDStatus( ul_HddState );

	// IDENTIFY DEVICE情報より使用可能なHDDかどうか判断し必要なパラメータを保存
	return( chkSaveIdentifyDevice( (ST_HDD_DEVICE_INFO *)puc_WorkMemAddr ) );
#endif
}

/*================================================================================================
［機能］	初期ロック解除処理.
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	uint32_t	:処理正常：HDD_STATE_OK.
                         処理異常：HDD_STATE_OK以外.
［詳細］	マウント処理をする前に、デバイスがロックしているかを確認し、ロックされている場合は.
　　　　　　ロック解除を行う.　
［変更履歴］
    2018/06/11 Sky井柳 [Eagle][制御構成変更No.97 パーティション構成変更対応]新規作成.
	2018/08/09 Sky井柳 [Eagle]制御構成変更No.99 呼び出し前にデバイスチェック処理が実行されているためこの処理からは削除.
	2019/05/10 Sky井柳【IT6_1.2MR】[AR.1076986]「暗号化とロックの併用対応」ロック解除後に暗号化デバイス作成処理を追加.
================================================================================================*/
uint32_t CP_HDDTrans::func_InitUnLock()		// 初期ロック解除処理.
{
	uint32_t ul_Ans = HDD_STATE_OK;
	// HDDロックパスワード(本体はul_PortNoを使用しているがSubsetは1固定).
	if( isLockPassword() == True ) {	// Nvram.
		printf("[InitUnLock]LockPassword=true\n");
		// ロック中かの判定をIdentify Device informationより取得する方法に変更 05/10/12 tsukiji
		if( isSecurityEnable(1) == True ) {		// デバイス情報.
			// セキュリティはON 
			if( isSecurityLocked(1) == True ) {	// デバイス情報.
				printf("[InitUnLock]Device Locked\n");
				// ロック状態にあるので解除を行う 
				ul_Ans = func_HDDSeq_UnLockMode_SetPass( uc_PassBuff );
				if( HDD_STATE_OK == ul_Ans ){
					/*D391対応 マウントし直し*/
					forceReMount();
					printf("[InitUnLock]Device enc Before Remount\n");
					// 暗号化デバイス作成処理(暗号化の有無は関数内で判断).
					Bool bl_Ret = THRL_makeMainStorageEncDevice();
					if( False == bl_Ret ){
						printf("[InitUnLock] make EncDevice Error\n");
						ul_Ans = HDD_STATE_NG;// NG設定.
					}
				}
			} else {
				// スリープ復帰後ですでにUNLOCK済みの場合
				printf("[InitUnLock]Device UnLocked\n");
			}
		} else {
			printf("[InitUnLock]Device NotLock\n");
		}
	} else {
		printf("[InitUnLock]LockPassword=False\n");
		if( isSecurityEnable(1) == True ) {
			printf("[InitUnLock]Device Locked\n");
		}
	}
	return ul_Ans;
}

/*================================================================================================.
［機能］	ストレージのロック解除処理.
--------------------------------------------------------------------------------------------------.
［引数］	無し.
［戻値］	Bool	:処理正常：True.
                     処理異常：False.
［詳細］	クラス外からコールするためfunc_InitUnLock()をラッピング.
［変更履歴］.
    2018/07/31	Sky山岸 [Eagle]制御構成変更No.99 新規作成.
================================================================================================*/
Bool CP_HDDTrans::unlockStorage()
{
	static ST_HDD_DEVICE_INFO	st_DeviceInfo;

	int32_t si32_DviceId = e_StartUpDeviceType == THRE_StartupDeviceType_SD ? THRE_SsdPortNo : THRE_HddPortNo;
	uint32_t ul_HddState = THRF_sysBspHddDeviceInfoGet( si32_DviceId, 0, &st_DeviceInfo);
	convertAndSetHDDStatus( ul_HddState );
	ushort us_Result = chkSaveIdentifyDevice( si32_DviceId, &st_DeviceInfo );
	if(us_Result != HDD_STATUS_OK) {
		SYSF_logMsg("unlockStorage : chkSaveIdentifyDevice NG.\n");
		uc_checkSecurityLocked = 0;
		return False;
	}
	
	if( THRE_StartupDeviceType_SSD == e_StartUpDeviceType ){	// 起動デバイスタイプSSDの時.
		// ロック解除処理実行.
		uint32_t ui32_Result = func_InitUnLock();		// 初期ロック解除処理.
		if( ui32_Result != HDD_STATUS_OK ) {
			SYSF_logMsg("unlockStorage : func_InitUnLock NG.\n");
			uc_checkSecurityLocked = 0;
			return False;
		}
	}
	uc_checkSecurityLocked = 0;
	return True;
}

/*================================================================================================
［機能］	【SEQ】APM有効化：『接続』
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	無し
［詳細］	APMを有効状態に設定する　
［変更履歴］07/05/07 morikawa Donau初版
 			07/10/19 morikawa	Donau HDDトラブルコード見直し対応
            08/11/21 takayama CQ:311209  Taiga APMコマンド非対応デバイス対応.
            09/04/05 takayama CQ:316760 SSD常時接続対応.
================================================================================================*/
void CP_HDDTrans::func_HDDSeq_EnableAPM()
{
#ifdef DEF_SupportSSD
	if (isAPMSupport(ul_PortNo) == True ) {
		
		uint32_t ul_Ans = 0;
		
		ul_Ans = THRF_sysBspHddAPMEnable( (uchar)ul_PortNo, IDE_APM_LEVEL );
		// HDDアクセス実行結果に応じた後処理.
		convertAndSetHDDStatus( ul_Ans );
	}
	// HDD接続の場合、SSDもAPM設定させる.
	if (ul_PortNo == THRE_HddPortNo) {
		if (isAPMSupport(THRE_SsdPortNo) == True ) {
			
			uint32_t ul_Ans = 0;
			
			ul_Ans = THRF_sysBspHddAPMEnable( THRE_SsdPortNo, IDE_APM_LEVEL );
			// HDDアクセス実行結果に応じた後処理.
			convertAndSetHDDStatus( ul_Ans );
		}
	}

#else //DEF_SupportSSD.
	uint32_t ul_Ans = THRF_sysBspHddAPMEnable( 0, IDE_APM_LEVEL );
	// HDDアクセス実行結果に応じた後処理　
	convertAndSetHDDStatus( ul_Ans );
#endif
}

/*================================================================================================
［機能］	【SEQ】暗号化設定　暗号モード設定：『接続』
--------------------------------------------------------------------------------------------------
［戻値］	無し 
［詳細］	セクタカウントレジスタに暗号化モードを設定し、コマンド発行する。 
			暗号化無効時にもモード設定を実施(ASIC10の場合は必須)
［作成者］	k_ozawa
［ｳｫｰｸｽﾙｰ］	未実施
［変更履歴］05/06/20 t.sasaki 暗号化ASIC正式対応.
			05/07/15 t.sasaki 「上書き優先」時はIV固定とする.
			07/03/23 morikawa Donau初版　SATA2
			08/01/30 morikawa 常時暗号化ボードありとする
			12/02/23 toda     ファイルメモリ分割対応 不要なデバッグ処理を削除.
			14/11/05 oshima   cPP暴露保護対応.
			15/10/09 中村   [SecV2-4]SecV1時の対応をマージ.
			16/02/05 中村   CQ524977[SecV2]メインストレージの暗号化設定が機能しない.暗号化設定(SSD/HDD)の判別を、呼び出し元で参照している getHddExist() を判別に用いる.
			18/03/22 井柳 [Eagle]制御構成変更No97「ストレージ装着設定対応」Eagleでは、SSD指定固定となるため、HDD装着設定では確認しないため.
						SSDの暗号化設定の確認のみとする。（DenebMLKでは、切り分けが必要になると考えられるため、DEF_DenebMLKCusで括っておく).
================================================================================================*/
void CP_HDDTrans::func_HDDSeq_SetAngouMode()
{
	// DonauではASIC200に暗号化機能が存在するので常時暗号化ボードありとする．
	APIC_SystemInfo::newInstance()->setEncryptionBoardExist( True ) ;
	
	b_EnableEnc = False;

// IT6では、メインストレージの暗号化のみ.
#if 0 // サブストレージ暗号化対応の為に、コードを残しておく.
#ifdef DEF_DenebMLKCus
// [Eagle]Eagle機種では、SSDの場合にgetHddExistはTrueとなるが、状態はSSDと同等に扱うため、除外.
	if( True == APIC_HddInfo::newInstance()->getHddExist() ){
		b_EnableEnc = (NVDC_SystemSetting::newInstance())->getEnableEncryption();
	}
	else {
		b_EnableEnc = THRG_IsSsdEncrypt();
	}
#else // DEF_DenebMLKCus.
	b_EnableEnc = THRG_IsSsdEncrypt();
#endif //DEF_DenebMLKCus.
#else //0.
	b_EnableEnc = THRG_IsSsdEncrypt();
#endif //0.

	// イニシャルベクタは可変モードで初期化　
	b_IVFixMode = False;
	if ( NVDC_SystemSetting::newInstance()->getEncryptionOrOverWrite() == False ) {	
		// 一時データ消去上書き優先時のみ、イニシャルベクタは固定にする　
		b_IVFixMode = True;
	}
	// HDDの暗号化の基本モード設定　
	THRC_AesTransSingletonIf::newInstance().setBasicMode( b_EnableEnc, b_IVFixMode );
	
}
/*================================================================================================
［機能］	【SEQ】暗号化設定　暗号モード解除
--------------------------------------------------------------------------------------------------
［戻値］	無し 
［詳細］	暗号化の設定モードを解除する。
			この処理は、HDD廃棄時フォーマットを行うときに必要に応じて行なわれる　
［作成者］	2005/08/10 Div.12 k-kawa
［ｳｫｰｸｽﾙｰ］	未実施
［変更履歴］07/03/23 morikawa Donau初版　SATA2
================================================================================================*/
void CP_HDDTrans::func_HDDSeq_ResetAngouMode()
{
	// HDDの暗号化の基本モード設定　
	THRC_AesTransSingletonIf::newInstance().setBasicMode( False, False );
	THRL_ThLogMsgErr("Decrypt\n");
}
/*================================================================================================
［機能］	【SEQ】暗号化設定　HDD廃棄時フォーマットを行うときに暗号モード解除が必要かどうか
--------------------------------------------------------------------------------------------------
［戻値］	Bool True:必要/False:不要
［詳細］	暗号ASICの有無、暗号化有効かどうか見て、暗号モード解除の必要性を判断する
［作成者］	2005/08/10 Div.12 k-kawa
［ｳｫｰｸｽﾙｰ］	未実施
［変更履歴］
  2018/09/21 Sky井柳 【DenebMLK】制御構成変更No78「データ移行」対応 MicroSD起動時のみ暗号化対象化修正.
================================================================================================*/
Bool CP_HDDTrans::isNeedResetAngouMode()
{
	Bool bl_ret = False;
	if (us_AllDataEraseMode != 0) {	// HDD廃棄時フォーマットを行う
// IT6では、メインストレージの暗号化のみ.
#if 0 // サブストレージ暗号化対応の為に、コードを残しておく.
		if( THRE_SsdPortNo == ul_PortNo ) {
			bl_ret = THRG_IsSsdEncrypt();
		}
		else {
		//	bl_ret = (NVDC_SystemSetting::newInstance())->getEnableEncryption();;
		// SSD暗号化に備えて、コメント化.
		// 現時点(2018/09)で暗号化はMicroSDのみ.
		}
#else //0.
		bl_ret = THRG_IsSsdEncrypt();
//Str【TAS000000005012対応 : 2022.01.21】.
#ifdef DEF_SupportSSD
		return False;	// 暗号化モードは解除しない.
#endif
//End【TAS000000005012対応 : 2022.01.21】.
#endif //0.
	}
	return bl_ret;
}

// ================================================================================================
//! <!--機能--> 一時データ消去の上書き優先のための消去パターン書き込み：『接続』
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param なし 
//! <!--戻値-->
//! \return なし
//! <!--詳細-->
//! \n 一時データ消去の上書き優先のための消去パターンを予め作成しておく　
//! \n 暗号化してHDDに書き込まれたデータがセキュリティー仕様で決められたコードと一致するように
//! \n 予め、決められたコードを複合化して消去パターンとして登録しておく
//! <!--履歴-->
// 2007/03/15 morikawa	新規作成 
// ================================================================================================
void CP_HDDTrans::func_HDDSeq_MakeErasePattern()
{
#if (defined TORNADO) && (!defined FUM_LINUX)
	uchar *uc_ErasePattern = (uchar *)((uintptr_t)st_ErasePatternTbl[2].puc_PtnBuf + DEF_SECTOR_SIZE);	// 消去パターンワークエリア
#else
	uchar uc_ErasePattern[DEF_SECTOR_SIZE];	// 消去パターンワークエリア
#endif // #if (defined TORNADO) && (!defined FUM_LINUX)
	for( uint32_t ul_Index = 0; ul_Index < DEF_ERASE_PATTERNNUM; ++ul_Index ) {
		
		ST_ErasePatternTbl* pst_Tbl = &st_ErasePatternTbl[ul_Index];
		// 元のパターンコード書き込み　
		memset( uc_ErasePattern, pst_Tbl->uc_PtnCode, pst_Tbl->us_Size ) ;
		
		// 複合化実行	
		// 元のパターンコードから暗号化ありの時のパターンコードを複合化して転送　
		THRC_AesTransSingletonIf::newInstance().encodeDecodeExec( THRE_EncHDDDirectBufToMem, 0, (uintptr_t)pst_Tbl->puc_PtnBuf, (uintptr_t)uc_ErasePattern, DEF_SECTOR_SIZE, 0 );
	}
}

/*================================================================================================
［機能］	【SEQ】フォーマット情報読みだし：『接続』
--------------------------------------------------------------------------------------------------
［引数］	uchar*					：読み出し情報格納先アドレス
［戻値］	無し
［詳細］	ここで読み出しコマンドを発行して割り込みの完了で結果が格納されている
［ｳｫｰｸｽﾙｰ］	実施済み：2001/1/10：森川
 [履歴]		2008/10/16 takayama CQ:311209 SSD 領域割り当て動的変更対応.
================================================================================================*/
void CP_HDDTrans::func_HDDSeq_ReadFormat( uchar* puc_ReadBuff )
{
// 2008/10/16 takayama CQ:311209 SSD 領域割り当て動的変更対応.
#ifdef DEF_SupportSSD
	// HDD転送とAES転送を実行する　転送サイズは１セクタ分.
	transDirectHDDAndAES( THRE_ReadWriteHDDToBuf, (uintptr_t)puc_ReadBuff, ul_SysStartBlock, 1 );
#else //DEF_SupportSSD.
	// HDD転送とAES転送を実行する　転送サイズは１セクタ分　			
	transDirectHDDAndAES( THRE_ReadWriteHDDToBuf, (uintptr_t)puc_ReadBuff, DEF_HDD_SYS_BLOCK, 1 );

#endif
}
/*================================================================================================
［機能］	【SEQ】フォーマットチェック：『接続』
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	usort:チェック結果
［詳細］	フォーマットチェックの結果を返す
［ｳｫｰｸｽﾙｰ］	実施済み：2001/1/10：森川
［履歴］	07/05/14 morikawa 静的解析不具合対応　
================================================================================================*/
ushort CP_HDDTrans::func_HDDSeq_CheckFormat( const uchar* puc_ReadBuff )
{
	return checkFormat( (char*)puc_ReadBuff );
}
// ================================================================================================
//! <!--機能--> 【SEQ】読み出しチェック：『接続』『フォーマット』
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param uchar*：バッファアドレス
//! <!--戻値-->
//! \return なし
//! <!--詳細-->
//! フォーマット情報を読み出すためのHDD転送を実行する
//! dosFSのアクセスで行うことにより、暗号化あり時の処理はideRW()内で行う。
//! <!--履歴-->
//  07/03/11 morikawa	Donau初版
//  07/05/14 morikawa 静的解析不具合修正　
//  08/10/16 takayama CQ:311209 SSD 領域割り当て動的変更対応.
//  09/04/05 takayama CQ:316760 SSD常時接続対応.
// ================================================================================================
void CP_HDDTrans::func_HDDSeq_CheckRead( uchar* puc_ReadBuff )
{
	register ushort us_cylinder = 0;
	register uchar uc_head = 0;
	register uchar uc_sector = 0;
	// 読み書きテスト領域指定( LBA=0 )
// ◇ 2008/10/16 takayama CQ:311209 SSD 領域割り当て動的変更対応.
#ifdef DEF_SupportSSD
	convertLBAToCHS( ul_SysRwChkStartBlock, us_cylinder, uc_head, uc_sector );
	// 2009/04/05   takayama CQ:316760 SSD常時接続対応.
	ideRW( c_DevicePath, (int32_t)us_cylinder, (int32_t)uc_head, (int32_t)uc_sector, puc_ReadBuff, ul_SysRwChkBlockNum, EC_HDDTransDirection_Read);
#else //DEF_SupportSSD.
	convertLBAToCHS( DEF_HDD_RW_CHECK_BLOCK, us_cylinder, uc_head, uc_sector );
	ideRW( 0, (int32_t)us_cylinder, (int32_t)uc_head, (int32_t)uc_sector, puc_ReadBuff, DEF_HDD_RW_CHECK_BLOCK_NUM, EC_HDDTransDirection_Read);
#endif
}
/*================================================================================================
［機能］	【SEQ】書き込みチェック：『接続』『フォーマット』
--------------------------------------------------------------------------------------------------
［引数］	uchar*					：バッファアドレス
			ushort					：データサイズ
			uchar					：書き込みチェックデータ
［戻値］	無し
［詳細］	ワークRAM→ＨＤＤのＤＭＡを用いて転送する
［ｳｫｰｸｽﾙｰ］	実施済み：2001/1/10：森川
================================================================================================*/
// ================================================================================================
//! <!--機能--> 【SEQ】書き込みチェック：『接続』『フォーマット』
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param uchar*：バッファアドレス
//! <!--戻値-->
//! \return なし
//! <!--詳細-->
//! フォーマット情報を読み出すためのHDD転送を実行する
//! dosFSのアクセスで行うことにより、暗号化あり時の処理はideRW()内で行う。
//! <!--履歴-->
//  07/03/11 morikawa	Donau初版
//  07/05/14 morikawa 静的解析不具合修正　
//  08/10/16 takayama CQ:311209 SSD 領域割り当て動的変更対応.
//  09/04/05 takayama CQ:316760 SSD常時接続対応.
// ================================================================================================
void CP_HDDTrans::func_HDDSeq_CheckWrite( uchar* puc_WriteBuff, ushort us_DataSize, uchar uc_WriteChar )
{
	register ushort us_cylinder = 0;
	register uchar uc_head = 0;
	register uchar uc_sector = 0;
#ifdef DEF_SupportSSD
	// 読み書きテスト領域指定.
	convertLBAToCHS( ul_SysRwChkStartBlock, us_cylinder, uc_head, uc_sector );
	// 書き込みチェックデータ書き込み.
	memset( puc_WriteBuff, uc_WriteChar, us_DataSize );
	// 2009/04/05   takayama CQ:316760 SSD常時接続対応.
	ideRW( c_DevicePath, (int32_t)us_cylinder, (int32_t)uc_head, (int32_t)uc_sector, puc_WriteBuff, ul_SysRwChkBlockNum, EC_HDDTransDirection_Write);
#else //DEF_SupportSSD.
	// 読み書きテスト領域指定
	convertLBAToCHS( DEF_HDD_RW_CHECK_BLOCK, us_cylinder, uc_head, uc_sector );
	// 書き込みチェックデータ書き込み
	memset( puc_WriteBuff, uc_WriteChar, us_DataSize );
	ideRW( 0, (int32_t)us_cylinder, (int32_t)uc_head, (int32_t)uc_sector, puc_WriteBuff, DEF_HDD_RW_CHECK_BLOCK_NUM, EC_HDDTransDirection_Write);
#endif
}
/*================================================================================================
［機能］	【SEQ】終了処理：『接続』
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	無し
［詳細］	メモリ管理部へ接続が終了したことを返す
［ｳｫｰｸｽﾙｰ］	実施済み：2001/1/10：森川
			04/07/20	t.sasaki HDDセキュリティ対応(TSS2からの移植)
			09/04/21 kurachi Donau3/Amur CQ:317353対策.
						R/Wチェック時にAES排他セマフォの開放処理を追加.
			10/03/03 	yashiro Gaia問題点リストNo286(CQ324193) : セマフォID dosImageMuteSem 関連処理の削除.
			10/07/13    iizuka	ＤＯＳとの調停処理削除 (8536)
			16/11/22	Sky井柳 【IT5_4.0】AR1007320 HDD R/Wチェック時にImageタスクを経由せずにMemFuncへ.
								直接結果設定するように変更(HDD初期処理で失敗すると、ImageタスクがAssetしてしまうため).
================================================================================================*/
void CP_HDDTrans::func_HDDSeq_End()
{
	if( (CC_MEMFunctionSingletonIf::newInstance().getFuncCode() == FUNC_HD_CHECK_MODE)&&						// 診断が「HDD R/Wチェック」の時.
		(CC_MEMFunctionSingletonIf::newInstance().getHddRwCheckFuncStatus() == TYPE_MemFuncStatusNOWExec) ){	// 「HDD R/Wチェック」が実行中の時.
		if( bl_ReadWriteCheck == False ) {
			// ＨＤＤＲ／Ｗチェック異常終了.
			CC_MEMFunctionSingletonIf::newInstance().SetMemFuncResult( False );		// 診断結果（総合）を異常指定.
			CC_MEMFunctionSingletonIf::newInstance().setHddRwCheckFuncStatus( TYPE_MemFuncStatusNGEnd );// 「HDD R/Wチェック」実行状態に、異常終了を設定.
		}else{
			// ＨＤＤＲ／Ｗチェック正常終了.
			CC_MEMFunctionSingletonIf::newInstance().SetMemFuncResult( True );		// 診断結果（総合）を正常指定.
			CC_MEMFunctionSingletonIf::newInstance().setHddRwCheckFuncStatus( TYPE_MemFuncStatusOKEnd );// 「HDD R/Wチェック」実行状態に、正常終了を設定.
		}
		CC_MEMFunctionSingletonIf::newInstance().ss_ExecPercent = 100;			// 診断終了.
		// R/Wチェックの場合はAES排他セマフォを開放.
#ifdef DEF_SupportSSD
		THRC_AesTransSingletonIf::newInstance().semGiveDosDirect( getNvRamDeviceType() );
#else // DEF_SupportSSD.
		THRC_AesTransSingletonIf::newInstance().semGiveDosDirect();
#endif // DEF_SupportSSD.
	}else{
		if( bl_ReadWriteCheck == False ) {
			sendStatusNotice( HDD_TRANS_ERROR );	// 転送異常送信
		} else {
			sendConnectionNotice( HDD_STATUS_OK );	// ＨＤＤ接続完了メッセージを送信
		}
	}
}
/*================================================================================================
［機能］	【内部】セキュリティをチェック
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	無し
［履歴］	03/04/09 tsukiji 【TSS2】セキュリティ処理追加
			14/09/08 yamada   ZeusS 制御構成変更No.4 HDDミラーリング対応.
================================================================================================*/
void CP_HDDTrans::func_HDDSeq_FormatPre()
{
	Bool bl_RbldActive = getRebuildActive();

	if (bl_RbldActive != False) {
		// リビルド実行中であれば停止する.

		// 現在のマスターチャネルに応じたHDDデバイスを取得する.
		THRE_HddMirroring_Chanel e_MasterChanel = getMirroringMasterChanel();
		uchar uc_DevNo = THRE_SsdPortNo;
		TYPE_HddChanelNo e_ChanelNo = TYPE_HddChanelNo_Unknown;

		switch(e_MasterChanel) {
		case THRE_HddMirroring_Chanel_A:
			uc_DevNo = THRE_HddPortNo;
			e_ChanelNo = TYPE_HddChanelNo_Chanel_A;
			break;
		case THRE_HddMirroring_Chanel_B:
			uc_DevNo = THRE_HddChBPortNo;
			e_ChanelNo = TYPE_HddChanelNo_Chanel_B;
			break;
		default:
			setDebugLowLogSpeedSeq(800, e_MasterChanel) ;
			return;
			break;
		}

		// リビルド停止.
		uint32_t ul_Ret = THRF_sysBspStopRebuild(uc_DevNo);
		if (ul_Ret != HDD_STATE_OK) {
			setDebugLowLogSpeedSeq(801, 0) ;
		}

		// リビルド進捗率を100%に設定.
		m_uc_ProgressRatio = 100;

		// リビルド動作状態を停止中で更新.
		m_bl_RebuildRunStatus = False;

		// リビルド中止を上位へ通知.
		APIC_HddInfo* pc_HddInfo = APIC_HddInfo::newInstance();
		if (pc_HddInfo == NULL) {
			setDebugLowLogSpeedSeq(802, 0) ;
			return;
		}

		TYPS_HDDMirroringEventInfo st_EventInfo;
		st_EventInfo.e_Event    = TYPE_HDDMirroringEvent_RebuildStop;	// リビルド中止.
		st_EventInfo.e_ChanelNo = e_ChanelNo;							// マスターチャネル.

		pc_HddInfo->notifyHDDMirroringEvent(st_EventInfo);
	}

	return;
}

/*================================================================================================
［機能］	【SEQ】フォーマット処理（クリア）：『フォーマット』
--------------------------------------------------------------------------------------------------
［引数］	uc_devType		デバイスタイプ
［戻値］	無し
［詳細］	フォーマット処理を起動する（物理フォーマット・廃棄)
［履歴］	2018/06/12  ishiga   AR.1047260 再設計.
			2018/11/01  iyanagi  [Eagle][AR1062347] フォーマット中フラグを落とす処理の追加.
			2018/12/07 SCD2-Sky Kawanishi [EagleH][SecV4][RQ-674][OP_BTS-931] アンマウント時にジョブログ停止/開始する処理削除(システム上位側で実施).
			2019/02/14  iyanagi  [IT6_1.0MR][AR1071479] FAT情報書き込み処理の追加.
================================================================================================*/
// 消去領域格納構造体.
typedef struct _THRS_EraseArea {
	uint64_t ul_StartSector;	// 先頭セクタ.
	uint64_t ul_EndSector;		// 終端セクタ.
} THRS_EraseArea;
#define THRD_EraseAreaMax	(30)// 消去領域最大数.
void CP_HDDTrans::func_HDDSeq_Format( uchar uc_devType )
{
	THRS_EraseArea s_EraseArea[THRD_EraseAreaMax];		// 消去対象領域.
	int32_t si_EraseCount = 0;							// 消去対象数.

	printf("[%s] Start(%d).\n", __FUNCTION__, uc_devType);

	// ストレージタイプ:SSDのみサポート.
	if( uc_devType != THRE_StorageDeviceTypeSSD ) {
		printf("[%s] Not Suppert Device.\n", __FUNCTION__);
		return;
	}

	// フォーマット制御に必要な情報.
	TYPE_StorageConfig e_storage_type = THRG_GetStorageType();
	const part_layout* ps_device_info = THRG_GetPartitionType();
	const char *pc_device_path = theHDDTrans.pc_strDeviceSSD;	// デバイスパスは、SSD,μSD共通.

	// 消去領域を取得.
	memset( s_EraseArea, 0, sizeof(s_EraseArea) );
	si_EraseCount = func_HDDSeq_Format_GetEraseArea( e_storage_type, ps_device_info, s_EraseArea, THRD_EraseAreaMax );

	// 消去実行.
	func_HDDSeq_Format_ExecErase( pc_device_path, s_EraseArea, si_EraseCount );
	
	// 消去後のFAT情報書き込み.
	if( False == func_HDDSeq_Format_FatWriteAfterErase( ) ) {
		printf("[%s] Fat write failed.\n", __FUNCTION__);
	}

	// AR1012671対応 タスクモニタ自動保存との調停 .
	uchar uc_Cnt=0;
	for ( uc_Cnt = 0; ( uc_Cnt < THRD_DspTaskMoniterMaxWait ) && ( guc_TaskMoniterDsp_State == THRD_TaskMoniterDsp_Exec ); uc_Cnt++ ) {
		SYSF_taskDelay( 200 );	// 1秒Wait.
	}
	printf("[%s] wait count %u \n", __FUNCTION__, uc_Cnt);

	// AR1012671対応 タスクモニタ自動保存との調停.
	gb_umount_exec = True;		//umount中を実行.

	// 論理フォーマット.
	if( False == func_HDDSeq_Format_LogicalFmtAfterErase( ps_device_info ) ) {
		printf("[%s] logical format failed.\n", __FUNCTION__);
	}

	delayTask( 1000 );			// 5秒Wait.

	// フォーマットチェック書込み.
	if( False == func_HDDSeq_Format_WriteChkIjcData( e_storage_type ) ) {
		printf("[%s] check_ijc_data failed.\n", __FUNCTION__);
	}

	// パターンチェックデータの書き込み.
	if( False == THRF_HWCheck_TestPatternWriteToSSD() ) {
		printf("[%s] TestPattern write failed.\n", __FUNCTION__);
	}

	// IDをWriteする.
	writeID_Direct();

//Str【TAS000000005012対応 : 2022.01.21】.
	// SPI-Flash暗号化設定情報クリア.
	APIC_SystemSetting::newInstance()->reqMainStorageEnc( False );
//End【TAS000000005012対応 : 2022.01.21】.

	// フォーマット処理終了となるため、フォーマット中フラグをおとす.
	APIC_SystemSetting *pc_SystemSetting = APIC_SystemSetting::newInstance();
	if( pc_SystemSetting != NULL ){
		uchar	uc_stat = (uchar)pc_SystemSetting->getFsStat();
		uc_stat = (uchar)(uc_stat & ~TYPD_SystemFsStatFormatting) ;
		pc_SystemSetting->setFsStat(uc_stat);
		printf("[%s] set FsStat=[0x%02X]\n", __FUNCTION__, uc_stat );
	}

	// Sync.
	THRF_DoSync();

	// AR1012671対応 タスクモニタ自動保存との調停 mount後にata0aへの書き込みがあるので、それが完了するまでは待つ(異なるプロセスで同時にアクセスするのを防ぐ為).
	gb_umount_exec = False;		//umount中を解除.

	printf("[%s] Finish.\n", __FUNCTION__);
}

/*================================================================================================
［機能］	【SUB】消去領域(セクタ）の取得：『物理フォーマット』
--------------------------------------------------------------------------------------------------
［引数］	e_StorageType	[I N] ストレージタイプ.
［引数］	p_DeviceInfo	[I N] パーティションテーブル.
［引数］	ps_EraseArea	[OUT] 消去領域テーブル.
［引数］	si_AreaMaxCnt	[I N] 消去領域最大数.
［戻値］	int32_t		テーブル格納数.
［詳細］	物理フォーマット時の消去領域を取得する.
		  ■消去対象領域.
			パーティションタイプ ( PART_FSTYPE_FS )
			パーティションタイプ ( PART_FSTYPE_FS_SSD )
			  パーティション全体消去
			パーティションタイプ ( PART_FSTYPE_RAW )
			  第1パーティション(パーティション情報領域以外)
				フォーマットID領域から後ろを全て消去する。蓄積画像FAT領域含
			  第10パーティション(SuperWarp領域)
				SWAP画像/FAT領域含
			  第12パーティション(画像領域)
				SSD256GB、SSD1TBの場合に、蓄積画像の先頭から後ろを全て消去
［履歴］	18/06/12 ishiga   AR.1047260 新規作成.
================================================================================================*/
#define THRD_DEVPATH_PART_FIRST	THRM_MOUNT_DEV( THRD_DEVFILE_SSD, 1 )	// 先頭パーティション.
#define THRD_DEVPATH_SUPERWARP	THRM_MOUNT_DEV( THRD_DEVFILE_SSD, 10 )	// SuperWarpのパーティション.
#define THRD_DEVPATH_PART_IMAGE	THRM_MOUNT_DEV( THRD_DEVFILE_SSD, 12 )	// 蓄積画像パーティション.
// 時短(デバッグ用).
static Bool bl_debug_erase_area_compact = False;
extern "C" void debug_HDDSeq_Format_compact( int flg ) {
	if ( flg > 0 ) {
		bl_debug_erase_area_compact = True;
	} else {
		bl_debug_erase_area_compact = False;
	}
}
static inline void debug_erase_area_compact( uint64_t& start, uint64_t& end ) {
#define THRD_EraseAreaCompactSize (100000)
	if ( bl_debug_erase_area_compact ) {
		if( (end - start + 1) > THRD_EraseAreaCompactSize ) {
			end = start + THRD_EraseAreaCompactSize - 1;
		}
	}
}
// 処理.
int32_t CP_HDDTrans::func_HDDSeq_Format_GetEraseArea(
		TYPE_StorageConfig e_StorageType,
		const part_layout* p_DeviceInfo,
		THRS_EraseArea* ps_EraseArea,
		int32_t si_AreaMaxCnt
) {
	if( NULL == p_DeviceInfo || NULL == ps_EraseArea ) {
		printf("[%s] pointer is null.\n", __FUNCTION__);
		return 0;
	}

	int32_t si_Idx = 0;
	int32_t si_EraseCnt = 0;
	for( si_Idx = 0; p_DeviceInfo[si_Idx].type != PART_TYPE_END; si_Idx++ ){
		switch( p_DeviceInfo[si_Idx].fstype ){
			case PART_FSTYPE_FS:
			case PART_FSTYPE_FS_SSD:
				if( 0 < si_Idx ) {
					ps_EraseArea[si_EraseCnt].ul_StartSector = p_DeviceInfo[si_Idx-1].end + 1 + 63;	// パーティション管理情報を除く.
					ps_EraseArea[si_EraseCnt].ul_EndSector = p_DeviceInfo[si_Idx].end;
					debug_erase_area_compact(ps_EraseArea[si_EraseCnt].ul_StartSector, ps_EraseArea[si_EraseCnt].ul_EndSector);
					si_EraseCnt++;
					if( us_debug_MemCF > 0 ) {
						printf( "erase partition: idx:%d type:%d devpath:%s mntpath:%s  sector:all\n",
								si_Idx, p_DeviceInfo[si_Idx].fstype, p_DeviceInfo[si_Idx].devpath[0], p_DeviceInfo[si_Idx].mntpnt);
					}
				} else {
					// 第1パーティションがFS, FS_SSDとなることはない.
				}
				break;
			case PART_FSTYPE_RAW:
				if( 0 == strcmp( p_DeviceInfo[si_Idx].devpath[0], THRD_DEVPATH_PART_FIRST ) ) {
					// フォーマットID領域から後ろを全て消去する。蓄積画像FAT領域含.
					ps_EraseArea[si_EraseCnt].ul_StartSector = ul_SysStartBlock;
					ps_EraseArea[si_EraseCnt].ul_EndSector = p_DeviceInfo[si_Idx].end;
					debug_erase_area_compact(ps_EraseArea[si_EraseCnt].ul_StartSector, ps_EraseArea[si_EraseCnt].ul_EndSector);
					si_EraseCnt++;
					if( us_debug_MemCF > 0 ) { printf( "erase partition: idx:%d type:%d(RAW) sector:%"PRIu64" - \n", si_Idx, p_DeviceInfo[si_Idx].fstype, ps_EraseArea[si_EraseCnt-1].ul_StartSector); }
				} else if( 0 == strcmp( p_DeviceInfo[si_Idx].devpath[0], THRD_DEVPATH_SUPERWARP ) ) {
					// SuperWarp領域、SWAP画像/FAT領域含
					ps_EraseArea[si_EraseCnt].ul_StartSector = p_DeviceInfo[si_Idx-1].end + 1 + 32;	// 本来はLBAの指定セクタから消去するべきだが、固定値(SuperWarpブートフラグ領域)で行う.
					ps_EraseArea[si_EraseCnt].ul_EndSector = p_DeviceInfo[si_Idx].end;
					debug_erase_area_compact(ps_EraseArea[si_EraseCnt].ul_StartSector, ps_EraseArea[si_EraseCnt].ul_EndSector);
					si_EraseCnt++;
					if( us_debug_MemCF > 0 ) { printf( "erase partition: idx:%d type:%d(RAW) sector:%"PRIu64" - \n", si_Idx, p_DeviceInfo[si_Idx].fstype, ps_EraseArea[si_EraseCnt-1].ul_StartSector); }
				} else if( 0 == strcmp( p_DeviceInfo[si_Idx].devpath[0], THRD_DEVPATH_PART_IMAGE ) ) {
					switch( e_StorageType ){
						case TYPE_StorageConfig_SSD_256GB:		// ストレージデバイス構成SSD256GB時.
						case TYPE_StorageConfig_SSD_1TB:		// ストレージデバイス構成SSD1TB時.
							// 蓄積画像の先頭から後ろを全て消去.
							ps_EraseArea[si_EraseCnt].ul_StartSector = ul_StoreStartBlock;
							ps_EraseArea[si_EraseCnt].ul_EndSector = p_DeviceInfo[si_Idx].end;
							debug_erase_area_compact(ps_EraseArea[si_EraseCnt].ul_StartSector, ps_EraseArea[si_EraseCnt].ul_EndSector);
							si_EraseCnt++;
							if( us_debug_MemCF > 0 ) { printf( "erase partition: idx:%d type:%d(RAW) sector:%"PRIu64" - \n", si_Idx, p_DeviceInfo[si_Idx].fstype, ps_EraseArea[si_EraseCnt-1].ul_StartSector); }
							break;
						default:
							break;
					}
				} else { // 他は対象外.
				}
				break;
			default:
				break;
		}
		// 上限チェック.
		if( si_EraseCnt >= si_AreaMaxCnt ) {
			break;
		}
	}
	return si_EraseCnt;
}

/*================================================================================================
［機能］	【SUB】消去実行：『物理フォーマット』
--------------------------------------------------------------------------------------------------
［引数］	pc_device_path	[I N] デバイスパス.
［引数］	ps_EraseArea	[I N] 消去領域テーブル.
［引数］	si_AreaMaxCnt	[I N] 消去テーブル数.
［戻値］	無し.
［詳細］	物理フォーマット時の消去を実行する.
［履歴］	18/06/12 ishiga   AR.1047260 新規作成.
			19/08/26 Sky鳥居 [EagleZ+] OP_BTS-11139対応.
================================================================================================*/
void CP_HDDTrans::func_HDDSeq_Format_ExecErase(
	const char* pc_device_path,
	const THRS_EraseArea* ps_EraseArea,
	int32_t si_EraseCount
) {
// 1回に書き込むセクタ数.
#define THRD_OneEraseSectorNum		256
// デバッグ用 何MB毎にログ出力するかの単位 128MB単位.
#define THRD_DebugEraceSecLogUnit	(256*8*128)		//  1セクタ=512B　256セクタ=0.125MB　256セクタ*8=1MB.

	int32_t si_Index;
	uint64_t ul_TotalSectorNum = 0;						// 総セクタサイズ.
	uint64_t ul_EraseTotalNum = 0;
	uint32_t ui_percent_current = 0;
	uint32_t ui_128MB_count = 0;

	if( NULL == ps_EraseArea ) {
		printf("[%s] pointer is null.\n", __FUNCTION__);
		return;
	}
	FuncCallstart( 26, __FUNCTION__, 1 );		// 割込みチェック用関数コールスタート.

	// 消去する総セクタ数.
	for( si_Index = 0; si_Index < si_EraseCount; si_Index++ ) {
		ul_TotalSectorNum += ( ps_EraseArea[si_Index].ul_EndSector - ps_EraseArea[si_Index].ul_StartSector + 1);
		printf( "ps_EraseArea[%"PRId32"] %"PRIu64" - %"PRIu64" \n", si_Index, ps_EraseArea[si_Index].ul_StartSector, ps_EraseArea[si_Index].ul_EndSector);
	}
	printf( "ul_TotalSectorNum %"PRIu64"\n", ul_TotalSectorNum);

	// 0x00 で上書き.
	printf("Now writing '0x00'.\n");
	memset( puc_WorkMemAddr, 0x00, DEF_HDD_FORMATDATA_SIZE );

	// 消去実行.
	for( si_Index = 0; si_Index < si_EraseCount; si_Index++ ) {
		uint64_t ul_SectorNo = ps_EraseArea[si_Index].ul_StartSector;
		uint64_t ul_RemainNum = ps_EraseArea[si_Index].ul_EndSector - ul_SectorNo + 1;
		if( us_debug_MemCF > 0 ) {
			printf( "Erasing... [%"PRId32"] CurrentSector=%"PRIu64"\n", si_Index, ul_SectorNo);
		}
		while( ul_RemainNum > 0 ) {
			// 書込みセクタ数.
			uint64_t ul_EraseNum = (THRD_OneEraseSectorNum < ul_RemainNum)? THRD_OneEraseSectorNum: ul_RemainNum;
			// 書込み.
			readWriteExecHDD( (char *)pc_device_path, THRE_ReadWriteBufToHDD, 0, (uintptr_t)puc_WorkMemAddr, (uint32_t)ul_SectorNo, (uint32_t)ul_EraseNum );
			// 進捗更新.
			ul_SectorNo += ul_EraseNum;
			ul_RemainNum -= ul_EraseNum;
			ul_EraseTotalNum += ul_EraseNum;

			// 128MB単位でログを出力する.
			if( us_debug_MemCF > 0 ) {
				uint32_t ui_128MB_temp = (uint32_t)(ul_EraseTotalNum / THRD_DebugEraceSecLogUnit);
				if ( ui_128MB_count < ui_128MB_temp ) {
					ui_128MB_count = ui_128MB_temp;
					if( ul_RemainNum > 0 ) {
						printf( "Erasing... [%"PRId32"] CurrentSector=%"PRIu64"\n", si_Index, ul_SectorNo);
					}
				}
			}
			// 進捗更新.
			uint32_t ui_percent = (uint32_t)((ul_EraseTotalNum * 100) / ul_TotalSectorNum);
			if( ui_percent > ui_percent_current ) {
				printf( "... %"PRIu32"%% Erased\n", ui_percent);
				sendStatusNotice( ui_percent );
				ui_percent_current = ui_percent;
				delayTask( 1 );		// TaskImageにメッセージを送信するため他のタスクが動作できるようにタスク遅延する.
			}
		}
	}

	// 消去完了終了時に100%にする.
	if( ui_percent_current < 100 ) {
		printf( "... 100%% erased\n");
		sendStatusNotice( 100 );
	}
	FuncCallend( 26, __FUNCTION__, 1 );			// 割込みチェック用関数コールエンド.
}

#include "system/sys/sysg/SYSC_SwapControl.h"	// EagleH_制御構成変更_No.49 swap有効化対応(swap有効化遅延実行対応).
/*================================================================================================
［機能］	【SUB】消去後の論理フォーマット：『物理フォーマット』
--------------------------------------------------------------------------------------------------
［引数］	p_DeviceInfo	[I N] パーティションテーブル.
［戻値］	Bool			True:成功 False:失敗.
［詳細］	物理フォーマット時の消去後に論理フォーマットを実行する.
［履歴］	18/06/12 ishiga   AR.1047260 新規作成.
			19/02/28 ads長谷川 EagleH_制御構成変更_No.49 swap有効化対応(swapファイル化対応).
			19/04/08 ads長谷川 EagleH_制御構成変更_No.49 swap有効化対応(swap有効化遅延実行対応).
			21/06/16 ads河井  該当マウントポイントの変換済フラグを未変換へ変更処理追加.
================================================================================================*/
Bool CP_HDDTrans::func_HDDSeq_Format_LogicalFmtAfterErase(
	const part_layout* p_DeviceInfo
) {
	int32_t si_Idx = 0;
	int32_t si_Ret = 0;
	int32_t si_Retry = 0;
	uchar uc_encrypt = THRG_IsSsdEncrypt()? 1 : 0;
	Bool bl_Ret = True;
	char sc_cmd[64];

	if( NULL == p_DeviceInfo ) {
		printf("[%s] pointer is null.\n", __FUNCTION__);
		return False;
	}

	// 「一般ユーザーアクセス変換管理ファイル」を読み込む.
	std::map<std::string, Bool> tmpctrlFileMap;
	bl_Ret = readMfpAccessCtrlFile(tmpctrlFileMap);
	if( bl_Ret == False ){
		printf( "[%s:%d] readMfpAccessCtrlFile NG.\n", __PRETTY_FUNCTION__, __LINE__ );
		return bl_Ret;
	}
	
	for( si_Idx = 1; PART_TYPE_END != p_DeviceInfo[si_Idx].type; si_Idx++ ) {		// 第1パーティションは論理フォーマット対象外なので、ループには含めない.
		if( ( PART_FSTYPE_FS == p_DeviceInfo[si_Idx].fstype )
		 || ( PART_FSTYPE_FS_SSD == p_DeviceInfo[si_Idx].fstype ) ) {

			const char *pc_devpath = p_DeviceInfo[si_Idx].devpath[uc_encrypt];
			const char *pc_mntpnt = p_DeviceInfo[si_Idx].mntpnt;
#if defined (DEF_64BIT_PF)	// EagleH_制御構成変更_No.49 swap有効化対応(swapファイル化対応)↓.
			if( strcmp(p_DeviceInfo[ si_Idx ].mntpnt, THRD_DevName_Ssd0a) == 0 ){	// ata0a判定.
				SYSC_SwapOffLock();		// swapをoffする(swap有効化遅延実行対応).
			}
#endif	// EagleH_制御構成変更_No.49 swap有効化対応(swapファイル化対応)↑.
			// アンマウント.
			for( si_Retry = 0; si_Retry <= THRD_UMOUNT_RETRYMAX; si_Retry++ ) {
				si_Ret = umount2( pc_mntpnt, MNT_FORCE );
				if ( 0 == si_Ret ) {
					break;
				} else if( -1 == si_Ret ) {
					delayTask( 10+(si_Retry*50) );	//初回10msecで回を重ねるごとに+50msec追加.
				}
			}
			if( si_Retry > THRD_UMOUNT_RETRYMAX ) {
				//アンマウントエラー時に、アクセス情報出力する.
				printf("[%s] umount error: %s\n", __FUNCTION__, pc_mntpnt);
				memset( sc_cmd, 0, sizeof(sc_cmd) );
				sprintf( sc_cmd, "/usr/bin/lsof | grep %s", pc_mntpnt );
				system( sc_cmd );
			}

			// 暗号化状態に応じてフォーマット先変更するように変更.
			si_Ret = formatFS( (char*)pc_devpath );
			if( -1 == si_Ret ) {
				printf( "[%s] format ERR(%s)\n", __FUNCTION__, pc_devpath );
				bl_Ret = False;
				break;
			}

		 	// フォーマットしたマウントポイントと一致するデータの変換済フラグを未変換に変更する.
			std::map<std::string, Bool>::iterator itrCtrlFile = tmpctrlFileMap.find( pc_mntpnt );
			if( itrCtrlFile != tmpctrlFileMap.end() ){
				itrCtrlFile->second = False;
			}
			
			// 論理フォーマットと同じ制御を行う為、フォーマット後にmountを行う.
			if( PART_FSTYPE_FS == p_DeviceInfo[si_Idx].fstype ) {
				si_Ret = mount( pc_devpath, pc_mntpnt, "ext3", MS_NOATIME, EXT3_MOUNT_OPTIONS );
			} else {
				si_Ret = mount( pc_devpath, pc_mntpnt, "ext2", MS_NOATIME, NULL );
			}
			if( -1 == si_Ret ) {
				printf("[%s] mount Error.%s -> %s errno:%d\n", __FUNCTION__, pc_devpath, pc_mntpnt, SYSF_errnoGet());
				bl_Ret = False;
				break;
			}
#if defined (DEF_64BIT_PF)	// EagleH_制御構成変更_No.49 swap有効化対応(swapファイル化対応)↓.
			if( strcmp(p_DeviceInfo[ si_Idx ].mntpnt, THRD_DevName_Ssd0a) == 0 ){	// ata0a判定.
				SYSC_SwapOnUnlock();		// swapをONする(swap有効化遅延実行対応).
			}
#endif	// EagleH_制御構成変更_No.49 swap有効化対応(swapファイル化対応)↑.

			// 本体FW起動に必要なディレクトリを作成する.
			if( False == createDirSystemNeed( pc_mntpnt ) ) {
				printf( "[%s] mkdir Error.%s errno:%d\n", __FUNCTION__, pc_mntpnt, SYSF_errnoGet());
				bl_Ret = False;
				break;
			}
		}
	}
	// 「一般ユーザーアクセス変換管理ファイル」を更新する.
	bl_Ret = writeMfpAccessCtrlFile(tmpctrlFileMap);
	if( bl_Ret == False ){
		printf( "[%s:%d] writeMfpUserCtrlFile NG.\n", __PRETTY_FUNCTION__, __LINE__ );
		return bl_Ret;
	}

	return bl_Ret;
}

/*================================================================================================
［機能］	【SUB】消去後のフォーマットチェックデータの書込み：『物理フォーマット』
--------------------------------------------------------------------------------------------------
［引数］	e_StorageType	[I N] ストレージタイプ.
［戻値］	Bool			True:成功 False:失敗.
［詳細］	物理フォーマット時の消去後にフォーマットチェックデータの書込む.
［履歴］	18/06/12 ishiga   AR.1047260 新規作成.
			18/10/16 Sky	  AR.1062659 不正アクセスでSegFになる問題修正.
================================================================================================*/
Bool CP_HDDTrans::func_HDDSeq_Format_WriteChkIjcData(
		TYPE_StorageConfig e_StorageType
) {
	FILE *fp_ijc = NULL;
	Bool bl_Ret = True;
	char *pc_ijc_dir;
	char *pc_ijc_data;
	THRE_StorageDeviceType uc_deviceType = getNvRamDeviceType();
	if(( uc_deviceType == THRE_StorageDeviceTypeSSD ) || ( theHDDTrans.e_StartUpDeviceType == THRE_StartupDeviceType_SD )) {
		// uSD起動時
		pc_ijc_dir = DEF_THR_DIR_PATH_SSD;		// /ata0a/ijc.
		pc_ijc_data = THRD_FileName_IJC_SSD;	// /ata0a/ijc/chk_ijc.data.
	} else {
		// SSD起動時
		pc_ijc_dir = DEF_THR_DIR_PATH;		// /ata01/ijc.
		pc_ijc_data = THRD_FileName_IJC;	// /ata01/ijc/chk_ijc.data.
	}

	do {
		// ディレクトリ生成.
		if( OK == SYSF_mkdir( (char*)pc_ijc_dir ) ) {
			// ファイル書込み.
			fp_ijc = SYSF_Fopen( pc_ijc_data, "we" );
			if( NULL != fp_ijc ) {
				// 暗号化情報をセット.
				memset( uc_ChkFormatHddData, 0x0, sizeof(uc_ChkFormatHddData) );	// 使用前初期化.
				memcpy( uc_ChkFormatHddData, uc_NonAngo, sizeof(uc_ChkFormatHddData) );
				// 書込み.
				SYSF_Fwrite( &uc_ChkFormatHddData[0], sizeof(uc_ChkFormatHddData), 1, fp_ijc );
				// テーブルをすべて保存 
				SYSF_Fclose( fp_ijc );
			} else {
				printf("write open error (%s)!!\n", pc_ijc_data );
				bl_Ret = False;
				break;
			}

			// Syncする.
			THRF_DoSync();

			// ファイル読込み.
			fp_ijc = SYSF_Fopen( pc_ijc_data, "re" );
			if( NULL != fp_ijc ) {
				memset( uc_ChkFormatHddData, 0x0, sizeof( uc_ChkFormatHddData ));	// 使用前初期化.
				SYSF_Fread( &uc_ChkFormatHddData, sizeof(uc_ChkFormatHddData), 1, fp_ijc );
				if( 0 != strcmp( (char*)uc_ChkFormatHddData, (char*)uc_NonAngo ) ){
					printf(" compare check error.\n");
					bl_Ret = False;
				}
				SYSF_Fclose( fp_ijc );
			} else {
				printf("read open error (%s)!!\n", pc_ijc_data );
				bl_Ret = False;
			}
		} else {
			printf("mkdir error (%s)!!\n", pc_ijc_dir );
			bl_Ret = False;
		}
	} while(0);

	return bl_Ret;
}

/*================================================================================================
［機能］	消去後のFAT情報書き込み：『物理フォーマット』
--------------------------------------------------------------------------------------------------
［引数］	なし.
［戻値］	Bool			True:成功 False:失敗.
［詳細］	物理フォーマット時の消去後にのFAT情報（蓄積領域）を書き込む.
			ただし、書き込み処理にエラー判定戻り値がないため、失敗チェックは.
			FAT情報のNULLチェックのみ.
［履歴］	2019/02/14  iyanagi  [IT6_1.0MR][AR1071479] 新規作成.
================================================================================================*/
Bool CP_HDDTrans::func_HDDSeq_Format_FatWriteAfterErase()
{
	// comMemoryManagerのインスタンス生成.
	IMGC_ImageMemoryPoolImpColorFileMemoryInstance& comMemoryManager = IMGC_ImageMemoryPoolImpColorFileMemorySingletonIf::newInstance();

	// FAT情報の初期化.
	comMemoryManager.initInternalFat();

	// 蓄積FAT情報の取得.
	CC_Fat* pcc_Fat = comMemoryManager.getFatPtr( IMGE_IPEType_HDDStore );			// 蓄積画像用FAT.
	CC_Fat* pcc_InfoFat = comMemoryManager.getInfoFatPtr( IMGE_IPEType_HDDStore );	// 画像蓄積管理用FAT.
	SEM_ID semID_Mutex = comMemoryManager.getFatSemID( IMGE_IPEType_HDDStore );		// 蓄積用セマフォID.

	// FAT情報のNULLチェック.
	if( (pcc_Fat == NULL) || (pcc_InfoFat == NULL) ){
		// FAT情報がNULLの場合は、以降の処理がAssertするので、異常終了としておく.
		return False;
	}

	// 蓄積領域の有無をチェック
	if( pcc_Fat->getMaxClusterNum() != 0 ){	// 領域サイズがない場合は、書き込むデータはない.
		st_MsgHDDTransInfo.em_HDD_RWMode = EM_HDD_RWMode_StoreFatWrite;// 蓄積FAT書き込み指定.
	 	startHDDTransFatWrite( pcc_Fat, pcc_InfoFat, &semID_Mutex );	// 書き込み処理
	}

	return True;// 書き込み処理は、エラーが取れないので、正常終了としておく.
}

/*================================================================================================
［機能］	【SEQ】フォーマット処理(開始)：『論理フォーマット』
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	無し
［詳細］	フォーマット処理を起動する（論理フォーマット）
［履歴］	03/07/28 tsukiji 【TSS2】論理フォーマットシーケンス化のため追加
			14/09/08 yamada   ZeusS 制御構成変更No.4 HDDミラーリング対応.
================================================================================================*/
void CP_HDDTrans::func_HDDSeq_Format_Logical()
{
	Bool bl_RbldActive = getRebuildActive();

	if (bl_RbldActive != False) {
		// リビルド実行中であれば停止する.
		// 現在のマスターチャネルに応じたHDDデバイスを取得する.
		THRE_HddMirroring_Chanel e_MasterChanel = getMirroringMasterChanel();
		uchar uc_DevNo = THRE_SsdPortNo;
		TYPE_HddChanelNo e_ChanelNo = TYPE_HddChanelNo_Unknown;

		switch(e_MasterChanel) {
		case THRE_HddMirroring_Chanel_A:
			uc_DevNo = THRE_HddPortNo;
			e_ChanelNo = TYPE_HddChanelNo_Chanel_A;
			break;
		case THRE_HddMirroring_Chanel_B:
			uc_DevNo = THRE_HddChBPortNo;
			e_ChanelNo = TYPE_HddChanelNo_Chanel_B;
			break;
		default:
			setDebugLowLogSpeedSeq(795, e_MasterChanel) ;
			return;
			break;
		}

		// リビルド停止.
		uint32_t ul_Ret = THRF_sysBspStopRebuild(uc_DevNo);
		if (ul_Ret != HDD_STATE_OK) {
			setDebugLowLogSpeedSeq(796, 0) ;
		}

		// リビルド進捗率を100%に設定.
		m_uc_ProgressRatio = 100;

		// リビルド動作状態を停止中で更新.
		m_bl_RebuildRunStatus = False;

		// リビルド中止を上位へ通知.
		APIC_HddInfo* pc_HddInfo = APIC_HddInfo::newInstance();
		if (pc_HddInfo == NULL) {
			setDebugLowLogSpeedSeq(797, 0) ;
			return;
		}

		TYPS_HDDMirroringEventInfo st_EventInfo;
		st_EventInfo.e_Event    = TYPE_HDDMirroringEvent_RebuildStop;	// リビルド中止.
		st_EventInfo.e_ChanelNo = e_ChanelNo;							// マスターチャネル.

		pc_HddInfo->notifyHDDMirroringEvent(st_EventInfo);
	}

	return;
}

/*================================================================================================
［機能］	【SEQ】フォーマット処理（ID書き込み）：『フォーマット』
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	無し
［詳細］	物理フォーマットするかわりに先頭セクタへ識別コードを書き込む
			ワークRAM→ＨＤＤのＤＭＡを用いて転送する
［ｳｫｰｸｽﾙｰ］	実施済み：2001/1/10：森川
 [履歴]		2008/10/16 takayama CQ:311209 SSD 領域割り当て動的変更対応.
            2016/02/01 A.Gouya  [ZeusS_ZX0] CQ.524543 フォーマットID書き込み後の書き込み確認及びリトライ処理の追加.
			2016/01/26 nakamura [SecV2-5]HDD暗号化鍵管理制御
			2016/02/05 nakamura CQ524977[SecV2]メインストレージの暗号化設定が機能しない.
			2019/04/05 Sky井柳  【IT6_1.2MR】制御構成変更No10「SSD暗号化対応」HDD暗号化鍵管理制御の削除.
								セキュリティFUMに確認し不要となったため、削除.
			19/08/26 Sky鳥居 [EagleZ+] OP_BTS-11139対応.
================================================================================================*/
void CP_HDDTrans::func_HDDSeq_FormatID()
{
	char System_Data[] = IDE_FORMAT_STRING;
	uint32_t ul_writeFmtIDTimes = 0 ;

	FuncCallstart( 27, __FUNCTION__, 1 );		// 割込みチェック用関数コールスタート.

	while ( ul_writeFmtIDTimes < THRD_CHK_FORMATID_MAX ){  //フォーマットID書き込み後確認（リトライ最大3回）.
		memcpy( puc_WorkMemAddr, System_Data, IDE_FORMAT_STRING_LEN  );
// ◇ 2008/10/16 takayama CQ:311209 SSD 領域割り当て動的変更対応.
#ifdef DEF_SupportSSD
		// HDD転送とAES転送を実行する　転送サイズは１セクタ分.
		transDirectHDDAndAES( THRE_ReadWriteBufToHDD, (uintptr_t)puc_WorkMemAddr, ul_SysStartBlock, 1 );
#else //DEF_SupportSSD.
		// HDD転送とAES転送を実行する　転送サイズは１セクタ分.　			
		transDirectHDDAndAES( THRE_ReadWriteBufToHDD, (uintptr_t)puc_WorkMemAddr, DEF_HDD_SYS_BLOCK, 1 );
#endif
		//書き込み後確認処理.
		THRF_DoSync();
		func_HDDSeq_ReadFormat( puc_WorkMemAddr );
		if( func_HDDSeq_CheckFormat( puc_WorkMemAddr ) == HDD_STATUS_OK ) {
			bl_writeID_Direct = True;// 書き込み成功を保持.
			THRL_ThLogMsgErr("%"PRId32"time format ID write OK\n", ul_writeFmtIDTimes + 1);
			break ;
		} else {
			THRL_ThLogMsgErr("%"PRId32"time format ID write NG\n", ul_writeFmtIDTimes + 1);
		}
		ul_writeFmtIDTimes++ ;
	}
	//新詳細ログに出力する.
	if ( ul_writeFmtIDTimes >= THRD_CHK_FORMATID_MAX) {
		uint64_t		ull_NewDetailData = THRD_NewDetail_ModuleId_Thread;		// 1,2桁：スレッド.
		ull_NewDetailData |= (uint64_t)( TYPD_TroubleCode_HDD_NOT_INITIALIZED ) << THRD_NewDetail_TroubleCode_Shift;	// 3-6桁：トラブルコード.
		ull_NewDetailData |= (uint64_t)( 1 ) << 32;			// 7桁：フォーマットID書き込み状態 0:正常1 1:異常.
		THRL_ThAssertLog2( 0, ull_NewDetailData );
	}
	FuncCallend( 27, __FUNCTION__, 1 );			// 割込みチェック用関数コールエンド.
}

/*================================================================================================
［機能］	【SEQ】パスワード設定（マスタ）
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	無し
［詳細］	【TSS2】パスワードを設定する【TSS2】
================================================================================================*/
void CP_HDDTrans::func_HDDSeq_SetMasterPassword()
{
	bl_SecurityErrorFlag = False;	// 実行前にフラグはクリアしておく
	// PIO書き込み終わるまで割り込みは発生しないので強制的にシーケンスは進める
}
/*================================================================================================
［機能］	【SEQ】パスワード設定（マスタ）
--------------------------------------------------------------------------------------------------
［引数］	uchar*					：パスワード設定領域（512バイト分必要)
［戻値］	無し
［詳細］	【TSS2】パスワードを設定する
［履歴］	07/03/23 morikawa SATA2
 			07/10/19 morikawa	Donau HDDトラブルコード見直し対応
 			08/03/10 sunami		パスワード隠蔽対応 
 			09/04/05 takayama CQ:316760 SSD常時接続対応.
            13/02/14 Takatsuji	FA14のアサート文字列のリファイン
            14/09/17 Ogura		HDDミラーリングが有効な場合にはスレーブ側も設定する.
			15/01/28 Ooka		CQ.509898 HDDミラーリング有効時にスレーブ接続時のみスレーブ側処理を行う.
================================================================================================*/
#ifdef __cplusplus
extern "C" {
char* GetMasterPassword( void );
}
#else 
extern	char* GetMasterPassword( void );
#endif  /* __cplusplus */
void CP_HDDTrans::func_HDDSeq_SetMasterPassword_SetPass( uchar* puc_WriteBuff )
{
	if( puc_WriteBuff != NULL ) {
	    ST_HDDPassword* pst_HDDPassword = (ST_HDDPassword*)puc_WriteBuff;
		// パスワード設定パラメータ(マスタのパスワード、レベルはMAX）
		THRL_ThAssert( ( GetMasterPassword() != NULL ), "CP_HDDTrans::func_HDDSeq_SetMasterPassword_SetPass(): GetMasterPassword NULL error." , 0, 0, 0, 0, CFA14_DetailCode0x01D4 );
		setParameterPassword( pst_HDDPassword, DEF_PASSWORD_MASTER, DEF_PASSWORD_LEVEL_MAX, GetMasterPassword() );
		
		THRE_HddMirroring_Chanel e_MasterCh	= getMirroringMasterChanel();
		THRE_HddMirroring_Chanel e_SlaveCh	= THRE_HddMirroring_Chanel_B;
		uchar uc_DevNoMaster	= THRE_HddPortNo;
		uchar uc_DevNoSlave		= THRE_HddChBPortNo;
		getDevNoAndChNo( uc_DevNoMaster, e_MasterCh, uc_DevNoSlave, e_SlaveCh );
		
		//	HDDミラーリングが有効な場合のみ、スレーブ側のパスワード設定処理を行う.
		uint32_t ul_Ans = HDD_STATE_OK;
		if ( True == isMirroringHDD() ) {
			// スレーブが接続されているとき
			if (m_e_HddMirroringDeviceStatus[e_SlaveCh] == THRE_HddMirroringStatus_Normal) {
				//	マスタ／スレーブの切り替え.
				ul_Ans = THRF_sysBspHddSelectChanel( uc_DevNoSlave, e_SlaveCh );
				if ( HDD_STATE_OK == ul_Ans ) {
					ul_Ans = THRF_sysBspHddPasswordSet( uc_DevNoSlave, HDD_PASSWORD_MASTER, (uchar*)pst_HDDPassword->uc_Password, HDD_SECURITY_LEVEL_MAXIMUM );
					
					//	パスワード設定時のATA PATH THROUGHでマスタに戻らないので、戻しておく.
					THRF_sysBspHddSelectChanel( uc_DevNoMaster, e_MasterCh );
					setDebugLowLogSpeedSeq( 841, (ushort)ul_Ans );
				} else {
					setDebugLowLogSpeedSeq( 842, e_SlaveCh );
				}
			}
		}
		
		//	ミラーリングが有効な場合は、スレーブの設定に成功した場合のみマスタも設定する.
		//	ミラーリングが無効な場合は、ul_Ansが必ずHDD_STATE_OKになるので、設定を行う.
		if ( HDD_STATE_OK == ul_Ans ) {
			// パスワードデータをPIO書き込みする
			// データ書き込み
			ul_Ans = THRF_sysBspHddPasswordSet( uc_DevNoMaster, HDD_PASSWORD_MASTER, (uchar*)pst_HDDPassword->uc_Password, HDD_SECURITY_LEVEL_MAXIMUM );
		}
		// HDDアクセス実行結果に応じた後処理　
		convertAndSetHDDStatus( ul_Ans );
	} else {
		THRL_ThAssert( False, "CP_HDDTrans::func_HDDSeq_SetMasterPassword_SetPass() puc_WriteBuff NULL Error!!", 0, 0, 0, 0, CFA14_DetailCode0x01D5 );
	}
}
/*================================================================================================
［機能］	【SEQ】フォーマット処理終了：『フォーマット』（Ｒ／Ｗチェック処理終了も行う）
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	無し
［ｳｫｰｸｽﾙｰ］	実施済み：2001/1/10：森川
［履歴］02/01/30 morikawa RWチェックがNGの場合は完了通知のパラメータではなくて、状態通知メッセージで
						エラーを送信するように修正。HDDのR/Wチェックの終了処理追加　
		02/04/09 morikawa フォーマットを２つに分けたことに伴う、フォーマット終了条件判定修正　
================================================================================================*/
void CP_HDDTrans::func_HDDSeq_FormatEnd()
{
	if( bl_ReadWriteCheck == False ) {
		// 読み書きで異常発生　
		sendStatusNotice( HDD_TRANS_ERROR );
	} else {
		if( CC_MEMFunctionSingletonIf::newInstance().getFuncCode() == FUNC_HD_CHECK_MODE ) { 
			// ＨＤＤＲ／Ｗチェック正常終了　
			sendMessage( TID_Image, TM_HDDRWCompleteNotice, 0 );
		} else {
			// ＨＤＤフォーマット正常終了　
			sendMessage( TID_Image, TM_HDDFormatCompleteNotice, (uintptr_t)HDD_STATUS_OK );
		}	
	}
}
/*================================================================================================
［機能］	【SEQ】フォーマット処理：『論理フォーマット』
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	無し
［詳細］	フォーマット処理を起動する（論理フォーマット）
［履歴］	03/07/28 tsukiji 【TSS2】論理フォーマットシーケンス化のため追加
================================================================================================*/
void CP_HDDTrans::func_HDDSeq_Format_OS()
{
// 処理なし.
}

/*================================================================================================
［機能］	【SEQ】フォーマット処理終了：『論理＆物理フォーマット』
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	無し
［履歴］03/07/28 tsukiji 【TSS2】論理フォーマット終了処理
		10/03/03 yashiro Gaia問題点リストNo286(CQ324193) : セマフォID dosImageMuteSem 関連処理の削除.
		14/09/08 yamada   ZeusS 制御構成変更No.4 HDDミラーリング対応.
================================================================================================*/
void CP_HDDTrans::func_HDDSeq_FormatEnd2()
{
	// ミラーリングのエラーが発生していればエラーの解除を行う.
	Bool bl_IsMirroring = isMirroringHDD();

	if (bl_IsMirroring != False) {
		m_c_MirroringBackupData.clrMirroringBackupDataError();			// 不揮発データの更新(個別:全エラークリア);

		// 自動リビルド許可.
		// 現在のマスターチャネルに応じたHDDデバイスを取得する.
		THRE_HddMirroring_Chanel e_MasterChanel = getMirroringMasterChanel();
		TYPE_HddChanelNo e_ChanelNo = TYPE_HddChanelNo_Unknown;

		switch(e_MasterChanel) {
		case THRE_HddMirroring_Chanel_A:
			e_ChanelNo = TYPE_HddChanelNo_Chanel_A;
			break;
		case THRE_HddMirroring_Chanel_B:
			e_ChanelNo = TYPE_HddChanelNo_Chanel_B;
			break;
		default:
			setDebugLowLogSpeedSeq(805, e_MasterChanel) ;
			return;
			break;
		}

		// リビルド進捗率を100%に設定.
		m_uc_ProgressRatio = 100;

		// 各チャネルのステータスを正常に設定.
		m_e_HddMirroringDeviceStatus[THRE_HddMirroring_Chanel_A] = THRE_HddMirroringStatus_Normal;
		m_e_HddMirroringDeviceStatus[THRE_HddMirroring_Chanel_B] = THRE_HddMirroringStatus_Normal;

		// ミラーリングエラー解除をを上位へ通知.
		APIC_HddInfo* pc_HddInfo = APIC_HddInfo::newInstance();
		if (pc_HddInfo == NULL) {
			setDebugLowLogSpeedSeq(807, 0) ;
			return;
		}

		TYPS_HDDMirroringEventInfo st_EventInfo;
		st_EventInfo.e_Event    = TYPE_HDDMirroringEvent_ErrorCancel;	// エラー解除.
		st_EventInfo.e_ChanelNo = e_ChanelNo;							// マスターチャネル.

		pc_HddInfo->notifyHDDMirroringEvent(st_EventInfo);

		// リビルド動作状態を停止中で更新.
		m_bl_RebuildRunStatus = False;
	}

	// ＨＤＤ論理＆物理フォーマット正常終了.
	sendMessage( TID_Image, TM_HDDFormatCompleteNotice, (uintptr_t)HDD_STATUS_OK );
	// 03/10/07 tsukiji 【TSS2】シーケンス上ロックしていたら解放（異常他で中断）. 
}
/*================================================================================================
［機能］	【SEQ】消去準備
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	無し
［詳細］	【TSS2】装置消去の準備
［履歴］	07/03/23 morikawa SATA2
 			07/10/19 morikawa	Donau HDDトラブルコード見直し対応
 			09/04/05 takayama CQ:316760 SSD常時接続対応.
================================================================================================*/
void CP_HDDTrans::func_HDDSeq_EracePrepare()
{
	// データ読みだし
#ifdef DEF_SupportSSD
// 2009/04/05   takayama CQ:316760 SSD常時接続対応.
	uint32_t ul_Ans = THRF_sysBspHddErasePrepare( (uchar)ul_PortNo );
#else
	uint32_t ul_Ans = THRF_sysBspHddErasePrepare( 0 );
#endif
	// HDDアクセス実行結果に応じた後処理　
	convertAndSetHDDStatus( ul_Ans );
}
/*================================================================================================
［機能］	【SEQ】装置消去
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	無し
［詳細］	【TSS2】装置消去(40GBで40分ほどかかる)
［履歴］	07/03/23 morikawa SATA2
			07/09/21 morikawa func_HDDSeq_EraceUnit_SetPass()で一括して行うことになったのでここでの処理は不要　
================================================================================================*/
void CP_HDDTrans::func_HDDSeq_EraceUnit()
{
}
/*================================================================================================
［機能］	【SEQ】装置消去（データ設定）
--------------------------------------------------------------------------------------------------
［引数］	uchar*					：パスワード設定領域（512バイト分必要)
［戻値］	無し
［詳細］	【TSS2】装置消去(40GBで40分ほどかかる)
［履歴］	07/03/23 morikawa SATA2
			07/09/21 morikawa Mosel以前の機種では、１：装置消去コマンド送信、２：パスワード設定という順番であったが
							DonauではBSP側で装置消去コマンドのパラメータでマスターパスワード設定を一度に行う
							従って、この関数で装置消去コマンドを送信すると同時にマスタパスワード設定を行う。
 			07/10/19 morikawa	Donau HDDトラブルコード見直し対応
 					バッファがNULLの時はありえない処理なのでアサートに修正　
  			08/03/10 sunami		パスワード隠蔽対応 
  			09/04/05 takayama CQ:316760 SSD常時接続対応.
            13/02/14 Takatsuji	FA14のアサート文字列のリファイン
================================================================================================*/
void CP_HDDTrans::func_HDDSeq_EraceUnit_SetPass( uchar* puc_WriteBuff )
{
	if( puc_WriteBuff != NULL ) {
	    ST_HDDPassword* pst_HDDPassword = (ST_HDDPassword*)puc_WriteBuff;
		// パスワード設定(マスタのパスワード、パスワード比較時はレベルなし）
		THRL_ThAssert( ( GetMasterPassword() != NULL ), "CP_HDDTrans::func_HDDSeq_EraceUnit_SetPass(): GetMasterPassword NULL error." , 0, 0, 0, 0, CFA14_DetailCode0x01D6 );
		setParameterPassword( pst_HDDPassword, DEF_PASSWORD_MASTER, DEF_PASSWORD_LEVEL_NON, GetMasterPassword() );
		// 装置消去コマンドを送信すると同時にマスタパスワード設定を行う
#ifdef DEF_SupportSSD
// 2009/04/05   takayama CQ:316760 SSD常時接続対応.
		uint32_t ul_Ans = THRF_sysBspHddEraseUnit( (uchar)ul_PortNo, HDD_PASSWORD_MASTER, (uchar*)pst_HDDPassword->uc_Password );
#else
		uint32_t ul_Ans = THRF_sysBspHddEraseUnit( 0, HDD_PASSWORD_MASTER, (uchar*)pst_HDDPassword->uc_Password );
#endif
		// HDDアクセス実行結果に応じた後処理　
		convertAndSetHDDStatus( ul_Ans );
	} else {
		THRL_ThAssert( False, "CP_HDDTrans::func_HDDSeq_EraceUnit_SetPass() puc_WriteBuff NULL Error!!", 0, 0, 0, 0, CFA14_DetailCode0x01D7 );
}
}
/*================================================================================================
［機能］	【SEQ】消去終了処理
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	無し
［詳細］	【TSS2】
================================================================================================*/
void CP_HDDTrans::func_HDDSeq_EraceEnd()
{
	// ＮＶＲＡＭユーザパスワードを消す
	// ＨＤＤフォーマット正常終了　
	sendMessage( TID_Image, TM_HDDFormatCompleteNotice, (uintptr_t)HDD_STATUS_OK );
}
/*================================================================================================
［機能］	【SEQ】パスワード設定（ユーザ）
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	無し
［詳細］	【TSS2】パスワードを設定する
 [履歴]     03/10/08 tsukiji 【TSS2】一連の動作中にDOSの処理が行なわれるとまずいのでプロテクト 
			06/06/01 tsukiji セマフォ処理追加(入れ忘れ)
			07/05/14 morikawa 静的解析不具合修正　
			10/03/03 yashiro Gaia問題点リストNo286(CQ324193) : セマフォID dosImageMuteSem 関連処理の削除.
================================================================================================*/
void CP_HDDTrans::func_HDDSeq_SetPassword()
{
	if( isCommandExec() == True ) {
		setDebugHighSpeedSeq(99, 14);
	}
	bl_CommandExecFlag = True;
	// 03/04/10 tsukiji パスワードが設定されていない場合は実行しない
}
/*================================================================================================
［機能］	【SEQ】パスワード設定（ユーザ）
--------------------------------------------------------------------------------------------------
［引数］	uchar*					：パスワード設定領域（512バイト分必要)
			uchar*					：パスワード文字列(ASCIIZ)
［戻値］	無し
［詳細］	【TSS2】パスワードを設定する
［履歴］	04/02/04 tsukiji 【TSS2】#1セキュリティ為パスワード表示を中止 
			07/03/23 morikawa SATA2対応 
 			07/10/19 morikawa	Donau HDDトラブルコード見直し対応
 			09/04/05 takayama CQ:316760 SSD常時接続対応.
 			14/09/17 Ogura		HDDミラーリングが有効な場合は、スレーブ側も設定する.
			14/11/21 y.morikawa CQ.504528 HDDパスワードロック中に本体の電源をOFFすると再起動後C-FA14が発生する.
			15/01/28 Ooka		CQ.509898 HDDミラーリング有効時にスレーブ接続時のみスレーブ側処理を行う.
			18/05/24 Sky小倉	制御変更99 【ストレージ構成変更】ストレージ管理(フォーマット/ロック/暗号化) .
================================================================================================*/
void CP_HDDTrans::func_HDDSeq_SetPassword_SetPass( uchar* puc_WriteBuff, uchar* puc_Password )
{
	bl_CommandExecFlag = True;
	
	// 2018/05/24 Sky 制御変更99.
	// SSD起動の場合、areaInitValueはDeviceTypeSSDを通るので、PortNoもSSDになる.
	if ( THRE_SsdPortNo != ul_PortNo )
	{
		return;
	}
	
	// 03/04/10 tsukiji パスワードが設定されていない場合は実行しない
	if( (puc_WriteBuff != NULL) && (puc_Password!=NULL) && (strlen((char*)puc_Password)>0) ) {
	    ST_HDDPassword* pst_HDDPassword = (ST_HDDPassword*)puc_WriteBuff;
		// パスワード設定パラメータ(ユーザのパスワード、レベルはMAX）
		setParameterPassword( pst_HDDPassword, DEF_PASSWORD_USER, DEF_PASSWORD_LEVEL_MAX, (char*)puc_Password );
		
		THRE_HddMirroring_Chanel e_MasterCh	= getMirroringMasterChanel();
		THRE_HddMirroring_Chanel e_SlaveCh	= THRE_HddMirroring_Chanel_B;
		uchar uc_DevNoMaster	= THRE_HddPortNo;
		uchar uc_DevNoSlave		= THRE_HddChBPortNo;
		getDevNoAndChNo( uc_DevNoMaster, e_MasterCh, uc_DevNoSlave, e_SlaveCh );
		
		//	HDDミラーリングが有効な場合のみ、スレーブ側のパスワード設定処理を行う.
		uint32_t ul_Ans = HDD_STATE_OK;
		if ( True == isMirroringHDD() ) {
			// スレーブが接続されているとき
			if (m_e_HddMirroringDeviceStatus[e_SlaveCh] == THRE_HddMirroringStatus_Normal) {
				//	マスタ／スレーブの切り替え.
				ul_Ans = THRF_sysBspHddSelectChanel( uc_DevNoSlave, e_SlaveCh );
				if ( HDD_STATE_OK == ul_Ans ) {
					// HDDロックパスワードがすでに設定されている場合は、一旦解除(ロック解除→パスワード解除).
					if ( isSecurityLocked( uc_DevNoSlave ) == True ) {
						if ( THRF_sysBspHddUnlock( uc_DevNoSlave, HDD_PASSWORD_USER, (uchar*)pst_HDDPassword->uc_Password ) != HDD_STATE_OK ) {
							THRL_ThLogMsgErr( "HddUnlock NG DevNoSlave=[%d]\n", uc_DevNoSlave );
						}
					}
					if ( isSecurityEnable( uc_DevNoSlave ) == True ) {
						if ( THRF_sysBspHddDisablePassword( uc_DevNoSlave, HDD_PASSWORD_USER, (uchar*)pst_HDDPassword->uc_Password ) != HDD_STATE_OK ) {
							THRL_ThLogMsgErr( "DisablePassword NG DevNoSlave=[%d]\n", uc_DevNoSlave );
						}
					}
					ul_Ans = THRF_sysBspHddPasswordSet( uc_DevNoSlave, HDD_PASSWORD_USER, (uchar*)pst_HDDPassword->uc_Password, HDD_SECURITY_LEVEL_MAXIMUM );
					//	パスワード設定時のATA PATH THROUGHでマスタに戻らないので、戻しておく.
					THRF_sysBspHddSelectChanel( uc_DevNoMaster, e_MasterCh );
					setDebugLowLogSpeedSeq( 843, (ushort)ul_Ans );
				} else {
					setDebugLowLogSpeedSeq( 844, e_SlaveCh );
				}
			}
		}
		
		//	ミラーリングが有効な場合は、スレーブの設定に成功した場合のみマスタも設定する.
		//	ミラーリングが無効な場合は、ul_Ansが必ずHDD_STATE_OKになるので、設定を行う.
		if ( HDD_STATE_OK == ul_Ans ) {
			// HDDロックパスワードがすでに設定されている場合は、一旦解除(ロック解除→パスワード解除).
			if ( isSecurityLocked( uc_DevNoMaster ) == True ) {
				if ( THRF_sysBspHddUnlock( uc_DevNoMaster, HDD_PASSWORD_USER, (uchar*)pst_HDDPassword->uc_Password ) != HDD_STATE_OK ) {
					THRL_ThLogMsgErr( "HddUnlock NG DevNoMaster=[%d]\n", uc_DevNoMaster );
				}
			}
			if ( isSecurityEnable( uc_DevNoMaster ) == True ) {
				if ( THRF_sysBspHddDisablePassword( uc_DevNoMaster, HDD_PASSWORD_USER, (uchar*)pst_HDDPassword->uc_Password ) != HDD_STATE_OK ) {
					THRL_ThLogMsgErr( "DisablePassword NG DevNoMaster=[%d]\n", uc_DevNoMaster );
				}
			}
			// パスワードデータをPIO書き込みする
			ul_Ans = THRF_sysBspHddPasswordSet( uc_DevNoMaster, HDD_PASSWORD_USER, (uchar*)pst_HDDPassword->uc_Password, HDD_SECURITY_LEVEL_MAXIMUM );
		}
		// HDDアクセス実行結果に応じた後処理　
		convertAndSetHDDStatus( ul_Ans );
	} else {
		THRL_ThLogMsgErr("Password Set NG\n");
	}
}
/*================================================================================================
［機能］	【SEQ】パスワード解除（ユーザ）
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	無し
［詳細］	【TSS2】パスワードを解除する
 [履歴]		10/03/03 yashiro Gaia問題点リストNo286(CQ324193) : セマフォID dosImageMuteSem 関連処理の削除.
================================================================================================*/
void CP_HDDTrans::func_HDDSeq_DisablePassword()
{
	bl_CommandExecFlag = True;
}
/*================================================================================================
［機能］	【SEQ】パスワード解除（ユーザ）
--------------------------------------------------------------------------------------------------
［引数］	uchar*					：パスワード設定領域（512バイト分必要)
			uchar*					：パスワード文字列(ASCIIZ)
［戻値］	無し
［詳細］	【TSS2】パスワードを解除する
［履歴］	04/02/04 tsukiji 【TSS2】#1セキュリティ為パスワード表示を中止 
		 	07/03/23 morikawa SATA2対応
		 	07/07/17 morikawa BSP関数への引数追加　
 			07/10/19 morikawa	Donau HDDトラブルコード見直し対応
 			09/04/05 takayama CQ:316760 SSD常時接続対応.
 			14/09/17 Ogura		HDDミラーリングが有効な場合は、スレーブ側も解除する.
			15/01/28 Ooka		CQ.509898 HDDミラーリング有効時にスレーブ接続時のみスレーブ側処理を行う.
================================================================================================*/
void CP_HDDTrans::func_HDDSeq_DisablePassword_SetPass( uchar* puc_WriteBuff, uchar* puc_Password)
{
	bl_CommandExecFlag = True;
	// 03/04/10 tsukiji パスワードが設定されていない場合は実行しない
	if( (puc_WriteBuff != NULL) && (puc_Password!=NULL) && (strlen((char*)puc_Password)>0) ) {
		ST_HDDPassword* pst_HDDPassword = (ST_HDDPassword*)puc_WriteBuff;
		// パスワード設定パラメータ(ユーザのパスワード、パスワード比較時はレベルなし）
		setParameterPassword( pst_HDDPassword, DEF_PASSWORD_USER, DEF_PASSWORD_LEVEL_NON, (char*)puc_Password );
		
		THRE_HddMirroring_Chanel e_MasterCh	= getMirroringMasterChanel();
		THRE_HddMirroring_Chanel e_SlaveCh	= THRE_HddMirroring_Chanel_B;
		uchar uc_DevNoMaster	= THRE_HddPortNo;
		uchar uc_DevNoSlave		= THRE_HddChBPortNo;
		getDevNoAndChNo( uc_DevNoMaster, e_MasterCh, uc_DevNoSlave, e_SlaveCh );
		
		//	HDDミラーリングが有効な場合のみ、スレーブ側のパスワード設定処理を行う.
		uint32_t ul_Ans = HDD_STATE_OK;
		if ( True == isMirroringHDD() ) {
			// スレーブが接続されているとき
			if (m_e_HddMirroringDeviceStatus[e_SlaveCh] == THRE_HddMirroringStatus_Normal) {
				//	マスタ／スレーブの切り替え.
				ul_Ans = THRF_sysBspHddSelectChanel( uc_DevNoSlave, e_SlaveCh );
				if ( HDD_STATE_OK == ul_Ans ) {
					ul_Ans = THRF_sysBspHddDisablePassword( uc_DevNoSlave, HDD_PASSWORD_USER, (uchar*)pst_HDDPassword->uc_Password );
					
					//	パスワード設定時のATA PATH THROUGHでマスタに戻らないので、戻しておく.
					THRF_sysBspHddSelectChanel( uc_DevNoMaster, e_MasterCh );
					setDebugLowLogSpeedSeq( 845, (ushort)ul_Ans );
				} else {
					setDebugLowLogSpeedSeq( 846, e_SlaveCh );
				}
			}
		}
		
		//	ミラーリングが有効な場合は、スレーブの設定に成功した場合のみマスタも設定する.
		//	ミラーリングが無効な場合は、ul_Ansが必ずHDD_STATE_OKになるので、設定を行う.
		if ( HDD_STATE_OK == ul_Ans ) {
			// パスワードデータをPIO書き込みする
			// 07/03/23 morikawa SATA2対応
			ul_Ans = THRF_sysBspHddDisablePassword( uc_DevNoMaster, HDD_PASSWORD_USER, (uchar*)pst_HDDPassword->uc_Password );
		}
		// HDDアクセス実行結果に応じた後処理　
		convertAndSetHDDStatus( ul_Ans );
	} else {
		THRL_ThLogMsgErr("Password Disable NG\n");
	}
}
/*================================================================================================
［機能］	【SEQ】パスワード解除（フォーマット時の独自）
--------------------------------------------------------------------------------------------------
［引数］	uchar*					：パスワード設定領域（512バイト分必要)
			uchar*					：パスワード文字列(ASCIIZ)
［戻値］	無し
［詳細］	【TSS2】ユーザパスワードを解除する
 [履歴]     03/10/08 tsukiji 【TSS2】一連の動作中にDOSの処理が行なわれるとまずいのでプロテクト 
			03/10/09 tsukiji 【TSS2】変数名称変更password 
			03/10/28 tsukiji 【TSS2】パスワード拡張 
			18/07/25 山岸     [Eagle][制御構成変更 No.99]【ストレージ構成変更】ストレージ管理(フォーマット_ロック_暗号化) パスワード取得API変更.
================================================================================================*/
void CP_HDDTrans::func_HDDSeq_FormatDisablePassword( uchar* puc_PassBuff )
{
	char cPassword[DEF_IJC_HDD_USER_PASSWORD_LEN+1];	// 終端分も用意 
	memset( cPassword, 0x20, DEF_IJC_HDD_USER_PASSWORD_LEN );	
	cPassword[DEF_IJC_HDD_USER_PASSWORD_LEN] = '\0'; // 終端へNULL設定
	NVDC_SystemSetting *pc_Instance = NVDC_SystemSetting::newInstance();
	if (NULL == pc_Instance) {
		setDebugHddSeq(400, 2); // NULL Check NG.
		return;
	}
	pc_Instance->getHddLockPassword_SPIFlash( cPassword ) ;
	// ユーザパスワード解除
	func_HDDSeq_DisablePassword_SetPass( puc_PassBuff, (uchar*)cPassword );
}
/*================================================================================================
［機能］	【SEQ】パスワード設定終了
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	無し
［詳細］	【TSS2】パスワードを設定する
［履歴］	07/10/19 morikawa Donau HDDトラブルコード見直し対応に伴い、エラー発生時の処理はコマンド実行時に行う　
			10/03/03 yashiro Gaia問題点リストNo286(CQ324193) : セマフォID dosImageMuteSem 関連処理の削除.
================================================================================================*/
void CP_HDDTrans::func_HDDSeq_PasswordEnd()
{
	pst_MsgSetPassword = NULL;
	// ＨＤＤパスワード設定完了通知（正常）　
	sendMessage( TID_Image, TM_HDDSetPasswordNotice, (uintptr_t)HDD_STATUS_OK );
}
/*================================================================================================
［機能］	【SEQ】パスワードパラメータ設定【TSS2】
--------------------------------------------------------------------------------------------------
［引数］	ST_HDDPassword*					：ＨＤＤパスワード構造体
			uchar							：ユーザモード(0:User,1:Master)
			uchar							：レベル(0:Non,1:High,2:Max) 
											  Security Set Passwordで使用それ以外の場合はNon固定で
			uchar*							：パスワード（8byteまでの文字列ASCIIZ)
［戻値］	無し
［詳細］	パスワード情報を設定する。この情報でパスワードを設定したり、セキュリティ解除したりする
［ｳｫｰｸｽﾙｰ］	未実施
［履歴］	04/02/04 tsukiji 【TSS2】#1セキュリティ為パスワード表示を中止 
			04/07/20 t.sasaki ユーザパスワードに接尾語をセット
			04/10/26 tsukiji 静的解析ツールでダメ出しされるので順番入れ替え(問題ではない)
			07/05/14 morikawa 静的解析不具合対応　
            13/02/14 Takatsuji	FA14のアサート文字列のリファイン
================================================================================================*/
void CP_HDDTrans::setParameterPassword( ST_HDDPassword* pst_HDDPassword, uchar uc_UserMode,  uchar uc_Level, char* pc_Password )
{
	_memset( pst_HDDPassword, DEF_SECTOR_SIZE );                // 全使用領域を予めクリアしておく
	// パスワードのユーザモード指定以外はユーザモードで動作する
// Rhein1/3 start 2004.10.26 tsukiji 静的解析ツールでダメ出しされるので順番入れ替え(問題ではない)
	// デフォルトユーザパスワードとする
	pst_HDDPassword->us_Control = IDE_SECURITY_PASSWORD_USER;
	switch( uc_UserMode ) {
	case DEF_PASSWORD_MASTER:
	    pst_HDDPassword->us_Control = IDE_SECURITY_PASSWORD_MASTER;  // マスターパスワード
		break;
	case DEF_PASSWORD_USER:
	    pst_HDDPassword->us_Control = IDE_SECURITY_PASSWORD_USER;    // ユーザパスワード
		break;
	default:
		break;
	}
// Rhein1/3 end 2004.10.26 tsukiji 静的解析ツールでダメ出しされるので順番入れ替え(問題ではない)
	// レベル指定は実際のセキュリティパスワード設定時のみ有効、モード変更時のパスワード指定では指定しない
	switch( uc_Level ) {
	case DEF_PASSWORD_LEVEL_HIGH:
	    pst_HDDPassword->us_Control |= IDE_SECURITY_LEVEL_HIGH;		// セキュリティレベル(高)
		break;
	case DEF_PASSWORD_LEVEL_MAX:
	    pst_HDDPassword->us_Control |= IDE_SECURITY_LEVEL_MAX;		// セキュリティレベル(最大)
		break;
	case DEF_PASSWORD_LEVEL_NON:
	default:	// レベル設定しない
		break;
	}
	// パスワードを32バイト分スペースで埋める
    memset( pst_HDDPassword->uc_Password, 0x20, IDE_SECURITY_PASSWORD_LENGTH );  // パスワードの設定
	// 指定パスワード文字列をパスワード領域へコピー（終端NULL等はコピーしない）
	int32_t len = strlen( pc_Password );
	if( len > IDE_SECURITY_PASSWORD_LENGTH ) {
		len = IDE_SECURITY_PASSWORD_LENGTH;
	}	
	// パスワードがNULLならば全0x20のパスワードとなる
	if( len > 0 ) {
		// パスワード分コピーする
	    _memcpy( pst_HDDPassword->uc_Password, pc_Password, len );  // パスワードの設定
	}
	if ( uc_UserMode == DEF_PASSWORD_USER ){
		void* pst_suffix	= (void*)&pst_HDDPassword->uc_Password[DEF_IJC_HDD_USER_PASSWORD_LEN] ;	// ユーザパスワードSUFFIXの格納位置.
		// 06/08/01 HDDロックパスワードが不一致になる問題の修正(65278) 河村
	    _memcpy( pst_suffix, (void*)DEF_USER_PASSWORD_SUFFIX, DEF_USER_PASSWORD_SUFFIX_LEN );  // パスワードの設定
	}
}

#ifdef TORNADO

/*================================================================================================
［機能］	コマンドラインプログラム実行・結果取得.
--------------------------------------------------------------------------------------------------
［引数］	char*							：コマンドライン実行文字列.
			char*							：実行結果格納バッファ.
			size_t							：格納バッファサイズ.
［戻値］	0：OK　それ以外：NG.
［詳細］	コマンドラインプログラムを実行し結果をReadする.
［ｳｫｰｸｽﾙｰ］	.
［履歴］	.
			13/02/14 Takatsuji	関数Header追加.　
            13/02/14 Takatsuji	FA14のアサート文字列のリファイン.
            15/04/13 Furuichi	CQ.512079 rebootランニングでHDD関連のFA14が発生する問題の対応.
================================================================================================*/
int32_t CP_HDDTrans::execprog_in(
    char* prog,
    char* buf,
    size_t siz
)
{
    FILE* in_pipe;
    int32_t ret;

    if( (in_pipe = popen( prog, "r" )) == NULL ){
        THRL_ThAssert( False, "[SATA2] CP_HDDTrans::setParameterPassword() execprog_in popen %d prog=%s errno=%d\n", SYSF_errnoGet() , (uintptr_t)prog, (uintptr_t)errno, 0, CFA14_DetailCode0x01D8 );
        return -1;
    }

    memset( buf, 0, siz );

    fread( buf, sizeof(char), siz, in_pipe );
    // エラーチェック処理を追加.
    if( ferror( in_pipe ) != 0 ){
        perror( "CP_HDDTrans::execprog_in fread" );
    }

    ret = pclose( in_pipe );
    // 2015/04/13 furuichi CQ.512079 pcloseがエラー返却した時errno==ECHILDならばエラーとしない.
	if( ( ret < 0 ) && ( errno == ECHILD ) ){
		SYSF_logMsg("CP_HDDTrans::execprog_in pclose err %"PRId32" %d\n", ret, errno);
		ret = 0;
	}
    return ret;
}

/*================================================================================================
［機能］	コマンドラインプログラム実行・メニュー処理.
--------------------------------------------------------------------------------------------------
［引数］	char*							：コマンドライン実行文字列.
			char*							：メニュー実行用文字列
			size_t							：メニュー実行文字列バッファサイズ.
［戻値］	0：OK　それ以外：NG.
［詳細］	コマンドラインプログラムを実行し、そのメニューを実行する.
［ｳｫｰｸｽﾙｰ］	.
［履歴］	.
			13/02/14 Takatsuji	関数Header追加.　
            13/02/14 Takatsuji	FA14のアサート文字列のリファイン.
================================================================================================*/
int32_t CP_HDDTrans::execprog_out(
    char* prog,
    char* buf,
    size_t siz
)
{
    FILE* out_pipe;
    int32_t ret;

    if( (out_pipe = popen( prog, "w" )) == NULL ){
        THRL_ThAssert( False, "[SATA2] CP_HDDTrans::setParameterPassword() execprog_out popen %d prog=%s errno=%d\n", SYSF_errnoGet() , (uintptr_t)prog, (uintptr_t)errno, 0, CFA14_DetailCode0x01D9 );
        return -1;
    }

    fwrite( buf, sizeof(char), siz, out_pipe );
    // エラーチェック処理を追加.
    if( ferror( out_pipe ) != 0 ){
        perror( "CP_HDDTrans::execprog_out fwrite" );
    }

    ret = pclose( out_pipe );

    return ret;
}

/*================================================================================================
［機能］	コマンドラインプログラム実行.
--------------------------------------------------------------------------------------------------
［引数］	char*							：コマンドライン実行文字列.
［戻値］	0：OK　それ以外：NG.
［詳細］	コマンドラインプログラムを実行し、そのメニューを実行する.
［ｳｫｰｸｽﾙｰ］	.
［履歴］	.
			13/02/14 Takatsuji	関数Header追加.　
            13/02/14 Takatsuji	FA14のアサート文字列のリファイン.
            15/01/22 Hatono		コマンド実行時のエラー出力を捨てない様に修正.
================================================================================================*/
int32_t CP_HDDTrans::execprog(
    char* prog
)
{
    // fread停止問題対応 popenをやめてsystemで実行するように処理を変更.
    int32_t ret;
    char cmd[512];

    sprintf( cmd, "%s 1> /dev/null", prog );

#if !defined(DEF_MinervaPreEmu) && !defined(DEF_ZeusRevCus)
// Str 2010/09/13 ft_kondo 8536環境Sim対応. 
#ifdef TORNADO			// 実機で有効. 
    ret = system( cmd );
#else 
    ret = 0;
#endif // #ifndef TORNADO. 
// End 2010/09/13 ft_kondo 8536環境Sim対応. 
#else
    ret = system( cmd );
#endif

    if( ret == -1 ){
        THRL_ThAssert( False, "[SATA2] CP_HDDTrans::setParameterPassword() execprog system %d prog=%s\n", SYSF_errnoGet() , (uintptr_t)prog, 0, 0, CFA14_DetailCode0x01DA );
    }

    return WEXITSTATUS( ret );
}

/*================================================================================================
［機能］	getAllSectorNum.
--------------------------------------------------------------------------------------------------
［引数］	char*							.
［戻値］	.
［詳細］	.
［ｳｫｰｸｽﾙｰ］	.
［履歴］	.
			13/02/14 Takatsuji	関数Header追加.　
            13/02/14 Takatsuji	FA14のアサート文字列のリファイン.
================================================================================================*/
uint64_t CP_HDDTrans::getAllSectorNum(
    char *pathname
)
{
    int32_t fd;
    int32_t i;
    unsigned short* id;
    static uint64_t allsec = 0;
    unsigned char args[512+4] = { WIN_IDENTIFY, 0, 0, 1, };

    if( allsec != 0 ){
        return allsec;
    }
    if( (fd = SYSF_devopen( pathname, O_RDWR, 0666 )) == -1 ){
        THRL_ThAssert( False, "[SATA2] CP_HDDTrans::setParameterPassword() getAllSectorNum SYSF_devopen pathname=%s errno=%d\n", (uintptr_t)pathname, (uintptr_t)SYSF_errnoGet(), 0, 0, CFA14_DetailCode0x01DB );
        return 0;
    }
    if( SYSF_ioctl( fd, HDIO_DRIVE_CMD, (intptr_t)&args) == -1 ){
        THRL_ThAssert( False, "[SATA2] CP_HDDTrans::setParameterPassword() getAllSectorNum SYSF_ioctl iocode=%d errno=%d\n", (uintptr_t)HDIO_DRIVE_CMD, (uintptr_t)SYSF_errnoGet(), 0, 0, CFA14_DetailCode0x01DC );
        SYSF_devclose( fd );
        return 0;
    }
    SYSF_devclose( fd );

    id = (unsigned short*)&args[4];
    for( i = 0; i < 0x100; i++ ){
        __le16_to_cpus( &id[i] );
    }
    if( id[CAPAB_0] & LBA_SUP ){
        if( ((id[CMDS_SUPP_1] & VALID) == VALID_VAL) && (id[CMDS_SUPP_1] & SUPPORT_48_BIT) ){
				allsec = (uint64_t)id[LBA_64_MSB] << 48 | (uint64_t)id[LBA_48_MSB] << 32 | 
                        (uint64_t)id[LBA_MID] << 16 | id[LBA_LSB];
        }
    }
    return allsec;
}

#ifdef DEF_SupportSSD

//  2017/10/10 Yoneda   S800 ストレージ構成変更対応.
// 2018/09/21 Sky井柳 【DenebMLK】制御構成変更No78「データ移行」対応 データ移行用使用できるようにインターフェース変更.
int64_t CP_HDDTrans::getExtSectorNum(			// DEF_SupportSSD.
	char *pathname,
    part_layout* pst_PartInfo
)
{
    int32_t no;
    int64_t secnum = 0;
	part_layout*	device_info = pst_PartInfo;
	if( device_info == NULL ){	// パーティション情報がNULLの時は、パス名からパーティション情報を取得.
		if(strcmp(pathname,theHDDTrans.pc_strDeviceSSD) == 0){
			device_info = &part_info[0];
		} else {
			device_info = &ssd_part_info[0];
		}
	}
    for( no = 0; device_info[no].type != PART_TYPE_END; no++ ){
        if( device_info[no].type == PART_TYPE_EXTENDED ){
            secnum = device_info[no].nsec;
            break;
        }
    }
    return secnum;
}

//  2017/10/10 Yoneda   S800 ストレージ構成変更対応.
// 2018/09/21 Sky井柳 【DenebMLK】制御構成変更No78「データ移行」対応 データ移行用使用できるようにインターフェース変更.
int64_t CP_HDDTrans::getFixedSectorNum(				// DEF_SupportSSD.
	char* hdddevname,
    part_layout* pst_PartInfo
)
{
    int32_t no = 0;
    int64_t secnum = 0;
	part_layout*	device_info = pst_PartInfo;
	if( device_info == NULL ){	// パーティション情報がNULLの時は、パス名からパーティション情報を取得.
		if(strcmp(hdddevname,theHDDTrans.pc_strDeviceSSD) == 0){
			device_info = &part_info[0];
		} else {
			device_info = &ssd_part_info[0];
		}
	}
    for( no = 0; device_info[no].type != PART_TYPE_END; no++ ){
        if( (device_info[no].type == PART_TYPE_PRIMARY && device_info[no].nsec > 0) ||
            (device_info[no].type == PART_TYPE_EXTENDED && device_info[no].nsec > 0) ){
            secnum += device_info[no].nsec;
        }
    }
    return secnum;
}

//  2017/10/10 Yoneda   S800 ストレージ構成変更対応.
// 2018/09/21 Sky井柳 【DenebMLK】制御構成変更No78「データ移行」対応 データ移行用使用できるようにインターフェース変更.
int64_t CP_HDDTrans::getFixedLogicalSectorNum(			// DEF_SupportSSD.
    char* hdddevname,
    part_layout* pst_PartInfo
)
{
    int32_t no = 0;
    int64_t secnum = 0;
	part_layout*	device_info = pst_PartInfo;
	if( device_info == NULL ){	// パーティション情報がNULLの時は、パス名からパーティション情報を取得.
		if(strcmp(hdddevname,theHDDTrans.pc_strDeviceSSD) == 0){
			device_info = &part_info[0];
		} else {
			device_info = &ssd_part_info[0];
		}
	}
    for( no = 0; device_info[no].type != PART_TYPE_END; no++ ){
        if( device_info[no].type == PART_TYPE_LOGICAL && device_info[no].nsec > 0 ){
            secnum += device_info[no].nsec;
        }
    }
    return secnum;
}

//  2017/10/10 Yoneda   S800 ストレージ構成変更対応.
// 2018/09/21 Sky井柳 【DenebMLK】制御構成変更No78「データ移行」対応 データ移行用使用できるようにインターフェース変更.
int32_t CP_HDDTrans::calcAndFixedPartInfo(			// DEF_SupportSSD.
    char* hdddevname,
    part_layout* pst_PartInfo,
    DLONG sdl_DevSize
)
{
    int32_t no = 0;
    int64_t start_curr = 0;
    int64_t start_ext = 0;
    DLONG device_size = sdl_DevSize;

	part_layout*	device_info = pst_PartInfo;
	if( device_info == NULL ){	// パーティション情報がNULLの時は、パス名からパーティション情報を取得.
		if(strcmp(hdddevname,theHDDTrans.pc_strDeviceSSD) == 0){
			device_info = &part_info[0];
			device_size = (DLONG)(getPhysicalHDDSize() / (DLONG)DEF_SECTOR_SIZE);
		} else {
			device_info = &ssd_part_info[0];
			device_size = (DLONG)(getPhysicalSubDeviceSize() / (DLONG)DEF_SECTOR_SIZE);
		}
	}

    for( no = 0; device_info[no].type != PART_TYPE_END; no++ ){
        if( device_info[no].nsec == -1 ){
            switch( device_info[no].type ){
              case PART_TYPE_PRIMARY:
                  //device_info[no].nsec = getAllSectorNum( hdddevname ) - getFixedSectorNum(port);
                  device_info[no].nsec = (int64_t)device_size - getFixedSectorNum(hdddevname, pst_PartInfo);
                  if( device_info[no].nsec < 0 ){
                      return -1;
                  }
                  break;
              case PART_TYPE_LOGICAL:
                  device_info[no].nsec = getExtSectorNum(hdddevname, pst_PartInfo) - getFixedLogicalSectorNum(hdddevname, pst_PartInfo);
                  break;
              default:
              case PART_TYPE_EXTENDED:
                  SYSF_logMsg( "[%s] device_info[%"PRId32"]->sec -> %"PRId64"\n", __FUNCTION__, no, device_info[no].nsec );
                  break;
            }            
        }
    }

    for( no = 0; device_info[no].type != PART_TYPE_END; no++ ){
        if( !device_info[no].nsec ){
            continue;
        }
        switch( device_info[no].type ){
          case PART_TYPE_EXTENDED:
              start_ext = start_curr;
          case PART_TYPE_PRIMARY:
              device_info[no].end = start_curr + device_info[no].nsec - 1;
              start_curr = device_info[no].end + 1;
              break;
          case PART_TYPE_LOGICAL:
              device_info[no].end = start_ext + device_info[no].nsec - 1;
              start_ext = device_info[no].end + 1;
              break;
        }
    }

#if 0
    for( no = 0; part_info[no].type != PART_TYPE_END; no++ ){
        SYSF_logMsg( "[%s] no.%d %d end:%lld nsec:%lld\n", __FUNCTION__, no, part_info[no].type, part_info[no].end, part_info[no].nsec );
    }
#endif

    return 0;
}

/*================================================================================================
［機能］	checkPartitionLayout.
--------------------------------------------------------------------------------------------------
［引数］	char*							：.
［戻値］	.
［詳細］	.
［ｳｫｰｸｽﾙｰ］	.
［履歴］	.
			13/02/14 Takatsuji	関数Header追加.　
            13/02/14 Takatsuji	FA14のアサート文字列のリファイン.
            17/10/10 Yoneda     S800 ストレージ構成変更対応.
            18/09/21 Sky井柳 【DenebMLK】制御構成変更No78「データ移行」対応 データ移行用使用できるようにインターフェース変更.
================================================================================================*/
int32_t CP_HDDTrans::checkPartitionLayout(			// DEF_SupportSSD.
    char* hdddevname,
    part_layout* pst_PartInfo,
    Bool bl_AssertEnable
)
{
    int32_t ret;
    char cmd[256];
    char* ptr;
    char dev[16];
    int64_t endsec;
    int32_t result = 0;
    int32_t no = 0;
    char val[3];
    int32_t pno;
    int32_t pno_pre = -1;
	
	part_layout*	device_info = pst_PartInfo;
	if( device_info == NULL ){	// パーティション情報がNULLの時は、パス名からパーティション情報を取得.
		if(strcmp(hdddevname,theHDDTrans.pc_strDeviceSSD) == 0){
			device_info = &part_info[0];
		} else {
			device_info = &ssd_part_info[0];
		}
	}
	
    snprintf( cmd, sizeof(cmd), FMT_PART_FDISK1, hdddevname );
    memset( sbuf, 0, sizeof(sbuf) );
    ret = execprog_in( cmd, sbuf, sizeof(sbuf)-1 );
    if( ret ){
        printf( "execprog_in ERR: cmd:%s\n", cmd );
        if( bl_AssertEnable ){
        	THRL_ThAssert( False, "[SATA2] CP_HDDTrans::checkPartitionLayout() checkPartitionLayout() execprog_in failed!! cmd %s\n" , (uintptr_t)cmd, 0, 0, 0, CFA14_DetailCode0x01DD );
        }else{
			THRL_ThLogMsgErr("[%s] execprog_in failed!! cmd [%s]\n", __FUNCTION__, cmd );
		}
        return -1;
    }
    ptr = sbuf;

    while( (ptr = strstr( ptr, hdddevname )) ){
        val[0] = (char)ptr[strlen(hdddevname)];
        if((char)ptr[strlen(hdddevname)+1] == ' ') {
            val[1] = '\0';
        }
        else {
            val[1] = (char)ptr[strlen(hdddevname)+1];
            val[2] = '\0';
        }
        pno = atoi( val );
        if( pno_pre != -1 && pno != pno_pre + 1 ){
            no++;
        }
        dev[0] = 0;
        endsec = 0;
        sscanf( ptr, "%s %lld", dev, &endsec );

        if( endsec != device_info[no].end ){
            result = -1;
            break;
        }

#if 0
        SYSF_logMsg( "[%s] %s %lld device_info %lld\n", __FUNCTION__, dev, endsec, device_info[no].end );
#endif

        no++;
        ptr++;
        pno_pre = pno;
    }

    if( device_info[no].type != PART_TYPE_END ){
        SYSF_logMsg( "Partition layout NG.\n" );
        result = -1;
    }
    return result;
}

// 2014/10/08 yamada	CQ503146 PSWC言語領域不足対応.
// 2014/11/06 simogaki	CQ506310対応.
// 2017.10.26 sky パーティション一本化対応.
// 2022.08.24 Sky OP_BTS-38505 mke2fsのタイムアウトとリトライ対応.
#define THRD_FORMAT_RETRY		3		// パーティション作成のリトライ回数.
#define THRD_FORMAT_TIMEOUT		900		// パーティション作成のタイムアウト時間（sec）.
#define THRD_FORMAT_ADD_TIME	300		// パーティション作成のタイムアウト追加時間（sec）.

int32_t CP_HDDTrans::formatFS(			// DEF_SupportSSD.
    char* devpath
)
{
    int32_t ret;
    char cmd[256] = {0};

    snprintf( cmd, sizeof(cmd), FMT_PART_MKE2FS, devpath );

    if ((strstr(devpath, THRD_DEVFILE_SSD) != NULL) && (theHDDTrans.e_StartUpDeviceType == THRE_StartupDeviceType_SD)){
        // eMMCの場合、PART_FSTYPE_FSのフォーマットはext2で行う.
        part_layout* s_DeviceInfo = THRG_GetPartitionType();

        for (uchar uc_No = 0; s_DeviceInfo[uc_No].type != PART_TYPE_END; uc_No++) {
            if ((strcmp(devpath, s_DeviceInfo[uc_No].devpath[0]) == 0) &&
#if defined (DEF_64BIT_PF)
            	((s_DeviceInfo[uc_No].fstype == PART_FSTYPE_FS) ||
            	 (s_DeviceInfo[uc_No].fstype == PART_FSTYPE_FS_SSD) ) ) {
#else /* defined (DEF_64BIT_PF)  */
                (s_DeviceInfo[uc_No].fstype == PART_FSTYPE_FS)) {
#endif /* defined (DEF_64BIT_PF) */
                snprintf(cmd, sizeof(cmd), FMT_PART_MKE2FSEXT2, devpath);
                break;
            }
        }
    }
    else if(theHDDTrans.e_StartUpDeviceType == THRE_StartupDeviceType_SSD)
    {
        // SSD起動の場合、元々eMMCのパーティションのPART_FSTYPE_FSのフォーマットはext2で行う.
        // PART_FSTYPE_FS_SSDが該当パーティション.
        for (uchar uc_No = 0; part_info[uc_No].type != PART_TYPE_END; uc_No++) {
           if ((strcmp(devpath, part_info[uc_No].devpath[0]) == 0) &&
               (part_info[uc_No].fstype == PART_FSTYPE_FS_SSD)) {
                snprintf(cmd, sizeof(cmd), FMT_PART_MKE2FSEXT2, devpath);
                break;
            }
        }
    }

#ifndef SIMULATOR
	// OP_BTS-38505 mke2fsのタイムアウトとリトライ対応(実機のみ).
	for( int i=0; i<THRD_FORMAT_RETRY; i++)
	{
		int32_t sys_ret = 0;
		uint32_t u32_timeout = THRD_FORMAT_TIMEOUT + THRD_FORMAT_ADD_TIME * i; 	// タイムアウト初回15分、以降5分ずつ増加.
		char internal_cmd[512] = {};

		SYSF_logMsg("[%s] syscall_timeout -> [%s][%dsec] retry[%d]\n", __FUNCTION__, cmd, u32_timeout, i);

		snprintf(internal_cmd, sizeof(internal_cmd), "syscall_timeout %s %d", cmd, u32_timeout);

		sys_ret = system(internal_cmd);
		if( WIFEXITED(sys_ret) )
		{
			ret = WEXITSTATUS( sys_ret );
		}
		else
		{
			// エラーを設定.
			ret = -1;
			SYSF_logMsg("system err\n");
		}

		// 正常終了.
		if( ret == 0 )
		{
			break;
		}
		else
		{
			SYSF_logMsg("[%s] ERR: clear partition info & retry [ret:%d]\n", __FUNCTION__, ret);

			// 同期しないとmke2fsのリトライ時、使用中になり失敗する.
			THRF_DoSync();

			// パーティション情報(デバイス先頭63セクタ)をクリア.
			char clear_partition[128] = {};
			sprintf( clear_partition, "dd if=/dev/zero of=%s bs=%d count=%d conv=notrunc", devpath, DEF_SECTOR_SIZE, DEF_PARTITION_INFO );
			system(clear_partition);

			// formatFSの戻り値は-1にしておく.
			ret = -1;
		}
	}

    sync();

    return ret;
#else
	SYSF_logMsg("[%s] execute->[%s]\n", __FUNCTION__, cmd );
    ret = execprog( cmd );
    if( ret ){
        printf( "execprog ERR: cmd:%s\n", cmd );
        return -1;
    }

    sync();

    return 0;
#endif
}

/*================================================================================================
［機能］	doFdisk.
--------------------------------------------------------------------------------------------------
［引数］	char*							：.
［戻値］	.
［詳細］	.
［ｳｫｰｸｽﾙｰ］	.
［履歴］	.
			13/02/14 Takatsuji	関数Header追加.　
            13/02/14 Takatsuji	FA14のアサート文字列のリファイン.
            17/03/15 Matsuoka	cmdエラー対策.
			17/06/09 matsui		【AR1023495】【IT5_4.1】【新詳細コード対応】0062647300000000.
            17/10/10 Yoneda     S800 ストレージ構成変更対応.
================================================================================================*/
int32_t CP_HDDTrans::doFdisk(			// DEF_SupportSSD.
    char* devpath
)
{
    int32_t no;
    int32_t pno = 1;
	//HDDセクタ変更 2010/12/16
    int32_t first = 0;
    int32_t logcalFirst = 0;
    int64_t extendStart = 0;
    int64_t startsec = 0;
    //↑  HDDセクタ変更
    int32_t ret;
    char cmd[256];
    char val[32];
    char tmpsbuf[512];		// 2017/03/15 Matsuoka コマンド文字列編集中に割り込まれる可能性も想定してAuto変数宣言する.
	part_layout*	device_info;
	int32_t index = 0;
	int32_t deviceType = 0;							//HDDセクタ変更 2010/12/16
	if(strcmp(devpath,theHDDTrans.pc_strDeviceSSD) == 0){
		device_info = &part_info[0];
		deviceType = THRE_StorageDeviceTypeHDD;	//HDDセクタ変更 2010/12/16
	} else {
		device_info = &ssd_part_info[0];
		deviceType = THRE_StorageDeviceTypeSSD;	//HDDセクタ変更 2010/12/16
	}
    memset( tmpsbuf, 0, sizeof(tmpsbuf) );

    strcpy( tmpsbuf, "d\n" );
    strcat( tmpsbuf, "1\n" );
    strcat( tmpsbuf, "d\n" );
    strcat( tmpsbuf, "2\n" );
    strcat( tmpsbuf, "d\n" );
    strcat( tmpsbuf, "3\n" );
    strcat( tmpsbuf, "d\n" );
    strcat( tmpsbuf, "4\n" );
    for( no = 0; device_info[no].type != PART_TYPE_END; no++ ){
        switch( device_info[no].type ){
          case PART_TYPE_PRIMARY:
              if( device_info[no].end ){
                  strcat( tmpsbuf, "n\n" );
                  strcat( tmpsbuf, "p\n" );
                  if(index < 3){
	                  snprintf( val, sizeof(val), "%"PRId32"\n", pno++ );
	                  strcat( tmpsbuf, val );		//HDDセクタ変更 2010/12/16
	                  index++;
	              }
                  // takayama MinervaZeus HDD4kbyteセクタ対応.
                  if (deviceType == THRE_StorageDeviceTypeHDD) {
	                  if (first == 0) {
		                  startsec = DEF_PARTITION_INFO + 1;
		                  first = 1;                  
	                  } else {
		                  startsec = (device_info[no - 1].end + 1);
	                  }
	                  snprintf(  val, sizeof(val), "%"PRId64"\n", startsec );
		              strcat( tmpsbuf, val );
	              } else {
					  strcat( tmpsbuf, "\n" );
				  }	//↑ takayama MinervaZeus HDD4kbyteセクタ対応.
                  snprintf(  val, sizeof(val), "%"PRId64"\n", device_info[no].end );
                  strcat( tmpsbuf, val );
                  if( device_info[no].fstype == PART_FSTYPE_RAW_HIB ){
					int32_t no_;
					strcat( tmpsbuf, "t\n" );
					no_ = no;
					no_++;
					if(no_ != 1){
						snprintf(  val, sizeof(val), "%"PRIx32"\n", no_ );
						strcat( tmpsbuf, val );
					}
					snprintf(  val, sizeof(val), "%x\n", 0xa0 );
					strcat( tmpsbuf, val );
				  }
				  if( device_info[no].fstype == PART_FSTYPE_FAT16 ){
					int32_t no_;
					strcat( tmpsbuf, "t\n" );
					no_ = no;
					no_++;
					if(no_ != 1){
						snprintf(  val, sizeof(val), "%"PRId32"\n", no_ );
						strcat( tmpsbuf, val );
					}
					snprintf(  val, sizeof(val), "%x\n", 0x6 );
					strcat( tmpsbuf, val );
				  }
              }
              break;
          case PART_TYPE_EXTENDED:
              strcat( tmpsbuf, "n\n" );
              strcat( tmpsbuf, "e\n" );
              if(index < 3){
                  snprintf( val, sizeof(val), "%"PRId32"\n", pno++ );
                  index++;
                  strcat( tmpsbuf, val );		//HDDセクタ変更 2010/12/16
	          }
              // takayama MinervaZeus HDD4kbyteセクタ対応.
              if (deviceType == THRE_StorageDeviceTypeHDD) {
	 	          if(first == 0) {
					  startsec = DEF_SSD_PARTITION_INFO;
		              first = 1;
	              } else {
		              startsec = (device_info[no - 1].end + 1 );
	              }
	              snprintf(  val, sizeof(val), "%"PRId64"\n", startsec );
	          	  strcat( tmpsbuf, val );
	          } else {
			  	  strcat( tmpsbuf, "\n" );
			  }	//↑ takayama MinervaZeus HDD4kbyteセクタ対応.
			  snprintf( val, sizeof(val), "%"PRId64"\n", device_info[no].end );
              strcat( tmpsbuf, val );
              extendStart = device_info[no - 1].end + 1;	//HDDセクタ変更 2010/12/16
              break;
          case PART_TYPE_LOGICAL:
              strcat( tmpsbuf, "n\n" );
              strcat( tmpsbuf, "l\n" );
              // takayama MinervaZeus HDD4kbyteセクタ対応.
 	          if (deviceType == THRE_StorageDeviceTypeHDD) {
	 	          if(first == 0) {
					 startsec = DEF_PARTITION_INFO + 1;
		             first = 1;
	              } else if(logcalFirst == 0) {
					 startsec = extendStart + DEF_SSD_PARTITION_INFO;
		             logcalFirst = 1;
	              } else {
					  startsec = (device_info[no - 1].end + DEF_SSD_PARTITION_INFO + 1);
	              }
	              snprintf(  val, sizeof(val), "%"PRId64"\n",startsec  );
		          strcat( tmpsbuf, val );
	          } else {
			      strcat( tmpsbuf, "\n" );
			  }	//↑ takayama MinervaZeus HDD4kbyteセクタ対応.
			  snprintf( val, sizeof(val), "%"PRId64"\n", device_info[no].end );
              strcat( tmpsbuf, val );
              if( device_info[no].fstype == PART_FSTYPE_RAW_HIB ){
					int32_t no_;
					strcat( tmpsbuf, "t\n" );
					no_ = no;
					no_++;
					if(no_ != 1){
						snprintf(  val, sizeof(val), "%"PRId32"\n", no_ );
						strcat( tmpsbuf, val );
					}
					snprintf(  val, sizeof(val), "%x\n", 0xa0 );
					strcat( tmpsbuf, val );
			  }
              if( device_info[no].fstype == PART_FSTYPE_FAT16 ){
				int32_t no_;
				strcat( tmpsbuf, "t\n" );
				no_ = no;
				no_++;
				if(no_ != 1){
					snprintf(  val, sizeof(val), "%"PRId32"\n", no_ );
					strcat( tmpsbuf, val );
				}
				snprintf(  val, sizeof(val), "%x\n", 0x6 );
				strcat( tmpsbuf, val );
			  }
              break;
        }
    }
    strcat( tmpsbuf, "w\n" );

    snprintf( cmd, sizeof(cmd), FMT_PART_FDISK2, devpath );
    ret = execprog_out( cmd, tmpsbuf, strlen(tmpsbuf) );
    if( ret ){
        printf( "execprog_out ERR: cmd:%s\n", cmd );
		// 2017/03/15 fdiskコマンド文字列の有意部分を抜き出してログ保存する.
		uint64_t          ull_NewDetailData = THRD_NewDetail_ModuleId_Thread;	// 1,2桁：スレッド.
		ull_NewDetailData |= (uint64_t)( TYPD_TroubleCode_HDD_TRANS_ERROR ) << THRD_NewDetail_TroubleCode_Shift;	// 3-6桁：トラブルコード.
		ull_NewDetailData |= (uint64_t)( 2 ) << 32;							// 7-8桁：HDD_TRANS_ERRORの通し番号.
		// 残り4bytes分はパーティション情報によって新詳細コードをセット.
		if( deviceType == THRE_StorageDeviceTypeHDD ) {			//devpathがHDDかを判定し、判定によって下位4bytesのコードを変える.
			ull_NewDetailData |= THRD_CMDErrorCode_sdb;
		} else {
			ull_NewDetailData |= THRD_CMDErrorCode_sda;
		}
		THRL_ThAssertLog2( 0, ull_NewDetailData );
		DebugForever( HDD_TRANS_ERROR );
        return -1;
    }

    /*
     * fdisk実行後、新しいパーティションレイアウトが認識されてHDDのデバイスファイルが
     * 生成されるまでタイムラグがあるためディレイする。
     */
    sleep( 2 );

    return 0;
}


// 10/06/3  takayama GaiaPF HDDレス時にata03をRamDiskで生成する.
int32_t CP_HDDTrans::createRamDisk(			// DEF_SupportSSD.
    char* devpath,
    int32_t size
)
{
	int32_t ret;
	char cmd[256];
	char val[32];
	snprintf( val, sizeof(val), "%"PRId32"\n", size );
	snprintf( cmd, sizeof(cmd), FMT_PART_RAMDISK, devpath, val);
    ret = execprog( cmd );
    if( ret ){
        printf( "execprog FMT_PART_RAMDISK ERR: cmd:%s\n", cmd );
        return -1;
    }
	return 0 ; 	//	2010.07.15	飯塚
}

// 2014/10/08 yamada	CQ503146 PSWC言語領域不足対応.
// 2014/11/06 simogaki	CQ506310対応.
// 2017.10.26 sky パーティション一本化対応.
int32_t CP_HDDTrans::doFsck(				// DEF_SupportSSD.
    char* devpath,
    int32_t i_fscklevel,
    int32_t fstype
)
{
    int32_t ret;
    char cmd[256];
    
    if( i_fscklevel ){
        snprintf( cmd, sizeof(cmd), FMT_PART_TUNE2FS1, devpath );
    }else{
        snprintf( cmd, sizeof(cmd), FMT_PART_TUNE2FS0, devpath );
    }
    ret = execprog( cmd );
    if( ret ){
        printf( "execprog ERR: cmd:%s\n", cmd );
        return -1;
    }

	if ( strstr(devpath, THRD_DEVFILE_SSD) != NULL ){

#if defined (DEF_64BIT_PF)
        if( (fstype == PART_FSTYPE_FS) || (fstype == PART_FSTYPE_FS_SSD) ||(fstype == PART_FSTYPE_FS_NF2) ){
#else
        if( (fstype == PART_FSTYPE_FS) || (fstype == PART_FSTYPE_FS_NF2) ){
#endif
            snprintf( cmd, sizeof(cmd), FMT_PART_FSCK1ST, "ext2", devpath );
        }else{
            snprintf( cmd, sizeof(cmd), FMT_PART_FSCK1ST, "ext3", devpath );
        }
    } else {
#if defined (DEF_64BIT_PF)
    	if( ( fstype == PART_FSTYPE_FS_NF2 ) || (fstype == PART_FSTYPE_FS_SSD) ){
#else
        if( fstype == PART_FSTYPE_FS_NF2 ){
#endif
            //printf( "[%s] [%s] ext2\n", __PRETTY_FUNCTION__, devpath );
            snprintf( cmd, sizeof(cmd), FMT_PART_FSCK1ST, "ext2", devpath );
        }else{
            //printf( "[%s] [%s] ext3\n", __PRETTY_FUNCTION__, devpath );
            snprintf( cmd, sizeof(cmd), FMT_PART_FSCK1ST, "ext3", devpath );
        }
    }
    ret = execprog( cmd );

    if( ret == 0 ){
        // エラーなし
        SYSF_logMsg( "fsck 1st %s %"PRId32"\t[ OK ]\n", devpath, i_fscklevel );
        return 0;
    }else if( ret == 1 ){
        // エラーがあったが修復済み
        SYSF_logMsg( "fsck 1st %s %"PRId32"\t[ Repair ]\n", devpath, i_fscklevel );
        return 0;
    }else if( ret > 1 ){
        // エラーあり
        SYSF_logMsg( "fsck 1st %s %"PRId32"\t[ NG ]\n", devpath, i_fscklevel );
    }

    if (strstr(devpath, THRD_DEVFILE_SSD) != NULL) {
#if defined (DEF_64BIT_PF)
        if( (fstype == PART_FSTYPE_FS) || (fstype == PART_FSTYPE_FS_SSD) ||(fstype == PART_FSTYPE_FS_NF2) ){
#else
    	if( (fstype == PART_FSTYPE_FS) || (fstype == PART_FSTYPE_FS_NF2) ){
#endif
            snprintf( cmd, sizeof(cmd), FMT_PART_FSCK2ND, "ext2", devpath );
        }else{
            snprintf( cmd, sizeof(cmd), FMT_PART_FSCK2ND, "ext3", devpath );
        }
    } else {
#if defined (DEF_64BIT_PF) 
        if( ( fstype == PART_FSTYPE_FS_NF2 ) || (fstype == PART_FSTYPE_FS_SSD) ){
#else
        if( fstype == PART_FSTYPE_FS_NF2 ){
#endif
            //printf( "[%s] [%s] ext2\n", __PRETTY_FUNCTION__, devpath );
            snprintf( cmd, sizeof(cmd), FMT_PART_FSCK2ND, "ext2", devpath );
        }else{
            //printf( "[%s] [%s] ext3\n", __PRETTY_FUNCTION__, devpath );
            snprintf( cmd, sizeof(cmd), FMT_PART_FSCK2ND, "ext3", devpath );
        }
    }
    ret = execprog( cmd );

    if( ret == 0 ){
        SYSF_logMsg( "fsck 2nd %s %"PRId32"\t[ OK ]\n", devpath, i_fscklevel );
        return 0;
    }else if( ret == 1 ){
        SYSF_logMsg( "fsck 2nd %s %"PRId32"\t[ Repair ]\n", devpath, i_fscklevel );
        return 0;
    }else{
        SYSF_logMsg( "fsck 2nd %s %"PRId32"\t[ Trouble(%"PRId32") ]\n", devpath, i_fscklevel, ret );
    }

    return ret;
}

/*================================================================================================
［機能］	makeDeviceFiles.
--------------------------------------------------------------------------------------------------
［引数］	char*							：.
［戻値］	.
［詳細］	.
［ｳｫｰｸｽﾙｰ］	.
［履歴］	.
			13/02/14 Takatsuji	関数Header追加.　
            13/02/14 Takatsuji	FA14のアサート文字列のリファイン.
			17/10/26 Sky		パーティション一本化対応.
================================================================================================*/
int32_t CP_HDDTrans::makeDeviceFiles(				// DEF_SupportSSD.
    char* pathname
)
{
	int32_t fd;
	int32_t sc_No, sc_Ret, sc_LimitCnt;
	struct stat    st_Stat;

	part_layout*	device_info;
	
	if(strcmp(pathname,THRD_HDD_DEVICE) == 0){
		device_info = &part_info[0];
	} else {
		device_info = &ssd_part_info[0];
	}

	if( ( fd = SYSF_devopen( pathname, O_RDWR, 0666 ) ) == -1 ){
		printf( "%s - device file open failed! errno %d\n", __FUNCTION__, SYSF_errnoGet() );
		return -1;
	}
	if( SYSF_ioctl( fd, BLKRRPART, 0 ) == -1 ){	// 強制的に、SCSI ディスクのパーティションテーブルの再読み込みを行う.
        if( errno == EBUSY ){ // HDDブート対応（マウント済みの場合EBUSYとなる。HDDブートの場合OS起動時にRFSがマウント済み）.
            SYSF_devclose( fd );
            return 0;
        }
		printf( "%s - making device file was failed! errno %d\n", __FUNCTION__, SYSF_errnoGet() );
		SYSF_devclose( fd );
		return -1;
	}

	sleep( 2 );	/* wait to create device files */
	for(sc_No=0; device_info[sc_No].type != PART_TYPE_END; sc_No++) {
		if( device_info[sc_No].fstype != PART_FSTYPE_FS &&
#if defined (DEF_64BIT_PF)
            device_info[sc_No].fstype != PART_FSTYPE_FS_SSD &&
#endif
			// AR1059900対応 PART_FSTYPE_FS_NFの判定削除 AR1059900の問題と直接関係はないが、見直しにより合わせて修正.
            device_info[sc_No].fstype != PART_FSTYPE_FS_NF2 &&
            device_info[sc_No].fstype != PART_FSTYPE_FS_NF3 ) {
			continue;
		}
		
		if ((THRC_PartitionInfo::chkMountPntOption(device_info[ sc_No ].mntpnt, THRE_OPTIONS_BIT_EXCEPTION_MOUNT)) == True) {
			continue;
		}

		sc_LimitCnt=0;
		while( 1 ) {
			sc_LimitCnt++;

			sc_Ret = SYSF_stat(device_info[sc_No].devpath[0], &st_Stat);
			if(0 == sc_Ret && S_ISBLK(st_Stat.st_mode)) {//デバイスファイルとして存在するか?.
				break;
			}

			if(30 <= sc_LimitCnt) {
				printf( "%s - not found: %s\n", __FUNCTION__, device_info[sc_No].devpath[0] );
				SYSF_devclose( fd );
				return -1;
			}

			sleep( 1 );
		}
	}

	SYSF_devclose( fd );
	return 0;
}

/*================================================================================================
［機能］	暗号化ブロックデバイスの生成／破棄
--------------------------------------------------------------------------------------------------
［引数］	ss_Encrypt : 0以外：暗号化デバイスブロック生成 / 0：暗号化デバイスブロック破棄
		ss_Flg：処理対象とするパーティションの暗号化フラグ
		psc_Password：暗号化パスワード文字列へのポインタ
［戻値］	short : 0:暗号化ブロックデバイス生成に成功/処理失敗
［詳細］	bl_EncryptがTrueなら、パーティションの暗号化フラグでss_Flgビットが立っている領域の暗号化デバイスブロックを生成.
		bl_EncryptがFalseなら、パーティションの暗号化フラグでss_Flgビットが立っている領域の暗号化デバイスブロックを破棄.
 [履歴]     2014/11/05 大嶋   cPP暴露保護対応
			2015/10/23 中村   [SecV2-4]SecV1時の対応をマージ、鍵管理制御更新.
			2017/11/09	SCD2-Sky Shindo [WP.53-54] 設定値見直し対応.
================================================================================================*/
short CP_HDDTrans::setupSsdDmcrypt( short ss_Encrypt, short ss_Flg, char* psc_Password )		// DEF_SupportSSD.
{
#ifdef DEF_SupportSSD
	char cmd[256] = {0};
	int32_t ret = 0;
	const part_layout* device_info = &ssd_part_info[0];
	THRC_Crypto *pc_Crypto = NULL;

	pc_Crypto = THRC_Crypto::newInstance();
	if( NULL == pc_Crypto ) {
		THRL_ThAssert( False, "[SATA2] CP_HDDTrans::setupSsdDmcrypt(%d) THRC_Crypto::newInstance() failed.\n", __LINE__, 0, 0, 0, CFA14_DetailCode0x01DF );
		return -1;
	}
	
	if(DEF_HDD_CRYPT_ON == ss_Encrypt){
	    char c_KeyFile[32] = {0};

		/* 鍵情報を生成する. */
		uchar uc_DEK[NVDC_KeyLen] = {0};
		if( 0 != pc_Crypto->getDEK( THRE_StorageDeviceTypeSSD, uc_DEK, NVDC_KeyLen ) ){
			THRL_ThAssert( False, "[SATA2] CP_HDDTrans::setupSsdDmcrypt(%d) getDEK() failed.\n", __LINE__, 0, 0, 0, CFA14_DetailCode0x01DF );
			return -1;
		}
		
		if( 0 != pc_Crypto->mkTempKeyFile(uc_DEK, NVDC_KeyLen, c_KeyFile, sizeof(c_KeyFile) ) ){
			THRL_ThAssert( False, "[SATA2] CP_HDDTrans::setupSsdDmcrypt(%d) mkTempKeyFile() failed.\n", __LINE__, 0, 0, 0, CFA14_DetailCode0x01DF );
			return -1;
		}


		/* 暴露保護後にNVRAMバックアップへ平文データを書かないよう、 */
		/* FlashCtrlBackupのfd(/dev/kmsda)をCloseする */
		raw_CloseFlashFd();
		
		/* パーティション1には暗号化領域と非暗号化領域が含まれるため、パーティションの部分暗号化を行う */

		/* /dev/kmsda?マウント中は、/dev/kmsdaのcryptsetupに失敗する為、 */
		/* /dev/kmsda ではなく /dev/kmsda1 を部分暗号化してNVRAMバックアップを暗号化する */
		int32_t si_RetryCount = 0;
		for( si_RetryCount = 0; si_RetryCount <= THRD_MountRetryMax; si_RetryCount++ ){
			ret = access( device_info[0].devpath[0], F_OK );
			if( 0 == ret ){
				break;
			}else if( -1 == ret ){
				delayTask( 10 + (si_RetryCount * 50) );		//初回10msecで回を重ねるごとに+50msec追加.
			}
		}
		snprintf( cmd, sizeof(cmd), THRD_DmcryptSsd1, c_KeyFile, THRD_CRYPT_KEYSIZE);

		ret = execprog( cmd );
		if( ret ){
			/* 鍵情報クリア. */
			pc_Crypto->clrTempKeyFile( c_KeyFile );
			printf( "execprog ERR: cmd:%s\n", cmd );
			return -1;
		}

		int32_t no;
		for( no = 1; device_info[no].type != PART_TYPE_END; no++ ){
			if(!(device_info[ no ].crypt_Flg & ss_Flg)){
				continue;
			}

			si_RetryCount = 0;
			for( si_RetryCount = 0; si_RetryCount <= THRD_MountRetryMax; si_RetryCount++ ){
				ret = access( device_info[no].devpath[0], F_OK );
				if( ret == 0 ){
					break;
				}else if( ret == -1 ){
					delayTask( 10 + (si_RetryCount * 50) );		//初回10msecで回を重ねるごとに+50msec追加.
				}
			}

			// イニシャルベクタ固定（上書き優先モード）.
			snprintf( cmd, sizeof(cmd), THRD_Dmcrypt, device_info[no].devpath[0], c_KeyFile, THRD_CRYPT_KEYSIZE, 
				no+1, device_info[no].devpath[0] );
			ret = execprog( cmd );

			if( ret ){
				/* 鍵情報クリア. */
				pc_Crypto->clrTempKeyFile( c_KeyFile );
				printf( "execprog ERR: cmd:%s\n", cmd );
				THRL_ThAssert( False, "[SATA2] CP_HDDTrans::setupSsdDmcrypt() execprog ERR.(%s) \n" , (uintptr_t)cmd, 0, 0, 0, CFA14_DetailCode0x01E1 );
				return -1;
			}
		}
		/* 鍵情報クリア. */
		pc_Crypto->clrTempKeyFile( c_KeyFile );
	} else {
		/* 暗号化デバイスブロックをremove出来るよう、NVRAMバックアップをCloseする */
		raw_CloseFlashFd();

		int32_t no;
		for(no = 0; device_info[no].type != PART_TYPE_END; no++ ){
			if(!(device_info[ no ].crypt_Flg & ss_Flg)){
				continue;
			}
				snprintf( cmd, sizeof(cmd), THRD_DmcryptRemove, no+1);
				ret = execprog( cmd );
			if( ret ){
				printf( "execprog ERR: cmd:%s\n", cmd );
				THRL_ThAssert( False, "[SATA2] CP_HDDTrans::setupSsdDmcrypt() execprog ERR.(%s) \n" , (uintptr_t)cmd, 0, 0, 0, CFA14_DetailCode0x01E2 );
				return -1;
			}
		}
	}
	
	return 0;
#else /* DEF_SupportSSD */
	return -1;
#endif /* DEF_SupportSSD */
}

/*================================================================================================
［機能］	HDDFWのDMCRYPT設定.
--------------------------------------------------------------------------------------------------
［引数］	無し.
［戻値］	無し.
［詳細］	hdd_fw領域をdmcryptで暗号化設定を行う.
［履歴］	2010/10/18 takayama CQ333996 HDD暗号化時に省電力から復帰しない問題対応.
			2012/01/20 takayama CQ360543 FD残る問題対策 ファイルアクセスAPIをラッパー関数に置き換え.
			2012/09/05 miyamoto CQ371341 省電力時にFA14対応 PART_FSTYPE_FS以外を暗号化削除しないように修正.
			2012/10/15 miyamoto CQ373404 省電力復帰時にHDDが認識されていない場合のリトライ処理追加.
            2013/02/14 Takatsuji	FA14のアサート文字列のリファイン.
			2013/07/08 fukuhara	MZ1stMIF機VerUp対応(ALL_2nd_MR対応).
			2013/08/13 yamada	CQ382517 MZ1stMIF機VerUp対応(一時データ上書き削除対応).
			2017/06/05 Sky井柳	[IT5_4.0] AR1023986 省電力復帰時に対象デバイスが一時的に消える対応として.
								暗号化デバイス作成コマンド実行のリトライ処理追加.
            2017/10/10 Yoneda   S800 ストレージ構成変更対応.
			2014/11/05 oshima   cPP暴露保護対応.
			2015/10/23 中村     [SecV2-4]SecV1時の対応をマージ、HDD暗号化鍵のHCD-PP適合.
			2018/09/21 Sky井柳 【DenebMLK】制御構成変更No78「データ移行」対応 データ移行用使用できるようにインターフェース変更.
================================================================================================*/
int32_t CP_HDDTrans::setupDmcrypt(	// DEF_SupportSSD.
    char* pathname,
    int32_t type,
    part_layout* pst_PartInfo,
    Bool bl_AssertEnable
)
{
    int32_t no;
    char cmd[256];
    int32_t ret;
    char c_KeyFile[32];
	part_layout*	device_info = pst_PartInfo;
	THRC_Crypto *pc_Crypto = THRC_Crypto::newInstance();

	if( NULL == pc_Crypto ) {
		if( bl_AssertEnable ){
			THRL_ThAssert( False, "[SATA2] CP_HDDTrans::setupSsdDmcrypt(%d) THRC_Crypto::newInstance() failed.\n", __LINE__, 0, 0, 0, CFA14_DetailCode0x01DF );
        }else{
			THRL_ThLogMsgErr("[%s] (%d) THRC_Crypto::newInstance() failed.\n", __FUNCTION__, __LINE__ );
		}
		return -1;
	}

	if( device_info == NULL ){	// パーティション情報がNULLの時は、パス名からパーティション情報を取得.
		if(strcmp(pathname,theHDDTrans.pc_strDeviceSSD) == 0){
			device_info = &part_info[0];
		} else {
			/* 暴露保護ON時のSSDに対するcryptsetupは、RFS起動時(fstab前)に行う。 */
			/* そのため、ここではSSDに対するcryptsetupは行わない */
			return 0;
		}
	}
	if(type == DEF_HDD_CRYPT_ON) {
		/* 鍵情報を生成する. */
		uchar uc_DEK[NVDC_KeyLen] = {0};
		if( 0 != pc_Crypto->getDEK( THRE_StorageDeviceTypeSSD, uc_DEK, NVDC_KeyLen ) ){
			if( bl_AssertEnable ){
				THRL_ThAssert( False, "[SATA2] CP_HDDTrans::setupDmcrypt(%d) getDEK() failed.\n", __LINE__, 0, 0, 0, CFA14_DetailCode0x01DF );
	        }else{
				THRL_ThLogMsgErr("[%s] (%d) getDEK() failed.\n", __FUNCTION__, __LINE__ );
			}
			return -1;
		}
		if( 0 != pc_Crypto->mkTempKeyFile(uc_DEK, NVDC_KeyLen, c_KeyFile, sizeof(c_KeyFile) ) ){
			if( bl_AssertEnable ){
				THRL_ThAssert( False, "[SATA2] CP_HDDTrans::setupDmcrypt(%d) mkTempKeyFile() failed.\n", __LINE__, 0, 0, 0, CFA14_DetailCode0x01DF );
	        }else{
				THRL_ThLogMsgErr("[%s] (%d) mkTempKeyFile() failed.\n", __FUNCTION__, __LINE__ );
			}
			return -1;
		}

	    for( no = 0; device_info[no].type != PART_TYPE_END; no++ ){
#if defined (DEF_64BIT_PF) 
	        if( device_info[ no ].mntpnt[0] == 0 ||
	        	 ( ( device_info[no].fstype != PART_FSTYPE_FS )
	        	 	&& ( device_info[no].fstype != PART_FSTYPE_FS_SSD )
	        	 	&& ( device_info[no].fstype != PART_FSTYPE_FS_SSD3 ) ) || 
#else
	        if( device_info[ no ].mntpnt[0] == 0 || device_info[no].fstype != PART_FSTYPE_FS || 
#endif
	        	(THRC_PartitionInfo::chkMountPntOption(device_info[ no ].mntpnt, THRE_OPTIONS_BIT_EXCEPTION_MOUNT) == True)){
	            continue;
	        }
			int32_t si_RetryCount = 0;								// Retry用カウンタ.
			for( si_RetryCount = 0; si_RetryCount <= THRD_MountRetryMax; si_RetryCount++ ){
				ret = access( device_info[no].devpath[0], F_OK );
				if( ret == 0 ){
					break;
				}else if( ret == -1 ){
					delayTask( 10 + (si_RetryCount * 50) );		//初回10msecで回を重ねるごとに+50msec追加.
				}
			}
	        if( NVDC_SystemSetting::newInstance()->getEncryptionOrOverWrite() == True ){
	            // イニシャルベクタ可変（暗号化優先モード）.
	            snprintf( cmd, sizeof(cmd), FMT_PART_DMCRYPT1, device_info[no].devpath[0], c_KeyFile, THRD_CRYPT_KEYSIZE,
	                      ul_OsStartBlock, no+1, device_info[no].devpath[0] );
	        }else{
#if defined(DEF_MinervaZeusVerUp) && !defined(DEF_MinervaBKCus)
	            // イニシャルベクタ固定（上書き優先モード）.
	            snprintf( cmd, sizeof(cmd), FMT_PART_DMCRYPT2, device_info[no].devpath[0], c_KeyFile, THRD_CRYPT_KEYSIZE,
	                      no+1, device_info[no].devpath[0] );
#else
				// イニシャルベクタ固定(NULLオプション指定)
	            snprintf( cmd, sizeof(cmd), FMT_PART_DMCRYPT3, device_info[no].devpath[0], c_KeyFile, THRD_CRYPT_KEYSIZE,
	                      no+1, device_info[no].devpath[0] );
#endif // defined(DEF_MinervaZeusVerUp) && !defined(DEF_MinervaBKCus).
	        }
			for( si_RetryCount = 0; si_RetryCount < THRD_DmcrypCmdtRetryMax; si_RetryCount++ ){
				ret = execprog( cmd );
				if( ret == 0 ){	// コマンド成功時は処理を抜ける.
					break;
				}
				// BSPによる確認結果で、hdparmでパーティションテーブルを、カーネルに 再読み込みさせると.
				// デバイス(kmsdb4)が200msほど消えることが判明したため.
				// インターバル 200msecとして、繰り返し確認する.
				delayTask( 200 );
	            printf( "execprog ERR retry[%ld]: cmd:%s\n", si_RetryCount, cmd );
			}
	        if( ret ){
	            // 一次ファイルを削除
				pc_Crypto->clrTempKeyFile( c_KeyFile );
	            printf( "execprog ERR: cmd:%s\n", cmd );
				if( bl_AssertEnable ){
					THRL_ThAssert( False, "[SATA2] CP_HDDTrans::setupDmcrypt() execprog ERR.(%s) (bl_endSleepHDDInit=%d)\n" , (uintptr_t)cmd, (uintptr_t)bl_endSleepHDDInit, 0, 0, CFA14_DetailCode0x01E1 );
		        }else{
					THRL_ThLogMsgErr("[%s] execprog ERR.(%s) (bl_endSleepHDDInit=%d)\n", __FUNCTION__, cmd, bl_endSleepHDDInit );
				}
	            return -1;
	        }
	    }

	    // 一次ファイルを削除
		pc_Crypto->clrTempKeyFile( c_KeyFile );
	} else {
		for( no = 0; device_info[no].type != PART_TYPE_END; no++ ){
#if defined (DEF_64BIT_PF)
	        if( device_info[ no ].mntpnt[0] == 0 || ( ( device_info[no].fstype != PART_FSTYPE_FS ) && ( device_info[no].fstype != PART_FSTYPE_FS_SSD ) ) || 
#else
	        if( device_info[ no ].mntpnt[0] == 0 || device_info[no].fstype != PART_FSTYPE_FS || 
#endif
	            (THRC_PartitionInfo::chkMountPntOption(device_info[ no ].mntpnt, THRE_OPTIONS_BIT_EXCEPTION_MOUNT) == True)){
	            continue;
	        }
			snprintf( cmd, sizeof(cmd), FMT_PART_DMCRYPT_REMOVE, no+1);
			ret = execprog( cmd );
	        if( ret ){
	            printf( "execprog ERR: cmd:%s\n", cmd );
				if( bl_AssertEnable ){
					THRL_ThAssert( False, "[SATA2] CP_HDDTrans::setupDmcrypt() execprog ERR.(%s) (bl_endSleepHDDInit=%d)\n" , (uintptr_t)cmd, (uintptr_t)bl_endSleepHDDInit, 0, 0, CFA14_DetailCode0x01E2 );
		        }else{
					THRL_ThLogMsgErr("[%s] execprog ERR.(%s) (bl_endSleepHDDInit=%d)\n", __FUNCTION__, cmd, bl_endSleepHDDInit );
				}
	            return -1;
	        }
		}
	}
    return 0;
}

/*================================================================================================
［機能］	HDDFWのDMCRYPT設定.
--------------------------------------------------------------------------------------------------
［引数］	無し.
［戻値］	無し.
［詳細］	hdd_fw領域をdmcryptで暗号化設定を行う.
［履歴］	2012/08/27 miyamoto WBS7112 HDDパーティション暗号化対応.
			2012/09/26 miyamoto CQ372075 HDD暗号化キーAESライブラリ対応.
			2012/10/15 miyamoto CQ373404 省電力復帰時にHDDが認識されていない場合のリトライ処理追加.
            2013/02/14 Takatsuji	FA14のアサート文字列のリファイン.
            2017/10/10 Yoneda   S800 ストレージ構成変更対応.
			2015/10/23 中村     [SecV2-4]HDD暗号化鍵のHCD-PP適合.
			2017/11/09	SCD2-Sky Shindo [WP.53-54] 設定値見直し対応.
================================================================================================*/
int32_t CP_HDDTrans::setupDmcryptHddFw(		// DEF_SupportSSD.
    int32_t type
)
{
    int32_t no;
    char cmd[256];
    int32_t ret;
    char c_KeyFile[32];
	char c_DevPath[32];
	part_layout*	device_info = &part_info[0];

#ifdef SIMULATOR
	return 0;
#endif

	// HddFw情報の検索.
	for( no = 0; device_info[no].type != PART_TYPE_END; no++ ){
		if(strcmp(device_info[no].mntpnt,THRD_DevName_Hdd11) == 0){
			break;
		}
	}
	if(type == DEF_HDD_CRYPT_ON) {
		THRC_Crypto *pc_Crypto = NULL;

		pc_Crypto = THRC_Crypto::newInstance();
		if( NULL == pc_Crypto ) {
			THRL_ThAssert( False, "[SATA2] CP_HDDTrans::setupDmcryptHddFw(%d) THRC_Crypto::newInstance() failed.\n", __LINE__, 0, 0, 0, CFA14_DetailCode0x01DF );
			return -1;
		}
		
		/* 鍵情報を生成する. */
		uchar uc_DEK[NVDC_KeyLen] = {0};
		if( 0 != pc_Crypto->getDEK( THRE_StorageDeviceTypeHDD, uc_DEK, NVDC_KeyLen ) ){
			THRL_ThAssert( False, "[SATA2] CP_HDDTrans::setupDmcryptHddFw(%d) getDEK() failed.\n", __LINE__, 0, 0, 0, CFA14_DetailCode0x01DF );
			return -1;
		}
		
		if( 0 != pc_Crypto->mkTempKeyFile(uc_DEK, NVDC_KeyLen, c_KeyFile, sizeof(c_KeyFile) ) ){
			THRL_ThAssert( False, "[SATA2] CP_HDDTrans::setupDmcryptHddFw(%d) mkTempKeyFile() failed.\n", __LINE__, 0, 0, 0, CFA14_DetailCode0x01DF );
			return -1;
		}
		
        // イニシャルベクタ可変（暗号化優先モード）.
		sprintf( c_DevPath, "%s%"PRId32"", theHDDTrans.pc_strDeviceSSD, no+1 );
		int32_t si_RetryCount = 0;								// Retry用カウンタ.
		for( si_RetryCount = 0; si_RetryCount <= THRD_MountRetryMax; si_RetryCount++ ){
			ret = access( c_DevPath, F_OK );
			if( ret == 0 ){
				break;
			}else if( ret == -1 ){
				delayTask( 10 + (si_RetryCount * 50) );		//初回10msecで回を重ねるごとに+50msec追加.
			}
		}
        snprintf( cmd, sizeof(cmd), FMT_PART_DMCRYPT1, c_DevPath, c_KeyFile, THRD_CRYPT_KEYSIZE,
                  ul_OsStartBlock, no+1, c_DevPath );
        ret = execprog( cmd );
        if( ret ){
            // 一次ファイルを削除
			pc_Crypto->clrTempKeyFile( c_KeyFile );
            printf( "execprog ERR: cmd:%s\n", cmd );
            THRL_ThAssert( False, "[SATA2] CP_HDDTrans::setupDmcryptHddFw() execprog ERR.(%s) (bl_endSleepHDDInit=%d)\n" , (uintptr_t)cmd, (uintptr_t)bl_endSleepHDDInit, 0, 0, CFA14_DetailCode0x0A74 );
            return -1;
        }

	    // 一次ファイルを削除
		pc_Crypto->clrTempKeyFile( c_KeyFile );
	} else {
		snprintf( cmd, sizeof(cmd), FMT_PART_DMCRYPT_REMOVE, no+1);
		ret = execprog( cmd );
        if( ret ){
            printf( "execprog ERR: cmd:%s\n", cmd );
            THRL_ThAssert( False, "[SATA2] CP_HDDTrans::setupDmcryptHddFw() execprog ERR.(%s) (bl_endSleepHDDInit=%d)\n" , (uintptr_t)cmd, (uintptr_t)bl_endSleepHDDInit, 0, 0, CFA14_DetailCode0x0A75 );
            return -1;
        }
	}
    return 0;
}

//12/04/18 R.sasaki Hddのmountエラー発生時のRetry対応.
// 2014/10/08 yamada	CQ503146 PSWC言語領域不足対応.
//14/11/05 T.simogaki CQ506310 対応.
//16/01/05 furuichi CQ523556 HDDマウントエラー時の不具合対応.
//17/10/10 Yoneda   S800 ストレージ構成変更対応.
//18/09/14 Sky ソリューション分離V2対応（HDD/SSDのマウント＆fsck制御).
//19/02/28 ads長谷川 EagleH_制御構成変更_No.49 swap有効化対応(swapファイル化対応).
//19/04/08 ads長谷川 EagleH_制御構成変更_No.49 swap有効化対応(swap有効化遅延実行対応).
//21/06/16 ads河井  管理者権限の変更処理追加.
int32_t CP_HDDTrans::mountHdd(				// DEF_SupportSSD.
    int32_t encrypt,
    char* pathname,
//【695新アーキFUM】20111111 FBSC渡辺 新アーキ論理フォーマット対応 start.
	THRE_OPTIONS_BIT e_OptionsBit	 // マウント指定.
//【695新アーキFUM】20111111 FBSC渡辺 新アーキ論理フォーマット対応 end.
)
{
    int32_t ret;
    int32_t no;
    int32_t result = 0;
	part_layout*	device_info;
	
	if(strcmp(pathname,theHDDTrans.pc_strDeviceSSD) == 0){
		device_info = &part_info[0];
	} else {
		device_info = &ssd_part_info[0];
	}
	
    if( encrypt < 0 || encrypt > 1 ){
        return -1;
    }
	
	// 「一般ユーザーアクセス変換管理ファイル」を読み込む.
	std::map<std::string, Bool> tmpctrlFileMap;
	Bool bl_Ret = readMfpAccessCtrlFile( tmpctrlFileMap );
	if( bl_Ret == False ){
		printf( "[%s:%d] readMfpAccessCtrlFile NG.\n", __PRETTY_FUNCTION__, __LINE__ );
		return -1;
	}
	
	// 2011/05/30 takayama CQ344683対応 fstypeがFSのみマウントするように修正.
    for( no = 0; device_info[no].type != PART_TYPE_END; no++ ){
// 2018/10/12 Sky新大阪 [AR.1061634]【制御_No.99】/ata07と/ata0eのfstypeを変更(NF2 -> FS,FS_SSD) ->.
        // 優先マウント条件をNF2、NF3に加え、THRE_OPTIONS_BIT_PRE_MOUNT指定のもかどうかをチェックする.
        if(  ( device_info[ no ].mntpnt[0] == 0 )
          || ( ( device_info[no].fstype != PART_FSTYPE_FS ) && ( device_info[no].fstype != PART_FSTYPE_FS_SSD ) )
          || ( THRC_PartitionInfo::chkMountPntOption( device_info[ no ].mntpnt, THRE_OPTIONS_BIT_PRE_MOUNT ) == True ) ) {
            continue;
        }
// 2018/10/12 Sky新大阪 [AR.1061634]【制御_No.99】/ata07と/ata0eのfstypeを変更(NF2 -> FS,FS_SSD) <-.

//【695新アーキFUM】20111111 FBSC渡辺 新アーキ論理フォーマット対応 start.
		// マウントの指定チェック.
		Bool b_chkTargetFlg = THRC_PartitionInfo::chkMountPntOption(device_info[ no ].mntpnt, e_OptionsBit);
		if( b_chkTargetFlg == False ){
			continue;
		}
//【695新アーキFUM】20111111 FBSC渡辺 新アーキ論理フォーマット対応 end.

		if( THRG_IsMountPoint( device_info[ no ].mntpnt ) )
		{
			continue;
		}

        //printf( "[%s] %d [%s] [%s]\n", __PRETTY_FUNCTION__, no, device_info[ no ].devpath[encrypt], device_info[ no ].mntpnt );
		int32_t si_RetryCount = 0;								// Retry用カウンタ.
		for( si_RetryCount = 0; si_RetryCount <= THRD_MountRetryMax; si_RetryCount++ ){
			if (strcmp(pathname,theHDDTrans.pc_strDeviceSSD) == 0) {
				// HDD.
				if(device_info[ no ].fstype != PART_FSTYPE_FS_SSD)
				{
            		ret = mount( device_info[ no ].devpath[encrypt], device_info[ no ].mntpnt, "ext3", MS_NOATIME, EXT3_MOUNT_OPTIONS );
				}else{
	            	ret = mount( device_info[ no ].devpath[encrypt], device_info[ no ].mntpnt, "ext2", MS_NOATIME, NULL );
				}
            	if( ( 0 != ret ) && 													// マウントエラー.
					 ( EBUSY == SYSF_errnoGet() ) && 										//errnoがEBUSY(16)の場合.
					 ( strcmp(device_info[ no ].mntpnt, THRD_DevName_Ssd0a) == 0 ) ){	//対象がata0a.
						//ata0aはfstabでマウントされている為エラー扱いとしない.
						//マウント自体をスキップするとアンマウントされた時にマウントされなくなるため、マウントのスキップはしない.
					ret = 0;
				}
			} else {
				// eMMC.
				ret = mount( device_info[ no ].devpath[encrypt], device_info[ no ].mntpnt, "ext2", MS_NOATIME, NULL );
				if ( ( 0 != ret ) && 													//マウントエラー.
					 ( EBUSY == SYSF_errnoGet() ) && 										//errnoがEBUSY(16)の場合.
					 ( strcmp(device_info[ no ].mntpnt, THRD_DevName_Ssd0a) == 0 ) ){	//対象がata0a.
						//ata0aはfstabでマウントされている為エラー扱いとしない.
						//マウント自体をスキップするとアンマウントされた時にマウントされなくなるため、マウントのスキップはしない.
					 	ret = 0;
				}
			}
			if( ret == 0 ){
				// 該当マウントポイントとディレクトリ配下の権限を変更する.
				if( tmpctrlFileMap.find( device_info[no].mntpnt ) != tmpctrlFileMap.end() ){		//tmpctrlFileMapのmntpnt存在確認.
					if( tmpctrlFileMap[device_info[no].mntpnt] == False ){
						int32_t ChangeAuthorityresult = 0;
						ChangeAuthorityresult = ChangeAuthority( device_info[no].mntpnt, tmpctrlFileMap );
						if( ChangeAuthorityresult == -1 ){
							printf( "[%s:%d] ChangeAuthority NG.\n", __PRETTY_FUNCTION__, __LINE__ );
							return -1;
						}
					}
				}
				break;

			} else if( ( 0 != ret ) && 														// InvalidArgument.
				( EINVAL == SYSF_errnoGet() ) ) {										//errnoがEINVAL(22)の場合.
				break;
				//暗号化・復号化の後は常にエラーなるのでEINVALのエラーはリトライ対象としない.

			}else if( ret == -1 ){
				delayTask( 10 + (si_RetryCount * 50) );		//初回10msecで回を重ねるごとに+50msec追加.
			}
		}
#if defined (DEF_64BIT_PF)	// EagleH_制御構成変更_No.49 swap有効化対応(swapファイル化対応)↓.
		if( strcmp(device_info[ no ].mntpnt, THRD_DevName_Ssd0a) == 0 ){	// ata0a判定.
			SYSC_SwapOnUnlock();		// swapをONする(swap有効化遅延実行対応).
		}
#endif	// EagleH_制御構成変更_No.49 swap有効化対応(swapファイル化対応)↑.
        if( ret == -1 ){
// 2013/07/02 KM(morikawa) SoC LinuxSim JFM merge.
// #ifndef DEF_Soc1200
#if !defined(DEF_Soc1200) || defined(FUM_LINUX)
			if ((THRC_PartitionInfo::chkMountPntOption(device_info[ no ].mntpnt, THRE_OPTIONS_BIT_EXCEPTION_MOUNT)) == True) {
				bl_HddFwAreaError = True;
			}
#endif
            printf( "[%s] %s -> %s errno:%d\n", __FUNCTION__, device_info[ no ].devpath[encrypt], device_info[ no ].mntpnt, SYSF_errnoGet() );
            result = -1;
        }
    }
	
	// 「一般ユーザーアクセス変換管理ファイル」を更新する.
	Bool writeResult = False;
	writeResult = writeMfpAccessCtrlFile( tmpctrlFileMap );
	if( writeResult == False ){
		printf( "[%s:%d] writeMfpUserCtrlFile NG.\n", __PRETTY_FUNCTION__, __LINE__ );
		return -1;
	}
	
	// 16/01/05 furuichi CQ523556 エラー時にマウント情報を出力する.
	if( result == -1 ){
		char cmd[512];
		memset( cmd, 0, sizeof(cmd) );
		sprintf( cmd, "mount");
		system( cmd );
	}
    
    return result;
}
/*================================================================================================
［機能］	doFsckAndMount.
--------------------------------------------------------------------------------------------------
［引数］	なし.							.
［戻値］	.
［詳細］	.
［ｳｫｰｸｽﾙｰ］	.
［履歴］	.
			21/10/11 Sky Fsckで作成されたLost+Fountフォルダを削除関数を作成.
================================================================================================*/
void CP_HDDTrans::delLostPlusFound(
	void
)
{
	int32_t no = 0;
 	char cmd[512] = {'\0'};
 	
 	if(NULL ==  part_info){
 		return;
 	}
	//SSD・SDのファイルシステム分ループする.
	for( no = 0; part_info[no].type != PART_TYPE_END; no++ ){
		if( 0 != strncmp( THRD_DevName_NON, part_info[no].mntpnt, sizeof(THRD_DevName_NON))){		// SSDが指定なしの場合は処理しない.
			//lost+foundフォルダを強制削除.
			snprintf(cmd,sizeof(cmd),"rm -fr %s/%s",part_info[no].mntpnt, THRD_DirName_LostPlusFound);
			system( cmd );
            //printf( "[%s] %d [%s]\n", __PRETTY_FUNCTION__, no,cmd );
		}
	}
}

// 2015/03/02 yamada	CQ511217 C-D010問題 常時暗号化領域(kmsdb8)を平文に戻す対応.
// 2017/10/10 Yoneda    S800 ストレージ構成変更対応.
// 2014/11/05 大嶋 選択中デバイスに対する暗号化デバイスのマウントを行わないように変更.
// 2016/02/05 中村 CQ524977[SecV2]メインストレージの暗号化設定が機能しない.暗号化設定(SSD/HDD)の判別を、関数内で参照している c_DevicePath を判別に用いる.
// 2018/09/21 Sky井柳 【DenebMLK】制御構成変更No78「データ移行」対応 MicroSD起動時のみ暗号化対象化修正.
int32_t CP_HDDTrans::mountHdd(			// DEF_SupportSSD.
    void
)
{
	Bool encrypt = False;
// IT6では、メインストレージの暗号化のみ.
#if 0 // サブストレージ暗号化対応の為に、コードを残しておく.
	if( 0 == strcmp(c_DevicePath, THRD_HDD_DEVICE) ){
		if( theHDDTrans.e_StartUpDeviceType == THRE_StartupDeviceType_SD ){		// MicroSD起動(現時点(2018/09)で暗号化はMicroSDのみ).
			encrypt = (NVDC_SystemSetting::newInstance())->getEnableEncryption();
		}
	}
	else {
		encrypt = THRG_IsSsdEncrypt();
	}
#else //0.
	encrypt = THRG_IsSsdEncrypt();
#endif //0.

//【695新アーキFUM】20111111 FBSC渡辺 新アーキ論理フォーマット対応 start.
	return mountHdd(encrypt, c_DevicePath, THRE_OPTIONS_BIT_NOT_CARE_PRIORITY);
//【695新アーキFUM】20111111 FBSC渡辺 新アーキ論理フォーマット対応 end.
}

//  2017/10/10 Yoneda   S800 ストレージ構成変更対応.
/* 2014/12/04 大嶋 アンマウント失敗を上位に返却 */
// 2016/02/15 中村 出力ログの訂正.
int32_t CP_HDDTrans::unmountHdd(		// DEF_SupportSSD.
    char* pathname
)
{
    int32_t no;
    int32_t ret = 0;
    int32_t funcRet = 0; // retはループ内で上書きする為、戻り値として不適切.
    char fpath[32];
	part_layout*	device_info;

	// 2011/06/07 ft_kondo CQ345333 省電力移行時にumountに失敗する問題. 
    char cmd[512];
	
	if(strcmp(pathname,theHDDTrans.pc_strDeviceSSD) == 0){
		device_info = &part_info[0];
	} else {
		device_info = &ssd_part_info[0];
	}
#if 0 // 2010/08/20 GaiaPF8536対応 河村 - スレッド電源制御部でコントロールする.
	sync();
    sleep( 2 );
#endif

	// 2011/06/07 ft_kondo CQ345333 省電力移行時にumountに失敗する問題. 
	// umount前に、カレントディレクトリを/rootに変更する. 
	if( chdir("/root") == ERROR){
		THRL_ThLogMsgErr("chdir error!! /root\n" );			// 2012/02/29 静的解析対応.
	}

    for( no = 0; device_info[no].type != PART_TYPE_END; no++ ){
        if( device_info[ no ].mntpnt[0] == 0 ){
            continue;
        }
        // アンマウント前にチェックファイルを生成する
        snprintf( fpath, sizeof(fpath), "%s/%s", device_info[no].mntpnt, THRD_FileName_NOFSCK );
        int32_t fd = SYSF_devopen( fpath, O_RDWR | O_CREAT, 0666 );
        if( fd != -1 ){
            fsync( fd );
            SYSF_devclose( fd );
        }
// 2012/05/13 sky_kageyama CQ 364868 start.
//        ret = umount2( device_info[no].mntpnt, MNT_FORCE );
		int32_t retry;
		int32_t si_errorNo = 0;
		for( retry = 0; retry <= THRD_UMOUNT_RETRYMAX; retry++ ){
   		    ret = umount2( device_info[no].mntpnt, MNT_FORCE );
   		    si_errorNo = errno; //umountのerrno取得。
			if( ret == 0 ){
				break;
			}else if( SYSF_errnoGet() == EBUSY ){
				delayTask( 10+(retry*50) );//初回10msecで回を重ねるごとに+50msec追加
			}else{
				printf( "umount2 %d [%s]\n", SYSF_errnoGet(), device_info[no].mntpnt );
				ret = 0;
				break;
			}
		}
// 2012/05/13 sky_kageyama CQ 364868 end.
        if( ret == -1 ){
            printf( "umount2 %d [%s]\n", SYSF_errnoGet(), device_info[no].mntpnt );
			// 2011/06/07 ft_kondo CQ345333 省電力移行時にumountに失敗する問題. 
            memset( cmd, 0, sizeof(cmd) );
			sprintf( cmd, "fuser -va -u %s", device_info[no].mntpnt );
            system( cmd );
            system( "/usr/bin/lsof" );

            us_FA14DetailCode = APIC_getUmountErrorDetailCode( device_info[no].mntpnt, si_errorNo ); //CQ374156 森田 umountエラー詳細コードを取得・保持.
            b_isUmountError = True;	// umountエラー時Reboot対応 2012/01/16 河村 umountエラーを記録.
			funcRet = -1;
        }
    }
    return funcRet;
}

// ================================================================================================
//! <!--機能--> HddFw領域の初期化.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param なし.
//! <!--戻値-->
//! \return int 0:正常終了、0以外：異常終了.
//! <!--詳細-->
//	HddFw領域をマウントして、失敗した場合はフォーマットを行う.
//! <!--履歴-->
//	2013/06/20 宮本 CQ378467 制御構成変更_No1240(HDD保存FWデータの個別インストール)対応.
// ================================================================================================
int32_t CP_HDDTrans::initHddFw( void )		// DEF_SupportSSD.
{
	uint32_t i = 0;
	int32_t i_Ret = 0;

	// パーティションテーブルからHddFw領域の情報を検索する.
	for ( i = 0; part_info[i].type != PART_TYPE_END; i++ ) {
		i_Ret = strcmp(part_info[i].mntpnt, THRD_DevName_Hdd11 );
		if ( i_Ret == 0 ) {
			break;
		}
	}
	if ( part_info[i].type == PART_TYPE_END ) {
		SYSF_logMsg( "[%s](%d) mount point is not found:MntPnt=%s.\n", __FUNCTION__, __LINE__, THRD_DevName_Hdd11 );
		return -1;
	}

	do {
		// HddFw領域が存在するかチェックする.
		i_Ret = access( part_info[i].devpath[DEF_HDD_CRYPT_ON], F_OK );
		if ( i_Ret != 0 ) {
			SYSF_logMsg( "[%s](%d) access error:Path=%s.\n", __FUNCTION__, __LINE__, part_info[i].devpath[DEF_HDD_CRYPT_ON] );
			break;
		}

		// HddFw領域をマウントする.
		i_Ret = mount( part_info[i].devpath[DEF_HDD_CRYPT_ON], part_info[i].mntpnt, "ext3", MS_NOATIME, NULL );
		if ( i_Ret == 0 || SYSF_errnoGet() == EBUSY ) {
			// マウント成功、又は、マウント済の場合はフォーマットせずに終了する.
			i_Ret = 0;
			break;
		}

		// マウント失敗時は、HddFw領域をフォーマットする.
		i_Ret = formatFS( part_info[i].devpath[DEF_HDD_CRYPT_ON] );
	    if ( i_Ret != 0 ) {
			SYSF_logMsg( "[%s](%d) format error:Path=%s.\n", __FUNCTION__, __LINE__, part_info[i].devpath[DEF_HDD_CRYPT_ON] );
	        break;
	    }

		// HddFw領域をマウントする.
		i_Ret = mount( part_info[i].devpath[DEF_HDD_CRYPT_ON], part_info[i].mntpnt, "ext3", MS_NOATIME, NULL );
		if ( i_Ret != 0 ) {
			SYSF_logMsg( "[%s](%d) mount error:Dev=%s Mnt=%s.\n", __FUNCTION__, __LINE__, part_info[i].devpath[DEF_HDD_CRYPT_ON], part_info[i].mntpnt );
			break;
		}
	} while( 0 );

	return i_Ret;
}
#else	// DEF_SupportSSD.

int64_t CP_HDDTrans::getExtSectorNum(
    void
)
{
    int32_t no;
    int64_t secnum = 0;

    for( no = 0; part_info[no].type != PART_TYPE_END; no++ ){
        if( part_info[no].type == PART_TYPE_EXTENDED ){
            secnum = part_info[no].nsec;
            break;
        }
    }
    return secnum;
}

int64_t CP_HDDTrans::getFixedSectorNum(
    void
)
{
    int32_t no;
    int64_t secnum = 0;

    for( no = 0; part_info[no].type != PART_TYPE_END; no++ ){
        if( (part_info[no].type == PART_TYPE_PRIMARY && part_info[no].nsec > 0) ||
            (part_info[no].type == PART_TYPE_EXTENDED && part_info[no].nsec > 0) ){
            secnum += part_info[no].nsec;
        }
    }
    return secnum;
}

int64_t CP_HDDTrans::getFixedLogicalSectorNum(
    void
)
{
    int32_t no;
    int64_t secnum = 0;

    for( no = 0; part_info[no].type != PART_TYPE_END; no++ ){
        if( part_info[no].type == PART_TYPE_LOGICAL && part_info[no].nsec > 0 ){
            secnum += part_info[no].nsec;
        }
    }
    return secnum;
}

int32_t CP_HDDTrans::calcAndFixedPartInfo(
    char* hdddevname
)
{
    int32_t no;
    int64_t start_curr = 0;
    int64_t start_ext = 0;
    DLONG device_size = 0;

    device_size = (DLONG)(getPhysicalHDDSize()/(DLONG)DEF_SECTOR_SIZE);

    for( no = 0; part_info[no].type != PART_TYPE_END; no++ ){
        if( part_info[no].nsec == -1 ){
            switch( part_info[no].type ){
              case PART_TYPE_PRIMARY:
                  //part_info[no].nsec = getAllSectorNum( hdddevname ) - getFixedSectorNum();
                  part_info[no].nsec = (int64_t)device_size - getFixedSectorNum();
                  if( part_info[no].nsec < 0 ){
                      return -1;
                  }
                  break;
              case PART_TYPE_LOGICAL:
                  part_info[no].nsec = getExtSectorNum() - getFixedLogicalSectorNum();
                  break;
              default:
              case PART_TYPE_EXTENDED:
                  SYSF_logMsg( "[%s] part_info[%d].sec -> %"PRId64"\n", __FUNCTION__, no, part_info[no].nsec );
                  break;
            }            
        }
    }

    for( no = 0; part_info[no].type != PART_TYPE_END; no++ ){
        if( !part_info[no].nsec ){
            continue;
        }
        switch( part_info[no].type ){
          case PART_TYPE_EXTENDED:
              start_ext = start_curr;
          case PART_TYPE_PRIMARY:
              part_info[no].end = start_curr + part_info[no].nsec - 1;
              start_curr = part_info[no].end + 1;
              break;
          case PART_TYPE_LOGICAL:
              part_info[no].end = start_ext + part_info[no].nsec - 1;
              start_ext = part_info[no].end + 1;
              break;
        }
    }

#if 0
    for( no = 0; part_info[no].type != PART_TYPE_END; no++ ){
        SYSF_logMsg( "[%s] no.%d %d end:%lld nsec:%lld\n", __FUNCTION__, no, part_info[no].type, part_info[no].end, part_info[no].nsec );
    }
#endif

    return 0;
}

int32_t CP_HDDTrans::checkPartitionLayout(
    char* hdddevname
)
{
    int32_t ret;
    char cmd[256];
    char* ptr;
    char dev[16];
    int64_t endsec;
    int32_t result = 0;
    int32_t no = 0;
    char val[3];
    int32_t pno;
    int32_t pno_pre = -1;

    snprintf( cmd, sizeof(cmd), FMT_PART_FDISK1, hdddevname );
    ret = execprog_in( cmd, sbuf, sizeof(sbuf) );
    if( ret ){
        printf( "execprog_in ERR: cmd:%s\n", cmd );
        THRL_ThAssert( False, "[SATA2] checkPartitionLayout()\n" , 0, 0, 0, 0, CFA14_DetailCode0x01E3 );
        return -1;
    }
    ptr = sbuf;

    while( (ptr = strstr( ptr, hdddevname )) ){
        val[0] = (char)ptr[strlen(hdddevname)];
        if((char)ptr[strlen(hdddevname)+1] == ' ') {
            val[1] = '\0';
        }
        else {
            val[1] = (char)ptr[strlen(hdddevname)+1];
            val[2] = '\0';
        }
        pno = atoi( val );
        if( pno_pre != -1 && pno != pno_pre + 1 ){
            no++;
        }
        dev[0] = 0;
        endsec = 0;
        sscanf( ptr, "%s %lld", dev, &endsec );
        if( endsec != part_info[no].end ){
            result = -1;
            break;
        }

#if 0
        SYSF_logMsg( "[%s] %s %lld part_info %lld\n", __FUNCTION__, dev, endsec, part_info[no].end );
#endif

        no++;
        ptr++;
        pno_pre = pno;
    }

    if( part_info[no].type != PART_TYPE_END ){
        SYSF_logMsg( "Partition layout NG.\n" );
        result = -1;
    }
    return result;
}

int32_t CP_HDDTrans::formatFS(
    char* devpath
)
{
    int32_t ret;
    char cmd[256];

    snprintf( cmd, sizeof(cmd), FMT_PART_MKE2FS, devpath );
	SYSF_logMsg("[%s] execute->[%s]\n", __FUNCTION__, cmd );
    ret = execprog( cmd );
    if( ret ){
        printf( "execprog ERR: cmd:%s\n", cmd );
        return -1;
    }

    sync();

    return 0;
}

int32_t CP_HDDTrans::formatFAT(
    char* devpath
)
{
    int32_t ret;
    char cmd[256];

    snprintf( cmd, sizeof(cmd), FMT_PART_MKFSVFAT, devpath );
    ret = execprog( cmd );
    if( ret ){
        printf( "execprog ERR: cmd:%s\n", cmd );
        return -1;
    }

    sync();

    return 0;
}

int32_t CP_HDDTrans::doFdisk(
    char* devpath
)
{
    int32_t no;
    int32_t pno = 1;
    int32_t ret;
    char cmd[256];
    char val[32];

    memset( sbuf, 0, sizeof(sbuf) );

    strcpy( sbuf, "d\n" );
    strcat( sbuf, "1\n" );
    strcat( sbuf, "d\n" );
    strcat( sbuf, "2\n" );
    strcat( sbuf, "d\n" );
    strcat( sbuf, "3\n" );
    strcat( sbuf, "d\n" );
    strcat( sbuf, "4\n" );

    for( no = 0; part_info[no].type != PART_TYPE_END; no++ ){
        switch( part_info[no].type ){
          case PART_TYPE_PRIMARY:
              if( part_info[no].end ){
                  strcat( sbuf, "n\n" );
                  strcat( sbuf, "p\n" );
                  snprintf( val, sizeof(val), "%"PRId32"\n", pno++ );
                  strcat( sbuf, val );
                  strcat( sbuf, "\n" );
                  snprintf(  val, sizeof(val), "%"PRId64"\n", part_info[no].end );
                  strcat( sbuf, val );
              }
              break;
          case PART_TYPE_EXTENDED:
              strcat( sbuf, "n\n" );
              strcat( sbuf, "e\n" );
              snprintf( val, sizeof(val), "%"PRId32"\n", pno++ );
              strcat( sbuf, val );
              strcat( sbuf, "\n" );
              snprintf( val, sizeof(val), "%"PRId64"\n", part_info[no].end );
              strcat( sbuf, val );
              break;
          case PART_TYPE_LOGICAL:
              strcat( sbuf, "n\n" );
              strcat( sbuf, "l\n" );
              strcat( sbuf, "\n" );
              snprintf( val, sizeof(val), "%"PRId64"\n", part_info[no].end );
              strcat( sbuf, val );
              break;
        }
    }
    strcat( sbuf, "w\n" );

    snprintf( cmd, sizeof(cmd), FMT_PART_FDISK2, devpath );
    ret = execprog_out( cmd, sbuf, strlen(sbuf) );
    if( ret ){
        printf( "execprog_out ERR: cmd:%s\n", cmd );
        THRL_ThAssert( False, "[SATA2] doFdisk()\n" , 0, 0, 0, 0, CFA14_DetailCode0x01E4 );
        return -1;
    }

    /*
     * fdisk実行後、新しいパーティションレイアウトが認識されてHDDのデバイスファイルが
     * 生成されるまでタイムラグがあるためディレイする。
     */
    sleep( 2 );

    return 0;
}

int32_t CP_HDDTrans::doFsck(
    char* devpath,
    int32_t i_fscklevel,
    int32_t fstype
)
{
    int32_t ret;
    char cmd[256];
    int32_t policy;
	struct sched_param sched_new;
	struct sched_param sched_org;

    // FSCK実行時は一時的に優先度を下げる
    pthread_getschedparam( pthread_self(), &policy, &sched_org );
    memcpy( &sched_new, &sched_org, sizeof(struct sched_param) );
    sched_new.sched_priority = 5;
    pthread_setschedparam( pthread_self(), policy, &sched_new );

    if( i_fscklevel ){
        snprintf( cmd, sizeof(cmd), FMT_PART_TUNE2FS1, devpath );
    }else{
        snprintf( cmd, sizeof(cmd), FMT_PART_TUNE2FS0, devpath );
    }
    ret = execprog( cmd );
    if( ret ){
        printf( "execprog ERR: cmd:%s\n", cmd );
        return -1;
    }

    snprintf( cmd, sizeof(cmd), FMT_PART_FSCK1ST, "ext3", devpath );
    ret = execprog( cmd );

    // 自タスクの優先度を元に戻す
    pthread_setschedparam( pthread_self(), policy, &sched_org );

    if( ret == 0 ){
        // エラーなし
        SYSF_logMsg( "fsck 1st %s %"PRId32"\t[ OK ]\n", devpath, i_fscklevel );
        return 0;
    }else if( ret == 1 ){
        // エラーがあったが修復済み
        SYSF_logMsg( "fsck 1st %s %"PRId32"\t[ Repair ]\n", devpath, i_fscklevel );
        return 0;
    }else if( ret > 1 ){
        // エラーあり
        SYSF_logMsg( "fsck 1st %s %"PRId32"\t[ NG ]\n", devpath, i_fscklevel );
    }

    snprintf( cmd, sizeof(cmd), FMT_PART_FSCK2ND, "ext3", devpath );
    ret = execprog( cmd );

    if( ret == 0 ){
        SYSF_logMsg( "fsck 2nd %s %"PRId32"\t[ OK ]\n", devpath, i_fscklevel );
        return 0;
    }else if( ret == 1 ){
        SYSF_logMsg( "fsck 2nd %s %"PRId32"\t[ Repair ]\n", devpath, i_fscklevel );
        return 0;
    }else{
        SYSF_logMsg( "fsck 2nd %s %"PRId32"\t[ Trouble(%"PRId32") ]\n", devpath, i_fscklevel, ret );
    }

    return ret;
}

//  2017/10/10 Yoneda   S800 ストレージ構成変更対応.
int32_t CP_HDDTrans::makeDeviceFiles(
    void
)
{
	int32_t fd;
	int32_t sc_No, sc_Ret, sc_LimitCnt;
	struct stat    st_Stat;

	if( ( fd = SYSF_devopen( theHDDTrans.pc_strDeviceSSD, O_RDWR, 0666 ) ) == -1 ){
		printf( "%s - device file open failed!\n", __FUNCTION__ );
		return -1;
	}
	if( SYSF_ioctl( fd, BLKRRPART, 0 ) == -1 ){
        if( errno == EBUSY ){ // HDDブート対応（マウント済みの場合EBUSYとなる。HDDブートの場合OS起動時にRFSがマウント済み）.
            SYSF_devclose( fd );
            return 0;
        }
		printf( "%s - making device file was failed!\n", __FUNCTION__ );
		SYSF_devclose( fd );
		return -1;
	}

	sleep( 2 );	/* wait to create device files */

	for(sc_No=0; part_info[sc_No].type != PART_TYPE_END; sc_No++) {
#if defined (DEF_64BIT_PF)
		if( ( part_info[sc_No].fstype != PART_FSTYPE_FS ) && ( part_info[sc_No].fstype != PART_FSTYPE_FS_SSD ) || // 2017.10.26 sky パーティション一本化対応.
		  (THRC_PartitionInfo::chkMountPntOption(device_info[ sc_No ].mntpnt, THRE_OPTIONS_BIT_EXCEPTION_MOUNT) == True)) {
#else
		if( part_info[sc_No].fstype != PART_FSTYPE_FS || (THRC_PartitionInfo::chkMountPntOption(device_info[ sc_No ].mntpnt, THRE_OPTIONS_BIT_EXCEPTION_MOUNT) == True)) {
#endif
			continue;
		}

		sc_LimitCnt=0;
		while( 1 ) {
			sc_LimitCnt++;

			sc_Ret = SYSF_stat(part_info[sc_No].devpath[0], &st_Stat);
			if(0 == sc_Ret && S_ISBLK(st_Stat.st_mode)) {
				break;
			}

			if(30 <= sc_LimitCnt) {
				printf( "%s - not found: %s\n", __FUNCTION__, part_info[sc_No].devpath[0] );
				SYSF_devclose( fd );
				return -1;
			}

			sleep( 1 );
		}
	}

	SYSF_devclose( fd );
	return 0;
}
// 2012/01/20 takayama CQ360543 FD残る問題対策 ファイルアクセスAPIをラッパー関数に置き換え.
// 2015/10/23 中村     [SecV2-4]HDD暗号化鍵のHCD-PP適合.
// 2017/11/09	SCD2-Sky Shindo [WP.53-54] 設定値見直し対応.
int32_t CP_HDDTrans::setupDmcrypt(
    void
)
{
    int32_t no;
    char cmd[256];
    int32_t ret;
    char c_KeyFile[32];
	THRC_Crypto *pc_Crypto = NULL;

	pc_Crypto = THRC_Crypto::newInstance();
	if( NULL == pc_Crypto ) {
		THRL_ThAssert( False, "[SATA2] CP_HDDTrans::setupDmcrypt(%d) THRC_Crypto::newInstance() failed.\n", __LINE__, 0, 0, 0, CFA14_DetailCode0x01DF );
		return -1;
	}
	
	/* 鍵情報を生成する. */
	uchar uc_DEK[NVDC_KeyLen] = {0};
	if( 0 != pc_Crypto->getDEK( THRE_StorageDeviceTypeHDD, uc_DEK, NVDC_KeyLen ) ){
		THRL_ThAssert( False, "[SATA2] CP_HDDTrans::setupDmcrypt(%d) getDEK() failed.\n", __LINE__, 0, 0, 0, CFA14_DetailCode0x01DF );
		return -1;
	}
	
	if( 0 != pc_Crypto->mkTempKeyFile(uc_DEK, NVDC_KeyLen, c_KeyFile, sizeof(c_KeyFile) ) ){
		THRL_ThAssert( False, "[SATA2] CP_HDDTrans::setupDmcrypt(%d) mkTempKeyFile() failed.\n", __LINE__, 0, 0, 0, CFA14_DetailCode0x01DF );
		return -1;
	}

    for( no = 0; part_info[no].type != PART_TYPE_END; no++ ){
        /* 暗号化有無でデバイス名が変化しないデバイスは暗号化対象外とする */
        if( strcmp( part_info[ no ].devpath[0], part_info[ no ].devpath[1] ) == 0 ){
            continue;
        }
        if( NVDC_SystemSetting::newInstance()->getEncryptionOrOverWrite() == True ){
            // イニシャルベクタ可変（暗号化優先モード）.
            snprintf( cmd, sizeof(cmd), FMT_PART_DMCRYPT1, part_info[no].devpath[0], c_KeyFile, THRD_CRYPT_KEYSIZE,
                      (uintptr_t)DEF_HDD_OS_START_BLOCK, no+1, part_info[no].devpath[0] );
        }else{
            // イニシャルベクタ固定（上書き優先モード）.
            snprintf( cmd, sizeof(cmd), FMT_PART_DMCRYPT2, part_info[no].devpath[0], c_KeyFile, THRD_CRYPT_KEYSIZE,
                      no+1, part_info[no].devpath[0] );
        }
        ret = execprog( cmd );
        if( ret ){
            // 一次ファイルを削除
        	pc_Crypto->clrTempKeyFile( c_KeyFile );
            printf( "execprog ERR: cmd:%s\n", cmd );
            THRL_ThAssert( False, "execprog ERR\n" , 0, 0, 0, 0, CFA14_DetailCode0x01E7 );
            return -1;
        }
    }

    // 一次ファイルを削除
	pc_Crypto->clrTempKeyFile( c_KeyFile );

    return 0;
}

int32_t CP_HDDTrans::mountHdd(
    int32_t encrypt,
//【695新アーキFUM】20111111 FBSC渡辺 新アーキ論理フォーマット対応 start.
	THRE_OPTIONS_BIT e_OptionsBit
//【695新アーキFUM】20111111 FBSC渡辺 新アーキ論理フォーマット対応 end.
)
{
    int32_t ret;
    int32_t no;
    int32_t result = 0;

    if( encrypt < 0 || encrypt > 1 ){
        return -1;
    }

    for( no = 0; part_info[no].type != PART_TYPE_END; no++ ){
        if( part_info[ no ].mntpnt[0] == 0 ){
            continue;
        }
//【695新アーキFUM】20111111 FBSC渡辺 新アーキ論理フォーマット対応 start.
		// マウントの指定チェック.
		Bool b_chkTargetFlg = THRC_PartitionInfo::chkMountPntOption(part_info[ no ].mntpnt, e_OptionsBit);
		if( b_chkTargetFlg == False ){
			continue;
		}
//【695新アーキFUM】20111111 FBSC渡辺 新アーキ論理フォーマット対応 end.
        ret = mount( part_info[ no ].devpath[encrypt], part_info[ no ].mntpnt, "ext3", MS_NOATIME, EXT3_MOUNT_OPTIONS );
        if( ret == -1 ){
            printf( "[%s] %s -> %s errno:%d\n", __FUNCTION__, part_info[ no ].devpath[encrypt], part_info[ no ].mntpnt, SYSF_errnoGet() );
            result = -1;
        }
    }
    return result;
}

int32_t CP_HDDTrans::mountHdd(
    void
)
{
//【695新アーキFUM】20111111 FBSC渡辺 新アーキ論理フォーマット対応 start.
//    return mountHdd( APIC_SystemSetting::newInstance()->getEnableEncryption() );
// IT6では、メインストレージの暗号化のみ.
#if 0 // サブストレージ暗号化対応の為に、コードを残しておく.
	return mountHdd( APIC_SystemSetting::newInstance()->getEnableEncryption(), THRE_OPTIONS_BIT_NOT_CARE_PRIORITY);
#else //0.
	return mountHdd( THRG_IsSsdEncrypt(), THRE_OPTIONS_BIT_NOT_CARE_PRIORITY);
#endif //0.
//【695新アーキFUM】20111111 FBSC渡辺 新アーキ論理フォーマット対応 end.
}

int32_t CP_HDDTrans::unmountHdd(
    void
)
{
    int32_t no;
    int32_t ret = 0;
    char fpath[32];
// 2012/05/13 sky_kageyama CQ 364868 add.
    char cmd[512];

    sync();
    sleep( 2 );

	if( chdir( "/km/fw" ) == ERROR ){
		THRL_ThLogMsgErr("chdir error!!\n" );
	}

    for( no = 0; part_info[no].type != PART_TYPE_END; no++ ){
        if( part_info[ no ].mntpnt[0] == 0 ){
            continue;
        }
		Bool b_chkTargetFlg = THRC_PartitionInfo::chkMountPntOption( part_info[ no ].mntpnt, THRE_OPTIONS_BIT_NOT_CARE_PRIORITY );
		if( b_chkTargetFlg == False ){
			continue;
		}
        // アンマウント前にチェックファイルを生成する
        snprintf( fpath, sizeof(fpath), "%s/%s", part_info[no].mntpnt, THRD_FileName_NOFSCK );
        int32_t fd = SYSF_devopen( fpath, O_RDWR | O_CREAT, 0666 );
        if( fd != -1 ){
            fsync( fd );
            SYSF_devclose( fd );
        }
// 2012/05/13 sky_kageyama CQ 364868 start.
//        ret = umount2( part_info[no].mntpnt, MNT_FORCE );
		int32_t retry;
		for( retry = 0; retry <= THRD_UMOUNT_RETRYMAX; retry++ ){
			ret = umount2( device_info[no].mntpnt, MNT_FORCE );
			if( ret == 0 ){
				break;
			}else if( SYSF_errnoGet() == EBUSY ){
				delayTask( 10+(retry*50) );//初回10msecで回を重ねるごとに+50msec追加
			}else{
				printf( "umount2 %d [%s]\n", SYSF_errnoGet(), part_info[no].mntpnt );
				ret = 0;
				break;
			}
		}
// 2012/05/13 sky_kageyama CQ 364868 end.
        if( ret == -1 ){
            printf( "umount2 %d [%s]\n", SYSF_errnoGet(), part_info[no].mntpnt );
// 2012/05/13 sky_kageyama CQ 364868 start.
            memset( cmd, 0, sizeof(cmd) );
			sprintf( cmd, "fuser -va -u %s", device_info[no].mntpnt );
            system( cmd );
            system( "/usr/bin/lsof" );
// 2012/05/13 sky_kageyama CQ 364868 end.
        }
    }
    return ret;
}

#endif
#endif

/*================================================================================================
［機能］	【内部】OSの初期化
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	True:成功、False：失敗
［詳細］	dosFs2.0の初期化を行う
 [履歴】	01/12/17 morikawa dosFS電源瞬断プロテクトに対応してPCシミュレーション仕様追加
 				IJCのファイルのオープン、クローズ関数名称変更  
 				但し、デバッグ中のため一部コメントアウト
 			01/12/21 morikawa  dosFSの使用領域は先頭からboarddef.hに定義されたエリアに限定する　	
 			01/21/21 morikawa まだデバッグが完全ではないので、デフォルトではパーティション無効、chkdsk()も行わない
 			01/12/27 morikawa パーティション有効とする 
 			02/01/08 morikawa chkdsk有効とする
 			02/01/30 morikawa  
 					・メモリチェックファンクション正式対応に伴い、フォーマット処理分離、不要な処理削除
 					パーティションは常にありとする。
 					・ideDevCreate()の第３引数のoffsetが0でなかったため、dosFSがHDDの先頭から割り当てられてなくて
 					２ＧＢの制限を越えていた不具合修正。　
 			02/02/07 中田		dosFsVolFormatのフォルダ文字列指定に(void*)を付加（低速ＦＵＭコンパイルエラー対策）
 			02/02/21 morikawa	IJCのパーティションを毎回chkdskしていしまう不具合修正
 							原因：
 								chkReadOSdriver()にてチェック用のファイルをfopenする際に電源瞬断フラグをセットするが、
 								キャッシュフラッシュが終了してこのフラグがリセットされる前にchkdsk判定を行っていた為。
 							対策：
								IJCのchkdsk実行判定の後、電源瞬断フラグがセットされていない（chkdsk()を行わない）時のみ
								chkReadOSdriver()を行うように修正　
 			02/05/28 morikawa	MIOとAPP0(プリンタアプリ）は立ち上げ時に毎回ファイル初期化を行うように修正 
 			02/06/18 morikawa	MIOの領域削除に伴い、APP1のエリアを旧MIOの領域分（2GByteの23%）をAPP1(スキャナアプリ）に加算する仕様変更
 								旧仕様では４つのパーティションに分割していたが、MIO削除により３つのパーティションに変更 
 			02/06/26 morikawa	スキャナアプリのエリア（app1)をchkdsk()を行わないで起動すると、なぜかTASK_API_NMで例外が発生するので
 								起動時は毎回app1はchkdsk()を行う
 								・例外発生時のログ
									Exception current instruction address: 0x00000004
									Machine Status Register: 0x0008b030
									Condition Register: 0x24000040
									Task: 0x2f69b58 "TASK_API_NM"
									0x2f69b58 (TASK_API_NM): Ecception!!!
									0x2f69b58 (TASK_API_NM): task            = 49716056
									0x2f69b58 (TASK_API_NM): vecNum          = 1792
									0x2f69b58 (TASK_API_NM): pESf            = 49714896
									0x2f69b58 (TASK_API_NM): pESf->spCopy    = 49715088
									0x2f69b58 (TASK_API_NM): pESf->vecOffset = 1792
									0x2f69b58 (TASK_API_NM): pESf->_errno    = 779384948
 									-> tt TasASK_API_NM
									5840e8 vxTaskEntry    +60 : taskAPI_NM ()
									24e80f0 taskAPI_NM     +3c : NM_Init(void) ()
									24e814c NM_Init(void)  +3c : N_CFuncManMIB::vProcRptInitData(void) ()
									24afab4 N_CFuncManMIB::vProcRptInitData(void)+c0 : n_vAtInitMsgFromIJC(void) ()
									24e82a8 n_vAtInitMsgFromIJC(void)+bc : N_CAdminUserBox::initCheck(void) (1)
									24e77ac N_CAdminUserBox::initCheck(void)+78 : c_HDD_Directry::init(void) ()
									24e6ab4 c_HDD_Directry::init(void)+40 : c_HDD_Directry::loadHDDDirTable(void) ()
									24e744c c_HDD_Directry::loadHDDDirTable(void)+4c : APP1fopen(void *, void *) (28f5d6c, 28f5d84)
									19fbc8 APP1fopen(void *, void *)+8c : fopen ()
									5031bc fopen          +4c : open ()
									524684 open           +dc : iosOpen ()
									525b24 iosOpen        +40 : 10cd3c (2d4dfd8, 2d999f0, 0, 8000)
									10d004 dosFsChkDsk    +22e0: 0 ()
									value = 0 = 0x0
 			02/09/09 morikawa	DosFsの初期化処理実行時にＨＤＤの転送タイムアウトが検出できない不具合修正
 								対策：
 									今までは、この関数は共有データセマフォのロック状態でコールされていたが、
 									タイムアウト発生時は、その他の共有データをアクセスするタスクに処理が回らないため
 									共有データセマフォをアンロックした状態でこのタスクがコールされるように変更になった。
 									しかし、共有データセマフォのアンロック状態にて、chkdsk()等の実行時間の長い処理が起動されると
 									その途中でこのタスク（TID_Image=50)より優先順位の低い共有データを用いるタスクが起動される。
 									しかし、HDDの転送完了により、この優先順位の低いタスクが共有データをロックしたまま、
 									このタスクに再度切り替わってしまうと。最後のchkReadOSdriver()の処理の中で
 									セマフォのロックアンロックの状態が入れ替わってしまいハングアップする。
 									したがって、chkReadOSdriver()の前では再度共有データロック処理を行い、処理実行後にこの関数が呼ばれた時の
 									セマフォアンロック状態に戻す。
			04/01/19 t.sasaki	dcacheDevCreate()の第一引数は型チェックに引っかかるので(void*)でキャストする。
			04/02/02 morikawa   Rhein1 "/ata01"は毎回フォーマットしないように修正　
			04/06/09 ikawa	不要な処理を削除
			04/07/08 morikawa warning対策 
			04/08/23 morikawa　dosFSの領域拡大に伴い、パーティション割合を修正　 
						旧　3,73,2(３つに分ける、１番目/ata01=25%,２番目/a0/app1=73%，３番目/a0/ijc=2%)
						新　3,33,2(３つに分ける、１番目/ata01=65%,２番目/a0/app1=33%，３番目/a0/ijc=2%)
			 		TSS2のロングネームで読み書き不具合の修正をRhein1にも反映
							Formatのパラメータを
							MSFT Long Names(VFAT)からWRS Long Names(VxLong)に修正
					Laplataと同じように、dosFsDevCreate()の第３引数の同時ファイルオープン数を20から256に変更　		
			04/09/13 t.sasaki 起動時のC-5023(メッセージキューフロー)対策－起動時のdosFSアクセスを極力減らす.
			04/10/05 t.sasaki chkdsk条件変更.
			05/06/13 t.sasaki グローバルセマフォ解放＆再確保処理の変更(今までの実装は解放できていない).
			05/06/20 t.sasaki 暗号化ASIC対応－暗号化異常発生時はchkdskしない(ファイルシステムは初期化する).
			05/07/15 t.sasaki DosFSエラー出力の抑止(dosFsDebug=0)、フォーマットチェック追加、自動フォーマット廃止
			05/07/22 t.sasaki SimWinではチェックファイル有無によらず正常起動させる.
			06/02/10 morikawa Mosel 60G対応
						旧　3,33,2(３つに分ける、１番目/ata01=65%,２番目/a0/app1=33%，３番目/a0/ijc=2%)
						新　3,20,2(３つに分ける、１番目/ata01=78%,２番目/a0/app1=20%，３番目/a0/ijc=2%)
			06/04/19 morikawa Mosel 
				 	dosFsDevCreate()の最後の引数のautoChkLevelを0と指定していたため
				 	DOS_CHK_ONLY | DOS_CHK_VERB2の設定になり
				 	chkdskを行って、大量のログがターミナルに出力され時間がかかっていた不具合修正
				 	対策：autoChkLevelはNONEに変更して、dosFsDevCreateではchkdskは行わない
			07/07/11 morikawa Donau dosFSのパーティション見直し、実機とSimでの処理を共通化
					160GByteのHDD対応
						【課題】暫定割合
								１番目	THRD_DevName_Hdd01=78%
								２番目	THRD_DevName_Hdd03=12%
								３番目	THRD_DevName_Hdd04=10%
			07/11/5 morikawa Donau dosFs容量決定
						Emperorは6GByteから3GByteへ
					 			１番目	THRD_DevName_Hdd01=81%
								２番目	THRD_DevName_Hdd03=10%
								３番目	THRD_DevName_Hdd04=9%
					さらに、全てのパーティションでユニコード対応を行う　			
			08/02/13 morikawa Donau機種対応 dosFs容量決定
					 			１番目	THRD_DevName_Hdd01=79%
								２番目	THRD_DevName_Hdd03=10%
								３番目	THRD_DevName_Hdd04=10%
								４番目	THRD_DevName_Hdd05=1%
			08/04/18 kurachi CQ:302140 HDDレイアウト変更（dosFS容量変更）.
						/ata04（音声・動画管理領域）は5GByteから10GByteへ.
						/ata05の管理データベースを/ata04に移動。/ata05のパーティションを削除.
					 			１番目	THRD_DevName_Hdd01=77%.
								２番目	THRD_DevName_Hdd03=7%.
								３番目	THRD_DevName_Hdd04=16%.
			09/04/15 koshimo  Taiga CQ:316762 SSD標準化対応（SSD/HDD dosFsマッピング変更）.
			09/05/19 koshimo  Taiga CQ:317056 SSD構成におけるUSBダイレクトプリント対応.
			09/08/02 takayama Taiga CQ:318248 SSDステータス別管理対応.
			10/05/17 nakagawa CQ:323484 強制的にfsck簡易版に変更.
			10/05/17 nakagawa CQ:327450 アンマウント失敗時にリトライ処理を行う.
			10/05/17 太田	  CQ:327450 デバッグ用として、fsck詳細版に切り替えれるよう修正.
			10/05/28 takayama GaiaPF fsckを常に簡易で実行する.
			10/06/3  takayama GaiaPF HDDレス時にata03をRamDiskで生成する.
			10/07/17 iizuka   CQ.328698 fdisk実施時にデバイスファイル作成を行うように修正.
			11/02/01 takayama MinervaZeusでUSBダイレクトプリントを機能をドロップ RamDiskを生成をコメントアウト.
			11/04/06 takayama MinervaZeus SSD512MB対応.
			11/05/30 takayama CQ344683対応. 
			11/07/05 yamashita MLK用のSSDパーティション切りなおしの暫定対応を削除.
			12/02/29 N.Kato   静的解析対応.
			12/04/17 M.Takagaki   SoC起動時間バラツキ抑制対応.
            13/02/14 Takatsuji	FA14のアサート文字列のリファイン.
			13/02/19 ADS横山 CQ.378292 Act_63200 MS-EFS-16 SSD利用対応 サブディレクトリ作成処理追加.
			14/03/13 伊藤史. CQ386433 コピー可タイミングとinitOSdriverが重なると圧縮完了割込みが阻害されFA17となる対策. 5秒遅延させる.
			15/03/02 yamada	  CQ511217 C-D010問題 常時暗号化領域(kmsdb8)を平文に戻す対応.
			15/04/13 Sky井柳 【IT5_4.0】AR1018857 THRD_EMP_SWAP_PATHの作成をeMMC(SSD)指定でのみ実行するように修正.
			17/04/19 Sky下垣 AR1017776再提起問題対応(eMMCの初期処理<initOSdriver>が完了前はログ出力はしない).
            17/10/10 Yoneda   S800 ストレージ構成変更対応.
            14/11/05 oshima   cPP暴露保護対応.
			15/10/09 中村   [SecV2-4]SecV1時の対応をマージ.
			16/02/05 中村   CQ524977[SecV2]メインストレージの暗号化設定が機能しない.暗号化設定(SSD/HDD)の判別を、device_path の判別に同期する.
			17/10/26 Sky	パーティション一本化対応.
			18/03/22 井柳 [Eagle]制御構成変更No97「ストレージ装着設定対応」タスクモニタ出力の許可フラグのON条件をSSD(μSD)からメインストレージ指定時に変更.
			18/06/25 Ogura AR.1049050 SSDロック＋イニシャライズ後に起動しない問題対応.
			18/09/21 Sky井柳 【DenebMLK】制御構成変更No78「データ移行」対応 MicroSD起動時のみ暗号化対象化修正と、データ移行チェック処理追加.
			19/01/28 Sky井柳	【IT6_1.0MR】[制御構成変更_No.8] initOSdriver成功状態の設定追加.
			20/01/31 Sky      FW検証モードで使用するハッシュ値をスレッド/BPS領域にバックアップする対応.
================================================================================================*/
#ifdef DEF_SupportSSD
// ※ ※ ※ ※ ※ ※ ※ ※ ※ ※ ※ ※ ※ ※ ※ ※ ※ ※ ※ ※  //
// ※ 本処理呼び出し時にはグローバルセマフォを解放すること！	//
// ※ ※ ※ ※ ※ ※ ※ ※ ※ ※ ※ ※ ※ ※ ※ ※ ※ ※ ※ ※  //
Bool CP_HDDTrans::initOSdriver(char *device_path)		// DEF_SupportSSD.
{
    uchar	uc_stat;
	TYPE_MOUNT_AREA_BIT e_AreaBit;
	APIC_HddInfo *p_HddInfo = NULL;
	THRE_StorageDeviceType uc_deviceType = getNvRamDeviceType();
	THRE_StorageDeviceType init_devicetype  = 0;
	Bool bl_ret = False;
#ifdef TORNADO
    int32_t ret;
    Bool encrypt = False;
	// AR1060044対応(暫定). 現状は.
	// sdの場合にpc_strDeviceSSDをkmsdaに設定するとSDの時もinit_devicetypeがTHRE_StorageDeviceTypeHDDになり、.
	//chkExistOSdriver()でエラーとなる(init_devicetypeがTHRE_StorageDeviceTypeHDDのため)のでpc_strDeviceSSDでの判定をやめる.
	//現状μSD+SSD構成はないため、問題にはならないが見直しは必要.
	if ( THRE_StartupDeviceType_SSD == THRF_GetStartupDeviceType() ) {
	//if(strcmp(device_path,theHDDTrans.pc_strDeviceSSD) == 0){
		init_devicetype = THRE_StorageDeviceTypeHDD;
// IT6では、メインストレージの暗号化のみ.
#if 0 // サブストレージ暗号化対応の為に、コードを残しておく.
		encrypt = (NVDC_SystemSetting::newInstance())->getEnableEncryption();
#else //0.
		encrypt = THRG_IsSsdEncrypt();
#endif //0.
	} else {
		init_devicetype = THRE_StorageDeviceTypeSSD;
		encrypt = THRG_IsSsdEncrypt();
	}
	
	// 起動シーケンス変更(2012.07.11).
	if ( init_devicetype == THRE_StorageDeviceTypeSSD ) {
		if ( THRL_Drvrif_IsLineMode() == 1 ) {
			printf( "[%s] THRE_StorageDeviceTypeSSD NoWait=10sec\n", __PRETTY_FUNCTION__ );
		} else {
			usleep(5000000);
			// 2014/03/13 伊藤史. CQ386433 コピー可タイミングとinitOSdriverが重なると圧縮完了割込みが阻害されFA17となる対策. 5秒遅延させる.
			usleep(5000000);
		}
	} else {
		// 2014/03/13 伊藤史. CQ386433 コピー可タイミングとinitOSdriverが重なると圧縮完了割込みが阻害されFA17となる対策. 5秒遅延させる.
		usleep(5000000);
	}

    printf( "[%s] [%s] init_devicetype:%d\n", __PRETTY_FUNCTION__, device_path, init_devicetype );

    // 実装HDDの情報を元にパーティションレイアウトテーブルを構築
	ret = calcAndFixedPartInfo( device_path );
	if (ret == -1) {
		if ((init_devicetype == THRE_StorageDeviceTypeSSD) && (uc_deviceType == THRE_StorageDeviceTypeHDDAndSSD)){
			bl_SsdNgFlg = True;
			SYSF_logMsg( "[SATA2] initOSdriver() SSD calcAndFixedPartInfo ERR\n" );
            goto error;
		} else {
    		THRL_ThAssert( False, "[SATA2] CP_HDDTrans::initOSdriver() HDD calcAndFixedPartInfo ERR\n" , 0, 0, 0, 0, CFA14_DetailCode0x01E8 );
    	}
	}

    printf( "[%s] calcAndFixedPartInfo() pass.\n", __PRETTY_FUNCTION__ );

	// 実装HDDのパーティション構成をチェック
	ret = checkPartitionLayout( device_path );
	if (ret == -1 ) {
		//	2018/06/25 Ogura AR.1049050 SSDロック＋イニシャライズ後に起動しない問題対応.
		//	イニシャライズでロック設定が解除（SSDはロック状態のまま）され、リバートで全領域がクリアされる.
		//	μSD＋オプションストレージの構成はないが、制御は残しておくために条件をμSD起動＋オプションストレージとする.
#if 0
		if((THRE_StartupDeviceType_SD == e_StartUpDeviceType) && (init_devicetype == THRE_StorageDeviceTypeHDD)) {
			SYSF_logMsg( "partition size\t[ NG ]\n" );
		    if(theHDDTrans.e_StartUpDeviceType == THRE_StartupDeviceType_SSD) {				// 2018/05/21 Sky 制御変更99.Type_SSDに変更. 
    		    // TCG OPALアクティベーション
    	    	ret = doSedOpal( device_path );
    			if(ret == -1) {
    	      	  	THRL_ThAssert( False, "[SATA2] CP_HDDTrans::initOSdriver() HDD doSedOpal ERR\n" , 0, 0, 0, 0, CFA14_DetailCode0x01E9 );
    	    	}
    	    	// パーティション作成
    	    	ret = doFdisk( device_path );
    			if(ret == -1) {
    	      	  	THRL_ThAssert( False, "[SATA2] CP_HDDTrans::initOSdriver() HDD doFdisk ERR\n" , 0, 0, 0, 0, CFA14_DetailCode0x01E9 );
    	    	}
		    } else {
    			THRL_ThAssert( False, "[SATA2] CP_HDDTrans::initOSdriver() HDD PartitionLayout ERR\n", 0, 0, 0, 0, CFA14_DetailCode0x08EF );
		    }
	    } else {
			THRL_ThAssert( False, "[SATA2] CP_HDDTrans::initOSdriver() SSD PartitionLayout ERR\n", 0, 0, 0, 0, CFA14_DetailCode0x08EF );
		} 
#else
		THRL_ThAssert( False, "[SATA2] CP_HDDTrans::initOSdriver() SSD PartitionLayout ERR\n", 0, 0, 0, 0, CFA14_DetailCode0x08EF );
#endif
	}
	
    printf( "[%s] checkPartitionLayout() [%s] pass.\n", __PRETTY_FUNCTION__, device_path );

	if(init_devicetype != THRE_StorageDeviceTypeSSD){
		//	2010.07.17	CQ.328698対応 飯塚.
		// デバイスファイル作成.
		ret = makeDeviceFiles(device_path);
		if(ret == -1) {
			// 2012/02/29 N.Kato 静的解析対応.
			THRL_ThAssert( False, "[SATA2] CP_HDDTrans::initOSdriver() HDD makeDeviceFiles ERR\n" , 0, 0, 0, 0, CFA14_DetailCode0x01EA );
		}
	}
	
	SYSF_logMsg( "partition size\t[ OK ]\n" );
#ifdef DEF_Soc1200	/* WUTバラツキ対策 */
	SYSF_taskDelay( 60 );	/* 300ms */
#endif	/* DEF_Soc1200 */

// この暗号化は、サブストレージの暗号化であるため、IT6では、現状無し.
#if 0 // サブストレージ暗号化対応の為に、コードを残しておく.
	// dm-crypt設定
	if ((init_devicetype == THRE_StorageDeviceTypeHDD) && ((uc_deviceType == THRE_StorageDeviceTypeHDDAndSSD) || 
	                                                       (theHDDTrans.e_StartUpDeviceType == THRE_StartupDeviceType_SSD))){
		if( encrypt ){
    		ret = setupDmcrypt(device_path,DEF_HDD_CRYPT_ON);	// 2010/10/18 takayama CQ333996 HDD暗号化時に省電力から復帰しない問題対応.
			if (ret == -1) {
     	   		THRL_ThAssert( False, "[SATA2] CP_HDDTrans::initOSdriver() HDD setupDmcrypt ERR\n" , 0, 0, 0, 0, CFA14_DetailCode0x01EB );
    		}
    	}
#ifdef DEF_Zeus5BkCus
		ret = setupDmcryptHddFw(DEF_HDD_CRYPT_ON);				// 2012/08/27 miyamoto WBS7112  HDDパーティション暗号化対応.
		if (ret == -1) {
 	   		THRL_ThAssert( False, "[SATA2] CP_HDDTrans::initOSdriver() HDD setupDmcryptHddFw ERR\n" , 0, 0, 0, 0, CFA14_DetailCode0x0A76 );
		}
#endif	// DEF_Zeus5BkCus.
 	}
#endif // 0.

	// データ移行チェック処理.
	checkDataMove();

	// ファイルシステムをマウント
//【695新アーキFUM】20111111 FBSC渡辺 新アーキ論理フォーマット対応 start.
//	ret = mountHdd( encrypt,device_path );
	ret = mountHdd( encrypt,device_path, THRE_OPTIONS_BIT_NOT_CARE_PRIORITY);
//【695新アーキFUM】20111111 FBSC渡辺 新アーキ論理フォーマット対応 end.
	if( ret == -1 ){
    	if ((init_devicetype == THRE_StorageDeviceTypeSSD) && (uc_deviceType == THRE_StorageDeviceTypeHDDAndSSD)){
			bl_SsdNgFlg = True;
		}
    	SYSF_logMsg( "[%s] mountHdd Failed device_path %s\n", __FUNCTION__, device_path );
        goto error;
	}
#if 0	//2011/1/12 takayama HDDレスでUSBダイレクトプリントを行う場合に必要 MinervaZeusでドロップ.
	// 10/06/3  takayama GaiaPF HDDレス時にata03をRamDiskで生成する.
    if( uc_deviceType == THRE_StorageDeviceTypeSSD ){
		ret = createRamDisk(RAMDISK_NAME,RAMDISK_SIZE);
		if(ret == -1){
			SYSF_logMsg( "[%s] createRamDisk Failed\n", __FUNCTION__ );
			return False;
		}
	}
#endif
#endif

	// フォーマット中フラグセット時はchkdskを実行しない 2004/09/30 added by t.sasaki
	uc_stat = (uchar)NVDC_SystemSetting::newInstance()->getFsStat();
	THRL_ThLogMsgErr( "@@@@@ FsStat=[0x%02X] @@@@@\n",uc_stat );
	if ( (uc_stat & TYPD_SystemFsStatFormatting) != 0 ) {
        if ((init_devicetype == THRE_StorageDeviceTypeSSD) && (uc_deviceType == THRE_StorageDeviceTypeHDDAndSSD)){
			bl_SsdNgFlg = True;
		}
        SYSF_logMsg( "[initOSdriver] %s getFsStat() %02x TYPD_SystemFsStatFormatting\n", ((init_devicetype == THRE_StorageDeviceTypeHDD) ? "HDD" : "SSD"), uc_stat );
		goto error;
	}

	// 本当にフォーマットされていないかはチェックファイルの有無で判定.
// Str 2010/09/13 ft_kondo 8536環境Sim対応. 
#ifdef TORNADO
	if (((init_devicetype == THRE_StorageDeviceTypeHDD) && (uc_deviceType == THRE_StorageDeviceTypeHDDAndSSD)) ||
#else
	if ( (uc_deviceType == THRE_StorageDeviceTypeHDDAndSSD) ||
#endif // #ifdef TORNADO.
	    (uc_deviceType == THRE_StorageDeviceTypeSSD)){
		bl_ExistCheckFile = chkExistOSdriver( init_devicetype );	// チェックファイル有無判定.
		if( bl_ExistCheckFile == False ){
			if ((init_devicetype == THRE_StorageDeviceTypeSSD) && (uc_deviceType == THRE_StorageDeviceTypeHDDAndSSD)){
				bl_SsdNgFlg = True;
			} 
			//AR1060044対応.
			SYSF_logMsg( "[initOSdriver] chkExistOSdriver() Err bl_ExistCheckFile:%u init_devicetype:%u uc_deviceType:%u \n", bl_ExistCheckFile, init_devicetype, uc_deviceType );
            goto error;
		}
	}

//2019/12/02 Sky 【自動測定】起動イベントの通知対応(Step2) start.
	/* 通常パーティションマウント 開始を通知 */
	THRF_setEventRecord(THRD_EventRecord_PertitionMountStart);
//2019/12/02 Sky 【自動測定】起動イベントの通知対応(Step2) end.
	bl_ret = doFsckAndMount(device_path, init_devicetype, uc_deviceType, THRE_OPTIONS_BIT_NORMAL_MOUNT );
//2019/12/02 Sky 【自動測定】起動イベントの通知対応(Step2) start.
	/* 通常パーティションマウント 終了を通知 */
	THRF_setEventRecord(THRD_EventRecord_PertitionMountEnd);
//2019/12/02 Sky 【自動測定】起動イベントの通知対応(Step2) end.
	if( bl_ret == False ){
		SYSF_logMsg( "[initOSdriver] %s doFsckAndMount Nomal NG\n", ((init_devicetype == THRE_StorageDeviceTypeHDD) ? "HDD" : "SSD") );
		return False;					// IJCフォーマットチェック用ファイル読み込み失敗
	}
#if 0
	//368238対応.
	bl_ret = doFsckAndMount(device_path, init_devicetype, uc_deviceType, THRE_OPTIONS_BIT_EXCEPTION_MOUNT );
	if( bl_ret == False ){
		//エラーは無視.
		SYSF_logMsg( "[initOSdriver] doFsckAndMount Exception NG\n" );
	}
#endif
		if( chkReadOSdriver(init_devicetype) == False ){
		if ((init_devicetype == THRE_StorageDeviceTypeSSD) && (uc_deviceType == THRE_StorageDeviceTypeHDDAndSSD)){
			bl_SsdNgFlg = True;
		}
		SYSF_logMsg( "[initOSdriver] %s chkReadOSdriver() NG\n", ((init_devicetype == THRE_StorageDeviceTypeHDD) ? "HDD" : "SSD") );
		return False;					// IJCフォーマットチェック用ファイル読み込み失敗
	}
//【695新アーキFUM】20111111 FBSC渡辺 新アーキ論理フォーマット対応 end.

	m_c_MirroringBackupData.initMirroringBackupData();	// 制御構成変更No.4 HDDミラーリング対応 不揮発データの初期化を行う.

#if defined(DEF_Soc1200) && !defined(FUM_LINUX) // SSD利用対応 暫定対応.
	// THRD_EMP_SWAP_PATHの作成は、eMMC(SSD)のみが対象の処理であるため、実行のための条件を追加.
	if( init_devicetype == THRE_StorageDeviceTypeSSD ){
		ret = hddtrans_mkdir( THRD_EMP_SWAP_PATH );
		if( ret == -1 ){
			SYSF_logMsg( "%s: ERR: mkdir:%d\n", __FUNCTION__, SYSF_errnoGet() );
			return False;
		}
	}
#endif
	if ( strcmp(device_path, THRD_DEVFILE_SSD ) == 0 ){	// メインストレージ(実装時、kmsda)の初期化完了.
		gb_Tm_exec = True;// タスクモニタ出力の許可フラグ（タスクモニタの出力先は、/ata0a）.
		SYSF_logMsg( "[initOSdriver] %s End  \n", device_path );
	}

	if ( THRL_Drvrif_IsLineMode() == 1 ) {

		ushort	us_loopC = 0;
		uchar	sc_OSSIndexHash[THRD_OSS_Index_Hash_Size];
		uchar	uc_bspthr_reg_ret;

		APIC_OSSIndividualUpdateSetting* pc_OSSIndividualUpdateSetting = APIC_OSSIndividualUpdateSetting::newInstance();
		if (NULL == pc_OSSIndividualUpdateSetting) {
			printf( "[%s](%d) Error, failed APIC_OSSIndividualUpdateSetting::newInstance.\n", __FUNCTION__, __LINE__ );
			return False;
		}

		for( us_loopC = 0; us_loopC < THRD_OSSINDEXHASH_UPDATE_COUNT; us_loopC++ ) {
			memset( sc_OSSIndexHash, 0x00, sizeof( sc_OSSIndexHash ) );
			uc_bspthr_reg_ret = THRG_getBspThrRegion( st_OSSIndexSPIFlashOffset[us_loopC], sc_OSSIndexHash, sizeof(sc_OSSIndexHash) );
			if( THRD_SET_SPI_OK == uc_bspthr_reg_ret ) {
				if ( False == pc_OSSIndividualUpdateSetting->setOSSIndexHash((char*)sc_OSSIndexHash, THRD_OSS_Index_Hash_Size, e_OSSIndexHashType[us_loopC])) {
					printf( "[%s](%d) Error, failed setOSSIndexHash(%d).\n", __FUNCTION__, __LINE__, us_loopC );
				} else {
					printf( "[%s](%d) OSSIndexHash Restore OK(%d).\n", __FUNCTION__, __LINE__, us_loopC );
				}
			} else {
				printf( "%s(%d) Error, failed OSSIndexHash get(%d).\n", __FUNCTION__, __LINE__, us_loopC );
			}
		}

		uchar	sc_IndexHash[THRD_INDEX_Hash_Size] = {0};
		uchar	uc_BootHash[THRD_BOOT_Hash_Size] = {0};
		APIC_SystemSetting *pc_SecuritySystemSetting = APIC_SystemSetting::newInstance();
		if( NULL == pc_SecuritySystemSetting ) {
			printf( "[%s](%d) Error, failed APIC_SystemSetting::newInstance.\n", __FUNCTION__, __LINE__ );
			return False;
		}

		uc_bspthr_reg_ret = THRG_getBspThrRegion( THRD_SPI_OFFSET_INDEXHASH, sc_IndexHash, sizeof(sc_IndexHash) );
		if( THRD_SET_SPI_OK == uc_bspthr_reg_ret ) {
			if ( False == pc_SecuritySystemSetting->setIndexHash( sc_IndexHash, THRD_INDEX_Hash_Size ) ){
				printf( "[%s](%d) Error, failed setIndexHash.\n", __FUNCTION__, __LINE__ );
			} else {
				printf( "[%s](%d) IndexHash Restore OK.\n", __FUNCTION__, __LINE__ );
			}
		} else {
			printf( "%s(%d) Error, failed IndexHash get.\n", __FUNCTION__, __LINE__ );
		}

		uc_bspthr_reg_ret = THRG_getBspThrRegion( THRD_SPI_OFFSET_BOOTROMHASH, uc_BootHash, sizeof(uc_BootHash) );
		if( THRD_SET_SPI_OK == uc_bspthr_reg_ret ) {
			if ( False == pc_SecuritySystemSetting->setBootHash(uc_BootHash, THRD_BOOT_Hash_Size ) ){
				printf( "[%s](%d) Error, failed setBootHash.\n", __FUNCTION__, __LINE__ );
			} else{
				printf( "[%s](%d) BootHash Restore OK.\n", __FUNCTION__, __LINE__ );
			}
		} else {
			printf( "[%s](%d) Error, failed BootHash get.\n", __FUNCTION__, __LINE__ );
		}
	}

    printf( "$$initOSdriver OK\n" );
    m_bl_initOSdriverOK = True;	// initOSdriver成功状態.
	// OP_BTS-27490 Tsukiji 【IT6 2.0MR3】【開発提起】市場のSSDパーティション情報(MBR)が消える現象の防止策.
	//MBR書き込み禁止を通知.
	THRL_ReqWriteProtectStorageTOP( 1 );
    return ( True );

error:
	/* errorの場合、UIからHDD論理フォーマットが必要のため強制的に先行マウントOK(strsのREADY条件)とする */
    /* PRE_MOUNT は SSD しかないので */
	e_AreaBit = THRC_PartitionInfo::getDeviceFormatTargetBit( THRE_DeviceType_SSD, THRE_OPTIONS_BIT_PRE_MOUNT );
	p_HddInfo = APIC_HddInfo::newInstance();
	if( p_HddInfo != NULL ){
		e_AreaBit |= p_HddInfo->getMountAreaBit();
		p_HddInfo->setMountAreaBit( e_AreaBit );
	}
	else{
		SYSF_logMsg( "[initOSdriver] %s p_HddInfo is NULL!\n", ((init_devicetype == THRE_StorageDeviceTypeHDD) ? "HDD" : "SSD") );
	}
	return False;
}

/*================================================================================================
［機能］	doFsckAndMount.
--------------------------------------------------------------------------------------------------
［引数］	char*							：.
［戻値］	.
［詳細］	.
［ｳｫｰｸｽﾙｰ］	.
［履歴］	.
			13/02/14 Takatsuji	関数Header追加.　
            13/02/14 Takatsuji	FA14のアサート文字列のリファイン.
			14/10/29 CQ.505524 FT_yamaguchi syslog-ngを停止/開始する処理を削除.
            14/11/05 oshima   cPP暴露保護対応.
			15/10/09 中村   [SecV2-4]SecV1時の対応をマージ.
			16/02/05 中村   CQ524977[SecV2]メインストレージの暗号化設定が機能しない.暗号化設定(SSD/HDD)の判別を、init_devicetype の判別に同期する.
			18/09/21 Sky井柳 【DenebMLK】制御構成変更No78「データ移行」対応 MicroSD起動時のみ暗号化対象化修正.
			18/12/07 SCD2-Sky Kawanishi [EagleH][SecV4][RQ-674][OP_BTS-931] アンマウント時にジョブログ停止/開始する処理削除(システム上位側で実施).
			19/02/28 ads長谷川 EagleH_制御構成変更_No.49 swap有効化対応(swapファイル化対応).
			19/04/08 ads長谷川 EagleH_制御構成変更_No.49 swap有効化対応(swap有効化遅延実行対応).
			21/10/11 Sky	　 mount実施後にlost+foundの削除処理追加.
================================================================================================*/
//【695新アーキFUM】20111111 FBSC渡辺 新アーキ論理フォーマット対応 start.
/// \brief fsチェックとマウント.
Bool CP_HDDTrans::doFsckAndMount(			// DEF_SupportSSD.
	char* device_path,				///< [in] デバイスバス(HDD/SSD).
	THRE_StorageDeviceType init_devicetype,
	THRE_StorageDeviceType uc_deviceType,
	THRE_OPTIONS_BIT e_OptionsBit	///< [in] オプション指定ビット(先行マウントか否か).
	)
{
	Bool bl_fserr_fsck = False;		// fsck異常判定 True: fsck異常  False: fsck正常終了.

	SYSF_logMsg( "[%s](%d) start device_path=[%s] e_OptionsBit=[%x] \n", __FUNCTION__, __LINE__, device_path, e_OptionsBit );
#ifdef TORNADO
    int32_t ret;
    Bool encrypt = False;
	int32_t i_fscklevel = 0;
	struct stat st;
	char fpath[32];
	int32_t retry;
	part_layout*	device_info = NULL;
	THRE_DeviceType e_DeviceType = THRE_DeviceType_HDD;
// 2012/05/13 sky_kageyama CQ 364868 add.
    char cmd[512];
	// AR1012671対応 タスクモニタ自動保存との調停 .
	uchar uc_Cnt = 0;
	if ( strcmp(device_path,THRD_SSD_DEVICE) == 0 ) {
		for ( uc_Cnt = 0; ( uc_Cnt < THRD_DspTaskMoniterMaxWait ) && ( guc_TaskMoniterDsp_State == THRD_TaskMoniterDsp_Exec ); uc_Cnt++ ) {
			SYSF_taskDelay( 200 ); // 1秒Wait.
		}
		// アンマウント状態に変更.
		gb_umount_exec = True;
	}
	SYSF_logMsg("[%s] wait count %u \n", __FUNCTION__, uc_Cnt);
	if(init_devicetype == THRE_StorageDeviceTypeHDD){
		device_info = &part_info[0];
// IT6では、メインストレージの暗号化のみ.
#if 0 // サブストレージ暗号化対応の為に、コードを残しておく.
		encrypt = APIC_SystemSetting::newInstance()->getEnableEncryption();
#else //0.
		encrypt = THRG_IsSsdEncrypt();
#endif //0.
	} else {
		device_info = &ssd_part_info[0];
		e_DeviceType = THRE_DeviceType_SSD;
		encrypt = THRG_IsSsdEncrypt();
	}

	for( int32_t no = 0; device_info[no].type != PART_TYPE_END; no++ ){
		
		// OP_BTS-30791対応 単体リブート中はumountやfsckを行わない.
		LIBE_RebootStatus e_RebootStatus = LIBG_GetRebootStatus();
		if( LIBE_RebootStatus_Reboot == e_RebootStatus ){
			break;
		}
		
#if defined (DEF_64BIT_PF)
		if( ( device_info[no].fstype != PART_FSTYPE_FS ) && ( device_info[no].fstype != PART_FSTYPE_FS_SSD ) ){
#else
		if( device_info[no].fstype != PART_FSTYPE_FS ){
#endif
			continue;
		}
		// マウントの指定チェック.
		Bool b_chkTargetFlg = THRC_PartitionInfo::chkMountPntOption(device_info[ no ].mntpnt, e_OptionsBit);
		if( b_chkTargetFlg == False ){
			continue;
		}
		snprintf( fpath, sizeof(fpath), "%s/%s", device_info[no].mntpnt, THRD_FileName_NOFSCK );
		ret = SYSF_stat( fpath, &st );
		if( ret == -1 ){
			// チェックファイルが存在しないため正常にアンマウントされていない。
			i_fscklevel = 1;
		}else{
			unlink( fpath );
			i_fscklevel = 0;
		}
#if defined (DEF_64BIT_PF)	// EagleH_制御構成変更_No.49 swap有効化対応(swapファイル化対応)↓.
		if( strcmp(device_info[ no ].mntpnt, THRD_DevName_Ssd0a) == 0 ){	// ata0a判定.
			SYSC_SwapOffLock();		// swapをoffする(swap有効化遅延実行対応).
		}
#endif	// EagleH_制御構成変更_No.49 swap有効化対応(swapファイル化対応)↑.
		for( retry = 0; retry <= THRD_UMOUNT_RETRYMAX; retry++ ){
			ret = umount2( device_info[no].mntpnt, MNT_FORCE );
			if( ret == 0 ){
				break;
			}else if( SYSF_errnoGet() == EBUSY ){
				delayTask( 10+(retry*50) );//初回10msecで回を重ねるごとに+50msec追加
			}else{
				printf( "umount2 %d [%s]\n", SYSF_errnoGet(), part_info[no].mntpnt );
				ret = 0;
				break;
			}
		}
		uc_umount2_retryCount_init = (uchar)retry;
		if( ret == -1 ){
			printf( "umount2 %d [%s]\n", SYSF_errnoGet(), device_info[no].mntpnt );
// 2012/05/13 sky_kageyama CQ 364868 start.
            memset( cmd, 0, sizeof(cmd) );
			sprintf( cmd, "fuser -va -u %s", device_info[no].mntpnt );
            system( cmd );
            system( "/usr/bin/lsof" );
// 2012/05/13 sky_kageyama CQ 364868 end.
		}

#if 0	// HDD初期化完了を待たずJOB実行できる対応.
            i_fscklevel = 0;	//強制的にfsck簡易版に変更.
#endif	// HDD初期化完了を待たずJOB実行できる対応.
		//2010.05.17 soota CQ327450(S).
		if( uc_debug_ChgFsckLevel != 0 ){
			// タ-ミナルからfscklevelの変更指示があった場合、詳細版(i_fscklevel=1)で実施.
			i_fscklevel = 1;
		}
		SYSF_logMsg( "[%s](%d) i_fscklevel: %"PRId32"\n", __FUNCTION__, __LINE__, i_fscklevel );
		//2010.05.17 soota CQ327450(E).
		// 2010/05/28 takayama GaiaPF fsckを常に簡易で実行する.
		ret = doFsck( (char*)device_info[no].devpath[(uchar)encrypt], 0, device_info[no].fstype );
		if( ret ){
			// ファイルシステムに異常があり
			bl_fserr_fsck = True;
			break;
		}
	}
#endif

	// fsck結果毎の処理
	if( bl_fserr_fsck == True ){
		// fsck異常(＝ファイルシステムが復旧できないくらい壊れている？)
		// この場合、C-D010発行(フォーマットは実施しない).で終了する　
		SYSF_logMsg( "[doFsckAndMount] fsck NG\n" );
		if ((init_devicetype == THRE_StorageDeviceTypeSSD) && (uc_deviceType == THRE_StorageDeviceTypeHDDAndSSD)){
			bl_SsdNgFlg = True;
		}
		return False;
	} else {
#ifdef TORNADO
		// ファイルシステムをマウント
		ret = mountHdd( encrypt,device_path, e_OptionsBit);
		if( ret == -1 ){
			if ((init_devicetype == THRE_StorageDeviceTypeSSD) && (uc_deviceType == THRE_StorageDeviceTypeHDDAndSSD)){
				bl_SsdNgFlg = True;
			}
			SYSF_logMsg( "[%s] mountHdd Failed device_path %s\n", __FUNCTION__, device_path );
			return False;
		}
		//マウント済みのファイルシステムのlost+foundの削除.
		delLostPlusFound();
#endif
	}
	
	TYPE_MOUNT_AREA_BIT e_AreaBit = THRC_PartitionInfo::getDeviceFormatTargetBit(e_DeviceType, e_OptionsBit);
	APIC_HddInfo* p_HddInfo = APIC_HddInfo::newInstance();
	if( p_HddInfo != NULL ){ // NULLはありえない.
		e_AreaBit |= p_HddInfo->getMountAreaBit();
		p_HddInfo->setMountAreaBit(e_AreaBit);
	}
	// AR1012671対応 タスクモニタ自動保存との調停 .
	if ( strcmp(device_path,THRD_SSD_DEVICE) == 0 ) {
		gb_umount_exec = False;
	}
	SYSF_logMsg( "[%s](%d) end device_path=[%s] e_OptionsBit=[%x] \n", __FUNCTION__, __LINE__, device_path, e_OptionsBit );
	return True;
}
//【695新アーキFUM】20111111 FBSC渡辺 新アーキ論理フォーマット対応 end.


/*================================================================================================
［機能］	initNvInfoSSD
--------------------------------------------------------------------------------------------------
［引数］	無し.
［戻値］	True:成功 False:失敗.
［ｳｫｰｸｽﾙｰ］	.
［履歴］.
		2011/05/30 takayama CQ344683対応.
		2013/02/14 takatsuji 関数ヘッダー追加.
		2013/02/14 takatsuji FA14コメントリファイン(詳細コードが間違ってたため修正).
		2014/02/21 toda		 Zeus5BK SSDパーティション構成変更 8GB用テーブル追加.
		2014/07/23 iguchi    Poseidon CQ501496 eMMCパーティション構成変更.
		2015/02/13 y.morikawa CQ510398 /dev/kmsda6をFAT32からEXT3に変更(kmsda6のデータが破損したとき復旧手段がないため).
		2014/11/05 oshima   cPP暴露保護対応(選択デバイスの暗号化状態を取得するよう変更).
		2015/10/23 中村   [SecV2-4]SecV1時の対応をマージ.
    	2018/06/11 Sky井柳 [Eagle][制御構成変更No.97 パーティション構成変更対応]起動デバイスがSSDの時、ロック解除処理を実行する.
		2018/07/20 Sky松井 [Eagle][AR.1053817]ラインカード起動フラグ設定関数のコール追加.
    	2018/08/08 Sky井柳 [Eagle][制御構成変更No.99]ロック解除処理を共通化.
		2018/09/14 Sky ソリューション分離V2対応（HDD/SSDのマウント＆fsck制御).
		2018/12/07 SCD2-Sky Kawanishi [EagleH][SecV4][RQ-674][OP_BTS-931] アンマウント時にジョブログ停止/開始する処理削除(システム上位側で実施).
		2019/07/12 Sky     [RQ-560]1TB_SSD対応により、DEF_FAT_TOTAL_SIZEの切り替え処理を追加(Sim時のみ).
		2019/08/26 Sky鳥居 [EagleZ+] OP_BTS-11139対応.
		2020/12/09 Sky		OP_BTS-28730 SIMのHDDバッファサイズを実機と同等に変更.
		2021/06/16 ADS河井 管理者権限の変更処理追加.
================================================================================================*/
Bool CP_HDDTrans::initNvInfoSSD(){

	FuncCallstart( 28, __FUNCTION__, 1 );		// 割込みチェック用関数コールスタート.

	if( NULL == puc_WorkMemAddr ) {
#if defined(TORNADO) && !defined(FUM_LINUX)	// 実機で有効.
		puc_WorkMemAddr = (uchar *)(((uintptr_t)pt_ALLOC_HDDBufferArea+0x1FF) & ~0x1FF);    /* alignment honda */
		if( ( (uintptr_t)puc_WorkMemAddr % 2 ) != 0 ) {
			THRL_ThAssert( False," CP_HDDTrans::initNvInfoSSD() pt_ALLOC_HDDBufferArea Adr Not Even Error! ", 0, 0, 0, 0, CFA14_DetailCode0x0B96 );	
		}
#else	// ＰＣシミュレータで有効
		// pt_ALLOC_HDDBufferAreaに合わせてSIMでも1TBサイズで確保.
		puc_WorkMemAddr = (uchar *)malloc( THRD_HDD_WORKMEM_SIZE * THRD_HDD_WORKMEM_NUM + DEF_FAT_TOTAL_SIZE_1T );
#endif	// #if defined(TORNADO) && !defined(FUM_LINUX).
		if( NULL == puc_WorkMemAddr ) {
			THRL_ThAssert( False," CP_HDDTrans::initNvInfoSSD() puc_WorkMemAddr Alloc Error! ", 0, 0, 0, 0, CFA14_DetailCode0x0B97 );	
		}
	}

	//	ロックを解除.
	Bool bl_Res = theHDDTrans.unlockStorage();
	if ( False == bl_Res ) {
		SYSF_logMsg( "[%s] Unlock ERR\n", __FUNCTION__);
		FuncCallend( 28, __FUNCTION__, 1 );			// 割込みチェック用関数コールエンド.
		return  False;
	}

	int32_t i_DviceId = e_StartUpDeviceType == THRE_StartupDeviceType_SD ? THRE_SsdPortNo : THRE_HddPortNo;
//	uint32_t ul_HddState = THRF_sysBspHddDeviceInfoGet( 0, 0, (ST_HDD_DEVICE_INFO *)theHDDTrans.puc_WorkMemAddr);
	uint32_t ul_HddState = THRF_sysBspHddDeviceInfoGet( i_DviceId, 0, (ST_HDD_DEVICE_INFO *)theHDDTrans.puc_WorkMemAddr);
	convertAndSetHDDStatus( ul_HddState );
//	ushort us_result = chkSaveIdentifyDevice( 0, (ST_HDD_DEVICE_INFO *)theHDDTrans.puc_WorkMemAddr );
	ushort us_result = chkSaveIdentifyDevice( i_DviceId, (ST_HDD_DEVICE_INFO *)theHDDTrans.puc_WorkMemAddr );
	if(us_result != HDD_STATUS_OK) {
		SYSF_logMsg("initOSdriverSSD : chkSaveIdentifyDevice NG.\n");
	}
	FuncCallend( 28, __FUNCTION__, 1 );			// 割込みチェック用関数コールエンド.

	part_layout*	device_info = NULL;
	int32_t no = 0;
	int32_t ret = 0;

	// 2014/07/23 iguchi CQ501496 eMMCパーティション構成変更.
	device_info = THRG_GetPartitionType();

	// AR1012671対応 タスクモニタ自動保存との調停 .
	uchar uc_Cnt = 0;
	for ( uc_Cnt = 0; ( uc_Cnt < THRD_DspTaskMoniterMaxWait ) && ( guc_TaskMoniterDsp_State == THRD_TaskMoniterDsp_Exec ); uc_Cnt++ ) {
		SYSF_taskDelay( 200 ); // 1秒Wait.
	}
	// アンマウント状態に変更.
	gb_umount_exec = True;
	SYSF_logMsg("[%s] wait count %u \n", __FUNCTION__, uc_Cnt);

	// /sdirはRFSで自動マウントされているため、doFsckを実行する前にunmoutする.
	// もしumountに失敗したときはdoFsckでTrouble(8)が発生する.
	umount2( THRD_DevName_Ssd_SDIR, MNT_FORCE );

	uchar uc_encrypt = THRG_IsSsdEncrypt() ? 1 : 0;

#ifdef TORNADO	
	
	// 「一般ユーザーアクセス変換管理ファイル」を読み込む.
	std::map<std::string, Bool> tmpctrlFileMap;
	Bool bl_Ret = readMfpAccessCtrlFile( tmpctrlFileMap );
	if( bl_Ret == False ){
		printf( "[%s:%d] readMfpAccessCtrlFile NG.\n", __PRETTY_FUNCTION__, __LINE__ );
		return False;
	}
	
	for( no = 0; device_info[no].type != PART_TYPE_END; no++ ){
		// AR1059900対応 PART_FSTYPE_FS_NFの判定削除 AR1059900の問題と直接関係はないが、見直しにより合わせて修正.

// 2018/10/12 Sky新大阪 [AR.1061634]【制御_No.99】/ata07と/ata0eのfstypeを変更(NF2 -> FS,FS_SSD) ->.
        // 優先マウント条件をNF2、NF3に加え、THRE_OPTIONS_BIT_PRE_MOUNT指定のもかどうかをチェックする.
        if( ( THRC_PartitionInfo::chkMountPntOption( device_info[ no ].mntpnt, THRE_OPTIONS_BIT_PRE_MOUNT ) != True )
         && ( device_info[no].fstype != PART_FSTYPE_FS_NF2 )
         && ( device_info[no].fstype != PART_FSTYPE_FS_NF3 ) )
        {
            continue;
        }
// 2018/10/12 Sky新大阪 [AR.1061634]【制御_No.99】/ata07と/ata0eのfstypeを変更(NF2 -> FS,FS_SSD) <-.

// 2015/04/13 ADS斉藤 CQ.511956 Simにてカード認証機能が有効化時にC180発生.
#ifdef FUM_LINUX
        // SIMの場合、/kmはシンボリックリンクで動作するため、マウント対象外.
        if (strncmp(device_info[no].mntpnt, "/km", 3) == 0) {
            continue;
        }
#endif  // FUM_LINUX

		if( THRG_IsMountPoint(device_info[ no ].mntpnt) )
		{
			printf ( "Step2-Apply: %s %d\n", __func__, __LINE__ );
			continue;				// 単体リブート時はマウントもfsckもスルーさせる
		}

		// AR1059900_G3Fax送受信以外のジョブ実行後で、ジョブ履歴なし問題対応.
		// PART_FSTYPE_FS_NFの判定→マウントポイントの判定に変更.
    	if ( ( NVDF_checkNvramClearFlag() == True ) && 								// 不揮発初期化されている
    	   ( ( strncmp( THRD_DevName_NVDData, device_info[no].mntpnt, sizeof(THRD_DevName_NVDData) ) == 0 ) || 		// マウントポイントがata0bかata02(=NV領域).
    	   	 ( strncmp( THRD_DevName_ata02, device_info[no].mntpnt, sizeof(THRD_DevName_ata02) ) == 0   ) ) ){
			//NV領域をフォーマットする.
            ret = formatFS( (char*)device_info[no].devpath[uc_encrypt] );
            if( ret == -1 ){
                SYSF_logMsg( "ata0b format ERR [%s]\n", (char*)device_info[no].devpath[uc_encrypt] );
            }
//Str【OP_BTS-22519対応 : 2020.05.22】.
#if defined(DEF_Helios_PF) && !defined(FUM_LINUX)
           if( strncmp( THRD_DevName_NVDData, device_info[no].mntpnt, sizeof(THRD_DevName_NVDData) ) == 0 ) {
		   		printf(">>>MicroSD EraseNvramBackupArea Clear!!\n");
				EraseNvramBackupArea(0, 0);
				EraseNvramBackupArea(0, 1);
				EraseNvramBackupArea(0, 2);	// バックアップエリア削除(グループ3).
			}
#endif	// defined(DEF_Helios_PF) && !defined(FUM_LINUX)
//End【OP_BTS-22519対応 : 2020.05.22】.
        } else {
            ret = doFsck( (char*)device_info[no].devpath[uc_encrypt], 0, device_info[no].fstype );
            if ( ret ){
                SYSF_logMsg( "doFsck [%s] fstype:%"PRId32" ERR\n", (char*)device_info[no].devpath[uc_encrypt], device_info[no].fstype );
                e_NvramInfoAreaState = TYPE_HddStatusNonFormat;
                return False;
            }
            SYSF_logMsg( "fsck [%s] fstype:%"PRId32" OK\n", (char*)device_info[no].devpath[uc_encrypt], device_info[no].fstype );
        }
		// AR1059900対応 PART_FSTYPE_FS_NFの判定削除 AR1059900の問題と直接関係はないが、見直しにより合わせて修正.
        if( device_info[no].fstype == PART_FSTYPE_FS_NF3 ){
            ret = mount( device_info[ no ].devpath[uc_encrypt], device_info[ no ].mntpnt, "ext3", MS_NOATIME, EXT3_MOUNT_OPTIONS );
        }else{
            ret = mount( device_info[ no ].devpath[uc_encrypt], device_info[ no ].mntpnt, "ext2", MS_NOATIME, NULL );
        }
        //マウント失敗.
        if( ret == -1 ){
            SYSF_logMsg( "[%s] %s -> %s errno:%d\n", __FUNCTION__, device_info[ no ].devpath[uc_encrypt], device_info[ no ].mntpnt, SYSF_errnoGet() );
            e_NvramInfoAreaState = TYPE_HddStatusNonFormat;
            return False;
        }
		// 該当マウントポイントとディレクトリ配下の権限を変更する.
		if( tmpctrlFileMap.find( device_info[no].mntpnt ) != tmpctrlFileMap.end() ){		//tmpctrlFileMapのmntpnt存在確認.
			if( tmpctrlFileMap[device_info[no].mntpnt] == False ){
				int32_t ChangeAuthorityresult = 0;
				ChangeAuthorityresult = ChangeAuthority( device_info[no].mntpnt, tmpctrlFileMap );
				if(ChangeAuthorityresult == -1){
					printf( "[%s:%d] ChangeAuthority NG.\n", __PRETTY_FUNCTION__, __LINE__ );
					return False;
				}
			}
		}
    }
	
	// 「一般ユーザーアクセス変換管理ファイル」を更新する.
	Bool writeResult = writeMfpAccessCtrlFile( tmpctrlFileMap );
	if( writeResult == False ){
		printf( "initNvInfoSSD writeMfpUserCtrlFile NG\n" );
		return False;
    }

	// HddInfoにマウント領域を設定.
	TYPE_MOUNT_AREA_BIT e_AreaBit = THRC_PartitionInfo::getDeviceFormatTargetBit( THRE_DeviceType_SSD, THRE_OPTIONS_BIT_PRE_MOUNT );
	APIC_HddInfo* p_HddInfo = APIC_HddInfo::newInstance();
	if( p_HddInfo != NULL ){ // NULLはありえない.
		e_AreaBit |= p_HddInfo->getMountAreaBit();
		p_HddInfo->setMountAreaBit(e_AreaBit);
	}

#endif
	// AR1012671対応 タスクモニタ自動保存との調停 .
	gb_umount_exec = False;
	// ラインカード起動時にはブラウザを起動させないようにする為、.
	// 優先パーティションのマウント完了のステータスをセットするより前に.
	// ラインカード起動フラグ設定関数をコール.
	checkLineModeInTar();
	e_NvramInfoAreaState = TYPE_HddStatusFormated;
	return True;

}

// 2010/12/20 MinervaZeus対応 河村 - NVRAM512KB対応.
// 2011/05/30 takayama CQ344683対応.
void initOSdriverSSD()
{
	register Bool bl_result = theHDDTrans.initNvInfoSSD();
	if( bl_result == False ) {
		SYSF_logMsg("initSSD : init error.\n");
	}
}

#else	// DEF_SupportSSD.


Bool CP_HDDTrans::initOSdriver()
{
	uchar uc_stat;
	TYPE_MOUNT_AREA_BIT e_AreaBit;
	APIC_HddInfo *p_HddInfo = NULL;
	Bool bl_ret = False;
#ifdef TORNADO
    int32_t ret;
    Bool encrypt = APIC_SystemSetting::newInstance()->getEnableEncryption();

    // 実装HDDの情報を元にパーティションレイアウトテーブルを構築
    ret = calcAndFixedPartInfo( THRD_HDD_DEVICE );
    if( ret == -1 ){
        THRL_ThAssert( False, "[SATA2] initOSdriver() calcAndFixedPartInfo ERR\n" , 0, 0, 0, 0, CFA14_DetailCode0x01EC );
    }

    // 実装HDDのパーティション構成をチェック
    ret = checkPartitionLayout( THRD_HDD_DEVICE );
    if( ret == -1 ){
        SYSF_logMsg( "partition size\t[ NG ]\n" );
        // パーティション作成
        ret = doFdisk( THRD_HDD_DEVICE );
        if( ret == -1 ){
            THRL_ThAssert( False, "[SATA2] initOSdriver() doFdisk ERR\n" , 0, 0, 0, 0, CFA14_DetailCode0x01ED );
        }
    }
    SYSF_logMsg( "partition size\t[ OK ]\n" );

	// デバイスファイル作成
	ret = makeDeviceFiles();
	if( ret == -1 ){
		THRL_ThAssert( False, "[SATA2] initOSdriver() makeDeviceFiles ERR\n" , 0, 0, 0, 0, CFA14_DetailCode0x01EE );
	}

    // dm-crypt設定
    if( encrypt ){
        ret = setupDmcrypt();
        if( ret == -1 ){
            THRL_ThAssert( False, "[SATA2] initOSdriver() setupDmcrypt ERR\n" , 0, 0, 0, 0, CFA14_DetailCode0x01EF );
        }
    }

    // ファイルシステムをマウント
//【695新アーキFUM】20111111 FBSC渡辺 新アーキ論理フォーマット対応 start.
//	ret = mountHdd( encrypt );
	ret = mountHdd( encrypt, THRE_OPTIONS_BIT_NOT_CARE_PRIORITY );
//【695新アーキFUM】20111111 FBSC渡辺 新アーキ論理フォーマット対応 end.
    if( ret == -1 ){
        SYSF_logMsg( "[%s] mountHdd Failed\n", __FUNCTION__ );
        goto error;
    }
#endif	/* #ifdef TORNADO */

	// フォーマット中フラグセット時はchkdskを実行しない 2004/09/30 added by t.sasaki
	uc_stat = (uchar)NVDC_SystemSetting::newInstance()->getFsStat();
	THRL_ThLogMsgErr( "@@@@@ FsStat=[0x%02X] @@@@@\n",uc_stat );
	if ( (uc_stat & TYPD_SystemFsStatFormatting) != 0 ) {
        SYSF_logMsg( "[initOSdriver] getFsStat() %02x TYPD_SystemFsStatFormatting\n", uc_stat );
		goto error;
	}

	// 本当にフォーマットされていないかはチェックファイルの有無で判定.
	bl_ExistCheckFile = chkExistOSdriver() ;// チェックファイル有無判定.
	if( bl_ExistCheckFile == False ){
        SYSF_logMsg( "[initOSdriver] bl_ExistCheckFile: %d\n", bl_ExistCheckFile );
		goto error;
	}
	bl_ret = doFsckAndMount(THRE_OPTIONS_BIT_NORMAL_MOUNT);
	if( bl_ret == False ){
		SYSF_logMsg( "[initOSdriver] doFsckAndMount Nomal NG\n" );
		return False;
	}
	if( chkReadOSdriver() == False ){
		SYSF_logMsg( "[initOSdriver] chkReadOSdriver() NG\n" );
		return False;					// IJCフォーマットチェック用ファイル読み込み失敗	
	}

	printf( "$$initOSdriver OK\n" );
    return ( True );

error:
	/* errorの場合、UIからHDD論理フォーマットが必要のため強制的に先行マウントOK(strsのREADY条件)とする */
	e_AreaBit = THRC_PartitionInfo::getDeviceFormatTargetBit( THRE_DeviceType_HDD, THRE_OPTIONS_BIT_PRE_MOUNT );
	p_HddInfo = APIC_HddInfo::newInstance();
	if( p_HddInfo != NULL ){
		e_AreaBit |= p_HddInfo->getMountAreaBit();
		p_HddInfo->setMountAreaBit( e_AreaBit );
	}
	else{
		SYSF_logMsg( "[initOSdriver] p_HddInfo is NULL!\n" );
	}
	return False;
}

//【695新アーキFUM】20111111 FBSC渡辺 新アーキ論理フォーマット対応 start.
/// \brief fsチェックとマウント.
Bool CP_HDDTrans::doFsckAndMount(
	THRE_OPTIONS_BIT e_OptionsBit	///< [in] オプション指定ビット(先行マウントか否か).
	)
{
//【695新アーキFUM】20120126 FBSC鈴木 SimWin.exe ビルドエラー対応.
#if defined(TORNADO )	// not compile when building SimWin.exe
	SYSF_logMsg( "[%s](%d) start  e_OptionsBit=[%x] \n", __FUNCTION__, __LINE__, e_OptionsBit );
#endif	// not compile when building SimWin.exe
	// 各HDDパーティションのfsck実行.
	// fsck NGを検出した場合には全てのHDDパーティションをフォーマットする.
	Bool bl_fserr_fsck = False;		// fsck異常判定 True: fsck異常  False: fsck正常終了.
#ifdef TORNADO
	int32_t encrypt = APIC_SystemSetting::newInstance()->getEnableEncryption();
	int32_t i_fscklevel = 0;
	struct stat st;
	char fpath[32];
	int32_t retry;
// 2012/05/13 sky_kageyama CQ 364868 add.
    char cmd[512];
	for( int32_t no = 0; part_info[no].type != PART_TYPE_END; no++ ){
		if( part_info[ no ].mntpnt[0] == 0 ){
			continue;
		}
		// マウントの指定チェック.
		Bool b_chkTargetFlg = THRC_PartitionInfo::chkMountPntOption(part_info[ no ].mntpnt, e_OptionsBit);
		if( b_chkTargetFlg == False ){
			continue;
		}
		snprintf( fpath, sizeof(fpath), "%s/%s", part_info[no].mntpnt, THRD_FileName_NOFSCK );
		int32_t ret = SYSF_stat( fpath, &st );
		if( ret == -1 ){
			// チェックファイルが存在しないため正常にアンマウントされていない。
			i_fscklevel = 1;
		}else{
			unlink( fpath );
			i_fscklevel = 0;
		}

		for( retry = 0; retry <= THRD_UMOUNT_RETRYMAX; retry++ ){
			ret = umount2( part_info[no].mntpnt, MNT_FORCE );
			if( ret == 0 ){
				break;
			}else if( SYSF_errnoGet() == EBUSY ){
				delayTask( 10+(retry*50) );//初回10msecで回を重ねるごとに+50msec追加
			}else{
				printf( "umount2 %d [%s]\n", SYSF_errnoGet(), part_info[no].mntpnt );
				ret = 0;
				break;
			}
		}
		uc_umount2_retryCount_init = (uchar)retry;
		if( ret == -1 ){
			printf( "umount2 %d [%s]\n", SYSF_errnoGet(), part_info[no].mntpnt );
// 2012/05/13 sky_kageyama CQ 364868 start.
            memset( cmd, 0, sizeof(cmd) );
			sprintf( cmd, "fuser -va -u %s", device_info[no].mntpnt );
            system( cmd );
            system( "/usr/bin/lsof" );
// 2012/05/13 sky_kageyama CQ 364868 end.
		}

#if 0	// HDD初期化完了を待たずJOB実行できる対応.
		i_fscklevel = 0;	//強制的にfsck簡易版に変更.
#endif	// HDD初期化完了を待たずJOB実行できる対応.
//2010.05.17 soota CQ327450(S).
		if( uc_debug_ChgFsckLevel != 0 ){
			// タ-ミナルからfscklevelの変更指示があった場合、詳細版(i_fscklevel=1)で実施.
			i_fscklevel = 1;
		}
		SYSF_logMsg( "[%s](%d) i_fscklevel: %"PRId32"\n", __FUNCTION__, __LINE__, i_fscklevel );
//2010.05.17 soota CQ327450(E).
		ret = doFsck( (char*)part_info[no].devpath[(uchar)encrypt], i_fscklevel );
		if( ret ){
			// ファイルシステムに異常があり
			bl_fserr_fsck = True;
			break;
		}
	}
#endif	/* #ifdef TORNADO */

	// fsck結果毎の処理
	if( bl_fserr_fsck == True ){
		// fsck異常(＝ファイルシステムが復旧できないくらい壊れている？)
		// この場合、C-D010発行(フォーマットは実施しない).で終了する　
		SYSF_logMsg( "[doFsckAndMount] fsck NG\n" );
		return False;
	} else {
#ifdef TORNADO
		// ファイルシステムをマウント
		int32_t ret;
		ret = mountHdd( encrypt, e_OptionsBit );
		if( ret == -1 ){
			SYSF_logMsg( "[%s] mountHdd Failed\n", __FUNCTION__ );
			return False;
		}
#endif	/* #ifdef TORNADO */
	}
	TYPE_MOUNT_AREA_BIT e_AreaBit = THRC_PartitionInfo::getDeviceFormatTargetBit(THRE_DeviceType_HDD, e_OptionsBit);
	APIC_HddInfo* p_HddInfo = APIC_HddInfo::newInstance();
	if( p_HddInfo != NULL ){ // NULLはありえない.
		e_AreaBit |= p_HddInfo->getMountAreaBit();
		p_HddInfo->setMountAreaBit(e_AreaBit);
	}
	SYSF_logMsg( "[doFsckAndMount()](%d) end e_OptionsBit=[%x] \n", __LINE__, e_OptionsBit );
	return True;
}
//【695新アーキFUM】20111111 FBSC渡辺 新アーキ論理フォーマット対応 end.


#endif

#if 1	/* 新SIM: 論理フォーマットの暫定対応 */
extern void logicformatHDD();
extern "C" {
//【695新アーキFUM】20111111 FBSC渡辺 新アーキ論理フォーマット対応 start.
//	void FORMATHDD( void ){
	void FORMATHDD( ushort bit ){
		printf( "@@@@@ exec formatOSdriver start\n" );
		if( bit == 0 ){
			theHDDTrans.m_usFormatTargetBit |= THRE_FORMATBIT_TERMINAL;
		}else{
			theHDDTrans.m_usFormatTargetBit = (ushort)(theHDDTrans.m_usFormatTargetBit | bit); // CE:2 admin:4
		}
//【695新アーキFUM】20111111 FBSC渡辺 新アーキ論理フォーマット対応 end.
		logicformatHDD();	// 論理フォーマット実行
		printf( "@@@@@ exec formatOSdriver end\n" );
	}
}
#endif	// DEF_SupportSSD.

/*================================================================================================
［機能］	【内部】OSのフォーマット
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	True:成功、False：失敗
［詳細］	dosFs2.0のフォーマットを行う
 [履歴】	02/01/30 morikawa	メモリチェックファンクションによるフォーマット実行に伴い初期化ルーチンから分離 
 			02/02/07 中田		IJCfopenのオープンモードに(void*)を付加（低速ＦＵＭコンパイルエラー対策）
 			02/02/07 中田		dosFsVolFormatのフォルダ文字列指定に(void*)を付加（低速ＦＵＭコンパイルエラー対策）
 			02/05/28 morikawa	MIOとAPP0は立ち上げ時に毎回ファイル初期化を行うので、ここではフォーマットしないように修正。
 			02/06/26 morikawa	VxWorksのシステムコールのフォーマット実行後にすぐdosFSのアクセスにいくとなぜか
 							memPartRealloc: invalid block 0x2d4b3d8 in partition 0x91c904.となり12CCとなるので
 							アクセス前に1secの遅延を入れる。
 							さらに、タスクディレイの前後でセマフォを一時的に開放する。
			04/01/30 morikawa Rhein1 初版　FLAT_FILE対応　
			04/02/02 morikawa "/ata01"は毎回初期化しないので、フォーマット時のみにフォーマットを行うように修正　　
			04/06/09 ikawa	不要な処理を削除
			04/08/23 morikawa TSS2のロングネームで読み書き不具合の修正をRhein1にも反映
								Formatのパラメータを
									MSFT Long Names(VFAT)からWRS Long Names(VxLong)に修正
			04/09/13 t.sasaki 起動時のC-5023(メッセージキューフロー)対策－起動時のdosFSアクセスを極力減らす.
			05/06/13 t.sasaki グローバルセマフォ解放＆再確保処理の変更(今までの実装は解放できていない).
			07/05/14 morikawa 静的解析不具合修正　
			07/07/11 morikawa Donau dosFSのパーティション見直し、実機とSimでの処理を共通化
				【課題】
					・THRD_DevName_Hdd01は現状DOS_OPT_VXLONGNAMESなので、これをなしに変更予定
					・THRD_DevName_Hdd03は現状/a0/app1であるが/ata03に定義変更予定
					・THRD_DevName_Hdd04は将来的にここではフォーマットせずに、別処理でフォーマットするようにする。
			07/10/04 yokoi Simの時も、ijcフォルダは作成するように処理を追加 
			07/11/05 morikawa Donau先行開発検証のため
					全てのパーティションでDOS_OPT_VXLONGNAMESをとりVFAT形式とする　		
					さらに、THRD_DevName_Hdd04の音声動画領域に関しては、デフォルトでフォーマットしない。
					debug_format_ata04が0でないときのみフォーマットする	
			07/11/05 morikawa HRD_DevName_Hdd04の音声動画領域に関しては、デフォルトでフォーマットするにする。
							こうしないと、デフォルトの状態でフォーマットしても未フォーマットのエラーが発生するため。
			08/02/13 morikawa Donau機種対応 パーティション追加
			08/05/15 kurachi Donau CQ:302954対策.
									フォーマットチェック用ファイルchk_ijc.dataに書き込むデータを変更.
									暗号化無効/暗号化有効（暗号化優先）/暗号化有効（上書き優先）、どのモードで
									フォーマットしたかという情報を残しておく.
			08/06/19 kurachi Donau CQ:304895対策.
									ata04（音声・動画管理領域）は、フォーマット切り替えを行なうようにする.
									case サービスからのフォーマット：クリアする.
									case 管理者からのフォーマット：クリアしない.
									case 暗号化設定変更時のフォーマット：クリアする.
            09/04/15 koshimo Taiga CQ:316762 SSD標準化対応（SSD/HDD dosFsマッピング変更）.
			09/05/19 koshimo Taiga CQ:317056 SSD構成におけるUSBダイレクトプリント対応 HDDレス時のフォーマットはata0a領域のみとする.
            09/08/02 takayama Taiga CQ:318248  SSDステータス別管理対応.
			10/05/17 nakagawa CQ:327450 アンマウント失敗時にリトライ処理を行う.
            11/04/06 河村 CQ:338349 chdir("/ata01")(HDD)にしているため省電力時にHDDがアンマウント出来ない問題を修正
			11/11/11 FBSC渡辺 新アーキ論理フォーマット対応(管理者からのフォーマット時 ata04 のみ判断 → テーブルのビットにて判断)
　　　　　　12/01/20 takayama CQ360543 FD残る問題対策 ファイルアクセスAPIをラッパー関数に置き換え.
            13/02/14 Takatsuji	FA14のアサート文字列のリファイン.
            14/10/08 yamada	CQ503146 PSWC言語領域不足対応.
			14/10/29 CQ.505524 FT_yamaguchi syslog-ngを停止/開始する処理を削除.
			14/11/06 simogaki CQ.506310対応.
            17/10/10 Yoneda    S800 ストレージ構成変更対応.
            14/11/05 oshima   cPP暴露保護対応(選択デバイスの暗号化状態を取得するよう変更).
			15/10/09 中村   [SecV2-4]SecV1時の対応をマージ.
			16/02/05 中村   CQ524977[SecV2]メインストレージの暗号化設定が機能しない.暗号化設定(SSD/HDD)の判別を、uc_dviceTypeの判別に同期する.
			18/03/22 井柳 [Eagle]制御構成変更No97「ストレージ装着設定対応」SSDパターンチェックデータ書込みの実行条件の変更（SSD(μSD)のみ→条件なし(μSD及びSSD)）
			18/12/07 SCD2-Sky Kawanishi [EagleH][SecV4][RQ-674][OP_BTS-931] アンマウント時にジョブログ停止/開始する処理削除(システム上位側で実施).
			19/02/28 ads長谷川 EagleH_制御構成変更_No.49 swap有効化対応(swapファイル化対応).
			19/04/08 ads長谷川 EagleH_制御構成変更_No.49 swap有効化対応(swap有効化遅延実行対応).
            2020/12/14 Tsukiji OP_BTS-29664 FWエラーが発生する問題対応(暫定)
			21/06/16 ads河井   該当マウントポイントの変換済フラグを未変換へ変更処理追加.
================================================================================================*/
// ※ ※ ※ ※ ※ ※ ※ ※ ※ ※ ※ ※ ※ ※ ※ ※ ※ ※ ※ ※  //
// ※ 本処理呼び出し時にはグローバルセマフォを解放すること！	//
// ※ ※ ※ ※ ※ ※ ※ ※ ※ ※ ※ ※ ※ ※ ※ ※ ※ ※ ※ ※  //
#ifdef DEF_SupportSSD
Bool CP_HDDTrans::formatOSdriver()		// DEF_SupportSSD.
{
	THRE_StorageDeviceType uc_deviceType = getNvRamDeviceType();
#ifdef TORNADO
    int32_t no;
    int32_t ret;
    part_layout*	device_info;
    int32_t retry;

 // 2012/05/13 sky_kageyama CQ 364868 add.
   char cmd[512];
	uchar uc_Cnt=0;
    Bool encrypt = False;

    if(( uc_deviceType == THRE_StorageDeviceTypeHDDAndSSD ) ||
       ( theHDDTrans.e_StartUpDeviceType == THRE_StartupDeviceType_SSD )) {
		device_info = &part_info[0];
// IT6では、メインストレージの暗号化のみ.
#if 0 // サブストレージ暗号化対応の為に、コードを残しておく.
		encrypt = APIC_SystemSetting::newInstance()->getEnableEncryption() ? 1 : 0;
#else //0.
		encrypt = THRG_IsSsdEncrypt() ? 1 : 0;
#endif //0.
	} else {
		device_info = &ssd_part_info[0];
		encrypt = THRG_IsSsdEncrypt() ? 1 : 0;
	}
	// AR1012671対応 タスクモニタ自動保存との調停 .
    if(( uc_deviceType != THRE_StorageDeviceTypeHDDAndSSD ) &&
       ( theHDDTrans.e_StartUpDeviceType == THRE_StartupDeviceType_SD )) {
		for ( uc_Cnt = 0; ( uc_Cnt < THRD_DspTaskMoniterMaxWait ) && ( guc_TaskMoniterDsp_State == THRD_TaskMoniterDsp_Exec ); uc_Cnt++ ) {
			SYSF_taskDelay( 200 ); // 1秒Wait.
		}
		// アンマウント状態に変更.
		gb_umount_exec = True;
	}
	SYSF_logMsg("[%s] wait count %u \n", __FUNCTION__, uc_Cnt);
	
	// 「一般ユーザーアクセス変換管理ファイル」を読み込む.
	std::map<std::string, Bool> tmpctrlFileMap;
	if( False == readMfpAccessCtrlFile( tmpctrlFileMap ) ) {
		SYSF_logMsg( "[%s:%d] readMfpAccessCtrlFile NG.\n", __PRETTY_FUNCTION__, __LINE__ );
		return False;
	}
	
    for( no = 0; device_info[no].type != PART_TYPE_END; no++ ){
#if defined (DEF_64BIT_PF)
        if( ( device_info[no].fstype != PART_FSTYPE_FS ) && ( device_info[no].fstype != PART_FSTYPE_FS_SSD ) ){
#else
        if( device_info[no].fstype != PART_FSTYPE_FS ){
#endif
            continue;
        }
        if( chdir( "/km/fw" ) == ERROR ){
            THRL_ThLogMsgErr("chdir error!!\n" );
        }
#ifdef FUM_LINUX
		if( m_usFormatTargetBit & THRE_FORMATBIT_TERMINAL ){
			if( device_info[ no ].mntpnt[0] == 0 ){
				ret = formatFS( (char*)device_info[no].devpath[(uchar)encrypt] );
				if( ret == -1 ){
					SYSF_logMsg( "[SATA2] formatOSdriver ERR [%s]\n", (char*)device_info[no].devpath[(uchar)encrypt] );
					return False;
				}
				continue;
			}
		}
#endif	/* #ifdef FUM_LINUX */

		// マウントポイントのフォーマットオプションをチェック.
		Bool b_formatFlg = THRC_PartitionInfo::chkFormatTarget(device_info[no].mntpnt, m_usFormatTargetBit);
		if( b_formatFlg == False ){
			SYSF_logMsg( "\nFormat No Exec %s debug_format=%s\n\n", device_info[no].mntpnt, device_info[no].devpath[(uchar)encrypt] );
			continue;
		}

#if defined (DEF_64BIT_PF)	// EagleH_制御構成変更_No.49 swap有効化対応(swapファイル化対応)↓.
		if( strcmp(device_info[ no ].mntpnt, THRD_DevName_Ssd0a) == 0 ){	// ata0a判定.
			SYSC_SwapOffLock();		// swapをOFFする(swap有効化遅延実行対応).
		}
#endif		// EagleH_制御構成変更_No.49 swap有効化対応(swapファイル化対応)↑.
        printf( "[%s] unmount [%s]\n", __PRETTY_FUNCTION__, device_info[no].mntpnt );
        for( retry = 0; retry <= THRD_UMOUNT_RETRYMAX; retry++ ){
        	ret = umount2( device_info[no].mntpnt, MNT_FORCE );
        	if( ret == 0 ){
                break;
			}else if( SYSF_errnoGet() == EBUSY ){
				delayTask( 10+(retry*50) );//初回10msecで回を重ねるごとに+50msec追加
			}else{
				printf( "umount2 %d [%s]\n", SYSF_errnoGet(), part_info[no].mntpnt );
				ret = 0;
				break;
			}
        }
        uc_umount2_retryCount_format = (uchar)retry;
        if( ret == -1 ){
            printf( "umount2 %d [%s]\n", SYSF_errnoGet(), device_info[no].mntpnt );
// 2012/05/13 sky_kageyama CQ 364868 start.
            memset( cmd, 0, sizeof(cmd) );
			sprintf( cmd, "fuser -va -u %s", device_info[no].mntpnt );
            system( cmd );
            system( "/usr/bin/lsof" );
// 2012/05/13 sky_kageyama CQ 364868 end.
        }

        printf( "[%s] format  [%s]\n", __PRETTY_FUNCTION__, device_info[no].devpath[(uchar)encrypt] );
        ret = formatFS( (char*)device_info[no].devpath[(uchar)encrypt] );
        if( ret == -1 ){
            SYSF_logMsg( "[SATA2] formatOSdriver ERR [%s]\n", (char*)device_info[no].devpath[(uchar)encrypt] );
        	// AR1012671対応 mountしてないので、gb_umount_execは更新しない.
            return False;
        }

        // フォーマットしたマウントポイントと一致するデータの変換済フラグを未変換に変更する.
        std::map<std::string, Bool>::iterator itrCtrlFile = tmpctrlFileMap.find( device_info[no].mntpnt );
        if( itrCtrlFile != tmpctrlFileMap.end() ){
        	itrCtrlFile->second = False;
        }

        // ファイルシステムをマウント
        // 2010/12/27 MinervaZeus対応 河村 - NVRAM削減対応.
        printf( "[%s] mount   [%s][%s]\n", __PRETTY_FUNCTION__, device_info[ no ].devpath[(uchar)encrypt], device_info[ no ].mntpnt );

        if(( uc_deviceType == THRE_StorageDeviceTypeHDDAndSSD ) ||
           ( theHDDTrans.e_StartUpDeviceType == THRE_StartupDeviceType_SSD )) {
            // HDD.
#if defined (DEF_64BIT_PF)
			if(device_info[ no ].fstype != PART_FSTYPE_FS_SSD)
			{
	            ret = mount( device_info[ no ].devpath[(uchar)encrypt], device_info[ no ].mntpnt, "ext3", MS_NOATIME, EXT3_MOUNT_OPTIONS );
			}else{
            	ret = mount( device_info[ no ].devpath[(uchar)encrypt], device_info[ no ].mntpnt, "ext2", MS_NOATIME, NULL );
			}
#else
            ret = mount( device_info[ no ].devpath[(uchar)encrypt], device_info[ no ].mntpnt, "ext3", MS_NOATIME, EXT3_MOUNT_OPTIONS );
#endif
        } else {
            // eMMC.
            ret = mount( device_info[ no ].devpath[(uchar)encrypt], device_info[ no ].mntpnt, "ext2", MS_NOATIME, NULL );
        }
        if( ret == -1 ){
        	// AR1012671対応 mount失敗なので、gb_umount_execは更新しない.
            printf( "[%s] %s -> %s errno:%d\n", __FUNCTION__, device_info[ no ].devpath[(uchar)encrypt], device_info[ no ].mntpnt, SYSF_errnoGet() );
            return False;
        }
#if defined (DEF_64BIT_PF)	// EagleH_制御構成変更_No.49 swap有効化対応(swapファイル化対応)↓.
		if( strcmp(device_info[ no ].mntpnt, THRD_DevName_Ssd0a) == 0 ){	// ata0a判定.
			SYSC_SwapOnUnlock();		// swapをONする(swap有効化遅延実行対応).
		}
#endif		// EagleH_制御構成変更_No.49 swap有効化対応(swapファイル化対応)↑.
//【695新アーキFUM】20111111 FBSC渡辺 新アーキ論理フォーマット対応 start.
		ret = createDirSystemNeed(device_info[ no ].mntpnt);
		if( ret == -1 ){
        	// AR1012671対応 mount失敗なので、gb_umount_execは更新しない.
			SYSF_logMsg( "[%s] mkdir error %s -> %s errno:%d\n", __FUNCTION__, device_info[ no ].devpath[(uchar)encrypt], device_info[ no ].mntpnt, SYSF_errnoGet() );
			return False;
		}
//【695新アーキFUM】20111111 FBSC渡辺 新アーキ論理フォーマット対応 start.
        
		// マウントポイントが「一般ユーザーアクセス変換管理ファイル」で変換対象の場合権限変換実施.
		if(tmpctrlFileMap.end() != itrCtrlFile) {
			if(-1 == ChangeAuthority( device_info[ no ].mntpnt, tmpctrlFileMap )) {
				SYSF_logMsg( "[%s:%d] ChangeAuthority(%s) NG.\n", __PRETTY_FUNCTION__, __LINE__, device_info[ no ].mntpnt );
				return False;
			}
		}
    }

	// 「一般ユーザーアクセス変換管理ファイル」を更新する.
	Bool writeResult = False;
    writeResult = writeMfpAccessCtrlFile(tmpctrlFileMap);
	if( writeResult == False ){
		SYSF_logMsg( "[%s:%d] writeMfpUserCtrlFile NG.\n", __PRETTY_FUNCTION__, __LINE__ );
		return False;
	}

	delayTask( 1000 );
	
	// 2011/04/06 CQ:338349 河村 - chdirをしない制御に変更.
//    if(( uc_deviceType == THRE_StorageDeviceTypeSSD ) &&
//       ( theHDDTrans.e_StartUpDeviceType == THRE_StartupDeviceType_SD )){
    if(( uc_deviceType == THRE_StorageDeviceTypeSSD ) || ( theHDDTrans.e_StartUpDeviceType == THRE_StartupDeviceType_SD )) {
		// uSD装着時.
		if( SYSF_mkdir( DEF_THR_DIR_PATH_SSD ) == ERROR ) {
			THRL_ThLogMsgErr("mkdir error!!\n" );
		}
	}
	else{
		// SSD装着時.
		if( SYSF_mkdir( DEF_THR_DIR_PATH ) == ERROR ) {
			THRL_ThLogMsgErr("mkdir error!!\n" );
		}
	}
#else
	if( getNvRamDeviceType() == THRE_StorageDeviceTypeSSD ){
		if( mkdir( DEF_THR_DIR_PATH_SSD ) != 0 ){
			THRL_ThLogMsgErr( "mkdir error!!\n" );
		}
	}
	else{
		if( mkdir( DEF_THR_DIR_PATH ) != 0 ){
			THRL_ThLogMsgErr( "mkdir error!!\n" );
		}
	}
#endif // #ifdef TORNADO		

	// ◇Start 2008/05/15 kurachi CQ:302954対策.
	// 暗号化有効/無効をチェック.
	memset( uc_ChkFormatHddData, 0x0, sizeof( uc_ChkFormatHddData ));	// 使用前初期化.
	if( b_EnableEnc == False ){
		// 暗号化無効という情報をセット.
		memcpy( &uc_ChkFormatHddData[0], &uc_NonAngo[0], sizeof( uc_ChkFormatHddData ));
	}else if( b_IVFixMode != True ){
		// 暗号化優先という情報をセット.
		memcpy( &uc_ChkFormatHddData[0], &uc_Angoyusen[0], sizeof( uc_ChkFormatHddData ));
	}else{
		// 上書き優先という情報をセット.
		memcpy( &uc_ChkFormatHddData[0], &uc_Uwagakiyusen[0], sizeof( uc_ChkFormatHddData ));
	}
	FILE *wfp = 0;
	// ◇End 2008/05/15 kurachi CQ:302954対策.
//    if(( uc_deviceType == THRE_StorageDeviceTypeHDDAndSSD ) ||
//       ( theHDDTrans.e_StartUpDeviceType == THRE_StartupDeviceType_SSD ) ){
//    if( uc_deviceType == THRE_StorageDeviceTypeHDDAndSSD ){
    if(( uc_deviceType == THRE_StorageDeviceTypeHDDAndSSD ) && ( theHDDTrans.e_StartUpDeviceType == THRE_StartupDeviceType_SSD )) {
		// SSD起動時						
		wfp = SYSF_Fopen( THRD_FileName_IJC, "we" );
	} else {							
		// uSD起動時						
		wfp = SYSF_Fopen( THRD_FileName_IJC_SSD, "we" );
	}							
	if( wfp != NULL ) {
	// ◇Start 2008/05/15 kurachi CQ:302954対策.
		SYSF_Fwrite( &uc_ChkFormatHddData[0], sizeof(uc_ChkFormatHddData), 1, wfp );
	// ◇End 2008/05/15 kurachi CQ:302954対策.

		// テーブルをすべて保存 
		SYSF_Fclose( wfp );
 	} else {
		THRL_ThLogMsgErr("formatOSdriver() Write fopen error!! errno=%d\n", SYSF_errnoGet());
		// AR1012671対応 ここで抜ける場合、以降ata0aへのアクセスはないので、アンマウント中は解除 .
		if( uc_deviceType != THRE_StorageDeviceTypeHDDAndSSD ) {
			gb_umount_exec = False;
		}
 		return( False );
  	}
	
    if(( uc_deviceType == THRE_StorageDeviceTypeHDDAndSSD ) ||
       ( theHDDTrans.e_StartUpDeviceType == THRE_StartupDeviceType_SSD )){
		uc_deviceType = THRE_StorageDeviceTypeHDD;
	} 
	if( chkReadOSdriver(uc_deviceType) == False ) {
		// AR1012671対応 ここで抜ける場合、以降ata0aへのアクセスはないので、アンマウント中は解除 .
		if( uc_deviceType != THRE_StorageDeviceTypeHDDAndSSD ) {
			gb_umount_exec = False;
		}
		return( False );	// IJCフォーマットチェック用ファイル読み込み失敗
	}

	// SSDパターンチェックデータ書込み.
	if( THRF_HWCheck_TestPatternWriteToSSD() == False ){
		printf("SSD TestPattern write failed.\n");
	}

	//2020/12/14 Tsukiji OP_BTS-29664 FWエラーが発生する問題対応(暫定)
	APIC_SoftDipSw* pc_SoftDipSw = APIC_SoftDipSw::newInstance();	// DipSw（API）・インスタンス取得.
	uchar uc_SWvalue = 0;
	if( RestoreSoftSW( &uc_SWvalue ) ) {
		//パスワード設定時に保存したものがある.
		printf( "[%s] Software DIPSW 19-bit0 = [%d]\n",__FUNCTION__,  uc_SWvalue );
		//もとに戻す.
		pc_SoftDipSw->setSoftDipSwBit(16, 1, uc_SWvalue?True:False );
	}

#ifdef TORNADO
	sync();
#endif
	// AR1012671対応 タスクモニタ自動保存との調停 ata0aへのアクセス中にログ出力しないよう、終端で解除.
	if( uc_deviceType != THRE_StorageDeviceTypeHDDAndSSD ) {
		gb_umount_exec = False;
	}
    return ( True );
}

#else		// DEF_SupportSSD.
Bool CP_HDDTrans::formatOSdriver()
{
#ifdef TORNADO
    int32_t no;
    int32_t ret;
    Bool encrypt = APIC_SystemSetting::newInstance()->getEnableEncryption();
    int32_t retry;
// 2012/05/13 sky_kageyama CQ 364868 add.
    char cmd[512];

	// 「一般ユーザーアクセス変換管理ファイル」を更新する.
	std::map<std::string, Bool> tmpctrlFileMap;
	Bool bl_Ret = readMfpAccessCtrlFile(tmpctrlFileMap);
	if( bl_Ret == False ){
		printf( "[%s:%d] readMfpAccessCtrlFile NG.\n", __PRETTY_FUNCTION__, __LINE__ );
		return False;
	}
	
    for( no = 0; part_info[no].type != PART_TYPE_END; no++ ){
        if( chdir( "/km/fw" ) == ERROR ){
            THRL_ThLogMsgErr("chdir error!!\n" );
        }
		if( part_info[no].fstype == PART_FSTYPE_FAT && m_usFormatTargetBit & THRE_FORMATBIT_TERMINAL ){
			ret = formatFAT( (char*)part_info[no].devpath[encrypt] );
			if( ret == -1 ){
				SYSF_logMsg( "[SATA2] formatOSdriver ERR [%s]\n", (char*)part_info[no].devpath[encrypt] );
				return False;
			}
			continue;
		}
#if defined (DEF_64BIT_PF)
    	if( ( part_info[no].fstype != PART_FSTYPE_FS ) && ( part_info[no].fstype != PART_FSTYPE_FS_SSD ) ){
#else
		if( part_info[no].fstype != PART_FSTYPE_FS ){
#endif
			continue;
		}
#ifdef FUM_LINUX
		if( m_usFormatTargetBit & THRE_FORMATBIT_TERMINAL ){
			if( part_info[ no ].mntpnt[0] == 0 ){
				ret = formatFS( (char*)part_info[no].devpath[encrypt] );
				if( ret == -1 ){
					SYSF_logMsg( "[SATA2] formatOSdriver ERR [%s]\n", (char*)part_info[no].devpath[encrypt] );
					return False;
				}
				continue;
			}
		}
#endif	/* #ifdef FUM_LINUX */
		// マウントポイントのフォーマットオプションをチェック.
		Bool b_formatFlg = THRC_PartitionInfo::chkFormatTarget(part_info[no].mntpnt, m_usFormatTargetBit);
		if( b_formatFlg == False ){
			SYSF_logMsg( "\nFormat No Exec %s debug_format=%s\n\n", part_info[no].mntpnt, part_info[no].devpath[encrypt] );
			continue;
		}
//【695新アーキFUM】20111111 FBSC渡辺 新アーキ論理フォーマット対応 end.
        for( retry = 0; retry <= THRD_UMOUNT_RETRYMAX; retry++ ){
            ret = umount2( part_info[no].mntpnt, MNT_FORCE );
            if( ret == 0 ){
                break;
			}else if( SYSF_errnoGet() == EBUSY ){
				delayTask( 10+(retry*50) );//初回10msecで回を重ねるごとに+50msec追加
			}else{
				printf( "umount2 %d [%s]\n", SYSF_errnoGet(), part_info[no].mntpnt );
				ret = 0;
				break;
			}
        }
        uc_umount2_retryCount_format = (uchar)retry;
        if( ret == -1 ){
            printf( "umount2 %d [%s]\n", SYSF_errnoGet(), part_info[no].mntpnt );
// 2012/05/13 sky_kageyama CQ 364868 start.
            memset( cmd, 0, sizeof(cmd) );
			sprintf( cmd, "fuser -va -u %s", device_info[no].mntpnt );
            system( cmd );
            system( "/usr/bin/lsof" );
// 2012/05/13 sky_kageyama CQ 364868 end.
        }

        ret = formatFS( (char*)part_info[no].devpath[(uchar)encrypt] );
        if( ret == -1 ){
            SYSF_logMsg( "[SATA2] formatOSdriver ERR [%s]\n", (char*)part_info[no].devpath[(uchar)encrypt] );
            return False;
        }
    	
    	// フォーマットしたマウントポイントと一致するデータの変換済フラグを未変換に変更する.
    	std::map<std::string, Bool>::iterator itrCtrlFile = tmpctrlFileMap.find( device_info[no].mntpnt );
        if( itrCtrlFile != tmpctrlFileMap.end() ){
        	itrCtrlFile->second = False;
        }
    	
//【695新アーキFUM】20111111 FBSC渡辺 新アーキ論理フォーマット対応 start.
		// ファイルシステムをマウント
		ret = mount( part_info[ no ].devpath[(uchar)encrypt], part_info[ no ].mntpnt, "ext3", MS_NOATIME, EXT3_MOUNT_OPTIONS );
		if( ret == -1 ){
			SYSF_logMsg( "[%s] %s -> %s errno:%d\n", __FUNCTION__, part_info[ no ].devpath[(uchar)encrypt], part_info[ no ].mntpnt, SYSF_errnoGet() );
			return False;
		}
		ret = createDirSystemNeed(part_info[ no ].mntpnt);
		if( ret == -1 ){
			SYSF_logMsg( "[%s] mkdir error %s -> %s errno:%d\n", __FUNCTION__, part_info[ no ].devpath[(uchar)encrypt], part_info[ no ].mntpnt, SYSF_errnoGet() );
			return False;
		}
//【695新アーキFUM】20111111 FBSC渡辺 新アーキ論理フォーマット対応 end.
	}
	// ファイルシステムをマウント
//【695新アーキFUM】20111111 FBSC渡辺 新アーキ論理フォーマット対応 start.
//    ret = mountHdd( (uchar) encrypt );
//    if( ret == -1 ){
//        SYSF_logMsg( "[%s] mountHdd Failed\n", __FUNCTION__ );
//        return False;
//    }
//【695新アーキFUM】20111111 FBSC渡辺 新アーキ論理フォーマット対応 end.	

	// 「一般ユーザーアクセス変換管理ファイル」を更新する.
    Bool writeResult = False;
    writeResult = writeMfpAccessCtrlFile(tmpctrlFileMap);
	if( writeResult == False ){
		printf( "[%s:%d] writeMfpUserCtrlFile NG.\n", __PRETTY_FUNCTION__, __LINE__ );
		return False;
	}

	delayTask( 1000 );
	if( chdir( THRD_DevName_Hdd01 ) == ERROR ) {
		THRL_ThLogMsgErr("chdir error!!\n" );
	}
	if( SYSF_mkdir( (char*)THRD_DirName_IJC ) == ERROR ) {
		THRL_ThLogMsgErr("mkdir error!!\n" );
	}
#else	/* #ifdef TORNADO */
	if( SYSF_mkdir( (char*)DEF_THR_DIR_PATH ) != 0 ){
		THRL_ThLogMsgErr( "mkdir error!!\n" );
	}
#endif	/* #ifdef TORNADO */

	// ◇Start 2008/05/15 kurachi CQ:302954対策.
	// 暗号化有効/無効をチェック.
	memset( uc_ChkFormatHddData, 0x0, sizeof( uc_ChkFormatHddData ));	// 使用前初期化.
	if( b_EnableEnc == False ){
		// 暗号化無効という情報をセット.
		memcpy( &uc_ChkFormatHddData[0], &uc_NonAngo[0], sizeof( uc_ChkFormatHddData ));
	}else if( b_IVFixMode != True ){
		// 暗号化優先という情報をセット.
		memcpy( &uc_ChkFormatHddData[0], &uc_Angoyusen[0], sizeof( uc_ChkFormatHddData ));
	}else{
		// 上書き優先という情報をセット.
		memcpy( &uc_ChkFormatHddData[0], &uc_Uwagakiyusen[0], sizeof( uc_ChkFormatHddData ));
	}
	// ◇End 2008/05/15 kurachi CQ:302954対策.

 	FILE *wfp = SYSF_Fopen( THRD_FileName_IJC, "we" );
	if( wfp != NULL ) {

	// ◇Start 2008/05/15 kurachi CQ:302954対策.
		SYSF_Fwrite( &uc_ChkFormatHddData[0], sizeof(uc_ChkFormatHddData), 1, wfp );
	// ◇End 2008/05/15 kurachi CQ:302954対策.

		// テーブルをすべて保存 
		SYSF_Fclose( wfp );
 	} else {
		THRL_ThLogMsgErr("formatOSdriver() Write fopen error!!\n");
 		return( False );
  	}

	if( chkReadOSdriver() == False ) {
		return( False );	// IJCフォーマットチェック用ファイル読み込み失敗
	}

#ifdef TORNADO
	sync();
#endif

    return ( True );
}

#endif	// DEF_SupportSSD.

//【695新アーキFUM】20111111 FBSC渡辺 新アーキ論理フォーマット対応 start.
/// \brief 指定位置以下にシステムとして必要なディレクトリを作製する.
Bool CP_HDDTrans::createDirSystemNeed(
	const char *mntPnt ///< [in] マウントポイント.
)
{
//【695新アーキFUM】20120126 FBSC鈴木 SimWin.exe ビルドエラー対応.
#if defined(TORNADO )	// not compile when building SimWin.exe
	if(mntPnt == NULL){
		// パラメータエラー.
		return False;
	}
	uint32_t ul_Tragetlen = strlen( mntPnt );
	if( ul_Tragetlen == 0 ){
		// パラメータエラー.
		return False;
	}
	for(uint32_t i=0; i < sizeof(c_systemNeedDir)/sizeof(c_systemNeedDir[0]); ++i ){
		uint32_t ul_len = strlen(c_systemNeedDir[i].p_mntPnt);
		if( ul_Tragetlen != ul_len ){
			continue;
		}
		if(strncmp(mntPnt, c_systemNeedDir[i].p_mntPnt, ul_Tragetlen) == 0 ){
			char tmp[256];
			snprintf(tmp, sizeof(tmp), "%s/%s", c_systemNeedDir[i].p_mntPnt,c_systemNeedDir[i].p_DirPath );
			if( SYSF_mkdir(tmp) == ERROR ) {
				SYSF_logMsg( "[SATA2] [%s](%d) end p_DirePath=[%s] \n", __FUNCTION__, __LINE__, tmp );
				return False;
			}
			int32_t ret = chmod( tmp,  S_IRUSR | S_IWUSR | S_IXUSR | S_IROTH | S_IWOTH | S_IXOTH );
			if(ret != 0){
				SYSF_logMsg( "[%s:%d] (%s) chmod is failed.(%d)\n", __PRETTY_FUNCTION__, __LINE__, tmp, errno );
				return False;
			}
		}
	}
#endif	// not compile when building SimWin.exe
	return True;
}


/*================================================================================================
［機能］	【内部】OSのファイル読み込みチェック
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	True:成功、False：失敗
［詳細］	dosFs2.0のファイル読み込みチェックを行う
 [履歴】	02/01/30 morikawa メモリチェックファンクションによるフォーマット実行に伴い新規追加 
 			02/02/07 中田		IJCfopenのオープンモードに(void*)を付加（低速ＦＵＭコンパイルエラー対策）
 			02/06/19 morikawa	ファイルの読み込みチェックにて、全てのデータが正しくなくても、成功で返していた不具合修正。
 								以前は読み出したiが10でなくてもTrueで返していたが、10の時のみTrueを返すように修正
			04/06/09 ikawa	不要な処理を削除
			04/09/13 t.sasaki 論理フォーマット時のC-5023(メッセージキューフロー)対策－不要なdosFSアクセスを無くす.
			05/06/13 t.sasaki グローバルセマフォ解放＆再確保処理の変更(今までの実装は解放できていない).
			07/07/11 morikawa Donau dosFSのパーティション見直し、実機とSimでの処理を共通化
			08/05/15 kurachi Donau CQ:302954対策.
									フォーマットチェック用ファイルchk_ijc.dataの中身が変更になったため.
									読み込みチェックの修正.
			09/04/15 koshimo Taiga CQ:316762 SSD標準化対応（SSD/HDD dosFsマッピング変更）.
            09/08/02 takayama Taiga CQ:318248  SSDステータス別管理対応.
            11/04/06 河村 CQ:338349 chdir("/ata01")(HDD)にしているため省電力時にHDDがアンマウント出来ない問題を修正
            12/01/20 takayama CQ360543 FD残る問題対策 ファイルアクセスAPIをラッパー関数に置き換え.
            13/02/14 Takatsuji	FA14のアサート文字列のリファイン.
================================================================================================*/
// ※ ※ ※ ※ ※ ※ ※ ※ ※ ※ ※ ※ ※ ※ ※ ※ ※ ※ ※ ※  //
// ※ 本処理呼び出し時にはグローバルセマフォを解放すること！	//
// ※ ※ ※ ※ ※ ※ ※ ※ ※ ※ ※ ※ ※ ※ ※ ※ ※ ※ ※ ※  //
#ifdef DEF_SupportSSD
Bool CP_HDDTrans::chkReadOSdriver( THRE_StorageDeviceType StorageDeviceType )
#else	// DEF_SupportSSD.
Bool CP_HDDTrans::chkReadOSdriver()
#endif	// DEF_SupportSSD.
{
#ifdef TORNADO	
	// 2011/04/06 CQ:338349 河村 - 不要なchdirなので削除.
#if 0
	if( chdir( THRD_DevName_Hdd01 ) == ERROR ) {
		THRL_ThLogMsgErr( "chdir error!!\n" );
		return False ;
	}
#endif
#endif
#ifdef DEF_SupportSSD
	FILE *fp = 0;
	if(( StorageDeviceType == THRE_StorageDeviceTypeSSD ) || (theHDDTrans.e_StartUpDeviceType == THRE_StartupDeviceType_SD)){
		fp = SYSF_Fopen( THRD_FileName_IJC_SSD, "re" );
	} else {
		fp = SYSF_Fopen( THRD_FileName_IJC, "re" );
	}
#else
	FILE *fp = SYSF_Fopen( THRD_FileName_IJC, "re" );
#endif
	if( fp != NULL ) {
		memset( uc_ChkFormatHddData, 0x0, sizeof( uc_ChkFormatHddData ));	// 使用前初期化.
		SYSF_Fread( &uc_ChkFormatHddData, sizeof(uc_ChkFormatHddData), 1, fp );
		if( strcmp( (char*)uc_ChkFormatHddData, (char*)uc_NonAngo ) != 0 ){
			if( strcmp( (char*)uc_ChkFormatHddData, (char*)uc_Angoyusen ) != 0 ){
				if( strcmp( (char*)uc_ChkFormatHddData, (char*)uc_Uwagakiyusen ) != 0 ){
					SYSF_Fclose( fp );
					return( False );
				}
			}
		}
		SYSF_Fclose( fp );
 	} else {
		THRL_ThLogMsgErr("chkReadOSdriver() read fopen erro error!! errno=%d\n", SYSF_errnoGet());
		return( False );
	}
	return True ;
}
/*================================================================================================
［機能］	【内部】OSのファイル読み込みチェック２
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	True:成功、False：失敗
［詳細］	dosFs2.0のチェックファイル有無のみ判定
 [履歴】	05/07/15 t.sasaki Rhein1/2/3新規追加(C-D010が発生しない問題対策、フォーマット有無を判定する)
			07/07/11 morikawa Donau dosFSのパーティション見直し、実機とSimでの処理を共通化
			08/05/15 kurachi Donau CQ:302954対策.
									フォーマットチェック用ファイルchk_ijc.dataの中身をチェックして.
									暗号化優先→上書き優先 or 上書き優先→暗号化優先にモード変更されている.
									場合には読み込み失敗（False）を返すように修正.
			09/04/15 koshimo Taiga CQ:316762 SSD標準化対応（SSD/HDD dosFsマッピング変更）.
            09/08/02 takayama Taiga CQ:318248  SSDステータス別管理対応.
			11/04/12 horishita MinervaZeus CQ340441 HDD非装着時SSD論理フォーマット不可問題対応.
			12/01/20 takayama CQ360543 FD残る問題対策 ファイルアクセスAPIをラッパー関数に置き換え.
================================================================================================*/
// ※ ※ ※ ※ ※ ※ ※ ※ ※ ※ ※ ※ ※ ※ ※ ※ ※ ※ ※ ※  //
// ※ 本処理呼び出し時にはグローバルセマフォを解放すること！	//
// ※ ※ ※ ※ ※ ※ ※ ※ ※ ※ ※ ※ ※ ※ ※ ※ ※ ※ ※ ※  //
#ifdef DEF_SupportSSD
Bool CP_HDDTrans::chkExistOSdriver( THRE_StorageDeviceType StorageDeviceType )
#else	// DEF_SupportSSD.
Bool CP_HDDTrans::chkExistOSdriver()
#endif	// DEF_SupportSSD.
{
	Bool bl_Ret = False ;	// 初期値NG(未フォーマット)とする。
#ifdef DEF_SupportSSD
	//CQ336126対策 Simが起動できない 20110107 H.Matsu
	FILE *fp = 0;
	if(( StorageDeviceType == THRE_StorageDeviceTypeSSD ) || (theHDDTrans.e_StartUpDeviceType == THRE_StartupDeviceType_SD)){
		// uSD起動時
		//CQ336126対策 Simが起動できない 20110107 H.Matsu
		fp = SYSF_Fopen( THRD_FileName_IJC_SSD, "re" );
	} else {
		// SSD起動時
		//CQ336126対策 Simが起動できない 20110107 H.Matsu
		fp = SYSF_Fopen( THRD_FileName_IJC, "re" );
	}
#else	// DEF_SupportSSD.
	//CQ336126対策 Simが起動できない 20110107 H.Matsu
	FILE *fp = SYSF_Fopen( THRD_FileName_IJC, "re" );
#endif	// DEF_SupportSSD.
	if ( fp != NULL ) {
		memset( uc_ChkFormatHddData, 0x0, sizeof( uc_ChkFormatHddData ));	// 使用前初期化.
		//CQ336126対策 Simが起動できない 20110107 H.Matsu
		SYSF_Fread( &uc_ChkFormatHddData, sizeof(uc_ChkFormatHddData), 1, fp );	// 情報を取り出す.

		// 暗号化無効でフォーマットされたものか.
		if( strcmp( (char*)uc_ChkFormatHddData, (char*)uc_NonAngo ) == 0 ){
			// 現在の設定が暗号化無効かどうか.
			if( b_EnableEnc == False ){
				// 読み込みチェックOK.
				bl_Ret = True;
			}
		// 暗号化有効（暗号化優先）でフォーマットされたものか.
		}else if( strcmp( (char*)uc_ChkFormatHddData, (char*)uc_Angoyusen ) == 0 ){
			// 現在の設定が暗号化有効（暗号化優先）かどうか.
			if(( b_EnableEnc == True ) && ( b_IVFixMode == False )){
				// 読み込みチェックOK.
				bl_Ret = True;
			}
		// 暗号化有効（上書き優先）でフォーマットされたものか.
		}else if( strcmp( (char*)uc_ChkFormatHddData, (char*)uc_Uwagakiyusen ) == 0 ){
			// 現在の設定が暗号化有効（上書き優先）かどうか.
			if(( b_EnableEnc == True ) && ( b_IVFixMode == True )){
				//読み込みチェックOK.
				bl_Ret = True;
			}
		}
		//CQ336126対策 Simが起動できない 20110107 H.Matsu
 		SYSF_Fclose( fp ) ;
	}
	return bl_Ret ;
}

#ifdef DEF_SupportSSD

/*================================================================================================
［機能］	【外部】ストレージ領域割り当て初期設定.
--------------------------------------------------------------------------------------------------
［引数］	ushort：デバイスタイプ（HDD:THRE_StorageDeviceTypeHDD SSD:THRE_StorageDeviceTypeSSD.
［戻値］	無し.
［詳細］	デバイスタイプに応じて領域割り当て値を設定する.
［履歴］	2008/10/16 takayama CQ:311209 SSD 領域割り当て動的変更対応.
			2009/02/18 koshimo TaigaFUM CQ:315867 SSD 1GB対応.
			2009/04/05 takayama CQ:316760 SSD常時接続対応.
			2009/06/01 kozushi  CQ:317898 HDD装着時のSSDのSWAP画像スタートブロック値を退避.
			2010/11/26 takayama HDD4kbyteセクタ対応.
			2011/04/06 takayama MinervaZeus SSD512MB対応.
			2013/06/17 ADS滕飛 CQ.381189(Act_66229) EE-SSC_2	HDDレス化対応_SSDへのスワップ対応.
        	2013/08/27 ADS川口 CQ.381189(Act_67618) EE-HDL-5	HDDレス化対応(パーティション各領域の初期設定).
        	2013/09/17 ADS川口 CQ.382834(Act_67818)	4GB SSD の場合スワップ最大ページ数はDEF_SSD_MEMSWAP_INFO_MAX_PAGEとし.
													スワップ画像のページ数はSSDに確保できるページ数で補正する.
			2014/03/06 toda		Zeus5BK HDDレス対応 HDD装着時にSSDへスワップする暫定対応を削除.
			2014/05/28 ADS滕飛 CQ.384987(Act_73302) CQ.500060(Act_500237) HDDレス_コンパイルスイッチ切替対応.
			2014/11/13 yamada	CQ506768 eMMC上書き消去不具合対応.
			2015/01/17 富田(日立JTE) A3/A4共通化FUM対応 HDDレスFAX対応 eMMC_16GB用の処理を追加.
			2015/06/29 yamada	CQ513998 廃棄時消去後の再起動でeMMCが使用出来ない(未フォーマット扱いとなる)問題対応.
			2016/05/23 Shinosaki MAv4 RQ-505 KMフォーマット蓄積画像用とバックアップ用の処理を追加.
            2017/10/10 Yoneda   S800 ストレージ構成変更対応.
			2017/09/26 simogaki パーティション構成変更対応.
			2018/03/27 iyanagi [Eagle]制御構成変更No97「ストレージ装着設定対応」KMフォーマット・バックアップ関連初期値設定追加.
			2018/04/03 Sky_松井	[Eagle]制御構成変更No.97 パーティション構成が変更されたことによる開始位置の変更.
			2018/04/05 大塚	[Sparrow]制御構成変更No.97 ストレージ構成変更(再).
			2018/08/03 sky simogaki cramfs領域破壊問題(AR1051769).
			2018/12/06 sky μSD8Gの時の蓄積領域のサイズ修正.
			2019/07/12 Sky [RQ-560]1TB_SSD対応により、1TB時の制御修正.
			2019/07/12 Sky [RQ-560]Imagge管理テーブルクラスと一時データ上書き削除要求クラスのテーブルを動的確保としたので、ここでinit()を実行するよう、処理追加.
			2019/07/17 shimogaki 制御構成変更_No.41_SWAP領域(ソートページ数上限)見直し対応.
			2020/01/30 sky SparrowAIO/SFP Swap領域拡張対応(128→200).
================================================================================================*/
void CP_HDDTrans::areaInitValue( uchar deviceType )
{
	uint32_t ul_SwapImageMaxPage = 0;
	uint32_t ul_StoreImageMaxPage = 0;
#ifdef DEF_A3A4COMM
	uint32_t ul_StoreInfoMaxPage = 0;
#endif // DEF_A3A4COMM.
	//   SSD.
	if (deviceType & THRE_StorageDeviceTypeSSD) {
		uint32_t ul_SwapInfoMaxPage = 0;				//	Swap領域の管理情報ページ最大数.
		uint32_t ul_SwapImageMargin = 0;				//	Swap領域のマージン.
		uint32_t ul_SwapMaxPage = 0;					//	Swap領域のページ最大数(マージン含む).
	    part_layout* s_DeviceInfo = THRG_GetPartitionType();

		// eMMCのサイズにより、セクタアクセス領域のサイズが異なる.
#ifdef DEF_A3A4COMM
		if(sd_part_info_16GB == s_DeviceInfo) {
			ul_SwapImageMaxPage = DEF_SWAP_IMAGE_MAX_PAGE_SD_16G;					// Swap領域の画像ページ最大数:128page.SparrowAIO/SFPの場合は200ページ.
			ul_SwapInfoMaxPage = DEF_SWAP_INFO_MAX_PAGE_SD_16G;						// Swap領域の管理情報ページ最大数:128page.
			ul_SwapImageMargin = DEF_SWAP_IMAGE_AREA_MARGIN_SD_16G;					// Swap領域マージン.
			ul_SwapStartBlock = DEF_SWAP_START_SD_16G;								// SWAP画像スタートブロック.
			ul_StorageMiniSize = DEF_SSD_MINIMUM_SIZE;								//ストレージ最小サイズ(セクタ).
			ul_StoreImageMaxPage = DEF_STORE_INFO_MAX_PAGE_SD_16G + DEF_STORE_IMAGE_AREA_MARGIN_SD_16G;		// 蓄積画像ページ最大数:160.
			ul_StoreinfoClsNum = DEF_STORE_INFO_MAX_PAGE_SD_16G  * DEF_SSD_INFO_CULSTER_NUM;					// 蓄積情報クラスタ数.
			ul_StoreStartBlock = DEF_STORE_START_BLOCK_16G;													//蓄積スタートブロック.
			ul_OsBlockNum = DEF_OS_BLOCK_NUM_16G;									// OSブロック数.
			ul_FatTotalSize = DEF_FAT_TOTAL_SIZE;									// FAT合計サイズ.
			ul_SectorFatSize = ul_FatTotalSize/DEF_SECTOR_SIZE;						// FATセクタ数.
			ul_SectorFatStoreFirst = DEF_SNAPSHOTINFO_START_BLOCK + DEF_SNAPSHOTINFO_BLOCK_NUM;
			ul_OsStartBlock = THRD_OS_STARTBLOCK_16G;								//OSスタートブロック.
			ul_StorageEnd = THRD_STORAGE_END_16G;									//最終セクタ.
			bl_SwapFatOnlyDisable = True;	// Swapのみ無効.
		}else if (sd_part_info_8GB == s_DeviceInfo) {
#else // DEF_A3A4COMM.
		if (sd_part_info_8GB == s_DeviceInfo) {										// ★★★SD-8GB(8589934592).
#endif // DEF_A3A4COMM.
			ul_SwapImageMaxPage = DEF_SWAP_IMAGE_MAX_PAGE_SD_8G;					// Swap領域の画像ページ最大数:128page DenebMLK又はHeliosMLKの場合は300ページ.
			ul_SwapInfoMaxPage = DEF_SWAP_INFO_MAX_PAGE_SD_8G;						// Swap領域の管理情報ページ最大数:200page.
			ul_SwapImageMargin = DEF_SWAP_IMAGE_AREA_MARGIN_SD_8G;					// Swap領域マージン.
			ul_SwapStartBlock = DEF_SWAP_START_SD_8G;								// SWAP画像スタートブロック.
			ul_StorageMiniSize = DEF_SSD_MINIMUM_SIZE;								// ストレージ最小サイズ(セクタ).
			ul_StoreImageMaxPage = DEF_STORE_INFO_MAX_PAGE_SD_8G + DEF_STORE_IMAGE_AREA_MARGIN_SD_8G;		// 蓄積画像ページ最大数:0.
			ul_StoreinfoClsNum = DEF_STORE_INFO_MAX_PAGE_SD_8G  * DEF_SSD_INFO_CULSTER_NUM;					// 蓄積情報クラスタ数.
			ul_StoreStartBlock = DEF_STORE_START_BLOCK_8G;													//蓄積スタートブロック.
			ul_OsBlockNum = DEF_OS_BLOCK_NUM_8G;									// OSブロック数(IT4時のdosFSのブロック):.
			ul_FatTotalSize = DEF_FAT_TOTAL_SIZE_8G;								// FAT合計サイズ.
			ul_SectorFatSize = ul_FatTotalSize/DEF_SECTOR_SIZE;						// FATセクタ数.
			if ( ul_SectorFatSize != 0 ) {
				bl_SwapFatOnlyDisable = True;	// Swapのみ無効.
			}
			ul_SectorFatStoreFirst = DEF_SNAPSHOTINFO_START_BLOCK + DEF_SNAPSHOTINFO_BLOCK_NUM;
			ul_OsStartBlock = THRD_OS_STARTBLOCK_8G;								//OSスタートブロック.
			ul_StorageEnd = THRD_STORAGE_END_8G;									//最終セクタ.
		} else if ( ssd_part_info_256GB == s_DeviceInfo ) {							// ★★★SSD-256GB(274877906944).
			ul_SwapImageMaxPage = DEF_SWAP_IMAGE_MAX_PAGE_SSD_256G;					// Swap領域の画像ページ最大数:3000page(マージン除く).
			ul_SwapInfoMaxPage = DEF_SWAP_INFO_MAX_PAGE_SSD_256G;					// Swap領域の管理情報ページ最大数:3000page.
			ul_SwapImageMargin = DEF_SWAP_IMAGE_AREA_MARGIN_SSD_256G;				// Swap領域マージン.
			ul_SwapStartBlock = DEF_SWAP_START_SSD_256G;							// SWAP画像スタートブロック.
			ul_StorageMiniSize = DEF_SSD_MINIMUM_SIZE;								// ストレージ最小サイズ(セクタ).
			ul_StoreImageMaxPage = DEF_STORE_INFO_MAX_PAGE_SSD_256G + DEF_STORE_IMAGE_AREA_MARGIN_SSD_256G;	// 蓄積画像ページ最大数:10052.
			ul_StoreinfoClsNum = DEF_STORE_INFO_MAX_PAGE_SSD_256G  * DEF_SSD_INFO_CULSTER_NUM;				// 蓄積情報クラスタ数.
			ul_StoreStartBlock = DEF_STORE_START_BLOCK_256G;												//蓄積スタートブロック.
			ul_OsBlockNum = DEF_OS_BLOCK_NUM_256G;									// OSブロック数(IT4時のdosFSのブロック):.
			ul_FatTotalSize = DEF_FAT_TOTAL_SIZE;									// FAT合計サイズ.
			ul_SectorFatSize = ul_FatTotalSize/DEF_SECTOR_SIZE;						// FATセクタ数.
			ul_SectorFatStoreFirst = DEF_SNAPSHOTINFO_START_BLOCK + DEF_SNAPSHOTINFO_BLOCK_NUM;
			ul_OsStartBlock = THRD_OS_STARTBLOCK_256G;								//OSスタートブロック.
			ul_StorageEnd = THRD_STORAGE_END_256G;									//最終セクタ.
		} else {																	// ★★★SSD-1TB(1099511627776).
			ul_SwapImageMaxPage = DEF_SWAP_IMAGE_MAX_PAGE_SSD_1T;					// Swap領域の画像ページ最大数:3000page(マージン除く).
			ul_SwapInfoMaxPage = DEF_SWAP_INFO_MAX_PAGE_SSD_1T;						// Swap領域の管理情報ページ最大数:3000page.
			ul_SwapImageMargin = DEF_SWAP_IMAGE_AREA_MARGIN_SSD_1T;					// Swap領域マージン.
			ul_SwapStartBlock = DEF_SWAP_START_SSD_1T;								// SWAP画像スタートブロック.
			ul_StorageMiniSize = DEF_SSD_MINIMUM_SIZE;								// ストレージ最小サイズ(セクタ).
			ul_StoreImageMaxPage = DEF_STORE_INFO_MAX_PAGE_SSD_1T + DEF_STORE_IMAGE_AREA_MARGIN_SSD_1T;		// 蓄積画像ページ最大数:50260.
			ul_StoreinfoClsNum = DEF_STORE_INFO_MAX_PAGE_SSD_1T * DEF_SSD_INFO_CULSTER_NUM;					// 蓄積情報クラスタ数.
			ul_StoreStartBlock = DEF_STORE_START_BLOCK_1T;													//蓄積スタートブロック.
			ul_OsBlockNum = DEF_OS_BLOCK_NUM_1T;									// OSブロック数(IT4時のdosFSのブロック).
			ul_FatTotalSize = DEF_FAT_TOTAL_SIZE_1T;								// FAT合計サイズ.
			ul_SectorFatSize = ul_FatTotalSize/DEF_SECTOR_SIZE;						// FATセクタ数.
			ul_SectorFatStoreFirst = DEF_SNAPSHOTINFO_START_BLOCK + DEF_SNAPSHOTINFO_BLOCK_NUM;
			ul_OsStartBlock = THRD_OS_STARTBLOCK_1T;								//OSスタートブロック.
			ul_StorageEnd = THRD_STORAGE_END_1T;									//最終セクタ.
		}

		ul_PortNo = THRE_SsdPortNo;
		c_DevicePath = theHDDTrans.pc_strDeviceSD;

		//   汎用.
		ul_CulsterSize = DEF_CLUSTER_SSD_SIZE;												//画像クラスタサイズ.
		ul_CulsterinfoSize = DEF_CLUSTER_INFO_SIZE;											//画像情報クラスタサイズ.
		ul_SwapMaxPage = ul_SwapImageMaxPage + ul_SwapImageMargin;							//SWAP画像最大ページ.
		// SWAP領域.
		ul_SwapStartBlock2 = ul_SwapStartBlock;												//SWAP画像スタートブロック( HDD装着時SSD値 ).
#if defined(DEF_DenebMLKCus) || defined(DEF_HeliosMLKCus) || defined(DEF_SparrowAIOSFPCus)  // DenebMLK、HeliosMLK、SparrowAIO.
		if ( ul_SwapInfoMaxPage > ul_SwapImageMaxPage ){
			// 管理情報が拡張されている場合 算出できないので、固定値.
			ul_SwapClsNum = DEF_SWAP_INFO_EXPANSION_CLUSTER_NUM;
		} else {
			ul_SwapClsNum = DEF_SSD_1IMAGE_CULSTER_NUM * ul_SwapMaxPage;						//SWAP画像クラスタ数.
		}
		ul_SwapBlkNum = ul_SwapClsNum *  (ul_CulsterSize/DEF_SECTOR_SIZE);						//SWAP画像ブロック数.
		if ( ul_SwapInfoMaxPage > ul_SwapImageMaxPage ){
			// 管理情報が拡張されている場合 算出できないので、固定値.
			ul_SwapinfoStartBlock = DEF_SWAP_INFO_EXPANSION_START_SECTER_NO_SWAPINFO;
		} else {
			ul_SwapinfoStartBlock = ul_SwapStartBlock + ul_SwapBlkNum;							//SWAP情報スタートブロック.
		}
#else // defined(DEF_DenebMLKCus) || defined(DEF_HeliosMLKCus) || defined(DEF_SparrowAIOSFPCus).
		ul_SwapClsNum = DEF_SSD_1IMAGE_CULSTER_NUM * ul_SwapMaxPage;						//SWAP画像クラスタ数.
		ul_SwapBlkNum = ul_SwapClsNum *  (ul_CulsterSize/DEF_SECTOR_SIZE);					//SWAP画像ブロック数.
		ul_SwapinfoStartBlock = ul_SwapStartBlock + ul_SwapBlkNum;							//SWAP情報スタートブロック.
#endif	// defined(DEF_DenebMLKCus) || defined(DEF_HeliosMLKCus) || defined(DEF_SparrowAIOSFPCus).
		ul_SwapinfoClsNum = ul_SwapInfoMaxPage * DEF_SSD_INFO_CULSTER_NUM;					//SWAP情報クラスタ数.
		ul_SwapinfoBlkNum = ul_SwapinfoClsNum * (ul_CulsterinfoSize/DEF_SECTOR_SIZE);		//SWAP情報ブロック数.
		// RW領域
		ul_SysStartBlock = THRD_SYS_STARTBLOCK;												//システムスタートブロック(フォーマットID書き込み開始位置).
		ul_EmmcSysStartBlock = ul_SysStartBlock;											//eMMCシステムスタートブロック(ID書き込み用).
		ul_SysRwChkStartBlock = ul_SwapinfoStartBlock + ul_SwapinfoBlkNum;					//RWチェック開始ブロック.
		ul_SysRwChkStartBlock2 = ul_SysRwChkStartBlock;										// 20012/05/22 N.Kato RWチェック開始ブロック退避.
		ul_SysRwChkBlockNum = DEF_HDD_RW_CHECK_BLOCK_NUM;									//RWチェックブロック数.
		// FAT領域.
		ul_SectorFatSwapFirst = ul_SysRwChkStartBlock + ul_SysRwChkBlockNum;				//SWAP FAT開始セクタ.
		if ( bl_SwapFatOnlyDisable == True ) { //Swapのみ無効.
				ul_SectorFatSwapSecond = ul_SectorFatSwapFirst + 0;							//SWAP FAT2開始セクタ.
		} else {
				// EgaleLowの8Gもこのケースを通るがul_SectorFatSizeが0なので、問題ない.
				ul_SectorFatSwapSecond = ul_SectorFatSwapFirst + ul_SectorFatSize;			//SWAP FAT2開始セクタ.
		}
		ul_SectorFatStoreSecond = ul_SectorFatStoreFirst + ul_SectorFatSize;				//蓄積 FAT2開始セクタ.
		// 蓄積領域.
		ul_StoreClsNum = ul_StoreImageMaxPage * DEF_SSD_1STORE_CULSTER_NUM;					//蓄積クラスタ数.
		ul_StoreBlkNum = ul_StoreClsNum * (ul_CulsterSize/DEF_SECTOR_SIZE);					//蓄積ブロック数.
		ul_StoreinfoStartBlock = ul_StoreStartBlock + ul_StoreBlkNum;						//蓄積情報開始ブロック.
		ul_StoreinfoBlkNum = ul_StoreinfoClsNum * (ul_CulsterinfoSize/DEF_SECTOR_SIZE);		//蓄積情報ブロック数.
		ul_StoreinfoEndBlock2 = ul_StoreinfoStartBlock + ul_StoreinfoBlkNum - 1;			//蓄積情報終端ブロック.

		// KMフォーマット用の画像蓄積領域.
		ul_StoreStartBlock_KM	= ul_StoreStartBlock;										// KMフォーマット用の画像蓄積スタートブロック.
		ul_StoreClsNum_KM		= (uint32_t)( ul_StoreClsNum/2 );							// KMフォーマット用の画像蓄積クラスタ数.
		// バックアップ用のFat管理領域.
		ul_BackupStartBlock		= (uint32_t)( ul_StoreStartBlock_KM + ( ul_StoreClsNum_KM * ( ul_CulsterSize/DEF_SECTOR_SIZE ) ) );	// バックアップ領域スタートブロック.
		ul_BackupClsNum			= (uint32_t)( ul_StoreClsNum/2 );							// バックアップ領域クラスタ数.

		//  OS領域.
		ul_OsStartBlock2 = ul_OsStartBlock;													//HDD装着時のSSDのOSスタートブロック.
		ul_OsBlockNum2 = ul_OsBlockNum;														//HDD装着時のSSDのOSブロック数.
		ul_StorageEnd2 = ul_StorageEnd;
		ul_FatMaxClsNum = ul_StoreClsNum;													//FAT最大クラスタ数.
		if (ul_StorageEnd > DEF_SSD_MINIMUM_SIZE) {
			THRL_ThLogMsgErr("areaInitValue() StorageEnd %u\n", ul_StorageEnd);
		}
	// HDD.
	} else {
        //*************************************************************************************************
        // eMMC分のOS先頭セクタ計算用 開始
        //*************************************************************************************************
        if( theHDDTrans.e_StartUpDeviceType == THRE_StartupDeviceType_SSD) {

            // eMMCのサイズにより、セクタアクセス領域のサイズが異なる.
    		uint32_t ul_eMMC_MemSwapInfo_MaxPage = DEF_SSD_MEMSWAP_INFO_MAX_PAGE_4G;
    		uint32_t ul_eMMC_SwapImage_MaxPage = DEF_SSD_SWAP_IMAGE_MAX_PAGE_4G;

    		//   汎用.
    		ul_CulsterSize = DEF_CLUSTER_SSD_SIZE;												//画像クラスタサイズ.
    		ul_CulsterinfoSize = DEF_CLUSTER_INFO_SIZE;											//画像情報クラスタサイズ.
#ifdef DEF_FUM_HDDLess_Enable
#ifdef DEF_A3A4COMM
    		ul_FatTotalSize = DEF_SSD_FAT_TOTAL_SIZE;											//FAT合計サイズ.
#else // DEF_A3A4COMM.
    		ul_FatTotalSize = DEF_SSD_FAT_TOTAL_SIZE;
#endif // DEF_A3A4COMM.
#else // DEF_FUM_HDDLess_Enable.
    		ul_FatTotalSize = DEF_FAT_TOTAL_SIZE;											//FAT合計サイズ.
#endif // DEF_FUM_HDDLess_Enable.
    		ul_StorageMiniSize = DEF_SSD_MINIMUM_SIZE;											//ストレージ最小サイズ(セクタ).

    		ul_SwapImageMaxPage = ul_eMMC_MemSwapInfo_MaxPage + DEF_SSD_IMAGE_AREA_MARGIN;		//SWAP画像最大ページ.
#ifdef DEF_Soc1200
    		ul_SwapImageMaxPage -= (ul_SwapImageMaxPage - ul_eMMC_SwapImage_MaxPage);			//SSD領域の制限によりメモリスワップ用最大ページ数(管理情報のページ数)と画像情報のページ数が異なるため補正する.
#endif // DEF_Soc1200
		
#ifdef DEF_A3A4COMM
    		ul_StoreImageMaxPage = DEF_SSD_STORE_INFO_MAX_PAGE + DEF_SSD_IMAGE_AREA_MARGIN;			//蓄積画像最大ページ.
    		ul_StoreInfoMaxPage = DEF_SSD_STORE_INFO_MAX_PAGE;
#else // DEF_A3A4COMM.
    		ul_StoreImageMaxPage = DEF_SSD_STORE_INFO_MAX_PAGE + DEF_SSD_IMAGE_AREA_MARGIN;			//蓄積画像最大ページ.
#endif // DEF_A3A4COMM.

    		// SWAP領域.
// ◇ 2009/2/18 koshimo CQ.315867 SSD 1GB対応.
#ifdef DEF_FUM_HDDLess_Enable
    		ul_SwapStartBlock = DEF_SNAPSHOTINFO_START_BLOCK + DEF_SNAPSHOTINFO_BLOCK_NUM + DEF_SSD_SWAP_ALIGNMENT;		//SWAP画像スタートブロック.
#else // DEF_FUM_HDDLess_Enable.
    		ul_SwapStartBlock = DEF_BACKUP_START + DEF_BACKUP_SIZE;											//SWAP画像スタートブロック.
#endif // DEF_FUM_HDDLess_Enable.
    		ul_SwapStartBlock2 = ul_SwapStartBlock;												//SWAP画像スタートブロック( HDD装着時SSD値 ).
    		ul_SwapClsNum = DEF_SSD_1IMAGE_CULSTER_NUM * ul_SwapImageMaxPage;					//SWAP画像クラスタ数.
    		ul_SwapBlkNum = ul_SwapClsNum *  (ul_CulsterSize/DEF_SECTOR_SIZE);					//SWAP画像ブロック数.
    		ul_SwapinfoStartBlock = ul_SwapStartBlock + ul_SwapBlkNum;							//SWAP情報スタートブロック.
    		ul_SwapinfoClsNum = ul_eMMC_MemSwapInfo_MaxPage * DEF_SSD_INFO_CULSTER_NUM;			//SWAP情報クラスタ数.
    		ul_SwapinfoBlkNum = ul_SwapinfoClsNum * (ul_CulsterinfoSize/DEF_SECTOR_SIZE);		//SWAP情報ブロック数.
    		// RW領域
    		ul_SysStartBlock = ul_SwapinfoStartBlock + ul_SwapinfoBlkNum;						//システムスタートブロック.
    		ul_EmmcSysStartBlock = ul_SysStartBlock;											//eMMCシステムスタートブロック(ID書き込み用).
    		ul_SysRwChkStartBlock = ul_SysStartBlock + DEF_HDD_SYS_BLOCK_NUM;					//RWチェック開始ブロック.
    		ul_SysRwChkStartBlock2 = ul_SysRwChkStartBlock;										// 20012/05/22 N.Kato RWチェック開始ブロック退避.
    		ul_SysRwChkBlockNum = DEF_HDD_RW_CHECK_BLOCK_NUM;									//RWチェックブロック数.
    		// FAT領域.
    		ul_SectorFatSize = ul_FatTotalSize/DEF_SECTOR_SIZE;									//FATセクタ数.
#ifdef DEF_FUM_HDDLess_Enable
    		ul_SectorFatSwapFirst = ul_SysRwChkStartBlock + ul_SysRwChkBlockNum;				//SWAP FAT開始セクタ.
    		ul_SectorFatSwapSecond = ul_SectorFatSwapFirst + ul_SectorFatSize;					//SWAP FAT2開始セクタ.
    		ul_SectorFatStoreFirst = ul_SectorFatSwapSecond + ul_SectorFatSize;					//蓄積 FAT開始セクタ.
#else // DEF_FUM_HDDLess_Enable.
    		ul_SectorFatSwapFirst = 0;															//SWAP FAT開始セクタ.
    		ul_SectorFatSwapSecond = 0;															//SWAP FAT2開始セクタ.
    		ul_SectorFatStoreFirst = ul_SysRwChkStartBlock + ul_SysRwChkBlockNum + DEF_SNAPSHOTINFO_BLOCK_NUM;			//蓄積 FAT開始セクタ.
#endif // DEF_FUM_HDDLess_Enable.
    		ul_SectorFatStoreSecond = ul_SectorFatStoreFirst + ul_SectorFatSize;				//蓄積 FAT2開始セクタ.
    		// 蓄積領域.
    		ul_StoreStartBlock = ul_SectorFatStoreSecond + ul_SectorFatSize;					//蓄積スタートブロック.
    		ul_StoreClsNum = ul_StoreImageMaxPage * DEF_SSD_1STORE_CULSTER_NUM;					//蓄積クラスタ数.
    		ul_StoreBlkNum = ul_StoreClsNum * (ul_CulsterSize/DEF_SECTOR_SIZE);					//蓄積ブロック数.
    		ul_StoreinfoStartBlock = ul_StoreStartBlock + ul_StoreBlkNum;						//蓄積情報開始ブロック.
#ifdef DEF_A3A4COMM
    		ul_StoreinfoClsNum = ul_StoreInfoMaxPage * DEF_SSD_INFO_CULSTER_NUM;				//蓄積情報クラスタ数.
#else // DEF_A3A4COMM.
    		ul_StoreinfoClsNum = DEF_SSD_STORE_INFO_MAX_PAGE * DEF_SSD_INFO_CULSTER_NUM;		//蓄積情報クラスタ数.
#endif // DEF_A3A4COMM.
    		ul_StoreinfoBlkNum = ul_StoreinfoClsNum * (ul_CulsterinfoSize/DEF_SECTOR_SIZE);		//蓄積情報ブロック数.
    		ul_StoreinfoEndBlock2 = ul_StoreinfoStartBlock + ul_StoreinfoBlkNum - 1;			//蓄積情報終端ブロック.
    		//  OS領域.
    		ul_OsStartBlock = ul_StoreinfoStartBlock + ul_StoreinfoBlkNum;						//OSスタートブロック.
    		ul_OsBlockNum = DEF_SSD_OS_BLOCK_NUM;												//OSブロック数.
    		ul_StorageEnd2 = ul_OsStartBlock + ul_OsBlockNum - 1;								//最終セクタ.
        }
        //*************************************************************************************************
        // eMMC分のOS先頭セクタ計算用 終了
        //*************************************************************************************************

	    ul_PortNo = THRE_HddPortNo;
		c_DevicePath = theHDDTrans.pc_strDeviceSSD;
		
		//   汎用.
		ul_CulsterSize = DEF_CLUSTER_HDD_SIZE;												//画像クラスタサイズ.
		ul_CulsterinfoSize = DEF_CLUSTER_INFO_SIZE;											//画像情報クラスタサイズ.

		ul_SwapImageMaxPage = DEF_HDD_MEMSWAP_INFO_MAX_PAGE + DEF_HDD_IMAGE_AREA_MARGIN; //SWAP画像最大ページ.
		ul_StoreImageMaxPage = DEF_HDD_STORE_INFO_MAX_PAGE + DEF_HDD_IMAGE_AREA_MARGIN;		//蓄積画像最大ページ.
		ul_FatTotalSize = DEF_FAT_TOTAL_SIZE;											//FAT合計サイズ.
		ul_StorageMiniSize = DEF_HDD_MINIMUM_SIZE;											//ストレージ最小サイズ(セクタ)

		// SWAP領域.
	    if( theHDDTrans.e_StartUpDeviceType == THRE_StartupDeviceType_SD) {
		    ul_SwapStartBlock = DEF_PARTITION_INFO + DEF_HDD_ALIGNMENT;					//SWAP画像スタートブロック. HDDセクタ変更20101216
	    }else{
//	        ul_SwapStartBlock = DEF_PARTITION_SWAP + DEF_PARTITION_INFO + DEF_HDD_ALIGNMENT;    //SWAP画像スタートブロック.
#if defined (DEF_64BIT_PF) && defined(FUM_LINUX)
	        ul_SwapStartBlock = DEF_PARTITION_SWAP + DEF_PARTITION_INFO;    //SWAP画像スタートブロック.
#else
	        ul_SwapStartBlock = DEF_PARTITION_SWAP + 32;    //SWAP画像スタートブロック.
#endif
	    }
		ul_SwapClsNum = DEF_HDD_1IMAGE_CULSTER_NUM * ul_SwapImageMaxPage;				//SWAP画像クラスタ数.
		ul_SwapBlkNum = ul_SwapClsNum *  (ul_CulsterSize/DEF_SECTOR_SIZE);				//SWAP画像ブロック数.
		ul_SwapinfoStartBlock = ul_SwapStartBlock + ul_SwapBlkNum;						//SWAP情報スタートブロック.
		ul_SwapinfoClsNum = DEF_HDD_MEMSWAP_INFO_MAX_PAGE * DEF_HDD_INFO_CULSTER_NUM;	//SWAP情報クラスタ数.
		ul_SwapinfoBlkNum = ul_SwapinfoClsNum * (ul_CulsterinfoSize/DEF_SECTOR_SIZE);	//SWAP情報ブロック数.

		//  OS領域.
		//  09/04/05 takayama CQ:316760 SSD常時接続対応.
		ul_OsStartBlock2 = ul_OsStartBlock;													//HDD装着時のSSDのOSスタートブロック.
		ul_OsBlockNum2 = ul_OsBlockNum;														//HDD装着時のSSDのOSブロック数.
	    if( theHDDTrans.e_StartUpDeviceType == THRE_StartupDeviceType_SD) {
		    ul_OsStartBlock = ul_SwapinfoStartBlock + ul_SwapinfoBlkNum + DEF_SSD_SYSBLOCK_OFFSET;		//OSスタートブロック. HDDセクタ変更20101216
	    }else{
		    ul_OsStartBlock = ul_SwapinfoStartBlock + ul_SwapinfoBlkNum + DEF_SSD_SYSBLOCK_OFFSET2;		//OSスタートブロック. HDDセクタ変更20101216
	    }

		ul_OsBlockNum = DEF_HDD_OS_BLOCK_NUM;												//OSブロック数.
		ul_SysStartBlock = ul_OsStartBlock + ul_OsBlockNum;									//システムスタートブロック.
		ul_SysRwChkStartBlock = ul_SysStartBlock + DEF_HDD_SYS_BLOCK_NUM;					//RWチェック開始ブロック.
		ul_SysRwChkBlockNum = DEF_HDD_RW_CHECK_BLOCK_NUM;									//RWチェックブロック数.
		// FAT領域.
		ul_SectorFatSize = ul_FatTotalSize/DEF_SECTOR_SIZE;									//FATセクタ数.
		ul_SectorFatSwapFirst = ul_SysRwChkStartBlock + ul_SysRwChkBlockNum + DEF_HDD_ALIGNMENT2;//SWAP FAT開始セクタ.HDDセクタ変更20101216
		ul_SectorFatSwapSecond = ul_SectorFatSwapFirst + ul_SectorFatSize;					//SWAP FAT2開始セクタ.
		ul_SectorFatStoreFirst = ul_SectorFatSwapSecond + ul_SectorFatSize;					//蓄積 FAT開始セクタ.
		ul_SectorFatStoreSecond = ul_SectorFatStoreFirst + ul_SectorFatSize;				//蓄積 FAT2開始セクタ.
		// 蓄積領域.
		ul_StoreStartBlock = ul_SectorFatStoreSecond + ul_SectorFatSize;					//蓄積スタートブロック.
		ul_StoreClsNum = ul_StoreImageMaxPage * DEF_HDD_1STORE_CULSTER_NUM;					//蓄積クラスタ数.
		ul_StoreBlkNum = ul_StoreClsNum * (ul_CulsterSize/DEF_SECTOR_SIZE);					//蓄積ブロック数.
		ul_StoreinfoStartBlock = ul_StoreStartBlock + ul_StoreBlkNum;						//蓄積情報開始ブロック.
		ul_StoreinfoClsNum = DEF_HDD_STORE_INFO_MAX_PAGE * DEF_HDD_INFO_CULSTER_NUM;		//蓄積情報クラスタ数.
		ul_StoreinfoBlkNum = ul_StoreinfoClsNum * (ul_CulsterinfoSize/DEF_SECTOR_SIZE);		//蓄積情報ブロック数.
// 2016.05.23 【MAv4】RQ-505[バックアップ機能強化対応] Start.
		// KMフォーマット用の画像蓄積領域.
		ul_StoreStartBlock_KM	= ul_StoreStartBlock;										// KMフォーマット用の画像蓄積スタートブロック.
		ul_StoreClsNum_KM		= (uint32_t)( ul_StoreClsNum/2 );								// KMフォーマット用の画像蓄積クラスタ数.
		// バックアップ用のFat管理領域.
		ul_BackupStartBlock		= (uint32_t)( ul_StoreStartBlock_KM + ( ul_StoreClsNum_KM * ( ul_CulsterSize/DEF_SECTOR_SIZE ) ) );	// バックアップ領域スタートブロック.
		ul_BackupClsNum			= (uint32_t)( ul_StoreClsNum/2 );								// バックアップ領域クラスタ数.
// 2016.05.23 【MAv4】RQ-505[バックアップ機能強化対応] End.
		
		ul_StorageEnd = ul_StoreinfoStartBlock + ul_StoreinfoBlkNum - 1;					//最終セクタ.
		ul_FatMaxClsNum = ul_StoreClsNum;													//FAT最大クラスタ数.
		if (ul_StorageEnd > DEF_HDD_MINIMUM_SIZE) {
			THRL_ThLogMsgErr("areaInitValue() size over HDD error!!\n");
		}
	}

	// Image管理テーブルクラスの初期化をここで行う.
	comImageFileCtrlTblStore.init();
	comImageFileCtrlTblSwap.init();

	// 一時データ上書き削除要求データクラスの初期化をここで行う.
	comHDDEraseStore.init();
	comHDDEraseSwap.init();
}

/*================================================================================================
［機能］	【外部】ストレージ情報の取得.
--------------------------------------------------------------------------------------------------
［引数］	ushort：ストレージ情報タイプ.
［戻値］	ストレージ情報.
［詳細］	デバイス情報タイプに応じてストレージ情報を返す.
［履歴］	2008/10/16 takayama CQ:311209 SSD 領域割り当て動的変更対応.
			2016/05/23 Shinosaki MAv4 RQ-505 KMフォーマット蓄積画像用とバックアップ用の処理を追加.
================================================================================================*/
uint32_t 	 CP_HDDTrans::getStorageInfo( THRE_StorageInfoType us_storageInfoType )
{
	uint32_t ul_StorageInfo = 0;
	
	switch(us_storageInfoType){
		case THRE_StgInfoCulsterSize:
			ul_StorageInfo = ul_CulsterSize;
			break;
		case THRE_StgInfoCulsterInfoSize:
			ul_StorageInfo = ul_CulsterinfoSize;
			break;
		case THRE_StgInfoFatTotalSize:
			ul_StorageInfo = ul_FatTotalSize;
			break;
		case THRE_StgInfoSwapStartBlock:
			ul_StorageInfo = ul_SwapStartBlock;
			break;
		case THRE_StgInfoSwapClsNum:
			ul_StorageInfo = ul_SwapClsNum;
			break;
		case THRE_StgInfoSwapBlkNum:
			ul_StorageInfo = ul_SwapBlkNum;
			break;
		case THRE_StgInfoSwapInfoStartBlock:
			ul_StorageInfo = ul_SwapinfoStartBlock;
			break;
		case THRE_StgInfoSwapInfoClsNum:
			ul_StorageInfo = ul_SwapinfoClsNum;
			break;
		case THRE_StgInfoSwapInfoBlkNum:
			ul_StorageInfo = ul_SwapinfoBlkNum;
			break;
		case THRE_StgInfoOsStartBlock:
			ul_StorageInfo = ul_OsStartBlock;
			break;
		case THRE_StgInfoOsBlockNum:
			ul_StorageInfo = ul_OsBlockNum;
			break;
		case THRE_StgInfoSectFatSize:
			ul_StorageInfo = ul_SectorFatSize;
			break;
		case THRE_StgInfoSectFatSwap1:
			ul_StorageInfo = ul_SectorFatSwapFirst;
			break;
		case THRE_StgInfoSectFatSwap2:
			ul_StorageInfo = ul_SectorFatSwapSecond;
			break;
		case THRE_StgInfoSectFatStore1:
			ul_StorageInfo = ul_SectorFatStoreFirst;
			break;
		case THRE_StgInfoSectFatStore2:
			ul_StorageInfo = ul_SectorFatStoreSecond;
			break;
		case THRE_StgInfoStoreStartBlock:
			ul_StorageInfo = ul_StoreStartBlock;
			break;
		case THRE_StgInfoStoreClsNum:
			ul_StorageInfo = ul_StoreClsNum;
			break;
		case THRE_StgInfoStoreBlkNum:
			ul_StorageInfo = ul_StoreBlkNum;
			break;
		case THRE_StgInfoStoreInfoStartBlock:
			ul_StorageInfo = ul_StoreinfoStartBlock;
			break;
		case THRE_StgInfoStoreInfoClsNum:
			ul_StorageInfo = ul_StoreinfoClsNum;
			break;
		case THRE_StgInfoStoreInfoBlkNum:
			ul_StorageInfo = ul_StoreinfoBlkNum;
			break;
		case THRE_StgInfoFatMaxClsNum:
			ul_StorageInfo = ul_FatMaxClsNum;
			break;
		case THRE_StgInfoStorageMiniSize:
			ul_StorageInfo = ul_StorageMiniSize;
			break;
		case THRE_StgInfoSysStartBlock:
			ul_StorageInfo = ul_SysStartBlock;
			break;
		case THRE_StgInfoSysRwChkStartBlock:
			ul_StorageInfo = ul_SysRwChkStartBlock;
			break;
		case THRE_StgInfoSysRwChkBlockNum:
			ul_StorageInfo = ul_SysRwChkBlockNum;
			break;
		case THRE_StgInfoHddEnd:
			ul_StorageInfo = ul_StorageEnd;
			break;
// 2016.05.23 【MAv4】RQ-505[バックアップ機能強化対応] Start.
		case THRE_StgInfoStoreStartBlock_KM:		// KMフォーマット用の画像蓄積スタートブロック.
			ul_StorageInfo = ul_StoreStartBlock_KM;
			break;
		case THRE_StgInfoStoreClsNum_KM:			// KMフォーマット用の画像蓄積クラスタ数.
			ul_StorageInfo = ul_StoreClsNum_KM;
			break;
		case THRE_StgInfoBackupStartBlock:			// バックアップ領域スタートブロック.
			ul_StorageInfo = ul_BackupStartBlock;
			break;
		case THRE_StgInfoBackupClsNum:				// バックアップ領域クラスタ数.
			ul_StorageInfo = ul_BackupClsNum;
			break;
// 2016.05.23 【MAv4】RQ-505[バックアップ機能強化対応] End.
		default:
			THRL_ThLogMsgErr("getStorageInfo() us_storageInfoType error!!\n");
			break;
	}
	return ul_StorageInfo;
}

/*================================================================================================
［機能］	SSDストレージ領域セクタ開始位置を取得.
--------------------------------------------------------------------------------------------------
［引数］	uchar：領域タイプ（.
［戻値］	ulong：領域開始セクタ位置.
					領域タイプが指定外の場合は0を返す.
［詳細］	指定領域に応じて領域の開始セクタ位置を取得する.
				areaInitValue()のSSD領域の計算式と同じにする.
［履歴］	2012/05/22 N.Kato HWチェックモード対応.
			2012/05/29 N.Kato パターンチェック領域を機種別に切り分け.
			2012/07/05 N.Kato パターンチェック領域をSSD容量別に切り分け.
			2012/09/28 M.Koshimo CQ.372391,372392,372395 コンパイルオプションを削除.
			2013/02/22 R.Ohashi  CQ.378074 機種ごとに別れていたセクタ位置を統一.
			2014/07/24 yamada    CQ.501164 HWチェックモード変更対応(eMMCパターンチェック開始セクタ修正).
			2016/03/09 yamada    CQ.525844 フォーマットID、パターンチェックID修正.
================================================================================================*/
uint32_t CP_HDDTrans::getAreaSector( uchar uc_Area )
{

	uint32_t ul_AreaSectorNum = 0;	// 領域開始セクタ番号.


	// 指定領域の開始セクタ番号を設定.
	switch ( uc_Area ){
		case THRE_CheckArea_SSDRW:
			ul_AreaSectorNum = ul_SysRwChkStartBlock2;		// SSDRWチェック領域を使用.
			break;
		default:
			break;
	}

	return ul_AreaSectorNum;
}
#endif //DEF_SupportSSD.



/*================================================================================================
［機能］	【内部】接続応答メッセージ
--------------------------------------------------------------------------------------------------
［引数］	ulong：接続結果（HDD_STATUS_OK：正常終了、HDD_NOT_INITIALIZED：未フォーマット、その他エラー)
［戻値］	無し
［詳細］	メモリ管理部へ接続処理が終了したことを返す。メモリ管理部は結果に応じて共有データに未フォーマット状態をセットする。
［ｳｫｰｸｽﾙｰ］	実施済み：2001/1/10：森川
※正常の場合だけ返す、異常の場合は異常ステイタス通知の方で通知する
［履歴］	07/05/08：Donau初版 以前使用していたが現在は不要な処理を削除
================================================================================================*/
void CP_HDDTrans::sendConnectionNotice( uint32_t ul_Result )
{
	// ＨＤＤ接続完了メッセージを送信
	sendMessage( TID_Image, TM_HDDConnectionNotice, (uintptr_t)ul_Result );
	THRL_ThLogMsgDebug("HDD Connect %s\n",(ul_Result==HDD_STATUS_OK)?"OK":"NG");
}
/*================================================================================================
［機能］	【内部】フォーマット完了メッセージ送信
--------------------------------------------------------------------------------------------------
［引数］	Bool					：フォーマット完了したかを返す(True;成功)
［戻値］	無し
［詳細］	メモリ管理部へ接続が終了したことを返す
［ｳｫｰｸｽﾙｰ］	実施済み：2001/1/10：森川
※正常の場合だけ返す、異常の場合は異常ステイタス通知の方で通知する
［履歴］	07/05/08：Donau初版 以前使用していたが現在は不要な処理を削除
================================================================================================*/
void CP_HDDTrans::sendFormatCompleteNotice( uint32_t ul_Result )
{
	// ＨＤＤフォーマット完了メッセージを送信
	sendMessage( TID_Image, TM_HDDFormatCompleteNotice, (uintptr_t)ul_Result );
}
/*================================================================================================
［機能］	【内部】状態通知メッセージ
--------------------------------------------------------------------------------------------------
［引数］	ulong	：接続結果（HDD_STATUS_OK：正常終了、HDD_NOT_INITIALIZED：未フォーマット、その他エラー)
					  注：この状態通知メッセージはフォーマットの実行％表示にも用いられる。この時は100以下　
［戻値］	無し
［詳細］	異常時にエラーを通知する
［ｳｫｰｸｽﾙｰ］	実施済み：2001/1/10：森川
［履歴］	07/05/08：Donau初版 以前使用していたが現在は不要な処理を削除
			07/09/27 morikawa 異常発生時にHDDのステータスを記憶する
			10/03/03 	yashiro Gaia問題点リストNo286(CQ324193) : セマフォID dosImageMuteSem 関連処理の削除.
================================================================================================*/
void CP_HDDTrans::sendStatusNotice( uint32_t ul_Result )
{
// 07/09/27 morikawa debug
	THRL_ThLogMsgErr("CP_HDDTrans::sendStatusNotice ul_Result=%"PRId32"\n",ul_Result);

	if( ul_Result >= HARD_ERROR_BASE ) {
		ul_HDDStatus = ul_Result;			// HDD状態保持用　
	}

	// ＨＤＤ接続完了メッセージを送信
	sendMessage( TID_Image, TM_HDDStatusNotice, (uintptr_t)ul_Result );
}

// ================================================================================================ 
//! <!--機能--> ＨＤＤの実行結果を必要に応じてトラブルコード変換＆Fatal状態通知処理 
//! <!---------------------------------------------------------------------------------------------> 
//! <!--引数--> 
//! \param  ulong BSP検出ステータス
//! <!--戻値--> 
//! \return なし
//! <!--詳細--> 
//! \n 	HDDアクセス実行結果に応じて、HDD_STATE_OKではない時に
//! \n 	HDDアクセス関数のBSP検出のトラブルコードをHDDのFATALコードに変換して通知する
//! \n 	デバッグログ１２の内容を見直し予定　
//! <!--履歴--> 
//  07/10/19 morikawa Doonau　新規作成 
//  				トラブルコードはsystem/prt/APIC_Printer.ｈに定義
//  07/11/05 morikawa 未使用変数削除　
// 13/02/14 Takatsuji	FA14のアサート文字列のリファイン.
// 18/08/31 Sky井柳 [Eagle][制御構成変更No.99]パラメータ宣言変更と、ILLEGAL_PRAM時のステータス変更（暫定でのOK化）.
// 18/09/13 Sky井柳 【Eagle】[AR.1049050] ILLEGAL_PRAM時のステータス変更の削除(暫定回避).
// ================================================================================================ 
void CP_HDDTrans::convertAndSetHDDStatus( uint32_t& ul_Status )
{
	uint32_t ul_FatalCode = 0;
	switch( ul_Status ) {
		case HDD_STATE_ERROR_TIMEOUT:
			ul_FatalCode = HDD_TRANS_ERROR; break;
		case HDD_STATE_ERROR_AMNF:
			ul_FatalCode = TYPD_TroubleCode_HDD_AMNF; break;
		case HDD_STATE_ERROR_TKONF:
			ul_FatalCode = TYPD_TroubleCode_HDD_TKONF; break;
		case HDD_STATE_ERROR_ABORT:
			switch( getHDDSeq() ) {
				case EM_HDDSeq_UnLockMode_SetPass:
					// アンロックパスワードコマンド実行時のエラーのみ特別にセキュリティーエラーとする　
					bl_SecurityErrorFlag = True;	// セキュリテーエラーセット　
					break;
				case EM_HDDSeq_EraceUnit_SetPass:
					// 装置消去のためのパスワード設定時にエラー発生は新品のHDDなのでシーケンス継続　
					// シーケンスをアイドル状態に遷移させないための警告フラグセット　
					bl_WarrnigErrorFlag = True;
					break;
				default:
					// 上記以外は全て転送エラーとする　
					ul_FatalCode = TYPD_TroubleCode_HDD_ABORT; 
					break;
			}
			break;	
		case HDD_STATE_ERROR_IDNF:
			ul_FatalCode = TYPD_TroubleCode_HDD_IDNF; break;
		case HDD_STATE_ERROR_UNC:
			ul_FatalCode = TYPD_TroubleCode_HDD_UNC; break;
		case HDD_STATE_ERROR_ICRC:
			ul_FatalCode = TYPD_TroubleCode_HDD_ICRC; break;
		case HDD_ERR_SCSILIB_ILLEGAL_REQ:
			ul_FatalCode = TYPD_TroubleCode_HDD_ILLEGAL_REQ; break;
		case HDD_ERR_SCSILIB_ABORT_CMD:
			ul_FatalCode = TYPD_TroubleCode_HDD_ABORT_CMD; break;
		case HDD_ERR_SCSILIB_BUS_RESET:
			ul_FatalCode = TYPD_TroubleCode_HDD_BUS_RESET; break;
		case HDD_ERR_SCSILIB_ILLEGAL_PRAM:
			ul_FatalCode = TYPD_TroubleCode_HDD_ILLEGAL_PRAM; break;
		case HDD_ERR_SCSILIB_ILLEGAL_BUSID:
			ul_FatalCode = TYPD_TroubleCode_HDD_ILLEGAL_BUSID; break;
		// 正常終了でコールされた時は何もしない　
		case HDD_STATE_OK:
			break;
		case HDD_STATE_NG:
			THRL_ThAssert( False,"CP_HDDTrans::convertAndSetHDDStatus() HDD_STATE_NG Error Status Occurr!!", 0, 0, 0, 0, CFA14_DetailCode0x01F0 );
			break;
		default:
			THRL_ThAssert( False,"CP_HDDTrans::convertAndSetHDDStatus() undefined Error Status Occurr!! status 0x%"PRIx32"", ul_Status, 0, 0, 0, CFA14_DetailCode0x01F1 );
			break;	
	}		
	if( ul_Status != HDD_STATE_OK 
	 && isWarrning() == False ) {
		// 何らかのエラー発生時でかつ、警告フラグがセットされていない時のみシーケンスをアイドル状態にする
		setHDDSeq( EM_HDDSeq_Idle );	// シーケンスを停止（待機状態へ）
	}	
	if( ul_FatalCode != 0 ) {
		// 定義されているエラー発生時のみFATAL表示
		sendStatusNotice( ul_FatalCode );
		// エラー発生時はシーケンスをアイドル状態にする
	}
}

/*================================================================================================
［機能］	【内部】ビット演算（ブロック）
--------------------------------------------------------------------------------------------------
［引数］	uchar*				：比較元アドレス
			uchar*				：比較先アドレス
			ushort				：比較データ数（バイト）
			EC_LogicMode		：演算モード
			uchar				：演算結果
［戻値］	Bool				：比較結果(True:一致 False:不一致)
［詳細］	
［ｳｫｰｸｽﾙｰ］	実施済み：2001/1/10：森川
［履歴］	07/03/23：Donau初版 以前のソースではデータ書き込み時に一時バッファにデータをコピーしていたので
								シミュレーションでの立ち上げ時のRead/Writeチェックにて結果的にデータが一致していたが
								Donauでは一時バッファへのコピーはなくなったので
								シミュレーションではシーケンスを通すためにコピーする。
================================================================================================*/
Bool CP_HDDTrans::bitData( uchar* puc_Sourece, uchar* puc_Destination, ushort us_DataSize, EC_LogicMode ec_LogicMode, uchar uc_Result )
{
	register Bool bl_Result = True;	// 全て等しい
	register uchar uc_BitData = 0;
	// データ数分比較
	for( register ushort idx=0; idx<us_DataSize; idx++ ) {
		// 演算モードで
		switch( ec_LogicMode ) {
			case EC_LogicMode_And:
				uc_BitData = uchar( *(puc_Sourece+idx) & *(puc_Destination+idx) );
				break;
			case EC_LogicMode_Or:
				uc_BitData = uchar( *(puc_Sourece+idx) | *(puc_Destination+idx) );
				break;
			default:
				break;
		}
		// データが異なる
		if( uc_BitData != uc_Result ) {
			// 返値：不一致
			bl_Result = False;
			break;
		}
	}
	return bl_Result;
}

/*================================================================================================
［機能］	【ＩＦ】スリープコマンド実行
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	無し
［詳細］	BSPのスリープ関数コールにより、HDDのスリープ移行処理を行う
［履歴］	07/03/26 morikawa SATA2
 			07/10/19 morikawa	Donau HDDトラブルコード見直し対応
 			09/04/05 takayama CQ:316760 SSD常時接続対応.
			09/04/07 koshimo  CQ.316761 SSD標準化対応（SSDアクセス制御）.
 			10/03/03 yashiro Gaia問題点リストNo286(CQ324193) : セマフォID dosImageMuteSem 関連処理の削除.
			10/07/13 iizuka   ＤＯＳとの調停処理削除 (8536)
			10/09/27 takayama GaiaPF CQ:332993 SSDの省電力処理無効化.
			10/10/18 takayama CQ333996 HDD暗号化時に省電力から復帰しない問題対応.
			11/04/06 河村 CQ:338349 アンマウントの前にクローズしなければならないのに逆になっていた.
			12/11/02 Gonsho	  CQ:374597 省電力移行/復帰時のumountエラーログ強化対応.
			15/03/02 yamada	  CQ511217 C-D010問題 常時暗号化領域(kmsdb8)を平文に戻す対応.
			18/09/21 Sky井柳 【DenebMLK】制御構成変更No78「データ移行」対応 MicroSD起動時のみ暗号化対象化修正.
            19/10/29 Sky      TAS000000001236対応.
================================================================================================*/
void CP_HDDTrans::execSleep()
{
// ◇ 2008/10/16 takayama CQ:311209 SSD 領域割り当て動的変更対応.
// 09/04/05   takayama CQ:316760 SSD常時接続対応.
#ifdef DEF_SupportSSD
	uc_Indicator_HDDSleep = 1 ;
	// 10/09/27 takayama GaiaPF CQ:332993 SSDの省電力処理無効化.
	if(getNvRamDeviceType() == THRE_StorageDeviceTypeHDDAndSSD) {

#ifdef TORNADO
		// 2011/04/06 CQ:338349 河村 - アンマウントの前にクローズしなければならないのに逆になっていた.
		for(typename std::map<std::string, int32_t>::iterator iter = fd_hdd_map.begin(); iter != fd_hdd_map.end(); ++iter)
		{
			SYSF_devclose( iter->second ); // 2010/08/12 GaiaPF8536対応 河村 - 切断するディスクをクローズする.
		}
		fd_hdd_map.clear();

	    unmountHdd(c_DevicePath);
	    if ( b_isUmountError == True ) { // アンマウントエラー確認.
// IT6では、メインストレージの暗号化のみ.
#if 0 // サブストレージ暗号化対応の為に、コードを残しておく.
			if( APIC_SystemSetting::newInstance()->getEnableEncryption() ){	//暗号化有効の時はアサート.
#else //0.
			if( THRG_IsSsdEncrypt() ){	//暗号化有効の時はアサート.
#endif //0.
				// アサート発生させ、詳細コードをNVRAMに保存する.
	            THRL_ThAssert( False, "CP_HDDTrans::execSleep() unmountHDD error." , 0, 0, 0, 0, (ushort)(getFA14DetailCode() + 0x6400) );
			}
		} else {
// この暗号化は、サブストレージの暗号化であるため、IT6では、現状無し.
#if 0 // サブストレージ暗号化対応の為に、コードを残しておく.
			if( APIC_SystemSetting::newInstance()->getEnableEncryption()){
			    setupDmcrypt(c_DevicePath,DEF_HDD_CRYPT_OFF);	// 10/10/18 takayama CQ333996 HDD暗号化時に省電力から復帰しない問題対応.
			}
#ifdef DEF_Zeus5BkCus
			setupDmcryptHddFw(DEF_HDD_CRYPT_OFF);				// 12/08/27 miyamoto WBS7112  HDDパーティション暗号化対応.
#endif	// DEF_Zeus5BkCus.
#endif // 0.
		}
#endif
#if 0	// 2011/04/08 制御案件-③ HDDの省電力モードの変更対応 河村 - SLEEP制御の削除.
		uint32_t ul_Ans = THRF_sysBspHddPowerSleep( (uchar)ul_PortNo, HDD_SLEEPMODE_SLEEP );
		// HDDアクセス実行結果に応じた後処理.
		convertAndSetHDDStatus( ul_Ans );
		// HDD接続の場合、SSDもスリープさせる.           
#endif
	}
#else	// DEF_SupportSSD.
	uc_Indicator_HDDSleep = 1 ;	// スリープ中インジケータ ON 2005/10/07 added by t.sasaki
#ifdef TORNADO
    unmountHdd();
#endif
	//ulong ul_Ans = THRF_sysBspHddPowerSleep( 0, HDD_SLEEPMODE_SLEEP );
	uint32_t ul_Ans = THRF_sysBspHddPowerSleep( 0, HDD_SLEEPMODE_STANDBY_IMMEDIATE );
	// HDDアクセス実行結果に応じた後処理　
	convertAndSetHDDStatus( ul_Ans );
#endif	// DEF_SupportSSD.
}

/*================================================================================================
［機能］	【ＩＦ】OS他アクセス禁止
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	無し
［詳細］	キャッシュが書かれるのを待って停止(電源投入中1度のみ動作) 
			ジョブ及びサービスのフォーマットは動作していない事
［履歴]		05/09/16 tsukiji HDDアクセス禁止処理追加
		05/12/22 t.sasaki DosFSデータ消去中に対応
		06/05/10 morikawa Rheinの修正をマージ
		09/01/26 tadamasa アンチパターン対応　FILC_OverWriteEraseIfを使用するように修正.
================================================================================================*/
Bool CP_HDDTrans::execHDDAccessSuspend()
{
#if defined(COSIM) && defined(THR_SIMWIN) // 【COSIM+ｽﾚｯﾄﾞSIMWIN対応】 
	return True;
#else
	if( False == bl_AccessSuspend ) {
		bl_AccessSuspend = True;
		//2009/03/30 newInstance対応 comMemoryManagerのインスタンス生成.
		IMGC_ImageMemoryPoolImpColorFileMemoryInstance& comMemoryManager = IMGC_ImageMemoryPoolImpColorFileMemorySingletonIf::newInstance();
#ifdef TORNADO
		FILC_FsControlIf::setHddSleepStart();		// HDDをアクセス禁止状態にする 2005/07/15 moved by t.sasaki
		short ss_Cnt = 0;
        sync();
		do {
			SYSF_taskDelay(3000);											// しばらくアクセスが残る可能性があるので遅延(3秒)
			// ここで回るのは一時データ消去中、もしくはジョブ動作中？
		} while ( ( (comMemoryManager.isHDDTransReq() == True)				// HDD転送中(画像・管理)
				 || (FILC_OverWriteEraseIf::getOverWriteEraseState() != TYPE_EraseState_Wait) )	// DosFS消去実行中.
					 && (10>ss_Cnt++) );
		// 最大30秒間はアクセス中か待つ、駄目なら諦める
		if( 10 <= ss_Cnt ) {
			SYSF_logMsg("Access Wait TimeOut\n",0,0,0,0,0,0);
		}
#if !defined(LINUX)
		if( SYSF_taskSuspend ( SYSF_taskNameToId((char*)"tDcacheUpd") ) == ERROR ) {
			THRL_ThLogMsgErr("taskSuspend tDcacheUpd Error\n");
		}
#endif
#endif				// #ifdef TORNADO
		comMemoryManager.setStorageBusy( True ) ;		// 全データ上書き削除時にsleep移行しまう不具合対策
		comMemoryManager.setHDDReady( HDD_NOT_INITIALIZED );
	}
	return True;
#endif // #if defined(COSIM) && defined(THR_SIMWIN) // 【COSIM+ｽﾚｯﾄﾞSIMWIN対応】 
}

/*================================================================================================
［機能］	【ＩＦ】OS他アクセス許可
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	無し
［詳細］	HDDアクセス許可を設定する.
［履歴]		12/06/19 takayama UV-01224対応 サービスモードでバックアップ中の場合、HDDアクセス許可設定.
							  UV-01224機能以外は従来どおり、HDDアクセスを許可しない.
================================================================================================*/
void CP_HDDTrans::execHDDAccessResume()
{
#if defined(COSIM) && defined(THR_SIMWIN) // 【COSIM+ｽﾚｯﾄﾞSIMWIN対応】 
	return True;
#else
	if ( APIC_SystemStatus::newInstance()->getServiceModeOn() == True && APIC_SystemStatus::newInstance()->getDoBackup() == True){
		FILC_FsControlIf::setHddSleepEnd();
	}
	return;
#endif // #if defined(COSIM) && defined(THR_SIMWIN) // 【COSIM+ｽﾚｯﾄﾞSIMWIN対応】 
}

/*================================================================================================
［機能］	【ＩＦ】バックアップする領域を返す
--------------------------------------------------------------------------------------------------
［引数］	short					：インデックス番号：１～１０
			ulong&					：開始セクタ番号(0-)
			ulong&					：終了セクタ番号(0-)
［戻値］	Bool					：True:正常
［詳細］
［履歴]		05/09/05 tsukiji 新規追加(BackUp Utility用I/F)
			08/06/13 kurachi CQ:303684対策（Donau_250GB-HDD対応）.
			08/08/26 kurachi Donau CQ:307542対策.
								1セクタ多くサイズを返していたので最終セクタ番号を修正.
			08/10/16 takayama CQ:311209 SSD 領域割り当て動的変更対応.
================================================================================================*/
Bool CP_HDDTrans::backup_Area( short ss_Index, int32_t& sl_StartSec, int32_t& sl_EndSec )
{
	switch( ss_Index ){
		case 1:
// ◇ 2008/10/16 takayama CQ:311209 SSD 領域割り当て動的変更対応.
#ifdef DEF_SupportSSD
			sl_StartSec = ( int32_t )ul_OsStartBlock;	// OS領域から.
			sl_EndSec = ( int32_t )ul_StorageEnd;	// 最終まで（OS+RW+FAT+蓄積）.
#else //DEF_SupportSSD.
			sl_StartSec = ( int32_t )DEF_HDD_OS_START_BLOCK;	// OS領域から.
			sl_EndSec = ( int32_t )DEF_HDD_ACCSESS_END;	// 最終まで（OS+RW+FAT+蓄積）.
#endif	// DEF_SupportSSD.
			break;
		default:
			sl_StartSec = -1L;
			sl_EndSec = -1L;
			break;
	}
	return True;
}

/*================================================================================================
［機能］	【ＩＦ】バックアップされたデータをHDDへ書き戻す
--------------------------------------------------------------------------------------------------
［引数］	uchar*					：書き込みデータアドレス
			ulong					：開始セクタ番号(0-)
			ulong					：終了セクタ番号(0-)
			void (*)()				：コールバック関数
			uchar					：バックアップタイプ.
［戻値］無し
［詳細］	関数をすぐ終了するために要求をメッセージとして処理する(他のシーケンスは動作していない事)
［履歴]		05/09/16 tsukiji 追加 
			05/11/01 tsukiji I/F変更 
			16/05/23 Shinosaki MAv4 RQ-505 引数にバックアップタイプを追加. 
================================================================================================*/
// 2016.05.23 【MAv4】RQ-505[バックアップ機能強化対応] Start.
#if 0
void CP_HDDTrans::backup_Restore( void* p_RestoreData, int32_t sl_StartSec, int32_t sl_EndSec, void (*ans_BackupHDD)(Bool bl_Ok) )
#else
void CP_HDDTrans::backup_Restore( void* p_RestoreData, int32_t sl_StartSec, int32_t sl_EndSec, void (*ans_BackupHDD)(Bool bl_Ok),uchar uc_backupType )
#endif
// 2016.05.23 【MAv4】RQ-505[バックアップ機能強化対応] End.
{
	memset( &st_MsgHDDBackupRestoreReq, 0, sizeof(ST_MsgHDDBackupRestoreReq) );
	st_MsgHDDBackupRestoreReq.ev_TaskID     = TID_HDDTrans;
	st_MsgHDDBackupRestoreReq.ss_req        = 2;
	st_MsgHDDBackupRestoreReq.p_RestoreData = p_RestoreData;
	st_MsgHDDBackupRestoreReq.sl_StartSec   = sl_StartSec;
	st_MsgHDDBackupRestoreReq.sl_EndSec     = sl_EndSec;
// 2016.05.23 【MAv4】RQ-505[バックアップ機能強化対応] Start.
	st_MsgHDDBackupRestoreReq.ss_reqType	= uc_backupType;
	setBackupCallBackFunc( ans_BackupHDD );
// 2016.05.23 【MAv4】RQ-505[バックアップ機能強化対応] End.
	sendMessage( TID_HDDTrans, TM_HDDRestoreReq, (uintptr_t)&st_MsgHDDBackupRestoreReq );
}
/*================================================================================================
［機能］	【ＩＦ】バックアップするデータをHDDから読み出す
--------------------------------------------------------------------------------------------------
［引数］	uchar*					：書き込みデータアドレス
			ulong					：開始セクタ番号(0-)
			ulong					：終了セクタ番号(0-)
			void (*)()				：コールバック関数
			uchar					：バックアップタイプ.
［戻値］無し
［詳細］	関数をすぐ終了するために要求をメッセージとして処理する(他のシーケンスは動作していない事)
［履歴]		05/09/16 tsukiji 追加 
			05/11/01 tsukiji I/F変更 
			16/05/23 Shinosaki MAv4 RQ-505 引数にバックアップタイプを追加. 
================================================================================================*/
// 2016.05.23 【MAv4】RQ-505[バックアップ機能強化対応] Start.
#if 0
void CP_HDDTrans::backup_Store( void* p_RestoreData, int32_t sl_StartSec, int32_t sl_EndSec, void (*ans_BackupHDD)(Bool bl_Ok) )
#else
void CP_HDDTrans::backup_Store( void* p_RestoreData, int32_t sl_StartSec, int32_t sl_EndSec, void (*ans_BackupHDD)(Bool bl_Ok),uchar uc_backupType )
#endif
// 2016.05.23 【MAv4】RQ-505[バックアップ機能強化対応] End.
{
	memset( &st_MsgHDDBackupRestoreReq, 0, sizeof(ST_MsgHDDBackupRestoreReq) );
	st_MsgHDDBackupRestoreReq.ev_TaskID     = TID_HDDTrans;
	st_MsgHDDBackupRestoreReq.ss_req        = 1;
	st_MsgHDDBackupRestoreReq.p_RestoreData = p_RestoreData;
	st_MsgHDDBackupRestoreReq.sl_StartSec   = sl_StartSec;
	st_MsgHDDBackupRestoreReq.sl_EndSec     = sl_EndSec;
// 2016.05.23 【MAv4】RQ-505[バックアップ機能強化対応] Start.
	st_MsgHDDBackupRestoreReq.ss_reqType	= uc_backupType;
	setBackupCallBackFunc( ans_BackupHDD );
// 2016.05.23 【MAv4】RQ-505[バックアップ機能強化対応] End.
	sendMessage( TID_HDDTrans, TM_HDDBackupReq, (uintptr_t)&st_MsgHDDBackupRestoreReq );
}

/*================================================================================================
［機能］	バックアップされたデータをHDDへ書き戻す
--------------------------------------------------------------------------------------------------
［引数］	uchar*					：書き込みデータアドレス
			ulong					：開始セクタ番号(0-)
			ulong					：終了セクタ番号(0-)
［戻値］	Bool					：True:正常 False:エラー(パラメータ異常他)
［詳細］	この関数はバックアップ処理中に何回も呼ばれるのでそれを考慮しておくこと
			処理時間が長い可能性があるので上位はそれを考慮できること（処理時間が数秒～）
［履歴]		05/09/05 tsukiji 新規追加(BackUp Utility用I/F)
			05/09/16 tsukiji I/F変更、HDDアクセスを禁止処理追加 
			05/11/01 tsukiji I/F変更 
			07/03/19 morikawa SATA2対応　
			07/05/14 morikawa 静的解析の不具合対応　
			07/06/04 morikawa 単体検証不具合対応（開始セクタ→終了セクタまでの実行条件が間違っており、１セクタ余分に転送しアサートしていた）
			08/06/27 kurachi Donau CQ:302882対策.
									上位から渡されるアドレスが奇数アドレスである場合を考慮して、.
									暗号化ありなし問わず、偶数アドレスのバッファに一度コピーしてデータ転送するように.
									AESのDMAをスルー設定で用いる.
			08/08/26 kurachi Donau CQ:307542対策.
									①全体バックアップにおいて、99％のままバックアップが終了しない問題対策.
										AES転送は1アレイ8K単位でデータ転送するように転送回数を計算しているが、.
										上位から指定してくる転送サイズ（開始セクタ、終了セクタ）が、8K単位（16セクタ単位）でない場合に.
										余りを切り捨てて転送回数を設定していたため、転送回数が1回足りなくなっていた。.
										8K単位に切り上げるように修正.
									②1セクタバックアップするとFA14発生する問題対策.
										アレイ転送は256セクタ単位で転送するため、while分で回しているが、.
										1セクタだけの指定だと、条件不成立で転送開始されない。.
										開始セクタと終了セクタがイコールでも転送するように修正.
			09/04/05 takayama CQ:316760 SSD常時接続対応.
 			14/12/09 ADS八木  CQ.505149 HDDバックアップ/リストア不具合対応.
 			14/03/10 岡部朋(日立JTE)	A3A4共通化FUM 暗号化DMA対応.
 										syncSendMsgExecHDD()の引数に「バックアップ/リストア識別」を渡す.
	        16/05/23 Shinosaki MAv4 RQ-505 サーバーへバックアップ2の時はHDDスリープ移行を抑制する.
	        16/12/21 日立田中  MAv5 RQ-668 SIMでバックアップ/リストアを動作させるための修正を追加.
================================================================================================*/
void CP_HDDTrans::execRestore( uintptr_t ul_Option )
{
	ST_MsgHDDBackupRestoreReq*	pst_MsgHDDBackupRestoreReq = (ST_MsgHDDBackupRestoreReq*)ul_Option;
	if( pst_MsgHDDBackupRestoreReq==NULL ) {
		//ありえないが・・・
		THRL_ThLogMsgErr("Backup Restore Message Parameter NG!!\n");
		DebugForever( CF062_DebugFatal );	// コマンド異常とする 
		return;
	}
	DLONG dl_Size = getPhysicalHDDSize();
	uint32_t ul_TotalSectorNum = (uint32_t)(dl_Size / (DLONG)DEF_SECTOR_SIZE);
	register uint32_t ul_SectorNo = (uint32_t)pst_MsgHDDBackupRestoreReq->sl_StartSec;	
	register uint32_t ul_EndSectorNo = (uint32_t)pst_MsgHDDBackupRestoreReq->sl_EndSec;	
	register uchar* puc_Data = (uchar*)pst_MsgHDDBackupRestoreReq->p_RestoreData;
	register uint32_t ul_SecNum = 256;	// 1回に読み書き行うセクタ数（デフォルト）
	setDebugLowSpeedSeq( 876, (uintptr_t)ul_SectorNo );
	if( us_debug_HDDdispparm > 0 ) {
		printf( "Backup Restore Addr[%p] Start Sector[%"PRId32"] End Sector[%"PRId32"] MAX[%"PRId32"]\n", puc_Data, ul_SectorNo, ul_EndSectorNo, ul_TotalSectorNum ); 
	}
	// 引数の正当性チェック(アドレスはNULLではない、開始<終了、開始セクタは最大セクタより小さい)
	if( (puc_Data==NULL) || (ul_SectorNo>ul_EndSectorNo) || (ul_SectorNo>=ul_TotalSectorNum) ) {
		THRL_ThLogMsgErr("Backup Restore Parameter NG!!\n");
		// 終了した場合はコールバック関数を呼び出す 
		execBackupCallBackFunc( False );
		return;
	}
	// シーケンスが動作していない事を確認
	if( getHDDSeq() != EM_HDDSeq_Idle ) {
		THRL_ThLogMsgErr("Backup Restore Execute NG!!\n");
		// 終了した場合はコールバック関数を呼び出す 
		execBackupCallBackFunc( False );
		return;
	}	// シーケンス実行中は待ち状態とする

	// OS他のアクセス禁止(最初の1回目のみ実行される)
// 2016.05.23 【MAv4】RQ-505[バックアップ機能強化対応] Start.
#if 0
	execHDDAccessSuspend();
#else
	if( THRD_BackupType_ToHDD == pst_MsgHDDBackupRestoreReq->ss_reqType ){
		execHDDAccessSuspend();
	}else{
		//printf("[%s][%s] No execHDDAccessSuspend\n",__FILE__,__FUNCTION__);
		;
	}
#endif
// 2016.05.23 【MAv4】RQ-505[バックアップ機能強化対応] End.
	// 
	
// 07/03/19 morikawa SATA2 変更　
//////////////////////////////////////////////////////
// HDDに転送すべきモードを決定
//////////////////////////////////////////////////////
	THRE_ReadWriteHDDMode e_ReadWriteHDDMode = THRE_ReadWriteBufToHDD;	// バッファを介してのHDDデータ転送.
	// puc_Dataをarrayにコピーする
	uint32_t ul_TransSize = ( ul_EndSectorNo - ul_SectorNo + 1 ) * DEF_SECTOR_SIZE;	// 転送サイズ.
	uint32_t ul_TransArrayNum = _roundup( ul_TransSize, (uint32_t)THRD_ONEARRAYSIZE );		// 転送アレイ数.
#ifdef DEF_Soc1200 //CQ.505149 HDDバックアップ/リストア不具合対応.
	memset( e_ArrayType, 0, sizeof( e_ArrayType ) );
#endif //#ifdef DEF_Soc1200.
	for( uint32_t ul_ArrayCount=0; ul_ArrayCount<ul_TransArrayNum; ++ul_ArrayCount ) {
		// 使用するアレイアドレスを転送個数分用意する 
		ul_TransArrayAdr[ ul_ArrayCount ] = (uintptr_t)puc_Data;
#ifdef DEF_Soc1200 //CQ.505149 HDDバックアップ/リストア不具合対応.
		e_ArrayType[ul_ArrayCount] = TYPE_ArrayMemoryType_Main;	//バックアップ・リストア用バッファ(netsDataBuf)はメインメモリ側. 
#endif //#ifdef DEF_Soc1200.
		puc_Data += THRD_ONEARRAYSIZE;
	}
	uint32_t ul_OneTransArrayNum = 256 * DEF_SECTOR_SIZE / THRD_ONEARRAYSIZE;
	// IV設定　
	setIVNum( ul_SectorNo, ul_TransArrayNum, ul_OneTransArrayNum );
// 暗号化ありのHDD読み出しはAES処理タスクへメッセージ必要　
// AES処理タスクと同期するために、最初に全てのアドレスを渡す必要がある　
// 同期は２５６セクタ単位で行なう　
#ifdef DEF_Soc1200 //CQ.505149 HDDバックアップ/リストア不具合対応.
#ifdef DEF_A3A4COMM
	syncSendMsgExecHDD( THRE_EncHDDArrayToBuf,
						ul_TransArrayAdr,
						False,		// アレイアドレスは複数個から成る　
						ul_TransArrayNum,
						ul_OneTransArrayNum,
						0,False,NULL,0,
						True,e_ArrayType ,True);	// 不連続領域から連続領域（偶数アドレス）へのデータ変換.
#else	// #ifdef DEF_A3A4COMM.
	syncSendMsgExecHDD( THRE_EncHDDArrayToBuf,
						ul_TransArrayAdr,
						False,		// アレイアドレスは複数個から成る　
						ul_TransArrayNum,
						ul_OneTransArrayNum,
						0,False,NULL,0,
						True,e_ArrayType );	// 不連続領域から連続領域（偶数アドレス）へのデータ変換.
#endif	// #ifdef DEF_A3A4COMM.
#else //#ifdef DEF_Soc1200.
	syncSendMsgExecHDD( THRE_EncHDDArrayToBuf,
						ul_TransArrayAdr,
						False,		// アレイアドレスは複数個から成る　
						ul_TransArrayNum,
						ul_OneTransArrayNum,
						0,False,NULL,0,
						True );	// 不連続領域から連続領域（偶数アドレス）へのデータ変換.
#endif //#ifdef DEF_Soc1200.					
	// 最終が最大サイズを超える場合は丸めて実行
	if( ul_EndSectorNo >= ul_TotalSectorNum ) {
		ul_EndSectorNo = ul_TotalSectorNum - 1;
	}	
	
	uint32_t ul_EndArrayNum = 0;	// 転送終了したアレイ数初期化
	
	// 開始セクタ→終了セクタまで（通常256セクタ単位で行う、最終の半端な分はそのサイズ）
	while( ul_SectorNo <= ul_EndSectorNo ) {
		setDebugHddSeq(300, 0);	// 2005/10/07 modified by t.sasaki
		// 残りが256より小さいの個数なら読み書きセクタ数変更
		if( (ul_EndSectorNo-ul_SectorNo+1) < 256 ) {
			ul_SecNum = ul_EndSectorNo - ul_SectorNo + 1;
		}
		// 暗号化ありの時のバッファアドレス獲得　
		uintptr_t ul_MemAdr = c_HDDBuf.getReadAdr();	// 暗号化で書かれたものを読み出し　
		
		// 暗号化ありを考慮したスタート前処理実行
		syncStartOneExecHDD( True ); // 不連続領域から連続領域（偶数アドレス）へのデータ変換モードで.
		
		// キャッシュフラッシュはHDD転送実行関数で行う　
		// HDD転送実行

#ifdef SIMULATOR
		// SIMでバックアップ/リストアを動作させるための修正.
		memcpy( (uchar*)ul_MemAdr, (uchar*)ul_TransArrayAdr[ul_EndArrayNum], ul_SecNum * DEF_SECTOR_SIZE );
#endif    /* #ifdef SIMULATOR */

// 2009/04/05   takayama CQ:316760 SSD常時接続対応.
#ifdef DEF_SupportSSD
		readWriteExecHDD( c_DevicePath, e_ReadWriteHDDMode, &ul_TransArrayAdr[ul_EndArrayNum], ul_MemAdr, ul_SectorNo, ul_SecNum ); 

#else	// DEF_SupportSSD.
		readWriteExecHDD( e_ReadWriteHDDMode, &ul_TransArrayAdr[ul_EndArrayNum], ul_MemAdr, ul_SectorNo, ul_SecNum );
#endif	// DEF_SupportSSD.

		// 暗号化ありを考慮した終了後処理実行
		syncEndOneExecHDD( True ); // 不連続領域から連続（偶数アドレス）へのデータ変換モードで.
		
		// 次の書き込みセクタと読み出しメモリアドレス設定
		ul_SectorNo += ul_SecNum;
		
		// 転送完了したアレイ数
		ul_EndArrayNum += ul_SecNum * DEF_SECTOR_SIZE / THRD_ONEARRAYSIZE;
	}
	
	// 暗号化ありを考慮した全終了後処理実行
	syncEndAllExecHDD( THRE_ReadWriteBufToHDD, True ); // 第2引数には、不連続領域から連続領域（偶数アドレス）へのデータ変換モードで.
	// UV-01224機能以外はAPIを呼び出してもHDDアクセス許可にはしない.
// 2016.05.23 【MAv4】RQ-505[バックアップ機能強化対応] Start.
#if 0
	execHDDAccessResume();
#else
	if( THRD_BackupType_ToHDD == pst_MsgHDDBackupRestoreReq->ss_reqType ){
		execHDDAccessResume();
	}else{
		// サーバへのバックアップの場合は、キャンセル機能に対応するためHDDアクセスを許可する.
		//printf("[%s][%s] No execHDDAccessResume\n",__FILE__,__FUNCTION__);
		;
	}
#endif
// 2016.05.23 【MAv4】RQ-505[バックアップ機能強化対応] End.

	// 終了した場合は、登録したコールバック関数に応じたコールバック関数を呼び出す 
// 2016.05.23 【MAv4】RQ-505[バックアップ機能強化対応] Start.
	execBackupCallBackFunc( True );
// 2016.05.23 【MAv4】RQ-505[バックアップ機能強化対応] End.
}

/*================================================================================================
［機能］	バックアップのためHDDから読み出す
--------------------------------------------------------------------------------------------------
［引数］	uchar*					：読み出しデータアドレス
			ulong					：開始セクタ番号(0-)
			ulong					：終了セクタ番号(0-)
［戻値］	Bool					：True:正常 False:エラー(パラメータ異常他)
［詳細］	この関数はバックアップ処理中に何回も呼ばれるのでそれを考慮しておくこと
			処理時間が長い可能性があるので上位はそれを考慮できること（処理時間が数秒～）
［履歴]		05/09/05 tsukiji 新規追加(BackUp Utility用I/F)
			05/09/16 tsukiji I/F変更、HDDアクセスを禁止処理追加 
			05/11/01 tsukiji I/F変更、コールバック関数で応答を返す 
			07/03/19 morikawa SATA2対応　
			07/05/14 morikawa 静的解析の不具合対応　
			07/06/04 morikawa 単体検証不具合対応（開始セクタ→終了セクタまでの実行条件が間違っており、１セクタ余分に転送しアサートしていた）
			08/03/11 ueda (CQ:301020) 
			              出力先アドレスにHDDからの読み出しデータを書き込む際、 
			              誤ってバッファの読み込み用アドレスを設定していたので、 
			              書き込み用アドレスを設定するように修正。 
			              念のため転送先のキャッシュフラッシュを追加。 
			08/06/27 kurachi Donau CQ:302882対策.
									上位から渡されるアドレスが奇数アドレスである場合を考慮して、.
									暗号化ありなし問わず、偶数アドレスのバッファに一度コピーしてデータ転送するように.
									AESのDMAをスルー設定で用いる.
			08/08/26 kurachi Donau CQ:307542対策.
									①全体バックアップにおいて、99％のままバックアップが終了しない問題対策.
										AES転送は1アレイ8K単位でデータ転送するように転送回数を計算しているが、.
										上位から指定してくる転送サイズ（開始セクタ、終了セクタ）が、8K単位（16セクタ単位）でない場合に.
										余りを切り捨てて転送回数を設定していたため、転送回数が1回足りなくなっていた。.
										8K単位に切り上げるように修正.
									②1セクタバックアップするとFA14発生する問題対策.
										アレイ転送は256セクタ単位で転送するため、while分で回しているが、.
										1セクタだけの指定だと、条件不成立で転送開始されない。.
										開始セクタと終了セクタがイコールでも転送するように修正.
			09/10/28 岡田			キャッシュ制御対応.
 			14/12/09 ADS八木		CQ.505149 HDDバックアップ/リストア不具合対応.
 			14/03/10 岡部朋(日立JTE)A3A4共通化FUM 暗号化DMA対応
 									syncSendMsgExecHDD()の引数に「バックアップ/リストア識別」を渡す.
	        16/05/23 Shinosaki MAv4 RQ-505 サーバーへバックアップ2の時はHDDスリープ移行を抑制する.
	        16/12/21 日立田中  MAv5 RQ-668 SIMでバックアップ/リストアを動作させるための修正を追加.
================================================================================================*/
void CP_HDDTrans::execBackup( uintptr_t ul_Option )
{
	ST_MsgHDDBackupRestoreReq*	pst_MsgHDDBackupRestoreReq = (ST_MsgHDDBackupRestoreReq*)ul_Option;
	if( pst_MsgHDDBackupRestoreReq==NULL ) {
		//ありえないが・・・
		THRL_ThLogMsgErr("Backup Backup Message Parameter NG!!\n");
		DebugForever( CF062_DebugFatal ); 	// コマンド異常とする 
		return;
	}
	DLONG dl_Size = getPhysicalHDDSize();
	uint32_t ul_TotalSectorNum = (uint32_t)(dl_Size / (DLONG)DEF_SECTOR_SIZE);
	register uint32_t ul_SectorNo = (uint32_t)pst_MsgHDDBackupRestoreReq->sl_StartSec;	
	register uint32_t ul_EndSectorNo = (uint32_t)pst_MsgHDDBackupRestoreReq->sl_EndSec;	
	register uchar* puc_Data = (uchar*)pst_MsgHDDBackupRestoreReq->p_RestoreData;
	register uint32_t ul_SecNum = 256;	// 1回に読み書き行うセクタ数（デフォルト）
	setDebugLowSpeedSeq( 877, (uintptr_t)ul_SectorNo );
	if( us_debug_HDDdispparm > 0 ) {
		printf( "Backup Store Addr[%p] Start Sector[%"PRId32"] End Sector[%"PRId32"] MAX[%"PRId32"]\n", puc_Data, ul_SectorNo, ul_EndSectorNo, ul_TotalSectorNum ); 
	}
	// 引数の正当性チェック(アドレスはNULLではない、開始<終了、開始セクタは最大セクタより小さい)
	if( (puc_Data==NULL) || (ul_SectorNo>ul_EndSectorNo) || (ul_SectorNo>=ul_TotalSectorNum) ) {
		THRL_ThLogMsgErr("Backup Store Parameter NG!!\n");
		// 終了した場合はコールバック関数を呼び出す 
		execBackupCallBackFunc( False );
		return;
	}
	// シーケンスが動作していない事を確認
	if( getHDDSeq() != EM_HDDSeq_Idle ) {
		THRL_ThLogMsgErr("Backup Store Execute NG!!\n");
		// 終了した場合はコールバック関数を呼び出す 
		execBackupCallBackFunc( False );
		return;
	}	// シーケンス実行中は待ち状態とする

	// OS他のアクセス禁止(最初の1回目のみ実行される)
// 2016.05.23 【MAv4】RQ-505[バックアップ機能強化対応] Start.
#if 0
	execHDDAccessSuspend();
#else
	// HDDバックアップ時のみサスペンドさせる
	if( THRD_BackupType_ToHDD == pst_MsgHDDBackupRestoreReq->ss_reqType ){
		execHDDAccessSuspend();
	}else{
		//printf("[%s][%s] No execHDDAccessSuspend\n",__FILE__,__FUNCTION__);
		;
	}
#endif
// 2016.05.23 【MAv4】RQ-505[バックアップ機能強化対応] End.
	// 

// 07/03/19 morikawa SATA2 変更　
	// puc_Dataをarrayにコピーする
	uint32_t ul_TransSize = ( ul_EndSectorNo - ul_SectorNo + 1 ) * DEF_SECTOR_SIZE;	// 転送サイズ.
	uint32_t ul_TransArrayNum = _roundup( ul_TransSize, (uint32_t)THRD_ONEARRAYSIZE );		// 転送アレイ数.
#ifdef DEF_Soc1200	//CQ.505149 HDDバックアップ/リストア不具合対応.
	memset( e_ArrayType, 0, sizeof( e_ArrayType ) );
#endif //#ifdef DEF_Soc1200.
	for( uint32_t ul_ArrayCount=0; ul_ArrayCount<ul_TransArrayNum; ++ul_ArrayCount ) {
		// 使用するアレイアドレスを転送個数分用意する 
		ul_TransArrayAdr[ ul_ArrayCount ] = (uintptr_t)puc_Data;
#ifdef DEF_Soc1200	//CQ.505149 HDDバックアップ/リストア不具合対応.
		e_ArrayType[ul_ArrayCount] = TYPE_ArrayMemoryType_Main; //バックアップ・リストア用バッファ(netsDataBuf)はメインメモリ側.
#endif //#ifdef DEF_Soc1200.
		puc_Data += THRD_ONEARRAYSIZE;
	}
	uint32_t ul_OneTransArrayNum = 256 * DEF_SECTOR_SIZE / THRD_ONEARRAYSIZE;
	// IV設定　
	setIVNum( ul_SectorNo, ul_TransArrayNum, ul_OneTransArrayNum );
	
//////////////////////////////////////////////////////
// HDDに転送すべきモードを決定
//////////////////////////////////////////////////////
	THRE_ReadWriteHDDMode e_ReadWriteHDDMode = THRE_ReadWriteHDDToBuf;
	// 念のため転送先をキャッシュフラッシュしておく 
	THRL_cacheFlush( pst_MsgHDDBackupRestoreReq->p_RestoreData, ul_TransArrayNum * THRD_ONEARRAYSIZE );

// 暗号化ありのHDD読み出しはAES処理タスクへメッセージ必要　
// AES処理タスクと同期するために、最初に全てのアドレスを渡す必要がある　
// 同期は２５６セクタ単位で行なう　
#ifdef DEF_Soc1200	// CQ.505149 HDDバックアップ/リストア不具合対応.
#ifdef DEF_A3A4COMM
	syncSendMsgExecHDD( THRE_EncHDDBufToArray,
						ul_TransArrayAdr,
						False,		// アレイアドレスは複数個から成る
						ul_TransArrayNum,
						ul_OneTransArrayNum,
						0,False,NULL,0,
						True,e_ArrayType ,true);	// 不連続領域から連続領域（偶数アドレス）へのデータ変換.
#else	// #ifdef DEF_A3A4COMM.
	syncSendMsgExecHDD( THRE_EncHDDBufToArray,
						ul_TransArrayAdr,
						False,		// アレイアドレスは複数個から成る
						ul_TransArrayNum,
						ul_OneTransArrayNum,
						0,False,NULL,0,
						True,e_ArrayType );	// 不連続領域から連続領域（偶数アドレス）へのデータ変換.
#endif	// #ifdef DEF_A3A4COMM.
#else //#ifdef DEF_Soc1200.
	syncSendMsgExecHDD( THRE_EncHDDBufToArray,
						ul_TransArrayAdr,
						False,		// アレイアドレスは複数個から成る
						ul_TransArrayNum,
						ul_OneTransArrayNum,
						0,False,NULL,0,
						True );	// 不連続領域から連続領域（偶数アドレス）へのデータ変換.
#endif //#ifdef DEF_Soc1200.
	
	// 最終が最大サイズを超える場合は丸めて実行
	if( ul_EndSectorNo >= ul_TotalSectorNum ) {
		ul_EndSectorNo = ul_TotalSectorNum - 1;
	}	
	
	uint32_t ul_EndArrayNum = 0;	// 転送終了したアレイ数初期化
	
	// 開始セクタ→終了セクタまで（通常256セクタ単位で行う、最終の半端な分はそのサイズ）
	while( ul_SectorNo <= ul_EndSectorNo ) {
		setDebugHddSeq(301, 0);	// 2005/10/07 modified by t.sasaki
		// 残りが256より小さいの個数なら読み書きセクタ数変更
		if( (ul_EndSectorNo-ul_SectorNo+1) < 256 ) {
			ul_SecNum = ul_EndSectorNo - ul_SectorNo + 1;
		}
		// 暗号化ありの時のバッファアドレス獲得　
		uintptr_t ul_MemAdr = c_HDDBuf.getWriteAdr();	// HDDからReadしたデータを暗号化バッファに書き込み 
		
		// 暗号化ありを考慮したスタート前処理実行
		syncStartOneExecHDD( True ); // 不連続領域から連続領域（偶数アドレス）へのデータ変換モードで.
		
		// キャッシュフラッシュはHDD転送実行関数で行う　
		// HDD転送実行
// 2009/04/05   takayama CQ:316760 SSD常時接続対応.
#ifdef DEF_SupportSSD
		readWriteExecHDD( c_DevicePath, e_ReadWriteHDDMode, &ul_TransArrayAdr[ul_EndArrayNum], ul_MemAdr, ul_SectorNo, ul_SecNum );
#else	// DEF_SupportSSD.
		readWriteExecHDD( e_ReadWriteHDDMode, &ul_TransArrayAdr[ul_EndArrayNum], ul_MemAdr, ul_SectorNo, ul_SecNum );
#endif	// DEF_SupportSSD.

#ifdef SIMULATOR
		// SIMでバックアップ/リストアを動作させるための修正.
		memcpy( (uchar*)ul_TransArrayAdr[ul_EndArrayNum], (uchar*)ul_MemAdr, ul_SecNum * DEF_SECTOR_SIZE );
#endif    /* #ifdef SIMULATOR */

		// 暗号化ありを考慮した終了後処理実行
		syncEndOneExecHDD( True ); // 不連続領域から連続領域（偶数アドレス）へのデータ変換モードで.
		
		// 次の書き込みセクタと読み出しメモリアドレス設定
		ul_SectorNo += ul_SecNum;
		
		// 転送完了したアレイ数
		ul_EndArrayNum += ul_SecNum * DEF_SECTOR_SIZE / THRD_ONEARRAYSIZE;
	}

	// 暗号化ありを考慮した全終了後処理実行
	syncEndAllExecHDD( THRE_ReadWriteHDDToBuf, True ); // 第2引数には、不連続領域から連続領域（偶数アドレス）へのデータ変換モードで.
	// UV-01224機能以外はAPIを呼び出してもHDDアクセス許可にはしない.
// 2016.05.23 【MAv4】RQ-505[バックアップ機能強化対応] Start.
#if 0
	execHDDAccessResume();
#else
	if( THRD_BackupType_ToHDD == pst_MsgHDDBackupRestoreReq->ss_reqType ){
		execHDDAccessResume();
	}else{
		// サーバへのバックアップの場合は、キャンセル機能に対応するためHDDアクセスを許可する.
		//printf("[%s][%s] No execHDDAccessResume\n",__FILE__,__FUNCTION__);
		;
	}
#endif
// 2016.05.23 【MAv4】RQ-505[バックアップ機能強化対応] End.
	// 終了した場合はコールバック関数を呼び出す 
// 2016.05.23 【MAv4】RQ-505[バックアップ機能強化対応] Start.
	execBackupCallBackFunc( True );
// 2016.05.23 【MAv4】RQ-505[バックアップ機能強化対応] End.
}

// ================================================================================================
//! <!--機能--> 【ＩＦ】Fatサイズ取得 
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param THRE_BackupType e_BackupType ... バックアップする種類 
//! <!--戻値-->
//! \return 引数で指定されたものに対応するFat情報のサイズを返す。
//! <!--詳細-->
//! 本関数にて得られたサイズだけ、Fatをバックアップ・リストアして頂く。\n 
//! <!--履歴-->
//  07/07/24 yokoi	 新規作成
//  12/11/21 ADS川口 CQ.377856  MS-LCL_3 CC_Fatのメンバを動的確保に変更したため動的確保領域のサイズを含めて計算する.
//  13/02/14 Takatsuji	FA14のアサート文字列のリファイン.
// ================================================================================================
uint32_t CP_HDDTrans::getFatSize( THRE_BackupType e_BackupType ) const
{
	uint32_t ul_Ans = 0;
	CC_Fat* pcc_Fat = NULL;
	IMGC_ImageMemoryPoolImpColorFileMemoryInstance& comMemoryManager = IMGC_ImageMemoryPoolImpColorFileMemorySingletonIf::newInstance();

	switch( e_BackupType ){
	case THRE_BackupTypeImage:
		pcc_Fat = comMemoryManager.getFatPtr( IMGE_IPEType_HDDStore );
		THRL_ThAssert( (pcc_Fat != NULL), "pcc_Fat is NULL", 0 , 0, 0, 0, CFA14_DetailCode0x0B4F );
		ul_Ans = pcc_Fat->getFatSize();
		break;
	case THRE_BackupTypeInfo:
		pcc_Fat = comMemoryManager.getInfoFatPtr( IMGE_IPEType_HDDStore );
		THRL_ThAssert( (pcc_Fat != NULL), "pcc_Fat is NULL", 0 , 0, 0, 0, CFA14_DetailCode0x0B50 );
		ul_Ans = pcc_Fat->getFatSize();
		break;
	case THRE_BackupTypeImage_Twain:	// Twainからの要求の場合動的確保対応前のサイズを返す.
		pcc_Fat = comMemoryManager.getFatPtr( IMGE_IPEType_HDDStore );
		THRL_ThAssert( (pcc_Fat != NULL), "pcc_Fat is NULL", 0 , 0, 0, 0, CFA14_DetailCode0x0B51 );
		ul_Ans = pcc_Fat->getOldFatSize();
		break;
	case THRE_BackupTypeInfo_Twain:		// Twainからの要求の場合動的確保対応前のサイズを返す.
		pcc_Fat = comMemoryManager.getInfoFatPtr( IMGE_IPEType_HDDStore );
		THRL_ThAssert( (pcc_Fat != NULL), "pcc_Fat is NULL", 0 , 0, 0, 0, CFA14_DetailCode0x0B52 );
		ul_Ans = pcc_Fat->getOldFatSize();
		break;
	default:
		THRL_ThAssert( False, "CP_HDDTrans::getFatSize() BackupTypeErr %d", e_BackupType , 0, 0, 0, CFA14_DetailCode0x01F2 );
		break;
	}

	return ( ul_Ans );
}

// ================================================================================================
//! <!--機能--> 【ＩＦ】Fatサイズ取得(IT4).
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param THRE_BackupType e_BackupType ... バックアップする種類 
//! <!--戻値-->
//! \return 引数で指定されたものに対応するFat情報のサイズ(IT4)を返す。
//! <!--詳細-->
//! 本関数にて得られたサイズだけ、Fatをバックアップ・リストアして頂く。\n 
//! <!--履歴-->
// ================================================================================================
uint32_t CP_HDDTrans::getFatSizeIT4( THRE_BackupType e_BackupType ) const
{
	uint32_t ul_Ans = 0;
	CC_Fat* pcc_Fat = NULL;
	IMGC_ImageMemoryPoolImpColorFileMemoryInstance& comMemoryManager = IMGC_ImageMemoryPoolImpColorFileMemorySingletonIf::newInstance();

	// IT4のFATサイズは、種別に関係なく、固定サイズとなる。
	pcc_Fat = comMemoryManager.getFatPtr( IMGE_IPEType_HDDStore );	// 蓄積画像FAT.
	if ( NULL != pcc_Fat ) {
		ul_Ans = pcc_Fat->getFatSizeIT4();
	} else {
		printf("[CP_HDDTrans::getFatSizeIT4] pcc_Fat == NULL!\n");
	}

	return ul_Ans;
}

// ================================================================================================
//! <!--機能--> 【ＩＦ】Fatのバックアップ 
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param THRE_BackupType e_BackupType ... バックアップする種類 
//! \param void* p_BackupArea ............. バックアップ先の領域 
//! \param ulong ul_Size .................. バックアップするサイズ 
//! \param ulong ul_Offset ................ 分割してバックアップする場合のオフセット値 
//! \param void (*ans_BackupHDD) .......... バックアップ完了時のコールバック関数 
//! <!--戻値-->
//! \return なし
//! <!--詳細-->
//! Fat情報をバックアップするAPI で、分割も可。															\n 
//! たとえば、Fatのサイズが1.6Mbyteであった場合で、1Mbyteしか領域を確保できない場合、					\n 
//! 本関数を2度コール頂き、1回目はul_Size = 1*1024*1024, ul_Offset = 0 とし、							\n 
//! 2回目は ul_Size = 600*1024, ul_Offset = 1*1024*1024 と設定すれば良い。								\n 
//! p_ BackupArea の領域に対して ul_Size分バックアップが完了すれば、ans_BackupHDD をコールする。		\n 
//! このコールバック関数の引数として、バックアップが成功したかどうかの bl_Ok と言うフラグを設けてある。	\n 
//! <!--履歴-->
//  07/07/24 yokoi	 新規作成
//  12/11/21 ADS川口 CQ.377856  MS-LCL_3 CC_Fat動的確保対応.memcpyでのコピーをwriteBackupOldBuffer()/writeBackupBuffer()に変更.
//  13/02/14 Takatsuji	FA14のアサート文字列のリファイン.
// ================================================================================================
void CP_HDDTrans::backupFat( THRE_BackupType e_BackupType, void* p_BackupArea, uint32_t ul_Size, uint32_t ul_Offset, void (*ans_BackupHDD)(Bool bl_Ok) )
{
	uint32_t ul_FatSize = 0;
	
	//2009/03/30 newInstance対応 comMemoryManagerのインスタンス生成.
	IMGC_ImageMemoryPoolImpColorFileMemoryInstance& comMemoryManager = IMGC_ImageMemoryPoolImpColorFileMemorySingletonIf::newInstance();

	// 念のため、サイズチェック・NULLチェック 
	ul_FatSize = getFatSize( e_BackupType );
	THRL_ThAssert( ((ul_Size + ul_Offset) <= ul_FatSize), "CP_HDDTrans::backupFat() sizeErr %"PRIdPTR" + %"PRId32" FatSize %"PRId32"", ul_Size, ul_Offset , ul_FatSize, 0, CFA14_DetailCode0x01F3 );
	THRL_ThAssert( (p_BackupArea != NULL), "CP_HDDTrans::backupFat() AddrErr p_BackupArea is NULL" , 0, 0, 0, 0, CFA14_DetailCode0x01F4 );

	// もしHDD利用中なら、念のため待つ 
	while( (comMemoryManager.cc_HDDQueue.getNumQueue() > 0) || (comHDDEraseStore.getEraseReqNotRequestNum() > 0) ){
		SYSF_taskDelay( 50 );
	}

	// コールバック関数のセット 
	setBackupCallBackFunc( ans_BackupHDD );

	// メモリ上のFat情報をコピー.
	CC_Fat* pcc_Fat = (CC_Fat*)getBackupFatPtr( e_BackupType );
	THRL_ThAssert( (pcc_Fat != NULL), "pcc_Fat is NULL", 0 , 0, 0, 0, CFA14_DetailCode0x0B53 );
	if( e_BackupType == THRE_BackupTypeImage_Twain || e_BackupType == THRE_BackupTypeInfo_Twain )
	{
		// 要求がTwainからの時.
		pcc_Fat->writeBackupOldBuffer( p_BackupArea, ul_Size, ul_Offset );
	}else
	{
		// 要求がTwain以外からの時.
		pcc_Fat->writeBackupBuffer( p_BackupArea, ul_Size, ul_Offset );
	}

	// コールバック関数をコールして完了 
	execBackupCallBackFunc( True );
}

// 19/08/23 [RQ-560]データ分割でのリストア要求時、正常にリストアされない問題に対応 ->.
static void*    p_Pool_RestoreArea      = NULL;
static uint32_t ul_Pool_RestoreAreaSize = 0;
// 19/08/23 [RQ-560]データ分割でのリストア要求時、正常にリストアされない問題に対応 <-.

// 19/09/20 Sky ストレージバックアップ時のPoolサイズ不足に対応 ->.
#define THRD_POOL_RESTORE_AREA_MAXSIZE	( 6 * 1024 * 1024 )			// Poolエリアは6MB確保.
// 19/09/20 Sky ストレージバックアップ時のPoolサイズ不足に対応 <-.

// ================================================================================================
//! <!--機能--> 【ＩＦ】Fatのリストア 
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param THRE_BackupType e_BackupType ... リストアする種類 
//! \param void* p_RestoreArea ............ リストア元の領域 
//! \param ulong ul_Size .................. リストアするサイズ 
//! \param ulong ul_Offset ................ 分割してリストアする場合のオフセット値 
//! \param void (*ans_RestoreHDD) ......... リストア完了時のコールバック関数 
//! <!--戻値-->
//! \return なし
//! <!--詳細-->
//! Fat情報をリストアするAPI で、分割も可。														\n 
//! 詳細はバックアップ時と同じなので(バックアップとリストアの差はあるが)、割愛する。			\n 
//! 但し、Fatのリストアが全て終了した事をスレッドが知る必要があるため、リストアの順序は、		\n 
//! リストア種別の数値として若い方から順にして頂く必要がある。									\n 
//! <!--履歴-->
//  07/07/24 yokoi	 新規作成
//  12/11/21 ADS川口 CQ.377856  MS-LCL_3 CC_Fat動的確保対応.memcpyでのコピーをreadBackupOldBuffer()/readBackupBuffer()に変更.
//  13/02/14 Takatsuji	FA14のアサート文字列のリファイン.
//  16/05/23 Shinosaki MAv4 RQ-505 readBackupBuffer()にリストアかどうかを示す引数を追加.
//  16/12/16 ishiga		[IT5_4.1][HDDデータバックアップ世代間互換].
//  19/04/22 Sky		AR1075858_リストア後フリーズする不具合対応.
//  19/08/23 Sky		[RQ-560]データ分割でのリストア要求時、正常にリストアされない問題に対応.
//  19/09/11 Sky		[RQ-560]CommonAPIからcheckFatSize()が複数回呼ばれる不具合に対応.
//  19/09/20 Sky		[RQ-560]ストレージバックアップ時のPoolサイズ不足に対応.
// ================================================================================================
void CP_HDDTrans::restoreFat( THRE_BackupType e_BackupType, void* p_RestoreArea, uint32_t ul_Size, uint32_t ul_Offset, void (*ans_RestoreHDD)(Bool bl_Ok) )
{
	uint32_t ul_FatSize = 0;
	//2009/03/30 newInstance対応 comMemoryManagerのインスタンス生成.
	IMGC_ImageMemoryPoolImpColorFileMemoryInstance& comMemoryManager = IMGC_ImageMemoryPoolImpColorFileMemorySingletonIf::newInstance();

	THRG_SetHDDRestoreLog( 9, e_BackupType, ul_Size, ul_Offset);

	// 念のため、サイズチェック・NULLチェック 
	if ( True == isIT4Ver_HddRestore() ) {
		ul_FatSize = getFatSizeIT4( e_BackupType );	// IT4からのリストア.
	} else {
		ul_FatSize = getFatSize( e_BackupType );	// IT5からのリストア.
	}
	THRL_ThAssert( ((ul_Size + ul_Offset) <= ul_FatSize), "CP_HDDTrans::restoreFat() sizeErr %d + %d FatSize %d", ul_Size, ul_Offset , ul_FatSize, 0, CFA14_DetailCode0x01F5 );
	THRL_ThAssert( (p_RestoreArea != NULL), "CP_HDDTrans::restoreFat() AddrErr p_RestoreArea is NULL" , 0, 0, 0, 0, CFA14_DetailCode0x01F6 );

	// コールバック関数のセット 
	setBackupCallBackFunc( ans_RestoreHDD );

	// ul_Offsetが0の時は初回なので、内部Pool領域のメモリをHeepに確保.
	if( ul_Offset == 0 ) {
		p_Pool_RestoreArea = malloc( THRD_POOL_RESTORE_AREA_MAXSIZE );
		THRL_ThAssert( (p_Pool_RestoreArea != NULL), "CP_HDDTrans::restoreFat() Pool Area malloc Error" , 0, 0, 0, 0, CFA14_DetailCode0x01F5 );
		memset( p_Pool_RestoreArea, 0x00, THRD_POOL_RESTORE_AREA_MAXSIZE );
		ul_Pool_RestoreAreaSize = 0;
		mb_FatRestoreStartF = True;		// [RQ-560] CommonAPIからcheckFatSize()が複数回呼ばれる不具合に対応.
	}

	// コール元から渡されたOffsetとSizeに従って、内部Pool領域にデータをためる.
	ul_Pool_RestoreAreaSize = ul_Pool_RestoreAreaSize + ul_Size;
	THRL_ThAssert( (ul_Pool_RestoreAreaSize <= THRD_POOL_RESTORE_AREA_MAXSIZE), "CP_HDDTrans::restoreFat() Pool Area Size Overflow!" , 0, 0, 0, 0, CFA14_DetailCode0x01F5 );
	memcpy( (void*)( p_Pool_RestoreArea + ul_Offset ), p_RestoreArea, ul_Size );

	// Sizeが1MB未満の時は最終データなので、中断せずに実際のリストア処理を開始する.
	// 現在の論理上、ul_Sizeが1MBジャスト＋最終データというのはありえないので、考慮外としている.
	// また、ul_Sizeに1MBを超えたサイズを指定されることもありえない.
	if( ul_Size < ( 1024 * 1024 ) ) {
		// Non Op.
		printf("[RQ-560] CP_HDDTrans::restoreFat() FAT area Restore Start!!\n");
	} else {
		// Sizeが1MB以上のときは、分割リストアの途中と判断し、コールバックして中断.
		execBackupCallBackFunc( True );
		printf("[RQ-560] CP_HDDTrans::restoreFat() FAT areea Data Pooled.\n");
		return;
	}

	// もしHDD利用中なら、念のため待つ 
	while( (comMemoryManager.cc_HDDQueue.getNumQueue() > 0) || (comHDDEraseStore.getEraseReqNotRequestNum() > 0) ){
		SYSF_taskDelay( 50 );
	}

	if( (NVDC_SystemSetting::newInstance()->getTempDataOverWriteEraseOn() == True) && (e_BackupType == THRE_BackupTypeImage) ){
		THRG_SetHDDRestoreLog( 15 );
		// 要求を覚えておく 
		p_RestoreAreaBak = p_Pool_RestoreArea;
		ul_RestoreSizeBak = ul_Pool_RestoreAreaSize;
		e_BackupTypeBak = e_BackupType;
		// リストア開始時で、一時データ消去ありなら、まず全て消去する 
		comMemoryManager.initEraseData2( IMGE_IPEType_HDDStore );
	} else {
		Bool bl_Ret = True;
		// メモリ上のFat情報をコピー.
		CC_Fat* pcc_Fat = (CC_Fat*)getBackupFatPtr( e_BackupType );
		THRL_ThAssert( (pcc_Fat != NULL), "pcc_Fat is NULL", 0 , 0, 0, 0, CFA14_DetailCode0x0B54 );
		if( e_BackupType == THRE_BackupTypeImage_Twain || e_BackupType == THRE_BackupTypeInfo_Twain )
		{
			// 要求がTwainからの時.
			pcc_Fat->readBackupOldBuffer( p_Pool_RestoreArea, ul_Pool_RestoreAreaSize, 0 );
		} else if ( True == isIT4Ver_HddRestore() ) {
			// クラスタ数.
			uint32_t ul_ClusterNum = ( THRE_BackupTypeImage == e_BackupType )? DEF_HDD_HDDSTORE_CULSTER_NUM_IT4: DEF_HDD_HDDSTORE_INFO_CULSTER_NUM_IT4;
			bl_Ret = pcc_Fat->readBackupBufferIT4( p_Pool_RestoreArea, ul_Pool_RestoreAreaSize, 0, ul_ClusterNum );
		} else
		{
			// 要求がTwain以外からの時.
// 2016.05.23 【MAv4】RQ-505[バックアップ機能強化対応] Start.
//			pcc_Fat->readBackupBuffer( p_Pool_RestoreArea, ul_Pool_RestoreAreaSize, 0 );
			bl_Ret = pcc_Fat->readBackupBuffer( p_Pool_RestoreArea, ul_Pool_RestoreAreaSize, 0 , True );
// 2016.05.23 【MAv4】RQ-505[バックアップ機能強化対応] End.
// 2021/05/11 Thread Tsukiji OP_BTS-33182 KMフォーマットバックアップリストアできず(世代間リストア).
			switch( e_BackupType ) {
			case THRE_BackupTypeImage:
				//printf( "[DEBUG]FAT CHANGE2 Image\n" );
				pcc_Fat->setStartSectorNo( theHDDTrans.getStorageInfo(THRE_StgInfoStoreStartBlock) );
        		break;
			case THRE_BackupTypeInfo:
				//printf( "[DEBUG]FAT CHANGE2 Info\n" );
				pcc_Fat->setStartSectorNo( theHDDTrans.getStorageInfo(THRE_StgInfoStoreInfoStartBlock) );
        		break;
			default:
				break;
			}
		}

		if( (True == bl_Ret) && ( ul_Pool_RestoreAreaSize == ul_FatSize) ){
			THRG_SetHDDRestoreLog( 16 );
			// Fatのメモリ上へのリストアが完了したので、HDDへもコピーする.
			comMemoryManager.directTransWriteFat( IMGE_IPEType_HDDStore );
		}
// 制御構成変更_No.62 IT不具合修正④.
//		if( (True == bl_Ret) && (e_BackupType == THRE_BackupTypeInfo) && ( ul_Pool_RestoreAreaSize == getFatSize( e_BackupType ) ) ){
		if( (True == bl_Ret) && (e_BackupType == THRE_BackupTypeInfo) && ( ul_Pool_RestoreAreaSize == ul_FatSize ) ){
			THRG_SetHDDRestoreLog( 21 );
//			THRG_SetHDDRestoreLog( 16 );
//			// Fatのメモリ上へのリストアが完了したので、HDDへもコピーする 
//			comMemoryManager.directTransWriteFat( IMGE_IPEType_HDDStore );
		} else {
			// コールバック関数をコールして完了 
			execBackupCallBackFunc( bl_Ret );
		}
	}

	// ここに来たということは、リストア処理が終わったということなので、内部Pool領域(3MB)を開放.
	if( p_Pool_RestoreArea != NULL ) {
		free( p_Pool_RestoreArea );
		p_Pool_RestoreArea = NULL;
		ul_Pool_RestoreAreaSize = 0;
	}

	mb_FatRestoreStartF = False;	// [RQ-560] checkFatSize()が複数回呼ばれる不具合に対応.
}

// ================================================================================================
//! <!--機能--> 【ＩＦ】クラスターサイズ取得 
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param THRE_BackupType e_BackupType ... バックアップする種類 
//! <!--戻値-->
//! \return 引数で指定されたものに対応する領域のクラスターサイズを返す。 
//! <!--詳細-->
//! 1クラスターにつき本関数にて得られたサイズだけ、HDDから画像などをバックアップ・リストアして頂く。\n 
//! <!--履歴-->
//  07/07/24 yokoi	 新規作成
//  08/10/16 takayama CQ:311209 SSD 領域割り当て動的変更対応.
//  13/02/14 Takatsuji	FA14のアサート文字列のリファイン.
// ================================================================================================
uint32_t CP_HDDTrans::getOneClusterSize( THRE_BackupType e_BackupType ) const
{
#ifdef DEF_SupportSSD
	uint32_t ul_Ans = 0;
	switch( e_BackupType ){
	case THRE_BackupTypeImage:
		ul_Ans = ul_CulsterSize;
		break;
	case THRE_BackupTypeInfo:
		ul_Ans = ul_CulsterinfoSize;
		break;
	default:
		THRL_ThAssert( False, "CP_HDDTrans::getOneClusterSize() BackupTypeErr %d", e_BackupType , 0, 0, 0, CFA14_DetailCode0x01F7 );
		break;
	}
	return ( ul_Ans );
#else //DEF_SupportSSD.
	uint32_t ul_Ans = 0;
	switch( e_BackupType ){
	case THRE_BackupTypeImage:
		ul_Ans = DEF_CLUSTER_SIZE;
		break;
	case THRE_BackupTypeInfo:
		ul_Ans = DEF_CLUSTER_INFO_SIZE;
		break;
	default:
		THRL_ThAssert( False, "CP_HDDTrans::getOneClusterSize() BackupTypeErr %d", e_BackupType , 0, 0, 0, CFA14_DetailCode0x01F8 );
		break;
	}
	return ( ul_Ans );
#endif	// DEF_SupportSSD.
}

// ================================================================================================
//! <!--機能--> 【ＩＦ】バックアップ・リストアするクラスター数取得 
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param THRE_BackupType e_BackupType ... バックアップする種類 
//! <!--戻値-->
//! \return 引数で指定されたものに対応する領域のクラスター個数を返す。 
//! <!--詳細-->
//! 本関数にて得られた個数だけ、HDDから画像などのクラスターをバックアップ・リストアして頂く。 			\n 
//! クラスター個数はFat情報より算出するので、リストアする場合には、Fat情報をリストアしてから本関数を 	\n 
//! コールする様に注意しなければならない。																\n 
//! <!--履歴-->
//  07/07/24 yokoi	 新規作成
//  12/11/21 ADS川口 CQ.377856  MS-LCL_3 pul_BackupClusterIdxの名前変更に伴う修正.同メンバのアサートを追加.
//  13/02/14 Takatsuji	FA14のアサート文字列のリファイン.
//  19/07/12 Sky [RQ-560]DEF_MAX_FATにtheHDDTrans.getStorageInfo( THRE_StgInfoStoreClsNum )置き換え.
//  19/09/12 Sky [RQ-560]μSD8GB装着時、ui32t_MAX_FATが0になる問題に対応.
// ================================================================================================
uint32_t CP_HDDTrans::getTotalClusterNum( THRE_BackupType e_BackupType )
{
	// Fatのポインタ取得 
	CC_Fat* pc_Fat = (CC_Fat*)getBackupFatPtr( e_BackupType );

	uint32_t ui32t_MAX_FAT = theHDDTrans.getStorageInfo( THRE_StgInfoStoreClsNum );
	// 0の場合はDEF_MAX_FATを今まで通り使用する(μSD8GBのときにあり得る).
	if( ui32t_MAX_FAT == 0 ) {
		ui32t_MAX_FAT = DEF_MAX_FAT;
	}

	// 有効なクラスターを数えると同時に、どこに入っているかも記憶しておく 
	uint32_t ul_TotalNum = pc_Fat->getMaxClusterNum();
	THRL_ThAssert( (ul_TotalNum <= ui32t_MAX_FAT), "CP_HDDTrans::getTotalClusterNum() TotalNumErr %"PRId32"", ul_TotalNum , 0, 0, 0, CFA14_DetailCode0x01F9 );
	THRL_ThAssert( (pul_BackupClusterIdx[e_BackupType] != NULL), "pul_BackupClusterIdx[%d] is NULL", e_BackupType , 0, 0, 0, CFA14_DetailCode0x0B5A );
	uint32_t ul_ClusterCnt = 0;
	for( uint32_t ul_ClusterIdx = 0; ul_ClusterIdx < ul_TotalNum; ul_ClusterIdx++ ){
		if( pc_Fat->isFreeCluster( ul_ClusterIdx ) == False ){
			pul_BackupClusterIdx[e_BackupType][ul_ClusterCnt] = ul_ClusterIdx;
			ul_ClusterCnt++;
		}
	}

	return ( ul_ClusterCnt );
}

// ================================================================================================
//! <!--機能--> 【ＩＦ】クラスタ単位でHDDから取り出す 
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param THRE_BackupType e_BackupType ... バックアップする種類 
//! \param void* p_BackupArea ............. バックアップ先の領域 
//! \param ulong ul_ClusterIdx ............ バックアップするクラスターのインデックス 
//! \param void (*ans_BackupHDD) .......... バックアップ完了時のコールバック関数 
//! \param uchar uc_backupType ............ バックアップタイプ(=0:HDD/=1:Server)
//! \param Bool b_Direct .................. クラスタダイレクト指定(=True:ul_ClusterIdxを直接クラスタ番号として使用) 
//! <!--戻値-->
//! \return なし
//! <!--詳細-->
//! 画像などをHDDよりバックアップするAPI で、分割は不可。												\n 
//! p_BackupAreaの領域に対して 1クラスター分バックアップが完了すれば、ans_BackupHDD をコールする。		\n 
//! このコールバック関数の引数として、バックアップが成功したかどうかの bl_Ok と言うフラグを設けてある。	\n 
//! ul_ClusterIdx は0始まりで、getBackupClusterNum() で取得したクラスター数以上を指定してはならない。	\n 
//! <!--履歴-->
//  07/07/24 yokoi	 新規作成 
//  07/10/09 yokoi	 クラスタ計算を、クラスタ管理情報から行っていたのを修正 
//	08/08/27 kurachi Donau CQ:307542対策.
//						クラスタ単位で転送するため、1MB分の開始セクタと終了セクタを指定するが、.
//						1セクタ多く指定していた。.
//						終了セクタから1セクタマイナスするように修正.
//  08/10/16 takayama CQ:311209 SSD 領域割り当て動的変更対応.
//  12/11/21 ADS川口 CQ.377856  MS-LCL_3 pul_BackupClusterIdxの名前変更に伴う修正.同メンバのアサートを追加.
//  12/12/18 ADS川口 CQ.377856  MS-LCL_3 pul_BackupClusterIdxの動的確保化に伴い配列最大数が変わるためアサート条件を修正.
//										 (ul_ClusterIdx < DEF_MAX_FAT) → (ul_ClusterIdx < pc_Fat->getMaxClusterNum()).
//  13/02/14 Takatsuji	FA14のアサート文字列のリファイン.
//  16/05/23 Shinosaki MAv4 RQ-505 バックアップ機能強化対応.
//					 	クラスタインデックスをダイレクト指定できるように修正.
// ================================================================================================
// 2016.05.23 【MAv4】RQ-505[バックアップ機能強化対応] Start.
#if 0
void CP_HDDTrans::backupCluster( THRE_BackupType e_BackupType, void* p_BackupArea, uint32_t ul_ClusterIdx, void (*ans_BackupHDD)(Bool bl_Ok) )
#else
void CP_HDDTrans::backupCluster( THRE_BackupType e_BackupType, void* p_BackupArea, uint32_t ul_ClusterIdx, void (*ans_BackupHDD)(Bool bl_Ok),uchar uc_backupType, Bool b_Direct )
#endif
// 2016.05.23 【MAv4】RQ-505[バックアップ機能強化対応] End.
{
	// Fatのポインタ取得 
	CC_Fat* pc_Fat = (CC_Fat*)getBackupFatPtr( e_BackupType );

	THRL_ThAssert( (pc_Fat != NULL), "CC_Fat::init not called. pc_Fat is NULL!!", 0 , 0, 0, 0, CFA14_DetailCode0x0B7E );
	THRL_ThAssert( (ul_ClusterIdx < pc_Fat->getMaxClusterNum()), "CP_HDDTrans::backupCluster() IdxErr ul_ClusterIdx=%d MaxClusterNum=%d e_BackupType=%d", ul_ClusterIdx , pc_Fat->getMaxClusterNum(), (uint32_t)e_BackupType, 0, CFA14_DetailCode0x01FA );
// 2016.05.23 【MAv4】RQ-505[バックアップ機能強化対応] Start.
#if 0
	THRL_ThAssert( (pul_BackupClusterIdx[e_BackupType] != NULL), "pul_BackupClusterIdx[%d] is NULL", e_BackupType , 0, 0, 0, CFA14_DetailCode0x0B5B );
	THRL_ThAssert( (pc_Fat->isFreeCluster(pul_BackupClusterIdx[e_BackupType][ul_ClusterIdx]) == False), "backupCluster IdxError! ul_ClusterIdx=%"PRId32" e_BackupType=%d", ul_ClusterIdx , (uintptr_t)e_BackupType, 0, 0, CFA14_DetailCode0x0B3B );
	// クラスタのインデックスから、セクタ番号を算出する 
	uint32_t ul_StartSec = pc_Fat->calcClusterToSector( pul_BackupClusterIdx[e_BackupType][ul_ClusterIdx] );
	uint32_t ul_SecterCluster = 0;	// 1クラスタにおけるセクタ数 
#ifdef DEF_SupportSSD
	if( e_BackupType == THRE_BackupTypeImage ){
		ul_SecterCluster = ul_CulsterSize/DEF_SECTOR_SIZE;
	} else {
		ul_SecterCluster = ul_CulsterinfoSize/DEF_SECTOR_SIZE;
	}
#else //DEF_SupportSSD
	if( e_BackupType == THRE_BackupTypeImage ){
		ul_SecterCluster = DEF_SECTOR_CLUSTER;
	} else {
		ul_SecterCluster = DEF_SECTOR_CLUSTER_INFO;
	}
#endif	// DEF_SupportSSD.

	// 1クラスタ分、バックアップ指示 
	backup_Store( p_BackupArea, ul_StartSec, ul_StartSec + ul_SecterCluster - 1, ans_BackupHDD );


#else

	uint32_t	ul_ClusterNum = 0;

	if( False == b_Direct ) {
		// ------------------------.
		// クラスタインデックス指定.
		// ------------------------.
		if( THRE_BackupTypeBackup != e_BackupType ){
			// バックアップ領域以外の場合は指定された領域のクラスタインデックスから該当するクラスタ番号を抽出する.
			THRL_ThAssert( (pul_BackupClusterIdx[e_BackupType] != NULL), "pul_BackupClusterIdx[%d] is NULL", e_BackupType , 0, 0, 0, CFA14_DetailCode0x0B5B );
			ul_ClusterNum = pul_BackupClusterIdx[e_BackupType][ul_ClusterIdx];
			// インデックス指定の場合、未使用クラスタの指定は不可.
			THRL_ThAssert( (pc_Fat->isFreeCluster(ul_ClusterNum) == False), "backupCluster IdxError! ul_ClusterIdx=%d e_BackupType=%d", ul_ClusterIdx , (uint32_t)e_BackupType, 0, 0, CFA14_DetailCode0x0B3B );
		}
		else{
			// バックアップ領域を指定された場合は画像領域のクラスタインデックスを参照する.
			THRL_ThAssert( (pul_BackupClusterIdx[THRE_BackupTypeImage] != NULL), "pul_BackupClusterIdx[%d] is NULL",  e_BackupType, 0, 0, 0, CFA14_DetailCode0x0B5B );
			ul_ClusterNum = pul_BackupClusterIdx[THRE_BackupTypeImage][ul_ClusterIdx];
		}
	}
	else {
		// --------------------------.
		// クラスタ番号ダイレクト指定.
		// --------------------------.
		// 指定されたクラスタインデックスをクラスタ番号として使用する.
		ul_ClusterNum = ul_ClusterIdx;
	}

	// バックアップ指示パラメータ設定.
	uint32_t	ul_StartSec = pc_Fat->calcClusterToSector( ul_ClusterNum );		// 抽出したクラスタ番号をセクタ番号に変換する.
	uint32_t	ul_SecterCluster = pc_Fat->getSectorNumPerCluster();			// 1クラスタあたりのセクタ数を取得する.


	// 1クラスタ分、バックアップ指示. 
	backup_Store( p_BackupArea, ul_StartSec, ul_StartSec + ul_SecterCluster - 1, ans_BackupHDD, uc_backupType );
// 2016.05.23 【MAv4】RQ-505[バックアップ機能強化対応] End.
#endif
}

// ================================================================================================
//! <!--機能--> 【ＩＦ】クラスタ単位でHDDへ戻す 
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param THRE_BackupType e_BackupType ... リストアする種類 
//! \param void* p_RestoreArea ............ リストア元の領域 
//! \param ulong ul_ClusterIdx ............ リストアするクラスターのインデックス 
//! \param void (*ans_RestoreHDD .......... リストア完了時のコールバック関数 
//! \param uchar uc_backupType ............ リストアタイプ(=0:HDD/=1:Server)
//! \param Bool b_Direct .................. クラスタダイレクト指定(=True:ul_ClusterIdxを直接クラスタ番号として使用) 
//! <!--戻値-->
//! \return なし
//! <!--詳細-->
//! 画像などをHDDへリストアするAPI で、分割は不可。										\n 
//! 詳細はバックアップ時と同じなので(バックアップとリストアの差はあるが)、割愛する。	\n 
//! <!--履歴-->
//  07/07/24 yokoi	 新規作成
//  07/10/09 yokoi	 クラスタ計算を、クラスタ管理情報から行っていたのを修正 
//	08/08/27 kurachi Donau CQ:307542対策.
//						クラスタ単位で転送するため、1MB分の開始セクタと終了セクタを指定するが、.
//						1セクタ多く指定していた。.
//						終了セクタから1セクタマイナスするように修正.
//  08/10/16 takayama CQ:311209 SSD 領域割り当て動的変更対応.
//  12/11/21 ADS川口 CQ.377856  MS-LCL_3 pul_BackupClusterIdxの名前変更に伴う修正.同メンバのアサートを追加.
//  12/12/18 ADS川口 CQ.377856  MS-LCL_3 pul_BackupClusterIdxの動的確保化に伴い配列最大数が変わるためアサート条件を修正.
//										 (ul_ClusterIdx < DEF_MAX_FAT) → (ul_ClusterIdx < pc_Fat->getMaxClusterNum()).
//  13/02/14 Takatsuji	FA14のアサート文字列のリファイン.
//  16/05/23 Shinosaki MAv4 RQ-505 バックアップ機能強化対応.
//						クラスタインデックスをダイレクト指定できるように修正.
//  16/12/16 ishiga		[IT5_4.1][HDDデータバックアップ世代間互換].
// ================================================================================================
// 2016.05.23 【MAv4】RQ-505[バックアップ機能強化対応] Start.
#if 0
void CP_HDDTrans::restoreCluster( THRE_BackupType e_BackupType, void* p_RestoreArea, uint32_t ul_ClusterIdx, void (*ans_RestoreHDD)(Bool bl_Ok) )
#else
void CP_HDDTrans::restoreCluster( THRE_BackupType e_BackupType, void* p_RestoreArea, uint32_t ul_ClusterIdx, void (*ans_RestoreHDD)(Bool bl_Ok), uchar uc_backupType, Bool b_Direct )
#endif
// 2016.05.23 【MAv4】RQ-505[バックアップ機能強化対応] End.
{
	// Fatのポインタ取得 
	CC_Fat* pc_Fat = (CC_Fat*)getBackupFatPtr( e_BackupType );

	THRG_SetHDDRestoreLog( 7, ul_ClusterIdx, uc_backupType);

	THRL_ThAssert( (pc_Fat != NULL), "CC_Fat::init not called. pc_Fat is NULL!!", 0 , 0, 0, 0, CFA14_DetailCode0x0B7F );
	THRL_ThAssert( (ul_ClusterIdx < pc_Fat->getMaxClusterNum()), "CP_HDDTrans::restoreCluster IdxErr %d, MaxClusterNum %d", ul_ClusterIdx , pc_Fat->getMaxClusterNum(), 0, 0, CFA14_DetailCode0x01FC );
// 2016.05.23 【MAv4】RQ-505[バックアップ機能強化対応] Start.
#if 0
	THRL_ThAssert( (pul_BackupClusterIdx[e_BackupType] != NULL), "pul_BackupClusterIdx[%d] is NULL", e_BackupType , 0, 0, 0, CFA14_DetailCode0x0B5C );
	THRL_ThAssert( (pc_Fat->isFreeCluster(pul_BackupClusterIdx[e_BackupType][ul_ClusterIdx]) == False), "restoreCluster IdxError! %"PRId32"", ul_ClusterIdx , 0, 0, 0, CFA14_DetailCode0x0B3C );
	// クラスタのインデックスから、セクタ番号を算出する 
	uint32_t ul_StartSec = pc_Fat->calcClusterToSector( pul_BackupClusterIdx[e_BackupType][ul_ClusterIdx] );
	uint32_t ul_SecterCluster = 0;	// 1クラスタにおけるセクタ数 
// ◇ 2008/10/16 takayama CQ:311209 SSD 領域割り当て動的変更対応.
#ifdef DEF_SupportSSD
	if( e_BackupType == THRE_BackupTypeImage ){
		ul_SecterCluster = ul_CulsterSize/DEF_SECTOR_SIZE;
	} else {
		ul_SecterCluster = ul_CulsterinfoSize/DEF_SECTOR_SIZE;
	}
#else //DEF_SupportSSD.
	if( e_BackupType == THRE_BackupTypeImage ){
		ul_SecterCluster = DEF_SECTOR_CLUSTER;
	} else {
		ul_SecterCluster = DEF_SECTOR_CLUSTER_INFO;
	}
#endif	// DEF_SupportSSD.

	// 1クラスタ分、リストア指示 
	backup_Restore( p_RestoreArea, ul_StartSec, ul_StartSec + ul_SecterCluster - 1, ans_RestoreHDD );

#else

	uint32_t	ul_ClusterNum = 0;

	if( False == b_Direct ) {
		// ------------------------.
		// クラスタインデックス指定.
		// ------------------------.
		if( THRE_BackupTypeBackup != e_BackupType ){
			// バックアップ領域以外の場合は指定された領域のクラスタインデックスから該当するクラスタ番号を抽出する.
			THRL_ThAssert( (pul_BackupClusterIdx[e_BackupType] != NULL), "pul_BackupClusterIdx[%d] is NULL", e_BackupType , 0, 0, 0, CFA14_DetailCode0x0B5C );
			ul_ClusterNum = pul_BackupClusterIdx[e_BackupType][ul_ClusterIdx];
			// インデックス指定の場合、未使用クラスタの指定は不可.
			THRL_ThAssert( (pc_Fat->isFreeCluster(ul_ClusterNum) == False), "restoreCluster IdxError! %d", ul_ClusterIdx , 0, 0, 0, CFA14_DetailCode0x0B3C );
		}
		else{
			// バックアップ領域を指定された場合は画像領域のクラスタインデックスを参照する.
			THRL_ThAssert( (pul_BackupClusterIdx[THRE_BackupTypeImage] != NULL), "pul_BackupClusterIdx[%d] is NULL",  e_BackupType, 0, 0, 0, CFA14_DetailCode0x0B5C );
			ul_ClusterNum = pul_BackupClusterIdx[THRE_BackupTypeImage][ul_ClusterIdx];
		}
	}
	else {
		// --------------------------.
		// クラスタ番号ダイレクト指定.
		// --------------------------.
		// 指定されたクラスタインデックスをクラスタ番号として使用する.
		ul_ClusterNum = ul_ClusterIdx;
	}

	// リストア指示パラメータ設定.
	uint32_t	ul_StartSec = pc_Fat->calcClusterToSector( ul_ClusterNum );		// 抽出したクラスタ番号をセクタ番号に変換する.
	uint32_t	ul_SecterCluster = pc_Fat->getSectorNumPerCluster();			// 1クラスタあたりのセクタ数を取得する.

	// HDDへのバックアップ、かつ、ダイレクト指定でない場合.
	if ( (THRD_BackupType_ToHDD == uc_backupType) && (False == b_Direct) ) {
		// 蓄積画像管理かつエンディアン変換が必要なの場合.
		if ( ( THRE_BackupTypeInfo == e_BackupType ) && ( True == isNeedConvertEndian_HddRestore() ) ) {
			// エンディアン変換.
			uint32_t ul_ClusterSize = pc_Fat->getClusterSize();	// クラスタサイズ.
			if ( False == CC_OneImageInfoData::convertEndian( p_RestoreArea, ul_ClusterSize ) ) {
				if ( NULL != ans_RestoreHDD ) {
					THRG_SetHDDRestoreLog( 8, ul_ClusterIdx);
					ans_RestoreHDD( False );	// コールバック（リストア失敗）.
				}
				return;
			}
		}
	}

	// 1クラスタ分、リストア指示. 
	backup_Restore( p_RestoreArea, ul_StartSec, ul_StartSec + ul_SecterCluster - 1, ans_RestoreHDD, uc_backupType );
#endif
// 2016.05.23 【MAv4】RQ-505[バックアップ機能強化対応] End.

}

// ================================================================================================
//! <!--機能--> 【内部】 コールバック関数の登録 
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param void (*ans_RestoreHDD) ... 登録するコールバック関数 
//! <!--戻値-->
//! \return なし
//! <!--詳細-->
//! HDDバックアップ用のメッセージ構造体に、引数のコールバック関数を設定する\n 
//! 内部関数だが、見易さの都合上、publicに定義してある \n 
//! <!--履歴-->
//  07/07/24 yokoi	 新規作成
//  13/02/14 Takatsuji	FA14のアサート文字列のリファイン.
// ================================================================================================
void CP_HDDTrans::setBackupCallBackFunc( void (*ans_RestoreHDD)(Bool bl_Ok) )
{
	THRL_ThAssert( (st_MsgHDDBackupRestoreReq.ans_BackupHDD == NULL), "CP_HDDTrans::setBackupCallBackFunc() Function already set." , 0, 0, 0, 0, CFA14_DetailCode0x01FE );
	st_MsgHDDBackupRestoreReq.ans_BackupHDD = ans_RestoreHDD;
}

// ================================================================================================
//! <!--機能--> 【内部】 コールバック関数の実行・削除 
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param Bool bl_Ok ... バックアップの成否 
//! <!--戻値-->
//! \return なし
//! <!--詳細-->
//! HDDバックアップ用のメッセージ構造体のコールバック関数を実行し、削除する\n 
//! 内部関数だが、見易さの都合上、publicに定義してある \n 
//! <!--履歴-->
//  07/07/24 yokoi	 新規作成
// ================================================================================================
void CP_HDDTrans::execBackupCallBackFunc( Bool bl_Ok )
{
	if( st_MsgHDDBackupRestoreReq.ans_BackupHDD != NULL ){
		THRG_SetHDDRestoreLog( 10, bl_Ok == True ? (intptr_t)"Success" : (intptr_t)"Fail" );
		(st_MsgHDDBackupRestoreReq.ans_BackupHDD)( bl_Ok );	// コールバック関数 
		st_MsgHDDBackupRestoreReq.ans_BackupHDD = NULL;
	}else{
		//printf("[%s][%s][%ld] Call back function Didn't called \n",__FILE__,__FUNCTION__,SYSF_tickGet());
		;
	}
}

// ================================================================================================.
//! <!--機能--> 【内部】 バックアップ用Fat変更コールバック関数の登録.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param void (*ans_FatChange)(Bool bl_Result) ... 登録するコールバック関数.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! バックアップ用のFat変更コールバック関数をメンバ変数設定する.
//! <!--履歴-->.
//! 16/12/19 Sky井柳【IT5_4.0】[AR1007099] 新規作成.
// ================================================================================================.
void CP_HDDTrans::setBackupFatChangeCallBackFunc( void (*ans_FatChange)(Bool bl_Result) )
{
	m_ans_BackupFatChangeCallBackFunc = ans_FatChange;
}

// ================================================================================================.
//! <!--機能--> 【内部】 バックアップ用Fat変更コールバック関数の実行・削除.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param Bool bl_Result ... Fat変更の結果.
//! <!--戻値-->
//! \return なし.
//! <!--詳細-->.
//! バックアップ用のFat変更コールバック関数を実行する\n.
//! <!--履歴-->.
//!  16/12/19 Sky井柳【IT5_4.0】[AR1007099] 新規作成.
// ================================================================================================
void CP_HDDTrans::execBackupFatChangeCallBackFunc( Bool bl_Result )
{
	if( m_ans_BackupFatChangeCallBackFunc != NULL ){
		(*m_ans_BackupFatChangeCallBackFunc)( bl_Result );	// 実行.
		m_ans_BackupFatChangeCallBackFunc = NULL;
	}
}

// ================================================================================================.
//! <!--機能--> 【ＩＦ】クラスタ移動時のバックアップ完了コールバック. 
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param Bool bl_Result ... 処理結果.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! クラスタ移動時のバックアップ（クラスタデータのバッファへの移動）処理に指定するコールバック処理\n.
//! 引数に処理結果が指定されるため、その処理結果をクラスタ移動時のバックアップ終了処理に設定して実行する。\n.
//! <!--履歴-->.
//!  17/01/24 iyanagi AR1012856【IT5_4.0】新規作成.
// ================================================================================================.
void ans_moveClusterBackup( Bool bl_Result )
{
	//クラスタ移動時のバックアップ終了処理を実行.
	theHDDTrans.setMoveClusterBackupEnd( bl_Result );
}

// ================================================================================================.
//! <!--機能--> 【ＩＦ】ラスタ移動時のリストア完了コールバック. 
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param Bool bl_Result ... 処理結果.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! クラスタ移動時のリストア(バッファからクラスタデータへの移動)処理に指定するコールバック処理\n.
//! 引数に処理結果が指定されるため、その処理結果をクラスタ移動時のリストア終了処理に設定して実行する。\n.
//! <!--履歴-->.
//!  17/01/24 iyanagi AR1012856【IT5_4.0】新規作成.
// ================================================================================================.
void ans_moveClusterRestore( Bool bl_Result )
{
	//クラスタ移動時のリストア終了処理を実行.
	theHDDTrans.setMoveClusterRestoreEnd( bl_Result );
}

// ================================================================================================.
//! <!--機能--> クラスタ移動のバックアップ処理開始. 
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param uintptr_t ul_Option ... メッセージ処理のオプション指定（未使用）. 
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! クラスタ移動時バックアップ処理要求の受信によりから実行される\n.
//! <!--履歴-->.
//!  17/01/24 iyanagi AR1012856【IT5_4.0】新規作成.
// ================================================================================================.
void CP_HDDTrans::setMoveClusterBackupStart( uintptr_t ul_Option )
{
	if( m_bl_MvClusterExec == False ){	// 実行中以外は受け付けない.
		THRL_ThLogMsgErr("moveCluster Not moving\n");
		return;
	}
	if( m_ul_MvClusterExecFromStartSec >= m_ul_MvClusterFromEndSec ){// 実行開始位置が、終了位置を上回った場合は、異常.
		THRL_ThLogMsgErr("MoveClusterBackup NG!! StartSec[%lu] > EndSec[%lu]\n", m_ul_MvClusterExecFromStartSec, m_ul_MvClusterFromEndSec);
		sendMessage( TID_HDDTrans, TM_MoveClusterEndReq, (uintptr_t)False );	// クラスタ移動終了処理へ.
		return;
	}

	uint32_t	ul_BackupStartSec = m_ul_MvClusterExecFromStartSec;							// バックアップ開始セクタ.
	uint32_t	ul_BackupEndSec = ul_BackupStartSec + THRD_MvCluster_OneMoveSectorSize -1;	// バックアップ終了セクタ.
								// バックアップ対象として終了セクタを含むため、値としては、「開始位置＋サイズ－１」となる.
	if( ul_BackupEndSec > m_ul_MvClusterFromEndSec ){// 終了値より大きい場合は、終了値にする.
		ul_BackupEndSec = m_ul_MvClusterFromEndSec;
	}

	if( 0 < us_DebugEnable ) {	// デバッグログ出力.
		printf("[setMoveClusterBackupStart] StartSec[%lu] - EndSec[%lu]\n", ul_BackupStartSec, ul_BackupEndSec);
	}

	// バックアップ処理実行.
	backup_Store( m_p_WorkArea, ul_BackupStartSec, ul_BackupEndSec, ans_moveClusterBackup, THRC_BackupType_ToServer );
}

// ================================================================================================.
//! <!--機能--> クラスタ移動のバックアップ終了処理. 
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param Bool bl_Result ... バックアップ処理結果. 
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! クラスタ移動のバックアップ終了コールバックから実行される\n.
//! 次回バックアップ処理の開始位置設定をする.
//! 次のリストア処理を実行する.
//! <!--履歴-->.
//!  17/01/24 iyanagi AR1012856【IT5_4.0】新規作成.
// ================================================================================================.
void CP_HDDTrans::setMoveClusterBackupEnd( Bool bl_Result )
{
	if( m_bl_MvClusterExec == False ){	// 実行中以外は受け付けない.
		THRL_ThLogMsgErr("moveCluster Not moving\n");
		return;
	}
	if( bl_Result == False ){	// 処理失敗時.
		sendMessage( TID_HDDTrans, TM_MoveClusterEndReq, (uintptr_t)False );	// クラスタ移動終了処理へ.
		return;
	}
	// 次回のバックアップ開始位置の更新.
	m_ul_MvClusterExecFromStartSec += THRD_MvCluster_OneMoveSectorSize;		// 1処理単位分のセクタ数を加算.
	
	// リストア処理へ.
	sendMessage( TID_HDDTrans, TM_MoveClusterRestoreReq, 0 );
}

// ================================================================================================.
//! <!--機能--> クラスタ移動のリストア処理開始. 
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param uintptr_t ul_Option ... メッセージ処理のオプション指定（未使用）. 
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! クラスタ移動時バックアップ処理要求の受信によりから実行される\n.
//! <!--履歴-->.
//!  17/01/24 iyanagi AR1012856【IT5_4.0】新規作成.
// ================================================================================================.
void CP_HDDTrans::setMoveClusterRestoreStart( uintptr_t ul_Option )
{
	if( m_bl_MvClusterExec == False ){	// 実行中以外は受け付けない.
		THRL_ThLogMsgErr("moveCluster Not moving\n");
		return;
	}
	if( m_ul_MvClusterExecToStartSec >= m_ul_MvClusterToEndSec ){// 実行開始位置が、終了位置を上回った場合は、異常.
		THRL_ThLogMsgErr("MoveClusterRestore NG!! StartSec[%lu] > EndSec[%lu]\n", m_ul_MvClusterExecFromStartSec, m_ul_MvClusterFromEndSec);
		sendMessage( TID_HDDTrans, TM_MoveClusterEndReq, (uintptr_t)False );	// クラスタ移動終了処理へ.
		return;
	}

	uint32_t	ul_RestoreStartSec = m_ul_MvClusterExecToStartSec;								// リストア開始セクタ.
	uint32_t	ul_RestoreEndSec = ul_RestoreStartSec + THRD_MvCluster_OneMoveSectorSize -1;	// リストア終了セクタ.
								// リストア対象として終了セクタを含むため、値としては、「開始位置＋サイズ－１」となる.
	if( ul_RestoreEndSec > m_ul_MvClusterToEndSec ){// 終了値より大きい場合は、終了値にする.
		ul_RestoreEndSec = m_ul_MvClusterToEndSec;
	}
	if( 0 < us_DebugEnable ) {	// デバッグログ出力.
		printf("[setMoveClusterRestoreStart] StartSec[%lu] - EndSec[%lu]\n", ul_RestoreStartSec, ul_RestoreEndSec);
	}
	
	// リストア処理実行.
	backup_Restore( m_p_WorkArea, ul_RestoreStartSec, ul_RestoreEndSec, ans_moveClusterRestore, THRD_BackupType_ToHDD );
}

// ================================================================================================.
//! <!--機能--> クラスタ移動のリストア終了処理. 
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param Bool bl_Result ... リストア処理結果. 
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! クラスタ移動のリストア終了コールバックから実行される\n.
//! 次回リストア処理の開始位置設定をする.
//! クラスタ移動が完了した場合は、正常終了として終了処理へ処理をまわす.
//! クラスタ移動が完了していない場合は、次のバックアップ処理を実行する.
//! <!--履歴-->.
//!  17/01/24 iyanagi AR1012856【IT5_4.0】新規作成.
// ================================================================================================.
void CP_HDDTrans::setMoveClusterRestoreEnd( Bool bl_Result )
{
	if( m_bl_MvClusterExec == False ){	// 実行中以外は受け付けない.
		THRL_ThLogMsgErr("moveCluster Not moving\n");
		return;
	}
	if( bl_Result == False ){	// 処理失敗時.
		sendMessage( TID_HDDTrans, TM_MoveClusterEndReq, (uintptr_t)False );	// クラスタ移動終了処理へ.
		return;
	}
	// 次回のリストア開始位置の更新.
	m_ul_MvClusterExecToStartSec += THRD_MvCluster_OneMoveSectorSize;		// 1処理単位分のセクタ数を加算.
	
	// 終了判定.
	if( m_ul_MvClusterExecToStartSec >= m_ul_MvClusterToEndSec ){// 実行開始位置が、終了位置を上回った場合は、終了.
		
		// 正常終了として、終了処理へ.
		sendMessage( TID_HDDTrans, TM_MoveClusterEndReq, (uintptr_t)True );
		return;
	}
	// バックアップ処理へ.
	sendMessage( TID_HDDTrans, TM_MoveClusterBackupReq, 0 );
}

// ================================================================================================.
//! <!--機能--> クラスタ移動終了処理. 
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param uintptr_t ul_Option ... 終了時状態：Bool値. 
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! クラスタ移動終了時に呼ばれる\n.
//! <!--履歴-->.
//!  17/01/24 iyanagi AR1012856【IT5_4.0】新規作成.
// ================================================================================================.
void CP_HDDTrans::setMoveClusterEnd( uintptr_t ul_Option )
{
	if( m_bl_MvClusterExec == False ){	// 実行中以外は受け付けない.
		THRL_ThLogMsgErr("moveCluster Not moving\n");
		return;
	}
	Bool bl_Result = (Bool)ul_Option;// 引数の変換.
	
	if( m_ans_MvClusterEndCallBackFunc != NULL ){// 終了コールバック実行.
		m_ans_MvClusterEndCallBackFunc( bl_Result );
		m_ans_MvClusterEndCallBackFunc = NULL;
	}
	m_bl_MvClusterExec = False;// 実行状態を落とす.
	if( 0 < us_DebugEnable ) {	// デバッグログ出力.
		printf("[moveCluster] End.\n");
	}
}

// ================================================================================================.
//! <!--機能--> 【ＩＦ】クラスタデータの移動を行う. 
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param THRE_BackupType e_BackupType ... バックアップ(クラスタ移動)する種類. 
//! \param void* p_WorkArea ............... クラスタデータ移動時の作業領域.
//! \param uint32_t ul_FromClusterIdx ........ 移動元クラスターのインデックス.
//! \param uint32_t ul_ToClusterIdx .......... 移動先クラスターのインデックス.
//! \param void (*ans_moveCluster) ........ クラスタ移動完了時のコールバック関数.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! 1クラスター単位での移動処理が完了すれば、ans_moveCluster をコールする。		\n.
//! このコールバック関数の引数として、バックアップが成功したかどうかの bl_Result と言うフラグを設けてある。	\n.
//! この処理は、バックアップ処理とリストア処理を使用して、クラスターの移動を行う	\n. 
//! <!--履歴-->.
//!  17/01/23 iyanagi AR1012856【IT5_4.0】新規作成.
// ================================================================================================.
void CP_HDDTrans::moveCluster( THRE_BackupType e_BackupType, void* p_WorkArea, uint32_t ul_FromClusterIdx, uint32_t ul_ToClusterIdx, void (*ans_moveCluster)(Bool bl_Result) ){
	
	if( m_bl_MvClusterExec == True ){	// 実行中は受け付けない.
		THRL_ThLogMsgErr("moveCluster already moving\n");
		return;
	}
	m_bl_MvClusterExec = True;// 実行状態にする.
	
	// Fatのポインタ取得 
	CC_Fat* pc_Fat = (CC_Fat*)getBackupFatPtr( e_BackupType );
	THRL_ThAssert( (pc_Fat != NULL), "CC_Fat::init not called. pc_Fat is NULL!!", 0 , 0, 0, 0, CFA14_DetailCode0x103C );

	uint32_t	ul_SecterCluster = pc_Fat->getSectorNumPerCluster();					// 1クラスタあたりのセクタ数を取得する.

	// クラスタ移動処理に使用する値の保持.
	m_ul_MvClusterFromStartSec = pc_Fat->calcClusterToSector( ul_FromClusterIdx );	// 移動元クラスタの開始セクタ.
	m_ul_MvClusterFromEndSec = m_ul_MvClusterFromStartSec + ul_SecterCluster -1;	// 移動元クラスタの終了セクタ.
								// バックアップ対象として終了セクタを含むため、値としては、「開始位置＋サイズ－１」となる.

	m_ul_MvClusterToStartSec = pc_Fat->calcClusterToSector( ul_ToClusterIdx );		// 移動先クラスタの開始セクタ.
	m_ul_MvClusterToEndSec = m_ul_MvClusterToStartSec + ul_SecterCluster -1;		// 移動先クラスタの終了セクタ.
								// リストア対象として終了セクタを含むため、値としては、「開始位置＋サイズ－１」となる.

	m_ul_MvClusterExecFromStartSec = m_ul_MvClusterFromStartSec;					// 実行:移動元クラスタの開始セクタ.
	m_ul_MvClusterExecToStartSec =  m_ul_MvClusterToStartSec;						// 実行:移動先クラスタの開始セクタ.
	m_p_WorkArea = p_WorkArea;														// ワークエリアポインタ.
	m_ans_MvClusterEndCallBackFunc = ans_moveCluster;								// 終了コールバック関数.
	if( 0 < us_DebugEnable ) {	// デバッグログ出力.
		printf("[moveCluster] Start.\n");
		printf("[moveCluster] m_ul_MvClusterFromStartSec[%lu], m_ul_MvClusterFromEndSec[%lu]\n", m_ul_MvClusterFromStartSec, m_ul_MvClusterFromEndSec );
		printf("[moveCluster] m_ul_MvClusterToStartSec  [%lu], m_ul_MvClusterToEndSec  [%lu]\n", m_ul_MvClusterToStartSec, m_ul_MvClusterToEndSec );
	}

	// バックアップ処理へ.
	sendMessage( TID_HDDTrans, TM_MoveClusterBackupReq, 0 );
}

// ================================================================================================
//! <!--機能--> 【内部】 Fatへのアドレス取得 
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param THRE_BackupType e_BackupType ... 取得したいFatの種類 
//! <!--戻値-->
//! \return 指定のFatへのポインタ(ulong値) 
//! <!--詳細-->
//! 指定のFatへのポインタをulong型で取得する \n 
//! 内部関数だが、見易さの都合上、publicに定義してある \n 
//! <!--履歴-->
//  07/07/24 yokoi	 新規作成
//  12/11/21 ADS川口 CQ.377856  MS-LCL_3 Twain判別用種別の分岐を追加.
//  13/02/14 Takatsuji	FA14のアサート文字列のリファイン.
//  16/05/23 Shinosaki  MAv4 RQ-505 一時保存領域の追加.
// ================================================================================================
uintptr_t CP_HDDTrans::getBackupFatPtr( THRE_BackupType e_BackupType )
{
	//2009/03/30 newInstance対応 comMemoryManagerのインスタンス生成.
	IMGC_ImageMemoryPoolImpColorFileMemoryInstance& comMemoryManager = IMGC_ImageMemoryPoolImpColorFileMemorySingletonIf::newInstance();

	uintptr_t ul_Ans = 0;
	switch( e_BackupType ){
	case THRE_BackupTypeImage:
	case THRE_BackupTypeImage_Twain:
		ul_Ans = (uintptr_t)comMemoryManager.getFatPtr( IMGE_IPEType_HDDStore );
		break;
	case THRE_BackupTypeInfo:
	case THRE_BackupTypeInfo_Twain:
		ul_Ans = (uintptr_t)comMemoryManager.getInfoFatPtr( IMGE_IPEType_HDDStore );
		break;
// 2016.05.23 【MAv4】RQ-505[バックアップ機能強化対応] Start.
	case THRE_BackupTypeBackup:
		ul_Ans = (uintptr_t)comMemoryManager.getFatPtr( IMGE_IPEType_HDDBackup );
		break;
// 2016.05.23 【MAv4】RQ-505[バックアップ機能強化対応] End.

	default:
		THRL_ThAssert( False, "CP_HDDTrans::getBackupFatPtr() BackupTypeErr %d", e_BackupType , 0, 0, 0, CFA14_DetailCode0x01FF );
		break;
	}
	return ( ul_Ans );
}

// ================================================================================================
//! <!--機能--> 【ＩＦ】一時データ上書き消去が全て終わった時の通知用 
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param なし
//! <!--戻値-->
//! \return なし
//! <!--詳細-->
//! 一時データ上書き消去が全て終わった時のイベントとして、HDD分割バックアップ処理を行う \n 
//! <!--履歴-->
//  07/07/24 yokoi	 新規作成
//  12/11/21 ADS川口 CQ.377856  MS-LCL_3 CC_Fat動的確保対応.memcpyでのコピーをreadBackupOldBuffer()/readBackupBuffer()に変更.
//  16/12/16 ishiga		[IT5_4.1][HDDデータバックアップ世代間互換].
// ================================================================================================
void CP_HDDTrans::endEraseData()
{
	if( st_MsgHDDBackupRestoreReq.ans_BackupHDD != NULL ){
		Bool bl_Ret = True;

		// メモリ上のFat情報をコピー 
		CC_Fat* pcc_Fat = (CC_Fat*)getBackupFatPtr( (THRE_BackupType)0 );
		THRL_ThAssert( (pcc_Fat != NULL), "pcc_Fat is NULL", 0 , 0, 0, 0, CFA14_DetailCode0x0B55 );
		if( e_BackupTypeBak == THRE_BackupTypeImage_Twain || e_BackupTypeBak == THRE_BackupTypeInfo_Twain )
		{
			pcc_Fat->readBackupOldBuffer( p_RestoreAreaBak, ul_RestoreSizeBak, 0 );	// 一時データ上書き消去が実行されるのはオフセット0の時のみ.ここでのオフセットは必ず0になる.
		} else if ( True == isIT4Ver_HddRestore() ) {	// バックアップデータがIT4である場合.
			// クラスタ数.
			uint32_t ul_ClusterNum = (THRE_BackupTypeImage == e_BackupTypeBak)? DEF_HDD_HDDSTORE_CULSTER_NUM_IT4: DEF_HDD_HDDSTORE_INFO_CULSTER_NUM_IT4;
			bl_Ret = pcc_Fat->readBackupBufferIT4( p_RestoreAreaBak, ul_RestoreSizeBak, 0, ul_ClusterNum );	// 一時データ上書き消去が実行されるのはオフセット0の時のみ.ここでのオフセットは必ず0になる.
		}else
		{
			bl_Ret = pcc_Fat->readBackupBuffer( p_RestoreAreaBak, ul_RestoreSizeBak, 0 );	// 一時データ上書き消去が実行されるのはオフセット0の時のみ.ここでのオフセットは必ず0になる.
		}
		
		p_RestoreAreaBak = NULL;
		ul_RestoreSizeBak = 0;
		e_BackupTypeBak = THRE_BackupTypeNum;
		// コールバック関数をコールして完了 
		execBackupCallBackFunc( bl_Ret );
	}
}

/*================================================================================================
［機能］	装置消去ステージ開始
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	無し
［詳細］	装置消去時にはカウントアップタイミングがないので実行しているように見せかける
 [履歴]     03/06/16 tsukiji 【TSS2】処理追加
 			07/09/27 morikawa Donau初版
 								装置消去実行中はHDD転送タスクは動けないため、表示更新のためのメッセージはイメージタスクへ送信する。
================================================================================================*/
void CP_HDDTrans::startEraceStage()
{
	ul_Percent = 0;
	// 03/06/16 tsukiji 【TSS2】容量に応じたカウントアップ速度
	setDelayMessage( TID_Image, TM_HDDtest, 1, DEF_FORMAT_PERCENT_TIME );
}
/*================================================================================================
［機能］	装置消去ステージ終了
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	無し
［詳細］	９９％以上はカウントしないので停止する
 [履歴]     03/06/16 tsukiji 【TSS2】処理追加
================================================================================================*/
void CP_HDDTrans::endEraceStage()
{
	ul_Percent = 99;	// 強制停止
}
/*================================================================================================
［機能］	転送種別は管理情か？
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	Bool					：True:管理情報 False:画像他
［詳細］	

================================================================================================*/
Bool CP_HDDTrans::isInfoData( uintptr_t ul_Option )
{
	ST_MsgHDDTransInfo*	pst_MsgHDDTransInfo = (ST_MsgHDDTransInfo*)ul_Option;
	switch( pst_MsgHDDTransInfo->em_TransDataKind ) {
		case EM_TransDataKind_Image:
		case EM_TransDataKind_Store:
		case EM_TransDataKind_BackImage:
		case EM_TransDataKind_AppImage:
			return False;
			break;
		case EM_TransDataKind_Info:
			return True;
			break;
		default:
			break;
	}
	return False;
}
/*================================================================================================
［機能］	ＨＤＤＲ／Ｗチェック要求処理
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	無し
［詳細］	ＨＤＤのＲ／Ｗチェックを起動する
［履歴］	02/01/30 morikawa メモリチェックファンクション仕様追加
			02/02/04 morikawa app1の空き容量チェックプログラム追加
			02/02/05 morikawa 02/02/04の追加はVxWorksのみで有効とする
			04/01/19 t.sasaki #ifdef TORNADOが誤って小文字のTornadoになって実機で実行されていなかった不具合修正、クローズ処理追加
			04/06/09 ikawa	不要な処理を削除
			04/07/08 morikawa warning対策
			05/02/18 t.sasaki HDD R/Wチェックで論理フォーマットがかかってしまう不具合対策(DB.33579)
			07/12/18 morikawa Douan デバイス名を/a0/ijcからTHRD_DevName_Hdd01に修正　
						FIONFREEでは３２ビット以上となり以下のエラーとなる
							printErrno 0x380001
							S_dosFsLib_32BIT_OVERFLOW
						FIONFREE64に変更してlong long型で扱うように修正　	
			09/04/21 kurachi Donau3/Amur CQ:317353対策.
						R/WチェックはdosFSのHDDアクセスI/FであるideRW()を用いてRead/Writeする制御に.
						なっている。そのためR/Wチェック時にはdosFSの割り込みを禁止する処理があるが、.
						AES転送用排他セマフォの獲得が抜けていた。この影響でスレッドタスクと他モジュールタスクにて、.
						ideRW()の多重アクセスが発生した場合に、お互いにAESセマフォ待ち＆DOSセマフォ待ちに.
						なってしまいR/Wチェックが完了しない問題が発生した。.
						対策として、R/Wチェック時にAESセマフォに関しても獲得した上でシーケンスを開始させるようにする。.
			10/03/03 yashiro Gaia問題点リストNo286(CQ324193) : セマフォID dosImageMuteSem 関連処理の削除.
			16/11/22	Sky井柳 【IT5_4.0】AR1007320 HDD R/Wチェックの要求にも関わらず、ストレージ領域の対象がHDDではない時は.
							失敗として終了させるようにする(HDD未認識でも、HDD R/Wチェックが実行されるように仕様が変更されたため).
			17/04/12	Sky松井 【IT5_4.0】AR1018857 HDD装着時にのみ、診断対象がeMMCの場合に「NG」に設定する処理を実行するように修正.
================================================================================================*/
void CP_HDDTrans::rwCheckReq()
{
	APIC_SharedSystemDataSetting* pc_SharedSystemDataSetting = APIC_SharedSystemDataSetting::newInstance();
	if( (pc_SharedSystemDataSetting != NULL)
		&& (pc_SharedSystemDataSetting->getHddExist() == True) ){
		// 自己診断では、HDD装着時のみで、HDD R/Wチェックが行われるため.
		// ストレージ診断では、HDD非装着ではeMMCを対象とし、HDD装着時は、HDDを対象として、R/Wチェックする必要があるため.
		if( ul_PortNo != THRE_HddPortNo ){	// ストレージ領域のポート番号がHDDのポート番号ではない時.
			// ＨＤＤＲ／Ｗチェック異常終了.
			CC_MEMFunctionSingletonIf::newInstance().SetMemFuncResult( False );		// 診断結果（総合）を異常指定.
			CC_MEMFunctionSingletonIf::newInstance().setHddRwCheckFuncStatus( TYPE_MemFuncStatusNGEnd );// 「HDD R/Wチェック」実行状態に、異常終了を設定.
			CC_MEMFunctionSingletonIf::newInstance().ss_ExecPercent = 100;			// 診断終了.
			return;
		}
	}
	
	// シーケンスを開始
#ifdef DEF_SupportSSD // CQ.316761 ＤＯＳとの調停（占有）.
	THRC_AesTransSingletonIf::newInstance().semTakeDosDirect( getNvRamDeviceType() );
#else	// DEF_SupportSSD.
	THRC_AesTransSingletonIf::newInstance().semTakeDosDirect();	// AES排他セマフォ獲得.
#endif	// DEF_SupportSSD.
// HDD R/Wチェックで論理フォーマットがかかってしまう不具合対策(DB.33579) 2005/02/18 fixed by t.sasaki
	setHDDSeq( EM_HDDSeq_CheckWrite_1 );
	// シーケンスに応じた処理を連続実行
	stateProcExecHDDSeq();
}
/*================================================================================================
［機能］	乱数クリア(_memsetの代用)
--------------------------------------------------------------------------------------------------
［引数］	void*					：クリアメモリ先頭アドレス
			int						：サイズ(byte)
［戻値］	無し
［詳細］	乱数クリア先にsrand()で乱数ジェネレータ初期化しておくと良い
［履歴］	03/03/26 tsukiji 【TSS2】関数追加
			13/09/12 fukuhara スレッドセーフでない関数の対応 rand -> rand_r.
================================================================================================*/
void CP_HDDTrans::randmemset( void* puc_MemAddr, int32_t size )
{
	register uchar* p1 = (uchar*)puc_MemAddr;
	static uint32_t ui_seedp = 1;	// 「rand() 関数は 自動的に 1 を種とする」.
	/* 乱数は0-0x7fffffff(RAND_MAX)で発生する */
	for( int32_t i=0; i<size; i++ ) {
		*(p1+i) = (uchar)(((double)(rand_r(&ui_seedp)) / RAND_MAX) * 255);
	}
}
/*================================================================================================
［機能］	【内部】警告判定
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	無し
［履歴］	03/04/14 tsukiji 【TSS2】異常では通知しない場合か判定 
================================================================================================*/
Bool CP_HDDTrans::isSecurityError()
{
	return bl_SecurityErrorFlag;
}
/*================================================================================================
［機能］	【内部】警告判定
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	無し
［履歴］	03/04/14 tsukiji 【TSS2】異常では通知しない場合か判定 
================================================================================================*/
Bool CP_HDDTrans::isWarrning()
{
	return bl_WarrnigErrorFlag;
}

/*================================================================================================
［機能］	【内部】NvRam HDD接続設定値からデバイスタイプ取得.
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	HDD接続時:  THRE_StorageDeviceTypeHDDAndSSD.
			HDD未接続時:THRE_StorageDeviceTypeSSD.
［履歴］	09/04/05 CQ.316761 koshimo 新規作成.
			09/06/02 CQ:317995 takayama HDD装着設定変更後にD047発生不具合対応.
										(NVRAMの設定値を毎回チェックせず、.
										初期化時に設定したデバイスタイプで判別するように修正).
================================================================================================*/
#ifdef DEF_SupportSSD
THRE_StorageDeviceType CP_HDDTrans::getNvRamDeviceType()
{
	IMGC_ImageMemoryPoolImpColorFileMemoryInstance& comMemoryManager = IMGC_ImageMemoryPoolImpColorFileMemorySingletonIf::newInstance();
	// 09/06/02 takayama CQ:317995.
	// 本APIでHDD装着設定確認するとHDD装着設定を切り替えられた場合にHDDアクセスすべき処理がSSDに.
	// アクセスする不都合が発生するため、初期化時のHDD装着設定の値でデバイスタイプを返すように修正.
	if( comMemoryManager.getStorageDeviceType() == THRE_StorageDeviceTypeHDD ){
		// HDD接続.
		return	THRE_StorageDeviceTypeHDDAndSSD;
	}
	else{
		// HDD未接続.
		return	THRE_StorageDeviceTypeSSD;
	}
}
#endif // DEF_SupportSSD.

// HDDRWCheck 
static Bool bl_HDDRWCheckMode = False;	// Treu : RWCheck 2 全領域に書き込んでから読み出してチェック　False : RWCheck1 128KByte単位RW 
void HddRWCheck( int32_t mode = 0 )
{
	bl_HDDRWCheckMode = ( mode == 0 ? False : True );
	theHDDTrans.hddrwCheck();
}
/*================================================================================================
［機能］	【ハンドラ】HDDRWチェックプログラム 
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	無し
［詳細］	
［履歴］	19/08/26 Sky鳥居 [EagleZ+] OP_BTS-11139対応.
================================================================================================*/
void CP_HDDTrans::hddrwCheck()
{
	while( getHDDSeq() != EM_HDDSeq_Idle ) {}	// シーケンス実行中は待ち状態とする
	FuncCallstart( 29, __FUNCTION__, 1 );		// 割込みチェック用関数コールスタート.
	memset( puc_WorkMemAddr, 0, DEF_HDD_FORMATDATA_SIZE );
	FuncCallend( 29, __FUNCTION__, 1 );			// 割込みチェック用関数コールエンド.

	// シーケンスを開始
	setHDDSeq( EM_HDDSeq_RWCheck );
	// 設定されたシーケンスを実行する
	execHDDSeq();
}

/*================================================================================================
［機能］	【内部】ステータスチェック(フォーマット異常)
--------------------------------------------------------------------------------------------------
［引数］	なし
［戻値］	Bool					：True IDチェック異常
 [詳細] 	
［履歴］	05/06/20 t.sasaki Rhein1初版(暗号化ASIC正式対応)
================================================================================================*/
Bool CP_HDDTrans::isCheckFormatError()
{
	return bl_CheckFormatError ;
}

/*================================================================================================
［機能］	【内部】HDD FW領域ステータスチェック(異常)
--------------------------------------------------------------------------------------------------
［引数］	なし
［戻値］	Bool					：True 異常
 [詳細] 	
［履歴］	12/10/01 takayama ZeusMLK HDDのFW領域追加対応.
================================================================================================*/
Bool CP_HDDTrans::isHddFwAreaError()
{
	return bl_HddFwAreaError ;
}

#ifdef DEF_SupportSSD
/*================================================================================================
［機能］	【外部】SSDステータスチェック(フォーマット異常).
--------------------------------------------------------------------------------------------------
［引数］	なし.
［戻値］	Bool					：True エラー発生.
　　　　　　　　　　　　　　　　　　　False 正常.
 [詳細] 	
［履歴］	09/08/02 takayama 新規 CQ:318248  SSDステータス別管理対応.
================================================================================================*/
Bool CP_HDDTrans::isCheckSsdFormatError()
{
	return bl_SsdNgFlg ;
}
#endif		// DEF_SupportSSD.

/*================================================================================================
［機能］	【内部】ステータスチェック(未フォーマット判定)
--------------------------------------------------------------------------------------------------
［引数］	なし
［戻値］	Bool					：True IDチェック異常
 [詳細] 	チェックファイル有無でフォーマット済みかどうかを判定する.
 			暗号化NG時も未フォーマット判定となるので注意する.
［履歴］	05/07/15 t.sasaki Rhein1/2/3初版(未フォーマットHDD検出用)
================================================================================================*/
Bool CP_HDDTrans::isNotFormatted()
{
	return !(bl_ExistCheckFile) ;
}
/*================================================================================================
［機能］	暗号化状態の確認.
［戻値］	Bool					True = 暗号化状態正常 False = 暗号化状態異常.
 [詳細] 	チェックファイル有無でフォーマット済みかどうかを判定する.
 			暗号化NG時も未フォーマット判定となるので注意する.
 			SYSC_SystemStatusから移植.
［履歴］	2016/02/12 Sky合屋 [ZeusS_ZX0]CQ.524976 暗号化状態取得用API 新規追加.
			2016/03/07 Sky山田 CQ525844 暗号化解除後にD010発生問題対応.
			2018/08/31 Sky井柳 [Eagle]制御構成変更No99 MicroSD暗号化対応 HDD装着条件の削除.
			18/09/21 Sky井柳 【DenebMLK】制御構成変更No78「データ移行」対応 MicroSD起動時のみ暗号化対象化修正.
			20/08/08 ADS美土路 SimLinでも実機同等の処理を行うように修正.
================================================================================================*/
Bool CP_HDDTrans::isEncryptionStatus()
{
	Bool bl_EncStatus = True; // デフォルトは暗号化正常.
#if (defined(COSIM) && defined(THR_SIMWIN))
	return bl_EncStatus;
#else
// IT6では、メインストレージの暗号化のみ.
#if 0 // サブストレージ暗号化対応の為に、コードを残しておく.
	APIC_SystemSetting *pc_APIC_SystemSetting = APIC_SystemSetting::newInstance();
	if( pc_APIC_SystemSetting == NULL ){
		THRL_ThLogMsgErr("[%s] APIC_SystemSetting is NULL.\n", __FUNCTION__ );
		return False;
	}
	Bool bl_Encrypt = pc_APIC_SystemSetting->getEnableEncryption();
#else //0.
	Bool bl_Encrypt = THRG_IsSsdEncrypt();
#endif //0.
	if (False == bl_Encrypt ) { // 暗号化設定無効.
		// 未フォーマット且つフォーマットIDが異常の時に暗号化異常とする.
		if ((theHDDTrans.isCheckFormatError() == True) &&	// フォーマットID異常.
			(theHDDTrans.isNotFormatted() == True)) {		// 未フォーマット.
			bl_EncStatus = False ;		// 暗号化異常.
		}
	} else {												// 暗号化設定有効.
		// 未フォーマットまたはフォーマットIDが異常の時に暗号化異常とする.
		if ((theHDDTrans.isCheckFormatError() == True) ||	// フォーマットID異常.
			(theHDDTrans.isNotFormatted() == True)) {		// 未フォーマット.
			bl_EncStatus = False ;		// 暗号化異常.
		}
	}
	return bl_EncStatus;
#endif
}

/*================================================================================================
［機能］	【内部】HDDロックパスワード(ユーザパスワード)設定有無の確認
--------------------------------------------------------------------------------------------------
［引数］	なし
［戻値］	Bool					：True:ロックパスワード設定あり fasle:なし
 [詳細] 	通常のAborted Commandは主にセキュリティ異常のため発生する。
			しかし、チェック用に使用したい場合はエラーとしない
［履歴］	04/07/20 t.sasaki Rhein1初版(セキュリティ対応)
			18/07/25 山岸     [Eagle][制御構成変更 No.99]【ストレージ構成変更】ストレージ管理(フォーマット_ロック_暗号化) パスワード取得API変更.
================================================================================================*/
Bool CP_HDDTrans::isLockPassword()
{
	Bool bl_res = False ;	// 初期値パスワードなしとする
	char sc_Pass[256] ;
	NVDC_SystemSetting *pc_Instance = NVDC_SystemSetting::newInstance();
	if (NULL == pc_Instance) {
		setDebugHddSeq(400, 3); // NULL Check NG.
		return False;
	}
	pc_Instance->getHddLockPassword_SPIFlash( sc_Pass ) ;

	if ( sc_Pass[0] != '\x0' ){
		bl_res = True ;
	}
	return bl_res ;

}

/*================================================================================================
［機能］	一時データ上書き削除要求か？
--------------------------------------------------------------------------------------------------
［戻値］	Bool			：True:一時データ上書き削除 False:通常の転送要求
［詳細］	
［履歴］	04/10/05 t.sasaki Rhein1初版
			05/03/29 t.sasaki 正式実装向けの修正.
================================================================================================*/
Bool CP_HDDTrans::isReqEraseData(
		uintptr_t ul_Option )			// (IN)転送要求メッセージ時のオプション
{
	Bool bl_Result	= False ;	// デフォルト：通常の転送要求
	ST_MsgHDDTransInfo*	pst_MsgHDDTransInfo = (ST_MsgHDDTransInfo*)ul_Option;
	// ガード.
	if ( pst_MsgHDDTransInfo == NULL ) {
		return False ;
	}
	// モードチェック.
	switch( pst_MsgHDDTransInfo->em_HDD_RWMode ) {
		case EM_HDD_RWMode_Erase:	// 一時データ上書き削除
			bl_Result	= True ;
			break ;
		default:	// モード不明
			break ;
	}
	return bl_Result ;
}

/*================================================================================================
［機能］	一時データ上書き消去パターン取得
--------------------------------------------------------------------------------------------------
［戻値］	Bool			：True:一時データ上書き削除 False:通常の転送要求
［詳細］	
［履歴］	05/09/19 t.sasaki Rhein1/2/3初版
================================================================================================*/
Bool CP_HDDTrans::getErasePattern(
		uchar	uc_ReqPtn,		// <IN>  0x00,0xff,0x61を指定 
		void**	ppt_PtnBuf,		// <OUT> 使用可能なパターンの先頭アドレス (NG時はNULL)
		ushort*	pus_Size )		// <OUT> パターンサイズ (NG時は0)
{
	*ppt_PtnBuf	= NULL ;
	*pus_Size	= 0 ;
	Bool bl_Result	= False ;
	ST_ErasePatternTbl* pEPTbl = getErasePatternTbl( uc_ReqPtn ) ;
	if ( pEPTbl != NULL ) {
		*ppt_PtnBuf	= pEPTbl->puc_PtnBuf ;
		*pus_Size	= pEPTbl->us_Size ;
		bl_Result	= True ;
	}
	return bl_Result ;
}

/*================================================================================================
［機能］	パターン比較サブルーチン.
--------------------------------------------------------------------------------------------------
［戻値］	int				：0:相違なし / !0:相違あり
［詳細］	比較対象全領域をバイトパターンで比較する
［履歴］	04/10/05 t.sasaki Rhein1初版
			05/03/29 t.sasaki 正式実装向けの修正.
================================================================================================*/
int32_t ptncmp(
	void* pt_Data,		// 対象データへのポインタ.
	uchar uc_Ptn,		// バイトパターン
	size_t size )		// 対象サイズ.
{
	// ガード.
	if ( pt_Data == NULL ) {
#ifdef TORNADO
		SYSF_taskSuspend( SYSF_taskIdSelf() ) ;
#endif
		return -1 ;
	}

	// 高速処理するために、4バイトずつ比較しているが、
	// さらに高速化する必要がある場合は、ひとつのループで複数データを比較するように変更する。

	// 4バイトずつ比較する.
	register uint32_t	ul_Ptn		= ((((uint32_t)uc_Ptn)<<24)
								 | (((uint32_t)uc_Ptn)<<16)
								 | (((uint32_t)uc_Ptn)<<8)
								 | ((uint32_t)uc_Ptn)) ;
	register uint32_t*	pul_Data	= (uint32_t*)pt_Data ;
	register size_t	less		= size/sizeof(uint32_t) ;
	for ( ; less > 0; less-- ) {
		if ( (*pul_Data) ^ ul_Ptn ) {	// XOR比較(一致していた時のみ0になる).
			return -1 ;	// 比較NG.
		}
		pul_Data++ ;
	}
	less	= size%sizeof(uint32_t) ;
	register uchar* puc_Data	= (uchar*)pul_Data ;
	for ( ; less > 0; less-- ) {
		if ( (*puc_Data) ^ uc_Ptn ) {	// XOR比較(一致していた時のみ0になる).
			return -1 ;	// 比較NG.
		}
		puc_Data++ ;
	}

	return 0 ;	// 比較OK.
}
// S 2005/07/15 t.sasaki SSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSS
/*================================================================================================
［機能］	パターン比較サブルーチン２.
--------------------------------------------------------------------------------------------------
［戻値］	int				：0:相違なし / !0:相違あり
［詳細］	比較対象全領域をパターン列で比較する
［履歴］	05/07/15 t.sasaki Rhein1/2/3初版.
================================================================================================*/
int32_t ptncmp2(
	void* pt_Data,		// 対象データへのポインタ.
	size_t dsize,		// 対象サイズ.
	void* pt_Ptn,		// パターン列.
	size_t psize )		// パターンサイズ.
{
	// ガード.
	if ( (pt_Data == NULL )
	  || (pt_Ptn == NULL) ) {
#ifdef TORNADO
		SYSF_taskSuspend( SYSF_taskIdSelf() ) ;
#endif
		return -1 ;
	}

	// 高速処理するために、4バイトずつ比較しているが、
	// さらに高速化する必要がある場合は、ひとつのループで複数データを比較するように変更する。

	// 4バイトずつ比較する.
	register uint32_t*	pul_Data	= (uint32_t*)pt_Data ;
	register size_t	less		= dsize/psize ;
	for ( ; less > 0; less-- ) {
		register uint32_t*	pul_Ptn		= (uint32_t*)pt_Ptn ;
		for ( register uint32_t unIndex = 0; unIndex < psize/sizeof(uint32_t); unIndex++ ) {
			if ( (*pul_Data) ^ (*pul_Ptn) ) {	// XOR比較(一致していた時のみ0になる).
				return -1 ;	// 比較NG.
			}
			pul_Data++ ;
			pul_Ptn++ ;
		}
	}
	register uchar* puc_Data	= (uchar*)pul_Data ;
	register uchar*	puc_Ptn		= (uchar*)pt_Ptn ;
	less	= dsize%psize ;
	for ( ; less > 0; less-- ) {
		if ( (*puc_Data) ^ (*puc_Ptn) ) {	// XOR比較(一致していた時のみ0になる).
			return -1 ;	// 比較NG.
		}
		puc_Data++ ;
		puc_Ptn++ ;
	}

	return 0 ;	// 比較OK.
}
/*================================================================================================
［機能］	パターンコピーサブルーチン.
--------------------------------------------------------------------------------------------------
［戻値］	なし.
［詳細］	コピー先全領域をコピー元領域の繰り返しで埋める.
［履歴］	05/07/15 t.sasaki Rhein1/2/3初版.
================================================================================================*/
void ptncpy( 
	void* pt_Dst,		// コピー先のポインタ.
	size_t dsize,		// コピー先の領域サイズ.
	void* pt_Src,		// コピー元のポインタ.
	size_t ssize )		// コピー先の領域サイズ.
{
	// ガード.
	if ( (pt_Dst == NULL)
	  || (pt_Src == NULL) ) {
#ifdef TORNADO
		SYSF_taskSuspend( SYSF_taskIdSelf() ) ;
#endif
		return ;
	}

	// まずはコピー元サイズ単位で転送バッファを埋める.
	uint32_t	ul_Num	= dsize/ssize ;
	uchar* pucBuf = (uchar*)pt_Dst ;
	for ( uint32_t ul_Idx = 0; ul_Idx < ul_Num; ul_Idx++ ) {
		_memcpy( pucBuf, pt_Src, ssize ) ;
		pucBuf += ssize ;
	}
	// パターンテーブル未満分の残りがあれば埋める.
	uint32_t	ul_RestSize	= dsize%ssize ;
	if ( ul_RestSize > 0 ) {
		_memcpy( pucBuf, pt_Src, ul_RestSize ) ;
	}
}
/*================================================================================================
［機能］	パターン反転サブルーチン.
--------------------------------------------------------------------------------------------------
［戻値］	なし.
［詳細］	対象領域の全データをビット反転する.
［履歴］	05/07/15 t.sasaki Rhein1/2/3初版.
================================================================================================*/
void ptninv( 
	void* pt_Data,		// 対象データへのポインタ.
	size_t size )		// 対象領域サイズ.
{
	// ガード.
	if ( pt_Data == NULL ) {
#ifdef TORNADO
		SYSF_taskSuspend( SYSF_taskIdSelf() ) ;
#endif
		return ;
	}

	// 高速処理するために、4バイトずつ処理しているが、
	// さらに高速化する必要がある場合は、ひとつのループで複数データを処理するように変更する。

	// 4バイトずつ.
	register uint32_t*	pul_Data	= (uint32_t*)pt_Data ;
	register size_t	less		= size/sizeof(uint32_t) ;
	for ( ; less > 0; less-- ) {
		*pul_Data = ~(*pul_Data) ;
		pul_Data++ ;
	}
	less	= size%sizeof(uint32_t) ;
	register uchar* puc_Data	= (uchar*)pul_Data ;
	for ( ; less > 0; less-- ) {
		*puc_Data = (uchar)~(*puc_Data) ;
		puc_Data++ ;
	}

}

/*================================================================================================
［機能］	【ＩＦ】ＨＤＤデータ消去要求受信
--------------------------------------------------------------------------------------------------
［引数］	ulong					：メッセージオプション
［戻値］	無し
 [詳細]		ＨＤＤアクセスの最小単位はアレイ単位なので注意すること（クラスタサイズはアレイサイズの倍数であること）
［ｳｫｰｸｽﾙｰ］	実施未：
 [履歴]		07/03/19 morikawa Donau初版	
 			07/05/13 morikawa 静的解析不具合修正　		
 			07/06/01 UT検証の不具合（転送アレイ個数の計算が間違っていた）不具合修正　
 			07/10/09 morikawa 暗号化優先時にpEPTblが設定されていなかったため暗号化タスクでNULLアクセスが発生していた不具合修正　
			08/03/11 okada		CQ301189：HDD_Eraseモード2時にトラブルコード【C-D020】が発生する不具合修正
										　・一時データ消去の比較サイズの間違い修正
										　・AESタスクとの同期処理関数のコールタイミング変更
			08/06/19 kurachi Donau CQ:304895対策.
								新ファンクションコードFUNC_HD_FORMAT_EXCEPT_ATA04の追加.
			08/07/09 morikawa Donau 
					CQ:306609一時データ上書き消去設定にて、HDDのBOX蓄積領域を大量に使用すると、画像消去時にFA14が発生する問題対策。
					原因：HDD容量増加に伴い、クラスタ番号はushotからulongに変更したにも関わらず、この関数でのクラスタ番号の取り扱いは
						ushortのままになっていた。
					対策：クラスタ番号の代入時のushortのキャストを削除
						上書き削除時のログコマンドの開始クラスタ番号をushort(us_StartClusterNo)からulong(ul_StartClusterNo)に変更
================================================================================================*/
// データ削除シナリオテーブル用定義.
#undef EN_Fill
#undef EN_Compare
#undef EN_END

#define EN_END		((uchar)0)		// レコードの終わり.
#define EN_Fill		((uchar)1)		// パターンフィル(書き込み).
#define EN_Compare	((uchar)2)		// 10%読み出し＆比較.

#define DEF_MAX_ERASESTATE	4		// 各シナリオステート最大数
#define DEF_FUM_ONEERASE_ARRAYNUM	4	// ERASE用転送最大数(1～16の間で調整する).

#define DEF_ERASE_BUF_SIZE	(THRD_ONEARRAYSIZE * DEF_FUM_ONEERASE_ARRAYNUM)

// HDD_FatTrans Start 06/04/17 morikawa 
// イレース用のバッファは管理転送用FAT転送用と共用するため、
// Rheinであった、　
// uchar	uc_EraseTransBuf[THRD_ONEARRAYSIZE * DEF_FUM_ONEERASE_ARRAYNUM] ;	// 削除パターンを収める転送元バッファ.
// は削除する、これに伴い、バッファの確保／開放制御も削除する　
// HDD_FatTrans End 

// デバッグ用.
ushort	us_debug_perfect_compare = 0 ;	// デバッグ用フラグ－全モードコンペア100%実施.
ushort us_debug_eraselog_verbose = 0 ;	// デバッグ用フラグ－一時データ上書き消去ログ出力の冗長化.

// ------------------------------------ //
// 上書き削除時のログコマンド(評価用)	//
// ------------------------------------ //
// 10h:ファイル単位定義.
struct ST_EraseDataFileLog {
	uint32_t		ul_StartClusterNo ;	// 開始クラスターNo.
	uint32_t		ul_FileSize ;		// 画像ファイルサイズ
	IMGE_IPEType	en_IPEType ;		// 入力ページ情報タイプ.
	IMGE_EraseMode	en_EraseMode ;		// 削除モード.
} ;

// 20h:クラスタ単位定義.
struct ST_EraseDataClusterLog {
	uint32_t		ul_SectorNo ;		// セクターNo.
	ushort			us_SectorNum ;		// セクター数.
	uchar			uc_Action ;			// 動作定義.
	uchar			uc_Ptn ;			// 書き込み・比較データパターン.
} ;

// F0h:ファイル単位定義－ステータス.
struct ST_EraseDataStatusLog {
	uint32_t		ul_NGSector ;		// NG Sector.
	uchar			uc_Status ;			// 0: OK !0:NG (uc_Mode == Status only).
} ;
// ログデータ定義.
struct ST_EraseDataLog {
	union {
		ST_EraseDataFileLog		stFileLog ;		// 10h:ファイル単位定義.
		ST_EraseDataClusterLog	stClusterLog ;	// 20h:クラスタ単位定義.
		ST_EraseDataStatusLog	stStatusLog ;	// F0h:ファイル単位定義－ステータス.
	} un ;
	uint32_t		ul_Tick ;			// tickカウンタ.
	uchar			uc_Mode ;			// 10h: File / 20h: Cluster / F0h: Status.
} ;

// デバッグ用関数定義
void setEraseDataLog( uchar uc_Mode, void* pst_Log );

/*================================================================================================
［機能］	HDD一時データ上書き消去
--------------------------------------------------------------------------------------------------
［引数］	ulong					：メッセージオプション.
［戻値］	無し.
［詳細］	HDD一時データ上書き消去.
［履歴］	13/06/17 ADS北村    関数説明を追加　※関数自体は以前から存在していた.
			13/06/17 ADS北村	CQ.381189(Act_66229) EE-SSC_4	HDDレス化対応_SSDへのスワップ対応.
			14/03/06 toda		Zeus5BK HDDレス対応 HDD装着時にSSDへスワップする暫定対応を削除.
 			14/12/09 ADS八木	CQ.505149 HDDバックアップ/リストア不具合対応.
			15/05/09 iyanagi 【Helios】CQ.509096対応 イニシャライズフォーマット指定の追加.
			15/10/22 大岡		[ZeusSZX0]制御構成変更No.76-1 新詳細コードデータ収集機能.
			18/09/21 Sky井柳 【DenebMLK】制御構成変更No78「データ移行」対応 MicroSD起動時のみ暗号化対象化修正.
			18/12/06 Sky井柳 【DenebMLK】AR.1069398【IT問題】【制御構成変更_No.78】データ移行用 論理フォーマット指示の追加.
			19/08/26 Sky鳥居  [EagleZ+] OP_BTS-11139対応.
================================================================================================*/
void CP_HDDTrans::startHDDEraseData( uintptr_t ulOption )
{
	const static char* pStateMsg[] = {
		"End",
		"Fill",
		"Compare"
	} ;

	struct ST_EraseDataState {
		uchar	uc_Action ;		// 動作定義.
		uchar	uc_Ptn ;		// 書き込み・比較データパターン.
	} ;
	struct ST_EraseDataStateTbl {
		IMGE_EraseMode		en_EraseMode ;
		ST_EraseDataState	st_State[DEF_MAX_ERASESTATE] ;	// 動作定義.
	} ;

	static ST_EraseDataStateTbl st_EraseDataStateTbl[] = {
		{IMGE_EraseMode_None,  {{EN_END, 0x00}}},											// データ削除なし(便宜上).
		{IMGE_EraseMode_Mode1, {{EN_Fill, 0x00}, {EN_END, 0x00}}},							// モード1.
		{IMGE_EraseMode_Mode2, {{EN_Fill, 0x00}, {EN_Fill, 0xFF}, {EN_Fill, 0x61}, {EN_Compare, 0x61}} },
																							// モード2.
		{101,                  {{EN_Fill, 0x00}, {EN_Compare, 0x00}, {EN_END, 0x00}}},		// モード1(デバッグ用).
	} ;
	// 暗号化ありのとき、メッセージパラメータで別タスクに渡すのでスタティックにする　
	static ST_ErasePatternTbl*	pEPTbl = NULL ;

	FuncCallstart( 2, __FUNCTION__, 0 );		// 割込みチェック用関数コールスタート.

	uint32_t ul_StartTime = SYSF_tickGet();	// 開始時間取得
	setHDDSeq( EM_HDDSeq_EraseData );		// シーケンスを「転送中」に設定.

	ST_MsgHDDTransInfo*	pst_MsgHDDTransInfo = (ST_MsgHDDTransInfo*)ulOption;
	if( pst_MsgHDDTransInfo!=NULL) {
		// メッセージの内容をローカルへ保存
		_memcpy( &st_MsgHDDTransInfo, pst_MsgHDDTransInfo, sizeof( ST_MsgHDDTransInfo ) );
		if( us_debug_dummyHDD == 0 ) {
			//2009/03/30 newInstance対応 comMemoryManagerのインスタンス生成.
			IMGC_ImageMemoryPoolImpColorFileMemoryInstance& comMemoryManager = IMGC_ImageMemoryPoolImpColorFileMemorySingletonIf::newInstance();

			// 削除モードに合わせてシナリオを決定する.
			IMGE_EraseMode	en_EraseMode	= IMGE_EraseMode_None ;
			if ( NVDC_SystemSetting::newInstance()->getTempDataOverWriteEraseOn() == True ) {	// 一時データ上書き削除あり
				TYPE_TempDataOverWriteErase en_Mode = NVDC_SystemSetting::newInstance()->getTempDataOverWriteErase();
				setDebugLowLogSpeedSeq( 501, en_Mode );
				switch( en_Mode ) {
					case TYPE_TempDataOverWriteErase2:	// モード2:０ｘ００で上書き→０ｘｆｆで上書き→文字［ａ］（０ｘ６１）で上書き→検証 
						en_EraseMode	= IMGE_EraseMode_Mode2 ;
						break ;
					case TYPE_TempDataOverWriteErase1:	// モード1:０ｘ００で上書き 
					default :
						en_EraseMode	= IMGE_EraseMode_Mode1 ;
						break ;
				}
			}
			// HDDフォーマット開始時は一時データ上書き消去をスルーさせる 2005/04/13 added by t.sasaki
			short ss_FuncCode = CC_MEMFunctionSingletonIf::newInstance().getFuncCode() ;
			// 05/11/01 tsukiji 問題点41797 全データ上書きも条件追加 
			if ( (ss_FuncCode == FUNC_HD_FORMAT_PHYSICAL)
			  || (ss_FuncCode == FUNC_HD_FORMAT_WASTE)
			  || (ss_FuncCode == FUNC_HD_FORMAT_LOGICAL)
//【695新アーキFUM】20111111 FBSC渡辺 新アーキ論理フォーマット対応 start.
//			  || (ss_FuncCode == FUNC_HD_FORMAT_EXCEPT_ATA04)	// 2008/06/19 kurachi CQ:304895対策（ata04フォーマット切り替え対応）.
			  || (ss_FuncCode == FUNC_HD_FORMAT_LOGICAL_ADMIN)
			  || (ss_FuncCode == FUNC_DATAMOVE_FORMAT_LOGICAL)
//【695新アーキFUM】20111111 FBSC渡辺 新アーキ論理フォーマット対応 end.
			  || (ss_FuncCode == FUNC_HD_FORMAT_LOGICAL_INITIALIZE)
			  ) {
				en_EraseMode	= IMGE_EraseMode_None ; 
			}
			// 暗号化有効＆上書き優先時は暗号化されHDDに書き込まれた結果が期待する値になるようにする. 
			Bool bl_EncryptionOrOverWrite = True ;		// デフォルトは「暗号化優先」とする.
			
			if ( en_EraseMode != IMGE_EraseMode_None ) {
#if 0 // サブストレージ暗号化対応の為に、コードを残しておく.
				if ( ( NVDC_SystemSetting::newInstance()->getEnableEncryption() == True )
#else //0.
				if ( ( THRG_IsSsdEncrypt() == True )
#endif //0.
				  && ( NVDC_SystemSetting::newInstance()->getEncryptionOrOverWrite() == False ) ) {
					bl_EncryptionOrOverWrite = False ;	// 「上書き優先」とする.
				}
			}
			ST_EraseDataState* pst_EraseDataState	= NULL ;
			switch ( en_EraseMode ) {
				case IMGE_EraseMode_Mode1:	// モード1
					pst_EraseDataState = &st_EraseDataStateTbl[1].st_State[0] ;
					// モード1の検証のためにコンペアを実施.
					if ( us_debug_perfect_compare == 1 ) {
						pst_EraseDataState = &st_EraseDataStateTbl[3].st_State[0] ;
					}
					break ;
				case IMGE_EraseMode_Mode2:	// モード2
					pst_EraseDataState = &st_EraseDataStateTbl[2].st_State[0] ;
					break ;
				case IMGE_EraseMode_None:	// 一時データ削除なし(設定が途中で変更になった？
				default:
					// 一時データ削除なしで完了として扱う.
					pst_EraseDataState = &st_EraseDataStateTbl[0].st_State[0] ;
					break ;
			}

			// 入力ページ情報タイプ取得.
			IMGE_IPEType	en_IPEType = IMGE_IPEType_Non;		// 入力ページ情報タイプ.
			switch ( st_MsgHDDTransInfo.em_TransDataKind ) {
				case EM_TransDataKind_Image:	// HDD画像データ
					en_IPEType	= IMGE_IPEType_Non ;
					break ;
				case EM_TransDataKind_Store:	// HDD蓄積画像データ
					en_IPEType	= IMGE_IPEType_HDDStore ;
					break ;
				default:
					THRL_ThLogMsgErr( "reqEraseData: Unknown em_TransDataKind=[%d]\n", st_MsgHDDTransInfo.em_TransDataKind );

					FuncCallend( 2, __FUNCTION__, 0 );			// 割込みチェック用関数コールエンド.
					return ;
					break ;
			}

			CC_HDDBlock	c_HDDBlock			= CC_HDDBlock() ;
			c_HDDBlock.pcc_Fat				= comMemoryManager.getFatPtr( en_IPEType ) ;
			c_HDDBlock.ec_IPEType			= en_IPEType ;
			c_HDDBlock.cc_StartClusterNo	= st_MsgHDDTransInfo.ul_StartClusterNo ;

			if ( (c_HDDBlock.pcc_Fat != NULL)
			  && (c_HDDBlock.judgeClusterNo()==True) ) {

				const uint32_t ul_TopCluster	= st_MsgHDDTransInfo.ul_StartClusterNo;
				const uint32_t ul_TopSector	= c_HDDBlock.calcClusterToSector( ul_TopCluster ) ;
				const uint32_t ul_FileSize		= c_HDDBlock.pcc_Fat->getFileSize( ul_TopCluster ) ;

				// 消去画像の分割対応に伴いログ出力を制限する(分割数分ログが増加するため)
				if ( us_debug_eraselog_verbose >= 1 ) {
					switch ( en_EraseMode ) {
						case IMGE_EraseMode_Mode1:	// モード1
							setDebugLowSpeedSeq( 867, ul_TopSector );
							break ;
						case IMGE_EraseMode_Mode2:	// モード2
							setDebugLowSpeedSeq( 868, ul_TopSector );
							break ;
						case IMGE_EraseMode_None:	// 一時データ削除なし(設定が途中で変更になった？
						default:
							setDebugLowSpeedSeq( 869, ul_TopSector );
							break ;
					}
				}

				// ---------------------------- //
				// ＠＠＠評価用ログ登録処理.	//
				// ---------------------------- //
				{
					ST_EraseDataFileLog		st_FileLog ;
					st_FileLog.en_IPEType			= en_IPEType ;
					st_FileLog.en_EraseMode			= en_EraseMode ;
					st_FileLog.ul_StartClusterNo	= ul_TopCluster ;
					st_FileLog.ul_FileSize			= ul_FileSize ;
					setEraseDataLog( 0x10, &st_FileLog ) ;
				}

				uchar	uc_Status	= 0 ;
				uint32_t	ul_NGSector	= UINT_MAX ;
				// シナリオのステートに合わせて繰り返す.
				Bool bl_EndSenario	= False ;
				for ( register uint32_t ul_StateIndex = 0; ul_StateIndex < DEF_MAX_ERASESTATE; ul_StateIndex++ ) {

					ST_EraseDataState* pst_State	= &pst_EraseDataState[ul_StateIndex] ;

					if( 0 < us_DebugEnable ) {
						printf("[%ld]Overwrite delete start[%s] PTN[0x%02X] cluster[%"PRIu32"] sector[%"PRIu32"] size[%"PRIu32"]\n", 
							SYSF_tickGet(),
							pStateMsg[pst_State->uc_Action], pst_State->uc_Ptn, ul_TopCluster, ul_TopSector, ul_FileSize );
					} else {
						THRL_ThLogMsgDebug("Overwrite delete start[%s] PTN[0x%02X] cluster[%"PRIu32"] sector[%"PRIu32"] size[%"PRIu32"]\n",
							pStateMsg[pst_State->uc_Action], pst_State->uc_Ptn, ul_TopCluster, ul_TopSector, ul_FileSize );
					}

					EM_HDD_RWMode	em_HDD_RWMode	= EM_HDD_RWMode_Write ;					// デフォルト(特に意味なし).
					THRE_ReadWriteHDDMode	e_ReadWriteHDDMode = THRE_ReadWriteBufToHDD;	// HDD
					THRE_EncHDDMode			e_EncHDDMode = THRE_EncHDDNone;					// HDDの暗号化転送識別初期化　

					uint32_t ul_TransDataSize	= 0 ;		// 転送サイズ.
					// HDDフォーマット開始時は一時データ上書き消去をスルーさせる 
					uchar uc_Action = pst_State->uc_Action ;
					pEPTbl = NULL ;
					short ss_FuncCode = CC_MEMFunctionSingletonIf::newInstance().getFuncCode() ;
					// 全データ上書きも条件追加　 
					if ( (ss_FuncCode == FUNC_HD_FORMAT_PHYSICAL)
					  || (ss_FuncCode == FUNC_HD_FORMAT_WASTE)
					  || (ss_FuncCode == FUNC_HD_FORMAT_LOGICAL)
//【695新アーキFUM】20111111 FBSC渡辺 新アーキ論理フォーマット対応 start.
//					  || (ss_FuncCode == FUNC_HD_FORMAT_EXCEPT_ATA04)	// 2008/06/19 kurachi CQ:304895対策（ata04フォーマット切り替え対応）.
					  || (ss_FuncCode == FUNC_HD_FORMAT_LOGICAL_ADMIN)
					  || (ss_FuncCode == FUNC_DATAMOVE_FORMAT_LOGICAL)
//【695新アーキFUM】20111111 FBSC渡辺 新アーキ論理フォーマット対応 end.
					  || (ss_FuncCode == FUNC_HD_FORMAT_LOGICAL_INITIALIZE)
					  ) {
						uc_Action	= EN_END ;
					}
					// 上書き優先or暗号化優先に関わらずパターンテーブルを設定する　
					pEPTbl = getErasePatternTbl( pst_State->uc_Ptn );
					// ガード処理－ありえない
					if ( pEPTbl == NULL ) {
						// ソフトエラー以外あり得ない.
						THRL_ThLogMsgErr("\n@@@@ startHDDEraseData():Pattern Error @@@@\n");
						uc_Action = EN_END;	// どうしようもないので終わり.
					}
					switch ( uc_Action ) {
						case EN_Fill:		// パターンフィル(書き込み).
							em_HDD_RWMode		= EM_HDD_RWMode_Write ;
							e_EncHDDMode		= THRE_EncHDDArrayToBuf;
							ul_TransDataSize	= ul_FileSize ;		// ファイルサイズと同じ.
							// 書き込みパターン作成(1転送サイズ分作成し使いまわす).
							// 消去モードに応じて消去パターン作成.
							// 暗号化有効＆上書き優先時は暗号化されHDDに書き込まれた結果が期待する値になる上書きパターンを使用.
							if ( bl_EncryptionOrOverWrite == False ) {
								// 上書き優先時（上書きパターンデータ選択）.
								ptncpy( &uc_HDDbuffer[0], DEF_ERASE_BUF_SIZE, pEPTbl->puc_PtnBuf, pEPTbl->us_Size ) ;
							} else {
								// 暗号化なしor暗号化優先時.
								memset( &uc_HDDbuffer[0], pst_State->uc_Ptn, DEF_ERASE_BUF_SIZE ) ;
							}
							if ( us_debug_hdderase > 0 ) {
							    THRL_ThLogMsgErr("CP_HDDTrans::startHDDEraseData : WORK->HDD\n");
							}
							break ;
						case EN_Compare:	// 10%読み出し＆比較.
							em_HDD_RWMode		= EM_HDD_RWMode_Read ;
							e_ReadWriteHDDMode 	= THRE_ReadWriteHDDToBuf;	// HDD→Buf読み出し　	
							e_EncHDDMode		= THRE_EncHDDBufToArray;
							ul_TransDataSize	= _roundup(ul_FileSize, 10L) ;		// ファイルサイズの10%以上.
							_memset( &uc_HDDbuffer[0], DEF_ERASE_BUF_SIZE ) ;
							// 上書き優先時（上書きパターンデータ選択）はすでに実行済み
							if ( us_debug_hdderase > 0 ) {
							    THRL_ThLogMsgErr("CP_HDDTrans::startHDDEraseData : HDD->WORK\n");
							}
							// テスト用－100%比較実施.
							if ( us_debug_perfect_compare == 1 ) {
								ul_TransDataSize	= ul_FileSize ;
							}
							break ;
						case EN_END:		// レコードの終わり.
						default:
							bl_EndSenario	= True ;
							break ;
					}

					if ( bl_EndSenario == True ) {
						break ;
					}

					uint32_t ul_Sector			= ul_TopSector ;
					uint32_t ul_NextCluster	= 0 ;
					uint32_t ul_NowCluster		= 0 ;
					uint32_t ul_BeforeCluster	= UINT_MAX ;
					ST_EraseDataClusterLog	st_ClusterLog ;		// ログ用.

					_DebugSequence( em_HDD_RWMode );	// 02/02/15 morikawa 常にログをとるように修正　

					// ---------------------------- //
					// ＠＠＠評価用ログ登録処理.	//
					// ---------------------------- //
					st_ClusterLog.uc_Action		= pst_State->uc_Action ;
					st_ClusterLog.uc_Ptn		= pst_State->uc_Ptn ;

//////////////////////////////////////////////////////
// HDD転送前処理
//////////////////////////////////////////////////////
					// 転送アレイ個数はデータサイズをアレイサイズで切り上げる
					uint32_t ul_TransArrayNum = ul_TransDataSize / THRD_ONEARRAYSIZE;
					if( (ul_TransDataSize % THRD_ONEARRAYSIZE) != 0 ) {
						++ul_TransArrayNum;
					} 
					uint32_t ul_EndArrayNum = 0;	// 転送終了したアレイ数初期化
					uint32_t ul_OffsetSector = 0;	// 先頭セクタ番号からのオフセット量
					Bool bl_CompareNG = False ;	// デフォルトは「コンペアNGではない」とする.
					Bool b_ReqCompareData = False;		// 一時データ消去におけるデータ比較要求初期化　
					if( pst_State->uc_Action == EN_Compare && b_EnableEnc == True ) {
						// データ比較ステート、かつ、暗号化有効の時のみAES転送タスクメッセージにデータ比較要求をセットする　
						b_ReqCompareData = True;
					}  
					
					// IV設定　
					setIVNum( c_HDDBlock.calcRealSector(0), ul_TransArrayNum, DEF_FUM_ONEERASE_ARRAYNUM );

#ifdef DEF_Soc1200 //CQ.505149 HDDバックアップ/リストア不具合対応.
					memset( e_ArrayType, 0, sizeof( e_ArrayType ) );
#endif //#ifdef DEF_Soc1200.
// 暗号化ありのHDD読み出しはAES処理タスクへメッセージ必要　
// 一時データ消去の場合は、転送すべきデータがメモリ上に転送サイズ分ないので、第４引数はTrueとする　
// 第２引数のアレイアドレスは配列になっているので、イレースデータのバッファアドレスを１回のイレースデータ分設定する　
					for( uchar uc_EraseArray = 0; uc_EraseArray<DEF_FUM_ONEERASE_ARRAYNUM; ++uc_EraseArray ) {
						ul_TransArrayAdr[uc_EraseArray] = (uintptr_t)(uc_HDDbuffer + THRD_ONEARRAYSIZE*uc_EraseArray);
#ifdef DEF_Soc1200	//CQ.505149 HDDバックアップ/リストア不具合対応.
						e_ArrayType[uc_EraseArray] = TYPE_ArrayMemoryType_Main; //HDDバッファ(HDDBufferArea)はメインメモリ側.
#endif //#ifdef DEF_Soc1200.
					}
#ifdef DEF_Soc1200 //CQ.505149 HDDバックアップ/リストア不具合対応.
					syncSendMsgExecHDD( e_EncHDDMode,
										ul_TransArrayAdr,
										True,		// アレイアドレスは１個でこれを繰り返し用いる　
										ul_TransArrayNum,
										DEF_FUM_ONEERASE_ARRAYNUM,
										0,
										b_ReqCompareData,
										pEPTbl,
										pst_State->uc_Ptn ,
										False ,e_ArrayType );
#else //#ifdef DEF_Soc1200.
					syncSendMsgExecHDD( e_EncHDDMode,
										ul_TransArrayAdr,
										True,		// アレイアドレスは１個でこれを繰り返し用いる　
										ul_TransArrayNum,
										DEF_FUM_ONEERASE_ARRAYNUM,
										0,
										b_ReqCompareData,
										pEPTbl,
										pst_State->uc_Ptn );
#endif //#ifdef DEF_Soc1200.
// １回のイレース転送での最大セクタ数　
#define DEF_MAX_HDD_ONEERASE_SECTOR_NUM	( DEF_FUM_ONEERASE_ARRAYNUM * THRD_ONEARRAYSIZE / DEF_SECTOR_SIZE )
	 			
//////////////////////////////////////////////////////
// HDD転送実行開始
//////////////////////////////////////////////////////
					while( True ) {
						// ＠＠＠評価用ログ登録処理開始 //
						// クラスタが変わったところでログ用保持データ初期化.
						if ( ul_BeforeCluster != ul_NowCluster ) {
							ul_BeforeCluster			= ul_NowCluster ;
							st_ClusterLog.ul_SectorNo	= c_HDDBlock.calcRealSector( ul_OffsetSector ) ;
							st_ClusterLog.us_SectorNum	= 0 ;
						}
						// ＠＠＠評価用ログ登録処理終了 //

						// １回のDMAで転送するセクタ数初期化
						uint32_t ul_OneTransSectorNum = DEF_MAX_HDD_ONEERASE_SECTOR_NUM;
					
						if( ul_TransArrayNum < DEF_FUM_ONEERASE_ARRAYNUM ) {
					 		// 転送すべきアレイ数が１回の最大個数より小さい時、転送セクタ数補正
							ul_OneTransSectorNum = ul_TransArrayNum * THRD_ONEARRAYSIZE / DEF_SECTOR_SIZE;
						} else if( (ul_TransArrayNum - ul_EndArrayNum) <  DEF_FUM_ONEERASE_ARRAYNUM ) {
					 		// 転送すべきアレイ数が最後の端数の時
							ul_OneTransSectorNum = (ul_TransArrayNum - ul_EndArrayNum) * THRD_ONEARRAYSIZE / DEF_SECTOR_SIZE;
						}

						// 今から転送するクラスタの次のクラスタ計算
						ul_NextCluster = c_HDDBlock.calcRelativeCluster( calcArrayToSector( ul_EndArrayNum + DEF_FUM_ONEERASE_ARRAYNUM ) );
					
						// HDD転送の先頭セクタ番号取得
						ul_Sector = c_HDDBlock.calcRealSector( ul_OffsetSector );

						// 暗号化なしのときの共通化処理のためにアドレス設定　
						uintptr_t ul_BufAdr = (uintptr_t)uc_HDDbuffer;
						if( b_EnableEnc == True ) {
							if( em_HDD_RWMode == EM_HDD_RWMode_Write ) {
								ul_BufAdr = c_HDDBuf.getReadAdr();	// バッファを読み出してHDDに書き込み
							} else {
								ul_BufAdr = c_HDDBuf.getWriteAdr();	// HDDから読み出して書き込むバッファアドレス獲得　
							}
						}	
						
						if ( pst_State->uc_Action == EN_Compare ) {
							// 比較実行時の処理実行　
							if( b_EnableEnc == False ) {
								// 暗号化なしの時のみHDD転送タスク側で一時データ上書き消去用データをクリヤ
								// 注：暗号化ありの時は、AES転送タスクで同じ関数をコールする
								FuncCallend( 2, __FUNCTION__, 0 );			// 割込みチェック用関数コールエンド.
								clearEraseCompareData( pEPTbl, pst_State->uc_Ptn );	
								FuncCallstart( 2, __FUNCTION__, 0 );		// 割込みチェック用関数コールスタート.
							}
						}

						// 暗号化ありを考慮したスタート前処理実行
						syncStartOneExecHDD(); 		
						
						// HDD転送実行
						// 暗号化なしの時は、ul_BufAdrはuc_HDDbufferと同じ
						// 通常のアレイ転送と異なり、常時連続したアドレス領域を転送するので
						// 暗号化あり、HDDへの書き込み時はすでび暗号化されたデータをc_HDDBufから読み出す。これを今からHDDへ書き込む。			
						// 暗号化あり、HDDからの読み出し時はHDDから取り出したデータを今からc_HDDBufに書き込む。その後に複合化する。			
#ifdef DEF_SupportSSD
						readWriteExecHDD( c_DevicePath, e_ReadWriteHDDMode, 0, ul_BufAdr, ul_Sector, ul_OneTransSectorNum );
#else	// DEF_SupportSSD.
						readWriteExecHDD( e_ReadWriteHDDMode, 0, ul_BufAdr, ul_Sector, ul_OneTransSectorNum );
#endif	// DEF_SupportSSD.

						// 暗号化ありを考慮した終了後処理実行
						syncEndOneExecHDD(); 		

						if ( pst_State->uc_Action == EN_Compare ) {
							// 比較実行時の処理実行　
							if( b_EnableEnc == False ) {
								// 暗号化なしの時のみHDD転送タスク側で一時データ上書きデータ比較関数をコール
								// 注：暗号化ありの時は、AES転送タスクで同じ関数をコールする
								FuncCallend( 2, __FUNCTION__, 0 );			// 割込みチェック用関数コールエンド.
								bl_CompareNG = chkEraseCompareNGData( pEPTbl, pst_State->uc_Ptn, DEF_SECTOR_SIZE * ul_OneTransSectorNum );	
								FuncCallstart( 2, __FUNCTION__, 0 );		// 割込みチェック用関数コールスタート.
							}
						}
						
						// 転送完了したアレイ数
						ul_EndArrayNum += ul_OneTransSectorNum * DEF_SECTOR_SIZE / THRD_ONEARRAYSIZE;
						// セクタ相対番号加算　 
						ul_OffsetSector += ul_OneTransSectorNum;
						
						// ＠＠＠評価用ログ登録処理開始 //
						st_ClusterLog.us_SectorNum	= (ushort)(st_ClusterLog.us_SectorNum + ul_OneTransSectorNum);	// セクタ数の加算 
						// 次にクラスタが変わる時の転送or最後の転送時にログ出力.
						if( (ul_NextCluster > ul_NowCluster) 
						 || (ul_EndArrayNum >= ul_TransArrayNum) ) {
							setEraseDataLog( 0x20, &st_ClusterLog ) ;
						}
						// 次のクラスタ番号を現在のクラスタ番号とする　	
						ul_NowCluster = ul_NextCluster;	
						// ＠＠＠評価用ログ登録処理終了 //
					
						if( ul_EndArrayNum >= ul_TransArrayNum
						 || bl_CompareNG == True ) {
							// 転送完了したアレイ数が、転送予定個数に達した
							// または、暗号化なしモードにおいて比較の結果NGが発生した　

							// 暗号化ありを考慮した全終了後処理実行
							syncEndAllExecHDD( e_ReadWriteHDDMode ); 		

							// 比較NGのログ
							if( bl_CompareNG == True
							 || THRC_AesTransSingletonIf::newInstance().chkEraseCompareNG() == True ) {
								// 比較エラー
								// 【課題：佐々木】一時データ上書き削除モード2で比較NGの時の処理は？
								bl_EndSenario	= True ;
								THRL_ThLogMsgErr("\n@@@@ startHDDEraseData():Compare Error @@@@\n");
								// 新詳細コード保存.
								uint64_t		ull_NewDetailData = THRD_NewDetail_ModuleId_Thread;				// 1,2桁：スレッド.
								ull_NewDetailData |= (uint64_t)( TYPD_TroubleCode_HDD_VERIFY_ERROR ) << THRD_NewDetail_TroubleCode_Shift;	// 3-6桁：トラブルコード.
								ull_NewDetailData |= (uint64_t)( b_EnableEnc ) << 32;							// 7,8桁：暗号化.
								ull_NewDetailData |= (uint64_t)( bl_CompareNG ) << 24;						// 9,10桁：データ比較結果.
								THRL_ThAssertLog2( 0, ull_NewDetailData );
								DebugForever( TYPD_TroubleCode_HDD_VERIFY_ERROR );		// 
								// 評価用ログ登録,
								{
									uc_Status		= 0xFF ;
									ul_NGSector		= ul_Sector ;
								}
							}
							break;
						}
					}
//////////////////////////////////////////////////////
// HDD転送実行終了
//////////////////////////////////////////////////////
				}
#ifdef SIMULATOR
				if (SIMG_IsOpSim2Mode()) {
					struct stat	st;
					char		szFile[DEF_PATH_LENGTH];
					
					memset(szFile, '\0', sizeof(char) * DEF_PATH_LENGTH);
					sprintf(szFile, THRD_TEMPIMAGE_ARRAY_FILE, ul_TopSector);
					
					if (0 == stat(szFile, &st)) {
						remove(szFile);
					}
				}
#endif
				// -------------------- //
				// 評価用ログ登録.		//
				// -------------------- //
				{
					ST_EraseDataStatusLog	st_StatusLog ;
					st_StatusLog.uc_Status		= uc_Status ;
					st_StatusLog.ul_NGSector	= ul_NGSector ;
					//st_StatusLog.uc_padding     = 0;
					//st_StatusLog.us_padding     = 0;
					setEraseDataLog( 0xF0, &st_StatusLog ) ;
				}
			} else {
				setDebugHighSpeedSeq( 99, 8 );		// 02/06/18 tsukiji 画像ＨＤＤデータ異常
				if( 0 < us_DebugEnable ) {
					printf( "[%"PRId32"]HDD transfer data error number OIP,or No FAT info,or cluster number error\n", SYSF_tickGet() );
				} else {
					THRL_ThLogMsgDebug(( "HDD transfer data error number OIP,or No FTP info,or cluster number error\n" ));
				}
			}
			_DebugSequence( (short)(st_MsgHDDTransInfo.em_HDD_RWMode + 90) );	// 02/02/15 morikawa 常にログをとるように修正　　

		}
		// ＨＤＤの転送が完了したメッセージを送信
		st_MsgHDDEraseCompleteInfo.ev_TaskID			= st_MsgHDDTransInfo.ev_TaskID ;
		st_MsgHDDEraseCompleteInfo.em_TransDataKind		= st_MsgHDDTransInfo.em_TransDataKind ;
		st_MsgHDDEraseCompleteInfo.ul_StartClusterNo	= st_MsgHDDTransInfo.ul_StartClusterNo ;
	}
	else {
		st_MsgHDDEraseCompleteInfo.ev_TaskID			= TID_Image ;
		st_MsgHDDEraseCompleteInfo.em_TransDataKind		= EM_TransDataKind_None ;
		st_MsgHDDEraseCompleteInfo.ul_StartClusterNo	= DEF_MAX_FAT ;
	}
	
	setDebugLowLogSpeedSeq( 591, 0 );

#ifdef TORNADO		// VXWORKSで有効 
	sendMessage( TID_Image, TM_HDDEraseDataEndNotice, (uintptr_t)&st_MsgHDDEraseCompleteInfo );
#else
	// SimWinではメッセージ遅延
	setDelayMessage( TID_Image, TM_HDDEraseDataEndNotice, (uintptr_t)&st_MsgHDDEraseCompleteInfo, ul_ImageDelayTimer );
#endif	// #ifdef TORNADO

	setHDDSeq( EM_HDDSeq_Idle );		// シーケンスを停止（待機状態へ）
	uint32_t ul_EndTime = SYSF_tickGet();	// 終了時間取得
	uint32_t ul_Time = ul_EndTime - ul_StartTime ;
	if ( ul_debug_Time_startHDDEraseData < ul_Time ) {
		ul_debug_Time_startHDDEraseData = ul_Time ;
	}
	// 転送時間デバッグ表示
	if( us_debug_HDDTime > 0 ) {
		printf( "HDD Image Erase Time = [%"PRId32"]ms\n", ul_Time * 5 );
	}
	FuncCallend( 2, __FUNCTION__, 0 );			// 割込みチェック用関数コールエンド.
}

// ================================================================================================
//! <!--機能--> 一時データ消去の比較データクリヤ　
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param ST_ErasePatternTbl* pEPTbl イレースパターンテーブルへのポインタ
//! \param uchar uc_Ptn イレースパターンテーブル（1バイトデータ）
//! <!--戻値-->
//! \return なし
//! <!--詳細-->
//! \n 暗号化なしの時はHDD転送タスクからHDD転送開始前にコールされる
//! \n 暗号化ありの時はAES転送タスクから複合化開始前にコールされる
//! \n クリヤの方法として、上書き優先時はパターンデータのビット反転で初期化する、暗号化なしor暗号化優先時は１バイトパターンの反転で初期化する
//! <!--履歴-->
// 2007/04/10 morikawa	新規作成 
// 2007/05/14 morikawa	静的解析不具合対応　 
// 2019/08/26 Sky鳥居 [EagleZ+] OP_BTS-11139対応.
// ================================================================================================
void CP_HDDTrans::clearEraseCompareData( const ST_ErasePatternTbl* pEPTbl, uchar uc_Ptn )
{
	FuncCallstart( 3, __FUNCTION__, 0 );		// 割込みチェック用関数コールスタート.

	// 比較実行時は、HDD読み出しバッファを比較パターン以外でクリアする.
	if ( b_IVFixMode == True ) {
		// 上書き優先時.
		ptncpy( &uc_HDDbuffer[0], DEF_ERASE_BUF_SIZE, pEPTbl->puc_PtnBuf, pEPTbl->us_Size ) ;
		ptninv( &uc_HDDbuffer[0], DEF_ERASE_BUF_SIZE ) ;	// ビット反転.
	} else {
		// 暗号化なしor暗号化優先時.
		memset( &uc_HDDbuffer[0], ~uc_Ptn, DEF_ERASE_BUF_SIZE ) ;
	}
	FuncCallend( 3, __FUNCTION__, 0 );			// 割込みチェック用関数コールエンド.
}

// ================================================================================================
//! <!--機能--> 一時データ消去の比較データNGチェック　
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param ST_ErasePatternTbl* pEPTbl イレースパターンテーブルへのポインタ
//! \param uchar uc_Ptn イレースパターンテーブル（1バイトデータ）
//! \param ulong ul_CompareSize 比較サイズ
//! <!--戻値-->
//! \return True:比較NG発生/False：比較OK　
//! <!--詳細-->
//! \n 暗号化なしの時はHDD転送タスクからHDD転送開始前にコールされる
//! \n 暗号化ありの時はAES転送タスクから複合化開始前にコールされる
//! \n データ比較の方法として、上書き優先時はパターンデータで比較する、暗号化なしor暗号化優先時は１バイトパターンで比較
//! <!--履歴-->
// 2007/04/10 morikawa	新規作成 
// 2007/05/14 morikawa	静的解析不具合対応　 
// 2008/03/11 okada		CQ301189：比較サイズを引数から取得するように修正
// 2019/08/26 Sky鳥居 [EagleZ+] OP_BTS-11139対応.
// ================================================================================================
Bool CP_HDDTrans::chkEraseCompareNGData( const ST_ErasePatternTbl* pEPTbl, uchar uc_Ptn, uint32_t ul_CompareSize )
{
	Bool bl_CompareNG = False;

	FuncCallstart( 4, __FUNCTION__, 0 );		// 割込みチェック用関数コールスタート.

	if ( b_IVFixMode == True ) {
		// 上書き優先時.
		// パターンテーブルで比較する　
		if ( ptncmp2( &uc_HDDbuffer[0], ul_CompareSize, pEPTbl->puc_PtnBuf, pEPTbl->us_Size ) != 0 ) {
			bl_CompareNG = True ;	// コンペアNG
		}
	} else {
		// 暗号化なしor暗号化優先時.
		if ( ptncmp( &uc_HDDbuffer[0], uc_Ptn, ul_CompareSize ) != 0 ) {
			bl_CompareNG = True ;
		}
	}
	FuncCallend( 4, __FUNCTION__, 0 );			// 割込みチェック用関数コールエンド.

	return( bl_CompareNG );
}

/*================================================================================================
［機能］	HDD転送完了メッセージ送信関数
--------------------------------------------------------------------------------------------------
［引数］	ST_MsgHDDTransInfo* HDD転送要求メッセージパラメータへのポインタ
			Bool b_Stop 停止したか否か
			ulong ul_EndArrayNum 転送完了したアレイ個数
［戻値］	なし			
［詳細］	HDD転送完了処理をまとめる。
				管理情報の転送完了時とアレイの転送完了時にコールされる
 [履歴]     06/03/03 morikawa Mosel新規追加　
 			07/04/02 morikawa Donauデフォルト引数に停止したか否かと転送完了したアレイ数を追加　
 			07/05/14 morikawa 静的解析不具合修正　
================================================================================================*/
void CP_HDDTrans::sendMsgHDDTransEnd( const ST_MsgHDDTransInfo* ptr_MsgHDDTransInfo, Bool b_Stop, uint32_t ul_EndArrayNum )
{
	ST_MsgHDDTransCompleteInfo* ptr_MsgHDDTransCompleteInfo = NULL;
	ptr_MsgHDDTransCompleteInfo = getMsgHDDTransEndPara();
	if( ptr_MsgHDDTransInfo != NULL ) {
		// 転送要求が有効な時のみパラメータを設定する　
		ptr_MsgHDDTransCompleteInfo->ev_TaskID = ptr_MsgHDDTransInfo->ev_TaskID;
		ptr_MsgHDDTransCompleteInfo->pcc_OneInputImage = ptr_MsgHDDTransInfo->pcc_OneInputImage;
		ptr_MsgHDDTransCompleteInfo->uc_OneThreadId = ptr_MsgHDDTransInfo->uc_OneThreadId;
	} else {
		ptr_MsgHDDTransCompleteInfo->ev_TaskID = TID_Image ;
		ptr_MsgHDDTransCompleteInfo->pcc_OneInputImage = NULL ;
		ptr_MsgHDDTransCompleteInfo->uc_OneThreadId = 0xFF;	// 未定義の値を設定する　
	}
	
	ptr_MsgHDDTransCompleteInfo->b_AnsStop = b_Stop;				// ストップ識別フラグを追加　
	ptr_MsgHDDTransCompleteInfo->ul_EndArrayNum = ul_EndArrayNum;	// 転送完了したアレイ数を追加　
	
#ifdef TORNADO		// VXWORKSで有効 
	sendMessage( TID_Image, TM_HDDTransportEndNotice, (uintptr_t)ptr_MsgHDDTransCompleteInfo );
#else
	// 02/02/06 tsukiji メッセージ遅延
	setDelayMessage( TID_Image, TM_HDDTransportEndNotice, (uintptr_t)ptr_MsgHDDTransCompleteInfo, ul_ImageDelayTimer );
#endif	// #ifdef TORNADO
}

/*================================================================================================
［機能］	HDD転送完了メッセージ領域取得関数
--------------------------------------------------------------------------------------------------
［引数］	なし
［戻値］	ST_MsgHDDTransCompleteInfo* HDD転送完了メッセージパラメータタへのポインタ
［詳細］	HDD転送完了のメッセージパラメータ領域を複数持って、順番に領域を書き換えて使用していく
			TASK_HDDが
				Image１のHDD転送要求開始メッセージ受信
				Image１のHDD転送完了メッセージ送信
			TASK_Imageが
				Image１のHDD転送完了メッセージ送信
					TASK_HDDへImage２のHDD転送要求開始メッセージ送信
				Image１のHDD転送完了メッセージをTASK_FileBufに送信
			TASK_HDDが
				Image２のHDD転送要求開始メッセージ受信
				Image２のHDD転送完了メッセージ送信
			TASK_FileToBufが
				Image１のHDD転送完了メッセージを受信
			となった場合に、HDD転送完了メッセージの領域が１つしかないため上書きされてしまう不具合修正のために
			この領域を複数持つためにMoselで新設
 [履歴]     06/03/03 morikawa Mosel新規追加　
 			07/05/14 morikawa 静的解析の不具合対応　
================================================================================================*/
ST_MsgHDDTransCompleteInfo* CP_HDDTrans::getMsgHDDTransEndPara()
{
	ST_MsgHDDTransCompleteInfo* ptr_MsgHDDTransEndPara = NULL;				// ＨＤＤ転送完了メッセージ
	ptr_MsgHDDTransEndPara = &st_MsgHDDTransEndPara[ uc_CurnPos ];
	++uc_CurnPos;
	if( uc_CurnPos >= DEF_HDD_TRANS_END_MSG_BUF_NUM ) {
		uc_CurnPos = 0;
	}
	return( ptr_MsgHDDTransEndPara );
}

// HDD_FatTrans Start morikawa 06/04/17
/*================================================================================================
［機能］	【ＩＦ】ＨＤＤ転送要求がＦＡＴの転送か否か判断する（FAT情報）
--------------------------------------------------------------------------------------------------
［引数］	ulong	：メッセージオプション
［戻値］	True:FATの転送である、False:FATの転送ではない
 [詳細]		ＨＤＤ転送要求がＦＡＴの転送か否か判断する
 [履歴]		06/04/17 morikawa Mosel初版
================================================================================================*/
Bool CP_HDDTrans::isTransFat( uintptr_t ul_Option )
{
	Bool bl_ans = False;	
	ST_MsgHDDTransInfo* pst_MsgHDDTransInfo = (ST_MsgHDDTransInfo*)ul_Option;
	switch( pst_MsgHDDTransInfo->em_HDD_RWMode ) {
		case EM_HDD_RWMode_StoreFatWrite:
		case EM_HDD_RWMode_SwapFatWrite:
		case EM_HDD_RWMode_StoreFatRead:
		case EM_HDD_RWMode_SwapFatRead:
			bl_ans = True;
			break;
		default:
			break;
	}
	return( bl_ans );
}
/*================================================================================================
［機能］	【ＩＦ】ＨＤＤ転送要求受信（FAT情報）
--------------------------------------------------------------------------------------------------
［引数］	ulong					：メッセージオプション
［戻値］	無し
 [詳細]		管理情報FAT、アレイ情報FATの書き込み／読み出しを行う
 [履歴]		06/04/17 morikawa Mosel初版
		06/05/15 morikawa 読み出しでエラー発生時の訂正書き込みの処理追加
		07/05/14 morikawa 静的解析不具合修正　
        2013/08/27 ADS川口 CQ.381189(Act_67618) EE-HDL-5	HDDレス化対応
        　　　　　　　　　　　　　　　　　　　　①SSDのFAT領域(スワップ)へのアクセス不要対応
        　　　　　　　　　　　　　　　　　　　　②スワップ/蓄積領域のサイズは0になった時、FAT領域(スワップ/蓄積)へのアクセスは不要.
		2020/12/07 Sky		OP_BTS-29394 A800 Completion Timeout発生時は、HDD転送を行わないよう対応.
================================================================================================*/
void CP_HDDTrans::startHDDTransFat( uintptr_t ulOption )
{
	// 「A800メモリマスタアクセスにおいてCompletion Timeout」が発生した場合、.
	// 暗号化転送するとMAXクラスタ数領域を破壊してしまうため、HDD転送を行わないよう対応.
	extern ushort us_DebugForeverCode;
	if( us_DebugForeverCode == TYPD_TroubleCode_CFCFE_DebugFatal )
	{
		setDebugLowLogSpeedSeq( 392, 0 );
		return;
	}

	//2009/03/30 newInstance対応 comMemoryManagerのインスタンス生成.
	IMGC_ImageMemoryPoolImpColorFileMemoryInstance& comMemoryManager = IMGC_ImageMemoryPoolImpColorFileMemorySingletonIf::newInstance();

	// 受けたメッセージのオプションをアドレスとしてメッセージデータアドレスへ変換する
	ST_MsgHDDTransInfo*	pst_MsgHDDTransInfo = (ST_MsgHDDTransInfo*)ulOption;
	setHDDSeq( EM_HDDSeq_HDDtrans );	// シーケンスを転送中をセット
	
	if( pst_MsgHDDTransInfo == NULL ) {
		return; // エラープロテクト
	}

	_memcpy( &st_MsgHDDTransInfo, pst_MsgHDDTransInfo, sizeof( ST_MsgHDDTransInfo ) );
	
	// debugLog 92に残す
	setDebugLowLogSpeedSeq( 301, st_MsgHDDTransInfo.em_HDD_RWMode );	

	CC_Fat* pcc_Fat = NULL;
	CC_Fat* pcc_InfoFat = NULL;
	SEM_ID semID_Mutex = 0;
	if( st_MsgHDDTransInfo.em_HDD_RWMode == EM_HDD_RWMode_SwapFatWrite 
	 || st_MsgHDDTransInfo.em_HDD_RWMode == EM_HDD_RWMode_SwapFatRead ) {
		// Swap
		pcc_Fat = comMemoryManager.getFatPtr( IMGE_IPEType_Non );
		pcc_InfoFat = comMemoryManager.getInfoFatPtr( IMGE_IPEType_Non );
		semID_Mutex = comMemoryManager.getFatSemID( IMGE_IPEType_Non );
	} else {
		// 蓄積
		pcc_Fat = comMemoryManager.getFatPtr( IMGE_IPEType_HDDStore );
		pcc_InfoFat = comMemoryManager.getInfoFatPtr( IMGE_IPEType_HDDStore );
		semID_Mutex = comMemoryManager.getFatSemID( IMGE_IPEType_HDDStore );
	}	
	
	if( comMemoryManager.getStorageDeviceType() == THRE_StorageDeviceTypeHDD
	    || (st_MsgHDDTransInfo.em_HDD_RWMode != EM_HDD_RWMode_SwapFatWrite
	        && st_MsgHDDTransInfo.em_HDD_RWMode != EM_HDD_RWMode_SwapFatRead) ){
		// スワップ領域/蓄積領域の有無をチェック
		if( pcc_Fat->getMaxClusterNum() != 0 ){
			// 書き込み処理
			if( st_MsgHDDTransInfo.em_HDD_RWMode == EM_HDD_RWMode_StoreFatWrite
			 || st_MsgHDDTransInfo.em_HDD_RWMode == EM_HDD_RWMode_SwapFatWrite ) {
			 	// 書き込み処理
			 	startHDDTransFatWrite( pcc_Fat, pcc_InfoFat, &semID_Mutex );
			} else {
		
			 	// 読み出し処理
				if( st_MsgHDDTransInfo.em_HDD_RWMode == EM_HDD_RWMode_StoreFatRead ){
					remakeFat( pcc_Fat );
				}
			 	// 戻り値として、チェックサムエラー発生時の訂正書き込み要求番号が帰ってくる
			 	THRE_FatSector e_WrCorrectFatSector = startHDDTransFatRead( pcc_Fat, pcc_InfoFat, &semID_Mutex );
			 	
			 	if( e_WrCorrectFatSector != THRE_FatSectorNum ) {
			 		// 訂正書き込み処理
			 		if( st_MsgHDDTransInfo.em_HDD_RWMode == EM_HDD_RWMode_StoreFatRead ) {
						THRL_ThLogMsgErr("EM_HDD_RWMode_StoreFatRead check_sum Error Occur Correct Write Exec\n");	
			 			st_MsgHDDTransInfo.em_HDD_RWMode = EM_HDD_RWMode_StoreFatWrite;
			 		} else {
						THRL_ThLogMsgErr("EM_HDD_RWMode_StoreFatRead check_sum Error Occur Correct Write Exec\n");	
			 			st_MsgHDDTransInfo.em_HDD_RWMode = EM_HDD_RWMode_SwapFatWrite;
			 		}
			 		startHDDTransFatWrite( pcc_Fat, pcc_InfoFat, &semID_Mutex, e_WrCorrectFatSector );
			 	}
			}
		}
	}
	
	setHDDSeq( EM_HDDSeq_Idle );	// シーケンスを停止（待機状態へ）
	
	// debugLog 92に残す
	setDebugLowLogSpeedSeq( 391, st_MsgHDDTransInfo.em_HDD_RWMode );	
	
	// 転送完了を送信する　
	endTransFat();
}

Bool CP_HDDTrans::checkFatSize( void* p_RestoreArea )
{
// [RQ-560] CommonAPIからcheckFatSize()が複数回呼ばれる不具合に対応. すでに転送が始まっている場合、本関数は実行しない(無条件にTrueを返す) ->.
	if( mb_FatRestoreStartF == True ) {
		printf("[%s][%s] Restore has already started.\n",__FILE__,__FUNCTION__);
		return True;
	}
// [RQ-560] CommonAPIからcheckFatSize()が複数回呼ばれる不具合に対応. すでに転送が始まっている場合、本関数は実行しない(無条件にTrueを返す) <-.

	CC_Fat* pcc_Fat;
	uint32_t ul_MaxClusterNumMem	= 	0;
	uint32_t ul_MaxClusterNumHDD	= 	0;
	uint32_t ul_FatSizeImage 		=	0;
	
	pcc_Fat = (CC_Fat*)getBackupFatPtr( THRE_BackupTypeImage );
	if( NULL == pcc_Fat ){
		THRG_SetHDDRestoreLog( 17 );
		printf("[%s][%s] pcc_Fat is NULL!!\n",__FILE__,__FUNCTION__);
		return False;
	}

	ul_MaxClusterNumMem	= 	pcc_Fat->getMaxClusterNum();

// 2016/12/16 ishiga [IT5_4.1][HDDデータバックアップ世代間互換] リストアデータがIT4の場合の対応.
	if ( True == isIT4Ver_HddRestore() ) {
		ul_MaxClusterNumHDD =	DEF_HDD_HDDSTORE_CULSTER_NUM_IT4;	// IT4の場合.
	} else {
		ul_FatSizeImage 	=	pcc_Fat->getFatSize();
		ul_MaxClusterNumHDD =	pcc_Fat->readMaxClusterNumFromBuffer( p_RestoreArea, ul_FatSizeImage, 0 );
	}
	
	printf("[%s][%s] ul_MaxClusterNumMem:[%ld] ul_MaxClusterNumHDD:[%ld]\n",__FILE__,__FUNCTION__,ul_MaxClusterNumMem,ul_MaxClusterNumHDD);
	if( ul_MaxClusterNumMem >= ul_MaxClusterNumHDD ){
		THRG_SetHDDRestoreLog( 18, ul_MaxClusterNumMem, ul_MaxClusterNumHDD );
		return True;
	}else{
		THRG_SetHDDRestoreLog( 19, ul_MaxClusterNumMem, ul_MaxClusterNumHDD );
		return False;
	}
}
/*================================================================================================
［機能］	Fat情報（Maxクラスタ数含む）を読み込む.
--------------------------------------------------------------------------------------------------
［引数］	CC_Fat* pcc_Fat：Fatポインタ.
［戻値］	無し.
 [詳細]		Fat情報（Maxクラスタ数含む）を読み込む.
 [履歴]		16/05/23 Shinosaki 新規作成 MAv4 RQ-505 バックアップ機能強化対応.
			17/01/11 Sky井柳【IT5_4.0】[AR1011224]処理終了時のコールバック関数の実行追加.
	 		18/03/29 eSOL石田 HDDレスFax対応（Sparrow）.
			19/08/26 Sky鳥居 [EagleZ+] OP_BTS-11139対応.
================================================================================================*/
void CP_HDDTrans::remakeFat( CC_Fat* pcc_Fat )
{
	FuncCallstart( 5, __FUNCTION__, 0 );		// 割込みチェック用関数コールスタート.

#if defined(DEF_A4_Cus) // Sparrow.
	// AR1035078 制御構成変更_No.204 HDDレスFAX（Sparrow）.
	// HDDレスだとul_HDDStoredMaxClusterNumの値が異常になっている可能性がある為、
	// HDDがない場合は「バックアップ用remakeFat終了コールバック関数」以外実行しない.
	if( APIC_HddInfo::newInstance()->getHddExist() == True ){
#endif // Sparrow.

	IMGC_ImageMemoryPoolImpColorFileMemoryInstance& comMemoryManager = IMGC_ImageMemoryPoolImpColorFileMemorySingletonIf::newInstance();
	CC_Fat* cc_BackupFat = (CC_Fat*)comMemoryManager.getFatPtr( IMGE_IPEType_HDDBackup );			//バックアップ領域.
	
	uint32_t 	ul_FatSizeImage 	=	pcc_Fat->getFatSize();
	uint32_t	ul_MaxClusterNum	=	pcc_Fat->getMaxClusterNum();
	uint32_t	ul_MaxDefinedNum	=	theHDDTrans.getStorageInfo(THRE_StgInfoStoreClsNum);
	

	FuncCallend( 5, __FUNCTION__, 0 );			// 割込みチェック用関数コールエンド.
	// HDD転送実行、
	startHDDTransFatOneReadWrite( THRE_FatSectorFirst );
	FuncCallstart( 5, __FUNCTION__, 0 );		// 割込みチェック用関数コールスタート.

	// HDDに格納されているクラスタ最大数を読み込む.
	uint32_t	ul_HDDStoredMaxClusterNum	=	pcc_Fat->readMaxClusterNumFromBuffer( &uc_HDDbuffer[0], ul_FatSizeImage, 0 );
	
	SYSF_logMsg("[%s][%s] ul_MaxClusterNum:[%ld] ul_MaxDefinedNum:[%ld] ul_HDDStoredMaxClusterNum:[%ld]\n", __FILE__,__FUNCTION__,ul_MaxClusterNum,ul_MaxDefinedNum,ul_HDDStoredMaxClusterNum);
	if( ul_HDDStoredMaxClusterNum == ul_MaxDefinedNum ){
		//バックアップ機能が無効の場合は、蓄積画像領域を確保し直す.
		pcc_Fat->setMaxClusterNum( ul_HDDStoredMaxClusterNum );								//蓄積画像領域Fatに最大値をセットする.
	}else{
		//バックアップ機能が有効の場合は、バックアップ用の領域を確保する.
		cc_BackupFat->init((uint32_t)theHDDTrans.getStorageInfo(THRE_StgInfoBackupStartBlock),
			theHDDTrans.getStorageInfo(THRE_StgInfoBackupClsNum), 
			theHDDTrans.getStorageInfo(THRE_StgInfoCulsterSize) );
	}
	
#if defined(DEF_A4_Cus) // Sparrow.
	// AR1035078 制御構成変更_No.204 HDDレスFAX（Sparrow）.
	}
#endif // Sparrow.

	execBackupRemakeFatEndCallBackFunc( True );	// バックアップ用remakeFat終了コールバック関数の実行.

	FuncCallend( 5, __FUNCTION__, 0 );			// 割込みチェック用関数コールエンド.
}

// ================================================================================================.
//! <!--機能--> 【内部】 バックアップ用remakeFat終了コールバック関数の登録.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param void (*ans_RemakeFatEnd)(Bool bl_Result) ... 登録するコールバック関数.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! バックアップ用のremakeFat終了コールバック関数をメンバ変数設定する.
//! <!--履歴-->.
//! 17/01/11 Sky井柳【IT5_4.0】[AR1011224] 新規作成.
// ================================================================================================.
void CP_HDDTrans::setBackupRemakeFatEndCallBackFunc( void (*ans_RemakeFatEnd)(Bool bl_Result) )
{
	m_ans_BackupRemakeFatEndCallBackFunc = ans_RemakeFatEnd;
}

// ================================================================================================.
//! <!--機能--> 【内部】 バックアップ用remakeFat終了コールバック関数の実行・削除.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param Bool bl_Result ... remakeFatの結果.
//! <!--戻値-->
//! \return なし.
//! <!--詳細-->.
//! バックアップ用のremakeFat終了コールバック関数を実行する\n.
//! <!--履歴-->.
//! 17/01/11 Sky井柳【IT5_4.0】[AR1011224] 新規作成.
// ================================================================================================
void CP_HDDTrans::execBackupRemakeFatEndCallBackFunc( Bool bl_Result )
{
	if( m_ans_BackupRemakeFatEndCallBackFunc != NULL ){
		(*m_ans_BackupRemakeFatEndCallBackFunc)( bl_Result );	// 実行.
		m_ans_BackupRemakeFatEndCallBackFunc = NULL;
	}
}
/*================================================================================================
［機能］	ＦＡＴ転送書き込み処理
--------------------------------------------------------------------------------------------------
［引数］	CC_Fat* pcc_Fat
		CC_Fat* pcc_InfoFat
		SEM_ID* semID_Mutex;
		THRE_FatSector  e_WrCorrectFatSector;	書き込み要求セクタ番号、エラー発生時の訂正書き込みで使用
			THRE_FatSectorFirst:１つめの領域のみ訂正書き込み要求発生
			THRE_FatSectorSecond: ２つめの領域のみ訂正書き込み訂正要求発生
			指定なし：THRE_FatSectorNum: 通常シーケンス（１つめと２つめの両方に書き込む）
［戻値］	無し
 [詳細]		管理情報FAT、アレイ情報FATの書き込みパラメータの設定、チェックサムの計算を行う
 [履歴]		06/04/17 morikawa Mosel初版
			06/05/15 morikawa 読み出しでエラー発生時の訂正書き込みの処理のために第３引数追加
			12/11/21 ADS川口 CQ.377856  MS-LCL_3 CC_Fat動的確保対応.memcpyでのコピーをwriteBackupOldBuffer()/writeBackupBufferに変更.
			19/08/26 Sky鳥居 [EagleZ+] OP_BTS-11139対応.
================================================================================================*/
void CP_HDDTrans::startHDDTransFatWrite( CC_Fat* pcc_Fat, CC_Fat* pcc_InfoFat, SEM_ID* semID_Mutex, THRE_FatSector e_WrCorrectFatSector )
{
	FuncCallstart( 6, __FUNCTION__, 0 );		// 割込みチェック用関数コールスタート.

	SYSF_semTake( *semID_Mutex, WAIT_FOREVER ) ;		// FATファイルのI/Oを開始する前にMutex取得.
	THRL_ThAssert( (pcc_Fat != NULL), "pcc_Fat is NULL", 0 , 0, 0, 0, CFA14_DetailCode0x0B4D );
	THRL_ThAssert( (pcc_InfoFat != NULL), "pcc_InfoFat is NULL", 0 , 0, 0, 0, CFA14_DetailCode0x0B57 );
	// ２番目のpcc_InfoFatバッファのオフセット指定用に計算する　
	uint32_t ul_FatSizeImage = pcc_Fat->getFatSize();
	uint32_t ul_FatSizeImageInfo = pcc_InfoFat->getFatSize();
	// チェックサムのバッファのオフセット指定用に計算する　
	uint32_t ul_FatSize2 = ul_FatSizeImage + ul_FatSizeImageInfo;
	
 	// 書き込みバッファにデータコピー
	pcc_Fat->writeBackupBuffer( &uc_HDDbuffer[0], ul_FatSizeImage, 0 );
	pcc_InfoFat->writeBackupBuffer( &uc_HDDbuffer[ul_FatSizeImage], ul_FatSizeImageInfo, 0 );
	
	// dosFsの時はここでファイルオープン　
	// チェックサム計算 
	CT_CheckSum<ushort> sum ;
	sum.calcSum( &uc_HDDbuffer[0], ul_FatSizeImage ) ;
	sum.calcSum( &uc_HDDbuffer[ul_FatSizeImage], ul_FatSizeImageInfo ) ;
	ushort	us_Sum	= sum.getSum() ;

 	// 書き込みバッファにチェックサムコピー
 	_memcpy( &uc_HDDbuffer[ul_FatSize2], &us_Sum, sizeof(ushort) );
 	
 	SYSF_semGive( *semID_Mutex ) ;			// FATファイルのI/Oが終了したらMutex解放.
 	
 	// HDD転送実行、第１、第２の２つに書く、または、領域指定で書き込む　
 	if( e_WrCorrectFatSector == THRE_FatSectorNum || e_WrCorrectFatSector == THRE_FatSectorFirst ) {
		if( debug_FatTrans != 0 ) {
			THRL_ThLogMsgErr("startHDDTransFatWrite check_sum=%d pcc_Fat=%"PRIxPTR" pcc_InfoFat=%"PRIxPTR" e_FatSector=%"PRId32"\n",us_Sum,(uintptr_t)pcc_Fat,(uintptr_t)pcc_InfoFat,THRE_FatSectorFirst);	
		}
		FuncCallend( 6, __FUNCTION__, 0 );			// 割込みチェック用関数コールエンド.
 		startHDDTransFatOneReadWrite( THRE_FatSectorFirst );
		FuncCallstart( 6, __FUNCTION__, 0 );		// 割込みチェック用関数コールスタート.
 	}
 	
 	if( e_WrCorrectFatSector == THRE_FatSectorNum || e_WrCorrectFatSector == THRE_FatSectorSecond ) {
		if( debug_FatTrans != 0 ) {
			THRL_ThLogMsgErr("startHDDTransFatWrite check_sum=%d pcc_Fat=%"PRIxPTR" pcc_InfoFat=%"PRIxPTR" e_FatSector=%"PRId32"\n",us_Sum,(uintptr_t)pcc_Fat,(uintptr_t)pcc_InfoFat,THRE_FatSectorSecond);	
		}
		FuncCallend( 6, __FUNCTION__, 0 );			// 割込みチェック用関数コールエンド.
 		startHDDTransFatOneReadWrite( THRE_FatSectorSecond );
		FuncCallstart( 6, __FUNCTION__, 0 );		// 割込みチェック用関数コールスタート.
 	}
	FuncCallend( 6, __FUNCTION__, 0 );			// 割込みチェック用関数コールエンド.
}

/*================================================================================================
［機能］	ＦＡＴ転送読み出し処理
--------------------------------------------------------------------------------------------------
［引数］	CC_Fat* pcc_Fat
		CC_Fat* pcc_InfoFat
		SEM_ID* semID_Mutex;
［戻値］	THRE_FatSector
			THRE_FatSectorFirst:１つめの領域エラー発生、書き込み訂正要求発生
			THRE_FatSectorSecond: ２つめの領域エラー発生、書き込み訂正要求発生
			THRE_FatSectorNum: 正常終了or両方ともエラー発生のためアサート　
 [詳細]		管理情報FAT、アレイ情報FATの読み出しパラメータの設定、チェックサムの計算を行う
 [履歴]		06/04/17 morikawa Mosel初版
		06/05/15 morikawa 
			電源瞬断のデバッグにより
			１つめのFAT書き込み中に強制的にタスクサスペンドして電源OFFした時に
			その後の電源ONで１つめのFATがチェックサムエラーとなるが
			このときにbl_Errorを初期化していなかったため、２つめのFATのチェックでもbl_ErrorがTrueのままで
			強制的にFATを初期化してしまっていた問題修正。
			対策１：bl_ErrorはチェックサムがOKの時はFalseを設定するように修正
			      さらに、２つともチェックサムエラーの時はシーケンス異常でアサートするように修正　	
			対策２：２つめの領域書き込み中に電源がOFFされて、再度電源ONした時に１つめの読み出しは成功するが
				このときさらに１つめの書き込みで電源OFFされることを考慮して
				チェックサムエラー発生の領域のみ訂正書き込みを行うように第３引数追加及び修正　      	
		06/05/22 yoshii		FAT２つともチェックサムエラーの時はトラブルコードD030に変更   
		07/05/14 morikawa 静的解析不具合修正　   	
		12/11/21 ADS川口 CQ.377856  MS-LCL_3 CC_Fat動的確保対応.memcpyでのコピーをreadBackupOldBuffer()/readBackupBuffer()に変更.
		15/10/22 大岡		[ZeusSZX0]制御構成変更No.76-1 新詳細コードデータ収集機能.
		19/08/26 Sky鳥居 [EagleZ+] OP_BTS-11139対応 新詳細コードデータ収集出力データ変更.
================================================================================================*/
THRE_FatSector CP_HDDTrans::startHDDTransFatRead( CC_Fat* pcc_Fat, CC_Fat* pcc_InfoFat, SEM_ID* semID_Mutex )
{
	register Bool	bl_Error  = False;		// ファイルエラー（エラー無しで初期化）
	Bool bl_ReadOk	= False ;	// 読み出しOK時にTrue
	THRE_FatSector e_WrCorrectFatSector = THRE_FatSectorNum;	// 訂正書き込み用セクタ番号初期化　

	FuncCallstart( 7, __FUNCTION__, 0 );		// 割込みチェック用関数コールスタート.

	SYSF_semTake( *semID_Mutex, WAIT_FOREVER ) ;		// FATファイルのI/Oを開始する前にMutex取得.
	THRL_ThAssert( (pcc_Fat != NULL), "pcc_Fat is NULL", 0 , 0, 0, 0, CFA14_DetailCode0x0B4E );
	THRL_ThAssert( (pcc_InfoFat != NULL), "pcc_InfoFat is NULL", 0 , 0, 0, 0, CFA14_DetailCode0x0B58 );
	// ２番目のpcc_InfoFatバッファのオフセット指定用に計算する　
	uint32_t ul_FatSizeImage = pcc_Fat->getFatSize();
	uint32_t ul_FatSizeImageInfo = pcc_InfoFat->getFatSize();
	// チェックサムのバッファのオフセット指定用に計算する　
	uint32_t ul_FatSize2 = ul_FatSizeImage + ul_FatSizeImageInfo;
	ushort	us_SumLog[2] = { 0 };
	
	// 転送モードだけ渡す、コールされた中でSimWinであればStoreファイルとSwapファイルを分けて読み出し　
	for( THRE_FatSector e_FatSector = 0; e_FatSector<THRE_FatSectorNum; ++e_FatSector ) {
	
		FuncCallend( 7, __FUNCTION__, 0 );			// 割込みチェック用関数コールエンド.
		// HDD転送実行、
		startHDDTransFatOneReadWrite( e_FatSector );
		FuncCallstart( 7, __FUNCTION__, 0 );		// 割込みチェック用関数コールスタート.
		
		// チェックサム読み出し 
		ushort	us_Sum = 0;
 		// 読み出しバッファからチェックサムをコピー
 		_memcpy( &us_Sum, &uc_HDDbuffer[ ul_FatSize2 ], sizeof(ushort) );
 		
		CT_CheckSum<ushort> sum ;
		sum.calcSum( &uc_HDDbuffer[0], ul_FatSizeImage );
		sum.calcSum( &uc_HDDbuffer[ul_FatSizeImage], ul_FatSizeImageInfo );

		if( debug_FatTrans != 0 ) {
			THRL_ThLogMsgErr("uc_HDDbuffer 0=[%d],1=[%d],2=[%d],3=[%d]\n",uc_HDDbuffer[0],uc_HDDbuffer[1],uc_HDDbuffer[2],uc_HDDbuffer[3]);	
			THRL_ThLogMsgErr("uc_HDDbuffer ul_FatSizeImage=[%d],1=[%d],2=[%d],3=[%d]\n",
				uc_HDDbuffer[ul_FatSizeImage],uc_HDDbuffer[ul_FatSizeImage+1],uc_HDDbuffer[ul_FatSizeImage+2],uc_HDDbuffer[ul_FatSizeImage+3]);	
			THRL_ThLogMsgErr("startHDDTransFatRead us_sum=%d pcc_Fat=%"PRIxPTR" pcc_InfoFat=%"PRIxPTR"\n",us_Sum,(uintptr_t)pcc_Fat,(uintptr_t)pcc_InfoFat,0);	
		}
		
		if ( us_Sum != sum.getSum() ) {
			bl_Error = True;
			us_SumLog[0] = us_Sum;
			us_SumLog[1] = sum.getSum();
			THRL_ThLogMsgErr( "startHDDTransFatRead One CheckSum Error e_FatSector=%d\n",e_FatSector,2,3,4);
		} else {
			bl_Error = False;	// 06/05/15 履歴参照、チェックサムOKの時はErrorフラグリセット　
		}
		
		if( bl_Error == True )	{
			// チェックサムエラー発生時は訂正書き込み要求セット　
			e_WrCorrectFatSector = e_FatSector;
			// (void)remove( pstrTbl[nIndex] );
		} else if( bl_ReadOk == False ) {
			// まだ一度もバッファにデータをセットしていない時のみ、１回だけセット
			// 読み出しOK
			bl_ReadOk = True ;
 			// 読み出しバッファからデータコピー
			pcc_Fat->readBackupBuffer( &uc_HDDbuffer[0], ul_FatSizeImage, 0 );
			pcc_InfoFat->readBackupBuffer( &uc_HDDbuffer[ul_FatSizeImage], ul_FatSizeImageInfo, 0 );
			if( debug_FatTrans != 0 ) {
				THRL_ThLogMsgErr("startHDDTransFatRead bl_ReadOk True Set Buffer Set\n");	
			}
			// 06/05/15 ２つめがエラーの可能性があるので必ず２回読み出す。
			// break;
		}	
	}
	// Fatサイズが0でなければトラブル登録する.
	if ( ( bl_ReadOk == False ) && ( ul_FatTotalSize != 0 ) ){
	//if ( bl_ReadOk == False ){
		// 両方ともエラー発生なので訂正書き込み要求はセットしない
		e_WrCorrectFatSector = THRE_FatSectorNum;
		// ２つもチェックサムエラーの時はトラブルコードD030　
		THRL_ThLogMsgErr( "startHDDTransFatRead All CheckSum Error ReadSum[%lu] CalSum[%lu]\n", us_SumLog[0], us_SumLog[1] );
		// 新詳細コード保存.
		uchar			uc_KindId = 1;
		uint64_t		ull_NewDetailData = THRD_NewDetail_ModuleId_Thread;		// 1,2桁：スレッド.
		ull_NewDetailData |= (uint64_t)( TYPD_TroubleCode_HDD_FAT_READ_ERROR ) << THRD_NewDetail_TroubleCode_Shift;	// 3-6桁：トラブルコード.
		//ull_NewDetailData |= (uint64_t)( us_SumLog[0] ) << 24;					// 7-10桁：読みだしたSUM値.
		//ull_NewDetailData |= (uint64_t)( us_SumLog[1] ) << 8;						// 11-14桁：SUM計算値.

		// 19/08/26 Sky鳥居 [EagleZ+] OP_BTS-11139対応 新詳細コードデータ収集出力データ変更.
		if((uchar)0 == m_uc_FuncCallPointIntErr[1][0] ){
			// 優先度高順でログ有り側の種別を設定.
			// 優先度：0:uc_HDDbuffer <  1:puc_WorkMemAddr.
			uc_KindId = 0;
		}
		ull_NewDetailData |= (uint64_t)( m_uc_FuncCallIntErrCnt[uc_KindId] )  << 32;		//  7- 8桁：割込みエラーカウント値.
		ull_NewDetailData |= (uint64_t)( m_uc_FuncCallPointIntErr[uc_KindId][0] ) << 24;		//  9-10桁：割込み関数ID.
		ull_NewDetailData |= (uint64_t)( m_uc_FuncCallPointIntErr[uc_KindId][1] ) << 16;		// 11-12桁：割込まれ関数ID.
		ull_NewDetailData |= (uint64_t)( m_uc_FuncCallPointIntErr[uc_KindId][2] ) <<  8;		// 13-14桁：割込み関数ID(１つ前).
		ull_NewDetailData |= (uint64_t)( m_uc_FuncCallPointIntErr[uc_KindId][3] );			// 15-16桁：割込まれ関数ID(１つ前).
		THRL_ThAssertLog2( 0, ull_NewDetailData );
		DebugForever( TYPD_TroubleCode_HDD_FAT_READ_ERROR );		// 06/05/22 yoshii トラブルコード対応追加　
		pcc_Fat->init() ;		// 事前に実施した初期化時のパラメータで初期化.
		pcc_InfoFat->init() ;	// 事前に実施した初期化時のパラメータで初期化.
	}
	SYSF_semGive( *semID_Mutex ) ;			// FATファイルのI/Oが終了したらMutex解放.
	
	FuncCallend( 7, __FUNCTION__, 0 );			// 割込みチェック用関数コールエンド.

	return( e_WrCorrectFatSector );
}

/*================================================================================================
［機能］	ＦＡＴ転送のHDDへのRW実行　
--------------------------------------------------------------------------------------------------
［引数］	THRE_FatSector e_FatSector : FATセクタ番号識別
［戻値］	無し
 [詳細]		管理情報FAT、アレイ情報FATの書き込み／読み出しを行う
 [履歴]		06/04/17 morikawa Mosel初版
 			07/05/14 morikawa 静的解析不具合修正　
 			08/10/16 takayama CQ:311209 SSD 領域割り当て動的変更対応.
 			12/01/20 takayama CQ360543 FD残る問題対策 ファイルアクセスAPIをラッパー関数に置き換え.
			2018/08/03 sky simogaki cramfs領域破壊問題(AR1051769).
			2018/09/20 sky yamagishi AR1058652 設試機にて、Boxにファイルを正常に保存できなくて、「ボックスに登録しているページ数が上限値です」と表示される。.
			2019/08/26 Sky鳥居 [EagleZ+] OP_BTS-11139対応.
================================================================================================*/
void CP_HDDTrans::startHDDTransFatOneReadWrite( THRE_FatSector e_FatSector  )
{
	// SimWinでもシーケンスのみ確認できるようにする　
	uint32_t	ul_Sector = 0;			// HDD書き込み先頭番号　
	// HDD書き込み読み出し転送識別を書き込みで初期化　
	THRE_ReadWriteHDDMode e_ReadWriteHDDMode = THRE_ReadWriteBufToHDD;

	FuncCallstart( 8, __FUNCTION__, 0 );		// 割込みチェック用関数コールスタート.

	// 転送方向決定　
	switch( st_MsgHDDTransInfo.em_HDD_RWMode ) {
		case EM_HDD_RWMode_StoreFatRead:
		case EM_HDD_RWMode_SwapFatRead:
			// OneReadWriteに渡すパラメータをアレイの転送と同じにするために設定　
			e_ReadWriteHDDMode = THRE_ReadWriteHDDToBuf;
			break;
		case EM_HDD_RWMode_StoreFatWrite:
		case EM_HDD_RWMode_SwapFatWrite:
			// OneReadWriteに渡すパラメータをアレイの転送と同じにするために設定　
			break;
		default:
			break;
	}

// ◇ 2008/10/16 takayama CQ:311209 SSD 領域割り当て動的変更対応.
#ifdef DEF_SupportSSD
	// 転送領域（蓄積orSWAP)決定.
	switch( st_MsgHDDTransInfo.em_HDD_RWMode ) {
		case EM_HDD_RWMode_StoreFatRead:
		case EM_HDD_RWMode_StoreFatWrite:
			if( e_FatSector == THRE_FatSectorFirst ) {
				ul_Sector = ul_SectorFatStoreFirst;
			} else {
				ul_Sector = ul_SectorFatStoreSecond;
			}
			break;
		case EM_HDD_RWMode_SwapFatRead:
		case EM_HDD_RWMode_SwapFatWrite:
			if( e_FatSector == THRE_FatSectorFirst ) {
				ul_Sector = ul_SectorFatSwapFirst;
			} else {
				ul_Sector = ul_SectorFatSwapSecond;
			}
			break;
		default:
			break;
	}
#else //DEF_SupportSSD.	
	
	// 転送領域（蓄積orSWAP)決定　
	switch( st_MsgHDDTransInfo.em_HDD_RWMode ) {
		case EM_HDD_RWMode_StoreFatRead:
		case EM_HDD_RWMode_StoreFatWrite:
			if( e_FatSector == THRE_FatSectorFirst ) {
				ul_Sector = DEF_SECTOR_FAT_STORE_FIRST;
			} else {
				ul_Sector = DEF_SECTOR_FAT_STORE_SECOND;
			}
			break;
		case EM_HDD_RWMode_SwapFatRead:
		case EM_HDD_RWMode_SwapFatWrite:
			if( e_FatSector == THRE_FatSectorFirst ) {
				ul_Sector = DEF_SECTOR_FAT_SWAP_FIRST;
			} else {
				ul_Sector = DEF_SECTOR_FAT_SWAP_SECOND;
			}
			break;
		default:
			break;
	}
#endif
	// debugLog 92
	if( debug_FatTrans != 0 ) {
		// ログが多くなるのでデバッグモードだけログに残す　
	 	setDebugLowLogSpeedSeq( 321, ul_Sector );
	} 	

//////////////////////////////////////////////////////
// SATA2対応 Start
//////////////////////////////////////////////////////
	// 転送アレイ初期化
	memset( ul_TransArrayAdr, 0, sizeof( ul_TransArrayAdr ) );
#ifdef DEF_SupportSSD
	uint32_t ul_SectorNum = 0;
	if ( bl_SwapFatOnlyDisable ) {	// Swapのみ無効の場合.
		if ( ( st_MsgHDDTransInfo.em_HDD_RWMode == EM_HDD_RWMode_StoreFatRead )||( st_MsgHDDTransInfo.em_HDD_RWMode == EM_HDD_RWMode_StoreFatWrite ) )  {
			// 蓄積ならサイズ計算より読み込む.
			// HDDへの転送セクタ数は、FATのトータルサイズから求める.
			 ul_SectorNum = ul_FatTotalSize / DEF_SECTOR_SIZE;
		} else {
			// SwapならFat領域はないので0固定.
			// Swapの場合は0に設定.
			 ul_SectorNum = 0;
		}
	} else {
			// EgaleLowの8Gもこのケースを通るがul_FatTotalSizeが0なので、問題ない.
			// HDDへの転送セクタ数は、FATのトータルサイズから求める.
			ul_SectorNum = ul_FatTotalSize / DEF_SECTOR_SIZE;
	}
#else //DEF_SupportSSD.		
	// HDDへの転送セクタ数は、FATのトータルサイズから求める。
	uint32_t ul_SectorNum = DEF_FAT_TOTAL_SIZE / DEF_SECTOR_SIZE;
#endif	// DEF_SupportSSD.

	// HDD転送とAES転送を実行する　	
	transDirectHDDAndAES( e_ReadWriteHDDMode, (uintptr_t)uc_HDDbuffer, ul_Sector, ul_SectorNum );
			
//////////////////////////////////////////////////////
// SATA2対応 End
//////////////////////////////////////////////////////

	FuncCallend( 8, __FUNCTION__, 0 );			// 割込みチェック用関数コールエンド.
}
/*================================================================================================
［機能］	ＨＤＤ転送ＦＡＴの転送完了を通知する（FAT情報）
--------------------------------------------------------------------------------------------------
［引数］	なし
［戻値］	なし
 [詳細]		ＨＨＤＤ転送ＦＡＴの転送完了を通知する
 [履歴]		06/04/17 morikawa Mosel初版 
 			07/07/24 yokoi	HDD分割バックアップ対応 
 			16/12/19 Sky井柳【IT5_4.0】[AR1007099] バックアップ用Fat変更コールバック関数の実行追加.
================================================================================================*/
void CP_HDDTrans::endTransFat()
{
	if( p_RestoreAreaBak == NULL ){	// 分割リストアシーケンスで、一時データ上書き処理中で無ければ 
		// Fatの転送が完了したので、バックアップ処理側にFat転送完了した事を通知する 
		execBackupCallBackFunc( True );
	}

	execBackupFatChangeCallBackFunc( True );	// バックアップ用Fat変更コールバック関数の実行.

#ifdef TORNADO		// VXWORKSで有効 
	sendMessage( TID_Image, TM_HDDFatDataEndNotice, 0 );
#else
	setDelayMessage( TID_Image, TM_HDDFatDataEndNotice, 0, ul_ImageDelayTimer );
#endif	// #ifdef TORNADO
}
// HDD_FatTrans End

#if defined(DEF_StartFromSSD) && defined(TORNADO)
extern "C" {
	void THRF_semTakeDOSForSSD ();
	void THRF_semGiveDOSForSSD ();
}

SEM_ID dosImageMuteSemCopy = dosImageMuteSem;

#endif // defined(DEF_StartFromSSD) && defined(TORNADO)

// ================================================================================================
//! <!--機能--> HDD転送デバッグ処理
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param なし
//! <!--戻値-->
//! \return なし
//! <!--詳細-->
//! この行以下はHDDデバッグようの処理です。
//! <!--履歴-->
//  07/03/07 morikawa	Donau初版
//  07/05/14 morikawa	静的解析不具合修正
//  13/06/13 suzuki		メモリ削減対応.THRD_ENABLE_AST_ERASEDATALOGで有効/無効を切り替え.
// ================================================================================================
uint32_t ul_EraseDataLogCounter		= 0 ;	// ログカウンタ.
#define	DEF_ERASEDATALOGNUM		500
#define	DEF_ERASEDATALOGCOUNT	((UINT_MAX/DEF_ERASEDATALOGNUM)*DEF_ERASEDATALOGNUM)

//メモリ削減のためにログ格納をコンパイルスイッチ化.
//#define THRD_ENABLE_AST_ERASEDATALOG	//メモリ削減対応.
#ifdef THRD_ENABLE_AST_ERASEDATALOG
ST_EraseDataLog ast_EraseDataLog[DEF_ERASEDATALOGNUM] ;	// ログ格納領域(サイズに注意)
#endif	//THRD_ENABLE_AST_ERASEDATALOG.


// ログ1データ登録.
void setEraseDataLog( uchar uc_Mode, void* pst_Log )
{
#ifdef THRD_ENABLE_AST_ERASEDATALOG
	ST_EraseDataLog	st_LogTmp ;
	_memset( (void*)&st_LogTmp, sizeof(ST_EraseDataLog) ) ;
	st_LogTmp.ul_Tick	= SYSF_tickGet() ;
	st_LogTmp.uc_Mode	= uc_Mode ;
	switch( uc_Mode ) {
		case 0x10:
			// 出力レベル0以上(常時)
			st_LogTmp.un.stFileLog		= *((ST_EraseDataFileLog*)pst_Log) ;
			break ;
		case 0x20:
			// 出力レベル1以上(1未満はログ登録しないで抜ける)
			if ( us_debug_eraselog_verbose < 1 ) {
				return ;
			}
			st_LogTmp.un.stClusterLog	= *((ST_EraseDataClusterLog*)pst_Log) ;
			break ;
		case 0xF0:
			// 出力レベル0以上(常時)
			st_LogTmp.un.stStatusLog	= *((ST_EraseDataStatusLog*)pst_Log) ;
			break ;
		default:
			break ;
	}

	ast_EraseDataLog[ul_EraseDataLogCounter%DEF_ERASEDATALOGNUM]	= st_LogTmp ;
	++ul_EraseDataLogCounter;
	ul_EraseDataLogCounter	= (ul_EraseDataLogCounter%DEF_ERASEDATALOGCOUNT) ;
#endif //THRD_ENABLE_AST_ERASEDATALOG.
}

// ログ1データ取得.
Bool getEraseDataLog( uint32_t ul_Index, ST_EraseDataLog* pst_Log )
{
#ifdef THRD_ENABLE_AST_ERASEDATALOG
	Bool	bl_Ret	= False ;
	// 注意：ul_EraseDataLogCounterがループして0に戻った場合は正確ではなくなる.
	if ( ul_Index < ul_EraseDataLogCounter ) {
		bl_Ret	= True ;
		*pst_Log	= ast_EraseDataLog[ul_Index%DEF_ERASEDATALOGNUM] ;
	}
	return bl_Ret ;
#else
	return False;
#endif	//THRD_ENABLE_AST_ERASEDATALOG.
}
extern "C" {
void dspEraseDataLog();
}
void dspEraseDataLog()
{
	//2009/03/30 newInstance対応 comMemoryManagerのインスタンス生成.
	IMGC_ImageMemoryPoolImpColorFileMemoryInstance& comMemoryManager = IMGC_ImageMemoryPoolImpColorFileMemorySingletonIf::newInstance();

	const static char* pStateMsg[] = {
		"End",
		"Fill",
		"Compare"
	} ;

	printf("EraseData Log Num=[%"PRIu32"]\n", ul_EraseDataLogCounter) ;

	ST_EraseDataLog		st_Log ;
	for ( uint32_t ul_Index = 0; ul_Index < DEF_ERASEDATALOGNUM; ul_Index++ ) {
		printf( "N[%08"PRIu32"]%c",ul_Index, ((ul_EraseDataLogCounter%DEF_ERASEDATALOGNUM)==ul_Index)?'*':' ' ) ;
		if ( getEraseDataLog( ul_Index, &st_Log ) == False ) {
			// ログ終了.
			printf("\n--- end ----------------------------------------------------------\n");
			break ;
		}

		printf( "%8"PRIu32"", st_Log.ul_Tick ) ;

		switch( st_Log.uc_Mode ) {
			case 0x10:
				{
					ST_EraseDataFileLog* p = &st_Log.un.stFileLog ;

					// 開始クラスタNo.より実セクタNo.を取得する.
					CC_HDDBlock	c_HDDBlock			= CC_HDDBlock() ;
					c_HDDBlock.pcc_Fat				= comMemoryManager.getFatPtr( p->en_IPEType ) ;
					c_HDDBlock.ec_IPEType			= p->en_IPEType ;
					c_HDDBlock.cc_StartClusterNo	= p->ul_StartClusterNo ;
					uint32_t	ul_SectorNo	= (uint32_t)-1 ;
					if ( c_HDDBlock.pcc_Fat != NULL ) {
						ul_SectorNo	= c_HDDBlock.calcClusterToSector( p->ul_StartClusterNo ) ;
					}

					printf( "[F  ]" ) ;
					printf( "%5s "			, (p->en_IPEType == IMGE_IPEType_HDDStore)?"Store":"Swap" ) ;
					printf( "M:%d "			, p->en_EraseMode ) ;
					printf( "Cluster:%8"PRIu32" "	, p->ul_StartClusterNo ) ;
					printf( "Sector:%8"PRIu32" "	, ul_SectorNo ) ;
					printf( "Size:%"PRIu32" "		, p->ul_FileSize ) ;
					printf( "\n" ) ;
				}
				break ;
			case 0x20:
				{
					ST_EraseDataClusterLog* p = &st_Log.un.stClusterLog ;
					printf( "[ C ]" ) ;
					printf( "Sector:%8"PRIu32" "	, p->ul_SectorNo ) ;
					printf( "Num:%4u  "		, p->us_SectorNum ) ;
					printf( "Act[%7s] "		, pStateMsg[p->uc_Action%3] ) ;
					printf( "Ptn[%02X] "	, p->uc_Ptn ) ;
					printf( "\n" ) ;
				}
				break ;
			case 0xF0:
				{
					ST_EraseDataStatusLog* p = &st_Log.un.stStatusLog ;
					printf( "[  S]" ) ;
					if ( p->uc_Status == 0 ) {
						printf( "Status[OK  ] " ) ;
					} else {
						printf( "Status[  NG] Sector=[%"PRIu32"] ", p->ul_NGSector ) ;
					}
					printf( "\n" ) ;
				}
				break ;
			default:
				break ;
		}
	}
}
extern "C" {
	void dspEraseMode() ;
}

void dspEraseMode()
{
	// 一時データ上書き消去する・しない
	if ( NVDC_SystemSetting::newInstance()->getTempDataOverWriteEraseOn() == True ) {
		printf( " Erase: Enable\n" ) ;
	} else {
		printf( " Erase: Disable\n" ) ;
	}
	// 一時データ上書き消去モード Mode1・Mode2
	TYPE_TempDataOverWriteErase en_Mode = NVDC_SystemSetting::newInstance()->getTempDataOverWriteErase();
	switch( en_Mode ) {
		case TYPE_TempDataOverWriteErase2:	// モード2
			printf( "  Erase Mode 2\n" ) ;
			break ;
		case TYPE_TempDataOverWriteErase1:	// モード1
		default :
			printf( "  Erase Mode 1\n" ) ;
			break ;
	}
	// 暗号化有効・無効
// IT6では、メインストレージの暗号化のみ.
#if 0 // サブストレージ暗号化対応の為に、コードを残しておく.
	if ( NVDC_SystemSetting::newInstance()->getEnableEncryption() == True ) {
#else //0.
	if ( THRG_IsSsdEncrypt() == True ) {
#endif //0.
		printf( "  Encryption: Enable\n" ) ;
	} else {
		printf( "  Encryption: Disable\n" ) ;
	}
	// 暗号化優先・上書き優先.
	if ( NVDC_SystemSetting::newInstance()->getEncryptionOrOverWrite() == True ) {
		printf( " EncryptionOrOverWrite: Encryption\n" ) ;
	} else {
		printf( " EncryptionOrOverWrite: OverWrite\n" ) ;
	}
}

// HDD一時データ上書き消去関連ステータス.
void dspEraseStatus()
{
	//2009/03/30 newInstance対応 comMemoryManagerのインスタンス生成.
	IMGC_ImageMemoryPoolImpColorFileMemoryInstance& comMemoryManager = IMGC_ImageMemoryPoolImpColorFileMemorySingletonIf::newInstance();

	printf("isHDDReady()              = %s\n",comMemoryManager.isHDDReady()==True?"True":"False") ;
	printf("getHDDStatus()thread      = %d\n",comMemoryManager.getHDDStatus()) ;
	printf("getHddStatus()common      = %d\n",APIC_HddInfo::newInstance()->getHddStatus()) ;
	APIC_SystemStatus* pc_Status = APIC_SystemStatus::newInstance() ;
	printf("getServiceModeOn()        = %s\n",pc_Status->getServiceModeOn()==True?"True":"False") ;
	printf("getAdminModeOnApp()       = %s\n",pc_Status->getAdminModeOnApp()==TYPE_AdminModeAppPanel?"AdminPanel":"else");
	printf("getEraseReqWaitNum() swap = %u\n",comHDDEraseSwap.getEraseReqWaitNum());
	printf("getEraseReqWaitNum() store= %u\n",comHDDEraseStore.getEraseReqWaitNum());
	printf("getOverWriteEraseInfo(0)  = %"PRId32"\n", FILC_OverWriteEraseIf::getOverWriteEraseInfo(0));
	printf("getOverWriteEraseState()  = ");
	{
		switch ( FILC_OverWriteEraseIf::getOverWriteEraseState() ) {
			case TYPE_EraseState_Wait:
				printf("Wait\n");
				break;
			case TYPE_EraseState_Active:
				printf("Active\n");
				break;
			default:
				printf("Unknown\n");
				break;
		}
	}
	printf("chkWriteFatFile() swap    = %s\n",comMemoryManager.chkWriteFatFile(IMGE_IPEType_Non)==True?"True":"False");
	printf("chkWriteFatFile() store   = %s\n",comMemoryManager.chkWriteFatFile(IMGE_IPEType_HDDStore)==True?"True":"False");
}

/*================================================================================================
［機能］	ターミナルコマンド
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
 [詳細] 	
			
［履歴］	04/07/20 t.sasaki Rhein1初版(セキュリティ対応)
			05/03/29 t.sasaki コマンドを分かりにくく変更.
			05/06/20 t.sasaki 暗号化ASIC対応.
			08/03/27 kurachi sEtLoCkPaSsWoRd()の追加.
			08/10/02 鈴村 CQ309716_dspEncryptKeyの配列が足りないので修正.
			18/07/25 Yamagishi [Eagle][制御構成変更 No.99]【ストレージ構成変更】ストレージ管理(フォーマット_ロック_暗号化) パスワード取得API変更 (dSpLoCkPaSsWoRd).
================================================================================================*/
extern "C"{
void sEtLoCkPaSsWoRd( char sc_HDDPassword[] );	// 2008/03/27 kurachi NVRAMにロックパスワードを設定する.
void dSpLoCkPaSsWoRd();
void dspHddStatus();
void setEnableEncrypt( ushort on ) ;
void setEncryptKey( char* pchKey );
//void compareEncryptKey( char* pchKey );
void dspEncryptStatus() ;
}
#define DEBUG_ENCDEC_SIZE 32

void sEtLoCkPaSsWoRd( char sc_HDDPassword[] )
{
	(NVDC_SystemSetting::newInstance())->setHddLockPassword( sc_HDDPassword );
	printf( "LockPassword Set To NVRAM!! pass = [%s]\n", sc_HDDPassword );
}

void dSpLoCkPaSsWoRd()
{
	char sc_Pass[256] ;
	NVDC_SystemSetting *pc_Instance = NVDC_SystemSetting::newInstance();
	if (NULL == pc_Instance) {
		printf("Failed to get NVDC_SystemSetting instance.\n");
		return;
	}
	pc_Instance->getHddLockPassword_SPIFlash( sc_Pass ) ;

	printf("LockPassword:[%s]\n", &sc_Pass[0]);
}
void dspHddStatus()
{
	printf("WarningError:[%s]\n", (theHDDTrans.isWarrning() == True) ? "True":"False") ;		// 警告エラー
	printf("SecurityError:[%s]\n", (theHDDTrans.isSecurityError() == True) ? "True":"False") ;	// セキュリティエラー
}

// S 2005/06/20 t.sasaki SSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSS
void setEnableEncrypt( ushort on )
{
	Bool bl_Enable = False ;
	if ( on == 1 ) {
		bl_Enable = True ;
	}
// 2016/02/01 CQ521285 監査ログHCD-PP対応	Sky平田	Start.
	NVDC_SystemSetting::newInstance()->setEnableEncryption( bl_Enable, JLGE_InspectLogOperateIDL_System, JLGE_InspectLogIfOTHER, False );
// 2016/02/01 CQ521285 監査ログHCD-PP対応	Sky平田	End.
}
void setEncryptKey( char* pchKey )
{
	if ( (pchKey == NULL)
	  || (strlen(pchKey) > 20) ) {
		printf( " Input Key(Max 20bytes)\n" ) ;
		return ;
	}
// 2016/02/01 CQ521285 監査ログHCD-PP対応	Sky平田	Start.
	NVDC_SystemSetting::newInstance()->setEncryptionPassword( pchKey, JLGE_InspectLogOperateIDL_System, JLGE_InspectLogIfOTHER, False );
// 2016/02/01 CQ521285 監査ログHCD-PP対応	Sky平田	End.
}
void chgToSaveFormatEncryptASICKey( char* ) ;
void chgFromSaveFormatEncryptASICKey( char* ) ;
// 2016/04/11 Sky大嶋 CQ526818 ストレージ暗号化パスワードのハッシュ化対応により削除.
//void compareEncryptKey( char* pchKey )
//{
//	if ( (pchKey == NULL)
//	  || (strlen(pchKey) > 20) ) {
//		printf( " Input Key(Max 20bytes)\n" ) ;
//		return ;
//	}
//	printf( " Input=[%s]\n", pchKey ) ;
//	printf( "\n" ) ;
//	printf( "        +0+1+2+3+4 +5+6+7+8+9 +0+1+2+3+4 +5+6+7+8+9\n" ) ;
//
//	char chKey[32] ;
//	char* p = NULL ;
//	int32_t i = 0;
//	// hash(input)
//	memset( chKey, '\x0', sizeof(chKey) ) ;
//	memcpy( chKey, pchKey, 20 ) ;
//	chgToSaveFormatEncryptASICKey( chKey ) ;
//	chgFromSaveFormatEncryptASICKey( chKey ) ;
//	printf( " HASH : " ) ;
//	p = &chKey[0] ;
//	for ( i=0; i<20; i++ ) {
//		printf( "%02X", (uchar)*p++ ) ;
//		if ( ((i+1)%5)==0 ) {
//			printf(" ");
//		}
//	}
//	printf( "\n" ) ;
//
//	// hash(NVRAM)
//	memset( chKey, '\x0', sizeof(chKey) ) ;
//	NVDC_SystemSetting::newInstance()->getEncryptionPassword( chKey );
//	printf( " NVRAM: " ) ;
//	p = &chKey[0] ;
//	for ( i=0; i<20; i++ ) {
//		printf( "%02X", (uchar)*p++ ) ;
//		if ( ((i+1)%5)==0 ) {
//			printf(" ");
//		}
//	}
//	printf( "\n" ) ;
//
//	// compare
//	printf( "\n" ) ;
//	Bool bl_Compare = NVDC_SystemSetting::newInstance()->chkEncryptionPassword( pchKey ) ;
//	printf( " Compare=[%s]\n", bl_Compare?"OK":"NG" ) ;
//}
Bool getEncryptASICStatus( uchar* puc_Exist, uchar* puc_KeyError, uint32_t* pul_Status ) ;
void dspEncryptStatus()
{
	uchar uc_Exist = 0;
	uchar uc_KeyError = 0;
	uint32_t ul_Status = 0;
	Bool bl_Init = getEncryptASICStatus( &uc_Exist, &uc_KeyError, &ul_Status ) ;
	printf( " Init=[%s] Exist=[%s] KeyError=[%s] Status=[0x%08"PRIX32"]\n",
		(bl_Init)?"YES":"--", (uc_Exist)?"EXIST":"--", (uc_KeyError)?"ERR":"--", ul_Status ) ;

}

// HDD系ステータスログ全部を出力.
// 2005/12/20 added by t.sasaki
void dspHddStatusAll()
{
	printf("---------------------\n\n") ;
	printf("-> dspHddStatus()\n") ;
	dspHddStatus() ;
	printf("---------------------\n\n") ;
	printf("-> dspEraseStatus()\n") ;
	dspEraseStatus() ;
	printf("---------------------\n\n") ;
	printf("-> dspEraseMode()\n") ;
	dspEraseMode() ;
	printf("---------------------\n\n") ;
	printf("-> dspEncryptStatus()\n") ;
	dspEncryptStatus() ;
	printf("---------------------\n\n") ;
	printf("-> dspGSIO(0)\n") ;
	dspGSIO(0) ;
	printf("---------------------\n\n") ;
	printf("-> dspStorageBusy()\n") ;
	dspStorageBusy() ;
	printf("---------------------\n\n") ;
}


// ================================================================================================
//! <!--機能--> HDDのアレイの転送を画像転送Simが有効な時のみ行なう
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param EM_HDD_RWMode	：読み込み/書き込みモード
//! \param ulong			：ブロックの先頭のアレイアドレス
//! \param ul_Sector		：開始セクタ番号
//! \param ulong  			:ページ先頭セクタ番号からのオフセット（ページの先頭時は0、ページの最後は0xFFFFFFFFとする）
//! <!--戻値-->
//! \return なし
//! <!--詳細-->
//! ページの先頭バンドをファイル名としたファイルにアレイ単位でRead/Writeする
//!  書き込み時：圧縮時にJBIGリンクテーブルにaddされたバッファサイズをファイルに最初に書き込み、その後、実データを書き込む
//!  読み出し時：ファイルから復帰したバッファサイズを最初に読み出し、その後、実データを読み出す
//! <!--履歴-->
//  07/07/02 morikawa	Donau初版
//  07/08/20 morikawa	画Simが安定するまで暫定で何もしないようにしていたが、実行するように正式対応する　
//  07/12/12 koshino	画Simファイルメモリのファイル化対応
//  12/01/20 takayama CQ360543 FD残る問題対策 ファイルアクセスAPIをラッパー関数に置き換え.
//  13/02/14 Takatsuji	FA14のアサート文字列のリファイン.
// ================================================================================================
// 2013/07/02 KM(morikawa) SoC LinuxSim JFM merge.
// LinuxSimの時も、ThreadSimWinのFax送受信ために変更．
// 2017/06/15 KM(tanaka.y) 警告対応のため、関数の宣言をコンパイルスイッチで囲む．
// #if defined(SIMULATOR) && !defined(FUM_LINUX)
#if defined(SIMULATOR) 	
int debugArrayLog = 0;	// デバッグ用変数　
void readWriteArraySim( EM_HDD_RWMode em_HDD_RWMode, uintptr_t ul_ArrayAdr, uint32_t ul_Sector, uint32_t ul_SectorOffset )
{
	int si_errno = 0;
	
	// 画像転送Simの有効状態を確認
	if( false == THRF_isActiveImageTransSim() ){	// DLLが無効状態
		// HDDにアクセスせずに終了する
		return;
	}

	// TRACE( "<<readWriteArraySim>>\n" );
	static char	str[ DEF_PATH_LENGTH ];
	static FILE *fp = NULL;
	if( ul_SectorOffset == 0 ) {	
		sprintf( str,"array-%08"PRIX32".dat", ul_Sector );
		if( em_HDD_RWMode == EM_HDD_RWMode_Read ) {
			fp = SYSF_Fopen( str, "rbe" );
		} else {
			fp = SYSF_Fopen( str, "wbe" );
		}
	}

	// Moselは毎回アレイの先頭アドレスがくる
	if( fp != NULL ) {
		if( ul_SectorOffset != 0xFFFFFFFF ) {
			// PC上のワークメモリアドレス　
			uintptr_t ul_Adr = 0;
			// １回のファイルアクセスで読み書きするサイズ　
			uint32_t ul_Size = 0;
			int32_t sl_Result = 0;
			if( em_HDD_RWMode == EM_HDD_RWMode_Read ) {
				// バッファサイズをHDDから読み出し
				sl_Result = fread( (void *)&ul_Size , sizeof(ul_Size), 1, fp );
				THRL_ThAssert( sl_Result != 0, "readWriteArraySim ReadMode SizeRead Error!! errno %d", (uintptr_t)errno, 0, 0, 0, CFA14_DetailCode0x0200 );
				THRL_ThAssert( ul_Size != 0, "readWriteArraySim ReadMode Size is 0!!", 0, 0, 0, 0, CFA14_DetailCode0x0201 );

				// リンクテーブル追加処理
				if( THRF_GetOptionStatus().b_IsFile_FileMem == true ){	// テーブルデータファイル保存時は領域を確保してHDDから読み込み 
					ul_Adr = (uintptr_t)( new uchar[ul_Size] );
				}else{	// テーブルデータをファイル保存しない
					// HDDから復帰したサイズで、リンクテーブル追加　
					THRF_addDataJbigLinkTbl( ul_ArrayAdr, ul_Size, &ul_Adr );
				// Synchronization issue with AppIn: Temporary fix suggested by KM-san on 2014-03-10
					if( ul_Adr == 0 ){
						TRACE( "<<THRF_addDataJbigLinkTbl>> Temporary fix is called\n");
                        THRF_getDataJbigLinkTbl( ul_ArrayAdr, &ul_Adr, &ul_Size );
					}
				}

				THRL_ThAssert( ul_Adr != 0, "readWriteArraySim ReadMode Addr NULL!!", 0, 0, 0, 0, CFA14_DetailCode0x0202 );

				if( debugArrayLog == 1 ) {
					TRACE( "<<ReadArraySim>> MemAddr=0x%"PRIxPTR" ul_Size=%"PRIdPTR" ul_Adr=0x%"PRIxPTR" \n", ul_ArrayAdr, ul_Size, ul_Adr );
				}
				// HDDから１ブロック読み出し	
				// HDDからデータ復帰後に伸張するアレイのデータにデータをセットする 
				sl_Result = fread( (void *)ul_Adr , ul_Size, 1, fp );
				si_errno = errno; // if文を出たところで、sl_Resultでアサートがあるので、それに対応.
			 
			 	// 異常画像になるかのデバッグ用
			  	if( debugArrayLog == 2 ) {
					TRACE( "##<<ReadArraySim>> No Exec debug Image Chk\n");
					memset( (void *)ul_Adr, 0xFF, ul_Size);
			  	}	

				if( THRF_GetOptionStatus().b_IsFile_FileMem == true ){	// テーブルデータファイル保存時のみファイル保存
					THRF_ITSSaveJBIGLinkTableDataFile( (void*)ul_Adr, ul_Size, ul_ArrayAdr );
					delete[] (uchar *)ul_Adr;
				}
			} else {
				// 圧縮時にaddされたデータのアドレスを取り出す 
				if( THRF_GetOptionStatus().b_IsFile_FileMem == true ){	// テーブルデータファイル保存時は領域を確保してHDDから読み込み 
					// テーブルデータがファイル保存されている場合は領域を確保して読み込み 
					ul_Size = THRF_ITSGetJBIGLinkTableDataSize( ul_ArrayAdr );
					THRL_ThAssert( ul_Size != 0, "readWriteArraySim WriteMode Size is 0!!", 0, 0, 0, 0, CFA14_DetailCode0x0203 );
					ul_Adr = (uintptr_t)( new uchar[ul_Size] );
					THRF_ITSReadJBIGLinkTableDataFile( (uchar *)ul_Adr, ul_ArrayAdr );
				}else{
					THRF_getDataJbigLinkTbl( ul_ArrayAdr, &ul_Adr, &ul_Size );
				}

				THRL_ThAssert( ul_Adr != 0, "readWriteArraySim WriteMode Addr NULL!!", 0, 0, 0, 0, CFA14_DetailCode0x0204 );

				if( debugArrayLog == 1 ) {
					TRACE( "<<WriteArraySim>> MemAddr=0x%"PRIxPTR" ul_Size=%"PRIdPTR" ul_Adr=0x%"PRIxPTR" \n", ul_ArrayAdr, ul_Size, ul_Adr );
				}
				// バッファサイズをHDDに記憶
				sl_Result = fwrite( (void *)&ul_Size , sizeof(ul_Size), 1, fp );
				THRL_ThAssert( sl_Result != 0, "readWriteArraySim WriteMode SizeWrite Error!! ferror %d", (uintptr_t)ferror(fp), 0, 0, 0, CFA14_DetailCode0x0205 );
				THRL_ThAssert( ul_Size != 0, "readWriteArraySim WriteMode Size is 0!!", 0, 0, 0, 0, CFA14_DetailCode0x0206 );
				
				// HDDに１ブロック書き込み
				sl_Result = fwrite( (void *)ul_Adr , ul_Size, 1, fp );
				si_errno = errno; // if文を出たところで、sl_Resultでアサートがあるので、それに対応.
				// HCL-Veera[01/24/2014] added support to clear memory during buf2hdd tranasfer only in case of 1GB memory size
				if( APIC_MemoryInfo::newInstance()->getWorkMemorySize() <= DIMM_MEMSIZE_ALL2 ) {
					THRF_ItsClearFileMemory(ul_ArrayAdr);
				}
				// テーブルデータファイル保存時のみテーブル削除
				if( THRF_GetOptionStatus().b_IsFile_FileMem == true ){ 
					delete[] (uchar *)ul_Adr;
				}

			}
			THRL_ThAssert( sl_Result != 0, "readWriteArraySim FileReadWrite Error!! errno %"PRId32"", (uintptr_t)si_errno, 0, 0, 0, CFA14_DetailCode0x0207 );
		} else {
			// ページ最終終了処理　
			SYSF_fclose( fp );
			fp = NULL;
		}	
	} else {
		THRL_ThAssert( False, "readWriteArraySim FileOpen Error!! errno %d", (uintptr_t)SYSF_errnoGet(), 0, 0, 0, CFA14_DetailCode0x0208 );
	}
}
#endif	/* defined(SIMULATOR) 	*/

#define testSecSize 2000
static void ReadFileToPCScsiLib(char* cpSaveFileName, char * cpFileBuff, int32_t iFileSize);
static int32_t read_index = 0;

int32_t aaa = 0;

/********************************************************************************/
/* Function Name : sysBspSATARead_Test                                          */
/*         Input : int           DeviceId   : 要求デバイスID                    */
/*                 int           Lun        : 要求論理ユニット番号              */
/*                 unsigned long Secter     : 指定開始セクタ                    */
/*                 long          RSize 　   : 読み出しセクタ数                  */
/*        Output : void                                                         */
/*       Summary : HDDに対してセクタ指定で Read Writeを行うテスト               */
/*［履歴］                                                                      */
/*  12/01/30 N.Kato     静的解析対応 バッファ領域初期化サイズ修正               */
/*  12/02/04 takayama   静的解析対応 静的解析対応 return箇所でのfree漏れ修正    */
/*  12/08/28 Gonsho     静的解析対応 静的解析対応 正常ケースでのfdのcloseを追加 */
/*  13/02/14 Takatsuji	FA14のアサート文字列のリファイン.                       */
/*  17/10/10 Yoneda     S800 ストレージ構成変更対応                             */
/********************************************************************************/
void sysBspSATARead_Test(int32_t DeviceId, int32_t Lun, uint32_t Secter, uint32_t RSize)
{
#ifdef TORNADO
	int32_t fd_hdd;
	int64_t l_syscall_Ret = 0;
	off_t l_read_ret = 0;
	char* buff = NULL;
	int32_t* buff2;
	char	pSaveFilename[128] = "sataRead%d.dat";
	uint32_t	readSec = 0;
	
	buff = (char*)malloc(512*testSecSize);
	if(buff == NULL) {
		printf("Buff == NULL\n");
		return;
	}
	memset(buff, 0, 512*testSecSize);
	if (DeviceId == 0) {
//		fd_hdd = open( "/dev/kmsdb", (O_RDWR);
		fd_hdd = SYSF_open( theHDDTrans.pc_strDeviceSSD, O_RDWR, 0666 );
	} else {
//		fd_hdd = open( "/dev/kmsda", (O_RDWR | O_DIRECT));
		fd_hdd = SYSF_open( theHDDTrans.pc_strDeviceSD, O_RDWR, 0666 );
	}
	if(fd_hdd < 0) {
		// open失敗.
		printf("[SATA2]fail open command. errno %d\n", SYSF_errnoGet());
		free(buff);
		return;		//2012/2/4 静的解析対応 takayama.
	}
	if (aaa == 0) {
		l_syscall_Ret = lseek( fd_hdd, (int64_t )(Secter * 512), SEEK_SET );
		if (l_syscall_Ret < 0) {
			// 失敗.
			printf( "[SATA2]fail lseek command fd_hdd %"PRId32" ul_Sector %"PRId32" l_syscall_Ret %"PRIx32" errno %d\n",(uint32_t)fd_hdd,(uint32_t)Secter,(uint32_t)l_syscall_Ret, errno );
			free(buff);
			SYSF_close(fd_hdd);
			return;		//2012/2/4 静的解析対応 takayama.
		}
	}
	while(readSec < RSize) {
//		l_syscall_Ret = read( fd_hdd, (char *)buff ,testSecSize*512 );
		l_read_ret = SYSF_read( fd_hdd, (char *)buff ,(int64_t )testSecSize*512 );
		if (l_read_ret < 0) {
			// 読み書き失敗.	
			printf("[SATA2]fail read/write command  Error!! errno %d", SYSF_errnoGet());
		}
		buff2 = (int32_t*)buff;
		printf("%02x,",*buff2);
		printf("%02x,",*(buff2++));
		printf("%02x,",*(buff2++));
		printf("%02x,",*(buff2++));
		printf("\n");
		/* Read File To PC */
		ReadFileToPCScsiLib(pSaveFilename, (char *)buff, (int32_t)testSecSize*512);
		readSec += testSecSize;
		printf("%"PRId32" %"PRId32"\n",readSec,RSize);
	}
	free(buff);
	l_syscall_Ret = SYSF_close(fd_hdd);
	if ( l_syscall_Ret < 0 ) {
		printf( "[SATA2]fail SYSF_close command \n" );
	}
#endif
	return;
}

/********************************************************************************/
/* Function Name : ReadFileToPCScsiLib                                          */
/*         Input : char* cpSaveFileName   :   PC保存用ファイル名                */
/*                 char * cpFileBuff      :   書き込みデータ                    */
/*                 int iFileSize          :   書き込みファイルサイズ            */
/*        Output : void                                                         */
/*       Summary : Read File To PC                                              */
/*［履歴］                                                                      */
/*  13/02/14 Takatsuji	履歴追加.                                               */
/*  13/02/14 Takatsuji	FA14のアサート文字列のリファイン.                       */
/********************************************************************************/
static void ReadFileToPCScsiLib(char* cpSaveFileName, char * cpFileBuff, int32_t iFileSize)
{
#ifdef TORNADO
	int32_t fd2;		/* save fileディスクリプタ */
	ssize_t size;		//2017/01/23 int32_tからssize_tに変更.

	char cFilePass[128] = "/bd/sdc1/";

	/* ファイルPASS */
	strcat(cFilePass, cpSaveFileName);
	sprintf(cFilePass,cFilePass,read_index);
	read_index++;
	
	printf("PC SAVE FILE NAME = %s    iFileSize = %"PRId32"\n", cFilePass, iFileSize);
	
	/* save ファイルオープン */
	fd2 = SYSF_devopen(cFilePass, O_CREAT | O_RDWR, 0666);
	if(fd2 < 0)
	{
		printf("ReadFileToPC : OPEN ERROR. errno %d\n", SYSF_errnoGet());
		return;
	}

	/* save */
	size = write(fd2, cpFileBuff, (int32_t)iFileSize);
	if(size == 0)
	{
		printf("WriteFileToPC : Write ERROR. errno %d\n", errno);
		SYSF_devclose(fd2);
		return;
	}

	/* save ファイルクローズ */
	SYSF_devclose(fd2);
#endif
	return;
}

/********************************************************************************/
/* Function Name : sysBspSATADump                                               */
/*         Input : int           mode	    : 読み込みデバイスモード            */
/*											  0=HDD、1=SSD                      */
/*                 unsigned long Secter     : 指定開始セクタ                    */
/*                 long          RSize 　   : 読み出しセクタ数                  */
/*        Output : void                                                         */
/*       Summary : SATAに対してセクタ指定で Read を行い結果をUSBに保存する      */
/********************************************************************************/
extern "C" void sysBspSATADump(int32_t mode, uint32_t Secter, uint32_t RSize)
{

	if (mode == 0) {
		printf("======= HDD Dump Start  =======\n");
		sysBspSATARead_Test(0,0,Secter,RSize);
	} else if (mode == 1) {
		printf("======= SSD Dump Start  =======\n");
		sysBspSATARead_Test(1,0,Secter,RSize);
	} else {
		printf("======= Device selection error  =======\n");
	}
	printf("======= Dump End  =======\n");
}

// 2017/11/06 Sky井柳 [Eagle][制御構成変更No.97] SWAP領域/Subset展開領域の併用 3->10に変更.
#define	DEF_SNAPSHOT_PART	THRM_MOUNT_DEV( THRD_DEVFILE_SSD, 10 )
// 2018/06/06 Sky松井 [Eagle][AR.1046712] FAT16指定のオプションを削除.
#define	DEF_FORMAT_VFAT		"mkfs.vfat -v -c %s"
int32_t CP_HDDTrans::system_chk( char *cmd, char *cMsg ){
#ifdef TORNADO
#if !defined(DEF_MinervaPreEmu) && !defined(DEF_ZeusRevCus)
	int32_t i_ret = system( cmd );

	if( WIFEXITED(i_ret) ){	// 子プロセスが正常終了したか確認.
		if(WEXITSTATUS(i_ret) != 0){	
			printf("[%s] error(%"PRId32")\n", cMsg,  WEXITSTATUS(i_ret));
			return WEXITSTATUS(i_ret);
		}
	}else{
		printf("[%s]Child process error(%"PRId32")\n", cMsg, WIFEXITED(i_ret));
		return WIFEXITED(i_ret);
	}
#endif
#endif
	return 0;
}

/********************************************************************************/
/* Function Name : sysBspStgInfoDump                                            */
/*         Input : void                                                         */
/*        Output : void                                                         */
/*       Summary : HDDに対してセクタ指定で Read を行いダンプ出力する            */
/* Modificate History                                                           */
/*  2017/10/10 Yoneda   S800 ストレージ構成変更対応                             */
/********************************************************************************/
static void dmpHddInfo(const char * FileName, uint32_t Sector, uint32_t Count)
{
#ifdef TORNADO
	FILE *fp = NULL;
	int32_t fd;
	char Buff[512] = { 0 };

	printf("[SATA2]Sector:%"PRId32" Count:%d \n", Sector, Count );

	// パラメータチェック.
	if( FileName == NULL ){
		printf( "[SATA2]null filename \n" );
		return;
	}

	// デバイスオープン.
	fd = SYSF_open( theHDDTrans.pc_strDeviceSSD, O_RDWR, 0666 );
	if( fd < 0 ){												// open失敗.
		printf( "[SATA2]fail open command \n" );
		return;
	}

	// ストリーム移動.
	if( lseek( fd, (int64_t)Sector * 512, SEEK_SET ) < 0 ){	// seek失敗.
		SYSF_close( fd );
		printf( "[SATA2]fail lseek command Sector %"PRId32"\n", Sector );
		return;
	}

	// ファイルオープン.
	fp = fopen( FileName, "wb" );
	if( fp == NULL ){											// fopen失敗.
		SYSF_close( fd );
		printf( "[SATA2]fail fopen command \n" );
		return;
	}

	while( Count-- ){
		// セクタ読込み.
		if( SYSF_read( fd, Buff , sizeof(Buff) ) < 0 ){			// read失敗.
			printf( "[SATA2]fail read command \n" );
			break;
		}

		// ファイル書込み.
		if( fwrite( Buff, 1, sizeof(Buff), fp ) < 0 ){		// fwrite失敗.
			printf( "[SATA2]fail fwrite command \n" );
			break;;
		}
	}

	// ストリームクローズ.
	fclose( fp );
	SYSF_close( fd );
#endif
	return;
}

extern "C" void sysBspStgInfoDump(void)
{
#ifdef TORNADO
	printf("======= Dump Start =======\n");
	dmpHddInfo( "/bd/sdc1/HDD_DUMP_FatStore1.dat",
				theHDDTrans.getStorageInfo(THRE_StgInfoSectFatStore1),
				theHDDTrans.getStorageInfo(THRE_StgInfoSectFatSize) );

	dmpHddInfo( "/bd/sdc1/HDD_DUMP_FatStore2.dat",
				theHDDTrans.getStorageInfo(THRE_StgInfoSectFatStore2),
				theHDDTrans.getStorageInfo(THRE_StgInfoSectFatSize) );

	dmpHddInfo( "/bd/sdc1/HDD_DUMP_StoreInfo.dat",
				theHDDTrans.getStorageInfo(THRE_StgInfoStoreInfoStartBlock),
				theHDDTrans.getStorageInfo(THRE_StgInfoStoreInfoBlkNum) );
	printf("======== Dump End ========\n");
#endif
	return;
}

// ================================================================================================
//! <!--機能--> InternetISW 設定.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param なし
//! <!--戻値-->
//! \return なし
//! <!--詳細-->
//!  スナップショット領域をInternetISWで使用できるようにする。
//! <!--履歴-->
//  11/08/19 karasaki	MinervaZeus初版
//  12/01/30 N.Kato     静的解析対応 不要ログ削除
//  12/06/06 miyamoto   新アーキ対応
//	13/01/08 Takatsuji  AES暗号正式対応によりSubset起動ファイルを追加.
//	13/04/01 Takatsuji  TMP対応によりSubset起動ファイルを追加.
// ================================================================================================
Bool CP_HDDTrans::internetISW_setting( void )
{
#ifdef TORNADO
#if !defined(DEF_MinervaPreEmu) && !defined(DEF_ZeusRevCus)
	char cmd[256];
	int32_t i_ret;
	
	setWarpInfo_ChgMFPData(True);
	
	/* 領域FATフォーマット */
	snprintf( cmd, sizeof(cmd), DEF_FORMAT_VFAT, DEF_SNAPSHOT_PART );
	printf(cmd);
	i_ret = system( cmd );

	if( WIFEXITED(i_ret) ){	// 子プロセスが正常終了したか確認.
		if(WEXITSTATUS(i_ret) != 0){
			printf("internetISW_setting format error(%"PRId32")\n", WEXITSTATUS(i_ret));
			printf("cmd=(%s)\n", cmd);
			return False;
		}
	}else{
		printf("internetISW_setting Child process error(%"PRId32")\n", WIFEXITED(i_ret));
		return False;
	}
	/* mount */
	snprintf( cmd, sizeof(cmd), "mount %s /mnt", DEF_SNAPSHOT_PART );
	if( system_chk( cmd, "mount NG" ) != 0 ){
		return False;
	}
	system( "exec mkdir /mnt/fw" );
	system( "exec mkdir /mnt/fw/bin" );
	system( "exec mkdir /mnt/fw/lib" );
	system( "exec mkdir /mnt/fw/lib/modules");
	/* InternetISW起動用ファイルコピー */
	if( system_chk( "exec cp /boot/* /mnt",			"uImage copy" ) != 0 ){
		system( "exec umount /mnt" );
		return False;
	}
	if( system_chk( "exec cp /km/fw/bin/mfp001_hwQt /mnt/fw/bin",		"mfp001_hwQt copy" ) != 0 ){
		system( "exec umount /mnt" );
		return False;
	}
	if( system_chk( "exec cp /km/fw/bin/mfp001_hwQt.map /mnt/fw/bin",	"mfp001_hwQt.map copy" ) != 0 ){
		system( "exec umount /mnt" );
		return False;
	}
	if( system_chk( "exec cp /km/fw/bin/SPS_RSHA /mnt/fw/bin",			"SPS_RSHA copy" ) != 0 ){
		system( "exec umount /mnt" );
		return False;
	}
	if( system_chk( "exec cp /km/libQt* /mnt",							"libQt* copy" ) != 0 ){
		system( "exec umount /mnt" );
		return False;
	}
	if( system_chk( "exec cp -r /km/module /mnt",						"/module copy" ) != 0 ){
		system( "exec umount /mnt" );
		return False;
	}
	if( system_chk( "exec cp /km/fw/lib/libshare_adapter.so /mnt/fw/lib","libshare_adapter.so copy" ) != 0 ){
		system( "exec umount /mnt" );
		return False;
	}
	if( system_chk( "exec cp /km/fw/lib/libccme_base.so /mnt/fw/lib","libccme_base.so copy" ) != 0 ){
		system( "exec umount /mnt" );
		return False;
	}
	if( system_chk( "exec cp /km/fw/lib/libccme_ecc.so /mnt/fw/lib","libccme_ecc.so copy" ) != 0 ){
		system( "exec umount /mnt" );
		return False;
	}
	if( system_chk( "exec cp /km/fw/lib/libccme_eccaccel.so /mnt/fw/lib","libccme_eccaccel.so copy" ) != 0 ){
		system( "exec umount /mnt" );
		return False;
	}
	if( system_chk( "exec cp /km/fw/lib/libccme_eccnistaccel.so /mnt/fw/lib","libccme_eccnistaccel.so copy" ) != 0 ){
		system( "exec umount /mnt" );
		return False;
	}
	if( system_chk( "exec cp /km/fw/lib/libcryptocme2.so /mnt/fw/lib","libcryptocme2.so copy" ) != 0 ){
		system( "exec umount /mnt" );
		return False;
	}
	if( system_chk( "exec cp /km/fw/lib/libcryptocme2.sig /mnt/fw/lib","libcryptocme2.sig copy" ) != 0 ){
		system( "exec umount /mnt" );
		return False;
	}
	if( system_chk( "exec cp /km/fw/lib/libcryptocme2_test_on_use.sig /mnt/fw/lib","libcryptocme2_test_on_use.sig copy" ) != 0 ){
		system( "exec umount /mnt" );
		return False;
	}
	if( system_chk( "exec cp /km/fw/lib/libtaw.so /mnt/fw/lib","libtaw.so copy" ) != 0 ){
		system( "exec umount /mnt" );
		return False;
	}
	if( system_chk( "exec cp /km/fw/lib/libiTss.so /mnt/fw/lib","libiTss.so copy" ) != 0 ){
		system( "exec umount /mnt" );
		return False;
	}
	if( system_chk( "exec cp /km/fw/lib/modules/tpm.ko /mnt/fw/lib/modules","tpm.ko copy" ) != 0 ){
		system( "exec umount /mnt" );
		return False;
	}
	if( system_chk( "exec cp /km/fw/lib/modules/tpm_nuvoton_i2c.ko /mnt/fw/lib/modules","tpm_nuvoton_i2c.ko copy" ) != 0 ){
		system( "exec umount /mnt" );
		return False;
	}
	system( "exec umount /mnt" );
	
	sync();
	sync();
	sync();
#endif
#endif
	return True;
}

// ================================================================================================
//! <!--機能--> IISW更新準備処理.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param const char *psc_filePath 更新ファイルパス.
//! <!--戻値-->
//! \return Bool    True:成功, False:失敗
//! <!--詳細-->
//!  指定したファイルパスに存在する更新ファイルを、一時領域(ハイバネーション領域)に展開・格納する.
//! <!--履歴-->
// 2013/11/08 yamada    UV-01385 更新対象ファイル中のSubsetにてFW更新を可能とする対応.
// 2013/12/03 yamada    UV-01385 ファームウェア更新失敗時のロールバック機能対応.
// 2014/01/24 yamada    CQ384576 IISW時のバックアップに失敗する問題点対応.
// 2014/10/15 yamada	CQ502327 IISW分割更新時のtar展開に失敗する対応(正式対応).
// 2018/06/13 Sky松井	AR1046712 [Eagle]tarファイル展開コマンドへオプション追加.
// 2020/09/28 伊藤史. 	OP_BTS-27780,27963 INDEXの値を書き換えることで、改ざんしたFWがダウンロード可能となる脆弱性の対応.
// ================================================================================================
Bool CP_HDDTrans::prepareIISWUpdate(const char *psc_FilePath)
{
    APIC_HddInfo* pc_HddInfo = NULL;
    char sc_Cmd[256+THRD_MaxLength_TarFilePath] = {0};
    int32_t si_Ret = 0;
    FILE *fp = NULL;
    char sc_Buf[256];

    memset(sc_Buf, 0x00, sizeof(sc_Buf) );

    APIC_SystemInfo* pc_SystemInfo = APIC_SystemInfo::newInstance();
    if (pc_SystemInfo == NULL) {
        THRL_SetPanelHardLog(8);
        return False;
    }

    // パーティション構成がUV-01385の適用が可能かをチェックする.
    if (pc_SystemInfo->isHeliosPFPart() == False) {
        THRL_SetPanelHardLog(9);
        return False;
    }

    // 引数NULLチェック.
    if (psc_FilePath == NULL) {
        THRL_SetPanelHardLog(10);
        return False;
    }

    // 引数長さチェック.
    if (strlen(psc_FilePath) >= THRD_MaxLength_TarFilePath) {
        THRL_SetPanelHardLog(11, (uintptr_t)strlen(psc_FilePath));
        return False;
    }

    // ファイル存在チェック.
    if (access(psc_FilePath, F_OK) != 0) {
        THRL_SetPanelHardLog(12, (uintptr_t)psc_FilePath);
        return False;
    }

    // スナップショットの再取得が必要な事をブートローダーに通知.
    setWarpInfo_ChgMFPData(True);

    /* 領域FATフォーマット */
    snprintf(sc_Cmd, sizeof(sc_Cmd), DEF_FORMAT_VFAT, DEF_SNAPSHOT_PART);
    if (system_chk(sc_Cmd, "format NG") != 0) {
        return False;
    }

    /* mount */
    int32_t ret = mount(DEF_SNAPSHOT_PART, "/mnt", "vfat", MS_NOATIME, NULL );
    if (ret == -1) {
        strerror_r(errno, sc_Buf, sizeof(sc_Buf));
        THRL_SetPanelHardLog(27, (uintptr_t)errno, (uintptr_t)sc_Buf);
        return False;
    }

    // TARファイル解凍.
    snprintf(sc_Cmd, sizeof(sc_Cmd), "exec tar xfm %s --no-same-owner -C /mnt", psc_FilePath);
    if (system_chk(sc_Cmd, "tar command NG") != 0) {
        umount2("/mnt", MNT_FORCE);
        return False;
    }

	// 2020/09/28 伊藤史. OP_BTS-27780,27963 INDEXの値を書き換えることで、改ざんしたFWがダウンロード可能となる脆弱性の対応.
    // 完全性チェック実行.
    int i_ret = 0;
	struct stat st_stat;

    snprintf(sc_Cmd, sizeof(sc_Cmd), "exec grityinte 2 /mnt/");
    i_ret = system_chk(sc_Cmd, "grityinte NG");
	// チェック後は、削除しておく.
	if(SYSF_stat(DEF_PRVNTFALSE_ENV, &st_stat) == 0){
		THRF_System( DEF_PRVNTFALSE_ERACE );
	}
	if( 0 != i_ret ){
        umount2("/mnt", MNT_FORCE);
        return False;
    }

	// 2021/07/09 A800レス実機への旧ROMのNWDLガード対応.
#if defined(DEF_EagleCus) && !defined(DEF_A800) && !defined(DEF_HeliosMLKCus)
	if( THRF_IsA800LessModel() )
	{
		// A800レスモデルの場合.
		if( !isA800LessFW( "/mnt" ) )
		{
			// A800レス非対応のFW.
	        umount2("/mnt", MNT_FORCE);
			return False;
		}
	}
#endif

    // HDD装着確認に必要なクラスのインスタンス生成.
    pc_HddInfo = APIC_HddInfo::newInstance();
    if (pc_HddInfo == NULL) {
        THRL_SetPanelHardLog(13);
        umount2("/mnt", MNT_FORCE);
        return False;
    }

    // HDD装着時はファイルパスを保存する.
    if (pc_HddInfo->getHddExist() == True) {
        snprintf(sc_Cmd, sizeof(sc_Cmd), "/mnt/%s", THRD_FileName_UpdateFwTar);
        fp = fopen(sc_Cmd, "w");
        if (fp == NULL) {
            strerror_r(errno, sc_Buf, sizeof(sc_Buf));
            THRL_SetPanelHardLog(14, (uintptr_t)sc_Cmd, (uintptr_t)errno, (uintptr_t)sc_Buf);
            umount2("/mnt", MNT_FORCE);
            return False;
        }

        // 作成したファイルに更新ファイルパスを書き込む.
        si_Ret = fputs(psc_FilePath, fp);
        if (si_Ret == EOF) {
            THRL_SetPanelHardLog(15, (uintptr_t)psc_FilePath);
            umount2("/mnt", MNT_FORCE);
            fclose(fp);
            return False;
        }

        // ファイルストリームクローズ.
        fclose(fp);
    }

    // アンマウント.
    si_Ret = umount2("/mnt", MNT_FORCE);
    if (si_Ret == -1) {
        strerror_r(errno, sc_Buf, sizeof(sc_Buf));
        THRL_SetPanelHardLog(16, (uintptr_t)errno, (uintptr_t)sc_Buf);
        return False;
    }

    return True;
}

// ================================================================================================
//! <!--機能--> IISW更新準備処理.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param const char *psc_FwPath	更新ファイルパス.
//! \param const char *psc_SigPath	署名ファイルパス.
//! <!--戻値-->
//! \return Bool    True:成功, False:失敗
//! <!--詳細-->
//!  指定したファイルパスに存在する更新ファイルと署名ファイルを使用して署名検証を行い、
//!  検証に成功した場合は、一時領域(ハイバネーション領域)に展開・格納する.
//! <!--履歴-->
// 2017/06/12 Shibahara [SecV2-2] ファームウェア更新時の真正性検証技術.
// 2017/11/09	SCD2-Sky Shindo [WP.53-54] 設定値見直し対応.
// ================================================================================================
Bool CP_HDDTrans::prepareIISWUpdate(const char *psc_FwPath, const char *psc_SigPath)
{

	APIC_SystemSetting *pc_SecuritySystemSetting = APIC_SystemSetting::newInstance();
	if (NULL == pc_SecuritySystemSetting) {
	    printf("[prepareIISWUpdate] Flash Access Class Error\n");
	    return False;
	}
	if (pc_SecuritySystemSetting->getSWUpdateVerifyMode()) {
	    if (!(verifySignature(psc_FwPath, psc_SigPath))) {
	        printf("[prepareIISWUpdate] verify signature ERR\n");
	        return False;
	    }
	}

	return prepareIISWUpdate(psc_FwPath);
}

// ================================================================================================
//! <!--機能--> ロールバック更新準備処理.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param なし.
//! <!--戻値-->
//! \return Bool    True:成功, False:失敗
//! <!--詳細-->
//!  バックアップ領域に存在する更新ファイルを、一時領域(ハイバネーション領域)に展開・格納する.
//! <!--履歴-->
// 2013/12/03 yamada    UV-01385 ファームウェア更新失敗時のロールバック機能対応.
// 2014/10/15 yamada	CQ502327 IISW分割更新時のtar展開に失敗する対応(正式対応).
// 2016/05/16 Shibahara CQ502327 [CQ526161] ファームウェア更新検証設定が有効な状態でロールバック時の改竄を検出しない問題の修正.
// 2018/06/13 Sky松井	AR1046712 [Eagle]tarファイル展開コマンドへオプション追加.
// ================================================================================================
Bool CP_HDDTrans::prepareFWRollback(void)
{
    char psc_Cmd[256+THRD_MaxLength_TarFilePath] = {0};
    int32_t si_Ret = 0;
    char sc_Buf[256];

    memset(sc_Buf, 0x00, sizeof(sc_Buf) );

    APIC_SystemInfo* pc_SystemInfo = APIC_SystemInfo::newInstance();
    if (pc_SystemInfo == NULL) {
        THRL_SetPanelHardLog(17);
        return False;
    }

    // パーティション構成がUV-01385の適用が可能かをチェックする.
    if (pc_SystemInfo->isHeliosPFPart() == False) {
        THRL_SetPanelHardLog(18);
        return False;
    }

    // ファイル存在チェック.
    if (access(THRD_FilePath_RollBackFw, F_OK) != 0) {
        THRL_SetPanelHardLog(19, (uintptr_t)THRD_FilePath_RollBackFw);
        return False;
    }

    // スナップショットの再取得が必要な事をブートローダーに通知.
    setWarpInfo_ChgMFPData(True);

    /* 領域FATフォーマット */
    snprintf(psc_Cmd, sizeof(psc_Cmd), DEF_FORMAT_VFAT, DEF_SNAPSHOT_PART);
    if (system_chk(psc_Cmd, "format NG") != 0) {
        return False;
    }

    /* mount */
    int32_t ret = mount(DEF_SNAPSHOT_PART, "/mnt", "vfat", MS_NOATIME, NULL );
    if (ret == -1) {
        strerror_r(errno, sc_Buf, sizeof(sc_Buf));
        THRL_SetPanelHardLog(28, (uintptr_t)errno, (uintptr_t)sc_Buf);
        return False;
    }

    // TARファイル解凍.
    snprintf(psc_Cmd, sizeof(psc_Cmd), "exec tar xfm %s --no-same-owner -C /mnt", THRD_FilePath_RollBackFw);
    if (system_chk(psc_Cmd, "tar command NG") != 0) {
        umount2("/mnt", MNT_FORCE);
        return False;
    }

//    // 完全性チェック実行.
//    snprintf(psc_Cmd, sizeof(psc_Cmd), "exec grityinte 2 /mnt/");
//    if (system_chk(psc_Cmd, "grityinte NG") != 0) {
//        umount2("/mnt", MNT_FORCE);
//	    return False;
//    }

    // アンマウント.
    si_Ret = umount2("/mnt", MNT_FORCE);
    if (si_Ret == -1) {
        strerror_r(errno, sc_Buf, sizeof(sc_Buf));
        THRL_SetPanelHardLog(20, errno, (uintptr_t)sc_Buf);
        return False;
    }

    return True;
}

// ================================================================================================
//! <!--機能--> カードバージョン取得処理.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param char *psc_Version    カードバージョン(OUT).
//! <!--戻値-->
//! \return Bool    True:成功, False:失敗
//! <!--詳細-->
//! ロールバックするバックアップファイルのカードバージョンを取得する.
//! <!--履歴-->
// 2013/12/03 yamada    UV-01385 ファームウェア更新失敗時のロールバック機能対応.
// ================================================================================================
Bool CP_HDDTrans::getRollbackVersion(char *psc_Version)
{
    FILE *fp = NULL;
    int32_t si_Ret = 0;

    APIC_SystemInfo* pc_SystemInfo = APIC_SystemInfo::newInstance();
    if (pc_SystemInfo == NULL) {
        THRL_SetPanelHardLog(21);
        return False;
    }

    // パーティション構成がUV-01385の適用が可能かをチェックする.
    if (pc_SystemInfo->isHeliosPFPart() == False) {
        THRL_SetPanelHardLog(22);
	    return False;
    }

    // 引数NULLチェック.
    if (psc_Version == NULL) {
        THRL_SetPanelHardLog(23);
        return False;
    }

    // ファイル存在チェック(ファイルオープンでチェックする).
    fp = fopen(THRD_FilePath_RollBackFw, "rb");
    if (fp == NULL) {
        THRL_SetPanelHardLog(24, errno, (uintptr_t)THRD_FilePath_RollBackFw);
        return False;
    }

    // カードバージョン情報の先頭までファイル位置指示子を移動.
    si_Ret = fseek(fp, THRD_CardVersion_Offset, SEEK_END);
    if (si_Ret != 0) {
        THRL_SetPanelHardLog(25, errno);
        fclose(fp);
        return False;
    }

    // カードバージョン読み出し.
    fread(psc_Version, THRD_CardVersion_Size, 1, fp);
    if (ferror(fp) != 0) {
        THRL_SetPanelHardLog(26, errno, (uintptr_t)THRD_FilePath_RollBackFw);
        fclose(fp);
        return False;
    }

    // ファイルストリームクローズ.
    fclose(fp);

    // 念の為、NULL文字を設定.
    psc_Version[THRD_CardVersion_Size - 1] = '\0';

    return True;
}

// ================================================================================================
//! <!--機能--> IISW更新準備処理(FW更新なし、MFP内Subset利用).
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! なし.
//! <!--戻値-->
//! \return Bool    True:成功, False:失敗
//! <!--詳細-->
//! Subset関連ファイルを更新領域にコピーする.
//! <!--履歴-->
// 2018/08/23 SCD2-Sky Kawanishi [EagleH][SecV4][RQ-676] OSS単独更新機能.
// 2018/09/29 SCD2-Sky Kawanishi [EagleH][SecV4][RQ-676] OSS単独更新-INDEX作成追加、RFSコピー追加.
// 2019/02/21 SCD2-Sky Kawanishi [EagleH][SecV4][RQ-676] 機種引継ぎ指摘事項対応.
//  THRL_SetPanelHardLog() に文字列を設定するとログ取得までポインタ保持するため、不正アクセスとなる.
//  このため、THRL_SetPanelHardLog()には文字列(ポインタ)を設定しないよう修正.
// ================================================================================================
Bool CP_HDDTrans::prepareIISWUpdateActForSubset(void)
{
	char sc_Cmd[256 + THRD_MaxLength_TarFilePath] = {0};
	int32_t si_Ret = 0;
	
	// スナップショットの再取得が必要なことをブートローダーに通知.
	setWarpInfo_ChgMFPData(True);
	
	// コピー先領域FATフォーマット.
	snprintf(sc_Cmd, sizeof(sc_Cmd), DEF_FORMAT_VFAT, DEF_SNAPSHOT_PART);
	if (0 != system_chk(sc_Cmd, "format NG"))
	{
		return False;
	}
	
	// コピー先領域をマウント.
	si_Ret = mount(DEF_SNAPSHOT_PART, THRD_TEMP_MOUNT_POINT_FOR_DEST, "vfat", MS_NOATIME, NULL);
	if (-1 == si_Ret)
	{
		THRL_SetPanelHardLog(34, (uintptr_t)errno);
		return False;
	}
	
	// Subset関連ファイルのコピー.
	// コピー元領域のマウント.
	si_Ret = mount(THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 2), THRD_TEMP_MOUNT_POINT_FOR_SOURCE, "vfat", MS_NOATIME, NULL);
	if (-1 == si_Ret)
	{
		umount2(THRD_TEMP_MOUNT_POINT_FOR_DEST, MNT_FORCE);
		THRL_SetPanelHardLog(34, (uintptr_t)errno);
		return False;
	}
	// Subset関連ファイルのコピー.
	if (0 != system_chk(THRD_CMD_COPY_SUBSET_FILES, "subset copy NG"))
	{
		umount2(THRD_TEMP_MOUNT_POINT_FOR_DEST, MNT_FORCE);
		umount2(THRD_TEMP_MOUNT_POINT_FOR_SOURCE, MNT_FORCE);
		return False;
	}
	
	// 念のため、FW更新と同じ位置にSUB.imgをコピーしておく.
	// コピー先にディレクトリ作成.
	if (0 != system_chk(THRD_CMD_MKDIR_EXP, "mkdir NG"))
	{
		umount2(THRD_TEMP_MOUNT_POINT_FOR_DEST, MNT_FORCE);
		umount2(THRD_TEMP_MOUNT_POINT_FOR_SOURCE, MNT_FORCE);
		return False;
	}
	// SUB.imgを作成したディレクトリにコピー.
	if (0 != system_chk(THRD_CMD_COPY_SUB_IMG_TO_EXP, "SUB.img copy NG"))
	{
		umount2(THRD_TEMP_MOUNT_POINT_FOR_DEST, MNT_FORCE);
		umount2(THRD_TEMP_MOUNT_POINT_FOR_SOURCE, MNT_FORCE);
		return False;
	}
	// Subset関連ファイルコピーが終ったので、コピー元領域をアンマウント.
	si_Ret = umount2(THRD_TEMP_MOUNT_POINT_FOR_SOURCE, MNT_FORCE);
	if (-1 == si_Ret)
	{
		THRL_SetPanelHardLog(35, errno);
		umount2(THRD_TEMP_MOUNT_POINT_FOR_DEST, MNT_FORCE);
		return False;
	}
	
	// RFS関連ファイルのコピー.
	// コピー元領域のマウント.
	si_Ret = mount(THRM_MOUNT_DEV(THRD_DEVFILE_SSD, 7), THRD_TEMP_MOUNT_POINT_FOR_SOURCE, "vfat", MS_NOATIME, NULL);
	if (-1 == si_Ret)
	{
		umount2(THRD_TEMP_MOUNT_POINT_FOR_DEST, MNT_FORCE);
		THRL_SetPanelHardLog(34, (uintptr_t)errno);
		return False;
	}
	// RFS関連ファイルのコピー.
	if (0 != system_chk(THRD_CMD_COPY_RFS_FILES, "rfs copy NG"))
	{
		umount2(THRD_TEMP_MOUNT_POINT_FOR_DEST, MNT_FORCE);
		umount2(THRD_TEMP_MOUNT_POINT_FOR_SOURCE, MNT_FORCE);
		return False;
	}
	// RFS関連ファイルコピーが終ったので、コピー元領域をアンマウント.
	si_Ret = umount2(THRD_TEMP_MOUNT_POINT_FOR_SOURCE, MNT_FORCE);
	if (-1 == si_Ret)
	{
		THRL_SetPanelHardLog(35, (uintptr_t)errno);
		umount2(THRD_TEMP_MOUNT_POINT_FOR_DEST, MNT_FORCE);
		return False;
	}
	
	// INDEXファイルを作成.
	if (False == makeIndexActForSubset(THRD_SOURCE_INDEX_PATH, THRD_DEST_INDEX_PATH))
	{
		umount2(THRD_TEMP_MOUNT_POINT_FOR_DEST, MNT_FORCE);
		return False;
	}

	// コピー先領域をアンマウント.
	si_Ret = umount2(THRD_TEMP_MOUNT_POINT_FOR_DEST, MNT_FORCE);
	if (-1 == si_Ret)
	{
		THRL_SetPanelHardLog(35, (uintptr_t)errno);
		return False;
	}
	
	return True;
}

// ================================================================================================
//! <!--機能--> IISW更新起動(MFP内Subset利用)のために使用するINDEXファイルを生成する.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param const char* psc_srcIndexPathName: 作成する元となるINDEXファイルのパス.
//! \param const char* psc_distIndexPathName: 作成する先のINDEXファイルのパス.
//! <!--戻値-->
//! \return Bool    True:成功, False:失敗
//! <!--詳細-->
//! IISW更新起動(MFP内Subset利用)のために使用するINDEXファイルを生成する.
//! MFPに保存されているINDEXは「通常ROM用」のため、IISW更新起動時、BootROMにてチェックされた結果、不適切と判定されてしまう.
//! 現状、BootROMの判断を変える修正は出来ないため、BootROM側判断に合わせてINDEXを書き換えて対応する.
//! 尚、BootROMはIISW更新起動かつ通常ROMのケースを想定していないが、ソフト上の制約はない(筈)なので、本来BootROMの判断を修正すべきである.
//! ただし、IISW更新起動に使うROMは、「高速ROM」のみが運用されるため、現行の判断でも既存機能に問題は生じない.
//! <!--履歴-->
// 2018/09/28 SCD2-Sky Kawanishi [EagleH][SecV4][RQ-676] OSS単独更新機能.
// ================================================================================================
Bool CP_HDDTrans::makeIndexActForSubset(const char* psc_srcIndexPathName, const char* psc_distIndexPathName)
{
	FILE* p_srcFile = NULL;
	FILE* p_distFile = NULL;
	
	if (NULL == psc_srcIndexPathName || NULL == psc_distIndexPathName)
	{
		THRL_SetPanelHardLog(42);
		return False;
	}
	// 作成元INDEX(read-only)のオープン.
	p_srcFile = fopen(psc_srcIndexPathName, "r");
	if (NULL == p_srcFile)
	{
		THRL_SetPanelHardLog(43, 1);
		return False;
	}
	// 作成先INDEX(create)のオープン.
	p_distFile = fopen(psc_distIndexPathName, "w");
	if (NULL == p_distFile)
	{
		THRL_SetPanelHardLog(43, 2);
		fclose(p_srcFile);
		return False;
	}
	
	char sc_Buff[THRD_INDEX_ONE_LINE_LENG];
	
	// 作成元INDEXを読み込み.
	while (NULL != fgets(sc_Buff, THRD_INDEX_ONE_LINE_LENG, p_srcFile))
	{
		// 読み込み行を作成先へ1行コピー.
		if (EOF == fputs(sc_Buff, p_distFile))
		{
			THRL_SetPanelHardLog(44, 1);
			fclose(p_srcFile);
			fclose(p_distFile);
			return False;
		}
		// 読み込み行がVERタグの場合.
		if (0 == strncmp(sc_Buff, THRD_INDEX_TYPE_VER, 4))
		{
			// 見つけたら次の行に追加.
			if (EOF == fputs(THRD_INDEX_DUMMYLINE_EXP, p_distFile))
			{
				THRL_SetPanelHardLog(44, 2);
				fclose(p_srcFile);
				fclose(p_distFile);
				return False;
			}
		}
		// 読み込み行がDBGタグの場合.
		if (0 == strncmp(sc_Buff, THRD_INDEX_TYPE_DBG, 4))
		{
			// 見つけたら次の行に追加.
			if (EOF == fputs(THRD_INDEX_DUMMYLINE_FOR_SUBSET, p_distFile))
			{
				THRL_SetPanelHardLog(44, 3);
				fclose(p_srcFile);
				fclose(p_distFile);
				return False;
			}
		}
	}
	fclose(p_srcFile);
	fclose(p_distFile);
	return True;
}

// ================================================================================================
//! <!--機能--> OSS更新準備処理(署名なし).
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param TYPE_OSSType uc_OSSType OSS種別.
//! \param const char* psc_OSSFilePathName 更新対象ファイル格納先.
//! <!--戻値-->
//! \return TYPE_PrepareIISWUpdateResult
//! <!--詳細-->
//! OSS更新準備のため、更新領域に指定されたtarを解凍する.
//! <!--履歴-->
// 2018/08/23 SCD2-Sky Kawanishi [EagleH][SecV4][RQ-676] OSS単独更新機能.
// 2018/12/04 SCD2-Sky Kawanishi [EagleH][SecV4][RQ-676] 追加対応-準備要求エラー通知対応.
// ================================================================================================
TYPE_PrepareIISWUpdateResult CP_HDDTrans::prepareIISWUpdateAddOSS(TYPE_OSSType uc_OSSType, const char* psc_OSSFilePathName)
{
	char sc_Cmd[256 + THRD_MaxLength_TarFilePath] = {0};
	int32_t si_Ret = 0;
	
	// 引数チェック.
	if (NULL == psc_OSSFilePathName)
	{
		THRL_SetPanelHardLog(36);
		return TYPE_PrepareIISWUpdateResult_Failured;
	}
	// 文字列長チェック.
	if (THRD_MaxLength_TarFilePath <= strlen(psc_OSSFilePathName))
	{
		THRL_SetPanelHardLog(37, (uintptr_t)strlen(psc_OSSFilePathName));
		return TYPE_PrepareIISWUpdateResult_Failured;
	}
	
	// ファイル存在チェック.
	if (0 != access(psc_OSSFilePathName, F_OK))
	{
		THRL_SetPanelHardLog(38, uc_OSSType);
		return TYPE_PrepareIISWUpdateResult_Failured;
	}
	
	// コピー先領域のマウント.
	si_Ret = mount(DEF_SNAPSHOT_PART, THRD_TEMP_MOUNT_POINT_FOR_DEST, "vfat", MS_NOATIME, NULL);
	if (-1 == si_Ret)
	{
		THRL_SetPanelHardLog(39, (uintptr_t)errno);
		return TYPE_PrepareIISWUpdateResult_Failured;
	}
	
	// TARファイル解凍.
	snprintf(sc_Cmd, sizeof(sc_Cmd), THRD_CMD_TAR_XFM_TO_DEST, psc_OSSFilePathName);
	if (0 != system_chk(sc_Cmd, "tar command NG"))
	{
		umount2(THRD_TEMP_MOUNT_POINT_FOR_DEST, MNT_FORCE);
		return TYPE_PrepareIISWUpdateResult_Failured;
	}
	
	// コピー先領域のアンマウント.
	si_Ret = umount2(THRD_TEMP_MOUNT_POINT_FOR_DEST, MNT_FORCE);
	if (-1 == si_Ret)
	{
		THRL_SetPanelHardLog(40, (uintptr_t)errno);
		return TYPE_PrepareIISWUpdateResult_Failured;
	}
	
	return TYPE_PrepareIISWUpdateResult_Success;
}

// ================================================================================================
//! <!--機能--> OSS更新準備処理(署名あり).
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param TYPE_OSSType uc_OSSType OSS種別.
//! \param const char* psc_OSSFilePathName 更新対象ファイル格納先.
//! \param const char* psc_OSSSigFilePath 署名ファイル格納先.
//! <!--戻値-->
//! \return TYPE_PrepareIISWUpdateResult
//! <!--詳細-->
//! OSS更新準備のため、更新領域に指定されたtarを解凍する(署名検証のみ実施してprepareIISWUpdateAddOSS(uc_OSSType, psc_OSSFilePath)へ委譲).
//! <!--履歴-->
// 2018/08/23 SCD2-Sky Kawanishi [EagleH][SecV4][RQ-676] OSS単独更新機能.
// 2018/12/04 SCD2-Sky Kawanishi [EagleH][SecV4][RQ-676] 追加対応-準備要求エラー通知対応.
// ================================================================================================
TYPE_PrepareIISWUpdateResult CP_HDDTrans::prepareIISWUpdateAddOSS(TYPE_OSSType uc_OSSType, const char* psc_OSSFilePath, const char* psc_OSSSigFilePath)
{
	APIC_SystemSetting* pc_SecuritySystemSetting = APIC_SystemSetting::newInstance();

	// NULLチェック.
	if (NULL == pc_SecuritySystemSetting)
	{
        THRL_SetPanelHardLog(45);
	    return TYPE_PrepareIISWUpdateResult_Failured;
	}

	// FW検証(更新時)が有効のとき.
	if (pc_SecuritySystemSetting->getSWUpdateVerifyMode())
	{
		// 署名ファイル有無チェック.
		if (NULL == psc_OSSSigFilePath || 0 != access(psc_OSSSigFilePath, F_OK))
		{
	        THRL_SetPanelHardLog(46);
	        return TYPE_PrepareIISWUpdateResult_SigNotFound;
		}
		// 署名検証実施.
	    if (!(verifySignature(psc_OSSFilePath, psc_OSSSigFilePath)))
	    {
	        THRL_SetPanelHardLog(47);
	        return TYPE_PrepareIISWUpdateResult_SigVerifyNG;
	    }
	}

	return prepareIISWUpdateAddOSS(uc_OSSType, psc_OSSFilePath);
}

// 2012/01/20 ft_kondo CQ.360494 umountが発生していた場合、C-FA14を発生させてリブートさせる. 
// umountエラーが発生しているかどうかを判定する. 
Bool CP_HDDTrans::isUmountError(void)
{
	return b_isUmountError;
}

// umount error発生時のFA14詳細コードを取得する.
ushort CP_HDDTrans::getFA14DetailCode(void)
{
	return us_FA14DetailCode;
}

// ================================================================================================
//! <!--機能--> FW検証用証明書インポート.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param const char* psc_CertDir		インポート元証明書ファイルパス.
//! \param const char* psc_CertFile		インポート元証明書ファイル名.
//! <!--戻値-->
//! \return Bool    True:成功, False:失敗.
//! <!--詳細-->
//!  USBフラッシュメモリに保存されたFW検証用証明書をインポートする.
//! <!--履歴-->
// 2017/11/24 Shibahara [SecV2-2] ファームウェア更新時の真正性検証技術.
// ================================================================================================
Bool CP_HDDTrans::importCertForFwVerification( const char* psc_CertDir, const char* psc_CertFile )
{
	char	sc_Cmd[256] = {0};
	if ( !psc_CertDir || !psc_CertFile ){
		printf("Invalid cert path.\n");
		return False;
	}

	if (sizeof(sc_Cmd) <= (unsigned int)snprintf(sc_Cmd, sizeof(sc_Cmd), "exec cp -f %s/%s /boot/%s", psc_CertDir, psc_CertFile, psc_CertFile)) {
		printf("Too long cert path.\n");
		return False;
	}

	if( 0 != system_chk(sc_Cmd, "Copying a certificate failed") ) {
		return False;
	}
	return True;
}

#if defined( DEF_SupportSSD )

/* ================================================================================================
//! <!--機能--> SSDデータ消失防止用 Readタスク エントリ関数.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param なし
//! <!--戻値-->
//! \return なし
//! <!--詳細-->
//!  SSD Readタスク エントリ関数
//! <!--履歴-->
//   2012/09/26 Gonsho	 新規作成.
//   2013/03/27 Miyamoto CQ378754 SSDデータ消失防止でFA14対応.
//   2017/02/20 Sky      DLONG対応.
// ================================================================================================*/
void taskSSDKeepData()	// DEF_SupportSSD.
{
	BE_TaskMessage e_Message = 0;
	uintptr_t ul_Option = 0;
	
	// SSDサイズからSSDセクタ数を取得.
	DLONG dl_SsdSectorNum = theHDDTrans.getSsdFixedSectorNum();
	
	// 2017/02/20 Sky DLONG対応.
	// DLONGが64bit環境の場合も下位32bit分を使用する.
	uint32_t dl_SsdSectorNumL = ( uint32_t ) ( dl_SsdSectorNum & 0xFFFFFFFF );
	
	uint32_t ul_SsdReadSectorNum1h = ( dl_SsdSectorNumL / DEF_SSD_CHECK_CYCLE ) + (( dl_SsdSectorNumL % DEF_SSD_CHECK_CYCLE ) ? 1:0);
	uint32_t ul_SsdReadSectorNum5h = ul_SsdReadSectorNum1h * 5;
	
	// メッセージ受信.
	while ( True ) {
		e_Message = 0;
		if ( receiveMessage( TID_SSDKeepData, e_Message, ul_Option ) == False ) {
			printf("taskSSDKeepData() receiveMessage Error.\n");
		}
		
		switch ( e_Message ) {
			case TM_ReqReadSSDIntarval:		// 定期Read.
				// 定期Read実施.
				theHDDTrans.keepSsdDataScheduledRead( dl_SsdSectorNumL, ul_SsdReadSectorNum5h, ul_SsdReadSectorNum1h );
				break;
			case TM_ReqReadSSDAll:			// 一括Read.
				// 一括Read実施.
				theHDDTrans.keepSsdDataAllRead( dl_SsdSectorNumL, dl_SsdSectorNumL, ul_SsdReadSectorNum5h );
				break;
			default:
				THRL_ThAssert( False, "taskSSDKeepData() : Unknown Msg! %d, para 0x%x", e_Message, ul_Option , 0, 0, CFA14_DetailCode0x0AA1 );
				break;
		}
	}
}

/* ================================================================================================
//! <!--機能--> SSDセクタ数取得.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param なし
//! <!--戻値-->
//! \return DLONG SSDセクタ数.
//! <!--詳細-->
//!  SSDデータ消失防止用、SSDセクタ数取得.
//! <!--履歴-->
//   2013/03/27 Miyamoto 新規作成.
//   2013/03/27 Miyamoto CQ378754 SSDデータ消失防止でFA14対応.
// ================================================================================================*/
DLONG CP_HDDTrans::getSsdFixedSectorNum( void )	// DEF_SupportSSD.
{
	return (DLONG)getFixedSectorNum( THRD_SSD_DEVICE );
}

/* ================================================================================================
//! <!--機能--> HDDセクタ数取得.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param なし
//! <!--戻値-->
//! \return DLONG HDDセクタ数.
//! <!--詳細-->
//!  容量表示用、HDDセクタ数取得.
//! <!--履歴-->
//   2014/07/09 yamada	制御構成変更No39 eMMC容量取得対応.
//   2017/10/10 Yoneda  S800 ストレージ構成変更対応.
// ================================================================================================*/
DLONG CP_HDDTrans::getHddFixedSectorNum( void )
{
	return (DLONG)getFixedSectorNum( theHDDTrans.pc_strDeviceSSD );
}

/* ================================================================================================
//! <!--機能--> SSD 定期Read実行.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param ulong ul_SsdSectorNum		SSD 全セクタ数.
//! \param ulong ul_MaxReadSector		Readセクタ上限数.
//! \param ulong ul_ReadSectorPerOur	1時間あたりのReadセクタ数.
//! <!--戻値-->
//! \return なし
//! <!--詳細-->
//!  SSDデータ消失防止用、SSD 定期Read実行
//! <!--履歴-->
//   2012/09/26 Gonsho		新規作成.
//   2012/12/21 Horishita	CQ376471 SSDを1GByteから512MByteに変更後、SSDデータ消失防止機能の挙動が不安定(FA14)になることがある問題の対応.
//   2018/07/24 Sky薗﨑 	[Eagle]AR1052929_TASK_SSDKeepDatでOopsの不具合対応.
// ================================================================================================*/
void CP_HDDTrans::keepSsdDataScheduledRead( uint32_t ul_SsdSectorNum, uint32_t ul_MaxReadSector, uint32_t ul_ReadSectorPerOur )	// DEF_SupportSSD.
{
	// DeepSleep投入抑止.
	APIC_Power* pc_ApiPower = APIC_Power::newInstance();
// 2018/07/24 Sky薗﨑  [Eagle]AR1052929_TASK_SSDKeepDatでOopsの不具合対応.
//	pc_ApiPower->setDeepSleepDelay( __FILE__, __LINE__ );
	
	APIC_SystemInfo* pc_SystemInfo = APIC_SystemInfo::newInstance();
	
	// 現在時刻取得.
	TYPC_Time c_Now = getSystimeRTC();
	
	// 前回Read情報取得.
	TYPC_Time c_Last;

	pc_SystemInfo->getSSDReadCompletionTime( &c_Last ); 								// 前回Read完了時刻.
	uint32_t ul_ScheduledSector = pc_SystemInfo->getScheduledSSDReadCompletionSectorNum();	// 定期分割Read完了セクタ.
	uint32_t ul_LastReadSector  = pc_SystemInfo->getSSDReadCompletionSetctorNum();			// 前回Read完了セクタ.

	
	// 前回Read情報-Read完了時間 初期値比較用.
	TYPC_Time c_Init(1990,0,0,0,0,0);
	
	// 前回Read時間からの経過時間を取得.
	uint64_t ul_TransitTime = 0;	//201701/23 uint32_tからuint64_tに変更
	
	// TYPC_Timeをtime_t（long）へ変換. TYPC_Timeは 年月日時分秒、time_tは秒.
	time_t s_NowTm, s_LastTm;
	s_NowTm  = c_Now.getTime();
	
	// 前回Readセクタが範囲外の場合はRead情報をリセット.
	if( ( ul_ScheduledSector > ul_SsdSectorNum ) || ( ul_LastReadSector > ul_SsdSectorNum ) ) {
		ul_ScheduledSector = 0;											// 定期分割Read完了セクタをリセット.
		ul_LastReadSector = 0;											// 前回Read完了セクタをリセット.
		c_Last = c_Init;												// 前回Read完了時刻をリセット.
	}

	if ( c_Last == c_Init ) {	// NVRAM前回Read完了時刻が初期値（工場出荷値）か確認.
		s_LastTm = s_NowTm - 3600;		// 初期値だったら現在時刻から1時間前にしておく.
	} else {
		s_LastTm = c_Last.getTime();
	}
	
	// 経過時間を時単位で算出.
	ul_TransitTime = (s_NowTm - s_LastTm)/3600 + ((s_NowTm - s_LastTm)%3600 ? 1:0);
	
	// 残Readセクタ数を計算.
	uint32_t ul_ReadSectorNum = getReadSectorNum_KeepSsdData( ul_SsdSectorNum, ul_ScheduledSector, ul_LastReadSector );
	
	// 読み込み予測セクタ数として、Read残セクタ数に経過時間分のセクタ数を足す.
	ul_ReadSectorNum += ul_ReadSectorPerOur * ul_TransitTime;
	if ( ul_ReadSectorNum > ul_SsdSectorNum ) {
		// SSDセクタ数を超えたら補正.
		ul_ReadSectorNum = ul_SsdSectorNum;
	}
	
	// 今回定期ReadでのRead予測セクタ位置を計算.
	uint32_t ul_NextScheduledSector = ( ul_LastReadSector + ul_ReadSectorNum ) % ul_SsdSectorNum;
	
	/*
	 *	下記状況を考慮し、Read予測セクタ位置を補正する.
	 *		1. Read予測セクタ位置を計算した結果、Read実績セクタ位置の周回遅れが発生.
	 *		2. Read予測セクタ位置をNVRAMに保存する.
	 *		3. Read実施中に電源OFFされる.
	 *		4. 次回Read実施時に、Read予測セクタ位置とRead実績セクタ位置が同じ位置になっている.
	 *		⇒本来は全セクタReadしなければならないのに、4の状況では残Readセクタ数が0の時と区別できない.
	 *
	 *	この状況を防ぐため、周回遅れが発生した場合は、Read予測セクタ位置を-1しておく.
	 *	→ 3で電源OFFされても、次のReadで残Readセクタ数が全セクタになるよう計算されるはず.
	 */
	Bool b_IsLapBehind = False;
	if ( ul_NextScheduledSector == ul_LastReadSector ) {
		if ( ul_LastReadSector == 0 ) {
			ul_NextScheduledSector = ul_SsdSectorNum - 1;	// -1 補正しておく.
		} else {
			ul_NextScheduledSector--;						// -1 補正しておく.
		}
		b_IsLapBehind = True;
	}
	
	// 定期Read完了セクタを更新.
	pc_SystemInfo->setScheduledSSDReadCompletionSectorNum( ul_NextScheduledSector );
	
	// Read完了時刻を先に設定する.
	pc_SystemInfo->setSSDReadCompletionTime( &c_Now );
	
#if (defined TORNADO) && (!defined FUM_LINUX) // 実機のみ有効.

	// SSD Read実施.
	readSsd_KeepData( ul_SsdSectorNum, ul_LastReadSector, ul_ReadSectorNum, ul_MaxReadSector, ul_ReadSectorPerOur );
	
#else // 新SIM環境.

	// Sim起動中におかしな動作にならないように、.
	// Read実績セクタをRead予測セクタの値で登録しておく.
	pc_SystemInfo->setSSDReadCompletionSetctorNum( ul_NextScheduledSector );
	
#endif /* #if (defined TORNADO) && (!defined FUM_LINUX) */

	if ( b_IsLapBehind == True ) {
		// 定期Read完了セクタの補正分を更新.
		if ( ( ul_NextScheduledSector +1 ) == ul_SsdSectorNum ) {
			ul_NextScheduledSector = 0;
		} else {
			ul_NextScheduledSector++;
		}
		pc_SystemInfo->setScheduledSSDReadCompletionSectorNum( ul_NextScheduledSector+1 );
	}

	// DeepSleep投入抑止解除.
	pc_ApiPower->resetDeepSleepDelay( __FILE__, __LINE__ );
}

/* ================================================================================================
//! <!--機能--> SSD 一括Read実行.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param ulong ul_SsdSectorNum	SSD 全セクタ数.
//! \param ulong ul_MaxReadSector	Readセクタ上限数.
//! \param ulong ul_DelaySector		DelayするまでのReadセクタ数.
//! <!--戻値-->
//! \return なし
//! <!--詳細-->
//!  SSDデータ消失防止用、SSD 一括Read実行
//! <!--履歴-->
//   2012/09/26 Gonsho		新規作成.
//   2012/12/21 Horishita	CQ376471 SSDを1GByteから512MByteに変更後、SSDデータ消失防止機能の挙動が不安定(FA14)になることがある問題の対応.
//   2018/07/24 Sky薗﨑 	[Eagle]AR1052929_TASK_SSDKeepDatでOopsの不具合対応.
// ================================================================================================*/
void CP_HDDTrans::keepSsdDataAllRead( uint32_t ul_SsdSectorNum, uint32_t ul_MaxReadSector, uint32_t ul_DelaySector )	// DEF_SupportSSD.
{
	// DeepSleep投入抑止.
	APIC_Power* pc_ApiPower = APIC_Power::newInstance();
// 2018/07/24 Sky薗﨑  [Eagle]AR1052929_TASK_SSDKeepDatでOopsの不具合対応.
//	pc_ApiPower->setDeepSleepDelay( __FILE__, __LINE__ );
	
	// 現在時刻取得.
	TYPC_Time c_Now = getSystimeRTC();
	
	// 前回Read情報取得.
	APIC_SystemInfo* pc_SystemInfo = APIC_SystemInfo::newInstance();
	TYPC_Time c_Last;
	pc_SystemInfo->getSSDReadCompletionTime( &c_Last ); 								// 前回Read完了時刻.
	uint32_t ul_ScheduledSector = pc_SystemInfo->getScheduledSSDReadCompletionSectorNum();	// 定期分割Read完了セクタ.
	uint32_t ul_LastReadSector  = pc_SystemInfo->getSSDReadCompletionSetctorNum();			// 前回Read完了セクタ.
	
	// 前回Read情報-Read完了時間 初期値比較用.
	TYPC_Time c_Init(1990,0,0,0,0,0);
	
	uint32_t ul_ReadSectorNum;
	
	// 前回Readセクタが範囲外の場合はRead情報をリセット.
	if( ( ul_ScheduledSector > ul_SsdSectorNum ) || ( ul_LastReadSector > ul_SsdSectorNum ) ) {
		ul_ScheduledSector = 0;											// 定期分割Read完了セクタをリセット.
		ul_LastReadSector = 0;											// 前回Read完了セクタをリセット.
		c_Last = c_Init;												// 前回Read完了時刻をリセット.
	}

	// 初期値（工場出荷値）と比較.
	if ( c_Last == c_Init ) {
		// 初期値の場合は全Read.
		ul_ReadSectorNum = ul_SsdSectorNum;
	} else {
		ul_ReadSectorNum = getReadSectorNum_KeepSsdData( ul_SsdSectorNum, ul_ScheduledSector, ul_LastReadSector );
	}
	
	// Read完了時刻を先に設定する.
	pc_SystemInfo->setSSDReadCompletionTime( &c_Now );
	
#if (defined TORNADO) && (!defined FUM_LINUX) // 実機のみ有効.

	// SSD Read実施.
	readSsd_KeepData( ul_SsdSectorNum, ul_LastReadSector, ul_ReadSectorNum, ul_MaxReadSector, ul_DelaySector );
	
#else // 新SIM環境.

	// Sim起動中におかしな動作にならないように、.
	// Read実績セクタをRead予測セクタの値で登録しておく.
	pc_SystemInfo->setSSDReadCompletionSetctorNum( ul_ScheduledSector );
	
#endif /* #if (defined TORNADO) && (!defined FUM_LINUX) */
	// 一括Read処理中フラグOFF.
	b_SsdKeepDataReadingAllFlg = False;
	
	// DeepSleep投入抑止解除.
	pc_ApiPower->resetDeepSleepDelay( __FILE__, __LINE__ );
}

/* ================================================================================================
//! <!--機能--> SSD一括Read処理中フラグ.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \return なし
//! <!--戻値-->
//! \return なし.
//! <!--詳細-->
//!  .
//! <!--履歴-->
//   2012/10/03 Gonsho	新規作成.
// ================================================================================================*/
void CP_HDDTrans::setReqStateSsdAllRead( void )	// DEF_SupportSSD.
{
	b_SsdKeepDataReadingAllFlg = True;
}

/* ================================================================================================
//! <!--機能--> SSD一括Read要求可否判断.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \return なし
//! <!--戻値-->
//! \return Bool True：要求可能、False：要求不可.
//! <!--詳細-->
//!  .
//! <!--履歴-->
//   2012/10/03 Gonsho	新規作成.
// ================================================================================================*/
Bool CP_HDDTrans::enableReqSsdAllRead( void )	// DEF_SupportSSD.
{
	// フラグが立っていれば処理中なので、次の要求は出来ない.
	if ( b_SsdKeepDataReadingAllFlg == True ) {
		return False;
	}
	return True;
}

/* ================================================================================================
//! <!--機能--> システム時間（RTC）取得.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \return なし
//! <!--戻値-->
//! \return TYPC_Time RTC時刻.
//! <!--詳細-->
//!  .
//! <!--履歴-->
//   2012/10/03 Gonsho	新規作成.
//   2012/10/25 Gonsho	RTC時刻取得方法を修正（RTCにアクセスしない）.
// ================================================================================================*/
TYPC_Time CP_HDDTrans::getSystimeRTC()	// DEF_SupportSSD.
{
	// RTC時間を取得.
	time_t time = SYSC_Timer::getRtcTime();
	
	// TYPC_Time型に変換して返す.
	TYPC_Time c_Time;
	c_Time.setTime(time);
	
	return c_Time;
}

/* ================================================================================================
//! <!--機能--> SSD 残Readセクタ数取得.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param ulong ul_SsdSectorNum		SSD 全セクタ数.
//! \param ulong ul_ScheduledSector		Read予測セクタ位置.
//! \param ulong ul_LastReadSector		Read実績セクタ位置.
//! <!--戻値-->
//! \return なし
//! <!--詳細-->
//!  SSDデータ消失防止用 残Readセクタ数を取得する.
//! <!--履歴-->
//   2012/09/26 Gonsho	新規作成.
// ================================================================================================*/
uint32_t CP_HDDTrans::getReadSectorNum_KeepSsdData( uint32_t ul_SsdSectorNum, uint32_t ul_ScheduledSector, uint32_t  ul_LastReadSector )	// DEF_SupportSSD.
{
	uint32_t ul_ReadSectorNum;
	
	// 読み込みセクタ数を計算.
	if ( ul_ScheduledSector == ul_LastReadSector ) {		// 読み込み蓄積セクタ無.
		ul_ReadSectorNum = 0;
	} else if ( ul_ScheduledSector > ul_LastReadSector ) {	// 読み込み蓄積セクタ有.
		ul_ReadSectorNum = ul_ScheduledSector - ul_LastReadSector;
	} else {												// 読み込み蓄積セクタ有（終端またぎ）.
		ul_ReadSectorNum = ul_ScheduledSector + ul_SsdSectorNum - ul_LastReadSector;
	}
	
	return ul_ReadSectorNum;
}

/* ================================================================================================
//! <!--機能--> SSD Read.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param ulong ul_SsdSectorNum	SSD 全セクタ数.
//! \param ulong ul_StartSector		Read開始セクタ位置.
//! \param ulong ul_ReadSectorNum	Readセクタ数.
//! \param ulong ul_MaxReadSector	Readセクタ上限数.
//! \param ulong ul_DelaySector		DelayするまでのReadセクタ数.
//! <!--戻値-->
//! \return なし
//! <!--詳細-->
//!  SSDデータ消失防止用 SSD Read処理.
//! <!--履歴-->
//   2012/09/26 Gonsho	  新規作成.
//   2013/02/14 Takatsuji FA14のアサート文字列のリファイン.
//   2013/03/27 Miyamoto  CQ378754 SSDデータ消失防止でFA14対応.
//   2021/06/17 Sky       詳細コード0x0AA5の発行を削除.
// ================================================================================================*/
extern uintptr_t ul_CS1_BaseAdr;				// NVRAM ベースアドレス.
#define THRD_NVRAM_BSPTHR_OFFSET		0x07FECD	// NVRAM BSPスレッド割り当て領域オフセットアドレス.
#define THRD_NVRAM_BAD_SECTOR_OFFSET	0xF7		// NVRAM SSD不良セクタ検出フラグオフセットアドレス.

void CP_HDDTrans::readSsd_KeepData( uint32_t ul_SsdSectorNum, uint32_t ul_StartSector, uint32_t ul_ReadSectorNum, uint32_t ul_MaxReadSector, uint32_t ul_DelaySector )	// DEF_SupportSSD.
{
	APIC_SystemInfo* pc_SystemInfo = APIC_SystemInfo::newInstance();
	off_t dl_Ret;
	int32_t si_errno;
	
	int32_t fd = SYSF_open( THRD_DEVFILE_SSD, O_RDONLY | O_CLOEXEC, 0666 );
	if ( fd < 0 ) {
		THRL_ThAssert( False, "CP_HDDTrans::readSsd_KeepData() SSD SYSF_open Error. errno %d", (uintptr_t)SYSF_errnoGet(), 0, 0, 0, CFA14_DetailCode0x0AA2 );
		return;	// 静的解析用.
	}
	
	// 前回読み込み完了位置までSeekする.
	if ( ul_StartSector != 0 ) {
		off_t dl_StartSector = (off_t)ul_StartSector * DEF_SECTOR_SIZE;
		dl_Ret = lseek( fd, dl_StartSector, SEEK_SET );
		if ( dl_Ret != dl_StartSector ) {
			si_errno = errno; // SYSF_close()実行でerrnoが上書きされるので、errnoを保存する.
			SYSF_close( fd );
			THRL_ThAssert( False, "CP_HDDTrans::readSsd_KeepData() SSD lseek Error. SEEK_SET to %jd. ReturnValue %"PRIdPTR" errno %"PRId32"", (uintptr_t)dl_StartSector, (uintptr_t)dl_Ret, (uintptr_t)si_errno, 0, CFA14_DetailCode0x0AA3 );
			return;	// 静的解析用.
		}
	}
	
	char ca_Buff[DEF_SECTOR_SIZE] = { 0 };
	uint32_t ul_ReadCompNum = 0;
	uint32_t ul_CurrentSector = ul_StartSector;
	off_t  sl_OneReadSize = 0;
	uchar *puc_BadSectorFlg = (uchar *)(ul_CS1_BaseAdr + THRD_NVRAM_BSPTHR_OFFSET + THRD_NVRAM_BAD_SECTOR_OFFSET);
	while ( ( ul_ReadCompNum < ul_ReadSectorNum ) && ( ul_ReadCompNum < ul_MaxReadSector ) ) {
		
		if ( ul_CurrentSector == ul_SsdSectorNum ) {
			dl_Ret = lseek( fd, 0, SEEK_SET );
			if ( dl_Ret != 0 ) {
				si_errno = errno;
				SYSF_close( fd );
				THRL_ThAssert( False, "CP_HDDTrans::readSsd_KeepData() SSD lseek Error (2). SEEK_SET to 0. ReturnValue %"PRIdPTR" errno %"PRId32"", (uintptr_t)dl_Ret, (uintptr_t)si_errno, 0, 0, CFA14_DetailCode0x0AA4 );
				return;	// 静的解析用.
			}
			ul_CurrentSector = 0;
		}
		
		sl_OneReadSize = SYSF_read( fd, ca_Buff, sizeof(ca_Buff) );
		if ( sl_OneReadSize != sizeof(ca_Buff) ) {
			si_errno = errno;
			// SSDの不良セクタ読込時にIOエラーが発生する為.
			// 初回のIOエラー検出時のみFA14詳細コードのみを設定.
			if ( si_errno == EIO ) {
				if ( *puc_BadSectorFlg == 0 ) {
					NVDF_memsetNvRam( puc_BadSectorFlg, 1, sizeof(uchar) );
				}
			} else {
				SYSF_close( fd );
				return;	// 静的解析用.
			}
		}
		
		ul_ReadCompNum++;
		ul_CurrentSector++;
		if ( ( ul_ReadCompNum % ul_DelaySector ) == 0 ) {
			// 前回Read完了セクタ更新.
			pc_SystemInfo->setSSDReadCompletionSetctorNum( ul_CurrentSector );
			
			SYSF_taskDelay( 3 );
		}
	}
	
	// 前回Read完了セクタ更新.
	if ( ul_CurrentSector == ul_SsdSectorNum ) {
		ul_CurrentSector = 0;
	}
	pc_SystemInfo->setSSDReadCompletionSetctorNum( ul_CurrentSector );
	// SSDクローズ.
	if ( SYSF_close( fd ) < 0 ) {
		THRL_ThAssert( False, "CP_HDDTrans::readSsd_KeepData() SSD SYSF_close Error. errno %d", (uintptr_t)errno, 0, 0, 0, CFA14_DetailCode0x0AA6 );
	}
}
#endif	/* #if defined( DEF_SupportSSD ) */

/* ================================================================================================
//! <!--機能--> クラスターインデックスの実体を確保.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param なし
//! <!--戻値-->
//! \return なし
//! <!--詳細-->
//!  分割バックアップ/リストア用のクラスターインデックスの実体を確保する.
//! <!--履歴-->
//!   2012/11/21 ADS川口 CQ.377856  MS-LCL_3 新規作成.
// ================================================================================================*/
void CP_HDDTrans::allocBackupClusterIdx()
{
	// クラスターインデックス確保.
	for( THRE_BackupType e_BackupType = THRE_BackupTypeImage; e_BackupType<THRE_BackupKindNum; e_BackupType++ )
	{
		if( pul_BackupClusterIdx[e_BackupType] == NULL )
		{
			CC_Fat* pcc_Image = (CC_Fat*)getBackupFatPtr( e_BackupType  );
			THRL_ThAssert( (pcc_Image != NULL), "CC_Fat::init not called. pcc_Image is NULL!!", 0, 0, 0, 0, CFA14_DetailCode0x0B56);
			pul_BackupClusterIdx[e_BackupType] = new uint32_t[pcc_Image->getMaxClusterNum()];
			THRL_ThAssert( (pul_BackupClusterIdx[e_BackupType] != NULL), "CP_HDDTrans::allocBackupClusterIdx, pul_BackupClusterIdx[%d] is NULL!!", e_BackupType, 0, 0, 0, CFA14_DetailCode0x0B59);
		}
	}
}

// ================================================================================================
//! <!--機能--> 【内部】 チェックサムを更新する
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param CC_Fat* pc_Fat 		... 蓄積管理画像のFat
//! \param CC_Fat* pc_InfoFat 	... 蓄積管理情報のFat
//! \param SEM_ID* semID_Mutex 	... セマフォ
//! <!--戻値-->
//! \return なし
//! <!--詳細-->
//! HDDバックアップ用のメッセージ構造体のコールバック関数を実行し、削除する\n 
//! 内部関数だが、見易さの都合上、publicに定義してある \n 
//! <!--履歴-->
//  16/05/23 Shinosaki 新規作成 MAv4 RQ-505 バックアップ機能強化対応.
// ================================================================================================
void CP_HDDTrans::updateFatChecksum( CC_Fat* pc_Fat, CC_Fat* pc_InfoFat, SEM_ID* semID_Mutex )
{
	//startHDDTransFatWriteを呼び出すことでCheckSumを更新する.
	startHDDTransFatWrite( pc_Fat, pc_InfoFat, semID_Mutex );
}

	
Bool THRC_internetISW_setting(){
	printf("THRC_internetISW_setting()\n");
	return theHDDTrans.internetISW_setting();
}

#ifdef DEF_SupportSSD
/*===========================================================================================================
	［機能］	SSDからのデータ読み込み（開始セクタ、サイズ指定）.
-------------------------------------------------------------------------------------------------------------
［引数］	unsigned long sec_num			開始セクタ番号.
			unsigned long size				読み込みデータサイズ(Byte).
			char*	buf						読み込み先アドレス.
［戻値］	（True : 成功 False : 失敗）.
-------------------------------------------------------------------------------------------------------------
［履歴］	12/05/17 N.Kato 新規作成 .
			12/08/20 N.Kato		CQ.366262 ファイルアクセスのClose処理がされない問題修正.
			12/08/28 Gonsho 静的解析対応 fdのcloseを随所に追加.
            13/02/14 Takatsuji	FA14のアサート文字列のリファイン.
            13/10/21 ADS川口 CQ.383235(Act_68509) SYSF_lseekの引数はlongだが、RWチェック領域が.
            　　　　　longで表せるオフセット以上となったためSYSF_lseek→lseekに変更.
            19/10/29 Sky        TAS000000001236対応.
===========================================================================================================*/
Bool THRF_HWCheck_ReadFromSSD( uint32_t sec_num, uint32_t size, char* buf )	// DEF_SupportSSD.
{
    int32_t fd = -1;
	int32_t ret = 0;
	off_t l_read_ret = 0;
	Bool bl_Ret = False;								// 戻り値（True：OK  False：NG) .
	off_t offset = 0;


	if( sec_num == 0 ){
        return bl_Ret;
	}
	if( size == 0 ){
        return bl_Ret;
	}
	if( buf == NULL ){
        return bl_Ret;
	}

	char *device_path = "/dev/kmsda";
	uint32_t sec_cnt = (size+DEF_SECTOR_SIZE-1)/DEF_SECTOR_SIZE;

 	device_path = THRL_GetPartitionPathFromOffset(device_path, sec_num, sec_cnt);

	if( NULL!=device_path )
	{
		fd = SYSF_open( device_path, O_RDWR, 0666 );  // SSD
	}
	if( fd == -1 ){
		printf("THRF_HWCheck_ReadFromSSD open NG. errno %d\n", SYSF_errnoGet());
        return bl_Ret;
    }

	offset = sec_num * DEF_SECTOR_SIZE;								// オフセット位置.

    off_t lseek_ret = lseek( fd, offset, SEEK_SET );
    if( lseek_ret == -1 ){
		printf("THRF_HWCheck_ReadFromSSD seek Err. SEEK_SET to %"PRIdPTR" errno %d\n", offset, errno);
		SYSF_close( fd );
		return bl_Ret;
    }

	l_read_ret = SYSF_read( fd, buf, size );
    if( l_read_ret == -1 ){
		printf("THRF_HWCheck_ReadFromSSD  read Err. sector %"PRId32" size %"PRId32" errno %d\n", sec_num, size, SYSF_errnoGet());
		SYSF_close( fd );
		return bl_Ret;
    }
	ret = SYSF_close( fd );
	if ( ret == -1 ) {
		printf("THRF_HWCheck_ReadFromSSD  close Err. errno %d\n", SYSF_errnoGet());
		return bl_Ret;
	}
	bl_Ret = True;
	
	return bl_Ret;
}
/*===========================================================================================================
［機能］	SSDへのデータ書込み（開始セクタ、セクタ数指定）.
-------------------------------------------------------------------------------------------------------------
［引数］	unsigned long ul_SectorNum			開始セクタ番号.
			unsigned long ul_SectorCnt			セクタ数.
			char* buf							書き込むデータのアドレス.
［戻値］	（True : 成功 False : 失敗）.
-------------------------------------------------------------------------------------------------------------
［履歴］	12/05/17 N.Kato 新規作成.
			12/08/20 N.Kato		CQ.366262 ファイルアクセスのClose処理がされない問題修正.
			12/08/28 Gonsho 静的解析対応 fdのcloseを随所に追加.
            13/02/14 Takatsuji	FA14のアサート文字列のリファイン.
            13/10/21 ADS川口 CQ.383235(Act_68509) SYSF_lseekの引数はlongだが、RWチェック領域が.
            　　　　　longで表せるオフセット以上となったためSYSF_lseek→lseekに変更.
            19/10/29 Sky        TAS000000001236対応.
===========================================================================================================*/
Bool THRF_HWCheck_WriteToSSD( uint32_t sec_num, uint32_t sec_cnt, char* buf )	// DEF_SupportSSD.
{
    int32_t ret = 0;
    off_t l_write_ret = 0;
    int32_t fd = -1;
	int32_t size;
	off_t offset = 0;
	Bool bl_Ret = False;								// 戻り値（True：OK  False：NG).

	if( sec_num == 0 ){
        return bl_Ret;
	}
	if( sec_cnt == 0 ){
        return bl_Ret;
	}
	if( buf == NULL ){
        return bl_Ret;
	}

	char *device_path = "/dev/kmsda";
	device_path = THRL_GetPartitionPathFromOffset("/dev/kmsda", sec_num, sec_cnt);

	if( NULL!=device_path )
	{
		fd = SYSF_open( device_path, O_RDWR, 0666 );
	}
    if( fd == -1 ){
		printf("THRF_HWCheck_WriteToSSD open NG. errno %d\n", SYSF_errnoGet());
        return bl_Ret;
    }

	offset = sec_num * DEF_SECTOR_SIZE;								// オフセット位置.

	off_t lseek_ret = lseek( fd, offset, SEEK_SET );
    if( lseek_ret == -1 ){
		printf("THRF_HWCheck_WriteToSSD seek NG. SEEK_SET to %"PRIdPTR" errno %d\n", offset, SYSF_errnoGet());
		SYSF_close( fd );
        return bl_Ret;
    }

	size = sec_cnt * DEF_SECTOR_SIZE;

    l_write_ret = SYSF_write( fd, buf, size );
    if( l_write_ret == -1 ){
		printf("THRF_HWCheck_WriteToSSD write NG. sector %"PRId32" size %"PRId32" errno %d\n", sec_num, size, errno);
		SYSF_close( fd );
        return bl_Ret;
    }
	ret = SYSF_close( fd );
	if ( ret == -1 ) {
		printf("THRF_HWCheck_WriteToSSD close NG. errno %d\n", errno);
		return bl_Ret;
	}
	bl_Ret = True;

	return bl_Ret;
}

/*===========================================================================================================
［機能］	eMMCからのパターンチェックデータ読み込み（終端開始セクタ、サイズ指定）.
-------------------------------------------------------------------------------------------------------------
［引数］	long sec_num					終端からの開始セクタ番号.
			unsigned long size				読み込みデータサイズ(Byte).
			char*	buf						読み込み先アドレス.
［戻値］	（True : 成功 False : 失敗）.
-------------------------------------------------------------------------------------------------------------
［履歴］	16/03/09 yamada CQ.525844 フォーマットID、パターンチェックID修正.
===========================================================================================================*/
Bool THRF_ReadPatternCheck( int32_t sec_num, int32_t size, char* buf )	// DEF_SupportSSD.
{
    int32_t fd;
	int32_t ret = 0;
	Bool bl_Ret = False;								// 戻り値（True：OK  False：NG) .

	if( sec_num == 0 ){
        return bl_Ret;
	}
	if( size == 0 ){
        return bl_Ret;
	}
	if( buf == NULL ){
        return bl_Ret;
	}

	fd = SYSF_open( "/dev/kmsda", O_RDWR, 0666 );  // SSD
	if( fd == -1 ){
		printf("THRF_HWCheck_ReadFromSSD open NG. errno %d\n", SYSF_errnoGet());
        return bl_Ret;
    }

    off_t lseek_ret = lseek( fd, (sec_num * DEF_SECTOR_SIZE), SEEK_END );
    if( lseek_ret == -1 ){
		printf("THRF_HWCheck_ReadFromSSD seek Err. SEEK_SET to %lld errno %d\n", sec_num, errno);
		SYSF_close( fd );
		return bl_Ret;
    }

	ret = SYSF_read( fd, buf, size );
    if( ret == -1 ){
		printf("THRF_HWCheck_ReadFromSSD  read Err. sector %ld size %ld errno %d\n", sec_num, size, SYSF_errnoGet());
		SYSF_close( fd );
		return bl_Ret;
    }
	ret = SYSF_close( fd );
	if ( ret == -1 ) {
		printf("THRF_HWCheck_ReadFromSSD  close Err. errno %d\n", SYSF_errnoGet());
		return bl_Ret;
	}
	bl_Ret = True;
	
	return bl_Ret;
}
/*===========================================================================================================
［機能］	eMMCへのパターンチェックデータ書き込み（終端開始セクタ、サイズ指定）.
-------------------------------------------------------------------------------------------------------------
［引数］	long ul_SectorNum					終端からの開始セクタ番号.
			unsigned long ul_SectorCnt			セクタ数.
			char* buf							書き込むデータのアドレス.
［戻値］	（True : 成功 False : 失敗）.
-------------------------------------------------------------------------------------------------------------
［履歴］	16/03/09 yamada CQ.525844 フォーマットID、パターンチェックID修正.
===========================================================================================================*/
Bool THRF_WritePatternCheck( int32_t sec_num, int32_t sec_cnt, char* buf )	// DEF_SupportSSD.
{
    int32_t ret = 0;
    int32_t fd;
	int32_t size;
	Bool bl_Ret = False;								// 戻り値（True：OK  False：NG).

	if( sec_num == 0 ){
        return bl_Ret;
	}
	if( sec_cnt == 0 ){
        return bl_Ret;
	}
	if( buf == NULL ){
        return bl_Ret;
	}

	fd = SYSF_open( "/dev/kmsda", O_RDWR, 0666 );
    if( fd == -1 ){
		printf("THRF_HWCheck_WriteToSSD open NG. errno %d\n", SYSF_errnoGet());
        return bl_Ret;
    }

	off_t lseek_ret = lseek( fd, (sec_num * DEF_SECTOR_SIZE), SEEK_END );
    if( lseek_ret == -1 ){
		printf("THRF_HWCheck_WriteToSSD seek NG. SEEK_SET to %ld errno %d\n", sec_num, SYSF_errnoGet());
		SYSF_close( fd );
        return bl_Ret;
    }

	size = sec_cnt * DEF_SECTOR_SIZE;

    ret = SYSF_write( fd, buf, size );
    if( ret == -1 ){
		printf("THRF_HWCheck_WriteToSSD write NG. sector %ld size %ld errno %d\n", sec_num, size, errno);
		SYSF_close( fd );
        return bl_Ret;
    }
	ret = SYSF_close( fd );
	if ( ret == -1 ) {
		printf("THRF_HWCheck_WriteToSSD close NG. errno %d\n", errno);
		return bl_Ret;
	}
	bl_Ret = True;

	return bl_Ret;
}

/*===========================================================================================================
［機能］	SSDパターンチェック用データ書込み.
-------------------------------------------------------------------------------------------------------------
［引数］	無し.
［戻値］	void
-------------------------------------------------------------------------------------------------------------
［履歴］	12/05/17 N.Kato 新規作成.
			16/03/09 yamada CQ.525844 フォーマットID、パターンチェックID修正.
===========================================================================================================*/
Bool THRF_HWCheck_TestPatternWriteToSSD( void )		// DEF_SupportSSD.
{
	char w_buf[ DEF_SECTOR_SIZE ]; // 512Byte.
	uint32_t ul_StartSectro = -1;
	Bool bl_Ret = False;

	memset( w_buf, 0, sizeof(w_buf) );
	strncpy( w_buf, DEF_PATTERN_CHECK_DATA, DEF_PATTERN_CHECK_LEN );  // 先頭にパターンデータ("konicaminolta")を設定する.

	// パターンデータを書き込む.
	bl_Ret = THRF_WritePatternCheck( ul_StartSectro, 1, w_buf ); 

	return bl_Ret;
}
#endif		// DEF_SupportSSD.

/*================================================================================================
［機能］	SSDRWチェック開始.
--------------------------------------------------------------------------------------------------
［引数］	無し.
［戻値］	Bool
［詳細］	SSDのRW領域を使用して検査パターンの書込み/読出しを行う。.
 [履歴]     2012/05/17 N.Kato    処理追加.
			2012/08/20 N.Kato	CQ.366260 処理関数の戻り値をチェックする変数型を修正.
================================================================================================*/
Bool THRF_HWCheck_SSDRWCheckStart( void )
{
	Bool bl_Result = False;								// 戻り値（True：OK  False：NG).
	static unsigned char r_buf[DEF_SECTOR_SIZE];	// SSDのRW領域から読み出したデータを格納する配列 512byt領域.
	uint32_t ul_StartSectro = 0;

	//// 検査パターン.
	const static unsigned short us_TestPattern[DEF_SECTOR_SIZE/2] = 
	{
		0x0001, 0x0002, 0x0004, 0x0008,	0x0010, 0x0020, 0x0040, 0x0080,	0x0100, 0x0200, 0x0400, 0x0800,	0x1000, 0x2000, 0x4000, 0x8000,
		0xFFFE, 0xFFFD, 0xFFFB, 0xFFF7,	0xFFEF, 0xFFDF, 0xFFBF, 0xFF7F,	0xFEFF, 0xFDFF, 0xFBFF, 0xF7FF,	0xEFFF, 0xDFFF, 0xBFFF, 0x7FFF,
		0x1111, 0x2222, 0x4444, 0x8888,	0x5050, 0x0505, 0xA0A0, 0x0A0A,	0x5500, 0x0055, 0xAA00, 0x00AA,	0xF0F0, 0x0F0F, 0x0000, 0xFFFF,
		0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007, 0x0008,	0x0009, 0x000A, 0x000B, 0x000C, 0x000D, 0x000E, 0x000F, 0x0010,
		0x0011, 0x0012, 0x0013, 0x0014, 0x0015, 0x0016, 0x0017, 0x0018,	0x0019, 0x001A, 0x001B, 0x001C, 0x001D, 0x001E, 0x001F, 0x0020,
		0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027, 0x0028,	0x0029, 0x002A, 0x002B, 0x002C, 0x002D, 0x002E, 0x002F, 0x0030,
		0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038,	0x0039, 0x003A, 0x003B, 0x003C, 0x003D, 0x003E, 0x003F, 0x0040,
		0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 0x0048,	0x0049, 0x004A, 0x004B, 0x004C, 0x004D, 0x004E, 0x004F, 0x0050,
		0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 0x0058,	0x0059, 0x005A, 0x005B, 0x005C, 0x005D, 0x005E, 0x005F, 0x0060,
		0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 0x0068,	0x0100, 0x0200, 0x0300, 0x0400, 0x0500, 0x0600, 0x0700, 0x0800,
		0x0900, 0x0A00, 0x0B00, 0x0C00, 0x0D00, 0x0E00, 0x0F00, 0x1000,	0x1100, 0x1200, 0x1300, 0x1400, 0x1500, 0x1600, 0x1700, 0x1800,
		0x1900, 0x1A00, 0x1B00, 0x1C00, 0x1D00, 0x1E00, 0x1F00, 0x2000,	0x2100, 0x2200, 0x2300, 0x2400, 0x2500, 0x2600, 0x2700, 0x2800,
		0x2900, 0x2A00, 0x2B00, 0x2C00, 0x2D00, 0x2E00, 0x2F00, 0x3000,	0x3100, 0x3200, 0x3300, 0x3400, 0x3500, 0x3600, 0x3700, 0x3800,
		0x3900, 0x3A00, 0x3B00, 0x3C00, 0x3D00, 0x3E00, 0x3F00, 0x4000,	0x4100, 0x4200, 0x4300, 0x4400, 0x4500, 0x4600, 0x4700, 0x4800,
		0x4900, 0x4A00, 0x4B00, 0x4C00, 0x4D00, 0x4E00, 0x4F00, 0x5000,	0x5100, 0x5200, 0x5300, 0x5400, 0x5500, 0x5600, 0x5700, 0x5800,
		0x5900, 0x5A00, 0x5B00, 0x5C00, 0x5D00, 0x5E00, 0x5F00, 0x6000,	0x6100, 0x6200, 0x6300, 0x6400, 0x6500, 0x6600, 0x6700, 0x6800
	};

#ifdef DEF_SupportSSD
	ul_StartSectro = theHDDTrans.getAreaSector( THRE_CheckArea_SSDRW );	// SSDRWチェック領域開始セクタ.
	if( ul_StartSectro == 0 ){
		return bl_Result;
	}

#ifdef TORNADO		// 実機で有効.
	// SSDRWチェック領域に書込み.
	if( THRF_HWCheck_WriteToSSD( ul_StartSectro, 1, (char *)us_TestPattern ) == False )
	{
		return bl_Result;
	}

	SYSF_taskDelay(FLASH_CHECK_WAIT);

	// SSDRWチェック領域から読み出し.
	if( THRF_HWCheck_ReadFromSSD( ul_StartSectro, DEF_SECTOR_SIZE, (char *)r_buf ) == False )
	{
		return bl_Result;
	}

	// 比較.
	if(memcmp(us_TestPattern, r_buf, DEF_SECTOR_SIZE) == 0)
	{
		bl_Result = True;
	}
#else		// DEF_SupportSSD.
		bl_Result = True;
#endif
#endif		// DEF_SupportSSD.
	return bl_Result;

}	

/*================================================================================================
［機能］	SSDパターンチェック開始.
--------------------------------------------------------------------------------------------------
［引数］	無し.
［戻値］	Bool
［詳細］	SSDから検査パターンの読出しチェックを行う。.
 [履歴]     2012/05/17 N.Kato    処理追加.
			2012/08/20 N.Kato	CQ.366260 処理関数の戻り値をチェックする変数型を修正.
			2016/03/09 yamada    CQ.525844 フォーマットID、パターンチェックID修正.
================================================================================================*/
Bool THRF_HWCheck_SSDPatternCheckStart()
{
	Bool bl_Result = False;									// 戻り値（True：OK  False：NG).
#ifdef TORNADO		// 実機で有効.
	char r_buf[DEF_PATTERN_CHECK_LEN];		// SSDのRW領域から読み出したデータを格納する配列 14byte領域.
	uint32_t ul_StartSectro = 0;
#endif

#ifdef DEF_SupportSSD
#ifdef TORNADO		// 実機で有効.

	// 読出し領域を初期化
	memset( r_buf, 0, sizeof(r_buf) );

	// SSDRWチェック領域から読み出し.
	if( THRF_ReadPatternCheck( ul_StartSectro, DEF_PATTERN_CHECK_LEN, (char *)r_buf ) == False )
	{
		return bl_Result;
	}
	
	SYSF_taskDelay(FLASH_CHECK_WAIT);

	// 比較.
	if(memcmp(DEF_PATTERN_CHECK_DATA, r_buf, DEF_PATTERN_CHECK_LEN) == 0)
	{
		bl_Result = True;
	}
#else
		bl_Result = True;
#endif
#endif		// DEF_SupportSSD.
	return bl_Result;

}	

/*================================================================================================
［機能］	SSD書込み回数チェック.
--------------------------------------------------------------------------------------------------
［引数］	無し.
［戻値］	Bool
［詳細］	SSDのRW領域を使用して検査パターンの書込み/読出しを行う。.
 [履歴]     2012/05/17 N.Kato    処理追加.
			2012/08/20	N.Kato	CQ.366260 処理関数の戻り値をチェックする変数型を修正.
================================================================================================*/
Bool THRF_HWCheck_getSSDLifeCount( uint32_t* ul_Count )
{
	uint32_t ul_Result = 0;
	uint32_t ul_LifeCnt = 0;
	uchar ucPortNo;

	ucPortNo = 0; // 0:SSD

#ifdef DEF_SupportSSD
	ul_Result = THRF_sysBspHddSmartLifeCount( ucPortNo, &ul_LifeCnt );

	if( ul_Result != HDD_STATE_OK ){
		return False;
	}

	*ul_Count = ul_LifeCnt;

	return True;
#else		// DEF_SupportSSD.
	return False;
#endif		// DEF_SupportSSD.
}

/*================================================================================================
［機能］	HDD自己診断機能 I/Fチェック.
--------------------------------------------------------------------------------------------------
［引数］	無し.
［戻値］	Bool
［詳細］	SSDのRW領域を使用して検査パターンの書込み/読出しを行う。.
 [履歴]     2012/05/17 N.Kato    処理追加.
			2013/05/08 Horishita HDDイレースレポート対応で型番の終端がNULL保障される為、NULL文字の文字の設定を削除。
================================================================================================*/
char THRF_HWCheck_HDDCheck_IFCheck( void )
{
	char c_Ret = 0;
	static ST_HDD_DEVICE_INFO	stDeviceInfo;

	// 第一引数 HDD = 1 .
	if( THRF_sysBspHddDeviceInfoGet( 1, 0, &stDeviceInfo ) == 0 ){
		// 正常終了.
		c_Ret = 1;
#ifdef TORNADO		// 実機で有効.
		stDeviceInfo.ctHddVendorId[8] = '\0';   // ベンダー名の終端がNULL保障されていない為。.

		// VenderID スレッドログ出力.
		THRL_SetThLog( 497, THRE_DebugLogType_ThCtrLog, 0, True, (uintptr_t)stDeviceInfo.ctHddVendorId );
		// ProductID スレッドログ出力.
		THRL_SetThLog( 498, THRE_DebugLogType_ThCtrLog, 0, True, (uintptr_t)stDeviceInfo.ctHddProductId );
#endif
	}

	return c_Ret;
}

/*================================================================================================
［機能］	HDD自己診断機能 バッファチェック・メディアチェック.
--------------------------------------------------------------------------------------------------
［引数］	無し.
［戻値］	Bool
［詳細］	
 [履歴]     2012/05/17 N.Kato    処理追加.
			2012/08/20	N.Kato	CQ.366260 処理関数の戻り値をチェックする変数型を修正.
================================================================================================*/
char THRF_HWCheck_HDDCheck_BuffCheck( void )
{
	char sc_Ret = False;

	// S.M.A.R.T発行 1:HDD.
	if( THRF_sysBspHddSmartBuffTest( 1 ) == HDD_STATE_OK ){
		sc_Ret = True;
	}

	return sc_Ret;
}

// 起動シーケンス変更 2012.04.26 start.
/*********************************************************************************/
/* Function Name : hddtrans_file_copy                                            */
/*         Input : cp_path:コピー元ファイル名                                    */
/*         Input : dest_path:コピー先ファイル名                                  */
/*        Output : なし                                                          */
/*       Summary : ファイルコピーを実施する                                      */
/*［履歴］                                                                       */
/*  13/02/14 Takatsuji	履歴追加.                                                */
/*  13/02/14 Takatsuji	FA14のアサート文字列のリファイン.                        */
/*********************************************************************************/
void hddtrans_file_copy( const char* cp_path, const char* dest_path )
{
	struct stat st;
	int32_t ret = stat( cp_path, &st );
	if( ret == 0 ){
		FILE *fi, *fo;
		int32_t c = 0;
		// errnoを表示するため、アサートを、それぞれのopen直後に移動した.
		fi = fopen( cp_path, "r" );
		THRL_ThAssert( fi != NULL, "hddtrans_file_copy() FileOpen Error(fi)!! errno %d", (uintptr_t)errno, 0, 0, 0, CFA14_DetailCode0x0A77 );
		fo = fopen( dest_path, "w" );
		THRL_ThAssert( fo != NULL, "hddtrans_file_copy() FileOpen Error(fo)!! errno %d", (uintptr_t)errno, 0, 0, 0, CFA14_DetailCode0x0A78 );
		c = fgetc( fi );
		while ( EOF != c ) {
			fputc( c, fo );
			c = fgetc( fi );
		}
		fclose( fi );
		fclose( fo );
	}
}

int32_t hddtrans_mkdir( const char* path )
{
	struct stat st;
	int32_t ret = stat( path, &st );
	if( ret != 0 ){
		ret = mkdir( path, 0777 );
	}
	return ret;
}

/* 2014/11/05 大嶋 SSDの暗号化状態に応じたコマンドを生成するよう変更 */
/* 2018/04/12 下垣 パーティション構成変更にともないwork/fwがkmsda9→kmsda14に変更になったため修正 */
// 2018/04/12 Sky_松井 [Eagle]制御構成変更No.97 km/work_fwのマウント先をストレージデバイス構成によって切り替える対応.
/* 2018/05/09 大塚 A4固有処理でA3処理が走らないように修正。又、THRG_GetPartitionTypeを使用するように修正 */
/* 2018/08/09 Sky_井柳 [Eagle]制御構成変更No.99 work_fwがロック領域にあるため、ロック解除処理を追加  */
// 2018/09/14 Sky ソリューション分離V2対応（HDD/SSDのマウント＆fsck制御).
int32_t copy_InstallConfig()
{
	int32_t ret = 0;
#ifndef FUM_LINUX
	char cmd[512] = { '\0' };
#endif // #ifndef FUM_LINUX.
	char path1[256] = { '\0' };
	char path2[256] = { '\0' };
	Bool b_Encrypt = THRG_IsSsdEncrypt();
	char *pc_DevPath_Work_fw = NULL;
#ifdef DEF_A3A4COMM
	part_layout* s_DeviceInfo = NULL;
	s_DeviceInfo = THRG_GetPartitionType();

	if(s_DeviceInfo == NULL)
	{
		printf("copy_InstallConfig error no choice table!!!\n");
		return -1;
	}

	while( s_DeviceInfo->type != PART_TYPE_END )
	{
		if ( !strcmp( THRD_DevName_Ssd_FW , s_DeviceInfo->mntpnt ) )
		{
			break;
		}
		s_DeviceInfo++;
	}
	/* 見つからなかった場合 */
	if( s_DeviceInfo->type == PART_TYPE_END )
	{
		/* エラー終了 */
		printf("not found.[%s]\n",THRD_DevName_Ssd_FW);
		return -1;
	}
	
	//	ロックを解除.
	Bool bl_Res = theHDDTrans.unlockStorage();
	if ( False == bl_Res ) {
		printf( "[%s] Unlock ERR\n", __FUNCTION__);
		return -1;
	}
	
	if(b_Encrypt)
	{
		pc_DevPath_Work_fw = &s_DeviceInfo->devpath[1][0];
	}
	else
	{
		pc_DevPath_Work_fw = &s_DeviceInfo->devpath[0][0];
	}
#else

	// ストレージデバイス構成と暗号化情報に応じてwork_fwのマウント先を変更する.
	TYPE_StorageConfig uc_StorageType = THRG_GetStorageType();

	//	ロックを解除.
	Bool bl_Res = theHDDTrans.unlockStorage();
	if ( False == bl_Res ) {
		printf( "[%s] Unlock ERR\n", __FUNCTION__);
		return -1;
	}

	switch( uc_StorageType ){
	case TYPE_StorageConfig_MicroSD_8GB:	// ストレージデバイス構成μSD8GB時.
		if(b_Encrypt == True){	// 暗号化有無に応じて、パスを切り替える.
			pc_DevPath_Work_fw = THRM_MOUNT_DEV( THRD_DEVFILE_ENCSSD, 13 );
		}else{
			pc_DevPath_Work_fw = THRM_MOUNT_DEV( THRD_DEVFILE_SSD, 13 );
		}
		break;
	case TYPE_StorageConfig_SSD_256GB:		// ストレージデバイス構成SSD256GB時.
	case TYPE_StorageConfig_SSD_1TB:		// ストレージデバイス構成SSD1TB時.
	// SSD 256GBとSSD 1TBは同じ値.
		if(b_Encrypt == True){	// 暗号化有無に応じて、パスを切り替える.
			pc_DevPath_Work_fw = THRM_MOUNT_DEV( THRD_DEVFILE_ENCSSD, 14 );
		}else{
			pc_DevPath_Work_fw = THRM_MOUNT_DEV( THRD_DEVFILE_SSD, 14 );
		}
		break;
	default:	// 取得エラー時.
		printf( "%s: GetStorageType ERR\n", __FUNCTION__);
		return -1;
	}
#endif

#ifdef DEF_SupportSSD

#ifndef FUM_LINUX
	if( !THRG_IsMountPoint( THRD_DevName_Ssd_FW ) )
	{
		// /km/work_fwをマウント.

		// fsck を実行.
		snprintf( cmd, sizeof(cmd), FMT_PART_TUNE2FS0, pc_DevPath_Work_fw );
		strcat( cmd, " 1> /dev/null 2>&1" );
		ret = system( cmd );
		if( WEXITSTATUS( ret ) ){
			printf( "%s: ERR: cmd:%s\n", __FUNCTION__, cmd );
			return -1;
		}
		snprintf( cmd, sizeof(cmd), FMT_PART_FSCK1ST, "ext3", pc_DevPath_Work_fw );
		strcat( cmd, " 1> /dev/null 2>&1" );
		ret = system( cmd );
		if( WEXITSTATUS( ret ) > 1 ){
			printf( "%s: ERR: cmd:%s\n", __FUNCTION__, cmd );
			snprintf( cmd, sizeof(cmd), FMT_PART_FSCK2ND, "ext3", pc_DevPath_Work_fw );
			strcat( cmd, " 1> /dev/null 2>&1" );
			ret = system( cmd );
			if( WEXITSTATUS( ret ) > 1 ){
				printf( "%s: ERR: cmd:%s\n", __FUNCTION__, cmd );
				return -1;
			}
		}

		// マウント.
		ret = mount( pc_DevPath_Work_fw, THRD_DevName_Ssd_FW, "ext3", MS_NOATIME, EXT3_MOUNT_OPTIONS );
		if( ret == -1 ){
			printf( "%s: ERR: mount:%d\n", __FUNCTION__, SYSF_errnoGet() );
			return -1;
		}
	}
#endif // #ifndef FUM_LINUX.

	// /ramdisk配下にディレクトリを作成.
	ret = hddtrans_mkdir( THRD_APPM_RAM_PATH );
	if( ret == -1 ){
		printf( "%s: ERR: mkdir:%d\n", __FUNCTION__, SYSF_errnoGet() );
		umount2( THRD_DevName_Ssd_FW, MNT_FORCE );
		return -1;
	}
	int32_t c_ret = chmod( THRD_APPM_RAM_PATH,  S_IRUSR | S_IWUSR | S_IXUSR | S_IROTH | S_IWOTH | S_IXOTH );
	if(c_ret != 0){
		printf( "[%s:%d] (%s) chmod is failed.(%d)\n", __PRETTY_FUNCTION__, __LINE__, THRD_APPM_RAM_PATH, errno );
		umount2( THRD_DevName_Ssd_FW, MNT_FORCE );
		return -1;
	}
	snprintf( path1, sizeof(path1), "%s/%s", THRD_APPM_RAM_PATH, THRD_APPM_DIR_NAME );
	ret = hddtrans_mkdir( path1 );
	if( ret == -1 ){
		printf( "%s: ERR: mkdir:%d\n", __FUNCTION__, SYSF_errnoGet() );
		umount2( THRD_DevName_Ssd_FW, MNT_FORCE );
		return -1;
	}

	// インストールアプリ管理ファイルを、/ramdisk 配下にコピー
	snprintf( path1, sizeof(path1), "%s/%s/%s", THRD_APPM_CONFIG_PATH, THRD_APPM_DIR_NAME, THRD_APPM_PREINST_CONFIG );
	snprintf( path2, sizeof(path2), "%s/%s/%s", THRD_APPM_RAM_PATH, THRD_APPM_DIR_NAME, THRD_APPM_PREINST_CONFIG );
	hddtrans_file_copy( path1, path2 );
	snprintf( path1, sizeof(path1), "%s/%s/%s", THRD_APPM_CONFIG_PATH, THRD_APPM_DIR_NAME, THRD_APPM_INST_CONFIG );
	snprintf( path2, sizeof(path2), "%s/%s/%s", THRD_APPM_RAM_PATH, THRD_APPM_DIR_NAME, THRD_APPM_INST_CONFIG );
	hddtrans_file_copy( path1, path2 );

#ifndef FUM_LINUX
	// アンマウント.
	ret = umount2( THRD_DevName_Ssd_FW, MNT_FORCE );
	if( ret == -1 ){
		printf( "%s: ERR: umount2:%d\n", __FUNCTION__, SYSF_errnoGet() );
		return -1;
	}
#endif // #ifndef FUM_LINUX.

#endif //DEF_SupportSSD

	return 0;
}
// 起動シーケンス変更 2012.04.26 end.


/*================================================================================================
［機能］	マウントポイントの状態をチェックする.
--------------------------------------------------------------------------------------------------
［引数］	マウントポイントへのパス.
［戻値］	Bool	: True（マウントされている）、 False（マウントされていない）.
［詳細］	チェック対象のマウントポイントへのパス(e.g. "/km/work_fw"など)を引数に取る。
			引数に渡されたマウントポイントが、マウントされていればTrueを返し、
			マウントされていない（エラー含む）場合はFalseを返す。
［履歴］	2018/09/14 Sky 新規作成 ソリューション分離V2対応（HDD/SSDのマウント＆fsck制御).
================================================================================================*/
Bool THRG_IsMountPoint( const char *target )
{
	// 引数チェック処理.
	if( NULL==target )
	{
		return False;
	}
	if( 0==strlen(target) )
	{
		return False;
	}

	// コマンド文字列を生成.
	char cmd[128] = {0};
	snprintf( cmd, sizeof(cmd)-1, "mountpoint -q %s", target );

	// mountpointをシステムコール.
	int ret_mountpoint = system( cmd );
	if( !WIFEXITED(ret_mountpoint) )
	{
		// mountpointコマンドのプロセスが正常終了しなかった.
		return False;
	}

	// mountpointの戻り値から、マウントされているかどうかを判定.
	Bool IsMountPoint = True;
	if( 0==WEXITSTATUS( ret_mountpoint ) )
	{
		IsMountPoint = True; // 終了ステータス(0)：マウントされている.
	}
	else
	{
		IsMountPoint = False; // 終了ステータス(0)以外：マウントされていない.
	}
	return IsMountPoint;
}



// 2013/06/17 ADS北村	CQ.381189(Act_66229) EE-SSC_4	HDDレス化対応_SSDへのスワップ対応.
// 2013/09/12 ADS滕飛	CQ.381189(Act_68354) EE-HDL-33	HDDレス化対応_送信履歴、デバッグログ、操作ログ、S/MIME、Emp/Fmt対応.
#if defined( DEF_SupportSSD )
/*================================================================================================
［機能］	スワップ領域の格納先がHDDかSSDかを取得する.
--------------------------------------------------------------------------------------------------
［引数］	無し.
［戻値］	Bool	: True（スワップ格納先がSSD）、 False（スワップ格納先がHDD）.
［詳細］	スワップ領域の格納先がHDDかSSDかを取得する.
［履歴］	13/06/17 ADS北村    新規作成.
================================================================================================*/
Bool CP_HDDTrans::isSwapAccessTargetSSD()	// DEF_SupportSSD.
{
#if defined(SIMULATOR)
	return False;    /* FUM_LINUX有効時、こちらが適用 */
#else	/* #if defined(SIMULATOR) */
	Bool bl_SwapTarget;
	// ファイルから情報を読んでm_us_RawAccessTargetに結果をセットする関数を呼ぶ.
	readRawAccessTarget();
	// m_us_RawAccessTargetにスワップ格納先がSSDを示すビットが立っているかチェック.
	if( DEF_SWAP_TARGET_SSD == ( DEF_SWAP_TARGET_SSD & m_us_RawAccessTarget ) ) {
		bl_SwapTarget = True;
	} else {
		bl_SwapTarget = False;
	}
	return bl_SwapTarget;
#endif	/* #if defined(SIMULATOR) */
}

/*================================================================================================
［機能］	送信履歴の格納先がHDDかSSDかを取得する.
--------------------------------------------------------------------------------------------------
［引数］	無し.
［戻値］	Bool	: True（送信履歴の格納先がSSD）、 False（送信履歴の格納先がHDD）.
［詳細］	送信履歴の格納先がHDDかSSDかを取得する.
［履歴］	13/09/12 ADS滕飛    新規作成.
================================================================================================*/
Bool CP_HDDTrans::isTransHistoryAccessTargetSSD()	// DEF_SupportSSD.
{
#if defined(SIMULATOR)
	return False;    /* FUM_LINUX有効時、こちらが適用 */
#else	/* #if defined(SIMULATOR) */
	Bool bl_TransHistoryTarget;
	// ファイルから情報を読んでm_us_RawAccessTargetに結果をセットする関数を呼ぶ.
	readRawAccessTarget();
	// m_us_RawAccessTargetに送信履歴格納先がSSDを示すビットが立っているかチェック.
	if( DEF_TRANS_HISTORY_TARGET_SSD == ( DEF_TRANS_HISTORY_TARGET_SSD & m_us_RawAccessTarget ) ) {
		bl_TransHistoryTarget = True;
	} else {
		bl_TransHistoryTarget = False;
	}
	return bl_TransHistoryTarget;
#endif	/* #if defined(SIMULATOR) */
}

/*================================================================================================
［機能］	デバッグログの格納先がHDDかSSDかを取得する.
--------------------------------------------------------------------------------------------------
［引数］	無し.
［戻値］	Bool	: True（デバッグログの格納先がSSD）、 False（デバッグログの格納先がHDD）.
［詳細］	デバッグログの格納先がHDDかSSDかを取得する.
［履歴］	13/09/12 ADS滕飛    新規作成.
================================================================================================*/
Bool CP_HDDTrans::isDbgLogAccessTargetSSD()	// DEF_SupportSSD.
{
#if defined(SIMULATOR)
	return False;    /* FUM_LINUX有効時、こちらが適用 */
#else	/* #if defined(SIMULATOR) */
	Bool bl_DbgLogTarget;
	// ファイルから情報を読んでm_us_RawAccessTargetに結果をセットする関数を呼ぶ.
	readRawAccessTarget();
	// m_us_RawAccessTargetにデバッグログ格納先がSSDを示すビットが立っているかチェック.
	if( DEF_DBG_LOG_TARGET_SSD == ( DEF_DBG_LOG_TARGET_SSD & m_us_RawAccessTarget ) ) {
		bl_DbgLogTarget = True;
	} else {
		bl_DbgLogTarget = False;
	}
	return bl_DbgLogTarget;
#endif	/* #if defined(SIMULATOR) */
}

/*================================================================================================
［機能］	操作ログの格納先がHDDかSSDかを取得する.
--------------------------------------------------------------------------------------------------
［引数］	無し.
［戻値］	Bool	: True（操作ログの格納先がSSD）、 False（操作ログの格納先がHDD）.
［詳細］	操作ログの格納先がHDDかSSDかを取得する.
［履歴］	13/09/12 ADS滕飛    新規作成.
================================================================================================*/
Bool CP_HDDTrans::isOplLogAccessTargetSSD()	// DEF_SupportSSD.
{
#if defined(SIMULATOR)
	return False;    /* FUM_LINUX有効時、こちらが適用 */
#else	/* #if defined(SIMULATOR) */
	Bool bl_OplLogTarget;
	// ファイルから情報を読んでm_us_RawAccessTargetに結果をセットする関数を呼ぶ.
	readRawAccessTarget();
	// m_us_RawAccessTargetに操作ログ格納先がSSDを示すビットが立っているかチェック.
	if( DEF_OPL_LOG_TARGET_SSD == ( DEF_OPL_LOG_TARGET_SSD & m_us_RawAccessTarget ) ) {
		bl_OplLogTarget = True;
	} else {
		bl_OplLogTarget = False;
	}
	return bl_OplLogTarget;
#endif	/* #if defined(SIMULATOR) */
}

/*================================================================================================
［機能］	S/MIME証明書の格納先がHDDかSSDかを取得する.
--------------------------------------------------------------------------------------------------
［引数］	無し.
［戻値］	Bool	: True（S/MIME証明書の格納先がSSD）、 False（S/MIME証明書の格納先がHDD）.
［詳細］	S/MIME証明書の格納先がHDDかSSDかを取得する.
［履歴］	13/09/25 ADS滕飛    新規作成.
================================================================================================*/
Bool CP_HDDTrans::isSmimeCertAccessTargetSSD()	// DEF_SupportSSD.
{
#if defined(SIMULATOR)
	return False;    /* FUM_LINUX有効時、こちらが適用 */
#else	/* #if defined(SIMULATOR) */
	Bool bl_SmimeCertTarget;
	// ファイルから情報を読んでm_us_RawAccessTargetに結果をセットする関数を呼ぶ.
	readRawAccessTarget();
	// m_us_RawAccessTargetにS/MIME証明書の格納先がSSDを示すビットが立っているかチェック.
	if( DEF_SMIME_CERT_TARGET_SSD == ( DEF_SMIME_CERT_TARGET_SSD & m_us_RawAccessTarget ) ) {
		bl_SmimeCertTarget = True;
	} else {
		bl_SmimeCertTarget = False;
	}
	return bl_SmimeCertTarget;
#endif	/* #if defined(SIMULATOR) */
}

/*================================================================================================
［機能］	Emp/Fmtの格納先がHDDかSSDかを取得する.
--------------------------------------------------------------------------------------------------
［引数］	無し.
［戻値］	Bool	: True（Emp/Fmtの格納先がSSD）、 False（Emp/Fmtの格納先がHDD）.
［詳細］	Emp/Fmtの格納先がHDDかSSDかを取得する.
［履歴］	13/09/25 ADS安田    新規作成.
================================================================================================*/
Bool CP_HDDTrans::isEmpFmtAccessTargetSSD()	// DEF_SupportSSD.
{
#if defined(SIMULATOR)
	return False;    /* FUM_LINUX有効時、こちらが適用 */
#else	/* #if defined(SIMULATOR) */
	Bool bl_EmpFmtTarget;
	// ファイルから情報を読んでm_us_RawAccessTargetに結果をセットする関数を呼ぶ.
	readRawAccessTarget();
	// m_us_RawAccessTargetにEmp/Fmtの格納先がSSDを示すビットが立っているかチェック.
	if( DEF_EMP_FMT_TARGET_SSD == ( DEF_EMP_FMT_TARGET_SSD & m_us_RawAccessTarget ) ) {
		bl_EmpFmtTarget = True;
	} else {
		bl_EmpFmtTarget = False;
	}
	return bl_EmpFmtTarget;
#endif	/* #if defined(SIMULATOR) */
}

/*================================================================================================
［機能］	RawAccessの対象を動的に切替えるためのファイルを読み、情報をメンバ変数に設定する.
--------------------------------------------------------------------------------------------------
［引数］	無し.
［戻値］	void.
［詳細］	RawAccessの対象を動的に切替えるためのファイルを読み、情報をメンバ変数に設定する.
［履歴］	13/06/17 ADS北村    新規作成.
			13/09/12 ADS滕飛 CQ.381189(Act_68354) EE-HDL-33 HDDレス化対応_16進数対応.
			13/10/31 ADS滕飛 CQ.381189(Act_68696) EE-HDL-33 HDDレス化対応_切替条件変更(メモリ搭載容量判定追加).
================================================================================================*/
void CP_HDDTrans::readRawAccessTarget()	// DEF_SupportSSD.
{
	int32_t			i_RawAccessFd;		// ファイルディスクリプタ.
	off_t		sz_ReadSize;		// 読み込みサイズ.
	char		c_RawAccessTarget[ 4 + 1 ];	// 読み込みバッファ(ushortの最大値未満になるよう4桁まで読み込み可とする).
	ushort		us_RawAccessTarget;	// 読み込みバッファを数値化する領域.
	char		*pc_end;			// 変換不可文字.

	// 初期化ビットが立っていない場合は、値を取得済みなので何もしない.
	if( DEF_NONINITIALIZE_RAW_ACCESS_TARGET != ( DEF_NONINITIALIZE_RAW_ACCESS_TARGET & m_us_RawAccessTarget ) ) {
		return;
	}

	// 変数の初期化.
	memset( c_RawAccessTarget, 0, sizeof( c_RawAccessTarget ) );

	// ファイルから読み込み.
	i_RawAccessFd = SYSF_Open( DEF_FILEPATH_RAW_ACCESS_TARGET, O_RDONLY );

	// ファイルオープンが失敗(-1)以外の場合.
	if( -1 != i_RawAccessFd ) {
		// 読み込みしてファイルクローズ.
		sz_ReadSize = SYSF_Read( i_RawAccessFd, c_RawAccessTarget, 4 ); // 4桁読み込み可とする.
		SYSF_Close( i_RawAccessFd );
		// ファイルリードが失敗(-1)以外の場合.
		if( -1 != sz_ReadSize ) {
			// 先頭文字が「-」または「+」の場合はデフォルト値を設定.
			if( c_RawAccessTarget[0] == '-' || c_RawAccessTarget[0] == '+' ) {
				// 入力値エラー。デフォルト値を設定.
				if( 0 < us_DebugEnable ) {
					printf( "CP_HDDTrans::readRawAccessTarget() input error. input=%s\n" , c_RawAccessTarget );
				}
				// メモリ搭載容量判定.
				if( DIMM_MEMSIZE_ALL2 >= APIC_MemoryInfo::newInstance()->getWorkMemorySize() ){
					m_us_RawAccessTarget = DEF_DEFAULT_RAW_ACCESS_TARGET_1G;		// メモリが1GB構成の場合.
				} else {
					m_us_RawAccessTarget = DEF_DEFAULT_RAW_ACCESS_TARGET_NOT_1G;	// メモリが1GB以外構成の場合.
				}
				// 設定情報をログに出力.
				printf( "Raw Access Target Setting = %X\n", m_us_RawAccessTarget );
				return;
			}
			// 改行コードがあれば「\0」に置き換える.
			for( int32_t i=0; i<=4; i++ )	// 4桁読み込み可とする.
			{
				if( c_RawAccessTarget[i] == '\n' ) {
					c_RawAccessTarget[i] = '\0';
					break;
				}
			}

			// 取得した文字列を数値化(16進数).
			us_RawAccessTarget = (ushort)strtol( c_RawAccessTarget, &pc_end, 16 );

			// 変換不可文字あり.
			if( *pc_end != '\0' ) {
				// 入力値エラー。デフォルト値を設定.
				if( 0 < us_DebugEnable ) {
					printf( "CP_HDDTrans::readRawAccessTarget() input error. input=%s\n" , c_RawAccessTarget );
				}
				// メモリ搭載容量判定.
				if( DIMM_MEMSIZE_ALL2 >= APIC_MemoryInfo::newInstance()->getWorkMemorySize() ){
					m_us_RawAccessTarget = DEF_DEFAULT_RAW_ACCESS_TARGET_1G;		// メモリが1GB構成の場合.
				} else {
					m_us_RawAccessTarget = DEF_DEFAULT_RAW_ACCESS_TARGET_NOT_1G;	// メモリが1GB以外構成の場合.
				}
				// 設定情報をログに出力.
				printf( "Raw Access Target Setting = %X\n", m_us_RawAccessTarget );
				return;
			}

			// us_RawAccessTargetにDEF_RAW_ACCESS_TARGET_MASK範囲外のビットが立っていた場合.
			if( 0 != ( ( ~DEF_RAW_ACCESS_TARGET_MASK) & us_RawAccessTarget ) ) {
				// 入力値エラー。デフォルト値を設定.
				if( 0 < us_DebugEnable ) {
					printf( "CP_HDDTrans::readRawAccessTarget() input error. input=%s\n" , c_RawAccessTarget );
				}
				// メモリ搭載容量判定.
				if( DIMM_MEMSIZE_ALL2 >= APIC_MemoryInfo::newInstance()->getWorkMemorySize() ){
					us_RawAccessTarget = DEF_DEFAULT_RAW_ACCESS_TARGET_1G;		// メモリが1GB構成の場合.
				} else {
					us_RawAccessTarget = DEF_DEFAULT_RAW_ACCESS_TARGET_NOT_1G;	// メモリが1GB以外構成の場合.
				}
			}
		} else {
			// ファイルリード失敗時。デフォルト値を設定.
			if( 0 < us_DebugEnable ) {
				printf( "CP_HDDTrans::readRawAccessTarget() read() error. errno=%d\n" , errno );
			}
			// メモリ搭載容量判定.
			if( DIMM_MEMSIZE_ALL2 >= APIC_MemoryInfo::newInstance()->getWorkMemorySize() ){
				us_RawAccessTarget = DEF_DEFAULT_RAW_ACCESS_TARGET_1G;		// メモリが1GB構成の場合.
			} else {
				us_RawAccessTarget = DEF_DEFAULT_RAW_ACCESS_TARGET_NOT_1G;	// メモリが1GB以外構成の場合.
			}
		}
	} else {
		// ファイルオープン失敗時。デフォルト値を設定.
		if( 0 < us_DebugEnable ) {
			printf( "CP_HDDTrans::readRawAccessTarget() open() error. errno=%d\n" , errno );
		}
		// メモリ搭載容量判定.
		if( DIMM_MEMSIZE_ALL2 >= APIC_MemoryInfo::newInstance()->getWorkMemorySize() ){
			us_RawAccessTarget = DEF_DEFAULT_RAW_ACCESS_TARGET_1G;		// メモリが1GB構成の場合.
		} else {
			us_RawAccessTarget = DEF_DEFAULT_RAW_ACCESS_TARGET_NOT_1G;	// メモリが1GB以外構成の場合.
		}
	}

	// メンバ変数に値を入力.
	m_us_RawAccessTarget = us_RawAccessTarget;
	// 設定情報をログに出力.
	printf( "Raw Access Target Setting = %X\n", m_us_RawAccessTarget );

	return;
}

/*================================================================================================
［機能］	スワップ転送判定.
--------------------------------------------------------------------------------------------------
［引数］	BE_TaskMessage			:タスクメッセージ
			ulong					:オプション情報（ST_MsgHDDTransInfo）
［戻値］	Bool					:スワップ領域へ転送（画像データまたは画像管理情報）の場合、Trueを返す。
									:それ以外はFalseを返す。
［詳細］	スワップ領域へ転送（画像データまたは画像管理情報）の場合、Trueを返す。それ以外はFalseを返す。
［履歴］	13/06/17 ADS北村    新規作成.
================================================================================================*/
Bool CP_HDDTrans::isTransSwap( BE_TaskMessage be_TaskMessage, uintptr_t ul_Option )	// DEF_SupportSSD.
{
	Bool bl_Ans = False;
	ST_MsgHDDTransInfo* pst_MsgHDDTransInfo = NULL;
	IMGC_Image* pc_Image = NULL;

	// 転送開始要求.
	if( TM_HDDTransportStartReq == be_TaskMessage ) {
		// 一時データ上書き消去の場合.
		if( True == isReqEraseData( ul_Option ) ) {
			bl_Ans = True;
		// Fat転送以外の場合.
		} else if( False == isTransFat( ul_Option ) ) {
			pst_MsgHDDTransInfo = (ST_MsgHDDTransInfo*)ul_Option;
			pc_Image = pst_MsgHDDTransInfo->pcc_OneInputImage;
			if( NULL != pc_Image && NULL != pc_Image->m_IMGC_ImageImp ) {
				// スワップ領域への転送処理.
				switch( pc_Image->m_IMGC_ImageImp->e_IPEType ) {
					// 通常画像.
					case IMGE_IPEType_Non:
					// 長尺通常.
					case IMGE_IPEType_LongNormal:
						// 画像アレイまたは画像管理情報の場合.
						if( EM_TransDataKind_Image == pst_MsgHDDTransInfo->em_TransDataKind
							|| EM_TransDataKind_Info == pst_MsgHDDTransInfo->em_TransDataKind ) {
							bl_Ans = True;
						}
						break;
					//その他.
					default:
						break;
				}
			}
		}
	}
	return bl_Ans;
}

#endif //DEF_SupportSSD.

//2013/08/06 伊藤史.
void dbgHDDimageClose()
{
	if( NULL != fp_debugHDDimage ){
		fclose(fp_debugHDDimage);
		fp_debugHDDimage = NULL;
		printf("dbgHDDimageClose Image\n");
	}
	if( NULL != fp_debugHDDinfo ){
		fclose(fp_debugHDDinfo);
		fp_debugHDDinfo = NULL;
		printf("dbgHDDimageClose Info\n");
	}
}

//2013/08/06 伊藤史.
void dbgHDDimageOpen(
	uint32_t	ul_JobID,		// スレッドを選択したジョブID
	short	ss_InPageID,		//	入力ページＩＤ (-1:なし)
	short	ss_mode
	)
{
	char	scPathname[256];

	if( 0 == us_debug_HDDcapture ){
		fp_debugHDDimage = NULL;
		fp_debugHDDinfo = NULL;
		return;
	}

	dbgHDDimageClose();

	sprintf(scPathname,"/ata01/Capture/Job%"PRId32"-Page%d-mod%d-Image.dat",ul_JobID,ss_InPageID,ss_mode);
	fp_debugHDDimage = fopen(scPathname,"w+");
	if( NULL != fp_debugHDDimage ){
		printf("dbgHDDimageOpen %s\n",scPathname);
	}
	
	sprintf(scPathname,"/ata01/Capture/Job%"PRId32"-Page%d-mod%d-Info.dat",ul_JobID,ss_InPageID,ss_mode);
	fp_debugHDDinfo = fopen(scPathname,"w+");
	if( NULL != fp_debugHDDinfo ){
		printf("dbgHDDimageOpen %s\n",scPathname);
	}
}

// ================================================================================================
//! <!--機能--> ストレージ全領域サイズ取得関数(外部公開API).
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param	なし.
//! <!--戻値-->
//! \return	DLONG : ストレージデバイスの総容量(byte).
//! <!--詳細-->
//! <!--履歴-->
//	14/07/09 yamada		制御構成変更No39 eMMC容量取得対応.
//	17/10/26 Sky		パーティション一本化対応.
// ================================================================================================
DLONG THRG_GetStorageSizeTotal()
{
	// 自動変数のDLONGを戻り値として設定すると SegFが発生する為、static を付与する.
	static DLONG dl_TotalSize = 0;

	if (theHDDTrans.getNvRamDeviceType() == THRE_StorageDeviceTypeHDDAndSSD) {
		// HDDのパーティション割り当て合計サイズを取得する.
		dl_TotalSize = theHDDTrans.getHddFixedSectorNum();
	} else {
		// eMMCのパーティション割り当て合計サイズを取得する.
		dl_TotalSize = theHDDTrans.getSsdFixedSectorNum();
	}

	// セクタ単位で取得した値を byte単位に変換する.
	dl_TotalSize = dl_TotalSize * (DLONG)DEF_SECTOR_SIZE;

	return dl_TotalSize;
}

// ================================================================================================
//! <!--機能--> ストレージ使用領域サイズ取得(外部公開API).
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param	なし.
//! <!--戻値-->
//! \return	DLONG : ストレージデバイスの使用容量(byte).
//! <!--詳細-->
//! 使用容量 = ストレージ総容量 - Box蓄積空き領域 - Box蓄積管理空き領域.
//! <!--履歴-->
//	14/07/09 yamada		制御構成変更No39 eMMC容量取得対応.
//	14/09/24 Sky鈴木	CQ503708 ストレージ容量の「使用領域」と「残量」の値が毎回違う対応.
// ================================================================================================
DLONG THRG_GetUsedStorageSizeTotal()
{
	// comMemoryManagerのインスタンス生成(空き容量取得に使用).
	IMGC_ImageMemoryPoolImpColorFileMemoryInstance& comMemoryManager = IMGC_ImageMemoryPoolImpColorFileMemorySingletonIf::newInstance();

	// 使用容量 = ストレージ総容量 - Box蓄積空き領域 - Box蓄積管理空き領域.
	DLONG dl_HddBoxFreeSize = comMemoryManager.GetFreeHDDSize(1);		// Box蓄積空き容量.
	DLONG dl_HddBoxInfoFreeSize = comMemoryManager.GetFreeHDDSize(3);	// Box蓄積管理空き容量.
	// 自動変数のDLONGを戻り値として設定すると SegFが発生する為、static を付与する.
	static DLONG dl_TotalUseSize = 0;									// ストレージ使用量（戻り値）.
	DLONG dl_TotalSize = THRG_GetStorageSizeTotal();					// ストレージ総容量.

	// 使用容量の算出.
	dl_TotalUseSize = dl_TotalSize - dl_HddBoxFreeSize - dl_HddBoxInfoFreeSize;

	return dl_TotalUseSize;

}

// ================================================================================================.
//! <!--機能--> ストレージ構成返却API.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param	なし.
//! <!--戻値-->.
//! \return TYPE_StorageConfig.
//!						ストレージデバイス容量1TB以上	TYPE_StorageConfig_SSD_1TB(2).
//!						ストレージデバイス容量256GB以上	TYPE_StorageConfig_SSD_256GB(1).
//!						ストレージデバイス容量8GB以上	TYPE_StorageConfig_MicroSD_8GB (3).
//!						ストレージデバイス容量取得エラーTYPE_StorageConfig_None(0).
//! <!--詳細-->.
//!	装着されているストレージサイズを取得し.
//!	1TB以上か、256GB以上か、8GB以上か取得エラーを返却する.
//! <!--履歴-->.
//!	18/03/05 Sky松井	【Eagle】制御構成変更No.97 新規作成.
//! 18/04/29 Sky松井	[Eagle] 制御構成変更No.97 ログを残すタイミング変更、取得エラーを返却した後再度判定を行わないよう対応.
//!	2018/04/05 大塚	[Sparrow]制御構成変更No.97 ストレージ構成変更(再).
//! 2019/03/08 Sky合屋 [EagleH]【OP_BTS-3685】パーティションサイズが1TB以上の時の処理を無効化する.
//! 2019/04/02 Sky鳥居 [EagleH] Warning除去対応.
//! 2019/07/12 Sky     [RQ-560] 1TBSSD対応に伴い、【OP_BTS-3685】対応をロールバック.
//! ================================================================================================.
TYPE_StorageConfig THRG_GetStorageType( void ){
	static Bool bl_Typeflg = False;
	static TYPE_StorageConfig uc_StrageTypeNo =TYPE_StorageConfig_None;

	// 1回実行された後は1回目実行時の値を返却する.
	if( bl_Typeflg == True ){
		return uc_StrageTypeNo;
	}
	bl_Typeflg = True;
	// μSD8GBのパーティション作成に必要なサイズを取得.
	int64_t ll_StrageSize_8G = THRG_GetPartirionSize( sd_part_info_8GB );
	// SSD256GBのパーティション作成に必要なサイズを取得.
	int64_t ll_StrageSize_256GB = THRG_GetPartirionSize( ssd_part_info_256GB );
	// SSD1TBのパーティション作成に必要なサイズを取得.
	int64_t ll_StrageSize_1TB = THRG_GetPartirionSize( ssd_part_info_1TB );
#ifdef DEF_A3A4COMM
	int64_t ll_StrageSize_16G = THRG_GetPartirionSize( sd_part_info_16GB );
#endif

	// 実際に装着されているストレージデバイスサイズを取得.
	int64_t ll_PhysicStorageSize = THRG_GetPhysicaleMMCSize();

	setDebugLowLogSpeedSeq( 890, (uint32_t)( 0xFFFFFFFFUL & (ll_PhysicStorageSize >> 32)) );	// 上位32ビットをログに保存.
	setDebugLowLogSpeedSeq( 891, (uint32_t)( 0xFFFFFFFFUL & ll_PhysicStorageSize) );			// 下位32ビットをログに保存.

	if( ll_StrageSize_1TB <= ll_PhysicStorageSize ){	// 1TB以上の場合.
		uc_StrageTypeNo = TYPE_StorageConfig_SSD_1TB;	// 戻り値に1TBを設定.
		// debugLog 92に残す.
		setDebugLowLogSpeedSeq( 892, uc_StrageTypeNo );
		return uc_StrageTypeNo;				// 戻り値として1TBを返却.
	}
	if( ll_StrageSize_256GB <= ll_PhysicStorageSize ){	// 256GB以上の場合.
		uc_StrageTypeNo = TYPE_StorageConfig_SSD_256GB;	// 戻り値に256GBを設定.
		// debugLog 92に残す.
		setDebugLowLogSpeedSeq( 892, uc_StrageTypeNo );
		return uc_StrageTypeNo;			// 戻り値として256GBを返却.
	}
#ifdef DEF_A3A4COMM
	if( ll_StrageSize_16G <= ll_PhysicStorageSize ){	// 256GB以上の場合.
		uc_StrageTypeNo = TYPE_StorageConfig_MicroSD_16GB;	// 戻り値に16GBを設定.
		// debugLog 92に残す.
		setDebugLowLogSpeedSeq( 892, uc_StrageTypeNo );
		return uc_StrageTypeNo;			// 戻り値として16GBを返却.
	}
#endif
	if( ll_StrageSize_8G <= ll_PhysicStorageSize ){		// 8GB以上の場合.
		uc_StrageTypeNo = TYPE_StorageConfig_MicroSD_8GB;	// 戻り値に8GBを設定
		// debugLog 92に残す.
		setDebugLowLogSpeedSeq( 892, uc_StrageTypeNo );
		return uc_StrageTypeNo ;			// 戻り値として8GBを返却.
	}
	// 8GB未満の場合には取得エラーを返却.
	setDebugLowLogSpeedSeq( 892, uc_StrageTypeNo );
	return uc_StrageTypeNo;
}

// ================================================================================================.
//! <!--機能--> ストレージサイズに応じたSwap管理情報最大ページ数を返却する.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param	なし.
//! <!--戻値-->.
//! \return int64_t Swap管理情報最大ページ数. 
//! 				取得失敗時は、THRD_GET_STOR_ERROR_NUM(0).
//! <!--詳細-->.
//!	ストレージサイズに応じたSwap管理情報最大ページ数を返却する.
//! <!--履歴-->.
//!	17/10/24 Sky松井	新規作成.
//! 19/04/03 Sky_松井	[Eagle]制御構成変更No.97 ストレージデバイスサイズではなく、ストレージ構成取得APIにて判定を行うよう修正.
//!	2018/04/05 大塚	[Sparrow]制御構成変更No.97 ストレージ構成変更(再).
//! 18/09/11 AR:1060519 eSOL田川 静的解析対応.
//! ================================================================================================.
int64_t THRG_GetSwap_InfoMaxPage ( void )
{
	static int64_t ll_result = THRD_GET_STOR_ERROR_NUM;	// 戻り値に取得失敗：THRD_GET_STOR_ERROR_NUM(0)をセット.

	// Swap管理情報最大ページ数が取得エラーの場合は装着されているストレージ構成を取得し判定を行う.
	if( ll_result == THRD_GET_STOR_ERROR_NUM ){
		// ストレージデバイス構成を取得.
		TYPE_StorageConfig uc_StorageType= THRG_GetStorageType();

		switch( uc_StorageType ){
		case TYPE_StorageConfig_MicroSD_8GB:	// ストレージデバイス構成μSD8GB時.
			ll_result = DEF_SWAP_INFO_MAX_PAGE_SD_8G;			// 戻り値に8GBのSwap管理情報最大ページ数をセット.
			break;
		case TYPE_StorageConfig_SSD_256GB:
			ll_result = DEF_SWAP_INFO_MAX_PAGE_SSD_256G;		// 戻り値に256GBのSwap管理情報最大ページ数をセット.
			break;
		case TYPE_StorageConfig_SSD_1TB:
			ll_result = DEF_SWAP_INFO_MAX_PAGE_SSD_1T;			// 戻り値に1TBのSwap管理情報最大ページ数をセット.
																// (現時点で、256GB / 1TBのSwap管理情報最大ページ数は同じ値).
			break;
#ifdef DEF_A3A4COMM
		case TYPE_StorageConfig_MicroSD_16GB: 	// ストレージ構成が16GBの場合.
			ll_result = DEF_SWAP_INFO_MAX_PAGE_SD_16G;			// 戻り値に8GBのSwap管理情報最大ページ数をセット.
			break;
#endif
		default:
			// ストレージデバイス構成がエラー(範囲外)の場合は取得失敗(初期値)を返却する.
			break;
		}
	}
	return ll_result;
}

// ================================================================================================
//! <!--機能--> パーティション作成に必要なサイズ(byte)を計算する.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \part_layout* pst_PartInfo 選択されたパーティションテーブル.
//! <!--戻値-->
//! \return unsigned long long パーティション作成に必要なサイズ. 
//! <!--詳細-->
//!	パーティションテーブル作成に必要なサイズ(byte)を計算する.
//! <!--履歴-->
//!  2014/07/24 iguchi	新規作成.
//! ================================================================================================
static int64_t THRG_GetPartirionSize( part_layout* pst_PartInfo )
{	
	int64_t ll_PartSize = 0;

	// 引数チェック.
	THRL_ThAssert( pst_PartInfo != NULL," THRG_GetPartirionSize() Partition size get ERR! ", 0, 0, 0, 0, CFA14_DetailCode0x0CF7 );	

	// パーティション作成に必要な総セクタ数を計算する.
	for( short ss_CountNum = 0; pst_PartInfo[ss_CountNum].type != PART_TYPE_END; ss_CountNum++ ){
	// 論理パーティション以外の、基本パーティションのセクタ数のみを加算する.
		if( pst_PartInfo[ss_CountNum].type != PART_TYPE_LOGICAL ){
			ll_PartSize += pst_PartInfo[ss_CountNum].nsec;
		}
	}

	// パーティション作成に必要な総サイズを計算する.
	ll_PartSize *= ( int64_t )DEF_SECTOR_SIZE;

	return ll_PartSize;
}
	
// ================================================================================================
//! \\function THRG_GetPhysicaleMMCSize.
//! <!--------------------------------------------------------------------------------------------->
//! \\brief eMMCの物理サイズを取得する	
//! \\param	なし
//! 無し.
//! \\return long long
//! <!--履歴-->
//!  2016/05/19	稲垣 新規作成.
//!  2017/04/25 Y.Tanaka DLONG型修正の対応
//! ================================================================================================
int64_t THRG_GetPhysicaleMMCSize()
{	
	DLONG dl_GetSize = 0;
	int32_t i_DviceId = THRE_SsdPortNo;//デフォルト：μSD.
	char*	pc_strDeviceSSD = NULL;
	if(THRE_StartupDeviceType_SD == THRF_GetStartupDeviceType() ) {
		// SD.
#ifdef FUM_LINUX
#ifdef DEF_64BIT_PF
		pc_strDeviceSSD = "/dev/sdd";
#else
		pc_strDeviceSSD = "/dev/sdb";
#endif
#else /* #ifdef FUM_LINUX */ 
		// AR1060044 対応.
		pc_strDeviceSSD = "/dev/kmsda";
#endif /* #ifdef FUM_LINUX */ 
	}else{
		// SSD.
		i_DviceId = THRE_HddPortNo;
#ifdef FUM_LINUX
#ifdef DEF_64BIT_PF
		pc_strDeviceSSD = "/dev/sdc";
#else
		pc_strDeviceSSD = "/dev/sdb";
#endif
#else /* #ifdef FUM_LINUX */ 
		pc_strDeviceSSD = "/dev/kmsda";
#endif /* #ifdef FUM_LINUX */ 
	}
    // sysBspHdd.cpp内で使用しているデバイス配列.
    extern ST_HDD_DEVICE_FILE_INFO gst_HddDeviceFileInfo[];
#ifdef FUM_LINUX
	gst_HddDeviceFileInfo[1].puc_devName = pc_strDeviceSSD;
#else /* #ifdef FUM_LINUX */ 
	gst_HddDeviceFileInfo[1].puc_devName = pc_strDeviceSSD;
	gst_HddDeviceFileInfo[2].puc_devName = pc_strDeviceSSD;
#endif /* #ifdef FUM_LINUX */ 
	
 	ST_HDD_DEVICE_INFO st_DeviceInfo;
	memset( &st_DeviceInfo, 0, sizeof(st_DeviceInfo) );
	if( HDD_STATE_OK == THRF_sysBspHddDeviceInfoGet(i_DviceId , 0, &st_DeviceInfo )){	// ストレージデバイス情報を取得.
		dl_GetSize = (DLONG)st_DeviceInfo.iTotalBlocks * (DLONG)DEF_SECTOR_SIZE;	// 取得OK時のみストレージデバイスサイズを初期値として格納.
	}
	return static_cast<int64_t>(dl_GetSize);
}

// ================================================================================================
//! <!--機能--> eMMCのサイズに合わせたパーティションテーブルを選択する.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! 無し.
//! <!--戻値-->
//! \return part_layout* 選択されたパーティションテーブル.
//! <!--詳細-->
//!	装着されているeMMCのサイズに合わせてパーティションテーブルを選択する.
//! <!--履歴-->
//!  2014/07/24 iguchi	新規作成.
//!  2015/01/17 富田(日立JTE) A3/A4共通化FUM対応 HDDレスFAX対応 eMMC_16GB用の処理を追加.
//!  2017/04/25 Y.Tanaka DLONG型修正の対応
//!  2019/03/08 Sky合屋 [EagleH]【OP_BTS-3685】パーティションサイズが1TBを返す処理を無効化する.
//!  2019/04/02 Sky鳥居 [EagleH] Warning除去対応.
//!  2019/07/12 Sky     [RQ-560] 1TBSSD対応に伴い、【OP_BTS-3685】対応をロールバック.
//! ================================================================================================
static part_layout* THRG_GetPartitionType()
{	
	static Bool bl_GetPart = False; 			// パーティションテーブルが決定済みかを表す.
	static 	part_layout* ps_PartType = NULL;
	DLONG dl_GetSize = 0;
	int64_t ll_EmmcSize = 0;
#ifdef DEF_A3A4COMM
	int64_t ll_PartSize_16 = 0;// 2015/01/17 富田(日立JTE) A3/A4共通化FUM対応 HDDレスFAX対応.
#endif // DEF_A3A4COMM.
	// 使用するパーティションテーブルが決定済みなら、以降の処理は行わない.
	if( bl_GetPart == True ){
		return ps_PartType;
	}

	// パーティションテーブル決定済みに設定する.
	bl_GetPart = True;

	// 装着されているeMMCのサイズを取得.
	dl_GetSize = theHDDTrans.getPhysicalSubDeviceSize();
	ll_EmmcSize = (int64_t)dl_GetSize;
	// eMMCのサイズが7.15GBと4GBの場合の、パーティション作成に必要なサイズを計算.
#ifdef DEF_A3A4COMM
	ll_PartSize_16 = THRG_GetPartirionSize( sd_part_info_16GB );// 2015/01/17 富田(日立JTE) A3/A4共通化FUM対応 HDDレスFAX対応.
#endif // DEF_A3A4COMM.

	// 8GBのパーティション作成に必要なサイズを計算.
	int64_t ll_PartSize_8GB = THRG_GetPartirionSize( sd_part_info_8GB );
	// 256GBのパーティション作成に必要なサイズを計算.
	int64_t ll_PartSize_256GB = THRG_GetPartirionSize( ssd_part_info_256GB );
	// 1TBのパーティション作成に必要なサイズを計算.
	int64_t ll_PartSize_1TB = THRG_GetPartirionSize( ssd_part_info_1TB );

	// SSDのサイズが1TB用パーティション作成に必要なサイズ以上の場合.
	if( ll_EmmcSize >= ll_PartSize_1TB ){
		ps_PartType = ssd_part_info_1TB;
	//SSDのサイズが256GB用パーティション作成に必要なサイズ以上の場合.
	} else if( ll_EmmcSize >= ll_PartSize_256GB ){
		ps_PartType = ssd_part_info_256GB;
#ifdef DEF_A3A4COMM
	// 2015/01/17 富田(日立JTE) A3/A4共通化FUM対応 HDDレスFAX対応.
	// eMMCのサイズが16GB用パーティション作成に必要なサイズより大きい場合.
	} else if( ll_EmmcSize >= ll_PartSize_16 ){
		ps_PartType = sd_part_info_16GB;
#endif // DEF_A3A4COMM.
	//SSDのサイズが8GB用パーティション作成に必要なサイズ以上の場合.
	} else if( ll_EmmcSize >= ll_PartSize_8GB ){
		ps_PartType = sd_part_info_8GB;
	// SSDのサイズがパーティション作成に必要なサイズより小さい場合.
	}else{	
		THRL_ThAssert( False," THRG_GetPartitionType() eMMC size ERR! ", 0, 0, 0, 0, CFA14_DetailCode0x0CF6 );		
	}
	return ps_PartType;
}

/*================================================================================================
［機能］	パーティション情報の取得.
--------------------------------------------------------------------------------------------------
［引数］	char *device_path       ：[in] デバイスへのパス("/dev/kmsda"など).
            uint32_t &sector_start  ：[in/out] アクセスしようとしている開始セクタ.
            size_t size             ：[in] アクセスしようとしているセクタ数.
［戻値］	パーティションへのパス("/dev/kmsda10"など).
［詳細］	"/dev/kmsda" + オフセットを"/dev/kmsda#" + 調整後オフセットに変換する.
［履歴］	19/10/29 Sky      TAS000000001236対応.
================================================================================================*/
static char *THRL_GetPartitionPathFromOffset(char *device_path, uint32_t &sector_start, size_t size )
{
	Bool isFixed = False; // パーティションを特定できたらTrueにする.
	off_t sector_start_modified = sector_start; // 特定したパーティションでの調整後開始セクタ.
	char *partition_path = device_path; // パーティションへのパス.
	off_t part_manage = 0; // パーティション管理領域用の調整値

	if( NULL==device_path )
	{
		isFixed = True;
	}

	if( !isFixed )
	{
		if( strcmp(device_path, THRD_DEVFILE_SSD)!=0 )
		{
			// "/dev/kmsda"以外は対象外.
			isFixed = True;
		}
	}

	if( !isFixed )
	{
#ifdef SIMULATOR
		part_manage = THRD_PartitionManageSize_SIM;
#else
		TYPE_StorageConfig uc_StorageType= THRG_GetStorageType();
		switch( uc_StorageType ){
		case TYPE_StorageConfig_SSD_256GB:
		case TYPE_StorageConfig_SSD_1TB:
			part_manage = THRD_PartitionManageSize_SSD;
			break;
		case TYPE_StorageConfig_MicroSD_8GB:
		case TYPE_StorageConfig_MicroSD_16GB:
			part_manage = THRD_PartitionManageSize_MicroSD;
			break;
		default:
			printf("[THRL_GetPartitionPathFromOffset] Unknown storage type:%d\n", uc_StorageType);
			isFixed = True;
			break;
		}
#endif	/* #if defined(SIMULATOR) */
	}

	if( !isFixed )
	{
		part_layout* ps_PartLayout = THRG_GetPartitionType();

		off_t range_start = 0;

		int32_t no = 0;
		do
		{
			off_t range_end=0;
			switch( ps_PartLayout[no].type )
			{
			case PART_TYPE_PRIMARY:
			case PART_TYPE_LOGICAL:
				range_end = range_start + ps_PartLayout[no].nsec - 1;

				if( sector_start<=range_end )
				{
					off_t manage_size = 0;
					if( 0==no )
					{
						manage_size = part_manage;
					}

					if( (range_start+manage_size<=sector_start) && (sector_start+(off_t)size-1<=range_end) )
					{
						// パーティション指定のパスと調整後セクタを取得.
						partition_path = ps_PartLayout[no].devpath[0];  // 暗号化あり時も、直接転送は"/dev/mapper/crypt_##"を使用しない.
						sector_start_modified = sector_start - (range_start+manage_size);
						isFixed = True;
					}
					else
					{
						// 該当領域で範囲超過していた.
						printf("[THRL_GetPartitionPathFromOffset] Partition range check NG(\"%s\" %d %d). \n", device_path, sector_start, size);
						partition_path = NULL;
						sector_start_modified = 0;
						isFixed = True;
					}
				}
				break;

			case PART_TYPE_EXTENDED:
				// 拡張領域は管理領域分だけ移動して抜ける.
				range_end = range_start + part_manage - 1;
				break;

			case PART_TYPE_END:
			default:
				// テーブルの最後まで該当領域が見つからなかった.
				printf("[THRL_GetPartitionPathFromOffset] Partition not found(\"%s\" %d %d). \n", device_path, sector_start, size);
				partition_path = NULL;
				sector_start_modified = 0;
				isFixed = True;
				break;
			}

			range_start = range_end+1;
			no++;
		}while( !isFixed );
	}

	if( NULL==partition_path )
	{
		uint64_t ull_NewDetailData = THRD_NewDetail_ModuleId_Thread; // 1,2桁：スレッド.
		ull_NewDetailData |= (uint64_t)( CFA14_DetailCode0x01C6 ) << THRD_NewDetail_TroubleCode_Shift; // 3-6桁：トラブルコード.
		ull_NewDetailData |= (uint64_t)( sector_start&0x00FFFFFF ) << 16; // 7-12桁：開始セクタ.
		ull_NewDetailData |= (uint64_t)( size&0x0000FFFF ); // 13-16桁：セクタ数.
		THRL_ThAssertLog2( 0, ull_NewDetailData ); 
	}
	sector_start = sector_start_modified;
	return partition_path;
}


// ================================================================================================
//! <!--機能--> HDD装着状況を取得する.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! THRE_HddMirroring_Chanel e_Chanel : チャネル情報.
//! <!--戻値-->
//! \return Bool 装着状態(True:接続 / False:未接続).
//! <!--詳細-->
//!	引数のチャネルのHDD装着状況を返す.
//! <!--履歴-->
//	2014/09/08 yamada	ZeusS 制御構成変更No.4 HDDミラーリング対応.
//! ================================================================================================
Bool CP_HDDTrans::getMirroringHddDevice(THRE_HddMirroring_Chanel e_Chanel)
{
	Bool bl_DevConnect = False;

	switch(e_Chanel) {
	case THRE_HddMirroring_Chanel_A:
		// 未接続以外の場合は戻り値にTrueを設定する.
		if (m_e_HddMirroringDeviceStatus[THRE_HddMirroring_Chanel_A] != THRE_HddMirroringStatus_Non) {
			bl_DevConnect = True;
		}
		break;
	case THRE_HddMirroring_Chanel_B:
		// 未接続以外の場合は戻り値にTrueを設定する.
		if (m_e_HddMirroringDeviceStatus[THRE_HddMirroring_Chanel_B] != THRE_HddMirroringStatus_Non) {
			bl_DevConnect = True;
		}
		break;
	default:
		setDebugLowLogSpeedSeq(710, e_Chanel) ;
		break;
	}

	return bl_DevConnect;
}

// ================================================================================================
//! <!--機能--> HDD動作状況を取得する.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! THRE_HddMirroring_Chanel e_Chanel : チャネル情報.
//! <!--戻値-->
//! \return THRE_HddMirroringStatus 動作状態.
//! 								THRE_HddMirroringStatus_Non		: 未接続.
//! 								THRE_HddMirroringStatus_Normal	: 正常.
//! 								THRE_HddMirroringStatus_Trouble	: 故障中.
//! 								THRE_HddMirroringStatus_Rebuild	: リビルド中.
//! <!--詳細-->
//!	引数のチャネルのHDD動作状況を返す.
//! <!--履歴-->
//	2014/09/08 yamada	ZeusS 制御構成変更No.4 HDDミラーリング対応.
//! ================================================================================================
THRE_HddMirroringStatus CP_HDDTrans::getMirroringHddStatus(THRE_HddMirroring_Chanel e_Chanel)
{
	THRE_HddMirroringStatus e_DevStatus = THRE_HddMirroringStatus_Non;

	// 指定チャネルに対応したステータスを設定する.
	switch(e_Chanel) {
	case THRE_HddMirroring_Chanel_A:
		e_DevStatus = m_e_HddMirroringDeviceStatus[THRE_HddMirroring_Chanel_A];
		break;
	case THRE_HddMirroring_Chanel_B:
		e_DevStatus = m_e_HddMirroringDeviceStatus[THRE_HddMirroring_Chanel_B];
		break;
	default:
		setDebugLowLogSpeedSeq(715, e_Chanel) ;
		break;
	}

	return e_DevStatus;
}

// ================================================================================================
//! <!--機能--> RAIDマスターHDDのチャネルIDを取得する.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! なし.
//! <!--戻値-->
//! \return THRE_HddMirroring_Chanel マスターチャネル.
//! 									THRE_HddMirroring_Chanel_A : Ch-A.
//! 									THRE_HddMirroring_Chanel_B : Ch-B.
//! <!--詳細-->
//!	起動時に初期化した、前回起動時のマスターチャネルを返す.
//! <!--履歴-->
//	2014/09/08 yamada	ZeusS 制御構成変更No.4 HDDミラーリング対応.
//! ================================================================================================
THRE_HddMirroring_Chanel CP_HDDTrans::getMirroringMasterChanel()
{
	return m_e_HddMirroringMasterChanel;
}

// ================================================================================================
//! <!--機能--> ミラーリング設定の取得.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! なし.
//! <!--戻値-->
//! \return Bool 現在のミラーリング設定.
//! 				True  : ミラーリング有効.
//! 				False : ミラーリング無効.
//! <!--詳細-->
//!	ミラーリング設定の取得.
//! <!--履歴-->
//	2014/09/08 yamada	ZeusS 制御構成変更No.4 HDDミラーリング対応.
//! ================================================================================================
Bool CP_HDDTrans::isMirroringHDD()
{
	return m_c_MirroringBackupData.isMirroringOn();
}

// ================================================================================================
//! <!--機能--> リビルド開始.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! なし.
//! <!--戻値-->
//! \return Bool リビルド開始結果.
//! 				True  : リビルド開始に成功.
//! 				False : リビルド開始に失敗.
//! <!--詳細-->
//!	前提としてミラーリング機能が有効である事.
//! <!--履歴-->
//	2014/09/08 yamada		ZeusS 制御構成変更No.4 HDDミラーリング対応.
//	2014/11/07 y.morikawa	CQ504509 リビルド中断問題の対応.
//! ================================================================================================
Bool CP_HDDTrans::RebuildStart()
{
	// ミラーリング有効判定.
	if (isMirroringHDD() == False) {
		// ミラーリング無効の場合は成功を返す.
		setDebugLowLogSpeedSeq(720, 0) ;
		return True;
	}

	// リビルド実行中判定.
	if (m_bl_RebuildRunStatus == True) {
		// リビルド実行中は成功を返す.
		setDebugLowLogSpeedSeq(721, 0) ;
		return True;
	}

	// 現在のマスターチャネルに応じたHDDデバイスを取得する.
	THRE_HddMirroring_Chanel e_MasterChanel = getMirroringMasterChanel();
	uchar uc_DevNo = THRE_SsdPortNo;
	TYPE_HddChanelNo e_ChanelNo = TYPE_HddChanelNo_Unknown;
	THRE_HddMirroring_Chanel e_SlaveChanel  = 0;

	switch(e_MasterChanel) {
	case THRE_HddMirroring_Chanel_A:
		uc_DevNo   = THRE_HddPortNo;
		e_ChanelNo = TYPE_HddChanelNo_Chanel_A;
		e_SlaveChanel = TYPE_HddChanelNo_Chanel_B;
		break;
	case THRE_HddMirroring_Chanel_B:
		uc_DevNo   = THRE_HddChBPortNo;
		e_ChanelNo = TYPE_HddChanelNo_Chanel_B;
		e_SlaveChanel = TYPE_HddChanelNo_Chanel_A;
		break;
	default:
		setDebugLowLogSpeedSeq(722, e_MasterChanel) ;
		return False;
		break;
	}

	// リビルド開始.
	uint32_t ul_Ret = THRF_sysBspStartRebuild(uc_DevNo, 0);
	if (ul_Ret != HDD_STATE_OK) {
		setDebugLowLogSpeedSeq(723, 0) ;
		return False;
	}

	// リビルド状態を実行中に設定.
	m_bl_RebuildRunStatus = True;

	// リビルド進捗率を0%に設定.
	m_uc_ProgressRatio = 0;

	// スレーブのHDDステータスをリビルド中に設定.
	m_e_HddMirroringDeviceStatus[e_SlaveChanel] = THRE_HddMirroringStatus_Rebuild;

	// 電源制御へHDDアクセス終了を通知.
	APIC_Power* pc_ApiPower = APIC_Power::newInstance();
	if (pc_ApiPower == NULL) {
		setDebugLowLogSpeedSeq(724, 0) ;
		return False;
	}
	pc_ApiPower->changePowerMode(TYPE_PowerModeDataAccessStart);

	// リビルド開始を上位へ通知.
	APIC_HddInfo* pc_HddInfo = APIC_HddInfo::newInstance();
	if (pc_HddInfo == NULL) {
		setDebugLowLogSpeedSeq(724, 1) ;
		return False;
	}

	setDebugLowLogSpeedSeq(725, 0) ;

	TYPS_HDDMirroringEventInfo st_EventInfo;
	st_EventInfo.e_Event    = TYPE_HDDMirroringEvent_Rebuild;	// リビルド中.
	st_EventInfo.e_ChanelNo = e_ChanelNo;						// マスターチャネル.

	pc_HddInfo->notifyHDDMirroringEvent(st_EventInfo);

	return True;
}

// ================================================================================================
//! <!--機能--> リビルド停止.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! なし.
//! <!--戻値-->
//! \return Bool リビルド停止結果.
//! 				True  : リビルド停止に成功.
//! 				False : リビルド停止に失敗.
//! <!--詳細-->
//!	前提としてミラーリング機能が有効である事.
//! <!--履歴-->
//	2014/09/08 yamada	ZeusS 制御構成変更No.4 HDDミラーリング対応.
//	2014/11/07 y.morikawa	CQ504509 リビルド中断問題の対応.
//! ================================================================================================
Bool CP_HDDTrans::RebuildStop()
{
	// ミラーリング有効判定.
	if (isMirroringHDD() == False) {
		// ミラーリング無効の場合は成功を返す.
		setDebugLowLogSpeedSeq(730, 0) ;
		return True;
	}

	// リビルド実行中判定.
	if (m_bl_RebuildRunStatus == False) {
		// リビルド停止中は成功を返す.
		setDebugLowLogSpeedSeq(731, 0) ;
		return True;
	}

	// 現在のマスターチャネルに応じたHDDデバイスを取得する.
	THRE_HddMirroring_Chanel e_MasterChanel = getMirroringMasterChanel();
	uchar uc_DevNo = THRE_SsdPortNo;
	TYPE_HddChanelNo e_ChanelNo = TYPE_HddChanelNo_Unknown;

	switch(e_MasterChanel) {
	case THRE_HddMirroring_Chanel_A:
		uc_DevNo   = THRE_HddPortNo;
		e_ChanelNo = TYPE_HddChanelNo_Chanel_A;
		break;
	case THRE_HddMirroring_Chanel_B:
		uc_DevNo   = THRE_HddChBPortNo;
		e_ChanelNo = TYPE_HddChanelNo_Chanel_B;
		break;
	default:
		setDebugLowLogSpeedSeq(732, e_MasterChanel) ;
		return False;
		break;
	}

	// リビルド停止.
	uint32_t ul_Ret = THRF_sysBspStopRebuild(uc_DevNo);
	if (ul_Ret != HDD_STATE_OK) {
		setDebugLowLogSpeedSeq(733, 0) ;
		return False;
	}

	// リビルド状態を停止中に設定.
	m_bl_RebuildRunStatus = False;

	// 電源制御へHDDアクセス終了を通知.
	APIC_Power* pc_ApiPower = APIC_Power::newInstance();
	if (pc_ApiPower == NULL) {
		setDebugLowLogSpeedSeq(734, 0) ;
		return False;
	}
	pc_ApiPower->changePowerMode(TYPE_PowerModeDataAccessEnd);

	// リビルド停止を上位へ通知.
	APIC_HddInfo* pc_HddInfo = APIC_HddInfo::newInstance();
	if (pc_HddInfo == NULL) {
		setDebugLowLogSpeedSeq(734, 1) ;
		return False;
	}

	setDebugLowLogSpeedSeq(735, 0) ;

	TYPS_HDDMirroringEventInfo st_EventInfo;
	st_EventInfo.e_Event    = TYPE_HDDMirroringEvent_RebuildStop;	// リビルド中止.
	st_EventInfo.e_ChanelNo = e_ChanelNo;							// マスターチャネル.

	pc_HddInfo->notifyHDDMirroringEvent(st_EventInfo);

	// リビルド状態を停止中に設定.
	m_bl_RebuildRunStatus = False;

	return True;
}

// ================================================================================================
//! <!--機能--> リビルド再開.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! なし.
//! <!--戻値-->
//! \return Bool リビルド再開結果.
//! 				True  : リビルド再開に成功.
//! 				False : リビルド再開に失敗.
//! <!--詳細-->
//!	前提としてミラーリング機能が有効である事.
//! <!--履歴-->
//	2014/09/08 yamada		ZeusS 制御構成変更No.4 HDDミラーリング対応.
//	2014/11/08 y.morikawa	CQ504509 リビルド中断問題の対応.
//	2015/01/21 y.morikawa	CQ507487 リビルド中の電源断問題の対応.
//	2015/03/10 fukuhara		CQ510790 ZeusS【管理No.425】【制御_No.4】リビルド処理再開しない対応.
//! ================================================================================================
Bool CP_HDDTrans::RebuildRestart()
{
	// ミラーリング有効判定.
	if (isMirroringHDD() == False) {
		// ミラーリング無効の場合は成功を返す.
		setDebugLowLogSpeedSeq(740, 0) ;
		return True;
	}

	// リビルド実行中判定.
	if (m_bl_RebuildRunStatus == True) {
		// リビルド実行中は成功を返す.
		setDebugLowLogSpeedSeq(741, 0) ;
		return True;
	}

	// CH-A/CH-Bの両方とも｛正常または再構築中｝でなければリビルドを再開しない.
	if(!(((m_e_HddMirroringDeviceStatus[THRE_HddMirroring_Chanel_A] == THRE_HddMirroringStatus_Normal) ||	// 正常.
		(m_e_HddMirroringDeviceStatus[THRE_HddMirroring_Chanel_A] == THRE_HddMirroringStatus_Rebuild)) &&	// 再構築中.

		((m_e_HddMirroringDeviceStatus[THRE_HddMirroring_Chanel_B] == THRE_HddMirroringStatus_Normal) ||	// 正常.
		(m_e_HddMirroringDeviceStatus[THRE_HddMirroring_Chanel_B] == THRE_HddMirroringStatus_Rebuild)))){	// 再構築中.

		setDebugLowLogSpeedSeq(741, 1) ;
		return True;
	}

	// リビルド進捗率が100%のときはリビルドを再開しない.
	if(m_uc_ProgressRatio == 100) {
		setDebugLowLogSpeedSeq(741, 2) ;
		return True;
	}

	// 現在のマスターチャネルに応じたHDDデバイスを取得する.
	THRE_HddMirroring_Chanel e_MasterChanel = getMirroringMasterChanel();
	uchar uc_DevNo = THRE_SsdPortNo;
	TYPE_HddChanelNo e_ChanelNo = TYPE_HddChanelNo_Unknown;
	TYPE_HddChanelNo e_SlaveChanel = TYPE_HddChanelNo_Unknown;

	switch(e_MasterChanel) {
	case THRE_HddMirroring_Chanel_A:
		uc_DevNo   = THRE_HddPortNo;
		e_ChanelNo = TYPE_HddChanelNo_Chanel_A;
		e_SlaveChanel = TYPE_HddChanelNo_Chanel_B;
		break;
	case THRE_HddMirroring_Chanel_B:
		uc_DevNo   = THRE_HddChBPortNo;
		e_ChanelNo = TYPE_HddChanelNo_Chanel_B;
		e_SlaveChanel = TYPE_HddChanelNo_Chanel_A;
		break;
	default:
		setDebugLowLogSpeedSeq(742, e_MasterChanel) ;
		return False;
		break;
	}

	// リビルド再開.
	uint32_t ul_Ret = THRF_sysBspStartRebuild(uc_DevNo, 1);
	if (ul_Ret != HDD_STATE_OK) {
		setDebugLowLogSpeedSeq(743, 0) ;
		return False;
	}

	// リビルド状態を実行中に設定.
	m_bl_RebuildRunStatus = True;

	// スレーブのHDDステータスをリビルド中に設定.
	m_e_HddMirroringDeviceStatus[e_SlaveChanel] = THRE_HddMirroringStatus_Rebuild;

	// 電源制御へHDDアクセス開始を通知.
	APIC_Power* pc_ApiPower = APIC_Power::newInstance();
	if (pc_ApiPower == NULL) {
		setDebugLowLogSpeedSeq(744, 0) ;
		return False;
	}
	pc_ApiPower->changePowerMode(TYPE_PowerModeDataAccessStart);
	// リビルド開始を上位へ通知.
	APIC_HddInfo* pc_HddInfo = APIC_HddInfo::newInstance();
	if (pc_HddInfo == NULL) {
		setDebugLowLogSpeedSeq(744, 1) ;
		return False;
	}

	setDebugLowLogSpeedSeq(745, 0) ;

	TYPS_HDDMirroringEventInfo st_EventInfo;
	st_EventInfo.e_Event    = TYPE_HDDMirroringEvent_Rebuild;	// リビルド中.
	st_EventInfo.e_ChanelNo = e_ChanelNo;						// マスターチャネル.

	pc_HddInfo->notifyHDDMirroringEvent(st_EventInfo);

	return True;
}

// ================================================================================================
//! <!--機能--> リビルド再開(電源断).
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! なし.
//! <!--戻値-->
//! \return Bool リビルド再開結果.
//! 				True  : リビルド再開に成功.
//! 				False : リビルド再開に失敗.
//! <!--詳細-->
//!	前提としてミラーリング機能が有効である事.
//!	電源制御へHDDアクセス開始通知はRAID構成情報の取得時に通知済みのため本関数では呼ばない.
//! <!--履歴-->
//	2015/01/21 y.morikawa	CQ507487 リビルド中の電源断問題の対応.
//! ================================================================================================
Bool CP_HDDTrans::RebuildRecoveryStart()
{
	// ミラーリング有効判定.
	if (isMirroringHDD() == False) {
		// ミラーリング無効の場合は成功を返す.
		setDebugLowLogSpeedSeq(855, 0) ;
		return True;
	}

	// リビルド実行中判定.
	if (m_bl_RebuildRunStatus == True) {
		// リビルド実行中は成功を返す.
		return True;
	}

	// 現在のマスターチャネルに応じたHDDデバイスを取得する.
	THRE_HddMirroring_Chanel e_MasterChanel = getMirroringMasterChanel();
	uchar uc_DevNo = THRE_SsdPortNo;
	TYPE_HddChanelNo e_ChanelNo = TYPE_HddChanelNo_Unknown;
	TYPE_HddChanelNo e_SlaveChanel = TYPE_HddChanelNo_Unknown;

	switch(e_MasterChanel) {
	case THRE_HddMirroring_Chanel_A:
		uc_DevNo   = THRE_HddPortNo;
		e_ChanelNo = TYPE_HddChanelNo_Chanel_A;
		e_SlaveChanel = TYPE_HddChanelNo_Chanel_B;
		break;
	case THRE_HddMirroring_Chanel_B:
		uc_DevNo   = THRE_HddChBPortNo;
		e_ChanelNo = TYPE_HddChanelNo_Chanel_B;
		e_SlaveChanel = TYPE_HddChanelNo_Chanel_A;
		break;
	default:
		setDebugLowLogSpeedSeq(856, e_MasterChanel) ;
		return False;
		break;
	}

	// リビルド再開.
	uint32_t ul_Ret = THRF_sysBspStartRebuild(uc_DevNo, 1);
	if (ul_Ret != HDD_STATE_OK) {
		setDebugLowLogSpeedSeq(857, 0) ;
		return False;
	}

	// リビルド状態を実行中に設定.
	m_bl_RebuildRunStatus = True;

	// スレーブのHDDステータスをリビルド中に設定.
	m_e_HddMirroringDeviceStatus[e_SlaveChanel] = THRE_HddMirroringStatus_Rebuild;

	// リビルド開始を上位へ通知.
	APIC_HddInfo* pc_HddInfo = APIC_HddInfo::newInstance();
	if (pc_HddInfo == NULL) {
		setDebugLowLogSpeedSeq(858, 0) ;
		return False;
	}

	setDebugLowLogSpeedSeq(859, 0) ;

	TYPS_HDDMirroringEventInfo st_EventInfo;
	st_EventInfo.e_Event    = TYPE_HDDMirroringEvent_Rebuild;	// リビルド中.
	st_EventInfo.e_ChanelNo = e_ChanelNo;						// マスターチャネル.

	pc_HddInfo->notifyHDDMirroringEvent(st_EventInfo);

	return True;
}

// ================================================================================================
//! <!--機能--> リビルド状態(進捗)取得.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! なし.
//! <!--戻値-->
//! \return uchar 進捗率[%] : 0 ～ 100.
//! <!--詳細-->
//!	前提としてミラーリング機能が有効である事\n.
//!	リビルド中でないときは100を返す.
//! <!--履歴-->
//	2014/09/08 yamada	ZeusS 制御構成変更No.4 HDDミラーリング対応.
//! ================================================================================================
uchar CP_HDDTrans::getRebuildProgress()
{
	return m_uc_ProgressRatio;
}

// ================================================================================================
//! <!--機能--> リビルド中取得.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! なし.
//! <!--戻値-->
//! \return Bool リビルド動作状態.
//! 				True  : リビルド中.
//! 				False : リビルド停止中.
//! <!--詳細-->
//!	前提としてミラーリング機能が有効である事.
//! <!--履歴-->
//	2014/09/08 yamada	ZeusS 制御構成変更No.4 HDDミラーリング対応.
//	2015/01/21 y.morikawa	CQ507487 リビルド中の電源断問題の対応.
//	2015/03/10 fukuhara		CQ510790 ZeusS【管理No.425】【制御_No.4】リビルド処理再開しない対応.
//! ================================================================================================
Bool CP_HDDTrans::getRebuildActive()
{
	return m_bl_RebuildRunStatus;
}

// ================================================================================================
//! <!--機能--> ミラーリング機能 設定変更.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param なし.
//! <!--戻値-->
//! \return Bool 設定変更結果.
//! 				True  : 成功.
//! 				False : 失敗.
//! <!--詳細-->
//!	ミラーリング機能の有効/無効を切り替える.
//!	無効の要求によってCHANGE_RAID_MODEのコマンドを発行し、RAIDを解除する.
//! <!--履歴-->
//	2014/09/15 y.morikawa	ZeusS 制御構成変更No.4 HDDミラーリング対応.
//	2014/11/07 y.morikawa	ZeusS 制御構成変更No.4 HDDミラーリング対応 SATAコントローラがHDD構成の変更不可のため削除.
//! ================================================================================================
void CP_HDDTrans::setRebuildActive()
{
	m_c_MirroringBackupData.setMirroringOn( False );	// ミラーリング機能を無効に設定変更する.

	setDebugLowLogSpeedSeq(810, 0) ;
}


// ================================================================================================
//! <!--機能--> リビルド状態更新関数.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! なし.
//! <!--戻値-->
//! なし.
//! <!--詳細-->
//!	前提としてミラーリング機能が有効である事.
//!	RAIDの状態を取得し、リビルドの開始／終了を判断する.
//!	リビルド中であれば進捗率を更新する.
//!	リビルドを終了した時はリビルドエラーを判定する.
//! <!--履歴-->
//	2014/09/08 yamada		ZeusS 制御構成変更No.4 HDDミラーリング対応.
//	2014/09/15 y.morikawa	ZeusS 制御構成変更No.4 HDDミラーリング対応(不揮発データ対応を追加).
//	2014/10/14 y.morikawa	CQ504382,CQ504646 HDDミラーリングエラー通知の問題対応.
//	2014/11/07 y.morikawa	CQ504509 リビルド中断問題の対応.
//	2015/01/21 y.morikawa	CQ507487 リビルド中の電源断問題の対応.
//! ================================================================================================
Bool CP_HDDTrans::RebuildStatusUpdate()
{
	// ミラーリング有効判定.
	if (isMirroringHDD() == False) {
		return True;
	}

	// HDDステータスを取得.
	IMGC_ImageMemoryPoolImpColorFileMemoryInstance& comMemoryManager = IMGC_ImageMemoryPoolImpColorFileMemorySingletonIf::newInstance();
	TYPE_HddStatus e_HddStatus = comMemoryManager.getHDDStatus();

	// HDDが使用可能かチェック.
	if (e_HddStatus != TYPE_HddStatusFormated) {
		// 使用可能状態になるまでは何もしない.
		setDebugLowLogSpeedSeq(761, e_HddStatus) ;
		return True;
	}

	// 上位APIのインスタンス生成.
	APIC_HddInfo* pc_HddInfo = APIC_HddInfo::newInstance();
	if (pc_HddInfo == NULL) {
		setDebugLowLogSpeedSeq(762, 0) ;
		return False;
	}

	// 現在のマスターチャネルに応じたHDDデバイス情報を取得する.
	THRE_HddMirroring_Chanel e_MasterChanel = getMirroringMasterChanel();
	uchar uc_DevNo = THRE_SsdPortNo;
	TYPE_HddChanelNo e_MasterChanelNo = TYPE_HddChanelNo_Unknown;
	TYPE_HddChanelNo e_SlaveChanelNo  = TYPE_HddChanelNo_Unknown;
	THRE_HddMirroring_Chanel e_MasterMirroringChanel = 0;
	THRE_HddMirroring_Chanel e_SlaveMirroringChanel  = 0;

	switch(e_MasterChanel) {
	case THRE_HddMirroring_Chanel_A:
		uc_DevNo = THRE_HddPortNo;
		e_MasterChanelNo = TYPE_HddChanelNo_Chanel_A;
		e_SlaveChanelNo  = TYPE_HddChanelNo_Chanel_B;
		e_MasterMirroringChanel = THRE_HddMirroring_Chanel_A;
		e_SlaveMirroringChanel  = THRE_HddMirroring_Chanel_B;
		break;
	case THRE_HddMirroring_Chanel_B:
		uc_DevNo = THRE_HddChBPortNo;
		e_MasterChanelNo = TYPE_HddChanelNo_Chanel_B;
		e_SlaveChanelNo  = TYPE_HddChanelNo_Chanel_A;
		e_MasterMirroringChanel = THRE_HddMirroring_Chanel_B;
		e_SlaveMirroringChanel  = THRE_HddMirroring_Chanel_A;
		break;
	default:
		setDebugLowLogSpeedSeq(763, e_MasterChanel) ;
		return False;
		break;
	}
	// リビルド動作状態取得.
	THRS_HDD_DEVICE_RBLD_INFO st_RebuildInfo = {0};
	uint32_t ul_Ret = THRF_sysBspGetRebuildInfo(uc_DevNo, &st_RebuildInfo);
	if (ul_Ret != HDD_STATE_OK) {
		setDebugLowLogSpeedSeq(764, 0) ;
		return False;
	}

	// リビルド動作状態判定.
	if (m_bl_RebuildRunStatus == True) {
		// 前回リビルド実行中.
		if (st_RebuildInfo.uc_RebuildRunning == 1) {
			// 現在リビルド実行中.
			// 進捗率更新.
			// リビルド実行中LBAアドレス / 全体容量で計算.
			// 2017/02/20 Sky DLONG対応.
			uint64_t ull_HddSize = getPhysicalHDDSize();
			
			uint64_t ull_LBAAddress = (uint64_t)st_RebuildInfo.us_RebuildLBAAddressH << 32 |
											   (uint64_t)st_RebuildInfo.us_RebuildLBAAddressM << 16 |
											   (uint64_t)st_RebuildInfo.us_RebuildLBAAddressL;

			m_uc_ProgressRatio = (uchar)( (ull_LBAAddress * 100) / ( ull_HddSize / DEF_SECTOR_SIZE ));
			// 進捗率 99%を超える場合は、99%で止めておく(リビルド完了時に100%にする).
			if (m_uc_ProgressRatio > 99) {
				m_uc_ProgressRatio = 99;
			}

		} else {
			// 現在リビルド停止中.
			setDebugLowLogSpeedSeq(760, st_RebuildInfo.uc_RebuildResult) ;

			// リビルド結果チェック.
			if (st_RebuildInfo.uc_RebuildResult == 0) {
				// 正常終了.
				// 進捗率を100%に更新.
				m_uc_ProgressRatio = 100;

				// ミラーリングのエラーを解除.
				m_c_MirroringBackupData.clrMirroringBackupDataError();

				// 各チャネルのステータスを正常に設定.
				m_e_HddMirroringDeviceStatus[THRE_HddMirroring_Chanel_A] = THRE_HddMirroringStatus_Normal;
				m_e_HddMirroringDeviceStatus[THRE_HddMirroring_Chanel_B] = THRE_HddMirroringStatus_Normal;

				// リビルド完了通知.
				TYPS_HDDMirroringEventInfo st_EventInfo;
				st_EventInfo.e_Event    = TYPE_HDDMirroringEvent_RebuildComplete;	// リビルド完了.
				st_EventInfo.e_ChanelNo = e_MasterChanelNo;							// マスターチャネル.

				pc_HddInfo->notifyHDDMirroringEvent(st_EventInfo);
			} else if ( (st_RebuildInfo.uc_RebuildResult == 2) ||
						(st_RebuildInfo.uc_RebuildResult == 5) ){
				// マスター側エラー有完了 or マスタ側ディスク初期化エラー終了.
				// マスター側エラー検出済チェック.
				THRS_MIRRORING_BACKUP_DATA st_BackupData;
				m_c_MirroringBackupData.getMirroringBackupData(st_BackupData);	// 不揮発データの取得.

				if (m_c_MirroringBackupData.isActiveError(THRE_HDD_MIRRORING_REBUILD_MASTER_ERROR) == False) {
					// 未検出.
					// マスター側エラー通知.
					TYPS_HDDMirroringEventInfo st_EventInfo;
					st_EventInfo.e_Event    = TYPE_HDDMirroringEvent_RebuildFailure;	// リビルド失敗.
					st_EventInfo.e_ChanelNo = e_MasterChanelNo;							// マスターチャネル.

					pc_HddInfo->notifyHDDMirroringEvent(st_EventInfo);

					// マスター側エラー検出済設定.
					m_c_MirroringBackupData.setMirroringBackupDataErrorBit (THRE_HDD_MIRRORING_REBUILD_MASTER_ERROR, True );

					// マスターのHDDステータスを故障中に更新.
					m_e_HddMirroringDeviceStatus[e_MasterMirroringChanel] = THRE_HddMirroringStatus_Trouble;

					// マスターのエラーは再起動でチャネルが変わるため、現時点のマスターチャネルを記録しておく.
					m_c_MirroringBackupData.setMirroringBackupDataMasterCh( m_e_HddMirroringMasterChanel );

					setDebugLowLogSpeedSeq(769, st_RebuildInfo.uc_RebuildResult) ;
					setDebugLowLogSpeedSeq(770, m_e_HddMirroringMasterChanel) ;

					// スレーブチャネルをマスターチャネルに切り替える.
					ul_Ret = THRF_sysBspChangeMasterChanel(uc_DevNo, e_SlaveMirroringChanel);
					if (ul_Ret != HDD_STATE_OK) {
						setDebugLowLogSpeedSeq(766, 0) ;
					}

					sendStatusNotice( TYPD_TroubleCode_HDD_REBUILD_MASTER_ERROR );		// HDDミラーリングエラー マスター（リビルド）.
				}
			} else if ( (st_RebuildInfo.uc_RebuildResult == 1) ||
						(st_RebuildInfo.uc_RebuildResult == 3) ||
						(st_RebuildInfo.uc_RebuildResult == 4) ||
						(st_RebuildInfo.uc_RebuildResult == 7) ){
				// バックアップ側エラー有完了 or バックアップ側エラー終了 or バックアップ側ディスク初期化エラー終了 or リビルド開始時エラー発生.
				THRS_MIRRORING_BACKUP_DATA st_BackupData;
				m_c_MirroringBackupData.getMirroringBackupData(st_BackupData);	// 不揮発データの取得.

				// スレーブ側エラー検出済チェック.
				if (m_c_MirroringBackupData.isActiveError(THRE_HDD_MIRRORING_REBUILD_SLAVE_ERROR) == False) {
					// 未検出.
					// スレーブ側エラー通知.
					TYPS_HDDMirroringEventInfo st_EventInfo;
					st_EventInfo.e_Event    = TYPE_HDDMirroringEvent_RebuildFailure;	// リビルド失敗.
					st_EventInfo.e_ChanelNo = e_SlaveChanelNo;							// スレーブチャネル.

					pc_HddInfo->notifyHDDMirroringEvent(st_EventInfo);

					// スレーブ側エラー検出済設定.
					m_c_MirroringBackupData.setMirroringBackupDataErrorBit (THRE_HDD_MIRRORING_REBUILD_SLAVE_ERROR, True );

					// スレーブのHDDステータスを故障中に更新.
					m_e_HddMirroringDeviceStatus[e_SlaveMirroringChanel] = THRE_HddMirroringStatus_Trouble;

					setDebugLowLogSpeedSeq(771, st_RebuildInfo.uc_RebuildResult) ;

					sendStatusNotice( TYPD_TroubleCode_HDD_REBUILD_SLAVE_ERROR );		// HDDミラーリングエラー スレーブ（リビルド）.
				}
			} else {
				// STOP_RBLDコマンドによる終了 or その他.
				// リビルド中止通知.
				TYPS_HDDMirroringEventInfo st_EventInfo;
				st_EventInfo.e_Event    = TYPE_HDDMirroringEvent_RebuildStop;	// リビルド中止.
				st_EventInfo.e_ChanelNo = e_MasterChanelNo;						// マスターチャネル.

				setDebugLowLogSpeedSeq(772, st_RebuildInfo.uc_RebuildResult) ;

				pc_HddInfo->notifyHDDMirroringEvent(st_EventInfo);
			}
			// リビルド動作状態を更新.
			m_bl_RebuildRunStatus = False;

			// 電源制御へHDDアクセス終了を通知.
			APIC_Power* pc_ApiPower = APIC_Power::newInstance();
			if (pc_ApiPower == NULL) {
				setDebugLowLogSpeedSeq(770, 0) ;
				return False;
			}
			pc_ApiPower->changePowerMode(TYPE_PowerModeDataAccessEnd);
		}
	} else {
		// リビルド停止中.
		chkMirroringCurrentTroubleNotify();							// トラブルの発生を確認する.

		// 前回リビルド停止中.
		if (st_RebuildInfo.uc_RebuildRunning == 1) {
			if((m_e_HddMirroringDeviceStatus[THRE_HddMirroring_Chanel_A] == THRE_HddMirroringStatus_Normal) &&		// 正常.
			   (m_e_HddMirroringDeviceStatus[THRE_HddMirroring_Chanel_B] == THRE_HddMirroringStatus_Normal)){		// 正常.
				if(m_uc_ProgressRatio != 100) {
					// 現在リビルド中.
					RebuildRecoveryStart();
				}
			}
		}
	}

	return True;
}

// ================================================================================================
//! <!--機能--> RAID構成情報の取得を行う.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! なし.
//! <!--戻値-->
//! \return Bool 処理結果.
//! 				True  : 正常終了.
//! 				False : 異常終了.
//! <!--詳細-->
//! RAID情報コマンド（SATA)を通知する。
//! 1の結果から、Ch-A、Ch-BのHDDの接続状態を決定する。
//! 現在のマスタチャネルを保存する。
//! リビルド情報の初期設定を行う。
//! <!--履歴-->
//	2014/09/08 yamada		ZeusS 制御構成変更No.4 HDDミラーリング対応.
//	2014/09/13 y.morikawa	ZeusS 制御構成変更No.4 HDDミラーリング対応 機能有効/無効処理の変更.
//	2014/09/08 y.morikawa	CQ504660 RAID構成の設定方法を修正.
//	2014/11/07 y.morikawa	ZeusS 制御構成変更No.4 HDDミラーリング対応 SATAコントローラがHDD構成の変更不可のため削除.
//	2015/01/21 y.morikawa	CQ507487 リビルド中の電源断問題の対応.
//  2016/03/14 yamada		D011詳細コード対応.
//! ================================================================================================
Bool CP_HDDTrans::initRaidInfo()
{
	if (isMirroringFunction() == False){
		m_e_HddMirroringDeviceStatus[THRE_HddMirroring_Chanel_A] = THRE_HddMirroringStatus_Normal;	// 正常.
		m_e_HddMirroringDeviceStatus[THRE_HddMirroring_Chanel_B] = THRE_HddMirroringStatus_Non;		// 未接続.
		return True;
	}

	// RAID情報の取得.
	THRS_HDD_DEVICE_RAID_INFO st_RaidInfo = {0};
	uint32_t ul_Ret = THRF_sysBspHddRaidInfo(THRE_HddPortNo, &st_RaidInfo);

	if (ul_Ret != HDD_STATE_OK) {
		setDebugLowLogSpeedSeq(781, ul_Ret) ;

		m_e_HddMirroringDeviceStatus[THRE_HddMirroring_Chanel_A] = THRE_HddMirroringStatus_Normal;	// 正常.
		m_e_HddMirroringDeviceStatus[THRE_HddMirroring_Chanel_B] = THRE_HddMirroringStatus_Non;		// 未接続.
		return True;
	} else {
		setDebugLowLogSpeedSeq(782, st_RaidInfo.uc_HddStructure) ;
		setDebugLowLogSpeedSeq(783, st_RaidInfo.uc_HddStatusChA) ;
		setDebugLowLogSpeedSeq(784, st_RaidInfo.uc_HddStatusChB) ;

		// CH.A 異常チェック.
		if (st_RaidInfo.uc_HddStatusChA == THRD_HDD_RAID_ERR_NON) {
			// 正常.
			m_e_HddMirroringDeviceStatus[THRE_HddMirroring_Chanel_A] = THRE_HddMirroringStatus_Normal;	// 正常.
		} else if (st_RaidInfo.uc_HddStatusChA & THRD_HDD_RAID_ERR_CONNECT_DEVICE) {
			// bit0: 1のときデバイス接続エラー.
			m_e_HddMirroringDeviceStatus[THRE_HddMirroring_Chanel_A] = THRE_HddMirroringStatus_Non;		// 未接続.
		} else {
			// 異常.
			m_e_HddMirroringDeviceStatus[THRE_HddMirroring_Chanel_A] = THRE_HddMirroringStatus_Trouble;	// 故障中.
		}

		// CH.B 異常チェック.
		if (st_RaidInfo.uc_HddStatusChB == THRD_HDD_RAID_ERR_NON) {
			// 正常.
			m_e_HddMirroringDeviceStatus[THRE_HddMirroring_Chanel_B] = THRE_HddMirroringStatus_Normal;	// 正常.
		} else if (st_RaidInfo.uc_HddStatusChB & THRD_HDD_RAID_ERR_CONNECT_DEVICE) {
			// bit0: 1のときデバイス接続エラー.
			m_e_HddMirroringDeviceStatus[THRE_HddMirroring_Chanel_B] = THRE_HddMirroringStatus_Non;		// 未接続.
		} else {
			// 異常.
			m_e_HddMirroringDeviceStatus[THRE_HddMirroring_Chanel_B] = THRE_HddMirroringStatus_Trouble;	// 故障中.
		}

		uchar uc_DeviceCount = 0;
		ul_Ret = THRF_sysBspHddGetDriveInfo(THRE_HddPortNo, &uc_DeviceCount);

		if (ul_Ret != HDD_STATE_OK) {
			// C-D011(HDD_OUT_OF_SPEC)詳細コードセット.
			us_FA14DetailCode = selectOutOfSpecDetailCode(THRE_D011Point_SCSIGetDriveInfoFailed);
			setDebugLowLogSpeedSeq(785, ul_Ret) ;
			return False;
		}

		if ( m_c_MirroringBackupData.isMirroringOn() == False ) {
															// HDD 個別構成.
			if (uc_DeviceCount == 2) {						// 個別構成でHDDを2台認識したとはRAID1へ構成変更する.
				setDebugLowLogSpeedSeq(786, 0) ;

				m_c_MirroringBackupData.setMirroringOn( True );				// HDD個別構成、2個装着ならばミラーリング機能は有効に設定変更してRAID情報の再取得を行う.
			}
		}
		else if (st_RaidInfo.uc_HddStructure == THRD_HDD_RAID_MODE_RAID1) {		// HDD RAID1構成.
			m_c_MirroringBackupData.setMirroringOn( True );					// ミラーリング有効.
		}
		else {												// HDD その他の構成.
			// C-D011(HDD_OUT_OF_SPEC)詳細コードセット.
			us_FA14DetailCode = selectOutOfSpecDetailCode(THRE_D011Point_RaidInfoError_NotRaid1);
			return False;
		}
	}

	setDebugLowLogSpeedSeq(788, st_RaidInfo.uc_MasterChanel) ;

	// THRF_sysBspHddRaidInfoに限り、取得するチャネルの値が 0:Non/1:ChA/2:ChBとなるため(他は0:ChA/1:ChB)、.
	// 直値で判定を行う.
	uchar uc_DevNo = THRE_SsdPortNo;
	if (st_RaidInfo.uc_MasterChanel == 0) {
		// C-D011(HDD_OUT_OF_SPEC)詳細コードセット.
		us_FA14DetailCode = selectOutOfSpecDetailCode(THRE_D011Point_RaidInfoError_ChanelNg);
		return False;
	}
	else if (st_RaidInfo.uc_MasterChanel == 1) {	// マスターチャネル=ChA.
		uc_DevNo = THRE_HddPortNo;
		m_e_HddMirroringMasterChanel = THRE_HddMirroring_Chanel_A;
	}
	else if (st_RaidInfo.uc_MasterChanel == 2) {	// マスターチャネル=ChB.
		uc_DevNo = THRE_HddChBPortNo;
		m_e_HddMirroringMasterChanel = THRE_HddMirroring_Chanel_B;
	}

	// 再起動前に発生したミラーリングエラーを再通知する.
	chkMirroringLastTroubleNotify(st_RaidInfo.uc_MasterChanel);

	// 起動時にリビルド再開処理（リビルド中の電源断からリカバリ必要）.
	// リビルド再開を通知するまでの間にスリープへ移行しないよう電源制御モジュールへHDDアクセス開始を通知.

	// リビルド動作状態取得.
	THRS_HDD_DEVICE_RBLD_INFO st_RebuildInfo = {0};
	ul_Ret = THRF_sysBspGetRebuildInfo(uc_DevNo, &st_RebuildInfo);
	if (ul_Ret != HDD_STATE_OK) {
		// C-D011(HDD_OUT_OF_SPEC)詳細コードセット.
		us_FA14DetailCode = selectOutOfSpecDetailCode(THRE_D011Point_SCSIGetRbldInfoFailed);
		setDebugLowLogSpeedSeq(789, ul_Ret);
		return False;
	}

	if (st_RebuildInfo.uc_RebuildRunning == 1) {
		// 電源制御へHDDアクセス開始を通知.
		APIC_Power* pc_ApiPower = APIC_Power::newInstance();
		if (pc_ApiPower == NULL) {
			// C-D011(HDD_OUT_OF_SPEC)詳細コードセット.
			us_FA14DetailCode = selectOutOfSpecDetailCode(THRE_D011Point_PowerApiInstanceFailed);
			setDebugLowLogSpeedSeq(787, 0) ;
			return False;
		}
		pc_ApiPower->changePowerMode(TYPE_PowerModeDataAccessStart);

		// 進捗率更新.
		// リビルド実行中LBAアドレス / 全体容量で計算.
		// 2017/02/20 Sky DLONG対応.
		uint64_t ull_HddSize = getPhysicalHDDSize();

		uint64_t ull_LBAAddress = (uint64_t)st_RebuildInfo.us_RebuildLBAAddressH << 32 |
										   (uint64_t)st_RebuildInfo.us_RebuildLBAAddressM << 16 |
										   (uint64_t)st_RebuildInfo.us_RebuildLBAAddressL;

		m_uc_ProgressRatio = (uchar)( (ull_LBAAddress * 100) / ( ull_HddSize / DEF_SECTOR_SIZE ));
		// 進捗率 99%を超える場合は、99%で止めておく(リビルド完了時に100%にする).
		if (m_uc_ProgressRatio > 99) {
			m_uc_ProgressRatio = 99;
		}
	}

	return True;
}

// ================================================================================================
//! <!--機能--> HDDトラブルを確認、通知する.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! なし.
//! <!--戻値-->
//! なし.
//! <!--詳細-->
//! 1. ミラーリング機能が無効のときは何もせずに終了。
//! 2. 不揮発から現在のHDDエラー情報を取得
//! 3. RAID動作の情報、状態を取得。(SATA)
//! 4. HDD動作構成がRAID1でなければ、不揮発の現在のHDDエラー情報を返して終了。
//! 6. マスターチャネルがNonのときは、ChA、ChBとも異常とする。
//! 7. 3のRAID動作の状態から異常が発生している場合、未検出の異常ならばエラーを発生する。
//! <!--履歴-->
//	2014/09/15 y.morikawa	ZeusS 制御構成変更No.4 HDDミラーリング対応.
//	2014/10/14 y.morikawa	CQ504382,CQ504646 HDDミラーリングエラー通知の問題対応.
//  2014/11/07 y.morikawa	ミラーリング機能 ログ修正.
//! ================================================================================================
void CP_HDDTrans::chkMirroringCurrentTroubleNotify()
{
	uchar  uc_DevNo = THRE_HddPortNo;
	uchar  uc_MasterChanel = THRE_HddMirroring_Chanel_Unknown;
	uchar  uc_SlaveChanel = THRE_HddMirroring_Chanel_Unknown;
	Bool   bl_IsMirroring = isMirroringHDD();
	Bool   bl_MasterErr = False;
	Bool   bl_SlaveErr = False;

	if (bl_IsMirroring == False) {
		return;		// ミラーリングがOFFのときはエラーなしで終了する.
	}

	// 上位APIのインスタンス生成.
	APIC_HddInfo* pc_HddInfo = APIC_HddInfo::newInstance();
	THRL_ThAssert( pc_HddInfo!=NULL, "CP_HDDTrans::chkMirroringCurrentTroubleNotify() pc_HddInfo is NULL", 0, 0 , 0, 0, CFA14_DetailCode0x0D1D );

	// RAID情報の取得.
	THRS_HDD_DEVICE_RAID_INFO st_RaidInfo = {0};
	uint32_t ul_Ret = THRF_sysBspHddRaidInfo(uc_DevNo, &st_RaidInfo);
	if (ul_Ret != HDD_STATE_OK) {
		setDebugLowLogSpeedSeq(815, ul_Ret) ;
		return;
	}
	if (st_RaidInfo.uc_HddStructure != THRD_HDD_RAID_MODE_RAID1) {		// HDD RAID1構成以外はRAID情報の結果を使用しない.
		return;
	}

	// CH.A 異常チェック.
	if (st_RaidInfo.uc_HddStatusChA == THRD_HDD_RAID_ERR_NON) {		// 正常.
		// リビルド、またはフォーマットで正常とするため状態を継続する.
	} else if (st_RaidInfo.uc_HddStatusChA & THRD_HDD_RAID_ERR_CONNECT_DEVICE) {					// 未接続.
		// bit0: 1のときデバイス接続エラー.
		m_e_HddMirroringDeviceStatus[THRE_HddMirroring_Chanel_A] = THRE_HddMirroringStatus_Non;

		setDebugLowLogSpeedSeq(816, st_RaidInfo.uc_HddStatusChA) ;
	} else {																						// 異常.
		m_e_HddMirroringDeviceStatus[THRE_HddMirroring_Chanel_A] = THRE_HddMirroringStatus_Trouble;
		setDebugLowLogSpeedSeq(816, st_RaidInfo.uc_HddStatusChA) ;
	}

	// CH.B 異常チェック.
	if (st_RaidInfo.uc_HddStatusChB == THRD_HDD_RAID_ERR_NON) {										// 正常.
		// リビルド、またはフォーマットで正常とするため状態を継続する.
	} else if (st_RaidInfo.uc_HddStatusChB & THRD_HDD_RAID_ERR_CONNECT_DEVICE) {					// 未接続.
		// bit0: 1のときデバイス接続エラー.
		m_e_HddMirroringDeviceStatus[THRE_HddMirroring_Chanel_B] = THRE_HddMirroringStatus_Non;
		setDebugLowLogSpeedSeq(817, st_RaidInfo.uc_HddStatusChB) ;
	} else {																						// 異常.
		m_e_HddMirroringDeviceStatus[THRE_HddMirroring_Chanel_B] = THRE_HddMirroringStatus_Trouble;
		setDebugLowLogSpeedSeq(817, st_RaidInfo.uc_HddStatusChB) ;
	}

	if (st_RaidInfo.uc_MasterChanel == 0) {			// マスターチャネルが確定しないときは両チャネルとも異常とする.
		m_e_HddMirroringDeviceStatus[THRE_HddMirroring_Chanel_A] = THRE_HddMirroringStatus_Trouble;	// 異常.
		m_e_HddMirroringDeviceStatus[THRE_HddMirroring_Chanel_B] = THRE_HddMirroringStatus_Trouble;	// 異常.
		uc_MasterChanel = THRE_HddMirroring_Chanel_A;
		uc_SlaveChanel = THRE_HddMirroring_Chanel_B;

	}
	else if (st_RaidInfo.uc_MasterChanel == 1) {	// マスターチャネル=ChA.
		uc_MasterChanel = THRE_HddMirroring_Chanel_A;
		uc_SlaveChanel = THRE_HddMirroring_Chanel_B;
	}
	else if (st_RaidInfo.uc_MasterChanel == 2) {	// マスターチャネル=ChB.
		uc_MasterChanel = THRE_HddMirroring_Chanel_B;
		uc_SlaveChanel = THRE_HddMirroring_Chanel_A;
	}

	if ((m_e_HddMirroringDeviceStatus[uc_MasterChanel] == THRE_HddMirroringStatus_Trouble) ||	// 未接続、または、異常発生をトラブルとする.
		(m_e_HddMirroringDeviceStatus[uc_MasterChanel] == THRE_HddMirroringStatus_Non)){
		bl_MasterErr = True;
	}
	if ((m_e_HddMirroringDeviceStatus[uc_SlaveChanel] == THRE_HddMirroringStatus_Trouble) ||
		(m_e_HddMirroringDeviceStatus[uc_SlaveChanel] == THRE_HddMirroringStatus_Non)){
		bl_SlaveErr = True;
	}

	if ((bl_MasterErr == True) && (bl_SlaveErr == True)) {						// マスターとスレーブの両方で異常発生かを確認.
		if (m_c_MirroringBackupData.isActiveError( THRE_HDD_MIRRORING_MASTER_SLAVE_ERROR ) == False) {				// HDDミラーリングエラー(マスタースレーブ)が未通知のときだけ.
			m_c_MirroringBackupData.setMirroringBackupDataErrorBit( THRE_HDD_MIRRORING_MASTER_SLAVE_ERROR, True );	// エラーを発生し、エラー履歴を残す.

			// commonAPIへHDDミラーリングエラー発生を通知.
			TYPS_HDDMirroringEventInfo st_EventInfo;
			st_EventInfo.e_Event    = TYPE_HDDMirroringEvent_Error;				// HDDミラーリングエラー発生.
			st_EventInfo.e_ChanelNo = uc_MasterChanel;							// マスターチャネル.

			setDebugLowLogSpeedSeq(818, st_EventInfo.e_Event) ;

			pc_HddInfo->notifyHDDMirroringEvent(st_EventInfo);

			sendStatusNotice( TYPD_TroubleCode_HDD_MASTER_SLAVE_ERROR );		// HDDミラーリングエラー マスタースレーブ.
		}
	}
	else if (bl_MasterErr == True){												// マスターのみで異常発生かを確認.
		if (m_c_MirroringBackupData.isActiveError( THRE_HDD_MIRRORING_MASTER_ERROR ) == False) {					// HDDミラーリングエラー(マスター)が未通知のときだけ.
			m_c_MirroringBackupData.setMirroringBackupDataErrorBit( THRE_HDD_MIRRORING_MASTER_ERROR, True );		// エラーを発生し、エラー履歴を残す.

			// commonAPIへHDDミラーリングエラー発生を通知.
			TYPS_HDDMirroringEventInfo st_EventInfo;
			st_EventInfo.e_Event    = TYPE_HDDMirroringEvent_Error;				// HDDミラーリングエラー発生.
			st_EventInfo.e_ChanelNo = uc_MasterChanel;							// マスターチャネル.

			setDebugLowLogSpeedSeq(818, st_EventInfo.e_Event) ;

			pc_HddInfo->notifyHDDMirroringEvent(st_EventInfo);

			// マスターのエラーは再起動でチャネルが変わるため、現時点のマスターチャネルを記録しておく.
			m_c_MirroringBackupData.setMirroringBackupDataMasterCh( m_e_HddMirroringMasterChanel );
			setDebugLowLogSpeedSeq(819, m_e_HddMirroringMasterChanel) ;

			sendStatusNotice( TYPD_TroubleCode_HDD_MASTER_ERROR );				// HDDミラーリングエラー マスタースレーブ.
		}
	}
	else if (bl_SlaveErr == True){												// スレーブのみで異常発生かを確認.
		if (m_c_MirroringBackupData.isActiveError( THRE_HDD_MIRRORING_SLAVE_ERROR ) == False) {						// HDDミラーリングエラー(スレーブ)が未通知のときだけ.
			m_c_MirroringBackupData.setMirroringBackupDataErrorBit (THRE_HDD_MIRRORING_SLAVE_ERROR, True );			// エラーを発生し、エラー履歴を残す.

			// commonAPIへHDDミラーリングエラー発生を通知.
			TYPS_HDDMirroringEventInfo st_EventInfo;
			st_EventInfo.e_Event    = TYPE_HDDMirroringEvent_Error;				// HDDミラーリングエラー発生.
			st_EventInfo.e_ChanelNo = uc_SlaveChanel;							// マスターチャネル.

			setDebugLowLogSpeedSeq(818, st_EventInfo.e_Event) ;
			pc_HddInfo->notifyHDDMirroringEvent(st_EventInfo);

			sendStatusNotice( TYPD_TroubleCode_HDD_SLAVE_ERROR );				// HDDミラーリングエラー スレーブ.
		}
	}

	return;
}

// ================================================================================================
//! <!--機能--> 前回起動時のトラブルを再通知する.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! なし.
//! <!--戻値-->
//! なし.
//! <!--詳細-->
//! 不揮発に保持した前回起動時のエラー発生情報を基に、HDDミラーリングイベント
//! （前回起動時HDDミラーリングエラー発生）を通知する
//! <!--履歴-->
//	2014/09/15 y.morikawa	ZeusS 制御構成変更No.4 HDDミラーリング対応.
//  2014/11/07 y.morikawa	ミラーリング機能 ログ修正.
//! ================================================================================================
void CP_HDDTrans::chkMirroringLastTroubleNotify(uchar uc_Mchan)
{
	Bool   bl_IsMirroring = isMirroringHDD();
	if (bl_IsMirroring == False){
		return ;		// ミラーリングがOFFのときはエラーなしで終了する.
	}

	// 上位APIのインスタンス生成.
	APIC_HddInfo* pc_HddInfo = APIC_HddInfo::newInstance();
	THRL_ThAssert( pc_HddInfo!=NULL, "CP_HDDTrans::chkMirroringLastTroubleNotify() pc_HddInfo is NULL", 0, 0 , 0, 0, CFA14_DetailCode0x0D1E );

	// 引数のマスターチャネルを基に、スレーブのチャネルを特定する.
	uchar uc_Schan;
	if (uc_Mchan == THRE_HddMirroring_Chanel_A) {
		uc_Schan = THRE_HddMirroring_Chanel_B;
	}
	else {
		uc_Schan = THRE_HddMirroring_Chanel_A;
	}

	if (m_c_MirroringBackupData.isActiveError(THRE_HDD_MIRRORING_MASTER_SLAVE_ERROR) == True) {
		TYPS_HDDMirroringEventInfo st_EventInfo;
		st_EventInfo.e_Event    = TYPE_HDDMirroringEvent_ErrorPreStartup;	// 前回起動時HDDミラーリングエラー発生.
		st_EventInfo.e_ChanelNo = uc_Mchan;									// マスターチャネル.

		setDebugLowLogSpeedSeq(850, st_EventInfo.e_Event) ;

		pc_HddInfo->notifyHDDMirroringEvent(st_EventInfo);
	}

	if (m_c_MirroringBackupData.isActiveError(THRE_HDD_MIRRORING_MASTER_ERROR) == True) {
		TYPS_HDDMirroringEventInfo st_EventInfo;
		st_EventInfo.e_Event    = TYPE_HDDMirroringEvent_ErrorPreStartup;	// 前回起動時HDDミラーリングエラー発生.
		st_EventInfo.e_ChanelNo = uc_Mchan;									// マスターチャネル.

		setDebugLowLogSpeedSeq(850, st_EventInfo.e_Event) ;

		pc_HddInfo->notifyHDDMirroringEvent(st_EventInfo);
	}

	if (m_c_MirroringBackupData.isActiveError(THRE_HDD_MIRRORING_SLAVE_ERROR) == True) {
		TYPS_HDDMirroringEventInfo st_EventInfo;
		st_EventInfo.e_Event    = TYPE_HDDMirroringEvent_ErrorPreStartup;	// 前回起動時HDDミラーリングエラー発生.
		st_EventInfo.e_ChanelNo = uc_Schan;									// スレーブチャネル.

		setDebugLowLogSpeedSeq(850, st_EventInfo.e_Event) ;

		pc_HddInfo->notifyHDDMirroringEvent(st_EventInfo);
	}

	if (m_c_MirroringBackupData.isActiveError(THRE_HDD_MIRRORING_REBUILD_MASTER_ERROR) == True) {
		TYPS_HDDMirroringEventInfo st_EventInfo;
		st_EventInfo.e_Event    = TYPE_HDDMirroringEvent_ErrorPreStartup;	// 前回起動時HDDミラーリングエラー発生.
		st_EventInfo.e_ChanelNo = uc_Mchan;									// マスターチャネル.

		setDebugLowLogSpeedSeq(850, st_EventInfo.e_Event) ;

		pc_HddInfo->notifyHDDMirroringEvent(st_EventInfo);
	}

	if (m_c_MirroringBackupData.isActiveError(THRE_HDD_MIRRORING_REBUILD_SLAVE_ERROR) == True) {
		TYPS_HDDMirroringEventInfo st_EventInfo;
		st_EventInfo.e_Event    = TYPE_HDDMirroringEvent_ErrorPreStartup;	// 前回起動時HDDミラーリングエラー発生.
		st_EventInfo.e_ChanelNo = uc_Schan;									// スレーブチャネル.

		setDebugLowLogSpeedSeq(850, st_EventInfo.e_Event) ;

		pc_HddInfo->notifyHDDMirroringEvent(st_EventInfo);
	}

	return;
}

// ================================================================================================
//! <!--機能--> ミラーリング機能対応モデル確認.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! なし.
//! \return Bool	ミラーリング機能へ対応するモデルかの判定結果.
//! 					True:対応/False:未対応.
//! <!--詳細-->
//! コンパイルスイッチからミラーリング機能へ対応するモデルかを判断する.
//! <!--履歴-->
//	2014/09/15 y.morikawa	ZeusS 制御構成変更No.4 HDDミラーリング対応.
//	2015/03/17 yamashita	MinervaSBK 制御構成変更No.10 HDDミラーリング対応.
//	2015/06/10 miyazaki		ZeusSZX0 制御構成変更No.53 コンパイルSW対応.
//	2016/01/22 furuichi		ZeusSBK  制御構成変更No.18 機種コード対応.
//	2016/10/14 Sky鈴木		[IT5_4.1] 制御構成変更No.10 コンパイル環境作成.
//  2017/09/07 Sky薗﨑		Eagle 制御構成変更_No.66_USB-SATA(Raid)制御対応.
//! ================================================================================================
Bool CP_HDDTrans::isMirroringFunction()
{
	Bool bl_FuncFlag = False;
	return bl_FuncFlag;
}

// ================================================================================================
//! <!--機能--> ストレージのベリファイを行う.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! THRE_StorageDeviceType	e_StrageType	:	デバイス種別.
//! ulong					ul_SectorNo		:	開始セクタNo.
//! uchar*					puc_Expect		:	期待値格納領域.
//! ulong					ul_SectorNum	:	ベリファイするセクタ数.
//! \return Bool	True:正常終了／False:異常終了.
//! <!--詳細-->
//! 指定されたストレージのベリファイを行う.
//! <!--履歴-->
//	2014/09/18	Ogura		新規作成（HDDミラーリング対応）.
//	2015/01/29	Ooka		CQ.509898 HDDミラーリングでスレーブ未接続時はスレーブ側処理なし
//	15/02/20 y.morikawa 	CQ.507060 HDDミラリング再構筑を実行中にHDDフォーマットを実施できない。SlaveのR/Wチェックを削除.
//  2017/10/10 Yoneda       S800 ストレージ構成変更対応.
//! ================================================================================================
Bool CP_HDDTrans::VerifyStrage( THRE_StorageDeviceType e_StrageType, uint32_t ul_SectorNo, uchar* puc_Expect, uint32_t ul_SectorNum )
{
	char* psc_DevicePath = theHDDTrans.pc_strDeviceSD;
	if ( THRE_StorageDeviceTypeHDD == e_StrageType ) {
		psc_DevicePath = theHDDTrans.pc_strDeviceSSD;
	}
	
	//	べリファイチェック.
	if ( False == VerifyHddOneSide( psc_DevicePath, ul_SectorNo, puc_Expect, ul_SectorNum ) ) {
		setDebugLowLogSpeedSeq( 830, e_StrageType );
		return False;
	}
	
	return True;
}

// ================================================================================================
//! <!--機能--> ストレージのベリファイを行う.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! char*					psc_DevPath		:	デバイスファイルパス.
//! ulong					ul_SectorNo		:	開始セクタNo.
//! uchar*					puc_Expect		:	期待値格納領域.
//! ulong					ul_SectorNum	:	ベリファイするセクタ数.
//! \return Bool	True:正常終了／False:異常終了.
//! <!--詳細-->
//! 指定されたストレージのベリファイを行う.
//! <!--履歴-->
//	2014/09/18	Ogura		新規作成（HDDミラーリング対応）.
//	2014/11/05 yamada		CQ506674 HDD廃棄時消去のべリファイ実行時にエラーとなる問題対応.
//! ================================================================================================
Bool CP_HDDTrans::VerifyHddOneSide( char* psc_DevPath, uint32_t ul_SectorNo, uchar* puc_Expect, uint32_t ul_SectorNum )
{
	if ( ( NULL == puc_Expect ) || ( NULL == psc_DevPath ) || ( 0 == ul_SectorNum ) ) {
		setDebugLowLogSpeedSeq( 834, 0 );
		return False;
	}
	
	uint32_t ul_AreaSize = ul_SectorNum * DEF_SECTOR_SIZE;
	uchar* puc_WorkAreaAlign = new uchar[ ul_AreaSize + 0x200 ];

	if ( NULL == puc_WorkAreaAlign ) {
		return False;
	}

	// PCI MEM先頭アドレス(64byte単位)のため切り上げ処理.
	uchar* puc_WorkArea = (uchar *)(((uintptr_t)puc_WorkAreaAlign & ~0x1FF) + 0x200);

	//	あり得ない値（0x55）で初期化しておく.
	memset( puc_WorkArea, 0x55, ul_AreaSize );
	
	Bool bl_Result = False;
	//	ディスクキャッシュのクリア.
	if ( True == THRL_ClearDiskCache( psc_DevPath ) ) {
		readWriteExecHDD( psc_DevPath, THRE_ReadWriteHDDToBuf, NULL, (uintptr_t)puc_WorkArea, ul_SectorNo, ul_SectorNum );
		if ( 0 == memcmp( puc_WorkArea, puc_Expect, ul_AreaSize ) ) {
			bl_Result = True;
		} else {
			setDebugLowLogSpeedSeq( 836, 0 );
		}
	} else {
		setDebugLowLogSpeedSeq( 835, 0 );
	}
	delete [] puc_WorkAreaAlign;
	
	return bl_Result;
}

// ================================================================================================
//! <!--機能--> C-D011(HDD_OUT_OF_SPEC)の詳細コード判定.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! uchar	us_Point	:	検出箇所.
//! <!--戻値-->
//! \return ushort		:	詳細コード.
//! <!--詳細-->
//! 検出箇所と条件を付与した詳細コードを返す\n
//! 戻り値 詳細\n
//! 　　┏━┯━┯━┯━┯━┯━┯━┯━┓\n
//! MSB ┃15│14│13│12│11│10│ 9│ 8┃\n
//! 　　┗━┷━┷━┷━┷━┷━┷━┷━┛\n
//! 　　┏━┯━┯━┯━┯━┯━┯━┯━┓\n
//! 　　┃ 7│ 6│ 5│ 4│ 3│ 2│ 1│ 0┃LSB \n
//! 　　┗━┷━┷━┷━┷━┷━┷━┷━┛\n
//! 15 ～ 8	:	使用不可\n
//! 7		:	バルクストレージデバイスファイル(sdc)有無	：有り(1)、無し(0)\n
//! 6		:	HDDデバイスファイル(kmsdb)有無				：有り(1)、無し(0)\n
//! 5  ～ 4 :	起動モード									：通常(0)、メンテナンス(1)、取得不可(2)\n
//! 3  ～ 0 :	検出箇所(現状0 ～ 10まで使用)\n
//! <!--履歴-->
//  2016/03/14	yamada		新規作成(D011詳細コード対応).
//! ================================================================================================
ushort CP_HDDTrans::selectOutOfSpecDetailCode(uchar us_Point)
{
	ushort us_DetailCode = CD011_DetailCode0x6600;	// D011詳細コードベース.

	// 検出箇所のセット.
	us_DetailCode += us_Point;

	// 起動モードのセット.
	ushort us_Mode = THRG_GetSataFwMode();
	switch(us_Mode) {
	case THRD_SataFwMode_Normal:
		// 通常モードの場合は何もセットしない.
		break;
	case THRD_SataFwMode_Maintenance:
		// メンテナンスモードの場合は1をセット.
		us_DetailCode |= (ushort)(1 << 4);
		break;
	default:
		//上記以外の場合2をセット.
		us_DetailCode |= (ushort)(2 << 4);
		break;
	}

	// kmsdb有無のセット.
	if (True == THRG_ExistHddDevFile()) {
		// kmsdbファイルがある場合は1をセット.
		us_DetailCode |= (ushort)(1 << 6);
	}

	// sdc有無のセット.
	if (True == THRG_ExistBulkStorageDevFile()) {
		// sdcファイルがある場合は1をセット.
		us_DetailCode |= (ushort)(1 << 7);
	}

	return us_DetailCode;
}

// ================================================================================================
//! <!--機能--> SATAコントローラ FW修復.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param なし.
//! <!--戻値-->
//! \return Bool : True 成功, False 失敗.
//! <!--詳細-->
//! SATAコントローラ FW修復を行う.
//! <!--履歴-->
//  2016/03/17	yamada		新規作成(SATAコントローラ、メンテナンスモード起動時の自動復旧対応).
// ================================================================================================
Bool CP_HDDTrans::fixSATACtrlFw(void)
{
	// IT6ではSATAコントローラは使用しないため削除し、必ずFalseを返す制御とする.
	// 本関数がコールされることが誤りとなるため、Falseとし、D011を発生させる.
	// ※Trueにすると自動でrebootされてしまう.
	return False;
}

// ================================================================================================
//! <!--機能--> checkInfomation NG時の後処理.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! なし.
//! <!--戻値-->
//! \return なし.
//! <!--詳細-->
//! func_HDDSeq_CheckInformation()でNG検出時(D011)の後処理(サブルーチン).
//! <!--履歴-->
//  2016/03/17	yamada		新規作成(SATAコントローラ、メンテナンスモード起動時の自動復旧対応).
//  2016/03/26	simogaki	D011発生条件見直し.
//  　　　　　　　　　　　　他トラブルによりFW自動復をせずリセット回数が上限に達した場合、最終トラブルはFA14で表示されることと.
//  　　　　　　　　　　　　HDDが破損している場合(SATAFWがメンテナンスモードでないエラーの場合)、D011トラブルが最大　5分(リトライ300回)*3(リブート上限)待たないと.
//  　　　　　　　　　　　　D011とならない(トラブル判定に時間がかかる)ため条件を以下に変更.
//  　　　　　　　　　　　　　変更前：トラブルリセット回数が上限に達した　且つ　SATAFW自動復旧実行済.
//  　　　　　　　　　　　　　変更後：トラブルリセット回数が上限に達した　または　SATAFWがメンテナンスモード以外の場合.
//! ================================================================================================
void CP_HDDTrans::postProc_CheckInfomationNg()
{
	ushort us_Mode = 0;					// SATAFWのモード.
	us_Mode = THRG_GetSataFwMode();
	// FW自動復旧関数を実行する(自動リブート無し指定).
	(void)autoRecoverySataFw(False);

	// トラブル検出回数の取得.
	APIC_MachineSetting *pAPIC_MachineSetting = APIC_MachineSetting::newInstance();
	if( pAPIC_MachineSetting == NULL ){
		return;
	}
	uint32_t ul_TroubleRetryCnt = pAPIC_MachineSetting->getFatalCancelRetryCnt(TYPE_FatalCancelRetryCntReboot);

	if (( TYPD_FatalCancelRetryCntRebootMax <= ul_TroubleRetryCnt ) || ( THRD_SataFwMode_Maintenance != us_Mode )){
		// D011を発生させる.
		THRL_ThAssertLog(getFA14DetailCode());	// D011詳細コードをセット.
		sendStatusNotice( HDD_OUT_OF_SPEC );	// スペック以外のＨＤＤで終了.
	} else {
		// Fatal連続検出2回目まではFA14.
		THRL_ThAssert( False, "[SATA2] func_HDDSeq_CheckInformation() OutOfSpec Error!! DetailCode(0x%02X)\n" , (ushort)getFA14DetailCode(), 0, 0, 0, getFA14DetailCode() );
	}

	return;
}

// ================================================================================================
//! <!--機能--> SATA FW自動復旧関数.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! Bool	bl_AutoReboot	:	自動リブートの有効無効.
//! <!--戻値-->
//! \return Bool			:	SATA FW更新成功(True)/失敗 or 更新対象外(False).
//! <!--詳細-->
//! 自動復旧のチェック、及び復旧処理まで実行する.
//! <!--履歴-->
//  2016/03/17	yamada		新規作成(SATAコントローラ、メンテナンスモード起動時の自動復旧対応).
//! ================================================================================================
Bool CP_HDDTrans::autoRecoverySataFw(Bool bl_AutoReboot)
{
	// セマフォ生成.
	SemCreateAutoRecoverySataFw();

	// 万が一、HDDの接続要求と上位モジュールからの要求が競合したケースを想定してセマフォ管理する.
	SYSF_semTake(semID_SataFwAutoRecover, WAIT_FOREVER);

	Bool bl_Result = False;		// 失敗で初期化.

		// メンテナンスモードの場合、FW更新を行い復旧する.
	ushort us_Mode = THRG_GetSataFwMode();
	if (THRD_SataFwMode_Maintenance == us_Mode) {
		// SATA FW更新処理.
		if (False == fixSATACtrlFw()) {
			// 更新失敗時はログを残して終了.
			setDebugLowLogSpeedSeq(871, 0);
			THRL_ThLogMsgErr("[autoRecoverySataFw] SATA FW Update Failed.\n");
			THRL_ThAssertLog(selectOutOfSpecDetailCode(THRE_D011Point_AutoRecoverSataFwFailed));
		} else {
			// 更新正常終了.
			setDebugLowLogSpeedSeq(872, 0);
			THRL_ThLogMsgErr("[autoRecoverySataFw] SATA FW Update Success!!\n");
			THRL_ThAssertLog(selectOutOfSpecDetailCode(THRE_D011Point_AutoRecoverSataFwSuccess));
			bl_Result = True;

			// SPI-Flashに更新フラグをセット.
			(void)THRG_setSataFwAutoRecoverExec();

			if (True == bl_AutoReboot) {
				setDebugLowLogSpeedSeq(878, 0);
				THRL_ThLogMsgErr("[autoRecoverySataFw] reboot start...\n");

				// リブート実行.
				extern void reboot();
				reboot();
			}
		}
	} else {
		// メンテナンスモード以外は何もしない.
		setDebugLowLogSpeedSeq(870, (ushort)us_Mode);
		THRL_ThLogMsgErr("[autoRecoverySataFw] Not MaintenanceMode(0x%04X)\n", us_Mode);
	}

	SYSF_semGive(semID_SataFwAutoRecover);

	return bl_Result;
}

// ================================================================================================
//! <!--機能--> SATA FW自動復旧セマフォ生成.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! なし.
//! <!--戻値-->
//! なし.
//! <!--詳細-->
//! 自動復旧用セマフォの生成を行う.
//! <!--履歴-->
//  2016/03/17	yamada		新規作成(SATAコントローラ、メンテナンスモード起動時の自動復旧対応).
//! ================================================================================================
void CP_HDDTrans::SemCreateAutoRecoverySataFw(void)
{
	if (NULL == semID_SataFwAutoRecover) {
		semID_SataFwAutoRecover = SYSF_semMCreate(SEM_Q_PRIORITY | SEM_DELETE_SAFE | SEM_INVERSION_SAFE); 
		THRL_ThAssert(semID_SataFwAutoRecover != NULL, "CP_HDDTrans::CP_HDDTrans semID_SataFwAutoRecover Sem Create Error!!" , 0, 0, 0, 0, CFA14_DetailCode0x1001);
	}

	return;
}

// ================================================================================================
//! <!--機能--> ディスクキャッシュクリア.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! char*	psc_DevPath	:	デバイスファイルパス.
//! \return Bool	True:正常終了／False:異常終了.
//! <!--詳細-->
//! 指定されたストレージのディスクキャッシュをクリアする.
//! <!--履歴-->
//	2014/09/18	Ogura		新規作成（HDDミラーリング対応）.
//! ================================================================================================
Bool THRL_ClearDiskCache( char* psc_DevPath )
{
	if ( NULL == psc_DevPath ) {
		setDebugLowLogSpeedSeq( 838, 0 );
		return False;
	}
	
	int32_t si_File = SYSF_open( psc_DevPath, O_RDWR, 0666 );
	if ( -1 == si_File ) {
		setDebugLowLogSpeedSeq( 839, 0 );
		return False;
	}
	
	//	ioctl BLKFLSBUFを発行し、ディスクキャッシュをクリアする.
	Bool bl_Result = True;
	int32_t si_Ret = SYSF_ioctl( si_File, BLKFLSBUF, 0 );
	if ( -1 == si_Ret ) {
		setDebugLowLogSpeedSeq( 840, 0 );
		bl_Result = False;
	}
	
	SYSF_Close( si_File );
	
	return bl_Result;
}

// ================================================================================================
//! <!--機能--> デバイスNo、チャネルNoの取得.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! uchar& ruc_MasterDevNo
//! THRE_HddMirroring_Chanel& re_MasterCh
//! uchar& ruc_SlaveDevNo
//! THRE_HddMirroring_Chanel& re_SlaveCh
//! \return void.
//! <!--詳細-->
//! マスタ／スレーブのデバイスNo、チャネルNoの取得.
//! <!--履歴-->
//	2014/09/18	Ogura		新規作成（HDDミラーリング対応）.
//! ================================================================================================
void CP_HDDTrans::getDevNoAndChNo( uchar& ruc_MasterDevNo, THRE_HddMirroring_Chanel& re_MasterCh, 
							uchar& ruc_SlaveDevNo, THRE_HddMirroring_Chanel& re_SlaveCh )
{
	ruc_MasterDevNo	= THRE_HddPortNo;
	re_MasterCh		= THRE_HddMirroring_Chanel_A;
	//	ミラーリングが有効な場合のみ、RAIDコントローラからの設定値取得、スレーブの設定値取得を行う.
	if ( True == isMirroringHDD() ) {
		re_MasterCh	= getMirroringMasterChanel();
		re_SlaveCh	= THRE_HddMirroring_Chanel_B;
		ruc_MasterDevNo	= THRE_HddPortNo;
		ruc_SlaveDevNo	= THRE_HddChBPortNo;
		if ( THRE_HddMirroring_Chanel_B == re_MasterCh ) {
			re_SlaveCh			= THRE_HddMirroring_Chanel_A;			//	マスタがChanelAであれば、ChanelBに切り替える.
			ruc_MasterDevNo		= THRE_HddChBPortNo;
			ruc_SlaveDevNo		= THRE_HddPortNo;
		}
	}
}

// ================================================================================================
//! <!--機能--> HDDのデバイスファイル(/dev/kmsdb)の存在有無を返却する.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! なし.
//! \return Bool	True  : HDDデバイスファイル生成済み.
//! 				False : HDDデバイスファイル未生成.
//! <!--詳細-->
//! HDDデバイスファイル(/dev/kmsdb)のファイル有無をチェックし、結果を返却する.
//! <!--履歴-->
//  2015/02/03	yamada		CQ509539 HDDデバイスファイル存在チェックAPI追加.
//! ================================================================================================
Bool THRG_ExistHddDevFile()
{
	if (access(theHDDTrans.pc_strDeviceSSD, F_OK) != 0) {
		return False;
	}
	return True;
}

// ================================================================================================
//! <!--機能--> SATA FW起動状態を取得する.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! なし.
//! <!--戻値-->
//! \return ushort	 : プロダクトID(起動モード).
//! <!--詳細-->
//! 0x202A : 通常モード.
//! 0x2027 : メンテナンスモード.
//! 0x0000 : 異常値.
//! <!--履歴-->
//  2016/03/14	yamada		新規作成(D011詳細コード対応).
//! ================================================================================================
ushort THRG_GetSataFwMode(void)
{
	struct usb_bus *st_Busses = NULL;	// USB Bus構造体.

	// これらはusb.hをincludeする事で使用可能.
	usb_init();				// libusbの初期化.
	usb_find_busses();		// 全てのUSBバスを検索.
	usb_find_devices();		// 全てのUSBデバイスを検索.
	st_Busses = usb_get_busses();	// 現在接続されているUSBバス情報を取得.

	for( struct usb_bus *st_Bus = st_Busses; NULL!=st_Bus; st_Bus = st_Bus->next ){					// 現在接続されているUSBバス.
		for( struct usb_device *st_Dev = st_Busses->devices; NULL!=st_Dev; st_Dev = st_Dev->next) {	// 現在接続されているUSBデバイス.
			// 下記条件の時に起動状態を取得する.
			// USB3.0 且つ 通常モード.
			// USB2.0 且つ メンテナンスモード.
			if (((THRD_SataFwUSB_3_0 == st_Dev->descriptor.bcdUSB) &&				// USB3.0.
				 (THRD_SataFwMode_Normal == st_Dev->descriptor.idProduct)) ||		// 通常モード.
				((THRD_SataFwUSB_2_0 == st_Dev->descriptor.bcdUSB) &&				// USB2.0.
				 (THRD_SataFwMode_Maintenance == st_Dev->descriptor.idProduct))) {	// メンテナンスモード.
				return st_Dev->descriptor.idProduct;
			}
		}
	}

	// 何れにも該当しない場合.
	THRL_ThLogMsgErr("[THRG_GetSataFwMode] Not Found. ProductID 0x%04X or 0x%04X \n", THRD_SataFwMode_Normal, THRD_SataFwMode_Maintenance);

	return 0;
}

// ================================================================================================
//! <!--機能--> バルクストレージデバイスファイル(/dev/sdc)の存在有無を返却する.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! なし.
//! <!--戻値-->
//! \return Bool	True  : デバイスファイル生成済み.
//! 				False : デバイスファイル未生成.
//! <!--詳細-->
//! デバイスファイル(/dev/sdc)のファイル有無をチェックし、結果を返却する.
//! <!--履歴-->
//  2016/03/14	yamada		新規作成(D011詳細コード対応).
//! ================================================================================================
Bool THRG_ExistBulkStorageDevFile()
{
	if (access("/dev/sdc", F_OK) != 0) {
		return False;
	}
	return True;
}

// ================================================================================================
//! <!--機能--> SATA FW自動復旧関数(外部公開用).
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! Bool	bl_AutoReboot	:	自動リブートの有効無効.
//! <!--戻値-->
//! \return Bool			:	SATA FW更新成功(True)/失敗 or 更新対象外(False).
//! <!--詳細-->
//! 自動復旧のチェック、及び復旧処理まで実行する.
//! <!--履歴-->
//  2016/03/17	yamada		新規作成(SATAコントローラ、メンテナンスモード起動時の自動復旧対応).
//! ================================================================================================
Bool THRG_AutoRecoverySataFw(Bool bl_AutoReboot)
{
	return theHDDTrans.autoRecoverySataFw(bl_AutoReboot);
}


// ================================================================================================
//! <!--機能--> SCSIコマンドのリトライ回数の取得.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! なし.
//! \return uint32_t		リトライ回数.
//! <!--詳細-->
//! HDDがメンテナンスモードであればリトライなし、そうでなければリトライなし.
//! <!--履歴-->
//  2016/03/23			D011問題対応.
//! ================================================================================================
static uint32_t THRL_GetSCSICommandRetryCount()
{
	uint32_t ul_RetryCnt = THRD_HddSCSICommandRetryMax;
	
	//	メンテナンスモードの場合はリトライしない.
	if ( THRD_SataFwMode_Maintenance == THRG_GetSataFwMode() ) {
		ul_RetryCnt = 0;
	}
	
	return ul_RetryCnt;
}

// ================================================================================================.
//! <!--機能--> パーティションチェック処理.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! char* psc_Device	:	デバイス名.
//! <!--戻値-->.
//! \return Bool		:	チェック結果(True:成功、False:失敗).
//! <!--詳細-->.
//! なし.
//! <!--履歴-->.
//  2016/08/25	iyanagi	【IT5_4.0】制御構成変更No45「基板交換低減対応」.
//  2017/02/17	iyanagi	【IT5_4.0】AR1012448 パーティション情報の有無チェックと再設定追加.
//! ================================================================================================.
Bool CP_HDDTrans::checkPartition( char* psc_Device )
{
	if( psc_Device == NULL ){
		SYSF_logMsg( "[checkPartition] Device is NULL.\n" );
		return False;
	}

	int si_Ret = checkPartitionLayout( psc_Device );
	if ( si_Ret == -1 ) {
		SYSF_logMsg( "[checkPartition] %s checkPartitionLayout NG.\n", psc_Device );

		// パーティション情報の計算設定.
		si_Ret = calcAndFixedPartInfo( psc_Device );
		if ( si_Ret == -1 ) {
			SYSF_logMsg( "[checkPartition] %s calcAndFixedPartInfo NG.\n", psc_Device );
			return False;
		}

		// 再チェック.
		si_Ret = checkPartitionLayout( psc_Device );
		if ( si_Ret == -1 ) {
			SYSF_logMsg( "[checkPartition] %s checkPartitionLayout NG2.\n", psc_Device );
			return False;
		}
	}
	return True;
}

// ================================================================================================.
//! <!--機能--> FSCK（FAT領域の正当性チェック）処理.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! char* psc_Device	:	デバイス名.
//! <!--戻値-->.
//! \return Bool		:	チェック結果(True:成功、False:失敗).
//! <!--詳細-->.
//! なし.
//! <!--履歴-->.
//  2016/08/25	iyanagi	【IT5_4.0】制御構成変更No45「基板交換低減対応」.
//  2017/10/10  Yoneda   S800 ストレージ構成変更対応.
//! ================================================================================================.
Bool CP_HDDTrans::checkFsck( char* psc_Device )
{
	if( psc_Device == NULL ){
		SYSF_logMsg( "[checkFsck] Device is NULL.\n" );
		return False;
	}
	Bool bl_Ret = True;
	THRE_StorageDeviceType e_InitDeviceType  = THRE_StorageDeviceTypeSSD;	// 初期値をSSD（eMMC）.
	if( strcmp(psc_Device,theHDDTrans.pc_strDeviceSSD) == 0){
		e_InitDeviceType = THRE_StorageDeviceTypeHDD;
	}else if(strcmp(psc_Device,theHDDTrans.pc_strDeviceSD) != 0){	// HDDでもeMMCでもない.
		SYSF_logMsg( "[checkFsck] Device[%s] unknown.\n", psc_Device );
		return False;
	}
	THRE_StorageDeviceType e_DeviceType = getNvRamDeviceType();	// HDD接続設定値からデバイスタイプ.

	// 先行マウント必要領域のチェック.
	bl_Ret = doFsckAndMount(psc_Device, e_InitDeviceType, e_DeviceType, THRE_OPTIONS_BIT_PRE_MOUNT );
	if( bl_Ret == False ){
		SYSF_logMsg( "[checkFsck] %s doFsckAndMount Pre NG.\n",
			 ((e_InitDeviceType == THRE_StorageDeviceTypeHDD) ? "HDD" : "SSD") );
		return False;
	}
	
	// 通常マウント領域のチェック.
	bl_Ret = doFsckAndMount(psc_Device, e_InitDeviceType, e_DeviceType, THRE_OPTIONS_BIT_NORMAL_MOUNT );
	if( bl_Ret == False ){
		SYSF_logMsg( "[checkFsck] %s doFsckAndMount Nomal NG.\n",
			((e_InitDeviceType == THRE_StorageDeviceTypeHDD) ? "HDD" : "SSD") );
		return False;
	}

	return True;
}

// ================================================================================================.
//! <!--機能--> パーティションチェック処理.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! char* psc_Device	:	デバイス名.
//! <!--戻値-->.
//! \return Bool		:	チェック結果(True:成功、False:失敗).
//! <!--詳細-->.
//! なし.
//! <!--履歴-->.
//  2016/08/25	iyanagi	【IT5_4.0】制御構成変更No45「基板交換低減対応」.
//! ================================================================================================.
Bool THRL_CheckPatition( char* psc_Device )
{
	return theHDDTrans.checkPartition( psc_Device );
}

// ================================================================================================.
//! <!--機能--> FSCK（FAT領域の正当性チェック）処理.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! char* psc_Device	:	デバイス名.
//! <!--戻値-->.
//! \return Bool		:	チェック結果(True:成功、False:失敗).
//! <!--詳細-->.
//! なし.
//! <!--履歴-->.
//  2016/08/25	iyanagi	【IT5_4.0】制御構成変更No45「基板交換低減対応」.
//! ================================================================================================.
Bool THRF_CheckFsck( char* psc_Device )
{
	return theHDDTrans.checkFsck( psc_Device );
}

// ================================================================================================.
//! <!--機能--> 蓄積領域の最大クラスタ数を取得する.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! THRE_FatSector e_FatSector	:	Fatセクタ指定.
//! <!--戻値-->.
//! \return uint32_t		:	最大クラスタ数.
//! <!--詳細-->.
//! HDD上に書き込まれている蓄積領域の最大クラスタ数を保存域となるFatセクタ指定に応じた場所から取得する.
//! 暗号化状態では、複合化して値を抽出する.
//! <!--履歴-->.
//  2016/12/19	Sky井柳  AR1007099【IT5_4.0】新規作成.
//  2017/10/10  Yoneda   S800 ストレージ構成変更対応.
//  2019/08/26 Sky鳥居 [EagleZ+] OP_BTS-11139対応.
//! ================================================================================================.
uint32_t CP_HDDTrans::getStoredMaxClsNum( THRE_FatSector e_FatSector )
{
	uint32_t			ul_StartBlock = 0;	// store用FAT1開始ブロック.//HDD.

	FuncCallstart( 9, __FUNCTION__, 0 );		// 割込みチェック用関数コールスタート.

	if( e_FatSector == THRE_FatSectorFirst ) {	// セクタ指定確認.
		ul_StartBlock = getStorageInfo(THRE_StgInfoSectFatStore1);	// store用FAT1開始ブロック.//HDD.
	} else {
		ul_StartBlock = getStorageInfo(THRE_StgInfoSectFatStore2);	// store用FAT2開始ブロック.//HDD.
	}
	void*			pv_Adder = uc_HDDbuffer;		// HDDバッファのアドレス.
	int64_t			sll_Syscall_Ret = 0;			// システムコールの戻り値判定用.
	int				si_fdp = 0;						// ファイルディスクリプタ.
	uint32_t 		ul_Ret = 0;						// 戻り値(最大クラスタ数).
	uint32_t			ul_Size = DEF_SECTOR_SIZE;

	// HDDデバイスオープン.
	si_fdp = SYSF_devopen( theHDDTrans.pc_strDeviceSSD, O_RDWR | O_CLOEXEC, 0666 );
	if( si_fdp < 0 ){
		printf( "[getStoredMaxClsNum]fail SYSF_devopen. errno:%d\n",errno );
		FuncCallend( 9, __FUNCTION__, 0 );			// 割込みチェック用関数コールエンド.
		return ul_Ret;
	}

	// 最大クラスタ数が書き込まれている位置までシーク.
	sll_Syscall_Ret = lseek( si_fdp, (long long)ul_StartBlock * (long long)DEF_SECTOR_SIZE, SEEK_SET );
	if( sll_Syscall_Ret < 0 ){
		SYSF_devclose(si_fdp);
		printf( "[getStoredMaxClsNum]fail lseek command. fd:%d, StartBlock:%lu, Syscall_Ret:%"PRIx64", errno:%d\n",
			si_fdp,(uint32_t)ul_StartBlock,(int64_t)sll_Syscall_Ret, errno );
		FuncCallend( 9, __FUNCTION__, 0 );			// 割込みチェック用関数コールエンド.
		return ul_Ret;
	}

	// 最大クラスタ数の読み込み.
	sll_Syscall_Ret = read( si_fdp, (char *)pv_Adder ,ul_Size );
	if( sll_Syscall_Ret < 0 ){
		SYSF_devclose(si_fdp);
		printf( "[getStoredMaxClsNum]fail read command. fd:%d, Adder:%08lx, Syscall_Ret:%"PRIx64", errno:%d",
			si_fdp, (uintptr_t)pv_Adder, (int64_t)sll_Syscall_Ret, errno );
		FuncCallend( 9, __FUNCTION__, 0 );			// 割込みチェック用関数コールエンド.
		return ul_Ret;
	}
	SYSF_devclose(si_fdp);	// デバイスクローズ.

	if( e_FatSector == THRE_FatSectorFirst ) {	// セクタ指定確認.
		//D030対応(読み出したデータのバックアップ（先頭4byteが最大クラスタ数)
		memcpy(&ul_BackupMaxCluster1st, pv_Adder, 4 );
		printf( "[Debug] Backup getStoredMaxClsNum 1=%lu\n",ul_BackupMaxCluster1st );
	} else {
		//D030対応(読み出したデータのバックアップ（先頭4byteが最大クラスタ数)
		memcpy(&ul_BackupMaxCluster2nd, pv_Adder, 4 );
		printf( "[Debug] Backup getStoredMaxClsNum 2=%lu\n",ul_BackupMaxCluster2nd );
	}

	if( isEnableEnc() == True ){// 暗号化HDD.

		FuncCallstart( 30, __FUNCTION__, 1 );		// 割込みチェック用関数コールスタート.
		// 複合化が必要.
		uintptr_t ul_WorkMemAddr = getWorkMemAddr();
		uintptr_t ul_AESBufAddr = ul_WorkMemAddr + DEF_SECTOR_SIZE;
	 	// 先頭奇数要求バイトバッファから先頭偶数バイトAESバッファにコピー　
	 	memcpy( (void *)ul_WorkMemAddr, pv_Adder, ul_Size );
	 	
		THRC_AesTransSingletonIf::newInstance().encodeDecodeExec( THRE_EncHDDDosBufToMem, 0, (uintptr_t)ul_AESBufAddr, ul_WorkMemAddr, ul_Size );
		// 戻り値の設定.
		memcpy( &ul_Ret, (void *)ul_AESBufAddr, 4 );
		printf( "[Debug][ENC] ul_AESBufAddr=%08X, ul_Ret=%lu\n",ul_AESBufAddr, ul_Ret);
		FuncCallend( 30, __FUNCTION__, 1 );			// 割込みチェック用関数コールエンド.
	
	}else{ // 非暗号化.
		// 戻り値の設定.
		memcpy(&ul_Ret, pv_Adder, 4 );
		printf( "[Debug][NON] pv_Adder=%08X, ul_Ret=%lu\n",pv_Adder, ul_Ret);
	}

	FuncCallend( 9, __FUNCTION__, 0 );			// 割込みチェック用関数コールエンド.

	return ul_Ret;
	
}

// ================================================================================================.
//! <!--機能--> 蓄積領域の最大クラスタ数を取得する.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! なし.
//! <!--戻値-->.
//! \return uint32_t		:	最大クラスタ数.
//! <!--詳細-->.
//! HDD上に書き込まれている蓄積領域の最大クラスタ数を取得する.
//! ※※※※※※※※※※※※※※※※※※※※※※※※※※※※※※※※※※※.
//! 他の処理との競合を考慮して、むやみに使用しないこと.
//! ※ 実装時は、起動時と論理フォーマット時のみに使用される.
//!   void IMGC_ImageMemoryPoolImpColorFileMemory::initInternalFat()でのみ使用している.
//! ※※※※※※※※※※※※※※※※※※※※※※※※※※※※※※※※※※※.
//! <!--履歴-->.
//  2016/11/16	Sky井柳  AR1005005【IT5_4.0】新規作成.
//  2016/12/19	Sky井柳  AR1007099【IT5_4.0】2面管理されている2箇所から値を抽出して確認するように修正.
//  2019/08/26	Sky鳥居  [EagleZ+] OP_BTS-11139対応 リトライ処理を追加.
//! ================================================================================================.
uint32_t CP_HDDTrans::getStoredMaxClsNum()
{
	uint32_t ul_StoredMaxClsNum1 = 0;								// 1面目の最大クラスタ数.
	uint32_t ul_StoredMaxClsNum2 = 0;								// 2面目の最大クラスタ数.
	uint32_t ul_StoredClsMaxNum = theHDDTrans.getStorageInfo(THRE_StgInfoStoreClsNum); 			// 汎用フォーマット時の最大クラスタ数.
	uint32_t ul_StoredClsMaxNum_KM = theHDDTrans.getStorageInfo(THRE_StgInfoStoreClsNum_KM);	// KMフォーマット時の最大クラスタ数.

	uint32_t ul_Ret = 0;											// 戻り値(最大クラスタ数).
	int32_t si_RetryCount = 0;										// Retry用カウンタ.
	for( si_RetryCount = 0; si_RetryCount <= THRD_MaxClsNumRetryMax; si_RetryCount++ ){
		ul_StoredMaxClsNum1 = getStoredMaxClsNum( THRE_FatSectorFirst  );					// 1面目の最大クラスタ数.
		ul_StoredMaxClsNum2 = getStoredMaxClsNum( THRE_FatSectorSecond  );					// 2面目の最大クラスタ数.

		if( ( ul_StoredMaxClsNum1 == ul_StoredClsMaxNum )||			// 汎用フォーマット時.
			( ul_StoredMaxClsNum1 == ul_StoredClsMaxNum_KM ) ){		// KMフォーマット時.
			ul_Ret = ul_StoredMaxClsNum1;	// 1面目の結果を設定.
			break;
		}else
		if( ( ul_StoredMaxClsNum2 == ul_StoredClsMaxNum )||			// 汎用フォーマット時.
			( ul_StoredMaxClsNum2 == ul_StoredClsMaxNum_KM ) ){		// KMフォーマット時.
			ul_Ret = ul_StoredMaxClsNum2;	// 2面目の結果を設定.
			break;
		}else{
			if( THRD_MaxClsNumRetryMax <= si_RetryCount ){
				// リトライ最大数失敗時はログ出力.
				// 共に数値がおかしい.
				SYSF_logMsg("[%s]:Error\n",__FUNCTION__);
				SYSF_logMsg("[%s]:ul_StoredMaxClsNum1  [%lu]\n",__FUNCTION__, ul_StoredMaxClsNum1);
				SYSF_logMsg("[%s]:ul_StoredMaxClsNum2  [%lu]\n",__FUNCTION__, ul_StoredMaxClsNum2);
				SYSF_logMsg("[%s]:StgInfoStoreClsNum   [%lu]\n",__FUNCTION__, ul_StoredClsMaxNum);
				SYSF_logMsg("[%s]:StgInfoStoreClsNum_KM[%lu]\n",__FUNCTION__, ul_StoredClsMaxNum_KM);
			}else{
				// リトライ時のインターバル 200msec.
				delayTask( 200 );
			}
		}
	}
	// リトライ発生時はログを出力.
	if( 0 < si_RetryCount ){
		SYSF_logMsg("[getStoredMaxClsNum]:Retry[%d] IntErrCnt[%d] Int[%d][%d] OldInt[%d][%d]\n", si_RetryCount,
			 m_uc_FuncCallIntErrCnt[0], m_uc_FuncCallPointIntErr[0][0], m_uc_FuncCallPointIntErr[0][1], m_uc_FuncCallPointIntErr[0][2], m_uc_FuncCallPointIntErr[0][3] );
	}
	return ul_Ret;
}
// ================================================================================================.
//! <!--機能--> HDDリストアパラメータの設定.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! const THRS_HddRestoreParam& HDDリストアパラメータ.
//! <!--戻値-->.
//! なし.
//! <!--詳細-->.
//! HDDリストアパラメータの設定を内部保持する.
//! <!--履歴-->.
//  2017/01/05 ishiga [IT5_4.1][HDDデータバックアップ世代間互換].
//! ================================================================================================.
void CP_HDDTrans::setHddRestoreParam( const THRS_HddRestoreParam& st_Param )
{
	THRG_SetHDDRestoreLog( 20, st_Param.e_BackupDataVer );
	m_st_HddRestoreParam = st_Param;
}

// ================================================================================================.
//! <!--機能--> HDDリストアパラメータの取得.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! なし.
//! <!--戻値-->.
//! const THRS_HddRestoreParam& HDDリストアパラメータ.
//! <!--詳細-->.
//! 内部保持しているHDDリストアパラメータの設定を返却する.
//! パラメータ未設定時は、0初期化されている.
//! <!--履歴-->.
//  2017/01/05 ishiga [IT5_4.1][HDDデータバックアップ世代間互換].
//! ================================================================================================.
const THRS_HddRestoreParam& CP_HDDTrans::getHddRestoreParam()
{
	return m_st_HddRestoreParam;
}

// ================================================================================================.
//! <!--機能--> HDDリストア時にエンディアン変換が必要であるか.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! const THRS_HddRestoreParam& HDDリストアパラメータ.
//! <!--戻値-->.
//! True:必要 , False:不要.
//! <!--詳細-->.
//! Hddリストア時にエンディアン変換が必要であるか返却する.
//! 現状は、IT4であるかと同等.
//! <!--履歴-->.
//  2017/01/05 ishiga [IT5_4.1][HDDデータバックアップ世代間互換].
//! ================================================================================================.
Bool CP_HDDTrans::isNeedConvertEndian_HddRestore()
{
	return isIT4Ver_HddRestore();
}

// ================================================================================================.
//! <!--機能--> HDDリストア時にエンディアン変換が必要であるか.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! const THRS_HddRestoreParam& HDDリストアパラメータ.
//! <!--戻値-->.
//! True:必要 , False:不要.
//! <!--詳細-->.
//! HDDリストアバージョンがIT4であるかチェックする.
//! <!--履歴-->.
//  2017/01/05 ishiga [IT5_4.1][HDDデータバックアップ世代間互換].
//! ================================================================================================.
Bool CP_HDDTrans::isIT4Ver_HddRestore()
{
	TYPE_FWVerInfo e_RestoreDataVer = getHddRestoreParam().e_BackupDataVer;		// データのバージョン.
	if ( TYPE_FWVerInfo_IT4_3rd <= e_RestoreDataVer && e_RestoreDataVer <= TYPE_FWVerInfo_IT4_5th ) {
		return True;
	} else {
		return False;
	}
}

// ================================================================================================.
//! <!--機能--> TCG OPALアクティベーション処理.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! char* pc_Device	:	デバイス名.
//! <!--戻値-->.
//! \return int32_t	:	0:OK / -1:NG.
//! <!--詳細-->.
//! なし.
//! <!--履歴-->.
//  2017/10/10	yoneda	新規:S800 デバイス構成変更対応.
//! ================================================================================================.
int32_t CP_HDDTrans::doSedOpal( char* pc_Device )
{
    char c_Cmd[256];
    int32_t i_Ret;
    

    // SSDの全消去(Revert)
    snprintf( c_Cmd, sizeof(c_Cmd), SED_OPAL_REVERT, pc_Device );
    SYSF_logMsg("[%s] execute->[%s]\n", __FUNCTION__, c_Cmd );
    i_Ret = execprog( c_Cmd );
    if( i_Ret ){
        // 新品の状態からこのコマンドを実行するとエラーになるが問題ないためコメント出力のみとする
        printf( "execprog ERR: cmd:%s\n", c_Cmd );
    }
    sync();

    // ストレージデバイスの所有権の取得
    snprintf( c_Cmd, sizeof(c_Cmd), SED_OPAL_OWNER, pc_Device );
    SYSF_logMsg("[%s] execute->[%s]\n", __FUNCTION__, c_Cmd );
    i_Ret = execprog( c_Cmd );
    if( i_Ret ){
        printf( "execprog ERR: cmd:%s\n", c_Cmd );
        return -1;
    }
    sync();

    // ロック機能のアクティベーション
    snprintf( c_Cmd, sizeof(c_Cmd), SED_OPAL_ACTIVATE, pc_Device );
    SYSF_logMsg("[%s] execute->[%s]\n", __FUNCTION__, c_Cmd );
    i_Ret = execprog( c_Cmd );
    if( i_Ret ){
        printf( "execprog ERR: cmd:%s\n", c_Cmd );
        return -1;
    }
    sync();

    // ロック範囲の設定
    // 範囲のサイズはパーティション割り当て合計サイズを指定する
    snprintf( c_Cmd, sizeof(c_Cmd), SED_OPAL_SETUP, pc_Device, getHddFixedSectorNum() );
    SYSF_logMsg("[%s] execute->[%s]\n", __FUNCTION__, c_Cmd );
    i_Ret = execprog( c_Cmd );
    if( i_Ret ){
        printf( "execprog ERR: cmd:%s\n", c_Cmd );
        return -1;
    }
    sync();

    return 0;
}

// ================================================================================================.
//! <!--機能--> 起動デバイスタイプ取得.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! なし.
//! <!--戻値-->.
//! \return THRE_StartupDeviceType	:	起動デバイスタイプ.
//! <!--詳細-->.
//! なし.
//! <!--履歴-->.
//  2017/10/10	yoneda	新規:S800 デバイス構成変更対応.
//! ================================================================================================.
#ifdef FUM_LINUX
int64_t CP_HDDTrans::GetSimStorageSize()
{
	static const char sector_num_file[] = "/sys/block/sdc/size";
	static const char sector_size_file[] = "/sys/block/sdc/queue/logical_block_size"; // hw_sector_size,physical_block_sizeなどもあるが、いずれも512
	static int64_t storageSize = -1;
	
	if (storageSize != -1) {
		return storageSize;
	}
	
	// この時点では、theHDDTrans.getPhysicalSubDeviceSize();は使用できない。
	int64_t sector_num;
	int64_t sector_size;
	char read_buf[20+1];	// 1T sector_num  2G(10桁) + '\0' + α

	int fd = open(sector_num_file, O_RDONLY);
	if ( fd == -1 )
	{
		printf("%s open failed.errno = %d\n", sector_num_file, errno);
		return -1;
	}
	ssize_t read_result = read(fd, read_buf, sizeof(read_buf));
	if(read_result == -1)
	{
		printf("%s read failed.errno = %d\n", sector_num_file, errno);
		close(fd);
		return -1;
	}
	read_buf[read_result] = '\0';
	sector_num = atol(read_buf);
	close(fd);
	

	fd = open(sector_size_file, O_RDONLY);
	if ( fd == -1 )
	{
		printf("%s open failed.errno = %d\n", sector_size_file, errno);
		return -1;
	}
	read_result = read(fd, read_buf, sizeof(read_buf));
	if(read_result == -1)
	{
        close(fd);
		printf("%s read failed.errno = %d\n", sector_size_file, errno);
		return -1;
	}
	read_buf[read_result] = '\0';
	sector_size = atol(read_buf);
	close(fd);

	storageSize = sector_num * sector_size;

	printf("SIM storage size = %" PRId64 "\n", storageSize);
	return storageSize;

}
static const int64_t MB = 1024 * 1024;
static const int64_t GB = 1024 * MB;
static const int64_t TB = 1024 * GB;	
THRE_StartupDeviceType THRF_GetStartupDeviceType(void)
{
	static THRE_StartupDeviceType deviceType = THRE_StartupDeviceType_SSD;	//後ろのswitchのdefault節と同じ値で初期化しておく。
	static Bool firstTime = True;
	
	if (firstTime) {
		int64_t storageSize = CP_HDDTrans::GetSimStorageSize();
		switch(storageSize) {
		case 1 * TB:	// fall through
		case 256 * GB:
			deviceType = THRE_StartupDeviceType_SSD;
			break;
		
		case 8 * GB:	// fall througn
		case 16 * GB:
			deviceType = THRE_StartupDeviceType_SD;
			break;
		default:
			deviceType = THRE_StartupDeviceType_SSD;
			//Emu800 Sim向けに提供しているvdiを装着する限りは、ここを通ることはない。
			printf("SIM unknown storage size = %" PRId64 "\n", storageSize);
		}
		firstTime = False;
	}
	return deviceType;
}
#else /* #ifdef FUM_LINUX */
THRE_StartupDeviceType THRF_GetStartupDeviceType(void)
{
	struct stat    st_Stat; 
	if( -1 == SYSF_stat( THRD_DEVTYPE_PATH, &st_Stat ) ) {
		return THRE_StartupDeviceType_SSD;      // typeが存在しない為、SSDと判断.
	}else{
		return THRE_StartupDeviceType_SD;       // typeが存在する為、SDと判断.
	}
}
#endif /* #ifdef FUM_LINUX */
/*================================================================================================
［機能］	INDEXファイル署名検証処理.
--------------------------------------------------------------------------------------------------
［引数］	psc_FwPath:	更新ファイルパス.
			psc_SigPath:署名ファイルパス.
［戻値］	True:検証成功 False:検証失敗.
［詳細］	ダウンロードROMの署名検証.
［履歴］	2014/10/22 motohashi [SecV1-3]ダウンロードROMの署名検証.
			2014/12/02 motohashi [SecV1-3]エンジン類のINDEX署名検証を追加.
			2017/06/12 Shibahara [SecV2-2]ファームウェア更新時の真正性検証技術.
================================================================================================*/
static Bool verifySignature( const char *psc_FwPath, const char *psc_SigPath )
{
	const char* const 	psc_CertFile	= "/mnt/rsapubkm.crt";
	THRS_VERIFY_FILE	s_VerifyFile	= { psc_FwPath, psc_SigPath };
	char sc_Cmd[256] = {0};
	int32_t i_Ret = 0;

	// 引数チェック.
	if( !(psc_FwPath) || !(psc_SigPath) ){
        THRL_ThLogMsgErr( "Input parameter error.\n" );
        return False;
    }

	// 証明書展開領域マウント.
	snprintf( sc_Cmd, sizeof(sc_Cmd), FMT_PART_RAMDISK, "/mnt", "2048");
	i_Ret = system( sc_Cmd );
	if( !WIFEXITED( i_Ret ) || 0 != WEXITSTATUS( i_Ret ) ){
       THRL_ThLogMsgErr( "system error. ramdisk mount failed.\n" );
       return False;
    }

	// 証明書ロード.
	if ( !(loadCertificate( psc_CertFile )) ){
		THRL_ThLogMsgErr("Load certificate error.\n");
    	umount2("/mnt", MNT_FORCE);
		return False;
	}

	// 署名検証.
	if (THRD_VERIFY_ERR_NONE != THRL_VerifySignatureFile( psc_CertFile, &s_VerifyFile )) {
		THRL_ThLogMsgErr("Verify signature error.\n");
		remove( psc_CertFile );
    	umount2("/mnt", MNT_FORCE);
		return False;
	}

	// 証明書削除.
	remove( psc_CertFile );

	// 証明書展開領域アンマウント.
    umount2("/mnt", MNT_FORCE);

	return True;
}

/*================================================================================================
［機能］	FW検証用証明書ロード処理.
--------------------------------------------------------------------------------------------------
［引数］	psc_CertPath:	ロード先のパス.
［戻値］	True:ロード成功 False:ロード失敗.
［詳細］	FW検証用証明書のロード処理.
［履歴］	2017/06/15 Shibahara [SecV2-2]ファームウェア更新時の真正性検証技術.
================================================================================================*/
static Bool loadCertificate( const char *psc_CertPath )
{
	int32_t i_Ret = 0;
	char sc_Cmd[256] = {0};

	if( !psc_CertPath ){
        THRL_ThLogMsgErr( "Input parameter error.\n" );
		return False;
	}

	snprintf(sc_Cmd, sizeof(sc_Cmd), "exec cp /boot/km.crt %s", psc_CertPath );
	i_Ret = system( sc_Cmd );
	if( !WIFEXITED( i_Ret ) || 0 != WEXITSTATUS( i_Ret ) ){
		THRL_ThLogMsgErr( "system error. loading certificate failed.\n" );
		return False;
	}

	if( THRD_CheckCertTimeResultCurrent != THRL_CheckCertificateTime( psc_CertPath ) ){
		THRL_ThLogMsgErr( "certificate time invalid.\n" );
		return False;
	}

	return True;
}

/*================================================================================================
［機能］	暴露保護状態を取得
--------------------------------------------------------------------------------------------------
［引数］	無し.
［戻値］	Bool
［詳細］	Flash上の暴露保護フラグを参照し、フラグが全て0なら暴露保護なし、フラグが立っていれば暴露保護ありを返す.
 [履歴]     2014/11/5 大嶋   cPP暴露保護対応
			2015/10/23 中村   [SecV2-4]SecV1時の対応をマージ.
			2017/11/09	SCD2-Sky Shindo [WP.53-54] 設定値見直し対応.
			2018/01/24	SCD2-Sky ohshima [WP.71] nvdマウント前に呼ばれる為、check_secで設定値を取得する.
			2020/08/08  ADS美土路 SimLin用の暗号化判定処理を追加.(check_secコマンドが使用できないため代替処理)
================================================================================================*/
Bool THRG_IsSsdEncrypt(void)
{
#ifdef DEF_SupportSSD
#ifdef FUM_LINUX
    //SimLinでは、cyheck_secコマンドが無いため、/sim/encrypt_doneの有無で暗号化済みの判断をする。
	struct stat s;
	static const char encrypt_done_file[] = "/sim/encrypt_done";
	int result = stat(encrypt_done_file, &s);
	return (result == 0)? True:False;	//ファイルがあれば暗号化済み。
#else	// !FUM_LINUX
	// NSバックアップがメインストレージに存在する為、ストレージマウント後のみNVDCクラスを利用可能.
	// 本関数はメインストレージマウント前に必要となる為、NVDクラスではなくcheck_secコマンドで情報を取得する.
	static Bool b_Get = False;
	static int i_Enc = 0;

	if (!b_Get) {
		char sc_Cmd[128] = {0};
		snprintf( sc_Cmd, sizeof(sc_Cmd), THRD_CMD_IS_MAINSTR_ENC);
		i_Enc = system( sc_Cmd );
		if (!WIFEXITED(i_Enc)) {
	       THRL_ThLogMsgErr( "system error. check mainstr enc.\n" );
	       return False;
	    }
	    i_Enc = WEXITSTATUS(i_Enc);
	    b_Get = True;
	}
	
	if (THRD_IS_MAINSTR_ENC == i_Enc) {
		return True;
	}
	
	return False;
#endif // !FUM_LINUX

#else /* DEF_SupportSSD */
	return False;
#endif /* DEF_SupportSSD */
}

// ================================================================================================.
//［機能］	メインストレージ暗号化デバイス作成処理.
//--------------------------------------------------------------------------------------------------.
//［引数］	なし.
//［戻値］	Bool	：True:処理成功	：Fasle:処理失敗.
//［履歴］	2019/05/10 Sky井柳  【IT6_1.2MR】[AR.1076986]「暗号化とロックの併用対応」新規作成.
//================================================================================================*/
Bool THRL_makeMainStorageEncDevice(void)
{
	// 暗号化中ではない時は、処理せずに正常終了.
	if ( THRG_IsSsdEncrypt() == False ) {
		THRL_ThLogMsgErr( "[%s] Non Encrypt.\n", __FUNCTION__ );
		return True;
	}
	
	int si_Ret = 0;
	
	// "/var/crypt"作成
	si_Ret = SYSF_mkdir( THRD_Encrypt_KeyDir );
	if( si_Ret == ERROR ) {
		THRL_ThLogMsgErr( "[%s] make directory[%s] error(ret:%d).\n", __FUNCTION__, THRD_Encrypt_KeyDir, si_Ret );
		return False;
	}
	
	// RAMディスク作成.
	si_Ret = THRF_System( THRD_Encrypt_KeyDirRamDiskMountCmd );
	if (si_Ret) { // 0以外はエラー.
		THRL_ThLogMsgErr( "[%s] make RAM disk[%s] error(%s, errno:%d).\n", __FUNCTION__, THRD_Encrypt_KeyDir, si_Ret, errno );
		return False;
	}
	THRF_DoSync();
	
	// 生成処理.
	si_Ret = THRF_System( THRD_Encrypt_EncDevMakeCmd );
	if (si_Ret) { // 0以外はエラー.
		THRL_ThLogMsgErr( "[%s] make EncDevice(cmd:\"%s\") error(ret:%d, errno:%d).\n", __FUNCTION__, THRD_Encrypt_EncDevMakeCmd, si_Ret, errno );
		return False;
	}
	THRF_DoSync();
	
	// RAMディスク解放.
	si_Ret = THRF_System( THRD_Encrypt_KeyDirRamDiskUnmountCmd );
	if (si_Ret) { // 0以外はエラー.
		THRL_ThLogMsgErr( "[%s] umount RAM disk[%s] error(ret:%d, errno:%d).\n", __FUNCTION__, THRD_Encrypt_KeyDir, si_Ret, errno );
		return False;
	}
	
	// "/var/crypt"削除.
	rmdir( THRD_Encrypt_KeyDir );
	
	return True;
}

/*================================================================================================
［機能］	暗号化ブロックデバイスの生成／破棄
--------------------------------------------------------------------------------------------------
［引数］	b_Protect：True:暴露保護ON / False：暴露保護OFF
			psc_Password：暗号化パスワード文字列へのポインタ
［戻値］	short：0：成功 / その他：失敗
［詳細］	bl_EncryptがTrueなら、パーティションの暗号化フラグでss_Flgビットが立っている領域の暗号化デバイスブロックを生成.
			bl_EncryptがFalseなら、パーティションの暗号化フラグでss_Flgビットが立っている領域の暗号化デバイスブロックを破棄.
 [履歴]     2014/11/5 大嶋   cPP暴露保護対応
			2015/10/23 中村   [SecV2-4]SecV1時の対応をマージ.
			2016/02/10 中村   CQ524977[SecV2]メインストレージの暗号化設定が機能しない."/res"mount状態で暗号化/復号化処理してしまう問題.
			2016/03/22 中村   CQ.525797:SSD暗号化処理中のアクセス抑止.
			2016/03/28 中村   CQ.526580:2回目でメインストレージ暗号化を設定して、MFPが正常に起動できない。Snapshot再取得が必要.
			2016/05/06 中村   CQ.528001:セマフォ確保を継続することでKernelにデッドロック検知され、Kernelパニックしてしまう問題対策.
			2016/05/20 中村   CQ.528585:IISWアプリから強制更新するとアプリ側ROM更新状態が「Active」のままとなる。メインストレージ暗号化をしていない状態で、セマフォタイムアウトが起きている問題対策.
			2016/06/10 中村   CQ.530467:PKIモード時メインストレージ暗号化設定に失敗する.
			2016/05/09 河西   SecV2-9: 暗号鍵インポート対応. パスワードから鍵を生成せず、予め設定された鍵情報を使って暗号化実行する処理を追加.
			2017/11/09	SCD2-Sky Shindo [WP.53-54] 設定値見直し対応.
			2018/01/24	SCD2-Sky ohshima [WP.67] 起動シーケンスにて暗号／復号化する為、リクエスト状態のみ保存する.
            2020/12/14 Tsukiji OP_BTS-29664 FWエラーが発生する問題対応(暫定)
================================================================================================*/
short CP_HDDTrans::setProtectExpose(Bool b_Protect, char* psc_Password, TYPS_StorageEncryptionKeyInfo* ps_KeyInfo)
{
#ifdef DEF_SupportSSD
	THRC_VerifyUserCtrl *pc_VerifyUserCtrl = NULL;
	THRC_Crypto *pc_Crypto = NULL;

	/* 暗号化OFF→ONでパスワードも鍵材料もないなら何もしない */
	if(( b_Protect ) && (NULL == psc_Password) && (NULL == ps_KeyInfo)){
		printf("%s(%d)error param.\n", __FUNCTION__, __LINE__);
		return 1;
	}

	pc_VerifyUserCtrl = THRC_VerifyUserCtrl::newInstance();
	if( NULL == pc_VerifyUserCtrl ) {
		printf( "%s(%d)Error, failed THRC_VerifyUserCtrl::newInstance().\n", __FUNCTION__, __LINE__ );
		return 1;
	}

	pc_Crypto = THRC_Crypto::newInstance();
	if( NULL == pc_Crypto ) {
		printf( "%s(%d)Error, failed THRC_Crypto::newInstance().\n", __FUNCTION__, __LINE__ );
		return 1;
	}
	// 2020/12/14 Tsukiji OP_BTS-29664 FWエラーが発生する問題対応(暫定)
	APIC_SoftDipSw* pc_SoftDipSw = APIC_SoftDipSw::newInstance();	// DipSw（API）・インスタンス取得.
	uchar uc_SWvalue = pc_SoftDipSw->getSoftDipSwBit( 16, 1 );		// DipSWCommonAPI

	// スナップショットの再取得が必要な事をブートローダーに通知.
	setWarpInfo_ChgMFPData(True);
	
	// 「一般ユーザーアクセス変換管理ファイル」が存在する場合削除する.
	if (0 == access(THRD_CtrlFile, F_OK)) {
		if (0 != remove(THRD_CtrlFile)) {
			printf( "%s(%d)Error, failed remove(%s).\n", __FUNCTION__, __LINE__, THRD_CtrlFile);
			return 1;
		}
	}
	
	APIC_SystemSetting *pc_SecuritySystemSetting = NULL;
	pc_SecuritySystemSetting = APIC_SystemSetting::newInstance();
	if( NULL == pc_SecuritySystemSetting ) {
		printf( "%s(%d)Error, failed APIC_SystemSetting::newInstance().\n", __FUNCTION__, __LINE__ );
		return 1;
	}

	if ( b_Protect ) {
		if( b_Protect == THRG_IsSsdEncrypt() ){
			/* 鍵材料更新 */
			if(NULL == psc_Password) {
				if( 0 != impKeyMaterial_updNvMemory( THRE_StorageDeviceTypeSSD, ps_KeyInfo, True ) ){
					printf( "%s(%d)Error, failed impKeyMaterial_updNvMemory().\n", __FUNCTION__, __LINE__ );
					return 1;
				}
			}
			else {
				if( 0 != updKeyMaterial_updNvMemory( THRE_StorageDeviceTypeSSD, psc_Password ) ){
					printf( "%s(%d)Error, failed updKeyMaterial_updNvMemory().\n", __FUNCTION__, __LINE__ );
					return 1;
				}
#ifdef FUM_LINUX	//暗号化ワード変更
				system("touch /sim/new_password");
#endif
			}

			/* SSD交換確認用のファイル更新 */
			if( 0 > pc_Crypto->updSsdCryptData() ){
				printf( "%s(%d)Error, failed THRC_Crypto::updSsdCryptData().\n", __FUNCTION__, __LINE__ );
				return 1;
			}
		}
		else {
			//2020/12/14 Tsukiji OP_BTS-29664 FWエラーが発生する問題対応(暫定)
			// メインストレージ暗号化解除と鍵材料破棄は、RFSにてメインストレージ復号化後に実施.
			BackupSoftSW( uc_SWvalue );

			printf( "[%s] enc Software DIPSW 19-bit0 = [%d]=>1\n",__FUNCTION__,  uc_SWvalue );
			pc_SoftDipSw->setSoftDipSwBit(16,1,True);

			/* 鍵材料生成 */
			if(NULL == psc_Password) {
				if( 0 != impKeyMaterial_updNvMemory( THRE_StorageDeviceTypeSSD, ps_KeyInfo, False ) ){
					printf( "%s(%d)Error, failed impcreKeyMaterial_updNvMemory().\n", __FUNCTION__, __LINE__ );
					return 1;
				}
			}
			else {
				if( 0 != creKeyMaterial_updNvMemory( THRE_StorageDeviceTypeSSD, psc_Password ) ){
					printf( "%s(%d)Error, failed creKeyMaterial_updNvMemory().\n", __FUNCTION__, __LINE__ );
					return 1;
				}
#if FUM_LINUX	//暗号化設定
				system("touch /sim/password");
#endif
			}

			pc_SecuritySystemSetting->reqMainStorageEnc(True);

			/* SSD交換確認用のファイル更新 */
			if( 0 > pc_Crypto->updSsdCryptData() ){
				printf( "%s(%d)Error, failed THRC_Crypto::updSsdCryptData().\n", __FUNCTION__, __LINE__ );
				return 1;
			}
		}
	} else {
#ifdef FUM_LINUX	// 暗号化解除設定
		system("touch /sim/decrypt");	
#endif
		//2020/12/14 Tsukiji OP_BTS-29664 FWエラーが発生する問題対応(暫定)
		BackupSoftSW( uc_SWvalue );

		printf( "[%s] noenc Software DIPSW 19-bit0 = [%d]=>1\n",__FUNCTION__,  uc_SWvalue );
		pc_SoftDipSw->setSoftDipSwBit(16,1,True);
		pc_SecuritySystemSetting->reqMainStorageEnc(False);
		// メインストレージ暗号化解除と鍵材料破棄は、RFSにてメインストレージ復号化後に実施.
	}

	return 0;
#else /* DEF_SupportSSD */
	return 1;
#endif /* DEF_SupportSSD */
}

/*================================================================================================
［機能］	鍵材料の作成と不揮発メモリ更新.
--------------------------------------------------------------------------------------------------
［引数］	uc_strage		ストレージ種別、THRE_StorageDeviceTypeHDD、THRE_StorageDeviceTypeSSDを期待.
		 	psc_Password	鍵生成用に指定されたパスワード.
［戻値］	short：0：成功 / その他：失敗.
［詳細］	対象ストレージ暗号化用の鍵材料（KEKサブマスクA、KEKサブマスクB、KEK[DEK]）を生成し、不揮発メモリ(Flash)を更新する.
［機能］	パスワード更新：SubmaskA・SubmaskB・KEK[DEK]生成、および、Flash更新.
 [履歴]     2015/10/23 中村   [SecV2-4]新規作成.
  			2016/02/04 中村   CQ524977[SecV2]メインストレージの暗号化設定が機能しない.KEK[DEK]データサイズの見直し対応.
			2016/03/11 中村   CQ.525797[SecV2-4]メインストレージ暗号化を変更すると、FW異常メッセージ提示され、デバイス利用不可.
			2017/11/09	SCD2-Sky Shindo [WP.53-54] 設定値見直し対応.
================================================================================================*/
short CP_HDDTrans::creKeyMaterial_updNvMemory(uchar uc_strage, const char* psc_Password)
{
	APIC_SystemSetting *pc_SecuritySystemSetting = NULL;
	THRC_Crypto *pc_Crypto = NULL;
	uchar uc_maskA[NVDC_KeyLen] = {0};
	uchar uc_maskB[NVDC_KeyLen] = {0};
	uchar uc_KEKDEK[NVDC_KEKDEKLen] = {0};
	uchar uc_Keys[NVDC_KeyLen + NVDC_KeyLen + NVDC_KEKDEKLen] = {0};
	TYPE_FlashKeyType enm_Key;
// 2016/04/11 CQ526818 ストレージ暗号化パスワードのハッシュ化対応 Sky大嶋 Start.
	TYPE_PswdInfo e_InfoType = TYPE_PswdInfo_None;
	TYPS_PasswordInfo passwordInfo;
// 2016/04/11 CQ526818 ストレージ暗号化パスワードのハッシュ化対応 Sky大嶋 End.

	if( THRE_StorageDeviceTypeHDD == uc_strage ){
		enm_Key = TYPE_FlashKeyType_HDD;
// 2016/04/11 CQ526818 ストレージ暗号化パスワードのハッシュ化対応 Sky大嶋 Start.
		e_InfoType = TYPE_PswdInfo_HDD;
// 2016/04/11 CQ526818 ストレージ暗号化パスワードのハッシュ化対応 Sky大嶋 End.
	}
	else if( THRE_StorageDeviceTypeSSD == uc_strage ){
		enm_Key = TYPE_FlashKeyType_SSD;
// 2016/04/11 CQ526818 ストレージ暗号化パスワードのハッシュ化対応 Sky大嶋 Start.
		e_InfoType = TYPE_PswdInfo_SSD;
// 2016/04/11 CQ526818 ストレージ暗号化パスワードのハッシュ化対応 Sky大嶋 End.
	}
	else {
		printf("%s(%d)error param.\n", __FUNCTION__, __LINE__);
		return -1;
	}


	if(NULL == psc_Password){
		printf("%s(%d)error param.\n", __FUNCTION__, __LINE__);
		return -1;
	}

	pc_SecuritySystemSetting = APIC_SystemSetting::newInstance();
	if( NULL == pc_SecuritySystemSetting ) {
		printf( "%s(%d)Error, failed APIC_SystemSetting::newInstance().\n", __FUNCTION__, __LINE__ );
		return -1;
	}

	pc_Crypto = THRC_Crypto::newInstance();
	if( NULL == pc_Crypto ) {
		printf( "%s(%d)Error, failed THRC_Crypto::newInstance().\n", __FUNCTION__, __LINE__ );
		return -1;
	}

	/* KEKサブマスクA、KEKサブマスクB、KEK[DEK]を生成する */
	if( 0 != pc_Crypto->creMaskA_MaskB_KEKDEK( psc_Password, uc_maskA, uc_maskB, NVDC_KeyLen, uc_KEKDEK, NVDC_KEKDEKLen ) ){
		printf( "%s(%d)Error, failed creMaskA_MaskB_KEKDEK().\n", __FUNCTION__, __LINE__ );
		return 1;
	}

	memcpy(uc_Keys, uc_maskA,  sizeof(uc_maskA) );
	memcpy(&uc_Keys[sizeof(uc_maskA)], uc_maskB,  sizeof(uc_maskB) );
	memcpy(&uc_Keys[sizeof(uc_maskA) + sizeof(uc_maskB)], uc_KEKDEK, sizeof(uc_KEKDEK));
// 2016/04/11 CQ526818 ストレージ暗号化パスワードのハッシュ化対応 Sky大嶋 Start.
	// HCD-PPのPBKDFは、バイト数ではなく文字数が64文字以上である事を要求する.
	// そのため、マルチバイトコードで何文字のパスワードが設定されたかを保存する.
	const char* psc_PasswordTemp = psc_Password;
	uchar uc_CharSize = 0;
	while('\0' != *psc_PasswordTemp){
		uc_CharSize = LIBF_getUTF8ByteCount(psc_PasswordTemp);
		
		// Byte数が不明の場合、文字無しとみなし、文字列終端として処理する.
		if(0 == uc_CharSize){
			break;
		}
		++passwordInfo.us_Len;
		psc_PasswordTemp += uc_CharSize;
	}
	KM_SHA256_digest(reinterpret_cast<const uchar*>(psc_Password), strlen(reinterpret_cast<const char*>(psc_Password)), passwordInfo.uc_HashBuff);
// 2016/04/11 CQ526818 ストレージ暗号化パスワードのハッシュ化対応 Sky大嶋 End.
	
	/* KEKサブマスクA、KEKサブマスクB、KEK[DEK]をFlashへ保存する */
	if( False == pc_SecuritySystemSetting->setKeyInfo( enm_Key, uc_Keys, sizeof( uc_Keys ) ) ){
		THRL_ThAssert( False, "%s(%d) setKeyInfo()", (uintptr_t)__FUNCTION__, __LINE__, 0, 0, CFA14_DetailCode0x01BA );
		return -1;
	}

// 2016/04/11 CQ526818 ストレージ暗号化パスワードのハッシュ化対応 Sky大嶋 Start.
	if(!pc_SecuritySystemSetting->setPasswordInfo(e_InfoType, passwordInfo)){
		return -1;
	}
// 2016/04/11 CQ526818 ストレージ暗号化パスワードのハッシュ化対応 Sky大嶋 End.
		
	return 0;
}

/*================================================================================================
［機能］	鍵材料の再生成と不揮発メモリ更新.
--------------------------------------------------------------------------------------------------
［引数］	uc_strage		ストレージ種別、THRE_StorageDeviceTypeHDD、THRE_StorageDeviceTypeSSDを期待.
		 	psc_Password	鍵更新用に指定されたパスワード.
［戻値］	short：0：成功 / その他：失敗.
［詳細］	対象ストレージ暗号化用の鍵材料（KEKサブマスクA、KEK[DEK]）を生成し、不揮発メモリ(Flash)を更新する.
［機能］	パスワード更新：SubmaskA・KEK[DEK]生成、および、Flash更新.
 [履歴]     2015/10/23 中村   [SecV2-4]新規作成.
  			2016/02/04 中村   CQ524977[SecV2]メインストレージの暗号化設定が機能しない.KEK[DEK]データサイズの見直し対応.
			2016/03/11 中村   CQ.525797[SecV2-4]メインストレージ暗号化を変更すると、FW異常メッセージ提示され、デバイス利用不可.
			2017/11/09	SCD2-Sky Shindo [WP.53-54] 設定値見直し対応.
================================================================================================*/
short CP_HDDTrans::updKeyMaterial_updNvMemory(uchar uc_strage, const char* psc_Password)
{
	APIC_SystemSetting *pc_SecuritySystemSetting = NULL;
	THRC_Crypto *pc_Crypto = NULL;
	uchar uc_Keys[NVDC_KeyLen + NVDC_KeyLen + NVDC_KEKDEKLen] = {0};
	uchar uc_maskA[NVDC_KeyLen] = {0};
	uchar uc_maskB[NVDC_KeyLen] = {0};
	uchar uc_KEKDEK[NVDC_KEKDEKLen] = {0};
	TYPE_FlashKeyType enm_maskB;
	TYPE_FlashKeyType enm_Key;
// 2016/04/11 CQ526818 ストレージ暗号化パスワードのハッシュ化対応 Sky大嶋 Start.
	TYPE_PswdInfo e_InfoType = TYPE_PswdInfo_None;
	TYPS_PasswordInfo passwordInfo;
// 2016/04/11 CQ526818 ストレージ暗号化パスワードのハッシュ化対応 Sky大嶋 End.

	if( THRE_StorageDeviceTypeHDD == uc_strage ){
		enm_maskB = TYPE_FlashKeyType_HDD_KEKSubmaskB;
		enm_Key = TYPE_FlashKeyType_HDD;
// 2016/04/11 CQ526818 ストレージ暗号化パスワードのハッシュ化対応 Sky大嶋 Start.
		e_InfoType = TYPE_PswdInfo_HDD;
// 2016/04/11 CQ526818 ストレージ暗号化パスワードのハッシュ化対応 Sky大嶋 End.
	}
	else if( THRE_StorageDeviceTypeSSD == uc_strage ){
		enm_maskB = TYPE_FlashKeyType_SSD_KEKSubmaskB;
		enm_Key = TYPE_FlashKeyType_SSD;
// 2016/04/11 CQ526818 ストレージ暗号化パスワードのハッシュ化対応 Sky大嶋 Start.
		e_InfoType = TYPE_PswdInfo_SSD;
// 2016/04/11 CQ526818 ストレージ暗号化パスワードのハッシュ化対応 Sky大嶋 End.
	}
	else {
		printf("%s(%d)error param.\n", __FUNCTION__, __LINE__);
		return -1;
	}


	if(NULL == psc_Password){
		printf("%s(%d)error param.\n", __FUNCTION__, __LINE__);
		return -1;
	}

	pc_SecuritySystemSetting = APIC_SystemSetting::newInstance();
	if( NULL == pc_SecuritySystemSetting ) {
		printf( "%s(%d)Error, failed APIC_SystemSetting::newInstance().\n", __FUNCTION__, __LINE__ );
		return -1;
	}

	pc_Crypto = THRC_Crypto::newInstance();
	if( NULL == pc_Crypto ) {
		printf( "%s(%d)Error, failed THRC_Crypto::newInstance().\n", __FUNCTION__, __LINE__ );
		return -1;
	}

	/* 現在のKEKサブマスクBを取得する */
	if( False == pc_SecuritySystemSetting->getKeyInfo( enm_maskB, uc_maskB, sizeof(uc_maskB) ) ){
		printf( "%s(%d)Error, failed getKeyInfo(maskB).\n", __FUNCTION__, __LINE__ );
		return -1;
	}

	/* 新KEKサブマスクA、新KEK[DEK]を生成する */
	if( 0 != pc_Crypto->updMaskA_KEKDEK( uc_strage, psc_Password, uc_maskA, NVDC_KeyLen, uc_KEKDEK, NVDC_KEKDEKLen) ){
		printf( "%s(%d)Error, failed updMaskA_KEKDEK().\n", __FUNCTION__, __LINE__ );
		return -1;
	}

	memcpy(uc_Keys, uc_maskA,  sizeof(uc_maskA) );
	memcpy(&uc_Keys[sizeof(uc_maskA)], uc_maskB,  sizeof(uc_maskB) );
	memcpy(&uc_Keys[sizeof(uc_maskA) + sizeof(uc_maskB)], uc_KEKDEK, sizeof(uc_KEKDEK));

// 2016/04/11 CQ526818 ストレージ暗号化パスワードのハッシュ化対応 Sky大嶋 Start.
	// HCD-PPのPBKDFは、バイト数ではなく文字数が64文字以上である事を要求する.
	// そのため、マルチバイトコードで何文字のパスワードが設定されたかを保存する.
	const char* psc_PasswordTemp = psc_Password;
	uchar uc_CharSize = 0;
	while('\0' != *psc_PasswordTemp){
		uc_CharSize = LIBF_getUTF8ByteCount(psc_PasswordTemp);
		
		// Byte数が不明の場合、文字無しとみなし、文字列終端として処理する.
		if(0 == uc_CharSize){
			break;
		}
		++passwordInfo.us_Len;
		psc_PasswordTemp += uc_CharSize;
	}
	KM_SHA256_digest(reinterpret_cast<const uchar*>(psc_Password), strlen(reinterpret_cast<const char*>(psc_Password)), passwordInfo.uc_HashBuff);
// 2016/04/11 CQ526818 ストレージ暗号化パスワードのハッシュ化対応 Sky大嶋 End.

	/* 新KEKサブマスクA、KEKサブマスクB、新KEK[DEK]をFlashへ保存する */
	if( False == pc_SecuritySystemSetting->setKeyInfo( enm_Key, uc_Keys, sizeof( uc_Keys ) ) ){
		THRL_ThAssert( False, "%s(%d)setKeyInfo()", (uintptr_t)__FUNCTION__, __LINE__, 0, 0, CFA14_DetailCode0x01BA );
		return -1;
	}
	
// 2016/04/11 CQ526818 ストレージ暗号化パスワードのハッシュ化対応 Sky大嶋 Start.
	if(!pc_SecuritySystemSetting->setPasswordInfo(e_InfoType, passwordInfo)){
		return -1;
	}
// 2016/04/11 CQ526818 ストレージ暗号化パスワードのハッシュ化対応 Sky大嶋 End.
		
	return 0;
}

/*================================================================================================
［機能］	鍵材料の初期化と不揮発メモリ更新.
--------------------------------------------------------------------------------------------------
［引数］	uc_strage		ストレージ種別、THRE_StorageDeviceTypeHDD、THRE_StorageDeviceTypeSSDを期待.
［戻値］	short：0：成功 / その他：失敗.
［詳細］	対象ストレージ暗号化用の鍵材料（KEKサブマスクA、KEKサブマスクB、KEK[DEK]）をクリアし、不揮発メモリ(Flash)を更新する.
［機能］	パスワード更新：SubmaskA・SubmaskB・KEK[DEK]のクリア、および、Flash更新.
 [履歴]     2015/10/23 中村   [SecV2-4]新規作成.
  			2016/02/04 中村   CQ524977[SecV2]メインストレージの暗号化設定が機能しない.KEK[DEK]データサイズの見直し対応.
			2016/03/11 中村   CQ.525797[SecV2-4]メインストレージ暗号化を変更すると、FW異常メッセージ提示され、デバイス利用不可.
			2017/11/09	SCD2-Sky Shindo [WP.53-54] 設定値見直し対応.
================================================================================================*/
short CP_HDDTrans::clrKeyMaterial_updNvMemory(uchar uc_strage)
{
	APIC_SystemSetting *pc_SecuritySystemSetting = NULL;
	uchar uc_Keys[NVDC_KeyLen + NVDC_KeyLen + NVDC_KEKDEKLen] = {0};
	TYPE_FlashKeyType enm_Key;
// 2016/04/11 CQ526818 ストレージ暗号化パスワードのハッシュ化対応 Sky大嶋 Start.
	TYPE_PswdInfo e_InfoType = TYPE_PswdInfo_None;
	TYPS_PasswordInfo passwordInfo;
// 2016/04/11 CQ526818 ストレージ暗号化パスワードのハッシュ化対応 Sky大嶋 End.

	if( THRE_StorageDeviceTypeHDD == uc_strage ){
		enm_Key = TYPE_FlashKeyType_HDD;
// 2016/04/11 CQ526818 ストレージ暗号化パスワードのハッシュ化対応 Sky大嶋 Start.
		e_InfoType = TYPE_PswdInfo_HDD;
// 2016/04/11 CQ526818 ストレージ暗号化パスワードのハッシュ化対応 Sky大嶋 End.
	}
	else if( THRE_StorageDeviceTypeSSD == uc_strage ){
		enm_Key = TYPE_FlashKeyType_SSD;
// 2016/04/11 CQ526818 ストレージ暗号化パスワードのハッシュ化対応 Sky大嶋 Start.
		e_InfoType = TYPE_PswdInfo_SSD;
// 2016/04/11 CQ526818 ストレージ暗号化パスワードのハッシュ化対応 Sky大嶋 End.
	}
	else {
		printf("%s(%d)error param.\n", __FUNCTION__, __LINE__);
		return -1;
	}

	pc_SecuritySystemSetting = APIC_SystemSetting::newInstance();
	if( NULL == pc_SecuritySystemSetting ) {
		printf( "%s(%d)Error, failed APIC_SystemSetting::newInstance().\n", __FUNCTION__, __LINE__ );
		return -1;
	}

	/* KEKサブマスクA, KEKサブマスクB, KEK[DEK]を初期化する */
	if( False == pc_SecuritySystemSetting->setKeyInfo( enm_Key, uc_Keys, sizeof( uc_Keys ) ) ){
		THRL_ThAssert( False, "%s(%d)setKeyInfo()", (uintptr_t)__FUNCTION__, __LINE__, 0, 0, CFA14_DetailCode0x01BA );
		return -1;
	}
	
// 2016/04/11 CQ526818 ストレージ暗号化パスワードのハッシュ化対応 Sky大嶋 Start.
	if(!pc_SecuritySystemSetting->setPasswordInfo(e_InfoType, passwordInfo)){
		return -1;
	}
// 2016/04/11 CQ526818 ストレージ暗号化パスワードのハッシュ化対応 Sky大嶋 End.
		
	return 0;
}
// 2015/05/09 SCD2-Sky Kawanishi [SecV2-9] 暗号化鍵のインポート対応.
// 2017/11/09	SCD2-Sky Shindo [WP.53-54] 設定値見直し対応.
short CP_HDDTrans::impKeyMaterial_updNvMemory(uchar uc_strage, const TYPS_StorageEncryptionKeyInfo* ps_KeyInfo, Bool bl_isEncrypt)
{
	APIC_SystemSetting *pc_SecuritySystemSetting = NULL;
	THRC_Crypto *pc_Crypto = NULL;
	TYPE_FlashKeyType enm_Key;
	TYPE_PswdInfo e_InfoType = TYPE_PswdInfo_None;
	TYPS_PasswordInfo passwordInfo;
	uchar uc_Keys[NVDC_KeyLen + NVDC_KeyLen + NVDC_KEKDEKLen] = {0};
	uchar uc_DEK[NVDC_KeyLen] = {0};
	uchar uc_KEK[NVDC_KeyLen] = {0};
	uchar uc_KEKDEK[NVDC_KEKDEKLen] = {0};

	if( THRE_StorageDeviceTypeHDD == uc_strage ){
		enm_Key = TYPE_FlashKeyType_HDD;
		e_InfoType = TYPE_PswdInfo_HDD;
	}
	else if( THRE_StorageDeviceTypeSSD == uc_strage ){
		enm_Key = TYPE_FlashKeyType_SSD;
		e_InfoType = TYPE_PswdInfo_SSD;
	}
	else {
		printf("%s(%d)error param.\n", __FUNCTION__, __LINE__);
		return -1;
	}
	
	if(NULL == ps_KeyInfo){
		printf("%s(%d)error param.\n", __FUNCTION__, __LINE__);
		return -1;
	}

	pc_SecuritySystemSetting = APIC_SystemSetting::newInstance();
	if( NULL == pc_SecuritySystemSetting ) {
		printf( "%s(%d)Error, failed APIC_SystemSetting::newInstance().\n", __FUNCTION__, __LINE__ );
		return -1;
	}

	pc_Crypto = THRC_Crypto::newInstance();
	if( NULL == pc_Crypto ) {
		printf( "%s(%d)Error, failed THRC_Crypto::newInstance().\n", __FUNCTION__, __LINE__ );
		return -1;
	}

	if (True == bl_isEncrypt) { // 暗号化環境時.
		// 現在のDEKを取得(復号化).
		if (0 != pc_Crypto->getDEK(uc_strage, uc_DEK, sizeof(uc_DEK))) {
			printf( "%s(%d)Error, failed getDEK().\n", __FUNCTION__, __LINE__ );
			return -1;
		}
	}
	else { // 平文環境時.
		/* 乱数生成機より DEKを取得 */
		if( 0 != pc_Crypto->creRandomValue(uc_DEK, sizeof(uc_DEK)) ){
			printf( "%s(%d)Error, creRandomValue() failed.\n", __FUNCTION__, __LINE__ );
			return -1;
		}
	}
	// 新しいサブマスクA、サブマスクBから新KEKを作成.
	if (0 != pc_Crypto->SubmasksToKEK(ps_KeyInfo->uc_KEK_A, ps_KeyInfo->uc_KEK_B, uc_KEK, sizeof(uc_KEK))) {
		printf( "%s(%d)Error, failed SubmasksToKEK().\n", __FUNCTION__, __LINE__ );
		return -1;
	}
	// KEK[DEK]を導出.
	if (0 != pc_Crypto->KWP(uc_KEK, sizeof(uc_KEK), uc_DEK, sizeof(uc_DEK), uc_KEKDEK, sizeof(uc_KEKDEK))) {
		printf( "%s(%d)Error, failed KWP().\n", __FUNCTION__, __LINE__ );
		return -1;
	}

	memcpy(uc_Keys, ps_KeyInfo->uc_KEK_A,  sizeof(ps_KeyInfo->uc_KEK_A) );
	memcpy(&uc_Keys[sizeof(ps_KeyInfo->uc_KEK_A)], ps_KeyInfo->uc_KEK_B,  sizeof(ps_KeyInfo->uc_KEK_B) );
	memcpy(&uc_Keys[sizeof(ps_KeyInfo->uc_KEK_A) + sizeof(ps_KeyInfo->uc_KEK_B)], uc_KEKDEK, sizeof(uc_KEKDEK));
	
	memcpy(passwordInfo.uc_HashBuff, ps_KeyInfo->uc_HashBuff, sizeof(passwordInfo.uc_HashBuff) );
	passwordInfo.us_Len = ps_KeyInfo->us_Len;

	/* 新KEKサブマスクA、KEKサブマスクB、新KEK[DEK]をFlashへ保存する */
	if( False == pc_SecuritySystemSetting->setKeyInfo( enm_Key, uc_Keys, sizeof( uc_Keys ) ) ){
		THRL_ThAssert( False, "%s(%d)setKeyInfo()", (uintptr_t)__FUNCTION__, __LINE__, 0, 0, CFA14_DetailCode0x01BA );
		return -1;
	}
	
	if(!pc_SecuritySystemSetting->setPasswordInfo(e_InfoType, passwordInfo)){
		return -1;
	}
		
	return 0;
}

#if 0
/* デバッグ用暗号化インタフェース */
extern CP_HDDTrans theHDDTrans;

int32_t creKey(int32_t dev, char* str)
{
	if((dev < 1) || (dev > 2) || (NULL == str)){
		printf("usage: %s <device 1:HDD, 2:SSD> <password>.\n", __func__);
		return -1;
	}
	return theHDDTrans.creKeyMaterial_updNvMemory((uchar)dev, str);
}

int32_t updKey(int32_t dev, char* str)
{
	if((dev < 1) || (dev > 2) || (NULL == str)){
		printf("usage: %s <device 1:HDD, 2:SSD> <password>.\n", __func__);
		return -1;
	}
	return theHDDTrans.updKeyMaterial_updNvMemory((uchar)dev, str);
}

int32_t clrKey(int32_t dev)
{
	if((dev < 1) || (dev > 2)){
		printf("usage: %s <device 1:HDD, 2:SSD>.\n", __func__);
		return -1;
	}
	return theHDDTrans.clrKeyMaterial_updNvMemory((uchar)dev);
}

int32_t chkKey(int32_t dev)
{
	uchar uc_DEK[NVDC_KeyLen] = {0};
	
	if((dev < 1) || (dev > 2)){
		printf("usage: %s <device 1:HDD, 2:SSD>.\n", __func__);
		return -1;
	}
	
	if( 0 != THRC_Crypto::newInstance()->getDEK( (uchar)dev, uc_DEK, NVDC_KeyLen ) ){
		return -1;
	}
	
	printf(" DEK  :[%016llx %016llx %016llx %016llx]\n",*(__u64*)&uc_DEK[ 0],*(__u64*)&uc_DEK[ 8],*(__u64*)&uc_DEK[16],*(__u64*)&uc_DEK[24]);
	return 0;
}
#endif
// ================================================================================================.
//! <!--機能--> areaInitValueで設定するパーティションの範囲サイズチェック.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! なし.
//! <!--履歴-->.
//  2018/09/07	simogaki	新規.
//! ================================================================================================.
void CP_HDDTrans::DspRangeSize() {

    int64_t ll_StorageSize = 0;						// ストレージの容量.
	const uint64_t u64t_ByteUnit = 1024;			// サイズ出力時の単位(KB).
    ll_StorageSize = getPhysicalSubDeviceSize();
	int64_t i64_PartSize_256 = THRG_GetPartirionSize( ssd_part_info_256GB );
	int64_t	i64_PartSize_1T  = THRG_GetPartirionSize( ssd_part_info_1TB );
#ifdef DEF_A3A4COMM
	int64_t	i64_PartSize_16  = THRG_GetPartirionSize( sd_part_info_16GB );
#endif // DEF_A3A4COMM.
	// ストレージの容量出力
	if ( ll_StorageSize >= i64_PartSize_1T) {
		printf("StoregeSize             :\t1T\t%"PRIu64"\n", ll_StorageSize );
	} else if ( ll_StorageSize >= i64_PartSize_256 ) {
		printf("StoregeSize             :\t256G\t%"PRIu64"\n", ll_StorageSize );
#ifdef DEF_A3A4COMM
	} else if ( ll_StorageSize >= i64_PartSize_16 ) {
		printf("StoregeSize             :\t16G\t%"PRIu64"\n", ll_StorageSize );
#endif // DEF_A3A4COMM.
	} else {
		printf("StoregeSize             :\t8G\t%"PRIu64"\n", ll_StorageSize );
	}
	//範囲チェックは先頭(セクタNo0～)のデータ領域から順に出力.
	printf("RangeCheck              :\tStart\tEnd\n");
	printf("FormatID Range          :\t%u\t%u\n", ul_SysStartBlock , ul_SysStartBlock + DEF_HDD_SYS_BLOCK_NUM );
	printf("SnapShot Range          :\t%u\t%u\n", DEF_SNAPSHOTINFO_START_BLOCK, DEF_SNAPSHOTINFO_START_BLOCK+ DEF_SNAPSHOTINFO_BLOCK_NUM);	// サブセットでは意識していないが、本体FWとフォーマットを合わせるため.
#ifdef DEF_A4_Cus
	if( ll_StorageSize < i64_PartSize_16 ) {		// 16Gより小さいのは8Gのみ.
#else // DEF_A4_Cus.
	if( ll_StorageSize < i64_PartSize_256 ) {		// 256Gより小さいのは8Gのみ.
#endif // DEF_A4_Cus.
		printf("StoreFat1st Range       :\t-\t-\n" );
		printf("StoreFat2nd Range       :\t-\t-\n" );
	} else {
		printf("StoreFat1st Range       :\t%u\t%u\n", ul_SectorFatStoreFirst, ul_SectorFatStoreFirst + ul_SectorFatSize );
		printf("StoreFat2nd Range       :\t%u\t%u\n", ul_SectorFatStoreSecond, ul_SectorFatStoreSecond + ul_SectorFatSize );
	}
	printf("SwapImage Range         :\t%u\t%u\n", ul_SwapStartBlock, ul_SwapStartBlock + ul_SwapBlkNum);
	printf("SwapInfo Range          :\t%u\t%u\n", ul_SwapinfoStartBlock, ul_SwapinfoStartBlock + ul_SwapinfoBlkNum );
	printf("R/Wcheck Range          :\t%u\t%u\n", ul_SysRwChkStartBlock, ul_SysRwChkStartBlock + ul_SysRwChkBlockNum );
	if( ll_StorageSize < i64_PartSize_256 ) {		// 256Gより小さい時のみ.
		printf("SwapFat1st Range        :\t-\t-\n" );
		printf("SwapFat2nd Range        :\t-\t-\n" );
		printf("StoreImage Range        :\t-\t-\n" );
		printf("StoreImage(KM) Range    :\t-\t-\n" );
		printf("StoreImage(BackUp) Range:\t-\t-\n" );
		printf("StoreInfo Range         :\t-\t-\n" );
	} else {
		printf("SwapFat1st Range        :\t%u\t%u\n", ul_SectorFatSwapFirst, ul_SectorFatSwapFirst + ul_SectorFatSize );
		printf("SwapFat2nd Range        :\t%u\t%u\n", ul_SectorFatSwapSecond, ul_SectorFatSwapSecond + ul_SectorFatSize );
		printf("StoreImage Range        :\t%u\t%u\n", ul_StoreStartBlock, ul_StoreStartBlock + ul_StoreBlkNum );
		printf("StoreImage(KM) Range    :\t%u\t%u\n", ul_StoreStartBlock, ul_StoreStartBlock + ul_StoreBlkNum / 2);
		printf("StoreImage(BackUp) Range:\t%u\t%u\n", ul_StoreStartBlock + ul_StoreBlkNum / 2,  ul_StoreStartBlock + ( ul_StoreBlkNum / 2 ) + ( ul_StoreBlkNum / 2 ));
		printf("StoreInfo Range         :\t%u\t%u\n", ul_StoreinfoStartBlock, ul_StoreinfoStartBlock + ul_StoreinfoBlkNum );
	}
	printf("OS Range                :\t%u\t%u\n", ul_OsStartBlock, ul_OsStartBlock + ul_OsBlockNum );
	//サイズチェックはByte単位で出すと値が大きいので、KB単位で出力.
	//MB、Byteする場合は、小数表示やlongの上限超えるので書式等見直すこと.
	printf("SizeCheck              :\tSector\tKB\n");
	printf("FormatID Size          :\t%u\t%u\n", DEF_HDD_SYS_BLOCK_NUM, DEF_HDD_SYS_BLOCK_NUM / ( u64t_ByteUnit / DEF_SECTOR_SIZE ));
	printf("SnapShot Size          :\t%u\t-\n", DEF_SNAPSHOTINFO_BLOCK_NUM );
#ifdef DEF_A4_Cus
	if( ll_StorageSize < i64_PartSize_16 ) {		// 16Gより小さいのは8Gのみ.
#else // DEF_A4_Cus.
	if( ll_StorageSize < i64_PartSize_256 ) {		// 256Gより小さいのは8Gのみ.
#endif // DEF_A4_Cus.
		printf("StoreFat1st Size       :\t-\t-\n" );
		printf("StoreFat2nd Size       :\t-\t-\n" );
	} else {
		printf("StoreFat1st Size       :\t%u\t%u\n", ul_SectorFatSize, ul_FatTotalSize / u64t_ByteUnit);
		printf("StoreFat2nd Size       :\t%u\t%u\n", ul_SectorFatSize, ul_FatTotalSize / u64t_ByteUnit);
	}
	printf("SwapImage Size         :\t%u\t%"PRIu64"\n", ul_SwapBlkNum, ul_SwapBlkNum / ( u64t_ByteUnit / DEF_SECTOR_SIZE ) );
	printf("SwapInfo Size          :\t%u\t%"PRIu64"\n", ul_SwapinfoBlkNum, ul_SwapinfoBlkNum / ( u64t_ByteUnit / DEF_SECTOR_SIZE ) );
	printf("R/Wcheck Size          :\t%u\t%u\n", ul_SysRwChkBlockNum, ul_SysRwChkBlockNum / ( u64t_ByteUnit / DEF_SECTOR_SIZE ) );
	if( ll_StorageSize < i64_PartSize_256 ) {		// 256Gより小さい時のみ.
		printf("SwapFat1st Size        :\t-\t-\n" );
		printf("SwapFat2nd Size        :\t-\t-\n" );
		printf("StoreImage Size        :\t-\t-\n" );
		printf("StoreImage(KM) Size    :\t-\t-\n" );
		printf("StoreImage(BackUp) Size:\t-\t-\n" );
		printf("StoreInfo Size         :\t-\t-\n" );
	} else {
		printf("SwapFat1st Size        :\t%u\t%"PRIu64"\n", ul_SectorFatSize, ul_FatTotalSize / u64t_ByteUnit );
		printf("SwapFat2nd Size        :\t%u\t%"PRIu64"\n", ul_SectorFatSize, ul_FatTotalSize / u64t_ByteUnit );
		printf("StoreImage Size        :\t%u\t%"PRIu64"\n", ul_StoreBlkNum, ul_StoreBlkNum / ( u64t_ByteUnit / DEF_SECTOR_SIZE ) );
		printf("StoreImage(KM) Size    :\t%u\t%"PRIu64"\n", ul_StoreBlkNum / 2, ul_StoreBlkNum / 2 / ( u64t_ByteUnit / DEF_SECTOR_SIZE ) );
		printf("StoreImage(BackUp) Size:\t%u\t%"PRIu64"\n", ul_StoreBlkNum / 2, ul_StoreBlkNum / 2 / ( u64t_ByteUnit / DEF_SECTOR_SIZE ) );
		printf("StoreInfo Size         :\t%u\t%"PRIu64"\n", ul_StoreinfoBlkNum, ul_StoreinfoBlkNum / ( u64t_ByteUnit / DEF_SECTOR_SIZE ) );
	}
	printf("OS Size                :\t%u\t%"PRIu64"\n", ul_OsBlockNum, ul_OsBlockNum / ( u64t_ByteUnit / DEF_SECTOR_SIZE ) );
}

// ================================================================================================.
//! <!--機能--> FormatID書き込み処理（デバッグ用）.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! Bool bl_Reboot	:	書き込み後のリブート処理実行指定（False：実行しない、True:実行する）.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! デバッグ専用 FormatID書き込み処理.
//! Subsetではなくシェルによるダウンロードの際にFormatIDが書き込めないため、直接書き込みを行う.
//! <!--履歴-->.
//  2018/02/01	iyanagi	【Eagle】機種立ち上げ不具合対応.
//! ================================================================================================.
void THRG_WriteFormatID( Bool bl_Reboot )
{
	// チェックID直接書き込み.
	Bool bl_Ret = theHDDTrans.writeID_Direct();
	if( bl_Ret == False ){
		printf( "FormatID Write Failure.\n" );
		return;
	}
	printf( "FormatID Write Complete.\n" );
	
	// リブート処理.
	if( bl_Reboot != False ){// 実行指定されている時のみ実行する.
		printf( "Exec reboot.\n" );
		system( "exec reboot" );
	}
}

// ================================================================================================.
//! <!--機能--> 優先パーティション使用可能有無取得処理.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! なし.
//! <!--戻値-->.
//! \return Bool	True：使用可能 / False：使用不可能.
//! <!--詳細-->.
//! 優先パーティションが使用可能かを返します.
//! NV情報領域状態を取得して.
//! 「フォーマット済み」の時、True（使用可能）.
//! 「フォーマット済み」以外の時、False（使用不可）.
//! NV情報領域状態は、優先パーティションのマウント正常終了後に、「フォーマット済み」に.
//! 設定されるため、その値を判断することで、機能を実現している.
//! <!--履歴-->.
//  2018.04.25 井柳 【Eagle】RQ-698-43 「WebUI 起動時間短縮対応」新規作成.
//! ================================================================================================.
Bool THRG_getEnablePrePartition()
{
	// NV情報領域状態を取得.
	TYPE_HddStatus e_State = theHDDTrans.getNvramInfoAreaStateInSSD();

	if( TYPE_HddStatusFormated != e_State ){	// 判定
		return False;	// 使用不可能.
	}

	return True;		// 使用可能.
}

// ================================================================================================.
//! <!--機能--> MicroSDメーカーID取得処理.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! uint32_t *pui32_ID : メーカーID.
//! <!--戻値-->.
//! \return Bool	True : 正常、False: 異常終了.
//! <!--詳細-->.
//! MicroSDからメーカーIDを取得する.
//! ローカル関数のラッパーを呼び出す.実処理は、Bool sysBspGetMicroSdMakerID( uint32_t *pui32_ID).
//! <!--履歴-->.
//  2018/07/10 SKY井柳	【Eagle】制御構成変更No102「基板交換低減対応」新規作成.
//! ================================================================================================.
Bool THRG_getMicroSdMakerID( uint32_t *pui32_ID)
{
	return THRF_getMicroSdMakerID( pui32_ID );
}

// ================================================================================================.
//! <!--機能--> MicroSD S.M.A.R.T情報取得処理.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! uint32_t ui32_MakerID : MicrSDのメーカーID.
//! uchar *puc_Info : S.M.A.R.T情報:512Byteのバイナリデータ.
//! <!--戻値-->.
//! \return Bool	True : 正常、False: 異常終了.
//! <!--詳細-->.
//! MicroSDからメーカーIDを取得する.
//! ローカル関数のラッパーを呼び出す.実処理は、Bool sysBspGetMicroSdSmartInfo( uchar *puc_Info ).
//! <!--履歴-->.
//  2018/07/10 SKY井柳	【Eagle】制御構成変更No102「基板交換低減対応」新規作成.
//! ================================================================================================.
Bool THRG_getMicroSdSmartInfo( uint32_t ui32_MakerID, uchar *puc_Info )
{
	return THRF_getMicroSdSmartInfo( ui32_MakerID, puc_Info );
}

// ================================================================================================.
//! <!--機能--> S.M.A.R.T情報ログ出力処理.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! なし.
//! <!--戻値-->.
//! なし.
//! <!--詳細-->.
//! SSD及び、MicroSDのS.M.A.R.T情報を出力する.
//! ただし、装着状態の有無にかかわらず、メインストレージ(/dev/kmsda)を対象として情報取得を行う.
//! <!--履歴-->.
//!  2018/10/29 SKY井柳	【Eagle】1062347 (追加実装) 新規作成.
//! ================================================================================================.
void THRG_dspMainDeviceSmartInfo()
{
	// SSD.
	printf( "SSD S.M.A.R.T Info.\n" );
	printf( "**********************************************************************\n" );
	printf( "Command=\"%s\"\n",THRD_SmartInfoCommand );
	system( THRD_SmartInfoCommand );
	printf( "**********************************************************************\n" );

	printf( "\n\n" );

	// MicroSD.
	printf( "MicroSD S.M.A.R.T Info.\n" );
	uint32_t ui32_Maker_ID = 0;
	Bool bl_Ret = THRG_getMicroSdMakerID( &ui32_Maker_ID );
	if( bl_Ret == True ){
		printf( "MicroSD MakerID[0x%08lx].\n", ui32_Maker_ID );
		uchar uc_SmartInfo[ THRD_MicroSdSmartInfoSize ] = {0};
		bl_Ret = THRG_getMicroSdSmartInfo( ui32_Maker_ID, uc_SmartInfo );
		if( bl_Ret == True ){
			printf( "**********************************************************************\n" );
			printf("         : 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F\n");
			printf("         :------------------------------------------------");
			ushort us_Loop = 0;
			for( us_Loop=0; us_Loop < THRD_MicroSdSmartInfoSize; us_Loop++){
				if (us_Loop%16==0){
					printf("\n%08"PRIX32" : ", us_Loop);
				}
				printf("%02X ", uc_SmartInfo[us_Loop]);
			}
			printf("\n");
			printf("         :------------------------------------------------\n");
			printf( "**********************************************************************\n" );
			printf( "\n" );
		}else{
			printf("MicroSD SmartInfo Get Error.\n" );
		}
	}else{
		printf("MicroSD MakerID Get Error.\n" );
	}
}
// ================================================================================================.
//! <!--機能--> ストレージ情報定義表示.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! ストレージ情報定義表示.\n.
//! <!--履歴-->.
//  18/08/06 simogaki	 cramfs破壊問題対応.
// ================================================================================================.
void THRG_DispStoragePartition(){
	
	part_layout* ps_dsp_part = NULL;
	ps_dsp_part = THRG_GetPartitionType();
	int32_t i32t_cnt;
	uint32_t u32t_sec=0;
	
	if ( NULL != ps_dsp_part ) {
		if ( sd_part_info_8GB == ps_dsp_part ) {
				printf("8G\n");
		} else if ( ssd_part_info_256GB == ps_dsp_part ) {
				printf("256G\n");
		} else if ( ssd_part_info_1TB == ps_dsp_part ) {
				printf("1T\n");
		} else {
				printf("UnKnown \n");
		}
    	printf("PartNo:\t mnt \t SecNum \t StartSecNo \n");
	    for( i32t_cnt = 0; ps_dsp_part[i32t_cnt].type != PART_TYPE_END; i32t_cnt++ ){
	    	printf("kmsda%02u:\t %s \t %u \t %u\n",i32t_cnt+1 , ps_dsp_part[i32t_cnt].mntpnt, ps_dsp_part[i32t_cnt].nsec, u32t_sec );
	    	if ( ps_dsp_part[i32t_cnt].type != PART_TYPE_EXTENDED ) {
	    		u32t_sec += ps_dsp_part[i32t_cnt].nsec;
	    	}
        }
    	printf("\n");
	} else {
		printf("THRG_GetPartitionType err\n");
	}
}

// ================================================================================================.
//! <!--機能--> データ移行用 元データ領域マウント処理.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! データ移行処理時に、呼ばれる\n.
//! データ移行用 元データ領域マウント処理(実行部)の結果をメモリファンクション結果に設定する\n.
//! <!--履歴-->.
//!  2018/09/21 Sky井柳 【DenebMLK】制御構成変更No78「データ移行」対応 新規作成.
// ================================================================================================.
void CP_HDDTrans::mountDataMoveBaseDataDevice( void )
{
	THRL_ThLogMsgErr("[%s] Start.\n", __FUNCTION__);
	Bool bl_Ret = func_MountDataMoveBaseDataDevice( );
	setDebugHddSeq( 2007, (uint32_t)bl_Ret);
	//処理結果をメモリファンクション結果に設定する。
	CC_MEMFunctionSingletonIf::newInstance().SetMemFuncResult( bl_Ret );	// 診断結果（総合）を指定.
	CC_MEMFunctionSingletonIf::newInstance().callReportFunc() ;				// 完了報告コールバックの呼出.
	THRL_ThLogMsgErr("[%s] End.(Result:%d)\n", __FUNCTION__, bl_Ret );
}

// ================================================================================================.
//! <!--機能--> データ移行用 元データデバイスの存在確認処理.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return Bool	：	True : 正常、False: 異常終了.
//! <!--詳細-->.
//! データ移行用 元データデバイスとして、MicroSDが装着されている場合に、\n.
//! 存在するパス（"/sys/block/kmsdb/device/type"）の有無により判定する。.
//! 最初に呼ばれた時に判定結果を保持し、次に呼ばれたときは保持した値を返す.
//! <!--履歴-->.
//!  2018/09/21 Sky井柳 【DenebMLK】制御構成変更No78「データ移行」対応 新規作成.
//!  2018/11/08 Sky山岸 【DenebMLK】AR.XXXXXXX μSD＋SSDかどうかのAPI作成 判定結果を保持するよう変更.
// ================================================================================================.
Bool CP_HDDTrans::isDataMoveBaseDataDevice( void )
{
	static Bool bl_IsCalledOnce = False;	// 関数が呼ばれたことがあるかを保持。初回コール後はTrue.
	static Bool bl_Ret = False;				// 判定結果を保持する。初回コール時のみ値がセットされる.

	// 既に関数が呼ばれて判定結果を保持している場合は、保持した値を返す.
	if (True == bl_IsCalledOnce) {
		return bl_Ret;
	}

	// ここから先の処理は、初回コール時のみ実行される.
	struct stat st_Stat;					// ファイルステータス.
	memset(&st_Stat, 0, sizeof(st_Stat));
	if( -1 == SYSF_stat( THRD_DataMove_ExistTypePath, &st_Stat ) ) {// ステータス取得で、存在確認する.
		THRL_ThLogMsgErr("[%s] File Not Found.[%s]\n", __FUNCTION__,THRD_DataMove_ExistTypePath );
		bl_Ret =  False;	// typeが存在しない為、存在しないと判断(データ元はMicroSDのみとして判断).
	} else {
		bl_Ret = True;
	}

	bl_IsCalledOnce = True;

	return bl_Ret;
}

// ================================================================================================.
//! <!--機能--> データ移行用 元データデバイスのパーティション＆サイズチェック処理.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return Bool	：	True : 正常、False: 異常終了.
//! <!--詳細-->.
//! データ移行用 元データデバイスとして、MicroSD（8G）が装着されているとして\n.
//! パーティションのサイズをチェックする.
//! <!--履歴-->.
//!  2018/09/21 Sky井柳 【DenebMLK】制御構成変更No78「データ移行」対応 新規作成.
// ================================================================================================.
Bool CP_HDDTrans::checkPartSizeDataMoveBaseDataDevice( void )
{
	char *pscDevicePath = THRD_DataMove_DevicePath;		// 元データデバイスは、kmsdb固定.
	part_layout* pst_PartInfo = sd_part_info_8GB;		// 元データパーティションは、MicroSD 8G固定.
	int32_t si_BlockSize = 0;
	DLONG sdl_DevSize = 0;

	// 元データデバイスのサイズ取得.
	int32_t si32_Ret = THRF_sysBspDeviceBlockSizeGet( DEF_DEVICEID_SSD, pscDevicePath, &si_BlockSize );
	setDebugHddSeq( 2008, (uint32_t)si32_Ret);
	if( si32_Ret != HDD_STATE_OK ) {
		THRL_ThLogMsgErr("[%s] SizeGet Error.[%ld]\n", __FUNCTION__,si32_Ret );
		return False;
	}
	sdl_DevSize = (DLONG)si_BlockSize * (DLONG)DEF_SECTOR_SIZE;	// ブロックサイズをバイトサイズ変換.

	setDebugHddSeq( 2009, (uint32_t)( 0xFFFFFFFFUL & (sdl_DevSize >> 32)) );	// 上位32ビットをログに保存.
	setDebugHddSeq( 2010, (uint32_t)( 0xFFFFFFFFUL & sdl_DevSize) );			// 下位32ビットをログに保存.

    // 実装HDDの情報を元にパーティションレイアウトテーブルを構築
	si32_Ret = calcAndFixedPartInfo( pscDevicePath, pst_PartInfo, sdl_DevSize );
	setDebugHddSeq( 2011, (uint32_t)si32_Ret);
	if (si32_Ret == -1) {
		THRL_ThLogMsgErr("[%s] calcAndFixedPartInfo Error.\n", __FUNCTION__ );
		return False;
	}

	// 実装HDDのパーティション構成をチェック
	si32_Ret = checkPartitionLayout( pscDevicePath, pst_PartInfo, False );
	setDebugHddSeq( 2012, (uint32_t)si32_Ret);
	if (si32_Ret == -1 ) {
		THRL_ThLogMsgErr("[%s] checkPartitionLayout Error.\n", __FUNCTION__ );
		return False;
	}

	return True;
}

// ================================================================================================.
//! <!--機能--> データ移行用 元データデバイスのFSCK処理.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param Bool bl_Encrypt				：暗号化状態（True:暗号化、False:非暗号化）.
//! \param part_layout* pst_PartInfo	：データ移行対象のパーティション情報.
//! <!--戻値-->.
//! \return Bool	：	True : 正常、False: 異常終了.
//! <!--詳細-->.
//! データ移行用対象のパーティションに対して、FSCKを実行する\n.
//! データ移行対象のパーティションすべてをアンマウントする(後の処理でマウントされるため)\n.
//! データ移行対象のパーティションのタイプが、ext2/ext3の時のみFSCKを実行する.
//! <!--履歴-->.
//!  2018/09/21 Sky井柳 【DenebMLK】制御構成変更No78「データ移行」対応 新規作成.
// ================================================================================================.
Bool CP_HDDTrans::doFsckDataMoveBaseDataDevice( Bool bl_Encrypt, part_layout* pst_PartInfo )
{
	int32_t si32_Ret = 0;
	int32_t si32_Loop = 0;					// ループカウンタ.
	uchar uc_Encrypt = bl_Encrypt ? 1:0;	// 暗号化状態のパスインデックス化
	if( pst_PartInfo == NULL ){	// パラメタチェック.
		setDebugHddSeq( 2013, 0 );
		THRL_ThLogMsgErr("[%s] pst_PartInfo is NULL.\n", __FUNCTION__ );
		return False;
	}

	// FSCK.
	for( si32_Loop = 0; pst_PartInfo[si32_Loop].type != PART_TYPE_END; si32_Loop++ ){
		umount2( (char*)pst_PartInfo[si32_Loop].devpath[uc_Encrypt], MNT_FORCE );	// 後の処理の為にアンマウント実行.
		
		Bool bl_FSCK = False;
		switch( pst_PartInfo[si32_Loop].fstype ){
			// ext2パーティション.
		case PART_FSTYPE_FS:
		case PART_FSTYPE_FS_NF2:
		case PART_FSTYPE_FS_SSD:
			// ext3パーティション.
		case PART_FSTYPE_FS_NF:
		case PART_FSTYPE_FS_NF3:
		case PART_FSTYPE_FS_NF4:
		case PART_FSTYPE_FS_SSD3:
			bl_FSCK = True;		// FSCK実施有効.
			break;

		case PART_FSTYPE_FAT:	// FATパーティション.
		case PART_FSTYPE_FAT16:
		case PART_FSTYPE_RAW:	// RAW.
		case PART_FSTYPE_NONE:	// None.
		default:
			bl_FSCK = False;	// FSCK実施対象外.
			break;
		}
		if( bl_FSCK == False ){
			continue;// FSCK実施対象外.
		}
		// FSCKを実施.
		si32_Ret = doFsck( (char*)pst_PartInfo[si32_Loop].devpath[uc_Encrypt], 0, pst_PartInfo[si32_Loop].fstype );
		if( si32_Ret != 0 ){
			setDebugHddSeq( 2014, 0 );
			setDebugHddSeq( 2015, (uint32_t)si32_Loop );
			setDebugHddSeq( 2016, (uint32_t)pst_PartInfo[si32_Loop].fstype );
			THRL_ThLogMsgErr("[%s] doFsck( \"%s\", %ld, %ld   ) Error.[%ld]\n", __FUNCTION__,
				(char*)pst_PartInfo[si32_Loop].devpath[uc_Encrypt], 0, pst_PartInfo[si32_Loop].fstype, si32_Ret );
			return False;
		}
	}
	
	return True;
}

// ================================================================================================.
//! <!--機能--> データ移行用 元データデバイスのマウント処理.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param Bool bl_Encrypt				：暗号化状態（True:暗号化、False:非暗号化）.
//! \param part_layout* pst_PartInfo	：データ移行対象のパーティション情報.
//! <!--戻値-->.
//! \return Bool	：	True : 正常、False: 異常終了.
//! <!--詳細-->.
//! データ移行用対象のパーティションのマウント先を作成の上、マウントを実施する\n.
//! <!--履歴-->.
//!  2018/09/21 Sky井柳 【DenebMLK】制御構成変更No78「データ移行」対応 新規作成.
//!  2019/8/27  Sky.     OP_BTS-10718 【EagleBK】静的解析対応用課題 ID:69112、69236、 69237  無駄なポインタ参照を削除.
// ================================================================================================.
Bool CP_HDDTrans::monutDataMoveBaseDataDevice( Bool bl_Encrypt, part_layout* pst_PartInfo )		// データ移行用 元データデバイスのマウント.
{
	int32_t si32_Ret = 0;
	int32_t si32_Loop = 0;					// ループカウンタ.
	uchar uc_Encrypt = bl_Encrypt ? 1:0;	// 暗号化状態のパスインデックス化
	if( pst_PartInfo == NULL ){	// パラメタチェック.
		setDebugHddSeq( 2013, 1 );
		THRL_ThLogMsgErr("[%s] pst_PartInfo is NULL.\n", __FUNCTION__ );
		return False;
	}

	for( si32_Loop = 0; pst_PartInfo[si32_Loop].type != PART_TYPE_END; si32_Loop++ ){
		
		//データ移行用 元データデバイスのマウント対象のマウント先を作成する。
		STATUS e_Status = SYSF_mkdir( pst_PartInfo[si32_Loop].mntpnt );
		if( e_Status != OK ){
			setDebugHddSeq( 2017, (uint32_t)si32_Loop );
			THRL_ThLogMsgErr("[%s] SYSF_mkdir( \"%s\" ) Error.[%ld]\n", __FUNCTION__, (uintptr_t)pst_PartInfo[si32_Loop].mntpnt, e_Status );
			return False;
		}
		
		//データ移行用 元データデバイスのマウント対象のパーティションをマウントする。
		char *psc_FileSystemType = NULL;		// ファイルシステムタイプ.
		uint32_t ui32_MountFlag = 0;			// マウントフラグ.
		char *psc_MountOption = NULL;			// マウントオプション.
		switch( pst_PartInfo[si32_Loop].fstype ){
			// FATパーティション.
		case PART_FSTYPE_FAT:
		case PART_FSTYPE_FAT16:
			psc_FileSystemType = "vfat";	// FAT指定.
			ui32_MountFlag = MS_NOATIME;
			psc_MountOption = NULL;
			break;

			// ext2パーティション.
		case PART_FSTYPE_FS:
		case PART_FSTYPE_FS_NF2:
		case PART_FSTYPE_FS_SSD:
			psc_FileSystemType = "ext2";	// ext2指定.
			ui32_MountFlag = MS_NOATIME;
			psc_MountOption = NULL;
			break;

			// ext3パーティション.
		case PART_FSTYPE_FS_NF:
		case PART_FSTYPE_FS_NF3:
		case PART_FSTYPE_FS_NF4:
		case PART_FSTYPE_FS_SSD3:
			psc_FileSystemType = "ext3";	// ext3指定.
			ui32_MountFlag = MS_NOATIME;
			psc_MountOption = (char *)EXT3_MOUNT_OPTIONS;
			break;

		case PART_FSTYPE_RAW:	// RAW.
		case PART_FSTYPE_NONE:	// None.
		default:
			si32_Ret = 0;// マウント処理なし(成功扱い).
			break;
		}
		
		if( psc_FileSystemType != NULL ){	// ファイルシステムタイプが指定されている時のみ、マウント.
			si32_Ret = mount( pst_PartInfo[ si32_Loop ].devpath[uc_Encrypt], pst_PartInfo[ si32_Loop ].mntpnt, psc_FileSystemType, ui32_MountFlag, psc_MountOption );
		}
		//マウント失敗.
		if( si32_Ret == -1 ){
			setDebugHddSeq( 2018, (uint32_t)si32_Loop );
			setDebugHddSeq( 2019, (uint32_t)pst_PartInfo[si32_Loop].fstype );
			THRL_ThLogMsgErr("[%s] mount( \"%s\", \"%s\", %lu , ", __FUNCTION__,
				(uintptr_t)pst_PartInfo[ si32_Loop ].devpath[uc_Encrypt], (uintptr_t)pst_PartInfo[ si32_Loop ].mntpnt, (uintptr_t)psc_FileSystemType, MS_NOATIME );
			if( psc_MountOption != NULL ){
				THRL_ThLogMsgErr("\"%s\" ) Error.\n", psc_MountOption );
			}else{
				THRL_ThLogMsgErr("NULL ) Error.\n" );
			}
			return False;
		}
	}
	
	return True;
}

// ================================================================================================.
//! <!--機能--> データ移行用 元データ領域マウント処理(実行部).
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return Bool	：	True : 正常、False: 異常終了.
//! <!--詳細-->.
//! 以下の処理を順次実行する\n.
//! ・データ移行用 元データデバイスの装着を確認する\n.
//! ・データ移行用 元データデバイスのパーティション数・サイズを確認する\n.
//! ・データ移行用 元データデバイスのマウント対象のパーティションに対して、FSCKを行う\n.
//! ・データ移行用 元データデバイスのマウント対象のマウント先を作成する\n.
//! ・データ移行用 元データデバイスのマウント対象のパーティションをマウントする\n.
//! <!--履歴-->.
//!  2018/09/21 Sky井柳 【DenebMLK】制御構成変更No78「データ移行」対応 新規作成.
//!  2019/04/05 Sky井柳 【IT6_1.2MR】制御構成変更No10「SSD暗号化対応」暗号化考慮を除外.
// ================================================================================================.
Bool CP_HDDTrans::func_MountDataMoveBaseDataDevice( void )
{
	//データ移行用 元データデバイスの装着を確認する。
	Bool bl_Ret = isDataMoveBaseDataDevice();
	setDebugHddSeq( 2000, (uint32_t)bl_Ret);
	if( bl_Ret != True ){
		// データ移行用 元データデバイス存在せず.
		THRL_ThLogMsgErr("[%s] DataMoveBaseDataDevice not found.\n", __FUNCTION__ );
		return False;
	}
	//データ移行用 元データデバイスのパーティション数・サイズを確認する。
	bl_Ret = checkPartSizeDataMoveBaseDataDevice();
	setDebugHddSeq( 2001, (uint32_t)bl_Ret);
	if( bl_Ret != True ){
		// データ移行用 元データデバイスのパーティション・サイズ異常.
		THRL_ThLogMsgErr("[%s] checkPartSizeDataMoveBaseDataDevice Error.\n", __FUNCTION__ );
		return False;
	}
	
	// 暗号化有効時(MicroSD)のデバイス設定.
	APIC_SystemSetting *pc_APIC_SystemSetting = APIC_SystemSetting::newInstance();
	if( pc_APIC_SystemSetting == NULL ){
		setDebugHddSeq( 2002, 0 );
		THRL_ThLogMsgErr("[%s] APIC_SystemSetting is NULL.\n", __FUNCTION__ );
		return False;
	}
// データ移行は、暗号化考慮無し.
#if 0 // 暗号化考慮の為に、コードを残しておく.
	int32_t si32_Ret = 0;
    Bool bl_Encrypt = pc_APIC_SystemSetting->getEnableEncryption();// 暗号化指定の取得.
	setDebugHddSeq( 2003, (uint32_t)bl_Encrypt);
	if( bl_Encrypt ){
		// 暗号化デバイス設定.
		si32_Ret = setupDmcrypt(THRD_DataMove_DevicePath, DEF_HDD_CRYPT_ON, st_part_info_DataMove, False);
		setDebugHddSeq( 2004, (uint32_t)si32_Ret);
		if (si32_Ret == -1) {
			THRL_ThLogMsgErr("[%s] setupDmcrypt( \"%s\" ) Error.\n", __FUNCTION__, THRD_DataMove_DevicePath );
			return False;
		}
	}
#else //0.
	Bool bl_Encrypt = False;
#endif //0.

	// データ移行用 パーティション情報設定.
	part_layout* pst_PartInfo = st_part_info_DataMove;
	
	//データ移行用 元データデバイスのマウント対象のパーティションに対して、FSCKを行う.
	bl_Ret = doFsckDataMoveBaseDataDevice( bl_Encrypt, pst_PartInfo );
	setDebugHddSeq( 2005, (uint32_t)bl_Ret);
	if( bl_Ret != True ){
		// データ移行用 元データデバイスのFSCK異常.
		THRL_ThLogMsgErr("[%s] doFsckDataMoveBaseDataDevice Error.\n", __FUNCTION__ );
		return False;
	}

	//データ移行用 元データデバイスのマウント対象のパーティションをマウントする.
	bl_Ret = monutDataMoveBaseDataDevice( bl_Encrypt, pst_PartInfo );
	setDebugHddSeq( 2006, (uint32_t)bl_Ret);
	if( bl_Ret != True ){
		// データ移行用 元データデバイスのFSCK・マウント異常.
		THRL_ThLogMsgErr("[%s] monutDataMoveBaseDataDevice Error.\n", __FUNCTION__ );
		return False;
	}
	return True;
}

// ================================================================================================.
//! <!--機能--> データ移行チェック処理.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! ・データ移行状態が、「データ移行開始」の時、SSD起動の場合、「C-D070：SSDデータコピー異常」を発生させる。\n.
//! ・データ移行状態が、「データ移行完了」の時、MicroSD起動の場合、「C-D071：MicroSD起動不可」を発生させる。\n.
//! <!--履歴-->.
//!  2018/09/21 Sky井柳 【DenebMLK】制御構成変更No78「データ移行」対応 新規作成.
//!  2018/11/08 Sky山岸 【DenebMLK】制御構成変更No78「データ移行」対応 getSsdDataTransferStatus()をgetSsdDataTransferStatus_SPIFlash()に変更.
// ================================================================================================.
void CP_HDDTrans::checkDataMove()
{
	// データ移行クラスのインスタンス取得.
	APIC_SsdDataTransfer *pc_APIC_SsdDataTransfer = APIC_SsdDataTransfer::newInstance();
	if( pc_APIC_SsdDataTransfer == NULL ){
		setDebugHddSeq( 2020, 0 );
		THRL_ThLogMsgErr("[%s] APIC_SystemSetting is NULL.\n", __FUNCTION__ );
		return;
	}
	
	// データ移行(MicroSD⇒SSD) 実行状態 取得.
	TYPE_SsdDataTransferStatus e_Status = pc_APIC_SsdDataTransfer->getSsdDataTransferStatus_SPIFlash();
	setDebugHddSeq( 2021, (uint32_t)e_Status );
	setDebugHddSeq( 2022, (uint32_t)theHDDTrans.e_StartUpDeviceType );
	
	ushort us_TroubleCode = 0;
	switch( e_Status ){
	case TYPE_SsdDataTransferStatus_Start:	// データ移行(MicroSD⇒SSD) 開始.
		if( theHDDTrans.e_StartUpDeviceType == THRE_StartupDeviceType_SSD ){	// SSD起動.
			THRL_ThLogMsgErr("[%s] DataMove Copy NG.\n", __FUNCTION__ );
			setDebugHddSeq( 2023, 0 );
			// 0xD070:データ移行 データコピー異常（SSDデータコピー異常）.
			us_TroubleCode = TYPD_TroubleCode_FATAL_DataMove_CopyNG;
		}
		break;
	case TYPE_SsdDataTransferStatus_End:	// データ移行(MicroSD⇒SSD) 完了.
		if( theHDDTrans.e_StartUpDeviceType == THRE_StartupDeviceType_SD ){		// MicroSD起動.
			THRL_ThLogMsgErr("[%s] BaseData Start Must Not.\n", __FUNCTION__ );
			setDebugHddSeq( 2024, 0 );
			// 0xD071:データ移行 元データ起動不可（MicroSD起動不可）.
			us_TroubleCode = TYPD_TroubleCode_FATAL_DataMove_BaseData_StartMustNot;
		}
		break;
	default:
		break;
	}

	if( us_TroubleCode != 0 ){	// トラブル発生.
		setDebugHddSeq( 2025, (uint32_t)us_TroubleCode );
		// 新詳細コード保存.
		uint64_t		ull_NewDetailData = THRD_NewDetail_ModuleId_Thread;		// 1,2桁：スレッド.
		ull_NewDetailData |= (uint64_t)( us_TroubleCode ) << THRD_NewDetail_TroubleCode_Shift;	// 3-6桁：トラブルコード.
		ull_NewDetailData |= (uint64_t)( e_Status ) << 24;										// 7-10桁：データ移行(MicroSD⇒SSD) 実行状態.
		ull_NewDetailData |= (uint64_t)( theHDDTrans.e_StartUpDeviceType ) << 8;				// 11-14桁：起動デバイス.
		THRL_ThAssertLog2( 0, ull_NewDetailData );
		DebugForever( us_TroubleCode );
	}
}

// ================================================================================================.
//! <!--機能--> データ移行用 元データデバイスが装着されているかを判定.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! なし.
//! <!--戻値-->.
//! Bool		True:データ移行用 元データデバイスが装着されている,  False:データ移行用 元データデバイスが装着されていない.
//! <!--詳細-->.
//! データ移行用 元データデバイスが装着されているかを判定し、.
//! 判定結果を戻り値で返す.
//! <!--履歴-->.
//! 2018/11/08 Sky山岸 【DenebMLK】AR.XXXXXXX 新規作成.
// ================================================================================================.
Bool THRG_IsDataMoveBaseDataDevice()
{
	return theHDDTrans.isDataMoveBaseDataDevice();
}

// ================================================================================================.
//! <!--機能--> initOSdriver成功状態取得.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! なし.
//! <!--戻値-->.
//! Bool ： True：initOSdriver正常終了、False：initOSdriver未終了（異常終了含む）.
//! <!--詳細-->.
//! initOSdriver成功状態を返す..
//! <!--履歴-->.
//! 2019/01/28 Sky井柳 【IT6_1.0MR】[制御構成変更_No.8] 【生産対応】初回起動時のロック解除対応：新規作成.
//! ================================================================================================.
Bool CP_HDDTrans::isInitOSdriverOK()
{
	return m_bl_initOSdriverOK;
}

// ================================================================================================
//! <!--機能--> 割込みチェック用関数コールスタート.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param uchar	関数コールポイント.
//! \param char*	コール関数名.
//! \param char		割込み対象種別 0:uc_HDDbuffer  1:puc_WorkMemAddr or getWorkMemAddr().
//! <!--戻値-->
//! \return なし
//! <!--詳細-->
//! バッファ[uc_HDDbuffer or puc_WorkMemAddr]使用中の再使用割込みチェックを行う\n
//! m_uc_FuncCallPointIntErr[KindId][0,1]: [1]処理中に[0]が割込んだログ (最新) \n
//! m_uc_FuncCallPointIntErr[KindId][2,3]: [3]処理中に[2]が割込んだログ (一つ前) \n
//! <!--履歴-->
// 2019/08/26 Sky鳥居 新規作成[EagleZ+] OP_BTS-11139対応.
// ================================================================================================
void CP_HDDTrans::FuncCallstart( uchar uc_PointId, const char *psc_funcname, uchar uc_KindId )
{
	// カウンタ上限値ガード.
	if( 0xFF > m_uc_FuncCallNestCnt[uc_KindId] ){
		m_uc_FuncCallNestCnt[uc_KindId]++;							// 関数コールネストカウンタUP.
	}
	if( us_debug_HDDdispparm > 0 ){
		if( NULL != psc_funcname ){
			SYSF_logMsg("[CP_HDDTrans::FuncCallstart][%d][%s] NestCnt[%d]\n", uc_PointId, psc_funcname, m_uc_FuncCallNestCnt[uc_KindId] );
		}
	}

	if((uchar)1 < m_uc_FuncCallNestCnt[uc_KindId] ){
	// 割り込み発生時、関数コールポイントのバックアップ.
		// 処理中の割込み発生.
		// 前回分.
		m_uc_FuncCallPointIntErr[uc_KindId][2] = m_uc_FuncCallPointIntErr[uc_KindId][0];	// 割込み用.
		m_uc_FuncCallPointIntErr[uc_KindId][3] = m_uc_FuncCallPointIntErr[uc_KindId][1];	// 割込まれ用.
		// 今回分.
		m_uc_FuncCallPointIntErr[uc_KindId][0] = uc_PointId;								// 割込み用.
		m_uc_FuncCallPointIntErr[uc_KindId][1] = m_uc_FuncCallPointCash[uc_KindId];			// 割込まれ用.

		if( m_uc_FuncCallIntErrCnt[uc_KindId] < 0xFF ){
			m_uc_FuncCallIntErrCnt[uc_KindId]++;					// 割込みエラーカウントアップ.
		}
		if( us_debug_HDDdispparm > 0 ){
			SYSF_logMsg("[CP_HDDTrans::FuncCallstart]IntErrCnt[%d] Int[%d][%d] OldInt[%d][%d]\n", m_uc_FuncCallIntErrCnt[uc_KindId],
								m_uc_FuncCallPointIntErr[uc_KindId][0], m_uc_FuncCallPointIntErr[uc_KindId][1],
								m_uc_FuncCallPointIntErr[uc_KindId][2], m_uc_FuncCallPointIntErr[uc_KindId][3] );
		}
	}else{
	// 割り込まれ関数コールポイントの保存.
		m_uc_FuncCallPointCash[uc_KindId] = uc_PointId;				// 関数コールポイントのキャッシュ.
	}
}

// ================================================================================================
//! <!--機能--> 割込みチェック用関数コールエンド.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param uchar	関数コールポイント.
//! \param char*	コール関数名.
//! \param char		割込み対象種別 0:uc_HDDbuffer  1:puc_WorkMemAddr or getWorkMemAddr().
//! <!--戻値-->
//! \return なし
//! <!--詳細-->
//! バッファ[uc_HDDbuffer]使用中の再使用割込みチェックを行う\n
//! <!--履歴-->
// 2019/08/26 Sky鳥居 新規作成[EagleZ+] OP_BTS-11139対応.
// ================================================================================================
void CP_HDDTrans::FuncCallend( uchar uc_PointId, const char *psc_funcname, uchar uc_KindId )
{
	// カウンタ下限値ガード.
	if((uchar)0 < m_uc_FuncCallNestCnt[uc_KindId] ){
		m_uc_FuncCallNestCnt[uc_KindId]--;				// 関数コールネストカウンタDOWN.
	}

	if( us_debug_HDDdispparm > 0 ) {
		if( NULL != psc_funcname ) {
			SYSF_logMsg("[CP_HDDTrans::FuncCallend  ][%d][%s] NestCnt[%d]\n", uc_PointId, psc_funcname, m_uc_FuncCallNestCnt[uc_KindId] );
		}
	}
}


// ================================================================================================
//! <!--機能--> SSDロック設定時の時、SSDアンロック済みか、SSDアンロック前かを判断する.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \なし	
//! <!--戻値-->
//! \return Bool	：	True : ロック中、False: アンロック(またはSSD以外）.
//! <!--詳細-->
//! SSDロック設定中でロック解除されているか判定する処理.
//! <!--履歴-->
// 2020/03/16 kondo 新規.
// ================================================================================================
Bool CP_HDDTrans::isSecurityLocked_SSD(void)
{
	if( uc_checkSecurityLocked == 0 ) {
		return False;
	}
	static ST_HDD_DEVICE_INFO	st_DeviceInfo;

	int32_t si32_DviceId = e_StartUpDeviceType == THRE_StartupDeviceType_SD ? THRE_SsdPortNo : THRE_HddPortNo;
	uint32_t ul_HddState = THRF_sysBspHddDeviceInfoGet( si32_DviceId, 0, &st_DeviceInfo);
	convertAndSetHDDStatus( ul_HddState );
	//[Debug]chkSaveIdentifyDevice 呼び出ししないようにしてデバイス情報コピー.
//	ushort us_Result = chkSaveIdentifyDevice( si32_DviceId, &st_DeviceInfo );
//	if(us_Result != HDD_STATUS_OK) {
//		SYSF_logMsg("isSecurityLocked_SSD : chkSaveIdentifyDevice NG.\n");
//		return False;
//	}
	
	if( THRE_StartupDeviceType_SSD == e_StartUpDeviceType ){	// 起動デバイスタイプSSDの時.
		//[Debug]chkSaveIdentifyDevice の代わりに情報コピー.
		// HDDのデバイス情報を内部のデータに保持.
		// セキュリティステイタスも含む.
		memcpy( &s_HDD_DEVICE_INFO[1], &st_DeviceInfo, sizeof( ST_HDD_DEVICE_INFO ) );

		return isSecurityLocked(1);
	}
	return False;
}

// ================================================================================================
//! <!--機能--> SSDロック状態設定.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param uchar	0:ロック無し,1:ロック.
//! <!--戻値-->
//! \return.
//! <!--詳細-->
//! SSDロック設定中でロック解除されているか判定する処理.
//! <!--履歴-->
// 2020/03/28 Tsukiji 新規.
// ================================================================================================
void CP_HDDTrans::setSecurityLocked( uchar uc_Lock )
{

	uc_checkSecurityLocked = uc_Lock ;	// セキュリティロック状態チェックフラグ追加.
	SYSF_logMsg("setSecurityLocked():uc_checkSecurityLocked=%d\n",uc_checkSecurityLocked);

}

// ================================================================================================
//! <!--機能--> SSDアクセス禁止状態設定処理.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param uchar	0:許可/1:禁止.
//! <!--戻値-->
//! \return.
//! <!--詳細-->
//! SSDアクセス禁止状態を設定する処理.
//! <!--履歴-->
// 2020/04/09 Natsume OP_BTS-22176対応.
// ================================================================================================
void CP_HDDTrans::setDisableSSDAccess( uchar uc_Disable )
{
	uc_disableSSDAccess = uc_Disable;	// SSDアクセス禁止状態.
	SYSF_logMsg("setDisableSSDAccess():uc_disableSSDAccess=%d\n",uc_disableSSDAccess);
}

// ================================================================================================
//! <!--機能--> SSDアクセス禁止状態取得処理.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param uchar	True:禁止/False:許可.
//! <!--戻値-->
//! \return.
//! <!--詳細-->
//! SSDアクセス禁止状態取得する処理.
//! <!--履歴-->
// 2020/04/09 Natsume OP_BTS-22176対応.
// ================================================================================================
Bool CP_HDDTrans::isDisableSSDAccess()
{
	Bool b_Disable = False;
	if( uc_disableSSDAccess == 1 ) {
		b_Disable = True;
	}
	return( b_Disable );
}

// ================================================================================================
//! <!--機能--> 強制的に再マウントする.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \なし	
//! <!--戻値-->
//! \return Bool	：	True : 正常、False: 異常.
//! <!--詳細-->
//! SSDロック設定中でロック解除されているか判定する処理.
//! <!--履歴-->
// 2020/03/16 tsukiji 新規.
// 2020/06/17 合屋 OP_BTS-24547 /ata01でReadOnlyでアクセスできないため、/ata01を追加.
// ================================================================================================
Bool CP_HDDTrans::forceReMount()
{
	//	DeepSleep復帰、SuperWarp起動時にBSPからコールされる.
	//	μSDやSSD（ロックなし）の場合は再マウントしない.
	Bool bl_isLock = isLockPassword();
	if( False == bl_isLock ) {
		SYSF_logMsg( "[forceReMount] Remount Skip()\n", bl_isLock );
		return True;
	}
	
	//	/ata02 ⇒ /ata09の順で、再マウントを行う.
	remountPartition( "/ata02", 15 );
	remountPartition( "/ata09", 13 );

	return True;
}

// ================================================================================================
//! <!--機能--> パーティション1つ分の強制再マウント.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \char* psc_MountPoint		マウントポイント.
//! \uchar uc_PartNo			パーティション番号.
//! <!--戻値-->
//! \return Bool	：	True : 正常、False: 異常.
//! <!--詳細-->
//! 指定されたパーティションのumountとmountを行う.
//! <!--履歴-->
// 2020/06/17 合屋 OP_BTS-24547 新規追加.
// ================================================================================================
Bool CP_HDDTrans::remountPartition( char* psc_MountPoint, uchar uc_PartNo )
{
	char sc_DevPath[ 256 ] = { 0 };
	char sc_Cmd[ 512 ] = { 0 };
	
	//	エラーチェックはSSD固定で行う.
	if ( ( NULL == psc_MountPoint ) || ( sizeof(ssd_part_info_256GB) / sizeof(part_layout) ) < uc_PartNo ) {
		SYSF_logMsg( "[remountPartition] ParamError MountPoint(0x%p) or PartNo(%d)\n", psc_MountPoint, uc_PartNo );
		return False;
	}
	
	//	デバイスファイルは暗号化設定によって切り替える.
	if ( True == THRG_IsSsdEncrypt() ) {
		snprintf( sc_DevPath, sizeof(sc_DevPath), "/dev/mapper/crypt_%d", uc_PartNo );
	} else {
		snprintf( sc_DevPath, sizeof(sc_DevPath), "/dev/kmsda%d", uc_PartNo );
	}
	
	//	umount ⇒ mountを行う.
	//	umountはビジーを考慮し、最大50回リトライする.
	Bool bl_Ret = False;
	uchar uc_Cnt = 0;
	uchar uc_Cnt2 = 0;
	int32_t si_ErrNoMount = 0;
	int32_t si_ErrNoUmount = 0;
	int32_t si_Ret = 0;
	
	for ( uc_Cnt = 0; uc_Cnt < THRD_UMOUNT_RETRYMAX; uc_Cnt++ ) {
		
		snprintf( sc_Cmd, sizeof(sc_Cmd), "umount %s", psc_MountPoint );
		si_Ret = THRF_System( sc_Cmd );
		si_ErrNoUmount = SYSF_errnoGet();
		if( 0 == si_Ret  ) {
			
			SYSF_logMsg( ">>>umount %s OK!!\n", psc_MountPoint );
			
			//	アンマウントに成功したのでマウントを行う.
			snprintf( sc_Cmd, sizeof(sc_Cmd), "mount %s %s", sc_DevPath, psc_MountPoint );
			for ( uc_Cnt2 = 0; uc_Cnt2 < THRD_UMOUNT_RETRYMAX; uc_Cnt2++ ) {
				si_Ret = THRF_System( sc_Cmd );
				si_ErrNoMount = SYSF_errnoGet();
				if( 0 == si_Ret  ) {
					SYSF_logMsg( ">>>mount %s OK!!\n", psc_MountPoint );
					bl_Ret = True;
					break;
				}
				SYSF_taskDelay( 2 );
			}
			if ( THRD_UMOUNT_RETRYMAX <= uc_Cnt2 ) {
				SYSF_logMsg( "[remountPartition] mount Error.%s -> %s errno:%d\n", sc_DevPath, psc_MountPoint, si_ErrNoMount );
			}
			break;
		} else {
			//	ビジーでumountに失敗することがあるので、10msecの待ちを入れる.
			//	最大で50回リトライ.
			SYSF_taskDelay( 2 );
		}
	}
	
	if( THRD_UMOUNT_RETRYMAX == uc_Cnt ) {
		SYSF_logMsg( "[remountPartition] Part(%d) Point(%s) Retry Over(%d)\n", uc_PartNo, psc_MountPoint, si_ErrNoUmount );
	}

	return bl_Ret;
}

// ================================================================================================
//! <!--機能--> バックアップしているMaxCluster値と現在の値が一致しているか判定.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param Bool bl_Recover				:	True:リカバリ有効 False:リカバリ無効.
//! <!--戻値-->
//! \return なし
//! <!--詳細-->
//! \n
//! <!--履歴-->
// 2020/03/27 Tsukiji [EagleBK]D030対応.
// ================================================================================================
void CP_HDDTrans::check_BackupMaxCluster( Bool bl_Recover )
{
//	int32_t si_RetryCount = 0;										// Retry用カウンタ.
	uint32_t ul_StoredMaxClsNum1 = 0;								// 1面目の最大クラスタ数.
	uint32_t ul_StoredMaxClsNum2 = 0;								// 2面目の最大クラスタ数.
//	for( si_RetryCount = 0; si_RetryCount <= THRD_MaxClsNumRetryMax; si_RetryCount++ ){
		ul_StoredMaxClsNum1 = judge_BackupMaxCluster( THRE_FatSectorFirst, bl_Recover  );					// 1面目の最大クラスタ数.
		ul_StoredMaxClsNum2 = judge_BackupMaxCluster( THRE_FatSectorSecond, bl_Recover  );					// 2面目の最大クラスタ数.
//	}
}
// ================================================================================================
//! <!--機能--> バックアップしてある情報を読み出し判定.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param THRE_FatSector e_FatSector	:	Fatセクタ指定.
//! \param Bool bl_Recover				:	True:リカバリ有効 False:リカバリ無効.
//! <!--戻値-->
//! \return uint32_t ul_Ret				:	0:読み出し失敗 それ以外:読み出し・書き込み最大セクタ数. 
//! <!--詳細-->
//! HDD上に書き込まれている蓄積領域の最大クラスタ数を保存域となるFatセクタ指定に応じた場所から取得する.
//! バックアップ値と比較し、一致しなければリカバリする
//! <!--履歴-->
// 2020/03/27 Tsukiji [EagleBK]D030対応.
// ================================================================================================
uint32_t CP_HDDTrans::judge_BackupMaxCluster( THRE_FatSector e_FatSector, Bool bl_Recover )
{
	uint32_t		ul_StartBlock = 0;	// store用FAT1開始ブロック.//HDD.

	FuncCallstart( 31, __FUNCTION__, 0 );		// 割込みチェック用関数コールスタート.

	if( e_FatSector == THRE_FatSectorFirst ) {	// セクタ指定確認.
		ul_StartBlock = getStorageInfo(THRE_StgInfoSectFatStore1);	// store用FAT1開始ブロック.//HDD.
	} else {
		ul_StartBlock = getStorageInfo(THRE_StgInfoSectFatStore2);	// store用FAT2開始ブロック.//HDD.
	}
	void*			pv_Adder = uc_HDDbuffer;		// HDDバッファのアドレス.
	int64_t			sll_Syscall_Ret = 0;			// システムコールの戻り値判定用.
	int				si_fdp = 0;						// ファイルディスクリプタ.
	uint32_t 		ul_Ret = 0;						// 戻り値(最大クラスタ数).
	uint32_t 		ul_ClusterTmp = 0;						// 戻り値(最大クラスタ数).
	uint32_t		ul_Size = 4;

	// HDDデバイスオープン.
	si_fdp = SYSF_devopen( theHDDTrans.pc_strDeviceSSD, O_RDWR | O_CLOEXEC, 0666 );
	if( si_fdp < 0 ){
		printf( "[judge_BackupMaxCluster]fail SYSF_devopen. errno:%d\n",errno );
		FuncCallend( 31, __FUNCTION__, 0 );			// 割込みチェック用関数コールエンド.
		return ul_Ret;
	}

	// 最大クラスタ数が書き込まれている位置までシーク.
	sll_Syscall_Ret = lseek( si_fdp, (long long)ul_StartBlock * (long long)DEF_SECTOR_SIZE, SEEK_SET );
	if( sll_Syscall_Ret < 0 ){
		SYSF_devclose(si_fdp);
		printf( "[judge_BackupMaxCluster]fail lseek command. fd:%d, StartBlock:%lu, Syscall_Ret:%"PRIx64", errno:%d\n",
			si_fdp,(uint32_t)ul_StartBlock,(int64_t)sll_Syscall_Ret, errno );
		FuncCallend( 31, __FUNCTION__, 0 );			// 割込みチェック用関数コールエンド.
		return ul_Ret;
	}

	// 最大クラスタ数の読み込み(先頭4byte).
	sll_Syscall_Ret = read( si_fdp, (char *)pv_Adder ,ul_Size );
	if( sll_Syscall_Ret < 0 ){
		SYSF_devclose(si_fdp);
		printf( "[judge_BackupMaxCluster]fail read command. fd:%d, Adder:%08lx, Syscall_Ret:%"PRIx64", errno:%d",
			si_fdp, (uintptr_t)pv_Adder, (int64_t)sll_Syscall_Ret, errno );
		FuncCallend( 31, __FUNCTION__, 0 );			// 割込みチェック用関数コールエンド.
		return ul_Ret;
	}
	SYSF_devclose(si_fdp);	// デバイスクローズ.

	// Maxクラスタ値読み込み設定.
	memcpy( &ul_ClusterTmp, pv_Adder, 4 );
	ul_Ret = ul_ClusterTmp;	//読み込み値設定.

	if( e_FatSector == THRE_FatSectorFirst ) {	// セクタ指定確認.
		// FAT1st.
		if( ul_ClusterTmp != ul_BackupMaxCluster1st ) {
			//リカバリ指定がある場合はリカバリする、なければ判定のみ.
			if( bl_Recover == True ) {
				//不一致だった場合、バックアップ値をリカバリ.
				ul_Ret = recover_BackupMaxCluster( e_FatSector, ul_BackupMaxCluster1st );
			}
			printf( "[judge_BackupMaxCluster] Fat1st unmatch ul_ClusterTmp:%lu ul_BackupMaxCluster1st=%lu\n"
				,(uint32_t)ul_ClusterTmp,(int32_t)ul_BackupMaxCluster1st);
		}
	} else {
		// FAT2nd.
		if( ul_ClusterTmp != ul_BackupMaxCluster2nd ) {
			//リカバリ指定がある場合はリカバリする、なければ判定のみ.
			if( bl_Recover == True ) {
				//不一致だった場合、バックアップ値をリカバリ.
				ul_Ret = recover_BackupMaxCluster( e_FatSector, ul_BackupMaxCluster2nd );
			}
			printf( "[judge_BackupMaxCluster] Fat2nd unmatch ul_ClusterTmp:%lu ul_BackupMaxCluster2nd=%lu\n"
				,(uint32_t)ul_ClusterTmp,(int32_t)ul_BackupMaxCluster2nd);
		}
	}
	FuncCallend( 31, __FUNCTION__, 0 );			// 割込みチェック用関数コールエンド.

	return ul_Ret;
}
// ================================================================================================
//! <!--機能--> バックアップ値のリカバリ処理.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param THRE_FatSector e_FatSector	:	Fatセクタ指定.
//! \param uint32_t ul_ClusterNum:	設定クラスタ値.
//! <!--戻値-->
//! \return なし
//! <!--詳細-->
//! HDD上に書き込まれている蓄積領域の最大クラスタ数を保存域となるFatセクタ指定に応じた場所から取得する.
//! バックアップ値をリカバリする
//! <!--履歴-->
// 2020/03/27 Tsukiji [EagleBK]D030対応.
// ================================================================================================
uint32_t CP_HDDTrans::recover_BackupMaxCluster( THRE_FatSector e_FatSector, uint32_t ul_ClusterNum )
{
	uint32_t		ul_StartBlock = 0;	// store用FAT1開始ブロック.//HDD.
	void*			pv_Adder = uc_HDDbuffer;		// HDDバッファのアドレス.
	int				si_fdp = 0;						// ファイルディスクリプタ.
	uint32_t 		ul_Ret = 0;						// 戻り値(最大クラスタ数).
	uint32_t		ul_Size = 4;
	int64_t			sll_Syscall_Ret = 0;			// システムコールの戻り値判定用.

	FuncCallstart( 32, __FUNCTION__, 0 );		// 割込みチェック用関数コールスタート.

	if( e_FatSector == THRE_FatSectorFirst ) {	// セクタ指定確認.
		ul_StartBlock = getStorageInfo(THRE_StgInfoSectFatStore1);	// store用FAT1開始ブロック.//HDD.
	} else {
		ul_StartBlock = getStorageInfo(THRE_StgInfoSectFatStore2);	// store用FAT2開始ブロック.//HDD.
	}
	// HDDデバイスオープン.
	si_fdp = SYSF_devopen( theHDDTrans.pc_strDeviceSSD, O_RDWR | O_CLOEXEC, 0666 );
	if( si_fdp < 0 ){
		printf( "[recover_BackupMaxCluster]fail SYSF_devopen. errno:%d\n",errno );
		FuncCallend( 32, __FUNCTION__, 0 );			// 割込みチェック用関数コールエンド.
		return ul_Ret;
	}

	// 最大クラスタ数が書き込まれている位置までシーク.
	sll_Syscall_Ret = lseek( si_fdp, (long long)ul_StartBlock * (long long)DEF_SECTOR_SIZE, SEEK_SET );
	if( sll_Syscall_Ret < 0 ){
		SYSF_devclose(si_fdp);
		printf( "[recover_BackupMaxCluster]fail lseek command. fd:%d, StartBlock:%lu, Syscall_Ret:%"PRIx64", errno:%d\n",
			si_fdp,(uint32_t)ul_StartBlock,(int64_t)sll_Syscall_Ret, errno );
		FuncCallend( 32, __FUNCTION__, 0 );			// 割込みチェック用関数コールエンド.
		return ul_Ret;
	}

	// 戻り値の設定.
	memcpy(pv_Adder, &ul_ClusterNum, 4 );
	// 最大クラスタ数の書き込み.
	sll_Syscall_Ret = write( si_fdp, (char *)pv_Adder ,ul_Size );
	if( sll_Syscall_Ret < 0 ){
		SYSF_devclose(si_fdp);
		printf( "[recover_BackupMaxCluster]fail write command. fd:%d, Adder:%08lx, Syscall_Ret:%"PRIx64", errno:%d",
			si_fdp, (uintptr_t)pv_Adder, (int64_t)sll_Syscall_Ret, errno );
		FuncCallend( 32, __FUNCTION__, 0 );			// 割込みチェック用関数コールエンド.
        return ul_Ret;
    }
	SYSF_devclose(si_fdp);	// デバイスクローズ.
	ul_Ret = ul_ClusterNum;

	//リカバリした事をコードに残す.
	// 新詳細コード保存(トラブル出さないが詳細コードだけ登録..
	set_BackupDetailError( TYPD_DetailTroubleCode_C0060_DebugFatal, e_FatSector, ul_ClusterNum, 0 );

	printf( "[recover_BackupMaxCluster] Recover MaxCluster  %lu:%lu",
			(uint32_t)e_FatSector, (uint32_t)ul_ClusterNum );

	FuncCallend( 32, __FUNCTION__, 0 );			// 割込みチェック用関数コールエンド.
	return ul_Ret;
}

// ================================================================================================
//! <!--機能--> バックアップ処理の詳細コード設定.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param uint32_t ul_ErrCode:	エラーコード(0x4x/0x5x/0x6x).
//! \param uint32_t ul_Para1:	引数1.
//! \param uint32_t ul_Para2:	引数2.
//! \param uchar uc_Para3:	引数3.
//! <!--戻値-->
//! \return なし
//! <!--詳細-->
//! スレッドの詳細コード設定.
//! <!--履歴-->
// 2020/03/27 Tsukiji [EagleBK]D030対応.2006
// ================================================================================================
void CP_HDDTrans::set_BackupDetailError( uint32_t ul_ErrCode, uint32_t ul_Para1, uint32_t ul_Para2, uchar uc_Para3 )
{
	// 新詳細コード保存.
	uint64_t		ull_NewDetailData = THRD_NewDetail_ModuleId_Thread;		// 1,2桁：スレッド.
	ull_NewDetailData |= (uint64_t)( ul_ErrCode ) << THRD_NewDetail_TroubleCode_Shift;	// 3-6桁：トラブルコード.
	ull_NewDetailData |= (uint64_t)( ul_Para1 ) << 32;									// 7-8桁：.
	ull_NewDetailData |= (uint64_t)( ul_Para2 );										// 9-16桁：.
//	ull_NewDetailData |= (uint64_t)( uc_Para3 );										// ---.
	THRL_ThAssertLog2( 0, ull_NewDetailData );
}

uint32_t CP_HDDTrans::dsp_BackupMaxCluster()
{
	uint32_t 		ul_Ret = 0;						// 戻り値(最大クラスタ数).

	printf( "[getBackupMaxCluster] ul_BackupMaxCluster1st=%lu\n", ul_BackupMaxCluster1st );
	printf( "[getBackupMaxCluster] ul_BackupMaxCluster2nd=%lu\n", ul_BackupMaxCluster2nd );
	return ul_Ret;
}

uint32_t CP_HDDTrans::test_BackupMaxCluster( THRE_FatSector e_FatSector, uint32_t ul_ClusterNum )
{

	uint32_t 		ul_Ret = 0;						// 戻り値(最大クラスタ数).

	ul_Ret = recover_BackupMaxCluster( e_FatSector, ul_ClusterNum );
	
	return ul_Ret;
}

//2020/12/14 Tsukiji OP_BTS-29664 FWエラーが発生する問題対応(暫定)
//ターミナル出力設定SoftwareDIPSW保存.
#define	THRD_BACKUPDIR_SOFTSW	      THRD_DevName_Ssd0a	//保存領域.
#define	THRD_BACKUP_SOFTSW	          "/backupsw.dat"		//保存ファイル名.
void BackupSoftSW( uchar uc_SWvalue )
{
	FILE *fp = NULL;
	char fname[40];
	uchar buff[1];
	memset( fname,0,sizeof(fname) );
	memcpy( fname,THRD_BACKUPDIR_SOFTSW,sizeof(THRD_BACKUPDIR_SOFTSW) );
	memcpy( fname+sizeof(THRD_BACKUPDIR_SOFTSW)-1,THRD_BACKUP_SOFTSW,sizeof(THRD_BACKUP_SOFTSW) );
	fp = fopen( fname, "wb+" );
	if( fp ) {
		buff[0] = uc_SWvalue;
		if( fwrite( buff, 1, sizeof(buff), fp ) < 0 ) {
			printf( "[%s] SoftSW backup write failed '%s' [%d]\n",__FUNCTION__, fname, uc_SWvalue );
		}
		fclose( fp );
		printf( "[%s] SoftSW backup '%s' [%d]\n",__FUNCTION__, fname, uc_SWvalue );
	} else {
		printf( "[%s] SoftSW backup failed '%s'\n",__FUNCTION__, fname );
	}
}
//ターミナル出力設定SoftwareDIPSW取得と一時データ削除.
Bool RestoreSoftSW( uchar *uc_SWvalue )
{
	Bool bl_Ret = False;
	FILE *fp = NULL;
	char fname[40];
	uchar buff[1];
	memset( fname,0,sizeof(fname) );
	memcpy( fname,THRD_BACKUPDIR_SOFTSW,sizeof(THRD_BACKUPDIR_SOFTSW) );
	memcpy( fname+sizeof(THRD_BACKUPDIR_SOFTSW)-1,THRD_BACKUP_SOFTSW,sizeof(THRD_BACKUP_SOFTSW) );
	buff[0] = 0;
	fp = fopen( fname, "rb" );
	if( fp ) {
		if( fread( buff, 1, sizeof(buff), fp ) < 0 ) {
			printf( "[%s] SoftSW restore write failed '%s' [%d]\n",__FUNCTION__, fname, uc_SWvalue );
		}
		fclose( fp );
		//ファイル削除.
		remove( fname );
		*uc_SWvalue = buff[0];	//有効時のみ書き換え.
		//ファイル存在していたら有効.
		bl_Ret = True;
		printf( "[%s] SoftSW restore '%s' [%d]\n",__FUNCTION__, fname, *uc_SWvalue );
	} else {
		printf( "[%s] SoftSW restore nothing! '%s'\n",__FUNCTION__, fname );
	}
	return bl_Ret;
}

/*================================================================================================
［機能］	A800レス対応判定処理.
--------------------------------------------------------------------------------------------------
［引数］	const char *psc_FwPath:更新用FWのディレクトリへのフルパス.
［戻値］	A800レス対応済みFWならTrue、それ以外(A800レス未対応/ファイル操作エラーなど)はFalseを返す.
［詳細］	INDEXファイルの記載からFWがA800レス対応かどうかを判定する.
［履歴］	2021/07/09 A800レス実機への旧ROMのNWDLガード対応.
================================================================================================*/
#if defined(DEF_EagleCus) && !defined(DEF_A800) && !defined(DEF_HeliosMLKCus)
#define THRD_MaxLength_INDEXFilePath   (512)        // INDEXの最大ファイルパス長.
#define THRD_MaxLength_INDEXFile1Line  (1024)       // INDEXファイル内の1行の最大長.
#define THRD_String_A800LessFW         ("@A8L=1")   // A800レス対応FWを示す文字列.

static Bool isA800LessFW( const char *psc_FwPath )
{
	Bool ret = False; //［戻値］A800レス対応済みFWならTrue、それ以外(A800レス未対応/ファイル操作エラーなど)はFalseを返す.

	Bool isError = False;
	FILE *fpINDEX = NULL; // "INDEX"ファイルへのFILEポインタ.

	if( NULL==psc_FwPath )
	{
		printf("[%s] psc_FwPath=(NULL)\n", __FUNCTION__);
		isError = True;
	}

	// INDEXファイルをOpen.
	if( !isError )
	{
		char FilePath[THRD_MaxLength_INDEXFilePath+1];
        snprintf(FilePath, sizeof(FilePath), "%s/INDEX", psc_FwPath);
		fpINDEX = fopen(FilePath, "r");
		if( NULL==fpINDEX )
		{
			printf("[%s] can't open [%s]\n", __FUNCTION__, FilePath);
			isError = True;
		}
	}

	if( !isError )
	{
		char FWIndexData[THRD_MaxLength_INDEXFile1Line+1] = {0};

		Bool ContinueLoop = True; // ループ処理制御用変数: 処理を継続＝True、処理を停止＝False.
		do
		{
			// 1行だけ読み込む.
			if( NULL==fgets(FWIndexData, sizeof(FWIndexData), fpINDEX) )
			{
				// ファイルの最後であればループを終了する.
				ContinueLoop = False;
			}

			if( ContinueLoop )
			{
				// A800レス対応FWを示す文字列"@A8L=1"があるか？.
				if( 0==strncmp(FWIndexData, THRD_String_A800LessFW, sizeof(THRD_String_A800LessFW)-1) )
				{
					ret = True; //［戻値］A800レス対応済みFWならTrue.

					ContinueLoop = False; // ループを終了する.
				}
			}
		} while( ContinueLoop );
	}

	if( NULL!=fpINDEX )
	{
		// INDEXファイルをClose.
		fclose( fpINDEX );
		fpINDEX = NULL;
	}

	return ret;
}
#endif

// ================================================================================================
//! <!--機能--> 「一般ユーザーアクセス変換管理ファイル」を読み込む.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \std::map<std::string, Bool>& ctrlFileMap	「一般ユーザーアクセス変換管理ファイル」のマップファイル.
//! <!--戻値-->
//! \return Bool.
//! <!--詳細-->
//! 「一般ユーザーアクセス変換管理ファイル」の読み込み.
//! <!--履歴-->
// 2021/06/16 ADS Kawai.
// ================================================================================================
Bool readMfpAccessCtrlFile( std::map<std::string, Bool>& ctrlFileMap )
{
	ctrlFileMap.clear();
	ifstream ifs( THRD_CtrlFile );
	
	Bool flg_end = False;
	std::string str = "";
	while( getline( ifs, str ) ) {
		istringstream stream( str );
		int32_t i = 0;
		std::string tmp = "";		// getlineの区切り文字仮格納用.
		Bool tmpValue = False;		// マップファイル:Value(変換済フラグ).
		if( str == "@end" ) {
			flg_end = True;
			break;
		} else {
			while ( getline( stream, tmp, ',' ) ) {
				if( i == 0 ) {			// 変換済フラグ.
					if( tmp == "0" ) {
						tmpValue = False;
					} else if( tmp == "1" ) {
						tmpValue = True;
					} else {
						printf( "[%s:%d] converted flag NG. converted_flag:%s\n", __PRETTY_FUNCTION__, __LINE__, tmp.c_str() );
						ifs.close();
						return False;
					}
				} else if( i == 1 ) {	// マウントポイント.
					ctrlFileMap[ tmp ] = tmpValue;
				} else {
					// 以降のデータは不要.
					break;
				}
				i++;
			}
		}
	}
	ifs.close();

	if ( flg_end == False  ) {
		printf( "[%s:%d] %s file is corrupted\n", __PRETTY_FUNCTION__, __LINE__, THRD_CtrlFile );
		ctrlFileMap.clear();
		return False;
	}

	return True;
}

// ================================================================================================
//! <!--機能--> 「一般ユーザーアクセス変換管理ファイル」に書き込む.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! std::map<std::string, Bool>& ctrlFileMap	「一般ユーザーアクセス変換管理ファイル」のマップファイル.
//! <!--戻値-->
//! \return Bool.
//! <!--詳細-->
//! 「一般ユーザーアクセス変換管理ファイル」に書き込み.
//! <!--履歴-->
// 2021/06/16 ADS Kawai.
// ================================================================================================
Bool writeMfpAccessCtrlFile( std::map<std::string, Bool>& ctrlFileMap )
{
	int32_t si_File = SYSF_open( THRD_CtrlTmpFile, O_CREAT | O_RDWR, 0600 );
	if( si_File == -1 ) {
		printf( "[%s:%d] open NG. Syscall_Ret:%d, path:%s, errno:%d\n", __PRETTY_FUNCTION__, __LINE__, si_File, THRD_CtrlTmpFile, SYSF_errnoGet() );
		return False;
	}
	
	std::string tmpdata = "";
	for( std::map<std::string, Bool>::iterator itr = ctrlFileMap.begin(); itr != ctrlFileMap.end(); ++itr ) {
		string svalue = itr->second?"1":"0";
		tmpdata += svalue + "," + ((std::string)itr->first) + "\n";
	}
	tmpdata += "@end\n";
	
	off_t si_Ret = SYSF_write( si_File, const_cast< char* >(tmpdata.c_str()), tmpdata.size() );
	if( (unsigned)si_Ret != tmpdata.size() ) {
		printf( "[%s:%d] write NG. Syscall_Ret:%ld, size:%lo, errno:%d\n", __PRETTY_FUNCTION__, __LINE__, si_Ret, tmpdata.size(), SYSF_errnoGet() );
		SYSF_close( si_File );
		sync();
		int removeResult = remove( THRD_CtrlTmpFile );
		if( removeResult != 0 ) {
			printf( "[%s:%d] remove NG. Syscall_Ret:%ld, path:%s, errno:%d\n", __PRETTY_FUNCTION__, __LINE__, si_Ret, THRD_CtrlTmpFile, SYSF_errnoGet() );
		}
		return False;
	}
	SYSF_close( si_File );
	sync();
	
	STATUS si_Ren = SYSF_rename( THRD_CtrlTmpFile, THRD_CtrlFile );
	if( si_Ren == ERROR ) {
		printf( "[%s:%d] rename NG. Syscall_Ret:%d, path:%s, errno:%d\n", __PRETTY_FUNCTION__, __LINE__, si_Ren, THRD_CtrlTmpFile, SYSF_errnoGet() );
		return False;
	}
	return True;
}

// ================================================================================================
//! <!--機能--> 「一般ユーザーアクセス変換対象リスト」の取得.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! std::string& targetMntPoint				引数1	in 取得対象のマウントポイント.
//! std::vector<THRS_CnvTargetInfo>& targetList	引数2	out 一般ユーザーアクセス変換対象リスト.
//! <!--戻値-->
//! \return なし.
//! <!--詳細-->
//! 「一般ユーザーアクセス変換対象リスト」を取得.
//! <!--履歴-->
// 2021/06/16 ADS Kawai.
// ================================================================================================
void getMfpAccessTargetList( std::string& targetMntPoint, std::vector<THRS_CnvTargetInfo>& targetList )
{
	targetList.clear();
	ifstream ifs( THRD_MfpAccessTargetList );

	std::string str = "";
	while( getline( ifs, str ) ) {
		istringstream stream( str );
		int32_t i = 0;
		std::string tmp = "";		// getlineの区切り文字仮格納用.
		THRS_CnvTargetInfo tmpInfo;
		while ( getline( stream, tmp, ',' ) ) {
			if( i == 0 ) {	// マウントポイント.
				if( targetMntPoint != tmp ) {
					// 対象のマウントポイントではない.
					break;
				}
			} else if( i == 1 ) {	// 変換処理区分.
				tmpInfo.kind = atoi( tmp.c_str() );
			} else if( i == 2 ) {	// 変換対象区分.
				tmpInfo.target = atoi( tmp.c_str() );
			} else if( i == 3 ) {	// ディレクトリ／ファイルパス.
				tmpInfo.path = tmp;
				targetList.push_back( tmpInfo );
			} else {
				// 以降は不要.
				break;
			}
			i++;
		}
	}

	ifs.close();
}
// ================================================================================================
//! <!--機能--> mode文字列変換.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param const char*	mode文字列
//! \param mode_t		現在のmode値
//! <!--戻値-->
//! \return int			処理結果.
//! <!--詳細-->
//! mode文字列をmode_tに変換する。不正文字列が設定されている場合は-1を返却する。
//! [ugoa]*([-+=]([rwxXst]*|[ugo]))+|[-+=][0-7]+形式以外は不正扱い。
//! <!--履歴-->
//  22/06/XX ADS Nakai.
// ================================================================================================
static int parse_mode(const char* str, mode_t current_mode)
{
	// oXXXX指定検索.
	if ((unsigned char)(*str - '0') < 8) {
		unsigned long oct_mode;
		char* end;
		oct_mode = strtoul(str, &end, 8);
		if (*end || (oct_mode > 07777U)) {
			printf("[%s:%d] end:%s oct_mode:%#lo\n", __PRETTY_FUNCTION__, __LINE__, end, oct_mode);
			return -1;
		}
		return (int)oct_mode;
	}
	
	bool search = false;
	mode_t wholist = 0;
	mode_t permlist = 0;
	mode_t new_mode = 0;
	char op = 0;

	new_mode = current_mode;

	while (*str) {
		if (*str == ',') {
			++str;
			continue;
		}
		
		// 設定対象(augo)文字列を検索.
		wholist = 0;
		search = true;
		while (search) {
			switch (*str) {
				case 'a': wholist |= S_ISUID | S_ISGID | S_ISVTX | S_IRWXU | S_IRWXG | S_IRWXO; if (!*++str) return -1; break;
				case 'u': wholist |= S_ISUID | S_IRWXU;                                         if (!*++str) return -1; break;
				case 'g': wholist |= S_ISGID | S_IRWXG;                                         if (!*++str) return -1; break;
				case 'o': wholist |= S_IRWXO;                                                   if (!*++str) return -1; break;
				default:  search = false; break; // 対象文字列が見つからない時点で検索終了.
			}
		}

		do {
			if ((*str != '+') && (*str != '-')) {
				if (*str != '=') { // +-=以外は不正指定.
					printf("[%s:%d] str:%s != '='\n", __PRETTY_FUNCTION__, __LINE__, str);
					return -1;
				}
				permlist = ~(S_ISUID | S_ISGID | S_ISVTX | S_IRWXU | S_IRWXG | S_IRWXO);
				if (wholist)
					permlist = ~wholist;
				new_mode &= permlist;
			}
			op = *str++;
			
			// +-=後のugo指定確認.
			search = true;
			switch (*str) {
				case 'u': permlist = new_mode & (S_IRWXU | S_IRWXG | S_IRWXO) & (S_ISUID | S_IRWXU); break;
				case 'g': permlist = new_mode & (S_IRWXU | S_IRWXG | S_IRWXO) & (S_ISGID | S_IRWXG); break;
				case 'o': permlist = new_mode & (S_IRWXU | S_IRWXG | S_IRWXO) & (S_IRWXO);           break;
				default:  search = false; break; //ugo指定無し.
			}
			
			if ( search ) { // +-=後のugo指定がある場合.
				if (permlist & (S_IRUSR | S_IRGRP | S_IROTH)) permlist |= (S_IRUSR | S_IRGRP | S_IROTH);
				if (permlist & (S_IWUSR | S_IWGRP | S_IWOTH)) permlist |= (S_IWUSR | S_IWGRP | S_IWOTH);
				if (permlist & (S_IXUSR | S_IXGRP | S_IXOTH)) permlist |= (S_IXUSR | S_IXGRP | S_IXOTH);
				++str;
			} else { // +-=後のugo指定がない場合.
				permlist = 0;
				search = true;
				while (search) {
					// +-=後のrwxXst指定検索.次の文字列がないまたは対象文字列がない場合は検索終了.
					switch (*str) {
						case 'r': permlist |= (S_IRUSR | S_IRGRP | S_IROTH); if (!*++str) search = false; break;
						case 'w': permlist |= (S_IWUSR | S_IWGRP | S_IWOTH); if (!*++str) search = false; break;
						case 'x': permlist |= (S_IXUSR | S_IXGRP | S_IXOTH); if (!*++str) search = false; break;
						case 's': permlist |= (S_ISUID | S_ISGID);           if (!*++str) search = false; break;
						case 't': permlist |= (S_ISVTX);                     if (!*++str) search = false; break;
						case 'X': 
							if (new_mode & (S_IFDIR | S_IXUSR | S_IXGRP | S_IXOTH))
								permlist |= (S_IXUSR | S_IXGRP | S_IXOTH);
							if (!*++str)
								search = false; 
							break;
						default: search = false; break;
					}
				}
			}
			
			// mode算出.
			if (permlist) {
				mode_t tmp = wholist;
				if (!wholist) {
					mode_t u_mask = umask(0);
					umask(u_mask);
					tmp = ~u_mask;
				}
				permlist &= tmp;
				if (op == '-') {
					new_mode &= ~permlist;
				} else {
					new_mode |= permlist;
				}
			}
		} while (*str && (*str != ','));
	}
	return new_mode;
}

// ================================================================================================
//! <!--機能--> ファイル属性変更.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param const char*	変更対象path
//! \param uid_t		所有者
//! \param gid_t		グループ
//! \param const char*	権限
//! <!--戻値-->
//! \return int			処理結果.
//! <!--詳細-->
//! 所有者、グループ、権限を一括変更する
//! immutable設定ファイルは解除して変更後再設定する
//! <!--履歴-->
//  22/06/XX ADS Nakai.
// ================================================================================================
static int change_attribute(const char* path, struct stat* statorg, uid_t uid, gid_t gid, const char* modestr)
{	
	if (!(path && statorg && modestr)) {
		printf("[%s:%d] path:%p statorg:%p modestr:%p\n", __PRETTY_FUNCTION__, __LINE__, path, statorg, modestr);
		return False;
	}
	
	if (S_ISLNK(statorg->st_mode)) {
		if ((statorg->st_uid == uid) 
		 && (statorg->st_gid == gid)) {
		 	// 変更対象無しのため正常終了.
		 } else {
		 	// linkはimmutable設定不可でchmodコマンドでlchmod行わないためlchownのみ実施。
			if (-1 == lchown(path, uid, gid)) {
				printf("[%s:%d] errno:%d = lchown(%s, %d, %d)\n", __PRETTY_FUNCTION__, __LINE__, SYSF_errnoGet(), path, uid, gid);
				return False;
			}
		}
		return True;
	}

	mode_t mode = parse_mode(modestr, statorg->st_mode);
	if (-1 == (int)mode) {
		printf("[%s:%d] mode:%d modestr:%s statorg->st_mode:%#o\n", __PRETTY_FUNCTION__, __LINE__, mode, modestr, statorg->st_mode);
		return False;
	}
	// 変更対象無しのため正常終了.
	if ((statorg->st_uid == uid) 
	 && (statorg->st_gid == gid) 
	 && (statorg->st_mode == mode)) {
		return True;
	}
	
	Bool result = True;
	int fd = open(path, (O_RDONLY|O_NONBLOCK));
	if (-1 != fd) {
		int attrOrg = 0;
		if (-1 == ioctl(fd, FS_IOC_GETFLAGS, &attrOrg)) {
			printf("[%s:%d] errno:%d = ioctl(%d(%s), FS_IOC_GETFLAGS, &attr)\n", __PRETTY_FUNCTION__, __LINE__, SYSF_errnoGet(), fd, path);
			result = False;
		}
		if (FS_IMMUTABLE_FL & attrOrg) {
			int attrBuf = attrOrg & ~FS_IMMUTABLE_FL;
			if (-1 == ioctl(fd, FS_IOC_SETFLAGS, &attrBuf)) {
				printf("[%s:%d] errno:%d = ioctl(%d(%s), FS_IOC_SETFLAGS, &attr)\n", __PRETTY_FUNCTION__, __LINE__, SYSF_errnoGet(), fd, path);
				result = False;
			}
		}
		if (-1 == fchown(fd, uid, gid)) {
			printf("[%s:%d] errno:%d = fchown(%d(%s), %d, %d)\n", __PRETTY_FUNCTION__, __LINE__, SYSF_errnoGet(), fd, path, uid, gid);
			result = False;
		}
		if (-1 == fchmod(fd, mode)) {
			printf("[%s:%d] errno:%d = fchmod(%d(%s), %#o)\n", __PRETTY_FUNCTION__, __LINE__, SYSF_errnoGet(), fd, path, mode);
			result = False;
		}
		if (FS_IMMUTABLE_FL & attrOrg) {
			if (-1 == ioctl(fd, FS_IOC_SETFLAGS, &attrOrg)) {
				printf("[%s:%d] errno:%d = ioctl(%d(%s), FS_IOC_SETFLAGS, &attr)\n", __PRETTY_FUNCTION__, __LINE__, SYSF_errnoGet(), fd, path);
				result = False;
			}
		}
		close(fd);
	} else {
		printf("[%s:%d] errno:%d = open(%s, (O_RDONLY|O_NONBLOCK))\n", __PRETTY_FUNCTION__, __LINE__, SYSF_errnoGet(), path);
		result = False;
	}
	return result;
}

// ================================================================================================
//! <!--機能--> 再帰属性変更.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param const char*	変更対象path
//! \param uchar		再帰有無
//! \param uid_t		所有者
//! \param gid_t		グループ
//! \param const char*	権限（chmodコマンド同様のmode指定可）
//! \param int			階層深度
//! <!--戻値-->
//! \return int			処理結果.
//! <!--詳細-->
//! 再帰なし、再帰あり（targetpathを含む）、再帰あり（targetpathを除く）のパターンで
//! 所有者、グループ、権限を一括変更する
//! immutable設定ファイルは解除して変更後再設定する
//! <!--履歴-->
//  22/06/XX ADS Nakai.
// ================================================================================================
static int recursive_chattrs(const char* path, uchar recurse, uid_t uid, gid_t gid, const char* mode, int depth) {
	struct stat statorg;
	int result = False;

	if (-1 == lstat(path, &statorg)) {
		printf("[%s:%d] errno:%d = lstat(%s, stat)\n", __PRETTY_FUNCTION__, __LINE__, SYSF_errnoGet(), path);
		return False;
	}
	
	// ディレクトリ以外.
	if (!S_ISDIR(statorg.st_mode)) {
		return change_attribute(path, &statorg, uid, gid, mode);
	}

	// 再帰なしの場合.
	if (THRD_MfpAccessNoRecurse == recurse) {
		return change_attribute(path, &statorg, uid, gid, mode);
	}
	
	DIR* dir = opendir(path);
	if (dir) {
		struct dirent* next = 0;
		char* nextpath = 0;
		bool lc = (path[strlen(path)-1] == '/');
		result = TRUE;
		
		while ((next = readdir(dir)) != NULL) {
			const char* dirname = next->d_name;
			// current、parentの場合はskip.
			if ((dirname)[0] == '.' && (!(dirname)[1] || ((dirname)[1] == '.' && !(dirname)[2])))
				continue;
			while (*dirname == '/')
				dirname++;
			
			if (-1 != asprintf(&nextpath, "%s%s%s", path, (lc?"":"/"), dirname)) {
				if (!recursive_chattrs(nextpath, recurse, uid, gid, mode, depth+1))
					result = False;
				free(nextpath);
				nextpath = 0;
			} else {
				printf("[%s:%d] errno:%d = asprintf(&nextpath, %s%s%s)\n", __PRETTY_FUNCTION__, __LINE__, SYSF_errnoGet(), path, (lc?"":"/"), dirname);
				result = False;
			}
		}
		closedir(dir);

		// 再帰ディレクトリまたは再帰あり（targetpathを含む）の場合.
		if ((0 < depth ) || (THRD_MfpAccessRecurseIn == recurse)) {
			if (!change_attribute(path, &statorg, uid, gid, mode))
				result = False;
		}
	} else {
		printf("[%s:%d] errno:%d = opendir(%s)\n", __PRETTY_FUNCTION__, __LINE__, SYSF_errnoGet(), path);
	}
	return result;
}

// ================================================================================================
//! <!--機能--> 変換対象情報を元に、ディレクトリ／ファイルの所有者、グループ、アクセス権限を変更.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! std::vector<THRS_CnvTargetInfo>& targetList	引数1	in 一般ユーザーアクセス変換対象リスト.
//! <!--戻値-->
//! \return Bool.
//! <!--詳細-->
//! 変換対象情報を元に、ディレクトリ／ファイルの所有者、グループ、アクセス権限を変更する.
//! <!--履歴-->
// 2021/11/01 ADS Moriwaki.
// ================================================================================================
Bool convertMfpAccessTarget( std::vector<THRS_CnvTargetInfo>& targetList )
{
	struct passwd pw_info = {0};
	struct passwd* pw_ptr = NULL;
	char pw_buf[THRD_GETPW_SIZE] = {0};
	uid_t pw_uid = 0;
	gid_t pw_gid = 0;
	uid_t public_uid = 0;
	gid_t public_gid = 0;
	uid_t root_uid = 0;
	gid_t root_gid = 0;
	struct stat st_Stat;
	THRS_CnvTargetInfo targetInfo;
	std::string permstr = "";
	char sc_Cmd[ THRD_MFPACCESS_CMD ] = {};
	const char* targetpath = NULL;
	int32_t si_Ret = 0;
	uchar recurse = 0;

	if (getpwnam_r(secure_getenv( "MFP_USERNAME" ), &pw_info, pw_buf, sizeof(pw_buf), &pw_ptr) != 0 || !pw_ptr ) {
		printf( "[%s:%d] getpwnam_r NG. user:MFP_USERNAME, errno:%d\n", __PRETTY_FUNCTION__, __LINE__, SYSF_errnoGet() );
		return False;
	}
	public_uid = pw_info.pw_uid;
	public_gid = pw_info.pw_gid;

	if (getpwnam_r("root", &pw_info, pw_buf, sizeof(pw_buf), &pw_ptr) != 0 || !pw_ptr ) {
		printf( "[%s:%d] getpwnam_r NG. user:root, errno:%d\n", __PRETTY_FUNCTION__, __LINE__, SYSF_errnoGet() );
		return False;
	}
	root_uid = pw_info.pw_uid;
	root_gid = pw_info.pw_gid;

	for( std::vector<THRS_CnvTargetInfo>::iterator itrTargetList = targetList.begin(); itrTargetList != targetList.end(); ++itrTargetList ) {
		targetInfo = *itrTargetList;
		targetpath = targetInfo.path.c_str();

		memset( &st_Stat, 0, sizeof(st_Stat) );
		if( -1 == SYSF_stat( (char*)targetpath, &st_Stat ) ) {
			// 存在しない.
			continue;
		}
		switch( targetInfo.kind ) {		// 変換処理区分.
			case 1:
				pw_uid = st_Stat.st_uid;
				pw_gid = st_Stat.st_gid;
				permstr = "o=u,g=";
				break;
			case 2:
				pw_uid = root_uid;
				pw_gid = root_gid;
				permstr = "o=u,g=";
				break;
			case 3:
				pw_uid = public_uid;
				pw_gid = public_gid;
				permstr = "u+X,go-w";
				break;
			case 4:
				pw_uid = public_uid;
				pw_gid = public_gid;
				permstr = "u+rwX,go-w";
				break;
			case 5:
				break;
			default:
				printf( "[%s:%d] kind NG. (%d)\n", __PRETTY_FUNCTION__, __LINE__, targetInfo.kind );
				return False;
		}
		switch( targetInfo.target ) {	// 変換対象区分.
			case 1: recurse = THRD_MfpAccessNoRecurse; break;	// 再帰なし
			case 2: recurse = THRD_MfpAccessRecurseIn; break;	// 再帰あり（targetpathを含む）
			case 3: recurse = THRD_MfpAccessRecurseEx; break;	// 再帰あり（targetpathを除く）
			default:
				printf( "[%s:%d] target NG. (%d)\n", __PRETTY_FUNCTION__, __LINE__, targetInfo.target );
				return False;
		}
		
		if ( targetInfo.kind == 5 ) {
			// 変換処理区分が、「5」の場合、postgresqlの設定ファイルに、ソケットファイルアクセス権限定義を追加する.
			snprintf( sc_Cmd, sizeof(sc_Cmd), THRD_MfpAccessCmd_Replace, targetpath);
			si_Ret = THRF_System( sc_Cmd );
			if ( si_Ret != 0 ) {
				printf( "[%s:%d] THRF_System(%s) NG. Syscall_Ret:%d, errno:%d\n", __PRETTY_FUNCTION__, __LINE__, sc_Cmd, si_Ret, SYSF_errnoGet() );
				return False;
			}
		} else {
			// 所有者・グループ・アクセス権限を変更.
			if (!recursive_chattrs(targetpath, recurse, pw_uid, pw_gid, permstr.c_str(), 0)) {
				printf( "[%s:%d] recursive_chattrs(%s, %u, %d, %d, %s) NG. errno:%d\n", __PRETTY_FUNCTION__, __LINE__, targetpath, recurse, pw_uid, pw_gid, permstr.c_str(), SYSF_errnoGet() );
				return False;
			}
		}
	}

	return True;
}


// ================================================================================================
//! <!--機能--> 該当マウントポイントとディレクトリ配下の権限を変更.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! char* mntpnt	引数1	in マウントポイント.
//! std::map<std::string, Bool> tmpMapFile	引数2	in マップファイル.
//! <!--戻値-->
//! \return int32_t.
//! <!--詳細-->
//! 該当マウントポイントとディレクトリ配下の権限を変更する.
//! <!--履歴-->
// 2021/06/16 ADS Kawai.
// ================================================================================================
int32_t CP_HDDTrans::ChangeAuthority( char* mntpnt, std::map<std::string, Bool>& tmpMapFile )
{
	std::vector<THRS_CnvTargetInfo> tmptargetList;
	std::string tmpMntPnt( mntpnt );

	//「一般ユーザーアクセス変換対象リスト」から、該当マウントポイントの変換対象情報を取得する.
	getMfpAccessTargetList( tmpMntPnt, tmptargetList );

	// 取得した変換対象情報のディレクトリ／ファイルの所有者、グループ、アクセス権限を変更する.
	Bool ret = convertMfpAccessTarget( tmptargetList );
	if ( ret == False ) {
		printf( "[%s:%d] convertMfpAccessTarget NG.\n", __PRETTY_FUNCTION__, __LINE__ );
		return -1;
	}

	// マウントポイントと一致するデータの変換済フラグを変換済に変更する.
	std::map<std::string, Bool>::iterator itrCtrlFile = tmpMapFile.find( mntpnt );
	if( itrCtrlFile != tmpMapFile.end() ){
		itrCtrlFile->second = True;
	}

	return 0;
}
