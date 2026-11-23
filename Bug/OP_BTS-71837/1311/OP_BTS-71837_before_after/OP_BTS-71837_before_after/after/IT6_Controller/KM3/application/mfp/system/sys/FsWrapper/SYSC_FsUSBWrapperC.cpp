// ----------------------------------------------------------------------------
// C level wrapper functions for the file system
// ----------------------------------------------------------------------------


#ifndef LINUX
#ifdef TORNADO
#include <vxWorks.h>
#include "system/sys/APIC_TaskPriority.h"
#include <lstLib.h>
#include <string.h>
#include <time.h>
#include <errnoLib.h>
#include <tickLib.h>
#include <stdlib.h>
#endif // TORNADO
#endif	// #ifndef LINUX 

#include "../../fil/FIL_FsWrapperC.h"
#include "../../fil/FIL_FwOsWrapper.h"
#include "../../fil/FILC_FilIf.h"
#include <math.h>
#include "SYSC_FsUSBWrapperC.h"
#include "printApp/printAppg/PAPC_USBPrintApp.h"
#include "system/saa/APIC_UsbMemDeviceInfo.h"
#include "system/sys/APIC_SystemStatus.h"
#include "system/sys/FsWrapper/SYSC_FSWrapper.h"

#ifdef TORNADO
#ifndef LINUX
#include "iosLib.h"
#include "dosFsLib.h"
#include "cbioLib.h"
#include "dcacheCbio.h"
#include "dpartCbio.h"
#include "usb/usbdLib.h"
#include "usb/usbQueueLib.h"
#include "drv/usb/usbBulkDevLib.h"
#include "private/dosFsLibP.h"

/* CCHOW - Including a Private header file */
#include "private/cbioLibP.h"
#endif	// #ifndef LINUX 

#endif // TORNADO
#ifdef LINUX
#include <sys/mount.h>
#include <dirent.h>	// [OP_BTS-71837] OP71837_DumpMountHolders
#include <unistd.h>	// [OP_BTS-71837] readlink
#include <stdio.h>	// [OP_BTS-71837] snprintf
#endif	// #ifdef LINUX.

#include "system/nvd/APIC_Setting_Other.h"
#include "../../shlib/LogLib/LIBC_LogLib.h"
#include "../../shlib/LogLib/LIBC_LogLibDefine.h"

#include "system/include/typs.h"
#include "system/thr/thrInclude/THRC_Threadif.h"

#if defined(TORNADO )	// not compile when building SimWin.exe
#include "../lib/filelib/SFLG_StorageFileLibraryIf.h"
#endif

// アーキテクチャリファイン 性能検証自動化（待ち時間測定）対応 start.
#include "system/dbg/PFMG_PerformAutoLogging.h"
// アーキテクチャリファイン 性能検証自動化（待ち時間測定）対応 end.

#include "system/sys/APIC_VirusScan.h"	// 2019.10.7 OP_BTS-11575.

extern "C" {
extern void DBGF_PrintNoOperationPath(char* psc_FileName, int32_t sl_LineNo); 
}

#ifdef TORV22
#undef NULL
#define NULL	(0)
#endif

#ifndef TORNADO			// ＰＣシミュレータで有効
#define NVDF_TRACE TRACE
#define MAX_LUN		2
#else					// VXWORKSで有効 
#define NVDF_TRACE( s, a1, a2, a3, a4, a5, a6 ) logMsg( s, a1, a2, a3, a4, a5, a6 ) 
#endif	// #ifdef TORNADO 

/*「target/config/comps/src/usrUsbBulkDevInit.c」から持ってきたもの */
#ifndef BULK_MAX_DEVS
static const int32_t BULK_MAX_DEVS =            2;                      /* Maximum number of bulk devices */
#endif

#ifndef BULK_MAX_DRIVE
#define BULK_MAX_DRIVE           MAX_LUN
#endif

#ifndef BULK_DRIVE_NAME
static const char* const BULK_DRIVE_NAME =          "/bd";                  /* Device name base */
#endif

#ifndef BULK_MAX_DRV_NAME_SZ
static const int32_t BULK_MAX_DRV_NAME_SZ =      20;                    /* Max size of device name */
#endif

#ifdef LINUX
static const char* const BLKD_DRIVE_File_NAME =     "/dev/";					/* Device file name base */
static const char* const BLKD_MOUNTS_File_NAME =     "/proc/mounts";			/* Mount name base */
#endif	// #ifdef LINUX.

#ifdef LINUX
/*
 オープンされたファイル名を記憶するための構造体
   Donau_Linuxでは同時に複数オープンされるケースは存在しないが、
   今後を踏まえて複数オープン対応にしておく.
*/
static const int32_t BLKD_MAX_FILEOPEN =		(2);						/* 同時オープン可能なファイル数 */
#endif	// LINUX.

static const int32_t VIRUSSCAN_FINALIZE_WAIT = 30;	// 2019.10.7 OP_BTS-11575.

// ----------------------------------------------------------------------------
// Local
// ----------------------------------------------------------------------------
static const int32_t FILD_DEVICENUM = 30;

struct SYSC_usbDevInfo
{
	char c_DevName[64];
	SEM_ID	 semID;
	LIST fileOpenContextList;
};

SYSC_usbDevInfo s_usbInfo[FILD_DEVICENUM];

static BOOL suspendWriting = FALSE;

static LIST reservedAreaList;

ushort us_debug_USBFlag = 0 ;
extern "C" {
	void debugDevName( ushort debugFlag, char* devName);
}

#ifdef LINUX		// Linux統合検証FUM FVT不具合No.51  第2電情技 岡本.
// オープンされたファイル名を記憶するための構造体.
typedef struct {
	char	c_FileName[SYSC_usbPrintFileNameMax + 1];		// フルパス名.
	int		fd;												// ファイルディスクリプタ.
}SYSC_usbFileOpenInfo;
static SYSC_usbFileOpenInfo	s_fileOpenInfo[BLKD_MAX_FILEOPEN];
#endif	// LINUX.

// 2020/8/31 OP_BTS-26686対応 start.
static char scBULK_DEVICE_NAME_DEV1[FILD_DEVICENUM+1] =				"/dev/sdc";		/* Device name device1 */
static char scBULK_DEVICE_NAME_DEV1_PART1[FILD_DEVICENUM+1] =		"/dev/sdc1";	/* Device name device1 partition1 */
static char scBULK_MOUNT_NAME_DEV1 [FILD_DEVICENUM+1]=				"/bd/sdc";		/* Mount name device1 */
static char scBULK_MOUNT_NAME_DEV1_PART1[FILD_DEVICENUM+1] =		"/bd/sdc1";		/* Mount name device1 partition1 */

static char scBULK_DEVICE_NAME_DEV1_NAME[FILD_DEVICENUM+1] =		"sdc";		/* Device name device1 */
static char scBULK_DEVICE_NAME_DEV1_PART1_NAME[FILD_DEVICENUM+1] =	"sdc1";		/* Device name device1 partition1 */
// 2020/8/31 OP_BTS-26686対応 end.

#define FMT_PART_FDISK1		"/sbin/fdisk -ul %s | grep \"Disk %s\" | sed 's/, /,/g' | cut -d\",\" -f2 | cut -d\" \" -f1"	// 2020/8/31 OP_BTS-26686対応.
// [AR1016604]セクタ数の取得コマンドをOSに依存しないコマンドに変更.
//#define FMT_PART_FDISK2		"/sbin/fdisk -ul %s | grep \"Units\" | sed 's/= /=/g' | cut -d\"=\" -f3 | cut -d\" \" -f1"
#define FMT_PART_FDISK2		"/sbin/fdisk -ul %s | grep \"Units\" | sed 's/.* = \\([0-9]*\\) bytes$/\\1/g'"
static const char* const FMT_PART_FDISK3 =		"/sbin/fdisk -u %s";
static const char* const FMT_PART_MKE2FS =		"/sbin/mke2fs -j -F %s";

Bool bl_isFormatUSB = False;		// ext3フォーマット実行中フラグ.

//TAS000000012453対応 2022/8/25 Start.
Bool isUSBWriteProtect( char* psc_devName );
//TAS000000012453対応 2022/8/25 End.
// ---------------------------------------------------------------------------------
// 関数名	：pickUp1stPathName
// ---------------------------------------------------------------------------------
// 概要 	：フルパスよりルートディレクトリを取得する
//				in ：pathName	フルパスのポインタ
//				out：devName	ルートディレクトリのポインタ
// 戻り値	：OK
//			  ERROR ->指定パスが不正
// 備考 	：
// 作成者	：N.Nakanishi
// レビュー ：レビュー実施日と名前を記述
// 変更履歴 ：変更日、変更者、変更内容（変更理由）を記述
// ---------------------------------------------------------------------------------
static STATUS pickUp1stPathName (const char *pathName,
								 char *devName)
{
	int i = 0;

	if ( pathName == NULL || devName == NULL )
	{
		return(ERROR);
	}

	if(pathName[0] != '/')
	{
		return(ERROR);
	}
	else
	{
		devName[0] = pathName[0];
	}

	for( i = 1 ; i < SYSC_usbPrintFileNameMax ; i++ )
	{
		devName[i] = pathName[i];
		if(devName[i] == '/')
		{
			devName[i] = 0;
			break;
		}
		else if(pathName[i] == 0)
		{
			break;
		}
		else {
			// no operation.
			DBGF_PrintNoOperationPath(__FILE__, __LINE__);
		}
	}

	if ( i == SYSC_usbPrintFileNameMax )
	{
		devName[SYSC_usbPrintFileNameMax] = 0;
	}

	return(OK);
}

#ifdef LINUX
// ---------------------------------------------------------------------------------
// 関数名	：SYSC_InitBulkDeviceNames.
// 概要     ：デバイス情報の初期化.
/// ---------------------------------------------------------------------------------
void SYSC_InitBulkDeviceNames()
{
	// デバイス情報の初期化.
	for( int n=0; n<FILD_DEVICENUM; n++ ){
		memset(s_usbInfo[ n ].c_DevName,0,sizeof(s_usbInfo[ n ].c_DevName));
	}
	// オープンファイル構造体のfdに-1を設定する.
	// フルパス部分は特に初期化しない.
	for (int cnt = 0; cnt < BLKD_MAX_FILEOPEN; cnt++) {
		s_fileOpenInfo[cnt].fd = -1;
	}
}

// [OP_BTS-71837] Diagnostic: print mountinfo lines of a mount point and
// every process whose cwd / exe / open fd lives under it. Called only on a
// mount failure or EBUSY on umount (rare path), so cost is irrelevant.
static void OP71837_DumpMountHolders( const char *pc_mnt )
{
	char sc_path[640];
	char sc_target[256];
	char sc_line[512];
	char sc_comm[32];
	const char *apc_kind[2] = { "cwd", "exe" };
	size_t ul_len = 0;
	FILE *fp_info = NULL;
	FILE *fp_comm = NULL;
	DIR *pd_proc = NULL;
	DIR *pd_fd = NULL;
	struct dirent *pe_proc = NULL;
	struct dirent *pe_fd = NULL;
	ssize_t sl_n = 0;
	int i_k = 0;

	if( NULL == pc_mnt ) {
		return;
	}
	ul_len = strlen( pc_mnt );
	printf( "(%010u) OP_BTS-71837: holders of %s\n", SYSF_tickGet(), pc_mnt );

	fp_info = fopen( "/proc/self/mountinfo", "r" );
	if( NULL != fp_info ) {
		while( NULL != fgets( sc_line, sizeof(sc_line), fp_info ) ) {
			if( NULL != strstr( sc_line, pc_mnt ) ) {
				printf( "OP_BTS-71837: mountinfo %s", sc_line );
			}
		}
		fclose( fp_info );
	}

	pd_proc = opendir( "/proc" );
	if( NULL == pd_proc ) {
		return;
	}
	while( NULL != ( pe_proc = readdir( pd_proc ) ) ) {
		if( pe_proc->d_name[0] < '0' || pe_proc->d_name[0] > '9' ) {
			continue;
		}
		memset( sc_comm, 0, sizeof(sc_comm) );
		snprintf( sc_path, sizeof(sc_path), "/proc/%s/comm", pe_proc->d_name );
		fp_comm = fopen( sc_path, "r" );
		if( NULL != fp_comm ) {
			if( NULL == fgets( sc_comm, sizeof(sc_comm), fp_comm ) ) {
				sc_comm[0] = '\0';
			}
			sc_comm[strcspn( sc_comm, "\n" )] = '\0';	// strip trailing newline
			fclose( fp_comm );
		}
		for( i_k = 0; i_k < 2; i_k++ ) {
			snprintf( sc_path, sizeof(sc_path), "/proc/%s/%s", pe_proc->d_name, apc_kind[i_k] );
			sl_n = readlink( sc_path, sc_target, sizeof(sc_target) - 1 );
			if( sl_n > 0 ) {
				sc_target[sl_n] = '\0';
				if( 0 == strncmp( sc_target, pc_mnt, ul_len ) ) {
					printf( "OP_BTS-71837:   pid=%s comm=%s %s -> %s\n", pe_proc->d_name, sc_comm, apc_kind[i_k], sc_target );
				}
			}
		}
		snprintf( sc_path, sizeof(sc_path), "/proc/%s/fd", pe_proc->d_name );
		pd_fd = opendir( sc_path );
		if( NULL == pd_fd ) {
			continue;
		}
		while( NULL != ( pe_fd = readdir( pd_fd ) ) ) {
			snprintf( sc_path, sizeof(sc_path), "/proc/%s/fd/%s", pe_proc->d_name, pe_fd->d_name );
			sl_n = readlink( sc_path, sc_target, sizeof(sc_target) - 1 );
			if( sl_n > 0 ) {
				sc_target[sl_n] = '\0';
				if( 0 == strncmp( sc_target, pc_mnt, ul_len ) ) {
					printf( "OP_BTS-71837:   pid=%s comm=%s fd %s -> %s\n", pe_proc->d_name, sc_comm, pe_fd->d_name, sc_target );
				}
			}
		}
		closedir( pd_fd );
	}
	closedir( pd_proc );
}

// ---------------------------------------------------------------------------------
// 関数名	：SYSC_SetBulkDeviceNames.
// 概要     ：デバイス情報の設定.
// 引数		：char*				psc_deviceName		デバイス名. 
//			：Bool				b_param				挿抜判定 True:add False:remove. 
//			：unsigned char*	uc_NotSupportCnt	未サポートデバイス数. 
//			：Bool				b_NonNumDevice		デバイス名の末尾に数字が付いているか(例：sdc1) True:付いている False:付いていない. 
// 変更履歴 ：2011/07/26 ft_kondo CQ.351735 USBバルクデバイスを抜いてもNGアイコン表示されたままになる問題. 
//											デバイス名の末尾に数字の無いバルクデバイスの場合は、NGアイコンの表示/非表示、未サポートデバイス数の増減処理を行わない.
//			  2012/04/26 河村 - UV-01224 ext3のマウントに対応.
// ---------------------------------------------------------------------------------
void SYSC_SetBulkDeviceNames( char* psc_deviceName, Bool b_param, unsigned char* uc_NotSupportCnt, Bool b_NonNumDevice )
{
	FILE *fp = NULL;
	char mntName[BULK_MAX_DRV_NAME_SZ], deviceFileName[BULK_MAX_DRV_NAME_SZ], mountPathName[BULK_MAX_DRV_NAME_SZ*2];
	char *psc_devName = NULL;
	char *psc_tmp = NULL;
	char *psc_saveptr = NULL;
	int i_res = 0;

	int i_umountRet = 0;
	int errsv = 0;

	// 配列初期化.
	memset(mntName,0,sizeof(mntName));
	memset(deviceFileName,0,sizeof(deviceFileName));

	// デバイスファイル名、mountフォルダ名生成.
	sprintf( deviceFileName,"%s%s", BLKD_DRIVE_File_NAME, psc_deviceName);
	sprintf( mntName,"%s/%s", BULK_DRIVE_NAME, psc_deviceName);

	printf("(%010u) (sys:PDLIF) SYSC_SetBulkDeviceNames Call1\n",SYSF_tickGet());	// 2019.3.12 AR1071423 ログ出力 日立上釜.
	if ( True == b_param ) {	// バルクデバイス接続の場合.
		printf( "mount %s to %s ...", deviceFileName, mntName );
		i_res = mount(deviceFileName,mntName,"vfat",0,"umask=070,utf8");
		if( i_res != 0 ) {	// [OP_BTS-71837] "error -1" hides errno: print it (read errno first)
			const int i_mntErrno71837 = errno;
			printf( "\n(%010u) OP_BTS-71837: mount(vfat) %s -> %s failed errno=%d(%s)\n", SYSF_tickGet(), deviceFileName, mntName, i_mntErrno71837, strerror( i_mntErrno71837 ) );
			OP71837_DumpMountHolders( mntName );
			errno = i_mntErrno71837;	// our own I/O must not change what the original code sees
		}
		if( i_res != 0 ) {	// vfatでマウント出来なかった場合はext3でのマウントを試みる.
//TAS000000012453対応 2022/8/25.Start.
			//USBライトプロテクトチェック.
			if( isUSBWriteProtect( deviceFileName ) ) {
				//ライトプロテクトだった場合の処理.
				i_res = mount(deviceFileName,mntName,"vfat",MS_RDONLY,"umask=070,utf8");
				if( i_res != 0 ) {	// vfatでマウント出来なかった場合はext3でのマウントを試みる.
					i_res = mount( deviceFileName, mntName, "ext3", MS_NOATIME|MS_RDONLY, EXT3_MOUNT_OPTIONS );
					if( i_res == 0 ) {
						printf( "type ext3.\n" );
					} else {
						printf( "error %d.\n", i_res );
					}
				} else {
					printf( "type vfat.\n" );
				}
//TAS000000012453対応 2022/8/25.End.
			} else {
				//プロテクト無し.
				i_res = mount( deviceFileName, mntName, "ext3", MS_NOATIME, EXT3_MOUNT_OPTIONS );
				if( i_res == 0 ) {
					printf( "type ext3.\n" );
				} else {
					const int i_extErrno71837 = errno;
					printf( "error %d. (OP_BTS-71837 ext3 errno=%d(%s))\n", i_res, i_extErrno71837, strerror( i_extErrno71837 ) );
					errno = i_extErrno71837;
				}
			}
		} else {
			printf( "type vfat.\n" );
		}
		if ( 0 == i_res ) {	// mount成功.
			if( ((*uc_NotSupportCnt) <= 0) && ( True == b_NonNumDevice )){ // 未サポートデバイス数が0以下で且つ、デバイス名の末尾に数字が付いていたら(例:sdc1等)NGアイコンを非表示にする. 
				// サポート外メディア以外（メディア無しの場合も含む）はCommonAPIでサポート可を設定.
				APIC_SystemStatus::newInstance()->setNotSupportUSBConnectSaa( False );
			}
			// アーキテクチャリファイン 性能検証自動化（待ち時間測定）対応 start.
			if ( True == PFMG_IsWaitTimePerformLoggingEnable() ){
				PFMG_WriteWaitTimePerformLogToUSB(mntName);
			}
			// アーキテクチャリファイン 性能検証自動化（待ち時間測定）対応 end.
			// アーキテクチャリファイン 性能検証自動化（パネル応答性測定）対応 start.
#if (defined(LINUX) && !defined(FUM_LINUX))
			else{
				struct stat st_FileStat, st_FileStatPanel;
				if( (OK != SYSF_Stat(PFMD_LogPerformSettingFile, &st_FileStat)) && (OK == SYSF_Stat(PFMD_LogPerformPanelSettingFile, &st_FileStatPanel)) ){
					PFMG_WritePanelPerformLogToUSB(mntName);
				}
			}
#endif	// #if (defined(LINUX) && !defined(FUM_LINUX))
			// アーキテクチャリファイン 性能検証自動化（パネル応答性測定）対応 end.
		} else {
			if( True == b_NonNumDevice ){	// デバイス名の末尾に数字が付いていたら(例:sdc1等)未サポートデバイス数をインクリメントする. 
				// サポート外メディアはCommonAPIでサポート不可を設定.
				APIC_SystemStatus::newInstance()->setNotSupportUSBConnectSaa( True );
				(*uc_NotSupportCnt)++;						// 未サポートデバイス数をインクリメント. 
			}
		}
	} else {	// バルクデバイス引き抜きの場合.
		i_umountRet = umount2(mntName, MNT_FORCE );
		errsv = errno;		// Coverity Check OK.
		printf( "(%010u) OP_BTS-71837: umount2(%s, MNT_FORCE) ret=%d errno=%d(%s)\n", SYSF_tickGet(), mntName, i_umountRet, errsv, strerror( errsv ) );
		errno = errsv;	// keep the following perror("umount2") correct
		if( i_umountRet == -1 ){
			perror("umount2");
		}		
		switch( errsv )
		{
			case EBUSY:				// 使用中 (busy) のため、 target をアンマウントできなかった.
				// Busyでなくなるまで繰り返す.
				OP71837_DumpMountHolders( mntName );	// [OP_BTS-71837] EBUSY: list holders before MNT_DETACH
				while( errsv == EBUSY )
				{
					SYSF_taskDelay(static_cast<int>(SYSF_sysClkRateGet() * 200 / 1000));   // 遅延(0.2s).
					i_umountRet = umount2(mntName, MNT_DETACH);	// 2019.4.8 AR1071423 オプション修正 .
					errsv = errno;
				}
				printf("(%010u) (sys:PDLIF) umount2 retry OK errsv = %d\n",SYSF_tickGet(),errsv);		// 2019.4.3 AR1071423 ログ出力 日立上釜.
				break;
			case EINVAL:			// target がマウントポイントではない.
				if( True == b_NonNumDevice ){		// デバイス名の末尾に数字が付いていたら(例:sdc1等)未サポートデバイス数をデクリメントする. 
					(*uc_NotSupportCnt)--;			// 未サポートデバイス数をデクリメント. 
				}
				break;
			case EAGAIN:			// umountリトライ要.
			case EFAULT:			// 失敗.
			case ENAMETOOLONG:		// パス名の長さが MAXPATHLEN より長かった.
			case ENOENT:			// パス名が空である。もしくは指定されたパスが存在しない.
			case ENOMEM:			// カーネルがファイル名やデータをコピーするための空きページを確保できなかった.
			case EPERM:				// 呼び出し元が必要な権限を持っていない.
			default :
				break;
		}
		if( ((*uc_NotSupportCnt) <= 0) && ( True == b_NonNumDevice )){ // 未サポートデバイス数が0以下で且つ、デバイス名の末尾に数字が付いていたら(例:sdc1等)NGアイコンを非表示にする. 
			APIC_SystemStatus::newInstance()->setNotSupportUSBConnectSaa( False );
		}
		printf("(%010u) (sys:PDLIF) umount2 end errsv = %d\n",SYSF_tickGet(),errsv);	// 2019.4.3 AR1071423 ログ出力 日立上釜.
	}

	// USB情報の初期化.
	for( int si_count = 0; FILD_DEVICENUM > si_count; si_count++ ){	
		memset(s_usbInfo[ si_count ].c_DevName,0,sizeof(s_usbInfo[ si_count ].c_DevName));
	}

	// Mount状態を確認するため、ファイルを確認.
	fp = FILF_Fopen(BLKD_MOUNTS_File_NAME,"r");
	if ( NULL == fp ) {	// 「/proc/mounts」のopenに失敗している場合は処理終了.
		NVDF_TRACE("BLKD_MOUNTS_File_NAME open error \n",0,0,0,0,0,0);
		return;
	}

	// USB情報の形成.
	{
		int si_count = 0;
		while ((FILF_Fgets(mountPathName, sizeof(mountPathName), fp)) != NULL) {
			// 「/bd」を含む場合は、バルクデバイスと判断する.
			if ( NULL != (psc_devName = strstr( mountPathName, BULK_DRIVE_NAME ))) {
				psc_tmp = strtok_r( psc_devName, " ", &psc_saveptr );		// スペースを区切り文字に検索.
				if ( psc_tmp != NULL ) {
					strcpy( s_usbInfo[ si_count ].c_DevName, psc_tmp);
					printf("(%010u) (sys:PDLIF) s_usbInfo[ si_count ].c_DevName = %s si_count = %d\n",SYSF_tickGet(),s_usbInfo[ si_count ].c_DevName,si_count);	// 2019.3.12 AR1071423 ログ出力 日立上釜.
				}
				si_count++;
			}
		}
	}

	// ファイルポインタクローズ.
	FILF_Fclose(fp);

//2021/08/29 HIENG Fix OP_BTS-34959 Start.
	if(b_param == False && i_umountRet == 0)
	{
		Bool b_UsbFound = False;
		for( int idx = 0; idx < FILD_DEVICENUM ; idx++ )
		{	
			if((s_usbInfo[idx].c_DevName[0] != 0) && (strncmp(s_usbInfo[idx].c_DevName, mntName, BULK_MAX_DRV_NAME_SZ) == 0))
			{
				// USB still exists in list.
				b_UsbFound = True;
				break;
			}
		}
		// USB unmount completely, notity to update
		if(b_UsbFound == False)
		{
			PAPC_USBPrintMsg msg = { PAPD_USBPrintWakeup, 1, NULL };
			SYSF_msgQSend( PAPC_USBPrintApp::getMsgQID(), (char*)&msg, sizeof(PAPC_USBPrintMsg), NO_WAIT, MSG_PRI_NORMAL );
		}
	}
//2021/08/29 HIENG Fix OP_BTS-34959 End.
}
#endif	// #ifdef LINUX.

#ifdef TORNADO

// Linuxの場合無効 ここから.
#ifndef LINUX
// ---------------------------------------------------------------------------------
// 関数名	：SYSC_GetBulkDeviceNames
// ---------------------------------------------------------------------------------
// 概要 	：Bulkデバイス数/名前の取得
// 引数		：char **names			取得したデバイス名を格納する領域。NULL指定可(戻り値のデバイス数のみ取得したい場合)
//			　short maxDevice		取得する最大のデバイス数
// 戻り値	：ushort, 取得したデバイス数(names=NULLの場合は接続デバイス数)
// 備考 	：
// 作成者	：制御12 河村
// レビュー ：レビュー実施日と名前を記述
// 変更履歴 ：06/10/06 初版 河村
//            07/03/08 濱田 75526 特定のUSBメモリでアクセスできない対応
//            07/04/11 樋口 78797 USBの差し方によって、外部メモリー複数検知が表示されない 
// ---------------------------------------------------------------------------------
extern "C" Bool usrUsbBulkDriveEmpty ( char *driveName );

ushort SYSC_GetBulkDeviceNames( char **names, short maxDevice )
{
	short cnt,i,enableDrive,connectDrive,s_NotSupport;
	char deviceName[BULK_MAX_DRV_NAME_SZ];
	DOS_VOLUME_DESC_ID pt_dosFsVolDescGet;
	int  fh;
	
	enableDrive = 0;
	connectDrive = 0;
	s_NotSupport = 0;
	for( cnt=0; cnt < BULK_MAX_DEVS; cnt++ ){
		for (i=0; i < BULK_MAX_DRIVE; i++){
			/* デバイス名を作成「target/config/comps/src/usrUsbBulkDevInit.c」参照 */
			memset( (void *)deviceName, 0, BULK_MAX_DRV_NAME_SZ );
			sprintf( deviceName,"%s%d", BULK_DRIVE_NAME, (i + (cnt*BULK_MAX_DRIVE)));
			if( usrUsbBulkDriveEmpty( deviceName ) != True ){
				/* メディアあり */
				if( names != NULL ){
					memcpy( (void*)(names[ enableDrive ]), (void*)deviceName, BULK_MAX_DRV_NAME_SZ );
				}

				// 保存先に既にある名前が検出した名前と違う場合は初期化 
				if((s_usbInfo[ enableDrive ].c_DevName[0] != 0) && (strcmp(s_usbInfo[ enableDrive ].c_DevName,deviceName))){
					memset(s_usbInfo[ enableDrive ].c_DevName,0,sizeof(s_usbInfo[ enableDrive ].c_DevName));
				}

				// DosFsでマウント出来たか確認する。
				if( s_usbInfo[ enableDrive ].c_DevName[0] == 0 ){
					// 最初のデバイス認識
					fh = SYSF_open( deviceName , 0 , 0);
					SYSF_close( fh );
					strncpy( s_usbInfo[ enableDrive ].c_DevName , deviceName , BULK_MAX_DRV_NAME_SZ );
					s_usbInfo[ enableDrive ].c_DevName[BULK_MAX_DRV_NAME_SZ] = 0;
				}

				pt_dosFsVolDescGet =  dosFsVolDescGet( deviceName , NULL );
				if( pt_dosFsVolDescGet->mounted != False ){
					// DosFsデバイス
					if( pt_dosFsVolDescGet->fatType != DOS_VOLUME_DESC::FAT32 ){
						// FAT12,FAT16の場合 1.1G以上は未サポートとする.
						if( (pt_dosFsVolDescGet->totalSec * static_cast<uint32_t>(pt_dosFsVolDescGet->bytesPerSec)) > (1124*1024*1024) ){
							// 1.1G以上は未サポート.
							s_NotSupport++;
						}
						else{
							enableDrive ++;
						}
					}
				}else{
					// DosFsでマウント出来ない場合はサポート外メディア.
					s_NotSupport++;
				}

				connectDrive++;
				if( connectDrive >= maxDevice && names!=NULL ){
					return connectDrive;
				}
			}
		}
	}

	if( 0 != s_NotSupport ){
		// サポート外メディアはCommonAPIでサポート不可を設定.
		APIC_SystemStatus::newInstance()->setNotSupportUSBConnectSaa( True );
	}else{
		// サポート外メディア以外（メディア無しの場合も含む）はCommonAPIでサポート可を設定.
		APIC_SystemStatus::newInstance()->setNotSupportUSBConnectSaa( False );
	}
	
	// 未登録分の名前を消す
	for( int n=enableDrive; n<FILD_DEVICENUM; n++ ){
		memset(s_usbInfo[ n ].c_DevName,0,sizeof(s_usbInfo[ n ].c_DevName));
	}
	return connectDrive;
}

#else	// #ifndef LINUX.
// Linuxの場合無効 ここまで.

// ---------------------------------------------------------------------------------
// 関数名	：SYSC_GetBulkDeviceNames.
// 概要     ：デバイス情報の取得.
// ---------------------------------------------------------------------------------
ushort SYSC_GetBulkDeviceNames( char **names, short maxDevice )
{
	ushort us_count = 0;
	// USB情報から、マウント状態を取得.
	for ( us_count = 0; us_count < FILD_DEVICENUM; ) {
		if ( s_usbInfo[ us_count ].c_DevName[0] == 0 ) {
			break;
		}
		if( names != NULL ){	// 名前の格納.
			memcpy( reinterpret_cast<void*>(names[ us_count ]), reinterpret_cast<void*>(s_usbInfo[ us_count ].c_DevName), BULK_MAX_DRV_NAME_SZ );
		}
		us_count++;  /* CQ.335866 */
		if( us_count >= maxDevice && names!=NULL ){
			// Maxを超えている場合は終了.
			break;
		}
	}
	return us_count;
}
// ---------------------------------------------------------------------------------
// 関数名	：SYSC_GetBulkDeviceNameIndex.
// 概要     ：デバイス情報の取得(BlackAPI用).
// ---------------------------------------------------------------------------------
ushort SYSC_GetBulkDeviceNameIndex( char *name, short idx )
{
	ushort us_count;
	// USB情報から、マウント状態を取得.
	for ( us_count = 0; us_count < FILD_DEVICENUM; ) {
		if ( s_usbInfo[ us_count ].c_DevName[0] == 0 ) {
			break;
		}
		if( name != NULL && us_count == (idx-1) ){	// 名前の格納.
			memcpy( reinterpret_cast<void*>(name), reinterpret_cast<void*>(s_usbInfo[ us_count ].c_DevName), BULK_MAX_DRV_NAME_SZ );
			us_count++;
			break;
		}
		us_count++;
	}
	return us_count;
}
#endif	// #ifndef LINUX.
#else	//	シミュレータ用処理

ushort	SYSC_SimUsbMemoryNum = 0;	//	USBメモリ装着数：0 or 1

ushort SYSC_GetBulkDeviceNames( char **names, short maxDevice )
{
	if( names != NULL ) {
		for( int n = 0; n < maxDevice; n++ ) {
			*names[n] = '\0';			//  シミュレータではデバイス名に""を設定する.
		}
	}

	return( SYSC_SimUsbMemoryNum );
}

#endif // TORNADO

// ---------------------------------------------------------------------------------
// 関数名	：devPath
// ---------------------------------------------------------------------------------
// 概要 	：デバイス名取得
// 引数		：なし
// 戻り値	：char *, デバイス名へのポインタ
// 備考 	：0もしくは2つ以上のBulkデバイスが検出された場合は"/"を返します(エラーとして扱う)
// 作成者	：制御12 河村
// レビュー ：レビュー実施日と名前を記述
// 変更履歴 ：2006/10/06 初版 河村
//            2007/04/11 伊藤史：問題点DB78797USB2本挿し解除後のフレキシブルパス対応
//            2007/05/25 樋口  ：問題点DB80401 USB保存中に抜くとメモリフル対応.
// ---------------------------------------------------------------------------------
static char* devPathNone = "/zzz";

static char *devPath()
{
#ifdef TORNADO
	static char deviceName[FILD_DEVICENUM][BULK_MAX_DRV_NAME_SZ];
	char* tmp[FILD_DEVICENUM];
	ushort num = 0;
	ushort us_LogUSBConnect = 0;
	char sc_HDDLogEncryptionPassword[TYPD_MAX_LogEncryptionPassword];
	char sc_DevicePath[SYSC_usbPrintFileNameMax + SYSC_usbPrintFileNameDiff + BULK_MAX_DRV_NAME_SZ + 1] = {0};

	for(ushort us_Cnt = 0; us_Cnt < FILD_DEVICENUM; us_Cnt++ ){
		tmp[us_Cnt] = deviceName[us_Cnt];
	}

	APIC_DebugLogSystemSetting::newInstance()->getLogUsbMemKeyFileEncryptionPassword( sc_HDDLogEncryptionPassword );
	num = SYSC_GetBulkDeviceNames( NULL, 0 );	//デバイス数取得

	SYSC_GetBulkDeviceNames( tmp, FILD_DEVICENUM );	// デバイス名取得.

	for(ushort us_Cnt = 0; us_Cnt < num; us_Cnt++ )
	{
		strcpy( sc_DevicePath, deviceName[us_Cnt] );
		strcat( sc_DevicePath, LIBD_KeyFilePath );
		if( LIBF_isLogSystemUSBMem( sc_DevicePath, sc_HDDLogEncryptionPassword ) == True )
		{
			us_LogUSBConnect++;
		}
	}
	if( num < 1 || ( num == 2 && us_LogUSBConnect == 0 ) || ( num == 2 && us_LogUSBConnect >= 2 ) || num > 2 ) { return devPathNone; }				//デバイス数が1でない場合はエラー

	SYSC_GetBulkDeviceNames( tmp, 1 );		//デバイス名取得
	return static_cast<char*>(deviceName[0]);
#else	// #ifdef TORNADO
	//simWinの場合はルートディレクトリを返す
	return "/";
#endif	// #ifdef TORNADO
}

// ---------------------------------------------------------------------------------
// 関数名	：waitWritingUsb
// ---------------------------------------------------------------------------------
// 概要 	：アクセス可能になるまで待つ。
// 戻り値	：なし
// 備考 	：
// 作成者	：N.Nakanishi
// レビュー ：レビュー実施日と名前を記述
// 変更履歴 ：変更日、変更者、変更内容（変更理由）を記述
// ---------------------------------------------------------------------------------
void waitWritingUsb ()
{
	while (FALSE!=suspendWriting)
	{
#ifdef TORNADO
		SYSF_taskDelay (1);
#endif // TORNADO
	}
}


// ---------------------------------------------------------------------------------
// 関数名	：getFullPath
// ---------------------------------------------------------------------------------
// 概要 	：USBのデバイス名を加えたフルパスを取得する。
//				in ：pathName	デバイス名を除いたファイルパスのポインタ
//				out：fullPathName	フルパスのポインタ
// 戻り値	：OK
//			  ERROR ->指定パスが不正
// 備考 	：
// 作成者	：Hamada
// レビュー ：レビュー実施日と名前を記述
// 変更履歴 ：変更日、変更者、変更内容（変更理由）を記述
//            2007/03/22 伊藤史  全体的に、getFullPath の戻り値チェックの誤りを修正
//               誤) if( getFullPath(pathName, name) == 0 ){
//               正) if( getFullPath(pathName, name) == ERROR ){
// ---------------------------------------------------------------------------------
static int getFullPath (const char *pathName, char *fullPathName)
{
#ifdef TORNADO
	if (strncmp(pathName, "/bd", 3) == 0){
		/* デバイス名があれば、そのまま返す */
		strcpy(fullPathName, pathName);
	}else{
		/* デバイス名がなければ、デバイス名を付与する */
		strcpy(fullPathName, devPath() );
		strcat(fullPathName, "/");
		strcat(fullPathName, pathName);
	}
#else  // Simulator
	/* シミュレータではファイルパスをルートディレクトリからの絶対パスで指定 */
	if( pathName[0] == '/' ) {
		strcpy(fullPathName, pathName);
	} else {
		strcpy(fullPathName, devPath() );
		strcat(fullPathName, pathName);
	}
#endif // TORNADO
	
	return(OK);
}

// 外部参照用：デバッグ用途
int USB_getFullPath (const char *pathName, char *fullPathName)
{
	return getFullPath ( pathName, fullPathName);
}


// ----------------------------------------------------------------------------
// 関数名	：searchReservedPathUsb
// ---------------------------------------------------------------------------------
// 概要 	：
//				in：pDevName	デバイス名のポインタ
// 戻り値	：NULL	not exist
//			  !=NULL : list element for the specified device
// 備考 	：
// 作成者	：Hamada
// レビュー ：レビュー実施日と名前を記述
// 変更履歴 ：変更日、変更者、変更内容（変更理由）を記述
// ----------------------------------------------------------------------------
static SYSC_usbReservedAreaList *searchReservedPathUsb(char *pDevName)
{
	SYSC_usbReservedAreaList	*rsvPath = NULL;

	rsvPath = reinterpret_cast<SYSC_usbReservedAreaList *> (FILF_LstFirst(&reservedAreaList));
	while(NULL!=rsvPath)
	{
		if(strncmp(rsvPath->DevName, pDevName, (SYSC_usbPrintFileNameMax) ) == 0)
		{
			break;
		}
		rsvPath = reinterpret_cast<SYSC_usbReservedAreaList *>(FILF_LstNext(reinterpret_cast<NODE *>(rsvPath)));
	}

	return rsvPath;
}

/*FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
	getReservedSizeUsb

	INPUT : char *name
		device name
	RETURN : FILS_ReservedAreaList *
		Reserved area

FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF*/
static SYSC_usbReservedAreaList *getReservedSizeUsb (char * name)
{
	STATUS status = OK;
	SYSC_usbReservedAreaList *rsvArea = NULL;
	char devName[SYSC_usbPrintFileNameMax + 1];

	/*pick up the device name*/
	memset(devName, 0x00, sizeof(devName));
	status = pickUp1stPathName( name, devName );
	if ( status != ERROR )
	{
		rsvArea = reinterpret_cast<SYSC_usbReservedAreaList *>(searchReservedPathUsb(devName));
	}

	return(rsvArea);
}

// ---------------------------------------------------------------------------------
// 関数名	：getReservedSizeUsb
// ---------------------------------------------------------------------------------
// 概要 	：指定デバイスの空きブロックを取得する。
//			  空きブロックがない場合はerrnoをセットする。
//				in ：pName			フルパスのポインタ
//				out：pFreeBlock 	空きブロック数のポインタ
// 戻り値	：OK
//			  ERROR
//			  *pFreeBlock -> 空きブロック
// 備考 	：
// 作成者	：Hamada
// レビュー ：レビュー実施日と名前を記述
// 変更履歴 ：変更日、変更者、変更内容（変更理由）を記述
//            07/03/08 濱田 77069・76807 USBメモリFULL対応
//            13/12/19 ADS滕 CQ.384044、CQ.384077、CQ.384095 Act_70038 USBエクスポート失敗対応.
//            14/10/22 OEC伊沢 CQ502850 本体設定エクスポート実行時、空き無しのUSB挿入、「USBの容量が不足しています」表示しない
// ---------------------------------------------------------------------------------
static STATUS getReservedSizeUsb(const char *pName, ssize_t *pFreeBlock)
{
	struct statfs			stat = {};
	ssize_t					thresh = 0;
	SYSC_usbReservedAreaList	*rsvArea = NULL;
	char					pDevName[SYSC_usbPrintFileNameMax + 1] = {};

	memset( pDevName, 0, sizeof(pDevName) );
	if (strncmp(pName, "/bd", 3) == 0){
		/* デバイス名があれば、そのまま返す */
		strncpy( pDevName, pName, SYSC_usbPrintFileNameMax );
	} else {
		strncpy( pDevName, devPath(), SYSC_usbPrintFileNameMax );
	}

	if(statfs(pDevName, &stat) == ERROR)
	{
		return(ERROR);
	}
	else
	{
		if((rsvArea = reinterpret_cast<SYSC_usbReservedAreaList *>(getReservedSizeUsb(pDevName))) != NULL)
		{
			if(static_cast<double>(rsvArea->ReservedSize*1024) < static_cast<double>(stat.f_bsize))
			{
				stat.f_bfree -= 1;
			}
			else
			{
				stat.f_bfree -= static_cast<size_t>((rsvArea->ReservedSize*1024)/stat.f_bsize);
				if(0!=(static_cast<ssize_t>((static_cast<double>(rsvArea->ReservedSize)) * 1024) % stat.f_bsize))
				{
					stat.f_bfree --;
				}
			}
		}
	}

	thresh = static_cast<ssize_t>((stat.f_blocks * FILD_ThresholdOfDiskFull) / 10000);	/* 0.0x % */

	*pFreeBlock = static_cast<ssize_t>(stat.f_bfree - thresh);

	if (*pFreeBlock <= 0)
	{
#ifdef TORNADO
		if (strncmp(pDevName, devPath(), 4) == 0) // 開こうとしたファイルがUSB('/bd/')かどうか判定
		{
			// USB FULLを設定、通知
			APIC_UsbMemDeviceInfo::newInstance()->setWarning(True, TYPE_UsbMemWarningMemoryFull);
		}
//		NVDF_TRACE("FIL : There is not enough space 1 [%s]\n",pName,0,0,0,0,0);
		SYSF_errnoSet(S_dosFsLib_DISK_FULL);
#endif // TORNADO
	}

	return(OK);
}

#ifdef TORNADO
// ---------------------------------------------------------------------------------
// 関数名	：searchFdUsb
// ---------------------------------------------------------------------------------
// 概要 	：指定ファイルディスクリプタがコンテキストにあるか検索する。
//				in：fd	ファイルディスクリプタ
// 戻り値	：NULL ->指定ファイルディスクリプタがない
//			  コンテキスト ->指定ファイルディスクリプタがコンテキストに存在する
// 備考 	：
// 作成者	：Hamada
// レビュー ：レビュー実施日と名前を記述
// 変更履歴 ：変更日、変更者、変更内容（変更理由）を記述
// ---------------------------------------------------------------------------------
static SYSC_usbFileOpenContext *searchFdUsb (int fd, int i_structCnt)
{
	SYSC_usbFileOpenContext *context = NULL;

	context = reinterpret_cast<SYSC_usbFileOpenContext*> (FILF_LstFirst (&s_usbInfo[i_structCnt].fileOpenContextList));
	while (NULL!=context)
	{
		if (context->Fd == fd)
		{
			break;
		}
		context = reinterpret_cast<SYSC_usbFileOpenContext*> (FILF_LstNext (reinterpret_cast<NODE*> (context)));
	}

	return context;
}

// ---------------------------------------------------------------------------------
// 関数名	：createFileOpenContextUsb
// ---------------------------------------------------------------------------------
// 概要 	：コンテキストに追加する
//				in：fd		ファイルディスクリプタ
//				in：name	ファイルのフルパスのポインタ
// 戻り値	：OK
//			  < 0 ->ERROR
// 備考 	：
// 作成者	：Hamada
// レビュー ：レビュー実施日と名前を記述
// 変更履歴 ：変更日、変更者、変更内容（変更理由）を記述
// ---------------------------------------------------------------------------------
static int createFileOpenContextUsb (int fd,			// ファイルディスクリプタ
								 const char *name,	// ファイルのフルパス
								 int i_structCnt)
{
	SYSC_usbFileOpenContext *context = NULL;
	char *PathName = NULL;

	// Contextに登録されてるかサーチする
	if ((context = searchFdUsb (fd, i_structCnt)) == NULL)
	{
		// 登録されて無い場合、Contextを作成する
		if ((context = reinterpret_cast<SYSC_usbFileOpenContext*>(FILF_Malloc (sizeof (SYSC_usbFileOpenContext), FILE_MallocModeContext))) == NULL)
		{
			return FILD_ErrNoMemory;
		}
		context->Fd = fd;
		// Contextに追加する
		FILF_LstAdd (&s_usbInfo[i_structCnt].fileOpenContextList, &context->Node);
	}
	// Contextに情報を付加する
	context->TaskId = SYSF_taskIdSelf();
	
	PathName = reinterpret_cast<char*>(FILF_Malloc(SYSC_usbPrintFileNameMax, FILE_MallocModeCache));
	
	if (PathName == NULL)
	{
		return FILD_ErrNoMemory;
	}
	
	strncpy (context->Name, PathName, (SYSC_usbPrintFileNameMax) );
	context->Name [SYSC_usbPrintFileNameMax] = 0;
	
	FILF_Free(PathName, FILE_MallocModeCache);
	return OK;
}

// ---------------------------------------------------------------------------------
// 関数名	：searchNameUsb
// ---------------------------------------------------------------------------------
// 概要 	：指定ファイル名がコンテキストに登録されているか検索する。
//				in：pName		ファイル名のポインタ
// 戻り値	：TRUE ->登録されている
//			  FALSE ->登録されていない
// 備考 	：
// 作成者	：Hamada
// レビュー ：レビュー実施日と名前を記述
// 変更履歴 ：変更日、変更者、変更内容（変更理由）を記述
// ---------------------------------------------------------------------------------
static int searchNameUsb (const char *pName, int i_structCnt)
{
	SYSC_usbFileOpenContext *context = NULL;

	/* This function must be controlled by the semaphore.		 */
	/* Otherwize, name cannot be allocated as a static variable. */

	if( NULL != pName )
	{
		context = reinterpret_cast<SYSC_usbFileOpenContext*> (FILF_LstFirst (&s_usbInfo[i_structCnt].fileOpenContextList));
		while (NULL!=context)
		{
			if (strncmp (context->Name, pName, SYSC_usbPrintFileNameMax) == 0)
			{
				return TRUE;
			}
			context = reinterpret_cast<SYSC_usbFileOpenContext*> (FILF_LstNext (reinterpret_cast<NODE*> (context)));
		}
	}

	return FALSE;
}

// ---------------------------------------------------------------------------------
// 関数名	：deleteFileOpenContextUsb
// ---------------------------------------------------------------------------------
// 概要 	：指定ファイルディスクリプタのコンテキストを削除する
//				in：fd		ファイルディスクリプタ
// 戻り値	：なし
// 備考 	：
// 作成者	：Hamada
// レビュー ：レビュー実施日と名前を記述
// 変更履歴 ：変更日、変更者、変更内容（変更理由）を記述
// ---------------------------------------------------------------------------------
static void deleteFileOpenContextUsb (int fd, int i_structCnt)
{
	SYSC_usbFileOpenContext *context = NULL;

	if (NULL!=(context = searchFdUsb (fd, i_structCnt)))
	{
		FILF_LstDelete (&s_usbInfo[i_structCnt].fileOpenContextList, reinterpret_cast<NODE*> (context));
		FILF_Free (context, FILE_MallocModeContext);
	}
}
#endif // TORNADO



#ifdef LINUX		// Linux統合検証FUM FVT不具合No.51  第2電情技 岡本.
/*
 ---------------------------------------------------------------------------------
 関数名		：setFileOpenInfo
 ---------------------------------------------------------------------------------
 概要 		：ファイルオープン情報構造体にパスとファイルディスクリプタをセットする。
					in ：fd				ファイルディスクリプタ
						 name			フルパス名
					out：なし
 戻り値		：void
 備考 		：
 作成者		：第2電情技 岡本
 レビュー	：レビュー実施日と名前を記述
 変更履歴	：変更日、変更者、変更内容（変更理由）を記述
 ---------------------------------------------------------------------------------
*/
static void setFileOpenInfo(int fd, const char* name)
{
	int check = 0;		// 構造体Full検出用.
	
	if (name != NULL) {
		// オープンファイル構造体の空きを探し、引数の情報を格納する.
		for (int cnt = 0; cnt < BLKD_MAX_FILEOPEN; cnt++) {
			if (s_fileOpenInfo[cnt].fd == -1) {
				s_fileOpenInfo[cnt].fd = fd;
				strncpy(s_fileOpenInfo[cnt].c_FileName, name, SYSC_usbPrintFileNameMax);
				s_fileOpenInfo[cnt].c_FileName[SYSC_usbPrintFileNameMax] = '\0';
				check++;
				break;
			}
		}
		if (0==check) {
			SYSF_logMsg("[setFileOpenInfo] s_fileOpenInfo FULL!!\n", 0, 0, 0, 0, 0, 0);
		}
	}
	else {
		SYSF_logMsg("[setFileOpenInfo] name is NULL!!\n", 0, 0, 0, 0, 0, 0);
	}
}

/*
 ---------------------------------------------------------------------------------
 関数名		：clearFileOpenInfo
 ---------------------------------------------------------------------------------
 概要 		：ファイルオープン情報構造体から指定したファイルディスクリプタの情報を
 			  削除する。
					in ：fd				ファイルディスクリプタ
					out：なし
 戻り値		：void
 備考 		：
 作成者		：第2電情技 岡本
 レビュー	：レビュー実施日と名前を記述
 変更履歴	：変更日、変更者、変更内容（変更理由）を記述
 ---------------------------------------------------------------------------------
*/
static void clearFileOpenInfo(int fd)
{
	int check = 0;		// 構造体エラー検出用.
	
	// オープンファイル構造体をチェックし、引数の情報をクリアする.
	for (int cnt = 0; cnt < BLKD_MAX_FILEOPEN; cnt++) {
		if (s_fileOpenInfo[cnt].fd == fd) {
			s_fileOpenInfo[cnt].fd = -1;		// 0はfdとして有効な値なので不可.
			check++;
			break;
		}
	}
	if (0==check) {
		SYSF_logMsg("[clearFileOpenInfo] s_fileOpenInfo FULL!!\n", 0, 0, 0, 0, 0, 0);
	}
}

/*
 ---------------------------------------------------------------------------------
 関数名		：getDevPath
 ---------------------------------------------------------------------------------
 概要 		：ファイルディスクリプタよりルートディレクトリを取得する。
					in ：fd 			ファイルディスクリプタ
					out：devName		ルートディレクトリのポインタ
 戻り値		：OK
			  ERROR - if the file descriptor does not exist.
 備考	 	：
 作成者		：第2電情技 岡本
 レビュー	：レビュー実施日と名前を記述
 変更履歴	：変更日、変更者、変更内容（変更理由）を記述
 ---------------------------------------------------------------------------------
*/
static STATUS getDevPath (int fd, char *devName)
{
	STATUS status = ERROR;

	if (devName != NULL) {
		// オープンファイル構造体をチェックし、フルパスを渡す.
		for (int cnt = 0; cnt < BLKD_MAX_FILEOPEN; cnt++) {
			if (s_fileOpenInfo[cnt].fd == fd) {
				strncpy(devName, s_fileOpenInfo[cnt].c_FileName, SYSC_usbPrintFileNameMax);
				devName[SYSC_usbPrintFileNameMax] = '\0';
				status = OK;
				break;
			}
		}
		if (status == ERROR) {
			SYSF_logMsg("[getDevPath] fd(%d) cannot find!!\n", fd, 0, 0, 0, 0, 0);
		}
	}
	else {
		SYSF_logMsg("[getDevPath] devName is NULL!!\n", 0, 0, 0, 0, 0, 0);
	}

	return status;
}
#else	// LINUX.
// ---------------------------------------------------------------------------------
// 関数名	：getDevPath
// ---------------------------------------------------------------------------------
// 概要 	：ファイルディスクリプタよりルートディレクトリを取得する。
//				in ：fd 			ファイルディスクリプタ
//				out：devName		ルートディレクトリのポインタ
// 戻り値	：OK
//			  ERROR - if the file descriptor does not exist.
// 備考 	：
// 作成者	：大貫
// レビュー ：レビュー実施日と名前を記述
// 変更履歴 ：変更日、変更者、変更内容（変更理由）を記述
// ---------------------------------------------------------------------------------
static STATUS getDevPath (int fd, char *devName)
{
	STATUS status = 0;

#ifdef TORNADO
	//	ioctl : The return value of the driver, or ERROR if the file descriptor does not exist. 
	char c_FilePath[SYSC_usbPrintFileNameMax + 1];
	memset(c_FilePath, 0x00, sizeof(c_FilePath));
	if ( ioctl (fd, FIOGETNAME, (int)c_FilePath) != ERROR )
	{
		status = pickUp1stPathName( c_FilePath, devName );
	}
	else
	{
		status = ERROR;
	}
#else
	strcpy(devName, devPath());
#endif

	return status;
}
#endif	// LINUX.


// ---------------------------------------------------------------------------------
// 関数名	：getFreeSpaceUsb
// ---------------------------------------------------------------------------------
// 概要 	：HDDのDiskFullチェックを行う。DiskFullに近い場合、空きなしを設定する。
//				in ：pDevName	デバイス名のポインタ
//				out：pStat		ステータスのポインタ
// 戻り値	：OK
//			  ERROR
// 備考 	：
// 作成者	：Hamada
// レビュー ：レビュー実施日と名前を記述
// 変更履歴 ：変更日、変更者、変更内容（変更理由）を記述
//            07/03/08 濱田 77069・76807 USBメモリFULL対応
// ---------------------------------------------------------------------------------
static STATUS getFreeSpaceUsb(const char *pDevName, struct statfs *pStat)
{
	ssize_t thresh = 0;
	SYSC_usbReservedAreaList *rsvArea = NULL;

	if(statfs(const_cast<char *>(pDevName), pStat) == ERROR)
	{
		return(ERROR);
	}
	else
	{
		if((rsvArea = reinterpret_cast<SYSC_usbReservedAreaList *>(getReservedSizeUsb(const_cast<char *>(pDevName)))) != NULL)
		{
			if(static_cast<double>(rsvArea->ReservedSize*1024) < static_cast<double>(pStat->f_bsize))
			{
				pStat->f_bfree -= 1;
			}
			else
			{
				pStat->f_bfree -= static_cast<size_t>((rsvArea->ReservedSize*1024)/pStat->f_bsize);
				if(0!=(static_cast<ssize_t>((static_cast<double>(rsvArea->ReservedSize)) * 1024) % pStat->f_bsize))
				{
					pStat->f_bfree --;
				}
			}
		}
	}

	thresh = static_cast<ssize_t>((pStat->f_blocks * FILD_ThresholdOfDiskFull) / 10000); /* 0.0x % */

	if(thresh > static_cast<ssize_t>(pStat->f_bfree))
	{
		pStat->f_bfree = 0;

#ifdef TORNADO
		if (strcmp(pDevName, devPath()) == 0)
		{
			// USB FULLを設定、通知
			APIC_UsbMemDeviceInfo::newInstance()->setWarning(True, TYPE_UsbMemWarningMemoryFull);
		}
#endif // TORNADO
	}
	else
	{
		pStat->f_bfree -= thresh;
	}

	return(OK);
}

// ---------------------------------------------------------------------------------
// 関数名	：getFreeSpaceByFdUsb
// ---------------------------------------------------------------------------------
// 概要 	：ファイルディスクリプタ使用によりHDDのDiskFullチェックを行う。
//			  DiskFullに近い場合、空きなしを設定する。
//				in ：fd 		ファイルディスクリプタ
//				in ：pDevName	デバイス名のポインタ
//				out：pStat		ステータスのポインタ
// 戻り値	：OK
//			  ERROR
// 備考 	：
// 作成者	：Hamada
// レビュー ：レビュー実施日と名前を記述
// 変更履歴 ：変更日、変更者、変更内容（変更理由）を記述
//            07/03/08 濱田 77069・76807 USBメモリFULL対応
// ---------------------------------------------------------------------------------
static STATUS getFreeSpaceByFdUsb(int fd, const char *pDevName, struct statfs *pStat)
{
	ssize_t thresh = 0;
	SYSC_usbReservedAreaList *rsvArea = NULL;

	if(fstatfs(fd, pStat) == ERROR)
	{
		return(ERROR);
	}
	else
	{
		if((rsvArea = reinterpret_cast<SYSC_usbReservedAreaList *>(getReservedSizeUsb(const_cast<char *>(pDevName)))) != NULL)
		{
			if(static_cast<double>(rsvArea->ReservedSize*1024) < static_cast<double>(pStat->f_bsize))
			{
				pStat->f_bfree -= 1;
			}
			else
			{
				pStat->f_bfree -= static_cast<size_t>((rsvArea->ReservedSize*1024)/pStat->f_bsize);
				if(0!=(static_cast<ssize_t>((static_cast<double>(rsvArea->ReservedSize)) * 1024) % pStat->f_bsize))
				{
					pStat->f_bfree --;
				}
			}
		}
	}

	thresh = static_cast<ssize_t>((pStat->f_blocks * FILD_ThresholdOfDiskFull) / 10000);

	if(thresh > static_cast<ssize_t>(pStat->f_bfree))
	{
		pStat->f_bfree = 0;

#ifdef TORNADO
		if (strcmp(pDevName, devPath()) == 0)
		{
			// USB FULLを設定、通知
			APIC_UsbMemDeviceInfo::newInstance()->setWarning(True, TYPE_UsbMemWarningMemoryFull);
		}
#endif // TORNADO
	}
	else
	{
		pStat->f_bfree -= thresh;
	}

	return(OK);
}

// ---------------------------------------------------------------------------------
// 関数名	：checkFreeSpaceUsb
// ---------------------------------------------------------------------------------
// 概要 	：指定デバイスの空きブロックを取得する。
//			  空きブロックがない場合はerrnoをセットする。
//				in ：pName			フルパスのポインタ
//				out：pFreeBlock 	空きブロック数のポインタ
// 戻り値	：OK
//			  ERROR
//			  *pFreeBlock -> 空きブロック
// 備考 	：
// 作成者	：Hamada
// レビュー ：レビュー実施日と名前を記述
// 変更履歴 ：変更日、変更者、変更内容（変更理由）を記述
//            07/03/08 濱田 77069・76807 USBメモリFULL対応
// ---------------------------------------------------------------------------------
static STATUS checkFreeSpaceUsb(const char *pName, ssize_t *pFreeBlock)
{
	struct statfs			stat = {};
	ssize_t					thresh = 0;
	SYSC_usbReservedAreaList	*rsvArea = NULL;
	char					pDevName[BULK_MAX_DRV_NAME_SZ] = {};

	memset( pDevName, 0, sizeof(pDevName) );
	strncpy( pDevName, devPath(), BULK_MAX_DRV_NAME_SZ - 1 );	/* デバイス名の取得 */// 2007/04/11 問題点DB78797 樋口 詳細はdevPath()参照 

	if(statfs(pDevName, &stat) == ERROR)
	{
		return(ERROR);
	}
	else
	{
		if((rsvArea = reinterpret_cast<SYSC_usbReservedAreaList *>(getReservedSizeUsb(pDevName))) != NULL)
		{
			if(static_cast<double>(rsvArea->ReservedSize*1024) < static_cast<double>(stat.f_bsize))
			{
				stat.f_bfree -= 1;
			}
			else
			{
				stat.f_bfree -= static_cast<size_t>((rsvArea->ReservedSize*1024)/stat.f_bsize);
				if(0!=(static_cast<ssize_t>((static_cast<double>(rsvArea->ReservedSize)) * 1024) % stat.f_bsize))
				{
					stat.f_bfree --;
				}
			}
		}
	}

	thresh = static_cast<ssize_t>((stat.f_blocks * FILD_ThresholdOfDiskFull) / 10000);	/* 0.0x % */
	*pFreeBlock = static_cast<ssize_t>(stat.f_bfree - thresh);

	if (*pFreeBlock <= 0)
	{
#ifdef TORNADO
		if (strcmp(pDevName,devPath()) == 0)
		{
			// USB FULLを設定、通知
			APIC_UsbMemDeviceInfo::newInstance()->setWarning(True, TYPE_UsbMemWarningMemoryFull);
		}
//		NVDF_TRACE("FIL : There is not enough space 1 [%s]\n",pName,0,0,0,0,0);
		SYSF_errnoSet(S_dosFsLib_DISK_FULL);
#endif // TORNADO
	}else{
#ifdef TORNADO
		if ( False!=APIC_UsbMemDeviceInfo::newInstance()->isWarning(TYPE_UsbMemWarningMemoryFull) )
		{
			// USB FULLを解除
			APIC_UsbMemDeviceInfo::newInstance()->setWarning(False, TYPE_UsbMemWarningMemoryFull);
		}
#endif // TORNADO
	}

	return(OK);
}

// ---------------------------------------------------------------------------------
// 関数名	：SYSC_CreatUsb
// ---------------------------------------------------------------------------------
// 概要 	：ファイルを作成する。
//				in	：pathName		フルパスのポインタ
//				in	：flag		モード
// 戻り値	：fd ->ファイルディスクリプタ
//			  ERROR
// 備考 	：
// 作成者	：Hamada
// レビュー ：レビュー実施日と名前を記述
// 変更履歴 ：2007/01/26 問題点72813,72316 Unicode対応修正 制御12 山本 
//            2007/02/27 Unicode変換削除  制御12 山本 
// ---------------------------------------------------------------------------------
int SYSC_CreatUsb (const char *pathName, int flag)
{
	int result = OK;
	int fd = OK;
	BOOL			alreadyFile = FALSE;
	struct stat 	st = {};
	ssize_t			bfree		= 0;
	int ret = OK;
	int i_structCnt = 0;
	

	char	name[SYSC_usbPrintFileNameMax+1];
	if( getFullPath(pathName, name) == ERROR ){
		return ERROR;
	}

	waitWritingUsb ();

	do {
		if (stat(static_cast<char *>(name), &st) == OK)
		{
			if (S_ISREG(st.st_mode))
			{
				alreadyFile = TRUE; 					/* 既にファイルが存在する */
			}
		}

		if ((fd = creat (name, flag)) != ERROR)
		{
			if (alreadyFile != TRUE)
			{				   /* 新たにファイルが作成された時 */
				result = getReservedSizeUsb(name, &bfree);	/* 空き容量の取得 */
				if ((result != OK) ||					/* エラー時 */
					(bfree <= 0))						/* または、空きがない時 */
				{
#ifdef TORNADO
					result = SYSF_errnoGet();
#endif // TORNADO
					SYSF_close(fd);
					remove(name);
#ifdef TORNADO
					(void)SYSF_errnoSet(result);
#endif // TORNADO
					ret = ERROR;
					break;
				}
			}
#ifdef TORNADO
			if ((result = createFileOpenContextUsb (fd, name, i_structCnt)) < 0)
			{
				SYSF_close (fd);
				fd = result;
			}
#endif // TORNADO
		}
	} while(FALSE);
	if(ret == ERROR)
	{
		fd = ret;
	}
	return fd;
}

// ---------------------------------------------------------------------------------
// 関数名	：SYSC_OpenUsb
// ---------------------------------------------------------------------------------
// 概要 	：ファイルをオープンする。
//			  CREATフラグ指定でファイルが無い場合ファイルを作成する。
//			  ただし、ディスク容量が無いときは作成しない。また、CREATフラグかつ
//			  RDONLYフラグが指定された場合はRDWRでファイルを作成（オープン）する。
//				in ：pathName		フルパスのポインタ
//				in ：flags		許可操作指定フラグ
//				in ：mode		モード
// 戻り値	：fd ->ファイルディスクリプタ
//			  ERROR
// 備考 	：
// 作成者	：Hamada
// レビュー ：レビュー実施日と名前を記述
// 変更履歴 ：2007/01/26 問題点72813,72316 Unicode対応修正 制御12 山本 
//            2007/02/27 Unicode変換削除  制御12 山本 
//			  2007/04/18 静的解析指摘対応変更 制御12 山本
// ---------------------------------------------------------------------------------
int SYSC_OpenUsb (const char *pathName, int flags, int mode)
{
	int 		result = OK;
	int 		fd = OK;
	BOOL		alreadyFile = FALSE;
	struct stat st = {};
	ssize_t		bfree = 0;
	int i_structCnt = 0;

	char	name[SYSC_usbPrintFileNameMax+1];

	if( getFullPath(pathName, name) == ERROR ){
		return ERROR;
	}

	waitWritingUsb ();

	do {
		/* create the new file */
		if(0!=(flags & O_CREAT))
		{
			if(stat(static_cast<char *>(name), &st) == OK)
			{
				if (S_ISREG(st.st_mode))
				{
					alreadyFile = TRUE; 					/* 既にファイルが存在する */
				}
			}
			if((flags & ~O_CREAT) == O_RDONLY)
			{
				if((fd = SYSF_open(name, O_RDWR | O_CREAT | O_BINARY, mode)) == ERROR)
				{
					break;
				}
				if (alreadyFile != TRUE)
				{					/* 新たにファイルが作成された時 */
					result = getReservedSizeUsb(name, &bfree);	/* 空き容量の取得 */
					if ((result != OK) ||					/* エラー時 */
						(bfree <= 0))						/* または、空きがない時 */
					{
#ifdef TORNADO
						result = SYSF_errnoGet();
#endif // TORNADO
						SYSF_close(fd);
						fd = ERROR;
						remove(name);
#ifdef TORNADO
						(void)SYSF_errnoSet(result);
#endif // TORNADO
						break;
					}
				}
			}
			else
			{
				if((fd = SYSF_open (name, flags | O_BINARY, mode)) == ERROR)
				{
					break;
				}
				if (alreadyFile != TRUE)
				{					/* 新たにファイルが作成された時 */
					result = getReservedSizeUsb(name, &bfree);	/* 空き容量の取得 */
					if ((result != OK) ||					/* エラー時 */
						(bfree <= 0))						/* または、空きがない時 */
					{
#ifdef TORNADO
						result = SYSF_errnoGet();
#endif // TORNADO
						SYSF_close(fd);
						fd = ERROR;
						remove(name);
#ifdef TORNADO
						(void)SYSF_errnoSet(result);
#endif // TORNADO
						break;
					}
				}
			}
		}
		else
		{
			fd = SYSF_open(name, flags | O_BINARY, mode);
		}
		if(fd != ERROR)
		{
#ifdef TORNADO
			if ( createFileOpenContextUsb (fd, name, i_structCnt) < 0 )
			{
				waitWritingUsb ();
				SYSF_close (fd);
				fd = ERROR;
				break;
			}
#endif // TORNADO
		}
	} while(FALSE);

	return fd;
}
// ---------------------------------------------------------------------------------
// 関数名	：SYSC_RemoveUsb
// ---------------------------------------------------------------------------------
// 概要 	：ファイルを削除する。
//				in	：pathName				フルパスのポインタ
//				in	：b_RemoveImmediately	TRUEの場合上書き対象外とする。
// 戻り値	：OK
//			  ERROR
// 備考 	：
// 作成者	：Hamada
// レビュー ：レビュー実施日と名前を記述
// 変更履歴 ：2007/01/26 問題点72813,72316 Unicode対応修正 制御12 山本 
//            2007/02/27 Unicode変換削除  制御12 山本 
//            07/03/08 濱田 77069・76807 USBメモリFULL対応
//            07/03/22 伊藤史：78280 Removeされない問題対策  引数 BOOL b_RemoveImmediately=FALSE 廃止
// ---------------------------------------------------------------------------------
STATUS SYSC_RemoveUsb (const char *pathName)
{
	int 				result = OK;
	ssize_t				FreeSpace = 0;
	STATUS				ret = ERROR;
	APIC_UsbMemDeviceInfo *pc_UsbMemDeviceInfo = APIC_UsbMemDeviceInfo::newInstance();

	char	name[SYSC_usbPrintFileNameMax+1];

	if( getFullPath(pathName, name) == ERROR ){
		return ERROR;
	}

	waitWritingUsb ();


	result = FILF_Remove(name);

	ret = getReservedSizeUsb(name, &FreeSpace);
	
	if(FreeSpace > 0 && ret != ERROR)
	{
#ifdef TORNADO
		if (memcmp(name, devPath(), 6) == 0 &&
		(False!=pc_UsbMemDeviceInfo->isWarning(TYPE_UsbMemWarningMemoryFull)) )
		{
			// USB FULLを設定、通知
			pc_UsbMemDeviceInfo->setWarning(False, TYPE_UsbMemWarningMemoryFull);
		}
#endif // TORNADO
	}

	return result;
}

// ---------------------------------------------------------------------------------
// 関数名	：SYSC_CloseUsb
// ---------------------------------------------------------------------------------
// 概要 	：ファイルをクローズする。（fd指定）
//				in ：fd 	ファイルディスクリプタ
// 戻り値	：OK
//			  ERROR
// 備考 	：
// 作成者	：Hamada
// レビュー ：レビュー実施日と名前を記述
// 変更履歴 ：変更日、変更者、変更内容（変更理由）を記述
// ---------------------------------------------------------------------------------
STATUS SYSC_CloseUsb (int fd)
{
	STATUS result = ERROR;
	int i_structCnt = 0;
	waitWritingUsb ();

#ifdef TORNADO
#ifndef LINUX    /* honda */
	result = ioctl (fd, FIOFLUSH, 0);
#else
	SYSF_fsync(fd);
#endif
#endif // TORNADO
	result = SYSF_close (fd);
#ifdef TORNADO
	deleteFileOpenContextUsb (fd, i_structCnt);
#endif // TORNADO
	SYSC_SyncForUSB();		// CQ511592.

	return result;
}

// ---------------------------------------------------------------------------------
// 関数名	：SYSC_RenameUsb
// ---------------------------------------------------------------------------------
// 概要 	：ファイル名を変更する。コンテキストに登録されている場合はERRORを返す。
//				in ：oldName		旧フルパスのポインタ
//				in ：newName		新フルパスのポインタ
// 戻り値	：OK
//			  ERROR
// 備考 	：
// 作成者	：Hamada
// レビュー ：レビュー実施日と名前を記述
// 変更履歴 ：2007/01/26 問題点72813,72316 Unicode対応修正 制御12 山本 
//            2007/02/27 Unicode変換削除  制御12 山本 
// ---------------------------------------------------------------------------------
int SYSC_RenameUsb (const char *oldPathName, const char *newPathName)
{
	int result = ERROR;
	int i_structCnt = 0;
	
	char	oldName[SYSC_usbPrintFileNameMax+1];
	char	newName[SYSC_usbPrintFileNameMax+1];
	if( getFullPath(oldPathName, oldName) == ERROR ){
		return ERROR;
	}

	if( getFullPath(newPathName, newName) == ERROR ){
		return ERROR;
	}

	waitWritingUsb ();
#ifdef TORNADO
	if (0!=searchNameUsb (oldName, i_structCnt))
	{
		result = ERROR;
	}
	else
	{
		result = FILF_Rename(oldName, newName);
	}
#else  // Simulator
	result = rename (oldName, newName);
#endif // TORNADO
	return result;
}

// ---------------------------------------------------------------------------------
// 関数名	：SYSC_ReadUsb
// ---------------------------------------------------------------------------------
// 概要 	：readのWrapper
//				in ：fd 		ファイルディスクリプタ
//				out：buffer 	読み込みデータ保存バッファのポインタ
//				in ：maxbytes	読み出す最大byte数
// 戻り値	：OK
//			  ERROR
// 備考 	：
// 作成者	：Hamada
// レビュー ：レビュー実施日と名前を記述
// 変更履歴 ：変更日、変更者、変更内容（変更理由）を記述
// ---------------------------------------------------------------------------------
ssize_t SYSC_ReadUsb (int fd, char *buffer, size_t maxbytes)
{
	ssize_t result = ERROR;

	waitWritingUsb ();
	result = read(fd, buffer, maxbytes);

	return result;
}

// ---------------------------------------------------------------------------------
// 関数名	：SYSC_WriteUsb
// ---------------------------------------------------------------------------------
// 概要 	：Write関数のWrapper。コンテキストに登録されてないときは0を返す。
//			  DiskFullの場合は書き込まないでERRORを返す。
//				in ：fd 		ファイルディスクリプタ
//				in ：buffer 	書き込みデータバッファのポインタ
//				in ：maxbytes	書き込み最大byte数
// 戻り値	：OK
//			  ERROR
// 備考 	：
// 作成者	：Hamada
// レビュー ：レビュー実施日と名前を記述
// 変更履歴 ：2007/01/26 問題点72813,72316 Unicode対応修正 制御12 山本 
//            2007/02/27 Unicode変換削除  制御12 山本 
// ---------------------------------------------------------------------------------
ssize_t SYSC_WriteUsb (int fd, char *buffer, size_t nbytes)
{
	ssize_t result = OK;
	struct statfs st = {};
	char devName[SYSC_usbPrintFileNameMax + 1] = {};
	
	struct stat s = {};
	off_t pos = 0;
#ifdef TORNADO
	SYSC_usbFileOpenContext *pContext = NULL;
	size_t	buffSize = 0;
#endif // TORNADO .
	size_t	dataSize = 0;
	int i_structCnt = 0;

	waitWritingUsb ();
	do {
#ifdef TORNADO
		if((pContext = searchFdUsb(fd, i_structCnt)) == NULL) {
			SYSF_errnoSet(S_dosFsLib_INVALID_PARAMETER);
			result = ERROR;
			break;
		}
#endif // TORNADO

		strcpy(devName, devPath());

		if(getFreeSpaceByFdUsb(fd, devName, &st) == ERROR) {
			result = ERROR;
			break;
		}

		if(st.f_bfree <= static_cast<fsblkcnt_t>(nbytes / st.f_bsize)) {
			do {
				if((fstat(fd, &s) != ERROR) && ((pos = lseek(fd, 0, SEEK_CUR)) != ERROR)) {
					dataSize = static_cast<size_t>(pos) + nbytes;
					if (static_cast<size_t>(s.st_size) >= dataSize) {
						break;
					}
#ifdef TORNADO
#if defined(LINUX)
					if(s.st_blksize > FILD_FstatBlksize){
						buffSize = static_cast<size_t>(s.st_blksize) * static_cast<size_t>(s.st_blocks / (s.st_blksize / FILD_FstatBlksize));
					}else{
					buffSize = static_cast<size_t>(s.st_blksize) * static_cast<size_t>(s.st_blocks);
					}
#else
					buffSize = static_cast<size_t>(s.st_blksize) * static_cast<size_t>(s.st_blocks);
#endif // defined(LINUX)
					if (dataSize <= buffSize) {
						break;
					}
#endif // TORNADO
				}
#ifdef TORNADO
//				NVDF_TRACE("FIL : There is not enough space 3 [%s] size[%ld]\n",c_FilePath,nbytes,0,0,0,0);
				SYSF_errnoSet(S_dosFsLib_DISK_FULL);
#endif // TORNADO
				result = ERROR;
			} while(FALSE);
		}
	} while(FALSE);

	if(result != ERROR)
	{
		result = write (fd, buffer, nbytes);
	}

	return result;
}

// ---------------------------------------------------------------------------------
// 関数名	：SYSC_LseekUsb
// ---------------------------------------------------------------------------------
// 概要 	：ファイルサイズを超えるoffsetが指定された場合0詰めを行う。
//				in ：fd 		ファイルディスクリプタ
//				in ：offset 	移動量
//				in ：whence 	初期位置
// 戻り値	：OK
//			  ERROR
// 備考 	：
// 作成者	：Hamada
// レビュー ：レビュー実施日と名前を記述
// 変更履歴 ：2007/01/26 問題点72813,72316 Unicode対応修正 制御12 山本 
//            2007/02/27 Unicode変換削除  制御12 山本 
// ---------------------------------------------------------------------------------
off_t SYSC_LseekUsb (int fd, off_t offset, int whence)
{
	struct stat st = {};
	off_t pos = 0;
	struct statfs s = {};
	char devName[SYSC_usbPrintFileNameMax + 1] = {};

#ifdef TORNADO
	SYSC_usbFileOpenContext *pContext = NULL;
#endif // TORNADO .
	char  add_data = 0x00;
	int add_len = 1;
	off_t result = OK;
	int i_structCnt = 0;

	waitWritingUsb ();
	do {
		if (fstat (fd, &st) == ERROR)
		{
			result = ERROR;
			break;
		}
		switch (whence)
		{
			case SEEK_CUR:
				if ((pos = lseek (fd, 0, SEEK_CUR)) == ERROR)
				{
					result = ERROR;
				}
				pos += offset;
				break;
			case SEEK_END:
				pos = st.st_size + offset;
				break;
			default: /* SEEK_SET */
				pos = offset;
				break;
		}
		if(result == ERROR)
		{
			break;
		}
		if (pos > st.st_size)
		{
			if (lseek (fd, 0, SEEK_END) == ERROR)
			{
				result = ERROR;
				break;
			}
#ifdef TORNADO
			// --- ディスクフルチェック・コンテキスト取得 ---
			if((pContext = searchFdUsb(fd, i_structCnt)) == NULL)
			{
				SYSF_errnoSet(S_dosFsLib_INVALID_PARAMETER);
				result = ERROR;
				break;
			}
#endif // TORNADO

			strcpy(devName, devPath());
			if(getFreeSpaceUsb(devName, &s) == ERROR)
			{
				result = ERROR;
				break;
			}
			if(static_cast<off_t>(s.f_bsize) > (pos - st.st_size))
			{
				;
			}
			else
			{
				if(s.f_bfree < static_cast<fsblkcnt_t>((pos-st.st_size)/s.f_bsize))
				{
#ifdef TORNADO
//					NVDF_TRACE("FIL : There is not enough space 4 [%s] size[%ld] pos[%ld]\n",c_FilePath,offset,whence,0,0,0);
					SYSF_errnoSet(S_dosFsLib_DISK_FULL);
#endif // TORNADO
					result = ERROR;
					break;
				}
			}

			for (off_t i = 0; i < pos - st.st_size; i++)
			{
				if (write (fd, &add_data, add_len) != add_len)
				{
					result = ERROR;
					break;
				}
			}
		}
		if(result == ERROR)
		{
			break;
		}
	}while (FALSE);
	
	if(result != ERROR)
	{
		result = lseek (fd, pos, SEEK_SET);
	}

	return result;
}

// ---------------------------------------------------------------------------------
// 関数名	：SYSC_MkdirUsb
// ---------------------------------------------------------------------------------
// 概要 	：mkdir関数のWrapper
//				in ：dirPathName	フルパスのポインタ
// 戻り値	：OK
//			  ERROR
// 備考 	：
// 作成者	：Hamada
// レビュー ：レビュー実施日と名前を記述
// 変更履歴 ：2007/01/26 問題点72813,72316 Unicode対応修正 制御12 山本 
//            2007/02/27 Unicode変換削除  制御12 山本 
// ---------------------------------------------------------------------------------
STATUS SYSC_MkdirUsb (char *dirPathName)
{
	STATUS result = ERROR;
	
	char	dirName[SYSC_usbPrintFileNameMax+1];

	if( getFullPath(dirPathName, dirName) == ERROR ){
		return ERROR;
	}

	waitWritingUsb ();
#ifdef LINUX
	result = SYSF_mkdir (dirName);
#else
	result = mkdir (dirName);
#endif	

	return result;
}

// ---------------------------------------------------------------------------------
// 関数名	：SYSC_RmdirUsb
// ---------------------------------------------------------------------------------
// 概要 	：rmdir関数のWrapper
//				in ：dirPathName	フルパスのポインタ
// 戻り値	：OK
//			  ERROR
// 備考 	：
// 作成者	：Hamada
// レビュー ：レビュー実施日と名前を記述
// 変更履歴 ：2007/01/26 問題点72813,72316 Unicode対応修正 制御12 山本 
//            2007/02/27 Unicode変換削除  制御12 山本 
// ---------------------------------------------------------------------------------
STATUS SYSC_RmdirUsb (char *dirPathName)
{
	STATUS result = ERROR;

	char	dirName[SYSC_usbPrintFileNameMax+1];
	
	if( getFullPath(dirPathName, dirName) == ERROR ){
		return ERROR;
	}

	waitWritingUsb ();

	result = rmdir (dirName);

	return result;
}

// ---------------------------------------------------------------------------------
// 関数名	：SYSC_StatUsb
// ---------------------------------------------------------------------------------
// 概要 	：指定ファイル名の情報を取得する。
//				in	：pathName	フルパスのポインタ
//				out ：pStat stat構造体変数のポインタ
// 戻り値	：OK
//			  ERROR
// 備考 	：
// 作成者	：Hamada
// レビュー ：レビュー実施日と名前を記述
// 変更履歴 ：2007/01/26 問題点72813,72316 Unicode対応修正 制御12 山本 
//            2007/02/27 Unicode変換削除  制御12 山本 
// ---------------------------------------------------------------------------------
STATUS SYSC_StatUsb (char *pathName, struct stat *pStat)
{
	int result = ERROR;

	char	name[SYSC_usbPrintFileNameMax+1];

	if( getFullPath(pathName, name) == ERROR ){
		return ERROR;
	}

	result = stat(name, pStat);

	return(result);
}

// ---------------------------------------------------------------------------------
// 関数名	：SYSC_FopenUsb
// ---------------------------------------------------------------------------------
// 概要 	：コンテキストに登録する。ファイルをオープンする。
//				in ：fileName		フルパスのポインタ
//				in ：mode		モード
// 戻り値	：fp ->ファイルポインタ
//			  NULL
// 備考 	：
// 作成者	：Hamada
// レビュー ：レビュー実施日と名前を記述
// 変更履歴 ：2007/01/26 問題点72813,72316 Unicode対応修正 制御12 山本 
//            2007/02/27 Unicode変換削除  制御12 山本 
// ---------------------------------------------------------------------------------
FILE *SYSC_FopenUsb (const char *fileName, const char *mode)
{
	FILE *fp = NULL;
	int fd = 0;
	int ret = ERROR;
	STATUS result = ERROR;
	struct stat pfileStat = {};
	ssize_t FreeSpace = 0;
	int i_structCnt = 0;
	
	char	file[SYSC_usbPrintFileNameMax+1];

	if( getFullPath(fileName, file) == ERROR ){
		return fp;
	}

	waitWritingUsb ();

	ret = stat(static_cast<char *>(file), &pfileStat);
	result = checkFreeSpaceUsb(const_cast<const char *>(file), &FreeSpace);
	if(result == ERROR)
	{
		return fp;
	}

	do {
		if(FreeSpace > 0)
		{
			if ((fp = SYSF_fopen (file, mode)) == NULL) {
				break;
			}
			if ((fd = fileno (fp)) == -1) {
				SYSF_fclose (fp);
				fp = NULL;
				break;
			}
#ifdef TORNADO
			if (createFileOpenContextUsb (fd, file, i_structCnt) < 0) {
				SYSF_fclose (fp);
				fp = NULL;
				break;
			}
#endif // TORNADO
		}
		else if(FreeSpace <= 0 && ret != ERROR)
		{
			if ((fp = SYSF_fopen (file, mode)) == NULL) {
				break;
			}
			if ((fd = fileno (fp)) == -1) {
				SYSF_fclose (fp);
				fp = NULL;
				break;
			}
#ifdef TORNADO
			if (createFileOpenContextUsb (fd, file, i_structCnt) < 0) {
				SYSF_fclose (fp);
				fp = NULL;
				break;
			}
#endif // TORNADO
		}
		else {
			// no operation.
			DBGF_PrintNoOperationPath(__FILE__, __LINE__);
		}
	} while (FALSE);
#ifdef LINUX		// Linux統合検証FUM FVT不具合No.51  第2電情技 岡本.
	if (fp != NULL) {
		setFileOpenInfo(fd, file);
	}
#endif	// LINUX.

	return fp;
}

// ---------------------------------------------------------------------------------
// 関数名	：SYSC_FcloseUsb
// ---------------------------------------------------------------------------------
// 概要 	：コンテキストを削除してファイルをクローズする。
//				in ：fp 	FILE構造体のポインタ
// 戻り値	：OK
//			  ERROR
// 備考 	：
// 作成者	：Hamada
// レビュー ：レビュー実施日と名前を記述
// 変更履歴 ：変更日、変更者、変更内容（変更理由）を記述
// ---------------------------------------------------------------------------------
int SYSC_FcloseUsb (FILE* fp)
{
	int result = ERROR;
	int fd = 0;
	int i_structCnt = 0;

	waitWritingUsb ();
	if ((fd = fileno (fp)) != -1)
	{
#ifdef TORNADO
		deleteFileOpenContextUsb (fd, i_structCnt);
#endif // TORNADO
	}
	if(fd != -1)
	{
		result = fflush(fp);
		result = syncfs(fd);
		result = SYSF_fclose (fp);
#ifdef LINUX		// Linux統合検証FUM FVT不具合No.51  第2電情技 岡本.
		clearFileOpenInfo(fd);
#endif	// LINUX.
	}
	SYSC_SyncForUSB();		// CQ511592.

	return result;
}

// 2019.11.6 OP_BTS-14651対応 start.
// ---------------------------------------------------------------------------------
// 関数名	：SYSC_FcloseUsbRead
// ---------------------------------------------------------------------------------
// 概要 	：コンテキストを削除してファイルをクローズする。(読み出しで使用時のクローズ)
//				in ：fp 	FILE構造体のポインタ
// 戻り値	：OK
//			  ERROR
// 備考 	：
// 作成者	：Uekama
// レビュー ：レビュー実施日と名前を記述
// 変更履歴 ：変更日、変更者、変更内容（変更理由）を記述
// ---------------------------------------------------------------------------------
int SYSC_FcloseUsbRead (FILE* fp)
{
	int result = ERROR;
	int fd = 0;
	int i_structCnt = 0;

	waitWritingUsb ();
	if ((fd = fileno (fp)) != -1)
	{
#ifdef TORNADO
		deleteFileOpenContextUsb (fd, i_structCnt);
#endif // TORNADO
	}
	if(fd != -1)
	{
		result = fflush(fp);
		result = syncfs(fd);
		result = SYSF_fclose (fp);
#ifdef LINUX
		clearFileOpenInfo(fd);
#endif	// LINUX.
	}
	
	// SYSC_SyncForUSB(); Read用途で使用するので呼び出さない.

	return result;
}
// 2019.11.6 OP_BTS-14651対応 end.

// ---------------------------------------------------------------------------------
// 関数名	：SYSC_FreadUsb
// ---------------------------------------------------------------------------------
// 概要 	：fread関数のWrapper
//				out：buf		データを保存するバッファのポインタ
//				in ：size		データひとつのサイズ
//				in ：count		読み出す最大データ数
//				in ：fp 		読み出すストリーム
// 戻り値	：OK
//			  ERROR
// 備考 	：
// 作成者	：Hamada
// レビュー ：レビュー実施日と名前を記述
// 変更履歴 ：変更日、変更者、変更内容（変更理由）を記述
// ---------------------------------------------------------------------------------
ssize_t SYSC_FreadUsb (void *buf, size_t size, size_t count, FILE *fp)
{
	ssize_t result = ERROR;

	waitWritingUsb ();
	result = fread (buf, size, count, fp);

	return result;
}

// ---------------------------------------------------------------------------------
// 関数名	：SYSC_FwriteUsb
// ---------------------------------------------------------------------------------
// 概要 	：fwrite関数のWrapper。コンテキストに登録されてないときは0を返す。
//			  DiskFullの場合は書き込まないでERRORを返す。
//				in ：buf		書き込み文字列
//				in ：size		書き込み文字数
//				in ：count		書き込み最大回数
//				in ：fp 		ストリーム
// 戻り値	：OK
//			  ERROR
// 備考 	：
// 作成者	：Hamada
// レビュー ：レビュー実施日と名前を記述
// 変更履歴 ：2007/01/26 問題点72813,72316 Unicode対応修正 制御12 山本 
//            2007/02/27 Unicode変換削除  制御12 山本 
// ---------------------------------------------------------------------------------
ssize_t SYSC_FwriteUsb (const void *buf, size_t size, size_t count, FILE *fp)
{
	ssize_t result = OK;
	int fd = 0;
#ifdef TORNADO
	SYSC_usbFileOpenContext *pContext = NULL;
#endif // TORNADO .
	
	char devName[SYSC_usbPrintFileNameMax + 1] = {};
	
	struct statfs st = {};
	struct stat s = {};
	off_t pos = 0;
	int i_structCnt = 0;

	waitWritingUsb ();
	do {
		if ((fd = fileno (fp)) == -1)
		{
			result = ERROR;
			break;
		}
#ifdef TORNADO
		if((pContext = searchFdUsb(fd, i_structCnt)) == NULL)
		{
			SYSF_errnoSet(S_dosFsLib_INVALID_PARAMETER);
			result = ERROR;
			break;
		}
#endif // TORNADO

		memset(devName, 0x00, sizeof(devName));
		if ( getDevPath( fd, devName ) == ERROR )
		{
			result = ERROR;
			break;
		}
		

		if(getFreeSpaceByFdUsb(fd, devName, &st) == ERROR)
		{
			result = ERROR;
			break;
		}

		if(st.f_bfree <= static_cast<fsblkcnt_t>((size * count) / st.f_bsize))
		{
			do {
				if((fstat(fd, &s) != ERROR) && ((pos = ftell(fp)) != -1))
				{
					if((s.st_size - pos) >= static_cast<off_t>(size * count))
					{
						break;
					}
				}
#ifdef TORNADO
//				NVDF_TRACE("FIL : There is not enough space 6 [%s] size[%ld]\n",c_FilePath,size,0,0,0,0);
				SYSF_errnoSet(S_dosFsLib_DISK_FULL);
#endif // TORNADO
				result = ERROR;
			} while(FALSE);
		}
	} while(FALSE);

	if(result != ERROR)
	{
		result = fwrite (buf, size, count, fp);
	}

	return result;
}

// ---------------------------------------------------------------------------------
// 関数名	：SYSC_FseekUsb
// ---------------------------------------------------------------------------------
// 概要 	：ファイルサイズを超えるoffsetが指定された場合0詰めを行う。
//				in ：fp 		ストリーム
//				in ：offset 	移動量
//				in ：whence 	初期位置
// 戻り値	：OK
//			  ERROR
// 備考 	：
// 作成者	：Hamada
// レビュー ：レビュー実施日と名前を記述
// 変更履歴 ：2007/01/26 問題点72813,72316 Unicode対応修正 制御12 山本 
//            2007/02/27 Unicode変換削除  制御12 山本 
// ---------------------------------------------------------------------------------
off_t SYSC_FseekUsb (FILE * fp, off_t offset, int whence)
{
	off_t pos = 0;
	int fd = 0;
	struct stat st = {};
	
	char devName[SYSC_usbPrintFileNameMax + 1] = {};
	
#ifdef TORNADO
	SYSC_usbFileOpenContext *pContext = NULL;
#endif // TORNADO .
	struct statfs s = {};
	char  add_data = 0x00;
	int add_len = 1;
	off_t result = OK;
	int i_structCnt = 0;

	waitWritingUsb ();
	do {
		if((fd = fileno(fp)) == -1)
		{
			result = ERROR;
			break;
		}
		if (fstat (fd, &st) == ERROR)
		{
			result = ERROR;
			break;
		}
	/* move to upper */
		switch (whence)
		{
			case SEEK_CUR:
				if ((pos = ftell(fp)) == -1) {
					result = ERROR;
				}
				pos += offset;
				break;
			case SEEK_END:
				pos = st.st_size + offset;
				break;
			default: /* SEEK_SET */
				pos = offset;
				break;
		}
		if(result == ERROR)
		{
			break;
		}
		if (pos > st.st_size)
		{
			if (fseek (fp, 0, SEEK_END) == ERROR)
			{
				result = ERROR;
				break;
			}

#ifdef TORNADO
			if((pContext = searchFdUsb(fd, i_structCnt)) == NULL)
			{
				SYSF_errnoSet(S_dosFsLib_INVALID_PARAMETER);
				result = ERROR;
				break;
			}
#endif // TORNADO
			strcpy(devName, devPath());
			if(getFreeSpaceUsb(devName, &s) == ERROR)
			{
				result = ERROR;
				break;
			}
			if(static_cast<off_t>(s.f_bsize) > (pos - st.st_size))
			{
				;
			}
			else
			{
				if(s.f_bfree < static_cast<fsblkcnt_t>((pos-st.st_size)/s.f_bsize))
				{
#ifdef TORNADO
//					NVDF_TRACE("FIL : There is not enough space 5 [%s] size[%ld] pos[%ld]\n",c_FilePath,offset,whence,0,0,0);
					SYSF_errnoSet(S_dosFsLib_DISK_FULL);
#endif // TORNADO
					result = ERROR;
					break;
				}
			}

			for (off_t i = 0; i < pos - st.st_size; i++)
			{
				if (write (fd, &add_data, add_len) != add_len)
				{
					result = ERROR;
					break;
				}
			}
		}
	} while(FALSE);
	if(result != ERROR)
	{
/* move to lower */
		result = fseek (fp, pos, SEEK_SET);
	}

	return result;
}

// ---------------------------------------------------------------------------------
// 関数名	：SYSC_FerrorUsb
// ---------------------------------------------------------------------------------
// 概要 	：指定ファイルポインタのエラー情報を取得する。.
//				in ：fp 	FILE構造体のポインタ.
// 戻り値	： 0 : OK.
//			   Not 0 : Error.
// 備考 	：.
// 作成者	：.
// レビュー ：.
// 変更履歴 ：.
// ---------------------------------------------------------------------------------
int SYSC_FerrorUsb (FILE * fp)
{
	int result = 0;

	result = ferror( fp );

	return(result);
}

// ---------------------------------------------------------------------------------
// 関数名	：SYSC_FstatUsb
// ---------------------------------------------------------------------------------
// 概要 	：指定ファイルディスクリプタの情報を取得する。
//				in	：fd	ファイルディスクリプタ
//				out ：pStat stat構造体変数のポインタ
// 戻り値	：OK
//			  ERROR
// 備考 	：
// 作成者	：Hamada
// レビュー ：レビュー実施日と名前を記述
// 変更履歴 ：変更日、変更者、変更内容（変更理由）を記述
// ---------------------------------------------------------------------------------
STATUS SYSC_FstatUsb (int fd, struct stat *pStat)
{
#ifdef TORNADO
	SYSC_usbFileOpenContext	*pOpenContext = NULL;
#endif // TORNADO .
	int result = ERROR;
	int i_structCnt = 0;

	waitWritingUsb ();
	do {
#ifdef TORNADO
		if((pOpenContext = searchFdUsb(fd, i_structCnt)) == NULL)
		{
			// --- コンテキストに登録されていない ---
			result = ERROR;
			break;
		}
#endif // TORNADO
		result = fstat(fd, pStat);
	} while(FALSE);

	return(result);
}

// ---------------------------------------------------------------------------------
// 関数名	：SYSC_FgetsUsb
// ---------------------------------------------------------------------------------
// 概要 	：fgets関数のWrapper
//				out：buf		データを保存するバッファのポインタ
//				in ：size		読み出す最大データ数
//				in ：fp 		読み出すストリーム
// 戻り値	：buf
//			  EOF、エラーの時はNULL
// 備考 	：
// 作成者	：
// レビュー ：レビュー実施日と名前を記述
// 変更履歴 ：変更日、変更者、変更内容（変更理由）を記述
// ---------------------------------------------------------------------------------
char*	SYSC_FgetsUsb(char* buf, int size, FILE* fp)
{
	waitWritingUsb ();

	return fgets(buf, size, fp);
}

// ---------------------------------------------------------------------------------
// 関数名	：SYSC_umountUsb.
// ---------------------------------------------------------------------------------
// 概要 	：USBデバイスのumount.
//				in ：psc_deviceName		デバイス名文字列.
// 戻り値	：OK.
//			  ERROR.
// 備考 	：.
// 作成者	：鳥居.
// レビュー ：.
// 変更履歴 ：.
// ---------------------------------------------------------------------------------
int SYSC_umountUsb( char* psc_deviceName )
{
	int errsv = 0;
	int i_umountRet = 0;
	int i_result = ERROR;

	if( NULL == psc_deviceName ){
		printf("SYSC_umountUsb pram NULL\n");
		return	i_result;
	}

	i_umountRet = umount2(psc_deviceName, MNT_FORCE );
	errsv = errno;
	if( i_umountRet == 0 ){
		i_result = OK;
	}else{
		i_result = ERROR;
		perror("SYSC_UmountUSB umount2");
	}
	switch( errsv )
	{
		case EBUSY:				// 使用中 (busy) のため、 target をアンマウントできなかった.
			// アンマウント失敗、かつ、Busyでなくなるまで繰り返す[CQ527221].
			while( i_umountRet != 0 && errsv == EBUSY )
			{
				SYSF_taskDelay(static_cast<int>(SYSF_sysClkRateGet() * 200 / 1000));   // 遅延(0.2s).
				i_umountRet = umount2(psc_deviceName, MNT_FORCE);
				errsv = errno;
				if( i_umountRet == 0 ){
					i_result = OK;
				}else{
					i_result = ERROR;
					perror("SYSC_UmountUSB umount2");
				}
			}
			break;
		case EINVAL:			// target がマウントポイントではない.
		case EAGAIN:			// umountリトライ要.
		case EFAULT:			// 失敗.
		case ENAMETOOLONG:		// パス名の長さが MAXPATHLEN より長かった.
		case ENOENT:			// パス名が空である。もしくは指定されたパスが存在しない.
		case ENOMEM:			// カーネルがファイル名やデータをコピーするための空きページを確保できなかった.
		case EPERM:				// 呼び出し元が必要な権限を持っていない.
		default :
			break;
	}
	return	i_result;
}

// 暫定:HW対応 fs_doi.
#if defined(LINUX) 
#if defined(TORNADO) 
#ifndef LINUX
DOS_VOLUME_DESC_ID dosFsVolDescGet(void * pDevNameOrPVolDesc,u_char **   ppTail) 
{
	DOS_VOLUME_DESC_ID tmp=NULL;
	return tmp;
}
#endif /*ifndef(LINUX)*/
#endif /*defined(TORNADO)*/
void ioTaskStdSet (int taskId, int stdFd, int newFd)
{
}

// ---------------------------------------------------------------------------------
// 関数名	：DebugDispdevPath.
// 概要     ：デバイス情報の表示（デバッグ用）.
// ---------------------------------------------------------------------------------
void DebugDispdevPath()
{
	// USB情報から、マウント状態を取得.
	for (int i = 0; i < FILD_DEVICENUM;i++) {
		if (0 == s_usbInfo[ i ].c_DevName[0]) {
			break;
		}
		printf("count = %d c_DevName = %s \n",i,s_usbInfo[ i ].c_DevName);
	}
	printf("device num = %d !!!\n",static_cast<int>(SYSC_GetBulkDeviceNames(NULL,0)));
}

#endif/*defined(LINUX) */

// ---------------------------------------------------------------------------------.
// 関数名	：getFreeSpaceSizeUsb.
// ---------------------------------------------------------------------------------.
// 概要 	： 指定デバイスの空きブロックを取得する。.
//			   空きブロックがない場合はerrnoをセットする。.
//				in	：pName	デバイス名.
//				out ：pFreeBlock int32_tポインタ.
// 戻り値	：.
// 備考 	：.
// 作成者	：海田.
// レビュー ：レビュー実施日と名前を記述.
// 変更履歴 ：変更日、変更者、変更内容（変更理由）を記述.
// ---------------------------------------------------------------------------------.
void getFreeSpaceSizeUsb(char *pName, int32_t *pFreeBlock)
{
	struct statfs				stat = {};
	SYSC_usbReservedAreaList	*rsvArea = NULL;

	if(statfs(pName, &stat) == ERROR)
	{
		return;
	}
	else
	{
		if((rsvArea = reinterpret_cast<SYSC_usbReservedAreaList *>(getReservedSizeUsb(pName))) != NULL)
		{
			if(static_cast<double>(rsvArea->ReservedSize*1024) < static_cast<double>(stat.f_bsize))
			{
				stat.f_bfree -= 1;
			}
			else
			{
				stat.f_bfree -= static_cast<size_t>((rsvArea->ReservedSize*1024)/stat.f_bsize);
				if(0!=(static_cast<ssize_t>((static_cast<double>(rsvArea->ReservedSize)) * 1024) % stat.f_bsize))
				{
					stat.f_bfree --;
				}
			}
		}
	}

	// 空きブロック数だと空き容量の判断が正常に出来ない.
	// 下記のように、ブロック数比較にする事で.
	// 最大ブロック数がさらに大きく確保できる.
	if (stat.f_bfree > static_cast<fsblkcnt_t>(0x7fffffff / stat.f_bsize)) {
		*pFreeBlock = 0x7fffffff;
	} else {
		*pFreeBlock = static_cast<int32_t>(stat.f_bfree * stat.f_bsize);
	}
}

// UV-01224(BOXを含むデータのバックアップ引継ぎ機能 (MA他・管理改善3)).
// HDD規模のサイズを扱えるように機能追加(500Gを想定).
STATUS getFreeSpaceSizeUsbHdd( char *pName, SAA_LLONG *pFreeBlock )
{
	STATUS si_Result = ERROR;
	SAA_LLONG	sll_UsbHddFreeSize = 0;	// USBHDD空きサイズ.
	SAA_LLONG	sll_UsbHddFreeSizeMax = 0;	// USBHDD空きサイズ上限(500Gbyte).
	SAA_LLONG	sll_UsbHddFreeThresh = 0;	// USBHDD空き閾値.

	if( NULL == pName )
	{
		SYSF_logMsg( "getFreeSpaceSizeUsbHdd(...) pName is NULL!!\n", 0, 0, 0, 0, 0, 0 );
	}
	else if( NULL == pFreeBlock )
	{
		SYSF_logMsg( "getFreeSpaceSizeUsbHdd(...) pFreeBlock is NULL!!\n", 0, 0, 0, 0, 0, 0 );
	}
	else
	{
		struct statfs	stat = {};

		if( OK == statfs( pName, &stat ) )
		{
			sll_UsbHddFreeSize = static_cast< SAA_LLONG >( stat.f_bfree * stat.f_bsize );

			// 整数定数領域がsize_t型で確保されているために4byteを超過する定数を通常の演算で使おうとするとwarningが出る.
			// (演算の保証がされるか怪しい).
			// シフト演算で500G値を構築することで上記を回避する.
			////////////////////////////////////////////////////////////////////////////////.
			sll_UsbHddFreeSizeMax = 0x7D;
			sll_UsbHddFreeSizeMax = sll_UsbHddFreeSizeMax << 32;
			////////////////////////////////////////////////////////////////////////////////.

			if( sll_UsbHddFreeSize > sll_UsbHddFreeSizeMax )
			{
				*pFreeBlock = sll_UsbHddFreeSizeMax;
			}
			else
			{
				// USB HDDの総ブロック数の空き有無の閾値を設定する( 全体の(SYSD_UsbHddFreeThresh)% ).
				if( stat.f_blocks < 100 )
				{	// 1%で1block未満となるが、2%以上で1block以上になり得る構成の考慮.
					// (総block数が大きい場合に使用するとオーバーフローする可能性があるため、特定条件下以外は使わない).
					sll_UsbHddFreeThresh = static_cast< SAA_LLONG >( ( stat.f_blocks * SYSD_UsbHddFreeThresh ) / 100 * stat.f_bsize );
				}
				else
				{
					sll_UsbHddFreeThresh = static_cast< SAA_LLONG >( ( stat.f_blocks / 100 ) * SYSD_UsbHddFreeThresh * stat.f_bsize );
				}

				if( sll_UsbHddFreeThresh >= sll_UsbHddFreeSize )
				{	// 空きサイズが全体の1%以下の場合→0byteを返す.
					*pFreeBlock = 0;
				}
				else
				{	// 空きサイズが全体の1%より大きい場合→( 空きサイズ - 1%サイズ )値を返す.
					*pFreeBlock = static_cast< SAA_LLONG >( sll_UsbHddFreeSize - sll_UsbHddFreeThresh );
				}
			}
			si_Result = OK;
		}
	}

	if( ( ERROR == si_Result )
	 && ( NULL != pFreeBlock ) )
	{
		*pFreeBlock = 0;
	}

	return ( si_Result );
}

#ifdef TORNADO
// thr/SATA2/THRC_HddTrans.cppから移植.
int execprog_in(
    char* prog,
    char* buf,
    size_t siz
)
{
    FILE* in_pipe = NULL;
    int ret = 0;

    if( (in_pipe = popen( prog, "r" )) == NULL ){
        printf("[SATA2] execprog_in popen %d\n", SYSF_errnoGet());
        return -1;
    }

    memset( buf, 0, siz );

    fread( buf, sizeof(char), siz, in_pipe );
    // エラーチェック処理を追加.
    if( ferror( in_pipe ) != 0 ){
        perror( "execprog_in fread" );
    }

    ret = pclose( in_pipe );

    return ret;
}

// thr/SATA2/THRC_HddTrans.cppから移植.
int execprog_out(
    char* prog,
    char* buf,
    size_t siz
)
{
    FILE* out_pipe = NULL;
    int ret = 0;

    if( (out_pipe = popen( prog, "w" )) == NULL ){
        printf("[SATA2] execprog_out popen %d\n", SYSF_errnoGet());
        return -1;
    }

    fwrite( buf, sizeof(char), siz, out_pipe );
    // エラーチェック処理を追加.
    if( ferror( out_pipe ) != 0 ){
        perror( "execprog_out fwrite" );
    }

    ret = pclose( out_pipe );

    return ret;
}

// thr/SATA2/THRC_HddTrans.cppから移植.
int execprog(
    char* prog
)
{
    // fread停止問題対応 popenをやめてsystemで実行するように処理を変更.
    int ret = 0;
	char cmd[512] = {};

    sprintf( cmd, "%s 1> /dev/null", prog );

#ifdef TORNADO			// 実機で有効. 
    ret = system( cmd );
#else 
    ret = 0;
#endif // #ifndef TORNADO. 

    if( ret == -1 ){
        printf("[SATA2] execprog system %d\n", SYSF_errnoGet());
    }

    return WEXITSTATUS( ret );
}

// thr/SATA2/THRC_HddTrans.cppから移植.
int formatFS(
    const char* devpath
)
{
    int ret = 0;
	char cmd[256] = {};

    snprintf( cmd, sizeof(cmd), FMT_PART_MKE2FS, devpath );
    ret = execprog( cmd );
    if( 0!=ret ){
        return -1;
    }

    sync();

    return 0;
}
#endif // #ifdef TORNADO

// ---------------------------------------------------------------------------------
// 関数名	：SYSC_formatUSBBulkDevice
// ---------------------------------------------------------------------------------
// 概要 	：USB HDDをext3でフォーマットする.
// 引数		：なし.
// 戻り値	：typedef int SYSE_FormatUSBResult;		フォーマット実行結果.
//				SYSE_FormatUSBResult_OK				フォーマット成功.
//				SYSE_FormatUSBResult_DevChkErr		デバイス検出エラー.
//				SYSE_FormatUSBResult_UmountErr		アンマウントエラー.
//				SYSE_FormatUSBResult_PartErr		パーティションエラー.
//				SYSE_FormatUSBResult_FormatErr		フォーマットエラー.
//				SYSE_FormatUSBResult_MountErr		マウントエラー.
// 備考 	：
//				・USB HDDがマウントされていた場合は自動的にアンマウントを行なってから.
//				  フォーマットを行ないます。.
//				・フォーマット後は自動的にマウントします。.
//				・USB HDDが使用中にこの関数を実行するとアンマウントでエラーになるので注意すること。.
//				・パーティションサイズについて.
//					パーティションサイズは500GBで切り直す。(HDDの物理的なサイズが500GB以上でも。).
//					HDD容量が500GB以下の場合は、最大容量でパーティションを切る。.
//				・エラー詳細について.
//					フォーマット実行結果では、フォーマットシーケンスの手順に応じた結果を返す。.
//					フォーマット実行結果がエラーであった場合、どのような要因でエラーとなったのか.
//					エラー詳細を確認する際には、OSによってグローバル変数の「errno」にエラー詳細が.
//					設定されているので、この変数を参照すること。.
// 作成者	：Harada
// レビュー ：レビュー実施日と名前を記述.
// 変更履歴 ：変更日、変更者、変更内容（変更理由）を記述.
//			  2012/05/30 Harada UV-01224 CQ.365410、CQ.365411 ext3フォーマット問題対策.
//								ext3フォーマット実行中はフラグによりデバイス情報の設定処理を実施しない対応.
// ---------------------------------------------------------------------------------
static const int32_t UMOUNT_EXE_NUM =		500;				// アンマウント回数.
static const uint64_t HDD_SIZE_500GB =		536870912000;	// 500GBのバイト数 500*1024*1024*1024.
static const int32_t BLKD_FORMAT_USB_DELAY =			60;

// 2020/8/31 OP_BTS-26686対応 start.
static char* p_mountNameTbl[] = {
	(scBULK_MOUNT_NAME_DEV1_PART1),	/* "/bd/sdc1"  Mount name device1 partition1 */
	(scBULK_MOUNT_NAME_DEV1),			/* "/bd/sdc"   Mount name device1 */
};
// 2020/8/31 OP_BTS-26686対応 end.

static int si_errno = 0;			// エラー番号保存用.

SYSE_FormatUSBResult SYSC_formatUSBBulkDevice(void)
{
#ifdef TORNADO
	int ret = 0;
	Bool bl_umountErr = False;
	int i=0, j=0;
	char cmd[256];
	char* ptr=NULL;
	uint64_t startSec=0, endSec=0;
	uint64_t totalBytes=0, sectorSize=0;
	char sbuf[1024];
	char val[32];
	char *endptr=NULL;
	char sc_devName[16];

printf("SYSC_formatUSBBulkDevice()\n");

	// 2020/8/31 OP_BTS-26686対応 start.
	// バルクデバイス名生成.
	SYSC_MakeBulkDeviceName();
	// 2020/8/31 OP_BTS-26686対応 end.

	// 2019.10.7 OP_BTS-11575 start.
	for(int cnt = 0;cnt < VIRUSSCAN_FINALIZE_WAIT; cnt++)
	{
printf("##VirusScan canFinalize() cnt=%d\n",cnt);
		if(True == APIC_VirusScan::newInstance()->canFinalize())
		{
printf("##VirusScan finalize\n");
			APIC_VirusScan::newInstance()->finalize();
			cnt=VIRUSSCAN_FINALIZE_WAIT;
		}
		else
		{
			SYSF_taskDelay( static_cast<int>( SYSF_sysClkRateGet() * 1000 / 1000 ) );	//1sec遅延.
		}
	}
	// 2019.10.7 OP_BTS-11575 end.

printf("bl_isFormatUSB = True\n");
	bl_isFormatUSB = True;		// ext3フォーマット実行中.

	si_errno = 0;

printf("##access()\n");
	// ◆デバイス確認.
	if ( -1 == access(scBULK_DEVICE_NAME_DEV1, F_OK) ) {
		si_errno = errno;
		printf("SYSC_formatUSBBulkDevice() SYSE_FormatUSBResult_DevChkErr  access() ERR! errno=%d\n", si_errno);
		bl_isFormatUSB = False;
		return SYSE_FormatUSBResult_DevChkErr;
	}

	// ◆マウント確認.
	// ※マウント確認不要.
	//   マウントされていたら解除のためマウント確認を行うが、マウントされていないデバイスもあり、その場合はエラーではないため.

printf("##umount2()\n");
	// ◆アンマウント.
	for (i=0; i<static_cast<int>(sizeof(p_mountNameTbl)/sizeof(char*)); i++) {
		bl_umountErr = False;
		ret = umount2(p_mountNameTbl[i], MNT_FORCE);
		if ( -1 == ret ) {
			si_errno = errno;
			switch(si_errno) {
				case EBUSY:				// 使用中 (busy) のため、 target をアンマウントできなかった.
					bl_umountErr = True;
					// 所定回数Busyでなくなるまで繰り返す.
					for (j=0; j<UMOUNT_EXE_NUM; j++) {
						SYSF_taskDelay(static_cast<int>(SYSF_sysClkRateGet() * 200 / 1000));	// 遅延(0.2s).
						ret = umount2(p_mountNameTbl[i], MNT_FORCE);
						si_errno = errno;
						if ( (0 == ret) && (EBUSY != si_errno) ) {		// Busyでない.
							bl_umountErr = False;
							break;
						}
					}
					break;
				case ENOENT:			// パス名が空である。もしくは指定されたパスが存在しない.
				case EINVAL:			// target がマウントポイントではない。.
					// マウントされていないデバイスの場合があるのでエラーとしない.
					si_errno = 0;	// クリア.
					break;
				default :
					bl_umountErr = True;
					break;
			}
		}
		if (True == bl_umountErr) {		// エラーあり.
			printf("SYSC_formatUSBBulkDevice() SYSE_FormatUSBResult_UmountErr  umount2() ERR! errno=%d\n", si_errno);
			bl_isFormatUSB = False;
			return SYSE_FormatUSBResult_UmountErr;
		}
	}

printf("##fdisk\n");
printf("##execprog_in()\n");
	// ◆パーティション削除.
	// ◆パーティション作成.
	// 開始セクタ.
	startSec = DEF_PARTITION_INFO + 1;	// HDD4kbyteセクタ対応として+1する.
	// 終了セクタ.
	memset( cmd, 0, sizeof(cmd) );
	memset( sbuf, 0, sizeof(sbuf) );
	// fdiskを実行し総バイト数を取得する.
	snprintf( cmd, sizeof(cmd), FMT_PART_FDISK1, scBULK_DEVICE_NAME_DEV1,scBULK_DEVICE_NAME_DEV1 );	// 2020/8/31 OP_BTS-26686対応.
	ret = execprog_in( cmd, sbuf, sizeof(sbuf) );
	if( -1 == ret ){
		si_errno = errno;
		printf("SYSC_formatUSBBulkDevice() SYSE_FormatUSBResult_PartErr 1  execprog_in() ERR! errno=%d\n", si_errno);
		bl_isFormatUSB = False;
		return SYSE_FormatUSBResult_PartErr;
	}
	ptr = sbuf;
	errno = 0;
	totalBytes = strtoull(ptr, &endptr, 0);
	if ((0 == totalBytes) || (0 != errno)) {		// 変換不可能な場合、またはエラーの場合.
		si_errno = errno;
		printf("SYSC_formatUSBBulkDevice() SYSE_FormatUSBResult_PartErr 2  strtoll() ERR! errno=%d\n", si_errno);
		if (NULL != endptr) {
			printf("endptr = %s\n", endptr);
		}
		bl_isFormatUSB = False;
		return SYSE_FormatUSBResult_PartErr;
	}
	// パーティションサイズは500GBで切り直す。(HDDの物理的なサイズが500GB以上でも。).
	// HDD容量が500GB以下の場合は、最大容量でパーティションを切る。.
	if ( HDD_SIZE_500GB <= totalBytes ) {	// 総バイト数が500GBのバイト数以上.
		totalBytes = HDD_SIZE_500GB;
	}
	memset( cmd, 0, sizeof(cmd) );
	memset( sbuf, 0, sizeof(sbuf) );
	// fdiskを実行しセクタサイズを取得する.
	snprintf( cmd, sizeof(cmd), FMT_PART_FDISK2, scBULK_DEVICE_NAME_DEV1 );
	ret = execprog_in( cmd, sbuf, sizeof(sbuf) );
	if( -1 == ret ){
		si_errno = errno;
		printf("SYSC_formatUSBBulkDevice() SYSE_FormatUSBResult_PartErr 3  execprog_in() ERR! errno=%d\n", si_errno);
		bl_isFormatUSB = False;
		return SYSE_FormatUSBResult_PartErr;
	}
	ptr = sbuf;
	errno = 0;
	sectorSize = strtoull(ptr, &endptr, 0);
	if ((0 == sectorSize) || (0 != errno)) {		// 変換不可能な場合、またはエラーの場合.
		si_errno = errno;
		printf("SYSC_formatUSBBulkDevice() SYSE_FormatUSBResult_PartErr 4  strtoll() ERR! errno=%d\n", si_errno);
		if (NULL != endptr) {
			printf("endptr = %s\n", endptr);
		}
		bl_isFormatUSB = False;
		return SYSE_FormatUSBResult_PartErr;
	}
	endSec = totalBytes / sectorSize;	// 総セクタ算出.
	endSec = endSec - startSec;			// 総セクタ数から開始セクタまでのセクタ数を減算しておく.

	memset( sbuf, 0, sizeof(sbuf) );
	strcpy( sbuf, "d\n" );	// d：区画を削除.
	strcat( sbuf, "1\n" );	// 1：パーティション指定、基本(プライマリ)パーティションは最大4つなので、強制的に4つ削除しておく.
	strcat( sbuf, "d\n" );
	strcat( sbuf, "2\n" );
	strcat( sbuf, "d\n" );
	strcat( sbuf, "3\n" );
	strcat( sbuf, "d\n" );
	strcat( sbuf, "4\n" );
	strcat( sbuf, "n\n" );	// n：新しい区画を作成.
	strcat( sbuf, "p\n" );	// p：基本(プライマリ)パーティション.
	strcat( sbuf, "1\n" );	// 1：パーティション指定.
	memset( val, 0, sizeof(val) );
	sprintf( val, "%llu\n", startSec );
	strcat( sbuf, val );	// 開始セクタ：パーティションの始点.
	memset( val, 0, sizeof(val) );
	sprintf( val, "%llu\n", endSec );
	strcat( sbuf, val );	// 終了セクタ：パーティションの終点.
	strcat( sbuf, "w\n" );	// w：テーブルを書き込んで終了.

printf("##execprog_out()\n");
	memset( cmd, 0, sizeof(cmd) );
	snprintf( cmd, sizeof(cmd), FMT_PART_FDISK3, scBULK_DEVICE_NAME_DEV1 );
	ret = execprog_out( cmd, sbuf, strlen(sbuf) );
	if( -1 == ret ){
		si_errno = errno;
		printf("SYSC_formatUSBBulkDevice() SYSE_FormatUSBResult_PartErr 5  execprog_out() ERR! errno=%d\n", si_errno);
		bl_isFormatUSB = False;
		return SYSE_FormatUSBResult_PartErr;
	}
	/*
	 * fdisk実行後、新しいパーティションレイアウトが認識されてHDDのデバイスファイルが
	 * 生成されるまでタイムラグがあるためディレイする。
	 */
	sleep( 2 );

printf("##formatFS()\n");
	// ◆フォーマット.
	ret = formatFS( scBULK_DEVICE_NAME_DEV1_PART1 );
	if( ret == -1 ){
		si_errno = errno;
		printf("SYSC_formatUSBBulkDevice() SYSE_FormatUSBResult_FormatErr  formatFS() ERR! errno=%d\n", si_errno);
		bl_isFormatUSB = False;
		return SYSE_FormatUSBResult_FormatErr;
	}

printf("##mount()\n");
	// ◆マウント.
	ret = mount( scBULK_DEVICE_NAME_DEV1_PART1,		// ファイルシステム(デバイス名).
				 scBULK_MOUNT_NAME_DEV1_PART1,		// マウントポイント.
				 "ext3",							// ファイルシステムタイプ.
				 MS_NOATIME,						// マジックナンバーとマウントフラグ.
				 EXT3_MOUNT_OPTIONS );				// オプション.
	if( ret == -1 ){
		si_errno = errno;
		printf("SYSC_formatUSBBulkDevice() SYSE_FormatUSBResult_MountErr  mount() ERR! errno=%d\n", si_errno);
		bl_isFormatUSB = False;
		return SYSE_FormatUSBResult_MountErr;
	}

printf("bl_isFormatUSB = False\n");
	bl_isFormatUSB = False;		// ext3フォーマット実行中解除.

	APIC_ChangeDevice *pc_ChangeDevice = APIC_ChangeDevice::newInstance();
	// デバイス変更通知のメッセージ(切断通知)を送信.
	memset(sc_devName, 0, sizeof(sc_devName));
	snprintf( sc_devName, sizeof(sc_devName), "%s-remove", scBULK_DEVICE_NAME_DEV1_PART1_NAME );	// 2020/8/31 OP_BTS-26686対応.
printf("SYSF_msgQSend()  %s\n", sc_devName);
	if ( NULL != pc_ChangeDevice ) {
		SYSF_msgQSend( pc_ChangeDevice->getMsgQID(), const_cast<char*>(reinterpret_cast<const char*>(&sc_devName)), sizeof(sc_devName), WAIT_FOREVER, MSG_PRI_NORMAL );
	}
	/*
	 * デバイスの変更通知によるデバイス情報設定処理が完了するまでタイムラグがあるためディレイする。.
	 */
printf("sleep() start\n");
	i = 0;
	while (true){
		SYSF_taskDelay( static_cast<int>( SYSF_sysClkRateGet() * 1000 / 1000 ) );	//1sec遅延.
		if( APIC_UsbMemDeviceInfo::newInstance()->getStatus() != TYPD_UsbMemStatusConnect ){
			printf("Umount OK\n");
			break;
		}
		if( i++ >= BLKD_FORMAT_USB_DELAY ){
			//アンマウントエラーとしてタイムアウト処理.
			si_errno = errno;
			printf("SYSC_formatUSBBulkDevice() SYSE_FormatUSBResult_UmountErr  Timeout ERR! errno=%d\n", si_errno);
			return SYSE_FormatUSBResult_UmountErr;
		}
	}
	
printf("sleep() end\n");
	// デバイス変更通知のメッセージ(接続通知)を送信.
	memset(sc_devName, 0, sizeof(sc_devName));
	snprintf( sc_devName, sizeof(sc_devName), "%s-add", scBULK_DEVICE_NAME_DEV1_PART1_NAME );	// 2020/8/31 OP_BTS-26686対応.
printf("SYSF_msgQSend()  %s\n", sc_devName);
	if ( NULL != pc_ChangeDevice ) {
		SYSF_msgQSend( pc_ChangeDevice->getMsgQID(), const_cast<char*>(reinterpret_cast<const char*>(&sc_devName)), sizeof(sc_devName), WAIT_FOREVER, MSG_PRI_NORMAL );
	}
	/*
	 * デバイスの変更通知によるデバイス情報設定処理が完了するまでタイムラグがあるためディレイする。.
	 */
printf("sleep() start\n");
	i = 0;
	while (true){
		SYSF_taskDelay( static_cast<int>( SYSF_sysClkRateGet() * 1000 / 1000 ) );	//1sec遅延.
		if( APIC_UsbMemDeviceInfo::newInstance()->getStatus() == TYPD_UsbMemStatusConnect ){
			printf("Mount OK\n");
			break;
		}
		if( i++ >= BLKD_FORMAT_USB_DELAY ){
			//マウントエラーとしてタイムアウト処理.
			si_errno = errno;
			printf("SYSC_formatUSBBulkDevice() SYSE_FormatUSBResult_MountErr  Timeout ERR! errno=%d\n", si_errno);
			return SYSE_FormatUSBResult_MountErr;
		}
	}
	
printf("sleep() end\n");

printf("SYSE_FormatUSBResult_OK\n");
	return SYSE_FormatUSBResult_OK;

#else // #ifdef TORNADO
	si_errno = 0;
	return SYSE_FormatUSBResult_OK;
#endif // #ifdef TORNADO
}



// ---------------------------------------------------------------------------------
// 関数名	：SYSC_reqFormatUSBBulkDevice
// ---------------------------------------------------------------------------------
// 概要 	：USB HDDをext3でフォーマットする(コールバック形式).
// 引数		：void (*ansFormatUSBBulkDevice)( SYSE_FormatUSBResult e_Result, int si_errno ).
//				ext3フォーマット完了通知用コールバック関数ポインタ.
// 戻り値	：Bool  ext3フォーマット要求 受付結果.
//					True：受付OK / False：受付NG.
// 備考 	：
//				・USB HDDがマウントされていた場合は自動的にアンマウントを行なってから.
//				  フォーマットを行ないます。.
//				・フォーマット後は自動的にマウントします。.
//				・USB HDDが使用中にこの関数を実行するとアンマウントでエラーになるので注意すること。.
//				・パーティションサイズについて.
//					パーティションサイズは500GBで切り直す。(HDDの物理的なサイズが500GB以上でも。).
//					HDD容量が500GB以下の場合は、最大容量でパーティションを切る。.
//				・戻り値について.
//					戻り値：「False：受付NG」が返るパターンは以下となり、コールバック関数は返さない。.
//					・コールバック関数がNULLの場合.
//					・ext3フォーマット用タスクが生成できなかった場合.
//				・コールバック関数について.
//					コールバック関数にて、下記を通知します。.
//					・フォーマット実行結果.
//					typedef int SYSE_FormatUSBResult;		フォーマット実行結果.
//					SYSE_FormatUSBResult_OK				フォーマット成功.
//					SYSE_FormatUSBResult_DevChkErr		デバイス検出エラー.
//					SYSE_FormatUSBResult_UmountErr		アンマウントエラー.
//					SYSE_FormatUSBResult_PartErr		パーティションエラー.
//					SYSE_FormatUSBResult_FormatErr		フォーマットエラー.
//					SYSE_FormatUSBResult_MountErr		マウントエラー.
//					・エラー番号.
//				・エラー詳細について.
//					フォーマット実行結果では、フォーマットシーケンスの手順に応じた結果を返す。.
//					フォーマット実行結果がエラーであった場合、どのような要因でエラーとなったのか.
//					エラー詳細を確認する際には、OSによってグローバル変数の「errno」にエラー詳細が.
//					設定されているので、この変数を参照すること。.
//					フォーマット実行結果が「フォーマット成功」の場合には確認不要。値は0。.
// 作成者	：Harada
// レビュー ：レビュー実施日と名前を記述.
// 変更履歴 ：変更日、変更者、変更内容（変更理由）を記述.
//			  2017/04/19 ADS長谷川 armv8ではスタックが足りないので3KBから24KBへ増やす.
// ---------------------------------------------------------------------------------
void SYSC_execFormatUSBBulkDevice(intptr_t si_pfunc)
{
	void (*pfunc)( SYSE_FormatUSBResult, int ) = NULL;

	// フォーマット実行.
	SYSE_FormatUSBResult e_Result = SYSC_formatUSBBulkDevice();

	// コールバック関数コール.
	pfunc = reinterpret_cast<void(*)(SYSE_FormatUSBResult,int)>(si_pfunc);
	pfunc(e_Result, si_errno);
}
Bool SYSC_reqFormatUSBBulkDevice( void (*ansFormatUSBBulkDevice)( SYSE_FormatUSBResult e_Result, int si_errno ) )
{
	THR_ID si_taskId = 0;
	if (NULL == ansFormatUSBBulkDevice) {		// コールバック関数がNULLの場合.
		printf("SYSC_reqFormatUSBBulkDevice() CallBask is NULL ERR!\n");
		return False;
	}
	// フォーマット実行タスク生成.
	si_taskId = SYSF_taskSpawn( "TASK_FormatUSB", TYPD_PRIONO_SYS__TASK_FormatUSB, VX_FP_TASK, 1024*24, reinterpret_cast<FUNCPTR>(SYSC_execFormatUSBBulkDevice), reinterpret_cast<intptr_t>(ansFormatUSBBulkDevice), 0, 0, 0, 0, 0, 0, 0, 0, 0);
	if ( ERROR == si_taskId ) {		// タスク生成失敗.
		printf("SYSC_reqFormatUSBBulkDevice() SYSF_taskSpawn() ERR!\n");
		return False;
	}
	return True;
}

// デバッグ関数 ----------------------------------------------.
void testDummyFormatCallBack(SYSE_FormatUSBResult e_Result, int si_errnoParam)
{
	printf("testDummyFormatCallBack()\n");
	printf("e_Result = %d, si_errno = %d\n", e_Result, si_errnoParam);
}
void testDummyFormat(void)
{
	printf("testDummyFormat()\n");
	Bool ret = False;
	printf("call SYSC_reqFormatUSBBulkDevice()\n");
	ret = SYSC_reqFormatUSBBulkDevice( testDummyFormatCallBack );
	printf("ret = %d\n", ret);
}
// デバッグ関数 ----------------------------------------------.
// ---------------------------------------------------------------------------------
// 関数名	：SYSC_isFormatUSB
// ---------------------------------------------------------------------------------
// 概要 	：ext3フォーマット実行状態判断.
// 引数		：なし.
// 戻り値	：Boolt ext3フォーマット実行状態.
//				True：実行中 / False：実行中でない.
// 備考 	：
// 作成者	：Harada
// レビュー ：レビュー実施日と名前を記述.
// 変更履歴 ：変更日、変更者、変更内容（変更理由）を記述.
// ---------------------------------------------------------------------------------
Bool SYSC_isFormatUSB(void)
{
	return bl_isFormatUSB;
}

// ---------------------------------------------------------------------------------
// 関数名	：SYSC_GetIdFromUSBDevice
// ---------------------------------------------------------------------------------
// 概要 	：USBデバイスID取得
//				in	：cp_devicePath	USBデバイスsysfsパス
//				out ：pus_idVendor	Vendor IDの格納先
//					  pus_idProduct	Product IDの格納先
// 戻り値	：OK	IDを取得できた
//			  ERROR	IDを取得出来なかった
// 備考 	：
// 作成者	：河村
// レビュー ：レビュー実施日と名前を記述
// 変更履歴 ：2012/05/13 河村　初版
// ---------------------------------------------------------------------------------
STATUS SYSC_GetIdFromUSBDevice( const char* cp_devicePath, ushort *pus_idVendor, ushort *pus_idProduct )
{
#ifdef TORNADO
	int fd = 0;
	char path[SYSC_usbPrintFileNameMax + 1] = {};
	char idString[5] = {};
	ssize_t size = 0;

	if( cp_devicePath == NULL ) {
		return ERROR;
	}
	if( strlen( cp_devicePath ) + 10 > SYSC_usbPrintFileNameMax ) { /*10は"/idProduct"の長さ*/
		return ERROR; /*指定パス長過ぎ*/
	}

	/*idVendor取得*/
	strcpy( path, cp_devicePath );
	if( path[strlen( path ) - 1] != '/' ) {
		strcat( path, "/" );
	}
	strcat( path, "idVendor" );
	fd = SYSF_open( path, O_RDONLY, 0 );
	if( fd == -1 ) {
		return ERROR; /*idVendorはオープン出来なかった*/
	}
	if( NULL!=pus_idVendor ) {
		size = SYSF_read( fd, idString, 4 );
		if( size == -1 ) {
			SYSF_close( fd );
			return ERROR; /*idVendorの読み出しが出来なかった*/
		}
		idString[size] = '\0';
		*pus_idVendor = static_cast<ushort>(strtoul( idString, NULL, 16 ));
	}
	SYSF_fsync(fd);
	SYSF_close( fd );

	/*idProduct取得*/
	strcpy( path, cp_devicePath );
	if( path[strlen( path ) - 1] != '/' ) {
		strcat( path, "/" );
	}
	strcat( path, "idProduct" );
	fd = SYSF_open( path, O_RDONLY, 0 );
	if( fd == -1 ) {
		return ERROR; /*idProductはオープン出来なかった*/
	}
	if( NULL!=pus_idProduct ) {
		size = SYSF_read( fd, idString, 4 );
		if( size == -1 ) {
			SYSF_close( fd );
			return ERROR; /*idProductの読み出しが出来なかった*/
		}
		idString[size] = '\0';
		*pus_idProduct = static_cast<ushort>(strtoul( idString, NULL, 16 ));
	}
	SYSF_fsync(fd);
	SYSF_close( fd );

	return OK;
#else
	return ERROR;
#endif
}

/*SYSC_GetIdFromUSBDeviceテスト用*/
#ifdef TORNADO
void test_GetIdFromUSBDevice( void )
{
	const char *devicePaths[] = { DEF_USB_DEVICE_1, DEF_USB_DEVICE_1_1, DEF_USB_DEVICE_1_2, DEF_USB_DEVICE_1_3,
			DEF_USB_DEVICE_1_4, DEF_USB_DEVICE_2, DEF_USB_DEVICE_2_1, DEF_USB_DEVICE_2_2, DEF_USB_DEVICE_2_3,
			DEF_USB_DEVICE_2_4, DEF_USB_DEVICE_1_4_1, DEF_USB_DEVICE_1_4_2, DEF_USB_DEVICE_1_4_3, DEF_USB_DEVICE_1_4_4,
			NULL, };
	ushort idVendor = 0;
	ushort idProduct = 0;
	int i = 0;
	STATUS status = 0;

	while( devicePaths[i] != NULL ) {
		status = SYSC_GetIdFromUSBDevice( devicePaths[i], &idVendor, &idProduct );
		if( status == OK ) {
			printf( "%40s : ID %04x:%04x\n", devicePaths[i], idVendor, idProduct );
		} else {
			printf( "%40s : not exist device\n", devicePaths[i] );
		}
		i++;
	}
}
#endif

// ---------------------------------------------------------------------------------
// 関数名	：SYSC_UsbAccess
// ---------------------------------------------------------------------------------
// 概要 	：USBデバイスの接続確認をする。.
//				in ：char*		psc_deviceName		デバイス名.
// 戻り値	： 0 : OK.
//			   Not 0 : Error.
// 備考 	：.
// 作成者	：鳥居.
// レビュー ：.
// 変更履歴 ：.
// ---------------------------------------------------------------------------------
STATUS SYSC_UsbAccess ( char* psc_deviceName )
{
	STATUS si_Result = OK;

#ifdef LINUX
	si_Result = access( static_cast<char*>(psc_deviceName), F_OK);
#endif	// LINUX.

	return(si_Result);
}

// ---------------------------------------------------------------------------------
// 関数名	：SYSC_SyncForUSB
// ---------------------------------------------------------------------------------
// 概要 	：USBデバイスに強制書込みする。.
// 引数		：なし.
// 戻り値	：なし.
// 備考 	：.
// 作成者	：勝又.
// レビュー ：.
// 変更履歴 ：.
// ---------------------------------------------------------------------------------
void SYSC_SyncForUSB()
{
	int fd_sync	= OK;
	int fd_dev	= OK;
	int fd_dir	= OK;

	// 2020/8/31 OP_BTS-26686対応 start.
	// バルクデバイス名生成.
	SYSC_MakeBulkDeviceName();
	// 2020/8/31 OP_BTS-26686対応 end.
	
	// USBデバイスに強制書込みする(SYSC_SyncForUSBを同期的に行う).
	if((fd_dir = SYSF_open(scBULK_MOUNT_NAME_DEV1_PART1, O_DIRECTORY, 0666)) != ERROR){
		syncfs(fd_dir);
		SYSF_close(fd_dir);
	} else {
		SYSF_logMsg("SYSC_SyncForUSB: \"%s\" cannot open.\n", scBULK_MOUNT_NAME_DEV1_PART1, 0, 0, 0, 0, 0);
		sync();
	}

	if ((fd_sync = SYSF_open("/proc/sys/vm/drop_caches", O_RDWR | O_CREAT | O_BINARY, 0666)) != ERROR) {
		write(fd_sync, "3", sizeof(char));		// "echo 3 > /proc/sys/vm/drop_caches".
		syncfs(fd_sync);
		SYSF_close(fd_sync);
	} else {
		SYSF_logMsg("SYSC_SyncForUSB: \"/proc/sys/vm/drop_caches\" cannot open.\n", 0, 0, 0, 0, 0, 0);
	}
	
	if((fd_dev = SYSF_open(scBULK_DEVICE_NAME_DEV1, O_RDONLY|O_NONBLOCK, 0444)) != ERROR){
		SYSF_ioctl(fd_dev, BLKFLSBUF, 0);			// "hdparam -f /dev/sdc".
		SYSF_close(fd_dev);
	} else {
		SYSF_logMsg("SYSC_SyncForUSB: \"%s\" cannot open.\n", scBULK_DEVICE_NAME_DEV1, 0, 0, 0, 0, 0);
	}
}

// ---------------------------------------------------------------------------------
// 関数名	：SYSC_FsyncEx.
// ---------------------------------------------------------------------------------
// 概要 	：実行環境に合わせた fsync を行なう.
//            IT5 Emu3 以降なら、fflush + syncfsを行う。PreEmu、Sim なら fflush + fsync を行う.
//            (PreEmu、Sim では syncfs が無いため).
// 引数		： FILE*	fp	ファイルポインタ.
// 戻り値	：なし.
// 備考 	：Poseidon CQ510369 Sync対応用関数 他の用途で使用しないこと.
// 作成者	：佐々木.
// レビュー ：.
// 変更履歴 ：.
// ---------------------------------------------------------------------------------
void SYSC_FsyncEx(
	FILE*			fp)
{
	int fd;
	if ( NULL != fp ){
		// Emu3（PreEmu、Sim以外）.
		SYSF_Fflush(fp);
		fd = fileno(fp);
		if (-1 != fd){
			syncfs( fd );
		}
	}
}

#ifdef LINUX
// ---------------------------------------------------------------------------------.
// 関数名	：SYSC_GetUsbDescriptorValue.
// ---------------------------------------------------------------------------------.
// 概要 	：USBディスクリプタ値取得
//				in  : psc_DevicePath	USBデバイス名(sysfsパスのディレクトリ名)
//				in  : psc_Descriptor	ディスクリプタ名.
//				in  : base				基数.
//				out : pul_value			値.
// 戻り値	：OK	取得成功.
//			  ERROR	取得失敗.
// 備考 	：
// 作成者	：ishiga.
// 変更履歴 ：2016/08/29 ishiga [IT5_4.0][制御構成変更No.45] 新規作成.
// ---------------------------------------------------------------------------------
STATUS SYSF_GetUsbDescriptorValue(const char *psc_DevicePath, const char *psc_Descriptor, int base, uint32_t* pul_value)
{
	char sc_buf[12];		// 32ビット整数文字列を格納.
	char sc_path[PATH_MAX];
	int fd = -1;
	int rsize = -1;
	STATUS si_result = ERROR;

	if ( NULL == pul_value ) {
		return ERROR;
	}

	// sysfsパス.
	snprintf( sc_path, PATH_MAX, "%s%s/%s", DEF_SYSFS_USB, psc_DevicePath, psc_Descriptor );
	sc_path[PATH_MAX - 1] = '\0';

	// オープン.
	fd = SYSF_open( sc_path, O_RDONLY, 0666 );
	if ( 0 <= fd ) {
		memset( sc_buf, 0, sizeof(sc_buf) );
		// 読み出し.
		rsize = static_cast<int>(SYSF_read( fd, sc_buf, sizeof(sc_buf) ));
		// クローズ.
		SYSF_close(fd);

		// 値に変換.
		if ( 0 <= rsize ) {
			sc_buf[sizeof(sc_buf) - 1] = '\0';
			*pul_value = static_cast<uint32_t>(strtoul(sc_buf, NULL, base));
			si_result = OK;
		} else {
			printf("SYSC_GetUsbDescriptorValue: \"%s\" read error.\n", sc_path);
		}
	} else {
		printf("SYSC_GetUsbDescriptorValue: \"%s\" cannot open.\n", sc_path);
	}

	return si_result;
}
// 2020/8/31 OP_BTS-26686対応 start.
void SYSC_MakeBulkDeviceName( void )
{
	const char BULK_DEVICE_NAME_DEV1[] =			"/dev/sdc";		/* Device name device1 */
	const char BULK_DEVICE_NAME_DEV1_PART1[] =		"/dev/sdc1";	/* Device name device1 partition1 */
	const char BULK_MOUNT_NAME_DEV1[] =				"/bd/sdc";		/* Mount name device1 */
	const char BULK_MOUNT_NAME_DEV1_PART1[] =		"/bd/sdc1";		/* Mount name device1 partition1 */

	char **names,*tmp;
	char sc_DeviceName_temp[64] = {};
	names = &tmp; *names = static_cast<char*>(sc_DeviceName_temp);
	char *sc_splitStr[10] = {};
	ushort num=0;
	char num1=0;
	char* pscTmp;
	
	// /dev/sdx.
	memset( scBULK_DEVICE_NAME_DEV1, 0, sizeof(scBULK_DEVICE_NAME_DEV1) );

	// /dev/sdx1.
	memset( scBULK_DEVICE_NAME_DEV1_PART1, 0, sizeof(scBULK_DEVICE_NAME_DEV1_PART1) );
	
	// /bd/sdx.
	memset( scBULK_MOUNT_NAME_DEV1, 0, sizeof(scBULK_MOUNT_NAME_DEV1) );
	
	// /bd/sdx1.
	memset( scBULK_MOUNT_NAME_DEV1_PART1, 0, sizeof(scBULK_MOUNT_NAME_DEV1_PART1) );

	num = SYSC_GetBulkDeviceNames( names, 1 );

	// 2020/9/7 OP_BTS-27172対応 start.
	if(0 == num)
	{
		// /dev/sdx.
		memcpy( reinterpret_cast<void*>(scBULK_DEVICE_NAME_DEV1), ((BULK_DEVICE_NAME_DEV1)), sizeof(BULK_DEVICE_NAME_DEV1) );

		// /dev/sdx1.
		memcpy( reinterpret_cast<void*>(scBULK_DEVICE_NAME_DEV1_PART1), (BULK_DEVICE_NAME_DEV1_PART1), sizeof(BULK_DEVICE_NAME_DEV1_PART1) );
		
		// /bd/sdx.
		memcpy( reinterpret_cast<void*>(scBULK_MOUNT_NAME_DEV1), (BULK_MOUNT_NAME_DEV1), sizeof(BULK_MOUNT_NAME_DEV1) );
		
		// /bd/sdx1.
		memcpy( reinterpret_cast<void*>(scBULK_MOUNT_NAME_DEV1_PART1), (BULK_MOUNT_NAME_DEV1_PART1), sizeof(BULK_MOUNT_NAME_DEV1_PART1) );

		SYSF_logMsg( "SYSC_MakeBulkDeviceName() num = 0!!\n", 0, 0, 0, 0, 0, 0 );
	}
	else
	{
		num1 = SYSC_MakeBulkDeviceNameSplit( sc_DeviceName_temp, "/" , sc_splitStr );

		// sdx1.
		memcpy( reinterpret_cast<void*>(scBULK_DEVICE_NAME_DEV1_PART1_NAME), reinterpret_cast<void*>(sc_splitStr[1]), (sizeof(scBULK_DEVICE_NAME_DEV1_PART1_NAME))-1 );		// 2020/11/19 静的解析対応.
		
		// sdx.
		memcpy( reinterpret_cast<void*>(scBULK_DEVICE_NAME_DEV1_NAME), reinterpret_cast<void*>(sc_splitStr[1]), (sizeof(scBULK_DEVICE_NAME_DEV1_NAME))-1 );					// 2020/11/19 静的解析対応.
		if ( NULL != (pscTmp = strstr( scBULK_DEVICE_NAME_DEV1_NAME, "1" )))
		{
			*pscTmp = NULL;	// 1が付加されているならNULLにする. 
		}
		
		// /dev/sdx.
		snprintf( scBULK_DEVICE_NAME_DEV1, sizeof(scBULK_DEVICE_NAME_DEV1), "/dev/%s", scBULK_DEVICE_NAME_DEV1_NAME );

		// /dev/sdx1.
		snprintf( scBULK_DEVICE_NAME_DEV1_PART1, sizeof(scBULK_DEVICE_NAME_DEV1_PART1), "/dev/%s", scBULK_DEVICE_NAME_DEV1_PART1_NAME );
		
		// /bd/sdx.
		snprintf( scBULK_MOUNT_NAME_DEV1, sizeof(scBULK_MOUNT_NAME_DEV1), "/bd/%s", scBULK_DEVICE_NAME_DEV1_NAME );
		
		// /bd/sdx1.
		snprintf( scBULK_MOUNT_NAME_DEV1_PART1, sizeof(scBULK_MOUNT_NAME_DEV1), "/bd/%s", scBULK_DEVICE_NAME_DEV1_PART1_NAME );
	}
}
char SYSC_MakeBulkDeviceNameSplit( char *sc_str, const char *sc_delim, char *sc_list[] )
{
	char	*sc_tk = NULL;
	uchar	 uc_cnt = 0;
	char* saveptr;

	sc_tk = strtok_r( sc_str, sc_delim ,&saveptr );
	while( sc_tk != NULL && uc_cnt < 10 ) {
		sc_list[uc_cnt++] = sc_tk;
		sc_tk = strtok_r( NULL, sc_delim ,&saveptr );
	}
	return uc_cnt;
}
// 2020/8/31 OP_BTS-26686対応 end.

/*================================================================================================
［機能］	USBデバイスがライトプロテクトされているか確認
--------------------------------------------------------------------------------------------------
［引数］なし
［戻値］Bool True:USB書き込み禁止 False：USB書き込み許可
--------------------------------------------------------------------------------------------------
［履歴］2022/8/25 TAS000000012453対応 新規作成
================================================================================================*/
Bool isUSBWriteProtect( char *psc_devName )
{
	int si_Ret = 0;
	int si_Fd = 0;
	int si_USBReadonly = 0;		//USB ライトプロテクト確認 0以外でReadOnly.

	si_Fd = SYSF_open(psc_devName, O_RDONLY, 0644);
	if (si_Fd >= 0) {
		//USBデバイスからReadOnly情報取得.
		si_Ret = SYSF_ioctl(si_Fd, BLKROGET, (intptr_t)&si_USBReadonly);
		if( si_Ret!=0 ) {
			si_USBReadonly = 0;	//エラーなら書き込み許可状態とする.
			printf( "\n(%010u) (sys:PDLIF)[%s][%s] ERROR IOCTL(BLKROGET) DEV=%s\n",SYSF_tickGet(),__FILE__,__FUNCTION__,psc_devName );
		}
		SYSF_close(si_Fd);
	} else {
		printf( "\n(%010u) (sys:PDLIF)[%s][%s] OPEN ERROR DEV=%s\n",SYSF_tickGet(),__FILE__,__FUNCTION__,psc_devName );
	}
	//状態確認.
	if( si_USBReadonly ) {
		printf( "\n(%010u) (sys:PDLIF)[%s][%s] %s READ ONLY\n",SYSF_tickGet(),__FILE__,__FUNCTION__,psc_devName );
	} else {
		printf( "\n(%010u) (sys:PDLIF)[%s][%s] %s Write Enable\n",SYSF_tickGet(),__FILE__,__FUNCTION__,psc_devName );
	}
	return (si_USBReadonly ? True : False );
}

#endif // #ifdef LINUX.

