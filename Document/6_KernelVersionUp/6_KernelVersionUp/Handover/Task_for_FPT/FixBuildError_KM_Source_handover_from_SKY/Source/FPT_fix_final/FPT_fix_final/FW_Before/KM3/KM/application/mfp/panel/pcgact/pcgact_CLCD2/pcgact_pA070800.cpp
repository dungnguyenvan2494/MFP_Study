//------------------------------------------------------------------------------
/**
 * @file pcgact_pA070800.cpp
 * @brief 画面クラスの実装（ポップアップ画面）.
 */
//------------------------------------------------------------------------------

//================================================================================================
//  インクルード・ファイル.
//================================================================================================
#include <panel/plibh/plibh_CLCD/plibh_Mtui.h>
#include <system/ats/APIC_AutoSelect.h>
#include <panel/pcgact/pcgact_CLCD2/pcgact_pA070800.h>
#include <panel/pcgact/pcgact_CLCD2/pigact_pA070800.h>
#include <panel/pcgact/pcgact_CLCD2/pipact_pA070800.h>
#include <panel/pcgact/pcgact_CLCD2/pigact_pA070203.h>
#include <panel/pcgact/pcgact_CLCD2/pigact_pA070201.h>
#include <panel/pcgact/pcgact_CLCD2/pigact_pA070202.h>
#include <panel/pcgact/pcgact_CLCD2/pigact_pA070801.h>
#include <panel/pcgact/pcgact_CLCD2/pigact_pA071000.h>
#include <panel/pcgact/pcgact_CLCD2/pigact_base.h>
#include <panel/plibh/plibhg/plibh.h>
#include <panel/plibh/plibh_CLCD2/plibh_CLCD2.h>
#include <panel/plibh/plibhg/pdpcomb.h>
#include <panel/plibh/plibhg/plibhspc.h>
#include <panel/plibh/plibh_FUM2/plibh_FUM2.h>
#include <panel/plibh/plibh_CLCD/plibh_CLCD.h>
#include <panel/plibh/plibh_CLCD2/PNLC_ActionDataLib.h>
#include <panel/pcgcom/pcgcom_CLCD2/PPopupAnimationCom.h>
#include <panel/pcgcom/pcgcom_CLCD2/PNLC_PopupListControl.h>
#include <panel/pcgstub/pcgstub_FUM5/pigdummy_popup.h>
#include <panel/ppcg/ppcg_CLCD2/ppcginclude_CLCD2.h>
#include <panel/pcgact/pcgactg/pcgact.h>
#include <panel/pcgact/pcgact_CLCD2/pigact_pA070204.h>
#include <panel/pddgrap/pddgrap_CLCD/pddgraptbl.h>
#include <panel/plibh/plibh_CLCD2/plibhinclude_CLCD2.h>
#include <panel/pddtext/pddtext_CLCD/pstr_CLCD.h>
#include <panel/pddtext/pddtext_CLCD/psstr_CLCD.h>
#include <panel/pddtext/pddtext_CLCD/pfstr_CLCD.h>
#include <panel/pcgact/pcgact_CLCD2/pigact_pA070801.h>
#include <panel/pcgact/pcgact_CLCD2/pigact_pA070800.h>
#include <panel/pcgact/pcgact_CLCD2/pigact_pA072000.h>
#include <panel/pcgact/pcgact_CLCD2/pigact_pA070200.h>
#include <panel/pcgact/pcgact_CLCD2/pigact_pA072000.h>
#include <panel/pcgcopy/pcgcopy_CLCD2/pigcopy_pA020175.h>
#include <panel/pcgcopy/pcgcopy_CLCD2/pigcopy_base.h>
#include <panel/pcgscanfax/pcgscanfax_CLCD2/pigscanfax_base.h>
#include <panel/pcgbox/pcgbox_CLCD2/pigbox_base.h>
#include <panel/pcgcopy/pcgcopy_CLCD2/pigcopy_pA020121.h>
#include <panel/pcgcopy/pcgcopy_CLCD2/pigcopy_pA0201A1.h>
#include <panel/plibh/plibh_CLCD2/PNLC_ActionDataKeyControlLib.h>
#include <panel/plibh/plibh_CLCD2/PNLC_FuncPatternSettingLib.h>
#include <panel/pddtext/pddtext_CLCD/psstr_CLCD.h>
#include <panel/pcgwarn/pcgwarn_CLCD2/pigwarn_pA0A4200.h>
#include <panel/plibh/plibh_CLCD2/PNLC_StackManJobSpec.h>
#include <panel/plibh/plibh_CLCD2/piglib_pA010194.h>
#include <panel/plibh/plibh_CLCD2/PNLC_OutsideKeyCallDataLib.h>
#include <panel/pcgwarn/pcgwarn_CLCD2/pigwarn_pA0A3900.h>
#include <panel/pcgcopy/pcgcopy_CLCD2/pcgcopy_FinishingPosTable.h>
#include "panel/ppcg/ppcg_CLCD/pidispd_CLCD.h"
#include <panel/plibh/plibh_CLCD2/PNLC_SlideMenuControlLib.h>
#include "panel/plibh/plibh_CLCD2/PNLC_SoftSlideMenuTenKeyTextDispLib.h"
#include <panel/pcgact/pcgact_PD/pcgact_A070201100.h>
#include <panel/pcgact/pcgact_PD/pcgact_A070201200.h>
#include <panel/pcgact/pcgact_PD/pcgact_A070201700.h>
#include <panel/pcgact/pcgact_PD/pcgact_A070203100.h>
#include <panel/pcgact/pcgact_PD/pcgact_A070203800.h>
#include <panel/pcgact/pcgact_PD/pcgact_A070204400.h>
#include <panel/pcgact/pcgact_PD/pcgact_A070205200.h>
#include <panel/pcgact/pcgact_PD/pcgact_A070800130.h>
#include <panel/pcgact/pcgact_PD/pcgact_A070800300.h>
#include <panel/pcgact/pcgact_PD/pcgact_A070800710.h>
#include <panel/pcgact/pcgact_PD/pcgact_A070800500.h>
#include <panel/pcgact/pcgact_PD/pcgact_A070801000.h>
#include <panel/pcgact/pcgact_PD/pcgact_A070202700.h>
#include <panel/pcgact/pcgact_PD/pcgact_A070202800.h>
#include <panel/pcgact/pcgact_PD/pcgact_A070800150.h>
#include <panel/pcgact/pcgact_PD/pcgact_A070800140.h>
// comment 2019/03/13 Sky飛田 【制御構成変更_No.66】 仕上がりプレビュー対応.
#include <panel/pdp/pdpg/pdpc_WebUIControl.h>
extern PA070201100 gp_A070201100;
extern PA070201200 gp_A070201200;
extern PA070201700 gp_A070201700;
extern PA070203100 gp_A070203100;
extern PA070203800 gp_A070203800;
extern PA070204400 gp_A070204400;
extern PA070205200 gp_A070205200;
extern PA070800300 gp_A070800300;
extern PA070800710 gp_A070800710;
extern PA070800500 gp_A070800500;
extern PA070801000 gp_A070801000;
extern PA070202700 gp_A070202700;
extern PA070202800 gp_A070202800;
extern PA070800130 gp_A070800130;
extern PA070800140 gp_A070800140;
extern PA070800150 gp_A070800150;

/*================================================================================================
  定数・型の宣言.
================================================================================================*/

//倍率値.
#define MaxMagInput		( 400.00f )
#define MinMagInput		( 25.00f )

// カラー選択状態.
#define PNLE_ProgramJobSetChangeMenu_act_AutoColor		0	// オｰトカラｰ.
#define PNLE_ProgramJobSetChangeMenu_act_FullColor		1	// フルカラｰ.
#define PNLE_ProgramJobSetChangeMenu_act_Black			2	// ブラック.
#define PNLE_ProgramJobSetChangeMenu_act_2Color			3	// 2色カラｰ.
#define PNLE_ProgramJobSetChangeMenu_act_SingleColor	4	// 単色カラｰ.

/*================================================================================================
  列挙型変数のタグ宣言.
================================================================================================*/
// ▼ 2010/06/03 AWS_マーク A070800100 UT BugNo:13623.
struct TYPS_DocumentSizeInfo {
	TYPE_SizeCode		e_PaperSize;
	TEXT_ID				puc_TextID;
};
// 用紙種類テーブル.
static TYPS_DocumentSizeInfo st_DocumentSizeInfoTbl[] = {
	{ TYPE_SizeCodeA3Sef, s_pNOTSheetSize_A3SEF_W096H028A },		// 外部定義_定型サイズ：A3 SEF.
	{ TYPE_SizeCodeA4Sef, s_pNOTSheetSize_A4SEF_W096H028A },		// 外部定義_定型サイズ：A4 SEF.
	{ TYPE_SizeCodeA4Lef, s_pNOTSheetSize_A4LEF_W096H028A },		// 外部定義_定型サイズ：A4 LEF.
	{ TYPE_SizeCodeA5Sef, s_pNOTSheetSize_A5SEF_W096H028A },		// 外部定義_定型サイズ：A5 SEF.
	{ TYPE_SizeCodeA5Lef, s_pNOTSheetSize_A5LEF_W096H028A },		// 外部定義_定型サイズ：A5 LEF.
	{ TYPE_SizeCodeA6Sef, s_pNOTSheetSize_A6SEF_W096H028A },		// 外部定義_定型サイズ：A6 SEF.
	{ TYPE_SizeCodeA6Lef, s_pNOTSheetSize_A6LEF_W080H028A },		// 外部定義_定型サイズ：A6 LEF.
	{ TYPE_SizeCodeB4Sef, s_pNOTSheetSize_B4SEF_W096H028A },		// 外部定義_定型サイズ：B4 SEF.
	{ TYPE_SizeCodeB5Sef, s_pNOTSheetSize_B5SEF_W096H028A },		// 外部定義_定型サイズ：B5 SEF.
	{ TYPE_SizeCodeB5Lef, s_pNOTSheetSize_B5LEF_W096H028A },		// 外部定義_定型サイズ：B5 LEF.
	{ TYPE_SizeCodeB6Sef, s_pNOTSheetSize_B6SEF_W096H028A },		// 外部定義_定型サイズ：B6 SEF.
	{ TYPE_SizeCodeB6Lef, s_pNOTSheetSize_B6LEF_W080H028A },		// 外部定義_定型サイズ：B6 LEF.
	{ TYPE_SizeCodeLedgerSef, s_pNOTSheetSize_11x17SEF_W096H028A },			// 外部定義_定型サイズ：Ledger SEF.
	{ TYPE_SizeCodeLegalSef, s_pNOTSheetSize_812x14SEF_W096H028A },			// 外部定義_定型サイズ：Legal SEF.
	{ TYPE_SizeCodeLetterSef, s_pNOTSheetSize_812x11SEF_W096H028A },		// 外部定義_定型サイズ：Letter SEF.
	{ TYPE_SizeCodeLetterLef, s_pNOTSheetSize_812x11LEF_W096H028A },		// 外部定義_定型サイズ：Letter LEF.
	{ TYPE_SizeCodeExecutiveSef, s_pNOTSheetSize_714x1012_W096H028A },		// 外部定義_定型サイズ：Executive SEF.
	{ TYPE_SizeCodeExecutiveLef, s_pNOTSheetSize_714x1012LEF_W096H028A },	// 外部定義_定型サイズ：Executive LEF.
	{ TYPE_SizeCodeInvoiceSef, s_pNOTSheetSize_512x812_W096H028A },			// 外部定義_定型サイズ：Invoice SEF.
	{ TYPE_SizeCodeInvoiceLef, s_pNOTSheetSize_512x812LEF_W096H028A },		// 外部定義_定型サイズ：Invoice LEF.
	{ TYPE_SizeCode8KSef, s_pNOTSheetSize_16KLEF_W096H028A },
	{ TYPE_SizeCode16KLef, s_pKEYSheetSize_PostcardSEF_W096H028A},
	{ TYPE_SizeCode16KSef, s_pNOTSheetSize_8KSEF_W096H028A},
	{ TYPE_SizeCodeFlsSef, s_pNOTSheetSize_16KSEF_W096H028A},
	{ TYPE_SizeCodeIN4x6Sef, s_pNOTSheetSize_4x6SEF_W096H028A},
	{ TYPE_SizeCodeIN4x6Lef, s_pNOTSheetSize_4x6LEF_W096H028A},
	{ TYPE_SizeCodePostcard_JpSef, s_pNOTSheetSize_8x13SEF_W096H028A },		// 外部定義_定型サイズ：PostCard_JP SEF.
	{ TYPE_SizeCodePostcard_JpLef, s_pKEYSheetSize_PostcardLEF_W096H028A },	// 外部定義_定型サイズ：PostCard_JP LEF.
	{ TYPE_SizeCodePostcard_EuSef, s_pKEYSheetSize_A6CardSEF_W096H028A },	// 外部定義_定型サイズ：PostCard_EU SEF.
	{ TYPE_SizeCodePostcard_EuLef, s_pKEYSheetSize_A6CardLEF_W096H028A },	// 外部定義_定型サイズ：PostCard_EU LEF.
	{ TYPE_SizeCodeA0x2Sef, s_pNOTSheetSize_A0x2_W072H028A },				// 外部定義_定型サイズ：拡大連写用  A0 ×  2 SEF.
	{ TYPE_SizeCodeA0Sef, s_pNOTSheetSize_A0_W072H028A },					// 外部定義_定型サイズ：拡大連写用  A0 SEF.
	{ TYPE_SizeCodeA1Sef, s_pNOTSheetSize_A1_W072H028A },					// 外部定義_定型サイズ：拡大連写用  A1 SEF.
	{ TYPE_SizeCodeA2Sef, s_pNOTSheetSize_A2_W072H028A },					// 外部定義_定型サイズ：拡大連写用  A2 SEF.
	{ TYPE_SizeCodeB0Sef, s_pNOTSheetSize_B0_W072H028A },					// 外部定義_定型サイズ：拡大連写用  B0 SEF.
	{ TYPE_SizeCodeB1Sef, s_pNOTSheetSize_B1_W072H028A },					// 外部定義_定型サイズ：拡大連写用  B1 SEF.
	{ TYPE_SizeCodeB2Sef, s_pNOTSheetSize_B2_W072H028A },					// 外部定義_定型サイズ：拡大連写用  B2 SEF.
	{ TYPE_SizeCodeB3Sef, s_pNOTSheetSize_B3_W072H028A },					// 外部定義_定型サイズ：拡大連写用  B3 SEF.
	{ TYPE_SizeCode48x64Sef, s_pNOTSheetSize_48x64_W072H028A },				// 外部定義_定型サイズ：拡大連写用  48 × 64 SEF.
	{ TYPE_SizeCode36x48Sef, s_pNOTSheetSize_36x48_W072H028A },				// 外部定義_定型サイズ：拡大連写用  36 × 48 SEF.
	{ TYPE_SizeCode24x36Sef, s_pNOTSheetSize_24x36_W072H028A },				// 外部定義_定型サイズ：拡大連写用  24 × 36 SEF.
	{ TYPE_SizeCode18x24Sef, s_pNOTSheetSize_18x24_W072H028A },				// 外部定義_定型サイズ：拡大連写用  18 × 24 SEF.
	{ TYPE_SizeCode44x68Sef, s_pNOTSheetSize_44x68_W072H028A },				// 外部定義_定型サイズ：拡大連写用  44 × 68 SEF.
	{ TYPE_SizeCode34x44Sef, s_pNOTSheetSize_34x44_W072H028A },				// 外部定義_定型サイズ：拡大連写用  34 × 44 SEF.
	{ TYPE_SizeCode22x34Sef, s_pNOTSheetSize_22x34_W072H028A },				// 外部定義_定型サイズ：拡大連写用  22 × 34 SEF.
	{ TYPE_SizeCode17x22Sef, s_pNOTSheetSize_17x22_W072H028A },				// 外部定義_定型サイズ：拡大連写用  17 × 22 SEF.
	{ TYPE_SizeCodePhotoESef, s_pKEYSheetSize_ESizeSEF_W096H028A },			// 外部定義_定型サイズ：写真サイズ・日本・Ｅサイズ ( 117mm × 83mm ) SEF.
	{ TYPE_SizeCodePhotoELef, s_pKEYSheetSize_ESizeLEF_W096H028A },			// 外部定義_定型サイズ：写真サイズ・日本・Ｅサイズ ( 117mm × 83mm ) LEF.
	{ TYPE_SizeCodePhotoLSef, s_pKEYSheetSize_LSizeSEF_W096H028A },			// 外部定義_定型サイズ：写真サイズ・日本・Ｌサイズ ( 127mm × 89mm ) SEF.
	{ TYPE_SizeCodePhotoLLef, s_pKEYSheetSize_LSizeLEF_W096H028A },			// 外部定義_定型サイズ：写真サイズ・日本・Ｌサイズ ( 127mm × 89mm ) LEF.
	{ TYPE_SizeCode9x13Sef,	s_pNOTSheetSize_9x13SEF_W096H028A },			// 外部定義_定型サイズ：写真サイズ・ＥＵ・90mm × 130mm SEF.
	{ TYPE_SizeCode9x13Lef,	s_pNOTSheetSize_9x13LEF_W096H028A },			// 外部定義_定型サイズ：写真サイズ・ＥＵ・90mm × 130mm LEF.
	{ TYPE_SizeCode10x15Sef, s_pNOTSheetSize_10x15SEF_W096H028A },			// 外部定義_定型サイズ：写真サイズ・ＥＵ・100mm × 150mm SEF.
	{ TYPE_SizeCode10x15Lef, s_pNOTSheetSize_10x15LEF_W096H028A },			// 外部定義_定型サイズ：写真サイズ・ＥＵ・100mm × 150mm LEF.
	{ TYPE_SizeCode13x18Sef, s_pNOTSheetSize_13x18SEF_W096H028A },			// 外部定義_定型サイズ：写真サイズ・ＥＵ・130mm × 180mm SEF.
	{ TYPE_SizeCode13x18Lef, s_pNOTSheetSize_13x18LEF_W096H028A },			// 外部定義_定型サイズ：写真サイズ・ＥＵ・130mm × 180mm LEF.
	{ TYPE_SizeCodeIN4x6Sef, s_pNOTSheetSize_4x6SEF_W096H028A },			// 外部定義_定型サイズ：写真サイズ・ＵＳ・4inch × 6inch SEF.
	{ TYPE_SizeCodeIN4x6Lef, s_pNOTSheetSize_4x6LEF_W096H028A },			// 外部定義_定型サイズ：写真サイズ・ＵＳ・4inch × 6inch LEF.
	{ TYPE_SizeCodeIN3x5Sef, s_pNOTSheetSize_3x5SEF_W096H028A },			// 外部定義_定型サイズ：写真サイズ・ＵＳ・3inch × 3inch SEF.
	{ TYPE_SizeCodeIN3x5Lef, s_pNOTSheetSize_3x5LEF_W096H028A },			// 外部定義_定型サイズ：写真サイズ・ＵＳ・3inch × 3inch LEF.
	{ TYPE_SizeCodeIN225x325Sef, SF_TEXT_DYNAMIC_ALLOC },					// 外部定義_定型サイズ：写真サイズ・ＵＳ・2_1/4inch × 3_1/4inch SEF.
	{ TYPE_SizeCodeIN225x325Lef, SF_TEXT_DYNAMIC_ALLOC },					// 外部定義_定型サイズ：写真サイズ・ＵＳ・2_1/4inch × 3_1/4inch LEF.
	{ TYPE_SizeCode12x18Sef, s_pNOTSheetSize_12x18SEF_W080H028A },			// 外部定義_定型サイズ：12 × 18 inch SEF.
	{ TYPE_SizeCode10x14Sef, SF_TEXT_DYNAMIC_ALLOC },						// 外部定義_定型サイズ：10 × 14 inch SEF.
	{ TYPE_SizeCodeGovermentLetterSef, SF_TEXT_DYNAMIC_ALLOC },				// 外部定義_定型サイズ：GovermentLetter：8inch × 10_1/2inch SEF.
	{ TYPE_SizeCode8x10Sef, SF_TEXT_DYNAMIC_ALLOC },						// 外部定義_定型サイズ：UX Quarto/8inch × 10inch NorthAmericanPaper SEF.
	{ TYPE_SizeCodeJpLetterL3, SF_TEXT_DYNAMIC_ALLOC },						// 外部定義_定型サイズ：長形3号.
	{ TYPE_SizeCodeJpLetterK2, SF_TEXT_DYNAMIC_ALLOC },						// 外部定義_定型サイズ：角形2号.
	{ TYPE_SizeCodeJpLetterY1, SF_TEXT_DYNAMIC_ALLOC },						// 外部定義_定型サイズ：洋形1号.
	{ TYPE_SizeCodeJpLetterY2, SF_TEXT_DYNAMIC_ALLOC },						// 外部定義_定型サイズ：洋形2号.
	{ TYPE_SizeCodeJpLetterY4, SF_TEXT_DYNAMIC_ALLOC },						// 外部定義_定型サイズ：洋形4号.
	{ TYPE_SizeCodeJpLetterY6, SF_TEXT_DYNAMIC_ALLOC },						// 外部定義_定型サイズ：洋形6号.
	{ TYPE_SizeCodeEnvelopeMornarch, s_pNOTenvMonarch_W096H028A },			// 外部定義_定型サイズ：キングサイズの封筒サイズ  3.87 × 7.5 inch.
	{ TYPE_SizeCodeEnvelopeCom10, s_pNOTenvCom10_W096H028A },				// 外部定義_定型サイズ：封筒 Com10 4.81 × 9.21 inch.
	{ TYPE_SizeCodeEnvelopeDL, s_pNOTenvDL_W080H028A },						// 外部定義_定型サイズ：封筒 DL 110mm × 220mm.
	{ TYPE_SizeCodeEnvelopeC5, s_pNOTenvC5_W080H028A },						// 外部定義_定型サイズ：封筒 C5 162mm × 229mm.
	{ TYPE_SizeCodeEnvelopeB5, s_pNOTenvB5_W080H028A },						// 外部定義_定型サイズ：封筒 B5.
};
const uchar uc_DocumentSizeInfoTbl = (uchar)(sizeof(st_DocumentSizeInfoTbl) / sizeof(TYPS_DocumentSizeInfo));
// ▲ 2010/06/03 AWS_マーク A070800100 UT BugNo:13623.
#ifdef DEF_A4_Cus	// A3A4FUM OPS2 Sky後藤(拓) 2015/01/19.
// インデック原稿・原稿サイズ 
const TYPE_SizeCode e_AryOriginalIndexPaperTblIndex_act[] = {
	TYPE_SizeCodeA4Sef,
	TYPE_SizeCodeB5Sef,
	TYPE_SizeCodeLegalSef,
	TYPE_SizeCodeLetterSef,
};
const uchar uc_OriginalIndexPaperTblIndex_act = (uchar)(sizeof(e_AryOriginalIndexPaperTblIndex_act) / sizeof(TYPE_SizeCode));

// インデック原稿・ボタンインデックス・ボタンセレクト対応 
const uchar uc_AryOriginalPaperBtnStatusTblIndex_act[] = {
	BTNID_KEY_MANUSCRIPTINDEXSIZEA4T,
	BTNID_KEY_MANUSCRIPTINDEXSIZEB5T,
	BTNID_KEY_MANUSCRIPTINDEXSIZE8H_14T,
	BTNID_KEY_MANUSCRIPTINDEXSIZE8H_11T,
};
const uchar uc_OriginalPaperBtnStatusTblIndex_act = (uchar)(sizeof(uc_AryOriginalPaperBtnStatusTblIndex_act) / sizeof(uc_AryOriginalPaperBtnStatusTblIndex_act[0]));

// US仕向けインデック原稿・原稿サイズ 
const TYPE_SizeCode e_AryOriginalIndexPaperTblIndexUS_act[] = {
	TYPE_SizeCodeLegalSef,
	TYPE_SizeCodeLetterSef,
	TYPE_SizeCodeA4Sef,
	TYPE_SizeCodeB5Sef,
};
const uchar uc_IndexIndexOriginalIndexPaperTblIndexUS_act = (uchar)(sizeof(e_AryOriginalIndexPaperTblIndexUS_act) / sizeof(TYPE_SizeCode));

// US仕向けインデック原稿・ボタンインデックス・ボタンセレクト対応 
const uchar uc_AryOriginalPaperBtnStatusTblIndexUS_act[] = {
	BTNID_KEY_MANUSCRIPTINDEXSIZE8H_14T,
	BTNID_KEY_MANUSCRIPTINDEXSIZE8H_11T,
	BTNID_KEY_MANUSCRIPTINDEXSIZEA4T,
	BTNID_KEY_MANUSCRIPTINDEXSIZEB5T,
};
const uchar uc_OriginalPaperBtnStatusTblIndexUS_act = (uchar)(sizeof(uc_AryOriginalPaperBtnStatusTblIndexUS_act) / sizeof(uc_AryOriginalPaperBtnStatusTblIndexUS_act[0]));
#else // DEF_A4_Cus	// A3A4FUM OPS2 Sky後藤(拓) 2015/01/19.
// comment MinervaZeus -> Sky金田 2010/11/09 [UV-00995].
// インデック原稿・原稿サイズ
const TYPE_SizeCode e_AryOriginalIndexPaperTblIndex_act[] = {
	TYPE_SizeCodeA3Sef,
	TYPE_SizeCodeB4Sef,
	TYPE_SizeCodeA4Sef,
	TYPE_SizeCodeA4Lef,
	TYPE_SizeCodeB5Sef,
	TYPE_SizeCodeLegalSef,
	TYPE_SizeCodeLetterSef,
	TYPE_SizeCodeLetterLef,
};
const uchar uc_OriginalIndexPaperTblIndex_act = ( uchar )( sizeof( e_AryOriginalIndexPaperTblIndex_act ) / sizeof( TYPE_SizeCode ) );

// インデック原稿・ボタンインデックス・ボタンセレクト対応
const uchar uc_AryOriginalPaperBtnStatusTblIndex_act[] = {
	BTNID_KEY_MANUSCRIPTINDEXSIZEA3T,
	BTNID_KEY_MANUSCRIPTINDEXSIZEB4T,
	BTNID_KEY_MANUSCRIPTINDEXSIZEA4Y,
	BTNID_KEY_MANUSCRIPTINDEXSIZEA4T,
	BTNID_KEY_MANUSCRIPTINDEXSIZEB5T,
	BTNID_KEY_MANUSCRIPTINDEXSIZE8H_14T,
	BTNID_KEY_MANUSCRIPTINDEXSIZE8H_11Y,
	BTNID_KEY_MANUSCRIPTINDEXSIZE8H_11T,
};
const uchar uc_OriginalPaperBtnStatusTblIndex_act = ( uchar )( sizeof( uc_AryOriginalPaperBtnStatusTblIndex_act ) / sizeof( uc_AryOriginalPaperBtnStatusTblIndex_act[0] ) );

// US仕向けインデック原稿・原稿サイズ
const TYPE_SizeCode e_AryOriginalIndexPaperTblIndexUS_act[] = {
	TYPE_SizeCodeLegalSef,
	TYPE_SizeCodeLetterSef,
	TYPE_SizeCodeLetterLef,
	TYPE_SizeCodeA3Sef,
	TYPE_SizeCodeB4Sef,
	TYPE_SizeCodeA4Sef,
	TYPE_SizeCodeA4Lef,
	TYPE_SizeCodeB5Sef,
};
const uchar uc_IndexIndexOriginalIndexPaperTblIndexUS_act = ( uchar )( sizeof( e_AryOriginalIndexPaperTblIndexUS_act ) / sizeof( TYPE_SizeCode ) );

// US仕向けインデック原稿・ボタンインデックス・ボタンセレクト対応
const uchar uc_AryOriginalPaperBtnStatusTblIndexUS_act[] = {
	BTNID_KEY_MANUSCRIPTINDEXSIZE8H_14T,
	BTNID_KEY_MANUSCRIPTINDEXSIZE8H_11Y,
	BTNID_KEY_MANUSCRIPTINDEXSIZE8H_11T,
	BTNID_KEY_MANUSCRIPTINDEXSIZEA3T,
	BTNID_KEY_MANUSCRIPTINDEXSIZEB4T,
	BTNID_KEY_MANUSCRIPTINDEXSIZEA4Y,
	BTNID_KEY_MANUSCRIPTINDEXSIZEA4T,
	BTNID_KEY_MANUSCRIPTINDEXSIZEB5T,
};
const uchar uc_OriginalPaperBtnStatusTblIndexUS_act = (uchar)(sizeof(uc_AryOriginalPaperBtnStatusTblIndexUS_act) / sizeof(uc_AryOriginalPaperBtnStatusTblIndexUS_act[0]));
// comment MinervaZeus <- Sky金田 2010/11/09 [UV-00995].
#endif // DEF_A4_Cus	// A3A4FUM OPS2 Sky後藤(拓) 2015/01/19.

struct PNLS_BaseColorPartsInfo {
	PNLS_Parts*			pst_Parts;			//!< パーツ.
	TEXT_ID				text_ID;			//!< シンボルID.
};
// 単色カラー用.
static const PNLS_BaseColorPartsInfo st_BaseColorPartsInfoTbl_SingleColor[] = {
#ifdef DEF_LCD_7INCH
	{ tbl_i00_029_000_X32_Y32,	s_pITMRed_W152H018A },				// 単色カラー（red）.
	{ tbl_i00_045_000_X32_Y32,	s_pITMTeal_W152H018A },				// 単色カラー（Teal）.
	{ tbl_i00_031_000_X32_Y32,	s_pITMBlue_W152H018A },				// 単色カラー（Blue）.
	{ tbl_i00_049_000_X32_Y32,	s_pITMOrange_W152H018A },			// 単色カラー（Orange）.
	{ tbl_i00_033_000_X32_Y32,	s_pITMGreen_W152H018A },			// 単色カラー（Green）.
	{ tbl_i00_051_000_X32_Y32,	s_pITMPurple_W152H018A },			// 単色カラー（Purple）.
	{ tbl_i00_047_000_X32_Y32,	s_pITMPumpkin_W152H018A },			// 単色カラー（Pumpkin）.
	{ tbl_i00_053_000_X32_Y32,	s_pITMYellowGreen_W152H018A },		// 単色カラー（Yellow Green）.
	{ tbl_i00_055_000_X32_Y32,	s_pITMPlum_W152H018A },				// 単色カラー（Plum）.
	{ tbl_i00_057_000_X32_Y32,	s_pITMCamel_W152H018A },			// 単色カラー（Camel）.
	{ tbl_i00_061_000_X32_Y32,	s_pITMMaroon_W152H018A },			// 単色カラー（Maroon）.
	{ tbl_i00_059_000_X32_Y32,	s_pITMForestGreen_W152H018A },		// 単色カラー（Forest Green）.
	{ tbl_i00_063_000_X32_Y32,	s_pITMBrown_W152H018A },			// 単色カラー（Brown）.
	{ tbl_i00_065_000_X32_Y32,	s_pITMAquaMarine_W152H018A },		// 単色カラー（Aqua Marine）.
	{ tbl_i00_067_000_X32_Y32,	s_pITMPink_W152H018A },				// 単色カラー（Pink）.
	{ tbl_i00_035_000_X32_Y32,	s_pITMYellow_W152H018A },			// 単色カラー（Yellow）.
	{ tbl_i00_037_000_X32_Y32,	s_pITMCyan_W152H018A },				// 単色カラー（Cyan）.
	{ tbl_i00_039_000_X32_Y32,	s_pITMMagenta_W152H018A },			// 単色カラー（Magenta）.
	{ tbl_i00_041_000_X32_Y32,	s_pITMSepia1_W152H018A },			// 単色カラー（Sepia1）.
	{ tbl_i00_043_000_X32_Y32,	s_pITMSepia2_W152H018A },			// 単色カラー（Sepia2）.
	{ NULL,						NULL },								// 単色カラー（Black）.
	{ tbl_i00_069_000_X32_Y32,	s_pITMCopperRed_W152H018A },		// 単色カラー（Copper Red）.
#else // #ifdef DEF_LCD_7INCH.
	{ tbl_i00_029_000_X32_Y32,	s_pKEYRed_W120H028A },				// 単色カラー（red）.
	{ tbl_i00_045_000_X32_Y32,	s_pKEYTeal_W120H028A },				// 単色カラー（Teal）.
	{ tbl_i00_031_000_X32_Y32,	s_pKEYBlue_W120H028A },				// 単色カラー（Blue）.
	{ tbl_i00_049_000_X32_Y32,	s_pKEYOrange_W120H028A },			// 単色カラー（Orange）.
	{ tbl_i00_033_000_X32_Y32,	s_pKEYGreen_W120H028A },			// 単色カラー（Green）.
	{ tbl_i00_051_000_X32_Y32,	s_pKEYPurple_W120H028A },			// 単色カラー（Purple）.
	{ tbl_i00_047_000_X32_Y32,	s_pKEYPumpkin_W120H028A },			// 単色カラー（Pumpkin）.
	{ tbl_i00_053_000_X32_Y32,	s_pKEYYellowGreen_W120H028A },		// 単色カラー（Yellow Green）.
	{ tbl_i00_055_000_X32_Y32,	s_pKEYPlum_W120H028A },				// 単色カラー（Plum）.
	{ tbl_i00_057_000_X32_Y32,	s_pKEYCamel_W120H028A },			// 単色カラー（Camel）.
	{ tbl_i00_061_000_X32_Y32,	s_pKEYMaroon_W120H028A },			// 単色カラー（Maroon）.
	{ tbl_i00_059_000_X32_Y32,	s_pKEYForestGreen_W120H028A },		// 単色カラー（Forest Green）.
	{ tbl_i00_063_000_X32_Y32,	s_pKEYBrown_W120H028A },			// 単色カラー（Brown）.
	{ tbl_i00_065_000_X32_Y32,	s_pKEYAquaMarine_W120H028A },		// 単色カラー（Aqua Marine）.
	{ tbl_i00_067_000_X32_Y32,	s_pKEYPink_W120H028A },				// 単色カラー（Pink）.
	{ tbl_i00_035_000_X32_Y32,	s_pKEYYellow_W120H028A },			// 単色カラー（Yellow）.
	{ tbl_i00_037_000_X32_Y32,	s_pKEYCyan_W120H028A },				// 単色カラー（Cyan）.
	{ tbl_i00_039_000_X32_Y32,	s_pKEYMagenta_W120H028A },			// 単色カラー（Magenta）.
	{ tbl_i00_041_000_X32_Y32,	s_pKEYSepia1_W120H028A },			// 単色カラー（Sepia1）.
	{ tbl_i00_043_000_X32_Y32,	s_pKEYSepia2_W120H028A },			// 単色カラー（Sepia2）.
	{ NULL,						NULL },								// 単色カラー（Black）.
	{ tbl_i00_069_000_X32_Y32,	s_pKEYCopperRed_W120H028A },		// 単色カラー（Copper Red）.
#endif // #ifdef DEF_LCD_7INCH.
};
// ２色カラー用.
static const PNLS_BaseColorPartsInfo st_BaseColorPartsInfoTbl_2Color[] = {
#ifdef DEF_LCD_7INCH
	{ tbl_i00_017_000_X32_Y32,	s_pITMBlackPlusRed_W152H018A },		// 2色カラー_レッド+ブラック.
#else // #ifdef DEF_LCD_7INCH.
	{ tbl_i00_017_000_X32_Y32,	s_pKEYBlackPlusRed_W120H028A },		// 2色カラー_レッド+ブラック.
#endif // #ifdef DEF_LCD_7INCH.
	{ NULL,						NULL },
#ifdef DEF_LCD_7INCH
	{ tbl_i00_019_000_X32_Y32,	s_pITMBlackPlusBlue_W152H018A },	// 2色カラー_ブルー+ブラック.
#else // #ifdef DEF_LCD_7INCH.
	{ tbl_i00_019_000_X32_Y32,	s_pKEYBlackPlusBlue_W122H028A },	// 2色カラー_ブルー+ブラック.
#endif // #ifdef DEF_LCD_7INCH.
	{ NULL,						NULL },
#ifdef DEF_LCD_7INCH
	{ tbl_i00_021_000_X32_Y32,	s_pITMBlackPlusGreen_W152H018A },	// 2色カラー_グリーン+ブラック.
#else // #ifdef DEF_LCD_7INCH.
	{ tbl_i00_021_000_X32_Y32,	s_pKEYBlackPlusGreen_W120H028A },	// 2色カラー_グリーン+ブラック.
#endif // #ifdef DEF_LCD_7INCH.
	{ NULL,						NULL },
	{ NULL,						NULL },
	{ NULL,						NULL },
	{ NULL,						NULL },
	{ NULL,						NULL },
	{ NULL,						NULL },
	{ NULL,						NULL },
	{ NULL,						NULL },
	{ NULL,						NULL },
	{ NULL,						NULL },
#ifdef DEF_LCD_7INCH
	{ tbl_i00_023_000_X32_Y32,	s_pITMBlackPlusYellow_W152H018A },	// 2色カラー_イエロー+ブラック.
	{ tbl_i00_025_000_X32_Y32,	s_pITMBlackPlusCyan_W152H018A },	// 2色カラー_シアン+ブラック.
	{ tbl_i00_027_000_X32_Y32,	s_pITMBlackPlusMagenta_W152H018A },	// 2色カラー_マゼンタ+ブラック.
#else // #ifdef DEF_LCD_7INCH.
	{ tbl_i00_023_000_X32_Y32,	s_pKEYBlackPlusYellow_W120H028A },	// 2色カラー_イエロー+ブラック.
	{ tbl_i00_025_000_X32_Y32,	s_pKEYBlackPlusCyan_W122H028A },	// 2色カラー_シアン+ブラック.
	{ tbl_i00_027_000_X32_Y32,	s_pKEYBlackPlusMagenta_W120H028A },	// 2色カラー_マゼンタ+ブラック.
#endif // #ifdef DEF_LCD_7INCH.
	{ NULL,						NULL },
	{ NULL,						NULL },
	{ NULL,						NULL },
	{ NULL,						NULL },
};

/*================================================================================================
	モジュール内変数の宣言
================================================================================================*/
PCPC_ActSettingChange_act*	PCPC_ActSettingChange_act::pc_mActSettingChange_act;
PCPC_ProgramJobSetChangeMenu_act* PCPC_ProgramJobSetChangeMenu_act::pc_mProgramJobSetChangeMenu_act;

/*================================================================================================
 マクロ関数の定義.
================================================================================================*/


//関数------------------------------------------------------------------------------------------------------------------
//概要    ：画面開始処理.
//備考    ：.
//戻値    ：.
//レビュー：.
//------------------------------------------------------------------------------
/**
* @brief A070800120 枠消し : 画面開始処理
*/
//------------------------------------------------------------------------------
void PCPC_SetChangeCollectEraseFrameInch_act::startGamen()
{
	PNLC_OutSideKeyCallDataLib::newInstance()->setSelectNo( PNLE_Soft10KeyUnselected );

	// 関数 Call 処理：上位クラスの処理.
	PPopupDecisionButtonCom::startGamen();
//comment 2023/1/4 FPT.KhaiPQ	[OP_BTS-40915][IT6_2.3]	スキャンのプレビュー画面に枠消し：ON、削去量が「10.0mm」を設定する時に、「OK」キーを押下し、無効音を出す問題対策.	
	APIC_FaxPanelSetting* pc_FaxPanelSetting = PnlComGetFaxPanelSetting();
	if( NULL == pc_FaxPanelSetting ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }

// comment 2023/01/31 FPT.ThanhLH11 [IT6_2.3][ALL MACHINE][KLOCWORK]UNINIT.STACK.ARRAY.MIGHT.
	float sf_Erase[5] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };

	Cmp_SetChangeCollectEraseFrameInch_act_MessageArea.setOnOffButton( True );		// 関数 Call 処理：メッセージコンポーネントへON/OFFの設定.
	Cmp_SetChangeCollectEraseFrameInch_act_MessageArea.showDestinationCount( False );
	Cmp_SetChangeCollectEraseFrameInch_act_MessageArea.changeMessageDisplay( PNLE_PopupMessageAreaMessageShowAll );
	Cmp_SetChangeCollectEraseFrameInch_act_MessageArea.setMode( PNLE_PopupMessageAreaMessageModeAdmin );
	Cmp_SetChangeCollectEraseFrameInch_act_MessageArea.setOnOffSwitchButtonState( isOn( CNK_FrameErase, comPanelJob ) );
	Cmp_SetChangeCollectEraseFrameInch_act_MessageArea.setNowPopupGamen( PNLE_SetChangeCollectEraseFrameInch_act );
	Bool b_Book = isOn( CNK_BookRepeat, comPanelJob );
	if( b_Book == True ) {
		sf_FrameEraseValue = PnlComGetOperationSetting()->getEraseFrameAllScan();		// OUTPUT：枠消し数値の設定.
		sf_TopEraseValue = PnlComGetOperationSetting()->getEraseFrameTopScan();
		sf_BottomEraseValue = PnlComGetOperationSetting()->getEraseFrameBottomScan();
		sf_LeftEraseValue = PnlComGetOperationSetting()->getEraseFrameLeftScan();
		sf_RightEraseValue = PnlComGetOperationSetting()->getEraseFrameRightScan();
	} else {
		sf_FrameEraseValue = PnlComGetOperationSetting()->getEraseFrameAll();		// OUTPUT：枠消し数値の設定.
		sf_TopEraseValue = PnlComGetOperationSetting()->getEraseFrameTop();
		sf_BottomEraseValue = PnlComGetOperationSetting()->getEraseFrameBottom();
		sf_LeftEraseValue = PnlComGetOperationSetting()->getEraseFrameLeft();
		sf_RightEraseValue = PnlComGetOperationSetting()->getEraseFrameRight();
	}

	m_e_Unit = PnlComGetUnitSystem();
	switch( m_e_Unit ) {		// 条件分岐: 単位系.
		case TYPE_UnitSystemMM_Number:
			sf_MinValue = PCGD_MinFrameErase;
			sf_MaxValue = PCGD_MaxFrameErase;
// comment 2016/11/10 [HCLT_Anitha] [IT5_4.0][Quarter Font Phase2] [1006281] Changes for Quarter font to Half font.
//			Cmp_SetChangeCollectEraseFrameInch_act_MessageArea.changeMainMessageText( s_pMSGEraseWidthSettingBig_W448H016A );
			Cmp_SetChangeCollectEraseFrameInch_act_MessageArea.changeMainMessageText( s_pMSGEraseWidthSettingBig_W464H016A );
			Txt_SetChangeCollectEraseFrameInch_act_TopMM.showEnable( True );
			Txt_SetChangeCollectEraseFrameInch_act_BottomMM.showEnable( True );
			Txt_SetChangeCollectEraseFrameInch_act_LeftMM.showEnable( True );
			Txt_SetChangeCollectEraseFrameInch_act_RightMM.showEnable( True );
			break;
		case TYPE_UnitSystemInch_Number:
			sf_MinValue = PCGD_MinFrameEraseInchMM;
			sf_MaxValue = PCGD_MaxFrameEraseInchMM;
// comment 2016/11/10 [HCLT_Anitha] [IT5_4.0][Quarter Font Phase2] [1006281] Changes for Quarter font to Half font.
//			Cmp_SetChangeCollectEraseFrameInch_act_MessageArea.changeMainMessageText( s_pMSGEraseWidthSettingBig_W448H016A );
			Cmp_SetChangeCollectEraseFrameInch_act_MessageArea.changeMainMessageText( s_pMSGEraseWidthSettingBig_W464H016A );
			Txt_SetChangeCollectEraseFrameInch_act_TopMM.showEnable( False );
			Txt_SetChangeCollectEraseFrameInch_act_BottomMM.showEnable( False );
			Txt_SetChangeCollectEraseFrameInch_act_LeftMM.showEnable( False );
			Txt_SetChangeCollectEraseFrameInch_act_RightMM.showEnable( False );
			break;
		case TYPE_UnitSystemInch_Fraction:
			sf_MinValue = PCGD_MinFrameEraseInch;
			sf_MaxValue = PCGD_MaxFrameEraseInch;
			Cmp_SetChangeCollectEraseFrameInch_act_MessageArea.changeMainMessageText( s_pMSGEraseWidthSettingInchBig_W448H016A );
			Txt_SetChangeCollectEraseFrameInch_act_TopMM.showEnable( False );
			Txt_SetChangeCollectEraseFrameInch_act_BottomMM.showEnable( False );
			Txt_SetChangeCollectEraseFrameInch_act_LeftMM.showEnable( False );
			Txt_SetChangeCollectEraseFrameInch_act_RightMM.showEnable( False );
			break;
		default:
			break;
	}
// comment 2022/12/09 FPT_LinhNV46 [IT6_2.3][OP_BTS-40861] Converted integer value from GUI.
	Cmp_SetChangeCollectEraseFrameInch_act_UpDown.setInchFloatStatus(True);
	Cmp_SetChangeCollectEraseFrameInch_act_UpDown.Initialize( &sf_FrameEraseValue, sf_MinValue, sf_MaxValue, m_e_Unit );
	TYPC_OriginalErase* pc_OriginalErase = &( comPanelJob->getJobSpec()->pc_Scan->c_OriginalErase );		// OUTPUT: 枠消し量を取得.

	// OUTPUT：ミリ／インチ切り替え時に起こる最大値/最小値オーバーを補正する.
//comment 2023/1/4 FPT.KhaiPQ	[OP_BTS-40915][IT6_2.3]	スキャンのプレビュー画面に枠消し：ON、削去量が「10.0mm」を設定する時に、「OK」キーを押下し、無効音を出す問題対策.
	switch( pc_FaxPanelSetting->getEdgeErase() ) {		
		case TYPE_FaxEdgeErase05:
			sf_Erase[0] = PNLD_DefaultFaxOriginalEraseMm;
			sf_Erase[1] = PNLD_DefaultFaxOriginalEraseMm;
			sf_Erase[2] = PNLD_DefaultFaxOriginalEraseMm;
			sf_Erase[3] = PNLD_DefaultFaxOriginalEraseMm;
			sf_Erase[4] = PNLD_DefaultFaxOriginalEraseMm;
			break;
		case TYPE_FaxEdgeErase10:
			sf_Erase[0] = PNLD_DefaultFaxOriginalErase10Mm;
			sf_Erase[1] = PNLD_DefaultFaxOriginalErase10Mm;
			sf_Erase[2] = PNLD_DefaultFaxOriginalErase10Mm;
			sf_Erase[3] = PNLD_DefaultFaxOriginalErase10Mm;
			sf_Erase[4] = PNLD_DefaultFaxOriginalErase10Mm;
			break;
		case TYPE_FaxEdgeErase15:
			sf_Erase[0] = PNLD_DefaultFaxOriginalErase15Mm;
			sf_Erase[1] = PNLD_DefaultFaxOriginalErase15Mm;
			sf_Erase[2] = PNLD_DefaultFaxOriginalErase15Mm;
			sf_Erase[3] = PNLD_DefaultFaxOriginalErase15Mm;
			sf_Erase[4] = PNLD_DefaultFaxOriginalErase15Mm;
			break;
		default: 
			break;
	}
	if( True == isOn( CNK_FrameErase , comPanelJob ) ) {
		sf_Erase[1]	= reviseMersurementUnit( pc_OriginalErase->getTop(), PCGD_MaxFrameErase, PCGD_MaxFrameEraseInch, PCGD_MaxFrameEraseInchMM, PCGD_MinFrameErase, PCGD_MinFrameEraseInch, PCGD_MinFrameEraseInchMM );
		sf_Erase[2] = reviseMersurementUnit( pc_OriginalErase->getBottom(), PCGD_MaxFrameErase, PCGD_MaxFrameEraseInch, PCGD_MaxFrameEraseInchMM, PCGD_MinFrameErase, PCGD_MinFrameEraseInch, PCGD_MinFrameEraseInchMM );
		sf_Erase[3] = reviseMersurementUnit( pc_OriginalErase->getLeft(),   PCGD_MaxFrameErase, PCGD_MaxFrameEraseInch, PCGD_MaxFrameEraseInchMM, PCGD_MinFrameErase, PCGD_MinFrameEraseInch, PCGD_MinFrameEraseInchMM );
		sf_Erase[4] = reviseMersurementUnit( pc_OriginalErase->getRight(),  PCGD_MaxFrameErase, PCGD_MaxFrameEraseInch, PCGD_MaxFrameEraseInchMM, PCGD_MinFrameErase, PCGD_MinFrameEraseInch, PCGD_MinFrameEraseInchMM );
	}
	Bool b_On = False;
	uchar i = 0;
	for( i = 1; i <= 4; i++ ) {		// OUTPUT: 設定確認.
		if( 0.0f != sf_Erase[i] ) {
			b_On = True;
			break;
		}
	}
// Comment 2020/05/12 HCL-Satheesh [IT6_HMLK][OP_BTS-23329] When interger value entered convert in to float and use for screen display.
// comment 2022/12/09 FPT_LinhNV46 [IT6_2.3][OP_BTS-40861] Converted integer value from GUI.
//	Cmp_SetChangeCollectEraseFrameInch_act_UpDown.setInchFloatStatus(True);
	if( ( True == pc_OriginalErase->getFlameOrEachSideErase() ) || ( False == b_On ) ) {		// 条件分岐：設定確認.
		// 分岐判定.
		if( True == b_On ) {		// 条件分岐 : 設定確認.
			sf_EraseValue[0] = sf_Erase[1];
			Cmp_SetChangeCollectEraseFrameInch_act_UpDown.Initialize( &sf_EraseValue[0], sf_MinValue, sf_MaxValue, m_e_Unit );
		}
		ss_SelPos = 0;		// OUTPUT: 枠全体を選択.
	} else {
		// 分岐判定.
		if( True == b_On ) {		// 条件分岐 : 設定確認.
			for( i = 1; i <= 4; i++ ) {
				sf_EraseValue[i] = sf_Erase[i];
			}
		}
		if( PNLE_AuxilEraseFrame_OFF == sf_EraseValue[1] ) {
			Cmp_SetChangeCollectEraseFrameInch_act_UpDown.Initialize( &sf_MinValue, sf_MinValue, sf_MaxValue, m_e_Unit );
		} else {
			Cmp_SetChangeCollectEraseFrameInch_act_UpDown.Initialize( &sf_EraseValue[1], sf_MinValue, sf_MaxValue, m_e_Unit );
		}
		ss_SelPos = 1;	// OUTPUT: 上を選択.
	}
	ss_SelPosLast = 1;
	for( i = 0; i < 5; i++ ) {		// OUTPUT: 消去するキーチェック.
		if( 0.0f == sf_EraseValue[i] ) {		// 条件分岐 : 設定確認.
			bl_EraseOn[i] = False;
			sf_EraseValue[i] = sf_MinValue;
		} else {
			bl_EraseOn[i] = True;
		}
	}
	if( False == Cmp_SetChangeCollectEraseFrameInch_act_MessageArea.getOnOffSwitchButtonState() ) {
		sf_EraseValue[0] = Cmp_SetChangeCollectEraseFrameInch_act_UpDown.getNum();
		Nfd_SetChangeCollectEraseFrameInch_act_Top.set( sf_EraseValue[0], True );
		Nfd_SetChangeCollectEraseFrameInch_act_Bottom.set( sf_EraseValue[0], True );
		Nfd_SetChangeCollectEraseFrameInch_act_Left.set( sf_EraseValue[0], True );
		Nfd_SetChangeCollectEraseFrameInch_act_Right.set( sf_EraseValue[0], True );
		Nfd_SetChangeCollectEraseFrameInch_act_Top.changeLayoutX( TYPE_LayoutRightAdjust );
		Nfd_SetChangeCollectEraseFrameInch_act_Bottom.changeLayoutX( TYPE_LayoutRightAdjust );
		Nfd_SetChangeCollectEraseFrameInch_act_Left.changeLayoutX( TYPE_LayoutRightAdjust );
		Nfd_SetChangeCollectEraseFrameInch_act_Right.changeLayoutX( TYPE_LayoutRightAdjust );
		b_FunctionOffOnFlg = False;
	} else {
		Nfd_SetChangeCollectEraseFrameInch_act_Top.set( sf_EraseValue[1], True );
		Nfd_SetChangeCollectEraseFrameInch_act_Bottom.set( sf_EraseValue[2], True );
		Nfd_SetChangeCollectEraseFrameInch_act_Left.set( sf_EraseValue[3], True );
		Nfd_SetChangeCollectEraseFrameInch_act_Right.set( sf_EraseValue[4], True );
		b_FunctionOffOnFlg = True;
	}
	if( ( TYPE_UnitSystemInch_Fraction != PnlComGetUnitSystem() ) && ( False == PnlComIsHard10Key() ) ) {
		m_bl_isSelectedInputField = False;
	} else {
		m_bl_isSelectedInputField = True;
	}

	// 画面開始時に機能Offの場合は、消去するチェックを一律ONとする ↓.
	if( False == isOn( CNK_FrameErase, comPanelJob ) ) {
		Key_SetChangeCollectEraseFrameInch_act_CheckButton.chkOn();
		bl_EraseOn[0] = True;		// 内部フラグも有効に設定しておく 伊丹Sky(坂梨)  2011/05/20 CQ340544.
	}
	// 画面開始時に機能Offの場合は、消去するチェックを一律ONとする ↑.
}

//関数------------------------------------------------------------------------------------------------------------------
//概要    ：表示部品のセットアップ.
//備考    ：.
//戻値    ：.
//レビュー：.
//------------------------------------------------------------------------------
/**
* @brief A070800120 枠消し : 表示部品のセットアップ
*/
//------------------------------------------------------------------------------
void PCPC_SetChangeCollectEraseFrameInch_act::setupGamen( short option )
{
	// 関数 Call 処理：上位クラスの処理.
	PPopupDecisionButtonCom::setupGamen();

	Bak_SetChangeCollectEraseFrameInch_act_Top.showEnable( False );
	Bak_SetChangeCollectEraseFrameInch_act_Bottom.showEnable( False );
	Bak_SetChangeCollectEraseFrameInch_act_Left.showEnable( False );
	Bak_SetChangeCollectEraseFrameInch_act_Right.showEnable( False );

	// プログラムジョブがONの場合、メインメッセージを変更する
	if( True == comPanelJob->getJobSpec()->pc_Common->c_JobType.getProgramJobOn() ) {
		Cmp_SetChangeCollectEraseFrameInch_act_MessageArea.changeMainMessageText( s_pMSGPgJobManuscriptBunch_W448H034A );
	}

	if( False == Cmp_SetChangeCollectEraseFrameInch_act_MessageArea.getOnOffSwitchButtonState() ) {		// 条件分岐：OFF場合または, グレイアウト表示とする.
		setupGamenOff();
	} else {
		setupGamenOn();
	}
	Bak_SetChangeCollectEraseFrameInch_act_Top.showEnable( True );
	Bak_SetChangeCollectEraseFrameInch_act_Bottom.showEnable( True );
	Bak_SetChangeCollectEraseFrameInch_act_Left.showEnable( True );
	Bak_SetChangeCollectEraseFrameInch_act_Right.showEnable( True );

	sendSlide10KeyData();

// comment 2020/07/14 SCD2-Sky 石原 [IT6_2.0MR2][OP_BTS-25362] フッターエリア対応.
#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus)  && defined (DEF_LCD_7INCH) ) )
	PnlComFooterPartsBtnActiveChange(homeKeyActiveCheck(), idKeyActiveCheck(), resetKeyActiveCheck());
#endif		//  ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus)  && defined (DEF_LCD_7INCH) ) )	.

}

//------------------------------------------------------------------------------
/**
* @brief A070800120 枠消し : セットアップ（ON選択時）
*/
//------------------------------------------------------------------------------
void PCPC_SetChangeCollectEraseFrameInch_act::setupGamenOn()
{
	Key_SetChangeCollectEraseFrameInch_act_Top.activate( True );
	Key_SetChangeCollectEraseFrameInch_act_Bottom.activate( True );
	Key_SetChangeCollectEraseFrameInch_act_Left.activate( True );
	Key_SetChangeCollectEraseFrameInch_act_Right.activate( True );
	Key_SetChangeCollectEraseFrameInch_act_CheckButton.activate( True );
	Key_SetChangeCollectEraseFrameInchCheck_act_AllSides.activate( True );

	// 背景活性.
	Bak_SetChangeCollectEraseFrameInch_act_Top.activate( True );
	Bak_SetChangeCollectEraseFrameInch_act_Bottom.activate( True );
	Bak_SetChangeCollectEraseFrameInch_act_Left.activate( True );
	Bak_SetChangeCollectEraseFrameInch_act_Right.activate( True );
	b_FunctionOffOnFlg = True;

	if( bl_EraseOn[ss_SelPos] ) {
		Key_SetChangeCollectEraseFrameInch_act_CheckButton.chkOn();
		if( True == m_bl_isSelectedInputField ) {
			Cmp_SetChangeCollectEraseFrameInch_act_UpDown.changeButtonActivate( PNLE_UpDownInputAreaBtnActivateStatus_Active );		// アップダウンコンポーネント表示活性化状態切り替え (選択表示).
		} else {
			Cmp_SetChangeCollectEraseFrameInch_act_UpDown.changeButtonActivate( PNLE_UpDownInputAreaBtnActivateStatus_Normal );		// アップダウンコンポーネント表示活性化状態切り替え (表示).
		}
	} else {
		Key_SetChangeCollectEraseFrameInch_act_CheckButton.chkOff();
		Cmp_SetChangeCollectEraseFrameInch_act_UpDown.changeButtonActivate( PNLE_UpDownInputAreaBtnActivateStatus_Disable );

	}

	if( 0 == ss_SelPos ) {
		setupGamenAllSide();		// 枠全体選択時のセットアップ.
	} else {
		setupGamenIndividual();		// 個別選択時のセットアップ.
	}

	// 数値入力コンポーネントエラー解除.
	if( True == Cmp_SetChangeCollectEraseFrameInch_act_UpDown.isErrorInfo() ) {
		if( EP_ErrorPartsKind_ErrorOff == onlyPanel.getErrorNumberFieldPartsKind() ) {	//エラー自動解除判定.
			onlyPanel.setErrorNumberFieldPartsKind( EP_ErrorPartsKind_Non );
			Cmp_SetChangeCollectEraseFrameInch_act_UpDown.ReleaseError();
		}
	}
}

//------------------------------------------------------------------------------
/**
* @brief A070800120 枠消し : セットアップ（OFF選択時）.
*/
//------------------------------------------------------------------------------
void PCPC_SetChangeCollectEraseFrameInch_act::setupGamenOff()
{
	Key_SetChangeCollectEraseFrameInch_act_Top.activate( False );
	Key_SetChangeCollectEraseFrameInch_act_Bottom.activate( False );
	Key_SetChangeCollectEraseFrameInch_act_Left.activate( False );
	Key_SetChangeCollectEraseFrameInch_act_Right.activate( False );
	Cmp_SetChangeCollectEraseFrameInch_act_UpDown.changeButtonActivate( PNLE_UpDownInputAreaBtnActivateStatus_Disable );
	Key_SetChangeCollectEraseFrameInch_act_CheckButton.activate( False );
	Key_SetChangeCollectEraseFrameInchCheck_act_AllSides.activate( False );

	// 枠消し量のセットアップ.
	setupGamenEraseValue();

	// 枠全体キーのチェックボックス状態.
	if( 0 == ss_SelPos ) {
		Key_SetChangeCollectEraseFrameInchCheck_act_AllSides.chkOn();
	} else {
		Key_SetChangeCollectEraseFrameInchCheck_act_AllSides.chkOff();
		// 背景非活性.
		Bak_SetChangeCollectEraseFrameInch_act_Top.activate( False );
		Bak_SetChangeCollectEraseFrameInch_act_Bottom.activate( False );
		Bak_SetChangeCollectEraseFrameInch_act_Left.activate( False );
		Bak_SetChangeCollectEraseFrameInch_act_Right.activate( False );
	}

	if( True == bl_EraseOn[ss_SelPos] ) {
		Key_SetChangeCollectEraseFrameInch_act_CheckButton.chkOn();
	} else {
		Key_SetChangeCollectEraseFrameInch_act_CheckButton.chkOff();
	}

	Key_SetChangeCollectEraseFrameInch_actOK.activate( True );
	if( b_FunctionOffOnFlg != Cmp_SetChangeCollectEraseFrameInch_act_MessageArea.getOnOffSwitchButtonState() ) {
		if( ( TYPE_UnitSystemInch_Fraction != PnlComGetUnitSystem() ) && ( False == PnlComIsHard10Key() ) ) {
			m_bl_isSelectedInputField = False;
		}
		// MTUIテンキー非表示.
		hide10KeyPopup();
	}
	b_FunctionOffOnFlg = False;

	return;
}

//------------------------------------------------------------------------------
/**
* @brief A070800120 枠消し : セットアップ（枠全体）
*/
//------------------------------------------------------------------------------
void PCPC_SetChangeCollectEraseFrameInch_act::setupGamenAllSide()
{
	bl_EraseOn[0] = True;
	Key_SetChangeCollectEraseFrameInch_act_CheckButton.activate( False );
	Key_SetChangeCollectEraseFrameInchCheck_act_AllSides.chkOn();
	// 各位置キーは非活性.
	Key_SetChangeCollectEraseFrameInch_act_Top.activate( False );		// 分岐判定 : 各位置キーの設定.
	Key_SetChangeCollectEraseFrameInch_act_Bottom.activate( False );
	Key_SetChangeCollectEraseFrameInch_act_Left.activate( False );
	Key_SetChangeCollectEraseFrameInch_act_Right.activate( False );
	// 数値・単位は非表示.
	setupGamenEraseValue();

	Bool bl_isActivateOk = True;	// 伊丹Sky(坂梨)  2011/05/20 CQ340544.
// Comment 2020/05/12 HCL-Satheesh [IT6_HMLK][OP_BTS-23329] When interger value entered convert in to float and use for screen display.
// comment 2022/12/09 FPT_LinhNV46 [IT6_2.3][OP_BTS-40861] Converted integer value from GUI.
//	if( !b_isFloatMode && ( PnlComGetUnitSystem() == TYPE_UnitSystemInch_Number ) ) {
//		sf_EraseValue[0] = Cmp_SetChangeCollectEraseFrameInch_act_UpDown.getNum() * 25.400f;
//	} else {
//		sf_EraseValue[0] = Cmp_SetChangeCollectEraseFrameInch_act_UpDown.getNum();
//	}
		sf_EraseValue[0] = Cmp_SetChangeCollectEraseFrameInch_act_UpDown.getNum();
	if( 0.0f >= sf_EraseValue[0] ) {
		bl_isActivateOk = False;	// ブランク時はOKキーグレイアウト // comment MinervaZeus Sky小尾 2011/06/24 [CQ339112].
	}
	Key_SetChangeCollectEraseFrameInch_actOK.activate( bl_isActivateOk );
}

//------------------------------------------------------------------------------
/**
* @brief A070800120 枠消し : セットアップ（個別）.
*/
//------------------------------------------------------------------------------
void PCPC_SetChangeCollectEraseFrameInch_act::setupGamenIndividual()
{
	Key_SetChangeCollectEraseFrameInchCheck_act_AllSides.chkOff();
// Comment 2020/05/12 HCL-Satheesh [IT6_HMLK][OP_BTS-23329] When interger value entered convert in to float and use for screen display.
// comment 2022/12/09 FPT_LinhNV46 [IT6_2.3][OP_BTS-40861] Converted integer value from GUI.
//	if( !b_isFloatMode && ( PnlComGetUnitSystem() == TYPE_UnitSystemInch_Number ) ) {
//		sf_EraseValue[ss_SelPos] = Cmp_SetChangeCollectEraseFrameInch_act_UpDown.getNum() * 25.400f;
//	} else {
//		sf_EraseValue[ss_SelPos] = Cmp_SetChangeCollectEraseFrameInch_act_UpDown.getNum();
//	}
		sf_EraseValue[ss_SelPos] = Cmp_SetChangeCollectEraseFrameInch_act_UpDown.getNum();
	
	Bool bl_PosSelect[PNLE_AuxilEraseFrame_Pos_MAX-1] = {False};
	bl_PosSelect[ss_SelPos-1] = True;
	Bool bl_isActivateOk = True;	// 伊丹Sky(坂梨)  2011/05/20 CQ340544.
	// 全ての位置が消去しない場合はOKをグレイアウト.
	if( ( False == bl_EraseOn[PNLE_AuxilEraseFrame_Pos_TOP] ) &&
	    ( False == bl_EraseOn[PNLE_AuxilEraseFrame_Pos_BOTTOM] ) &&
	    ( False == bl_EraseOn[PNLE_AuxilEraseFrame_Pos_LEFT] ) &&
	    ( False == bl_EraseOn[PNLE_AuxilEraseFrame_Pos_RIGHT] ) ) {
		bl_isActivateOk = False;
	}

	setupGamenEraseValue();

	for( int i = 1; i < PNLE_AuxilEraseFrame_Pos_MAX; i++ ) {
		switch( i ) {		// 条件分岐：枠の位置.
			// 分岐判定：上.
			case 1:
				if( bl_EraseOn[1] ) {
					if( sf_EraseValue[i] > 0.0f ) {
						Nfd_SetChangeCollectEraseFrameInch_act_Top.set( sf_EraseValue[i], True );
					} else {
						Nfd_SetChangeCollectEraseFrameInch_act_Top.set( sf_EraseValue[i], False );
						bl_isActivateOk = False;	// ブランク時はOKキーグレイアウト // comment MinervaZeus Sky小尾 2011/06/24 [CQ339112].
					}
					Nfd_SetChangeCollectEraseFrameInch_act_Top.changeLayoutX( TYPE_LayoutRightAdjust );
				} else {
					Nfd_SetChangeCollectEraseFrameInch_act_Top.changeLayoutX( TYPE_LayoutLeftAdjust );
					Nfd_SetChangeCollectEraseFrameInch_act_Top.changeText( s_pITMNotErase_W132H024A );
				}
				Key_SetChangeCollectEraseFrameInch_act_Top.select( bl_PosSelect[i-1] );
				break;
			// 分岐判定：下.
			case 2:
				if( bl_EraseOn[2] ) {
					if( sf_EraseValue[i] > 0.0f ) {
						Nfd_SetChangeCollectEraseFrameInch_act_Bottom.set( sf_EraseValue[i], True );
					} else {
						Nfd_SetChangeCollectEraseFrameInch_act_Bottom.set( sf_EraseValue[i], False );
						bl_isActivateOk = False;	// ブランク時はOKキーグレイアウト // comment MinervaZeus Sky小尾 2011/06/24 [CQ339112].
					}
					Nfd_SetChangeCollectEraseFrameInch_act_Bottom.changeLayoutX( TYPE_LayoutRightAdjust );
				} else {
					Nfd_SetChangeCollectEraseFrameInch_act_Bottom.changeLayoutX( TYPE_LayoutLeftAdjust );
					Nfd_SetChangeCollectEraseFrameInch_act_Bottom.changeText( s_pITMNotErase_W132H024A );
				}
				Key_SetChangeCollectEraseFrameInch_act_Bottom.select( bl_PosSelect[i-1] );
				break;
			// 分岐判定：左.
			case 3:
				if( bl_EraseOn[3] ) {
					if( sf_EraseValue[i] > 0.0f ) {
						Nfd_SetChangeCollectEraseFrameInch_act_Left.set( sf_EraseValue[i], True );
					} else {
						Nfd_SetChangeCollectEraseFrameInch_act_Left.set( sf_EraseValue[i], False );
						bl_isActivateOk = False;	// ブランク時はOKキーグレイアウト // comment MinervaZeus Sky小尾 2011/06/24 [CQ339112].
					}
					Nfd_SetChangeCollectEraseFrameInch_act_Left.changeLayoutX( TYPE_LayoutRightAdjust );
				} else {
					Nfd_SetChangeCollectEraseFrameInch_act_Left.changeLayoutX( TYPE_LayoutLeftAdjust );
					Nfd_SetChangeCollectEraseFrameInch_act_Left.changeText( s_pITMNotErase_W132H024A );
				}
				Key_SetChangeCollectEraseFrameInch_act_Left.select( bl_PosSelect[i-1] );
				break;
			// 分岐判定：右.
			case 4:
				if( bl_EraseOn[4] ) {
					if( sf_EraseValue[i] > 0.0f ) {
						Nfd_SetChangeCollectEraseFrameInch_act_Right.set( sf_EraseValue[i], True );
					} else {
						Nfd_SetChangeCollectEraseFrameInch_act_Right.set( sf_EraseValue[i], False );
						bl_isActivateOk = False;	// ブランク時はOKキーグレイアウト // comment MinervaZeus Sky小尾 2011/06/24 [CQ339112].
					}
					Nfd_SetChangeCollectEraseFrameInch_act_Right.changeLayoutX( TYPE_LayoutRightAdjust );
				} else {
					Nfd_SetChangeCollectEraseFrameInch_act_Right.changeLayoutX( TYPE_LayoutLeftAdjust );
					Nfd_SetChangeCollectEraseFrameInch_act_Right.changeText( s_pITMNotErase_W132H024A );
				}
				Key_SetChangeCollectEraseFrameInch_act_Right.select( bl_PosSelect[i-1] );
				break;
		}
	}
	Key_SetChangeCollectEraseFrameInch_actOK.activate( bl_isActivateOk );
}

//------------------------------------------------------------------------------
/**
* @brief A070800120 枠消し : セットアップ（枠消し量）.
*/
//------------------------------------------------------------------------------
void PCPC_SetChangeCollectEraseFrameInch_act::setupGamenEraseValue()
{
	// 枠全体の場合非表示.
	Bool bl_ShowValue = False;
	Bool bl_ShowMmUnit = False;
	if( 0 != ss_SelPos ) {	// 個別の場合.
		bl_ShowValue = True;
		if( TYPE_UnitSystemMM_Number == m_e_Unit ) {
			bl_ShowMmUnit = True;	// 単位表示.
		}
	}
	// 数値・単位表示.
	Nfd_SetChangeCollectEraseFrameInch_act_Top.showEnable( bl_ShowValue );
	Nfd_SetChangeCollectEraseFrameInch_act_Bottom.showEnable( bl_ShowValue );
	Nfd_SetChangeCollectEraseFrameInch_act_Left.showEnable( bl_ShowValue );
	Nfd_SetChangeCollectEraseFrameInch_act_Right.showEnable( bl_ShowValue );
	Txt_SetChangeCollectEraseFrameInch_act_TopMM.showEnable( ( True == bl_ShowMmUnit ) && ( True == bl_EraseOn[PNLE_AuxilEraseFrame_Pos_TOP] ) );
	Txt_SetChangeCollectEraseFrameInch_act_BottomMM.showEnable( ( True == bl_ShowMmUnit ) && ( True == bl_EraseOn[PNLE_AuxilEraseFrame_Pos_BOTTOM] ) );
	Txt_SetChangeCollectEraseFrameInch_act_LeftMM.showEnable( ( True == bl_ShowMmUnit ) && ( True == bl_EraseOn[PNLE_AuxilEraseFrame_Pos_LEFT] ) );
	Txt_SetChangeCollectEraseFrameInch_act_RightMM.showEnable( ( True == bl_ShowMmUnit ) && ( True == bl_EraseOn[PNLE_AuxilEraseFrame_Pos_RIGHT] ) );
}

//------------------------------------------------------------------------------
/**
* @brief A070800120 枠消し : ボタン押下
*/
//------------------------------------------------------------------------------
void PCPC_SetChangeCollectEraseFrameInch_act::procButtonOn( uchar option )
{
	short ss_NextPos = ss_SelPos;

	if( True == isMessageActive( &Cmp_SetChangeCollectEraseFrameInch_act_UpDown ) ) {
		// コンポーネントのキー処理.
	}
	else {
		switch( option ) {		// 条件分岐 : 各位置キー.
			case PNLD_SETCHANGECOLLECTERASEFRAMEINCH_ACT_ALLSIDES:		// 分岐判定：枠全体.
				if( PNLD_SETCHANGECOLLECTERASEFRAMEINCH_ACT_ALLSIDES != ss_SelPos ) {
					if( ( ( sf_MinValue > sf_EraseValue[ss_SelPos] ) ||
					      ( False == bl_EraseOn[ss_SelPos] ) ) ) {
// Comment 2020/05/12 HCL-Satheesh [IT6_HMLK][OP_BTS-23329] When interger value entered convert in to float and use for screen display.
					    Cmp_SetChangeCollectEraseFrameInch_act_UpDown.setInchFloatStatus(True);
						Cmp_SetChangeCollectEraseFrameInch_act_UpDown.Initialize( &sf_FrameEraseValue, sf_MinValue, sf_MaxValue, m_e_Unit );
					}
					ss_NextPos = 0;
					bl_EraseOn[0] = True;
				} else {
					ss_NextPos = PNLE_AuxilEraseFrame_Pos_TOP;				// タブ切り替え時の選択位置は上固定とする.
					setClearFrameErase();
				}
				if( ( False == PnlComIsHard10Key() ) && ( TYPE_UnitSystemInch_Fraction != PnlComGetUnitSystem() ) ) {
					m_bl_isSelectedInputField = False;
				}
				// MTUIテンキー非表示.
				hide10KeyPopup();
				break;
			case PNLD_SETCHANGECOLLECTERASEFRAMEINCH_ACT_TOP:		// 分岐判定：上.
			case PNLD_SETCHANGECOLLECTERASEFRAMEINCH_ACT_BOTTOM:	// 分岐判定：下.
			case PNLD_SETCHANGECOLLECTERASEFRAMEINCH_ACT_LEFT:		// 分岐判定：左.
			case PNLD_SETCHANGECOLLECTERASEFRAMEINCH_ACT_RIGHT:		// 分岐判定：右.
// Comment 2020/05/12 HCL-Satheesh [IT6_HMLK][OP_BTS-23329] When interger value entered convert in to float and use for screen display.
// comment 2022/12/09 FPT_LinhNV46 [IT6_2.3][OP_BTS-40861] Converted integer value from GUI.
//				b_isFloatMode = True;
				if( ( PNLD_SETCHANGECOLLECTERASEFRAMEINCH_ACT_ALLSIDES != ss_SelPos )
				    && ( ( option - 1 ) != ss_SelPos ) ) {
					// 消去量未入力で現在位置以外を選択した場合は拒否.
					if( ( PNLE_AuxilEraseFrame_OFF == sf_EraseValue[ss_SelPos] ) && ( True == bl_EraseOn[ss_SelPos] ) &&
					    ( ss_SelPos != ( option-1 ) ) ) {
						setBuzzerType( PCGE_BuzzerType_InputNg );
					} else {
						ss_NextPos = static_cast<short>( option-1 );
						if( ss_NextPos != ss_SelPos ) {
// Comment 2020/05/12 HCL-Satheesh [IT6_HMLK][OP_BTS-23329] When interger value entered convert in to float and use for screen display.
							Cmp_SetChangeCollectEraseFrameInch_act_UpDown.setInchFloatStatus(True);
							if( PNLE_AuxilEraseFrame_OFF == sf_EraseValue[ss_NextPos] ) {
								Cmp_SetChangeCollectEraseFrameInch_act_UpDown.Initialize( &sf_MinValue, sf_MinValue, sf_MaxValue, m_e_Unit );
							} else {
								Cmp_SetChangeCollectEraseFrameInch_act_UpDown.Initialize( &sf_EraseValue[ss_NextPos], sf_MinValue, sf_MaxValue, m_e_Unit );
							}
						}
						if( True != bl_EraseOn[ss_NextPos] ) {
							if( ( TYPE_UnitSystemInch_Fraction != PnlComGetUnitSystem() ) && ( False == PnlComIsHard10Key() ) ) {
								m_bl_isSelectedInputField = False;
							}
							// MTUIテンキー非表示.
							hide10KeyPopup();
						}
					}
				} else {	//MTUIv1_ichiyama CQ362426対応.枠全体設定時に上下左右ボタンを押すと受付音が鳴るため,ソフト10キーが呼び出せると勘違いしてしまう.
					setBuzzerType( PCGE_BuzzerType_InputNg );	//無効音を鳴らす.
				}
				break;
			case PNLD_SETCHANGECOLLECTERASEFRAMEINCH_ACT_CHECKBUTTON:		// 分岐判定：消去する.
				if( PNLD_SETCHANGECOLLECTERASEFRAMEINCH_ACT_ALLSIDES == ss_SelPos ) {
					// 受付拒否.
					setBuzzerType( PCGE_BuzzerType_InputNg );
				} else {
					if( True == bl_EraseOn[ss_SelPos] ) {
						if( ( TYPE_UnitSystemInch_Fraction != PnlComGetUnitSystem() ) && ( False == PnlComIsHard10Key() ) ) {
							m_bl_isSelectedInputField = False;
						}
						// MTUIテンキー非表示.
						hide10KeyPopup();
//comment 2023/1/4 FPT.KhaiPQ	[OP_BTS-40915][IT6_2.3]	スキャンのプレビュー画面に枠消し：ON、削去量が「10.0mm」を設定する時に、「OK」キーを押下し、無効音を出す問題対策.
						sf_FrameEraseValue = sf_EraseValue[ss_SelPos];

					}
					bl_EraseOn[ss_SelPos] = !bl_EraseOn[ss_SelPos];
				}
				break;
			case PNLD_SETCHANGECOLLECTERASEFRAMEINCH_ACT_INPUTAREA:
				m_bl_isSelectedInputField = True;	// comment 2012/06/21 OPS1川手 [MinervaBK][CQ365578]ソフト10キーUD対応 No.7 [数値入力エリアの選択状態更新].
				if( True != PLBC_MultiTouchFunction::isEnable()
				    &&	 True == PLBC_FuncVersionCheck::isOK_Type4() ) {
					// 旧SWテンキー表示.
					procSoft10Key( option );
				}
				break;
			case PNLE_UpDownNumInputArea_KeyPlus:				//!< キー[+].
			case PNLE_UpDownNumInputArea_KeyMinus:				//!< キー[-].
				m_bl_isSelectedInputField = True;
				break;
			default:
				LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
				return;
		}
	}
	if( ss_NextPos != ss_SelPos ) {		// 条件分岐：変更有り.
		ss_SelPosLast = ss_SelPos;
		ss_SelPos = ss_NextPos;
	}
	setupGamen( 0 );					// 伊丹Sky(坂梨)  2011/05/20 CQ340544.
}
//------------------------------------------------------------------------------
/**
* @brief A070800120 折り目消し : スタートキー押下処理.
*/
//------------------------------------------------------------------------------
void PCPC_SetChangeCollectEraseFrameInch_act::procStart( uchar option )
{
	checkSoft10KeyData();
	if( True == checkValue() ) {
		if( ( PNLE_APPMODE_SCANFAX == PnlComGetAppMode() ) && ( False == comPanelJob->getJobSpec()->pc_Scan->c_MemoryIn.getMemoryInContinue() ) ) {
//Comment 2019/12/12 SCD2 K.Kasemura  [EagleZ+][OP_BTS-15196] 送信中ジョブが無いにもかかわらず、送信中ジョブの警告が表示される.
//			SP_StatusCheck  sp_StatusCheck = isMachineStatusChack();  // 送信中ステータスのチェック結果.
			if( ( True == PnlComIsEnableSearchablePDFAutoExtrFileNameConfirm( comPanelJob ) )
//			    && ( True == sp_StatusCheck.bl_NowTrans ) ) {
				&& ( True == isFaxJobStatusCheck() ) ) {
				PNLC_PopupListControl::newInstance()->addPopup( &gp_ConfirmContinualOperation_act_warn );
				return;
			}
		}
		PNLC_ActionDataKeyControlLib::newInstance()->procStartActionReadStop( option );
	} else {
		onlyPanel.setErrorNumberFieldPartsKind( EP_ErrorPartsKind_NumberField );
		if( sf_MinValue > Cmp_SetChangeCollectEraseFrameInch_act_UpDown.getNum() ) {
			Cmp_SetChangeCollectEraseFrameInch_act_UpDown.setNum( sf_MinValue - 1 );
		}
		setupGamen();
	}
}

//------------------------------------------------------------------------------
/**
* @brief A070800120 枠消し : ストップキー押下処理.
*/
//------------------------------------------------------------------------------
void PCPC_SetChangeCollectEraseFrameInch_act::procStop( uchar option )
{
	PNLC_ActionDataKeyControlLib* pc_ActionDataKeyControlLib = PNLC_ActionDataKeyControlLib::newInstance();
	pc_ActionDataKeyControlLib->procStopActionReadStop( option );
}

//------------------------------------------------------------------------------
/**
* @brief A070800120 枠消し : Okキー
*/
//------------------------------------------------------------------------------
void PCPC_SetChangeCollectEraseFrameInch_act::procOk( uchar option )
{
	TYPC_OriginalErase* pc_OriginalErase = &( comPanelJob->getJobSpec()->pc_Scan->c_OriginalErase );
	if( False == Cmp_SetChangeCollectEraseFrameInch_act_MessageArea.getOnOffSwitchButtonState() ) {		// 条件分岐 : ON/OFF状態, 枠消状態更新.
		cancel( CNK_FrameErase, CNK_Size, comPanelJob );		// 分岐判定 : 枠消し設定を行わないことを指定する.
	} else {
		if( checkValue() ) {
			if( 0 == ss_SelPos ) {		// 条件分岐 : 枠全体.
				pc_OriginalErase->setTop( sf_EraseValue[0] );
				pc_OriginalErase->setLeft( sf_EraseValue[0] );
				pc_OriginalErase->setRight( sf_EraseValue[0] );
				pc_OriginalErase->setBottom( sf_EraseValue[0] );
				pc_OriginalErase->setFlameOrEachSideErase( True );
			} else {
				// 分岐判定 : 個別.
				sf_EraseValue[1] = Nfd_SetChangeCollectEraseFrameInch_act_Top.getNumber();		// OUTPUT : 消去量のONを設定する.
				sf_EraseValue[2] = Nfd_SetChangeCollectEraseFrameInch_act_Bottom.getNumber();
				sf_EraseValue[3] = Nfd_SetChangeCollectEraseFrameInch_act_Left.getNumber();
				sf_EraseValue[4] = Nfd_SetChangeCollectEraseFrameInch_act_Right.getNumber();
				for( uchar i = 1; i <= 4; i++ ) {		// OUTPUT : 消去量のOFFを設定する。.
					if( False == bl_EraseOn[i] ) {
						sf_EraseValue[i] = 0.0f;
					}
				}
				pc_OriginalErase->setTop( sf_EraseValue[1] );		// OUTPUT : 枠消し設定を個別に指定する.
				pc_OriginalErase->setBottom( sf_EraseValue[2] );
				pc_OriginalErase->setLeft( sf_EraseValue[3] );
				pc_OriginalErase->setRight( sf_EraseValue[4] );
				pc_OriginalErase->setFlameOrEachSideErase( False );
			}
		} else {
			setupGamen();
			return;				// 伊丹Sky(坂梨)  2011/05/20 CQ340544.
		}
	}
	PPopupDecisionButtonCom::procOk( option );
// Comment 2020/05/12 HCL-Satheesh [IT6_HMLK][OP_BTS-23329] When interger value entered convert in to float and use for screen display.
// comment 2022/12/09 FPT_LinhNV46 [IT6_2.3][OP_BTS-40861] Converted integer value from GUI.
//	b_isFloatMode = True;
//	Cmp_SetChangeCollectEraseFrameInch_act_UpDown.setInchFloatStatus(True);
}

//------------------------------------------------------------------------------
/**
* @brief A070800120 枠消し : リセットキー.
*/
//------------------------------------------------------------------------------
void PCPC_SetChangeCollectEraseFrameInch_act::procReset( uchar option )
{
	// 変更前の状態に復帰させる.
	if( True == comPanelJob->getJobSpec()->pc_Common->c_JobType.getProgramJobOn() ) {
		c_ActSettingChangeSampleCopy.PnlRecoverySettingSampleCopySpecInfo( comPanelJob->getJobSpec(), comPanelJob->getJobStatus() );
	} else {
		c_SettingChengeBasicInfo.PnlRecoverySettingBasicInfo( 1 );
	}
	PNLC_PopupListControl::newInstance()->closeTop( True );
}

// comment 2020/07/14 SCD2-Sky 石原 [IT6_2.0MR2][OP_BTS-25362] フッターエリア対応.
//------------------------------------------------------------------------------
/**
* @brief A070800120 枠消し : リセットキー活性状態.
*/
//------------------------------------------------------------------------------
Bool PCPC_SetChangeCollectEraseFrameInch_act::resetKeyActiveCheck()
{
	ACTIVEKEY_RET( False );
}
//------------------------------------------------------------------------------
/**
* @brief A070800120 枠消し : クリアキー.
*/
//------------------------------------------------------------------------------
void PCPC_SetChangeCollectEraseFrameInch_act::procClear( uchar option ) {

	if( False == bl_EraseOn[ss_SelPos] ||
	    ( PnlComGetUnitSystem() == TYPE_UnitSystemInch_Fraction ) ) {
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}
	PPopupDecisionButtonCom::procClear( option );
}
//------------------------------------------------------------------------------
/**
* @brief A070800120 枠消し : 10キー.
*/
//------------------------------------------------------------------------------
void PCPC_SetChangeCollectEraseFrameInch_act::proc10Key( uchar option )
{
// Comment 2020/05/12 HCL-Satheesh [IT6_HMLK][OP_BTS-23329] When interger value entered convert in to float and use for screen display.
// comment 2022/12/09 FPT_LinhNV46 [IT6_2.3][OP_BTS-40861] Converted integer value from GUI.
//	b_isFloatMode = Cmp_SetChangeCollectEraseFrameInch_act_UpDown.getInchFloatStatus();
	// 単位がインチ分数の場合.
	if( TYPE_UnitSystemInch_Fraction == m_e_Unit ) {
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}
	// 「消去する」がOFFの場合.
	if( False == bl_EraseOn[ss_SelPos] ) {
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}

	// エラー時はブザーを鳴らす.
	if( True == Cmp_SetChangeCollectEraseFrameInch_act_UpDown.isErrorInfo() ) {
		onlyPanel.setErrorNumberFieldPartsKind( EP_ErrorPartsKind_NumberField );
		setBuzzerType( PCGE_BuzzerType_InputNg );
	}
}

//------------------------------------------------------------------------------
/**
* @brief A070800120 枠消し : アクティブ状態の通知.
*/
//------------------------------------------------------------------------------
void PCPC_SetChangeCollectEraseFrameInch_act::procActivate()
{
	// 入力値確定処理.
	checkSoft10KeyData();

	// 親クラスを呼び出し.
	PPopupDecisionButtonCom::procActivate();
}

//------------------------------------------------------------------------------
/**
* @brief A070800120 枠消し : ソフトテンキー.
*/
//------------------------------------------------------------------------------
void PCPC_SetChangeCollectEraseFrameInch_act::procSoft10Key( uchar option )
{
	char sc_No = 0;
	float f_Magtmp;
	Bool bl_ClearOn;
	Bool bl_DirectOn;

	// インチ分数の場合は、ソフトテンキー対応しない.
	if( ( TYPE_UnitSystemInch_Fraction == PnlComGetUnitSystem() )
	    || ( False == m_bl_isSelectedInputField ) ) {
		setBuzzerType( PCGE_BuzzerType_InputNg );								// 受付拒否音.
		return;
	}

	// OFF状態、個別選択で「消去する」キー OFF時は、受付拒否.
	if( ( False == Cmp_SetChangeCollectEraseFrameInch_act_MessageArea.getOnOffSwitchButtonState() )
	    || ( ( False == bl_EraseOn[ ss_SelPos ] ) && ( PNLE_ChangeCollectEraseFrame_Pos_ALL != ss_SelPos ) ) )
	{
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}

	// スライドレス、MTUI有効の場合.
	if( True == PLBC_NonSlideMultiTouchFunction::isEnable() ) {
		// ソフトテンキー表示(センター表示).
// comment 2015/09/16 OPS1 Sky溝部 [操作性改善] テンキー改善対応.
//		PnlComMtuiIf_TenkeyPad::instance()->showDefaultPos();
		PnlComMtuiIf_TenkeyPad::instance()->showDefaultPos( False, 0, 0, 0, 0, PMTE_MtuiIf_TenkeyPattern_Period );
	} else {
		if( True == Cmp_SetChangeCollectEraseFrameInch_act_UpDown.isErrorInfo() ) {	// アップダウンコンポーネントエラーチェック.
			onlyPanel.setErrorNumberFieldPartsKind( EP_ErrorPartsKind_ErrorOff );	// エラー状態セット.
			setupGamen();
		} else {
			PNLC_OutSideKeyCallDataLib::newInstance()->setColorMode( PNLE_Soft10KeyColorModeCommon );	//	画面の背景色.

			switch( ss_SelPos ) {
				case PNLE_ChangeCollectEraseFrame_Pos_ALL:
					sc_No = PNLE_Soft10KeySelected_EraseFrameAct_index1;
					break;
				case PNLE_ChangeCollectEraseFrame_Pos_TOP:
					sc_No = PNLE_Soft10KeySelected_EraseFrameAct_index2;
					break;
				case PNLE_ChangeCollectEraseFrame_Pos_BTM:
					sc_No = PNLE_Soft10KeySelected_EraseFrameAct_index3;
					break;
				case PNLE_ChangeCollectEraseFrame_Pos_LEF:
					sc_No = PNLE_Soft10KeySelected_EraseFrameAct_index4;
					break;
				case PNLE_ChangeCollectEraseFrame_Pos_RGH:
					sc_No = PNLE_Soft10KeySelected_EraseFrameAct_index5;
					break;
				default:
					break;
			}
			PNLC_OutSideKeyCallDataLib::newInstance()->setSelectNo( sc_No );		//	ソフトテンキー中判定用値設定.

			f_Magtmp	= Cmp_SetChangeCollectEraseFrameInch_act_UpDown.getNum();		//!< 現在の値を取得.
			bl_DirectOn = Cmp_SetChangeCollectEraseFrameInch_act_UpDown.isDirectOn();	//!< 数値の変更状態取得.
			bl_ClearOn	= Cmp_SetChangeCollectEraseFrameInch_act_UpDown.isClearOn();	//!< クリア状態取得.
			PNLC_OutSideKeyCallDataLib::newInstance()->setInputNumeric( f_Magtmp, bl_DirectOn, sf_MinValue, sf_MaxValue, bl_ClearOn, "%2.1f", "%1.3f" );

			PNLC_PopupListControl* pc_PopupListControl = PNLC_PopupListControl::newInstance();
			pc_PopupListControl->addPopup( &gp_Soft10Key_Size, this, NULL );
		}
	}
}

//------------------------------------------------------------------------------
/**
* @brief A070800120 枠消し : ソフトテンキー入力値確定処理.
*/
//------------------------------------------------------------------------------
void PCPC_SetChangeCollectEraseFrameInch_act::checkSoft10KeyData()
{
	char sc_No;
	float f_Magtmp;
	Bool bl_NonSetting = False;

	sc_No = PNLC_OutSideKeyCallDataLib::newInstance()->getSelectNo();

	if( PNLE_Soft10KeyUnselected != sc_No ) {
		bl_NonSetting = PNLC_OutSideKeyCallDataLib::newInstance()->getInputNumeric( &f_Magtmp );

		if( True == bl_NonSetting ) {
			// 未設定の場合は、何もしない.
		} else {
			Cmp_SetChangeCollectEraseFrameInch_act_UpDown.setNum( f_Magtmp );
			Cmp_SetChangeCollectEraseFrameInch_act_UpDown.setDirectOn( False );
			Cmp_SetChangeCollectEraseFrameInch_act_UpDown.setClearOn( False );

			switch( sc_No ) {
				case PNLE_Soft10KeySelected_EraseFrameAct_index1:
					sf_EraseValue[PNLE_ChangeCollectEraseFrame_Pos_ALL] = f_Magtmp;
					break;
				case PNLE_Soft10KeySelected_EraseFrameAct_index2:
					sf_EraseValue[PNLE_ChangeCollectEraseFrame_Pos_TOP] = f_Magtmp;
					break;
				case PNLE_Soft10KeySelected_EraseFrameAct_index3:
					sf_EraseValue[PNLE_ChangeCollectEraseFrame_Pos_BTM] = f_Magtmp;
					break;
				case PNLE_Soft10KeySelected_EraseFrameAct_index4:
					sf_EraseValue[PNLE_ChangeCollectEraseFrame_Pos_LEF] = f_Magtmp;
					break;
				case PNLE_Soft10KeySelected_EraseFrameAct_index5:
					sf_EraseValue[PNLE_ChangeCollectEraseFrame_Pos_RGH] = f_Magtmp;
					break;
				default:
					break;
			}
		}

		// データのクリア.
		PNLC_OutSideKeyCallDataLib::newInstance()->setSelectNo( PNLE_Soft10KeyUnselected );
	} else {
		// ソフトテンキー中でない場合は、何もしない.
	}
}

//------------------------------------------------------------------------------.
/**
 *	@brief	A070800120 枠消し : ユーティリティ
 */
//------------------------------------------------------------------------------.
void PCPC_SetChangeCollectEraseFrameInch_act::procUtility( uchar option )
{
	checkSoft10KeyData();
	PPopupDecisionButtonCom::procUtility( option );
}
//------------------------------------------------------------------------------.
/**
 *	@brief	A070800120 枠消し : IDキー処理
 */
//------------------------------------------------------------------------------.
void PCPC_SetChangeCollectEraseFrameInch_act::procIDInputVoice( uchar option )
{
	setBuzzerType( PCGE_BuzzerType_InputNg );
}

//------------------------------------------------------------------------------
/**
* @brief A0201A1000 枠消し : 設定値の最小値確認.
*/
//------------------------------------------------------------------------------
Bool PCPC_SetChangeCollectEraseFrameInch_act::checkValue()
{
	if( PNLD_SETCHANGECOLLECTERASEFRAMEINCH_ACT_ALLSIDES == ss_SelPos ) {
		//枠全体.
		if( bl_EraseOn[0] ) {
// Comment 2020/05/12 HCL-Satheesh [IT6_HMLK][OP_BTS-23329] When minimum value enter allow OK button.
//			if( sf_EraseValue[0] < sf_MinValue ) {
			if( (fabs(sf_EraseValue[0]-sf_MinValue) > 0.001f ) && sf_EraseValue[0] < sf_MinValue ) {
// comment 2022/12/09 FPT_LinhNV46 [IT6_2.3][OP_BTS-40861] Converted integer value from GUI.
				Cmp_SetChangeCollectEraseFrameInch_act_UpDown.setInchFloatStatus(True);
				Cmp_SetChangeCollectEraseFrameInch_act_UpDown.Initialize( sf_EraseValue, sf_MinValue, sf_MaxValue, m_e_Unit );
				Cmp_SetChangeCollectEraseFrameInch_act_UpDown.setNum( sf_EraseValue[0] );
				onlyPanel.setErrorNumberFieldPartsKind( EP_ErrorPartsKind_NumberField );
				return False;
			}
		}
	} else {
		// 個別.
		sf_EraseValue[1] = Nfd_SetChangeCollectEraseFrameInch_act_Top.getNumber();
		sf_EraseValue[2] = Nfd_SetChangeCollectEraseFrameInch_act_Bottom.getNumber();
		sf_EraseValue[3] = Nfd_SetChangeCollectEraseFrameInch_act_Left.getNumber();
		sf_EraseValue[4] = Nfd_SetChangeCollectEraseFrameInch_act_Right.getNumber();
		for( uchar i = 1; i <= 4; i++ ) {
			if( True == bl_EraseOn[i] ) {
// Comment 2020/05/12 HCL-Satheesh [IT6_HMLK][OP_BTS-23329] When minimum value enter allow OK button.
//				if( sf_EraseValue[i] < sf_MinValue ) {
// comment 2023/02/15 FPT_LinhNV46 [IT6_2.3][OP_BTS-41851].
				if( (fabs(sf_EraseValue[i]-sf_MinValue) > 0.001f ) && sf_EraseValue[i] < sf_MinValue ) {
// comment 2022/12/09 FPT_LinhNV46 [IT6_2.3][OP_BTS-40861] Converted integer value from GUI.
					Cmp_SetChangeCollectEraseFrameInch_act_UpDown.setInchFloatStatus(True);
					Cmp_SetChangeCollectEraseFrameInch_act_UpDown.Initialize( &sf_EraseValue[i], sf_MinValue, sf_MaxValue, m_e_Unit );
					Cmp_SetChangeCollectEraseFrameInch_act_UpDown.setNum( sf_EraseValue[i] );
					sf_EraseValue[i] = sf_MinValue;
					onlyPanel.setErrorNumberFieldPartsKind( EP_ErrorPartsKind_NumberField );
					setBuzzerType( PCGE_BuzzerType_InputNg );
					return False;
				} else if( sf_MaxValue < sf_EraseValue[i] ) {
					sf_EraseValue[i] = sf_MaxValue;
					onlyPanel.setErrorNumberFieldPartsKind( EP_ErrorPartsKind_NumberField );
					setBuzzerType( PCGE_BuzzerType_InputNg );
					return False;
				}
			} else {
				sf_EraseValue[i] = 0.0f;
			}
		}
	}

	return True;
}

//------------------------------------------------------------------------------
/**
* @brief A070800120 枠消し : クリアした消去データの設定.
*/
//------------------------------------------------------------------------------
void PCPC_SetChangeCollectEraseFrameInch_act::setClearFrameErase()
{
	int i = 0;
// Comment 2020/05/12 HCL-Satheesh [IT6_HMLK][OP_BTS-23329] When interger value entered convert in to float and use for screen display.
// comment 2022/12/09 FPT_LinhNV46 [IT6_2.3][OP_BTS-40861] Converted integer value from GUI.
//	b_isFloatMode = True;
	for( i = 1; i < PNLE_AuxilEraseFrame_Pos_MAX; i++ ) {
		switch( i ) {
			case PNLE_AuxilEraseFrame_Pos_TOP:
				if( sf_MinValue > sf_EraseValue[PNLE_AuxilEraseFrame_Pos_ALL] ) {
					sf_EraseValue[i] = sf_TopEraseValue;
					if( PNLE_AuxilEraseFrame_OFF == sf_EraseValue[i] ) {
						bl_EraseOn[i] = False;
					} else {
						bl_EraseOn[i] = True;
					}
				} else {
// Comment 2020/05/12 HCL-Satheesh [IT6_HMLK][OP_BTS-23329] When interger value entered convert in to float and use for screen display.
// comment 2022/12/09 FPT_LinhNV46 [IT6_2.3][OP_BTS-40861] Converted integer value from GUI.
//					if( !b_isFloatMode&& ( PnlComGetUnitSystem() == TYPE_UnitSystemInch_Number )) {
//						sf_EraseValue[i] = sf_EraseValue[PNLE_AuxilEraseFrame_Pos_ALL] * 25.400f;
//					} else {
//						sf_EraseValue[i] = sf_EraseValue[PNLE_AuxilEraseFrame_Pos_ALL];
//					}
						sf_EraseValue[i] = sf_EraseValue[PNLE_AuxilEraseFrame_Pos_ALL];
					bl_EraseOn[i] = True;
				}
				break;
			case PNLE_AuxilEraseFrame_Pos_BOTTOM:
				if( sf_MinValue > sf_EraseValue[PNLE_AuxilEraseFrame_Pos_ALL] ) {
					sf_EraseValue[i] = sf_BottomEraseValue;
					if( PNLE_AuxilEraseFrame_OFF == sf_EraseValue[i] ) {
						bl_EraseOn[i] = False;
					} else {
						bl_EraseOn[i] = True;
					}
				} else {
// Comment 2020/05/12 HCL-Satheesh [IT6_HMLK][OP_BTS-23329] When interger value entered convert in to float and use for screen display.
// comment 2022/12/09 FPT_LinhNV46 [IT6_2.3][OP_BTS-40861] Converted integer value from GUI.
//					if( !b_isFloatMode&& ( PnlComGetUnitSystem() == TYPE_UnitSystemInch_Number )) {
//						sf_EraseValue[i] = sf_EraseValue[PNLE_AuxilEraseFrame_Pos_ALL]* 25.400f;
//					} else {
//						sf_EraseValue[i] = sf_EraseValue[PNLE_AuxilEraseFrame_Pos_ALL];
//					}
						sf_EraseValue[i] = sf_EraseValue[PNLE_AuxilEraseFrame_Pos_ALL];
					bl_EraseOn[i] = True;
				}
				break;
			case PNLE_AuxilEraseFrame_Pos_LEFT:
				if( sf_MinValue > sf_EraseValue[PNLE_AuxilEraseFrame_Pos_ALL] ) {
					sf_EraseValue[i] = sf_LeftEraseValue;
					if( PNLE_AuxilEraseFrame_OFF == sf_EraseValue[i] ) {
						bl_EraseOn[i] = False;
					} else {
						bl_EraseOn[i] = True;
					}
				} else {
// Comment 2020/05/12 HCL-Satheesh [IT6_HMLK][OP_BTS-23329] When interger value entered convert in to float and use for screen display.
// comment 2022/12/09 FPT_LinhNV46 [IT6_2.3][OP_BTS-40861] Converted integer value from GUI.
//					if( !b_isFloatMode&& ( PnlComGetUnitSystem() == TYPE_UnitSystemInch_Number )) {
//						sf_EraseValue[i] = sf_EraseValue[PNLE_AuxilEraseFrame_Pos_ALL]* 25.400f;
//					} else {
//						sf_EraseValue[i] = sf_EraseValue[PNLE_AuxilEraseFrame_Pos_ALL];
//					}
						sf_EraseValue[i] = sf_EraseValue[PNLE_AuxilEraseFrame_Pos_ALL];
					bl_EraseOn[i] = True;
				}
				break;
			case PNLE_AuxilEraseFrame_Pos_RIGHT:
				if( sf_MinValue > sf_EraseValue[PNLE_AuxilEraseFrame_Pos_ALL] ) {
					sf_EraseValue[i] = sf_RightEraseValue;
					if( PNLE_AuxilEraseFrame_OFF == sf_EraseValue[i] ) {
						bl_EraseOn[i] = False;
					} else {
						bl_EraseOn[i] = True;
					}
				} else {
// Comment 2020/05/12 HCL-Satheesh [IT6_HMLK][OP_BTS-23329] When interger value entered convert in to float and use for screen display.
// comment 2022/12/09 FPT_LinhNV46 [IT6_2.3][OP_BTS-40861] Converted integer value from GUI.
//				if( !b_isFloatMode&& ( PnlComGetUnitSystem() == TYPE_UnitSystemInch_Number )) { 
//					sf_EraseValue[i] = sf_EraseValue[PNLE_AuxilEraseFrame_Pos_ALL]* 25.400f;
//				} else {
//					sf_EraseValue[i] = sf_EraseValue[PNLE_AuxilEraseFrame_Pos_ALL];
//				}
					sf_EraseValue[i] = sf_EraseValue[PNLE_AuxilEraseFrame_Pos_ALL];
					bl_EraseOn[i] = True;
				}
			default:
				break;
		}
	}
// comment 2022/12/09 FPT_LinhNV46 [IT6_2.3][OP_BTS-40861] Converted integer value from GUI.
	Cmp_SetChangeCollectEraseFrameInch_act_UpDown.setInchFloatStatus(True);
	if( PNLE_AuxilEraseFrame_OFF == sf_EraseValue[PNLE_AuxilEraseFrame_Pos_TOP] ) {
		Cmp_SetChangeCollectEraseFrameInch_act_UpDown.Initialize( &sf_MinValue, sf_MinValue, sf_MaxValue, m_e_Unit );
	} else {
// Comment 2020/05/12 HCL-Satheesh [IT6_HMLK][OP_BTS-23329] When interger value entered convert in to float and use for screen display.
// comment 2023/02/15 FPT_LinhNV46 [IT6_2.3][OP_BTS-41851].
//		if ( PnlComGetUnitSystem() == TYPE_UnitSystemInch_Number ) {
//			sf_EraseValue[PNLE_AuxilEraseFrame_Pos_TOP] = Cmp_SetChangeCollectEraseFrameInch_act_UpDown.getNum(); 
//		}
		Cmp_SetChangeCollectEraseFrameInch_act_UpDown.Initialize( &sf_EraseValue[PNLE_AuxilEraseFrame_Pos_TOP], sf_MinValue, sf_MaxValue, m_e_Unit );
	}
}
/**
 * @brief	MTUIテンキーポップアップ非表示処理.
 * @param	.
 */
void PCPC_SetChangeCollectEraseFrameInch_act::hide10KeyPopup()
{
	if( True == PLBC_NonSlideMultiTouchFunction::isEnable() ) {
		// MTUIテンキー非表示.
		PnlComMtuiIf_TenkeyPad::instance()->hide();
	}
}
// ===============================================================================================.
//	概要	: スライドメニュー
//  引数    : オプション.
//	戻り値	: なし.
// ===============================================================================================.
void PCPC_SetChangeCollectEraseFrameInch_act::procSlideMenu( uchar option )
{
	// 10キー表示可否.
	Bool bl_Show10Key = IsShowSlide10Key();
	PNLC_SlideMenuControlLib::newInstance()->setTenKey( bl_Show10Key );
// comment 2015/09/16 OPS1 Sky溝部 [操作性改善] テンキー改善対応.
	PNLC_SlideMenuControlLib::newInstance()->setTenKeyPattern( PMTE_MtuiIf_TenkeyPattern_Period );

	if( True == bl_Show10Key ) {
		// 10キー入力表示情報設定.
		sendSlide10KeyData( True );
	}
	// スライドメニュー表示.
	PBasicMessageProc::procSlideMenu( option );
}

// ===============================================================================================.
//	概要	: スライドメニューテンキー表示可否.
//  引数    : なし.
//	戻り値	: True：表示可能、False：表示不可.
// ===============================================================================================.
Bool PCPC_SetChangeCollectEraseFrameInch_act::IsShowSlide10Key()
{
	if( ( TYPE_UnitSystemInch_Fraction == PnlComGetUnitSystem() )
	    || ( False == m_bl_isSelectedInputField ) ) {
		return False;
	}

	// OFF状態、個別選択で「消去する」キー OFF時は、受付拒否.
	if( ( False == Cmp_SetChangeCollectEraseFrameInch_act_MessageArea.getOnOffSwitchButtonState() )
	    ||	( ( False == bl_EraseOn[ ss_SelPos ] )
	          && ( PNLE_ChangeCollectEraseFrame_Pos_ALL != ss_SelPos ) ) ) {
		return False;
	}

	return True;
}

// ===============================================================================================.
//	概要	: スライドメニュー入力エリアデータ送信.
//  引数    : True：初期化あり、False：初期化なし.
//	戻り値	: なし.
// ===============================================================================================.
void PCPC_SetChangeCollectEraseFrameInch_act::sendSlide10KeyData( Bool bl_Flag )
{
	float f_Num = 0;			//現在の値.
	Bool bl_DirectOn = False;	//直接入力しているかどうかのフラグ.
	Bool bl_ClearOn = False;	//クリアしているかどうかのフラグ.
	Bool bl_Error = False;
	uchar puc_Dst[PNLD_SLIDEMENU_TEXTBUFFER_SIZE];				//整数部(/小数)変換後格納先.
	uchar puc_Dst_fraction[PNLD_SLIDEMENU_TEXTBUFFER_SIZE];		//分数部変換後格納先.
	PNLD_CURSOR_TYPE e_Cursor_type = PNLD_CURSOR_TYPE_OFF;
	Bool bl_Blink = False;
	memset( puc_Dst,0,sizeof( puc_Dst ) );
	memset( puc_Dst_fraction,0,sizeof( puc_Dst_fraction ) );

	if( True == bl_Flag ) {
		//スライドメニューOpen時の処理.
		PNLC_SoftSlideMenuTenKeyTextDispLib::newInstance()->create( PNLE_OutsideKeyCall_RIGHT_A_SIZE );

// comment 2015/09/16 OPS1 Sky溝部 [操作性改善] テンキー改善対応.
		// 小数入力はコンポーネントの共通処理にてフォーマットを指定するためここでは設定しない.
//		//値を文字列に変更する為の情報を渡す.
//		PNLC_SoftSlideMenuTenKeyConvLib::newInstance()->setParam( PnlComGetUnitSystem(),	//単位系設定.
//		        "%2.1f", 													//MMのフォーマット.
//		        "%1.3f" );													//インチのフォーマット.
	}

	//数値情報の情報.
	f_Num = Cmp_SetChangeCollectEraseFrameInch_act_UpDown.getNum();			// 現在の値を取得.
	bl_ClearOn  = ( True == Cmp_SetChangeCollectEraseFrameInch_act_UpDown.isClearOn() ) ? False : True;		// クリア状態取得.
	bl_DirectOn = Cmp_SetChangeCollectEraseFrameInch_act_UpDown.isDirectOn();		// 入力状態取得.
	bl_Error = Cmp_SetChangeCollectEraseFrameInch_act_UpDown.isErrorInfo();

	if( True == bl_Error )
	{
		//入力エラーの場合は、それを出す.
		const uchar* puc_Str = getTextPartsPointer( s_NumberFieldError );
		if( NULL == puc_Str ) {
			LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
			return;
		}
		strncpy( ( char* )puc_Dst, ( const char* )puc_Str, sizeof( puc_Dst )-1 );
	} else {
		//入力数値を表示文字列に変更.
		PNLC_SoftSlideMenuTenKeyConvLib::newInstance()->getSetText( puc_Dst, puc_Dst_fraction, f_Num, bl_ClearOn, -1 );
		if( True == bl_DirectOn ) {
			//カーソルタイプをリプライスにする.
			e_Cursor_type = PNLD_CURSOR_TYPE_REPLACE;
		} else {
			//カーソルタイプをインサートにする.
			e_Cursor_type = PNLD_CURSOR_TYPE_INSERT;
		}
		bl_Blink = True;
	}

	PNLC_SoftSlideMenuTenKeyTextDispLib::newInstance()->update( puc_Dst,					//文字列.
	        e_Cursor_type,			//カーソルタイプ.
	        ( short )strlen( ( char* )puc_Dst ),	//カーソルポジション.
	        bl_Blink );				//ブリンク.
}

//関数--------------------------------------------------------------------------
//概要    ：片面/両面設定処理.
//備考    ：.
//戻値    ：.
//作成者  ：AWS_ボン.
//変更履歴：.
//------------------------------------------------------------------------------
/**
 * @brief プログラムジョブ読込み停止中/設定変更メニュー/原稿束 : 片面/両面設定処理.
 */
//------------------------------------------------------------------------------
void setPJFaceSelect( const Bool b_OrignalDupMode, const TYPE_DuplexMode e_DuplexMode )
{
	TYPC_Print* pc_Print = comPanelJob->getJobSpec()->pc_Print;
	TYPC_Scan*  pc_Scan  = comPanelJob->getJobSpec()->pc_Scan;
	if( ( NULL == pc_Print ) || ( NULL == pc_Scan ) ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }
	EP_Combination e_CombinationOriginalDupMode;
	EP_Combination e_CombinationDuplexMode;
	e_CombinationOriginalDupMode = ( False == b_OrignalDupMode ) ? CNK_1SideOrig: CNK_2SideOrig ;
	e_CombinationDuplexMode = ( TYPE_DuplexModeOff == e_DuplexMode ) ? CNK_1SideCopy: CNK_2SideCopy;
	// 片面/両面キーと組み合わせ禁止チェック.
// Comment 2019/07/16 【IT6_EagleH】【HCL_Prabakaran】OP_BTS-8347 jobSpec Type has been changed for common use.
//	if( True == combinationCheck( e_CombinationOriginalDupMode, PnlComGetJobCopy(), False, True, e_CombinationDuplexMode )
//	    && True == combinationCheck( e_CombinationDuplexMode, PnlComGetJobCopy(), False, True, e_CombinationOriginalDupMode ) ) {
	if( True == combinationCheck( e_CombinationOriginalDupMode, comPanelJob, False, True, e_CombinationDuplexMode )
	    && True == combinationCheck( e_CombinationDuplexMode, comPanelJob, False, True, e_CombinationOriginalDupMode ) ) {
		// 片面or両面原稿組み合わせ禁止チェック.
		combinationCheck( e_CombinationOriginalDupMode );
		// 片面or両面プリント組み合わせ禁止チェック.
		combinationCheck( e_CombinationDuplexMode );
		pc_Scan->setDuplex( b_OrignalDupMode );
		pc_Print->setDuplexMode( e_DuplexMode );
	}
}
//クラス------------------------------------------------------------------------
//概要    ：画面開始処理.
//備考    ：.
//作成者  ：AWS_ボン.
//変更履歴：.
//------------------------------------------------------------------------------
/**
 * @brief PCPC_PullDownProgramJobSetChangeMenu_actManuscript：画面開始処理.
 */
//------------------------------------------------------------------------------
void PCPC_PullDownProgramJobSetChangeMenu_actManuscript::startGamen()
{
	PPopupPullDownCom::startGamen();		// 親クラスの画面開始処理.
	uchar u_selectItem = setupSelectedManuscriptType();
	setSelectMenuNo( u_selectItem );
}
// comment 2020/07/14 SCD2-Sky 石原 [IT6_2.0MR2][OP_BTS-25362] フッターエリア対応.
//クラス------------------------------------------------------------------------
//概要    ：表示部品のセットアップ.
//備考    ：.
//作成者  ：Sky石原.
//変更履歴：.
//------------------------------------------------------------------------------
/**
 * @brief PCPC_PullDownProgramJobSetChangeMenu_actManuscript：画面開始処理.
 */
//------------------------------------------------------------------------------
void PCPC_PullDownProgramJobSetChangeMenu_actManuscript::setupGamen( short option )
{
	PPopupPullDownCom::setupGamen( option );
#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus)  && defined (DEF_LCD_7INCH) ) )
	PnlComFooterPartsBtnActiveChange(homeKeyActiveCheck(), idKeyActiveCheck(), resetKeyActiveCheck());
#endif		//  ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus)  && defined (DEF_LCD_7INCH) ) )	.
	
}
//関数--------------------------------------------------------------------------
//概要    ：ボタン（押された）.
//備考    ：.
//戻値    ：.
//作成者  ：AWS_ボン.
//変更履歴：.
//------------------------------------------------------------------------------
/**
 * @brief PCPC_PullDownProgramJobSetChangeMenu_actManuscript：ボタン（押された）.
 */
//------------------------------------------------------------------------------
void PCPC_PullDownProgramJobSetChangeMenu_actManuscript::procButtonOn( uchar option )
{
	TYPC_Print* pc_Print = comPanelJob->getJobSpec()->pc_Print;
	if( NULL == pc_Print ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }
	TYPE_DuplexMode e_DuplexMode = pc_Print->getDuplexMode();
	switch( option ) {
		case PNLD_PullDownProgramJobSetChangeMenu_actManuscriptOneSide:
			setPJFaceSelect( False, e_DuplexMode );		// 原稿片面.
			break;
		case PNLD_PullDownProgramJobSetChangeMenu_actManuscriptTwoSide:
			setPJFaceSelect( True, e_DuplexMode );		// 原稿両面.
			break;
		default:	LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );	break;
	}
	uchar u_selectItem = setupSelectedManuscriptType();
	setSelectMenuNo( u_selectItem );
	PNLC_PopupListControl::newInstance()->closeTop( False );		// 普通にポップアップ終了させる.
}

//クラス------------------------------------------------------------------------
//概要    ：選択原稿タイプのセットアップ.
//備考    ：.
//作成者  ：AWS_ボン.
//変更履歴：.
//------------------------------------------------------------------------------
/**
 * @brief PCPC_PullDownProgramJobSetChangeMenu_actManuscript：選択原稿タイプのセットアップ.
 */
//------------------------------------------------------------------------------
uchar PCPC_PullDownProgramJobSetChangeMenu_actManuscript::setupSelectedManuscriptType()
{
	uchar u_selectItem = PNLD_PullDownProgramJobSetChangeMenu_actManuscriptOneSide + 1;
// Comment 2019/07/16 【IT6_EagleH】【HCL_Prabakaran】OP_BTS-8347 jobSpec Type has been changed for common use.
//	if( True == isOn( CNK_2SideOrig, PnlComGetJobCopy() ) ) {
	if( True == isOn( CNK_2SideOrig, comPanelJob ) ) {
		u_selectItem = PNLD_PullDownProgramJobSetChangeMenu_actManuscriptTwoSide + 1;
	}
	Drp_ProgramJobSetChangeMenu_actManuscript.setSelectNo( u_selectItem );
	return u_selectItem;
}
//クラス------------------------------------------------------------------------
//概要    ：画面開始処理.
//備考    ：.
//作成者  ：AWS_ボン.
//変更履歴：.
//------------------------------------------------------------------------------
/**
 * @brief PCPC_PullDownProgramJobSetChangeMenu_actPrint：画面開始処理.
 */
//------------------------------------------------------------------------------
void PCPC_PullDownProgramJobSetChangeMenu_actPrint::startGamen()
{
	PPopupPullDownCom::startGamen();		// 親クラスの画面開始処理.

	uchar u_selectItem = setupSelectedPrintType();
	setSelectMenuNo( u_selectItem );
}
// comment 2020/07/14 SCD2-Sky 石原 [IT6_2.0MR2][OP_BTS-25362] フッターエリア対応.
//クラス------------------------------------------------------------------------
//概要    ：表示部品のセットアップ.
//備考    ：.
//作成者  ：Sky石原.
//変更履歴：.
//------------------------------------------------------------------------------
/**
 * @brief PCPC_PullDownProgramJobSetChangeMenu_actPrint：画面開始処理.
 */
//------------------------------------------------------------------------------
void PCPC_PullDownProgramJobSetChangeMenu_actPrint::setupGamen( short option )
{
	PPopupPullDownCom::setupGamen( option );
#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus)  && defined (DEF_LCD_7INCH) ) )
	PnlComFooterPartsBtnActiveChange(homeKeyActiveCheck(), idKeyActiveCheck(), resetKeyActiveCheck());
#endif		//  ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus)  && defined (DEF_LCD_7INCH) ) )	.
}

//関数--------------------------------------------------------------------------
//概要    ：ボタン（押された）.
//備考    ：.
//戻値    ：.
//作成者  ：AWS_ボン.
//変更履歴：.
//------------------------------------------------------------------------------
/**
 * @brief PCPC_PullDownProgramJobSetChangeMenu_actPrint：ボタン（押された）.
 */
//------------------------------------------------------------------------------
void PCPC_PullDownProgramJobSetChangeMenu_actPrint::procButtonOn( uchar option )
{
	TYPC_Scan* pc_Scan = comPanelJob->getJobSpec()->pc_Scan;
	if( NULL == pc_Scan ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }
	Bool b_OrignalDupMode = pc_Scan->getDuplex();
	switch( option ) {
		case PNLD_PullDownProgramJobSetChangeMenu_actManuscriptOneSide:
			setPJFaceSelect( b_OrignalDupMode, TYPE_DuplexModeOff );		// 印刷片面.
			break;
		case PNLD_PullDownProgramJobSetChangeMenu_actManuscriptTwoSide:
			setPJFaceSelect( b_OrignalDupMode, TYPE_DuplexModeOn );			// 印刷両面.
			break;
		default:	LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );	break;
	}
	uchar u_selectItem = setupSelectedPrintType();
	setSelectMenuNo( u_selectItem );
	PNLC_PopupListControl::newInstance()->closeTop( False );		// 普通にポップアップ終了させる.
}

//クラス------------------------------------------------------------------------
//概要    ：選択印刷タイプのセットアップ.
//備考    ：.
//作成者  ：AWS_ボン.
//変更履歴：.
//------------------------------------------------------------------------------
/**
 * @brief PCPC_PullDownProgramJobSetChangeMenu_actPrint：選択印刷タイプのセットアップ.
 */
//------------------------------------------------------------------------------
uchar PCPC_PullDownProgramJobSetChangeMenu_actPrint::setupSelectedPrintType()
{
	uchar u_selectItem = PNLD_PullDownProgramJobSetChangeMenu_actPrintOneSide + 1;
// Comment 2019/07/16 【IT6_EagleH】【HCL_Prabakaran】OP_BTS-8347 jobSpec Type has been changed for common use.
//	if( False == isOn( CNK_1SideCopy, PnlComGetJobCopy() ) ) {
	if( False == isOn( CNK_1SideCopy, comPanelJob ) ) {
		u_selectItem = PNLD_PullDownProgramJobSetChangeMenu_actPrintTwoSide + 1;
	}
	Drp_ProgramJobSetChangeMenu_actPrint.setSelectNo( u_selectItem );
	return u_selectItem;
}
//クラス------------------------------------------------------------------------
//概要    ：画面開始処理.
//備考    ：.
//作成者  ：AWS_ボン.
//変更履歴：.
//------------------------------------------------------------------------------
/**
 * @brief PCPC_PullDownProgramJobSetChangeMenu_actBindPosition：画面開始処理.
 */
//------------------------------------------------------------------------------
void PCPC_PullDownProgramJobSetChangeMenu_actBindPosition::startGamen()
{
	PPopupPullDownCom::startGamen();		// 親クラスの画面開始処理.

	uchar u_selectItem = setupSelectedBindPositionType();
	setSelectMenuNo( u_selectItem );
}
// comment 2020/07/14 SCD2-Sky 石原 [IT6_2.0MR2][OP_BTS-25362] フッターエリア対応.
//クラス------------------------------------------------------------------------
//概要    ：表示部品のセットアップ.
//備考    ：.
//作成者  ：Sky石原.
//変更履歴：.
//------------------------------------------------------------------------------
/**
 * @brief PCPC_PullDownProgramJobSetChangeMenu_actBindPosition：画面開始処理.
 */
//------------------------------------------------------------------------------
void PCPC_PullDownProgramJobSetChangeMenu_actBindPosition::setupGamen( short option )
{
	PPopupPullDownCom::setupGamen( option );
#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus)  && defined (DEF_LCD_7INCH) ) )
	PnlComFooterPartsBtnActiveChange(homeKeyActiveCheck(), idKeyActiveCheck(), resetKeyActiveCheck());
#endif		//  ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus)  && defined (DEF_LCD_7INCH) ) )	.
}

//関数--------------------------------------------------------------------------
//概要    ：表示部品のセットアップ.
//備考    ：.
//戻値    ：.
//作成者  ：AWS_ボン.
//変更履歴：.
//------------------------------------------------------------------------------
/**
 * @brief PCPC_PullDownProgramJobSetChangeMenu_actBindPosition：表示部品のセットアップ.
 */
//------------------------------------------------------------------------------
void PCPC_PullDownProgramJobSetChangeMenu_actBindPosition::procButtonOn( uchar option )
{
	TYPC_Scan* pc_Scan = comPanelJob->getJobSpec()->pc_Scan;
	if( NULL == pc_Scan ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }
	// 原稿のとじ代の方向を設定.
	TYPE_BindDirection e_BindDirection = pc_Scan->c_BindDirection.getBindDirection();
	switch( option ) {
		case PNLD_PullDownProgramJobSetChangeMenu_actBindPositionAuto:	e_BindDirection = TYPE_BindDirectionAuto;	break;	// 自動.
		case PNLD_PullDownProgramJobSetChangeMenu_actBindPositionLeft:	e_BindDirection = TYPE_BindDirectionLeft;	break;	// 左.
		case PNLD_PullDownProgramJobSetChangeMenu_actBindPositionUp:	e_BindDirection = TYPE_BindDirectionTop;	break;	// 上.
		case PNLD_PullDownProgramJobSetChangeMenu_actBindPositionRight:	e_BindDirection = TYPE_BindDirectionRight;	break;	// 右.
		default:	LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );	break;
	}
	// 原稿のとじ代の方向を設定.
	pc_Scan->c_BindDirection.setBindDirection( e_BindDirection );
	uchar u_selectItem = setupSelectedBindPositionType();
	setSelectMenuNo( u_selectItem );
	PNLC_PopupListControl::newInstance()->closeTop( False );		// 普通にポップアップ終了させる.
}

//クラス------------------------------------------------------------------------
//概要    ：選択原稿のとじしろタイプのセットアップ.
//備考    ：.
//作成者  ：AWS_ボン.
//変更履歴：.
//------------------------------------------------------------------------------
/**
 * @brief PCPC_PullDownProgramJobSetChangeMenu_actBindPosition：選択原稿のとじしろタイプのセットアップ.
 */
//------------------------------------------------------------------------------
uchar PCPC_PullDownProgramJobSetChangeMenu_actBindPosition::setupSelectedBindPositionType()
{
	TYPC_Scan* pc_Scan = comPanelJob->getJobSpec()->pc_Scan;
	if( NULL == pc_Scan ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return 0;}	//【 静的解析対応 : 酒井(健) : 2011.01.20 : CID-287669 】.
	TYPE_BindDirection e_BindDirection = pc_Scan->c_BindDirection.getBindDirection();
	// 原稿のとじ代の方向の取得とローテーションボタンのインデックス設定.
	short ss_SelectBindDirection = -1;
	switch( e_BindDirection ) {
		case TYPE_BindDirectionAuto:	ss_SelectBindDirection = PNLD_PullDownProgramJobSetChangeMenu_actBindPositionAuto;	break;	// 自動.
		case TYPE_BindDirectionLeft:	ss_SelectBindDirection = PNLD_PullDownProgramJobSetChangeMenu_actBindPositionLeft;	break;	// 左.
		case TYPE_BindDirectionTop:		ss_SelectBindDirection = PNLD_PullDownProgramJobSetChangeMenu_actBindPositionUp;	break;	// 上.
		case TYPE_BindDirectionRight:	ss_SelectBindDirection = PNLD_PullDownProgramJobSetChangeMenu_actBindPositionRight;	break;	// 右.
		default:	LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );	break;
	}
	uchar u_selectNo = static_cast<uchar>( ss_SelectBindDirection + 1 );
	Drp_ProgramJobSetChangeMenu_actBindPosition.setSelectNo( u_selectNo );
	return u_selectNo;
}



//関数--------------------------------------------------------------------------
//概要    ：画面開始処理.
//備考    ：.
//戻値    ：.
//作成者  ：AWS_ボン.
//変更履歴：.
//------------------------------------------------------------------------------
/**
 * @brief A070800700 プログラムジョブ停止中_設定変更メニュー原稿束 : 画面開始処理
 */
//------------------------------------------------------------------------------
void PCPC_ProgramJobSetChangeMenu_act::startGamen()
{
	PPopupDecisionButtonCom::startGamen();		// 上位クラスの処理.
	Cmp_ProgramJobSetChangeMenu_actMessageArea.setOnOffButton( False );

	Cmp_ProgramJobSetChangeMenu_actMessageArea.setMode( PNLE_PopupMessageAreaMessageModeAdmin );

	Cmp_ProgramJobSetChangeMenu_actMessageArea.showDestinationCount( False );
	Cmp_ProgramJobSetChangeMenu_actMessageArea.setNowPopupGamen( PNLE_ProgramJobSetChangeMenu_act );

	Drp_ProgramJobSetChangeMenu_actManuscript.setPullDownTxtTable( ( ushort* )us_mManuscript_Txt, PNLD_ProgramJobSetChangeMenu_ManuscriptSettings_MaxCnt );
	Drp_ProgramJobSetChangeMenu_actPrint.setPullDownTxtTable( ( ushort* )us_mPrint_Txt, PNLD_ProgramJobSetChangeMenu_PrintSettings_MaxCnt );

	p_ManuscriptSetting = Drp_ProgramJobSetChangeMenu_actManuscript.getPullDownMenu();	// プルダウンアドレス取得.
	p_PrintSetting      = Drp_ProgramJobSetChangeMenu_actPrint.getPullDownMenu();		// プルダウンアドレス取得.
	initDynamicPullDownInfo();
	pc_mProgramJobSetChangeMenu_act = this;

// Comment VenusMLK -> 2017/03/22 SCD2 3G Sky瀬戸 [AccuRev.1017581]プログラムジョブ停止中の設定変更では混載設定時に原稿サイズ設定を表示させない.
#ifdef DEF_A4_Cus
	TYPC_Scan* pc_Scan = comPanelJob->getJobSpec()->pc_Scan;
	if( TYPE_OriginalSizeModeMix == pc_Scan->c_OriginalSize.getOriginalSizeMode() ) {		// 原稿サイズモード：混載原稿か.
		Key_ProgramJobSetChangeMenu_actOrigSize.showEnable( False );						// 「原稿サイズ」キーの非表示.
	} else {
		Key_ProgramJobSetChangeMenu_actOrigSize.showEnable( True );							// 原稿サイズの変更は混載原稿が設定されていないときのみ可能.		//「原稿サイズ」キーの表示.
	}
#endif // DEF_A4_Cus.
// Comment VenusMLK <- 2017/03/22 SCD2 3G Sky瀬戸 [AccuRev.1017581]プログラムジョブ停止中の設定変更では混載設定時に原稿サイズ設定を表示させない.
	// 原稿のとじしろ初期化処理.
	initManuscriptCloseAndCarryOut();
	setupSelectedBindPositionType();

}

//関数--------------------------------------------------------------------------
//概要    ：表示部品のセットアップ.
//備考    ：.
//戻値    ：.
//作成者  ：AWS_ボン.
//変更履歴：.
//------------------------------------------------------------------------------
/**
 * @brief A070800700 プログラムジョブ停止中_設定変更メニュー原稿束 : 表示部品の
 */
//------------------------------------------------------------------------------
void PCPC_ProgramJobSetChangeMenu_act::setupGamen( short option )
{
	PPopupDecisionButtonCom::setupGamen( option );

	if( False == m_bl_InitialOneTime ) { setChangeDisplayParts(); }

	Cmp_ProgramJobSetChangeMenu_actMessageArea.showDestinationCount( False );
	Cmp_ProgramJobSetChangeMenu_actMessageArea.changeMainMessageText( s_pMSGPgJobManuscriptBunch_W448H034A );
	Cmp_ProgramJobSetChangeMenu_actMessageArea.changeMessageDisplay( PNLE_PopupMessageAreaMessageShowAll );
// Comment 2019/07/16 【IT6_EagleH】【HCL_Prabakaran】OP_BTS-8347 jobSpec Type has been changed for common use.
//	APIC_Job* pc_JobCopy = PnlComGetJobCopy();
	APIC_Job* pc_JobCopy = comPanelJob;
	if( NULL == pc_JobCopy ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return;}	//【 静的解析対応 : 酒井(健) : 2011.03.03 : CID-288963 】.

	setupSelectedBindPositionType();	// 原稿のとじしろのセットアップ.
	// 用紙サイズのセットアップ.
	// 原稿外消去のセットアップ.
	if( isOn( CNK_SkyShot, pc_JobCopy ) ) {		//【 静的解析対応 : 酒井(健) : 2011.03.03 : CID-288963 】.
		Key_ProgramJobSetChangeMenu_actNonImageAreaErase.chkOn();
	} else {
		Key_ProgramJobSetChangeMenu_actNonImageAreaErase.chkOff();
	}

//倍率
	TYPC_Common* pc_Common = comPanelJob->getJobSpec()->pc_Common;
//comment [2017/01/13] [HCLT-Kalaiyarasi] [IT5_4.0] [AR1011305] When the AMS is On.
//	TYPC_Point<float> c_magVal = pc_Common->c_Mag.getMag();		// 倍率値の設定.
	TYPC_Point<float> c_magVal = pc_Common->c_Mag.getMag().xyReverse();		// 倍率値の設定.
	TYPE_MagType	e_MagType = pc_Common->c_Mag.getMagType();
	if( pc_Common->c_Mag.getAmsOn() ) {
		Tfd_ProgramJobSetChangeMenu_actZoomPercent.showEnable( False );
		Txt_ProgramJobSetChangeMenu_actZoom2Percent.showEnable( False );
//		Txt_ProgramJobSetChangeMenu_acPercent.showEnable( False );
		Txt_ProgramJobSetChangeMenu_ColonX.showEnable( False );
		Num_ProgramJobSetChangeMenu_FieldX.showEnable( False );
		Txt_ProgramJobSetChangeMenu_ColonY.showEnable( False );
		Num_ProgramJobSetChangeMenu_FieldY.showEnable( False );
		Txt_ProgramJobSetChangeMenu_PercentX.showEnable( False );
		Txt_ProgramJobSetChangeMenu_PercentY.showEnable( False );
	} else {
//		Key_ProgramJobSetChangeMenu_actZoom.set( &Tfd_ProgramJobSetChangeMenu_actZoomPercent, c_magVal.x * 100 , True );
		Txt_ProgramJobSetChangeMenu_actZoom2Percent.showEnable( True );
		Tfd_ProgramJobSetChangeMenu_actZoomPercent.showEnable( True );
//		Txt_ProgramJobSetChangeMenu_acPercent.showEnable( True );
		Txt_ProgramJobSetChangeMenu_ColonX.showEnable( True );
		Num_ProgramJobSetChangeMenu_FieldX.showEnable( True );
		Txt_ProgramJobSetChangeMenu_ColonY.showEnable( True );
		Num_ProgramJobSetChangeMenu_FieldY.showEnable( True );
		Txt_ProgramJobSetChangeMenu_PercentX.showEnable( True );
		Txt_ProgramJobSetChangeMenu_PercentY.showEnable( True );
	}

	uchar uc_Destination = PnlComGetDestination();	// 仕向設定内容取得.
	if( pc_Common->c_Mag.getAmsOn() ) {
#ifdef DEF_LCD_7INCH
		Key_ProgramJobSetChangeMenu_actZoom.changeText( &Txt_ProgramJobSetChangeMenu_acZoomPaperType, s_pITMAuto_W152H018A );
#else // #ifdef DEF_LCD_7INCH.
		Key_ProgramJobSetChangeMenu_actZoom.changeText( &Txt_ProgramJobSetChangeMenu_acZoomPaperType, s_pBDGAuto_W080H036A );
#endif // #ifdef DEF_LCD_7INCH.

	}
	else {
//comment [2017/01/13] [HCLT-Kalaiyarasi] [IT5_4.0] [AR1011305] Handling the Zoom Value incase of Free Setting.
		if( TYPE_MagTypeXyZoom == e_MagType ) {
			Key_ProgramJobSetChangeMenu_actZoom.showEnableAdd( &Tfd_ProgramJobSetChangeMenu_actZoomPercent ,False );
			Key_ProgramJobSetChangeMenu_actZoom.showEnableAdd( &Txt_ProgramJobSetChangeMenu_actZoom2Percent ,False );
			Key_ProgramJobSetChangeMenu_actZoom.showEnableAdd( &Txt_ProgramJobSetChangeMenu_ColonX ,True );			// X:.
			Key_ProgramJobSetChangeMenu_actZoom.showEnableAdd( &Num_ProgramJobSetChangeMenu_FieldX ,True );			// X:.
			Key_ProgramJobSetChangeMenu_actZoom.showEnableAdd( &Txt_ProgramJobSetChangeMenu_ColonY ,True );			// Y:.
			Key_ProgramJobSetChangeMenu_actZoom.showEnableAdd( &Num_ProgramJobSetChangeMenu_FieldY ,True );			// Y:.
			Key_ProgramJobSetChangeMenu_actZoom.showEnableAdd( &Txt_ProgramJobSetChangeMenu_PercentX ,True );			// Y:.
			Key_ProgramJobSetChangeMenu_actZoom.showEnableAdd( &Txt_ProgramJobSetChangeMenu_PercentY ,True );			// Y:.
			Num_ProgramJobSetChangeMenu_FieldX.set( c_magVal.x * 100, True );
			Num_ProgramJobSetChangeMenu_FieldY.set( c_magVal.y * 100, True );	
		} else {
			Key_ProgramJobSetChangeMenu_actZoom.showEnableAdd( &Txt_ProgramJobSetChangeMenu_ColonX ,False );			// X:.
			Key_ProgramJobSetChangeMenu_actZoom.showEnableAdd( &Num_ProgramJobSetChangeMenu_FieldX ,False );			// X:.
			Key_ProgramJobSetChangeMenu_actZoom.showEnableAdd( &Txt_ProgramJobSetChangeMenu_ColonY ,False );			// X:.
			Key_ProgramJobSetChangeMenu_actZoom.showEnableAdd( &Num_ProgramJobSetChangeMenu_FieldY ,False );			// X:.
			Key_ProgramJobSetChangeMenu_actZoom.showEnableAdd( &Txt_ProgramJobSetChangeMenu_PercentX ,False );			// Y:.
			Key_ProgramJobSetChangeMenu_actZoom.showEnableAdd( &Txt_ProgramJobSetChangeMenu_PercentY ,False);			// Y:.
			Key_ProgramJobSetChangeMenu_actZoom.showEnableAdd( &Tfd_ProgramJobSetChangeMenu_actZoomPercent ,True );		// X:.
			Key_ProgramJobSetChangeMenu_actZoom.showEnableAdd( &Txt_ProgramJobSetChangeMenu_actZoom2Percent ,True );
			Key_ProgramJobSetChangeMenu_actZoom.set( &Tfd_ProgramJobSetChangeMenu_actZoomPercent, c_magVal.x * 100 , True ); // Zoom Value.
		}
		switch( pc_Common->c_Mag.getMagType() ) {
			case TYPE_MagTypeFull:
				Key_ProgramJobSetChangeMenu_actZoom.changeText( &Txt_ProgramJobSetChangeMenu_acZoomPaperType, NULL );
				break;
			case TYPE_MagTypeE1:
				if( TYPE_SystemType_US == uc_Destination ) {
#ifdef DEF_LCD_7INCH
					Key_ProgramJobSetChangeMenu_actZoom.changeText( &Txt_ProgramJobSetChangeMenu_acZoomPaperType, s_pNOT81P2x14to11x17_W152H018A );
#else // #ifdef DEF_LCD_7INCH.
					Key_ProgramJobSetChangeMenu_actZoom.changeText( &Txt_ProgramJobSetChangeMenu_acZoomPaperType, s_pNOT81P2x14to11x17_W144H016A );
#endif // #ifdef DEF_LCD_7INCH.
				} else {
						//HCL<330631><20100803>-Modify Start.
						//Txt_ProgramJobSetChangeMenu_acZoomPaperType.changeText( s_pNOTA3B4toA4B5_W144H016A );		//拡大1
						//HCL<328487><20100910>-Modify Start.
						//Txt_ProgramJobSetChangeMenu_acZoomPaperType.changeText( s_pNOTB4B5toA3A4_W144H016A/*s_pNOTA3A4toB4B5_W144H016A*/ );  		//縮小3
// comment 2013.08.22 OPS1-Sky浜野 [次世代アーキ] [7inch化対応].
//						Key_ProgramJobSetChangeMenu_actZoom.changeText( &Txt_ProgramJobSetChangeMenu_acZoomPaperType, s_pNOTB4B5toA3A4_W144H016A );
#ifdef DEF_LCD_7INCH
#ifdef DEF_A4_Cus   // A3A4共通化FUM ST 2015.2.5 KM我妻.
						Key_ProgramJobSetChangeMenu_actZoom.changeText( &Txt_ProgramJobSetChangeMenu_acZoomPaperType, s_pNOTB5B6toA4A5_W152H018A );
#else // #ifdef DEF_A4_Cus.
						Key_ProgramJobSetChangeMenu_actZoom.changeText( &Txt_ProgramJobSetChangeMenu_acZoomPaperType, s_pNOTB4B5toA3A4_W152H018A );
#endif // #ifdef DEF_A4_Cus.
#else // #ifdef DEF_LCD_7INCH.
//Comment 2018/03/14 【IT6_Sparrow】【HCL_Rajkumar】制御構成変更_No.222 Message Modification.
#ifdef DEF_A4_Cus
					Key_ProgramJobSetChangeMenu_actZoom.changeText( &Txt_ProgramJobSetChangeMenu_acZoomPaperType, s_pNOTB5B6toA4A5_W152H018A );	
#else // DEF_A4_Cus.
					Key_ProgramJobSetChangeMenu_actZoom.changeText( &Txt_ProgramJobSetChangeMenu_acZoomPaperType, s_pNOTB4B5toA3A4_W144H016A );
#endif // DEF_A4_Cus.
#endif // #ifdef DEF_LCD_7INCH.
				}
				break;
			case TYPE_MagTypeE2:
					if( TYPE_SystemType_US == uc_Destination ){
// comment 2013.08.22 OPS1-Sky浜野 [次世代アーキ] [7inch化対応].
//						Key_ProgramJobSetChangeMenu_actZoom.changeText( &Txt_ProgramJobSetChangeMenu_acZoomPaperType, s_pNOT81P2x11to11x17_W144H016A );
#ifdef DEF_LCD_7INCH
#ifdef DEF_A4_Cus   // A3A4共通化FUM ST 2015.2.5 KM我妻.
						Key_ProgramJobSetChangeMenu_actZoom.changeText( &Txt_ProgramJobSetChangeMenu_acZoomPaperType, s_pNOT51P2x81P2to81P2x11_W152H018A );
#else // #ifdef DEF_A4_Cus.
						Key_ProgramJobSetChangeMenu_actZoom.changeText( &Txt_ProgramJobSetChangeMenu_acZoomPaperType, s_pNOT81P2x11to11x17_W152H018A );
#endif // #ifdef DEF_LCD_7INCH.
#else // #ifdef DEF_LCD_7INCH.
//Comment 2018/03/14 【IT6_Sparrow】【HCL_Rajkumar】制御構成変更_No.222 Message Modification.
#ifdef DEF_A4_Cus
					Key_ProgramJobSetChangeMenu_actZoom.changeText( &Txt_ProgramJobSetChangeMenu_acZoomPaperType, s_pNOT51P2x81P2to81P2x11_W152H018A );
#else // DEF_A4_Cus.
					Key_ProgramJobSetChangeMenu_actZoom.changeText( &Txt_ProgramJobSetChangeMenu_acZoomPaperType, s_pNOT81P2x11to11x17_W144H016A );
#endif // DEF_A4_Cus
#endif // #ifdef DEF_LCD_7INCH.
				} else {
						//HCL<328487><20100910>-Modify Start.
						//Txt_ProgramJobSetChangeMenu_acZoomPaperType.changeText( s_pNOTA4toB4_W052H016A ); 		//拡大2
// comment 2013.08.22 OPS1-Sky浜野 [次世代アーキ] [7inch化対応].
//						Key_ProgramJobSetChangeMenu_actZoom.changeText( &Txt_ProgramJobSetChangeMenu_acZoomPaperType, s_pNOTA4toB4_W052H016A );
#ifdef DEF_LCD_7INCH
#ifdef DEF_A4_Cus   // A3A4共通化FUM ST 2015.2.5 KM我妻.
						Key_ProgramJobSetChangeMenu_actZoom.changeText( &Txt_ProgramJobSetChangeMenu_acZoomPaperType, s_pNOTA5A6toB5B6_W152H018A );
#else // #ifdef DEF_A4_Cus.
						Key_ProgramJobSetChangeMenu_actZoom.changeText( &Txt_ProgramJobSetChangeMenu_acZoomPaperType, s_pNOTA4toB4_W152H018A );
#endif // #ifdef DEF_LCD_7INCH.
#else // #ifdef DEF_LCD_7INCH.
//Comment 2018/03/14 【IT6_Sparrow】【HCL_Rajkumar】制御構成変更_No.222 Message Modification.
#ifdef DEF_A4_Cus
					Key_ProgramJobSetChangeMenu_actZoom.changeText( &Txt_ProgramJobSetChangeMenu_acZoomPaperType, s_pNOTA5A6toB5B6_W152H018A );
#else // DEF_A4_Cus.
					Key_ProgramJobSetChangeMenu_actZoom.changeText( &Txt_ProgramJobSetChangeMenu_acZoomPaperType, s_pNOTA4toB4_W052H016A );
#endif // DEF_A4_Cus
#endif // #ifdef DEF_LCD_7INCH.
				}
				break;
			case TYPE_MagTypeE3:
				if( TYPE_SystemType_US == uc_Destination ) {
#ifdef DEF_LCD_7INCH
					Key_ProgramJobSetChangeMenu_actZoom.changeText( &Txt_ProgramJobSetChangeMenu_acZoomPaperType, s_pNOT51P2x81P21to81P2x14_W152H018A );
#else // #ifdef DEF_LCD_7INCH.
					Key_ProgramJobSetChangeMenu_actZoom.changeText( &Txt_ProgramJobSetChangeMenu_acZoomPaperType, s_pNOT51P2x81P21to81P2x14_W144H016A );
#endif // #ifdef DEF_LCD_7INCH.
				} else {
						//HCL<328487><20100910>-Modify Start.
						//Txt_ProgramJobSetChangeMenu_acZoomPaperType.changeText( s_pNOTA3A4toB4B5_W144H016A/*s_pNOTB4B5toA3A4_W144H016A*/ ); 		//拡大3
// comment 2013.08.22 OPS1-Sky浜野 [次世代アーキ] [7inch化対応].
//						Key_ProgramJobSetChangeMenu_actZoom.changeText( &Txt_ProgramJobSetChangeMenu_acZoomPaperType, s_pNOTA3A4toB4B5_W144H016A );
#ifdef DEF_LCD_7INCH
#ifdef DEF_A4_Cus   // A3A4共通化FUM ST 2015.2.5 KM我妻.
						Key_ProgramJobSetChangeMenu_actZoom.changeText( &Txt_ProgramJobSetChangeMenu_acZoomPaperType, s_pNOTA5B6toA4B5_W152H018A );
#else // #ifdef DEF_A4_Cus.
						Key_ProgramJobSetChangeMenu_actZoom.changeText( &Txt_ProgramJobSetChangeMenu_acZoomPaperType, s_pNOTA3A4toB4B5_W152H018A );
#endif // #ifdef DEF_LCD_7INCH.
#else // #ifdef DEF_LCD_7INCH.
//Comment 2018/03/14 【IT6_Sparrow】【HCL_Rajkumar】制御構成変更_No.222 Message Modification.
#ifdef DEF_A4_Cus
					Key_ProgramJobSetChangeMenu_actZoom.changeText( &Txt_ProgramJobSetChangeMenu_acZoomPaperType, s_pNOTA5B6toA4B5_W152H018A );
#else // DEF_A4_Cus.
					Key_ProgramJobSetChangeMenu_actZoom.changeText( &Txt_ProgramJobSetChangeMenu_acZoomPaperType, s_pNOTA3A4toB4B5_W144H016A );
#endif // DEF_A4_Cus
#endif // #ifdef DEF_LCD_7INCH.
				}
				break;
			case TYPE_MagTypeR1:
				if( TYPE_SystemType_US == uc_Destination ) {
#ifdef DEF_LCD_7INCH
					Key_ProgramJobSetChangeMenu_actZoom.changeText( &Txt_ProgramJobSetChangeMenu_acZoomPaperType, s_pNOT81P2x14to81P2x11_W152H018A );
#else // #ifdef DEF_LCD_7INCH.
					Key_ProgramJobSetChangeMenu_actZoom.changeText( &Txt_ProgramJobSetChangeMenu_acZoomPaperType, s_pNOT81P2x14to81P2x11_W144H016A );
#endif // #ifdef DEF_LCD_7INCH.
				} else {
						//HCL<328487><20100910>-Modify Start.
						//Txt_ProgramJobSetChangeMenu_acZoomPaperType.changeText( s_pNOTA3A4toB4B5_W144H016A/*s_pNOTA4B5toA3B4_W144H016A*/ );  		//縮小1
// comment 2013.08.22 OPS1-Sky浜野 [次世代アーキ] [7inch化対応].
//						Key_ProgramJobSetChangeMenu_actZoom.changeText( &Txt_ProgramJobSetChangeMenu_acZoomPaperType, s_pNOTA3A4toB4B5_W144H016A );
#ifdef DEF_LCD_7INCH
#ifdef DEF_A4_Cus   // A3A4共通化FUM ST 2015.2.5 KM我妻.
						Key_ProgramJobSetChangeMenu_actZoom.changeText( &Txt_ProgramJobSetChangeMenu_acZoomPaperType, s_pNOTA4A5toB5B6_W152H018A );
#else // #ifdef DEF_A4_Cus.
						Key_ProgramJobSetChangeMenu_actZoom.changeText( &Txt_ProgramJobSetChangeMenu_acZoomPaperType, s_pNOTA3A4toB4B5_W152H018A );
#endif // #ifdef DEF_A4_Cus.
#else // #ifdef DEF_LCD_7INCH.
//Comment 2018/03/14 【IT6_Sparrow】【HCL_Rajkumar】制御構成変更_No.222 Message Modification.
#ifdef DEF_A4_Cus   // A3A4共通化FUM ST 2015.2.5 KM我妻.
						Key_ProgramJobSetChangeMenu_actZoom.changeText( &Txt_ProgramJobSetChangeMenu_acZoomPaperType, s_pNOTA4A5toB5B6_W152H018A ); 
#else // DEF_A4_Cus.
					Key_ProgramJobSetChangeMenu_actZoom.changeText( &Txt_ProgramJobSetChangeMenu_acZoomPaperType, s_pNOTA3A4toB4B5_W144H016A );
#endif // DEF_A4_Cus
#endif // #ifdef DEF_LCD_7INCH.
				}
				break;
			case TYPE_MagTypeR2:
				if( TYPE_SystemType_US == uc_Destination ) {
#ifdef DEF_LCD_7INCH
					Key_ProgramJobSetChangeMenu_actZoom.changeText( &Txt_ProgramJobSetChangeMenu_acZoomPaperType, s_pNOT11x17to81P2x14_W152H018A );
#else // #ifdef DEF_LCD_7INCH.
					Key_ProgramJobSetChangeMenu_actZoom.changeText( &Txt_ProgramJobSetChangeMenu_acZoomPaperType, s_pNOT11x17to81P2x14_W144H016A );
#endif // #ifdef DEF_LCD_7INCH.
				} else {
						//HCL<328487><20100910>-Modify Start.
						//Txt_ProgramJobSetChangeMenu_acZoomPaperType.changeText( s_pNOTB4toA4_W052H016A );  		//縮小2
// comment 2013.08.22 OPS1-Sky浜野 [次世代アーキ] [7inch化対応].
//						Key_ProgramJobSetChangeMenu_actZoom.changeText( &Txt_ProgramJobSetChangeMenu_acZoomPaperType, s_pNOTB4toA4_W052H016A );
#ifdef DEF_LCD_7INCH
#ifdef DEF_A4_Cus   // A3A4共通化FUM ST 2015.2.5 KM我妻.
						Key_ProgramJobSetChangeMenu_actZoom.changeText( &Txt_ProgramJobSetChangeMenu_acZoomPaperType, s_pNOTB5B6toA5A6_W152H018A );
#else // #ifdef DEF_A4_Cus.
						Key_ProgramJobSetChangeMenu_actZoom.changeText( &Txt_ProgramJobSetChangeMenu_acZoomPaperType, s_pNOTB4toA4_W152H018A );
#endif // #ifdef DEF_LCD_7INCH.
#else // #ifdef DEF_LCD_7INCH.
//Comment 2018/03/14 【IT6_Sparrow】【HCL_Rajkumar】制御構成変更_No.222 Message Modification.
#ifdef DEF_A4_Cus   // A3A4共通化FUM ST 2015.2.5 KM我妻.
						Key_ProgramJobSetChangeMenu_actZoom.changeText( &Txt_ProgramJobSetChangeMenu_acZoomPaperType, s_pNOTB5B6toA5A6_W152H018A );  
#else // DEF_A4_Cus.
					Key_ProgramJobSetChangeMenu_actZoom.changeText( &Txt_ProgramJobSetChangeMenu_acZoomPaperType, s_pNOTB4toA4_W052H016A );
#endif // DEF_A4_Cus
#endif // #ifdef DEF_LCD_7INCH.
				}
				break;
			case TYPE_MagTypeR3:
					if( TYPE_SystemType_US == uc_Destination ){
// comment 2013.08.22 OPS1-Sky浜野 [次世代アーキ] [7inch化対応].
//						Key_ProgramJobSetChangeMenu_actZoom.changeText( &Txt_ProgramJobSetChangeMenu_acZoomPaperType, s_pNOT11x17to81P2x11_W144H016A );
#ifdef DEF_LCD_7INCH
#ifdef DEF_A4_Cus   // A3A4共通化FUM ST 2015.2.5 KM我妻.
// Comment 2019/07/04 HCL-Ranjith [EagleH][OP_BTS-9559] Message ID change for Zoom 64.7%.
//						Key_ProgramJobSetChangeMenu_actZoom.changeText( &Txt_ProgramJobSetChangeMenu_acZoomPaperType, s_pNOT81P2x13to51P2x81P2_W152H018A );
						Key_ProgramJobSetChangeMenu_actZoom.changeText( &Txt_ProgramJobSetChangeMenu_acZoomPaperType, s_pNOT81P2x11to51P2x81P2_W152H018A );
#else // #ifdef DEF_A4_Cus.
						Key_ProgramJobSetChangeMenu_actZoom.changeText( &Txt_ProgramJobSetChangeMenu_acZoomPaperType, s_pNOT11x17to81P2x11_W152H018A );
#endif // #ifdef DEF_LCD_7INCH.
#else // #ifdef DEF_LCD_7INCH.
//Comment 2018/03/14 【IT6_Sparrow】【HCL_Rajkumar】制御構成変更_No.222 Message Modification.
#ifdef DEF_A4_Cus   // A3A4共通化FUM ST 2015.2.5 KM我妻.
// Comment 2019/07/04 HCL-Ranjith [EagleH][OP_BTS-9559] Message ID change for Zoom 64.7%.
//					Key_ProgramJobSetChangeMenu_actZoom.changeText( &Txt_ProgramJobSetChangeMenu_acZoomPaperType, s_pNOT81P2x13to51P2x81P2_W152H018A );		
					Key_ProgramJobSetChangeMenu_actZoom.changeText( &Txt_ProgramJobSetChangeMenu_acZoomPaperType, s_pNOT81P2x11to51P2x81P2_W152H018A );		
#else // DEF_A4_Cus.
					Key_ProgramJobSetChangeMenu_actZoom.changeText( &Txt_ProgramJobSetChangeMenu_acZoomPaperType, s_pNOT11x17to81P2x11_W144H016A );
#endif // DEF_A4_Cus
#endif // #ifdef DEF_LCD_7INCH.
				} else {
						//HCL<328487><20100910>-Modify Start.
						//Txt_ProgramJobSetChangeMenu_acZoomPaperType.changeText( s_pNOTA3B4toA4B5_W144H016A/*s_pNOTA3A4toB4B5_W144H016A */);  		//縮小3
// comment 2013.08.22 OPS1-Sky浜野 [次世代アーキ] [7inch化対応].
//						Key_ProgramJobSetChangeMenu_actZoom.changeText( &Txt_ProgramJobSetChangeMenu_acZoomPaperType, s_pNOTA3B4toA4B5_W144H016A );
#ifdef DEF_LCD_7INCH
#ifdef DEF_A4_Cus   // A3A4共通化FUM ST 2015.2.5 KM我妻.
						Key_ProgramJobSetChangeMenu_actZoom.changeText( &Txt_ProgramJobSetChangeMenu_acZoomPaperType, s_pNOTA4B5toA5B6_W152H018A );
#else // #ifdef DEF_A4_Cus.
						Key_ProgramJobSetChangeMenu_actZoom.changeText( &Txt_ProgramJobSetChangeMenu_acZoomPaperType, s_pNOTA3B4toA4B5_W152H018A );
#endif // #ifdef DEF_A4_Cus.
#else // #ifdef DEF_LCD_7INCH.
//Comment 2018/03/14 【IT6_Sparrow】【HCL_Rajkumar】制御構成変更_No.222 Message Modification.
#ifdef DEF_A4_Cus   // A3A4共通化FUM ST 2015.2.5 KM我妻.
						Key_ProgramJobSetChangeMenu_actZoom.changeText( &Txt_ProgramJobSetChangeMenu_acZoomPaperType, s_pNOTA4B5toA5B6_W152H018A );	
#else // DEF_A4_Cus.
					Key_ProgramJobSetChangeMenu_actZoom.changeText( &Txt_ProgramJobSetChangeMenu_acZoomPaperType, s_pNOTA3B4toA4B5_W144H016A );
#endif // DEF_A4_Cus.
#endif // #ifdef DEF_LCD_7INCH.
				}
				break;
#ifdef DEF_A4_Cus	// VenusMLK 管理No.98_1_6 OPS2 Sky後藤(拓) 2015/12/17.
			case TYPE_MagTypeE5:
				Key_ProgramJobSetChangeMenu_actZoom.changeText( &Txt_ProgramJobSetChangeMenu_acZoomPaperType, s_pNOTB6toA4_W152H018A );
				break;
			case TYPE_MagTypeR5:
				if( TYPE_SystemType_US == uc_Destination ){
						Key_ProgramJobSetChangeMenu_actZoom.changeText( &Txt_ProgramJobSetChangeMenu_acZoomPaperType, s_pNOTLegalTo512x812_W152H018A );
				} else {
						Key_ProgramJobSetChangeMenu_actZoom.changeText( &Txt_ProgramJobSetChangeMenu_acZoomPaperType, s_pNOTA4toB6_W152H018A );
				}
				break;
#endif // DEF_A4_Cus. VenusMLK 管理No.98_1_6 OPS2 Sky後藤(拓) 2015/12/17.
			case TYPE_MagTypeM1:
			case TYPE_MagTypeM2:
			case TYPE_MagTypeM3:
			case TYPE_MagTypeZoom:
			case TYPE_MagTypeE4:
			case TYPE_MagTypeR4:
			case TYPE_MagTypeUnselect:
				Key_ProgramJobSetChangeMenu_actZoom.changeText( &Txt_ProgramJobSetChangeMenu_acZoomPaperType, NULL );
				break;
			default:
				break;
		}
	}//HCL<332590-90><20100827>-Modify Added.

	Drp_ProgramJobSetChangeMenu_actManuscript.setSelectNo( e_mManuscriptSelectIndex );	// 原稿.	// 2011/05/13 伊丹Sky平野 CQ.345143 .
	Drp_ProgramJobSetChangeMenu_actPrint.setSelectNo( e_mPrintSelectIndex );	// 印刷.	// 2011/05/13 伊丹Sky平野 CQ.345143 .

	// 用紙ボタン設定表示処理.

	TYPC_Print* pc_Print = pc_JobCopy->getJobSpec()->pc_Print;				// オモテ表紙の取得 【 静的解析対応 : 酒井(健) : 2011.03.03 : CID-288963 】.
	if( NULL == pc_Print ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }

	TYPE_FeedTray e_FeedTray = pc_Print->c_PaperTray.getFeedTray();
	PNLS_setOriginalSizeCLCD2Key s_partsFront;
	s_partsFront.ps_KeyOriginalSize =	  &Key_ProgramJobSetChangeMenu_actPaperSize;
	s_partsFront.ps_TxtOriginalSize01 =   &Ktx_ProgramJobSetChangeMenu_actPaperType;	// １行目 用紙コード表示.
	s_partsFront.ps_TxtOriginalSize02 =   &Key_ProgramJobSetChangeMenu_actAuto;			// ２行目 自動表示.
	s_partsFront.ps_TxtOriginalSizeX =    &Key_ProgramJobSetChangeMenu_actFrontInsertSheetX;	// 用紙サイズ表示 X.
	s_partsFront.ps_TxtOriginalSizeY =    &Key_ProgramJobSetChangeMenu_actFrontInsertSheetY;	// 用紙サイズ表示 Y.
	s_partsFront.ps_NumOriginalSizeX =    &Num_ProgramJobSetChangeMenu_actFrontInsertSheetX;	// 用紙サイズ数値表示 X.
	s_partsFront.ps_NumOriginalSizeY =    &Num_ProgramJobSetChangeMenu_actFrontInsertSheetY;	// 用紙サイズ数値表示 Y.
	s_partsFront.ps_TrayTxtPaperSize =    &Ktx_ProgramJobSetChangeMenu_actTrayType;
	s_partsFront.ps_TrayIconPaperSize =   &Gra_ProgramJobSetChangeMenu_act_ManualIcon;
	s_partsFront.ps_RemainIconPaperSize = &Gra_ProgramJobSetChangeMenu_actTrayMarkIcon;
	s_partsFront.ps_TypeIconPaperSize =   &Gra_ProgramJobSetChangeMenu_actPaperSizeIcon;

	// インターシートトレイに関するの表示.
	PnlComsetPaperSizeCLCD2Grap( &s_partsFront, e_FeedTray, PNLE_PaperSizeShowKind_Type1 );

	//原稿サイズ
	PNLS_setOriginalSizeCLCD2KeyPosition s_parts;
	s_parts.ps_KeyOriginalSize   =	&Key_ProgramJobSetChangeMenu_actOrigSize;
	s_parts.ps_TxtOriginalSize01 =	&Ktx_ProgramJobSetChangeMenu_actOrigSize_Sub;			// １行目 用紙コード表示.
	s_parts.ps_TxtOriginalSize02 =	&Ktx_ProgramJobSetChangeMenu_actOrigSize_Sub2;			// ２行目 自動表示.
	s_parts.ps_TxtOriginalSize03 =	&Ktx_ProgramJobSetChangeMenu_actOrigSize_Sub3;			// １行目 自動表示.
	s_parts.ps_TxtOriginalSizeX =	&Ktx_ProgramJobSetChangeMenu_actOrigSize_XSizeText;	// 用紙サイズ表示 X.
	s_parts.ps_TxtOriginalSizeY =	&Ktx_ProgramJobSetChangeMenu_actOrigSize_YSizeText;	// 用紙サイズ表示 Y.
	s_parts.ps_NumOriginalSizeX =	&Num_ProgramJobSetChangeMenu_actOrigSize_SizeXNum;		// 用紙サイズ数値表示 X.
	s_parts.ps_NumOriginalSizeY =	&Num_ProgramJobSetChangeMenu_actOrigSize_SizeYNum;		// 用紙サイズ数値表示 Y.
	s_parts.ps_TrayTxtPaperSize = NULL;
	s_parts.ps_TrayIconPaperSize = NULL;
	s_parts.ps_RemainIconPaperSize = NULL;
	s_parts.ps_TypeIconPaperSize = NULL;

	//原稿サイズの表示を行う.
	PnlComsetOriginalSizeCLCD2( &s_parts );

	uchar  us_TextFldIndex = 2;
	//画像シフト
	TYPC_Point<float> c_PointFront = pc_Print->c_ImageOffset.getFrontOffset();	// 表面の値取得.
	TYPC_Point<float> c_PointRear = pc_Print->c_ImageOffset.getRearOffset();
	if( ( 0.0f != c_PointFront.x ) || ( 0.0f != c_PointFront.y ) || ( 0.0f != c_PointRear.x ) || ( 0.0f != c_PointRear.y ) ) {
#ifdef DEF_LCD_7INCH
		Key_ProgramJobSetChangeMenu_actImageShift.changeText( s_pITMOn_W152H018A, us_TextFldIndex );
#else // #ifdef DEF_LCD_7INCH.
		Key_ProgramJobSetChangeMenu_actImageShift.changeText( s_pITMOn_W144H016A, us_TextFldIndex );
#endif // #ifdef DEF_LCD_7INCH.
	}
	else {
#ifdef DEF_LCD_7INCH
		Key_ProgramJobSetChangeMenu_actImageShift.changeText(s_pITMOFF_W152H018A, us_TextFldIndex);
#else // #ifdef DEF_LCD_7INCH.
		Key_ProgramJobSetChangeMenu_actImageShift.changeText( s_pITMOFF_W144H016A, us_TextFldIndex );
#endif // #ifdef DEF_LCD_7INCH.
	}
	//枠消し
	APIC_Job* pc_Job = comPanelJob;
	Bool b_FunctionOnFlag = isOn( CNK_FrameErase, pc_Job );
	if( True == b_FunctionOnFlag ) {
#ifdef DEF_LCD_7INCH
		Key_ProgramJobSetChangeMenu_actFrameErase.changeText( s_pITMOn_W152H018A/*s_pITMON_W032H016A*/,PNLE_TransitionButtonText2Index );
#else // #ifdef DEF_LCD_7INCH.
		Key_ProgramJobSetChangeMenu_actFrameErase.changeText( s_pITMOn_W144H016A/*s_pITMON_W032H016A*/,PNLE_TransitionButtonText2Index );
#endif // #ifdef DEF_LCD_7INCH.
	}
	else {
#ifdef DEF_LCD_7INCH
		Key_ProgramJobSetChangeMenu_actFrameErase.changeText( s_pITMOFF_W152H018A ,PNLE_TransitionButtonText2Index );
#else // #ifdef DEF_LCD_7INCH.
		Key_ProgramJobSetChangeMenu_actFrameErase.changeText( s_pITMOFF_W144H016A ,PNLE_TransitionButtonText2Index );
#endif // #ifdef DEF_LCD_7INCH.
	}

	if( False == APIC_SoftDipSw::newInstance()->getSoftDipSwBitForPanelSpecNo( BYPASS_SETTING, 6 ) ) {	// MZ-144 千代田化工建設様追加対応要望(2nd Step).
		Key_ProgramJobSetChangeMenu_actColor.showEnable( False );
	} else {
		// カラーボタンの表示状態.
		TYPC_Scan* pc_Scan = pc_JobCopy->getJobSpec()->pc_Scan;
		if( NULL == pc_Scan ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }

		short ss_ColorMenu = -1;
		if( True == pc_Scan->getAcsOn() ) {
			ss_ColorMenu = PNLE_ProgramJobSetChangeMenu_act_AutoColor;						// 自動ボタンの選択状態.
		} else if( True == pc_Common->c_2ColorMode.get2ColorOn() ) {
			ss_ColorMenu = PNLE_ProgramJobSetChangeMenu_act_2Color;							// 2色カラーの選択状態.
		} else if( True == pc_Common->c_MonoColor.getMonoColorOn() ) {
			ss_ColorMenu = PNLE_ProgramJobSetChangeMenu_act_SingleColor;					// モノカラーの選択状態.
		} else {
			switch( pc_Common->getColorMode() ) {
				case TYPE_ColorModeFullColor:
					ss_ColorMenu = PNLE_ProgramJobSetChangeMenu_act_FullColor;				// フルカラーの選択状態.
					break;
				case TYPE_ColorModeBlack:
					ss_ColorMenu = PNLE_ProgramJobSetChangeMenu_act_Black;					// ブラックの選択状態.
					break;
				default:
					break;
			}
		}

		// ボタン選択状態による他ボタンの状態変更.
		switch( ss_ColorMenu ) {
			case PNLE_ProgramJobSetChangeMenu_act_AutoColor:		// 自動選択状態.
				Key_ProgramJobSetChangeMenu_actColor.changeParts( &Gra_ProgramJobSetChangeMenu_actColorIcon, tbl_i00_009_000_X32_Y32 );
#ifdef DEF_LCD_7INCH
				Key_ProgramJobSetChangeMenu_actColor.changeText( s_pITMAutoColor_W152H018A, PNLE_TransitionButtonText2Index );
#else // #ifdef DEF_LCD_7INCH.
				Key_ProgramJobSetChangeMenu_actColor.changeText( s_pKEYAutoColor_W132H036A, PNLE_TransitionButtonText2Index );
#endif // #ifdef DEF_LCD_7INCH.
				break;
			case PNLE_ProgramJobSetChangeMenu_act_FullColor:		// フルカラー選択状態.
				Key_ProgramJobSetChangeMenu_actColor.changeParts( &Gra_ProgramJobSetChangeMenu_actColorIcon, tbl_i00_011_000_X32_Y32 );
#ifdef DEF_LCD_7INCH
				Key_ProgramJobSetChangeMenu_actColor.changeText( s_pITMFullColor_W152H018A, PNLE_TransitionButtonText2Index );
#else // #ifdef DEF_LCD_7INCH.
				Key_ProgramJobSetChangeMenu_actColor.changeText( s_pKEYFullColor_W132H036A, PNLE_TransitionButtonText2Index );
#endif // #ifdef DEF_LCD_7INCH.
				break;
			case PNLE_ProgramJobSetChangeMenu_act_Black:			// ブラック選択状態.
				Key_ProgramJobSetChangeMenu_actColor.changeParts( &Gra_ProgramJobSetChangeMenu_actColorIcon, tbl_i00_015_000_X32_Y32 );
#ifdef DEF_LCD_7INCH
				Key_ProgramJobSetChangeMenu_actColor.changeText( s_pITMBlack_W152H018A, PNLE_TransitionButtonText2Index );
#else // #ifdef DEF_LCD_7INCH.
				Key_ProgramJobSetChangeMenu_actColor.changeText( s_pKEYBlack_W132H036A, PNLE_TransitionButtonText2Index );
#endif // #ifdef DEF_LCD_7INCH.
				break;
			case PNLE_ProgramJobSetChangeMenu_act_2Color:			// 2色カラー選択状態.
				Key_ProgramJobSetChangeMenu_actColor.changeParts( &Gra_ProgramJobSetChangeMenu_actColorIcon, st_BaseColorPartsInfoTbl_2Color[( uchar )pc_Common->c_2ColorMode.getMonoColor()].pst_Parts );
				Key_ProgramJobSetChangeMenu_actColor.changeText( st_BaseColorPartsInfoTbl_2Color[( uchar )pc_Common->c_2ColorMode.getMonoColor()].text_ID, PNLE_TransitionButtonText2Index );
				break;
			case PNLE_ProgramJobSetChangeMenu_act_SingleColor:		// 単色カラー選択状態.
				Key_ProgramJobSetChangeMenu_actColor.changeParts( &Gra_ProgramJobSetChangeMenu_actColorIcon, st_BaseColorPartsInfoTbl_SingleColor[( uchar )pc_Common->c_MonoColor.getMonoColor()].pst_Parts );
				Key_ProgramJobSetChangeMenu_actColor.changeText( st_BaseColorPartsInfoTbl_SingleColor[( uchar )pc_Common->c_MonoColor.getMonoColor()].text_ID, PNLE_TransitionButtonText2Index );
				break;
			default:
				break;
		}
	}

	setupPatternCustomize();

// Comment VenusMLK -> 2017/03/22 SCD2 3G Sky瀬戸 [AccuRev.1017581]インデックス原稿は非対応のため表示させない.
#ifndef DEF_A4_Cus
	setupIndexPaperSize();		// インデックス原稿キー.
#endif // DEF_A4_Cus.
// Comment VenusMLK <- 2017/03/22 SCD2 3G Sky瀬戸 [AccuRev.1017581]インデックス原稿は非対応のため表示させない.
// comment 2020/07/14 SCD2-Sky 石原 [IT6_2.0MR2][OP_BTS-25362] フッターエリア対応.
#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus)  && defined (DEF_LCD_7INCH) ) )
	PnlComFooterPartsBtnActiveChange(homeKeyActiveCheck(), idKeyActiveCheck(), resetKeyActiveCheck());
#endif		//  ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus)  && defined (DEF_LCD_7INCH) ) )	.
}

//関数--------------------------------------------------------------------------
//概要    ：画面終了処理.
//備考    ：.
//戻値    ：.
//作成者  ：.
//レビュー：.
//変更履歴：.
//------------------------------------------------------------------------------
/**
 * @brief A070800700 プログラムジョブ停止中_設定変更メニュー原稿束 : 画面終了処理.
 */
//------------------------------------------------------------------------------
void PCPC_ProgramJobSetChangeMenu_act::endGamen()
{
	PPopupDecisionButtonCom::endGamen();
	m_bl_InitialOneTime = False;
}

//関数--------------------------------------------------------------------------
//概要    ：ボタン（押された).
//備考    ：.
//戻値    ：.
//作成者  ：AWS_ボン.
//変更履歴：.
//------------------------------------------------------------------------------
/**
 * @brief A070800700 プログラムジョブ停止中_設定変更メニュー原稿束 : ボタン（押された)
 */
//------------------------------------------------------------------------------
void PCPC_ProgramJobSetChangeMenu_act::procButtonOn( uchar option )
{
// Comment 2019/07/16 【IT6_EagleH】【HCL_Prabakaran】OP_BTS-8347 jobSpec Type has been changed for common use.
//	APIC_Job* pc_JobCopy = PnlComGetJobCopy();
	APIC_Job* pc_JobCopy = comPanelJob;
	if( NULL == pc_JobCopy ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return;}	//【 静的解析対応 : 酒井(健) : 2011.03.03 : CID-288964 】.
	short s_xPosition = PNLD_ZeroInit, s_yPosition = PNLD_ZeroInit,
	      s_width = PNLD_ZeroInit, s_height = PNLD_ZeroInit;

	switch( option ) {
		case PNLD_ProgramJobSetChangeMenu_actPaperSize:				// 用紙サイズ.
			Key_ProgramJobSetChangeMenu_actPaperSize.getPosition( s_xPosition, s_yPosition );
			Key_ProgramJobSetChangeMenu_actPaperSize.getSize( s_width, s_height );
			PNLC_PopupListControl::newInstance()->addPopup( &gp_A070801000, this, NULL, s_xPosition, s_yPosition, s_width, s_height );
			break;
		case PNLD_ProgramJobSetChangeMenu_actZoom:					// 倍率.
			if( True == combinationCheck( CNK_ZoomKey, comPanelJob ) ) {
				Key_ProgramJobSetChangeMenu_actZoom.getPosition( s_xPosition, s_yPosition );
				Key_ProgramJobSetChangeMenu_actZoom.getSize( s_width, s_height );
				PNLC_PopupListControl::newInstance()->addPopup( &gp_A070800140, this, NULL, s_xPosition, s_yPosition, s_width, s_height );
			}
			break;
		case PNLD_ProgramJobSetChangeMenu_actManuscript:			// 原稿.
			PNLC_PopupListControl::newInstance()->addPopup( p_ManuscriptSetting, this, ans_ManuscriptPullDownEnd );
			break;
		case PNLD_ProgramJobSetChangeMenu_actPrint:					// 印刷.
			PNLC_PopupListControl::newInstance()->addPopup( p_PrintSetting, this, ans_PrintPullDownEnd );
			break;
		case PNLD_ProgramJobSetChangeMenu_actOrigSize:				// 原稿サイズ.
			Key_ProgramJobSetChangeMenu_actOrigSize.getPosition( s_xPosition, s_yPosition );
			Key_ProgramJobSetChangeMenu_actOrigSize.getSize( s_width, s_height );
			PNLC_PopupListControl::newInstance()->addPopup( &gp_SetChangeOriginalSizeMetric_act, this, NULL, s_xPosition, s_yPosition, s_width, s_height );
			break;
		case PNLD_ProgramJobSetChangeMenu_actImageShift:			// 画像シフト.
			Key_ProgramJobSetChangeMenu_actImageShift.getPosition( s_xPosition, s_yPosition );
			Key_ProgramJobSetChangeMenu_actImageShift.getSize( s_width, s_height );
			PNLC_PopupListControl::newInstance()->addPopup( &gp_SetChangeShiftEditFront_act, this, NULL, s_xPosition, s_yPosition, s_width, s_height );
			break;
		case PNLD_ProgramJobSetChangeMenu_actFrameErase:			// 枠消し.
			Key_ProgramJobSetChangeMenu_actFrameErase.getPosition( s_xPosition, s_yPosition );
			Key_ProgramJobSetChangeMenu_actFrameErase.getSize( s_width, s_height );
			PNLC_PopupListControl::newInstance()->addPopup( &gp_SetChangeCollectEraseFrameInch_act, this, NULL, s_xPosition, s_yPosition, s_width, s_height );
			break;
		case PNLD_ProgramJobSetChangeMenu_actBindPosition:			// 原稿のとじしろ.
			PNLC_PopupListControl::newInstance()->addPopup( &gp_PullDownProgramJobSetChangeMenu_actBindPosition, this, ans_Manuscript );
			break;
		case PNLD_ProgramJobSetChangeMenu_actNonImageAreaErase:		// 原稿外消去.
			if( isOn( CNK_SkyShot, pc_JobCopy ) ) {		//【 静的解析対応 : 酒井(健) : 2011.03.03 : CID-288964 】.
				// 原稿外消去を取り消し.
				cancel( CNK_SkyShot, CNK_Size, pc_JobCopy );		//【 静的解析対応 : 酒井(健) : 2011.03.03 : CID-288964 】.
				Key_ProgramJobSetChangeMenu_actNonImageAreaErase.chkOff();
			} else {
				// 組み合わせ禁止チェック.
				if( combinationCheck( CNK_SkyShot ) ) {
					TYPC_Scan* pc_Scan = pc_JobCopy->getJobSpec()->pc_Scan;		//【 静的解析対応 : 酒井(健) : 2011.03.03 : CID-288964 】.
					// 原稿外消去の設定.
					pc_Scan->setSkyShotOn( True );
					Key_ProgramJobSetChangeMenu_actNonImageAreaErase.chkOn();
				}
			}
			break;
// Comment VenusMLK -> 2017/03/22 SCD2 3G Sky瀬戸 [AccuRev.1017581]インデックス原稿は非対応のため表示させない.
#ifndef DEF_A4_Cus
		case PNLD_ProgramJobSetChangeMenu_actIndexPaperSize:				// 用紙サイズ.
			Key_ProgramJobSetChangeMenu_actIndexPaperSize.getPosition( s_xPosition, s_yPosition );
			Key_ProgramJobSetChangeMenu_actIndexPaperSize.getSize( s_width, s_height );
			PNLC_PopupListControl::newInstance()->addPopup( &gp_A070800710, this, NULL, s_xPosition, s_yPosition, s_width, s_height );
			break;
#endif // DEF_A4_Cus.
// Comment VenusMLK <- 2017/03/22 SCD2 3G Sky瀬戸 [AccuRev.1017581]インデックス原稿は非対応のため表示させない.
		case PNLD_ProgramJobSetChangeMenu_actColor:							// カラー.
			Key_ProgramJobSetChangeMenu_actColor.getPosition( s_xPosition, s_yPosition );
			Key_ProgramJobSetChangeMenu_actColor.getSize( s_width, s_height );
			PNLC_PopupListControl::newInstance()->addPopup( &gp_ProgramJobSetChangeMenuColor_act, this, NULL, s_xPosition, s_yPosition, s_width, s_height );
			break;
		default:	LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );	return;
	}
	setupGamen();
}

///関数--------------------------------------------------------------------------
//概要    ：リセット.
//備考    ：.
//戻値    ：.
//作成者  ：AWS_ボン.
//変更履歴：.
//------------------------------------------------------------------------------
/**
 * @brief A070800700 プログラムジョブ停止中_設定変更メニュー原稿束 : リセット
 */
//------------------------------------------------------------------------------
void PCPC_ProgramJobSetChangeMenu_act::procReset( uchar option )
{
	PPopupDecisionButtonCom::procCancel( option );
}
// comment 2020/07/14 SCD2-Sky 石原 [IT6_2.0MR2][OP_BTS-25362] フッターエリア対応.
///関数--------------------------------------------------------------------------
//概要    ：リセット.
//備考    ：.
//戻値    ：.
//作成者  ：Sky石原.
//変更履歴：.
//------------------------------------------------------------------------------
/**
 * @brief A070800700 プログラムジョブ停止中_設定変更メニュー原稿束 : リセットキー活性状態.
 */
//------------------------------------------------------------------------------
Bool PCPC_ProgramJobSetChangeMenu_act::resetKeyActiveCheck()
{
	ACTIVEKEY_RET( True );
}
///関数--------------------------------------------------------------------------
//概要    ：スタート.
//備考    ：.
//戻値    ：.
//作成者  ：AWS_ボン.
//変更履歴：.
//------------------------------------------------------------------------------
/**
 * @brief A070800700 プログラムジョブ停止中_設定変更メニュー原稿束 : スタート
 */
//------------------------------------------------------------------------------
void PCPC_ProgramJobSetChangeMenu_act::procStart( uchar option )
{
	TYPC_Scan*	 pc_Scan   = comPanelJob->getJobSpec()->pc_Scan;
	// 動作中モード取得
	TYPC_JobStatus*  pc_JobStatus = comPanelJob->getJobStatus();
	if( ( True == comPanelJob->getJobSpec()->pc_Common->c_JobType.getProgramJobOn() )
	    && ( comPanelJob->getJobStatus()->getOutputStatus() == TYPE_OutputStatusIdle )
	    && ( comPanelJob->getJobStatus()->getInputStatus() == TYPE_OutputStatusIdle ) ) {
		// プログラムジョブがONの場合.
		APIC_SystemStatus* pc_SystemStatus = APIC_SystemStatus::newInstance();
		uchar uc_BundleNum = static_cast<uchar>( pc_SystemStatus->getProgJobDocIDCounter() ); // 読み込み束数設定.
		if( uc_BundleNum == PCGD_NUM_BUNDLENUM_MAX ) {
			return;
		}
	}
	// プリント禁止でない時.
	if( False == printStartDisable() ) {
		if( True == pc_Scan->c_MemoryIn.getMemoryInContinue()
		    || ( True == comPanelJob->getJobSpec()->pc_Common->c_JobType.getProgramJobOn()
		         && ( comPanelJob->getJobStatus()->getOutputStatus() == TYPE_OutputStatusIdle )
		         && ( comPanelJob->getJobStatus()->getInputStatus() == TYPE_OutputStatusIdle ) ) ) {
			// スタート押下処理禁止チェック.
			Bool b_OriginalSizeReq = startProhibitionCheck();
			// プラテン原稿サイズ確定要求中であればCallBackを待つ.
			if( b_OriginalSizeReq ) {
				return;
			}
			if( onlyPanel.isWarningStartDisable( comPanelJob ) == True ) {	// 警告によるスタートキー受付チェックで受付不可なら処理を終了する
				setBuzzerType( PCGE_BuzzerType_InputNg );
				return;
			}
			onlyPanel.ct_Copying1Sec.start();
		}
		if( ( True == comPanelJob->getJobSpec()->pc_Common->c_JobType.getProgramJobOn() )
		    && ( comPanelJob->getJobStatus()->getOutputStatus() == TYPE_OutputStatusIdle )
		    && ( comPanelJob->getJobStatus()->getInputStatus() == TYPE_OutputStatusIdle ) ) {
			//読み直し状態判別フラグセット.
			progJobCheckAndSetForceMemoryIn( comPanelJob, False );        // 強制的に分割読み込みにするモードか？
			onlyPanel.ep_PanelStatus = EP_PNS_ReadExec;
			onlyPanel.ct_Copying1Sec.start();
			c_SystemRequest.JobStart( comPanelJob );
			c_JobControl.setInheritorJob( comPanelJob );
			rebuildGamen( 0 );
		} else {
			if( True == comPanelJob->getJobSpec()->pc_Common->c_JobType.getProgramJobOn()
			    && ( comPanelJob->getJobStatus()->getInputStatus() != TYPE_InputStatusMemoryInScanStop ) ) {
				return;
			}
			if( ( True == comPanelJob->getJobSpec()->pc_Common->getVerifyPreviewOn() )
			    && ( False == pc_Scan->c_MemoryIn.getMemoryInContinue() ) ) {
				// (画像ログ転送OFF)仕上りプレビューONかつ読込み終了の場合.
				// 差込みページ読込み設定で差し込まれる側の原稿読込み終了時以外.
				if( ( False == pc_Scan->c_OriginalInsertion.getOn() ) || ( True == pc_JobStatus->c_DetailStatus.getOriginalInserting() ) ) {
					PCGC_VerifyThumbnailPreviewControl* pc_VerifyControl = PnlComGetVerifyPreviewControl();
					if( NULL == pc_VerifyControl ) {
						LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, ( char* )"Assert Failed!" );
						return;
					}
					pc_VerifyControl->endVerify( True );
				}
			}
			if( ( True == comPanelJob->getJobSpec()->pc_Common->getVerifyPreviewOn() ) && ( TYPE_OverlayOn == comPanelJob->getJobSpec()->pc_Common->getOverlay().e_Overlay ) ) {
				PCGC_VerifyThumbnailPreviewControl* pc_VerifyControl = PnlComGetVerifyPreviewControl();
				if( NULL != pc_VerifyControl && True == pc_VerifyControl->isStartingVerifyPreview() ) {
					// オーバーレイリーフを戻す.
					comPanelJob->returnOverLayLeafForPreviewOPO( NULL );
				}
			}
			c_SystemRequest.JobRestart( comPanelJob );
			if( False == pc_Scan->c_MemoryIn.getMemoryInContinue()
			    && True == comPanelJob->getJobSpec()->pc_Common->c_JobType.getProgramJobOn() ) {
				c_JobControl.setInheritorJob( comPanelJob );
			}
		}
	} else {
		setBuzzerType( PCGE_BuzzerType_InputNg );
	}
	onlyPanel.bl_isSampleCopyJob = False;
}

//概要    ：ストップ.
//備考    ：.
//戻値    ：.
//作成者  ：AWS_ボン.
//変更履歴：.
//------------------------------------------------------------------------------
/**
 * @brief A070800700 プログラムジョブ停止中_設定変更メニュー原稿束 : ストップ
 */
//------------------------------------------------------------------------------
void PCPC_ProgramJobSetChangeMenu_act::procStop( uchar option )
{
	PCGC_VerifyThumbnailPreviewControl* pc_VerifyControl = PnlComGetVerifyPreviewControl();
	if( False == pc_VerifyControl->checkDocRelease() ) {		// 条件分岐： 仕上りプレビュー終了処理禁止時.
		setBuzzerType( PCGE_BuzzerType_InputNg );		// 戻り.
		return;
	}
	onlyPanel.bl_isJobStopping = False;
	if( onlyPanel.bl_isJobStopping == False ) {		// 条件分岐： 停止指示したジョブは無い.
		if( ( True == comPanelJob->getJobSpec()->pc_Common->c_JobType.getProgramJobOn() )		// 条件分岐： プログラムジョブがONの場合.
		    && ( ( PnlComGetSystemStatus()->getProgJobInStatus() != TYPE_ProgJobInStatusIdle )
		         || ( PnlComGetSystemStatus()->getProgJobOutStatus() != TYPE_ProgJobOutStatusIdle ) ) ) {
			PNLC_ActionDataLib::newInstance()->setReturnProgramDeleteGamen( getNowGamen() );
			PNLC_PopupListControl::newInstance()->addPopup( &gp_ProgramJobDeleteConfirm_act, this, NULL, 360, 216, 80, 48 );		// 破棄画面.
		} else {
			if( True == comPanelJob->getJobSpec()->pc_Common->getVerifyPreviewOn() ) {		// 条件分岐： プログラムジョブがOFFの場合.
				pc_VerifyControl->endVerify( False );		// 仕上りプレビューを終了する.
				onlyPanel.bl_isSampleCopyJob = False;
			}
			c_SystemRequest.JobCancel( comPanelJob );		// ジョブを破棄する.
		}
		setBuzzerType( PCGE_BuzzerType_InputOk, True );		// ブザーOK.
	}
	// ジョブをFalseにセットする.
	onlyPanel.bl_isJobStopping = False;
}

///関数--------------------------------------------------------------------------
//概要    ：確認コピー.
//備考    ：.
//戻値    ：.
//作成者  ：AWS_ボン.
//変更履歴：.
//------------------------------------------------------------------------------
/**
 * @brief A070800700 プログラムジョブ停止中_設定変更メニュー原稿束 : 確認コピー
 */
//------------------------------------------------------------------------------
void PCPC_ProgramJobSetChangeMenu_act::procSampleCopy( uchar option )
{
	if( ( True == comPanelJob->getJobSpec()->pc_Common->c_JobType.getProgramJobOn() )		// 条件分岐： プログラムジョブがONの場合.
	    && ( comPanelJob->getJobStatus()->getOutputStatus() == TYPE_OutputStatusIdle )
	    && ( comPanelJob->getJobStatus()->getInputStatus() == TYPE_OutputStatusIdle )
	    && False == printStartDisable() ) {
		comPanelJob->getJobSpec()->pc_Common->c_JobType.setSingleStepOn( True );
		procStart( 0 );
	} else {
		setBuzzerType( PCGE_BuzzerType_InputNg );
	}
}

//関数------------------------------------------------------------------------------------------------------------------
//概要    ：パターンカスタマイズによる表示/非表示.
//備考    ：.
//戻値    ：.
//作成者  ：.
//レビュー：.
//変更履歴：.
//------------------------------------------------------------------------------
/**
* @brief A070800700 プログラムジョブ停止中_設定変更メニュー原稿束 : パターンカスタマイズによる表示/非表示
*/
//------------------------------------------------------------------------------
void PCPC_ProgramJobSetChangeMenu_act::setupPatternCustomize()
{
	PNLC_FuncPatternSettingLib* cp_FuncSetting = PNLC_FuncPatternSettingLib::newInstance();
	uchar baseCnt = 4;
#ifndef DEF_MonoCus
	if( True == APIC_SoftDipSw::newInstance()->getSoftDipSwBitForPanelSpecNo( BYPASS_SETTING, 6 ) ) {	// MZ-144 千代田化工建設様追加対応要望(2nd Step).
		baseCnt = static_cast<uchar>( baseCnt + 1 );
	}
#endif // #ifndef DEF_MonoCus.
// Comment VenusMLK -> 2017/03/22 SCD2 3G Sky瀬戸 [AccuRev.1017581]インデックス原稿は非対応のため表示させない.
#ifdef DEF_A4_Cus
	// インデックス原稿を削除するので、カウントも減らしておく.
	uchar appCnt = 5;
#else // DEF_A4_Cus.
	uchar appCnt = 6;
#endif // DEF_A4_Cus.
// Comment VenusMLK <- 2017/03/22 SCD2 3G Sky瀬戸 [AccuRev.1017581]インデックス原稿は非対応のため表示させない.

	// 用紙サイズ.
	if( cp_FuncSetting->isFunctionShowCopyPaper() == False ) {
		Key_ProgramJobSetChangeMenu_actPaperSize.showEnable( False );
		baseCnt--;
	}
	// 倍率.
	Bool b_Zoom = True;
	if( False == PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_Zoom ) ) {		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
		// 倍率機能自体が非表示.
		b_Zoom = False;
	} else {
		if( False == PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_Zoom_Auto ) &&		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
		    False == PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_Zoom_Manual ) &&		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
		    False == PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_Zoom_FullSize ) &&		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
		    False == PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_Zoom_FixedZoom ) &&		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
		    False == PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_Zoom_FixedZoom_UserPreset ) ) {		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
			// 「倍率」表示無効.
			b_Zoom = False;
		} else {
			// 「倍率」表示有効.
			b_Zoom = True;
		}
	}
	if( b_Zoom == False ) {
		Key_ProgramJobSetChangeMenu_actZoom.showEnable( False );
		baseCnt--;
	}

#ifndef DEF_MonoCus
	if( True == APIC_SoftDipSw::newInstance()->getSoftDipSwBitForPanelSpecNo( BYPASS_SETTING, 6 ) ) {	// MZ-144 千代田化工建設様追加対応要望(2nd Step).
		if( ( False == PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_Color ) )
		    || ( ( False == PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_Color_Auto ) )
		         && ( False == PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_Color_Full ) )
		         && ( False == PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_Color_2Color ) )
		         && ( False == PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_Color_Black ) )
		         && ( False == PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_Color_Single ) ) ) ) {
			Key_ProgramJobSetChangeMenu_actColor.showEnable( False );
			baseCnt--;
		}
	}
#endif // #ifndef DEF_MonoCus.

	// 原稿・印刷.
	if( PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_DuplexCombine )        == False ||		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
	    PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_DuplexCombine_Duplex ) == False ||		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
	    (	PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_DuplexCombine_Duplex_1To1 ) == False &&		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
	        PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_DuplexCombine_Duplex_1To2 ) == False &&		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
	        PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_DuplexCombine_Duplex_2To1 ) == False &&		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
	        PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_DuplexCombine_Duplex_2To2 ) == False ) ) {		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
		Drp_ProgramJobSetChangeMenu_actManuscript.showEnable( False );
		Drp_ProgramJobSetChangeMenu_actPrint.showEnable( False );
		baseCnt= static_cast<uchar>( baseCnt - 2 );
	}
	// 原稿サイズ.
// Comment VenusMLK -> 2017/03/22 SCD2 3G Sky瀬戸 [AccuRev.1017581]プログラムジョブ停止中の設定変更では混載設定時に原稿サイズ設定を表示させない.
#ifdef DEF_A4_Cus
	if( PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_OrgSet ) 				== False ||		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
	    PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_OrgSet_OriginalSize )	== False ||		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
	    TYPE_OriginalSizeModeMix == comPanelJob->getJobSpec()->pc_Scan->c_OriginalSize.getOriginalSizeMode() ) {	// 混載設定.
		Key_ProgramJobSetChangeMenu_actOrigSize.showEnable( False );
		appCnt--;
	}
#else // DEF_A4_Cus.
	if( PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_OrgSet ) 				== False ||		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
	    PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_OrgSet_OriginalSize )	== False ) {		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
		Key_ProgramJobSetChangeMenu_actOrigSize.showEnable( False );
		appCnt--;
	}
#endif // DEF_A4_Cus.
// Comment VenusMLK <- 2017/03/22 SCD2 3G Sky瀬戸 [AccuRev.1017581]プログラムジョブ停止中の設定変更では混載設定時に原稿サイズ設定を表示させない.
	// 原稿のとじしろ.
	if( PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_OrgSet ) 					== False ||		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
	    PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_OrgSet_BindingPosition )	== False ) {		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
		Drp_ProgramJobSetChangeMenu_actBindPosition.showEnable( False );
		appCnt--;
	}
// Comment VenusMLK -> 2017/03/22 SCD2 3G Sky瀬戸 [AccuRev.1017581]インデックス原稿は非対応のため表示させない.
#ifndef DEF_A4_Cus
	// インデックス原稿.
	if( False == PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_OriginalIndex ) ) {
		Key_ProgramJobSetChangeMenu_actIndexPaperSize.showEnable( False );
		appCnt--;
	}
#endif // DEF_A4_Cus.
// Comment VenusMLK <- 2017/03/22 SCD2 3G Sky瀬戸 [AccuRev.1017581]インデックス原稿は非対応のため表示させない.
	// 画像シフト.
	if( PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_ImageShift ) == False ) {		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
		Key_ProgramJobSetChangeMenu_actImageShift.showEnable( False );
		appCnt--;
	}
	// 枠消し.
	if( cp_FuncSetting->isFunctionShowCopyFrameErase() == False ) {
		Key_ProgramJobSetChangeMenu_actFrameErase.showEnable( False );
		appCnt--;
	}
	// 原稿外消去.
	if( PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_NonImageAreaErase ) == False ) {		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
		Key_ProgramJobSetChangeMenu_actNonImageAreaErase.showEnable( False );
		appCnt--;
	}

	// グループの消去.
	if( baseCnt <= 0 ) {
		Bak_ProgramJobSetChangeMenu_actBasicTitleBackground.showEnable( False );
		Txt_ProgramJobSetChangeMenu_actBasicTitle.showEnable( False );
	}
	if( appCnt <= 0 ) {
		Bak_ProgramJobSetChangeMenu_actApplicationTitleBackground.showEnable( False );
		Txt_ProgramJobSetChangeMenu_actApplicationTitle.showEnable( False );
	}
}
//------------------------------------------------------------------------------
/**
* @brief A070800700 動的プルダウン情報の初期化.
*/
//------------------------------------------------------------------------------
void PCPC_ProgramJobSetChangeMenu_act::initDynamicPullDownInfo()
{
	uchar uc_Cnt = 0;
	uchar uc_i   = 0;

	// 原稿プルダウン初期化.
	// ベーステーブルからのコピー
	for( int i = 0; i < PNLD_ProgramJobSetChangeMenu_ManuscriptSettings_MaxCnt; i++ ) {
		s_mManuscript_Info[i].b_Show         = s_ProgramJobSetChangeMenu_ManuscriptSettings_Info[i].b_Show;
		s_mManuscript_Info[i].uc_ButtonIndex = s_ProgramJobSetChangeMenu_ManuscriptSettings_Info[i].uc_ButtonIndex;
		s_mManuscript_Info[i].e_Kind         = s_ProgramJobSetChangeMenu_ManuscriptSettings_Info[i].e_Kind;
		s_mManuscript_Info[i].us_KindTxt     = s_ProgramJobSetChangeMenu_ManuscriptSettings_Info[i].us_KindTxt;
	}

	// ボタンのクリアと表示・非表示設定.
	for( uc_i = 0; uc_i < PNLD_ProgramJobSetChangeMenu_ManuscriptSettings_MaxCnt; uc_i++ ) {
		s_ProgramJobSetChangeMenu_actManuscriptPullDownButtonInfo[uc_i].pc_PullButton->changeText( s_mManuscript_Info[uc_i].us_KindTxt );
		s_ProgramJobSetChangeMenu_actManuscriptPullDownButtonInfo[uc_i].pc_PullButton->activate( False );
		s_ProgramJobSetChangeMenu_actManuscriptPullDownButtonInfo[uc_i].pc_PullButton->showEnable( False );
		s_mManuscript_Info[uc_i].uc_ButtonIndex = 0;
		us_mManuscript_Txt[uc_i] = 0;	// 原稿の表示窓用テキスト初期化.

		switch( s_mManuscript_Info[uc_i].e_Kind ) {
			case PNLD_ManuscriptOneSide:
			case PNLD_ManuscriptTwoSide:
			case PNLD_ManuscriptCoverTwoSide:
				s_mManuscript_Info[uc_i].b_Show = isEnableManuscript( s_mManuscript_Info[uc_i].e_Kind );
				break;
			default:
				break;
		}
	}

	// 表示可能な原稿を設定.
	for( uc_i = 0; uc_i < PNLD_ProgramJobSetChangeMenu_ManuscriptSettings_MaxCnt; uc_i++ ) {
		if( s_mManuscript_Info[uc_i].b_Show ) {
			s_ProgramJobSetChangeMenu_actManuscriptPullDownButtonInfo[uc_Cnt].pc_PullButton->changeText( s_mManuscript_Info[uc_i].us_KindTxt );
			s_ProgramJobSetChangeMenu_actManuscriptPullDownButtonInfo[uc_Cnt].pc_PullButton->activate( True );
			s_ProgramJobSetChangeMenu_actManuscriptPullDownButtonInfo[uc_Cnt].pc_PullButton->showEnable( True );
			s_mManuscript_Info[uc_i].uc_ButtonIndex = static_cast<uchar>( uc_Cnt + 1 );
			us_mManuscript_Txt[uc_Cnt] = s_mManuscript_Info[uc_i].us_KindTxt;	// 表示窓用テキストを変更.
			uc_Cnt++;
		}
	}

	// 原稿プルダウンメニューに有効ボタン数を設定する.
	p_ManuscriptSetting->setEnableItemCnt( uc_Cnt );
	// 原稿プルダウンメニューの行数変更.
	p_ManuscriptSetting->changePullDownRow( uc_Cnt );

	// ジョブスペックから原稿設定を取得.
	TYPC_Scan* pc_Scan = comPanelJob->getJobSpec()->pc_Scan;
	Bool b_OrignalDupMode = pc_Scan->getDuplex();
	if( False == b_OrignalDupMode ) {
		uc_mManuscriptSelectKind = PNLD_ManuscriptOneSide;
	} else if( True == b_OrignalDupMode ) {
		uc_mManuscriptSelectKind = PNLD_ManuscriptTwoSide;
	}

	for( uc_i = 0; uc_i < PNLD_ProgramJobSetChangeMenu_ManuscriptSettings_MaxCnt; uc_i++ ) {
		if( uc_mManuscriptSelectKind == s_mManuscript_Info[uc_i].e_Kind ) {
			e_mManuscriptSelectIndex = s_mManuscript_Info[uc_i].uc_ButtonIndex;
			break;
		}
	}

	// インデックスが1未満の場合は.
	// 対象がプルダウンメニューから外れた事になるので初期状態に設定する.
	if( e_mManuscriptSelectIndex < 1 ) {
		e_mManuscriptSelectIndex = 1;
		uc_mManuscriptSelectKind = 0;
	}

	// ドロップダウンボタン・プルダウンメニューの設定.
	Drp_ProgramJobSetChangeMenu_actManuscript.setSelectNo( e_mManuscriptSelectIndex );
	p_ManuscriptSetting->setSelectMenuNo( e_mManuscriptSelectIndex );

	// 印刷プルダウン初期化.
	uc_Cnt = 0;
	// ベーステーブルからのコピー
	for( i = 0; i < PNLD_ProgramJobSetChangeMenu_PrintSettings_MaxCnt; i++ ) {
		s_mPrint_Info[i].b_Show         = s_ProgramJobSetChangeMenu_PrintSettings_Info[i].b_Show;
		s_mPrint_Info[i].uc_ButtonIndex = s_ProgramJobSetChangeMenu_PrintSettings_Info[i].uc_ButtonIndex;
		s_mPrint_Info[i].e_Kind         = s_ProgramJobSetChangeMenu_PrintSettings_Info[i].e_Kind;
		s_mPrint_Info[i].us_KindTxt     = s_ProgramJobSetChangeMenu_PrintSettings_Info[i].us_KindTxt;
	}

	// ボタンのクリアと表示・非表示設定.
	for( uc_i = 0; uc_i < PNLD_ProgramJobSetChangeMenu_PrintSettings_MaxCnt; uc_i++ ) {
		s_ProgramJobSetChangeMenu_actPrintPullDownButtonInfo[uc_i].pc_PullButton->changeText( s_mPrint_Info[uc_i].us_KindTxt );
		s_ProgramJobSetChangeMenu_actPrintPullDownButtonInfo[uc_i].pc_PullButton->activate( False );
		s_ProgramJobSetChangeMenu_actPrintPullDownButtonInfo[uc_i].pc_PullButton->showEnable( False );
		s_mPrint_Info[uc_i].uc_ButtonIndex = 0;
		us_mPrint_Txt[uc_i] = 0;	// 印刷の表示窓用テキスト初期化.

		switch( s_mPrint_Info[uc_i].e_Kind ) {
			case PNLD_ManuscriptOneSide:
			case PNLD_ManuscriptTwoSide:
			case PNLD_ManuscriptCoverTwoSide:
				s_mPrint_Info[uc_i].b_Show = isEnablePrint( s_mPrint_Info[uc_i].e_Kind );
				break;
			default:
				break;
		}
	}

	// 表示可能な印刷を設定.
	for( uc_i = 0; uc_i < PNLD_ProgramJobSetChangeMenu_PrintSettings_MaxCnt; uc_i++ ) {
		if( s_mPrint_Info[uc_i].b_Show ) {
			s_ProgramJobSetChangeMenu_actPrintPullDownButtonInfo[uc_Cnt].pc_PullButton->changeText( s_mPrint_Info[uc_i].us_KindTxt );
			s_ProgramJobSetChangeMenu_actPrintPullDownButtonInfo[uc_Cnt].pc_PullButton->activate( True );
			s_ProgramJobSetChangeMenu_actPrintPullDownButtonInfo[uc_Cnt].pc_PullButton->showEnable( True );
			s_mPrint_Info[uc_i].uc_ButtonIndex = static_cast<uchar>( uc_Cnt + 1 );
			us_mPrint_Txt[uc_Cnt] = s_mPrint_Info[uc_i].us_KindTxt;	// 表示窓用テキストを変更.
			uc_Cnt++;
		}
	}

	// 印刷プルダウンメニューに有効ボタン数を設定する.
	p_PrintSetting->setEnableItemCnt( uc_Cnt );
	// 印刷プルダウンメニューの行数変更.
	p_PrintSetting->changePullDownRow( uc_Cnt );

	// ジョブスペックから印刷設定を取得.
	TYPC_Print* pc_Print = comPanelJob->getJobSpec()->pc_Print;
	char c_DuplexMode = pc_Print->getDuplexMode();
	if( TYPE_DuplexModeOff == c_DuplexMode ) {
		uc_mPrintSelectKind = PNLD_ManuscriptOneSide;
	} else if( TYPE_DuplexModeOn == c_DuplexMode ) {
		uc_mPrintSelectKind = PNLD_ManuscriptTwoSide;
	}

	for( uc_i = 0; uc_i < PNLD_ProgramJobSetChangeMenu_PrintSettings_MaxCnt; uc_i++ ) {
		if( uc_mPrintSelectKind == s_mPrint_Info[uc_i].e_Kind ) {
			e_mPrintSelectIndex = s_mPrint_Info[uc_i].uc_ButtonIndex;
			break;
		}
	}

	// インデックスが1未満の場合は.
	// 対象がプルダウンメニューから外れた事になるので初期状態に設定する.
	if( e_mPrintSelectIndex < 1 ) {
		e_mPrintSelectIndex = 1;
		uc_mPrintSelectKind = 0;
	}

	// ドロップダウンボタン・プルダウンメニューの設定.
	Drp_ProgramJobSetChangeMenu_actPrint.setSelectNo( e_mPrintSelectIndex );
	p_PrintSetting->setSelectMenuNo( e_mPrintSelectIndex );
}

//------------------------------------------------------------------------------
/**
* @brief A070800700 選択した原稿ボタンのオプション値を取得.
*/
//------------------------------------------------------------------------------
uchar PCPC_ProgramJobSetChangeMenu_act::getManuscriptKindId( uchar e_index )
{
	uchar uc_i = 0;
	uchar e_retKind = PNLD_ManuscriptOneSide;

	for( uc_i = 0; uc_i < PNLD_ProgramJobSetChangeMenu_ManuscriptSettings_MaxCnt; uc_i++ ) {
		if( s_mManuscript_Info[uc_i].uc_ButtonIndex == e_index ) {
			e_retKind = s_mManuscript_Info[uc_i].e_Kind;
			break;
		}
	}
	return e_retKind;
}

//------------------------------------------------------------------------------
/**
* @brief A070800700 指定した原稿設定の使用可否.
*/
//------------------------------------------------------------------------------
Bool PCPC_ProgramJobSetChangeMenu_act::isEnableManuscript( uchar e_Kind )
{
	Bool b_Enable = True;

	if( PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_DuplexCombine )        == False ||		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
	    PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_DuplexCombine_Duplex ) == False ) {		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
		b_Enable = False;
	} else {
		switch( e_Kind ) {
			case PNLD_ManuscriptOneSide:
				if( PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_DuplexCombine_Duplex_1To1 ) == False &&		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
				    PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_DuplexCombine_Duplex_1To2 ) == False ) {		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
					b_Enable = False;
				}
				break;
			case PNLD_ManuscriptTwoSide:
				if( PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_DuplexCombine_Duplex_2To1 ) == False &&		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
				    PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_DuplexCombine_Duplex_2To2 ) == False ) {		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
					b_Enable = False;
				}
				break;
			default:
				b_Enable = False;
				break;
		}
	}
	return b_Enable;
}

//関数--------------------------------------------------------------------------
//概要    ：インデックス用紙サイズのセットアップ
//備考    ：.
//戻値    ：.
//作成者  ：Sky_金田.
//レビュー：.
//変更履歴：.
//------------------------------------------------------------------------------
/**
 * @brief A070800700 プログラムジョブ停止中_設定変更メニュー原稿束 : インデックス用紙サイズのセットアップ
 */
//------------------------------------------------------------------------------
void PCPC_ProgramJobSetChangeMenu_act::setupIndexPaperSize()
{
#ifdef DEF_LCD_7INCH
	ushort 			us_text 		 = s_pITMOFF_W152H018A;										// OFF.
#else // #ifdef DEF_LCD_7INCH.
	ushort 			us_text 		 = s_pBDGOff_W080H016A;										// OFF.
#endif // #ifdef DEF_LCD_7INCH.
// Comment 2019/07/16 【IT6_EagleH】【HCL_Prabakaran】OP_BTS-8347 jobSpec Type has been changed for common use.
//	APIC_Job* pc_JobCopy			 = PnlComGetJobCopy();
	APIC_Job* pc_JobCopy			 = comPanelJob;
	if( NULL == pc_JobCopy ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return;}
	TYPC_JobSpec*	pc_JobSpec		 = pc_JobCopy->getJobSpec();
	Bool			bl_OriginalIndex = pc_JobSpec->pc_Print->c_ImageRotate.getIndexOriginal();	// インデックス原稿設定.

	// インデックス原稿設定ONの場合.
	if( True == bl_OriginalIndex ) {

		// サイズコード取得.
		TYPE_SizeCode sizeCode = pc_JobSpec->pc_Scan->c_OriginalSize.getPaperSize().e_SizeCode;

		// サイズコードチェック.
		switch( sizeCode ) {
#ifdef DEF_LCD_7INCH
			case TYPE_SizeCodeA3Sef:
				us_text = s_pNOTSheetSize_A3SEF_W144H018A; 		break; // A3縦.
			case TYPE_SizeCodeB4Sef:
				us_text = s_pNOTSheetSize_B4SEF_W144H018A;		break; // B4縦.
			case TYPE_SizeCodeA4Lef:
				us_text = s_pNOTSheetSize_A4LEF_W144H018A;		break; // A4横.
			case TYPE_SizeCodeA4Sef:
				us_text = s_pNOTSheetSize_A4SEF_W144H018A;		break; // A4縦.
			case TYPE_SizeCodeB5Sef:
				us_text = s_pNOTSheetSize_B5SEF_W144H018A;		break; // B5縦.
			case TYPE_SizeCodeLegalSef:
				us_text = s_pNOTSheetSize_812x14SEF_W144H018A;	break; // Legal縦.
			case TYPE_SizeCodeLetterLef:
				us_text = s_pNOTSheetSize_812x11LEF_W144H018A;	break; // Letter横.
			case TYPE_SizeCodeLetterSef:
				us_text = s_pNOTSheetSize_812x11SEF_W144H018A;	break; // Letter縦.
#else // #ifdef DEF_LCD_7INCH.
			case TYPE_SizeCodeA3Sef:
				us_text = s_pNOTSheetSize_A3SEF_W144H016A; 		break; // A3縦.
			case TYPE_SizeCodeB4Sef:
				us_text = s_pNOTSheetSize_B4SEF_W144H016A;		break; // B4縦.
			case TYPE_SizeCodeA4Lef:
				us_text = s_pNOTSheetSize_A4LEF_W144H016A;		break; // A4横.
			case TYPE_SizeCodeA4Sef:
				us_text = s_pNOTSheetSize_A4SEF_W144H016A;		break; // A4縦.
			case TYPE_SizeCodeB5Sef:
				us_text = s_pNOTSheetSize_B5SEF_W144H016A;		break; // B5縦.
			case TYPE_SizeCodeLegalSef:
				us_text = s_pNOTSheetSize_812x14SEF_W144H016A;	break; // Legal縦.
			case TYPE_SizeCodeLetterLef:
				us_text = s_pNOTSheetSize_812x11LEF_W144H016A;	break; // Letter横.
			case TYPE_SizeCodeLetterSef:
				us_text = s_pNOTSheetSize_812x11SEF_W144H016A;	break; // Letter縦.
#endif // #ifdef DEF_LCD_7INCH.
			default: break;
		}
	}

	Key_ProgramJobSetChangeMenu_actIndexPaperSize.changeText( &Ktx_ProgramJobSetChangeMenu_actIndexPaperSize_Sub, us_text );
}

//------------------------------------------------------------------------------
/**
* @brief A070800700 原稿プルダウンのコールバック処理.
*/
//------------------------------------------------------------------------------
void PCPC_ProgramJobSetChangeMenu_act::ans_ManuscriptPullDownEnd( uchar uc_Param, short ss_Param, void* p_Param, uint32_t ul_Param )
{
	TYPC_Print* pc_Print = comPanelJob->getJobSpec()->pc_Print;
	if( NULL == pc_Print ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }
	// インデックスを保持.
	pc_mProgramJobSetChangeMenu_act->e_mManuscriptSelectIndex = uc_Param;
	// インデックスに応じたオプション値を設定.
	pc_mProgramJobSetChangeMenu_act->uc_mManuscriptSelectKind = pc_mProgramJobSetChangeMenu_act->getManuscriptKindId( pc_mProgramJobSetChangeMenu_act->e_mManuscriptSelectIndex );

	// 原稿を設定.
	// オプション値によって処理分岐.
	switch( pc_mProgramJobSetChangeMenu_act->uc_mManuscriptSelectKind ) {
		case PNLD_ManuscriptOneSide:
			setPJFaceSelect( False, pc_Print->getDuplexMode() );		// 原稿片面.
			break;
		case PNLD_ManuscriptTwoSide:
			setPJFaceSelect( True, pc_Print->getDuplexMode() );			// 原稿片面.
			break;
	}
}

//------------------------------------------------------------------------------
/**
* @brief A070800700 選択した印刷ボタンのオプション値を取得.
*/
//------------------------------------------------------------------------------
uchar PCPC_ProgramJobSetChangeMenu_act::getPrintKindId( uchar e_index )
{
	uchar uc_i = 0;
	uchar e_retKind = PNLD_ManuscriptOneSide;

	for( uc_i = 0; uc_i < PNLD_ProgramJobSetChangeMenu_PrintSettings_MaxCnt; uc_i++ ) {
		if( s_mPrint_Info[uc_i].uc_ButtonIndex == e_index ) {
			e_retKind = s_mPrint_Info[uc_i].e_Kind;
			break;
		}
	}
	return e_retKind;
}

//------------------------------------------------------------------------------
/**
* @brief A070800700 指定した印刷設定の使用可否.
*/
//------------------------------------------------------------------------------
Bool PCPC_ProgramJobSetChangeMenu_act::isEnablePrint( uchar e_Kind )
{
	Bool b_Enable = True;

	if( PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_DuplexCombine )        == False ||		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
	    PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_DuplexCombine_Duplex ) == False ) {		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
		b_Enable = False;
	} else {
		switch( e_Kind ) {
			case PNLD_ManuscriptOneSide:
				if( PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_DuplexCombine_Duplex_1To1 ) == False &&		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
				    PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_DuplexCombine_Duplex_2To1 ) == False ) {		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
					b_Enable = False;
				}
				break;
			case PNLD_ManuscriptTwoSide:
				if( PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_DuplexCombine_Duplex_1To2 ) == False &&		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
				    PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_DuplexCombine_Duplex_2To2 ) == False ) {		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
					b_Enable = False;
				}
				break;
			default:
				b_Enable = False;
				break;
		}
	}
	return b_Enable;
}

//------------------------------------------------------------------------------
/**
* @brief A070800700 印刷プルダウンのコールバック処理.
*/
//------------------------------------------------------------------------------
void PCPC_ProgramJobSetChangeMenu_act::ans_PrintPullDownEnd( uchar uc_Param, short ss_Param, void* p_Param, uint32_t ul_Param )
{
	TYPC_Scan* pc_Scan = comPanelJob->getJobSpec()->pc_Scan;
	if( NULL == pc_Scan ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }
	// インデックスを保持.
	pc_mProgramJobSetChangeMenu_act->e_mPrintSelectIndex = uc_Param;
	// インデックスに応じたオプション値を設定.
	pc_mProgramJobSetChangeMenu_act->uc_mPrintSelectKind = pc_mProgramJobSetChangeMenu_act->getPrintKindId( pc_mProgramJobSetChangeMenu_act->e_mPrintSelectIndex );

	// 印刷を設定.
	// オプション値によって処理分岐.
	switch( pc_mProgramJobSetChangeMenu_act->uc_mPrintSelectKind ) {
		case PNLD_ManuscriptOneSide:
			setPJFaceSelect( pc_Scan->getDuplex(), TYPE_DuplexModeOff );		// 印刷片面.
			break;
		case PNLD_ManuscriptTwoSide:
			setPJFaceSelect( pc_Scan->getDuplex(), TYPE_DuplexModeOn );			// 印刷片面.
			break;
	}
}
// 他のプルダウンと処理を合わせるためメソッド追加.
void PCPC_ProgramJobSetChangeMenu_act::initManuscriptCloseAndCarryOut()
{
	TYPC_Scan* pc_Scan = comPanelJob->getJobSpec()->pc_Scan;
	if( NULL == pc_Scan ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return;}

	// 原稿のとじ代の方向の取得とローテーションボタンのインデックス設定.
	TYPE_BindDirection e_BindDirection = pc_Scan->c_BindDirection.getBindDirection();
	uchar uc_SelectPullDownMenu = 0;

	switch( e_BindDirection ) {
		case TYPE_BindDirectionAuto:	uc_SelectPullDownMenu = PNLD_PullDownProgramJobSetChangeMenu_actBindPositionAuto;		break;	// 自動.
		case TYPE_BindDirectionLeft:	uc_SelectPullDownMenu = PNLD_PullDownProgramJobSetChangeMenu_actBindPositionLeft;		break;	// 左.
		case TYPE_BindDirectionTop:		uc_SelectPullDownMenu = PNLD_PullDownProgramJobSetChangeMenu_actBindPositionUp;			break;	// 上.
		case TYPE_BindDirectionRight:	uc_SelectPullDownMenu = PNLD_PullDownProgramJobSetChangeMenu_actBindPositionRight;		break;	// 右.
		default:	LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );	break;
	}
	Drp_ProgramJobSetChangeMenu_actBindPosition.getPullDownMenu()->setSelectMenuNo( uc_SelectPullDownMenu );
}
void PCPC_ProgramJobSetChangeMenu_act::ans_Manuscript( uchar uc_Param, short ss_Param, void* p_Param, uint32_t ul_Param )
{
	TYPC_Scan* pc_Scan = comPanelJob->getJobSpec()->pc_Scan;
	if( NULL == pc_Scan ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }

	TYPE_BindDirection e_BindDirection = pc_Scan->c_BindDirection.getBindDirection();	// 原稿のとじ代の方向を設定.

	switch( uc_Param ) {
		case PNLD_PullDownProgramJobSetChangeMenu_actBindPositionAuto:	e_BindDirection = TYPE_BindDirectionAuto;	break;	// 自動.
		case PNLD_PullDownProgramJobSetChangeMenu_actBindPositionLeft:	e_BindDirection = TYPE_BindDirectionLeft;	break;	// 左.
		case PNLD_PullDownProgramJobSetChangeMenu_actBindPositionUp:	e_BindDirection = TYPE_BindDirectionTop;	break;	// 上.
		case PNLD_PullDownProgramJobSetChangeMenu_actBindPositionRight:	e_BindDirection = TYPE_BindDirectionRight;	break;	// 右.
		default:	LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );	break;
	}
	pc_Scan->c_BindDirection.setBindDirection( e_BindDirection );
}

void PCPC_ProgramJobSetChangeMenu_act::setupSelectedBindPositionType()
{

	// 原稿のとじ代の方向の取得とローテーションボタンのインデックス設定.

	TYPC_Scan* pc_Scan = comPanelJob->getJobSpec()->pc_Scan;
	if( NULL == pc_Scan ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return;}

	// 原稿のとじ代の方向の取得とローテーションボタンのインデックス設定.
	TYPE_BindDirection e_BindDirection = pc_Scan->c_BindDirection.getBindDirection();

	switch( e_BindDirection ) {
		case TYPE_BindDirectionAuto:	uc_mSelectBindDirection = PNLD_PullDownProgramJobSetChangeMenu_actBindPositionAuto;		break;	// 自動.
		case TYPE_BindDirectionLeft:	uc_mSelectBindDirection = PNLD_PullDownProgramJobSetChangeMenu_actBindPositionLeft;		break;	// 左.
		case TYPE_BindDirectionTop:		uc_mSelectBindDirection = PNLD_PullDownProgramJobSetChangeMenu_actBindPositionUp;		break;	// 上.
		case TYPE_BindDirectionRight:	uc_mSelectBindDirection = PNLD_PullDownProgramJobSetChangeMenu_actBindPositionRight;	break;	// 右.
		default:	LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );	break;
	}
	Drp_ProgramJobSetChangeMenu_actBindPosition.setSelectNo( uc_mSelectBindDirection );
}
//------------------------------------------------------------------------------
/**
* @brief A070800700 ポップアップウインドウサイズ変更要求.
*/
//------------------------------------------------------------------------------
void PCPC_ProgramJobSetChangeMenu_act::resizePopup()
{
	// 親クラスをコール.
	PPopupDecisionButtonCom::resizePopup();
#ifndef DEF_MonoCus
	if( True == APIC_SoftDipSw::newInstance()->getSoftDipSwBitForPanelSpecNo( BYPASS_SETTING, 6 ) ) {	// MZ-144 千代田化工建設様追加対応要望(2nd Step).
#ifdef DEF_LCD_7INCH
		changeWindow( PNLD_POPUP_START_X( 780 ), PNLD_POPUP_START_Y( 452, PNLD_POPUP_BUTTON_EXIST ), 780, 452 );
#else // #ifdef DEF_LCD_7INCH.
		changeWindow( PNLD_POPUP_START_X( 748 ), PNLD_POPUP_START_Y( 424, PNLD_POPUP_BUTTON_EXIST ), 748, 424 );
#endif // #ifdef DEF_LCD_7INCH.
	} else {
#ifdef DEF_LCD_7INCH
		changeWindow( PNLD_POPUP_START_X( 780 ), PNLD_POPUP_START_Y( 412, PNLD_POPUP_BUTTON_EXIST ), 780, 412 );
#else // #ifdef DEF_LCD_7INCH.
		changeWindow( PNLD_POPUP_START_X( 748 ), PNLD_POPUP_START_Y( 372, PNLD_POPUP_BUTTON_EXIST ), 748, 372 );
#endif // #ifdef DEF_LCD_7INCH.
	}
#endif // #ifndef DEF_MonoCus.
}

//------------------------------------------------------------------------------
/**
* @brief A070800700 パーツ差し替え関数.
*/
//------------------------------------------------------------------------------
void PCPC_ProgramJobSetChangeMenu_act::setChangeDisplayParts()
{
#ifndef DEF_MonoCus
	// 本ポップアップ画面の表示開始座標をオフセットとして計算して使用する.
	short ss_OffsetX = 0;
	short ss_OffsetY = 0;
	getPosition( &ss_OffsetX, &ss_OffsetY );
	if( True == APIC_SoftDipSw::newInstance()->getSoftDipSwBitForPanelSpecNo( BYPASS_SETTING, 6 ) ) {	// MZ-144 千代田化工建設様追加対応要望(2nd Step).
#ifdef DEF_LCD_7INCH
		Bak_ProgramJobSetChangeMenu_actBase.changeSize( 780, 452 );						// ベース画面背景.
		Bak_ProgramJobSetChangeMenu_actBasicTitleBackground.changeSize( 236, 340 );		// 基本設定背景.
		Key_ProgramJobSetChangeMenu_actOK.changePosition( static_cast<short>( ss_OffsetX + 704 ), static_cast<short>( ss_OffsetY + 428 ) );		// OK.
		Key_ProgramJobSetChangeMenu_actCancel.changePosition( static_cast<short>( ss_OffsetX + 632 ), static_cast<short>( ss_OffsetY + 428 ) );	// 中止.
#else // #ifdef DEF_LCD_7INCH.
		Bak_ProgramJobSetChangeMenu_actBase.changeSize( 748, 424 );						// ベース画面背景.
		Bak_ProgramJobSetChangeMenu_actBasicTitleBackground.changeSize( 228, 308 );		// 基本設定背景.
		Key_ProgramJobSetChangeMenu_actOK.changePosition( static_cast<short>( ss_OffsetX + 688 ), static_cast<short>( ss_OffsetY + 400 ) );		// OK.
		Key_ProgramJobSetChangeMenu_actCancel.changePosition( static_cast<short>( ss_OffsetX + 632 ), static_cast<short>( ss_OffsetY + 400 ) );	// 中止.
#endif // #ifdef DEF_LCD_7INCH.
	} else {
#ifdef DEF_LCD_7INCH
		Bak_ProgramJobSetChangeMenu_actBase.changeSize( 780, 412 );						// ベース画面背景.
		Bak_ProgramJobSetChangeMenu_actBasicTitleBackground.changeSize( 236, 284 );		// 基本設定背景.
		Key_ProgramJobSetChangeMenu_actOK.changePosition( static_cast<short>( ss_OffsetX + 704 ), static_cast<short>( ss_OffsetY + 388 ) );		// OK.
		Key_ProgramJobSetChangeMenu_actCancel.changePosition( static_cast<short>( ss_OffsetX + 632 ), static_cast<short>( ss_OffsetY + 388 ) );	// 中止.
#else // #ifdef DEF_LCD_7INCH.
		Bak_ProgramJobSetChangeMenu_actBase.changeSize( 748, 372 );						// ベース画面背景.
		Bak_ProgramJobSetChangeMenu_actBasicTitleBackground.changeSize( 228, 256 );		// 基本設定背景.
		Key_ProgramJobSetChangeMenu_actOK.changePosition( static_cast<short>( ss_OffsetX + 688 ), static_cast<short>( ss_OffsetY + 348 ) );		// OK.
		Key_ProgramJobSetChangeMenu_actCancel.changePosition( static_cast<short>( ss_OffsetX + 632 ), static_cast<short>( ss_OffsetY + 348 ) );	// 中止.
#endif // #ifdef DEF_LCD_7INCH.
	}
#endif // #ifndef DEF_MonoCus.
	m_bl_InitialOneTime = True;
}

/*================================================================================================.
	テーブルの定義.
================================================================================================*/
PNLS_PnlTpl_ReadSize st_tbl_A070800110 = {
	PNLE_ReadSize_Scr_A070800110,
	&Cmp_ActTabOriginalSize,
	&Cmp_OriginalSizeUnfix_MessageArea,
	NULL,
	NULL,
	&Rbt_ActMetricSizes,
	&Rbt_ActInchSizes,
	&Rbt_ActOtherSizes,
	&Rbt_ActPhotoSizesME,
	&Rbt_ActPhotoSizesMC,
	&Rbt_ActPhotoSizesMJ,
	&Key_ActSheetSize_PostcardSEF,
	&Key_ActSheetSize_PostcardLEF,
#ifdef DEF_A4_Cus	// 2015/08/04 VenusMLK OPS2高田(庸) A3A4共通化コンフリクト対応.
	&Key_ActSheetSize_DoublePostcardSEF,
// VenusMLK 管理No.1434_14_4,5 OPS2 Sky後藤(拓) 2015/10/28 Start.
	&Key_ActSheetSize_B5ISOSEF_JP,
	&Key_ActSheetSize_B5ISOSEF_EUUS,
// VenusMLK 管理No.1434_14_4,5 OPS2 Sky後藤(拓) 2015/10/28 End.
#endif // DEF_A4_Cus	// 2015/08/04 VenusMLK OPS2高田(庸) A3A4共通化コンフリクト対応.
	&Key_ActSheetSize_8x13SEF,
	NULL,
	NULL,
	&Bak_ActChildTabBase
};

//関数------------------------------------------------------------------------------------------------------------------
//概要    ：画面開始処理..
//備考    ：.
//戻値    ：.
//作成者  ：AWS_ウィング.
//レビュー：.
//変更履歴：.
//------------------------------------------------------------------------------
/**
* @brief A070800110 原稿サイズ : 画面開始処理.
*/
//------------------------------------------------------------------------------
void PCPC_SetChangeOriginalSizeMetric_act::startGamen()
{
	PPopupDecisionButtonCom::startGamen();		// 親クラスをコールする.

	// タブコンポーネントへコールバック関数設定.
	Cmp_ActTabOriginalSize.setCallBackFunc( ans_TabKey );
	Cmp_ActTabOriginalSize.setChildSpaceColor( PNLE_TabChildSpaceColorPaletteAdmin );
	Cmp_ActTabOriginalSize.normalTabTypeChange( PNLD_AUTOSIZETABTYPE_WARN );
	m_PnlTpl_ReadSize.cmn_startGamen();

	// ポップアップメッセージエリア設定.
	Cmp_OriginalSizeUnfix_MessageArea.setNowPopupGamen( PNLE_UnKnownPopup );
	Cmp_OriginalSizeUnfix_MessageArea.setMode( PNLE_PopupMessageAreaMessageModeAdmin );

#ifdef DEF_LCD_7INCH
	Cmp_ActTabOriginalSize.setCmpAreaSize( 348 , 160 );
#else // #ifdef DEF_LCD_7INCH.
	Cmp_ActTabOriginalSize.setCmpAreaSize( 332 , 160 );
#endif // #ifdef DEF_LCD_7INCH.
// comment 2014/01/15 OPS1-Sky浜野 [Poseidon][CQ508992].
	m_bl_Closed = False;
}

//関数------------------------------------------------------------------------------------------------------------------
//概要    ：表示部品のセットアップ.
//備考    ：.
//戻値    ：.
//作成者  ：AWS_ウィング.
//レビュー：.
//変更履歴：.
//------------------------------------------------------------------------------
/**
* @brief A070800110 原稿サイズ : 表示部品のセットアップ
*/
//------------------------------------------------------------------------------
void PCPC_SetChangeOriginalSizeMetric_act::setupGamen( short option )
{
// comment 2014/01/15 OPS1-Sky浜野 [Poseidon][CQ508992].
	// closeTop後に呼び出された場合.
	// 自インスタンスが解放されている場合に不正アクセスとなるため、実行を抑止する.
	if ( True == m_bl_Closed ) {
		return;
	}
	PPopupDecisionButtonCom::setupGamen();		// 親クラスをコールする.
	
	Bool bl_Auto = (( TYPE_KeyModeVendor == APIC_KeyDevice::getKeyMode() )? False : True );
	m_PnlTpl_ReadSize.cmn_setupGamen( bl_Auto );

	if( True == comPanelJob->getJobSpec()->pc_Common->c_JobType.getProgramJobOn() ) {
		Cmp_OriginalSizeUnfix_MessageArea.changeMainMessageText( s_pMSGPgJobManuscriptBunch_W448H034A );
	} else {
		Cmp_OriginalSizeUnfix_MessageArea.changeMainMessageText( NULL );
	}

// comment 2020/07/14 SCD2-Sky 石原 [IT6_2.0MR2][OP_BTS-25362] フッターエリア対応.
#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus)  && defined (DEF_LCD_7INCH) ) )
	PnlComFooterPartsBtnActiveChange(homeKeyActiveCheck(), idKeyActiveCheck(), resetKeyActiveCheck());
#endif		//  ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus)  && defined (DEF_LCD_7INCH) ) )	.
}
//関数------------------------------------------------------------------------------------------------------------------
//概要    ：タブ選択をセットアップする..
//備考    ：.
//戻値    ：.
//作成者  ：AWS_ウィング.
//レビュー：.
//変更履歴：.
//------------------------------------------------------------------------------
/**
* @brief A070800110 原稿サイズ : タブ選択をセットアップする.
*/
//------------------------------------------------------------------------------
void PCPC_SetChangeOriginalSizeMetric_act::selectTabSetup()
{
	TYPC_Scan* pc_Scan = onlyPanel.cc_DisplayJob->getJobSpec()->pc_Scan;
	// 設定値による、タブ選択処理を追加 管理No.893_28 選択確定キーの仕様修正 酒井健.
	TYPC_PaperSize c_PaperSize = pc_Scan->c_OriginalSize.getPaperSize();				// 原稿サイズ取得.
	char sc_SizeType = PnlComJudgePaperSizeKind( c_PaperSize );

#ifndef DEF_A4_Cus	// VenusMLK OPS2 Sky高田(庸) 2015/07/10.
	if( False == pc_Scan->c_OriginalSize.getAutoSize() ) {
#endif // DEF_A4_Cus	// VenusMLK OPS2 Sky高田(庸) 2015/07/10.
		switch( sc_SizeType ) {
			case PNLCOM_METRICSIZE:								// メトリック選択.
				Cmp_ActTabOriginalSize.selectTab( PNLD_TabMetricSizes );
				break;
			case PNLCOM_INCHSIZE:								// インチ選択.
				Cmp_ActTabOriginalSize.selectTab( PNLD_TabInchSizes );
				break;
			case PNLCOM_816SIZE:								// 8開16開選択.
			case PNLCOM_FLSSIZE:								// FLS選択.
			case PNLCOM_POSTCARDSIZE:							// はがきサイズ選択.
			case PNL_COM_4_6_SEFSIZE:							// 4×6選択.
				Cmp_ActTabOriginalSize.selectTab( PNLD_TabOtherSizes );
				break;
			case PNLCOM_PHOTOSIZE:								// 写真サイズ選択.
				Cmp_ActTabOriginalSize.selectTab( PNLD_TabPhotoSizes );
				break;
			default:									break;
		}
#ifndef DEF_A4_Cus	// VenusMLK OPS2 Sky高田(庸) 2015/07/10.
	} else {
		Cmp_ActTabOriginalSize.selectTab( PNLD_TabAutoSize );
	}
#endif // DEF_A4_Cus	// VenusMLK OPS2 Sky高田(庸) 2015/07/10.
}
//-----------------------------------------------------------------------------------------------
/**
 *	@brief	A070800110 原稿サイズ : タブキー選択時コールバック関数.
 */
//-----------------------------------------------------------------------------------------------
void PCPC_SetChangeOriginalSizeMetric_act::ans_TabKey( uchar option )
{
// comment 2014/01/15 OPS1-Sky浜野 [Poseidon][CQ508992].
//	TYPC_Scan* pc_Scan = onlyPanel.cc_DisplayJob->getJobSpec()->pc_Scan;
	//タブインデックス押下時にセットアップ画面をコール.
	gp_SetChangeOriginalSizeMetric_act.changeDefaultBackParts();
	gp_SetChangeOriginalSizeMetric_act.b_ShowBack = True;
	gp_SetChangeOriginalSizeMetric_act.setupGamen( option );

	switch( Cmp_ActTabOriginalSize.getTabNo() ) {
		case PNLD_TabOtherSizes:
			gp_SetChangeOriginalSizeMetric_act.m_PnlTpl_ReadSize.cmn_setupPostCardKey();
			break;
		case PNLD_TabPhotoSizes:
			gp_SetChangeOriginalSizeMetric_act.m_PnlTpl_ReadSize.cmn_setupPhotoTabContents();
			break;
		case PNLD_TabAutoSize:
// comment 2014/01/15 OPS1-Sky浜野 [Poseidon][CQ508992].
//	       pc_Scan->c_OriginalSize.setAutoSize( True );		// 全面画像・OFF.
//           Cmp_ActTabOriginalSize.selectTab( PNLD_TabAutoSize );	// 自動タブを選択  伊丹Sky(坂梨)  2011/08/17 CQ352807.
//	       // 本画面を閉じ.
//	       PNLC_PopupListControl::newInstance()->closeTop( True );  
			CallBackIndi( TYPE_PNL_CallBack_PopupCloseExec );
			break;
		default:
			break;
	}

	gp_SetChangeOriginalSizeMetric_act.setupGamen( 0 );
}
//-----------------------------------------------------------------------------------------------
/**
 *	@brief	A020121100_原稿サイズ : タブ切替後のデフォルト背景変更処理..
 */
//-----------------------------------------------------------------------------------------------
void PCPC_SetChangeOriginalSizeMetric_act::changeDefaultBackParts()
{
	switch( Cmp_ActTabOriginalSize.getTabNo() ) {
		case PNLD_TabMetricSizes:
		case PNLD_TabInchSizes:
		case PNLD_TabOtherSizes:
		case PNLD_TabPhotoSizes:
			Bak_ActChildTabBase.showEnable( True );
			break;
		case PNLD_TabAutoSize:
			Bak_ActChildTabBase.showEnable( False );
			break;
		default: break;
	}
}


//-----------------------------------------------------------------------------------------------
/**
 *	@brief	A070800110 : スタートキー押下処理..
 */
//-----------------------------------------------------------------------------------------------
void PCPC_SetChangeOriginalSizeMetric_act::procStart( uchar option )
{
// Comment 2019/07/16 【IT6_EagleH】【HCL_Prabakaran】OP_BTS-8347 jobSpec Type has been changed for common use.
//	APIC_Job* pc_Job = PnlComGetJobCopy();
	APIC_Job* pc_Job = comPanelJob;
	if( NULL == pc_Job ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );	return; }	// NULLチェックを追加.
	TYPC_Common* pc_Common = pc_Job->getJobSpec()->pc_Common;
	if( False == onlyPanel.bl_MovingConfirmMethodGamen ) {		// 確認コピー選択画面遷移処理中の時は設定しない.
// Comment 2019/07/16 【IT6_EagleH】【HCL_Prabakaran】OP_BTS-8347 jobSpec Type has been changed for common use.
//		if( isOn( CNK_CatalogBind, PnlComGetJobCopy( sc_JobPutNo ) ) == True		// カタログ連写が設定されている時は、原稿の向きはランドスケープ固定.
//		    || isOn( CNK_CatalogCopy, PnlComGetJobCopy( sc_JobPutNo ) ) == True ) {
		if( isOn( CNK_CatalogBind, comPanelJob ) == True		// カタログ連写が設定されている時は、原稿の向きはランドスケープ固定.
		    || isOn( CNK_CatalogCopy, comPanelJob ) == True ) {
			pc_Common->c_Orientation.setOrientation( TYPE_OrientationLandscape );
			selectAutoPaperMag();
		}
	}
	if( ( PNLE_APPMODE_SCANFAX == PnlComGetAppMode() ) && ( False == comPanelJob->getJobSpec()->pc_Scan->c_MemoryIn.getMemoryInContinue() ) ) {
//Comment 2019/12/12 SCD2 K.Kasemura  [EagleZ+][OP_BTS-15196] 送信中ジョブが無いにもかかわらず、送信中ジョブの警告が表示される.
//		SP_StatusCheck  sp_StatusCheck = isMachineStatusChack();  // 送信中ステータスのチェック結果.
		if( ( True == PnlComIsEnableSearchablePDFAutoExtrFileNameConfirm( comPanelJob ) )
//		    && ( True == sp_StatusCheck.bl_NowTrans ) ) {
			&& ( True == isFaxJobStatusCheck() ) ) {
			PNLC_PopupListControl::newInstance()->addPopup( &gp_ConfirmContinualOperation_act_warn );
			return;
		}
	}
	PNLC_ActionDataKeyControlLib::newInstance()->procStartActionReadStop( option );
}

void PCPC_SetChangeOriginalSizeMetric_act::procStop( uchar option )
{
	PNLC_ActionDataKeyControlLib* pc_ActionDataKeyControlLib = PNLC_ActionDataKeyControlLib::newInstance();
	pc_ActionDataKeyControlLib->procStopActionReadStop( option );
}

//関数------------------------------------------------------------------------------------------------------------------
//概要    ：リセットボタン（押された)..
//備考    ：.
//戻値    ：.
//作成者  ：Sky鈴木理.
//レビュー：.
//変更履歴：.
//------------------------------------------------------------------------------
/**
* @brief A070800110 原稿サイズ : ボタン（押された).
*/
//------------------------------------------------------------------------------
void PCPC_SetChangeOriginalSizeMetric_act::procReset( uchar option )
{
	// 変更前の状態に復帰させる.
	if( True == comPanelJob->getJobSpec()->pc_Common->c_JobType.getProgramJobOn() ) {
		c_ActSettingChangeSampleCopy.PnlRecoverySettingSampleCopySpecInfo( comPanelJob->getJobSpec(), comPanelJob->getJobStatus() );
	} else {
		c_SettingChengeBasicInfo.PnlRecoverySettingBasicInfo( 1 );
	}
	PNLC_PopupListControl::newInstance()->closeTop( True );
// comment 2014/01/15 OPS1-Sky浜野 [Poseidon][CQ508992].
	m_bl_Closed = True;
}
// comment 2020/07/14 SCD2-Sky 石原 [IT6_2.0MR2][OP_BTS-25362] フッターエリア対応.
//------------------------------------------------------------------------------
/**
* @brief A070800110 原稿サイズ : リセットキー活性状態.
*/
//------------------------------------------------------------------------------
Bool PCPC_SetChangeOriginalSizeMetric_act::resetKeyActiveCheck()
{
	ACTIVEKEY_RET( True );
}
//関数------------------------------------------------------------------------------------------------------------------
//概要    ：ボタン（押された)..
//備考    ：.
//戻値    ：.
//作成者  ：AWS_ウィング.
//レビュー：.
//変更履歴：.
//------------------------------------------------------------------------------
/**
* @brief A070800110 原稿サイズ : ボタン（押された).
*/
//------------------------------------------------------------------------------
void PCPC_SetChangeOriginalSizeMetric_act::procButtonOn( uchar option )
{
#ifdef DEF_A4_Cus
	// 仕向けを判定.
	TEXT_ID puc_StrPostCardT = s_pKEYSheetSize_PostcardSEF_W096H028A;
	switch ( PnlComGetDestination() ) {		// 仕向けを判定.
	    // 日本仕向け.
	    case TYPE_SystemType_JP:
	        puc_StrPostCardT = s_pKEYSheetSize_PostcardSEF_W096H028A;		// はがき.
	        break;
	    // US仕向け.
	    case TYPE_SystemType_US:
	        puc_StrPostCardT = s_pNOTSheetSize_4x6SEF_W096H028A;		// 4X6.
	        break;
	    // EU仕向け.
	    case TYPE_SystemType_EU:
	        puc_StrPostCardT = s_pKEYSheetSize_A6CardSEF_W096H028A;		// A6カード.
	        break;
	    default:
			LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
	        break;
	}
#else
	// 仕向けを判定.
	TEXT_ID puc_StrPostCardT = s_pKEYSheetSize_PostcardSEF_W096H028A;
	TEXT_ID puc_StrPostCardY = s_pKEYSheetSize_PostcardLEF_W096H028A;
	switch ( PnlComGetDestination() ) {		// 仕向けを判定.
	    // 日本仕向け.
	    case TYPE_SystemType_JP:
	        puc_StrPostCardT = s_pKEYSheetSize_PostcardSEF_W096H028A;		// はがき.
	        puc_StrPostCardY = s_pKEYSheetSize_PostcardLEF_W096H028A;
	        break;
	    // US仕向け.
	    case TYPE_SystemType_US:
	        puc_StrPostCardT = s_pNOTSheetSize_4x6SEF_W096H028A;		// 4X6.
	        puc_StrPostCardY = s_pNOTSheetSize_4x6LEF_W096H028A;
	        break;
	    // EU仕向け.
	    case TYPE_SystemType_EU:
	        puc_StrPostCardT = s_pKEYSheetSize_A6CardSEF_W096H028A;		// A6カード.
	        puc_StrPostCardY = s_pKEYSheetSize_A6CardLEF_W096H028A;
	        break;
		default:
			m_PnlTpl_ReadSize.cmn_procComButtonOnMetricSize( option );		// 原稿サイズのボタン（押された)処理.
			m_PnlTpl_ReadSize.cmn_procComButtonOnInchSize( option );
			m_PnlTpl_ReadSize.cmn_procComButtonOnOthers( option );
			m_PnlTpl_ReadSize.cmn_procComButtonOnPhotoSize( option );
			break;
	}
#endif
#ifndef DEF_A4_Cus	// A3A4FUM OPS2 Sky後藤(拓) 2015/01/19.
	Key_ActSheetSize_PostcardLEF.changeText( puc_StrPostCardY );		// はがきサイズの文字列切り替え.
#endif // DEF_A4_Cus	// A3A4FUM OPS2 Sky後藤(拓) 2015/01/19.
	Key_ActSheetSize_PostcardSEF.changeText( puc_StrPostCardT );

#ifdef DEF_A4_Cus	// A3A4FUM OPS2 Sky後藤(拓) 2015/01/19.
	// 往復はがきの表示・非表示.
	if (PnlComGetDestination() == TYPE_SystemType_JP) {
		Key_ActSheetSize_DoublePostcardSEF.showEnable(True);
	} else {
		Key_ActSheetSize_DoublePostcardSEF.showEnable(False);
	}
#endif // DEF_A4_Cus	// A3A4FUM OPS2 Sky後藤(拓) 2015/01/19.

	switch ( pc_MachineSetting->getFLSType() ) {
		case TYPE_FLSType85x135:	// 8_1/2inch x 13_1/2inch.
			Key_ActSheetSize_8x13SEF.changeText( s_pNOTPaperSizeFoolscap_001_W96H028A );
			break;
		case TYPE_FLSType220x330:	// 220mm x 330mm.
			Key_ActSheetSize_8x13SEF.changeText( s_pNOTPaperSizeFoolscap_002_W96H028A );
			break;
		case TYPE_FLSType85x13:		// 8_1/2inch x 13inch.
			Key_ActSheetSize_8x13SEF.changeText( s_pNOTPaperSizeFoolscap_003_W96H028A );
			break;
		case TYPE_FLSType825x13:	// 8_1/4inch x 13inch.
			Key_ActSheetSize_8x13SEF.changeText( s_pNOTPaperSizeFoolscap_004_W96H028A );
			break;
		case TYPE_FLSType8125x1325:	// 8_1/8inch x 13_1/4inch.:.
			Key_ActSheetSize_8x13SEF.changeText( s_pNOTPaperSizeFoolscap_005_W96H028A );
			break;
		case TYPE_FLSType8x13:		// 8inch x 13inch.
			Key_ActSheetSize_8x13SEF.changeText( s_pNOTPaperSizeFoolscap_006_W96H028A );
			break;
		default:
			// なにもしない.
			break;
	};

#ifdef DEF_A4_Cus	// VenusMLK OPS2 Sky高田(庸) 2015/07/10.
	if ((PNLE_APPMODE_SCANFAX == PnlComGetAppMode()) && (False == combinationCheck(CNK_UnfixSize))) {
		return;
	}
	if ((PNLE_APPMODE_COPY == PnlComGetAppMode()) && (False == combinationCheck(CNK_OriginalManual))) {
		return;
	}
#endif // DEF_A4_Cus	// VenusMLK OPS2 Sky高田(庸) 2015/07/10.

	switch( option ) {		// オプションチェック.
		default:
			m_PnlTpl_ReadSize.cmn_procComButtonOnMetricSize( option );		// 原稿サイズのボタン（押された)処理.
			m_PnlTpl_ReadSize.cmn_procComButtonOnInchSize( option );
			m_PnlTpl_ReadSize.cmn_procComButtonOnOthers( option );
			m_PnlTpl_ReadSize.cmn_procComButtonOnPhotoSize( option );
			break;
	}
	setupGamen( 0 );	// 伊丹Sky(坂梨)  2011/08/17 CQ352807.
}

//関数------------------------------------------------------------------------------------------------------------------
//概要    ：画面終了処理.
//備考    ：.
//戻値    ：.
//作成者  ：AWS_ウィング.
//レビュー：.
//変更履歴：.
//------------------------------------------------------------------------------
/**
* @brief A070800110 原稿サイズ : 画面終了処理
*/
//------------------------------------------------------------------------------
void PCPC_SetChangeOriginalSizeMetric_act::endGamen()
{
	PPopupDecisionButtonCom::endGamen();
}

//関数------------------------------------------------------------------------------------------------------------------
//概要    ：サイズタイプ.
//備考    ：.
//戻値    ：.
//作成者  ：AWS_ウィング.
//レビュー：.
//変更履歴：.
//------------------------------------------------------------------------------
/**
* @brief A070800110 原稿サイズ : サイズタイプ
*/
//------------------------------------------------------------------------------
void PCPC_SetChangeOriginalSizeMetric_act::sizeTypeSetup()
{
	TYPC_Scan* pc_Scan = onlyPanel.cc_DisplayJob->getJobSpec()->pc_Scan;
	TYPC_PaperSize c_PaperSize = pc_Scan->c_OriginalSize.getPaperSize();
#ifndef DEF_A4_Cus	// VenusMLK OPS2 Sky高田(庸) 2015/07/10.
	if( False == pc_Scan->c_OriginalSize.getAutoSize() ) {	// 自動検出モードの判定.
#endif // DEF_A4_Cus	// VenusMLK OPS2 Sky高田(庸) 2015/07/10.
		m_PnlTpl_ReadSize.cmn_sizeTypeSetup( c_PaperSize );
#ifndef DEF_A4_Cus	// VenusMLK OPS2 Sky高田(庸) 2015/07/10.
	}
#endif // DEF_A4_Cus	// VenusMLK OPS2 Sky高田(庸) 2015/07/10.
}

//関数------------------------------------------------------------------------------------------------------------------
//概要    ：サイズタイプ写真MJ..
//備考    ：.
//戻値    ：.
//作成者  ：AWS_ウィング.
//レビュー：.
//変更履歴：.
//------------------------------------------------------------------------------
/**
* @brief A070800110 原稿サイズ : サイズタイプ写真MJ.
*/
//------------------------------------------------------------------------------
void PCPC_SetChangeOriginalSizeMetric_act::sizeTypePhotoSelectMJ()
{
	TYPC_Scan* pc_Scan = onlyPanel.cc_DisplayJob->getJobSpec()->pc_Scan;
#ifndef DEF_A4_Cus	// VenusMLK OPS2 Sky高田(庸) 2015/07/10.
	if( False == pc_Scan->c_OriginalSize.getAutoSize() ) {
#endif // DEF_A4_Cus	// VenusMLK OPS2 Sky高田(庸) 2015/07/10.
		m_PnlTpl_ReadSize.cmn_sizeTypePhotoSelectMJ( pc_Scan );
#ifndef DEF_A4_Cus	// VenusMLK OPS2 Sky高田(庸) 2015/07/10.
	}
#endif // DEF_A4_Cus	// VenusMLK OPS2 Sky高田(庸) 2015/07/10.
}

//関数------------------------------------------------------------------------------------------------------------------
//概要    ：サイズタイプ写真MC..
//備考    ：.
//戻値    ：.
//作成者  ：AWS_ウィング.
//レビュー：.
//変更履歴：.
//------------------------------------------------------------------------------
/**
* @brief A070800110 原稿サイズ : サイズタイプ写真MC.
*/
//------------------------------------------------------------------------------
void PCPC_SetChangeOriginalSizeMetric_act::sizeTypePhotoSelectMC()
{
	TYPC_Scan* pc_Scan = onlyPanel.cc_DisplayJob->getJobSpec()->pc_Scan;
#ifndef DEF_A4_Cus	// VenusMLK OPS2 Sky高田(庸) 2015/07/10.
	if( False == pc_Scan->c_OriginalSize.getAutoSize() ) {
#endif // DEF_A4_Cus	// VenusMLK OPS2 Sky高田(庸) 2015/07/10.
		m_PnlTpl_ReadSize.cmn_sizeTypePhotoSelectMC( pc_Scan );
#ifndef DEF_A4_Cus	// VenusMLK OPS2 Sky高田(庸) 2015/07/10.
	}
#endif // DEF_A4_Cus	// VenusMLK OPS2 Sky高田(庸) 2015/07/10.
}

//関数------------------------------------------------------------------------------------------------------------------
//概要    ：サイズタイプ写真ME..
//備考    ：.
//戻値    ：.
//作成者  ：AWS_ウィング.
//レビュー：.
//変更履歴：.
//------------------------------------------------------------------------------
/**
* @brief A070800110 原稿サイズ : サイズタイプ写真ME.
*/
//------------------------------------------------------------------------------
void PCPC_SetChangeOriginalSizeMetric_act::sizeTypePhotoSelectME()
{
	TYPC_Scan* pc_Scan = onlyPanel.cc_DisplayJob->getJobSpec()->pc_Scan;
#ifndef DEF_A4_Cus	// VenusMLK OPS2 Sky高田(庸) 2015/07/10.
	if( False == pc_Scan->c_OriginalSize.getAutoSize() ) {
#endif // DEF_A4_Cus	// VenusMLK OPS2 Sky高田(庸) 2015/07/10.
		m_PnlTpl_ReadSize.cmn_sizeTypePhotoSelectME( pc_Scan );
#ifndef DEF_A4_Cus	// VenusMLK OPS2 Sky高田(庸) 2015/07/10.
	}
#endif // DEF_A4_Cus	// VenusMLK OPS2 Sky高田(庸) 2015/07/10.
}

// comment 2014/01/15 OPS1-Sky浜野 [Poseidon][CQ508992].
//------------------------------------------------------------------------------
/**
* @brief A070800110 原稿サイズ : コールバック.
*/
//------------------------------------------------------------------------------
void PCPC_SetChangeOriginalSizeMetric_act::procCallBack( TYPE_PNL_CallBack option )
{
// comment 2014/02/03 OPS1-Sky浜野 [Poseidon][CQ509465].
	PPopupDecisionButtonCom::procCallBack( option );

	if ( ( NULL == onlyPanel.cc_DisplayJob )
	||	 ( NULL == onlyPanel.cc_DisplayJob->getJobSpec() ) ) {
		return;
	}
	
	TYPC_Scan* pc_Scan = onlyPanel.cc_DisplayJob->getJobSpec()->pc_Scan;
	if ( NULL == pc_Scan ) { return; }
	
	
	switch ( option ) {
		case TYPE_PNL_CallBack_PopupCloseExec:
			pc_Scan->c_OriginalSize.setAutoSize( True );		// 全面画像・OFF.
			Cmp_ActTabOriginalSize.selectTab( PNLD_TabAutoSize );	// 自動タブを選択  伊丹Sky(坂梨)  2011/08/17 CQ352807.
			// 本画面を閉じ.
			PNLC_PopupListControl::newInstance()->closeTop( True );  
			break;
		default:	break;
	}
}
// ===============================================================================================.
//	概要	: サイズ設定.
//  引数    : 原稿サイズ.
//	戻り値	: なし.
// ===============================================================================================.//------------------------------------------------------------------------------.
void PCPC_SetChangeOriginalSizeMetric_act::setSize( TYPC_PaperSize c_PaperSize )
{
	TYPC_Scan* pc_Scan = onlyPanel.cc_DisplayJob->getJobSpec()->pc_Scan;
	pc_Scan->c_OriginalSize.setPaperSize( c_PaperSize );	 // 原稿サイズ設定.
	onlyPanel.setAutoSizeDetect( onlyPanel.cc_DisplayJob, False );	// 原稿自動検出・OFF.
#ifdef DEF_A4_Cus	// VenusMLK OPS2 Sky高田(庸) 2015/07/10.
	PnlComSetOriginalSizeByCurrentOriginalMode( onlyPanel.cc_DisplayJob );
#endif // DEF_A4_Cus	// VenusMLK OPS2 Sky高田(庸) 2015/07/10.
	// ポップアップ管理クラスのインスタンス取得.
	PNLC_PopupListControl* pc_PopupListControl = PNLC_PopupListControl::newInstance();
	if( NULL != pc_PopupListControl ) {
		pc_PopupListControl->closeTop( True );		//本画面を閉じ.
// comment 2014/01/15 OPS1-Sky浜野 [Poseidon][CQ508992].
		m_bl_Closed = True;
	}
}

void PCPC_SampleCopySetChangeMenu_act::startGamen()
{
	PNLC_OutSideKeyCallDataLib::newInstance()->setSelectNo( PNLE_Soft10KeyUnselected );			// データクリア.

	PPopupDecisionButtonCom::startGamen();		// 上位層の画面セットアップを呼ぶ.
	Key_SampleCopySetChangeMenu_actNumberOfCopiesInput.activate( True );
	TYPC_Print* pc_Print = comPanelJob->getJobSpec()->pc_Print;
	Key_SampleCopySetChangeMenu_actNumberOfCopiesInput.set( &Num_SampleCopySetChangeMenu_actNumberOfCopiesInput, static_cast<float>( pc_Print->getNumOfCopies() ), True );			// コピー部数の初期表示値設定.
	uchar uc_MaxPage = PNLD_SampleCopySetChangeMenu_MaxAppSetting / PNLD_SampleCopySetChangeMenu_MaxAppOnePage;
	if( 0 != ( PNLD_SampleCopySetChangeMenu_MaxAppSetting % PNLD_SampleCopySetChangeMenu_MaxAppOnePage ) ) { uc_MaxPage++; }
	Cmp_SampleCopySetChangeMenu_actScrollBar.setScrollMaxPage( uc_MaxPage );

	Cmp_SampleCopySetChangeMenu_actPopupMessageArea.setMode( PNLE_PopupMessageAreaMessageModeAdmin );
	Cmp_SampleCopySetChangeMenu_actPopupMessageArea.showDestinationCount( False );
	Cmp_SampleCopySetChangeMenu_actPopupMessageArea.setOnOffButton( False );
	Cmp_SampleCopySetChangeMenu_actPopupMessageArea.setNowPopupGamen( PNLE_SampleCopySetChangeMenu_act );

	short s_PosX = PNLD_ZeroInit, s_PosY = PNLD_ZeroInit, s_SizeW = PNLD_ZeroInit, s_SizeH = PNLD_ZeroInit;
	//!< 部数ボタン表示位置取得.
	Key_SampleCopySetChangeMenu_actNumberOfCopiesInput.getPosition( s_PosX, s_PosY );
	//!< メッセージエリアサイズ取得.
	Cmp_SampleCopySetChangeMenu_actPopupMessageArea.getSize( s_SizeW, s_SizeH );
	//!< メッセージエリアの拡大エリアサイズを部数表示位置までに変更（）.
	Cmp_SampleCopySetChangeMenu_actPopupMessageArea.setScalingAreaSize( static_cast<short>( s_PosX - Cmp_SampleCopySetChangeMenu_actPopupMessageArea.getPositionX() ), s_SizeH );

	PCGC_JobSpecCopyBackup* pc_JobSpecCopyBackup = PnlComGetJobSpecCopyBack(); // コピージョブ用バックアップ領域を取得.
	if( NULL == pc_JobSpecCopyBackup )
	{
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return;
	}
	pc_JobSpecCopyBackup->b_SampleStopMenuCopyDirectInput = True; // 直接入力フラグを初期化する.

	// 入力エリア押下機能有無.
	if( True != PLBC_FuncVersionCheck::isOK_Type4() ) {
		Key_SampleCopySetChangeMenu_actNumberOfCopiesInput.setShowPush( False );
	}
	Key_SampleCopySetChangeMenu_actNumberOfCopiesInput.changeShadow( PNLE_Shadow_none );
}
//関数------------------------------------------------------------------------------------------------------------------
//概要    ：表示部品のセットアップ..
//備考    ：.
//戻値    ：.
//作成者  ：AWS_ウィング
//レビュー：.
//変更履歴：.
//------------------------------------------------------------------------------
/**
* @brief A070800400 確認コピー停止中_設定変更_メニュー : 表示部品のセットアップ.
*/
//------------------------------------------------------------------------------
void PCPC_SampleCopySetChangeMenu_act::setupGamen( short option )
{
// Comment 2019/07/16 【IT6_EagleH】【HCL_Prabakaran】OP_BTS-8347 jobSpec Type has been changed for common use.	
//	APIC_Job* pc_JobCopy = PnlComGetJobCopy();
	APIC_Job* pc_JobCopy = comPanelJob;
	if( NULL == pc_JobCopy ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return;}	//【 静的解析対応 : 酒井(健) : 2011.03.03 : CID-288966 】.
	PPopupDecisionButtonCom::setupGamen( option );		// 上位層の画面セットアップを呼ぶ.

	TYPC_JobStatus* pc_JobStatus = comPanelJob->getJobStatus();		// コピー部数の表示.
	TYPC_Print* pc_CopyNum = comPanelJob->getJobSpec()->pc_Print;
	if( False == comPanelJob->getJobSpec()->pc_Common->c_JobType.getSingleStepOn() ) {											// 確認コピーでない場合は、部数表示を-1しない.
		Key_SampleCopySetChangeMenu_actNumberOfCopiesInput.set( &Num_SampleCopySetChangeMenu_actNumberOfCopiesInput, static_cast<float>( pc_CopyNum->getNumOfCopies() ), True );
	} else {
		Key_SampleCopySetChangeMenu_actNumberOfCopiesInput.set( &Num_SampleCopySetChangeMenu_actNumberOfCopiesInput, static_cast<float>( pc_CopyNum->getNumOfCopies() - PCGD_DefaultCopyNumber ), True );		// // コピー部数の出力状況 1部プリント済み.
	}
	PCGC_JobSpecCopyBackup* pc_JobSpecCopyBackup = PnlComGetJobSpecCopyBack();
	if( NULL == pc_JobSpecCopyBackup ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }
	Txt_SampleCopySetChangeMenu_actNumberOfCopiesInput.showEnableShow( True );


	if( comPanelJob->getJobSpec()->pc_Common->c_JobType.getSingleStepOn() ) {
		// 確認コピー.
		Cmp_SampleCopySetChangeMenu_actPopupMessageArea.changeMainMessageText( s_pMSGSetChangeSampleCopyJob_W448H033A );	// 2010/07/26 シンボル変更対応 伊丹Sky水嶋.
	} else {
		// 通常コピー.
		Cmp_SampleCopySetChangeMenu_actPopupMessageArea.changeMainMessageText( s_pMSGMemoryCopyChangeSetting_W448H033A );
	}

	Key_SampleCopySetChangeMenu_actFinishing.showEnable( True );		// 「仕上り」キーの表示/非表示：パターンカスタマイズの[仕上り]機能の設定による。.

	int jobID_value=( int )( ( pc_JobStatus->getJobID() ) );
	char jobNumText[5] = {'\0'};
	snprintf( ( char* )jobNumText, sizeof( jobNumText ), "%04d", jobID_value );
	Btx_SampleCopySetChangeMenu_actJobNoField.changeExternalText( ( uchar* )jobNumText, ( short )strlen( jobNumText ), getCharSetForPanel() );

	TYPC_PaperSize ct_PaperSize = pc_JobCopy->getJobSpec()->pc_Print->c_PaperSize;		// 警告チェックする用紙サイズ 【 静的解析対応 : 酒井(健) : 2011.03.03 : CID-288966 】.
	if( APIC_OutputTray::newInstance()->isDuplexEnableSize( ct_PaperSize ) == False ) {		// 片面/両面.
		Key_SampleCopySetChangeMenu_actPrint.showEnable( False );		// 印刷キーは非表示.
		Key_SampleCopySetChangeMenu_actPrint_OneSide.showEnable( False );
		Key_SampleCopySetChangeMenu_actPrint_TwoSide.showEnable( False );
	} else {
		uchar u_selectItem = PNLD_PullDownProgramJobSetChangeMenu_actPrintOneSide + 1;
// Comment 2019/07/16 【IT6_EagleH】【HCL_Prabakaran】OP_BTS-8347 jobSpec Type has been changed for common use.
//		if( False == isOn( CNK_1SideCopy, PnlComGetJobCopy() ) ) {
		if( False == isOn( CNK_1SideCopy, comPanelJob ) ) {
			u_selectItem = PNLD_PullDownProgramJobSetChangeMenu_actPrintTwoSide + 1;
		}
		Key_SampleCopySetChangeMenu_actPrint.setSelectNo( u_selectItem );
	}

	gp_SampleCopySetChangeMenu_actPullDown.setupSelectedPrintType();

	// スクロールバー表示.
	// 応用機能オプション「文字スタンプ」が有効時に表示.
	if( PNLE_AppOptionStatus_Enable != PnlComGetEnableAppOptionStatus( TYPE_AppOptionStamp ) ) {
		Cmp_SampleCopySetChangeMenu_actScrollBar.showEnable( False );
	}

	uchar uc_Page = Cmp_SampleCopySetChangeMenu_actScrollBar.getSliderPage();
	if( 1 == uc_Page ) {
		Grp_SampleCopySetChangeMenu_actApplicationSetupPage2.showEnable( False );
		Grp_SampleCopySetChangeMenu_actApplicationSetupPage1.showEnable( True );
		setupApplicationSettingPage1();
	}
	else if( 2 == uc_Page ) {
		Grp_SampleCopySetChangeMenu_actApplicationSetupPage1.showEnable( False );
		Grp_SampleCopySetChangeMenu_actApplicationSetupPage2.showEnable( True );
		setupApplicationSettingPage2();
	}

	// 差込みページON-OFF設定取得.
// Comment 2019/07/16 【IT6_EagleH】【HCL_Prabakaran】OP_BTS-8347 jobSpec Type has been changed for common use.
//	Bool b_isOriginalInsertion = isOn( CNK_OriginalInsertion, PnlComGetJobCopy() );
	Bool b_isOriginalInsertion = isOn( CNK_OriginalInsertion, comPanelJob );
	// 差込みページON設定の場合、カバーシート、インターシート、章分け の各遷移キーは非表示.
	Key_SampleCopySetChangeMenu_actCopyProgramImageInsert.showEnable( b_isOriginalInsertion );		// 差込みページ 表示/非表示.
	Key_SampleCopySetChangeMenu_actCoverSheet.showEnable( !b_isOriginalInsertion );					// カバーシート 表示/非表示.
	Key_SampleCopySetChangeMenu_actSheetCoverInsertion.showEnable( !b_isOriginalInsertion );		// インターシート 表示/非表示.
	Key_SampleCopySetChangeMenu_actChapterPaper.showEnable( !b_isOriginalInsertion );				// 章分け 表示/非表示.

	// パターンカスタマイズ.
	showPatternCustomize();

	sendSlide10KeyData();

// comment 2020/07/14 SCD2-Sky 石原 [IT6_2.0MR2][OP_BTS-25362] フッターエリア対応.
#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus)  && defined (DEF_LCD_7INCH) ) )
	PnlComFooterPartsBtnActiveChange(homeKeyActiveCheck(), idKeyActiveCheck(), resetKeyActiveCheck());
#endif		//  ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus)  && defined (DEF_LCD_7INCH) ) )	.

}
//関数------------------------------------------------------------------------------------------------------------------
//概要    ：ボタン押下.
//備考    ：.
//戻値    ：.
//作成者  ：.
//レビュー：.
//変更履歴：.
//------------------------------------------------------------------------------
/**
* @brief A070800400 確認コピー停止中_設定変更_メニュー : 表示更新処理（応用設定1ページ目）
*/
//------------------------------------------------------------------------------
void PCPC_SampleCopySetChangeMenu_act::setupApplicationSettingPage1()
{
// Comment 2019/07/16 【IT6_EagleH】【HCL_Prabakaran】OP_BTS-8347 jobSpec Type has been changed for common use.
//	APIC_Job* pc_JobCopy = PnlComGetJobCopy();
	APIC_Job* pc_JobCopy = comPanelJob;
	if( NULL == pc_JobCopy ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return;}	//【 静的解析対応 : 酒井(健) : 2011.03.03 : CID-288966 】.
	TYPC_JobSpec* pc_BackupSpec = c_SettingChangeSampleCopy.getSampleCopySpecInfo();
// Comment 2019/07/16 【IT6_EagleH】【HCL_Prabakaran】OP_BTS-8347 jobSpec Type has been changed for common use.
//	const Bool bl_Coversheet = isOn( CNK_CoverMode, PnlComGetJobCopy() );
//	const Bool bl_Intersheet = isOn( CNK_MiddleMode, PnlComGetJobCopy() );
//	const Bool bl_Chapter = isOn( CNK_Chapter, PnlComGetJobCopy() );
//	const Bool bl_DateStamp = isOn( CNK_StampDate, PnlComGetJobCopy() );
	TYPC_Print* pc_Print = pc_JobCopy->getJobSpec()->pc_Print;
	const Bool bl_Coversheet = isOn( CNK_CoverMode, comPanelJob );
	const Bool bl_Intersheet = isOn( CNK_MiddleMode, comPanelJob );
	const Bool bl_Chapter = isOn( CNK_Chapter, comPanelJob );
	const Bool bl_DateStamp = isOn( CNK_StampDate, comPanelJob );

	// 小冊子.
	// 小冊子非設定時（※小冊子はOFFで画像を1度読み込んだ後は、ON不可能).
	if( False == isOn( CNK_Magazine, pc_BackupSpec ) ) {
		Key_SampleCopySetChangeMenu_actBooklet.showEnable( False );
	}
	else {
		Key_SampleCopySetChangeMenu_actBooklet.showEnable( True );
	}
	if( pc_Print->c_BindDirection.getBindDirection() == TYPE_BindDirectionLeft )		// 「小冊子」の設定は「左開き」場合.
	{
#ifdef DEF_LCD_7INCH
		Key_SampleCopySetChangeMenu_actBooklet.changeTextLine2( s_pKEYLeftBind_W152H018A );		//  「小冊子」キーに「左開き」を表示する.
#else // #ifdef DEF_LCD_7INCH.
		Key_SampleCopySetChangeMenu_actBooklet.changeTextLine2( s_pKEYLeftBind_W144H016A );		//  「小冊子」キーに「左開き」を表示する.
#endif // #ifdef DEF_LCD_7INCH.
		// 「小冊子」の設定は「右開き」場合.
	} else if( pc_Print->c_BindDirection.getBindDirection() == TYPE_BindDirectionRight ) {
		// 「小冊子」キーに「右開き」を表示する.
		Key_SampleCopySetChangeMenu_actBooklet.changeTextLine2( s_pKEYRightBind_W144H028A );

	}

	// とじしろ.
	Key_SampleCopySetChangeMenu_actPageMargin.showEnable( True );		// 「とじしろ」キーの表示/非表示：パターンカスタマイズの[とじしろ]機能の設定による。.
	if( pc_Print->c_BindDirection.getBindDirection() != TYPE_BindDirectionAuto
	    || pc_Print->c_BindDirection.getBindWidth() != 0.0f ) {
// Comment 2019/07/16 【IT6_EagleH】【HCL_Prabakaran】OP_BTS-8347 jobSpec Type has been changed for common use.
//		switch( pc_JobCopy->getJobSpec()->pc_Print->c_BindDirection.getBindDirection() ) {	//【 静的解析対応 : 酒井(健) : 2011.03.03 : CID-288966 】.
		switch( comPanelJob->getJobSpec()->pc_Print->c_BindDirection.getBindDirection() ) {	//【 静的解析対応 : 酒井(健) : 2011.03.03 : CID-288966 】.
#ifdef DEF_LCD_7INCH
			case TYPE_BindDirectionAuto:	// 自動.
				Key_SampleCopySetChangeMenu_actPageMargin.changeTextLine2( s_pITMAuto_W152H018A );
				break;
			case TYPE_BindDirectionLeft:	// 左開き/とじ.
				Key_SampleCopySetChangeMenu_actPageMargin.changeTextLine2( s_pITMLeftOpenBind_W152H018A );
				break;
			case TYPE_BindDirectionRight:	// 右開き/とじ.
				Key_SampleCopySetChangeMenu_actPageMargin.changeTextLine2( s_pKEYRightOpenBind_W152H018A );
				break;
			case TYPE_BindDirectionTop:		// 上開き/とじ.
				Key_SampleCopySetChangeMenu_actPageMargin.changeTextLine2( s_pKEYTopOpenBind_W152H018A );
				break;
#else // #ifdef DEF_LCD_7INCH.
			case TYPE_BindDirectionAuto:	// 自動.
				Key_SampleCopySetChangeMenu_actPageMargin.changeTextLine2( s_pITMAuto_W144H016A );
				break;
			case TYPE_BindDirectionLeft:	// 左開き/とじ.
				Key_SampleCopySetChangeMenu_actPageMargin.changeTextLine2( s_pITMLeftOpenBind_W144H016A );
				break;
			case TYPE_BindDirectionRight:	// 右開き/とじ.
				Key_SampleCopySetChangeMenu_actPageMargin.changeTextLine2( s_pKEYRightOpenBind_W112H036A );
				break;
			case TYPE_BindDirectionTop:		// 上開き/とじ.
				Key_SampleCopySetChangeMenu_actPageMargin.changeTextLine2( s_pKEYTopOpenBind_W112H036A );
				break;
#endif // #ifdef DEF_LCD_7INCH.
			default:
				break;
		}
	} else {
		//とじしろOFF
#ifdef DEF_LCD_7INCH
		Key_SampleCopySetChangeMenu_actPageMargin.changeText( s_pITMOFF_W152H018A, 2 );
#else // #ifdef DEF_LCD_7INCH.
		Key_SampleCopySetChangeMenu_actPageMargin.changeText( s_pITMOFF_W144H016A, 2 );
#endif // #ifdef DEF_LCD_7INCH.
	}

	// 画像シフト.
	TYPC_Point<float> c_Point = pc_Print->c_ImageOffset.getFrontOffset();		// 表面の値取得.
	TYPC_Point<float> c_PointRear = pc_Print->c_ImageOffset.getRearOffset();		// 裏面の値取得.
	if( ( PNLD_IMAGE_SHIFT_OFF == c_Point.x ) && ( PNLD_IMAGE_SHIFT_OFF == c_Point.y ) 		// 「OFF」の場合.
	    && ( PNLD_IMAGE_SHIFT_OFF == c_PointRear.x ) && ( PNLD_IMAGE_SHIFT_OFF == c_PointRear.y ) ) {
#ifdef DEF_LCD_7INCH
		Key_SampleCopySetChangeMenu_actImageShift.changeTextLine2( s_pITMOFF_W152H018A );
#else // #ifdef DEF_LCD_7INCH.
		Key_SampleCopySetChangeMenu_actImageShift.changeTextLine2( s_pITMOFF_W144H016A );
#endif // #ifdef DEF_LCD_7INCH.
	} else {
#ifdef DEF_LCD_7INCH
		Key_SampleCopySetChangeMenu_actImageShift.changeTextLine2( s_pITMOn_W152H018A );
#else // #ifdef DEF_LCD_7INCH.
		Key_SampleCopySetChangeMenu_actImageShift.changeTextLine2( s_pITMOn_W144H016A );
#endif // #ifdef DEF_LCD_7INCH.
	}
	// カバーシート.
#ifdef DEF_LCD_7INCH
	Key_SampleCopySetChangeMenu_actCoverSheet.changeTextLine2( bl_Coversheet ? s_pITMOn_W152H018A : s_pITMOFF_W152H018A );
#else // #ifdef DEF_LCD_7INCH.
	Key_SampleCopySetChangeMenu_actCoverSheet.changeTextLine2( bl_Coversheet ? s_pITMOn_W144H016A : s_pITMOFF_W144H016A );
#endif // #ifdef DEF_LCD_7INCH.
	// 章分け.
#ifdef DEF_LCD_7INCH
	Key_SampleCopySetChangeMenu_actChapterPaper.changeTextLine2( bl_Chapter ? s_pITMOn_W152H018A : s_pITMOFF_W152H018A );
#else // #ifdef DEF_LCD_7INCH.
	Key_SampleCopySetChangeMenu_actChapterPaper.changeTextLine2( bl_Chapter ? s_pITMOn_W144H016A : s_pITMOFF_W144H016A );
#endif // #ifdef DEF_LCD_7INCH.
	// インターシート.
#ifdef DEF_LCD_7INCH
	Key_SampleCopySetChangeMenu_actSheetCoverInsertion.changeTextLine2( bl_Intersheet ? s_pITMOn_W152H018A : s_pITMOFF_W152H018A );
#else // #ifdef DEF_LCD_7INCH.
	Key_SampleCopySetChangeMenu_actSheetCoverInsertion.changeTextLine2( bl_Intersheet ? s_pITMOn_W144H016A : s_pITMOFF_W144H016A );
#endif // #ifdef DEF_LCD_7INCH.
	// 章分け.
	if( PnlComGetPrinterSystem()->isExistDuplex() ) {					// 両面給紙ユニット装着.
		Key_SampleCopySetChangeMenu_actChapterPaper.showEnable( True );
	} else {
		Key_SampleCopySetChangeMenu_actChapterPaper.showEnable( False );
	}

	// 応用機能オプション「文字スタンプ」の有効/無効.
	Bool bl_AppOptionStamp = ( PNLE_AppOptionStatus_Enable == PnlComGetEnableAppOptionStatus( TYPE_AppOptionStamp ) ) ? True : False;

	// 日付/時刻.
	// 応用機能オプション「文字スタンプ」が有効でない時は非表示.
	if( False == bl_AppOptionStamp ) {
		Key_SampleCopySetChangeMenu_actDateTime.showEnable( False );
	}
#ifdef DEF_LCD_7INCH
	Key_SampleCopySetChangeMenu_actDateTime.changeTextLine2( bl_DateStamp ? s_pITMOn_W152H018A : s_pITMOFF_W152H018A );
#else // #ifdef DEF_LCD_7INCH.
	Key_SampleCopySetChangeMenu_actDateTime.changeTextLine2( bl_DateStamp ? s_pITMOn_W144H016A : s_pITMOFF_W144H016A );
#endif // #ifdef DEF_LCD_7INCH.

	// ページ番号.
	// 応用機能オプション「文字スタンプ」が有効でない時は非表示.
	if( False == bl_AppOptionStamp ) {
		Key_SampleCopySetChangeMenu_actPageNumber.showEnable( False );
	}
#ifdef DEF_LCD_7INCH
	;	// 7インチは2ページ目で表示制御を行う.
#else //  DEF_LCD_7INCH.
	setupApplicationSettingPageNumber();
#endif  //  DEF_LCD_7INCH.

	// スタンプ.
#ifdef DEF_LCD_7INCH
	;	// 7インチは2ページ目で表示制御を行う.
#else //  DEF_LCD_7INCH.
	setupApplicationSettingStamp();
#endif  //  DEF_LCD_7INCH.

}
//関数------------------------------------------------------------------------------------------------------------------
//概要    ：ボタン押下.
//備考    ：.
//戻値    ：.
//作成者  ：.
//レビュー：.
//変更履歴：.
//------------------------------------------------------------------------------
/**
* @brief A070800400 確認コピー停止中_設定変更_メニュー : 表示更新処理（応用設定2ページ目）
*/
//------------------------------------------------------------------------------
void PCPC_SampleCopySetChangeMenu_act::setupApplicationSettingPage2()
{
	Bool bl_KeyCounterIf = False;
	if( TYPE_KeyModeVendor == APIC_KeyDevice::getKeyMode()
	    && True == PnlComGetMachineSetting()->getKeyCounterIfVendor() ) {
		bl_KeyCounterIf = True;
	}

	// 応用機能オプション「文字スタンプ」の有効/無効.
	Bool bl_AppOptionStamp = ( PNLE_AppOptionStatus_Enable == PnlComGetEnableAppOptionStatus( TYPE_AppOptionStamp ) ) ? True : False;

	// ページ番号.
#ifdef DEF_LCD_7INCH
	setupApplicationSettingPageNumber();
#else //  DEF_LCD_7INCH.
	;	// 9インチは1ページ目で表示制御を行う.
#endif  //  DEF_LCD_7INCH.

	// スタンプ.
#ifdef DEF_LCD_7INCH
	setupApplicationSettingStamp();
#else //  DEF_LCD_7INCH.
	;	// 9インチは1ページ目で表示制御を行う.
#endif  //  DEF_LCD_7INCH.

	// 繰り返しスタンプ.
	if( ( True == bl_KeyCounterIf )
	    || ( False == bl_AppOptionStamp )
	  ) {
		Key_SampleCopySetChangeMenu_actStampRepeat.showEnable( False );
	}
// Comment 2019/07/16 【IT6_EagleH】【HCL_Prabakaran】OP_BTS-8347 jobSpec Type has been changed for common use.
//#ifdef DEF_LCD_7INCH
//	Key_SampleCopySetChangeMenu_actStampRepeat.changeTextLine2( isOn( CNK_Watermark, PnlComGetJobCopy() ) ? s_pITMOn_W152H018A : s_pITMOFF_W152H018A );
//#else // #ifdef DEF_LCD_7INCH.
//	Key_SampleCopySetChangeMenu_actStampRepeat.changeTextLine2( isOn( CNK_Watermark, PnlComGetJobCopy() ) ? s_pITMOn_W144H016A : s_pITMOFF_W144H016A );
//#endif // #ifdef DEF_LCD_7INCH.
#ifdef DEF_LCD_7INCH
	Key_SampleCopySetChangeMenu_actStampRepeat.changeTextLine2( isOn( CNK_Watermark, comPanelJob ) ? s_pITMOn_W152H018A : s_pITMOFF_W152H018A );
#else // #ifdef DEF_LCD_7INCH.
	Key_SampleCopySetChangeMenu_actStampRepeat.changeTextLine2( isOn( CNK_Watermark, comPanelJob ) ? s_pITMOn_W144H016A : s_pITMOFF_W144H016A );
#endif // #ifdef DEF_LCD_7INCH.
	// コピープロテクト.
	if( ( True == bl_KeyCounterIf )
	    || ( False == bl_AppOptionStamp )
	  ) {
		Key_SampleCopySetChangeMenu_actCopyProtect.showEnable( False );
	}
// Comment 2019/07/16 【IT6_EagleH】【HCL_Prabakaran】OP_BTS-8347 jobSpec Type has been changed for common use.
//#ifdef DEF_LCD_7INCH
//	Key_SampleCopySetChangeMenu_actCopyProtect.changeTextLine2( isOn( CNK_CopyProtect, PnlComGetJobCopy() ) ? s_pITMOn_W152H018A : s_pITMOFF_W152H018A );
//#else // #ifdef DEF_LCD_7INCH.
//	Key_SampleCopySetChangeMenu_actCopyProtect.changeTextLine2( isOn( CNK_CopyProtect, PnlComGetJobCopy() ) ? s_pITMOn_W144H016A : s_pITMOFF_W144H016A );
//#endif // #ifdef DEF_LCD_7INCH.
	// ヘッダ/フッタ.
#ifdef DEF_LCD_7INCH
	Key_SampleCopySetChangeMenu_actCopyProtect.changeTextLine2( isOn( CNK_CopyProtect, comPanelJob ) ? s_pITMOn_W152H018A : s_pITMOFF_W152H018A );
#else // #ifdef DEF_LCD_7INCH.
	Key_SampleCopySetChangeMenu_actCopyProtect.changeTextLine2( isOn( CNK_CopyProtect, comPanelJob ) ? s_pITMOn_W144H016A : s_pITMOFF_W144H016A );
#endif // #ifdef DEF_LCD_7INCH.
	// ヘッダ/フッタ.
	Bool bl_HeaderFooterSet = True;
	short ss_EntryCount = 0;
	TYPC_HeaderFooterStamp rc_HeaderFooterStamp;
	for( uchar i = 1; PCGD_HeaderFooter >= i; ++i ) {
		PnlComGetOperationSetting()->getHeaderFooterStamp( i, rc_HeaderFooterStamp );						//!<条件分岐：ヘッダー/フッター登録状態を取得.
		if( ( True == rc_HeaderFooterStamp.getHeaderOn() ) || ( True == rc_HeaderFooterStamp.getFooterOn() ) ) {
			ss_EntryCount++;
			break;
		}
	}
	if( ( True == bl_KeyCounterIf )
	    || ( 0 >= ss_EntryCount )
	    || ( False == bl_AppOptionStamp ) 	// 応用機能オプション「文字スタンプ」が有効でない.
	  ) {
		bl_HeaderFooterSet = False;
	}
	Key_SampleCopySetChangeMenu_actHeaderFooter.showEnable( bl_HeaderFooterSet );									//!<ヘッダー/フッターキー表示の判定.
// Comment 2019/07/16 【IT6_EagleH】【HCL_Prabakaran】OP_BTS-8347 jobSpec Type has been changed for common use.
//#ifdef DEF_LCD_7INCH
//	Key_SampleCopySetChangeMenu_actHeaderFooter.changeTextLine2( isOn( CNK_HeaderFooterStamp, PnlComGetJobCopy() ) ? s_pITMOn_W152H018A : s_pITMOFF_W152H018A );
//#else // #ifdef DEF_LCD_7INCH.
//	Key_SampleCopySetChangeMenu_actHeaderFooter.changeTextLine2( isOn( CNK_HeaderFooterStamp, PnlComGetJobCopy() ) ? s_pITMOn_W144H016A : s_pITMOFF_W144H016A );
//#endif // #ifdef DEF_LCD_7INCH.
#ifdef DEF_LCD_7INCH
	Key_SampleCopySetChangeMenu_actHeaderFooter.changeTextLine2( isOn( CNK_HeaderFooterStamp, comPanelJob ) ? s_pITMOn_W152H018A : s_pITMOFF_W152H018A );
#else // #ifdef DEF_LCD_7INCH.
	Key_SampleCopySetChangeMenu_actHeaderFooter.changeTextLine2( isOn( CNK_HeaderFooterStamp, comPanelJob ) ? s_pITMOn_W144H016A : s_pITMOFF_W144H016A );
#endif // #ifdef DEF_LCD_7INCH.

	// コピーガード.
	Bool bl_CopyGuard = True;
// Comment 2019/07/16 【IT6_EagleH】【HCL_Prabakaran】OP_BTS-8347 jobSpec Type has been changed for common use.	
//	APIC_Job* pc_JobCopy = PnlComGetJobCopy( sc_JobPutNo );
	APIC_Job* pc_JobCopy = comPanelJob;
	if( NULL == pc_JobCopy ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );	return; }
	if( ( False == PnlComGetOperationSetting()->getCopyGuardEnable() )
	    || ( 0 < pc_JobCopy->getJobSpec()->pc_Common->getJimonPattern2()->getJimonRead() ) ) {		//!<コピーガード機能検出時の表示.
		bl_CopyGuard = False;
	}
	if( ( True == bl_KeyCounterIf )
	    || ( False == bl_AppOptionStamp ) 	// 応用機能オプション「文字スタンプ」が有効でない.
	  ) {
		bl_CopyGuard = False;
	}
	Key_SampleCopySetChangeMenu_actCopyGuard.showEnable( bl_CopyGuard );
// Comment 2019/07/16 【IT6_EagleH】【HCL_Prabakaran】OP_BTS-8347 jobSpec Type has been changed for common use.
//#ifdef DEF_LCD_7INCH
//	Key_SampleCopySetChangeMenu_actCopyGuard.changeTextLine2( isOn( CNK_CopyGuard, PnlComGetJobCopy() ) ? s_pITMOn_W152H018A : s_pITMOFF_W152H018A );
//#else // #ifdef DEF_LCD_7INCH.
//	Key_SampleCopySetChangeMenu_actCopyGuard.changeTextLine2( isOn( CNK_CopyGuard, PnlComGetJobCopy() ) ? s_pITMOn_W144H016A : s_pITMOFF_W144H016A );
//#endif // #ifdef DEF_LCD_7INCH.
#ifdef DEF_LCD_7INCH
	Key_SampleCopySetChangeMenu_actCopyGuard.changeTextLine2( isOn( CNK_CopyGuard, comPanelJob ) ? s_pITMOn_W152H018A : s_pITMOFF_W152H018A );
#else // #ifdef DEF_LCD_7INCH.
	Key_SampleCopySetChangeMenu_actCopyGuard.changeTextLine2( isOn( CNK_CopyGuard, comPanelJob ) ? s_pITMOn_W144H016A : s_pITMOFF_W144H016A );
#endif // #ifdef DEF_LCD_7INCH.

	// パスワードコピー.
	Bool bl_PassWordCopy = True;
	if( False == PnlComGetOperationSetting()->getPasswordCopyEnable() ) { bl_PassWordCopy = False; }	//!<パスワードコピー機能検出時の表示.
	if( ( True == bl_KeyCounterIf )
	    || ( False == bl_AppOptionStamp )	// 応用機能オプション「文字スタンプ」が有効でない.
	  ) {
		bl_PassWordCopy = False;
	}
	Key_SampleCopySetChangeMenu_actPasswordCopy.showEnable( bl_PassWordCopy );
// Comment 2019/07/16 【IT6_EagleH】【HCL_Prabakaran】OP_BTS-8347 jobSpec Type has been changed for common use.
//#ifdef DEF_LCD_7INCH
//	Key_SampleCopySetChangeMenu_actPasswordCopy.changeTextLine2( isOn( CNK_PassWordCopy, PnlComGetJobCopy() ) ? s_pITMOn_W152H018A : s_pITMOFF_W152H018A );
//#else // #ifdef DEF_LCD_7INCH.
//	Key_SampleCopySetChangeMenu_actPasswordCopy.changeTextLine2( isOn( CNK_PassWordCopy, PnlComGetJobCopy() ) ? s_pITMOn_W144H016A : s_pITMOFF_W144H016A );
//#endif // #ifdef DEF_LCD_7INCH.
#ifdef DEF_LCD_7INCH
	Key_SampleCopySetChangeMenu_actPasswordCopy.changeTextLine2( isOn( CNK_PassWordCopy, comPanelJob ) ? s_pITMOn_W152H018A : s_pITMOFF_W152H018A );
#else // #ifdef DEF_LCD_7INCH.
	Key_SampleCopySetChangeMenu_actPasswordCopy.changeTextLine2( isOn( CNK_PassWordCopy, comPanelJob ) ? s_pITMOn_W144H016A : s_pITMOFF_W144H016A );
#endif // #ifdef DEF_LCD_7INCH.
}

//関数------------------------------------------------------------------------------------------------------------------
//概要    ：表示更新処理（ページ番号）.
//備考    ：.
//戻値    ：.
//作成者  ：.
//レビュー：.
//------------------------------------------------------------------------------
/**
* @brief A070800400 確認コピー停止中_設定変更_メニュー : 表示更新処理（ページ番号）.
*/
//------------------------------------------------------------------------------
void PCPC_SampleCopySetChangeMenu_act::setupApplicationSettingPageNumber()
{
// Comment 2019/07/16 【IT6_EagleH】【HCL_Prabakaran】OP_BTS-8347 jobSpec Type has been changed for common use.
//	const Bool bl_PageStamp = isOn( CNK_StampPage, PnlComGetJobCopy() );
	const Bool bl_PageStamp = isOn( CNK_StampPage, comPanelJob );
#ifdef DEF_LCD_7INCH
	Key_SampleCopySetChangeMenu_actPageNumber.changeTextLine2( bl_PageStamp ? s_pITMOn_W152H018A : s_pITMOFF_W152H018A );
#else // #ifdef DEF_LCD_7INCH.
	Key_SampleCopySetChangeMenu_actPageNumber.changeTextLine2( bl_PageStamp ? s_pITMOn_W144H016A : s_pITMOFF_W144H016A );
#endif // #ifdef DEF_LCD_7INCH.
}
//関数------------------------------------------------------------------------------------------------------------------
//概要    ：表示更新処理（スタンプ）.
//備考    ：.
//戻値    ：.
//作成者  ：.
//レビュー：.
//------------------------------------------------------------------------------
/**
* @brief A070800400 確認コピー停止中_設定変更_メニュー : 表示更新処理（スタンプ）.
*/
//------------------------------------------------------------------------------
void PCPC_SampleCopySetChangeMenu_act::setupApplicationSettingStamp()
{
// Comment 2019/07/16 【IT6_EagleH】【HCL_Prabakaran】OP_BTS-8347 jobSpec Type has been changed for common use.
//	const Bool bl_StampFix = isOn( CNK_StampFix, PnlComGetJobCopy() );
	const Bool bl_StampFix = isOn( CNK_StampFix, comPanelJob );
	Bool bl_KeyCounterIf = False;
	if( TYPE_KeyModeVendor == APIC_KeyDevice::getKeyMode()
	    && True == PnlComGetMachineSetting()->getKeyCounterIfVendor() ) {
		bl_KeyCounterIf = True;
	}

	// 応用機能オプション「文字スタンプ」の有効/無効.
	Bool bl_AppOptionStamp = ( PNLE_AppOptionStatus_Enable == PnlComGetEnableAppOptionStatus( TYPE_AppOptionStamp ) ) ? True : False;

	if( ( True == bl_KeyCounterIf )
	    || ( False == bl_AppOptionStamp )
	  ) {
		Key_SampleCopySetChangeMenu_actStamp.showEnable( False );
	}
#ifdef DEF_LCD_7INCH
	Key_SampleCopySetChangeMenu_actStamp.changeTextLine2( bl_StampFix ? s_pITMOn_W152H018A : s_pITMOFF_W152H018A );
#else // #ifdef DEF_LCD_7INCH.
	Key_SampleCopySetChangeMenu_actStamp.changeTextLine2( bl_StampFix ? s_pITMOn_W144H016A : s_pITMOFF_W144H016A );
#endif // #ifdef DEF_LCD_7INCH.
}

//関数------------------------------------------------------------------------------------------------------------------
//概要    ：ボタン押下.
//備考    ：.
//戻値    ：.
//作成者  ：AWS_ウィング
//レビュー：.
//変更履歴：.
//------------------------------------------------------------------------------
/**
* @brief A070800400 確認コピー停止中_設定変更_メニュー : ボタン押下
*/
//------------------------------------------------------------------------------
void PCPC_SampleCopySetChangeMenu_act::procButtonOn( uchar option )
{
	PNLC_PopupListControl* pc_PopupListControl = PNLC_PopupListControl::newInstance();		//処理 : ポップアップ管理クラスのインスタンス取得.
	short s_xPosition = PNLD_ZeroInit, s_yPosition = PNLD_ZeroInit,
	      s_width = PNLD_ZeroInit, s_height = PNLD_ZeroInit;
	switch( option ) {		// ボタンを押す時.
		case PNLD_BTNID_SampleCopySetChangeMenu_actPrint:		// 印際キーを押す時.
			if( PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_DuplexCombine_Duplex_1To1 ) == False &&		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
			    PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_DuplexCombine_Duplex_2To1 ) == False ) {		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
				pc_PopupListControl->addPopup( &gp_SampleCopySetChangeMenu_actPullDown_TwoSide, this, NULL, 42, 54, 716, 372 );
			} else if( PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_DuplexCombine_Duplex_1To2 ) == False &&		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
			           PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_DuplexCombine_Duplex_2To2 ) == False ) {		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
				pc_PopupListControl->addPopup( &gp_SampleCopySetChangeMenu_actPullDown_OneSide, this, NULL, 42, 54, 716, 372 );
			} else {
				pc_PopupListControl->addPopup( &gp_SampleCopySetChangeMenu_actPullDown, this, NULL, 42, 54, 716, 372 );
			}
			break;
		case PNLD_BTNID_SampleCopySetChangeMenu_actFinishing:		// 仕上りキーを押す時.
			Key_SampleCopySetChangeMenu_actFinishing.getPosition( s_xPosition, s_yPosition );		// 仕上りキー.
			Key_SampleCopySetChangeMenu_actFinishing.getSize( s_width, s_height );
			PNLC_PopupListControl::newInstance()->addPopup( &gp_A070800500, this, NULL, s_xPosition, s_yPosition , s_width, s_height );
			break;
		case PNLD_BTNID_SampleCopySetChangeMenu_actBooklet:		// 小冊子キーを押す時.
			Key_SampleCopySetChangeMenu_actBooklet.getPosition( s_xPosition, s_yPosition );		// 小冊子キー.
			Key_SampleCopySetChangeMenu_actBooklet.getSize( s_width, s_height );
			PNLC_PopupListControl::newInstance()->addPopup( &gp_SetChangeBooklet_act, this, NULL, s_xPosition, s_yPosition , s_width, s_height );
			break;
		case PNLD_BTNID_SampleCopySetChangeMenu_actImageShift:		// 画像シフトキーを押す時.
			Key_SampleCopySetChangeMenu_actImageShift.getPosition( s_xPosition, s_yPosition );		// 画像シフトキー.
			Key_SampleCopySetChangeMenu_actImageShift.getSize( s_width, s_height );
			PNLC_PopupListControl::newInstance()->addPopup( &gp_SetChangeShiftEditFront_act, this, NULL, s_xPosition, s_yPosition , s_width, s_height );
			break;
		case PNLD_BTNID_SampleCopySetChangeMenu_actPageMargin:		// とじしろキーを押す時.
			Key_SampleCopySetChangeMenu_actPageMargin.getPosition( s_xPosition, s_yPosition );		// とじしろキー.
			Key_SampleCopySetChangeMenu_actPageMargin.getSize( s_width, s_height );
			PNLC_PopupListControl::newInstance()->addPopup( &gp_A070201200, this, NULL, s_xPosition, s_yPosition , s_width, s_height );
			break;
		case PNLD_BTNID_SampleCopySetChangeMenu_actCoverSheet:					// カバーシートキーを押す時.
			Key_SampleCopySetChangeMenu_actCoverSheet.getPosition( s_xPosition, s_yPosition );
			Key_SampleCopySetChangeMenu_actCoverSheet.getSize( s_width, s_height );
			PNLC_PopupListControl::newInstance()->addPopup( &gp_SampleSetChangePageCover_act, this, NULL, s_xPosition, s_yPosition , s_width, s_height );
			break;
		case PNLD_BTNID_SampleCopySetChangeMenu_actCoverInsertion:				// インターシートキーを押す時.
			Key_SampleCopySetChangeMenu_actSheetCoverInsertion.getPosition( s_xPosition, s_yPosition );
			Key_SampleCopySetChangeMenu_actSheetCoverInsertion.getSize( s_width, s_height );
			PNLC_PopupListControl::newInstance()->addPopup( &gp_SetChangePageIntersheet_act, this, NULL, s_xPosition, s_yPosition , s_width, s_height );
			break;
		case PNLD_BTNID_SampleCopySetChangeMenu_actProgramImageInsert:			// 差込みページキーを押す時.
			if( True == combinationCheck( CNK_OriginalInsertion ) ) {
				Key_SampleCopySetChangeMenu_actCopyProgramImageInsert.getPosition( s_xPosition, s_yPosition );
				Key_SampleCopySetChangeMenu_actCopyProgramImageInsert.getSize( s_width, s_height );
				PNLC_PopupListControl::newInstance()->addPopup( &gp_SetChangePageInsertEdit_act, this, NULL, s_xPosition, s_yPosition , s_width, s_height );
			}
			break;
		case PNLD_BTNID_SampleCopySetChangeMenu_actChapterPaper:				// 章分けキーを押す時.
			Key_SampleCopySetChangeMenu_actChapterPaper.getPosition( s_xPosition, s_yPosition );
			Key_SampleCopySetChangeMenu_actChapterPaper.getSize( s_width, s_height );
			PNLC_PopupListControl::newInstance()->addPopup( &gp_SetChangePageChapterEdit_act, this, NULL, s_xPosition, s_yPosition , s_width, s_height );
			break;
		case PNLD_BTNID_SampleCopySetChangeMenu_actDateTime:					// 日付/時刻キーを押す時.
			if( isOn( CNK_ForcedAddDate, comPanelJob ) == False ) {
				Key_SampleCopySetChangeMenu_actDateTime.getPosition( s_xPosition, s_yPosition );
				Key_SampleCopySetChangeMenu_actDateTime.getSize( s_width, s_height );
				PNLC_PopupListControl::newInstance()->addPopup( &gp_A070202700, this, NULL, s_xPosition, s_yPosition , s_width, s_height );
			} else {
				onlyPanel.ep_Combination = CNK_ForcedAddDate;	//フラグをセットする
			}
			break;
		case PNLD_BTNID_SampleCopySetChangeMenu_actPageNumber:					// ページ番号キーを押す時.
			if( isOn( CNK_ForcedAddPage, comPanelJob ) == False ) {
				Key_SampleCopySetChangeMenu_actPageNumber.getPosition( s_xPosition, s_yPosition );
				Key_SampleCopySetChangeMenu_actPageNumber.getSize( s_width, s_height );
				PNLC_PopupListControl::newInstance()->addPopup( &gp_A070202800, this, NULL, s_xPosition, s_yPosition , s_width, s_height );
			} else {
				onlyPanel.ep_Combination = CNK_ForcedAddPage;	//フラグをセットする
			}
			break;
		case PNLD_BTNID_SampleCopySetChangeMenu_actStamp:						// スタンプキーを押す時.
			if( isOn( CNK_ForcedAddStamp, comPanelJob ) == False ) {
				Key_SampleCopySetChangeMenu_actStamp.getPosition( s_xPosition, s_yPosition );
				Key_SampleCopySetChangeMenu_actStamp.getSize( s_width, s_height );
				PNLC_PopupListControl::newInstance()->addPopup( &gp_A070201700, this, NULL, s_xPosition, s_yPosition , s_width, s_height );
			} else {
				onlyPanel.ep_Combination = CNK_ForcedAddStamp;	//フラグをセットする
			}

			break;
		case PNLD_BTNID_SampleCopySetChangeMenu_actStampRepeat:					// 繰り返しスタンプキーを押す時.
			if( isOn( CNK_ForcedAddWatermark, comPanelJob ) == False ) {
				Key_SampleCopySetChangeMenu_actStampRepeat.getPosition( s_xPosition, s_yPosition );
				Key_SampleCopySetChangeMenu_actStampRepeat.getSize( s_width, s_height );
				PNLC_PopupListControl::newInstance()->addPopup( &gp_A070203100, this, NULL, s_xPosition, s_yPosition , s_width, s_height );
			} else {
				onlyPanel.ep_Combination = CNK_ForcedAddWatermark;	//フラグをセットする
			}
			break;
		case PNLD_BTNID_SampleCopySetChangeMenu_actHeaderFooter:				// ヘッダー/フッターキーを押す時.
			if( isOn( CNK_ForcedAddHeaderFooter, comPanelJob ) == False ) {
				Key_SampleCopySetChangeMenu_actHeaderFooter.getPosition( s_xPosition, s_yPosition );
				Key_SampleCopySetChangeMenu_actHeaderFooter.getSize( s_width, s_height );
				PNLC_PopupListControl::newInstance()->addPopup( &gp_A070203800, this, NULL, s_xPosition, s_yPosition , s_width, s_height );
			} else {
				onlyPanel.ep_Combination = CNK_ForcedAddHeaderFooter;	//フラグをセットする
			}
			break;
		case PNLD_BTNID_SampleCopySetChangeMenu_actCopyProtect:					// コピープロテクトキーを押す時.
			if( isOn( CNK_ForcedAddCopyProtect, comPanelJob ) == False ) {
				Key_SampleCopySetChangeMenu_actCopyProtect.getPosition( s_xPosition, s_yPosition );
				Key_SampleCopySetChangeMenu_actCopyProtect.getSize( s_width, s_height );
				PNLC_PopupListControl::newInstance()->addPopup( &gp_A070204400, this, NULL, s_xPosition, s_yPosition , s_width, s_height );
			} else {
				onlyPanel.ep_Combination = CNK_ForcedAddCopyProtect;	//フラグをセットする
			}
			break;
		case PNLD_BTNID_SampleCopySetChangeMenu_actCopyGuard:					// コピーガードキーを押す時.
			if( isOn( CNK_ForcedAddCopyGuard, comPanelJob ) == False ) {
				Key_SampleCopySetChangeMenu_actCopyGuard.getPosition( s_xPosition, s_yPosition );
				Key_SampleCopySetChangeMenu_actCopyGuard.getSize( s_width, s_height );
				PNLC_PopupListControl::newInstance()->addPopup( &gp_A070205200, this, NULL, s_xPosition, s_yPosition , s_width, s_height );
			} else {
				onlyPanel.ep_Combination = CNK_ForcedAddCopyGuard;	//フラグをセットする
			}
			break;
		case PNLD_BTNID_SampleCopySetChangeMenu_actPasswordCopy:				// パスワードコピーキーを押す時.
			if( isOn( CNK_ForcedAddPassWordCopy, comPanelJob ) == False ) {
				Key_SampleCopySetChangeMenu_actPasswordCopy.getPosition( s_xPosition, s_yPosition );
				Key_SampleCopySetChangeMenu_actPasswordCopy.getSize( s_width, s_height );
				PNLC_PopupListControl::newInstance()->addPopup( &gp_A070201100, this, NULL, s_xPosition, s_yPosition , s_width, s_height );
			} else {
				onlyPanel.ep_Combination = CNK_ForcedAddPassWordCopy;	//フラグをセットする
			}
			break;
		case PNLD_BTNID_SampleCopySetChangeMenu_actNumInputArea:
			// テンキーポップアップ表示.
			show10KeyPopup();
			break;
		default: PPopupDecisionButtonCom::procButtonOn( option ); break;
	}
}
//関数------------------------------------------------------------------------------------------------------------------
//概要    ：設定取消.
//備考    ：.
//戻値    ：.
//作成者  ：1OSW-oikawa
//レビュー：.
//------------------------------------------------------------------------------
/**
* @brief A070800400 確認コピー停止中_設定変更_メニュー : 設定取消.
*/
//------------------------------------------------------------------------------
void PCPC_SampleCopySetChangeMenu_act::procCancel( uchar option )
{
	c_SettingChangeSampleCopy.PnlRecoverySettingSampleCopySpecInfo( comPanelJob->getJobSpec(), comPanelJob->getJobStatus() );	// 設定データを復帰.
	PNLC_PopupListControl::newInstance()->closeTop( False );		// 関数 Call 処理： ポップアップ終了させる.
}
//関数------------------------------------------------------------------------------------------------------------------
//概要    ：スタート.
//備考    ：.
//戻値    ：.
//作成者  ：AWS_ウィング
//レビュー：.
//変更履歴：.
//------------------------------------------------------------------------------
/**
* @brief A070800400 確認コピー停止中_設定変更_メニュー : スタート
*/
//------------------------------------------------------------------------------
void PCPC_SampleCopySetChangeMenu_act::procStart( uchar option )
{
	checkSoft10KeyData();
	TYPC_Common* pc_Common = comPanelJob->getJobSpec()->pc_Common;		// 変数を初期化.
	if( False == printStartDisable() ) {		// プリント禁止でない時.
		//	 お試しプリントフラグをおろす.
		pc_Common->c_JobType.setSingleStepOn( False );		//  印刷を開始できます.
		c_SystemRequest.JobRestart( comPanelJob );
	} else {
		//	 印刷を開始できない.
		setBuzzerType( PCGE_BuzzerType_InputNg );		// 印刷禁止ブザー.
		setupGamen();
	}
}

//関数------------------------------------------------------------------------------------------------------------------
//概要    ：クリアキー押下処理.
//備考    ：.
//戻値    ：.
//作成者  ：AWS_ウィング
//レビュー：.
//変更履歴：.
//------------------------------------------------------------------------------
/**
* @brief A070800400 確認コピー停止中_設定変更_メニュー : クリアキー押下処理
*/
//------------------------------------------------------------------------------
void PCPC_SampleCopySetChangeMenu_act::procClear( uchar option )
{
	comPanelJob->getJobStatus()->setNumOfCopies( PCGD_DefaultCopyNumber );		//  部数をデフォルトに戻し、直接入力フラグを設定.
	comPanelJob->getJobSpec()->pc_Print->setNumOfCopies( 2 );		//  総部数は「２」と設定する CQ349591 1OSW-oikawa.

	if( True == comPanelJob->getJobSpec()->pc_Common->getMemoryCopy()		// 入力した部数を初期化（１）する。.
	    && True == comPanelJob->getJobStatus()->getConfirmPrintStopState()
	    && False == comPanelJob->getJobSpec()->pc_Common->c_JobType.getSingleStepOn() ) {
		comPanelJob->getJobSpec()->pc_Print->setNumOfCopies( 1 );		//  メモリコピー停止中のため、総部数は「1」と設定する.
	}
	PCGC_JobSpecCopyBackup* pc_JobSpecCopyBackup = PnlComGetJobSpecCopyBack();		//  直接入力フラグを初期化.
	if( NULL == pc_JobSpecCopyBackup ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );	return; }	// NULLチェックを追加.//【 静的解析対応 : 仁枝 : 2011.03.02 : CID-288969 】.
	pc_JobSpecCopyBackup->b_SampleStopMenuCopyDirectInput = True;
	setupGamen();		//  setupGamen()をコールされる.
}

//関数------------------------------------------------------------------------------------------------------------------
//概要    ：テン(0～9)キー..
//備考    ：.
//戻値    ：.
//作成者  ：AWS_ウィング
//レビュー：.
//変更履歴：.
//------------------------------------------------------------------------------
/**
* @brief A070800400 確認コピー停止中_設定変更_メニュー : テン(0～9)キー.
*/
//------------------------------------------------------------------------------
void PCPC_SampleCopySetChangeMenu_act::proc10Key( uchar option )
{
// Comment 2019/07/16 【IT6_EagleH】【HCL_Prabakaran】OP_BTS-8347 jobSpec Type has been changed for common use.
//	APIC_Job* pc_Job = PnlComGetJobCopy();
	PCGC_JobSpecCopyBackup* pc_JobSpecCopyBackup = PnlComGetJobSpecCopyBack();		//  直接入力フラグを取得.
	Bool b_isError = False;			// エラー制御フラグ.
	APIC_SoftDipSw* pc_SoftDipSw = APIC_SoftDipSw::newInstance();
	int32_t i32_MaxNumOfCopiesAtAdminMachineVendor = PnlComGetCopyJobSetting()->getMaxNumOfCopiesAtAdminMachineVendor();
// Comment 2019/07/16 【IT6_EagleH】【HCL_Prabakaran】OP_BTS-8347 jobSpec Type has been changed for common use.
//	if( NULL == pc_Job || NULL == pc_JobSpecCopyBackup ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );	return; }	// NULLチェックを追加.
	if( NULL == comPanelJob || NULL == pc_JobSpecCopyBackup ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );	return; }	// NULLチェックを追加.
	int32_t i32_NumberOfCopyNow = comPanelJob->getJobStatus()->getNumOfCopies();		//  取得した部数と入力された数値を元に部数を算出.
	if( True == comPanelJob->getJobSpec()->pc_Common->getMemoryCopy()		// メモリコピーである、OHP合紙である場合、テンキー受付禁止.
// Comment 2019/07/16 【IT6_EagleH】【HCL_Prabakaran】OP_BTS-8347 jobSpec Type has been changed for common use.
//	    && True == isOn( CNK_OHP, pc_Job->getJobSpec() ) ) {
	    && True == isOn( CNK_OHP, comPanelJob->getJobSpec() ) ) {
		return;		//Return処理：.

	} else {

		// プリント部数制限ありの場合.
		if( True == isLimitedPrintNumberOfCopies() ) {
			i32_NumberOfCopyNow = proc10KeyInput( i32_NumberOfCopyNow, getMaxPrintNumberOfCopies(), option, pc_JobSpecCopyBackup->b_SampleStopMenuCopyDirectInput, EP_InputMaxLimitPattern_Refuse, &b_isError );
		} else if( True == pc_SoftDipSw->getSoftDipSwBitForPanelSpecNo( 130, 0 ) ) { 	// DipSWNo.130 bit0.
			i32_NumberOfCopyNow = proc10KeyInput( i32_NumberOfCopyNow, i32_MaxNumOfCopiesAtAdminMachineVendor, option,
			                                     pc_JobSpecCopyBackup->b_SampleStopMenuCopyDirectInput, EP_InputMaxLimitPattern_Refuse, &b_isError );
			// プリント部数制限なしの場合.
		} else {
			i32_NumberOfCopyNow = proc10KeyInput( i32_NumberOfCopyNow, PCGD_MaxCopyNumber, option, pc_JobSpecCopyBackup->b_SampleStopMenuCopyDirectInput, EP_InputMaxLimitPattern_Refuse, &b_isError );
		}
		if( b_isError == True ) {		// .
			setBuzzerType( PCGE_BuzzerType_InputNg );		//  最大値を超えた入力の場合.
		}
	}
	int32_t i32_NumberOfCopySet = i32_NumberOfCopyNow + PCGD_DefaultCopyNumber;		//  先出し分「1部」と本画面にて設定された値を足すと、総部数(設定値)が算出される.
	if( True == comPanelJob->getJobSpec()->pc_Common->getMemoryCopy()		// .
	    && True == comPanelJob->getJobStatus()->getConfirmPrintStopState()
	    && False == comPanelJob->getJobSpec()->pc_Common->c_JobType.getSingleStepOn() ) {
		// メモリコピー停止中のため、先だし分は無い.
		// 確認コピーではない、メモリコピージョブである、メモリコピー停止中の場合.
		i32_NumberOfCopySet = i32_NumberOfCopyNow;
	}
	// プリント部数制限ありの場合.
	if( True == isLimitedPrintNumberOfCopies() ) {
		// 総部数が制限値以上になった場合.
		if( True == b_isError || getMaxPrintNumberOfCopies() < i32_NumberOfCopySet ) {
			onlyPanel.setPanelWarning( PNL_PCAUTION_NumOfCopyNotSet );	// プリント部数制限警告.
			setupGamen();
			return;		// 入力値を更新させずに抜ける.
		}
	} else if( True == pc_SoftDipSw->getSoftDipSwBitForPanelSpecNo( 130, 0 ) ) { 	// DipSWNo.130 bit0.
		if( i32_MaxNumOfCopiesAtAdminMachineVendor < i32_NumberOfCopySet ) {		//  総部数が部数上限値以上になった場合設定しない.
			setBuzzerType( PCGE_BuzzerType_InputNg );		//  拒否音鳴動.
			return;		//Return処理：.
		}
	} else {

		if( PCGD_MaxCopyNumber < i32_NumberOfCopySet ) {		//  総部数が最大数以上になった場合設定しない.
			setBuzzerType( PCGE_BuzzerType_InputNg );		//  拒否音鳴動.
			return;		//Return処理：.
		}
	}
	comPanelJob->getJobStatus()->setNumOfCopies( i32_NumberOfCopyNow );		//  算出された部数を設定（システム側の処理に合わせてStatsuに設定）. oikawa 先出し分を加算
	comPanelJob->getJobSpec()->pc_Print->setNumOfCopies( i32_NumberOfCopySet );		//  算出された部数を設定（読み込み中画面やジョブ履歴等パネルに合わせて設定）CQ344609 1OSW-oikawa  先出し分を加算.
	if( pc_JobSpecCopyBackup->b_SampleStopMenuCopyDirectInput /*&& (0 != option)*/ ) {		// 直接入力の場合は直接入力フラグをクリア.
		pc_JobSpecCopyBackup->b_SampleStopMenuCopyDirectInput = False;		// 直接入力フラグを初期化する.
	}
	setupGamen();		//  setupGamen()をコールされる.
}

//関数------------------------------------------------------------------------------------------------------------------
//概要    ：パネルリセット.
//備考    ：.
//戻値    ：.
//作成者  ：AWS_ウィング
//レビュー：.
//変更履歴：.
//------------------------------------------------------------------------------
/**
* @brief A070800400 確認コピー停止中_設定変更_メニュー : パネルリセット
*/
//------------------------------------------------------------------------------
void PCPC_SampleCopySetChangeMenu_act::procReset( uchar option )
{
	c_SettingChangeSampleCopy.PnlRecoverySettingSampleCopySpecInfo( comPanelJob->getJobSpec(), comPanelJob->getJobStatus() );	// 設定データを復帰.
	if( TYPE_DuplexModeOn == comPanelJob->getJobSpec()->pc_Print->getDuplexMode() ) {		// 両面プリントON. .
		comPanelJob->getJobStatus()->setDuplexPrint( True );		// .
	} else {
		comPanelJob->getJobStatus()->setDuplexPrint( False );		// .
	}
	setupGamen();		// .
}
// comment 2020/07/14 SCD2-Sky 石原 [IT6_2.0MR2][OP_BTS-25362] フッターエリア対応.
//------------------------------------------------------------------------------
/**
* @brief A070800400 確認コピー停止中_設定変更_メニュー : リセットキー活性状態.
*/
//------------------------------------------------------------------------------
Bool PCPC_SampleCopySetChangeMenu_act::resetKeyActiveCheck()
{
	ACTIVEKEY_RET( True );
}
//関数------------------------------------------------------------------------------------------------------------------
//概要    ：確認コピー.
//備考    ：.
//戻値    ：.
//作成者  ：AWS_ウィング
//レビュー：.
//変更履歴：.
//------------------------------------------------------------------------------
/**
* @brief A070800400 確認コピー停止中_設定変更_メニュー : 確認コピー
*/
//------------------------------------------------------------------------------
void PCPC_SampleCopySetChangeMenu_act::procSampleCopy( uchar option )
{
	checkSoft10KeyData();
	if( False == printStartDisable() ) {		//  プリント禁止でない時.
		short retStatus = c_SettingChangeSampleCopy.StartSingleStepJobSample();		// 値取得.
		if( -1 != retStatus ) {		// retStatusは-1じゃない場合.
			switch( retStatus ) {		//  メモリコピーの確認コピー開始である場合.
				case 1:		// 確認コピー開始可.
					c_SystemRequest.JobRestart( comPanelJob );
					return;
				case 2:		// 確認コピー開始はできない 画面更新は必要.
					setupGamen();
					return;
				case 0:		// 確認コピー開始不可.
				default:
					return;
			}
		}
		if( True == c_SettingChangeSampleCopy.RestartSingleStepJobSample() ) {
			return;		//Return処理：.
		}
	} else {
		setBuzzerType( PCGE_BuzzerType_InputNg );		// NG.
	}
	setupGamen();		// 画面セットアップ.
}

//------------------------------------------------------------------------------
/**
* @brief A070800400 確認コピー停止中_設定変更_メニュー : パターンカスタマイズ対応
*/
//------------------------------------------------------------------------------
void PCPC_SampleCopySetChangeMenu_act::showPatternCustomize()
{
	// ライブラリ使用.
	PNLC_FuncPatternSettingLib* cp_FuncSettingLib = PNLC_FuncPatternSettingLib::newInstance();
	uchar appCnt = PNLD_SampleCopySetChangeMenu_MaxAppSetting;

	// 印刷ドロップダウンキーの項目変更.
	if( PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_DuplexCombine_Duplex_1To1 ) == False &&		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
	    PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_DuplexCombine_Duplex_2To1 ) == False ) {		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
		Key_SampleCopySetChangeMenu_actPrint.showEnable( False );		// 両方のプルダウンを実装したキー.
		Key_SampleCopySetChangeMenu_actPrint_OneSide.showEnable( False );		// 「片面」のみのプルダウンを実装したキー.
	} else if( PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_DuplexCombine_Duplex_1To2 ) == False &&		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
	           PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_DuplexCombine_Duplex_2To2 ) == False ) {		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
		Key_SampleCopySetChangeMenu_actPrint.showEnable( False );
		Key_SampleCopySetChangeMenu_actPrint_TwoSide.showEnable( False );		// 「両面」のみのプルダウンを実装したキー.
	} else {
		Key_SampleCopySetChangeMenu_actPrint_OneSide.showEnable( False );
		Key_SampleCopySetChangeMenu_actPrint_TwoSide.showEnable( False );
	}

	if( ( PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_DuplexCombine ) )        == False ||		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
	    ( PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_DuplexCombine_Duplex ) ) == False ) {		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
		Key_SampleCopySetChangeMenu_actPrint.showEnable( False );		// 「印刷」キー.
		Key_SampleCopySetChangeMenu_actPrint_OneSide.showEnable( False );		// 「印刷」キー.
		Key_SampleCopySetChangeMenu_actPrint_TwoSide.showEnable( False );		// 「印刷」キー.
	}
	else {
		short sc_ItemCnt = 4;	// 子アイテム数.
		// 片面>片面.
		if( PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_DuplexCombine_Duplex_1To1 ) == False ) {		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
			sc_ItemCnt--;
		}
		// 片面>両面.
		if( PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_DuplexCombine_Duplex_1To2 ) == False ) {		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
			sc_ItemCnt--;
		}
		// 両面>片面.
		if( PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_DuplexCombine_Duplex_2To1 ) == False ) {		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
			sc_ItemCnt--;
		}
		// 両面>両面.
		if( PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_DuplexCombine_Duplex_2To2 ) == False ) {		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
			sc_ItemCnt--;
		}
		//.
		// 全て非表示の場合はドロップダウンボタンも非表示.
		if( sc_ItemCnt <= 0 ) {
			Key_SampleCopySetChangeMenu_actPrint.showEnable( False );
			Key_SampleCopySetChangeMenu_actPrint_OneSide.showEnable( False );
			Key_SampleCopySetChangeMenu_actPrint_TwoSide.showEnable( False );
		}
	}

	// 「仕上り」キー.
	if( PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_Finishing ) == False ) {		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
		Key_SampleCopySetChangeMenu_actFinishing.showEnable( False );
	}
	else {
// A3A4FUM so_okada 2015.02.12 Start.
#ifdef DEF_A4_Cus
		APIC_PrinterSystem* pc_PrinterSystem = PnlComGetPrinterSystem();
		if( NULL != pc_PrinterSystem ){
			if ( (False == pc_PrinterSystem->isExistFinisher()) ||
			     (TYPE_TemporaryRescueStatusSet == ppc_SystemStatus->getTemporaryRescueUserStatus(TYPE_TemporaryRescueUnitFinisher)) ){	// 【管理No.1433_4_4、制御構成変更_No.335】仕上げモード設定（後処理装置設定モード）Finisher装着時 OPS2 Sky西場 2015/11/05.
				Key_SampleCopySetChangeMenu_actFinishing.showEnable( False );
			}else{
				// ステープル.
				Bool b_Staple = True;
				if ( False == PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_Finishing_CornerStaple)) {		// ユーザー部門別パターン設定.
					b_Staple = False;
				}
				if( False == PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_Finishing_Offset     ) &&		// ユーザー部門別パターン設定.
					False == PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_Finishing_OutputTray ) &&		// ユーザー部門別パターン設定.
					False == b_Staple) {
					Key_SampleCopySetChangeMenu_actFinishing.showEnable( False );
				}
			}
		}
#else
		
		// 伊丹Sky(山村和)　2010/09/08 CQ333120 ↓.
		// 紙折り/中折りの判定.
		Bool b_FoldBind = PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_Finishing_FoldBind );		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
		if( True == b_FoldBind ) {
			if( False == PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_Finishing_FoldBind_HalfFold ) &&		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
			    False == PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_Finishing_FoldBind_CenterStapleAndFold ) &&		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
			    False == PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_Finishing_FoldBind_3Fold ) &&		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
			    False == PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_Finishing_FoldBind_ZFold ) ) {		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
				b_FoldBind = False;
			}
		}

		// パンチの判定(現在設定されているパンチのみ判定する).
		// パンチ機能状態取得.
		TYPS_PunchFunction s_PunchFunction = PnlComGetPrinterSystem()->getPunchFunction();
		// 有効パンチ機能テーブル.
		Bool b_AryPunchFunction[4];
		b_AryPunchFunction[0] = s_PunchFunction.b_Punch2Hole;
		b_AryPunchFunction[1] = s_PunchFunction.b_Punch3Hole;
		b_AryPunchFunction[2] = s_PunchFunction.b_Punch4Hole;
		b_AryPunchFunction[3] = s_PunchFunction.b_Punch4HoleSWE;
		const uchar uc_PunchFunctionCnt = ( uchar )( sizeof( b_AryPunchFunction ) / sizeof( Bool ) );
		// パンチ種類.
		uchar uc_AryPunchPanelMenu[] = {
			TYPE_PNLMENU_COPY_Finishing_Punch2,	// 2穴パンチ.
			TYPE_PNLMENU_COPY_Finishing_Punch3,	// 3穴パンチ.
			TYPE_PNLMENU_COPY_Finishing_Punch4,	// 4穴パンチ.
			TYPE_PNLMENU_COPY_Finishing_Punch4,	// 4穴パンチ.
		};
		Bool b_Punch = False;
		uchar i = 0;
		for( i = 0; i < uc_PunchFunctionCnt; i++ )
		{
			if( True == b_AryPunchFunction[i] ) {
				b_Punch = PnlComGetFunctionShowCopy( uc_AryPunchPanelMenu[i] );		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
				if( True == b_Punch ) { break; }
			}
		}

		// ステープル.
		Bool b_Staple = True;
		if( False == PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_Finishing_CornerStaple ) &&		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
		    False == PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_Finishing_2Position ) ) {		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
			b_Staple = False;
		}
		// 位置指定.
		Bool b_Position = True;
		if( ( False == b_Punch && False == b_Staple ) ||
		    False == PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_Finishing_Punch_PositionSetting ) ) {		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
			b_Position = False;
		}

		if( False == PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_Finishing_Offset ) &&		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
		    False == PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_Finishing_OutputTray ) &&		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
		    False == b_FoldBind                                                                     &&
		    False == b_Punch                                                                        &&
		    False == b_Staple                                                                       &&
		    False == b_Position ) {
			Key_SampleCopySetChangeMenu_actFinishing.showEnable( False );
		}
		// 伊丹Sky(山村和)　2010/09/08 CQ333120 ↑.
		
#endif //DEF_A4_Cus.
// A3A4FUM so_okada 2015.02.12 End.
	}

	uchar uc_Page = Cmp_SampleCopySetChangeMenu_actScrollBar.getSliderPage();
	// 「小冊子」キー.
	if( PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_Booklet ) == False ) {
		if( 1 == uc_Page ) { Key_SampleCopySetChangeMenu_actBooklet.showEnable( False ); }
		appCnt--;
	}
	else {
		short sc_ItemCnt = 5;	// 子アイテム数 ※「用紙」は遷移先画面に無いボタンなので計上・チェックしない.
		// 左開き.
		if( PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_Booklet_LeftBind ) == False ) {		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
			sc_ItemCnt--;
		}
		// 右開き.
		if( PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_Booklet_RightBind ) == False ) {		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
			sc_ItemCnt--;
		}
		// 中とじ.
		if( PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_Booklet_CenterStapleFold ) == False ) {		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
			sc_ItemCnt--;
		}
		// 中折り.
		if( PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_Booklet_HalfFold ) == False ) {		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
			sc_ItemCnt--;
		}
		// カバーシート.
		if( PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_Booklet_CoverMode ) == False ) {		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
			sc_ItemCnt--;
		}
		//.
		// 全て非表示の場合は非表示.
		if( sc_ItemCnt <= 0 ) {
			if( 1 == uc_Page ) { Key_SampleCopySetChangeMenu_actBooklet.showEnable( False ); }
			appCnt--;
		}
	}

	// 「とじしろ」キー.
	if( PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_PageMargin ) == False ) {
		if( 1 == uc_Page ) { Key_SampleCopySetChangeMenu_actPageMargin.showEnable( False ); }
		appCnt--;
	}
	else {
		short sc_ItemCnt = 5;	// 子アイテム数 ※「原稿セット方向」は遷移先画面に無いボタンなので計上・チェックしない.
		// 自動.
		if( PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_PageMargin_Auto ) == False ) {		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
			sc_ItemCnt--;
		}
		// 上開き とじ.
		if( PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_PageMargin_Top ) == False ) {		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
			sc_ItemCnt--;
		}
		// 左開き とじ.
		if( PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_PageMargin_Left ) == False ) {		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
			sc_ItemCnt--;
		}
		// 右開き とじ.
		if( PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_PageMargin_Right ) == False ) {		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
			sc_ItemCnt--;
		}
		// とじしろ量の調整.
		if( PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_PageMargin_Adjustment ) == False ) {		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
			sc_ItemCnt--;
		}
		//.
		// 全て非表示の場合は非表示.
		if( sc_ItemCnt <= 0 ) {
			if( 1 == uc_Page ) { Key_SampleCopySetChangeMenu_actPageMargin.showEnable( False ); }
			appCnt--;
		}
	}

	// 「画像シフト」キー.
	if( PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_ImageShift ) == False ) {
		if( 1 == uc_Page ) { Key_SampleCopySetChangeMenu_actImageShift.showEnable( False ); }
		appCnt--;
	}

	// カバーシート.
	if( False == cp_FuncSettingLib->isFunctionShowCopyCoverSheet() ) {
		if( 1 == uc_Page ) { Key_SampleCopySetChangeMenu_actCoverSheet.showEnable( False ); }
		appCnt--;
	}
	// 章分け.
	if( False == cp_FuncSettingLib->isFunctionShowCopyChapter() ) {
		if( 1 == uc_Page ) { Key_SampleCopySetChangeMenu_actChapterPaper.showEnable( False ); }
		appCnt--;
	}
	// 差込みページ.
	if( False == cp_FuncSettingLib->isFunctionShowCopyInsertImage() ) {
		if( 1 == uc_Page ) { Key_SampleCopySetChangeMenu_actCopyProgramImageInsert.showEnable( False ); }
		appCnt--;
	}
	// インターシート.
	if( False == cp_FuncSettingLib->isFunctionShowCopyInsertSheet() ) {
		if( 1 == uc_Page ) { Key_SampleCopySetChangeMenu_actSheetCoverInsertion.showEnable( False ); }
		appCnt--;
	}
	// 日付 / 時刻.
	if( False == cp_FuncSettingLib->isFunctionShowCopyDateTime() ) {
		if( 1 == uc_Page ) { Key_SampleCopySetChangeMenu_actDateTime.showEnable( False ); }
		appCnt--;
	}
	// ページ番号.
	if( False == cp_FuncSettingLib->isFunctionShowCopyPageNumber() ) {
#ifdef DEF_LCD_7INCH
		if( 2 == uc_Page ) { Key_SampleCopySetChangeMenu_actPageNumber.showEnable( False ); }
#else // #ifdef DEF_LCD_7INCH.
		if( 1 == uc_Page ) { Key_SampleCopySetChangeMenu_actPageNumber.showEnable( False ); }
#endif // #ifdef DEF_LCD_7INCH.
		appCnt--;
	}
	// スタンプ.
	if( False == cp_FuncSettingLib->isFunctionShowCopyStamp() ) {
#ifdef DEF_LCD_7INCH
		if( 2 == uc_Page ) { Key_SampleCopySetChangeMenu_actStamp.showEnable( False ); }
#else // #ifdef DEF_LCD_7INCH.
		if( 1 == uc_Page ) { Key_SampleCopySetChangeMenu_actStamp.showEnable( False ); }
#endif // #ifdef DEF_LCD_7INCH.
		appCnt--;
	}
	// ヘッダー / フッター
	if( False == cp_FuncSettingLib->isFunctionShowCopyHeaderFooter() ) {
		if( 2 == uc_Page ) { Key_SampleCopySetChangeMenu_actHeaderFooter.showEnable( False ); }
		appCnt--;
	}
	// 繰り返しスタンプ.
	if( False == cp_FuncSettingLib->isFunctionShowCopyStampRepeat() ) {
		if( 2 == uc_Page ) { Key_SampleCopySetChangeMenu_actStampRepeat.showEnable( False ); }
		appCnt--;
	}
	// コピープロテクト.
	if( False == cp_FuncSettingLib->isFunctionShowCopyCopyProtect() ) {
		if( 2 == uc_Page ) { Key_SampleCopySetChangeMenu_actCopyProtect.showEnable( False ); }
		appCnt--;
	}
	// コピーガード.
	if( False == cp_FuncSettingLib->isFunctionShowCopyCopyGuard() ) {
		if( 2 == uc_Page ) { Key_SampleCopySetChangeMenu_actCopyGuard.showEnable( False ); }
		appCnt--;
	}
	// パスワードコピー.
	if( False == cp_FuncSettingLib->isFunctionShowCopyPasswordCopy() ) {
		if( 2 == uc_Page ) { Key_SampleCopySetChangeMenu_actPasswordCopy.showEnable( False ); }
		appCnt--;
	}

	// グループ背景の非表示
	if( appCnt <= 0 ) {
		Bak_SampleCopySetChangeMenu_actApplicationSetup.showEnable( False );
		Txt_SampleCopySetChangeMenu_actApplicationSetup.showEnable( False );
		Cmp_SampleCopySetChangeMenu_actScrollBar.showEnable( False );
	}
}

//------------------------------------------------------------------------------.
/**
 *	@brief	A070800400 確認コピー停止中_設定変更_メニュー : ユーティリティ
 */
//------------------------------------------------------------------------------.
void PCPC_SampleCopySetChangeMenu_act::procUtility( uchar option )
{
	checkSoft10KeyData();
	PMessageArea::procUtility( option );
}
//------------------------------------------------------------------------------.
/**
 *	@brief	A070800400 確認コピー停止中_設定変更_メニュー : ユーザー補助
 */
//------------------------------------------------------------------------------.
void PCPC_SampleCopySetChangeMenu_act::procAccessibility( uchar option )
{
	checkSoft10KeyData();
	PMessageArea::procAccessibility( option );
}
//------------------------------------------------------------------------------.
/**
 *	@brief	A070800400 確認コピー停止中_設定変更_メニュー : IDキー処理
 */
//------------------------------------------------------------------------------.
void PCPC_SampleCopySetChangeMenu_act::procIDInputVoice( uchar option )
{
	setBuzzerType( PCGE_BuzzerType_InputNg );
}

//関数--------------------------------------------------------------------------.
//概要    ：ソフトテンキー呼出しキー.
//備考    ：.
//戻値    ：.
//作成者  ：Sky_西原
//レビュー：.
//変更履歴：.
//------------------------------------------------------------------------------.
/**
* @brief A070800400 確認コピー停止中_設定変更_メニュー : ソフトテンキー呼出しキー.
*/
//------------------------------------------------------------------------------.
void PCPC_SampleCopySetChangeMenu_act::procSoft10Key( uchar option )
{
// Comment 2019/07/16 【IT6_EagleH】【HCL_Prabakaran】OP_BTS-8347 jobSpec Type has been changed for common use.
//	APIC_Job* pc_Job = PnlComGetJobCopy();
	APIC_SoftDipSw* pc_SoftDipSw = APIC_SoftDipSw::newInstance();
	int32_t i32_MaxNumOfCopiesAtAdminMachineVendor = PnlComGetCopyJobSetting()->getMaxNumOfCopiesAtAdminMachineVendor();
// Comment 2019/07/16 【IT6_EagleH】【HCL_Prabakaran】OP_BTS-8347 jobSpec Type has been changed for common use.
//	if( NULL == pc_Job ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );	return; }	// NULLチェックを追加.

	if( True == comPanelJob->getJobSpec()->pc_Common->getMemoryCopy()		// メモリコピーである、OHP合紙である場合、テンキー受付禁止.
// Comment 2019/07/16 【IT6_EagleH】【HCL_Prabakaran】OP_BTS-8347 jobSpec Type has been changed for common use.
//	    && True == isOn( CNK_OHP, pc_Job->getJobSpec() ) ) {
	    && True == isOn( CNK_OHP, comPanelJob->getJobSpec() ) ) {
		return;		//Return処理：.
	}


	// スライドレス、MTUI有効の場合.
	if( True == PLBC_NonSlideMultiTouchFunction::isEnable() ) {
		// ソフトテンキー表示(センター表示).
		PnlComMtuiIf_TenkeyPad::instance()->showDefaultPos();
	} else {
		if( PNLE_Soft10KeyUnselected != PNLC_OutSideKeyCallDataLib::newInstance()->getSelectNo() ) { return; }

		PNLC_OutSideKeyCallDataLib::newInstance()->setColorMode( PNLE_Soft10KeyColorModeCommon );
		PNLC_OutSideKeyCallDataLib::newInstance()->setSelectNo( PNLE_Soft10KeySelected );
		PCGC_JobSpecCopyBackup* pc_JobSpecCopyBackup = PnlComGetJobSpecCopyBack();
		if( NULL == pc_JobSpecCopyBackup ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }
		Bool bl_DirectInput = pc_JobSpecCopyBackup->b_SampleStopMenuCopyDirectInput;		//  直接入力フラグを取得.
		int32_t i32_NumOfCopy = comPanelJob->getJobStatus()->getNumOfCopies();
		float f_NumMax;
		if( True == comPanelJob->getJobSpec()->pc_Common->getMemoryCopy()
		    && True == comPanelJob->getJobStatus()->getConfirmPrintStopState()
		    && False == comPanelJob->getJobSpec()->pc_Common->c_JobType.getSingleStepOn() ) {
			// メモリコピー停止中のため、先だし分は無い.
			// 確認コピーではない、メモリコピージョブである、メモリコピー停止中の場合.
			if( True == pc_SoftDipSw->getSoftDipSwBitForPanelSpecNo( 130, 0 ) ) {	// DipSWNo.130 bit0.
				f_NumMax = ( float )( i32_MaxNumOfCopiesAtAdminMachineVendor );
			} else {
				f_NumMax = PCGD_MaxCopyNumber;
			}
		}
		else {
			// プリント部数制限ありの場合.
			if( True == isLimitedPrintNumberOfCopies() ) {
				// コピー部数制限を考慮した最大コピー部数取得(先出しがある場合は最大部数から先出しの「1部」を除く).
				// -1することで最大値が0となるケースは発生しない(2部以上を設定して確認コピーした場合しか、本画面へは遷移しない為).
				f_NumMax = ( float )( getMaxPrintNumberOfCopies() - PCGD_DefaultCopyNumber );
			} else if( True == pc_SoftDipSw->getSoftDipSwBitForPanelSpecNo( 130, 0 ) ) {	// DipSWNo.130 bit0.
				f_NumMax = ( float )( i32_MaxNumOfCopiesAtAdminMachineVendor - PCGD_DefaultCopyNumber );
			} else {
				// 先出しがある場合は最大部数から先出しの「1部」を除く.
				f_NumMax = PCGD_MaxCopyNumber - PCGD_DefaultCopyNumber;
			}
		}
		PNLC_OutSideKeyCallDataLib::newInstance()->setInputNumeric( static_cast<float>( i32_NumOfCopy ), bl_DirectInput, 1, f_NumMax, bl_DirectInput );
		PNLC_PopupListControl::newInstance()->addPopup( &gp_Soft10Key_NumOfCopies, this );
	}
}

//関数--------------------------------------------------------------------------.
//概要    ：ソフトテンキー呼出しキー.
//備考    ：.
//戻値    ：.
//作成者  ：Sky_西原
//レビュー：.
//変更履歴：.
//------------------------------------------------------------------------------.
/**
* @brief A070800400 確認コピー停止中_設定変更_メニュー : アクティブ状態の通知.
*/
//------------------------------------------------------------------------------.
void PCPC_SampleCopySetChangeMenu_act::procActivate()
{
	checkSoft10KeyData();

	// 親クラスを呼び出し.
	PPopupDecisionButtonCom::procActivate();
	Key_SampleCopySetChangeMenu_actNumberOfCopiesInput.showEnableShow( True );
}

//関数--------------------------------------------------------------------------.
//概要    ：ソフトテンキー呼出しキー.
//備考    ：.
//戻値    ：.
//作成者  ：Sky_西原
//レビュー：.
//変更履歴：.
//------------------------------------------------------------------------------.
/**
 *	@brief	A070800400 確認コピー停止中_設定変更_メニュー : ソフトテンキーデータチェック.
 */
//------------------------------------------------------------------------------.
void PCPC_SampleCopySetChangeMenu_act::checkSoft10KeyData()
{
	char sc_No = PNLC_OutSideKeyCallDataLib::newInstance()->getSelectNo();
	if( PNLE_Soft10KeyUnselected != sc_No ) {
		float f_Num;
		// 入力値を取得して更新.
		Bool bl_NonSetting = PNLC_OutSideKeyCallDataLib::newInstance()->getInputNumeric( &f_Num );
		if( False == bl_NonSetting ) {
			comPanelJob->getJobStatus()->setNumOfCopies( static_cast<int32_t>( f_Num ) );		//  算出された部数を設定（システム側の処理に合わせてStatsuに設定）.
			if( True == comPanelJob->getJobSpec()->pc_Common->getMemoryCopy()		// .
			    && True == comPanelJob->getJobStatus()->getConfirmPrintStopState()
			    && False == comPanelJob->getJobSpec()->pc_Common->c_JobType.getSingleStepOn() ) {
				// メモリコピー停止中のため、先だし分は無い.
				// 確認コピーではない、メモリコピージョブである、メモリコピー停止中の場合.
				comPanelJob->getJobSpec()->pc_Print->setNumOfCopies( static_cast<int32_t>( f_Num ) );		//  算出された部数を設定（読み込み中画面やジョブ履歴等パネルに合わせて設定）.
			} else {
				// 算出された部数を設定(先だし分「1部」を加算).
				comPanelJob->getJobSpec()->pc_Print->setNumOfCopies( static_cast<int32_t>( f_Num + PCGD_DefaultCopyNumber ) );
			}
			PCGC_JobSpecCopyBackup* pc_JobSpecCopyBackup = PnlComGetJobSpecCopyBack();
			if( NULL == pc_JobSpecCopyBackup ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }
			pc_JobSpecCopyBackup->b_SampleStopMenuCopyDirectInput = False;
		}
		// データのクリア.
		PNLC_OutSideKeyCallDataLib::newInstance()->setSelectNo( PNLE_Soft10KeyUnselected );
	} else {
		// 0時は、ソフトテンキーは無効.
	}
}

/**
 * @brief	MTUIテンキーポップアップ表示処理.
 * @param	.
 */
void PCPC_SampleCopySetChangeMenu_act::show10KeyPopup()
{
// Comment 2019/07/16 【IT6_EagleH】【HCL_Prabakaran】OP_BTS-8347 jobSpec Type has been changed for common use.
//	APIC_Job* pc_Job = PnlComGetJobCopy();
//	if( NULL == pc_Job ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );	return; }	// NULLチェックを追加.

	if( True != PLBC_NonSlideMultiTouchFunction::isEnable() ) {
		if( True == PLBC_FuncVersionCheck::isOK_Type4() ) {
			procSoft10Key( 0 );
		} else {
			setBuzzerType( PCGE_BuzzerType_Off, True );		// ブザー鳴らさない.
		}
	} else {
		if( True == comPanelJob->getJobSpec()->pc_Common->getMemoryCopy()		// メモリコピーである、OHP合紙である場合、テンキー受付禁止.
// Comment 2019/07/16 【IT6_EagleH】【HCL_Prabakaran】OP_BTS-8347 jobSpec Type has been changed for common use.
//		    && True == isOn( CNK_OHP, pc_Job->getJobSpec() ) ) {
			&& True == isOn( CNK_OHP, comPanelJob->getJobSpec() ) ) {
			setBuzzerType( PCGE_BuzzerType_InputNg );
		} else {
			// MTUIテンキー表示.
			PnlComMtuiIf_TenkeyPad::instance()->show();
		}
	}
}

// ===============================================================================================.
//	概要	: スライドメニュー
//  引数    : オプション.
//	戻り値	: なし.
// ===============================================================================================.
void PCPC_SampleCopySetChangeMenu_act::procSlideMenu( uchar option )
{
	// 10キー表示可否.
	Bool bl_Show10Key = IsShowSlide10Key();
	PNLC_SlideMenuControlLib::newInstance()->setTenKey( bl_Show10Key );
// comment 2015/10/09 OPS1 Sky溝部 [操作性改善] テンキー改善対応.
	PNLC_SlideMenuControlLib::newInstance()->setTenKeyPattern( PMTE_MtuiIf_TenkeyPattern_Normal );

	if( True == bl_Show10Key ) {
		// 10キー入力表示情報設定.
		sendSlide10KeyData( True );
	}

	// スライドメニュー表示.
	PBasicMessageProc::procSlideMenu( option );
}

// ===============================================================================================.
//	概要	: スライドメニューテンキー表示可否.
//  引数    : なし.
//	戻り値	: True：表示可能、False：表示不可.
// ===============================================================================================.
Bool PCPC_SampleCopySetChangeMenu_act::IsShowSlide10Key()
{
// Comment 2019/07/16 【IT6_EagleH】【HCL_Prabakaran】OP_BTS-8347 jobSpec Type has been changed for common use.
//	APIC_Job* pc_Job = PnlComGetJobCopy();
//	if( NULL == pc_Job ) {
//		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
//		return False;
//	}

	if( ( True == comPanelJob->getJobSpec()->pc_Common->getMemoryCopy() )		// メモリコピーである、OHP合紙である場合、テンキー受付禁止.
// Comment 2019/07/16 【IT6_EagleH】【HCL_Prabakaran】OP_BTS-8347 jobSpec Type has been changed for common use.
//	    &&	( True == isOn( CNK_OHP, pc_Job->getJobSpec() ) ) ) {
	    &&	( True == isOn( CNK_OHP, comPanelJob->getJobSpec() ) ) ) {
		return False;
	}

	return True;
}

// ===============================================================================================.
//	概要	: スライドメニュー入力エリアデータ送信.
//  引数    : True：初期化あり、False：初期化なし.
//	戻り値	: なし.
// ===============================================================================================.
void PCPC_SampleCopySetChangeMenu_act::sendSlide10KeyData( Bool bl_Flag )
{
// commnet 2015/04/06 OPS1-Sky西原 [Helios][スライドメニュードラッグ対応].
//	float f_Num = 0;			//現在の値.
	int32_t i32_Num = 0;		//現在の値.
//	Bool bl_ClearOn = False;	//クリアしているかどうかのフラグ.
	uchar puc_Dst[PNLD_SLIDEMENU_TEXTBUFFER_SIZE];				//整数部(/小数)変換後格納先.
//	uchar puc_Dst_fraction[PNLD_SLIDEMENU_TEXTBUFFER_SIZE];		//分数部変換後格納先.
	memset( puc_Dst,0,sizeof( puc_Dst ) );
//	memset(puc_Dst_fraction,0,sizeof(puc_Dst_fraction));
	PNLD_CURSOR_TYPE e_Cursor_type = PNLD_CURSOR_TYPE_OFF;
	Bool bl_Blink = False;

	if( True == bl_Flag ) {
		//スライドメニューOpen時の処理.
		PNLC_SoftSlideMenuTenKeyTextDispLib::newInstance()->create( PNLE_OutsideKeyCall_RIGHT_C_NumOfCopies );
// commnet 2015/04/06 OPS1-Sky西原 [Helios][スライドメニュードラッグ対応].
		PNLC_SoftSlideMenuTenKeyConvLib::newInstance()->setParam( PnlComGetUnitSystem(), "%4d", NULL );
	}

// commnet 2015/04/06 OPS1-Sky西原 [Helios][スライドメニュードラッグ対応].
//	f_Num = comPanelJob->getJobStatus()->getNumOfCopies();			// 現在の値を取得.
	i32_Num = comPanelJob->getJobStatus()->getNumOfCopies();		// 現在の値を取得.
	//入力数値を表示文字列に変更.
//	PNLC_SoftSlideMenuTenKeyConvLib::newInstance()->getSetText(puc_Dst, puc_Dst_fraction, f_Num, bl_ClearOn, -1 );
	PNLC_SoftSlideMenuTenKeyConvLib::newInstance()->getSetText(puc_Dst, i32_Num, True, False );

	PNLC_SoftSlideMenuTenKeyTextDispLib::newInstance()->update( puc_Dst,					//文字列.
	        e_Cursor_type,			//カーソルタイプ.
	        ( short )strlen( ( const char* )puc_Dst ),	//カーソルポジション.
	        bl_Blink );				//ブリンク.
}


//クラス------------------------------------------------------------------------
//概要    ：画面開始処理.
//備考    ：.
//作成者  ：AWS_グライザ.
//レビュー：
//変更履歴：.
//------------------------------------------------------------------------------
/**
 * @brief PCPC_SampleCopySetChangeMenu_actPullDown.
 */
//------------------------------------------------------------------------------
void PCPC_SampleCopySetChangeMenu_actPullDown::startGamen()
{
	PPopupPullDownCom::startGamen();		// 親クラスの画面開始処理.
	uchar u_selectItem = setupSelectedPrintType();
	setSelectMenuNo( u_selectItem );
}
// comment 2020/07/14 SCD2-Sky 石原 [IT6_2.0MR2][OP_BTS-25362] フッターエリア対応.
//クラス------------------------------------------------------------------------
//概要    ：表示部品のセットアップ.
//備考    ：.
//作成者  ：Sky石原.
//変更履歴：.
//------------------------------------------------------------------------------
/**
 * @brief PCPC_SampleCopySetChangeMenu_actPullDown.
 */
//------------------------------------------------------------------------------
void PCPC_SampleCopySetChangeMenu_actPullDown::setupGamen( short option )
{
	PPopupPullDownCom::setupGamen( option );
#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus)  && defined (DEF_LCD_7INCH) ) )
	PnlComFooterPartsBtnActiveChange(homeKeyActiveCheck(), idKeyActiveCheck(), resetKeyActiveCheck());
#endif		//  ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus)  && defined (DEF_LCD_7INCH) ) )	.
}

//クラス------------------------------------------------------------------------
//概要    ：画面開始処理.
//備考    ：.
//作成者  ：AWS_グライザ.
//レビュー：
//変更履歴：.
//------------------------------------------------------------------------------
/**
 * @brief PCPC_SampleCopySetChangeMenu_actPullDown.
 */
//------------------------------------------------------------------------------
void PCPC_SampleCopySetChangeMenu_actPullDown::procButtonOn( uchar option )
{
	TYPC_Scan* pc_Scan = comPanelJob->getJobSpec()->pc_Scan;
	if( NULL == pc_Scan ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }
	Bool b_OrignalDupMode = pc_Scan->getDuplex();
	switch( option ) {
		case PNLD_BTNID_SampleCopySetChangeMenu_actPrintSimplex:
			setPJFaceSelect( b_OrignalDupMode, TYPE_DuplexModeOff );		// 印刷片面.
			break;
		case PNLD_BTNID_SampleCopySetChangeMenu_actPrintDuplex:
			setPJFaceSelect( b_OrignalDupMode, TYPE_DuplexModeOn );			// 印刷両面.
			break;
		default:	LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );	break;
	}
	uchar u_selectItem = setupSelectedPrintType();
	setSelectMenuNo( u_selectItem );
	PNLC_PopupListControl::newInstance()->closeTop( False );		// 普通にポップアップ終了させる.
}

//クラス------------------------------------------------------------------------
//概要    ：画面開始処理.
//備考    ：.
//作成者  ：AWS_グライザ.
//レビュー：
//変更履歴：.
//------------------------------------------------------------------------------
/**
 * @brief PCPC_SampleCopySetChangeMenu_actPullDown.
 */
//------------------------------------------------------------------------------
uchar PCPC_SampleCopySetChangeMenu_actPullDown::setupSelectedPrintType()
{
	// アイテム数1の場合、強制的に1行目選択.
	if( m_ucMaxItem == 1 ) {
		Key_SampleCopySetChangeMenu_actPrint_OneSide.setSelectNo( 1 );
		Key_SampleCopySetChangeMenu_actPrint_TwoSide.setSelectNo( 1 );
		return 1;
	}
	uchar u_selectItem = PNLD_PullDownProgramJobSetChangeMenu_actPrintOneSide + 1;
// Comment 2019/07/16 【IT6_EagleH】【HCL_Prabakaran】OP_BTS-8347 jobSpec Type has been changed for common use.
//	if( False == isOn( CNK_1SideCopy, PnlComGetJobCopy() ) ) {
	if( False == isOn( CNK_1SideCopy, comPanelJob ) ) {
		u_selectItem = PNLD_PullDownProgramJobSetChangeMenu_actPrintTwoSide + 1;
	}
	return u_selectItem;
}

//関数------------------------------------------------------------------------------------------------------------------
//概要    ：印刷キーの処理をする.
//備考    ：.
//戻値    ：.
//作成者  ：AWS_ニコル.
//レビュー：.
//変更履歴：.
//------------------------------------------------------------------------------
/**
* @brief A070800800 プログラムジョブ停止中_設定変更メニュー全原稿 : 印刷キーの処理をする
*/
//------------------------------------------------------------------------------
void PCPC_ProgramJobAllSetChangeMenu_act::displayPrintItem()
{
	ushort us_Text;
	uchar  us_TextFldIndex = 2;
	TYPC_JobSpec* pc_Spec = comPanelJob->getJobSpec();
// Comment 2019/07/16 【IT6_EagleH】【HCL_Prabakaran】OP_BTS-8347 jobSpec Type has been changed for common use.	
//	APIC_Job* pc_Job = PnlComGetJobCopy( sc_JobPutNo );
	APIC_Job* pc_Job = comPanelJob;
	if( NULL == pc_Job ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );	return; }	// NULLチェックを追加.
	TYPC_Common* pc_Common = pc_Job->getJobSpec()->pc_Common;

	TYPC_Print* pc_Print = comPanelJob->getJobSpec()->pc_Print;
	TYPC_Point<float> c_PointFront = pc_Print->c_ImageOffset.getFrontOffset();	// 画像シフトの設定状態を取得.
	TYPC_Point<float> c_PointRear = pc_Print->c_ImageOffset.getRearOffset();

#ifdef DEF_LCD_7INCH
	const ushort PNLD_ITM_OFF_ID = s_pITMOFF_W152H018A;
#else // #ifdef DEF_LCD_7INCH.
	const ushort PNLD_ITM_OFF_ID = s_pITMOFF_W144H016A;
#endif // #ifdef DEF_LCD_7INCH.

	us_Text = isOn( CNK_StampPage, pc_Spec ) ? s_pKEYON_W040H032A : PNLD_ITM_OFF_ID;
	Key_ProgramJobAllSetChangeMenu_act_PageNumber.changeText( us_Text, us_TextFldIndex );
	us_Text = isOn( CNK_StampDate, pc_Spec ) ? s_pKEYON_W040H032A : PNLD_ITM_OFF_ID;
	Key_ProgramJobAllSetChangeMenu_act_DateTime.changeText( us_Text, us_TextFldIndex );
	us_Text = isOn( CNK_Watermark, pc_Spec ) ? s_pKEYON_W040H032A : PNLD_ITM_OFF_ID;
	Key_ProgramJobAllSetChangeMenu_act_StampRepeat.changeText( us_Text, us_TextFldIndex );
	us_Text = isOn( CNK_StampFix, pc_Spec ) ? s_pKEYON_W040H032A : PNLD_ITM_OFF_ID;
	Key_ProgramJobAllSetChangeMenu_act_Stamp.changeText( us_Text, us_TextFldIndex );

	us_Text = pc_Common->c_JimonPattern.getOn() ? s_pKEYON_W040H032A : PNLD_ITM_OFF_ID;
	Key_ProgramJobAllSetChangeMenu_act_CopyProtect.changeText( us_Text, us_TextFldIndex );
	us_Text = pc_Common->getJimonPattern2()->getCopyGuardOn() ? s_pKEYON_W040H032A : PNLD_ITM_OFF_ID;
	Key_ProgramJobAllSetChangeMenu_act_CopyGuard.changeText( us_Text, us_TextFldIndex );
	us_Text =  isOn( CNK_PassWordCopy, pc_Spec ) ? s_pKEYON_W040H032A : PNLD_ITM_OFF_ID;
	Key_ProgramJobAllSetChangeMenu_act_PasswordCopy.changeText( us_Text, us_TextFldIndex );
	us_Text = isOn( CNK_HeaderFooterStamp, pc_Spec ) ? s_pKEYON_W040H032A : PNLD_ITM_OFF_ID;
	Key_ProgramJobAllSetChangeMenu_act_HeaderFooter.changeText( us_Text, us_TextFldIndex );
	
// Comment 2019/07/16 【IT6_EagleH】【HCL_Prabakaran】OP_BTS-8347 jobSpec Type has been changed for common use.
//	us_Text = isOn( CNK_CoverMode, PnlComGetJobCopy() ) ? s_pKEYON_W040H032A : PNLD_ITM_OFF_ID;
	us_Text = isOn( CNK_CoverMode, comPanelJob ) ? s_pKEYON_W040H032A : PNLD_ITM_OFF_ID;
	Key_ProgramJobAllSetChangeMenu_act_CoverSheet.changeText( us_Text, us_TextFldIndex );
	if( ( c_PointFront.x != 0.0f )
	    || ( c_PointFront.y != 0.0f )
	    || ( c_PointRear.x != 0.0f )
	    || ( c_PointRear.y != 0.0f ) ) {
		Key_ProgramJobAllSetChangeMenu_act_ImageShift.changeText( s_pKEYON_W040H032A, us_TextFldIndex );
	} else {
		Key_ProgramJobAllSetChangeMenu_act_ImageShift.changeText( PNLD_ITM_OFF_ID, us_TextFldIndex );
	}

	changeCopyFinishingBadgeView();		// 仕上りキーの表示更新.

	if( True == comPanelJob->getJobSpec()->pc_Common->c_JobType.getSeparateDuplexOn() ) {
		Drp_ProgramJobAllSetChangeMenu_actPrint.setSelectNo( PNLD_ProgramJobAllSetChangeMenu_actPrintEach );
	} else {
		if( True == isOn( CNK_1SideCopy, comPanelJob->getJobSpec() ) ) {
			Drp_ProgramJobAllSetChangeMenu_actPrint.setSelectNo( PNLD_ProgramJobAllSetChangeMenu_actPrintOneSide );
		} else {
			Drp_ProgramJobAllSetChangeMenu_actPrint.setSelectNo( PNLD_ProgramJobAllSetChangeMenu_actPrintDuplex );
		}
	}
}

//関数------------------------------------------------------------------------------------------------------------------
//概要    ：仕上りキーの表示を設定に応じて切り替える.
//作成者  ：OSW1_小尾.
//------------------------------------------------------------------------------
/**
* @brief A070800800 プログラムジョブ停止中_設定変更メニュー全原稿 : 仕上りキーの表示更新
*/
//------------------------------------------------------------------------------
void PCPC_ProgramJobAllSetChangeMenu_act::changeCopyFinishingBadgeView()
{
	PNLS_Parts* ps_parts = NULL;
	ushort text = 0;

	TYPC_Print* pc_Print = comPanelJob->getJobSpec()->pc_Print;

	if( TYPE_StapleModeCorner == pc_Print->c_Finishing.getStapleMode() ||
	    TYPE_StapleModeSide   == pc_Print->c_Finishing.getStapleMode() ) {
		// ステープル判定.
		switch( pc_Print->c_Finishing.getStapleMode() ) {
			case TYPE_StapleModeCorner: // コーナーステープル.
				ps_parts = tbl_i00_116_000_X32_Y32;
// comment 2013.08.27 OPS1-Sky浜野 [次世代アーキ] [7inch化対応].
#ifdef DEF_LCD_7INCH
// A3A4FUM so_okada 2015.02.12 Start.
#ifdef DEF_A4_Cus
				text = s_pITMStaple_W152H018A;
#else
				text = s_pITMCornerStaple_W152H018A;
#endif //DEF_A4_Cus.
// A3A4FUM so_okada 2015.02.12 End.
#else // #ifdef DEF_LCD_7INCH.
//Comment 2018/03/14 【IT6_Sparrow】【HCL_Rajkumar】制御構成変更_No.222 Message Modification.
#ifdef DEF_A4_Cus
				text = s_pITMStaple_W152H018A;		
#else
				text = s_pITMCornerStaple_W144H016A;
#endif //DEF_A4_Cus.
#endif // #ifdef DEF_LCD_7INCH.
				break;
			case TYPE_StapleModeSide:	// 2点ステープル.
				ps_parts = tbl_i00_117_000_X32_Y32;
#ifdef DEF_LCD_7INCH
				text = s_pITM2PosStaple_W152H018A;
#else // #ifdef DEF_LCD_7INCH.
				text = s_pITM2PosStaple_W144H016A;
#endif // #ifdef DEF_LCD_7INCH.
				break;
			default: break;
		}
	} else if( TYPE_FoldModeOff != pc_Print->c_Finishing.getFoldMode() ) {
		// 紙折り判定.
		ps_parts = tbl_i00_120_000_X32_Y32;
#ifdef DEF_LCD_7INCH
		text = s_pITMZFold_W152H018A;
#else // #ifdef DEF_LCD_7INCH.
		text = s_pITMZFold_W144H016A;
#endif // #ifdef DEF_LCD_7INCH.

		// 紙折り設定値.
		switch( pc_Print->c_Finishing.getFoldMode() ) {
			case TYPE_FoldModeZShaped:
#ifdef DEF_LCD_7INCH
				text = s_pITMZFold_W152H018A;					// Z折り.
#else // #ifdef DEF_LCD_7INCH.
				text = s_pITMZFold_W144H016A;					// Z折り.
#endif // #ifdef DEF_LCD_7INCH.
				break;
			default: break;
		}
	} else if( TYPE_PunchModeNone != pc_Print->c_Finishing.getPunchMode() ) {
		// パンチ判定.
		switch( pc_Print->c_Finishing.getPunchMode() ) {
			case TYPE_PunchMode2:	// 2穴.
				ps_parts = tbl_i00_118_000_X32_Y32;
#ifdef DEF_LCD_7INCH
				text = s_pITM2Punch_W152H018A;
#else // #ifdef DEF_LCD_7INCH.
				text = s_pITM2Punch_W144H016A;
#endif // #ifdef DEF_LCD_7INCH.
				break;
			case TYPE_PunchMode3:	// 3穴.
				ps_parts = tbl_i00_119_000_X32_Y32;
#ifdef DEF_LCD_7INCH
				text = s_pITM3Punch_W152H018A;
#else // #ifdef DEF_LCD_7INCH.
				text = s_pITM3Punch_W144H016A;
#endif // #ifdef DEF_LCD_7INCH.
				break;
			case TYPE_PunchMode4:	// 4穴.
				ps_parts = tbl_i00_461_000_X32_Y32;
#ifdef DEF_LCD_7INCH
				text = s_pITM4Punch_W152H018A;
#else // #ifdef DEF_LCD_7INCH.
				text = s_pITM4Punch_W144H016A;
#endif // #ifdef DEF_LCD_7INCH.
				break;
			default: break;
		}
	} else if( True == pc_Print->c_Finishing.getOffsetOn() ) {
		// 仕分けあり.
		ps_parts = tbl_i00_115_000_X32_Y32;
#ifdef DEF_LCD_7INCH
		text = s_pITM4OffSet_W152H018A;
#else // #ifdef DEF_LCD_7INCH.
		text = s_pITM4OffSet_W144H016A;
#endif // #ifdef DEF_LCD_7INCH.
	} else {
		// 全て設定しない場合.
		// プリンタシステム構成取得.
		APIC_PrinterSystem* pc_PrinterSystem = PnlComGetPrinterSystem();
		// ステープル機能状態取得.
		TYPS_StapleFunction s_StapleFunction = pc_PrinterSystem->getStapleFunction();
		Bool b_ShowStapleOn = ( ( s_StapleFunction.b_Staple2Side || s_StapleFunction.b_Staple2SideCenter || s_StapleFunction.b_Staple1Corner ) ? True :False );

		if( False == b_ShowStapleOn ) {
			// ステープルが機器構成上ない場合は紙折りアイコン.
			ps_parts = tbl_i00_120_000_X32_Y32;
		} else {
			// それ以外はステープルアイコン.
			ps_parts = tbl_i00_416_000_X32_Y32;
		}
#ifdef DEF_LCD_7INCH
		text = s_pITMNotStaple_W152H018A;	// ステープルしない.
#else // #ifdef DEF_LCD_7INCH.
		text = s_pITMNotStaple_W144H016A;	// ステープルしない.
#endif // #ifdef DEF_LCD_7INCH.
	}

	// アイコンのNULLチェック.
	if( 0 != ps_parts ) {
		// アイコンを表示.
		Key_ProgramJobAllSetChangeMenu_act_Finishing.changeIconParts( ps_parts, 0 );
	}

	// テキストのNULLチェック.
	if( 0 != text ) {
		// テキストを表示.
		Key_ProgramJobAllSetChangeMenu_act_Finishing.changeText( text, 2 );
	}
}

//関数------------------------------------------------------------------------------------------------------------------
//概要    ：ヘッダー／フッターが登録されているかどうかチェック.
//備考    ：.
//戻値    ：.
//作成者  ：AWS_ニコル.
//レビュー：.
//変更履歴：.
//------------------------------------------------------------------------------
/**
* @brief A070800800 プログラムジョブ停止中_設定変更メニュー全原稿 : ヘッダー／フッターが登録されているかどうかチェック
*/
//------------------------------------------------------------------------------
Bool PCPC_ProgramJobAllSetChangeMenu_act::isHeaderFooterSet()
{
	// 初期化
	uchar index;
	Bool b_HeaderFooterSet = False;
	TYPC_HeaderFooterStamp _rc_HeaderFooterStamp;
	for( index = 1; PCGD_HeaderFooter >= index; ++index ) {		//ヘッダー/フッター登録確認.
		PnlComGetOperationSetting()->getHeaderFooterStamp( index, _rc_HeaderFooterStamp );		//ヘッダー/フッター登録状態を取得.
		if( ( True == _rc_HeaderFooterStamp.getHeaderOn() )
		    || ( True == _rc_HeaderFooterStamp.getFooterOn() ) ) {		//ヘッダー/フッター登録したか?.
			b_HeaderFooterSet = TRUE;		//ヘッダー/フッター登録設定する.
			break;
		}
	}
	return b_HeaderFooterSet;		//ヘッダー/フッター登録を返す.
}

//関数------------------------------------------------------------------------------------------------------------------
//概要    ：スタートキー.
//備考    ：.
//戻値    ：.
//作成者  ：AWS_ニコル.
//レビュー：.
//変更履歴：.
//------------------------------------------------------------------------------
/**
* @brief A070800800 プログラムジョブ停止中_設定変更メニュー全原稿 : スタートキー
*/
//------------------------------------------------------------------------------
void PCPC_ProgramJobAllSetChangeMenu_act::procStart( uchar option )
{
	checkSoft10KeyData();
	if( False == printStartDisable() ) {		//プリント禁止ではないですか？.
		PPopupAnimationCom::procStart( 0 );
	} else {
		setBuzzerType( PCGE_BuzzerType_InputNg );		//ブザーを鳴らせる.
	}
}

//関数------------------------------------------------------------------------------------------------------------------
//概要    ：ストップ.
//備考    ：.
//戻値    ：.
//作成者  ：AWS_ニコル.
//レビュー：.
//変更履歴：.
//------------------------------------------------------------------------------
/**
* @brief A070800800 プログラムジョブ停止中_設定変更メニュー全原稿 : ストップ
*/
//------------------------------------------------------------------------------
void PCPC_ProgramJobAllSetChangeMenu_act::procStop( uchar option )
{

	PNLC_PopupListControl::newInstance()->addPopup( &gp_ProgramJobDeleteConfirm_act );
}

//関数------------------------------------------------------------------------------------------------------------------
//概要    ：クリア.
//備考    ：.
//戻値    ：.
//作成者  ：AWS_ニコル.
//レビュー：.
//変更履歴：.
//------------------------------------------------------------------------------
/**
* @brief A070800800 プログラムジョブ停止中_設定変更メニュー全原稿 : クリア
*/
//------------------------------------------------------------------------------
void PCPC_ProgramJobAllSetChangeMenu_act::procClear( uchar option )
{
	// 既に部数値が初期値(=1).
	if( ( b_InputDirect == True ) && ( 1 == comPanelJob->getJobStatus()->getNumOfCopies() ) ) {
		// 報知音設定.
		setBuzzerType( PCGE_BuzzerType_InputNg );
	}
	// 部数値セット.
	comPanelJob->getJobSpec()->pc_Print->setNumOfCopies( 1 );
	b_InputDirect = True;
	setupGamen();
}
//関数------------------------------------------------------------------------------------------------------------------
//概要    ：リセット.
//備考    ：.
//戻値    ：.
//作成者  ：AWS_ニコル.
//レビュー：.
//変更履歴：.
//------------------------------------------------------------------------------
/**
* @brief A070800800 プログラムジョブ停止中_設定変更メニュー全原稿 : リセット
*/
//------------------------------------------------------------------------------
void PCPC_ProgramJobAllSetChangeMenu_act::procReset( uchar option )
{
	PNLC_PopupListControl::newInstance()->addPopup( &gp_ProgramJobDeleteConfirm_act );		//プログラムジョブ破棄確認画面に遷移する.
}
// comment 2020/07/14 SCD2-Sky 石原 [IT6_2.0MR2][OP_BTS-25362] フッターエリア対応.
//関数------------------------------------------------------------------------------------------------------------------
//概要    ：リセット.
//備考    ：.
//戻値    ：.
//作成者  ：Sky石原.
//レビュー：.
//変更履歴：.
//------------------------------------------------------------------------------
/**
* @brief A070800800 プログラムジョブ停止中_設定変更メニュー全原稿 : リセットキー活性状態.
*/
//------------------------------------------------------------------------------
Bool PCPC_ProgramJobAllSetChangeMenu_act::resetKeyActiveCheck()
{
	ACTIVEKEY_RET( True );
}
//関数------------------------------------------------------------------------------------------------------------------
//概要    ：確認コピー.
//備考    ：.
//戻値    ：.
//作成者  ：AWS_ニコル.
//レビュー：.
//変更履歴：.
//------------------------------------------------------------------------------
/**
* @brief A070800800 プログラムジョブ停止中_設定変更メニュー全原稿 : 確認コピー
*/
//------------------------------------------------------------------------------
void PCPC_ProgramJobAllSetChangeMenu_act::procSampleCopy( uchar option )
{
	checkSoft10KeyData();
	//プリント禁止でない時.
	if( True == comPanelJob->getJobSpec()->pc_Common->c_JobType.getProgramJobOn()
	    && False == printStartDisable() ) {
		//プログラムジョブを開始する.
		if( False == comPanelJob->getJobSpec()->pc_Common->getVerifyPreviewOn() ) {
// comment 2014/11/26 OPS1-Sky門秀隆 [FQ-ISDCS-1400451-OFP152] プログラムジョブを利用して[プレビュー]キーで確認コピーを行うと、結果的に1部余計に印刷される.
//			setJobName( comPanelJob );
//			c_SystemRequest.PJobSamplePrintJobStart();
//			onlyPanel.ep_PanelStatus = EP_PNS_PrintWait;
			// 印刷部数が1の場合、且つ、読み込み完了後の初回の確認コピーキー押下の場合.
			if( ( 1 == comPanelJob->getJobSpec()->pc_Print->getNumOfCopies() )
			 && ( PNLE_ProgramSampleCopyStatus_None == c_JobControl.getProgramSampleCopyStatus() ) ){
				// スタート処理を実行後、ジョブを終了する.
				procStart( 0 );
			}else{
				// 確認コピー実行.
			setJobName( comPanelJob );
			c_SystemRequest.PJobSamplePrintJobStart();
			onlyPanel.ep_PanelStatus = EP_PNS_PrintWait;
				// 初回の確認コピーキー押下の場合.
				if( PNLE_ProgramSampleCopyStatus_None == c_JobControl.getProgramSampleCopyStatus() ){
					// 確認コピー実行状態を"1回実行"に変更.
					c_JobControl.setProgramSampleCopyStatus( PNLE_ProgramSampleCopyStatus_Once );
				}
			}
		} else {
			setBuzzerType( PCGE_BuzzerType_InputNg );		//ブザーを鳴らせる.
		}
	}
}

//関数------------------------------------------------------------------------------------------------------------------
//概要    ：ボタン押下処理.
//備考    ：.
//戻値    ：.
//作成者  ：AWS_ニコル.
//レビュー：.
//変更履歴：.
//------------------------------------------------------------------------------
/**
* @brief A070800800 プログラムジョブ停止中_設定変更メニュー全原稿 : ボタン押下処理
*/
//------------------------------------------------------------------------------
void PCPC_ProgramJobAllSetChangeMenu_act::procButtonOn( uchar option )
{
	short s_xPosition = 0 , s_yPosition = 0 , s_width = 0 , s_height = 0;

	switch( option ) {
		case PNLD_ProgramJobAllSetChangeMenu_act_Finishing:			// 仕上りを表示.
			Key_ProgramJobAllSetChangeMenu_act_Finishing.getPosition( s_xPosition, s_yPosition );
			Key_ProgramJobAllSetChangeMenu_act_Finishing.getSize( s_width, s_height );
			PNLC_PopupListControl::newInstance()->addPopup( &gp_A070800500, this, NULL, s_xPosition, s_yPosition, s_width, s_height );
			break;

		case PNLD_ProgramJobAllSetChangeMenu_act_ImageShift:		// 画像シフトを表示.
			Key_ProgramJobAllSetChangeMenu_act_ImageShift.getPosition( s_xPosition, s_yPosition );
			Key_ProgramJobAllSetChangeMenu_act_ImageShift.getSize( s_width, s_height );
			PNLC_PopupListControl::newInstance()->addPopup( &gp_ProgramJobAllSetChangeShiftEditFront_act, this, NULL, s_xPosition, s_yPosition, s_width, s_height );
			break;

		case PNLD_ProgramJobAllSetChangeMenu_act_CoverSheet:		// カバーシートを表示.
			Key_ProgramJobAllSetChangeMenu_act_CoverSheet.getPosition( s_xPosition, s_yPosition );
			Key_ProgramJobAllSetChangeMenu_act_CoverSheet.getSize( s_width, s_height );
			PNLC_PopupListControl::newInstance()->addPopup( &gp_SampleSetChangePageCover_act, this, NULL, s_xPosition, s_yPosition, s_width, s_height );
			break;

		case PNLD_ProgramJobAllSetChangeMenu_act_DateTime:			// 日付/時刻を表示.
			Key_ProgramJobAllSetChangeMenu_act_DateTime.getPosition( s_xPosition, s_yPosition );
			Key_ProgramJobAllSetChangeMenu_act_DateTime.getSize( s_width, s_height );
			PNLC_PopupListControl::newInstance()->addPopup( &gp_A070202700, this, NULL, s_xPosition, s_yPosition, s_width, s_height );
			break;

		case PNLD_ProgramJobAllSetChangeMenu_act_PageNumber:		// ページ番号を表示.
			Key_ProgramJobAllSetChangeMenu_act_PageNumber.getPosition( s_xPosition, s_yPosition );
			Key_ProgramJobAllSetChangeMenu_act_PageNumber.getSize( s_width, s_height );
			PNLC_PopupListControl::newInstance()->addPopup( &gp_A070202800, this, NULL, s_xPosition, s_yPosition, s_width, s_height );
			break;

		case PNLD_ProgramJobAllSetChangeMenu_act_Stamp:				// スタンプを表示.
			Key_ProgramJobAllSetChangeMenu_act_Stamp.getPosition( s_xPosition, s_yPosition );
			Key_ProgramJobAllSetChangeMenu_act_Stamp.getSize( s_width, s_height );
			PNLC_PopupListControl::newInstance()->addPopup( &gp_A070201700, this, NULL, s_xPosition, s_yPosition, s_width, s_height );
			break;

		case PNLD_ProgramJobAllSetChangeMenu_act_CopyProtect:		// コピープロテクトを表示.
			Key_ProgramJobAllSetChangeMenu_act_CopyProtect.getPosition( s_xPosition, s_yPosition );
			Key_ProgramJobAllSetChangeMenu_act_CopyProtect.getSize( s_width, s_height );
			PNLC_PopupListControl::newInstance()->addPopup( &gp_A070204400, this, NULL, s_xPosition, s_yPosition, s_width, s_height );
			break;

		case PNLD_ProgramJobAllSetChangeMenu_act_CopyGuard:			// コピーガードを表示.
			Key_ProgramJobAllSetChangeMenu_act_CopyGuard.getPosition( s_xPosition, s_yPosition );
			Key_ProgramJobAllSetChangeMenu_act_CopyGuard.getSize( s_width, s_height );
			PNLC_PopupListControl::newInstance()->addPopup( &gp_A070205200, this, NULL, s_xPosition, s_yPosition, s_width, s_height );
			break;

		case PNLD_ProgramJobAllSetChangeMenu_act_PasswordCopy:		// パスワードコピーを表示.
			Key_ProgramJobAllSetChangeMenu_act_PasswordCopy.getPosition( s_xPosition, s_yPosition );
			Key_ProgramJobAllSetChangeMenu_act_PasswordCopy.getSize( s_width, s_height );
			PNLC_PopupListControl::newInstance()->addPopup( &gp_A070201100, this, NULL, s_xPosition, s_yPosition, s_width, s_height );
			break;

		case PNLD_ProgramJobAllSetChangeMenu_act_StampRepeat:		// 繰り返しスタンプを表示.
			Key_ProgramJobAllSetChangeMenu_act_StampRepeat.getPosition( s_xPosition, s_yPosition );
			Key_ProgramJobAllSetChangeMenu_act_StampRepeat.getSize( s_width, s_height );
			PNLC_PopupListControl::newInstance()->addPopup( &gp_A070203100, this, NULL, s_xPosition, s_yPosition, s_width, s_height );
			break;

		case PNLD_ProgramJobAllSetChangeMenu_act_HeaderFooter:		// ヘッダー／フッターを表示.
			Key_ProgramJobAllSetChangeMenu_act_HeaderFooter.getPosition( s_xPosition, s_yPosition );
			Key_ProgramJobAllSetChangeMenu_act_HeaderFooter.getSize( s_width, s_height );
			PNLC_PopupListControl::newInstance()->addPopup( &gp_A070203800, this, NULL, s_xPosition, s_yPosition, s_width, s_height );
			break;

		case PNLD_ProgramJobAllSetChangeMenu_actPrint:				// 印刷プルダウンキー.
		{
			if( p_PrintSetting != NULL )
			{ PNLC_PopupListControl::newInstance()->addPopup( p_PrintSetting, this, &ansPrintSetting ); }
		}
		break;
		case PNLD_ProgramJobAllSetChangeMenu_CopyNum:
			// テンキーポップアップ表示.
			show10KeyPopup();
			break;

		default:
			break;
	}
}

//関数------------------------------------------------------------------------------------------------------------------
//概要    ：実行.
//備考    ：.
//戻値    ：.
//作成者  ：AWS_ニコル.
//レビュー：.
//変更履歴：.
//------------------------------------------------------------------------------
/**
* @brief A070800800 プログラムジョブ停止中_設定変更メニュー全原稿 : 実行
*/
//------------------------------------------------------------------------------
void PCPC_ProgramJobAllSetChangeMenu_act::procExecute( uchar option )
{
	procStart( option );	// スタートキーに処理を統一する.

}

//関数------------------------------------------------------------------------------------------------------------------
//概要    ：中止.
//備考    ：.
//戻値    ：.
//作成者  ：AWS_ニコル.
//レビュー：.
//変更履歴：.
//------------------------------------------------------------------------------
/**
* @brief A070800800 プログラムジョブ停止中_設定変更メニュー全原稿 : 中止
*/
//------------------------------------------------------------------------------
void PCPC_ProgramJobAllSetChangeMenu_act::procCancel( uchar option )
{
	if( False == comPanelJob->getJobSpec()->pc_Common->getVerifyPreviewOn() ) {		//受付け禁止ですか？.
		//全設定を変更前の設定に戻し.
		getSettingChangeSampleCopyPointerCLCD()->PnlRecoverySettingSampleCopySpecInfo( comPanelJob->getJobSpec(), comPanelJob->getJobStatus() );
		PNLC_PopupListControl::newInstance()->addPopup( &gp_ProgramJobDeleteConfirm_act, this, NULL );
	} else {
		//全設定を変更前の設定に戻す.
		getSettingChangeSampleCopyPointerCLCD()->PnlRecoverySettingSampleCopySpecInfo( comPanelJob->getJobSpec(), comPanelJob->getJobStatus() );
		PNLC_PopupListControl::newInstance()->closeTop( False );
	}
}

//関数------------------------------------------------------------------------------------------------------------------
//概要    ：設定終了.
//備考    ：.
//戻値    ：.
//作成者  ：AWS_ニコル.
//レビュー：.
//変更履歴：.
//------------------------------------------------------------------------------
/**
* @brief A070800800 プログラムジョブ停止中_設定変更メニュー全原稿 : 設定終了
*/
//------------------------------------------------------------------------------
void PCPC_ProgramJobAllSetChangeMenu_act::procOk( uchar option )
{
	if( False == comPanelJob->getJobSpec()->pc_Common->getVerifyPreviewOn() ) {		//受付け禁止ですか？.
		if( PNLE_MTUIMODE_V0 <= PLBC_MultiTouchFunction::getMultiTouchMode()
		    && False == comPanelJob->getJobSpec()->pc_Common->c_JobType.getProgramJobOn() ) {
			PNLC_PopupListControl::newInstance()->addPopup( &gp_VerifyPreviewDetailMulti_act );	//仕上りプレビュー 詳細マルチページ.
		} else {
			PNLC_PopupListControl::newInstance()->addPopup( &gp_VerifyPreviewDetail_act );		//仕上りプレビュー詳細.
		}
		PNLC_PopupListControl::newInstance()->closeTop( False );
	} else {
		PNLC_PopupListControl::newInstance()->closeTop( False );
	}
}
//関数------------------------------------------------------------------------------------------------------------------
//概要    ：画面開始処理.
//備考    ：.
//戻値    ：.
//作成者  ：AWS_ニコル.
//レビュー：.
//------------------------------------------------------------------------------
/**
* @brief A070800800 プログラムジョブ停止中_設定変更メニュー全原稿 : 表示部品のセットアップ
*/
//------------------------------------------------------------------------------
void PCPC_ProgramJobAllSetChangeMenu_act::startGamen()
{
	PNLC_OutSideKeyCallDataLib::newInstance()->setSelectNo( PNLE_Soft10KeyUnselected );			// データクリア.

	// 親クラス.
	PPopupAnimationCom::startGamen();
	PCGC_JobSpecCopyBackup* pc_JobSpecCopyBackup = PnlComGetJobSpecCopyBack();
// comment 2014/11/26 OPS1-Sky門秀隆 [FQ-ISDCS-1400451-OFP152] プログラムジョブを利用して[プレビュー]キーで確認コピーを行うと、結果的に1部余計に印刷される.
	int32_t i32_NumOfCopies = 0;
	// 1回目の確認コピー実行後の画面表示の場合.
	if( PNLE_ProgramSampleCopyStatus_Once == c_JobControl.getProgramSampleCopyStatus() ) {
		i32_NumOfCopies = comPanelJob->getJobSpec()->pc_Print->getNumOfCopies();
		if( 1 < i32_NumOfCopies ){
			// 部数を減算する.
			comPanelJob->getJobSpec()->pc_Print->setNumOfCopies( i32_NumOfCopies - 1 );
			// 確認コピー実行状態を"1回完了"に変更.
			c_JobControl.setProgramSampleCopyStatus( PNLE_ProgramSampleCopyStatus_EndOnce );
		}
	}
	if( NULL == pc_JobSpecCopyBackup ){ LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );	return; }	// NULLチェックを追加.//【 静的解析対応 : 仁枝 : 2011.03.02 : CID-288980 】.
	if (True == pc_JobSpecCopyBackup->b_StopJobBackupFlg) {
		TYPC_JobSpec* pc_JobSpec = comPanelJob->getJobSpec();
		TYPC_JobStatus* pc_JobStatus = comPanelJob->getJobStatus();
		// バックアップ.
		c_SettingChangeSampleCopy.PnlBackupSettingSampleCopySpecInfo( pc_JobSpec, pc_JobStatus );
		// 直接入力フラグを初期化する.
		pc_JobSpecCopyBackup->b_SampleStopMenuCopyDirectInput = True;
		// ジョブバックアップ管理フラグを落とす(1回しかバックアップ行わないため).
		pc_JobSpecCopyBackup->b_StopJobBackupFlg = False;
	}
	Key_ProgramJobAllSetChangeMenu_act_NumericInputField.activate( True );
	// 倍率初期設定.
	Key_ProgramJobAllSetChangeMenu_act_NumericInputField.set( &Num_ProgramJobAllSetChangeMenu_actNumberOfCopiesInput, static_cast<float>( comPanelJob->getJobSpec()->pc_Print->getNumOfCopies() ), True );	// 初期表示値設定.

	Cmp_ProgramJobAllSetChangeMenu_actMessageArea.setNowPopupGamen( PNLE_ProgramJobAllSetChangeMenu_act );
	Cmp_ProgramJobAllSetChangeMenu_actMessageArea.setMode( PNLE_PopupMessageAreaMessageModeAdmin );
	Cmp_ProgramJobAllSetChangeMenu_actMessageArea.setOnOffButton( False );
	Cmp_ProgramJobAllSetChangeMenu_actMessageArea.showDestinationCount( False );
	Cmp_ProgramJobAllSetChangeMenu_actMessageArea.changeMessageDisplay( PNLE_PopupMessageAreaMessageShowAll );

	Drp_ProgramJobAllSetChangeMenu_actPrint.setPullDownTxtTable( ( ushort* )us_mPrint_Txt, PNLD_PrintSettings_MaxCnt );
	p_PrintSetting = Drp_ProgramJobAllSetChangeMenu_actPrint.getPullDownMenu();	// プルダウンアドレス取得.
	initPrintInfo();

	// 入力エリア押下機能有無.
	if( True != PLBC_FuncVersionCheck::isOK_Type4() ) {
		Key_ProgramJobAllSetChangeMenu_act_NumericInputField.setShowPush( False );
	}
	Key_ProgramJobAllSetChangeMenu_act_NumericInputField.changeShadow( PNLE_Shadow_none );

	b_InputDirect = True;

// comment 2015/02/04 【HCLT-Karthi】【CQ508222】Setting Message Area width for Arabic Language based on No of Set Key.
	short s_PosX = PNLD_ZeroInit, s_PosY = PNLD_ZeroInit, s_SizeW = PNLD_ZeroInit, s_SizeH = PNLD_ZeroInit;
	
	// Number of Sets button display Position.
	Key_SampleCopySetChangeMenu_actNumberOfCopiesInput.getPosition( s_PosX, s_PosY );
	
	// Message Area Size.
	Cmp_ProgramJobAllSetChangeMenu_actMessageArea.getSize( s_SizeW, s_SizeH );
	
	// Change Message Area size based on Numbe of sets key.
	Cmp_ProgramJobAllSetChangeMenu_actMessageArea.setScalingAreaSize( static_cast<short>( s_PosX - Cmp_ProgramJobAllSetChangeMenu_actMessageArea.getPositionX() ), s_SizeH );
	
	// プレビューキー押下から遷移した場合、ベース画面のstartGamen()でTrueがセットされ、警告画面から戻れなくなるため再度Falseをセットする.
	PNLC_WarningManager::newInstance()->setReqChangeGamen( False );
}
//関数------------------------------------------------------------------------------------------------------------------
//概要    ：表示部品のセットアップ.
//備考    ：.
//戻値    ：.
//作成者  ：AWS_ニコル.
//レビュー：.
//変更履歴：.
//------------------------------------------------------------------------------
/**
* @brief A070800800 プログラムジョブ停止中_設定変更メニュー全原稿 : 表示部品のセットアップ
*/
//------------------------------------------------------------------------------
void PCPC_ProgramJobAllSetChangeMenu_act::setupGamen( short option )
{
	// 親クラス.
	PPopupAnimationCom::setupGamen( option );

	if( ( True == ppc_PrinterSystem->isExistFinisher() )
	    && ( True == comPanelJob->getJobSpec()->pc_Common->c_JobType.getProgramJobOn() ) ) {
		Key_ProgramJobAllSetChangeMenu_act_Finishing.showEnable( True );		//仕上りを表示.
	} else {
		Key_ProgramJobAllSetChangeMenu_act_Finishing.showEnable( False );		//仕上りを非表示.
	}
	short ss_XPos, ss_YPos = 0;
	if( True == comPanelJob->getJobSpec()->pc_Common->getVerifyPreviewOn() ) {		//仕上りプレビューONですか？.
		Key_ProgramJobAllSetChangeMenu_act_Ok.showEnable( True );		//OKキー表示・実行キー非表示.
		Key_ProgramJobAllSetChangeMenu_act_Execution.showEnable( False );
		Key_ProgramJobAllSetChangeMenu_act_Ok.getPosition( ss_XPos, ss_YPos );
#ifdef DEF_LCD_7INCH
		Key_ProgramJobAllSetChangeMenu_act_Cancel.changePosition( static_cast<short>( ss_XPos - 72 ), ss_YPos, 0, 0 );
#else // DEF_LCD_7INCH.
		Key_ProgramJobAllSetChangeMenu_act_Cancel.changePosition( static_cast<short>( ss_XPos - 56 ), ss_YPos, 0, 0 );
#endif // DEF_LCD_7INCH.
		Cmp_ProgramJobAllSetChangeMenu_actMessageArea.changeMainMessageText( s_pMSGCanChangeNumberOfCopies_W448H034A );
	} else {
		Key_ProgramJobAllSetChangeMenu_act_Ok.showEnable( False );		//OKキー非表示・実行キー表示.
		Key_ProgramJobAllSetChangeMenu_act_Execution.showEnable( True );
		Key_ProgramJobAllSetChangeMenu_act_Execution.getPosition( ss_XPos, ss_YPos );
#ifdef DEF_LCD_7INCH
		Key_ProgramJobAllSetChangeMenu_act_Cancel.changePosition( static_cast<short>( ss_XPos - 72 ), ss_YPos, 0, 0 );
#else // DEF_LCD_7INCH.
		Key_ProgramJobAllSetChangeMenu_act_Cancel.changePosition( static_cast<short>( ss_XPos - 56 ), ss_YPos, 0, 0 );
#endif // DEF_LCD_7INCH.
		Cmp_ProgramJobAllSetChangeMenu_actMessageArea.changeMainMessageText( s_pMSGProgramJobAllSetChangeMenu2_W448H034A );
	}

	//現在設定している印刷部数を表示.
	Key_ProgramJobAllSetChangeMenu_act_NumericInputField.set( &Num_ProgramJobAllSetChangeMenu_actNumberOfCopiesInput, static_cast<float>( comPanelJob->getJobSpec()->pc_Print->getNumOfCopies() ), True );

	Txt_ProgramJobAllSetChangeMenu_act_NumberOfCopy.showEnableShow( True );
	displayPrintItem();

	setupPatternCustomize();
	sendSlide10KeyData();
	
// comment 2020/07/14 SCD2-Sky 石原 [IT6_2.0MR2][OP_BTS-25362] フッターエリア対応.
#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus)  && defined (DEF_LCD_7INCH) ) )
	PnlComFooterPartsBtnActiveChange(homeKeyActiveCheck(), idKeyActiveCheck(), resetKeyActiveCheck());
#endif		//  ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus)  && defined (DEF_LCD_7INCH) ) )	.
}

//関数------------------------------------------------------------------------------------------------------------------
//概要    ：プールダウンコールバック.
//備考    ：.
//戻値    ：無し.
//作成者  ：AWS_イギ.
//レビュー：.
//変更履歴：.
//------------------------------------------------------------------------------
void PCPC_ProgramJobAllSetChangeMenu_act::ansPrintSetting( uchar uc_Param, short ss_Param, void* p_Param, uint32_t ul_Param )
{
	switch( uc_Param ) {
		case PNLD_ProgramJobAllSetChangeMenu_actPrintEach:
			comPanelJob->getJobSpec()->pc_Common->c_JobType.setSeparateDuplexOn( True );
			break;
		case PNLD_ProgramJobAllSetChangeMenu_actPrintOneSide:
			comPanelJob->getJobSpec()->pc_Print->setDuplexMode( TYPE_DuplexModeOff );
			comPanelJob->getJobSpec()->pc_Common->c_JobType.setSeparateDuplexOn( False );
			break;
		case PNLD_ProgramJobAllSetChangeMenu_actPrintDuplex:
			comPanelJob->getJobSpec()->pc_Print->setDuplexMode( TYPE_DuplexModeOn );
			comPanelJob->getJobSpec()->pc_Common->c_JobType.setSeparateDuplexOn( False );
			break;
		default:return;
	}
}
//関数------------------------------------------------------------------------------------------------------------------
//概要    ：テンキー.
//備考    ：.
//戻値    ：無し.
//作成者  ：AWS_ニコル.
//レビュー：.
//変更履歴：.
//------------------------------------------------------------------------------
void PCPC_ProgramJobAllSetChangeMenu_act::proc10Key( uchar option )
{
	float us_NowValue;
	float us_NextValue = 0.0f;
	us_NowValue = static_cast<float>( comPanelJob->getJobSpec()->pc_Print->getNumOfCopies() );
	if( True == b_InputDirect ) {

		if( option == 0 )
		{
			setBuzzerType( PCGE_BuzzerType_InputNg );		// 報知音設定.
			return;
		}
		us_NowValue = 0.0f;
		b_InputDirect = False;
	}
	if( ( 0<= option ) && ( option <= 9 ) ) {
		int32_t i32_MaxValue = PCGD_MaxCopyNumber;		// 最大コピー部数:9999部.
		if( True == APIC_SoftDipSw::newInstance()->getSoftDipSwBitForPanelSpecNo( 130, 0 ) ) { 	// DipSWNo.130 bit0.
			i32_MaxValue = PnlComGetCopyJobSetting()->getMaxNumOfCopiesAtAdminMachineVendor();
		}
		us_NextValue = static_cast<float>( proc10KeyInput( static_cast<int32_t>( us_NowValue ), i32_MaxValue, option, False ) );
		comPanelJob->getJobSpec()->pc_Print->setNumOfCopies( static_cast<int32_t>( us_NextValue ) );
		b_InputDirect = False;
	}
	if( us_NowValue == us_NextValue ) {
		setBuzzerType( PCGE_BuzzerType_InputNg );		// 報知音設定.
	}
	setupGamen();
}

//関数------------------------------------------------------------------------------------------------------------------
//概要    ：表示部品のセットアップ.
//備考    ：.
//戻値    ：.
//作成者  ：AWS_ニコル.
//レビュー：.
//変更履歴：.
//------------------------------------------------------------------------------
/**
* @brief A070800800 プログラムジョブ停止中_設定変更メニュー全原稿 : 表示部品のセットアップ
*/
//------------------------------------------------------------------------------
void PCPC_ProgramJobAllSetChangeMenu_act::setupPatternCustomize()
{
	PNLC_FuncPatternSettingLib* cp_FuncSetting = PNLC_FuncPatternSettingLib::newInstance();
	uchar appCnt = 2;
	uchar stampCnt = 8;

	Key_ProgramJobAllSetChangeMenu_act_CoverSheet.showEnable( True );
	Key_ProgramJobAllSetChangeMenu_act_Finishing.showEnable( True );
	Key_ProgramJobAllSetChangeMenu_act_ImageShift.showEnable( True );
	Key_ProgramJobAllSetChangeMenu_act_CoverSheet.showEnable( True );
	Key_ProgramJobAllSetChangeMenu_act_DateTime.showEnable( True );
	Key_ProgramJobAllSetChangeMenu_act_PageNumber.showEnable( True );
	Key_ProgramJobAllSetChangeMenu_act_Stamp.showEnable( True );
	Key_ProgramJobAllSetChangeMenu_act_StampRepeat.showEnable( True );
	Key_ProgramJobAllSetChangeMenu_act_HeaderFooter.showEnable( True );
	Key_ProgramJobAllSetChangeMenu_act_CopyProtect.showEnable( True );
	Key_ProgramJobAllSetChangeMenu_act_CopyGuard.showEnable( True );
	Key_ProgramJobAllSetChangeMenu_act_PasswordCopy.showEnable( True );

	// キーの表示状態取得.
	// 仕上りキー.
	Bool b_OnFinisher = False;
// 【管理No.1433_4_4、制御構成変更_No.335】仕上げモード設定（後処理装置設定モード）Finisher装着時 OPS2 Sky西場 2015/11/05 start.
#ifdef DEF_A4_Cus
	if( ( True == ppc_PrinterSystem->isExistFinisher() )
	    &&( True == comPanelJob->getJobSpec()->pc_Common->c_JobType.getProgramJobOn() )
	    &&( TYPE_TemporaryRescueStatusSet != ppc_SystemStatus->getTemporaryRescueUserStatus(TYPE_TemporaryRescueUnitFinisher) ) )
#else
	if( ( True == ppc_PrinterSystem->isExistFinisher() )
	    &&( True == comPanelJob->getJobSpec()->pc_Common->c_JobType.getProgramJobOn() ) )
#endif
// 【管理No.1433_4_4、制御構成変更_No.335】仕上げモード設定（後処理装置設定モード）Finisher装着時 OPS2 Sky西場 2015/11/05 end.
	{
		b_OnFinisher = True;
	}

	// カバーシートキー.
	Bool b_OnCoverSheet = False;
	if( True == PnlComGetPrinterSystem()->isCoverSheetFeeder() ) {
		b_OnCoverSheet = True;
	}
	// キーカウンター.
	Bool b_OnKeyCounter = False;
	if( ( TYPE_KeyModeKeyCounter != APIC_KeyDevice::getKeyMode() ) ||
	    ( False == APIC_KeyDevice::isKeyCntIFVndMode() ) ) {
		b_OnKeyCounter = True;
	}
	// ヘッダー/フッターキー.
	Bool b_OnHeaderFooter = False;
	if( True == isHeaderFooterSet() ) {
		b_OnHeaderFooter = True;
	}

	// 印刷.
	if( PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_DuplexCombine ) 		== False ||		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
	    PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_DuplexCombine_Duplex )	== False ) {		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
		Drp_ProgramJobAllSetChangeMenu_actPrint.showEnable( False );
	}
	// 仕上り.
	// 紙折り/中折りの判定.
	Bool b_FoldBind = PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_Finishing_FoldBind );		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
	if( True == b_FoldBind ) {
		if( ( False == PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_Finishing_FoldBind_HalfFold ) ) &&		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
		    ( False == PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_Finishing_FoldBind_CenterStapleAndFold ) ) &&		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
		    ( False == PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_Finishing_FoldBind_3Fold ) ) &&		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
		    ( False == PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_Finishing_FoldBind_ZFold ) )		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
		  ) {
			// 紙折り/中折り 無効
			b_FoldBind = False;
		}
	}

	// パンチの判定(現在設定されているパンチのみ判定する).
	// パンチ機能状態取得.
	TYPS_PunchFunction s_PunchFunction = PnlComGetPrinterSystem()->getPunchFunction();
	// 有効パンチ機能テーブル.
	Bool b_AryPunchFunction[4];
	b_AryPunchFunction[0] = s_PunchFunction.b_Punch2Hole;
	b_AryPunchFunction[1] = s_PunchFunction.b_Punch3Hole;
	b_AryPunchFunction[2] = s_PunchFunction.b_Punch4Hole;
	b_AryPunchFunction[3] = s_PunchFunction.b_Punch4HoleSWE;
	const uchar uc_PunchFunctionCnt = ( uchar )( sizeof( b_AryPunchFunction ) / sizeof( Bool ) );
	// パンチ種類.
	uchar uc_AryPunchPanelMenu[] = {
		TYPE_PNLMENU_COPY_Finishing_Punch2,	// 2穴パンチ.
		TYPE_PNLMENU_COPY_Finishing_Punch3,	// 3穴パンチ.
		TYPE_PNLMENU_COPY_Finishing_Punch4,	// 4穴パンチ.
		TYPE_PNLMENU_COPY_Finishing_Punch4,	// 4穴パンチ.
	};
	Bool b_Punch = False;
	uchar i = 0;
	for( i = 0; i < uc_PunchFunctionCnt; i++ )
	{
		if( True == b_AryPunchFunction[i] ) {
			b_Punch = PnlComGetFunctionShowCopy( uc_AryPunchPanelMenu[i] );		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
			if( True == b_Punch ) { break; }
		}
	}

	// ステープル
	Bool b_Staple = True;
	if( False == ppc_PrinterSystem->isExistFinisher() )
	{	// フィニッシャー非装着
		b_Staple = False;
	}
	else
	{
		if( ( PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_Finishing_CornerStaple ) == False ) && // コーナーステープル.		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
		    ( PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_Finishing_2Position ) == False ) )     // 2点ステープル.		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
		{	// ステープル機能が全て無効
			b_Staple = False;
		}
	}

	if( ( PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_Finishing ) == False ) || // 仕上がりがOFF.		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
	    ( // 「位置指定」「排紙トレイ」「仕分け」キーは プログラムジョブからの遷移時、表示なしなので考慮しない.
	        ( b_FoldBind == False ) && // 紙折り/中折り.
	        ( b_Punch == False ) && // パンチ.
	        ( b_Staple == False )   // ステープル.
	    ) || // 仕上り下位機能 OFF
	    ( False == b_OnFinisher ) ) // フィニシャーが無効.
	{
		// 仕上がりキー非表示.
		Key_ProgramJobAllSetChangeMenu_act_Finishing.showEnable( False );
	}
	// 2010/08/31 CQ332746 伊丹Sky三澤 ▲.
// A3A4FUM so_okada 2015.03.20 Start.
#ifdef DEF_A4_Cus
	// ステープル暫定救済中.
	if( TYPE_TemporaryRescueStatusSet == ppc_SystemStatus->getTemporaryRescueUserStatus(TYPE_TemporaryRescueUnitStaple) ){
		Key_ProgramJobAllSetChangeMenu_act_Finishing.showEnable( False );	// 仕上がりキー非表示.
	}
#endif //DEF_A4_Cus.
// A3A4FUM so_okada 2015.03.20 End.
	// 画像シフト.
	if( PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_ImageShift ) == False ) {		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
		Key_ProgramJobAllSetChangeMenu_act_ImageShift.showEnable( False );
		appCnt--;
	}
	// カバーシート.
	if( cp_FuncSetting->isFunctionShowCopyCoverSheet() == False ||
	    False == b_OnCoverSheet ) {
		Key_ProgramJobAllSetChangeMenu_act_CoverSheet.showEnable( False );
		appCnt--;
	}

	// 応用機能オプション「文字スタンプ」の有効/無効.
	Bool bl_AppOptionStamp = ( PNLE_AppOptionStatus_Enable == PnlComGetEnableAppOptionStatus( TYPE_AppOptionStamp ) ) ? True : False;

	// 日付/時刻.
	if( ( False == PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_DateTime ) ) 		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
	    || ( False == bl_AppOptionStamp ) ) {
		Key_ProgramJobAllSetChangeMenu_act_DateTime.showEnable( False );
		stampCnt--;
	}
	// ページ番号.
	if( ( False == PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_PageNumber ) ) 		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
	    || ( False == bl_AppOptionStamp ) ) {
		Key_ProgramJobAllSetChangeMenu_act_PageNumber.showEnable( False );
		stampCnt--;
	}
	// スタンプ.
	if( PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_Stamp ) == False ||		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
	    ( False == bl_AppOptionStamp ) ||
	    False == b_OnKeyCounter ) {
		Key_ProgramJobAllSetChangeMenu_act_Stamp.showEnable( False );
		stampCnt--;
	}
	// 繰り返しスタンプ.
	if( PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_StampRepeat ) == False ||		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
	    ( False == bl_AppOptionStamp ) ||
	    False == b_OnKeyCounter ) {
		Key_ProgramJobAllSetChangeMenu_act_StampRepeat.showEnable( False );
		stampCnt--;
	}
	// ヘッダー/フッター.
	if( PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_HeaderFooter ) == False ||		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
	    ( False == bl_AppOptionStamp ) ||
	    False == b_OnHeaderFooter ||
	    False == b_OnKeyCounter ) {
		Key_ProgramJobAllSetChangeMenu_act_HeaderFooter.showEnable( False );
		stampCnt--;
	}
	// コピープロテクト.
	if( PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_CopyProtect ) == False ||		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
	    ( False == bl_AppOptionStamp ) ||
	    False == b_OnKeyCounter ) {
		Key_ProgramJobAllSetChangeMenu_act_CopyProtect.showEnable( False );
		stampCnt--;
	}
	// コピーガード.
	if( PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_CopyGuard ) == False ||		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
	    ( False == bl_AppOptionStamp ) ||
	    PnlComGetOperationSetting()->getCopyGuardEnable() == False ||
	    ( 0 < comPanelJob->getJobSpec()->pc_Common->getJimonPattern2()->getJimonRead() ) ||		// パスワードコピー原稿検出時.comment MinervaZeus Sky上家 2011/06/10 [CQ339158 パスワードコピー設定時は、「コピーガード」キーは非表示しない].
	    False == b_OnKeyCounter ) {
		Key_ProgramJobAllSetChangeMenu_act_CopyGuard.showEnable( False );
		stampCnt--;
	}
	// パスワードコピー.
	if( PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_PasswordCopy ) == False ||		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
	    ( False == bl_AppOptionStamp ) ||
	    PnlComGetOperationSetting()->getPasswordCopyEnable() == False ||
	    False == b_OnKeyCounter ) {
		Key_ProgramJobAllSetChangeMenu_act_PasswordCopy.showEnable( False );
		stampCnt--;
	}

	// グループ背景の消去
	if( appCnt <= 0 && stampCnt <= 0 ) {
		Bak_ProgramJobAllSetChangeMenu_act_StampPagePrint.showEnable( False );
		Txt_ProgramJobAllSetChangeMenu_act_ApplicationSetting.showEnable( False );
	}
}
//------------------------------------------------------------------------------.
/**
 *	@brief	A070800400 確認コピー停止中_設定変更_メニュー : 暗証番号入力
 */
//------------------------------------------------------------------------------.
void PCPC_ProgramJobAllSetChangeMenu_act::procIDInput( uchar option )
{
	checkSoft10KeyData();
	PPopupAnimationCom::procIDInput( option );
}
// comment 2020/07/14 SCD2-Sky 石原 [IT6_2.0MR2][OP_BTS-25362] フッターエリア対応.
//------------------------------------------------------------------------------.
/**
 *	@brief	A070800400 確認コピー停止中_設定変更_メニュー : IDキー活性状態.
 */
//------------------------------------------------------------------------------.
Bool PCPC_ProgramJobAllSetChangeMenu_act::idKeyActiveCheck()
{
	return PPopupAnimationCom::idKeyActiveCheck();
}
//------------------------------------------------------------------------------.
/**
 *	@brief	A070800400 確認コピー停止中_設定変更_メニュー : コピー
 */
//------------------------------------------------------------------------------.
void PCPC_ProgramJobAllSetChangeMenu_act::procCopy( uchar option )
{
	if( False == c_ExtPanel.isEnableMFPFunction() ) {
		// キーを受け付けない.
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}

	checkSoft10KeyData();
	PPopupAnimationCom::procCopy( option );
}
//------------------------------------------------------------------------------.
/**
 *	@brief	A070800400 確認コピー停止中_設定変更_メニュー : 拡大表示
 */
//------------------------------------------------------------------------------.
void PCPC_ProgramJobAllSetChangeMenu_act::procEnlarge( uchar option )
{
	checkSoft10KeyData();
	PPopupAnimationCom::procEnlarge( option );
}
//------------------------------------------------------------------------------.
/**
 *	@brief	A070800400 確認コピー停止中_設定変更_メニュー : ジョブメモリ
 */
//------------------------------------------------------------------------------.
void PCPC_ProgramJobAllSetChangeMenu_act::procJobMemory( uchar option )
{
	checkSoft10KeyData();
	PPopupAnimationCom::procJobMemory( option );
}
//------------------------------------------------------------------------------.
/**
 *	@brief	A070800400 確認コピー停止中_設定変更_メニュー : ユーティリティ
 */
//------------------------------------------------------------------------------.
void PCPC_ProgramJobAllSetChangeMenu_act::procUtility( uchar option )
{
	checkSoft10KeyData();
	PMessageArea::procUtility( option );
}
//------------------------------------------------------------------------------.
/**
 *	@brief	A070800400 確認コピー停止中_設定変更_メニュー : FAX
 */
//------------------------------------------------------------------------------.
void PCPC_ProgramJobAllSetChangeMenu_act::procFax( uchar option )
{
	if( False == c_ExtPanel.isEnableMFPFunction() ) {
		// キーを受け付けない.
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}

	checkSoft10KeyData();
	PPopupAnimationCom::procFax( option );
}
//------------------------------------------------------------------------------.
/**
 *	@brief	A070800400 確認コピー停止中_設定変更_メニュー : FAX
 */
//------------------------------------------------------------------------------.
void PCPC_ProgramJobAllSetChangeMenu_act::procFaxOnly( uchar option )
{
	// MFP基本機能使用設定が無効の場合.
	if( False == c_ExtPanel.isEnableMFPFunction() ) {
		// キーを受け付けない.
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}

	checkSoft10KeyData();
	PPopupAnimationCom::procFaxOnly( option );
}

//------------------------------------------------------------------------------.
/**
 *	@brief	A070800400 確認コピー停止中_設定変更_メニュー : ボックス
 */
//------------------------------------------------------------------------------.
void PCPC_ProgramJobAllSetChangeMenu_act::procBox( uchar option )
{
	if( False == c_ExtPanel.isEnableMFPFunction() ) {
		// キーを受け付けない.
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}

	checkSoft10KeyData();
	PPopupAnimationCom::procBox( option );
}
//------------------------------------------------------------------------------.
/**
 *	@brief	A070800400 確認コピー停止中_設定変更_メニュー : ユビキタスプリント.
 */
//------------------------------------------------------------------------------.
void PCPC_ProgramJobAllSetChangeMenu_act::procUbqPrint( uchar option )
{
	if( False == c_ExtPanel.isEnableMFPFunction() ) {
		// キーを受け付けない.
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}

	checkSoft10KeyData();
	PPopupAnimationCom::procUbqPrint( option );
}
//------------------------------------------------------------------------------.
/**
 *	@brief	A070800400 確認コピー停止中_設定変更_メニュー : パワーセーブ
 */
//------------------------------------------------------------------------------.
void PCPC_ProgramJobAllSetChangeMenu_act::procPowerSave( uchar option )
{
	checkSoft10KeyData();
	PPopupAnimationCom::procPowerSave( option );
}
//------------------------------------------------------------------------------.
/**
 *	@brief	A070800400 確認コピー停止中_設定変更_メニュー : ヘルプ
 */
//------------------------------------------------------------------------------.
void PCPC_ProgramJobAllSetChangeMenu_act::procHelp( uchar option )
{
	checkSoft10KeyData();
	PPopupAnimationCom::procHelp( option );
}
//------------------------------------------------------------------------------.
/**
 *	@brief	A070800400 確認コピー停止中_設定変更_メニュー : IDキー処理
 */
//------------------------------------------------------------------------------.
void PCPC_ProgramJobAllSetChangeMenu_act::procIDInputVoice( uchar option )
{
	checkSoft10KeyData();
	PPopupAnimationCom::procIDInputVoice( option );
}
//------------------------------------------------------------------------------.
/**
 *	@brief	A070800400 確認コピー停止中_設定変更_メニュー : ブラウザ
 */
//------------------------------------------------------------------------------.
void PCPC_ProgramJobAllSetChangeMenu_act::procBrowser( uchar option )
{
	// MFP基本機能使用設定が無効の場合.
	if( False == c_ExtPanel.isEnableMFPFunction() ) {
		// キーを受け付けない.
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}

	checkSoft10KeyData();
	PPopupAnimationCom::procBrowser( option );
}
//関数--------------------------------------------------------------------------.
//概要    ：ソフトテンキー呼出しキー.
//備考    ：.
//戻値    ：.
//作成者  ：Sky_西原.
//レビュー：.
//変更履歴：.
//------------------------------------------------------------------------------.
/**
* @brief A070800400 確認コピー停止中_設定変更_メニュー : ソフトテンキー呼出しキー.
*/
//------------------------------------------------------------------------------.
void PCPC_ProgramJobAllSetChangeMenu_act::procSoft10Key( uchar option )
{

	// スライドレス、MTUI有効の場合.
	if( True == PLBC_NonSlideMultiTouchFunction::isEnable() ) {
		// ソフトテンキー表示(センター表示).
		PnlComMtuiIf_TenkeyPad::instance()->showDefaultPos();
	} else {
		if( PNLE_Soft10KeyUnselected != PNLC_OutSideKeyCallDataLib::newInstance()->getSelectNo() ) { return; }

		PNLC_OutSideKeyCallDataLib::newInstance()->setColorMode( PNLE_Soft10KeyColorModeCommon );
		PNLC_OutSideKeyCallDataLib::newInstance()->setSelectNo( PNLE_Soft10KeySelected );
// Comment 2019/07/16 【IT6_EagleH】【HCL_Prabakaran】OP_BTS-8347 jobSpec Type has been changed for common use.
//		APIC_Job* pc_Job = PnlComGetJobCopy();
		APIC_Job* pc_Job = comPanelJob;
		if( NULL == pc_Job ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }
		int32_t i32_NumOfCopy = pc_Job->getJobSpec()->pc_Print->getNumOfCopies();

		PNLC_OutSideKeyCallDataLib::newInstance()->setInputNumeric( static_cast<float>( i32_NumOfCopy ), b_InputDirect, static_cast<float>( 1 ), static_cast<float>( PCGD_MaxCopyNumber ), b_InputDirect );
		PNLC_PopupListControl::newInstance()->addPopup( &gp_Soft10Key_NumOfCopies, this );
	}
}

//関数--------------------------------------------------------------------------.
//概要    ：アクティブ状態の通知.
//備考    ：.
//戻値    ：.
//作成者  ：Sky_西原.
//レビュー：.
//変更履歴：.
//------------------------------------------------------------------------------.
/**
* @brief A070800400 確認コピー停止中_設定変更_メニュー : アクティブ状態の通知.
*/
//------------------------------------------------------------------------------.
void PCPC_ProgramJobAllSetChangeMenu_act::procActivate()
{
	checkSoft10KeyData();

	// 親クラスを呼び出し.
	PPopupAnimationCom::procActivate();

	Key_ProgramJobAllSetChangeMenu_act_NumericInputField.showEnableShow( True );
}

//関数--------------------------------------------------------------------------.
//概要    ：ソフトテンキーデータチェック.
//備考    ：.
//戻値    ：.
//作成者  ：Sky_西原.
//レビュー：.
//変更履歴：.
//------------------------------------------------------------------------------.
/**
 *	@brief	A070800400 確認コピー停止中_設定変更_メニュー : ソフトテンキーデータチェック.
 */
//------------------------------------------------------------------------------.
void PCPC_ProgramJobAllSetChangeMenu_act::checkSoft10KeyData()
{
	char sc_No = PNLC_OutSideKeyCallDataLib::newInstance()->getSelectNo();
	if( PNLE_Soft10KeyUnselected != sc_No ) {
		float f_Num;
		// 入力値を取得して更新.
		Bool bl_NonSetting = PNLC_OutSideKeyCallDataLib::newInstance()->getInputNumeric( &f_Num );
		if( False == bl_NonSetting ) {
			comPanelJob->getJobSpec()->pc_Print->setNumOfCopies( static_cast<int32_t>( f_Num ) );
			b_InputDirect = False;
			// データのクリア.
		}
		PNLC_OutSideKeyCallDataLib::newInstance()->setSelectNo( PNLE_Soft10KeyUnselected );
	} else {
		// 0時は、ソフトテンキーは無効.
	}
}


//関数------------------------------------------------------------------------------------------------------------------
//概要    ：.
//備考    ：.
//戻値    ：.
//作成者  ：.
//レビュー：.
//変更履歴：.
//------------------------------------------------------------------------------
/**
* @brief A070800800 印刷ボタン情報の初期化.
*/
//------------------------------------------------------------------------------
void PCPC_ProgramJobAllSetChangeMenu_act::initPrintInfo()
{
	uchar uc_Cnt = 0;
	uchar uc_i   = 0;

	// ベーステーブルからのコピー
	for( int i = 0; i < PNLD_ManuscriptSettings_MaxCnt; i++ ) {
		s_mPrint_Info[i].b_Show         = s_PullDownProgramJobAllSetChangeMenu_Info[i].b_Show;
		s_mPrint_Info[i].uc_ButtonIndex = s_PullDownProgramJobAllSetChangeMenu_Info[i].uc_ButtonIndex;
		s_mPrint_Info[i].e_Kind         = s_PullDownProgramJobAllSetChangeMenu_Info[i].e_Kind;
		s_mPrint_Info[i].us_KindTxt     = s_PullDownProgramJobAllSetChangeMenu_Info[i].us_KindTxt;
	}

	// ボタンのクリアと表示・非表示設定.
	for( uc_i = 0; uc_i < PNLD_PrintSettings_MaxCnt; uc_i++ ) {
		s_PullDownProgramJobAllSetChangeMenu_PullDownButtonInfo[uc_i].pc_PullButton->changeText( s_mPrint_Info[uc_i].us_KindTxt );
		s_PullDownProgramJobAllSetChangeMenu_PullDownButtonInfo[uc_i].pc_PullButton->activate( False );
		s_PullDownProgramJobAllSetChangeMenu_PullDownButtonInfo[uc_i].pc_PullButton->showEnable( False );
		s_mPrint_Info[uc_i].uc_ButtonIndex = 0;
		us_mPrint_Txt[uc_i] = 0;	// 原稿の表示窓用テキスト初期化.

		switch( s_mPrint_Info[uc_i].e_Kind ) {
			case PNLD_AllSettingPrintEach:
			case PNLD_AllSettingPrintOneSide:
			case PNLD_AllSettingPrintTwoSide:
				s_mPrint_Info[uc_i].b_Show = isEnablePrint( s_mPrint_Info[uc_i].e_Kind );
				break;
			default:
				break;
		}
	}

	// 表示可能な原稿を設定.
	for( uc_i = 0; uc_i < PNLD_PrintSettings_MaxCnt; uc_i++ ) {
		if( s_mPrint_Info[uc_i].b_Show ) {
			s_PullDownProgramJobAllSetChangeMenu_PullDownButtonInfo[uc_Cnt].pc_PullButton->changeText( s_mPrint_Info[uc_i].us_KindTxt );
			s_PullDownProgramJobAllSetChangeMenu_PullDownButtonInfo[uc_Cnt].pc_PullButton->activate( True );
			s_PullDownProgramJobAllSetChangeMenu_PullDownButtonInfo[uc_Cnt].pc_PullButton->showEnable( True );
			s_mPrint_Info[uc_i].uc_ButtonIndex = static_cast<uchar>( uc_Cnt + 1 );
			us_mPrint_Txt[uc_Cnt] = s_mPrint_Info[uc_i].us_KindTxt;	// 表示窓用テキストを変更.
			uc_Cnt++;
		}
	}

	// 印刷プルダウンメニューに有効ボタン数を設定する.
	p_PrintSetting->setEnableItemCnt( uc_Cnt );
	// 印刷プルダウンメニューの行数変更.
	p_PrintSetting->changePullDownRow( uc_Cnt );

	// ジョブスペックから原稿設定を取得.
	TYPC_Print* pc_Print = comPanelJob->getJobSpec()->pc_Print;
	char c_DuplexMode = pc_Print->getDuplexMode();
	Bool b_SeparateDuplex = comPanelJob->getJobSpec()->pc_Common->c_JobType.getSeparateDuplexOn();

	pc_Print->setDuplexMode( c_DuplexMode );

	if( b_SeparateDuplex == True )
	{
		uc_mPrintSelectKind = PNLD_AllSettingPrintEach; // 原稿ごとの設定を使用.
	}
	else if( TYPE_DuplexModeOff == c_DuplexMode )
	{
		uc_mPrintSelectKind = PNLD_AllSettingPrintOneSide; // 一括片面.
	}
	else if( TYPE_DuplexModeOn == c_DuplexMode )
	{
		uc_mPrintSelectKind = PNLD_AllSettingPrintTwoSide; // 一括両面.
	}

	for( uc_i = 0; uc_i < PNLD_PrintSettings_MaxCnt; uc_i++ ) {
		if( uc_mPrintSelectKind == s_mPrint_Info[uc_i].e_Kind ) {
			e_mPrintSelectIndex = s_mPrint_Info[uc_i].uc_ButtonIndex;
			break;
		}
	}

	// インデックスが1未満の場合は.
	// 対象がプルダウンメニューから外れた事になるので初期状態に設定する.
	if( e_mPrintSelectIndex < 1 ) {
		e_mPrintSelectIndex = 1;
		uc_mPrintSelectKind = 0;
	}

	// ドロップダウンボタン・プルダウンメニューの設定.
	Drp_ProgramJobAllSetChangeMenu_actPrint.setSelectNo( e_mPrintSelectIndex );
	p_PrintSetting->setSelectMenuNo( e_mPrintSelectIndex );
}

//------------------------------------------------------------------------------
/**
* @brief A070800800 選択したボタンのオプション値を取得.
*/
//------------------------------------------------------------------------------
uchar PCPC_ProgramJobAllSetChangeMenu_act::getPrintKindId( uchar e_index )
{
	uchar uc_i = 0;
	uchar e_retKind = PNLD_AllSettingPrintEach;

	for( uc_i = 0; uc_i < PNLD_PrintSettings_MaxCnt; uc_i++ ) {
		if( s_mPrint_Info[uc_i].uc_ButtonIndex == e_index ) {
			e_retKind = s_mPrint_Info[uc_i].e_Kind;
			break;
		}
	}
	return e_retKind;
}

//------------------------------------------------------------------------------
/**
* @brief A070800800 指定した印刷設定の使用可否.
*/
//------------------------------------------------------------------------------
Bool PCPC_ProgramJobAllSetChangeMenu_act::isEnablePrint( uchar e_Kind )
{
	Bool b_Enable = True;

	if( PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_DuplexCombine )        == False ||   // 両面/ページ集約.		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
	    PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_DuplexCombine_Duplex ) == False ) {  // 原稿 > コピー.		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
		b_Enable = False;
	} else {
		switch( e_Kind ) {
			case PNLD_AllSettingPrintEach: //!< 原稿ごとの設定を使用.
				b_Enable = True;
				break;
			case PNLD_AllSettingPrintOneSide: //!< 一括片面.
				if( PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_DuplexCombine_Duplex_1To1 ) == False &&		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
				    PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_DuplexCombine_Duplex_2To1 ) == False ) {		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
					b_Enable = False;
				}
				break;
			case PNLD_AllSettingPrintTwoSide: //!< 一括両面.
				if( PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_DuplexCombine_Duplex_1To2 ) == False &&		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
				    PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_DuplexCombine_Duplex_2To2 ) == False ) {		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
					b_Enable = False;
				}
				break;
			default:
				b_Enable = False;
				break;
		}
	}
	return b_Enable;
}

/**
 * @brief	MTUIテンキーポップアップ表示処理.
 * @param	.
 */
void PCPC_ProgramJobAllSetChangeMenu_act::show10KeyPopup()
{
	if( True != PLBC_NonSlideMultiTouchFunction::isEnable() ) {
		if( True == PLBC_FuncVersionCheck::isOK_Type4() ) {
			procSoft10Key( 0 );
		} else {
			setBuzzerType( PCGE_BuzzerType_Off, True );		// ブザー鳴らさない.
		}
	} else {
		// MTUIテンキー表示.
// comment 2015/09/16 OPS1 Sky溝部 [操作性改善] テンキー改善対応.
//		PnlComMtuiIf_TenkeyPad::instance()->show();
		PnlComMtuiIf_TenkeyPad::instance()->show( PMTE_MtuiIf_TenkeyPattern_Normal );
	}
}

// ===============================================================================================.
//	概要	: スライドメニュー
//  引数    : オプション.
//	戻り値	: なし.
// ===============================================================================================.
void PCPC_ProgramJobAllSetChangeMenu_act::procSlideMenu( uchar option )
{
	// 10キー表示可否.
// commnet 2015/04/06 OPS1-Sky西原 [Helios][スライドメニュードラッグ対応].
//	Bool bl_Show10Key = True;
	Bool bl_Show10Key = IsShowSlide10Key();
	PNLC_SlideMenuControlLib::newInstance()->setTenKey( bl_Show10Key );
// comment 2015/10/09 OPS1 Sky溝部 [操作性改善] テンキー改善対応.
	PNLC_SlideMenuControlLib::newInstance()->setTenKeyPattern( PMTE_MtuiIf_TenkeyPattern_Normal );

	// 10キー入力表示情報設定.
	if( True == bl_Show10Key ) {
		sendSlide10KeyData( True );
	}

	// スライドメニュー表示.
	PBasicMessageProc::procSlideMenu( option );
}

// ===============================================================================================.
//	概要	: スライドメニューテンキー表示可否.
//  引数    : なし.
//	戻り値	: True：表示可能、False：表示不可.
//	変更履歴: 2015/04/06 OPS1-Sky西原 [Helios][スライドメニュードラッグ対応].
// ===============================================================================================.
Bool PCPC_ProgramJobAllSetChangeMenu_act::IsShowSlide10Key()
{
	return True;
}

// ===============================================================================================.
//	概要	: スライドメニュー入力エリアデータ送信.
//  引数    : True：初期化あり、False：初期化なし.
//	戻り値	: なし.
// ===============================================================================================.
void PCPC_ProgramJobAllSetChangeMenu_act::sendSlide10KeyData( Bool bl_Flag )
{
// commnet 2015/04/06 OPS1-Sky西原 [Helios][スライドメニュードラッグ対応].
//	float f_Num = 0;			//現在の値.
	int32_t i32_Num = 0;		//現在の値.
//	Bool bl_ClearOn = False;	//クリアしているかどうかのフラグ.
	uchar puc_Dst[PNLD_SLIDEMENU_TEXTBUFFER_SIZE];				//整数部(/小数)変換後格納先.
//	uchar puc_Dst_fraction[PNLD_SLIDEMENU_TEXTBUFFER_SIZE];		//分数部変換後格納先.
	memset( puc_Dst,0,sizeof( puc_Dst ) );
//	memset(puc_Dst_fraction,0,sizeof(puc_Dst_fraction));
	PNLD_CURSOR_TYPE e_Cursor_type = PNLD_CURSOR_TYPE_OFF;
	Bool bl_Blink = False;

	if( True == bl_Flag ) {
		//スライドメニューOpen時の処理.
		PNLC_SoftSlideMenuTenKeyTextDispLib::newInstance()->create( PNLE_OutsideKeyCall_RIGHT_C_NumOfCopies );
// commnet 2015/04/06 OPS1-Sky西原 [Helios][スライドメニュードラッグ対応].
		PNLC_SoftSlideMenuTenKeyConvLib::newInstance()->setParam( PnlComGetUnitSystem(), "%4d", NULL );
	}

// commnet 2015/04/06 OPS1-Sky西原 [Helios][スライドメニュードラッグ対応].
//	f_Num = comPanelJob->getJobSpec()->pc_Print->getNumOfCopies();	
	i32_Num = comPanelJob->getJobSpec()->pc_Print->getNumOfCopies();	
	//入力数値を表示文字列に変更.
//	PNLC_SoftSlideMenuTenKeyConvLib::newInstance()->getSetText(puc_Dst, puc_Dst_fraction, f_Num, bl_ClearOn, -1 );
	PNLC_SoftSlideMenuTenKeyConvLib::newInstance()->getSetText(puc_Dst, i32_Num, True, False );

	PNLC_SoftSlideMenuTenKeyTextDispLib::newInstance()->update( puc_Dst,					//文字列.
	        e_Cursor_type,			//カーソルタイプ.
	        ( short )strlen( ( const char* )puc_Dst ),	//カーソルポジション.
	        bl_Blink );				//ブリンク.
}



void PCPC_ActSettingChange_act::procUtility( uchar option )
{
	PMessageArea::procUtility( option );
}
void PCPC_ActSettingChange_act::procAccessibility( uchar option )
{
	PMessageArea::procAccessibility( option );
}


//関数------------------------------------------------------------------------------------------------------------------
//概要    ：ウインドウを閉じ、元の画面へ遷移する.
//備考    ：.
//戻値    ：無し.
//作成者  ：AWS_フェクス.
//レビュー：.
//変更履歴：.
//------------------------------------------------------------------------------
/**
* @brief A070800100 分割読込み停止中／設定変更メニュー : ウインドウを閉じ、元の画面へ遷移する
*/
//------------------------------------------------------------------------------
void PCPC_ActSettingChange_act::procOk()
{
	PNLC_PopupListControl::newInstance()->closeTop( True );		// ウインドウを閉じ、元の画面へ遷移する.
}

//関数------------------------------------------------------------------------------------------------------------------
//概要    ：ボタン押下処理..
//備考    ：.
//戻値    ：無し.
//作成者  ：AWS_フェクス.
//レビュー：.
//変更履歴：.
//------------------------------------------------------------------------------
/**
* @brief A070800100 分割読込み停止中／設定変更メニュー : ボタン押下処理.
*/
//------------------------------------------------------------------------------
void PCPC_ActSettingChange_act::procButtonOn( uchar option )
{
	Bool b_Setup = False;
	short posX, posY, sizeX, sizeY;

	switch( option ) {		// 押下されたボタンに従いCommon_APIへの設定処理を行う.
		case BTNID_KEY_ZOOM_OPTION_ID:		// 倍率設定ボタン.
			if( True == combinationCheck( CNK_ZoomKey, comPanelJob ) ) {
				Key_ActSettingChange_actZoom.getPosition( posX, posY );
				Key_ActSettingChange_actZoom.getSize( sizeX, sizeY );
				PNLC_PopupListControl::newInstance()->addPopup( &gp_A070800140, this, NULL, posX, posY, sizeX, sizeY );
			} else {
				b_Setup = True;
			}
			break;
		case BTNID_KEY_CENTERERASE_OPTION_ID:		// 折り目消し設定ボタン.
			Key_ActSettingChange_actCenterErase.getPosition( posX, posY );
			Key_ActSettingChange_actCenterErase.getSize( sizeX, sizeY );
			PNLC_PopupListControl::newInstance()->addPopup( &gp_A070800130, this, NULL, posX, posY, sizeX, sizeY );
			break;
// Comment VenusMLK -> 2017/03/22 SCD2 3G Sky瀬戸 [AccuRev.1017581]分割読込み停止中の設定変更では原稿サイズ設定を表示させない.
#ifndef DEF_A4_Cus
		case BTNID_KEY_ORIGINALSIZE_OPTION_ID:		// 原稿サイズ設定ボタン.
			Key_ActSettingChange_actOriginalSize.getPosition( posX, posY );
			Key_ActSettingChange_actOriginalSize.getSize( sizeX, sizeY );
			PNLC_PopupListControl::newInstance()->addPopup( &gp_SetChangeOriginalSizeMetric_act, this, NULL, posX, posY, sizeX, sizeY );
			break;
#endif // DEF_A4_Cus.
// Comment VenusMLK <- 2017/03/22 SCD2 3G Sky瀬戸 [AccuRev.1017581]分割読込み停止中の設定変更では原稿サイズ設定を表示させない.
		case BTNID_KEY_STAPLEANDPUNCH_OPTION_ID:		// ステープル/パンチ位置設定ボタン.
			Key_ActSettingChange_actStapleAndPunchPosition.getPosition( posX, posY );
			Key_ActSettingChange_actStapleAndPunchPosition.getSize( sizeX, sizeY );
			PNLC_PopupListControl::newInstance()->addPopup( &gp_A070800300, this, NULL, posX, posY, sizeX, sizeY );
			break;
		case BTNID_KEY_MARGINPOSITION_OPTION_ID:		// 枠消し設定ボタン.
			Key_ActSettingChange_actFrameErase.getPosition( posX, posY );
			Key_ActSettingChange_actFrameErase.getSize( sizeX, sizeY );
			PNLC_PopupListControl::newInstance()->addPopup( &gp_SetChangeCollectEraseFrameInch_act, this, NULL, posX, posY, sizeX, sizeY );
			break;
		// プルダウンメニューオープン.
		case BTNID_KEY_ActSettingChange_actManuscriptCloseAndCarryOutPositionSetting:
			PNLC_PopupListControl::newInstance()->addPopup( p_ManuscriptCloseAndCarryOutPositionSetting, this, ans_BindPositionPullDownEnd );
			break;
		// プルダウンメニューオープン.
		case BTNID_KEY_ActSettingChange_actManuscriptCloseAndCarryOut:
// comment 2019/03/13 Sky飛田 【制御構成変更_No.66】 仕上がりプレビュー対応.
//			PNLC_PopupListControl::newInstance()->addPopup( p_ManuscriptCloseAndCarryOut, this, ans_BindPositionCloseAndCarryOut, 520, 180, 160, 120 );
			PNLC_PopupListControl::newInstance()->addPopup( p_ManuscriptCloseAndCarryOut, this, ans_BindPositionCloseAndCarryOut );
			break;
		case BTNID_KEY_ActSettingChange_actManuscriptSetting:
			PNLC_PopupListControl::newInstance()->addPopup( p_ManuscriptSetting, this, ans_ManuscriptPullDownEnd );
			break;
		default:
			break;
	}

	if( True == b_Setup ) {
		setupGamen();
	}
}

//クラス------------------------------------------------------------------------
//概要    ：画面開始処理.
//備考    ：.
//作成者  ：AWS_グライザ.
//レビュー：
//変更履歴：.
//------------------------------------------------------------------------------
/**
 * @brief PCPC_PullDownActSettingChange_actManuscriptSettings：画面開始処理.
 */
//------------------------------------------------------------------------------
void PCPC_PullDownActSettingChange_actManuscriptSettings::startGamen()
{
	PPopupPullDownCom::startGamen();		// 親クラスの画面開始処理.
	uchar u_selectItem = setupSelectedManuscriptType();
	setSelectMenuNo( u_selectItem );
}
// comment 2020/07/14 SCD2-Sky 石原 [IT6_2.0MR2][OP_BTS-25362] フッターエリア対応.
//クラス------------------------------------------------------------------------
//概要    ：表示部品のセットアップ.
//備考    ：.
//作成者  ：Sky石原.
//変更履歴：.
//------------------------------------------------------------------------------
/**
 * @brief PCPC_PullDownActSettingChange_actManuscriptSettings.
 */
//------------------------------------------------------------------------------
void PCPC_PullDownActSettingChange_actManuscriptSettings::setupGamen( short option )
{
	PPopupPullDownCom::setupGamen( option );
#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus)  && defined (DEF_LCD_7INCH) ) )
	PnlComFooterPartsBtnActiveChange(homeKeyActiveCheck(), idKeyActiveCheck(), resetKeyActiveCheck());
#endif		//  ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus)  && defined (DEF_LCD_7INCH) ) )	.
}

//関数--------------------------------------------------------------------------
//概要    ：ボタン（押された）.
//備考    ：.
//戻値    ：.
//作成者  ：AWS_グライザ.
//レビュー：
//変更履歴：.
//------------------------------------------------------------------------------
/**
 * @brief PCPC_PullDownActSettingChange_actManuscriptSettings：ボタン（押された）.
 */
//------------------------------------------------------------------------------
void PCPC_PullDownActSettingChange_actManuscriptSettings::procButtonOn( uchar option )
{
	TYPC_Print* pc_Print = comPanelJob->getJobSpec()->pc_Print;
	if( NULL == pc_Print ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }
	EP_Combination e_DuplexMode = ( ( pc_Print->getDuplexMode() == TYPE_DuplexModeOff ) ? CNK_1SideCopy: CNK_2SideCopy );
	switch( option ) {
		case PNLD_PullDownProgramJobSetChangeMenu_actManuscriptOneSide:
			combinationCheck( CNK_1SideOrig );
			// 片面or両面プリント組み合わせ禁止チェック.
			combinationCheck( e_DuplexMode );
			comPanelJob->getJobSpec()->pc_Print->setDuplexMode( TYPE_DuplexModeOff );
			comPanelJob->getJobSpec()->pc_Scan->setDuplex( False );
			comPanelJob->getJobSpec()->pc_Scan->setCoverAndDuplex( False );
			break;
		case PNLD_PullDownProgramJobSetChangeMenu_actManuscriptTwoSide:
			comPanelJob->getJobSpec()->pc_Print->setDuplexMode( TYPE_DuplexModeOn );
			comPanelJob->getJobSpec()->pc_Scan->setDuplex( False );
			comPanelJob->getJobSpec()->pc_Scan->setCoverAndDuplex( False );
			break;
		case PNLD_PullDownProgramJobSetChangeMenu_actManuscriptCoverTwoSide:
			comPanelJob->getJobSpec()->pc_Scan->setDuplex( True );
			comPanelJob->getJobSpec()->pc_Scan->setCoverAndDuplex( True );
			break;
		default:	LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );	break;
	}
	uchar u_selectItem = setupSelectedManuscriptType();
	setSelectMenuNo( u_selectItem );
	PNLC_PopupListControl::newInstance()->closeTop( False );		// 普通にポップアップ終了させる.
}

//クラス------------------------------------------------------------------------
//概要    ：選択原稿タイプのセットアップ.
//備考    ：.
//作成者  ：AWS_グライザ.
//レビュー：
//変更履歴：.
//------------------------------------------------------------------------------
/**
 * @brief PCPC_PullDownActSettingChange_actManuscriptSettings：選択原稿タイプのセットアップ.
 */
//------------------------------------------------------------------------------
uchar PCPC_PullDownActSettingChange_actManuscriptSettings::setupSelectedManuscriptType()
{
	TYPC_Scan* pc_Scan = comPanelJob->getJobSpec()->pc_Scan;
	if( NULL == pc_Scan ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return 0; }
	if( m_ucMaxItem == 1 ) {
		Key_ActSettingChange_actManuscriptSetting_OneSide.setSelectNo( 1 );
		Key_ActSettingChange_actManuscriptSetting_TwoSide.setSelectNo( 1 );
		return 1;
	} else if( m_ucMaxItem == 2 ) {
		TYPC_Print* pc_Print = comPanelJob->getJobSpec()->pc_Print;
		uchar sc_select = PNLD_PullDownProgramJobSetChangeMenu_actManuscriptOneSide + 1;
		if( pc_Print->getDuplexMode() == TYPE_DuplexModeOn ) {
			sc_select = PNLD_PullDownProgramJobSetChangeMenu_actManuscriptTwoSide + 1;
		}
		Key_ActSettingChange_actManuscriptSetting_Copy.setSelectNo( sc_select );
		return sc_select;
	}

	TYPC_Print* pc_Print = comPanelJob->getJobSpec()->pc_Print;
	uchar sc_select = PNLD_PullDownProgramJobSetChangeMenu_actManuscriptOneSide + 1;

	if( ( pc_Scan->getDuplex() == False ) && ( pc_Scan->getCoverAndDuplex() == False ) ) {

		if( pc_Print->getDuplexMode() == TYPE_DuplexModeOn ) {
			sc_select = PNLD_PullDownProgramJobSetChangeMenu_actManuscriptTwoSide + 1;
		}
	} else {
		sc_select = PNLD_PullDownProgramJobSetChangeMenu_actManuscriptCoverTwoSide + 1;
	}
	Key_ActSettingChange_actManuscriptSetting.setSelectNo( sc_select );
	return sc_select;
}
//関数------------------------------------------------------------------------------------------------------------------
//概要    ：リセット.
//備考    ：.
//戻値    ：無し.
//作成者  ：AWS_フェクス.
//レビュー：.
//変更履歴：.
//------------------------------------------------------------------------------
/**
* @brief A070800100 分割読込み停止中／設定変更メニュー : リセット
*/
//------------------------------------------------------------------------------
void PCPC_ActSettingChange_act::procReset( uchar option )
{
	c_SettingChengeBasicInfo.PnlRecoverySettingBasicInfo( 1 );  	//設定変更画面内戻る.
	PNLC_PopupListControl::newInstance()->closeTop( False );
}

// comment 2020/07/14 SCD2-Sky 石原 [IT6_2.0MR2][OP_BTS-25362] フッターエリア対応.
//関数------------------------------------------------------------------------------------------------------------------
//概要    ：リセット.
//備考    ：.
//戻値    ：無し.
//作成者  ：Sky石原.
//レビュー：.
//変更履歴：.
//------------------------------------------------------------------------------
/**
* @brief A070800100 分割読込み停止中／設定変更メニュー : リセットキー活性状態.
*/
//------------------------------------------------------------------------------
Bool PCPC_ActSettingChange_act::resetKeyActiveCheck()
{
	ACTIVEKEY_RET( True );
}
//関数------------------------------------------------------------------------------------------------------------------
//概要    ：スタートキー　.
//備考    ：.
//戻値    ：無し.
//作成者  ：AWS_フェクス.
//レビュー：.
//変更履歴：.
//------------------------------------------------------------------------------
/**
* @brief A070800100 分割読込み停止中／設定変更メニュー : スタートキー　
*/
//------------------------------------------------------------------------------
void PCPC_ActSettingChange_act::procStart( uchar option )
{
	if( ( PNLE_APPMODE_SCANFAX == PnlComGetAppMode() ) && ( False == comPanelJob->getJobSpec()->pc_Scan->c_MemoryIn.getMemoryInContinue() ) ) {
//Comment 2019/12/12 SCD2 K.Kasemura  [EagleZ+][OP_BTS-15196] 送信中ジョブが無いにもかかわらず、送信中ジョブの警告が表示される.
//		SP_StatusCheck  sp_StatusCheck = isMachineStatusChack();  // 送信中ステータスのチェック結果.
		if( ( True == PnlComIsEnableSearchablePDFAutoExtrFileNameConfirm( comPanelJob ) )
//		    && ( True == sp_StatusCheck.bl_NowTrans ) ) {
			&& ( True == isFaxJobStatusCheck() ) ) {
			PNLC_PopupListControl::newInstance()->addPopup( &gp_ConfirmContinualOperation_act_warn );
			return;
		}
	}
	PNLC_ActionDataKeyControlLib::newInstance()->procStartActionReadStop( option );
}

//関数------------------------------------------------------------------------------------------------------------------
//概要    ：ストップキー.
//備考    ：.
//戻値    ：無し.
//作成者  ：AWS_フェクス.
//レビュー：.
//変更履歴：.
//------------------------------------------------------------------------------
/**
* @brief A070800100 分割読込み停止中／設定変更メニュー : ストップキー
*/
//------------------------------------------------------------------------------
void PCPC_ActSettingChange_act::procStop( uchar option )
{
	PNLC_ActionDataKeyControlLib::newInstance()->procStopActionReadStop( option );
}

//関数------------------------------------------------------------------------------------------------------------------
//概要    ：中止.
//備考    ：.
//戻値    ：無し.
//作成者  ：AWS_フェクス.
//レビュー：.
//変更履歴：.
//------------------------------------------------------------------------------
/**
* @brief A070800100 分割読込み停止中／設定変更メニュー : 中止
*/
//------------------------------------------------------------------------------
void PCPC_ActSettingChange_act::procCancel( uchar option )
{
	PPopupDecisionButtonCom::procCancel( option );			// 親クラスの画面終了処理.
}

//関数------------------------------------------------------------------------------------------------------------------
//概要    ：画面開始.
//備考    ：.
//戻値    ：無し.
//作成者  ：AWS_フェクス.
//レビュー：.
//変更履歴：.
//------------------------------------------------------------------------------
/**
* @brief A070800100 分割読込み停止中／設定変更メニュー : 画面開始
*/
//------------------------------------------------------------------------------
void PCPC_ActSettingChange_act::startGamen()
{
	PPopupDecisionButtonCom::startGamen();			// 親クラスの画面開始処理.
	Cmp_ActSettingChange_act_MessageArea.setMode( PNLE_PopupMessageAreaMessageModeAdmin );
	Cmp_ActSettingChange_act_MessageArea.setOnOffButton( False );
	Cmp_ActSettingChange_act_MessageArea.showDestinationCount( False );
	Cmp_ActSettingChange_act_MessageArea.setNowPopupGamen( PNLE_UnKnownPopup );	// 自動取得および警告表示非対象な為、UnKnownを仮設定する.

	b_m_actManuscriptCloseAndCarryOut_Show = True;

	uc_mSelectBindDirection = 0;	// 2011/05/13 伊丹Sky平野 CQ.345143 .

	PCGC_VerifyThumbnailPreviewControl* pc_VerifyControl = PnlComGetVerifyPreviewControl();
	if( NULL == pc_VerifyControl )
	{
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return;
	}
// Comment VenusMLK -> 2017/03/22 SCD2 3G Sky瀬戸 [AccuRev.1017581]分割読込み停止中の設定変更では原稿サイズ設定を表示させない.
#ifndef DEF_A4_Cus
	TYPC_Scan* pc_Scan = comPanelJob->getJobSpec()->pc_Scan;
#endif // DEF_A4_Cus.
// Comment VenusMLK <- 2017/03/22 SCD2 3G Sky瀬戸 [AccuRev.1017581]分割読込み停止中の設定変更では原稿サイズ設定を表示させない.
	TYPC_Send*  pc_Send  = comPanelJob->getJobSpec()->pc_Send;
// Comment VenusMLK -> 2017/03/22 SCD2 3G Sky瀬戸 [AccuRev.1017581]分割読込み停止中の設定変更では原稿サイズ設定を表示させない.
#ifndef DEF_A4_Cus
	if( TYPE_OriginalSizeModeMix == pc_Scan->c_OriginalSize.getOriginalSizeMode() ) {		// 原稿サイズモード：混載原稿ですか.
		Key_ActSettingChange_actOriginalSize.showEnable( True );                // 原稿サイズの変更は混載原稿が設定されているときのみ可能.		//「原稿サイズ」キーの表示.
	} else {
		Key_ActSettingChange_actOriginalSize.showEnable( False );		//「原稿サイズ」キーの非表示.
	}
#endif // DEF_A4_Cus.
// Comment VenusMLK <- 2017/03/22 SCD2 3G Sky瀬戸 [AccuRev.1017581]分割読込み停止中の設定変更では原稿サイズ設定を表示させない.
	if( pc_Send->c_ComSend.getWebBrowserScanOn() == True ) {		//「原稿のとじしろ」プルダウンキーの表示/非表示.
		Key_ActSettingChange_actManuscriptCloseAndCarryOut.showEnable( False );		// 原稿とじしろキーを非表示.
		b_m_actManuscriptCloseAndCarryOut_Show = False;
	}
	TYPE_JobType e_JobType = comPanelJob->getJobSpec()->pc_Common->c_JobType.getJobType();
	switch( e_JobType ) {
		case TYPE_JobTypeScan:					// スキャン時.
		case TYPE_JobTypeFaxSendMemory:			// メモリ送信.
		case TYPE_JobTypeFaxSendTimer:			// タイマ通信.
		case TYPE_JobTypeFaxReceivePolling:		// ポーリング送信(掲示板ポーリング送信を含む).
			Key_ActSettingChange_actZoom.showEnable( False );	// 倍率キー非表示.
			break;
		case TYPE_JobTypeCopy:
		case TYPE_JobTypeIntCopy:
// comment 2016.05.20 OPS1-Sky西原 [ZeusSBk][CQ529203].
//			if( comPanelJob->getJobSpec()->pc_Common->c_Mag.getAmsOn() == True ) {	// 自動倍率(AMS)ONの場合、倍率キー非表示.
			if( True == comPanelJob->getJobSpec()->pc_Common->c_Mag.getAmsOn()		// 自動倍率(AMS)ONの場合、倍率キー非表示.
			||  TYPE_Nin1_1in1 != comPanelJob->getJobSpec()->pc_Print->c_Nin1.getNin1() ) {		// N in 1の場合.
				Key_ActSettingChange_actZoom.showEnable( False );
			} else {
				Key_ActSettingChange_actZoom.showEnable( True );
			}
			break;
		default:break;
	}


	if( PNLE_APPMODE_COPY != PnlComGetAppMode() ) {
		if( !( comPanelJob->getJobSpec()->pc_Send->c_ComSend.getUniversalAddressType() & TYPE_UniversalAddressTypeScan )
		    && !( comPanelJob->getJobSpec()->pc_Send->c_ComSend.getUniversalAddressType() & TYPE_UniversalAddressTypeBox ) ) {
			Key_ActSettingChange_actCenterErase.showEnable( False );		//「折り目消し」キーの非表示 .
			Key_ActSettingChange_actManuscriptCloseAndCarryOut.showEnable( False );		//「原稿のとじしろ」キーの非表示 .
			b_m_actManuscriptCloseAndCarryOut_Show = False;
		}
	}


// comment 2019/03/13 Sky飛田 【制御構成変更_No.66】 仕上がりプレビュー対応.
//	p_ManuscriptCloseAndCarryOut = Key_ActSettingChange_actManuscriptCloseAndCarryOut.getPullDownMenu();	// 2011/05/13 伊丹Sky平野 CQ.345143 .


	if( TYPE_JobTypeScan == e_JobType						// スキャン時.
	    ||  TYPE_JobTypeFaxSendMemory == e_JobType				// メモリ送信.
	    ||  TYPE_JobTypeFaxSendTimer == e_JobType				// タイマ通信.
	    ||  TYPE_JobTypeFaxReceivePolling == e_JobType ) {		// ポーリング送信(掲示板ポーリング送信を含む).
		Key_PullDownActSettingChange_actManuscriptSettingsCoverTwoSide.showEnable( True );
	} else {
		Key_PullDownActSettingChange_actManuscriptSettingsCoverTwoSide.showEnable( False );
	}
	Key_ActSettingChange_actCloseAndCarryOutPositionSetting.setPullDownTxtTable( ( ushort* )us_mBindPosition_Txt, PNLD_BindPosition_MaxKindCnt );
	p_ManuscriptCloseAndCarryOutPositionSetting = Key_ActSettingChange_actCloseAndCarryOutPositionSetting.getPullDownMenu();	// プルダウンアドレス取得.
	initBindPositionInfo();
	pc_mActSettingChange_act = this;
	Key_ActSettingChange_actManuscriptSetting.setPullDownTxtTable( ( ushort* )us_mManuscript_Txt, PNLD_ManuscriptSettings_MaxCnt );
	p_ManuscriptSetting = Key_ActSettingChange_actManuscriptSetting.getPullDownMenu();	// プルダウンアドレス取得.
	initManuscriptInfo();
// comment 2019/03/13 Sky飛田 【制御構成変更_No.66】 仕上がりプレビュー対応.
	Key_ActSettingChange_actManuscriptCloseAndCarryOut.setPullDownTxtTable( ( ushort* )us_mBindPositionCloseAndCarryOut_Txt, PNLD_BindPosition_MaxKindCnt );
	p_ManuscriptCloseAndCarryOut = Key_ActSettingChange_actManuscriptCloseAndCarryOut.getPullDownMenu();	// プルダウンアドレス取得.
	initManuscriptCloseAndCarryOut();

//	if( True == b_m_actManuscriptCloseAndCarryOut_Show ) {
//		initManuscriptCloseAndCarryOut();	// プルダウンメニューの初期化.
//		setupSelectedBindPositionType();	// 原稿のとじしろの設定状態取得.
//	}
}

//関数------------------------------------------------------------------------------------------------------------------
//概要    ：表示部品のセットアップ.
//備考    ：.
//戻値    ：無し.
//作成者  ：AWS_フェクス.
//レビュー：.
//変更履歴：.
//------------------------------------------------------------------------------
/**
* @brief A070800100 分割読込み停止中／設定変更メニュー : 表示部品のセットアップ
*/
//------------------------------------------------------------------------------
void PCPC_ActSettingChange_act::setupGamen( short option )
{
	PPopupDecisionButtonCom::setupGamen( option );			// 親クラスの表示部品のセットアップ処理.


	PCGC_VerifyThumbnailPreviewControl* pc_VerifyControl = PnlComGetVerifyPreviewControl();
	if( NULL == pc_VerifyControl )
	{
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return;
	}
	APIC_PrinterSystem* pc_PrinterSystem = PnlComGetPrinterSystem();
	TYPC_JobStatus*  pc_JobStatus = comPanelJob->getJobStatus();
	Bool b_ShowStaplePunchKey = False;   //フィニッシャー装着状態判定.
	TYPC_Print* pc_Print = comPanelJob->getJobSpec()->pc_Print;
	APIC_AutoSelect* pc_AutoSelect = comPanelJob->getAutoSelect();
	if( NULL == pc_AutoSelect ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );	return; }
	TYPE_OriginalSetting e_OriginalSetting = pc_AutoSelect->chkOriginalSetting( True );	// ADF/プラテン.
	TYPE_StapleMode c_stapleMode =  pc_Print->c_Finishing.getStapleMode();
	TYPE_PunchMode c_punchMode = pc_Print->c_Finishing.getPunchMode();
	uint32_t ui32_JobNumber = ( pc_JobStatus->getJobID() )%10000;		//ジョブナンバー獲得.
	if( ( isOn( CNK_BookSpread, comPanelJob ) )		//ブック原稿設定時.
	    || ( isOn( CNK_BookSeparation, comPanelJob ) ) ) {
		Key_ActSettingChange_actManuscriptSetting.activate( False );
		Key_ActSettingChange_actManuscriptCloseAndCarryOut.activate( False );		//「原稿のとじしろ」プルダウンキーはグレイアウトする.
		Key_ActSettingChange_actCenterErase.activate( True );						//「折目消し」キーはアクティブにする.
	} else {
		Key_ActSettingChange_actManuscriptSetting.activate( True );
// Comment VenusMLK -> 2017/03/22 SCD2 3G Sky瀬戸 [AccuRev.1017581]分割読込み停止中の設定変更では原稿サイズ設定を表示させない.
#ifndef DEF_A4_Cus
		Key_ActSettingChange_actOriginalSize.activate( True );
#endif // DEF_A4_Cus.
// Comment VenusMLK <- 2017/03/22 SCD2 3G Sky瀬戸 [AccuRev.1017581]分割読込み停止中の設定変更では原稿サイズ設定を表示させない.
		if( TYPE_OriginalSettingADF != e_OriginalSetting											// ADF以外.
		    && ( ( PNLD_ManuscriptTwoSide == pc_mActSettingChange_act->uc_mManuscriptSelectKind 		// 両面.
		           || PNLD_ManuscriptCoverTwoSide == pc_mActSettingChange_act->uc_mManuscriptSelectKind )	// 表示＋両面(ファクス／スキャン、ボックスモード時のみ表示).
		         && True == pc_AutoSelect->isDocSecondFace( True, True ) ) )								// 両面2面目(プラテンから両面読込中、奇数面を読み込んだ後は片面読み込みに変更させない).
		{
			Key_ActSettingChange_actManuscriptSetting.activate( False );
// Comment VenusMLK -> 2017/03/22 SCD2 3G Sky瀬戸 [AccuRev.1017581]分割読込み停止中の設定変更では原稿サイズ設定を表示させない.
#ifdef DEF_A4_Cus
			Key_ActSettingChange_actOriginalSize.activate( False );
#endif // DEF_A4_Cus.
// Comment VenusMLK <- 2017/03/22 SCD2 3G Sky瀬戸 [AccuRev.1017581]分割読込み停止中の設定変更では原稿サイズ設定を表示させない.
		}
		Key_ActSettingChange_actManuscriptSetting.setSelectNo( e_mManuscriptSelectIndex );	// 原稿.	// 2011/05/13 伊丹Sky平野 CQ.345143 .
		Key_ActSettingChange_actManuscriptCloseAndCarryOut.activate( True );		//「原稿のとじしろ」プルダウンキーはアクティブにする.
		Key_ActSettingChange_actCenterErase.activate( False );						//「折目消し」キーはグレイアウトする.

// comment 2019/03/13 Sky飛田 【制御構成変更_No.66】 仕上がりプレビュー対応.
//		setupSelectedBindPositionType();	// 原稿のとじしろの設定.	// 2011/05/13 伊丹Sky平野 CQ.345143 .
		Key_ActSettingChange_actManuscriptCloseAndCarryOut.setSelectNo( pc_mActSettingChange_act->e_mBindPositionCloseAndCarryOutSelectIndex );
	}

	if( ( True == comPanelJob->getJobSpec()->pc_Common->getVerifyPreviewOn() )				//コピーの仕上りプレビュー設定時.
	    && ( TYPE_ReqImgTypeCopy == pc_VerifyControl->getReqVerifyImgType() ) ) {

		Key_ActSettingChange_actCloseAndCarryOutPositionSetting.showEnable( True );			//「とじしろ位置」プルダウンキーの表示.

		Key_ActSettingChange_actCloseAndCarryOutPositionSetting.setSelectNo( e_mSelectIndex );	// 2011/05/13 伊丹Sky平野 CQ.345143 .

		if( ( False == pc_PrinterSystem->isExistFinisher() )								// フィニッシャが装着されていない.
		    || ( ( TYPE_StapleModeNone == c_stapleMode )										// ステープルモードOFF.
		         && ( TYPE_PunchModeNone  == c_punchMode ) ) ) {									// パンチモードOFF.
			b_ShowStaplePunchKey = False;													// パンチステープル非表示.
		} else {
			b_ShowStaplePunchKey = True;													// パンチステープル表示.
		}

		if( True == b_ShowStaplePunchKey ) {												// ステープル/パンチチェック状態.
			stapleModeSetup();																// ステープルモードセットアップ.
		}
	}
// A3A4FUM so_okada 2015.02.12 Start.
#ifndef DEF_A4_Cus	
	Key_ActSettingChange_actStapleAndPunchPosition.showEnable( b_ShowStaplePunchKey );		// ステープル/パンチ位置指定キーの表示/非表示.
#endif //DEF_A4_Cus.
// A3A4FUM so_okada 2015.02.12 End.
	// 伊丹Sky(眞狩)　2011/05/09 CQ340499 ↓.
	// 原稿ボタンの選択名を変更.
	Key_ActSettingChange_actManuscriptSetting.setSelectNo( pc_mActSettingChange_act->e_mManuscriptSelectIndex );

	if( ( True == comPanelJob->getJobSpec()->pc_Common->getVerifyPreviewOn() )
	    && ( TYPE_ReqImgTypeCopy == pc_VerifyControl->getReqVerifyImgType() ) ) {
// A3A4FUM so_okada 2015.02.12 Start.
#ifdef DEF_A4_Cus
		// コピーの仕上りプレビュー設定時に以下の条件で表示を切替える.
		Cmp_ActSettingChange_act_MessageArea.changeMainMessageText( s_pMSGActSetChangeModesettingMarginPos2_W448H034A );
#else
		// コピーの仕上りプレビュー設定時に以下の条件で表示を切替える.
		if( ( TYPE_StapleModeNone == c_stapleMode )
		    && ( TYPE_PunchModeNone == c_punchMode ) ) {
			// ステープルとパンチが設定されていない時.
			// メッセージ:「次に読込む原稿から、以下の設定が変更できます。\とじしろ位置の設定のみ、全原稿に反映されます。」.
// comment 2017/01/09 [HCLT_Priyanka] [IT5_4.0][Quarter Font Phase2] [1006281] Changes for Quarter font to Half font.
//			Cmp_ActSettingChange_act_MessageArea.changeMainMessageText( s_pMSGActSetChangeModesettingMarginPos2_W448H034A );
		    Cmp_ActSettingChange_act_MessageArea.changeMainMessageText( s_pMSGActSetChangeModesettingMarginPos2_W700H034A );
		} else if( TYPE_StapleModeNone == c_stapleMode ) {
			// パンチが設定されている時.
			// メッセージ:「次に読込む原稿から、以下の設定が変更できます。\とじしろ位置とパンチ位置の設定のみ、全原稿に反映されます。」.
// comment 2017/01/09 [HCLT_Priyanka] [IT5_4.0][Quarter Font Phase2] [1006281] Changes for Quarter font to Half font.
//			Cmp_ActSettingChange_act_MessageArea.changeMainMessageText( s_pMSGActSetChangeModesettingMarginPosPunch2_W448H034A );
			Cmp_ActSettingChange_act_MessageArea.changeMainMessageText( s_pMSGActSetChangeModesettingMarginPosPunch2_W700H034A );
#ifdef DEF_LCD_7INCH
			Key_ActSettingChange_actStapleAndPunchPosition.changeText( &Ktx_ActSettingChange_actStapleAndPunchPosition ,s_pKEYPunchPosition_W176H020A );
#else // #ifdef DEF_LCD_7INCH.
			Key_ActSettingChange_actStapleAndPunchPosition.changeText( &Ktx_ActSettingChange_actStapleAndPunchPosition ,s_pKEYPunchPosition_W164H016A );
#endif // #ifdef DEF_LCD_7INCH.
		} else if( TYPE_PunchModeNone == c_punchMode ) {
			// ステープルが設定されている時.
			// メッセージ:「次に読込む原稿から、以下の設定が変更できます。\とじしろ位置とステープル位置の設定のみ、全原稿に反映されます。」.
//Comment 2017/01/25 HCL-K.Vijay[1006281] Changes made for conversion to half font.
//			Cmp_ActSettingChange_act_MessageArea.changeMainMessageText( s_pMSGNextScanDocSettingsStaplePosition_W448H034A );
#ifdef DEF_LCD_7INCH
			Cmp_ActSettingChange_act_MessageArea.changeMainMessageText( s_pMSGNextScanDocSettingsStaplePosition_W660H038A );
#else // #ifdef DEF_LCD_7INCH.
			Cmp_ActSettingChange_act_MessageArea.changeMainMessageText( s_pMSGNextScanDocSettingsStaplePosition_W606H034A );
#endif // #ifdef DEF_LCD_7INCH.

#ifdef DEF_LCD_7INCH
			Key_ActSettingChange_actStapleAndPunchPosition.changeText( &Ktx_ActSettingChange_actStapleAndPunchPosition ,s_pKEYStaplePosition_W176H020A );
#else // #ifdef DEF_LCD_7INCH.
			Key_ActSettingChange_actStapleAndPunchPosition.changeText( &Ktx_ActSettingChange_actStapleAndPunchPosition ,s_pKEYStaplePosition_W164H016A );
#endif // #ifdef DEF_LCD_7INCH.
		} else {
			// ステープルとパンチが設定されている時.
			// メッセージ:「次に読込む原稿から、以下の設定が変更できます。\とじしろ位置とステープル/パンチ位置の設定のみ、全原稿に反映されます。」.
// comment 2017/01/09 [HCLT_Priyanka] [IT5_4.0][Quarter Font Phase2] [1006281] Changes for Quarter font to Half font.
//			Cmp_ActSettingChange_act_MessageArea.changeMainMessageText( s_pMSGActSetChangeModesettingMarginPosStaplePunch_W448H034A );	// 2010/07/26 シンボル変更対応 伊丹Sky水嶋.
			Cmp_ActSettingChange_act_MessageArea.changeMainMessageText( s_pMSGActSetChangeModesettingMarginPosStaplePunch_W700H034A );
#ifdef DEF_LCD_7INCH
			Key_ActSettingChange_actStapleAndPunchPosition.changeText( &Ktx_ActSettingChange_actStapleAndPunchPosition , s_pKEYStapleAndPunchPosition_W176H020A );
#else // #ifdef DEF_LCD_7INCH.
			Key_ActSettingChange_actStapleAndPunchPosition.changeText( &Ktx_ActSettingChange_actStapleAndPunchPosition , s_pKEYStapleAndPunchPosition_W164H016A );
#endif // #ifdef DEF_LCD_7INCH.
		}
#endif //DEF_A4_Cus.
// A3A4FUM so_okada 2015.02.12 End.		
	} else {
		Cmp_ActSettingChange_act_MessageArea.changeMainMessageText( s_pMSGActSetChangeModesetting_W448H034A );		// メッセージエリアセット.
	}

	//倍率ボタンの設定.
	TYPC_Common* pc_Common = comPanelJob->getJobSpec()->pc_Common;

	TYPC_Point<float> c_magVal = pc_Common->c_Mag.getMag().xyReverse();		// 倍率値取得.
	TYPE_MagType	e_MagType = pc_Common->c_Mag.getMagType();
	if( TYPE_MagTypeXyZoom == e_MagType ) {
		Key_ActSettingChange_actZoom.showEnableAdd( &Ktx_ActSettingChange_actZoom_Sub2 ,False );
		Key_ActSettingChange_actZoom.showEnableAdd( &Txt_ActSettingChange_ColonX ,True );			// X:.
		Key_ActSettingChange_actZoom.showEnableAdd( &Num_ActSettingChange_FieldX ,True );			// X:.
		Key_ActSettingChange_actZoom.showEnableAdd( &Txt_ActSettingChange_ColonY ,True );			// Y:.
		Key_ActSettingChange_actZoom.showEnableAdd( &Num_ActSettingChange_FieldY ,True );			// Y:.
		Num_ActSettingChange_FieldX.set( c_magVal.x * 100, True );
		Num_ActSettingChange_FieldY.set( c_magVal.y * 100, True );
	} else {
		Key_ActSettingChange_actZoom.showEnableAdd( &Txt_ActSettingChange_ColonX ,False );			// X:.
		Key_ActSettingChange_actZoom.showEnableAdd( &Num_ActSettingChange_FieldX ,False );			// X:.
		Key_ActSettingChange_actZoom.showEnableAdd( &Txt_ActSettingChange_ColonY ,False );			// X:.
		Key_ActSettingChange_actZoom.showEnableAdd( &Num_ActSettingChange_FieldY ,False );			// X:.
		Key_ActSettingChange_actZoom.showEnableAdd( &Ktx_ActSettingChange_actZoom_Sub2 ,True );		// X:.
		Key_ActSettingChange_actZoom.set( &Ktx_ActSettingChange_actZoom_Sub2, c_magVal.x * 100 , True ); //倍率を表示.
	}


	if( TYPE_MagTypeXyZoom == e_MagType ) {
		Key_ActSettingChange_actZoom.showEnableAdd( &Ktx_ActSettingChange_actZoom_Sub2Percent ,False );	// %を非表示.
		Key_ActSettingChange_actZoom.showEnableAdd( &Ktx_ActSettingChange_actZoom_Sub ,False );
		Key_ActSettingChange_actZoom.showEnableAdd( &Txt_ActSettingChange_PercentX ,True );		// %(X)を表示.
		Key_ActSettingChange_actZoom.showEnableAdd( &Txt_ActSettingChange_PercentY ,True );		// %(Y)を表示.
	} else {
		Key_ActSettingChange_actZoom.showEnableAdd( &Txt_ActSettingChange_PercentX ,False );		// %(X)を非表示.
		Key_ActSettingChange_actZoom.showEnableAdd( &Txt_ActSettingChange_PercentY ,False );		// %(Yを非表示).
		Key_ActSettingChange_actZoom.showEnableAdd( &Ktx_ActSettingChange_actZoom_Sub2Percent ,True );	// %を表示.
		Key_ActSettingChange_actZoom.showEnableAdd( &Ktx_ActSettingChange_actZoom_Sub ,True );
	}

	uchar uc_Destination = PnlComGetDestination();	// 仕向設定内容取得.
	switch( pc_Common->c_Mag.getMagType() ) {
		case TYPE_MagTypeFull:
			Key_ActSettingChange_actZoom.changeText( &Ktx_ActSettingChange_actZoom_Sub, NULL );
			break;
		case TYPE_MagTypeE1:
			if( TYPE_SystemType_US == uc_Destination ) {
#ifdef DEF_LCD_7INCH
				Key_ActSettingChange_actZoom.changeText( &Ktx_ActSettingChange_actZoom_Sub, s_pNOT81P2x14to11x17_W156H018A );	//拡大1
#else // #ifdef DEF_LCD_7INCH.
				Key_ActSettingChange_actZoom.changeText( &Ktx_ActSettingChange_actZoom_Sub, s_pNOT81P2x14to11x17_W144H016A );	//拡大1
#endif // #ifdef DEF_LCD_7INCH.
			} else {
// comment 2013.08.22 OPS1-Sky浜野 [次世代アーキ] [7inch化対応].
//					Key_ActSettingChange_actZoom.changeText( &Ktx_ActSettingChange_actZoom_Sub, s_pNOTB4B5toA3A4_W144H016A );	//拡大1
#ifdef DEF_LCD_7INCH
#ifdef DEF_A4_Cus   // A3A4共通化FUM ST 2015.2.5 KM我妻.
					Key_ActSettingChange_actZoom.changeText( &Ktx_ActSettingChange_actZoom_Sub, s_pNOTB5B6toA4A5_W156H018A );	//拡大1
#else // #ifdef DEF_A4_Cus.
					Key_ActSettingChange_actZoom.changeText( &Ktx_ActSettingChange_actZoom_Sub, s_pNOTB4B5toA3A4_W156H018A );	//拡大1
#endif // #ifdef DEF_A4_Cus.
#else // #ifdef DEF_LCD_7INCH.
//Comment 2018/03/14 【IT6_Sparrow】【HCL_Rajkumar】制御構成変更_No.222 Message Modification.
#ifdef DEF_A4_Cus
				Key_ActSettingChange_actZoom.changeText( &Ktx_ActSettingChange_actZoom_Sub, s_pNOTB5B6toA4A5_W156H018A );
#else // DEF_A4_Cus.
				Key_ActSettingChange_actZoom.changeText( &Ktx_ActSettingChange_actZoom_Sub, s_pNOTB4B5toA3A4_W144H016A );	//拡大1
#endif // DEF_A4_Cus.
#endif // #ifdef DEF_LCD_7INCH.
			}
			break;
		case TYPE_MagTypeE2:
				if( TYPE_SystemType_US == uc_Destination ){
// comment 2013.08.22 OPS1-Sky浜野 [次世代アーキ] [7inch化対応].
//					Key_ActSettingChange_actZoom.changeText( &Ktx_ActSettingChange_actZoom_Sub, s_pNOT81P2x11to11x17_W144H016A );		//拡大2
#ifdef DEF_LCD_7INCH
#ifdef DEF_A4_Cus   // A3A4共通化FUM ST 2015.2.5 KM我妻.
					Key_ActSettingChange_actZoom.changeText( &Ktx_ActSettingChange_actZoom_Sub, s_pNOT51P2x81P2to81P2x11_W156H018A );	//拡大2
#else // #ifdef DEF_A4_Cus.
					Key_ActSettingChange_actZoom.changeText( &Ktx_ActSettingChange_actZoom_Sub, s_pNOT81P2x11to11x17_W156H018A );		//拡大2
#endif // #ifdef DEF_A4_Cus.
#else // #ifdef DEF_LCD_7INCH.
//Comment 2018/03/14 【IT6_Sparrow】【HCL_Rajkumar】制御構成変更_No.222 Message Modification.
#ifdef DEF_A4_Cus
				Key_ActSettingChange_actZoom.changeText( &Ktx_ActSettingChange_actZoom_Sub, s_pNOT51P2x81P2to81P2x11_W156H018A );
#else // DEF_A4_Cus.
				Key_ActSettingChange_actZoom.changeText( &Ktx_ActSettingChange_actZoom_Sub, s_pNOT81P2x11to11x17_W144H016A );		//拡大2
#endif // DEF_A4_Cus.
#endif // #ifdef DEF_LCD_7INCH.
			} else {
// comment 2013.08.22 OPS1-Sky浜野 [次世代アーキ] [7inch化対応].
//					Key_ActSettingChange_actZoom.changeText( &Ktx_ActSettingChange_actZoom_Sub, s_pNOTA4toB4_W052H016A );		//拡大2
#ifdef DEF_LCD_7INCH
#ifdef DEF_A4_Cus   // A3A4共通化FUM ST 2015.2.5 KM我妻.
					Key_ActSettingChange_actZoom.changeText( &Ktx_ActSettingChange_actZoom_Sub, s_pNOTA5A6toB5B6_W156H018A );	//拡大2
#else // #ifdef DEF_A4_Cus.
					Key_ActSettingChange_actZoom.changeText( &Ktx_ActSettingChange_actZoom_Sub, s_pNOTA4toB4_W156H018A );		//拡大2
#endif // #ifdef DEF_A4_Cus.
#else // #ifdef DEF_LCD_7INCH.
//Comment 2018/03/14 【IT6_Sparrow】【HCL_Rajkumar】制御構成変更_No.222 Message Modification.
#ifdef DEF_A4_Cus
				Key_ActSettingChange_actZoom.changeText( &Ktx_ActSettingChange_actZoom_Sub, s_pNOTA5A6toB5B6_W156H018A );
#else // DEF_A4_Cus.
				Key_ActSettingChange_actZoom.changeText( &Ktx_ActSettingChange_actZoom_Sub, s_pNOTA4toB4_W052H016A );		//拡大2
#endif // DEF_A4_Cus.
#endif // #ifdef DEF_LCD_7INCH.
			}
			break;
		case TYPE_MagTypeE3:
			if( TYPE_SystemType_US == uc_Destination ) {
#ifdef DEF_LCD_7INCH
				Key_ActSettingChange_actZoom.changeText( &Ktx_ActSettingChange_actZoom_Sub, s_pNOT51P2x81P21to81P2x14_W156H018A );	//拡大3
#else // #ifdef DEF_LCD_7INCH.
				Key_ActSettingChange_actZoom.changeText( &Ktx_ActSettingChange_actZoom_Sub, s_pNOT51P2x81P21to81P2x14_W144H016A );	//拡大3
#endif // #ifdef DEF_LCD_7INCH.
			} else {
// comment 2013.08.22 OPS1-Sky浜野 [次世代アーキ] [7inch化対応].
//					Key_ActSettingChange_actZoom.changeText( &Ktx_ActSettingChange_actZoom_Sub, s_pNOTA4B5toA3B4_W144H016A );	//拡大3
#ifdef DEF_LCD_7INCH
#ifdef DEF_A4_Cus   // A3A4共通化FUM ST 2015.2.5 KM我妻.
					Key_ActSettingChange_actZoom.changeText( &Ktx_ActSettingChange_actZoom_Sub, s_pNOTA5B6toA4B5_W156H018A );	//拡大3
#else // #ifdef DEF_A4_Cus.
					Key_ActSettingChange_actZoom.changeText( &Ktx_ActSettingChange_actZoom_Sub, s_pNOTA4B5toA3B4_W156H018A );	//拡大3
#endif // #ifdef DEF_A4_Cus.
#else // #ifdef DEF_LCD_7INCH.
//Comment 2018/03/14 【IT6_Sparrow】【HCL_Rajkumar】制御構成変更_No.222 Message Modification.
#ifdef DEF_A4_Cus
				Key_ActSettingChange_actZoom.changeText( &Ktx_ActSettingChange_actZoom_Sub, s_pNOTA5B6toA4B5_W156H018A );
#else // DEF_A4_Cus.
				Key_ActSettingChange_actZoom.changeText( &Ktx_ActSettingChange_actZoom_Sub, s_pNOTA4B5toA3B4_W144H016A );	//拡大3
#endif // DEF_A4_Cus.
#endif // #ifdef DEF_LCD_7INCH.
			}
			break;
		case TYPE_MagTypeR1:
			if( TYPE_SystemType_US == uc_Destination ) {
#ifdef DEF_LCD_7INCH
				Key_ActSettingChange_actZoom.changeText( &Ktx_ActSettingChange_actZoom_Sub, s_pNOT81P2x14to81P2x11_W156H018A );	//縮小1
#else // #ifdef DEF_LCD_7INCH.
				Key_ActSettingChange_actZoom.changeText( &Ktx_ActSettingChange_actZoom_Sub, s_pNOT81P2x14to81P2x11_W144H016A );	//縮小1
#endif // #ifdef DEF_LCD_7INCH.
			} else {
// comment 2013.08.22 OPS1-Sky浜野 [次世代アーキ] [7inch化対応].
//					Key_ActSettingChange_actZoom.changeText( &Ktx_ActSettingChange_actZoom_Sub, s_pNOTA3A4toB4B5_W144H016A );	//縮小1
#ifdef DEF_LCD_7INCH
#ifdef DEF_A4_Cus   // A3A4共通化FUM ST 2015.2.5 KM我妻.
					Key_ActSettingChange_actZoom.changeText( &Ktx_ActSettingChange_actZoom_Sub, s_pNOTA4A5toB5B6_W156H018A );	//縮小1
#else // #ifdef DEF_A4_Cus.
					Key_ActSettingChange_actZoom.changeText( &Ktx_ActSettingChange_actZoom_Sub, s_pNOTA3A4toB4B5_W156H018A );	//縮小1
#endif // #ifdef DEF_A4_Cus.
#else // #ifdef DEF_LCD_7INCH.
//Comment 2018/03/14 【IT6_Sparrow】【HCL_Rajkumar】制御構成変更_No.222 Message Modification.
#ifdef DEF_A4_Cus
				Key_ActSettingChange_actZoom.changeText( &Ktx_ActSettingChange_actZoom_Sub, s_pNOTA4A5toB5B6_W156H018A );
#else // DEF_A4_Cus.
				Key_ActSettingChange_actZoom.changeText( &Ktx_ActSettingChange_actZoom_Sub, s_pNOTA3A4toB4B5_W144H016A );	//縮小1
#endif // DEF_A4_Cus.
#endif // #ifdef DEF_LCD_7INCH.
			}
			break;
		case TYPE_MagTypeR2:
			if( TYPE_SystemType_US == uc_Destination ) {
#ifdef DEF_LCD_7INCH
				Key_ActSettingChange_actZoom.changeText( &Ktx_ActSettingChange_actZoom_Sub, s_pNOT11x17to81P2x14_W156H018A );		//縮小2
#else // #ifdef DEF_LCD_7INCH.
				Key_ActSettingChange_actZoom.changeText( &Ktx_ActSettingChange_actZoom_Sub, s_pNOT11x17to81P2x14_W144H016A );		//縮小2
#endif // #ifdef DEF_LCD_7INCH.
			} else {
// comment 2013.08.22 OPS1-Sky浜野 [次世代アーキ] [7inch化対応].
//					Key_ActSettingChange_actZoom.changeText( &Ktx_ActSettingChange_actZoom_Sub, s_pNOTB4toA4_W052H016A );		//縮小2
#ifdef DEF_LCD_7INCH
#ifdef DEF_A4_Cus   // A3A4共通化FUM ST 2015.2.5 KM我妻.
					Key_ActSettingChange_actZoom.changeText( &Ktx_ActSettingChange_actZoom_Sub, s_pNOTB5B6toA5A6_W156H018A );	//縮小2
#else // #ifdef DEF_A4_Cus.
					Key_ActSettingChange_actZoom.changeText( &Ktx_ActSettingChange_actZoom_Sub, s_pNOTB4toA4_W156H018A );		//縮小2
#endif // #ifdef DEF_A4_Cus.
#else // #ifdef DEF_LCD_7INCH.
//Comment 2018/03/14 【IT6_Sparrow】【HCL_Rajkumar】制御構成変更_No.222 Message Modification.
#ifdef DEF_A4_Cus
				Key_ActSettingChange_actZoom.changeText( &Ktx_ActSettingChange_actZoom_Sub, s_pNOTB5B6toA5A6_W156H018A );
#else // DEF_A4_Cus.
				Key_ActSettingChange_actZoom.changeText( &Ktx_ActSettingChange_actZoom_Sub, s_pNOTB4toA4_W052H016A );		//縮小2
#endif // DEF_A4_Cus.
#endif // #ifdef DEF_LCD_7INCH.
			}
			break;
		case TYPE_MagTypeR3:
				if( TYPE_SystemType_US == uc_Destination ){
// comment 2013.08.22 OPS1-Sky浜野 [次世代アーキ] [7inch化対応].
//					Key_ActSettingChange_actZoom.changeText( &Ktx_ActSettingChange_actZoom_Sub, s_pNOT11x17to81P2x11_W144H016A );	//縮小3
#ifdef DEF_LCD_7INCH
#ifdef DEF_A4_Cus   // A3A4共通化FUM ST 2015.2.5 KM我妻.
// Comment 2019/07/04 HCL-Ranjith [EagleH][OP_BTS-9559] Message ID change for Zoom 64.7%.
//					Key_ActSettingChange_actZoom.changeText( &Ktx_ActSettingChange_actZoom_Sub, s_pNOT81P2x13to51P2x81P2_W156H018A );	//縮小3
					Key_ActSettingChange_actZoom.changeText( &Ktx_ActSettingChange_actZoom_Sub, s_pNOT81P2x11to51P2x81P2_W156H018A );	//縮小3
#else // #ifdef DEF_A4_Cus.
					Key_ActSettingChange_actZoom.changeText( &Ktx_ActSettingChange_actZoom_Sub, s_pNOT11x17to81P2x11_W156H018A );	//縮小3
#endif // #ifdef DEF_A4_Cus.
#else // #ifdef DEF_LCD_7INCH.
//Comment 2018/03/14 【IT6_Sparrow】【HCL_Rajkumar】制御構成変更_No.222 Message Modification.
#ifdef DEF_A4_Cus
// Comment 2019/07/04 HCL-Ranjith [EagleH][OP_BTS-9559] Message ID change for Zoom 64.7%.
//				Key_ActSettingChange_actZoom.changeText( &Ktx_ActSettingChange_actZoom_Sub, s_pNOT81P2x13to51P2x81P2_W156H018A );	
				Key_ActSettingChange_actZoom.changeText( &Ktx_ActSettingChange_actZoom_Sub, s_pNOT81P2x11to51P2x81P2_W156H018A );	
#else // DEF_A4_Cus.
				Key_ActSettingChange_actZoom.changeText( &Ktx_ActSettingChange_actZoom_Sub, s_pNOT11x17to81P2x11_W144H016A );	//縮小3
#endif // DEF_A4_Cus.
#endif // #ifdef DEF_LCD_7INCH.
			} else {
// comment 2013.08.22 OPS1-Sky浜野 [次世代アーキ] [7inch化対応].
//					Key_ActSettingChange_actZoom.changeText( &Ktx_ActSettingChange_actZoom_Sub, s_pNOTA3B4toA4B5_W144H016A );	//縮小3
#ifdef DEF_LCD_7INCH
#ifdef DEF_A4_Cus   // A3A4共通化FUM ST 2015.2.5 KM我妻.
					Key_ActSettingChange_actZoom.changeText( &Ktx_ActSettingChange_actZoom_Sub, s_pNOTA4B5toA5B6_W156H018A );	//縮小3
#else // #ifdef DEF_A4_Cus.
					Key_ActSettingChange_actZoom.changeText( &Ktx_ActSettingChange_actZoom_Sub, s_pNOTA3B4toA4B5_W156H018A );	//縮小3
#endif // #ifdef DEF_A4_Cus.
#else // #ifdef DEF_LCD_7INCH.
//Comment 2018/03/14 【IT6_Sparrow】【HCL_Rajkumar】制御構成変更_No.222 Message Modification.
#ifdef DEF_A4_Cus
				Key_ActSettingChange_actZoom.changeText( &Ktx_ActSettingChange_actZoom_Sub, s_pNOTA4B5toA5B6_W156H018A ); 
#else // DEF_A4_Cus.
				Key_ActSettingChange_actZoom.changeText( &Ktx_ActSettingChange_actZoom_Sub, s_pNOTA3B4toA4B5_W144H016A );	//縮小3
#endif // DEF_A4_Cus.
#endif // #ifdef DEF_LCD_7INCH.
			}
			break;
#ifdef DEF_A4_Cus	// VenusMLK 管理No.98_1_6 OPS2 Sky後藤(拓) 2015/12/17.
			case TYPE_MagTypeE5:
				Key_ActSettingChange_actZoom.changeText( &Ktx_ActSettingChange_actZoom_Sub, s_pNOTB6toA4_W152H018A );
				break;
			case TYPE_MagTypeR5:
				if( TYPE_SystemType_US == uc_Destination ){
						Key_ActSettingChange_actZoom.changeText( &Ktx_ActSettingChange_actZoom_Sub, s_pNOTLegalTo512x812_W152H018A );
				} else {
						Key_ActSettingChange_actZoom.changeText( &Ktx_ActSettingChange_actZoom_Sub, s_pNOTA4toB6_W152H018A );
				}
				break;
#endif // DEF_A4_Cus. VenusMLK 管理No.98_1_6 OPS2 Sky後藤(拓) 2015/12/17.
		case TYPE_MagTypeM1:
		case TYPE_MagTypeM2:
		case TYPE_MagTypeM3:
		case TYPE_MagTypeZoom:
		case TYPE_MagTypeE4:
		case TYPE_MagTypeR4:
		case TYPE_MagTypeUnselect:
			Key_ActSettingChange_actZoom.changeText( &Ktx_ActSettingChange_actZoom_Sub, NULL );
			break;
		default:
			break;
	} //HCL<330745><20100804>-Modify change.


// Comment VenusMLK -> 2016/08/09 SCD2 3G Sky瀬戸 スクロールバー表示/非表示対応.
#ifndef DEF_A4_Cus
	PNLS_setOriginalSizeCLCD2KeyPosition s_parts;
	s_parts.ps_KeyOriginalSize   =	&Key_ActSettingChange_actOriginalSize;
	s_parts.ps_TxtOriginalSize01 =	&Ktx_ActSettingChange_actOriginalSize_Sub;			// １行目 用紙コード表示.
	s_parts.ps_TxtOriginalSize02 =	&Ktx_ActSettingChange_actOriginalSize_Sub2;			// ２行目 自動表示.
	s_parts.ps_TxtOriginalSize03 =	&Ktx_ActSettingChange_actOriginalSize_Sub3;			// ２行目 自動表示.
	s_parts.ps_TxtOriginalSizeX =	&Ktx_ActSettingChange_actOriginalSize_XSizeText;	// 用紙サイズ表示 X.
	s_parts.ps_TxtOriginalSizeY =	&Ktx_ActSettingChange_actOriginalSize_YSizeText;	// 用紙サイズ表示 Y.
	s_parts.ps_NumOriginalSizeX =	&Num_ActSettingChange_actOriginalSize_SizeXNum;		// 用紙サイズ数値表示 X.
	s_parts.ps_NumOriginalSizeY =	&Num_ActSettingChange_actOriginalSize_SizeYNum;		// 用紙サイズ数値表示 Y.
	s_parts.ps_TrayTxtPaperSize = NULL;
	s_parts.ps_TrayIconPaperSize = NULL;
	s_parts.ps_RemainIconPaperSize = NULL;
	s_parts.ps_TypeIconPaperSize = NULL;

	//原稿サイズの表示を行う.
	PnlComsetOriginalSizeCLCD2( &s_parts );
#endif // DEF_A4_Cus.
// Comment VenusMLK <- 2016/08/09 SCD2 3G Sky瀬戸 スクロールバー表示/非表示対応.

	//枠消しの設定.
	if( True == isOn( CNK_FrameErase, comPanelJob ) ) {
#ifdef DEF_LCD_7INCH
		Key_ActSettingChange_actFrameErase.changeText( s_pITMOn_W156H018A, PNLE_TransitionButtonText2Index );
#else // #ifdef DEF_LCD_7INCH.
		Key_ActSettingChange_actFrameErase.changeText( s_pITMOn_W144H016A, PNLE_TransitionButtonText2Index );
#endif // #ifdef DEF_LCD_7INCH.
	} else {
#ifdef DEF_LCD_7INCH
		Key_ActSettingChange_actFrameErase.changeText( s_pITMOFF_W156H018A, PNLE_TransitionButtonText2Index );
#else // #ifdef DEF_LCD_7INCH.
		Key_ActSettingChange_actFrameErase.changeText( s_pITMOFF_W144H016A, PNLE_TransitionButtonText2Index );
#endif // #ifdef DEF_LCD_7INCH.
	}

	// 折り目消し.
	TYPC_Scan* pc_Scan = comPanelJob->getJobSpec()->pc_Scan;
	float sf_EraseSize = pc_Scan->c_OriginalErase.getCenter();
	if( isOn( CNK_CreaseErase, comPanelJob ) ) {
		Key_ActSettingChange_actCenterErase.setNum( sf_EraseSize );
	} else {
		// テキストOFF表示.
#ifdef DEF_LCD_7INCH
		Key_ActSettingChange_actCenterErase.changeText( s_pITMOFF_W156H018A, PNLE_TransitionButtonText2Index );
#else // #ifdef DEF_LCD_7INCH.
		Key_ActSettingChange_actCenterErase.changeText( s_pITMOFF_W144H016A, PNLE_TransitionButtonText2Index );
#endif // #ifdef DEF_LCD_7INCH.
	}
// comment 2016/01/12 [HCLT-Thangachamy][Accurev ID:1009146] Job number display.
//	Num_ActSettingChange_actJobNumber.set( ( float )( ui32_JobNumber ),True );		// ジョブナンバー価値ディスプレイ.
	Num_ActSettingChange_actJobNumber = static_cast<float>( ui32_JobNumber );		// ジョブナンバー価値ディスプレイ.

	showPatternCustomize();

// comment 2020/07/14 SCD2-Sky 石原 [IT6_2.0MR2][OP_BTS-25362] フッターエリア対応.
#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus)  && defined (DEF_LCD_7INCH) ) )
	PnlComFooterPartsBtnActiveChange(homeKeyActiveCheck(), idKeyActiveCheck(), resetKeyActiveCheck());
#endif		//  ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus)  && defined (DEF_LCD_7INCH) ) )	.

}

//関数------------------------------------------------------------------------------------------------------------------
//概要    ：ステープルモードセットアップ.
//備考    ：.
//戻値    ：無し.
//作成者  ：AWS_フェクス.
//レビュー：.
//変更履歴：.
//------------------------------------------------------------------------------
/**
* @brief A070800100 分割読込み停止中／設定変更メニュー : ステープルモードセットアップ
*/
//------------------------------------------------------------------------------
void PCPC_ActSettingChange_act::stapleModeSetup()
{
// A3A4FUM so_okada 2015.02.12 Start.
#ifndef DEF_A4_Cus
	TYPC_Print* pc_Print = comPanelJob->getJobSpec()->pc_Print;
	TYPE_StapleMode c_stapleMode =  pc_Print->c_Finishing.getStapleMode();
	TYPE_PunchMode c_punchMode = pc_Print->c_Finishing.getPunchMode();
	TYPE_BindDirection c_staplePosition = pc_Print->c_Finishing.getStaplePosition();
	TYPE_BindDirection c_punchPosition = pc_Print->c_Finishing.getPunchPosition();
	Bool b_PositionAutoStaple = pc_Print->c_Finishing.getStaplePositionAuto();
	Bool b_PositionAutoPunch = pc_Print->c_Finishing.getPunchPositionAuto();
	ushort us_textID = 0;
	if( ( True == b_PositionAutoStaple ) && ( True == b_PositionAutoPunch ) ) {
		us_textID = s_pKEYAuto_W136H028A;
	} else {
		switch( c_stapleMode ) {
			case TYPE_StapleModeSide:      		// ステープル_2点キー選択.
				if( c_punchMode == TYPE_PunchModeNone ) {		// ステープル_2点キーのみ選択時.
					switch( c_staplePosition ) {
#ifdef DEF_LCD_7INCH
						case TYPE_BindDirectionTop:		//　ステープル：上　　
							us_textID = s_pKEYStapleTopNoPunch_W156H040A;
							break;
						case TYPE_BindDirectionLeft:		//　ステープル：左
							us_textID =  s_pKEYStapleLeftNoPunch_W156H040A;
							break;
						case TYPE_BindDirectionRight:	//　ステープル：右　
							us_textID = s_pKEYStapleRightNoPunch_W156H040A;
							break;
#else // #ifdef DEF_LCD_7INCH.
						case TYPE_BindDirectionTop:		//　ステープル：上　　
							us_textID = s_pKEYStapleTopNoPunch_W136H036A;
							break;
						case TYPE_BindDirectionLeft:		//　ステープル：左
							us_textID =  s_pKEYStapleLeftNoPunch_W136H036A;
							break;
						case TYPE_BindDirectionRight:	//　ステープル：右　
							us_textID = s_pKEYStapleRightNoPunch_W136H036A;
							break;
#endif // #ifdef DEF_LCD_7INCH.
						default:
							break;
					}
				} else {		// ステープル_2点キー選択、かつパンチ2穴4穴選択時.
					switch( c_punchPosition ) {
#ifdef DEF_LCD_7INCH
						case TYPE_BindDirectionTop:	//　　　ステープル/パンチ：上　
							us_textID = s_pKEYStapleTopPunchTop_W156H040A;
							break;
						case TYPE_BindDirectionLeft:	//　　　ステープル/パンチ：左　
							us_textID = s_pKEYStapleLeftPunchLeft_W156H040A;
							break;
						case TYPE_BindDirectionRight:	//　　　ステープル/パンチ：右　
							us_textID = s_pKEYStapleRightPunchRight_W156H040A;
							break;
#else // #ifdef DEF_LCD_7INCH.
						case TYPE_BindDirectionTop:	//　　　ステープル/パンチ：上　
							us_textID = s_pKEYStapleTopPunchTop_W136H036A;
							break;
						case TYPE_BindDirectionLeft:	//　　　ステープル/パンチ：左　
							us_textID = s_pKEYStapleLeftPunchLeft_W136H036A;
							break;
						case TYPE_BindDirectionRight:	//　　　ステープル/パンチ：右　
							us_textID = s_pKEYStapleRightPunchRight_W136H036A;
							break;
#endif // #ifdef DEF_LCD_7INCH.
						default:
							break;
					}
				}
				break;
			case TYPE_StapleModeCorner:			// ステープル_コーナーキー選択.
				if( c_punchMode != TYPE_PunchModeNone ) {	//	ステープル_コーナーキー選択、かつパンチ_なしキー未選択時
					switch( c_staplePosition ) {
						case TYPE_BindDirectionLeft:
#ifdef DEF_LCD_7INCH
							if( c_punchPosition == TYPE_BindDirectionTop ) {
								us_textID = s_pKEYStapleTopLeftPunchTop_W156H040A;		//　コーナー：左上　パンチ上
							} else {
								us_textID = s_pKEYStapleTopLeftPunchLeft_W156H040A;	//　コーナー：左上　パンチ左　
							}
#else // #ifdef DEF_LCD_7INCH.
							if( c_punchPosition == TYPE_BindDirectionTop ) {
								us_textID = s_pKEYStapleTopLeftPunchTop_W136H036A;		//　コーナー：左上　パンチ上
							} else {
								us_textID = s_pKEYStapleTopLeftPunchLeft_W136H036A;	//　コーナー：左上　パンチ左　
							}
#endif // #ifdef DEF_LCD_7INCH.
							break;
						case TYPE_BindDirectionRight:
#ifdef DEF_LCD_7INCH
							if( c_punchPosition == TYPE_BindDirectionTop ) {
								us_textID = s_pKEYStapleTopRightPunchTop_W156H040A;		//　コーナー：右上　パンチ上
							} else {
								us_textID = s_pKEYStapleTopRightPunchRight_W156H040A;	//　コーナー：右上　パンチ右　
							}
#else // #ifdef DEF_LCD_7INCH.
							if( c_punchPosition == TYPE_BindDirectionTop ) {
								us_textID = s_pKEYStapleTopRightPunchTop_W136H036A;		//　コーナー：右上　パンチ上
							} else {
								us_textID = s_pKEYStapleTopRightPunchRight_W136H036A;	//　コーナー：右上　パンチ右　
							}
#endif // #ifdef DEF_LCD_7INCH.
							break;
						default:
							break;
					}
				} else {	// ステープル_コーナーキー選択、かつパンチ_なしキー選択時
					switch( c_staplePosition ) {
#ifdef DEF_LCD_7INCH
						case TYPE_BindDirectionTop:	//　　　コーナーステープル：左上.
						case TYPE_BindDirectionLeft:	//　　　コーナーステープル：左上.
							us_textID = s_pKEYStapleTopLeftNoPunch_W156H040A;
							break;
						case TYPE_BindDirectionRight:	//　　　コーナーステープル：右上.
							us_textID = s_pKEYStapleTopRightNoPunch_W156H040A;
							break;
#else // #ifdef DEF_LCD_7INCH.
						case TYPE_BindDirectionTop:	//　　　コーナーステープル：左上.
						case TYPE_BindDirectionLeft:	//　　　コーナーステープル：左上.
							us_textID = s_pKEYStapleTopLeftNoPunch_W136H036A;
							break;
						case TYPE_BindDirectionRight:	//　　　コーナーステープル：右上.
							us_textID = s_pKEYStapleTopRightNoPunch_W136H036A;
							break;
#endif // #ifdef DEF_LCD_7INCH.
						default:
							break;
					}
				}
				break;
			case TYPE_StapleModeNone:	// パンチ_2穴/4穴キーのみ選択時.
				switch( c_punchPosition ) {
#ifdef DEF_LCD_7INCH
					case TYPE_BindDirectionTop:		//　　　パンチ：上　
						us_textID = s_pKEYNoStaplePunchTop_W156H040A;
						break;
					case TYPE_BindDirectionLeft:	//　　　パンチ：左
						us_textID = s_pKEYNoStaplePunchLeft_W156H040A;
						break;
					case TYPE_BindDirectionRight:	//　　　パンチ：右
						us_textID = s_pKEYNoStaplePunchRight_W156H040A;
						break;
#else // #ifdef DEF_LCD_7INCH.
					case TYPE_BindDirectionTop:		//　　　パンチ：上　
						us_textID = s_pKEYNoStaplePunchTop_W136H036A;
						break;
					case TYPE_BindDirectionLeft:	//　　　パンチ：左
						us_textID = s_pKEYNoStaplePunchLeft_W136H036A;
						break;
					case TYPE_BindDirectionRight:	//　　　パンチ：右
						us_textID = s_pKEYNoStaplePunchRight_W136H036A;
						break;
#endif // #ifdef DEF_LCD_7INCH.
					default:
						break;
				}
			default:
				break;
		}
	}
	Key_ActSettingChange_actStapleAndPunchPosition.changeText( &Ktx_ActSettingChange_actStapleAndPunchPosition_Sub, us_textID );		// 表示テキスト「ステープル」モード.
#endif //DEF_A4_Cus.
// A3A4FUM so_okada 2015.02.12 End.

//	}
// ▲ 2010/04/12 AWS_フェクス A070800100 UT Bug No:13311.
}

//関数------------------------------------------------------------------------------------------------------------------
//概要    ：各部品表示／非表示.
//備考    ：.
//戻値    ：.
//作成者  ：AWS_クリストファー.
//レビュー：.
//変更履歴：.
//------------------------------------------------------------------------------
/**
* @brief A070800100 分割読込み停止中／設定変更メニュー : 各部品表示／非表示
*/
//------------------------------------------------------------------------------
void PCPC_ActSettingChange_act::showPatternCustomize()
{
	PNLE_APPMODE_E e_appMode = PnlComGetAppMode();			// アプリ種別を取得.
#ifndef DEF_A4_Cus
	APIC_PrinterSystem* pc_PrinterSystem = PnlComGetPrinterSystem();
	TYPC_Print* pc_Print = comPanelJob->getJobSpec()->pc_Print;
// 伊丹Sky(平野)　2010/08/31 動作中-FVT3分析&弱点強化対応 No.148 ↑.
#endif
	uchar baseCnt = 4;		// グループのパーツ数
	uchar appliedCnt = 4;	// グループのパーツ数

	if( PNLE_APPMODE_COPY == e_appMode ) {
		// コピーモード.
		// キーの表示状態
		// 倍率.
		// 原稿サイズ.
// Comment VenusMLK -> 2017/03/22 SCD2 3G Sky瀬戸 [AccuRev.1017581]分割読込み停止中の設定変更では原稿サイズ設定を表示させない.
#ifdef DEF_A4_Cus
		Bool b_OrgSize = False;
#else // DEF_A4_Cus.
		Bool b_OrgSize = True;
		if( TYPE_OriginalSizeModeMix != comPanelJob->getJobSpec()->pc_Scan->c_OriginalSize.getOriginalSizeMode() ) {
			b_OrgSize = False;
		}
#endif // DEF_A4_Cus.
// Comment VenusMLK <- 2017/03/22 SCD2 3G Sky瀬戸 [AccuRev.1017581]分割読込み停止中の設定変更では原稿サイズ設定を表示させない.
		// とじしろ位置.
		Bool b_PageMargin = True;
		if( False == comPanelJob->getJobSpec()->pc_Common->getVerifyPreviewOn() ||	//コピーの仕上りプレビュー設定時.
		    TYPE_ReqImgTypeCopy != PnlComGetVerifyPreviewControl()->getReqVerifyImgType() ) {
			b_PageMargin = False;
		}

#ifndef DEF_A4_Cus
		// ステープル/パンチ位置.
		Bool b_StaplePanch = False;
		TYPE_StapleMode c_stapleMode =  pc_Print->c_Finishing.getStapleMode();
		TYPE_PunchMode c_punchMode = pc_Print->c_Finishing.getPunchMode();
		Bool b_CenterStaple = False;
		if( ( TYPE_FoldModeCenterInner == pc_Print->c_Finishing.getFoldMode() ||
		      TYPE_FoldModeCenterOuter == pc_Print->c_Finishing.getFoldMode() ) &&
		    TYPE_StapleModeCenter == pc_Print->c_Finishing.getStapleMode() ) {
			b_CenterStaple = True;
		}
		if( b_PageMargin == True &&
		    ( True == pc_PrinterSystem->isExistFinisher() &&
		      ( ( TYPE_StapleModeNone != c_stapleMode )||
		        ( TYPE_PunchModeNone != c_punchMode ) ) ) &&
		    False == b_CenterStaple ) {
			b_StaplePanch = True;
		}
#endif


		// 倍率.
		if( PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_Zoom ) == False ) {		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
			Key_ActSettingChange_actZoom.showEnable( False );
			baseCnt--;
		}
		else {
			// 固定倍率の判定.

			if( PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_Zoom_Manual ) == False		&& // 独立ズーム.		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
			    PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_Zoom_FullSize ) == False	&& // 等倍.		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
			    PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_Zoom_FixedZoom ) == False	&&		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
			    PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_Zoom_FixedZoom_UserPreset ) == False ) {		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
				Key_ActSettingChange_actZoom.showEnable( False );
				baseCnt--;
			}
		}

		// 原稿.
		if( PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_DuplexCombine )        == False ||		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
		    PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_DuplexCombine_Duplex ) == False ||		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
		    (	PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_DuplexCombine_Duplex_1To1 ) == False &&		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
		        PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_DuplexCombine_Duplex_1To2 ) == False &&		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
		        PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_DuplexCombine_Duplex_2To1 ) == False &&		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
		        PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_DuplexCombine_Duplex_2To2 ) == False ) ) {		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
			Key_ActSettingChange_actManuscriptSetting.showEnable( False );
			Key_ActSettingChange_actManuscriptSetting_Copy.showEnable( False );		// 「印刷」キー.
			Key_ActSettingChange_actManuscriptSetting_OneSide.showEnable( False );		// 「印刷」キー.
			Key_ActSettingChange_actManuscriptSetting_TwoSide.showEnable( False );		// 「印刷」キー.
			baseCnt--;
		}
		// とじしろ位置.
		if( PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_PageMargin ) == False ||		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
		    (	PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_PageMargin_Auto ) == False &&		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
		        PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_PageMargin_Left ) == False &&		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
		        PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_PageMargin_Top ) == False &&		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
		        PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_PageMargin_Right ) == False ) ||		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
		    b_PageMargin == False ) {
			Key_ActSettingChange_actCloseAndCarryOutPositionSetting.showEnable( False );
			baseCnt--;
		}
// A3A4FUM so_okada 2015.02.12 Start.
#ifdef DEF_A4_Cus
		baseCnt--;
#else
		// ステープル/パンチ位置.
		if( PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_Finishing )                       == False ||		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
		    PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_Finishing_Punch_PositionSetting ) == False ||		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
		    b_StaplePanch == False ) {
			Key_ActSettingChange_actStapleAndPunchPosition.showEnable( False );
			baseCnt--;
		}
#endif //DEF_A4_Cus.
// A3A4FUM so_okada 2015.02.12 End.
		// 原稿サイズ.
		if( PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_OrgSet )              == False ||		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
		    PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_OrgSet_OriginalSize ) == False ||		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
		    b_OrgSize == False ) {
// Comment VenusMLK -> 2017/03/22 SCD2 3G Sky瀬戸 [AccuRev.1017581]分割読込み停止中の設定変更では原稿サイズ設定を表示させない.
#ifndef DEF_A4_Cus
			Key_ActSettingChange_actOriginalSize.showEnable( False );
#endif // DEF_A4_Cus.
// Comment VenusMLK <- 2017/03/22 SCD2 3G Sky瀬戸 [AccuRev.1017581]分割読込み停止中の設定変更では原稿サイズ設定を表示させない.
			appliedCnt--;
		}
		// 原稿のとじしろ.
		if( PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_OrgSet )                 == False ||		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
		    PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_OrgSet_BindingPosition ) == False ) {		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
			Key_ActSettingChange_actManuscriptCloseAndCarryOut.showEnable( False );
			appliedCnt--;
		}
		// 枠消し.
		if( PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_FrameErase )         == False ) {		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
			Key_ActSettingChange_actFrameErase.showEnable( False );
			appliedCnt--;
		}
		// 折り目消し.
		if( PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_OrgBook )             == False ||		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
		    PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_OrgBook_CenterErase ) == False ) {		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
			Key_ActSettingChange_actCenterErase.showEnable( False );
			appliedCnt--;
		}
// comment 2016/1/26 MSE岡田 [ZeusS_ZX0][CQ524019] ボックス時にもパターンカスタマイズによる表示/非表示を行うようにする.
//	} else if( PNLE_APPMODE_SCANFAX == e_appMode ) {
	} else if( ( PNLE_APPMODE_SCANFAX == e_appMode )
			|| ( PNLE_APPMODE_BOX == e_appMode ) ) {
		// ファクス/スキャンモード.
		// キーの表示状態
		// 倍率.
		// 原稿サイズ.
// Comment VenusMLK -> 2017/03/22 SCD2 3G Sky瀬戸 [AccuRev.1017581]分割読込み停止中の設定変更では原稿サイズ設定を表示させない.
#ifdef DEF_A4_Cus
		Bool b_OrgSize = False;
#else // DEF_A4_Cus.
		Bool b_OrgSize = True;
		if( TYPE_OriginalSizeModeMix != comPanelJob->getJobSpec()->pc_Scan->c_OriginalSize.getOriginalSizeMode() ) {
			b_OrgSize = False;
		}
#endif // DEF_A4_Cus.
// Comment VenusMLK <- 2017/03/22 SCD2 3G Sky瀬戸 [AccuRev.1017581]分割読込み停止中の設定変更では原稿サイズ設定を表示させない.


		// 原稿.
		if( PnlComGetFunctionShowFaxScan( TYPE_PNLMENU_FAXSCAN_DuplexSet ) == False ||		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
		    (	PnlComGetFunctionShowFaxScan( TYPE_PNLMENU_FAXSCAN_DuplexSet_1Sided )      == False &&		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
		        PnlComGetFunctionShowFaxScan( TYPE_PNLMENU_FAXSCAN_DuplexSet_2Sided )      == False &&		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
		        PnlComGetFunctionShowFaxScan( TYPE_PNLMENU_FAXSCAN_DuplexSet_2SidedCover ) == False ) ) {		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
			Key_ActSettingChange_actManuscriptSetting.showEnable( False );
			Key_ActSettingChange_actManuscriptSetting_Copy.showEnable( False );		// 「印刷」キー.
			Key_ActSettingChange_actManuscriptSetting_OneSide.showEnable( False );		// 「印刷」キー.
			Key_ActSettingChange_actManuscriptSetting_TwoSide.showEnable( False );		// 「印刷」キー.
			baseCnt--;
		}
		// とじしろ位置.
		Key_ActSettingChange_actCloseAndCarryOutPositionSetting.showEnable( False );
		baseCnt--;
// A3A4FUM so_okada 2015.02.12 Start.
#ifdef DEF_A4_Cus
		baseCnt--;
#else
		// ステープル/パンチ位置.
		Key_ActSettingChange_actStapleAndPunchPosition.showEnable( False );
		baseCnt--;
#endif //DEF_A4_Cus.
// A3A4FUM so_okada 2015.02.12 End.
		// 原稿サイズ.
		if( PnlComGetFunctionShowFaxScan( TYPE_PNLMENU_FAXSCAN_ScanSize ) == False ||		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
		    b_OrgSize == False ) {
// Comment VenusMLK -> 2017/03/22 SCD2 3G Sky瀬戸 [AccuRev.1017581]分割読込み停止中の設定変更では原稿サイズ設定を表示させない.
#ifndef DEF_A4_Cus
			Key_ActSettingChange_actOriginalSize.showEnable( False );
#endif // DEF_A4_Cus.
// Comment VenusMLK <- 2017/03/22 SCD2 3G Sky瀬戸 [AccuRev.1017581]分割読込み停止中の設定変更では原稿サイズ設定を表示させない.
			appliedCnt--;
		}
		// 原稿のとじしろ.
		if( PnlComGetFunctionShowFaxScan( TYPE_PNLMENU_FAXSCAN_OrgSet ) == False ||		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
		    PnlComGetFunctionShowFaxScan( TYPE_PNLMENU_FAXSCAN_OrgSet_BindingPosition ) == False ) {		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
			Key_ActSettingChange_actManuscriptCloseAndCarryOut.showEnable( False );
			appliedCnt--;
		}
		// 枠消し.
// comment 2019/03/13 Sky飛田 【制御構成変更_No.66】 仕上がりプレビュー対応.
//		if( PnlComGetFunctionShowFaxScan( TYPE_PNLMENU_FAXSCAN_FrameErase )         == False ) {		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
		if( ( PnlComGetFunctionShowFaxScan( TYPE_PNLMENU_FAXSCAN_FrameErase )         == False ) ||
			( ( True == PNLC_WebUIControl::getInstance()->judgeWebUIJobRunning() ) && ( PNLC_WebUIControl::getInstance()->getWebUIJobID() == comPanelJob->getJobStatus()->getJobID() ) ) ){		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
			Key_ActSettingChange_actFrameErase.showEnable( False );
			appliedCnt--;
		}
		// 折り目消し.
// comment 2019/03/13 Sky飛田 【制御構成変更_No.66】 仕上がりプレビュー対応.
//		if( PnlComGetFunctionShowFaxScan( TYPE_PNLMENU_FAXSCAN_OrgBook )             == False ||		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
//		    PnlComGetFunctionShowFaxScan( TYPE_PNLMENU_FAXSCAN_OrgBook_CenterErase ) == False ) {		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
		if( ( PnlComGetFunctionShowFaxScan( TYPE_PNLMENU_FAXSCAN_OrgBook )             == False ) ||		
			( PnlComGetFunctionShowFaxScan( TYPE_PNLMENU_FAXSCAN_OrgBook_CenterErase ) == False ) ||
			( ( True == PNLC_WebUIControl::getInstance()->judgeWebUIJobRunning() ) && ( PNLC_WebUIControl::getInstance()->getWebUIJobID() == comPanelJob->getJobStatus()->getJobID() ) ) ){		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
			Key_ActSettingChange_actCenterErase.showEnable( False );
			appliedCnt--;
		}
	}

	// グループ背景の非表示
	if( baseCnt <= 0 ) {
		Bak_SetChangeStampMenu_actBasicSetup.showEnable( False );
		Txt_ActSettingChange_actBasicSettings.showEnable( False );
	}
	if( appliedCnt <= 0 ) {
		Bak_SetChangeStampMenu_actAppSetup.showEnable( False );
		Txt_ActSettingChange_actAppSettings.showEnable( False );
	}
}

//関数------------------------------------------------------------------------------------------------------------------
//概要    ：.
//備考    ：.
//戻値    ：.
//作成者  ：.
//レビュー：.
//変更履歴：.
//------------------------------------------------------------------------------
/**
* @brief A070800100 とじしろ方向ボタン情報の初期化.
*/
//------------------------------------------------------------------------------
void PCPC_ActSettingChange_act::initBindPositionInfo()
{
	uchar uc_Cnt = 0;
	uchar uc_i   = 0;

	// ベーステーブルからのコピー
	for( int i = 0; i < PNLD_BindPosition_MaxKindCnt; i++ ) {
		s_mBindPosition_Info[i].b_Show         = s_BindPosition_Info[i].b_Show;
		s_mBindPosition_Info[i].uc_ButtonIndex = s_BindPosition_Info[i].uc_ButtonIndex;
		s_mBindPosition_Info[i].e_Kind         = s_BindPosition_Info[i].e_Kind;
		s_mBindPosition_Info[i].us_KindTxt     = s_BindPosition_Info[i].us_KindTxt;
	}

	// ボタンのクリアと表示・非表示設定.
	for( uc_i = 0; uc_i < PNLD_BindPosition_MaxKindCnt; uc_i++ ) {
		s_BindPosition_PullDownButtonInfo[uc_i].pc_PullButton->changeText( s_mBindPosition_Info[uc_i].us_KindTxt );
		s_BindPosition_PullDownButtonInfo[uc_i].pc_PullButton->activate( False );
		s_BindPosition_PullDownButtonInfo[uc_i].pc_PullButton->showEnable( False );
		s_mBindPosition_Info[uc_i].uc_ButtonIndex = 0;
		us_mBindPosition_Txt[uc_i] = 0;	// とじしろ方向の表示窓用テキスト初期化.

		switch( s_mBindPosition_Info[uc_i].e_Kind ) {
			case TYPE_BindDirectionAuto:
			case TYPE_BindDirectionLeft:
			case TYPE_BindDirectionTop:
			case TYPE_BindDirectionRight:
				s_mBindPosition_Info[uc_i].b_Show = isEnableBindPosition( s_mBindPosition_Info[uc_i].e_Kind );
				break;
			default:
				break;
		}
	}

	// 表示可能なとじしろ方向を設定.
	for( uc_i = 0; uc_i < PNLD_BindPosition_MaxKindCnt; uc_i++ ) {
		if( s_mBindPosition_Info[uc_i].b_Show ) {
			s_BindPosition_PullDownButtonInfo[uc_Cnt].pc_PullButton->changeText( s_mBindPosition_Info[uc_i].us_KindTxt );
			s_BindPosition_PullDownButtonInfo[uc_Cnt].pc_PullButton->activate( True );
			s_BindPosition_PullDownButtonInfo[uc_Cnt].pc_PullButton->showEnable( True );
			s_mBindPosition_Info[uc_i].uc_ButtonIndex = static_cast<uchar>( uc_Cnt + 1 );
			us_mBindPosition_Txt[uc_Cnt] = s_mBindPosition_Info[uc_i].us_KindTxt;	// 表示窓用テキストを変更.
			uc_Cnt++;
		}
	}

	// とじしろ方向プルダウンメニューに有効ボタン数を設定する.
	p_ManuscriptCloseAndCarryOutPositionSetting->setEnableItemCnt( uc_Cnt );
	// とじしろ方向プルダウンメニューの行数変更.
	p_ManuscriptCloseAndCarryOutPositionSetting->changePullDownRow( uc_Cnt );

	// ジョブスペックからとじしろ方向を取得.
	TYPC_Print* pc_Print = comPanelJob->getJobSpec()->pc_Print;
	uc_mSelectKind = pc_Print->c_BindDirection.getBindDirection();
	for( uc_i = 0; uc_i < PNLD_BindPosition_MaxKindCnt; uc_i++ ) {
		if( uc_mSelectKind == s_mBindPosition_Info[uc_i].e_Kind ) {
			e_mSelectIndex = s_mBindPosition_Info[uc_i].uc_ButtonIndex;
			break;
		}
	}

	// インデックスが1未満の場合は.
	// 対象がプルダウンメニューから外れた事になるので初期状態に設定する.
	if( e_mSelectIndex < 1 ) {
		e_mSelectIndex = 1;
		uc_mSelectKind = 0;
	}

	// ドロップダウンボタン・プルダウンメニューの設定.
	Key_ActSettingChange_actCloseAndCarryOutPositionSetting.setSelectNo( e_mSelectIndex );
	p_ManuscriptCloseAndCarryOutPositionSetting->setSelectMenuNo( e_mSelectIndex );
}

//------------------------------------------------------------------------------
/**
* @brief A070800100 選択中とじしろ方向ボタン設定.
*/
//------------------------------------------------------------------------------
void PCPC_ActSettingChange_act::setSelectPullDown()
{
	uchar uc_i = 0;

	// 選択中のとじしろ方向からインデックスを取得.
	for( uc_i = 0; uc_i < PNLD_BindPosition_MaxKindCnt; uc_i++ ) {
		if( uc_mSelectKind == s_mBindPosition_Info[uc_i].e_Kind ) {
			e_mSelectIndex = s_mBindPosition_Info[uc_i].uc_ButtonIndex;
			break;
		}
	}

	// インデックスが1未満の場合は.
	// 対象がプルダウンメニューから外れた事になるので初期状態に設定する.
	if( e_mSelectIndex < 1 ) {
		e_mSelectIndex = 1;
		uc_mSelectKind = 0;
	}

	// ドロップダウンボタン・プルダウンメニューの設定.
	Key_ActSettingChange_actCloseAndCarryOutPositionSetting.setSelectNo( e_mSelectIndex );
	p_ManuscriptCloseAndCarryOutPositionSetting->setSelectMenuNo( e_mSelectIndex );
}

//------------------------------------------------------------------------------
/**
* @brief A070800100 選択したボタンのオプション値(とじしろ方向)を取得.
*/
//------------------------------------------------------------------------------
uchar PCPC_ActSettingChange_act::getBindPositionKindId( uchar e_index )
{
	uchar uc_i = 0;
	uchar e_retKind = TYPE_BindDirectionAuto;

	for( uc_i = 0; uc_i < PNLD_BindPosition_MaxKindCnt; uc_i++ ) {
		if( s_mBindPosition_Info[uc_i].uc_ButtonIndex == e_index ) {
			e_retKind = s_mBindPosition_Info[uc_i].e_Kind;
			break;
		}
	}
	return e_retKind;
}

//------------------------------------------------------------------------------
/**
* @brief A070800100 指定したとじしろ方向の使用可否.
*/
//------------------------------------------------------------------------------
Bool PCPC_ActSettingChange_act::isEnableBindPosition( uchar e_Kind )
{
	Bool b_Enable = True;

	// パターンカスタマイズチェック
	if( PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_PageMargin ) == False ) {		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
		b_Enable = False;
	} else {
		switch( e_Kind ) {
			case TYPE_BindDirectionAuto:
				b_Enable = PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_PageMargin_Auto );		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
				break;
			case TYPE_BindDirectionLeft:
				b_Enable = PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_PageMargin_Left );		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
				break;
			case TYPE_BindDirectionTop:
				b_Enable = PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_PageMargin_Top );		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
				break;
			case TYPE_BindDirectionRight:
				b_Enable = PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_PageMargin_Right );		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
				break;
		}
	}

	return b_Enable;
}

//------------------------------------------------------------------------------
/**
* @brief A070800100 とじしろ方向プルダウンのコールバック処理.
*/
//------------------------------------------------------------------------------
void PCPC_ActSettingChange_act::ans_BindPositionPullDownEnd( uchar uc_Param, short ss_Param, void* p_Param, uint32_t ul_Param )
{
	TYPC_Print* pc_Print = comPanelJob->getJobSpec()->pc_Print;
	if( NULL == pc_Print ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }
	// インデックスを保持.
	pc_mActSettingChange_act->e_mSelectIndex = uc_Param;
	// インデックスに応じたオプション値を設定.
	pc_mActSettingChange_act->uc_mSelectKind = pc_mActSettingChange_act->getBindPositionKindId( pc_mActSettingChange_act->e_mSelectIndex );

	// とじしろ方向を設定.
	pc_Print->c_BindDirection.setBindDirection( pc_mActSettingChange_act->uc_mSelectKind );

}
//関数------------------------------------------------------------------------------------------------------------------
//概要    ：.
//備考    ：.
//戻値    ：.
//作成者  ：.
//レビュー：.
//変更履歴：.
//------------------------------------------------------------------------------
/**
* @brief A070800100 原稿ボタン情報の初期化.
*/
//------------------------------------------------------------------------------
void PCPC_ActSettingChange_act::initManuscriptInfo()
{
	uchar uc_Cnt = 0;
	uchar uc_i   = 0;

	// ベーステーブルからのコピー
	for( int i = 0; i < PNLD_ManuscriptSettings_MaxCnt; i++ ) {
		s_mManuscript_Info[i].b_Show         = s_ManuscriptSettings_Info[i].b_Show;
		s_mManuscript_Info[i].uc_ButtonIndex = s_ManuscriptSettings_Info[i].uc_ButtonIndex;
		s_mManuscript_Info[i].e_Kind         = s_ManuscriptSettings_Info[i].e_Kind;
		s_mManuscript_Info[i].us_KindTxt     = s_ManuscriptSettings_Info[i].us_KindTxt;
	}

	// ボタンのクリアと表示・非表示設定.
	for( uc_i = 0; uc_i < PNLD_ManuscriptSettings_MaxCnt; uc_i++ ) {
		s_ManuscriptSettings_PullDownButtonInfo[uc_i].pc_PullButton->changeText( s_mManuscript_Info[uc_i].us_KindTxt );
		s_ManuscriptSettings_PullDownButtonInfo[uc_i].pc_PullButton->activate( False );
		s_ManuscriptSettings_PullDownButtonInfo[uc_i].pc_PullButton->showEnable( False );
		s_mManuscript_Info[uc_i].uc_ButtonIndex = 0;
		us_mManuscript_Txt[uc_i] = 0;	// 原稿の表示窓用テキスト初期化.

		switch( s_mManuscript_Info[uc_i].e_Kind ) {
			case PNLD_ManuscriptOneSide:
			case PNLD_ManuscriptTwoSide:
			case PNLD_ManuscriptCoverTwoSide:
				s_mManuscript_Info[uc_i].b_Show = isEnableManuscript( s_mManuscript_Info[uc_i].e_Kind );
				break;
			default:
				break;
		}
	}

	// 表示可能な原稿を設定.
	for( uc_i = 0; uc_i < PNLD_ManuscriptSettings_MaxCnt; uc_i++ ) {
		if( s_mManuscript_Info[uc_i].b_Show ) {
			s_ManuscriptSettings_PullDownButtonInfo[uc_Cnt].pc_PullButton->changeText( s_mManuscript_Info[uc_i].us_KindTxt );
			s_ManuscriptSettings_PullDownButtonInfo[uc_Cnt].pc_PullButton->activate( True );
			s_ManuscriptSettings_PullDownButtonInfo[uc_Cnt].pc_PullButton->showEnable( True );
			s_mManuscript_Info[uc_i].uc_ButtonIndex = static_cast<uchar>( uc_Cnt + 1 );
			us_mManuscript_Txt[uc_Cnt] = s_mManuscript_Info[uc_i].us_KindTxt;	// 表示窓用テキストを変更.
			uc_Cnt++;
		}
	}

	// 原稿プルダウンメニューに有効ボタン数を設定する.
	p_ManuscriptSetting->setEnableItemCnt( uc_Cnt );
	// 原稿プルダウンメニューの行数変更.
	p_ManuscriptSetting->changePullDownRow( uc_Cnt );

	// ジョブスペックから原稿設定を取得.
	Bool b_Duplex = comPanelJob->getJobSpec()->pc_Scan->getDuplex();
	Bool b_CoverAndDuplex = comPanelJob->getJobSpec()->pc_Scan->getCoverAndDuplex();
	if( False == b_Duplex && False == b_CoverAndDuplex ) {
		uc_mManuscriptSelectKind = PNLD_ManuscriptOneSide;
	} else if( b_CoverAndDuplex == False ) {
		uc_mManuscriptSelectKind = PNLD_ManuscriptTwoSide;
	} else {
		uc_mManuscriptSelectKind = PNLD_ManuscriptCoverTwoSide;
	}

	for( uc_i = 0; uc_i < PNLD_ManuscriptSettings_MaxCnt; uc_i++ ) {
		if( uc_mManuscriptSelectKind == s_mManuscript_Info[uc_i].e_Kind ) {
			e_mManuscriptSelectIndex = s_mManuscript_Info[uc_i].uc_ButtonIndex;
			break;
		}
	}

	// インデックスが1未満の場合は.
	// 対象がプルダウンメニューから外れた事になるので初期状態に設定する.
	if( e_mManuscriptSelectIndex < 1 ) {
		e_mManuscriptSelectIndex = 1;
		uc_mManuscriptSelectKind = 0;
	}

	// ドロップダウンボタン・プルダウンメニューの設定.
	Key_ActSettingChange_actManuscriptSetting.setSelectNo( e_mManuscriptSelectIndex );
	p_ManuscriptSetting->setSelectMenuNo( e_mManuscriptSelectIndex );
}

//------------------------------------------------------------------------------
/**
* @brief A070800100 選択したボタンのオプション値を取得.
*/
//------------------------------------------------------------------------------
uchar PCPC_ActSettingChange_act::getManuscriptKindId( uchar e_index )
{
	uchar uc_i = 0;
	uchar e_retKind = PNLD_ManuscriptOneSide;

	for( uc_i = 0; uc_i < PNLD_ManuscriptSettings_MaxCnt; uc_i++ ) {
		if( s_mManuscript_Info[uc_i].uc_ButtonIndex == e_index ) {
			e_retKind = s_mManuscript_Info[uc_i].e_Kind;
			break;
		}
	}
	return e_retKind;
}

//------------------------------------------------------------------------------
/**
* @brief A070800100 指定した原稿設定の使用可否.
*/
//------------------------------------------------------------------------------
Bool PCPC_ActSettingChange_act::isEnableManuscript( uchar e_Kind )
{
	Bool b_Enable = True;

	if( PNLE_APPMODE_COPY == PnlComGetAppMode() ) {
		if( PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_DuplexCombine )        == False ||		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
		    PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_DuplexCombine_Duplex ) == False ) {		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
			b_Enable = False;
		} else {
			switch( e_Kind ) {
				case PNLD_ManuscriptOneSide:
					if( PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_DuplexCombine_Duplex_1To1 ) == False &&		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
					    PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_DuplexCombine_Duplex_1To2 ) == False ) {		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
						b_Enable = False;
					}
					break;
				case PNLD_ManuscriptTwoSide:
					if( PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_DuplexCombine_Duplex_2To1 ) == False &&		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
					    PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_DuplexCombine_Duplex_2To2 ) == False ) {		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
						b_Enable = False;
					}
					break;
				default:
					b_Enable = False;
					break;
			}
		}
// comment 2016/1/26 MSE岡田 [ZeusS_ZX0][CQ524019] ボックス時にもパターンカスタマイズによる表示/非表示を行うようにする.
//	} else if( PNLE_APPMODE_SCANFAX == PnlComGetAppMode() ) {
	} else if( ( PNLE_APPMODE_SCANFAX == PnlComGetAppMode() )
			|| ( PNLE_APPMODE_BOX == PnlComGetAppMode() ) ) {
		if( PnlComGetFunctionShowFaxScan( TYPE_PNLMENU_FAXSCAN_DuplexSet ) == False ) {		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
			b_Enable = False;
		} else {
			switch( e_Kind ) {
				case PNLD_ManuscriptOneSide:
					b_Enable = PnlComGetFunctionShowFaxScan( TYPE_PNLMENU_FAXSCAN_DuplexSet_1Sided );		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
					break;
				case PNLD_ManuscriptTwoSide:
					b_Enable = PnlComGetFunctionShowFaxScan( TYPE_PNLMENU_FAXSCAN_DuplexSet_2Sided );		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
					break;
				case PNLD_ManuscriptCoverTwoSide:
					b_Enable = PnlComGetFunctionShowFaxScan( TYPE_PNLMENU_FAXSCAN_DuplexSet_2SidedCover );		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
// comment 2016/1/26 MSE岡田 [ZeusS_ZX0][CQ524019] 「片面」または「両面」の場合、原稿プルダウンに「表紙+両面」を表示しないようにする。defaultケースを追加.
					if( True == b_Enable ) {
						Bool b_CoverAndDuplex = comPanelJob->getJobSpec()->pc_Scan->getCoverAndDuplex();
						if( False == b_CoverAndDuplex ) {	// 片面または両面.
							b_Enable = False;
						}
					}
					break;
				default:
					b_Enable = False;
					break;
			}
		}
	}
	return b_Enable;
}

//------------------------------------------------------------------------------
/**
* @brief A070800100 原稿プルダウンのコールバック処理.
*/
//------------------------------------------------------------------------------
void PCPC_ActSettingChange_act::ans_ManuscriptPullDownEnd( uchar uc_Param, short ss_Param, void* p_Param, uint32_t ul_Param )
{
	TYPC_Print* pc_Print = comPanelJob->getJobSpec()->pc_Print;
	if( NULL == pc_Print ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }
	// インデックスを保持.
	pc_mActSettingChange_act->e_mManuscriptSelectIndex = uc_Param;
	// インデックスに応じたオプション値を設定.
	pc_mActSettingChange_act->uc_mManuscriptSelectKind = pc_mActSettingChange_act->getManuscriptKindId( pc_mActSettingChange_act->e_mManuscriptSelectIndex );
	// ボタンの選択名を変更.

	// 原稿を設定.
	// オプション値によって処理分岐.
	switch( pc_mActSettingChange_act->uc_mManuscriptSelectKind ) {
		case PNLD_ManuscriptOneSide:
			comPanelJob->getJobSpec()->pc_Scan->setDuplex( False );
			comPanelJob->getJobSpec()->pc_Scan->setCoverAndDuplex( False );
			break;
		case PNLD_ManuscriptTwoSide:
			comPanelJob->getJobSpec()->pc_Scan->setDuplex( True );
			comPanelJob->getJobSpec()->pc_Scan->setCoverAndDuplex( False );
			break;
		case PNLD_ManuscriptCoverTwoSide:
			comPanelJob->getJobSpec()->pc_Scan->setDuplex( True );
			comPanelJob->getJobSpec()->pc_Scan->setCoverAndDuplex( True );
			break;
	}

}
// comment 2019/03/13 Sky飛田 【制御構成変更_No.66】 仕上がりプレビュー対応.
//関数------------------------------------------------------------------------------------------------------------------
//概要    ：.
//備考    ：.
//戻値    ：.
//作成者  ：.
//レビュー：.
//変更履歴：.
//------------------------------------------------------------------------------
/**
* @brief A070800100 原稿のとじしろボタン情報の初期化.
*/
//------------------------------------------------------------------------------
void PCPC_ActSettingChange_act::initManuscriptCloseAndCarryOut()
{
	uchar uc_Cnt = 0;
	uchar uc_i   = 0;

	// ベーステーブルからのコピー
	for( int i = 0; i < PNLD_BindPosition_MaxKindCnt; i++ ) {
		s_mBindPositionCloseAndCarryOut_Info[i].b_Show         = s_BindPositionCloseAndCarryOut_Info[i].b_Show;
		s_mBindPositionCloseAndCarryOut_Info[i].uc_ButtonIndex = s_BindPositionCloseAndCarryOut_Info[i].uc_ButtonIndex;
		s_mBindPositionCloseAndCarryOut_Info[i].e_Kind         = s_BindPositionCloseAndCarryOut_Info[i].e_Kind;
		s_mBindPositionCloseAndCarryOut_Info[i].us_KindTxt     = s_BindPositionCloseAndCarryOut_Info[i].us_KindTxt;
	}

	// ボタンのクリアと表示・非表示設定.
	for( uc_i = 0; uc_i < PNLD_BindPosition_MaxKindCnt; uc_i++ ) {
		s_BindPositionCloseAndCarryOut_PullDownButtonInfo[uc_i].pc_PullButton->changeText( s_mBindPositionCloseAndCarryOut_Info[uc_i].us_KindTxt );
		s_BindPositionCloseAndCarryOut_PullDownButtonInfo[uc_i].pc_PullButton->activate( False );
		s_BindPositionCloseAndCarryOut_PullDownButtonInfo[uc_i].pc_PullButton->showEnable( False );
		s_mBindPositionCloseAndCarryOut_Info[uc_i].uc_ButtonIndex = 0;
		us_mBindPositionCloseAndCarryOut_Txt[uc_i] = 0;	// とじしろ方向の表示窓用テキスト初期化.

		switch( s_mBindPositionCloseAndCarryOut_Info[uc_i].e_Kind ) {
			case TYPE_BindDirectionAuto:
			case TYPE_BindDirectionLeft:
			case TYPE_BindDirectionTop:
				s_mBindPositionCloseAndCarryOut_Info[uc_i].b_Show = True;
				break;
			case TYPE_BindDirectionRight:
				s_mBindPositionCloseAndCarryOut_Info[uc_i].b_Show = True;
				if( ( True == PNLC_WebUIControl::getInstance()->judgeWebUIJobRunning() )
				 && ( PNLC_WebUIControl::getInstance()->getWebUIJobID() == comPanelJob->getJobStatus()->getJobID() ) ){
						s_mBindPositionCloseAndCarryOut_Info[uc_i].b_Show = False;
				}
				break;
			default:
				break;
		}
	}

	// 表示可能なとじしろ方向を設定.
	for( uc_i = 0; uc_i < PNLD_BindPosition_MaxKindCnt; uc_i++ ) {
		if( s_mBindPositionCloseAndCarryOut_Info[uc_i].b_Show ) {
			s_BindPositionCloseAndCarryOut_PullDownButtonInfo[uc_Cnt].pc_PullButton->changeText( s_mBindPositionCloseAndCarryOut_Info[uc_i].us_KindTxt );
			s_BindPositionCloseAndCarryOut_PullDownButtonInfo[uc_Cnt].pc_PullButton->activate( True );
			s_BindPositionCloseAndCarryOut_PullDownButtonInfo[uc_Cnt].pc_PullButton->showEnable( True );
			s_mBindPositionCloseAndCarryOut_Info[uc_i].uc_ButtonIndex = static_cast<uchar>( uc_Cnt + 1 );
			us_mBindPositionCloseAndCarryOut_Txt[uc_Cnt] = s_mBindPositionCloseAndCarryOut_Info[uc_i].us_KindTxt;	// 表示窓用テキストを変更.
			uc_Cnt++;
		}
	}

	if( NULL != p_ManuscriptCloseAndCarryOut ) { 
		// とじしろ方向プルダウンメニューに有効ボタン数を設定する.
		p_ManuscriptCloseAndCarryOut->setEnableItemCnt( uc_Cnt );
		// とじしろ方向プルダウンメニューの行数変更.
		p_ManuscriptCloseAndCarryOut->changePullDownRow( uc_Cnt );
	}

	// ジョブスペックからとじしろ方向を取得.
	TYPC_Scan* pc_Scan = comPanelJob->getJobSpec()->pc_Scan;
	uc_mBindPositionCloseAndCarryOutSelectKind = pc_Scan->c_BindDirection.getBindDirection();
	for( uc_i = 0; uc_i < PNLD_BindPosition_MaxKindCnt; uc_i++ ) {
		if( uc_mBindPositionCloseAndCarryOutSelectKind == s_mBindPositionCloseAndCarryOut_Info[uc_i].e_Kind ) {
			e_mBindPositionCloseAndCarryOutSelectIndex = s_mBindPositionCloseAndCarryOut_Info[uc_i].uc_ButtonIndex;
			break;
		}
	}

	// インデックスが1未満の場合は.
	// 対象がプルダウンメニューから外れた事になるので初期状態に設定する.
	if( e_mBindPositionCloseAndCarryOutSelectIndex < 1 ) {
		e_mBindPositionCloseAndCarryOutSelectIndex = 1;
		uc_mBindPositionCloseAndCarryOutSelectKind = 0;
	}

	// ドロップダウンボタン・プルダウンメニューの設定.
	Key_ActSettingChange_actManuscriptCloseAndCarryOut.setSelectNo( e_mBindPositionCloseAndCarryOutSelectIndex );
	if( NULL != p_ManuscriptCloseAndCarryOut ) { 
		p_ManuscriptCloseAndCarryOut->setSelectMenuNo( e_mBindPositionCloseAndCarryOutSelectIndex );
	}
}

//------------------------------------------------------------------------------
/**
* @brief A070800100 選択したボタンのオプション値(原稿のとじしろ方向)を取得.
*/
//------------------------------------------------------------------------------
uchar PCPC_ActSettingChange_act::getBindPositionCloseAndCarryOutKindId( uchar e_index )
{
	uchar uc_i = 0;
	uchar e_retKind = TYPE_BindDirectionAuto;

	for( uc_i = 0; uc_i < PNLD_BindPosition_MaxKindCnt; uc_i++ ) {
		if( s_mBindPositionCloseAndCarryOut_Info[uc_i].uc_ButtonIndex == e_index ) {
			e_retKind = s_mBindPositionCloseAndCarryOut_Info[uc_i].e_Kind;
			break;
		}
	}
	return e_retKind;
}

//------------------------------------------------------------------------------
/**
* @brief A070800100 原稿のとじしろプルダウンのコールバック処理.
*/
//------------------------------------------------------------------------------
void PCPC_ActSettingChange_act::ans_BindPositionCloseAndCarryOut( uchar uc_Param, short ss_Param, void* p_Param, uint32_t ul_Param )
{
	TYPC_Scan* pc_Scan = comPanelJob->getJobSpec()->pc_Scan;
	if( NULL == pc_Scan ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }
	if( NULL != pc_mActSettingChange_act ) { 
		// インデックスを保持.
		pc_mActSettingChange_act->e_mBindPositionCloseAndCarryOutSelectIndex = uc_Param;
		// インデックスに応じたオプション値を設定.
		pc_mActSettingChange_act->uc_mBindPositionCloseAndCarryOutSelectKind = pc_mActSettingChange_act->getBindPositionCloseAndCarryOutKindId( pc_mActSettingChange_act->e_mBindPositionCloseAndCarryOutSelectIndex );
//Comment 2019/03/27 [HCLT_Naveena] [OP_BTS-2893] 【IT6_EagleH】 Klocwork Changes ID: 1004574 [NPD.CHECK.MUST] Added Null check for pc_mActSettingChange_act.
//	}
		// とじしろ方向を設定.
		pc_Scan->c_BindDirection.setBindDirection( pc_mActSettingChange_act->uc_mBindPositionCloseAndCarryOutSelectKind );
	}

}

// comment 2019/03/13 Sky飛田 【制御構成変更_No.66】 仕上がりプレビュー対応.
#if 0
// 他のプルダウンと処理を合わせるためメソッド追加.
void PCPC_ActSettingChange_act::initManuscriptCloseAndCarryOut()
{
	TYPC_Scan* pc_Scan = comPanelJob->getJobSpec()->pc_Scan;
	if( NULL == pc_Scan ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return;}

	// 原稿のとじ代の方向の取得とローテーションボタンのインデックス設定.
	TYPE_BindDirection e_BindDirection = pc_Scan->c_BindDirection.getBindDirection();
	uchar uc_SelectPullDownMenu = 0;

	switch( e_BindDirection ) {
		case TYPE_BindDirectionAuto:	uc_SelectPullDownMenu = PNLD_PullDownManuscriptCloseAndCarryOutBindPositionAuto;		break;	// 自動.
		case TYPE_BindDirectionLeft:	uc_SelectPullDownMenu = PNLD_PullDownManuscriptCloseAndCarryOutBindPositionLeft;		break;	// 左.
		case TYPE_BindDirectionRight:	uc_SelectPullDownMenu = PNLD_PullDownManuscriptCloseAndCarryOutBindPositionRight;		break;	// 右.
		case TYPE_BindDirectionTop:		uc_SelectPullDownMenu = PNLD_PullDownManuscriptCloseAndCarryOutBindPositionUp;			break;	// 上.
		default:	LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );	break;
	}
	p_ManuscriptCloseAndCarryOut->setSelectMenuNo( uc_SelectPullDownMenu );
}

void PCPC_ActSettingChange_act::ans_BindPositionCloseAndCarryOut( uchar uc_Param, short ss_Param, void* p_Param, uint32_t ul_Param )
{
	TYPC_Scan* pc_Scan = comPanelJob->getJobSpec()->pc_Scan;
	if( NULL == pc_Scan ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }

	TYPE_BindDirection e_BindDirection = pc_Scan->c_BindDirection.getBindDirection();	// 原稿のとじ代の方向を設定.

	switch( uc_Param ) {
		case PNLD_PullDownManuscriptCloseAndCarryOutBindPositionAuto:	e_BindDirection = TYPE_BindDirectionAuto;	break;	// 自動.
		case PNLD_PullDownManuscriptCloseAndCarryOutBindPositionLeft:	e_BindDirection = TYPE_BindDirectionLeft;	break;	// 左.
		case PNLD_PullDownManuscriptCloseAndCarryOutBindPositionRight:	e_BindDirection = TYPE_BindDirectionRight;	break;	// 右.
		case PNLD_PullDownManuscriptCloseAndCarryOutBindPositionUp:		e_BindDirection = TYPE_BindDirectionTop;	break;	// 上.
		default:	LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );	break;
	}
	pc_Scan->c_BindDirection.setBindDirection( e_BindDirection );
}

void PCPC_ActSettingChange_act::setupSelectedBindPositionType()
{
	TYPC_Scan* pc_Scan = comPanelJob->getJobSpec()->pc_Scan;
	if( NULL == pc_Scan ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return;}

	// 原稿のとじ代の方向の取得とローテーションボタンのインデックス設定.
	TYPE_BindDirection e_BindDirection = pc_Scan->c_BindDirection.getBindDirection();
	switch( e_BindDirection ) {
		case TYPE_BindDirectionAuto:	uc_mSelectBindDirection = PNLD_PullDownManuscriptCloseAndCarryOutBindPositionAuto;		break;	// 自動.
		case TYPE_BindDirectionLeft:	uc_mSelectBindDirection = PNLD_PullDownManuscriptCloseAndCarryOutBindPositionLeft;		break;	// 左.
		case TYPE_BindDirectionRight:	uc_mSelectBindDirection = PNLD_PullDownManuscriptCloseAndCarryOutBindPositionRight;		break;	// 右.
		case TYPE_BindDirectionTop:		uc_mSelectBindDirection = PNLD_PullDownManuscriptCloseAndCarryOutBindPositionUp;		break;	// 上.
		default:	LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );	break;
	}
	Key_ActSettingChange_actManuscriptCloseAndCarryOut.setSelectNo( uc_mSelectBindDirection );
}
#endif

void PCPC_ActSettingChange_act::procSomethingKeyOn( uchar uc_option ) {
	PPopupDecisionButtonCom::procSomethingKeyOn( uc_option );
	if( NULL != comPanelJob
	    && TYPE_JobTypeCopy == comPanelJob->getJobSpec()->pc_Common->c_JobType.getJobType() ) {
		cp_ActiveJobList->extendPrintStart();
	}
}

void PCPC_SetChangeOriginalSizeMetric_act::procSomethingKeyOn( uchar uc_option ) {
	PPopupDecisionButtonCom::procSomethingKeyOn( uc_option );
	if( NULL != comPanelJob
	    && TYPE_JobTypeCopy == comPanelJob->getJobSpec()->pc_Common->c_JobType.getJobType() ) {
		cp_ActiveJobList->extendPrintStart();
	}
}

void PCPC_SetChangeCollectEraseFrameInch_act::procSomethingKeyOn( uchar uc_option ) {
	PPopupDecisionButtonCom::procSomethingKeyOn( uc_option );
	if( NULL != comPanelJob
	    && TYPE_JobTypeCopy == comPanelJob->getJobSpec()->pc_Common->c_JobType.getJobType() ) {
		cp_ActiveJobList->extendPrintStart();
	}
}
