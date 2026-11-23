//クラス-----------------------------------------------------------------------------------------------------------------.
// ファイル : CHOP_A020161000_OHP合紙.
// Version  : PD ver.016.
//-----------------------------------------------------------------------------------------------------------------------.
#include <panel/pddgrap/pddgrap_CLCD/pddgrapinclude_CLCD.h>
#include <panel/pddgrap/pddgrap_CLCD/pddgraptbl.h>
#include <panel/pddtext/pddtext_CLCD/pstr_CLCD.h>
#include <panel/pddtext/pddtext_CLCD/psstr_CLCD.h>
#include <panel/pddtext/pddtext_CLCD/pfstr_CLCD.h>
#include <panel/ppcg/ppcg_CLCD2/ppcgformsettingbutton_CLCD2.h>
#include <panel/pcgcomponent/pcgcomponentg/PSmallMachineComponent.h>
#include <panel/plibh/plibh_PD/PNLC_SelectPaperTray_PD.h>
#include <panel/plibh/plibh_CLCD2/plibh_CLCD2.h>
#include <panel/plibh/plibhg/pdpcomb.h>
#include <panel/plibh/plibh_CLCD2/PNLC_StackManJobSpec.h>
#include <panel/pcgwarn/pcgwarng/pctwarng.h>
#include <panel/pcgwarn/pcgwarn_CLCD2/pctwarn_CLCD2.h>
#include <panel/pcgcopy/pcgcopy_PD/pcgcopy_A020161000.h>
#define SIZE(table)	(sizeof(table) / sizeof(PDisplayParts*))
#define PNLD_Cmp_AuxilPageOHPIns_copy_SmallMachine	const_cast<PDisplayParts**>( tbl_SmallMachineParts ), static_cast<uchar>( ss_SmallMachinePars ), 20, 111
#define PNLD_Key_AuxilPageOHPIns_copy_Tray1	MSG_MSButton, 24, tbl_m30_075_000_X128_Y40, 180, 80, const_cast<PSingleParts**>( tbl_AuxilPageOHPIns_copy_Tray1 ), SIZE(tbl_AuxilPageOHPIns_copy_Tray1), True, PNLE_Shadow_Parts_2
#define PNLD_Ktx_AuxilPageOHPIns_copy_Tray1	s_pNOTTray1_W032H040A, 180, 80, 32, 40, UC_GPARTS_COLOR_BLACK, UC_GPARTS_COLOR_GRAY3, TYPE_LayoutCentering, TYPE_VLayoutMiddle, PNLE_SELECT_FONT_A, 0
#define PNLD_Gra_AuxilPageOHPIns_copy_Tray1_FreeIcon	tbl_i00_275_000_X16_Y16, 180, 80, TYPE_LayoutFree, False, PNLE_Shadow_none, 16, 16
#define PNLD_Ktx_AuxilPageOHPIns_copy_Tray1_Sheet	s_pNOTA4SEF_W068H036A, 180, 80, 68, 36, UC_GPARTS_COLOR_BLACK, UC_GPARTS_COLOR_GRAY3, TYPE_LayoutFree, TYPE_VLayoutMiddle, PNLE_SELECT_FONT_A, 0
#define PNLD_Ktx_AuxilPageOHPIns_copy_Tray1_X	s_pNOTXEdge_W016H016A, 180, 80, 16, 16, UC_GPARTS_COLOR_BLACK, UC_GPARTS_COLOR_GRAY3, TYPE_LayoutFree, TYPE_VLayoutMiddle, PNLE_SELECT_FONT_A, 0
#define PNLD_Ktx_AuxilPageOHPIns_copy_Tray1_Y	s_pNOTXEdge_W016H016A, 180, 80, 16, 16, UC_GPARTS_COLOR_BLACK, UC_GPARTS_COLOR_GRAY3, TYPE_LayoutFree, TYPE_VLayoutMiddle, PNLE_SELECT_FONT_A, 0
#define PNLD_Num_AuxilPageOHPIns_copy_Tray1_X_Num	SF_TEXT_DYNAMIC_ALLOC, 180, 80, 48, 16, UC_GPARTS_COLOR_BLACK, UC_GPARTS_COLOR_GRAY3, "%3.1f", "%2.2f", PNLE_SELECT_FONT_A, TYPE_LayoutFree, TYPE_VLayoutMiddle, 0
#define PNLD_Num_AuxilPageOHPIns_copy_Tray1_Y_Num	SF_TEXT_DYNAMIC_ALLOC, 180, 80, 48, 16, UC_GPARTS_COLOR_BLACK, UC_GPARTS_COLOR_GRAY3, "%3.1f", "%2.2f", PNLE_SELECT_FONT_A, TYPE_LayoutFree, TYPE_VLayoutMiddle, 0
#define PNLD_Gra_AuxilPageOHPIns_copy_Tray1_Icon2	tbl_i00_270_000_X16_Y16, 180, 80, TYPE_LayoutFree, False, PNLE_Shadow_none, 16, 16
#define PNLD_Gra_AuxilPageOHPIns_copy_Tray1_Icon1	tbl_i00_273_000_X16_Y16, 180, 80, TYPE_LayoutFree, False, PNLE_Shadow_none, 16, 16
#define PNLD_Key_AuxilPageOHPIns_copy_Tray2	MSG_MSButton, 19, tbl_m30_075_000_X128_Y40, 180, 128, const_cast<PSingleParts**>( tbl_AuxilPageOHPIns_copy_Tray2 ), SIZE(tbl_AuxilPageOHPIns_copy_Tray2), True, PNLE_Shadow_Parts_2
#define PNLD_Ktx_AuxilPageOHPIns_copy_Tray2	s_pNOTTray2_W032H040A, 180, 128, 32, 40, UC_GPARTS_COLOR_BLACK, UC_GPARTS_COLOR_GRAY3, TYPE_LayoutCentering, TYPE_VLayoutMiddle, PNLE_SELECT_FONT_A, 0
#define PNLD_Gra_AuxilPageOHPIns_copy_Tray2_FreeIcon	tbl_i00_275_000_X16_Y16, 180, 128, TYPE_LayoutFree, False, PNLE_Shadow_none, 16, 16
#define PNLD_Ktx_AuxilPageOHPIns_copy_Tray2_Sheet	s_pNOTA4SEF_W068H036A, 180, 128, 68, 36, UC_GPARTS_COLOR_BLACK, UC_GPARTS_COLOR_GRAY3, TYPE_LayoutFree, TYPE_VLayoutMiddle, PNLE_SELECT_FONT_A, 0
#define PNLD_Ktx_AuxilPageOHPIns_copy_Tray2_X	s_pNOTXEdge_W016H016A, 180, 128, 16, 16, UC_GPARTS_COLOR_BLACK, UC_GPARTS_COLOR_GRAY3, TYPE_LayoutFree, TYPE_VLayoutMiddle, PNLE_SELECT_FONT_A, 0
#define PNLD_Ktx_AuxilPageOHPIns_copy_Tray2_Y	s_pNOTXEdge_W016H016A, 180, 128, 16, 16, UC_GPARTS_COLOR_BLACK, UC_GPARTS_COLOR_GRAY3, TYPE_LayoutFree, TYPE_VLayoutMiddle, PNLE_SELECT_FONT_A, 0
#define PNLD_Num_AuxilPageOHPIns_copy_Tray2_X_Num	SF_TEXT_DYNAMIC_ALLOC, 180, 128, 48, 16, UC_GPARTS_COLOR_BLACK, UC_GPARTS_COLOR_GRAY3, "%3.1f", "%2.2f", PNLE_SELECT_FONT_A, TYPE_LayoutFree, TYPE_VLayoutMiddle, 0
#define PNLD_Num_AuxilPageOHPIns_copy_Tray2_Y_Num	SF_TEXT_DYNAMIC_ALLOC, 180, 128, 48, 16, UC_GPARTS_COLOR_BLACK, UC_GPARTS_COLOR_GRAY3, "%3.1f", "%2.2f", PNLE_SELECT_FONT_A, TYPE_LayoutFree, TYPE_VLayoutMiddle, 0
#define PNLD_Gra_AuxilPageOHPIns_copy_Tray2_Icon2	tbl_i00_270_000_X16_Y16, 180, 128, TYPE_LayoutFree, False, PNLE_Shadow_none, 16, 16
#define PNLD_Gra_AuxilPageOHPIns_copy_Tray2_Icon1	tbl_i00_273_000_X16_Y16, 180, 128, TYPE_LayoutFree, False, PNLE_Shadow_none, 16, 16
#define PNLD_Key_AuxilPageOHPIns_copy_Tray3	MSG_MSButton, 9, tbl_m30_075_000_X128_Y40, 180, 176, const_cast<PSingleParts**>( tbl_AuxilPageOHPIns_copy_Tray3 ), SIZE(tbl_AuxilPageOHPIns_copy_Tray3), True, PNLE_Shadow_Parts_2
#define PNLD_Ktx_AuxilPageOHPIns_copy_Tray3	s_pNOTTray3_W032H040A, 180, 176, 32, 40, UC_GPARTS_COLOR_BLACK, UC_GPARTS_COLOR_GRAY3, TYPE_LayoutCentering, TYPE_VLayoutMiddle, PNLE_SELECT_FONT_A, 0
#define PNLD_Gra_AuxilPageOHPIns_copy_Tray3_FreeIcon	tbl_i00_275_000_X16_Y16, 180, 176, TYPE_LayoutFree, False, PNLE_Shadow_none, 16, 16
#define PNLD_Ktx_AuxilPageOHPIns_copy_Tray3_Sheet	s_pNOTA4SEF_W068H036A, 180, 176, 68, 36, UC_GPARTS_COLOR_BLACK, UC_GPARTS_COLOR_GRAY3, TYPE_LayoutFree, TYPE_VLayoutMiddle, PNLE_SELECT_FONT_A, 0
#define PNLD_Ktx_AuxilPageOHPIns_copy_Tray3_X	s_pNOTXEdge_W016H016A, 180, 176, 16, 16, UC_GPARTS_COLOR_BLACK, UC_GPARTS_COLOR_GRAY3, TYPE_LayoutFree, TYPE_VLayoutMiddle, PNLE_SELECT_FONT_A, 0
#define PNLD_Ktx_AuxilPageOHPIns_copy_Tray3_Y	s_pNOTXEdge_W016H016A, 180, 176, 16, 16, UC_GPARTS_COLOR_BLACK, UC_GPARTS_COLOR_GRAY3, TYPE_LayoutFree, TYPE_VLayoutMiddle, PNLE_SELECT_FONT_A, 0
#define PNLD_Num_AuxilPageOHPIns_copy_Tray3_X_Num	SF_TEXT_DYNAMIC_ALLOC, 180, 176, 48, 16, UC_GPARTS_COLOR_BLACK, UC_GPARTS_COLOR_GRAY3, "%3.1f", "%2.2f", PNLE_SELECT_FONT_A, TYPE_LayoutFree, TYPE_VLayoutMiddle, 0
#define PNLD_Num_AuxilPageOHPIns_copy_Tray3_Y_Num	SF_TEXT_DYNAMIC_ALLOC, 180, 176, 48, 16, UC_GPARTS_COLOR_BLACK, UC_GPARTS_COLOR_GRAY3, "%3.1f", "%2.2f", PNLE_SELECT_FONT_A, TYPE_LayoutFree, TYPE_VLayoutMiddle, 0
#define PNLD_Gra_AuxilPageOHPIns_copy_Tray3_Icon2	tbl_i00_270_000_X16_Y16, 180, 176, TYPE_LayoutFree, False, PNLE_Shadow_none, 16, 16
#define PNLD_Gra_AuxilPageOHPIns_copy_Tray3_Icon1	tbl_i00_273_000_X16_Y16, 180, 176, TYPE_LayoutFree, False, PNLE_Shadow_none, 16, 16
#define PNLD_Key_AuxilPageOHPIns_copy_Tray4	MSG_MSButton, 14, tbl_m30_075_000_X128_Y40, 180, 224, const_cast<PSingleParts**>( tbl_AuxilPageOHPIns_copy_Tray4 ), SIZE(tbl_AuxilPageOHPIns_copy_Tray4), True, PNLE_Shadow_Parts_2
#define PNLD_Ktx_AuxilPageOHPIns_copy_Tray4	s_pNOTTray4_W032H040A, 180, 224, 32, 40, UC_GPARTS_COLOR_BLACK, UC_GPARTS_COLOR_GRAY3, TYPE_LayoutCentering, TYPE_VLayoutMiddle, PNLE_SELECT_FONT_A, 0
#define PNLD_Gra_AuxilPageOHPIns_copy_Tray4_FreeIcon	tbl_i00_275_000_X16_Y16, 180, 224, TYPE_LayoutFree, False, PNLE_Shadow_none, 16, 16
#define PNLD_Ktx_AuxilPageOHPIns_copy_Tray4_Sheet	s_pNOTA4SEF_W068H036A, 180, 224, 68, 36, UC_GPARTS_COLOR_BLACK, UC_GPARTS_COLOR_GRAY3, TYPE_LayoutFree, TYPE_VLayoutMiddle, PNLE_SELECT_FONT_A, 0
#define PNLD_Ktx_AuxilPageOHPIns_copy_Tray4_X	s_pNOTXEdge_W016H016A, 180, 224, 16, 16, UC_GPARTS_COLOR_BLACK, UC_GPARTS_COLOR_GRAY3, TYPE_LayoutFree, TYPE_VLayoutMiddle, PNLE_SELECT_FONT_A, 0
#define PNLD_Ktx_AuxilPageOHPIns_copy_Tray4_Y	s_pNOTXEdge_W016H016A, 180, 224, 16, 16, UC_GPARTS_COLOR_BLACK, UC_GPARTS_COLOR_GRAY3, TYPE_LayoutFree, TYPE_VLayoutMiddle, PNLE_SELECT_FONT_A, 0
#define PNLD_Num_AuxilPageOHPIns_copy_Tray4_X_Num	SF_TEXT_DYNAMIC_ALLOC, 180, 224, 48, 16, UC_GPARTS_COLOR_BLACK, UC_GPARTS_COLOR_GRAY3, "%3.1f", "%2.2f", PNLE_SELECT_FONT_A, TYPE_LayoutFree, TYPE_VLayoutMiddle, 0
#define PNLD_Num_AuxilPageOHPIns_copy_Tray4_Y_Num	SF_TEXT_DYNAMIC_ALLOC, 180, 224, 48, 16, UC_GPARTS_COLOR_BLACK, UC_GPARTS_COLOR_GRAY3, "%3.1f", "%2.2f", PNLE_SELECT_FONT_A, TYPE_LayoutFree, TYPE_VLayoutMiddle, 0
#define PNLD_Gra_AuxilPageOHPIns_copy_Tray4_Icon2	tbl_i00_270_000_X16_Y16, 180, 224, TYPE_LayoutFree, False, PNLE_Shadow_none, 16, 16
#define PNLD_Gra_AuxilPageOHPIns_copy_Tray4_Icon1	tbl_i00_273_000_X16_Y16, 180, 224, TYPE_LayoutFree, False, PNLE_Shadow_none, 16, 16
#define PNLD_Key_AuxilPageOHPIns_copy_TrayLct	MSG_MSButton, 27, tbl_m30_075_000_X128_Y40, 316, 224, const_cast<PSingleParts**>( tbl_AuxilPageOHPIns_copy_TrayLct ), SIZE(tbl_AuxilPageOHPIns_copy_TrayLct), True, PNLE_Shadow_Parts_2
// comment 2018/05/17 SCD2-Sky中野 [DenebMLK][機器構成変更No.7] 5段増設プリント機能追加対応.
#ifdef DEF_DenebMLKCus
#define PNLD_Ktx_AuxilPageOHPIns_copy_TrayLct	s_pNOTTray5_W016H040A, 316, 224, 32, 40, UC_GPARTS_COLOR_BLACK, UC_GPARTS_COLOR_GRAY3, TYPE_LayoutCentering, TYPE_VLayoutMiddle, PNLE_SELECT_FONT_A, 0
#else
#define PNLD_Ktx_AuxilPageOHPIns_copy_TrayLct	s_pNOTTrayLct_W032H040A, 316, 224, 32, 40, UC_GPARTS_COLOR_BLACK, UC_GPARTS_COLOR_GRAY3, TYPE_LayoutCentering, TYPE_VLayoutMiddle, PNLE_SELECT_FONT_A, 0
#endif	// DEF_DenebMLKCus.
#define PNLD_Gra_AuxilPageOHPIns_copy_TrayLct_FreeIcon	tbl_i00_275_000_X16_Y16, 316, 224, TYPE_LayoutFree, False, PNLE_Shadow_none, 16, 16
#define PNLD_Ktx_AuxilPageOHPIns_copy_TrayLct_Sheet	s_pNOTA4SEF_W068H036A, 316, 224, 68, 36, UC_GPARTS_COLOR_BLACK, UC_GPARTS_COLOR_GRAY3, TYPE_LayoutFree, TYPE_VLayoutMiddle, PNLE_SELECT_FONT_A, 0
#define PNLD_Ktx_AuxilPageOHPIns_copy_TrayLct_X	s_pNOTXEdge_W016H016A, 316, 224, 16, 16, UC_GPARTS_COLOR_BLACK, UC_GPARTS_COLOR_GRAY3, TYPE_LayoutFree, TYPE_VLayoutMiddle, PNLE_SELECT_FONT_A, 0
#define PNLD_Ktx_AuxilPageOHPIns_copy_TrayLct_Y	s_pNOTYEdge_W016H016A, 316, 224, 16, 16, UC_GPARTS_COLOR_BLACK, UC_GPARTS_COLOR_GRAY3, TYPE_LayoutFree, TYPE_VLayoutMiddle, PNLE_SELECT_FONT_A, 0
#define PNLD_Num_AuxilPageOHPIns_copy_TrayLct_X_Num	SF_TEXT_DYNAMIC_ALLOC, 316, 224, 48, 16, UC_GPARTS_COLOR_BLACK, UC_GPARTS_COLOR_GRAY3, "%3.1f", "%2.2f", PNLE_SELECT_FONT_A, TYPE_LayoutFree, TYPE_VLayoutMiddle, 0
#define PNLD_Num_AuxilPageOHPIns_copy_TrayLct_Y_Num	SF_TEXT_DYNAMIC_ALLOC, 316, 224, 48, 16, UC_GPARTS_COLOR_BLACK, UC_GPARTS_COLOR_GRAY3, "%3.1f", "%2.2f", PNLE_SELECT_FONT_A, TYPE_LayoutFree, TYPE_VLayoutMiddle, 0
#define PNLD_Gra_AuxilPageOHPIns_copy_TrayLct_Icon2	tbl_i00_270_000_X16_Y16, 316, 224, TYPE_LayoutFree, False, PNLE_Shadow_none, 16, 16
#define PNLD_Gra_AuxilPageOHPIns_copy_TrayLct_Icon1	tbl_i00_273_000_X16_Y16, 316, 224, TYPE_LayoutFree, False, PNLE_Shadow_none, 16, 16
//  トレイ１ボタン.
PSinglePartsTextEx				Ktx_AuxilPageOHPIns_copy_Tray1( PNLD_Ktx_AuxilPageOHPIns_copy_Tray1 );		// 1.
PSinglePartsResizableGraphic	Gra_AuxilPageOHPIns_copy_Tray1_FreeIcon( PNLD_Gra_AuxilPageOHPIns_copy_Tray1_FreeIcon );		// １：手差しマーク（通常）.
PSinglePartsTextEx				Ktx_AuxilPageOHPIns_copy_Tray1_Sheet( PNLD_Ktx_AuxilPageOHPIns_copy_Tray1_Sheet );		// A4 _PT.
PSinglePartsTextEx				Ktx_AuxilPageOHPIns_copy_Tray1_X( PNLD_Ktx_AuxilPageOHPIns_copy_Tray1_X );		// １：Ｘ.
PSinglePartsTextEx				Ktx_AuxilPageOHPIns_copy_Tray1_Y( PNLD_Ktx_AuxilPageOHPIns_copy_Tray1_Y );		// １：Ｙ.
PNumberFieldEx					Num_AuxilPageOHPIns_copy_Tray1_X_Num( PNLD_Num_AuxilPageOHPIns_copy_Tray1_X_Num );		// １：Ｘ値.
PNumberFieldEx					Num_AuxilPageOHPIns_copy_Tray1_Y_Num( PNLD_Num_AuxilPageOHPIns_copy_Tray1_Y_Num );		// １：Ｙ値.
PSinglePartsResizableGraphic	Gra_AuxilPageOHPIns_copy_Tray1_Icon2( PNLD_Gra_AuxilPageOHPIns_copy_Tray1_Icon2 );		// アイコン：種類.
PSinglePartsResizableGraphic	Gra_AuxilPageOHPIns_copy_Tray1_Icon1( PNLD_Gra_AuxilPageOHPIns_copy_Tray1_Icon1 );		// アイコン：残量.
static PSingleParts* const tbl_AuxilPageOHPIns_copy_Tray1[] = {
	&Ktx_AuxilPageOHPIns_copy_Tray1,			// 1.
	&Gra_AuxilPageOHPIns_copy_Tray1_FreeIcon,	// 手差しマーク.
	&Ktx_AuxilPageOHPIns_copy_Tray1_Sheet,		// ボタン名称.
	&Ktx_AuxilPageOHPIns_copy_Tray1_X,			// X.
	&Ktx_AuxilPageOHPIns_copy_Tray1_Y,			// Y.
	&Num_AuxilPageOHPIns_copy_Tray1_X_Num,		// X数値.
	&Num_AuxilPageOHPIns_copy_Tray1_Y_Num,		// Y数値.
	&Gra_AuxilPageOHPIns_copy_Tray1_Icon2,		// 用紙種類.
	&Gra_AuxilPageOHPIns_copy_Tray1_Icon1,		// 用紙残量.
};
PFormSettingButton				Key_AuxilPageOHPIns_copy_Tray1( PNLD_Key_AuxilPageOHPIns_copy_Tray1 );		// 1.
//  トレイ２ボタン.
PSinglePartsTextEx				Ktx_AuxilPageOHPIns_copy_Tray2( PNLD_Ktx_AuxilPageOHPIns_copy_Tray2 );		// 2.
PSinglePartsResizableGraphic	Gra_AuxilPageOHPIns_copy_Tray2_FreeIcon( PNLD_Gra_AuxilPageOHPIns_copy_Tray2_FreeIcon );		// ２：手差しマーク（通常）.
PSinglePartsTextEx				Ktx_AuxilPageOHPIns_copy_Tray2_Sheet( PNLD_Ktx_AuxilPageOHPIns_copy_Tray2_Sheet );		// A4 _PT.
PSinglePartsTextEx				Ktx_AuxilPageOHPIns_copy_Tray2_X( PNLD_Ktx_AuxilPageOHPIns_copy_Tray2_X );		// ２：Ｘ.
PSinglePartsTextEx				Ktx_AuxilPageOHPIns_copy_Tray2_Y( PNLD_Ktx_AuxilPageOHPIns_copy_Tray2_Y );		// ２：Ｙ.
PNumberFieldEx					Num_AuxilPageOHPIns_copy_Tray2_X_Num( PNLD_Num_AuxilPageOHPIns_copy_Tray2_X_Num );		// ２：Ｘ値.
PNumberFieldEx					Num_AuxilPageOHPIns_copy_Tray2_Y_Num( PNLD_Num_AuxilPageOHPIns_copy_Tray2_Y_Num );		// ２：Ｙ値.
PSinglePartsResizableGraphic	Gra_AuxilPageOHPIns_copy_Tray2_Icon2( PNLD_Gra_AuxilPageOHPIns_copy_Tray2_Icon2 );		// アイコン：種類.
PSinglePartsResizableGraphic	Gra_AuxilPageOHPIns_copy_Tray2_Icon1( PNLD_Gra_AuxilPageOHPIns_copy_Tray2_Icon1 );		// アイコン：残量.
static PSingleParts* const tbl_AuxilPageOHPIns_copy_Tray2[] = {
	&Ktx_AuxilPageOHPIns_copy_Tray2,
	&Gra_AuxilPageOHPIns_copy_Tray2_FreeIcon,
	&Ktx_AuxilPageOHPIns_copy_Tray2_Sheet,
	&Ktx_AuxilPageOHPIns_copy_Tray2_X,
	&Ktx_AuxilPageOHPIns_copy_Tray2_Y,
	&Num_AuxilPageOHPIns_copy_Tray2_X_Num,
	&Num_AuxilPageOHPIns_copy_Tray2_Y_Num,
	&Gra_AuxilPageOHPIns_copy_Tray2_Icon2,
	&Gra_AuxilPageOHPIns_copy_Tray2_Icon1,
};
PFormSettingButton				Key_AuxilPageOHPIns_copy_Tray2( PNLD_Key_AuxilPageOHPIns_copy_Tray2 );		// 2.
//  トレイ３ボタン.
PSinglePartsTextEx				Ktx_AuxilPageOHPIns_copy_Tray3( PNLD_Ktx_AuxilPageOHPIns_copy_Tray3 );		// 3.
PSinglePartsResizableGraphic	Gra_AuxilPageOHPIns_copy_Tray3_FreeIcon( PNLD_Gra_AuxilPageOHPIns_copy_Tray3_FreeIcon );		// ３：手差しマーク（通常）.
PSinglePartsTextEx				Ktx_AuxilPageOHPIns_copy_Tray3_Sheet( PNLD_Ktx_AuxilPageOHPIns_copy_Tray3_Sheet );		// A4 _PT.
PSinglePartsTextEx				Ktx_AuxilPageOHPIns_copy_Tray3_X( PNLD_Ktx_AuxilPageOHPIns_copy_Tray3_X );		// ３：Ｘ.
PSinglePartsTextEx				Ktx_AuxilPageOHPIns_copy_Tray3_Y( PNLD_Ktx_AuxilPageOHPIns_copy_Tray3_Y );		// ３：Ｙ.
PNumberFieldEx					Num_AuxilPageOHPIns_copy_Tray3_X_Num( PNLD_Num_AuxilPageOHPIns_copy_Tray3_X_Num );		// ３：Ｘ値.
PNumberFieldEx					Num_AuxilPageOHPIns_copy_Tray3_Y_Num( PNLD_Num_AuxilPageOHPIns_copy_Tray3_Y_Num );		// ３：Ｙ値.
PSinglePartsResizableGraphic	Gra_AuxilPageOHPIns_copy_Tray3_Icon2( PNLD_Gra_AuxilPageOHPIns_copy_Tray3_Icon2 );		// アイコン：種類.
PSinglePartsResizableGraphic	Gra_AuxilPageOHPIns_copy_Tray3_Icon1( PNLD_Gra_AuxilPageOHPIns_copy_Tray3_Icon1 );		// アイコン：残量.
static PSingleParts* const tbl_AuxilPageOHPIns_copy_Tray3[] = {
	&Ktx_AuxilPageOHPIns_copy_Tray3,
	&Gra_AuxilPageOHPIns_copy_Tray3_FreeIcon,
	&Ktx_AuxilPageOHPIns_copy_Tray3_Sheet,
	&Ktx_AuxilPageOHPIns_copy_Tray3_X,
	&Ktx_AuxilPageOHPIns_copy_Tray3_Y,
	&Num_AuxilPageOHPIns_copy_Tray3_X_Num,
	&Num_AuxilPageOHPIns_copy_Tray3_Y_Num,
	&Gra_AuxilPageOHPIns_copy_Tray3_Icon2,
	&Gra_AuxilPageOHPIns_copy_Tray3_Icon1,
};
PFormSettingButton				Key_AuxilPageOHPIns_copy_Tray3( PNLD_Key_AuxilPageOHPIns_copy_Tray3 );		// 3.
//  トレイ４ボタン.
PSinglePartsTextEx				Ktx_AuxilPageOHPIns_copy_Tray4( PNLD_Ktx_AuxilPageOHPIns_copy_Tray4 );		// 4.
PSinglePartsResizableGraphic	Gra_AuxilPageOHPIns_copy_Tray4_FreeIcon( PNLD_Gra_AuxilPageOHPIns_copy_Tray4_FreeIcon );		// ４：手差しマーク（通常）.
PSinglePartsTextEx				Ktx_AuxilPageOHPIns_copy_Tray4_Sheet( PNLD_Ktx_AuxilPageOHPIns_copy_Tray4_Sheet );		// A4 _PT.
PSinglePartsTextEx				Ktx_AuxilPageOHPIns_copy_Tray4_X( PNLD_Ktx_AuxilPageOHPIns_copy_Tray4_X );		// ４：Ｘ.
PSinglePartsTextEx				Ktx_AuxilPageOHPIns_copy_Tray4_Y( PNLD_Ktx_AuxilPageOHPIns_copy_Tray4_Y );		// ４：Ｙ.
PNumberFieldEx					Num_AuxilPageOHPIns_copy_Tray4_X_Num( PNLD_Num_AuxilPageOHPIns_copy_Tray4_X_Num );		// ４：Ｘ値.
PNumberFieldEx					Num_AuxilPageOHPIns_copy_Tray4_Y_Num( PNLD_Num_AuxilPageOHPIns_copy_Tray4_Y_Num );		// ４：Ｙ値.
PSinglePartsResizableGraphic	Gra_AuxilPageOHPIns_copy_Tray4_Icon2( PNLD_Gra_AuxilPageOHPIns_copy_Tray4_Icon2 );		// アイコン：種類.
PSinglePartsResizableGraphic	Gra_AuxilPageOHPIns_copy_Tray4_Icon1( PNLD_Gra_AuxilPageOHPIns_copy_Tray4_Icon1 );		// アイコン：残量.
static PSingleParts* const tbl_AuxilPageOHPIns_copy_Tray4[] = {
	&Ktx_AuxilPageOHPIns_copy_Tray4,
	&Gra_AuxilPageOHPIns_copy_Tray4_FreeIcon,
	&Ktx_AuxilPageOHPIns_copy_Tray4_Sheet,
	&Ktx_AuxilPageOHPIns_copy_Tray4_X,
	&Ktx_AuxilPageOHPIns_copy_Tray4_Y,
	&Num_AuxilPageOHPIns_copy_Tray4_X_Num,
	&Num_AuxilPageOHPIns_copy_Tray4_Y_Num,
	&Gra_AuxilPageOHPIns_copy_Tray4_Icon2,
	&Gra_AuxilPageOHPIns_copy_Tray4_Icon1,
};
PFormSettingButton				Key_AuxilPageOHPIns_copy_Tray4( PNLD_Key_AuxilPageOHPIns_copy_Tray4 );		// 4.
//  トレイＬＣＴボタン.
PSinglePartsTextEx				Ktx_AuxilPageOHPIns_copy_TrayLct( PNLD_Ktx_AuxilPageOHPIns_copy_TrayLct );		// L.
PSinglePartsResizableGraphic	Gra_AuxilPageOHPIns_copy_TrayLct_FreeIcon( PNLD_Gra_AuxilPageOHPIns_copy_TrayLct_FreeIcon );		// Ｌ：手差しマーク（通常）.
PSinglePartsTextEx				Ktx_AuxilPageOHPIns_copy_TrayLct_Sheet( PNLD_Ktx_AuxilPageOHPIns_copy_TrayLct_Sheet );		// A4 _PT.
PSinglePartsTextEx				Ktx_AuxilPageOHPIns_copy_TrayLct_X( PNLD_Ktx_AuxilPageOHPIns_copy_TrayLct_X );		// Ｌ：Ｘ.
PSinglePartsTextEx				Ktx_AuxilPageOHPIns_copy_TrayLct_Y( PNLD_Ktx_AuxilPageOHPIns_copy_TrayLct_Y );		// Ｌ：Ｙ.
PNumberFieldEx					Num_AuxilPageOHPIns_copy_TrayLct_X_Num( PNLD_Num_AuxilPageOHPIns_copy_TrayLct_X_Num );		// Ｌ：Ｘ値.
PNumberFieldEx					Num_AuxilPageOHPIns_copy_TrayLct_Y_Num( PNLD_Num_AuxilPageOHPIns_copy_TrayLct_Y_Num );		// Ｌ：Ｙ値.
PSinglePartsResizableGraphic	Gra_AuxilPageOHPIns_copy_TrayLct_Icon2( PNLD_Gra_AuxilPageOHPIns_copy_TrayLct_Icon2 );		// アイコン：種類.
PSinglePartsResizableGraphic	Gra_AuxilPageOHPIns_copy_TrayLct_Icon1( PNLD_Gra_AuxilPageOHPIns_copy_TrayLct_Icon1 );		// アイコン：残量.
static PSingleParts* const tbl_AuxilPageOHPIns_copy_TrayLct[] = {
	&Ktx_AuxilPageOHPIns_copy_TrayLct,
	&Gra_AuxilPageOHPIns_copy_TrayLct_FreeIcon,
	&Ktx_AuxilPageOHPIns_copy_TrayLct_Sheet,
	&Ktx_AuxilPageOHPIns_copy_TrayLct_X,
	&Ktx_AuxilPageOHPIns_copy_TrayLct_Y,
	&Num_AuxilPageOHPIns_copy_TrayLct_X_Num,
	&Num_AuxilPageOHPIns_copy_TrayLct_Y_Num,
	&Gra_AuxilPageOHPIns_copy_TrayLct_Icon2,
	&Gra_AuxilPageOHPIns_copy_TrayLct_Icon1,
};
PFormSettingButton				Key_AuxilPageOHPIns_copy_TrayLct( PNLD_Key_AuxilPageOHPIns_copy_TrayLct );		// L.
//   小マシン図.
PSinglePartsGraphic		Gra_AuxilPageOHPIns_copy_SmallMachine_1( PG_BPPPC_Do_X056Y086, 56, 32, 16, TYPE_LayoutFree, TYPE_VLayoutFree );		// 用紙トレイ選択（小マシン図－マシン本体）.
PSinglePartsGraphic		Gra_AuxilPageOHPIns_copy_SmallMachine_2( PG_BPFS526_New_Do_X056Y096, 0, 24, 16, TYPE_LayoutFree, TYPE_VLayoutFree );		// 用紙トレイ選択（小マシン図－フィニッシャー).
PSinglePartsGraphic		Gra_AuxilPageOHPIns_copy_SmallMachine_3( PG_BPEDH_X048Y013, 56, 32, 16, TYPE_LayoutFree, TYPE_VLayoutFree );		// 用紙トレイ選択（小マシン図－EDH）.
PSinglePartsGraphic		Gra_AuxilPageOHPIns_copy_SmallMachine_4( PG_BPDuplex_Do_X008Y022, 104, 56, 16, TYPE_LayoutFree, TYPE_VLayoutFree );		// 用紙トレイ選択（小マシン図－Duplex）.
PSinglePartsGraphic		Gra_AuxilPageOHPIns_copy_SmallMachine_5( PG_BPManualTray_Do_X040Y011, 104, 78, 16, TYPE_LayoutFree, TYPE_VLayoutFree );		// 用紙トレイ選択（小マシン図－手差しトレイ）.
PSinglePartsGraphic		Gra_AuxilPageOHPIns_copy_SmallMachine_6( PG_BP1stTray_Do_X040Y011, 56, 81, 16, TYPE_LayoutFree, TYPE_VLayoutFree );		// 用紙トレイ選択（小マシン図－トレイ1）.
PSinglePartsGraphic		Gra_AuxilPageOHPIns_copy_SmallMachine_7( PG_BP2ndTray_Do_X040Y011, 56, 91, 16, TYPE_LayoutFree, TYPE_VLayoutFree );		// 用紙トレイ選択（小マシン図－トレイ2）.
PSinglePartsGraphic		Gra_AuxilPageOHPIns_copy_SmallMachine_8( PG_BP3rdTray_Do_X040Y015, 56, 101, 16, TYPE_LayoutFree, TYPE_VLayoutFree );		// 用紙トレイ選択（小マシン図－トレイ3）.
PSinglePartsGraphic		Gra_AuxilPageOHPIns_copy_SmallMachine_9( PG_BP4thTray_Do_X016Y012, 80, 104, 16, TYPE_LayoutFree, TYPE_VLayoutFree );		// 用紙トレイ選択（小マシン図－トレイ4）.
PSinglePartsGraphic		Gra_AuxilPageOHPIns_copy_SmallMachine_10( PG_BPLCC_X040Y015, 56, 101, 16, TYPE_LayoutFree, TYPE_VLayoutFree );		// 用紙トレイ選択（小マシン図－LCC）.
PSinglePartsGraphic		Gra_AuxilPageOHPIns_copy_SmallMachine_11( PG_BPLCT_Do_X032Y030, 104, 88, 16, TYPE_LayoutFree, TYPE_VLayoutFree );		// 用紙トレイ選択（小マシン図－LCT）.
static PDisplayParts* const tbl_AuxilPageOHPIns_copy_SmallMachine[] = {
	&Gra_AuxilPageOHPIns_copy_SmallMachine_1,	// 本体.
	&Gra_AuxilPageOHPIns_copy_SmallMachine_2,	// フィニッシャー.
	&Gra_AuxilPageOHPIns_copy_SmallMachine_3,	// EDH.
	&Gra_AuxilPageOHPIns_copy_SmallMachine_4,	// Duplex.
	&Gra_AuxilPageOHPIns_copy_SmallMachine_5,	// 手差し.
	&Gra_AuxilPageOHPIns_copy_SmallMachine_6,	// トレイ1.
	&Gra_AuxilPageOHPIns_copy_SmallMachine_7,	// トレイ２.
	&Gra_AuxilPageOHPIns_copy_SmallMachine_8,	// トレイ３.
	&Gra_AuxilPageOHPIns_copy_SmallMachine_9,	// トレイ４.
	&Gra_AuxilPageOHPIns_copy_SmallMachine_10,	// LCC.
	&Gra_AuxilPageOHPIns_copy_SmallMachine_11,	// LCT.
};
static  PSmallMachineComponent	Cmp_AuxilPageOHPIns_copy_SmallMachine( PNLD_Cmp_AuxilPageOHPIns_copy_SmallMachine );	// 用紙トレイ選択（装置図).
#ifdef DEF_LCD_7INCH
#include "PD/A020161000_7inch.cpp"
#else
#include "PD/A020161000.cpp"
#endif
//================================================================================================.
//  インクルード・ファイル.
//================================================================================================.

/*================================================================================================
  定数・型の宣言.
================================================================================================*/

/*================================================================================================
  列挙型変数のタグ宣言.
================================================================================================*/

/*================================================================================================
 マクロ関数の定義.
================================================================================================*/

static PNLS_SelectPaperTrayCmnMndParts s_parts_tbl = {	// 必須パーツ.
	A020161000_Txt0063,
	Cmp_AuxilPageOHPIns_copy_SmallMachine,
	A020161000_Rbt1001
};
static uchar uc_TrayPartsIdTbl[] = {
	PNLD_ID_SELECTPAPERTRAY1,
	PNLD_ID_SELECTPAPERTRAY2,
	PNLD_ID_SELECTPAPERTRAY3,
	PNLD_ID_SELECTPAPERTRAY4,
	PNLD_ID_SELECTPAPERTRAYLCT
};
//PanelDesigner対応------------------------------------------------------------------------------------------------------.
// 画面開始処理.
void PA020161000::startGamen()
{
	bl_PanelDesigner = True; // PanelDesigner対応.
	PPopupDecisionButtonCom::startGamen();	// 親クラスコール.
// コールバック関数の登録.
	ATTACH_OBSERVER( PA020161000, gp_A020161000, PA020161000::prTray, A020161000_Rbt1001, Bool, short );	// トレイ.
	ATTACH_OBSERVER( PA020161000, gp_A020161000, PA020161000::prOnOff, A020161000_Rbt1003, Bool, short );	// ON/OFF.
	ATTACH_OBSERVER( PA020161000, gp_A020161000, PA020161000::prCancel, A020161000_Btn0032, Bool, Bool );	// キャンセル.
	setOkButtonForFunctionBadge( &A020161000_Btn0005 ); // 機能バッチボタン押下時に代替え処理を行うＯＫキーを登録.
// 変数等の初期化処理を記載する.
	short ss_StartPositionX = 0;	short ss_StartPositionY = 0;	// [AC]用紙トレイ選択初期設定.
// ここに構成による表示条件を記載する.
	this->getPosition( &ss_StartPositionX, &ss_StartPositionY );
	PNLC_SelectPaperTrayOHPInsPaperSet_PD::newInstance()->Initialize(s_parts_tbl, uc_TrayPartsIdTbl, sizeof(uc_TrayPartsIdTbl)/sizeof(uchar), PnlComGetJobCopy(), PnlComGetFeedTray(), True, ss_StartPositionX, ss_StartPositionY );
	A020161000_Rbt1003.setN( isOn( CNK_OHP, PnlComGetJobCopy() ) ? 0 : 1, True );	// ON/OFFボタン設定.
	if( ( onlyPanel.isWarning( PCAUTION_OHPPaperEmpty_S ) == True ) ) A020161000_Cmp1005.changeMainMessageText( s_pMSGOHPFilmEmpty_W448H034A );
	else A020161000_Cmp1005.changeMainMessageText( 0 );
// ここに画面の初期化処理を記載する.
	A020161000_Txt0055.setCursorType(PNLD_CURSOR_TYPE_OFF);	// 用紙サイズX カーソル表示しない.
	A020161000_Txt0053.setCursorType(PNLD_CURSOR_TYPE_OFF);	// 用紙サイズY カーソル表示しない.
	A020161000_Cmp1005.setNowPopupGamen( PNLE_CopyOHPPaper );
}
// トレイ.
Bool PA020161000::prTray( short n )
{
	PNLC_SelectPaperTrayOHPInsPaperSet_PD::newInstance()->setSelectedTray( static_cast<uchar>( n ) );	// 各給紙口ボタン押下.
	return True;
}
// ON/OFF.
Bool PA020161000::prOnOff( short n )
{
	switch (n) {
		case 0:	// メッセージエリア、[ON]ボタンの場合.
			if( -1 != m_ssJobSpecBackUpFlg ) {		// バックアップ済みの場合.
				PnlComGetCopyDataLib()->setJobSpecBackup( m_ssJobSpecBackUpFlg, PnlComGetJobCopy()->getJobSpec() );	// OFFにする前のジョブスペックを元に戻す.
				PnlComGetCopyDataLib()->JobSpecBackupClear( m_ssJobSpecBackUpFlg );	// バックアップデータの消去.
				m_ssJobSpecBackUpFlg = -1;
// Comment 2016/09/08 HCLT-Venkataramesh [IT5_3.3][1003656] Handling "ON"," OFF "selection.
//			} else if( combinationCheck( CNK_OHP, PnlComGetJobCopy() ) == False ) return True;			// バックアップ無しの場合, OHP 合紙モードの組合せ禁止チェック.
			} else {
				if( combinationCheck( CNK_OHP, PnlComGetJobCopy() ) == False ) {
//comment 2018/8/2 SCD2-Sky肥後 [Sparrow][AR1050098]入力無効音が2回再生される問題.
					A020161000_Rbt1003.setBuzzerOff( True );
					return False;					// バックアップ無しの場合, OHP 合紙モードの組合せ禁止チェック.
				}
				setOHPInsertionMode();				// OHP 合紙モードを設定.
			}
			break;
		case 1:	// メッセージエリア、[OFF]ボタンの場合.
			m_ssJobSpecBackUpFlg = PnlComGetCopyDataLib()->saveJobSpecBackup( m_ssJobSpecBackUpFlg, PnlComGetJobCopy()->getJobSpec() );	// ジョブスペックのバックアップ.
			PnlComGetJobCopy()->getJobSpec()->pc_Print->c_OhpInsertion.setFeedTray( TYPE_TrayTypeNon );	// トレイの選択状態クリア.
			cancel( CNK_OHP, CNK_Size, PnlComGetJobCopy() );	// OHPの設定をOFFにする.
			break;
	}
	return True;
}

// キャンセル.
Bool PA020161000::prCancel( Bool on )
{
	PNLC_StackManJobSpec::newInstance()->rollbackJobSpec();	// 設定を元に戻す.
	return True;
}
//従来処理(画面外処理)---------------------------------------------------------------------------------------------------.
// 表示部品のセットアップ.
void PA020161000::setupGamen( short option )
{
	PPopupDecisionButtonCom::setupGamen();	// 前処理.
	PNLC_SelectPaperTrayOHPInsPaperSet_PD::newInstance()->update();
	PNLS_setOriginalSizeCLCD2Key s_parts = {&A020161000_Gra0052, &A020161000_Txt0057, NULL, &A020161000_Txt0054, &A020161000_Txt0056, &A020161000_Txt0055, &A020161000_Txt0053, NULL, NULL, NULL, &A020161000_Gra0059};
	PnlComsetPaperSizeCLCD2( &s_parts, PnlComGetJobCopy()->getJobSpec()->pc_Print->c_PaperTray.getFeedTray() );
	if( ( onlyPanel.isWarning( PCAUTION_OHPPaperEmpty_S ) == True ) ) A020161000_Cmp1005.changeMainMessageText( s_pMSGOHPFilmEmpty_W448H034A );
	else A020161000_Cmp1005.changeMainMessageText( 0 );
}
// アクティブ状態の通知.
void PA020161000::procActivate()
{
	if( gp_A0A3900000.getSettingOn() ) {	// 組合せ禁止（設定変更確認）画面からの遷移時.
		gp_A0A3900000.initSettingOn();
// Comment 2016/09/08 HCLT-Venkataramesh [IT5_3.3][1003656] Handling default Tray selection.
		prOnOff( 0 );
	}
	short ss_TrayNum = -1;
	switch( PnlComGetJobCopy()->getJobSpec()->pc_Print->c_OhpInsertion.getFeedTray() ) {
		case TYPE_FeedTray1: ss_TrayNum = 0;	break;
		case TYPE_FeedTray2: ss_TrayNum = 1;	break;
		case TYPE_FeedTray3: ss_TrayNum = 2;	break;
		case TYPE_FeedTray4: ss_TrayNum = 3;	break;
		case TYPE_FeedTray5: ss_TrayNum = 4;	break;
	}
	A020161000_Rbt1001.setN( ss_TrayNum );		// トレー選択更新.
	PPopupDecisionButtonCom::procActivate();	// 親クラス処理.
}
// 画面終了処理.
void PA020161000::endGamen()
{
	if( m_ssJobSpecBackUpFlg != -1 ) {
		PnlComGetCopyDataLib()->JobSpecBackupClear( m_ssJobSpecBackUpFlg );	// バックアップしたJobSpecをクリアする.
		m_ssJobSpecBackUpFlg = -1;
	}
	PPopupDecisionButtonCom::endGamen();
}
// 組み禁時の先設定変更許可通知ハンドラ.
void PA020161000::procChangePreSettingOk( uchar option )
{
	switch( sp_MessageWarningTable[ onlyPanel.us_NowMessageWarning ].ec_WarningName ) {
		case PNL_ERROR_Combination_ACS :
		case PNL_ERROR_Combination_FullColorMode :
		case PNL_ERROR_Combination_2Color:
		case PNL_ERROR_Combination_MonoColor:
			if( CNK_OHP == onlyPanel.ep_CombinationOrigin ) {
				PnlComGetJobCopy()->getJobSpec()->pc_Scan->setAcsOn( False );
				PnlComGetJobCopy()->getJobSpec()->pc_Common->setColorMode( TYPE_ColorModeBlack );
				PnlComGetJobCopy()->getJobSpec()->pc_Common->c_MonoColor.setMonoColorOn( False );
				PnlComGetJobCopy()->getJobSpec()->pc_Common->c_MonoColor.setMonoColor( TYPE_MonoColorRed );
				PnlComGetJobCopy()->getJobSpec()->pc_Common->c_2ColorMode.set2ColorOn( False );
				PnlComGetJobCopy()->getJobSpec()->pc_Common->c_2ColorMode.setMonoColor( TYPE_MonoColorRed );
			}
			break;
	}
	setOHPInsertionMode();
}
//画面固有処理-----------------------------------------------------------------------------------------------------------.
// 合紙設定処理.
void PA020161000::setOHPInsertionMode()
{
	TYPE_FeedTray e_FeedTray = PnlComGetJobCopy()->getJobSpec()->pc_Print->c_PaperTray.getFeedTray();
	if( TYPE_TrayTypeMultiMan != PnlComGetFeedTray()->getTrayType( e_FeedTray ) || True == PnlComGetJobCopy()->getJobSpec()->pc_Print->c_PaperTray.getApsOn() ) {	// 現在設定されているトレイがMPCかを判定.
		TYPE_FeedTray e_FeedTrayOhp = TYPE_FeedTrayManual;
		TYPE_FeedTray e_FeedTrayOhpCandidate = TYPE_FeedTrayManual;
		Bool b_Discovery = False;
		uchar i;
		for( i = 0; uc_SizeShowFeedTrayTbl > i; ++i ) {
			if( TYPE_TrayTypeMultiMan == PnlComGetFeedTray()->getTrayType( e_AryShowFeedTrayTbl[i] ) ) {	// マルチ手差しである場合.
				if( False == b_Discovery ) {
					e_FeedTrayOhpCandidate = e_AryShowFeedTrayTbl[i];
					b_Discovery = True;
				}
				if( TYPE_PaperTypeOHP == PnlComGetFeedTray()->getPaperType( e_AryShowFeedTrayTbl[i] ).getPaperType() ) {	// 用紙種類を取得.
					e_FeedTrayOhp = e_AryShowFeedTrayTbl[i];	// OHP用紙のトレイを取得.
					break;
				}
			}
		}
		if( uc_SizeShowFeedTrayTbl == i ) e_FeedTrayOhp = e_FeedTrayOhpCandidate;
		onlyPanel.setFeedMode( PnlComGetJobCopy(), e_FeedTrayOhp, False );
	}
	PnlComGetJobCopy()->getJobSpec()->pc_Print->c_OhpInsertion.setPaperInsertion( TYPE_PaperInsertionWithoutImage );
	if( combinationCheck( CNK_AMS, PnlComGetJobCopy(), True, False ) == True ) {	// オート倍率は、組み合わせ可能な場合のみ設定する .
		PnlComGetJobCopy()->getJobSpec()->pc_Common->c_Mag.setAmsOn( True );	// オート倍率を設定.
		PnlComGetJobCopy()->getJobSpec()->pc_Common->c_Mag.setAvailable( False );
		PnlComGetJobCopy()->getJobSpec()->pc_Common->c_Mag.setMagType( TYPE_MagTypeUnselect );
	}
	static const struct CheckFeedTrayTable_CLCD {
		TYPE_FeedTray				e_FeedTray;					// 給紙口.
		TYPE_TemporaryRescueUnit	e_TemporaryRescueUnit;		// 一時切り離し対象 .
	} checkFeedTrayTable_CLCD[] = {
		{ TYPE_FeedTray1,		TYPE_TemporaryRescueUnitTray1		},	// トレイ1.
		{ TYPE_FeedTray2,		TYPE_TemporaryRescueUnitTray2		},	// トレイ2.
		{ TYPE_FeedTray3,		TYPE_TemporaryRescueUnitTray3		},	// トレイ3.
		{ TYPE_FeedTray4,		TYPE_TemporaryRescueUnitTray4		},	// トレイ4.
// comment 2018/05/17 SCD2-Sky中野 [DenebMLK][機器構成変更No.7] 5段増設プリント機能追加対応.
#ifdef DEF_DenebMLKCus
		{ TYPE_FeedTray5,		TYPE_TemporaryRescueUnitTray5		},	// トレイ5.
#else
		{ TYPE_FeedTray5,		TYPE_TemporaryRescueUnitTrayLCT		},	// LCT.
// comment 2018/05/17 SCD2-Sky中野 [DenebMLK][機器構成変更No.7] 5段増設プリント機能追加対応.
#endif	// DEF_DenebMLKCus.
	};
	for( uchar i=0; i< static_cast<uchar>( sizeof( checkFeedTrayTable_CLCD )/sizeof( CheckFeedTrayTable_CLCD ) ); i++ ) {
		if( ppc_FeedTray->getTrayType( checkFeedTrayTable_CLCD[i].e_FeedTray ) != TYPE_TrayTypeNon && ppc_SystemStatus->getTemporaryRescueUserStatus( checkFeedTrayTable_CLCD[i].e_TemporaryRescueUnit ) == TYPE_TemporaryRescueStatusReset ) {
			PnlComGetJobCopy()->getJobSpec()->pc_Print->c_OhpInsertion.setFeedTray( checkFeedTrayTable_CLCD[i].e_FeedTray );	// OHP合紙用紙のトレイを設定.
			break;
		}
	}
	A020161000_Rbt1003.setN( True == isOn( CNK_OHP, PnlComGetJobCopy() ) ? 0 : 1, True );
	PnlComGetJobCopy()->getJobSpec()->pc_Print->setNumOfCopies( 1 );	// 置数を1に設定.
}
