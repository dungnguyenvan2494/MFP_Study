/*================================================================================================
//! \file ［タイトル］	スレッドＩＦデータ実装	ファイル名：THRC_Threadif.cpp.
================================================================================================*/
#include <unistd.h> //制御構成変更1331　WebROM一本化対応　access()用.
#include "lib/define.h"
#include <sys/mount.h>									// CQ.384445.
#include "THRC_Threadif.h"
#include "THRC_BSPif.h"
#include "system/thr/thrInclude/THRC_ThreadInstance.h"
#include "system/thr/thrInclude/THRC_PortInstance.h"	// 2008/05/27 suzumura Port設定を見る.
#include "system/thr/thrInclude/THRC_ThreadDebug.h"		// UV-01385 ログ出力.
#include "system/fil/FIL_FsWrapperC.h"
#include "system/irc/IRMG_InitModuleIf.h"	// 2022/08/23 Sky [IT6_2.3] 機器構成変更_No.39_CISサプライヤ変更.

#if defined(TORNADO)
#include <Linux/Platform/HW_interim.h>
#endif	//#if defined(TORNADO).

#ifdef COSIM
#ifndef THR_SIMWIN
#include "system/thr/COSIM/THRC_CosimTestDrv.h"
#endif
#endif
#include "system/thr/thrInclude/THRC_Port.h" 

#include "system/thr/thrInclude/THRC_tskImagePattern.h"
extern Bool b_debug_ct_log;			// CT検証ログ用.
extern	void ThLogRecord( const char *logFmt , CC_MultiDataLog *data);	//	不定形デバッグログ:１文字列/１変数.
extern	void ThTickLog();	//ログ時刻記録：ThLogRecord とペアで使用する。.

#ifdef TORNADO
#include "Dummy_BSP.h"
#endif

#include "system/thr/thrInclude/THRC_Threaddef.h"
#include "system/prt/prtc/prtcImp/ENGC_PrinterImpColor.h" //アンチパターン対応.
#include "system/thr/thrInclude/THRC_Multithread.h" //CC_MultiDataLogを使用するため.
// チューニングツール対応.
#include "ThrCommonDrvrIf.h"

#include "../thr/thrCrypto.h"

#include "Linux/Platform/Mediaif.h"		// 2021/06/04 Sky 復旧治具の診断結果データ収集機能対応.

extern	void ThLogRecord( const char *logFmt , CC_MultiDataLog *data);	//	不定形デバッグログ:１文字列/１変数.
extern	void ThTickLog();	//ログ時刻記録：ThLogRecord とペアで使用する.

// 【課題】debugF( 300 )で変更するパラメータなので、ここで変数宣言しているが、.
//  いずれTHRC_ThreadDebug.cpp/h に移行したい。 2006/04/25 yokoi.
ushort us_debug_ForceBlack = 0;	// 強制モノクロ印字.

#define THRD_SELECT_LANG_DATA    ("/res/SelectLangData")	// UV-00872:選択言語組み合わせファイル名.
#define THRD_WEBROMFILE_PATH	 ("/km/BROWSER_ON")			//制御構成変更1331.
short ss_HDRegist = 0;			//  エンジン出力 レフトマージン調整値[0.1mm単位].

//  2014.07.21	iyanagi		[Poseidon]CQ501023 取得先変更対応(Emu3以降の正式対応).
#define THRD_BootOffset_Version		0x280002						// Boot起動ROM先頭からのオフセット(バージョン情報).
#define THRD_BootOffset_CheckSum	0x280000						// Boot起動ROM先頭からのオフセット(チェックサム値).
#define THRD_BootName_CopyDevice	"/dev/mtdblock1"				// Boot起動ROM格納デバイス名(SPI-Flash).
#define THRD_BootSize_Version		23								// Bootバージョン情報のサイズ.

#define THRD_SCBOffset_Version		0x1FFFD0						// SCB先頭からのオフセット(バージョン情報).
#define THRD_SCBOffset_VersionSize	23								// バージョン情報のサイズ(Byte).
#define THRD_SCBOffset_CheckSum		0x1FFFFE						// SCB先頭からのオフセット(チェックサム値).
#define THRD_SCBOffset_CheckSumSize	2								// チェックサム値のサイズ(Byte).
#define THRD_SCBName_CopyDevice		"/dev/mtdblock0"				// SCB格納デバイス名(SPI-Flash).

// 2018/10/16 Sky松井 [Eagle_High]制御構成変更No.47 画パラファイル切り替え用定義.
// 2021/07/01 Sky	  A800レス対応.
// 画パラ切り替え用構造体.
struct THRS_ImageParameterInfo{
	const char *psc_FileDir;	// 画パラファイル名称(/km/fw/lib).
	const char *psc_LibLinkName;// 画パラライブラリリンク名称.
	const char **psc_LibName;	// 画パラライブラリファイル名称リスト.
	size_t st_LibCnt;			// 切り替えライブラリ数.
};
#define THRD_ImageParam_LinkMakePath	"/res"											// シンボリックリンク作成先パス.

// 画パラの切り替えは、Eagle全機種、Sparrow全機種が対象.
#if (defined( DEF_EagleCus ) && !defined( DEF_HeliosMLKCus ) && !defined(DEF_MinervaSSBKCus)) || defined( DEF_SparrowCus )

#define THRD_ImageParam_LibPath						"/km/fw/lib"						// 画パラライブラリパス.
#define THRD_ThreadImageParameterFileName			"/libThr_Iprom.so"					// Thread画パラファイル名称.
#define THRD_ThreadICCImageParameterFileName		"/libThr_Icc.so"					// Thread画パラ(ICC)ファイル名称.
#if defined( DEF_EagleBKCus ) || defined( DEF_SparrowBKCus )
#define THRD_PicImageParameterFileName				"/libPic_bh360iSeries_Iprom0.so"	// PIC画パラファイル名称.
#define THRD_IrImageParameterFileName				"/libIrc_bh.so"						// IR画パラファイル名称.
#else // defined( DEF_EagleBKCus ) || defined( DEF_SparrowBKCus ).
#define THRD_PicImageParameterFileName				"/libPic_bhC360iSeries_Iprom0.so"	// PIC画パラファイル名称.
#define THRD_IrImageParameterFileName				"/libIrc_bhC.so"					// IR画パラファイル名称.
#endif // defined( DEF_EagleBKCus ) || defined( DEF_SparrowBKCus ).

#ifdef DEF_A4_Cus		//A4機種.
#if defined( DEF_SparrowBKAIOSFPCus )
/*****************
  SparrowBK_SFP.
******************/
const char *psc_LibNameThr[] = {						// Thread画パラ(IPROM)ライブラリ名称リスト.
	"/libThr_bh4050i_Iprom.so.1",						// SparrowBK_SFP I/II.
	"/libThr_bh4050i_Iprom_S3.so.1",					// SparrowBK_SFP III.
};

const char *psc_LibNameThrICC[] = {};					// Thread画パラ(ICC)ライブラリ名称リスト.

const char *psc_LibNamePIC[] = {						// PIC画パラライブラリ名称リスト.
	"/libPic_bh4050iSeries_Iprom1.so.1",				// SparrowBK_SFP I/II.
	"/libPic_bh4050iSeries_Iprom2.so.1",				// SparrowBK_SFP III.
};

const char *psc_LibNameIR[] = {};						// IR画パラライブラリ名称リスト.

#elif defined( DEF_SparrowBKCus )
/*****************
  SparrowBK_MFP.
******************/
const char *psc_LibNameThr[] = {						// Thread画パラ(IPROM)ライブラリ名称リスト.
	"/libThr_bh4050i_Iprom.so.1",						// SparrowBK_MFP I/II.
	"/libThr_bh4050i_Iprom_S3.so.1",					// SparrowBK_MFP III.
};

const char *psc_LibNameThrICC[] = {};					// Thread画パラ(ICC)ライブラリ名称リスト.

const char *psc_LibNamePIC[] = {						// PIC画パラライブラリ名称リスト.
	"/libPic_bh4050iSeries_Iprom1.so.1",				// SparrowBK_MFP I/II.
	"/libPic_bh4050iSeries_Iprom2.so.1",				// SparrowBK_MFP III.
};

const char *psc_LibNameIR[] = {};						// IR画パラライブラリ名称リスト.

#elif defined( DEF_SparrowAIOSFPCus )
/*****************
  SparrowAIO/SFP.
******************/
const char *psc_LibNameThr[] = {						// Thread画パラ(IPROM)ライブラリ名称リスト.
	"/libThr_bhC4050i_Iprom.so.1",						// SparrowAIO/SFP I/II.
	"/libThr_bhC4050i_Iprom_S3.so.1",					// SparrowAIO/SFP III.
};

const char *psc_LibNameThrICC[] = {						// Thread画パラ(ICC)ライブラリ名称リスト.
	"/libThr_bhC4050i_Icc.so.1",						// SparrowAIO/SFP I/II.
	"/libThr_bhC4050i_Icc_S3.so.1",						// SparrowAIO/SFP III.
};

const char *psc_LibNamePIC[] = {						// PIC画パラライブラリ名称リスト.
	"/libPic_bhC4050iSeries_Iprom2.so.1",				// SparrowAIO/SFP I/II.
	"/libPic_bhC4050iSeries_Iprom4.so.1",				// SparrowAIO/SFP III.
};

const char *psc_LibNameIR[] = {};						// IR画パラライブラリ名称リスト.

#else
/*****************
  SparrowMFP.
******************/
const char *psc_LibNameThr[] = {						// Thread画パラ(IPROM)ライブラリ名称リスト.
	"/libThr_bhC4050i_Iprom.so.1",						// SparrowMFP I/II.
	"/libThr_bhC4050i_Iprom_S3.so.1",					// SparrowMFP III.
};

const char *psc_LibNameThrICC[] = {						// Thread画パラ(ICC)ライブラリ名称リスト.
	"/libThr_bhC4050i_Icc.so.1",						// SparrowMFP I/II.
	"/libThr_bhC4050i_Icc_S3.so.1",						// SparrowMFP III.
};

const char *psc_LibNamePIC[] = {						// PIC画パラライブラリ名称リスト.
	"/libPic_bhC4050iSeries_Iprom1.so.1",				// SparrowMFP I/II.
	"/libPic_bhC4050iSeries_Iprom3.so.1",				// SparrowMFP III.
};

const char *psc_LibNameIR[] = {};						// IR画パラライブラリ名称リスト.

#endif
#else // DEF_A4_Cus.

#if defined( DEF_EagleZBKpCus )
/*****************
  EagleBKZ+.
******************/
const char *psc_LibNameThr[] = {						// Thread画パラ(IPROM)ライブラリ名称リスト.
	"/libThr_bh650i_A_Iprom.so.1",						// EagleBKZ+ I/II.
	"/libThr_bh650i_A_Iprom_E3.so.1",					// EagleBKZ+ III.
};

const char *psc_LibNameThrICC[] = {};					// Thread画パラ(ICC)ライブラリ名称リスト.

const char *psc_LibNamePIC[] = {						// PIC画パラライブラリ名称リスト.
	"/libPic_bh750iSeries_Iprom1.so.1",					// EagleBKZ+ I/II.
	"/libPic_bh750iSeries_Iprom2.so.1",					// EagleBKZ+ III.
};

const char *psc_LibNameIR[] = {};						// IR画パラライブラリ名称リスト.

#elif defined( DEF_EagleZpCus )
/*****************
  EagleZ+.
******************/
const char *psc_LibNameThr[] = {						// Thread画パラ(IPROM)ライブラリ名称リスト.
	"/libThr_bhC750i_A_Iprom.so.1",						// EagleZ+ I/II.
	"/libThr_bhC750i_A_Iprom_E3.so.1",					// EagleZ+ III.
};

const char *psc_LibNameThrICC[] = {						// Thread画パラ(ICC)ライブラリ名称リスト.
	"/libThr_bhC750i_A_Icc.so.1",						// EagleZ+ I/II.
	"/libThr_bhC750i_A_Icc_E3.so.1",					// EagleZ+ III.
};

const char *psc_LibNamePIC[] = {						// PIC画パラライブラリ名称リスト.
	"/libPic_bhC750iSeries_Iprom1.so.1",				// EagleZ+ I/II.
	"/libPic_bhC750iSeries_Iprom2.so.1",				// EagleZ+ III.
};

const char *psc_LibNameIR[] = {};						// IR画パラライブラリ名称リスト.

#elif defined( DEF_A800 ) && defined( DEF_EagleBKCus )
/*****************
  EagleBKZX.
******************/
const char *psc_LibNameThr[] = {						// Thread画パラ(IPROM)ライブラリ名称リスト.
	"/libThr_bh650i_A_Iprom.so.1",						// EagleBKZX I/II.
	"/libThr_bh650i_A_Iprom_E3.so.1",					// EagleBKZX III.
};

const char *psc_LibNameThrICC[] = {};					// Thread画パラ(ICC)ライブラリ名称リスト.

const char *psc_LibNamePIC[] = {						// PIC画パラライブラリ名称リスト.
	"/libPic_bh650iSeries_Iprom1.so.1",					// EagleBKZX I/II.
	"/libPic_bh650iSeries_Iprom2.so.1",					// EagleBKZX III.
};

const char *psc_LibNameIR[] = {};						// IR画パラライブラリ名称リスト.

#elif defined( DEF_A800 )								// Eagle_ZX.
/*****************
  EagleZX.
******************/
const char *psc_LibNameThr[] = {						// Thread画パラ(IPROM)ライブラリ名称リスト.
	"/libThr_bhC650i_A_Iprom.so.1",						// EagleZ/X I/II.
	"/libThr_bhC650i_A_Iprom_E3.so.1",					// EagleZ/X III.
};

const char *psc_LibNameThrICC[] = {						// Thread画パラ(ICC)ライブラリ名称リスト.
	"/libThr_bhC650i_A_Icc.so.1",						// EagleZ/X I/II.
	"/libThr_bhC650i_A_Icc_E3.so.1",					// EagleZ/X III.
};

const char *psc_LibNamePIC[] = {						// PIC画パラライブラリ名称リスト.
	"/libPic_bhC650iSeries_Iprom1.so.1",				// EagleZ/X I/II.
	"/libPic_bhC650iSeries_Iprom2.so.1",				// EagleZ/X III.
};

const char *psc_LibNameIR[] = {};						// IR画パラライブラリ名称リスト.

#elif defined( DEF_EagleBKCus )							// EagleBK.
/*****************
  EagleBK.
******************/
const char *psc_LibNameThr[] = {						// Thread画パラ(IPROM)ライブラリ名称リスト.
	"/libThr_bh360i_Iprom.so.1",						// EagleBK 1/2 I/II（旧CIS）.
	"/libThr_bh360i_Iprom.so.1",						// EagleBK 1/2 I/II（新CIS）.
	"/libThr_bh650i_Iprom.so.1",						// EagleBK 0 I/II.
	"/libThr_bh650i_Iprom2.so.1",						// EagleBK Z/X(A800less) I/II.
	"/libThr_bh360i_Iprom_E3.so.1",						// EagleBK 1/2 III（旧CIS）.
	"/libThr_bh360i_Iprom_E3.so.1",						// EagleBK 1/2 III（新CIS）.
	"/libThr_bh650i_Iprom_E3.so.1",						// EagleBK 0 III.
	"/libThr_bh650i_Iprom_E3_2.so.1",					// EagleBK Z/X(A800less) III.
};

const char *psc_LibNameThrICC[] = {};					// Thread画パラ(ICC)ライブラリ名称リスト.

const char *psc_LibNamePIC[] = {						// PIC画パラライブラリ名称リスト.
	"/libPic_bh360iSeries_Iprom1.so.1",					// EagleBK 1/2 I/II（旧CIS）.
	"/libPic_bh360iSeries_Iprom5.so.1",					// EagleBK 1/2 I/II（新CIS）. // 2022/08/23 Sky [IT6_2.3] 機器構成変更_No.39_CISサプライヤ変更.
	"/libPic_bh360iSeries_Iprom2.so.1",					// EagleBK 0 I/II.
	"/libPic_bh360iSeries_Iprom2.so.1",					// EagleBK Z/X(A800less) I/II.
	"/libPic_bh360iSeries_Iprom3.so.1",					// EagleBK 1/2 III（旧CIS）.
	"/libPic_bh360iSeries_Iprom6.so.1",					// EagleBK 1/2 III（新CIS）. // 2022/08/23 Sky [IT6_2.3] 機器構成変更_No.39_CISサプライヤ変更.
	"/libPic_bh360iSeries_Iprom4.so.1",					// EagleBK 0 III.
	"/libPic_bh360iSeries_Iprom4.so.1",					// EagleBK Z/X(A800less) III.
};

const char *psc_LibNameIR[] = {							// IR画パラライブラリ名称リスト.
	"/libIrc_bh360i.so.1",								// EagleBK 1/2 I/II（旧CIS）.
	"/libIrc_bh360i_CIS1.so.1",							// EagleBK 1/2 I/II（新CIS）. // 2022/08/23 Sky [IT6_2.3] 機器構成変更_No.39_CISサプライヤ変更.
	"/libIrc_bh450i.so.1",								// EagleBK 0 I/II.
	"/libIrc_bh450i.so.1",								// EagleBK Z/X(A800less) I/II.
	"/libIrc_bh360i.so.1",								// EagleBK 1/2 III（旧CIS）.
	"/libIrc_bh360i_CIS1.so.1",							// EagleBK 1/2 III（新CIS）. // 2022/08/23 Sky [IT6_2.3] 機器構成変更_No.39_CISサプライヤ変更.
	"/libIrc_bh450i.so.1",								// EagleBK 0 III.
	"/libIrc_bh450i.so.1",								// EagleBK Z/X(A800less) III.
};

#else
/*****************
  EagleL.
******************/
const char *psc_LibNameThr[] = {						// Thread画パラ(IPROM)ライブラリ名称リスト.
	"/libThr_bhC360i_Iprom.so.1",						// Eagle1/2/2.5 I/II（旧CIS）.
	"/libThr_bhC360i_Iprom.so.1",						// Eagle1/2/2.5 I/II（新CIS）.
	"/libThr_bhC650i_Iprom.so.1",						// Eagle0 I/II.
	"/libThr_bhC650i_Iprom2.so.1",						// EagleX(A800less) I/II.
	"/libThr_bhC360i_Iprom_E3.so.1",					// Eagle1/2/2.5 III（旧CIS）.
	"/libThr_bhC360i_Iprom_E3.so.1",					// Eagle1/2/2.5 III（新CIS）.
	"/libThr_bhC650i_Iprom_E3.so.1",					// Eagle0 III.
	"/libThr_bhC650i_Iprom_E3_2.so.1",					// EagleX(A800less) III.
};

const char *psc_LibNameThrICC[] = {						// Thread画パラ(ICC)ライブラリ名称リスト.
	"/libThr_bhC360i_Icc.so.1",							// Eagle1/2/2.5 I/II（旧CIS）.
	"/libThr_bhC360i_Icc.so.1",							// Eagle1/2/2.5 I/II（新CIS）.
	"/libThr_bhC650i_Icc.so.1",							// Eagle0 I/II.
	"/libThr_bhC650i_Icc2.so.1",						// EagleX(A800less) I/II.
	"/libThr_bhC360i_Icc_E3.so.1",						// Eagle1/2/2.5 III（旧CIS）.
	"/libThr_bhC360i_Icc_E3.so.1",						// Eagle1/2/2.5 III（新CIS）.
	"/libThr_bhC650i_Icc_E3.so.1",						// Eagle0 III.
	"/libThr_bhC650i_Icc_E3_2.so.1",					// EagleX(A800less) III.
};

const char *psc_LibNamePIC[] = {						// PIC画パラライブラリ名称リスト.
	"/libPic_bhC360iSeries_Iprom1.so.1",				// Eagle1/2/2.5 I/II（旧CIS）.
	"/libPic_bhC360iSeries_Iprom5.so.1",				// Eagle1/2/2.5 I/II（新CIS）. // 2022/08/23 Sky [IT6_2.3] 機器構成変更_No.39_CISサプライヤ変更.
	"/libPic_bhC360iSeries_Iprom2.so.1",				// Eagle0 I/II.
	"/libPic_bhC360iSeries_Iprom2.so.1",				// EagleX(A800less) I/II.
	"/libPic_bhC360iSeries_Iprom3.so.1",				// Eagle1/2/2.5 III（旧CIS）.
	"/libPic_bhC360iSeries_Iprom6.so.1",				// Eagle1/2/2.5 III（新CIS）. // 2022/08/23 Sky [IT6_2.3] 機器構成変更_No.39_CISサプライヤ変更.
	"/libPic_bhC360iSeries_Iprom4.so.1",				// Eagle0 III.
	"/libPic_bhC360iSeries_Iprom4.so.1",				// EagleX(A800less) III.
};

const char *psc_LibNameIR[] = {							// IR画パラライブラリ名称リスト.
	"/libIrc_bhC360i.so.1",								// Eagle1/2/2.5 I/II（旧CIS）.
	"/libIrc_bhC360i_CIS1.so.1",						// Eagle1/2/2.5 I/II（新CIS）. // 2022/08/23 Sky [IT6_2.3] 機器構成変更_No.39_CISサプライヤ変更.
	"/libIrc_bhC450i.so.1",								// Eagle0 I/II.
	"/libIrc_bhC450i.so.1",								// EagleX(A800less) I/II.
	"/libIrc_bhC360i.so.1",								// Eagle1/2/2.5 III（旧CIS）.
	"/libIrc_bhC360i_CIS1.so.1",						// Eagle1/2/2.5 III（新CIS）. // 2022/08/23 Sky [IT6_2.3] 機器構成変更_No.39_CISサプライヤ変更.
	"/libIrc_bhC450i.so.1",								// Eagle0 III.
	"/libIrc_bhC450i.so.1",								// EagleX(A800less) III.
};

#endif
#endif // DEF_A4_Cus.


THRS_ImageParameterInfo st_ImgPrmInf_Thr = {							// Thread画パラ切り替え用データ.
	THRD_ImageParam_LibPath,											// 画パラファイルパス(/km/fw/lib).
	THRD_ThreadImageParameterFileName,									// Thread画パラライブラリリンク名称.
	psc_LibNameThr,														// Thread画パラライブラリ名称リスト.
	( sizeof( psc_LibNameThr ) / sizeof( psc_LibNameThr[ 0 ] ) )		// 切り替えライブラリ数.
};

THRS_ImageParameterInfo st_ImgPrmInf_ThrICC = {							// Thread画パラ(ICC)切り替え用データ.
	THRD_ImageParam_LibPath,											// 画パラファイルパス(/km/fw/lib).
	THRD_ThreadICCImageParameterFileName,								// Thread画パラライブラリリンク名称.
	psc_LibNameThrICC,													// Thread画パラライブラリ名称リスト.
	( sizeof( psc_LibNameThrICC ) / sizeof( psc_LibNameThrICC[ 0 ] ) )	// 切り替えライブラリ数.
};

THRS_ImageParameterInfo st_ImgPrmInf_PIC = {							// PIC画パラ切り替え用データ.
	THRD_ImageParam_LibPath,											// 画パラファイルパス(/km/fw/lib).
	THRD_PicImageParameterFileName,										// PIC画パラファイル名称(リンク名称).
	psc_LibNamePIC,														// PIC画パラライブラリ名称リスト.
	( sizeof( psc_LibNamePIC ) / sizeof( psc_LibNamePIC[ 0 ] ) )		// 切り替えライブラリ数.
};

THRS_ImageParameterInfo st_ImgPrmInf_IR = {								// IR画パラ切り替え用データ.
	THRD_ImageParam_LibPath,											// 画パラファイルパス(/km/fw/lib).
	THRD_IrImageParameterFileName,										// IR画パラライブラリリンク名称.
	psc_LibNameIR,														// IR画パラライブラリ名称リスト.
	( sizeof( psc_LibNameIR ) / sizeof( psc_LibNameIR[ 0 ] ) )			// 切り替えライブラリ数.
};

THRS_ImageParameterInfo *pst_ImgPrmInfList[] = {
	&st_ImgPrmInf_Thr,		// Thread画パラ切り替え情報.
	&st_ImgPrmInf_ThrICC,	// Thread画パラ(ICC)切り替え情報.
	&st_ImgPrmInf_PIC,		// PIC画パラ切り替え情報.	
	&st_ImgPrmInf_IR,		// IR画パラ切り替え情報.
};

#define THRD_ImgPrmInfListCount	( sizeof( pst_ImgPrmInfList ) / sizeof( pst_ImgPrmInfList[ 0 ] ) )	// 画パラファイル(種別)数.

#else // 画パラ切り替え対象機種以外.
#define THRD_ImgPrmInfListCount	0	// 画パラファイル(種別)数.
// 画パラ切り替え情報リスト.
// ※画パラ切り替えが不要の場合はリストはNULLとする.
THRS_ImageParameterInfo *pst_ImgPrmInfList[] = { NULL };
#endif //  (defined( DEF_EagleCus ) && !defined( DEF_HeliosMLKCus ) && !defined(DEF_MinervaSSBKCus)) || defined( DEF_SparrowCus ).

// 画パラ切り替え用関数.
Bool THRL_makeImageParamSymLink( THRS_ImageParameterInfo *, THRE_ImageParameterType );
THRE_ImageParameterType THRL_getImageParameterType( char sc_Type );

//2009/03/30 newInstance対応 comMemoryManagerのインスタンス生成.
IMGC_ImageMemoryPoolImpColorFileMemoryInstance& comMemoryManager = IMGC_ImageMemoryPoolImpColorFileMemorySingletonIf::newInstance();

// 2014/02/19 マルチRAS対応.
static SEM_ID semID_memset = SYSF_semMCreate(SEM_Q_PRIORITY | SEM_DELETE_SAFE);

static THRS_DataProtect	stDataProtect;

Bool THRF_getSubsetVersion_Sub( char *sc_Version, int32_t *si_Chksum, char *psc_FileName );		// CQ.384445.

// 制御構成変更No.35_A800Less対応後の基板交換間違い検知.
static const char FwDir[][BSN_FWDIR_LENGTH+1] = {
#ifdef DEF_A800
	// A800装着機種のROM名称確認.
	"FW0021",		// EagleZX.
	"FW0026",		// EagleBKZX.
#else // DEF_A800.
	// A800非装着機種のROM名称確認.
	"FW0020",		// Eagle0/1/2/2.5 / EagleX(A800Less).
	"FW0025",		// EagleBK0/1/2 / EagleBKX(A800Less) / EagleBKZ(A800Less).
#endif // DEF_A800.
};

// ================================================================================================.
// ★★★ チューニングツール対応(制御構成変更No20 画像設計用機能追加) .
// ================================================================================================.
// ------------------------------------------------.
// ★★関数宣言.
// ------------------------------------------------.
// レジスタ書き込み.
static short THRL_writeReg( uchar uc_AsicNo, ushort us_IpNo, uchar* puc_Buffer, uint32_t ul_Length, uint32_t ul_Offset );
// レジスタ読み出し.
static short THRL_readReg( uchar uc_AsicNo, ushort us_IpNo, uchar* puc_Buffer, uint32_t* pul_Length, uint32_t ul_Offset );
// ------------------------------------------------.
// ★★構造体宣言.
// ------------------------------------------------.
// 接続チェック.
struct THRS_IPROMChangeConnectChk {
	uchar	uc_AsicNo;		// AsicID(コマンド).
	char	sc_AsicUID;		// AsicUID.
	char	sc_AsicKind;	// Asic種別.
	char	sc_Tmp;			// パティング.
};

// レジスタサイズ算出用テーブル.
struct THRS_IPROMRegSizeTbl {
	ushort	us_IpNo;		// 回路番号.
	ushort	us_Tmp;			// パティング.
	uint32_t	ul_Size;		// サイズ.
	uintptr_t	ul_Address;		// 開始アドレス.
};
// ------------------------------------------------.
// ★★変数宣言.
// ------------------------------------------------.
// 接続チェックテーブル.
#if defined(DEF_A4_Cus) // Sparrow.
static THRS_IPROMChangeConnectChk	gs_ConnectChk[] = {
//	{1, THRE_AsicKind_Non	, THRE_AsicKind_Non		, 0},		// ASIC777(本体).
//	{2, THRE_AsicKind_Non	, THRE_AsicKind_Non		, 0},		// ASIC777(DS).
//	{3, THRE_AsicUID_Asic_2	, THRE_AsicKind_Asic30	, 0},		// ASIC30(Fiery).
//	{4, THRE_AsicUID_Asic_3	, THRE_AsicKind_Asic2014, 0},		// ASIC2014(Ds).
//	{5, THRE_AsicUID_Asic_2	, THRE_AsicKind_Asic2014, 0},		// ASIC2014(Fiery).
//	{6, THRE_AsicKind_Non	, THRE_AsicKind_Non		, 0},		// PMS.
	{7, THRE_AsicUID_Asic_1	, THRE_AsicKind_S800	, 0},		// S800(本体).
};
#else // Sparrow.
static THRS_IPROMChangeConnectChk	gs_ConnectChk[] = {
	{1, THRE_AsicUID_Asic_1, THRE_AsicKind_S800, 0},		// S800(本体).
#ifdef DEF_A800
	{2, THRE_AsicUID_Asic_2, THRE_AsicKind_A800, 0},		// A800.
#endif // DEF_A800.
#ifdef THRD_TURNING_TOOL_ASIC_FREE	// 現状はS800しか対応しないため、コンパイルSWで無効化する
	{3, THRE_AsicUID_Asic_2, THRE_AsicKind_Asic30, 0},		// Asic30(Fiery).
	{4, THRE_AsicUID_Asic_3, THRE_AsicKind_Asic2014, 0},	// Asic2014(DS).
	{5, THRE_AsicUID_Asic_2, THRE_AsicKind_Asic2014, 0}		// Asic2014(Fiery).
#endif // THRD_TURNING_TOOL_ASIC_FREE.
};
#endif // Sparrow.
#define THRD_ConnectTblNumMax ( sizeof( gs_ConnectChk ) / sizeof( THRS_IPROMChangeConnectChk ) )

static THRS_IPROMRegSizeTbl			gs_IPROMReadRegDataSizeTbl[] =
{
	// 回路番号,パティング,	サイズ,	開始アドレス.
	{ 0x3300, 	0, 			28, 		0x4300 },	// 色変換部_1.
	{ 0x3301, 	0, 			132, 		0x4340 },	// RGBLab.
	{ 0x3302, 	0, 			56, 		0x4700 },	// 誤差拡散部.
	{ 0x3303, 	0, 			112, 		0x4B80 },	// 多値合成.
	{ 0x3304, 	0, 			24, 		0x45E0 },	// 解像度変換部.
	{ 0x3305, 	0, 			16, 		0x0018 },	// 予備圧縮部.
	{ 0x3306, 	0, 			228,		0x7B00 },	// 色変換部_2.
	{ 0x3307, 	0, 			0, 			0x0000 },	// 画像補正部 ※ASIC777では、非搭載.
	{ 0x3308, 	0, 			96, 		0x7E00 },	// スクリーン処理部.
	{ 0x3309, 	0, 			8, 			0x4688 },	// ﾓﾉｶﾗｰ用ﾃﾞｰﾀ選択.
	{ 0x330A, 	0, 			16, 		0x4014 }	// TIFF-C送信.
};


// ------------------------------------------------.
// ★★定数宣言.
// ------------------------------------------------.
// サイズテーブル要素数最大値.
#define	THRD_IPROMReadRegSizeTblNumMax	( sizeof( gs_IPROMReadRegDataSizeTbl ) / sizeof( THRS_IPROMRegSizeTbl ) )


// ================================================================================================.
// FW Up/Down対応.
// ================================================================================================.
#define	THRD_FWUDTest_RetryNum				3									//	実行回数(3回).
#define THRD_FWUDTest_WaitTime				5									//	リトライ待ち時間（秒）.
#define	THRD_FWUDTest_DstEMMC				"/ata0a/AutotestTool/nvdata"		//	バックアップ先（HDD非装着）.
#define	THRD_FWUDTest_DstHDD				"/ata01/AutotestTool/nvdata"		//	バックアップ先（HDD装着）.
#define	THRD_GadgetBaseDir					"/ata0a/Gadget"						//	任意ガジェットの格納されているディレクトリパス.
#define	THRD_UserThemeBaseDir				"/ata0a/UICustom"					//	ユーザーテーマの格納されているディレクトリパス.
#define	THRD_FixGadgetBaseDir				"/km/"								//	固定ガジェットファイルの格納されているディレクトリパス.
#define	THRD_StampBaseDir					"/km/"								//	スタンプ系ファイルの格納されているディレクトリパス.
#define	THRD_ArbBaseDir						"/sdir/"							//	スタンプ系ファイルの格納されているディレクトリパス.
#define	THRD_FWUDTest_ListName				"thread_list.txt"					//	ファイルリスト名.
#define	THRD_ModuleDir						"thread"							//	モジュール別格納先ディレクトリ.
#define	THRD_MaxPathLen						512									//	最大パス長.

//! 2017/09/19 Sky井柳 [Eagle][制御構成変更No10] Eagle機種時の統一名称指定対応.
#define	THRD_ArbFileName					"ARBITRARY.bin"						//	任意スタンプファイル名.
#define	THRD_StampFileName					"STAMP.bin"							//	固定スタンプファイル名.
#define THRD_FixGadgetFileName				"GAD.bin"							//	固定ガジェットファイル名.
#define	THRD_ArbBasePath		THRD_ArbBaseDir THRD_ArbFileName				//	任意スタンプファイルの格納されているディレクトリパス.
#define	THRD_StampBasePath		THRD_StampBaseDir THRD_StampFileName			//	固定スタンプファイルの格納されているディレクトリパス.
#define	THRD_FixGadgetBasePath	THRD_FixGadgetBaseDir THRD_FixGadgetFileName	//	固定ガジェットファイルの格納されているディレクトリパス.

#define	THRD_GadgetFileName					"Gadget.bin"						//	任意ガジェットファイル名(保存ファイル名).
#define	THRD_UserThemeFileName				"UICustom.bin"						//	ユーザーテーマファイル名(保存ファイル名).

struct THRS_FWUDTestFile {
	char*		psc_BasePath;		//	コピー元パス.
	Bool		bl_BaseIsFile;		//	コピー元がファイルかどうか(True：File、False:Dir).
	uint32_t	ui_NGParam;			//	NG時のパラメータ.
	char*		psc_DstName;		//	コピー先ファイル名.
};

//	FW Up/Down対象ファイルの一覧.
//	対象ファイルが増える場合はこのリストに追加.
static THRS_FWUDTestFile s_TestFile[] = {
	{ THRD_ArbBasePath,			True,	THRD_FWUDTestCollect_ArbNG,			THRD_ArbFileName },			//	任意スタンプ.
	{ THRD_StampBasePath,		True,	THRD_FWUDTestCollect_StampNG,		THRD_StampFileName },		//	固定スタンプ.
	{ THRD_GadgetBaseDir,		False,	THRD_FWUDTestCollect_GadgetNG,		THRD_GadgetFileName },		//	任意ガジェット.
	{ THRD_FixGadgetBasePath,	True,	THRD_FWUDTestCollect_FixGadgetNG,	THRD_FixGadgetFileName },	//	固定ガジェット.
	{ THRD_UserThemeBaseDir,	False,	THRD_FWUDTestCollect_UserThemeNG,	THRD_UserThemeFileName },	//	ユーザーテーマ.
};
#define THR_FWUDTestFileNum 	(sizeof(s_TestFile) / sizeof(THRS_FWUDTestFile))

static uint32_t THRL_FWUDTestCollectRetry( const char *psc_DstDir );
static uint32_t THRL_FWUDTestCollect( const char *psc_DstDir );
static uint32_t THRL_FWUDTestCollectDelete( const char *psc_DstDir );
static Bool THRL_FWUDTestCollectOne( const THRS_FWUDTestFile st_Data, const char* psc_DstDir );
static Bool THRL_FWUDTestCollectOneFile( const char* psc_BasePath, const char* psc_DstDir, const char* psc_DstName );
static Bool THRL_FWUDTestCollectOneDir( const char* psc_BasePath, const char* psc_DstDir, const char* psc_DstName );
static Bool THRL_FWUDTest_ListOut( const char* psc_Str, const char* psc_ListFilePath );

// ================================================================================================.
//! <!--機能--> 【スレッドとPrtのIF】テストパターンモードか否か判断.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return Bool	True: テストパターン動作, False: 通常動作.
//! <!--詳細-->.
//! prtc/ENGC_PrinterImpColor.cpp で定義 \n.
//! スレッドとPrtのIFを統一するために extern Bool isTestPatternPrint の直接Readしないこと \n.
//! <!--履歴-->.
// 2005/05/23 橋本昌 	新規作成.
// ================================================================================================.
Bool isTestPatternPrintMode()
{
	extern Bool getTestPtnPrint();	// prtc/ENGC_PrinterImpColorDebug.cpp で定義.
	return getTestPtnPrint();
}


// ================================================================================================.
//! <!--機能--> 【スレッドとPrtのIF】γ自動調整・誤差拡散か否か判断.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return Bool	True: テストパターン動作, False: 通常動作.
//! <!--詳細-->.
//! \common\system\prt\prtc\ccpic.cppで定義  \n.
//! スレッドとPrtのIFを統一するために extern Bool isAutoAdjustPrint の直接Readしないこと \n.
//! <!--履歴-->.
// 2005/05/23 橋本昌 	新規作成.
// ================================================================================================.
Bool isAutoAdjustPrintMode()
{
	extern Bool CCG_GetAutoAdjustPrintOn();	// \common\system\prt\prtc\ccpic.cppで定義.
	return CCG_GetAutoAdjustPrintOn();
}

// ================================================================================================.
//! <!--機能--> 【スレッドとPICのIF】エンジン出力時のレフトマージン指定値をｔｓｋｐｉｃから設定.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! \common\system/cnv/cnvc/Asic117\THRC_Asic117EngOutputSeq.cppで定義  \n.
//! ｔｓｋｐｉｃからの関数コールにて設定 \n.
//! \n.
//! レフトマージン値はRhein1ではASIC10にて補正することになった。\n.
//! 値はシステム固定でなく給紙段ごとに異なるため、従来のＰＩＣへの通知タイミングにて\n.
//! 本Ｉ／Ｆを利用して、スレッド層へ通知する。\n.
//! <!--履歴-->.
// 2005/05/23 橋本昌 	新規作成.
// 2009/09/24 陰山		Soc対応.
// ================================================================================================.
void setEngOutHDRegist( short ss_Data )
{
	// ｔｓｋｐｉｃから設定.
#ifndef COSIM	// 実機のとき.
	ss_HDRegist = _ushortBitDToShort( ss_Data );
#endif
}

// ================================================================================================.
//! <!--機能--> 【スレッドとDecompのIF】変倍時の解像度を返す.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param IMGC_Image* pc_Image ............... 変倍するイメージへのぽいんた.
//! \param TYPC_Point<short> ct_Size .......... 変倍後のサイズ(dot).
//! \param TYPC_Rectangle<short> ct_Margine ... 出力サイズとイメージのマージン.
//! <!--戻値-->.
//! \return 変倍に必要な解像度.
//! <!--詳細-->.
//! 回転は考慮無しで算出。\n.
//! \n.
//! <!--履歴-->.
// 2005/12/12 yokoi 	新規作成.
// 2006/07/07 kageyama	画幅指定モード修正対応.
// 2006/09/05 kageyama	TransSizeからImageDotSizeに変更.
//  08/01/28 大原達也　　修正　　　ThUnknownParaを呼び出すように変更.
// 2009/03/17 tadamasa  CQ316536 再現しない為、今後の為にログが出るようにする.
// ================================================================================================.
// 2006/07/07 [VarWidthAppoint] 画幅指定モード修正対応 kageyama change 1line.
//TYPC_Resolution THRF_getChgSizeResolution( IMGC_Image* pc_Image, TYPC_Point<ulong> ct_Size, TYPC_Rectangle<short> ct_Margine ).
TYPC_Resolution THRF_getChgSizeResolution( IMGC_Image* pc_Image, TYPC_Point<uint32_t> ct_Size, TYPC_Rectangle<short> ct_Margine, TYPC_Point<float> ct_OutputSize )
{
	THRL_ThAssert( (pc_Image != NULL), "THRF_getChgSizeResolution() Img error" , 0, 0, 0, 0, CFA14_DetailCode0x0581 );
	TYPC_Resolution ct_AnsResolution;
	TYPC_Point<float> ct_Ans( 0, 0 );
// 2006/09/05 [TransToImage] TransSizeからImageDotSizeに変更 kageyama Change 1line start.
//	TYPC_Point<ulong> ct_InputSize( pc_Image->c_ImageInfo.ct_TransSize.x + ct_Margine.p1.x + ct_Margine.p2.x, pc_Image->c_ImageInfo.ct_TransSize.y + ct_Margine.p1.y + ct_Margine.p2.y );.
	TYPC_Point<uint32_t> ct_InputSize( pc_Image->c_ImageInfo.ct_ImageDotSize.x + ct_Margine.p1.x + ct_Margine.p2.x, pc_Image->c_ImageInfo.ct_ImageDotSize.y + ct_Margine.p1.y + ct_Margine.p2.y );
	TYPC_Point<float> ct_ImageSize( 0, 0 );

// 2006/07/07 [VarWidthAppoint] 画幅指定モード修正対応 kageyama change start.
	// 出力画幅が指定されていない場合はイメージインフォの値から解像度を求め、.
	// 出力画幅が指定された場合は出力画幅サイズから解像度を求める.
	if( ( ct_OutputSize.x == 0 ) || ( ct_OutputSize.y == 0 ) ){
		// まず、素直に解像度を求める.
		ct_Ans.x = ((float)ct_Size.x * pc_Image->c_ImageInfo.c_Resolution.getDpi().x) / (float)ct_InputSize.x;
		ct_Ans.y = ((float)ct_Size.y * pc_Image->c_ImageInfo.c_Resolution.getDpi().y) / (float)ct_InputSize.y;

		ct_ImageSize.x = DOTtoMM((short)ct_InputSize.x, pc_Image->c_ImageInfo.c_Resolution.getDpi().x );
		ct_ImageSize.y = DOTtoMM((short)ct_InputSize.y, pc_Image->c_ImageInfo.c_Resolution.getDpi().y );
	}else{
		ct_ImageSize.x = ct_OutputSize.x;
		ct_ImageSize.y = ct_OutputSize.y;

		ct_Ans.x = (float)ct_Size.x / ct_ImageSize.x * 25.4f;
		ct_Ans.y = (float)ct_Size.y / ct_ImageSize.y * 25.4f;
	}
// 2006/07/07 [VarWidthAppoint] 画幅指定モード修正対応 kageyama change end.

	// 元のドットサイズに戻せるよう、近似させる.
	uchar uc_Cnt = 0;
#define THRD_ChkNum (10)
	for( uc_Cnt = 0; uc_Cnt < THRD_ChkNum; uc_Cnt++ ){
		uint32_t ul_DotSize = MMtoDOT( ct_ImageSize.x, ct_Ans.x );
		if( ul_DotSize > ct_Size.x ){
			ct_Ans.x -= (((float)(ul_DotSize - ct_Size.x) * pc_Image->c_ImageInfo.c_Resolution.getDpi().x) / (float)pc_Image->c_ImageInfo.ct_TransSize.x)/(float)(uc_Cnt+1);
		} else if ( ul_DotSize < ct_Size.x ){
			ct_Ans.x += (((float)(ct_Size.x - ul_DotSize) * pc_Image->c_ImageInfo.c_Resolution.getDpi().x) / (float)pc_Image->c_ImageInfo.ct_TransSize.x)/(float)(uc_Cnt+1);
		} else {
			// 元に戻せる.
			break;
		}
	}
	
	if(uc_Cnt >= THRD_ChkNum){ 
		CC_MultiDataLog	cc_CC_MultiDataLog;		//CQ316536対応の為、NG発生時にログを出す.
		ThTickLog();
		cc_CC_MultiDataLog = (uint32_t)0;
		ThLogRecord("THRF_getChgSizeResolution() x Size error \n",&cc_CC_MultiDataLog);
		cc_CC_MultiDataLog = (uint32_t)ct_Margine.p1.x;
		ThLogRecord("ct_Margine : p1.x = %ld\n",&cc_CC_MultiDataLog);
		cc_CC_MultiDataLog = (uint32_t)ct_Margine.p1.y;
		ThLogRecord("ct_Margine : p1.y = %ld\n",&cc_CC_MultiDataLog);
		cc_CC_MultiDataLog = (uint32_t)ct_Margine.p2.x;
		ThLogRecord("ct_Margine : p2.x = %ld\n",&cc_CC_MultiDataLog);
		cc_CC_MultiDataLog = (uint32_t)ct_Margine.p2.y;
		ThLogRecord("ct_Margine : p2.y = %ld\n",&cc_CC_MultiDataLog);
		cc_CC_MultiDataLog = (uint32_t)pc_Image->c_ImageInfo.ct_ImageDotSize.x;
		ThLogRecord("ct_ImageDotSize : x = %ld\n",&cc_CC_MultiDataLog);
		cc_CC_MultiDataLog = (uint32_t)pc_Image->c_ImageInfo.ct_ImageDotSize.y;
		ThLogRecord("ct_ImageDotSize : y = %ld\n",&cc_CC_MultiDataLog);
		cc_CC_MultiDataLog = (uint32_t)ct_InputSize.x;
		ThLogRecord("ct_InputSize : x = %ld\n",&cc_CC_MultiDataLog);
		cc_CC_MultiDataLog = (uint32_t)ct_InputSize.y;
		ThLogRecord("ct_InputSize : y = %ld\n",&cc_CC_MultiDataLog);
		cc_CC_MultiDataLog = (double)ct_OutputSize.x;
		ThLogRecord("ct_OutputSize : x = %lf\n",&cc_CC_MultiDataLog);
		cc_CC_MultiDataLog = (double)ct_OutputSize.y;
		ThLogRecord("ct_OutputSize : y = %lf\n",&cc_CC_MultiDataLog);
		cc_CC_MultiDataLog = (double)pc_Image->c_ImageInfo.c_Resolution.getDpi().x;
		ThLogRecord("getDpi() : x = %lf\n",&cc_CC_MultiDataLog);
		cc_CC_MultiDataLog = (double)pc_Image->c_ImageInfo.c_Resolution.getDpi().y;
		ThLogRecord("getDpi() : y = %lf\n",&cc_CC_MultiDataLog);
		cc_CC_MultiDataLog = (uint32_t)uc_Cnt;
		ThLogRecord("i : i = %ld\n",&cc_CC_MultiDataLog);
		cc_CC_MultiDataLog = (uint32_t)ct_Size.x;
		ThLogRecord("ct_Size : x = %ld\n",&cc_CC_MultiDataLog);
		cc_CC_MultiDataLog = (uint32_t)ct_Size.y;
		ThLogRecord("ct_Size : y = %ld\n",&cc_CC_MultiDataLog);
		cc_CC_MultiDataLog = (double)ct_ImageSize.x;
		ThLogRecord("ct_ImageSize : x = %lf\n",&cc_CC_MultiDataLog);
		cc_CC_MultiDataLog = (double)ct_ImageSize.y;
		ThLogRecord("ct_ImageSize : y = %lf\n",&cc_CC_MultiDataLog);
		cc_CC_MultiDataLog = (uint32_t)pc_Image->c_ImageInfo.ct_TransSize.x;
		ThLogRecord("ct_TransSize : x = %ld\n",&cc_CC_MultiDataLog);
		cc_CC_MultiDataLog = (uint32_t)pc_Image->c_ImageInfo.ct_TransSize.y;
		ThLogRecord("ct_TransSize : y = %ld\n",&cc_CC_MultiDataLog);	
		THRL_ThUnknownPara( False, "THRF_getChgSizeResolution() x Size error" , 0, 0, 0, 0, CFA14_DetailCode0x0582 ); 
	}
	
	for( uc_Cnt = 0; uc_Cnt < THRD_ChkNum; uc_Cnt++ ){
		uint32_t ul_DotSize = MMtoDOT( ct_ImageSize.y, ct_Ans.y );
		if( ul_DotSize > ct_Size.y ){
			ct_Ans.y -= (((float)(ul_DotSize - ct_Size.y) * pc_Image->c_ImageInfo.c_Resolution.getDpi().y) / (float)pc_Image->c_ImageInfo.ct_TransSize.y)/ (float)(uc_Cnt+1);
		} else if ( ul_DotSize < ct_Size.y ){
			ct_Ans.y += (((float)(ct_Size.y - ul_DotSize) * pc_Image->c_ImageInfo.c_Resolution.getDpi().y) / (float)pc_Image->c_ImageInfo.ct_TransSize.y)/ (float)(uc_Cnt+1);
		} else {
			// 元に戻せる.
			break;
		}
	}
	if(uc_Cnt >= THRD_ChkNum){ 
		CC_MultiDataLog	cc_CC_MultiDataLog;		//CQ316536対応の為、NG発生時にログを出す.
		ThTickLog();
		cc_CC_MultiDataLog = (uint32_t)0;
		ThLogRecord("THRF_getChgSizeResolution() y Size error \n",&cc_CC_MultiDataLog);
		cc_CC_MultiDataLog = (uint32_t)ct_Margine.p1.x;
		ThLogRecord("ct_Margine : p1.x = %ld\n",&cc_CC_MultiDataLog);
		cc_CC_MultiDataLog = (uint32_t)ct_Margine.p1.y;
		ThLogRecord("ct_Margine : p1.y = %ld\n",&cc_CC_MultiDataLog);
		cc_CC_MultiDataLog = (uint32_t)ct_Margine.p2.x;
		ThLogRecord("ct_Margine : p2.x = %ld\n",&cc_CC_MultiDataLog);
		cc_CC_MultiDataLog = (uint32_t)ct_Margine.p2.y;
		ThLogRecord("ct_Margine : p2.y = %ld\n",&cc_CC_MultiDataLog);
		cc_CC_MultiDataLog = (uint32_t)pc_Image->c_ImageInfo.ct_ImageDotSize.x;
		ThLogRecord("ct_ImageDotSize : x = %ld\n",&cc_CC_MultiDataLog);
		cc_CC_MultiDataLog = (uint32_t)pc_Image->c_ImageInfo.ct_ImageDotSize.y;
		ThLogRecord("ct_ImageDotSize : y = %ld\n",&cc_CC_MultiDataLog);
		cc_CC_MultiDataLog = (uint32_t)ct_InputSize.x;
		ThLogRecord("ct_InputSize : x = %ld\n",&cc_CC_MultiDataLog);
		cc_CC_MultiDataLog = (uint32_t)ct_InputSize.y;
		ThLogRecord("ct_InputSize : y = %ld\n",&cc_CC_MultiDataLog);
		cc_CC_MultiDataLog = (double)ct_OutputSize.x;
		ThLogRecord("ct_OutputSize : x = %lf\n",&cc_CC_MultiDataLog);
		cc_CC_MultiDataLog = (double)ct_OutputSize.y;
		ThLogRecord("ct_OutputSize : y = %lf\n",&cc_CC_MultiDataLog);
		cc_CC_MultiDataLog = (double)pc_Image->c_ImageInfo.c_Resolution.getDpi().x;
		ThLogRecord("getDpi() : x = %lf\n",&cc_CC_MultiDataLog);
		cc_CC_MultiDataLog = (double)pc_Image->c_ImageInfo.c_Resolution.getDpi().y;
		ThLogRecord("getDpi() : y = %lf\n",&cc_CC_MultiDataLog);
		cc_CC_MultiDataLog = (uint32_t)uc_Cnt;
		ThLogRecord("i : i = %ld\n",&cc_CC_MultiDataLog);
		cc_CC_MultiDataLog = (uint32_t)ct_Size.x;
		ThLogRecord("ct_Size : x = %ld\n",&cc_CC_MultiDataLog);
		cc_CC_MultiDataLog = (uint32_t)ct_Size.y;
		ThLogRecord("ct_Size : y = %ld\n",&cc_CC_MultiDataLog);
		cc_CC_MultiDataLog = (double)ct_ImageSize.x;
		ThLogRecord("ct_ImageSize : x = %lf\n",&cc_CC_MultiDataLog);
		cc_CC_MultiDataLog = (double)ct_ImageSize.y;
		ThLogRecord("ct_ImageSize : y = %lf\n",&cc_CC_MultiDataLog);
		cc_CC_MultiDataLog = (uint32_t)pc_Image->c_ImageInfo.ct_TransSize.x;
		ThLogRecord("ct_TransSize : x = %ld\n",&cc_CC_MultiDataLog);
		cc_CC_MultiDataLog = (uint32_t)pc_Image->c_ImageInfo.ct_TransSize.y;
		ThLogRecord("ct_TransSize : y = %ld\n",&cc_CC_MultiDataLog);	
		THRL_ThUnknownPara( False, "THRF_getChgSizeResolution() y Size error" , 0, 0, 0, 0, CFA14_DetailCode0x0583 ); 
	}
	ct_AnsResolution.setFaxResolution( TYPE_FaxResolutionOther );
	ct_AnsResolution.setDpi( ct_Ans );
	return ( ct_AnsResolution );
}

// ================================================================================================.
//! <!--機能--> 【スレッドとIMGのIF】マシンタイプ取得(Asic/オプション状態).
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return マシンタイプ.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
// 2005/10/26 yokoi 	新規作成.
// ================================================================================================.
THRE_MachineType THRF_GetMachineType()
{
	THRE_MachineType e_Ret = THRE_MachineType_None;

	if( THRF_IsAsic_1Exist() == True ){
		// 【課題】05/10/26 長尺オプション判定は？.
		if( 1 ){
			// Asic117 2枚差し、長尺オプション無し.
			e_Ret = THRE_MachineType_117x2_Long;
		} else {
			// Asic117 2枚差し、長尺オプションアリ.
			e_Ret = THRE_MachineType_117x2;
		}
#ifdef COSIM
			e_Ret = THRE_MachineType_Cosim_117x2;
#endif
	} else {
		if( 1 ){
			// Asic117 1枚差し、長尺オプション無し.
			e_Ret = THRE_MachineType_117x1_Long;
		} else {
			// Asic117 1枚差し、長尺オプションアリ.
			e_Ret = THRE_MachineType_117x1;
		}
#ifdef COSIM
			e_Ret = THRE_MachineType_Cosim_117x1;
#endif
	}

#if defined (COSIM) && defined(LARGE)
	e_Ret = THRE_MachineType_CosimLarge;
#endif
	return ( e_Ret );
}

extern "C" {
uchar uc_SelectKeyFlag = 0;							// 第２パネルSelectキー確認用フラグ、他のモジュールから参照可能にしたくない 関数コールのIFとしたい.
}

// ================================================================================================.
//! <!--機能--> 【スレッドとSYSのIF】第２パネルSelectキー確認用フラグ 設定.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param uchar uc_Key.
//! <!--戻値-->.
//! \return uchar uc_SelectKeyFlag.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
// 2005/10/26 橋本昌 	新規作成.
// ================================================================================================.
void set2ndPanelSelectKey( uchar uc_Key )
{
	uc_SelectKeyFlag = uc_Key;
}

// ================================================================================================.
//! <!--機能--> 【スレッドとSYSのIF】第２パネルSelectキー確認用フラグ 取得.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return uchar uc_SelectKeyFlag.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
// 2005/10/26 橋本昌 	新規作成.
// ================================================================================================.
uchar get2ndPanelSelectKey()
{
	return uc_SelectKeyFlag;
}

// ================================================================================================.
//! <!--機能--> 【スレッドとSYSのIF】省電力中からのreboot処理.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
// 2008/09/19 Sky-horiuchi	CQ309242 新規作成.
//	09/09/11 越野		Soc対応.
//	13/03/06 Sky		不要箇所削除.
// 2018/10/18 河村 AR.1051005 リブート中を表すフラグを設定する.
// ================================================================================================.
extern Bool bl_isRebooting;	// リブートの際THRF_reboot()にて値を設定する(Trueを維持したままリブートする).
void THRF_reboot()
{
#ifdef TORNADO      // 実機で有効.
	bl_isRebooting = True;
	thePowerManagerTask.RunRebootSequence();	//電源管理 リブート前処理.
	// Motion_ENをDisableに設定する。.
	CB_BitPortN<THRD_Port_DataType>::newInstance(CBBPTN_portMOTER_EN) = 0;
	
	ddOutPort_IrPowerOn_setInActive();	// IrPowerOnをInActiveにする.
#endif // Tornado.
}

// ================================================================================================.
//! <!--機能--> 【スレッドとパネル、PSWCのIF】言語データ取得.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param unsigned short *pss_Buf ............... 転送先のバッファへのポインタ(2バイト境界).
//! \param uint32_t ul_Offset ............ 言語データ先頭からのオフセット(偶数).
//! \param uint32_t ul_Size .............. 転送するデータサイズ(バイト単位)(偶数).
//! <!--戻値-->.
//! \return True:成功／False:失敗.
//! <!--詳細-->.
//! 回転は考慮無しで算出。\n.
//! \param なし.
//! \n.
//! <!--履歴-->.
// 2006/03/06 k-kawa	新規作成.
// 2006/07/25 k-kawa	[64661対応]言語データの最初のセクタだけキャッシュしておき、CF にアクセスしに行かない.
// 2007/11/15 kurachi	Donau改修.
//	09/09/11 越野		Soc対応.
// 2010/03/08 oomoto	レベルチェック時のWarning対応.
// 2013/03/06 Sky		Linux以前の処理を削除.
// ================================================================================================.
#if defined(TORNADO)
Bool getLanguageDataForUI(unsigned short *pss_Buf, uint32_t ul_Offset, uint32_t ul_Size)
{
// 2009.12.10 y.oomoto ビルドエラー対応 start.
#if !defined(COSIM)
	#define MSG_DATA_CACHE	512				// 言語データキャッシュサイズ用定義.
	
	static Bool bl_Init = False;			// 言語データを読み込んだかどうか.
	static Bool bl_ExistLangData = False;	// 言語データの有無フラグ.
	static Bool bl_ExistCacheData = False;	// 言語データ（キャッシュ分）の有無フラグ.
	static unsigned short us_FirstSector[MSG_DATA_CACHE / sizeof(unsigned short)];	// 言語データ最初の512Byteデータ.
	FILE *fp;								// 言語データ用ファイルポインタ.
	Bool bl_ret = False;					// 戻り値.
	memset(pss_Buf, 0, ul_Size);			// 転送先クリア.

	if ( False == bl_Init ) {	// 最初の一回のみ言語データ有無確認を行う.
		if ( NULL != ( fp = FILF_Fopen(LINUX_MSG_BIN, "rb")) ){
			bl_ExistLangData = True;	// 言語データ有り.
			
			/* 先頭512Byteのデータを保持しておく */
			size_t s_cashdata = FILF_Fread( us_FirstSector, 1, MSG_DATA_CACHE, fp );
			if ( s_cashdata != (uint32_t)ERROR )
			{
				bl_ExistCacheData = True;
			}
			
			FILF_Fclose(fp);
		}
		bl_Init = True;	// 確認完了.
	}
	if ( True == bl_ExistLangData ) {	// 言語データが存在する場合はファイル読込みを行う.
		if ( bl_ExistCacheData == True )
		{	/* キャッシュデータがある場合はそちらから読み出す */
			if ( ul_Offset + ul_Size <= MSG_DATA_CACHE )
			{	// 512Byte以内かどうか.
				// キャッシュしたデータ内に要求されたデータがある.
				memcpy( pss_Buf, us_FirstSector + ul_Offset / sizeof(unsigned short), ul_Size );
				return True;
			}
		}
		fp = FILF_Fopen(LINUX_MSG_BIN, "rb");
		if ( NULL != fp ) {
			FILF_Fseek( fp, ul_Offset, SEEK_SET );
			size_t s_ret = FILF_Fread(pss_Buf, 1, ul_Size, fp);
			if (s_ret != ul_Size) {
				memset(pss_Buf, 0, ul_Size);	// 全部読込めなかったら全クリアする.
			} else {
				bl_ret = True;	// 読込み完了.
			}
			FILF_Fclose( fp );
		}
	}
	return bl_ret;
#else	//#if !defined(COSIM)
//[64661対応]--->.
	static Bool bl_Init = False;														// 最初のセクタを読み込んだかどうか.
	static Bool bl_ExistLangData = False;												// 言語データの有無フラグ.
#define MSG_DATA_CASH	512
	static ushort us_FirstSector[MSG_DATA_CASH / sizeof(unsigned short)];				// 言語データ最初の512Byteデータ.
	static uint32_t ulSectorNum = 0;
//<---[64661対応].

	// 転送先クリア.
	memset(pss_Buf, 0, ul_Size);

	// パラメータチェック.
	if (((uint32_t)pss_Buf & 0x01) == 1) {
		// ポインタが2バイト境界にない.
		return False;
	}
	if ((ul_Offset & 0x01) == 1) {
		// オフセットが奇数.
		return False;
	}
	if ((ul_Size & 0x01) == 1) {
		// サイズが奇数.
		return False;
	}

	// 言語データの有無チェック.
//[64661対応]--->.
	if (bl_Init) {
		if (bl_ExistLangData == False) {
			// 言語データなし.
			return False;
		}
	} else {
		bl_ExistLangData = True;
		bl_Init = True;
//<---[64661対応].
	}

	// 領域内かチェック.
	// 1セクタ256KB単位によるMSGバイナリ領域内チェック.
	if ((ul_Offset + ul_Size + FLASH_BYTE_PER_SECTOR - 1) / FLASH_BYTE_PER_SECTOR > ulSectorNum) {
		// 領域外アクセス.
		return False;
	}

	static ushort us_Buf[FLASH_BYTE_PER_SECTOR / sizeof(unsigned short)];	// 1セクタ256KB領域をstatic定義.
	uint32_t ul_SecNum = ul_Offset / FLASH_BYTE_PER_SECTOR;					// オフセットのセクタ換算(256KB単位).
	uint32_t ul_CpySize = 0;

//[64661対応]--->.
	// 要求されたデータがキャッシュしたデータ内にあるかチェック.
	if (ul_Offset + ul_Size <= MSG_DATA_CASH) {	// 512Byte以内かどうか.
		// キャッシュしたデータ内に要求されたデータがある.
		memcpy(pss_Buf, us_FirstSector + ul_Offset / sizeof(unsigned short), ul_Size);
		return True;
	}
//<---[64661対応].

	// オフセットがセクタの単位で始まっているかチェック.
	if (ul_Offset % FLASH_BYTE_PER_SECTOR != 0) {	// 256KB単位かどうか.
		// セクタ内の途中のデータが必要.
		ul_CpySize = ul_Size;
		if( ul_Offset % FLASH_BYTE_PER_SECTOR + ul_Size <= FLASH_BYTE_PER_SECTOR ){
			// 指定されたデータが読み込んだセクタ内にある.
			// セクタの途中から指定サイズをコピーして終了.
			memcpy(pss_Buf, us_Buf + (ul_Offset % FLASH_BYTE_PER_SECTOR) / sizeof(unsigned short), ul_Size);
			return True;
		} else {
			// セクタの途中からセクタの最後までコピー. 続きあり.
			ul_CpySize = FLASH_BYTE_PER_SECTOR - ul_Offset % FLASH_BYTE_PER_SECTOR;
			memcpy(pss_Buf, us_Buf + (ul_Offset % FLASH_BYTE_PER_SECTOR) / sizeof(unsigned short), ul_CpySize);
			pss_Buf += ul_CpySize / sizeof(unsigned short);
			ul_Offset += ul_CpySize;
			ul_Size -= ul_CpySize;
		}
	}

	// セクタの先頭からセクタの最後まで必要なデータをコピーする.
	ul_SecNum = ul_Offset / FLASH_BYTE_PER_SECTOR;				// セクタ開始位置(256KB単位).
	uint32_t ul_SecCnt = ul_Size / FLASH_BYTE_PER_SECTOR;			// セクタ数(256KB単位).
	ul_CpySize = ul_SecCnt * FLASH_BYTE_PER_SECTOR;				// 読み込みバイト数.
	// セクタ数が1以上(512バイト以上)のとき読み込む 2006/07/22 Div.12 k-kawa.
	pss_Buf += ul_CpySize / sizeof(unsigned short);
	ul_Offset += ul_CpySize;
	ul_Size -= ul_CpySize;

	// 最後の端数があれば、その部分をコピーする.
	if (ul_Size > 0) {
		memcpy(pss_Buf, (int)us_Buf, ul_Size);
	}
	ul_CpySize = ul_Size;

	return True;
#endif	// #if !defined(COSIM)
// 2009.12.10 y.oomoto ビルドエラー対応 end.
}

#if 0
	// getLanguageDataForUI()の動作確認用→確認できたのでコメント.

#include <ctype.h>
static void dump(unsigned char *psc_Pt, uint32_t sl_Size)
{
	uint32_t ul_Poi, ul_Poj;
	for (ul_Poi = 0; ul_Poi < sl_Size; ul_Poi+=16) {
		if (ul_Poi % 256 == 0){
			printf("\n");
		}
		printf("%08X :", (unsigned)psc_Pt + ul_Poi);
		for (ul_Poj = ul_Poi; ul_Poj < ul_Poi+16;ul_Poj++) {
			printf(" %02X", psc_Pt[ul_Poj]);
		}
		printf(" :");
		for (ul_Poj = ul_Poi; ul_Poj < ul_Poi+16;ul_Poj++) {
			printf("%c", isprint(psc_Pt[ul_Poj]) ? psc_Pt[ul_Poj] : '.');
		}
		printf("\n");
	}
	printf("\n");
}

unsigned char buf[8*1024*1024];
extern "C" int debugReadLangData()
{
	uint32_t start, end;

printf("Test1:\n");
	memset(buf, 0, sizeof(buf));
	start = SYSF_tickGet();
	printf("start=%lu\n", start);
	getLanguageDataForUI((unsigned short*)buf, 0, 8*1024*1024);
	end = SYSF_tickGet();
	printf("end=%lu, spend=%f sec.\n", end, (double)(end - start) / 1000 * 5);
	dump(buf, 256);
	dump(buf + 8*1024*1024-256, 256);

printf("Test2:\n");
	memset(buf, 0, sizeof(buf));
	start = SYSF_tickGet();
	printf("start=%lu\n", start);
	getLanguageDataForUI((unsigned short*)buf, 16, 1024);
	end = SYSF_tickGet();
	printf("end=%lu, spend=%f sec.\n", end, (double)(end - start) / 1000 * 5);
	dump(buf, 1024+256);

printf("Test3:\n");
	memset(buf, 0, sizeof(buf));
	start = SYSF_tickGet();
	printf("start=%lu\n", start);
	getLanguageDataForUI((unsigned short*)buf, 16, 100);
	end = SYSF_tickGet();
	printf("end=%lu, spend=%f sec.\n", end, (double)(end - start) / 1000 * 5);
	dump(buf, 256);

printf("Test4:\n");
	memset(buf, 0, sizeof(buf));
	start = SYSF_tickGet();
	printf("start=%lu\n", start);
	getLanguageDataForUI((unsigned short*)buf, 8*1024*1024-4, 4);
	end = SYSF_tickGet();
	printf("end=%lu, spend=%f sec.\n", end, (double)(end - start) / 1000 * 5);
	dump(buf, 16);

printf("Test5:\n");
	memset(buf, 0, sizeof(buf));
	start = SYSF_tickGet();
	printf("start=%lu\n", start);
	getLanguageDataForUI((unsigned short*)buf, 8*1024*1024-4, 6);
	end = SYSF_tickGet();
	printf("end=%lu, spend=%f sec.\n", end, (double)(end - start) / 1000 * 5);
	dump(buf, 16);
	return 0;
}
#endif	//#if 0.
#endif	//#if defined(TORNADO).

// ================================================================================================.
//! <!--機能--> 強制LOG出力設定取得.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! なし.
//! <!--戻値-->.
//! \return True:強制LOG出力ON／False:強制LOG出力OFF.
//! <!--詳細-->.
//! HWDipsw設定を見て強制LOG出力ON/OFFを返す.
//! <!--履歴-->.
//  08/05/27 suzumura 新規作成.
//  08/09/27 suzumura CQ309596SW19OFFでラインカードでLog出ない対策。ラインモードかどうかも判断する.
// ================================================================================================.
Bool THRF_getTerminalSel()
{
	if( THRL_Drvrif_IsLineMode() ) {	// ラインモードであれば強制Log出力ONと同じ扱い.
		return True;
	}
// ポートが仕様削除となったため呼び出し元のモジュール(NVD)が対応するまでの仮対応.
//	return (Bool)CB_BitPort<THRD_Port_DataType>::newInstance(CBBPTN_portTERMINAL_SEL);
	return False;
}

// 2012/07/06 takayama 新アーキ対応.

#ifdef DEF_Helios_PF /* Imageフッター対応 */
#define KERNEL_MAGIC  0x56190527  /* Image Magic Number         */
#define IMAGE_FOOTER_SIZE	64L

#define KERNEL_PATH_SSD	"/boot/Image"
#define KERNEL_PATH_USB	"/km/Image"
#else /* DEF_Helios_PF */
#define KERNEL_PATH_SSD	"/boot/uImage0"
#define KERNEL_PATH_USB	"/km/uImage0"
#endif /* DEF_Helios_PF */
#define LINUX_VERSION_FILE	"/proc/version"
#define LINUX_VERSION_DLMT	" "

#define SUBSET_PATH_ARCH	"/km/fw/bin/mfp001_hwQt"
#define SUBSET_PATH 		"/mfp/mfp001_hwQt"

// SubSet Version取得に関する定義.
#define SUBSET_PATH_ARCH					"/km/fw/bin/mfp001_hwQt"		// SubSet Binaryファイル.

#define THRD_SUBSET_PATH_CRAMFS_ARCH		"fw/bin/mfp001_hwQt"			// SubSet UV-1385対応 Binaryファイル. 
#define THRD_SUBSET_IMAGE_FILE				"SUB.img"						// SubSet cramfsイメージファイル.

#define THRD_SUBSET_PATH_FROM_MOUNT			"/dev/kmsda2"					// SSD mount元.

#define THRD_SUBSET_PATH_TO_MOUNT			"/tmp/thr"						// SubSet 一次用のmount先.
#define THRD_SUBSET_SSD_MOUNT_PATH			"/tmp/thr/ssd"					// SubSet SSDデバイスのmount先.
#define THRD_SUBSET_IMAGE_FILE_MOUNT_PATH	"/tmp/thr/img"					// SubSet cramfsイメージファイルのmount先.

// ================================================================================================.
//! <!--機能--> uImageからチェックサムを取得.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! uint *checksum_h: Imageヘッダチェックサム格納先バッファアドレス.
//! uint *checksum_d: Imageデータチェックサム格納先バッファアドレス.
//! <!--戻値-->.
//! \return BOOL TRUE:取得成功 / FALSE:取得失敗.
//! <!--詳細-->.
//! uImageのヘッダから、ヘッダチェックサムとデータチェックサムを読み出し、それぞれ格納先に格納する.
//! <!--履歴-->.
//  2011.01.20	horishita	新規作成.
//  2011.04.21	horishita	起動種別の判定値を修正。デバッグログ追加.
//  2012/07/06  takayama    新アーキ対応.
//  2015/01/20  iyanagi    【Poseidon】CQ509040不要垂れ流しログ削除対応.
//  2018/08/20  佐藤(高)	IT6はチェックサム不要.
// ================================================================================================.
#ifndef SIMULATOR	// 実機時.
extern uchar uc_BootMode;
#endif
Bool THRF_getKernelChecksum( uint32_t *checksum_h, uint32_t *checksum_d )
{
#ifdef SIMULATOR	// シミュレータ時.
	*checksum_h = 0x01234567;									// dummy値を格納する.
	*checksum_d = 0x89abcdef;
	return True;
#else				// 実機時.
#if 0 // Imageを読む旧コードを無効化.
	FILE *fp = NULL;
	char uimage_path[16];

#ifdef DEF_Helios_PF
	uint32_t magic;
#endif /* DEF_Helios_PF */

	if(uc_BootMode == 2)										// USB起動時.
	{
		strcpy(uimage_path, KERNEL_PATH_USB);					// USB起動時のuImageのpath.
	}
	else														// 本体起動時.
	{
		strcpy(uimage_path, KERNEL_PATH_SSD);					// 本体起動時のuImageのpath.
	}

	if((fp = SYSF_fopen(uimage_path, "rb")) == NULL)					// uImageをopen.
	{
		printf("THRF_getKernelChecksum(): Can't open uImage file. Bootmode = %d, path = %s\n", uc_BootMode, uimage_path);
		return False;
	}

#ifdef DEF_Helios_PF /* Imageフッター対応 */
	fseek(fp, -IMAGE_FOOTER_SIZE, SEEK_END);									// Imageの終端から64バイト分に情報(チェックサムを含む)がある
	fread(&magic, sizeof(uint32_t), 1, fp);

	if (magic == KERNEL_MAGIC)
	{
		fseek(fp, -IMAGE_FOOTER_SIZE+4L, SEEK_END);									// フッターのチェックサム(+4~7)を読み込む.
		fread(checksum_h, sizeof(uint32_t), 1, fp);

		fseek(fp, -IMAGE_FOOTER_SIZE+24L, SEEK_END);									// データのチェックサム(+24~27)を読み込む.
		fread(checksum_d, sizeof(uint32_t), 1, fp);
	}
	else
	{
		SYSF_fclose(fp);
		printf("THRF_getKernelChecksum(): There is no image_footer to Image(path = %s)\n", uimage_path);
		return False;
	}

#else /* DEF_Helios_PF */

	fseek(fp, 4L, SEEK_SET);									// ヘッダのチェックサム(+4~7)を読み込む.
	fread(checksum_h, sizeof(uint32_t), 1, fp);

	fseek(fp, 24L, SEEK_SET);									// データのチェックサム(+24~27)を読み込む.
	fread(checksum_d, sizeof(uint32_t), 1, fp);
#endif /* DEF_Helios_PF */

	SYSF_fclose(fp);

	return True;
#else // Imageを読む旧コードを無効化.
	// IT6はチェックサム不要のため、Falseを返す.
	return False;
#endif // Imageを読む旧コードを無効化.
#endif // 実機時.
}

// ================================================================================================.
//! <!--機能--> uImageからLinuxバージョン(Image Name)を取得.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! char *dst: Linuxバージョン格納先バッファアドレス.
//! <!--戻値-->.
//! \return BOOL TRUE:取得成功 / FALSE:取得失敗.
//! <!--詳細-->.
//! uImageのヘッダから、Linuxバージョン(Image Name)を読み出し、格納先に格納する.
//! ★注意★ 格納先のバッファサイズは32バイト以上である事.
//! <!--履歴-->.
//  2011.01.21	horishita	新規作成.
//  2011.01.24	horishita	sizeチェックをdstのNULLチェック処理に変更。バッファサイズの注意コメント追記.
//  2011.04.21	horishita	起動種別の判定値を修正。デバッグログ追加.
//  2012/07/06  takayama    新アーキ対応.
//  2015/01/20  iyanagi    【Poseidon】CQ509040不要垂れ流しログ削除対応.
//  2018/08/20  佐藤(高)	"/proc/version"から取得する対応.
// ================================================================================================.
Bool THRF_getLinuxVer( char *dst )
{
#ifdef SIMULATOR	// シミュレータ時.
	strcpy(dst, "Linux-2.6.26.8-rt16");							// dummy値を格納する.
#else				// 実機時.
#if 0 // Imageを読む旧コードを無効化.
	FILE *fp = NULL;
	char uimage_path[16];

#ifdef DEF_Helios_PF
	uint32_t magic;
#endif /* DEF_Helios_PF */

	if(uc_BootMode == 2)										// USB起動時.
	{
		strcpy(uimage_path, KERNEL_PATH_USB);					// USB起動時のuImageのpath.
	}
	else														// 本体起動時.
	{

		strcpy(uimage_path, KERNEL_PATH_SSD);					// 本体起動時のuImageのpath.
	}

	if(*dst != 0x00)											// dstのNULLチェック.
	{
		printf("THRF_getLinuxVer(): dst is not empty.\n");
		return False;
	}

	if((fp = SYSF_fopen(uimage_path, "rb")) == NULL)					// uImageをopen.
	{
		printf("THRF_getLinuxVer(): Can't open uImage file. Bootmode = %d, path = %s\n", uc_BootMode, uimage_path);
		return False;
	}

#ifdef DEF_Helios_PF /* Imageフッター対応 */
	fseek(fp, -IMAGE_FOOTER_SIZE, SEEK_END);									// Imageの終端から64バイト分に情報(Image Nameを含む)がある
	fread(&magic, sizeof(uint32_t), 1, fp);

	if (magic == KERNEL_MAGIC)
	{
		fseek(fp, -IMAGE_FOOTER_SIZE+32L, SEEK_END);									// Image Name(+32~63)を読み込む.
		fread(dst, sizeof(char), 32, fp);
	}
	else
	{
		SYSF_fclose(fp);
		printf("THRF_getLinuxVer(): There is no image_footer to Image(path = %s)\n", uimage_path);
		return False;
	}
#else /* DEF_Helios_PF */
	fseek(fp, 32L, SEEK_SET);									// Image Name(+32~63)を読み込む.
	fread(dst, sizeof(char), 32, fp);
#endif /* DEF_Helios_PF */

	SYSF_fclose(fp);
	dst[31] = 0x00;
#else // Imageを読む旧コードを無効化.
	// ここから "/proc/version" を読む新コード.
	FILE *fp = NULL;
	char buf[256];		// 256文字もあれば十分.
	char ver[33];		// 呼び出し元に合わせる.
	char *tok;
	
	if(*dst != 0x00)	// dstのNULLチェック.
	{
		printf("THRF_getLinuxVer(): dst is not empty.\n");
		return False;
	}
	
	if((fp = SYSF_fopen(LINUX_VERSION_FILE, "r")) == NULL)	// バージョンファイルを開く.
	{
		printf("%s: Can't open file %s\n", __FUNCTION__, LINUX_VERSION_FILE);
		return False;
	}
	
	if(SYSF_Fgets(buf, sizeof(buf), fp) == NULL)	// 1行のみ読み込む.
	{
		printf("%s: Can't read file %s\n", __FUNCTION__, LINUX_VERSION_FILE);
		SYSF_fclose(fp);
		return False;
	}
	SYSF_fclose(fp);

	// /proc/version で取得する文字列は"Linux version x.x.x ..."であることを前提とする.
	// 表示するLinuxバージョンの形式は"Linux-x.x.x"とする.
	memset(ver, 0x00, sizeof(ver));
	
	// "Linux"を取る.
	if((tok = strtok(buf, LINUX_VERSION_DLMT)) == NULL)
	{
		printf("%s: Can't get version header string\n", __FUNCTION__);
		return False;
	}
	if(strcmp(tok, "Linux") != 0)
	{
		printf("%s: Find unexpected string %s in %s\n", __FUNCTION__, tok, LINUX_VERSION_FILE);
		return False;
	}
	strcpy(ver, tok);
	
	// "version"の文字列をスキップ.
	strtok(NULL, LINUX_VERSION_DLMT);
	strcat(ver, "-");
	
	// "x.x.x"を取る(xの桁数は不明).
	if((tok = strtok(NULL, LINUX_VERSION_DLMT)) == NULL)
	{
		printf("%s: Can't get version number string\n", __FUNCTION__);
		return False;
	}
	if(strlen(tok) > (33 - 6))	// 文字数オーバーのチェック.
	{
		printf("%s: Find unexpected string %s in %s\n", __FUNCTION__, tok, LINUX_VERSION_FILE);
		return False;
	}
	strcat(ver, tok);
	
	strcpy(dst, ver);
#endif // Imageを読む旧コードを無効化.
#endif // 実機時.

	return True;
}

extern Bool getVersionFromFwTar(const char* cs_TarFileName, char* cs_Version);

// ================================================================================================.
//! <!--機能--> カードバージョン取得.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! unsigned char* pc_CardVersion O カードバージョン文字列ポインタ.
//! <!--戻値-->.
//! \return True:強制LOG出力ON／False:強制LOG出力OFF.
//! <!--詳細-->.
//! NVRAMからカードバージョンを取得する。
//! <!--履歴-->.
//  10/04/23 karasaki 新規作成.
// ================================================================================================.
Bool THRF_getCardVersion( unsigned char* pc_CardVersion )
{
	Bool	bRet = False;
#ifdef SIMULATOR	// シミュレータ時.
	memcpy(pc_CardVersion,"Zeus",4);
#else
	bRet = getVersionFromFwTar( "/boot/INDEX",(char *)pc_CardVersion  );
#endif	

	return bRet;
}

// ================================================================================================.
//! <!--機能--> サブセットバージョン、チェックサムを取得する.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! char *version	: サブセットバージョン文字列ポインタ.
//! int *chksum		: チェックサム数値ポインタ.
//! <!--戻値-->.
//! \return BOOL TRUE:取得成功 / FALSE:取得失敗.
//! <!--詳細-->.
//! SubSetイメージファイル、または、SubSetファイルのオープン、クローズ処理を行う。.
//! <!--履歴-->.
//  2011.02.21	karasaki	新規作成.
//  2012/07/06  takayama    新アーキ対応.
//  2012/08/27  itakura		CQ370707 Subsetバージョンが表示されない問題対応.
//  2014/02/10  morikawa	CQ384445 Subsetバージョンが表示されない問題対応.
// ================================================================================================.
#define CHKSUM_SIZE		4
#define VERSION_SIZE	40
#define CHKSUM_OFFSET	64
#define VERSION_OFFSET	59
Bool THRF_getSubsetVersion( char *version, int32_t *chksum)
{
#if !defined(DEF_MinervaPreEmu) && !defined(DEF_ZeusRevCus) && !defined(SIMULATOR)
	char	sc_Work[256];				// ワーク用.
	Bool	bl_TmpDir = True;			// 作業用ディレクトリ作成結果.
	Bool	bl_GetVer = False;			// Version読み出し結果.
	Bool	bl_NeedMount = False;		// CQ.384445 Subsetバージョンが表示されない問題対応.

	memset( sc_Work, 0 , sizeof(sc_Work) );
	
	if( (version == NULL) || (chksum == NULL) ){
		THRL_SetPanelHardLog( 6, (uintptr_t)version, (uintptr_t)chksum );
		return False;
	}
	
	APIC_SystemInfo* pc_SystemInfo = APIC_SystemInfo::newInstance();
	if( pc_SystemInfo == NULL ) {
		THRL_SetPanelHardLog( 7 );
		return False;
	}

	// パーティション構成がUV-01385の適用が可能かをチェックする.
	if( pc_SystemInfo->isHeliosPFPart() == True ) {
		bl_NeedMount = True;
	}

	if( bl_NeedMount == False ) {									// CQ.384445 ZeusCD(Aral45)でなければ、SubSetのBinaryからバージョン、チェックサムを取得する.
		bl_GetVer = THRF_getSubsetVersion_Sub( version, chksum, SUBSET_PATH_ARCH );
		return bl_GetVer;
	} else {														// CQ.384445 UV-1385でSubSetのBinaryがcramfsのイメージファイルとなっているのでmountしてファイル参照する.
		if( SYSF_mkdir(THRD_SUBSET_PATH_TO_MOUNT) == ERROR ) {		// CQ.384445 mount実施前にワーク用のディレクトリを作成する.
			bl_TmpDir = False;
		}
		if( SYSF_mkdir(THRD_SUBSET_SSD_MOUNT_PATH) == ERROR ) {
			bl_TmpDir = False;
		}
		if( SYSF_mkdir(THRD_SUBSET_IMAGE_FILE_MOUNT_PATH) == ERROR ) {
			bl_TmpDir = False;
		}

		if( bl_TmpDir == True ) {									// CQ.384445 ディレクトリ作成に成功したとき、"SUB.img"をmountしてVersionとCheckSumを取得する.
			mount( THRD_SUBSET_PATH_FROM_MOUNT, THRD_SUBSET_SSD_MOUNT_PATH, "vfat", MS_RDONLY, NULL );		// "exec mount /dev/kmsda2"

			// mount -t cramfs -o loop について、"SUB.img"はCramfsファイルシステムタイプ typeのため-tオプションが必要。.
			// -o loopでloop デバイス指定することで、mount先に"SUB.img"内のファイルを参照できる。
			snprintf( sc_Work, sizeof(sc_Work), "exec mount -t cramfs -o loop %s/%s %s", THRD_SUBSET_SSD_MOUNT_PATH, THRD_SUBSET_IMAGE_FILE, THRD_SUBSET_IMAGE_FILE_MOUNT_PATH );
			system( sc_Work );										// "exec mount -t cramfs -o loop /tmp/thr/ssd/SUB.img /tmp/thr/img".

			snprintf( sc_Work, sizeof(sc_Work), "%s/%s", THRD_SUBSET_IMAGE_FILE_MOUNT_PATH, THRD_SUBSET_PATH_CRAMFS_ARCH );
			bl_GetVer = THRF_getSubsetVersion_Sub( version, chksum, sc_Work );
			// umount -d はアンマウント対象のデバイスが loop デバイスの場合、 loop デバイスの解放も行う。
			snprintf( sc_Work, sizeof(sc_Work), "exec umount -d %s", THRD_SUBSET_IMAGE_FILE_MOUNT_PATH );
			system( sc_Work );										// "exec umount -d /tmp/thr/img".
			umount2( THRD_SUBSET_SSD_MOUNT_PATH, MNT_FORCE );		// "exec umount /tmp/thr/ssd".
		}
		snprintf( sc_Work, sizeof(sc_Work), "exec rmdir %s", THRD_SUBSET_SSD_MOUNT_PATH );
		system( sc_Work );
		snprintf( sc_Work, sizeof(sc_Work), "exec rmdir %s", THRD_SUBSET_IMAGE_FILE_MOUNT_PATH );
		system( sc_Work );
		snprintf( sc_Work, sizeof(sc_Work), "exec rmdir %s", THRD_SUBSET_PATH_TO_MOUNT );
		system( sc_Work );

		return bl_GetVer;
	}
#else
	return False;
#endif
}

// ================================================================================================.
//! <!--機能--> サブセットバージョン、チェックサムを取得する.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! char *version   : サブセットバージョン文字列ポインタ.
//! int *chksum     : チェックサム数値ポインタ.
//! FILE *fp        : 読み出し対象のファイルポインタ.
//! <!--戻値-->.
//! \return BOOL TRUE:取得成功 / FALSE:取得失敗.
//! <!--詳細-->.
//! ファイル終端から64Byteの位置にチェックサムとバージョンが格納されている。.
//! XX：チェックサム(固定)、VV：バージョン(可変).
//!         00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F .
//! 0x00C0  XX XX XX XX 00 VV VV VV VV VV VV VV VV VV VV VV .
//! 0x00D0  VV VV VV VV VV VV VV VV VV VV VV VV VV VV 00 00 .
//! 0x00E0  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 .
//! 0x00F0  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 ファイル終端.
//! <!--履歴-->.
//  2014/02/10  morikawa    CQ.384445 Subsetバージョンが表示されない問題対応でサブ関数化.
// ================================================================================================.
Bool THRF_getSubsetVersion_Sub( char *sc_Version, int32_t *si_Chksum, char *psc_FileName )
{
	FILE    *pst_Fp = NULL;                 // ファイルポインタ.
	char    sc_Wkchksum[CHKSUM_SIZE + 1];   // チェックサム作業用.
	char    sc_Wkversion[VERSION_SIZE +1];  // サブセットバージョン作業用.
	size_t     si_Ret;

	memset( sc_Wkchksum, 0 , sizeof(sc_Wkchksum) );
	memset( sc_Wkversion, 0 , sizeof(sc_Wkversion) );

	if( (pst_Fp = SYSF_fopen( psc_FileName, "rb" )) == NULL ){
		return False;
	}

	// ファイル終端にSeekしてファイルサイズを求める.
	si_Ret = fseek( pst_Fp, 0, SEEK_END );
	if( si_Ret != 0 ){
		THRL_SetPanelHardLog(29);
		SYSF_fclose( pst_Fp );
		return False;
	}
	uint32_t ul_End = ftell( pst_Fp ); // ファイルサイズ.
	
	// チェックサム格納位置終端から64byte前へSeek.
	si_Ret = fseek( pst_Fp, ul_End - CHKSUM_OFFSET, SEEK_SET );     // チェックサム、終端から64byte前.
	if( si_Ret != 0 ){
		THRL_SetPanelHardLog(30);
		SYSF_fclose( pst_Fp );
		return False;
	}
	// チェックサム読み出し、4byte.
	si_Ret = fread( sc_Wkchksum, 1, CHKSUM_SIZE, pst_Fp );
	if( si_Ret == (size_t)-1 ){
		THRL_SetPanelHardLog(31);
		SYSF_fclose( pst_Fp );
		return False;
	}
	
	// バージョン取得 終端から59byte前へSeek.
	si_Ret = fseek( pst_Fp, ul_End - VERSION_OFFSET, SEEK_SET );    // バージョン格納位置 終端から59byte前.
	if( si_Ret != 0 ){
		THRL_SetPanelHardLog(32);
		SYSF_fclose( pst_Fp );
		return False;
	}
	
	int32_t si_Cnt;
	for( si_Cnt = 0; si_Cnt < VERSION_SIZE; si_Cnt++ ) {
		// バージョン読み出し、1byte単位.
		si_Ret = fread( &sc_Wkversion[si_Cnt], 1, 1, pst_Fp );
		
		if( si_Ret == (size_t)-1 ){
			THRL_SetPanelHardLog(33);
			SYSF_fclose( pst_Fp );
			return False;
		}
		if( sc_Wkversion[si_Cnt] == 0x00 ||
			sc_Wkversion[si_Cnt] == 0x0D ||
			sc_Wkversion[si_Cnt] == 0x0A ){
			sc_Wkversion[si_Cnt] = 0x00;
			break;
		}
	}
	// 引数に格納.
	memcpy( sc_Version, sc_Wkversion, si_Cnt+1 );       // 終端も含めてコピー.
	*si_Chksum = (int32_t)strtol( sc_Wkchksum, NULL, 16 );       // 文字列⇒数値変換.	2017/01/24 int32_tで値が超えないことを前提で(int32_t)でキャスト.
	SYSF_fclose( pst_Fp );

	return True;
}

// ================================================================================================.
//! <!--機能--> Bootバージョン、チェックサムを取得する.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! char *version	: Bootバージョン文字列ポインタ.
//! int *chksum		: チェックサム数値ポインタ.
//! <!--戻値-->.
//! \return BOOL TRUE:取得成功 / FALSE:取得失敗.
//! <!--詳細-->.
//! ファイル先端にチェックサムとバージョンが格納されている。
//! <!--履歴-->.
//  2011.03.31	takayama	新規作成.
//  2014.07.21	iyanagi		[Poseidon]CQ501023 取得先変更対応(Emu3以降の正式対応).
//  2017.03.28	K.Morikawa(VSN)		Bootバージョン固定設定.
//  								[注] S800 PreEmu環境の暫定対応.
//  								[注] 正式対応時に対応を削除する.
// ================================================================================================.
Bool THRF_getBootVersion( char *version, ushort *chksum)
{
#if !defined(SIMULATOR)
	int32_t	 	bf_fd = 0;					// ファイルポインタ.
	char	wkchksum[CHKSUM_SIZE + 1];	// チェックサム作業用.
	off_t		ret;
	
	if( (version == NULL) || (chksum == NULL) ){
		printf("THRF_getBootVersion() parameter NULL\n");
		return False;
	}
	memset( wkchksum, 0 , sizeof(wkchksum) );
	
	// BootROMのOPEN
	bf_fd = SYSF_devopen( THRD_BootName_CopyDevice, O_RDWR | O_SYNC, 0644 );
	if ( bf_fd == -1 ){
		printf( "SYSF_open ERR errno:%d\n", SYSF_errnoGet() );
		return False;
	}

	SYSF_lseek( bf_fd, THRD_BootOffset_CheckSum, SEEK_SET );	// チェックサムの位置まで移動.

	ret = SYSF_read( bf_fd, wkchksum, 2 );
	if( ret == -1 ){
		SYSF_devclose(bf_fd);
		printf( "read: ioctl ERR errno:%d\n",SYSF_errnoGet() );
		return False;
	}
	
	memcpy(chksum,wkchksum,2);

	SYSF_lseek( bf_fd, THRD_BootOffset_Version, SEEK_SET );		// バージョンの位置まで移動.

	// BootROMのバージョンを取得 
	ret = SYSF_read( bf_fd, version, THRD_BootSize_Version );
	if( ret == -1 ){
		SYSF_devclose(bf_fd);
		printf( "read: ioctl ERR errno:%d\n",SYSF_errnoGet() );
		return False;
	}

	SYSF_devclose(bf_fd);

	return True;
#else
	return False;
#endif
}

// ================================================================================================.
//! <!--機能--> SCBバージョン、チェックサムを取得する.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! char* psc_Version	: SCBバージョン文字列ポインタ.
//! ushort* pus_Chksum	: チェックサム数値ポインタ.
//! <!--戻値-->.
//! \return BOOL TRUE:取得成功 / FALSE:取得失敗.
//! <!--詳細-->.
//! SCB FW内にチェックサムとバージョンが格納されている.
//! <!--履歴-->.
//  2014.10.31	arisawa		新規作成.
//  2017.03.28	K.Morikawa(VSN)		SCBバージョン固定設定.
//  								[注] S800 PreEmu環境の暫定対応.
//  								[注] 正式対応時に対応を削除する.
// ================================================================================================.
Bool THRF_getSCBVersion( char* psc_Version, ushort* pus_Chksum)
{
#if !defined(SIMULATOR)
#ifdef DEF_64BIT_PF
	/****************************************************/
	/* SCBバージョンは以下の固定値とする				*/
	/* SCBバージョン :A7AH0Y0-1A00-G00-02    			*/
	/* チェックサム  :0x41D8							*/
	/* [注] S800 PreEmu環境の暫定対応					*/
	/*      正式対応時にはこの対応を削除すること !!!!!	*/
	/****************************************************/
	char	sc_WkChksum[CHKSUM_SIZE + 1];	// チェックサム作業用.

	if( psc_Version == NULL ){
		THRL_SetThLog( 562, THRE_DebugLogType_ThCtrLog, 0, False, 1 );
		return False;
	}
	if( pus_Chksum == NULL ){
		THRL_SetThLog( 562, THRE_DebugLogType_ThCtrLog, 0, False, 2 );
		return False;
	}

	/* チェックサム設定 */
	memset( sc_WkChksum, 0 , sizeof(sc_WkChksum) );
	sc_WkChksum[0] = 0xD8;
	sc_WkChksum[1] = 0x41;
	memcpy( pus_Chksum, sc_WkChksum, THRD_SCBOffset_CheckSumSize );

	/* SCBバージョン設定 */
	strncpy(psc_Version,"A7AH0Y0-1A00-G00-02    ",THRD_SCBOffset_VersionSize);
#else	/* DEF_64BIT_PF */
	int32_t	 	si_Fd = 0;						// ファイルポインタ.
	char	sc_WkChksum[CHKSUM_SIZE + 1];	// チェックサム作業用.
	off_t		si_Ret;

	if( psc_Version == NULL ){
		THRL_SetThLog( 562, THRE_DebugLogType_ThCtrLog, 0, False, 1 );
		return False;
	}
	if( pus_Chksum == NULL ){
		THRL_SetThLog( 562, THRE_DebugLogType_ThCtrLog, 0, False, 2 );
		return False;
	}
	memset( sc_WkChksum, 0 , sizeof(sc_WkChksum) );

	// SCBのOPEN
	si_Fd = SYSF_devopen( THRD_SCBName_CopyDevice, O_RDWR | O_SYNC, 0644 );
	if ( si_Fd == -1 ){
		THRL_SetThLog( 563, THRE_DebugLogType_ThCtrLog, 0, False, SYSF_errnoGet() );
		return False;
	}

	SYSF_lseek( si_Fd, THRD_SCBOffset_Version, SEEK_SET );		// バージョンの位置まで移動.

	// SCBのバージョンを取得 
	si_Ret = SYSF_read( si_Fd, psc_Version, THRD_SCBOffset_VersionSize );
	if( si_Ret == -1 ){
		SYSF_devclose(si_Fd);
		THRL_SetThLog( 564, THRE_DebugLogType_ThCtrLog, 0, False, SYSF_errnoGet(), 1 );
		return False;
	}

	SYSF_lseek( si_Fd, THRD_SCBOffset_CheckSum, SEEK_SET );		// チェックサムの位置まで移動.

	// SCBのチェックサムを取得 
	si_Ret = SYSF_read( si_Fd, sc_WkChksum, THRD_SCBOffset_CheckSumSize );
	if( si_Ret == -1 ){
		SYSF_devclose(si_Fd);
		THRL_SetThLog( 564, THRE_DebugLogType_ThCtrLog, 0, False, SYSF_errnoGet(), 2 );
		return False;
	}

	memcpy( pus_Chksum, sc_WkChksum, THRD_SCBOffset_CheckSumSize );

	SYSF_devclose(si_Fd);
#endif	/* DEF_64BIT_PF */

	return True;
#else
	return False;
#endif
}

// ================================================================================================.
//! <!--機能--> フォントファイル(.ttf)のディレクトリの文字列を取得する.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! なし
//! <!--戻値-->.
//! \return CHAR "/res/", "/mfp/" のどちらかの文字列を返す。
//! <!--詳細-->.
//! 機種がPreEmuの場合"/res/"を返す。
//! 機種がMinerva, Zeusの場合"/mfp/"を返す。
//! <!--履歴-->.
//  2011.06.02	itakura	新規作成.
//  2012/07/06  takayama    新アーキ対応.
//  2012/08/27  itakura		CQ370707 Subsetバージョンが表示されない問題関連でフォントファイルのアクセス先を修正.
// ================================================================================================.
const char* getFontPathType(void)
{
#if defined(DEF_MinervaPreEmu) || defined(DEF_ZeusRevCus)
	return FONT_PATH_RES;
#else
	return FONT_PATH_KM;
#endif
}


// ================================================================================================.
//! <!--機能--> RamDiskの作成
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! devpath		: マウント先のパス.
//! size		: サイズ.
//! <!--戻値-->.
//! なし
//! <!--詳細-->.
//! RamDiskを作成する.
//! <!--履歴-->.
//  2011.06.02	itakura	新規作成.
// ================================================================================================.
bool createRamDisk( char* devpath, int32_t size )
{
#if defined(TORNADO)
	int32_t ret;
	char cmd[256];
	char val[32];
	snprintf( val, sizeof(val), "%"PRId32"\n", size );
	snprintf( cmd, sizeof(cmd), FMT_PART_RAMDISK, devpath, val);
	ret = system(cmd);
	if( ret != 0 ) {
		return False;
	}
#endif	
	return True;
}


/*================================================================================================
［機能］	MSGバイナリのファイル名の文字列を取得する.
--------------------------------------------------------------------------------------------------
［引数］	buf=格納領域　bufsize=サイズ.
［戻値］	なし.
［詳細］	機種がZeusの場合"/res/ZEUS_MSG.bin"を返す.
            機種がMinervaBKの場合"/res/MINERVABK_MSG.bin"を返す.
            機種がMinervaの場合"/res/MINERVA_MSG.bin"を返す.
            この関数は、BlackAPIのdivlibから呼ばれる.
［ｳｫｰｸｽﾙｰ］	
--------------------------------------------------------------------------------------------------
 [履歴]
 '12/10/30 N.Suzuki	 新規作成. CQ372025.
================================================================================================*/
void THRF_getMsgBinName(char *buf, int32_t bufsize)
{
	strncpy(buf,LINUX_MSG_BIN,bufsize);
	buf[bufsize-1] = '\0';	// 終端記号の保証.
}

/*================================================================================================
［機能］	「選択言語組み合わせファイル」のパス名（ファイル名含む）を取得する。
--------------------------------------------------------------------------------------------------
［引数］	cp_filename_buf	取得するパス（ファイル名含む）を格納するバッファー(O)
			uc_bufsize			cp_filename_bufに指定したバッファーのサイズ(I)
［戻値］	なし.
［詳細］	「選択言語組み合わせファイル」のファイルパス文字列を、
			cp_filename_buf で指定されたバッファに、最大uc_bufsizeで指定されたサイズ分までコピーします。
			コピーした文字列は、バッファ先頭からuc_bufsize[Byte]進んだ位置でのNULL終端を保証します。
--------------------------------------------------------------------------------------------------
 [履歴]
 '13/09/02 N.Suzuki	 新規作成.UV-00872:29言語対応
================================================================================================*/
void THRF_getSelectLangDataFilePath(char *cp_filename_buf, uchar uc_bufsize)
{
	if ( cp_filename_buf == NULL || uc_bufsize < 1 ) {
		printf("%s(cp_filename_buf=%p uc_bufsize = %d)\n",__FUNCTION__,cp_filename_buf,uc_bufsize);
		return;
	}
	strncpy(cp_filename_buf ,THRD_SELECT_LANG_DATA , uc_bufsize);
	cp_filename_buf[uc_bufsize-1] = '\0';	// 終端記号の保証.
}


/*================================================================================================
［機能］	シンボリックリンクの切り替え.
--------------------------------------------------------------------------------------------------
［引数］	TYPE_PHType	エンジンのPH構成情報.
［戻値］	Bool 	True:切り替え成功.
					False:切り替え失敗.
［履歴］	2013/07/08 itakura 緊急CD向け1ビーム/2ビーム切替対応.
			2013/08/08 itakura CQ382291 コンパイルスイッチ対応.
			2017/09/14 Sky井柳 [Eagle][制御構成変更No5] コンパイルスイッチ変更(条件削除).
================================================================================================*/
Bool THRG_changeIPROMSymLink( TYPE_PHType sc_EngPH )
{
#if (defined TORNADO) && (!defined FUM_LINUX)		// 実機で有効.
	char sc_PicCmd[THRD_MakeSymLinkCmdLength] = {0};
	char sc_ThrCmd[THRD_MakeSymLinkCmdLength] = {0};
	int32_t si_Ret = -1;
	
	// 引数チェック.
	if ( sc_EngPH != TYPE_PHType1Beam && sc_EngPH != TYPE_PHType2Beam ) {
		printf(" THRG_changeIPROMSymLink: invalid parameter %d \n", sc_EngPH );
		return False;
	}

	// 既存の/res領域内シンボリックリンクを削除.
	// もともと存在しなかったことを想定して、削除に失敗してもFalseを返さない.
	si_Ret = remove(THRD_PicIpromSymlink);
	if ( si_Ret != 0 ) {
		printf( "THRG_changeIPROMSymLink:remove %s Error. \n", THRD_PicIpromSymlink );
	}
	si_Ret = remove(THRD_ThrIpromSymlink);
	if ( si_Ret != 0 ) {
		printf( "THRG_changeIPROMSymLink:remove %s Error. \n", THRD_PicIpromSymlink );
	}

	// シンボリックリンク作成コマンド作成.
	if ( sc_EngPH == TYPE_PHType1Beam ) {
		snprintf( sc_PicCmd, sizeof(sc_PicCmd), "exec ln -s  %s %s", THRD_1BeamPicIpromFilePath, THRD_PicIpromSymlink );
		snprintf( sc_ThrCmd, sizeof(sc_ThrCmd), "exec ln -s  %s %s", THRD_1BeamThrIpromFilePath, THRD_ThrIpromSymlink );
	} else {	// TYPE_PHType2Beam.
		snprintf( sc_PicCmd, sizeof(sc_PicCmd), "exec ln -s  %s %s", THRD_2BeamPicIpromFilePath, THRD_PicIpromSymlink );
		snprintf( sc_ThrCmd, sizeof(sc_ThrCmd), "exec ln -s  %s %s", THRD_2BeamThrIpromFilePath, THRD_ThrIpromSymlink );
	}

	si_Ret = system( sc_PicCmd );
	if ( si_Ret != 0 ) {
		printf( "THRG_changeIPROMSymLink %s Error. \n", sc_PicCmd );
		return False;
	}
	
	si_Ret = system( sc_ThrCmd );
	if ( si_Ret != 0 ) {
		printf( "THRG_changeIPROMSymLink %s Error. \n", sc_ThrCmd );
		return False;
	}
#endif	// 実機で有効.
	return True;
}

/*================================================================================================
［機能］	画パラファイルのPHビーム情報を返す.
--------------------------------------------------------------------------------------------------
［引数］	void.
［戻値］	char	-1:画パラファイル異常(1ビーム・2ビームいずれでもない).
					0:2ビーム.
					1:1ビーム.
［履歴］	2013/07/08 itakura 緊急CD向け1ビーム/2ビーム切替対応.
			2013/08/08 itakura CQ382291 コンパイルスイッチ対応.
================================================================================================*/
char getSSDIPRomInfo()
{
#if (defined TORNADO) && (!defined FUM_LINUX)		// 実機で有効.
#if defined( DEF_ZeusMLKCus ) || defined( DEF_ZeusMLKEmu )
	return THRG_getSSDIPROMInfo();
#else	// defined( DEF_ZeusMLKCus ) || defined( DEF_ZeusMLKEmu ).
	return THRD_SsdIpromVersion2Beam;
#endif
#else	// (defined TORNADO) && (!defined FUM_LINUX).
	return THRD_SsdIpromVersion2Beam;
#endif
}

// ================================================================================================.
//! <!--機能--> コンストラクタ.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
//  09/08/07 越野	 Donau環境から分離して作成.
// 11/05/27 koshimo CQ.346617 混載状態の初期化追加.
// 12/08/31 Gonsho	静的解析対応 未初期化のメンバ変数を初期化.
// 12/10/01 yamauchi UV-01173、CQ.364064 Box内プリントデータのCPDF化対応.
// 12/02/28 Sky 機種共通化対応 THRC_ThreadifSoC.cppから移植.
// 13/04/10 KITAZAWA UV-1235 属性解像度変換パラメータ切り替え対応.
// 15/04/24 Sky鈴木理 CQ510610 Helios[FVT2] 静的解析対応 初期化漏れ.
// 15/08/24	Yamashita 静的解析対応 b_1PassDuplexJobの初期化追加.
// 16/03/18 Sky スキュー補正対応.
// 17/06/29 Sky OP/PP共通化.ページモード、コントローラー種別、バッファ振り分け番号の初期化.
// ================================================================================================.
ST_ThreadComPara::ST_ThreadComPara()
{
	bl_BandEndFlg = False;
//2006/09/25 Fujii(Sky) 長尺プリントで無画像バンド対応 (S).
	b_NoImgBand[0] = False;
	b_NoImgBand[1] = False;
//2006/09/25 Fujii(Sky) 長尺プリントで無画像バンド対応 (E).

	b_ExpandInCompScan = False;	//入力側EDHFCOTフラグ.
	e_Consolidation_State = THRE_Not_Consolidation;	// 非混載で初期化.

//	→ 静的解析対応(CIM22440) 2011/05/24 saka.
	pcc_OneImageOutputMode = NULL;					//	出力モードへのポインタ.
	e_FaxImageCompress = IMGE_ImageCompress_Non;		// FAX圧縮情報(FAX圧縮時に使用する).
	e_FaxLineID = EN_FaxLineID_Unknown;					// FAX回線/iFAX情報.
	e_JpegCompressionLevel = TYPE_JpegCompressionLevelStandard;		// JPEG圧縮圧縮レベル.
	e_ThFileFormat = TYPE_FileFormatPdf;				//JPEGASICファイルフォーマット種別.
	en_ColorElementNumber = EN_ColorElement_1;				// カラー番号を指定する.
	en_JpegAsicMode = EN_ThJpegAsicNone;					//JPEGASICの使用方法を指定する.
	en_JpegResolMode = EN_ThJpegResolEqual;
	en_ThreadImgProcMode = THRE_ImgProcMode_None;			// 画像処理モードを指定する.
	memset( &s_TIFFCInfo, 0.0f, sizeof( s_TIFFCInfo ));		// TiFF-Cヘッダ情報(Lab上下限値).
	memset( sc_SysDescr, 0, sizeof( sc_SysDescr ));			// 製品名.
	ss_BandID = 0;									//	バンドＩＤ.
	ss_TransIndexNum = 0;							//	画像転送管理情報のインデックス番号.
	uc_FaxOutSwapType = 0;							// 圧縮/伸張用出力側スワップ対応.
	ul_AsicToolAppMemSize = 0;
	memset( ul_AsicToolCompSize, 0, sizeof( ul_AsicToolCompSize ));
//	← 静的解析対応.
	
	e_ImagePlace = THRE_ImagePlaceMainMemory;
	b_TonerSave  = False;
	b_EdgeEmp    = False;
	b_EnableDensityRevisionTable = False;

//  2012/10/01 yamauchi UV-01173、CQ.364064 Box内プリントデータのCPDF化対応.
	b_ForcingAttExpandMode = False;					// 属性伸張の強制実行フラグ.

	b_AttResoCnv = False;							// 属性解像度変換パラメータ切り替え指示(False:間引き処理(既存)).
	e_Nin1 = TYPE_Nin1_1in1;
	e_PageShot = TYPE_PageShotNone;
	b_AMSMix = False;
	b_1PassDuplexJob = False;
	b_SkewCorrection = False;
	// OP/PP共通化.	
	b_isPageMode  = False;				// ページモード.
	i64_selBufNo  = THRD_NOUSE_ENGBUF;	// バッファ割り振り番号.
	e_ExInCTLMode = THRE_ExInCTL_OPFiery;	// コントローラー種別.
}

// ================================================================================================.
//! 【スレッドとPICのIF】合成画像アドレスをPICに渡す.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param int		opt:用途未定.
//! <!--戻値-->.
//! \return THRS_ImagePatternBufferAdr	合成画像アドレス構造体.
//! <!--詳細-->.
//! \common\system/thr/Asic1200\THRC_Asic1200EngOutputSeq.cppで定義  \n.
//! ｔｓｋｐｉｃからの関数コールにて設定 \n.
//! \n.
//! 本Ｉ／Ｆを利用して、合成画像アドレスをPICへ通知する。\n.
//! <!--履歴-->.
//  09/08/07 越野	 Donau環境から分離して作成.
// 12/02/28 Sky 機種共通化対応 THRC_ThreadifSoC.cppから移植.
// ================================================================================================.
THRS_ImagePatternBufferAdr getPatternBufferAdr(int32_t opt)
{
	THRS_ImagePatternBufferAdr s_ImagePatternBufferAdr;
	memset(&s_ImagePatternBufferAdr,0,sizeof( THRS_ImagePatternBufferAdr ) );
	uint32_t ul_Size = 0;

	// 各値に情報を設定していく。.
	// 領域は下記のようなものを考えている。.
	// +----------------------------+  ←THRE_BufId_Main_Combine.
	// |  ページ領域1				|.
	// +----------------------------+.
	// |  ページ領域2				|.
	// +----------------------------+.
	// |  日付領域					|.
	// +----------------------------+.
	// |  スタンプ領域				|.
	// +----------------------------+  ←THRE_BufId_Main_CombineJimon.
	// |  地紋・ウォーターマーク領域|.
	// +----------------------------+.

	// 合成画像用バッファ.
	THRC_BufferResource* pst_BufResource = THRC_BufMemorySingletonIf::newInstance().getBufferResourceInstance( THRE_BufId_Main_Combine );
	//  地紋・ウォーターマーク用バッファ.
	THRC_BufferResource* pst_BufResourceJimon = THRC_BufMemorySingletonIf::newInstance().getBufferResourceInstance( THRE_BufId_Main_CombineJimon );
	
	// 合成画像用バッファ有り.
	if( pst_BufResource != NULL ){
		// ページ番号1.
		s_ImagePatternBufferAdr.ul_PageStampBuffer[0] = pst_BufResource->getAddr();
		//  地紋・ウォーターマーク用バッファ有り.
		if( pst_BufResourceJimon != NULL ){
			// 地紋.
			s_ImagePatternBufferAdr.ul_JimonBuffer = pst_BufResourceJimon->getAddr();
			// 差分計算.
			ul_Size = ( s_ImagePatternBufferAdr.ul_JimonBuffer - s_ImagePatternBufferAdr.ul_PageStampBuffer[0] ) / 5;
		//  地紋・ウォーターマーク用バッファ無し.
		}else{
			// 差分計算.
			ul_Size = pst_BufResource->getSizeOfMemory() / 5;
		}
		// ページ番号2.
		s_ImagePatternBufferAdr.ul_PageStampBuffer[1] = s_ImagePatternBufferAdr.ul_PageStampBuffer[0] + ul_Size;
		// 日付.
		s_ImagePatternBufferAdr.ul_DateStampBuffer = s_ImagePatternBufferAdr.ul_PageStampBuffer[1] + ul_Size;
		// スタンプ.
		s_ImagePatternBufferAdr.ul_StampBuffer = s_ImagePatternBufferAdr.ul_DateStampBuffer + ul_Size;
	// 合成画像用バッファ無し.
	}else{
		//  地紋・ウォーターマーク用バッファ有り.
		if( pst_BufResourceJimon != NULL ){
			// 地紋.
			s_ImagePatternBufferAdr.ul_JimonBuffer = pst_BufResourceJimon->getAddr();
		}
	}

	THRL_SetThLog( 66, THRE_DebugLogType_Combine, 255, False,s_ImagePatternBufferAdr.ul_DateStampBuffer );

	return( s_ImagePatternBufferAdr );

}

// ================================================================================================.
//! 【スレッド内部で用いるIF】回転バッファ内のEng用領域とApp用領域のアドレスを取得.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param uchar		uc_UseType THRD_ROTBUF_USETYPE_ENG	0:エンジン用.
//! 					uc_UseType THRD_ROTBUF_USETYPE_APP	1:アプリ用.
//! <!--戻値-->.
//! \return ulong		回転バッファ内で区切ったアドレス.
//!.
//! <!--履歴-->.
//  09/08/07 越野	 Donau環境から分離して作成.
// 12/02/28 Sky 機種共通化対応 THRC_ThreadifSoC.cppから移植.
// ================================================================================================.
void getPatternBufferRotAdr(   uchar	uc_UseType		// 入力 Eng出力で使う/App出力で使う?.
							 , uintptr_t&	rul_BufAddr		//.
							 , uint32_t&	rul_BufSize
)
{

	// 範囲チェック.
	THRL_ThAssert( ( uc_UseType <= THRD_ROTBUF_USETYPE_APP ), "getPatternBufferRotAdr() uc_UseType = %d" ,uc_UseType , 0, 0, 0, CFA14_DetailCode0x08AF );

	// バッファサイズの算出（エンジンとアプリで半分ずつ使うので、取得サイズを1/2したもの.
	rul_BufSize = THRC_BufMemorySingletonIf::newInstance().getBufferResourceInstance( THRE_BufId_Main_Rot )->getSizeOfMemory() / 2;
	// バッファサイズチェック.
	THRL_ThAssert( ( rul_BufSize != 0  ), "getPatternBufferRotAdr() rul_BufSize = 0" , 0, 0, 0, 0, CFA14_DetailCode0x08B0 );

	// アドレスの算出.
	if( uc_UseType == THRD_ROTBUF_USETYPE_ENG ){
		// 地紋用回転バッファアドレス.
		rul_BufAddr = THRC_BufMemorySingletonIf::newInstance().getBufferResourceInstance( THRE_BufId_Main_Rot )->getAddr();
	} else if(  uc_UseType == THRD_ROTBUF_USETYPE_APP ){
		// アプリの定型スタンプ.
		rul_BufAddr = THRC_BufMemorySingletonIf::newInstance().getBufferResourceInstance( THRE_BufId_Main_Rot )->getAddr()
						+ rul_BufSize;
	}
	
	// アドレスチェック.
	THRL_ThAssert( ( rul_BufAddr != 0  ), "getPatternBufferRotAdr() rul_BufAddr = 0" , 0, 0, 0, 0, CFA14_DetailCode0x08B1 );
}

// ================================================================================================.
//! <!--機能--> 多値合成(フルカラー合成)するかどうか.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param const TYPC_Resolution& rc_Resolution		解像度.
//! <!--戻値-->.
//! \return True : 多値合成する。False : 多値合成しない(2値合成)。.
//! <!--詳細-->.
//! エンジン出力の場合、多値合成ならばスレッドで合成し、2値合成ならばPIC で合成する。\n.
//! よって、prt - PIC のIFに関しては、本関数にて判断を行って頂く。\n.
//! <!--履歴-->.
//  09/08/07 越野	 Donau環境から分離して作成.
// 12/02/28 Sky 機種共通化対応 THRC_ThreadifSoC.cppから移植.
// ================================================================================================.
Bool THRF_IsSynthesisWithFullColor( const TYPC_Resolution& rc_Resolution )
{
	TYPC_Resolution c_Resolution = rc_Resolution;
	if( c_Resolution.getDpi().x == 1200.0f && c_Resolution.getDpi().y == 1200.0f ) {
		return False;					// ニ値合成.
	} else {
		return True;					// 多値合成.
	}
}

// ================================================================================================.
//! <!--機能--> AesのDMAが存在するかどうか.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return True：存在する False：存在しない.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
//  09/08/07 越野	 Donau環境から分離して作成.
// 12/02/28 Sky 機種共通化対応 THRC_ThreadifSoC.cppから移植.
// ================================================================================================.
Bool THRF_isAesDmaExist()
{
	// Donau では存在する.
	return True;
}

// ================================================================================================.
//! <!--機能--> 【スレッドとPrtのIF】伸張情報へのカラー情報設定、及びPICへ渡すカラーモード判定.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param ENGC_OnePPage* pc_OPP 出力画像情報.
//! <!--戻値-->.
//! \return 最終的に出力するカラーモード.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
//  09/08/07 越野	 Donau環境から分離して作成.
//  09/12/16 大本	 CQ323170:スレッドワーニング除去対応.
//  12/06/21 佐々木  1200dpiパフォーマンス改善(getImage関数のコール数削減).
//  12/07/10 仲川    静的解析対応.
// 12/02/28 Sky 機種共通化対応 THRC_ThreadifSoC.cppから移植.
//  16/03/18 Sky     バーコード属性対応.
// ================================================================================================.
void THRF_setOPPColorInfo( ENGC_OnePPage* pc_OPP )
{
	THRL_ThAssert( (pc_OPP != NULL), "THRF_setOEMColorInfo OPP error!" , 0, 0, 0, 0, CFA14_DetailCode0x08B2 );
	THRL_ThAssert( (pc_OPP->getpOPO() != NULL), "THRF_setOEMColorInfo OPO error!" , 0, 0, 0, 0, CFA14_DetailCode0x08B3 );

	CC_OneExpandMode& rc_OEM = pc_OPP->getpOPO()->OEM;

	// 印字すべき対象がある時だけ設定するようにする.
	if( rc_OEM.isNonPrint() == False ){
		IMGE_ImageColor e_ResultImageColor = IMGE_ImageColor_Max;
		IMGE_ImageTone e_ImgTone = IMGE_ImageTone_24bit;
		register uchar ucPos = 0;
		Bool b_isBlack = pc_OPP->getBlReliefColorPrint();	// 初期値は強制モノクロ印字フラグにする.

		if( us_debug_ForceBlack > 0 ){
			// デバッグ用強制モノクロ印字フラグオン.
			b_isBlack = True;
		}

		// まず、揃えるべきカラーを決定.
		IMGE_ImageColor e_Black = IMGE_ImageColor_Max;
		IMGE_ImageColor e_Color = IMGE_ImageColor_Max;
		TYPE_ColorMode e_RequestedColorMode = TYPE_ColorModeBlack;	// 出力要求されている、原稿のカラーモード。初期値をブラックとしておく.
		for( ucPos=0; ucPos < InputPageNumMax; ucPos++ ){
			if( rc_OEM.III[ucPos].pOIP != NULL ){
				IMGC_Image* pc_Image = NULL;	//Imageインスタンスアドレス.

				pc_Image = rc_OEM.III[ucPos].pOIP->getImage();
				THRL_ThAssert( ( pc_Image != NULL ), "THRF_setOPPColorInfo(): getImage NULL error." , 0, 0, 0, 0, CFA14_DetailCode0x08B4 );
				// 有効データ発見.
				IMGE_ImageColor e_NextColor = pc_Image->c_ImageInfo.ec_ImageColorResult;

				if( e_ImgTone != IMGE_ImageTone_24bit ){
					// 【課題】05/12/27 yokoi 一面に階調が混載する事は現状ないはず.
					THRL_ThAssert( (e_ImgTone == pc_Image->c_ImageInfo.ec_ImageTone), "THRF_setOEMColorInfo ImageTone" , 0, 0, 0, 0, CFA14_DetailCode0x08B5 );
				}
				if( theColorInformation.isBlackMode( pc_Image->c_ImageInfo.ec_ColorMode ) == True ){
					if( e_Black != IMGE_ImageColor_Max ){
						// 小冊子バーコード補正あり/なしの混載で、2番目がバーコード補正なしの場合は色を変える.
						// 1番目がバーコード補正なしの場合は、2番目の色が優先される.
						if( theModeInfoLib.chk1200Dpi( rc_OEM.c_Resolution ) == True ) {
							if( e_Black == IMGE_ImageColor_Density_2 && e_NextColor == IMGE_ImageColor_Density_1 ) {
								e_NextColor = IMGE_ImageColor_Density_2;
							}
						} else {
							if( e_ResultImageColor != IMGE_ImageColor_Max ){	// 無意味な条件であるがIT5にあわせる.
								// 【課題】05/12/27 yokoi 一面にﾌﾞﾗｯｸの色が混載する事は現状ないはず.
								THRL_ThAssert( (e_Black == e_NextColor), "THRF_setOEMColorInfo ColorNg %d != %d", e_Black ,e_NextColor , 0, 0, CFA14_DetailCode0x08B6 );
							}
						}
					}
					e_Black = e_NextColor;
					e_ImgTone = pc_Image->c_ImageInfo.ec_ImageTone;
				} else {
					if( e_Color != IMGE_ImageColor_Max ){
						// 小冊子バーコード補正あり/なしの混載で、2番目がバーコード補正なしの場合は色を変える.
						// 1番目がバーコード補正なしの場合は、2番目の色が優先される.
						if( theModeInfoLib.chk1200Dpi( rc_OEM.c_Resolution ) == True ) {
							if( e_Color == IMGE_ImageColor_YMCK_Att2 && e_NextColor == IMGE_ImageColor_YMCK_Att1 ) {
								e_NextColor = IMGE_ImageColor_YMCK_Att2;
							}
						} else {
							if( e_ResultImageColor != IMGE_ImageColor_Max ){	// 無意味な条件であるがIT5にあわせる.
								// 【課題】05/12/27 yokoi 一面にﾌﾙｶﾗｰの色が混載する事は現状ないはず.
								THRL_ThAssert( (e_Color == e_NextColor), "THRF_setOEMColorInfo ColorNg %d != %d", e_Color ,e_NextColor , 0, 0, CFA14_DetailCode0x08B7 );
							}
						}
					}
					e_Color = e_NextColor;
					e_ImgTone = pc_Image->c_ImageInfo.ec_ImageTone;
				}
				if( rc_OEM.III[ucPos].e_OriginalColorMode == TYPE_ColorModeFullColor ){
					// 一つでもカラーがあれば、全面カラー要求とする.
					e_RequestedColorMode = TYPE_ColorModeFullColor;
				}
			}
		}
		if( e_RequestedColorMode == TYPE_ColorModeBlack ){
			// カラーモードの要求が無い.
			b_isBlack = True;
		}

		// この時点で、カラー/ブラックの判断が決まっているはず！.
		// 強制モノクロ印字または、用紙一面にカラー画像が無い場合はブラック.
		// 用紙一面にカラー画像が一つでもあり、強制モノクロ印字でなければカラーとしている。.

		//ログ出力用.
		CC_MultiDataLog		cc_CC_MultiDataLog;
		cc_CC_MultiDataLog = (uint32_t)e_Color;
		ThLogRecord("e_Color = %d\n",&cc_CC_MultiDataLog);
		cc_CC_MultiDataLog = (uint32_t)e_Black;
		ThLogRecord("e_Black = %d\n",&cc_CC_MultiDataLog);
		cc_CC_MultiDataLog = (uint32_t)e_ImgTone;
		ThLogRecord("e_ImgTone = %d\n",&cc_CC_MultiDataLog);
		cc_CC_MultiDataLog = (uint32_t)b_isBlack;
		ThLogRecord("b_isBlack = %d\n",&cc_CC_MultiDataLog);
		
		// モードに応じたPICへ渡す色を取得.
		e_ResultImageColor = theColorInformation.getColorForPic( e_Color, e_Black, e_ImgTone, b_isBlack );
		// 多値合成時、Light:RGB Density:YMCKで出力するようにする.
		Bool b_isSynthesis = ( theModeInfoLib.getOverlayType( rc_OEM ) == THRE_OverlayType_Synthesis ) ? True : False;

#if !(defined(COSIM)) || !(defined(THR_SIMWIN))
		APIC_SystemStatus* pc_SystemStatus = APIC_SystemStatus::newInstance();							// システムステータス ・インスタンス取得.
#endif
		if ( b_isSynthesis == True ){	// 多値合成の場合.
#if defined(COSIM) && defined(THR_SIMWIN)
			// COSIM 1st./2nd検証では多値合成 暫定救済は行わない為、処理をスルー.
			if( False ){ 
#else
			if( pc_SystemStatus->getTemporaryRescueUserOn( TYPE_TemporaryRescueUnitColorDev ) == True ){
#endif
				// 暫定救済動作.
				// 暫定救済時は、PICへ渡す色をRGB色にする.
				switch( e_ResultImageColor ){
					case IMGE_ImageColor_Light:						// Donau/Amurでは、未使用.
					case IMGE_ImageColor_Density:					// Fieryプリント グレースケール(Edge Enhancement OFF).
						e_ResultImageColor = IMGE_ImageColor_RGB;
						break;
					case IMGE_ImageColor_Light_2:					// PCLプリント グレースケール.
					case IMGE_ImageColor_Density_2:					// PSプリント グレースケール.
					case IMGE_ImageColor_YMCK_Att2:					// PSプリント(Amur) グレースケール.
						e_ResultImageColor = IMGE_ImageColor_RGB_ATT2;
						break;
					case IMGE_ImageColor_Density_1:					// Fieryプリントグレースケール(Edge Enhancement ON).
						e_ResultImageColor = IMGE_ImageColor_RGB_ATT1;
						break;
					default:
						break;
				}
			}else{
					// 通常動作.
				switch( e_ResultImageColor ){
					case IMGE_ImageColor_Light:						// Donau/Amurでは、未使用.
						e_ResultImageColor = IMGE_ImageColor_RGB;
						break;
					case IMGE_ImageColor_Light_2:					// PCLプリント グレースケール.
						e_ResultImageColor = IMGE_ImageColor_RGB_ATT2;
						break;
					//YMCK強制印字対応 density時はYMCKで出力 Nakayama.
					case IMGE_ImageColor_Density:					// Fieryプリント グレースケール(Edge Enhancement OFF).
						if( theModeInfoLib.chk1200Dpi( rc_OEM.c_Resolution ) == False ) {
							e_ResultImageColor = IMGE_ImageColor_YMCK;
						}
						break;
					case IMGE_ImageColor_Density_1:					// Fieryプリント グレースケール(Edge Enhancement ON).
						e_ResultImageColor = IMGE_ImageColor_YMCK_Att1;
						break;
					case IMGE_ImageColor_Density_2:					// PSプリント グレースケール.
						e_ResultImageColor = IMGE_ImageColor_YMCK_Att2;
						break;
					default:
						break;
				}
			}
		}
		else{	// 多値合成以外で、グレー補償OFFでモノクロ原稿をオートカラーモードで印刷した場合.
#if defined(COSIM) && defined(THR_SIMWIN)
			// COSIM 1st/2nd検証 暫定救済時.
			if( us_debug_ForceBlack == True ){
#else			
			if( pc_SystemStatus->getTemporaryRescueUserOn( TYPE_TemporaryRescueUnitColorDev ) == True ){
#endif
				// 暫定救済動作.
				// 暫定救済時は、PICへ渡す色をRGB色にする.
				switch( e_ResultImageColor ){
					case IMGE_ImageColor_YMCK_Att2:					// PSプリント(Amur) グレースケール.
						// 1200dpiのYMCK_Att2はバーコード属性用のため除外する（本来 YMCK_Att1のため）.
						if( theModeInfoLib.chk1200Dpi( rc_OEM.c_Resolution ) == False ) {
							e_ResultImageColor = IMGE_ImageColor_RGB_ATT2;
						}
						break;
					default:
						break;
				}
			}
		}

//////////////////////// スレッドとのIF用にパラメータ設定.
		// OEM に、パラメータを設定.
		for( ucPos=0; ucPos < InputPageNumMax; ucPos++ ){
			if( rc_OEM.III[ucPos].pOIP != NULL ){
				rc_OEM.III[ucPos].ec_ImageColor = e_ResultImageColor;
				rc_OEM.III[ucPos].ec_ImageTone = e_ImgTone;
				if( (b_isSynthesis == True) && (b_isBlack == True) ){
					// 多値合成で、原稿ブラックにする場合、スレッドでブラックにする.
					// 【課題】Nin1の一つだけBlackにする、などが発生すれば、スレッドでブラックにする対応をいれる必要がある.
					rc_OEM.III[ucPos].e_OriginalColorMode = TYPE_ColorModeBlack;
				} else {
					// 通常は、そのまま出力する.
					rc_OEM.III[ucPos].e_OriginalColorMode = TYPE_ColorModeFullColor;
				}
			}
		}
//////////////////////// PICとのIF用にパラメータ設定.
		// 最終的なカラーモードを決定.
		if( (b_isBlack == True)
			&& (theModeInfoLib.chkCombineBlackMode( rc_OEM ) == True) ){
			// 2006/03/23 yokoi add.
			// スレッドはブラックだが、合成画像がカラーの場合、上位にはカラーの設定をする必要があるので.
			// ここでチェックしておく.
			// 逆のパターン(スレッドはカラーで上位がブラック)はありえないので、そのケースはかかない.
			pc_OPP->getpOPO()->ec_OutputColorMode = TYPE_ColorModeBlack;
		} else {
			pc_OPP->getpOPO()->ec_OutputColorMode = (TYPE_ColorMode)(( pc_OPP->getBlReliefColorPrint() == True ) ? TYPE_ColorModeBlack : TYPE_ColorModeFullColor);
		}
		// 属性ビット数も決定.
		//! 1200dpiの場合、1200dpi用の色テーブル情報を使用するため、引数に判定フラグを追加(なお、1200dpiのときの属性数は0).
		pc_OPP->getpOPO()->uc_InputImageAttBit = theColorInformation.calcAttributeNum( e_ResultImageColor, theModeInfoLib.chk1200Dpi( rc_OEM.c_Resolution ) );
		// PICへ出力する原稿のカラーモードを設定.
		if( (b_isBlack == True) && (b_isSynthesis == False) ){
			// 黒要求で、多値合成でなければ、PICに黒変換してもらう.
			pc_OPP->getpOPO()->ec_ColorMode = TYPE_ColorModeBlack;
		} else {
			// 多値オーバーレイでフォーム画像と原稿画像がブラックなら、PICにブラックデータとして渡す.
			if( (b_isBlack == True) && (b_isSynthesis == True) && (theModeInfoLib.chkCombineBlackMode( rc_OEM ) == True) ){
				pc_OPP->getpOPO()->ec_ColorMode = TYPE_ColorModeBlack;
			}else{
				// カラー要求、又は多値合成ありなら、PICにカラーデータとして渡す.
				pc_OPP->getpOPO()->ec_ColorMode = TYPE_ColorModeFullColor;
			}
		}
	}
//Str【Mosel対応 : K.Natsume : 2006.06.28 : No.62766】.
	else
	{
		// 無印字指定で初期化.
		pc_OPP->getpOPO()->ec_OutputColorMode = TYPE_ColorModeNoColor;

		TYPE_ColorKind e_ColorKind1 = TYPE_ColorKind_ERR;							// 色指定なし.
		TYPE_ColorKind e_ColorKind2 = TYPE_ColorKind_ERR;							// 色指定なし.
		TYPE_ColorKind e_ColorKind3 = TYPE_ColorKind_ERR;							// 色指定なし.

		if( rc_OEM.s_DateStamp.s_DateStamp1PageInfo[0].b_On == True					// 日付１.
		 || rc_OEM.s_DateStamp.s_DateStamp1PageInfo[1].b_On == True					// 日付２.
		)
		{
			if( rc_OEM.s_DateStamp.e_ColorKind != TYPE_ColorKind_ERR )
			{
				if( e_ColorKind1 == TYPE_ColorKind_ERR )
				{
					e_ColorKind1 = rc_OEM.s_DateStamp.e_ColorKind;
				}
				else if( e_ColorKind2 == TYPE_ColorKind_ERR && e_ColorKind1 != rc_OEM.s_DateStamp.e_ColorKind )
				{
					e_ColorKind2 = rc_OEM.s_DateStamp.e_ColorKind;
				}
				else if( e_ColorKind3 == TYPE_ColorKind_ERR && e_ColorKind1 != rc_OEM.s_DateStamp.e_ColorKind && e_ColorKind2 != rc_OEM.s_DateStamp.e_ColorKind )
				{
					e_ColorKind3 = rc_OEM.s_DateStamp.e_ColorKind;
				}
			}
		}
		if( rc_OEM.s_PageStamp.s_PageStamp1PageInfo[0].b_On == True					// ページ１.
		 || rc_OEM.s_PageStamp.s_PageStamp1PageInfo[1].b_On == True					// ページ２.
		)
		{
			if( rc_OEM.s_PageStamp.e_ColorKind != TYPE_ColorKind_ERR )
			{
				if( e_ColorKind1 == TYPE_ColorKind_ERR )
				{
					e_ColorKind1 = rc_OEM.s_PageStamp.e_ColorKind;
				}
				else if( e_ColorKind2 == TYPE_ColorKind_ERR && e_ColorKind1 != rc_OEM.s_PageStamp.e_ColorKind )
				{
					e_ColorKind2 = rc_OEM.s_PageStamp.e_ColorKind;
				}
				else if( e_ColorKind3 == TYPE_ColorKind_ERR && e_ColorKind1 != rc_OEM.s_PageStamp.e_ColorKind && e_ColorKind2 != rc_OEM.s_PageStamp.e_ColorKind )
				{
					e_ColorKind3 = rc_OEM.s_PageStamp.e_ColorKind;
				}
			}
		}
		if( rc_OEM.s_Stamp.s_Stamp1PageInfo[0].b_On == True							// 定型スタンプ１.
		 || rc_OEM.s_Stamp.s_Stamp1PageInfo[1].b_On == True							// 定型スタンプ２.
		)
		{
			if( rc_OEM.s_Stamp.e_ColorKind != TYPE_ColorKind_ERR )
			{
				if( e_ColorKind1 == TYPE_ColorKind_ERR )
				{
					e_ColorKind1 = rc_OEM.s_Stamp.e_ColorKind;
				}
				else if( e_ColorKind2 == TYPE_ColorKind_ERR && e_ColorKind1 != rc_OEM.s_Stamp.e_ColorKind )
				{
					e_ColorKind2 = rc_OEM.s_Stamp.e_ColorKind;
				}
				else if( e_ColorKind3 == TYPE_ColorKind_ERR && e_ColorKind1 != rc_OEM.s_Stamp.e_ColorKind && e_ColorKind2 != rc_OEM.s_Stamp.e_ColorKind )
				{
					e_ColorKind3 = rc_OEM.s_Stamp.e_ColorKind;
				}
			}
		}
		if( rc_OEM.s_JimonPatternIn.uc_JimonFlagTheread == EN_JimonFlagEnable			// 地紋１.
		 || rc_OEM.s_JimonPatternIn.uc_JimonFlagTheread == EN_JimonFlagContinue		// 地紋２.
		)
		{
			if( rc_OEM.s_JimonPatternIn.e_ColorKind != TYPE_ColorKind_ERR )
			{
				if( e_ColorKind1 == TYPE_ColorKind_ERR )
				{
					e_ColorKind1 = rc_OEM.s_JimonPatternIn.e_ColorKind;
				}
				else if( e_ColorKind2 == TYPE_ColorKind_ERR && e_ColorKind1 != rc_OEM.s_JimonPatternIn.e_ColorKind )
				{
					e_ColorKind2 = rc_OEM.s_JimonPatternIn.e_ColorKind;
				}
				else if( e_ColorKind3 == TYPE_ColorKind_ERR && e_ColorKind1 != rc_OEM.s_JimonPatternIn.e_ColorKind && e_ColorKind2 != rc_OEM.s_JimonPatternIn.e_ColorKind )
				{
					e_ColorKind3 = rc_OEM.s_JimonPatternIn.e_ColorKind;
				}
			}
		}
		if( rc_OEM.s_OverLayInfo.b_isOverLay == True )								// オーバーレイ情報.
		{
			if( rc_OEM.s_OverLayInfo.e_ColorKind != TYPE_ColorKind_ERR )
			{
				if( e_ColorKind1 == TYPE_ColorKind_ERR )
				{
					e_ColorKind1 = rc_OEM.s_OverLayInfo.e_ColorKind;
				}
				else if( e_ColorKind2 == TYPE_ColorKind_ERR && e_ColorKind1 != rc_OEM.s_OverLayInfo.e_ColorKind )
				{
					e_ColorKind2 = rc_OEM.s_OverLayInfo.e_ColorKind;
				}
				else if( e_ColorKind3 == TYPE_ColorKind_ERR && e_ColorKind1 != rc_OEM.s_OverLayInfo.e_ColorKind && e_ColorKind2 != rc_OEM.s_OverLayInfo.e_ColorKind )
				{
					e_ColorKind3 = rc_OEM.s_OverLayInfo.e_ColorKind;
				}
			}
		}
		if( rc_OEM.s_FilingNumberInfo.s_FilingNumber1PageInfo[0].b_On == True			// ファイリングナンバー１.
		 || rc_OEM.s_FilingNumberInfo.s_FilingNumber1PageInfo[1].b_On == True			// ファイリングナンバー２.
		)
		{
			if( rc_OEM.s_FilingNumberInfo.e_ColorKind != TYPE_ColorKind_ERR )
			{
				if( e_ColorKind1 == TYPE_ColorKind_ERR )
				{
					e_ColorKind1 = rc_OEM.s_FilingNumberInfo.e_ColorKind;
				}
				else if( e_ColorKind2 == TYPE_ColorKind_ERR && e_ColorKind1 != rc_OEM.s_FilingNumberInfo.e_ColorKind )
				{
					e_ColorKind2 = rc_OEM.s_FilingNumberInfo.e_ColorKind;
				}
				else if( e_ColorKind3 == TYPE_ColorKind_ERR && e_ColorKind1 != rc_OEM.s_FilingNumberInfo.e_ColorKind && e_ColorKind2 != rc_OEM.s_FilingNumberInfo.e_ColorKind )
				{
					e_ColorKind3 = rc_OEM.s_FilingNumberInfo.e_ColorKind;
				}
			}
		}
		if( rc_OEM.s_HeaderFooterInfo.s_Header.s_HeaderFooter1PageInfo[0].b_isSetHeaderFooter == True		// ヘッダ１.
		 || rc_OEM.s_HeaderFooterInfo.s_Header.s_HeaderFooter1PageInfo[1].b_isSetHeaderFooter == True		// ヘッダ２.
		 || rc_OEM.s_HeaderFooterInfo.s_Footer.s_HeaderFooter1PageInfo[0].b_isSetHeaderFooter == True		// フッタ１.
		 || rc_OEM.s_HeaderFooterInfo.s_Footer.s_HeaderFooter1PageInfo[1].b_isSetHeaderFooter == True		// フッタ２.
		)
		{
			if( rc_OEM.s_HeaderFooterInfo.e_ColorKind != TYPE_ColorKind_ERR )
			{
				if( e_ColorKind1 == TYPE_ColorKind_ERR )
				{
					e_ColorKind1 = rc_OEM.s_HeaderFooterInfo.e_ColorKind;
				}
				else if( e_ColorKind2 == TYPE_ColorKind_ERR && e_ColorKind1 != rc_OEM.s_HeaderFooterInfo.e_ColorKind )
				{
					e_ColorKind2 = rc_OEM.s_HeaderFooterInfo.e_ColorKind;
				}
				else if( e_ColorKind3 == TYPE_ColorKind_ERR && e_ColorKind1 != rc_OEM.s_HeaderFooterInfo.e_ColorKind && e_ColorKind2 != rc_OEM.s_HeaderFooterInfo.e_ColorKind )
				{
					e_ColorKind3 = rc_OEM.s_HeaderFooterInfo.e_ColorKind;
				}
			}
		}
		if( rc_OEM.s_TTIInfo.b_blExec == True )										// RTI/TTI FAX発信元情報.
		{
			// ブラックのみか？.
			if( e_ColorKind1 == TYPE_ColorKind_ERR )
			{
				e_ColorKind1 = TYPE_ColorKind_Black;
			}
			else if( e_ColorKind2 == TYPE_ColorKind_ERR && e_ColorKind1 != TYPE_ColorKind_Black )
			{
				e_ColorKind2 = TYPE_ColorKind_Black;
			}
			else if( e_ColorKind3 == TYPE_ColorKind_ERR && e_ColorKind1 != TYPE_ColorKind_Black && e_ColorKind2 != TYPE_ColorKind_Black )
			{
				e_ColorKind3 = TYPE_ColorKind_Black;
			}
		}

//////////////////////// PICとのIF用にパラメータ設定.
		if( e_ColorKind1 != TYPE_ColorKind_ERR && e_ColorKind2 != TYPE_ColorKind_ERR && e_ColorKind3 != TYPE_ColorKind_ERR )
		{
			pc_OPP->getpOPO()->ec_OutputColorMode = (TYPE_ColorMode)(( pc_OPP->getBlReliefColorPrint() == True ) ? TYPE_ColorModeBlack : TYPE_ColorModeFullColor);
		}
		else if( e_ColorKind1 != TYPE_ColorKind_ERR && e_ColorKind2 != TYPE_ColorKind_ERR )
		{
			pc_OPP->getpOPO()->ec_OutputColorMode = (TYPE_ColorMode)(( pc_OPP->getBlReliefColorPrint() == True ) ? TYPE_ColorModeBlack : TYPE_ColorModeFullColor);
		}
		else if( e_ColorKind1 == TYPE_ColorKind_Black )
		{
			pc_OPP->getpOPO()->ec_OutputColorMode = TYPE_ColorModeBlack;
		}
		else if( e_ColorKind1 != TYPE_ColorKind_ERR )
		{
			pc_OPP->getpOPO()->ec_OutputColorMode = (TYPE_ColorMode)(( pc_OPP->getBlReliefColorPrint() == True ) ? TYPE_ColorModeBlack : TYPE_ColorModeFullColor);
		}
		Bool b_isCombByAsic = ( theModeInfoLib.getOverlayType( rc_OEM ) == THRE_OverlayType_Synthesis ) ? True : False;
		if( b_isCombByAsic == True ){
			// Asicで合成するので、原稿画像は合成画像に合わせて指定しておく.
			pc_OPP->getpOPO()->ec_ColorMode = pc_OPP->getpOPO()->ec_OutputColorMode;
			pc_OPP->getpOPO()->uc_InputImageAttBit = 4;
		} else {
			// Asicでは合成しないので、原稿指定もなしとする.
			pc_OPP->getpOPO()->ec_ColorMode = TYPE_ColorModeNoColor;
			pc_OPP->getpOPO()->uc_InputImageAttBit = 0;
		}
//////////////////////// スレッドとのIF用にパラメータ設定.
		if( b_isCombByAsic == True ){
			// Asicで合成するので、RGB多値として出力.
			rc_OEM.III[0].ec_ImageColor = IMGE_ImageColor_RGB_ATT4;
			rc_OEM.III[0].ec_ImageTone = IMGE_ImageTone_8bit;
			rc_OEM.III[0].e_OriginalColorMode = pc_OPP->getpOPO()->ec_OutputColorMode;
			// 白紙のサイズは出力サイズと同じとしておく.
			rc_OEM.III[0].ct_ImageSize = rc_OEM.ct_OutputSize;
			rc_OEM.III[0].ct_EffectSize = rc_OEM.ct_OutputSize;
		} else {
			// Asicでは合成しないので、モノクロ２値として出力.
			rc_OEM.III[0].ec_ImageColor = IMGE_ImageColor_Density;
			rc_OEM.III[0].ec_ImageTone = IMGE_ImageTone_1bit;
			rc_OEM.III[0].e_OriginalColorMode = TYPE_ColorModeBlack;
		}
	}

	THRL_DbgColorKindLogCT( rc_OEM );
//End【Mosel対応 : K.Natsume : 2006.06.28 : No.62766】.
}

// ================================================================================================.
//! <!--機能--> メモリクリア(Emperor領域)実行要求用のセマフォ取得関数.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
//! 14/02/19 Sky 新規作成.
// ================================================================================================.
void THRF_memsetByDMA_semTake()
{
	SYSF_semTake( semID_memset, WAIT_FOREVER );
}

// ================================================================================================.
//! <!--機能--> メモリクリア(Emperor領域)実行要求用のセマフォ解放関数.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
//!  14/02/19 Sky 新規作成.
// ================================================================================================.
void THRF_memsetByDMA_semGive()
{
	SYSF_semGive( semID_memset );
}

// ================================================================================================.
//! <!--機能--> 【スレッド-PDL IF】メモリクリア(Emperor領域)実行.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param	uintptr_t	ul_ClearTopAddress	：クリア対象領域先頭アドレス.
//! \param	uint32_t	ul_ClearData		：クリア処理で書き込むデータ値.
//! \param	uint32_t	ul_ClearSize		：クリア処理サイズ.
//! \param	FUNCPTR	pfc_Callback		：クリア処理完了後に呼び出すコールバック関数ポインタ.
//! 									  ※pfc_Callback :【void foo(void) (戻り値、引数なし)】であること。.
//! 									  ※NULLであればコールバックは実行しない。.
//! <!--戻値-->.
//! \return	(none).
//! <!--詳細-->.
//! メモリクリア(Emperor領域)実行要求.
//! 　※Donau(Gaia) …ドライバ経由にて、BSPのIDMA(PCIブリッジ)を利用.
//! 　※SoC以降	…DMA32を利用.
//! <!--履歴-->.
//  09/10/21 T.Shima		クリア処理(新規DMA)対応：新規作成.
//  09/12/15 nakagawa		CQ:323103　THRL_Drvrif_MemoryClear()に渡す引数修正.
//  10/12/27	佐藤	Asic30対応.
// 12/02/28 Sky 機種共通化対応 THRC_ThreadifSoC.cppから移植.
//  14/02/19 Sky マルチRAS対応.
//  15/10/19 Sky		システム統合SoC(S800)対応.
//  17/04/07 Sky ZeusSMLKPF対応.
// ================================================================================================.
void THRF_memsetByDMA( uintptr_t ul_ClearTopAddress, uint32_t ul_ClearData, uint32_t ul_ClearSize,
					THRE_ImagePlace e_ImagePlace, void (*pfc_Callback)() )
{
	if( NULL == pfc_Callback ){
		//! 念のため、ログ表示.
		THRL_ThLogMsgErr( "THRF_memsetByDMA() > callback is NULL.\n" );
	}
	
	THRF_memsetByDMA_semTake();

	if(THRE_ImagePlaceMainMemory == e_ImagePlace){	
		//! メモリクリア(Emperor領域)実行要求.
		if( THRE_AsicKind_S800 == THRL_Drvrif_GetAsicKind( THRE_AsicUID_Asic_1 )) {
			THRL_Drvrif_MemoryClear( ul_ClearTopAddress, ul_ClearData, ul_ClearSize, e_ImagePlace, pfc_Callback );
		}
		else {
			THRL_Drvrif_Cmemapi_Memset( ul_ClearTopAddress, (int32_t)ul_ClearData, ul_ClearSize, pfc_Callback );
		}
	}else{
		if((0x80000000 & ul_ClearTopAddress) != 0 ) {
			// 内部情報をサブメモリ1に変更する.
			e_ImagePlace = THRE_ImagePlaceSubMemory1;
			// アドレスはドライバ側で考慮.
		}
		//! メモリクリア(Emperor領域)実行要求.
		THRL_Drvrif_MemoryClear( ul_ClearTopAddress, ul_ClearData, ul_ClearSize, e_ImagePlace, pfc_Callback );
	}
}

//Str【OP_BTS-30755 : 2021/04/07】.
static PDLS_CMD_ImageTransStartFromAPP tbl_CmdImageTransStartFromAPP[PDLE_TransAddrIndexMax*PDLE_BlockIndexMax];

// ================================================================================================
//! <!--機能--> Emperor->PDL 画像転送パラメータ情報設定処理 外部API.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param	PDLS_CMD_ImageTransStartFromAPP*	ps_CmdImageTransStartFromAPP
//! <!--戻値-->
//! \return なし.
//! <!--詳細-->
//! PDLからコールされ、画像転送パラメータ情報をBuffringする.
//! <!--履歴-->
//! 2021/04/07 OP_BTS-30755対応 新規作成.
// ================================================================================================
void THRF_setPDLS_CMD_ImageTransStartFromAPP( PDLS_CMD_ImageTransStartFromAPP* ps_CmdImageTransStartFromAPP )
{
	if( ( ps_CmdImageTransStartFromAPP != NULL )
	 && ( ps_CmdImageTransStartFromAPP->ss_BandNo >= 1 ) ) {
		short ss_Index;
		ss_Index = ( ps_CmdImageTransStartFromAPP->ss_BandNo - 1 ) % 8;
		
		tbl_CmdImageTransStartFromAPP[ss_Index].ul_JobID    = ps_CmdImageTransStartFromAPP->ul_JobID;
		tbl_CmdImageTransStartFromAPP[ss_Index].ss_PageNo   = ps_CmdImageTransStartFromAPP->ss_PageNo;
		tbl_CmdImageTransStartFromAPP[ss_Index].ss_BandNo   = ps_CmdImageTransStartFromAPP->ss_BandNo;
		tbl_CmdImageTransStartFromAPP[ss_Index].b_NoImage[0]= ps_CmdImageTransStartFromAPP->b_NoImage[0];
		tbl_CmdImageTransStartFromAPP[ss_Index].b_NoImage[1]= ps_CmdImageTransStartFromAPP->b_NoImage[1];
		for( uchar uc_ColorNo = 0; uc_ColorNo < PDLE_TransAddrIndexMax; uc_ColorNo++ ){
			for( uchar uc_BandNo1 = 0; uc_BandNo1 < PDLE_BlockIndexMax; uc_BandNo1++ ){
				tbl_CmdImageTransStartFromAPP[ss_Index].s_SrcPtr[uc_ColorNo][uc_BandNo1].pv_Ptr
					= ps_CmdImageTransStartFromAPP->s_SrcPtr[uc_ColorNo][uc_BandNo1].pv_Ptr;
				tbl_CmdImageTransStartFromAPP[ss_Index].s_SrcPtr[uc_ColorNo][uc_BandNo1].ul_Size
					= ps_CmdImageTransStartFromAPP->s_SrcPtr[uc_ColorNo][uc_BandNo1].ul_Size;
			}
		}
	}
}

// ================================================================================================
//! <!--機能--> Emperor->PDL 画像転送パラメータ情報取得処理.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param	uint32_t	ul_JobID
//! \param	short		ss_PageNo
//! \param	short		ss_BandNo
//! <!--戻値-->
//! \return PDLS_CMD_ImageTransStartFromAPP*	ps_CmdImageTransStartFromAPP.
//! <!--詳細-->
//! Thread処理からコールされ、Bufferingされた画像転送パラメータ情報を取得する.
//! <!--履歴-->
//! 2021/04/07 OP_BTS-30755対応 新規作成.
// ================================================================================================
PDLS_CMD_ImageTransStartFromAPP* THRF_getPDLS_CMD_ImageTransStartFromAPP( uint32_t ul_JobID, short ss_PageNo, short ss_BandNo )
{
	PDLS_CMD_ImageTransStartFromAPP *ps_CmdImageTransStartFromAPP = NULL;
	if( ss_BandNo >= 1 ) {
		short ss_Index;
		ss_Index = ( ss_BandNo - 1 ) % 8;
		if( ( tbl_CmdImageTransStartFromAPP[ss_Index].ul_JobID  == ul_JobID  )
		 && ( tbl_CmdImageTransStartFromAPP[ss_Index].ss_PageNo == ss_PageNo )
		 && ( tbl_CmdImageTransStartFromAPP[ss_Index].ss_BandNo == ss_BandNo ) ) {
			ps_CmdImageTransStartFromAPP = &tbl_CmdImageTransStartFromAPP[ss_Index];
		}
	}
	return ps_CmdImageTransStartFromAPP;
}
// ================================================================================================
//! <!--機能--> 画像転送パラメータ情報不一致検知時詳細コード登録処理.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param	uchar		uc_AppID
//! \param	uint32_t	ul_JobID
//! \param	short		ss_PageNo
//! \param	short		ss_BandNo
//! <!--戻値-->
//! \return なし.
//! <!--詳細-->
//! Thread処理からコールされ、AppToBuf転送パラメータ情報不一致検知時に詳細コードを登録するためにコール.
//! <!--履歴-->
//! 2021/04/07 OP_BTS-30755対応 新規作成（新詳細コード登録から別の処理に変更予定）.
// ================================================================================================
void setMismatchPDLS_CMD_ImageTransStartFromAPPDetailCode( uchar uc_AppID, uint32_t ul_JobID, short ss_PageNo, short ss_BandNo )
{
	Bool bl_ret = True;
	uint64_t ull_NewDetailCode = 0;

	// 各スレッド状態の詳細コードデータ整形.
	ull_NewDetailCode = THRD_NewDetail_ModuleId_Thread;							//  1- 2桁：0x20（モジュール番号：Thread）.
	ull_NewDetailCode |= ((uint64_t) 0x1FFF		<< 40) & 0x00ffff0000000000LL;	//  3- 6桁：シーケンス/イベント異常識別(0x1FFF:トラブルなし).
	ull_NewDetailCode |= ((uint64_t) 0x10		<< 32) & 0x000000ff00000000LL;	//  7- 8桁：10:画像転送パラメータ情報不一致(Page/Band).
	ull_NewDetailCode |= ((uint64_t) uc_AppID	<< 24) & 0x00000000ff000000LL;	//  9-10桁：AppID.
	ull_NewDetailCode |= ((uint64_t) ss_PageNo	<<  8) & 0x0000000000ffff00LL;	// 11-14桁：PageNo.
	ull_NewDetailCode |= ((uint64_t) ss_BandNo	<<  0) & 0x00000000000000ffLL;	// 15-16桁：BandNo.

#if 0
	bl_ret = APIC_setNewDetailCode( 0, ull_NewDetailCode );		// 詳細コード登録.
#endif
	if (False == bl_ret) {
		SYSF_logMsg( "## %s %d : APIC_setNewDetailCode NG\n", __FUNCTION__, __LINE__, 3,4,5,6 );
	}
	// 各スレッド状態の詳細コードデータ整形.
	ull_NewDetailCode = THRD_NewDetail_ModuleId_Thread;							//  1- 2桁：0x20（モジュール番号：Thread）.
	ull_NewDetailCode |= ((uint64_t) 0x1FFF		<< 40) & 0x00ffff0000000000LL;	//  3- 6桁：シーケンス/イベント異常識別(0x1FFF:トラブルなし).
	ull_NewDetailCode |= ((uint64_t) 0x11		<< 32) & 0x000000ff00000000LL;	//  7- 8桁：11:画像転送パラメータ情報不一致(JobID).
	ull_NewDetailCode |= ((uint64_t) ul_JobID	<<  0) & 0x00000000ffffffffLL;	//  9-16桁：JobID.

#if 0
	bl_ret = APIC_setNewDetailCode( 0, ull_NewDetailCode );		// 詳細コード登録.
#endif
	if (False == bl_ret) {
		SYSF_logMsg( "## %s %d : APIC_setNewDetailCode NG\n", __FUNCTION__, __LINE__, 3,4,5,6 );
	}
}
//End【OP_BTS-30755 : 2021/04/07】.

// ================================================================================================.
//! <!--機能--> 【スレッド-PIC IF】SDRAMアドレス取得.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \void
//! <!--戻値-->.
//! \return	uintptr_t スレッドからPICに通知するSDRAMのアドレス.
//! <!--詳細-->.
//! <!--履歴-->.
//!  13/03/25 Sky ZeusS CPS/PIC対応にて 新規作成.
//!  14/08/27 谷口 PMSありの場合のアドレス変更.
//!  2015/01/20 ADS森本  CQ.506800(Act_504831) AS-MSC_6 A3A4共通化(メモリスケーラビリティ拡張).
//!                      UT_CT_CIT不具合管理表No.56: A4機 PIC-PC間遅延メモリ動的対応.
//!  2015/04/28 ADS森本  CQ.510156(Act_507195) 【A3A4共通化】コンパイルスイッチ対応(機能/メモリスケーラビリティ拡張分).
//!  2015/10/06 Sky      システム統合SoC(S800)対応.
// ================================================================================================.
uintptr_t THRF_getPICSdramAddr()
{
    uint32_t ul_RetAddr = 0x00000000;
	Bool	 b_TypeOK = False;
	uint32_t ul_Size0 = 0;

#if 0
//#ifdef DEF_A3A4COMM // 2015/04/28 ADS森本 CQ.510156(Act_507195) 【A3A4共通化】コンパイルスイッチ対応(機能/メモリスケーラビリティ拡張分).
	//  2015/01/20 ADS森本  CQ.506800(Act_504831) AS-MSC_6 A3A4共通化(メモリスケーラビリティ拡張).
	// スレッドの使用領域取得しPICにSDRAMアドレスを通知 .
	// thr_comItfc002.cのus_DIMMSizeTblテーブルを参照している.
	// スレッドの使用領域サイズの取得.
	ul_Size0 = THRF_GetDIMMSizeBySPD( THRE_SPD_DEVICE_0, b_TypeOK );
	ul_RetAddr = ul_Size0 * ((uint32_t)1024*1024);
#else // #ifdef DEF_A3A4COMM.

	if( True == THRF_IsA800IntegrationExist() ) {
		// スレッドの使用領域サイズ(A800のSDRAM1)の取得.
		ul_Size0 = THRF_GetDIMMSizeBySPD( THRE_SPD_DEVICE_3, b_TypeOK );
		ul_RetAddr = ul_Size0 * ((uint32_t)1024*1024);
	}
	else {
		// S800ではPC間遅延バッファがメインメモリ側にあるので、この関数は呼ばれない.
		THRL_ThLogMsgErr( "THRF_getPICSdramAddr() > This Function not used.\n" );
	}
#endif // #ifdef DEF_A3A4COMM.

    return ul_RetAddr;
}


// ================================================================================================.			
//! <!--機能--> 【スレッド-PIC IF】SDRAMアドレス取得.			
//! <!--------------------------------------------------------------------------------------------->.			
//! <!--引数-->.			
//! \uchar uc_AsicUID ：ASIC 種別.			
//! <!--戻値-->.			
//! \return	uintptr_t スレッドからPICに通知するSDRAMのアドレス.		
//! <!--詳細-->.			
//! <!--履歴-->.			
//!  13/03/25 Sky ZeusS CPS/PIC対応にて 新規作成.			
//!  14/08/27 谷口 PMSありの場合のアドレス変更.			
//!  2015/01/20 ADS森本  CQ.506800(Act_504831) AS-MSC_6 A3A4共通化(メモリスケーラビリティ拡張).			
//!                      UT_CT_CIT不具合管理表No.56: A4機 PIC-PC間遅延メモリ動的対応.			
//!  2015/04/28 ADS森本  CQ.510156(Act_507195) 【A3A4共通化】コンパイルスイッチ対応(機能/メモリスケーラビリティ拡張分).			
//!  2015/10/06 Sky      システム統合SoC(S800)対応.			
// ================================================================================================.			
#define THRL_ThrLogMsgDebug printf
uintptr_t THRF_getPICSdramAddr(unsigned char uc_asicNo)
{
    uint32_t ul_RetAddr = 0x00000000;
#if defined( DEF_A3A4COMM ) || defined( DEF_PP_Cus )
	Bool	 b_TypeOK = False;
	uint32_t ul_Size0 = 0;
#endif // #if defined( DEF_A3A4COMM ) || defined( DEF_PP_Cus ) .
			
#ifdef DEF_A3A4COMM // 2015/04/28 ADS森本 CQ.510156(Act_507195) 【A3A4共通化】コンパイルスイッチ対応(機能/メモリスケーラビリティ拡張分).			
	if( True == THRF_IsA800IntegrationExist() ) {
		// スレッドの使用領域サイズ(A800のSDRAM1)の取得.
		ul_Size0 = THRF_GetDIMMSizeBySPD( THRE_SPD_DEVICE_3, b_TypeOK );
		ul_RetAddr = ul_Size0 * ((uint32_t)1024*1024);
	}
	else {
		// S800ではPC間遅延バッファがメインメモリ側にあるので、この関数は呼ばれない.
		THRL_ThLogMsgErr( "THRF_getPICSdramAddr() > This Function not used.\n" );
	}
#else // #ifdef DEF_A3A4COMM.

#ifdef DEF_PP_Cus
			
	switch( uc_asicNo){
	case THRE_ASIC_1:
		ul_RetAddr =THRF_getPICSdramAddr();
	break;
			
	case THRE_ASIC_3: ///A800　2枚目（Asic4枚目）.
	if( True == THRF_IsA800IntegrationExist(THRE_ASIC_3) ) {
		// スレッドの使用領域サイズ(A800のSDRAM0)の取得.
		ul_Size0 = THRF_GetDIMMSizeBySPD( THRE_SPD_DEVICE_6, b_TypeOK );
		ul_RetAddr = ul_Size0 * ((uint32_t)1024*1024);
			
		//二枚目A800指定：未接続状態は、特に処理無し。.
		//★★　接続時はLog表示する。　★★.
		THRL_ThrLogMsgDebug("Thr Debug: [getPICSdramAddr] asicNo:%d ul_RetAddr:0x%lx(%d)\n",uc_asicNo,ul_RetAddr,ul_RetAddr);
	}
	break;
			
	///A800 3枚目、4枚目はPC間遅延メモリ無し.
	}

#endif // DEF_PP_Cus.

#endif // #ifdef DEF_A3A4COMM.
			
    return ul_RetAddr;
}


/*================================================================================================
［機能］	Webブラウザが有効かどうかを判定する(CommonAPIに提供).
--------------------------------------------------------------------------------------------------
［引数］	void.
［戻値］	Bool	True:WebROMあり.
					False:WebROMなし.
［履歴］	2014/01/08 忠政 新規作成.
================================================================================================*/
uint32_t ul_ThWebRomFlg = 0;
Bool THRF_isWebRom(){
#if (defined TORNADO) && (!defined FUM_LINUX)		// 実機で有効.
	Bool b_ans = False;
	if( access( THRD_WEBROMFILE_PATH , F_OK )==0 ){
		//WebROMファイル有り.
		if( ul_ThWebRomFlg == 1 ){ printf("WebRomFile Result True\n"); }
		b_ans = True;
	}else{
		//WebROMファイル無し.
		if( ul_ThWebRomFlg == 1 ){ printf("WebRomFile Result False\n"); }
		b_ans = False;
	}
	return b_ans;
#else
	return True;//Simの場合は常にWebROMありとする.
#endif
}
// ================================================================================================
//! <!--機能--> キャッシュをフラッシュする.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param	なし.
//! <!--戻値-->
//! \return なし.
//! <!--詳細-->
//! <!--履歴-->
//  2015/02/03 simogaki	CQ509588 OEMバイナリDL後、コンバージョンファイルが表示されない対応.
//                      SubSetにあるキャッシュフラッシュ関数を本体FWにも移植.
// ================================================================================================
void THRF_DoSync(void)
{
	// 通例として3回行う.
	sync();
	sync();
	sync();
}
// ================================================================================================.
//! <!--機能--> 書き込み許可を設定する.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param	なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! 	JOB動作時にIPROMデータの値でAsicレジスタの書き換え行えるようにする(通常動作に戻す).
//! <!--履歴-->.
//  2015/08/19 simogaki	ZeusSZX0 制御構成変更No20 画像設計機能追加対応に伴い外部IFを新規追加.
// ================================================================================================	.
void THRG_setEnableToWrite(void){

	THRS_IPROMValidChgArg s_ArgValid;

	s_ArgValid.bl_Valid = False;	//許可設定.

	THRL_SetIPROMChangeSeqLog( THRD_IPROMChangeLogReqKindFuncCallValidChange, __FUNCTION__, 0, 0, 0, 0, 0, 0 );

	THRL_Drvrif_ValidChg( s_ArgValid );
	
	THRL_SetIPROMChangeSeqLog( THRD_IPROMChangeLogReqKindReturnVoid, __FUNCTION__, 0, 0, 0, 0, 0, 0 );
}
// ================================================================================================.
//! <!--機能--> 書き込み禁止を設定する.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param	なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! 	JOB動作時にIPROMデータの値でAsicレジスタの書き換えしない(チューニングツールで設定した値で継続動作するようにする).
//! <!--履歴-->.
//  2015/08/19 simogaki	ZeusSZX0 制御構成変更No20 画像設計機能追加対応に伴い外部IFを新規追加.
// ================================================================================================	.
void THRG_setDisableToWrite(void){
	
	THRS_IPROMValidChgArg s_ArgValid;

	s_ArgValid.bl_Valid = True;	//禁止設定.

	THRL_SetIPROMChangeSeqLog( THRD_IPROMChangeLogReqKindFuncCallValidChange, __FUNCTION__, 0, 0, 0, 0, 0, 0 );

	THRL_Drvrif_ValidChg( s_ArgValid );

	THRL_SetIPROMChangeSeqLog( THRD_IPROMChangeLogReqKindReturnVoid, __FUNCTION__, 0, 0, 0, 0, 0, 0 );
}
// ================================================================================================.
//! <!--機能--> IPROMデータ レジスタ設定.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param 	unsigned char	uc_asicNo	:Asic番号.
//! \param	unsigned short	us_ipNo		:回路番号.
//! \param	unsigned char*	puc_buffer	:Writeデータの先頭アドレス.
//! \param	uint32_t		ul_length	:Writeデータのサイズ.
//! \param	uint32_t		ul_offset	:先頭アドレスからのオフセット.
//! <!--戻値-->.
//! \return  :成否(正常:0、失敗:-1).
//! <!--詳細-->.
//! 	指定されたデータでAsicレジスタへ書き込みを行う.
//! 	対象Asic、対象レジスタアドレスはAsic番号、回路番号より決定する.
//! <!--履歴-->.
//  2015/08/19 simogaki	ZeusSZX0 制御構成変更No20 画像設計機能追加対応に伴い外部IFを新規追加.
// ================================================================================================	.
short THRG_writeReg( unsigned char uc_asicNo, unsigned short us_ipNo, unsigned char* puc_buffer, uint32_t ul_length, uint32_t ul_offset ){

	short ss_LocalFuncRet = -1;		//内部関数戻り.
	short ss_Ret = 0;				//IF関数戻り.

	THRL_SetIPROMChangeSeqLog( THRD_IPROMChangeLogReqKindFuncCallRegChange, __FUNCTION__, ( uintptr_t )uc_asicNo, ( uintptr_t )us_ipNo, ( uintptr_t )puc_buffer, ul_length, ul_offset, 0 );
	if ( NULL != puc_buffer ) {

		// 内部関数コール
		ss_LocalFuncRet = THRL_writeReg( uc_asicNo, us_ipNo, puc_buffer, ul_length, ul_offset );
		if( 0 != ss_LocalFuncRet) {
			// 失敗に丸める.
			ss_Ret = -1;
		}
	}

	THRL_SetIPROMChangeSeqLog( THRD_IPROMChangeLogReqKindReturnValue, __FUNCTION__, ( uintptr_t )ss_Ret, 0, 0, 0, 0, 0 );

	return ss_Ret;
}
// ================================================================================================.
//! <!--機能--> IPROMデータ レジスタ読み込み.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param 	unsigned char	uc_asicNo	:Asic番号.
//! \param	unsigned short	us_ipNo		:回路番号.
//! \param	unsigned char*	puc_buffer	:Readデータを格納するバッファの先頭アドレス.
//! \param	uint32_t*		pul_length	:Readしたデータのサイズ.
//! \param	uint32_t		ul_offset	:先頭アドレスからのオフセット.
//! <!--戻値-->.
//! \return  :成否(正常:0、失敗:-1).
//! <!--詳細-->.
//! 	Asicレジスタに書き込まれているレジスタ値を読み込む.
//! 	読み込んだデータサイズはReadしたデータのサイズに設定する.
//! 	対象Asic、対象レジスタアドレスはAsic番号、回路番号より決定する.
//! <!--履歴-->.
//  2015/08/19 simogaki	ZeusSZX0 制御構成変更No20 画像設計機能追加対応に伴い外部IFを新規追加.
// ================================================================================================	.
short THRG_readReg( unsigned char uc_asicNo, unsigned short us_ipNo, unsigned char* puc_buffer, uint32_t* pul_length, uint32_t ul_offset ){

	short ss_LocalFuncRet = -1;		//内部関数戻り.
	short ss_Ret = 0;		//IF関数戻り.
	THRL_SetIPROMChangeSeqLog( THRD_IPROMChangeLogReqKindFuncCallRegChange, __FUNCTION__, ( uintptr_t )uc_asicNo, ( uintptr_t )us_ipNo, ( uintptr_t )puc_buffer, ( uintptr_t )pul_length, ul_offset, 0 );

	if (( NULL != puc_buffer ) && ( NULL != pul_length)){
		// 内部関数コール
		ss_LocalFuncRet = THRL_readReg( uc_asicNo, us_ipNo, puc_buffer, pul_length, ul_offset );
		if( 0 != ss_LocalFuncRet ) {
			// 失敗に丸める.
			ss_Ret = -1;
		}
	}
	THRL_SetIPROMChangeSeqLog( THRD_IPROMChangeLogReqKindReturnValue, __FUNCTION__, ( uintptr_t )ss_Ret, 0, 0, 0, 0, 0 );

	return ss_Ret;
}
// ================================================================================================.
//! <!--機能--> IPROMデータ レジスタ設定(内部).
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param 	unsigned char	uc_AsicNo	:Asic番号.
//! \param	unsigned short	us_IpNo		:回路番号.
//! \param	unsigned char*	puc_Buffer	:Writeデータの先頭アドレス.
//! \param	uint32_t		ul_Length	:Writeデータのサイズ.
//! \param	uint32_t		ul_Offset	:先頭アドレスからのオフセット.
//! <!--戻値-->.
//! \return  :成否(正常:0、失敗:-1～-4).
//! <!--詳細-->.
//! 	指定されたデータでAsicレジスタへ書き込みを行う.
//! 	対象Asic、対象レジスタアドレスはAsic番号、回路番号より決定する.
//! <!--履歴-->.
//  2015/08/19 simogaki	ZeusSZX0 制御構成変更No20 画像設計機能追加対応に伴い外部IFを新規追加.
//  2015/10/06 Sky		システム統合SoC(S800)対応.
// ================================================================================================	.
static short THRL_writeReg( uchar uc_AsicNo, ushort us_IpNo, uchar* puc_Buffer, uint32_t ul_Length, uint32_t ul_Offset ){

	char sc_AsicUid = THRE_AsicUID_Asic_1;		// AsicID.
	char sc_AsicKind = THRE_AsicKind_S800 ;	// Asic種別.
	short ss_Ret = THRD_IPROMLocalFuncIfRetrunSuccee;							// 関数戻り.
	uint32_t ul_Index = 0;							// 接続チェック、サイズ計算テーブルのインデックス
	uintptr_t ul_StartAddr = 0;					// レジスタ書き込み開始アドレス.
	THRS_IPROMRegArg s_SetRegAvg;			// ドライバーへの通知要求メッセージ.

	// 引数チェック.
	if ( NULL == puc_Buffer ) {
		ss_Ret = THRD_IPROMLocalFuncIfRetrunParameterErr;
	} else {
		// 開始アドレスの検索.
		for ( ul_Index = 0; ul_Index < THRD_IPROMReadRegSizeTblNumMax; ul_Index++ ){
			if ( gs_IPROMReadRegDataSizeTbl[ul_Index].us_IpNo == us_IpNo ){
				break;
			}
		}
		if ( ul_Index >= THRD_IPROMReadRegSizeTblNumMax ){
			// インデックスが上限値は意図しないパラメータなのでパラメータエラー.
			ss_Ret = THRD_IPROMLocalFuncIfRetrunParameterErr;
		} else {
			ul_StartAddr = gs_IPROMReadRegDataSizeTbl[ul_Index].ul_Address;
		}

	}

	// 対象Asicの接続判定.
	if ( 0 == ss_Ret ) {
		for ( ul_Index = 0; ul_Index < THRD_ConnectTblNumMax ; ul_Index++ ){
			if ( gs_ConnectChk[ul_Index].uc_AsicNo == uc_AsicNo ) {
				break;
			}
		}
		if ( ul_Index >= THRD_ConnectTblNumMax ) {
			// インデックスが上限値は意図しないパラメータなのでパラメータエラー.
			ss_Ret = THRD_IPROMLocalFuncIfRetrunParameterErr;
		} else if ( gs_ConnectChk[ul_Index].sc_AsicKind != THRL_Drvrif_GetAsicKind(gs_ConnectChk[ul_Index].sc_AsicUID )){
			//未接続の場合は未接続エラーを設定.
			ss_Ret = THRD_IPROMLocalFuncIfRetrunUnconnectedErr;
		} else {
			// 接続時はAsicID、Asic種別を保持(ドライバー側でベースアドレスを取得するため).
			sc_AsicUid = gs_ConnectChk[ul_Index].sc_AsicUID;
			sc_AsicKind = gs_ConnectChk[ul_Index].sc_AsicKind;
		}
	}

	// 引数チェック、接続チェックで異常がなく書き込みサイズが0でない場合のみドライバーへ要求.
	if ( ( THRD_IPROMLocalFuncIfRetrunSuccee == ss_Ret ) && ( 0 != ul_Length) ){

		memset( &s_SetRegAvg, 0, sizeof(s_SetRegAvg) );
		s_SetRegAvg.sc_AsicUid = sc_AsicUid;
		s_SetRegAvg.sc_AsicKind = sc_AsicKind;
		s_SetRegAvg.ul_StartAdress = ul_StartAddr;
		s_SetRegAvg.ul_DataSize = ul_Length;
		s_SetRegAvg.ul_DataAddress = ( uintptr_t )puc_Buffer;
		THRL_SetIPROMChangeSeqLog( THRD_IPROMChangeLogReqKindParaWriteReg, __FUNCTION__, ul_StartAddr, 0, 0, 0, 0, 0 );

		// ドライバーへ通知.
		ss_Ret = THRL_Drvrif_IPROMwriteReg( s_SetRegAvg );

		if( THRD_IPROMLocalFuncIfRetrunSuccee == ss_Ret) {
			// 成功時はデータログを保存.
			THRL_SetIPROMChangeDataLog( 'W',( char* )puc_Buffer, ul_StartAddr, ul_Length );
		}
	}
	THRL_SetIPROMChangeSeqLog( THRD_IPROMChangeLogReqKindReturnValue, __FUNCTION__, ( uintptr_t )ss_Ret, 0, 0, 0, 0, 0 );

	return ss_Ret;
}
// ================================================================================================.
//! <!--機能--> IPROMデータ レジスタ読み込み(内部).
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param 	unsigned char	uc_AsicNo	:Asic番号.
//! \param	unsigned short	us_IpNo		:回路番号.
//! \param	unsigned char*	puc_Buffer	:Readデータを格納するバッファの先頭アドレス.
//! \param	uint32_t*		pul_Length	:Readしたデータのサイズ.
//! \param	uint32_t		ul_Offset	:先頭アドレスからのオフセット.
//! <!--戻値-->.
//! \return  :成否(正常:0、失敗:-1～-4).
//! <!--詳細-->.
//! 	Asicレジスタに書き込まれているレジスタ値を読み込む.
//! 	読み込んだデータサイズはReadしたデータのサイズに設定する.
//! 	対象Asic、対象レジスタアドレスはAsic番号、回路番号より決定する.
//! <!--履歴-->.
//  2015/08/19 simogaki	ZeusSZX0 制御構成変更No20 画像設計機能追加対応に伴い外部IFを新規追加.
//  2015/10/06 Sky		システム統合SoC(S800)対応.
// ================================================================================================	.
static short THRL_readReg( uchar uc_AsicNo, ushort us_IpNo, uchar* puc_Buffer, uint32_t* pul_Length, uint32_t ul_Offset ){

	char sc_AsicUid = THRE_AsicUID_Asic_1;		// AsicID.
	char sc_AsicKind = THRE_AsicKind_S800 ;	// Asic種別.
	short ss_Ret = THRD_IPROMLocalFuncIfRetrunSuccee;							// 関数戻り.
	uintptr_t ul_StartAddr = 0;					// レジスタ書き込み開始アドレス.
	uint32_t ul_Index = 0;							// 接続チェック、サイズ計算テーブルのインデックス.
	THRS_IPROMRegArg s_GetRegAvg;			// ドライバーへの通知要求メッセージ.

	// 引数チェック.
	if (( NULL == puc_Buffer ) || (NULL == pul_Length )){
		ss_Ret = THRD_IPROMLocalFuncIfRetrunParameterErr;
	} else {
		// 開始アドレスの検索.
		for ( ul_Index = 0; ul_Index < THRD_IPROMReadRegSizeTblNumMax; ul_Index++ ){
			if ( gs_IPROMReadRegDataSizeTbl[ul_Index].us_IpNo == us_IpNo ){
				break;
			}
		}
		if ( ul_Index >= THRD_IPROMReadRegSizeTblNumMax ){
			// インデックスが上限値は意図しないパラメータなのでパラメータエラー.
			ss_Ret = THRD_IPROMLocalFuncIfRetrunParameterErr;
		} else {
			ul_StartAddr = gs_IPROMReadRegDataSizeTbl[ul_Index].ul_Address;
			*pul_Length = gs_IPROMReadRegDataSizeTbl[ul_Index].ul_Size;
		}
	}

	// 対象Asicの接続判定.
	if ( THRD_IPROMLocalFuncIfRetrunSuccee == ss_Ret ) {
		for ( ul_Index = 0; ul_Index < THRD_ConnectTblNumMax; ul_Index++ ){
			if ( gs_ConnectChk[ul_Index].uc_AsicNo == uc_AsicNo ) {
				break;
			}
		}
		if ( ul_Index >= THRD_ConnectTblNumMax ) {
			// インデックスが上限値は意図しないパラメータなのでパラメータエラー.
			ss_Ret = THRD_IPROMLocalFuncIfRetrunParameterErr;
		} else if ( gs_ConnectChk[ul_Index].sc_AsicKind != THRL_Drvrif_GetAsicKind( gs_ConnectChk[ul_Index].sc_AsicUID )){
			//未接続の場合は未接続エラーを設定.
			ss_Ret = THRD_IPROMLocalFuncIfRetrunUnconnectedErr;
		} else {
			// 接続時はAsicID、Asic種別を保持(ドライバー側でベースアドレスを取得するため).
			sc_AsicUid = gs_ConnectChk[ul_Index].sc_AsicUID;
			sc_AsicKind = gs_ConnectChk[ul_Index].sc_AsicKind;
		}
	}

	// 引数チェック、接続チェックで異常がなく読み込みサイズが0でない場合のみドライバーへ要求.
	if ( ( THRD_IPROMLocalFuncIfRetrunSuccee == ss_Ret ) && ( 0 != *pul_Length ) ){
		
		memset( &s_GetRegAvg, 0, sizeof(s_GetRegAvg) );
		s_GetRegAvg.sc_AsicUid = sc_AsicUid;
		s_GetRegAvg.sc_AsicKind = sc_AsicKind;
		s_GetRegAvg.ul_StartAdress = ul_StartAddr;
		s_GetRegAvg.ul_DataSize = ( uint32_t )*pul_Length;
		s_GetRegAvg.ul_DataAddress = ( uintptr_t )puc_Buffer;
		THRL_SetIPROMChangeSeqLog( THRD_IPROMChangeLogReqKindParaReadReg, __FUNCTION__, ul_StartAddr, ( uintptr_t )*pul_Length, 0, 0, 0, 0 );

		// ドライバーへ通知.
		ss_Ret = THRL_Drvrif_IPROMReadReg( &s_GetRegAvg );

		if( THRD_IPROMLocalFuncIfRetrunSuccee == ss_Ret) {
			// 成功時はデータログを保存.
			THRL_SetIPROMChangeDataLog( 'R', ( char* )puc_Buffer, ul_StartAddr, *pul_Length );
		}
	}
	THRL_SetIPROMChangeSeqLog( THRD_IPROMChangeLogReqKindReturnValue, __FUNCTION__, ( uintptr_t )ss_Ret, 0, 0, 0, 0, 0 );

	return ss_Ret;
}
// ================================================================================================.
//! <!--機能--> IPROMデータ メモリ設定.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param 	unsigned char	uc_asicNo	:Asic番号.
//! \param	unsigned short	us_ipNo		:回路番号.
//! \param	unsigned short	us_funcNo	:機能番号.
//! \param	unsigned char*	puc_buffer	:Writeデータの先頭アドレス.
//! \param	uint32_t		ul_length	:Writeデータのサイズ.
//! \param	uint32_t		ul_offset	:先頭アドレスからのオフセット.
//! <!--戻値-->.
//! \return  :成否(正常:0、失敗:-1).
//! <!--詳細-->.
//! 	IPROMのデータを書き換える.
//! 	対象IPROMデータはAsic番号、回路番号、機能番号より決定する.
//!		現状は未対応の為、常に失敗を返す.
//! <!--履歴-->.
//  2015/08/19 simogaki	ZeusSZX0 制御構成変更No20 画像設計機能追加対応に伴い外部IFを新規追加.
// ================================================================================================	.
short THRG_writeMem( unsigned char uc_asicNo, unsigned short us_ipNo, unsigned short us_funcNo, unsigned char* puc_buffer, uint32_t ul_length, uint32_t ul_offset ){
	//現状は未対応の為、常に失敗を返す.
	return -1;
}
// ================================================================================================.
//! <!--機能-->  IPROMデータ メモリ読み込み.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param 	unsigned char	uc_asicNo	:Asic番号.
//! \param	unsigned short	us_ipNo		:回路番号.
//! \param	unsigned char*	puc_buffer	:Readデータを格納するバッファの先頭アドレス.
//! \param	uint32_t*		pul_length	:Readしたデータのサイズ.
//! \param	uint32_t		ul_offset	:先頭アドレスからのオフセット.
//! <!--戻値-->.
//! \return  :成否(正常:0、失敗:-1).
//! <!--詳細-->.
//! 	IPROMのデータを読み込む.
//! 	読み込んだデータサイズはReadしたデータのサイズに設定する.
//! 	対象IPROMデータははAsic番号、回路番号より決定する.
//! <!--履歴-->.
//  2015/08/19 simogaki	ZeusSZX0 制御構成変更No20 画像設計機能追加対応に伴い外部IFを新規追加.
// ================================================================================================	.
short THRG_readMem( unsigned char uc_asicNo, unsigned short us_ipNo, unsigned char* puc_buffer, uint32_t* pul_length, uint32_t ul_offset){
	//現状は未対応の為、常に失敗を返す.
	return -1;
}
//! ================================================================================================.
//! <!--機能--> チューニングツールレジスタ読み出しサイズ変更.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param uchar uc_AsicKind	:Asic種別.
//! \param ushort us_IpNo		:回路番号.
//! \param uint32_t ul_chg_size	:変更サイズ.
//! <!--戻値-->.
//! \return void.
//! <!--詳細-->.
//!    チューニングツールでのレジスタ読み出しサイズを指定サイズに変更する. \n.
//! <!--履歴-->.
//  2015/08/19 simogaki	ZeusSZX0 制御構成変更No20 画像設計機能追加対応に伴いログを新規追加.
// ================================================================================================.
void THRG_IPROMChange_ReadRegSizeChg( uchar uc_AsicKind, ushort us_IpNo, uint32_t ul_ChgSize ){

	uint32_t ul_Cnt = 0;			// カウンタ値.
	uint32_t ul_BeforSize = 0;	// 変更前サイズ.
	
	THRS_IPROMRegSizeTbl*	ps_ReadSizeTbl = gs_IPROMReadRegDataSizeTbl;
	uint32_t ul_TblSize = THRD_IPROMReadRegSizeTblNumMax;

	// テーブル内を検索.
	for ( ul_Cnt = 0; ul_Cnt < ul_TblSize; ul_Cnt++ ) {
		if ( us_IpNo == ps_ReadSizeTbl[ul_Cnt].us_IpNo ) {
			ul_BeforSize = ps_ReadSizeTbl[ul_Cnt].ul_Size;
	     	ps_ReadSizeTbl[ul_Cnt].ul_Size = ul_ChgSize;
			break;
		}
	}
	
	if ( ul_Cnt >= ul_TblSize) {
		printf("%s Parameter Err AsicKind:%d ipNo:%d\n", __FUNCTION__, uc_AsicKind, us_IpNo);
	} else {
		printf("%s Asic:%d ipNo:%d BeforSize:%"PRId32" AfterSize:%"PRId32"\n", __FUNCTION__, uc_AsicKind, us_IpNo, ul_BeforSize, ps_ReadSizeTbl[ul_Cnt].ul_Size );
	}
}

// ================================================================================================
//! <!--機能--> シェルコマンドの実行.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param psc_Command		シェルコマンド.
//! <!--戻値-->
//! \return int 0:成功、0以外：失敗.
//! <!--詳細-->
//! シェルコマンド実行後の子プロセスの終了状態をチェック.
//! <!--履歴-->
//! 2013/11/08 yamada	新規作成.
// ================================================================================================
int THRF_System(const char* psc_Command)
{
	int si_Ret = 0;

	// 引数チェック.
	if (psc_Command == NULL) {
		SYSF_logMsg("[THRF_System] invalid parameter\n");
		return -1;
	}

	si_Ret = system(psc_Command);
	// 子プロセスが正常に終了した場合に真を返す.
	if (!WIFEXITED(si_Ret)) {
		SYSF_logMsg("[THRF_System] proccess exit error.\n");
		return -2;
	}

	// コマンドの終了コードを取得する.
	return WEXITSTATUS(si_Ret);
}

//↓ 2016/08/31 Sky合屋 [IT5_4.0]制御構成変更No.48.
#if defined( DEF_CCD_CHG_ENABLE)
static char sc_ResCCDGaparaSymLinkTbl[THRD_CCDGaparaModuleNum][THRD_CCDGaparaFileLength] = {
	THRD_PIC_CCDGaparaSymLink,
	THRD_THR_CCDGaparaSymLink,
	THRD_IR_CCDGaparaSymLink,
};

static char sc_CCDGaparaFileTbl[THRD_CCDGaparaModuleNum][THRD_CCDGaparaFileNum][THRD_CCDGaparaFileLength] = {
	{THRD_PIC_CCDGaparaFileName0, THRD_PIC_CCDGaparaFileName1},
	{THRD_THR_CCDGaparaFileName0, THRD_THR_CCDGaparaFileName1},
	{THRD_IR_CCDGaparaFileName0, THRD_IR_CCDGaparaFileName1},
};
#if defined(DEF_ZEUSSBKX0_CUS)	// ZeusSBK X/0 …  2016.10.25 【IT5_4.0】制御構成変更No48「CCD切り替え対応」. Beam切り替え＆CCD切り替え.
	#define THRD_CCDGaparaModule_PIC	0	// PIC画パラ(データIndex).
	#define THRD_CCDGaparaModule_THR	1	// Thread画パラ(データIndex).
	#define THRD_CCDGaparaModule_IR		2	// IR画パラ(データIndex).
	#define THRD_CCDGaparaBeam_1		0	// 1Beam(データIndex).
	#define THRD_CCDGaparaBeam_2		1	// 2Beam(データIndex).
	// PIC画パラファイルリスト[新旧][1/2Beam].
	static char sc_CCDGaparaFileTbl_PIC[THRD_CCDGaparaFileNum][THRD_CCDGaparaBeamNum][THRD_CCDGaparaFileLength] = {
		{THRD_1Beam_PIC_CCDGaparaFileName0, THRD_2Beam_PIC_CCDGaparaFileName0},	// 旧CCD.
		{THRD_1Beam_PIC_CCDGaparaFileName1, THRD_2Beam_PIC_CCDGaparaFileName1},	// 新CCD.
	};
	// Thread画パラファイルリスト[新旧][1/2Beam].
	static char sc_CCDGaparaFileTbl_THR[THRD_CCDGaparaFileNum][THRD_CCDGaparaBeamNum][THRD_CCDGaparaFileLength] = {
		{THRD_1Beam_THR_CCDGaparaFileName0, THRD_2Beam_THR_CCDGaparaFileName0},	// 旧CCD.
		{THRD_1Beam_THR_CCDGaparaFileName1, THRD_2Beam_THR_CCDGaparaFileName1},	// 新CCD.
	};

// 取得したPH構成情報.
#define THRD_PhTypeUnknown	-1	// 不明.
#define THRD_PhType1Beam	1	// 1Beam.
#define THRD_PhType2Beam	0	// 2Beam.

// ================================================================================================
//! <!--機能--> エンジンからのPH構成情報取得.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//!  char *psc_PhType PH構成情報(1Beam:THRD_PhType1Beam、2Beam:THRD_PhType2Beam、不明:THRD_PhTypeUnknown)..
//! <!--戻値-->
//! Bool True:成功、False：失敗.
//! <!--詳細-->
//! エンジンからのPH構成情報(Beam種別)を取得する.
//! <!--履歴-->
//! 2016/10/25 Sky井柳	【IT5_4.0】制御構成変更No48「CCD切り替え対応」新規作成.
// ================================================================================================
static Bool THRL_getPHBeamInfoEngine( char *psc_PhType )
{
	if( psc_PhType == NULL ){	// パラメタエラー.
		return False;
	}
	char sc_PhType = THRD_PhTypeUnknown;	// 不明.

	// エンジンからのPH構成情報を取得する.
	TYPE_PHType e_EngPhType = comENG.getRx().getCcSystem1().getPhType();
	switch ( e_EngPhType ) {
	case TYPE_PHType1Beam:
		SYSF_logMsg( "[THRL_getPHBeamInfoEngine] ENG PH 1Beam.\n" );
		sc_PhType = THRD_PhType1Beam;
		break;
	case TYPE_PHType2Beam:
		SYSF_logMsg( "[THRL_getPHBeamInfoEngine] ENG PH 2Beam.\n" );
		sc_PhType = THRD_PhType2Beam;
		break;
	default:
		SYSF_logMsg( "[THRL_getPHBeamInfoEngine] ENG PH not 1Beam/2Beam %d.\n", e_EngPhType );
		break;
	}
	
	// 戻り値設定.
	Bool bl_Ret = False;
	if( sc_PhType != THRD_PhTypeUnknown ){
		bl_Ret = True;	// 取得OK.
	}
	*psc_PhType = sc_PhType;
	
	return bl_Ret;
}

// ================================================================================================
//! <!--機能--> SPI-Flash(Eeprom領域)に保存されている値からのPH構成情報取得.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//!  char *psc_PhType PH構成情報(1Beam:THRD_PhType1Beam、2Beam:THRD_PhType2Beam、不明:THRD_PhTypeUnknown)..
//! <!--戻値-->
//! Bool True:成功、False：失敗.
//! <!--詳細-->
//! SPI-Flash(Eeprom領域)に保存されている値からのPH構成情報(Beam種別)を取得する.
//! <!--履歴-->
//! 2016/10/25 Sky井柳	【IT5_4.0】制御構成変更No48「CCD切り替え対応」新規作成.
// ================================================================================================
static Bool THRL_getPHBeamInfoSpiFlash( char *psc_PhType )
{
	if( psc_PhType == NULL ){	// パラメタエラー.
		return False;
	}
	char sc_PhType = THRD_PhTypeUnknown;	// 不明.

	// SPI-Flashから設定された情報を取得.
	Bool bl_Ret = THRG_getEepromIPROMInfo( &sc_PhType );
	if( bl_Ret == False ){
		SYSF_logMsg( "[THRL_getPHBeamInfoSpiFlash] THRG_getEepromIPROMInfo Error. \n");
		return False;
	}

	switch ( sc_PhType ) {
	case TYPE_PHType1Beam:
		SYSF_logMsg( "[THRL_getPHBeamInfoSpiFlash] SPI-Flash PH 1Beam.\n" );
		sc_PhType = THRD_PhType1Beam;
		break;
	case TYPE_PHType2Beam:
		SYSF_logMsg( "[THRL_getPHBeamInfoSpiFlash] SPI-Flash PH 2Beam.\n" );
		sc_PhType = THRD_PhType2Beam;
		break;
	default:
		SYSF_logMsg( "[THRL_getPHBeamInfoSpiFlash] SPI-Flash PH not 1Beam/2Beam %d.\n", sc_PhType );
		break;
	}
	
	// 戻り値設定.
	bl_Ret = False;
	if( sc_PhType != THRD_PhTypeUnknown ){
		bl_Ret = True;	// 取得OK.
	}
	*psc_PhType = sc_PhType;
	
	return bl_Ret;
}

// ================================================================================================
//! <!--機能--> マシンタイプからのPH構成情報取得.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//!  char *psc_PhType PH構成情報(1Beam:THRD_PhType1Beam、2Beam:THRD_PhType2Beam、不明:THRD_PhTypeUnknown)..
//! <!--戻値-->
//! Bool True:成功、False：失敗.
//! <!--詳細-->
//! マシンタイプからのPH構成情報(Beam種別)を取得する.
//! <!--履歴-->
//! 2016/10/25 Sky井柳	【IT5_4.0】制御構成変更No48「CCD切り替え対応」新規作成.
// ================================================================================================
static Bool THRL_getPHBeamInfoMachineType( char *psc_PhType )
{
	if( psc_PhType == NULL ){	// パラメタエラー.
		return False;
	}
	char sc_PhType = THRD_PhTypeUnknown;	// 不明.

	// マシンタイプから判断する.
	TYPE_MachineType e_MachineType = THRF_GetOriginalMachineTypeInfo(); // 製品情報を取得.
	// 製品情報から1Beam/2Beamの切り替えを行う.
	switch( e_MachineType ) {
		case TYPE_MachineTypeZeusSBK0: 		// ZeusSBK0.
			SYSF_logMsg( "[THRG_getPHBeamInfo] MachineType 1Beam.\n" );
			sc_PhType = THRD_PhType1Beam;
			break;
		case TYPE_MachineTypeZeusSBKX: 		// ZeusSBKX.
			SYSF_logMsg( "[THRG_getPHBeamInfo] MachineType 2Beam.\n" );
			sc_PhType = THRD_PhType2Beam;
			break;
		default:
			SYSF_logMsg( "[THRG_getPHBeamInfo] Not PHBeamInfo Switch MachineType %d.\n", e_MachineType );
			break;
	}
	
	// 戻り値設定.
	Bool bl_Ret = False;
	if( sc_PhType != THRD_PhTypeUnknown ){
		bl_Ret = True;	// 取得OK.
	}
	*psc_PhType = sc_PhType;
	
	return bl_Ret;
}

// ================================================================================================
//! <!--機能--> PH構成情報(Beam種別)取得.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//!  なし.
//! <!--戻値-->
//! char PH構成情報(1Beam:THRD_PhType1Beam、2Beam:THRD_PhType2Beam、不明:THRD_PhTypeUnknown).
//! <!--詳細-->
//! PH構成情報(Beam種別)を取得する.
//! <!--履歴-->
//! 2016/10/25 Sky井柳	【IT5_4.0】制御構成変更No48「CCD切り替え対応」新規作成.
// ================================================================================================
static char THRL_getPHBeamInfo()
{
	// エンジンからのPH構成情報取得.
	char sc_PhType = THRD_PhTypeUnknown;	// 不明.
	Bool bl_Ret = THRL_getPHBeamInfoEngine( &sc_PhType );
	if( bl_Ret == True ){
		return sc_PhType;	// 成功時は取得できている.
	}

	// SPI-Flashから設定された情報を取得.
	bl_Ret = THRL_getPHBeamInfoSpiFlash( &sc_PhType );
	if( bl_Ret == True ){
		return sc_PhType;	// 成功時は取得できている.
	}
	
	// マシンタイプからのPH構成情報取得.
	bl_Ret = THRL_getPHBeamInfoMachineType( &sc_PhType );
	if( bl_Ret == True ){
		return sc_PhType;	// 成功時は取得できている.
	}

	// 最終的にわからなかった場合.
	return THRD_PhTypeUnknown;
}
#endif // defined(DEF_ZEUSSBKX0_CUS).
#endif

// ================================================================================================
//! <!--機能--> CCD画パラファイルへのシンボリックリンクの切り替え処理.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! THRE_TYPE_CCD_TYPE e_ccdType：CCD種別 .
//! <!--戻値-->
//! Bool True:成功、False：失敗.
//! <!--詳細-->
//! 指定したCCD種別に対応したCCD画パラファイルへのシンボリックリンクを作成する.
//! Beam切り替え処理の追加について、本来、新旧CCD切り替え時に考慮するべきだったが.
//! 対応時点では、ROM容量の関係で複合することは除外されていた.
//! その後、方針が変わり急遽対応することになり、元々のBeam切り替えを無視して
//! 「CCD切り替え対応」に追加する形での簡易的実現とした.
//! 次対応時にリファイン要.
//! <!--履歴-->
//! 2016/09/08 Sky合屋	【IT5_4.0】制御構成変更No48「CCD切り替え対応」新規作成.
//! 2016/10/25 Sky井柳	【IT5_4.0】制御構成変更No48「CCD切り替え対応」Beam切り替えの複合対応(ZeusSBK X/0のみ).
// ================================================================================================
Bool THRG_changeCCDSymLink( THRE_TYPE_CCD_TYPE e_ccdType )
{
#if defined( DEF_CCD_CHG_ENABLE)
	int si_Ret = -1;
	unsigned int us_Cnt = 0;

	// 指定されたCCD種別が有効な種別か判定する.
	if( (e_ccdType >= THRE_TYPE_CCD_TYPE_MAX) || (0x00 == ((0x01 << e_ccdType) & THRD_CCD_TARGET_TYPE)) ){
		printf("THRG_makeCCDLink:error CCDType=%d.\n", e_ccdType);
		return False;
	}

#if defined(DEF_ZEUSSBKX0_CUS)	// ZeusSBK X/0 … Beam切り替え＆CCD切り替え.
	// PH構成情報取得.
	char sc_PhType = THRL_getPHBeamInfo();
	// PH構成情報から、Beam番号を設定.
	uchar uc_BeamNo = THRD_CCDGaparaBeam_1;
	if ( sc_PhType == THRD_PhType1Beam ) {	// 1Beam.
		uc_BeamNo = THRD_CCDGaparaBeam_1;
	} else if ( sc_PhType == THRD_PhType2Beam ) {	// 2Beam.
		uc_BeamNo = THRD_CCDGaparaBeam_2;
	} else {
		// PH構成情報が不正な場合.
		SYSF_logMsg( "[THRG_changeCCDSymLink] Beam info is nothing Error. \n");
		return False;
	}
#endif //defined(DEF_ZEUSSBKX0_CUS).

	for ( us_Cnt = 0; us_Cnt < THRD_CCDGaparaModuleNum; us_Cnt++ ) {
		//既存のシンボリックリンクを削除.
		si_Ret = remove(sc_ResCCDGaparaSymLinkTbl[us_Cnt]);
#if defined(DEF_ZEUSSBKX0_CUS)	// ZeusSBK X/0 … Beam切り替え＆CCD切り替え.
		// データIndex(モジュール種別)により、Beam切り替えが必要な場合に対応するファイルリストを変更する.
		if( us_Cnt == THRD_CCDGaparaModule_PIC ){	// PIC画パラ.
			si_Ret = symlink(sc_CCDGaparaFileTbl_PIC[e_ccdType][uc_BeamNo], sc_ResCCDGaparaSymLinkTbl[us_Cnt]);
		}
		else if( us_Cnt == THRD_CCDGaparaModule_THR ){	// Thread画パラ.
			si_Ret = symlink(sc_CCDGaparaFileTbl_THR[e_ccdType][uc_BeamNo], sc_ResCCDGaparaSymLinkTbl[us_Cnt]);
		}
		else{	// Beam切り分けの無いもの(CCD切り替えのみ).
		si_Ret = symlink(sc_CCDGaparaFileTbl[us_Cnt][e_ccdType], sc_ResCCDGaparaSymLinkTbl[us_Cnt]);
		}
#else //defined(DEF_ZEUSSBKX0_CUS).
		si_Ret = symlink(sc_CCDGaparaFileTbl[us_Cnt][e_ccdType], sc_ResCCDGaparaSymLinkTbl[us_Cnt]);
#endif //defined(DEF_ZEUSSBKX0_CUS).
		if( si_Ret != 0 ) {
			printf( "THRG_changeIPROMSymLink %s Error. \n", sc_ResCCDGaparaSymLinkTbl[us_Cnt] );
			return False;
		}
	}
#endif
	return True;
}
//↑ 2016/08/31 Sky合屋 [IT5_4.0]制御構成変更No.48.

// ================================================================================================
//! <!--機能--> サム計算(バイトスワップ) - ushort.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! ushort * ps_Buff : アドレス.
//! ulong  ul_Length : バイトサイズ.
//! <!--戻値-->
//! ushort サム値.
//! <!--詳細-->
//! 指定アドレスから指定バイト数まで、バイトスワップしながら２バイトサムを計算する。
//! <!--履歴-->
//! 2016/12/27 ishiga [IT5_4.1][HDDデータバックアップ世代間互換] 新規作成.
// ================================================================================================
ushort THRF_CalcSum_bswap( ushort *ps_Buff, uint32_t ul_Length )
{
	if ( NULL == ps_Buff ) {
		return 0;
	}

	ushort us_Sum = 0;
	for ( uint32_t ul_Cnt = 0; ul_Cnt < ( ul_Length / sizeof(ushort) ); ul_Cnt++ ) {
		ushort us_Data = *ps_Buff++;
		us_Sum += THRF_bswap( us_Data );	// バイトスワップして加算.
	}

	return us_Sum;
}

// ================================================================================================.
//! <!--機能--> HDDリストアパラメータの設定.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! const THRS_HddRestoreParam& HDDリストアパラメータ.
//! <!--戻値-->.
//! なし.
//! <!--詳細-->.
//! CP_HDDTransに HDDリストアパラメータを設定する.
//! <!--履歴-->.
//! 2017/01/05 ishiga [IT5_4.1][HDDデータバックアップ世代間互換].
//! ================================================================================================.
void THRF_SetHddRestoreParam( const THRS_HddRestoreParam& st_Param )
{
	theHDDTrans.setHddRestoreParam( st_Param );
}

// ================================================================================================.
//! <!--機能--> FWVerUp/Downツール用不揮発データ集約関数.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! なし.
//! <!--戻値-->.
//! uint32_t		実行結果		0		：成功.
//! 								650～699：失敗.
//! <!--詳細-->.
//! 対象データをバイナリデータとして集約フォルダに格納をする.
//! 対象データ：パネルのウィジェット情報、パネルのトップメニューテーマ情報、スタンプデータ、コピープロテクトデータ.
//! <!--履歴-->.
//! 2017/03/31	matsui		FW_VerUp/Downツール比較対象の不揮発データ追加対応.
//! ================================================================================================.
uint32_t THRG_FWUDTestCollectData(void)
{
	const char* psc_DstDir = NULL;
	
	APIC_HddInfo* pc_HddInfo = APIC_HddInfo::newInstance();
	if ( NULL == pc_HddInfo ) {
		return THRD_FWUDTestCollect_InternalNG;
	}
	
	//	HDD装着／非装着によってコピー先のディレクトリを差し替える.
	if( True == pc_HddInfo->getHddExist() ) {
		psc_DstDir = THRD_FWUDTest_DstHDD;
	} else {
		psc_DstDir = THRD_FWUDTest_DstEMMC;
	}

	return THRL_FWUDTestCollectRetry( psc_DstDir );
}

// ================================================================================================.
//! <!--機能--> FWVerUp/Downツール用不揮発データ集約関数（内部関数）.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! char*	psc_DstDir				格納先ディレクトリ.
//! <!--戻値-->.
//! uint32_t		実行結果		0		：成功.
//! 								650～699：失敗.
//! <!--詳細-->.
//! 各ファイルのコピーとリスト出力処理を実行する.
//! 失敗した場合は5秒待ってリトライ（最大2回）.
//! <!--履歴-->.
//! 2017/03/31	matsui		FW_VerUp/Downツール比較対象の不揮発データ追加対応.
//! ================================================================================================.
static uint32_t THRL_FWUDTestCollectRetry( const char *psc_DstDir )
{
	uint32_t ul_ExecNum = 0;
	uint32_t ui_Res = 0;
	
	//	リトライ含めて3回まで行う.
	for ( ul_ExecNum = 0; ul_ExecNum < THRD_FWUDTest_RetryNum; ul_ExecNum++ ) {
		
		ui_Res = THRL_FWUDTestCollect( psc_DstDir );
		if ( THRD_FWUDTestCollect_OK == ui_Res )
		{
			break;
		}
		
		//	リトライ間隔は5秒.
		sleep( THRD_FWUDTest_WaitTime );
	}
	
	return ui_Res;
}

// ================================================================================================.
//! <!--機能--> ファイルコピー＆リスト出力.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! char*	psc_DstDir				格納先ディレクトリ.
//! <!--戻値-->.
//! uint32_t		実行結果		0		：成功.
//! 								650～699：失敗.
//! <!--詳細-->.
//! 各ファイルのコピーとリスト出力を行う.
//! <!--履歴-->.
//! 2017/03/31	matsui		FW_VerUp/Downツール比較対象の不揮発データ追加対応.
//! 2017/04/20	iyanagi		【IT5_4.1】[AR1018514]処理を変更.
//! 2017/06/14	iyanagi		【IT5_4.1】[AR1024231]ファイルリスト内のパスを変更.
//! ================================================================================================.
static uint32_t THRL_FWUDTestCollect( const char* psc_DstDir )
{
	if ( NULL == psc_DstDir ) {					// パラメタチェック.
		return THRD_FWUDTestCollect_InternalNG;
	}

	//	ゴミデータ削除.
	uint32_t ui_Res = THRL_FWUDTestCollectDelete( psc_DstDir );
	if( THRD_FWUDTestCollect_OK != ui_Res ){	// 削除処理の失敗チェック.
		return ui_Res;
	}

	//	格納先ディレクトリの生成.
	char sc_DstPath[ THRD_MaxPathLen ] = { 0 };
	snprintf( sc_DstPath, sizeof(sc_DstPath), "%s/%s", psc_DstDir, THRD_ModuleDir );	// 格納先ディレクトリパス作成.
	STATUS e_Result = FILF_Mkdir( sc_DstPath );
	if ( OK != e_Result ) {		//	格納先ディレクトリ作成失敗チェック.
		return THRD_FWUDTestCollect_MkdirNG;
	}

	//	ファイルリストの生成.
	//	ファイルリストのフォーマットは.
	//	date:yyyy.mm.dd.hh.mm.ss.
	//	numoffiles:ファイル数.
	//	格納先ファイルパス （ファイル数分繰り返す）.
	char sc_ListPath[ THRD_MaxPathLen ]	= { 0 };
	snprintf( sc_ListPath, sizeof(sc_ListPath), "%s/%s", psc_DstDir, THRD_FWUDTest_ListName );	// ファイルリストパス作成.

	//	日付データの書き込み.
	time_t s_Time;
	time( &s_Time );
    tm* ps_LocalTime = localtime(&s_Time);
    
    char sc_ListBuf[ THRD_MaxPathLen ] = { 0 };
    snprintf( sc_ListBuf, sizeof(sc_ListBuf), "date:%d.%02d.%02d.%02d.%02d.%02d",
    	(ps_LocalTime->tm_year + 1900), (ps_LocalTime->tm_mon + 1), (ps_LocalTime->tm_mday),
    	ps_LocalTime->tm_hour, ps_LocalTime->tm_min, ps_LocalTime->tm_sec );			//	日付データ作成.
	
	Bool bl_Res = THRL_FWUDTest_ListOut( sc_ListBuf, sc_ListPath );
	if ( False == bl_Res ) {	// 書き込み確認.
		return THRD_FWUDTestCollect_FileListNG;
	}
	
	//	ファイル数の書き込み.
	memset( sc_ListBuf, 0, sizeof(sc_ListBuf) );
    snprintf( sc_ListBuf, sizeof(sc_ListBuf), "numoffiles:%ld", THR_FWUDTestFileNum );	//	ファイル数データ作成.
	bl_Res = THRL_FWUDTest_ListOut( sc_ListBuf, sc_ListPath );
	if ( False == bl_Res ) {	// 書き込み確認.
		return THRD_FWUDTestCollect_FileListNG;
	}
	
	//	各ファイルのコピーとリスト出力を行う.
	uint32_t ul_Cnt = 0;
	for ( ul_Cnt = 0; ul_Cnt < THR_FWUDTestFileNum; ul_Cnt++ ) {
		// データコピー.
		Bool bl_Res = THRL_FWUDTestCollectOne( s_TestFile[ ul_Cnt ], sc_DstPath );
		if ( False == bl_Res ) {	// コピー失敗.
			return s_TestFile[ ul_Cnt ].ui_NGParam;
		}
		// リスト出力.
		memset( sc_ListBuf, 0, sizeof(sc_ListBuf) );
		snprintf( sc_ListBuf, sizeof(sc_ListBuf), "/%s/%s", THRD_ModuleDir, s_TestFile[ ul_Cnt ].psc_DstName );	//	ファイルリストデータ作成.
		bl_Res = THRL_FWUDTest_ListOut( sc_ListBuf, sc_ListPath );
		if ( False == bl_Res ) {	// 書き込み確認.
			return THRD_FWUDTestCollect_FileListNG;
		}
	}
	
	return THRD_FWUDTestCollect_OK;
}

// ================================================================================================.
//! <!--機能--> 集約データの削除.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! char*	psc_DstDir				格納先ディレクトリ.
//! <!--戻値-->.
//! Bool	コピー結果				True：成功 ／ False：失敗.
//! <!--詳細-->.
//! スレッド出力対象の格納先ディレクトリとファイルリストの削除.
//! <!--履歴-->.
//! 2017/04/20	iyanagi		【IT5_4.1】[AR1018514]新規作成.
//! ================================================================================================.
static uint32_t THRL_FWUDTestCollectDelete( const char *psc_DstDir )
{
	if ( NULL == psc_DstDir ) {		// パラメタチェック.
		return THRD_FWUDTestCollect_InternalNG;
	}

	uint32_t ui_FuncRet = THRD_FWUDTestCollect_OK;	// 関数戻り値.

	char sc_CmdBuf[ THRD_MaxPathLen ] = { 0 };	//コマンド実行用文字列.
	int si_Ret = 0;	// コマンド実行戻り値.

	//	ゴミデータを消すため、格納先のスレッドディレクトリを消しておく.
	char sc_Path[ THRD_MaxPathLen ] = { 0 };
	snprintf( sc_Path, sizeof(sc_Path), "%s/%s", psc_DstDir, THRD_ModuleDir );	// 格納先スレッドディレクトリパス作成.
	if( 0 == access(sc_Path, F_OK) ){	// 存在する場合のみ削除する.
		memset( sc_CmdBuf, 0, sizeof(sc_CmdBuf) );
		snprintf( sc_CmdBuf, sizeof(sc_CmdBuf), "exec rm -rf %s", sc_Path );	// ディレクトリ削除コマンド作成.
		si_Ret = THRF_System( sc_CmdBuf );
		if ( si_Ret != 0 ) {	// コマンド実行結果チェック.
			printf( "[%s] Delete Error %s\n", __FUNCTION__, sc_Path );
			ui_FuncRet = THRD_FWUDTestCollect_OldDeleteNG;
		}
	}

	//	ゴミデータを消すため、格納先のスレッドリストファイルを消しておく.
	memset( sc_Path, 0, sizeof(sc_Path) );
	snprintf( sc_Path, sizeof(sc_Path), "%s/%s", psc_DstDir, THRD_FWUDTest_ListName );	//スレッドリストファイルパス作成.
	if( 0 == access(sc_Path, F_OK) ){	// 存在する場合のみ削除する.
		memset( sc_CmdBuf, 0, sizeof(sc_CmdBuf) );
		snprintf( sc_CmdBuf, sizeof(sc_CmdBuf), "exec rm -f %s", sc_Path );	// リストファイル削除コマンド作成.
		si_Ret = THRF_System( sc_CmdBuf );
		if ( si_Ret != 0 ) {	// コマンド実行結果チェック.
			printf( "[%s] Delete Error %s\n", __FUNCTION__, sc_Path );
			ui_FuncRet = THRD_FWUDTestCollect_OldDeleteNG;
		}
	}

	THRF_DoSync();
	return ui_FuncRet;
}

// ================================================================================================.
//! <!--機能--> ファイルコピー（1データ分）.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! THRS_FWUDTestFile st_Data		ファイルデータ.
//! char*	psc_DstDir				格納先ディレクトリ.
//! <!--戻値-->.
//! Bool	コピー結果				True：成功 ／ False：失敗.
//! <!--詳細-->.
//! ファイルデータ単位でコピー処理を振り分ける.
//! <!--履歴-->.
//! 2017/04/20	iyanagi		【IT5_4.1】[AR1018514]新規作成.
//! ================================================================================================.
static Bool THRL_FWUDTestCollectOne( const THRS_FWUDTestFile st_Data, const char* psc_DstDir )
{
	Bool bl_Res = False;
	if( st_Data.bl_BaseIsFile == True ){	// Fileコピー.
		bl_Res = THRL_FWUDTestCollectOneFile( st_Data.psc_BasePath, psc_DstDir, st_Data.psc_DstName );
	}else{	// Dir -> tar File.
		bl_Res = THRL_FWUDTestCollectOneDir( st_Data.psc_BasePath, psc_DstDir, st_Data.psc_DstName );
	}
	THRF_DoSync();
	return bl_Res;
}

// ================================================================================================.
//! <!--機能--> ファイルコピー（1ファイル分）.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! char*	psc_BasePath			コピー元ファイルパス.
//! char*	psc_DstDir				格納先ディレクトリ.
//! char*	psc_DstName				格納先ファイル名.
//! <!--戻値-->.
//! Bool	コピー結果				True：成功 ／ False：失敗.
//! <!--詳細-->.
//! ファイル1つ分のコピー.
//! <!--履歴-->.
//! 2017/03/31	matsui		FW_VerUp/Downツール比較対象の不揮発データ追加対応.
//! 2017/04/20	iyanagi		【IT5_4.1】[AR1018514]引数や処理を変更.
//! ================================================================================================.
static Bool THRL_FWUDTestCollectOneFile( const char* psc_BasePath, const char* psc_DstDir, const char* psc_DstName )
{
	if ( (NULL == psc_BasePath) || (NULL == psc_DstDir) || (NULL == psc_DstName) ) {	// パラメタチェック.
		return False;
	}
	
	//	指定されたファイルのコピー.
	char sc_DstPath[ THRD_MaxPathLen ] = { 0 };
	char sc_CmdBuf[ THRD_MaxPathLen ] = { 0 };
	snprintf( sc_DstPath, sizeof(sc_DstPath), "%s/%s", psc_DstDir, psc_DstName );		// 格納先ファイルパス作成.
	snprintf( sc_CmdBuf, sizeof(sc_CmdBuf), "exec cp %s %s", psc_BasePath, sc_DstPath );// コピーコマンド作成.
	printf( sc_CmdBuf );
	printf( "\n" );

	int si_Ret = THRF_System( sc_CmdBuf );
	if( si_Ret != 0 ){
		return False;
	}

	return True;
}

// ================================================================================================.
//! <!--機能--> ファイルコピー（1ディレクトリ分）.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! char*	psc_BasePath			コピー元ディレクトリパス.
//! char*	psc_DstDir				格納先ディレクトリ.
//! char*	psc_DstName				格納先ファイル名.
//! <!--戻値-->.
//! Bool	コピー結果				True：成功 ／ False：失敗.
//! <!--詳細-->.
//! コピー元のディレクトリを、１つのファイルとしてコピー（tar圧縮）.
//! <!--履歴-->.
//! 2017/04/20	iyanagi		【IT5_4.1】[AR1018514]新規作成.
//! ================================================================================================.
static Bool THRL_FWUDTestCollectOneDir( const char* psc_BasePath, const char* psc_DstDir, const char* psc_DstName )
{
	if ( (NULL == psc_BasePath) || (NULL == psc_DstDir) || (NULL == psc_DstName) ) {	// パラメタチェック.
		return False;
	}
	//	指定されたディレクトリを出力先のファイルに固める(tar圧縮).
	char sc_DstPath[ THRD_MaxPathLen ] = { 0 };
	char sc_CmdBuf[ THRD_MaxPathLen ] = { 0 };
	snprintf( sc_DstPath, sizeof(sc_DstPath), "%s/%s", psc_DstDir, psc_DstName );		// 格納先ファイルパス作成.
	snprintf( sc_CmdBuf, sizeof(sc_CmdBuf), "exec tar cvf %s %s", sc_DstPath, psc_BasePath );// tar圧縮コマンド作成.
	printf( sc_CmdBuf );
	printf( "\n" );

	int si_Ret = THRF_System( sc_CmdBuf );			// コマンド実行.
	if( si_Ret != 0 ){
		return False;
	}
	
	return True;
}

// ================================================================================================.
//! <!--機能--> リストへの出力.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! char*	psc_Str					出力する文字列.
//! char*	psc_ListFilePath		リストファイルパス.
//! <!--戻値-->.
//! Bool	出力結果				True：成功 ／ False：失敗.
//! <!--詳細-->.
//! リストへ文字列を追記する.
//! <!--履歴-->.
//! 2017/03/31	matsui		FW_VerUp/Downツール比較対象の不揮発データ追加対応.
//! 2017/04/20	iyanagi		【IT5_4.1】[AR1018514]引数や処理を変更.
//! ================================================================================================.
static Bool THRL_FWUDTest_ListOut( const char* psc_Str, const char* psc_ListFilePath )
{
	if ( (NULL == psc_Str) || (NULL == psc_ListFilePath) ) {
		return False;
	}
	
	//	1回のバックアップ処理中に何度かコールされるので、追記モードでオープン.
	//	バックアップ処理の初回でディレクトリごと削除されるので、ゴミデータが混じることはない.
	int si_File = FILF_Open( psc_ListFilePath, O_CREAT | O_RDWR | O_APPEND, 0666 );
	if ( ERROR == si_File ) {
		printf( "[%s] OpenError %s\n", __FUNCTION__, psc_ListFilePath );
		return False;
	}
	
	//	指定された文字列を追記.
	int si_Len = strlen(psc_Str);
	int si_Res = FILF_Write( si_File, (char*)psc_Str, si_Len );
	if ( si_Len != si_Res ) {
		FILF_Close( si_File );
		printf( "[%s] WriteStringError(%d/%d) %s\n", __FUNCTION__, si_Len, si_Res, psc_Str );
		return False;
	}
	
	//	改行コードを追記.
	si_Len = strlen("\r\n");
	si_Res = FILF_Write( si_File, "\r\n", si_Len );
	if ( si_Len != si_Res ) {
		FILF_Close( si_File );
		printf( "[%s] WriteReturnError(%d/%d) %s\n", __FUNCTION__, si_Len, si_Res );
		return False;
	}
	
	FILF_Close( si_File );
	THRF_DoSync();
	return True;
}

// ================================================================================================
//! <!--機能--> PIOデータ取得 外部API.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//!  なし.
//! <!--戻値-->
//! ulong 0xFFFFFFFF:Asicの電源が入っていない.
//!       0         :Asic30, Asic2014の両方が接続されていない.
//!       上記以外  :Asic30, Asic2014どちらかのPIOデータ.
//! <!--詳細-->
//! FieryのDMAタイムアウト問題にて新規作成.
//! Asic30 または Asic2014のFiery入力部がリンクダウンしているかどうか判別する.
//! <!--履歴-->
//! 2017/06/07 Sky西村	FieryのDMAタイムアウト問題対応 新規作成.
// ================================================================================================
uint32_t THRG_GetFieryLync()
{
	uint32_t ul_Ret = 0xFFFFFFFF;

	// Asicステート確認用.
	THRC_PowerManagerDevice *pc_PowerDevice = THRC_PowerManagerDevice::getInstance();
	if( pc_PowerDevice == NULL ){
		printf("NULL err\t pc_PowerDevice:%08x\n",pc_PowerDevice );
		return ul_Ret;
	}

	THRC_PowerStateID uc_PCIe_Bus_State = pc_PowerDevice->GetPowerState( THRC_PowerDeviceID_PCIe_BUS );
	THRC_PowerStateID uc_Pexpd_State = pc_PowerDevice->GetPowerState( THRC_PowerDeviceID_PEXPD );

	// Asicのステートを確認する.
	if( ( THRC_PowerStateID_LOW == uc_PCIe_Bus_State ) || ( THRC_PowerStateID_LOW == uc_Pexpd_State ) ){
		// Asicの電源が入っていない場合、何も処理しない.
	}
	else {
		// Asicの電源が入っている場合、PIOデータを取得.
		ul_Ret = THRL_Drvrif_GetFieryLync();
	}

    return ul_Ret;
}

// ================================================================================================.
//! <!--機能--> CPRDYの状態変化通知.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数--> なし.
//! <!--戻値--> なし.
//! <!--詳細-->.
//! 外部コントローラ用I/F
//! CPRDYポートがLow（アクティブ）になったタイミングをスレッドに通知する.
//! <!--履歴-->.
//  2017/04/07 Sky		ZeusSMLKPF対応.
// ================================================================================================.
void THRF_ChangeStatusCprdy( void )
{
	SYSF_logMsg("portCPRDY H->L\n");
	THRL_Drvrif_ChangeStatusCprdy();
}

// ================================================================================================.
//! <!--機能--> Flashで管理している設定値管理クラスのコンストラクタ.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
//  2014/10/22	motohashi	[SecV1-2,3,4] 新規作成.
//  2015/07/09	shibahara	IISW(分割)更新 IISW更新結果の取得処理を追加.
//  2015/10/23	中村		[SecV2-4]暴露保護対応向けI/F更新.
//  2016/01/26	中村 		[SecV2-5]鍵材料管理フラグI/F追加.
//  2016/03/11	中村		CQ.525797[SecV2-4]メインストレージ暗号化を変更すると、FW異常メッセージ提示され、デバイス利用不可.
//  2017/11/09	SCD2-Sky Shindo [WP.53-54] 設定値見直し対応.
// ================================================================================================.
THRC_VerifyUserCtrl::THRC_VerifyUserCtrl()
{
}

// ================================================================================================.
//! <!--機能--> Flashで管理している設定値管理クラスのデストラクタ.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
//  2014/10/22	motohashi	 [SecV1-2,3,4] 新規作成.
// ================================================================================================.
THRC_VerifyUserCtrl::~THRC_VerifyUserCtrl()
{
}

// ================================================================================================.
//! <!--機能--> Flashで管理している設定値管理クラスのインスタンス生成.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return THRC_VerifyUserCtrl* インスタンス.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
//  2014/10/22	motohashi	 [SecV1-2,3,4] 新規作成.
// ================================================================================================.
THRC_VerifyUserCtrl* THRC_VerifyUserCtrl::newInstance()
{
	static THRC_VerifyUserCtrl	s_VerifyUserCtrl;

	return &s_VerifyUserCtrl;
}

// ================================================================================================.
//! <!--機能--> メインストレージ(SSD、eMMC)データ暴露保護機能の設定.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param Bool bl_Mode 設定の有効無効指定、True:有効/False:無効.
//! \param char psc_Password メインストレージ暗号化パスワード.
//! \param uint32_t p_func コールバック関数ポインタ.
//! <!--戻値-->.
//! \return Bool True:成功 / False:失敗.
//! <!--詳細-->.
//! 非同期型なので、戻り値はタスク生成の成功かどうかの情報になる。処理結果はコールバック関数で通知する。\n.
//! 暗号化が通常に完了した場合、暴露保護フラグ更新に合わせてメインストレージの暗号化も行われる。\n.
//! 復号化が通常に完了した場合、暴露保護フラグ更新に合わせてメインストレージの復号化も行われる。\n.
//! <!--履歴-->.
//  2016/01/25	中村	新規作成.
//	2016/02/24	中村	CQ.525378:メインストレージ暗号化設定を行うと、エラーとなるときがある.
//	2016/04/26	中村	CQ.528008:メインストレージ暗号化処理中で、本体自動更新は実行できてしまう.
//	2016/05/11	中村	CQ.528016:メインストレージ暗号化処理中で、副電源OFFモードを移行できてしまう.
// ================================================================================================.
Bool THRC_VerifyUserCtrl::setDataProtectMode( Bool bl_Mode, char* psc_Password, void (*p_func)( int32_t sl_Param ) )
{
	if( NULL == p_func ) {
		printf("%s(%d) %s callback is NULL.\n", __FILE__, __LINE__, __FUNCTION__);
		return False;
	}

	if( NULL == psc_Password ) {
		printf("%s(%d) %s password is NULL.\n", __FILE__, __LINE__, __FUNCTION__);
		return False;
	}

	// 電源キー押下による副電OFF移行を抑止する.
	comMemoryManager.setStorageBusy( True );
	APIC_Power::newInstance()->changePowerMode( TYPE_PowerModeDataAccessStart, __FILE__, __LINE__ );

	stDataProtect.bl_Mode = bl_Mode;
	memset( stDataProtect.sc_Password, 0x0, sizeof(stDataProtect.sc_Password) );
	strncpy( stDataProtect.sc_Password, psc_Password, TYPD_MainStorageEncryptionPassword_MaxLen );
	memset( &(stDataProtect.s_KeyInfo), 0x00, sizeof(stDataProtect.s_KeyInfo) );
	stDataProtect.p_func = p_func;
	
	int si_Ret = SYSF_taskSpawn( "TASK_DataProtect", 90, VX_FP_TASK, 8192, (FUNCPTR)THRC_VerifyUserCtrl::DataProtectTask, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 );

	return ( ERROR == si_Ret ) ? False : True;
}
// 2016/05/09 SCD2-Sky Kawanishi [SecV2-9] 暗号鍵インポート対応.
// ================================================================================================.
//! <!--機能--> メインストレージ(SSD、eMMC)データ暴露保護機能の設定(鍵材料インポート用).
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param Bool bl_Mode 設定の有効無効指定、True:有効/False:無効.
//! \param TYPS_StorageEncryptionKeyInfo* ps_KeyInfo メインストレージ暗号化鍵材料.
//! \param uint32_t p_func コールバック関数ポインタ.
//! <!--戻値-->.
//! \return Bool True:成功 / False:失敗.
//! <!--詳細-->.
//! 非同期型なので、戻り値はタスク生成の成功かどうかの情報になる。処理結果はコールバック関数で通知する。\n.
//! 暗号化が通常に完了した場合、暴露保護フラグ更新に合わせてメインストレージの暗号化も行われる。\n.
//! 復号化が通常に完了した場合、暴露保護フラグ更新に合わせてメインストレージの復号化も行われる。\n.
//! <!--履歴-->.
//	2016/05/09 SCD2-Sky Kawanishi 新規作成 [SecV2-9] 暗号鍵インポート対応.
// ================================================================================================.
Bool THRC_VerifyUserCtrl::setDataProtectMode( Bool bl_Mode, const TYPS_StorageEncryptionKeyInfo* ps_KeyInfo, void (*p_func)( int32_t sl_Param ) )
{
	if( NULL == p_func ) {
		printf("%s(%d) %s callback is NULL.\n", __FILE__, __LINE__, __FUNCTION__);
		return False;
	}

	if( NULL == ps_KeyInfo ) {
		printf("%s(%d) %s keyinfo is NULL.\n", __FILE__, __LINE__, __FUNCTION__);
		return False;
	}

	// 電源キー押下による副電OFF移行を抑止する.
	comMemoryManager.setStorageBusy( True );
	APIC_Power::newInstance()->changePowerMode( TYPE_PowerModeDataAccessStart, __FILE__, __LINE__ );

	stDataProtect.bl_Mode = bl_Mode;
	memset( stDataProtect.sc_Password, 0x0, sizeof(stDataProtect.sc_Password) );
	memcpy( &(stDataProtect.s_KeyInfo), ps_KeyInfo, sizeof(stDataProtect.s_KeyInfo) );
	stDataProtect.p_func = p_func;
	
	int si_Ret = SYSF_taskSpawn( "TASK_DataProtect", 90, VX_FP_TASK, 8192, (FUNCPTR)THRC_VerifyUserCtrl::DataProtectTask, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 );

	return ( ERROR == si_Ret ) ? False : True;
}
// ================================================================================================.
//! <!--機能--> メインストレージ(SSD、eMMC)データ保護タスク.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
//  2015/10/23	中村	新規作成.
//  2016/01/25	中村	メインストレージ(SSD、eMMC)向けとするため、引数見直しならびにサブストレージ処理を削除.
//	2016/02/24	中村	CQ.525378:メインストレージ暗号化設定を行うと、エラーとなるときがある.
//	2016/05/09 SCD2-Sky Kawanishi 新規作成 [SecV2-9] 暗号鍵インポート対応.
//  2017/11/09	SCD2-Sky Shindo [WP.53-54] 設定値見直し対応.
// ================================================================================================.
void THRC_VerifyUserCtrl::DataProtectTask()
{
	int32_t				sl_Ret = 0;
	short				s_Ret = 0;
	APIC_SystemSetting	*pc_SecuritySystemSetting = NULL;

	// 処理結果を返せない状態では、処理を行わない.
	if ( NULL == stDataProtect.p_func ) {
		printf("%s(%d) %s no callback param.\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}

	TYPS_StorageEncryptionKeyInfo s_KeyInfo;
	memset(&s_KeyInfo, 0x00, sizeof(s_KeyInfo));
	if ( stDataProtect.bl_Mode && (0 == strlen( stDataProtect.sc_Password)) && (0 == memcmp(&(stDataProtect.s_KeyInfo), &s_KeyInfo, sizeof(stDataProtect.s_KeyInfo)))){
		printf( "%s(%d) %s Error, password length is zero.\n", __FILE__, __LINE__, __FUNCTION__ );
		sl_Ret = -1;
		goto callback;
	}

	pc_SecuritySystemSetting = APIC_SystemSetting::newInstance();
	if( NULL == pc_SecuritySystemSetting ) {
		printf( "%s(%d) %s Error, failed APIC_SystemSetting::newInstance().\n", __FILE__, __LINE__, __FUNCTION__ );
		sl_Ret = -1;
		goto callback;
	}

	// データ暴露保護セットアップ処理.
	if (0 != strlen( stDataProtect.sc_Password)) {
		s_Ret = theHDDTrans.setProtectExpose(stDataProtect.bl_Mode, stDataProtect.sc_Password, NULL);
	}
	else {
		s_Ret = theHDDTrans.setProtectExpose(stDataProtect.bl_Mode, NULL, &(stDataProtect.s_KeyInfo));
	}
	if ( 0 != s_Ret ) {
		printf("%s(%d) %s setProtectExpose() error.\n", __FILE__, __LINE__, __FUNCTION__);
		sl_Ret = -1;
		goto callback;
	}

	// 正常終了したので、暴露保護フラグ(Flash)更新.
	if( False == pc_SecuritySystemSetting->reqMainStorageEnc(stDataProtect.bl_Mode) ) {
		printf("%s(%d) %s reqMainStorageEnc error.\n", __FILE__, __LINE__, __FUNCTION__);
		sl_Ret = -1;
		goto callback;
	}

callback:
	// コールバック関数を呼び出し.
	stDataProtect.p_func( sl_Ret );

}

// ================================================================================================
//! <!--機能--> JAM発生時詳細コード設定 外部API.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//!  なし.
//! <!--戻値-->
//! \return なし.
//! <!--詳細-->
//! PrintPからコールされ、JAM発生時に各スレッドの状態や.
//! Kernel側のデータを詳細コードと共に残す.
//! <!--履歴-->
//! 2018/07/06 Sky西村	FQ-OPSD-1700439-QU097対応 新規作成.
// ================================================================================================
void setJAMDataDetailCode() {
	Bool bl_ret = True;
	uint64_t ull_NewDetailCode = 0;
	THRS_GetJamDataArg s_getJamDataArg;

	// 構造体パラメータ初期化.
	s_getJamDataArg.e_KernelTodInterruptState = 0;
	s_getJamDataArg.b_DMAStatus_DMA02 = False;
	s_getJamDataArg.b_DMAStatus_DMA03 = False;
	s_getJamDataArg.b_DMAStatus_DMA05 = False;
	memset(s_getJamDataArg.b_DMAStatus_DMA32, 0, sizeof(s_getJamDataArg.b_DMAStatus_DMA32));
	s_getJamDataArg.us_EngQueueFreeNum = 0;
	s_getJamDataArg.us_ConvQueueFreeNum = 0;

	// Kernel側のデータを取得する.
	THRL_DrvrIf_GetJAMKernelData( &s_getJamDataArg );

	// 各スレッド状態の詳細コードデータ整形.
	ull_NewDetailCode = THRD_NewDetail_ModuleId_Thread;									//  1- 2桁：0x20（モジュール番号：Thread）.
	ull_NewDetailCode |= ((uint64_t) 0x1FFF				<< 40) & 0x00ffff0000000000LL;	//  3- 6桁：シーケンス/イベント異常識別(0x1FFF:トラブルなし).
	ull_NewDetailCode |= ((uint64_t) 0x05				<< 32) & 0x000000ff00000000LL;	//  7- 8桁：05:JAM発生時データ設定.
	ull_NewDetailCode |= ((uint64_t) e_MfpFileToBufStart_1						<< 28) & 0x00000000f0000000LL;	//  9桁：FileToBuf(1次伸張)転送開始ステータス.
	ull_NewDetailCode |= ((uint64_t) e_MfpFileToBufStart_2						<< 24) & 0x000000000f000000LL;	// 10桁：FileToBuf(2次伸張)転送開始ステータス.
	ull_NewDetailCode |= ((uint64_t) e_MfpBufToBufStart							<< 20) & 0x0000000000f00000LL;	// 11桁：BufToBuf転送開始ステータス.
	ull_NewDetailCode |= ((uint64_t) e_MfpBufToFileStart						<< 16) & 0x00000000000f0000LL;	// 12桁：BufToFile転送開始ステータス.
	ull_NewDetailCode |= ((uint64_t) e_MfpBufToExtStart							<< 12) & 0x000000000000f000LL;	// 13桁：BufToFile転送開始ステータス.
	ull_NewDetailCode |= ((uint64_t) s_getJamDataArg.e_KernelTodInterruptState	<<  8) & 0x0000000000000f00LL;	// 14桁：TODの通知処理のステータス.
	ull_NewDetailCode |= ((uint64_t) s_getJamDataArg.us_EngQueueFreeNum			<<  4) & 0x00000000000000f0LL;	// 15桁：エンジン用Queueの空きキュー数.
	ull_NewDetailCode |= ((uint64_t) s_getJamDataArg.us_ConvQueueFreeNum		<<  0) & 0x000000000000000fLL;	// 16桁：画像変換用Queueの空きキュー数.

	bl_ret = APIC_setNewDetailCode( 0, ull_NewDetailCode );		// 詳細コード登録.
	if (False == bl_ret) {
		SYSF_logMsg( "## %s %d : APIC_setNewDetailCode NG\n", __FUNCTION__, __LINE__, 3,4,5,6 );
	}

	// DMA動作状態(DMA02,03,05,32-36)の詳細コードデータ整形.
	ull_NewDetailCode = THRD_NewDetail_ModuleId_Thread;														//  1- 2桁：0x20（モジュール番号：Thread）.
	ull_NewDetailCode |= ((uint64_t) 0x1FFF									<< 40) & 0x00ffff0000000000LL;	//  3- 6桁：シーケンス/イベント異常識別(0x1FFF:トラブルなし).
	ull_NewDetailCode |= ((uint64_t) 0x06									<< 32) & 0x000000ff00000000LL;	//  7- 8桁：06:JAM発生時Kernel側データ設定.
	ull_NewDetailCode |= ((uint64_t) s_getJamDataArg.b_DMAStatus_DMA02		<< 31) & 0x0000000080000000LL;	//  9桁1bit目：DMA02の動作状態.
	ull_NewDetailCode |= ((uint64_t) s_getJamDataArg.b_DMAStatus_DMA03		<< 30) & 0x0000000040000000LL;	//  9桁2bit目：DMA03の動作状態.
	ull_NewDetailCode |= ((uint64_t) s_getJamDataArg.b_DMAStatus_DMA05		<< 29) & 0x0000000020000000LL;	//  9桁3bit目：DMA05の動作状態.
	ull_NewDetailCode |= ((uint64_t) s_getJamDataArg.b_DMAStatus_DMA32[0]	<< 28) & 0x0000000010000000LL;	//  9桁4bit目：DMA32の動作状態.
	ull_NewDetailCode |= ((uint64_t) s_getJamDataArg.b_DMAStatus_DMA32[1]	<< 27) & 0x0000000008000000LL;	// 10桁1bit目：DMA33の動作状態.
	ull_NewDetailCode |= ((uint64_t) s_getJamDataArg.b_DMAStatus_DMA32[2]	<< 26) & 0x0000000004000000LL;	// 10桁2bit目：DMA34の動作状態.
	ull_NewDetailCode |= ((uint64_t) s_getJamDataArg.b_DMAStatus_DMA32[3]	<< 25) & 0x0000000002000000LL;	// 10桁3bit目：DMA35の動作状態.
	ull_NewDetailCode |= ((uint64_t) s_getJamDataArg.b_DMAStatus_DMA32[4]	<< 24) & 0x0000000001000000LL;	// 10桁4bit目：DMA36の動作状態.
	ull_NewDetailCode |= ((uint64_t) s_getJamDataArg.b_DMAStatus_DMA32[5]	<< 23) & 0x0000000000800000LL;	// 11桁1bit目：DMA37の動作状態.
	ull_NewDetailCode |= ((uint64_t) s_getJamDataArg.b_DMAStatus_DMA32[6]	<< 22) & 0x0000000000400000LL;	// 11桁2bit目：DMA38の動作状態.
	ull_NewDetailCode |= ((uint64_t) s_getJamDataArg.b_DMAStatus_DMA32[7]	<< 21) & 0x0000000000200000LL;	// 11桁3bit目：DMA39の動作状態.
	ull_NewDetailCode |= ((uint64_t) s_getJamDataArg.b_DMAStatus_DMA32[8]	<< 20) & 0x0000000000100000LL;	// 11桁4bit目：DMA40の動作状態.
	ull_NewDetailCode |= ((uint64_t) s_getJamDataArg.b_DMAStatus_DMA32[9]	<< 19) & 0x0000000000080000LL;	// 12桁1bit目：DMA41の動作状態.
	ull_NewDetailCode |= ((uint64_t) s_getJamDataArg.b_DMAStatus_DMA32[10]	<< 18) & 0x0000000000040000LL;	// 12桁2bit目：DMA42の動作状態.
	ull_NewDetailCode |= ((uint64_t) s_getJamDataArg.b_DMAStatus_DMA32[11]	<< 17) & 0x0000000000020000LL;	// 12桁3bit目：DMA43の動作状態.
	ull_NewDetailCode |= ((uint64_t) s_getJamDataArg.b_DMAStatus_DMA32[12]	<< 16) & 0x0000000000010000LL;	// 12桁4bit目：DMA44の動作状態.
	ull_NewDetailCode |= ((uint64_t) s_getJamDataArg.b_DMAStatus_DMA32[13]	<< 15) & 0x0000000000008000LL;	// 13桁1bit目：DMA45の動作状態.
	ull_NewDetailCode |= ((uint64_t) s_getJamDataArg.b_DMAStatus_DMA32[14]	<< 14) & 0x0000000000004000LL;	// 13桁2bit目：DMA46の動作状態.
	ull_NewDetailCode |= ((uint64_t) s_getJamDataArg.b_DMAStatus_DMA32[15]	<< 13) & 0x0000000000002000LL;	// 13桁3bit目：DMA47の動作状態.
	ull_NewDetailCode |= ((uint64_t) s_getJamDataArg.b_DMAStatus_DMA32[16]	<< 12) & 0x0000000000001000LL;	// 13桁4bit目：DMA48の動作状態.
	ull_NewDetailCode |= ((uint64_t) s_getJamDataArg.b_DMAStatus_DMA32[17]	<< 11) & 0x0000000000000800LL;	// 14桁1bit目：DMA49の動作状態.
	ull_NewDetailCode |= ((uint64_t) s_getJamDataArg.b_DMAStatus_DMA32[18]	<< 10) & 0x0000000000000400LL;	// 14桁2bit目：DMA50の動作状態.
	ull_NewDetailCode |= ((uint64_t) s_getJamDataArg.b_DMAStatus_DMA32[19]	<<  9) & 0x0000000000000200LL;	// 14桁3bit目：DMA51の動作状態.
	ull_NewDetailCode |= ((uint64_t) s_getJamDataArg.b_DMAStatus_DMA32[20]	<<  8) & 0x0000000000000100LL;	// 14桁4bit目：DMA52の動作状態.
	ull_NewDetailCode |= ((uint64_t) s_getJamDataArg.b_DMAStatus_DMA32[21]	<<  7) & 0x0000000000000080LL;	// 15桁1bit目：DMA53の動作状態.
	ull_NewDetailCode |= ((uint64_t) s_getJamDataArg.b_DMAStatus_DMA32[22]	<<  6) & 0x0000000000000040LL;	// 15桁2bit目：DMA54の動作状態.
	ull_NewDetailCode |= ((uint64_t) s_getJamDataArg.b_DMAStatus_DMA32[23]	<<  5) & 0x0000000000000020LL;	// 15桁3bit目：DMA55の動作状態.
	ull_NewDetailCode |= ((uint64_t) s_getJamDataArg.b_DMAStatus_DMA32[24]	<<  4) & 0x0000000000000010LL;	// 15桁4bit目：DMA56の動作状態.

	bl_ret = APIC_setNewDetailCode( 0, ull_NewDetailCode );		// 詳細コード登録.
	if (False == bl_ret) {
		SYSF_logMsg( "## %s %d : APIC_setNewDetailCode NG\n", __FUNCTION__, __LINE__, 3,4,5,6 );
	}
}

// Debug.
#if 0
#define PASS_SIZE		(64)
char scPass[PASS_SIZE + 1] = "123456789012345678901234567890123456789012345678901234567890aaaa";
char* gpscPass = NULL;

void TVerifyUserCtrl_cb( int32_t res )
{
	printf( "DBG:% 6d:%s: result[%d].\n", __LINE__, __func__, res );
	
	if( NULL != gpscPass ) {
		delete[] gpscPass;
		gpscPass = NULL;
	}
}

int32_t TVerifyUserCtrl_debug(Bool bl_mode, Bool bl_pwd, Bool bl_func)
{
	void(*cb)( int32_t res ) = NULL;
	char *pass = NULL;

	if(bl_func) {
		cb = &TVerifyUserCtrl_cb;
	}

	if(bl_pwd) {
		pass = new char[PASS_SIZE + 1];
		if(NULL == pass) {
			printf("DBG:% 6d:%s, Error new().\n", __LINE__, __func__ );
			return False;
		}

		memset(pass, 0x0, PASS_SIZE + 1 );
		memcpy(pass, scPass, PASS_SIZE);
		gpscPass = pass;
	}

	Bool bl_ret = THRC_VerifyUserCtrl::newInstance()->setDataProtectMode(bl_mode, pass, cb);

	printf( "DBG:% 6d:%s: result[%d].\n", __LINE__, __func__, bl_ret );
	return bl_ret;
}

#endif
// ================================================================================================.
//! <!--機能--> エンジンタイプによって画パラタイプをシンボリックリンク切り替え関数へ渡す.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//!  uchar	エンジンタイプ.
//! <!--戻値-->.
//! \Bool True:作成成功 False:作成失敗.
//! <!--詳細-->.
//! 引数で受け取ったエンジンタイプをもとに画パラのシンボリックリンクを切り替える.
//! エンジンタイプが対象機種以外の場合はリンクの作成は行わずTrueで返却する.
//! <!--履歴-->.
//! 2018/10/18 Sky松井	[Eagle_High]制御構成変更No.47 新規作成.
//! 2021/07/01 Sky		A800レス対応.
//! 2022/06/22 Sky		[IT6_2.3] 制御構成変更No.5 画像設計対応.
// ================================================================================================.
Bool THRL_changeImageParamSymLinkEngine( uchar uc_EngineType )
{
// 画パラの切り替えは、Eagle全機種、Sparrow全機種が対象.
#if (defined( DEF_EagleCus ) && !defined( DEF_HeliosMLKCus ) && !defined(DEF_MinervaSSBKCus)) || defined( DEF_SparrowCus )

	THRE_ImageParameterType e_ImgPrmType = THRE_ImageParameterType1;	// 画パラタイプ設定用.
	char sc_Type = 0;

// EagleL/EagleBKLのみ変換(その他の機種は、エンジンタイプ不要).
#if (defined( DEF_EagleBKCus ) || defined( DEF_EagleCus )) && !defined(DEF_A800)
	switch( uc_EngineType )
	{
		case TYPE_EngineType3:
			sc_Type = THRE_MACHINENO_1;
			break;

		case TYPE_EngineType4:
			sc_Type = THRE_MACHINENO_2;
			break;

		case TYPE_EngineType5:
			sc_Type = THRE_MACHINENO_2_5;
			break;

		case TYPE_EngineType2:
			sc_Type = THRE_MACHINENO_0;
			break;

		case TYPE_EngineType1:
			sc_Type = THRE_MACHINENO_X;
			break;

		case TYPE_EngineType6:
			sc_Type = THRE_MACHINENO_Z;
			break;

		default:
			SYSF_logMsg( "[%s]symlink Failed uc_EngineType = [%x]\n", __FUNCTION__, uc_EngineType );
			return False;
	}
#endif // (defined( DEF_EagleBKCus ) || defined( DEF_EagleCus )) && !defined(DEF_A800)

	e_ImgPrmType = THRL_getImageParameterType(sc_Type);

	Bool bl_Ret = THRL_changeImageParamSymLink( e_ImgPrmType );
	if( bl_Ret == False){
		SYSF_logMsg( "[%s]symlink Failed e_ImgPrmType = [%d]\n", __FUNCTION__, e_ImgPrmType );
		return False;
	}
#endif // (defined( DEF_EagleCus ) && !defined( DEF_HeliosMLKCus ) && !defined(DEF_MinervaSSBKCus)) || defined( DEF_SparrowCus ).

	return True;
}

// ================================================================================================.
//! <!--機能--> マシンタイプによって画パラタイプをシンボリックリンク切り替え関数へ渡す.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//!  char	速度情報(タイプ).
//! <!--戻値-->.
//! \Bool True:作成成功 False:作成失敗.
//! <!--詳細-->.
//! 引数で受け取った速度情報(タイプ)をもとに画パラのシンボリックリンクを切り替える.
//! 速度情報(タイプ)が対象機種以外の場合はリンクの作成は行わずTrueで返却する.
//! <!--履歴-->.
//! 2018/10/18 Sky松井	[Eagle_High]制御構成変更No.47 新規作成.
//! 2021/07/01 Sky		A800レス対応.
//! 2022/06/22 Sky		[IT6_2.3] 制御構成変更No.5 画像設計対応.
// ================================================================================================.
Bool THRL_changeImageParamSymLinkMachine( char sc_Type )
{
// 画パラの切り替えは、Eagle全機種、Sparrow全機種が対象.
#if (defined( DEF_EagleCus ) && !defined( DEF_HeliosMLKCus ) && !defined(DEF_MinervaSSBKCus)) || defined( DEF_SparrowCus )

	THRE_ImageParameterType e_ImgPrmType = THRL_getImageParameterType(sc_Type);		// 画パラタイプ取得.

	Bool bl_Ret = THRL_changeImageParamSymLink( e_ImgPrmType );
	if( bl_Ret == False){
		SYSF_logMsg( "[%s]symlink Failed e_ImgPrmType = [%d]\n", __FUNCTION__, e_ImgPrmType );
		return False;
	}
#endif // (defined( DEF_EagleCus ) && !defined( DEF_HeliosMLKCus ) && !defined(DEF_MinervaSSBKCus)) || defined( DEF_SparrowCus ).

	return True;
}

// ================================================================================================.
//! <!--機能--> 引数で受け取った画パラによって画パラファイルのシンボリックリンクを切り替える.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//!  uchar	画パラタイプ.
//! <!--戻値-->.
//! \Bool True:作成成功 False:作成失敗.
//! <!--詳細-->.
//! 引数で受け取ったエンジンタイプをもとに画パラのシンボリックリンクを切り替える.
//! エンジンタイプが対象機種以外の場合はリンクの作成は行わずTrueで返却する.
//! <!--履歴-->.
//! 2018/10/18 Sky松井	[Eagle_High]制御構成変更No.47 新規作成.
//! 2022/06/22 Sky		[IT6_2.3] 制御構成変更No.5 画像設計対応.
// ================================================================================================.
Bool THRL_changeImageParamSymLink( THRE_ImageParameterType e_ImgPrmType )
{
	Bool bl_Ret = False;	// 作成関数戻り値判定用.
	uchar uc_Cnt = 0;		// ループカウント用.

	// 画パラ切り替え情報リストを基にシンボリックリンクの作成を行う.
	// 画パラ切り替え対象機種以外の場合は「THRD_ImgPrmInfListCount」の値が0となる為、実行されない.
	for( uc_Cnt = 0; uc_Cnt < THRD_ImgPrmInfListCount; uc_Cnt++ ){
		// シンボリックリンク不要な場合は、skip.
		if( 0 == pst_ImgPrmInfList[ uc_Cnt ]->st_LibCnt )
		{
			continue;
		}

		// シンボリックリンクの作成.
		bl_Ret = THRL_makeImageParamSymLink( pst_ImgPrmInfList[ uc_Cnt ], e_ImgPrmType );
		if( False == bl_Ret){	// シンボリックリンク作成失敗時はログに残す.
			SYSF_logMsg( "[%s]symlink Failed uc_Cnt = [ %d ] uc_ImgPrmType = [ %d ]\n", __FUNCTION__, uc_Cnt, e_ImgPrmType );
			return False;
		}
	}
	return True;
}
// ================================================================================================.
//! <!--機能--> 画パラのシンボリックリンクの作成を行う.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! THRS_ImageParameterInfo * 画パラ切り替え.
//! uchar 画パラタイプ指定.
//! <!--戻値-->.
//! \return Bool	True:作成成功、False:作成失敗.
//! <!--詳細-->.
//! 引数で受け取った画パラ切り替え情報を元にシンボリックリンクの作成を行う.
//! <!--履歴-->.
//  2018/10/13	Sky松井	[Eagle_High] 制御構成変更No.47 新規作成.
// ================================================================================================.
Bool THRL_makeImageParamSymLink( THRS_ImageParameterInfo *psc_ImgPrm, THRE_ImageParameterType e_ImgPrmType )
{
	if( NULL == psc_ImgPrm ) {		// 画パラ切り替え情報 NULLチェック.
		SYSF_logMsg( "[%s]symlink Failed psc_ImgPrm is NULL\n", __FUNCTION__ );
		return False;
	}
	// 画パラタイプが範囲外の場合はFalseを返却する.
	if( psc_ImgPrm->st_LibCnt <= e_ImgPrmType ){
		SYSF_logMsg( "[%s]symlink Failed ImgPrmType is outLange uc_ImgPrmType = [%d]\n", __FUNCTION__, e_ImgPrmType );
		return False;
	}

	char ch_LinkFilePath[256] = { 0 };	// シンボリックリンク作成先パス.
	char ch_LibFilePath[256] = { 0 };	// ライブラリ(実体)パス.

	// シンボリックリンク作成先/削除先パスを生成.
	snprintf(ch_LinkFilePath, sizeof( ch_LinkFilePath ), "%s%s", THRD_ImageParam_LinkMakePath, psc_ImgPrm->psc_LibLinkName );
	// シンボリックリンクを削除.
	short ss_Ret = remove( ch_LinkFilePath );

	// シンボリックリンク作成元パス生成.
	snprintf(ch_LibFilePath, sizeof(ch_LibFilePath), "%s%s", psc_ImgPrm->psc_FileDir, psc_ImgPrm->psc_LibName[ e_ImgPrmType ] );

	// /res配下にシンボリックリンクを作成.
	ss_Ret = symlink( ch_LibFilePath, ch_LinkFilePath );
	if( ss_Ret != 0 ) {		// シンボリックリンク作成に失敗した場合はSubLogに残す.
		SYSF_logMsg( "[%s]symlink Failed symlink( [%s], [%s] ) errno(%d details:%s)\n", __FUNCTION__, ch_LibFilePath, ch_LinkFilePath, errno, strerror(errno));
		return False;
	}
	return True;
}

// ================================================================================================.
//! <!--機能--> 画パラタイプ取得.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//!  char sc_Type. エンジンタイプ.
//! <!--戻値-->.
//! \return THRE_ImageParameterType	画パラタイプ.
//! <!--詳細-->.
//! 画パラタイプの判定を行い画パラタイプを返却する.
//! EagleL/EagleBKLの場合のみエンジンタイプによって切り替える.
//! その他の機種の場合は、DipSwのみ参照するためsc_Typeは影響しない.
//! <!--履歴-->.
//! 2022/06/22 Sky		[IT6_2.3] 制御構成変更No.5 画像設計対応.
//! 2022/08/23 Sky		[IT6_2.3] 機器構成変更_No.39_CISサプライヤ変更.
//! 2022/10/03 Sky		[IT6_2.3] 制御構成変更No.64 画パラをDipSWで差し替える.
// ================================================================================================.
THRE_ImageParameterType THRL_getImageParameterType( char sc_Type )
{
	THRE_ImageParameterType e_ImgPrmType = THRE_ImageParameterType1;	// 画パラタイプ設定用.

	// 画質設定用パラメータを取得.
	uchar uc_ImgQuality = 0;
	APIC_SoftDipSw* pc_SoftDipSw = APIC_SoftDipSw::newInstance();
	if( pc_SoftDipSw != NULL )
	{
		uc_ImgQuality = (uchar)pc_SoftDipSw->getSoftDipSwBit( 155, 3 );	// DipSW188-bit7.
	}

// EagleL/EagleBKのみエンジン種別で異なる.
#if (defined( DEF_EagleBKCus ) || defined( DEF_EagleCus )) && !defined(DEF_A800) && !defined(DEF_HeliosMLKCus)
	switch( uc_ImgQuality )
	{
		// 従来画質.
		case 0:
			switch( sc_Type )
			{
				case THRE_MACHINENO_1:
				case THRE_MACHINENO_2:
				case THRE_MACHINENO_2_5:
// 2022/08/23 Sky [IT6_2.3] 機器構成変更_No.39_CISサプライヤ変更_START.
#ifdef SIMULATOR	// SIMではAPはコールしない.
					if( 1 ) // 旧CIS.
#else
					if( 0 == IRMG_GetCISTypeFlag() ) // 旧CIS.
#endif
// 2022/08/23 Sky [IT6_2.3] 機器構成変更_No.39_CISサプライヤ変更_END.
					{
						e_ImgPrmType = THRE_ImageParameterType1;
					}
					else	// 新CIS.
					{
						e_ImgPrmType = THRE_ImageParameterType2;
					}
					break;

				case THRE_MACHINENO_0:
					e_ImgPrmType = THRE_ImageParameterType3;
					break;

				case THRE_MACHINENO_X:
				case THRE_MACHINENO_Z:
					e_ImgPrmType = THRE_ImageParameterType4;
					break;
			}
			break;

		// 画質改善.
		case 1:
			switch( sc_Type )
			{
				case THRE_MACHINENO_1:
				case THRE_MACHINENO_2:
				case THRE_MACHINENO_2_5:
// 2022/08/23 Sky [IT6_2.3] 機器構成変更_No.39_CISサプライヤ変更_START.
#ifdef SIMULATOR	// SIMではAPはコールしない.
					if( 1 ) // 旧CIS.
#else
					if( 0 == IRMG_GetCISTypeFlag() ) // 旧CIS.
#endif
// 2022/08/23 Sky [IT6_2.3] 機器構成変更_No.39_CISサプライヤ変更_END.
					{
						e_ImgPrmType = THRE_ImageParameterType5;
					}
					else	// 新CIS.
					{
						e_ImgPrmType = THRE_ImageParameterType6;
					}
					break;

				case THRE_MACHINENO_0:
					e_ImgPrmType = THRE_ImageParameterType7;
					break;

				case THRE_MACHINENO_X:
				case THRE_MACHINENO_Z:
					e_ImgPrmType = THRE_ImageParameterType8;
					break;
			}
			break;

		default:
			break;
	}
#else // (defined( DEF_EagleBKCus ) || defined( DEF_EagleCus )) && !defined(DEF_A800) && !defined(DEF_HeliosMLKCus).
	// 画質改善.
	if( 1 == uc_ImgQuality )
	{
		e_ImgPrmType = THRE_ImageParameterType2;
	}
#endif // (defined( DEF_EagleBKCus ) || defined( DEF_EagleCus )) && !defined(DEF_A800) && !defined(DEF_HeliosMLKCus).
	SYSF_logMsg("THRL_getImageParameterType:e_ImgPrmType=%d (uc_ImgQuality=%d)(sc_Type=%d)\n", e_ImgPrmType, uc_ImgQuality, sc_Type);

	return e_ImgPrmType;
}

// ================================================================================================.
//! <!--機能--> DipSW確定時の画パラシンボリックリンク切り替え.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//!  なし.
//! <!--戻値-->.
//! \Bool True:作成成功 False:作成失敗.
//! <!--詳細-->.
//! サービスモードでの画質変更DipSWにより画パラのシンボリックリンクを切り替える場合に呼び出す.
//! エンジンタイプによって切り替えるため、THRL_changeImageParamSymLinkEngineをwrapして対応する.
//! <!--履歴-->.
//! 2022/06/22 Sky		[IT6_2.3] 制御構成変更No.5 画像設計対応.
// ================================================================================================.
Bool THRL_changeImageParamSymLinkSw( void )
{
	Bool bl_Ret = True;
	unsigned char uc_EngineType = 0;

	APIC_PrinterSystem* pc_APIPrinterSystem = APIC_PrinterSystem::newInstance();
	if( pc_APIPrinterSystem == NULL )
	{
		return False; 
	}

	uc_EngineType = pc_APIPrinterSystem->getEngineType();

	// エンジンに合わせてシンボリックリンク切り替え.
	bl_Ret = THRL_changeImageParamSymLinkEngine(uc_EngineType);

	return bl_Ret;
}

// 2018/08/23 SCD2-Sky Kawanishi [EagleH][SecV4][RQ-676] OSS単独更新機能.
#define THRD_OSS_VersionSize	40
#define THRD_OSL_VerFileName	"OSL.ver"
#define THRD_CHR_VerFileName	"CHR.ver"
#define THRD_NDJ_VerFileName	"NDJ.ver"
#define THRD_UPL_VerFileName	"UPL.ver"

// ================================================================================================.
//! <!--機能--> OSS種別→ファイル名変換.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数--> uc_OSSType	OSS種別.
//! <!--戻値--> ファイル名.
//! <!--詳細-->.
//! OSSTypeをファイル名に変換する.
//! <!--履歴-->.
// 2018/08/23 SCD2-Sky Kawanishi [EagleH][SecV4][RQ-676] OSS単独更新機能.
// ================================================================================================.
static const char* OSSType2FileName(TYPE_OSSType uc_OSSType)
{
	switch (uc_OSSType)
	{
		case TYPE_OSSType_OSL:
			return THRD_OSL_VerFileName;
		case TYPE_OSSType_CHR:
			return THRD_CHR_VerFileName;
		case TYPE_OSSType_NDJ:
			return THRD_NDJ_VerFileName;
		case TYPE_OSSType_UPL:
			return THRD_UPL_VerFileName;
		default:
			SYSF_logMsg("%s: uc_OSSType is wrong(%d).", __FUNCTION__, uc_OSSType);
			return "";
	}
}
// ================================================================================================.
//! <!--機能--> OSSバージョン情報取得.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数--> uc_OSSType	OSS種別.
//! 			psc_version	取得するバージョン文字列.
//!				si_size		バッファサイズ.
//! <!--戻値--> True:成功 False:失敗.
//! <!--詳細-->.
//! OSSのバージョンファイルから、バージョン情報を取得.
//! <!--履歴-->.
//  2018/08/23 SCD2-Sky Kawanishi [EagleH][SecV4][RQ-676] 新規作成.
// ================================================================================================.
Bool THRF_getOSSVersion(TYPE_OSSType uc_OSSType, char* psc_version, size_t si_size)
{
	int si_Fd = 0;
	char sc_OSSVersion[THRD_OSS_VersionSize];
	int32_t si_Ret = 0;
	char sc_VerFilePathName[PATH_MAX] = {0};
	
	if (NULL == psc_version)
	{
		SYSF_logMsg("%s: parameter NULL\n", __FUNCTION__);
		return False;
	}
	if (THRD_OSS_VersionSize > si_size)
	{
		SYSF_logMsg("%s: parameter size too short(%d)\n", __FUNCTION__, si_size);
		return False;
	}
	
	memset(sc_OSSVersion, 0x00, sizeof(sc_OSSVersion));

	// 各OSS用verの格納先取得.
	snprintf(sc_VerFilePathName, sizeof(sc_VerFilePathName), "/boot/%s", OSSType2FileName(uc_OSSType));
	
	// OSSバージョンファイルオープン.
	si_Fd = SYSF_open(sc_VerFilePathName, O_RDWR | O_SYNC, 0644);
	if (-1 == si_Fd)
	{
		SYSF_logMsg("%s: SYSF_open ERR %s errno:%d\n", __FUNCTION__, sc_VerFilePathName, SYSF_errnoGet());
		return False;
	}
	
	// OSSバージョン読み込み.
	si_Ret = SYSF_read(si_Fd, sc_OSSVersion, (THRD_OSS_VersionSize - 1));
	if (-1 == si_Ret)
	{
		SYSF_close(si_Fd);
		SYSF_logMsg("%s: read: ERR errno:%d\n", __FUNCTION__, SYSF_errnoGet());
		return False;
	}
	
	SYSF_close(si_Fd);

	strncpy(psc_version, sc_OSSVersion, THRD_OSS_VersionSize);
	
	return True;
}

//FUNCTION-------------------------------------------------------------------------------------------------------------------.
// \brief	機能：暗号化キー初期化変更通知.
// \par		備考：　D030リカバリー用に、暗号化キー初期化変更通知を取得.
// \param	なし.
// \result	なし.
// \par	変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//		2020/3/28 PDD1 日立 伊藤史.  新規作成.
//---------------------------------------------------------------------------------------------------------------------------.
void THRF_setKeyInfoEvent( void )
{
	THRC_Crypto *pc_Crypto = NULL;
	pc_Crypto = THRC_Crypto::newInstance();
	if( NULL == pc_Crypto ) {
		printf( "%s(%d)Error, failed THRC_Crypto::newInstance().\n", __FUNCTION__, __LINE__ );
		return;
	}
	pc_Crypto->setKeyInfoEvent();
}

//FUNCTION-------------------------------------------------------------------------------------------------------------------.
// \brief	概要：副電源ボタン割り込み許可.
// \par		備考：
// \param	なし.
// \result	なし.
// \par	変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//		2020/07/14 伊藤史. OP_BTS-21031 起動時副電源ボタン割込抑制.
//---------------------------------------------------------------------------------------------------------------------------.
void THRF_SubPowerEnable( void )
{
	THRL_SetThLog( 744, THRE_DebugLogType_ThCtrLog, 0, True, 0 );
	THRC_PowerManagerDeviceSingletonIf::newInstance().SwitchSubPowerSWState( THRC_PowerStateID_LOW );
	THRC_PowerManagerDeviceSingletonIf::newInstance().SwitchPowerState( THRC_PowerDeviceID_SET_INTP4, THRC_PowerStateID_LOW );
	printf( "(%010u)(BSP) THRF_SubPowerEnable.\n", SYSF_tickGet() );
}


//FUNCTION-------------------------------------------------------------------------------------------------------------------.
// \brief	概要：A800 メインメモリアクセスチェック.
// \par		備考：
// \param	なし.
// \result	チェック結果（True=チェックOK、False:チェックNG）.
// \par	変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//		2021/04/02 Sky OP_BTS-30755 トラブルコード[C-FCFE]対策.
//---------------------------------------------------------------------------------------------------------------------------.
#define THRD_MainMemCheckSize 64  //メモリチェックするサイズ[Bytes].
                                  // ※64バイトであれば処理時間は0.067 [ms]程度で、十分高速にチェックが可能.

Bool THRF_CheckMainMemoryByA800( void )
{
	THRL_SetThLog(753, THRE_DebugLogType_ThCtrLog, 0, False);
	if( False==THRF_IsA800IntegrationExist() )
	{
        // A800非装着時はチェックOKを返す.
		return True;
	}

	Bool result = True;
	switch( Drvrif_CheckMainMemoryByA800( THRD_MainMemCheckSize ) )
	{
	case 1:  // メモリチェックNG.
		result = False;
		break;

	case 0:  // メモリチェックOK.
	default: // チェックエラー発生（A800の再初期化はしない）.
		result = True;
		break;
	}

	THRL_SetThLog(754, THRE_DebugLogType_ThCtrLog, 0, False, result);
	return result;
}


//FUNCTION-------------------------------------------------------------------------------------------------------------------.
// \brief	概要：復旧治具による診断結果データを取得する。.
// \par		備考：
// \param	診断結果データが格納されるuint64_t変数への参照。.
// \result	-.
// \par	変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//		2021/05/27 Sky 復旧治具の診断結果データ収集機能対応.
//		2022/11/28 Sky 診断治具の書き込み位置移動による対応.
//---------------------------------------------------------------------------------------------------------------------------.
#define THRD_SPI_OFFSET_Recovery		(0x0059)

#define DEF_KM_WARP_SETTINGINFO_DISK        "/dev/kmsda"
#define DEF_KM_WARP_SETTINGINFO_ADR         65
#define DEF_KM_WARP_SI_INDEX_Recovery       374 // [in] 診断結果データ(ストレージ).
void THRF_getRecoveryCSRAData( uint64_t& u64_CSRAResultData )
{
	u64_CSRAResultData = 0;

	uint8_t *pDst = (uint8_t *)&u64_CSRAResultData;
	{
		// SPI-Flashから診断結果保存データを読み込む.
		(void)THRG_getBspThrRegion( THRD_SPI_OFFSET_Recovery, pDst, sizeof(uint32_t));
	}

	pDst += sizeof(uint32_t);
	{
		// ストレージから診断結果保存データを読み込む.
		Bool isError = False;
		int fdStorage = open( DEF_KM_WARP_SETTINGINFO_DISK, O_RDWR );
		if( fdStorage<0 )
		{
		    isError = True;
		}
		if( False==isError
		    && lseek(fdStorage, (DEF_KM_WARP_SETTINGINFO_ADR*512 + DEF_KM_WARP_SI_INDEX_Recovery), SEEK_SET)<0 )
		{
		    isError = True;
		}
		if( False==isError 
		    && read(fdStorage,  pDst, sizeof(uint32_t))<0 )
		{
		    isError = True;
		}
		close( fdStorage );
	}
}

// テストデータの設定用デバッグ関数.
extern "C" void thrDebugRecoveryCSRAData( uint32_t mode, uint32_t u32_ResultData )
{
	switch( mode )
	{
	case 1: // 設定値取得.
		{
			uint64_t u64_CSRAResultData=0;
			uint8_t *pByte = (uint8_t *)&u64_CSRAResultData;

			THRF_getRecoveryCSRAData( u64_CSRAResultData );
			printf("CSRAResultData = %p\n", u64_CSRAResultData);
			printf("  [%02X %02X %02X %02X %02X %02X %02X %02X]\n", pByte[0], pByte[1], pByte[2], pByte[3], pByte[4], pByte[5], pByte[6], pByte[7]);
		}
		break;
	case 2: // SPI-Flashにデータを保存.
		{
			uint8_t data[4] = {0};
			data[0] = (u32_ResultData>>24u)&0xFFu;
			data[1] = (u32_ResultData>>16u)&0xFFu;
			data[2] = (u32_ResultData>> 8u)&0xFFu;
			data[3] = (u32_ResultData>> 0u)&0xFFu;

			(void)THRG_setBspThrRegion( THRD_SPI_OFFSET_Recovery, data, sizeof(uint32_t));
		}
		break;
	case 3: // ストレージにデータを保存.
		{
			// ストレージから診断結果保存データを読み込む.
			uint8_t data[4] = {0};
			data[0] = (u32_ResultData>>24u)&0xFFu;
			data[1] = (u32_ResultData>>16u)&0xFFu;
			data[2] = (u32_ResultData>> 8u)&0xFFu;
			data[3] = (u32_ResultData>> 0u)&0xFFu;

			Bool isError = False;
			int fdStorage = open( DEF_KM_WARP_SETTINGINFO_DISK, O_RDWR );
			if( fdStorage<0 )
			{
			    isError = True;
			}
			if( False==isError
			    && lseek(fdStorage, (DEF_KM_WARP_SETTINGINFO_ADR*512 + DEF_KM_WARP_SI_INDEX_Recovery), SEEK_SET)<0 )
			{
			    isError = True;
			}
			if( False==isError 
			    && write(fdStorage,  data, sizeof(uint32_t))<0 )
			{
			    isError = True;
			}
			close( fdStorage );
		}
		break;

	default:
		printf("thrDebugRecoveryCSRAData MODE [VALUE]\n");
		printf("  MODE 1: Show saved data.\n");
		printf("       2: Set [VALUE] to data in the SPI-Flash.\n");
		printf("       3: Set [VALUE] to data in the storage(kmsda).\n");
		break;
	}
}


//FUNCTION-------------------------------------------------------------------------------------------------------------------.
// \brief	概要：メモリ使用量監視を有効/無効化する.
// \par		備考：
//! \param なし.
// \result	なし.
// \par	変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//		2022/05/18 Sky メモリ測定強化対応.
//---------------------------------------------------------------------------------------------------------------------------.
void THRF_setMeminfoLogDisable( char action )
{
	if ( action == 1 ){
		//メモリ使用量監視を無効化する為のフラグファイルを作成する.
		// 保存用ディレクトリが無い場合は作成する.
		if ( 0 != access(THRD_MEMINFOLOG_PATH, F_OK) ) {
			SYSF_mkdir(THRD_MEMINFOLOG_PATH);
		}
		system( THRD_MEMINFOLOG_DISABLE_ON );
		sync();
		sync();
		sync();
	}else{
		//メモリ使用量監視を無効化する為のフラグファイルを削除する.
		system( THRD_MEMINFOLOG_DISABLE_OFF );		
		sync();
		sync();
		sync();
	}
	return;

}

//FUNCTION-------------------------------------------------------------------------------------------------------------------.
// \brief	概要：メモリ使用量監視ログを保存する.
// \par		備考：
//! \param なし.
// \result	Bool True=成功 / False=失敗.
// \par	変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//		2022/05/18 Sky メモリ測定強化対応.
//---------------------------------------------------------------------------------------------------------------------------.
Bool THRF_createMeminfoLogFile( void )
{
	struct stat				st_Stat;
	Bool b_ret = False;
// 【暫定】DipSwNoが決定次第、ファイル有無判定からDipSw判定方式へ切り替える.
//	APIC_SoftDipSw* pc_SoftDipSw = APIC_SoftDipSw::newInstance();
//	if (( pc_SoftDipSw == NULL ) || (pc_SoftDipSw->getSoftDipSwBit( 0, 0 ) == False )) {
	if ( 0 == SYSF_Stat( THRD_MEMINFOLOG_DISABLE, &st_Stat)){ //ファイルが存在しなければメモリ使用量確認が有効.
		return False;	
	}
	
	// コピー先削除.
	system( THRD_DEL_MARKETUSAGEINFO_MEMLOG );

	// ログ生成.
	b_ret = THRL_createMeminfoLogFile(True);
	if (b_ret == False ){
		return False;
	}
	
	// コピー実施.
	system( THRD_MV_MEMINFOLOG );

	sync();
	sync();
	sync();
	
	return True;

}

// ================================================================================================.
//! <!--機能--> A800レスモデル判定.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return Bool True：A800Less機種  / False：A800Lessではない.
//! <!--詳細-->.
//!  なし\n.
//! <!--履歴-->.
//  2021/07/01	Sky		A800レス A800装着判定処理追加対応.
// ================================================================================================.
Bool THRF_IsA800LessModel( void )
{
	static Bool bl_ret = False;

#if defined(DEF_EagleCus) && !defined(DEF_A800) && !defined(DEF_HeliosMLKCus)
	static Bool bl_judge = False;

	// モデル未取得.
	if( False == bl_judge )
	{
		TYPE_MachineType e_MachineType = THRF_GetOriginalMachineTypeInfo();

		// A800レスのマシンタイプ確認.
		switch( e_MachineType )
		{
			case TYPE_MachineTypeEagleX:
			case TYPE_MachineTypeEagleBKX:
			case TYPE_MachineTypeEagleBKZ:
				bl_ret = True;
				break;

			default:
				break;
		}

		// 問い合わせは初回のみ.
		bl_judge = True;
		SYSF_logMsg("LessModel:%d, MachineType(%d)\n", bl_ret, e_MachineType);
	}
#endif // defined(DEF_EagleCus) && !defined(DEF_A800) && !defined(DEF_HeliosMLKCus).

	return bl_ret;
}

typedef struct {
	char 	sc_machine;		/* Machine	*/
	char 	sc_type;		/* Type		*/
} THRF_A800LLESS_MODEL;

/* 機種情報. */
const THRF_A800LLESS_MODEL	A800Less_machineTypeTbl[]	= {
	{ THRE_MACHINEINFO_EAGLE	,THRE_MACHINENO_X	},/* EagleX		*/
	{ THRE_MACHINEINFO_EAGLEBK	,THRE_MACHINENO_X	},/* EagleBKX	*/
	{ THRE_MACHINEINFO_EAGLEBK	,THRE_MACHINENO_Z	},/* EagleBKZ	*/
	{ THRE_MACHINEINFO_EAGLE3	,THRE_MACHINENO_X	},/* EagleIII X		*/
	{ THRE_MACHINEINFO_EAGLEBK3	,THRE_MACHINENO_X	},/* EagleIII BKX	*/
	{ THRE_MACHINEINFO_EAGLEBK3	,THRE_MACHINENO_Z	},/* EagleIII BKZ	*/
};

// ================================================================================================.
//! <!--機能--> A800レスモデル判定(SPI-Flashから取得).
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return Bool True：A800Less機種  / False：A800Lessではない.
//! <!--詳細-->.
//! THRF_GetOriginalMachineTypeInfo()からマシンタイプを取得すると再起動後でないと反映されない為、.
//! SPI-Flashから取得する関数を新たに作成する.
//! <!--履歴-->.
//  2022/12/20	Sky		A800レス A800装着判定処理追加対応(SPI-Flashから取得).
// ================================================================================================.
Bool THRF_IsA800LessModel_fromSPIFlash( void )
{
	Bool bl_ret = False;
#if defined(DEF_EagleCus) && !defined(DEF_A800) && !defined(DEF_HeliosMLKCus)
	// マシン情報取得開始
	char sc_machine = -1, sc_type = -1;
	THRF_getMachineType(&sc_machine, &sc_type);

	uint32_t ui32_searchCnt = 0;
	uint32_t ui32_machineTypeMax = sizeof(A800Less_machineTypeTbl) / sizeof(THRF_A800LLESS_MODEL);

	/* A800LESSテーブル検索 */
	for( ui32_searchCnt = 0;  ui32_searchCnt < ui32_machineTypeMax; ui32_searchCnt++ )
	{
		if( (A800Less_machineTypeTbl[ui32_searchCnt].sc_machine	== sc_machine	)&&
		    (A800Less_machineTypeTbl[ui32_searchCnt].sc_type	== sc_type		)
		){
			bl_ret = True;
			break;
		}
	}
	SYSF_logMsg("LessModel:%d, sc_machine:%d, sc_type:%d\n", bl_ret, sc_machine, sc_type);
#endif // defined(DEF_EagleCus) && !defined(DEF_A800) && !defined(DEF_HeliosMLKCus).
	return bl_ret;
}

//FUNCTION-------------------------------------------------------------------------------------------------------------------.
// \brief	概要：垂れ流しログをコピー（生産RINNEログ保存対応）.
// \par		備考：
// \param	char* psc_path コピー先のパス.
// \result	True:成功、False:失敗.
// \par	変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//		2022/09/27 生産RINNEログ保存対応.
//---------------------------------------------------------------------------------------------------------------------------.
extern Bool THRL_cpTermLog(char*);
Bool THRF_cpTermLog(char* psc_path)
{
	return THRL_cpTermLog(psc_path);
}

//FUNCTION-------------------------------------------------------------------------------------------------------------------.
// \brief	概要：パス削除要求（生産RINNEログ保存対応）.
// \par		備考：
// \param	char* psc_path 削除するパス.
// \result	True:成功、False:失敗.
// \par	変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//		2022/09/27 生産RINNEログ保存対応.
//---------------------------------------------------------------------------------------------------------------------------.
extern Bool THRL_rmLogPath(char*);
Bool THRF_rmLogPath(char* psc_path)
{
	return THRL_rmLogPath(psc_path);
}

// ================================================================================================.
//! <!--機能--> BOOT/本体FWの組み合わせエラー検出.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return Bool True：正常  / False：組み合わせエラー.
//! <!--詳細-->.
//!  なし\n.
//! <!--履歴-->.
//  2021/10/07	Sky		制御構成変更No.35_A800Less対応後の基板交換間違い検知.
// ================================================================================================.
void THRF_CheckFwCombinationBootAndMain( void )
{
// 実機、EagleL/ZX/BKL/BKZXのみ対象.
#if !defined(SIMULATOR) && defined(DEF_EagleCus) && !defined(DEF_EagleZpCus) && !defined(DEF_HeliosMLKCus) && !defined(DEF_MinervaSSBKCus)

	int		i = 0;
	char	c_fwdir[BSN_FWDIR_LENGTH+1] = "";

	memset(c_fwdir, 0x00, BSN_FWDIR_LENGTH+1);

	THRL_Drvrif_GetFwDir( c_fwdir );
	SYSF_logMsg("GetFwDir:[%s]\n", c_fwdir);

	// A800装着機種の確認.
	for( i=0; i < sizeof(FwDir)/sizeof(FwDir[0]); i++)
	{
		if( strncmp(FwDir[i], c_fwdir, strnlen(c_fwdir, BSN_FWDIR_LENGTH)) == 0 )
		{
			// 見つかれば即終了.
			return;
		}
	}

	// 見つからない場合、エラー表示.
	APIC_SystemStatus::newInstance()->setFatal( TYPD_TroubleCode_FATAL_BOOT_FW_ERR );

#else //if !defined(SIMULATOR) && defined(DEF_EagleCus) && !defined(DEF_EagleZpCus) && !defined(DEF_HeliosMLKCus) && !defined(DEF_MinervaSSBKCus).
	SYSF_logMsg("No FwDir check\n");
#endif //if !defined(SIMULATOR) && defined(DEF_EagleCus) && !defined(DEF_EagleZpCus) && !defined(DEF_HeliosMLKCus) && !defined(DEF_MinervaSSBKCus).

	return;
}
