//
// HW_interim.cpp
// ファイル名：HW_interim.cpp
//
// 概要：HW対応関数定義 
//

// JD.KIM 
#include <termios.h>

#include <system/sys/OsWrapper/SYSC_OSWrapper.h>
#include <HW_interim.h>

/* Y.Okamoto 2009/08/11 MIO SimWinビルド対応 start */
#ifdef TORNADO
#include <nicfum/mioApi.h>
#else
#include <nicfum/mioApi_sim.h>
#endif
/* Y.Okamoto 2009/08/11 MIO SimWinビルド対応 end */
#include <stdarg.h>
#include <algorithm>

#include <system/thr/thrInclude/ThrCmemDrvrIf.h> // drvrif_cmemapi_cache_invalidate(), drvrif_cmemapi_cache_flush()
#include <system/doc/docg/DOCC_Leaf.h>
#include <system/thr/thrInclude/THRC_Port.h>
#include <system/thr/thrInclude/THRC_PortInstance.h>
#include "system/thr/thrInclude/ThrCommonDrvrIf.h"    /* drvrif_getMainMemoryBankSize honda */
#include "system/thr/thrInclude/THRC_BSPif.h"    /* THRE_MEMORY_BANK honda */

//2013/03/08 【MP-BUS-01】日立JTE 国吉 BootupScreenバッファ非常駐化対応.
#include "system/sys/APIC_OptionSetting.h"

#ifdef DEF_PANELDRV
///< 暫定:パネルドライバ対応 
#include "panelcomdrv_info.h"
#include  "system/thr/thrInclude/THRC_Vxworks.h"
#define DEVICENAME "/dev/outpanelcom"
#if 0	// デバック用 
#define __PANELDRVDEBUG 1
#endif
#endif // DEF_PANELDRV
#include "system/sys/APIC_AndroidPanel.h"// 2013/01/19 k.komaba [操作性改善FUMv4]Single⇔Dual表示切換対応.

#include  "system/thr/AndroidCtrl/THRC_AndroidControl.h"
#include  "system/thr/AndroidCtrl/THRC_AndroidControlIf.h"

#define MODULE_NAME "BSP"
#define LOG_LEVEL "DEBUG"
#define BSP_LOG(format, ...) \
    printf("[%s][%s][%s:%d][%s] " format "\n", MODULE_NAME, LOG_LEVEL, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)

//2017/01/27 未使用フォルダ・ファイルの論理削除.
//#include "lib/pdflib/CNVC_KMPdfLib.h"
/* 2018/05/22 ft_kondo SimLin不要コンパイルSW撲滅対応 */
#if defined(FUM_LINUX)	// シミュレータで有効.
#include "../../simulator/SimShared/include/SIMC_SoundObserverIf.h"
#endif	// defined(FUM_LINUX).
#include "system/thr/thrcom/THRC_PowerManagerDevice.h"
#include "system/thr/thrcom/ThrCommonDrvrIf.h"
// 2022/10/31 生産RINNE自動画像調整 Sky start.
#include "system/adj/APIC_Adjust.h"
// 2022/10/31 生産RINNE自動画像調整 Sky end.
// JD.KIM HW対応 
//関数--------------------------------------------------------------------------------------------------------------- 
//実体対応 
//

// JD.KIM HW対応 
// common\system\cnv\cnvc\thr\pchddtrans.cpp 
u_int dosFsDebug;

// JD.KIM HW対応 
// Tateishi 2007-4-5 
//size_t sdramSize = 512*1024*1024; //512000000;
size_t sdramSize;

// JD.KIM HW対応 
// common\system\cnv\cnvc\thr\thrcosimdummy.cpp 
MSG_Q_ID    NetMsgID, MioMsgID;
st_FlashManager	st_FRMT[14];		/* FlashROM */
// ↓↓↓ROM_DIV有効化対応 Start
//char *pt_NETG_FixedValueDefine_getKmScnCtrlName = NULL;

// JD.KIM HW対応 
// common\system\sys\sysg\appInit.c 
//char *pt_NETG_FixedValueDefine_getSysObjectID	= NULL;
//char *pt_NETG_FixedValueDefine_getSysDescr		= NULL;
// ↑↑↑ROM_DIV有効化対応 End

// Fax関連 
//char *pt_getPuiTextPartsPointer = NULL;
void getFax1PciMemBaseAdr(uchar hoge1, uint32_t* hoge2) {}
void getFax2PciMemBaseAdr(uchar hoge1, uint32_t* hoge2) {}

// ROM分割関連 appInit2.cpp
// ↓↓↓ROM_DIV有効化対応 Start
//Bool *pt_MediaConverter_getPdfInfo				= NULL;
//Bool *pt_MediaConverter_getPdfThumbnailData		= NULL;
//char *pt_mioOpenWebDAVClient		= NULL;	//ROM_DIV_3対応
//char *pt_mioCloseWebDAVClient		= NULL;	//ROM_DIV_3対応
//char *pt_IGC_TraceDsp					= NULL ;
//char *pt_OAP_gp_Traceout				= NULL;	// OpenAPI SysLog追加対応 
//char *pt_BMLF_SWAP_ShowLog = NULL;

// ROM分割関連 OCR appInit2.cpp
//char *pt_IDRMGetVersion			= NULL ;
//char *pt_IDRMSetUnicodeMode		= NULL ;
//char *pt_IDRSetProcMode			= NULL ;
//char *pt_IDRMInitialize			= NULL ;
//char *pt_IDRMReinitialize		= NULL ;
//char *pt_IDRMSetImageInfo		= NULL ;
//char *pt_IDRMSetImage			= NULL ;
//char *pt_IDRMSkewAuto			= NULL ;
//char *pt_IDRMSkewManual			= NULL ;
//char *pt_IDRMRotateAuto			= NULL ;
//char *pt_IDRMRotateManual		= NULL ;
//char *pt_IDRMRecognizeRegn		= NULL ;
//char *pt_IDRMRecognize			= NULL ;
//char *pt_IDRMFreeRecog			= NULL ;
//char *pt_IDRMSetDebugLog		= NULL ;
// ↑↑↑ROM_DIV有効化対応 End

// ↓↓↓ROM_DIV有効化対応 Start
//char *pt_mioProtocolReset       = NULL;	//ROM_DIV_3対応
// ↑↑↑ROM_DIV有効化対応 End

// JD.KIM HW対応 
extern "C"
{
	SEM_ID SEM_ID_Gloval		= 0;			// システム全体セマフォ 
}

// ↓↓↓ROM_DIV有効化対応 Start
// JD.KIM HW対応 
//int dbg_is_jscf_init_done;		// JScribe起動確認対応 
// ↑↑↑ROM_DIV有効化対応 End

// JD.KIM HW対応 
SYMTAB_ID	sysSymTbl;

// Tsukiyama HW対応
// Tornado2.2/target/config/Mosel/sysLib.h
char	sysBootHost[] = { "c:/" };	/* name of host from which system was booted */

ushort us_Debug_output_disp = 0;
uintptr_t ul_uart_pciMemAdr[6];
char uc_LineModeIPAddr[16];		// \common\nicfum\src\mio_init.c に定義されている 
char uc_LineModeMask[16];		// \common\nicfum\src\mio_init.c に定義されている 
char uc_LineModeGateway[16];	// \common\nicfum\src\mio_init.c に定義されている 

// ↓↓↓ROM_DIV有効化対応 Start
// CNVC_PdfFormatterColor2.cpp を追加したら問題が出たので 
//char *pt_HGBIOCreateObject				= NULL;
//char *pt_HGBIOSetFilePointer			= NULL;
//char *pt_HGBIOSetMemoryPointer			= NULL;
//char *pt_HGBIOSetMaxOutputSize			= NULL;
//char *pt_HGBIOSetParamInt				= NULL;
//char *pt_HGBIOGetParam					= NULL;
//char *pt_HGBIOReleaseObject				= NULL;
//char *pt_HGBIOVPrintf					= NULL;
//char *pt_HGBIOWrite						= NULL;
//char *pt_HGBIOTell						= NULL;
//char *pt_HGBIORead						= NULL;
//char *pt_HGBIOSeek						= NULL;
//char *pt_HGBIOEof						= NULL;
//char *pt_HGBIOError						= NULL;
//char *pt_HGUDAlloc						= NULL;
//char *pt_HGUDFree						= NULL;
//char *pt_HGUDAllocAndCopy				= NULL;
//char *pt_HGUDAllocAndCopyString			= NULL;
//char *pt_HGUDAllocAndCopyData			= NULL;
//char *pt_HGPDFDocOpen					= NULL;
//char *pt_HGPDFDocSetComment				= NULL;
//char *pt_HGPDFDocSetInfo				= NULL;
//char *pt_HGPDFDocSetSignatureInfo		= NULL;
//char *pt_HGPDFDocSetSecurityInfo		= NULL;
//char *pt_HGPDFDocSetDBCEncoding			= NULL;
//char *pt_HGPDFDocStart					= NULL;
//char *pt_HGPDFCryptDictStart			= NULL;
//char *pt_HGPDFCryptDictAddRecipient		= NULL;
//char *pt_HGPDFCryptDictEnd				= NULL;
//char *pt_HGPDFPageStart					= NULL;
//char *pt_HGPDFImageStart				= NULL;
//char *pt_HGPDFImageWrite				= NULL;
//char *pt_HGPDFImageEnd					= NULL;
//char *pt_HGPDFTextStart					= NULL;
//char *pt_HGPDFTextWrite					= NULL;
//char *pt_HGPDFTextGetExtent				= NULL;
//char *pt_HGPDFTextEnd					= NULL;
//char *pt_HGPDFOpeStart					= NULL;
//char *pt_HGPDFOpeDraw					= NULL;
//char *pt_HGPDFOpeEnd					= NULL;
//char *pt_HGPDFOcrTextStart				= NULL;
//char *pt_HGPDFOcrTextWrite				= NULL;
//char *pt_HGPDFOcrTextEnd				= NULL;
//char *pt_HGPDFThumbnailStart			= NULL;
//char *pt_HGPDFThumbnailWrite			= NULL;
//char *pt_HGPDFThumbnailEnd				= NULL;
//char *pt_HGPDFPageSetRotate				= NULL;
//char *pt_HGPDFPageEnd					= NULL;
//char *pt_HGPDFDocEnd					= NULL;
//char *pt_HGPDFDocClose					= NULL;
//char *pt_HGPDDCreateObject				= NULL;
//char *pt_HGPDDSetDecryptInfo			= NULL;
//char *pt_HGPDDSetRecipient				= NULL;
//char *pt_HGPDDReadSecurityInfo			= NULL;
//char *pt_HGPDDInit						= NULL;
//char *pt_HGPDDUpdate					= NULL;
//char *pt_HGPDDFinal						= NULL;
//char *pt_HGPDDReleaseObject				= NULL;
//char *pt_HGPICreateObject				= NULL;
//char *pt_HGPIInit						= NULL;
//char *pt_HGPIGetPageNum					= NULL;
//char *pt_HGPIGetEncryptInfo				= NULL;
//char *pt_HGPIReleaseEncryptInfo			= NULL;
//char *pt_HGPIGetInfo					= NULL;
//char *pt_HGPISetParam					= NULL;
//char *pt_HGPIReleaseObject				= NULL;
//char *pt_HGPSECreateObject				= NULL;
//char *pt_HGPSESetDocInfo				= NULL;
//char *pt_HGPSESetEncryptInfo			= NULL;
//char *pt_HGPSESetSignatureInfo			= NULL;
//char *pt_HGPSESetParam					= NULL;
//char *pt_HGPSEStart						= NULL;
//char *pt_HGPSECryptDictStart			= NULL;
//char *pt_HGPSECryptDictAddRecipient		= NULL;
//char *pt_HGPSECryptDictEnd				= NULL;
//char *pt_HGPSEEnd						= NULL;
//char *pt_HGPSEReleaseObject				= NULL;
//char *pt_HGPFDCreateObject				= NULL;
//char *pt_HGPFDSetDocInfo				= NULL;
//char *pt_HGPFDSetDecryptInfo			= NULL;
//char *pt_HGPFDExecute					= NULL;
//char *pt_HGPFDReleaseObject				= NULL;
//char *pt_HGPFMCreateObject				= NULL;
//char *pt_HGPFMSetDocInfo				= NULL;
//char *pt_HGPFMStart						= NULL;
//char *pt_HGPFMAppend					= NULL;
//char *pt_HGPFMEnd						= NULL;
//char *pt_HGPFMReleaseObject				= NULL;
//char *pt_HGPDPCreateObject				= NULL;
//char *pt_HGPDPSetPageRange				= NULL;
//char *pt_HGPDPExecute					= NULL;
//char *pt_HGPDPReleaseObject				= NULL;
//char *pt_HGPEPCreateObject				= NULL;
//char *pt_HGPEPSetPageRange				= NULL;
//char *pt_HGPEPExecute					= NULL;
//char *pt_HGPEPReleaseObject				= NULL;
//char *pt_HGPETCreateObject				= NULL;
//char *pt_HGPETGetThumbnail				= NULL;
//char *pt_HGPETReleaseThumbnailInfo		= NULL;
//char *pt_HGPETReleaseObject				= NULL;
//char *pt_HGPRTCreateObject				= NULL;
//char *pt_HGPRTExecute					= NULL;
//char *pt_HGPRTReleaseObject				= NULL;
//char *pt_HGP11SetCbFunc					= NULL;
//char *pt_HGP11SetCbParam				= NULL;
//char *pt_HGPIIsEncrypted				= NULL;
// ↑↑↑ROM_DIV有効化対応 End



// Tateishi 2007-3-20 LynxOS実機対応
//関数--------------------------------------------------------------------------------------------------------------- 
//概要　　：バッファをフラッシュする
//戻り値　： 
//備考　　：fsyncを呼び出すだけ 
//作成者　：立石義明
//レビュー：未 
//変更履歴：変更日、変更者、変更内容（変更理由）を記述 
//
int SYSF_fsync( int fd )
{
    // 2008-03-06 Tateishi fsyncは廃止。キャッシュがフラッシュされるまで処理が遅延するため。
    // 2008-04-02 Tateishi fsyncが必要な箇所も存在するためfsync()は復活させる。問題となる箇所では呼び出さないようにする。
	return fsync( fd );
}

// Tornado2.2\target\h\stdio.h 
//関数--------------------------------------------------------------------------------------------------------------- 
//概要　　：書式付き文字列を標準エラー出力に書き込む 
//戻り値　： 
//備考　　： 
//作成者　：本田征裕 
//レビュー：未 
//変更履歴：変更日、変更者、変更内容（変更理由）を記述 
//
extern "C"	// JD.KIM HW対応 
int	printErr (const char *fmt, ...)
{
	int ret;
	va_list vl;

	va_start(vl, fmt);
	ret = vfprintf(stderr, fmt, vl);
	va_end(vl);

	return ret;
}


// Tsukiyama HW対応 
// Tornado2.2\target\h\dcacheCbio.h 
CBIO_DEV_ID _CBIO_DEV_ID;
CBIO_DEV_ID dcacheDevCreate
(
    CBIO_DEV_ID subDev,	  /* lower level device handle */
    char  *   pRamAddr,    /* where it is in memory (NULL = malloc)  */
    int       memSize,	  /* amount of memory to use                */
    char *    pDesc /* device pDesc string */
    )
{
	// Phase2の実装を流用
	return NULL;
}

// JD.KIM HW対応 
// Tornado2.2\target\h\dpartCbio.h 
// FUNCPTR → void*
CBIO_DEV_ID dpartDevCreate
(
	CBIO_DEV_ID subDev,
	int nPart, void* pPartDecodeFunc
	)
{
	// Phase2の実装を流用
	return NULL;
}

// Tsukiyama HW対応 
// Tornado2.2\target\h\dpartCbio.h 
CBIO_DEV_ID dpartPartGet
(
	CBIO_DEV_ID masterHandle, int partNum
	)
{
	// Phase2の実装を流用
	return NULL;
}

// Tsukiyama HW対応 
// Tornado2.2\target\h\dosFsLib.h 
// STATUS → int
int dosFsDevCreate
(
	char * pDevName, 
	CBIO_DEV_ID cbio,
	u_int maxFiles, 
	u_int autoChkLevel
	)
{
	// Phase2の実装を流用
	return OK;
}

// Tsukiyama HW対応 
// Tornado2.2\target\h\dosFsLib.h 
// STATUS → int
// FUNCPTR → void*
int dosFsVolFormat
( 
	void * pDev, int opt, void* pPromptFunc
	)
{
	// Phase2の実装を流用
	return OK;
}

// Tsukiyama HW対応 
// Tornado2.2\target\h\intLib.h 
Bool intContext (void)
{
	// 常にFALSEを返すようにする
	//return FALSE;
    return SYSF_intContext();
}

// JD.KIM HW対応 
// bsp_mosel\Mosel\pciConfigLib.h
// STATUS → int
int pciFindDevice
	(
	int vendorId, 
	int deviceId, 
	int index,
	int * pBusNo, 
	int * pDeviceNo, 
	int * pFuncNo
	)
{
	return TRUE;
}

// JD.KIM HW対応 
// bsp_mosel\Mosel\pciConfigLib.h 
// STATUS → int
int pciConfigInLong
	(
	int busNo, 
	int	deviceNo, 
	int funcNo, 
	int address,
	UINT32 * pData
	)
{
	return TRUE;
}

// JD.KIM HW対応 
// bsp_mosel\Mosel\pciConfigLib.h 
// STATUS → int
int pciConfigInByte
	(
	int busNo, 
	int	deviceNo, 
	int funcNo, 
	int address,
	UINT8 * pData
	)
{
	return TRUE;
}

// Tsukiyama HW対応
// Tornado2.2/target/h/wrn/coreip/ftpLib.h
int ftpHookup(char *host)
{
	return TRUE;
}

// Tsukiyama HW対応
// Tornado2.2/target/h/wrn/coreip/ftpLib.h
// STATUS -> int
int ftpLogin(FAST int ctrlSock, char *user, char *passwd, char *account)
{
	return TRUE;
}

// Tsukiyama HW対応
// Tornado2.2/target/h/wrn/coreip/ftpLib.h
int ftpCommand (int ctrlSock, char *fmt, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6)
{
	return TRUE;
}

// Tsukiyama HW対応
// Tornado2.2/target/h/wrn/coreip/ftpLib.h
int ftpDataConnInit (int ctrlSock)
{
	return TRUE;
}

// Tsukiyama HW対応
// Tornado2.2/target/h/wrn/coreip/ftpLib.h
int ftpDataConnGet (int dataSock)
{
	return TRUE;
}

// Tsukiyama HW対応
// Tornado2.2/target/h/wrn/coreip/ftpLib.h
int ftpReplyGet (int ctrlSock, Bool expecteof)
{
	return TRUE;
}

// Tsukiyama HW対応
// Tornado2.2/target/h/usrLib.h
// STATUS -> int
// Phase2のFILF_Copy()の定義を流用する
int copy (char *in, char *out)
{
	struct stat st;
	char* buf;
	int is = -1, os = -1;	// 2012/12/03 ft_kobayashi 静的解析 CID:454877.
	ssize_t len;
	STATUS s;

	buf = (char*)malloc(4096);
// 2011.02.03 horishita: 静的解析NULLポインタ間接参照の対応 ココから.
	if(buf == NULL)
	{
		return ERROR;
	}
// 2011.02.03 horishita: 静的解析NULLポインタ間接参照の対応 ココまで.
	s = SYSF_stat(in, &st); // stat_honda
	if(s == -1){
		free(buf);	// 2011/10/05 Harada 静的解析(リソースリーク)対応.
		return ERROR;
	}
	if(((is = SYSF_open(in,  O_RDONLY, 0444)) == -1) ||				// open_honda
	   ((os = SYSF_open(out, O_RDWR | O_CREAT | O_TRUNC, 0666)) == -1)){	// open_honda
		free(buf);	// 2011/10/05 Harada 静的解析(リソースリーク)対応.
		
		// 2012/08/31 ft_kobayashi 静的解析 CID:434874.
		if(is != -1){
			SYSF_close(is);
		}
		if(os != -1){
			SYSF_close(os);
		}
	   	
		return ERROR;
	}

	while((len = SYSF_read(is, buf, 4096)) > 0){ // read_honda
		SYSF_write(os, buf, (size_t)len); // write_honda
	}

	SYSF_close(is);	// close_honda
	SYSF_close(os);	// close_honda
	free(buf);
	return OK;
}

// Tsukiyama HW対応
// system/sys/sysg/SYSC_OSWrapper.h
// STATUS -> int
int symFindByName(SYMTAB_ID symtblId, char* name, char** pValue, SYM_TYPE* pType)
{
	return TRUE;
}

// Tsukiyama HW対応
// Tornado2.2/target/h/usrLib.h
// STATUS -> int
int chkdsk (const char * pDevName, u_int repairLevel, u_int verbose)
{
	// Phase2の実装を流用
	return OK;
}

// Tsukiyama HW対応
// Tornado2.2/target/h/cbioLib.h
extern "C"	// JD.KIM HW対応 
STATUS cbioBlkRW		
(
	CBIO_DEV_ID		dev,
	block_t		startBlock,
	block_t		numBlocks,
	addr_t		buffer,
	CBIO_RW		rw,
	cookie_t *		pCookie
	)
{
	// Phase2の実装を流用
	return OK;
}

// Tsukiyama HW対応
// Tornado2.2/target/h/cbioLib.h
extern "C"	// JD.KIM HW対応 
STATUS	cbioIoctl			
    (
    CBIO_DEV_ID		dev,
    int			command,
    addr_t		arg
    )
{
	// Phase2の実装を流用
	return OK;
}

// JD.KIM HW対応 
// Tornado2.2\target\h\cbioLib.h
/* fill CBIO_PARAMS structure */
// STATUS → int
extern "C"	// JD.KIM HW対応 
STATUS cbioParamsGet		
    (
    CBIO_DEV_ID dev,
    CBIO_PARAMS * pCbioParams		
    )
{
	// OK or ERROR if
	return OK;
}

// JD.KIM HW対応 
// Tornado2.2\target\config\Mosel\sysLib.c
extern "C"	// JD.KIM HW対応 
Bool sysBspIntDisable( int irq )
{
	return TRUE;
}

// JD.KIM HW対応 
// Tornado2.2\target\config\Mosel\Controller\cntmr\mvCntmr.h 
/* bsp_donau\Donau\Controller\cntmr\mvCntmr.c */
typedef enum _mvCntmrMode
{
    _COUNTER,            /* Counts down to terminal count and stops          */
    TIMER               /* Reloads itself to the programmed value           */

}MV_CNTMR_MODE;

typedef enum _mvCntmrTcntClocks
{
    ONE_TCLK_CYC, 
    TWO_TCLK_CYC

}MV_CNTMR_TCNT_CLOCKS;

typedef struct _mvCntmrCtrl
{
	MV_BOOL 				enable;		/* enable                           */
	MV_BOOL 				extTrig;	/* External trigger                 */
	MV_CNTMR_MODE 			mode;		/* counter/Timer                    */
	MV_CNTMR_TCNT_CLOCKS 	clocks;		/* No of clocks TC_NTn is asserted  */

}MV_CNTMR_CTRL;

extern "C" {
MV_STATUS mvCntmrDisable(MV_U32 countNum){return TRUE;}
MV_STATUS mvCntmrStart(MV_U32 countNum, MV_U32 value, MV_CNTMR_CTRL *pCtrl){return TRUE;}
MV_STATUS mvCntmrLoad(MV_U32 countNum, MV_U32 value){return TRUE;}
MV_STATUS mvCntmrEnable(MV_U32 countNum){return TRUE;}
}

// JD.KIM HW対応 
// Tornado2.2\target\h\usrFdiskPartLib.h 
#if 0
extern "C"	// JD.KIM HW対応 
STATUS usrFdiskPartCreate
    (
    CBIO_DEV_ID cDev, 	/* device representing the entire disk */
    int		nPart,	/* how many partitions needed, default=1, max=4 */
    int		size1,	/* space percentage for second partition */
    int		size2,	/* space percentage for third partition */
    int		size3	/* space percentage for fourth partition */
    )
{
	return TRUE;
}
#endif

// JD.KIM HW対応 
// Tornado2.2\target\h\usrFdiskPartLib.h 
#if 0
extern "C"	// JD.KIM HW対応 
STATUS usrFdiskPartRead
    (
    CBIO_DEV_ID cDev,            /* device from which to read blocks */
    PART_TABLE_ENTRY *pPartTab,  /* table where to fill results */
    int nPart                    /* # of entries in <pPartTable> */
    )
{
	return TRUE;
}
#endif

// JD.KIM HW対応 
// Tornado2.2\target\src\usr\usrDosFsOld.c 
#if 0
extern "C"	// JD.KIM HW対応 
void dosFsDateTimeInstall (FUNCPTR pDateTimeFunc)
{
}
#endif

#ifndef TORNADO // E.Sasaki HW-FAX :HW(実機)対応において、コンパイルエラーを回避する

// JD.KIM HW対応 
// common\system\fax\faxg\FAXC_OneAddrCommCtrl.cpp 
extern "C"	// JD.KIM HW対応 
void getCommCtrlLog(void)
{
}

// JD.KIM HW対応 
// common\system\fax\faxg\FAXC_OneAddrCommCtrl.cpp 
extern "C"	// JD.KIM HW対応 
void getPageCtrlLog(void)
{
}

// JD.KIM HW対応 
// common\system\fax\faxg\FAX_CommonClass.cpp 
extern "C"	// JD.KIM HW対応 
void printFaxDrv1LogInfo(void)
{
}

// JD.KIM HW対応 
// common\system\fax\faxg\FAXC_OneAddrCommCtrl.cpp 
extern "C"	// JD.KIM HW対応 
void getCommCtrlLog2(void)
{
}

// JD.KIM HW対応 
// common\system\fax\faxg\FAX_CommonClass.cpp 
extern "C"	// JD.KIM HW対応 
void printFaxDrv2LogInfo(void)
{
}

// JD.KIM HW対応 
// common\system\fax\faxg\FAXC_PageCtrlImp.cpp 
extern "C"	// JD.KIM HW対応 
void getPageCtrlLog2(void)
{
}

#endif // HW-FAX

// JD.KIM HW対応 
// bsp_mosel\Mosel\sysLib.c 
extern void wait_10us( uint32_t count );
extern "C"	// JD.KIM HW対応 
void sysUsDelay
    (
    int	delay        /* length of time in microsec to delay */
    )
{
	//void wait_10us( ulong count );
	wait_10us( (uint32_t)(delay/10) );
}

///< パネルドライバ対応 start 
/*================================================================================================
［機能］	デバイスファイルオープン
-------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	成否
［詳細］	デバイスファイル(/dev/paneloutcom)をオープンする。
			オープンに失敗した場合にエラーを返す。
 [履歴]		
================================================================================================*/
static int panelFd = -1;
int sysBspOpenPanelSio()
{
///< パネルドライバ対応 start
#ifdef DEF_PANELDRV	
    if( panelFd == -1 ){
        panelFd = SYSF_devopen( DEVICENAME , O_RDWR, 0666 ); // パネルドライバを使用する
       if( panelFd == -1 )
       { 
#if defined(__PANELDRVDEBUG)
	       perror( DEVICENAME );
#endif
	       return -1;
	   }
		return 0;
    }
    
#else
    int ret;
    struct termios tios_old;
    struct termios tios_new;

    if( panelFd == -1 ){
        panelFd = SYSF_devopen( "/dev/tts/1", O_RDWR, 0666 ); // MoselはCOM2ポートを使用する
        if( panelFd == -1 ) return 0;
    }
    ret = tcgetattr( panelFd, &tios_old );
    if( ret == -1 ) return 0;

    memset( &tios_new, 0, sizeof(tios_new) );

    tios_new.c_lflag &= ~ICANON;					// 非カノニカルモード
    tios_new.c_lflag &= ~(ECHO | ECHONL);			// ローカルエコー無し

    tios_new.c_oflag &= ~ONLCR;						// NL->CR-NL変換無効

    tios_new.c_iflag &= ~(IXON | IXOFF | IXANY);	// ソフトフロー無効
    tios_new.c_iflag &= ~ICRNL;						// CR->NL変換無し
    tios_new.c_iflag &= ~ISTRIP;					// 8ビット目を落とさない
    tios_new.c_iflag &= ~IGNPAR;					// フレームエラーおよびパリティエラーを無視しない
    tios_new.c_iflag |= IGNBRK;						// 入力中のBREAK信号を無視
    tios_new.c_iflag |= INPCK;						// 入力のパリティチェックを有効

    tios_new.c_cflag |= CLOCAL;						// モデムの制御線を無視
    tios_new.c_cflag |= CREAD;						// 受信を有効
    tios_new.c_cflag |= CS8;						// 8ビットデータ
    tios_new.c_cflag &= ~CRTSCTS;					// フロー制御無効	// [D401]NACK ERROR - DataLength Error. 回避のため honda 2007-04-06
    tios_new.c_cflag &= ~CSTOPB;					// ストップビット1ビット
    tios_new.c_cflag |= PARENB;						// パリティあり
    tios_new.c_cflag &= ~PARODD;					// 偶数パリティ

    tios_new.c_cc[VMIN] = 0;
    tios_new.c_cc[VTIME] = 0;

    cfsetospeed( &tios_new, B19200 );
    cfsetispeed( &tios_new, B19200 );

    // 設定
    tcflush( panelFd, TCIFLUSH );
    ret = tcsetattr( panelFd, TCSANOW, &tios_new );
    if( ret == -1 ) return 0;
#endif // DEF_PANELDRV 
///< パネルドライバ対応 end 
    return 0;
}

#ifdef DEF_PANELDRV
/*================================================================================================
［機能］	パネルデータ種別からIoctlデータを取得する。
-------------------------------------------------------------------------------------------------
［引数］	EXDB_PanelKind *kind : パネルデータ種別
［戻値］	int 				 : Ioctl用種別
［詳細］	パネルデータ種別から、Ioctl種別を取得する。
 [履歴]		10/11/16 浦崎 輝度設定対応.
			2012.03.05	井柳	CQ362599 LED一括設定対応.
			2018/01/18 	荒木 	AR.1031986 振動パネル対応.
================================================================================================*/
int sysBspIoctlPaneldata(EXDB_PanelKind kind){
	int ret;
	switch(kind){
	case EXDB_PanelKindsetLEDBrightness:	// LED輝度設定 
		ret = ENM_IOCTL_CMD_LEDBRIGHTNESS_SET;
		break;
	case EXDB_PanelKindSetLed:				// LED設定 
	case EXDB_PanelKindSetLedAll:			// 全LED点灯/消灯設定	2012/03/05 井柳(SKY) CQ362599 LED一括設定対応.
		ret = ENM_IOCTL_CMD_LEDONOFF_SET;
		break;
	case EXDB_PanelKindSetLedBlink:			// LED点滅設定 
	case EXDB_PanelKindSetLedBlinkMulti:	// 一括LED点滅設定		2012/03/05 井柳(SKY) CQ362599 LED一括設定対応.
		ret = ENM_IOCTL_CMD_LEDBLINK_SET;
		break;
	case EXDB_PanelKindSetBackLight:		// バックライト設定 
		ret = ENM_IOCTL_CMD_BACKLIGHTONOFF_SET;
		break;
	case EXDB_PanelKindSetBuzzer:			// ブザー設定 
		ret = ENM_IOCTL_CMD_BUZZER_SET;
		break;
	case EXDB_PanelKindSetBuzzerVal:		// ブザー設定(音量) 
		ret = ENM_IOCTL_CMD_BUZZER_SET;
		break;
	case EXDB_PanelKindGetLed:				// LED取得 
		ret = ENM_IOCTL_CMD_LEDOUTSTATE_GET;
		break;
	case EXDB_PanelKindGetLCDVersion:		// LCDバージョン取得 
		ret = ENM_IOCTL_CMD_KM_SOFTINFO_GET;
		break;
	case EXDB_PanelKindGetLockKey:			// ロックキー取得 
		ret = ENM_IOCTL_CMD_LOCKKEYSTATE_GET;
		break;
	case EXDB_PanelDataGet:					// パネルデータ取得要求
		ret = ENM_IOCTL_CMD_OUTSTATE_GET;
		break;
	case EXDB_PanelCommunicaitonSuspend:	// 通信停止/再開 省電力制御用 
		ret = ENM_IOCTL_CMD_COMMUNICATION_SUSPEND;
		break;
	case EXDB_PanelKindSetBackLightBrightness:	// バックライト輝度設定要求. 10/11/12 浦崎 輝度設定対応.
		ret = ENM_IOCTL_CMD_BACKLIGHTBRIGHTNESS_SET;
		break;
	case EXDB_PanelKindGetBackLightInfo:	// バックライト情報取得要求. 10/11/16 浦崎 輝度設定対応.
		ret = ENM_IOCTL_CMD_BACKLIGHTINFO_GET;
		break;
	case EXDB_PanelType_Get:	// t-kasahara タッチパネル対応.
		ret = ENM_IOCTL_CMD_PANELTYPE_GET;
		break;
	case EXDB_PanelCalibration:	// t-kasahara タッチパネル対応.
		ret = ENM_IOCTL_CMD_CALIBRATION;
		break;
	case EXDB_Panelinit:	// t-kasahara タッチパネル対応.
		ret = ENM_IOCTL_CMD_PANEL_INIT;
		break;
	case EXDB_Panelsleep:	// t-kasahara タッチパネル対応.
		ret = ENM_IOCTL_CMD_PANEL_SLEEP;
		break;
	case EXDB_PanelGetPanelInfo:	// t-kasahara タッチパネル対応.
		ret = ENM_IOCTL_CMD_PANEL_GETPANELINFO;
		break;
	case EXDB_PanelDebug:	// 2011.08.18 KM成松 パネル応答性評価対応.
		ret = ENM_IOCTL_CMD_PANEL_DEBUG;
		break;
	// ◆Start 2011/12/02 kruachi MinervaZeus マップスクローラ問題対応.
	case EXDB_PanelSetSensitivity:	// t-kasahara タッチパネル対応.
		ret = ENM_IOCTL_CMD_PANEL_SETSENSITIVITY;
		break;
	// ◆End 2011/12/02 kruachi MinervaZeus マップスクローラ問題対応.
	// ◆Start 2012/01/18 kurachi MinervaZeus CQ:360387ログ仕込み対応.
	case EXDB_Ele_PanelLog:
		ret = ENM_IOCTL_CMD_PANEL_LOG;
		break;
	// ◆End 2012/01/18 kurachi MinervaZeus CQ:360387ログ仕込み対応.
	case EXDB_PanelKindGetPanelTypeStruct:	// 2012.02.28 uchino CQ362429 パネル端部26dot対応 パネルタイプ取得.
		ret = ENM_IOCTL_CMD_PANELTYPESTRUCT_GET;
		break;
	case EXDB_PanelOffset:	// 2012.02.28 uchino CQ362429 パネル端部26dot対応 静電TP不感領域オフセット対応.
		ret = ENM_IOCTL_CMD_PANEL_OFFSET_GET;
		break;
	case EXDB_PanelSoundPlay:	// 2012/03/07 堀井 スライドレス_音声IC対応.
		ret = ENM_IOCTL_CMD_PANEL_SOUND_PLAY;
		break;
	case EXDB_PanelSoundStop:	// 2012/03/07 堀井 スライドレス_音声IC対応.
		ret = ENM_IOCTL_CMD_PANEL_SOUND_STOP;
		break;
	case EXDB_PanelSoundVolume:	// 2012/03/07 堀井 スライドレス_音声IC対応.
		ret = ENM_IOCTL_CMD_PANEL_SOUND_VOLUME;
		break;
	case EXDB_PanelSoundStatus:	// 2012/03/07 堀井 スライドレス_音声IC対応.
		ret = ENM_IOCTL_CMD_PANEL_SOUND_STATUS;
		break;
	case EXDB_PanelSoundDownload:	// 2012/04/02 堀井 スライドレス_音声ICダウンロード対応.
		ret = ENM_IOCTL_CMD_PANEL_SOUND_DOWNLOAD;
		break;
	case EXDB_PanelEEPROMWrite:	// 2012/04/26 堀井 パネルへの人体検出感度データ保持化.
		ret = ENM_IOCTL_CMD_PANEL_EEPROM_WRITE;
		break;
	case EXDB_PanelEEPROMRead:	// 2012/04/26 堀井 パネルへの人体検出感度データ保持化.
		ret = ENM_IOCTL_CMD_PANEL_EEPROM_READ;
		break;
	case EXDB_PanelSoundVersion:	// 2012/05/07 堀井 スライドレスパネル_音声バージョン通知.
		ret = ENM_IOCTL_CMD_PANEL_SOUND_VERSION;
		break;
	case EXDB_PanelSoundCheckSum:	// 2012/05/07 堀井 スライドレスパネル_音声バージョン通知.
		ret = ENM_IOCTL_CMD_PANEL_SOUND_CHECKSUM;
		break;
	case EXDB_PanelSetSensitivitySlidelessGDP:	// 2012/06/22 堀井 スライドレス_GDP閾値調整機能.
		ret = ENM_IOCTL_CMD_PANEL_SETSENSITIVITY_SLIDELESSGDP;
		break;
	case EXDB_PanelSetAndroidStatus:
		ret = ENM_IOCTL_CMD_PANEL_SETANDROIDSTATUS;
		break;
	case EXDB_PanelAndroidEvent:
		ret = ENM_IOCTL_CMD_PANEL_ANDROIDEVENT;
		break;
	case EXDB_PanelAndroidEvent2:
		ret = ENM_IOCTL_CMD_PANEL_ANDROIDEVENT2;
		break;
	case EXDB_PanelAndroidEvent2_OFF:
		ret = ENM_IOCTL_CMD_PANEL_ANDROIDEVENT2_OFF;
		break;
	case EXDB_PanelLockPanelTablet:	// パネルおよび端末のロック、アンロック要求.	// 2014/01/10 Harada CQ.384345 制御構成変更No.8 bizhubRA市場指摘対応.
		ret = ENM_IOCTL_CMD_PANEL_LOCKPANELTABLET;
		break;
	case EXDB_PanelFWDownload:					// 2013/08/05 堀井 パネルマイコンFWダウンロード対応.
		ret = ENM_IOCTL_CMD_PANEL_FW_DOWNLOAD;
		break;
	case EXDB_PanelKindSetLedExtend:	// 2013/08/08 堀井 次世代パネル対応_cap1166パネルマイコン対応.
		ret = ENM_IOCTL_CMD_PANEL_EXTENDLED_SET;
		break;
	case EXDB_PanelKindSetCTSC:			// 2013/08/08 堀井 次世代パネル対応_cap1166パネルマイコン対応.
		ret = ENM_IOCTL_CMD_PANEL_CTSC_SET;
		break;
	case EXDB_PointTouchPanelCalibration:		// キャリブレーション情報取得	2014/05/19  FT_yamaguchi 抵抗膜キャリブレーション対応.
		ret = ENM_IOCTL_CMD_PANEL_POINT_TOUCH_CALIBRATION;
		break;
	case EXDB_PanelKindNFCPairingDataTrans:		// NFCペアリングデータ転送			2014/07/03 FT_yamaguchi NFC対応.
		ret = ENM_IOCTL_CMD_PANEL_NFC_PAIRING_DATA_TRANS;
		break;
	case EXDB_PanelKindPossibleNFCPairing:		// NFCペアリング実行可否設定		2014/07/03 FT_yamaguchi NFC対応.
		ret = ENM_IOCTL_CMD_PANEL_POSSIBLE_NFC_PAIRING;
		break;
	case EXDB_PanelKindNFCAuthDataTrans:		// NFC認証データ転送				2014/09/08 FT_yamaguchi NFC対応(ZeusS)
		ret = ENM_IOCTL_CMD_PANEL_NFC_AUTH_DATA_TRANS;
		break;
	case EXDB_PanelKindGetSoftInfo:		// パネルマイコンFWダウンロード対応 2014/11/16 堀井.
		ret = ENM_IOCTL_CMD_SOFTINFO_GET;
		break;
	case EXDB_PanelKindNFCBLEFunctionCount:		// NFCBLE機能カウント				2015/10/19 機能カウンタ追加対応 山口.
		ret = ENM_IOCTL_CMD_PANEL_NFCBLE_FUNCTION_COUNT;
		break;
	case EXDB_Pcie0LinkUpCheck:					// PCIe0 LinkUpチェック				2016/01/16 近藤 CQ.524101.
		ret = ENM_IOCTL_CMD_PCIE0LINKUPCHECK;
		break;
	case EXDB_PanelSetVibrator:					// 振動パネル対応 					2018/01/18 Sky-Araki AR.1031986.
		ret= ENM_IOCTL_CMD_PANEL_SET_VIBRATOR;
		break;
	case EXDB_PanelKindErr:					// その他エラー
	default:
		ret = -1;
		break;
	}
	return ret;
}

/*================================================================================================
［機能］	パネルデータ設定・取得
-------------------------------------------------------------------------------------------------
［引数］	void *buffer 			各パネルデータ
			EXDB_PanelKind *kind	パネルデータ種別
［戻値］	成功 0
			失敗 -1
［詳細］	デバイスファイルにパネルデータを設定する。
			デバイスファイルのオープン失敗、パネルデータの設定・取得
			に失敗した場合にエラーを返す
			Ioctlに失敗した場合、パネル通信ドライバのバッファオーバーフローの
			可能性がある為100msディレイを掛けてから、DebugForeverを呼び出す。
			ディレイを掛けなければ、バッファオーバーフローの場合にブザー音が鳴りません。
 [履歴]		2011.11.21	堀下	Android端末LED表示対応追加.
			2011.12.26	堀下	CQ359300(StartKeyLEDが変化しない)対応.
			2012.01.16	堀下	CQ358620(電源LEDが点かない)対応.
			2012.02.22	堀下	CQ361928(スタートキーが効かない)対応.
			2012.02.28	堀下	CQ360428(LED連動)対応.
			2012.03.05	井柳	CQ362599 LED一括設定対応.
			2012.08.21	Gonsho	CQ366909,367638 Android LED表示問題対応.
			2015.10.22	大岡	[ZeusSZX0]制御構成変更No.76-1 新詳細コードデータ収集機能.
================================================================================================*/
extern "C" 
int sysBspIoctlPanelSio( FUNCPTR buffer, EXDB_PanelKind kind ){
	// Android端末接続有無.
	Bool bl_Connect = THRC_AndroidControl::getInstance()->THRG_IsConnectAndroid();
	if( bl_Connect != False ) {
		THRC_AndroidControlIf::getInstance()->THRL_AndroidSetLedPanelcom( buffer, kind );
	}
	
	///< ファイルオープン 
	if( panelFd == -1 ){
        sysBspOpenPanelSio();
        if( panelFd == -1 ) return -1;
    }
   	///< パネル種別取得 
	int cmd;
	cmd = sysBspIoctlPaneldata(kind);
	if(cmd == -1){
		return cmd;
	}
	// ToDo:暫定
	int ret;
	ret = SYSF_ioctl( panelFd, cmd, (intptr_t)buffer);
/* 2018/05/22 ft_kondo SimLin不要コンパイルSW撲滅対応 */
#if defined(FUM_LINUX)	// シミュレータで有効.
	if( 0 == ret ) {
		// エラーが無い場合は、擬似ioctlも実施.
		ret = SIMC_SoundObserverIf::newInstance()->ioctl( cmd, buffer);
	}
#endif
	switch(cmd){	// 上位にてエラー対応するため暫定対応 2012/04/02 堀井 スライドレス_音声ICダウンロード対応.
		case ENM_IOCTL_CMD_PANEL_SOUND_DOWNLOAD:
		case ENM_IOCTL_CMD_PANEL_SOUND_VERSION:	// 2012/05/07 堀井 スライドレスパネル_音声バージョン通知.
		case ENM_IOCTL_CMD_PANEL_SOUND_CHECKSUM:	// 2012/05/07 堀井 スライドレスパネル_音声バージョン通知.
		case ENM_IOCTL_CMD_PANEL_SETSENSITIVITY_SLIDELESSGDP:	// 2012/06/22 堀井 スライドレスパネル_GDP閾値調整機能.
		case ENM_IOCTL_CMD_PANEL_FW_DOWNLOAD:					// 2013/08/08 堀井 次世代パネル対応_パネルマイコンFWダウンロード対応.
		case ENM_IOCTL_CMD_PANEL_NFC_PAIRING_DATA_TRANS:		// 2014/11/27 山口 CQ.507043 ペアリングデータ転送失敗時はトラブルを出さない.
			return ret;
	}
	if( ret <= -1 )	{
		SYSF_taskDelay(100);
		// 新詳細コード保存.
		uint64_t		ull_NewDetailData = THRD_NewDetail_ModuleId_Thread;		// 1,2桁：スレッド.	
		ull_NewDetailData |= (uint64_t)( 0xD401 ) << THRD_NewDetail_TroubleCode_Shift;	// 3-6桁：トラブルコード.
		ull_NewDetailData |= (uint64_t)( cmd ) << 24;							// 7-10桁：コマンド.
		THRL_ThAssertLog2( 0, ull_NewDetailData );
		DebugForever( 0xD401 );
	}
	return ret;
}
/*================================================================================================
［機能］	Androidパワーセーブキー情報設定.
-------------------------------------------------------------------------------------------------
［引数］	unsigned char mode 			長押し、短押し.
［戻値］	なし.
［詳細］	
 [履歴]		2011/11/28 t-kasahara 新規追加　省電力対応.
================================================================================================*/
extern "C" 
void sysBspSetAndroidPowerSaveStat( unsigned char mode ){
	
	THRC_AndroidControl::getInstance()->THRL_AndroidsetPowerSaveStat( mode );
	
}


// 2018/04/10 ft_kondo 制御構成変更No110 ログリファイン. 
// debug_Paneldriver()は未使用のため削除する. 
#endif // DEF_PANELDRV

//デバッグ用
extern "C" 
void debug_AndroidStat( uchar mode ){
	
	sysBspIoctlPanelSio((void *)&mode, EXDB_PanelSetAndroidStatus);
//	sysBspIoctlPanelSio((void *)mode, EXDB_PanelSetAndroidStatus);
}

extern "C" 
void debug_AndroidSetInfo( unsigned short x1, unsigned short y1, uchar state ){
	ANDROD_PANELINFO tmp_info;
	
	tmp_info.PanelX =		x1;
	tmp_info.PanelY =		y1;
	tmp_info.PanelState =	state;
	
	sysBspIoctlPanelSio((void *)&tmp_info, EXDB_PanelAndroidEvent);
//	sysBspIoctlPanelSio((void *)x1, EXDB_PanelAndroidEvent);
}

extern "C" 
void debug_AndroidSetEvent( ushort state ){
	
	sysBspIoctlPanelSio((void *)&state, EXDB_PanelAndroidEvent2);
//	sysBspIoctlPanelSio((void *)state, EXDB_PanelAndroidEvent2);
}

///< パネルドライバ対応 end 
// JD.KIM HW対応 
// Tornado2.2\target\config\Mosel\sysLib.c 
extern "C"	// JD.KIM HW対応 
ssize_t sysBspWritePanelSio( char * buffer, size_t size )
{
    ssize_t ret;

    if( panelFd == -1 ){
        ret = sysBspOpenPanelSio();
		// 2011/12/19 ft_kondo 静的解析 CID:16020. 
        if( ret == -1 ) return 0;
    }

    ret = SYSF_write( panelFd, buffer, size );

	return ret;
}

// JD.KIM HW対応 
// Tornado2.2\target\config\Mosel\sysLib.c 
extern "C"	// JD.KIM HW対応 
ssize_t sysBspReadPanelSio( char * buffer, size_t size )
{
    int ret = 0;
    int bytesRead = 0;
    ssize_t dataNum = 0;

    if( panelFd == -1 ){
        ret = sysBspOpenPanelSio();
		// 2011/12/19 ft_kondo 静的解析 CID:16019. 
        if( ret == -1 ) return 0;
    }
	ret = SYSF_ioctl( panelFd, FIONREAD, (intptr_t)&bytesRead );

	if( bytesRead != 0 ){
		dataNum = SYSF_read( panelFd, buffer, size );
	}
	return dataNum;
}

// Tornado2.2\target\h\sysLib.h 
// LINUX 200905 Sakiyama
//extern char* getThreadBufferTopAdrOnMainMemory();
char *	sysMemTop (void)
{
// LINUX 200905 Sakiyama
//	return getThreadBufferTopAdrOnMainMemory();
	return 0;
}

// JD.KIM HW対応 
extern "C" {
// Tornado2.2\target\h\time.h 
int timer_connect (timer_t timerid, VOIDFUNCPTR routine, int arg){return OK;}
}

// Tornado2.2\target\h\cbioLib.h 
IMPORT int cbioRdyChgdGet(CBIO_DEV_ID dev){return TRUE;}
IMPORT STATUS cbioRdyChgdSet(CBIO_DEV_ID dev,Bool status){return TRUE;}


// ↓↓↓ROM_DIV有効化対応 Start
////関数--------------------------------------------------------------------------------------------------------------- 
////appInit空関数対応
////
//
//// common\system\sys\sysg\appInit.c 
//extern "C"
//void appInit(void){}
// ↑↑↑ROM_DIV有効化対応 End

#if defined(INCLUDE_MIO) || defined(FUM_LINUX)
#else
//関数--------------------------------------------------------------------------------------------------------------- 
//MIO空関数対応
//

// common/nicfum/mioApi.h
int mioSockDelete(void *msock){return 0;}
t_eMioMibResult mioSetMibDataObj( t_sMioMibDataDesc* pDataDesc ){return MIB_RESULT_NoError;}
t_eMioMibResult mioGetMibDataObj( t_sMioMibDataDesc* pDataDesc ){return MIB_RESULT_NoError;}
extern "C" {
// common/nicfum/simulator/src/mioApi.c 
int mioSntpGettime(struct timespec *pTimespec)
{
	time_t	s_time;
	time( &s_time );
	pTimespec->tv_sec = s_time;
	return 0;
}

// common\system\cnv\cnvc\thr\thrcosimdummy.cpp 
void NclDataWrite(int val){}

// common/nicfum/mioApi.h
//STATUS mioSetDeviceInfo( t_eDeviceSelect eSetDev, void* value ){return 0;}
int mioSSLGetDigitalCertInfo(char* pCert, int CertSize, t_sMioSslCertInfo *pCertInfo){return 0;}
int mioSetRendezvousInfo(t_sMioRendezvousInfo *pInfo){return 0;}
int mioSendEofRequest(MioFileDesc fd){return 0;}
int mioPing(char *pHost, int nNumPackets, uint32_t ulOptions){return 0;}
int mioPing6(char *pszHost, int nNumPackets, uint32_t ulOptions){return 0;}
int mioIPsecActivate(t_sMioIPsecSetting* p_sIPsecSetting){return 0;}
int mioIPsecShutdown( void ){return 0;}
int mioGetNetWareQueueStatus(t_sMioNetWareQueueStatus* nwQstat, unsigned int startIndex, unsigned int maxQ){return 0;}
int mioCheckLDAPSetting(t_sMioLdapSetting *pSetting){return 0;}
t_eMioSslStatus mioSSLCertSelfCreate(t_sMioSslCertInfo *pInfo, t_eMioSSLEncryption ucMech, t_eMioPrivKeyType eKeyType){return (t_eMioSslStatus)0;}
t_eMioSslStatus mioSSLGetCertInfo(t_sMioSslCertInfo *pInfo, UINT32 uiCertID){return (t_eMioSslStatus)0;}
STATUS mioSSLGetReqData(char *pCSR){return 0;}
STATUS mioSSLCertReqStatus(void){return 0;}
STATUS mioSSLCertSelfStatus(void){return 0;}
t_eMioSslStatus mioSSLCertReqCreate(t_sMioSslCertInfo *pInfo, t_eMioPrivKeyType eKeyType){return (t_eMioSslStatus)0;}
t_eMioSslStatus mioSSLRemoveCert(UINT32 uiCertID){return (t_eMioSslStatus)0;}
t_eMioSslStatus mioSSLInstallCert(const char *pCert, t_eMioSSLEncryption ucMech, UINT32 uiCertID){return (t_eMioSslStatus)0;}
int mioMngListenPort(MioMngLPortFunc func, UINT16 port){return 0;}
void mioSockSetSSL(void *msock, BOOL flag, t_eMioCertUsage eCertUsage){}
void mioSockFdSet(void *msock, fd_set *fds){}
int mioSockFdIsSet(void *msock, fd_set *fds){return 0;}
void *mioSockAccept(void *msock, struct sockaddr *name,int *namelen){}
int mioSockGetSD(void *msock){return 0;}
int mioSockSelect(void *msock,int width, fd_set *pReadFds, fd_set *pWriteFds, fd_set *pExceptFds, struct timeval *pTimeOut){return 0;}
int mioSockCreate(void **msock,int domain, int type, int protocol){return 0;}
int mioSockSetsockopt(void *msock, int level, int optName,char *optValue, int optLen){return 0;}
int mioSockBind(void *msock, struct sockaddr *name,int namelen){return 0;}
int mioSockListen(void *msock, int backlog){return 0;}
int mioSockRecv(void *msock, char *buf, int buflen, int flags){return 0;}
BOOL mioSockDataAvailable(void *msock, char *buf, int buflen, int *plErr){return 0;}
int mioSockSend(void *msock, const char *buf, int buflen, int flags){return 0;}
int mioSetFuncTblToGetName(t_sMioFuncTblToGetName *pMioFuncTbl){return 0;}
int mioSockGetpeername(void *msock, struct sockaddr *name, int *namelen){return 0;}
int mioSockClose(void *msock){return 0;}
STATUS mioGenerateUuidString(char *paUuidString){return 0;}
STATUS mioMngPortVerify(t_eMioMngPortProtocol eProto, UINT16 usPort){return 0;}
STATUS mioMngSettingPortRegist(t_eMioMngPortProtocol eProto, UINT16 usPort){return 0;}
STATUS mioMngSettingPortDelete(t_eMioMngPortProtocol eProto, UINT16 usPort){return 0;}
t_eMioDot1xAuthStatus  mioDot1xAuthStatusGet( void ){return (t_eMioDot1xAuthStatus)0;}
t_eMioDot1xConnStatus  mioDot1xConnStatusGet( void ){return (t_eMioDot1xConnStatus)0;}
void mioDot1xSetDefault( void ){}
STATUS mioDot1xCompleteConfig( void ){return 0;}
t_eMioSslStatus mioSSLGetCertIssuerInfo(t_sMioSslCertInfo *pInfo, UINT32 uiCertID){return (t_eMioSslStatus)0;}
UINT32 mioSSLGetCertID( t_eMioCertUsage eCertUsage ){return 0;}
t_eMioSslStatus mioSSLExportCert(UINT32 uiCertID, t_eMioCertFormat eFormat, UINT8* pPass, UINT32* pSize, UINT8* pCert){return (t_eMioSslStatus)0;}
t_eMioSslStatus mioSSLSetDevCertProtocol(UINT32 uiCertID, UINT32 uiCertUsage, UINT32 uiCertUsageApp){return (t_eMioSslStatus)0;}
t_eMioSslStatus mioSSLImportCert(UINT8* pCert, UINT32 uiSize, UINT32 uiCertType, const char* pPass ){return (t_eMioSslStatus)0;}
t_eMioSslStatus mioSSLGetCertList(UINT32 uiCertID, UINT32 nuiCertType, UINT32* pCertNum, t_sMioCertList* pCertList){return (t_eMioSslStatus)0;}
STATUS mioSSLGetPKCS12(UINT32 uiCertNo, const char* pPass, INT32* pPkcs12Size, char* pPkcs12){return 0;}
MIO_CHAN_STATUS mioGetChannelStatus(MioChan chan){return (MIO_CHAN_STATUS)0;}
STATUS mioNvramVersionDown(t_eNvramVersionDownType eDownType){return 0;}
STATUS mioSendSNMPTrap(t_sMioSnmpTrapGroup* pSendTarget, t_sMioSendTrapParam* pSendParam){return 0;}

// common/nicfum/simulator/OpenSSL/inc32/openssl/ssl.h
int SSL_library_init(void){return 0;}
int SSL_shutdown(SSL *s){return 0;}
void SSL_load_error_strings(void ){}
void SSL_free(SSL *ssl){}
void SSL_CTX_free(SSL_CTX *){}
int	SSL_pending(SSL *s){return 0;}
SSL_CTX *SSL_CTX_new(SSL_METHOD *meth){return 0;}
SSL *	SSL_new(SSL_CTX *ctx){return 0;}
int	SSL_set_fd(SSL *s, int fd){return 0;}
int SSL_connect(SSL *ssl){return 0;}
int SSL_write(SSL *ssl,const char *buf,int num){return 0;}
int SSL_read(SSL *ssl,char *buf,int num){return 0;}
int SSL_accept(SSL *ssl){return 0;}
int	SSL_use_certificate_file(SSL *ssl, const char *file, int type){return 0;}
int	SSL_use_PrivateKey_file(SSL *ssl, const char *file, int type){return 0;}
SSL_METHOD *SSLv2_method(void){return 0;}
SSL_METHOD *SSLv2_server_method(void){return 0;}
SSL_METHOD *SSLv2_client_method(void){return 0;}
SSL_METHOD *SSLv3_method(void){return 0;}
SSL_METHOD *SSLv3_server_method(void){return 0;}
SSL_METHOD *SSLv3_client_method(void){return 0;}
SSL_METHOD *SSLv23_method(void){return 0;}
SSL_METHOD *SSLv23_server_method(void){return 0;}
SSL_METHOD *SSLv23_client_method(void){return 0;}
SSL_METHOD *TLSv1_method(void){return 0;}
SSL_METHOD *TLSv1_server_method(void){return 0;}
SSL_METHOD *TLSv1_client_method(void){return 0;}

// common/nicfum/simulator/OpenSSL/inc32/openssl/err.h
uint32_t ERR_get_error(void){return 0;}
uint32_t ERR_get_error_line(const char **file,int *line){return 0;}
uint32_t ERR_get_error_line_data(const char **file,int *line,const char **data, int *flags){return 0;}
char *ERR_error_string(uint32_t e,char *buf){return 0;}

// common/nicfum/simulator/OpenSSL/inc32/openssl/rand.h
int RAND_status(void){return 0;}
void RAND_seed(const void *buf,int num){}
int  RAND_bytes(unsigned char *buf,int num){return 0;}

// common/nicfum/simulator/OpenSSL/inc32/openssl/md5.h
unsigned char *MD5(const unsigned char *d, uint32_t n, unsigned char *md){return 0;}

}	// extern "C"
//------------------------------------------------------------------------------------------------------------------- 
#endif	/* INCLUDE_MIO	*/

/* Linux暫定対応 */
extern "C" {
#if 0	/* #if0 -> BootUpScreen対応 adsol t.sasaki */
    Bool EraseBSBackupArea(void){return 0;}
    Bool setBootupScreen( void* addr, unsigned long size ){return 0;}
    unsigned long getBootupScreen( void ){return 0;}
#else

/* --------------------------------*/
/* BootUpScreen対応 adsol t.sasaki */
/* --------------------------------*/

//Str 2011/02/02 ft_kondo CQ336999 BootupScreenが登録出来ない問題対策.
/* BootUpScreenの実体(シンボリックリンクのリンク先) */
#define BUS_BACKUP_IMG1_FILEPATH	"/sdir/bootupscreen1"
#define BUS_BACKUP_IMG2_FILEPATH	"/sdir/bootupscreen2"
//End 2011/02/02 ft_kondo CQ336999 BootupScreenが登録出来ない問題対策.

/* BootUpScreen 最大サイズ(理論値として 800×480 の非圧縮サイズを規定) */
#define BOOTUP_SCREEN_MAX_SIZE			(800 * 480 * (24/8))

/* MBFフラッシュのためのウェイト(1tick:5ms) */
#define BUS_WAIT_TIME_PER_1SECTOR		(400)

/* MirrorBitFlash 1セクタサイズ(256K) */
#define MBF_SECTOR_SIZE					(256*1024)

/* MBF版スイッチ */
/* --------------------------------------------*/
/* MinervaZeus実機(GaiaPF)ではフラッシュROMが、*/
/* MBFからSSDへ変更されるが、先行開発期間では、*/
/* MBFを使用して開発を行なうため               */
/* 切り替えようのディファインを定義する        */
/* (SSDでは「BUS_FOR_MBF」をコメントアウトする) */
/* --------------------------------------------*/
#define BUS_FOR_MBF
/* --------------------------------------------*/

/* BootUpScreen登録取得Area. */
extern char* pt_ALLOC_BootupScreen;

static Bool verifyBUS(const char *filePath, void* addr, size_t size)
{
	int fd;
	int ret;
	ssize_t readRet;
/*	unsigned long cnt;*/
	int cnt;	/* 2013/09/18 Harada MinervaZeus3rd コンパイルワーニング除去対応 */
	struct stat statInfo;
	char *buf = NULL;

	/* BootUpScreenファイルオープン */
	fd = SYSF_open(filePath, O_RDONLY, 0666);
	if(fd == -1){
		/* ファイルオープンエラーならBootUpScreenなしと判断する */
		printf( "(%010u)(BSP) %s There is none(verfy)\n", SYSF_tickGet() , filePath);
		return False;
	}

	/* BootUpScreenファイルサイズ取得のため */
	ret = SYSF_fstat(fd, &statInfo);
	if(ret != 0) {
		printf( "(%010u)(BSP) %s File status acquisition error(verfy)\n", SYSF_tickGet() , filePath);
		SYSF_close(fd);
		return False;
	}

	/* ベリファイ用のデータバッファを確保 */
	buf = (char*)malloc(size);
	if(buf == NULL) {
		SYSF_close(fd);
		return False;
	}

	/* BootUpScreenを読み出して内部バッファへ格納 */
	readRet = SYSF_read(fd, buf, (size_t)statInfo.st_size);
	if(readRet == -1) {
		SYSF_close(fd);
		free(buf);
		printf( "(%010u)(BSP) %s Read error(verfy)\n", SYSF_tickGet() , filePath);
		return False;
	}

	/* BootUpScreenファイルクローズ */
	SYSF_close(fd);

	// 2011/12/19 ft_kondo 静的解析 CID:33191. 
	for(cnt = 0; cnt < readRet; cnt++) {
		if(((char *)addr)[cnt] != buf[cnt]) {
			printf( "(%010u)(BSP) %s Verify NG!!\n", SYSF_tickGet() , filePath);
			/* ベリファイ用のデータバッファを解放 */
			free(buf);
			return False;
		}
	}

	/* ベリファイ用のデータバッファを解放 */
	free(buf);

	return True;
}

static Bool checkEraseBUS(void)
{
	int fd;

	/* BootUpScreen が存在するか確認のためのファイルオープン */
	fd = SYSF_open(BOOTUP_SCREEN_FILEPATH, O_RDONLY, 0666);

	if(fd != -1) {
		SYSF_close(fd);
		return False;
	}

	return True;
}
#if 0
static Bool creBSSymLink(const char *filePath)
{
	int fd;
	int ret;

	/* シンボリックリンクが存在するかチェック */
	fd = SYSF_open(BOOTUP_SCREEN_FILEPATH, O_RDONLY, 0666);
	if(fd != -1) {
		/* 存在する場合はファイルクローズ */
		SYSF_close(fd);

		/* シンボリックリンクを削除 */
		SYSF_unlink(BOOTUP_SCREEN_FILEPATH);
	}

	/* シンボリックリンク作成 */
	ret = symlink(filePath, BOOTUP_SCREEN_FILEPATH);
	if(ret == -1) {
		/* 作成失敗 */
		printf("%s シンボリックリンク作成失敗\n", filePath);
		return False;
	}

	return True;
}

static void getBSBackupStatus(char *filePath)
{
	int ret;
	struct stat stat;
	char backupImgFilepath[128];

	/* シンボリックリンク情報を取得 */
	ret = lstat(BOOTUP_SCREEN_FILEPATH, &stat);
	if(ret == -1) {
		/* 画像データ①のファイルパスを出力 */
		strcpy(filePath, BUS_BACKUP_IMG1_FILEPATH);
		return;
	}

	/* シンボリックリックかどうかチェック */
	if((stat.st_mode & S_IFMT) != S_IFLNK) {
		/* 画像データ①のファイルパスを出力 */
		strcpy(filePath, BUS_BACKUP_IMG1_FILEPATH);
		return;
	}

	/* テンポラリバッファをクリア */
	memset(backupImgFilepath, '\0', sizeof(backupImgFilepath));

	/* シンボリックリンクのリンク先を取得 */
	ret = readlink(BOOTUP_SCREEN_FILEPATH, backupImgFilepath, sizeof(backupImgFilepath));
	if(ret == -1) {
		/* リンク先取得エラー */
		strcpy(filePath, BUS_BACKUP_IMG1_FILEPATH);
		return;
	}
	/* シンボリックリンクのリンク先がどちらのバックアップデータかチェック */
	if(strcmp(backupImgFilepath, BUS_BACKUP_IMG1_FILEPATH) == 0) {
		/* 画像データ②のファイルパスを出力 */
		strcpy(filePath, BUS_BACKUP_IMG2_FILEPATH);
	}
	else {
		/* 画像データ①のファイルパスを出力 */
		strcpy(filePath, BUS_BACKUP_IMG1_FILEPATH);
	}

	return;
}
#endif
static void syncBUS(size_t size)
{
#ifdef BUS_FOR_MBF
	int recoveryTime;
#endif
	/* ファイル書き込み処理を更新 */
	sync();

#ifdef BUS_FOR_MBF
	/* ファイル書き込み処理リカバリー時間を算出(セクタまたぎを考慮して "+ BUS_WAIT_TIME_PER_1SECTOR") */
	if(size <= MBF_SECTOR_SIZE) {
		recoveryTime = BUS_WAIT_TIME_PER_1SECTOR * 2;
	}
	else {
		recoveryTime = (((size / MBF_SECTOR_SIZE) * BUS_WAIT_TIME_PER_1SECTOR)		/* 書き込みサイズ分 */
						+ (size % MBF_SECTOR_SIZE ? BUS_WAIT_TIME_PER_1SECTOR : 0)) /* 端数サイズ考慮 */
						+ BUS_WAIT_TIME_PER_1SECTOR;								/* セクタまたぎ考慮分 */
	}

	/* ファイル書き込み処理処理のリカバリ時間をウェイト */
	SYSF_taskDelay(recoveryTime);
#endif
	return;
}

static Bool writeBSBackupIMG(const char *filePath, char *buf, size_t size)
{
	int fd;
	ssize_t ret;

	/* MBFに画像データ(実体)を作成する */
	fd = SYSF_open(filePath, (O_CREAT|O_WRONLY|O_TRUNC), 0666);
	if(fd == -1) {
		printf( "(%010u)(BSP) %s File generation error!!\n", SYSF_tickGet() , filePath);
		return False;
	}

	/* 画像データ(実体)を書き込む */
	ret = SYSF_write(fd, buf, size);
/*	if(ret == -1 && ret != size) {*/
	if(ret == -1 && ret != (ssize_t)size) {	/* 2013/09/18 Harada MinervaZeus3rd コンパイルワーニング除去対応 */
		printf( "(%010u)(BSP) %s File write error!!\n", SYSF_tickGet() , filePath);
		SYSF_close(fd);
		return False;
	}

	/* ファイルクローズ */
	SYSF_close(fd);

#ifdef BUS_FOR_MBF
	/* ファイル書き込み処理を更新 */
	syncBUS(size);
#endif
	/* 書き込んだ画像データのベリファイ */
	if(verifyBUS(filePath, buf, size) != True) {
		printf( "(%010u)(BSP) %s Verify error!!\n", SYSF_tickGet() , filePath);
		return False;
	}

#if 0
	/* シンボリックリンクの更新 */
	if(creBSSymLink(filePath) == False) {
		/* 更新失敗 */
		printf("%s シンボリックリンク更新失敗!!\n", filePath);
		return False;
	}
#endif
	/* BootUpScreen ファイル書き込み処理を更新 */
	syncBUS(size);

//	printf( "(%010u)(BSP) %s 書き込み成功!!\n", SYSF_tickGet() , BOOTUP_SCREEN_FILEPATH);

	return True;
}

Bool EraseBSBackupArea(void)
{
	STATUS ret;

	/* BootUpScreen ファイル削除 */
	ret = SYSF_unlink(BOOTUP_SCREEN_FILEPATH);
	if(ret != OK) {
		printf( "(%010u)(BSP) %s Deletion failure!! [%08x]\n", SYSF_tickGet() , BOOTUP_SCREEN_FILEPATH, ret);
		return False;
	}

	/* BootUpScreen ファイル書き込み処理を更新 */
	sync();
#ifdef BUS_FOR_MBF
	/* BootUpScreen ファイル削除処理処理のリカバリ時間をウェイト */
	SYSF_taskDelay(BUS_WAIT_TIME_PER_1SECTOR*2);
#endif
	if(checkEraseBUS() != True) {
		printf( "(%010u)(BSP) %s Delete error!!\n", SYSF_tickGet() , BOOTUP_SCREEN_FILEPATH);
		return False;
	}

	printf( "(%010u)(BSP) %s 削除成功!\n", SYSF_tickGet() , BOOTUP_SCREEN_FILEPATH);

	return True;
}

Bool setBootupScreen( void* addr, uint32_t size )
{
/*	char filePath[128]; */

	/* パラメータチェック */
	if((addr == NULL) || (size == 0)) {
		return False;
	}

	/* 書き込み先ファイルパスを取得 */
/*	getBSBackupStatus(filePath);*/

	/* 画像データを書き込み */
	if(writeBSBackupIMG((const char *)BOOTUP_SCREEN_FILEPATH, (char *)addr, size) == False) {
		/* 書き込み失敗 */
		return False;
	}

	return True;
}

/*================================================================================================
	\brief	BootupScreen画像データの読込.
	\param	なし.
	\result	uintptr_t	読込んだBootupScreen画像データのポインタ.
	\par	変更履歴.
			2013/03/08 【MP-BUS-01】日立JTE 国吉 BootupScreenバッファ非常駐化対応.
						BootupScreenバッファのオプション無効時はBootupScreen画像データの読込は行わない.
================================================================================================*/
uintptr_t getBootupScreen( void )
{
	int fd;
	int ret;
	ssize_t readRet;
	struct stat statInfo;

	/* 2013/03/08 【MP-BUS-01】日立JTE 国吉 BootupScreenバッファ非常駐化対応. */
	if( TYPE_AppOptSettingOff == SYSF_getAppOptionSetting(TYPE_AppOptionBootupScreen) ){
		return ((uintptr_t)NULL);
	}

	/* BootUpScreenファイルオープン */
	fd = SYSF_open(BOOTUP_SCREEN_FILEPATH, O_RDONLY, 0666);
	if(fd == -1){
		/* ファイルオープンエラーならBootUpScreenなしと判断する */
		printf( "(%010u)(BSP) %s There is none\n", SYSF_tickGet() , BOOTUP_SCREEN_FILEPATH);
		return ((uintptr_t)NULL);
	}

	/* BootUpScreenファイルサイズ取得のため */
	ret = SYSF_fstat(fd, &statInfo);
	if(ret != 0) {
		printf( "(%010u)(BSP) %s File status acquisition error\n", SYSF_tickGet() , BOOTUP_SCREEN_FILEPATH);
		SYSF_close(fd);
		return ((uintptr_t)NULL);
	}

	/* BootUpScreenを読み出して内部バッファへ格納 */
	readRet = SYSF_read(fd, pt_ALLOC_BootupScreen, (size_t)statInfo.st_size);
	if(readRet == -1) {
		SYSF_close(fd);
		printf( "(%010u)(BSP) %s Read error\n", SYSF_tickGet() , BOOTUP_SCREEN_FILEPATH);
		return ((uintptr_t)NULL);
	}

	/* BootUpScreenファイルクローズ */
	SYSF_close(fd);

//	printf( "(%010u)(BSP) %s 読み出し成功!!\n", SYSF_tickGet() , BOOTUP_SCREEN_FILEPATH);

	return ((uintptr_t)pt_ALLOC_BootupScreen);
}
#endif	/* #if0 -> BootUpScreen対応 adsol t.sasaki */
	char* getPuiTextPartsPointer(unsigned short hoge){return 0;}
	void SYSF_memShow(void){}
}

static char sc_MotionENStatus = -1; // MotionENの状態保存 (-1なら保存されていない) 
extern "C" char MotionENGetSignal(void){
	if( sc_MotionENStatus == -1 ){
		{
			int pmuFd;
			struct pmu_store_direct pmuInfo;
			char outputData[64];

			pmuFd = SYSF_open(THRD_DeviceFile_Pmu, O_RDWR, 0666);
			if (pmuFd != -1) {
				memset(outputData, 0, sizeof(outputData));
				pmuInfo.status = &outputData[0];
				pmuInfo.pmudev = THRE_StoreDirectDeviceIX_ENG_MORTION_ENABLE;
				if (ioctl(pmuFd, THRD_PMU_IOCSSFROMINPUT, &pmuInfo) == 0) {
					if (strncmp(outputData, "on", 2) == 0) {
						sc_MotionENStatus = 1;
					} else {
						sc_MotionENStatus = 0;
					}
				}
				SYSF_close(pmuFd);
			}
		}
    }
    return sc_MotionENStatus;
}

// ブラウザ関連 
extern "C" {
char gDNS_Addr[100];
int getCacheDeleteTiming(void){return 0;}
int getCookieDeleteTiming(void){return 0;}
int getAuthDeleteTiming(void){return 0;}
uint32_t getDynamicMultilangArea(void){return 0;}
void application_main(void){}
}

// UAC関連 
typedef struct r_cm_st R_CM;
typedef struct r_cm_ctx_st R_CM_CTX;
typedef struct r_cert_st R_CERT;
typedef struct r_cert_ctx_st R_CERT_CTX;
//2017/01/27 未使用フォルダ・ファイルの論理削除.
#if 0
int	CbStub_decode_cms(CNVS_CbParam* cbParam,CNVS_PdfData* cms,CNVS_PdfData* env){return 0;}
int	CbStub_sign_data(CNVS_CbParam* cbParam,int cmsType,int hashAlgorithm,CNVS_PdfData* toBeSigned,CNVS_PdfData* signedMessage){return 0;}
extern "C" {
int CreateEncData(CNVS_PdfData*,CNVS_PdfData*){return 0;}
void VerifySign(CNVS_PdfData* psignedData){}
}
#endif
/* bsp_donau\Donau\Mediaif.cpp */
Bool setMidAuthApplication(void* addr, uint32_t size)
{
	return FALSE;
}
uintptr_t getMidAuthApplication(void)
{
	return 0;
}

/* bsp_donau\Donau\usrUsbBulkDevInit.c */
Bool usrUsbBulkDriveEmpty(char *driveName)
{
	return TRUE;
}

void dspBootKind(void){}

// ↓↓↓ROM_DIV有効化対応 Start
//void _dbgLoadLog(void){}
// ↑↑↑ROM_DIV有効化対応 End

Bool chkIndex( uint32_t a, uint32_t* b, uint32_t* c, uint32_t d ){return 0;}

void _dbgExpLog(int sel){
//	drvrif_dbgExpLog(sel);
}

void _dbgIsExp(void) {
//	drvrif_dbgIsExp();
}

void _dbgCompLog(void) {
//	drvrif_dbgCompLog();
}

// 2013/01/09 H.Takahashi(CEC) SIMメモリ切替対応 Start.
/* 2018/05/22 ft_kondo SimLin不要コンパイルSW撲滅対応 */
#ifdef FUM_LINUX
// 2013/11/21 ADS坪井 CQ.381188(Act_68930)	サブメモリのファイルメモリ化 新SIM(1G)対応.
//											setSimPlatformSDRAMSize関数を戻り値を返却するgetSimPlatformSDRAMSize関数に変更し.
//											setSimPlatformSDRAMSize関数からはgetSimPlatformSDRAMSize関数を実行するように変更.
size_t getSimPlatformSDRAMSize( void )
{
	char *pEnvVal = NULL;
	
	size_t sdramSize = 0;
	// メモリサイズの環境変数を取得.
	pEnvVal = getenv( SIM_ENVVAL_MEMORYSIZE );
	if( NULL == pEnvVal ) {
		printf( "(%010u)(BSP) [setSimPlatformSDRAMSize]EnvVal Get Error\n", SYSF_tickGet()  );
		// デフォルト値(4GB)に設定.
		sdramSize = SIM_MEMORYSIZE_4GB;
	}
	else {
		// メモリサイズ判定
// 2017/11/01 Sky S800対応 メモリー構成が6Gまであるため判定方法変更.
// WinSimが4Gまでしか設定できないため、判定方法は暫定.
// WinSim 2G選択  ->  4Gと判断.
// WinSim 3G選択  ->  5Gと判断.
// WinSim 4G選択  ->  6Gと判断.
		switch ( *pEnvVal ) {
			case '1':			// 1GB.
				sdramSize = SIM_MEMORYSIZE_1GB;
				break;
			case '2':			// 2GB.
				sdramSize = SIM_MEMORYSIZE_4GB;
				break;
			case '3':			// 3GB.
				sdramSize = SIM_MEMORYSIZE_5GB;
				break;
			case '4':			// 4GB.
				sdramSize = SIM_MEMORYSIZE_6GB;
				break;
			default:			// エラー.
				printf( "(%010u)(BSP) [setSimPlatformSDRAMSize]Memory Size Judge Error : %s \n", SYSF_tickGet() ,pEnvVal  );
				// 4GBに設定.
				sdramSize = SIM_MEMORYSIZE_6GB;
				break;
		}
	}

	return sdramSize;
}

void setSimPlatformSDRAMSize( void )
{
	sdramSize = getSimPlatformSDRAMSize();
}
#endif // #ifdef SIMULATOR.
// 2013/01/09 H.Takahashi(CEC) SIMメモリ切替対応 End.

extern "C" {
    Bool isIRPowerONNeed();	// IR電源ON要否判定(スレッド提供用API) 
}
void platformInitialize( void )
{
//	THRE_MEMORY_BANK mb;

    CB_BitPortN<THRD_Port_DataType>::newInstance(CBBPTN_portPCIRST_CPS) = 0;

	if (isIRPowerONNeed()) {
        ddOutPort_IrPowerOn_setActive();
        ddOutPort_IrOEN_setActive();
        CB_BitPortN<THRD_Port_DataType>::newInstance(CBBPTN_portIRC_RESET) = 0;			// IRCリセット解除 
	}

#ifndef FUM_LINUX
	/* DIMM総メモリサイズを取得 honda */
	THRE_MEMORY_BANK mb;
	sdramSize = 0;
	for( mb = THRE_MEM_BANK0; mb <= THRE_MEM_BANK3; mb++ ){
		sdramSize += (size_t)THRL_Drvrif_GetMainMemoryBankSize( mb );
	}
// 2013/01/09 H.Takahashi(CEC) SIMメモリ切替対応 Start.
#else // #ifndef FUM_LINUX.
	setSimPlatformSDRAMSize();
// 2013/01/09 H.Takahashi(CEC) SIMメモリ切替対応 End.
#endif // #ifndef FUM_LINUX.
}

void setPlatformSDRAMSize( void )
{
#ifndef FUM_LINUX
	THRE_MEMORY_BANK mb;
	sdramSize = 0;
	for( mb = THRE_MEM_BANK0; mb <= THRE_MEM_BANK3; mb++ ){
		sdramSize += (size_t)THRL_Drvrif_GetMainMemoryBankSize( mb );
	}
// 2013/01/09 H.Takahashi(CEC) SIMメモリ切替対応 Start.
#else // #ifndef FUM_LINUX.
	setSimPlatformSDRAMSize();
// 2013/01/09 H.Takahashi(CEC) SIMメモリ切替対応 End.
#endif // #ifndef FUM_LINUX.
}

#define THRD_COMMANDLENGTH 256
#define THRD_RINNE_MAX_FILE_NUM 20
#define THRD_RINNE_TERM_LOG_PATH "/ata0a/TerminalLog/"
// ================================================================================================
//! <!--機能--> Rinneログ保存機能ファイルNo.取得. 
//! <!---------------------------------------------------------------------------------------------
//! <!--引数-->
//! \param sc_vspath ディレクトリ.
//! <!--戻値-->
//! \return uchar ファイルNo.
//! <!--詳細--> ディレクトリ内のファイル名の最大のものの値を返す。 
//! <!--履歴--> 22/09/27 Sky 新規作成.
// ================================================================================================
uchar getRinneFileNo( const char sc_vspath[1023] )
{
	DIR* ps_Dir = NULL;
	struct dirent* ps_GetFile = NULL;
	std::vector<uchar> vec;
	
	// ディレクトリ内のファイル検索.
	ps_Dir = SYSF_Opendir( const_cast<char*>(sc_vspath) );
	if( NULL == ps_Dir ){
		printf("[getFileNo]cannot SYSF_Opendir.\n");
		return 0;
	}
	// ディレクトリ内のファイル名取得.
	while( NULL != (ps_GetFile = SYSF_Readdir( ps_Dir )) ){
		if ( 0 == strcmp( ps_GetFile->d_name, ".." ) || 0 == strcmp( ps_GetFile->d_name, "." ) ){
			continue;
		}
		vec.push_back(atoi(ps_GetFile->d_name));
	}
	SYSF_Closedir(ps_Dir);
	
	if( vec.empty() ){
		return 0;
	}
	
	// 不要ログファイル削除.
	if( vec.size() > THRD_RINNE_MAX_FILE_NUM ){
		uchar uc_cnt = vec.size() - THRD_RINNE_MAX_FILE_NUM;
		// 昇順に並び替え.
		sort(vec.begin(), vec.end());
		char sc_Command[THRD_COMMANDLENGTH] = {0};          // 実行するシステムコール.
		// 不要個数分のファイル削除.
		for(int i = 0; i < uc_cnt; ++i) {
			printf( "[getFileNo]:rm -f %s%d.log.\n", THRD_RINNE_TERM_LOG_PATH, vec[i] );
			snprintf( sc_Command, THRD_COMMANDLENGTH, "rm -f %s%d.log", THRD_RINNE_TERM_LOG_PATH, vec[i] );
			int si_Result = THRF_System( sc_Command );
			if( 0 != si_Result ){
				printf("[getFileNo]cannot rm -f %s%d.log.\n", THRD_RINNE_TERM_LOG_PATH, vec[i] );
			}
		}
		THRF_DoSync();
	}
	
	return vec.back();
}

// ================================================================================================
//! <!--機能--> Rinneログ保存機能初期化. 
//! <!---------------------------------------------------------------------------------------------
//! <!--引数-->
//! \param なし.
//! <!--戻値-->
//! \return なし.
//! <!--詳細--> Rinneログ保存機能の初期化処理を行う。 
//! <!--履歴--> 22/09/27 Sky 新規作成.
// ================================================================================================
void initRinneLog(void)
{
	THRS_RinneInfoArg s_RinneInfoArg = {0};
	
	s_RinneInfoArg.uc_FileNo = 1;
	// ディレクトリ存在チェック.
	if( access( THRD_RINNE_TERM_LOG_PATH, F_OK ) == 0 ) {
		// Rinneログ保存機能ファイルNo.取得. 
		s_RinneInfoArg.uc_FileNo = getRinneFileNo( THRD_RINNE_TERM_LOG_PATH ) + 1;
	}else{
		// ディレクトリ作成.
		if( SYSF_mkdir( THRD_RINNE_TERM_LOG_PATH ) == ERROR ){
			printf("[initRinneLog]SYSF_mkdir failed!!\n");
			return;
		}
	}
	
	// ログ収集開始. 
	THRL_ReqRinneLog( THRD_COM_DBG_START_RINNE_LOG, (void*)&s_RinneInfoArg );
}

// ↓↓↓ROM_DIV有効化対応 Start
// 2010/12/24 iyanagi ラインカード対応 start.
//extern "C" unsigned char isLineMode()
//{
//	return 0;
//}
#include "Linux/Platform/TarLib.h"
void THRF_CheckLineCardMode();// LineMode用のLAN設定

extern uchar uc_BootMode;
Bool bl_LineInitFlag = False;
// ラインモードフラグ.
uchar uc_isLineMode = 0 ;

// CQ.338626 koshimo 暫定対策.
// ラインカードの検出処理がPanelより遅く動作しているので
// ラインカードとして認識されないのが原因
// 暫定として検出処理を早く動作させる為、checkLineModeInTarをexternとする。
extern "C" void checkLineModeInTar();

// ================================================================================================
//! <!--機能--> ラインカード検出処理 
//! <!---------------------------------------------------------------------------------------------
//! <!--引数-->
//! \param なし 
//! <!--戻値-->
//! \return なし
//! <!--詳細--> USBメモリ/CFの中に統合パターンがあるかを確認する。setBootUpKind内からのみ呼ぶ。 
//! <!--履歴--> 10/12/24 S.Iyanagi MinervaZeus 新規作成 
//!				11/01/27 ft_kondo  CQ337792 ラインカードモードで起動しない問題. 
//!             11/03/15 M.Koshimo CQ.338626暫定対応. 
//!             11/09/02 第1OSW 浦崎 CQ.354603「ラインカード起動で所定のIPアドレスにならない」対応. 
//!				18/07/30 Sky松井	[Eagle][AR.1053817]CommonAPIへラインカード起動かの判定フラグを設定する処理を追加.
//!             22/09/27 Sky 生産RINNEログ保存対応.
//!             22/10/31 Sky Rinne自動画像調整対応.
// ================================================================================================
void checkLineModeInTar()
{
	Bool bl_LineModeFlg = False;	// ラインモード起動設定.
	// 実行モードがラインカードモードの時.
	if( 7 == uc_BootMode ){// 起動種別：ラインカード起動.
// 2011/05/11 Harada CQ.343963 ラインカードを挿入して起動した際、ラインモードで起動しない >>.
// ラインカードモード判断について、すでにBoot起動時に判断しており起動種別変数に設定されて.
// いるので起動種別のみで判断するように変更.
//		// tarファイルを開く
//		unsigned char uc_Tar = TarOpen( );
//		if( TAR_STS_OK == uc_Tar ){
//			// tarファイル上のINDEXファイルの再チェック
//			unsigned long	ulSectorNum = 0, ulSectorCount = 0;
//			uchar uc_chk = TarIndexCheck( e_ImgeType_XYZ );
//			if( !uc_chk ){
//				// 統合パターンイメージがあるかどうかをチェックする
//				if (   ( !TarFileCheck( e_ImgeType_A3T, &ulSectorNum, &ulSectorCount ) )
//					&& ( !TarFileCheck( e_ImgeType_WLT, &ulSectorNum, &ulSectorCount ) )
//					&& ( !TarFileCheck( e_ImgeType_A4Y, &ulSectorNum, &ulSectorCount ) )
//					&& ( !TarFileCheck( e_ImgeType_LTY, &ulSectorNum, &ulSectorCount ) ) ) {
					// ラインモードフラグON.
					uc_isLineMode = 1 ;
					bl_LineModeFlg = True;	// ラインカード起動時はTrueをセットする.
					comSystem.bl_FactoryAdjustMode = True;	// 11/01/27 ft_kondo CQ337792. 
					// 11/09/02 第1OSW 浦崎 CQ.354603「ラインカード起動で所定のIPアドレスにならない」対応. 
					if( !bl_LineInitFlag ){	/* ラインモードの判定がされたかどうかを確認 */ 
						// ここでcheckLineModeInTarでコメントアウトしたフラグをONにする.
						bl_LineInitFlag = True;
						printf( "LineMode = %d\n" , uc_isLineMode );
// 2022/03/30 生産RINNE対応 Sky start.
						APIC_SystemInfo::newInstance()->setCardLineModeVer( TYPE_LineModeVer_1st );
						// USBの中から設定ファイルを読み込む。NICに設定するIPも含むためTHRF_CheckLineCardMode()の前に実行すること.
#ifndef SIMULATOR
						// Rinneの設定ファイルの有無を確認するためUSBのマウント待ち.
						for(int64_t i=0;i<60;i++){
							sleep(1);
							{
								const int i_mntRet71837 = system("mount /dev/sdc1 /bd/sdc1");	// [OP_BTS-71837] keep the result
								printf("OP_BTS-71837: checkLineModeInTar mount /dev/sdc1 /bd/sdc1 try=%d ret=%d\n", (int)i, i_mntRet71837);
							}
							if( 0 == access( "/dev/sdc1" , F_OK ) ) {
								system("grep ' /bd/sdc1 ' /proc/mounts");	// [OP_BTS-71837] options prove who created the mount
								break;
							}
						}
#endif

						if( True == THRF_ReadLineModeSettingFile() ) {
							APIC_SystemInfo::newInstance()->setCardLineModeVer( TYPE_LineModeVer_2nd );
							// Rinneログ取得開始.
							initRinneLog();
							// 2022/10/31 生産RINNE自動画像調整 Sky start.
							// Rinneモジュール初期化.
							APIC_Adjust::newInstance()->initClass();
							// 2022/10/31 生産RINNE自動画像調整 Sky end.
						}
// 2022/03/30 生産RINNE対応 Sky end.
						THRF_CheckLineCardMode();// nicfumが対応した後で有効化する.
					}
//				}
//			}
//			// tarファイルを閉じる
//			TarClose( );
//		}
// 2011/05/11 Harada CQ.343963 ラインカードを挿入して起動した際、ラインモードで起動しない <<.
	}
// CQ.338626 koshimo 暫定対策.
// 暫定でラインカード検出処理は早く動作させるが.
// ラインカード判定処理実施確認のフラグはここでは立てない.
// 理由：本関数コール後にisLineModeが実行される為.
// その為、ここではコメントアウトする.
//	bl_LineInitFlag = True;
//	printf( "LineMode = %d\n" , uc_isLineMode );
	
	// ラインカード起動指定を設定.
	APIC_SystemInfo* pc_SystemInfo = APIC_SystemInfo::newInstance();
	if( pc_SystemInfo != NULL ){
		pc_SystemInfo->setCardLineMode_ForSharedMem( bl_LineModeFlg );
	}else{	// インスタンス生成に失敗したらログだけ残す.
		printf("[checkLineModeInTar]pc_SystemInfo is NULL.\n");
	}
}

// ================================================================================================
//! <!--機能--> ラインモードフラグの取得 
//! <!---------------------------------------------------------------------------------------------
//! <!--引数-->
//! <!--戻値--> uc_LineMode 1:Lineモード 0:通常起動 
//! <!--詳細--> True or Falseを返す BSPから呼ぶことも考慮してBoolは使用しない 
//! <!--履歴--> 10/12/24 S.Iyanagi MinervaZeus 新規作成 
//!             11/03/15 M.Koshimo CQ.338626暫定対応. 
//!             11/09/02 第1OSW 浦崎 CQ.354603「ラインカード起動で所定のIPアドレスにならない」対応. 
// ================================================================================================
extern "C" unsigned char isLineMode()
{
// CQ.338626 koshimo 暫定対策.
#if 1
//11/09/02 第1OSW 浦崎 CQ.354603.以下の処理をcheckLineModeInTar()に移動(mioInit()よりも先に行う必要があるため). 
//	if( !bl_LineInitFlag ){	/* ラインモードの判定がされたかどうかを確認 */ 
//		// ここでcheckLineModeInTarでコメントアウトしたフラグをONにする.
//		bl_LineInitFlag = True;
//		printf( "LineMode = %d\n" , uc_isLineMode );
//		THRF_CheckLineCardMode();// nicfumが対応した後で有効化する.
//	}
#else	// 正規の処理
	if( !bl_LineInitFlag ){	/* ラインモードの判定がされたかどうかを確認 */ 
		checkLineModeInTar();
		THRF_CheckLineCardMode();// nicfumが対応した後で有効化する.
	}
#endif
	return uc_isLineMode;
}
// 2010/12/24 iyanagi ラインカード対応 end.
// ↑↑↑ROM_DIV有効化対応 End

// ROMバージョンの設定.
char* GetSysRomVersion(char* psc_buf)
{
	int    si_Fd = -1;
	ssize_t si_Ret = -1;
	char*    psc_Ret = psc_buf;

	memset(psc_buf, '\0', 40);

	si_Fd = SYSF_open("/boot/version", O_RDONLY, 0444);
	if(si_Fd != -1) {
		si_Ret = SYSF_read(si_Fd, psc_buf, 40);
	}
	else {
		si_Fd = SYSF_open("/mfp/version", O_RDONLY, 0444);
		if(si_Fd != -1) {
			si_Ret = SYSF_read(si_Fd, psc_buf, 40);
		}
	}
	psc_buf[39] = '\0';

	if(si_Ret == -1) {
		psc_Ret = NULL;
	}

	if(si_Fd != -1) {
		SYSF_close(si_Fd);
	}

	return psc_Ret;
}

// Bootシーケンス診断モードフラグ.
uchar uc_isBootSelfCheckMode = 0;
extern "C" void checkBootSelfCheckMode();
// ================================================================================================
//! <!--機能--> Bootシーケンス自己診断モード検出処理.
//! <!---------------------------------------------------------------------------------------------
//! <!--引数-->
//! \param なし.
//! <!--戻値-->
//! \return なし.
//! <!--詳細--> 
//! <!--履歴--> 12/05/29 N.Kato          新規作成.
// ================================================================================================
void  checkBootSelfCheckMode()
{
	// 実行モードがBootシーケンス診断モードの時.
	if( 9 == uc_BootMode ){// 起動種別：Bootシーケンス診断.
		uc_isBootSelfCheckMode = 1;
	}
}

// 2011/11/30 Gonsho Android ブザー・音声再生対応 Start.
#ifdef __cplusplus
extern "C" {
#endif
// ================================================================================================
//! <!--機能--> 音声再生要求通知処理. 
//! <!---------------------------------------------------------------------------------------------
//! <!--引数--> int32_t sl_SeqNo シーケンシャルNo.
//!             char* psc_FileName 再生ファイル名.
//! <!--戻値--> なし.
//! <!--詳細--> Android端末へ音声再生要求を通知する.
//! <!--履歴--> 2011/11/30 Gonsho 新規作成.
// ================================================================================================
void reqVoicePlay( const int32_t sl_SeqNo, const char* psc_FileName )
{
	// AndroidIF 音声再生要求処理 呼び出し.
	THRC_AndroidControlIf* pc_AndroidIf = THRC_AndroidControlIf::getInstance();
	pc_AndroidIf->THRL_AndroidReqVoicePlay( sl_SeqNo, psc_FileName );
}

// ================================================================================================
//! <!--機能--> 音声停止要求通知処理. 
//! <!---------------------------------------------------------------------------------------------
//! <!--引数--> なし.
//! <!--戻値--> なし.
//! <!--詳細--> Android端末へ音声再生停止要求を通知する.
//! <!--履歴--> 2011/11/30 Gonsho 新規作成.
// ================================================================================================
void reqVoiceStop()
{
	// AndroidIF 音声停止要求通知処理 呼び出し.
	THRC_AndroidControlIf* pc_AndroidIf = THRC_AndroidControlIf::getInstance();
	pc_AndroidIf->THRL_AndroidReqVoiceStop();
}

// ================================================================================================
//! <!--機能--> ブザー・音楽再生要求通知処理. 
//! <!---------------------------------------------------------------------------------------------
//! <!--引数--> unsigned char uc_State		再生・停止区分
//!             unsigned char uc_Kind		再生する反応音(小分類).
//!             unsigned char uc_Select		反応音選択種類.
//!             unsigned char uc_Vol		反応音ボリューム.
//!             short ss_IdNo				カスタムID番号(再生する反応音のファイルを指定する為のID番号で、反応音選択種類がカスタム時のみ有効な数値).
//! <!--戻値--> なし.
//! <!--詳細--> Android端末へブザー・音楽再生停止要求を通知する.
//! <!--履歴--> 2011/11/30 Gonsho 新規作成.
//!             2011/12/22 Gonsho 引数を追加.
// ================================================================================================
void reqMusicPlay( 
		const unsigned char uc_State, const unsigned char uc_Kind, const unsigned char uc_Select, 
		const unsigned char uc_Vol, const short ss_IdNo, const unsigned char uc_TestPlayFlag )
{
	// AndroidIF ブザー・音楽再生要求通知処理 呼び出し.
	THRC_AndroidControlIf* pc_AndroidIf = THRC_AndroidControlIf::getInstance();
	pc_AndroidIf->THRL_AndroidReqBuzzerMusicPlay( uc_State, uc_Kind, uc_Select, uc_Vol, ss_IdNo, uc_TestPlayFlag );
}

// ================================================================================================
//! <!--機能--> 再生ファイル情報(リスト)取得要求通知処理. 
//! <!---------------------------------------------------------------------------------------------
//! <!--引数--> char* psc_MfpName MFP名称.
//! <!--戻値--> なし.
//! <!--詳細--> Android端末へ音声ファイル情報(リスト)取得要求を通知する.
//! <!--履歴--> 2011/11/30 Gonsho 新規作成.
// ================================================================================================
void reqMusicFileList( const char* psc_MfpName )
{
	// AndroidIF 再生ファイル情報(リスト)取得要求通知処理 呼び出し.
	THRC_AndroidControlIf* pc_AndroidIf = THRC_AndroidControlIf::getInstance();
	pc_AndroidIf->THRL_AndroidReqVoiceList( psc_MfpName );
}

// ================================================================================================
//! <!--機能--> 音声カスタマイズファイル設定要求通知処理. 
//! <!---------------------------------------------------------------------------------------------
//! <!--引数--> short ss_CustomId			カスタムID.
//! <!--戻値--> なし.
//! <!--詳細--> Android端末へ音声カスタマイズファイル設定要求を通知する.
//! <!--履歴--> 2011/12/14 Gonsho 新規作成.
// ================================================================================================
void reqMusicCustomIdSet( const short ss_CustomID )
{
	// AndroidIF 音声カスタマイズファイル設定要求通知処理 呼び出し.
	THRC_AndroidControlIf* pc_AndroidIf = THRC_AndroidControlIf::getInstance();
	pc_AndroidIf->THRL_AndroidReqBuzzerCustom( ss_CustomID );
}

// ================================================================================================.
//! <!--機能--> 接続タイムアウト時間設定要求通知処理. 
//! <!---------------------------------------------------------------------------------------------.
//! <!--引数--> short ss_Timeout	接続タイムアウト時間.
//! <!--戻値--> なし.
//! <!--詳細--> Android端末へタイムアウト時間設定要求を通知する.
//! <!--履歴--> 2012/02/22 k.morita CQ.359305対応 新規作成.
// 				2012/03/07 hanaoka	CQ362658 NVRAMクリアでFA14対策.
// ================================================================================================.
void reqConnectTimeoutSet( const short ss_Timeout )
{
	// AndroidIF 接続タイムアウト時間設定要求通知処理 呼び出し.
	THRC_AndroidControlIf* pc_AndroidIf = THRC_AndroidControlIf::getInstance();
	if( True == THRC_AndroidControl::getInstance()->THRG_IsConnectAndroid() ){
		pc_AndroidIf->THRL_AndroidReqConnectTimeoutSet( ss_Timeout );
	}
}

// ================================================================================================.
//! <!--機能--> IME文字列情報送信通知処理. 
//! <!---------------------------------------------------------------------------------------------.
//! <!--引数--> char* psc_CommittedTxt	確定文字列.
//!				int32_t sl_Pos				カーソル位置.
//!				int32_t sl_SeqNo			シーケンスNo.
//! <!--戻値--> なし.
//! <!--詳細--> IME文字列情報の送信通知処理を行う.
//! <!--履歴--> 2012/11/20 OPS1川手 [操作性改善FUM4] 手書き文字入力対応 新規作成.
// ================================================================================================.
void reqSendIMETxt( const char* psc_CommittedTxt, const int32_t sl_Pos, const int32_t sl_SeqNo )
{
	THRC_AndroidControlIf* pc_AndroidIf = THRC_AndroidControlIf::getInstance();
	pc_AndroidIf->reqSendIMETxt( psc_CommittedTxt, sl_Pos, sl_SeqNo );
}

// ================================================================================================.
//! <!--機能--> IME状態設定要求通知処理. 
//! <!---------------------------------------------------------------------------------------------.
//! <!--引数--> Bool bl_State			状態（ON/OFF）.
//!				char* psc_CommittedTxt	確定文字列.
//!				int32_t sl_Pos				カーソル位置.
//!				int32_t sl_SeqNo			シーケンスNo.
//!				Bool bl_Password		伏字文字フラグ.
//! <!--戻値--> なし.
//! <!--詳細--> IMEの状態設定通知処理を行う.
//! <!--履歴--> 2012/11/20 OPS1川手 [操作性改善FUM4] 手書き文字入力対応 新規作成.
//!             2013/10/07 y.chiba  [操作性改善FUM5] IME連携対応 伏字文字フラグ 追加.
// ================================================================================================.
void reqSetIMEState( const Bool bl_State, const char* psc_CommittedTxt, const int32_t sl_Pos, const int32_t sl_SeqNo, const Bool bl_Password )
{
	THRC_AndroidControlIf* pc_AndroidIf = THRC_AndroidControlIf::getInstance();
	pc_AndroidIf->reqSetIMEState( bl_State, psc_CommittedTxt, sl_Pos, sl_SeqNo, bl_Password );
}

// ================================================================================================.
//! <!--機能--> IME文字列情報取得処理. 
//! <!---------------------------------------------------------------------------------------------.
//! <!--引数--> char* psc_IMETxtInfo	IME文字列情報.
//! <!--戻値--> なし.
//! <!--詳細--> IMEの状態設定通知処理を行う.
//! <!--履歴--> 2012/11/20 OPS1川手 [操作性改善FUM4] 手書き文字入力対応 新規作成.
// ================================================================================================.
void getRecvIMETxt( char* psc_IMETxtInfo )
{
	THRC_AndroidControlIf* pc_AndroidIf = THRC_AndroidControlIf::getInstance();
	pc_AndroidIf->getRecvIMETxt( psc_IMETxtInfo );		// IME文字列情報.
}

// ================================================================================================.
//! <!--機能--> IME文字列情報バッファ取得処理. 
//! <!---------------------------------------------------------------------------------------------.
//! <!--引数--> なし.
//! <!--戻値--> char* 	IME文字列情報バッファアドレス.
//! <!--詳細--> IME文字列情報バッファアドレスを取得する.
//!				一度取得したポインタを、バッファのアドレスとして保持し続け、その値を返す.
//! <!--履歴--> 2012/10/04 OPS1川手 [操作性改善FUM4] 手書き文字入力対応 新規作成.
// ================================================================================================.
char* getIMETxtBuf()
{
	static char* psc_Buf = NULL;
	if ( NULL == psc_Buf ) {	// 未取得の場合.
		THRC_AndroidControlIf* pc_AndroidIf = THRC_AndroidControlIf::getInstance();
		psc_Buf = pc_AndroidIf->getIMETxtBuf();
	}
	return psc_Buf;				// 1度取得したアドレスを返す.
}

// ================================================================================================.
//! <!--機能--> アドレス帳情報取得完了通知処理.
//! <!---------------------------------------------------------------------------------------------.
//! <!--引数--> なし.
//! <!--戻値--> なし.
//! <!--詳細--> アドレス帳情報を取得したことを通知する.
//! <!--履歴--> 2014/09/15 OPS1鈴木 [操作性改善FUM5] アドレス帳連携 新規作成.
// ================================================================================================.
void notifyAddresInfoConmplete( const int32_t sl_SeqNo, const int32_t sl_Result )
{
	THRC_AndroidControlIf* pc_AndroidIf = THRC_AndroidControlIf::getInstance();
	pc_AndroidIf->notifyAddresInfoConmplete( sl_SeqNo, sl_Result );
}

// ================================================================================================.
//! <!--機能--> タブレット端末切断処理. 
//! <!---------------------------------------------------------------------------------------------.
//! <!--引数--> なし.
//! <!--戻値--> なし.
//! <!--詳細--> タブレット端末のと接続を切断.
//! <!--履歴--> 2012/10/12 k.nakada 新規作成.
// ================================================================================================.
void disconnectTablet()
{
	THRC_AndroidControlIf* pc_AndroidIf = THRC_AndroidControlIf::getInstance();
	pc_AndroidIf->THRL_AndroidDisconnect();
}

// ================================================================================================.
//! <!--機能--> 本体TP、タブレット端末 双方表示処理. 
//! <!---------------------------------------------------------------------------------------------.
//! <!--引数--> True:双方表示 False:タブレット端末のみ表示.
//! <!--戻値--> なし.
//! <!--詳細--> 本体TP、タブレット端末双方表示するか、タブレット端末のみ表示をするかを指定.
//! <!--履歴--> 2012/10/18 s.ohba		新規作成.
// 				2013/01/10 k.komaba		[操作性改善FUMv4]Single⇔Dual表示切換対応.
// 				2013/03/01 S.Iyanagi	[操作性改善FUMv4]Single⇔Dual表示切換の通知先変更 CT:No135,136対策.
// ================================================================================================.
void setDualDisplay( const Bool bl_DualFlg )
{
	APIE_DisplayMode e_SetDisplayMode = APIE_DisplayModeTablet;		// 表示モード 「Tablet表示」.
	if( True == bl_DualFlg ){										// 「双方表示」指定.
		e_SetDisplayMode = APIE_DisplayModeDual;					// 表示モード 「双方表示」.
	}
	THRC_AndroidControlIf* pc_AndroidIf = THRC_AndroidControlIf::getInstance();
	if( NULL != pc_AndroidIf ){
		pc_AndroidIf->changeDisplayMode( e_SetDisplayMode );		// 表示モード変更.
	}
}

// ================================================================================================.
//! <!--機能--> タブレット端末接続時 双方表示設定処理. 
//! <!---------------------------------------------------------------------------------------------.
//! <!--引数--> True:タブレット端末接続時に双方表示 False:タブレット端末接続時にTablet表示.
//! <!--戻値--> なし.
//! <!--詳細--> 本体TP、タブレット端末双方表示するか、タブレット端末のみ表示をするかを指定.
//! <!--履歴--> 2013/04/25	OPS1本橋	[操作性改善FUMVer4][CQ378881] 新規作成.
// ================================================================================================.
void setConnectDualDisplay( const Bool bl_ConnectDual )
{
	APIE_DisplayMode e_SetDisplayMode = APIE_DisplayModeTablet;		// 表示モード 「Tablet表示」.
	if( True == bl_ConnectDual ){									// 「双方表示」指定.
		e_SetDisplayMode = APIE_DisplayModeDual;					// 表示モード 「双方表示」.
	}
	THRC_AndroidControlIf* pc_AndroidIf = THRC_AndroidControlIf::getInstance();
	if( NULL != pc_AndroidIf ){
		pc_AndroidIf->setConnectDisplayMode( e_SetDisplayMode );	// 端末接続時表示モード設定.
	}
}

// ================================================================================================.
//! <!--機能--> 端末接続制限設定処理.
//! <!---------------------------------------------------------------------------------------------.
//! <!--引数--> Bool bl_Restrict		接続制限有無.
//! <!--引数--> Bool bl_ConnectState	接続制限設定時の端末接続状態.
//! <!--戻値--> なし.
//! <!--詳細--> IMEの状態設定通知処理を行う.
//! <!--履歴--> 2013/06/13 OPS1井柳 [操作性改善FUMVer4][CQ381889] 新規作成.
// ================================================================================================.
void setConnectRestrict( const Bool bl_Restrict, const Bool bl_ConnectState )
{
	THRC_AndroidControlIf* pc_AndroidIf = THRC_AndroidControlIf::getInstance();
	if( NULL != pc_AndroidIf ){
		pc_AndroidIf->setConnectRestrict( bl_Restrict, bl_ConnectState );
	}
}

// ================================================================================================.
//! <!--機能--> 接続確認結果設定処理.
//! <!---------------------------------------------------------------------------------------------.
//! <!--引数--> Bool bl_Result		接続確認結果（True：操作許可、False：操作拒否）.
//! <!--戻値--> なし.
//! <!--詳細--> 接続確認結果設定処理を行う.
//! <!--履歴--> 2014/08/28 OPS1井柳 [ZeusS][UV-01448] 新規作成.
// ================================================================================================.
void setConnectConfirmResult( const Bool bl_Result )
{
	THRC_AndroidControlIf* pc_AndroidIf = THRC_AndroidControlIf::getInstance();
	if( NULL != pc_AndroidIf ){
		pc_AndroidIf->setConnectConfirmResult( bl_Result );
	}
}
// 2015/1/23 UI2スケーラブル対応 MSE 諸岡 Add Start.
// ================================================================================================.
//! <!--機能--> .
//! <!---------------------------------------------------------------------------------------------.
//! <!--引数--> ushort resx	解像度X.
//! <!--引数--> ushort resy	解像度Y.
//! <!--戻値--> なし.
//! <!--詳細--> 解像度変更.
//! <!--履歴--> 2015/01/23 新規作成.
// ================================================================================================.
void changeResolutionDisplay( const ushort us_resx, const ushort us_resy )
{
	THRC_AndroidControl::getInstance()->changeResolutionDisplay( us_resx, us_resy );
}
// 2015/1/23 UI2スケーラブル対応 MSE 諸岡 Add End.

// ================================================================================================.
//! <!--機能--> ScanToAndroidのジョブ情報を、bizhubRA端末へ送信する.
//! <!---------------------------------------------------------------------------------------------.
//! <!--引数-->  const THRE_FileFormat				e_FileFormat		ファイルフォーマット.
//! <!--引数-->  const THRE_OCR						e_Ocr				OCR実行有無.
//! <!--引数-->  const THRE_Save					e_Save				保存先.
//! <!--引数-->  const THRE_Resolution				e_Resoltion			解像度.
//! <!--引数-->  const THRE_Read					e_Read				読み込み先.
//! <!--引数-->  const THRE_Color					e_Color				カラー設定
//! <!--引数-->  const THRE_ImageQuality			e_ImageQuality		原稿画質.
//! <!--引数-->  const THRE_ImageSide				e_ImageSide			片面/両面原稿設定.
//! <!--引数-->  const THRE_RemoveBackGroundMode	e_RemoveGroundMode	下地色除去モード.
//! <!--引数-->  const THRE_RemoveBackGroundLevel	e_RemoveGroundLevel	下地色除去レベル.
//! <!--引数-->  const THRE_OriginalPlace			e_OriginalPlace		原稿方向指定.
//! <!--引数-->  const ushort						us_PaperSizeCode	用紙サイズコード※用紙サイズコードは、""02_基本的な操作動作(用紙の定義)""の「用紙コード」欄を参照.
//! <!--引数-->  const char*						psc_OutputFileName	出力ファイル名 ※ 文字列長は、出力ファイル名長（THRD_OutputFileNameLen（NULL文字含む)）とする.
//! <!--戻値--> 無し.
//! <!--詳細--> ScanToAndroidのジョブ情報を、bizhubRA端末へ送信する.
//! <!--注意--> パラメタ数が多い理由は、ヘッダにて構造体渡しにした場合、原因不明のコンパイルエラーが.
//! 			発生するため、やむを得ず個別設定するようにしての対応となっている.
//! <!--履歴--> 2014/09/16 OPS1井柳 新規作成.
// ================================================================================================.
void sendScanToAndroidJobData(
 const THRE_FileFormat e_FileFormat,
 const THRE_OCR e_Ocr,
 const THRE_Save e_Save,
 const THRE_Resolution e_Resoltion,
 const THRE_Read e_Read,
 const THRE_Color e_Color,
 const THRE_ImageQuality e_ImageQuality,
 const THRE_ImageSide e_ImageSide,
 const THRE_RemoveBackGroundMode e_RemoveGroundMode,
 const THRE_RemoveBackGroundLevel e_RemoveGroundLevel,
 const THRE_OriginalPlace e_OriginalPlace,
 const ushort us_PaperSizeCode,
 const char* psc_OutputFileName )
{
	THRC_AndroidControlIf* pc_AndroidIf = THRC_AndroidControlIf::getInstance();
	if( NULL != pc_AndroidIf ){
		pc_AndroidIf->sendScanToAndroidJobData(
			e_FileFormat,
			e_Ocr,
			e_Save,
			e_Resoltion,
			e_Read,
			e_Color,
			e_ImageQuality,
			e_ImageSide,
			e_RemoveGroundMode,
			e_RemoveGroundLevel,
			e_OriginalPlace,
			us_PaperSizeCode,
			psc_OutputFileName );
	}
}

// ================================================================================================.
//! <!--機能--> 画面データ送信要求処理.
//! <!---------------------------------------------------------------------------------------------.
//! <!--引数--> Bool bl_Trans		画面データ転送指定（True:転送する、False:転送しない）.
//! <!--引数--> Bool bl_Dual		双方向表示指定（True:双方表示、False:タブレット端末のみ表示）.
//! <!--戻値--> なし.
//! <!--詳細--> 画面データ送信要求処理を行う.
//! <!--履歴-->.
//! 2015/10/15 OPS1前田 [MinervaSBK][CQ514710] 新規作成.
//! 2017/09/19 Sky井柳 [Eagle][制御構成変更No10] IT4以前機種実行制限（コンパイルスイッチ）削除.
// ================================================================================================.
void reqScreenDataTrans( const Bool bl_Trans, const Bool bl_Dual )
{
	THRC_AndroidControlIf* pc_AndroidIf = THRC_AndroidControlIf::getInstance();
	if( NULL != pc_AndroidIf ){
		pc_AndroidIf->reqScreenDataTrans( bl_Trans, bl_Dual );
	} else {
		SYSF_logMsg( "(%010u)(BSP) reqScreenDataTrans pc_AndroidIf is NULL \n", SYSF_tickGet() );
	}
}

// ================================================================================================.
//! <!--機能--> 確認画面表示結果設定処理.
//! <!---------------------------------------------------------------------------------------------.
//! <!--引数--> Bool bl_ConfirmScreenDisplayResult	確認画面表示結果（True：表示、False：非表示）.
//! <!--戻値--> なし.
//! <!--詳細--> 確認画面表示結果を設定する.
//! <!--履歴-->.
//! 2015/11/05 OPS1山下 [MinervaSBK][CQ515617] 新規作成.
//! 2017/09/19 Sky井柳 [Eagle][制御構成変更No10] IT4以前機種実行制限（コンパイルスイッチ）削除.
// ================================================================================================.
void setConfirmScreenDisplayResult( const Bool bl_ConfirmScreenDisplayResult )
{
	THRC_AndroidControlIf* pc_AndroidIf = THRC_AndroidControlIf::getInstance();
	if( NULL != pc_AndroidIf ){
		pc_AndroidIf->setConfirmScreenDisplayResult( bl_ConfirmScreenDisplayResult );
	} else {
		SYSF_logMsg( "(%010u)(BSP) setConfirmScreenDisplayResult pc_AndroidIf is NULL \n", SYSF_tickGet() );
	}
}

// ================================================================================================.
//! <!--機能--> 確認画面非表示期間設定処理.
//! <!---------------------------------------------------------------------------------------------.
//! <!--引数--> Bool bl_Value	確認画面非表示期間設定 設定値（True：開始、False：終了）.
//! <!--戻値--> なし.
//! <!--詳細--> 確認画面非表示期間設定を設定する.
//! <!--履歴-->.
//! 2016/06/07 OPS1酒井純 [IT5-4.0][制御構成変更No.40] 新規作成.
//! 2017/09/19 Sky井柳 [Eagle][制御構成変更No10] IT4以前機種実行制限（コンパイルスイッチ）削除.
// ================================================================================================.
void setHidingConnectConfirm( const Bool bl_Value )
{
	THRC_AndroidControlIf* pc_AndroidIf = THRC_AndroidControlIf::getInstance();
	if( NULL != pc_AndroidIf ){
		pc_AndroidIf->setHidingConnectConfirm( bl_Value );
	} else {
		SYSF_logMsg( "(%010u)(BSP) setHidingConnectConfirm pc_AndroidIf is NULL \n", SYSF_tickGet() );
	}
}
//デバッグ用
void debug_setHidingConnectConfirm( int i_Switch )
{
	if( 1 == i_Switch ){
		setHidingConnectConfirm( true );
	}else{
		setHidingConnectConfirm( false );
	}
}
#ifdef __cplusplus
}
#endif

// 2011/11/30 Gonsho Android ブザー・音声再生対応 End.

// 速度可変起動モード状態フラグ.
Bool bl_SpeedVariadStatus = False ;

// ================================================================================================.
//! <!--機能--> 速度可変起動モード検出処理.
//! <!---------------------------------------------------------------------------------------------.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! <!--履歴--> 12/09/28 M.Koshimo 新規作成.
// ================================================================================================.
extern "C" void checkSpeedVariableMode()
{
	bl_SpeedVariadStatus = False;

    // LongNT98 WP_No.39 Start.
    THRE_MACHINEINFO     eMachineInfo = THRE_MACHINEINFO_INVALID;    // 機種情報.
    THRE_MACHINENO       eMachineNo   = THRE_MACHINENO_INVALID;      // 機種No.

    THRF_getMachineType((char*)&eMachineInfo, (char*)&eMachineNo);
    BSP_LOG("eMachineInfo = 0x%X, eMachineNo = 0x%X\n", eMachineInfo, eMachineNo);

    bool isEagle3MLK = (eMachineInfo == THRE_MACHINEINFO_EAGLE3MLK &&
        (eMachineNo == THRE_MACHINENO_1 |
         eMachineNo == THRE_MACHINENO_2 ||
         eMachineNo == THRE_MACHINENO_2_5));
         
    bool isEagleBK3MLK = (eMachineInfo == THRE_MACHINEINFO_EAGLEBK3MLK &&
          (eMachineNo == THRE_MACHINENO_1 ||
           eMachineNo == THRE_MACHINENO_2));

    if (isEagle3MLK || isEagleBK3MLK) {
        BSP_LOG("Speed Variable Mode Disabled for this machine.\n");
        return;
    }
    BSP_LOG("Continue check boot mode = %d\n", uc_BootMode);
    // LongNT98 WP_No.39 End.

	// 実行モードがラインカードモードの時.
	if( 10 == uc_BootMode ){// 起動種別：速度可変モード起動.
		// 速度可変起動モード状態フラグON.
		bl_SpeedVariadStatus = True ;
		printf( "SpeedVariableModeEnable\n"  );
	}
}

// ================================================================================================.
//! <!--機能--> 速度可変起動モードフラグの取得.
//! <!---------------------------------------------------------------------------------------------.
//! <!--引数-->.
//! <!--戻値--> bl_SpeedVariadStatus 1:速度可変起動モード 0:通常起動モード.
//! <!--詳細--> 速度可変起動状態を返す.
//! <!--履歴-->  12/09/28 M.Koshimo 新規作成.
// ================================================================================================.
extern "C" Bool getSpeedVariableMode()
{
	return bl_SpeedVariadStatus;
}
