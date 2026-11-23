/*======================================================================================= 

	［タイトル］	IPP 共有ライブラリの定義  

	［  説明  ］	IPP 共有ライブラリを定義する 

=======================================================================================*/ 

/*=======================================================================================
	インクルードファイル 
=======================================================================================*/
// ..System
#include	<stdlib.h>
#include	<string.h>
#include	<stdio.h>

// ..Common

// FWリファクタリング APIC_Setting.hの分割.
#include "system/nvd/APIC_Setting_PswcMibSetting.h"
#include "system/nvd/APIC_Setting_MachineSetting.h"
#include "system/nvd/APIC_Setting_CopyScanJobSetting.h"
#include "system/nvd/APIC_Setting_PrintJobSetting.h"
#include "system/nvd/APIC_Setting_FaxGfpSetting.h"
#include "system/nvd/APIC_Setting_SoftDipSw.h"
#include "system/nvd/APIC_Setting_FaxJobSetting.h"
#include "system/nvd/APIC_Setting_FaxListReportSetting.h"
#include "system/nvd/APIC_Setting_Network.h"
#include "system/nvd/APIC_Setting_SystemSetting.h"
#include "system/job/APIC_Log.h"

#include	"system/include/typs_cpp.h"
#include 	"system/job/APIC_JobList.h"
// UV1429 AirPrint-1.4 Support- HCL- VIJAY 2014/5/16 START
#include 	"system/nvd/APIC_FaxPanelSetting.h"
#include 	"system/sys/OsWrapper/SYSC_OSWrapper.h"
#include 	"system/sys/sysg/SYSF_MemoryAllocFree.h"
// UV1429 AirPrint-1.4 Support- HCL- VIJAY 2014/5/16 END

// 機種毎に、include先を切り替える必要がある.
// HelosL 制御No1 OSP1八代 2014.11.19 START.
#if ( defined DEF_Zeus5BkCus || defined DEF_HeliosCus )
// HelosL 制御No1 OSP1八代 2014.11.19 END.
#include 	"mib/mibColor3/MIBC_IFCommonColor3.h"
// ZeusSBK 制御No1 OPS1八代 2016.02.03 START.
//ZeusS ZX0 制御構成変更_No.2 2015/06/08 HCL-Vijay START 
#elif ( defined DEF_A4_Cus )
#include 	"mib/mibA4Color1/MIBC_IFA4CommonColor1.h"

#else
#include 	"mib/mibColor2/MIBC_IFCommonColor2.h"		// ZeusS, ZeusS ZX0, ZeusSBK X/0/1/2.
// ZeusSBK 制御No1 OPS1八代 2016.02.03 END.
//#else
//#include 	"mib/mibColor1/MIBC_IFCommonColor1.h"		// MinervaSBK.
//ZeusS ZX0 制御構成変更_No.2 2015/06/08 HCL-Vijay END
#endif

#include    "mib/NETG_FixedValueDefine.h"		// ZeusCD CQ386774 OPS1八代 2014/02/25 START.

// ..Local
// ZeusCD UV-01249 AppleAirPrint OPS1八代 2014/01/31 START.
#include	"IPPC_IFLibraryColor1.h"
#include	"ipp/ippg/IPPG_Define.h"
// ZeusCD UV-01249 AppleAirPrint OPS1八代 2014/01/31 END.
#include 	"IPPC_IFCyclicColor1.h"		   //MinervaSBK CQ520623 HCL-Sankar 2015/10/28 Addded.

/*=======================================================================================
	ファイル内変数定義
=======================================================================================*/
// ZeusCD UV-01249 AppleAirPrint OPS1八代 2014/01/31 START.
static APIC_MachineSetting* 		pc_MachineSetting   	= NULL;
static APIC_FeedTray* 				pc_FeedTray 	    	= NULL;
static APIC_PrintJobSetting*		pc_PrintJobSetting  	= NULL;
static APIC_SystemInfo* 			pc_SystemInfo 	    	= NULL;
static APIC_PrinterSystem*			pc_PrinterSystem    	= NULL;
static APIC_CopyJobSetting* 		pc_CopyJobSetting   	= NULL;
static APIC_PrinterWarning* 		pc_PrinterWarning   	= NULL;		// ZeusCD UV-01249 AppleAirPrint OPS1正部家 2014/01/31.
static IPPC_LibDebug* 				pc_LibDebug		    	= NULL;
static APIC_ActiveJobList*			pc_ActiveJobList    	= NULL;
static NETG_FixedValueDefine* 		pc_FixedValueDefine 	= NULL; 	// ZeusCD CQ386774 OPS1八代 2014/02/25.
static APIC_PswcMibSetting* 		pc_PswcMibSetting   	= NULL;		// ZeusCD CQ386774 OPS1八代 2014/02/25.
static APIC_OutputTray* 			pc_OutputTray 			= NULL;		// ZeusCD CQ386774 OPS1八代 2014/02/25.
static APIC_FaxPanelSetting* 		pc_FaxPanelSetting  	= NULL;		// UV1429 AppleAirPrint 1.4 Support HCL-VIJAY 2014/5/31.
static APIC_FaxJobSetting* 			pc_FaxJobSetting    	= NULL;
static APIC_SoftDipSw* 				pc_SoftDipSw 			= NULL;
static APIC_FaxListReportSetting*	pc_FaxListReportSetting = NULL;		// HeliosL KRQ-93 AirPrint1.5(WP14) 2015.02.13 OPS1八代.
static APIC_FaxGfpSetting* 			pc_FaxGfpSetting 		= NULL;		// HeliosL KRQ-93 AirPrint1.5(WP14) 2015.02.13 OPS1八代.
static APIC_IppSetting* 			pc_IppSetting 			= NULL;		// HeliosL KRQ-93 AirPrint1.5(WP14) 2015.02.13 OPS1八代.
static APIC_AirPrintSetting* 		pc_AirPrintSetting 		= NULL;		// HeliosL KRQ-93 AirPrint1.5(WP14) 2015.02.13 OPS1八代.
static APIC_SystemSetting* 			pc_SystemSetting 		= NULL;		// HeliosL KRQ-93 AirPrint1.5(WP14) 2015.02.13 OPS1八代.

// Poseidon UV-01429 AppleAirPrint1.4対応 Bairavi 2014/06/13 Start.
char g_scDialno[MAX_STRING64];
SEM_ID	g_SemID;
// Poseidon UV-01429 AppleAirPrint1.4対応 Bairavi 2014/06/13 End.

void _vCreateNewInstanceIPP_LibraryColor()
{
	pc_MachineSetting   	= APIC_MachineSetting::newInstance();
	pc_FeedTray 	    	= APIC_FeedTray::newInstance();
	pc_PrintJobSetting  	= APIC_PrintJobSetting::newInstance();
	pc_SystemInfo	    	= APIC_SystemInfo::newInstance();
	pc_PrinterSystem    	= APIC_PrinterSystem::newInstance();
	pc_CopyJobSetting   	= APIC_CopyJobSetting::newInstance();
	pc_PrinterWarning   	= APIC_PrinterWarning::newInstance();		// ZeusCD UV-01249 AppleAirPrint OPS1正部家 2014/01/31.
	pc_ActiveJobList    	= APIC_ActiveJobList::newInstance();
	pc_FixedValueDefine 	= NETG_FixedValueDefine::newInstance();		// ZeusCD CQ386774 OPS1八代 2014/02/25.
    pc_PswcMibSetting   	= APIC_PswcMibSetting::newInstance();		// ZeusCD CQ386774 OPS1八代 2014/02/25.
	pc_OutputTray 			= APIC_OutputTray::newInstance();			// ZeusCD CQ386774 OPS1八代 2014/02/25.
	pc_FaxPanelSetting 		= APIC_FaxPanelSetting::newInstance();		// UV-1429 AirPrint-1.4 Support-HCL-VIJAY
	pc_FaxJobSetting 		= APIC_FaxJobSetting::newInstance();
	pc_SoftDipSw 			= APIC_SoftDipSw::newInstance();
	pc_FaxListReportSetting = APIC_FaxListReportSetting::newInstance();	// HeliosL KRQ-93 AirPrint1.5(WP14) 2015.02.13 OPS1八代.
	pc_FaxGfpSetting 		= APIC_FaxGfpSetting::newInstance();		// HeliosL KRQ-93 AirPrint1.5(WP14) 2015.02.13 OPS1八代.
	pc_IppSetting 			= APIC_IppSetting::newInstance();			// HeliosL KRQ-93 AirPrint1.5(WP14) 2015.02.13 OPS1八代.
	pc_AirPrintSetting 		= APIC_AirPrintSetting::newInstance();		// HeliosL KRQ-93 AirPrint1.5(WP14) 2015.02.13 OPS1八代.
	pc_SystemSetting 		= APIC_SystemSetting::newInstance();		// HeliosL KRQ-93 AirPrint1.5(WP14) 2015.02.13 OPS1八代.

	pc_LibDebug		    = IPPC_LibDebug::newInstance();
	return;
}
/*================================================================================================
［機能］	Converting NICFUM ENUM(Supplied by client) -> Library ENUM
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2014/06/30 新規作成 UV-1429 Airprint-1.4 Character Code Conversion Support
================================================================================================*/
static struct IPPS_CnvCharset {
	IPP_ATTR_CHARSET	e_CharSet;		// NICFUM ENUM
	LIBE_CharSet 		e_LibCharSet;	// Library ENUM
	TYPE_CharSet		e_ComCharSet;
} st_CnvTable_CnvCharset  [] = {
	{ CHARSET_US_ASCII, 	 	LIBE_CharSetASCII, 		TYPE_CharSetASCII		},
	{ CHARSET_ISO_8859_1, 	 	LIBE_CharSetISO8859_1, 	TYPE_CharSetISO8859_1	},
	{ CHARSET_ISO_8859_2, 	 	LIBE_CharSetISO8859_2, 	TYPE_CharSetISO8859_2	},
	{ CHARSET_UTF_8, 	 		LIBE_CharSetUTF8,	 	TYPE_CharSetUTF8 		},
};

LIBE_CharSet getLibCharSet(IPP_ATTR_CHARSET e_CharSet)
{
	LIBE_CharSet e_Return = LIBE_CharSetUTF8;

	for( ushort i = 0; i < sizeof( st_CnvTable_CnvCharset ) / sizeof( IPPS_CnvCharset ); i++ ) {
		if( e_CharSet == st_CnvTable_CnvCharset[i].e_CharSet ) {
			e_Return = st_CnvTable_CnvCharset[i].e_LibCharSet;
			break;
		}
	}

	return e_Return;
}

TYPE_CharSet getComCharSet(IPP_ATTR_CHARSET e_CharSet)
{
	TYPE_CharSet e_Return = TYPE_CharSetUTF8;

	for( ushort i = 0; i < sizeof( st_CnvTable_CnvCharset ) / sizeof( IPPS_CnvCharset ); i++ ) {
		if( e_CharSet == st_CnvTable_CnvCharset[i].e_CharSet ) {
			e_Return = st_CnvTable_CnvCharset[i].e_ComCharSet;
			break;
		}
	}

	return e_Return;
}

/*================================================================================================
［機能］	設定関数 Character Code Conversion Support for Name/Text Attributes.
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2014/06/27 新規作成 UV1429 Air-Print-1.4 Character Code Conversion.Support 
================================================================================================*/
void IPP_ConvertFromUTF8( char* uc_InOut, int i_InOutBufferSize, IPP_ATTR_CHARSET e_FromStringCharSet )
{
	uchar *pc_Temp = NULL;
	LIBE_CharSet e_CharSet = getLibCharSet(e_FromStringCharSet);
	
	for ( ;; ) {
		pc_Temp = (uchar *)SYSF_Malloc(i_InOutBufferSize);
		if ( pc_Temp NE NULL ) {
			memset( pc_Temp, 0, i_InOutBufferSize );
			break;
		}
		int32_t sl_Tick = (int32_t)SYSF_sysClkRateGet() * 1000 / 1000;	// ｍｓ→ティック数に変換(1000ms)
		if( sl_Tick == 0L ) { sl_Tick = 1L; }
		++sl_Tick;
		SYSF_taskDelay( sl_Tick );									// 1000ms 遅延
	}
 	memcpy( pc_Temp, uc_InOut, i_InOutBufferSize );
	memset( uc_InOut, 0, i_InOutBufferSize );
 	LIBF_ConvertFromUTF8( (char *)pc_Temp , e_CharSet, (char*)uc_InOut , i_InOutBufferSize );
	SYSF_Free( pc_Temp );
 	return;
}
// Poseidon UV-01429 AppleAirPrint-1.4 Character Code Conversion 2014/06/27 HCL_VIJAY END.
/*================================================================================================
［機能］	設定関数 IPP_isFaxoutEnabled.
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2014/10/28 新規作成 CQ504484 Poseidon UV-1429
================================================================================================*/
Bool IPP_isFaxoutEnabled() 
{
	Bool b_FaxExists = False;
	TYPE_CommLine e_TypeCommLine[TYPD_CommLineMax] = {TYPE_CommLine1, TYPE_CommLine2, TYPE_CommLine3, TYPE_CommLine4};
	for(int ii=0; ii<TYPD_CommLineMax; ii++)
	{
		b_FaxExists = pc_SystemInfo->getFaxExistAll(e_TypeCommLine[ii]);
		if(True == b_FaxExists){
			break;	
		}
	}
	TYPE_PcFaxSendMode e_FaxSendMode = pc_FaxJobSetting->getPcFaxSendMode();
	if((True == b_FaxExists) && (e_FaxSendMode == TYPE_PcFaxSendModeEnable)){	
		return True;
    } else {
	    return False;
	}
}

// Helios KRQ-93 AirPrint1.5対応 HCL-Sankar 2015/02/02 ADDED START
/*================================================================================================
［機能］	To Get Airprint version that is enabled 
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
[履歴]     Helios KRQ-93 AirPrint1.5対応
================================================================================================*/
TYPE_AirPrintLicence IPP_GetAirPrintVersion()
{
	// Helios KRQ-101 AirPrint1.5対応 HCL-Bairavi 2015/03/02 Modified Start.
	TYPE_AirPrintLicence e_AirprintLicense = TYPE_AirPrintLicence_None;
	pc_SystemInfo->lock();
	e_AirprintLicense = pc_SystemInfo->getAirPrintLicenceStatus();
	pc_SystemInfo->unlock();
	// Helios KRQ-101 AirPrint1.5対応 HCL-Bairavi 2015/03/02 Modified End.
	return e_AirprintLicense;
}
// Helios KRQ-93 AirPrint1.5対応 HCL-Sankar 2015/02/02 ADDED END
// MinervaSBK 制御構成変更_No.71 HCL-Sankar 2015/09/10 ADDED START.
/*================================================================================================
［機能］	To Get Mopria version that is enabled 
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
[履歴] MinervaSBK 制御構成変更_No.71(WP1797)
================================================================================================*/
TYPE_MopriaLicence IPP_GetMopriaVersion()
{
	TYPE_MopriaLicence e_MopriaLicence = TYPE_MopriaLicence_None;
	pc_SystemInfo->lock();
	e_MopriaLicence = pc_SystemInfo->getMopriaLicenceStatus();
	pc_SystemInfo->unlock();
	return e_MopriaLicence;
}
// MinervaSBK 制御構成変更_No.71 HCL-Sankar 2015/09/10 ADDED END.

#define IPPD_Max_ActiveJobListAll			TYPD_MaxActiveJob
#define IPPD_Max_ActiveJobListPrint			TYPD_MaxActiveJob
// ZeusCD UV-01249 AppleAirPrint OPS1八代 2014/01/31 END.

/*================================================================================================
  IPP ライブラリ 関数群
================================================================================================*/

// ZeusCD UV-01249 AppleAirPrint OPS1八代 2014/01/31 START.
/*================================================================================================
［機能］	取得関数 media
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2014/01/31 新規作成 CQ384241 ZeusCD UV-01249 AppleAirPrint.
================================================================================================*/
const char* p_IPP_Media[] = {
	"iso_a3_297x420mm",						// IPPE_Media_A3.
	"iso_a4_210x297mm",						// IPPE_Media_A4.
	"iso_a5_148x210mm",						// IPPE_Media_A5.
	"iso_a6_105x148mm",						// IPPE_Media_A6.
	"jis_b4_257x364mm",						// IPPE_Media_B4.
	"jis_b5_182x257mm",						// IPPE_Media_B5.
	"jis_b6_128x182mm",						// IPPE_Media_B6.
	"na_ledger_11x17in",					// IPPE_Media_11x17.
	"na_legal_8.5x14in",					// IPPE_Media_8Hx14.
	"na_letter_8.5x11in",					// IPPE_Media_8Hx11.
	"na_executive_7.25x10.5in",				// IPPE_Media_7Qx10H.
	"na_invoice_5.5x8.5in",					// IPPE_Media_5Hx8H.
	"iso_sra3_320x450mm",					// IPPE_Media_SRA3.				// ZeusCD CQ386580 OPS1八代 2014/02/25.
	"na_govt-legal_8x13in",					// IPPE_Media_8x13.
	"oe_foolscap_8.125x13.25in",			// IPPE_Media_8125x13Q.			// ZeusCD CQ386580 OPS1八代 2014/02/25.
	"om_folio_210x330mm",					// IPPE_Media_8Qx13.			// ZeusCD CQ386580 OPS1八代 2014/02/25.
	"na_foolscap_8.5x13in",					// IPPE_Media_8Hx13.
	"oe_f4_8.5x13.5in",						// IPPE_Media_8Hx13H.			// ZeusCD CQ386580 OPS1八代 2014/02/25.
	"om_foolscap_220x330mm",				// IPPE_Media_220x330.			// ZeusCD CQ386580 OPS1八代 2014/02/25.
	"om_kai8_270x390mm",					// IPPE_Media_KAI8.				// ZeusCD CQ386580 OPS1八代 2014/02/25.
	"om_kai16_195x270mm",					// IPPE_Media_KAI16.			// ZeusCD CQ386580 OPS1八代 2014/02/25.
	"na_arch-b_12x18in",					// IPPE_Media_12x18.
	"jpn_hagaki_100x148mm",					// IPPE_Media_JPSHagaki.
	"na_index-4x6_4x6in",					// IPPE_Media_4x6.
	"om_a6card_105x148mm",					// IPPE_Media_A6Card.			// ZeusCD CQ386580 OPS1八代 2014/02/25.
	"na_monarch_3.875x7.5in",				// IPPE_Media_EnvelopeMonarch.
	"na_number-10_4.125x9.5in",				// IPPE_Media_EnvelopeCom10.
	"iso_dl_110x220mm",						// IPPE_Media_EnvelopeDL.
	"iso_c5_162x229mm",						// IPPE_Media_EnvelopeC5.
	"iso_b5_176x250mm",						// IPPE_Media_EnvelopeB5.
	"iso_c4_229x324mm",						// IPPE_Media_EnvelopeC4.
	"iso_c6_114x162mm",						// IPPE_Media_EnvelopeC6.
	"jpn_chou3_120x235mm",					// IPPE_Media_chou3.
	"jpn_chou4_90x205mm",					// IPPE_Media_chou4.
	"om_you3_98x148mm",						// IPPE_Media_you3.				// ZeusCD CQ386580 OPS1八代 2014/02/25.
	"jpn_you4_105x235mm",					// IPPE_Media_you4.
	"jpn_kaku1_270x382mm",					// IPPE_Media_kaku1.			// ZeusCD CQ386580 OPS1八代 2014/02/25.
	"jpn_kaku2_240x332mm",					// IPPE_Media_kaku2.
	"jpn_kaku3_216x277mm",					// IPPE_Media_kaku3.			// ZeusCD CQ386580 OPS1八代 2014/02/25.
	"custom_max_12.5x18in",					// IPPE_Media_custom_max.		// ZeusCD CQ386580 OPS1八代 2014/02/25.
	"custom_min_3.5x5.5in",					// IPPE_Media_custom_min.		// ZeusCD CQ386580 OPS1八代 2014/02/25.
#ifdef DEF_A4_Cus // VenusMLK先行開発 2OPS Yamaguchi.
	"oe_govt-letter_8x10.5in",				// IPPE_Media_8x10H.
	"jpn_oufuku_148x200mm",					// IPPE_Media_JPNOufuku.
	"na_letter-plus_8.5x12.69in",			// IPPE_Media_8Hx1269.
	"na_govt-letter_8x10in",				// IPPE_Media_8x10.
	"custom_max_215.9x355.6mm",				// IPPE_Media_custom_max.
	"custom_min_76.2x127mm",				// IPPE_Media_custom_min.
	"custom_min_92x148mm",					// IPPE_Media_custom_min.
#endif // VenusMLK先行開発 2OPS Yamaguchi.
	"",										// IPPE_Media_NONE.
};
const char* IPPC_LibMedia::Get_Media( IPPE_Media e_Media )
{
	if( e_Media < IPPE_Media_A3 || IPPE_Media_NONE < e_Media ) {
		// 不正な配列番号を参照しないよう、(ログを出力した上で)、配列番号の適用範囲内に丸め込む.
		// (このログが出力されるということは、呼び元の実装に誤りがある).
		DIPPTRACE1( "[IPP][fail safe] Get_Media = %d\n",e_Media );
		e_Media = IPPE_Media_NONE;
	}

	return p_IPP_Media[ e_Media ];
}

/*================================================================================================
［機能］	media-size
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2014/01/31 新規作成 CQ384241 ZeusCD UV-01249 AppleAirPrint.
================================================================================================*/
const IPPS_MediaSize st_MediaSize  [] = {
// ZeusCD CQ386774 OPS1八代 2014/02/25 START.
	{ IPP_MEDIASIZE_OPERATION_SINGLE, 	{ 29700, 		0 }, 	{ 42000,	 0 } 	},	// IPPE_Media_A3.
	{ IPP_MEDIASIZE_OPERATION_SINGLE, 	{ 21000, 		0 }, 	{ 29700,	 0 } 	},	// IPPE_Media_A4.
	{ IPP_MEDIASIZE_OPERATION_SINGLE, 	{ 14800, 		0 }, 	{ 21000,	 0 } 	},	// IPPE_Media_A5.
	{ IPP_MEDIASIZE_OPERATION_SINGLE, 	{ 10500,		0 }, 	{ 14800,	 0 } 	},	// IPPE_Media_A6.
	{ IPP_MEDIASIZE_OPERATION_SINGLE, 	{ 25700,		0 }, 	{ 36400,	 0 } 	},	// IPPE_Media_B4.
	{ IPP_MEDIASIZE_OPERATION_SINGLE, 	{ 18200,		0 }, 	{ 25700,	 0 } 	},	// IPPE_Media_B5.
	{ IPP_MEDIASIZE_OPERATION_SINGLE, 	{ 12800,		0 }, 	{ 18200,	 0 } 	},	// IPPE_Media_B6.
	{ IPP_MEDIASIZE_OPERATION_SINGLE, 	{ 27940,		0 }, 	{ 43180,	 0 } 	},	// IPPE_Media_11x17.
	{ IPP_MEDIASIZE_OPERATION_SINGLE, 	{ 21590,		0 }, 	{ 35560,	 0 } 	},	// IPPE_Media_8Hx14.
	{ IPP_MEDIASIZE_OPERATION_SINGLE, 	{ 21590,		0 }, 	{ 27940,	 0 } 	},	// IPPE_Media_8Hx11.
	{ IPP_MEDIASIZE_OPERATION_SINGLE, 	{ 18415,		0 }, 	{ 26670,	 0 } 	},	// IPPE_Media_7Qx10H.			// ZeusCD CQ386774 OPS1八代 2014/02/25.
	{ IPP_MEDIASIZE_OPERATION_SINGLE, 	{ 13970,		0 }, 	{ 21590,	 0 } 	},	// IPPE_Media_5Hx8H.
	{ IPP_MEDIASIZE_OPERATION_SINGLE, 	{ 32000,		0 }, 	{ 45000,	 0 } 	},	// IPPE_Media_SRA3.
	{ IPP_MEDIASIZE_OPERATION_SINGLE, 	{ 20320,		0 }, 	{ 33020,	 0 } 	},	// IPPE_Media_8x13.
	{ IPP_MEDIASIZE_OPERATION_SINGLE, 	{ 20637,		0 }, 	{ 33655,	 0 } 	},	// IPPE_Media_8125x13Q.
	{ IPP_MEDIASIZE_OPERATION_SINGLE, 	{ 21000,		0 }, 	{ 33000,	 0 } 	},	// IPPE_Media_8Qx13.
	{ IPP_MEDIASIZE_OPERATION_SINGLE, 	{ 21590,		0 }, 	{ 33020,	 0 } 	},	// IPPE_Media_8Hx13.
	{ IPP_MEDIASIZE_OPERATION_SINGLE, 	{ 21590,		0 }, 	{ 34290,	 0 } 	},	// IPPE_Media_8Hx13H.
	{ IPP_MEDIASIZE_OPERATION_SINGLE, 	{ 22000,		0 }, 	{ 33000,	 0 } 	},	// IPPE_Media_220x330.
	{ IPP_MEDIASIZE_OPERATION_SINGLE, 	{ 27000,		0 }, 	{ 39000,	 0 } 	},	// IPPE_Media_KAI8.
	{ IPP_MEDIASIZE_OPERATION_SINGLE, 	{ 19500,		0 }, 	{ 27000,	 0 } 	},	// IPPE_Media_KAI16.
	{ IPP_MEDIASIZE_OPERATION_SINGLE, 	{ 30480,		0 }, 	{ 45720,	 0 } 	},	// IPPE_Media_12x18.
	{ IPP_MEDIASIZE_OPERATION_SINGLE, 	{ 10000,		0 }, 	{ 14800,	 0 } 	},	// IPPE_Media_JPSHagaki.
	{ IPP_MEDIASIZE_OPERATION_SINGLE, 	{ 10160,		0 }, 	{ 15240,	 0 } 	},	// IPPE_Media_4x6.
	{ IPP_MEDIASIZE_OPERATION_SINGLE, 	{ 10500,		0 }, 	{ 14800,	 0 } 	},	// IPPE_Media_A6Card.
	{ IPP_MEDIASIZE_OPERATION_SINGLE, 	{  9842,		0 }, 	{ 19050,	 0 } 	},	// IPPE_Media_EnvelopeMonarch.
	{ IPP_MEDIASIZE_OPERATION_SINGLE, 	{ 10477,		0 }, 	{ 24130,	 0 } 	},	// IPPE_Media_EnvelopeCom10.
	{ IPP_MEDIASIZE_OPERATION_SINGLE, 	{ 11000,		0 }, 	{ 22000,	 0 } 	},	// IPPE_Media_EnvelopeDL.		// ZeusCD CQ386774 OPS1八代 2014/02/25.
	{ IPP_MEDIASIZE_OPERATION_SINGLE, 	{ 16200,		0 }, 	{ 22900,	 0 } 	},	// IPPE_Media_EnvelopeC5.
	{ IPP_MEDIASIZE_OPERATION_SINGLE, 	{ 17600,		0 }, 	{ 25000,	 0 } 	},	// IPPE_Media_EnvelopeB5.
	{ IPP_MEDIASIZE_OPERATION_SINGLE, 	{ 22900,		0 }, 	{ 32400,	 0 } 	},	// IPPE_Media_EnvelopeC4.
	{ IPP_MEDIASIZE_OPERATION_SINGLE, 	{ 11400,		0 }, 	{ 16200,	 0 } 	},	// IPPE_Media_EnvelopeC6.
	{ IPP_MEDIASIZE_OPERATION_SINGLE, 	{ 12000,		0 }, 	{ 23500,	 0 } 	},	// IPPE_Media_chou3.
	{ IPP_MEDIASIZE_OPERATION_SINGLE, 	{  9000,		0 }, 	{ 20500,	 0 } 	},	// IPPE_Media_chou4.
	{ IPP_MEDIASIZE_OPERATION_SINGLE, 	{  9800,		0 }, 	{ 14800,	 0 } 	},	// IPPE_Media_you3.
	{ IPP_MEDIASIZE_OPERATION_SINGLE, 	{ 10500,		0 }, 	{ 23500,	 0 } 	},	// IPPE_Media_you4.
	{ IPP_MEDIASIZE_OPERATION_SINGLE, 	{ 27000,		0 }, 	{ 38200,	 0 } 	},	// IPPE_Media_kaku1.
	{ IPP_MEDIASIZE_OPERATION_SINGLE, 	{ 24000,		0 }, 	{ 33200,	 0 } 	},	// IPPE_Media_kaku2.
	{ IPP_MEDIASIZE_OPERATION_SINGLE, 	{ 21600,		0 }, 	{ 27700,	 0 } 	},	// IPPE_Media_kaku3.
	{ IPP_MEDIASIZE_OPERATION_RANGE,  	{  8890,	31750 }, 	{ 13970, 45720 } 	},	// IPPE_Media_custom_max.		// ZeusCD CQ386580 OPS1八代 2014/02/25.
	{ IPP_MEDIASIZE_OPERATION_NONE,  	{     0,		0 }, 	{ 	  0,	 0 } 	},	// IPPE_Media_custom_min.		// ZeusCD CQ386580 OPS1八代 2014/02/25.
#ifdef DEF_A4_Cus // VenusMLK先行開発 2OPS Yamaguchi.
	{ IPP_MEDIASIZE_OPERATION_SINGLE, 	{ 20320,		0 }, 	{ 26670,	 0 } 	},	// IPPE_Media_8x10H.
	{ IPP_MEDIASIZE_OPERATION_SINGLE, 	{ 14800,		0 }, 	{ 20000,	 0 } 	},	// IPPE_Media_JPNOufuku.
	{ IPP_MEDIASIZE_OPERATION_SINGLE, 	{ 21590,		0 }, 	{ 32232,	 0 } 	},	// IPPE_Media_8Hx1269.
	{ IPP_MEDIASIZE_OPERATION_SINGLE, 	{ 20320,		0 }, 	{ 25400,	 0 } 	},	// IPPE_Media_8x10.
	{ IPP_MEDIASIZE_OPERATION_RANGE,  	{  7620,	21590 }, 	{ 12700, 35560 } 	},	// IPPE_Media_A4custom_max.
	{ IPP_MEDIASIZE_OPERATION_RANGE,  	{  9200,	21590 }, 	{ 14800, 35560 } 	},	// IPPE_Media_A4custom_max.		// ZeusCD CQ386580 OPS1八代 2014/02/25.
	{ IPP_MEDIASIZE_OPERATION_NONE,  	{     0,		0 }, 	{ 	  0,	 0 } 	},	// IPPE_Media_A4custom_min.		// ZeusCD CQ386580 OPS1八代 2014/02/25.
#endif // VenusMLK先行開発 2OPS Yamaguchi.
	{ IPP_MEDIASIZE_OPERATION_NONE,   	{     0,		0 }, 	{  	  0,	 0 } 	},	// IPPE_Media_NONE.
// ZeusCD CQ386774 OPS1八代 2014/02/25 END.
};

IPPS_MediaSize IPPC_LibMedia::Get_MediaSize( IPPE_Media e_Media )
{
	IPPS_MediaSize st_Rtn_MediaSize;
	memset( &st_Rtn_MediaSize, NULL, sizeof(IPPS_MediaSize) );

	if( e_Media < IPPE_Media_A3 || IPPE_Media_NONE < e_Media ) {
		// 不正な配列番号を参照しないよう、(ログを出力した上で)、配列番号の適用範囲内に丸め込む.
		// (このログが出力されるということは、呼び元の実装に誤りがある).
		DIPPTRACE1( "[IPP][fail safe] Get_MediaSize = %d\n",e_Media );
		e_Media = IPPE_Media_NONE;
	}

	st_Rtn_MediaSize.set         = st_MediaSize[e_Media].set;
	// ZeusCD CQ386774 OPS1八代 2014/02/25 START.
	st_Rtn_MediaSize.X_Dimension[0] = st_MediaSize[e_Media].X_Dimension[0];
	st_Rtn_MediaSize.Y_Dimension[0] = st_MediaSize[e_Media].Y_Dimension[0];
	st_Rtn_MediaSize.X_Dimension[1] = st_MediaSize[e_Media].X_Dimension[1];
	st_Rtn_MediaSize.Y_Dimension[1] = st_MediaSize[e_Media].Y_Dimension[1];
	// ZeusCD CQ386774 OPS1八代 2014/02/25 END.

	return st_Rtn_MediaSize;
}

/*================================================================================================
［機能］	取得関数 media-source
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2014/01/31 新規作成 CQ384241 ZeusCD UV-01249 AppleAirPrint.
================================================================================================*/
const char* p_IPP_MediaSource[] = {
	"auto",					// IPPE_MediaSource_Auto.
	"tray-1",				// IPPE_MediaSource_Tray1.
	"tray-2",				// IPPE_MediaSource_Tray2.
	"tray-3",				// IPPE_MediaSource_Tray3.
	"tray-4",				// IPPE_MediaSource_Tray4.
	"by-pass-tray",			// IPPE_MediaSource_ByPass.
	"large-capacity",		// IPPE_MediaSource_LCT.
	"tray-5",				//IPPE_MediaSource_Tray5					//2018/05/31 DenebMLK 機器構成変更_No.7 HCL-VIJAY Added.
	"",						// IPPE_MediaSource_NONE.
};
const char* IPPC_LibMedia::Get_MediaSource( IPPE_MediaSource e_MediaSource )
{
	if( e_MediaSource < IPPE_MediaSource_Auto || IPPE_MediaSource_NONE < e_MediaSource ) {
		// 不正な配列番号を参照しないよう、(ログを出力した上で)、配列番号の適用範囲内に丸め込む.
		// (このログが出力されるということは、呼び元の実装に誤りがある).
		DIPPTRACE1( "[IPP][fail safe] Get_MediaSource = %d\n",e_MediaSource );
		e_MediaSource = IPPE_MediaSource_NONE;
	}

	return p_IPP_MediaSource[ e_MediaSource ];
}

/*================================================================================================
［機能］	media-type
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2014/01/31 新規作成 CQ384241 ZeusCD UV-01249 AppleAirPrint.
================================================================================================*/
const char* p_IPP_MediaType[] = {
	"auto",																// IPPE_MediaType_Auto.
	"stationery",														// IPPE_MediaType_Stat.
	"com.konicaminolta.stationery-secondside",							// IPPE_MediaType_StatSec.
	"stationery-heavyweight",											// IPPE_MediaType_Thick1.
	"com.konicaminolta.stationery-heavyweight-secondside",				// IPPE_MediaType_Thick1Sec.
	"com.konicaminolta.expanded-stationery-heavyweight1",				// IPPE_MediaType_ExpThick1.
	"com.konicaminolta.expanded-stationery-heavyweight1-secondside",	// IPPE_MediaType_ExpThick1Sec.
	"com.konicaminolta.stationery-heavyweight2",						// IPPE_MediaType_Thick2.
	"com.konicaminolta.stationery-heavyweight2-secondside",				// IPPE_MediaType_Thick2Sec.
	"com.konicaminolta.stationery-heavyweight3",						// IPPE_MediaType_Thick3.
	"com.konicaminolta.stationery-heavyweight3-secondside",				// IPPE_MediaType_Thick3Sec.
	"com.konicaminolta.stationery-heavyweight4",						// IPPE_MediaType_Thick4.
	"com.konicaminolta.stationery-heavyweight4-secondside",				// IPPE_MediaType_Thick4Sec.
	"stationery-lightweight",											// IPPE_MediaType_Thin1.
	"com.konicaminolta.stationery-lightweight-secondside",				// IPPE_MediaType_Thin1Sec.
	"envelope",															// IPPE_MediaType_Env.
	"transparency",														// IPPE_MediaType_ThinOhp.
	"stationery-colored",												// IPPE_MediaType_ColorP.
	"tab-stock",														// IPPE_MediaType_IndexP.
	"stationery-letterhead",											// IPPE_MediaType_LetterH.
	"com.konicaminolta.special-paper",									// IPPE_MediaType_SpecialP.
	"com.konicaminolta.printed-paper",									// IPPE_MediaType_PrintedP.
	"com.konicaminolta.userdefine1",									// IPPE_MediaType_UserDef1.
	"com.konicaminolta.userdefine1-secondside",							// IPPE_MediaType_UserDef1Sec.
	"com.konicaminolta.userdefine2",									// IPPE_MediaType_UserDef2.
	"com.konicaminolta.userdefine2-secondside",							// IPPE_MediaType_UserDef2Sec.
	"com.konicaminolta.userdefine3",									// IPPE_MediaType_UserDef3.
	"com.konicaminolta.userdefine3-secondside",							// IPPE_MediaType_UserDef3Sec.
	"com.konicaminolta.userdefine4",									// IPPE_MediaType_UserDef4.
	"com.konicaminolta.userdefine4-secondside",							// IPPE_MediaType_UserDef4Sec.
	"com.konicaminolta.userdefine5",									// IPPE_MediaType_UserDef5.
	"com.konicaminolta.userdefine5-secondside",							// IPPE_MediaType_UserDef5Sec.
	"com.konicaminolta.userdefine6",									// IPPE_MediaType_UserDef6.
	"com.konicaminolta.userdefine6-secondside",							// IPPE_MediaType_UserDef6Sec.
	"com.konicaminolta.custom-paper1",									// IPPE_MediaType_Cus1.
	"com.konicaminolta.custom-paper2",									// IPPE_MediaType_Cus2.
	"com.konicaminolta.custom-paper3"			,						// IPPE_MediaType_Cus3.
	"com.konicaminolta.custom-paper4",									// IPPE_MediaType_Cus4.
	"com.konicaminolta.custom-paper5",									// IPPE_MediaType_Cus5.
	"com.konicaminolta.custom-paper6",									// IPPE_MediaType_Cus6.
	"com.konicaminolta.custom-paper7",									// IPPE_MediaType_Cus7.
	"com.konicaminolta.custom-paper8",									// IPPE_MediaType_Cus8.
	"com.konicaminolta.custom-paper9",									// IPPE_MediaType_Cus9.
	"com.konicaminolta.custom-paper10",									// IPPE_MediaType_Cus10.
	"com.konicaminolta.custom-paper11",									// IPPE_MediaType_Cus11.
	"com.konicaminolta.custom-paper12",									// IPPE_MediaType_Cus12.
	"com.konicaminolta.custom-paper13",									// IPPE_MediaType_Cus13.
	"com.konicaminolta.custom-paper14",									// IPPE_MediaType_Cus14.
	"com.konicaminolta.custom-paper15",									// IPPE_MediaType_Cus15.
	"com.konicaminolta.custom-paper16",									// IPPE_MediaType_Cus16.
	"com.konicaminolta.custom-paper17",									// IPPE_MediaType_Cus17.
	"com.konicaminolta.custom-paper18",									// IPPE_MediaType_Cus18.
	"com.konicaminolta.custom-paper19",									// IPPE_MediaType_Cus19.
	"com.konicaminolta.custom-paper20",									// IPPE_MediaType_Cus20.
	"com.konicaminolta.recycled-paper",									// IPPE_MediaType_Recycled.
	"com.konicaminolta.recycled-paper-secondside",						// IPPE_MediaType_RecycledSec.
#ifdef DEF_A4_Cus // VenusMLK先行開発 2OPS Yamaguchi.
	"labels",															// IPPE_MediaType_Label.
	"com.konicaminolta.postcard",										// IPPE_MediaType_Postcard.
	"photographic-semi-gloss",											// IPPE_MediaType_PhotoSemGloss.
	"photographic-glossy",												// IPPE_MediaType_PhotoGloss.
	"photographic-high-gloss",											// IPPE_MediaType_PhotoHighGloss.
#endif // VenusMLK先行開発 2OPS Yamaguchi.
	"com.konicaminolta.userdefine7",									// IPPE_MediaType_UserDef7.		// EagleZ+ 機器構成変更No.7 コート紙対応(Step2).
	"com.konicaminolta.userdefine7-secondside",							// IPPE_MediaType_UserDef7Sec.
	"",																	// IPPE_MediaType_NONE.

};
const char* IPPC_LibMedia::Get_MediaType( IPPE_MediaType e_MediaType )
{
	if( e_MediaType < IPPE_MediaType_Auto || IPPE_MediaType_NONE < e_MediaType ) {
		// 不正な配列番号を参照しないよう、(ログを出力した上で)、配列番号の適用範囲内に丸め込む.
		// (このログが出力されるということは、呼び元の実装に誤りがある).
		DIPPTRACE1( "[IPP][fail safe] Get_MediaType = %d\n",e_MediaType );
		e_MediaType = IPPE_MediaType_NONE;
	}

	return p_IPP_MediaType[ e_MediaType ];
}


/*================================================================================================
［機能］	仕様変換関数(KM⇒IPP)  media.
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2014/01/31 新規作成 CQ384241 ZeusCD UV-01249 AppleAirPrint.
================================================================================================*/
static struct IPPS_CnvTable_Media {
	TYPE_SizeCode 	e_SizeCode;		// KM定義.
	IPPE_Media		e_Media;		// IPP定義 - media.
} st_CnvTable_Media [] = {
	{ TYPE_SizeCodeUnknown, 	 		IPPE_Media_NONE 			},
	{ TYPE_SizeCodeA3Sef, 		 		IPPE_Media_A3 		  	 	},
	{ TYPE_SizeCodeA4Sef, 		 		IPPE_Media_A4 		  	 	},
	{ TYPE_SizeCodeA4Lef, 		 		IPPE_Media_A4 		  	 	},
	{ TYPE_SizeCodeA5Sef, 		 		IPPE_Media_A5 		  	 	},
	{ TYPE_SizeCodeA5Lef, 		 		IPPE_Media_A5 		  	 	},
	{ TYPE_SizeCodeA6Sef, 		 		IPPE_Media_A6 		  	 	},
	{ TYPE_SizeCodeA6Lef, 		 		IPPE_Media_A6 		  	 	},
	{ TYPE_SizeCodeB4Sef, 		 		IPPE_Media_B4 		  	 	},
	{ TYPE_SizeCodeB5Sef, 		 		IPPE_Media_B5 		  	 	},
	{ TYPE_SizeCodeB5Lef, 		 		IPPE_Media_B5 		  	 	},
	{ TYPE_SizeCodeB6Sef, 		 		IPPE_Media_B6 		  	 	},
	{ TYPE_SizeCodeB6Lef, 		 		IPPE_Media_B6 		  	 	},
	{ TYPE_SizeCodeLedgerSef, 	 		IPPE_Media_11x17 	  	 	},
	{ TYPE_SizeCode11x15Sef, 	 		IPPE_Media_NONE 			},
	{ TYPE_SizeCode11x14Sef, 	 		IPPE_Media_NONE 			},
	{ TYPE_SizeCodeLegalSef, 	 		IPPE_Media_8Hx14 	  	 	},
	{ TYPE_SizeCodeLetterSef, 	 		IPPE_Media_8Hx11 	  	 	},
	{ TYPE_SizeCodeLetterLef, 	 		IPPE_Media_8Hx11 	  	 	},	// ZeusCD CQ389751 OPS1八代 2014/04/22 .
	{ TYPE_SizeCodeExecutiveSef, 		IPPE_Media_7Qx10H 	  	 	},
	{ TYPE_SizeCodeExecutiveLef, 		IPPE_Media_7Qx10H 	  	 	},
	{ TYPE_SizeCodeInvoiceSef, 	 		IPPE_Media_5Hx8H 	  	 	},
	{ TYPE_SizeCodeInvoiceLef, 	 		IPPE_Media_5Hx8H 	  	 	},
	{ TYPE_SizeCodePostcard_JpSef, 		IPPE_Media_JPSHagaki  	 	},
	{ TYPE_SizeCodePostcard_JpLef, 		IPPE_Media_JPSHagaki  	 	},
	{ TYPE_SizeCodePostcard_EuSef, 		IPPE_Media_A6Card 			},
	{ TYPE_SizeCodePostcard_EuLef, 		IPPE_Media_A6Card 			},
	{ TYPE_SizeCodeFlsSef, 				IPPE_Media_NONE 			},	// FoolScapの実サイズはサービス設定のチョイスに従う.
	{ TYPE_SizeCode8KSef, 				IPPE_Media_KAI8 	  	 	},
	{ TYPE_SizeCode16KSef, 				IPPE_Media_KAI16 	  	 	},
	{ TYPE_SizeCode16KLef, 				IPPE_Media_KAI16 	  	 	},
	{ TYPE_SizeCodeA0x2Sef, 			IPPE_Media_NONE 			},
	{ TYPE_SizeCodeA0Sef, 				IPPE_Media_NONE 			},
	{ TYPE_SizeCodeA1Sef, 				IPPE_Media_NONE 			},
	{ TYPE_SizeCodeA2Sef, 				IPPE_Media_NONE 			},
	{ TYPE_SizeCodeB0Sef, 				IPPE_Media_NONE 			},
	{ TYPE_SizeCodeB1Sef, 				IPPE_Media_NONE 			},
	{ TYPE_SizeCodeB2Sef, 				IPPE_Media_NONE 			},
	{ TYPE_SizeCodeB3Sef, 				IPPE_Media_NONE 			},
	{ TYPE_SizeCode48x64Sef, 			IPPE_Media_NONE 			},
	{ TYPE_SizeCode36x48Sef, 			IPPE_Media_NONE 			},
	{ TYPE_SizeCode24x36Sef, 			IPPE_Media_NONE 			},
	{ TYPE_SizeCode18x24Sef, 			IPPE_Media_NONE 			},
	{ TYPE_SizeCode44x68Sef, 			IPPE_Media_NONE 			},
	{ TYPE_SizeCode34x44Sef, 			IPPE_Media_NONE 			},
	{ TYPE_SizeCode22x34Sef, 			IPPE_Media_NONE 			},
	{ TYPE_SizeCode17x22Sef, 			IPPE_Media_NONE 			},
	{ TYPE_SizeCodePhotoESef, 			IPPE_Media_NONE 			},
	{ TYPE_SizeCodePhotoELef, 			IPPE_Media_NONE 			},
	{ TYPE_SizeCodePhotoLSef, 			IPPE_Media_NONE 			},
	{ TYPE_SizeCodeA0x2Sef, 			IPPE_Media_NONE 			},
	{ TYPE_SizeCodePhotoLLef, 			IPPE_Media_NONE 			},
	{ TYPE_SizeCode9x13Sef, 			IPPE_Media_NONE 			},
	{ TYPE_SizeCode9x13Lef, 			IPPE_Media_NONE 			},
	{ TYPE_SizeCode10x15Sef, 			IPPE_Media_NONE 			},
	{ TYPE_SizeCode10x15Lef, 			IPPE_Media_NONE 			},
	{ TYPE_SizeCode13x18Sef, 			IPPE_Media_NONE 			},
	{ TYPE_SizeCode13x18Lef, 			IPPE_Media_NONE 			},
	{ TYPE_SizeCodeIN4x6Sef, 			IPPE_Media_4x6 				},
	{ TYPE_SizeCodeIN4x6Lef, 			IPPE_Media_4x6 				},
	{ TYPE_SizeCodeIN3x5Sef, 			IPPE_Media_NONE 			},
	{ TYPE_SizeCodeIN3x5Lef, 			IPPE_Media_NONE 			},
	{ TYPE_SizeCodeIN225x325Sef, 		IPPE_Media_NONE 			},
	{ TYPE_SizeCodeIN225x325Lef, 		IPPE_Media_NONE 			},
	{ TYPE_SizeCodeNonStandard, 		IPPE_Media_NONE 			},
	{ TYPE_SizeCode12x18Sef, 			IPPE_Media_12x18 	  	 	},	// ZeusCD CQ386422 OPS1八代 2014/02/14.
	{ TYPE_SizeCode10x14Sef, 			IPPE_Media_NONE 			},
#ifdef DEF_A4_Cus  // VenusMLK先行開発 OPS2 Yamaguchi.
	{ TYPE_SizeCodeGovermentLetterSef, 	IPPE_Media_8x10H 			},
	{ TYPE_SizeCode8x10Sef, 			IPPE_Media_8x10 			},
#else
	{ TYPE_SizeCodeGovermentLetterSef, 	IPPE_Media_NONE 			},
	{ TYPE_SizeCode8x10Sef, 			IPPE_Media_NONE 			},
#endif
	{ TYPE_SizeCodeJpLetterL3, 			IPPE_Media_chou3 	  	 	},
	{ TYPE_SizeCodeJpLetterK2, 			IPPE_Media_kaku2 	  	 	},
	{ TYPE_SizeCodeJpLetterY1, 			IPPE_Media_NONE 			},
	{ TYPE_SizeCodeJpLetterY2, 			IPPE_Media_EnvelopeC6 		},
	{ TYPE_SizeCodeJpLetterY4, 			IPPE_Media_you4 	  	 	},
	{ TYPE_SizeCodeJpLetterY6, 			IPPE_Media_NONE 			},
	{ TYPE_SizeCodeEnvelopeMornarch, 	IPPE_Media_EnvelopeMonarch 	},
	{ TYPE_SizeCodeEnvelopeCom10, 		IPPE_Media_EnvelopeCom10 	},
	{ TYPE_SizeCodeEnvelopeDL, 			IPPE_Media_EnvelopeDL 		},
	{ TYPE_SizeCodeEnvelopeC5, 			IPPE_Media_EnvelopeC5 		},
	{ TYPE_SizeCodeEnvelopeB5, 			IPPE_Media_EnvelopeB5 		},
	{ TYPE_SizeCodeLongPaper, 			IPPE_Media_NONE 			},
	{ TYPE_SizeCodeSRA3Sef, 			IPPE_Media_SRA3 			},
	{ TYPE_SizeCodeEnvelopeC4, 			IPPE_Media_EnvelopeC4 		},
	{ TYPE_SizeCodeJpLetterL4, 			IPPE_Media_chou4 			},
	{ TYPE_SizeCodeJpLetterY3, 			IPPE_Media_you3 			},
	{ TYPE_SizeCodeJpLetterK1, 			IPPE_Media_kaku1 			},
	{ TYPE_SizeCodeJpLetterK3, 			IPPE_Media_kaku3 			},
	{ TYPE_SizeCodeDontCare, 			IPPE_Media_NONE 			},
#ifdef DEF_A4_Cus // VenusMLK先行開発 2OPS Yamaguchi.
	{ TYPE_SizeCodeDoublePostcard_JpSef,	IPPE_Media_JPNOufuku 		},
	{ TYPE_SizeCodeSpFolioSef, 				IPPE_Media_8Hx1269 			},
	{ TYPE_SizeCodeJpLetterY2Lef, 			IPPE_Media_EnvelopeC6		},
	{ TYPE_SizeCodeEnvelopeDLLef, 			IPPE_Media_EnvelopeDL		},
	{ TYPE_SizeCodeEnvelopeMornarchLef, 	IPPE_Media_EnvelopeMonarch	},
	{ TYPE_SizeCodeJpLetterY3Lef, 			IPPE_Media_you3				},
#endif // VenusMLK先行開発 2OPS Yamaguchi.
	{ TYPE_SizeCodeMax, 				IPPE_Media_NONE 			},	// 終端.
};

IPPE_Media IPPC_LibMedia::CnvSpec_Media( TYPE_SizeCode e_SizeCode )
{
	IPPE_Media e_Return = IPPE_Media_NONE;

	for( ushort i = 0; i < sizeof( st_CnvTable_Media ) / sizeof( IPPS_CnvTable_Media ); i++ ) {

		if( e_SizeCode == st_CnvTable_Media[i].e_SizeCode ) {

			// FoolScapの実サイズはサービス設定のチョイスに従う.
			if( e_SizeCode == TYPE_SizeCodeFlsSef ) {
				TYPE_FLSType e_FLSType = TYPE_FLSType8x13;

				pc_MachineSetting->lock();
				e_FLSType = pc_MachineSetting->getFLSType();
				pc_MachineSetting->unlock();

				switch( e_FLSType ){
					case TYPE_FLSType220x330:	e_Return = IPPE_Media_220x330;	break;
					case TYPE_FLSType85x135:	e_Return = IPPE_Media_8Hx13H;	break;
					case TYPE_FLSType85x13:		e_Return = IPPE_Media_8Hx13;	break;
					case TYPE_FLSType825x13:	e_Return = IPPE_Media_8Qx13;	break;
					case TYPE_FLSType8125x1325:	e_Return = IPPE_Media_8125x13Q;	break;
					case TYPE_FLSType8x13:		e_Return = IPPE_Media_8x13;		break;
					default:					e_Return = IPPE_Media_NONE;		break;
				}

			} else {
				e_Return = st_CnvTable_Media[i].e_Media;
			}

			break;
		}
	}

	return e_Return;
}

/*================================================================================================
［機能］	仕様変換関数(KM⇒IPP) media-source.
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2014/01/31 新規作成 CQ384241 ZeusCD UV-01249 AppleAirPrint.
================================================================================================*/
static struct IPPS_CnvTable_MediaSource {
	TYPE_FeedTray 		e_FeedTray;		// KM定義.
	IPPE_MediaSource	e_MediaSource;	// IPP定義 - media-source.
} st_CnvTable_MediaSource  [] = {
	{ TYPE_FeedTrayManual, 	 	IPPE_MediaSource_ByPass },	// 手差しトレイ.
	{ TYPE_FeedTray1, 	 		IPPE_MediaSource_Tray1 	},	// トレイ1.
	{ TYPE_FeedTray2, 	 		IPPE_MediaSource_Tray2 	},	// トレイ2.
	{ TYPE_FeedTray3, 	 		IPPE_MediaSource_Tray3 	},	// トレイ3.
	{ TYPE_FeedTray4, 	 		IPPE_MediaSource_Tray4 	},	// トレイ4.
//2018/05/31 DenebMLK 機器構成変更_No.7 HCL-VIJAY Added Start.
#ifdef DEF_DenebMLKCus
	{ TYPE_FeedTray5, 	 		IPPE_MediaSource_Tray5 	},	// トレイ5.
#else
	{ TYPE_FeedTray5, 	 		IPPE_MediaSource_LCT 	},	// トレイ5(LCT).
#endif
//2018/05/31 DenebMLK 機器構成変更_No.7 HCL-VIJAY Added End.
	{ TYPE_FeedTrayPI1, 	 	IPPE_MediaSource_NONE 	},	// PIトレイ1.
	{ TYPE_FeedTrayPI2, 	 	IPPE_MediaSource_NONE 	},	// PIトレイ2.
	{ TYPE_FeedTrayUnselect, 	IPPE_MediaSource_NONE 	},	// 未選択.
	{ TYPE_FeedTrayMax, 		IPPE_MediaSource_NONE 	},	// 終端.
};

IPPE_MediaSource IPPC_LibMedia::CnvSpec_MediaSource( TYPE_FeedTray e_FeedTray )
{
	IPPE_MediaSource e_Return = IPPE_MediaSource_NONE;

	for( ushort i = 0; i < sizeof( st_CnvTable_MediaSource ) / sizeof( IPPS_CnvTable_MediaSource ); i++ ) {
		if( e_FeedTray == st_CnvTable_MediaSource[i].e_FeedTray ) {
			e_Return = st_CnvTable_MediaSource[i].e_MediaSource;
			break;
		}
	}

	return e_Return;
}

/*================================================================================================
［機能］	仕様変換関数(KM⇒IPP) media-type.
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2014/01/31 新規作成 CQ384241 ZeusCD UV-01249 AppleAirPrint.
================================================================================================*/
static struct IPPS_CnvTable_MediaType {
	TYPE_PaperType  e_PaperType;	// KM定義.
	IPPE_MediaType	e_MediaType;	// IPP定義 - media-type.
} st_CnvTable_MediaType  [] = {
	{ TYPE_PaperTypeNormal, 		 		IPPE_MediaType_Stat 	 	},
	{ TYPE_PaperRecycle1, 			 		IPPE_MediaType_Recycled 	},
	{ TYPE_PaperRecycle2, 			 		IPPE_MediaType_RecycledSec 	},
	{ TYPE_PaperHiQualty1, 			 		IPPE_MediaType_NONE 	 	},
	{ TYPE_PaperHiQualty2, 			 		IPPE_MediaType_NONE 	 	},
#ifdef DEF_A4_Cus
	{ TYPE_PaperPostCard, 			 		IPPE_MediaType_Postcard 	},
#else
	{ TYPE_PaperPostCard, 			 		IPPE_MediaType_NONE 	 	},
#endif
	{ TYPE_PaperTypeEnvelope, 		 		IPPE_MediaType_Env 		 	},
	{ TYPE_PaperTypeOHP, 			 		IPPE_MediaType_ThinOhp 	 	},
#ifdef DEF_A4_Cus
	{ TYPE_PaperTypeLabel, 			 		IPPE_MediaType_Label 	 	},
#else
	{ TYPE_PaperTypeLabel, 			 		IPPE_MediaType_NONE 	 	},
#endif
	{ TYPE_PaperType2ndOriginal, 	 		IPPE_MediaType_NONE 	 	},
	{ TYPE_PaperTypeUser1, 			 		IPPE_MediaType_UserDef1  	},
	{ TYPE_PaperTypeUser2, 			 		IPPE_MediaType_UserDef2  	},
	{ TYPE_PaperTypeUser3, 			 		IPPE_MediaType_UserDef3  	},
	{ TYPE_PaperTypeUser4, 			 		IPPE_MediaType_UserDef4  	},
	{ TYPE_PaperTypeUser5, 			 		IPPE_MediaType_UserDef5  	},
	{ TYPE_PaperTypeUser6, 			 		IPPE_MediaType_UserDef6  	},
	{ TYPE_PaperTypeIndex, 			 		IPPE_MediaType_IndexP 	 	},
	{ TYPE_PaperTypeLetterHead, 	 		IPPE_MediaType_LetterH 	 	},
	{ TYPE_PaperTypeSpecial1, 		 		IPPE_MediaType_SpecialP  	},
	{ TYPE_PaperTypeNotDuplex1, 	 		IPPE_MediaType_PrintedP  	},
	{ TYPE_PaperTypeThin1, 			 		IPPE_MediaType_Thin1 	 	},
	{ TYPE_PaperTypeThick1, 		 		IPPE_MediaType_Thick1 	 	},
	{ TYPE_PaperTypeThick2, 		 		IPPE_MediaType_Thick2 	 	},
	{ TYPE_PaperTypeThick3, 		 		IPPE_MediaType_Thick3 	 	},
	{ TYPE_PaperTypeColor, 			 		IPPE_MediaType_ColorP 	 	},
	{ TYPE_PaperTypeGeneral, 		 		IPPE_MediaType_NONE 	 	},
	{ TYPE_PaperTypeDontcare, 		 		IPPE_MediaType_Stat 	 	},	//temporary solution to fix MCT4.1 test
	{ TYPE_PaperTypeThick1Plus, 	 		IPPE_MediaType_ExpThick1 	},
	{ TYPE_PaperTypeThick4, 		 		IPPE_MediaType_Thick4 	 	},
	{ TYPE_PaperTypeNormal_2nd, 	 		IPPE_MediaType_StatSec 	 	},
	{ TYPE_PaperTypeThick1_2nd, 	 		IPPE_MediaType_Thick1Sec 	},
	{ TYPE_PaperTypeThick1Plus_2nd,  		IPPE_MediaType_ExpThick1Sec },
	{ TYPE_PaperTypeThick2_2nd, 	 		IPPE_MediaType_Thick2Sec 	},
	{ TYPE_PaperTypeThick3_2nd, 	 		IPPE_MediaType_Thick3Sec 	},
	{ TYPE_PaperTypeThick4_2nd, 	 		IPPE_MediaType_Thick4Sec 	},
	{ TYPE_PaperTypeUser1_2nd, 		 		IPPE_MediaType_UserDef1Sec 	},
	{ TYPE_PaperTypeUser2_2nd, 		 		IPPE_MediaType_UserDef2Sec 	},
	{ TYPE_PaperTypeUser3_2nd, 		 		IPPE_MediaType_UserDef3Sec 	},
	{ TYPE_PaperTypeUser4_2nd, 		 		IPPE_MediaType_UserDef4Sec 	},
	{ TYPE_PaperTypeUser5_2nd, 		 		IPPE_MediaType_UserDef5Sec 	},
	{ TYPE_PaperTypeUser6_2nd, 		 		IPPE_MediaType_UserDef6Sec 	},
	{ TYPE_PaperTypeUser7, 			 		IPPE_MediaType_UserDef7		},	// EagleZ+ 機器構成変更No.7 コート紙対応(Step2).
	{ TYPE_PaperTypeUser7_2nd, 		 		IPPE_MediaType_UserDef7Sec	},
	{ TYPE_PaperTypeCustom1, 		 		IPPE_MediaType_Cus1 		},
	{ TYPE_PaperTypeCustom2, 		 		IPPE_MediaType_Cus2 		},
	{ TYPE_PaperTypeCustom3, 		 		IPPE_MediaType_Cus3 		},
	{ TYPE_PaperTypeCustom4, 		 		IPPE_MediaType_Cus4 		},
	{ TYPE_PaperTypeCustom5, 		 		IPPE_MediaType_Cus5 		},
	{ TYPE_PaperTypeCustom6, 		 		IPPE_MediaType_Cus6 		},
	{ TYPE_PaperTypeCustom7, 		 		IPPE_MediaType_Cus7 		},
	{ TYPE_PaperTypeCustom8, 		 		IPPE_MediaType_Cus8 		},
	{ TYPE_PaperTypeCustom9, 		 		IPPE_MediaType_Cus9 		},
	{ TYPE_PaperTypeCustom10, 		 		IPPE_MediaType_Cus10 		},
	{ TYPE_PaperTypeCustom11, 		 		IPPE_MediaType_Cus11 		},
	{ TYPE_PaperTypeCustom12, 		 		IPPE_MediaType_Cus12 		},
	{ TYPE_PaperTypeCustom13, 		 		IPPE_MediaType_Cus13 		},
	{ TYPE_PaperTypeCustom14, 		 		IPPE_MediaType_Cus14 		},
	{ TYPE_PaperTypeCustom15, 		 		IPPE_MediaType_Cus15 		},
	{ TYPE_PaperTypeCustom16, 		 		IPPE_MediaType_Cus16 		},
	{ TYPE_PaperTypeCustom17, 		 		IPPE_MediaType_Cus17 		},
	{ TYPE_PaperTypeCustom18, 		 		IPPE_MediaType_Cus18 		},
	{ TYPE_PaperTypeCustom19, 		 		IPPE_MediaType_Cus19 		},
	{ TYPE_PaperTypeCustom20, 		 		IPPE_MediaType_Cus20 		},
	{ TYPE_PaperTypeCustom1_2nd, 	 		IPPE_MediaType_NONE 		},
	{ TYPE_PaperTypeCustom2_2nd, 	 		IPPE_MediaType_NONE 		},
	{ TYPE_PaperTypeCustom3_2nd, 	 		IPPE_MediaType_NONE 		},
	{ TYPE_PaperTypeCustom4_2nd, 	 		IPPE_MediaType_NONE 		},
	{ TYPE_PaperTypeCustom5_2nd, 	 		IPPE_MediaType_NONE 		},
	{ TYPE_PaperTypeCustom6_2nd, 	 		IPPE_MediaType_NONE 		},
	{ TYPE_PaperTypeCustom7_2nd, 	 		IPPE_MediaType_NONE 		},
	{ TYPE_PaperTypeCustom8_2nd, 	 		IPPE_MediaType_NONE 		},
	{ TYPE_PaperTypeCustom9_2nd, 	 		IPPE_MediaType_NONE 		},
	{ TYPE_PaperTypeCustom10_2nd, 	 		IPPE_MediaType_NONE 		},
	{ TYPE_PaperTypeCustom11_2nd, 	 		IPPE_MediaType_NONE 		},
	{ TYPE_PaperTypeCustom12_2nd, 	 		IPPE_MediaType_NONE 		},
	{ TYPE_PaperTypeCustom13_2nd, 	 		IPPE_MediaType_NONE 		},
	{ TYPE_PaperTypeCustom14_2nd, 	 		IPPE_MediaType_NONE 		},
	{ TYPE_PaperTypeCustom15_2nd, 	 		IPPE_MediaType_NONE 		},
	{ TYPE_PaperTypeCustom16_2nd, 	 		IPPE_MediaType_NONE 		},
	{ TYPE_PaperTypeCustom17_2nd, 	 		IPPE_MediaType_NONE 		},
	{ TYPE_PaperTypeCustom18_2nd, 	 		IPPE_MediaType_NONE 		},
	{ TYPE_PaperTypeCustom19_2nd, 	 		IPPE_MediaType_NONE 		},
	{ TYPE_PaperTypeCustom20_2nd, 	 		IPPE_MediaType_NONE 		},
	{ TYPE_PaperTypeThin_2nd, 		 		IPPE_MediaType_Thin1Sec 	},
	{ TYPE_PaperTypeMax, 			 		IPPE_MediaType_NONE 		},
	{ TYPE_PaperTypeUnknown, 		 		IPPE_MediaType_Auto 	 	},
#ifdef DEF_A4_Cus // VenusMLK先行開発 2OPS Yamaguchi.
	{ TYPE_PaperTypeGloss1, 				IPPE_MediaType_PhotoSemGloss	},
	{ TYPE_PaperTypeGloss1Plus, 			IPPE_MediaType_PhotoGloss 		},
	{ TYPE_PaperTypeGloss2, 				IPPE_MediaType_PhotoHighGloss 	},
#endif // VenusMLK先行開発 2OPS Yamaguchi.
};

IPPE_MediaType IPPC_LibMedia::CnvSpec_MediaType( TYPE_PaperType e_PaperType )
{
	IPPE_MediaType e_Return = IPPE_MediaType_NONE;

	for( ushort i = 0; i < sizeof( st_CnvTable_MediaType ) / sizeof( IPPS_CnvTable_MediaType ); i++ ) {
		if( e_PaperType == st_CnvTable_MediaType[i].e_PaperType ) {
			e_Return = st_CnvTable_MediaType[i].e_MediaType;
			break;
		}
	}

	return e_Return;
}

/*================================================================================================
［機能］	判定関数 media-ready.
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2014/01/31 新規作成 CQ384241 ZeusCD UV-01249 AppleAirPrint.
================================================================================================*/
IPPE_Media IPPC_LibMedia::Judge_Media_Ready( TYPE_FeedTray e_FeedTray )
{
	IPPE_Media     e_Media     = IPPE_Media_NONE;

	// media-readyの対象であるかを判断する.
	Bool           bl_Target   = True;

	pc_FeedTray->lock();

	// 給紙トレイが非装着のときは、media-readyの対象から外す.
	if( TYPE_TrayStatusNon == pc_FeedTray->getTrayStatus( e_FeedTray ) ) {
		bl_Target = False;
	}

	// PE(ペーパーエンプティ)のときは、media-readyの対象から外す.
	if( ( TYPE_PaperStatusEmpty == pc_FeedTray->getPaperStatus( e_FeedTray ) )
	    && ( False == pc_FeedTray->getPaperUnKnownInfo( e_FeedTray ) ) ) {
		bl_Target = False;
	}

	// IPP印刷として、非サポートの給紙トレイのときは、media-readyの対象から外す.
	if( IPPE_MediaSource_NONE == CnvSpec_MediaSource( e_FeedTray ) ) {
		bl_Target = False;
	}

	// media-readyの対象の場合、その給紙トレイに入っている用紙サイズから、mediaを判別する.
	if( True == bl_Target ) {
		e_Media = CnvSpec_Media( pc_FeedTray->getPaperSize( e_FeedTray ).e_SizeCode );
	}

	pc_FeedTray->unlock();

	return e_Media;
}

/*================================================================================================
［機能］	判定関数 media-default.
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2014/01/31 新規作成 CQ384241 ZeusCD UV-01249 AppleAirPrint.
================================================================================================*/
IPPE_Media IPPC_LibMedia::Judge_Media_Default()
{
	IPPE_Media	e_Media = IPPE_Media_NONE;

	pc_PrintJobSetting->lock();
	TYPC_PaperSize c_PaperSize = pc_PrintJobSetting->getPaperSize();
	pc_PrintJobSetting->unlock();

	e_Media = CnvSpec_Media( c_PaperSize.e_SizeCode );

	return e_Media;
}

/*================================================================================================
［機能］	判定関数 media-type-default.
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2014/01/31 新規作成 CQ384241 ZeusCD UV-01249 AppleAirPrint.
================================================================================================*/
IPPE_MediaType IPPC_LibMedia::Judge_MediaType_Default()
{
	IPPE_MediaType	e_MediaType = IPPE_MediaType_NONE;

	pc_PrintJobSetting->lock();
	TYPE_PaperType e_PaperType = pc_PrintJobSetting->get_PaperType();
	pc_PrintJobSetting->unlock();
	e_MediaType = CnvSpec_MediaType( e_PaperType );
	return e_MediaType;
}

/*================================================================================================
［機能］	判定関数 media-source-default.
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2014/01/31 新規作成 CQ384241 ZeusCD UV-01249 AppleAirPrint.
================================================================================================*/
IPPE_MediaSource IPPC_LibMedia::Judge_MediaSource_Default()
{
	IPPE_MediaSource	e_MediaSource = IPPE_MediaSource_NONE;

	if( True == pc_PrintJobSetting->getAutoFeedTray() ) {
		e_MediaSource = IPPE_MediaSource_Auto;
	} else {
		pc_PrintJobSetting->lock();
		TYPE_FeedTray e_FeedTray = pc_PrintJobSetting->getFeedTray();
		pc_PrintJobSetting->unlock();

		e_MediaSource = CnvSpec_MediaSource( e_FeedTray );
	}

	return e_MediaSource;
}

/*================================================================================================
［機能］	判定関数 media-source-feed-direction.
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2014/01/31 新規作成 CQ384241 ZeusCD UV-01249 AppleAirPrint.
================================================================================================*/
IPP_FEED_ORIENTATION IPPC_LibMedia::Judge_MediaSourceFeedDirection( TYPC_PaperSize c_PaperSize )
{
	IPP_FEED_ORIENTATION e_FEED_ORIENTATION = FEED_ORIENTATION_END;

	if( True == c_PaperSize.isTate() ) {
		e_FEED_ORIENTATION = SHORT_EDGE_FIRST;	// SEF.
	} else {
		e_FEED_ORIENTATION = LONG_EDGE_FIRST;	// LEF;
	}

	return e_FEED_ORIENTATION;
}

/*================================================================================================
［機能］	算出関数 media-size.
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2014/01/31 新規作成 CQ384241 ZeusCD UV-01249 AppleAirPrint.
================================================================================================*/
IPPS_MediaSize IPPC_LibMedia::Calc_MediaSize( const char* puc_Media )
{
	IPPS_MediaSize st_MediaSize;
	memset( &st_MediaSize, NULL, sizeof(IPPS_MediaSize) );

	for( IPPE_Media e_Media = 0; e_Media < IPPE_Media_NONE; e_Media++ ) {
		if( strcmp( Get_Media( e_Media ), puc_Media) == 0 ) {
			st_MediaSize = Get_MediaSize( e_Media );
			break;
		}
	}

	return st_MediaSize;
}

/*================================================================================================
［機能］	判定関数 FS 装着状態.
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2014/01/31 新規作成 CQ384241 ZeusCD UV-01249 AppleAirPrint.
            2019/02/27 FPT-LucNH2 IT6_2.0 [EAGLEH] 機器構成変更_No.29 add comment for JS-508.
================================================================================================*/
IPPE_AttachFS IPPC_LibOpMachine::Judge_AttachFSType()
{
	IPPE_AttachFS e_AttachFS = IPPE_AttachFS_None;

    pc_PrinterSystem->lock();
	Bool bl_ExistFS 				 = pc_PrinterSystem->isExistFinisher();
	TYPE_FinisherType e_FinisherType = pc_PrinterSystem->getFinisherType();
    pc_PrinterSystem->unlock();

	if( True == bl_ExistFS ) {
		switch( e_FinisherType ) {
			case TYPE_FinisherTypeJS506:	e_AttachFS = IPPE_AttachFS_JS602;	break;	// JS-506 or JS-508.
			case TYPE_FinisherTypeFS533:	e_AttachFS = IPPE_AttachFS_FS533;	break;
			case TYPE_FinisherTypeFS534:	e_AttachFS = IPPE_AttachFS_FS534;	break;
			case TYPE_FinisherTypeFS535:	e_AttachFS = IPPE_AttachFS_FS535;	break;
#ifdef DEF_A4_Cus // VenusMLK先行開発 2OPS Yamaguchi.
			case TYPE_FinisherTypeVenusFNS:	e_AttachFS = IPPE_AttachFS_VenusFNS;	break;
#endif // VenusMLK先行開発 2OPS Yamaguchi.
			case TYPE_FinisherTypeFS536:	e_AttachFS = IPPE_AttachFS_FS536;	break;	// MinervaSBK 制御No14_FS-536 OPS1八代 2015.03.26.
			case TYPE_FinisherTypeFS537:	e_AttachFS = IPPE_AttachFS_FS537;	break;	// MinervaSBK 制御No15_FS-537 OPS1八代 2015.04.06.
#ifdef DEF_KRONOSS_Cus // KronosS フィニッシャ対応 START.
			case TYPE_FinisherTypeKronosS:	e_AttachFS = IPPE_AttachFS_FSP02;	break;
#endif // KronosS フィニッシャ対応 END.
// 2018/10/17 FPT-LucNH2 【IT6_2.0】【EagleH】 機器構成変更_No.5 Start.
			case TYPE_FinisherTypeFS539:	e_AttachFS = IPPE_AttachFS_FS539;	break;
			case TYPE_FinisherTypeFS540:	e_AttachFS = IPPE_AttachFS_FS540;	break;
// 2018/10/17 FPT-LucNH2 【IT6_2.0】【EagleH】 機器構成変更_No.5 End.
			case TYPE_FinisherTypeFS542:	e_AttachFS = IPPE_AttachFS_FS542;	break;	// 2022/06/27 FPT-LucNH2 【IT6_2.3】 制御構成変更_No.2 Add.
			default: 						e_AttachFS = IPPE_AttachFS_None;	break;
		}
	}

	return e_AttachFS;
}

/*================================================================================================
［機能］	判定関数 LCT 装着状態.
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2014/01/31 新規作成 CQ384241 ZeusCD UV-01249 AppleAirPrint.
================================================================================================*/
IPPE_AttachLCT IPPC_LibOpMachine::Judge_AttachLCTType()
{
	IPPE_AttachLCT e_AttachLCT = IPPE_AttachLCT_None;

    pc_FeedTray->lock();
    TYPE_TrayStatus e_TrayStatus = pc_FeedTray->getTrayStatus( TYPE_FeedTray5 );	// 給紙トレイ状態.
	TYPE_TrayType 	e_TrayType 	 = pc_FeedTray->getTrayType( TYPE_FeedTray5 );		// 給紙トレータイプ.
    pc_FeedTray->unlock();

    pc_SystemInfo->lock();
    TYPE_LctKind e_LctKind = pc_SystemInfo->getLctKind();							// LCT種別取得 
    pc_SystemInfo->unlock();

	if( ( TYPE_TrayStatusNon != e_TrayStatus ) && ( TYPE_TrayTypeLCT == e_TrayType ) ) {
		if( TYPE_LctKind_Normal ==  e_LctKind ) {
			e_AttachLCT = 	IPPE_AttachLCT_LU302;	// LU302(A4 LCT).				//2018/05/31 DenebMLK 制御構成変更_No.1 HCL-VIJAY Modified.
		} else {
			e_AttachLCT = 	IPPE_AttachLCT_LU207;	// LU207(A3 LCT).				//2018/05/31 DenebMLK 制御構成変更_No.1 HCL-VIJAY Modified.
		}
	} else {
		e_AttachLCT = 	IPPE_AttachLCT_None;		// LCT非装着.
	}
	return e_AttachLCT;
}

// ZeusCD UV-01249 AppleAirPrint OPS1正部家 2014/01/31 START.
/*================================================================================================
［機能］	判定関数 SD 装着状態.
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2014/01/31 新規作成 ZeusCD UV-01249 AppleAirPrint.
================================================================================================*/
IPPE_AttachSD IPPC_LibOpMachine::Judge_AttachSDType()
{
	IPPE_AttachSD e_SDType = IPPE_AttachSD_None;
	IPPE_AttachFS e_FSType = Judge_AttachFSType();

	pc_PrinterSystem->lock();

	switch( e_FSType )
	{
		case IPPE_AttachFS_FS534:
		case IPPE_AttachFS_FS536:	// MinervaSBK 制御No14_FS-536 OPS1八代 2015.03.26.
		case IPPE_AttachFS_FS539:	//FS-539. // 2018/10/17 FPT-LucNH2 【IT6_EagleH】【EagleH】 機器構成変更_No.5 Add.
			if( pc_PrinterSystem->isExistSaddleUnitOption() )
			{
				e_SDType = IPPE_AttachSD_SD511;
			}
			break;
		case IPPE_AttachFS_FS535:
		case IPPE_AttachFS_FS537:	// MinervaSBK 制御No15_FS-537 OPS1八代 2015.04.06.
		case IPPE_AttachFS_FS540:	//FS-540. // 2018/10/17 FPT-LucNH2 【IT6_EagleH】【EagleH】 機器構成変更_No.5 Add.
			if( pc_PrinterSystem->isExistSaddleUnitOption() )
			{
				e_SDType = IPPE_AttachSD_SD512;
			}
			break;
		default:
			e_SDType = IPPE_AttachSD_None;
			break;
	}

	pc_PrinterSystem->unlock();

	return e_SDType;
}

/*================================================================================================
［機能］	判定関数 PK 装着状態.
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2014/01/31 新規作成 ZeusCD UV-01249 AppleAirPrint.
================================================================================================*/
IPPE_AttachPK IPPC_LibOpMachine::Judge_AttachPKType()
{
	IPPE_AttachPK e_PKType = IPPE_AttachPK_None;

	pc_MachineSetting->lock();
	TYPE_PunchKitType e_PunchKit = pc_MachineSetting->getPunchKitType();
	pc_MachineSetting->unlock();

	pc_PrinterSystem->lock();
	TYPS_PunchFunction s_PunchFunction = pc_PrinterSystem->getPunchFunction();		// パンチ機能.
	pc_PrinterSystem->unlock();

	switch( e_PunchKit ) {
		case TYPE_PunchKitType_PK_519:
			e_PKType = IPPE_AttachPK_PK519;
			break;
		case TYPE_PunchKitType_PK_520:
			e_PKType = IPPE_AttachPK_PK520;
			break;
		case TYPE_PunchKitType_PK_521:
			e_PKType = IPPE_AttachPK_PK521;
			break;
		//MinervaSBK 制御構成変更_No.1-2 パンチキット名称変更 HCL-Sankar 2015/08/28 Added START.
		case TYPE_PunchKitType_PK_523:
			e_PKType = IPPE_AttachPK_PK523;
			break;
		//MinervaSBK 制御構成変更_No.1-2 パンチキット名称変更 HCL-Sankar 2015/08/28 Added END.
//2021/09/13 [MinervaSSBK][OP_BTS-35461] HIENG Start.
		case TYPE_PunchKitType_PK_524:
			e_PKType = IPPE_AttachPK_PK524;
			break;
		case TYPE_PunchKitType_PK_526:
			e_PKType = IPPE_AttachPK_PK526;
			break;
//2021/09/13 [MinervaSSBK][OP_BTS-35461] HIENG End.
// 2022/06/27 FPT-LucNH2 【IT6_2.3】 制御構成変更_No.2 Add Start.
		case TYPE_PunchKitType_PK_527:
			e_PKType = IPPE_AttachPK_PK527;
			break;
// 2022/06/27 FPT-LucNH2 【IT6_2.3】 制御構成変更_No.2 Add End.
		default:
			e_PKType = IPPE_AttachPK_None;
			break;
	}

	// 念のため、XX穴も確認しておく.
	if( ( False == s_PunchFunction.b_Punch2Hole )
     && ( False == s_PunchFunction.b_Punch3Hole )
     && ( False == s_PunchFunction.b_Punch4Hole )
     && ( False == s_PunchFunction.b_Punch4HoleSWE ) ) {
		e_PKType = IPPE_AttachPK_None;
	}

	return e_PKType;
}
// ZeusCD UV-01249 AppleAirPrint OPS1正部家 2014/01/31 END.

#ifdef DEF_A4_Cus // CQ521331 VenusMLK 2OPS Yamaguchi START.
/*================================================================================================
［機能］	判定関数 Tray2 装着状態.
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2015/11/4 新規作成 CQ521331 VenusMLK AppleAirPrint.
================================================================================================*/
// 見直した方がいい.
Bool IPPC_LibOpMachine::isAttachTray2()
{
	Bool bl_Attach = False;

    pc_FeedTray->lock();
	if( TYPE_TrayStatusNon != pc_FeedTray->getTrayStatus( TYPE_FeedTray2 ) ) {
		bl_Attach = True;
	}
    pc_FeedTray->unlock();

	return bl_Attach;
}
#endif // CQ521331 VenusMLK 2OPS Yamaguchi 20151104 END.

/*================================================================================================
［機能］	判定関数 Tray3 装着状態.
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2014/01/31 新規作成 CQ384241 ZeusCD UV-01249 AppleAirPrint.
================================================================================================*/
// 見直した方がいい.
Bool IPPC_LibOpMachine::isAttachTray3()
{
	Bool bl_Attach = False;

    pc_FeedTray->lock();
	if( TYPE_TrayStatusNon != pc_FeedTray->getTrayStatus( TYPE_FeedTray3 ) ) {
		bl_Attach = True;
	}
    pc_FeedTray->unlock();

	return bl_Attach;
}

/*================================================================================================
［機能］	判定関数 Tray4 装着状態.
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2014/01/31 新規作成 CQ384241 ZeusCD UV-01249 AppleAirPrint.
================================================================================================*/
// 見直した方がいい.
Bool IPPC_LibOpMachine::isAttachTray4()
{
	Bool bl_Attach = False;

    pc_FeedTray->lock();
	if( TYPE_TrayStatusNon != pc_FeedTray->getTrayStatus( TYPE_FeedTray4 ) ) {
		bl_Attach = True;
	}
    pc_FeedTray->unlock();

	return bl_Attach;
}

/*================================================================================================
［機能］	判定関数 Tray5 装着状態.
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2018/05/31 新規作成 DenebMLK 機器構成変更_No.7 HCL-VIJAY Added.
================================================================================================*/
// 見直した方がいい.
Bool IPPC_LibOpMachine::isAttachTray5()
{
	Bool bl_Attach = False;

    pc_FeedTray->lock();
	if( TYPE_TrayStatusNon != pc_FeedTray->getTrayStatus( TYPE_FeedTray5 ) ) {
		bl_Attach = True;
	}
    pc_FeedTray->unlock();

	return bl_Attach;
}

/*================================================================================================
［機能］	仕様変換関数(KM⇒IPP) output-bin.
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2014/01/31 新規作成 CQ384241 ZeusCD UV-01249 AppleAirPrint.
================================================================================================*/
IPPE_OutputBin IPPC_LibOpMachine::CnvSpec_OutputBin( TYPE_OutputTray e_OutputTray )
{
	TYPE_OutputTray e_Return = IPPE_OutputBin_NONE;

	IPPE_AttachFS e_AttachFS = Judge_AttachFSType();

	// ZeusCD CQ386774 OPS1八代 2014/02/25 START.
	pc_PrinterSystem->lock();
	Bool bl_OptionTray3  = pc_PrinterSystem->isOptionTray3();
	Bool bl_OptionTray4  = pc_PrinterSystem->isOptionTray4();		// ZeusS ZX0 機器構成変更_No.3対応 HCL-VIJAY 2015/06/22 Added.
	pc_PrinterSystem->unlock();
	// ZeusCD CQ386774 OPS1八代 2014/02/25 END.

	switch ( e_OutputTray ) {

		// メイントレイ（Ｍ：エレベートトレイ）.
		case TYPE_OutputTrayMain:
			switch( e_AttachFS ) {
				case IPPE_AttachFS_None:
#ifdef DEF_A4_Cus // VenusMLK 2OPS Yamaguchi 20151006 START.
					e_Return = IPPE_OutputBin_FaceDown;
#else // A3.
					e_Return = IPPE_OutputBin_Top;
#endif // VenusMLK 2OPS Yamaguchi 20151006 END.
					break;
				case IPPE_AttachFS_JS602:	e_Return = IPPE_OutputBin_Middle;	break;
				case IPPE_AttachFS_FS542:	// 2022/06/27 FPT-LucNH2 【IT6_2.3】 制御構成変更_No.2 Add.
				case IPPE_AttachFS_FS533:	e_Return = IPPE_OutputBin_Top;		break;
				case IPPE_AttachFS_FS534:	e_Return = IPPE_OutputBin_Middle;	break;
				case IPPE_AttachFS_FS535:
				case IPPE_AttachFS_FS537:	// MinervaSBK 制御No15_FS-537 OPS1八代 2015.04.06.
				case IPPE_AttachFS_FS540:	// 2018/10/17 FPT-LucNH2 【IT6_2.0】【EagleH】 機器構成変更_No.5 Add.
					// ZeusCD CQ386774 OPS1八代 2014/02/25 START.
					if( True == bl_OptionTray3 ) {
						e_Return = IPPE_OutputBin_Bottom;
					} else {
						e_Return = IPPE_OutputBin_Middle;
					}
					break;
					// ZeusCD CQ386774 OPS1八代 2014/02/25 END.
#ifdef DEF_A4_Cus // VenusMLK 2OPS Yamaguchi 20151006 START.
				case IPPE_AttachFS_VenusFNS:	e_Return = IPPE_OutputBin_FaceDown;	break;
#endif // VenusMLK 2OPS Yamaguchi 20151006 END.
				case IPPE_AttachFS_FS539:	// 2018/10/17 FPT-LucNH2 【IT6_2.0】【EagleH】 機器構成変更_No.5 Add.
				case IPPE_AttachFS_FS536:	e_Return = IPPE_OutputBin_Middle;	break;	// MinervaSBK 制御No14_FS-536 OPS1八代 2015.03.26.

#ifdef DEF_KRONOSS_Cus // KronosS フィニッシャ対応 START.
				case IPPE_AttachFS_FSP02:	e_Return = IPPE_OutputBin_FaceDown;	break;
#endif // KronosS フィニッシャ対応 END.

				default:					e_Return = IPPE_OutputBin_NONE;		break;
			}
			break;

		// サブトレイ（Ｍ：第１トレイ） メイントレイ以外のトレイ.
		case TYPE_OutputTraySub1:
			switch( e_AttachFS ) {
				case IPPE_AttachFS_None:	e_Return = IPPE_OutputBin_Top;		break;
				case IPPE_AttachFS_JS602:	e_Return = IPPE_OutputBin_Top;		break;
				case IPPE_AttachFS_FS542:	// 2022/06/27 FPT-LucNH2 【IT6_2.3】 制御構成変更_No.2 Add.
				case IPPE_AttachFS_FS533:	e_Return = IPPE_OutputBin_NONE;		break;
				case IPPE_AttachFS_FS534:	e_Return = IPPE_OutputBin_Top;		break;
				case IPPE_AttachFS_FS535:
				case IPPE_AttachFS_FS537:	// MinervaSBK 制御No15_FS-537 OPS1八代 2015.04.06.
				case IPPE_AttachFS_FS540:	// 2018/10/17 FPT-LucNH2 【IT6_2.0】【EagleH】 機器構成変更_No.5 Add.
					// ZeusCD CQ386774 OPS1八代 2014/02/25 START.
					if( True == bl_OptionTray3 ) {
						e_Return = IPPE_OutputBin_Middle;
					} else {
						e_Return = IPPE_OutputBin_Top;
					}
					break;
					// ZeusCD CQ386774 OPS1八代 2014/02/25 END.
					break;
				case IPPE_AttachFS_FS539:	// 2018/10/17 FPT-LucNH2 【IT6_2.0】【EagleH】 機器構成変更_No.5 Add.
				case IPPE_AttachFS_FS536:	e_Return = IPPE_OutputBin_Top;		break;	// MinervaSBK 制御No14_FS-536 OPS1八代 2015.03.26.
				default:					e_Return = IPPE_OutputBin_NONE;		break;
			}
			break;

		// サブトレイ（Ｍ：第３トレイ） メイントレイ以外のトレイ.
		case TYPE_OutputTraySub2:
			switch( e_AttachFS ) {
				case IPPE_AttachFS_None:	e_Return = IPPE_OutputBin_Top;		break;
				case IPPE_AttachFS_JS602:	e_Return = IPPE_OutputBin_NONE;		break;
				case IPPE_AttachFS_FS542:	// 2022/06/27 FPT-LucNH2 【IT6_2.3】 制御構成変更_No.2 Add.
				case IPPE_AttachFS_FS533:	e_Return = IPPE_OutputBin_NONE;		break;
				case IPPE_AttachFS_FS534:
					// ZeusCD CQ386774 OPS1八代 2014/02/25 START.
					if( True == bl_OptionTray3 ) {
						e_Return = IPPE_OutputBin_Bottom;
					} else {
						e_Return = IPPE_OutputBin_Top;
					}
					break;
				case IPPE_AttachFS_FS535:
				case IPPE_AttachFS_FS537:	// MinervaSBK 制御No15_FS-537 OPS1八代 2015.04.06.
				case IPPE_AttachFS_FS540:// 2018/10/17 FPT-LucNH2 【IT6_2.0】【EagleH】 機器構成変更_No.5 Add.
					if( True == bl_OptionTray3 ) {
						e_Return = IPPE_OutputBin_Top;
					} else {
						e_Return = IPPE_OutputBin_NONE;
					}
					break;
					// ZeusCD CQ386774 OPS1八代 2014/02/25 END.
				case IPPE_AttachFS_FS539:	// 2018/10/17 FPT-LucNH2 【IT6_2.0】【EagleH】 機器構成変更_No.5 Add.
				case IPPE_AttachFS_FS536:	e_Return = IPPE_OutputBin_NONE;		break;	// MinervaSBK 制御No14_FS-536 OPS1八代 2015.03.26.
				default:					e_Return = IPPE_OutputBin_NONE;		break;
			}
			break;
					
		// ZeusS ZX0 機器構成変更_No.3対応 HCL-VIJAY 2015/06/22 Added START.
		// サブトレイ３.
		case TYPE_OutputTraySub3:
			switch( e_AttachFS ) {
				case IPPE_AttachFS_None:	e_Return = IPPE_OutputBin_Top;		break;
				case IPPE_AttachFS_JS602:	e_Return = IPPE_OutputBin_NONE;		break;
				case IPPE_AttachFS_FS542:	// 2022/06/27 FPT-LucNH2 【IT6_2.3】 制御構成変更_No.2 Add.
				case IPPE_AttachFS_FS533:	e_Return = IPPE_OutputBin_NONE;		break;
				case IPPE_AttachFS_FS534:	e_Return = IPPE_OutputBin_NONE;		break;
				case IPPE_AttachFS_FS535:	e_Return = IPPE_OutputBin_NONE;		break;
				case IPPE_AttachFS_FS536:
				case IPPE_AttachFS_FS537:
// 2018/10/17 FPT-LucNH2 【IT6_2.0】【EagleH】 機器構成変更_No.5 Start.
				case IPPE_AttachFS_FS539:
				case IPPE_AttachFS_FS540:
// 2018/10/17 FPT-LucNH2 【IT6_2.0】【EagleH】 機器構成変更_No.5 End.
					if( True == bl_OptionTray4 ) {
						e_Return = IPPE_OutputBin_Right;
					} else {
						e_Return = IPPE_OutputBin_NONE;
					}
					break;
				default:					e_Return = IPPE_OutputBin_NONE;		break;
			}
			break;
		// ZeusS ZX0 機器構成変更_No.3対応 HCL-VIJAY 2015/06/22 Added END.

		// 上記以外.
		default:
			e_Return = IPPE_OutputBin_NONE;
			break;
	}

	if( IPPE_OutputBin_NONE == e_Return ) {
		DIPPTRACE1( "[IPP][UnExpected] CnvSpec_OutputBin = %d\n",e_Return );
	}

	return e_Return;
}

// ZeusCD CQ386774 OPS1八代 2014/02/25 START.
/*================================================================================================
［機能］	仕様変換関数(IPP⇒KM) output-bin.
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2014/02/25 新規作成 CQ386774.
================================================================================================*/
TYPE_OutputTray IPPC_LibOpMachine::CnvSpecRev_OutputBin( IPP_OUTPUT_BIN e_OutputBin )
{
	TYPE_OutputTray e_Return = TYPE_OutputTrayMain;

	IPPE_AttachFS e_AttachFS = Judge_AttachFSType();

	pc_PrinterSystem->lock();
	Bool bl_OptionTray3  = pc_PrinterSystem->isOptionTray3();
	Bool bl_OptionTray4  = pc_PrinterSystem->isOptionTray4();	// ZeusS ZX0 機器構成変更_No.3対応 HCL-VIJAY 2015/06/22 Added.
	pc_PrinterSystem->unlock();

	switch ( e_OutputBin ) {

		case TOP:
			switch( e_AttachFS ) {
				case IPPE_AttachFS_None:	e_Return = TYPE_OutputTrayMain;		break;
				case IPPE_AttachFS_JS602:	e_Return = TYPE_OutputTraySub1;		break;
				case IPPE_AttachFS_FS542:	// 2022/06/27 FPT-LucNH2 【IT6_2.3】 制御構成変更_No.2 Add.
				case IPPE_AttachFS_FS533:	e_Return = TYPE_OutputTrayMain;		break;
				case IPPE_AttachFS_FS534:	e_Return = TYPE_OutputTraySub1;		break;
				case IPPE_AttachFS_FS535:
				case IPPE_AttachFS_FS537:	// MinervaSBK 制御No15_FS-537 OPS1八代 2015.04.06.
				case IPPE_AttachFS_FS540:	// 2018/10/17 FPT-LucNH2 【IT6_2.0】【EagleH】 機器構成変更_No.5 Add.
					if( True == bl_OptionTray3 ) {
						e_Return = TYPE_OutputTraySub2;
					} else {
						e_Return = TYPE_OutputTraySub1;
					}
					break;
				case IPPE_AttachFS_FS539:	// 2018/10/17 FPT-LucNH2 【IT6_2.0】【EagleH】 機器構成変更_No.5 Add.
				case IPPE_AttachFS_FS536:	e_Return = TYPE_OutputTraySub1;		break;	// MinervaSBK 制御No14_FS-536 OPS1八代 2015.03.26.
				default:					e_Return = TYPE_OutputTrayMain;		break;
			}
			break;

		case MIDDLE:
			switch( e_AttachFS ) {
				case IPPE_AttachFS_JS602:	e_Return = TYPE_OutputTrayMain;		break;
				case IPPE_AttachFS_FS534:	e_Return = TYPE_OutputTrayMain;		break;
				case IPPE_AttachFS_FS535:
				case IPPE_AttachFS_FS537:	// MinervaSBK 制御No15_FS-537 OPS1八代 2015.04.06.
				case IPPE_AttachFS_FS540:	// 2018/10/17 FPT-LucNH2 【IT6_2.0】【EagleH】 機器構成変更_No.5 Add.
					if( True == bl_OptionTray3 ) {
						e_Return = TYPE_OutputTraySub1;
					} else {
						e_Return = TYPE_OutputTrayMain;
					}
					break;
				case IPPE_AttachFS_FS539:	// 2018/10/17 FPT-LucNH2 【IT6_2.0】【EagleH】 機器構成変更_No.5 Add.
				case IPPE_AttachFS_FS536:	e_Return = TYPE_OutputTrayMain;		break;	// MinervaSBK 制御No14_FS-536 OPS1八代 2015.03.26.
				default:					e_Return = TYPE_OutputTrayMain;		break;
			}
			break;

		case BOTTOM:
			switch( e_AttachFS ) {
				case IPPE_AttachFS_FS534:
					if( True == bl_OptionTray3 ) {
						e_Return = TYPE_OutputTraySub2;
					}
					break;
				case IPPE_AttachFS_FS535:
				case IPPE_AttachFS_FS537:	// MinervaSBK 制御No15_FS-537 OPS1八代 2015.04.06.
				case IPPE_AttachFS_FS540:	// 2018/10/17 FPT-LucNH2 【IT6_2.0】【EagleH】 機器構成変更_No.5 Add.
					if( True == bl_OptionTray3 ) {
						e_Return = TYPE_OutputTrayMain;
					}
					break;
				default:					e_Return = TYPE_OutputTrayMain;		break;
			}
			break;
		// ZeusS ZX0 機器構成変更_No.3対応 HCL-VIJAY 2015/06/22 Added START.
		case RIGHT:
			switch( e_AttachFS ) {
				case IPPE_AttachFS_FS536:
				case IPPE_AttachFS_FS537:
// 2018/10/17 FPT-LucNH2 【IT6_2.0】【EagleH】 機器構成変更_No.5 Start.
				case IPPE_AttachFS_FS539:
				case IPPE_AttachFS_FS540:
// 2018/10/17 FPT-LucNH2 【IT6_2.0】【EagleH】 機器構成変更_No.5 End.
					if( True == bl_OptionTray4 ) {
						e_Return = TYPE_OutputTraySub3;
					}
					break;
				default:					e_Return = TYPE_OutputTrayMain;		break;
			}
			break;
		// ZeusS ZX0 機器構成変更_No.3対応 HCL-VIJAY 2015/06/22 Added END.	

#ifdef DEF_A4_Cus // VenusMLK先行開発 2OPS Yamaguchi START.
		case FACE_DOWN:
			switch( e_AttachFS ) {
				case IPPE_AttachFS_VenusFNS:	e_Return = TYPE_OutputTrayMain;
				break;
#ifdef DEF_KRONOSS_Cus // KronosS フィニッシャ対応 START.
				case IPPE_AttachFS_FSP02:	e_Return = TYPE_OutputTrayMain;
				break;
#endif // KronosS フィニッシャ対応 END.
				case IPPE_AttachFS_None:		e_Return = TYPE_OutputTrayMain;
				break;
				default:						e_Return = TYPE_OutputTrayMain;		break;
			}
			break;
#endif // VenusMLK先行開発 2OPS Yamaguchi END.

		default:
			e_Return = TYPE_OutputTrayMain;
			break;
	}

	return e_Return;
}
// ZeusCD CQ386774 OPS1八代 2014/02/25 END.

/*================================================================================================
［機能］	仕様変換関数(KM⇒IPP) finishings.
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2014/01/31 新規作成 CQ384241 ZeusCD UV-01249 AppleAirPrint.
================================================================================================*/
static struct IPPS_CnvTable_Finishings {
	TYPE_IPPFinishings 	e_IPPFinishings;		// KM定義.
	IPP_FINISHINGS		e_FINISHINGS;			// IPP定義(NICFUM) - finishings.
} st_CnvTable_Finishings[] = {
	{ TYPE_IPPFinishingsNone, 	 				FIN_NONE 				},	// 'none'.
	{ TYPE_IPPFinishingsStaple, 				FIN_STAPLE 				},	// 'staple'.
	{ TYPE_IPPFinishingsPunch, 	 				FIN_PUNCH 				},	// 'punch'.
	{ TYPE_IPPFinishingsCover, 	 				FIN_COVER 				},	// 'cover'.
	{ TYPE_IPPFinishingsBind, 	 				FIN_BIND 				},	// 'bind'.
	{ TYPE_IPPFinishingsSaddleStitch, 			FIN_SADDLE_STITCH 		},	// 'saddle-stitch'.
	{ TYPE_IPPFinishingsFold, 					FIN_FOLD 				},	// 'fold'.
	{ TYPE_IPPFinishingsJogOffset, 	 			FIN_JOG_OFFSET 			},	// 'jog-offset'.
	{ TYPE_IPPFinishingsStapleTopLeft, 			FIN_STAPLE_TOP_LEFT 	},
	{ TYPE_IPPFinishingsStapleBottomLeft, 	 	FIN_STAPLE_BOTTOM_LEFT 	},
	{ TYPE_IPPFinishingsStapleTopRight, 	 	FIN_STAPLE_TOP_RIGHT 	},
	{ TYPE_IPPFinishingsStapleBottomRight, 	 	FIN_STAPLE_BOTTOM_RIGHT },
	{ TYPE_IPPFinishingsStapleDualLeft, 	 	FIN_STAPLE_DUAL_LEFT 	},
	{ TYPE_IPPFinishingsStapleDualTop, 	 		FIN_STAPLE_DUAL_TOP 	},
	{ TYPE_IPPFinishingsStapleDualRight, 	 	FIN_STAPLE_DUAL_RIGHT 	},
	{ TYPE_IPPFinishingsStapleDualBottom, 	 	FIN_STAPLE_DUAL_BOTTOM 	},
	{ TYPE_IPPFinishingsPunchTopLeft, 	 		FIN_PUNCH_TOP_LEFT 		},
	{ TYPE_IPPFinishingsPunchBottomLeft, 	 	FIN_PUNCH_BOTTOM_LEFT 	},
	{ TYPE_IPPFinishingsPunchTopRight, 	 		FIN_PUNCH_TOP_RIGHT 	},
	{ TYPE_IPPFinishingsPunchBottomRight, 	 	FIN_PUNCH_BOTTOM_RIGHT 	},
	{ TYPE_IPPFinishingsPunchDualLeft, 	 		FIN_PUNCH_DUAL_LEFT 	},
	{ TYPE_IPPFinishingsPunchDualTop, 	 		FIN_PUNCH_DUAL_TOP 		},
	{ TYPE_IPPFinishingsPunchDualRight, 	 	FIN_PUNCH_DUAL_RIGHT 	},
	{ TYPE_IPPFinishingsPunchDualBottom, 	 	FIN_PUNCH_DUAL_BOTTOM 	},
	{ TYPE_IPPFinishingsPunchTripleLeft, 	 	FIN_PUNCH_TRIPLE_LEFT 	},
	{ TYPE_IPPFinishingsPunchTripleTop, 	 	FIN_PUNCH_TRIPLE_TOP 	},
	{ TYPE_IPPFinishingsPunchTripleRight, 	 	FIN_PUNCH_TRIPLE_RIGHT 	},
	{ TYPE_IPPFinishingsPunchTripleBottom, 	 	FIN_PUNCH_TRIPLE_BOTTOM },
	{ TYPE_IPPFinishingsPunchQuadLeft, 	 		FIN_PUNCH_QUAD_LEFT 	},
	{ TYPE_IPPFinishingsPunchQuadTop, 	 		FIN_PUNCH_QUAD_TOP 		},
	{ TYPE_IPPFinishingsPunchQuadRight, 	 	FIN_PUNCH_QUAD_RIGHT 	},
	{ TYPE_IPPFinishingsPunchQuadBottom, 	 	FIN_PUNCH_QUAD_BOTTOM 	},
	// Poseidon UV-01429 AppleAirPrint1.4対応 Bairavi 2014/07/07 START.
	{ TYPE_IPPFinishingsFoldHalf, 	 			FIN_FOLD_HALF	 		},
	{ TYPE_IPPFinishingsFoldLetter, 	 		FIN_FOLD_LETTER 		},
	{ TYPE_IPPFinishingsFoldZ, 	 				FIN_FOLD_ENGINEERING_Z	},			//IT5_4.1 Airprint1.7対応 HCL_VIJAY 2017/1/30 Modified.
	// Poseidon UV-01429 AppleAirPrint1.4対応 Bairavi 2014/07/07 END.
	{ TYPE_IPPFinishingsEnd, 	 				FIN_END  				},
};

IPP_FINISHINGS IPPC_LibOpMachine::CnvSpec_finishings( TYPE_IPPFinishings e_IPPFinishings )
{
	IPP_FINISHINGS e_Return = FIN_NONE;

	for( ushort i = 0; i < sizeof( st_CnvTable_Finishings ) / sizeof( IPPS_CnvTable_Finishings ); i++ ) {
		if( e_IPPFinishings == st_CnvTable_Finishings[i].e_IPPFinishings ) {
			e_Return = st_CnvTable_Finishings[i].e_FINISHINGS;
			break;
		}
	}

	return e_Return;
}

/*================================================================================================
［機能］    仕様変換関数(KM⇒IPP) finishings-col.
--------------------------------------------------------------------------------------------------
［引数］    
［戻値］    
--------------------------------------------------------------------------------------------------
[履歴]  Helios KRQ-93 AirPrint1.5対応 HCL-Sankar 2015/02/03 ADDED
================================================================================================*/
static struct IPPS_CnvTable_Finishings_Col {
	TYPE_IPPFinishings  e_IPPFinishings_Col;	// KM定義.
	UINT8	uc_FINISHINGS_Col[MAX_STRING32];	// IPP定義(NICFUM) - finishings-col.
} st_CnvTable_Finishings_Col[] = {
	{ TYPE_IPPFinishingsNone,					"none"					},	// 'none'.
	{ TYPE_IPPFinishingsStaple,					"staple"				},	// 'staple'.
	{ TYPE_IPPFinishingsPunch,					"punch"					},	// 'punch'.
	{ TYPE_IPPFinishingsBind,					"bind"					},	// 'bind'.
	{ TYPE_IPPFinishingsSaddleStitch,			"saddle-stitch"			},	// 'saddle-stitch'.
	{ TYPE_IPPFinishingsFold,					"fold"					},	// 'fold'.
	{ TYPE_IPPFinishingsJogOffset,				"jog-offset"			},	// 'jog-offset'.
	{ TYPE_IPPFinishingsStapleTopLeft,			"staple-top-left"		},
	{ TYPE_IPPFinishingsStapleTopRight,			"staple-top-right"		},
	{ TYPE_IPPFinishingsStapleDualLeft,			"staple-dual-left"		},
	{ TYPE_IPPFinishingsStapleDualTop,			"staple-dual-top"		},
	{ TYPE_IPPFinishingsStapleDualRight,		"staple-dual-right"		},
	{ TYPE_IPPFinishingsPunchDualLeft,			"punch-dual-left"		},
	{ TYPE_IPPFinishingsPunchDualTop,			"punch-dual-top"		},
	{ TYPE_IPPFinishingsPunchDualRight,			"punch-dual-right"		},
	{ TYPE_IPPFinishingsPunchTripleLeft,		"punch-triple-left"		},
	{ TYPE_IPPFinishingsPunchTripleTop,			"punch-triple-top"		},
	{ TYPE_IPPFinishingsPunchTripleRight,		"punch-triple-right"	},
	{ TYPE_IPPFinishingsPunchQuadLeft,			"punch-quad-left"		},
	{ TYPE_IPPFinishingsPunchQuadTop,			"punch-quad-top"		},
	{ TYPE_IPPFinishingsPunchQuadRight,			"punch-quad-right"		},
	{ TYPE_IPPFinishingsFoldHalf,				"fold-half"				},
	{ TYPE_IPPFinishingsFoldLetter,				"fold-letter"			},
	{ TYPE_IPPFinishingsFoldZ,					"fold-engineering-z"	},	//IT5_4.1 Airprint1.7対応 HCL_VIJAY 2017/1/30 Modified.(fold-z->fold-engineering-z)
	{ TYPE_IPPFinishingsEnd,					""						},
};

const char* IPPC_LibOpMachine::Get_FinishingsTemplate( TYPE_IPPFinishings e_IPPFinishings )
{

	for( ushort i = 0; i < sizeof( st_CnvTable_Finishings_Col ) / sizeof( IPPS_CnvTable_Finishings_Col )-1; i++ ) {
		if( e_IPPFinishings == st_CnvTable_Finishings_Col[i].e_IPPFinishings_Col ) {
			break;
		}
	}

	return (const char *)st_CnvTable_Finishings_Col[i].uc_FINISHINGS_Col;

}

// ZeusCD UV-01249 AppleAirPrint OPS1正部家 2014/01/31 START.
/*================================================================================================
［機能］	判定関数 MachineType0.
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2014/01/31 新規作成 ZeusCD UV-01249 AppleAirPrint.
		   2017/11/7 Eagle 制御構成変更_No.6 HCL-VIJAY Modified.
================================================================================================*/
Bool IPPC_LibOpMachine::isMachineType0()
{
	Bool bl_Ret = False;

	TYPE_MachineType e_MachineType = TYPE_MachineTypeEagle1;
  	pc_SystemInfo->lock();
	e_MachineType = pc_SystemInfo->getMachineType();
	pc_SystemInfo->unlock();

	if(TYPE_MachineTypeEagle0 == e_MachineType){
		bl_Ret = True;
	}

	return bl_Ret;
}

/*================================================================================================
［機能］	判定関数 MachineTypeX.
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2014/01/31 新規作成 ZeusCD UV-01249 AppleAirPrint.
		   2017/11/7 Eagle 制御構成変更_No.6 HCL-VIJAY Modified.
================================================================================================*/
Bool IPPC_LibOpMachine::isMachineTypeX()
{
	Bool bl_Ret = False;

	TYPE_MachineType e_MachineType = TYPE_MachineTypeEagle1;
  	pc_SystemInfo->lock();
	e_MachineType = pc_SystemInfo->getMachineType();
	pc_SystemInfo->unlock();
	
	if(TYPE_MachineTypeEagleX == e_MachineType){
		bl_Ret = True;
	}

	return bl_Ret;
}

/*================================================================================================
［機能］	判定関数 MachineTypeZ.
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2017/11/7 Eagle 制御構成変更_No.6 HCL-VIJAY Added.
================================================================================================*/
Bool IPPC_LibOpMachine::isMachineTypeZ()
{
	Bool bl_Ret = False;

	TYPE_MachineType e_MachineType = TYPE_MachineTypeEagle1;
  	pc_SystemInfo->lock();
	e_MachineType = pc_SystemInfo->getMachineType();
	pc_SystemInfo->unlock();
	
	if(TYPE_MachineTypeEagleZ == e_MachineType){
		bl_Ret = True;
	}

	return bl_Ret;
}

/*================================================================================================
［機能］	判定関数 MachineType = Sparrow1/2SFP.
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     //Eagle 制御構成変更_No.100対応 HCL-VIJAY 2018/04/06 Added.
================================================================================================*/
Bool IPPC_LibOpMachine::isMachineTypeSparrow12SFP()
{
	Bool bl_Ret = False;

	TYPE_MachineType e_MachineType = TYPE_MachineTypeSparrow1MFP;
  	pc_SystemInfo->lock();
	e_MachineType = pc_SystemInfo->getMachineType();
	pc_SystemInfo->unlock();
	
	if(TYPE_MachineTypeSparrow1SFP == e_MachineType || TYPE_MachineTypeSparrow2SFP == e_MachineType){
		bl_Ret = True;
	}

	return bl_Ret;
}

/*================================================================================================
［機能］	判定関数 ZU装着状態.
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2014/01/31 新規作成 ZeusCD UV-01249 AppleAirPrint.
================================================================================================*/
Bool IPPC_LibOpMachine::isAttachZU()
{
	Bool b_Ret = False;

	pc_PrinterSystem->lock();
	if( pc_PrinterSystem->isExistZUOption() )
	{
		b_Ret = True;
	}
	pc_PrinterSystem->unlock();
	
	return b_Ret;
}
/*================================================================================================
［機能］	判定関数 PI装着状態.
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2014/01/31 新規作成 ZeusCD UV-01249 AppleAirPrint.
================================================================================================*/
Bool IPPC_LibOpMachine::isAttachPI()
{
	Bool b_Ret = False;

	pc_PrinterSystem->lock();
	if( pc_PrinterSystem->isCoverSheetFeeder() )
	{
		b_Ret = True;
	}
	pc_PrinterSystem->unlock();
	
	return b_Ret;
}


/*================================================================================================
［機能］	判定関数 フィッシャー機能
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
[履歴]	Helios KRQ-93 AirPrint1.5対応 HCL-Sankar 2015/02/03 
        Eagle  AR1071315 MopriaプリントでBind指定で印刷を実行すると、中折りで印刷される 2019/02/25
================================================================================================*/
Bool IPPC_LibOpMachine::isEnableFSFunc( TYPE_IPPFinishings e_IPPFinishings )
{
#ifdef DEF_A4_Cus // VenusMLK 2OPS Yamaguchi 20151002 START.
	// A4 Device has no Punch and Fold Function.
	IPPE_AttachFS 	   e_AttachFS 	   = Judge_AttachFSType();
#else // A3.
	IPPE_AttachFS 	   e_AttachFS 	   = Judge_AttachFSType();
	TYPS_PunchFunction s_PunchFunction = pc_PrinterSystem->getPunchFunction();
	TYPS_FoldFunction  s_FoldFunction  = pc_PrinterSystem->getFoldFunction();
#endif // VenusMLK 2OPS Yamaguchi 20151002 END.
	
#ifdef DEF_A4_Cus // VenusMLK 2OPS Yamaguchi 20151002 START.
	if( TYPE_IPPFinishingsNone == e_IPPFinishings ){
		return True;
	}
	
#ifdef DEF_KRONOSS_Cus // KronosS フィニッシャ対応 START.
	// finisher.
	else if( ( TYPE_IPPFinishingsStaple		== e_IPPFinishings )
	 || ( TYPE_IPPFinishingsStapleTopLeft	== e_IPPFinishings ) ) {
		if( IPPE_AttachFS_FSP02 == e_AttachFS ) {
			return True;
		}
	}
#else // VenusMLK.
	// finisher.
	else if( ( TYPE_IPPFinishingsStaple		== e_IPPFinishings )
	 || ( TYPE_IPPFinishingsStapleTopLeft	== e_IPPFinishings )
	 || ( TYPE_IPPFinishingsJogOffset		== e_IPPFinishings ) ) {
		if( IPPE_AttachFS_VenusFNS == e_AttachFS ) {
			return True;
		}
	}
#endif	// KronosS フィニッシャ対応 END.

	// other attributes are not supported.
	else{
		return False;
	}
	// 2018/10/24 【AR1029580】 ADS Warning除去対応 : 全パターンに対し必ず値を返すように処理追加.
	return False;
#else // A3.
	
	// 非サポートの定義は飛ばす.
	// ZeusCD CQ386774 OPS1八代 2014/02/25 START.
//	if( ( TYPE_IPPFinishingsStaple 			  == e_IPPFinishings )
//	 || ( TYPE_IPPFinishingsPunch  			  == e_IPPFinishings )
//	 || ( TYPE_IPPFinishingsCover  			  == e_IPPFinishings )
	if( ( TYPE_IPPFinishingsCover 			  == e_IPPFinishings )
	// ZeusCD CQ386774 OPS1八代 2014/02/25 END.
	 || ( TYPE_IPPFinishingsStapleBottomLeft  == e_IPPFinishings )
	 || ( TYPE_IPPFinishingsStapleBottomRight == e_IPPFinishings )
	 || ( TYPE_IPPFinishingsStapleDualBottom  == e_IPPFinishings )
	 || ( TYPE_IPPFinishingsPunchTopLeft  	  == e_IPPFinishings )
	 || ( TYPE_IPPFinishingsPunchBottomLeft   == e_IPPFinishings )
	 || ( TYPE_IPPFinishingsPunchTopRight     == e_IPPFinishings )
	 || ( TYPE_IPPFinishingsPunchBottomRight  == e_IPPFinishings )
	 || ( TYPE_IPPFinishingsPunchDualBottom   == e_IPPFinishings )
	 || ( TYPE_IPPFinishingsPunchTripleBottom == e_IPPFinishings )
	 || ( TYPE_IPPFinishingsPunchQuadBottom   == e_IPPFinishings ) ) {
		return False;
	}

	// 以下、オプションの装着状態を参照し、サポート有無を切り替える.
	
	// 中折り.
	if( TYPE_IPPFinishingsBind == e_IPPFinishings ) {
		//Eagle AR1071315 ht_kurihara 2019/02/25 start.
		//"bind"というPWG仕様は本来は「糊付けやテープ等、エッジを纏める」という意で、KM-OP機ではサポートしていなため、IPP能力通知でbindを非サポートとする.
		return False;
		//Eagle AR1071315 ht_kurihara 2019/02/25 end.
	}

	// 中綴じ.
	if( TYPE_IPPFinishingsSaddleStitch == e_IPPFinishings ) {
		if( False == s_FoldFunction.b_FoldCenterInner ) {
			return False;
		}
	}

	// 三つ折.
	if( TYPE_IPPFinishingsFold == e_IPPFinishings ) {
		if( False == s_FoldFunction.b_FoldLetter ) {
			return False;
		}
	}
	if( TYPE_IPPFinishingsFoldHalf == e_IPPFinishings ) {
		if( False == s_FoldFunction.b_FoldCenterInner ) {
			return False;
		}
	}
	if( TYPE_IPPFinishingsFoldLetter == e_IPPFinishings ) {
		if( False == s_FoldFunction.b_FoldLetter ) {
			return False;
		}
	}
	if( TYPE_IPPFinishingsFoldZ == e_IPPFinishings ) {
		if( False == s_FoldFunction.b_FoldZshaped ) {
			return False;
		}
	}
	// ステープル(1点/2点).
	// ZeusCD CQ386774 OPS1八代 2014/02/25 START.
//	if( ( TYPE_IPPFinishingsStapleTopLeft 	 == e_IPPFinishings )
	if( ( TYPE_IPPFinishingsStaple		 	 == e_IPPFinishings )
	 || ( TYPE_IPPFinishingsStapleTopLeft    == e_IPPFinishings )
	// ZeusCD CQ386774 OPS1八代 2014/02/25 END.
	 || ( TYPE_IPPFinishingsStapleTopRight   == e_IPPFinishings )
	 || ( TYPE_IPPFinishingsStapleDualLeft 	 == e_IPPFinishings )
	 || ( TYPE_IPPFinishingsStapleDualTop 	 == e_IPPFinishings )
	 || ( TYPE_IPPFinishingsStapleDualRight  == e_IPPFinishings ) ) {
		if( ( IPPE_AttachFS_None == e_AttachFS ) || ( IPPE_AttachFS_JS602 == e_AttachFS ) ) {
			return False;
		}
	}
	// ZeusCD CQ386774 OPS1八代 2014/02/25 START.
	// パンチ.
	if( TYPE_IPPFinishingsPunch		 	 	== e_IPPFinishings ) {
		if(	 ( False == s_PunchFunction.b_Punch2Hole 		)
		  && ( False == s_PunchFunction.b_Punch3Hole 		)
		  && ( False == s_PunchFunction.b_Punch4Hole 		)
		  && ( False == s_PunchFunction.b_Punch4HoleSWE 	) ) {
			return False;
		}
	}
	// ZeusCD CQ386774 OPS1八代 2014/02/25 END.
	// パンチ(2穴).
	if( ( TYPE_IPPFinishingsPunchDualLeft 	 == e_IPPFinishings )
	 || ( TYPE_IPPFinishingsPunchDualTop 	 == e_IPPFinishings )
	 || ( TYPE_IPPFinishingsPunchDualRight   == e_IPPFinishings ) ) {
		if( False == s_PunchFunction.b_Punch2Hole ) {
			return False;
		}
	}
	// パンチ(3穴).
	if( ( TYPE_IPPFinishingsPunchTripleLeft  == e_IPPFinishings )
	 || ( TYPE_IPPFinishingsPunchTripleTop 	 == e_IPPFinishings )
	 || ( TYPE_IPPFinishingsPunchTripleRight == e_IPPFinishings ) ) {
		if( False == s_PunchFunction.b_Punch3Hole ) {
			return False;
		}
	}
	// パンチ(4穴).
	if( ( TYPE_IPPFinishingsPunchQuadLeft    == e_IPPFinishings )
	 || ( TYPE_IPPFinishingsPunchQuadTop 	 == e_IPPFinishings )
	 || ( TYPE_IPPFinishingsPunchQuadRight   == e_IPPFinishings ) ) {
		if( ( False == s_PunchFunction.b_Punch4Hole ) && ( False == s_PunchFunction.b_Punch4HoleSWE ) ) {	
			return False;
		}
	}
#endif // VenusMLK 2OPS Yamaguchi 20151002 END.
	return True;	/* 2018/11/01 [AR1029580] Sparrow_MFP 静的解析対応: ID28738, 戻り値 */
}

const char* p_IPP_MarkerColors[] = {
// ZeusCD CQ386774 OPS1八代 2014/02/25 START.
	"#00FFFF",				// IPPE_MarkerColors_Cyan.
	"#FF00FF",				// IPPE_MarkerColors_Magenta.
	"#FFFF00",				// IPPE_MarkerColors_Yellow.
	"#000000",				// IPPE_MarkerColors_Black.
	"none",					// IPPE_MarkerColors_None.
	"",						// IPPE_MarkerColors_Unidentified.
// ZeusCD CQ386774 OPS1八代 2014/02/25 END.
};
const char* IPPC_LibMarker::Get_MarkerColors( IPPE_MarkerColors e_Colors )
{
	if( e_Colors < IPPE_MarkerColors_Cyan || IPPE_MarkerColors_Unidentified < e_Colors ) {
		// 不正な配列番号を参照しないよう、(ログを出力した上で)、配列番号の適用範囲内に丸め込む.
		// (このログが出力されるということは、呼び元の実装に誤りがある).
		DIPPTRACE1( "[IPP][fail safe] Get_MarkerColors = %d\n", e_Colors );
		e_Colors = IPPE_MarkerColors_Unidentified;
	}

	return p_IPP_MarkerColors[ e_Colors ];
}

const char* p_IPP_MarkerNames[] = {
	"Toner (Cyan)",					// IPPE_MarkerNames_Toner_C.
	"Toner (Magenta)",				// IPPE_MarkerNames_Toner_M.
	"Toner (Yellow)",				// IPPE_MarkerNames_Toner_Y.
	"Toner (Black)",				// IPPE_MarkerNames_Toner_B.
	"Toner Cartridge",				// IPPE_MarkerNames_TonerCart.
	"Toner Cartridge (C)",			// IPPE_MarkerNames_TonerCart_C.
	"Toner Cartridge (M)",			// IPPE_MarkerNames_TonerCart_M.
	"Toner Cartridge (Y)",			// IPPE_MarkerNames_TonerCart_Y.
	"Toner Cartridge (K)",			// IPPE_MarkerNames_TonerCart_K.
	"Imaging Unit (Cyan)",			// IPPE_MarkerNames_ImgUnit_C.
	"Imaging Unit (Magenta)",		// IPPE_MarkerNames_ImgUnit_M.
	"Imaging Unit (Yellow)",		// IPPE_MarkerNames_ImgUnit_Y.
	"Drum Cartridge(C)",			// IPPE_MarkerNames_DrumCart_C.
	"Developer Cartridge(C)",		// IPPE_MarkerNames_DevCart_C.
	"Drum Cartridge(M)",			// IPPE_MarkerNames_DrumCart_M.
	"Developer Cartridge(M)",		// IPPE_MarkerNames_DevCart_M.
	"Drum Cartridge(Y)",			// IPPE_MarkerNames_DrumCart_Y.
	"Developer Cartridge(Y)",		// IPPE_MarkerNames_DevCart_Y.
	"Drum Cartridge",				// IPPE_MarkerNames_DrumCart.
	"Developer Cartridge",			// IPPE_MarkerNames_DevCart.
	"Drum Cartridge(K)",			// IPPE_MarkerNames_DrumCart_K.
	"Developer Cartridge(K)",		// IPPE_MarkerNames_DevCart_K.
	"Waste Toner Box",				// IPPE_MarkerNames_WasteTonerBox.
	"Fusing Unit",					// IPPE_MarkerNames_FusingUnit.
	"Image Transfer Belt Unit",		// IPPE_MarkerNames_ImgTransBeltUnit.
	"Transfer Roller Unit",			// IPPE_MarkerNames_TransRollUnit.
	"Ozone Filter",					// IPPE_MarkerNames_OzeneFilter.
	"Toner Filter",					// IPPE_MarkerNames_TonerFilter.
	"Staple Cartridge",				// IPPE_MarkerNames_StapleCart.
	"Saddle Staple Cartridge1",		// IPPE_MarkerNames_SaddleStapleCart1.
	"Saddle Staple Cartridge2",		// IPPE_MarkerNames_SaddleStapleCart2.
	"Hole-Punch Scrap Box",			// IPPE_MarkerNames_HolePunchScrapBox.
	"Z Fold Punch Dust Box",		// IPPE_MarkerNames_ZFoldPunchDustBox.
	"PI Main Tray",					// IPPE_MarkerNames_PIMainTray.
	"PI Sub Tray",					// IPPE_MarkerNames_PISubTray.
	"Paper Dust Remover",			// IPPE_MarkerNames_PaperDustRemover.
#ifdef DEF_A4_Cus // VenusMLK 2OPS Yamaguchi 20151006 START.
	"Imaging Unit (Black)",			// IPPE_MarkerNames_ImgUnit_K.
#endif // VenusMLK 2OPS Yamaguchi 20151006 END.
	"",								// IPPE_MarkerNames_NONE.
};

const char* IPPC_LibMarker::Get_MarkerNames( IPPE_MarkerNames e_Names )
{
	if( e_Names < IPPE_MarkerNames_Toner_C || IPPE_MarkerNames_NONE < e_Names ) {
		// 不正な配列番号を参照しないよう、(ログを出力した上で)、配列番号の適用範囲内に丸め込む.
		// (このログが出力されるということは、呼び元の実装に誤りがある).
		DIPPTRACE1( "[IPP][fail safe] Get_MarkerNames = %d\n", e_Names );
		e_Names = IPPE_MarkerNames_NONE;
	}

	return p_IPP_MarkerNames[ e_Names ];
}

Bool IPPC_LibMarker::isMarkerSuppliesSkip(IPPE_MarkerNames e_Name)
{
	Bool bl_IsSkip = False;

	// ZeusSBK CQ528255(MZV-171) HCL-VIJAY 2016/05/02 ADDED START.
	pc_SoftDipSw->lock(); 
	Bool bl_SW = pc_SoftDipSw->getSoftDipSwBit( 111, 6 );		// DipSW227 0x80(bit7 ON)
	pc_SoftDipSw->unlock();
	// ZeusSBK CQ528255(MZV-171) HCL-VIJAY 2016/05/02 ADDED END.
	
	switch( e_Name ) {
	//オプションによるサポートチェック.
		case IPPE_MarkerNames_StapleCart:
			if( Judge_AttachFSType() == IPPE_AttachFS_None ) bl_IsSkip = True;
			break;
		case IPPE_MarkerNames_SaddleStapleCart1:
		case IPPE_MarkerNames_SaddleStapleCart2:
			if( Judge_AttachSDType() == IPPE_AttachSD_None ) bl_IsSkip = True;
			break;
		case IPPE_MarkerNames_HolePunchScrapBox:
			if( Judge_AttachPKType() == IPPE_AttachPK_None ) bl_IsSkip = True;
			break;
		case IPPE_MarkerNames_ZFoldPunchDustBox:
			if( isAttachZU() == False )						 bl_IsSkip = True;
			break;
		case IPPE_MarkerNames_PIMainTray:
		case IPPE_MarkerNames_PISubTray:
			if( isAttachPI() == False )						 bl_IsSkip = True;
			break;
		// ZeusSBK CQ528255(MZV-171) HCL-VIJAY 2016/05/02 ADDED START.
		case IPPE_MarkerNames_ImgUnit_C:
		case IPPE_MarkerNames_ImgUnit_Y:
		case IPPE_MarkerNames_ImgUnit_M:
		case IPPE_MarkerNames_DrumCart:
		case IPPE_MarkerNames_DrumCart_C:
		case IPPE_MarkerNames_DrumCart_Y:
		case IPPE_MarkerNames_DrumCart_M:
		case IPPE_MarkerNames_DrumCart_K:
		case IPPE_MarkerNames_DevCart:
		case IPPE_MarkerNames_DevCart_C:
		case IPPE_MarkerNames_DevCart_Y:
		case IPPE_MarkerNames_DevCart_M:
		case IPPE_MarkerNames_DevCart_K:
		case IPPE_MarkerNames_FusingUnit:
		case IPPE_MarkerNames_ImgTransBeltUnit:
		case IPPE_MarkerNames_TransRollUnit:
		case IPPE_MarkerNames_TonerFilter:
		case IPPE_MarkerNames_OzeneFilter:
		case IPPE_MarkerNames_PaperDustRemover:
			if (True == bl_SW) bl_IsSkip = True;				// DipSWNo.227 bit7がONの場合
			break;
	// ZeusSBK CQ528255(MZV-171) HCL-VIJAY 2016/05/02 ADDED END.
		default:
			break;
	}

	return bl_IsSkip;
}

INT32 IPPC_LibMarker::GetMakerLevels(IPPE_MarkerNames e_Name)
{
	INT32 i_MakerLevel = IPPD_MarkerLevel_Current;
	
	pc_PrinterWarning->lock();
	switch( e_Name ) {
		case IPPE_MarkerNames_DrumCart_C:
			i_MakerLevel = ( 100 - pc_PrinterWarning->getCyanDCConsumptionRate() );
			break;
		case IPPE_MarkerNames_DevCart_C:
			i_MakerLevel = ( 100 - pc_PrinterWarning->getCyanDevCConsumptionRate() );
			break;
		case IPPE_MarkerNames_DrumCart_M:
			i_MakerLevel = ( 100 - pc_PrinterWarning->getMagentaDCConsumptionRate() );
			break;
		case IPPE_MarkerNames_DevCart_M:
			i_MakerLevel = ( 100 - pc_PrinterWarning->getMagentaDevCConsumptionRate() );
			break;
		case IPPE_MarkerNames_DrumCart_Y:
			i_MakerLevel = ( 100 - pc_PrinterWarning->getYellowDCConsumptionRate() );
			break;
		case IPPE_MarkerNames_DevCart_Y:
			i_MakerLevel = ( 100 - pc_PrinterWarning->getYellowDevCConsumptionRate() );
			break;
		case IPPE_MarkerNames_Toner_C:
		case IPPE_MarkerNames_TonerCart_C:
			i_MakerLevel = pc_PrinterWarning->getCTonerQuantity();
			break;
		case IPPE_MarkerNames_Toner_M:
		case IPPE_MarkerNames_TonerCart_M:
			i_MakerLevel = pc_PrinterWarning->getMTonerQuantity();
			break;
		case IPPE_MarkerNames_Toner_Y:
		case IPPE_MarkerNames_TonerCart_Y:
			i_MakerLevel = pc_PrinterWarning->getYTonerQuantity();
			break;
		case IPPE_MarkerNames_Toner_B:
		case IPPE_MarkerNames_TonerCart:
		case IPPE_MarkerNames_TonerCart_K:
			i_MakerLevel = pc_PrinterWarning->getKTonerQuantity();
			break;
		case IPPE_MarkerNames_ImgUnit_C:
			i_MakerLevel = ( 100 - pc_PrinterWarning->getCyanIUConsumptionRate() );
			break;
		case IPPE_MarkerNames_ImgUnit_M:
			i_MakerLevel = ( 100 - pc_PrinterWarning->getMagentaIUConsumptionRate() );
			break;
		case IPPE_MarkerNames_ImgUnit_Y:
			i_MakerLevel = ( 100 - pc_PrinterWarning->getYellowIUConsumptionRate() );
			break;
#ifdef DEF_A4_Cus // VenusMLK 2OPS Yamaguchi 20151006 START.
		case IPPE_MarkerNames_ImgUnit_K:
			i_MakerLevel = ( 100 - pc_PrinterWarning->getBlackIUConsumptionRate() );
			break;
#endif // VenusMLK 2OPS Yamaguchi 20151006 END.
		case IPPE_MarkerNames_DrumCart:
		case IPPE_MarkerNames_DrumCart_K:
			i_MakerLevel = ( 100 - pc_PrinterWarning->getBlackDCConsumptionRate() );
			break;
		case IPPE_MarkerNames_DevCart:
		case IPPE_MarkerNames_DevCart_K:
			i_MakerLevel = ( 100 - pc_PrinterWarning->getBlackDevCConsumptionRate() );
			break;
		case IPPE_MarkerNames_FusingUnit:
			i_MakerLevel = ( 100 - pc_PrinterWarning->getFuserUnitConsumptionRate() );
			break;
		case IPPE_MarkerNames_ImgTransBeltUnit:
			i_MakerLevel = ( 100 - pc_PrinterWarning->getTransferBeltUnitConsumptionRate() );
			break;
		case IPPE_MarkerNames_TransRollUnit:
			i_MakerLevel = ( 100 - pc_PrinterWarning->getTransRollerConsumptionRate() );
			break;
		case IPPE_MarkerNames_OzeneFilter:
			i_MakerLevel = ( 100 - pc_PrinterWarning->getOzonFilterConsumptionRate() );
			break;
		case IPPE_MarkerNames_TonerFilter:
			i_MakerLevel = ( 100 - pc_PrinterWarning->getTonerFilterConsumptionRate() );
			break;
		case IPPE_MarkerNames_PaperDustRemover:
			i_MakerLevel = ( 100 - pc_PrinterWarning->getPaperDustRemovalCleanerConsumptionRate() );
			break;
// [Eagle] 2018/10/22 1PF2G八代 AR_1046111 START.
		case IPPE_MarkerNames_WasteTonerBox:
			if( True == pc_PrinterWarning->isWarning( ERROR_UsedTonerBottleFull ) ) {
				i_MakerLevel = IPPD_MarkerLevel_99;
			}
			else if( True == pc_PrinterWarning->isWarning( ATTENTION_UsedTonerNearFull ) ) {
				i_MakerLevel = IPPD_MarkerLevel_95;
			}
			else {
				i_MakerLevel = IPPD_MarkerLevel_0;
			}
			break;
		case IPPE_MarkerNames_StapleCart:
			if( True == pc_PrinterWarning->isStapleEmpty() ) {
				i_MakerLevel = IPPD_MarkerLevel_0;
			}
			else {
				i_MakerLevel = IPPD_MarkerLevel_100;
			}
			break;
		case IPPE_MarkerNames_SaddleStapleCart1:
			if( True == pc_PrinterWarning->is2ndStapleEmpty() ) {
				i_MakerLevel = IPPD_MarkerLevel_0;
			}
			else {
				i_MakerLevel = IPPD_MarkerLevel_100;
			}
			break;
		case IPPE_MarkerNames_SaddleStapleCart2:
			if( True == pc_PrinterWarning->is3rdStapleEmpty() ) {
				i_MakerLevel = IPPD_MarkerLevel_0;
			}
			else {
				i_MakerLevel = IPPD_MarkerLevel_100;
			}
			break;
		case IPPE_MarkerNames_HolePunchScrapBox:
			if( True == pc_PrinterWarning->isPunchDustFull() ) {
				i_MakerLevel = IPPD_MarkerLevel_100;
			}
			else {
				i_MakerLevel = IPPD_MarkerLevel_0;
			}
			break;
// [Eagle] 2018/10/22 1PF2G八代 AR_1046111 END.
		default:
			break;
	}
    pc_PrinterWarning->unlock();


	return i_MakerLevel;
}
// ZeusCD UV-01249 AppleAirPrint OPS1正部家 2014/01/31 END.

/*================================================================================================
［機能］	IPPC_LibOpeGetJobsクラスのインスタンス取得
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2014/01/31 新規作成 CQ384241 ZeusCD UV-01249 AppleAirPrint.
================================================================================================*/
IPPC_LibOpeGetJobs* IPPC_LibOpeGetJobs::newInstance()
{
	static IPPC_LibOpeGetJobs c_LibOpeGetJobs;
	return	&c_LibOpeGetJobs;
}


/*================================================================================================
［機能］	設定関数 media-col(ActiveJob).
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2014/01/31 新規作成 CQ384241 ZeusCD UV-01249 AppleAirPrint.
================================================================================================*/
void IPPC_LibOpeGetJobs::MediaCol_Active( APIC_Job* pc_Job, IPP_Media_Col* pc_MediaCol )
{
	memset( pc_MediaCol, NULL, sizeof(IPP_Media_Col) );

	TYPE_FeedTray  e_FeedTray  = pc_Job->getJobSpec()->pc_Print->c_PaperTray.getFeedTray();
	TYPE_PaperType e_PaperType = pc_Job->getJobSpec()->pc_Print->c_PaperType.getPaperType();

	IPPS_MediaSize st_MediaSize;
	memset( &st_MediaSize, NULL, sizeof(IPPS_MediaSize) );

	char sc_PaparTrayMedia[32+1];
	memset( sc_PaparTrayMedia , NULL , sizeof(sc_PaparTrayMedia) );
	pc_Job->getJobSpec()->pc_IPP->c_JobTempAttr.getPaparTrayMedia( sc_PaparTrayMedia );
	// Helios KRQ-93 AirPrint1.5対応(Print Job Processing Behavior対応) HCL-Vijay 2015/02/11 Modified START
	if(0 == strcmp(sc_PaparTrayMedia, "")) {	// If the media attribute is not specified by the client during the request.
		// media-default(用紙サイズ).
		st_MediaSize = Get_MediaSize( Judge_Media_Default() );
	} 
	else {							// If the (media-col->media-size) attribute is specified by the client during the request.
		st_MediaSize = Calc_MediaSize( sc_PaparTrayMedia );
	}
	// Helios KRQ-93 AirPrint1.5対応(Print Job Processing Behavior対応) HCL-Vijay 2015/02/11 Modified START
	
	pc_MediaCol->set = 1;

	TYPS_MediaCol s_MediaCol;
	memset( &s_MediaCol, NULL, sizeof(s_MediaCol) );
	s_MediaCol = pc_Job->getJobSpec()->pc_IPP->c_JobTempAttr.getMediaCol();

	pc_MediaCol->Media_Col_Botton_Margin = s_MediaCol.ul_MediaBottomoMargin;
	pc_MediaCol->Media_Col_Top_Margin    = s_MediaCol.ul_MediaTopMargin;
	pc_MediaCol->Media_Col_Right_Margin  = s_MediaCol.ul_MediaRightMargin;
	pc_MediaCol->Media_Col_Left_Margin   = s_MediaCol.ul_MediaLeftMargin;

	strcpy( (char*)pc_MediaCol->Media_Col_Source, Get_MediaSource( CnvSpec_MediaSource( e_FeedTray ) ) );
	strcpy( (char*)pc_MediaCol->Media_Col_Type,   Get_MediaType( CnvSpec_MediaType( e_PaperType ) ) );

	pc_MediaCol->Media_Size[0].set            = st_MediaSize.set;
	// ZeusCD CQ386774 OPS1八代 2014/02/25 START.
	pc_MediaCol->Media_Size[0].X_Dimension[0] = st_MediaSize.X_Dimension[0];
	pc_MediaCol->Media_Size[0].Y_Dimension[0] = st_MediaSize.Y_Dimension[0];
	pc_MediaCol->Media_Size[0].X_Dimension[1] = st_MediaSize.X_Dimension[1];
	pc_MediaCol->Media_Size[0].Y_Dimension[1] = st_MediaSize.Y_Dimension[1];
	// ZeusCD CQ386774 OPS1八代 2014/02/25 END.

	pc_MediaCol->Media_Source_Properties.set = 0;
	pc_MediaCol->Media_Source_Properties.Media_Source_Feed_Direction   = FEED_ORIENTATION_END;
	pc_MediaCol->Media_Source_Properties.Media_Source_Feed_Orientation = ORIENT_END;

	return;
}

/*================================================================================================
［機能］	設定関数 media-col(DoneJob).
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2014/01/31 新規作成 CQ384241 ZeusCD UV-01249 AppleAirPrint.
================================================================================================*/
void IPPC_LibOpeGetJobs::MediaCol_Done( APIC_JobLog* pc_JobLog, IPP_Media_Col* pc_MediaCol )
{
	memset( pc_MediaCol, NULL, sizeof(IPP_Media_Col) );

	TYPE_FeedTray  e_FeedTray  = pc_JobLog->getFeedTray();
	TYPE_PaperType e_PaperType = pc_JobLog->getPaperType();

	IPPS_MediaSize st_MediaSize;
	memset( &st_MediaSize, NULL, sizeof(IPPS_MediaSize) );

	char sc_PaparTrayMedia[32+1];
	memset( sc_PaparTrayMedia , NULL , sizeof(sc_PaparTrayMedia) );
	pc_JobLog->getPaparTrayMedia( sc_PaparTrayMedia );
	// Helios KRQ-93 AirPrint1.5対応(Print Job Processing Behavior対応) HCL-Vijay 2015/02/11 Modified START
	if(0 == strcmp(sc_PaparTrayMedia, "")) {	// If the media attribute is not specified by the client during the request.
		// media-default(用紙サイズ).
		st_MediaSize = Get_MediaSize( Judge_Media_Default() );
	} 
	else {							// If the (media-col->media-size) attribute is specified by the client during the request.
		st_MediaSize = Calc_MediaSize( sc_PaparTrayMedia );
	}
	// Helios KRQ-93 AirPrint1.5対応(Print Job Processing Behavior対応) HCL-Vijay 2015/02/11 Modified END

	TYPS_MediaCol s_MediaCol;
	memset( &s_MediaCol, NULL, sizeof(s_MediaCol) );

	pc_MediaCol->set = 1;

	s_MediaCol = pc_JobLog->getMediaCol();

	pc_MediaCol->Media_Col_Botton_Margin = s_MediaCol.ul_MediaBottomoMargin;
	pc_MediaCol->Media_Col_Top_Margin    = s_MediaCol.ul_MediaTopMargin;
	pc_MediaCol->Media_Col_Right_Margin  = s_MediaCol.ul_MediaRightMargin;
	pc_MediaCol->Media_Col_Left_Margin   = s_MediaCol.ul_MediaLeftMargin;

	strcpy( (char*)pc_MediaCol->Media_Col_Source, Get_MediaSource( CnvSpec_MediaSource( e_FeedTray ) ) );
	strcpy( (char*)pc_MediaCol->Media_Col_Type,   Get_MediaType( CnvSpec_MediaType( e_PaperType ) ) );

	pc_MediaCol->Media_Size[0].set            = st_MediaSize.set;
	// ZeusCD CQ386774 OPS1八代 2014/02/25 START.
	pc_MediaCol->Media_Size[0].X_Dimension[0] = st_MediaSize.X_Dimension[0];
	pc_MediaCol->Media_Size[0].Y_Dimension[0] = st_MediaSize.Y_Dimension[0];
	pc_MediaCol->Media_Size[0].X_Dimension[1] = st_MediaSize.X_Dimension[1];
	pc_MediaCol->Media_Size[0].Y_Dimension[1] = st_MediaSize.Y_Dimension[1];
	// ZeusCD CQ386774 OPS1八代 2014/02/25 END.

	pc_MediaCol->Media_Source_Properties.set = 0;
	pc_MediaCol->Media_Source_Properties.Media_Source_Feed_Direction   = FEED_ORIENTATION_END;
	pc_MediaCol->Media_Source_Properties.Media_Source_Feed_Orientation = ORIENT_END;

	return;
}
/*================================================================================================
［機能］    設定関数 finishings-col(ActiveJob).
--------------------------------------------------------------------------------------------------
［引数］    
［戻値］    
--------------------------------------------------------------------------------------------------
[履歴]      Helios KRQ-93 AirPrint1.5対応 HCL-Sankar 2015/02/02 
================================================================================================*/
void IPPC_LibOpeGetJobs::Finishings_Col_Active(TYPC_IPPJobTempAttr* pc_JobTempAttr, IPP_Finishings_Col (&Finishings_Col)[MAX_IPP_FIN_COL] )
{
	memset(&Finishings_Col,0,sizeof(Finishings_Col));
	// CQ510579 KRQ-93対応 Sankar 2015/02/27 Modified Start.
	for( ushort i = 0; i < TYPD_MAX_IPP_FINISHINGS && i < MAX_IPP_FIN_COL; i++ ) {
		TYPE_IPPFinishings e_IPPFinishings = pc_JobTempAttr->getIPPFinishings( i+1 );
		if( TYPE_IPPFinishingsEnd == e_IPPFinishings ){
			break;
		}
		Finishings_Col[i].type = IPP_FIN_COL_END;
		strcpy( (char*)Finishings_Col[i].FinishingsTemplate, Get_FinishingsTemplate( e_IPPFinishings ) );
		if(0 != strcmp( (const char*)Finishings_Col[i].FinishingsTemplate,"")){
			Finishings_Col[i].set = 1;
		}
		
	}
	// CQ510579 KRQ-93対応 Sankar 2015/02/27 Modified End.
	return;
}
 
/*================================================================================================
［機能］    設定関数 finishings-col(DoneJob).
--------------------------------------------------------------------------------------------------
［引数］    
［戻値］    
--------------------------------------------------------------------------------------------------
[履歴]      Helios KRQ-93 AirPrint1.5対応 HCL-Sankar 2015/02/02 
================================================================================================*/
void IPPC_LibOpeGetJobs::Finishings_Col_Done(APIC_JobLog* pc_JobLog, IPP_Finishings_Col (&Finishings_Col)[MAX_IPP_FIN_COL] )
{
	memset(&Finishings_Col,0,sizeof(Finishings_Col));
	// CQ510579 KRQ-93対応 Sankar 2015/02/27 Modified Start.
	for( ushort i = 0; i < TYPD_MAX_IPP_FINISHINGS && i < MAX_IPP_FIN_COL; i++ ) {
		TYPE_IPPFinishings e_IPPFinishings = pc_JobLog->getIPPFinishings( i+1 );
		if( TYPE_IPPFinishingsEnd == e_IPPFinishings ) {
			break;
		}
		Finishings_Col[i].type = IPP_FIN_COL_END;
		strcpy( (char*)Finishings_Col[i].FinishingsTemplate, Get_FinishingsTemplate( e_IPPFinishings ) );
		if(0 != strcmp( (const char*)Finishings_Col[i].FinishingsTemplate,"")){
			Finishings_Col[i].set = 1;
		}
	}
	// CQ510579 KRQ-93対応 Sankar 2015/02/27 Modified End.
	return;
}
/*================================================================================================
［機能］	設定関数 finishings(ActiveJob).
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2014/01/31 新規作成 CQ384241 ZeusCD UV-01249 AppleAirPrint.
================================================================================================*/
void IPPC_LibOpeGetJobs::Finishings_Active( TYPC_IPPJobTempAttr* pc_JobTempAttr, IPP_FINISHINGS (&Finishings)[MAX_IPP_FINISHINGS] )
{
	for( int j = 0; j < MAX_IPP_FINISHINGS; j++ ) Finishings[j] = FIN_END;

	// CQ510579 KRQ-93対応 Sankar 2015/02/27 Modified Start.
	for( ushort i = 0; i < TYPD_MAX_IPP_FINISHINGS && i < MAX_IPP_FINISHINGS ; i++ ) {

		Finishings[i] = CnvSpec_finishings( pc_JobTempAttr->getIPPFinishings( i+1 ) );

		if( FIN_END == Finishings[i] ) break;	// 終端まで来たのなら途中で抜ける.
	}
	// CQ510579 KRQ-93対応 Sankar 2015/02/27 Modified End.
	return;
}


/*================================================================================================
［機能］	設定関数 finishings(DoneJob).
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2014/01/31 新規作成 CQ384241 ZeusCD UV-01249 AppleAirPrint.
================================================================================================*/
void IPPC_LibOpeGetJobs::Finishings_Done( APIC_JobLog* pc_JobLog, IPP_FINISHINGS (&Finishings)[MAX_IPP_FINISHINGS] )
{
	for( int j = 0; j < MAX_IPP_FINISHINGS; j++ ) Finishings[j] = FIN_END;

	// CQ510579 KRQ-93対応 Sankar 2015/02/27 Modified Start.
	for( ushort i = 0; i < TYPD_MAX_IPP_FINISHINGS && i < MAX_IPP_FINISHINGS; i++ ) {

		Finishings[i] = CnvSpec_finishings( pc_JobLog->getIPPFinishings( i+1 ) );

		if( FIN_END == Finishings[i] ) break;	// 終端まで来たのなら途中で抜ける.
	}
	// CQ510579 KRQ-93対応 Sankar 2015/02/27 Modified End.
	return;
}

/*================================================================================================
［機能］	設定関数 output-bin(ActiveJob/DoneJob).
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2014/01/31 新規作成 CQ384241 ZeusCD UV-01249 AppleAirPrint.
================================================================================================*/
IPP_OUTPUT_BIN IPPC_LibOpeGetJobs::OutputBin( TYPE_OutputTray e_OutputTray )
{
	IPP_OUTPUT_BIN e_Return = OUTPUT_BIN_END;

	IPPE_OutputBin	e_OutputBin = CnvSpec_OutputBin( e_OutputTray );

	switch( e_OutputBin ) {
		case IPPE_OutputBin_Top:	e_Return = TOP;				break;
		case IPPE_OutputBin_Middle: e_Return = MIDDLE;			break;
		case IPPE_OutputBin_Bottom: e_Return = BOTTOM;			break;
		case IPPE_OutputBin_Right:	e_Return = RIGHT;			break;		// ZeusS ZX0 機器構成変更_No.3対応 HCL-VIJAY 2015/06/22 Added.
#ifdef DEF_A4_Cus // VenusMLK先行開発 2OPS Yamaguchi START.
		case IPPE_OutputBin_FaceDown: e_Return = FACE_DOWN;			break;
#endif // VenusMLK先行開発 2OPS Yamaguchi END.
		default: 					e_Return = OUTPUT_BIN_END;	break;
	}

	return e_Return;
}

/*================================================================================================
［機能］	設定関数 multiple-document-handling(ActiveJob/DoneJob).
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2014/01/31 新規作成 CQ384241 ZeusCD UV-01249 AppleAirPrint.
================================================================================================*/
static struct IPPS_CnvTable_MultipleDocumentHandling {
	TYPE_IPPMultiDocHand 	e_IPPMultiDocHand;		// KM定義.
	IPP_MULTI_DOC_HAND		e_IPP_MULTI_DOC_HAND;	// IPP定義(NICFUM) - multiple-document-handling.
} st_CnvTable_MultipleDocumentHandling[] = {
	{ TYPE_IPPMultiDocHandSingleDoc, 	 			 SINGLE_DOC 			 },
	{ TYPE_IPPMultiDocHandSeparateDocUncollated, 	 SEPARATE_DOC_UNCOLLATED },
	{ TYPE_IPPMultiDocHandSeparateDocCollated, 	 	 SEPARATE_DOC_COLLATED 	 },
	{ TYPE_IPPMultiDocHandSingleDocNewSheet, 		 SINGLE_DOC_NEW_SHEET 	 },
	{ TYPE_IPPMultiDocHandNoSpecific,				 SEPARATE_DOC_COLLATED	 },			// [multiple-document-handling-default] value will be set if "multiple-document-handling" is not specified by the client.
	{ TYPE_IPPMultiDocHandEnd, 	 					 MULTI_DOC_HAND_END 	 },
};
IPP_MULTI_DOC_HAND IPPC_LibOpeGetJobs::MultipleDocumentHandling( TYPE_IPPMultiDocHand e_IPPMultiDocHand )
{
	IPP_MULTI_DOC_HAND e_Return = MULTI_DOC_HAND_END;

	for( ushort i = 0; i < sizeof( st_CnvTable_MultipleDocumentHandling ) / sizeof( IPPS_CnvTable_MultipleDocumentHandling ); i++ ) {
		if( e_IPPMultiDocHand == st_CnvTable_MultipleDocumentHandling[i].e_IPPMultiDocHand ) {
			e_Return = st_CnvTable_MultipleDocumentHandling[i].e_IPP_MULTI_DOC_HAND;
			break;
		}
	}

	return e_Return;
}


/*================================================================================================
［機能］	設定関数 orientation-requested(ActiveJob/DoneJob).
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2014/01/31 新規作成 CQ384241 ZeusCD UV-01249 AppleAirPrint.
================================================================================================*/
static struct IPPS_CnvTable_OrientationRequested {
	TYPE_IPPOrientation 	e_IPPOrientation;		// KM定義.
	IPP_ORIENTATION			e_ORIENTATION;			// IPP定義(NICFUM) - orientation-requested.
} st_CnvTable_OrientationRequested[] = {
	{ TYPE_IPPOrientationPortrait, 	 		ORIENT_PORTRAIT 			},
	{ TYPE_IPPOrientationLandscape, 	 	ORIENT_LANDSCAPE 			},
	{ TYPE_IPPOrientationReserveLandscape, 	ORIENT_REVERSE_LANDSCAPE  	},
	{ TYPE_IPPOrientationReservePortrait, 	ORIENT_REVERSE_PORTRAIT	 	},
	{ TYPE_IPPOrientationNone, 				ORIENT_REVERSE_NONE	 		},	// ZeusCD CQ386774 OPS1八代 2014/02/25.
	{ TYPE_IPPOrientationNoSpecific,		ORIENT_PORTRAIT				},	// [orientation-requested-default] value will be set if "orientation-requested" is not specified by the client.
	{ TYPE_IPPOrientationEnd, 	 			ORIENT_END 	 				},
};
IPP_ORIENTATION IPPC_LibOpeGetJobs::OrientationRequested( TYPE_IPPOrientation e_IPPOrientation )
{
	IPP_ORIENTATION e_Return = ORIENT_END;

	for( ushort i = 0; i < sizeof( st_CnvTable_OrientationRequested ) / sizeof( IPPS_CnvTable_OrientationRequested ); i++ ) {
		if( e_IPPOrientation == st_CnvTable_OrientationRequested[i].e_IPPOrientation ) {
			e_Return = st_CnvTable_OrientationRequested[i].e_ORIENTATION;
			break;
		}
	}

	return e_Return;
}

/*================================================================================================
［機能］	設定関数 print-color-mode(ActiveJob/DoneJob).
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2014/01/31 新規作成 CQ384241 ZeusCD UV-01249 AppleAirPrint.
================================================================================================*/
static struct IPPS_CnvTable_PrintColorMode {
	TYPE_PrintColorMode 	e_PrintColorMode;	// KM定義.
	IPP_PRINT_COLOR_MODE	e_PRINT_COLOR_MODE;	// IPP定義(NICFUM) - print-color-mode.
} st_CnvTable_PrintColorMode[] = {
	{ TYPE_PrintColorModeAuto, 				CM_AUTO 				}, 
	{ TYPE_PrintColorModeBiLevel, 	 		CM_BI_LEVEL 			},
	{ TYPE_PrintColorModeColor, 			CM_COLOR  				},
	{ TYPE_PrintColorModeHighlight, 		CM_HIGHLIGHT			},
	{ TYPE_PrintColorModeMonochrome, 		CM_MONOCHROME			},
	{ TYPE_PrintColorModeProcessBiLevel, 	CM_PROCESS_BI_LEVEL		},
	{ TYPE_PrintColorModeProcessMonochrome, CM_PROCESS_MONOCHROME	},
	{ TYPE_PrintColorModeAutoMonochrome,	CM_AUTO_MONOCHROME		},	// ZeusCD CQ386774 OPS1八代 2014/02/25.
};
IPP_PRINT_COLOR_MODE IPPC_LibOpeGetJobs::PrintColorMode( TYPE_PrintColorMode e_PrintColorMode )
{
	IPP_PRINT_COLOR_MODE e_Return = IPP_PRINT_COLOR_MODE_END;

	for( ushort i = 0; i < sizeof( st_CnvTable_PrintColorMode ) / sizeof( IPPS_CnvTable_PrintColorMode ); i++ ) {
		if( e_PrintColorMode == st_CnvTable_PrintColorMode[i].e_PrintColorMode ) {
			e_Return = st_CnvTable_PrintColorMode[i].e_PRINT_COLOR_MODE;
			break;
		}
	}

	return e_Return;
}

/*================================================================================================
［機能］	設定関数 output-mode(ActiveJob/DoneJob).
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2014/11/20 新規作成 CQ506324 ZeusS_IT5_2nd
================================================================================================*/
static struct IPPS_CnvTable_OutputMode {
	TYPE_PrintColorMode e_PrintColorMode;		// Print-Color-Mode.
	IPP_OUTPUT_MODE	e_OUTPUT_MODE;				// Print-Color-Mode - Output-mode.
} st_CnvTable_OutputMode[] = {
	{ TYPE_PrintColorModeAuto, 				OUTPUT_MODE_AUTO				}, 
	{ TYPE_PrintColorModeBiLevel, 	 		OUTPUT_MODE_BI_LEVEL 			},
	{ TYPE_PrintColorModeColor, 			OUTPUT_MODE_COLOR  				},
	{ TYPE_PrintColorModeHighlight, 		OUTPUT_MODE_HIGHLIGHT			},
	{ TYPE_PrintColorModeMonochrome, 		OUTPUT_MODE_MONOCHROME			},
	{ TYPE_PrintColorModeProcessBiLevel, 	OUTPUT_MODE_PROCESS_BI_LEVEL	},
	{ TYPE_PrintColorModeProcessMonochrome, OUTPUT_MODE_PROCESS_MONOCHROME	},
	{ TYPE_PrintColorModeAutoMonochrome,	OUTPUT_MODE_AUTO_MONOCHROME		},
};
IPP_OUTPUT_MODE IPPC_LibOpeGetJobs::OutputMode( TYPE_PrintColorMode e_PrintColorMode )
{
	IPP_OUTPUT_MODE e_Return = IPP_OUTPUT_MODE_END;

	for( ushort i = 0; i < sizeof( st_CnvTable_OutputMode ) / sizeof( IPPS_CnvTable_OutputMode ); i++ ) {
		if( e_PrintColorMode == st_CnvTable_OutputMode[i].e_PrintColorMode ) {
			e_Return = st_CnvTable_OutputMode[i].e_OUTPUT_MODE;
			break;
		}
	}

	return e_Return;
}

/*================================================================================================
［機能］	設定関数 print-content-optimize(ActiveJob/DoneJob).
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2014/01/31 新規作成 CQ384241 ZeusCD UV-01249 AppleAirPrint.
================================================================================================*/
static struct IPPS_CnvTable_PrintContentOptimize {
	TYPE_OriginalImageType 		e_OriginalImageType;		// KM定義.
	IPP_PRINT_CONTENT_OPTIMIZE	e_PRINT_CONTENT_OPTIMIZE;	// IPP定義(NICFUM) - print-content-optimize.
} st_CnvTable_PrintContentOptimize[] = {
	{ TYPE_OriginalImageTypeDocument, 	OPT_AUTO 				}, // 'auto'は、KM定義上は「文書」として扱う.
	{ TYPE_OriginalImageTypePhoto, 	 	OPT_PHOTO 				},
	{ TYPE_OriginalImageTypeDTP, 		IPP_PRINT_CONTENT_END  	},
	{ TYPE_OriginalImageTypeWeb, 		IPP_PRINT_CONTENT_END	},
	{ TYPE_OriginalImageTypeCAD, 		IPP_PRINT_CONTENT_END 	},
};
IPP_PRINT_CONTENT_OPTIMIZE IPPC_LibOpeGetJobs::PrintContentOptimize( TYPE_OriginalImageType e_OriginalImageType )
{
	IPP_PRINT_CONTENT_OPTIMIZE e_Return = IPP_PRINT_CONTENT_END;

	for( ushort i = 0; i < sizeof( st_CnvTable_PrintContentOptimize ) / sizeof( IPPS_CnvTable_PrintContentOptimize ); i++ ) {
		if( e_OriginalImageType == st_CnvTable_PrintContentOptimize[i].e_OriginalImageType ) {
			e_Return = st_CnvTable_PrintContentOptimize[i].e_PRINT_CONTENT_OPTIMIZE;
			break;
		}
	}

	return e_Return;
}

/*================================================================================================
［機能］	設定関数 print-quality(ActiveJob/DoneJob).
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2014/01/31 新規作成 CQ384241 ZeusCD UV-01249 AppleAirPrint.
================================================================================================*/
static struct IPPS_CnvTable_PrintQuality {
	TYPE_IPPQuality e_IPPQuality;	// KM定義.
	IPP_QUALITY		e_QUALITY;		// IPP定義(NICFUM) - print-quality.
} st_CnvTable_PrintQuality[] = {
	{ TYPE_IPPQualityDraft, 	QTY_DRAFT 	},
	{ TYPE_IPPQualityNormal, 	QTY_NORMAL 	},
	{ TYPE_IPPQualityHigh, 		QTY_HIGH  	},
//	{ , 						QTY_PHOTO  	}, NICFUMに、不明な定義が。対象外なので、ひとまず無視.
	{ TYPE_IPPQualityEnd, 		QTY_END		},

};
IPP_QUALITY IPPC_LibOpeGetJobs::PrintQuality( TYPE_IPPQuality e_IPPQuality )
{
	IPP_QUALITY e_Return = QTY_END;

	for( ushort i = 0; i < sizeof( st_CnvTable_PrintQuality ) / sizeof( IPPS_CnvTable_PrintQuality ); i++ ) {
		if( e_IPPQuality == st_CnvTable_PrintQuality[i].e_IPPQuality ) {
			e_Return = st_CnvTable_PrintQuality[i].e_QUALITY;
			break;
		}
	}

	return e_Return;
}

/*================================================================================================
［機能］	設定関数 print-scaling(ActiveJob/DoneJob).
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2014/01/31 新規作成 CQ384241 ZeusCD UV-01249 AppleAirPrint.
================================================================================================*/
static struct IPPS_CnvTable_PrintScaling {
	TYPE_PrintScaling 			e_PrintScaling;				// KM定義.
	IPP_PRINTER_SCALING_ATTRS	e_PRINTER_SCALING_ATTRS;	// IPP定義(NICFUM) - print-scaling.
} st_CnvTable_PrintScaling[] = {
	{ TYPE_PrintScalingNon, 	   IPP_ATTR_PRN_SCL_NONE 		},
	{ TYPE_PrintScalingAuto, 	   IPP_ATTR_PRN_SCL_AUTO 		},
	{ TYPE_PrintScalingAutoFit,    IPP_ATTR_PRN_SCL_AUTO_FIT  	},
	{ TYPE_PrintScalingFill, 	   IPP_ATTR_PRN_SCL_FILL		},
	{ TYPE_PrintScalingFit, 	   IPP_ATTR_PRN_SCL_FIT			},
	{ TYPE_PrintScalingNoSpecific, IPP_ATTR_PRN_SCL_AUTO		},	// [print-scaling-default] value will be set if "print-scaling" is not specified by the client.
	{ TYPE_PrintScalingEnd,		   IPP_ATTR_PRN_SCA_END			},
};
IPP_PRINTER_SCALING_ATTRS IPPC_LibOpeGetJobs::PrintScaling( TYPE_PrintScaling  e_PrintScaling )
{
	IPP_PRINTER_SCALING_ATTRS e_Return = IPP_ATTR_PRN_SCA_END;

	for( ushort i = 0; i < sizeof( st_CnvTable_PrintScaling ) / sizeof( IPPS_CnvTable_PrintScaling ); i++ ) {
		if( e_PrintScaling == st_CnvTable_PrintScaling[i].e_PrintScaling ) {
			e_Return = st_CnvTable_PrintScaling[i].e_PRINTER_SCALING_ATTRS;
			break;
		}
	}

	return e_Return;
}

/*================================================================================================
［機能］	設定関数 printer-resolution(ActiveJob/DoneJob).
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2014/01/31 新規作成 CQ384241 ZeusCD UV-01249 AppleAirPrint.
================================================================================================*/
static struct IPPS_CnvTable_PrinterResolution {
	TYPE_IPPResolution 		e_IPPResolution;	// KM定義.
	MIO_IPP_RESOLUTION		e_IPP_RESOLUTION;	// IPP定義(NICFUM) - printer-resolution.
} st_CnvTable_PrinterResolution[] = {
	{ TYPE_IPPResolution300_300_3, 		RES_300_300_3 	},
	{ TYPE_IPPResolution600_600_3, 		RES_600_600_3 	},
	{ TYPE_IPPResolution1200_1200_3, 	RES_1200_1200_3 },
	{ TYPE_IPPResolution1200_600_3, 	RES_1200_600_3	},
	{ TYPE_IPPResolution600_1200_3, 	RES_600_1200_3	},
	{ TYPE_IPPResolutionEnd, 			RES_END			},

};
MIO_IPP_RESOLUTION IPPC_LibOpeGetJobs::PrinterResolution( TYPE_IPPResolution e_IPPResolution )
{
	MIO_IPP_RESOLUTION e_Return = RES_END;

	for( ushort i = 0; i < sizeof( st_CnvTable_PrinterResolution ) / sizeof( IPPS_CnvTable_PrinterResolution ); i++ ) {
		if( e_IPPResolution == st_CnvTable_PrinterResolution[i].e_IPPResolution ) {
			e_Return = st_CnvTable_PrinterResolution[i].e_IPP_RESOLUTION;
			break;
		}
	}

	return e_Return;
}

/*================================================================================================
［機能］	設定関数 sides(ActiveJob/DoneJob).
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2014/01/31 新規作成 CQ384241 ZeusCD UV-01249 AppleAirPrint.
================================================================================================*/
static struct IPPS_CnvTable_Sides {
	TYPE_IPPSides 	e_IPPSides;	// KM定義.
	IPP_SIDES		e_SIDES;	// IPP定義(NICFUM) - sides.
} st_CnvTable_Sides[] = {
	{ TYPE_IPPSidesOneSided, 			MIO_IPP_ONE_SIDED 				},
	{ TYPE_IPPSidesTwoSidedLongEdge, 	MIO_IPP_TWO_SIDED_LONG_EDGE 	},
	{ TYPE_IPPSidesTwoSidedShortEdge, 	MIO_IPP_TWO_SIDED_SHORT_EDGE 	},
	{ TYPE_IPPSidesEnd, 				MIO_IPP_SIDES_END				},
};
IPP_SIDES IPPC_LibOpeGetJobs::Sides( TYPE_IPPSides 	e_IPPSides )
{
	IPP_SIDES e_Return = MIO_IPP_SIDES_END;

	for( ushort i = 0; i < sizeof( st_CnvTable_Sides ) / sizeof( IPPS_CnvTable_Sides ); i++ ) {
		if( e_IPPSides == st_CnvTable_Sides[i].e_IPPSides ) {
			e_Return = st_CnvTable_Sides[i].e_SIDES;
			break;
		}
	}

	return e_Return;
}

/*================================================================================================
［機能］	設定関数 compression-supplied(ActiveJob/DoneJob).
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2014/05/31 新規作成 UV1429 Airprint-1.4 Support-HCL-VIJAY.
================================================================================================*/
static struct IPPS_CnvTable_Compression {
	TYPE_IPPCompress 	e_IPPCompress;	// KM定義.
	IPP_COMPRESS 		e_COMPRESS;	// IPP定義(NICFUM) - Compression-Supplied.COMP_NONE,
} st_CnvTable_Compression[] = {
	{ TYPE_IPPCompressNone, 			COMP_NONE				},
	{ TYPE_IPPCompressDeflate, 			COMP_DEFLATE			},
	{ TYPE_IPPCompressGZIP, 			COMP_GZIP 				},
	{ TYPE_IPPCompressCompress, 		COMP_COMPRESS 			},
	{ TYPE_IPPCompressEnd, 				COMP_END				},
};
IPP_COMPRESS IPPC_LibOpeGetJobs::CompressionSupplied( TYPE_IPPCompress 	e_IPPCompress )
{
	IPP_COMPRESS e_Return = COMP_END;

	for( ushort i = 0; i < sizeof( st_CnvTable_Compression ) / sizeof( IPPS_CnvTable_Compression ); i++ ) {
		if( e_IPPCompress == st_CnvTable_Compression[i].e_IPPCompress ) {
			e_Return = st_CnvTable_Compression[i].e_COMPRESS;
			break;
		}
	}

	return e_Return;
}

/*================================================================================================
［機能］	設定関数 document-format-supplied(ActiveJob/DoneJob).
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2014/05/31 新規作成 UV1429 Airprint-1.4 Support-HCL-VIJAY.
================================================================================================*/
static struct IPPS_CnvTable_DocumentFormat {
	TYPE_IPPDocFormat 	e_IPPDocFormat;	// KM定義.
	IPP_DOC_FORMAT 		e_DocFormat;	// IPP定義(NICFUM) - Compression-Supplied.COMP_NONE,
} st_CnvTable_DocumentFormat[] = {
	{ TYPE_IPPDocFormatTextHtml, 				DOC_TEXT_HTML				},
	{ TYPE_IPPDocFormatTextPlain, 				DOC_TEXT_PLAIN				},
	{ TYPE_IPPDocFormatTextPlainUSASCII,	 	DOC_TEXT_PLAIN_US_ASCII 	},
	{ TYPE_IPPDocFormatTextPlainISO8859, 		DOC_TEXT_PLAIN_ISO8859 		},
	{ TYPE_IPPDocFormatTextPlainUTF8, 			DOC_TEXT_PLAIN_UTF8			},
	{ TYPE_IPPDocFormatAPPPS, 					DOC_APP_PS					},
	{ TYPE_IPPDocFormatAPPPCL, 					DOC_APP_PCL					},
	{ TYPE_IPPDocFormatAPPPDF, 					DOC_APP_PDF 				},
	{ TYPE_IPPDocFormatAPPOctectStream, 		DOC_APP_OCTET_STREAM 		},
	{ TYPE_IPPDocFormatIMGURF, 					DOC_IMAGE_URF				},
	{ TYPE_IPPDocFormatIMGJPEG, 				DOC_IMAGE_JPEG 				},
	{ TYPE_IPPDocFormatEnd, 					DOC_END						},
};
IPP_DOC_FORMAT IPPC_LibOpeGetJobs::DocumentFormatSupplied( TYPE_IPPDocFormat 	e_IPPDocFormat )
{
	IPP_DOC_FORMAT e_Return = DOC_END;

	for( ushort i = 0; i < sizeof( st_CnvTable_DocumentFormat ) / sizeof( IPPS_CnvTable_DocumentFormat ); i++ ) {
		if( e_IPPDocFormat == st_CnvTable_DocumentFormat[i].e_IPPDocFormat ) {
			e_Return = st_CnvTable_DocumentFormat[i].e_DocFormat;
			break;
		}
	}

	return e_Return;
}
/*================================================================================================
［機能］	設定関数 number-up(ActiveJob/DoneJob).
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2015/12/14  Icarus Airprint-1.6対応 HCL-Sankar added.
================================================================================================*/
static struct IPPS_CnvTable_NumberUp {
	TYPE_IPPNumberUp 	e_IPPNumberUp;	// KM定義.
	IPP_NUMBER_UP 		e_NumberUp;		// IPP定義(NICFUM).
} st_CnvTable_NumberUp[] = {
	{ TYPE_NumberUp1, 				NUP1	},
	{ TYPE_NumberUp2, 				NUP2	},
	{ TYPE_NumberUp4,	 			NUP4 	},
	{ TYPE_NumberUp6, 				NUP6 	},
	{ TYPE_NumberUp9, 				NUP9	},
	{ TYPE_NumberUp16, 				NUP16	},
	{ TYPE_NumberUpEnd, 			NUP_END	},

};
IPP_NUMBER_UP IPPC_LibOpeGetJobs::NumberUp( TYPE_IPPNumberUp 	e_IPPNumberUp )
{
	IPP_NUMBER_UP e_Return = NUP_END;

	for( ushort i = 0; i < sizeof( st_CnvTable_NumberUp ) / sizeof( IPPS_CnvTable_NumberUp ); i++ ) {
		if( e_IPPNumberUp == st_CnvTable_NumberUp[i].e_IPPNumberUp ) {
			e_Return = st_CnvTable_NumberUp[i].e_NumberUp;
			break;
		}
	}

	return e_Return;
}

/*================================================================================================
［機能］	設定関数 Destination_Uris_Active(ActiveJob).
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     UV1429 AirPrint-1.4 Support
================================================================================================*/
void IPPC_LibOpeGetJobs::Destination_Uris_Active( APIC_Job* pc_Job, IPP_Destination_Uris (&ps_Destination)[MAX_DESTINATION_URIS])
{
	ushort us_AddrCnt = 0;
	char   sc_Dialnum[MAX_STRING64];
	char   sc_temp[MAX_STRING64];
	uchar   uc_PreDialStringLen=0;// ADD UV-01429 AirPrint 1.4 pre-dial-string対応  HCL-Sankar
	uchar   uc_DestinationLen = 0;
	memset( ps_Destination, NULL , sizeof(ps_Destination) );
	
	us_AddrCnt = pc_Job->getJobSpec()->pc_Send->c_ComSend.c_DirectAddressBook.getSelectCount();	// Getting no of destination	
	
	for( ushort i = 0; i < us_AddrCnt; i++ )
	{
		memset( sc_Dialnum, 0 , sizeof(sc_Dialnum) );
		memset( sc_temp, 0 , sizeof(sc_temp) );
		TYPC_DirectFaxAddress* pc_DirectFaxAddress = pc_Job->getJobSpec()->pc_Send->c_ComSend.c_DirectAddressBook.pageFaxAddress( i+1 );
		// G3Faxの場合
		if( NULL != pc_DirectFaxAddress ) {
			if( pc_DirectFaxAddress->getSendMode() == TYPE_SendModeFax ) {
				if( pc_DirectFaxAddress->getOn() == True ) {
					ps_Destination[i].set = 1;
					ps_Destination[i].T33_Subaddress = -1;//CQ513254 HCL-Sankar 2015/05/04 Added.
					strcpy((char*)sc_Dialnum, pc_DirectFaxAddress->getFaxAddress().sc_DialNo);
					// UV-01429 AirPrint 1.4 pre-dial-string対応 added HCL-Sankar START 
					uc_PreDialStringLen = (uchar)pc_Job->getJobSpec()->pc_IPP->c_JobTempAttr.getPreDialString(static_cast<unsigned char>(i+1));//getting the pre-dial-string length //CQ513254 HCL-Sankar 2015/04/29 Modified  
					uc_DestinationLen = static_cast<uchar>(strlen(sc_Dialnum));
					
					if((uc_PreDialStringLen > 0 ) && ( uc_DestinationLen > uc_PreDialStringLen )) {
						strncpy((char*)ps_Destination[i].Pre_Dial_String, sc_Dialnum, uc_PreDialStringLen);
						strncpy( sc_Dialnum, sc_Dialnum+uc_PreDialStringLen, uc_DestinationLen-uc_PreDialStringLen);
						sc_Dialnum[uc_DestinationLen-uc_PreDialStringLen]='\0';
					}
					
					// UV-01429 AirPrint 1.4 pre-dial-string対応 added HCL-Sankar END 
					if(True == pc_FaxPanelSetting->getPINNumNonDisplayOn())		// When Masking is On.
					{
						LIBF_ConvertPINStringToHiddenChar( reinterpret_cast<const char*>(sc_Dialnum), sc_temp, static_cast<unsigned short>(strlen(reinterpret_cast<const char*>(sc_Dialnum) ))); 
						strcat((char*)ps_Destination[i].Destination_Uri, "tel:");
						strncat((char*)ps_Destination[i].Destination_Uri, sc_temp, sizeof(ps_Destination[i].Destination_Uri)-5); //Poseidon IPP Static Analysis HCL-Vaibhav 2014/10/14.
					} else {													// When Masking is Off
						strcat((char*)ps_Destination[i].Destination_Uri, "tel:");
						strncat((char*)ps_Destination[i].Destination_Uri, sc_Dialnum, sizeof(ps_Destination[i].Destination_Uri)-5);	//Poseidon IPP Static Analysis HCL-Vaibhav 2014/10/14.
					}
				}
			}
		}
	}
	return;
}


/*================================================================================================
［機能］	CallBack function for getting the Multiple Destinations for Broadcast TX job.
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     UV1429 AirPrint-1.4 Support
================================================================================================*/

void ans_getFaxAddress( TYPC_SendLog c_SendLog )
{
	memset(g_scDialno, '\0', sizeof(g_scDialno));
    c_SendLog.getSendAddress(g_scDialno);
    // Release semaphore.
	SYSF_semGive( g_SemID );
}

/*================================================================================================
［機能］	設定関数 Destination_Uris_Done(DoneJob).
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     UV1429 AirPrint-1.4 Support
================================================================================================*/
void IPPC_LibOpeGetJobs::Destination_Uris_Done( APIC_JobLog* pc_JobLog, IPP_Destination_Uris (&ps_Destination)[MAX_DESTINATION_URIS] )
{
	char sc_DialAddr[MAX_STRING64];
	char sc_temp[MAX_STRING64];
	uchar uc_PreDialStringLen=0;// ADD UV-01429 AirPrint 1.4 pre-dial-string対応  HCL-Sankar 2014/12/05
	uchar uc_DestinationLen = 0;
	
	if((TYPE_SendModeFax == pc_JobLog->getSendMode()) && (True == pc_JobLog->checkDestinationTypeDirect())) {
		if( False == pc_JobLog->getAddressFlg() ) {
			memset( sc_DialAddr, '\0', sizeof(sc_DialAddr) );
			memset( sc_temp, '\0', sizeof(sc_temp)  );
			pc_JobLog->getDestNameUTF8( sc_temp );
			ps_Destination[0].set = 1;
			ps_Destination[0].T33_Subaddress = -1;//CQ513254 HCL-Sankar 2015/05/04 Added.
			// UV-01429 AirPrint 1.4 pre-dial-string対応 added HCL-Sankar START 
			uc_PreDialStringLen = (uchar)pc_JobLog->getPreDialString((unsigned char)1);//getting the pre-dial-string length //CQ513254 HCL-Sankar 2015/04/29 Modified
			uc_DestinationLen = static_cast<uchar>(strlen(sc_temp));   
			if((uc_PreDialStringLen > 0 ) && ( uc_DestinationLen > uc_PreDialStringLen )) {
				strncpy( (char* )ps_Destination[0].Pre_Dial_String, sc_temp, uc_PreDialStringLen);
				strncpy( sc_temp, sc_temp+uc_PreDialStringLen, uc_DestinationLen-uc_PreDialStringLen);
				sc_temp[uc_DestinationLen-uc_PreDialStringLen]='\0';
			}
			// UV-01429 AirPrint 1.4 pre-dial-string対応 added HCL-Sankar END 
			if(True == pc_FaxPanelSetting->getPINNumNonDisplayOn())		//When Masking is On.
			{
				LIBF_ConvertPINStringToHiddenChar( reinterpret_cast<const char*>(sc_temp), sc_DialAddr,  static_cast<unsigned short>(strlen(reinterpret_cast<const char*>(sc_temp) )));  
				strcpy((char*)ps_Destination[0].Destination_Uri, "tel:");
				strncat( (char*)ps_Destination[0].Destination_Uri, sc_DialAddr, sizeof(ps_Destination[0].Destination_Uri)-5 ); //Poseidon IPP Static Analysis HCL-Vaibhav 2014/10/14.
				
			} else {													//When Masking is Off
				strcpy((char*)ps_Destination[0].Destination_Uri, "tel:");
				strncat( (char*)ps_Destination[0].Destination_Uri, sc_temp, sizeof(ps_Destination[0].Destination_Uri)-5 ); //Poseidon IPP Static Analysis HCL-Vaibhav 2014/10/14.
			}
		} else {											// When Fax TX to Multiple Destination
			uint32_t ul_Jobid = 0;
			short ss_Jobnum = 0;
			short ss_Destcount = 0;
			APIC_DoneJobList *pc_DoneJobList = APIC_DoneJobList::newInstance();
			ul_Jobid = pc_JobLog->getJobID();
			ss_Jobnum = pc_DoneJobList->getSendLogJobNo(ul_Jobid);
			ss_Destcount = pc_DoneJobList->getSendLogAddressNum(ss_Jobnum);
			
			//Create Binary semaphore
			g_SemID = SYSF_semBCreate( SEM_Q_PRIORITY, SEM_EMPTY );
			ushort i=0;
			while(i < ss_Destcount)
			{
				memset( sc_DialAddr, '\0', sizeof(sc_DialAddr) );
				memset( sc_temp, '\0', sizeof(sc_temp)  );
				pc_DoneJobList->getSendLog(ss_Jobnum, (i+1), ans_getFaxAddress );
				ps_Destination[i].set = 1;
				ps_Destination[i].T33_Subaddress = -1;//CQ513254 HCL-Sankar 2015/05/04 Added.
				// Acquire semaphore. Wait till data obtained from callback function ans_getFaxAddress.
				//	[AR : 1026431] 【 Eagle 】【静的解析対応用課題】 2018/02/09 [HCL_Ramkisson] Mod Start.
				if(g_SemID != NULL)
				{
					SYSF_semTake( g_SemID, WAIT_FOREVER );
				}
				//	[AR : 1026431] 【 Eagle 】【静的解析対応用課題】 2018/02/09 [HCL_Ramkisson] Mod Start.
				// UV-01429 AirPrint 1.4 pre-dial-string対応 added HCL-Sankar START 
				uc_PreDialStringLen = (uchar)pc_JobLog->getPreDialString(static_cast<unsigned char>(i+1));//getting the pre-dial-string length //CQ513254 HCL-Sankar 2015/04/29 Modified  
				uc_DestinationLen = static_cast<uchar>(strlen(g_scDialno));
				if((uc_PreDialStringLen > 0 ) && ( uc_DestinationLen > uc_PreDialStringLen )) {
					strncpy((char *) ps_Destination[i].Pre_Dial_String, g_scDialno, uc_PreDialStringLen);
					strncpy( g_scDialno, g_scDialno+uc_PreDialStringLen, uc_DestinationLen-uc_PreDialStringLen);
					g_scDialno[uc_DestinationLen-uc_PreDialStringLen]='\0';
				}			
				// UV-01429 AirPrint 1.4 pre-dial-string対応 added HCL-Sankar END 
				if(False == pc_FaxPanelSetting->getPINNumNonDisplayOn())		// When Masking is Off.
				{
					strcat((char*)ps_Destination[i].Destination_Uri, "tel:");
					strncat((char*)ps_Destination[i].Destination_Uri, g_scDialno, sizeof(ps_Destination[i].Destination_Uri)-5); //Poseidon IPP Static Analysis HCL-Vaibhav 2014/10/14.
				} else {													   // When Masking is On.	
					strcat((char*)ps_Destination[i].Destination_Uri, "tel:");
					LIBF_ConvertPINStringToHiddenChar( reinterpret_cast<const char*>(g_scDialno), sc_DialAddr, static_cast<unsigned short>(strlen(reinterpret_cast<const char*>(g_scDialno) )));
					strncat((char*)ps_Destination[i].Destination_Uri, sc_DialAddr, sizeof(ps_Destination[i].Destination_Uri)-5); //Poseidon IPP Static Analysis HCL-Vaibhav 2014/10/14.
				}
				i++;
			}
		}
		SYSF_semDelete(g_SemID);
	}
	return;
}

/*================================================================================================
［機能］	設定関数 overrides(ActiveJob).
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
[履歴]     2015/02/04 新規作成 HeliosL Airprint-1.5 Support
================================================================================================*/
void IPPC_LibOpeGetJobs::Overrides_Active( TYPC_IPPJobTempAttr* pc_JobTempAttr, IPP_Overrides (&Overrides)[MAX_IPP_OVERRIDES] )
{
	TYPS_Overrides s_Overrides;
	IPPS_MediaSize st_MediaSize;
	
	for(uchar uc_Count = 0; uc_Count < MAX_IPP_OVERRIDES; uc_Count++ ) {
		memset( &s_Overrides, NULL, sizeof(TYPS_Overrides) );
		memset( &st_MediaSize, NULL, sizeof(IPPS_MediaSize) );
		s_Overrides = pc_JobTempAttr->getOverrides( uc_Count+1 );
		if( 0 == strcmp(s_Overrides.sc_PageRange, "") ) {		//Client hasn't specified overrides for the index "uc_count"
		    Overrides[uc_Count].set = 0;
			break;
		}
		else {										//Client has specified overrides for the index uc_count

			Overrides[uc_Count].set = 1;			// Set Flag value that client has specified overrides in the job request.
			// pages
			strncpy((char *)Overrides[uc_Count].Pages, s_Overrides.sc_PageRange,sizeof( Overrides[uc_Count].Pages )-1);

			// sides
			Overrides[uc_Count].Sides = Sides( s_Overrides.e_Sides );

			// media
			strcpy((char*)Overrides[uc_Count].Media , s_Overrides.sc_PaparTrayMedia );

			//media-col
			// CQ511074 KRQ-93対応 HCL-Sankar 2015/03/17 Added Start.
			if (s_Overrides.e_FeedTray == TYPE_FeedTrayUnselect 	&&
				s_Overrides.e_PaperType == TYPE_PaperTypeUnknown 	&&
	 			s_Overrides.s_MediaCol.ul_MediaBottomoMargin == 0 	&&
				s_Overrides.s_MediaCol.ul_MediaTopMargin 	 == 0 	&&
		 		s_Overrides.s_MediaCol.ul_MediaRightMargin 	 == 0 	&&
		   		s_Overrides.s_MediaCol.ul_MediaLeftMargin    == 0	&&
				s_Overrides.s_MediaCol.s_MediaSize.ul_X_Dimension == 0 && 
				s_Overrides.s_MediaCol.s_MediaSize.ul_Y_Dimension == 0 ) {
				Overrides[uc_Count].Media_Col.set = 0;
			 } else {
		 		Overrides[uc_Count].Media_Col.set = 1;
				//media-col-->media-source
				strcpy( (char*)Overrides[uc_Count].Media_Col.Media_Col_Source, Get_MediaSource( CnvSpec_MediaSource( s_Overrides.e_FeedTray ) ) );

				//media-col-->media-type
				strcpy( (char*)Overrides[uc_Count].Media_Col.Media_Col_Type, Get_MediaType( CnvSpec_MediaType( s_Overrides.e_PaperType ) ) );

				//media-col-->media-source-properties
				Overrides[uc_Count].Media_Col.Media_Source_Properties.set=0;
				Overrides[uc_Count].Media_Col.Media_Source_Properties.Media_Source_Feed_Direction   = FEED_ORIENTATION_END;
				Overrides[uc_Count].Media_Col.Media_Source_Properties.Media_Source_Feed_Orientation = ORIENT_END;

				//media-col-->media-margin
				Overrides[uc_Count].Media_Col.Media_Col_Botton_Margin = s_Overrides.s_MediaCol.ul_MediaBottomoMargin;
				Overrides[uc_Count].Media_Col.Media_Col_Top_Margin    = s_Overrides.s_MediaCol.ul_MediaTopMargin;
				Overrides[uc_Count].Media_Col.Media_Col_Right_Margin  = s_Overrides.s_MediaCol.ul_MediaRightMargin;
				Overrides[uc_Count].Media_Col.Media_Col_Left_Margin   = s_Overrides.s_MediaCol.ul_MediaLeftMargin;

				//media-col-->media-size
				if(s_Overrides.s_MediaCol.s_MediaSize.ul_X_Dimension != 0 && s_Overrides.s_MediaCol.s_MediaSize.ul_Y_Dimension != 0) {
					Overrides[uc_Count].Media_Col.Media_Size[0].set = 1;
					Overrides[uc_Count].Media_Col.Media_Size[0].X_Dimension[0] = s_Overrides.s_MediaCol.s_MediaSize.ul_X_Dimension;
					Overrides[uc_Count].Media_Col.Media_Size[0].Y_Dimension[0] = s_Overrides.s_MediaCol.s_MediaSize.ul_Y_Dimension;
				} else {
					Overrides[uc_Count].Media_Col.Media_Size[0].set = 0;
				}
			}
			// CQ511074 KRQ-93対応 HCL-Sankar 2015/03/17 Added End.
			//finishings
			Overrides[uc_Count].Finishings[0] = CnvSpec_finishings(s_Overrides.e_FinishingsStaple);

			// CQ510581 KRQ-93対応 Bairavi 2015/02/27 Added Start.
			// For Not Supported Members of overrides collection XXX_END should be returned
			//job-sheets
			Overrides[uc_Count].Job_Sheets = SHEETS_END;
			
			//multiple-document-handling
			Overrides[uc_Count].Multi_Doc_Hand = MULTI_DOC_HAND_END;
			
			//number-up
			Overrides[uc_Count].Number_Up = NUP_END;
			
			//orientation-requested
			Overrides[uc_Count].Orientation = ORIENT_END;
			
			//output-bin
			Overrides[uc_Count].Output_Bin = OUTPUT_BIN_END;
			
			//print-quality
			Overrides[uc_Count].Quality = QTY_END;
			
			//printer-resolution
			Overrides[uc_Count].Resolution = RES_END;
			// CQ510581 KRQ-93対応 Bairavi 2015/02/27 Added End.
			
			//IT5_4.1 Airprint1.7対応 HCL_VIJAY 2017/2/16 Added Start.
			//document-numbers
			strncpy((char *)Overrides[uc_Count].Document_Numbers, s_Overrides.sc_DocNoRange,sizeof( Overrides[uc_Count].Document_Numbers)-1);
			//IT5_4.1 Airprint1.7対応 HCL_VIJAY 2017/2/16 Added End.
		}
	}
	return;
}

/*================================================================================================
［機能］	設定関数 overrides(DoneJob).
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
[履歴]     2015/02/04 新規作成 HeliosL Airprint-1.5 Support
================================================================================================*/
void IPPC_LibOpeGetJobs::Overrides_Done( APIC_JobLog* pc_JobLog, IPP_Overrides (&Overrides)[MAX_IPP_OVERRIDES] )
{
	TYPS_Overrides s_Overrides;
	IPPS_MediaSize st_MediaSize;
	
	for(uchar uc_Count = 0; uc_Count < MAX_IPP_OVERRIDES; uc_Count++ ) {
		memset( &s_Overrides, NULL, sizeof(TYPS_Overrides) );
		memset( &st_MediaSize, NULL, sizeof(IPPS_MediaSize) );
		s_Overrides = pc_JobLog->getOverrides( uc_Count+1 );
		if( 0 == strcmp(s_Overrides.sc_PageRange, "") ) {			//Client hasn't specified overrides for the index "uc_count"
			Overrides[uc_Count].set = 0;
			break;
		} else {										//Client has specified overrides for the index uc_count
			// Setting Flag for the scenario when overrides specified	by the client.
			Overrides[uc_Count].set = 1;
			
			// pages
			strncpy((char *)Overrides[uc_Count].Pages, s_Overrides.sc_PageRange,sizeof( Overrides[uc_Count].Pages )-1);

			// sides
			Overrides[uc_Count].Sides = Sides( s_Overrides.e_Sides );

			// media
			strcpy((char *)Overrides[uc_Count].Media , s_Overrides.sc_PaparTrayMedia );
			//media-col
			// CQ511074 KRQ-93対応 HCL-Sankar 2015/03/17 Added Start.
			if (s_Overrides.e_FeedTray == TYPE_FeedTrayUnselect 	&&
				s_Overrides.e_PaperType == TYPE_PaperTypeUnknown 	&&
	 			s_Overrides.s_MediaCol.ul_MediaBottomoMargin == 0 	&&
				s_Overrides.s_MediaCol.ul_MediaTopMargin 	 == 0 	&&
		 		s_Overrides.s_MediaCol.ul_MediaRightMargin 	 == 0 	&&
		   		s_Overrides.s_MediaCol.ul_MediaLeftMargin    == 0	&&
				s_Overrides.s_MediaCol.s_MediaSize.ul_X_Dimension == 0 && 
				s_Overrides.s_MediaCol.s_MediaSize.ul_Y_Dimension ==0 ) {
				Overrides[uc_Count].Media_Col.set = 0;
			 } else {
		 		Overrides[uc_Count].Media_Col.set = 1;
				//media-col-->media-source
				strcpy( (char*)Overrides[uc_Count].Media_Col.Media_Col_Source, Get_MediaSource( CnvSpec_MediaSource( s_Overrides.e_FeedTray ) ) );

				//media-col-->media-type
				strcpy( (char*)Overrides[uc_Count].Media_Col.Media_Col_Type,   Get_MediaType( CnvSpec_MediaType( s_Overrides.e_PaperType ) ) );

				//media-col-->media-source-properties
				Overrides[uc_Count].Media_Col.Media_Source_Properties.set=0;
				Overrides[uc_Count].Media_Col.Media_Source_Properties.Media_Source_Feed_Direction   = FEED_ORIENTATION_END;
				Overrides[uc_Count].Media_Col.Media_Source_Properties.Media_Source_Feed_Orientation = ORIENT_END;

				//media-col-->media-margin
				Overrides[uc_Count].Media_Col.Media_Col_Botton_Margin = s_Overrides.s_MediaCol.ul_MediaBottomoMargin;
				Overrides[uc_Count].Media_Col.Media_Col_Top_Margin    = s_Overrides.s_MediaCol.ul_MediaTopMargin;
				Overrides[uc_Count].Media_Col.Media_Col_Right_Margin  = s_Overrides.s_MediaCol.ul_MediaRightMargin;
				Overrides[uc_Count].Media_Col.Media_Col_Left_Margin   = s_Overrides.s_MediaCol.ul_MediaLeftMargin;

				//media-col-->media-size
				if((s_Overrides.s_MediaCol.s_MediaSize.ul_X_Dimension != 0 && s_Overrides.s_MediaCol.s_MediaSize.ul_Y_Dimension != 0)) {
					Overrides[uc_Count].Media_Col.Media_Size[0].set = 1;
					Overrides[uc_Count].Media_Col.Media_Size[0].X_Dimension[0] = s_Overrides.s_MediaCol.s_MediaSize.ul_X_Dimension;
					Overrides[uc_Count].Media_Col.Media_Size[0].Y_Dimension[0] = s_Overrides.s_MediaCol.s_MediaSize.ul_Y_Dimension;
				} else {
					Overrides[uc_Count].Media_Col.Media_Size[0].set = 0;
				}
			}
			// CQ511074 KRQ-93対応 HCL-Sankar 2015/03/17 Added End.
			//finishings
			Overrides[uc_Count].Finishings[0] = CnvSpec_finishings(s_Overrides.e_FinishingsStaple);
			// CQ510581 KRQ-93対応 Bairavi 2015/02/27 Added Start.
			// For Not Supported Members of overrides collection XXX_END should be returned
			//job-sheets
			Overrides[uc_Count].Job_Sheets = SHEETS_END;
			
			//multiple-document-handling
			Overrides[uc_Count].Multi_Doc_Hand = MULTI_DOC_HAND_END;
			
			//number-up
			Overrides[uc_Count].Number_Up = NUP_END;
			
			//orientation-requested
			Overrides[uc_Count].Orientation = ORIENT_END;
			
			//output-bin
			Overrides[uc_Count].Output_Bin = OUTPUT_BIN_END;
			
			//print-quality
			Overrides[uc_Count].Quality = QTY_END;
			
			//printer-resolution
			Overrides[uc_Count].Resolution = RES_END;
			// CQ510581 KRQ-93対応 Bairavi 2015/02/27 Added End.
			
			//IT5_4.1 Airprint1.7対応 HCL_VIJAY 2017/2/16 Added Start.
			//document-numbers
			strncpy((char *)Overrides[uc_Count].Document_Numbers, s_Overrides.sc_DocNoRange,sizeof( Overrides[uc_Count].Document_Numbers)-1);
			//IT5_4.1 Airprint1.7対応 HCL_VIJAY 2017/2/16 Added End.
		}
	}
	return;
}

/*================================================================================================
［機能］	IPPC_LibOpeGetPrintAttrクラスのインスタンス取得
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2014/01/31 新規作成 CQ384241 ZeusCD UV-01249 AppleAirPrint.
================================================================================================*/
IPPC_LibOpeGetPrintAttr* IPPC_LibOpeGetPrintAttr::newInstance()
{
	static IPPC_LibOpeGetPrintAttr c_LibOpeGetPrintAttr;
	return	&c_LibOpeGetPrintAttr;
}

/*================================================================================================
［機能］	設定関数 printer-state, printer-state-reasons.
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2014/01/31 新規作成 CQ384241 ZeusCD UV-01249 AppleAirPrint.
 					   従来の処理を関数化した.
			2015.02.13 新規作成 HeliosL KRQ-93 AirPrint1.5(WP14).
 					   IPPの定期起床タスクへ処理を移動.
================================================================================================*/
// HeliosL KRQ-93 AirPrint1.5(WP14) 2015.02.13 OPS1八代 START.
extern IPP_PRINTER_STATE g_e_PrinterState;
extern UINT32 g_i_PrinterStateReason[MAX_IPP_PRINTER_STATE_REASONS];
extern void IPP_CycSemLock();
extern void IPP_CycSemUnLock();
// HeliosL KRQ-93 AirPrint1.5(WP14) 2015.02.13 OPS1八代 END.
void IPPC_LibOpeGetPrintAttr::PrinterStateAndReasons( IPP_PRINTER_STATE (&Printer_State), UINT32 (&State_Reason)[MAX_IPP_PRINTER_STATE_REASONS] )
{
	// HeliosL KRQ-93 AirPrint1.5(WP14) 2015.02.13 OPS1八代 START.
	IPP_CycSemLock();
	IPPC_PrinterStateAndReasons::newInstance()->main();		//MinervaSBK CQ520623 HCL-Sankar 2015/10/28 Addded.
	Printer_State = g_e_PrinterState;
	for( int i = 0;	i < MAX_IPP_PRINTER_STATE_REASONS; i++ ) {
		State_Reason[i] = g_i_PrinterStateReason[i];
	}
	IPP_CycSemUnLock();
	// HeliosL KRQ-93 AirPrint1.5(WP14) 2015.02.13 OPS1八代 END.

	return;
}

// HeliosL KRQ-93 AirPrint1.5(WP14) 2015.02.13 OPS1八代 START.
/*================================================================================================
［機能］	設定関数  copies-default.
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2015.02.13 新規作成 HeliosL KRQ-93 AirPrint1.5(WP14).
            従来処理を関数化した.
================================================================================================*/
INT32 IPPC_LibOpeGetPrintAttr::CopiesDefault()
{
	pc_PrintJobSetting->lock();
	INT32 i_Copies = pc_PrintJobSetting->getNumOfCopies();
	pc_PrintJobSetting->unlock();

	return i_Copies;
}

/*================================================================================================
［機能］	設定関数  job-sheets-default.
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2015.02.13 新規作成 HeliosL KRQ-93 AirPrint1.5(WP14).
            従来処理を関数化した.
================================================================================================*/
IPP_JOB_SHEETS IPPC_LibOpeGetPrintAttr::JobSheetsDefault()
{
	IPP_JOB_SHEETS e_JobSheets = SHEETS_NONE;

	pc_PrintJobSetting->lock();
	Bool b_BannerPage = pc_PrintJobSetting->getBannerPageOn();
	pc_PrintJobSetting->unlock();

	if( b_BannerPage ){
		e_JobSheets = SHEETS_STANDARD;
	}else{
		e_JobSheets = SHEETS_NONE;
	}

	return e_JobSheets;
}

/*================================================================================================
［機能］	設定関数  confirmation-sheet-print-default.
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2015.02.13 新規作成 HeliosL KRQ-93 AirPrint1.5(WP14).
            従来処理を関数化した.
================================================================================================*/
BOOL IPPC_LibOpeGetPrintAttr::ConfirmationSheetPrintDefault()
{
	BOOL bl_CSPDef = FALSE;
	if( TYPE_CommRoportResultAll == pc_FaxListReportSetting->getCommRoportResult() ){
		bl_CSPDef = TRUE;
	}

	return bl_CSPDef;
}

/*================================================================================================
［機能］	設定関数  number-of-retries-default.
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2015.02.13 新規作成 HeliosL KRQ-93 AirPrint1.5(WP14).
            従来処理を関数化した.
================================================================================================*/
UINT32 IPPC_LibOpeGetPrintAttr::NumberOfRetriesDefault()
{
	pc_FaxJobSetting->lock();
	UINT32 i_NumberOfRetries = pc_FaxJobSetting->getBusyCount();
	pc_FaxJobSetting->unlock();

	return i_NumberOfRetries;
}

/*================================================================================================
［機能］	設定関数  retry-interval-default.
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2015.02.13 新規作成 HeliosL KRQ-93 AirPrint1.5(WP14).
            従来処理を関数化した.
================================================================================================*/
UINT32 IPPC_LibOpeGetPrintAttr::RetryIntervalDefault()
{
	pc_FaxJobSetting->lock();
	UINT32 i_RetryInterval = pc_FaxJobSetting->getReDialInterval();
	pc_FaxJobSetting->unlock();

	// Checking the value set is 0. Since for retry-interval the min value is 1 as per PWG spec.
	if(0 == i_RetryInterval) {
		i_RetryInterval = -2;	   //Not-Supported
	}

	return i_RetryInterval;
}

/*================================================================================================
［機能］	設定関数  retry-time-out-default.
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2015.02.13 新規作成 HeliosL KRQ-93 AirPrint1.5(WP14).
            従来処理を関数化した.
================================================================================================*/
UINT32 IPPC_LibOpeGetPrintAttr::RetryTimeOutDefault()
{
	pc_FaxGfpSetting->lock();
	UINT32 i_RetryTimeOut = pc_FaxGfpSetting->getAnswerWaitAll( TYPE_CommLine1 );
	pc_FaxGfpSetting->unlock();
	return i_RetryTimeOut;
}

/*================================================================================================
［機能］	設定関数  queued-job-count(実行中リストに入っているIPPジョブの数).
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2015.02.13 新規作成 HeliosL KRQ-93 AirPrint1.5(WP14).
            従来処理を関数化した.
================================================================================================*/
extern IPP_JOB_STATE getFaxOutJobStatus(TYPC_JobStatus *pJobStatus);
extern IPP_JOB_STATE IPP_GetJobState( TYPE_InputStatus e_InputStatus, TYPE_RIPStatus e_RipStatus, TYPE_OutputStatus e_OutputStatus );
#define IPPD_Max_JobListToMio2	50
UINT32 IPPC_LibOpeGetPrintAttr::QueuedJobCount()
{
	UINT32 i_QueuedJobCount = 0;

	LIBC_List<APIC_Job> c_JobList;

	pc_ActiveJobList->lock();
	short ss_ActiveJobAllNum = pc_ActiveJobList->getList( TYPE_ActiveJobListIpp, 1, IPPD_Max_ActiveJobListPrint, &c_JobList );
	pc_ActiveJobList->unlock();

	APIC_Job* pc_Job = NULL;
	for ( short ii = 0; ii < ss_ActiveJobAllNum; ii++ ) {
		if ( ii == 0 ) {
			// リストの先頭 
			pc_Job = c_JobList.findFirst();
		} else {
			// リストの次へ進む 
			pc_Job = c_JobList.findNext();
		}
		// 取得できなかったら次へ.
		if( NULL == pc_Job )
		{
			continue;
		}
		// セマフォのロック 
		pc_Job->lock();
		if ( i_QueuedJobCount < IPPD_Max_JobListToMio2 ) {

			IPP_JOB_STATE e_JobState = JSTATE_PENDING;
			if(TYPE_NetworkJobType_FaxOut == pc_Job->getJobSpec()->pc_Common->getNetworkJobType()) {
				e_JobState = getFaxOutJobStatus(pc_Job->getJobStatus());
			} else {
				TYPE_InputStatus e_InputStatus = pc_Job->getJobStatus()->getInputStatus();
				TYPE_OutputStatus e_OutputStatus = pc_Job->getJobStatus()->getOutputStatus();
				TYPE_RIPStatus e_RipStatus = pc_Job->getJobStatus()->getRipStatus();
				e_JobState = IPP_GetJobState( e_InputStatus, e_RipStatus, e_OutputStatus );
			}
			if( e_JobState == JSTATE_PENDING
			 || e_JobState == JSTATE_PENDING_HELD
			 || e_JobState == JSTATE_PROCESSING
			 || e_JobState == JSTATE_PROCESSING_STOPPED ) {
				i_QueuedJobCount++;
			}
		}
		pc_Job->unlock();
	}

	return i_QueuedJobCount;
}

/*================================================================================================
［機能］	設定関数  printer-name.
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2015.02.13 新規作成 HeliosL KRQ-93 AirPrint1.5(WP14).
            従来処理を関数化した.
================================================================================================*/
void IPPC_LibOpeGetPrintAttr::PrinterName( UCHAR (&Printer_Name)[MAX_STRING128], IPP_ATTR_CHARSET e_CharSet )
{
	pc_IppSetting->lock();
	pc_IppSetting->getIppPrtName( (char*)Printer_Name );
	pc_IppSetting->unlock();

	IPP_ConvertFromUTF8( (char*)( Printer_Name ), sizeof( Printer_Name ), e_CharSet );		// UTF8へ変換.

	return;
}

/*================================================================================================
［機能］	設定関数  printer-location.
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2015.02.13 新規作成 HeliosL KRQ-93 AirPrint1.5(WP14).
            従来処理を関数化した.
================================================================================================*/
void IPPC_LibOpeGetPrintAttr::PrinterLocation( UCHAR (&Printer_Location)[MAX_STRING128], IPP_ATTR_CHARSET e_CharSet )
{
	pc_IppSetting->lock();
	pc_IppSetting->getIppPrtLocation( (char*)Printer_Location );
	pc_IppSetting->unlock();

	IPP_ConvertFromUTF8( (char*)( Printer_Location ), sizeof( Printer_Location ), e_CharSet );		// UTF8へ変換.

	return;
}

/*================================================================================================
［機能］	設定関数  printer-driver-installer.
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2015.02.13 新規作成 HeliosL KRQ-93 AirPrint1.5(WP14).
            従来処理を関数化した.
================================================================================================*/
void IPPC_LibOpeGetPrintAttr::PrinterDriverInstaller( UCHAR (&Printer_Driver_URI)[MAX_STRING128] )
{
	pc_IppSetting->lock();
	pc_IppSetting->getIppPrinterDriverInstaller( (char*)Printer_Driver_URI );
	pc_IppSetting->unlock();
	return;
}

/*================================================================================================
［機能］	設定関数  printer-more-info-manufacturer.
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2015.02.13 新規作成 HeliosL KRQ-93 AirPrint1.5(WP14).
            従来処理を関数化した.
================================================================================================*/
void IPPC_LibOpeGetPrintAttr::PrinterMoreInfoManufacturer( UCHAR (&Printer_More_Info_Man)[MAX_STRING128] )
{
	pc_IppSetting->lock();
	pc_IppSetting->getIppPrinterMoreInfoManufacturer( (char*)Printer_More_Info_Man );
	pc_IppSetting->unlock();
	return;
}

/*================================================================================================
［機能］	設定関数  multiple-operation-time-out.
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2015.02.13 新規作成 HeliosL KRQ-93 AirPrint1.5(WP14).
            従来処理を関数化した.
================================================================================================*/
INT32 IPPC_LibOpeGetPrintAttr::MultipleOperationTimeOut()
{
	INT32 i_MultipleOperationTimeout = pc_AirPrintSetting->getAirPrintTimeout();
	return i_MultipleOperationTimeout;
}

/*================================================================================================
［機能］	設定関数  printer-fax-log-uri.
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2015.02.13 新規作成 HeliosL KRQ-93 AirPrint1.5(WP14).
            従来処理を関数化した.
================================================================================================*/
void IPPC_LibOpeGetPrintAttr::PrinterFaxLogUri( UCHAR (&Printer_Fax_Log_Uri)[MAX_STRING128] )
{
	pc_AirPrintSetting->getFaxoutPrinterLogUri( (char*)Printer_Fax_Log_Uri );
	return;
}

/*================================================================================================
［機能］	設定関数  printer-organization.
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2015.02.13 新規作成 HeliosL KRQ-93 AirPrint1.5(WP14).
            従来処理を関数化した.
			2017.03.06 Modified IT5-4.1(CQ1014723) Printer_Organization String length is increased
			from 128-->256.			
================================================================================================*/
void IPPC_LibOpeGetPrintAttr::PrinterOrganization( UCHAR (&Printer_Organization)[MAX_STRING256], TYPE_CharSet e_ComCharSet )
{
	pc_SystemSetting->lock();
	pc_SystemSetting->getAdminOrganizationUTF8( (char*)Printer_Organization, e_ComCharSet );
	pc_SystemSetting->unlock();
	return;
}

/*================================================================================================
［機能］	設定関数  printer-organizational-unit.
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2015.02.13 新規作成 HeliosL KRQ-93 AirPrint1.5(WP14).
            従来処理を関数化した.
			2017.03.06 Modified IT5-4.1(CQ1014723) Printer_Organizational_Unit String length is 
			increased from 128-->256.			
================================================================================================*/
void IPPC_LibOpeGetPrintAttr::PrinterOrganizationUnit( UCHAR (&Printer_Organizational_Unit)[MAX_STRING256], TYPE_CharSet e_ComCharSet )
{
	pc_SystemSetting->lock();
	pc_SystemSetting->getAdminOrganizationUnitUTF8( (char*)Printer_Organizational_Unit, e_ComCharSet );
	pc_SystemSetting->unlock();
	return;
}
// HeliosL KRQ-93 AirPrint1.5(WP14) 2015.02.13 OPS1八代 END.

/*================================================================================================
［機能］	設定関数  sides-default.
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2014/01/31 新規作成 CQ384241 ZeusCD UV-01249 AppleAirPrint.
 					   従来の処理を関数化した.
================================================================================================*/
IPP_SIDES IPPC_LibOpeGetPrintAttr::SidesDefault()
{
	IPP_SIDES e_Return = MIO_IPP_ONE_SIDED;

	pc_PrintJobSetting->lock();
	TYPE_DuplexMode 	e_DuplexMode	= pc_PrintJobSetting->getDuplexMode();
	TYPE_Orientation 	e_Orientation 	= pc_PrintJobSetting->getOrientation();
	TYPE_BindDirection 	e_BindDirection = pc_PrintJobSetting->getBindDirection();
	pc_PrintJobSetting->unlock();

	// 片面プリント.
    if( e_DuplexMode == TYPE_DuplexModeOff ) {
		e_Return = MIO_IPP_ONE_SIDED;


	// 両面プリント.
    }else{

		// ランドスケープ 
		if( e_Orientation == TYPE_OrientationLandscape ) {

			switch( e_BindDirection ) {
				case TYPE_BindDirectionLeft:	// 左綴じ 
				case TYPE_BindDirectionRight:	// 右綴じ 
					e_Return = MIO_IPP_TWO_SIDED_LONG_EDGE;
					break;

				case TYPE_BindDirectionTop:		// 上綴じ 
// 2019/03/08 FPT-LucNH2 IT6_2.0 【OP_BTS-2893】 fix warning Start.
					e_Return = MIO_IPP_TWO_SIDED_SHORT_EDGE;
					break;
// 2019/03/08 FPT-LucNH2 IT6_2.0 【OP_BTS-2893】 fix warning End.
				default:
					e_Return = MIO_IPP_TWO_SIDED_SHORT_EDGE;
					break;
			}

		// ポートレート.
		}else{

			switch( e_BindDirection ) {
				case TYPE_BindDirectionLeft:	// 左綴じ 
				case TYPE_BindDirectionRight:	// 右綴じ 
					e_Return = MIO_IPP_TWO_SIDED_SHORT_EDGE;
					break;

				case TYPE_BindDirectionTop:		// 上綴じ 
// 2019/03/08 FPT-LucNH2 IT6_2.0 【OP_BTS-2893】 fix warning Start.
					e_Return = MIO_IPP_TWO_SIDED_LONG_EDGE;
					break;
// 2019/03/08 FPT-LucNH2 IT6_2.0 【OP_BTS-2893】 fix warning End.
				default:
					e_Return = MIO_IPP_TWO_SIDED_LONG_EDGE;
					break;
			}
		}
	}
	return e_Return;
}

/*================================================================================================
［機能］	設定関数  media-default.
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2014/01/31 新規作成 CQ384241 ZeusCD UV-01249 AppleAirPrint.
================================================================================================*/
void IPPC_LibOpeGetPrintAttr::MediaDefault( UCHAR (&PaperTrayMedia)[MAX_STRING128] )
{
	memset( PaperTrayMedia, NULL, sizeof(PaperTrayMedia) );
	strcpy( (char*)PaperTrayMedia, Get_Media( Judge_Media_Default() ) );
	return;
}

/*================================================================================================
［機能］	設定関数 media-supported.
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2014/01/31 新規作成 CQ384241 ZeusCD UV-01249 AppleAirPrint.
================================================================================================*/
void IPPC_LibOpeGetPrintAttr::MediaSupported( UCHAR (&PaperTrayMedia)[MAX_IPP_PAPERTRAY_MEDIA][MAX_STRING128] )
{
	memset( PaperTrayMedia, NULL, sizeof(PaperTrayMedia) );
	//Zeus5BK 制御構成変更No172 OPS1前田 2014/05/13 START.
	uchar uc_Count = 0;
	TYPE_SystemType e_SystemType = pc_MachineSetting->getSystemType();
// 2021/05/24 [IT6_2.2] OP_BTS-33816 HIENG Start.
#ifdef DEF_DenebMLKCus
	TYPE_SubSystemType e_SubSystem = pc_MachineSetting->getSubSystemType();
#endif
// 2021/05/24 [IT6_2.2] OP_BTS-33816 HIENG End.
	IPPE_Media e_Media = CnvSpec_Media( TYPE_SizeCodeFlsSef );
	//Zeus5BK 制御構成変更No172 OPS1前田 2014/05/13 END.

	for ( IPPE_Media e_MediaCount = 0; e_MediaCount < IPPE_Media_NONE; e_MediaCount++ ){
#ifdef DEF_A4_Cus // VenusMLK先行開発 2OPS Yamaguchi.
		// not suuported size by VenusMLK
		if( ( IPPE_Media_A3 == e_MediaCount ) 
		|| ( IPPE_Media_B4 == e_MediaCount )
		|| ( IPPE_Media_11x17 == e_MediaCount )
		|| ( IPPE_Media_SRA3 == e_MediaCount )
		|| ( IPPE_Media_220x330 == e_MediaCount )
		|| ( IPPE_Media_KAI8 == e_MediaCount )
		|| ( IPPE_Media_12x18 == e_MediaCount )
		|| ( IPPE_Media_EnvelopeC5 == e_MediaCount )
		|| ( IPPE_Media_EnvelopeC4 == e_MediaCount )
		|| ( IPPE_Media_you4 == e_MediaCount )
		|| ( IPPE_Media_kaku1 == e_MediaCount )
		|| ( IPPE_Media_kaku2 == e_MediaCount )
		|| ( IPPE_Media_kaku3 == e_MediaCount )
		|| ( IPPE_Media_custom_max == e_MediaCount )
		|| ( IPPE_Media_custom_min == e_MediaCount ) ){
			continue;
		}
#endif // VenusMLK先行開発 2OPS Yamaguchi.

		//Zeus5BK 制御構成変更No172 OPS1前田 2014/05/13 START.
		// HelosL 制御No1 OSP1八代 2014.11.19 START.
#if ( defined DEF_Zeus5BkCus || defined DEF_HeliosCus || defined DEF_HeliosMLKCus )
		if( ( IPPE_Media_SRA3 == e_MediaCount ) || ( IPPE_Media_12x18 == e_MediaCount ) ){
			continue;
		}
#endif //#if ( DEF_Zeus5BkCus || defined DEF_HeliosCus || defined DEF_HeliosMLKCus )
		// HelosL 制御No1 OSP1八代 2014.11.19 END.
// 2021/05/24 [IT6_2.2] OP_BTS-33816 HIENG Start.
#ifdef DEF_DenebMLKCus
		if(( IPPE_Media_SRA3      == e_MediaCount )
		|| ( IPPE_Media_JPSHagaki == e_MediaCount )
		|| ( IPPE_Media_4x6       == e_MediaCount )
		|| ( IPPE_Media_12x18     == e_MediaCount )){
			continue;
		}

		if(( IPPE_Media_KAI8  == e_MediaCount )
		|| ( IPPE_Media_KAI16 == e_MediaCount )) {
			// Not China Market.
			// China Market (Other3) when e_SystemType is TYPE_SystemType_EU and e_SubSystem is TYPE_SubSystemType4.
			if (!((TYPE_SystemType_EU == e_SystemType) && (TYPE_SubSystemType4 == e_SubSystem ))) {
				continue;
			}
		}
#endif
// 2021/05/24 [IT6_2.2] OP_BTS-33816 HIENG End.
		if( ( IPPE_Media_A6Card == e_MediaCount ) && ( TYPE_SystemType_EU != e_SystemType ) ){
			continue;
		}

		// ZeusCD CQ390843 OPS1八代 2014/06/02 START.
		if( IPPE_Media_JPSHagaki == e_MediaCount ) {
			if( TYPE_SystemType_JP != e_SystemType ) {
				continue;
			}
		}
		// ZeusCD CQ390843 OPS1八代 2014/06/02 END.

		// ZeusS CQ510893 OPS1八代 2015.2.24 START.
		if( ( IPPE_Media_4x6 == e_MediaCount ) && ( TYPE_SystemType_US != e_SystemType ) ){
			continue;
		}
		// ZeusS CQ510893 OPS1八代 2015.2.24 END.

		if( ( IPPE_Media_8x13     == e_MediaCount )
		 || ( IPPE_Media_8125x13Q == e_MediaCount )
		 || ( IPPE_Media_8Qx13    == e_MediaCount )
		 || ( IPPE_Media_8Hx13    == e_MediaCount )
		 || ( IPPE_Media_8Hx13H   == e_MediaCount )
		 || ( IPPE_Media_220x330  == e_MediaCount ) ){
			if( e_Media != e_MediaCount ){
				continue;
			}
		}

		//strcpy( (char*)(PaperTrayMedia[e_MediaCount]) , Get_Media(e_MediaCount) );
		strcpy( (char*)(PaperTrayMedia[uc_Count]) , Get_Media(e_MediaCount) );
		uc_Count++;
		//Zeus5BK 制御構成変更No172 OPS1前田 2014/05/13 END.

		// 次順で、NICFUMが用意している配列の要素数を超えるようなら.
		// 不正な配列番号を参照してしまうため、(ログを出力した上で)、処理を抜ける.
		// (このログが出力されるということは、NICFUMに要素数の追加を依頼する必要があるということ).
		if( MAX_IPP_PAPERTRAY_MEDIA <=  e_MediaCount ) {
			DIPPTRACE1( "[IPP][fail safe] MediaSupported = %d\n",e_MediaCount );
			break;
		}
	}

	return;
}

/*================================================================================================
［機能］	設定関数 media-ready.
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2014/01/31 新規作成 CQ384241 ZeusCD UV-01249 AppleAirPrint.
================================================================================================*/
void IPPC_LibOpeGetPrintAttr::MediaReady( UCHAR (&Media_Ready)[MAX_MEDIA_READY][MAX_STRING128] )
{
	memset( Media_Ready, NULL, sizeof(Media_Ready) );

	uchar uc_Count = 0;
	IPPE_Media e_MediaReady = IPPE_Media_NONE;

	for( TYPE_FeedTray e_FeedTray = TYPE_FeedTrayManual; e_FeedTray < TYPE_FeedTrayMax; e_FeedTray++ ) {

		e_MediaReady = Judge_Media_Ready( e_FeedTray );

		if( IPPE_Media_NONE != e_MediaReady ) {
			strcpy( (char*)(Media_Ready[uc_Count]), Get_Media( e_MediaReady ) );
			uc_Count++;

			// 次順で、NICFUMが用意している配列の要素数を超えるようなら.
			// 不正な配列番号を参照してしまうため、(ログを出力した上で)、処理を抜ける.
			// (このログが出力されるということは、NICFUMに要素数の追加を依頼する必要があるということ).
			if( MAX_MEDIA_READY <= uc_Count ) {
				DIPPTRACE2( "[IPP][fail safe] MediaReady uc_Count = %d e_FeedTray = %d\n",uc_Count, e_FeedTray );
				break;
			}
		}
	}

	return;
}

/*================================================================================================
［機能］	設定関数 media-col-default.
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2014/01/31 新規作成 CQ384241 ZeusCD UV-01249 AppleAirPrint.
================================================================================================*/
void IPPC_LibOpeGetPrintAttr::MediaColDefault( IPP_Media_Col* pc_MediaCol )
{
	memset( pc_MediaCol, NULL, sizeof(IPP_Media_Col) );

	IPPS_MediaSize st_MediaSize;
	memset( &st_MediaSize, NULL, sizeof(IPPS_MediaSize) );

	IPPE_Media e_MediaSizeDefault = Judge_Media_Default();


	pc_MediaCol->set = 1;

	pc_MediaCol->Media_Col_Botton_Margin = IPPD_MediaMargin;
	pc_MediaCol->Media_Col_Top_Margin    = IPPD_MediaMargin;
	pc_MediaCol->Media_Col_Right_Margin  = IPPD_MediaMargin;
	pc_MediaCol->Media_Col_Left_Margin   = IPPD_MediaMargin;

	strcpy( (char*)pc_MediaCol->Media_Col_Source, Get_MediaSource( Judge_MediaSource_Default() ) );

	strcpy( (char*)pc_MediaCol->Media_Col_Type,   Get_MediaType( Judge_MediaType_Default() ) );

	st_MediaSize = Get_MediaSize( e_MediaSizeDefault );
	pc_MediaCol->Media_Size[0].set            = st_MediaSize.set;
	// ZeusCD CQ386774 OPS1八代 2014/02/25 START.
	pc_MediaCol->Media_Size[0].X_Dimension[0] = st_MediaSize.X_Dimension[0];
	pc_MediaCol->Media_Size[0].Y_Dimension[0] = st_MediaSize.Y_Dimension[0];
	pc_MediaCol->Media_Size[0].X_Dimension[1] = st_MediaSize.X_Dimension[1];
	pc_MediaCol->Media_Size[0].Y_Dimension[1] = st_MediaSize.Y_Dimension[1];
	// ZeusCD CQ386774 OPS1八代 2014/02/25 END.

	pc_MediaCol->Media_Source_Properties.set = 0;
	pc_MediaCol->Media_Source_Properties.Media_Source_Feed_Direction   = FEED_ORIENTATION_END;
	pc_MediaCol->Media_Source_Properties.Media_Source_Feed_Orientation = ORIENT_END;

	return;
}

/*================================================================================================
［機能］	設定関数 media-col-ready.
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2014/01/31 新規作成 CQ384241 ZeusCD UV-01249 AppleAirPrint.
================================================================================================*/
void IPPC_LibOpeGetPrintAttr::MediaColReady( IPP_Media_Col (&Media_Col_Ready)[MAX_IPP_MEDIA_COL_READY] )
{
	memset( Media_Col_Ready, NULL, sizeof(Media_Col_Ready) );

	uchar uc_Count = 0;
	IPPE_Media e_MediaColReady = IPPE_Media_NONE;

	TYPC_PaperSize c_PaperSize;
	memset( &c_PaperSize, NULL, sizeof(TYPC_PaperSize) );

	TYPC_PaperType c_PaperType;
	memset( &c_PaperType, NULL, sizeof(TYPC_PaperType) );

	IPPS_MediaSize st_MediaSize;
	memset( &st_MediaSize, NULL, sizeof(IPPS_MediaSize) );

	for( TYPE_FeedTray e_FeedTray = TYPE_FeedTrayManual; e_FeedTray < TYPE_FeedTrayMax; e_FeedTray++ ) {

		e_MediaColReady = Judge_Media_Ready( e_FeedTray );

		if( IPPE_Media_NONE != e_MediaColReady ) {
		    pc_FeedTray->lock();
		    c_PaperSize = pc_FeedTray->getPaperSize( e_FeedTray );
		    c_PaperType = pc_FeedTray->getPaperType( e_FeedTray );
		    pc_FeedTray->unlock();

			Media_Col_Ready[uc_Count].set = 1;
			Media_Col_Ready[uc_Count].Media_Col_Botton_Margin = IPPD_MediaMargin;
			Media_Col_Ready[uc_Count].Media_Col_Top_Margin    = IPPD_MediaMargin;
			Media_Col_Ready[uc_Count].Media_Col_Right_Margin  = IPPD_MediaMargin;
			Media_Col_Ready[uc_Count].Media_Col_Left_Margin   = IPPD_MediaMargin;

			strcpy( (char*)(Media_Col_Ready[uc_Count].Media_Col_Source), Get_MediaSource( CnvSpec_MediaSource( e_FeedTray ) ) );
			strcpy( (char*)(Media_Col_Ready[uc_Count].Media_Col_Type),   Get_MediaType( CnvSpec_MediaType( c_PaperType.getPaperType() ) ) );

			st_MediaSize = Get_MediaSize( e_MediaColReady );
			Media_Col_Ready[uc_Count].Media_Size[0].set            = st_MediaSize.set;
			// ZeusCD CQ386774 OPS1八代 2014/02/25 START.
			Media_Col_Ready[uc_Count].Media_Size[0].X_Dimension[0] = st_MediaSize.X_Dimension[0];
			Media_Col_Ready[uc_Count].Media_Size[0].Y_Dimension[0] = st_MediaSize.Y_Dimension[0];
			Media_Col_Ready[uc_Count].Media_Size[0].X_Dimension[1] = st_MediaSize.X_Dimension[1];
			Media_Col_Ready[uc_Count].Media_Size[0].Y_Dimension[1] = st_MediaSize.Y_Dimension[1];
			// ZeusCD CQ386774 OPS1八代 2014/02/25 END.

			Media_Col_Ready[uc_Count].Media_Source_Properties.set = 1;
			Media_Col_Ready[uc_Count].Media_Source_Properties.Media_Source_Feed_Direction   = Judge_MediaSourceFeedDirection( c_PaperSize );
			Media_Col_Ready[uc_Count].Media_Source_Properties.Media_Source_Feed_Orientation = ORIENT_LANDSCAPE;

			memset( &c_PaperSize, NULL, sizeof(TYPC_PaperSize) );
			memset( &c_PaperType, NULL, sizeof(TYPC_PaperType) );
			memset( &st_MediaSize, NULL, sizeof(IPPS_MediaSize) );

			uc_Count++;

			// 次順で、NICFUMが用意している配列の要素数を超えるようなら.
			// 不正な配列番号を参照してしまうため、(ログを出力した上で)、処理を抜ける.
			// (このログが出力されるということは、NICFUMに要素数の追加を依頼する必要があるということ).
			if( MAX_IPP_MEDIA_COL_READY <= uc_Count ) {
				DIPPTRACE2( "[IPP][fail safe] MediaColReady uc_Count = %d e_FeedTray = %d\n",uc_Count, e_FeedTray );
				break;
			}
		}
	}
	return;
}

/*================================================================================================
［機能］	設定関数 media-source-supported.
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2014/01/31 新規作成 CQ384241 ZeusCD UV-01249 AppleAirPrint.
================================================================================================*/
void IPPC_LibOpeGetPrintAttr::MediaSourceSupported( UCHAR (&Media_Source_Supported)[MAX_MEDIA_SOURCES_SUPPORTED][MAX_STRING128] )
{
	memset( Media_Source_Supported, NULL, sizeof(Media_Source_Supported) );

	IPPE_MediaSource e_MediaSource = IPPE_MediaSource_NONE;
	uchar uc_Count = 0;

	// ZeusCD CQ386580 OPS1八代 2014/02/25 START.
	strcpy( (char *)( Media_Source_Supported[uc_Count] ), Get_MediaSource( IPPE_MediaSource_Auto ) );
	uc_Count++;
	// ZeusCD CQ386580 OPS1八代 2014/02/25 END.

	pc_FeedTray->lock();
	for( TYPE_FeedTray e_FeedTray = TYPE_FeedTrayManual; e_FeedTray < TYPE_FeedTrayMax; e_FeedTray++ ) {
		if( TYPE_TrayStatusNon != pc_FeedTray->getTrayStatus( e_FeedTray ) ) {
			e_MediaSource = CnvSpec_MediaSource( e_FeedTray );
			if( IPPE_MediaSource_NONE != e_MediaSource ) {
				strcpy( (char *)( Media_Source_Supported[uc_Count] ), Get_MediaSource( e_MediaSource ) );
				uc_Count++;

				// 次順で、NICFUMが用意している配列の要素数を超えるようなら.
				// 不正な配列番号を参照してしまうため、(ログを出力した上で)、処理を抜ける.
				// (このログが出力されるということは、NICFUMに要素数の追加を依頼する必要があるということ).
				if( MAX_MEDIA_SOURCES_SUPPORTED <= uc_Count ) {
					DIPPTRACE2( "[IPP][fail safe] MediaSourceSupported uc_Count = %d e_FeedTray = %d\n",uc_Count, e_FeedTray );
					break;
				}
			}
		}
	}
	pc_FeedTray->unlock();

	return;
}

/*================================================================================================
［機能］	設定関数 media-type-supported.
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2014/01/31 新規作成 CQ384241 ZeusCD UV-01249 AppleAirPrint.
================================================================================================*/
void IPPC_LibOpeGetPrintAttr::MediaTypeSupported( UCHAR (&Media_Type_Supported)[MAX_MEDIA_TYPES_SUPPORTED][MAX_STRING128] )
{
	memset( Media_Type_Supported, NULL, sizeof(Media_Type_Supported) );
	//Zeus5BK 制御構成変更No172 OPS1前田 2014/05/13 START.
	uchar uc_Count = 0;
	//Zeus5BK 制御構成変更No172 OPS1前田 2014/05/13 END.

	for ( IPPE_MediaType e_MediaTypeCount = IPPE_MediaType_Auto; e_MediaTypeCount < IPPE_MediaType_NONE; e_MediaTypeCount++ ){
#ifdef DEF_A4_Cus // VenusMLK先行開発 2OPS Yamaguchi.
		// not suuported media type by VenusMLK
		if( ( IPPE_MediaType_Thick3 == e_MediaTypeCount ) 
		|| ( IPPE_MediaType_Thick3Sec == e_MediaTypeCount )
		|| ( IPPE_MediaType_Thick4 == e_MediaTypeCount )
		|| ( IPPE_MediaType_Thick4Sec == e_MediaTypeCount )
		|| ( IPPE_MediaType_Thin1 == e_MediaTypeCount )
		|| ( IPPE_MediaType_Thin1Sec == e_MediaTypeCount )
		|| ( IPPE_MediaType_ThinOhp == e_MediaTypeCount )
		|| ( IPPE_MediaType_IndexP == e_MediaTypeCount )
		|| ( IPPE_MediaType_UserDef6 == e_MediaTypeCount )
		|| ( IPPE_MediaType_UserDef6Sec == e_MediaTypeCount )
		|| ( IPPE_MediaType_Cus1 == e_MediaTypeCount )
		|| ( IPPE_MediaType_Cus2 == e_MediaTypeCount )
		|| ( IPPE_MediaType_Cus3 == e_MediaTypeCount )
		|| ( IPPE_MediaType_Cus4 == e_MediaTypeCount )
		|| ( IPPE_MediaType_Cus5 == e_MediaTypeCount )
		|| ( IPPE_MediaType_Cus6 == e_MediaTypeCount )
		|| ( IPPE_MediaType_Cus7 == e_MediaTypeCount )
		|| ( IPPE_MediaType_Cus8 == e_MediaTypeCount )
		|| ( IPPE_MediaType_Cus9 == e_MediaTypeCount )
		|| ( IPPE_MediaType_Cus10 == e_MediaTypeCount )
		|| ( IPPE_MediaType_Cus11 == e_MediaTypeCount )
		|| ( IPPE_MediaType_Cus12 == e_MediaTypeCount )
		|| ( IPPE_MediaType_Cus13 == e_MediaTypeCount )
		|| ( IPPE_MediaType_Cus14 == e_MediaTypeCount )
		|| ( IPPE_MediaType_Cus15 == e_MediaTypeCount )
		|| ( IPPE_MediaType_Cus16 == e_MediaTypeCount )
		|| ( IPPE_MediaType_Cus17 == e_MediaTypeCount )
		|| ( IPPE_MediaType_Cus18 == e_MediaTypeCount )
		|| ( IPPE_MediaType_Cus19 == e_MediaTypeCount )
		|| ( IPPE_MediaType_Cus20 == e_MediaTypeCount ) ) {
			continue;
		}
#endif // VenusMLK先行開発 2OPS Yamaguchi.
// CQ391054 2014/06/17 OPS1八代 START.
// ZeusSBK 制御No1 OPS1八代 2016.02.03 START.
// #if ( defined DEF_Zeus5BkCus || defined DEF_ZEUSS_CUS || defined DEF_HeliosCus || defined DEF_ZEUSS_ZX0_CUS) //ZeusS ZX0 制御構成変更_No.2 2015/06/08 HCL-Vijay
// 2019/04/05【EagleBK】【Control No.6】 HIENG Start.
//#if ( defined DEF_Zeus5BkCus || defined DEF_ZEUSS_CUS || defined DEF_HeliosCus || defined DEF_ZEUSS_ZX0_CUS || defined DEF_ZEUSSBK12_CUS || defined DEF_ZEUSSBKX0_CUS || DEF_ZEUSSBK2ND_12_CUS || DEF_ZEUSSBK2ND_ZX0_CUS || DEF_MINERVAMLK_CUS ) //IT5_4.1 制御構成変更_No.1 2016/11/02 HCL-Sankar Added.
#if ( defined DEF_EagleBKCus || defined DEF_EagleCus || defined DEF_Zeus5BkCus || defined DEF_ZEUSS_CUS || defined DEF_HeliosCus || defined DEF_ZEUSS_ZX0_CUS || defined DEF_ZEUSSBK12_CUS || defined DEF_ZEUSSBKX0_CUS || DEF_ZEUSSBK2ND_12_CUS || DEF_ZEUSSBK2ND_ZX0_CUS || DEF_MINERVAMLK_CUS ) //IT5_4.1 制御構成変更_No.1 2016/11/02 HCL-Sankar Added.
// 2019/04/05【EagleBK】【Control No.6】 HIENG End.
// ZeusSBK 制御No1 OPS1八代 2016.02.03 END.
		// 薄紙両面2面目は、Zeus系はサポート対象外.
// 2021/04/28 【MinervaSSBK】【機器構成変更_No.21-12】 HIENG Start. 
#ifndef DEF_MinervaSSBKCus
		if( IPPE_MediaType_Thin1Sec == e_MediaTypeCount ) {
			continue;
		}
#endif
// 2021/04/28 【MinervaSSBK】【機器構成変更_No.21-12】 HIENG End.
#endif
// CQ391054 2014/06/17 OPS1八代 END.
#if ( defined DEF_Zeus5BkCus || defined DEF_HeliosCus || defined DEF_HeliosMLKCus )
		//Zeus5BK 制御構成変更No172 OPS1前田 2014/05/13 START.
		if( ( IPPE_MediaType_Thick4    == e_MediaTypeCount )
		 || ( IPPE_MediaType_Thick4Sec == e_MediaTypeCount )
		 || ( IPPE_MediaType_Thin1     == e_MediaTypeCount )
		 || ( IPPE_MediaType_Thin1Sec  == e_MediaTypeCount ) ){
			continue;
		}
#endif //#if ( DEF_Zeus5BkCus || DEF_HeliosCus || defined DEF_HeliosMLKCus )

#ifdef DEF_KRONOSS_Cus // KronosS 用紙種別カスタマイズ対応 START.
		// not suuported media type by KronosS.
		if( ( IPPE_MediaType_PhotoSemGloss == e_MediaTypeCount )
		|| ( IPPE_MediaType_PhotoGloss == e_MediaTypeCount )
		|| ( IPPE_MediaType_PhotoHighGloss == e_MediaTypeCount ) ) {
			continue;		
		}
#endif // KronosS 用紙種別カスタマイズ対応 END.		

#ifdef DEF_DenebMLKCus	// EagleZ+ 制御構成変更No.36 MIF機対応.
		if( ( IPPE_MediaType_Thick2     == e_MediaTypeCount )
		|| ( IPPE_MediaType_Thick2Sec   == e_MediaTypeCount )
		|| ( IPPE_MediaType_Thick3      == e_MediaTypeCount )
		|| ( IPPE_MediaType_Thick3Sec   == e_MediaTypeCount )
		|| ( IPPE_MediaType_Thick4      == e_MediaTypeCount )
		|| ( IPPE_MediaType_Thick4Sec   == e_MediaTypeCount )
		|| ( IPPE_MediaType_Thin1       == e_MediaTypeCount )
		|| ( IPPE_MediaType_Thin1Sec    == e_MediaTypeCount )
		|| ( IPPE_MediaType_ColorP      == e_MediaTypeCount )
		|| ( IPPE_MediaType_IndexP      == e_MediaTypeCount )
		|| ( IPPE_MediaType_LetterH     == e_MediaTypeCount )
		|| ( IPPE_MediaType_SpecialP    == e_MediaTypeCount )
		|| ( IPPE_MediaType_PrintedP    == e_MediaTypeCount )
		|| ( IPPE_MediaType_UserDef5    == e_MediaTypeCount )
		|| ( IPPE_MediaType_UserDef5Sec == e_MediaTypeCount )
		|| ( IPPE_MediaType_UserDef6    == e_MediaTypeCount )
		|| ( IPPE_MediaType_UserDef6Sec == e_MediaTypeCount ) ) {
			continue;		
		}
#endif

#ifndef DEF_EagleZpCus	// EagleZ+ 機器構成変更No.7 コート紙対応(Step2).
		if(( IPPE_MediaType_UserDef7    == e_MediaTypeCount )
		|| ( IPPE_MediaType_UserDef7Sec == e_MediaTypeCount )){
			continue;
		}
#endif

		//strcpy( (char *)(Media_Type_Supported[e_MediaTypeCount]) , Get_MediaType( e_MediaTypeCount ) );
		strcpy( (char *)(Media_Type_Supported[uc_Count]) , Get_MediaType( e_MediaTypeCount ) );
		uc_Count++;
		//Zeus5BK 制御構成変更No172 OPS1前田 2014/05/13 END.

		// 次順で、NICFUMが用意している配列の要素数を超えるようなら.
		// 不正な配列番号を参照してしまうため、(ログを出力した上で)、処理を抜ける.
		// (このログが出力されるということは、NICFUMに要素数の追加を依頼する必要があるということ).
		if( MAX_MEDIA_TYPES_SUPPORTED <= e_MediaTypeCount ) {
			DIPPTRACE1( "[IPP][fail safe] MediaTypeSupported = %d\n",e_MediaTypeCount );
			break;
		}
	}

	return;
}

/*================================================================================================
［機能］	設定関数 media-size-supported.
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2014/01/31 新規作成 CQ384241 ZeusCD UV-01249 AppleAirPrint.
================================================================================================*/
void IPPC_LibOpeGetPrintAttr::MediaSizeSupported( IPP_Media_Col_Media_Size (&Media_Size_Supported)[MAX_IPP_MEDIA_COL_SIZE_SUPPORTED] )
{
	memset( Media_Size_Supported, NULL, sizeof(Media_Size_Supported) );

	IPPS_MediaSize st_MediaSize;
	memset( &st_MediaSize, NULL, sizeof(IPPS_MediaSize) );
	//Zeus5BK 制御構成変更No172 OPS1前田 2014/05/13 START.
	TYPE_SystemType e_SystemType	= pc_MachineSetting->getSystemType();
// 2021/05/24 [IT6_2.2] OP_BTS-33816 HIENG Start.
#ifdef DEF_DenebMLKCus
	TYPE_SubSystemType e_SubSystem = pc_MachineSetting->getSubSystemType();
#endif
// 2021/05/24 [IT6_2.2] OP_BTS-33816 HIENG End.
	IPPE_Media e_Media_Fls = CnvSpec_Media( TYPE_SizeCodeFlsSef );
	//Zeus5BK 制御構成変更No172 OPS1前田 2014/05/13 END.

	uchar uc_Count = 0;

	for ( IPPE_Media e_Media = IPPE_Media_A3; e_Media < IPPE_Media_NONE; e_Media++ ){
#ifdef DEF_A4_Cus // VenusMLK先行開発 2OPS Yamaguchi 20151005 START.
		// not suuported size by VenusMLK
		if( ( IPPE_Media_A3 == e_Media ) 
		|| ( IPPE_Media_B4 == e_Media )
		|| ( IPPE_Media_11x17 == e_Media )
		|| ( IPPE_Media_SRA3 == e_Media )
		|| ( IPPE_Media_220x330 == e_Media )
		|| ( IPPE_Media_KAI8 == e_Media )
		|| ( IPPE_Media_12x18 == e_Media )
		|| ( IPPE_Media_EnvelopeC5 == e_Media )
		|| ( IPPE_Media_EnvelopeC4 == e_Media )
		|| ( IPPE_Media_you4 == e_Media )
		|| ( IPPE_Media_kaku1 == e_Media )
		|| ( IPPE_Media_kaku2 == e_Media )
		|| ( IPPE_Media_kaku3 == e_Media )
		|| ( IPPE_Media_custom_max == e_Media )
		|| ( IPPE_Media_custom_min == e_Media )){
			continue;
		}
#endif // VenusMLK先行開発 2OPS Yamaguchi 20151005 END.
		//Zeus5BK 制御構成変更No172 OPS1前田 2014/05/13 START.
		// HelosL 制御No1 OSP1八代 2014.11.19 START.
#if ( defined DEF_Zeus5BkCus || defined DEF_HeliosCus || defined DEF_HeliosMLKCus )
		if( ( IPPE_Media_SRA3 == e_Media ) || ( IPPE_Media_12x18 == e_Media ) ){
			continue;
		}
#endif //#if ( DEF_Zeus5BkCus || defined DEF_HeliosCus || defined DEF_HeliosMLKCus )
		// HelosL 制御No1 OSP1八代 2014.11.19 END.

#ifdef DEF_DenebMLKCus	// EagleZ+ 制御構成変更No.36 MIF機対応.
// 2021/05/24 [IT6_2.2] OP_BTS-33816 HIENG Start.
		//if( ( IPPE_Media_11x17    == e_Media )
		if(( IPPE_Media_SRA3      == e_Media )
		|| ( IPPE_Media_JPSHagaki == e_Media )
		|| ( IPPE_Media_4x6       == e_Media )
		|| ( IPPE_Media_12x18     == e_Media )){
			continue;
		}

		if(( IPPE_Media_KAI8  == e_Media )
		|| ( IPPE_Media_KAI16 == e_Media )) {
			// Not China Market.
			// China Market (Other3) when e_SystemType is TYPE_SystemType_EU and e_SubSystem is TYPE_SubSystemType4.
			if (!((TYPE_SystemType_EU == e_SystemType) && (TYPE_SubSystemType4 == e_SubSystem ))) {
				continue;
			}
		}
// 2021/05/24 [IT6_2.2] OP_BTS-33816 HIENG End.
#endif

		if( ( IPPE_Media_A6Card == e_Media ) && ( TYPE_SystemType_EU != e_SystemType ) ){
			continue;
		}

		// ZeusCD CQ390843 OPS1八代 2014/06/02 START.
		if( IPPE_Media_JPSHagaki == e_Media ) {
			if( TYPE_SystemType_JP != e_SystemType ) {
				continue;
			}
		}
		// ZeusCD CQ390843 OPS1八代 2014/06/02 END.

		// ZeusS CQ510893 OPS1八代 2015.2.24 START.
		if( ( IPPE_Media_4x6 == e_Media ) && ( TYPE_SystemType_US != e_SystemType ) ){
			continue;
		}
		// ZeusS CQ510893 OPS1八代 2015.2.24 END.

		if( ( IPPE_Media_8x13     == e_Media )
		 || ( IPPE_Media_8125x13Q == e_Media )
		 || ( IPPE_Media_8Qx13    == e_Media )
		 || ( IPPE_Media_8Hx13    == e_Media )
		 || ( IPPE_Media_8Hx13H   == e_Media )
		 || ( IPPE_Media_220x330  == e_Media ) ){
			if( e_Media_Fls != e_Media ){
				continue;
			}
		}
		//Zeus5BK 制御構成変更No172 OPS1前田 2014/05/13 END.

		st_MediaSize = Get_MediaSize( e_Media );

		Media_Size_Supported[uc_Count].set            = st_MediaSize.set;
		// ZeusCD CQ386774 OPS1八代 2014/02/25 START.
		Media_Size_Supported[uc_Count].X_Dimension[0] = st_MediaSize.X_Dimension[0];
		Media_Size_Supported[uc_Count].Y_Dimension[0] = st_MediaSize.Y_Dimension[0];
		Media_Size_Supported[uc_Count].X_Dimension[1] = st_MediaSize.X_Dimension[1];
		Media_Size_Supported[uc_Count].Y_Dimension[1] = st_MediaSize.Y_Dimension[1];
		// ZeusCD CQ386774 OPS1八代 2014/02/25 END.

		uc_Count++;

		memset( &st_MediaSize, NULL, sizeof(IPPS_MediaSize) );

		// 次順で、NICFUMが用意している配列の要素数を超えるようなら.
		// 不正な配列番号を参照してしまうため、(ログを出力した上で)、処理を抜ける.
		// (このログが出力されるということは、NICFUMに要素数の追加を依頼する必要があるということ).
		if( MAX_IPP_MEDIA_COL_SIZE_SUPPORTED <= uc_Count ) {
			DIPPTRACE2( "[IPP][fail safe] MediaSizeSupported uc_Count = %d e_Media = %d\n",uc_Count, e_Media );
			break;
		}
	}

	return;
}

/*================================================================================================
［機能］	設定関数 media-col-database.
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2014/01/31 新規作成 CQ384241 ZeusCD UV-01249 AppleAirPrint.
================================================================================================*/
static struct IPPS_MediaColDatbase{
	IPPE_Media				e_Media;		// IPP定義 - media.
	IPP_FEED_ORIENTATION 	e_Feed_Direction;
	IPPE_MediaSource		e_MediaSource;	// IPP定義 - media-source.
} st_MediaColDatbase [] = {
//Zeus5BK 制御構成変更No172 OPS1前田 2014/05/13 START.
	{ IPPE_Media_A3, 				SHORT_EDGE_FIRST,		IPPE_MediaSource_ByPass	 },
	{ IPPE_Media_A3, 				SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray1	 },
	{ IPPE_Media_A3, 				SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray2	 },
	{ IPPE_Media_A3, 				SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray3	 },
	{ IPPE_Media_A3, 				SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray4	 },
	{ IPPE_Media_A3, 				SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray5	 },		//2018/05/31 DenebMLK 機器構成変更_No.7 HCL-VIJAY Added.
	{ IPPE_Media_A3, 				SHORT_EDGE_FIRST,		IPPE_MediaSource_LCT	 },		// A4 LCTのときは、非サポート.
	{ IPPE_Media_A4, 				LONG_EDGE_FIRST,		IPPE_MediaSource_NONE	 },
	{ IPPE_Media_A4, 				SHORT_EDGE_FIRST,		IPPE_MediaSource_ByPass	 },
	{ IPPE_Media_A4, 				SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray1	 },
	{ IPPE_Media_A4, 				SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray2	 },
	{ IPPE_Media_A4, 				SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray3	 },
	{ IPPE_Media_A4, 				SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray4	 },
	{ IPPE_Media_A4, 				SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray5	 },		//2018/05/31 DenebMLK 機器構成変更_No.7 HCL-VIJAY Added.
	{ IPPE_Media_A4, 				SHORT_EDGE_FIRST,		IPPE_MediaSource_LCT	 },		// A4 LCTのときは、非サポート.
	{ IPPE_Media_A5, 				LONG_EDGE_FIRST,		IPPE_MediaSource_ByPass	 },
	{ IPPE_Media_A5, 				SHORT_EDGE_FIRST,		IPPE_MediaSource_ByPass	 },
	{ IPPE_Media_A5, 				SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray1	 },
	{ IPPE_Media_A5, 				SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray2	 },
	{ IPPE_Media_A5, 				SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray3	 },
	{ IPPE_Media_A5, 				SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray4	 },
	{ IPPE_Media_A5, 				SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray5	 },		//2018/05/31 DenebMLK 機器構成変更_No.7 HCL-VIJAY Added. 
	{ IPPE_Media_A6, 				SHORT_EDGE_FIRST,		IPPE_MediaSource_ByPass	 },
	{ IPPE_Media_B4, 				SHORT_EDGE_FIRST,		IPPE_MediaSource_ByPass	 },
	{ IPPE_Media_B4, 				SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray1	 },
	{ IPPE_Media_B4, 				SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray2	 },
	{ IPPE_Media_B4, 				SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray3	 },
	{ IPPE_Media_B4, 				SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray4	 },
	{ IPPE_Media_B4, 				SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray5	 },		//2018/05/31 DenebMLK 機器構成変更_No.7 HCL-VIJAY Added. 
	{ IPPE_Media_B4, 				SHORT_EDGE_FIRST,		IPPE_MediaSource_LCT	 },		// A4 LCTのときは、非サポート.
	{ IPPE_Media_B5, 				FEED_ORIENTATION_END,	IPPE_MediaSource_ByPass	 },
	{ IPPE_Media_B5, 				FEED_ORIENTATION_END,	IPPE_MediaSource_Tray1	 },
	{ IPPE_Media_B5, 				FEED_ORIENTATION_END,	IPPE_MediaSource_Tray2	 },
	{ IPPE_Media_B5, 				FEED_ORIENTATION_END,	IPPE_MediaSource_Tray3	 },
	{ IPPE_Media_B5, 				FEED_ORIENTATION_END,	IPPE_MediaSource_Tray4	 },
	{ IPPE_Media_B5, 				FEED_ORIENTATION_END,	IPPE_MediaSource_Tray5	 },		//2018/05/31 DenebMLK 機器構成変更_No.7 HCL-VIJAY Added. 
	{ IPPE_Media_B6, 				SHORT_EDGE_FIRST,		IPPE_MediaSource_ByPass	 },
	{ IPPE_Media_11x17, 			SHORT_EDGE_FIRST,		IPPE_MediaSource_ByPass	 },
	{ IPPE_Media_11x17, 			SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray1	 },
	{ IPPE_Media_11x17, 			SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray2	 },
	{ IPPE_Media_11x17, 			SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray3	 },
	{ IPPE_Media_11x17, 			SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray4	 },
	{ IPPE_Media_11x17, 			SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray5	 },		//2018/05/31 DenebMLK 機器構成変更_No.7 HCL-VIJAY Added. 
	{ IPPE_Media_11x17, 			SHORT_EDGE_FIRST,		IPPE_MediaSource_LCT	 },		// A4 LCTのときは、非サポート.
	{ IPPE_Media_8Hx14, 			SHORT_EDGE_FIRST,		IPPE_MediaSource_ByPass	 },
	{ IPPE_Media_8Hx14, 			SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray1	 },
	{ IPPE_Media_8Hx14, 			SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray2	 },
	{ IPPE_Media_8Hx14, 			SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray3	 },
	{ IPPE_Media_8Hx14, 			SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray4	 },
	{ IPPE_Media_8Hx14, 			SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray5	 },		//2018/05/31 DenebMLK 機器構成変更_No.7 HCL-VIJAY Added. 
	{ IPPE_Media_8Hx14, 			SHORT_EDGE_FIRST,		IPPE_MediaSource_LCT	 },		// A4 LCTのときは、非サポート.
	{ IPPE_Media_8Hx11, 			LONG_EDGE_FIRST,		IPPE_MediaSource_NONE	 },
	{ IPPE_Media_8Hx11, 			SHORT_EDGE_FIRST,		IPPE_MediaSource_ByPass	 },
	{ IPPE_Media_8Hx11, 			SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray1	 },
	{ IPPE_Media_8Hx11, 			SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray2	 },
	{ IPPE_Media_8Hx11, 			SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray3	 },
	{ IPPE_Media_8Hx11, 			SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray4	 },
	{ IPPE_Media_8Hx11, 			SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray5	 },		//2018/05/31 DenebMLK 機器構成変更_No.7 HCL-VIJAY Added. 
	{ IPPE_Media_8Hx11, 			SHORT_EDGE_FIRST,		IPPE_MediaSource_LCT	 },		// A4 LCTのときは、非サポート.
	{ IPPE_Media_7Qx10H, 			FEED_ORIENTATION_END,	IPPE_MediaSource_ByPass	 },
	{ IPPE_Media_5Hx8H, 			LONG_EDGE_FIRST,		IPPE_MediaSource_ByPass	 },
	{ IPPE_Media_5Hx8H, 			SHORT_EDGE_FIRST,		IPPE_MediaSource_ByPass	 },
	{ IPPE_Media_5Hx8H, 			SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray1	 },		// 日本、EU仕向けのときは、非サポート.
	{ IPPE_Media_5Hx8H, 			SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray2	 },		// 日本、EU仕向けのときは、非サポート.
	{ IPPE_Media_5Hx8H, 			SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray3	 },		// 日本、EU仕向けのときは、非サポート.
	{ IPPE_Media_5Hx8H, 			SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray4	 },		// 日本、EU仕向けのときは、非サポート.
	{ IPPE_Media_5Hx8H, 			SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray5	 },		// 日本、EU仕向けのときは、非サポート.	
																							//2018/05/31 DenebMLK 機器構成変更_No.7 HCL-VIJAY Added. 
	{ IPPE_Media_SRA3, 				SHORT_EDGE_FIRST,		IPPE_MediaSource_ByPass	 },
	{ IPPE_Media_SRA3, 				SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray2	 },
	{ IPPE_Media_SRA3, 				SHORT_EDGE_FIRST,		IPPE_MediaSource_LCT	 },		// A4 LCTのときは、非サポート.
	{ IPPE_Media_8x13, 				SHORT_EDGE_FIRST,		IPPE_MediaSource_ByPass	 },
	{ IPPE_Media_8x13, 				SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray1	 },
	{ IPPE_Media_8x13, 				SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray2	 },
	{ IPPE_Media_8x13, 				SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray3	 },
	{ IPPE_Media_8x13, 				SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray4	 },
	{ IPPE_Media_8x13, 				SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray5	 },		//2018/05/31 DenebMLK 機器構成変更_No.7 HCL-VIJAY Added. 
	{ IPPE_Media_8125x13Q, 			SHORT_EDGE_FIRST,		IPPE_MediaSource_ByPass	 },
	{ IPPE_Media_8125x13Q, 			SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray1	 },
	{ IPPE_Media_8125x13Q, 			SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray2	 },
	{ IPPE_Media_8125x13Q, 			SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray3	 },
	{ IPPE_Media_8125x13Q, 			SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray4	 },
	{ IPPE_Media_8125x13Q, 			SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray5	 },		//2018/05/31 DenebMLK 機器構成変更_No.7 HCL-VIJAY Added.
	{ IPPE_Media_8Qx13, 			SHORT_EDGE_FIRST,		IPPE_MediaSource_ByPass	 },
	{ IPPE_Media_8Qx13, 			SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray1	 },
	{ IPPE_Media_8Qx13, 			SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray2	 },
	{ IPPE_Media_8Qx13, 			SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray3	 },
	{ IPPE_Media_8Qx13, 			SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray4	 },
	{ IPPE_Media_8Qx13, 			SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray5	 },		//2018/05/31 DenebMLK 機器構成変更_No.7 HCL-VIJAY Added.
	{ IPPE_Media_8Hx13, 			SHORT_EDGE_FIRST,		IPPE_MediaSource_ByPass	 },
	{ IPPE_Media_8Hx13, 			SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray1	 },
	{ IPPE_Media_8Hx13, 			SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray2	 },
	{ IPPE_Media_8Hx13, 			SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray3	 },
	{ IPPE_Media_8Hx13, 			SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray4	 },
	{ IPPE_Media_8Hx13, 			SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray5	 },		//2018/05/31 DenebMLK 機器構成変更_No.7 HCL-VIJAY Added.
	{ IPPE_Media_8Hx13H, 			SHORT_EDGE_FIRST,		IPPE_MediaSource_ByPass	 },
	{ IPPE_Media_8Hx13H, 			SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray1	 },
	{ IPPE_Media_8Hx13H, 			SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray2	 },
	{ IPPE_Media_8Hx13H, 			SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray3	 },
	{ IPPE_Media_8Hx13H, 			SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray4	 },
	{ IPPE_Media_8Hx13H, 			SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray5	 },		//2018/05/31 DenebMLK 機器構成変更_No.7 HCL-VIJAY Added.
	{ IPPE_Media_220x330, 			SHORT_EDGE_FIRST,		IPPE_MediaSource_ByPass	 },
	{ IPPE_Media_220x330, 			SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray1	 },
	{ IPPE_Media_220x330, 			SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray2	 },
	{ IPPE_Media_220x330, 			SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray3	 },
	{ IPPE_Media_220x330, 			SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray4	 },
	{ IPPE_Media_220x330, 			SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray5	 },		//2018/05/31 DenebMLK 機器構成変更_No.7 HCL-VIJAY Added.
	{ IPPE_Media_KAI8, 				SHORT_EDGE_FIRST,		IPPE_MediaSource_ByPass	 },
	{ IPPE_Media_KAI8, 				SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray1	 },
	{ IPPE_Media_KAI8, 				SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray2	 },
	{ IPPE_Media_KAI8, 				SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray3	 },
	{ IPPE_Media_KAI8, 				SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray4	 },
	{ IPPE_Media_KAI8, 				SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray5	 },		//2018/05/31 DenebMLK 機器構成変更_No.7 HCL-VIJAY Added.
	{ IPPE_Media_KAI16, 			LONG_EDGE_FIRST,		IPPE_MediaSource_ByPass	 },
	{ IPPE_Media_KAI16, 			LONG_EDGE_FIRST,		IPPE_MediaSource_Tray1	 },
	{ IPPE_Media_KAI16, 			LONG_EDGE_FIRST,		IPPE_MediaSource_Tray2	 },
	{ IPPE_Media_KAI16, 			LONG_EDGE_FIRST,		IPPE_MediaSource_Tray3	 },
	{ IPPE_Media_KAI16, 			LONG_EDGE_FIRST,		IPPE_MediaSource_Tray4	 },
	{ IPPE_Media_KAI16, 			LONG_EDGE_FIRST,		IPPE_MediaSource_Tray5	 },		//2018/05/31 DenebMLK 機器構成変更_No.7 HCL-VIJAY Added.
	{ IPPE_Media_KAI16, 			SHORT_EDGE_FIRST,		IPPE_MediaSource_ByPass	 },
	{ IPPE_Media_12x18, 			SHORT_EDGE_FIRST,		IPPE_MediaSource_ByPass	 },
	{ IPPE_Media_12x18, 			SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray2	 },
	{ IPPE_Media_12x18, 			SHORT_EDGE_FIRST,		IPPE_MediaSource_LCT	 },		// A4 LCTのときは、非サポート.
	{ IPPE_Media_JPSHagaki, 		SHORT_EDGE_FIRST,		IPPE_MediaSource_ByPass	 },
	{ IPPE_Media_4x6, 				SHORT_EDGE_FIRST,		IPPE_MediaSource_ByPass	 },
	{ IPPE_Media_A6Card, 			SHORT_EDGE_FIRST,		IPPE_MediaSource_ByPass	 },
	{ IPPE_Media_EnvelopeMonarch,	SHORT_EDGE_FIRST,		IPPE_MediaSource_ByPass	 },
	{ IPPE_Media_EnvelopeCom10, 	SHORT_EDGE_FIRST,		IPPE_MediaSource_ByPass	 },
	{ IPPE_Media_EnvelopeDL, 		SHORT_EDGE_FIRST,		IPPE_MediaSource_ByPass	 },
	{ IPPE_Media_EnvelopeC5, 		SHORT_EDGE_FIRST,		IPPE_MediaSource_ByPass	 },
	{ IPPE_Media_EnvelopeB5, 		SHORT_EDGE_FIRST,		IPPE_MediaSource_ByPass	 },
	{ IPPE_Media_EnvelopeC4, 		SHORT_EDGE_FIRST,		IPPE_MediaSource_ByPass	 },
	{ IPPE_Media_EnvelopeC6, 		SHORT_EDGE_FIRST,		IPPE_MediaSource_ByPass	 },
	{ IPPE_Media_chou3, 			SHORT_EDGE_FIRST,		IPPE_MediaSource_ByPass	 },
	{ IPPE_Media_chou4, 			SHORT_EDGE_FIRST,		IPPE_MediaSource_ByPass	 },
	{ IPPE_Media_you3, 				SHORT_EDGE_FIRST,		IPPE_MediaSource_ByPass	 },
	{ IPPE_Media_you4, 				SHORT_EDGE_FIRST,		IPPE_MediaSource_ByPass	 },
	{ IPPE_Media_kaku1, 			SHORT_EDGE_FIRST,		IPPE_MediaSource_ByPass	 },
	{ IPPE_Media_kaku2, 			SHORT_EDGE_FIRST,		IPPE_MediaSource_ByPass	 },
	{ IPPE_Media_kaku3, 			SHORT_EDGE_FIRST,		IPPE_MediaSource_ByPass	 },
//OP_BTS-34283 [IT6-2.2] HIENG Start.
#ifdef DEF_EagleCus
	{ IPPE_Media_EnvelopeCom10, 	SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray1	 },
	{ IPPE_Media_EnvelopeDL, 		SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray1	 },
	{ IPPE_Media_EnvelopeC5, 		SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray1	 },
	{ IPPE_Media_EnvelopeB5, 		SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray1	 },
	{ IPPE_Media_EnvelopeC4, 		SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray1	 },
	{ IPPE_Media_EnvelopeC6, 		SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray1	 },
	{ IPPE_Media_chou3, 			SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray1	 },
	{ IPPE_Media_chou4, 			SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray1	 },
	{ IPPE_Media_you3, 				SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray1	 },
	{ IPPE_Media_you4, 				SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray1	 },
	{ IPPE_Media_kaku1, 			SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray1	 },
	{ IPPE_Media_kaku2, 			SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray1	 },
	{ IPPE_Media_kaku3, 			SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray1	 },
#endif
//OP_BTS-34283 [IT6-2.2] HIENG End.
	// ZeusCD CQ386774 OPS1八代 2014/02/25 START.
	{ IPPE_Media_custom_max, 		FEED_ORIENTATION_END,	IPPE_MediaSource_ByPass	 },
//	{ IPPE_Media_custom_min, 		FEED_ORIENTATION_END,	IPPE_MediaSource_ByPass	 },
	// ZeusCD CQ386774 OPS1八代 2014/02/25 END.
//	{ IPPE_Media_NONE, 				FEED_ORIENTATION_END,	IPPE_MediaSource_NONE	 },		// ZeusCD CQ386774 OPS1八代 2014/02/25.
//Zeus5BK 制御構成変更No172 OPS1前田 2014/05/13 END.
#ifdef DEF_A4_Cus // VenusMLK 2OPS Yamaguchi 20151005 START.
	{ IPPE_Media_A6, 				SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray1	 },
	{ IPPE_Media_B5, 				SHORT_EDGE_FIRST,		IPPE_MediaSource_ByPass	 },
	{ IPPE_Media_B5, 				SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray1	 },
	{ IPPE_Media_B5, 				SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray2	 },
	{ IPPE_Media_B5, 				SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray3	 },
#ifdef DEF_KRONOSS_Cus // KronosS 用紙サイズカスタマイズ対応 START.
	{ IPPE_Media_B5, 				SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray4	 },
#endif	// KronosS 用紙サイズカスタマイズ対応 END.
	{ IPPE_Media_B6, 				SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray1	 },
	{ IPPE_Media_7Qx10H, 			SHORT_EDGE_FIRST,		IPPE_MediaSource_ByPass	 },
	{ IPPE_Media_7Qx10H, 			SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray1	 },
	{ IPPE_Media_7Qx10H, 			SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray2	 },
	{ IPPE_Media_7Qx10H, 			SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray3	 },
#ifdef DEF_KRONOSS_Cus // KronosS 用紙サイズカスタマイズ対応 START.
	{ IPPE_Media_7Qx10H, 			SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray4	 },
#endif	// KronosS 用紙サイズカスタマイズ対応 END.
#ifndef DEF_KRONOSS_Cus // KronosS 用紙サイズカスタマイズ対応 KronosSの場合は非サポート START.
	// KronosSでは非サポート.
	{ IPPE_Media_JPSHagaki, 		SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray1	 },
#endif	// KronosS 用紙サイズカスタマイズ対応 END.
	{ IPPE_Media_EnvelopeMonarch,	LONG_EDGE_FIRST,		IPPE_MediaSource_ByPass	 },
#ifndef DEF_KRONOSS_Cus // KronosS 用紙サイズカスタマイズ対応 KronosSの場合は非サポート START.
	// KronosSでは非サポート.
	{ IPPE_Media_EnvelopeCom10,		SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray1	 },
#endif	// KronosS 用紙サイズカスタマイズ対応 END.
	{ IPPE_Media_EnvelopeDL, 		LONG_EDGE_FIRST,		IPPE_MediaSource_ByPass	 },
	{ IPPE_Media_EnvelopeC6, 		LONG_EDGE_FIRST,		IPPE_MediaSource_ByPass	 },
#ifndef DEF_KRONOSS_Cus // KronosS 用紙サイズカスタマイズ対応 KronosSの場合は非サポート START.
	// KronosSでは非サポート.
	{ IPPE_Media_chou3, 			SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray1	 },
#endif	// KronosS 用紙サイズカスタマイズ対応 END.
	{ IPPE_Media_8x10H, 			SHORT_EDGE_FIRST,		IPPE_MediaSource_ByPass	 },
	{ IPPE_Media_8x10H, 			SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray1	 },		// VenusMLK 2OPS Yamaguchi 20151005.
	{ IPPE_Media_JPNOufuku,			SHORT_EDGE_FIRST,		IPPE_MediaSource_ByPass	 },
#ifndef DEF_KRONOSS_Cus // KronosS 用紙サイズカスタマイズ対応 KronosSの場合は非サポート START.
	// KronosSでは非サポート.
	{ IPPE_Media_JPNOufuku, 		SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray1	 },
#endif	// KronosS 用紙サイズカスタマイズ対応 END.
	{ IPPE_Media_8Hx1269, 			SHORT_EDGE_FIRST,		IPPE_MediaSource_ByPass	 },
	{ IPPE_Media_8Hx1269, 			SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray1	 },
	{ IPPE_Media_8x10, 				SHORT_EDGE_FIRST,		IPPE_MediaSource_ByPass	 },
	{ IPPE_Media_A6Card, 			SHORT_EDGE_FIRST,		IPPE_MediaSource_Tray1	 },		// VenusMLK 2OPS Yamaguchi 20151005.
	{ IPPE_Media_you3, 				LONG_EDGE_FIRST,		IPPE_MediaSource_ByPass	 },		// VenusMLK 2OPS Yamaguchi 20151005.
	{ IPPE_Media_A4custom_max, 		FEED_ORIENTATION_END,	IPPE_MediaSource_ByPass	 },		// VenusMLK 2OPS Yamaguchi 20151005.
	{ IPPE_Media_A4custom_min, 		FEED_ORIENTATION_END,	IPPE_MediaSource_ByPass	 },		
#endif // VenusMLK 2OPS Yamaguchi 20151005 END.
};

void IPPC_LibOpeGetPrintAttr::MediaColDatabase( IPP_Media_Col (&Media_Col_Database)[MAX_IPP_MEDIA_COL_DATABASE], INT32 First_Index, INT32 Limit ) // MinervaZeus5 UV-01429 AppleAirPrint1.4対応 Bairavi 2014/05/31.
{
	memset( Media_Col_Database, NULL, sizeof(Media_Col_Database) );

	uchar uc_Count = 0;
	// Poseidon UV-01429 AppleAirPrint1.4対応 Bairavi 2014/05/31 START.
	uchar uc_loop = 0;
	INT32 i_Start = 0 ;
	INT32 i_End = 0;
	// Poseidon UV-01429 AppleAirPrint1.4対応 Bairavi 2014/05/31 END.

	IPPS_MediaSize st_MediaSize;
	memset( &st_MediaSize, NULL, sizeof(IPPS_MediaSize) );

	IPPE_AttachLCT e_AttachLCT 		= Judge_AttachLCTType();
#ifdef DEF_A4_Cus // CQ521331 2OPS Yamaguchi 20151104 START.
	Bool bl_AttachTray2	   			= isAttachTray2();
#endif // CQ521331 VenusMLK 2OPS Yamaguchi 20151104 END.
	Bool bl_AttachTray3	   			= isAttachTray3();
	Bool bl_AttachTray4		   		= isAttachTray4();
	Bool bl_AttachTray5		   		= isAttachTray5();						//2018/05/31 DenebMLK 機器構成変更_No.7 HCL-VIJAY Added. 
	TYPE_SystemType e_SystemType	= pc_MachineSetting->getSystemType();
// 2021/05/24 [IT6_2.2] OP_BTS-33816 HIENG Start.
#ifdef DEF_DenebMLKCus
	TYPE_SubSystemType e_SubSystem = pc_MachineSetting->getSubSystemType();
#endif
// 2021/05/24 [IT6_2.2] OP_BTS-33816 HIENG End.
	//Zeus5BK 制御構成変更No172 OPS1前田 2014/05/13 START.
	IPPE_Media e_Media = CnvSpec_Media( TYPE_SizeCodeFlsSef );
	//Zeus5BK 制御構成変更No172 OPS1前田 2014/05/13 END.
	// Poseidon UV-01429 AppleAirPrint1.4対応 Bairavi 2014/05/31 START.
	if ((First_Index == 1) && (Limit == 0)) {
		i_Start = First_Index;
		i_End = sizeof( st_MediaColDatbase ) / sizeof( IPPS_MediaColDatbase );
	} else {
		i_Start = First_Index;
		i_End = Limit;
	}
	// Poseidon UV-01429 AppleAirPrint1.4対応 Bairavi 2014/05/31 END.

	for( ushort i = 0; i < sizeof( st_MediaColDatbase ) / sizeof( IPPS_MediaColDatbase ); i++ ) {
#ifdef DEF_A4_Cus // VenusMLK先行開発 2OPS Yamaguchi.
		// not supported tray by VenusMLK
		if(( IPPE_MediaSource_LCT == st_MediaColDatbase[i].e_MediaSource )
#ifndef DEF_KRONOSS_Cus // KronosS 用紙サイズカスタマイズ対応 KronosSの場合はチェックしない START.
		|| ( IPPE_MediaSource_Tray4 == st_MediaColDatbase[i].e_MediaSource )
#endif // KronosS 用紙サイズカスタマイズ対応 END.
		|| ( IPPE_MediaSource_NONE == st_MediaColDatbase[i].e_MediaSource ) ) {
			continue;
		}
		// not supported media by VenusMLK.
		if(( IPPE_Media_A3 == st_MediaColDatbase[i].e_Media )
		|| ( IPPE_Media_B4 == st_MediaColDatbase[i].e_Media )
		|| ( IPPE_Media_11x17 == st_MediaColDatbase[i].e_Media )
		|| ( IPPE_Media_SRA3 == st_MediaColDatbase[i].e_Media )
		|| ( IPPE_Media_220x330 == st_MediaColDatbase[i].e_Media )
		|| ( IPPE_Media_KAI8 == st_MediaColDatbase[i].e_Media )
		|| ( IPPE_Media_12x18 == st_MediaColDatbase[i].e_Media )
		|| ( IPPE_Media_EnvelopeC5 == st_MediaColDatbase[i].e_Media )
		|| ( IPPE_Media_EnvelopeC4 == st_MediaColDatbase[i].e_Media )
		|| ( IPPE_Media_you4 == st_MediaColDatbase[i].e_Media )
		|| ( IPPE_Media_kaku1 == st_MediaColDatbase[i].e_Media )
		|| ( IPPE_Media_kaku2 == st_MediaColDatbase[i].e_Media )
		|| ( IPPE_Media_kaku3 == st_MediaColDatbase[i].e_Media )
		|| ( IPPE_Media_custom_min == st_MediaColDatbase[i].e_Media )
		|| ( IPPE_Media_custom_max == st_MediaColDatbase[i].e_Media )) {
			continue;
		}
		
		// followings are media supported
		// but not supported cases for tray or feed direction.
		if( IPPE_Media_A4 == st_MediaColDatbase[i].e_Media ) {
			if( LONG_EDGE_FIRST == st_MediaColDatbase[i].e_Feed_Direction ) {
				continue;
			}
		}
		
		if( IPPE_Media_A5 == st_MediaColDatbase[i].e_Media ) {
			if( LONG_EDGE_FIRST == st_MediaColDatbase[i].e_Feed_Direction ) {
				continue;
			}
#ifndef DEF_KRONOSS_Cus // KronosS 用紙サイズカスタマイズ対応 KronosSの場合はチェックしない START.
			// KronosSではチェックしない.
			if( SHORT_EDGE_FIRST == st_MediaColDatbase[i].e_Feed_Direction ){
				if(( IPPE_MediaSource_Tray2 == st_MediaColDatbase[i].e_MediaSource )
				|| ( IPPE_MediaSource_Tray3 == st_MediaColDatbase[i].e_MediaSource ) ) {
					continue;
				}
			}
#endif // KronosS 用紙サイズカスタマイズ対応 END.
		}
		if(( IPPE_Media_B5 == st_MediaColDatbase[i].e_Media)
		&& ( FEED_ORIENTATION_END == st_MediaColDatbase[i].e_Feed_Direction )) {
			continue;
		}
		if(( IPPE_Media_7Qx10H == st_MediaColDatbase[i].e_Media)
		&& ( FEED_ORIENTATION_END == st_MediaColDatbase[i].e_Feed_Direction )) {
			continue;
		}
		if( IPPE_Media_5Hx8H == st_MediaColDatbase[i].e_Media ) {
			if( LONG_EDGE_FIRST == st_MediaColDatbase[i].e_Feed_Direction ) {
				continue;
			}
			if( SHORT_EDGE_FIRST == st_MediaColDatbase[i].e_Feed_Direction ){
				if(( IPPE_MediaSource_Tray2 == st_MediaColDatbase[i].e_MediaSource )
#ifdef DEF_KRONOSS_Cus // KronosS 用紙サイズカスタマイズ対応 START.
				|| ( IPPE_MediaSource_Tray3 == st_MediaColDatbase[i].e_MediaSource )
				|| ( IPPE_MediaSource_Tray4 == st_MediaColDatbase[i].e_MediaSource ) ) {
#else
				|| ( IPPE_MediaSource_Tray3 == st_MediaColDatbase[i].e_MediaSource ) ) {
#endif // KronosS 用紙サイズカスタマイズ対応 END.
					continue;
				}
			}
		}
		if(( IPPE_Media_8x13 == st_MediaColDatbase[i].e_Media )
		&& ( SHORT_EDGE_FIRST == st_MediaColDatbase[i].e_Feed_Direction )){
			if(( IPPE_MediaSource_Tray2 == st_MediaColDatbase[i].e_MediaSource )
#ifdef DEF_KRONOSS_Cus // KronosS 用紙サイズカスタマイズ対応 START.
			|| ( IPPE_MediaSource_Tray3 == st_MediaColDatbase[i].e_MediaSource )
			|| ( IPPE_MediaSource_Tray4 == st_MediaColDatbase[i].e_MediaSource ) ) {
#else
			|| ( IPPE_MediaSource_Tray3 == st_MediaColDatbase[i].e_MediaSource ) ) {
#endif // KronosS 用紙サイズカスタマイズ対応 END.
				continue;
			}
		}
		if(( IPPE_Media_8125x13Q == st_MediaColDatbase[i].e_Media )
		&& ( SHORT_EDGE_FIRST == st_MediaColDatbase[i].e_Feed_Direction )){
			if(( IPPE_MediaSource_Tray2 == st_MediaColDatbase[i].e_MediaSource )
#ifdef DEF_KRONOSS_Cus // KronosS 用紙サイズカスタマイズ対応 START.
			|| ( IPPE_MediaSource_Tray3 == st_MediaColDatbase[i].e_MediaSource )
			|| ( IPPE_MediaSource_Tray4 == st_MediaColDatbase[i].e_MediaSource ) ) {
#else
			|| ( IPPE_MediaSource_Tray3 == st_MediaColDatbase[i].e_MediaSource ) ) {
#endif // KronosS 用紙サイズカスタマイズ対応 END.
				continue;
			}
		}
		if(( IPPE_Media_8Qx13 == st_MediaColDatbase[i].e_Media )
		&& ( SHORT_EDGE_FIRST == st_MediaColDatbase[i].e_Feed_Direction )){
			if(( IPPE_MediaSource_Tray2 == st_MediaColDatbase[i].e_MediaSource )
#ifdef DEF_KRONOSS_Cus // KronosS 用紙サイズカスタマイズ対応 START.
			|| ( IPPE_MediaSource_Tray3 == st_MediaColDatbase[i].e_MediaSource )
			|| ( IPPE_MediaSource_Tray4 == st_MediaColDatbase[i].e_MediaSource ) ) {
#else
			|| ( IPPE_MediaSource_Tray3 == st_MediaColDatbase[i].e_MediaSource ) ) {
#endif // KronosS 用紙サイズカスタマイズ対応 END.
				continue;
			}
		}
#ifndef DEF_KRONOSS_Cus // KronosS 用紙サイズカスタマイズ対応 KronosSの場合はチェックしない START.
			// KronosSではチェックしない.
		if(( IPPE_Media_8Hx13 == st_MediaColDatbase[i].e_Media )
		&& ( SHORT_EDGE_FIRST == st_MediaColDatbase[i].e_Feed_Direction )){
			if(( IPPE_MediaSource_Tray2 == st_MediaColDatbase[i].e_MediaSource )
			|| ( IPPE_MediaSource_Tray3 == st_MediaColDatbase[i].e_MediaSource ) ) {
				continue;
			}
		}
#endif // KronosS 用紙サイズカスタマイズ対応 END.
		if(( IPPE_Media_8Hx13H == st_MediaColDatbase[i].e_Media )
		&& ( SHORT_EDGE_FIRST == st_MediaColDatbase[i].e_Feed_Direction )){
			if(( IPPE_MediaSource_Tray2 == st_MediaColDatbase[i].e_MediaSource )
#ifdef DEF_KRONOSS_Cus // KronosS 用紙サイズカスタマイズ対応 START.
			|| ( IPPE_MediaSource_Tray3 == st_MediaColDatbase[i].e_MediaSource )
			|| ( IPPE_MediaSource_Tray4 == st_MediaColDatbase[i].e_MediaSource ) ) {
#else
			|| ( IPPE_MediaSource_Tray3 == st_MediaColDatbase[i].e_MediaSource ) ) {
#endif // KronosS 用紙サイズカスタマイズ対応 END.
				continue;
			}
		}
		if(( IPPE_Media_KAI16 == st_MediaColDatbase[i].e_Media )
		&& ( LONG_EDGE_FIRST == st_MediaColDatbase[i].e_Feed_Direction )){
			continue;
		}
		if(( IPPE_Media_EnvelopeMonarch == st_MediaColDatbase[i].e_Media )
#ifdef DEF_KRONOSS_Cus // KronosS 用紙サイズカスタマイズ対応 START.
		&& ( LONG_EDGE_FIRST == st_MediaColDatbase[i].e_Feed_Direction )){
#else
		&& ( SHORT_EDGE_FIRST == st_MediaColDatbase[i].e_Feed_Direction )){
#endif // KronosS 用紙サイズカスタマイズ対応 END.
			continue;
		}
		if(( IPPE_Media_EnvelopeDL == st_MediaColDatbase[i].e_Media )
#ifdef DEF_KRONOSS_Cus // KronosS 用紙サイズカスタマイズ対応 START.
		&& ( LONG_EDGE_FIRST == st_MediaColDatbase[i].e_Feed_Direction )){
#else
		&& ( SHORT_EDGE_FIRST == st_MediaColDatbase[i].e_Feed_Direction )){
#endif // KronosS 用紙サイズカスタマイズ対応 END.
			continue;
		}
		if(( IPPE_Media_EnvelopeC6 == st_MediaColDatbase[i].e_Media )
#ifdef DEF_KRONOSS_Cus // KronosS 用紙サイズカスタマイズ対応 START.
		&& ( LONG_EDGE_FIRST == st_MediaColDatbase[i].e_Feed_Direction )){
#else
		&& ( SHORT_EDGE_FIRST == st_MediaColDatbase[i].e_Feed_Direction )){
#endif // KronosS 用紙サイズカスタマイズ対応 END.
			continue;
		}
		if(( IPPE_Media_you3 == st_MediaColDatbase[i].e_Media )
#ifdef DEF_KRONOSS_Cus // KronosS 用紙サイズカスタマイズ対応 START.
		&& ( LONG_EDGE_FIRST == st_MediaColDatbase[i].e_Feed_Direction )){
#else
		&& ( SHORT_EDGE_FIRST == st_MediaColDatbase[i].e_Feed_Direction )){
#endif // KronosS 用紙サイズカスタマイズ対応 END.
			continue;
		}
#endif // DEF_A4_Cus CQ.516592,Warning対応 2OPS Yamaguchi.

		if( IPPE_MediaSource_LCT == st_MediaColDatbase[i].e_MediaSource ) {

			if( IPPE_AttachLCT_None  == e_AttachLCT ) {
				continue;
			}

			if( IPPE_AttachLCT_LU302 == e_AttachLCT ) {						//2018/05/31 DenebMLK 制御構成変更_No.1 HCL-VIJAY Modified.
				if( ( ( IPPE_Media_A4    == st_MediaColDatbase[i].e_Media ) && ( LONG_EDGE_FIRST == st_MediaColDatbase[i].e_Feed_Direction ) )
				 || ( ( IPPE_Media_8Hx11 == st_MediaColDatbase[i].e_Media ) && ( LONG_EDGE_FIRST == st_MediaColDatbase[i].e_Feed_Direction ) ) ) {
				} else {
					continue;
				}
			}
//OP_BTS-34283 [IT6-2.2] HIENG Start.
#ifdef DEF_EagleCus
			// A3LCT.
			if (IPPE_AttachLCT_LU207 == e_AttachLCT ) {
				if ((IPPE_Media_A3 == st_MediaColDatbase[i].e_Media) 
				|| (IPPE_Media_A4 == st_MediaColDatbase[i].e_Media)
				|| (IPPE_Media_B4 == st_MediaColDatbase[i].e_Media)
				|| (IPPE_Media_11x17 == st_MediaColDatbase[i].e_Media)
				|| (IPPE_Media_8Hx14 == st_MediaColDatbase[i].e_Media)
				|| (IPPE_Media_8Hx11 == st_MediaColDatbase[i].e_Media)
				|| (IPPE_Media_SRA3 == st_MediaColDatbase[i].e_Media)
				|| (IPPE_Media_12x18 == st_MediaColDatbase[i].e_Media) ) {
				}
				else {
					// Other media is not supported for Eagle 1/2/2.5 and Eagle Z/X/O
					continue;
				}
			}
#endif
//OP_BTS-34283 [IT6-2.2] HIENG End.
		}

#ifdef DEF_A4_Cus // CQ521331 2OPS Yamaguchi 20151104 START.
		if( IPPE_MediaSource_Tray2 == st_MediaColDatbase[i].e_MediaSource ) {
			if( False == bl_AttachTray2 ) {
				continue;
			}
		}
#endif // CQ521331 VenusMLK 2OPS Yamaguchi 20151104 END.

		if( IPPE_MediaSource_Tray3 == st_MediaColDatbase[i].e_MediaSource ) {
			if( False == bl_AttachTray3 ) {
				continue;
			}
		}

		if( IPPE_MediaSource_Tray4 == st_MediaColDatbase[i].e_MediaSource ) {
			if( False == bl_AttachTray4 ) {
				continue;
			}
		}

		//2018/05/31 DenebMLK 機器構成変更_No.7 HCL-VIJAY Added Start.		
		if( IPPE_MediaSource_Tray5 == st_MediaColDatbase[i].e_MediaSource ) {
			if( False == bl_AttachTray5 ) {
				continue;
			}
		}
		//2018/05/31 DenebMLK 機器構成変更_No.7 HCL-VIJAY Added End.

#ifndef DEF_A4_Cus // VenusMLK 2OPS Yamaguchi 20151005 START.
		// A3 device only (A4 device supports A5SEF Tray1 for SystemTypeUS and 5Hx8HSEF for any marketing area).
		if( ( IPPE_Media_A5 == st_MediaColDatbase[i].e_Media ) && ( SHORT_EDGE_FIRST == st_MediaColDatbase[i].e_Feed_Direction ) ) {
			if( IPPE_MediaSource_ByPass != st_MediaColDatbase[i].e_MediaSource ) {
				if( TYPE_SystemType_US == e_SystemType ) {
					continue;
				}
			}
		}

		if( ( IPPE_Media_5Hx8H == st_MediaColDatbase[i].e_Media  ) && ( SHORT_EDGE_FIRST == st_MediaColDatbase[i].e_Feed_Direction ) ) {
			if( IPPE_MediaSource_ByPass != st_MediaColDatbase[i].e_MediaSource ) {
				if( ( TYPE_SystemType_JP == e_SystemType ) || ( TYPE_SystemType_EU == e_SystemType ) ) {
					continue;
				}
			}
		}
#endif // VenusMLK 2OPS Yamaguchi 20151005 END.
		
		//Zeus5BK 制御構成変更No172 OPS1前田 2014/05/13 START.
		// HelosL 制御No1 OSP1八代 2014.11.19 START.
#if ( defined DEF_Zeus5BkCus || defined DEF_HeliosCus || defined DEF_HeliosMLKCus )
//OP_BTS-34283 [IT6-2.2] HIENG Start.
		if (( IPPE_Media_A3 == st_MediaColDatbase[i].e_Media ) && ( IPPE_MediaSource_ByPass != st_MediaColDatbase[i].e_MediaSource )) {
			continue;
		}
		if (( IPPE_Media_A6 == st_MediaColDatbase[i].e_Media ) && ( IPPE_MediaSource_ByPass != st_MediaColDatbase[i].e_MediaSource )) {
			continue;
		}	
		if (( IPPE_Media_B6 == st_MediaColDatbase[i].e_Media) && (IPPE_MediaSource_ByPass != st_MediaColDatbase[i].e_MediaSource)) {
			continue;
		}
		if ((IPPE_Media_11x17 == st_MediaColDatbase[i].e_Media) && (IPPE_MediaSource_Tray1 == st_MediaColDatbase[i].e_MediaSource )) {
			continue;
		}
//OP_BTS-34283 [IT6-2.2] HIENG End.
		if( ( IPPE_Media_SRA3 == st_MediaColDatbase[i].e_Media ) || ( IPPE_Media_12x18 == st_MediaColDatbase[i].e_Media ) ) {
			continue;
		}
//OP_BTS-34283 [IT6-2.2] HIENG Start.
		if ((IPPE_Media_7Qx10H == st_MediaColDatbase[i].e_Media) && (IPPE_MediaSource_ByPass != st_MediaColDatbase[i].e_MediaSource )) {
			continue;
		}
		
		if (IPPE_Media_12x18 == st_MediaColDatbase[i].e_Media ) {
			continue;
		}
		
		if( IPPE_Media_JPSHagaki == st_MediaColDatbase[i].e_Media && IPPE_MediaSource_ByPass != st_MediaColDatbase[i].e_MediaSource) {
			continue;
		}
		
		if (IPPE_Media_8125x13Q == st_MediaColDatbase[i].e_Media && IPPE_MediaSource_ByPass != st_MediaColDatbase[i].e_MediaSource) {
			continue;
		}

		if (IPPE_Media_220x330 == st_MediaColDatbase[i].e_Media && IPPE_MediaSource_ByPass != st_MediaColDatbase[i].e_MediaSource) {
			continue;
		}
//OP_BTS-34283 [IT6-2.2] HIENG End.
#endif //#if ( DEF_Zeus5BkCus || DEF_HeliosCus || defined DEF_HeliosMLKCus )
		// HelosL 制御No1 OSP1八代 2014.11.19 END.
//OP_BTS-34283 [IT6-2.2] HIENG Start.
#ifdef DEF_EagleCus
		if ( (IPPE_Media_A6 == st_MediaColDatbase[i].e_Media) && (IPPE_MediaSource_ByPass != st_MediaColDatbase[i].e_MediaSource) ) {
			if (IPPE_MediaSource_Tray1 !=st_MediaColDatbase[i].e_Media) {
				continue;
			} else if(TYPE_SystemType_US == e_SystemType) { // Tray1 don't support for US area.
				continue;
			}
			
		}
		
		if ( (IPPE_Media_B6 == st_MediaColDatbase[i].e_Media) && (IPPE_MediaSource_ByPass != st_MediaColDatbase[i].e_MediaSource) ) {
			if (TYPE_SystemType_US == e_SystemType) {
				continue;
			}
		}
		
		if ( (IPPE_Media_7Qx10H == st_MediaColDatbase[i].e_Media) && (IPPE_MediaSource_ByPass != st_MediaColDatbase[i].e_MediaSource) ) {
			continue;
		}
#endif // End DEF_EagleCus.
//OP_BTS-34283 [IT6-2.2] HIENG End.

		if( IPPE_Media_A6Card == st_MediaColDatbase[i].e_Media ) {
			if( TYPE_SystemType_EU != e_SystemType ) {
				continue;
			}
		}

		// ZeusCD CQ390843 OPS1八代 2014/06/02 START.
		if( IPPE_Media_JPSHagaki == st_MediaColDatbase[i].e_Media ) {
			if( TYPE_SystemType_JP != e_SystemType ) {
				continue;
			}
		}
		// ZeusCD CQ390843 OPS1八代 2014/06/02 END.

		// ZeusS CQ510893 OPS1八代 2015.2.24 START.
		if( ( IPPE_Media_4x6 == st_MediaColDatbase[i].e_Media ) && ( TYPE_SystemType_US != e_SystemType ) ){
			continue;
		}
		// ZeusS CQ510893 OPS1八代 2015.2.24 END.

#ifdef DEF_DenebMLKCus	// EagleZ+ 制御構成変更No.36 MIF機対応.
// 2021/05/24 [IT6_2.2] OP_BTS-33816 HIENG Start.
		if(( IPPE_Media_11x17     == st_MediaColDatbase[i].e_Media ) 
		&& ( IPPE_MediaSource_ByPass != st_MediaColDatbase[i].e_MediaSource )) {
			
			if( IPPE_MediaSource_LCT == st_MediaColDatbase[i].e_MediaSource ) {
				continue;
			}
			// Tray 1,2,3,4,5.
			if ( TYPE_SystemType_US != e_SystemType ) {
				continue;
			}
		}

		if(( IPPE_Media_SRA3      == st_MediaColDatbase[i].e_Media )
		|| ( IPPE_Media_JPSHagaki == st_MediaColDatbase[i].e_Media )
		|| ( IPPE_Media_4x6       == st_MediaColDatbase[i].e_Media )
		|| ( IPPE_Media_12x18     == st_MediaColDatbase[i].e_Media )) {
			continue;
		}

		if(( IPPE_Media_KAI8  == st_MediaColDatbase[i].e_Media )
		|| ( IPPE_Media_KAI16 == st_MediaColDatbase[i].e_Media )) {
			// Not China Market.
			// China Market (Other3) when e_SystemType is TYPE_SystemType_EU and e_SubSystem is TYPE_SubSystemType4.
			if (!((TYPE_SystemType_EU == e_SystemType) && (TYPE_SubSystemType4 == e_SubSystem ))) {
				continue;
			}
		}
// 2021/05/24 [IT6_2.2] OP_BTS-33816 HIENG End.
#endif

		if( ( IPPE_Media_8x13     == st_MediaColDatbase[i].e_Media )
		 || ( IPPE_Media_8125x13Q == st_MediaColDatbase[i].e_Media )
		 || ( IPPE_Media_8Qx13    == st_MediaColDatbase[i].e_Media )
		 || ( IPPE_Media_8Hx13    == st_MediaColDatbase[i].e_Media )
		 || ( IPPE_Media_8Hx13H   == st_MediaColDatbase[i].e_Media )
		 || ( IPPE_Media_220x330  == st_MediaColDatbase[i].e_Media ) ){
			if( e_Media != st_MediaColDatbase[i].e_Media ){
				continue;
			}
		}
		//Zeus5BK 制御構成変更No172 OPS1前田 2014/05/13 END.

		// Poseidon UV-01429 AppleAirPrint1.4対応 Bairavi 2014/05/31 START.
		// Skip till First-Index is reached.
		if(uc_loop < (i_Start-1)){
			uc_loop++;
			continue;	
		}
		// Poseidon UV-01429 AppleAirPrint1.4対応 Bairavi 2014/05/31 END.

		Media_Col_Database[uc_Count].set = 1;

		Media_Col_Database[uc_Count].Media_Col_Botton_Margin = IPPD_MediaMargin;
		Media_Col_Database[uc_Count].Media_Col_Top_Margin    = IPPD_MediaMargin;
		Media_Col_Database[uc_Count].Media_Col_Right_Margin  = IPPD_MediaMargin;
		Media_Col_Database[uc_Count].Media_Col_Left_Margin   = IPPD_MediaMargin;

		strcpy( (char*)(Media_Col_Database[uc_Count].Media_Col_Source), Get_MediaSource( st_MediaColDatbase[i].e_MediaSource ) );
		strcpy( (char*)(Media_Col_Database[uc_Count].Media_Col_Type),   "" );															// media-typeは省略可能.

		st_MediaSize = Calc_MediaSize( Get_Media( st_MediaColDatbase[i].e_Media ) );

		Media_Col_Database[uc_Count].Media_Size[0].set            = st_MediaSize.set;
		// ZeusCD CQ386774 OPS1八代 2014/02/25 START.
		Media_Col_Database[uc_Count].Media_Size[0].X_Dimension[0] = st_MediaSize.X_Dimension[0];
		Media_Col_Database[uc_Count].Media_Size[0].Y_Dimension[0] = st_MediaSize.Y_Dimension[0];
		Media_Col_Database[uc_Count].Media_Size[0].X_Dimension[1] = st_MediaSize.X_Dimension[1];
		Media_Col_Database[uc_Count].Media_Size[0].Y_Dimension[1] = st_MediaSize.Y_Dimension[1];
		// ZeusCD CQ386774 OPS1八代 2014/02/25 END.

		Media_Col_Database[uc_Count].Media_Source_Properties.set = 1;
		Media_Col_Database[uc_Count].Media_Source_Properties.Media_Source_Feed_Direction   = st_MediaColDatbase[i].e_Feed_Direction;
		Media_Col_Database[uc_Count].Media_Source_Properties.Media_Source_Feed_Orientation = ORIENT_END;								// media-source-feed-orientation は省略可能.


		memset( &st_MediaSize, NULL, sizeof(IPPS_MediaSize) );

		uc_Count++;
		// Poseidon UV-01429 AppleAirPrint1.4対応 Bairavi 2014/05/31 START.
		// Exit when Limit is reached.
		if(uc_Count >= i_End) {
			break;	
		}
		// Poseidon UV-01429 AppleAirPrint1.4対応 Bairavi 2014/05/31 END.
		// 次順で、NICFUMが用意している配列の要素数を超えるようなら.
		// 不正な配列番号を参照してしまうため、(ログを出力した上で)、処理を抜ける.
		// (このログが出力されるということは、NICFUMに要素数の追加を依頼する必要があるということ).
		if( MAX_IPP_MEDIA_COL_DATABASE <= uc_Count ) {
			DIPPTRACE2( "[IPP][fail safe] MediaColDatabase uc_Count = %d i = %d\n",uc_Count, i );
			break;
		}
	}

	return;
}

/*================================================================================================
［機能］	設定関数 output-bin-default.
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2014/01/31 新規作成 CQ384241 ZeusCD UV-01249 AppleAirPrint.
================================================================================================*/
IPP_OUTPUT_BIN IPPC_LibOpeGetPrintAttr::OutputBinDefault()
{
	IPP_OUTPUT_BIN e_Return = OUTPUT_BIN_END;

	pc_CopyJobSetting->lock();
	TYPE_OutputTray e_OutputTray = pc_CopyJobSetting->getOutputTrayForPrint();
	pc_CopyJobSetting->unlock();

	IPPE_OutputBin	e_OutputBin = CnvSpec_OutputBin( e_OutputTray );

	switch( e_OutputBin ) {
		case IPPE_OutputBin_Top:	e_Return = TOP;				break;
		case IPPE_OutputBin_Middle: e_Return = MIDDLE;			break;
		case IPPE_OutputBin_Bottom: e_Return = BOTTOM;			break;
		case IPPE_OutputBin_Right:	e_Return = RIGHT;			break;		// ZeusS ZX0 機器構成変更_No.3対応 HCL-VIJAY 2015/06/22 Added.
#ifdef DEF_A4_Cus // VenusMLK 2OPS Yamaguchi 20151006 START.
		case IPPE_OutputBin_FaceDown:	e_Return = FACE_DOWN;	break;
#endif // VenusMLK先行開発 2OPS Yamaguchi 20151006 END.
		default: 					e_Return = OUTPUT_BIN_END;	break;
	}

	return e_Return;
}

/*================================================================================================
［機能］	設定関数 output-bin-supported.
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2014/01/31 新規作成 CQ384241 ZeusCD UV-01249 AppleAirPrint.
================================================================================================*/
void IPPC_LibOpeGetPrintAttr::OutputBinSupported( IPP_OUTPUT_BIN  (&Output_Bin)[MAX_IPP_OUTPUT_BIN] )
{
	for(int i=0; i < MAX_IPP_OUTPUT_BIN; i++ ) Output_Bin[i] = OUTPUT_BIN_END;

	IPPE_AttachFS e_AttachFS = Judge_AttachFSType();

	pc_PrinterSystem->lock();
	Bool bl_OptionTray3  = pc_PrinterSystem->isOptionTray3();
	Bool bl_OptionTray4  = pc_PrinterSystem->isOptionTray4();	// ZeusS ZX0 機器構成変更_No.3対応 HCL-VIJAY 2015/06/22 Added.
	pc_PrinterSystem->unlock();

	switch( e_AttachFS ) {
		case IPPE_AttachFS_None:
#ifdef DEF_A4_Cus // VenusMLK 2OPS Yamaguchi 20151006 START.
			Output_Bin[0] = FACE_DOWN;
#else // A3.
			Output_Bin[0] = TOP;
#endif // VenusMLK 2OPS Yamaguchi 20151006 END.
			break;

		case IPPE_AttachFS_JS602:
			Output_Bin[0] = TOP;
			Output_Bin[1] = MIDDLE;
			break;

		case IPPE_AttachFS_FS542:	// 2022/06/27 FPT-LucNH2 【IT6_2.3】 制御構成変更_No.2 Add.
		case IPPE_AttachFS_FS533:
			Output_Bin[0] = TOP;
			break;

		case IPPE_AttachFS_FS534:	// ZeusCD CQ386422 OPS1八代 2014/02/18.
		case IPPE_AttachFS_FS535:
		case IPPE_AttachFS_FS537:	// MinervaSBK 制御No15_FS-537 OPS1八代 2015.04.06.
		case IPPE_AttachFS_FS540:	// 2018/10/17 FPT-LucNH2 【IT6_2.0】【EagleH】 機器構成変更_No.5 Add.
		// ZeusS ZX0 機器構成変更_No.3対応 HCL-VIJAY 2015/06/22 Added START.
			Output_Bin[0] = TOP;
			Output_Bin[1] = MIDDLE;
			
			if( True == bl_OptionTray3 ) {
				Output_Bin[2] = BOTTOM;
				if( True == bl_OptionTray4 ) {		// JSオプショントレイとRU増設トレイ装着時
				  Output_Bin[3] = RIGHT;	
				}
			} else if( True == bl_OptionTray4 ) {	// RU増設トレイ装着時(JSオプショントレイは非装着)
				Output_Bin[2] = RIGHT;
			} else {
							
			}
		// ZeusS ZX0 機器構成変更_No.3対応 HCL-VIJAY 2015/06/22 Added END.
			break;

		// MinervaSBK 制御No14_FS-536 OPS1八代 2015.03.26 START.
		case IPPE_AttachFS_FS536:
		case IPPE_AttachFS_FS539:	// 2018/10/17 FPT-LucNH2 【IT6_2.0】【EagleH】 機器構成変更_No.5 Add.
			Output_Bin[0] = TOP;
			Output_Bin[1] = MIDDLE;
		// ZeusS ZX0 機器構成変更_No.3対応 HCL-VIJAY 2015/06/22 Added START.
			if( True == bl_OptionTray4 ) {		// RU増設トレイ装着時
				Output_Bin[2] = RIGHT;
			} 
		// ZeusS ZX0 機器構成変更_No.3対応 HCL-VIJAY 2015/06/22 Added END.			
			break;
#ifdef DEF_A4_Cus // VenusMLK 2OPS Yamaguchi 20151006 START.
		case IPPE_AttachFS_VenusFNS:
#ifdef DEF_KRONOSS_Cus // KronosS フィニッシャ対応 START.
		case IPPE_AttachFS_FSP02:
#endif // KronosS フィニッシャ対応 END.
			Output_Bin[0] = FACE_DOWN;
			break;
#endif // VenusMLK 2OPS Yamaguchi 20151006 END.

		// MinervaSBK 制御No14_FS-536 OPS1八代 2015.03.26 END.


		default:
			Output_Bin[0] = TOP;
			break;
	}

	return;
}
/*================================================================================================
［機能］	設定関数 Printer_Alert.
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2014/06/09 Poseidon UV-01429 AppleAirPrint1.4対応.
================================================================================================*/
void IPPC_LibOpeGetPrintAttr::GetPrinterAlert( UCHAR (&Printer_Alert)[MAX_IPP_PRINTER_ALERT][MAX_STRING128] )
{
	char sc_Alert[20]={'\0'};
	for(ushort i=0;i<MAX_IPP_PRINTER_ALERT;i++)
	{
		memset(sc_Alert, '\0', sizeof(sc_Alert));
		if (MIBE_NoAlert == eCrntAlertTbl[i])
		{
			// Helios CQ-512040 AirPrint1.5対応 HCL-Vijay 2015/04/10 ADDED START
			if(0 == i) {								// If there is no alerts.
				strcpy((char *)Printer_Alert[i],"code=" );
			    sprintf(sc_Alert, "%d", 507 ); 
			    strcat((char *)Printer_Alert[i], sc_Alert);
			    strcat((char *)Printer_Alert[i],";index=");
			    sprintf( sc_Alert, "%ld", 0 ); 
			    strcat((char *)Printer_Alert[i], sc_Alert);
			    strcat((char *)Printer_Alert[i],";severity=");
			    sprintf( sc_Alert, "%d", 1 ); 
			    strcat((char *)Printer_Alert[i], sc_Alert);
			    strcat((char *)Printer_Alert[i],";group=");
			    sprintf(sc_Alert, "%d", 5 ); 
			    strcat((char *)Printer_Alert[i], sc_Alert);
			    strcat((char *)Printer_Alert[i],";groupindex=");
			    sprintf(sc_Alert, "%d", -1 ); 
			    strcat((char *)Printer_Alert[i], sc_Alert);
			    strcat((char *)Printer_Alert[i],";location=");
			    sprintf(sc_Alert, "%ld", -2 ); 
			    strcat((char *)Printer_Alert[i], sc_Alert);
			}
			// Helios CQ-512040 AirPrint1.5対応 HCL-Vijay 2015/04/10 ADDED END
			break;
		} else {
		    strcpy((char *)Printer_Alert[i],"code=" );
		    sprintf(sc_Alert, "%d", s_AlertTblFixed[i].ss_Code ); 
		    strcat((char *)Printer_Alert[i], sc_Alert);
		    strcat((char *)Printer_Alert[i],";index=");
		    sprintf( sc_Alert, "%ld", s_AlertTblFixed[i].sl_Index ); 
		    strcat((char *)Printer_Alert[i], sc_Alert);
		    strcat((char *)Printer_Alert[i],";severity=");
		    sprintf( sc_Alert, "%d", s_AlertTblFixed[i].sc_SeverityLevel ); 
		    strcat((char *)Printer_Alert[i], sc_Alert);
		    strcat((char *)Printer_Alert[i],";group=");
		    sprintf(sc_Alert, "%d", s_AlertTblFixed[i].sc_Group ); 
		    strcat((char *)Printer_Alert[i], sc_Alert);
		    strcat((char *)Printer_Alert[i],";groupindex=");
		    sprintf(sc_Alert, "%d", s_AlertTblFixed[i].sc_GroupIndex ); 
		    strcat((char *)Printer_Alert[i], sc_Alert);
		    strcat((char *)Printer_Alert[i],";location=");
		    sprintf(sc_Alert, "%ld", s_AlertTblFixed[i].sl_Location ); 
		    strcat((char *)Printer_Alert[i], sc_Alert);
		} 
	}
}
/*================================================================================================
［機能］	設定関数 Printer_Alert_Description.
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2014/06/09 Poseidon UV-01429 AppleAirPrint1.4対応.
================================================================================================*/
void IPPC_LibOpeGetPrintAttr::GetPrinterAlertDescription(UCHAR (&Printer_Alert_Description)[MAX_IPP_PRINTER_ALERT][MAX_STRING256])
{
    for(ushort i=0;i<MAX_IPP_PRINTER_ALERT;i++)
    {
    	if(MIBE_NoAlert == eCrntAlertTbl[i]) {
			// Helios CQ-512040 AirPrint1.5対応 HCL-Vijay 2015/04/10 ADDED START
			if(0 == i) {								// If there is no alerts in the MIB alert table.
				strcpy((char *)Printer_Alert_Description[i],"printerReadyToPrint" );
			}
			// Helios CQ-512040 AirPrint1.5対応 HCL-Vijay 2015/04/10 ADDED END
			break;
    	} else {
    		strcpy((char *)Printer_Alert_Description[i], s_AlertTblFixed[i].sc_Description );	
    	} 
    }
}
/*================================================================================================
［機能］	設定関数 finishings-supported.
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2014/01/31 新規作成 CQ384241 ZeusCD UV-01249 AppleAirPrint.
================================================================================================*/
void IPPC_LibOpeGetPrintAttr::FinishingsSupported( IPP_FINISHINGS  (&Finishings)[MAX_IPP_FINISHINGS] )
{
	for(int j=0; j < MAX_IPP_FINISHINGS; j++ ) Finishings[j] = FIN_END;

	uchar uc_Count = 0;

	for( TYPE_IPPFinishings e_IPPFinishings = TYPE_IPPFinishingsNone; e_IPPFinishings < TYPE_IPPFinishingsEnd; e_IPPFinishings++ ) {

		if (True == isEnableFSFunc( e_IPPFinishings )) {

			// KM定義をIPP(NICFUM)定義へ変換する.
			Finishings[uc_Count] = CnvSpec_finishings( e_IPPFinishings );
			uc_Count++;

			// 次順で、NICFUMが用意している配列の要素数を超えるようなら.
			// 不正な配列番号を参照してしまうため、(ログを出力した上で)、処理を抜ける.
			// (このログが出力されるということは、NICFUMに要素数の追加を依頼する必要があるということ).
			if( MAX_IPP_FINISHINGS <= uc_Count ) {
				DIPPTRACE2( "[IPP][fail safe] FinishingsSupported uc_Count = %d e_IPPFinishings = %d\n",uc_Count, e_IPPFinishings );
				break;
			}

		}
	}

	return;
}

/*================================================================================================
［機能］	設定関数 finishings-template-supported
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
[履歴]	Helios CQ-513459 HCL-Sankar 2015/05/08 
================================================================================================*/
void IPPC_LibOpeGetPrintAttr::FinishingsTemplateSupported( UINT8 (&FinishingTemplateSupported)[MAX_IPP_FINISHIGS_TEMPLATE_SUPPORTED][MAX_STRING32])
{
	uchar uc_Count = 0;
	//As per PWG 5100.1-2014 IPP Finishings 2.0(FIN) finishings-template-supported must list finishings-supported values other than none.
	for( TYPE_IPPFinishings e_IPPFinishings = TYPE_IPPFinishingsStaple; e_IPPFinishings < TYPE_IPPFinishingsEnd; e_IPPFinishings++ ) {

		if(True == isEnableFSFunc( e_IPPFinishings )) {
			// KM定義をIPP(NICFUM)定義へ変換する.
			strcpy( (char*)FinishingTemplateSupported[uc_Count], Get_FinishingsTemplate( e_IPPFinishings ) );
			uc_Count++;
			
			// 次順で、NICFUMが用意している配列の要素数を超えるようなら.
			// 不正な配列番号を参照してしまうため、(ログを出力した上で)、処理を抜ける.
			// (このログが出力されるということは、NICFUMに要素数の追加を依頼する必要があるということ).
			if( MAX_IPP_FINISHIGS_TEMPLATE_SUPPORTED <= uc_Count ) {
				DIPPTRACE2( "[IPP][fail safe] FinishingsTemplateSupported uc_Count = %d e_IPPFinishings = %d\n",uc_Count, e_IPPFinishings );
				break;
			}
		}
	}
	return;
}

/*================================================================================================
［機能］	設定関数 finishings-ready
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
[履歴]	Helios CQ-513459 HCL-Sankar 2015/05/08 
================================================================================================*/
void IPPC_LibOpeGetPrintAttr::FinishingsReady( IPP_FINISHINGS 	(&FinishingsReady)[FIN_END] )
{
	uchar uc_Count = 0;
	for(uchar j=0; j < FIN_END; j++ ) FinishingsReady[j] = FIN_END;
	
	for( TYPE_IPPFinishings e_IPPFinishings = TYPE_IPPFinishingsNone; e_IPPFinishings < TYPE_IPPFinishingsEnd; e_IPPFinishings++ ) {
		if(True == isEnableFSFunc( e_IPPFinishings )) {
			// KM定義をIPP(NICFUM)定義へ変換する.
			FinishingsReady[uc_Count] = CnvSpec_finishings( e_IPPFinishings );
			uc_Count++;
			
			// 次順で、NICFUMが用意している配列の要素数を超えるようなら.
			// 不正な配列番号を参照してしまうため、(ログを出力した上で)、処理を抜ける.
			// (このログが出力されるということは、NICFUMに要素数の追加を依頼する必要があるということ).
			if( FIN_END <= uc_Count ) {
				DIPPTRACE2( "[IPP][fail safe] FinishingsReady uc_Count = %d e_IPPFinishings = %d\n",uc_Count, e_IPPFinishings );
				break;
			}
		}
	}
	return;
}

/*================================================================================================
［機能］	設定関数 finishings-col-database, finishings-col-ready
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
[履歴]	Helios KRQ-93 AirPrint1.5対応 HCL-Sankar 2015/02/03 
================================================================================================*/
void IPPC_LibOpeGetPrintAttr::FinishingsCol( IPP_Finishings_Col (&Finishings_Col)[MAX_IPP_FIN_COL] )
{
	memset(&Finishings_Col,0,sizeof(Finishings_Col));
	for( int j = 0; j < MAX_IPP_FIN_COL; j++ ){
		Finishings_Col[j].type = IPP_FIN_COL_END;
	}

	uchar uc_Count = 0;
	for( TYPE_IPPFinishings e_IPPFinishings = TYPE_IPPFinishingsNone; e_IPPFinishings < TYPE_IPPFinishingsEnd; e_IPPFinishings++ ) {

		if(True == isEnableFSFunc( e_IPPFinishings )) {
			// KM定義をIPP(NICFUM)定義へ変換する.
			strcpy( (char*)Finishings_Col[uc_Count].FinishingsTemplate, Get_FinishingsTemplate( e_IPPFinishings ) );
			if( 0 != strcmp( (const char*)Finishings_Col[uc_Count].FinishingsTemplate,"")){
				Finishings_Col[uc_Count].set = 1;
			}
			uc_Count++;
			
			// 次順で、NICFUMが用意している配列の要素数を超えるようなら.
			// 不正な配列番号を参照してしまうため、(ログを出力した上で)、処理を抜ける.
			// (このログが出力されるということは、NICFUMに要素数の追加を依頼する必要があるということ).
			if( MAX_IPP_FIN_COL <= uc_Count ) {
				DIPPTRACE2( "[IPP][fail safe] FinishingsCol uc_Count = %d e_IPPFinishings = %d\n",uc_Count, e_IPPFinishings );
				break;
			}
		}
	}
	return;
}

/*================================================================================================
［機能］	設定関数 urf-supported.
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2014/01/31 新規作成 CQ384241 ZeusCD UV-01249 AppleAirPrint.
================================================================================================*/
typedef unsigned char IPPE_UrfS;
	#define	IPPE_UrfS_V						0		// 最新URFバージョン.
	#define	IPPE_UrfS_W						1		// 色空間(W = 輝度).
	#define	IPPE_UrfS_SRGB					2		// 色空間(sRGB).
	#define	IPPE_UrfS_ADOBERGB				3		// 色空間(AdobeRGB).
	#define	IPPE_UrfS_DEVW					4		// 色空間(DeviceW).
	#define	IPPE_UrfS_DEVRGB				5		// 色空間(DeviceRGB).
	#define	IPPE_UrfS_DEVCMYK				6		// 色空間(DeviceCMYK).
	#define	IPPE_UrfS_RS					7		// 解像度.
	#define	IPPE_UrfS_IS					8		// 給紙トレイ.
	#define	IPPE_UrfS_IFU					9		// 給紙トレイ(Face-up).
	#define	IPPE_UrfS_MT				   10		// 用紙種類.
	#define	IPPE_UrfS_OB				   11		// 排紙トレイ.
	#define	IPPE_UrfS_OFU				   12		// 排紙トレイ(Face-up).
	#define	IPPE_UrfS_PQ				   13		// 印刷品質.
	#define	IPPE_UrfS_DM				   14		// 両面印刷.
	#define	IPPE_UrfS_FN				   15		// 仕上げ.
	#define	IPPE_UrfS_CP				   16		// 最大印刷部数 (1 - 255).
	#define	IPPE_UrfS_MAX				   17		// 終端.

// オプションの装着状態に依存するパラメータは、空文字列して、別ロジックで判定して、設定する.
const char* p_IPP_UrfS[] = {
#ifdef DEF_MonoCus
	"V1.5",				// IPPE_UrfS_V.
	"W8",				// IPPE_UrfS_W.
	"",					// IPPE_UrfS_SRGB(モノクロ機は設定不要).
	"",					// IPPE_UrfS_ADOBERGB(モノクロ機は設定不要).
	"DEVW8",			// IPPE_UrfS_DEVW.
	"",					// IPPE_UrfS_DEVRGB(モノクロ機は設定不要).
	"",					// IPPE_UrfS_DEVCMYK(モノクロ機は設定不要).
	"",					// IPPE_UrfS_RS.
	"",					// IPPE_UrfS_IS.
	"",					// IPPE_UrfS_IFU.
	"",					// IPPE_UrfS_MT.
	"",					// IPPE_UrfS_OB.
	"",					// IPPE_UrfS_OFU(排紙トレイは、すべてFace-Down).
	"PQ4",				// IPPE_UrfS_PQ.
	"DM1",				// IPPE_UrfS_DM.
	"",					// IPPE_UrfS_FN.
	"CP255",			// IPPE_UrfS_CP.
#elif ( defined DEF_A4_Cus )	// VenusMLK先行開発 2OPS Yamaguchi START.
	"V1.5",				// IPPE_UrfS_V.
	"W8",				// IPPE_UrfS_W.
	"SRGB24",			// IPPE_UrfS_SRGB.
	"ADOBERGB24-48",	// IPPE_UrfS_ADOBERGB.
	"DEVW8",			// IPPE_UrfS_DEVW.
	"DEVRGB24",			// IPPE_UrfS_DEVRGB.
	"DEVCMYK32",		// IPPE_UrfS_DEVCMYK.
	"",					// IPPE_UrfS_RS.
	"",					// IPPE_UrfS_IS.
	"",					// IPPE_UrfS_IFU.
	"",					// IPPE_UrfS_MT.
	"",					// IPPE_UrfS_OB.
	"",					// IPPE_UrfS_OFU(排紙トレイは、すべてFace-Down).
	"PQ4",				// IPPE_UrfS_PQ.
	"DM1",				// IPPE_UrfS_DM.
	"",					// IPPE_UrfS_FN.
	"CP255",			// IPPE_UrfS_CP.
// VenusMLK先行開発 2OPS Yamaguchi END.
#else
	"V1.5",				// IPPE_UrfS_V.
	"W8",				// IPPE_UrfS_W.
	"SRGB24",			// IPPE_UrfS_SRGB.
// ZeusCD CQ388690 OPS1八代 2014/04/01 START.
//	"ADOBERGB24-28",	// IPPE_UrfS_ADOBERGB.
	"ADOBERGB24-48",	// IPPE_UrfS_ADOBERGB.
// ZeusCD CQ388690 OPS1八代 2014/04/01 END.
	"DEVW8",			// IPPE_UrfS_DEVW.
	"DEVRGB24",			// IPPE_UrfS_DEVRGB.
	"DEVCMYK32",		// IPPE_UrfS_DEVCMYK.
	"",					// IPPE_UrfS_RS.
	"",					// IPPE_UrfS_IS.
	"",					// IPPE_UrfS_IFU.
	"",					// IPPE_UrfS_MT.
	"",					// IPPE_UrfS_OB
	"",					// IPPE_UrfS_OFU(排紙トレイは、すべてFace-Down).
	"PQ4",				// IPPE_UrfS_PQ.
	"DM1",				// IPPE_UrfS_DM.
	"",					// IPPE_UrfS_FN.
	"CP255",			// IPPE_UrfS_CP.
#endif
};


void IPPC_LibOpeGetPrintAttr::UrfSupported( UCHAR (&URF_Supprted)[MAX_IPP_URF_SUPPORTED][MAX_STRING128] , IPP_REQUEST_TYPE e_ReqType )
{
	memset( URF_Supprted, NULL, sizeof(URF_Supprted) );

	uchar uc_Count = 0;

	for( IPPE_UrfS e_UrfS = IPPE_UrfS_V; e_UrfS <  IPPE_UrfS_MAX; e_UrfS++ ) {

#ifdef DEF_MonoCus
		if( ( IPPE_UrfS_OFU  	 == e_UrfS )
		 || ( IPPE_UrfS_SRGB 	 == e_UrfS )
		 || ( IPPE_UrfS_ADOBERGB == e_UrfS )
		 || ( IPPE_UrfS_DEVRGB 	 == e_UrfS )
		 || ( IPPE_UrfS_DEVCMYK  == e_UrfS ) ) {
#else
		if( IPPE_UrfS_OFU == e_UrfS ) {
#endif
			continue;
		}

		// Helios KRQ-93 AirPrint1.5対応 HCL-Bairavi 2015/02/02 Added START
		if((TYPE_AirPrintLicence_Ver1_5 <= IPP_GetAirPrintVersion()) && (IPP_REQ_TYPE_FAXOUT == e_ReqType)){	//Airprint1.6 Support HCL-Sankar 2016/01/30 Modified.
			if( ( IPPE_UrfS_IFU  	 == e_UrfS )
			 || ( IPPE_UrfS_IS  == e_UrfS )
			 || ( IPPE_UrfS_OB == e_UrfS )
			 || ( IPPE_UrfS_OFU == e_UrfS )
			 || ( IPPE_UrfS_DM  == e_UrfS ) 
			 || ( IPPE_UrfS_MT  == e_UrfS ) 
			 || ( IPPE_UrfS_FN  == e_UrfS ) 
			 || ( IPPE_UrfS_CP  == e_UrfS ) ) {
				continue; 
			}
		}
		// Helios KRQ-93 AirPrint1.5対応 HCL-Bairavi 2015/02/02 Added END
		strcpy( (char *)( URF_Supprted[uc_Count] ), p_IPP_UrfS[e_UrfS] );

		switch( e_UrfS ) {
			case IPPE_UrfS_RS:	UrfSupported_RS(  URF_Supprted[uc_Count] , e_ReqType); break;
			case IPPE_UrfS_IS:	UrfSupported_IS(  URF_Supprted[uc_Count] ); break;
			case IPPE_UrfS_IFU:	UrfSupported_IFU( URF_Supprted[uc_Count] );	break;
			case IPPE_UrfS_MT:	UrfSupported_MT(  URF_Supprted[uc_Count] );	break;
			case IPPE_UrfS_OB:	UrfSupported_OB(  URF_Supprted[uc_Count] );	break;
			case IPPE_UrfS_FN:	UrfSupported_FN(  URF_Supprted[uc_Count] );	break;
			default:			break;
		}

		uc_Count++;

		// 次順で、NICFUMが用意している配列の要素数を超えるようなら.
		// 不正な配列番号を参照してしまうため、(ログを出力した上で)、処理を抜ける.
		// (このログが出力されるということは、NICFUMに要素数の追加を依頼する必要があるということ).
		if( MAX_IPP_URF_SUPPORTED <= uc_Count ) {
			DIPPTRACE2( "[IPP][fail safe] UrfSupported uc_Count = %d e_UrfS = %d\n",uc_Count, e_UrfS );
			break;
		}
	}

	return;
}
/*================================================================================================
［機能］	設定関数 urf-supported Prefix RS(Resolution).
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2014/10/28 新規作成 CQ504484 Poseidon UV-1429
================================================================================================*/
void IPPC_LibOpeGetPrintAttr::UrfSupported_RS( UCHAR (&URF_Supprted)[MAX_STRING128] , IPP_REQUEST_TYPE e_ReqType  )
{
	char uc_Temp[MAX_STRING128];
	memset( uc_Temp, NULL , sizeof(uc_Temp) );
	strcpy( uc_Temp, "RS600" );
	if(IPP_REQ_TYPE_FAXOUT == e_ReqType){
		strcat( uc_Temp, "-400");
		strcat( uc_Temp, "-200");
		strcat( uc_Temp, "-100");
	}
	strcpy( (char*)URF_Supprted, uc_Temp );
	return;
}
/*================================================================================================
［機能］	設定関数 urf-supported Prefix IS(給紙トレイ).
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2014/01/31 新規作成 CQ384241 ZeusCD UV-01249 AppleAirPrint.
================================================================================================*/
static struct IPPS_UrfSupported_IS {
	IPPE_MediaSource	e_MediaSource;	// IPP定義 - media-source.
	char*				pc_UrfValue;	// Urf値.
} st_UrfSupported_IS [] = {
// ZeusCD CQ388690 OPS1八代 2014/04/01 START.
//	{ IPPE_MediaSource_Auto, 		 "0"	 },
// ZeusCD CQ388690 OPS1八代 2014/04/01 END.
	{ IPPE_MediaSource_LCT, 		 "3"	 },
	{ IPPE_MediaSource_ByPass, 		"19"	 },
	{ IPPE_MediaSource_Tray1, 		"20"	 },
	{ IPPE_MediaSource_Tray2, 		"21"	 },
	{ IPPE_MediaSource_Tray3, 		"22"	 },
	{ IPPE_MediaSource_Tray4, 		"23"	 },
	{ IPPE_MediaSource_Tray5, 		"24"	 },		//2018/05/31 DenebMLK 機器構成変更_No.7 HCL-VIJAY Added.
};
void IPPC_LibOpeGetPrintAttr::UrfSupported_IS( UCHAR (&URF_Supprted)[MAX_STRING128] )
{
	char uc_Temp[MAX_STRING128];
	memset( uc_Temp, NULL , sizeof(uc_Temp) );

	strcpy( uc_Temp, "IS" );

	for( ushort i = 0; i < sizeof( st_UrfSupported_IS ) / sizeof( IPPS_UrfSupported_IS ); i++ ) {

#ifdef DEF_A4_Cus // CQ521331 2OPS Yamaguchi 20151104 START.
		if( IPPE_MediaSource_Tray2 == st_UrfSupported_IS[i].e_MediaSource ) {
			if( False == isAttachTray2() ) {
				continue;
			}
		}
#endif // CQ521331 VenusMLK 2OPS Yamaguchi 20151104 END.

		if( IPPE_MediaSource_Tray3 == st_UrfSupported_IS[i].e_MediaSource ) {
			if( False == isAttachTray3() ) {
				continue;
			}
		}

		if( IPPE_MediaSource_Tray4 == st_UrfSupported_IS[i].e_MediaSource ) {
			if( False == isAttachTray4() ) {
				continue;
			}
		}
		
		//2018/05/31 DenebMLK 機器構成変更_No.7 HCL-VIJAY Added Start.
		if( IPPE_MediaSource_Tray5 == st_UrfSupported_IS[i].e_MediaSource ) {
			if( False == isAttachTray5() ) {
				continue;
			}
		}
		//2018/05/31 DenebMLK 機器構成変更_No.7 HCL-VIJAY Added End.
		if( IPPE_MediaSource_LCT == st_UrfSupported_IS[i].e_MediaSource ) {
			if( IPPE_AttachLCT_None == Judge_AttachLCTType() ) {
				continue;
			}
		}

		// ZeusCD CQ388690 OPS1八代 2014/04/01 START.
//		// 先頭は、ハイフン(-)不要.
//		if( 0 != i ) {
//			strcat( uc_Temp, "-" );
//		}
		// ZeusCD CQ388690 OPS1八代 2014/04/01 START.

		 strcat( uc_Temp, st_UrfSupported_IS[i].pc_UrfValue );

		 strcat( uc_Temp, "-" );	// ZeusCD CQ388690 OPS1八代 2014/04/01.
	}

	// ZeusCD CQ388690 OPS1八代 2014/04/01 START.
	// 最後のハイフン(-)を刈り取る.
	char* pLast = strrchr( uc_Temp, '-' );
	if( NULL != pLast ) { *pLast = NULL; }
	// ZeusCD CQ388690 OPS1八代 2014/04/01 END.

	strcpy( (char*)URF_Supprted, uc_Temp );
	return;
}

/*================================================================================================
［機能］	設定関数 urf-supported Prefix IFU(給紙トレイ(Face-up)).
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2014/01/31 新規作成 CQ384241 ZeusCD UV-01249 AppleAirPrint.
			2019/04/18 OP_BTS-5025 Tray5の場合に参照するテーブルを修正.
================================================================================================*/
static struct IPPS_UrfSupported_IFU {
	IPPE_MediaSource	e_MediaSource;	// IPP定義 - media-source.
	char*				pc_UrfValue;	// Urf値.
} st_UrfSupported_IFU [] = {
//	{ IPPE_MediaSource_Auto, 		 "0"	 },		// Face-Upでない.
//	{ IPPE_MediaSource_LCT, 		 "3"	 },		// Face-Upでない.
//	{ IPPE_MediaSource_ByPass, 		"19"	 },		// Face-Upでない.
	{ IPPE_MediaSource_Tray1, 		"20"	 },
	{ IPPE_MediaSource_Tray2, 		"21"	 },
	{ IPPE_MediaSource_Tray3, 		"22"	 },
	{ IPPE_MediaSource_Tray4, 		"23"	 },
	{ IPPE_MediaSource_Tray5, 		"24"	 },		//2018/05/31 DenebMLK 機器構成変更_No.7 HCL-VIJAY Added.
};
void IPPC_LibOpeGetPrintAttr::UrfSupported_IFU( UCHAR (&URF_Supprted)[MAX_STRING128] )
{
	char uc_Temp[MAX_STRING128];
	memset( uc_Temp, NULL , sizeof(uc_Temp) );

	strcpy( uc_Temp, "IFU" );

	for( ushort i = 0; i < sizeof( st_UrfSupported_IFU ) / sizeof( IPPS_UrfSupported_IFU ); i++ ) {

#ifdef DEF_A4_Cus // CQ521331 2OPS Yamaguchi 20151104 START.
		if( IPPE_MediaSource_Tray2 == st_UrfSupported_IFU[i].e_MediaSource ) {
			if( False == isAttachTray2() ) {
				continue;
			}
		}
#endif // CQ521331 VenusMLK 2OPS Yamaguchi 20151104 END.

		if( IPPE_MediaSource_Tray3 == st_UrfSupported_IFU[i].e_MediaSource ) {
			if( False == isAttachTray3() ) {
				continue;
			}
		}

		if( IPPE_MediaSource_Tray4 == st_UrfSupported_IFU[i].e_MediaSource ) {
			if( False == isAttachTray4() ) {
				continue;
			}
		}
		
		//2018/05/31 DenebMLK 機器構成変更_No.7 HCL-VIJAY Added Start.
		if( IPPE_MediaSource_Tray5 == st_UrfSupported_IFU[i].e_MediaSource ) {
			if( False == isAttachTray5() ) {
				continue;
			}
		}
		//2018/05/31 DenebMLK 機器構成変更_No.7 HCL-VIJAY Added End.

		// 先頭は、ハイフン(-)不要.
		if( 0 != i ) {
			strcat( uc_Temp, "-" );
		}

		 strcat( uc_Temp, st_UrfSupported_IFU[i].pc_UrfValue );
	}

	strcpy( (char*)URF_Supprted, uc_Temp );
	return;
}

/*================================================================================================
［機能］	設定関数 urf-supported Prefix MT(用紙種別).
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2014/01/31 新規作成 CQ384241 ZeusCD UV-01249 AppleAirPrint.
================================================================================================*/
static struct IPPS_UrfSupported_MT {
	IPPE_MediaType		e_MediaType;	// IPP定義 - media-type.
	char*				pc_UrfValue;	// Urf値.
} st_UrfSupported_MT [] = {
#ifdef DEF_A4_Cus // VenusMLK先行開発 2OPS Yamaguchi.
#ifdef DEF_KRONOSS_Cus // KronosS 用紙種別カスタマイズ対応 START.
	{ IPPE_MediaType_Stat, 		 "1"	 },
	{ IPPE_MediaType_Env, 		 "3"	 },
	{ IPPE_MediaType_Label, 	 "5"	 },
#else	// VenusMLK.
	{ IPPE_MediaType_Stat, 		 "1"	 },
	{ IPPE_MediaType_Env, 		 "3"	 },
	{ IPPE_MediaType_Label, 	 "5"	 },
	{ IPPE_MediaType_PhotoSemGloss, 	 "10"	 },
	{ IPPE_MediaType_PhotoGloss, "11"	 },
	{ IPPE_MediaType_PhotoHighGloss, 	 "12"	 },
#endif // KronosS 用紙種別カスタマイズ対応 END.		
#else
// ZeusCD CQ388690 OPS1八代 2014/04/01 START.
//	{ IPPE_MediaType_Auto, 		 "0"	 },
// ZeusCD CQ388690 OPS1八代 2014/04/01 END.
	{ IPPE_MediaType_Stat, 		 "1"	 },
	{ IPPE_MediaType_ThinOhp, 	 "2"	 },		// CQ391054 2014/06/17 OPS1八代.
	{ IPPE_MediaType_Env, 		 "3"	 },
	{ IPPE_MediaType_LetterH, 	 "6"	 },		// CQ391054 2014/06/17 OPS1八代.
#endif // VenusMLK先行開発 2OPS Yamaguchi.
};
void IPPC_LibOpeGetPrintAttr::UrfSupported_MT( UCHAR (&URF_Supprted)[MAX_STRING128] )
{
	char uc_Temp[MAX_STRING128];
	memset( uc_Temp, NULL , sizeof(uc_Temp) );

	strcpy( uc_Temp, "MT" );

	for( ushort i = 0; i < sizeof( st_UrfSupported_MT ) / sizeof( IPPS_UrfSupported_MT ); i++ ) {

		// 先頭は、ハイフン(-)不要.
		if( 0 != i ) {
			strcat( uc_Temp, "-" );
		}

		 strcat( uc_Temp, st_UrfSupported_MT[i].pc_UrfValue );
	}

	strcpy( (char*)URF_Supprted, uc_Temp );
	return;
}

/*================================================================================================
［機能］	設定関数 urf-supported Prefix OB(排紙トレイ).
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2014/01/31 新規作成 CQ384241 ZeusCD UV-01249 AppleAirPrint.
================================================================================================*/
void IPPC_LibOpeGetPrintAttr::UrfSupported_OB( UCHAR (&URF_Supprted)[MAX_STRING128] )
{
	char uc_Temp[MAX_STRING128];
	memset( uc_Temp, NULL , sizeof(uc_Temp) );

	strcpy( uc_Temp, "OB" );

	IPP_OUTPUT_BIN  Output_Bin[MAX_IPP_OUTPUT_BIN];
	OutputBinSupported( Output_Bin );

	for( ushort i = 0; i < MAX_IPP_OUTPUT_BIN; i++ ) {

		if( OUTPUT_BIN_END == Output_Bin[i] ) break;

		if( 0 != i ) strcat( uc_Temp, "-" );			// 先頭は、ハイフン(-)不要.

		switch( Output_Bin[i] ) {
			case TOP:	 strcat( uc_Temp, "1" );	break;
			case MIDDLE: strcat( uc_Temp, "2" );	break;
			case BOTTOM: strcat( uc_Temp, "3" );	break;
			case RIGHT : strcat( uc_Temp, "6" );	break;   // ZeusS ZX0 機器構成変更_No.3対応 HCL-VIJAY 2015/06/22 Added.
#ifdef DEF_A4_Cus // VenusMLK 2OPS Yamaguchi 20151006 START.
			case FACE_DOWN: strcat( uc_Temp, "10" );	break;
#endif // VenusMLK 2OPS Yamaguchi 20151006 END.
			default:								break;
		}
	}

	strcpy( (char*)URF_Supprted, uc_Temp );
	return;
}

/*================================================================================================
［機能］	設定関数 urf-supported Prefix FN(仕上げ).
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2014/01/31 新規作成 CQ384241 ZeusCD UV-01249 AppleAirPrint.
================================================================================================*/
static struct IPPS_UrfSupported_FN {
	IPP_FINISHINGS		e_FINISHINGS;	// IPP定義(NICFUM) - finishings.
	char*				pc_UrfValue;	// Urf値.
} st_UrfSupported_FN [] = {
	{ FIN_NONE,					"3"		},	// 'none'.	// ZeusCD CQ386774 OPS1八代 2014/02/25.
	{ FIN_STAPLE, 				"4"		},	// 'staple'.
	{ FIN_PUNCH, 				"5" 	},	// 'punch'.
	{ FIN_COVER, 				"6"		},	// 'cover'.
	{ FIN_BIND, 				"7"		},	// 'bind'.
	{ FIN_SADDLE_STITCH, 		"8"		},	// 'saddle-stitch'.
	{ FIN_FOLD, 				"10"	},	// 'fold'.
	{ FIN_JOG_OFFSET, 			"14"	},	// 'jog-offset'.
	{ FIN_STAPLE_TOP_LEFT,		"20"	},
	{ FIN_STAPLE_BOTTOM_LEFT, 	"21"	},
	{ FIN_STAPLE_TOP_RIGHT, 	"22"	},
	{ FIN_STAPLE_BOTTOM_RIGHT, 	"23"	},
	{ FIN_STAPLE_DUAL_LEFT, 	"28"	},
	{ FIN_STAPLE_DUAL_TOP, 		"29"	},
	{ FIN_STAPLE_DUAL_RIGHT, 	"30"	},
	{ FIN_STAPLE_DUAL_BOTTOM, 	"31"	},
	{ FIN_PUNCH_TOP_LEFT, 		"70"	},
	{ FIN_PUNCH_BOTTOM_LEFT, 	"71"	},
	{ FIN_PUNCH_TOP_RIGHT, 		"72"	},
	{ FIN_PUNCH_BOTTOM_RIGHT, 	"73"	},
	{ FIN_PUNCH_DUAL_LEFT, 		"74"	},
	{ FIN_PUNCH_DUAL_TOP, 		"75"	},
	{ FIN_PUNCH_DUAL_RIGHT, 	"76"	},
	{ FIN_PUNCH_DUAL_BOTTOM, 	"77"	},
	{ FIN_PUNCH_TRIPLE_LEFT, 	"78"	},
	{ FIN_PUNCH_TRIPLE_TOP, 	"79"	},
	{ FIN_PUNCH_TRIPLE_RIGHT, 	"80"	},
	{ FIN_PUNCH_TRIPLE_BOTTOM, 	"81"	},
	{ FIN_PUNCH_QUAD_LEFT, 		"82"	},
	{ FIN_PUNCH_QUAD_TOP, 		"83"	},
	{ FIN_PUNCH_QUAD_RIGHT, 	"84"	},
	{ FIN_PUNCH_QUAD_BOTTOM, 	"85"	},
	// Poseidon UV-01429 AppleAirPrint1.4対応 Bairavi 2014/07/07 END.
	{ FIN_FOLD_HALF,  			"93"	},
	{ FIN_FOLD_LETTER,  		"96"	},
//	{ FIN_FOLD_Z,  				"100"	},
	{ FIN_FOLD_ENGINEERING_Z,	"101"	},
	// Poseidon UV-01429 AppleAirPrint1.4対応 Bairavi 2014/07/07 END.
	{ FIN_END,  				""		},
};
void IPPC_LibOpeGetPrintAttr::UrfSupported_FN( UCHAR (&URF_Supprted)[MAX_STRING128] )
{
	char uc_Temp[MAX_STRING128];
	memset( uc_Temp, NULL , sizeof(uc_Temp) );

	IPP_FINISHINGS FinishingsS[MAX_IPP_FINISHINGS];
	FinishingsSupported( FinishingsS );

	strcpy( uc_Temp, "FN" );

	for( ushort i = 0; i < MAX_IPP_FINISHINGS; i++ ) {

		if( FIN_END == FinishingsS[i] ) break;

		for( ushort j = 0; j < sizeof( st_UrfSupported_FN ) / sizeof( IPPS_UrfSupported_FN ); j++ ) {

			if( st_UrfSupported_FN[j].e_FINISHINGS == FinishingsS[i] ) {

				// 先頭は、ハイフン(-)不要.
				if( 0 != i ) {
					strcat( uc_Temp, "-" );
				}

				strcat( uc_Temp, st_UrfSupported_FN[j].pc_UrfValue );
			}
		}
	}

	strcpy( (char*)URF_Supprted, uc_Temp );
	return;
}

// ZeusCD UV-01249 AppleAirPrint OPS1正部家 2014/01/31 START.
/*================================================================================================
［機能］	設定関数 marker-colors, marker-high-levels, marker-levels, marker-low-levels, marker-names, marker-types.
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2014/01/31 新規作成 CQ384241 ZeusCD UV-01249 AppleAirPrint.
		   2017/11/7 Eagle 制御構成変更_No.6 HCL-VIJAY Modified.
================================================================================================*/
// 2019/04/08 【EagleBK】【Control No.6】 HIENG Start.
// e_Nameがテーブルの検索キーとなる.
struct IPPS_MakerSupplies{
//	UCHAR				uc_Color[MAX_STRING16];
	IPPE_MarkerColors	e_Color;
	INT32				i_HighLevels; 
	INT32				i_Levels; 
	INT32				i_LowLevels; 
//	UCHAR				uc_Name[MAX_STRING64];
	IPPE_MarkerNames	e_Name;
	IPP_MARKER_TYPE	e_Types;
};
static IPPS_MakerSupplies st_MakerSupplies [] = {
// 2019/04/08 【EagleBK】【Control No.6】 HIENG End.
//Zeus5BK 制御構成変更No172 OPS1前田 2014/05/13 START.
//2018/05/31 DenebMLK 制御構成変更_No.1 HCL-VIJAY Modified Start.
#if ( defined DEF_A4_Cus )	// VenusMLK先行開発 2OPS Yamaguchi START.
	// Sparrow(MFP 1 / MFP 2 / AIO 2 / SFP 1 / SFP 2).
#ifdef DEF_KRONOSS_Cus		// KronosS モノクロ化対応 START
// KronosS.										
	{ IPPE_MarkerColors_Black, 		IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10, 	IPPE_MarkerNames_TonerCart_K, 		IPP_MARKER_TYPE_TONER	 },	
	{ IPPE_MarkerColors_Black, 		IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10, 	IPPE_MarkerNames_ImgUnit_K, 		IPP_MARKER_TYPE_OPC	 },	
	{ IPPE_MarkerColors_None, 		IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10, 	IPPE_MarkerNames_FusingUnit, 		IPP_MARKER_TYPE_FUSER	 },	
	{ IPPE_MarkerColors_None, 		IPPD_MarkerLevel_99,	IPPD_MarkerLevel_NONE,		IPPD_MarkerLevel_0, 	IPPE_MarkerNames_StapleCart, 		IPP_MARKER_TYPE_STAPLES	 },
	{ IPPE_MarkerColors_Unidentified, IPPD_MarkerLevel_TERM,	IPPD_MarkerLevel_TERM,		IPPD_MarkerLevel_TERM, 	IPPE_MarkerNames_NONE, 				IPP_MARKER_TYPE_MAX	 },
#else
// [Eagle] 2018/10/22 1PF2G八代 AR_1046111 START.
#ifdef DEF_MonoCus	// SparrowBK Start.
// SparrowBK.
	{ IPPE_MarkerColors_Black, 		IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10, 	IPPE_MarkerNames_TonerCart_K, 		IPP_MARKER_TYPE_TONER	 },
	{ IPPE_MarkerColors_Black, 		IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10, 	IPPE_MarkerNames_ImgUnit_K, 		IPP_MARKER_TYPE_OPC	 },
	{ IPPE_MarkerColors_None, 		IPPD_MarkerLevel_95,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_0, 	IPPE_MarkerNames_WasteTonerBox, 	IPP_MARKER_TYPE_WASTE_TONER	 },
	{ IPPE_MarkerColors_None, 		IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10, 	IPPE_MarkerNames_FusingUnit, 		IPP_MARKER_TYPE_FUSER	 },
	{ IPPE_MarkerColors_None, 		IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10, 	IPPE_MarkerNames_ImgTransBeltUnit, 	IPP_MARKER_TYPE_TRANSFER_UNIT	 },
	{ IPPE_MarkerColors_None, 		IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10, 	IPPE_MarkerNames_TransRollUnit, 	IPP_MARKER_TYPE_TRANSFER_UNIT	 },
	{ IPPE_MarkerColors_None,		IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10,	IPPE_MarkerNames_TonerFilter,		IPP_MARKER_TYPE_CLEANER_UNIT	},	
	{ IPPE_MarkerColors_Unidentified, IPPD_MarkerLevel_TERM,	IPPD_MarkerLevel_TERM,		IPPD_MarkerLevel_TERM, 	IPPE_MarkerNames_NONE, 				IPP_MARKER_TYPE_MAX	 },
#else
// Sparrow.
	{ IPPE_MarkerColors_Cyan, 		IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10, 	IPPE_MarkerNames_TonerCart_C, 		IPP_MARKER_TYPE_TONER	 },
	{ IPPE_MarkerColors_Magenta, 	IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10, 	IPPE_MarkerNames_TonerCart_M, 		IPP_MARKER_TYPE_TONER	 },
	{ IPPE_MarkerColors_Yellow, 	IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10, 	IPPE_MarkerNames_TonerCart_Y, 		IPP_MARKER_TYPE_TONER	 },
	{ IPPE_MarkerColors_Black, 		IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10, 	IPPE_MarkerNames_TonerCart_K, 		IPP_MARKER_TYPE_TONER	 },
	{ IPPE_MarkerColors_Cyan, 		IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10, 	IPPE_MarkerNames_ImgUnit_C, 		IPP_MARKER_TYPE_OPC	 },
	{ IPPE_MarkerColors_Magenta, 	IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10, 	IPPE_MarkerNames_ImgUnit_M, 		IPP_MARKER_TYPE_OPC	 },
	{ IPPE_MarkerColors_Yellow, 	IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10, 	IPPE_MarkerNames_ImgUnit_Y, 		IPP_MARKER_TYPE_OPC	 },
	{ IPPE_MarkerColors_Black, 		IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10, 	IPPE_MarkerNames_ImgUnit_K, 		IPP_MARKER_TYPE_OPC	 },
	{ IPPE_MarkerColors_None, 		IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10, 	IPPE_MarkerNames_ImgTransBeltUnit, 	IPP_MARKER_TYPE_TRANSFER_UNIT	 },
	{ IPPE_MarkerColors_None, 		IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10, 	IPPE_MarkerNames_TransRollUnit, 	IPP_MARKER_TYPE_TRANSFER_UNIT	 },
	{ IPPE_MarkerColors_None, 		IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10, 	IPPE_MarkerNames_FusingUnit, 		IPP_MARKER_TYPE_FUSER	 },
	{ IPPE_MarkerColors_None, 		IPPD_MarkerLevel_95,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_0, 	IPPE_MarkerNames_WasteTonerBox, 	IPP_MARKER_TYPE_WASTE_TONER	 },
	{ IPPE_MarkerColors_Unidentified, IPPD_MarkerLevel_TERM,	IPPD_MarkerLevel_TERM,		IPPD_MarkerLevel_TERM, 	IPPE_MarkerNames_NONE, 				IPP_MARKER_TYPE_MAX	 },
// VenusMLK先行開発 2OPS Yamaguchi End.
#endif					// SparrowBK End.
#endif	// KronosS モノクロ化対応 END
//2017/11/7 Eagle 制御構成変更_No.6 HCL-VIJAY Modified Start.
#elif ( defined DEF_DenebMLKCus  )
	// DenebMLK 1/2/3/4
	{ IPPE_MarkerColors_Black, 		IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10, 	IPPE_MarkerNames_TonerCart_K, 		IPP_MARKER_TYPE_TONER	 },
	{ IPPE_MarkerColors_Black, 		IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10, 	IPPE_MarkerNames_DrumCart_K, 		IPP_MARKER_TYPE_OPC	 },
	{ IPPE_MarkerColors_Black, 		IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10, 	IPPE_MarkerNames_DevCart_K, 		IPP_MARKER_TYPE_DEVELOPER	 },	
	{ IPPE_MarkerColors_None, 		IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10, 	IPPE_MarkerNames_FusingUnit, 		IPP_MARKER_TYPE_FUSER	 },	
	{ IPPE_MarkerColors_None, 		IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10, 	IPPE_MarkerNames_TransRollUnit, 	IPP_MARKER_TYPE_TRANSFER_UNIT	 },
	{ IPPE_MarkerColors_None,		IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10,	IPPE_MarkerNames_TonerFilter,		IPP_MARKER_TYPE_CLEANER_UNIT	},	
	{ IPPE_MarkerColors_None,		IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10,	IPPE_MarkerNames_PaperDustRemover,	IPP_MARKER_TYPE_CLEANER_UNIT	},	
	{ IPPE_MarkerColors_Unidentified,IPPD_MarkerLevel_TERM,	IPPD_MarkerLevel_TERM,		IPPD_MarkerLevel_TERM, 	IPPE_MarkerNames_NONE, 				IPP_MARKER_TYPE_MAX		},
//2018/05/15 DenebMLK 制御構成変更_No.1 HCL-VIJAY Modified End.
#elif defined(DEF_HeliosMLKCus)
	// HeliosMLK.
	{ IPPE_MarkerColors_Cyan, 		IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10, 	IPPE_MarkerNames_TonerCart_C, 		IPP_MARKER_TYPE_TONER	 },
	{ IPPE_MarkerColors_Magenta, 	IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10, 	IPPE_MarkerNames_TonerCart_M, 		IPP_MARKER_TYPE_TONER	 },
	{ IPPE_MarkerColors_Yellow, 	IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10, 	IPPE_MarkerNames_TonerCart_Y, 		IPP_MARKER_TYPE_TONER	 },
	{ IPPE_MarkerColors_Black, 		IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10, 	IPPE_MarkerNames_TonerCart_K, 		IPP_MARKER_TYPE_TONER	 },
	{ IPPE_MarkerColors_Cyan, 		IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10, 	IPPE_MarkerNames_ImgUnit_C, 		IPP_MARKER_TYPE_OPC	 },
	{ IPPE_MarkerColors_Magenta, 	IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10, 	IPPE_MarkerNames_ImgUnit_M, 		IPP_MARKER_TYPE_OPC	 },
	{ IPPE_MarkerColors_Yellow, 	IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10, 	IPPE_MarkerNames_ImgUnit_Y, 		IPP_MARKER_TYPE_OPC	 },
	{ IPPE_MarkerColors_Black, 		IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10, 	IPPE_MarkerNames_DrumCart_K, 		IPP_MARKER_TYPE_OPC	 },
	{ IPPE_MarkerColors_Black, 		IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10, 	IPPE_MarkerNames_DevCart_K, 		IPP_MARKER_TYPE_DEVELOPER	 },
	{ IPPE_MarkerColors_None, 		IPPD_MarkerLevel_95,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_0, 	IPPE_MarkerNames_WasteTonerBox, 	IPP_MARKER_TYPE_WASTE_TONER	 },
	{ IPPE_MarkerColors_None, 		IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10, 	IPPE_MarkerNames_FusingUnit, 		IPP_MARKER_TYPE_FUSER	 },
	{ IPPE_MarkerColors_None, 		IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10, 	IPPE_MarkerNames_ImgTransBeltUnit, 	IPP_MARKER_TYPE_TRANSFER_UNIT	 },
	{ IPPE_MarkerColors_None, 		IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10, 	IPPE_MarkerNames_TransRollUnit, 	IPP_MARKER_TYPE_TRANSFER_UNIT	 },
	{ IPPE_MarkerColors_None, 		IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10, 	IPPE_MarkerNames_StapleCart, 		IPP_MARKER_TYPE_STAPLES	 },
	{ IPPE_MarkerColors_None, 		IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10, 	IPPE_MarkerNames_SaddleStapleCart1, IPP_MARKER_TYPE_STAPLES	 },
	{ IPPE_MarkerColors_None, 		IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10, 	IPPE_MarkerNames_SaddleStapleCart2, IPP_MARKER_TYPE_STAPLES	 },
	{ IPPE_MarkerColors_None, 		IPPD_MarkerLevel_95,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_0, 	IPPE_MarkerNames_HolePunchScrapBox, IPP_MARKER_TYPE_WASTE_PAPER	 },	
	{ IPPE_MarkerColors_Unidentified,IPPD_MarkerLevel_TERM,	IPPD_MarkerLevel_TERM,		IPPD_MarkerLevel_TERM, 	IPPE_MarkerNames_NONE, 				IPP_MARKER_TYPE_MAX		},
//2017/11/7 Eagle 制御構成変更_No.6 HCL-VIJAY Modified Start.
// 2019/04/08 【EagleBK】【Control No.6】 HIENG Start.
//2021/07/09 [IT6_2.2][A800Less] 制御構成変更_No.41 HIENG Start.	
#elif defined(DEF_EagleBKCus) // EagleBK Z/X/0, EagleZ+BK.
	{ IPPE_MarkerColors_Black,		IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10,	IPPE_MarkerNames_TonerCart_K,		IPP_MARKER_TYPE_TONER },
	{ IPPE_MarkerColors_Black,		IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10,	IPPE_MarkerNames_DrumCart_K,		IPP_MARKER_TYPE_OPC },
	{ IPPE_MarkerColors_Black,		IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10,	IPPE_MarkerNames_DevCart_K,			IPP_MARKER_TYPE_DEVELOPER },
	{ IPPE_MarkerColors_None,		IPPD_MarkerLevel_95,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_0,		IPPE_MarkerNames_WasteTonerBox,		IPP_MARKER_TYPE_WASTE_TONER },
	{ IPPE_MarkerColors_None,		IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10,	IPPE_MarkerNames_FusingUnit,		IPP_MARKER_TYPE_FUSER },
	{ IPPE_MarkerColors_None,		IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10,	IPPE_MarkerNames_ImgTransBeltUnit,	IPP_MARKER_TYPE_TRANSFER_UNIT },
	{ IPPE_MarkerColors_None,		IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10,	IPPE_MarkerNames_TransRollUnit,		IPP_MARKER_TYPE_TRANSFER_UNIT },
	{ IPPE_MarkerColors_None,		IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10,	IPPE_MarkerNames_TonerFilter,		IPP_MARKER_TYPE_CLEANER_UNIT },
	{ IPPE_MarkerColors_None,		IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10,	IPPE_MarkerNames_StapleCart,		IPP_MARKER_TYPE_STAPLES },
	{ IPPE_MarkerColors_None,		IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10,	IPPE_MarkerNames_SaddleStapleCart1,	IPP_MARKER_TYPE_STAPLES },
	{ IPPE_MarkerColors_None,		IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10,	IPPE_MarkerNames_SaddleStapleCart2,	IPP_MARKER_TYPE_STAPLES },
	{ IPPE_MarkerColors_None,		IPPD_MarkerLevel_95,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_0,		IPPE_MarkerNames_HolePunchScrapBox,	IPP_MARKER_TYPE_WASTE_PAPER },
	{ IPPE_MarkerColors_None,		IPPD_MarkerLevel_95,	IPPD_MarkerLevel_NONE,		IPPD_MarkerLevel_0,		IPPE_MarkerNames_PIMainTray,		IPP_MARKER_TYPE_INSERTS },
	{ IPPE_MarkerColors_None,		IPPD_MarkerLevel_95,	IPPD_MarkerLevel_NONE,		IPPD_MarkerLevel_0,		IPPE_MarkerNames_PISubTray,			IPP_MARKER_TYPE_INSERTS },
	{ IPPE_MarkerColors_Unidentified,IPPD_MarkerLevel_TERM, IPPD_MarkerLevel_TERM,		IPPD_MarkerLevel_TERM, 	IPPE_MarkerNames_NONE, 				IPP_MARKER_TYPE_MAX },
	{ IPPE_MarkerColors_Unidentified,IPPD_MarkerLevel_TERM, IPPD_MarkerLevel_TERM,		IPPD_MarkerLevel_TERM, 	IPPE_MarkerNames_NONE, 				IPP_MARKER_TYPE_MAX },
//2021/07/09 [IT6_2.2][A800Less] 制御構成変更_No.41 HIENG End.

// 2019/04/08 【EagleBK】【Control No.6】 HIENG End.
#else
	// Eagle Z/X/0, EagleZ+.
	{ IPPE_MarkerColors_Cyan, 		IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10, 	IPPE_MarkerNames_TonerCart_C, 		IPP_MARKER_TYPE_TONER	 },
	{ IPPE_MarkerColors_Magenta, 	IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10, 	IPPE_MarkerNames_TonerCart_M, 		IPP_MARKER_TYPE_TONER	 },
	{ IPPE_MarkerColors_Yellow, 	IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10, 	IPPE_MarkerNames_TonerCart_Y, 		IPP_MARKER_TYPE_TONER	 },
	{ IPPE_MarkerColors_Black, 		IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10, 	IPPE_MarkerNames_TonerCart_K, 		IPP_MARKER_TYPE_TONER	 },
	{ IPPE_MarkerColors_Cyan, 		IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10, 	IPPE_MarkerNames_DrumCart_C, 		IPP_MARKER_TYPE_OPC	 },
	{ IPPE_MarkerColors_Cyan, 		IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10, 	IPPE_MarkerNames_DevCart_C, 		IPP_MARKER_TYPE_DEVELOPER	 },
	{ IPPE_MarkerColors_Magenta, 	IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10, 	IPPE_MarkerNames_DrumCart_M, 		IPP_MARKER_TYPE_OPC	 },
	{ IPPE_MarkerColors_Magenta, 	IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10, 	IPPE_MarkerNames_DevCart_M, 		IPP_MARKER_TYPE_DEVELOPER	 },
	{ IPPE_MarkerColors_Yellow, 	IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10, 	IPPE_MarkerNames_DrumCart_Y, 		IPP_MARKER_TYPE_OPC	 },
	{ IPPE_MarkerColors_Yellow, 	IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10, 	IPPE_MarkerNames_DevCart_Y, 		IPP_MARKER_TYPE_DEVELOPER	 },
	{ IPPE_MarkerColors_Black, 		IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10, 	IPPE_MarkerNames_DrumCart_K, 		IPP_MARKER_TYPE_OPC	 },
	{ IPPE_MarkerColors_Black, 		IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10, 	IPPE_MarkerNames_DevCart_K, 		IPP_MARKER_TYPE_DEVELOPER	 },
	{ IPPE_MarkerColors_None, 		IPPD_MarkerLevel_95,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_0, 	IPPE_MarkerNames_WasteTonerBox, 	IPP_MARKER_TYPE_WASTE_TONER	 },
	{ IPPE_MarkerColors_None, 		IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10, 	IPPE_MarkerNames_FusingUnit, 		IPP_MARKER_TYPE_FUSER	 },
	{ IPPE_MarkerColors_None, 		IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10, 	IPPE_MarkerNames_ImgTransBeltUnit, 	IPP_MARKER_TYPE_TRANSFER_UNIT	 },
	{ IPPE_MarkerColors_None, 		IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10, 	IPPE_MarkerNames_TransRollUnit, 	IPP_MARKER_TYPE_TRANSFER_UNIT	 },
	{ IPPE_MarkerColors_None,		IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10,	IPPE_MarkerNames_TonerFilter,		IPP_MARKER_TYPE_CLEANER_UNIT	},
	{ IPPE_MarkerColors_None, 		IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10, 	IPPE_MarkerNames_StapleCart, 		IPP_MARKER_TYPE_STAPLES	 },
	{ IPPE_MarkerColors_None, 		IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10, 	IPPE_MarkerNames_SaddleStapleCart1, IPP_MARKER_TYPE_STAPLES	 },
	{ IPPE_MarkerColors_None, 		IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10, 	IPPE_MarkerNames_SaddleStapleCart2, IPP_MARKER_TYPE_STAPLES	 },
	{ IPPE_MarkerColors_None, 		IPPD_MarkerLevel_95,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_0, 	IPPE_MarkerNames_HolePunchScrapBox, IPP_MARKER_TYPE_WASTE_PAPER	 },	
//2021/09/13 [MinervaSSBK][OP_BTS-35461] HIENG Start.
	{ IPPE_MarkerColors_None,		IPPD_MarkerLevel_95,	IPPD_MarkerLevel_NONE,		IPPD_MarkerLevel_0,		IPPE_MarkerNames_PIMainTray,		IPP_MARKER_TYPE_INSERTS },
	{ IPPE_MarkerColors_None,		IPPD_MarkerLevel_95,	IPPD_MarkerLevel_NONE,		IPPD_MarkerLevel_0,		IPPE_MarkerNames_PISubTray,			IPP_MARKER_TYPE_INSERTS },
//2021/09/13 [MinervaSSBK][OP_BTS-35461] HIENG End.
	{ IPPE_MarkerColors_Unidentified,IPPD_MarkerLevel_TERM,	IPPD_MarkerLevel_TERM,		IPPD_MarkerLevel_TERM, 	IPPE_MarkerNames_NONE, 				IPP_MARKER_TYPE_MAX		}, 
//2017/11/7 Eagle 制御構成変更_No.6 HCL-VIJAY Modified End.
// [Eagle] 2018/10/22 1PF2G八代 AR_1046111 END.
#endif
};


//2021/07/09 [IT6_2.2][A800Less] 制御構成変更_No.41 HIENG Start.
static IPPS_MakerSupplies st_MakerSupplies_EagleL [] = {
#if ( defined DEF_EagleBKCus )  // EagleBK 1/2.
	{ IPPE_MarkerColors_Black,		IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10,	IPPE_MarkerNames_TonerCart_K,		IPP_MARKER_TYPE_TONER },
	{ IPPE_MarkerColors_Black,		IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10,	IPPE_MarkerNames_DrumCart_K,		IPP_MARKER_TYPE_OPC },
	{ IPPE_MarkerColors_Black,		IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10,	IPPE_MarkerNames_DevCart_K,			IPP_MARKER_TYPE_DEVELOPER },
	{ IPPE_MarkerColors_None,		IPPD_MarkerLevel_95,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_0,		IPPE_MarkerNames_WasteTonerBox,		IPP_MARKER_TYPE_WASTE_TONER },
	{ IPPE_MarkerColors_None,		IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10,	IPPE_MarkerNames_FusingUnit,		IPP_MARKER_TYPE_FUSER },
	{ IPPE_MarkerColors_None,		IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10,	IPPE_MarkerNames_ImgTransBeltUnit,	IPP_MARKER_TYPE_TRANSFER_UNIT },
	{ IPPE_MarkerColors_None,		IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10,	IPPE_MarkerNames_TransRollUnit,		IPP_MARKER_TYPE_TRANSFER_UNIT },
	{ IPPE_MarkerColors_None,		IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10,	IPPE_MarkerNames_TonerFilter,		IPP_MARKER_TYPE_CLEANER_UNIT },
	{ IPPE_MarkerColors_None,		IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10,	IPPE_MarkerNames_StapleCart,		IPP_MARKER_TYPE_STAPLES },
	{ IPPE_MarkerColors_None,		IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10,	IPPE_MarkerNames_SaddleStapleCart1,	IPP_MARKER_TYPE_STAPLES },
	{ IPPE_MarkerColors_None,		IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10,	IPPE_MarkerNames_SaddleStapleCart2,	IPP_MARKER_TYPE_STAPLES },
	{ IPPE_MarkerColors_None,		IPPD_MarkerLevel_95,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_0,		IPPE_MarkerNames_HolePunchScrapBox,	IPP_MARKER_TYPE_WASTE_PAPER },
	{ IPPE_MarkerColors_Unidentified,IPPD_MarkerLevel_TERM, IPPD_MarkerLevel_TERM,		IPPD_MarkerLevel_TERM, 	IPPE_MarkerNames_NONE, 				IPP_MARKER_TYPE_MAX },

#else  //  Eagle 1/2/2.5
	{ IPPE_MarkerColors_Cyan, 		IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10, 	IPPE_MarkerNames_TonerCart_C, 		IPP_MARKER_TYPE_TONER	 },
	{ IPPE_MarkerColors_Magenta, 	IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10, 	IPPE_MarkerNames_TonerCart_M, 		IPP_MARKER_TYPE_TONER	 },
	{ IPPE_MarkerColors_Yellow, 	IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10, 	IPPE_MarkerNames_TonerCart_Y, 		IPP_MARKER_TYPE_TONER	 },
	{ IPPE_MarkerColors_Black, 		IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10, 	IPPE_MarkerNames_TonerCart_K, 		IPP_MARKER_TYPE_TONER	 },
	{ IPPE_MarkerColors_Cyan, 		IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10, 	IPPE_MarkerNames_DrumCart_C, 		IPP_MARKER_TYPE_OPC	 },
	{ IPPE_MarkerColors_Cyan, 		IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10, 	IPPE_MarkerNames_DevCart_C, 		IPP_MARKER_TYPE_DEVELOPER	 },
	{ IPPE_MarkerColors_Magenta, 	IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10, 	IPPE_MarkerNames_DrumCart_M, 		IPP_MARKER_TYPE_OPC	 },
	{ IPPE_MarkerColors_Magenta, 	IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10, 	IPPE_MarkerNames_DevCart_M, 		IPP_MARKER_TYPE_DEVELOPER	 },
	{ IPPE_MarkerColors_Yellow, 	IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10, 	IPPE_MarkerNames_DrumCart_Y, 		IPP_MARKER_TYPE_OPC	 },
	{ IPPE_MarkerColors_Yellow, 	IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10, 	IPPE_MarkerNames_DevCart_Y, 		IPP_MARKER_TYPE_DEVELOPER	 },
	{ IPPE_MarkerColors_Black, 		IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10, 	IPPE_MarkerNames_DrumCart_K, 		IPP_MARKER_TYPE_OPC	 },
	{ IPPE_MarkerColors_Black, 		IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10, 	IPPE_MarkerNames_DevCart_K, 		IPP_MARKER_TYPE_DEVELOPER	 },
	{ IPPE_MarkerColors_None, 		IPPD_MarkerLevel_95,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_0, 	IPPE_MarkerNames_WasteTonerBox, 	IPP_MARKER_TYPE_WASTE_TONER	 },
	{ IPPE_MarkerColors_None, 		IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10, 	IPPE_MarkerNames_FusingUnit, 		IPP_MARKER_TYPE_FUSER	 },
	{ IPPE_MarkerColors_None, 		IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10, 	IPPE_MarkerNames_ImgTransBeltUnit, 	IPP_MARKER_TYPE_TRANSFER_UNIT	 },
	{ IPPE_MarkerColors_None, 		IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10, 	IPPE_MarkerNames_TransRollUnit, 	IPP_MARKER_TYPE_TRANSFER_UNIT	 },
	{ IPPE_MarkerColors_None,		IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10,	IPPE_MarkerNames_TonerFilter,		IPP_MARKER_TYPE_CLEANER_UNIT	},
	{ IPPE_MarkerColors_None, 		IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10, 	IPPE_MarkerNames_StapleCart, 		IPP_MARKER_TYPE_STAPLES	 },
	{ IPPE_MarkerColors_None, 		IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10, 	IPPE_MarkerNames_SaddleStapleCart1, IPP_MARKER_TYPE_STAPLES	 },
	{ IPPE_MarkerColors_None, 		IPPD_MarkerLevel_100,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_10, 	IPPE_MarkerNames_SaddleStapleCart2, IPP_MARKER_TYPE_STAPLES	 },
	{ IPPE_MarkerColors_None, 		IPPD_MarkerLevel_95,	IPPD_MarkerLevel_Current,	IPPD_MarkerLevel_0, 	IPPE_MarkerNames_HolePunchScrapBox, IPP_MARKER_TYPE_WASTE_PAPER	 },	
	{ IPPE_MarkerColors_Unidentified,IPPD_MarkerLevel_TERM,	IPPD_MarkerLevel_TERM,		IPPD_MarkerLevel_TERM, 	IPPE_MarkerNames_NONE, 				IPP_MARKER_TYPE_MAX		},
#endif
};
//2021/07/09 [IT6_2.2][A800Less] 制御構成変更_No.41 HIENG End.

void IPPC_LibOpeGetPrintAttr::MarkerSupplies( UCHAR (&Marker_Color)[MIO_IPP_MARKER_NUM][MAX_STRING16],
											  INT32 (&Marker_High_Levels)[MIO_IPP_MARKER_NUM], 
											  INT32 (&Marker_Levels)[MIO_IPP_MARKER_NUM], 
											  INT32 (&Marker_Low_Levels)[MIO_IPP_MARKER_NUM], 
											  UCHAR (&Maker_Name)[MIO_IPP_MARKER_NUM][MAX_STRING64], 
											  IPP_MARKER_TYPE (&Maker_Types)[MIO_IPP_MARKER_NUM] )
{
	memset( Marker_Color, NULL, sizeof(Marker_Color) );
	memset( Marker_High_Levels, NULL, sizeof(Marker_High_Levels) );
	memset( Marker_Levels, NULL, sizeof(Marker_Levels) );
	memset( Marker_Low_Levels, NULL, sizeof(Marker_Low_Levels) );
	memset( Maker_Name, NULL, sizeof(Maker_Name) );
	memset( Maker_Types, NULL, sizeof(Maker_Types) );
	
	// 2019/04/08 【EagleBK】【Control No.6】 HIENG Start.
	IPPS_MakerSupplies* st_MakerSuppliesTemp = NULL;
	ushort us_MakerSuppliesNum = 0;
	APIC_SystemInfo* pc_SystemInfo = APIC_SystemInfo::newInstance();
	if( NULL == pc_SystemInfo ) {
		DIPPTRACE0( "[IPP][fail safe] NULL.\n");
		return;
	}
	
	pc_SystemInfo->lock();
	TYPE_MachineType e_MachineType = pc_SystemInfo->getMachineType();
	pc_SystemInfo->unlock();
	//2021/07/09 [IT6_2.2][A800Less] 制御構成変更_No.41 HIENG Start.	
	if(( TYPE_MachineTypeEagleBK1 == e_MachineType )
	 ||(TYPE_MachineTypeEagleBK2 == e_MachineType ) 
	 ||(TYPE_MachineTypeEagle1 == e_MachineType ) 
	 ||(TYPE_MachineTypeEagle2 == e_MachineType ) 
	 ||(TYPE_MachineTypeEagle2_5 == e_MachineType )) {	
		st_MakerSuppliesTemp = &st_MakerSupplies_EagleL[0];
		us_MakerSuppliesNum = sizeof(st_MakerSupplies_EagleL) / sizeof(st_MakerSupplies_EagleL[0]);
	//2021/07/09 [IT6_2.2][A800Less] 制御構成変更_No.41 HIENG End.
	} else {
		st_MakerSuppliesTemp = &st_MakerSupplies[0];
		us_MakerSuppliesNum = sizeof(st_MakerSupplies) / sizeof(st_MakerSupplies[0]);
	}

	ushort i=0, uc_Count=0;
	for( i = 0; i < us_MakerSuppliesNum; i++ ) {
		//marker 出力スキップチェック.
		if( isMarkerSuppliesSkip( st_MakerSuppliesTemp[i].e_Name ) )
		{
			continue;
		}

		//marker-names.
		if( strlen( Get_MarkerNames( st_MakerSuppliesTemp[i].e_Name ) ) < MAX_STRING64 )
			strcpy( (char*)(Maker_Name[uc_Count]), Get_MarkerNames( st_MakerSuppliesTemp[i].e_Name ) );
		else
			DIPPTRACE1( "[IPP][fail safe] Maker_Name BufferOver(index=%d).\n", i );

		//marker-colors.
		if( strlen( Get_MarkerColors( st_MakerSuppliesTemp[i].e_Color ) ) < MAX_STRING16 )
			strcpy( (char*)(Marker_Color[uc_Count]), Get_MarkerColors( st_MakerSuppliesTemp[i].e_Color ) );
		else
			DIPPTRACE1( "[IPP][fail safe] Maker_Color BufferOver(index=%d).\n", i );
		
		//marker-high-levels.
		Marker_High_Levels[uc_Count] 	= st_MakerSuppliesTemp[i].i_HighLevels;

		//marker-levels.
		if( st_MakerSuppliesTemp[i].i_Levels == IPPD_MarkerLevel_Current )
			Marker_Levels[uc_Count] 		= GetMakerLevels( st_MakerSuppliesTemp[i].e_Name );	//現在の設定値.
		else
			Marker_Levels[uc_Count] 		= st_MakerSuppliesTemp[i].i_Levels;

		//marker-low-levels.
		Marker_Low_Levels[uc_Count] 		= st_MakerSuppliesTemp[i].i_LowLevels;

		//marker-types.
		Maker_Types[uc_Count] 			= st_MakerSuppliesTemp[i].e_Types;
		// 2019/04/08 【EagleBK】【Control No.6】 HIENG End.

		uc_Count++;

		// 次順で、NICFUMが用意している配列の要素数を超えるようなら.
		// 不正な配列番号を参照してしまうため、(ログを出力した上で)、処理を抜ける.
		// (このログが出力されるということは、NICFUMに要素数の追加を依頼する必要があるということ).
		if( uc_Count >= MIO_IPP_MARKER_NUM ) {
			DIPPTRACE2( "[IPP][fail safe] MarkerSupplies uc_Count = %d i = %d\n", uc_Count, i );
			break;
		}

	}
	
	return;
}
// ZeusCD UV-01249 AppleAirPrint OPS1正部家 2014/01/31 END.

// ZeusCD CQ386774 OPS1八代 2014/02/25 START.
/*================================================================================================
［機能］	設定関数 printer-input-tray.
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2014/02/25 新規作成 CQ386774.
================================================================================================*/
void IPPC_LibOpeGetPrintAttr::PrinterInputTray( UCHAR (&Printer_Input_Tray)[MAX_MEDIA_SOURCES_SUPPORTED][MAX_STRING256] )
{
	memset( Printer_Input_Tray, NULL, sizeof(Printer_Input_Tray) );

	char c_temp[MAX_STRING256];
	memset( c_temp, NULL, MAX_STRING256 );

	char uc_Rtn[MAX_STRING256];
	memset( uc_Rtn, NULL, MAX_STRING256 );

	uchar uc_Count = 0;

	// printer-input-tray は、media-source-supported と、要素数及びその順序を等しくする必要がある.
	// そのため、media-source-supported と、同様のロジックを使用(本来なら、共通化すべき).

	// 給紙トレイ自動(auto).
	// CQ391054 2014/06/17 OPS1八代 START.
//	strcpy( (char*)(Printer_Input_Tray[uc_Count]), "type=other;" );		// typeのみ設定.
//	uc_Count++;
//	strcpy( (char*)(Printer_Input_Tray[uc_Count]), "type=other;mediafeed=215900;mediaxfeed=279400;maxcapacity=150;level=0;status=8;name=Auto;" );

	TYPS_TraySearchPriority s_TraySearchPriority;
	memset( &s_TraySearchPriority, NULL, sizeof(TYPS_TraySearchPriority) );

	pc_CopyJobSetting->lock();
	s_TraySearchPriority = pc_CopyJobSetting->getTraySearchPriority();
	pc_CopyJobSetting->unlock();

	TYPE_FeedTray e_MustPriFeedTray = s_TraySearchPriority.e_FeedTray[0];

	// Poseidon CQ507116 OPS1八代  2014.11.20 START.
	// 有効な給紙トレイタイプのときのみ参照する.
	if( ( e_MustPriFeedTray == TYPE_FeedTrayManual )
	 || ( e_MustPriFeedTray == TYPE_FeedTray1 )
	 || ( e_MustPriFeedTray == TYPE_FeedTray2 )
	 || ( e_MustPriFeedTray == TYPE_FeedTray3 )
	 || ( e_MustPriFeedTray == TYPE_FeedTray4 )
	 || ( e_MustPriFeedTray == TYPE_FeedTray5 ) ) {

		// type.
		strcpy( (char*)c_temp, "type=other;" );

		// mediafeed.
		PrinterInputTray_MeFe( e_MustPriFeedTray, uc_Rtn );
		strcat( (char*)c_temp, uc_Rtn );
		memset( uc_Rtn, NULL, MAX_STRING256 );

		// mediaxfeed.
		PrinterInputTray_MeXFe( e_MustPriFeedTray, uc_Rtn );
		strcat( (char*)c_temp, uc_Rtn );
		memset( uc_Rtn, NULL, MAX_STRING256 );

		// maxcapacity.
		PrinterInputTray_MaxC( e_MustPriFeedTray, uc_Rtn );
		strcat( (char*)c_temp, uc_Rtn );
		memset( uc_Rtn, NULL, MAX_STRING256 );

		// level.
		PrinterInputTray_Leve( e_MustPriFeedTray, uc_Rtn );
		strcat( (char*)c_temp, uc_Rtn );
		memset( uc_Rtn, NULL, MAX_STRING256 );

		// status.
		PrinterInputTray_Stat( e_MustPriFeedTray, uc_Rtn );
		strcat( (char*)c_temp, uc_Rtn );
		memset( uc_Rtn, NULL, MAX_STRING256 );

		// name.
		strcat( (char*)c_temp, "name=Auto;" );				// 固定.

		// 成果物を格納.
		strcpy( (char*)(Printer_Input_Tray[uc_Count]), c_temp );
		memset( c_temp, NULL, MAX_STRING256 );

		uc_Count++;
	}
	// Poseidon CQ507116 OPS1八代  2014.11.20 END.
	// CQ391054 2014/06/17 OPS1八代 END.

	for( TYPE_FeedTray e_FeedTray = TYPE_FeedTrayManual; e_FeedTray < TYPE_FeedTrayMax; e_FeedTray++ ) {
		if( TYPE_TrayStatusNon != pc_FeedTray->getTrayStatus( e_FeedTray ) ) {

			// media-source-supported対象外はスルー.
			if( ( TYPE_FeedTrayPI1 		== e_FeedTray ) 
			 || ( TYPE_FeedTrayPI2 		== e_FeedTray )
			 || ( TYPE_FeedTrayUnselect == e_FeedTray )
			 || ( TYPE_FeedTrayMax 		== e_FeedTray ) ) {
				continue;
			}

			// type.
			PrinterInputTray_Type( e_FeedTray, uc_Rtn );
			strcpy( (char*)c_temp, uc_Rtn );
			memset( uc_Rtn, NULL, MAX_STRING256 );

			// mediafeed.
			PrinterInputTray_MeFe( e_FeedTray, uc_Rtn );
			strcat( (char*)c_temp, uc_Rtn );
			memset( uc_Rtn, NULL, MAX_STRING256 );

			// mediaxfeed.
			PrinterInputTray_MeXFe( e_FeedTray, uc_Rtn );
			strcat( (char*)c_temp, uc_Rtn );
			memset( uc_Rtn, NULL, MAX_STRING256 );

			// maxcapacity.
			PrinterInputTray_MaxC( e_FeedTray, uc_Rtn );
			strcat( (char*)c_temp, uc_Rtn );
			memset( uc_Rtn, NULL, MAX_STRING256 );

			// level.
			PrinterInputTray_Leve( e_FeedTray, uc_Rtn );
			strcat( (char*)c_temp, uc_Rtn );
			memset( uc_Rtn, NULL, MAX_STRING256 );

			// status.
			PrinterInputTray_Stat( e_FeedTray, uc_Rtn );
			strcat( (char*)c_temp, uc_Rtn );
			memset( uc_Rtn, NULL, MAX_STRING256 );

			// name.
			PrinterInputTray_Name( e_FeedTray, uc_Rtn );
			strcat( (char*)c_temp, uc_Rtn );
			memset( uc_Rtn, NULL, MAX_STRING256 );

			// 成果物を格納.
			strcpy( (char*)(Printer_Input_Tray[uc_Count]), c_temp );
			memset( c_temp, NULL, MAX_STRING256 );

			uc_Count++;

			// 次順で、NICFUMが用意している配列の要素数を超えるようなら.
			// 不正な配列番号を参照してしまうため、(ログを出力した上で)、処理を抜ける.
			// (このログが出力されるということは、NICFUMに要素数の追加を依頼する必要があるということ).
			if( uc_Count >= MAX_MEDIA_SOURCES_SUPPORTED ) {
				DIPPTRACE2( "[IPP][fail safe] PrinterInputTray uc_Count = %d e_FeedTray = %d\n", uc_Count, e_FeedTray );
				break;
			}
		}
	}

	return;
}
/*================================================================================================
［機能］	設定関数 printer-input-tray type.
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2014/02/25 新規作成 CQ386774.
			2018/05/31 DenebMLK 制御構成変更_No.6 HCL-VIJAY Modified.
================================================================================================*/
void IPPC_LibOpeGetPrintAttr::PrinterInputTray_Type( TYPE_FeedTray e_FeedTray, char* uc_Rtn )
{
	char sc_temp[40];
	memset( sc_temp, NULL, sizeof(sc_temp) ); 

	switch( e_FeedTray ) {
// CQ391054 2014/06/17 OPS1八代 START.
//		case TYPE_FeedTrayManual:	strcpy( sc_temp, "sheetFeedManual" ); 				break;	// 手差し.
		case TYPE_FeedTrayManual:	strcpy( sc_temp, "sheetFeedAutoNonRemovableTray" ); break;	// 手差し.
// CQ391054 2014/06/17 OPS1八代 END.
//2018/05/31 DenebMLK 制御構成変更_No.1 HCL-VIJAY Modified Start.
#ifdef DEF_A4_Cus // 20161222 VenusMLK Tray1 sheetFeedManual set START .
		case TYPE_FeedTray1:		strcpy( sc_temp, "sheetFeedManual" ); break;	// Tray１.
#else
		case TYPE_FeedTray1:		strcpy( sc_temp, "sheetFeedAutoNonRemovableTray" ); break;	// Tray１.
#endif// // 20161222 VenusMLK Tray1 sheetFeedManual set END .

#ifdef DEF_KRONOSS_Cus // KronosS 課題1014826対応 START.
		case TYPE_FeedTray2:		strcpy( sc_temp, "sheetFeedManual" );	break;
#elif ( defined DEF_A4_Cus || defined DEF_DenebMLKCus ) 
		case TYPE_FeedTray2:		strcpy( sc_temp, "sheetFeedAutoRemovableTray" );	break;		
#else
		case TYPE_FeedTray2:		strcpy( sc_temp, "sheetFeedAutoNonRemovableTray" ); break;
#endif // KronosS 課題1014826対応 END.
#ifdef DEF_KRONOSS_Cus // KronosS 課題1014826対応 START.
		case TYPE_FeedTray3:		strcpy( sc_temp, "sheetFeedManual" );	break;
		case TYPE_FeedTray4:		strcpy( sc_temp, "sheetFeedManual" );	break;
// HelosL 制御No1 OSP1八代 2014.11.19 START.
// ZeusSBK 制御No1 OPS1八代 2016.02.03 START.
#else
// ZeusSBK 制御No1 OPS1八代 2016.02.03 END.
		case TYPE_FeedTray3:		strcpy( sc_temp, "sheetFeedAutoRemovableTray" ); 	break;
		case TYPE_FeedTray4:		strcpy( sc_temp, "sheetFeedAutoRemovableTray" ); 	break;
//#else
//		case TYPE_FeedTray3:		strcpy( sc_temp, "sheetFeedAutoNonRemovableTray" ); break;
//		case TYPE_FeedTray4:		strcpy( sc_temp, "sheetFeedAutoNonRemovableTray" ); break;
// HelosL 制御No1 OSP1八代 2014.11.19 END.
#endif // KronosS 課題1014826対応 END.
		case TYPE_FeedTray5:		strcpy( sc_temp, "sheetFeedAutoRemovableTray" ); 	break;	// LCT.
//2018/05/31 DenebMLK 制御構成変更_No.1 HCL-VIJAY Modified End.
		default:																		break;
	}

	sprintf( uc_Rtn, "type=%s;", sc_temp );
	return;
}

/*================================================================================================
［機能］	設定関数 printer-input-tray mediafeed.
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2014/02/25 新規作成 CQ386774.
================================================================================================*/
void IPPC_LibOpeGetPrintAttr::PrinterInputTray_MeFe( TYPE_FeedTray e_FeedTray, char* uc_Rtn )
{
	INT32 i_temp = 0;

	pc_FeedTray->lock();
	TYPC_PaperSize c_PaperSize = pc_FeedTray->getPaperSize( e_FeedTray );
	pc_FeedTray->unlock();

	i_temp = (INT32)(c_PaperSize.c_SizeMM.y * 1000);

	sprintf( uc_Rtn, "mediafeed=%d;", i_temp );
	return;
}

/*================================================================================================
［機能］	設定関数 printer-input-tray mediaxfeed.
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2014/02/25 新規作成 CQ386774.
================================================================================================*/
void IPPC_LibOpeGetPrintAttr::PrinterInputTray_MeXFe( TYPE_FeedTray e_FeedTray, char* uc_Rtn )
{
	INT32 i_temp = 0;

	pc_FeedTray->lock();
	TYPC_PaperSize c_PaperSize = pc_FeedTray->getPaperSize( e_FeedTray );
	pc_FeedTray->unlock();

	i_temp = (INT32)(c_PaperSize.c_SizeMM.x * 1000);

	sprintf( uc_Rtn, "mediaxfeed=%d;", i_temp );
	return;
}

/*================================================================================================
［機能］	設定関数 printer-input-tray maxcapacity.
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2014/02/25 新規作成 CQ386774.
================================================================================================*/
void IPPC_LibOpeGetPrintAttr::PrinterInputTray_MaxC( TYPE_FeedTray e_FeedTray, char* uc_Rtn )
{
    pc_FeedTray->lock();
    short ss_temp = pc_FeedTray->getTotalPaper( e_FeedTray );
    pc_FeedTray->unlock();

	sprintf( uc_Rtn, "maxcapacity=%d;", ss_temp );
	return;
}

/*================================================================================================
［機能］	設定関数 printer-input-tray level.
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2014/02/25 新規作成 CQ386774.
================================================================================================*/
void IPPC_LibOpeGetPrintAttr::PrinterInputTray_Leve( TYPE_FeedTray e_FeedTray, char* uc_Rtn )
{
	short ss_temp = 0;

	pc_FeedTray->lock();
	TYPE_PaperStatus e_PaperStatus = pc_FeedTray->getPaperStatus( e_FeedTray );
    short ss_RemainPaper 		   = pc_FeedTray->getRemainPaper( e_FeedTray );
	pc_FeedTray->unlock();

	// Tray1, Tray2.
	if( ( TYPE_FeedTray1 == e_FeedTray ) || ( TYPE_FeedTray2 == e_FeedTray ) ) {
	    switch( e_PaperStatus ) {
			case TYPE_PaperStatusExist:			ss_temp = 500;				break;
			case TYPE_PaperStatusNearEmpty:		ss_temp =  50;				break;
			case TYPE_PaperStatusEmpty:			ss_temp =   0;				break;
			default:							ss_temp =   0;				break;
	    }

	// Tray3, LCT.
	} else if( ( TYPE_FeedTray3 == e_FeedTray ) || ( TYPE_FeedTray5 == e_FeedTray ) ) {
		ss_temp = ss_RemainPaper;

	// Tray4.
	} else if( TYPE_FeedTray4 == e_FeedTray ) {
	    switch( ss_RemainPaper ) {
			case 333:							ss_temp = 350;				break;	// 用紙残量1/3.
			case 666:							ss_temp = 650;				break;	// 用紙残量2/3.
			case  -1:							ss_temp = 	0;				break;	// トレイopen.
			default:							ss_temp = ss_RemainPaper;	break;
	    }

	// BypassTray.
    } else if( TYPE_FeedTrayManual == e_FeedTray ) {
	    switch( e_PaperStatus ) {
			case TYPE_PaperStatusExist:			ss_temp =  -3;				break;
			case TYPE_PaperStatusNearEmpty:		ss_temp =   0;				break;
			case TYPE_PaperStatusEmpty:			ss_temp =   0;				break;
			default:							ss_temp =   0;				break;
	    }

	} else {
		// ありえないケース.
	}

	sprintf( uc_Rtn, "level=%d;", ss_temp );
	return;
}

/*================================================================================================
［機能］	設定関数 printer-input-tray status.
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2014/02/25 新規作成 CQ386774.
================================================================================================*/
void IPPC_LibOpeGetPrintAttr::PrinterInputTray_Stat( TYPE_FeedTray e_FeedTray, char* uc_Rtn )
{
	short ss_temp = 0;
	//CQ526087 ZeusSZX0 HCL-Sankar 2016/03/10 Modified Start.
/*
	// 先頭に優先度の最も高いアラート情報があり、それのみを参照.
	switch( eCrntAlertTbl[0] ) {
		case MIBE_ServiceCall:						ss_temp = 19;	break;
		case MIBE_Error_PunchDustFull:				ss_temp = 17;	break;
		case MIBE_Error_StapleJam:					ss_temp = 17;	break;
		case MIBE_Error_PaperJam:					ss_temp = 17;	break;
		case MIBE_Error_PaperRemainingOutputTray:	ss_temp = 17;	break;
		case MIBE_Error_CoverOpen:					ss_temp = 17;	break;
		case MIBE_Error_NoPaperTray1:				ss_temp = 17;	break;
		case MIBE_Error_NoPaperTray2:				ss_temp = 17;	break;
		case MIBE_Error_NoPaperTray3:				ss_temp = 17;	break;
		case MIBE_Error_NoPaperTray4:				ss_temp = 17;	break;
		case MIBE_Error_NoPaperLCT:					ss_temp = 17;	break;
		case MIBE_Error_NoPaperBypassTray:			ss_temp = 17;	break;
		case MIBE_Error_SizeMediaError:				ss_temp = 17;	break;
		case MIBE_Error_SetUpError:					ss_temp = 17;	break;
		case MIBE_Warning_StapleEmpty:				ss_temp =  8;	break;
		case MIBE_Warning_PunchDustFull:			ss_temp =  8;	break;
		case MIBE_Warning_NoPaperTray1:				ss_temp =  8;	break;
		case MIBE_Warning_NoPaperTray2:				ss_temp =  8;	break;
		case MIBE_Warning_NoPaperTray3:				ss_temp =  8;	break;
		case MIBE_Warning_NoPaperTray4:				ss_temp =  8;	break;
		case MIBE_Warning_NoPaperLCT:				ss_temp =  8;	break;
		case MIBE_Warning_NoPaper2Tray1:			ss_temp =  8;	break;
		case MIBE_Warning_NoPaper2Tray2:			ss_temp =  8;	break;
		case MIBE_Warning_NoPaper2Tray3:			ss_temp =  8;	break;
		case MIBE_Warning_NoPaper2Tray4:			ss_temp =  8;	break;
		case MIBE_Warning_NoPaper2LCT:				ss_temp =  8;	break;
		case MIBE_Warning_NoPaper2BypassTray:		ss_temp =  8;	break;
		case MIBE_Warning_LowPaperTray1:			ss_temp =  8;	break;
		case MIBE_Warning_LowPaperTray2:			ss_temp =  8;	break;
		case MIBE_Warning_LowPaperTray3:			ss_temp =  8;	break;
		case MIBE_Warning_LowPaperTray4:			ss_temp =  8;	break;
		case MIBE_Warning_SizeMediaWarning:			ss_temp =  8;	break;
		default:									ss_temp =  0;	break;
	}*/
	//CQ526087 ZeusSZX0 HCL-Sankar 2016/03/10 Modified End.
	//CQ526087 ZeusSZX0 HCL-Sankar 2016/03/10 Added Start.
	//media-empty-error
	if( isMediaEmptyError( e_FeedTray) == True ) {
		ss_temp = 19;
	}
	
	// media-empty-warning
	APIC_FeedTray* pc_FeedTray = APIC_FeedTray::newInstance();
	pc_FeedTray->lock();
	if(  TYPE_PaperStatusEmpty == pc_FeedTray->getPaperStatus( e_FeedTray ) 
		  && False == pc_FeedTray->getPaperUnKnownInfo( e_FeedTray ) ) {			// 用紙状態未確定.
		ss_temp = 19;
	}
	
	//media-low-warning
	if( TYPE_PaperStatusNearEmpty == pc_FeedTray->getPaperStatus( e_FeedTray ) ) {
		ss_temp = 8;
	}
	pc_FeedTray->unlock();
	
	//media-jam
	if( IPPC_PrinterStateAndReasons::newInstance()->isMediaJamError() == True ) {
		ss_temp = 19;
	}
	//CQ526087 ZeusSZX0 HCL-Sankar 2016/03/10 Added End.
	sprintf( uc_Rtn, "status=%d;", ss_temp );
	return;
}

/*================================================================================================
［機能］	設定関数 printer-input-tray name.
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2014/02/25 新規作成 CQ386774.
================================================================================================*/
void IPPC_LibOpeGetPrintAttr::PrinterInputTray_Name( TYPE_FeedTray e_FeedTray, char* uc_Rtn )
{
	char sc_temp[64 +1];
	memset( sc_temp, NULL, sizeof(sc_temp) );

	//2018/05/31 DenebMLK 制御構成変更_No.1 HCL-VIJAY Modified Start.
	#ifndef DEF_DenebMLKCus
    pc_SystemInfo->lock();
    TYPE_LctKind e_LctKind = pc_SystemInfo->getLctKind();
    pc_SystemInfo->unlock();
	#endif
	//2018/05/31 DenebMLK 制御構成変更_No.1 HCL-VIJAY Modified End.

	pc_PswcMibSetting->lock();
	switch( e_FeedTray ) {
		case TYPE_FeedTray1: 		pc_PswcMibSetting->getprtInputName_Tray1( sc_temp ); 	 	break;
		case TYPE_FeedTray2: 		pc_PswcMibSetting->getprtInputName_Tray2( sc_temp ); 	 	break;
		case TYPE_FeedTray3: 		pc_PswcMibSetting->getprtInputName_Tray3( sc_temp ); 	 	break;
		case TYPE_FeedTray4: 		pc_PswcMibSetting->getprtInputName_Tray4( sc_temp ); 	 	break;
		case TYPE_FeedTray5:
		//2018/05/31 DenebMLK 機器構成変更_No.7 HCL-VIJAY Modified Start.
		#if ( defined DEF_DenebMLKCus  )
			pc_PswcMibSetting->getprtInputName_Tray5( sc_temp );
		#else 
			if( TYPE_LctKind_Normal == e_LctKind ) {
				pc_PswcMibSetting->getprtInputName_LCT1( sc_temp );		// A4 LCT.
			} else {
				pc_PswcMibSetting->getprtInputName_LCT2( sc_temp ); 	// A3 LCT.
			}
		#endif
		//2018/05/31 DenebMLK 機器構成変更_No.7 HCL-VIJAY Modified End.
			break;
		case TYPE_FeedTrayManual: 	pc_PswcMibSetting->getprtInputName_BypassTray( sc_temp ); 	break;
		default:																		    	break;
	}
    pc_PswcMibSetting->unlock();

	sprintf( uc_Rtn, "name=%s;", sc_temp );
	return;
}

/*================================================================================================
［機能］	設定関数 printer-output-tray.
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2014/02/25 新規作成 CQ386774.
================================================================================================*/
void IPPC_LibOpeGetPrintAttr::PrinterOutputTray( UCHAR (&Printer_Output_Tray)[MAX_IPP_OUTPUT_BIN][MAX_STRING256] )
{
	memset( Printer_Output_Tray, NULL, sizeof(Printer_Output_Tray) );

	char c_temp[MAX_STRING256];
	memset( c_temp, NULL, MAX_STRING256 );

	char uc_Rtn[MAX_STRING256];
	memset( uc_Rtn, NULL, MAX_STRING256 );

	// printer-output-tray は、output-bin-supported と、要素数及びその順序を等しくする必要がある.
	IPP_OUTPUT_BIN  Output_Bin[MAX_IPP_OUTPUT_BIN];
	OutputBinSupported( Output_Bin );

	TYPE_OutputTray e_OutputTray = TYPE_OutputTrayMain;

	for( ushort i = 0; i < MAX_IPP_OUTPUT_BIN; i++ ) {

		if( OUTPUT_BIN_END == Output_Bin[i] ) break;

		// KM定義に逆変換.
		e_OutputTray = CnvSpecRev_OutputBin( Output_Bin[i] );

		// type.
		PrinterOutputTray_Type( uc_Rtn );
		strcpy( (char*)c_temp, uc_Rtn );
		memset( uc_Rtn, NULL, MAX_STRING256 );

		// unit.
		strcat( (char*)c_temp, "unit=sheets;" );

		// maxcapacity.
		PrinterOutputTray_MaxC( e_OutputTray, uc_Rtn );
		strcat( (char*)c_temp, uc_Rtn );
		memset( uc_Rtn, NULL, MAX_STRING256 );

		// remaining.
		PrinterOutputTray_Rema( e_OutputTray, uc_Rtn );
		strcat( (char*)c_temp, uc_Rtn );
		memset( uc_Rtn, NULL, MAX_STRING256 );

		// status.
		PrinterOutputTray_Stat( e_OutputTray, uc_Rtn );
		strcat( (char*)c_temp, uc_Rtn );
		memset( uc_Rtn, NULL, MAX_STRING256 );

		// name.
		PrinterOutputTray_Name( e_OutputTray, uc_Rtn );
		strcat( (char*)c_temp, uc_Rtn );
		memset( uc_Rtn, NULL, MAX_STRING256 );

		// stackingorder.
		strcat( (char*)c_temp, "stackingorder=firstToLast;" );

		// pagedelivery.
		strcat( (char*)c_temp, "pagedelivery=faceDown;" );

		// 成果物を格納.
		strcpy( (char*)(Printer_Output_Tray[i]), c_temp );
		memset( c_temp, NULL, MAX_STRING256 );
	}

	return;
}
/*================================================================================================
［機能］	設定関数 printer-output-tray type.
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2014/02/25 新規作成 CQ386774.
================================================================================================*/
void IPPC_LibOpeGetPrintAttr::PrinterOutputTray_Type( char* uc_Rtn )
{
	char sc_temp[20];
	memset( sc_temp, NULL, sizeof(sc_temp) );

	if( IPPE_AttachFS_None == Judge_AttachFSType() ) {
		strcpy( sc_temp, "unRemovableBin" );

#ifdef DEF_A4_Cus // VenusMLK先行開発 2OPS Yamaguchi.
	} else if( IPPE_AttachFS_VenusFNS == Judge_AttachFSType() ) {
		strcpy( sc_temp, "unRemovableBin" );
#endif // VenusMLK先行開発 2OPS Yamaguchi.
#ifdef DEF_KRONOSS_Cus // KronosS フィニッシャ対応 START.
	} else if( IPPE_AttachFS_FSP02 == Judge_AttachFSType() ) {
		strcpy( sc_temp, "unRemovableBin" );
#endif // KronosS フィニッシャ対応 END.
	} else {
		strcpy( sc_temp, "removableBin" );
	}

	sprintf( uc_Rtn, "type=%s;", sc_temp );
	return;
}

/*================================================================================================
［機能］	設定関数 printer-output-tray maxcapacity.
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2014/02/25 新規作成 CQ386774.
================================================================================================*/
void IPPC_LibOpeGetPrintAttr::PrinterOutputTray_MaxC( TYPE_OutputTray e_OutputTray, char* uc_Rtn )
{
	short ss_temp = pc_FixedValueDefine->getOutputMaxCapacity( e_OutputTray );

	sprintf( uc_Rtn, "maxcapacity=%d;", ss_temp );
	return;
}

/*================================================================================================
［機能］	設定関数 printer-output-tray remaining.
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2014/02/25 新規作成 CQ386774.
================================================================================================*/
void IPPC_LibOpeGetPrintAttr::PrinterOutputTray_Rema( TYPE_OutputTray e_OutputTray, char* uc_Rtn )
{
	short ss_temp = 0;

	IPPE_AttachFS e_AttachFS = Judge_AttachFSType();

	if( ( IPPE_AttachFS_None == e_AttachFS ) || ( IPPE_AttachFS_FS533 == e_AttachFS )|| ( IPPE_AttachFS_FS542 == e_AttachFS ) ) {	// 2022/06/27 FPT-LucNH2 【IT6_2.3】 制御構成変更_No.2 Add.
		ss_temp = -2;			// unknown.

#ifdef DEF_A4_Cus // VenusMLK先行開発 2OPS Yamaguchi.
	} else if( IPPE_AttachFS_VenusFNS == e_AttachFS ) {
		ss_temp = -2;			// unknown.
#endif // VenusMLK先行開発 2OPS Yamaguchi.
#ifdef DEF_KRONOSS_Cus // KronosS フィニッシャ対応 START.
	} else if( IPPE_AttachFS_FSP02 == e_AttachFS ) {
		ss_temp = -2;			// unknown.
#endif // KronosS フィニッシャ対応 END.
	} else {
		pc_OutputTray->lock();
	    Bool b_Result = pc_OutputTray->isPaperFull( e_OutputTray );
	   	pc_OutputTray->unlock();

		if( True == b_Result ) {
			ss_temp =  0;		// Full.
		} else {
			ss_temp = -3;		// at least one.
		}
	}

	sprintf( uc_Rtn, "remaining=%d;", ss_temp );
	return;
}

/*================================================================================================
［機能］	設定関数 printer-output-tray status.
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2014/02/25 新規作成 CQ386774.
================================================================================================*/
void IPPC_LibOpeGetPrintAttr::PrinterOutputTray_Stat( TYPE_OutputTray e_OutputTray, char* uc_Rtn )
{
	short ss_temp = 0;

	// 先頭に優先度の最も高いアラート情報があり、それのみを参照.
	switch( eCrntAlertTbl[0] ) {
		case MIBE_ServiceCall:						ss_temp = 19;	break;
		case MIBE_Error_StapleEmpty:				ss_temp = 17;	break;
		case MIBE_Error_PunchDustFull:				ss_temp = 17;	break;
		case MIBE_Error_StapleJam:					ss_temp = 17;	break;
		case MIBE_Error_PaperJam:					ss_temp = 17;	break;
		case MIBE_Error_PaperRemainingOutputTray:	ss_temp = 17;	break;
		case MIBE_Error_CoverOpen:					ss_temp = 17;	break;
		default:									ss_temp =  0;	break;
	}

	sprintf( uc_Rtn, "status=%d;", ss_temp );
	return;
}

/*================================================================================================
［機能］	設定関数 printer-output-tray name.
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2014/02/25 新規作成 CQ386774.
================================================================================================*/
void IPPC_LibOpeGetPrintAttr::PrinterOutputTray_Name( TYPE_OutputTray e_OutputTray, char* uc_Rtn )
{
	char sc_temp[64 +1];
	memset( sc_temp, NULL, sizeof(sc_temp) );

	IPPE_AttachFS e_AttachFS = Judge_AttachFSType();

	pc_PswcMibSetting->lock();
	switch ( e_AttachFS ) {
		case IPPE_AttachFS_None: 
			pc_PswcMibSetting->getprtOutputName_StandardBin( sc_temp );
			break;

		case IPPE_AttachFS_JS602: 
			switch ( e_OutputTray ) {
				case TYPE_OutputTrayMain: 	pc_PswcMibSetting->getprtOutputName_JS_Tray2( sc_temp );	break;
				case TYPE_OutputTraySub1:	pc_PswcMibSetting->getprtOutputName_JS_Tray1( sc_temp );	break;
				default:																				break;
			}
			break;

		case IPPE_AttachFS_FS542: 	// 2022/06/27 FPT-LucNH2 【IT6_2.3】 制御構成変更_No.2 Add.
		case IPPE_AttachFS_FS533: 
			switch ( e_OutputTray ) {
				case TYPE_OutputTrayMain: 	pc_PswcMibSetting->getprtOutputName_FS_Tray1( sc_temp );	break;
				default:																				break;
			}
			break;

		case IPPE_AttachFS_FS534: 
		case IPPE_AttachFS_FS535: 
		case IPPE_AttachFS_FS537:	// MinervaSBK 制御No15_FS-537 OPS1八代 2015.04.06.
		case IPPE_AttachFS_FS540:	// 2018/10/17 FPT-LucNH2 【IT6_2.0】【EagleH】 機器構成変更_No.5 Add.
			switch ( e_OutputTray ) {
				case TYPE_OutputTrayMain: 	pc_PswcMibSetting->getprtOutputName_Tray2( sc_temp );			break;
				case TYPE_OutputTraySub1:	pc_PswcMibSetting->getprtOutputName_Tray1( sc_temp );			break;
				case TYPE_OutputTraySub2:	pc_PswcMibSetting->getprtOutputName_AdditionalTray( sc_temp );	break;
				case TYPE_OutputTraySub3:	pc_PswcMibSetting->getprtOutputName_Tray3( sc_temp );	break;		// ZeusS ZX0 機器構成変更_No.3対応 HCL-VIJAY 2015/06/22 Added.
				default:																					break;
			}
			break;
		
#ifdef DEF_A4_Cus // VenusMLK先行開発 2OPS Yamaguchi.
		case IPPE_AttachFS_VenusFNS: 
#ifdef DEF_KRONOSS_Cus // KronosS フィニッシャ対応 START.
		case IPPE_AttachFS_FSP02: 
#endif // KronosS フィニッシャ対応 END.
			pc_PswcMibSetting->getprtOutputName_StandardBin( sc_temp );
			break;
#endif // VenusMLK先行開発 2OPS Yamaguchi.

		// MinervaSBK 制御No14_FS-536 OPS1八代 2015.03.26 START.
		case IPPE_AttachFS_FS536: 
		case IPPE_AttachFS_FS539:	// 2018/10/17 FPT-LucNH2 【IT6_2.0】【EagleH】 機器構成変更_No.5 Add.
			switch ( e_OutputTray ) {
				case TYPE_OutputTrayMain: 	pc_PswcMibSetting->getprtOutputName_Tray2( sc_temp );			break;
				case TYPE_OutputTraySub1:	pc_PswcMibSetting->getprtOutputName_Tray1( sc_temp );			break;
				case TYPE_OutputTraySub3:	pc_PswcMibSetting->getprtOutputName_Tray3( sc_temp );	break;		// ZeusS ZX0 機器構成変更_No.3対応 HCL-VIJAY 2015/06/22 Added.
				default:																					break;
			}
			break;
		// MinervaSBK 制御No14_FS-536 OPS1八代 2015.03.26 END.
		default:
			break;
	}
	pc_PswcMibSetting->unlock();

	sprintf( uc_Rtn, "name=%s;", sc_temp );
	return;
}
// ZeusCD CQ386774 OPS1八代 2014/02/25 END.

//2021/09/24 [MinervaSSBK][OP_BTS-35757] HIENG Start.
#define IPPD_GetPrinterfinisher(uc_Index, FinisherType, Maxcapicity) \
	strncpy( (char*)PrinterFinisher[uc_Index], FinisherType, MAX_STRING64-1); \
	strncat( (char*)PrinterFinisher[uc_Index], "unit=sheets;", MAX_STRING64-1); \
	strncat( (char*)PrinterFinisher[uc_Index], Maxcapicity, MAX_STRING64-1); \
	strncat( (char*)PrinterFinisher[uc_Index], "capacity=-2", MAX_STRING64-1); 
//2021/09/24 [MinervaSSBK][OP_BTS-35757] HIENG End.
/*================================================================================================
［機能］	設定関数 printer-finisher.
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
[履歴]     2015/02/03  HeliosL Airprint-1.5 Support
================================================================================================*/
void IPPC_LibOpeGetPrintAttr::GetPrinterfinisher(UINT8 (&PrinterFinisher)[MAX_IPP_PRINTER_FINISHER][MAX_STRING64])
{
	uchar uc_Index=0;
	pc_PrinterSystem->lock();
	TYPE_FinisherType e_FinisherType = pc_PrinterSystem->getFinisherType();
	pc_PrinterSystem->unlock();
	switch( e_FinisherType ){		
		case TYPE_FinisherTypeFS533:	// FS-533.
			// Staple Details.
			IPPD_GetPrinterfinisher( uc_Index,"type=staple;","maxcapacity=50;" );				//2021/09/24 [MinervaSSBK][OP_BTS-35757] HIENG.
			// Check whether Punch-Kit is installed or not
			if( IPPE_AttachPK_PK519 == Judge_AttachPKType()) {
				uc_Index++;
				IPPD_GetPrinterfinisher( uc_Index,"type=punch;","maxcapacity=-2;" ); 			//2021/09/24 [MinervaSSBK][OP_BTS-35757] HIENG.
			}
			break;
		case TYPE_FinisherTypeFS534:	// FS-534.
		case TYPE_FinisherTypeFS536:	// FS-536.	// MinervaSBK 制御No14_FS-536 OPS1八代 2015.03.26.
			// Staple Details.
			IPPD_GetPrinterfinisher( uc_Index,"type=staple;","maxcapacity=50;" ); 				//2021/09/24 [MinervaSSBK][OP_BTS-35757] HIENG.
			// Check whether Punch-Kit is installed or not
			if(IPPE_AttachPK_PK520 == Judge_AttachPKType()) {
				uc_Index++;
				IPPD_GetPrinterfinisher( uc_Index,"type=punch;","maxcapacity=-2;" );			//2021/09/24 [MinervaSSBK][OP_BTS-35757] HIENG.
			}
			// Check whether Saddle-Stitcher is installed or not
			if(IPPE_AttachSD_SD511 == Judge_AttachSDType()) {
				uc_Index++;
				IPPD_GetPrinterfinisher( uc_Index,"type=saddle-stitch;","maxcapacity=-2;" );	//2021/09/24 [MinervaSSBK][OP_BTS-35757] HIENG.
			}
			break;
		case TYPE_FinisherTypeFS535:	// FS-535.
		//MinervaSBK 制御構成変更_No.1-2 パンチキット名称変更 HCL-Sankar 2015/08/28 Modified START.
			// Staple Details.
			IPPD_GetPrinterfinisher( uc_Index,"type=staple;","maxcapacity=100;" ); 				//2021/09/24 [MinervaSSBK][OP_BTS-35757] HIENG.
			// Check whether Punch-Kit is installed or not
			if(IPPE_AttachPK_PK521 == Judge_AttachPKType()) {
				uc_Index++;
				IPPD_GetPrinterfinisher( uc_Index,"type=punch;","maxcapacity=-2;" );			//2021/09/24 [MinervaSSBK][OP_BTS-35757] HIENG.
			}
			// Check whether Saddle-Stitcher is installed or not
			if(IPPE_AttachSD_SD512 == Judge_AttachSDType()) {
				uc_Index++;
				IPPD_GetPrinterfinisher( uc_Index,"type=saddle-stitch;","maxcapacity=-2;" );	//2021/09/24 [MinervaSSBK][OP_BTS-35757] HIENG.
			}
			// Check whether Z- Folding Unit is installed or not
			if(isAttachZU()) {
				uc_Index++;
				IPPD_GetPrinterfinisher( uc_Index,"type=fold;","maxcapacity=-2;" );	 			//2021/09/24 [MinervaSSBK][OP_BTS-35757] HIENG.
			}
			break;
		case TYPE_FinisherTypeFS537:	// FS-537.	// MinervaSBK 制御No15_FS-537 OPS1八代 2015.04.06.
			// Staple Details.
			IPPD_GetPrinterfinisher( uc_Index,"type=staple;","maxcapacity=100;" ); 				//2021/09/24 [MinervaSSBK][OP_BTS-35757] HIENG.
			// Check whether Punch-Kit is installed or not
			if(IPPE_AttachPK_PK523 == Judge_AttachPKType()) {
				uc_Index++;
				IPPD_GetPrinterfinisher( uc_Index,"type=punch;","maxcapacity=-2;" );			//2021/09/24 [MinervaSSBK][OP_BTS-35757] HIENG.
			}
			// Check whether Saddle-Stitcher is installed or not
			if(IPPE_AttachSD_SD512 == Judge_AttachSDType()) {
				uc_Index++;
				IPPD_GetPrinterfinisher( uc_Index,"type=saddle-stitch;","maxcapacity=-2;" );	//2021/09/24 [MinervaSSBK][OP_BTS-35757] HIENG.
			}
			// Check whether Z- Folding Unit is installed or not
			if(isAttachZU()) {
				uc_Index++;
				IPPD_GetPrinterfinisher( uc_Index,"type=fold;","maxcapacity=-2;" );				//2021/09/24 [MinervaSSBK][OP_BTS-35757] HIENG.
			}
			break;
		//MinervaSBK 制御構成変更_No.1-2 パンチキット名称変更 HCL-Sankar 2015/08/28 Modified END.
// 2018/10/17 FPT-LucNH2 【IT6_EagleH】【EagleH】 機器構成変更_No.5 Start.
		case TYPE_FinisherTypeFS539:	// FS-539.
			// Staple Details.
			IPPD_GetPrinterfinisher( uc_Index,"type=staple;","maxcapacity=50;" ); 				//2021/09/24 [MinervaSSBK][OP_BTS-35757] HIENG.
			// Check whether Punch-Kit is installed or not
			if(IPPE_AttachPK_PK524 == Judge_AttachPKType()) {
				uc_Index++;
				IPPD_GetPrinterfinisher( uc_Index,"type=punch;","maxcapacity=-2;" );			//2021/09/24 [MinervaSSBK][OP_BTS-35757] HIENG.
			}
			// Check whether Saddle-Stitcher is installed or not
			if(IPPE_AttachSD_SD511 == Judge_AttachSDType()) {
				uc_Index++;
				IPPD_GetPrinterfinisher( uc_Index,"type=saddle-stitch;","maxcapacity=-2;" );	//2021/09/24 [MinervaSSBK][OP_BTS-35757] HIENG.
			}
			break;
		case TYPE_FinisherTypeFS540:	// FS-540.
			// Staple Details.
			IPPD_GetPrinterfinisher(uc_Index,"type=staple;","maxcapacity=100;");				//2021/09/24 [MinervaSSBK][OP_BTS-35757] HIENG.
			// Check whether Punch-Kit is installed or not
			if(IPPE_AttachPK_PK526 == Judge_AttachPKType()) {
				uc_Index++;
				IPPD_GetPrinterfinisher(uc_Index,"type=punch;","maxcapacity=-2;");				//2021/09/24 [MinervaSSBK][OP_BTS-35757] HIENG.
			}
			// Check whether Saddle-Stitcher is installed or not
			if(IPPE_AttachSD_SD512 == Judge_AttachSDType()) {
				uc_Index++;
				IPPD_GetPrinterfinisher(uc_Index,"saddle-stitch;","maxcapacity=-2;");			//2021/09/24 [MinervaSSBK][OP_BTS-35757] HIENG.
			}
			// Check whether Z- Folding Unit is installed or not
			if(isAttachZU()) {
				uc_Index++;
				IPPD_GetPrinterfinisher(uc_Index,"type=fold;","maxcapacity=-2;");				//2021/09/24 [MinervaSSBK][OP_BTS-35757] HIENG.
			}
			break;
// 2018/10/17 FPT-LucNH2 【IT6_EagleH】【EagleH】 機器構成変更_No.5 End.
// 2022/06/27 FPT-LucNH2 【IT6_2.3】 制御構成変更_No.2 Add Start.
		case TYPE_FinisherTypeFS542:	// FS-542.
			// Staple Details.
			IPPD_GetPrinterfinisher( uc_Index,"type=staple;","maxcapacity=50;" );
			// Check whether Punch-Kit is installed or not
			if( IPPE_AttachPK_PK527 == Judge_AttachPKType()) {
				uc_Index++;
				IPPD_GetPrinterfinisher( uc_Index,"type=punch;","maxcapacity=-2;" );
			}
			break;
// 2022/06/27 FPT-LucNH2 【IT6_2.3】 制御構成変更_No.2 Add End.
		default:
			strcpy( (char*)PrinterFinisher[uc_Index], "" );
			break;
	}
}
/*================================================================================================
［機能］	設定関数 printer-finisher-description.
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
[履歴]     2015/02/03  HeliosL Airprint-1.5 Support
================================================================================================*/
void IPPC_LibOpeGetPrintAttr::GetPrinterfinisherDescription(UINT8 (&PrinterFinisherDescription)[MAX_IPP_PRINTER_FINISHER][MAX_STRING64])
{
	uchar uc_Index=0;
    Bool isOwn = pc_SystemInfo->isOwnOrGeneric();	// OwnかGenericかを判別する
    pc_PrinterSystem->lock();
    TYPE_FinisherType e_FinisherType = pc_PrinterSystem->getFinisherType();
    pc_PrinterSystem->unlock();
    switch( e_FinisherType ){		
		case TYPE_FinisherTypeFS533:	// FS-533.
			// Staple Details.
			if ( isOwn ) {
				strcpy( (char*)PrinterFinisherDescription[uc_Index], "FS-533" );			// 'FS-535'.
			} else {
				strcpy( (char*)PrinterFinisherDescription[uc_Index], "Finisher" );				// 'Finisher'.
			}
			// Check whether Punch-Kit is installed or not
			if( IPPE_AttachPK_PK519 == Judge_AttachPKType()) {
				uc_Index++;
				if ( isOwn ) {
					strcpy( (char*)PrinterFinisherDescription[uc_Index], "PK-519" );						// 'PK-519'.
				} else {
					strcpy( (char*)PrinterFinisherDescription[uc_Index], "Punch Kit" );						// 'Punch Kit'.
				}
			}
			break;
		case TYPE_FinisherTypeFS534:	// FS-534.
			// Staple Details.
			if ( isOwn ) {
				strcpy( (char*)PrinterFinisherDescription[uc_Index], "FS-534" );							// 'FS-534'.
			} else {
				strcpy( (char*)PrinterFinisherDescription[uc_Index], "Floor Type Finisher" );				// 'Finisher'.
			}
			// Check whether Punch-Kit is installed or not
			if(IPPE_AttachPK_PK520 == Judge_AttachPKType()) {
				uc_Index++;
				if ( isOwn ) {
					strcpy( (char*)PrinterFinisherDescription[uc_Index], "PK-520" );			// 'PK-520'.
				} else {
					strcpy( (char*)PrinterFinisherDescription[uc_Index] ,"Punch Kit" );			// 'Punch-Kit'.
				}
			}
			// Check whether Saddle-Stitcher is installed or not
			if(IPPE_AttachSD_SD511 == Judge_AttachSDType()) {
				uc_Index++;
				if ( isOwn ) {
					strcpy( (char*)PrinterFinisherDescription[uc_Index], "SD-511" );			// 'SD-511'.
				} else {
					strcpy( (char*)PrinterFinisherDescription[uc_Index], "Saddle Stitch" );	// 'saddle-stitch'.
				}
			}
			break;
		case TYPE_FinisherTypeFS535:	// FS-535.
			// Staple Details.
			if ( isOwn ) {
				strcpy( (char*)PrinterFinisherDescription[uc_Index], "FS-535" );				// 'FS-535'.
			} else {
				strcpy( (char*)PrinterFinisherDescription[uc_Index], "Finisher" );				// 'Finisher'.
			}
			// Check whether Punch-Kit is installed or not
			if(IPPE_AttachPK_PK521 == Judge_AttachPKType()) {
				uc_Index++;
				if ( isOwn ) {
					strcpy( (char*)PrinterFinisherDescription[uc_Index], "PK-521" );			// 'PK-521'.
				} else {
					strcpy( (char*)PrinterFinisherDescription[uc_Index], "Punch Kit" );		// 'Punch-Kit'.
				}
			}
			// Check whether Saddle-Stitcher is installed or not
			if(IPPE_AttachSD_SD512 == Judge_AttachSDType()) {
				uc_Index++;
				if ( isOwn ) {
					strcpy( (char*)PrinterFinisherDescription[uc_Index], "SD-512" );			// 'SD-512'.
				} else {
					strcpy( (char*)PrinterFinisherDescription[uc_Index], "Saddle Stitch" );	// 'saddle-stitch'.
				}
			}
			// Check whether Z- Folding Unit is installed or not
			if(isAttachZU()) {
				uc_Index++;
				if ( isOwn ) {
					strcpy( (char*)PrinterFinisherDescription[uc_Index], "ZU-606" );			// 'ZU-606'.
				} else {
					strcpy( (char*)PrinterFinisherDescription[uc_Index], "Z Folding Unit" );	// 'Z Folding Unit'.
				}
			}
			break;

		// MinervaSBK 制御No14_FS-536 OPS1八代 2015.03.26 START.
		case TYPE_FinisherTypeFS536:	// FS-536.
			// Staple Details.
			if ( isOwn ) {
				strcpy( (char*)PrinterFinisherDescription[uc_Index], "FS-536" );							// 'FS-534'.
			} else {
				strcpy( (char*)PrinterFinisherDescription[uc_Index], "Floor Type Finisher" );				// 'Finisher'.
			}
			// Check whether Punch-Kit is installed or not
			if(IPPE_AttachPK_PK520 == Judge_AttachPKType()) {
				uc_Index++;
				if ( isOwn ) {
					strcpy( (char*)PrinterFinisherDescription[uc_Index], "PK-520" );			// 'PK-520'.
				} else {
					strcpy( (char*)PrinterFinisherDescription[uc_Index] ,"Punch Kit" );			// 'Punch-Kit'.
				}
			}
			// Check whether Saddle-Stitcher is installed or not
			if(IPPE_AttachSD_SD511 == Judge_AttachSDType()) {
				uc_Index++;
				if ( isOwn ) {
					strcpy( (char*)PrinterFinisherDescription[uc_Index], "SD-511" );			// 'SD-511'.
				} else {
					strcpy( (char*)PrinterFinisherDescription[uc_Index], "Saddle Stitch" );	// 'saddle-stitch'.
				}
			}
			break;
		// MinervaSBK 制御No14_FS-536 OPS1八代 2015.03.26 END.

		// MinervaSBK 制御No15_FS-537 OPS1八代 2015.04.06 START.
		case TYPE_FinisherTypeFS537:	// FS-537.
			// Staple Details.
			if ( isOwn ) {
				strcpy( (char*)PrinterFinisherDescription[uc_Index], "FS-537" );				// 'FS-537'.
			} else {
				strcpy( (char*)PrinterFinisherDescription[uc_Index], "Finisher" );				// 'Finisher'.
			}
			// Check whether Punch-Kit is installed or not
			if(IPPE_AttachPK_PK523 == Judge_AttachPKType()) {			//MinervaSBK 制御構成変更_No.1-2 パンチキット名称変更 HCL-Sankar 2015/08/28 Modified.
				uc_Index++;

				// MinervaSBK 機器No19_FS-537 OPS1八代 2015.04.22 START.
				// FS-537のPKはシリアルナンバーに対応している.
				char sc_SerialNoPK[TYPD_MAX_SerialNoPK] = {};

				pc_SystemInfo->lock();
				pc_SystemInfo->getSerialNoPK( sc_SerialNoPK );
				pc_SystemInfo->unlock();

				strncpy( (char*)PrinterFinisherDescription[uc_Index], sc_SerialNoPK, TYPD_MAX_SerialNoPK );
				// MinervaSBK 機器No19_FS-537 OPS1八代 2015.04.22 EMD.
			}
			// Check whether Saddle-Stitcher is installed or not
			if(IPPE_AttachSD_SD512 == Judge_AttachSDType()) {
				uc_Index++;
				if ( isOwn ) {
					strcpy( (char*)PrinterFinisherDescription[uc_Index], "SD-512" );			// 'SD-512'.
				} else {
					strcpy( (char*)PrinterFinisherDescription[uc_Index], "Saddle Stitch" );	// 'saddle-stitch'.
				}
			}
			// Check whether Z- Folding Unit is installed or not
			if(isAttachZU()) {
				uc_Index++;
				if ( isOwn ) {
					strcpy( (char*)PrinterFinisherDescription[uc_Index], "ZU-609" );			// 'ZU-609'.
				} else {
					strcpy( (char*)PrinterFinisherDescription[uc_Index], "Z Folding Unit" );	// 'Z Folding Unit'.
				}
			}
			break;		// 2019/03/12 FPT-LucNH2 IT6_2.0 【OP_BTS-2893】 fix warning.
// 2018/10/17 FPT-LucNH2 【IT6_2.0】【EagleH】 機器構成変更_No.5 Start.
		case TYPE_FinisherTypeFS539:	// FS-539.
			// Staple Details.
			if ( isOwn ) {
				strcpy( (char*)PrinterFinisherDescription[uc_Index], "FS-539" );							// 'FS-539'.
			} else {
				strcpy( (char*)PrinterFinisherDescription[uc_Index], "Floor Type Finisher" );				// 'Finisher'.
			}
			// Check whether Punch-Kit is installed or not
			if(IPPE_AttachPK_PK524 == Judge_AttachPKType()) {
				uc_Index++;
				if ( isOwn ) {
					strcpy( (char*)PrinterFinisherDescription[uc_Index], "PK-524" );			// 'PK-524'.
				} else {
					strcpy( (char*)PrinterFinisherDescription[uc_Index] ,"Punch Kit" );			// 'Punch-Kit'.
				}
			}
			// Check whether Saddle-Stitcher is installed or not
			if(IPPE_AttachSD_SD511 == Judge_AttachSDType()) {
				uc_Index++;
				if ( isOwn ) {
					strcpy( (char*)PrinterFinisherDescription[uc_Index], "SD-511" );			// 'SD-511'.
				} else {
					strcpy( (char*)PrinterFinisherDescription[uc_Index], "Saddle Stitch" );	// 'saddle-stitch'.
				}
			}
			break;
		case TYPE_FinisherTypeFS540:	// FS-540.
			// Staple Details.
			if ( isOwn ) {
				strcpy( (char*)PrinterFinisherDescription[uc_Index], "FS-540" );				// 'FS-540'.
			} else {
				strcpy( (char*)PrinterFinisherDescription[uc_Index], "Finisher" );				// 'Finisher'.
			}
			// Check whether Punch-Kit is installed or not
			if(IPPE_AttachPK_PK526 == Judge_AttachPKType()) {
				uc_Index++;

				// FS-540のPKはシリアルナンバーに対応している.
				char sc_SerialNoPK[TYPD_MAX_SerialNoPK] = {};

				pc_SystemInfo->lock();
				pc_SystemInfo->getSerialNoPK( sc_SerialNoPK );
				pc_SystemInfo->unlock();

				strncpy( (char*)PrinterFinisherDescription[uc_Index], sc_SerialNoPK, TYPD_MAX_SerialNoPK );
			}
			// Check whether Saddle-Stitcher is installed or not
			if(IPPE_AttachSD_SD512 == Judge_AttachSDType()) {
				uc_Index++;
				if ( isOwn ) {
					strcpy( (char*)PrinterFinisherDescription[uc_Index], "SD-512" );			// 'SD-512'.
				} else {
					strcpy( (char*)PrinterFinisherDescription[uc_Index], "Saddle Stitch" );	// 'saddle-stitch'.
				}
			}
			// Check whether Z- Folding Unit is installed or not
			if(isAttachZU()) {
				uc_Index++;
				if ( isOwn ) {
					strcpy( (char*)PrinterFinisherDescription[uc_Index], "SK-704" );			// 'SK-704'.
				} else {
					strcpy( (char*)PrinterFinisherDescription[uc_Index], "Z Folding Unit" );	// 'Z Folding Unit'.
				}
			}
// 2018/10/17 FPT-LucNH2 【IT6_2.0】【EagleH】 機器構成変更_No.5 End.
			break;
		// MinervaSBK 制御No15_FS-537 OPS1八代 2015.04.06 END.
// 2022/06/27 FPT-LucNH2 【IT6_2.3】 制御構成変更_No.2 Add Start.
		case TYPE_FinisherTypeFS542:	// FS-542.
			// Staple Details.
			if ( isOwn ) {
				strcpy( (char*)PrinterFinisherDescription[uc_Index], "FS-542" );			// 'FS-542'.
			} else {
				strcpy( (char*)PrinterFinisherDescription[uc_Index], "Finisher" );				// 'Finisher'.
			}
			// Check whether Punch-Kit is installed or not
			if( IPPE_AttachPK_PK527 == Judge_AttachPKType()) {
				uc_Index++;
				if ( isOwn ) {
					strcpy( (char*)PrinterFinisherDescription[uc_Index], "PK-527" );						// 'PK-527'.
				} else {
					strcpy( (char*)PrinterFinisherDescription[uc_Index], "Punch Kit" );						// 'Punch Kit'.
				}
			}
			break;
// 2022/06/27 FPT-LucNH2 【IT6_2.3】 制御構成変更_No.2 Add End.

		default:
			strcpy( (char*)PrinterFinisherDescription[uc_Index], "" );
			break;
	}
}

/*================================================================================================
［機能］	設定関数 mopria-version.
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     Helios KRQ-101 AirPrint1.5対応 HCL-Vijay 2015/03/09 Added
================================================================================================*/
static struct IPPS_CnvMopriaVer {
	TYPE_MopriaLicence		e_ComMopriaver;// Common API Enum
	MIO_MOPRIA_VERSION	e_LibMopriaver;		// NICFUM ENUM
} st_CnvTable_CnvMopriaVer  [] = {
	{ TYPE_MopriaLicence_Ver1_1, 	 	MIO_MOPRIA_1_1		},
	{ TYPE_MopriaLicence_Ver1_2, 	 	MIO_MOPRIA_1_2		},
	{ TYPE_MopriaLicence_Ver1_3, 	 	MIO_MOPRIA_1_3		},  // MinervaS BK WP1796 制御構成変更_No.71 added 2015/09/02 HCL-Sankar.
	{ TYPE_MopriaLicence_Ver2_0, 	 	MIO_MOPRIA_2_0		},	// 2018/07/09 FPT-CuongVV7 IT6_1.0 [EAGLE][SPARROW] 制御構成変更_No.114-1
	{ TYPE_MopriaLicence_Ver2_0_1, 	 	MIO_MOPRIA_2_0		},	// 2019/06/17 ht_kurihara [EagleH]OP_BTS-7982 mopria certified.
	{ TYPE_MopriaLicence_Ver2_1, 	 	MIO_MOPRIA_2_1		},	// 2022/10/27 [IT6_2.3] [FPT-HuuTX] [制御構成変更_No.68] Modify.
	{ TYPE_MopriaLicence_None, 			MIO_MOPRIA_END		}
};

MIO_MOPRIA_VERSION IPPC_LibOpeGetPrintAttr::getMopriaVersion()
{
	MIO_MOPRIA_VERSION e_Return = MIO_MOPRIA_END;		// NICFUM Enum
	TYPE_MopriaLicence e_ComMopriaver = TYPE_MopriaLicence_None;	// CommonAPI Enum
	
	pc_SystemInfo->lock();
	e_ComMopriaver = pc_SystemInfo->getMopriaLicenceStatus();
	pc_SystemInfo->unlock();

	for( ushort i = 0; i < sizeof( st_CnvTable_CnvMopriaVer ) / sizeof( IPPS_CnvMopriaVer ); i++ ) {
		if( e_ComMopriaver == st_CnvTable_CnvMopriaVer[i].e_ComMopriaver ) {
			e_Return = st_CnvTable_CnvMopriaVer[i].e_LibMopriaver;
			break;
		}
	}

	return e_Return;
}
/*================================================================================================
［機能］	設定関数 ipp-features-supported.
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     MinervaSBK 制御構成変更_No.71 (WP1797) 2015/09/10 HCL-Sankar ADDED.
================================================================================================*/
void IPPC_LibOpeGetPrintAttr::IppFeaturesSupported( IPP_FEATURES_SUPPORTED (&Ipp_Features_Supported)[MAX_IPP_FEATURES_SUPPORTED], IPP_REQUEST_TYPE e_RequestType )
{
	// 初期化.
	for(ushort i = 0; i < MAX_IPP_FEATURES_SUPPORTED; i++ ) Ipp_Features_Supported[i] = IPP_FEATURES_SUPPORTED_END;

	Bool bl_AirPrintEnable = pc_AirPrintSetting->getEnableAirPrint();	// AirPrint有効無効.
	TYPE_AirPrintLicence e_AirprintLicence = IPP_GetAirPrintVersion();	// AirPrintラインセンス.
	TYPE_MopriaLicence e_MopriaLicence = IPP_GetMopriaVersion();		// Mopriaラインセンス.

	Bool bl_AirPrint = False;

	// AirPrint.
	if( True == bl_AirPrintEnable ) {
		switch( e_AirprintLicence ) {

		//IT6_2.2 Airprint2.1対応(RQ-890) ht_kurihara 2021/2/25 added Start.
			case TYPE_AirPrintLicence_Ver2_1:
				Ipp_Features_Supported[0] = AIRPRINT_21;
				Ipp_Features_Supported[1] = PAGE_OVERRIDES;
				if( IPP_REQ_TYPE_FAXOUT == e_RequestType ) {
					Ipp_Features_Supported[2] = IPP_FEATURES_FAXOUT;
				}
				bl_AirPrint = True;		// AirPrintライセンスが有効かつ、AirPrint設定も有効の場合.
				break;
		//IT6_2.2 Airprint2.1対応(RQ-890) ht_kurihara 2021/2/25 added End.
		//Eagle_Sparrow Airprint1.8対応(制御構成変更_No.114) HCL_VIJAY 2018/4/26 added Start.
			case TYPE_AirPrintLicence_Ver1_8:
				Ipp_Features_Supported[0] = AIRPRINT_18;
				Ipp_Features_Supported[1] = PAGE_OVERRIDES;
				if( IPP_REQ_TYPE_FAXOUT == e_RequestType ) {
					Ipp_Features_Supported[2] = IPP_FEATURES_FAXOUT;
				}
				bl_AirPrint = True;		// AirPrintライセンスが有効かつ、AirPrint設定も有効の場合.
				break;
		//Eagle_Sparrow Airprint1.8対応(制御構成変更_No.114) HCL_VIJAY 2018/4/26 added End.
		//IT5_4.1 Airprint1.7対応 HCL_VIJAY 2017/1/30 added Start.
			case TYPE_AirPrintLicence_Ver1_7:
				Ipp_Features_Supported[0] = AIRPRINT_17;
				Ipp_Features_Supported[1] = PAGE_OVERRIDES;
				if( IPP_REQ_TYPE_FAXOUT == e_RequestType ) {
					Ipp_Features_Supported[2] = IPP_FEATURES_FAXOUT;
				}
				bl_AirPrint = True;		// AirPrintライセンスが有効かつ、AirPrint設定も有効の場合.
				break;
		//IT5_4.1 Airprint1.7対応 HCL_VIJAY 2017/1/30 added End.
		//Icarus Airprint1.6対応 HCL_Sankar 2015/12/10 added Start.
			case TYPE_AirPrintLicence_Ver1_6:
				Ipp_Features_Supported[0] = AIRPRINT_16;
				Ipp_Features_Supported[1] = PAGE_OVERRIDES;
				if( IPP_REQ_TYPE_FAXOUT == e_RequestType ) {
					Ipp_Features_Supported[2] = IPP_FEATURES_FAXOUT;
				}
				bl_AirPrint = True;		// AirPrintライセンスが有効かつ、AirPrint設定も有効の場合.
				break;
		//Icarus Airprint1.6対応 HCL_Sankar 2015/12/10 added End.
			case TYPE_AirPrintLicence_Ver1_5:
				Ipp_Features_Supported[0] = AIRPRINT_15;
				Ipp_Features_Supported[1] = PAGE_OVERRIDES;
				if( IPP_REQ_TYPE_FAXOUT == e_RequestType ) {
					Ipp_Features_Supported[2] = IPP_FEATURES_FAXOUT;
				}
				bl_AirPrint = True;		// AirPrintライセンスが有効かつ、AirPrint設定も有効の場合.
				break;

			case TYPE_AirPrintLicence_Ver1_4:
				Ipp_Features_Supported[0] = AIRPRINT_14;
				if( IPP_REQ_TYPE_FAXOUT == e_RequestType ) {
					Ipp_Features_Supported[1] = IPP_FEATURES_FAXOUT;
				}
				bl_AirPrint = True;		// AirPrintライセンスが有効かつ、AirPrint設定も有効の場合.
				break;

			default:
				break;
		}
	}

	// Mopiria.
	if( False == bl_AirPrint ) {
		switch( e_MopriaLicence ) {
			case TYPE_MopriaLicence_Ver1_1:
				break;

			case TYPE_MopriaLicence_Ver1_2:
			case TYPE_MopriaLicence_Ver1_3:
			case TYPE_MopriaLicence_Ver2_0:	// 2018/07/09 FPT-CuongVV7 IT6_1.0 [EAGLE][SPARROW] 制御構成変更_No.114-1
			case TYPE_MopriaLicence_Ver2_0_1:	// 2019/06/17 ht_kurihara [EagleH]OP_BTS-7982.
			case TYPE_MopriaLicence_Ver2_1:		// 2022/10/27 [IT6_2.3] [FPT-HuuTX] [制御構成変更_No.68] Modify.
				Ipp_Features_Supported[0] = PAGE_OVERRIDES;
				break;

			default:
				break;
		}
	}

	return;
}
/*================================================================================================
［機能］	設定関数 isMediaEmptyError.
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     ZeusSZX0 CQ526087 2016/03/10 HCL-Sankar ADDED.
================================================================================================*/
Bool IPPC_LibOpeGetPrintAttr::isMediaEmptyError( TYPE_FeedTray e_FeedTray )
{
	Bool bl_Rtn = False;
	APIC_PrinterWarning* pc_PrinterWarning = APIC_PrinterWarning::newInstance();

	APIC_ActiveJobList*	pc_ActiveJobList = APIC_ActiveJobList::newInstance();

	pc_ActiveJobList->lock();
	APIC_Job* pc_CurrentPrintJob = pc_ActiveJobList->getCurrentPrintJob();	// 実行中プリントジョブのポインタ.
	pc_ActiveJobList->unlock();

	// 実行中ジョブがある場合.
	if( NULL != pc_CurrentPrintJob ) {
		pc_CurrentPrintJob->lock();
		if( True == pc_CurrentPrintJob->isWarning(ERROR_PaperEmpty)				// ペーパーエンプティ(プリント開始後).
		 || True == pc_CurrentPrintJob->isWarning(ERROR_PrintPaperError) ) {	// プリント用紙不適合.
			// 実行中プリントジョブの給紙トレイ.
			if( pc_CurrentPrintJob->getFeedTrayOfCurrectPrintJob() == e_FeedTray ) {
					bl_Rtn = True;
			}
		}
		pc_CurrentPrintJob->unlock();
	}

	pc_PrinterWarning->lock();

	// 用紙サイズ不一致.
	if( ( True == pc_PrinterWarning->isWarning( ERROR_PaperSizeChanged ) ) 
	 || ( True == pc_PrinterWarning->isWarning( ERROR_PaperSizeChangedManual ) ) ){	
		bl_Rtn = True;
	}

	// 用紙種類不一致.
	if( True == pc_PrinterWarning->isWarning( ERROR_PaperKindChanged ) ) {
		switch( pc_PrinterWarning->getMediaError() ) {
			case TYPE_SizeMediaErrorTray1:
				if( TYPE_FeedTray1 == e_FeedTray ) {
					bl_Rtn = True;
				}
				break;
			case TYPE_SizeMediaErrorTray2:
				if( TYPE_FeedTray2 == e_FeedTray ) {
					bl_Rtn = True;
				}
				break;
			case TYPE_SizeMediaErrorTray3:
				if( TYPE_FeedTray3 == e_FeedTray ) {
					bl_Rtn = True;
				}
				break;
			case TYPE_SizeMediaErrorTray4:
				if( TYPE_FeedTray4 == e_FeedTray ) {
					bl_Rtn = True;
				}
				break;
			case TYPE_SizeMediaErrorTray5:
				if( TYPE_FeedTray5 == e_FeedTray ) {
					bl_Rtn = True;
				}
				break;
			case TYPE_SizeMediaErrorTrayManual:
				if( TYPE_FeedTrayManual == e_FeedTray ) {
					bl_Rtn = True;
				}
				break;
			default:
				break;
		}
	}
	pc_PrinterWarning->unlock();
	
	return bl_Rtn;
}
/*================================================================================================
［機能］	IPPC_LibDebugクラスのインスタンス取得
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2014/01/31 新規作成 CQ384241 ZeusCD UV-01249 AppleAirPrint.
================================================================================================*/
IPPC_LibDebug* IPPC_LibDebug::newInstance()
{
	static IPPC_LibDebug c_LibDebug;
	return	&c_LibDebug;
}


/*================================================================================================
［機能］	Debugの有効無効を変更する.
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2014/01/31 新規作成 CQ384241 ZeusCD UV-01249 AppleAirPrint.
================================================================================================*/
// ターミナルコール用.
extern "C"{
	void IPP_chageDis( Bool bl_DebugOn );
}

void IPP_chageDis( Bool bl_DebugOn )
{
	pc_LibDebug->changeDbg( bl_DebugOn );
}

void IPPC_LibDebug::changeDbg( Bool bl_DebugOnTemp )
{
	SYSF_logMsg("[IPP] bl_DebugOn %d -> ",bl_DebugOn,0,0,0,0,0);

	bl_DebugOn = bl_DebugOnTemp;

	SYSF_logMsg("%d\n",bl_DebugOn,0,0,0,0,0);

	return;
}

/*================================================================================================
［機能］	IPP_Media_Colの設定内容を表示する.
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2014/01/31 新規作成 CQ384241 ZeusCD UV-01249 AppleAirPrint.
================================================================================================*/
void IPPC_LibDebug::DispMedaiCol( IPP_Media_Col* pc_MediaCol )
{
	SYSF_logMsg("[IPP]  Media_Col_Botton_Margin = %d\n",pc_MediaCol->Media_Col_Botton_Margin,0,0,0,0,0);
	SYSF_logMsg("[IPP]  Media_Col_Top_Margin = %d\n"   ,pc_MediaCol->Media_Col_Top_Margin,0,0,0,0,0);
	SYSF_logMsg("[IPP]  Media_Col_Right_Margin = %d\n" ,pc_MediaCol->Media_Col_Right_Margin,0,0,0,0,0);
	SYSF_logMsg("[IPP]  Media_Col_Left_Margin = %d\n"  ,pc_MediaCol->Media_Col_Left_Margin,0,0,0,0,0);
	SYSF_logMsg("[IPP]  Media_Col_Source = %s\n"  	   ,pc_MediaCol->Media_Col_Source,0,0,0,0,0);
	SYSF_logMsg("[IPP]  Media_Col_Type = %s\n"  	   ,pc_MediaCol->Media_Col_Type,0,0,0,0,0);

	for( int i=0; i < MAX_IPP_MEDIA_COL_MEDIA_SIZE; i++ ) {
		SYSF_logMsg("[IPP]  Media_Size[%d].set = %d\n"  	      ,i, pc_MediaCol->Media_Size[0].set,0,0,0,0);
		// ZeusCD CQ386774 OPS1八代 2014/02/25 START.
		SYSF_logMsg("[IPP]  Media_Size[%d].X_Dimension[0] = %d\n" ,i, pc_MediaCol->Media_Size[0].X_Dimension[0],0,0,0,0);
		SYSF_logMsg("[IPP]  Media_Size[%d].Y_Dimension[0] = %d\n" ,i, pc_MediaCol->Media_Size[0].Y_Dimension[0],0,0,0,0);
		SYSF_logMsg("[IPP]  Media_Size[%d].X_Dimension[1] = %d\n" ,i, pc_MediaCol->Media_Size[0].X_Dimension[1],0,0,0,0);
		SYSF_logMsg("[IPP]  Media_Size[%d].Y_Dimension[1] = %d\n" ,i, pc_MediaCol->Media_Size[0].Y_Dimension[1],0,0,0,0);
		// ZeusCD CQ386774 OPS1八代 2014/02/25 END.
	}

	SYSF_logMsg("[IPP]  Media_Source_Properties.set = %d\n"							  ,pc_MediaCol->Media_Source_Properties.set,0,0,0,0,0);
	SYSF_logMsg("[IPP]  Media_Source_Properties.Media_Source_Feed_Direction = %d\n"	  ,pc_MediaCol->Media_Source_Properties.Media_Source_Feed_Direction,0,0,0,0,0);
	SYSF_logMsg("[IPP]  Media_Source_Properties.Media_Source_Feed_Orientation = %d\n" ,pc_MediaCol->Media_Source_Properties.Media_Source_Feed_Orientation,0,0,0,0,0);
	SYSF_logMsg("[IPP]  Media_Col_Auto_Dimension = %d\n" ,pc_MediaCol->Media_Col_Auto_Dimension,0,0,0,0,0);			//IT5_4.1 Airprint1.7対応 HCL_VIJAY 2017/1/30 Added.
	return;
}
/*================================================================================================
［機能］	IPP_Finishings_Colの設定内容を表示する.
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
[履歴]  Helios KRQ-93 AirPrint1.5対応 HCL-Sankar 2015/02/03 
================================================================================================*/
void IPPC_LibDebug::DispFinshingsCol( IPP_Finishings_Col* pc_FinishingsCol )
{
	SYSF_logMsg("[IPP]  set = %d\n"						,pc_FinishingsCol->set,0,0,0,0,0);
	SYSF_logMsg("[IPP]  FinishingsTemplate = %s\n"		,pc_FinishingsCol->FinishingsTemplate,0,0,0,0,0);
	SYSF_logMsg("[IPP]  type = %d\n" 					,pc_FinishingsCol->type,0,0,0,0,0);
	
	return;
}

/*================================================================================================
［機能］	IPP_JobTempAttrの設定内容を表示する.
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2014/01/31 新規作成 CQ384241 ZeusCD UV-01249 AppleAirPrint.
================================================================================================*/
void IPPC_LibDebug::DispJobTempAttr( char* pc_Caller, IPP_JobTempAttr* ps_JobTempAttr )
{
	if( False == bl_DebugOn ) return;

	int i = 0;

	SYSF_logMsg("[IPP]DispJobTempAttr START : %s \n",pc_Caller,0,0,0,0,0);

	SYSF_logMsg("[IPP] Job_Priority = %d\n"  ,ps_JobTempAttr->Job_Priority,0,0,0,0,0);
	SYSF_logMsg("[IPP] Job_Hold_Until = %d\n",ps_JobTempAttr->Job_Hold_Until,0,0,0,0,0);
	SYSF_logMsg("[IPP] Job_Sheets = %d\n"    ,ps_JobTempAttr->Job_Sheets,0,0,0,0,0);
	SYSF_logMsg("[IPP] Multi_Doc_Hand = %d\n",ps_JobTempAttr->Multi_Doc_Hand,0,0,0,0,0);
	SYSF_logMsg("[IPP] Copies = %d\n"		,ps_JobTempAttr->Copies,0,0,0,0,0);

	for( i=0; i < MAX_IPP_FINISHINGS; i++ ) {
		SYSF_logMsg("[IPP] Finishings[%d] = %d\n",i,ps_JobTempAttr->Finishings[i],0,0,0,0);
	}

	SYSF_logMsg("[IPP] PageRange = %s\n"		,ps_JobTempAttr->PageRange,0,0,0,0,0);
	SYSF_logMsg("[IPP] Sides = %d\n"			,ps_JobTempAttr->Sides,0,0,0,0,0);
	SYSF_logMsg("[IPP] Number_Up = %d\n"		,ps_JobTempAttr->Number_Up,0,0,0,0,0);
	SYSF_logMsg("[IPP] Orientation = %d\n"	,ps_JobTempAttr->Orientation,0,0,0,0,0);
	SYSF_logMsg("[IPP] PaperTrayMedia = %s\n",ps_JobTempAttr->PaperTrayMedia,0,0,0,0,0);
	SYSF_logMsg("[IPP] Resolution = %d\n"	,ps_JobTempAttr->Resolution,0,0,0,0,0);
	SYSF_logMsg("[IPP] Quality = %d\n"		,ps_JobTempAttr->Quality,0,0,0,0,0);
	SYSF_logMsg("[IPP] Output_Bin = %d\n"	,ps_JobTempAttr->Output_Bin,0,0,0,0,0);
	SYSF_logMsg("[IPP] Feed_Orientation = %d\n"	,ps_JobTempAttr->Feed_Orientation,0,0,0,0,0);
	SYSF_logMsg("[IPP] Print_Color_Mode = %d\n"	,ps_JobTempAttr->Print_Color_Mode,0,0,0,0,0);
	SYSF_logMsg("[IPP] Output_Mode = %d\n"	,ps_JobTempAttr->Output_Mode,0,0,0,0,0);
	SYSF_logMsg("[IPP] Print_Content_Optimize = %d\n",ps_JobTempAttr->Print_Content_Optimize,0,0,0,0,0);
	SYSF_logMsg("[IPP] Print_Rendering_Intent = %d\n",ps_JobTempAttr->Print_Rendering_Intent,0,0,0,0,0);
	SYSF_logMsg("[IPP] Job_Account_Id = %s\n",ps_JobTempAttr->Job_Account_Id,0,0,0,0,0);
	SYSF_logMsg("[IPP] Job_Accounting_User_Id = %s\n",ps_JobTempAttr->Job_Accounting_User_Id,0,0,0,0,0);

	// 非サポート.
	for( i=0; i < MAX_IPP_MANDATORY_ATTRIBUTES; i++ ) {
		SYSF_logMsg("[IPP] Job_Mandatory_Attributes[%d] = %d\n",i,ps_JobTempAttr->Job_Mandatory_Attributes[i],0,0,0,0);
	}
	
	// Helios KRQ-93 AirPrint1.5対応 HCL-Vijay 2015/02/04 Added START
	for( i=0; i < MAX_IPP_OVERRIDES; i++ ) {
		SYSF_logMsg("[IPP] Overides = %d\n",ps_JobTempAttr->Overrides[i].set,0,0,0,0,0);
		SYSF_logMsg("[IPP] Overrides_Pages = %s\n",ps_JobTempAttr->Overrides[i].Pages,0,0,0,0,0);
		SYSF_logMsg("[IPP] Overrides_Sides = %d\n",ps_JobTempAttr->Overrides[i].Sides,0,0,0,0,0);
		SYSF_logMsg("[IPP] Overrides_Media = %s\n",ps_JobTempAttr->Overrides[i].Media,0,0,0,0,0);
		SYSF_logMsg("[IPP] Overrides_Media_Col_Source = %s\n",ps_JobTempAttr->Overrides[i].Media_Col.Media_Col_Source,0,0,0,0,0);
		SYSF_logMsg("[IPP] Overrides_Media_Col_Type = %s\n",ps_JobTempAttr->Overrides[i].Media_Col.Media_Col_Type,0,0,0,0,0);
		SYSF_logMsg("[IPP] Overrides_Media_Source_Properties = %d\n",ps_JobTempAttr->Overrides[i].Media_Col.Media_Source_Properties.set,0,0,0,0,0);
		SYSF_logMsg("[IPP] Overrides_Media_Source_Feed_Direction = %d\n",ps_JobTempAttr->Overrides[i].Media_Col.Media_Source_Properties.Media_Source_Feed_Direction,0,0,0,0,0);
		SYSF_logMsg("[IPP] Overrides_Media_Source_Feed_Orientation = %d\n",ps_JobTempAttr->Overrides[i].Media_Col.Media_Source_Properties.Media_Source_Feed_Orientation,0,0,0,0,0);
		SYSF_logMsg("[IPP] Overrides_Media_Size_X1 = %d\n",ps_JobTempAttr->Overrides[i].Media_Col.Media_Size[0].X_Dimension[0],0,0,0,0,0);
		SYSF_logMsg("[IPP] Overrides_Media_Size_Y1 = %d\n",ps_JobTempAttr->Overrides[i].Media_Col.Media_Size[0].Y_Dimension[0],0,0,0,0,0);
		SYSF_logMsg("[IPP] Overrides_Media_Size_X2 = %d\n",ps_JobTempAttr->Overrides[i].Media_Col.Media_Size[0].X_Dimension[1],0,0,0,0,0);
		SYSF_logMsg("[IPP] Overrides_Media_Size_Y2 = %d\n",ps_JobTempAttr->Overrides[i].Media_Col.Media_Size[0].Y_Dimension[1],0,0,0,0,0);
		SYSF_logMsg("[IPP] Overrides_Media_Col_Botton_Margin = %d\n",ps_JobTempAttr->Overrides[i].Media_Col.Media_Col_Botton_Margin,0,0,0,0,0);
		SYSF_logMsg("[IPP] Overrides_Media_Col_Top_Margin = %d\n",ps_JobTempAttr->Overrides[i].Media_Col.Media_Col_Top_Margin,0,0,0,0,0);		
		SYSF_logMsg("[IPP] Overrides_Media_Col_Right_Margin = %d\n",ps_JobTempAttr->Overrides[i].Media_Col.Media_Col_Right_Margin,0,0,0,0,0);
		SYSF_logMsg("[IPP] Overrides_Media_Col.Media_Col_Left_Margin = %d\n",ps_JobTempAttr->Overrides[i].Media_Col.Media_Col_Left_Margin,0,0,0,0,0);
		SYSF_logMsg("[IPP] Overrides_Finishings = %d\n",ps_JobTempAttr->Overrides[i].Finishings[0],0,0,0,0,0);
		SYSF_logMsg("[IPP] Overrides_Job_Sheets = %d\n",ps_JobTempAttr->Overrides[i].Job_Sheets,0,0,0,0,0);
		SYSF_logMsg("[IPP] Overrides_Multi_Doc_Hand = %d\n",ps_JobTempAttr->Overrides[i].Multi_Doc_Hand,0,0,0,0,0);
		SYSF_logMsg("[IPP] Overrides_Number_Up = %d\n",ps_JobTempAttr->Overrides[i].Number_Up,0,0,0,0,0);
		SYSF_logMsg("[IPP] Overrides_Orientation = %d\n",ps_JobTempAttr->Overrides[i].Orientation,0,0,0,0,0);
		SYSF_logMsg("[IPP] Overrides_Output_Bin = %d\n",ps_JobTempAttr->Overrides[i].Output_Bin,0,0,0,0,0);
		SYSF_logMsg("[IPP] Overrides_Quality = %d\n",ps_JobTempAttr->Overrides[i].Quality,0,0,0,0,0);
		SYSF_logMsg("[IPP] Overrides_Resolution = %d\n",ps_JobTempAttr->Overrides[i].Resolution,0,0,0,0,0);
	}
	// Helios KRQ-93 AirPrint1.5対応 HCL-Vijay 2015/02/04 Added END
	
	SYSF_logMsg("[IPP] Media = %s\n",ps_JobTempAttr->Media,0,0,0,0,0);

	SYSF_logMsg("[IPP] Media_Col \n",0,0,0,0,0,0);
	DispMedaiCol( &ps_JobTempAttr->Media_Col );

	SYSF_logMsg("[IPP] Media_Col_Media_Size = %s\n",ps_JobTempAttr->Media_Col_Media_Size,0,0,0,0,0);
	SYSF_logMsg("[IPP] Print_Scaling = %d\n",ps_JobTempAttr->Print_Scaling,0,0,0,0,0);
	
	// UV1429 AppleAirPrint 1.4 Support HCL-VIJAY 2014/5/16 START.	
	SYSF_logMsg("[IPP] Confirmation_Sheet_Print = %d\n"		,ps_JobTempAttr->Confirmation_Sheet_Print,0,0,0,0,0);
	SYSF_logMsg("[IPP] Cover_Sheet_Info \n",0,0,0,0,0,0);
	SYSF_logMsg("[IPP] From_Name  = %s\n",ps_JobTempAttr->Cover_Sheet_Info.From_Name,0,0,0,0,0);
	SYSF_logMsg("[IPP] Subject  = %s\n",ps_JobTempAttr->Cover_Sheet_Info.Subject,0,0,0,0,0);
	SYSF_logMsg("[IPP] To_Name  = %s\n",ps_JobTempAttr->Cover_Sheet_Info.To_Name,0,0,0,0,0);
	SYSF_logMsg("[IPP] Destination_Uris \n",0,0,0,0,0,0);
	for( i=0; i < MAX_DESTINATION_URIS ; i++ ) {
		SYSF_logMsg("[IPP] Destination_Uri Set[%d] = %d\n"		,i,ps_JobTempAttr->Destination_Uris[i].set);
		SYSF_logMsg("[IPP] Destination_Uri[%d] = %s\n"		,i,ps_JobTempAttr->Destination_Uris[i].Destination_Uri);
		SYSF_logMsg("[IPP] Pre_Dial_String[%d] = %s\n"		,i,ps_JobTempAttr->Destination_Uris[i].Pre_Dial_String);
	}
	SYSF_logMsg("[IPP] Number_Of_Retries = %d\n"		,ps_JobTempAttr->Number_Of_Retries,0,0,0,0,0);
	SYSF_logMsg("[IPP] Retry_Interval = %d\n"		,ps_JobTempAttr->Retry_Interval,0,0,0,0,0);
	SYSF_logMsg("[IPP] Retry_Time_Out = %d\n"		,ps_JobTempAttr->Retry_Time_Out,0,0,0,0,0);
	// UV1429 AppleAirPrint 1.4 Support HCL-VIJAY 2014/5/16 END.
	// Helios KRQ-93 AirPrint1.5対応 HCL-Sankar 2015/02/02 ADDED START
	for( i=0; i < MAX_IPP_FIN_COL; i++ ) {
		SYSF_logMsg("[IPP] Finishings_Col[%d] \n",i,0,0,0,0,0);
		DispFinshingsCol( &ps_JobTempAttr->Finishings_Col[i] );
	}
	// Helios KRQ-93 AirPrint1.5対応 HCL-Sankar 2015/02/02 ADDED END
	SYSF_logMsg("[IPP]DispJobTempAttr END : %s \n",pc_Caller,0,0,0,0,0);

	return;
}

/*================================================================================================
［機能］	IPP_JobTempAttrSupportの設定内容を表示する.
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2014/01/31 新規作成 CQ384241 ZeusCD UV-01249 AppleAirPrint.
================================================================================================*/
void IPPC_LibDebug::DispJobTempAttrSupport( IPP_JobTempAttrSupport* ps_JobTempAttrSupport )
{
	if( False == bl_DebugOn ) return;

	int i = 0;

	SYSF_logMsg("[IPP]DispJobTempAttrSupport START\n",0,0,0,0,0,0);

	SYSF_logMsg("[IPP] Job_Priority = %d\n",ps_JobTempAttrSupport->Job_Priority,0,0,0,0,0);

	for( i=0; i < MAX_IPP_JOB_HOLD; i++ ) {
		SYSF_logMsg("[IPP] Job_Hold_Until[%d] = %d\n",i,ps_JobTempAttrSupport->Job_Hold_Until[i],0,0,0,0);
	}

	for( i=0; i < MAX_IPP_JOB_SHEETS; i++ ) {
		SYSF_logMsg("[IPP] Job_Sheets[%d] = %d\n",i,ps_JobTempAttrSupport->Job_Sheets[i],0,0,0,0);
	}

	for( i=0; i < MAX_IPP_MULTI_DOC_HAND; i++ ) {
		SYSF_logMsg("[IPP] Multi_Doc_Hand[%d] = %d\n",i,ps_JobTempAttrSupport->Multi_Doc_Hand[i],0,0,0,0);
	}

	SYSF_logMsg("[IPP] Copies = %d\n",ps_JobTempAttrSupport->Copies,0,0,0,0,0);

	for( i=0; i < MAX_IPP_FINISHINGS; i++ ) {
		SYSF_logMsg("[IPP] Finishings[%d] = %d\n",i,ps_JobTempAttrSupport->Finishings[i],0,0,0,0);
	}

	SYSF_logMsg("[IPP] PageRange = %d\n",ps_JobTempAttrSupport->PageRange,0,0,0,0,0);

	for( i=0; i < MAX_IPP_SIDES; i++ ) {
		SYSF_logMsg("[IPP] Sides[%d] = %d\n",i,ps_JobTempAttrSupport->Sides[i],0,0,0,0);
	}

	for( i=0; i < MAX_IPP_NUM_UP; i++ ) {
		SYSF_logMsg("[IPP] Number_Up[%d] = %d\n",i,ps_JobTempAttrSupport->Number_Up[i],0,0,0,0);
	}

	for( i=0; i < MAX_IPP_ORIENTATION; i++ ) {
		SYSF_logMsg("[IPP] Orientation[%d] = %d\n",i,ps_JobTempAttrSupport->Orientation[i],0,0,0,0);
	}

	for( i=0; i < MAX_IPP_PAPERTRAY_MEDIA; i++ ) {
		SYSF_logMsg("[IPP] PaperTrayMedia[%d] = %s\n",i,ps_JobTempAttrSupport->PaperTrayMedia[i],0,0,0,0);
	}

	for( i=0; i < MAX_IPP_RESOLUTION; i++ ) {
		SYSF_logMsg("[IPP] Resolution[%d] = %d\n",i,ps_JobTempAttrSupport->Resolution[i],0,0,0,0);
	}

	for( i=0; i < MAX_IPP_QUALITY; i++ ) {
		SYSF_logMsg("[IPP] Quality[%d] = %d\n",i,ps_JobTempAttrSupport->Quality[i],0,0,0,0);
	}

	for( i=0; i < MAX_IPP_OUTPUT_BIN; i++ ) {
		SYSF_logMsg("[IPP] Output_Bin[%d] = %d\n",i,ps_JobTempAttrSupport->Output_Bin[i],0,0,0,0);
	}

	// 非サポート.
	for( i=0; i < MAX_IPP_FEED_ORIENTATION; i++ ) {
		SYSF_logMsg("[IPP] Feed_Orientation[%d] = %d\n",i,ps_JobTempAttrSupport->Feed_Orientation[i],0,0,0,0);
	}

	for( i=0; i < MAX_IPP_PRINT_COLOR_MODE; i++ ) {
		SYSF_logMsg("[IPP] Print_Color_Mode[%d] = %d\n",i,ps_JobTempAttrSupport->Print_Color_Mode[i],0,0,0,0);
	}
	
	for( i=0; i < MAX_IPP_OUTPUT_MODE; i++ ) {
		SYSF_logMsg("[IPP] Output_Mode[%d] = %d\n",i,ps_JobTempAttrSupport->Output_Mode[i],0,0,0,0);
	}

	for( i=0; i < MAX_IPP_PRINT_CONTENT_OPTIMIZE; i++ ) {
		SYSF_logMsg("[IPP] Print_Content_Optimize[%d] = %d\n",i,ps_JobTempAttrSupport->Print_Content_Optimize[i],0,0,0,0);
	}

	// 非サポート.
	for( i=0; i < MAX_IPP_PRINT_RENDERING_INTENT; i++ ) {
		SYSF_logMsg("[IPP] Print_Rendering_Intent[%d] = %d\n",i,ps_JobTempAttrSupport->Print_Rendering_Intent[i],0,0,0,0);
	}

	// 非サポート.
	for( i=0; i < IPP_OVERRIDES_SUPPORTED_END; i++ ) {
		SYSF_logMsg("[IPP] Overrides_Supported[%d] = %d\n",i,ps_JobTempAttrSupport->Overrides_Supported[i],0,0,0,0);
	}

	// 非サポート.
	for( i=0; i < MAX_IPP_JOB_ACCOUNT_IDS; i++ ) {
		SYSF_logMsg("[IPP] Job_Account_Id[%d] = %s\n",i,ps_JobTempAttrSupport->Job_Account_Id[i],0,0,0,0);
	}

	// 非サポート.
	for( i=0; i < MAX_IPP_ACCOUNTING_USER_IDS; i++ ) {
		SYSF_logMsg("[IPP] Job_Accounting_User_Id[%d] = %s\n",i,ps_JobTempAttrSupport->Job_Accounting_User_Id[i],0,0,0,0);
	}

	for( i=0; i < MAX_IPP_PRINT_MEDIA_COL_SUPPORTED; i++ ) {
		SYSF_logMsg("[IPP] Media_Col_Supported[%d] = %d\n",i,ps_JobTempAttrSupport->Media_Col_Supported[i],0,0,0,0);
	}

	for( i=0; i < MAX_MEDIA_READY; i++ ) {
		SYSF_logMsg("[IPP] Media_Ready[%d] = %s\n",i,ps_JobTempAttrSupport->Media_Ready[i],0,0,0,0);
	}

	SYSF_logMsg("[IPP] Media_Col_Default \n",0,0,0,0,0,0);
	DispMedaiCol( &ps_JobTempAttrSupport->Media_Col_Default );

	for( i=0; i < MAX_IPP_MEDIA_COL_READY; i++ ) {
		SYSF_logMsg("[IPP] Media_Col_Ready[%d] \n",i,0,0,0,0,0);
		DispMedaiCol( &ps_JobTempAttrSupport->Media_Col_Ready[i] );
	}

	for( i=0; i < MAX_IPP_PRINTER_SCALING; i++ ) {
		SYSF_logMsg("[IPP] Print_Scaling_Support[%d] = %d\n",i,ps_JobTempAttrSupport->Print_Scaling_Support[i],0,0,0,0);
	}
	
	// UV1429 AppleAirPrint 1.4 Support HCL-VIJAY 2014/6/25 START.
	
	for( i=0; i < MAX_IPP_SIZERANGE; i++ ) {
		SYSF_logMsg("[IPP] Number_Of_Retries_Supported[%d] = %d\n",i,ps_JobTempAttrSupport->Number_Of_Retries_Supported[i],0,0,0,0);
	}
	
	for( i=0; i < MAX_IPP_SIZERANGE; i++ ) {
		SYSF_logMsg("[IPP] Retry_Interval_Supported[%d] = %d\n",i,ps_JobTempAttrSupport->Retry_Interval_Supported[i],0,0,0,0);
	}
	
	for( i=0; i < MAX_IPP_SIZERANGE; i++ ) {
		SYSF_logMsg("[IPP] Retry_Time_Out_Supported[%d] = %d\n",i,ps_JobTempAttrSupport->Retry_Time_Out_Supported[i],0,0,0,0);
	}
	
	for( i=0; i < MAX_IPP_COVER_SHEET_INFO_SUPPORTED; i++ ) {
		SYSF_logMsg("[IPP] Cover_Sheet_Info_Supported[%d] = %d\n",i,ps_JobTempAttrSupport->Cover_Sheet_Info_Supported[i],0,0,0,0);
	}
	
	for( i=0; i < MAX_IPP_DESTINATION_URI_SCHEMES_SUPPORTED; i++ ) {
		SYSF_logMsg("[IPP] DESTINATION_URI_SCHEMES_SUPPORTED[%d] = %d\n",i,ps_JobTempAttrSupport->Destination_Uri_Schemes_Supported[i],0,0,0,0);
	}
	
	for( i=0; i < MAX_IPP_DESTINATION_URIS_SUPPORTED; i++ ) {
		SYSF_logMsg("[IPP] DESTINATION_URIS_SUPPORTED[%d] = %d\n",i,ps_JobTempAttrSupport->Destination_Uris_Supported[i],0,0,0,0);
	}
	
	SYSF_logMsg("[IPP] Multiple_Destination_Uris_Supported = %d\n"		,ps_JobTempAttrSupport->Multiple_Destination_Uris_Supported,0,0,0,0,0);
	
	// UV1429 AppleAirPrint 1.4 Support HCL-VIJAY 2014/6/25 END.
	// Helios KRQ-93 AirPrint1.5対応 HCL-Sankar 2015/02/03 ADDED START
	for( i=0; i < IPP_FIN_COL_END; i++ ) {
		SYSF_logMsg("[IPP] FinishingsColSupported[%d] = %d\n",i,ps_JobTempAttrSupport->FinishingsColSupported[i],0,0,0,0);
	}
	SYSF_logMsg("[IPP] JobPagePerSetSupported = %d\n"		,ps_JobTempAttrSupport->JobPagePerSetSupported,0,0,0,0,0);

	//Helios s CQ-513459 HCL-Sankar 2015/05/08 Added Start
	for( i=0; i < FIN_END; i++ ) {
		SYSF_logMsg("[IPP] FinishingsReady[%d] = %d\n",i,ps_JobTempAttrSupport->FinishingsReady[i],0,0,0,0);
	}
	//Helios s CQ-513459 HCL-Sankar 2015/05/08 Added End
	// Helios KRQ-93 AirPrint1.5対応 HCL-Sankar 2015/02/03 ADDED END
	SYSF_logMsg("[IPP]DispJobTempAttrSupport END\n",0,0,0,0,0,0);

	return;
}

/*================================================================================================
［機能］	IPP_JobDescAttrの設定内容を表示する.
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2014/01/31 新規作成 CQ384241 ZeusCD UV-01249 AppleAirPrint.
================================================================================================*/
void IPPC_LibDebug::DispJobDescAttr( char* pc_Caller, IPP_JobDescAttr* ps_JobDescAttr )
{
	if( False == bl_DebugOn ) return;

	int i = 0;

	SYSF_logMsg("[IPP]DispJobDescAttr START : %s \n",pc_Caller,0,0,0,0,0);

	SYSF_logMsg("[IPP] Job_URI = %s\n"  ,ps_JobDescAttr->Job_URI,0,0,0,0,0);
	SYSF_logMsg("[IPP] Job_ID = %d\n"  ,ps_JobDescAttr->Job_ID,0,0,0,0,0);
	SYSF_logMsg("[IPP] Job_UUID = %s\n"  ,ps_JobDescAttr->Job_UUID,0,0,0,0,0);
	SYSF_logMsg("[IPP] Job_More_Info = %s\n"  ,ps_JobDescAttr->Job_More_Info,0,0,0,0,0);
	SYSF_logMsg("[IPP] JobName = %s\n"  ,ps_JobDescAttr->JobName,0,0,0,0,0);
	SYSF_logMsg("[IPP] UserName = %s\n"  ,ps_JobDescAttr->UserName,0,0,0,0,0);
	SYSF_logMsg("[IPP] job-originating-user-uri = %s\n"  ,ps_JobDescAttr->Job_Originating_User_Uri,0,0,0,0,0);
	SYSF_logMsg("[IPP] JobState = %d\n"  ,ps_JobDescAttr->JobState,0,0,0,0,0);

	for( i=0; i < MAX_IPP_JOB_STATE_REASONS; i++ ) {
		SYSF_logMsg("[IPP] JobStateReason[%d] = %d\n",i,ps_JobDescAttr->JobStateReason[i],0,0,0,0);
	}

	SYSF_logMsg("[IPP] Job_State_Message = %s\n"  ,ps_JobDescAttr->Job_State_Message,0,0,0,0,0);

	for( i=0; i < MAX_IPP_JOB_DETAILED_STATUS_MESSAGES; i++ ) {
		SYSF_logMsg("[IPP] Job_Detailed_Status_Messages[%d] = %s\n",i,ps_JobDescAttr->Job_Detailed_Status_Messages[i],0,0,0,0);
	}

	for( i=0; i < MAX_IPP_JOB_DOCUMENT_ACCESS_ERRORS; i++ ) {
		SYSF_logMsg("[IPP] Job_Document_Access_Errors[%d] = %s\n",i,ps_JobDescAttr->Job_Document_Access_Errors[i],0,0,0,0);
	}

	SYSF_logMsg("[IPP] Number_of_Documents = %d\n"  ,ps_JobDescAttr->Number_of_Documents,0,0,0,0,0);
	SYSF_logMsg("[IPP] Output_Device_Assigned = %s\n"  ,ps_JobDescAttr->Output_Device_Assigned,0,0,0,0,0);
	SYSF_logMsg("[IPP] TimeCreation = %d\n"  ,ps_JobDescAttr->TimeCreation,0,0,0,0,0);
	SYSF_logMsg("[IPP] TimeProcessing = %d\n"  ,ps_JobDescAttr->TimeProcessing,0,0,0,0,0);
	SYSF_logMsg("[IPP] TimeCompleted = %d\n"  ,ps_JobDescAttr->TimeCompleted,0,0,0,0,0);
	SYSF_logMsg("[IPP] Job_Printer_Up_Time = %d\n"  ,ps_JobDescAttr->Job_Printer_Up_Time,0,0,0,0,0);

	for( i=0; i < MAX_STRING16; i++ ) {
		SYSF_logMsg("[IPP] DateTimeCreation[%d] = %d\n",i,ps_JobDescAttr->DateTimeCreation[i],0,0,0,0);
	}

	for( i=0; i < MAX_STRING16; i++ ) {
		SYSF_logMsg("[IPP] DateTimeProcessing[%d] = %d\n",i,ps_JobDescAttr->DateTimeProcessing[i],0,0,0,0);
	}

	for( i=0; i < MAX_STRING16; i++ ) {
		SYSF_logMsg("[IPP] DateTimeCompleted[%d] = %d\n",i,ps_JobDescAttr->DateTimeCompleted[i],0,0,0,0);
	}

	SYSF_logMsg("[IPP] Number_of_Interveing_Job = %d\n"  ,ps_JobDescAttr->Number_of_Interveing_Job,0,0,0,0,0);
	SYSF_logMsg("[IPP] Job_Message_From_Operator = %s\n"  ,ps_JobDescAttr->Job_Message_From_Operator,0,0,0,0,0);

	SYSF_logMsg("[IPP] Job_k_octets = %d\n"  ,ps_JobDescAttr->Job_k_octets,0,0,0,0,0);
	SYSF_logMsg("[IPP] Job_impressions = %d\n"  ,ps_JobDescAttr->Job_impressions,0,0,0,0,0);
	SYSF_logMsg("[IPP] Job_media_sheets = %d\n"  ,ps_JobDescAttr->Job_media_sheets,0,0,0,0,0);
	SYSF_logMsg("[IPP] Job_k_octets_processed = %d\n"  ,ps_JobDescAttr->Job_k_octets_processed,0,0,0,0,0);
	SYSF_logMsg("[IPP] Job_impressions_completed = %d\n"  ,ps_JobDescAttr->Job_impressions_completed,0,0,0,0,0);
	SYSF_logMsg("[IPP] Job_media_sheets_completed = %d\n"  ,ps_JobDescAttr->Job_media_sheets_completed,0,0,0,0,0);
	SYSF_logMsg("[IPP] Attr_Charset = %d\n"  ,ps_JobDescAttr->Attr_Charset,0,0,0,0,0);
	SYSF_logMsg("[IPP] Attr_Lang = %d\n"  ,ps_JobDescAttr->Attr_Lang,0,0,0,0,0);
	
	// UV1429 AppleAirPrint 1.4 Support HCL-VIJAY 2014/5/15 START.
	SYSF_logMsg("[IPP] Compression_Supplied = %d\n"  ,ps_JobDescAttr->Compression_Supplied,0,0,0,0,0);
	SYSF_logMsg("[IPP] Destination_Statuses \n",0,0,0,0,0,0);
	for( i=0; i < MAX_DESTINATION_URIS ; i++ ) {
		SYSF_logMsg("[IPP] Destination_statuses Set[%d] = %d\n",i,ps_JobDescAttr->Destination_Statuses[i].set);
		SYSF_logMsg("[IPP] Destination_Uri = %s\n"  ,ps_JobDescAttr->Destination_Statuses[i].Destination_Uri,0,0,0,0,0);
		SYSF_logMsg("[IPP] Images_Completed = %d\n"  ,ps_JobDescAttr->Destination_Statuses[i].Images_Completed,0,0,0,0,0);
		SYSF_logMsg("[IPP] Transmission_Status = %d\n"  ,ps_JobDescAttr->Destination_Statuses[i].Transmission_Status,0,0,0,0,0);
	}
	SYSF_logMsg("[IPP] Document_Format_Supplied = %d\n"  ,ps_JobDescAttr->Document_Format_Supplied,0,0,0,0,0);
	SYSF_logMsg("[IPP] Document_Name_Supplied = %s\n"  ,ps_JobDescAttr->Document_Name_Supplied,0,0,0,0,0);		
	// UV1429 AppleAirPrint 1.4 Support HCL-VIJAY 2014/5/15 END.
	
	SYSF_logMsg("[IPP]DispJobDescAttr END : %s \n",pc_Caller,0,0,0,0,0);

	return;
}

/*================================================================================================
［機能］	IPP_PrintDescAttrの設定内容を表示する.
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	
--------------------------------------------------------------------------------------------------
 [履歴]     2014/01/31 新規作成 CQ384241 ZeusCD UV-01249 AppleAirPrint.
================================================================================================*/
void IPPC_LibDebug::DispPrintDescAttr( IPP_PrintDescAttr* ps_PrintDescAttr )
{
	if( False == bl_DebugOn ) return;

	int i = 0;

	SYSF_logMsg("[IPP]DispPrintDescAttr START\n",0,0,0,0,0,0);

	SYSF_logMsg("[IPP] Printer_Name = %s\n"				,ps_PrintDescAttr->Printer_Name,0,0,0,0,0);
	SYSF_logMsg("[IPP] Printer_Location = %s\n"			,ps_PrintDescAttr->Printer_Location,0,0,0,0,0);
	SYSF_logMsg("[IPP] Printer_Info = %s\n"				,ps_PrintDescAttr->Printer_Info,0,0,0,0,0);
	SYSF_logMsg("[IPP] Printer_More_Info = %s\n"		,ps_PrintDescAttr->Printer_More_Info,0,0,0,0,0);
	SYSF_logMsg("[IPP] Printer_Driver_URI = %s\n"		,ps_PrintDescAttr->Printer_Driver_URI,0,0,0,0,0);
	SYSF_logMsg("[IPP] Printer_Make_Model = %s\n"		,ps_PrintDescAttr->Printer_Make_Model,0,0,0,0,0);
	SYSF_logMsg("[IPP] Printer_More_Info_Man = %s\n"	,ps_PrintDescAttr->Printer_More_Info_Man,0,0,0,0,0);
	SYSF_logMsg("[IPP] Printer_State = %d\n"			,ps_PrintDescAttr->Printer_State,0,0,0,0,0);

	for( i=0; i < MAX_IPP_PRINTER_STATE_REASONS; i++ ) {
		SYSF_logMsg("[IPP] State_Reason[%d] = %d\n",i,ps_PrintDescAttr->State_Reason[i],0,0,0,0);
	}

	SYSF_logMsg("[IPP] Printer_State_Message = %s\n"	,ps_PrintDescAttr->Printer_State_Message,0,0,0,0,0);

	// NICFUMが設定.
	for( i=0; i < MAX_IPP_VERSION; i++ ) {
		SYSF_logMsg("[IPP] Ipp_Versions_Supported[%d] = %d\n",i,ps_PrintDescAttr->Ipp_Versions_Supported[i],0,0,0,0);
	}

	SYSF_logMsg("[IPP] Multiple_Document_Jobs_Supported = %d\n"	,ps_PrintDescAttr->Multiple_Document_Jobs_Supported,0,0,0,0,0);

	SYSF_logMsg("[IPP] CharSet_Configured = %d\n"	,ps_PrintDescAttr->CharSet_Configured,0,0,0,0,0);

	for( i=0; i < MAX_IPP_ATTR_CHARSET; i++ ) {
		SYSF_logMsg("[IPP] CharSet_Supported[%d] = %d\n",i,ps_PrintDescAttr->CharSet_Supported[i],0,0,0,0);

	}

	SYSF_logMsg("[IPP] Lang_Configured = %d\n"	,ps_PrintDescAttr->Lang_Configured,0,0,0,0,0);

	for( i=0; i < MAX_IPP_LANG; i++ ) {
		SYSF_logMsg("[IPP] Lang_Supported[%d] = %d\n",i,ps_PrintDescAttr->Lang_Supported[i],0,0,0,0);

	}

	SYSF_logMsg("[IPP] Doc_Format_Default = %d\n"	,ps_PrintDescAttr->Doc_Format_Default,0,0,0,0,0);

	for( i=0; i < MAX_IPP_DOC_FORMAT; i++ ) {
		SYSF_logMsg("[IPP] Doc_Format_Support[%d] = %d\n",i,ps_PrintDescAttr->Doc_Format_Support[i],0,0,0,0);

	}

	SYSF_logMsg("[IPP] Queued_Job_Count = %d\n"					,ps_PrintDescAttr->Queued_Job_Count,0,0,0,0,0);
	SYSF_logMsg("[IPP] Printer_Message_From_Operator = %s\n"	,ps_PrintDescAttr->Printer_Message_From_Operator,0,0,0,0,0);
	SYSF_logMsg("[IPP] Color_Supported = %d\n"					,ps_PrintDescAttr->Color_Supported,0,0,0,0,0);

	for( i=0; i < MAX_IPP_URI_SCHEME; i++ ) {
		SYSF_logMsg("[IPP] Reference_Uri_Scheme[%d] = %d\n",i,ps_PrintDescAttr->Reference_Uri_Scheme[i],0,0,0,0);
	}

	SYSF_logMsg("[IPP] Pdl_Override = %d\n"		,ps_PrintDescAttr->Pdl_Override,0,0,0,0,0);
	SYSF_logMsg("[IPP] Printer_Up_Time = %d\n"	,ps_PrintDescAttr->Printer_Up_Time,0,0,0,0,0);

	for( i=0; i < MAX_STRING16; i++ ) {
		SYSF_logMsg("[IPP] Printer_Curr_Time[%d] = %d\n",i,ps_PrintDescAttr->Printer_Curr_Time[i],0,0,0,0);
	}

	for( i=0; i < MAX_IPP_COMPRESSION; i++ ) {
		SYSF_logMsg("[IPP] Compression_Supported[%d] = %d\n",i,ps_PrintDescAttr->Compression_Supported[i],0,0,0,0);
	}

	for( i=0; i < MAX_IPP_SIZERANGE; i++ ) {
		SYSF_logMsg("[IPP] Job_k_octets_supported[%d] = %d\n",i,ps_PrintDescAttr->Job_k_octets_supported[i],0,0,0,0);
	}

	for( i=0; i < MAX_IPP_SIZERANGE; i++ ) {
		SYSF_logMsg("[IPP] Job_impressions_supported[%d] = %d\n",i,ps_PrintDescAttr->Job_impressions_supported[i],0,0,0,0);
	}

	for( i=0; i < MAX_IPP_SIZERANGE; i++ ) {
		SYSF_logMsg("[IPP] Job_media_sheets_supported[%d] = %d\n",i,ps_PrintDescAttr->Job_media_sheets_supported[i],0,0,0,0);
	}

	SYSF_logMsg("[IPP] Pages_Per_Minute = %d\n"			,ps_PrintDescAttr->Pages_Per_Minute,0,0,0,0,0);
	SYSF_logMsg("[IPP] Pages_Per_Minute_Color = %d\n"	,ps_PrintDescAttr->Pages_Per_Minute_Color,0,0,0,0,0);

	for( i=0; i < MAX_IPP_IDENTIFY_DEFAULT_ACTIONS; i++ ) {
		SYSF_logMsg("[IPP] Identify_Actions_Default[%d] = %d\n",i,ps_PrintDescAttr->Identify_Actions_Default[i],0,0,0,0);
	}

	for( i=0; i < MAX_IPP_IDENTIFY_ACTIONS; i++ ) {
		SYSF_logMsg("[IPP] Identify_Actions_Supported[%d] = %d\n",i,ps_PrintDescAttr->Identify_Actions_Supported[i],0,0,0,0);
	}

	for( i=0; i < MAX_IPP_FEATURES_SUPPORTED; i++ ) {
		SYSF_logMsg("[IPP] Ipp_Features_Supported[%d] = %d\n",i,ps_PrintDescAttr->Ipp_Features_Supported[i],0,0,0,0);
	}

	SYSF_logMsg("[IPP] Job_Password_Supported = %d\n"	,ps_PrintDescAttr->Job_Password_Supported,0,0,0,0,0);

	for( i=0; i < MAX_IPP_JOB_PWS_ENCRYPT_SUPPORTED; i++ ) {
		SYSF_logMsg("[IPP] Job_Psw_Encryp_Supported[%d] = %d\n",i,ps_PrintDescAttr->Job_Psw_Encryp_Supported[i],0,0,0,0);
	}

	SYSF_logMsg("[IPP] Job_Ids_Supported = %d\n"					,ps_PrintDescAttr->Job_Ids_Supported,0,0,0,0,0);
	SYSF_logMsg("[IPP] Multiple_Operation_Timeout = %d\n"			,ps_PrintDescAttr->Multiple_Operation_Timeout,0,0,0,0,0);
	SYSF_logMsg("[IPP] Multiple_Operation_Timeout_Action = %d\n"	,ps_PrintDescAttr->Multiple_Operation_Timeout_Action,0,0,0,0,0);
	SYSF_logMsg("[IPP] Printer_Charge_Info = %s\n"					,ps_PrintDescAttr->Printer_Charge_Info,0,0,0,0,0);
	SYSF_logMsg("[IPP] Printer_Charge_Info_Uri = %s\n"				,ps_PrintDescAttr->Printer_Charge_Info_Uri,0,0,0,0,0);

	for( i=0; i < MAX_IPP_PRINT_KIND; i++ ) {
		SYSF_logMsg("[IPP] Printer_Kind[%d] = %d\n",i,ps_PrintDescAttr->Printer_Kind[i],0,0,0,0);
	}

	SYSF_logMsg("[IPP] Printer_Geo_Location = %s\n"			,ps_PrintDescAttr->Printer_Geo_Location,0,0,0,0,0);
	SYSF_logMsg("[IPP] Printer_Organization = %s\n"			,ps_PrintDescAttr->Printer_Organization,0,0,0,0,0);
	SYSF_logMsg("[IPP] Printer_Organizational_Unit = %s\n"	,ps_PrintDescAttr->Printer_Organizational_Unit,0,0,0,0,0);
	SYSF_logMsg("[IPP] Printer_Supply = %s\n"				,ps_PrintDescAttr->Printer_Supply,0,0,0,0,0);
	SYSF_logMsg("[IPP] Printer_Supply_Description = %s\n"	,ps_PrintDescAttr->Printer_Supply_Description,0,0,0,0,0);
	SYSF_logMsg("[IPP] Printer_Supply_Info_Uri = %s\n"		,ps_PrintDescAttr->Printer_Supply_Info_Uri,0,0,0,0,0);
	SYSF_logMsg("[IPP] Document_Password_Supported = %d\n"	,ps_PrintDescAttr->Document_Password_Supported,0,0,0,0,0);

	for( i=0; i < MAX_IPP_JOB_CREATION_ATTRS_SUPP; i++ ) {
		SYSF_logMsg("[IPP] Job_Creation_Attributes_Supported[%d] = %d\n",i,ps_PrintDescAttr->Job_Creation_Attributes_Supported[i],0,0,0,0);
	}

	for( i=0; i < MAX_IPP_JOB_CREATION_ATTRS_SUPP; i++ ) {
		SYSF_logMsg("[IPP] Job_Preferred_Attributes_Supported[%d] = %d\n",i,ps_PrintDescAttr->Job_Preferred_Attributes_Supported[i],0,0,0,0);
	}

	for( i=0; i < MAX_IPP_PRINTER_MANDATORY_ATTRS_SUPP; i++ ) {
		SYSF_logMsg("[IPP] Printer_Mandatory_Job_Attributes[%d] = %d\n",i,ps_PrintDescAttr->Printer_Mandatory_Job_Attributes[i],0,0,0,0);
	}

	for( i=0; i < MAX_IPP_PRINTER_ICONS_URIS; i++ ) {
		SYSF_logMsg("[IPP] Printer_Icons[%d] = %s\n",i,ps_PrintDescAttr->Printer_Icons[i],0,0,0,0);
	}

	for( i=0; i < MAX_IPP_PRINTER_ALERT; i++ ) {
		SYSF_logMsg("[IPP] Printer_Alert[%d] = %s\n",i,ps_PrintDescAttr->Printer_Alert[i],0,0,0,0,0);
		SYSF_logMsg("[IPP] Printer_Alert_Description[%d] = %s\n",i,ps_PrintDescAttr->Printer_Alert_Description[i],0,0,0,0,0);
	}

	for( i=0; i < MAX_IPP_PWD_RASTER_DOC_RES_SUPP; i++ ) {
		SYSF_logMsg("[IPP] Pwg_Raster_Document_Resolution_Supported[%d] = %d\n",i,ps_PrintDescAttr->Pwg_Raster_Document_Resolution_Supported[i],0,0,0,0);
	}

	for( i=0; i < MAX_IPP_PWD_RASTER_TYPE_SUPP; i++ ) {
		SYSF_logMsg("[IPP] Pwg_Raster_Doc_Type_Supported[%d] = %d\n",i,ps_PrintDescAttr->Pwg_Raster_Doc_Type_Supported[i],0,0,0,0);
	}

	SYSF_logMsg("[IPP] Pwg_Raster_Doc_Sheet_Back = %d\n"	,ps_PrintDescAttr->Pwg_Raster_Doc_Sheet_Back,0,0,0,0,0);

	for( i=0; i < WHICH_JOBS_END; i++ ) {																	//UV1429- AirPrint-1.4 Support-Modified maximum value check from MAX_IPP_WHICH_JOBS_SUPPORTED -> WHICH_JOBS_END
		SYSF_logMsg("[IPP] Which_Jobs_Supported[%d] = %d\n",i,ps_PrintDescAttr->Which_Jobs_Supported[i],0,0,0,0);
	}
	

	for( i=0; i < MAX_MEDIA_MARGINS_SUPPORTED; i++ ) {
		SYSF_logMsg("[IPP] Media_Bottom_Margin_Supported[%d] = %d\n",i,ps_PrintDescAttr->Media_Bottom_Margin_Supported[i],0,0,0,0);
	}

	for( i=0; i < MAX_MEDIA_MARGINS_SUPPORTED; i++ ) {
		SYSF_logMsg("[IPP] Media_Top_Margin_Supported[%d] = %d\n"	,i,ps_PrintDescAttr->Media_Top_Margin_Supported[i],0,0,0,0);
	}

	for( i=0; i < MAX_MEDIA_MARGINS_SUPPORTED; i++ ) {
		SYSF_logMsg("[IPP] Media_Left_Margin_Supported[%d] = %d\n"	,i,ps_PrintDescAttr->Media_Left_Margin_Supported[i],0,0,0,0);
	}

	for( i=0; i < MAX_MEDIA_MARGINS_SUPPORTED; i++ ) {
		SYSF_logMsg("[IPP] Media_Right_Margin_Supported[%d] = %d\n"	,i,ps_PrintDescAttr->Media_Right_Margin_Supported[i],0,0,0,0);
	}

	for( i=0; i < MAX_MEDIA_SOURCES_SUPPORTED; i++ ) {
		SYSF_logMsg("[IPP] Media_Source_Supported[%d] = %s\n",i,ps_PrintDescAttr->Media_Source_Supported[i],0,0,0,0);
	}

	for( i=0; i < MAX_MEDIA_TYPES_SUPPORTED; i++ ) {
		SYSF_logMsg("[IPP] Media_Type_Supported[%d] = %s\n",i,ps_PrintDescAttr->Media_Type_Supported[i],0,0,0,0);
	}

	for( i=0; i < MAX_IPP_MEDIA_COL_DATABASE; i++ ) {
		SYSF_logMsg("[IPP] Media_Col_Database[%d] \n",i,0,0,0,0,0);
		DispMedaiCol( &ps_PrintDescAttr->Media_Col_Database[i] );
	}
	// Helios KRQ-93 AirPrint1.5対応 HCL-Sankar 2015/02/03 ADDED START
	for( i=0; i < MAX_IPP_FIN_COL; i++ ) {
		SYSF_logMsg("[IPP] Finishings_Col_Database[%d] \n",i,0,0,0,0,0);
		DispFinshingsCol( &ps_PrintDescAttr->Finishings_Col_Database[i] );
	}
	
	for( i=0; i < MAX_IPP_FIN_COL; i++ ) {
		SYSF_logMsg("[IPP] Finishings_Col_Ready[%d] \n",i,0,0,0,0,0);
		DispFinshingsCol( &ps_PrintDescAttr->Finishings_Col_Ready[i] );
	}
	// Helios KRQ-93 AirPrint1.5対応 HCL-Sankar 2015/02/03 ADDED END

	//Helios s CQ-513459 HCL-Sankar 2015/05/08 Added Start
	for( i=0; i < MAX_IPP_FINISHIGS_TEMPLATE_SUPPORTED; i++ ) {
		SYSF_logMsg("[IPP] FinishingTemplateSupported[%d] = %s\n",i,ps_PrintDescAttr->FinishingTemplateSupported[i],0,0,0,0);
	}
	//Helios s CQ-513459 HCL-Sankar 2015/05/08 Added End
	
	for( i=0; i < MAX_IPP_SIZERANGE; i++ ) {
		SYSF_logMsg("[IPP] nJpeg_k_octets_supported[%d] = %d\n",i,ps_PrintDescAttr->nJpeg_k_octets_supported[i],0,0,0,0);
	}

	for( i=0; i < MAX_IPP_SIZERANGE; i++ ) {
		SYSF_logMsg("[IPP] nPdf_k_octets_supported[%d] = %d\n",i,ps_PrintDescAttr->nPdf_k_octets_supported[i],0,0,0,0);
	}

	for( i=0; i < MAX_IPP_JPEG_X_DIM_RANGE; i++ ) {
		SYSF_logMsg("[IPP] Jpeg_X_Dimention_Supported[%d] = %d\n",i,ps_PrintDescAttr->Jpeg_X_Dimention_Supported[i],0,0,0,0);
	}

	for( i=0; i < MAX_IPP_JPEG_X_DIM_RANGE; i++ ) {
		SYSF_logMsg("[IPP] Jpeg_Y_Dimention_Supported[%d] = %d\n",i,ps_PrintDescAttr->Jpeg_Y_Dimention_Supported[i],0,0,0,0);
	}

	SYSF_logMsg("[IPP] Landscape_Orientation_Requested_Prefered = %d\n"	,ps_PrintDescAttr->Landscape_Orientation_Requested_Prefered,0,0,0,0,0);


	for( i=0; i < MIO_IPP_MARKER_NUM; i++ ) {
		SYSF_logMsg("[IPP] Marker_Color[%d] = %s\n",i,ps_PrintDescAttr->Marker_Color[i],0,0,0,0);
	}

	for( i=0; i < MIO_IPP_MARKER_NUM; i++ ) {
		SYSF_logMsg("[IPP] Marker_High_Levels[%d] = %d\n",i,ps_PrintDescAttr->Marker_High_Levels[i],0,0,0,0);
	}

	for( i=0; i < MIO_IPP_MARKER_NUM; i++ ) {
		SYSF_logMsg("[IPP] Marker_Levels[%d] = %d\n",i,ps_PrintDescAttr->Marker_Levels[i],0,0,0,0);
	}

	for( i=0; i < MIO_IPP_MARKER_NUM; i++ ) {
		SYSF_logMsg("[IPP] Marker_Low_Levels[%d] = %d\n",i,ps_PrintDescAttr->Marker_Low_Levels[i],0,0,0,0);
	}

	for( i=0; i < MIO_IPP_MARKER_NUM; i++ ) {
		SYSF_logMsg("[IPP] Marker_Name[%d] = %s\n",i,ps_PrintDescAttr->Marker_Name[i],0,0,0,0);
	}

	for( i=0; i < MIO_IPP_MARKER_NUM; i++ ) {
		SYSF_logMsg("[IPP] Marker_Types[%d] = %d\n",i,ps_PrintDescAttr->Marker_Types[i],0,0,0,0);
	}

	SYSF_logMsg("[IPP] PDF_Version_Supportd = %s\n"	,ps_PrintDescAttr->PDF_Version_Supportd,0,0,0,0,0);

	for( i=0; i < MAX_IPP_URF_SUPPORTED; i++ ) {
		SYSF_logMsg("[IPP] URF_Supprted[%d] = %s\n",i,ps_PrintDescAttr->URF_Supprted[i],0,0,0,0);
	}

	for( i=0; i < MAX_IPP_MEDIA_COL_SIZE_SUPPORTED; i++ ) {
		SYSF_logMsg("[IPP] Media_Size_Supported[%d].set            = %d\n",i,ps_PrintDescAttr->Media_Size_Supported[i].set,0,0,0,0);
		SYSF_logMsg("[IPP] Media_Size_Supported[%d].X_Dimension[0] = %d\n",i,ps_PrintDescAttr->Media_Size_Supported[i].X_Dimension[0],0,0,0,0);
		SYSF_logMsg("[IPP] Media_Size_Supported[%d].Y_Dimension[0] = %d\n",i,ps_PrintDescAttr->Media_Size_Supported[i].Y_Dimension[0],0,0,0,0);
		SYSF_logMsg("[IPP] Media_Size_Supported[%d].X_Dimension[1] = %d\n",i,ps_PrintDescAttr->Media_Size_Supported[i].X_Dimension[1],0,0,0,0);
		SYSF_logMsg("[IPP] Media_Size_Supported[%d].Y_Dimension[1] = %d\n",i,ps_PrintDescAttr->Media_Size_Supported[i].Y_Dimension[1],0,0,0,0);
	}

	// ZeusCD CQ386774 OPS1八代 2014/02/25 START.
	for( i=0; i < MAX_IPP_PRINTER_FIRMWARE_INFO; i++ ) {
		SYSF_logMsg("[IPP] Printer_Firmware_Name[%d] = %s\n",i,ps_PrintDescAttr->Printer_Firmware_Name[i],0,0,0,0);
		SYSF_logMsg("[IPP] Printer_Firmware_Patches[%d] = %s\n",i,ps_PrintDescAttr->Printer_Firmware_Patches[i],0,0,0,0); // 2021/04/13【IT6_2.2】【RQ-890】 HIENG.
		SYSF_logMsg("[IPP] Printer_Firmware_String_Version[%d] = %s\n",i,ps_PrintDescAttr->Printer_Firmware_String_Version[i],0,0,0,0);
		SYSF_logMsg("[IPP] Printer_Firmware_Version[%d] = %s\n",i,ps_PrintDescAttr->Printer_Firmware_Version[i],0,0,0,0);
	}

	for( i=0; i < MAX_MEDIA_SOURCES_SUPPORTED; i++ ) {
		SYSF_logMsg("[IPP] Printer_Input_Tray[%d] = %s\n",i,ps_PrintDescAttr->Printer_Input_Tray[i],0,0,0,0);
	}

	for( i=0; i < MAX_IPP_OUTPUT_BIN; i++ ) {
		SYSF_logMsg("[IPP] Printer_Output_Tray[%d] = %s\n",i,ps_PrintDescAttr->Printer_Output_Tray[i],0,0,0,0);
	}
	// ZeusCD CQ386774 OPS1八代 2014/02/25 END.
	
	// UV1429 AppleAirPrint 1.4 Support HCL-VIJAY 2014/6/25 START.
	SYSF_logMsg("[IPP] Printer_Fax_Log_Uri = %s\n"		,ps_PrintDescAttr->Printer_Fax_Log_Uri,0,0,0,0,0);
	
	for( i=0; i < PRINTER_GET_ATTRIBUTES_SUPPORTED_END; i++ ) {
		SYSF_logMsg("[IPP] Printer_Get_Attributes_Supported[%d] = %d\n",i,ps_PrintDescAttr->Printer_Get_Attributes_Supported[i],0,0,0,0);
	}
	// UV1429 AppleAirPrint 1.4 Support HCL-VIJAY 2014/6/25 END	

	// HeliosL KRQ-93 AirPrint1.5(WP14) 2015.02.13 OPS1八代 START.
	SYSF_logMsg("[IPP]PrinterConfigChangeTime = %d\n", ps_PrintDescAttr->PrinterConfigChangeTime,0,0,0,0,0);
	SYSF_logMsg("[IPP] PrinterConfigChangeDateTime.year = %d\n", 			ps_PrintDescAttr->PrinterConfigChangeDateTime.year,0,0,0,0,0 );
	SYSF_logMsg("[IPP] PrinterConfigChangeDateTime.month = %d\n",			ps_PrintDescAttr->PrinterConfigChangeDateTime.month,0,0,0,0,0 );
	SYSF_logMsg("[IPP] PrinterConfigChangeDateTime.day = %d\n",				ps_PrintDescAttr->PrinterConfigChangeDateTime.day,0,0,0,0,0 );
	SYSF_logMsg("[IPP] PrinterConfigChangeDateTime.hour = %d\n",			ps_PrintDescAttr->PrinterConfigChangeDateTime.hour,0,0,0,0,0 );
	SYSF_logMsg("[IPP] PrinterConfigChangeDateTime.minutes = %d\n",			ps_PrintDescAttr->PrinterConfigChangeDateTime.minutes,0,0,0,0,0 );
	SYSF_logMsg("[IPP] PrinterConfigChangeDateTime.seconds = %d\n",			ps_PrintDescAttr->PrinterConfigChangeDateTime.seconds,0,0,0,0,0 );
	SYSF_logMsg("[IPP] PrinterConfigChangeDateTime.deci_seconds = %d\n",	ps_PrintDescAttr->PrinterConfigChangeDateTime.deci_seconds,0,0,0,0,0 );
	SYSF_logMsg("[IPP] PrinterConfigChangeDateTime.direction_plus = %d\n",	ps_PrintDescAttr->PrinterConfigChangeDateTime.direction_plus,0,0,0,0,0 );
	SYSF_logMsg("[IPP] PrinterConfigChangeDateTime.hour_from_utc = %d\n",	ps_PrintDescAttr->PrinterConfigChangeDateTime.hour_from_utc,0,0,0,0,0 );
	SYSF_logMsg("[IPP] PrinterConfigChangeDateTime.minutes_from_utc = %d\n",ps_PrintDescAttr->PrinterConfigChangeDateTime.minutes_from_utc,0,0,0,0,0 );

	SYSF_logMsg("[IPP]PrinterStateChangeTime = %d\n" , ps_PrintDescAttr->PrinterStateChangeTime,0,0,0,0,0);
	SYSF_logMsg("[IPP] PrinterStateChangeDateTime.year = %d\n", 			ps_PrintDescAttr->PrinterStateChangeDateTime.year,0,0,0,0,0 );
	SYSF_logMsg("[IPP] PrinterStateChangeDateTime.month = %d\n",			ps_PrintDescAttr->PrinterStateChangeDateTime.month,0,0,0,0,0 );
	SYSF_logMsg("[IPP] PrinterStateChangeDateTime.day = %d\n",				ps_PrintDescAttr->PrinterStateChangeDateTime.day,0,0,0,0,0 );
	SYSF_logMsg("[IPP] PrinterStateChangeDateTime.hour = %d\n",				ps_PrintDescAttr->PrinterStateChangeDateTime.hour,0,0,0,0,0 );
	SYSF_logMsg("[IPP] PrinterStateChangeDateTime.minutes = %d\n",			ps_PrintDescAttr->PrinterStateChangeDateTime.minutes,0,0,0,0,0 );
	SYSF_logMsg("[IPP] PrinterStateChangeDateTime.seconds = %d\n",			ps_PrintDescAttr->PrinterStateChangeDateTime.seconds,0,0,0,0,0 );
	SYSF_logMsg("[IPP] PrinterStateChangeDateTime.deci_seconds = %d\n",		ps_PrintDescAttr->PrinterStateChangeDateTime.deci_seconds,0,0,0,0,0 );
	SYSF_logMsg("[IPP] PrinterStateChangeDateTime.direction_plus = %d\n",	ps_PrintDescAttr->PrinterStateChangeDateTime.direction_plus,0,0,0,0,0 );
	SYSF_logMsg("[IPP] PrinterStateChangeDateTime.hour_from_utc = %d\n",	ps_PrintDescAttr->PrinterStateChangeDateTime.hour_from_utc,0,0,0,0,0 );
	SYSF_logMsg("[IPP] PrinterStateChangeDateTime.minutes_from_utc = %d\n",	ps_PrintDescAttr->PrinterStateChangeDateTime.minutes_from_utc,0,0,0,0,0 );
	// HeliosL KRQ-93 AirPrint1.5(WP14) 2015.02.13 OPS1八代 END.
	
	// Helios KRQ-93 AirPrint1.5対応 HCL-Sankar 2015/02/03 ADDED START
	for( i=0; i < IPP_PRINT_JOB_M_ATTR_END; i++ ) {
		SYSF_logMsg("[IPP] DocumentFormatVaryingAttribtues[%d] = %d\n",i,ps_PrintDescAttr->DocumentFormatVaryingAttribtues[i],0,0,0,0);
	}

	SYSF_logMsg("[IPP] Document_Format_Preferred = %d\n",ps_PrintDescAttr->Document_Format_Preferred,0,0,0,0,0);
	SYSF_logMsg("[IPP] PrinterStringsUri = %s\n",ps_PrintDescAttr->PrinterStringsUri,0,0,0,0,0);
	
	for( i=0; i < IPP_ATTR_LANG_END; i++ ) {
		SYSF_logMsg("[IPP] PrinterStringsLanguageSupported[%d] = %d\n",i,ps_PrintDescAttr->PrinterStringsLanguageSupported[i],0,0,0,0);
	}
	// Helios KRQ-93 AirPrint1.5対応 HCL-Sankar 2015/02/03 ADDED END
	
	// Helios KRQ-93 AirPrint1.5対応 HCL-Vijay 2015/02/04 Added START
	for( i=0; i < MAX_IPP_PRINTER_FINISHER; i++ ) {
		SYSF_logMsg("[IPP] PrinterFinisher[%d] = %s\n",i,ps_PrintDescAttr->PrinterFinisher[i],0,0,0,0);
		SYSF_logMsg("[IPP] PrinterFinisherDescription[%d] = %s\n",i,ps_PrintDescAttr->PrinterFinisherDescription[i],0,0,0,0);
	}
	// Helios KRQ-93 AirPrint1.5対応 HCL-Vijay 2015/02/04 Added END
	
	// Helios KRQ-101 AirPrint1.5対応 HCL-Vijay 2015/03/09 Start.
	SYSF_logMsg("[IPP] MopriaVersion = %d\n",ps_PrintDescAttr->MopriaVersion,0,0,0,0,0);
	// Helios KRQ-101 AirPrint1.5対応 HCL-Vijay 2015/03/09 End.

	//HeliosL_IT5_2.1 CQ511353 HCL-Sankar 2015/03/12 Added START
	for( i=0; i < IPP_STITCHING_END; i++ ) {
		SYSF_logMsg("[IPP] StitchingSupported[%d] = %d\n",i,ps_PrintDescAttr->StitchingSupported[i],0,0,0,0);
	}
	for( i=0; i < IPP_STICHING_END; i++ ) {
		SYSF_logMsg("[IPP] StitchingReferenceEdgeSupported[%d] = %d\n",i,ps_PrintDescAttr->StitchingReferenceEdgeSupported[i],0,0,0,0);
	}
	for( i=0; i < IPP_BALING_TYPE_END; i++ ) {
		SYSF_logMsg("[IPP] BalingTypeSupported[%d] = %d\n",i,ps_PrintDescAttr->BalingTypeSupported[i],0,0,0,0);
	}
	for( i=0; i < IPP_BALING_WHEN_END; i++ ) {
		SYSF_logMsg("[IPP] BalingWhenSupported[%d] = %d\n",i,ps_PrintDescAttr->BalingWhenSupported[i],0,0,0,0);
	}
	for( i=0; i < IPP_BINDING_EDGE_END; i++ ) {
		SYSF_logMsg("[IPP] BindingReferenceEdgeSupported[%d] = %d\n",i,ps_PrintDescAttr->BindingReferenceEdgeSupported[i],0,0,0,0);
	}
	for( i=0; i < IPP_BINDING_TYPE_END; i++ ) {
		SYSF_logMsg("[IPP] BindingTypeSupported[%d] = %d\n",i,ps_PrintDescAttr->BindingTypeSupported[i],0,0,0,0);
	}
	for( i=0; i < IPP_COATING_SIDES_END; i++ ) {
		SYSF_logMsg("[IPP] CoatingSideSupported[%d] = %d\n",i,ps_PrintDescAttr->CoatingSideSupported[i],0,0,0,0);
	}
	for( i=0; i < IPP_COATING_TYPE_END; i++ ) {
		SYSF_logMsg("[IPP] CoatingTypeSupported[%d] = %d\n",i,ps_PrintDescAttr->CoatingTypeSupported[i],0,0,0,0);
	}
	for( i=0; i < IPP_COVERING_NAME_END; i++ ) {
		SYSF_logMsg("[IPP] CoveringNameSupported[%d] = %d\n",i,ps_PrintDescAttr->CoveringNameSupported[i],0,0,0,0);
	}
	for( i=0; i < IPP_FOLDING_DIRECTION_END; i++ ) {
		SYSF_logMsg("[IPP] FoldingDirectionSupported[%d] = %d\n",i,ps_PrintDescAttr->FoldingDirectionSupported[i],0,0,0,0);
	}
	for( i=0; i < IPP_FOLDING_REFERENCE_END; i++ ) {
		SYSF_logMsg("[IPP] FoldingReferenceEdgeSupported[%d] = %d\n",i,ps_PrintDescAttr->FoldingReferenceEdgeSupported[i],0,0,0,0);
	}
	for( i=0; i < IPP_LAMINATING_SIDES_END; i++ ) {
		SYSF_logMsg("[IPP] LaminatingSidesSupported[%d] = %d\n",i,ps_PrintDescAttr->LaminatingSidesSupported[i],0,0,0,0);
	}
	for( i=0; i < IPP_LAMINATING_TYPE_END; i++ ) {
		SYSF_logMsg("[IPP] LaminatingTypeSupported[%d] = %d\n",i,ps_PrintDescAttr->LaminatingTypeSupported[i],0,0,0,0);
	}
	for( i=0; i < IPP_PUNCHING_REFERENCE_EDGE_END; i++ ) {
		SYSF_logMsg("[IPP] PunchingReferenceEdgeSupported[%d] = %d\n",i,ps_PrintDescAttr->PunchingReferenceEdgeSupported[i],0,0,0,0);
	}
	for( i=0; i < IPP_TRIMMING_REFERENCE_EDGE_END; i++ ) {
		SYSF_logMsg("[IPP] TrimmingReferenceEdgeSupported[%d] = %d\n",i,ps_PrintDescAttr->TrimmingReferenceEdgeSupported[i],0,0,0,0);
	}
	for( i=0; i < IPP_TRIMMING_TYPE_END; i++ ) {
		SYSF_logMsg("[IPP] TrimmingTypeSupported[%d] = %d\n",i,ps_PrintDescAttr->TrimmingTypeSupported[i],0,0,0,0);
	}
	for( i=0; i < IPP_TRIMMING_WHEN_END; i++ ) {
		SYSF_logMsg("[IPP] TrimmingWhenSupported[%d] = %d\n",i,ps_PrintDescAttr->TrimmingWhenSupported[i],0,0,0,0);
	}
	for( i=0; i < MAX_IPP_TRIM_OFFSET_SUPPORTED; i++ ) {
		SYSF_logMsg("[IPP] TrimmingOffsetSupported[%d] = %d\n",i,ps_PrintDescAttr->TrimmingOffsetSupported[i],0,0,0,0);
	}
	for( i=0; i < MAX_IPP_PUNCH_LOC_SUPPORTED; i++ ) {
		SYSF_logMsg("[IPP] PunchingLocationsSupported[%d] = %d\n",i,ps_PrintDescAttr->PunchingLocationsSupported[i],0,0,0,0);
	}
	for( i=0; i < MAX_IPP_PUNCH_OFFSET_SUPPORTED; i++ ) {
		SYSF_logMsg("[IPP] PunchingOffsetSupported[%d] = %d\n",i,ps_PrintDescAttr->PunchingOffsetSupported[i],0,0,0,0);
	}
	for( i=0; i < MAX_IPP_FOLDING_OFFSET_SUPPORTED; i++ ) {
		SYSF_logMsg("[IPP] FoldingOffsetSupported[%d] = %d\n",i,ps_PrintDescAttr->FoldingOffsetSupported[i],0,0,0,0);
	}
	for( i=0; i < MAX_IPP_STITCHING_LOC_SUPPORTED; i++ ) {
		SYSF_logMsg("[IPP] StitchingLocationsSupported[%d] = %d\n",i,ps_PrintDescAttr->StitchingLocationsSupported[i],0,0,0,0);
	}
	for( i=0; i < MAX_IPP_STITCHING_OFFSET_SUPPORTED; i++ ) {
		SYSF_logMsg("[IPP] StitchingOffsetSupported[%d] = %d\n",i,ps_PrintDescAttr->StitchingOffsetSupported[i],0,0,0,0);
	}
	//HeliosL_IT5_2.1 CQ511353 HCL-Sankar 2015/03/12 Added END
	//Icarus AirPrint1.6対応 HCL-Sankar 2016/01/04 Added Start.
	for( i=0; i < IPP_JPEG_END; i++ ) {
		SYSF_logMsg("[IPP] Jpeg_Features_Supported[%d] = %d\n",i,ps_PrintDescAttr->Jpeg_Features_Supported[i],0,0,0,0);
	}
	//Icarus AirPrint1.6対応 HCL-Sankar 2016/01/04 Added End.
	//IT5_4.1 Airprint1.7対応 HCL_VIJAY 2017/1/30 Added Start.
	for( i=0; i < IPP_REPERTOIRE_END; i++ ) {
		SYSF_logMsg("[IPP] Job_Passwd_Repertoire_Supported[%d] = %d\n",i,ps_PrintDescAttr->Job_Passwd_Repertoire_Supported[i],0,0,0,0);
	}
	SYSF_logMsg("[IPP] Job_Passwd_Repertoire_Configured = %d\n",ps_PrintDescAttr->Job_Passwd_Repertoire_Configured,0,0,0,0,0);
	//IT5_4.1 Airprint1.7対応 HCL_VIJAY 2017/1/30 Added End.
	SYSF_logMsg("[IPP]DispPrintDescAttr END\n",0,0,0,0,0,0);
	return;
}
// ZeusCD UV-01249 AppleAirPrint OPS1八代 2014/01/31 END.
