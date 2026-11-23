//------------------------------------------------------------------------------
/**
 * @file pcgcopy_pA020114.cpp
 * @brief 画面クラスの実装（ポップアップ画面）.
 */
//------------------------------------------------------------------------------

//================================================================================================
//  インクルード・ファイル.
//================================================================================================
#include <panel/plibh/plibh_CLCD/plibh_Mtui.h>
#include <panel/pcgcopy/pcgcopy_CLCD2/pcgcopy_pA020114.h>
#include <panel/pcgcopy/pcgcopy_CLCD2/pigcopy_pA020114.h>
#include <panel/pcgcopy/pcgcopy_CLCD2/pipcopy_pA020114.h>
#include <panel/pcgcom/pcgcom_CLCD2/PNLC_PopupListControl.h>
#include <panel/plibh/plibhg/pdpcomb.h>
#include <panel/plibh/plibhg/plibh.h>
#include <panel/plibh/plibh_CLCD/plibh_CLCD.h>
#include <panel/plibh/plibh_FUM2/plibh_FUM2.h>
#include <panel/plibh/plibh_CLCD2/plibhinclude_CLCD2.h>
#include <panel/plibh/plibh_CLCD2/plibh_CLCD2.h>
#include <panel/pcgstub/pcgstub_FUM5/pigdummy_popup.h>
#include <panel/ppcg/ppcg_CLCD2/ppcgfunctionbadgebutton_CLCD2_pip.h>
#include <panel/plibh/plibh_CLCD2/PNLC_SelectPaperTrayCom_DataLib.h>	// 2011/02/01 Hibino Minerva UV-00724 メディアタイプ情報をマニュアルで追加/編集できる機能.
#include <panel/plibh/plibh_CLCD2/piglib_pA010194.h>
#include <panel/plibh/plibh_CLCD2/PNLC_OutsideKeyCallDataLib.h>
#include <panel/plibh/plibh_CLCD2/PNLC_DetectPaperSizeLib.h>// comment 2014/07/14 OPS1-Sky村井 [Zeus5BK][機器No70] トレイのFD検出廃止.
#include "panel/ppcg/ppcg_CLCD/pidispd_CLCD.h"
#include <panel/plibh/plibh_CLCD2/PNLC_SlideMenuControlLib.h>
#include "panel/plibh/plibh_CLCD2/PNLC_SoftSlideMenuTenKeyTextDispLib.h"
#include <panel/pcgcomponent/pcgcomponentg/PPaperSizeListComponent.h>
#include <panel/pcgcomponent/pcgcomponentg/PPaperTypeListComponent.h>
#include <panel/plibh/plibh_CLCD2/PNLC_CopyDataLib.h>
#include <panel/ppcg/ppcg_CLCD/pidispd_CLCD.h>
#include <panel/pcgcopy/pcgcopy_PD/pcgcopy_A020114140.h>
#include <panel/pcgcopy/pcgcopy_PD/pcgcopy_A020114120.h>
extern PA020114120_6 gp_A020114120_6;
#include <panel/pcgcopy/pcgcopy_PD/pcgcopy_A020114150.h>
extern PA020114150_4 gp_A020114150_4;
#include <panel/pcgcopy/pcgcopy_PD/pcgcopy_J320114220.h>
extern PJ320114220_4 gp_J320114220_4;
//Comment [2015/10/09] [HCL_Prabakaran][MinervaSBK][CQ518365]Header file for PNLD_POPUP_START_X.
#include <panel/pcgcom/pcgcomg/pimsg.h>
// [AR1022515] KronosS用のアイコン画像への置き換え SCD2 3G Sky西場 2017/05/26 start.
#ifdef DEF_KRONOSS_Cus
#include <panel/pddgrap/pddgrap_KronosS/pddgrapg256_CLCD_KronosS.h>
#endif
//comment 01/09/2018 【HCLT-AppalaRaju】 【IT6 EAGLE】 【AR1044533】  Adding the below header files.
#include <panel/plibh/plibh_CLCD2/PNLC_DetectPaperSizeLib.h>
// [AR1022515] KronosS用のアイコン画像への置き換え SCD2 3G Sky西場 2017/05/26 end.
//Comment 2019/12/06 SCD2-Sky K.Kasemura [EagleZ+_BK+][OP_BTS-13540] レターヘッド紙での自動検知は排他.
#include <panel/plibh/plibh_CLCD2/PNLC_CopySettingLib.h>
/*================================================================================================
  定数・型の宣言.
================================================================================================*/
#define PNLD_TabNumABSize			0	//A系B系のタブNo.
#define PNLD_TabNumInch				1	//インチ系のタブNo.
#define PNLD_SizeIndex_Xpos			20	//用紙タブインデックス用のボタンのX座標.
#define PNLD_SizeIndex_Ypos_1		88	//用紙タブインデックス用の上のボタンのY座標.
#define PNLD_SizeIndex_Ypos_2		128	//用紙タブインデックス用の下のボタンのY座標.
#define PNLD_TabBalloon_Xpos		168	//子タブ吹き出しベースのX座標.

// 2015/04/07 D.Suzuki 操作性改善活動 CQ513202対応.
#define PNLD_PaperType_Icon_Metric	(1)	// A系、B系.
#define PNLD_PaperType_Icon_Inch	(2)	// Inch系.
#define PNLD_PaperType_Icon_Post	(3)	// はがき.
#define PNLD_PaperType_Icon_Env		(4)	// 封筒(洋形).
#define PNLD_PaperType_Icon_Letter	(6)	// 封筒(長形).
#define PNLD_PaperType_Icon_ETC		(7)	// その他.
// [CQ525608] 用紙サイズ選択アイコン OPS2 Sky西場 2016/04/14 start.
#ifdef DEF_A4_Cus
#define PNLD_PaperType_Icon_EnvLef		(8)	// 封筒(洋形) Lef.
#define PNLD_PaperType_Icon_LetterLef	(9)	// 封筒(長形) Lef.
//2016/07/12 封筒・はがき用アイコン追加 sky草柳 start.
#define PNLD_PaperType_Icon_LetterSef	(10)
#define PNLD_PaperType_Icon_DoublePost	(11)
#define PNLD_PaperType_Icon_LetterCom10Sef	(12)
//2016/07/12 封筒・はがき用アイコン追加 sky草柳 end.
#endif
// [CQ525608] 用紙サイズ選択アイコン OPS2 Sky西場 2016/04/14 end.

// A3A4FUM 用紙種類設定 t-okada 2015/01/19 Start 選択トレイの設定変更.
#ifdef DEF_A4_Cus
// 各テーブルのトレイごとの有効データ数.
// [KronosS] Sky荻堂 2016/09/26 管理No.22 用紙種類カスタマイズ.
#ifdef DEF_KRONOSS_Cus
#define		PNLD_PaperKindNewTbl_Tray1Num						11	// st_PaperKindNewTblのトレイ1 用紙種類数.
// Comment KronosS [課題No.1012563] -> 2017/02/21 OPS2 Sky竹菴 テーブルの要素番号定義を修正.
//#define		PNLD_PaperKindNewTbl_Tray2Num						10	// st_PaperKindNewTblのトレイ1 用紙種類数.
//#define		PNLD_PaperKindNewTbl_Tray3Num						10	// st_PaperKindNewTblのトレイ1 用紙種類数.
//// 各テーブルのトレイごとの有効データ数.
//#define		PNLD_PaperKindNewTbl_Tray4Num						10	// st_PaperKindNewTblのトレイ4 用紙種類数.
#define		PNLD_PaperKindNewTbl_Tray2Num						11	// st_PaperKindNewTblのトレイ1 用紙種類数.
#define		PNLD_PaperKindNewTbl_Tray3Num						11	// st_PaperKindNewTblのトレイ1 用紙種類数.
#define		PNLD_PaperKindNewTbl_Tray4Num						11	// st_PaperKindNewTblのトレイ4 用紙種類数.
// Comment KronosS [課題No.1012563] <- 2017/02/21 OPS2 Sky竹菴 テーブルの要素番号定義を修正.
#else
#define		PNLD_PaperKindNewTbl_Tray1Num						21	// st_PaperKindNewTblのトレイ1 用紙種類数.			// comment 2018/03/29 SCD2 K.Sato [Eagle][RQ-722]用紙種別追加（厚紙1を2種類に分割）対応.
// Comment 2023/09/07 FPT-ThinhND40 [IT6_2.3] [ 機器構成変更_No.55 ] Removal of prohibition on letterhead of additional cassettes.
#define		PNLD_PaperKindNewTbl_Tray2Num						9	// st_PaperKindNewTblのトレイ1 用紙種類数.
#define		PNLD_PaperKindNewTbl_Tray3Num						9	// st_PaperKindNewTblのトレイ1 用紙種類数.
//comment 2023/07/25 FPT.LONGTH32 [IT6_2.3] 機器構成変更_No.75 Add tray 4 for Sparrow.
#define		PNLD_PaperKindNewTbl_Tray4Num						9	// st_PaperKindNewTblのトレイ4 用紙種類数.
#endif
#define		PNLD_PaperKindNewTbl_MaxNum							PNLD_PaperKindNewTbl_Tray1Num					// st_PaperKindNewTblの最大要素数.
// [KronosS] Sky荻堂 2016/09/26 管理No.22 用紙種類カスタマイズ.
#ifdef DEF_KRONOSS_Cus
#define		PNLD_AryPaperTypeBtnStatusTbl_Tray1Num				11	// uc_AryPaperTypeBtnStatusTblのトレイ1 用紙種類数.
// Comment KronosS [課題No.1012563] -> 2017/02/21 OPS2 Sky竹菴 テーブルの要素番号定義を修正.
//#define		PNLD_AryPaperTypeBtnStatusTbl_Tray2Num				10	// uc_AryPaperTypeBtnStatusTblのトレイ2 用紙種類数.
//#define		PNLD_AryPaperTypeBtnStatusTbl_Tray3Num				10	// uc_AryPaperTypeBtnStatusTblのトレイ3 用紙種類数.
//// [KronosS] Sky川名 2016/10/25 管理No21_44 各トレイ毎の表示用紙サイズをKronosS仕様にあわせる.
//#define		PNLD_AryPaperTypeBtnStatusTbl_Tray4Num				10	// uc_AryPaperTypeBtnStatusTblのトレイ4 用紙種類数.
#define		PNLD_AryPaperTypeBtnStatusTbl_Tray2Num				11	// uc_AryPaperTypeBtnStatusTblのトレイ2 用紙種類数.
#define		PNLD_AryPaperTypeBtnStatusTbl_Tray3Num				11	// uc_AryPaperTypeBtnStatusTblのトレイ3 用紙種類数.
#define		PNLD_AryPaperTypeBtnStatusTbl_Tray4Num				11	// uc_AryPaperTypeBtnStatusTblのトレイ4 用紙種類数.
// Comment KronosS [課題No.1012563] <- 2017/02/21 OPS2 Sky竹菴 テーブルの要素番号定義を修正.
#else
#define		PNLD_AryPaperTypeBtnStatusTbl_Tray1Num				21	// uc_AryPaperTypeBtnStatusTblのトレイ1 用紙種類数.			// comment 2018/03/29 SCD2 K.Sato [Eagle][RQ-722]用紙種別追加（厚紙1を2種類に分割）対応.
// Comment 2023/09/07 FPT-ThinhND40 [IT6_2.3] [ 機器構成変更_No.55 ] Removal of prohibition on letterhead of additional cassettes.
#define		PNLD_AryPaperTypeBtnStatusTbl_Tray2Num				9	// uc_AryPaperTypeBtnStatusTblのトレイ2 用紙種類数.
#define		PNLD_AryPaperTypeBtnStatusTbl_Tray3Num				9	// uc_AryPaperTypeBtnStatusTblのトレイ3 用紙種類数.
//comment 2023/07/25 FPT.LONGTH32 [IT6_2.3] 機器構成変更_No.75 Add tray 4 for Sparrow.
#define		PNLD_AryPaperTypeBtnStatusTbl_Tray4Num				9	// uc_AryPaperTypeBtnStatusTblのトレイ4 用紙種類数.
#endif
#define		PNLD_AryPaperTypeBtnStatusTbl_MaxNum				PNLD_AryPaperTypeBtnStatusTbl_Tray1Num			// uc_AryPaperTypeBtnStatusTblの最大要素数.
// [KronosS] Sky荻堂 2016/09/26 管理No.22 用紙種類カスタマイズ.
#ifdef DEF_KRONOSS_Cus
#define		PNLD_Tbl_TrayPaperKindBtnFromPaperType_Tray1Num		31	// tbl_TrayPaperKindBtnFromPaperTypeのトレイ1 用紙種類数.
// Comment KronosS [課題No.1012563] -> 2017/02/21 OPS2 Sky竹菴 テーブルの要素番号定義を修正.
//#define		PNLD_Tbl_TrayPaperKindBtnFromPaperType_Tray2Num		30	// tbl_TrayPaperKindBtnFromPaperTypeのトレイ2 用紙種類数.
//#define		PNLD_Tbl_TrayPaperKindBtnFromPaperType_Tray3Num		30	// tbl_TrayPaperKindBtnFromPaperTypeのトレイ3 用紙種類数.
//// [KronosS] Sky川名 2016/10/25 管理No21_44 各トレイ毎の表示用紙サイズをKronosS仕様にあわせる.
//#define		PNLD_Tbl_TrayPaperKindBtnFromPaperType_Tray4Num		30	// tbl_TrayPaperKindBtnFromPaperTypeのトレイ4 用紙種類数.
#define		PNLD_Tbl_TrayPaperKindBtnFromPaperType_Tray2Num		31	// tbl_TrayPaperKindBtnFromPaperTypeのトレイ2 用紙種類数.
#define		PNLD_Tbl_TrayPaperKindBtnFromPaperType_Tray3Num		31	// tbl_TrayPaperKindBtnFromPaperTypeのトレイ3 用紙種類数.
#define		PNLD_Tbl_TrayPaperKindBtnFromPaperType_Tray4Num		31	// tbl_TrayPaperKindBtnFromPaperTypeのトレイ4 用紙種類数.
// Comment KronosS [課題No.1012563] <- 2017/02/21 OPS2 Sky竹菴 テーブルの要素番号定義を修正.
#else
#define		PNLD_Tbl_TrayPaperKindBtnFromPaperType_Tray1Num		41	// tbl_TrayPaperKindBtnFromPaperTypeのトレイ1 用紙種類数.			// comment 2018/03/29 SCD2 K.Sato [Eagle][RQ-722]用紙種別追加（厚紙1を2種類に分割）対応.
// Comment 2023/09/07 FPT-ThinhND40 [IT6_2.3] [ 機器構成変更_No.55 ] Removal of prohibition on letterhead of additional cassettes.
#define		PNLD_Tbl_TrayPaperKindBtnFromPaperType_Tray2Num		29	// tbl_TrayPaperKindBtnFromPaperTypeのトレイ2 用紙種類数.
//#define		PNLD_Tbl_TrayPaperKindBtnFromPaperType_Tray3Num		28	// tbl_TrayPaperKindBtnFromPaperTypeのトレイ3 用紙種類数.
#define		PNLD_Tbl_TrayPaperKindBtnFromPaperType_Tray3Num		29	// tbl_TrayPaperKindBtnFromPaperTypeのトレイ3 用紙種類数.
//comment 2023/07/25 FPT.LONGTH32 [IT6_2.3] 機器構成変更_No.75 Add tray 4 for Sparrow.
#define		PNLD_Tbl_TrayPaperKindBtnFromPaperType_Tray4Num		29	// tbl_TrayPaperKindBtnFromPaperTypeのトレイ4 用紙種類数.
#endif
#define		PNLD_Tbl_TrayPaperKindBtnFromPaperType_MaxNum		PNLD_Tbl_TrayPaperKindBtnFromPaperType_Tray1Num	// tbl_TrayPaperKindBtnFromPaperTypeの最大要素数.
// [KronosS] Sky川名 2016/10/25 管理No21_44 各トレイ毎の表示用紙サイズをKronosS仕様にあわせる.
//comment 2023/07/25 FPT.LONGTH32 [IT6_2.3] 機器構成変更_No.75 Add tray 4 for Sparrow.
#define		PNLD_PaperTypeTrayMax								4	// トレイ数.
#endif // DEF_A4_Cus.
// A3A4FUM 用紙種類設定 t-okada 2015/01/19 End.
extern PA020114140		gp_A020114140;


//Comment [2015/10/09] [HCL_Prabakaran][MinervaSBK][CQ518365]New macros for PI usage.
#define PNLD_Paper_Copy_PopupMessageArea_Height 40
#define PNLD_PAPER_COPY_Close_Position_Y 0
#ifdef DEF_LCD_7INCH
#define PNLD_Paper_Copy_Close_Position_X 50
#define PNLD_Paper_Copy_Width 728
#define PNLD_Paper_Copy_Width_Wide 778
#else
#define PNLD_Paper_Copy_Close_Position_X 60
#define PNLD_Paper_Copy_Width 652
#define PNLD_Paper_Copy_Width_Wide 712
#endif
#define PNLD_Paper_Copy_Position_X (PNLD_POPUP_START_X(PNLD_Paper_Copy_Width))
#define PNLD_Paper_Copy_Wide_Position_X (PNLD_POPUP_START_X(PNLD_Paper_Copy_Width_Wide))	

// comment [2015/11/05] [OPS1-Sky延原][MinervaSBK][CQ520933] 選択トレイの設定変更キーとポストインサーターの用紙切れ表示が重なる
// 			ポストインサーター用紙切れ表示の表示位置移動座標値の定義 《5G_2.2》以降
#ifdef DEF_LCD_7INCH
#define PNLD_PI_Txt_PaperEmptyPI_Px (short)(532 + _ss_StartPositionX)
#define PNLD_PI_Txt_PaperEmptyPI_Py (short)(166 + _ss_StartPositionY)
#define PNLD_PI_Gra_PaperEmptyPI_Px (short)(524 + _ss_StartPositionX)
#define PNLD_PI_Gra_PaperEmptyPI_Py (short)(176 + _ss_StartPositionY)
#define PNLD_PI_Bak_PaperArea_F1_Px (short)(532 + _ss_StartPositionX)
#define PNLD_PI_Bak_PaperArea_F1_Py (short)(188 + _ss_StartPositionY)
#define PNLD_PI_Txt_SelPITray_F1_Px (short)(532 + _ss_StartPositionX)
#define PNLD_PI_Txt_SelPITray_F1_Py (short)(190 + _ss_StartPositionY)
#define PNLD_PI_Txt_XSizeText_F1_Px (short)(568 + _ss_StartPositionX)
#define PNLD_PI_Txt_XSizeText_F1_Py (short)(190 + _ss_StartPositionY)
#define PNLD_PI_Txt_YSizeText_F1_Px (short)(568 + _ss_StartPositionX)
#define PNLD_PI_Txt_YSizeText_F1_Py (short)(212 + _ss_StartPositionY)
#define PNLD_PI_Num_SizeX_Num_F1_Px (short)(584 + _ss_StartPositionX)
#define PNLD_PI_Num_SizeX_Num_F1_Py (short)(190 + _ss_StartPositionY)
#define PNLD_PI_Num_SizeY_Num_F1_Px (short)(584 + _ss_StartPositionX)
#define PNLD_PI_Num_SizeY_Num_F1_Py (short)(212 + _ss_StartPositionY)
#define PNLD_PI_Txt_PaperSize_F1_Px (short)(568 + _ss_StartPositionX)
#define PNLD_PI_Txt_PaperSize_F1_Py (short)(200 + _ss_StartPositionY)
#define PNLD_PI_Gra_RemainIcn_F1_Px (short)(684 + _ss_StartPositionX)
#define PNLD_PI_Gra_RemainIcn_F1_Py (short)(192 + _ss_StartPositionY)
#define PNLD_PI_Gra_PaperIcon_F1_Px (short)(684 + _ss_StartPositionX)
#define PNLD_PI_Gra_PaperIcon_F1_Py (short)(214 + _ss_StartPositionY)
#define PNLD_PI_Bak_PaperArea_F2_Px (short)(532 + _ss_StartPositionX)
#define PNLD_PI_Bak_PaperArea_F2_Py (short)(240 + _ss_StartPositionY)
#define PNLD_PI_Txt_SelPITray_F2_Px (short)(532 + _ss_StartPositionX)
#define PNLD_PI_Txt_SelPITray_F2_Py (short)(242 + _ss_StartPositionY)
#define PNLD_PI_Txt_XSizeText_F2_Px (short)(568 + _ss_StartPositionX)
#define PNLD_PI_Txt_XSizeText_F2_Py (short)(242 + _ss_StartPositionY)
#define PNLD_PI_Txt_YSizeText_F2_Px (short)(568 + _ss_StartPositionX)
#define PNLD_PI_Txt_YSizeText_F2_Py (short)(264 + _ss_StartPositionY)
#define PNLD_PI_Num_SizeX_Num_F2_Px (short)(584 + _ss_StartPositionX)
#define PNLD_PI_Num_SizeX_Num_F2_Py (short)(242 + _ss_StartPositionY)
#define PNLD_PI_Num_SizeY_Num_F2_Px (short)(584 + _ss_StartPositionX)
#define PNLD_PI_Num_SizeY_Num_F2_Py (short)(264 + _ss_StartPositionY)
#define PNLD_PI_Txt_PaperSize_F2_Px (short)(568 + _ss_StartPositionX)
#define PNLD_PI_Txt_PaperSize_F2_Py (short)(252 + _ss_StartPositionY)
#define PNLD_PI_Gra_RemainIcn_F2_Px (short)(684 + _ss_StartPositionX)
#define PNLD_PI_Gra_RemainIcn_F2_Py (short)(244 + _ss_StartPositionY)
#define PNLD_PI_Gra_PaperIcon_F2_Px (short)(684 + _ss_StartPositionX)
#define PNLD_PI_Gra_PaperIcon_F2_Py (short)(266 + _ss_StartPositionY)
#define PNLD_PI_Key_FormClose_Px (short)(702 + _ss_StartPositionX)
#define PNLD_PI_Key_FormClose_Py (short)(292 + _ss_StartPositionY)
#else
#define PNLD_PI_Txt_PaperEmptyPI_Px (short)(472 + _ss_StartPositionX)
#define PNLD_PI_Txt_PaperEmptyPI_Py (short)(186 + _ss_StartPositionY)
#define PNLD_PI_Gra_PaperEmptyPI_Px (short)(464 + _ss_StartPositionX)
#define PNLD_PI_Gra_PaperEmptyPI_Py (short)(196 + _ss_StartPositionY)
#define PNLD_PI_Bak_PaperArea_F1_Px (short)(472 + _ss_StartPositionX)
#define PNLD_PI_Bak_PaperArea_F1_Py (short)(208 + _ss_StartPositionY)
#define PNLD_PI_Txt_SelPITray_F1_Px (short)(472 + _ss_StartPositionX)
#define PNLD_PI_Txt_SelPITray_F1_Py (short)(208 + _ss_StartPositionY)
#define PNLD_PI_Txt_XSizeText_F1_Px (short)(508 + _ss_StartPositionX)
#define PNLD_PI_Txt_XSizeText_F1_Py (short)(212 + _ss_StartPositionY)
#define PNLD_PI_Txt_YSizeText_F1_Px (short)(508 + _ss_StartPositionX)
#define PNLD_PI_Txt_YSizeText_F1_Py (short)(228 + _ss_StartPositionY)
#define PNLD_PI_Num_SizeX_Num_F1_Px (short)(524 + _ss_StartPositionX)
#define PNLD_PI_Num_SizeX_Num_F1_Py (short)(212 + _ss_StartPositionY)
#define PNLD_PI_Num_SizeY_Num_F1_Px (short)(524 + _ss_StartPositionX)
#define PNLD_PI_Num_SizeY_Num_F1_Py (short)(228 + _ss_StartPositionY)
#define PNLD_PI_Txt_PaperSize_F1_Px (short)(508 + _ss_StartPositionX)
#define PNLD_PI_Txt_PaperSize_F1_Py (short)(220 + _ss_StartPositionY)
#define PNLD_PI_Gra_RemainIcn_F1_Px (short)(609 + _ss_StartPositionX)
#define PNLD_PI_Gra_RemainIcn_F1_Py (short)(210 + _ss_StartPositionY)
#define PNLD_PI_Gra_PaperIcon_F1_Px (short)(609 + _ss_StartPositionX)
#define PNLD_PI_Gra_PaperIcon_F1_Py (short)(230 + _ss_StartPositionY)
#define PNLD_PI_Bak_PaperArea_F2_Px (short)(472 + _ss_StartPositionX)
#define PNLD_PI_Bak_PaperArea_F2_Py (short)(256 + _ss_StartPositionY)
#define PNLD_PI_Txt_SelPITray_F2_Px (short)(472 + _ss_StartPositionX)
#define PNLD_PI_Txt_SelPITray_F2_Py (short)(256 + _ss_StartPositionY)
#define PNLD_PI_Txt_XSizeText_F2_Px (short)(508 + _ss_StartPositionX)
#define PNLD_PI_Txt_XSizeText_F2_Py (short)(260 + _ss_StartPositionY)
#define PNLD_PI_Txt_YSizeText_F2_Px (short)(508 + _ss_StartPositionX)
#define PNLD_PI_Txt_YSizeText_F2_Py (short)(276 + _ss_StartPositionY)
#define PNLD_PI_Num_SizeX_Num_F2_Px (short)(524 + _ss_StartPositionX)
#define PNLD_PI_Num_SizeX_Num_F2_Py (short)(260 + _ss_StartPositionY)
#define PNLD_PI_Num_SizeY_Num_F2_Px (short)(524 + _ss_StartPositionX)
#define PNLD_PI_Num_SizeY_Num_F2_Py (short)(276 + _ss_StartPositionY)
#define PNLD_PI_Txt_PaperSize_F2_Px (short)(508 + _ss_StartPositionX)
#define PNLD_PI_Txt_PaperSize_F2_Py (short)(268 + _ss_StartPositionY)
#define PNLD_PI_Gra_RemainIcn_F2_Px (short)(609 + _ss_StartPositionX)
#define PNLD_PI_Gra_RemainIcn_F2_Py (short)(258 + _ss_StartPositionY)
#define PNLD_PI_Gra_PaperIcon_F2_Px (short)(609 + _ss_StartPositionX)
#define PNLD_PI_Gra_PaperIcon_F2_Py (short)(278 + _ss_StartPositionY)
#define PNLD_PI_Key_FormClose_Px (short)(652 + _ss_StartPositionX)
#define PNLD_PI_Key_FormClose_Py (short)(304 + _ss_StartPositionY)
#endif
// comment [2015/11/05] [OPS1-Sky延原][MinervaSBK][CQ520933] 選択トレイの設定変更キーとポストインサーターの用紙切れ表示が重なる
// 			ポストインサーター用紙切れ表示の表示位置移動座標値の定義 《5G_2.1》以前
#ifdef DEF_LCD_7INCH
#define PNLD_Txt_PaperEmptyPI_Px (short)(532 + _ss_StartPositionX)
#define PNLD_Txt_PaperEmptyPI_Py (short)(126 + _ss_StartPositionY)
#define PNLD_Gra_PaperEmptyPI_Px (short)(524 + _ss_StartPositionX)
#define PNLD_Gra_PaperEmptyPI_Py (short)(136 + _ss_StartPositionY)
#define PNLD_Bak_PaperArea_F1_Px (short)(532 + _ss_StartPositionX)
#define PNLD_Bak_PaperArea_F1_Py (short)(148 + _ss_StartPositionY)
#define PNLD_Txt_SelPITray_F1_Px (short)(532 + _ss_StartPositionX)
#define PNLD_Txt_SelPITray_F1_Py (short)(150 + _ss_StartPositionY)
#define PNLD_Txt_XSizeText_F1_Px (short)(568 + _ss_StartPositionX)
#define PNLD_Txt_XSizeText_F1_Py (short)(150 + _ss_StartPositionY)
#define PNLD_Txt_YSizeText_F1_Px (short)(568 + _ss_StartPositionX)
#define PNLD_Txt_YSizeText_F1_Py (short)(172 + _ss_StartPositionY)
#define PNLD_Num_SizeX_Num_F1_Px (short)(584 + _ss_StartPositionX)
#define PNLD_Num_SizeX_Num_F1_Py (short)(150 + _ss_StartPositionY)
#define PNLD_Num_SizeY_Num_F1_Px (short)(584 + _ss_StartPositionX)
#define PNLD_Num_SizeY_Num_F1_Py (short)(172 + _ss_StartPositionY)
#define PNLD_Txt_PaperSize_F1_Px (short)(568 + _ss_StartPositionX)
#define PNLD_Txt_PaperSize_F1_Py (short)(160 + _ss_StartPositionY)
#define PNLD_Gra_RemainIcn_F1_Px (short)(684 + _ss_StartPositionX)
#define PNLD_Gra_RemainIcn_F1_Py (short)(152 + _ss_StartPositionY)
#define PNLD_Gra_PaperIcon_F1_Px (short)(684 + _ss_StartPositionX)
#define PNLD_Gra_PaperIcon_F1_Py (short)(174 + _ss_StartPositionY)
#define PNLD_Bak_PaperArea_F2_Px (short)(532 + _ss_StartPositionX)
#define PNLD_Bak_PaperArea_F2_Py (short)(200 + _ss_StartPositionY)
#define PNLD_Txt_SelPITray_F2_Px (short)(532 + _ss_StartPositionX)
#define PNLD_Txt_SelPITray_F2_Py (short)(202 + _ss_StartPositionY)
#define PNLD_Txt_XSizeText_F2_Px (short)(568 + _ss_StartPositionX)
#define PNLD_Txt_XSizeText_F2_Py (short)(202 + _ss_StartPositionY)
#define PNLD_Txt_YSizeText_F2_Px (short)(568 + _ss_StartPositionX)
#define PNLD_Txt_YSizeText_F2_Py (short)(224 + _ss_StartPositionY)
#define PNLD_Num_SizeX_Num_F2_Px (short)(584 + _ss_StartPositionX)
#define PNLD_Num_SizeX_Num_F2_Py (short)(202 + _ss_StartPositionY)
#define PNLD_Num_SizeY_Num_F2_Px (short)(584 + _ss_StartPositionX)
#define PNLD_Num_SizeY_Num_F2_Py (short)(224 + _ss_StartPositionY)
#define PNLD_Txt_PaperSize_F2_Px (short)(568 + _ss_StartPositionX)
#define PNLD_Txt_PaperSize_F2_Py (short)(212 + _ss_StartPositionY)
#define PNLD_Gra_RemainIcn_F2_Px (short)(684 + _ss_StartPositionX)
#define PNLD_Gra_RemainIcn_F2_Py (short)(204 + _ss_StartPositionY)
#define PNLD_Gra_PaperIcon_F2_Px (short)(684 + _ss_StartPositionX)
#define PNLD_Gra_PaperIcon_F2_Py (short)(226 + _ss_StartPositionY)
#define PNLD_Key_FormClose_Px (short)(652 + _ss_StartPositionX)
#define PNLD_Key_FormClose_Py (short)(292 + _ss_StartPositionY)
#else
#define PNLD_Txt_PaperEmptyPI_Px (short)(472 + _ss_StartPositionX)
#define PNLD_Txt_PaperEmptyPI_Py (short)(124 + _ss_StartPositionY)
#define PNLD_Gra_PaperEmptyPI_Px (short)(464 + _ss_StartPositionX)
#define PNLD_Gra_PaperEmptyPI_Py (short)(132 + _ss_StartPositionY)
#define PNLD_Bak_PaperArea_F1_Px (short)(472 + _ss_StartPositionX)
#define PNLD_Bak_PaperArea_F1_Py (short)(144 + _ss_StartPositionY)
#define PNLD_Txt_SelPITray_F1_Px (short)(472 + _ss_StartPositionX)
#define PNLD_Txt_SelPITray_F1_Py (short)(144 + _ss_StartPositionY)
#define PNLD_Txt_XSizeText_F1_Px (short)(508 + _ss_StartPositionX)
#define PNLD_Txt_XSizeText_F1_Py (short)(148 + _ss_StartPositionY)
#define PNLD_Txt_YSizeText_F1_Px (short)(508 + _ss_StartPositionX)
#define PNLD_Txt_YSizeText_F1_Py (short)(164 + _ss_StartPositionY)
#define PNLD_Num_SizeX_Num_F1_Px (short)(524 + _ss_StartPositionX)
#define PNLD_Num_SizeX_Num_F1_Py (short)(148 + _ss_StartPositionY)
#define PNLD_Num_SizeY_Num_F1_Px (short)(524 + _ss_StartPositionX)
#define PNLD_Num_SizeY_Num_F1_Py (short)(164 + _ss_StartPositionY)
#define PNLD_Txt_PaperSize_F1_Px (short)(508 + _ss_StartPositionX)
#define PNLD_Txt_PaperSize_F1_Py (short)(156 + _ss_StartPositionY)
#define PNLD_Gra_RemainIcn_F1_Px (short)(609 + _ss_StartPositionX)
#define PNLD_Gra_RemainIcn_F1_Py (short)(146 + _ss_StartPositionY)
#define PNLD_Gra_PaperIcon_F1_Px (short)(609 + _ss_StartPositionX)
#define PNLD_Gra_PaperIcon_F1_Py (short)(166 + _ss_StartPositionY)
#define PNLD_Bak_PaperArea_F2_Px (short)(472 + _ss_StartPositionX)
#define PNLD_Bak_PaperArea_F2_Py (short)(192 + _ss_StartPositionY)
#define PNLD_Txt_SelPITray_F2_Px (short)(472 + _ss_StartPositionX)
#define PNLD_Txt_SelPITray_F2_Py (short)(192 + _ss_StartPositionY)
#define PNLD_Txt_XSizeText_F2_Px (short)(508 + _ss_StartPositionX)
#define PNLD_Txt_XSizeText_F2_Py (short)(196 + _ss_StartPositionY)
#define PNLD_Txt_YSizeText_F2_Px (short)(508 + _ss_StartPositionX)
#define PNLD_Txt_YSizeText_F2_Py (short)(212 + _ss_StartPositionY)
#define PNLD_Num_SizeX_Num_F2_Px (short)(524 + _ss_StartPositionX)
#define PNLD_Num_SizeX_Num_F2_Py (short)(196 + _ss_StartPositionY)
#define PNLD_Num_SizeY_Num_F2_Px (short)(524 + _ss_StartPositionX)
#define PNLD_Num_SizeY_Num_F2_Py (short)(212 + _ss_StartPositionY)
#define PNLD_Txt_PaperSize_F2_Px (short)(508 + _ss_StartPositionX)
#define PNLD_Txt_PaperSize_F2_Py (short)(204 + _ss_StartPositionY)
#define PNLD_Gra_RemainIcn_F2_Px (short)(609 + _ss_StartPositionX)
#define PNLD_Gra_RemainIcn_F2_Py (short)(194 + _ss_StartPositionY)
#define PNLD_Gra_PaperIcon_F2_Px (short)(609 + _ss_StartPositionX)
#define PNLD_Gra_PaperIcon_F2_Py (short)(214 + _ss_StartPositionY)
#define PNLD_Key_FormClose_Px (short)(592 + _ss_StartPositionX)
#define PNLD_Key_FormClose_Py (short)(292 + _ss_StartPositionY)
#endif

/*================================================================================================
  列挙型変数のタグ宣言.
================================================================================================*/
struct TYPS_PaperSizeInfo {
	TYPE_SizeCode   e_SizeCode;				// 用紙サイズ.
	uchar			uc_BtnID;				// ボタンID.
};
// マルチパーパス用紙サイズテーブル（メトリック）.
static const TYPS_PaperSizeInfo st_AryPaperSizeMetricMPC[] = {
	{ TYPE_SizeCodeA3Sef, BTNID_KEY_PAPERSIZEA3T },
	{ TYPE_SizeCodeA4Lef, BTNID_KEY_PAPERSIZEA4Y },
	{ TYPE_SizeCodeB5Lef, BTNID_KEY_PAPERSIZEB5Y },
	{ TYPE_SizeCodeA5Lef, BTNID_KEY_PAPERSIZEA5Y },
	{ TYPE_SizeCodeB6Sef, BTNID_KEY_PAPERSIZEB6T },
	{ TYPE_SizeCodeB4Sef, BTNID_KEY_PAPERSIZEB4T },
	{ TYPE_SizeCodeA4Sef, BTNID_KEY_PAPERSIZEA4T },
	{ TYPE_SizeCodeB5Sef, BTNID_KEY_PAPERSIZEB5T },
	{ TYPE_SizeCodeA5Sef, BTNID_KEY_PAPERSIZEA5T },
	{ TYPE_SizeCodeA6Sef, BTNID_KEY_PAPERSIZEA6T },
};
// マルチ手差し用紙サイズテーブル（メトリック）.
static const TYPS_PaperSizeInfo st_AryPaperSizeMetricMultiMan[] = {
	{ TYPE_SizeCodeSRA3Sef, BTNID_PAPERSIZESRA3T },
	{ TYPE_SizeCodeA3Sef, BTNID_KEY_PAPERSIZEA3T },
	{ TYPE_SizeCodeA4Lef, BTNID_KEY_PAPERSIZEA4Y },
	{ TYPE_SizeCodeB5Lef, BTNID_KEY_PAPERSIZEB5Y },
	{ TYPE_SizeCodeA5Lef, BTNID_KEY_PAPERSIZEA5Y },
	{ TYPE_SizeCodeB6Sef, BTNID_KEY_PAPERSIZEB6T },
	{ TYPE_SizeCodeB4Sef, BTNID_KEY_PAPERSIZEB4T },
	{ TYPE_SizeCodeA4Sef, BTNID_KEY_PAPERSIZEA4T },
	{ TYPE_SizeCodeB5Sef, BTNID_KEY_PAPERSIZEB5T },
	{ TYPE_SizeCodeA5Sef, BTNID_KEY_PAPERSIZEA5T },
	{ TYPE_SizeCodeA6Sef, BTNID_KEY_PAPERSIZEA6T },
};
// マルチパーパス用紙サイズテーブル（インチ）.
static const TYPS_PaperSizeInfo st_AryPaperSizeInchMPC[] = {
	{ TYPE_SizeCodeLedgerSef,		BTNID_KEY_PAPERSIZE11_17INCH		},	// 11x17SEF.
	{ TYPE_SizeCodeLetterLef,		BTNID_KEY_PAPERSIZE8HALF_11INCHY	},	// 8_1/2x11LEF.
	{ TYPE_SizeCodeExecutiveLef,	BTNID_KEY_PAPERSIZE725_10HALFINCHY	},	// 7_1/4x10_1/2LEF.
	{ TYPE_SizeCodeInvoiceLef,		BTNID_KEY_PAPERSIZE5HALF_8HARFINCHY	},	// 5_1/2x8_1/2LEF.
	{ TYPE_SizeCodeNonStandard,		BTNID_KEY_WIDEPAPERSIZE12_18		},	// 12x18SEF.
	{ TYPE_SizeCodeLegalSef,		BTNID_KEY_PAPERSIZE8HALF_14INCH		},	// 8_1/2x14SEF.
	{ TYPE_SizeCodeLetterSef,		BTNID_KEY_PAPERSIZE8HALF_11INCHT	},	// 8_1/2x11SEL.
	{ TYPE_SizeCodeExecutiveSef,	BTNID_KEY_PAPERSIZE725_10HALFINCHT	},	// 7_1/4x10_1/2SEF.
	{ TYPE_SizeCodeInvoiceSef,		BTNID_KEY_PAPERSIZE5HALF_8HARFINCHT	},	// 5_1/2x8_1/2SEF.
};
// マルチ手差し用紙サイズテーブル（インチ）.
static const TYPS_PaperSizeInfo st_AryPaperSizeInchMultiMan[] = {
	{ TYPE_SizeCodeLedgerSef,		BTNID_KEY_PAPERSIZE11_17INCH		},	// 11x17SEF.
	{ TYPE_SizeCodeLetterLef,		BTNID_KEY_PAPERSIZE8HALF_11INCHY	},	// 8_1/2x11LEF.
	{ TYPE_SizeCodeExecutiveLef,	BTNID_KEY_PAPERSIZE725_10HALFINCHY	},	// 7_1/4x10_1/2LEF.
	{ TYPE_SizeCodeInvoiceLef,		BTNID_KEY_PAPERSIZE5HALF_8HARFINCHY	},	// 5_1/2x8_1/2LEF.
	{ TYPE_SizeCode12x18Sef,		BTNID_KEY_WIDEPAPERSIZE12_18		},	// 12x18SEF.
	{ TYPE_SizeCodeLegalSef,		BTNID_KEY_PAPERSIZE8HALF_14INCH		},	// 8_1/2x14SEF.
	{ TYPE_SizeCodeLetterSef,		BTNID_KEY_PAPERSIZE8HALF_11INCHT	},	// 8_1/2x11SEL.
	{ TYPE_SizeCodeExecutiveSef,	BTNID_KEY_PAPERSIZE725_10HALFINCHT	},	// 7_1/4x10_1/2SEF.
	{ TYPE_SizeCodeInvoiceSef,		BTNID_KEY_PAPERSIZE5HALF_8HARFINCHT	},	// 5_1/2x8_1/2SEF.
};
// マルチパーパス用紙サイズテーブル（8開・16開:日本仕向）.
static const TYPS_PaperSizeInfo st_AryPaperSize8K16KJpMPC[] = {
	{ TYPE_SizeCodeFlsSef,			BTNID_KEY_PAPERSIZEFLS		},	// Foolscap.
	{ TYPE_SizeCode16KLef,			BTNID_KEY_PAPERSIZE16KYINCH	},
	{ TYPE_SizeCodePostcard_JpSef,	BTNID_KEY_PAPERSIZEPOSTCARD	},	// はがきSEF.
	{ TYPE_SizeCode8KSef,			BTNID_KEY_PAPERSIZE8KTINCH	},
	{ TYPE_SizeCode16KSef,			BTNID_KEY_PAPERSIZE16KTINCH	},
};
// マルチ手差し用紙サイズテーブル（8開・16開:日本仕向）.
static const TYPS_PaperSizeInfo st_AryPaperSize8K16KJpMultiMan[] = {
	{ TYPE_SizeCodeFlsSef,				BTNID_KEY_PAPERSIZEFLS				},	// Foolscap.
	{ TYPE_SizeCode16KLef,				BTNID_KEY_PAPERSIZE16KYINCH			},
	{ TYPE_SizeCodePostcard_JpSef,		BTNID_KEY_PAPERSIZEPOSTCARD			},	// はがきSEF.
	{ TYPE_SizeCodeEnvelopeC4,			BTNID_KEY_PAPERSIZEENVELOPEC4T		},	// envC4.
	{ TYPE_SizeCodeEnvelopeDL,			BTNID_KEY_PAPERSIZEENVELOPEDLT		},
	{ TYPE_SizeCodeEnvelopeMornarch,	BTNID_KEY_PAPERSIZEENVELOPEMONARCHT	},
	{ TYPE_SizeCode8KSef,				BTNID_KEY_PAPERSIZE8KTINCH			},
	{ TYPE_SizeCode16KSef,				BTNID_KEY_PAPERSIZE16KTINCH			},
	{ TYPE_SizeCodeEnvelopeB5,			BTNID_KEY_PAPERSIZEENVELOPEB5T		},
	{ TYPE_SizeCodeEnvelopeC5,			BTNID_KEY_PAPERSIZEENVELOPEC5T		},
	{ TYPE_SizeCodeEnvelopeCom10,		BTNID_KEY_PAPERSIZEENVELOPECOM10T	},
	{ TYPE_SizeCodeJpLetterY2,			BTNID_KEY_PAPERSIZEENVELOPEY2T		},	// env洋形2号.
	{ TYPE_SizeCodeJpLetterY3,			BTNID_KEY_PAPERSIZEENVELOPEY3T		},	// env洋形3号.
	{ TYPE_SizeCodeJpLetterL3,			BTNID_KEY_PAPERSIZEENVELOPEN3T		},	// env長形3号.
	{ TYPE_SizeCodeJpLetterK1,			BTNID_KEY_PAPERSIZEENVELOPEK1T		},	// env角形1号.
	{ TYPE_SizeCodeJpLetterK3,			BTNID_KEY_PAPERSIZEENVELOPEK3T		},	// env角形3号.
	{ TYPE_SizeCodeJpLetterY4,			BTNID_KEY_PAPERSIZEENVELOPEY4T		},	// env洋形4号.
	{ TYPE_SizeCodeJpLetterL4,			BTNID_KEY_PAPERSIZEENVELOPEN4T		},	// env長形4号.
	{ TYPE_SizeCodeJpLetterK2,			BTNID_KEY_PAPERSIZEENVELOPEK2T		},	// env角形2号.
};

// マルチパーパス用紙サイズテーブル（8開・16開:北米仕向）.
static const TYPS_PaperSizeInfo st_AryPaperSize8K16KUsMPC[] = {
	{ TYPE_SizeCodeFlsSef,			BTNID_KEY_PAPERSIZEFLS		},	// Foolscap.
	{ TYPE_SizeCode16KLef,			BTNID_KEY_PAPERSIZE16KYINCH	},
	{ TYPE_SizeCodeIN4x6Sef,		BTNID_KEY_PAPERSIZEPOSTCARD	},	// 4x6SEF.
	{ TYPE_SizeCode8KSef,			BTNID_KEY_PAPERSIZE8KTINCH	},
	{ TYPE_SizeCode16KSef,			BTNID_KEY_PAPERSIZE16KTINCH	},
};
// マルチ手差し用紙サイズテーブル（8開・16開:北米仕向）.
static const TYPS_PaperSizeInfo st_AryPaperSize8K16KUsMultiMan[] = {
	{ TYPE_SizeCodeFlsSef,				BTNID_KEY_PAPERSIZEFLS				},	// Foolscap.
	{ TYPE_SizeCode16KLef,				BTNID_KEY_PAPERSIZE16KYINCH			},
	{ TYPE_SizeCodeIN4x6Sef,			BTNID_KEY_PAPERSIZEPOSTCARD			},	// 4x6SEF.
	{ TYPE_SizeCodeEnvelopeC4,			BTNID_KEY_PAPERSIZEENVELOPEC4T		},	// envC4.
	{ TYPE_SizeCodeJpLetterY2,			BTNID_KEY_PAPERSIZEENVELOPEY2T		},	// envC6.
	{ TYPE_SizeCodeEnvelopeCom10,		BTNID_KEY_PAPERSIZEENVELOPECOM10T	},
	{ TYPE_SizeCode8KSef,				BTNID_KEY_PAPERSIZE8KTINCH			},
	{ TYPE_SizeCode16KSef,				BTNID_KEY_PAPERSIZE16KTINCH			},
	{ TYPE_SizeCodeEnvelopeB5,			BTNID_KEY_PAPERSIZEENVELOPEB5T		},
	{ TYPE_SizeCodeEnvelopeC5,			BTNID_KEY_PAPERSIZEENVELOPEC5T		},
	{ TYPE_SizeCodeEnvelopeDL,			BTNID_KEY_PAPERSIZEENVELOPEDLT		},
	{ TYPE_SizeCodeEnvelopeMornarch,	BTNID_KEY_PAPERSIZEENVELOPEMONARCHT	},
	{ TYPE_SizeCodeJpLetterY3,			BTNID_KEY_PAPERSIZEENVELOPEY3T		},	// env洋形3号.
	{ TYPE_SizeCodeJpLetterL3,			BTNID_KEY_PAPERSIZEENVELOPEN3T		},	// env長形3号.
	{ TYPE_SizeCodeJpLetterK1,			BTNID_KEY_PAPERSIZEENVELOPEK1T		},	// env角形1号.
	{ TYPE_SizeCodeJpLetterK3,			BTNID_KEY_PAPERSIZEENVELOPEK3T		},	// env角形3号.
	{ TYPE_SizeCodeJpLetterY4,			BTNID_KEY_PAPERSIZEENVELOPEY4T		},	// env洋形4号.
	{ TYPE_SizeCodeJpLetterL4,			BTNID_KEY_PAPERSIZEENVELOPEN4T		},	// env長形4号.
	{ TYPE_SizeCodeJpLetterK2,			BTNID_KEY_PAPERSIZEENVELOPEK2T		},	// env角形2号.
};
// マルチパーパス用紙サイズテーブル（8開・16開:欧州仕向）.
static const TYPS_PaperSizeInfo st_AryPaperSize8K16KEuMPC[] = {
	{ TYPE_SizeCodeFlsSef,			BTNID_KEY_PAPERSIZEFLS		},	// Foolscap.
	{ TYPE_SizeCode16KLef,			BTNID_KEY_PAPERSIZE16KYINCH	},
	{ TYPE_SizeCodePostcard_EuSef,	BTNID_KEY_PAPERSIZEPOSTCARD	},	// A6cardSEF.
	{ TYPE_SizeCode8KSef,			BTNID_KEY_PAPERSIZE8KTINCH	},
	{ TYPE_SizeCode16KSef,			BTNID_KEY_PAPERSIZE16KTINCH	},
};
// マルチ手差し用紙サイズテーブル（8開・16開:欧州仕向）.
static const TYPS_PaperSizeInfo st_AryPaperSize8K16KEuMultiMan[] = {
	{ TYPE_SizeCodeFlsSef,				BTNID_KEY_PAPERSIZEFLS				},	// Foolscap.
	{ TYPE_SizeCode16KLef,				BTNID_KEY_PAPERSIZE16KYINCH			},
	{ TYPE_SizeCodePostcard_EuSef,		BTNID_KEY_PAPERSIZEPOSTCARD			},	// A6cardSEF.
	{ TYPE_SizeCodeEnvelopeC4,			BTNID_KEY_PAPERSIZEENVELOPEC4T		},	// envC4.
	{ TYPE_SizeCodeJpLetterY2,			BTNID_KEY_PAPERSIZEENVELOPEY2T		},	// envC6.
	{ TYPE_SizeCodeEnvelopeCom10,		BTNID_KEY_PAPERSIZEENVELOPECOM10T	},
	{ TYPE_SizeCode8KSef,				BTNID_KEY_PAPERSIZE8KTINCH			},
	{ TYPE_SizeCode16KSef,				BTNID_KEY_PAPERSIZE16KTINCH			},
	{ TYPE_SizeCodeEnvelopeB5,			BTNID_KEY_PAPERSIZEENVELOPEB5T		},
	{ TYPE_SizeCodeEnvelopeC5,			BTNID_KEY_PAPERSIZEENVELOPEC5T		},
	{ TYPE_SizeCodeEnvelopeDL,			BTNID_KEY_PAPERSIZEENVELOPEDLT		},
	{ TYPE_SizeCodeEnvelopeMornarch,	BTNID_KEY_PAPERSIZEENVELOPEMONARCHT	},
	{ TYPE_SizeCodeJpLetterY3,			BTNID_KEY_PAPERSIZEENVELOPEY3T		},	// env洋形3号.
	{ TYPE_SizeCodeJpLetterL3,			BTNID_KEY_PAPERSIZEENVELOPEN3T		},	// env長形3号.
	{ TYPE_SizeCodeJpLetterK1,			BTNID_KEY_PAPERSIZEENVELOPEK1T		},	// env角形1号.
	{ TYPE_SizeCodeJpLetterK3,			BTNID_KEY_PAPERSIZEENVELOPEK3T		},	// env角形3号.
	{ TYPE_SizeCodeJpLetterY4,			BTNID_KEY_PAPERSIZEENVELOPEY4T		},	// env洋形4号.
	{ TYPE_SizeCodeJpLetterL4,			BTNID_KEY_PAPERSIZEENVELOPEN4T		},	// env長形4号.
	{ TYPE_SizeCodeJpLetterK2,			BTNID_KEY_PAPERSIZEENVELOPEK2T		},	// env角形2号.
};
// 用紙サイズテーブルサイズ.
const uchar uc_PaperSizeMetricTblSizeMPC= ( uchar )( sizeof( st_AryPaperSizeMetricMPC ) / sizeof( TYPS_PaperSizeInfo ) );
const uchar uc_PaperSizeMetricTblSizeMultiMan = ( uchar )( sizeof( st_AryPaperSizeMetricMultiMan ) / sizeof( TYPS_PaperSizeInfo ) );
const uchar uc_PaperSizeInchTblSizeMPC= ( uchar )( sizeof( st_AryPaperSizeInchMPC ) / sizeof( TYPS_PaperSizeInfo ) );
const uchar uc_PaperSizeInchTblSizeMultiMan = ( uchar )( sizeof( st_AryPaperSizeInchMultiMan ) / sizeof( TYPS_PaperSizeInfo ) );
const uchar uc_PaperSize8K16KJpTblSizeMPC= ( uchar )( sizeof( st_AryPaperSize8K16KJpMPC ) / sizeof( TYPS_PaperSizeInfo ) );
const uchar uc_PaperSize8K16KJpTblSizeMultiMan = ( uchar )( sizeof( st_AryPaperSize8K16KJpMultiMan ) / sizeof( TYPS_PaperSizeInfo ) );
const uchar uc_PaperSize8K16KUsTblSizeMPC= ( uchar )( sizeof( st_AryPaperSize8K16KUsMPC ) / sizeof( TYPS_PaperSizeInfo ) );
const uchar uc_PaperSize8K16KUsTblSizeMultiMan = ( uchar )( sizeof( st_AryPaperSize8K16KUsMultiMan ) / sizeof( TYPS_PaperSizeInfo ) );
const uchar uc_PaperSize8K16KEuTblSizeMPC= ( uchar )( sizeof( st_AryPaperSize8K16KEuMPC ) / sizeof( TYPS_PaperSizeInfo ) );
const uchar uc_PaperSize8K16KEuTblSizeMultiMan = ( uchar )( sizeof( st_AryPaperSize8K16KEuMultiMan ) / sizeof( TYPS_PaperSizeInfo ) );

// デフォルト用紙サイズの優先順位のテーブル(メトリック).
static const TYPE_SizeCode PaperSizeMetoricPriorityTbl[] =
{
	TYPE_SizeCodeA4Lef,
	TYPE_SizeCodeA4Sef,
	TYPE_SizeCodeB4Sef,
	TYPE_SizeCodeA3Sef,
	TYPE_SizeCodeB5Lef,
	TYPE_SizeCodeB5Sef,
	TYPE_SizeCodeA5Lef,
	TYPE_SizeCodeA5Sef,
	TYPE_SizeCodeB6Sef,
	TYPE_SizeCodeA6Sef,
	TYPE_SizeCodeA4Lef, 	// 全てNGだった場合、優先順最上位のサイズを強制的にセットする.
};
const uchar uc_PaperSizeMetoricPriority = ( uchar )( sizeof( PaperSizeMetoricPriorityTbl ) / sizeof( TYPE_SizeCode ) );

// デフォルト用紙サイズの優先順位のテーブル(メトリック)ワイド紙.
static const TYPE_SizeCode PaperSizeMetoricWidePriorityTbl[] =
{
	TYPE_SizeCodeA4Lef,
	TYPE_SizeCodeA4Sef,
	TYPE_SizeCodeB4Sef,
	TYPE_SizeCodeA3Sef,
	TYPE_SizeCodeB5Lef,
	TYPE_SizeCodeB5Sef,
	TYPE_SizeCodeA5Lef,
	TYPE_SizeCodeA5Sef,
	TYPE_SizeCodeA4Lef, 	// 全てNGだった場合、優先順最上位のサイズを強制的にセットする.
};
const uchar uc_PaperSizeMetoricWidePriority = ( uchar )( sizeof( PaperSizeMetoricWidePriorityTbl ) / sizeof( TYPE_SizeCode ) );

// デフォルト用紙サイズの優先順位のテーブル(インチ).
static const TYPE_SizeCode PaperSizeInchPriorityTbl[] =
{
	TYPE_SizeCodeLetterLef,		// 8_1/2x11LEF.
	TYPE_SizeCodeLetterSef,		// 8_1/2x11SEF.
	TYPE_SizeCodeLegalSef,		// 8_1/2x14SEF.
	TYPE_SizeCodeLedgerSef,		// 11x17SEF.
	TYPE_SizeCode12x18Sef,		// 12x18SEF.
	TYPE_SizeCodeExecutiveLef,	// 7_1/4x10_1/2LEF.
	TYPE_SizeCodeExecutiveSef,	// 7_1/4x10_1/2SEF.
	TYPE_SizeCodeInvoiceLef,	// 5_1/2x8_1/2LEF.
	TYPE_SizeCodeInvoiceSef,	// 5_1/2x8_1/2SEF.
	TYPE_SizeCodeLetterLef,		// 8_1/2x11LEF  	// 全てNGだった場合、優先順最上位のサイズを強制的にセットする.
};
const uchar uc_PaperSizeInchPriority = ( uchar )( sizeof( PaperSizeInchPriorityTbl ) / sizeof( TYPE_SizeCode ) );

// デフォルト用紙サイズの優先順位のテーブル(インチ)ワイド紙.
static const TYPE_SizeCode PaperSizeInchWidePriorityTbl[] =
{
	TYPE_SizeCodeLetterLef,		// 8_1/2x11LEF.
	TYPE_SizeCodeLetterSef,		// 8_1/2x11SEF.
	TYPE_SizeCodeLedgerSef,		// 11x17SEF.
	TYPE_SizeCodeInvoiceLef,	// 5_1/2x8_1/2LEF.
	TYPE_SizeCodeInvoiceSef,	// 5_1/2x8_1/2SEF.
	TYPE_SizeCodeLetterLef,		// 8_1/2x11LEF  	// 全てNGだった場合、優先順最上位のサイズを強制的にセットする.
};
const uchar uc_PaperSizeInchWidePriority = ( uchar )( sizeof( PaperSizeInchWidePriorityTbl ) / sizeof( TYPE_SizeCode ) );

// デフォルト用紙サイズの優先順位のテーブル(その他).
static const TYPE_SizeCode PaperSize8K16KPriorityTbl[] =
{
	TYPE_SizeCode16KLef,
	TYPE_SizeCode16KSef,
	TYPE_SizeCode8KSef,
	TYPE_SizeCodeFlsSef,			// Foolscap.
	TYPE_SizeCodePostcard_JpSef,	// はがきSEF.
	TYPE_SizeCodeEnvelopeB5,
	TYPE_SizeCodeEnvelopeC5,
	TYPE_SizeCodeEnvelopeDL,
	TYPE_SizeCodeEnvelopeCom10,
	TYPE_SizeCodeEnvelopeMornarch,
	TYPE_SizeCode16KLef,			// 全てNGだった場合、優先順最上位のサイズを強制的にセットする.
};
const uchar uc_PaperSize8K16KPriority = ( uchar )( sizeof( PaperSize8K16KPriorityTbl ) / sizeof( TYPE_SizeCode ) );

// 用紙サイズ警告テーブル.
static const TYPE_WarningName PaperSizeErrorTbl[] =
{
	PERROR_StaplePaperSizeERROR_S,				// ステープル用紙サイズ不適合(プリント開始前) .
	PERROR_SaddlePaperSizeERROR_S,				// サドル用紙サイズ不適合(プリント開始前) .
	PERROR_PunchPaperSizeERROR_S,				// パンチ用紙サイズ不適合(プリント開始前) .
	PERROR_CenterStapleSizeERROR_S,				// センターステープル用紙サイズ不適合(プリント開始前).
	PERROR_FoldPaperSizeError_S,				// 折り用紙サイズ不適合(プリント開始前) .
	PERROR_StapleAndPunchPaperSizeError_S,		// ステープル＆パンチ用紙サイズ不適合(プリント開始前) .
};
const uchar uc_PaperSizeError = ( uchar )( sizeof( PaperSizeErrorTbl ) / sizeof( TYPE_WarningName ) );

// ワイド紙用紙サイズテーブル.
struct PaperSizeSelectTableWide {
	uchar				uc_option;			// オプション.
	TYPE_SizeCode		e_SizeCode;			// 用紙サイズコード.
	PNormalButton*		p_WidePaperKey;		// ワイド紙キーのポインタ.
	Bool				b_isTate;			// ＳＥＦ方向かの判断.
};
// ワイド紙用紙サイズテーブル（メトリック）.
static const struct PaperSizeSelectTableWide st_PaperSizeSelectTableWideMetric[] = {
// Comment 2019/10/14 [HCLT_Shalini] [IT6_HeliosMLK] [ 機器構成変更_No12-4 ] Support for A3WSEF Key for HeliosMLK.
//// comment 2016/09/07 SCD2-Sky後藤 [IT5_3.3][1003602].
//// Comment 2019/07/26 [HCLT_Paul] [IT6_HeliosMLK] [ Seigyu.10 ] Adding the HeliosMLK compiler switch.
//#if !defined(DEF_Zeus5BkCus) && !defined(DEF_HeliosMLKCus)
#if !defined(DEF_Zeus5BkCus) 
	{ BTNID_KEY_A3WSEF,	TYPE_SizeCodeA3Sef,	&Key_SheetSizeA3WSEF,	True	},	// A3WT.
// comment 2016/09/07 SCD2-Sky後藤 [IT5_3.3][1003602].
#endif
//#endif
	{ BTNID_KEY_B4WSEF,	TYPE_SizeCodeB4Sef,	&Key_SheetSizeB4WSEF,	True	},	// B4WT.
	{ BTNID_KEY_A4WLEF,	TYPE_SizeCodeA4Lef,	&Key_SheetSizeA4WLEF,	False	},	// A4WY.
	{ BTNID_KEY_A4WSEF,	TYPE_SizeCodeA4Sef,	&Key_SheetSizeA4WSEF,	True	},	// A4WT.
	{ BTNID_KEY_B5WLEF,	TYPE_SizeCodeB5Lef,	&Key_SheetSizeB5WLEF,	False	},	// B5WY.
	{ BTNID_KEY_B5WSEF,	TYPE_SizeCodeB5Sef,	&Key_SheetSizeB5WSEF,	True	},	// B5WT.
	{ BTNID_KEY_A5WLEF,	TYPE_SizeCodeA5Lef,	&Key_SheetSizeA5WLEF,	False	},	// A5WY.
	{ BTNID_KEY_A5WSEF,	TYPE_SizeCodeA5Sef,	&Key_SheetSizeA5WSEF,	True	},	// A5WT.
};
// ワイド紙用紙サイズテーブル（インチ）.
static const struct PaperSizeSelectTableWide st_PaperSizeSelectTableWideInch[] = {
// Comment 2019/10/14 [HCLT_Shalini] [IT6_HeliosMLK] [ 機器構成変更_No12-4 ] Support for 11x17WSEF Key for HeliosMLK.
//// comment 2016/09/07 SCD2-Sky後藤 [IT5_3.3][1003602].
//// Comment 2019/07/26 [HCLT_Paul] [IT6_HeliosMLK] [ Seigyu.10 ] Adding the HeliosMLK compiler switch.
//#if !defined(DEF_Zeus5BkCus) && !defined(DEF_HeliosMLKCus)
#if !defined(DEF_Zeus5BkCus) 
	{ BTNID_KEY_WIDEPAPER11_17INCHT,		TYPE_SizeCodeLedgerSef,		&Key_SheetSize11x17WSEF,	True	},	// 11×17.
// comment 2016/09/07 SCD2-Sky後藤 [IT5_3.3][1003602].
#endif
//#endif
	{ BTNID_KEY_WIDEPAPERLETTERINCHY,		TYPE_SizeCodeLetterLef,		&Key_SheetSize8Hx11WLEF,	False	},	// Letterの横.
	{ BTNID_KEY_WIDEPAPERLETTERINCHT,		TYPE_SizeCodeLetterSef,		&Key_SheetSize8Hx11WSEF,	True	},	// Letterの縦.
	{ BTNID_KEY_WIDEPAPER8HALF_5HALFINCHY,	TYPE_SizeCodeInvoiceLef,	&Key_SheetSize5hx8hWLEF,	False	},	// 8 1/2×5 1/2の横.
	{ BTNID_KEY_WIDEPAPER8HALF_5HALFINCHT,	TYPE_SizeCodeInvoiceSef,	&Key_SheetSize5hx8hWSEF,	True	},	// 8 1/2×5 1/2の縦.
};
// ワイド紙用紙サイズテーブルサイズ.
const uchar uc_PaperSizeSelectTableWideMetric = ( uchar )( sizeof( st_PaperSizeSelectTableWideMetric ) / sizeof( PaperSizeSelectTableWide ) );	// メトリック.
const uchar uc_PaperSizeSelectTableWideInch = ( uchar )( sizeof( st_PaperSizeSelectTableWideInch ) / sizeof( PaperSizeSelectTableWide ) );		// インチ.

// 用紙ワイド紙 : 用紙サイズ/方向表示テーブル.
static const struct PNLC_SizeCode_PaperSizeWide {
	TYPE_SizeCode	size;			// サイズコード.
	ushort			us_TextID;		// サイズ表示テキスト.
	ushort			us_WideTextID;	// サイズ表示テキスト（ワイド紙).
} tbl_SizeCode_PaperSizeWide[] = {
#ifdef DEF_LCD_7INCH
	{ TYPE_SizeCodeA3Sef,			s_pNOTSheetSize_A3SEF_W096H018A,		s_pNOTSheetSize_A3WSEF_W096H018A },			// A3 SEF.
	{ TYPE_SizeCodeA4Sef,			s_pNOTA4SEF_W096H018A,					s_pNOTSheetSize_A4WSEF_W096H018A },			// A4 SEF.
	{ TYPE_SizeCodeA4Lef,			s_pNOTSheetSize_A4LEF_W096H018A,		s_pNOTSheetSize_A4WLEF_W096H018A },			// A4 LEF.
	{ TYPE_SizeCodeA5Sef,			s_pNOTSheetSize_A5SEF_W096H018A,		s_pNOTSheetSize_A5WSEF_W096H018A },			// A5 SEF.
	{ TYPE_SizeCodeA5Lef,			s_pNOTSheetSize_A4LEF_W096H018A,		s_pNOTSheetSize_A5WLEF_W096H018A },			// A5 LEF.
	{ TYPE_SizeCodeA6Sef,			s_pNOTSheetSize_A6SEF_W096H018A,		s_Dummy },									// A6 SEF.
	{ TYPE_SizeCodeB4Sef,			s_pNOTSheetSize_B4SEF_W096H018A,		s_pNOTSheetSize_B4WSEF_W096H018A },			// B4 SEF.
	{ TYPE_SizeCodeB5Sef,			s_pNOTSheetSize_B5SEF_W096H018A,		s_pNOTSheetSize_B5WSEF_W096H018A },			// B5 SEF.
	{ TYPE_SizeCodeB5Lef,			s_pNOTSheetSize_B5LEF_W096H018A,		s_pNOTSheetSize_B5WLEF_W096H018A },			// B5 LEF.
	{ TYPE_SizeCodeB6Sef,			s_pNOTSheetSize_B6SEF_W096H018A,		s_Dummy },									// B6 SEF.
	{ TYPE_SizeCodeLedgerSef,		s_pNOTSheetSize_11x17SEF_W096H018A,		s_pNOTSheetSize_11x17WSEF_W096H018A },		// 11×17 SEF.
	{ TYPE_SizeCodeLegalSef,		s_pNOTSheetSize_812x14SEF_W096H018A,	s_Dummy },									// 8 1/2×14 SEF.
	{ TYPE_SizeCodeLetterSef,		s_pNOTSheetSize_812x11SEF_W096H018A,	s_pNOTSheetSize_812x11WSEF_W096H018A },		// 8 1/2×11 SEF.
	{ TYPE_SizeCodeLetterLef,		s_pNOTSheetSize_812x11LEF_W096H018A,	s_pNOTSheetSize_812x11WLEF_W096H018A },		// 8 1/2×11 LEF.
	{ TYPE_SizeCodeInvoiceSef,		s_pNOTSheetSize_512x812_W096H018A,		s_pNOTSheetSize_512x812WSEF_W096H018A },	// 5 1/2×8 1/2 SEF.
	{ TYPE_SizeCodeInvoiceLef,		s_pNOTSheetSize_512x812LEF_W096H018A,	s_pNOTSheetSize_512x812WLEF_W096H018A },	// 5 1/2×8 1/2 LEF.
	{ TYPE_SizeCodeExecutiveSef,	s_pNOTSheetSize_714x1012_W096H018A,		s_Dummy },									// 7 1/4×10 1/2 SEF.
	{ TYPE_SizeCodeExecutiveLef,	s_pNOTSheetSize_714x1012LEF_W096H018A,	s_Dummy },									// 7 1/4×10 1/2 LEF.
	{ TYPE_SizeCode12x18Sef,		s_pNOTSheetSize_12x18_W096H018A,		s_pNOTSheetSize_12x18_W096H018A },			// 12×18.
	{ TYPE_SizeCodeSRA3Sef,			s_pNOTSheetSize_SRA3SEF_W096H018A,		s_Dummy },									// SRA3【CQ.348170】.
#else //  DEF_LCD_7INCH.
	{ TYPE_SizeCodeA3Sef,			s_pNOTSheetSize_A3SEF_W096H016A,		s_pNOTSheetSize_A3WSEF_W096H016A },			// A3 SEF.
	{ TYPE_SizeCodeA4Sef,			s_pNOTA4SEF_W096H016A,					s_pNOTSheetSize_A4WSEF_W096H016A },			// A4 SEF.
	{ TYPE_SizeCodeA4Lef,			s_pNOTSheetSize_A4LEF_W096H016A,		s_pNOTSheetSize_A4WLEF_W096H016A },			// A4 LEF.
	{ TYPE_SizeCodeA5Sef,			s_pNOTSheetSize_A5SEF_W096H016A,		s_pNOTSheetSize_A5WSEF_W096H016A },			// A5 SEF.
	{ TYPE_SizeCodeA5Lef,			s_pNOTSheetSize_A4LEF_W096H016A,		s_pNOTSheetSize_A5WLEF_W096H016A },			// A5 LEF.
	{ TYPE_SizeCodeA6Sef,			s_pNOTSheetSize_A6SEF_W096H016A,		s_Dummy },									// A6 SEF.
	{ TYPE_SizeCodeB4Sef,			s_pNOTSheetSize_B4SEF_W096H016A,		s_pNOTSheetSize_B4WSEF_W096H016A },			// B4 SEF.
	{ TYPE_SizeCodeB5Sef,			s_pNOTSheetSize_B5SEF_W096H016A,		s_pNOTSheetSize_B5WSEF_W096H016A },			// B5 SEF.
	{ TYPE_SizeCodeB5Lef,			s_pNOTSheetSize_B5LEF_W096H016A,		s_pNOTSheetSize_B5WLEF_W096H016A },			// B5 LEF.
	{ TYPE_SizeCodeB6Sef,			s_pNOTSheetSize_B6SEF_W096H016A,		s_Dummy },									// B6 SEF.
	{ TYPE_SizeCodeLedgerSef,		s_pNOTSheetSize_11x17SEF_W096H016A,		s_pNOTSheetSize_11x17WSEF_W096H016A },		// 11×17 SEF.
	{ TYPE_SizeCodeLegalSef,		s_pNOTSheetSize_812x14SEF_W096H016A,	s_Dummy },									// 8 1/2×14 SEF.
	{ TYPE_SizeCodeLetterSef,		s_pNOTSheetSize_812x11SEF_W096H016A,	s_pNOTSheetSize_812x11WSEF_W096H016A },		// 8 1/2×11 SEF.
	{ TYPE_SizeCodeLetterLef,		s_pNOTSheetSize_812x11LEF_W096H016A,	s_pNOTSheetSize_812x11WLEF_W096H016A },		// 8 1/2×11 LEF.
	{ TYPE_SizeCodeInvoiceSef,		s_pNOTSheetSize_512x812_W096H016A,		s_pNOTSheetSize_512x812WSEF_W096H016A },	// 5 1/2×8 1/2 SEF.
	{ TYPE_SizeCodeInvoiceLef,		s_pNOTSheetSize_512x812LEF_W096H016A,	s_pNOTSheetSize_512x812WLEF_W096H016A },	// 5 1/2×8 1/2 LEF.
	{ TYPE_SizeCodeExecutiveSef,	s_pNOTSheetSize_714x1012_W096H016A,		s_Dummy },									// 7 1/4×10 1/2 SEF.
	{ TYPE_SizeCodeExecutiveLef,	s_pNOTSheetSize_714x1012LEF_W096H016A,	s_Dummy },									// 7 1/4×10 1/2 LEF.
	{ TYPE_SizeCode12x18Sef,		s_pNOTSheetSize_12x18_W096H016A,		s_pNOT12x18SEF_W064H036A },					// 12×18.
	{ TYPE_SizeCodeSRA3Sef,			s_pNOTSheetSize_SRA3SEF_W096H016A,		s_Dummy },									// SRA3【CQ.348170】.
#endif  //  DEF_LCD_7INCH.
	{ TYPE_SizeCodeUnknown,			s_pNOTDummy_W068H036A,					s_Dummy },									// 未定・不明.
};
const uchar uc_SizeCode_PaperSizeWide = ( uchar )( sizeof( tbl_SizeCode_PaperSizeWide ) / sizeof( PNLC_SizeCode_PaperSizeWide ) );


// A3A4FUM 用紙種類設定 t-okada 2015/01/19 Start 選択トレイの設定変更.
#ifndef DEF_A4_Cus
// A3A4FUM 用紙種類設定 t-okada 2015/01/19 End.
// 手差し以外用紙種類テーブル.
static const TYPS_PaperKindInfo st_PaperKindNewTbl[] = {
// 今後用紙種類に増減する事を踏まえ、uc_PaperWeightIndexを定数化した.
	// 標準用紙.
// Comment 2015/01/29 HCLT-Vinothini [Compilation Switch ID: 9] for HeliosL support.
//#ifdef DEF_Zeus5BkCus
// Comment 2019/07/26 [HCLT_Paul] [IT6_HeliosMLK] [ Seigyu.10 ] Adding the HeliosMLK compiler switch.
//comment 2019/11/18 [HCLT-Thangachamy] [HeliosMLK] [機器構成変更_No.11] Removing HeliosMLK compiler switch.
//#if defined(DEF_Zeus5BkCus) || defined(DEF_HeliosMLKCus)
#ifndef DEF_HeliosMLKCus
#if defined(DEF_Zeus5BkCus)
	{ TYPE_PaperTypeNormal,			False,			s_pKEYPlainPaper_W144H028A,					CNK_NomalPaper,					PNLD_PAPERTYPEIND_NORMAL,			PNLE_PAPERTYPE_WEIGHT_NORMAL,	},	// 普通紙.
	{ TYPE_PaperTypeNotDuplex1,		False,			s_pKEYSingleSidedOnlyPlainPaper_W144H028A,	CNK_S3Paper,					PNLD_PAPERTYPEIND_SINGLSIDE,		PNLE_PAPERTYPE_WEIGHT_NORMAL,	},	// 片面専用用紙.
	{ TYPE_PaperTypeSpecial1,		False,			s_pKEYSpecialPaper_W144H028A,				CNK_SpecialPaper,				PNLD_PAPERTYPEIND_SPECIAL,			PNLE_PAPERTYPE_WEIGHT_NORMAL,	},	// 特殊紙.
//Comment 2018/07/29 HCLT_Catherine for DenebMLK [Issue ID:1050339] adding plain paper+.
    { TYPE_PaperTypeNormalPlus,		False,			s_pKEYPlainPaperP_W144H028A,				CNK_NomalPaperPlus,				PNLD_PAPERTYPEIND_NORMALP,			PNLE_PAPERTYPE_WEIGHT_NORMALP	},	// 普通紙+.
	{ TYPE_PaperTypeThick1,			False,			s_pKEYThickPaper1_W144H028A,				CNK_SpecialPaperCard1,			PNLD_PAPERTYPEIND_THICK1,			PNLE_PAPERTYPE_WEIGHT_THICK1,	},	// 厚紙1.
	{ TYPE_PaperTypeThick1Plus,		False,			s_pKEYThickPaper1P_W144H028A,				CNK_SpecialPaperCard1P,			PNLD_PAPERTYPEIND_THICK1P,			PNLE_PAPERTYPE_WEIGHT_THICK1P,	},	// 厚紙1+.
	{ TYPE_PaperTypeThick2,			False,			s_pKEYThickPaper2_W144H028A,				CNK_SpecialPaperCard2,			PNLD_PAPERTYPEIND_THICK2,			PNLE_PAPERTYPE_WEIGHT_THICK2,	},	// 厚紙2.
	{ TYPE_PaperTypeThick3,			False,			s_pKEYThickPaper3_W144H028A,				CNK_SpecialPaperCard3,			PNLD_PAPERTYPEIND_THICK3,			PNLE_PAPERTYPE_WEIGHT_THICK3,	},	// 厚紙3.
	{ TYPE_PaperRecycle1,			False,			s_pKEYRecyclePaper_W144H028A,				CNK_NomalPaper,					PNLD_PAPERTYPEIND_RECYCLE,			PNLE_PAPERTYPE_WEIGHT_NORMAL,	},	// 再生紙.		// comment MinervaZeus ADD Sky上西 2011/05/05 [UV-01038][機器1001].
	{ TYPE_PaperTypeLetterHead,		False,			s_pKEYLetterhead_W144H028A,					CNK_LetterHeadPaper,			PNLD_PAPERTYPEIND_LTRHEAD,			PNLE_PAPERTYPE_WEIGHT_NORMAL,	},	// レターヘッド紙.
	{ TYPE_PaperTypeColor,			False,			s_pKEYColorPaper_W144H028A,					CNK_ColorPaper,					PNLD_PAPERTYPEIND_COLOR,			PNLE_PAPERTYPE_WEIGHT_NORMAL,	},	// 色紙.
	// 標準用紙(2面目).
	{ TYPE_PaperTypeNormal,			True,			s_pKEYPlainPaper_W144H028A,					CNK_NomalPaperRemove,			PNLD_PAPERTYPEIND_NORMALRMV,		PNLE_PAPERTYPE_WEIGHT_NORMAL,	},	// 普通紙.
	{ TYPE_PaperTypeThick1,			True,			s_pKEYThickPaper1_W144H028A,				CNK_SpecialPaperCard1Remove,	PNLD_PAPERTYPEIND_THICK1RMV,		PNLE_PAPERTYPE_WEIGHT_THICK1,	},	// 厚紙1.
	{ TYPE_PaperTypeThick1Plus,		True,			s_pKEYThickPaper1P_W144H028A,				CNK_SpecialPaperCard1P,			PNLD_PAPERTYPEIND_THICK1PRMV,		PNLE_PAPERTYPE_WEIGHT_THICK1P,	},	// 厚紙1+.
	{ TYPE_PaperTypeThick2,			True,			s_pKEYThickPaper2_W144H028A,				CNK_SpecialPaperCard2Remove,	PNLD_PAPERTYPEIND_THICK2RMV,		PNLE_PAPERTYPE_WEIGHT_THICK2,	},	// 厚紙2.
	{ TYPE_PaperTypeThick3,			True,			s_pKEYThickPaper3_W144H028A,				CNK_SpecialPaperCard3Remove,	PNLD_PAPERTYPEIND_THICK3RMV,		PNLE_PAPERTYPE_WEIGHT_THICK3,	},	// 厚紙3.
	{ TYPE_PaperRecycle1,			True,			s_pKEYRecyclePaper_W144H028A,				CNK_NomalPaperRemove,			PNLD_PAPERTYPEIND_RECYCLERMV,		PNLE_PAPERTYPE_WEIGHT_NORMAL,	},	// 再生紙.			// comment MinervaZeus ADD Sky上西 2011/05/05 [UV-01038][機器1001].
//comment 2019/11/18 [HCLT-Thangachamy] [HeliosMLK] [機器構成変更_No.11] Removing HeliosMLK compiler switch.
//#else // #if defined(DEF_Zeus5BkCus) || defined(DEF_HeliosCus).
#else //#if defined(DEF_Zeus5BkCus)
	{ TYPE_PaperTypeNormal,			False,			s_pKEYPlainPaper_W144H028A,					CNK_NomalPaper,					PNLD_PAPERTYPEIND_NORMAL,			PNLE_PAPERTYPE_WEIGHT_NORMAL,	},	// 普通紙.
	{ TYPE_PaperTypeNotDuplex1,		False,			s_pKEYSingleSidedOnlyPlainPaper_W144H028A,	CNK_S3Paper,					PNLD_PAPERTYPEIND_SINGLSIDE,		PNLE_PAPERTYPE_WEIGHT_NORMAL,	},	// 片面専用用紙.
	{ TYPE_PaperTypeThin1,			False,			s_pKEYThinPaper_W144H028A,					CNK_ThinPaper,					PNLD_PAPERTYPEIND_THIN,				PNLE_PAPERTYPE_WEIGHT_THIN,		},	// 薄紙. 		   // comment MinervaZeus Sky小尾 2011/04/15 [UV-01038_1：No.01].
	{ TYPE_PaperTypeSpecial1,		False,			s_pKEYSpecialPaper_W144H028A,				CNK_SpecialPaper,				PNLD_PAPERTYPEIND_SPECIAL,			PNLE_PAPERTYPE_WEIGHT_NORMAL,	},	// 特殊紙.
	{ TYPE_PaperTypeNormalPlus,		False,			s_pKEYPlainPaperP_W144H028A,				CNK_NomalPaperPlus,				PNLD_PAPERTYPEIND_NORMALP,			PNLE_PAPERTYPE_WEIGHT_NORMALP	},	// 普通紙+.			// comment 2018/03/29 SCD2 K.Sato [Eagle][RQ-722]用紙種別追加（厚紙1を2種類に分割）対応.
	{ TYPE_PaperTypeThick1,			False,			s_pKEYThickPaper1_W144H028A,				CNK_SpecialPaperCard1,			PNLD_PAPERTYPEIND_THICK1,			PNLE_PAPERTYPE_WEIGHT_THICK1,	},	// 厚紙1.
	{ TYPE_PaperTypeThick1Plus,		False,			s_pKEYThickPaper1P_W144H028A,				CNK_SpecialPaperCard1P,			PNLD_PAPERTYPEIND_THICK1P,			PNLE_PAPERTYPE_WEIGHT_THICK1P,	},	// 厚紙1+.
	{ TYPE_PaperTypeThick2,			False,			s_pKEYThickPaper2_W144H028A,				CNK_SpecialPaperCard2,			PNLD_PAPERTYPEIND_THICK2,			PNLE_PAPERTYPE_WEIGHT_THICK2,	},	// 厚紙2.
	{ TYPE_PaperTypeThick3,			False,			s_pKEYThickPaper3_W144H028A,				CNK_SpecialPaperCard3,			PNLD_PAPERTYPEIND_THICK3,			PNLE_PAPERTYPE_WEIGHT_THICK3,	},	// 厚紙3.
	{ TYPE_PaperRecycle1,			False,			s_pKEYRecyclePaper_W144H028A,				CNK_NomalPaper,					PNLD_PAPERTYPEIND_RECYCLE,			PNLE_PAPERTYPE_WEIGHT_NORMAL,	},	// 再生紙.		// comment MinervaZeus ADD Sky上西 2011/05/05 [UV-01038][機器1001].
	{ TYPE_PaperTypeLetterHead,		False,			s_pKEYLetterhead_W144H028A,					CNK_LetterHeadPaper,			PNLD_PAPERTYPEIND_LTRHEAD,			PNLE_PAPERTYPE_WEIGHT_NORMAL,	},	// レターヘッド紙.
	{ TYPE_PaperTypeColor,			False,			s_pKEYColorPaper_W144H028A,					CNK_ColorPaper,					PNLD_PAPERTYPEIND_COLOR,			PNLE_PAPERTYPE_WEIGHT_NORMAL,	},	// 色紙.
	// 標準用紙(2面目).
	{ TYPE_PaperTypeNormal,			True,			s_pKEYPlainPaper_W144H028A,					CNK_NomalPaperRemove,			PNLD_PAPERTYPEIND_NORMALRMV,		PNLE_PAPERTYPE_WEIGHT_NORMAL,	},	// 普通紙.
	{ TYPE_PaperTypeThin1,			True,			s_pITMLightPaper_W152H016A,					CNK_ThinPaperRemove,			PNLD_PAPERTYPEIND_THINRMV,			PNLE_PAPERTYPE_WEIGHT_THIN,		},	// 薄紙. 		   // comment MinervaZeus Sky小尾 2011/04/15 [UV-01038_1：No.01].
	{ TYPE_PaperTypeNormalPlus,		True,			s_pKEYPlainPaperP_W144H028A,				CNK_NomalPaperPlusRemove,		PNLD_PAPERTYPEIND_NORMALPRMV,		PNLE_PAPERTYPE_WEIGHT_NORMALP	},	// 普通紙+.			// comment 2018/03/29 SCD2 K.Sato [Eagle][RQ-722]用紙種別追加（厚紙1を2種類に分割）対応.
	{ TYPE_PaperTypeThick1,			True,			s_pKEYThickPaper1_W144H028A,				CNK_SpecialPaperCard1Remove,	PNLD_PAPERTYPEIND_THICK1RMV,		PNLE_PAPERTYPE_WEIGHT_THICK1,	},	// 厚紙1.
	{ TYPE_PaperTypeThick1Plus,		True,			s_pKEYThickPaper1P_W144H028A,				CNK_SpecialPaperCard1P,			PNLD_PAPERTYPEIND_THICK1PRMV,		PNLE_PAPERTYPE_WEIGHT_THICK1P,	},	// 厚紙1+.
	{ TYPE_PaperTypeThick2,			True,			s_pKEYThickPaper2_W144H028A,				CNK_SpecialPaperCard2Remove,	PNLD_PAPERTYPEIND_THICK2RMV,		PNLE_PAPERTYPE_WEIGHT_THICK2,	},	// 厚紙2.
	{ TYPE_PaperTypeThick3,			True,			s_pKEYThickPaper3_W144H028A,				CNK_SpecialPaperCard3Remove,	PNLD_PAPERTYPEIND_THICK3RMV,		PNLE_PAPERTYPE_WEIGHT_THICK3,	},	// 厚紙3.
	{ TYPE_PaperRecycle1,			True,			s_pKEYRecyclePaper_W144H028A,				CNK_NomalPaperRemove,			PNLD_PAPERTYPEIND_RECYCLERMV,		PNLE_PAPERTYPE_WEIGHT_NORMAL,	},	// 再生紙.			// comment MinervaZeus ADD Sky上西 2011/05/05 [UV-01038][機器1001].
//comment 2019/11/18 [HCLT-Thangachamy] [HeliosMLK] [機器構成変更_No.11] Removing HeliosMLK compiler switch.
//#endif // #if defined(DEF_Zeus5BkCus) || defined(DEF_HeliosCus).
#endif // #if defined(DEF_Zeus5BkCus)
// ユーザー紙(※坪量はチョイスで変更可能なため「0」としておく).
//comment 2018/12/12 HCLT-Basker [DenebMLK][制御構成変更_No.45 ] Remove the unneccessary PaperTypes from DenebMLK models.
#ifndef DEF_DenebMLKCus
	{ TYPE_PaperTypeUser1,			False,			s_pKEYUserPaper1_W144H028A,					CNK_NomalPaper,					PNLD_PAPERTYPEIND_USER1,			PNLE_PAPERTYPE_WEIGHT_NONE,	},		// ユーザー紙1.
// comment 2018/04/17 SCD2-Sky鳫林 [Eagle][RQ-722] 普通紙+追加対応.
//	{ TYPE_PaperTypeUser2,			False,			s_pKEYUserPaper2_W144H028A,					CNK_NomalPaper,					PNLD_PAPERTYPEIND_USER2,			PNLE_PAPERTYPE_WEIGHT_NONE,	},		// ユーザー紙2.
	{ TYPE_PaperTypeUser2,			False,			s_pKEYUserPaper2_W144H028A,					CNK_NomalPaperPlus,				PNLD_PAPERTYPEIND_USER2,			PNLE_PAPERTYPE_WEIGHT_NONE,	},		// ユーザー紙2.
	{ TYPE_PaperTypeUser3,			False,			s_pKEYUserPaper4_W144H028A,					CNK_SpecialPaperCard1,			PNLD_PAPERTYPEIND_USER3,			PNLE_PAPERTYPE_WEIGHT_NONE,	},		// ユーザー紙3.
	{ TYPE_PaperTypeUser4,			False,			s_pKEYUserPaper3_W144H028A,					CNK_SpecialPaperCard1P,			PNLD_PAPERTYPEIND_USER4,			PNLE_PAPERTYPE_WEIGHT_NONE,	},		// ユーザー紙4.
#else // DEF_DenebMLKCus.
	{ TYPE_PaperTypeUser1,			False,			s_pKEYUserPaper1_W144H028A,					CNK_NomalPaper,					PNLD_PAPERTYPEIND_USER1,			PNLE_PAPERTYPE_WEIGHT_NORMAL,	},		// ユーザー紙1.
	{ TYPE_PaperTypeUser2,			False,			s_pKEYUserPaper2_W144H028A,					CNK_NomalPaperPlus,				PNLD_PAPERTYPEIND_USER2,			PNLE_PAPERTYPE_WEIGHT_NORMALP,	},		// ユーザー紙2.
	{ TYPE_PaperTypeUser3,			False,			s_pKEYUserPaper4_W144H028A,					CNK_SpecialPaperCard1,			PNLD_PAPERTYPEIND_USER3,			PNLE_PAPERTYPE_WEIGHT_THICK1,	},		// ユーザー紙3.
	{ TYPE_PaperTypeUser4,			False,			s_pKEYUserPaper3_W144H028A,					CNK_SpecialPaperCard1P,			PNLD_PAPERTYPEIND_USER4,			PNLE_PAPERTYPE_WEIGHT_THICK1P,	},		// ユーザー紙4.
#endif // DEF_DenebMLKCus.
	{ TYPE_PaperTypeUser5,			False,			s_pKEYUserPaper5_W144H028A,					CNK_SpecialPaperCard2,			PNLD_PAPERTYPEIND_USER5,			PNLE_PAPERTYPE_WEIGHT_NONE,	},		// ユーザー紙5.
	{ TYPE_PaperTypeUser6,			False,			s_pKEYUserPaper6_W144H028A,					CNK_SpecialPaperCard3,			PNLD_PAPERTYPEIND_USER6,			PNLE_PAPERTYPE_WEIGHT_NONE,	},		// ユーザー紙6.
	// ユーザー紙(2面目)(※坪量はチョイスで変更可能なため「0」としておく).
//comment 2018/12/12 HCLT-Basker [DenebMLK][制御構成変更_No.45 ] Remove the unneccessary PaperTypes from DenebMLK models.
#ifndef DEF_DenebMLKCus
	{ TYPE_PaperTypeUser1,			True,			s_pKEYUserPaper1_W144H028A,					CNK_NomalPaper,					PNLD_PAPERTYPEIND_USER1RMV,			PNLE_PAPERTYPE_WEIGHT_NONE,	},		// ユーザー紙1.
// comment 2018/04/17 SCD2-Sky鳫林 [Eagle][RQ-722] 普通紙+追加対応.
//	{ TYPE_PaperTypeUser2,			True,			s_pKEYUserPaper2_W144H028A,					CNK_NomalPaper,					PNLD_PAPERTYPEIND_USER2RMV,			PNLE_PAPERTYPE_WEIGHT_NONE,	},		// ユーザー紙2.
	{ TYPE_PaperTypeUser2,			True,			s_pKEYUserPaper2_W144H028A,					CNK_NomalPaperPlus,				PNLD_PAPERTYPEIND_USER2RMV,			PNLE_PAPERTYPE_WEIGHT_NONE,	},		// ユーザー紙2.
	{ TYPE_PaperTypeUser3,			True,			s_pKEYUserPaper4_W144H028A,					CNK_SpecialPaperCard1,			PNLD_PAPERTYPEIND_USER3RMV,			PNLE_PAPERTYPE_WEIGHT_NONE,	},		// ユーザー紙3.
	{ TYPE_PaperTypeUser4,			True,			s_pKEYUserPaper3_W144H028A,					CNK_SpecialPaperCard1P,			PNLD_PAPERTYPEIND_USER4RMV,			PNLE_PAPERTYPE_WEIGHT_NONE,	},		// ユーザー紙4.
#else // DEF_DenebMLKCus.
	{ TYPE_PaperTypeUser1,			True,			s_pKEYUserPaper1_W144H028A,					CNK_NomalPaper,					PNLD_PAPERTYPEIND_USER1RMV,			PNLE_PAPERTYPE_WEIGHT_NORMAL,	},		// ユーザー紙1.
	{ TYPE_PaperTypeUser2,			True,			s_pKEYUserPaper2_W144H028A,					CNK_NomalPaperPlus,				PNLD_PAPERTYPEIND_USER2RMV,			PNLE_PAPERTYPE_WEIGHT_NORMALP,	},		// ユーザー紙2.
	{ TYPE_PaperTypeUser3,			True,			s_pKEYUserPaper4_W144H028A,					CNK_SpecialPaperCard1,			PNLD_PAPERTYPEIND_USER3RMV,			PNLE_PAPERTYPE_WEIGHT_THICK1,	},		// ユーザー紙3.
	{ TYPE_PaperTypeUser4,			True,			s_pKEYUserPaper3_W144H028A,					CNK_SpecialPaperCard1P,			PNLD_PAPERTYPEIND_USER4RMV,			PNLE_PAPERTYPE_WEIGHT_THICK1P,	},		// ユーザー紙4.
#endif // DEF_DenebMLKCus.
	{ TYPE_PaperTypeUser5,			True,			s_pKEYUserPaper5_W144H028A,					CNK_SpecialPaperCard2,			PNLD_PAPERTYPEIND_USER5RMV,			PNLE_PAPERTYPE_WEIGHT_NONE,	},		// ユーザー紙5.
	{ TYPE_PaperTypeUser6,			True,			s_pKEYUserPaper6_W144H028A,					CNK_SpecialPaperCard3,			PNLD_PAPERTYPEIND_USER6RMV,			PNLE_PAPERTYPE_WEIGHT_NONE,	},		// ユーザー紙6.
//comment 2019/11/18 [HCLT-Thangachamy] [HeliosMLK] [機器構成変更_No.11] Creating new array entries with HELIOSMLK supported papers.
#else //#ifndef DEF_HeliosMLKCus
	{ TYPE_PaperTypeNormal,			False,			s_pKEYPlainPaper_W144H028A,					CNK_NomalPaper,					PNLD_PAPERTYPEIND_NORMAL,			PNLE_PAPERTYPE_WEIGHT_NORMAL,	},	// 普通紙.
	{ TYPE_PaperTypeNotDuplex1,		False,			s_pKEYSingleSidedOnlyPlainPaper_W144H028A,	CNK_S3Paper,					PNLD_PAPERTYPEIND_SINGLSIDE,		PNLE_PAPERTYPE_WEIGHT_NORMAL,	},	// 片面専用用紙.
	{ TYPE_PaperTypeSpecial1,		False,			s_pKEYSpecialPaper_W144H028A,				CNK_SpecialPaper,				PNLD_PAPERTYPEIND_SPECIAL,			PNLE_PAPERTYPE_WEIGHT_NORMAL,	},	// 特殊紙.
	{ TYPE_PaperTypeThick1,			False,			s_pKEYThickPaper1_W144H028A,				CNK_SpecialPaperCard1,			PNLD_PAPERTYPEIND_THICK1,			PNLE_PAPERTYPE_WEIGHT_THICK1,	},	// 厚紙1.
	{ TYPE_PaperTypeThick1Plus,		False,			s_pKEYThickPaper1P_W144H028A,				CNK_SpecialPaperCard1P,			PNLD_PAPERTYPEIND_THICK1P,			PNLE_PAPERTYPE_WEIGHT_THICK1P,	},	// 厚紙1+.
	{ TYPE_PaperTypeThick2,			False,			s_pKEYThickPaper2_W144H028A,				CNK_SpecialPaperCard2,			PNLD_PAPERTYPEIND_THICK2,			PNLE_PAPERTYPE_WEIGHT_THICK2,	},	// 厚紙2.
	{ TYPE_PaperTypeThick3,			False,			s_pKEYThickPaper3_W144H028A,				CNK_SpecialPaperCard3,			PNLD_PAPERTYPEIND_THICK3,			PNLE_PAPERTYPE_WEIGHT_THICK3,	},	// 厚紙3.
	{ TYPE_PaperRecycle1,			False,			s_pKEYRecyclePaper_W144H028A,				CNK_NomalPaper,					PNLD_PAPERTYPEIND_RECYCLE,			PNLE_PAPERTYPE_WEIGHT_NORMAL,	},	// 再生紙.		// comment MinervaZeus ADD Sky上西 2011/05/05 [UV-01038][機器1001].
	{ TYPE_PaperTypeLetterHead,		False,			s_pKEYLetterhead_W144H028A,					CNK_LetterHeadPaper,			PNLD_PAPERTYPEIND_LTRHEAD,			PNLE_PAPERTYPE_WEIGHT_NORMAL,	},	// レターヘッド紙.
	{ TYPE_PaperTypeColor,			False,			s_pKEYColorPaper_W144H028A,					CNK_ColorPaper,					PNLD_PAPERTYPEIND_COLOR,			PNLE_PAPERTYPE_WEIGHT_NORMAL,	},	// 色紙.
	// 標準用紙(2面目).
	{ TYPE_PaperTypeNormal,			False,			s_pKEYPlainPaper_W144H028A,					CNK_NomalPaperRemove,			PNLD_PAPERTYPEIND_NORMALRMV,		PNLE_PAPERTYPE_WEIGHT_NORMAL,	},	// 普通紙.
	{ TYPE_PaperTypeThick1,			False,			s_pKEYThickPaper1_W144H028A,				CNK_SpecialPaperCard1Remove,	PNLD_PAPERTYPEIND_THICK1RMV,		PNLE_PAPERTYPE_WEIGHT_THICK1,	},	// 厚紙1.
	{ TYPE_PaperTypeThick1Plus,		False,			s_pKEYThickPaper1P_W144H028A,				CNK_SpecialPaperCard1P,			PNLD_PAPERTYPEIND_THICK1PRMV,		PNLE_PAPERTYPE_WEIGHT_THICK1P,	},	// 厚紙1+.
	{ TYPE_PaperTypeThick2,			False,			s_pKEYThickPaper2_W144H028A,				CNK_SpecialPaperCard2Remove,	PNLD_PAPERTYPEIND_THICK2RMV,		PNLE_PAPERTYPE_WEIGHT_THICK2,	},	// 厚紙2.
	{ TYPE_PaperTypeThick3,			False,			s_pKEYThickPaper3_W144H028A,				CNK_SpecialPaperCard3Remove,	PNLD_PAPERTYPEIND_THICK3RMV,		PNLE_PAPERTYPE_WEIGHT_THICK3,	},	// 厚紙3.
	{ TYPE_PaperRecycle1,			False,			s_pKEYRecyclePaper_W144H028A,				CNK_NomalPaperRemove,			PNLD_PAPERTYPEIND_RECYCLERMV,		PNLE_PAPERTYPE_WEIGHT_NORMAL,	},	// 再生紙.			// comment MinervaZeus ADD Sky上西 2011/05/05 [UV-01038][機器1001].
	// User Paper 1~6 for Tray 2-4
	{ TYPE_PaperTypeUser1,			False,			s_pKEYUserPaper1_W144H028A,					CNK_NomalPaper,					PNLD_PAPERTYPEIND_USER1,			PNLE_PAPERTYPE_WEIGHT_NONE,	},		// ユーザー紙1.
	{ TYPE_PaperTypeUser2,			False,			s_pKEYUserPaper2_W144H028A,					CNK_NomalPaper,					PNLD_PAPERTYPEIND_USER2,			PNLE_PAPERTYPE_WEIGHT_NONE,	},		// ユーザー紙2.
	{ TYPE_PaperTypeUser3,			False,			s_pKEYUserPaper4_W144H028A,					CNK_SpecialPaperCard1,			PNLD_PAPERTYPEIND_USER3,			PNLE_PAPERTYPE_WEIGHT_NONE,	},		// ユーザー紙3.
	{ TYPE_PaperTypeUser4,			False,			s_pKEYUserPaper3_W144H028A,					CNK_SpecialPaperCard1P,			PNLD_PAPERTYPEIND_USER4,			PNLE_PAPERTYPE_WEIGHT_NONE,	},		// ユーザー紙4.
	{ TYPE_PaperTypeUser5,			False,			s_pKEYUserPaper5_W144H028A,					CNK_SpecialPaperCard2,			PNLD_PAPERTYPEIND_USER5,			PNLE_PAPERTYPE_WEIGHT_NONE,	},		// ユーザー紙5.
	{ TYPE_PaperTypeUser6,			False,			s_pKEYUserPaper6_W144H028A,					CNK_SpecialPaperCard3,			PNLD_PAPERTYPEIND_USER6,			PNLE_PAPERTYPE_WEIGHT_NONE,	},		// ユーザー紙6.
	// ユーザー紙(2面目)
	{ TYPE_PaperTypeUser1,			False,			s_pKEYUserPaper1_W144H028A,					CNK_NomalPaper,					PNLD_PAPERTYPEIND_USER1RMV,			PNLE_PAPERTYPE_WEIGHT_NONE,	},		// ユーザー紙1.
	{ TYPE_PaperTypeUser2,			False,			s_pKEYUserPaper2_W144H028A,					CNK_NomalPaper,					PNLD_PAPERTYPEIND_USER2RMV,			PNLE_PAPERTYPE_WEIGHT_NONE,	},		// ユーザー紙2.
	{ TYPE_PaperTypeUser3,			False,			s_pKEYUserPaper4_W144H028A,					CNK_SpecialPaperCard1,			PNLD_PAPERTYPEIND_USER3RMV,			PNLE_PAPERTYPE_WEIGHT_NONE,	},		// ユーザー紙3.
	{ TYPE_PaperTypeUser4,			False,			s_pKEYUserPaper3_W144H028A,					CNK_SpecialPaperCard1P,			PNLD_PAPERTYPEIND_USER4RMV,			PNLE_PAPERTYPE_WEIGHT_NONE,	},		// ユーザー紙4.
	{ TYPE_PaperTypeUser5,			False,			s_pKEYUserPaper5_W144H028A,					CNK_SpecialPaperCard2,			PNLD_PAPERTYPEIND_USER5RMV,			PNLE_PAPERTYPE_WEIGHT_NONE,	},		// ユーザー紙5.
	{ TYPE_PaperTypeUser6,			False,			s_pKEYUserPaper6_W144H028A,					CNK_SpecialPaperCard3,			PNLD_PAPERTYPEIND_USER6RMV,			PNLE_PAPERTYPE_WEIGHT_NONE,	},		// ユーザー紙6.		
#endif	
};
//Comment [IT6-Eagle] 機器構成変更No.2 HCLT-Deeparaj 2018/02/20 New table for Tray1.
#ifndef DEF_A4_Cus
static const TYPS_PaperKindInfo st_PaperKindNewTblTry1[] = {
// 今後用紙種類に増減する事を踏まえ、uc_PaperWeightIndexを定数化した.
//comment 2019/11/18 [HCLT-Thangachamy] [HeliosMLK] [機器構成変更_No.11] Creating new array entries with HELIOSMLK supported papers.
#ifndef DEF_HeliosMLKCus
	// 標準用紙.
	{TYPE_PaperTypeNormal,			False,			s_pKEYPlainPaper_W144H028A,					CNK_NomalPaper,					PNLD_PAPERTYPEIND_NORMAL,			PNLE_PAPERTYPE_WEIGHT_NORMAL,	},	// 普通紙.
	{ TYPE_PaperTypeNotDuplex1,		False,			s_pKEYSingleSidedOnlyPlainPaper_W144H028A,	CNK_S3Paper,					PNLD_PAPERTYPEIND_SINGLSIDE,		PNLE_PAPERTYPE_WEIGHT_NORMAL,	},	// 片面専用用紙.
	{ TYPE_PaperTypeThin1,			False,			s_pKEYThinPaper_W144H028A,					CNK_ThinPaper,					PNLD_PAPERTYPEIND_THIN,				PNLE_PAPERTYPE_WEIGHT_THIN,		},	// 薄紙. 		   // comment MinervaZeus Sky小尾 2011/04/15 [UV-01038_1：No.01].
	{ TYPE_PaperTypeSpecial1,		False,			s_pKEYSpecialPaper_W144H028A,				CNK_SpecialPaper,				PNLD_PAPERTYPEIND_SPECIAL,			PNLE_PAPERTYPE_WEIGHT_NORMAL,	},	// 特殊紙.
// comment 2018/04/18 SCD2-Sky中野 [EagleL][RQ-722]普通紙+ - 追加対応.
	{ TYPE_PaperTypeNormalPlus,		False,			s_pKEYPlainPaperP_W144H028A,				CNK_NomalPaperPlus,				PNLD_PAPERTYPEIND_NORMALP,			PNLE_PAPERTYPE_WEIGHT_NORMALP	},	// 普通紙+.
	{ TYPE_PaperTypeThick1,			False,			s_pKEYThickPaper1_W144H028A,				CNK_SpecialPaperCard1,			PNLD_PAPERTYPEIND_THICK1,			PNLE_PAPERTYPE_WEIGHT_THICK1,	},	// 厚紙1.
	{ TYPE_PaperTypeThick1Plus,		False,			s_pKEYThickPaper1P_W144H028A,				CNK_SpecialPaperCard1P,			PNLD_PAPERTYPEIND_THICK1P,			PNLE_PAPERTYPE_WEIGHT_THICK1P,	},	// 厚紙1+.
	{ TYPE_PaperTypeThick2,			False,			s_pKEYThickPaper2_W144H028A,				CNK_SpecialPaperCard2,			PNLD_PAPERTYPEIND_THICK2,			PNLE_PAPERTYPE_WEIGHT_THICK2,	},	// 厚紙2.
	{ TYPE_PaperTypeThick3,			False,			s_pKEYThickPaper3_W144H028A,				CNK_SpecialPaperCard3,			PNLD_PAPERTYPEIND_THICK3,			PNLE_PAPERTYPE_WEIGHT_THICK3,	},	// 厚紙3.
// Comment 2021/06/01 【FPT-LinhNV46】 【MinervaSSBK】 【機器構成変更_No.21-20】 Delete paper type Envelope of Tray1 for MinervaSSBK.
#ifndef DEF_MinervaSSBKCus
	{ TYPE_PaperTypeEnvelope,		False,			s_pKEYEnvelope_W144H028A,					CNK_SpecialEnvelope,			PNLD_PAPERTYPEIND_ENVELOPE,			PNLE_PAPERTYPE_WEIGHT_NONE,		},		// 封筒.
#endif //DEF_MinervaSSBKCus.
	{ TYPE_PaperRecycle1,			False,			s_pKEYRecyclePaper_W144H028A,				CNK_NomalPaper,					PNLD_PAPERTYPEIND_RECYCLE,			PNLE_PAPERTYPE_WEIGHT_NORMAL,	},	// 再生紙.		// comment MinervaZeus ADD Sky上西 2011/05/05 [UV-01038][機器1001].
	{ TYPE_PaperTypeLetterHead,		False,			s_pKEYLetterhead_W144H028A,					CNK_LetterHeadPaper,			PNLD_PAPERTYPEIND_LTRHEAD,			PNLE_PAPERTYPE_WEIGHT_NORMAL,	},	// レターヘッド紙.
	{ TYPE_PaperTypeColor,			False,			s_pKEYColorPaper_W144H028A,					CNK_ColorPaper,					PNLD_PAPERTYPEIND_COLOR,			PNLE_PAPERTYPE_WEIGHT_NORMAL,	},	// 色紙.
	// 標準用紙(2面目).
	{ TYPE_PaperTypeNormal,			True,			s_pKEYPlainPaper_W144H028A,					CNK_NomalPaperRemove,			PNLD_PAPERTYPEIND_NORMALRMV,		PNLE_PAPERTYPE_WEIGHT_NORMAL,	},	// 普通紙.
	{ TYPE_PaperTypeThin1,			True,			s_pITMLightPaper_W152H016A,					CNK_ThinPaperRemove,			PNLD_PAPERTYPEIND_THINRMV,			PNLE_PAPERTYPE_WEIGHT_THIN,		},	// 薄紙. 		   // comment MinervaZeus Sky小尾 2011/04/15 [UV-01038_1：No.01].
	{ TYPE_PaperTypeThick1,			True,			s_pKEYThickPaper1_W144H028A,				CNK_SpecialPaperCard1Remove,	PNLD_PAPERTYPEIND_THICK1RMV,		PNLE_PAPERTYPE_WEIGHT_THICK1,	},	// 厚紙1.
	{ TYPE_PaperTypeThick1Plus,		True,			s_pKEYThickPaper1P_W144H028A,				CNK_SpecialPaperCard1P,			PNLD_PAPERTYPEIND_THICK1PRMV,		PNLE_PAPERTYPE_WEIGHT_THICK1P,	},	// 厚紙1+.
	{ TYPE_PaperTypeThick2,			True,			s_pKEYThickPaper2_W144H028A,				CNK_SpecialPaperCard2Remove,	PNLD_PAPERTYPEIND_THICK2RMV,		PNLE_PAPERTYPE_WEIGHT_THICK2,	},	// 厚紙2.
	{ TYPE_PaperTypeThick3,			True,			s_pKEYThickPaper3_W144H028A,				CNK_SpecialPaperCard3Remove,	PNLD_PAPERTYPEIND_THICK3RMV,		PNLE_PAPERTYPE_WEIGHT_THICK3,	},	// 厚紙3.
	{ TYPE_PaperRecycle1,			True,			s_pKEYRecyclePaper_W144H028A,				CNK_NomalPaperRemove,			PNLD_PAPERTYPEIND_RECYCLERMV,		PNLE_PAPERTYPE_WEIGHT_NORMAL,	},	// 再生紙.			// comment MinervaZeus ADD Sky上西 2011/05/05 [UV-01038][機器1001].
	// ユーザー紙(※坪量はチョイスで変更可能なため「0」としておく).
//comment 2018/12/12 HCLT-Basker [DenebMLK][制御構成変更_No.45 ] Remove the unneccessary PaperTypes from DenebMLK models.
#ifndef DEF_DenebMLKCus
	{ TYPE_PaperTypeUser1,			False,			s_pKEYUserPaper1_W144H028A,					CNK_NomalPaper,					PNLD_PAPERTYPEIND_USER1,			PNLE_PAPERTYPE_WEIGHT_NONE,	},		// ユーザー紙1.
// comment 2018/04/17 SCD2-Sky鳫林 [Eagle][RQ-722] 普通紙+追加対応.
//	{ TYPE_PaperTypeUser2,			False,			s_pKEYUserPaper2_W144H028A,					CNK_NomalPaper,					PNLD_PAPERTYPEIND_USER2,			PNLE_PAPERTYPE_WEIGHT_NONE,	},		// ユーザー紙2.
	{ TYPE_PaperTypeUser2,			False,			s_pKEYUserPaper2_W144H028A,					CNK_NomalPaperPlus,				PNLD_PAPERTYPEIND_USER2,			PNLE_PAPERTYPE_WEIGHT_NONE,	},		// ユーザー紙2.
	{ TYPE_PaperTypeUser3,			False,			s_pKEYUserPaper4_W144H028A,					CNK_SpecialPaperCard1,			PNLD_PAPERTYPEIND_USER3,			PNLE_PAPERTYPE_WEIGHT_NONE,	},		// ユーザー紙3.
	{ TYPE_PaperTypeUser4,			False,			s_pKEYUserPaper3_W144H028A,					CNK_SpecialPaperCard1P,			PNLD_PAPERTYPEIND_USER4,			PNLE_PAPERTYPE_WEIGHT_NONE,	},		// ユーザー紙4.
#else // DEF_DenebMLKCus.
	{ TYPE_PaperTypeUser1,			False,			s_pKEYUserPaper1_W144H028A,					CNK_NomalPaper,					PNLD_PAPERTYPEIND_USER1,			PNLE_PAPERTYPE_WEIGHT_NORMAL,	},		// ユーザー紙1.
	{ TYPE_PaperTypeUser2,			False,			s_pKEYUserPaper2_W144H028A,					CNK_NomalPaperPlus,				PNLD_PAPERTYPEIND_USER2,			PNLE_PAPERTYPE_WEIGHT_NORMALP,	},		// ユーザー紙2.
	{ TYPE_PaperTypeUser3,			False,			s_pKEYUserPaper4_W144H028A,					CNK_SpecialPaperCard1,			PNLD_PAPERTYPEIND_USER3,			PNLE_PAPERTYPE_WEIGHT_THICK1,	},		// ユーザー紙3.
	{ TYPE_PaperTypeUser4,			False,			s_pKEYUserPaper3_W144H028A,					CNK_SpecialPaperCard1P,			PNLD_PAPERTYPEIND_USER4,			PNLE_PAPERTYPE_WEIGHT_THICK1P,	},		// ユーザー紙4.
#endif // DEF_DenebMLKCus.
	{ TYPE_PaperTypeUser5,			False,			s_pKEYUserPaper5_W144H028A,					CNK_SpecialPaperCard2,			PNLD_PAPERTYPEIND_USER5,			PNLE_PAPERTYPE_WEIGHT_NONE,	},		// ユーザー紙5.
	{ TYPE_PaperTypeUser6,			False,			s_pKEYUserPaper6_W144H028A,					CNK_SpecialPaperCard3,			PNLD_PAPERTYPEIND_USER6,			PNLE_PAPERTYPE_WEIGHT_NONE,	},		// ユーザー紙6.
	// ユーザー紙(2面目)(※坪量はチョイスで変更可能なため「0」としておく).
//comment 2018/12/12 HCLT-Basker [DenebMLK][制御構成変更_No.45 ] Remove the unneccessary PaperTypes from DenebMLK models.
#ifndef DEF_DenebMLKCus
	{ TYPE_PaperTypeUser1,			True,			s_pKEYUserPaper1_W144H028A,					CNK_NomalPaper,					PNLD_PAPERTYPEIND_USER1RMV,			PNLE_PAPERTYPE_WEIGHT_NONE,	},		// ユーザー紙1.
// comment 2018/04/17 SCD2-Sky鳫林 [Eagle][RQ-722] 普通紙+追加対応.
//	{ TYPE_PaperTypeUser2,			True,			s_pKEYUserPaper2_W144H028A,					CNK_NomalPaper,					PNLD_PAPERTYPEIND_USER2RMV,			PNLE_PAPERTYPE_WEIGHT_NONE,	},		// ユーザー紙2.
	{ TYPE_PaperTypeUser2,			True,			s_pKEYUserPaper2_W144H028A,					CNK_NomalPaperPlus,				PNLD_PAPERTYPEIND_USER2RMV,			PNLE_PAPERTYPE_WEIGHT_NONE,	},		// ユーザー紙2.
	{ TYPE_PaperTypeUser3,			True,			s_pKEYUserPaper4_W144H028A,					CNK_SpecialPaperCard1,			PNLD_PAPERTYPEIND_USER3RMV,			PNLE_PAPERTYPE_WEIGHT_NONE,	},		// ユーザー紙3.
	{ TYPE_PaperTypeUser4,			True,			s_pKEYUserPaper3_W144H028A,					CNK_SpecialPaperCard1P,			PNLD_PAPERTYPEIND_USER4RMV,			PNLE_PAPERTYPE_WEIGHT_NONE,	},		// ユーザー紙4.
#else // DEF_DenebMLKCus.
	{ TYPE_PaperTypeUser1,			True,			s_pKEYUserPaper1_W144H028A,					CNK_NomalPaper,					PNLD_PAPERTYPEIND_USER1RMV,			PNLE_PAPERTYPE_WEIGHT_NORMAL,	},		// ユーザー紙1.
	{ TYPE_PaperTypeUser2,			True,			s_pKEYUserPaper2_W144H028A,					CNK_NomalPaperPlus,				PNLD_PAPERTYPEIND_USER2RMV,			PNLE_PAPERTYPE_WEIGHT_NORMALP,	},		// ユーザー紙2.
	{ TYPE_PaperTypeUser3,			True,			s_pKEYUserPaper4_W144H028A,					CNK_SpecialPaperCard1,			PNLD_PAPERTYPEIND_USER3RMV,			PNLE_PAPERTYPE_WEIGHT_THICK1,	},		// ユーザー紙3.
	{ TYPE_PaperTypeUser4,			True,			s_pKEYUserPaper3_W144H028A,					CNK_SpecialPaperCard1P,			PNLD_PAPERTYPEIND_USER4RMV,			PNLE_PAPERTYPE_WEIGHT_THICK1P,	},		// ユーザー紙4.
#endif // DEF_DenebMLKCus.
	{ TYPE_PaperTypeUser5,			True,			s_pKEYUserPaper5_W144H028A,					CNK_SpecialPaperCard2,			PNLD_PAPERTYPEIND_USER5RMV,			PNLE_PAPERTYPE_WEIGHT_NONE,	},		// ユーザー紙5.
	{ TYPE_PaperTypeUser6,			True,			s_pKEYUserPaper6_W144H028A,					CNK_SpecialPaperCard3,			PNLD_PAPERTYPEIND_USER6RMV,			PNLE_PAPERTYPE_WEIGHT_NONE,	},		// ユーザー紙6.
#else //#ifndef DEF_HeliosMLKCus 
// For HELIOSMLK
	// 標準用紙.
	{ TYPE_PaperTypeNormal,			False,			s_pKEYPlainPaper_W144H028A,					CNK_NomalPaper,					PNLD_PAPERTYPEIND_NORMAL,			PNLE_PAPERTYPE_WEIGHT_NORMAL,	},	// 普通紙.
	{ TYPE_PaperTypeNotDuplex1,		False,			s_pKEYSingleSidedOnlyPlainPaper_W144H028A,	CNK_S3Paper,					PNLD_PAPERTYPEIND_SINGLSIDE,		PNLE_PAPERTYPE_WEIGHT_NORMAL,	},	// 片面専用用紙.
	{ TYPE_PaperTypeSpecial1,		False,			s_pKEYSpecialPaper_W144H028A,				CNK_SpecialPaper,				PNLD_PAPERTYPEIND_SPECIAL,			PNLE_PAPERTYPE_WEIGHT_NORMAL,	},	// 特殊紙.
	{ TYPE_PaperTypeThick1,			False,			s_pKEYThickPaper1_W144H028A,				CNK_SpecialPaperCard1,			PNLD_PAPERTYPEIND_THICK1,			PNLE_PAPERTYPE_WEIGHT_THICK1,	},	// 厚紙1.
	{ TYPE_PaperTypeThick1Plus,		False,			s_pKEYThickPaper1P_W144H028A,				CNK_SpecialPaperCard1P,			PNLD_PAPERTYPEIND_THICK1P,			PNLE_PAPERTYPE_WEIGHT_THICK1P,	},	// 厚紙1+.
	{ TYPE_PaperTypeThick2,			False,			s_pKEYThickPaper2_W144H028A,				CNK_SpecialPaperCard2,			PNLD_PAPERTYPEIND_THICK2,			PNLE_PAPERTYPE_WEIGHT_THICK2,	},	// 厚紙2.
	{ TYPE_PaperTypeThick3,			False,			s_pKEYThickPaper3_W144H028A,				CNK_SpecialPaperCard3,			PNLD_PAPERTYPEIND_THICK3,			PNLE_PAPERTYPE_WEIGHT_THICK3,	},	// 厚紙3.
	{ TYPE_PaperRecycle1,			False,			s_pKEYRecyclePaper_W144H028A,				CNK_NomalPaper,					PNLD_PAPERTYPEIND_RECYCLE,			PNLE_PAPERTYPE_WEIGHT_NORMAL,	},	// 再生紙.
	{ TYPE_PaperTypeLetterHead,		False,			s_pKEYLetterhead_W144H028A,					CNK_LetterHeadPaper,			PNLD_PAPERTYPEIND_LTRHEAD,			PNLE_PAPERTYPE_WEIGHT_NORMAL,	},	// レターヘッド紙.
	{ TYPE_PaperTypeColor,			False,			s_pKEYColorPaper_W144H028A,					CNK_ColorPaper,					PNLD_PAPERTYPEIND_COLOR,			PNLE_PAPERTYPE_WEIGHT_NORMAL,	},	// 色紙.
	// 標準用紙(2面目).
	{ TYPE_PaperTypeNormal,			False,			s_pKEYPlainPaper_W144H028A,					CNK_NomalPaperRemove,			PNLD_PAPERTYPEIND_NORMALRMV,		PNLE_PAPERTYPE_WEIGHT_NORMAL,	},	// 普通紙.
	{ TYPE_PaperTypeThick1,			False,			s_pKEYThickPaper1_W144H028A,				CNK_SpecialPaperCard1Remove,	PNLD_PAPERTYPEIND_THICK1RMV,		PNLE_PAPERTYPE_WEIGHT_THICK1,	},	// 厚紙1.
	{ TYPE_PaperTypeThick1Plus,		False,			s_pKEYThickPaper1P_W144H028A,				CNK_SpecialPaperCard1P,			PNLD_PAPERTYPEIND_THICK1PRMV,		PNLE_PAPERTYPE_WEIGHT_THICK1P,	},	// 厚紙1+.
	{ TYPE_PaperTypeThick2,			False,			s_pKEYThickPaper2_W144H028A,				CNK_SpecialPaperCard2Remove,	PNLD_PAPERTYPEIND_THICK2RMV,		PNLE_PAPERTYPE_WEIGHT_THICK2,	},	// 厚紙2.
	{ TYPE_PaperTypeThick3,			False,			s_pKEYThickPaper3_W144H028A,				CNK_SpecialPaperCard3Remove,	PNLD_PAPERTYPEIND_THICK3RMV,		PNLE_PAPERTYPE_WEIGHT_THICK3,	},	// 厚紙3.
	{ TYPE_PaperRecycle1,			False,			s_pKEYRecyclePaper_W144H028A,				CNK_NomalPaperRemove,			PNLD_PAPERTYPEIND_RECYCLERMV,		PNLE_PAPERTYPE_WEIGHT_NORMAL,	},	// 再生紙.
	// User Paper 1~6 for Tray 1
	{ TYPE_PaperTypeUser1,			False,			s_pKEYUserPaper1_W144H028A,					CNK_NomalPaper,					PNLD_PAPERTYPEIND_USER1,			PNLE_PAPERTYPE_WEIGHT_NORMAL,	},	// ユーザー紙1.
	{ TYPE_PaperTypeUser2,			False,			s_pKEYUserPaper2_W144H028A,					CNK_NomalPaper,					PNLD_PAPERTYPEIND_USER2,			PNLE_PAPERTYPE_WEIGHT_NORMAL,	},	// ユーザー紙2.
	{ TYPE_PaperTypeUser3,			False,			s_pKEYUserPaper4_W144H028A,					CNK_SpecialPaperCard1,			PNLD_PAPERTYPEIND_USER3,			PNLE_PAPERTYPE_WEIGHT_THICK1,	},	// ユーザー紙3.
	{ TYPE_PaperTypeUser4,			False,			s_pKEYUserPaper3_W144H028A,					CNK_SpecialPaperCard1P,			PNLD_PAPERTYPEIND_USER4,			PNLE_PAPERTYPE_WEIGHT_THICK1P,	},	// ユーザー紙4.
	{ TYPE_PaperTypeUser5,			False,			s_pKEYUserPaper5_W144H028A,					CNK_SpecialPaperCard2,			PNLD_PAPERTYPEIND_USER5,			PNLE_PAPERTYPE_WEIGHT_THICK2,	},	// ユーザー紙5.
	{ TYPE_PaperTypeUser6,			False,			s_pKEYUserPaper6_W144H028A,					CNK_SpecialPaperCard3,			PNLD_PAPERTYPEIND_USER6,			PNLE_PAPERTYPE_WEIGHT_THICK3,	},	// ユーザー紙6.
	// ユーザー紙(2面目)
	{ TYPE_PaperTypeUser1,			False,			s_pKEYUserPaper1_W144H028A,					CNK_NomalPaper,					PNLD_PAPERTYPEIND_USER1RMV,			PNLE_PAPERTYPE_WEIGHT_NORMAL,	},	// ユーザー紙1.
	{ TYPE_PaperTypeUser2,			False,			s_pKEYUserPaper2_W144H028A,					CNK_NomalPaper,					PNLD_PAPERTYPEIND_USER2RMV,			PNLE_PAPERTYPE_WEIGHT_NORMAL,	},	// ユーザー紙2.
	{ TYPE_PaperTypeUser3,			False,			s_pKEYUserPaper4_W144H028A,					CNK_SpecialPaperCard1,			PNLD_PAPERTYPEIND_USER3RMV,			PNLE_PAPERTYPE_WEIGHT_THICK1,	},	// ユーザー紙3.
	{ TYPE_PaperTypeUser4,			False,			s_pKEYUserPaper3_W144H028A,					CNK_SpecialPaperCard1P,			PNLD_PAPERTYPEIND_USER4RMV,			PNLE_PAPERTYPE_WEIGHT_THICK1P,	},	// ユーザー紙4.
	{ TYPE_PaperTypeUser5,			False,			s_pKEYUserPaper5_W144H028A,					CNK_SpecialPaperCard2,			PNLD_PAPERTYPEIND_USER5RMV,			PNLE_PAPERTYPE_WEIGHT_THICK2,	},	// ユーザー紙5.
	{ TYPE_PaperTypeUser6,			False,			s_pKEYUserPaper6_W144H028A,					CNK_SpecialPaperCard3,			PNLD_PAPERTYPEIND_USER6RMV,			PNLE_PAPERTYPE_WEIGHT_THICK3,	},	// ユーザー紙6.
#endif //#ifndef DEF_HeliosMLKCus
};
#endif	//DEF_A4_Cus.
// A3A4FUM 用紙種類設定 t-okada 2015/01/19 Start 選択トレイの設定変更.
#else // DEF_A4_Cus.
// 手差し以外用紙種類テーブル.
static const TYPS_PaperKindInfo st_PaperKindNewTbl[PNLD_PaperTypeTrayMax][PNLD_PaperKindNewTbl_MaxNum] = {
		// 用紙種類				両面2面目フラグ			用紙種類を表す文字列						機能種類						用紙種類画像のインデックス			用紙重量文字列のインデックス.
	// トレイ1.
	{	{ TYPE_PaperTypeNormal,			False,			s_pKEYPlainPaper_W144H028A,					CNK_NomalPaper,					PNLD_PAPERTYPEIND_NORMAL,			PNLE_PAPERTYPE_WEIGHT_NORMAL,	},	// 普通紙.
		{ TYPE_PaperTypeNotDuplex1,		False,			s_pKEYSingleSidedOnlyPlainPaper_W144H028A,	CNK_S3Paper,					PNLD_PAPERTYPEIND_SINGLSIDE,		PNLE_PAPERTYPE_WEIGHT_NORMAL,	},	// 片面専用用紙.
		{ TYPE_PaperTypeSpecial1,		False,			s_pKEYSpecialPaper_W144H028A,				CNK_SpecialPaper,				PNLD_PAPERTYPEIND_SPECIAL,			PNLE_PAPERTYPE_WEIGHT_NORMAL,	},	// 特殊紙.
		{ TYPE_PaperTypeNormalPlus,		False,			s_pKEYPlainPaperP_W144H028A,				CNK_NomalPaperPlus,				PNLD_PAPERTYPEIND_NORMALP,			PNLE_PAPERTYPE_WEIGHT_NORMALP	},	// 普通紙+.			// comment 2018/03/29 SCD2 K.Sato [Eagle][RQ-722]用紙種別追加（厚紙1を2種類に分割）対応.
		{ TYPE_PaperTypeThick1,			False,			s_pKEYThickPaper1_W144H028A,				CNK_SpecialPaperCard1,			PNLD_PAPERTYPEIND_THICK1,			PNLE_PAPERTYPE_WEIGHT_THICK1,	},	// 厚紙1.
// [KronosS] Sky荻堂 2016/09/26 管理No.22 用紙種類カスタマイズ.
//Comment 2018/03/05 【IT6_Sparrow】【HCL_Rajkumar】制御構成変更_No.222 Compiler Switch Modification.		
//#ifdef DEF_VenusMLK
#if defined ( DEF_VenusMLK ) || defined ( DEF_SparrowCus )		
		{ TYPE_PaperTypeThick1Plus,		False,			s_pKEYThickPaper1P_W144H028A,				CNK_SpecialPaperCard1P,			PNLD_PAPERTYPEIND_THICK1P,			PNLE_PAPERTYPE_WEIGHT_THICK1P,	},	// 厚紙1+.
		{ TYPE_PaperTypeThick2,			False,			s_pKEYThickPaper2_W144H028A,				CNK_SpecialPaperCard2,			PNLD_PAPERTYPEIND_THICK2,			PNLE_PAPERTYPE_WEIGHT_THICK2,	},	// 厚紙2.
		{ TYPE_PaperTypeEnvelope,		False,			s_pKEYEnvelope_W144H028A,					CNK_SpecialEnvelope,			PNLD_PAPERTYPEIND_ENVELOPE,			PNLE_PAPERTYPE_WEIGHT_NONE,		},	// 封筒.
#endif
		{ TYPE_PaperRecycle1,			False,			s_pKEYRecyclePaper_W144H028A,				CNK_NomalPaper,					PNLD_PAPERTYPEIND_RECYCLE,			PNLE_PAPERTYPE_WEIGHT_NORMAL,	},	// 再生紙.		// comment MinervaZeus ADD Sky上西 2011/05/05 [UV-01038][機器1001].
		{ TYPE_PaperTypeLetterHead,		False,			s_pKEYLetterhead_W144H028A,					CNK_LetterHeadPaper,			PNLD_PAPERTYPEIND_LTRHEAD,			PNLE_PAPERTYPE_WEIGHT_NORMAL,	},	// レターヘッド紙.
		{ TYPE_PaperTypeColor,			False,			s_pKEYColorPaper_W144H028A,					CNK_ColorPaper,					PNLD_PAPERTYPEIND_COLOR,			PNLE_PAPERTYPE_WEIGHT_NORMAL,	},	// 色紙.
// [KronosS] Sky荻堂 2016/09/26 管理No.22 用紙種類カスタマイズ.
//Comment 2018/03/05 【IT6_Sparrow】【HCL_Rajkumar】制御構成変更_No.222 Compiler Switch Modification.
//#ifdef DEF_VenusMLK
#if defined ( DEF_VenusMLK ) || defined ( DEF_SparrowCus )
		{ TYPE_PaperTypeGloss1,			False,			s_pKEYGlossPaper1_W144H028A,				CNK_GlossPaper1,				PNLD_PAPERTYPEIND_GLOSS1,			PNLE_PAPERTYPE_WEIGHT_GLOSS1,	},	// 光沢紙1.
		{ TYPE_PaperTypeGloss1Plus,		False,			s_pKEYGlossPaper1Plus_W144H028A,			CNK_GlossPaper1Plus,			PNLD_PAPERTYPEIND_GLOSS1P,			PNLE_PAPERTYPE_WEIGHT_GLOSS1P,	},	// 光沢紙1+.
		{ TYPE_PaperTypeGloss2,			False,			s_pKEYGlossPaper2_W144H028A,				CNK_GlossPaper2,				PNLD_PAPERTYPEIND_GLOSS2,			PNLE_PAPERTYPE_WEIGHT_GLOSS2,	},	// 光沢紙2.
		{ TYPE_PaperPostCard,			False,			s_pKEYPostCard_W144H028A,					CNK_PostCard,					PNLD_PAPERTYPEIND_POSTCARD,			PNLE_PAPERTYPE_WEIGHT_NONE,		},	// はがき.
#endif
		{ TYPE_PaperTypeLabel,			False,			s_pKEYLabelPaper1_W144H028A,				CNK_LabelPaper,					PNLD_PAPERTYPEIND_LABEL,			PNLE_PAPERTYPE_WEIGHT_NONE,		},	// ラベル紙.
// Comment KronosS -> 2017/03/01 SCD2 3G Sky瀬戸 [AccuRev.1012111]ユーザー紙坪量問題.
#ifdef DEF_KRONOSS_Cus
		// Comment KronosS 2017/03/01 SCD2 3G Sky瀬戸 [AccuRev.1012111]ユーザー紙坪量問題.
		// ユーザー紙も坪量固定となるため設定値を「0」から変更.
		//// ユーザー紙(※坪量はチョイスで変更可能なため「0」としておく).
		{ TYPE_PaperTypeUser1,			False,			s_pKEYUserPaper1_W144H028A,					CNK_NomalPaper,					PNLD_PAPERTYPEIND_USER1,			PNLE_PAPERTYPE_WEIGHT_NORMAL,	},	// ユーザー紙1.
		{ TYPE_PaperTypeUser2,			False,			s_pKEYUserPaper2_W144H028A,					CNK_NomalPaper,					PNLD_PAPERTYPEIND_USER2,			PNLE_PAPERTYPE_WEIGHT_NORMAL,	},	// ユーザー紙2.
		{ TYPE_PaperTypeUser3,			False,			s_pKEYUserPaper4_W144H028A,					CNK_SpecialPaperCard1,			PNLD_PAPERTYPEIND_USER3,			PNLE_PAPERTYPE_WEIGHT_THICK1,	},	// ユーザー紙3.
#else // DEF_KRONOSS_Cus.
		// ユーザー紙(※坪量はチョイスで変更可能なため「0」としておく).
		{ TYPE_PaperTypeUser1,			False,			s_pKEYUserPaper1_W144H028A,					CNK_NomalPaper,					PNLD_PAPERTYPEIND_USER1,			PNLE_PAPERTYPE_WEIGHT_NONE,	},		// ユーザー紙1.
// comment 2018/04/17 SCD2-Sky鳫林 [Eagle][RQ-722] 普通紙+追加対応.
//		{ TYPE_PaperTypeUser2,			False,			s_pKEYUserPaper2_W144H028A,					CNK_NomalPaper,					PNLD_PAPERTYPEIND_USER2,			PNLE_PAPERTYPE_WEIGHT_NONE,	},		// ユーザー紙2.
		{ TYPE_PaperTypeUser2,			False,			s_pKEYUserPaper2_W144H028A,					CNK_NomalPaperPlus,				PNLD_PAPERTYPEIND_USER2,			PNLE_PAPERTYPE_WEIGHT_NONE,	},		// ユーザー紙2.
		{ TYPE_PaperTypeUser3,			False,			s_pKEYUserPaper4_W144H028A,					CNK_SpecialPaperCard1,			PNLD_PAPERTYPEIND_USER3,			PNLE_PAPERTYPE_WEIGHT_NONE,	},		// ユーザー紙3.
		{ TYPE_PaperTypeUser4,			False,			s_pKEYUserPaper3_W144H028A,					CNK_SpecialPaperCard1P,			PNLD_PAPERTYPEIND_USER4,			PNLE_PAPERTYPE_WEIGHT_NONE,	},		// ユーザー紙4.
		{ TYPE_PaperTypeUser5,			False,			s_pKEYUserPaper5_W144H028A,					CNK_SpecialPaperCard2,			PNLD_PAPERTYPEIND_USER5,			PNLE_PAPERTYPE_WEIGHT_NONE,	},		// ユーザー紙5.
#endif // DEF_KRONOSS_Cus.
// Comment KronosS <- 2017/03/01 SCD2 3G Sky瀬戸 [AccuRev.1012111]ユーザー紙坪量問題.
	},
	// トレイ2.
	{	{ TYPE_PaperTypeNormal,			False,			s_pKEYPlainPaper_W144H028A,					CNK_NomalPaper,					PNLD_PAPERTYPEIND_NORMAL,			PNLE_PAPERTYPE_WEIGHT_NORMAL,	},	// 普通紙.
		{ TYPE_PaperTypeNotDuplex1,		False,			s_pKEYSingleSidedOnlyPlainPaper_W144H028A,	CNK_S3Paper,					PNLD_PAPERTYPEIND_SINGLSIDE,		PNLE_PAPERTYPE_WEIGHT_NORMAL,	},	// 片面専用用紙.
		{ TYPE_PaperTypeSpecial1,		False,			s_pKEYSpecialPaper_W144H028A,				CNK_SpecialPaper,				PNLD_PAPERTYPEIND_SPECIAL,			PNLE_PAPERTYPE_WEIGHT_NORMAL,	},	// 特殊紙.
		{ TYPE_PaperTypeNormalPlus,		False,			s_pKEYPlainPaperP_W144H028A,				CNK_NomalPaperPlus,				PNLD_PAPERTYPEIND_NORMALP,			PNLE_PAPERTYPE_WEIGHT_NORMALP	},	// 普通紙+.			// comment 2018/03/29 SCD2 K.Sato [Eagle][RQ-722]用紙種別追加（厚紙1を2種類に分割）対応.
// [KronosS] Sky荻堂 2016/09/26 管理No.22 用紙種類カスタマイズ.
#ifdef DEF_KRONOSS_Cus
		{ TYPE_PaperTypeThick1,			False,			s_pKEYThickPaper1_W144H028A,				CNK_SpecialPaperCard1,			PNLD_PAPERTYPEIND_THICK1,			PNLE_PAPERTYPE_WEIGHT_THICK1,	},	// 厚紙1.
#endif
		{ TYPE_PaperRecycle1,			False,			s_pKEYRecyclePaper_W144H028A,				CNK_NomalPaper,					PNLD_PAPERTYPEIND_RECYCLE,			PNLE_PAPERTYPE_WEIGHT_NORMAL,	},	// 再生紙.		// comment MinervaZeus ADD Sky上西 2011/05/05 [UV-01038][機器1001].
// comment 2019/03/22 SCD2-Sky鳥居 [特ROM][FQ-OPSD-1800408-OFP218]KronosS MFP / トレイ２にレターヘッド設定ができてしまう対策.
//		{ TYPE_PaperTypeLetterHead,		False,			s_pKEYLetterhead_W144H028A,					CNK_LetterHeadPaper,			PNLD_PAPERTYPEIND_LTRHEAD,			PNLE_PAPERTYPE_WEIGHT_NORMAL,	},	// レターヘッド紙.
// Comment 2023/09/07 FPT-ThinhND40 [IT6_2.3] [ 機器構成変更_No.55 ] Removal of prohibition on letterhead of additional cassettes.
		{ TYPE_PaperTypeLetterHead,		False,			s_pKEYLetterhead_W144H028A,					CNK_LetterHeadPaper,			PNLD_PAPERTYPEIND_LTRHEAD,			PNLE_PAPERTYPE_WEIGHT_NORMAL,	},	// レターヘッド紙.
		{ TYPE_PaperTypeColor,			False,			s_pKEYColorPaper_W144H028A,					CNK_ColorPaper,					PNLD_PAPERTYPEIND_COLOR,			PNLE_PAPERTYPE_WEIGHT_NORMAL,	},	// 色紙.
// [KronosS] Sky荻堂 2016/09/26 管理No.22 用紙種類カスタマイズ.
#ifdef DEF_KRONOSS_Cus
		{ TYPE_PaperTypeLabel,			False,			s_pKEYLabelPaper1_W144H028A,				CNK_LabelPaper,					PNLD_PAPERTYPEIND_LABEL,			PNLE_PAPERTYPE_WEIGHT_NONE,		},	// ラベル紙.
#endif
// Comment KronosS -> 2017/03/01 SCD2 3G Sky瀬戸 [AccuRev.1012111]ユーザー紙坪量問題.
#ifdef DEF_KRONOSS_Cus
		// Comment KronosS 2017/03/01 SCD2 3G Sky瀬戸 [AccuRev.1012111]ユーザー紙坪量問題.
		// ユーザー紙も坪量固定となるため設定値を「0」から変更.
		//// ユーザー紙(※坪量はチョイスで変更可能なため「0」としておく).
		{ TYPE_PaperTypeUser1,			False,			s_pKEYUserPaper1_W144H028A,					CNK_NomalPaper,					PNLD_PAPERTYPEIND_USER1,			PNLE_PAPERTYPE_WEIGHT_NORMAL,	},	// ユーザー紙1.
		{ TYPE_PaperTypeUser2,			False,			s_pKEYUserPaper2_W144H028A,					CNK_NomalPaper,					PNLD_PAPERTYPEIND_USER2,			PNLE_PAPERTYPE_WEIGHT_NORMAL,	},	// ユーザー紙2.
		{ TYPE_PaperTypeUser3,			False,			s_pKEYUserPaper4_W144H028A,					CNK_SpecialPaperCard1,			PNLD_PAPERTYPEIND_USER3,			PNLE_PAPERTYPE_WEIGHT_THICK1,	},	// ユーザー紙3.
#else // DEF_KRONOSS_Cus.
		// ユーザー紙(※坪量はチョイスで変更可能なため「0」としておく).
		{ TYPE_PaperTypeUser1,			False,			s_pKEYUserPaper1_W144H028A,					CNK_NomalPaper,					PNLD_PAPERTYPEIND_USER1,			PNLE_PAPERTYPE_WEIGHT_NONE,	},		// ユーザー紙1.
// comment 2018/04/17 SCD2-Sky鳫林 [Eagle][RQ-722] 普通紙+追加対応.
//		{ TYPE_PaperTypeUser2,			False,			s_pKEYUserPaper2_W144H028A,					CNK_NomalPaper,					PNLD_PAPERTYPEIND_USER2,			PNLE_PAPERTYPE_WEIGHT_NONE,	},		// ユーザー紙2.
		{ TYPE_PaperTypeUser2,			False,			s_pKEYUserPaper2_W144H028A,					CNK_NomalPaperPlus,				PNLD_PAPERTYPEIND_USER2,			PNLE_PAPERTYPE_WEIGHT_NONE,	},		// ユーザー紙2.
// Comment 2023/09/07 FPT-ThinhND40 [IT6_2.3] [ 機器構成変更_No.55 ] Removal of prohibition on letterhead of additional cassettes.
//		{ TYPE_PaperTypeUnknown,		False,			0,											CNK_NomalPaper,					PNLD_PAPERTYPEIND_NON,				PNLE_PAPERTYPE_WEIGHT_NONE,	},
		{ TYPE_PaperTypeUnknown,		False,			0,											CNK_NomalPaper,					PNLD_PAPERTYPEIND_NON,				PNLE_PAPERTYPE_WEIGHT_NONE,	},
		{ TYPE_PaperTypeUnknown,		False,			0,											CNK_NomalPaper,					PNLD_PAPERTYPEIND_NON,				PNLE_PAPERTYPE_WEIGHT_NONE,	},
		{ TYPE_PaperTypeUnknown,		False,			0,											CNK_NomalPaper,					PNLD_PAPERTYPEIND_NON,				PNLE_PAPERTYPE_WEIGHT_NONE,	},
		{ TYPE_PaperTypeUnknown,		False,			0,											CNK_NomalPaper,					PNLD_PAPERTYPEIND_NON,				PNLE_PAPERTYPE_WEIGHT_NONE,	},
		{ TYPE_PaperTypeUnknown,		False,			0,											CNK_NomalPaper,					PNLD_PAPERTYPEIND_NON,				PNLE_PAPERTYPE_WEIGHT_NONE,	},
		{ TYPE_PaperTypeUnknown,		False,			0,											CNK_NomalPaper,					PNLD_PAPERTYPEIND_NON,				PNLE_PAPERTYPE_WEIGHT_NONE,	},
		{ TYPE_PaperTypeUnknown,		False,			0,											CNK_NomalPaper,					PNLD_PAPERTYPEIND_NON,				PNLE_PAPERTYPE_WEIGHT_NONE,	},
		{ TYPE_PaperTypeUnknown,		False,			0,											CNK_NomalPaper,					PNLD_PAPERTYPEIND_NON,				PNLE_PAPERTYPE_WEIGHT_NONE,	},
		{ TYPE_PaperTypeUnknown,		False,			0,											CNK_NomalPaper,					PNLD_PAPERTYPEIND_NON,				PNLE_PAPERTYPE_WEIGHT_NONE,	},
		{ TYPE_PaperTypeUnknown,		False,			0,											CNK_NomalPaper,					PNLD_PAPERTYPEIND_NON,				PNLE_PAPERTYPE_WEIGHT_NONE,	},
		{ TYPE_PaperTypeUnknown,		False,			0,											CNK_NomalPaper,					PNLD_PAPERTYPEIND_NON,				PNLE_PAPERTYPE_WEIGHT_NONE,	},
#endif // DEF_KRONOSS_Cus.
		{ TYPE_PaperTypeUnknown,		False,			0,											CNK_NomalPaper,					PNLD_PAPERTYPEIND_NON,				PNLE_PAPERTYPE_WEIGHT_NONE,	},
// Comment KronosS <- 2017/03/01 SCD2 3G Sky瀬戸 [AccuRev.1012111]ユーザー紙坪量問題.
	},
	// トレイ3.
	{	{ TYPE_PaperTypeNormal,			False,			s_pKEYPlainPaper_W144H028A,					CNK_NomalPaper,					PNLD_PAPERTYPEIND_NORMAL,			PNLE_PAPERTYPE_WEIGHT_NORMAL,	},	// 普通紙.
		{ TYPE_PaperTypeNotDuplex1,		False,			s_pKEYSingleSidedOnlyPlainPaper_W144H028A,	CNK_S3Paper,					PNLD_PAPERTYPEIND_SINGLSIDE,		PNLE_PAPERTYPE_WEIGHT_NORMAL,	},	// 片面専用用紙.
		{ TYPE_PaperTypeSpecial1,		False,			s_pKEYSpecialPaper_W144H028A,				CNK_SpecialPaper,				PNLD_PAPERTYPEIND_SPECIAL,			PNLE_PAPERTYPE_WEIGHT_NORMAL,	},	// 特殊紙.
		{ TYPE_PaperTypeNormalPlus,		False,			s_pKEYPlainPaperP_W144H028A,				CNK_NomalPaperPlus,				PNLD_PAPERTYPEIND_NORMALP,			PNLE_PAPERTYPE_WEIGHT_NORMALP	},	// 普通紙+.			// comment 2018/03/29 SCD2 K.Sato [Eagle][RQ-722]用紙種別追加（厚紙1を2種類に分割）対応.
// [KronosS] Sky荻堂 2016/09/26 管理No.22 用紙種類カスタマイズ.
#ifdef DEF_KRONOSS_Cus
		{ TYPE_PaperTypeThick1,			False,			s_pKEYThickPaper1_W144H028A,				CNK_SpecialPaperCard1,			PNLD_PAPERTYPEIND_THICK1,			PNLE_PAPERTYPE_WEIGHT_THICK1,	},	// 厚紙1.
#endif
		{ TYPE_PaperRecycle1,			False,			s_pKEYRecyclePaper_W144H028A,				CNK_NomalPaper,					PNLD_PAPERTYPEIND_RECYCLE,			PNLE_PAPERTYPE_WEIGHT_NORMAL,	},	// 再生紙.		// comment MinervaZeus ADD Sky上西 2011/05/05 [UV-01038][機器1001].
// comment 2019/03/22 SCD2-Sky鳥居 [特ROM][FQ-OPSD-1800408-OFP218]KronosS MFP / トレイ２にレターヘッド設定ができてしまう対策.
//		{ TYPE_PaperTypeLetterHead,		False,			s_pKEYLetterhead_W144H028A,					CNK_LetterHeadPaper,			PNLD_PAPERTYPEIND_LTRHEAD,			PNLE_PAPERTYPE_WEIGHT_NORMAL,	},	// レターヘッド紙.
// Comment 2023/09/07 FPT-ThinhND40 [IT6_2.3] [ 機器構成変更_No.55 ] Removal of prohibition on letterhead of additional cassettes.
		{ TYPE_PaperTypeLetterHead,		False,			s_pKEYLetterhead_W144H028A,					CNK_LetterHeadPaper,			PNLD_PAPERTYPEIND_LTRHEAD,			PNLE_PAPERTYPE_WEIGHT_NORMAL,	},	// レターヘッド紙.
		{ TYPE_PaperTypeColor,			False,			s_pKEYColorPaper_W144H028A,					CNK_ColorPaper,					PNLD_PAPERTYPEIND_COLOR,			PNLE_PAPERTYPE_WEIGHT_NORMAL,	},	// 色紙.
// [KronosS] Sky荻堂 2016/09/26 管理No.22 用紙種類カスタマイズ.
#ifdef DEF_KRONOSS_Cus
		{ TYPE_PaperTypeLabel,			False,			s_pKEYLabelPaper1_W144H028A,				CNK_LabelPaper,					PNLD_PAPERTYPEIND_LABEL,			PNLE_PAPERTYPE_WEIGHT_NONE,		},	// ラベル紙.
#endif
// Comment KronosS -> 2017/03/01 SCD2 3G Sky瀬戸 [AccuRev.1012111]ユーザー紙坪量問題.
#ifdef DEF_KRONOSS_Cus
		// Comment KronosS 2017/03/01 SCD2 3G Sky瀬戸 [AccuRev.1012111]ユーザー紙坪量問題.
		// ユーザー紙も坪量固定となるため設定値を「0」から変更.
		//// ユーザー紙(※坪量はチョイスで変更可能なため「0」としておく).
		{ TYPE_PaperTypeUser1,			False,			s_pKEYUserPaper1_W144H028A,					CNK_NomalPaper,					PNLD_PAPERTYPEIND_USER1,			PNLE_PAPERTYPE_WEIGHT_NORMAL,	},	// ユーザー紙1.
		{ TYPE_PaperTypeUser2,			False,			s_pKEYUserPaper2_W144H028A,					CNK_NomalPaper,					PNLD_PAPERTYPEIND_USER2,			PNLE_PAPERTYPE_WEIGHT_NORMAL,	},	// ユーザー紙2.
		{ TYPE_PaperTypeUser3,			False,			s_pKEYUserPaper4_W144H028A,					CNK_SpecialPaperCard1,			PNLD_PAPERTYPEIND_USER3,			PNLE_PAPERTYPE_WEIGHT_THICK1,	},	// ユーザー紙3.
#else // DEF_KRONOSS_Cus.
		// ユーザー紙(※坪量はチョイスで変更可能なため「0」としておく).
		{ TYPE_PaperTypeUser1,			False,			s_pKEYUserPaper1_W144H028A,					CNK_NomalPaper,					PNLD_PAPERTYPEIND_USER1,			PNLE_PAPERTYPE_WEIGHT_NONE,	},		// ユーザー紙1.
// comment 2018/04/17 SCD2-Sky鳫林 [Eagle][RQ-722] 普通紙+追加対応.
//		{ TYPE_PaperTypeUser2,			False,			s_pKEYUserPaper2_W144H028A,					CNK_NomalPaper,					PNLD_PAPERTYPEIND_USER2,			PNLE_PAPERTYPE_WEIGHT_NONE,	},		// ユーザー紙2.
		{ TYPE_PaperTypeUser2,			False,			s_pKEYUserPaper2_W144H028A,					CNK_NomalPaperPlus,				PNLD_PAPERTYPEIND_USER2,			PNLE_PAPERTYPE_WEIGHT_NONE,	},		// ユーザー紙2.
// Comment 2023/09/07 FPT-ThinhND40 [IT6_2.3] [ 機器構成変更_No.55 ] Removal of prohibition on letterhead of additional cassettes.
//		{ TYPE_PaperTypeUnknown,		False,			0,											CNK_NomalPaper,					PNLD_PAPERTYPEIND_NON,				PNLE_PAPERTYPE_WEIGHT_NONE,	},
		{ TYPE_PaperTypeUnknown,		False,			0,											CNK_NomalPaper,					PNLD_PAPERTYPEIND_NON,				PNLE_PAPERTYPE_WEIGHT_NONE,	},
		{ TYPE_PaperTypeUnknown,		False,			0,											CNK_NomalPaper,					PNLD_PAPERTYPEIND_NON,				PNLE_PAPERTYPE_WEIGHT_NONE,	},
		{ TYPE_PaperTypeUnknown,		False,			0,											CNK_NomalPaper,					PNLD_PAPERTYPEIND_NON,				PNLE_PAPERTYPE_WEIGHT_NONE,	},
		{ TYPE_PaperTypeUnknown,		False,			0,											CNK_NomalPaper,					PNLD_PAPERTYPEIND_NON,				PNLE_PAPERTYPE_WEIGHT_NONE,	},
		{ TYPE_PaperTypeUnknown,		False,			0,											CNK_NomalPaper,					PNLD_PAPERTYPEIND_NON,				PNLE_PAPERTYPE_WEIGHT_NONE,	},
		{ TYPE_PaperTypeUnknown,		False,			0,											CNK_NomalPaper,					PNLD_PAPERTYPEIND_NON,				PNLE_PAPERTYPE_WEIGHT_NONE,	},
		{ TYPE_PaperTypeUnknown,		False,			0,											CNK_NomalPaper,					PNLD_PAPERTYPEIND_NON,				PNLE_PAPERTYPE_WEIGHT_NONE,	},
		{ TYPE_PaperTypeUnknown,		False,			0,											CNK_NomalPaper,					PNLD_PAPERTYPEIND_NON,				PNLE_PAPERTYPE_WEIGHT_NONE,	},
		{ TYPE_PaperTypeUnknown,		False,			0,											CNK_NomalPaper,					PNLD_PAPERTYPEIND_NON,				PNLE_PAPERTYPE_WEIGHT_NONE,	},
		{ TYPE_PaperTypeUnknown,		False,			0,											CNK_NomalPaper,					PNLD_PAPERTYPEIND_NON,				PNLE_PAPERTYPE_WEIGHT_NONE,	},
		{ TYPE_PaperTypeUnknown,		False,			0,											CNK_NomalPaper,					PNLD_PAPERTYPEIND_NON,				PNLE_PAPERTYPE_WEIGHT_NONE,	},
#endif // DEF_KRONOSS_Cus.
		{ TYPE_PaperTypeUnknown,		False,			0,											CNK_NomalPaper,					PNLD_PAPERTYPEIND_NON,				PNLE_PAPERTYPE_WEIGHT_NONE,	},
// Comment KronosS <- 2017/03/01 SCD2 3G Sky瀬戸 [AccuRev.1012111]ユーザー紙坪量問題.
	},
// [KronosS] Sky川名 2016/10/25 管理No21_44 各トレイ毎の表示用紙サイズをKronosS仕様にあわせる.
#ifdef DEF_KRONOSS_Cus
	// トレイ4.
	{	{ TYPE_PaperTypeNormal,			False,			s_pKEYPlainPaper_W144H028A,					CNK_NomalPaper,					PNLD_PAPERTYPEIND_NORMAL,			PNLE_PAPERTYPE_WEIGHT_NORMAL,	},	// 普通紙.
		{ TYPE_PaperTypeNotDuplex1,		False,			s_pKEYSingleSidedOnlyPlainPaper_W144H028A,	CNK_S3Paper,					PNLD_PAPERTYPEIND_SINGLSIDE,		PNLE_PAPERTYPE_WEIGHT_NORMAL,	},	// 片面専用用紙.
		{ TYPE_PaperTypeSpecial1,		False,			s_pKEYSpecialPaper_W144H028A,				CNK_SpecialPaper,				PNLD_PAPERTYPEIND_SPECIAL,			PNLE_PAPERTYPE_WEIGHT_NORMAL,	},	// 特殊紙.
		{ TYPE_PaperTypeThick1,			False,			s_pKEYThickPaper1_W144H028A,				CNK_SpecialPaperCard1,			PNLD_PAPERTYPEIND_THICK1,			PNLE_PAPERTYPE_WEIGHT_THICK1,	},	// 厚紙1.
		{ TYPE_PaperRecycle1,			False,			s_pKEYRecyclePaper_W144H028A,				CNK_NomalPaper,					PNLD_PAPERTYPEIND_RECYCLE,			PNLE_PAPERTYPE_WEIGHT_NORMAL,	},	// 再生紙.
// comment 2019/03/22 SCD2-Sky鳥居 [特ROM][FQ-OPSD-1800408-OFP218]KronosS MFP / トレイ２にレターヘッド設定ができてしまう対策.
//		{ TYPE_PaperTypeLetterHead,		False,			s_pKEYLetterhead_W144H028A,					CNK_LetterHeadPaper,			PNLD_PAPERTYPEIND_LTRHEAD,			PNLE_PAPERTYPE_WEIGHT_NORMAL,	},	// レターヘッド紙. //[KronosS] Sky川名 2016/12/21 課題番号:1007853 選択トレイ画面にレターヘッド紙キーが表示されない.
		{ TYPE_PaperTypeColor,			False,			s_pKEYColorPaper_W144H028A,					CNK_ColorPaper,					PNLD_PAPERTYPEIND_COLOR,			PNLE_PAPERTYPE_WEIGHT_NORMAL,	},	// 色紙.
		{ TYPE_PaperTypeLabel,			False,			s_pKEYLabelPaper1_W144H028A,				CNK_LabelPaper,					PNLD_PAPERTYPEIND_LABEL,			PNLE_PAPERTYPE_WEIGHT_NONE,		},	// ラベル紙.
		// Comment KronosS 2017/03/01 SCD2 3G Sky瀬戸 [AccuRev.1012111]ユーザー紙坪量問題.
		// ユーザー紙も坪量固定となるため設定値を「0」から変更.
		//// ユーザー紙(※坪量はチョイスで変更可能なため「0」としておく).
		{ TYPE_PaperTypeUser1,			False,			s_pKEYUserPaper1_W144H028A,					CNK_NomalPaper,					PNLD_PAPERTYPEIND_USER1,			PNLE_PAPERTYPE_WEIGHT_NORMAL,	},	// ユーザー紙1.
		{ TYPE_PaperTypeUser2,			False,			s_pKEYUserPaper2_W144H028A,					CNK_NomalPaper,					PNLD_PAPERTYPEIND_USER2,			PNLE_PAPERTYPE_WEIGHT_NORMAL,	},	// ユーザー紙2.
		{ TYPE_PaperTypeUser3,			False,			s_pKEYUserPaper4_W144H028A,					CNK_SpecialPaperCard1,			PNLD_PAPERTYPEIND_USER3,			PNLE_PAPERTYPE_WEIGHT_THICK1,	},	// ユーザー紙3.
		{ TYPE_PaperTypeUnknown,		False,			0,											CNK_NomalPaper,					PNLD_PAPERTYPEIND_NON,				PNLE_PAPERTYPE_WEIGHT_NONE,	},
	},
//comment 2023/07/25 FPT.LONGTH32 [IT6_2.3] 機器構成変更_No.75 Add tray 4 for Sparrow.
#else
	// トレイ4.
	{	{ TYPE_PaperTypeNormal,			False,			s_pKEYPlainPaper_W144H028A,					CNK_NomalPaper,					PNLD_PAPERTYPEIND_NORMAL,			PNLE_PAPERTYPE_WEIGHT_NORMAL,	},	// 普通紙.
		{ TYPE_PaperTypeNotDuplex1,		False,			s_pKEYSingleSidedOnlyPlainPaper_W144H028A,	CNK_S3Paper,					PNLD_PAPERTYPEIND_SINGLSIDE,		PNLE_PAPERTYPE_WEIGHT_NORMAL,	},	// 片面専用用紙.
		{ TYPE_PaperTypeSpecial1,		False,			s_pKEYSpecialPaper_W144H028A,				CNK_SpecialPaper,				PNLD_PAPERTYPEIND_SPECIAL,			PNLE_PAPERTYPE_WEIGHT_NORMAL,	},	// 特殊紙.
		{ TYPE_PaperTypeNormalPlus,		False,			s_pKEYPlainPaperP_W144H028A,				CNK_NomalPaperPlus,				PNLD_PAPERTYPEIND_NORMALP,			PNLE_PAPERTYPE_WEIGHT_NORMALP	},	// 普通紙+.
		{ TYPE_PaperRecycle1,			False,			s_pKEYRecyclePaper_W144H028A,				CNK_NomalPaper,					PNLD_PAPERTYPEIND_RECYCLE,			PNLE_PAPERTYPE_WEIGHT_NORMAL,	},	// 再生紙.
// Comment 2023/09/07 FPT-ThinhND40 [IT6_2.3] [ 機器構成変更_No.55 ] Removal of prohibition on letterhead of additional cassettes.
		{ TYPE_PaperTypeLetterHead,		False,			s_pKEYLetterhead_W144H028A,					CNK_LetterHeadPaper,			PNLD_PAPERTYPEIND_LTRHEAD,			PNLE_PAPERTYPE_WEIGHT_NORMAL,	},	// レターヘッド紙.
		{ TYPE_PaperTypeColor,			False,			s_pKEYColorPaper_W144H028A,					CNK_ColorPaper,					PNLD_PAPERTYPEIND_COLOR,			PNLE_PAPERTYPE_WEIGHT_NORMAL,	},	// 色紙.
		{ TYPE_PaperTypeUser1,			False,			s_pKEYUserPaper1_W144H028A,					CNK_NomalPaper,					PNLD_PAPERTYPEIND_USER1,			PNLE_PAPERTYPE_WEIGHT_NONE,	},		// ユーザー紙1.
		{ TYPE_PaperTypeUser2,			False,			s_pKEYUserPaper2_W144H028A,					CNK_NomalPaperPlus,				PNLD_PAPERTYPEIND_USER2,			PNLE_PAPERTYPE_WEIGHT_NONE,	},		// ユーザー紙2.
//		{ TYPE_PaperTypeUnknown,		False,			0,											CNK_NomalPaper,					PNLD_PAPERTYPEIND_NON,				PNLE_PAPERTYPE_WEIGHT_NONE,	},
		{ TYPE_PaperTypeUnknown,		False,			0,											CNK_NomalPaper,					PNLD_PAPERTYPEIND_NON,				PNLE_PAPERTYPE_WEIGHT_NONE,	},
		{ TYPE_PaperTypeUnknown,		False,			0,											CNK_NomalPaper,					PNLD_PAPERTYPEIND_NON,				PNLE_PAPERTYPE_WEIGHT_NONE,	},
		{ TYPE_PaperTypeUnknown,		False,			0,											CNK_NomalPaper,					PNLD_PAPERTYPEIND_NON,				PNLE_PAPERTYPE_WEIGHT_NONE,	},
		{ TYPE_PaperTypeUnknown,		False,			0,											CNK_NomalPaper,					PNLD_PAPERTYPEIND_NON,				PNLE_PAPERTYPE_WEIGHT_NONE,	},
		{ TYPE_PaperTypeUnknown,		False,			0,											CNK_NomalPaper,					PNLD_PAPERTYPEIND_NON,				PNLE_PAPERTYPE_WEIGHT_NONE,	},
		{ TYPE_PaperTypeUnknown,		False,			0,											CNK_NomalPaper,					PNLD_PAPERTYPEIND_NON,				PNLE_PAPERTYPE_WEIGHT_NONE,	},
		{ TYPE_PaperTypeUnknown,		False,			0,											CNK_NomalPaper,					PNLD_PAPERTYPEIND_NON,				PNLE_PAPERTYPE_WEIGHT_NONE,	},
		{ TYPE_PaperTypeUnknown,		False,			0,											CNK_NomalPaper,					PNLD_PAPERTYPEIND_NON,				PNLE_PAPERTYPE_WEIGHT_NONE,	},
		{ TYPE_PaperTypeUnknown,		False,			0,											CNK_NomalPaper,					PNLD_PAPERTYPEIND_NON,				PNLE_PAPERTYPE_WEIGHT_NONE,	},
		{ TYPE_PaperTypeUnknown,		False,			0,											CNK_NomalPaper,					PNLD_PAPERTYPEIND_NON,				PNLE_PAPERTYPE_WEIGHT_NONE,	},
		{ TYPE_PaperTypeUnknown,		False,			0,											CNK_NomalPaper,					PNLD_PAPERTYPEIND_NON,				PNLE_PAPERTYPE_WEIGHT_NONE,	},
		{ TYPE_PaperTypeUnknown,		False,			0,											CNK_NomalPaper,					PNLD_PAPERTYPEIND_NON,				PNLE_PAPERTYPE_WEIGHT_NONE,	},
	},
#endif // DEF_KRONOSS_Cus.
};
#endif // DEF_A4_Cus.
// A3A4FUM 用紙種類設定 t-okada 2015/01/19 End.

static const TYPS_PaperKindInfo st_PaperKindNewTbl_SP[] = {
	{ TYPE_PaperTypeEnvelope,		False,			s_pKEYEnvelope_W144H028A,					CNK_SpecialEnvelope,			PNLD_PAPERTYPEIND_ENVELOPE,			PNLE_PAPERTYPE_WEIGHT_NONE,		},		// 封筒.
};

// Comment 2017/10/04 【IT6_Eagle】【HCL_Ranjith】 【制御構成変更_No.1】Removed existing compiler switch, giving support to eagle.
#ifdef DEF_A4_Cus
// 用紙重量文字列テーブル.
TEXT_ID puc_AryNewPaperWeight[] = {
// 今後用紙種類に増減する事を踏まえ、参照用indexを定数定義した.
	s_Space2,						// スペース.		PNLE_PAPERTYPE_WEIGHT_NONE.
//Comment 2018/03/05 【IT6_Sparrow】【HCL_Rajkumar】制御構成変更_No.222 Message Modification. 	
#ifdef DEF_LCD_7INCH	
	s_pNOT52to59gm2_W060H040A,		// 50～59g/㎡(仮).	PNLE_PAPERTYPE_WEIGHT_THIN.
#else // DEF_LCD_7INCH.	
	s_pNOT52to59gm2_W128H018A,
#endif // DEF_LCD_7INCH.	
	s_pNOT60to90gm2_W128H018A,		// 60～90g/㎡.		PNLE_PAPERTYPE_WEIGHT_NORMAL.
// comment 2018/04/17 SCD2-Sky鳫林 [Eagle][RQ-722]用紙種類追加（厚紙１を２種類に分割）対応.
//	s_pNOT106to120gm2_W064H033A,	// 106～120g/㎡.		PNLE_PAPERTYPE_WEIGHT_THICK1.		// comment 2018/03/29 SCD2 K.Sato [Eagle][RQ-722]用紙種別追加（厚紙1を2種類に分割）対応.
	s_pNOT106to120gm2_W128H018A,	// 106～120g/㎡.		PNLE_PAPERTYPE_WEIGHT_THICK1.
	s_pNOT121To157gm2_W128H018A,	// 121～157g/㎡.	PNLE_PAPERTYPE_WEIGHT_THICK1P.
// [KronosS] Sky川名 2016/12/20 課題番号:1007827 厚紙2の坪量の値が機能仕様書と異なっている.
#ifdef DEF_KRONOSS_Cus
	s_pNOT158To216gm2_W128H018B,	// 158～216g/㎡.	PNLE_PAPERTYPE_WEIGHT_THICK2.
#else // DEF_KRONOSS_Cus.
//comment 2020/01/31 HCL_Aruna [IT6_SparrowBK][OP_BTS-11734] Changing the symbol ID	
//	s_pNOT158To209gm2_W128H018A,	// 158～209g/㎡.	PNLE_PAPERTYPE_WEIGHT_THICK2.
	s_pNOTITMThick2BasisWeight_W128H018A, 	// 158～210g/㎡.PNLE_PAPERTYPE_WEIGHT_THICK2.
#endif // DEF_KRONOSS_Cus.
	s_pNOT210To256gm2_W128H018A,	// 210～256g/㎡.	PNLE_PAPERTYPE_WEIGHT_THICK3.
	s_pNOT257To300gm2_W128H018A,	// 257～300g/㎡.	PNLE_PAPERTYPE_WEIGHT_THICK4.
	s_pNOT100To120gm2_W128H018A,	// 100～120g/㎡.	PNLE_PAPERTYPE_WEIGHT_GLOSS1.
	s_pNOT121To157gm2_W128H018A,	// 121～157g/㎡.	PNLE_PAPERTYPE_WEIGHT_GLOSS1P.
	s_pNOT158gm2_W128H018A,			// 158g/㎡. 		PNLE_PAPERTYPE_WEIGHT_GLOSS2.
// comment 2018/04/17 SCD2-Sky鳫林 [Eagle][RQ-722]用紙種類追加（厚紙１を２種類に分割）対応.
//	s_pNOT91to105gm2_W064H033A,		// 91～105g/㎡.		PNLE_PAPERTYPE_WEIGHT_NORMALP.		// comment 2018/03/29 SCD2 K.Sato [Eagle][RQ-722]用紙種別追加（厚紙1を2種類に分割）対応.
	s_pNOT91to105gm2_W128H018A,		// 91～105g/㎡.		PNLE_PAPERTYPE_WEIGHT_NORMALP.
};
#else // DEF_A4_Cus.
// 用紙重量文字列テーブル.
TEXT_ID puc_AryNewPaperWeight[] = {
// 今後用紙種類に増減する事を踏まえ、参照用indexを定数定義した.
	s_Space2,						// スペース.		PNLE_PAPERTYPE_WEIGHT_NONE.
#ifdef DEF_LCD_7INCH
//comment 2019/11/18 [HCLT-Thangachamy] [HeliosMLK] [機器構成変更_No.11] Removing not required message ID. 
#ifndef DEF_HeliosMLKCus
	s_pNOT52to59gm2_W060H040A,		// 50～59g/㎡(仮).	PNLE_PAPERTYPE_WEIGHT_THIN.
#endif //#ifndef DEF_HeliosMLKCus
// Comment 2019/07/26 [HCLT_Paul] [IT6_HeliosMLK] [ Seigyu.10 ] Adding the HeliosMLK compiler switch.
//#if defined(DEF_ZEUSS_CUS) || defined(DEF_HeliosCus)
//comment 2019/11/18 [HCLT-Thangachamy] [HeliosMLK] [機器構成変更_No.11] Removing HeliosMLK support and creating new array for HeliosMLK.
//#if defined(DEF_ZEUSS_CUS) || defined(DEF_HeliosMLKCus)
#if defined(DEF_ZEUSS_CUS)
	s_pNOT60to90gm2_W128H018A,		// 60～90g/㎡.		PNLE_PAPERTYPE_WEIGHT_NORMAL.
	s_pNOT91To120gm2_W128H018A,		// 91～120g/㎡.		PNLE_PAPERTYPE_WEIGHT_THICK1.
	s_pNOT121To157gm2_W128H018A,	// 121～157g/㎡.	PNLE_PAPERTYPE_WEIGHT_THICK1P.
	s_pNOT158To209gm2_W128H018A,	// 158～209g/㎡.	PNLE_PAPERTYPE_WEIGHT_THICK2.
	s_pNOT210To256gm2_W128H018A,	// 210～256g/㎡.	PNLE_PAPERTYPE_WEIGHT_THICK3.
	s_pNOT257To300gm2_W128H018A,	// 257～300g/㎡.	PNLE_PAPERTYPE_WEIGHT_THICK4.
//comment 2019/11/18 [HCLT-Thangachamy] [HeliosMLK] [機器構成変更_No.11] creating new entried for HeliosMLK model.	
#elif defined(DEF_HeliosMLKCus) //DEF_HeliosMLKCus
	s_pNOT60to90gm2_W128H018A,		// 60～90g/㎡.		PNLE_PAPERTYPE_WEIGHT_NORMAL.
	s_pNOT60to90gm2_W128H018A,		// 60～90g/㎡.		PNLE_PAPERTYPE_WEIGHT_NORMAL. // SpecialPaper, LetterHead, Color Paper.
	s_pNOT91To120gm2_W128H018A,		// 91～120g/㎡.		PNLE_PAPERTYPE_WEIGHT_THICK1.
	s_pNOT121To157gm2_W128H018A,	// 121～157g/㎡.	PNLE_PAPERTYPE_WEIGHT_THICK1P.
	s_pNOT158To209gm2_W128H018A,	// 158～209g/㎡.	PNLE_PAPERTYPE_WEIGHT_THICK2.
	s_pNOT210To256gm2_W128H018A,	// 210～256g/㎡.	PNLE_PAPERTYPE_WEIGHT_THICK3.
#else	// !DEF_ZEUSS_CUS.
//Comment 2018/11/29 HCLT_Sarada [IT6_DenebMLK][制御構成変更_No.45] changing paper basic weights.
#ifndef DEF_DenebMLKCus
	s_pNOT60to90gm2_W128H018A,		// 60～90g/㎡.		PNLE_PAPERTYPE_WEIGHT_NORMAL.
	s_pNOT91To120gm2_W128H018A,		// 91～120g/㎡.		PNLE_PAPERTYPE_WEIGHT_THICK1.
#else
//	s_EXP64_90gm2,					// 64～90g/㎡.		PNLE_PAPERTYPE_WEIGHT_NORMAL.
	s_pNOT64to90gm2_W128H018A,
	s_pNOT106to120gm2_W128H018A,	// 106～120g/㎡.	PNLE_PAPERTYPE_WEIGHT_THICK1.
#endif //DEF_DenebMLKCus.
	s_pNOT121To157gm2_W128H018A,	// 121～157g/㎡.	PNLE_PAPERTYPE_WEIGHT_THICK1P.
	s_pNOT158To209gm2_W128H018A,	// 158～209g/㎡.	PNLE_PAPERTYPE_WEIGHT_THICK2.
#ifdef DEF_Zeus5BkCus
	s_pNOT210to220gm2_W128H018A,	// 210～220g/㎡.	PNLE_PAPERTYPE_WEIGHT_THICK3.
#else // DEF_Zeus5BkCus.
	s_pNOT210To256gm2_W060H040B,	// 210～256g/㎡.	PNLE_PAPERTYPE_WEIGHT_THICK3.
#endif // DEF_Zeus5BkCus.
	s_pNOT257To300gm2_W128H018A,	// 257～300g/㎡.	PNLE_PAPERTYPE_WEIGHT_THICK4.
#endif	// DEF_ZEUSS_CUS.
//Comment 2018/07/29 HCLT_Catherine for DenebMLK [Issue ID:1050339] adding plain paper+.
//comment 2019/11/18 [HCLT-Thangachamy] [HeliosMLK] [機器構成変更_No.11] Removing plainpaper+ related weight.
#ifndef DEF_HeliosMLKCus
	s_pNOT91to105gm2_W128H018A,		// 91～105g/㎡.		PNLE_PAPERTYPE_WEIGHT_NORMALP.
#endif //#ifndef DEF_HeliosMLKCus
#else // DEF_LCD_7INCH.
//comment 2019/11/18 [HCLT-Thangachamy] [HeliosMLK] [機器構成変更_No.11] Change weight of Thick1 as 91-120g.m2 for Heliosmlk 9inch.
#ifndef DEF_HeliosMLKCus
#if defined(DEF_EaglePanel_Low)	//Eagle_Low
	// TBD - EagleBK
	s_pNOT52to59gm2_W128H018A,		// 52～59g/㎡(仮).	PNLE_PAPERTYPE_WEIGHT_THIN.	
	s_pNOT60to90gm2_W128H018A,		// 60～90g/㎡.		PNLE_PAPERTYPE_WEIGHT_NORMAL.
// comment 2018/04/17 SCD2-Sky鳫林 [Eagle][RQ-722]用紙種類追加（厚紙１を２種類に分割）対応.
//	s_pNOT106to120gm2_W064H033A,	// 106～120g/㎡.	PNLE_PAPERTYPE_WEIGHT_THICK1.		// comment 2018/03/29 SCD2 K.Sato [Eagle][RQ-722]用紙種別追加（厚紙1を2種類に分割）対応.
	s_pNOT106to120gm2_W128H018A,	// 106～120g/㎡.	PNLE_PAPERTYPE_WEIGHT_THICK1.
	s_pNOT121To157gm2_W128H018A,	// 121～157g/㎡.	PNLE_PAPERTYPE_WEIGHT_THICK1P.
	s_pNOT158To209gm2_W128H018A,	// 158～209g/㎡.	PNLE_PAPERTYPE_WEIGHT_THICK2.
	s_pNOT210To256gm2_W128H018A,	// 210～256g/㎡.	PNLE_PAPERTYPE_WEIGHT_THICK3.
	s_pNOT257To300gm2_W128H018A,	// 257～300g/㎡.	PNLE_PAPERTYPE_WEIGHT_THICK4.
#else	// DEF_EaglePanel_Low.
	s_pNOT52to59gm2_W128H018A,		// 52～59g/㎡(仮).	PNLE_PAPERTYPE_WEIGHT_THIN.	
	s_pNOT60to90gm2_W128H018A, 		// 60～90g/㎡.		PNLE_PAPERTYPE_WEIGHT_NORMAL.
// comment 2018/04/17 SCD2-Sky鳫林 [Eagle][RQ-722]用紙種類追加（厚紙１を２種類に分割）対応.
//	s_pNOT106to120gm2_W064H033A, 	// 106～120g/㎡.	PNLE_PAPERTYPE_WEIGHT_THICK1.		// comment 2018/03/29 SCD2 K.Sato [Eagle][RQ-722]用紙種別追加（厚紙1を2種類に分割）対応.
	s_pNOT106to120gm2_W128H018A, 	// 106～120g/㎡.	PNLE_PAPERTYPE_WEIGHT_THICK1.
	s_pNOT121To157gm2_W128H018A,	// 121～157g/㎡.	PNLE_PAPERTYPE_WEIGHT_THICK1P.
	s_pNOT158To209gm2_W128H018A,	// 158～209g/㎡.	PNLE_PAPERTYPE_WEIGHT_THICK2.
#endif	// DEF_EaglePanel_Low.
	s_pNOT210To256gm2_W128H018A,	// 210～256g/㎡.	PNLE_PAPERTYPE_WEIGHT_THICK3.
	s_pNOT257To300gm2_W128H018A,	// 257～300g/㎡.	PNLE_PAPERTYPE_WEIGHT_THICK4【CQ.346520】第一OSW 酒井(健).
// comment 2018/04/17 SCD2-Sky鳫林 [Eagle][RQ-722]用紙種類追加（厚紙１を２種類に分割）対応.
//	s_pNOT91to105gm2_W064H033A,		// 91～105g/㎡.		PNLE_PAPERTYPE_WEIGHT_NORMALP.		// comment 2018/03/29 SCD2 K.Sato [Eagle][RQ-722]用紙種別追加（厚紙1を2種類に分割）対応.
	s_pNOT91to105gm2_W128H018A,		// 91～105g/㎡.		PNLE_PAPERTYPE_WEIGHT_NORMALP.
//comment 2019/11/18 [HCLT-Thangachamy] [HeliosMLK] [機器構成変更_No.11] Add seperate paper weight entries for 9-inch HeliosMLK.
#else //#ifndef DEF_HeliosMLKCus
	s_pNOT60to90gm2_W128H018A,		// 60～90g/㎡.		PNLE_PAPERTYPE_WEIGHT_NORMAL.
	s_pNOT60to90gm2_W128H018A,		// 60～90g/㎡.		PNLE_PAPERTYPE_WEIGHT_NORMAL. // SpecialPaper, LetterHead, Color Paper.
	s_pNOT91To120gm2_W128H018A,		// 91～120g/㎡.		PNLE_PAPERTYPE_WEIGHT_THICK1.
	s_pNOT121To157gm2_W128H018A,	// 121～157g/㎡.	PNLE_PAPERTYPE_WEIGHT_THICK1P.
	s_pNOT158To209gm2_W128H018A,	// 158～209g/㎡.	PNLE_PAPERTYPE_WEIGHT_THICK2.
	s_pNOT210To256gm2_W128H018A,	// 210～256g/㎡.	PNLE_PAPERTYPE_WEIGHT_THICK3.
#endif //#ifndef DEF_HeliosMLKCus
#endif // DEF_LCD_7INCH.
	};
#endif // DEF_A4_Cus.

// comment [2015/06/01] [HCLT-Pankaj][MinervaSBK][機器構成変更_No.23] created a new table for paper wait for PI tray.
TEXT_ID puc_PIPaperWeight[] = {
//comment 2019/04/26 HCLT-Sivateja [IT6_EagleH][OP_BTS-4773 ] Adding PaperTypeWidth for PI tray in EagleHI models.
	s_pNOT52to59gm2_W128H018A,		// 52～59g/㎡(仮).	PNLE_PAPERTYPE_WEIGHT_THIN.	
//comment 2019/09/12 HCLT-Shalini] [IT6_EagleH][OP_BTS-13399 ] Changing the Paper Weight.
////comment 2019/04/26 HCLT-Sivateja [IT6_EagleH][OP_BTS-4773 ] Adding PaperTypeWidth for PI tray in EagleHI models.
//#if defined ( DEF_EagleCus ) || defined ( DEF_EaglePanel_Hi )
//	s_pNOTITM71to90gm2_W128H018A,	// 71～90g/㎡(仮).	PNLE_PAPERTYPE_WEIGHT_Plain.	
//#else	
//	s_pNOT60to90gm2_W128H018A,		// 60～90g/㎡.		PNLE_PAPERTYPE_WEIGHT_NORMAL.
//#endif	
	s_pNOT60to90gm2_W128H018A,		// 60～90g/㎡.		PNLE_PAPERTYPE_WEIGHT_NORMAL.
// comment 2018/04/17 SCD2-Sky鳫林 [Eagle][RQ-722]用紙種類追加（厚紙１を２種類に分割）対応.
//	s_pNOT91to105gm2_W064H033A,		// 91～105g/㎡.     PNLE_PAPERTYPE_WEIGHT_NORMALP.		// comment 2018/03/29 SCD2 K.Sato [Eagle][RQ-722]用紙種別追加（厚紙1を2種類に分割）対応.
//	s_pNOT106to120gm2_W064H033A,	// 106～120g/㎡.	PNLE_PAPERTYPE_WEIGHT_THICK1.
	s_pNOT91to105gm2_W128H018A,		// 91～105g/㎡.     PNLE_PAPERTYPE_WEIGHT_NORMALP.
	s_pNOT106to120gm2_W128H018A,	// 106～120g/㎡.	PNLE_PAPERTYPE_WEIGHT_THICK1.
	s_pNOT121To157gm2_W128H018A,	// 121～157g/㎡.	PNLE_PAPERTYPE_WEIGHT_THICK1P.
	s_pNOT158To209gm2_W128H018A,	// 158～209g/㎡.	PNLE_PAPERTYPE_WEIGHT_THICK2.
//comment 2019/04/26 HCLT-Sivateja [IT6_EagleH][OP_BTS-4773 ] Adding PaperTypeWidth for PI tray in EagleHI models.	
	s_pNOT210To256gm2_W128H018A,	// 210～256g/㎡.	PNLE_PAPERTYPE_WEIGHT_THICK3.
	s_pNOT257To300gm2_W128H018A,	// 257～300g/㎡.	PNLE_PAPERTYPE_WEIGHT_THICK4.
	s_Space2,						// スペース.		PNLE_PAPERTYPE_WEIGHT_NONE.
};

static const TYPS_PaperKindInfo st_PaperKindNewTblPI[] = {
//comment 2019/04/26 HCLT-Sivateja [IT6_EagleH][OP_BTS-4773 ] Adding PaperType for PI tray in EagleHI models.	
//comment 2019/11/18 [HCLT-Thangachamy] [HeliosMLK] [機器構成変更_No.11] Removing thin paper.
#ifndef DEF_HeliosMLKCus
	{ TYPE_PaperTypeThin1,			False,			s_pKEYThinPaper_W144H028A,					CNK_ThinPaper,					PNLD_PAPERTYPEIND_THIN,				PNLE_PAPERTYPE_WEIGHT_THIN,		},	// 薄紙	
#endif //#ifndef DEF_HeliosMLKCus
	{ TYPE_PaperTypeNormal,			False,			s_pKEYPlainPaper_W144H028A,					CNK_NomalPaper,					PNLD_PAPERTYPEIND_NORMAL,			PNLE_PAPERTYPE_WEIGHT_NORMAL,	},	// 普通紙.
//comment 2019/11/18 [HCLT-Thangachamy] [HeliosMLK] [機器構成変更_No.11] Removing PlainPaper+.
#ifndef DEF_HeliosMLKCus
	{ TYPE_PaperTypeNormalPlus,		False,			s_pKEYPlainPaperP_W144H028A,				CNK_NomalPaperPlus,				PNLD_PAPERTYPEIND_NORMALP,			PNLE_PAPERTYPE_WEIGHT_NORMALP,	},	// 普通紙+.		// comment 2018/03/29 SCD2 K.Sato [Eagle][RQ-722]用紙種別追加（厚紙1を2種類に分割）対応.
#endif //#ifndef DEF_HeliosMLKCus
	{ TYPE_PaperTypeThick1,			False,			s_pKEYThickPaper1_W144H028A,				CNK_SpecialPaperCard1,			PNLD_PAPERTYPEIND_THICK1,			PNLE_PAPERTYPE_WEIGHT_THICK1,	},	// 厚紙1.
	{ TYPE_PaperTypeThick1Plus,		False,			s_pKEYThickPaper1P_W144H028A,				CNK_SpecialPaperCard1P,			PNLD_PAPERTYPEIND_THICK1P,			PNLE_PAPERTYPE_WEIGHT_THICK1P,	},	// 厚紙1+.
	{ TYPE_PaperTypeThick2,			False,			s_pKEYThickPaper2_W144H028A,				CNK_SpecialPaperCard2,			PNLD_PAPERTYPEIND_THICK2,			PNLE_PAPERTYPE_WEIGHT_THICK2,	},	// 厚紙2.
//comment 2019/04/26 HCLT-Sivateja [IT6_EagleH][OP_BTS-4773 ] Adding PaperType for PI tray in EagleHI models.
	{ TYPE_PaperTypeThick3,			False,			s_pKEYThickPaper3_W144H028A,				CNK_SpecialPaperCard3,			PNLD_PAPERTYPEIND_THICK3,			PNLE_PAPERTYPE_WEIGHT_THICK3,	},	// 厚紙3.
//comment 2019/11/18 [HCLT-Thangachamy] [HeliosMLK] [機器構成変更_No.11] Removing Thick4 and Label which are not applicable for heliosmlk.
#ifndef DEF_HeliosMLKCus
	{ TYPE_PaperTypeThick4,			False,			s_pKEYThickPaper4_W144H028A,				CNK_SpecialPaperCard4,			PNLD_PAPERTYPEIND_THICK4,			PNLE_PAPERTYPE_WEIGHT_THICK4,	},		// 厚紙4.
	{ TYPE_PaperTypeLabel,			False,			s_pKEYLabelPaper1_W144H028A ,				CNK_LabelPaper,					PNLD_PAPERTYPEIND_LABEL,			PNLE_PAPERTYPE_WEIGHT_NONE,		},	// ラベル紙.
#endif //#ifndef DEF_HeliosMLKCus
};
const uchar uc_SizePaperKindNewTblPI = ( uchar )( sizeof( st_PaperKindNewTblPI ) / sizeof( TYPS_PaperKindInfo ) );

static const struct Manual2ndSidedPaperTypeSelectTable_CLCD {
	TYPE_PaperType		e_PaperType;		// 用紙種類.
	EP_Combination		e_Combination;		// 機能種類
	EP_Combination		e_Combination2;		// 機能種類
} manual2ndSidedPaperTypeSelectTable_CLCD[] = {
	{ TYPE_PaperTypeNormal,		CNK_NomalPaper,			CNK_NomalPaperRemove		},	// 普通紙.
//comment 2019/11/18 [HCLT-Thangachamy] [HeliosMLK] [機器構成変更_No.11] Removing dispay of not supported paper types.
#ifndef DEF_HeliosMLKCus
	{ TYPE_PaperTypeThin1,		CNK_ThinPaper,			CNK_ThinPaperRemove			},	// 薄紙.		// comment MinervaZeus Sky小尾 2011/04/18 [UV-01038_1：No.01].両面2面目パラメータについては組禁時に対応.
	{ TYPE_PaperTypeNormalPlus,	CNK_NomalPaperPlus,		CNK_NomalPaperPlusRemove	},	// 普通紙+.		// comment 2018/03/29 SCD2 K.Sato [Eagle][RQ-722]用紙種別追加（厚紙1を2種類に分割）対応.
#endif //#ifndef DEF_HeliosMLKCus
	{ TYPE_PaperTypeThick1,		CNK_SpecialPaperCard1,	CNK_SpecialPaperCard1Remove	},	// 厚紙1.
	{ TYPE_PaperTypeThick1Plus,	CNK_SpecialPaperCard1P,	CNK_SpecialPaperCard1PRemove},	// 厚紙1+.
	{ TYPE_PaperTypeThick2,		CNK_SpecialPaperCard2,	CNK_SpecialPaperCard2Remove	},	// 厚紙2.
	{ TYPE_PaperTypeThick3,		CNK_SpecialPaperCard3,	CNK_SpecialPaperCard3Remove	},	// 厚紙3.
//comment 2019/11/18 [HCLT-Thangachamy] [HeliosMLK] [機器構成変更_No.11] Removing dispay of not supported paper types.
#ifndef DEF_HeliosMLKCus
	{ TYPE_PaperTypeThick4,		CNK_SpecialPaperCard4,	CNK_SpecialPaperCard4Remove	},	// 厚紙4.
#endif //#ifndef DEF_HeliosMLKCus
	{ TYPE_PaperRecycle1,		CNK_NomalPaper,			CNK_NomalPaperRemove		},	// 再生紙.		// comment MinervaZeus ADD Sky上西 2011/05/05 [UV-01038][機器1001].
	{ TYPE_PaperTypeUser1,		CNK_NomalPaper,			CNK_NomalPaperRemove		},	// ﾕｰｻﾞｰ紙1.
// comment 2018/05/08 SCD2-4G吉田 [EagleL][RQ-722] 用紙種類追加(普通紙+).
//comment 2019/11/18 [HCLT-Thangachamy] [HeliosMLK] [機器構成変更_No.11] UserPaper combination is changed from PlainPaper+ to PlainPaper for heliosmlk.
#ifdef DEF_HeliosMLKCus
	{ TYPE_PaperTypeUser2,		CNK_NomalPaper,			CNK_NomalPaperRemove		},	// ﾕｰｻﾞｰ紙2.
#else //#ifdef DEF_HeliosMLKCus
	{ TYPE_PaperTypeUser2,		CNK_NomalPaperPlus,		CNK_NomalPaperPlusRemove	},	// ﾕｰｻﾞｰ紙2.
#endif //#ifdef DEF_HeliosMLKCus
	{ TYPE_PaperTypeUser3,		CNK_SpecialPaperCard1,	CNK_SpecialPaperCard1Remove	},	// ﾕｰｻﾞｰ紙3.
	{ TYPE_PaperTypeUser4,		CNK_SpecialPaperCard1P,	CNK_SpecialPaperCard1PRemove},	// ﾕｰｻﾞｰ紙4.
	{ TYPE_PaperTypeUser5,		CNK_SpecialPaperCard2,	CNK_SpecialPaperCard2Remove	},	// ﾕｰｻﾞｰ紙5.
	{ TYPE_PaperTypeUser6,		CNK_SpecialPaperCard3,	CNK_SpecialPaperCard3Remove	},	// ﾕｰｻﾞｰ紙6.
#ifndef DEF_A4_Cus  // comment 2019/06/28 SCD4-Sky西村 [EagleZ+][機器構成変更No.1] ユーザー紙C 追加対応.
//comment 2019/11/18 [HCLT-Thangachamy] [HeliosMLK] [機器構成変更_No.11] Removing dispay of not supported paper types.
#ifndef DEF_HeliosMLKCus
	{ TYPE_PaperTypeUser7,		CNK_SpecialPaperCard2,	CNK_SpecialPaperCard2Remove	},	// ﾕｰｻﾞｰ紙C.
#endif //#ifndef DEF_HeliosMLKCus
#endif // DEF_A4_Cus
};




// A3A4FUM 用紙種類設定 t-okada 2015/01/19 Start 選択トレイの設定変更.
#ifndef DEF_A4_Cus
// A3A4FUM 用紙種類設定 t-okada 2015/01/19 End.
// 手差し以外用紙種類ボタンインデックステーブル.
static const uchar uc_AryPaperTypeBtnStatusTbl[][2] = {
// Comment 2015/01/29 HCLT-Vinothini [Compilation Switch ID: 16] for HeliosL support.
//#ifdef DEF_Zeus5BkCus
// Comment 2019/07/26 [HCLT_Paul] [IT6_HeliosMLK] [ Seigyu.10 ] Adding the HeliosMLK compiler switch.
//comment 2019/11/18 [HCLT-Thangachamy] [HeliosMLK] [機器構成変更_No.11] Removing HELIOSMLK compiler switch.
//#if defined(DEF_Zeus5BkCus) || defined(DEF_HeliosMLKCus)
#if defined(DEF_Zeus5BkCus)
	{ BTNID_KEY_PAPERKINDP1_1,	0	},	// 普通紙.
	{ BTNID_KEY_PAPERKINDP1_2,	1	},	// 片面専用用紙.
	{ BTNID_KEY_PAPERKINDP1_3,	static_cast<uchar>( -1 )	},	// 薄紙は無効キー.
	{ BTNID_KEY_PAPERKINDP1_4,	2	},	// 特殊紙.
//Comment 2018/07/29 HCLT_Catherine for DenebMLK [Issue ID:1050339] adding plain paper+.
//	{ BTNID_KEY_PAPERKINDP1_5,	3	},	// 厚紙1.
//	{ BTNID_KEY_PAPERKINDP1_6,	4	},	// 厚紙1+.
//	{ BTNID_KEY_PAPERKINDP1_7,	5	},	// 厚紙2.
//	{ BTNID_KEY_PAPERKINDP1_8,	6	},	// 厚紙3.
//	{ BTNID_KEY_PAPERKINDP1_9,	7	},	// 再生紙.
//	{ BTNID_KEY_PAPERKINDP1_10,	8	},	// レターヘッド紙.	// comment MinervaZeus 7→8 Sky小尾 2011/04/15 [UV-01038_1：No.01].
//	{ BTNID_KEY_PAPERKINDP2_1,	9	},	// 色紙.	// comment MinervaZeus 8→9 Sky小尾 2011/04/15 [UV-01038_1：No.01].
//	{ BTNID_KEY_PAPERKINDP2_2,	16	},	// ユーザー紙1.
//	{ BTNID_KEY_PAPERKINDP2_3,	17	},	// ユーザー紙2.
//	{ BTNID_KEY_PAPERKINDP2_4,	18	},	// ユーザー紙3.
//	{ BTNID_KEY_PAPERKINDP2_5,	19	},	// ユーザー紙4.
//	{ BTNID_KEY_PAPERKINDP2_6,	20	},	// ユーザー紙5.
//	{ BTNID_KEY_PAPERKINDP2_7,	21	},	// ユーザー紙6.
	{ BTNID_KEY_PAPERKINDP1_5,	3   },	// 普通紙+.		
	{ BTNID_KEY_PAPERKINDP1_6,	4	},	// 厚紙1.
	{ BTNID_KEY_PAPERKINDP1_7,	5	},	// 厚紙1+.
	{ BTNID_KEY_PAPERKINDP1_8,	6	},	// 厚紙2.
	{ BTNID_KEY_PAPERKINDP1_9,	7	},	// 厚紙3.
	{ BTNID_KEY_PAPERKINDP1_10,	8	},	// 再生紙.
	{ BTNID_KEY_PAPERKINDP1_11,	9	},	// レターヘッド紙.	// comment MinervaZeus 7→8 Sky小尾 2011/04/15 [UV-01038_1：No.01].
	{ BTNID_KEY_PAPERKINDP2_1,	10	},	// 色紙.	// comment MinervaZeus 8→9 Sky小尾 2011/04/15 [UV-01038_1：No.01].
	{ BTNID_KEY_PAPERKINDP2_2,	17	},	// ユーザー紙1.
	{ BTNID_KEY_PAPERKINDP2_3,	18	},	// ユーザー紙2.
	{ BTNID_KEY_PAPERKINDP2_4,	19	},	// ユーザー紙3.
	{ BTNID_KEY_PAPERKINDP2_5,	20	},	// ユーザー紙4.
	{ BTNID_KEY_PAPERKINDP2_6,	21	},	// ユーザー紙5.
	{ BTNID_KEY_PAPERKINDP2_7,	22	},	// ユーザー紙6.
	{ BTNID_KEY_PAPERKINDP2_8,	static_cast<uchar>( -1 )	},	// なし.
	{ BTNID_KEY_PAPERKINDP2_9,	static_cast<uchar>( -1 )	},	// なし.
	{ BTNID_KEY_PAPERKINDP2_10,	static_cast<uchar>( -1 )	},	// なし.
//comment 2019/11/18 [HCLT-Thangachamy] [HeliosMLK] [機器構成変更_No.11] Creating separate entries of button ID mapping for HELIOSMLK.
#elif defined (DEF_HeliosMLKCus) // (DEF_Zeus5BkCus)
	{ BTNID_KEY_PAPERKINDP1_1,	0	},	// 普通紙.
	{ BTNID_KEY_PAPERKINDP1_2,	1	},	// 片面専用用紙.
	{ BTNID_KEY_PAPERKINDP1_3,	2	},	// 特殊紙.
	{ BTNID_KEY_PAPERKINDP1_4,	3	},	// 厚紙1.
	{ BTNID_KEY_PAPERKINDP1_5,	4   },	// 厚紙1+.
	{ BTNID_KEY_PAPERKINDP1_6,	5	},	// 厚紙2.
	{ BTNID_KEY_PAPERKINDP1_7,	6	},	// 厚紙3.
	{ BTNID_KEY_PAPERKINDP1_8,	7	},	// 再生紙.
	{ BTNID_KEY_PAPERKINDP1_9,	8	},	// レターヘッド紙.
	{ BTNID_KEY_PAPERKINDP1_10,	9	},	// 色紙.
	{ BTNID_KEY_PAPERKINDP1_11,	static_cast<uchar>( -1 )	},
	{ BTNID_KEY_PAPERKINDP2_1,	22	},	// ユーザー紙1.
	{ BTNID_KEY_PAPERKINDP2_2,	23	},	// ユーザー紙2.
	{ BTNID_KEY_PAPERKINDP2_3,	24	},	// ユーザー紙3.
	{ BTNID_KEY_PAPERKINDP2_4,	25	},	// ユーザー紙4.
	{ BTNID_KEY_PAPERKINDP2_5,	26	},	// ユーザー紙5.
	{ BTNID_KEY_PAPERKINDP2_6,	27	},	// ユーザー紙6.
	{ BTNID_KEY_PAPERKINDP2_7,	static_cast<uchar>( -1 )	},	// なし.
	{ BTNID_KEY_PAPERKINDP2_8,	static_cast<uchar>( -1 )	},	// なし.
	{ BTNID_KEY_PAPERKINDP2_9,	static_cast<uchar>( -1 )	},	// なし.
	{ BTNID_KEY_PAPERKINDP2_10,	static_cast<uchar>( -1 )	},	// なし.
//#else //#if defined(DEF_Zeus5BkCus) || defined(DEF_HeliosCus).
#else //#if defined(DEF_Zeus5BkCus)
	{ BTNID_KEY_PAPERKINDP1_1,	0	},	// 普通紙.
	{ BTNID_KEY_PAPERKINDP1_2,	1	},	// 片面専用用紙.
	{ BTNID_KEY_PAPERKINDP1_3,	2	},	// 薄紙.	// comment MinervaZeus なし→薄紙 Sky小尾 2011/04/15 [UV-01038_1：No.01].
	{ BTNID_KEY_PAPERKINDP1_4,	3	},	// 特殊紙.
	{ BTNID_KEY_PAPERKINDP1_5,	4	},	// 普通紙+.		// comment 2018/03/29 SCD2 K.Sato [Eagle][RQ-722]用紙種別追加（厚紙1を2種類に分割）対応.
	{ BTNID_KEY_PAPERKINDP1_6,	5	},	// 厚紙1.
	{ BTNID_KEY_PAPERKINDP1_7,	6	},	// 厚紙1+.
	{ BTNID_KEY_PAPERKINDP1_8,	7	},	// 厚紙2.
	{ BTNID_KEY_PAPERKINDP1_9,	8	},	// 厚紙3.
	{ BTNID_KEY_PAPERKINDP1_10,	9	},	// 再生紙.
	{ BTNID_KEY_PAPERKINDP1_11,	10	},	// レターヘッド紙.	// comment MinervaZeus 7→8 Sky小尾 2011/04/15 [UV-01038_1：No.01].
	{ BTNID_KEY_PAPERKINDP2_1,	11	},	// 色紙.	// comment MinervaZeus 8→9 Sky小尾 2011/04/15 [UV-01038_1：No.01].
// comment 2018/05/08 SCD2-4G吉田 [EagleL][RQ-722] 用紙種類追加(普通紙+).
	{ BTNID_KEY_PAPERKINDP2_2,	20	},	// ユーザー紙1.
	{ BTNID_KEY_PAPERKINDP2_3,	21	},	// ユーザー紙2.
	{ BTNID_KEY_PAPERKINDP2_4,	22	},	// ユーザー紙3.
	{ BTNID_KEY_PAPERKINDP2_5,	23	},	// ユーザー紙4.
	{ BTNID_KEY_PAPERKINDP2_6,	24	},	// ユーザー紙5.
	{ BTNID_KEY_PAPERKINDP2_7,	25	},	// ユーザー紙6.
	{ BTNID_KEY_PAPERKINDP2_8,	static_cast<uchar>( -1 )	},	// なし.
	{ BTNID_KEY_PAPERKINDP2_9,	static_cast<uchar>( -1 )	},	// なし.
	{ BTNID_KEY_PAPERKINDP2_10,	static_cast<uchar>( -1 )	},	// なし.
#endif //#if defined(DEF_Zeus5BkCus) || defined(DEF_HeliosCus).
};
//Comment [IT6-Eagle] [1044008] HCLT-Deeparaj 2018/02/20 Changing the button index for Colored Paper.
//Comment [IT6-Eagle] 機器構成変更No.2 HCLT-Deeparaj 2018/02/20 Adding envelope key for tray 1.
#ifndef DEF_A4_Cus
static const uchar uc_AryPaperTypeBtnStatusTblTry1[][2] = {
//comment 2019/11/18 [HCLT-Thangachamy] [HeliosMLK] [機器構成変更_No.11] Creating separate entries with HELIOSMLK supported paper types for Tray1.
#ifndef DEF_HeliosMLKCus
	{ BTNID_KEY_PAPERKINDP1_1,	0	},	// 普通紙.
	{ BTNID_KEY_PAPERKINDP1_2,	1	},	// 片面専用用紙.
	{ BTNID_KEY_PAPERKINDP1_3,	2	},	// 薄紙.
	{ BTNID_KEY_PAPERKINDP1_4,	3	},	// 特殊紙.
	{ BTNID_KEY_PAPERKINDP1_5,	4	},	// 普通紙+.
	{ BTNID_KEY_PAPERKINDP1_6,	5	},	// 厚紙1.
	{ BTNID_KEY_PAPERKINDP1_7,	6	},	// 厚紙1+.
	{ BTNID_KEY_PAPERKINDP1_8,	7	},	// 厚紙2.
	{ BTNID_KEY_PAPERKINDP1_9,	8	},	// 厚紙3.
	{ BTNID_KEY_PAPERKINDP1_10,	9	},	// 封筒.
// comment 2018/04/18 SCD2-Sky中野 [EagleL][RQ-722]普通紙+ - 追加対応.
	{ BTNID_KEY_PAPERKINDP1_11,	10	},	// 再生紙.
//	{ BTNID_KEY_PAPERKINDP2_1,	10	},	// ユーザー紙1.
////	{ BTNID_KEY_PAPERKINDP2_2,	18	},	// ユーザー紙1.
//	{ BTNID_KEY_PAPERKINDP2_2,	11	},	// ユーザー紙1.
//	{ BTNID_KEY_PAPERKINDP2_3,	19	},	// ユーザー紙1.
//	{ BTNID_KEY_PAPERKINDP2_4,	20	},	// ユーザー紙2.
//	{ BTNID_KEY_PAPERKINDP2_5,	21	},	// ユーザー紙3.
//	{ BTNID_KEY_PAPERKINDP2_6,	22	},	// ユーザー紙4.
//	{ BTNID_KEY_PAPERKINDP2_7,	23	},	// ユーザー紙5.
//	{ BTNID_KEY_PAPERKINDP2_8,	24  },	// ユーザー紙6.
	{ BTNID_KEY_PAPERKINDP2_1,	11	},	// レターヘッド紙.
// Comment 2021/06/01 【FPT-LinhNV46】 【MinervaSSBK】 【機器構成変更_No.21-20】 Delete paper type Envelope of Tray1 for MinervaSSBK.
#ifdef DEF_MinervaSSBKCus
	{ BTNID_KEY_PAPERKINDP2_2,	19	},	// 色紙.
	{ BTNID_KEY_PAPERKINDP2_3,	20	},	// ユーザー紙1.
	{ BTNID_KEY_PAPERKINDP2_4,	21	},	// ユーザー紙2.
	{ BTNID_KEY_PAPERKINDP2_5,	22	},	// ユーザー紙3.
	{ BTNID_KEY_PAPERKINDP2_6,	23	},	// ユーザー紙4.
	{ BTNID_KEY_PAPERKINDP2_7,	24	},	// ユーザー紙5.
	{ BTNID_KEY_PAPERKINDP2_8,	static_cast<uchar>( -1 )	},	// ユーザー紙6.
	{ BTNID_KEY_PAPERKINDP2_9,	static_cast<uchar>( -1 )	},	// なし.
	{ BTNID_KEY_PAPERKINDP2_10,	static_cast<uchar>( -1 )	},	// なし.
#else //DEF_MinervaSSBKCus.
	{ BTNID_KEY_PAPERKINDP2_2,	12	},	// 色紙.
	{ BTNID_KEY_PAPERKINDP2_3,	20	},	// ユーザー紙1.
	{ BTNID_KEY_PAPERKINDP2_4,	21	},	// ユーザー紙2.
	{ BTNID_KEY_PAPERKINDP2_5,	22	},	// ユーザー紙3.
	{ BTNID_KEY_PAPERKINDP2_6,	23	},	// ユーザー紙4.
	{ BTNID_KEY_PAPERKINDP2_7,	24	},	// ユーザー紙5.
	{ BTNID_KEY_PAPERKINDP2_8,	25  },	// ユーザー紙6.
	{ BTNID_KEY_PAPERKINDP2_9,	static_cast<uchar>( -1 )	},	// なし.
	{ BTNID_KEY_PAPERKINDP2_10,	static_cast<uchar>( -1 )	},	// なし.
#endif //DEF_MinervaSSBKCus.
#else //#ifndef DEF_HeliosMLKCus
	{ BTNID_KEY_PAPERKINDP1_1,	0	},	// 普通紙.
	{ BTNID_KEY_PAPERKINDP1_2,	1	},	// 片面専用用紙.
	{ BTNID_KEY_PAPERKINDP1_3,	2	},	// 特殊紙.
	{ BTNID_KEY_PAPERKINDP1_4,	3	},	// 厚紙1.
	{ BTNID_KEY_PAPERKINDP1_5,	4	},	// 厚紙1+..
	{ BTNID_KEY_PAPERKINDP1_6,	5	},	// 厚紙2.
	{ BTNID_KEY_PAPERKINDP1_7,	6	},	// 厚紙3.
	{ BTNID_KEY_PAPERKINDP1_8,	7	},	// 再生紙.
	{ BTNID_KEY_PAPERKINDP1_9,	8	},	// レターヘッド紙.
	{ BTNID_KEY_PAPERKINDP1_10,	9	},	// 色紙.
	{ BTNID_KEY_PAPERKINDP1_11,	static_cast<uchar>( -1 )	},	// 色紙.
	{ BTNID_KEY_PAPERKINDP2_1,	22	},	// ユーザー紙1.
	{ BTNID_KEY_PAPERKINDP2_2,	23	},	// ユーザー紙2.
	{ BTNID_KEY_PAPERKINDP2_3,	24	},	// ユーザー紙3.
	{ BTNID_KEY_PAPERKINDP2_4,	25	},	// ユーザー紙4.
	{ BTNID_KEY_PAPERKINDP2_5,	26	},	// ユーザー紙5.
	{ BTNID_KEY_PAPERKINDP2_6,	27	},	// ユーザー紙6.
	{ BTNID_KEY_PAPERKINDP2_7,	static_cast<uchar>( -1 )	},	// なし.
	{ BTNID_KEY_PAPERKINDP2_8,	static_cast<uchar>( -1 )  	},	// なし.
	{ BTNID_KEY_PAPERKINDP2_9,	static_cast<uchar>( -1 )	},	// なし.
	{ BTNID_KEY_PAPERKINDP2_10,	static_cast<uchar>( -1 )	},	// なし.
#endif //#ifndef DEF_HeliosMLKCus
};
#endif	//DEF_A4_Cus.
// A3A4FUM 用紙種類設定 t-okada 2015/01/19 Start 選択トレイの設定変更.
#else // DEF_A4_Cus.

// 手差し以外用紙種類ボタンインデックステーブル.
static const uchar uc_AryPaperTypeBtnStatusTbl[PNLD_PaperTypeTrayMax][PNLD_AryPaperTypeBtnStatusTbl_MaxNum][2] = {
	// トレイ1.
	{	{ BTNID_KEY_PAPERKINDP1_1,	0	},	// 普通紙.
		{ BTNID_KEY_PAPERKINDP1_2,	1	},	// 片面専用用紙.
		{ BTNID_KEY_PAPERKINDP1_3,	2	},	// 特殊紙.
		{ BTNID_KEY_PAPERKINDP1_4,	3	},	// 普通紙+.		// comment 2018/03/29 SCD2 K.Sato [Eagle][RQ-722]用紙種別追加（厚紙1を2種類に分割）対応.
		{ BTNID_KEY_PAPERKINDP1_5,	4	},	// 厚紙1.
// [KronosS] Sky荻堂 2016/09/26 管理No.22 用紙種類カスタマイズ.
#ifdef DEF_KRONOSS_Cus		
		{ BTNID_KEY_PAPERKINDP1_5,	4	},	// 再生紙.	
		{ BTNID_KEY_PAPERKINDP1_6,	5	},	// レターヘッド紙.
		{ BTNID_KEY_PAPERKINDP1_7,	6	},	// 色紙.
		{ BTNID_KEY_PAPERKINDP1_8,	7	},	// ラベル紙.
		{ BTNID_KEY_PAPERKINDP1_9,	8	},	// ユーザー紙1.
		{ BTNID_KEY_PAPERKINDP1_10,	9	},	// ユーザー紙2.
		{ BTNID_KEY_PAPERKINDP2_1,	10	},	// ユーザー紙3.
#else
		{ BTNID_KEY_PAPERKINDP1_6,	5	},	// 厚紙1+.
		{ BTNID_KEY_PAPERKINDP1_7,	6	},	// 厚紙2.
		{ BTNID_KEY_PAPERKINDP1_8,	7	},	// 封筒.
		{ BTNID_KEY_PAPERKINDP1_9,	8	},	// 再生紙.	
		{ BTNID_KEY_PAPERKINDP1_10,	9	},	// レターヘッド紙.
		{ BTNID_KEY_PAPERKINDP1_11,	10	},	// 色紙.
		{ BTNID_KEY_PAPERKINDP2_1,	11	},	// 光沢紙1.
		{ BTNID_KEY_PAPERKINDP2_2,	12	},	// 光沢紙1+.
		{ BTNID_KEY_PAPERKINDP2_3,	13	},	// 光沢紙2.
		{ BTNID_KEY_PAPERKINDP2_4,	14	},	// はがき.
		{ BTNID_KEY_PAPERKINDP2_5,	15	},	// ラベル紙.
		{ BTNID_KEY_PAPERKINDP2_6,	16	},	// ユーザー紙1.
		{ BTNID_KEY_PAPERKINDP2_7,	17	},	// ユーザー紙2.
		{ BTNID_KEY_PAPERKINDP2_8,	18	},	// ユーザー紙3.
		{ BTNID_KEY_PAPERKINDP2_9,	19	},	// ユーザー紙4.
		{ BTNID_KEY_PAPERKINDP2_10,	20	},	// ユーザー紙5.
#endif
	},
	// トレイ2.
	{	{ BTNID_KEY_PAPERKINDP1_1,	0	},	// 普通紙.
		{ BTNID_KEY_PAPERKINDP1_2,	1	},	// 片面専用用紙.
		{ BTNID_KEY_PAPERKINDP1_3,	2	},	// 特殊紙.
		{ BTNID_KEY_PAPERKINDP1_4,	3	},	// 普通紙+.		// comment 2018/03/29 SCD2 K.Sato [Eagle][RQ-722]用紙種別追加（厚紙1を2種類に分割）対応.
// [KronosS] Sky荻堂 2016/09/26 管理No.22 用紙種類カスタマイズ.
#ifdef DEF_KRONOSS_Cus		
		{ BTNID_KEY_PAPERKINDP1_4,	3	},	// 厚紙1.
		{ BTNID_KEY_PAPERKINDP1_5,	4	},	// 再生紙.	
// comment 2019/03/22 SCD2-Sky鳥居 [特ROM][FQ-OPSD-1800408-OFP218]KronosS MFP / トレイ２にレターヘッド設定ができてしまう対策.
//[KronosS] Sky川名 2016/12/21 課題番号:1007853 選択トレイ画面にレターヘッド紙キーが表示されない.
//		{ BTNID_KEY_PAPERKINDP1_6,	5	},	// レターヘッド紙.
		{ BTNID_KEY_PAPERKINDP1_6,	5	},	// 色紙.
		{ BTNID_KEY_PAPERKINDP1_7,	6	},	// ラベル紙.
		{ BTNID_KEY_PAPERKINDP1_8,	7	},	// ユーザー紙1.
		{ BTNID_KEY_PAPERKINDP1_9,	8	},	// ユーザー紙2.
		{ BTNID_KEY_PAPERKINDP1_10,	9	},	// ユーザー紙3.
		{ BTNID_KEY_PAPERKINDP2_1,	255	},	// なし
#else
		{ BTNID_KEY_PAPERKINDP1_4,	4	},	// 再生紙.	
//		{ BTNID_KEY_PAPERKINDP1_5,	5	},	// レターヘッド紙.
// Comment 2023/09/07 FPT-ThinhND40 [IT6_2.3] [ 機器構成変更_No.55 ] Removal of prohibition on letterhead of additional cassettes.
		{ BTNID_KEY_PAPERKINDP1_5,	5	},	// レターヘッド紙.
		{ BTNID_KEY_PAPERKINDP1_6,	6	},	// 色紙.
		{ BTNID_KEY_PAPERKINDP1_7,	7	},	// ユーザー紙1.
		{ BTNID_KEY_PAPERKINDP1_8,	8	},	// ユーザー紙2.
		{ BTNID_KEY_PAPERKINDP1_9,	255	},	// なし.
		{ BTNID_KEY_PAPERKINDP1_10,	255	},	// なし.
		{ BTNID_KEY_PAPERKINDP2_1,	255	},	// なし.
		{ BTNID_KEY_PAPERKINDP2_2,	255	},	// なし.
		{ BTNID_KEY_PAPERKINDP2_3,	255	},	// なし.
		{ BTNID_KEY_PAPERKINDP2_4,	255	},	// なし.
		{ BTNID_KEY_PAPERKINDP2_5,	255	},	// なし.
		{ BTNID_KEY_PAPERKINDP2_6,	255	},	// なし.
		{ BTNID_KEY_PAPERKINDP2_7,	255	},	// なし.
		{ BTNID_KEY_PAPERKINDP2_8,	255	},	// なし.
		{ BTNID_KEY_PAPERKINDP2_9,	255	},	// なし.
		{ BTNID_KEY_PAPERKINDP2_10,	255	},	// なし.
#endif
	},
	// トレイ3.
	{	{ BTNID_KEY_PAPERKINDP1_1,	0	},	// 普通紙.
		{ BTNID_KEY_PAPERKINDP1_2,	1	},	// 片面専用用紙.
		{ BTNID_KEY_PAPERKINDP1_3,	2	},	// 特殊紙.
		{ BTNID_KEY_PAPERKINDP1_4,	3	},	// 普通紙+.		// comment 2018/03/29 SCD2 K.Sato [Eagle][RQ-722]用紙種別追加（厚紙1を2種類に分割）対応.
// [KronosS] Sky荻堂 2016/09/26 管理No.22 用紙種類カスタマイズ.
#ifdef DEF_KRONOSS_Cus
		{ BTNID_KEY_PAPERKINDP1_4,	3	},	// 厚紙1.
		{ BTNID_KEY_PAPERKINDP1_5,	4	},	// 再生紙.	
// comment 2019/03/22 SCD2-Sky鳥居 [特ROM][FQ-OPSD-1800408-OFP218]KronosS MFP / トレイ２にレターヘッド設定ができてしまう対策.
//[KronosS] Sky川名 2016/12/21 課題番号:1007853 選択トレイ画面にレターヘッド紙キーが表示されない.
//		{ BTNID_KEY_PAPERKINDP1_6,	5	},	// レターヘッド紙.
		{ BTNID_KEY_PAPERKINDP1_6,	5	},	// 色紙.
		{ BTNID_KEY_PAPERKINDP1_7,	6	},	// ラベル紙.
		{ BTNID_KEY_PAPERKINDP1_8,	7	},	// ユーザー紙1.
		{ BTNID_KEY_PAPERKINDP1_9,	8	},	// ユーザー紙2.
		{ BTNID_KEY_PAPERKINDP1_10,	9	},	// ユーザー紙3.
		{ BTNID_KEY_PAPERKINDP2_1,	255	},	// なし.
#else
		{ BTNID_KEY_PAPERKINDP1_4,	4	},	// 再生紙.	
// Comment 2023/09/07 FPT-ThinhND40 [IT6_2.3] [ 機器構成変更_No.55 ] Removal of prohibition on letterhead of additional cassettes.
		{ BTNID_KEY_PAPERKINDP1_5,	5	},	// レターヘッド紙.
		{ BTNID_KEY_PAPERKINDP1_6,	6	},	// 色紙.
		{ BTNID_KEY_PAPERKINDP1_7,	7	},	// ユーザー紙1.
		{ BTNID_KEY_PAPERKINDP1_8,	8	},	// ユーザー紙2.
		{ BTNID_KEY_PAPERKINDP1_9,	255	},	// なし.
		{ BTNID_KEY_PAPERKINDP1_10,	255	},	// なし.
		{ BTNID_KEY_PAPERKINDP2_1,	255	},	// なし.
		{ BTNID_KEY_PAPERKINDP2_2,	255	},	// なし.
		{ BTNID_KEY_PAPERKINDP2_3,	255	},	// なし.
		{ BTNID_KEY_PAPERKINDP2_4,	255	},	// なし.
		{ BTNID_KEY_PAPERKINDP2_5,	255	},	// なし.
		{ BTNID_KEY_PAPERKINDP2_6,	255	},	// なし.
		{ BTNID_KEY_PAPERKINDP2_7,	255	},	// なし.
		{ BTNID_KEY_PAPERKINDP2_8,	255	},	// なし.
		{ BTNID_KEY_PAPERKINDP2_9,	255	},	// なし.
		{ BTNID_KEY_PAPERKINDP2_10,	255	},	// なし.
#endif
	},
// [KronosS] Sky川名 2016/10/25 管理No21_44 各トレイ毎の表示用紙サイズをKronosS仕様にあわせる.
#ifdef DEF_KRONOSS_Cus
	// トレイ4.
	{	{ BTNID_KEY_PAPERKINDP1_1,	0	},	// 普通紙.
		{ BTNID_KEY_PAPERKINDP1_2,	1	},	// 片面専用用紙.
		{ BTNID_KEY_PAPERKINDP1_3,	2	},	// 特殊紙.
		{ BTNID_KEY_PAPERKINDP1_4,	3	},	// 厚紙1.
		{ BTNID_KEY_PAPERKINDP1_5,	4	},	// 再生紙.	
// comment 2019/03/22 SCD2-Sky鳥居 [特ROM][FQ-OPSD-1800408-OFP218]KronosS MFP / トレイ２にレターヘッド設定ができてしまう対策.
//[KronosS] Sky川名 2016/12/21 課題番号:1007853 選択トレイ画面にレターヘッド紙キーが表示されない.
//		{ BTNID_KEY_PAPERKINDP1_6,	5	},	// レターヘッド紙.
		{ BTNID_KEY_PAPERKINDP1_6,	5	},	// 色紙.
		{ BTNID_KEY_PAPERKINDP1_7,	6	},	// ラベル紙.
		{ BTNID_KEY_PAPERKINDP1_8,	7	},	// ユーザー紙1.
		{ BTNID_KEY_PAPERKINDP1_9,	8	},	// ユーザー紙2.
		{ BTNID_KEY_PAPERKINDP1_10,	9	},	// ユーザー紙3.
		{ BTNID_KEY_PAPERKINDP2_1,	255	},	// なし.
	},
//comment 2023/07/25 FPT.LONGTH32 [IT6_2.3] 機器構成変更_No.75 Add tray 4 for Sparrow.
#else
	// トレイ4.
	{	{ BTNID_KEY_PAPERKINDP1_1,	0	},	// 普通紙.
		{ BTNID_KEY_PAPERKINDP1_2,	1	},	// 片面専用用紙.
		{ BTNID_KEY_PAPERKINDP1_3,	2	},	// 特殊紙.
		{ BTNID_KEY_PAPERKINDP1_4,	3	},	// 普通紙+.
		{ BTNID_KEY_PAPERKINDP1_4,	4	},	// 再生紙.	
// Comment 2023/09/07 FPT-ThinhND40 [IT6_2.3] [ 機器構成変更_No.55 ] Removal of prohibition on letterhead of additional cassettes.
		{ BTNID_KEY_PAPERKINDP1_5,	5	},	// レターヘッド紙.
		{ BTNID_KEY_PAPERKINDP1_6,	6	},	// 色紙.
		{ BTNID_KEY_PAPERKINDP1_7,	7	},	// ユーザー紙1.
		{ BTNID_KEY_PAPERKINDP1_8,	8	},	// ユーザー紙2.
		{ BTNID_KEY_PAPERKINDP1_9,	255	},	// なし.
		{ BTNID_KEY_PAPERKINDP1_10,	255	},	// なし.
		{ BTNID_KEY_PAPERKINDP2_1,	255	},	// なし.
		{ BTNID_KEY_PAPERKINDP2_2,	255	},	// なし.
		{ BTNID_KEY_PAPERKINDP2_3,	255	},	// なし.
		{ BTNID_KEY_PAPERKINDP2_4,	255	},	// なし.
		{ BTNID_KEY_PAPERKINDP2_5,	255	},	// なし.
		{ BTNID_KEY_PAPERKINDP2_6,	255	},	// なし.
		{ BTNID_KEY_PAPERKINDP2_7,	255	},	// なし.
		{ BTNID_KEY_PAPERKINDP2_8,	255	},	// なし.
		{ BTNID_KEY_PAPERKINDP2_9,	255	},	// なし.
		{ BTNID_KEY_PAPERKINDP2_10,	255	},	// なし.
	},
#endif // DEF_KRONOSS_Cus.
};
#endif // DEF_A4_Cus.
// A3A4FUM 用紙種類設定 t-okada 2015/01/19 End.


static const uchar uc_BtPaperTypeTbl[][2] = {
// Comment 2015/01/29 HCLT-Vinothini [Compilation Switch ID: 19] for HeliosL support.
//#ifdef DEF_Zeus5BkCus
// Comment 2019/07/26 [HCLT_Paul] [IT6_HeliosMLK] [ Seigyu.10 ] Adding the HeliosMLK compiler switch.
//comment 2019/11/18 [HCLT-Thangachamy] [HeliosMLK] [機器構成変更_No.11] Removing HELIOSMLK compiler switch.
//#if defined(DEF_Zeus5BkCus) || defined(DEF_HeliosMLKCus)
#if defined(DEF_Zeus5BkCus)
	{ TYPE_PaperTypeNormal,		0   },
	{ TYPE_PaperTypeNotDuplex1,	1   },
	{ TYPE_PaperTypeSpecial1,	2	},
	{ TYPE_PaperTypeThick1,		3	},
	{ TYPE_PaperTypeThick1Plus,	4	},
	{ TYPE_PaperTypeThick2,		5	},
	{ TYPE_PaperTypeThick3,		6	},
	{ TYPE_PaperTypeLetterHead,	7	},
	{ TYPE_PaperTypeColor,		8	},
	{ TYPE_PaperRecycle1,		9	},	// comment MinervaZeus ADD Sky上西 2011/05/05 [UV-01038][機器1001].
	{ TYPE_PaperTypeUser1,		10	},
	{ TYPE_PaperTypeUser2,		11	},
	{ TYPE_PaperTypeUser3,		12	},
	{ TYPE_PaperTypeUser4,		13	},
	{ TYPE_PaperTypeUser5,		14	},
	{ TYPE_PaperTypeUser6,		15	},
//comment 2019/11/18 [HCLT-Thangachamy] [HeliosMLK] [機器構成変更_No.11]Creating separate entries with HELIOSMLK supported paper types for Tray2~4.
#elif defined DEF_HeliosMLKCus //#if defined(DEF_Zeus5BkCus)
	{ TYPE_PaperTypeNormal,		0   },
	{ TYPE_PaperTypeNotDuplex1,	1   },
	{ TYPE_PaperTypeSpecial1,	2	},
	{ TYPE_PaperTypeThick1,		3	},
	{ TYPE_PaperTypeThick1Plus,	4	},
	{ TYPE_PaperTypeThick2,		5	},
	{ TYPE_PaperTypeThick3,		6	},
	{ TYPE_PaperRecycle1,		7	},
	{ TYPE_PaperTypeLetterHead,	8	},
	{ TYPE_PaperTypeColor,		9	},
	{ TYPE_PaperTypeUser1,		10	},
	{ TYPE_PaperTypeUser2,		11	},
	{ TYPE_PaperTypeUser3,		12	},
	{ TYPE_PaperTypeUser4,		13	},
	{ TYPE_PaperTypeUser5,		14	},
	{ TYPE_PaperTypeUser6,		15	},
//#else // #if defined(DEF_Zeus5BkCus) || defined(DEF_HeliosCus).
#else // #if defined(DEF_Zeus5BkCus)
	{ TYPE_PaperTypeNormal,		0   },
	{ TYPE_PaperTypeNotDuplex1,	1   },
	{ TYPE_PaperTypeThin1,		2	},	// comment MinervaZeus 薄紙追加 Sky小尾 2011/04/15 [UV-01038_1：No.01].
	{ TYPE_PaperTypeSpecial1,	3   },
	{ TYPE_PaperTypeThick1,		4	},
	{ TYPE_PaperTypeThick1Plus,	5	},
	{ TYPE_PaperTypeThick2,		6	},
	{ TYPE_PaperTypeThick3,		7	},
	{ TYPE_PaperRecycle1,		8	},	// comment MinervaZeus ADD Sky上西 2011/05/05 [UV-01038][機器1001].
	{ TYPE_PaperTypeLetterHead,	9	},
	{ TYPE_PaperTypeColor,		10	},
	{ TYPE_PaperTypeUser1,		11	},
	{ TYPE_PaperTypeUser2,		12	},
	{ TYPE_PaperTypeUser3,		13	},
	{ TYPE_PaperTypeUser4,		14	},
	{ TYPE_PaperTypeUser5,		15	},
	{ TYPE_PaperTypeUser6,		16	},
#endif // #if defined(DEF_Zeus5BkCus) || defined(DEF_HeliosCus).
};
//Comment [IT6-Eagle] 機器構成変更No.2 HCLT-Deeparaj 2018/02/20 Adding envelope key for tray 1.
#ifndef DEF_A4_Cus
static const uchar uc_BtPaperTypeTblTry1[][2] = {
	{ TYPE_PaperTypeNormal,		0   },
	{ TYPE_PaperTypeNotDuplex1,	1   },
//comment 2019/11/18 [HCLT-Thangachamy] [HeliosMLK] [機器構成変更_No.11] Adding compiler switch.
#ifndef DEF_HeliosMLKCus
	{ TYPE_PaperTypeThin1,		2	},
	{ TYPE_PaperTypeSpecial1,	3   },
	{ TYPE_PaperTypeThick1,		4	},
	{ TYPE_PaperTypeThick1Plus,	5	},
	{ TYPE_PaperTypeThick2,		6	},
	{ TYPE_PaperTypeThick3,		7	},
// Comment 2021/06/01 【FPT-LinhNV46】 【MinervaSSBK】 【機器構成変更_No.21-20】 Delete paper type Envelope of Tray1 for MinervaSSBK.
#ifdef DEF_MinervaSSBKCus
	{ TYPE_PaperRecycle1,		8	},
	{ TYPE_PaperTypeLetterHead,	9	},
	{ TYPE_PaperTypeColor,		10	},
	{ TYPE_PaperTypeUser1,		11	},
	{ TYPE_PaperTypeUser2,		12	},
	{ TYPE_PaperTypeUser3,		13	},
	{ TYPE_PaperTypeUser4,		14	},
	{ TYPE_PaperTypeUser5,		15	},
	{ TYPE_PaperTypeUser6,		16	},
#else //DEF_MinervaSSBKCus.
	{TYPE_PaperTypeEnvelope	,	8	},
	{ TYPE_PaperRecycle1,		9	},	// comment MinervaZeus ADD Sky上西 2011/05/05 [UV-01038][機器1001].
	{ TYPE_PaperTypeLetterHead,	10	},
	{ TYPE_PaperTypeColor,		11	},
	{ TYPE_PaperTypeUser1,		12	},
	{ TYPE_PaperTypeUser2,		13	},
	{ TYPE_PaperTypeUser3,		14	},
	{ TYPE_PaperTypeUser4,		15	},
	{ TYPE_PaperTypeUser5,		16	},
	{ TYPE_PaperTypeUser6,		17	},
#endif //DEF_MinervaSSBKCus.
//comment 2019/11/18 [HCLT-Thangachamy] [HeliosMLK] [機器構成変更_No.11] Separate entries added with HELIOSMLK supported paper types.
#else //#ifndef DEF_HeliosMLKCus
	{ TYPE_PaperTypeSpecial1,	2   },
	{ TYPE_PaperTypeThick1,		3	},
	{ TYPE_PaperTypeThick1Plus,	4	},
	{ TYPE_PaperTypeThick2,		5	},
	{ TYPE_PaperTypeThick3,		6	},
	{ TYPE_PaperRecycle1,		7	},
	{ TYPE_PaperTypeLetterHead,	8	},
	{ TYPE_PaperTypeColor,		9	},
	{ TYPE_PaperTypeUser1,		10	},
	{ TYPE_PaperTypeUser2,		11	},
	{ TYPE_PaperTypeUser3,		12	},
	{ TYPE_PaperTypeUser4,		13	},
	{ TYPE_PaperTypeUser5,		14	},
	{ TYPE_PaperTypeUser6,		15	},
#endif //#ifndef DEF_HeliosMLKCus
};
#endif	//DEF_A4_Cus.
const uchar uc_BtPaperTypeTblSize = ( uchar )( sizeof( uc_BtPaperTypeTbl ) / sizeof( uc_BtPaperTypeTbl[0] ) );
//Comment [IT6-Eagle] 機器構成変更No.2 HCLT-Deeparaj 2018/02/20 Get the size of the table.
#ifndef DEF_A4_Cus
const uchar uc_BtPaperTypeTblSizeTry1 = ( uchar )( sizeof( uc_BtPaperTypeTblTry1 ) / sizeof( uc_BtPaperTypeTblTry1[0] ) );
#endif	//DEF_A4_Cus.

// A3A4FUM 用紙種類設定 t-okada 2015/01/19 Start 選択トレイの設定変更.
#ifndef DEF_A4_Cus
// A3A4FUM 用紙種類設定 t-okada 2015/01/19 End.
// 手差し用紙種類ボタンインデック番号.
// [i][0]→本テーブル検索用の用紙種類, [i][1]→TYPS_PaperKindInfoテーブル参照用index.
static const uchar uc_IconPaperType[][2] = {
	{ TYPE_PaperTypeNormal,		PNLD_PAPERTYPEIND_NORMAL	},
	{ TYPE_PaperTypeNotDuplex1,	PNLD_PAPERTYPEIND_SINGLSIDE	},
//comment 2019/11/18 [HCLT-Thangachamy] [HeliosMLK] [機器構成変更_No.11] Heliosmlk not supported papertype icon entries are restricted.
#ifndef DEF_HeliosMLKCus
	{ TYPE_PaperTypeThin1,		PNLD_PAPERTYPEIND_THIN		}, // comment MinervaZeus Sky小尾 2011/04/15 [UV-01038_1：No.01].
#endif //#ifndef DEF_HeliosMLKCus
	{ TYPE_PaperTypeSpecial1,	PNLD_PAPERTYPEIND_SPECIAL	},
//comment 2019/11/18 [HCLT-Thangachamy] [HeliosMLK] [機器構成変更_No.11] Heliosmlk not supported papertype icon entries are restricted.
#ifndef DEF_HeliosMLKCus
	{ TYPE_PaperTypeNormalPlus,	PNLD_PAPERTYPEIND_NORMALP	}, // comment 2018/03/29 SCD2 K.Sato [Eagle][RQ-722]用紙種別追加（厚紙1を2種類に分割）対応.
#endif //#ifndef DEF_HeliosMLKCus
	{ TYPE_PaperTypeThick1,		PNLD_PAPERTYPEIND_THICK1	},
	{ TYPE_PaperTypeThick1Plus,	PNLD_PAPERTYPEIND_THICK1P	},
	{ TYPE_PaperTypeThick2,		PNLD_PAPERTYPEIND_THICK2	},
	{ TYPE_PaperTypeThick3,		PNLD_PAPERTYPEIND_THICK3	},
//comment 2019/11/18 [HCLT-Thangachamy] [HeliosMLK] [機器構成変更_No.11] Heliosmlk not supported papertype icon entries are restricted.
#ifndef DEF_HeliosMLKCus
	{ TYPE_PaperTypeThick4,		PNLD_PAPERTYPEIND_THICK4	},
	{ TYPE_PaperTypeOHP,		PNLD_PAPERTYPEIND_OHP		},
#endif //#ifndef DEF_HeliosMLKCus
	{ TYPE_PaperTypeLetterHead,	PNLD_PAPERTYPEIND_LTRHEAD	},
	{ TYPE_PaperTypeColor,		PNLD_PAPERTYPEIND_COLOR		},
//comment 2019/11/18 [HCLT-Thangachamy] [HeliosMLK] [機器構成変更_No.11] Heliosmlk not supported papertype icon entries are restricted.
#ifndef DEF_HeliosMLKCus
	{ TYPE_PaperTypeEnvelope,	PNLD_PAPERTYPEIND_ENVELOPE	},
	{ TYPE_PaperTypeIndex,		PNLD_PAPERTYPEIND_INDEX		}, // comment MinervaZeus Sky金田 2010/11/25 [UV-00995].
#endif //#ifndef DEF_HeliosMLKCus
	{ TYPE_PaperRecycle1,		PNLD_PAPERTYPEIND_RECYCLE	}, // comment MinervaZeus ADD Sky上西 2011/05/05 [UV-01038][機器1001].
	{ TYPE_PaperTypeUser1,		PNLD_PAPERTYPEIND_USER1		},
	{ TYPE_PaperTypeUser2,		PNLD_PAPERTYPEIND_USER2		},
	{ TYPE_PaperTypeUser3,		PNLD_PAPERTYPEIND_USER3		},
	{ TYPE_PaperTypeUser4,		PNLD_PAPERTYPEIND_USER4		},
	{ TYPE_PaperTypeUser5,		PNLD_PAPERTYPEIND_USER5		},
	{ TYPE_PaperTypeUser6,		PNLD_PAPERTYPEIND_USER6		},
// comment 2019/06/28 SCD4-Sky西村 [EagleZ+][機器構成変更No.1] ユーザー紙C 追加対応.
//comment 2019/11/18 [HCLT-Thangachamy] [HeliosMLK] [機器構成変更_No.11] Heliosmlk not supported papertype icon entries are restricted.
#ifndef DEF_HeliosMLKCus
	{ TYPE_PaperTypeUser7,		PNLD_PAPERTYPEIND_USERC		},
#endif //#ifndef DEF_HeliosMLKCus
};
// A3A4FUM 用紙種類設定 t-okada 2015/01/19 Start 選択トレイの設定変更.
#else // DEF_A4_Cus.

// 手差し用紙種類ボタンインデック番号.
// [i][0]→本テーブル検索用の用紙種類, [i][1]→TYPS_PaperKindInfoテーブル参照用index.
static const uchar uc_IconPaperType[][2] = {
	{ TYPE_PaperTypeNormal,		PNLD_PAPERTYPEIND_NORMAL	},
	{ TYPE_PaperTypeNotDuplex1,	PNLD_PAPERTYPEIND_SINGLSIDE	},
	{ TYPE_PaperTypeSpecial1,	PNLD_PAPERTYPEIND_SPECIAL	},
	{ TYPE_PaperTypeNormalPlus,	PNLD_PAPERTYPEIND_NORMALP	}, // comment 2018/03/29 SCD2 K.Sato [Eagle][RQ-722]用紙種別追加（厚紙1を2種類に分割）対応.
	{ TYPE_PaperTypeThick1,		PNLD_PAPERTYPEIND_THICK1	},
	{ TYPE_PaperTypeThick1Plus,	PNLD_PAPERTYPEIND_THICK1P	},
	{ TYPE_PaperTypeThick2,		PNLD_PAPERTYPEIND_THICK2	},
	{ TYPE_PaperTypeLetterHead,	PNLD_PAPERTYPEIND_LTRHEAD	},
	{ TYPE_PaperTypeColor,		PNLD_PAPERTYPEIND_COLOR		},
	{ TYPE_PaperTypeEnvelope,	PNLD_PAPERTYPEIND_ENVELOPE	},
	{ TYPE_PaperRecycle1,		PNLD_PAPERTYPEIND_RECYCLE	},
	{ TYPE_PaperTypeUser1,		PNLD_PAPERTYPEIND_USER1		},
	{ TYPE_PaperTypeUser2,		PNLD_PAPERTYPEIND_USER2		},
	{ TYPE_PaperTypeUser3,		PNLD_PAPERTYPEIND_USER3		},
	{ TYPE_PaperTypeUser4,		PNLD_PAPERTYPEIND_USER4		},
	{ TYPE_PaperTypeUser5,		PNLD_PAPERTYPEIND_USER5		},
	{ TYPE_PaperTypeGloss1,		PNLD_PAPERTYPEIND_GLOSS1	},
	{ TYPE_PaperTypeGloss1Plus,	PNLD_PAPERTYPEIND_GLOSS1P	},
	{ TYPE_PaperTypeGloss2,		PNLD_PAPERTYPEIND_GLOSS2	},
	{ TYPE_PaperPostCard,		PNLD_PAPERTYPEIND_POSTCARD	},
	{ TYPE_PaperTypeLabel,		PNLD_PAPERTYPEIND_LABEL		},
};
#endif // DEF_A4_Cus.
// A3A4FUM 用紙種類設定 t-okada 2015/01/19 End.

const uchar uc_IconPaperTypeSize = ( uchar )( sizeof( uc_IconPaperType ) / sizeof( uc_IconPaperType[0] ) );

// 手差し用紙種類ボタンインデックス番号.
// [i][0]→本テーブル検索用の用紙種類, [i][1]→TYPS_PaperKindInfoテーブル参照用index.
static const uchar uc_IconPaperTypeDobule[][2] = {
// A3A4FUM 用紙種類設定 t-okada 2015/01/19 Start 選択トレイの設定変更.
#ifndef DEF_A4_Cus
// A3A4FUM 用紙種類設定 t-okada 2015/01/19 End.
	{ TYPE_PaperTypeNormal,		PNLD_PAPERTYPEIND_NORMALRMV		},
//comment 2019/11/18 [HCLT-Thangachamy] [HeliosMLK] [機器構成変更_No.11] Heliosmlk not supported papertype icon entries are restricted.
#ifndef DEF_HeliosMLKCus
	{ TYPE_PaperTypeNormalPlus,	PNLD_PAPERTYPEIND_NORMALPRMV	}, // comment 2018/03/29 SCD2 K.Sato [Eagle][RQ-722]用紙種別追加（厚紙1を2種類に分割）対応.
	{ TYPE_PaperTypeThin1,		PNLD_PAPERTYPEIND_THINRMV		}, // comment MinervaZeus Sky小尾 2011/04/15 [UV-01038_1：No.01].
#endif //#ifndef DEF_HeliosMLKCus
	{ TYPE_PaperTypeThick1,		PNLD_PAPERTYPEIND_THICK1RMV		},
	{ TYPE_PaperTypeThick1Plus,	PNLD_PAPERTYPEIND_THICK1PRMV	},
	{ TYPE_PaperTypeThick2,		PNLD_PAPERTYPEIND_THICK2RMV		},
	{ TYPE_PaperTypeThick3,		PNLD_PAPERTYPEIND_THICK3RMV		},
//comment 2019/11/18 [HCLT-Thangachamy] [HeliosMLK] [機器構成変更_No.11] Heliosmlk not supported papertype icon entries are restricted.
#ifndef DEF_HeliosMLKCus
	{ TYPE_PaperTypeThick4,		PNLD_PAPERTYPEIND_THICK4RMV		},
#endif //#ifndef DEF_HeliosMLKCus
	{ TYPE_PaperRecycle1,		PNLD_PAPERTYPEIND_NORMALRMV		}, // comment MinervaZeus ADD Sky上西 2011/05/05 [UV-01038][機器1001].
	{ TYPE_PaperTypeUser1,		PNLD_PAPERTYPEIND_USER1RMV		},
	{ TYPE_PaperTypeUser2,		PNLD_PAPERTYPEIND_USER2RMV		},
	{ TYPE_PaperTypeUser3,		PNLD_PAPERTYPEIND_USER3RMV		},
	{ TYPE_PaperTypeUser4,		PNLD_PAPERTYPEIND_USER4RMV		},
	{ TYPE_PaperTypeUser5,		PNLD_PAPERTYPEIND_USER5RMV		},
	{ TYPE_PaperTypeUser6,		PNLD_PAPERTYPEIND_USER6RMV		},
// comment 2019/06/28 SCD4-Sky西村 [EagleZ+][機器構成変更No.1] ユーザー紙C 追加対応.
//comment 2019/11/18 [HCLT-Thangachamy] [HeliosMLK] [機器構成変更_No.11] Heliosmlk not supported papertype icon entries are restricted.
#ifndef DEF_HeliosMLKCus
	{ TYPE_PaperTypeUser7,		PNLD_PAPERTYPEIND_USERCRMV		},
#endif //#ifndef DEF_HeliosMLKCus
// A3A4FUM 用紙種類設定 t-okada 2015/01/19 Start 選択トレイの設定変更.
#else // DEF_A4_Cus.
	{ TYPE_PaperTypeNormal,		PNLD_PAPERTYPEIND_NORMALRMV		},
	{ TYPE_PaperTypeNormalPlus,	PNLD_PAPERTYPEIND_NORMALPRMV	}, // comment 2018/03/29 SCD2 K.Sato [Eagle][RQ-722]用紙種別追加（厚紙1を2種類に分割）対応.
	{ TYPE_PaperTypeThick1,		PNLD_PAPERTYPEIND_THICK1RMV		},
	{ TYPE_PaperTypeThick1Plus,	PNLD_PAPERTYPEIND_THICK1PRMV	},
	{ TYPE_PaperTypeThick2,		PNLD_PAPERTYPEIND_THICK2RMV		},
	{ TYPE_PaperRecycle1,		PNLD_PAPERTYPEIND_NORMALRMV		}, // comment MinervaZeus ADD Sky上西 2011/05/05 [UV-01038][機器1001].
	{ TYPE_PaperTypeUser1,		PNLD_PAPERTYPEIND_USER1RMV		},
	{ TYPE_PaperTypeUser2,		PNLD_PAPERTYPEIND_USER2RMV		},
	{ TYPE_PaperTypeUser3,		PNLD_PAPERTYPEIND_USER3RMV		},
	{ TYPE_PaperTypeUser4,		PNLD_PAPERTYPEIND_USER4RMV		},
	{ TYPE_PaperTypeUser5,		PNLD_PAPERTYPEIND_USER5RMV		},
#endif // DEF_A4_Cus.
// A3A4FUM 用紙種類設定 t-okada 2015/01/19 End.
};
const uchar uc_IconPaperTypeDobuleSize = ( uchar )( sizeof( uc_IconPaperTypeDobule ) / sizeof( uc_IconPaperTypeDobule[0] ) );

// comment 2019/01/22 SCD2-4G Sky武井 OP_BTS-2216【開発提起】【Nランク】【RQ-720】紙種自動検知がONで用紙種類が未確定の時に、未確定であることがわかるアイコンを用紙種類の設定値表示箇所に表示する。.
static struct PNLS_PaperTypUndecidedIconIndex {
	const uchar		uc_IconIndex;			// 用紙種類アイコン.
	const uchar		uc_UndecidedIconIndex;	// 用紙種類未確定アイコン.
} tbl_PaperTypUndecidedIconIndex[] = {
#ifndef DEF_A4_Cus
	{ PNLD_PAPERTYPEIND_NORMAL,		PNLD_PAPERTYPEIND_UNDECIDED_NORMAL		},	// 普通紙.
	{ PNLD_PAPERTYPEIND_SINGLSIDE,	PNLD_PAPERTYPEIND_UNDECIDED_SINGLSIDE	},	// 片面専用用紙.
//comment 2019/11/18 [HCLT-Thangachamy] [HeliosMLK] [機器構成変更_No.11] Heliosmlk not supported papertype icon entries are restricted.
#ifndef DEF_HeliosMLKCus
	{ PNLD_PAPERTYPEIND_THIN,		PNLD_PAPERTYPEIND_UNDECIDED_THIN		},	// 薄紙.
#endif //#ifndef DEF_HeliosMLKCus
	{ PNLD_PAPERTYPEIND_SPECIAL,	PNLD_PAPERTYPEIND_UNDECIDED_SPECIAL		},	// 特殊紙.
	{ PNLD_PAPERTYPEIND_THICK1,		PNLD_PAPERTYPEIND_UNDECIDED_THICK1		},	// 厚紙1.
	{ PNLD_PAPERTYPEIND_THICK1P,	PNLD_PAPERTYPEIND_UNDECIDED_THICK1P		},	// 厚紙1+.
	{ PNLD_PAPERTYPEIND_THICK2,		PNLD_PAPERTYPEIND_UNDECIDED_THICK2		},	// 厚紙2.
	{ PNLD_PAPERTYPEIND_THICK3,		PNLD_PAPERTYPEIND_UNDECIDED_THICK3		},	// 厚紙3.
//comment 2019/11/18 [HCLT-Thangachamy] [HeliosMLK] [機器構成変更_No.11] Heliosmlk not supported papertype icon entries are restricted.
#ifndef DEF_HeliosMLKCus
	{ PNLD_PAPERTYPEIND_THICK4,		PNLD_PAPERTYPEIND_UNDECIDED_THICK4		},	// 厚紙4.
	{ PNLD_PAPERTYPEIND_OHP,		PNLD_PAPERTYPEIND_UNDECIDED_OHP			},	// OHPフィルム.
#endif //#ifndef DEF_HeliosMLKCus
	{ PNLD_PAPERTYPEIND_LTRHEAD,	PNLD_PAPERTYPEIND_UNDECIDED_LTRHEAD		},	// レターヘッド.
	{ PNLD_PAPERTYPEIND_COLOR,		PNLD_PAPERTYPEIND_UNDECIDED_COLOR		},	// 色紙.
//comment 2019/11/18 [HCLT-Thangachamy] [HeliosMLK] [機器構成変更_No.11] Heliosmlk not supported papertype icon entries are restricted.
#ifndef DEF_HeliosMLKCus
	{ PNLD_PAPERTYPEIND_ENVELOPE,	PNLD_PAPERTYPEIND_UNDECIDED_ENVELOPE	},	// 封筒.
	{ PNLD_PAPERTYPEIND_INDEX,		PNLD_PAPERTYPEIND_UNDECIDED_INDEX		},	// インデックス紙.
#endif //#ifndef DEF_HeliosMLKCus
	{ PNLD_PAPERTYPEIND_RECYCLE,	PNLD_PAPERTYPEIND_UNDECIDED_RECYCLE		},	// 再生紙.
	{ PNLD_PAPERTYPEIND_NORMALRMV,	PNLD_PAPERTYPEIND_UNDECIDED_NORMALRMV	},	// 普通紙(2面目).
//comment 2019/11/18 [HCLT-Thangachamy] [HeliosMLK] [機器構成変更_No.11] Heliosmlk not supported papertype icon entries are restricted.
#ifndef DEF_HeliosMLKCus
	{ PNLD_PAPERTYPEIND_THINRMV,	PNLD_PAPERTYPEIND_UNDECIDED_THINRMV		},	// 薄紙(2面目).
#endif //#ifndef DEF_HeliosMLKCus
	{ PNLD_PAPERTYPEIND_THICK1RMV,	PNLD_PAPERTYPEIND_UNDECIDED_THICK1RMV	},	// 厚紙1(2面目).
	{ PNLD_PAPERTYPEIND_THICK1PRMV,	PNLD_PAPERTYPEIND_UNDECIDED_THICK1PRMV	},	// 厚紙1+(2面目).
	{ PNLD_PAPERTYPEIND_THICK2RMV,	PNLD_PAPERTYPEIND_UNDECIDED_THICK2RMV	},	// 厚紙2(2面目).
	{ PNLD_PAPERTYPEIND_THICK3RMV,	PNLD_PAPERTYPEIND_UNDECIDED_THICK3RMV	},	// 厚紙3(2面目).
//comment 2019/11/18 [HCLT-Thangachamy] [HeliosMLK] [機器構成変更_No.11] Heliosmlk not supported papertype icon entries are restricted.
#ifndef DEF_HeliosMLKCus
	{ PNLD_PAPERTYPEIND_THICK4RMV,	PNLD_PAPERTYPEIND_UNDECIDED_THICK4RMV	},	// 厚紙4(2面目).
#endif //#ifndef DEF_HeliosMLKCus
	{ PNLD_PAPERTYPEIND_RECYCLERMV,	PNLD_PAPERTYPEIND_UNDECIDED_RECYCLERMV	},	// 再生紙(2面目).
	{ PNLD_PAPERTYPEIND_USER1,		PNLD_PAPERTYPEIND_UNDECIDED_USER1		},	// ユーザー紙1.
	{ PNLD_PAPERTYPEIND_USER2,		PNLD_PAPERTYPEIND_UNDECIDED_USER2		},	// ユーザー紙2.
	{ PNLD_PAPERTYPEIND_USER3,		PNLD_PAPERTYPEIND_UNDECIDED_USER3		},	// ユーザー紙3.
	{ PNLD_PAPERTYPEIND_USER4,		PNLD_PAPERTYPEIND_UNDECIDED_USER4		},	// ユーザー紙4.
	{ PNLD_PAPERTYPEIND_USER5,		PNLD_PAPERTYPEIND_UNDECIDED_USER5		},	// ユーザー紙5.
	{ PNLD_PAPERTYPEIND_USER6,		PNLD_PAPERTYPEIND_UNDECIDED_USER6		},	// ユーザー紙6.
	{ PNLD_PAPERTYPEIND_USER1RMV,	PNLD_PAPERTYPEIND_UNDECIDED_USER1RMV	},	// ユーザー紙1(2面目).
	{ PNLD_PAPERTYPEIND_USER2RMV,	PNLD_PAPERTYPEIND_UNDECIDED_USER2RMV	},	// ユーザー紙2(2面目).
	{ PNLD_PAPERTYPEIND_USER3RMV,	PNLD_PAPERTYPEIND_UNDECIDED_USER3RMV	},	// ユーザー紙3(2面目).
	{ PNLD_PAPERTYPEIND_USER4RMV,	PNLD_PAPERTYPEIND_UNDECIDED_USER4RMV	},	// ユーザー紙4(2面目).
	{ PNLD_PAPERTYPEIND_USER5RMV,	PNLD_PAPERTYPEIND_UNDECIDED_USER5RMV	},	// ユーザー紙5(2面目).
	{ PNLD_PAPERTYPEIND_USER6RMV,	PNLD_PAPERTYPEIND_UNDECIDED_USER6RMV	},	// ユーザー紙6(2面目).
//comment 2019/11/18 [HCLT-Thangachamy] [HeliosMLK] [機器構成変更_No.11] Heliosmlk not supported papertype icon entries are restricted.
#ifndef DEF_HeliosMLKCus
	{ PNLD_PAPERTYPEIND_NORMALP,	PNLD_PAPERTYPEIND_UNDECIDED_NORMALP		},	// 普通紙+.
	{ PNLD_PAPERTYPEIND_NORMALPRMV,	PNLD_PAPERTYPEIND_UNDECIDED_NORMALPRMV	},	// 普通紙+(2面目).
// comment 2019/06/28 SCD4-Sky西村 [EagleZ+][機器構成変更No.1] ユーザー紙C 追加対応.
	{ PNLD_PAPERTYPEIND_USERC,		PNLD_PAPERTYPEIND_UNDECIDED_USERC		},	// ユーザー紙C.
	{ PNLD_PAPERTYPEIND_USERCRMV,	PNLD_PAPERTYPEIND_UNDECIDED_USERCRMV	},	// ユーザー紙C(2面目).
#endif //#ifndef DEF_HeliosMLKCus
#else // DEF_A4_Cus.
	{ PNLD_PAPERTYPEIND_NORMAL,		PNLD_PAPERTYPEIND_UNDECIDED_NORMAL		},	// 普通紙.	// 未サポートのためダミー設定.
#endif // DEF_A4_Cus.
};


static const struct TYPS_TypeUserConversion {
	TYPE_PrinterAdjust			e_PrinterAdjust;
	TYPE_PaperType				e_PaperType;
} st_NewTypeUserConversion[] = {
	{ TYPE_PrinterAdjustBasicWeightUser1,	TYPE_PaperTypeUser1	},
	{ TYPE_PrinterAdjustBasicWeightUser2,	TYPE_PaperTypeUser2	},
	{ TYPE_PrinterAdjustBasicWeightUser3,	TYPE_PaperTypeUser3	},
	{ TYPE_PrinterAdjustBasicWeightUser4,	TYPE_PaperTypeUser4	},
	{ TYPE_PrinterAdjustBasicWeightUser5,	TYPE_PaperTypeUser5	},
	{ TYPE_PrinterAdjustBasicWeightUser6,	TYPE_PaperTypeUser6	},
#ifndef DEF_A4_Cus  // comment 2019/06/28 SCD4-Sky西村 [EagleZ+][機器構成変更No.1] ユーザー紙C 追加対応.
//comment 2019/11/18 [HCLT-Thangachamy] [HeliosMLK] [機器構成変更_No.11] Heliosmlk not supported papertype restricted.
#ifndef DEF_HeliosMLKCus
	{ TYPE_PrinterAdjustBasicWeightUser7,	TYPE_PaperTypeUser7	},
#endif //#ifndef DEF_HeliosMLKCus
#endif // DEF_A4_Cus.
};
static const uchar uc_TypeUserConversionSize = ( sizeof( st_NewTypeUserConversion ) / sizeof( st_NewTypeUserConversion[0] ) );

struct PCGS_ManualFeedTrayPaperTypeSet {
	TYPE_PaperType e_PaperTypePanel;	// 用紙設定値.
	TYPE_PaperType e_PaperType2ndSide;	// 両面２面目用紙種類設定値.
};
static const struct PCGS_ManualFeedTrayPaperTypeSet st_ManualFeedTrayPaperTypeSet[] = {		// 両面２面目設定可能用紙種類テーブル.
	{TYPE_PaperTypeNormal,		TYPE_PaperTypeNormal_2nd	},		// 普通紙.
//comment 2019/11/18 [HCLT-Thangachamy] [HeliosMLK] [機器構成変更_No.11] Heliosmlk not supported papertypea are restricted.
#ifndef DEF_HeliosMLKCus
	{TYPE_PaperTypeThin1,		TYPE_PaperTypeThin_2nd		},		// 薄紙.	// comment MinervaZeus Sky小尾 2011/04/15 [UV-01038_1：No.01].
	{TYPE_PaperTypeNormalPlus,	TYPE_PaperTypeNormalPlus_2nd},		// 普通紙+.
#endif //#ifndef DEF_HeliosMLKCus
	{TYPE_PaperTypeThick1,		TYPE_PaperTypeThick1_2nd	},		// 厚紙１.
	{TYPE_PaperTypeThick1Plus,	TYPE_PaperTypeThick1Plus_2nd},		// 厚紙１.
	{TYPE_PaperTypeThick2,		TYPE_PaperTypeThick2_2nd	},		// 厚紙２.
	{TYPE_PaperTypeThick3,		TYPE_PaperTypeThick3_2nd	},		// 厚紙３.
//comment 2019/11/18 [HCLT-Thangachamy] [HeliosMLK] [機器構成変更_No.11] Heliosmlk not supported papertypea are restricted.
#ifndef DEF_HeliosMLKCus
	{TYPE_PaperTypeThick4,		TYPE_PaperTypeThick4_2nd	},		// 厚紙４.
#endif //#ifndef DEF_HeliosMLKCus
	{TYPE_PaperRecycle1,		TYPE_PaperTypeNormal_2nd	},		// 再生紙 // comment MinervaZeus ADD Sky上西 2011/05/05 [UV-01038][機器1001].
	{TYPE_PaperTypeUser1,		TYPE_PaperTypeUser1_2nd		},		// ユーザー紙１.
	{TYPE_PaperTypeUser2,		TYPE_PaperTypeUser2_2nd		},		// ユーザー紙２.
	{TYPE_PaperTypeUser3,		TYPE_PaperTypeUser3_2nd		},		// ユーザー紙３.
	{TYPE_PaperTypeUser4,		TYPE_PaperTypeUser4_2nd		},		// ユーザー紙４.
	{TYPE_PaperTypeUser5,		TYPE_PaperTypeUser5_2nd		},		// ユーザー紙５.
	{TYPE_PaperTypeUser6,		TYPE_PaperTypeUser6_2nd		},		// ユーザー紙６.
#ifndef DEF_A4_Cus  // comment 2019/06/28 SCD4-Sky西村 [EagleZ+][機器構成変更No.1] ユーザー紙C 追加対応.
//comment 2019/11/18 [HCLT-Thangachamy] [HeliosMLK] [機器構成変更_No.11] Heliosmlk not supported papertypea are restricted.
#ifndef DEF_HeliosMLKCus
	{TYPE_PaperTypeUser7,		TYPE_PaperTypeUser7_2nd		},		// ユーザー紙６.
#endif //#ifndef DEF_HeliosMLKCus
#endif // DEF_A4_Cus.

};
// 両面２面目設定可能用紙種類テーブル数.
static const uchar uc_ManualFeedTrayPaperTypeTblNum = ( sizeof( st_ManualFeedTrayPaperTypeSet ) / sizeof( st_ManualFeedTrayPaperTypeSet[0] ) );

// カスタム用紙種類情報テーブル.
struct TYPS_CustomPaperTypeInfo {
	uchar						uc_BtnID;			// ボタンID.
	TYPE_CustomPaperType		e_CustomPaperType;	// カスタム用紙属性.
};
static TYPS_CustomPaperTypeInfo st_CustomPaperTypeInfo[] = {
	{ BTNID_KEY_PAPERKINDCUSTOM_1,	TYPE_CustomPaperType1,	},	// カスタム１.
	{ BTNID_KEY_PAPERKINDCUSTOM_2,	TYPE_CustomPaperType2,	},	// カスタム２.
	{ BTNID_KEY_PAPERKINDCUSTOM_3,	TYPE_CustomPaperType3,	},	// カスタム３.
	{ BTNID_KEY_PAPERKINDCUSTOM_4,	TYPE_CustomPaperType4,	},	// カスタム４.
	{ BTNID_KEY_PAPERKINDCUSTOM_5,	TYPE_CustomPaperType5,	},	// カスタム５.
	{ BTNID_KEY_PAPERKINDCUSTOM_6,	TYPE_CustomPaperType6,	},	// カスタム６.
	{ BTNID_KEY_PAPERKINDCUSTOM_7,	TYPE_CustomPaperType7,	},	// カスタム７.
	{ BTNID_KEY_PAPERKINDCUSTOM_8,	TYPE_CustomPaperType8,	},	// カスタム８.
	{ BTNID_KEY_PAPERKINDCUSTOM_9,	TYPE_CustomPaperType9,	},	// カスタム９.
	{ BTNID_KEY_PAPERKINDCUSTOM_10,	TYPE_CustomPaperType10,	},	// カスタム１０.
	{ BTNID_KEY_PAPERKINDCUSTOM_11,	TYPE_CustomPaperType11,	},	// カスタム１１.
	{ BTNID_KEY_PAPERKINDCUSTOM_12,	TYPE_CustomPaperType12,	},	// カスタム１２.
	{ BTNID_KEY_PAPERKINDCUSTOM_13,	TYPE_CustomPaperType13,	},	// カスタム１３.
	{ BTNID_KEY_PAPERKINDCUSTOM_14,	TYPE_CustomPaperType14,	},	// カスタム１４.
	{ BTNID_KEY_PAPERKINDCUSTOM_15,	TYPE_CustomPaperType15,	},	// カスタム１５.
	{ BTNID_KEY_PAPERKINDCUSTOM_16,	TYPE_CustomPaperType16,	},	// カスタム１６.
	{ BTNID_KEY_PAPERKINDCUSTOM_17,	TYPE_CustomPaperType17,	},	// カスタム１７.
	{ BTNID_KEY_PAPERKINDCUSTOM_18,	TYPE_CustomPaperType18,	},	// カスタム１８.
	{ BTNID_KEY_PAPERKINDCUSTOM_19,	TYPE_CustomPaperType19,	},	// カスタム１９.
	{ BTNID_KEY_PAPERKINDCUSTOM_20,	TYPE_CustomPaperType20,	},	// カスタム２０.
};
const uchar uc_CustomPaperTypeInfoNum = ( sizeof( st_CustomPaperTypeInfo ) / sizeof( st_CustomPaperTypeInfo[0] ) );


// A3A4FUM 用紙種類設定 t-okada 2015/01/19 Start 選択トレイの設定変更.
#ifndef DEF_A4_Cus
// A3A4FUM 用紙種類設定 t-okada 2015/01/19 End.
// 選択された用紙種別から、ボタンIDに変換するテーブル(Qtボタン押下処理用)
// 1-4段目、3段目(大容量)トレイの変換テーブル.
// このテーブルはst_PaperKindNewTblとuc_AryPaperTypeBtnStatusTblの並びに合わせて変更が必要です.
static const  PNLC_TrayPaperKindBtnFromPaperType tbl_TrayPaperKindBtnFromPaperType[] = {
	{	TYPE_PaperTypeNormal	,	BTNID_KEY_PAPERKINDP1_1			},	// 普通紙.
	{	TYPE_PaperTypeNotDuplex1,	BTNID_KEY_PAPERKINDP1_2			},	// 片面専用用紙.
// Comment 2015/01/29 HCLT-Vinothini [Compilation Switch ID: 22] for HeliosL support.
//#ifdef DEF_Zeus5BkCus
// Comment 2019/07/26 [HCLT_Paul] [IT6_HeliosMLK] [ Seigyu.10 ] Adding the HeliosMLK compiler switch.
//comment 2019/11/18 [HCLT-Thangachamy] [HeliosMLK] [機器構成変更_No.11] Separate enties added with HELIOSMLK supported paper types.
//#if defined(DEF_Zeus5BkCus) || defined(DEF_HeliosMLKCus)
#ifndef DEF_HeliosMLKCus
#if defined(DEF_Zeus5BkCus)
#else // #if defined(DEF_Zeus5BkCus) || defined(DEF_HeliosMLKCus).
	{	TYPE_PaperTypeThin1		,	BTNID_KEY_PAPERKINDP1_3			},	// 薄紙.
#endif // #if defined(DEF_Zeus5BkCus) || defined(DEF_HeliosCus).
	{	TYPE_PaperTypeSpecial1	,	BTNID_KEY_PAPERKINDP1_4			},	// 特殊紙.
	{	TYPE_PaperTypeNormalPlus,	BTNID_KEY_PAPERKINDP1_5			},	// 普通紙+.		// comment 2018/03/29 SCD2 K.Sato [Eagle][RQ-722]用紙種別追加（厚紙1を2種類に分割）対応.
	{	TYPE_PaperTypeThick1	,	BTNID_KEY_PAPERKINDP1_6			},	// 厚紙1.
	{	TYPE_PaperTypeThick1Plus,	BTNID_KEY_PAPERKINDP1_7			},	// 厚紙1+.
	{	TYPE_PaperTypeThick2	,	BTNID_KEY_PAPERKINDP1_8			},	// 厚紙2.
	{	TYPE_PaperTypeThick3	,	BTNID_KEY_PAPERKINDP1_9			},	// 厚紙3.
//comment 2019/04/26 HCLT-Sivateja [IT6_EagleH][OP_BTS-4773 ] Adding PaperType for PI tray in EagleHI models.
	{	TYPE_PaperTypeThick4	,	BTNID_KEY_PAPERKINDP1_12	 	},	// 厚紙4.
	{	TYPE_PaperTypeLabel		,	BTNID_KEY_PAPERKINDP1_13		},	// label.
	{	TYPE_PaperRecycle1		,	BTNID_KEY_PAPERKINDP1_10		},	// 再生紙.
	{	TYPE_PaperTypeLetterHead,	BTNID_KEY_PAPERKINDP1_11		},	// レターヘッド紙.
	{	TYPE_PaperTypeColor		,	BTNID_KEY_PAPERKINDP2_1			},	// 色紙.
	{	TYPE_PaperTypeUser1		,	BTNID_KEY_PAPERKINDP2_2			},	// ユーザー紙1.
	{	TYPE_PaperTypeUser2		,	BTNID_KEY_PAPERKINDP2_3			},	// ユーザー紙2.
	{	TYPE_PaperTypeUser3		,	BTNID_KEY_PAPERKINDP2_4			},	// ユーザー紙3.
	{	TYPE_PaperTypeUser4		,	BTNID_KEY_PAPERKINDP2_5			},	// ユーザー紙4.
	{	TYPE_PaperTypeUser5		,	BTNID_KEY_PAPERKINDP2_6			},	// ユーザー紙5.
	{	TYPE_PaperTypeUser6		,	BTNID_KEY_PAPERKINDP2_7			},	// ユーザー紙6.
	{	TYPE_PaperTypeCustom1	,	BTNID_KEY_PAPERKINDCUSTOM_1		},	// カスタマイズ用紙1.
	{	TYPE_PaperTypeCustom2	,	BTNID_KEY_PAPERKINDCUSTOM_2		},	// カスタマイズ用紙2.
	{	TYPE_PaperTypeCustom3	,	BTNID_KEY_PAPERKINDCUSTOM_3		},	// カスタマイズ用紙3.
	{	TYPE_PaperTypeCustom4	,	BTNID_KEY_PAPERKINDCUSTOM_4		},	// カスタマイズ用紙4.
	{	TYPE_PaperTypeCustom5	,	BTNID_KEY_PAPERKINDCUSTOM_5		},	// カスタマイズ用紙5.
	{	TYPE_PaperTypeCustom6	,	BTNID_KEY_PAPERKINDCUSTOM_6		},	// カスタマイズ用紙6.
	{	TYPE_PaperTypeCustom7	,	BTNID_KEY_PAPERKINDCUSTOM_7		},	// カスタマイズ用紙7.
	{	TYPE_PaperTypeCustom8	,	BTNID_KEY_PAPERKINDCUSTOM_8		},	// カスタマイズ用紙8.
	{	TYPE_PaperTypeCustom9	,	BTNID_KEY_PAPERKINDCUSTOM_9		},	// カスタマイズ用紙9.
	{	TYPE_PaperTypeCustom10	,	BTNID_KEY_PAPERKINDCUSTOM_10	},	// カスタマイズ用紙10.
	{	TYPE_PaperTypeCustom11	,	BTNID_KEY_PAPERKINDCUSTOM_11	},	// カスタマイズ用紙11.
	{	TYPE_PaperTypeCustom12	,	BTNID_KEY_PAPERKINDCUSTOM_12	},	// カスタマイズ用紙12.
	{	TYPE_PaperTypeCustom13	,	BTNID_KEY_PAPERKINDCUSTOM_13	},	// カスタマイズ用紙13.
	{	TYPE_PaperTypeCustom14	,	BTNID_KEY_PAPERKINDCUSTOM_14	},	// カスタマイズ用紙14.
	{	TYPE_PaperTypeCustom15	,	BTNID_KEY_PAPERKINDCUSTOM_15	},	// カスタマイズ用紙15.
	{	TYPE_PaperTypeCustom16	,	BTNID_KEY_PAPERKINDCUSTOM_16	},	// カスタマイズ用紙16.
	{	TYPE_PaperTypeCustom17	,	BTNID_KEY_PAPERKINDCUSTOM_17	},	// カスタマイズ用紙17.
	{	TYPE_PaperTypeCustom18	,	BTNID_KEY_PAPERKINDCUSTOM_18	},	// カスタマイズ用紙18.
	{	TYPE_PaperTypeCustom19	,	BTNID_KEY_PAPERKINDCUSTOM_19	},	// カスタマイズ用紙19.
	{	TYPE_PaperTypeCustom20	,	BTNID_KEY_PAPERKINDCUSTOM_20	},	// カスタマイズ用紙20.
// 特ROM キンコーズ・ジャパン様の封筒専用機対応.
	{	TYPE_PaperTypeEnvelope	,	BTNID_KEY_PAPERKINDP1_1	},	// 封筒.// comment 2014/09/05 OPS1-Sky東島 [特ROM対応][MZ2ND-138_機種マージ]キンコーズ・ジャパン様の封筒専用機対応.
//comment 2019/11/18 [HCLT-Thangachamy] [HeliosMLK] [機器構成変更_No.11] Separate enties added with HELIOSMLK supported paper types.
#else //#ifndef DEF_HeliosMLKCus
// for HeliosMLK
	{	TYPE_PaperTypeSpecial1	,	BTNID_KEY_PAPERKINDP1_3			},	// 特殊紙.
	{	TYPE_PaperTypeThick1	,	BTNID_KEY_PAPERKINDP1_4			},	// 厚紙1.
	{	TYPE_PaperTypeThick1Plus,	BTNID_KEY_PAPERKINDP1_5			},	// 厚紙1+.
	{	TYPE_PaperTypeThick2	,	BTNID_KEY_PAPERKINDP1_6			},	// 厚紙2.
	{	TYPE_PaperTypeThick3	,	BTNID_KEY_PAPERKINDP1_7			},	// 厚紙3.
	{	TYPE_PaperRecycle1		,	BTNID_KEY_PAPERKINDP1_8			},	// 再生紙.
	{	TYPE_PaperTypeLetterHead,	BTNID_KEY_PAPERKINDP1_9			},	// レターヘッド紙.
	{	TYPE_PaperTypeColor		,	BTNID_KEY_PAPERKINDP1_10		},	// 色紙.
	{	TYPE_PaperTypeUser1		,	BTNID_KEY_PAPERKINDP2_1			},	// ユーザー紙1.
	{	TYPE_PaperTypeUser2		,	BTNID_KEY_PAPERKINDP2_2			},	// ユーザー紙2.
	{	TYPE_PaperTypeUser3		,	BTNID_KEY_PAPERKINDP2_3			},	// ユーザー紙3.
	{	TYPE_PaperTypeUser4		,	BTNID_KEY_PAPERKINDP2_4			},	// ユーザー紙4.
	{	TYPE_PaperTypeUser5		,	BTNID_KEY_PAPERKINDP2_5			},	// ユーザー紙5.
	{	TYPE_PaperTypeUser6		,	BTNID_KEY_PAPERKINDP2_6			},	// ユーザー紙6.
	{	TYPE_PaperTypeCustom1	,	BTNID_KEY_PAPERKINDCUSTOM_1		},	// カスタマイズ用紙1.
	{	TYPE_PaperTypeCustom2	,	BTNID_KEY_PAPERKINDCUSTOM_2		},	// カスタマイズ用紙2.
	{	TYPE_PaperTypeCustom3	,	BTNID_KEY_PAPERKINDCUSTOM_3		},	// カスタマイズ用紙3.
	{	TYPE_PaperTypeCustom4	,	BTNID_KEY_PAPERKINDCUSTOM_4		},	// カスタマイズ用紙4.
	{	TYPE_PaperTypeCustom5	,	BTNID_KEY_PAPERKINDCUSTOM_5		},	// カスタマイズ用紙5.
	{	TYPE_PaperTypeCustom6	,	BTNID_KEY_PAPERKINDCUSTOM_6		},	// カスタマイズ用紙6.
	{	TYPE_PaperTypeCustom7	,	BTNID_KEY_PAPERKINDCUSTOM_7		},	// カスタマイズ用紙7.
	{	TYPE_PaperTypeCustom8	,	BTNID_KEY_PAPERKINDCUSTOM_8		},	// カスタマイズ用紙8.
	{	TYPE_PaperTypeCustom9	,	BTNID_KEY_PAPERKINDCUSTOM_9		},	// カスタマイズ用紙9.
	{	TYPE_PaperTypeCustom10	,	BTNID_KEY_PAPERKINDCUSTOM_10	},	// カスタマイズ用紙10.
	{	TYPE_PaperTypeCustom11	,	BTNID_KEY_PAPERKINDCUSTOM_11	},	// カスタマイズ用紙11.
	{	TYPE_PaperTypeCustom12	,	BTNID_KEY_PAPERKINDCUSTOM_12	},	// カスタマイズ用紙12.
	{	TYPE_PaperTypeCustom13	,	BTNID_KEY_PAPERKINDCUSTOM_13	},	// カスタマイズ用紙13.
	{	TYPE_PaperTypeCustom14	,	BTNID_KEY_PAPERKINDCUSTOM_14	},	// カスタマイズ用紙14.
	{	TYPE_PaperTypeCustom15	,	BTNID_KEY_PAPERKINDCUSTOM_15	},	// カスタマイズ用紙15.
	{	TYPE_PaperTypeCustom16	,	BTNID_KEY_PAPERKINDCUSTOM_16	},	// カスタマイズ用紙16.
	{	TYPE_PaperTypeCustom17	,	BTNID_KEY_PAPERKINDCUSTOM_17	},	// カスタマイズ用紙17.
	{	TYPE_PaperTypeCustom18	,	BTNID_KEY_PAPERKINDCUSTOM_18	},	// カスタマイズ用紙18.
	{	TYPE_PaperTypeCustom19	,	BTNID_KEY_PAPERKINDCUSTOM_19	},	// カスタマイズ用紙19.
	{	TYPE_PaperTypeCustom20	,	BTNID_KEY_PAPERKINDCUSTOM_20	},	// カスタマイズ用紙20.
#endif //#ifndef DEF_HeliosMLKCus
};
//Comment [IT6-Eagle] 機器構成変更No.2 HCLT-Deeparaj 2018/02/20 Adding envelope key for tray 1.
#ifndef DEF_A4_Cus
static const  PNLC_TrayPaperKindBtnFromPaperType tbl_Tray1PaperKindBtnFromPaperType[] = {
//comment 2019/11/18 [HCLT-Thangachamy] [HeliosMLK] [機器構成変更_No.11] Separate enties added with HELIOSMLK supported paper types for Tray1.
#ifndef DEF_HeliosMLKCus
	{	TYPE_PaperTypeNormal	,	BTNID_KEY_PAPERKINDP1_1			},	// 普通紙.
	{	TYPE_PaperTypeNotDuplex1,	BTNID_KEY_PAPERKINDP1_2			},	// 片面専用用紙.
	{	TYPE_PaperTypeThin1		,	BTNID_KEY_PAPERKINDP1_3			},	// 薄紙.
	{	TYPE_PaperTypeSpecial1	,	BTNID_KEY_PAPERKINDP1_4			},	// 特殊紙.
// comment 2018/04/18 SCD2-Sky中野 [EagleL][RQ-722]普通紙+ - 追加対応.
	//{	TYPE_PaperTypeThick1	,	BTNID_KEY_PAPERKINDP1_5			},	// 厚紙1.
	//{	TYPE_PaperTypeThick1Plus,	BTNID_KEY_PAPERKINDP1_6			},	// 厚紙1+.
	//{	TYPE_PaperTypeThick2	,	BTNID_KEY_PAPERKINDP1_7			},	// 厚紙2.
	//{	TYPE_PaperTypeThick3	,	BTNID_KEY_PAPERKINDP1_8			},	// 厚紙3.
	//{	TYPE_PaperTypeEnvelope	,	BTNID_KEY_PAPERKINDP1_9			},	// 封筒.
	//{	TYPE_PaperRecycle1		,	BTNID_KEY_PAPERKINDP1_10			},	// 再生紙.
	{	TYPE_PaperTypeNormalPlus,	BTNID_KEY_PAPERKINDP1_5			},	// 普通紙+.
	{	TYPE_PaperTypeThick1	,	BTNID_KEY_PAPERKINDP1_6			},	// 厚紙1.
	{	TYPE_PaperTypeThick1Plus,	BTNID_KEY_PAPERKINDP1_7			},	// 厚紙1+.
	{	TYPE_PaperTypeThick2	,	BTNID_KEY_PAPERKINDP1_8			},	// 厚紙2.
	{	TYPE_PaperTypeThick3	,	BTNID_KEY_PAPERKINDP1_9			},	// 厚紙3.
// Comment 2021/06/01 【FPT-LinhNV46】 【MinervaSSBK】 【機器構成変更_No.21-20】 Delete paper type Envelope of Tray1 for MinervaSSBK.
#ifdef DEF_MinervaSSBKCus
	{	TYPE_PaperRecycle1		,	BTNID_KEY_PAPERKINDP1_10		},	// 再生紙.
	{	TYPE_PaperTypeLetterHead,	BTNID_KEY_PAPERKINDP1_11			},	// レターヘッド紙.
	{	TYPE_PaperTypeColor		,	BTNID_KEY_PAPERKINDP2_1			},	// 色紙.
	{	TYPE_PaperTypeUser1		,	BTNID_KEY_PAPERKINDP2_2			},	// ユーザー紙1.
	{	TYPE_PaperTypeUser2		,	BTNID_KEY_PAPERKINDP2_3			},	// ユーザー紙2.
	{	TYPE_PaperTypeUser3		,	BTNID_KEY_PAPERKINDP2_4			},	// ユーザー紙3.
	{	TYPE_PaperTypeUser4		,	BTNID_KEY_PAPERKINDP2_5			},	// ユーザー紙4.
	{	TYPE_PaperTypeUser5		,	BTNID_KEY_PAPERKINDP2_6			},	// ユーザー紙5.
	{	TYPE_PaperTypeUser6		,	BTNID_KEY_PAPERKINDP2_7			},	// ユーザー紙6.
#else //DEF_MinervaSSBKCus.
	{	TYPE_PaperTypeEnvelope	,	BTNID_KEY_PAPERKINDP1_10		},	// 封筒.
	{	TYPE_PaperRecycle1		,	BTNID_KEY_PAPERKINDP1_11		},	// 再生紙.
	{	TYPE_PaperTypeLetterHead,	BTNID_KEY_PAPERKINDP2_1			},	// レターヘッド紙.
	{	TYPE_PaperTypeColor		,	BTNID_KEY_PAPERKINDP2_2			},	// 色紙.
	{	TYPE_PaperTypeUser1		,	BTNID_KEY_PAPERKINDP2_3			},	// ユーザー紙1.
	{	TYPE_PaperTypeUser2		,	BTNID_KEY_PAPERKINDP2_4			},	// ユーザー紙2.
	{	TYPE_PaperTypeUser3		,	BTNID_KEY_PAPERKINDP2_5			},	// ユーザー紙3.
	{	TYPE_PaperTypeUser4		,	BTNID_KEY_PAPERKINDP2_6			},	// ユーザー紙4.
	{	TYPE_PaperTypeUser5		,	BTNID_KEY_PAPERKINDP2_7			},	// ユーザー紙5.
	{	TYPE_PaperTypeUser6		,	BTNID_KEY_PAPERKINDP2_8			},	// ユーザー紙6.
#endif //DEF_MinervaSSBKCus.
//Separate enties added with HELIOSMLK supported paper types for Tray1.
#else //#ifndef DEF_HeliosMLKCus
	{	TYPE_PaperTypeNormal	,	BTNID_KEY_PAPERKINDP1_1			},	// 普通紙.
	{	TYPE_PaperTypeNotDuplex1,	BTNID_KEY_PAPERKINDP1_2			},	// 片面専用用紙.
	{	TYPE_PaperTypeSpecial1	,	BTNID_KEY_PAPERKINDP1_3			},	// 特殊紙.
	{	TYPE_PaperTypeThick1	,	BTNID_KEY_PAPERKINDP1_4			},	// 厚紙1.
	{	TYPE_PaperTypeThick1Plus,	BTNID_KEY_PAPERKINDP1_5			},	// 厚紙1+.
	{	TYPE_PaperTypeThick2	,	BTNID_KEY_PAPERKINDP1_6			},	// 厚紙2.
	{	TYPE_PaperTypeThick3	,	BTNID_KEY_PAPERKINDP1_7			},	// 厚紙3.
	{	TYPE_PaperRecycle1		,	BTNID_KEY_PAPERKINDP1_8			},	// 再生紙.
	{	TYPE_PaperTypeLetterHead,	BTNID_KEY_PAPERKINDP1_9			},	// レターヘッド紙.
	{	TYPE_PaperTypeColor		,	BTNID_KEY_PAPERKINDP1_10		},	// 色紙.
	{	TYPE_PaperTypeUser1		,	BTNID_KEY_PAPERKINDP2_1			},	// ユーザー紙1.
	{	TYPE_PaperTypeUser2		,	BTNID_KEY_PAPERKINDP2_2			},	// ユーザー紙2.
	{	TYPE_PaperTypeUser3		,	BTNID_KEY_PAPERKINDP2_3			},	// ユーザー紙3.
	{	TYPE_PaperTypeUser4		,	BTNID_KEY_PAPERKINDP2_4			},	// ユーザー紙4.
	{	TYPE_PaperTypeUser5		,	BTNID_KEY_PAPERKINDP2_5			},	// ユーザー紙5.
	{	TYPE_PaperTypeUser6		,	BTNID_KEY_PAPERKINDP2_6			},	// ユーザー紙6.
#endif //#ifndef DEF_HeliosMLKCus
	{	TYPE_PaperTypeCustom1	,	BTNID_KEY_PAPERKINDCUSTOM_1		},	// カスタマイズ用紙1.
	{	TYPE_PaperTypeCustom2	,	BTNID_KEY_PAPERKINDCUSTOM_2		},	// カスタマイズ用紙2.
	{	TYPE_PaperTypeCustom3	,	BTNID_KEY_PAPERKINDCUSTOM_3		},	// カスタマイズ用紙3.
	{	TYPE_PaperTypeCustom4	,	BTNID_KEY_PAPERKINDCUSTOM_4		},	// カスタマイズ用紙4.
	{	TYPE_PaperTypeCustom5	,	BTNID_KEY_PAPERKINDCUSTOM_5		},	// カスタマイズ用紙5.
	{	TYPE_PaperTypeCustom6	,	BTNID_KEY_PAPERKINDCUSTOM_6		},	// カスタマイズ用紙6.
	{	TYPE_PaperTypeCustom7	,	BTNID_KEY_PAPERKINDCUSTOM_7		},	// カスタマイズ用紙7.
	{	TYPE_PaperTypeCustom8	,	BTNID_KEY_PAPERKINDCUSTOM_8		},	// カスタマイズ用紙8.
	{	TYPE_PaperTypeCustom9	,	BTNID_KEY_PAPERKINDCUSTOM_9		},	// カスタマイズ用紙9.
	{	TYPE_PaperTypeCustom10	,	BTNID_KEY_PAPERKINDCUSTOM_10	},	// カスタマイズ用紙10.
	{	TYPE_PaperTypeCustom11	,	BTNID_KEY_PAPERKINDCUSTOM_11	},	// カスタマイズ用紙11.
	{	TYPE_PaperTypeCustom12	,	BTNID_KEY_PAPERKINDCUSTOM_12	},	// カスタマイズ用紙12.
	{	TYPE_PaperTypeCustom13	,	BTNID_KEY_PAPERKINDCUSTOM_13	},	// カスタマイズ用紙13.
	{	TYPE_PaperTypeCustom14	,	BTNID_KEY_PAPERKINDCUSTOM_14	},	// カスタマイズ用紙14.
	{	TYPE_PaperTypeCustom15	,	BTNID_KEY_PAPERKINDCUSTOM_15	},	// カスタマイズ用紙15.
	{	TYPE_PaperTypeCustom16	,	BTNID_KEY_PAPERKINDCUSTOM_16	},	// カスタマイズ用紙16.
	{	TYPE_PaperTypeCustom17	,	BTNID_KEY_PAPERKINDCUSTOM_17	},	// カスタマイズ用紙17.
	{	TYPE_PaperTypeCustom18	,	BTNID_KEY_PAPERKINDCUSTOM_18	},	// カスタマイズ用紙18.
	{	TYPE_PaperTypeCustom19	,	BTNID_KEY_PAPERKINDCUSTOM_19	},	// カスタマイズ用紙19.
	{	TYPE_PaperTypeCustom20	,	BTNID_KEY_PAPERKINDCUSTOM_20	},	// カスタマイズ用紙20.
};
#endif	//DEF_A4_Cus.
const uchar uc_TrayPaperKindBtnFromPaperTypeNum = ( sizeof( tbl_TrayPaperKindBtnFromPaperType ) / sizeof( tbl_TrayPaperKindBtnFromPaperType[0] ) );
//Comment [IT6-Eagle] 機器構成変更No.2 HCLT-Deeparaj 2018/02/20 Adding envelope key for tray 1.
#ifndef DEF_A4_Cus
const uchar uc_Tray1PaperKindBtnFromPaperTypeNum = ( sizeof( tbl_Tray1PaperKindBtnFromPaperType ) / sizeof( tbl_Tray1PaperKindBtnFromPaperType[0] ) );
#endif	//DEF_A4_Cus.

// A3A4FUM 用紙種類設定 t-okada 2015/01/19 Start 選択トレイの設定変更.
#else // DEF_A4_Cus.

// 選択された用紙種別から、ボタンIDに変換するテーブル(Qtボタン押下処理用)
// 1-4段目、3段目(大容量)トレイの変換テーブル.
// このテーブルはst_PaperKindNewTblとuc_AryPaperTypeBtnStatusTblの並びに合わせて変更が必要です.
static const  PNLC_TrayPaperKindBtnFromPaperType tbl_TrayPaperKindBtnFromPaperType[PNLD_PaperTypeTrayMax][PNLD_Tbl_TrayPaperKindBtnFromPaperType_MaxNum] = {
	// トレイ1.
	{	{	TYPE_PaperTypeNormal	,	BTNID_KEY_PAPERKINDP1_1			},	// 普通紙.
		{	TYPE_PaperTypeNotDuplex1,	BTNID_KEY_PAPERKINDP1_2			},	// 片面専用用紙.
		{	TYPE_PaperTypeSpecial1	,	BTNID_KEY_PAPERKINDP1_3			},	// 特殊紙.
		{	TYPE_PaperTypeNormalPlus,	BTNID_KEY_PAPERKINDP1_4			},	// 普通紙+.		// comment 2018/03/29 SCD2 K.Sato [Eagle][RQ-722]用紙種別追加（厚紙1を2種類に分割）対応.
		{	TYPE_PaperTypeThick1	,	BTNID_KEY_PAPERKINDP1_5			},	// 厚紙1.
// [KronosS] Sky荻堂 2016/09/26 管理No.22 用紙種類カスタマイズ.
#ifdef DEF_KRONOSS_Cus
		{	TYPE_PaperRecycle1		,	BTNID_KEY_PAPERKINDP1_5			},	// 再生紙.	
		{	TYPE_PaperTypeLetterHead,	BTNID_KEY_PAPERKINDP1_6			},	// レターヘッド紙.
		{	TYPE_PaperTypeColor		,	BTNID_KEY_PAPERKINDP1_7			},	// 色紙.
		{	TYPE_PaperTypeLabel		,	BTNID_KEY_PAPERKINDP1_8			},	// ラベル紙.
		{	TYPE_PaperTypeUser1		,	BTNID_KEY_PAPERKINDP1_9			},	// ユーザー紙1.
		{	TYPE_PaperTypeUser2		,	BTNID_KEY_PAPERKINDP1_10		},	// ユーザー紙2.
		{	TYPE_PaperTypeUser3		,	BTNID_KEY_PAPERKINDP2_1			},	// ユーザー紙3.
#else
		{	TYPE_PaperTypeThick1Plus,	BTNID_KEY_PAPERKINDP1_6			},	// 厚紙1+.
		{	TYPE_PaperTypeThick2	,	BTNID_KEY_PAPERKINDP1_7			},	// 厚紙2.
		{	TYPE_PaperTypeEnvelope	,	BTNID_KEY_PAPERKINDP1_8			},	// 封筒.
		{	TYPE_PaperRecycle1		,	BTNID_KEY_PAPERKINDP1_9			},	// 再生紙.	
		{	TYPE_PaperTypeLetterHead,	BTNID_KEY_PAPERKINDP1_10			},	// レターヘッド紙.
		{	TYPE_PaperTypeColor		,	BTNID_KEY_PAPERKINDP1_11		},	// 色紙.
		{	TYPE_PaperTypeGloss1	,	BTNID_KEY_PAPERKINDP2_1			},	// 光沢紙1.
		{	TYPE_PaperTypeGloss1Plus,	BTNID_KEY_PAPERKINDP2_2			},	// 光沢紙1+.
		{	TYPE_PaperTypeGloss2	,	BTNID_KEY_PAPERKINDP2_3			},	// 光沢紙2.
		{	TYPE_PaperPostCard		,	BTNID_KEY_PAPERKINDP2_4			},	// はがき.
		{	TYPE_PaperTypeLabel		,	BTNID_KEY_PAPERKINDP2_5			},	// ラベル紙.
		{	TYPE_PaperTypeUser1		,	BTNID_KEY_PAPERKINDP2_6			},	// ユーザー紙1.
		{	TYPE_PaperTypeUser2		,	BTNID_KEY_PAPERKINDP2_7			},	// ユーザー紙2.
		{	TYPE_PaperTypeUser3		,	BTNID_KEY_PAPERKINDP2_8			},	// ユーザー紙3.
		{	TYPE_PaperTypeUser4		,	BTNID_KEY_PAPERKINDP2_9			},	// ユーザー紙4.
		{	TYPE_PaperTypeUser5		,	BTNID_KEY_PAPERKINDP2_10		},	// ユーザー紙5.
#endif
		{	TYPE_PaperTypeCustom1	,	BTNID_KEY_PAPERKINDCUSTOM_1		},	// カスタマイズ用紙1.
		{	TYPE_PaperTypeCustom2	,	BTNID_KEY_PAPERKINDCUSTOM_2		},	// カスタマイズ用紙2.
		{	TYPE_PaperTypeCustom3	,	BTNID_KEY_PAPERKINDCUSTOM_3		},	// カスタマイズ用紙3.
		{	TYPE_PaperTypeCustom4	,	BTNID_KEY_PAPERKINDCUSTOM_4		},	// カスタマイズ用紙4.
		{	TYPE_PaperTypeCustom5	,	BTNID_KEY_PAPERKINDCUSTOM_5		},	// カスタマイズ用紙5.
		{	TYPE_PaperTypeCustom6	,	BTNID_KEY_PAPERKINDCUSTOM_6		},	// カスタマイズ用紙6.
		{	TYPE_PaperTypeCustom7	,	BTNID_KEY_PAPERKINDCUSTOM_7		},	// カスタマイズ用紙7.
		{	TYPE_PaperTypeCustom8	,	BTNID_KEY_PAPERKINDCUSTOM_8		},	// カスタマイズ用紙8.
		{	TYPE_PaperTypeCustom9	,	BTNID_KEY_PAPERKINDCUSTOM_9		},	// カスタマイズ用紙9.
		{	TYPE_PaperTypeCustom10	,	BTNID_KEY_PAPERKINDCUSTOM_10	},	// カスタマイズ用紙10.
		{	TYPE_PaperTypeCustom11	,	BTNID_KEY_PAPERKINDCUSTOM_11	},	// カスタマイズ用紙11.
		{	TYPE_PaperTypeCustom12	,	BTNID_KEY_PAPERKINDCUSTOM_12	},	// カスタマイズ用紙12.
		{	TYPE_PaperTypeCustom13	,	BTNID_KEY_PAPERKINDCUSTOM_13	},	// カスタマイズ用紙13.
		{	TYPE_PaperTypeCustom14	,	BTNID_KEY_PAPERKINDCUSTOM_14	},	// カスタマイズ用紙14.
		{	TYPE_PaperTypeCustom15	,	BTNID_KEY_PAPERKINDCUSTOM_15	},	// カスタマイズ用紙15.
		{	TYPE_PaperTypeCustom16	,	BTNID_KEY_PAPERKINDCUSTOM_16	},	// カスタマイズ用紙16.
		{	TYPE_PaperTypeCustom17	,	BTNID_KEY_PAPERKINDCUSTOM_17	},	// カスタマイズ用紙17.
		{	TYPE_PaperTypeCustom18	,	BTNID_KEY_PAPERKINDCUSTOM_18	},	// カスタマイズ用紙18.
		{	TYPE_PaperTypeCustom19	,	BTNID_KEY_PAPERKINDCUSTOM_19	},	// カスタマイズ用紙19.
		{	TYPE_PaperTypeCustom20	,	BTNID_KEY_PAPERKINDCUSTOM_20	},	// カスタマイズ用紙20.
	},
	// トレイ2.
	{	{	TYPE_PaperTypeNormal	,	BTNID_KEY_PAPERKINDP1_1			},	// 普通紙.
		{	TYPE_PaperTypeNotDuplex1,	BTNID_KEY_PAPERKINDP1_2			},	// 片面専用用紙.
		{	TYPE_PaperTypeSpecial1	,	BTNID_KEY_PAPERKINDP1_3			},	// 特殊紙.
		{	TYPE_PaperTypeNormalPlus,	BTNID_KEY_PAPERKINDP1_4			},	// 普通紙+.		// comment 2018/03/29 SCD2 K.Sato [Eagle][RQ-722]用紙種別追加（厚紙1を2種類に分割）対応.
// [KronosS] Sky荻堂 2016/09/26 管理No.22 用紙種類カスタマイズ.
#ifdef DEF_KRONOSS_Cus
		{	TYPE_PaperTypeThick1	,	BTNID_KEY_PAPERKINDP1_4			},	// 厚紙1.
		{	TYPE_PaperRecycle1		,	BTNID_KEY_PAPERKINDP1_5			},	// 再生紙.	
// comment 2019/03/22 SCD2-Sky鳥居 [特ROM][FQ-OPSD-1800408-OFP218]KronosS MFP / トレイ２にレターヘッド設定ができてしまう対策.
//[KronosS] Sky川名 2016/12/21 課題番号:1007853 選択トレイ画面にレターヘッド紙キーが表示されない.
//		{	TYPE_PaperTypeLetterHead,	BTNID_KEY_PAPERKINDP1_6			},	// レターヘッド紙.
		{	TYPE_PaperTypeColor		,	BTNID_KEY_PAPERKINDP1_6			},	// 色紙.
		{	TYPE_PaperTypeLabel		,	BTNID_KEY_PAPERKINDP1_7			},	// ラベル紙.
		{	TYPE_PaperTypeUser1		,	BTNID_KEY_PAPERKINDP1_8			},	// ユーザー紙1.
		{	TYPE_PaperTypeUser2		,	BTNID_KEY_PAPERKINDP1_9			},	// ユーザー紙2.
		{	TYPE_PaperTypeUser3		,	BTNID_KEY_PAPERKINDP1_10		},	// ユーザー紙3.
#else
		{	TYPE_PaperRecycle1		,	BTNID_KEY_PAPERKINDP1_5			},	// 再生紙.	
//		{	TYPE_PaperTypeLetterHead,	BTNID_KEY_PAPERKINDP1_6			},	// レターヘッド紙.
// Comment 2023/09/07 FPT-ThinhND40 [IT6_2.3] [ 機器構成変更_No.55 ] Removal of prohibition on letterhead of additional cassettes.
		{	TYPE_PaperTypeLetterHead,	BTNID_KEY_PAPERKINDP1_6			},	// レターヘッド紙.
		{	TYPE_PaperTypeColor		,	BTNID_KEY_PAPERKINDP1_7			},	// 色紙.
		{	TYPE_PaperTypeUser1		,	BTNID_KEY_PAPERKINDP1_8			},	// ユーザー紙1.
		{	TYPE_PaperTypeUser2		,	BTNID_KEY_PAPERKINDP1_9			},	// ユーザー紙2.
#endif
		{	TYPE_PaperTypeCustom1	,	BTNID_KEY_PAPERKINDCUSTOM_1		},	// カスタマイズ用紙1.
		{	TYPE_PaperTypeCustom2	,	BTNID_KEY_PAPERKINDCUSTOM_2		},	// カスタマイズ用紙2.
		{	TYPE_PaperTypeCustom3	,	BTNID_KEY_PAPERKINDCUSTOM_3		},	// カスタマイズ用紙3.
		{	TYPE_PaperTypeCustom4	,	BTNID_KEY_PAPERKINDCUSTOM_4		},	// カスタマイズ用紙4.
		{	TYPE_PaperTypeCustom5	,	BTNID_KEY_PAPERKINDCUSTOM_5		},	// カスタマイズ用紙5.
		{	TYPE_PaperTypeCustom6	,	BTNID_KEY_PAPERKINDCUSTOM_6		},	// カスタマイズ用紙6.
		{	TYPE_PaperTypeCustom7	,	BTNID_KEY_PAPERKINDCUSTOM_7		},	// カスタマイズ用紙7.
		{	TYPE_PaperTypeCustom8	,	BTNID_KEY_PAPERKINDCUSTOM_8		},	// カスタマイズ用紙8.
		{	TYPE_PaperTypeCustom9	,	BTNID_KEY_PAPERKINDCUSTOM_9		},	// カスタマイズ用紙9.
		{	TYPE_PaperTypeCustom10	,	BTNID_KEY_PAPERKINDCUSTOM_10	},	// カスタマイズ用紙10.
		{	TYPE_PaperTypeCustom11	,	BTNID_KEY_PAPERKINDCUSTOM_11	},	// カスタマイズ用紙11.
		{	TYPE_PaperTypeCustom12	,	BTNID_KEY_PAPERKINDCUSTOM_12	},	// カスタマイズ用紙12.
		{	TYPE_PaperTypeCustom13	,	BTNID_KEY_PAPERKINDCUSTOM_13	},	// カスタマイズ用紙13.
		{	TYPE_PaperTypeCustom14	,	BTNID_KEY_PAPERKINDCUSTOM_14	},	// カスタマイズ用紙14.
		{	TYPE_PaperTypeCustom15	,	BTNID_KEY_PAPERKINDCUSTOM_15	},	// カスタマイズ用紙15.
		{	TYPE_PaperTypeCustom16	,	BTNID_KEY_PAPERKINDCUSTOM_16	},	// カスタマイズ用紙16.
		{	TYPE_PaperTypeCustom17	,	BTNID_KEY_PAPERKINDCUSTOM_17	},	// カスタマイズ用紙17.
		{	TYPE_PaperTypeCustom18	,	BTNID_KEY_PAPERKINDCUSTOM_18	},	// カスタマイズ用紙18.
		{	TYPE_PaperTypeCustom19	,	BTNID_KEY_PAPERKINDCUSTOM_19	},	// カスタマイズ用紙19.
		{	TYPE_PaperTypeCustom20	,	BTNID_KEY_PAPERKINDCUSTOM_20	},	// カスタマイズ用紙20.
		{	TYPE_PaperTypeUnknown	,	0								},	// なし.
// [KronosS] Sky荻堂 2016/09/26 管理No.22 用紙種類カスタマイズ.
// comment 2019/03/22 SCD2-Sky鳥居 [特ROM][FQ-OPSD-1800408-OFP218]KronosS MFP / トレイ２にレターヘッド設定ができてしまう対策.
#ifndef DEF_KRONOSS_Cus
// Comment 2023/09/07 FPT-ThinhND40 [IT6_2.3] [ 機器構成変更_No.55 ] Removal of prohibition on letterhead of additional cassettes.
//		{	TYPE_PaperTypeUnknown	,	0								},	// なし.
		{	TYPE_PaperTypeUnknown	,	0								},	// なし.
		{	TYPE_PaperTypeUnknown	,	0								},	// なし.
		{	TYPE_PaperTypeUnknown	,	0								},	// なし.
		{	TYPE_PaperTypeUnknown	,	0								},	// なし.
		{	TYPE_PaperTypeUnknown	,	0								},	// なし.
		{	TYPE_PaperTypeUnknown	,	0								},	// なし.
		{	TYPE_PaperTypeUnknown	,	0								},	// なし.
		{	TYPE_PaperTypeUnknown	,	0								},	// なし.
		{	TYPE_PaperTypeUnknown	,	0								},	// なし.
		{	TYPE_PaperTypeUnknown	,	0								},	// なし.
		{	TYPE_PaperTypeUnknown	,	0								},	// なし.
#endif
	},
	// トレイ3.
	{	{	TYPE_PaperTypeNormal	,	BTNID_KEY_PAPERKINDP1_1			},	// 普通紙.
		{	TYPE_PaperTypeNotDuplex1,	BTNID_KEY_PAPERKINDP1_2			},	// 片面専用用紙.
		{	TYPE_PaperTypeSpecial1	,	BTNID_KEY_PAPERKINDP1_3			},	// 特殊紙.
		{	TYPE_PaperTypeNormalPlus,	BTNID_KEY_PAPERKINDP1_4			},	// 普通紙+.		// comment 2018/03/29 SCD2 K.Sato [Eagle][RQ-722]用紙種別追加（厚紙1を2種類に分割）対応.
// [KronosS] Sky荻堂 2016/09/26 管理No.22 用紙種類カスタマイズ.
#ifdef DEF_KRONOSS_Cus
		{	TYPE_PaperTypeThick1	,	BTNID_KEY_PAPERKINDP1_4			},	// 厚紙1.
		{	TYPE_PaperRecycle1		,	BTNID_KEY_PAPERKINDP1_5			},	// 再生紙.	
// comment 2019/03/22 SCD2-Sky鳥居 [特ROM][FQ-OPSD-1800408-OFP218]KronosS MFP / トレイ２にレターヘッド設定ができてしまう対策.
//[KronosS] Sky川名 2016/12/21 課題番号:1007853 選択トレイ画面にレターヘッド紙キーが表示されない.
//		{	TYPE_PaperTypeLetterHead,	BTNID_KEY_PAPERKINDP1_6			},	// レターヘッド紙.
		{	TYPE_PaperTypeColor		,	BTNID_KEY_PAPERKINDP1_6			},	// 色紙.
		{	TYPE_PaperTypeLabel		,	BTNID_KEY_PAPERKINDP1_7			},	// ラベル紙.
		{	TYPE_PaperTypeUser1		,	BTNID_KEY_PAPERKINDP1_8			},	// ユーザー紙1.
		{	TYPE_PaperTypeUser2		,	BTNID_KEY_PAPERKINDP1_9			},	// ユーザー紙2.
		{	TYPE_PaperTypeUser3		,	BTNID_KEY_PAPERKINDP1_10		},	// ユーザー紙3.
#else
		{	TYPE_PaperRecycle1		,	BTNID_KEY_PAPERKINDP1_5			},	// 再生紙.	
//		{	TYPE_PaperTypeLetterHead,	BTNID_KEY_PAPERKINDP1_6			},	// レターヘッド紙.
// Comment 2023/09/07 FPT-ThinhND40 [IT6_2.3] [ 機器構成変更_No.55 ] Removal of prohibition on letterhead of additional cassettes.
		{	TYPE_PaperTypeLetterHead,	BTNID_KEY_PAPERKINDP1_6			},	// レターヘッド紙.
		{	TYPE_PaperTypeColor		,	BTNID_KEY_PAPERKINDP1_7			},	// 色紙.
		{	TYPE_PaperTypeUser1		,	BTNID_KEY_PAPERKINDP1_8			},	// ユーザー紙1.
		{	TYPE_PaperTypeUser2		,	BTNID_KEY_PAPERKINDP1_9			},	// ユーザー紙2.
#endif
		{	TYPE_PaperTypeCustom1	,	BTNID_KEY_PAPERKINDCUSTOM_1		},	// カスタマイズ用紙1.
		{	TYPE_PaperTypeCustom2	,	BTNID_KEY_PAPERKINDCUSTOM_2		},	// カスタマイズ用紙2.
		{	TYPE_PaperTypeCustom3	,	BTNID_KEY_PAPERKINDCUSTOM_3		},	// カスタマイズ用紙3.
		{	TYPE_PaperTypeCustom4	,	BTNID_KEY_PAPERKINDCUSTOM_4		},	// カスタマイズ用紙4.
		{	TYPE_PaperTypeCustom5	,	BTNID_KEY_PAPERKINDCUSTOM_5		},	// カスタマイズ用紙5.
		{	TYPE_PaperTypeCustom6	,	BTNID_KEY_PAPERKINDCUSTOM_6		},	// カスタマイズ用紙6.
		{	TYPE_PaperTypeCustom7	,	BTNID_KEY_PAPERKINDCUSTOM_7		},	// カスタマイズ用紙7.
		{	TYPE_PaperTypeCustom8	,	BTNID_KEY_PAPERKINDCUSTOM_8		},	// カスタマイズ用紙8.
		{	TYPE_PaperTypeCustom9	,	BTNID_KEY_PAPERKINDCUSTOM_9		},	// カスタマイズ用紙9.
		{	TYPE_PaperTypeCustom10	,	BTNID_KEY_PAPERKINDCUSTOM_10	},	// カスタマイズ用紙10.
		{	TYPE_PaperTypeCustom11	,	BTNID_KEY_PAPERKINDCUSTOM_11	},	// カスタマイズ用紙11.
		{	TYPE_PaperTypeCustom12	,	BTNID_KEY_PAPERKINDCUSTOM_12	},	// カスタマイズ用紙12.
		{	TYPE_PaperTypeCustom13	,	BTNID_KEY_PAPERKINDCUSTOM_13	},	// カスタマイズ用紙13.
		{	TYPE_PaperTypeCustom14	,	BTNID_KEY_PAPERKINDCUSTOM_14	},	// カスタマイズ用紙14.
		{	TYPE_PaperTypeCustom15	,	BTNID_KEY_PAPERKINDCUSTOM_15	},	// カスタマイズ用紙15.
		{	TYPE_PaperTypeCustom16	,	BTNID_KEY_PAPERKINDCUSTOM_16	},	// カスタマイズ用紙16.
		{	TYPE_PaperTypeCustom17	,	BTNID_KEY_PAPERKINDCUSTOM_17	},	// カスタマイズ用紙17.
		{	TYPE_PaperTypeCustom18	,	BTNID_KEY_PAPERKINDCUSTOM_18	},	// カスタマイズ用紙18.
		{	TYPE_PaperTypeCustom19	,	BTNID_KEY_PAPERKINDCUSTOM_19	},	// カスタマイズ用紙19.
		{	TYPE_PaperTypeCustom20	,	BTNID_KEY_PAPERKINDCUSTOM_20	},	// カスタマイズ用紙20.
		{	TYPE_PaperTypeUnknown	,	0								},	// なし.
// [KronosS] Sky荻堂 2016/09/26 管理No.22 用紙種類カスタマイズ.
// comment 2019/03/22 SCD2-Sky鳥居 [特ROM][FQ-OPSD-1800408-OFP218]KronosS MFP / トレイ２にレターヘッド設定ができてしまう対策.
#ifndef DEF_KRONOSS_Cus
// Comment 2023/09/07 FPT-ThinhND40 [IT6_2.3] [ 機器構成変更_No.55 ] Removal of prohibition on letterhead of additional cassettes.
//		{	TYPE_PaperTypeUnknown	,	0								},	// なし.
		{	TYPE_PaperTypeUnknown	,	0								},	// なし.
		{	TYPE_PaperTypeUnknown	,	0								},	// なし.
		{	TYPE_PaperTypeUnknown	,	0								},	// なし.
		{	TYPE_PaperTypeUnknown	,	0								},	// なし.
		{	TYPE_PaperTypeUnknown	,	0								},	// なし.
		{	TYPE_PaperTypeUnknown	,	0								},	// なし.
		{	TYPE_PaperTypeUnknown	,	0								},	// なし.
		{	TYPE_PaperTypeUnknown	,	0								},	// なし.
		{	TYPE_PaperTypeUnknown	,	0								},	// なし.
		{	TYPE_PaperTypeUnknown	,	0								},	// なし.
		{	TYPE_PaperTypeUnknown	,	0								},	// なし.
#endif
	},
// [KronosS] Sky川名 2016/10/25 管理No21_44 各トレイ毎の表示用紙サイズをKronosS仕様にあわせる.
#ifdef DEF_KRONOSS_Cus
	// トレイ4.
	{	{	TYPE_PaperTypeNormal	,	BTNID_KEY_PAPERKINDP1_1			},	// 普通紙.
		{	TYPE_PaperTypeNotDuplex1,	BTNID_KEY_PAPERKINDP1_2			},	// 片面専用用紙.
		{	TYPE_PaperTypeSpecial1	,	BTNID_KEY_PAPERKINDP1_3			},	// 特殊紙.
		{	TYPE_PaperTypeThick1	,	BTNID_KEY_PAPERKINDP1_4			},	// 厚紙1.
		{	TYPE_PaperRecycle1		,	BTNID_KEY_PAPERKINDP1_5			},	// 再生紙.	
// comment 2019/03/22 SCD2-Sky鳥居 [特ROM][FQ-OPSD-1800408-OFP218]KronosS MFP / トレイ２にレターヘッド設定ができてしまう対策.
//[KronosS] Sky川名 2016/12/21 課題番号:1007853 選択トレイ画面にレターヘッド紙キーが表示されない.
//		{	TYPE_PaperTypeLetterHead,	BTNID_KEY_PAPERKINDP1_6			},	// レターヘッド紙.
		{	TYPE_PaperTypeColor		,	BTNID_KEY_PAPERKINDP1_6			},	// 色紙.
		{	TYPE_PaperTypeLabel		,	BTNID_KEY_PAPERKINDP1_7			},	// ラベル紙.
		{	TYPE_PaperTypeUser1		,	BTNID_KEY_PAPERKINDP1_8			},	// ユーザー紙1.
		{	TYPE_PaperTypeUser2		,	BTNID_KEY_PAPERKINDP1_9			},	// ユーザー紙2.
		{	TYPE_PaperTypeUser3		,	BTNID_KEY_PAPERKINDP1_10		},	// ユーザー紙3.
// Comment KronosS [課題No.1012563] -> 2017/02/21 OPS2 Sky竹菴 カスタム紙の要素を追加.
		{	TYPE_PaperTypeCustom1	,	BTNID_KEY_PAPERKINDCUSTOM_1		},	// カスタマイズ用紙1.
		{	TYPE_PaperTypeCustom2	,	BTNID_KEY_PAPERKINDCUSTOM_2		},	// カスタマイズ用紙2.
		{	TYPE_PaperTypeCustom3	,	BTNID_KEY_PAPERKINDCUSTOM_3		},	// カスタマイズ用紙3.
		{	TYPE_PaperTypeCustom4	,	BTNID_KEY_PAPERKINDCUSTOM_4		},	// カスタマイズ用紙4.
		{	TYPE_PaperTypeCustom5	,	BTNID_KEY_PAPERKINDCUSTOM_5		},	// カスタマイズ用紙5.
		{	TYPE_PaperTypeCustom6	,	BTNID_KEY_PAPERKINDCUSTOM_6		},	// カスタマイズ用紙6.
		{	TYPE_PaperTypeCustom7	,	BTNID_KEY_PAPERKINDCUSTOM_7		},	// カスタマイズ用紙7.
		{	TYPE_PaperTypeCustom8	,	BTNID_KEY_PAPERKINDCUSTOM_8		},	// カスタマイズ用紙8.
		{	TYPE_PaperTypeCustom9	,	BTNID_KEY_PAPERKINDCUSTOM_9		},	// カスタマイズ用紙9.
		{	TYPE_PaperTypeCustom10	,	BTNID_KEY_PAPERKINDCUSTOM_10	},	// カスタマイズ用紙10.
		{	TYPE_PaperTypeCustom11	,	BTNID_KEY_PAPERKINDCUSTOM_11	},	// カスタマイズ用紙11.
		{	TYPE_PaperTypeCustom12	,	BTNID_KEY_PAPERKINDCUSTOM_12	},	// カスタマイズ用紙12.
		{	TYPE_PaperTypeCustom13	,	BTNID_KEY_PAPERKINDCUSTOM_13	},	// カスタマイズ用紙13.
		{	TYPE_PaperTypeCustom14	,	BTNID_KEY_PAPERKINDCUSTOM_14	},	// カスタマイズ用紙14.
		{	TYPE_PaperTypeCustom15	,	BTNID_KEY_PAPERKINDCUSTOM_15	},	// カスタマイズ用紙15.
		{	TYPE_PaperTypeCustom16	,	BTNID_KEY_PAPERKINDCUSTOM_16	},	// カスタマイズ用紙16.
		{	TYPE_PaperTypeCustom17	,	BTNID_KEY_PAPERKINDCUSTOM_17	},	// カスタマイズ用紙17.
		{	TYPE_PaperTypeCustom18	,	BTNID_KEY_PAPERKINDCUSTOM_18	},	// カスタマイズ用紙18.
		{	TYPE_PaperTypeCustom19	,	BTNID_KEY_PAPERKINDCUSTOM_19	},	// カスタマイズ用紙19.
		{	TYPE_PaperTypeCustom20	,	BTNID_KEY_PAPERKINDCUSTOM_20	},	// カスタマイズ用紙20.
		{	TYPE_PaperTypeUnknown	,	0								},	// なし.
// Comment KronosS [課題No.1012563] <- 2017/02/21 OPS2 Sky竹菴 カスタム紙の要素を追加.
	},
//comment 2023/07/25 FPT.LONGTH32 [IT6_2.3] 機器構成変更_No.75 Add tray 4 for Sparrow.
#else
	// トレイ4.
	{	{	TYPE_PaperTypeNormal	,	BTNID_KEY_PAPERKINDP1_1			},	// 普通紙.
		{	TYPE_PaperTypeNotDuplex1,	BTNID_KEY_PAPERKINDP1_2			},	// 片面専用用紙.
		{	TYPE_PaperTypeSpecial1	,	BTNID_KEY_PAPERKINDP1_3			},	// 特殊紙.
		{	TYPE_PaperTypeNormalPlus,	BTNID_KEY_PAPERKINDP1_4			},	// 普通紙+.
		{	TYPE_PaperRecycle1		,	BTNID_KEY_PAPERKINDP1_5			},	// 再生紙.	
// Comment 2023/09/07 FPT-ThinhND40 [IT6_2.3] [ 機器構成変更_No.55 ] Removal of prohibition on letterhead of additional cassettes.
		{	TYPE_PaperTypeLetterHead,	BTNID_KEY_PAPERKINDP1_6			},	// レターヘッド紙.
		{	TYPE_PaperTypeColor		,	BTNID_KEY_PAPERKINDP1_7			},	// 色紙.
		{	TYPE_PaperTypeUser1		,	BTNID_KEY_PAPERKINDP1_8			},	// ユーザー紙1.
		{	TYPE_PaperTypeUser2		,	BTNID_KEY_PAPERKINDP1_9			},	// ユーザー紙2.
		{	TYPE_PaperTypeCustom1	,	BTNID_KEY_PAPERKINDCUSTOM_1		},	// カスタマイズ用紙1.
		{	TYPE_PaperTypeCustom2	,	BTNID_KEY_PAPERKINDCUSTOM_2		},	// カスタマイズ用紙2.
		{	TYPE_PaperTypeCustom3	,	BTNID_KEY_PAPERKINDCUSTOM_3		},	// カスタマイズ用紙3.
		{	TYPE_PaperTypeCustom4	,	BTNID_KEY_PAPERKINDCUSTOM_4		},	// カスタマイズ用紙4.
		{	TYPE_PaperTypeCustom5	,	BTNID_KEY_PAPERKINDCUSTOM_5		},	// カスタマイズ用紙5.
		{	TYPE_PaperTypeCustom6	,	BTNID_KEY_PAPERKINDCUSTOM_6		},	// カスタマイズ用紙6.
		{	TYPE_PaperTypeCustom7	,	BTNID_KEY_PAPERKINDCUSTOM_7		},	// カスタマイズ用紙7.
		{	TYPE_PaperTypeCustom8	,	BTNID_KEY_PAPERKINDCUSTOM_8		},	// カスタマイズ用紙8.
		{	TYPE_PaperTypeCustom9	,	BTNID_KEY_PAPERKINDCUSTOM_9		},	// カスタマイズ用紙9.
		{	TYPE_PaperTypeCustom10	,	BTNID_KEY_PAPERKINDCUSTOM_10	},	// カスタマイズ用紙10.
		{	TYPE_PaperTypeCustom11	,	BTNID_KEY_PAPERKINDCUSTOM_11	},	// カスタマイズ用紙11.
		{	TYPE_PaperTypeCustom12	,	BTNID_KEY_PAPERKINDCUSTOM_12	},	// カスタマイズ用紙12.
		{	TYPE_PaperTypeCustom13	,	BTNID_KEY_PAPERKINDCUSTOM_13	},	// カスタマイズ用紙13.
		{	TYPE_PaperTypeCustom14	,	BTNID_KEY_PAPERKINDCUSTOM_14	},	// カスタマイズ用紙14.
		{	TYPE_PaperTypeCustom15	,	BTNID_KEY_PAPERKINDCUSTOM_15	},	// カスタマイズ用紙15.
		{	TYPE_PaperTypeCustom16	,	BTNID_KEY_PAPERKINDCUSTOM_16	},	// カスタマイズ用紙16.
		{	TYPE_PaperTypeCustom17	,	BTNID_KEY_PAPERKINDCUSTOM_17	},	// カスタマイズ用紙17.
		{	TYPE_PaperTypeCustom18	,	BTNID_KEY_PAPERKINDCUSTOM_18	},	// カスタマイズ用紙18.
		{	TYPE_PaperTypeCustom19	,	BTNID_KEY_PAPERKINDCUSTOM_19	},	// カスタマイズ用紙19.
		{	TYPE_PaperTypeCustom20	,	BTNID_KEY_PAPERKINDCUSTOM_20	},	// カスタマイズ用紙20.
// Comment 2023/09/07 FPT-ThinhND40 [IT6_2.3] [ 機器構成変更_No.55 ] Removal of prohibition on letterhead of additional cassettes.
//		{	TYPE_PaperTypeUnknown	,	0								},	// なし.
		{	TYPE_PaperTypeUnknown	,	0								},	// なし.
		{	TYPE_PaperTypeUnknown	,	0								},	// なし.
		{	TYPE_PaperTypeUnknown	,	0								},	// なし.
		{	TYPE_PaperTypeUnknown	,	0								},	// なし.
		{	TYPE_PaperTypeUnknown	,	0								},	// なし.
		{	TYPE_PaperTypeUnknown	,	0								},	// なし.
		{	TYPE_PaperTypeUnknown	,	0								},	// なし.
		{	TYPE_PaperTypeUnknown	,	0								},	// なし.
		{	TYPE_PaperTypeUnknown	,	0								},	// なし.
		{	TYPE_PaperTypeUnknown	,	0								},	// なし.
		{	TYPE_PaperTypeUnknown	,	0								},	// なし.
		{	TYPE_PaperTypeUnknown	,	0								},	// なし.
	},
#endif // DEF_KRONOSS_Cus.
};
// トレイごとの有効用紙種類数は、getTrayPaperKindBtnFromPaperTypeTblSize()から取得する.
#endif // DEF_A4_Cus.
// A3A4FUM 用紙種類設定 t-okada 2015/01/19 End.


// カスタム用紙種別から、用紙種別に変換するテーブル
const  PNLC_PaperTypeFromCustomPaperType tbl_PaperTypeFromCustomPaperType[] = {
	{	TYPE_CustomPaperType1	,	TYPE_PaperTypeCustom1		},	// カスタマイズ用紙1.
	{	TYPE_CustomPaperType2	,	TYPE_PaperTypeCustom2		},	// カスタマイズ用紙2.
	{	TYPE_CustomPaperType3	,	TYPE_PaperTypeCustom3		},	// カスタマイズ用紙3.
	{	TYPE_CustomPaperType4	,	TYPE_PaperTypeCustom4		},	// カスタマイズ用紙4.
	{	TYPE_CustomPaperType5	,	TYPE_PaperTypeCustom5		},	// カスタマイズ用紙5.
	{	TYPE_CustomPaperType6	,	TYPE_PaperTypeCustom6		},	// カスタマイズ用紙6.
	{	TYPE_CustomPaperType7	,	TYPE_PaperTypeCustom7		},	// カスタマイズ用紙7.
	{	TYPE_CustomPaperType8	,	TYPE_PaperTypeCustom8		},	// カスタマイズ用紙8.
	{	TYPE_CustomPaperType9	,	TYPE_PaperTypeCustom9		},	// カスタマイズ用紙9.
	{	TYPE_CustomPaperType10	,	TYPE_PaperTypeCustom10		},	// カスタマイズ用紙10.
	{	TYPE_CustomPaperType11	,	TYPE_PaperTypeCustom11		},	// カスタマイズ用紙11.
	{	TYPE_CustomPaperType12	,	TYPE_PaperTypeCustom12		},	// カスタマイズ用紙12.
	{	TYPE_CustomPaperType13	,	TYPE_PaperTypeCustom13		},	// カスタマイズ用紙13.
	{	TYPE_CustomPaperType14	,	TYPE_PaperTypeCustom14		},	// カスタマイズ用紙14.
	{	TYPE_CustomPaperType15	,	TYPE_PaperTypeCustom15		},	// カスタマイズ用紙15.
	{	TYPE_CustomPaperType16	,	TYPE_PaperTypeCustom16		},	// カスタマイズ用紙16.
	{	TYPE_CustomPaperType17	,	TYPE_PaperTypeCustom17		},	// カスタマイズ用紙17.
	{	TYPE_CustomPaperType18	,	TYPE_PaperTypeCustom18		},	// カスタマイズ用紙18.
	{	TYPE_CustomPaperType19	,	TYPE_PaperTypeCustom19		},	// カスタマイズ用紙19.
	{	TYPE_CustomPaperType20	,	TYPE_PaperTypeCustom20		},	// カスタマイズ用紙20.
};
const uchar uc_PaperTypeFromCustomPaperTypeNum = ( sizeof( tbl_PaperTypeFromCustomPaperType ) / sizeof( tbl_PaperTypeFromCustomPaperType[0] ) );

// 2015/04/07 D.Suzuki 操作性改善活動 CQ513202対応.
struct st_PaperSizeTable {
	TYPE_SizeCode	e_SizeCode;		// 用紙サイズコード.
	float			f_SizeX;		// Xサイズ.
	float			f_SizeY;		// Yサイズ.
	char			sc_PaperType;	// 用紙種類.
	TEXT_ID			s_TextID;		// 文字.
} tbl_PaperSizeTable[] = {
// A系.B系.
	{	TYPE_SizeCodeSRA3Sef,			320.0f,		450.0f,	PNLD_PaperType_Icon_Metric	,	s_pNOTSheetSize_SRA3_W056H036A		}, ///< SRA3 SEF キー.
	{	TYPE_SizeCodeA3Sef,				297.0f,		420.0f,	PNLD_PaperType_Icon_Metric	,	s_pNOTSheetSize_A3_W056H036A		}, ///< A3 SEF キー.
	{	TYPE_SizeCodeB4Sef,				257.0f,		364.0f,	PNLD_PaperType_Icon_Metric	,	s_pNOTSheetSize_B4_W056H036A		}, ///< B4 SEF キー.
	{	TYPE_SizeCodeA4Lef,				297.0f,		210.0f,	PNLD_PaperType_Icon_Metric	,	s_pNOTSheetSize_A4_W056H036A		}, ///< A4 LEF キー.
	{	TYPE_SizeCodeA4Sef,				210.0f,		297.0f,	PNLD_PaperType_Icon_Metric	,	s_pNOTSheetSize_A4_W056H036A		}, ///< A4 SEF キー.
	{	TYPE_SizeCodeB5Lef,				257.0f,		182.0f,	PNLD_PaperType_Icon_Metric	,	s_pNOTSheetSize_B5_W056H036A		}, ///< B5 LEF キー.
	{	TYPE_SizeCodeB5Sef,				182.0f,		257.0f,	PNLD_PaperType_Icon_Metric	,	s_pNOTSheetSize_B5_W056H036A		}, ///< B5 SEF キー.
	{	TYPE_SizeCodeA5Lef,				210.0f,		148.0f,	PNLD_PaperType_Icon_Metric	,	s_pNOTSheetSize_A5_W056H036A		}, ///< A5 LEF キー.
	{	TYPE_SizeCodeA5Sef,				148.0f,		210.0f,	PNLD_PaperType_Icon_Metric	,	s_pNOTSheetSize_A5_W056H036A		}, ///< A5 SEF キー.
	{	TYPE_SizeCodeB6Sef,				128.0f,		182.0f,	PNLD_PaperType_Icon_Metric	,	s_pNOTSheetSize_B6_W056H036A		}, ///< B6 SEF キー.
	{	TYPE_SizeCodeA6Sef,				105.0f,		148.0f,	PNLD_PaperType_Icon_Metric	,	s_pNOTSheetSize_A6_W056H036A		}, ///< A6 SEF キー.
// inch系.
	{	TYPE_SizeCode12x18Sef,			304.8f,		457.2f,	PNLD_PaperType_Icon_Inch	,	s_pNOTSheetSize_12x18_W072H036A		}, ///< 12  x 18キー.
	{	TYPE_SizeCodeLedgerSef,			279.4f,		431.8f,	PNLD_PaperType_Icon_Inch	,	s_pNOTSheetSize_11x17_W072H036A		}, ///< 11  x 17キー(SEF).
	{	TYPE_SizeCodeLegalSef,			215.9f,		355.6f,	PNLD_PaperType_Icon_Inch	,	s_pNOTSheetSize_812x14_W072H036A	}, ///< 812 x 14キー(SEF).
	{	TYPE_SizeCodeLetterLef,			279.4f,		215.9f,	PNLD_PaperType_Icon_Inch	,	s_pNOTSheetSize_812x11_W072H036A	}, ///< 812 x 11LEFキー.
	{	TYPE_SizeCodeLetterSef,			215.9f,		279.4f,	PNLD_PaperType_Icon_Inch	,	s_pNOTSheetSize_812x11_W072H036A	}, ///< 812 x 11SEFキー.
// Comment 2016/03/25 【HCLT-Ashok】 【CQ526536】 Changing the "7_1/4×10_1/2 SEF = X" and "7_1/4×10_1/2 LEF = Y" size as per Functional Specificaiton.
//	{	TYPE_SizeCodeExecutiveLef,		266.7f,		184.15f,	PNLD_PaperType_Icon_Inch	,	s_pNOTSheetSize_714x1012_W072H036A	}, ///< 714 x 1012LEFキー.
//	{	TYPE_SizeCodeExecutiveSef,		184.15f,		266.7f,	PNLD_PaperType_Icon_Inch	,	s_pNOTSheetSize_714x1012_W072H036A	}, ///< 714 x 1012SEFキー.
	{	TYPE_SizeCodeExecutiveLef,		266.7f,		184.2f,	PNLD_PaperType_Icon_Inch	,	s_pNOTSheetSize_714x1012_W072H036A	}, ///< 714 x 1012LEFキー.
	{	TYPE_SizeCodeExecutiveSef,		184.2f,		266.7f,	PNLD_PaperType_Icon_Inch	,	s_pNOTSheetSize_714x1012_W072H036A	}, ///< 714 x 1012SEFキー.	
	{	TYPE_SizeCodeInvoiceLef,		215.9f,		139.7f,	PNLD_PaperType_Icon_Inch	,	s_pNOTSheetSize_512x812_W072H036A	}, ///< 512 x 812LEFキー.
	{	TYPE_SizeCodeInvoiceSef,		139.7f,		215.9f,	PNLD_PaperType_Icon_Inch	,	s_pNOTSheetSize_512x812_W072H036A	}, ///< 512 x 812SEFキー.
#ifdef DEF_A4_Cus
	{	TYPE_SizeCodeSpFolioSef,		215.9f,		322.3f,	PNLD_PaperType_Icon_Inch	,	s_pNOTSheetSize_SPFOLIO_W072H036A	}, ///< SP Folio(Letter Plus)キー.
	{	TYPE_SizeCodeGovermentLetterSef,203.2f,		266.7f,	PNLD_PaperType_Icon_Inch	,	s_pNOTSheetSize_8x1012_W072H036A	}, ///< 8 x 1012SEFキー.
	{	TYPE_SizeCode8x10Sef,			203.2f,		254.0f,	PNLD_PaperType_Icon_Inch	,	s_pNOTSheetSize_8x10_W072H036A		}, ///< 8 x 10SEFキー.
#endif
// はがき系.
//Comment 2018/11/29 HCLT_Sarada [IT6_DenebMLK][制御構成変更_No.45] hiding post card paper size.
#ifndef DEF_DenebMLKCus
	{	TYPE_SizeCodePostcard_JpSef,	100.0f,		148.0f,	PNLD_PaperType_Icon_Post	,	s_pKEYSheetSize_Postcard_W064H036A	}, ///< はがきSEF.
#endif //DEF_DenebMLKCus.
	{	TYPE_SizeCodeIN4x6Sef,			101.6f,		152.4f,	PNLD_PaperType_Icon_Post	,	s_pNOTSheetSize_4x6_W056H036A		}, ///< 4x6SEF.
//2016/07/15 CQ532469 sky草柳 ★A3共通問題★A6カードのシンボル変更start.
#ifdef DEF_A4_Cus
	{	TYPE_SizeCodePostcard_EuSef,	105.0f,		148.0f,	PNLD_PaperType_Icon_Post	,	s_pKEYSheetSize_A6Card_W080H036A	}, ///< A6カードSEF.
#else
// comment 2017/05/03 [HCL_Sindhu] [IT5_4.1][1016955_Sup 13] Display of A6 card in Japanese.
//	{	TYPE_SizeCodePostcard_EuSef,	105.0f,		148.0f,	PNLD_PaperType_Icon_Post	,	s_pKEYSheetSize_A6Card_W056H036A	}, ///< A6CardSEF.
	{	TYPE_SizeCodePostcard_EuSef,	105.0f,		148.0f,	PNLD_PaperType_Icon_Post	,	s_pKEYSheetSize_A6Card_W080H036A	}, ///< A6CardSEF.
#endif
//2016/07/15 CQ532469 sky草柳 ★A3共通問題★A6カードのシンボル変更end.
#ifdef DEF_A4_Cus
//2016/07/12 [CQ532540]封筒・はがき用アイコン追加 sky草柳 start.
	{	TYPE_SizeCodeDoublePostcard_JpSef,148.0f,		200.0f,	PNLD_PaperType_Icon_DoublePost	,	s_pKEYSheetSize_DoublePostcardSef_W080H036A	}, ///< 往復はがき SEF.
//2016/07/12 [CQ532540]封筒・はがき用アイコン追加 sky草柳 end.
#endif
// 封筒.
#ifdef DEF_A4_Cus
//2016/07/12 [CQ532540]封筒・はがき用アイコン追加 sky草柳 start.
	{	TYPE_SizeCodeEnvelopeB5,		176.0f,		250.0f,	PNLD_PaperType_Icon_LetterSef	,	s_pNOTSheetSize_envB5SEF_W056H036A	}, ///< envB5SEF.
//2016/07/12 [CQ532540]封筒・はがき用アイコン追加 sky草柳 end.
#else
	{	TYPE_SizeCodeEnvelopeB5,		176.0f,		250.0f,	PNLD_PaperType_Icon_Letter	,	s_pNOTSheetSize_envB5_W056H036A		}, ///< envB5SEF.
#endif
	{	TYPE_SizeCodeEnvelopeC4,		229.0f,		324.0f,	PNLD_PaperType_Icon_Letter	,	s_NOTenvC4							}, ///< envC4SEF.
	{	TYPE_SizeCodeEnvelopeC5,		162.0f,		229.0f,	PNLD_PaperType_Icon_Letter	,	s_pNOTSheetSize_envC5_W056H036A		}, ///< envC5SEF.
//Comment 2018/04/23 【HCLT-Chinna】【IT6_Sp】【AR-1044433】Paper size icon changed LEF to SEF
#if defined ( DEF_KRONOSS_Cus ) || defined ( DEF_SparrowCus ) 
	{	TYPE_SizeCodeEnvelopeDL,		110.0f,		220.0f,	PNLD_PaperType_Icon_Letter	,	s_pNOTSheetSize_envDLSEF_W056H036A	}, ///< envDLSEF.
#else // DEF_KRONOSS_Cus.
	{	TYPE_SizeCodeEnvelopeDL,		110.0f,		220.0f,	PNLD_PaperType_Icon_Letter	,	s_pNOTSheetSize_envDL_W056H036A		}, ///< envDLSEF.
#endif //#if defined ( DEF_KRONOSS_Cus ) || defined ( DEF_SparrowCus ) 
// Comment 2016/03/28 【HCLT-Ashok】 【CQ526536】 Changing the "envCom10 = Y" and "envMonarch = X" size as per Functional Specificaiton.
//	{	TYPE_SizeCodeEnvelopeCom10,		104.775f,	241.3f,	PNLD_PaperType_Icon_Letter	,	s_pNOTenvCom10_W096H028A			}, ///< envCom10.
//	{	TYPE_SizeCodeEnvelopeMornarch,	98.425f,		190.5f,	PNLD_PaperType_Icon_Letter	,	s_pNOTenvMonarch_W096H028A			}, ///< envMonarch.
#ifdef DEF_A4_Cus
//2016/07/12 [CQ532540]封筒・はがき用アイコン追加 sky草柳 start.
	{	TYPE_SizeCodeEnvelopeCom10,		104.8f,		241.3f,	PNLD_PaperType_Icon_LetterCom10Sef	,	s_pNOTenvCom10SEF_W096H028A	}, ///< envCom10.
//2016/07/12 [CQ532540]封筒・はがき用アイコン追加 sky草柳 end.
#else
	{	TYPE_SizeCodeEnvelopeCom10,		104.8f,		241.3f,	PNLD_PaperType_Icon_Letter	,	s_pNOTenvCom10_W096H028A			}, ///< envCom10.
#endif
//Comment 2018/04/23 【HCLT-Chinna】【IT6_Sp】【AR-1044433】Paper size icon changed LEF to SEF
#if defined ( DEF_KRONOSS_Cus ) || defined ( DEF_SparrowCus ) 
	{	TYPE_SizeCodeEnvelopeMornarch,	98.4f,		190.5f,	PNLD_PaperType_Icon_Letter	,	s_pNOTenvMonarchSEF_W096H028A		}, ///< envMonarch.
	{	TYPE_SizeCodeJpLetterY2,		114.0f,		162.0f,	PNLD_PaperType_Icon_Env		,	s_pKEYEnvY2SEF_W096H028A			}, ///< env洋形2号.
	{	TYPE_SizeCodeJpLetterY3,		 98.0f,		148.0f,	PNLD_PaperType_Icon_Env		,	s_pKEYEnvY3SEF_W096H028A			}, ///< env洋形3号.
#else // DEF_KRONOSS_Cus.
	{	TYPE_SizeCodeEnvelopeMornarch,	98.4f,		190.5f,	PNLD_PaperType_Icon_Letter	,	s_pNOTenvMonarch_W096H028A			}, ///< envMonarch.
	{	TYPE_SizeCodeJpLetterY2,		114.0f,		162.0f,	PNLD_PaperType_Icon_Env		,	s_pKEYEnvY2_W096H028A				}, ///< env洋形2号.
	{	TYPE_SizeCodeJpLetterY3,		 98.0f,		148.0f,	PNLD_PaperType_Icon_Env		,	s_pKEYEnvY3_W096H028A				}, ///< env洋形3号.
#endif // #if defined ( DEF_KRONOSS_Cus ) || defined ( DEF_SparrowCus ) 
//Comment 2018/04/23 【HCLT-Chinna】【IT6_Sp】【AR-1044433】Paper size icon changed LEF to SEF
#ifdef DEF_SparrowCus
	{	TYPE_SizeCodeJpLetterY4,		105.0f,		235.0f,	PNLD_PaperType_Icon_Env		,	s_pKeyY4SEF_W096H028A				}, ///< env洋形4号.
#else	
	{	TYPE_SizeCodeJpLetterY4,		105.0f,		235.0f,	PNLD_PaperType_Icon_Env		,	s_pKEYEnvY4_W096H028A				}, ///< env洋形4号.
#endif // DEF_SparrowCus
#ifdef DEF_A4_Cus
//2016/07/12 [CQ532540]封筒・はがき用アイコン追加 sky草柳 start.
	{	TYPE_SizeCodeJpLetterL3,		120.0f,		235.0f,	PNLD_PaperType_Icon_LetterSef	,	s_pKEYEnvL3SEF_W096H028A			}, ///< env長形3号.
	{	TYPE_SizeCodeJpLetterL4,		 90.0f,		205.0f,	PNLD_PaperType_Icon_LetterSef	,	s_pKEYEnvL4SEF_W096H028A			}, ///< env長形4号.
//2016/07/12 [CQ532540]封筒・はがき用アイコン追加 sky草柳 end.
#else
	{	TYPE_SizeCodeJpLetterL3,		120.0f,		235.0f,	PNLD_PaperType_Icon_Letter	,	s_pKEYEnvL3_W096H028A				}, ///< env長形3号.
	{	TYPE_SizeCodeJpLetterL4,		 90.0f,		205.0f,	PNLD_PaperType_Icon_Letter	,	s_pKEYEnvL4_W096H028A				}, ///< env長形4号.
#endif
	{	TYPE_SizeCodeJpLetterK1,		270.0f,		382.0f,	PNLD_PaperType_Icon_Letter	,	s_pKEYEnvK1_W096H028A				}, ///< env角形1号.
	{	TYPE_SizeCodeJpLetterK2,		240.0f,		332.0f,	PNLD_PaperType_Icon_Letter	,	s_pKEYEnvK2_W096H028A				}, ///< env角形2号.
#ifdef DEF_A4_Cus
	{	TYPE_SizeCodeJpLetterK3,		216.0f,		277.0f,	PNLD_PaperType_Icon_Letter	,	s_pKEYEnvK3SEF_W096H028A			}, ///< env角形3号.
#else
	{	TYPE_SizeCodeJpLetterK3,		216.0f,		277.0f,	PNLD_PaperType_Icon_Letter	,	s_pKEYEnvK3_W096H028A				}, ///< env角形3号.
#endif
#ifdef DEF_A4_Cus
// [CQ525608] 用紙サイズ選択アイコン OPS2 Sky西場 2016/04/14 start.
	{	TYPE_SizeCodeJpLetterY2Lef,		162.0f,		114.0f,	PNLD_PaperType_Icon_EnvLef		,	s_pKEYEnvY2LEF_W096H028A			}, ///< env洋形2号LEF.
	{	TYPE_SizeCodeJpLetterY3Lef,		148.0f,		 98.0f,	PNLD_PaperType_Icon_EnvLef		,	s_pKEYEnvY3LEF_W096H028A			}, ///< env洋形3号LEF.
	{	TYPE_SizeCodeEnvelopeDLLef,		220.0f,		110.0f,	PNLD_PaperType_Icon_LetterLef	,	s_pNOTSheetSize_envDLLEF_W056H036A	}, ///< envDLキー.
	{	TYPE_SizeCodeEnvelopeMornarchLef,190.5f,	98.425f,	PNLD_PaperType_Icon_LetterLef	,	s_pNOTenvMonarchLEF_W096H028A		}, ///< envMonarchLEFキー.
// [CQ525608] 用紙サイズ選択アイコン OPS2 Sky西場 2016/04/14 end.
#endif
// 8K16K.
	{	TYPE_SizeCodeFlsSef,			203.2f,		330.2f,	PNLD_PaperType_Icon_Inch	,	s_pNOT8x13_W080H028A				}, ///< FoolsCap.
	{	TYPE_SizeCode8KSef,				270.0f,		390.0f,	PNLD_PaperType_Icon_ETC		,	s_pNOTSheetSize_8K_W056H036A		}, ///< 8KSEF.
	{	TYPE_SizeCode16KLef,			270.0f,		195.0f,	PNLD_PaperType_Icon_ETC		,	s_pNOTSheetSize_16K_W056H036A		}, ///< 16KLEF.
	{	TYPE_SizeCode16KSef,			195.0f,		270.0f,	PNLD_PaperType_Icon_ETC		,	s_pNOTSheetSize_16K_W056H036A		}, ///< 16KSEF.
#ifdef DEF_A4_Cus
	{	TYPE_SizeCode32KSef,			130.0f,		185.0f,	PNLD_PaperType_Icon_ETC		,	s_pNOTSheetSize_32K_W056H036A		}, ///< 32KSEF.
#endif
};
const uchar uc_SizeCount = sizeof( tbl_PaperSizeTable ) / sizeof( st_PaperSizeTable );

// comment 2016/05/19 SCD2-2G-Sky福田 [ZeusSBK][CQ529248] TBD230対応.
// Local側の実装はGUIから持ってくることができなかったのでテーブル作成.
struct PNLS_PaperEnl_copyReplaceTable{
	TYPE_SizeCode	e_SizeCode;					// 検索用紙サイズ.
	ushort			us_DstTextID;				// 置き換え用紙サイズ.
	ushort			us_DstDirTextID;			// 置き換え用紙サイズ向き.
	short			ss_PosX_Offset;				// 用紙サイズ向きX座標オフセット.
};

const PNLS_PaperEnl_copyReplaceTable st_ReplacePaperSizeTable[] = {
	{	TYPE_SizeCodeSRA3Sef,					s_pNOTSheetSize_SRA3_W056H036A,					s_pNOTSef_W036H036A,	40	},
	{	TYPE_SizeCodeA3Sef,						s_pNOTSheetSize_A3_W056H036A,					s_pNOTSef_W036H036A,	20	},
	{	TYPE_SizeCodeA4Sef,						s_pNOTSheetSize_A4_W056H036A,					s_pNOTSef_W036H036A,	20	},
	{	TYPE_SizeCodeA4Lef,						s_pNOTSheetSize_A4_W056H036A,					s_pNOTLef_W036H036A,	20	},
	{	TYPE_SizeCodeA5Sef,						s_pNOTSheetSize_A5_W056H036A,					s_pNOTSef_W036H036A,	20	},
	{	TYPE_SizeCodeA5Lef,						s_pNOTSheetSize_A5_W056H036A,					s_pNOTLef_W036H036A,	20	},
	{	TYPE_SizeCodeA6Sef,						s_pNOTSheetSize_A6_W056H036A,					s_pNOTSef_W036H036A,	20	},
	{	TYPE_SizeCodeB4Sef,						s_pNOTSheetSize_B4_W056H036A,					s_pNOTSef_W036H036A,	20	},
	{	TYPE_SizeCodeB5Sef,						s_pNOTSheetSize_B5_W056H036A,					s_pNOTSef_W036H036A,	20	},
	{	TYPE_SizeCodeB5Lef,						s_pNOTSheetSize_B5_W056H036A,					s_pNOTLef_W036H036A,	20	},
	{	TYPE_SizeCodeB6Sef,						s_pNOTSheetSize_B6_W056H036A,					s_pNOTSef_W036H036A,	20	},
//Comment 2018/11/29 HCLT_Sarada [IT6_DenebMLK][制御構成変更_No.45] hiding post card paper size.
#ifndef DEF_DenebMLKCus
	{	TYPE_SizeCodePostcard_JpSef,			s_pKEYSheetSize_Postcard_W064H036A,				0,						0	},
	{	TYPE_SizeCodePostcard_JpLef,			s_pKEYSheetSize_Postcard_W064H036A,				0,						0	},
#endif //DEF_DenebMLKCus
	{	TYPE_SizeCodePostcard_EuSef,			s_pKEYSheetSize_A6Card_W080H036A,				0,						0	},
	{	TYPE_SizeCodePostcard_EuLef,			s_pKEYSheetSize_A6Card_W080H036A,				0,						0	},
	{	TYPE_SizeCodeLedgerSef, 				s_pNOTSheetSize_11x17_W072H036A,				s_pNOTSef_W036H036A,	62	},
	{	TYPE_SizeCodeLegalSef,					s_pNOTSheetSize_812x14_W072H036A,				s_pNOTSef_W036H036A,	64	},
	{	TYPE_SizeCodeLetterSef, 				s_pNOTSheetSize_812x11_W072H036A,				s_pNOTSef_W036H036A,	62	},
	{	TYPE_SizeCodeLetterLef, 				s_pNOTSheetSize_812x11_W072H036A,				s_pNOTLef_W036H036A,	62	},
	{	TYPE_SizeCodeInvoiceSef, 				s_pNOTSheetSize_512x812_W072H036A,				s_pNOTSef_W036H036A,	66	},
	{	TYPE_SizeCodeInvoiceLef, 				s_pNOTSheetSize_512x812_W072H036A,				s_pNOTLef_W036H036A,	66	},
	{	TYPE_SizeCodeExecutiveSef, 				s_pNOTSheetSize_714x1012_W072H036A,				s_pNOTSef_W036H036A,	66	},
	{	TYPE_SizeCodeExecutiveLef, 				s_pNOTSheetSize_714x1012_W072H036A,				s_pNOTLef_W036H036A,	66	},
	{	TYPE_SizeCodeIN4x6Sef,					s_pNOTSheetSize_4x6_W072H036A,					0,						0	},
	{	TYPE_SizeCodeIN4x6Lef,					s_pNOTSheetSize_4x6_W072H036A,					0,						0	},
	{	TYPE_SizeCode8KSef, 					s_pNOTSheetSize_8K_W056H036A,					s_pNOTSef_W036H036A,	30	},
	{	TYPE_SizeCode16KSef, 					s_pNOTSheetSize_16K_W056H036A,					s_pNOTSef_W036H036A,	30	},
	{	TYPE_SizeCode16KLef, 					s_pNOTSheetSize_16K_W056H036A,					s_pNOTLef_W036H036A,	30	},
	{	TYPE_SizeCode12x18Sef,					s_pNOTSheetSize_12x18_W072H036A,				s_pNOTSef_W036H036A,	62	},
#ifndef DEF_A4_Cus		// Comment VenusMLK -> 2016/08/30 Sky高田(庸)[CQ533590]SEF/LEFアイコン追加.
	{	TYPE_SizeCodeEnvelopeMornarch,			s_pNOTenvMonarch_W096H028A,						0,						0	},
	{	TYPE_SizeCodeEnvelopeCom10,				s_pNOTenvCom10_W096H028A,						0,						0	},
	{	TYPE_SizeCodeEnvelopeDL,				s_pNOTSheetSize_envDL_W056H036A,				0,						0	},
#else	// DEF_A4_Cus	// Comment VenusMLK <- 2016/08/30 Sky高田(庸)[CQ533590]SEF/LEFアイコン追加.
#if defined ( DEF_KRONOSS_Cus ) || defined ( DEF_SparrowCus ) // 2017/12/21 HCLT-Vandana Modification for [IT6_Sp][機器構成変更_No.282]
	{	TYPE_SizeCodeEnvelopeMornarch,			s_pNOTenvMonarchSEF_W096H028A,					0,						0	},
#else // defined ( DEF_KRONOSS_Cus ) || defined ( DEF_SparrowCus )
	{	TYPE_SizeCodeEnvelopeMornarch,			s_pNOTenvMonarchLEF_W096H028A,					0,						0	},
#endif // defined ( DEF_KRONOSS_Cus ) || defined ( DEF_SparrowCus )
	{	TYPE_SizeCodeEnvelopeCom10,				s_pNOTenvCom10SEF_W096H028A,					0,						0	},
#if defined ( DEF_KRONOSS_Cus ) || defined ( DEF_SparrowCus ) // 2017/12/21 HCLT-Vandana Modification for [IT6_Sp][機器構成変更_No.282]
	{	TYPE_SizeCodeEnvelopeDL,				s_pNOTSheetSize_envDLSEF_W056H036A,				0,						0	},
#else // defined ( DEF_KRONOSS_Cus ) || defined ( DEF_SparrowCus )
	{	TYPE_SizeCodeEnvelopeDL,				s_pNOTSheetSize_envDLLEF_W056H036A,				0,						0	},
#endif // defined ( DEF_KRONOSS_Cus ) || defined ( DEF_SparrowCus )
#endif	// DEF_A4_Cus	// Comment VenusMLK <- 2016/08/30 Sky高田(庸)[CQ533590]SEF/LEFアイコン追加.
	{	TYPE_SizeCodeEnvelopeC5,				s_pNOTenvC5_W080H028A,							0,						0	},
#ifndef DEF_A4_Cus		// Comment VenusMLK -> 2016/08/30 Sky高田(庸)[CQ533590]SEF/LEFアイコン追加.
	{	TYPE_SizeCodeEnvelopeB5,				s_pNOTSheetSize_envB5_W056H036A,				0,						0	},
#else	// DEF_A4_Cus	// Comment VenusMLK <- 2016/08/30 Sky高田(庸)[CQ533590]SEF/LEFアイコン追加.
	{	TYPE_SizeCodeEnvelopeB5,				s_pNOTSheetSize_envB5SEF_W056H036A,				0,						0	},
#endif	// DEF_A4_Cus	// Comment VenusMLK <- 2016/08/30 Sky高田(庸)[CQ533590]SEF/LEFアイコン追加.
	{	TYPE_SizeCodeEnvelopeC4,				s_NOTenvC4,										0,						0	},
#if defined ( DEF_KRONOSS_Cus ) || defined ( DEF_SparrowCus ) // 2017/12/21 HCLT-Vandana Modification for [IT6_Sp][機器構成変更_No.282]
	{	TYPE_SizeCodeJpLetterY2,				s_pNOTenvC6SEF_W056H028A,						0,						0	},
	{	TYPE_SizeCodeJpLetterY3,				s_pKEYEnvY3SEF_W096H028A,						0,						0	},
#else // defined ( DEF_KRONOSS_Cus ) || defined ( DEF_SparrowCus )
	{	TYPE_SizeCodeJpLetterY2,				s_NOTenvC6,										0,						0	},
	{	TYPE_SizeCodeJpLetterY3,				s_pKEYEnvY3_W096H028A,							0,						0	},
#endif // defined ( DEF_KRONOSS_Cus ) || defined ( DEF_SparrowCus )
//Comment 2018/04/13 【HCLT-Chinna】【IT6_Sp】【機器構成変更_No.364】Paper size input support added on the panel
#ifdef DEF_SparrowCus
	{ 	TYPE_SizeCodeJpLetterY4,  				s_pKeyY4SEF_W096H028A,  						0,  					0  	},
#else
	{	TYPE_SizeCodeJpLetterY4,				s_pKEYEnvY4_W096H028A,							0,						0	},
#endif //DEF_SparrowCus
#ifndef DEF_A4_Cus		// Comment VenusMLK -> 2016/08/30 Sky高田(庸)[CQ533590]SEF/LEFアイコン追加.
	{	TYPE_SizeCodeJpLetterL3,				s_pKEYEnvL3_W096H028A,							0,						0	},
	{	TYPE_SizeCodeJpLetterL4,				s_pKEYEnvL4_W096H028A,							0,						0	},
#else	// DEF_A4_Cus	// Comment VenusMLK <- 2016/08/30 Sky高田(庸)[CQ533590]SEF/LEFアイコン追加.
	{	TYPE_SizeCodeJpLetterL3,				s_pKEYEnvL3SEF_W096H028A,						0,						0	},
	{	TYPE_SizeCodeJpLetterL4,				s_pKEYEnvL4SEF_W096H028A,						0,						0	},
#endif	// DEF_A4_Cus	// Comment VenusMLK <- 2016/08/30 Sky高田(庸)[CQ533590]SEF/LEFアイコン追加.
	{	TYPE_SizeCodeJpLetterK1,				s_pKEYEnvK1_W096H028A,							0,						0	},
	{	TYPE_SizeCodeJpLetterK2,				s_pKEYEnvK2_W096H028A,							0,						0	},
	{	TYPE_SizeCodeJpLetterK3,				s_pKEYEnvK3_W096H028A,							0,						0	},
#ifdef DEF_A4_Cus
	{	TYPE_SizeCodeDoublePostcard_JpSef,		s_pKEYSheetSize_DoublePostcardSef_W080H036A,	0,						0	},
	{	TYPE_SizeCodeGovermentLetterSef,		s_pNOTSheetSize_8x1012_W072H036A,				s_pNOTSef_W036H036A,	66	},		// Comment 2016/08/25 Sky荻堂 VenusMLK 【CQ533599】オフセット値を48→66に変更.
	{	TYPE_SizeCode8x10Sef,					s_pNOTSheetSize_8x10_W072H036A,					s_pNOTSef_W036H036A,	66	},		// Comment 2016/08/25 Sky荻堂 VenusMLK 【CQ533599】オフセット値を48→66に変更.
	{	TYPE_SizeCodeSpFolioSef,				s_pNOTSheetSize_SPFOLIO_W072H036A,				s_pNOTSef_W036H036A,	66	},		// Comment 2016/08/25 Sky荻堂 VenusMLK 【CQ533599】オフセット値を48→66に変更.
	{	TYPE_SizeCode32KSef,					s_pNOTSheetSize_32K_W056H036A,					s_pNOTSef_W036H036A,	48	},
	{	TYPE_SizeCodeEnvelopeMornarchLef,		s_pNOTenvMonarch_W096H028A,						0,						0	},
	{	TYPE_SizeCodeEnvelopeDLLef,				s_pNOTSheetSize_envDL_W056H036A,				0,						0	},
	{	TYPE_SizeCodeJpLetterY2Lef,				s_pKEYEnvY2LEF_W096H028A,						0,						0	},		// Comment VenusMLK <- 2016/08/30 Sky高田(庸)[CQ533590]SEF/LEFアイコン追加.
	{	TYPE_SizeCodeJpLetterY3Lef,				s_pKEYEnvY3LEF_W096H028A,						0,						0	},		// Comment VenusMLK <- 2016/08/30 Sky高田(庸)[CQ533590]洋形3LEFの追加.
#endif  // DEF_A4_Cus.
	{	TYPE_SizeCodeUnknown,					s_Dummy,										0,						0	},	// 未定・不明.
};

#ifdef DEF_LCD_7INCH
#define	PNLD_PaperEnl_PaperSizeDir_CorrectX				(4)
#else
#define	PNLD_PaperEnl_PaperSizeDir_CorrectX				(0)
#endif

// comment 2017/12/27 SCD2-4G Sky平田 [Eagle][RQ-720]紙種自動検知.
static const int32_t i32_A020114100_PaperTypeSelectList_PosX	= 30;
static const int32_t i32_A020114100_PaperTypeSelectList_PosY	= 100 + 38;
static const int32_t i32_A020114100_PaperTypeSelectList_Width	= 368;
static const int32_t i32_A020114100_PaperTypeSelectList_Height	= 262 - 38;
static const int32_t i32_A020114100_PaperTypeScrollArea_PosX	= 0;
static const int32_t i32_A020114100_PaperTypeScrollArea_PosY	= 40;
static const int32_t i32_A020114100_PaperTypeScrollArea_Width	= 368;
static const int32_t i32_A020114100_PaperTypeScrollArea_Height	= 222 - 38;

// comment 2017/06/16 SCD2-Sky後藤 [IT5_4.1][AR1024210] 不定形サイズ可能トレイの印刷出来るサイズ範囲外の用紙を設定できない処理を追加.
// A020114130_用紙ワイド紙(定形/不定形) : メッセージ切り替え用テーブル.
struct PNLS_PaperChangeMessageTable{
	Bool			bl_Fixed;					// 定形可能トレイ.
	Bool			bl_Auto;					// 自動検出判定.
	Bool			bl_SizeNonStandard;			// 12_1/4ｘ18判定.
	Bool			bl_AutoResult;				// 自動検出結果.
	Bool			bl_SizeSelect;				// サイズキー選択.
	ushort			us_MSGTextID;				// メッセージシンボル.
};

const PNLS_PaperChangeMessageTable st_PaperChangeMessageTable[] = {
// 不定形サイズ可能トレイ.
	{	True,	True,	False,	True,	True,	s_pMSGPushSizeChangeKeyFromBesidesFixedSize_W680H036A	},  // 定形サイズ以外の用紙に印刷する場合は[サイズ変更]キーを押下してください。.
	{	True,	True,	False,	True,	False, 	s_pMSGPrinTraySetAndChooseWidePaper_W680H036A			},  // 印刷したい用紙をトレイにセットしてから中央に配置する画像サイズを、ワイド紙から選択してください。.
	{	True,	False,	True,	True,	True,	s_pMSGPrinTraySetAndChooseWidePaper_W680H036A			},  // 印刷したい用紙をトレイにセットしてから中央に配置する画像サイズを、ワイド紙から選択してください。.
	{	True,	False,	True,	True,	False,	s_pMSGPrinTraySetAndChooseWidePaper_W680H036A			},  // 印刷したい用紙をトレイにセットしてから中央に配置する画像サイズを、ワイド紙から選択してください。.
	{	True,	False,	False,	True,	True,	s_pMSGPushSizeChangeKeyFromCheckPaperSize_W680H036A		},  // 用紙サイズを確認し、設定を変更する場合は[サイズ変更]キーを押下してください。.
	{	True,	False,	False,	False,	True,	s_pMSGPushSizeChangeKeyFromCheckPaperSize_W680H036A		},  // 用紙サイズを確認し、設定を変更する場合は[サイズ変更]キーを押下してください。.
// 定形サイズ可能トレイ.
	{	False,	True,	False,	True,	True,	s_pMSGPrinTraySetAndChooseWidePaper_W680H036A			},  // 印刷したい用紙をトレイにセットしてから中央に配置する画像サイズを、ワイド紙から選択してください。.
	{	False,	True,	False,	True,	False, 	s_pMSGPrinTraySetAndChooseWidePaper_W680H036A			},  // 印刷したい用紙をトレイにセットしてから中央に配置する画像サイズを、ワイド紙から選択してください。.
	{	False,	False,	True,	True,	True,	s_pMSGPrinTraySetAndChooseWidePaper_W680H036A			},  // 印刷したい用紙をトレイにセットしてから中央に配置する画像サイズを、ワイド紙から選択してください。.
	{	False,	False,	True,	True,	False,	s_pMSGPrinTraySetAndChooseWidePaper_W680H036A			},  // 印刷したい用紙をトレイにセットしてから中央に配置する画像サイズを、ワイド紙から選択してください。.
};
// テーブルサイズ.
const uchar uc_PaperChangeMessageTable = ( uchar )( sizeof( st_PaperChangeMessageTable ) / sizeof( PNLS_PaperChangeMessageTable ) );


// A020114130_用紙ワイド紙(手差し) : メッセージ切り替え用テーブル.
struct PNLS_PaperChangeMessageManualTable{
	Bool			bl_AutoResult;				// 自動検出結果.
	Bool			bl_SizeSelect;				// サイズキー選択.
	ushort			us_MSGTextID;				// メッセージシンボル.
};

const PNLS_PaperChangeMessageManualTable st_PaperChangeMessageManualTable[] = {
	{	True,	True,	s_pMSGPushSizeChangeKeyFromCheckPaperSize_W680H036A		},	
	{	True,	False, 	s_pMSGPrinTraySetAndChooseWidePaper_W680H036A			},
	{	False,	True,	s_pMSGPushSizeChangeKeyFromCheckPaperSize_W680H036A		},	
	{	False,	False, 	s_pMSGPrinTraySetAndChooseWidePaper_W680H036A			},
};
// テーブルサイズ.
const uchar uc_PaperChangeMessageManualTable = ( uchar )( sizeof( st_PaperChangeMessageManualTable ) / sizeof( PNLS_PaperChangeMessageManualTable ) );

/*================================================================================================
 マクロ関数の定義.
================================================================================================*/
const uchar uc_SizePaperKindNewTbl = ( uchar )( sizeof( st_PaperKindNewTbl ) / sizeof( TYPS_PaperKindInfo ) );													// 手差し以外.
//Comment [IT6-Eagle] 機器構成変更No.2 HCLT-Deeparaj 2018/02/20 Get the size of the table.
#ifndef DEF_A4_Cus
const uchar uc_SizePaperKindNewTblTry1 = ( uchar )( sizeof( st_PaperKindNewTblTry1 ) / sizeof( TYPS_PaperKindInfo ) );													// 手差し以外.
#endif	//DEF_A4_Cus.
// 用紙種類ボタンインデックステーブルサイズ.
const uchar uc_SizeNewPaperTypeBtnStatus = ( uchar )( sizeof( uc_AryPaperTypeBtnStatusTbl ) / sizeof( uc_AryPaperTypeBtnStatusTbl[0] ) );
//Comment [IT6-Eagle] 機器構成変更No.2 HCLT-Deeparaj 2018/02/20 Get the size of the table.
#ifndef DEF_A4_Cus
const uchar uc_SizeNewPaperTypeBtnStatusTry1 = ( uchar )( sizeof( uc_AryPaperTypeBtnStatusTblTry1 ) / sizeof( uc_AryPaperTypeBtnStatusTblTry1[0] ) );
#endif	//DEF_A4_Cus.
//-----------------------------------------------------------------------------------------------
/**
 *	@brief	A020114110 用紙定形サイズ  : コンストラクタ.
 */
//-----------------------------------------------------------------------------------------------
PCPC_PaperSizeMetric_copy::PCPC_PaperSizeMetric_copy(
    PDisplayParts**			partsTable,
    short					partsNumber,
    Bool					isNoWarningGamen,
    PPopupAnimationGroup*	pc_AnimationComponent,
    short					ss_PosX,
    short					ss_PosY,
    short					ss_SizeX,
    short					ss_SizeY,
    PDisplayParts*			pc_FunctionBadgeButton
)
	: PPopupDecisionButtonCom( partsTable, partsNumber, isNoWarningGamen, pc_AnimationComponent, ss_PosX, ss_PosY, ss_SizeX, ss_SizeY, pc_FunctionBadgeButton )
{
	m_SystemType         = TYPE_SystemType_Size;
	m_SelectTabKeyOption = -1;
	m_SelectPaperSize    = TYPE_SizeCodeUnknown;
// Comment 2020/04/18 [HCL_Prasanjeet][IT6_EagLeZ+][OP_BTS-22867] paper type canceletion.
    m_bl_Postcard = False;
	uc_page = 0;
}

//-----------------------------------------------------------------------------------------------
/**
 *	@brief	A020114110 用紙定形サイズ  : 画面開始処理.
 *	備考	JobSpecより現在の用紙サイズを取得し、各パラメータを設定する.
 */
//-----------------------------------------------------------------------------------------------
void PCPC_PaperSizeMetric_copy::startGamen()
{
	GFWS_GraphicTable st_ABSizesTable;
	GFWS_GraphicTable st_InchSizesTabTable;

	//A系B系グラフィック.
	st_ABSizesTable.st_Normal.puc_Parts = ( unsigned char* )i00_192_000_n_X24_Y24;
	st_ABSizesTable.st_Normal.uc_PaletteType =             PNLD_PaletteCommon;
	st_ABSizesTable.st_Select.puc_Parts = ( unsigned char* )i00_192_000_n_X24_Y24;
	st_ABSizesTable.st_Select.uc_PaletteType =             PNLD_PaletteCommon;
	st_ABSizesTable.st_Disable.puc_Parts = ( unsigned char* )NULL;
	st_ABSizesTable.st_Disable.uc_PaletteType =            PNLD_PaletteCommon;
	//インチ系グラフィック.
	st_InchSizesTabTable.st_Normal.puc_Parts = ( unsigned char* )i00_191_000_n_X24_Y24;
	st_InchSizesTabTable.st_Normal.uc_PaletteType =             PNLD_PaletteCommon;
	st_InchSizesTabTable.st_Select.puc_Parts = ( unsigned char* )i00_191_000_n_X24_Y24;
	st_InchSizesTabTable.st_Select.uc_PaletteType =             PNLD_PaletteCommon;
	st_InchSizesTabTable.st_Disable.puc_Parts = ( unsigned char* )NULL;
	st_InchSizesTabTable.st_Disable.uc_PaletteType =            PNLD_PaletteCommon;

	PPopupDecisionButtonCom::startGamen();	// 親クラスのstart画面呼び出し.

	m_SystemType = PnlComGetDestination();	// 仕向け取得.


	if( ( NULL == PnlComGetJobCopy() ) ||
	    ( NULL == PnlComGetJobCopy()->getJobSpec() ) ||
	    ( NULL == PnlComGetJobCopy()->getJobSpec()->pc_Print ) ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "[PCPC_PaperSizeMetric_copy::startGamen]Assert Failed!" );
		return;
	}
	// JobSpecより用紙サイズ取得.
	APIC_Job* pc_Job 			= PnlComGetJobCopy();
	if( pc_Job == NULL ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return;
	}
	TYPC_Print* pc_Print		= pc_Job->getJobSpec()->pc_Print;
	TYPE_FeedTray e_FeedTray	= pc_Print->c_PaperTray.getFeedTray();
	APIC_FeedTray* pc_FeedTray	= PnlComGetFeedTray();
	TYPC_PaperSize c_PaperSize	= pc_FeedTray->getPaperSize( e_FeedTray );
// comment 2020/01/17 Sky東島 [EagleBK][OP_BTS-12665].
	correctNonStandardPaperSize( &c_PaperSize , e_FeedTray );

// 用紙サイズ選択処理の返却値が必ずTrueになる仕様に変わったため、分岐処理を削除.
	// 画面遷移時用紙サイズ選択処理.
//	Bool b_Enable = Cmp_PaperSizeSelectList.setSelectPaperSize( c_PaperSize.e_SizeCode, PNLE_PaperSizeSelect_MoveNoAnimation );
	Cmp_PaperSizeSelectList.setSelectPaperSize( c_PaperSize.e_SizeCode, PNLE_PaperSizeSelect_MoveNoAnimation );

//	if( False == b_Enable ) {		// 用紙サイズ選択処理に失敗した場合、遷移時に押下された用紙サイズグループで表示する.
	PNLC_CopyDataLib* pc_CopyDataLib = PNLC_CopyDataLib::newInstance();
	if( NULL == pc_CopyDataLib ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "[PCPC_PaperEnl_copy::procButtonOn]Assert Failed!" );
		return;
	}

	// 遷移時の押下された用紙サイズグループを取得する.
	PNLE_CopyPaperTypeButton e_CopyPaperTypeButton = pc_CopyDataLib->getCopyPaperTypeButton();
	PNLE_PaperSizeGroup e_SelectedPaperSizeGroup = PNLE_PaperSizeGroup_Metric;

	switch( e_CopyPaperTypeButton ) {
		case PNLD_ButtonPaperKindEnvelope:
			// 封筒／はがき系.
			e_SelectedPaperSizeGroup = PNLE_PaperSizeGroup_Envelope;

			break;
		case PNLD_ButtonPaperKindOther:
			// その他(8K16K)系.
			e_SelectedPaperSizeGroup = PNLE_PaperSizeGroup_8K16K;

			break;
		case PNLD_ButtonPaperKindAB:
			// A系B系(メトリック).
			e_SelectedPaperSizeGroup = PNLE_PaperSizeGroup_Metric;

			break;
		case PNLD_ButtonPaperKindInch:
			// インチ系.
			e_SelectedPaperSizeGroup = PNLE_PaperSizeGroup_Inch;

			break;
		default:
			break;
	}

	//用紙サイズグループを選択状態に変更する.
	Cmp_PaperSizeSelectList.setSelectPaperSizeGroup( e_SelectedPaperSizeGroup, PNLE_PaperSizeSelect_MoveNoAnimation );
//	}



	// メッセージエリアコンポーネント設定.
	Cmp_STPSize_PopupMessageArea.setMode( PNLE_PopupMessageAreaMessageModeCopy );	// モード：コピー.
	Cmp_STPSize_PopupMessageArea.setNowPopupGamen( PNLE_CopyPaperFixSize );			// 画面定義ID：A020114110 用紙定形サイズ.
	Cmp_STPSize_PopupMessageArea.setOnOffButton( False );							// ON/OFFボタン非表示.
	Cmp_STPSize_PopupMessageArea.showDestinationCount( False );						// 件数非表示.
}

//-----------------------------------------------------------------------------------------------
/**
 *	@brief	A020114110 用紙定形サイズ  : 表示部品のセットアップ.
 */
//-----------------------------------------------------------------------------------------------
void PCPC_PaperSizeMetric_copy::setupGamen( short option )
{
	// 親クラスのsetupGamen呼び出し.
	PPopupDecisionButtonCom::setupGamen( option );

// 2015/04/07 D.Suzuki 操作性改善活動 CQ513202対応.
	uchar uc_LoopCount = 0;
//	Bool bl_IconBak = False;
	Bool bl_PaperSize = False;
	char sc_PaperIcon = PNLD_PaperType_Icon_Metric;
	
	TYPC_Point<float> c_SizePoint;
	TYPE_SizeCode e_SizeCode = Cmp_PaperSizeSelectList.getSelectedPaperSize();
	// 選択されている用紙を検索する.
	for( uc_LoopCount = 0; uc_LoopCount < uc_SizeCount; uc_LoopCount++ ) {
		if( e_SizeCode == tbl_PaperSizeTable[uc_LoopCount].e_SizeCode ) {
// comment 2015/11/5 OPS1-MSE田村 [PZH2.1c][CQ521299] Foolscapの場合、tbl_PaperSizeTableのサイズが固定のため用紙サイズコードから取得するよう修正.
			if( e_SizeCode == TYPE_SizeCodeFlsSef ) {
				// Foolscapの場合、用紙サイズコードから取得.
				const TYPC_PaperSize c_PaperSize = PnlComGetFixedPaperSizeCodeToMM( e_SizeCode );
				c_SizePoint.x = c_PaperSize.c_SizeMM.x;										// 取得したXYを設定.
				c_SizePoint.y = c_PaperSize.c_SizeMM.y;
				switch( PnlComGetMachineSetting()->getFLSType() ) {							// FLSTypeを取得しテキストシンボルを設定.
					case TYPE_FLSType85x135:												// 8_1/2inch x 13_1/2inch.
						Txt_PaperSize_copy_Text.changeText( s_EXP8Hx13H ); break;
					case TYPE_FLSType220x330:												// 220mm x 330mm.
						Txt_PaperSize_copy_Text.changeText( s_ServiceSystem220330MM ); break;
					case TYPE_FLSType85x13:													// 8_1/2inch x 13inch.
						Txt_PaperSize_copy_Text.changeText( s_8Hx13 ); break;
					case TYPE_FLSType825x13:												// 8_1/4inch x 13inch.
						Txt_PaperSize_copy_Text.changeText( s_8Qx13 ); break;
					case TYPE_FLSType8125x1325:												// 8_1/8inch x 13_1/4inch.
						Txt_PaperSize_copy_Text.changeText( s_88x13Q ); break;
					case TYPE_FLSType8x13:													// 8inch x 13inch.
						Txt_PaperSize_copy_Text.changeText( s_pNOT8x13_W080H028A ); break;
					default:break;
				}
				sc_PaperIcon =  tbl_PaperSizeTable[uc_LoopCount].sc_PaperType;
			} else {
				c_SizePoint.x = tbl_PaperSizeTable[uc_LoopCount].f_SizeX;
				c_SizePoint.y = tbl_PaperSizeTable[uc_LoopCount].f_SizeY;
				sc_PaperIcon =  tbl_PaperSizeTable[uc_LoopCount].sc_PaperType;
				// comment 2015/10/26 OPS1-Sky_nozaki [MinervaSBk][CQ520804] C6サイズが洋形2号で表示される問題.
				if( ( TYPE_SizeCodeJpLetterY2 == e_SizeCode ) && ( TYPE_SystemType_JP != PnlComGetDestination() ) ){
//Comment 2018/04/23 【HCLT-Chinna】【IT6_Sp】【AR-1044433】Paper size icon changed LEF to SEF
#if defined ( DEF_KRONOSS_Cus ) || defined ( DEF_SparrowCus ) 
					// 2016/10/7 [KronosS][管理No.21] 用紙サイズカスタマイズ SCD2 3G Sky瀬戸.
					Txt_PaperSize_copy_Text.changeText( s_pNOTenvC6SEF_W056H028A );
#else // DEF_KRONOSS_Cus.
					Txt_PaperSize_copy_Text.changeText( s_NOTenvC6 );
#endif // #if defined ( DEF_KRONOSS_Cus ) || defined ( DEF_SparrowCus ) 
#ifdef DEF_A4_Cus		// Comment VenusMLK -> 2016/02/12 OPS2 Sky高田(庸) [CQ520249]C6 LEFが表示されない問題の修正.
				} else if( ( TYPE_SizeCodeJpLetterY2Lef == e_SizeCode ) && ( TYPE_SystemType_JP != PnlComGetDestination() ) ){
					Txt_PaperSize_copy_Text.changeText( s_NOTenvC6LEF );
#endif // DEF_A4_Cus	// Comment VenusMLK <- 2016/02/12 OPS2 Sky高田(庸) [CQ520249]C6 LEFが表示されない問題の修正.
				}else{
					Txt_PaperSize_copy_Text.changeText( tbl_PaperSizeTable[uc_LoopCount].s_TextID );
				}
			}
			break;
		}
	}
	
	if( !PnlComIsPointNull( c_SizePoint ) ){						// XYサイズが0ではない場合.
//		bl_IconBak = True;
		bl_PaperSize = True;
// comment 2015/11/5 OPS1-MSE田村 [PZH2.1c][CQ521299] パネルの表示用にX、Yを入れ替え.
//		Num_PaperSize_copy_NumX.set( c_SizePoint.x, True );			// Xサイズをセット.
//		Num_PaperSize_copy_NumY.set( c_SizePoint.y, True );			// Yサイズをセット.
		Num_PaperSize_copy_NumX.set( c_SizePoint.y, True );			// Xサイズをセット.
		Num_PaperSize_copy_NumY.set( c_SizePoint.x, True );			// Yサイズをセット.
		// 各パーツの表示.
// [CQ525608] 用紙サイズ選択アイコン OPS2 Sky西場 2016/04/14 start.
#ifdef DEF_A4_Cus
// [AR1022515] KronosS用のアイコン画像への置き換え SCD2 3G Sky西場 2017/05/26 start.
		const TYPE_FeedTray e_SelectedFeedTray = getSelectedFeedTray();
		switch ( sc_PaperIcon ) {
			case PNLD_PaperType_Icon_Post:
				if( e_SelectedFeedTray == TYPE_FeedTrayManual ){
#ifdef DEF_KRONOSS_Cus
					Gra_PaperSize_copy_Icon.changeGraphic( i01_948_000_n_X96_Y96 );
#else
					Gra_PaperSize_copy_Icon.changeGraphic( i01_758_000_n_X96_Y96 );
#endif
				}else{
					Gra_PaperSize_copy_Icon.changeGraphic( i01_898_000_n_X96_Y96 );
				}
			break;
			case PNLD_PaperType_Icon_Env:
				if( e_SelectedFeedTray == TYPE_FeedTrayManual ){
#ifdef DEF_KRONOSS_Cus
					Gra_PaperSize_copy_Icon.changeGraphic( i01_947_000_n_X96_Y96  );
#else
					Gra_PaperSize_copy_Icon.changeGraphic( i01_756_000_n_X96_Y96  );
#endif
				}else{
					Gra_PaperSize_copy_Icon.changeGraphic( i01_893_000_n_X96_Y96  );
				}
			break;
			case PNLD_PaperType_Icon_Letter:
				if( e_SelectedFeedTray == TYPE_FeedTrayManual ){
#ifdef DEF_KRONOSS_Cus
					Gra_PaperSize_copy_Icon.changeGraphic( i01_950_000_n_X96_Y96  );
#else
					Gra_PaperSize_copy_Icon.changeGraphic( i01_884_000_n_X96_Y96  );
#endif
				}else{
					Gra_PaperSize_copy_Icon.changeGraphic( i01_896_000_n_X96_Y96  );
				}
			break;
			case PNLD_PaperType_Icon_Metric:
			case PNLD_PaperType_Icon_Inch:
			case PNLD_PaperType_Icon_ETC:
				if( e_SelectedFeedTray == TYPE_FeedTrayManual ){
#ifdef DEF_KRONOSS_Cus
					Gra_PaperSize_copy_Icon.changeGraphic( i01_946_000_n_X96_Y96  );
#else
					Gra_PaperSize_copy_Icon.changeGraphic( i01_755_000_n_X96_Y96  );
#endif
				}else{
#ifdef DEF_KRONOSS_Cus
					Gra_PaperSize_copy_Icon.changeGraphic( i01_951_000_n_X96_Y96  );
#else
					Gra_PaperSize_copy_Icon.changeGraphic( i01_891_000_n_X96_Y96  );
#endif
				}
			break;
			case PNLD_PaperType_Icon_EnvLef:
				if( e_SelectedFeedTray == TYPE_FeedTrayManual ){
					Gra_PaperSize_copy_Icon.changeGraphic( i01_892_000_n_X96_Y96  );
				}else{
					Gra_PaperSize_copy_Icon.changeGraphic( i01_894_000_n_X96_Y96  );
				}
			break;
//2016/07/12 [CQ532540]封筒・はがき用アイコン追加 sky草柳 start.
			case PNLD_PaperType_Icon_LetterLef:
				if( e_SelectedFeedTray == TYPE_FeedTrayManual ){
//					Gra_PaperSize_copy_Icon.changeGraphic( i01_895_000_n_X96_Y96  );
					Gra_PaperSize_copy_Icon.changeGraphic( i01_892_000_n_X96_Y96  );
				}else{
//					Gra_PaperSize_copy_Icon.changeGraphic( i01_897_000_n_X96_Y96  );
					Gra_PaperSize_copy_Icon.changeGraphic( i01_894_000_n_X96_Y96  );
				}
			break;
			case PNLD_PaperType_Icon_DoublePost:
				if( e_SelectedFeedTray == TYPE_FeedTrayManual ){
#ifdef DEF_KRONOSS_Cus
					Gra_PaperSize_copy_Icon.changeGraphic( i01_949_000_n_X96_Y96  );
#else
					Gra_PaperSize_copy_Icon.changeGraphic( i01_911_000_n_X96_Y96  );
#endif
				}else{
					Gra_PaperSize_copy_Icon.changeGraphic( i01_910_000_n_X96_Y96  );
				}
			break;
			case PNLD_PaperType_Icon_LetterCom10Sef:
				if( e_SelectedFeedTray == TYPE_FeedTrayManual ){
#ifdef DEF_KRONOSS_Cus
					Gra_PaperSize_copy_Icon.changeGraphic( i01_947_000_n_X96_Y96  );
#else
					Gra_PaperSize_copy_Icon.changeGraphic( i01_756_000_n_X96_Y96  );
#endif
				}else{
					Gra_PaperSize_copy_Icon.changeGraphic( i01_893_000_n_X96_Y96  );
				}
			break;
			case PNLD_PaperType_Icon_LetterSef:
				if( e_SelectedFeedTray == TYPE_FeedTrayManual ){
#ifdef DEF_KRONOSS_Cus
					Gra_PaperSize_copy_Icon.changeGraphic( i01_950_000_n_X96_Y96  );
#else
					Gra_PaperSize_copy_Icon.changeGraphic( i01_912_000_n_X96_Y96  );
#endif
				}else{
					Gra_PaperSize_copy_Icon.changeGraphic( i01_913_000_n_X96_Y96  );
				}
			break;
			default:
			break;
		}
//2016/07/12 [CQ532540]封筒・はがき用アイコン追加 sky草柳 end.
// [AR1022515] KronosS用のアイコン画像への置き換え SCD2 3G Sky西場 2017/05/26 end.
#else
// comment 2016/10/17 SCD2-2G南波 [IT5_4.0][RQ-226] 不定型紙の標準カセット対応.
		const TYPE_FeedTray e_SelectedFeedTray = getSelectedFeedTray();
		switch ( sc_PaperIcon ) {
			case PNLD_PaperType_Icon_Post:
				Gra_PaperSize_copy_Icon.changeGraphic( i01_758_000_n_X96_Y96 );
			break;
			case PNLD_PaperType_Icon_Env:
				Gra_PaperSize_copy_Icon.changeGraphic( i01_756_000_n_X96_Y96  );
			break;
			case PNLD_PaperType_Icon_Letter:
//	comment 2016/02/08 HCLT_Naveen [ZeusS_ZX0][CQ525041] New Image for Envelope Icon.
//				Gra_PaperSize_copy_Icon.changeGraphic( i01_757_000_n_X96_Y96  );
				Gra_PaperSize_copy_Icon.changeGraphic( i01_884_000_n_X96_Y96  );
			break;
			case PNLD_PaperType_Icon_Metric:
			case PNLD_PaperType_Icon_Inch:
			case PNLD_PaperType_Icon_ETC:
//				Gra_PaperSize_copy_Icon.changeGraphic( i01_755_000_n_X96_Y96  );
				if( TYPE_FeedTrayManual == e_SelectedFeedTray ){
					Gra_PaperSize_copy_Icon.changeGraphic( i01_755_000_n_X96_Y96  );
				} else {
					Gra_PaperSize_copy_Icon.changeGraphic( i01_914_000_X96_Y96  );
				}
//Comment [IT6-Eagle] 機器構成変更No.2 HCLT-Deeparaj 2018/02/20 Adding Condition for tray 1.
#ifndef DEF_A4_Cus
				if( TYPE_FeedTray1 == e_SelectedFeedTray ) {
					Gra_PaperSize_copy_Icon.changeGraphic( i01_755_000_n_X96_Y96  );
				} else {
					Gra_PaperSize_copy_Icon.changeGraphic( i01_914_000_X96_Y96  );
				}
#endif	//DEF_A4_Cus.
			break;
			default:
			break;
		}
#endif
// [CQ525608] 用紙サイズ選択アイコン OPS2 Sky西場 2016/04/14 end.
	}

// 2015/04/07 D.Suzuki 操作性改善活動 CQ513202対応.
//	PNLE_PaperSizeGroup e_SelectedPaperSizeGroup = Cmp_PaperSizeSelectList.getSelectedPaperSizeGroup();
//	if ( PNLE_PaperSizeGroup_Envelope == e_SelectedPaperSizeGroup) {	// 封筒/はがきタブ選択中の場合.
//		//Gra_EnvelopePostCardIconBak.showEnable( True );
//		//Gra_EnvelopePostCardIcon.showEnable( True );
//		Gra_EnvelopePostCardIcon.showEnable( False );
//	} else {							// 封筒/はがきタブ以外選択中の場合
//		//Gra_EnvelopePostCardIconBak.showEnable( False );
//		Gra_EnvelopePostCardIcon.showEnable( False );
//	}
	Gra_EnvelopePostCardIcon.showEnable( False );
	Gra_EnvelopePostCardIconBak.showEnable( bl_PaperSize );

	// 各パーツの表示.
	Gra_EnvelopePostCardIconBak.showEnable( bl_PaperSize );
	Grp_Paper_Size_Icon.showEnable( bl_PaperSize );

// comment 2014/11/13 OPS1-Sky_nozaki [Poseidon][CQ.506137][デモNo.21] 封筒はがきの裏表セット方向が分かりにくい.
	// 選択されているトレイの取得.
	const TYPE_FeedTray e_FeedTray = getSelectedFeedTray();
// comment 2016/10/17 SCD2-2G南波 [IT5_4.0][RQ-226] 不定型紙の標準カセット対応.
#ifndef DEF_A4_Cus
	if( TYPE_FeedTrayManual == e_FeedTray ) {
		Cmp_STPSize_PopupMessageArea.changeMainMessageText( s_pMSGSetPrintPaper_W448H034A  );		// 「印刷したい面を下向きにし・・」.
	} else {
		Cmp_STPSize_PopupMessageArea.changeMainMessageText( 0 );		// メッセージをださない.
	}
#else	//#ifndef DEF_A4_Cus.
// Comment KronosS -> 2017/07/10 SCD2 3G Sky東野 [AccuRev.1026289].
#ifdef DEF_KRONOSS_Cus
	if( TYPE_FeedTrayManual == e_FeedTray ) {
		Cmp_STPSize_PopupMessageArea.changeMainMessageText( 0 );		// メッセージをださない.
	} else {
		Cmp_STPSize_PopupMessageArea.changeMainMessageText( s_pMSGSetPrintPaper_W448H034A  );		// 「印刷したい面を下向きにし・・」.
	}
#else	//#ifdef DEF_KRONOSS_Cus.
	if( TYPE_FeedTrayManual == e_FeedTray ) {
		Cmp_STPSize_PopupMessageArea.changeMainMessageText( s_pMSGSetPrintPaper_W448H034A  );		// 「印刷したい面を下向きにし・・」.
	} else {
		Cmp_STPSize_PopupMessageArea.changeMainMessageText( 0 );		// メッセージをださない.
	}
#endif	//#ifdef DEF_KRONOSS_Cus.
#endif	//#ifndef DEF_A4_Cus.

}

//-----------------------------------------------------------------------------------------------
/**
 *	@brief	A020114110 用紙定形サイズ  : タブメニュー内キー押下時処理(Qt).
 */
//-----------------------------------------------------------------------------------------------
void PCPC_PaperSizeMetric_copy::procQtButtonOn( ushort us_option )
{
	// 指定サイズを選択状態にする.
	Cmp_PaperSizeSelectList.setSelectPaperSize( ( TYPE_SizeCode )us_option, PNLE_PaperSizeSelect_NoMove );

	// 用紙サイズの設定.
	const TYPC_PaperSize c_PaperSize = PnlComGetFixedPaperSizeCodeToMM( ( TYPE_SizeCode )us_option );
	setPaperSize( c_PaperSize );

	// 画面セットアップ.
	setupGamen();
}

//-----------------------------------------------------------------------------------------------
/**
 *	@brief	A020114110 用紙定形サイズ  : タブメニューキー押下時処理(Qt).
 */
//-----------------------------------------------------------------------------------------------
void PCPC_PaperSizeMetric_copy::procQtEventNotice( ushort us_option )
{
	// 画面セットアップ.
	setupGamen();
}


//-----------------------------------------------------------------------------------------------
/**
 *	@brief	A020114110 用紙定形サイズ  : Ａ・Ｂ系インデックス取得.
 */
//-----------------------------------------------------------------------------------------------
short PCPC_PaperSizeMetric_copy::getButtonIndexPaperSizeMetric( const TYPC_PaperSize  c_PaperSize )
{
	APIC_FeedTray* pc_FeedTray = PnlComGetFeedTray();
	// メトリック系定型ボタンのインデックス.
	short ss_SelectPaperSize = -1;
	// 選択されているトレイの取得.
	const TYPE_FeedTray e_FeedTray = getSelectedFeedTray();
	if( False == pc_FeedTray->getPaperSizeAutoDetectOn( e_FeedTray ) && False == c_PaperSize.b_WideOn ) {
		if( NULL == getPaperSizeMetricTbl() ) {
			LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "[PCPC_PaperSizeMetric_copy::getButtonIndexPaperSizeMetric]Assert Failed!" );
			return ss_SelectPaperSize;
		}
		// 用紙サイズ情報、用紙サイズ情報のサイズを取得.
		const TYPS_PaperSizeInfo* pst_PaperSizeInfo = getPaperSizeMetricTbl();
		const uchar uc_PaperInfoSize = getPaperSizeMetricTblSize();
		if( pst_PaperSizeInfo == NULL ) {
			LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
			return ss_SelectPaperSize;
		}
		for( uchar i = 0; uc_PaperInfoSize > i; ++i ) {
			// サイズコードが一致した場合.
			if( pst_PaperSizeInfo[i].e_SizeCode == c_PaperSize.e_SizeCode ) {
				// ボタンのインデックスを取得.
				ss_SelectPaperSize = ( short )i;
				break;
			}
		}
	}
	return ss_SelectPaperSize;
}

//-----------------------------------------------------------------------------------------------
/**
 *	@brief	A020114110 用紙定形サイズ  : インチ系インデックス取得.
 */
//-----------------------------------------------------------------------------------------------
short PCPC_PaperSizeMetric_copy::getButtonIndexPaperSizeInch( const TYPC_PaperSize  c_PaperSize )
{
	APIC_FeedTray* pc_FeedTray = PnlComGetFeedTray();
	// インチ系定型ボタンのインデックス.
	short ss_SelectPaperSize = -1;
	// 選択されているトレイの取得.
	const TYPE_FeedTray e_FeedTray = getSelectedFeedTray();
	if( False == pc_FeedTray->getPaperSizeAutoDetectOn( e_FeedTray ) && False == c_PaperSize.b_WideOn ) {
		if( NULL == getPaperSizeInchTbl() ) {
			LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "[PCPC_PaperSizeMetric_copy::getButtonIndexPaperSizeInch]Assert Failed!" );
			return ss_SelectPaperSize;
		}
		// 用紙サイズ情報、用紙サイズ情報のサイズを取得.
		const TYPS_PaperSizeInfo* pst_PaperSizeInfo = getPaperSizeInchTbl();
		const uchar uc_PaperInfoSize = getPaperSizeInchTblSize();
		if( pst_PaperSizeInfo == NULL ) {
			LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
			return ss_SelectPaperSize;
		}

		for( uchar i = 0; uc_PaperInfoSize > i; ++i ) {
			// サイズコードが一致した場合.
			if( pst_PaperSizeInfo[i].e_SizeCode == c_PaperSize.e_SizeCode ) {
				// ボタンのインデックスを取得.
				ss_SelectPaperSize = ( short )i;
				break;
			}
		}
	}
	return ss_SelectPaperSize;
}

//-----------------------------------------------------------------------------------------------
/**
 *	@brief	A020114110 用紙定形サイズ  : その他インデックス取得.
 */
//-----------------------------------------------------------------------------------------------
short PCPC_PaperSizeMetric_copy::getButtonIndexPaperSize8K16K( const TYPC_PaperSize  c_PaperSize )
{
	APIC_FeedTray* pc_FeedTray = PnlComGetFeedTray();
	// インチ系定型ボタンのインデックス.
	short ss_SelectPaperSize = -1;
	// 選択されているトレイの取得.
	const TYPE_FeedTray e_FeedTray = getSelectedFeedTray();
	if( False == pc_FeedTray->getPaperSizeAutoDetectOn( e_FeedTray ) && False == c_PaperSize.b_WideOn ) {
		// 用紙サイズ情報、用紙サイズ情報のサイズを取得.
		if( NULL == getPaperSize8K16KTbl() ) {
			LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "[PCPC_PaperSizeMetric_copy::getButtonIndexPaperSize8K16K]Assert Failed!" );
			return ss_SelectPaperSize;
		}
		const TYPS_PaperSizeInfo* pst_PaperSizeInfo = getPaperSize8K16KTbl();
		const uchar uc_PaperInfoSize = getPaperSize8K16KTblSize();
		if( pst_PaperSizeInfo == NULL ) {
			LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
			return ss_SelectPaperSize;
		}

		for( uchar i = 0; uc_PaperInfoSize > i; ++i ) {
			// サイズコードが一致した場合.
			if( pst_PaperSizeInfo[i].e_SizeCode == c_PaperSize.e_SizeCode ) {
				// ボタンのインデックスを取得.
				ss_SelectPaperSize = ( short )i;
				break;
			}
		}
	}
	return ss_SelectPaperSize;
}

//-----------------------------------------------------------------------------------------------
/**
 *	@brief	A020114110 用紙定形サイズ  : Ａ・Ｂ系コードテーブル取得.
 */
//-----------------------------------------------------------------------------------------------
const TYPS_PaperSizeInfo* PCPC_PaperSizeMetric_copy::getPaperSizeMetricTbl()
{
	// 設定変更の対象となるトレイタイプの取得.
	const TYPE_TrayType e_TrayType = getSelectedTrayType();
	const TYPS_PaperSizeInfo* pst_PaperSizeInfo = NULL;
	if( TYPE_TrayTypeMPC == e_TrayType ) {
		// マルチパーパス.
		pst_PaperSizeInfo = &( st_AryPaperSizeMetricMPC[0] );
	} else if( TYPE_TrayTypeMultiMan == e_TrayType ) {
		// 手差し.
		pst_PaperSizeInfo = &( st_AryPaperSizeMetricMultiMan[0] );
	} else { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); }
	return pst_PaperSizeInfo;
}

//-----------------------------------------------------------------------------------------------
/**
 *	@brief	A020114110 用紙定形サイズ  : インチ系コードテーブル取得.
 */
//-----------------------------------------------------------------------------------------------
const TYPS_PaperSizeInfo* PCPC_PaperSizeMetric_copy::getPaperSizeInchTbl()
{
	// 設定変更の対象となるトレイタイプの取得.
	const TYPE_TrayType e_TrayType = getSelectedTrayType();
	const TYPS_PaperSizeInfo* pst_PaperSizeInfo = NULL;
	if( TYPE_TrayTypeMPC == e_TrayType ) {
		// マルチパーパス.
		pst_PaperSizeInfo = &( st_AryPaperSizeInchMPC[0] );
	} else if( TYPE_TrayTypeMultiMan == e_TrayType ) {
		// 手差し.
		pst_PaperSizeInfo = &( st_AryPaperSizeInchMultiMan[0] );
	} else { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); }
	return pst_PaperSizeInfo;
}

//-----------------------------------------------------------------------------------------------
/**
 *	@brief	A020114110 用紙定形サイズ  : その他コードテーブル取得.
 */
//-----------------------------------------------------------------------------------------------
const TYPS_PaperSizeInfo* PCPC_PaperSizeMetric_copy::getPaperSize8K16KTbl()
{
	// 設定変更の対象となるトレイタイプの取得.
	const TYPE_TrayType e_TrayType = getSelectedTrayType();
	const TYPS_PaperSizeInfo* pst_PaperSizeInfo = NULL;
	// 仕向けを判定.
	switch( PnlComGetDestination() ) {
		case TYPE_SystemType_JP:		// 日本仕向け.
			if( TYPE_TrayTypeMPC == e_TrayType ) {
				// マルチパーパス.
				pst_PaperSizeInfo = &( st_AryPaperSize8K16KJpMPC[0] );
			} else if( TYPE_TrayTypeMultiMan == e_TrayType ) {
				// 手差し.
				pst_PaperSizeInfo = &( st_AryPaperSize8K16KJpMultiMan[0] );
			} else { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); }
			break;
		case TYPE_SystemType_US:		// US仕向け.
			if( TYPE_TrayTypeMPC == e_TrayType ) {
				// マルチパーパス.
				pst_PaperSizeInfo = &( st_AryPaperSize8K16KUsMPC[0] );
			} else if( TYPE_TrayTypeMultiMan == e_TrayType ) {
				// 手差し.
				pst_PaperSizeInfo = &( st_AryPaperSize8K16KUsMultiMan[0] );
			} else { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); }
			break;
		case TYPE_SystemType_EU:		// EU仕向け.
			if( TYPE_TrayTypeMPC == e_TrayType ) {
				// マルチパーパス.
				pst_PaperSizeInfo = &( st_AryPaperSize8K16KEuMPC[0] );
			} else if( TYPE_TrayTypeMultiMan == e_TrayType ) {
				// 手差し.
				pst_PaperSizeInfo = &( st_AryPaperSize8K16KEuMultiMan[0] );
			} else { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); }
			break;
		default: LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); break;
	}
	return pst_PaperSizeInfo;
}

//-----------------------------------------------------------------------------------------------
/**
 *	@brief	A020114110 用紙定形サイズ  : Ａ・Ｂ系コードテーブルサイズ取得.
 */
//-----------------------------------------------------------------------------------------------
uchar PCPC_PaperSizeMetric_copy::getPaperSizeMetricTblSize()
{
	// 設定変更の対象となるトレイタイプの取得.
	const TYPE_TrayType e_TrayType = getSelectedTrayType();
	uchar uc_PaperInfoSize = 0;
	if( TYPE_TrayTypeMPC == e_TrayType ) {
		// マルチパーパス.
		uc_PaperInfoSize = uc_PaperSizeMetricTblSizeMPC;
	} else if( TYPE_TrayTypeMultiMan == e_TrayType ) {
		// 手差し.
		uc_PaperInfoSize = uc_PaperSizeMetricTblSizeMultiMan;
	} else { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); }
	return uc_PaperInfoSize;
}

//-----------------------------------------------------------------------------------------------
/**
 *	@brief	A020114110 用紙定形サイズ  : インチ系コードテーブルサイズ取得.
 */
//-----------------------------------------------------------------------------------------------
uchar PCPC_PaperSizeMetric_copy::getPaperSizeInchTblSize()
{
	// 設定変更の対象となるトレイタイプの取得.
	const TYPE_TrayType e_TrayType = getSelectedTrayType();
	uchar uc_PaperInfoSize = 0;
	if( TYPE_TrayTypeMPC == e_TrayType ) {
		// マルチパーパス.
		uc_PaperInfoSize = uc_PaperSizeInchTblSizeMPC;
	} else if( TYPE_TrayTypeMultiMan == e_TrayType ) {
		// 手差し.
		uc_PaperInfoSize = uc_PaperSizeInchTblSizeMultiMan;
	} else { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); }
	return uc_PaperInfoSize;
}

//-----------------------------------------------------------------------------------------------
/**
 *	@brief	A020114110 用紙定形サイズ  : その他系コードテーブルサイズ取得.
 */
//-----------------------------------------------------------------------------------------------
uchar PCPC_PaperSizeMetric_copy::getPaperSize8K16KTblSize()
{
	// 設定変更の対象となるトレイタイプの取得.
	const TYPE_TrayType e_TrayType = getSelectedTrayType();
	uchar uc_PaperInfoSize = 0;
	// 仕向けを判定.
	switch( PnlComGetDestination() ) {
		case TYPE_SystemType_JP:		// 日本仕向け.
			if( TYPE_TrayTypeMPC == e_TrayType ) {
				// マルチパーパス.
				uc_PaperInfoSize = uc_PaperSize8K16KJpTblSizeMPC;
			} else if( TYPE_TrayTypeMultiMan == e_TrayType ) {
				// 手差し.
				uc_PaperInfoSize = uc_PaperSize8K16KJpTblSizeMultiMan;
			} else { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); }
			break;
		case TYPE_SystemType_US:		// US仕向け.
			if( TYPE_TrayTypeMPC == e_TrayType ) {
				// マルチパーパス.
				uc_PaperInfoSize = uc_PaperSize8K16KUsTblSizeMPC;
			} else if( TYPE_TrayTypeMultiMan == e_TrayType ) {
				// 手差し.
				uc_PaperInfoSize = uc_PaperSize8K16KUsTblSizeMultiMan;
			} else { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); }
			break;
		case TYPE_SystemType_EU:		// EU仕向け.
			if( TYPE_TrayTypeMPC == e_TrayType ) {
				// マルチパーパス.
				uc_PaperInfoSize = uc_PaperSize8K16KEuTblSizeMPC;
			} else if( TYPE_TrayTypeMultiMan == e_TrayType ) {
				// 手差し.
				uc_PaperInfoSize = uc_PaperSize8K16KEuTblSizeMultiMan;
			} else { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); }
			break;
		default: LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); break;
	}
	return uc_PaperInfoSize;
}

//-----------------------------------------------------------------------------------------------
/**
 *	@brief	A020114110 用紙定形サイズ  : トレイタイプを取得する.
 */
//-----------------------------------------------------------------------------------------------
TYPE_TrayType PCPC_PaperSizeMetric_copy::getSelectedTrayType()
{
	APIC_FeedTray* pc_FeedTray = PnlComGetFeedTray();
	// 設定変更の対象となるトレイの取得.
	const TYPE_FeedTray e_FeedTray = getSelectedFeedTray();
	// 設定変更の対象となるトレイタイプの取得.
	TYPE_TrayType e_TrayType = pc_FeedTray->getTrayType( e_FeedTray );
	return e_TrayType;
}

//-----------------------------------------------------------------------------------------------
/**
 *	@brief	A020114110 用紙定形サイズ  : 用紙サイズを設定する.
 */
//-----------------------------------------------------------------------------------------------
void PCPC_PaperSizeMetric_copy::setPaperSize( TYPC_PaperSize  c_PaperSize )
{
	APIC_FeedTray* pc_FeedTray = PnlComGetFeedTray();
	// 対象トレイの用紙種類を取得.
	const TYPE_FeedTray e_FeedTray = getSelectedFeedTray();
	// 用紙種類の取得.
	TYPC_PaperType c_PaperType = pc_FeedTray->getPaperType( e_FeedTray );
	TYPE_PaperType e_PaperType = c_PaperType.getPaperType();
	Bool b_2ndSidedPaper = c_PaperType.getFeed2ndSidedPaper();
	const TYPE_WidePaperImagePosition e_WidePaperImagePosition = c_PaperType.getWidePaperImagePosition();
	TYPE_CustomPaperType e_CustomPaperType = c_PaperType.getCustomPaperType();
//comment 2018/04/08 【HCLT-Prasanjet】 【IT6 EAGLE】 【AR1042477】 setting last selection paper size.
#ifndef DEF_A4_Cus
	APIC_SoftDipSw* pc_SoftDipSw = APIC_SoftDipSw::newInstance();
	if( False == pc_SoftDipSw->getSoftDipSwBitForPanelSpecNo( DEFSOFTWARESW142, 4) ) {
		if (  TYPE_FeedTray1 == e_FeedTray ) {
			if (IsPaperSizeB6OrLess ()){
				APIC_SystemSetting* p_SystemSetting = PnlComGetSystemSetting();
				if( NULL != p_SystemSetting ) {
					switch(c_PaperSize.e_SizeCode){
					  case TYPE_SizeCodeA6Sef:
					  case TYPE_SizeCodeB6Sef:
					  case TYPE_SizeCodePostcard_JpSef:
					  case TYPE_SizeCodePostcard_EuSef:
					  case TYPE_SizeCodeIN4x6Sef:
					  case TYPE_SizeCodeNonStandard:
					  	{
					  		p_SystemSetting->setLastMainUnitTrayEnvelopeSize(c_PaperSize.e_SizeCode);
//comment 01/09/2018 【HCLT-AppalaRaju】 【IT6 EAGLE】 【AR1044533】  set the paperdetection size to NULL.
						    PNLC_DetectPaperSizeLib* cp_DetectPaperSizeLib = PNLC_DetectPaperSizeLib::newInstance();
						    cp_DetectPaperSizeLib->setPaperSizeDetect(c_PaperSize);
	  					}
							break;
	  					default:break;
					}
				}
//comment 18/10/2018 【HCLT-Deeparaj】 【IT6 EAGLE】 【AR1065054】 Set the paper type to "Plain" other than "Postcard" Size.
//comment 2019/01/10 HCLT-Basker [DenebMLK][AR1071302] Remove special condition for setting thick3 papertype if 4x6, A6 card paper size id selected.
#ifndef DEF_DenebMLKCus
				if ((TYPE_SizeCodePostcard_JpSef == c_PaperSize.e_SizeCode) 	
					|| (TYPE_SizeCodeIN4x6Sef == c_PaperSize.e_SizeCode )
					|| (TYPE_SizeCodePostcard_EuSef == c_PaperSize.e_SizeCode )) {
						e_PaperType = TYPE_PaperTypeThick3;
				} else {
#endif // DEF_DenebMLKCus.
//comment 2020/05/18 【HCLT-prasanjeet】 【IT6_HeliosMLK】 【OP_BTS-23938】 Set the paper type to "Plain" other than "Postcard/Envelope size" Size.
//						e_PaperType = TYPE_PaperTypeNormal;	
#ifndef DEF_DenebMLKCus
				}
#endif // DEF_DenebMLKCus.
			}
		}
	}
#endif
// Comment 2020/04/18 [HCL_Prasanjeet][IT6_EagLeZ+][OP_BTS-22867] paper type canceletion.
	Bool bl_PostcardSet=False;
	// はがきサイズの場合.
//comment 2019/01/10 HCLT-Basker [DenebMLK][AR1071302] Remove special condition for setting thick3 papertype if 4x6, A6 card paper size id selected.
#ifndef DEF_DenebMLKCus
	if( ( TYPE_SizeCodePostcard_JpSef == c_PaperSize.e_SizeCode )
// AS-PVD_14_15 用紙サイズ設定 kuroda Start.
#ifdef DEF_A4_Cus
		|| (TYPE_SizeCodeDoublePostcard_JpSef == c_PaperSize.e_SizeCode)
#endif
// AS-PVD_14_15 用紙サイズ設定 kuroda End.
	    || ( TYPE_SizeCodeIN4x6Sef == c_PaperSize.e_SizeCode )
	    || ( TYPE_SizeCodePostcard_EuSef == c_PaperSize.e_SizeCode ) ) {
		// 用紙種類を厚紙2に設定.→厚紙３に設定に変更 .
// AS-PVD_14_15 用紙サイズ設定 kuroda Start.
// Comment 2020/04/18 [HCL_Prasanjeet][IT6_EagLeZ+][OP_BTS-22867] paper type canceletion.
	    bl_PostcardSet =True;
	    m_bl_Postcard =True;
#ifdef DEF_A4_Cus
		e_PaperType = TYPE_PaperPostCard;
#else
		e_PaperType = TYPE_PaperTypeThick3;
#endif
// AS-PVD_14_15 用紙サイズ設定 kuroda End.
		b_2ndSidedPaper = False;
		e_CustomPaperType = TYPE_CustomPaperType_UnSelect;
	}
#endif // DEF_DenebMLKCus.
	// 封筒サイズの場合.
	if( ( ( TYPE_SizeCodeEnvelopeB5 == c_PaperSize.e_SizeCode )
// AS-PVD_14_15 用紙サイズ設定 kuroda Start.
#ifdef DEF_A4_Cus
			|| (TYPE_SizeCodeJpLetterY2Lef == c_PaperSize.e_SizeCode)
			|| (TYPE_SizeCodeEnvelopeDLLef == c_PaperSize.e_SizeCode)
			|| (TYPE_SizeCodeEnvelopeMornarchLef == c_PaperSize.e_SizeCode)
			|| (TYPE_SizeCodeJpLetterY3Lef == c_PaperSize.e_SizeCode)			// VenusMLK機種より洋形3号LEFを追加.
#endif
// AS-PVD_14_15 用紙サイズ設定 kuroda End.
	      || ( TYPE_SizeCodeEnvelopeC5 == c_PaperSize.e_SizeCode )
	      || ( TYPE_SizeCodeEnvelopeDL == c_PaperSize.e_SizeCode )
	      || ( TYPE_SizeCodeEnvelopeCom10 == c_PaperSize.e_SizeCode )
	      || ( TYPE_SizeCodeEnvelopeMornarch == c_PaperSize.e_SizeCode )
	      || ( TYPE_SizeCodeEnvelopeC4 == c_PaperSize.e_SizeCode )
	      || ( TYPE_SizeCodeJpLetterY2 == c_PaperSize.e_SizeCode )
	      || ( TYPE_SizeCodeJpLetterY3 == c_PaperSize.e_SizeCode )
	      || ( TYPE_SizeCodeJpLetterY4 == c_PaperSize.e_SizeCode )
	      || ( TYPE_SizeCodeJpLetterL3 == c_PaperSize.e_SizeCode )
	      || ( TYPE_SizeCodeJpLetterL4 == c_PaperSize.e_SizeCode )
	      || ( TYPE_SizeCodeJpLetterK1 == c_PaperSize.e_SizeCode )
	      || ( TYPE_SizeCodeJpLetterK2 == c_PaperSize.e_SizeCode )
	      || ( TYPE_SizeCodeJpLetterK3 == c_PaperSize.e_SizeCode ) )
	    && ( True == combinationCheck( CNK_SpecialEnvelope,PnlComGetJobCopy(),False,False ) ) ) {
		// 用紙種類を封筒に設定.
		e_PaperType = TYPE_PaperTypeEnvelope;
		e_CustomPaperType = TYPE_CustomPaperType_UnSelect;
		b_2ndSidedPaper = False;
// Comment 2020/04/18 [HCL_Prasanjeet][IT6_EagLeZ+][OP_BTS-22867] paper type canceletion.
	    	m_bl_Postcard =True;
	    } else {
	    	if( False ==bl_PostcardSet) {
//comment 2020/05/18 【HCLT-prasanjeet】 【IT6_HeliosMLK】 【OP_BTS-23938】 Set the paper type to "Plain" other than "Postcard/Envelope size" Size.
//	    		if( True == m_bl_Postcard){ 
//	    			 m_bl_Postcard = False;
				if ( True ==canclePostCardEnvPaperType(e_FeedTray )) {
					e_PaperType = TYPE_PaperTypeNormal;	
				}
		}
	}
	if( ( NULL == PnlComGetJobCopy() ) ||
	    ( NULL == PnlComGetJobCopy()->getJobSpec() ) ||
	    ( NULL == PnlComGetJobCopy()->getJobSpec()->pc_Print ) ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "[PCPC_PaperSizeMetric_copy::setPaperSize]Assert Failed!" );
		return;
	}
	TYPC_Print* pc_Print = PnlComGetJobCopy()->getJobSpec()->pc_Print;
	// 用紙種類をジョブに設定.
	pc_Print->c_PaperType.setPaperType( e_PaperType );
	pc_Print->c_PaperType.setFeed2ndSidedPaper( b_2ndSidedPaper );
	pc_Print->c_PaperType.setWidePaperImagePosition( e_WidePaperImagePosition );
	// 用紙サイズをジョブに設定.
	pc_Print->c_PaperSize = c_PaperSize;
	// カスタム用紙属性をジョブに設定.
	pc_Print->c_PaperType.setCustomPaperType( e_CustomPaperType );
	// 自動検出をオフ.
	pc_FeedTray->setPaperSizeAutoDetectOn( e_FeedTray, False );
	// トレイへの設定.
	onlyPanel.setFeedSizeKind( e_FeedTray );
	// APS/AMS処理関数.
	selectAutoPaperMag();
}
//comment 2020/05/18 【HCLT-prasanjeet】 【IT6_HeliosMLK】 【OP_BTS-23938】 Set the paper type to "Plain" other than "Postcard/Envelope size" Size.
//-----------------------------------------------------------------------------------------------
/**
 *	@brief	A020114110 用紙定形サイズ  : paper type cancellation check.
 */
//-----------------------------------------------------------------------------------------------
Bool PCPC_PaperSizeMetric_copy::canclePostCardEnvPaperType(TYPE_FeedTray e_FeedTray )
{
	Bool bl_Ret=False;
	APIC_FeedTray* pc_FeedTray = PnlComGetFeedTray();
	TYPC_PaperSize c_PaperSizeOld = pc_FeedTray->getPaperSize( e_FeedTray );
	if( ( TYPE_SizeCodePostcard_JpSef == c_PaperSizeOld.e_SizeCode )
#ifdef DEF_A4_Cus
		|| (TYPE_SizeCodeDoublePostcard_JpSef == c_PaperSizeOld.e_SizeCode)
#endif
	    || ( TYPE_SizeCodeIN4x6Sef == c_PaperSizeOld.e_SizeCode )
	    || ( TYPE_SizeCodePostcard_EuSef == c_PaperSizeOld.e_SizeCode)
        || (TYPE_SizeCodeEnvelopeB5 == c_PaperSizeOld.e_SizeCode )
#ifdef DEF_A4_Cus
		|| (TYPE_SizeCodeJpLetterY2Lef == c_PaperSizeOld.e_SizeCode)
		|| (TYPE_SizeCodeEnvelopeDLLef == c_PaperSizeOld.e_SizeCode)
		|| (TYPE_SizeCodeEnvelopeMornarchLef ==  c_PaperSizeOld.e_SizeCode)
		|| (TYPE_SizeCodeJpLetterY3Lef ==  c_PaperSizeOld.e_SizeCode)			
#endif
	    || ( TYPE_SizeCodeEnvelopeC5 ==  c_PaperSizeOld.e_SizeCode )
	    || ( TYPE_SizeCodeEnvelopeDL ==  c_PaperSizeOld.e_SizeCode )
	    || ( TYPE_SizeCodeEnvelopeCom10 ==  c_PaperSizeOld.e_SizeCode )
	    || ( TYPE_SizeCodeEnvelopeMornarch ==  c_PaperSizeOld.e_SizeCode )
	    || ( TYPE_SizeCodeEnvelopeC4 ==  c_PaperSizeOld.e_SizeCode )
	    || ( TYPE_SizeCodeJpLetterY2 == c_PaperSizeOld.e_SizeCode )
	    || ( TYPE_SizeCodeJpLetterY3 == c_PaperSizeOld.e_SizeCode )
	    || ( TYPE_SizeCodeJpLetterY4 == c_PaperSizeOld.e_SizeCode )
	    || ( TYPE_SizeCodeJpLetterL3 == c_PaperSizeOld.e_SizeCode )
	    || ( TYPE_SizeCodeJpLetterL4 == c_PaperSizeOld.e_SizeCode )
	    || ( TYPE_SizeCodeJpLetterK1 == c_PaperSizeOld.e_SizeCode )
	    || ( TYPE_SizeCodeJpLetterK2 == c_PaperSizeOld.e_SizeCode )
	    || ( TYPE_SizeCodeJpLetterK3 == c_PaperSizeOld.e_SizeCode ) ) {
	       bl_Ret =True;;
	    } 
	return bl_Ret;
}
//-----------------------------------------------------------------------------------------------
/**
 *	@brief	A020114110 用紙定形サイズ  : 選択トレイ取得.
 *	戻値	TYPE_FeedTrayクラス.
 */
//-----------------------------------------------------------------------------------------------
TYPE_FeedTray  PCPC_PaperSizeMetric_copy::getSelectedFeedTray()
{
	if( ( NULL == PnlComGetJobCopy() ) ||
	    ( NULL == PnlComGetJobCopy()->getJobSpec() ) ||
	    ( NULL == PnlComGetJobCopy()->getJobSpec()->pc_Print ) ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "[PCPC_PaperSizeMetric_copy::getSelectedFeedTray]Assert Failed!" );
		return TYPE_CustomPaperType_UnSelect;											// カスタム用紙属性[未選択].
	}
	TYPC_Print* pc_Print = PnlComGetJobCopy()->getJobSpec()->pc_Print;
	// 設定変更の対象となるトレイを取得.
	TYPE_FeedTray e_FeedTray = pc_Print->c_PaperTray.getFeedTray();
	return e_FeedTray;
}

PNLE_PaperTypeCategory PCPC_PaperEnl_copy::m_e_PaperTypeCategory = PNLE_PaperTypeCategory_Normal;		// 用紙種類カテゴリ.
uchar PCPC_PaperEnl_copy::m_uc_CustomPaperTypeBtnID = BTNID_KEY_PAPERKINDCUSTOM_1;						// カスタム用紙種類ボタンID.
//-----------------------------------------------------------------------------------------------
/**
 *	@brief	A020114100 選択トレイの設定変更 : 画面開始処理.
 */
//-----------------------------------------------------------------------------------------------
void PCPC_PaperEnl_copy::startGamen()
{
	PNLC_CopyDataLib* pc_CopyDataLib = PNLC_CopyDataLib::newInstance();
	if ( NULL != pc_CopyDataLib ) {
		pc_CopyDataLib->setCopyPaperTypeButton( PNLD_ButtonPaperKindUnSelect );
		pc_CopyDataLib->setPIToggleStatus(False);
	}
	// 親のスタート画面をコールする
	PPopupDecisionButtonCom::startGamen();

	// 初期化.
	m_sc_PaperSizeTab = PNLD_PaperSizeTabDetection;
	m_sc_PaperSizeSelect = PNLD_PaperSizeUnSelect;

	// 対象トレイの用紙種類を取得.
	const TYPE_FeedTray e_FeedTray = getSelectedFeedTray();
//	comment 2015/03/13 HCLT-Durga [HeliosL][CQ510997&CQ510999]  To set the Thick3 for A6card Type.	
//// comment 2015/02/24 HCLT-Ezhil [HeliosL][CQ510368] Paper type selection display based on paper in manual tray.
//	m_bl_Thick3AutoSelectFlag = False;			// Flag updation whether to set paper type as Thick3.

// comment 2017/12/27 SCD2-4G Sky平田 [Eagle][RQ-720]紙種自動検知.
	// メディア自動検知設定有効?.
	if( True == PnlComGet_isPaperAutoDetectAdmin( e_FeedTray ) ) {
		Key_ChkPaperTypeAutoDetect.showEnable( True );
		Cmp_PaperTypeSelectList.setPaperTypeListGeometry( i32_A020114100_PaperTypeSelectList_PosX
														, i32_A020114100_PaperTypeSelectList_PosY
														, i32_A020114100_PaperTypeSelectList_Width
														, i32_A020114100_PaperTypeSelectList_Height );
		Cmp_PaperTypeSelectList.setScrollAreaGeometry( i32_A020114100_PaperTypeScrollArea_PosX
														, i32_A020114100_PaperTypeScrollArea_PosY
														, i32_A020114100_PaperTypeScrollArea_Width
														, i32_A020114100_PaperTypeScrollArea_Height );
	} else {
		Key_ChkPaperTypeAutoDetect.showEnable( False );
	}

	TYPE_PaperType	e_PaperType = getJobPaperType();		// 用紙種類.
	// 用紙を選択状態にする.
	Cmp_PaperTypeSelectList.setSelectPaperType( e_PaperType, PNLE_PaperTypeSelect_MoveNoAnimation );

	if( e_FeedTray == TYPE_FeedTrayManual ) {		// 手差しトレイの場合.
// AS-PVD_14_15 用紙サイズ設定 kuroda Start.
// Comment  2019/01/10 【HCLT-Rajkumar】 【IT6_DenebMLK】 【AR1070372_Reopen】 DenebMLK Doesn't detect FD size and CD width so operation behaviour same as A4.
//#ifdef DEF_A4_Cus
#if defined ( DEF_A4_Cus ) || defined ( DEF_DenebMLKCus )
		Rbt_PaperSizeTab.showEnableAll( False );				// タブグループ非表示.
		Bak_SelectPaperSizeArea.changeSize( 344, 176 );			// 検知サイズ候補背景拡大.
		Bak_SelectPaperSizeArea.changePosition( 416 + PNLD_POPUP_START_X( 780 ), 176 + PNLD_POPUP_START_Y( 440, PNLD_POPUP_BUTTON_EXIST ) );		// 検知サイズ候補背景拡大.
#else
		if( True == PnlComGetPrinterSystem()->isManualTrayFDSizeDetect() ) {	// FDセンサーあり.
			Rbt_PaperSizeTab.showEnableAll( False );				// タブグループ非表示.
			Bak_SelectPaperSizeArea.changeSize( 344, 176 );			// 検知サイズ候補背景拡大.
			Bak_SelectPaperSizeArea.changePosition( 416 + PNLD_POPUP_START_X( 780 ), 176 + PNLD_POPUP_START_Y( 440, PNLD_POPUP_BUTTON_EXIST ) );		// 検知サイズ候補背景拡大.
		} else {			// FDセンサーなし.
			Rbt_PaperSizeTab.showEnableAll( True );					// タブグループ表示.
			Bak_SelectPaperSizeArea.changeSize( 344, 136 );			// 検知サイズ候補背景縮小.
			Bak_SelectPaperSizeArea.changePosition( 416 + PNLD_POPUP_START_X( 780 ), 216 + PNLD_POPUP_START_Y( 440, PNLD_POPUP_BUTTON_EXIST ) );		// 検知サイズ候補背景縮小.
		}
#endif
// AS-PVD_14_15 用紙サイズ設定 kuroda End.
	} else {		// 手差しトレイ以外の場合.
		Rbt_PaperSizeTab.showEnableAll( False );					// タブグループ非表示.
		Bak_SelectPaperSizeArea.changeSize( 344, 176 );				// 検知サイズ候補背景拡大.
		Bak_SelectPaperSizeArea.changePosition( 416 + PNLD_POPUP_START_X( 780 ), 176 + PNLD_POPUP_START_Y( 440, PNLD_POPUP_BUTTON_EXIST ) );			// 検知サイズ候補背景拡大.
	}

	// 動的背景状態を再初期化.
	// トレイ３または４でLCC、もしくはLCTの場合は用紙サイズのグループを非表示.
	if( ( ( TYPE_FeedTray3 == getSelectedFeedTray() || TYPE_FeedTray4 == getSelectedFeedTray() )
	      && TYPE_TrayTypeLCC == getSelectedTrayType() ) || ( TYPE_TrayTypeLCT == getSelectedTrayType() ) ) {
		Txt_PaperSize.showEnable( False );
		Bak_PaperSizeAndDirectionArea.showEnable( False );
		Key_AutoSearch.showEnable( False );
		Key_12x18.showEnable( False );
// comment 2016/10/17 SCD2-2G南波 [IT5_4.0][RQ-226] 不定型紙の標準カセット対応.
		Key_12x18_Indeterminate.showEnable( False );
		Rbt_PaperSizeTab.showEnableAll( False );					// タブグループ.
		Grp_OtherPaperSize.showEnableAll( False );					// その他サイズで使用されるグループ.
		Rbt_SelectPaperSize.showEnableAll( False );					// 検知サイズ候補で使用されるグループ.
		Bak_SelectPaperSizeArea.showEnable( False );				// 検知サイズ候補背景.
		Gra_InfoIcon.showEnable( False );							// インフォアイコン.
		Txt_InfoMsg.showEnable( False );							// インフォメッセージ.
		Key_WidePaper.showEnable( False );
		Bak_PaperSizeGrpBase.showEnable( False );
// comment 2014/11/28 OPS1-Sky東島 [ZeusS][機器構成変更No.11] 普通紙用紙分割.
		Txt_PaperSizeEx.showEnable(False);
		Bak_PaperSizeGrpBaseEx.showEnable(False);
	}

	// メッセージエリア初期化.
	Cmp_CopyPaperTypeUniversalSettingChangePMA.setMode( PNLE_PopupMessageAreaMessageModeCopy );
	Cmp_CopyPaperTypeUniversalSettingChangePMA.showDestinationCount( False );
	Cmp_CopyPaperTypeUniversalSettingChangePMA.setOnOffButton( False );
	Cmp_CopyPaperTypeUniversalSettingChangePMA.setNowPopupGamen( PNLE_CopySelectionTraySettingChange );

// comment 2015/02/12 OPS1-Sky浜野 [Poseidon][CQ510130] はがき、封筒の用紙種類の初期化なし対応.
	Bool bl_IsPostcardEnvelope = False;
	APIC_SoftDipSw* pc_SoftDipSw = APIC_SoftDipSw::newInstance();
	if( ( NULL != pc_SoftDipSw ) && ( False == pc_SoftDipSw->getSoftDipSwBitForPanelSpecNo( DEFSOFTWARESW142, 1 ) ) ){
		APIC_FeedTray* pc_FeedTray = PnlComGetFeedTray();
		if ( NULL != pc_FeedTray ) {
			TYPC_PaperSize c_PaperSize	= pc_FeedTray->getPaperSize( TYPE_FeedTrayManual );
			switch ( c_PaperSize.e_SizeCode ) {
				case TYPE_SizeCodePostcard_JpSef:	// PostCard JP SEF.
				case TYPE_SizeCodeIN4x6Sef:			// 写真サイズ・ＵＳ・4inch × 6inch SEF.
				case TYPE_SizeCodePostcard_EuSef:	// PostCard EU SEF.
				case TYPE_SizeCodeEnvelopeB5:		// 封筒 B5.
				case TYPE_SizeCodeEnvelopeC5:		// 封筒 C5 162mm × 229mm.
				case TYPE_SizeCodeEnvelopeDL:		// 封筒 DL 110mm × 220mm.
				case TYPE_SizeCodeEnvelopeCom10:	// 封筒 Com10 4.81×9.21 inch.
				case TYPE_SizeCodeEnvelopeMornarch:	// キングサイズの封筒サイズ  3.87×7.5 inch.
				case TYPE_SizeCodeEnvelopeC4:		// 封筒 C4 229mm × 324mm.
				case TYPE_SizeCodeJpLetterY2:		// 洋形2号.
				case TYPE_SizeCodeJpLetterY3:		// 洋形3号.
				case TYPE_SizeCodeJpLetterY4:		// 洋形4号.
				case TYPE_SizeCodeJpLetterL3:		// 長形3号.
				case TYPE_SizeCodeJpLetterL4:		// 長形4号.
				case TYPE_SizeCodeJpLetterK1:		// 角形1号.
				case TYPE_SizeCodeJpLetterK2:		// 角形2号.
				case TYPE_SizeCodeJpLetterK3:		// 角形3号.
#ifdef DEF_A4_Cus
				case TYPE_SizeCodeDoublePostcard_JpSef:	// 往復はがき Sef.
				case TYPE_SizeCodeDoublePostcard_JpLef:	// 往復はがき Lef.
				case TYPE_SizeCodeJpLetterY2Lef:		// 洋形2号 Lef.
				case TYPE_SizeCodeEnvelopeDLLef:		// 封筒 DL Lef.
				case TYPE_SizeCodeEnvelopeMornarchLef:	// キングサイズの封筒サイズ Lef.
				case TYPE_SizeCodeJpLetterY3Lef:		// 洋形3号 Lef.
#endif //DEF_A4_Cus.
					bl_IsPostcardEnvelope = True;	// はがき、封筒系用紙サイズ.
					break;
				default:	// はがき、封筒以外.
					break;
			}
		}
	}
	// 手差しトレイの場合、条件により手差し用紙種類初期設定の設定値を反映する.
	// 条件の「手差し用紙セット時」の処理は PCGC_CopyBaseCom::procPaperInsertToManualFeed() で行っている.
	TYPE_PaperType e_DefaultType = TYPE_PaperTypeNormal;
	TYPE_CustomPaperType e_DefaultCustomPaperType = TYPE_CustomPaperType_UnSelect;
	PnlComGetCopyJobSetting()->getDefaultPaperType_TrayManual( &e_DefaultType, &e_DefaultCustomPaperType );	// 現在の手差し用紙種類初期設定を取得.
	if( ( getSelectedFeedTray() == TYPE_FeedTrayManual ) 								// 選択トレイの種類が手差しトレイ .
// comment 2015/02/12 OPS1-Sky浜野 [Poseidon][CQ510130] はがき、封筒の用紙種類の初期化なし対応.
//		&& (PnlComGetCopyJobSetting()->getEnableDefaultPaperType_TrayManual() == True) 	// 手差し用紙種類初期設定が「する」.
		&& ( ( False == bl_IsPostcardEnvelope ) && ( PnlComGetCopyJobSetting()->getEnableDefaultPaperType_TrayManual() == True ) ) 	// 手差し用紙種類初期設定が「する」.
// 2010/12/15 Harada Minerva UV-00724 メディアタイプ情報をマニュアルで追加/編集できる機能 >>.
	    && ( ( m_e_LastDefaultType != e_DefaultType )										// 前回画面表示時と手差し用紙種類初期設定が違う.
	         || ( m_e_LastDefaultCustomPaperType != e_DefaultCustomPaperType ) )				// 前回画面表示時と手差し用紙種類初期設定(カスタム用紙属性)が違う.
	    && ( comPrintJob == NULL ) && ( onlyPanel.us_NowMessageWarning != 0 ) )				// かつジョブなしかつペーパーエンプティ発生時.
	{
		m_e_LastDefaultType = e_DefaultType;
		Bool b_Default2ndSidedPaper = False;
		m_e_LastDefaultCustomPaperType = e_DefaultCustomPaperType;
		for( uchar i = 0; i < uc_ManualFeedTrayPaperTypeTblNum; i++ ) 					// 両面２面目設定か.
		{
			if( e_DefaultType == st_ManualFeedTrayPaperTypeSet[i].e_PaperType2ndSide )	// 手差し用紙初期設定値が両面２面目の設定値になっているか.
			{
				e_DefaultType = st_ManualFeedTrayPaperTypeSet[i].e_PaperTypePanel;
				b_Default2ndSidedPaper = True;											// 両面２面目設定有効.
				break;
			}
		}
		setPaperTypeCom( e_DefaultType, b_Default2ndSidedPaper, e_DefaultCustomPaperType );		// 用紙種類設定.
	}


	m_e_PaperTypeCategory = PNLE_PaperTypeCategory_Normal;			// 用紙種類カテゴリ.
	m_uc_CustomPaperTypeBtnID = BTNID_KEY_PAPERKINDCUSTOM_1;		// カスタム用紙種類ボタンID.
}

//-----------------------------------------------------------------------------------------------
/**
 *	@brief	A020114100 選択トレイの設定変更 : 画面描画処理.
 */
//								12x18時、用紙サイズにアイコンが出てしまう不具合修正。
//-----------------------------------------------------------------------------------------------
void PCPC_PaperEnl_copy::setupGamen( short option )
{
//comment 2018/30/04 【HCLT-Hepzibah】 【IT6 EAGLE】 【AR1044109】 Check for US destination and set paper size.
#ifndef DEF_A4_Cus
	APIC_SoftDipSw* pc_SoftDipSw = APIC_SoftDipSw::newInstance();
	if( True == pc_SoftDipSw->getSoftDipSwBitForPanelSpecNo( 142, 4 ) ) {
		if ( TYPE_SystemType_US == PnlComGetDestination() ) {
			TYPC_PaperSize c_PaperSize = ppc_FeedTray->getPaperSize( TYPE_FeedTray1 );
			if ( TYPE_SizeCodeB6Sef == c_PaperSize.e_SizeCode ) {
				if ( True == PnlComGetFeedTray()->getPaperSizeAutoDetectOn( TYPE_FeedTray1 ) ) {
					ppc_FeedTray->setPaperSize( TYPE_FeedTray1 ,TYPE_SizeCodeInvoiceSef );
					ppc_FeedTray->setPaperSizeAutoDetectOn(TYPE_FeedTray1, False);
				}
			}
		}
	}
#endif
	// 親のsetupGamenをコールする.
	PPopupDecisionButtonCom::setupGamen( option );
	if( True == PnlComGetPrintJobSetting()->getPaperTypeNormalEx() ){		// 普通紙用紙分割有効.
		Txt_PaperDirectionType.showEnable( False );
		Txt_PaperSize.showEnable(False);
		Txt_PaperDirectionNewTypeUser.showEnable( False );
		Tfd_PaperDirectionNewTypeUser.showEnable( False );
		Rot_NewPaperTypeIcon.showEnable( False );
		Bak_PaperSizeAndDirectionArea.showEnable( False );
		Bak_PaperSizeGrpBase.showEnable( False );
		Key_Ex.showEnable( True );
		if( TYPE_SystemType_JP == PnlComGetMachineSetting()->getSystemType() ){
			Key_Ex.changeText( s_pKEYNormalEcoPaper_W120H028A );
		} else {
			Key_Ex.changeText( s_pKEYNormalExPaper_W120H028A );
		}
		Bak_PaperSizeAndDirectionAreaEx.showEnable( True );
// comment 2015/12/29 HCLT-Ashok.D [MinervaSBK] [CQ523568] Handled info icon and warning message position.
 #ifndef DEF_Zeus5BkCus
		Gra_InfoIcon.changePosition(438, 151);
		Txt_InfoMsg.changePosition(478, 155);
#endif
	} else {
		Rot_NewPaperTypeIconEx.showEnable( False );
		Bak_PaperSizeAndDirectionAreaEx.showEnable( False );
		Txt_PaperSizeEx.showEnable(False);
		Key_Ex.showEnable( False );
		Bak_PaperSizeGrpBaseEx.showEnable( False );
		if( ( ( TYPE_FeedTray3 == getSelectedFeedTray() || TYPE_FeedTray4 == getSelectedFeedTray() ) 
			&& TYPE_TrayTypeLCC == getSelectedTrayType() ) || ( TYPE_TrayTypeLCT == getSelectedTrayType() ) ) {
			Bak_PaperSizeAndDirectionArea.showEnable( False );
		} else {
			Bak_PaperSizeAndDirectionArea.showEnable( True );
		}
// comment 2015/12/29 HCLT-Ashok.D [MinervaSBK] [CQ523568] Handled info icon and warning message position.
#ifndef DEF_Zeus5BkCus
		Gra_InfoIcon.changePosition(430, 96);
		Txt_InfoMsg.changePosition(470, 102);
#endif
	}

	// メッセージの変更.
	changeMessage();

	// 現在の選択トレイを取得.
	TYPE_FeedTray e_FeedTray = getSelectedFeedTray();			// comment MinervaZeus ADD Sky上西 2011/05/05 [UV-01038][機器1001].


	// 用紙の種類のボタンのシンボル表示.
	setPaperKindBtn();

	// タイトル表示.
	setupTitle( &Txt_BypassTraySettings );

	// 用紙種類の選択状態切替.
	APIC_FeedTray* pc_FeedTray = PnlComGetFeedTray();


//	// 用紙種類の判定と表示.

	TYPC_PaperType c_PaperType = convertPaperType( pc_FeedTray->getPaperType( e_FeedTray ), True );	// comment MinervaZeus CHG Sky上西 2011/05/05 [UV-01038][機器1001].

	const Bool b_2ndSidedPaper = c_PaperType.getFeed2ndSidedPaper();
// comment 2018/12/17 SCD2-Sky工藤 [EagleH][RQ-720]紙種自動検知 自動検知キーにグレーアウト条件を追加.
//// comment 2017/12/27 SCD2-4G Sky平田 [Eagle][RQ-720]紙種自動検知.
//	// メディア自動検知設定有効?.
//	if( True == PnlComGet_isPaperAutoDetectAdmin( e_FeedTray ) ) {
//		// メディア自動検知要求あり?
//		if( True == pc_FeedTray->getMediaAutoDetect( e_FeedTray ) ) {
//			Key_ChkPaperTypeAutoDetect.chkOn();
//		} else {
//			Key_ChkPaperTypeAutoDetect.chkOff();
//		}
//	}

	const TYPE_PaperType e_PaperType_Setting = getJobPaperType();										// カスタム用紙を含めた用紙種類を取得.
	Cmp_PaperTypeSelectList.setSelectPaperType( e_PaperType_Setting, PNLE_PaperTypeSelect_MoveNoAnimation );

	const TYPE_PaperType e_PaperType = c_PaperType.getPaperType();					// comment MinervaZeus ADD Sky上西 2011/05/05 [UV-01038][機器1001].
// comment 2015/04/16 HCLT_Jones [Helios][CQ512356] Hide the Eco key for all paper type except the Plain Paper.
// comment 2019/1/09 【IT6_DenebMLK】【HCLT_Nandhini】【AR1059379】 Warning Removal Fix.
#ifndef DEF_DenebMLKCus
	TYPE_CustomPaperType e_CustomPaperType = c_PaperType.getCustomPaperType();
#endif // DEF_DenebMLKCus.

// comment 2018/12/17 SCD2-Sky工藤 [EagleH][RQ-720]紙種自動検知 自動検知キーにグレーアウト条件を追加.
	// メディア自動検知設定有効?.
	if( True == PnlComGet_isPaperAutoDetectAdmin( e_FeedTray ) ) {
		// メディア自動検知要求あり?.
		if( True == pc_FeedTray->getMediaAutoDetect( e_FeedTray ) ) {
			Key_ChkPaperTypeAutoDetect.chkOn();
		} else {
			Key_ChkPaperTypeAutoDetect.chkOff();
		}
		if( ( 1 == PnlComGetPrinterServiceSetting()->getPrinterAdjust( TYPE_PrinterAdjustEnvelopeDetect ) ) && ( TYPE_PaperTypeEnvelope == e_PaperType ) ) { // 封筒検知がオフ(検知しない場合が1) かつ 用紙種類が封筒.
			Key_ChkPaperTypeAutoDetect.activate( False );	//自動検出キーのアクティブ/非アクティブを設定.
//Comment 2019/12/06 SCD2-Sky 岩渕 [EagleZ+_BK+][OP_BTS-13540] レターヘッド紙での自動検知は排他.
		} else if (False == PNLC_CopySettingLib::newInstance()->isAutoPaperTypeCheckObjDisp(e_PaperType)) {		// 紙種が自動検出表示対応.
			Key_ChkPaperTypeAutoDetect.activate( False );	//自動検出キーのアクティブ/非アクティブを設定.
		} else {											//上記以外の紙種は非表示.
			Key_ChkPaperTypeAutoDetect.activate( True );	//自動検出キーのアクティブ/非アクティブを設定.
		}
	}



	Bool b_ActInchSize = False;
	Bool b_SelectInchSize = False;

	TYPC_PaperSize c_PaperSize = pc_FeedTray->getPaperSize( e_FeedTray );
	c_PaperSize.c_SizeMM = c_PaperSize.c_SizeMM.xyReverse();	// 2010/04/26 Sky越野 JobSpecに入っているサイズはxy反転していることから、反転処理を追加した.

	if( ( False == c_PaperSize.b_WideOn )
	    && ( True == pc_FeedTray->getPaperSizeAutoDetectOn( e_FeedTray ) ) )
	{
		Key_AutoSearch.select( True );
	}
	else
	{
		Key_AutoSearch.select( False );
		if( ( TYPE_SizeCodeNonStandard == c_PaperSize.e_SizeCode )
		    && ( PCGD_1225_18PaperSizeX == c_PaperSize.c_SizeMM.x )
		    && ( PCGD_1225_18PaperSizeY == c_PaperSize.c_SizeMM.y ) ) {
			b_SelectInchSize = True;
		}
	}
	// 12×18検出時は「12_1/4x18」キーをアクティブにする.
	if( ( c_PaperSize.c_SizeMM.x == PCGD_12_18PaperSizeX && c_PaperSize.c_SizeMM.y == PCGD_12_18PaperSizeY )
	    || ( c_PaperSize.c_SizeMM.x == PCGD_1225_18PaperSizeX && c_PaperSize.c_SizeMM.y == PCGD_1225_18PaperSizeY ) )
	{
		b_ActInchSize = True;

	}

	Key_12x18.activate( b_ActInchSize );
// comment 2016/10/17 SCD2-2G南波 [IT5_4.0][RQ-226] 不定型紙の標準カセット対応.
	Key_12x18_Indeterminate.activate( b_ActInchSize );

	// 非アクティブの場合は未選択とする
	if( b_ActInchSize == False ) {
		b_SelectInchSize = False;
	}
	Key_12x18.select( b_SelectInchSize );

// comment 2014/12/24 【HCLT-Harish】【CQ507671】Add Text mm for Irregular Size display.
	if(b_SelectInchSize){
// comment 2016/01/12 SCD2-2G延原 [IT5_3.4][AR1011683].
//		if (TYPE_SystemType_US != PnlComGetDestination() ){
		if ( TYPE_UnitSystemMM_Number == PnlComGetUnitSystem() ){
			if( True == PnlComGetPrintJobSetting()->getPaperTypeNormalEx() ){
				Txt_XSizemmEx.showEnable(True);
				Txt_YSizemmEx.showEnable(True);
			} else {
				Txt_XSizemm.showEnable(True);
				Txt_YSizemm.showEnable(True);
			}
// comment 2016/01/12 SCD2-2G延原 [IT5_3.4][AR1011683].
//	} 	} else {
		} else {
			if( True == PnlComGetPrintJobSetting()->getPaperTypeNormalEx() ){
				Txt_XSizemmEx.showEnable(False);
				Txt_YSizemmEx.showEnable(False);
			} else {
				Txt_XSizemm.showEnable(False);
				Txt_YSizemm.showEnable(False);
			}
		}
	} else {
		if( True == PnlComGetPrintJobSetting()->getPaperTypeNormalEx() ){
			Txt_XSizemmEx.showEnable(False);
			Txt_YSizemmEx.showEnable(False);
		} else {
			Txt_XSizemm.showEnable(False);
			Txt_YSizemm.showEnable(False);
		}
	}
	// レッドヘッダ用紙キー表示切替.
	APIC_PrinterSystem* pc_PrinterSystem = PnlComGetPrinterSystem();
	if( pc_PrinterSystem->isExistDuplex() == False )
	{
		Key_OtherPaperLetterHead.showEnable( False );
	}

	// 両面2画面キーの表示切替.
	showActivateDoubleSide( e_PaperType, b_2ndSidedPaper );	// comment MinervaZeus Sky小尾 2011/04/15 [UV-01038_1：No.01] Mi/Zeで表示に差分があるので関数化.

// comment 2015/04/16 HCLT_Jones [Helios][CQ512356] Hide the Eco key for all paper type except the Plain Paper.
//  if ( TYPE_PaperTypeNormal == e_PaperType ) {
//comment 2018/12/12 HCLT-Basker [DenebMLK][制御構成変更_No.45 ] Remove the unneccessary PaperTypes from DenebMLK models.
#ifndef DEF_DenebMLKCus
	if( ( TYPE_PaperTypeNormal == e_PaperType ) && ( TYPE_CustomPaperType_UnSelect == e_CustomPaperType ) ) {
		Key_Ex.activate( True );
		if( True == c_PaperType.getPaperTypeNormalEx() ) {
			Key_Ex.chkOn();
		} else {
			Key_Ex.chkOff();
		}
	} else {
		Key_Ex.chkOff();
		Key_Ex.activate( False );
	}
#else // DEF_DenebMLKCus.
	Key_Ex.showEnable( False );
#endif // DEF_DenebMLKCus.

	setupPaperTypeImage();

	// 選択トレイの情報表示.
	PNLS_setOriginalSizeCLCD2Key s_parts;
	if( True == PnlComGetPrintJobSetting()->getPaperTypeNormalEx() ){			// 普通紙用紙分割有効.
		s_parts.ps_KeyOriginalSize      =	&Bak_PaperSizeAndDirectionAreaEx;			// サイズ表示エリア.
		s_parts.ps_TxtOriginalSize01	=	&Txt_DispPaperSize1Ex;					// １行目 用紙コード表示.
		s_parts.ps_TxtOriginalSize02	=	NULL;									// ２行目 自動表示.
		s_parts.ps_TxtOriginalSizeX		=	&Txt_XSizeText1Ex;						// 用紙サイズ表示 X.
		s_parts.ps_TxtOriginalSizeY		=	&Txt_YSizeText1Ex;						// 用紙サイズ表示 Y.
		s_parts.ps_NumOriginalSizeX		=	&Num_SizeXNum1Ex;							// 用紙サイズ数値表示 X.
		s_parts.ps_NumOriginalSizeY		=	&Num_SizeYNum1Ex;							// 用紙サイズ数値表示 Y.
// comment 2016/05/19 SCD2-2G-Sky福田 [ZeusSBK][CQ529248] TBD230対応.
//		s_parts.ps_TrayTxtPaperSize		=	&Ktx_PaperSizeAndDirectionArea;			// トレイ表示テキスト.
//		s_parts.ps_TrayIconPaperSize	=	&Gra_PaperSizeAndDirectionAreaEx;			// トレイ表示アイコン（手差しアイコン）.
		s_parts.ps_TrayTxtPaperSize		=	NULL;			// トレイ表示テキスト.
		s_parts.ps_TrayIconPaperSize	=	NULL;			// トレイ表示アイコン（手差しアイコン）.
		s_parts.ps_RemainIconPaperSize	=	NULL;	
		s_parts.ps_TypeIconPaperSize	=	NULL;
	} else {
// comment 2016/05/19 SCD2-2G-Sky福田 [ZeusSBK][CQ529248] TBD230対応.
		s_parts.ps_KeyOriginalSize      =	&Bak_PaperSizeAndDirectionArea;			// サイズ表示エリア.
		s_parts.ps_TxtOriginalSize01	=	&Txt_DispPaperSize1;					// １行目 用紙コード表示.
		s_parts.ps_TxtOriginalSize02	=	NULL;									// ２行目 自動表示.
		s_parts.ps_TxtOriginalSizeX		=	&Txt_XSizeText1;						// 用紙サイズ表示 X.
		s_parts.ps_TxtOriginalSizeY		=	&Txt_YSizeText1;						// 用紙サイズ表示 Y.
		s_parts.ps_NumOriginalSizeX		=	&Num_SizeXNum1;							// 用紙サイズ数値表示 X.
		s_parts.ps_NumOriginalSizeY		=	&Num_SizeYNum1;							// 用紙サイズ数値表示 Y.
//		s_parts.ps_TrayTxtPaperSize		=	&Ktx_PaperSizeAndDirectionArea;			// トレイ表示テキスト.
//		s_parts.ps_TrayIconPaperSize	=	&Gra_PaperSizeAndDirectionArea;			// トレイ表示アイコン（手差しアイコン）.
		s_parts.ps_TrayTxtPaperSize		=	NULL;			// トレイ表示テキスト.
		s_parts.ps_TrayIconPaperSize	=	NULL;			// トレイ表示アイコン（手差しアイコン）.
		s_parts.ps_RemainIconPaperSize	=	NULL;
		s_parts.ps_TypeIconPaperSize	=	NULL;
	}

// comment 2016/05/19 SCD2-2G-Sky福田 [ZeusSBK][CQ529248] TBD230対応.
//	PnlComsetPaperSizeCLCD2( &s_parts, e_FeedTray );
	PnlComsetPaperSizeCLCD2( &s_parts, e_FeedTray, PNLE_PaperSizeShowKind_StandardPaperSizeHide );
	
	// トレイアイコンの表示.
	setupPaperTray();
	
	// 定型用紙サイズの表示.
	setupStandardPaperSize();
	
// AS-PVD_14_15 用紙サイズ設定 kuroda Start.
#ifdef DEF_A4_Cus
	Gra_InfoIcon.showEnable( False );	// インフォアイコン.
	Txt_InfoMsg.showEnable( False );	// インフォメッセージ.
#else
// comment 2015.04.16 OPS1-Sky西原 [Helios][CQ511303] 用紙サイズ不明の場合もメッセージを表示する.
	Bool bl_Show = False;
	if( False == PnlComGetPrinterSystem()->isManualTrayFDSizeDetect() ) {			// FDセンサ無し.
		if( ( TYPE_FeedTrayManual == e_FeedTray )									// 手差しトレイ選択時.
			&& ( PERROR_ManualTrayUnmatchSet_S == sp_MessageWarningTable[onlyPanel.us_NowMessageWarning].ec_WarningName ) ) {		// 手差し用紙サイズ不一致(設定時)発生.
			//用紙サイズが未設定の場合.
			bl_Show = True;
		}
	} else {																		// FDセンサあり.
		if( ( TYPE_FeedTrayManual == e_FeedTray )									// 手差しトレイ選択時.
			&& ( TYPE_SizeCodeUnknown == c_PaperSize.e_SizeCode ) ) {				// 用紙不明の場合.
			bl_Show = True;
		}
	}
	Gra_InfoIcon.showEnable( bl_Show );	// インフォアイコン.
	Txt_InfoMsg.showEnable( bl_Show );	// インフォメッセージ.
#endif
// AS-PVD_14_15 用紙サイズ設定 kuroda End.

	if( True == APIC_SoftDipSw::newInstance()->getSoftDipSwBitForPanelSpecNo( 49, 6 )
	    && True == PnlComIsSpecificationListCounterPaperType( e_FeedTray ) ) {
// Comment 2023/10/18 FPT-QuyenTT9 [IT6_2.3][OP_BTS-45300] Hide Paper Type for tray 2,3,4 on the copy screen when DipSW49 bit6 ON and set papers to [Paper Type: Plain Paper+].
		Key_Ex.showEnable( False );							// Eco.
		Txt_PaperType.showEnable( False );					// 用紙種類タイトル.
		Bak_PaperTypeGrpBase.showEnable( False );			// 用紙種類のグループ背景.
		Cmp_PaperTypeSelectList.showEnable( False );		// 用紙種類選択リスト.
	} else {
		Txt_PaperType.showEnable( True );					// 用紙種類タイトル.
		Bak_PaperTypeGrpBase.showEnable( True );			// 用紙種類のグループ背景.
		Cmp_PaperTypeSelectList.showEnable( True );		// 用紙種類選択リスト.
	}

}

//-----------------------------------------------------------------------------------------------
/**
 *	@brief	A020114100 選択トレイの設定変更 : タイトルエリア変更処理.
 */
//-----------------------------------------------------------------------------------------------
void PCPC_PaperEnl_copy::setupTitle( PSinglePartsText* pc_SingleParts )
{
	if( NULL == pc_SingleParts )
	{
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return;
	}
	// 設定変更の対象となるトレイの取得.
	const TYPE_FeedTray e_FeedTray = getSelectedFeedTray();

#ifdef DEF_LCD_7INCH
	TEXT_ID puc_StrTitle = s_pITMBypassTraySettings_W212H024A;
			
	Bool b_ShowEnable = True;
			
	switch (e_FeedTray)
	{		
		case TYPE_FeedTrayManual:
			puc_StrTitle = s_pITMBypassTraySettings_W212H024A;
			break;
		case TYPE_FeedTray1:
			puc_StrTitle = s_pITM1stTraySettings_W212H024A;
			break;
		case TYPE_FeedTray2:
			puc_StrTitle = s_pITM2ndTraySettings_W212H024A;
			break;
		case TYPE_FeedTray3:
			puc_StrTitle = s_pITM3rdTraySettings_W212H024A;
			break;
		case TYPE_FeedTray4:
			puc_StrTitle = s_pITM4thTraySettings_W212H024A;
			break;
		case TYPE_FeedTray5:
// comment 2018/05/17 SCD2-Sky中野 [DenebMLK][機器構成変更No.7] 5段増設プリント機能追加対応.
#ifdef DEF_DenebMLKCus
			puc_StrTitle = s_pITM5thTraySettings_W212H024A;
#else
			puc_StrTitle = s_pITMLCTTraySettings_W212H024A;
// comment 2018/05/17 SCD2-Sky中野 [DenebMLK][機器構成変更No.7] 5段増設プリント機能追加対応.
#endif	// DEF_DenebMLKCus.
			break;
		default: b_ShowEnable = False;  break;
	}		
#else // #ifdef DEF_LCD_7INCH.
	TEXT_ID puc_StrTitle = s_pITMBypassTraySettings_W196H024A;

	Bool b_ShowEnable = True;

	switch( e_FeedTray )
	{
		case TYPE_FeedTrayManual:
			puc_StrTitle = s_pITMBypassTraySettings_W196H024A;
			break;
		case TYPE_FeedTray1:
			puc_StrTitle = s_pITM1stTraySettings_W196H024A;
			break;
		case TYPE_FeedTray2:
			puc_StrTitle = s_pITM2ndTraySettings_W196H024A;
			break;
		case TYPE_FeedTray3:
			puc_StrTitle = s_pITM3rdTraySettings_W196H024A;
			break;
		case TYPE_FeedTray4:
			puc_StrTitle = s_pITM4stTraySettings_W196H024A;
			break;
		case TYPE_FeedTray5:
			puc_StrTitle = s_pITMLCTTraySettings_W196H024A;
			break;
		default: b_ShowEnable = False;  break;
	}
#endif // #ifdef DEF_LCD_7INCH.
	
	// 表示するタイトルの判定.
	// タイトルの切り替え.
	pc_SingleParts->changeText( puc_StrTitle );
	pc_SingleParts->showEnable( b_ShowEnable );
}

//-----------------------------------------------------------------------------------------------
/**
 *	@brief	A020114100 選択トレイの設定変更 : 用紙種類コードの切り替え処理.
 */
//-----------------------------------------------------------------------------------------------
TYPC_PaperType PCPC_PaperEnl_copy::convertPaperType( TYPC_PaperType  c_PaperType, const Bool b_OnlyDisplay )
{

	// 用紙種類の取得.
	TYPE_PaperType e_PaperType = c_PaperType.getPaperType();
	TYPE_CustomPaperType e_CustomPaperType = c_PaperType.getCustomPaperType();

	// 用紙属性が普通紙かつ、カスタム用紙が20に設定されている場合
	if( ( TYPE_PaperTypeNormal == e_PaperType ) && ( TYPE_CustomPaperType20 == e_CustomPaperType ) ) {
		c_PaperType.setPaperType( TYPE_PaperRecycle1 );
		c_PaperType.setCustomPaperType( TYPE_CustomPaperType_UnSelect );
	}
	return c_PaperType;
}

//-----------------------------------------------------------------------------------------------
/**
 *	@brief	A020114100 選択トレイの設定変更 : 用紙種類テーブル取得処理.
 */
//-----------------------------------------------------------------------------------------------
const TYPS_PaperKindInfo* PCPC_PaperEnl_copy::getPaperKindInfo( short& ss_Type )
{
	const TYPS_PaperKindInfo* pst_PaperKindInfo = NULL;

	APIC_Job* pc_Job = PnlComGetJobCopy();
	if( NULL != pc_Job ) {
		TYPC_Print* pc_Print = pc_Job->getJobSpec()->pc_Print;
		TYPE_FeedTray e_FeedTray = TYPE_FeedTray1;
		e_FeedTray = pc_Print->c_PaperTray.getFeedTray();
		if( ( True == APIC_SoftDipSw::newInstance()->getSoftDipSwBitForPanelSpecNo( DEFSOFTWARESW205, 5 ) )		// DipSWNo.205 bit5 ON.
		    && ( TYPE_FeedTray1 == e_FeedTray || TYPE_FeedTray2 == e_FeedTray ) ) {
			ss_Type = PNLE_PEPARKIND_TYPE_STD;
			return &( st_PaperKindNewTbl_SP[0] );
		}
	}

	// 選択中のトレイが手差しの場合.
	if( TYPE_TrayTypeMultiMan == getSelectedTrayType() ||
	    TYPE_TrayTypeSingleMan == getSelectedTrayType() )
	{
		ss_Type = PNLE_PEPARKIND_TYPE_MUL;
		pst_PaperKindInfo = &( st_PaperKindNewTblMultiMan[0] );
	}
	else if( TYPE_TrayTypeNon !=  getSelectedTrayType() )
	{
		ss_Type = PNLE_PEPARKIND_TYPE_STD;
// A3A4FUM 用紙種類設定 t-okada 2015/01/19 Start 選択トレイの設定変更.
#ifndef DEF_A4_Cus
//Comment [IT6-Eagle] 機器構成変更No.2 HCLT-Deeparaj 2018/02/20 Assign new table for tray 1.
		if( TYPE_FeedTray1 == getSelectedFeedTray()){
			ss_Type = PNLE_PEPARKIND_TYPE_TRY1;
			pst_PaperKindInfo = &( st_PaperKindNewTblTry1[0] );
		} else {
		pst_PaperKindInfo = &( st_PaperKindNewTbl[0] );
		}
#else // DEF_A4_Cus.
		// 対象トレイの用紙種類テーブル.
		uchar uc_idx = getTrayTblIdxFromFeedTray(getSelectedFeedTray());
		pst_PaperKindInfo = &(st_PaperKindNewTbl[uc_idx][0]);
#endif // DEF_A4_Cus.
// A3A4FUM 用紙種類設定 t-okada 2015/01/19 End.
	}
	else
	{
		ss_Type = PNLE_PEPARKIND_TYPE_ERR;
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
	}
	return pst_PaperKindInfo;
}

//-----------------------------------------------------------------------------------------------
/**
 *	@brief	A020114100 選択トレイの設定変更 : 用紙アイコン描画処理.
 */
//-----------------------------------------------------------------------------------------------
void PCPC_PaperEnl_copy::setupPaperTypeImage()
{
	// 設定変更の対象となるトレイの取得.
	const TYPE_FeedTray e_FeedTray = getSelectedFeedTray();
	// 用紙種類、両面2面目フラグの取得.

	TYPC_PaperType c_PaperType = convertPaperType( PnlComGetFeedTray()->getPaperType( e_FeedTray ), True );
	TYPE_PaperType e_PaperType = c_PaperType.getPaperType();

	short ss_Type;
	// 用紙種類テーブルの取得.
	const TYPS_PaperKindInfo* pst_AryPaperKindInfo = getPaperKindInfo( ss_Type );
// Comment 2015/11/12【HCLT-Priya C】【CQ519679】【ID: 33165】NPD.FUNC.MUST.
	if( NULL == pst_AryPaperKindInfo ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return;
	}
	const uchar uc_PaperKindInfoSize = getPaperKindInfoSize();
	// 用紙種類の検索.
	short ss_SelectPaperType = -1;
	uchar uc_PaperWeight = 0;
	for( uchar i = 0; uc_PaperKindInfoSize > i; ++i )
	{
		if( e_PaperType == pst_AryPaperKindInfo[i].e_PaperType )
		{
			ss_SelectPaperType = getPaperTypeIconIndex( e_PaperType );
			uc_PaperWeight = ( uchar )pst_AryPaperKindInfo[i].uc_PaperWeightIndex;
		}
	}

	Bool b_DispPaperTypeUser = False;// 坪量テキストパーツ表示フラグ (False : 固定文字列パーツ / True : 可変文字列パーツ).

// Comment KronosS -> 2017/03/01 SCD2 3G Sky瀬戸 [AccuRev.1012111]ユーザー紙坪量問題.
//comment 2018/12/11 HCLT-Basker [DenebMLK][制御構成変更_No.45 ] Remove the unneccessary PaperTypes from DenebMLK models.
//#ifndef DEF_KRONOSS_Cus
#if !defined (DEF_KRONOSS_Cus) && !defined(DEF_DenebMLKCus)
	if( ( TYPE_PaperTypeUser1 == e_PaperType ) ||
	    ( TYPE_PaperTypeUser2 == e_PaperType ) ||
	    ( TYPE_PaperTypeUser3 == e_PaperType ) ||
	    ( TYPE_PaperTypeUser4 == e_PaperType ) ||
	    ( TYPE_PaperTypeUser5 == e_PaperType ) ||
// comment 2019/06/28 SCD4-Sky西村 [EagleZ+][機器構成変更No.1] ユーザー紙C 追加対応.
//	    ( TYPE_PaperTypeUser6 == e_PaperType ) )
	    ( TYPE_PaperTypeUser6 == e_PaperType ) ||
	    ( TYPE_PaperTypeUser7 == e_PaperType ) )
	{
		short ss_DispNum = 0;
		char sc_AryDisp[4];
		memset( sc_AryDisp, '\0', sizeof( sc_AryDisp ) );

		for( uchar uc_Count = 0; uc_TypeUserConversionSize > uc_Count ; uc_Count++ )
		{
			if( st_NewTypeUserConversion[uc_Count].e_PaperType == e_PaperType )
			{
				ss_DispNum = PnlComGetPrinterServiceSetting()->getPrinterAdjust( st_NewTypeUserConversion[uc_Count].e_PrinterAdjust );
				break;
			}
		}
		snprintf( sc_AryDisp, sizeof( sc_AryDisp ), "%d", ss_DispNum );
		Tfd_PaperDirectionNewTypeUser.changeASCIIText( ( uchar* )sc_AryDisp );
		Tfd_PaperDirectionNewTypeUserEx.changeASCIIText( (uchar*)sc_AryDisp );
		b_DispPaperTypeUser = True;
	}
#endif // DEF_KRONOSS_Cus.
// Comment KronosS <- 2017/03/01 SCD2 3G Sky瀬戸 [AccuRev.1012111]ユーザー紙坪量問題.

	// 用紙種類・重量の切り替え.
	if( True == APIC_SoftDipSw::newInstance()->getSoftDipSwBitForPanelSpecNo( 49, 6 ) ) {
		Txt_PaperDirectionType.showEnable( False );			// 坪量.
		Tfd_PaperDirectionNewTypeUser.showEnable( False );
		Txt_PaperDirectionNewTypeUser.showEnable( False );
		Rot_NewPaperTypeIcon.showEnable( False );			// 用紙種類アイコン.
		Txt_PaperDirectionTypeEx.showEnable( False );			// 坪量.
		Tfd_PaperDirectionNewTypeUserEx.showEnable( False );
		Txt_PaperDirectionNewTypeUserEx.showEnable( False );
		Rot_NewPaperTypeIconEx.showEnable( False );			// 用紙種類アイコン.
	} else {
		if( True == PnlComGetPrintJobSetting()->getPaperTypeNormalEx() ){		// 普通紙用紙分割有効.
			Rot_NewPaperTypeIconEx.showEnable( True );			// 用紙種類アイコン.
			if(0 <= ss_SelectPaperType)	{
				// comment 2019/01/22 SCD2-4G Sky武井 OP_BTS-2216【開発提起】【Nランク】【RQ-720】紙種自動検知がONで用紙種類が未確定の時に、未確定であることがわかるアイコンを用紙種類の設定値表示箇所に表示する。.
//				Rot_NewPaperTypeIconEx.changeGraphic( ( uchar )ss_SelectPaperType );
				uchar uc_Index = ( uchar )ss_SelectPaperType;
				if ( True == PnlComGet_isMediaUndecided( e_FeedTray ) ) {
					uc_Index = getMediaUndecidedIndex( uc_Index );
				}
				Rot_NewPaperTypeIconEx.changeGraphic( uc_Index );
// Comment 2019/05/24 SCD2-Sky谷尾 [EagleH][OP_BTS-7302]UD指摘対応.
			} else if( True == PnlComGet_isPaperAutoDetectAdmin( TYPE_FeedTrayUnselect ) ) {	// メディア自動検知有効機種のみ.
				uchar uc_Index = getPaperTypeIconIndex( e_PaperType );
				if ( True == PnlComGet_isMediaUndecided( e_FeedTray ) ) {
					uc_Index = getMediaUndecidedIndex( uc_Index );
				}
				Rot_NewPaperTypeIconEx.changeGraphic( uc_Index );
			} else {
				PnlComPutDebugLog( True, UC_PNL_LOGPATTERN_1, "PCPC_PaperEnl_copy::setupPaperTypeImage", UC_PNL_ERROR_PARAMETER );
			}
			TEXT_ID us_TextID = puc_AryNewPaperWeight[uc_PaperWeight];
// Comment 2019/05/24 SCD2-Sky谷尾 [EagleH][OP_BTS-7302]UD指摘対応.
			if( -1 == ss_SelectPaperType ) {	// トレイサポート外
				if( True == PnlComGet_isPaperAutoDetectAdmin( TYPE_FeedTrayUnselect ) ) {	// メディア自動検知有効機種のみ.
					uchar uc_PaperWeightIndex = 0;
					switch( e_PaperType ) {
// comment 2022/07/08 FPT.ThanhLH11 [IT6_2.2MR3][HeliosMLK][Klocwork][ABV.GENERAL].
#ifndef DEF_HeliosMLKCus
						case TYPE_PaperTypeThick4:	uc_PaperWeightIndex = PNLE_PAPERTYPE_WEIGHT_THICK4;	break;	// 厚紙4.
#endif // DEF_HeliosMLKCus
						case TYPE_PaperTypeThin1:	uc_PaperWeightIndex = PNLE_PAPERTYPE_WEIGHT_THIN;	break;	// 薄紙.
						default:					uc_PaperWeightIndex = 0;							break;
					}
					us_TextID = puc_AryNewPaperWeight[uc_PaperWeightIndex];
				}
			}
//comment 2018/12/12 HCLT-Basker [DenebMLK][制御構成変更_No.45 ] Remove the unneccessary PaperTypes from DenebMLK models.
#ifndef DEF_DenebMLKCus
			if ( TYPE_PaperTypeNormal == e_PaperType ) {						// 普通紙.
				if ( True == c_PaperType.getPaperTypeNormalEx() ) {
					us_TextID	= s_pNOT60to70gm2_W0128H018A;
				} else {
//comment 2019/09/12 HCLT-Shalini] [IT6_EagleH][OP_BTS-13399 ] Changing the Paper Weight.
////comment 2019/07/08 HCLT-Sivteja] [IT6_EagleH][OP_BTS-4773 ] fix done for ECO/Alterthickness selction according to the paper type and destination.
////				us_TextID	= s_pNOT60to90gm2_W128H018A;
// 					us_TextID	= s_pNOTITM71to90gm2_W128H018A;
					us_TextID	= s_pNOT60to90gm2_W128H018A;
				}
			}
			if( TYPE_PaperRecycle1 == e_PaperType ){
//comment 2019/09/12 HCLT-Shalini] [IT6_EagleH][OP_BTS-13399 ] Changing the Paper Weight.
//				us_TextID	= s_pNOTITM71to90gm2_W128H018A;
				us_TextID	= s_pNOT60to90gm2_W128H018A;
			}
#endif // DEF_DenebMLKCus.
			Txt_PaperDirectionTypeEx.changeText( us_TextID );
			if( False == b_DispPaperTypeUser )		
			{		
				Txt_PaperDirectionNewTypeUserEx.showEnable( False );
				Tfd_PaperDirectionNewTypeUserEx.showEnable( False );
				Txt_PaperDirectionTypeEx.showEnable( True );
			} else {
				Txt_PaperDirectionTypeEx.showEnable( False );
				Tfd_PaperDirectionNewTypeUserEx.showEnable( True );
				Txt_PaperDirectionNewTypeUserEx.showEnable( True );
			}
		} else {
			Rot_NewPaperTypeIcon.showEnable( True );			// 用紙種類アイコン.
			if(0 <= ss_SelectPaperType)	{
				// comment 2019/01/22 SCD2-4G Sky武井 OP_BTS-2216【開発提起】【Nランク】【RQ-720】紙種自動検知がONで用紙種類が未確定の時に、未確定であることがわかるアイコンを用紙種類の設定値表示箇所に表示する。.
//				Rot_NewPaperTypeIcon.changeGraphic( ( uchar )ss_SelectPaperType );
				uchar uc_Index = ( uchar )ss_SelectPaperType;
				if ( True == PnlComGet_isMediaUndecided( e_FeedTray ) ) {
					uc_Index = getMediaUndecidedIndex( uc_Index );
				}
				Rot_NewPaperTypeIcon.changeGraphic( uc_Index );
// Comment 2019/05/24 SCD2-Sky谷尾 [EagleH][OP_BTS-7302]UD指摘対応.
			} else if( True == PnlComGet_isPaperAutoDetectAdmin( TYPE_FeedTrayUnselect ) ) {	// メディア自動検知有効機種のみ.
				uchar uc_Index = getPaperTypeIconIndex( e_PaperType );
				if ( True == PnlComGet_isMediaUndecided( e_FeedTray ) ) {
					uc_Index = getMediaUndecidedIndex( uc_Index );
				}
				Rot_NewPaperTypeIcon.changeGraphic( uc_Index );
			} else {
				PnlComPutDebugLog( True, UC_PNL_LOGPATTERN_1, "PCPC_PaperEnl_copy::setupPaperTypeImage", UC_PNL_ERROR_PARAMETER );
			}
// comment 2017/01/16 【HCL_Jagadeesh】 【IT5_4.0】 【AR_1010532】 basic weight display support.
//		Txt_PaperDirectionType.changeText( puc_AryNewPaperWeight[uc_PaperWeight] );
		PNLC_SelectPaperTrayCom_DataLib* pDataLib = PNLC_SelectPaperTrayCom_DataLib::newInstance();
		if ( NULL != pDataLib ){
			Txt_PaperDirectionType.changeText( pDataLib->getUserDirectionTypeString( e_PaperType ) );
		}
			
		if( False == b_DispPaperTypeUser )
		{
			Txt_PaperDirectionNewTypeUser.showEnable( False );
			Tfd_PaperDirectionNewTypeUser.showEnable( False );
			Txt_PaperDirectionType.showEnable( True );
			} else {
			Txt_PaperDirectionType.showEnable( False );
			Tfd_PaperDirectionNewTypeUser.showEnable( True );
			Txt_PaperDirectionNewTypeUser.showEnable( True );
		}
	}
	}
}

//-----------------------------------------------------------------------------------------------
/**
 *	@brief	A020114100 選択トレイの設定変更 : ボタン押下処理.
 */
//-----------------------------------------------------------------------------------------------
void PCPC_PaperEnl_copy::procButtonOn( uchar option )
{
	// 設定変更の対象となるトレイの取得.
	const TYPE_FeedTray e_FeedTray = getSelectedFeedTray();	
	// 用紙の種類を取得.
	APIC_FeedTray* pc_FeedTray = PnlComGetFeedTray();
	TYPC_PaperSize c_PaperSize = PnlComGetFixedPaperSizeCodeToMM( TYPE_SizeCodeNonStandard );			// 12 1/4*18サイズの用紙は不定形として扱う【CQ.347020】第一OSW 酒井(健).
	if( ( NULL == PnlComGetJobCopy() ) ||
	    ( NULL == PnlComGetJobCopy()->getJobSpec() ) ||
	    ( NULL == PnlComGetJobCopy()->getJobSpec()->pc_Print ) ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "[PCPC_PaperEnl_copy::procButtonOn]Assert Failed!" );
		return;
	}
	TYPC_Print* pc_Print = PnlComGetJobCopy()->getJobSpec()->pc_Print;

	short ss_xPositon = 0;
	short ss_yPosition = 0;
	short ss_width = 0;
	short ss_height = 0;
	m_uc_CustomPaperTypeBtnID = option;		// カスタム用紙種類ボタンID.

	PNLC_CopyDataLib* pc_CopyDataLib = PNLC_CopyDataLib::newInstance();
	if( NULL == pc_CopyDataLib ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "[PCPC_PaperEnl_copy::procButtonOn]Assert Failed!" );
		return;
	}

// comment 2017/12/27 SCD2-4G Sky平田 [Eagle][RQ-720]紙種自動検知.
	Bool	bl_UpdateMediaAutoDetect = False;
	Bool	bl_MediaAutoDetectStatus = False;

// Comment 2020/04/18 [HCL_Prasanjeet][IT6_EagLeZ+][OP_BTS-22867] paper type canceletion.
	Bool isBeforePostCardOrEnv = False;
	
	TYPC_PaperType c_PaperTypeAuto = pc_FeedTray->getPaperType(e_FeedTray);
	TYPC_PaperSize c_PaperSizeAuto = pc_FeedTray->getPaperSize( e_FeedTray );

    if( ( TYPE_SizeCodePostcard_JpSef == c_PaperSizeAuto.e_SizeCode )
#ifdef DEF_A4_Cus
        || (TYPE_SizeCodeDoublePostcard_JpSef == c_PaperSizeAuto.e_SizeCode)
#endif
        || ( TYPE_SizeCodeIN4x6Sef == c_PaperSizeAuto.e_SizeCode )
        || ( TYPE_SizeCodePostcard_EuSef == c_PaperSizeAuto.e_SizeCode ) 
        || ( TYPE_SizeCodeEnvelopeB5 == c_PaperSizeAuto.e_SizeCode )
#ifdef DEF_A4_Cus
        || (TYPE_SizeCodeJpLetterY2Lef == c_PaperSizeAuto.e_SizeCode)
        || (TYPE_SizeCodeEnvelopeDLLef == c_PaperSizeAuto.e_SizeCode)
        || (TYPE_SizeCodeEnvelopeMornarchLef == c_PaperSizeAuto.e_SizeCode)
        || (TYPE_SizeCodeJpLetterY3Lef == c_PaperSizeAuto.e_SizeCode)        
#endif
        || ( TYPE_SizeCodeEnvelopeC5 == c_PaperSizeAuto.e_SizeCode )
        || ( TYPE_SizeCodeEnvelopeDL == c_PaperSizeAuto.e_SizeCode )
        || ( TYPE_SizeCodeEnvelopeCom10 == c_PaperSizeAuto.e_SizeCode )
        || ( TYPE_SizeCodeEnvelopeMornarch == c_PaperSizeAuto.e_SizeCode )
        || ( TYPE_SizeCodeEnvelopeC4 == c_PaperSizeAuto.e_SizeCode )
        || ( TYPE_SizeCodeJpLetterY2 == c_PaperSizeAuto.e_SizeCode )
        || ( TYPE_SizeCodeJpLetterY3 == c_PaperSizeAuto.e_SizeCode )
        || ( TYPE_SizeCodeJpLetterY4 == c_PaperSizeAuto.e_SizeCode )
        || ( TYPE_SizeCodeJpLetterL3 == c_PaperSizeAuto.e_SizeCode )
        || ( TYPE_SizeCodeJpLetterL4 == c_PaperSizeAuto.e_SizeCode )
        || ( TYPE_SizeCodeJpLetterK1 == c_PaperSizeAuto.e_SizeCode )
        || ( TYPE_SizeCodeJpLetterK2 == c_PaperSizeAuto.e_SizeCode )
        || ( TYPE_SizeCodeJpLetterK3 == c_PaperSizeAuto.e_SizeCode ) ){
            isBeforePostCardOrEnv = True;
    }


	switch( option )
	{
		case BTNID_KEY_AUTOSERCH:				// 自動検出.
		{
			setAutoDetectionPaperSize();			// 用紙サイズの自動検出設定.
// Comment 2020/04/18 [HCL_Prasanjeet][IT6_EagLeZ+][OP_BTS-22867] paper type canceletion.
			if(True == isBeforePostCardOrEnv) {
				c_PaperTypeAuto.setPaperType( TYPE_PaperTypeNormal );
				pc_FeedTray->setPaperType(e_FeedTray, c_PaperTypeAuto);
			}
		}
		break;
		case BTNID_KEY_PAPERKINDP1_1:				// 用紙種類P1_1キー.
		case BTNID_KEY_PAPERKINDP1_2:				// 用紙種類P1_2キー.
		case BTNID_KEY_PAPERKINDP1_3:				// 用紙種類P1_3キー.
		case BTNID_KEY_PAPERKINDP1_4:				// 用紙種類P1_4キー.
		case BTNID_KEY_PAPERKINDP1_5:				// 用紙種類P1_5キー.
		case BTNID_KEY_PAPERKINDP1_6:				// 用紙種類P1_6キー.
		case BTNID_KEY_PAPERKINDP1_7:				// 用紙種類P1_7キー.
		case BTNID_KEY_PAPERKINDP1_8:				// 用紙種類P1_8キー.
		case BTNID_KEY_PAPERKINDP1_9:				// 用紙種類P1_9キー.
		case BTNID_KEY_PAPERKINDP1_10:				// 用紙種類P1_10キー.
		case BTNID_KEY_PAPERKINDP1_11:				// 用紙種類P1_11キー.		// comment 2018/03/29 SCD2 K.Sato [Eagle][RQ-722]用紙種別追加（厚紙1を2種類に分割）対応.
		case BTNID_KEY_PAPERKINDP2_1:				// 用紙種類P2_1キー.
		case BTNID_KEY_PAPERKINDP2_2:				// 用紙種類P2_2キー.
		case BTNID_KEY_PAPERKINDP2_3:				// 用紙種類P2_3キー.
		case BTNID_KEY_PAPERKINDP2_4:				// 用紙種類P2_4キー.
		case BTNID_KEY_PAPERKINDP2_5:				// 用紙種類P2_5キー.
		case BTNID_KEY_PAPERKINDP2_6:				// 用紙種類P2_6キー.
		case BTNID_KEY_PAPERKINDP2_7:				// 用紙種類P2_7キー.
		case BTNID_KEY_PAPERKINDP2_8:				// 用紙種類P2_8キー.
		case BTNID_KEY_PAPERKINDP2_9:				// 用紙種類P2_9キー.
		case BTNID_KEY_PAPERKINDP2_10:				// 用紙種類P2_10キー.
		case BTNID_KEY_PAPERKINDP3_1:				// 用紙種類P3_1キー.	// comment MinervaZeus ADD Sky上西 2011/05/05 [UV-01038][機器1001].
		case BTNID_KEY_PAPERKINDP3_2:				// 用紙種類P3_2キー.	// comment 2019/06/28 SCD4-Sky西村 [EagleZ+][機器構成変更No.1] ユーザー紙C 追加対応.
			m_e_PaperTypeCategory = PNLE_PaperTypeCategory_Normal;		// 用紙種類カテゴリ.
			setPaperType( option ); 					// 用紙種類の設定.
// comment 2017/12/27 SCD2-4G Sky平田 [Eagle][RQ-720]紙種自動検知.
			bl_UpdateMediaAutoDetect = True;
			bl_MediaAutoDetectStatus = False;
			break;
		case BTNID_KEY_PAPERKINDCUSTOM_1:			// カスタム_1.
		case BTNID_KEY_PAPERKINDCUSTOM_2:			// カスタム_2.
		case BTNID_KEY_PAPERKINDCUSTOM_3:			// カスタム_3.
		case BTNID_KEY_PAPERKINDCUSTOM_4:			// カスタム_4.
		case BTNID_KEY_PAPERKINDCUSTOM_5:			// カスタム_5.
		case BTNID_KEY_PAPERKINDCUSTOM_6:			// カスタム_6.
		case BTNID_KEY_PAPERKINDCUSTOM_7:			// カスタム_7.
		case BTNID_KEY_PAPERKINDCUSTOM_8:			// カスタム_8.
		case BTNID_KEY_PAPERKINDCUSTOM_9:			// カスタム_9.
		case BTNID_KEY_PAPERKINDCUSTOM_10:			// カスタム_10.
		case BTNID_KEY_PAPERKINDCUSTOM_11:			// カスタム_11.
		case BTNID_KEY_PAPERKINDCUSTOM_12:			// カスタム_12.
		case BTNID_KEY_PAPERKINDCUSTOM_13:			// カスタム_13.
		case BTNID_KEY_PAPERKINDCUSTOM_14:			// カスタム_14.
		case BTNID_KEY_PAPERKINDCUSTOM_15:			// カスタム_15.
		case BTNID_KEY_PAPERKINDCUSTOM_16:			// カスタム_16.
		case BTNID_KEY_PAPERKINDCUSTOM_17:			// カスタム_17.
		case BTNID_KEY_PAPERKINDCUSTOM_18:			// カスタム_18.
		case BTNID_KEY_PAPERKINDCUSTOM_19:			// カスタム_19.
		case BTNID_KEY_PAPERKINDCUSTOM_20:			// カスタム_20.
			m_e_PaperTypeCategory = PNLE_PaperTypeCategory_Custom;		// 用紙種類カテゴリ.
			setCustomPaperType( option );
// comment 2017/12/27 SCD2-4G Sky平田 [Eagle][RQ-720]紙種自動検知.
			bl_UpdateMediaAutoDetect = True;
			bl_MediaAutoDetectStatus = False;
			break;
		case BTNID_KEY_DOUBLESIDE:					// 両面2面目キー.
			setPaperback(); 						// 両面2面目の設定.
			break;
		case BTNID_KEY_EX:	// Exキー.
			setPaperEx();
			break;
//		case BTNID_KEY_GOTOFIXPAPER:				// 定型サイズキー.
//			Key_StandardSize.getPosition( ss_xPositon, ss_yPosition );
//			Key_StandardSize.getSize( ss_width, ss_height );
//			break;
		case BTNID_KEY_GOTOENVSIZE:					// はがき／封筒キー.
			pc_CopyDataLib->setCopyPaperTypeButton( PNLD_ButtonPaperKindEnvelope );
			Key_EnvelopeSize.getPosition( ss_xPositon, ss_yPosition );
			Key_EnvelopeSize.getSize( ss_width, ss_height );
			PNLC_PopupListControl::newInstance()->addPopup( &gp_PaperSizeMetric_copy, this, NULL, ss_xPositon, ss_yPosition, ss_width, ss_height );
			break;
		case BTNID_KEY_GOTO8K16KSIZE:				// その他キー.
			pc_CopyDataLib->setCopyPaperTypeButton( PNLD_ButtonPaperKindOther );
			Kex_8k16kSize.getPosition( ss_xPositon, ss_yPosition );
			Kex_8k16kSize.getSize( ss_width, ss_height );
			PNLC_PopupListControl::newInstance()->addPopup( &gp_PaperSizeMetric_copy, this, NULL, ss_xPositon, ss_yPosition, ss_width, ss_height );
			break;
		case BTNID_KEY_GOTOMETRICSIZE:				// AB系キー.
			pc_CopyDataLib->setCopyPaperTypeButton( PNLD_ButtonPaperKindAB );
			Kex_MetricSize.getPosition( ss_xPositon, ss_yPosition );
			Kex_MetricSize.getSize( ss_width, ss_height );
			PNLC_PopupListControl::newInstance()->addPopup( &gp_PaperSizeMetric_copy, this, NULL, ss_xPositon, ss_yPosition, ss_width, ss_height );
			break;
		case BTNID_KEY_GOTOINCHSIZE:				// インチ系キー.
			pc_CopyDataLib->setCopyPaperTypeButton( PNLD_ButtonPaperKindInch );
			Kex_InchSize.getPosition( ss_xPositon, ss_yPosition );
			Kex_InchSize.getSize( ss_width, ss_height );
			PNLC_PopupListControl::newInstance()->addPopup( &gp_PaperSizeMetric_copy, this, NULL, ss_xPositon, ss_yPosition, ss_width, ss_height );
			break;
		case BTNID_KEY_SELECTPAPERSIZE_1:			// 用紙候補１キー.
		case BTNID_KEY_SELECTPAPERSIZE_2:			// 用紙候補２キー.
		case BTNID_KEY_SELECTPAPERSIZE_3:			// 用紙候補３キー.
		{
			PNLC_DetectPaperSizeLib* cp_DetectPaperSizeLib = PNLC_DetectPaperSizeLib::newInstance();
			if( NULL == cp_DetectPaperSizeLib ) {
				LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
				return;
			}
			PNLS_PaperSizeDisp st_DetectPaperSize;
			cp_DetectPaperSizeLib->getDispPaperSizeinfo( &st_DetectPaperSize );
			c_PaperSize = PnlComGetFixedPaperSizeCodeToMM( st_DetectPaperSize.m_st_paperSizeDispParts[ option - BTNID_KEY_SELECTPAPERSIZE_1 ].e_PaperSize );
//	comment 2015/03/13 HCLT-Durga [HeliosL][CQ510997&CQ510999]  To set the Thick3 for A6card Type.
				// If Post card key selected set the paper type to Thick3.
//comment 2019/01/10 HCLT-Basker [DenebMLK][AR1071302] Remove special condition for setting thick3 papertype if 4x6, A6 card paper size id selected.
#ifndef DEF_DenebMLKCus
				if (   (TYPE_SizeCodePostcard_JpSef == c_PaperSize.e_SizeCode)
					|| (TYPE_SizeCodeIN4x6Sef == c_PaperSize.e_SizeCode)
					|| (TYPE_SizeCodePostcard_EuSef == c_PaperSize.e_SizeCode)) {

					TYPC_PaperType c_PaperType = pc_FeedTray->getPaperType(e_FeedTray);
					TYPC_Print* pc_Print = PnlComGetJobCopy()->getJobSpec()->pc_Print;
					const TYPE_WidePaperImagePosition e_WidePaperImagePosition = c_PaperType.getWidePaperImagePosition();
					// 用紙種類をジョブに設定.
					pc_Print->c_PaperType.setPaperType(TYPE_PaperTypeThick3);
					pc_Print->c_PaperType.setFeed2ndSidedPaper(False);
					pc_Print->c_PaperType.setWidePaperImagePosition(e_WidePaperImagePosition);
					// カスタム用紙属性をジョブに設定.
					pc_Print->c_PaperType.setCustomPaperType( TYPE_CustomPaperType_UnSelect );
					//To disable the ECO key.
					pc_Print->c_PaperType.setPaperTypeNormalEx( False );
					// 自動検出をオフ.
					pc_FeedTray->setPaperSizeAutoDetectOn(e_FeedTray, False);
// Comment 2020/04/18 [HCL_Prasanjeet][IT6_EagLeZ+][OP_BTS-22867] paper type canceletion.
			} else if( ( TYPE_SizeCodePostcard_JpSef != c_PaperSize.e_SizeCode )
					&& (TYPE_SizeCodeIN4x6Sef != c_PaperSize.e_SizeCode)
					&& (TYPE_SizeCodeIN4x6Sef != c_PaperSize.e_SizeCode)
					&& (TYPE_SizeCodeEnvelopeB5 != c_PaperSize.e_SizeCode )
#ifdef DEF_A4_Cus
					&& (TYPE_SizeCodeJpLetterY2Lef != c_PaperSize.e_SizeCode)
					&& (TYPE_SizeCodeEnvelopeDLLef != c_PaperSize.e_SizeCode)
					&& (TYPE_SizeCodeEnvelopeMornarchLef != c_PaperSize.e_SizeCode)
					&& (TYPE_SizeCodeJpLetterY3Lef != c_PaperSize.e_SizeCode)
#endif
					&& (TYPE_SizeCodeEnvelopeC5 != c_PaperSize.e_SizeCode)
					&& (TYPE_SizeCodeEnvelopeDL != c_PaperSize.e_SizeCode)
					&& (TYPE_SizeCodeEnvelopeCom10 != c_PaperSize.e_SizeCode)
					&& (TYPE_SizeCodeEnvelopeMornarch != c_PaperSize.e_SizeCode)
					&& (TYPE_SizeCodeEnvelopeC4 != c_PaperSize.e_SizeCode)
					&& (TYPE_SizeCodeJpLetterY2 != c_PaperSize.e_SizeCode)
					&& (TYPE_SizeCodeJpLetterY3 != c_PaperSize.e_SizeCode)
					&& (TYPE_SizeCodeJpLetterY4 != c_PaperSize.e_SizeCode)
					&& (TYPE_SizeCodeJpLetterL3 != c_PaperSize.e_SizeCode)
					&& (TYPE_SizeCodeJpLetterL4 != c_PaperSize.e_SizeCode)
					&& (TYPE_SizeCodeJpLetterK1 != c_PaperSize.e_SizeCode)
					&& (TYPE_SizeCodeJpLetterK2 != c_PaperSize.e_SizeCode)
					&& (TYPE_SizeCodeJpLetterK3 != c_PaperSize.e_SizeCode ) ) {
						pc_Print->c_PaperType.setPaperType(TYPE_PaperTypeNormal);
			}
#endif // DEF_DenebMLKCus.
			// 用紙サイズをジョブに設定.
			pc_Print->c_PaperSize = c_PaperSize;
			// トレイへの設定.
			onlyPanel.setFeedSizeKind( e_FeedTray );
			// APS/AMS処理関数.
			selectAutoPaperMag();
			// 押下ボタン取得.
			m_sc_PaperSizeSelect = ( char )( option - BTNID_KEY_SELECTPAPERSIZE_1 );
		}
		break;
		case BTNID_KEY_CHOICESIZE:					// 検知サイズ候補タブキー.
			m_sc_PaperSizeTab = PNLD_PaperSizeTabDetection;
			break;
		case BTNID_KEY_OTHERSIZE:					// その他サイズタブキー.
			m_sc_PaperSizeTab = PNLD_PaperSizeTabOther;
			break;
		case BTNID_KEY_GOTONOFIXPAPER:				// 不定型サイズキー.
			Key_CustomSize.getPosition( ss_xPositon, ss_yPosition );
			Key_CustomSize.getSize( ss_width, ss_height );
			PNLC_PopupListControl::newInstance()->addPopup( &gp_A020114120_6, this, NULL, ss_xPositon, ss_yPosition, ss_width, ss_height );
			break;
		case BTNID_KEY_GOTOWIDEPAPER:				// ワイド紙キー.
			Key_WidePaper.getPosition( ss_xPositon, ss_yPosition );
			Key_WidePaper.getSize( ss_width, ss_height );
			// 72版以降の画面仕様書マージ【A020114130】.
			if( TYPE_FeedTray1 == PnlComGetJobCopy()->getJobSpec()->pc_Print->c_PaperTray.getFeedTray() ) {
				PNLC_PopupListControl::newInstance()->addPopup( &gp_PaperSizeWideTray1_copy, this, NULL, ss_xPositon, ss_yPosition, ss_width, ss_height );
			} else {
				PNLC_PopupListControl::newInstance()->addPopup( &gp_PaperSizeWideInch_copy, this, NULL, ss_xPositon, ss_yPosition, ss_width, ss_height );
			}
			break;
		case BTNID_KEY_12_1_4x18:					// 12_1_4x18キー.
			c_PaperSize.c_SizeMM.x = PCGD_1225_18PaperSizeX;
			c_PaperSize.c_SizeMM.y = PCGD_1225_18PaperSizeY;
			c_PaperSize.c_SizeMM = c_PaperSize.c_SizeMM.xyReverse();
			// 用紙サイズをジョブに設定.
			pc_Print->c_PaperSize = c_PaperSize;
			// 自動検出をオフ.
			pc_FeedTray->setPaperSizeAutoDetectOn( e_FeedTray, False );
			// トレイへの設定.
			onlyPanel.setFeedSizeKind( e_FeedTray );
			// APS/AMS処理関数.
			selectAutoPaperMag();
			break;
// comment 2015/05/18 OPS1-MSE渡邊(哲) [MinervaSBK][KRQ-49] 不定形用紙サイズ自動検出機能の搭載.
		case BTNID_KEY_PAPERSIZEMEASUREMENT_COPY:			// サイズ測定キー.
			// サイズ測定中警告を設定.
			onlyPanel.setPanelWarning( PNL_OTHER_PaperSizeMeasurement );
			break;
// comment 2017/12/27 SCD2-4G Sky平田 [Eagle][RQ-720]紙種自動検知.
		case BTNID_KEY_PaperTypeAutoDetect:					// 自動検出キー.
			bl_UpdateMediaAutoDetect = True;
			if( True == pc_FeedTray->getMediaAutoDetect( e_FeedTray ) ) {
				bl_MediaAutoDetectStatus = False;
			} else {
				bl_MediaAutoDetectStatus = True;
			}
			break;
		default: LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); break;
	}

// comment 2017/12/27 SCD2-4G Sky平田 [Eagle][RQ-720]紙種自動検知.
	// メディア自動検知設定有効?.
	if( True == PnlComGet_isPaperAutoDetectAdmin( e_FeedTray ) ) {
		if( True == bl_UpdateMediaAutoDetect ) {
			Bool bl_NowStatus = pc_FeedTray->getMediaAutoDetect( e_FeedTray );
			if( bl_NowStatus != bl_MediaAutoDetectStatus ) {
				pc_FeedTray->setMediaAutoDetect( e_FeedTray, bl_MediaAutoDetectStatus );
			}
		}
	}
// Comment 2019/12/06 SCD2-Sky 岩渕 [EagleZ+_BK+][OP_BTS-13540] レターヘッド紙での自動検知は排他.
	TYPC_PaperType c_PaperType = convertPaperType( pc_FeedTray->getPaperType( e_FeedTray ), True );
	const TYPE_PaperType e_PaperType = c_PaperType.getPaperType();
	if ( False == PNLC_CopySettingLib::newInstance()->isAutoPaperTypeCheckObjDisp( e_PaperType ) ) {		// 紙種が自動検出表示対応.
		pc_FeedTray->setMediaAutoDetect( e_FeedTray,  False);		//自動検出機能OFF.
	}
	// 画面セットアップ.
	setupGamen();
}

//-----------------------------------------------------------------------------------------------
/**
 *	@brief	A020114100 選択トレイの設定変更 : Qtボタン押下処理.
 */
//-----------------------------------------------------------------------------------------------
void PCPC_PaperEnl_copy::procQtButtonOn( ushort us_option )
{
	uchar			uc_BtnID = 0;									// ボタンID.
	uchar			uc_Cnt = 0;										// ループカウンタ.
	TYPE_FeedTray	e_FeedTray = getSelectedFeedTray();				// トレイ種別.

	// 手差しトレイの場合.
	if( TYPE_FeedTrayManual == e_FeedTray ) {
		for( uc_Cnt = 0; uc_Cnt < uc_TrayManualPaperKindBtnFromPaperTypeNum; uc_Cnt++ )	{
			// 用紙種類の比較.
			if( tbl_TrayManualPaperKindBtnFromPaperType[uc_Cnt].e_PaperType == us_option ) {
				uc_BtnID = tbl_TrayManualPaperKindBtnFromPaperType[uc_Cnt].uc_BtnID;
				procButtonOn( uc_BtnID );
				break;
			}
		}
//Comment [IT6-Eagle] 機器構成変更No.2 HCLT-Deeparaj 2018/02/20 Adding envelope key for tray 1.
#ifndef DEF_A4_Cus
	} else if( TYPE_FeedTray1 == e_FeedTray ) {
		for( uc_Cnt = 0; uc_Cnt < uc_Tray1PaperKindBtnFromPaperTypeNum; uc_Cnt++ )	{
			// 用紙種類の比較.
			if( tbl_Tray1PaperKindBtnFromPaperType[uc_Cnt].e_PaperType == us_option ) {
				uc_BtnID = tbl_Tray1PaperKindBtnFromPaperType[uc_Cnt].uc_BtnID;
				procButtonOn( uc_BtnID );
				break;
			}
		}
#endif	//DEF_A4_Cus.
	} else {	// 手差しトレイ以外の場合.
// A3A4FUM 用紙種類設定 t-okada 2015/01/19 Start 選択トレイの設定変更.
#ifndef DEF_A4_Cus
// A3A4FUM 用紙種類設定 t-okada 2015/01/19 End.
		for( uc_Cnt = 0; uc_Cnt < uc_TrayPaperKindBtnFromPaperTypeNum; uc_Cnt++ ) {
			// 用紙種類の比較.
			if( tbl_TrayPaperKindBtnFromPaperType[uc_Cnt].e_PaperType == us_option ) {
				uc_BtnID = tbl_TrayPaperKindBtnFromPaperType[uc_Cnt].uc_BtnID;
				procButtonOn( uc_BtnID );
				break;
			}
		}
// A3A4FUM 用紙種類設定 t-okada 2015/01/19 Start 選択トレイの設定変更.
#else // DEF_A4_Cus.
// A3A4FUM 用紙種類設定 t-okada 2014/12/08 Start 選択トレイの設定変更.
		uchar uc_idx = getTrayTblIdxFromFeedTray(e_FeedTray);
		for( uc_Cnt = 0; uc_Cnt < getTrayPaperKindBtnFromPaperTypeTblSize(); uc_Cnt++ ) {
			// 用紙種類の比較.
			if( tbl_TrayPaperKindBtnFromPaperType[uc_idx][uc_Cnt].e_PaperType == us_option ) {
				uc_BtnID = tbl_TrayPaperKindBtnFromPaperType[uc_idx][uc_Cnt].uc_BtnID;
				procButtonOn(uc_BtnID);
				break;
			}
		}

#endif // DEF_A4_Cus.
// A3A4FUM 用紙種類設定 t-okada 2015/01/19 End.
	}
}

//-----------------------------------------------------------------------------------------------
/**
 *	@brief	A020114100 選択トレイの設定変更 : Jobから選択中の用紙種別を取得.
 */
//-----------------------------------------------------------------------------------------------
TYPE_PaperType PCPC_PaperEnl_copy::getJobPaperType()
{
	uchar uc_Cnt = 0;																			// ループカウンタ.
	TYPE_FeedTray  e_FeedTray  = getSelectedFeedTray();											// 選択トレイを取得.
	APIC_FeedTray* pc_FeedTray = PnlComGetFeedTray();											// 給紙トレイ情報を取得.
	TYPC_PaperType c_PaperType = convertPaperType( pc_FeedTray->getPaperType( e_FeedTray ), True );	// 用紙種別コードの切り替え.
	TYPE_PaperType e_PaperType = c_PaperType.getPaperType();									// 用紙種別を取得.
	TYPE_CustomPaperType e_CustomPaperType = c_PaperType.getCustomPaperType();					// カスタム用紙種別を取得.

	// カスタム用紙が選択中の場合.
	if( TYPE_CustomPaperType_UnSelect != e_CustomPaperType ) {
		// カスタム用紙種別から、用紙種別に変換.
		for( uc_Cnt = 0; uc_Cnt < uc_PaperTypeFromCustomPaperTypeNum; uc_Cnt++ ) {
			if( tbl_PaperTypeFromCustomPaperType[uc_Cnt].e_CustomPaperType == e_CustomPaperType ) {
				e_PaperType = tbl_PaperTypeFromCustomPaperType[uc_Cnt].e_PaperType;
				break;
			}
		}
	}
	return e_PaperType;
}

//-----------------------------------------------------------------------------------------------
/**
 *	@brief	A020114100 選択トレイの設定変更 : アクティブ状態の通知.
 */
//-----------------------------------------------------------------------------------------------
void PCPC_PaperEnl_copy::procActivate()
{
	TYPE_PaperType	e_PaperType = getJobPaperType();		// 用紙種類.
	// 用紙を選択状態にする.
	Cmp_PaperTypeSelectList.setSelectPaperType( e_PaperType, PNLE_PaperTypeSelect_MoveNoAnimation );
//	comment 2015/03/13 HCLT-Durga [HeliosL][CQ510997&CQ510999]  To set the Thick3 for A6card Type.
//// comment 2015/02/24 HCLT-Ezhil [HeliosL][CQ510368] Paper type selection display based on paper in manual tray.
//	m_bl_Thick3AutoSelectFlag = False;			// Flag updation whether to set paper type as Thick3.
	// 親クラスを呼び出し.
	PPopupDecisionButtonCom::procActivate();
}


//-----------------------------------------------------------------------------------------------
/**
 *	@brief	A020114100 選択トレイの設定変更 : 用紙サイズの自動検出設定.
 */
//-----------------------------------------------------------------------------------------------
void PCPC_PaperEnl_copy::setAutoDetectionPaperSize()
{
	// 選択されているトレイの取得.
	const TYPE_FeedTray e_FeedTray = getSelectedFeedTray();

	if( False == PnlComGetFeedTray()->getPaperSizeAutoDetectOn( e_FeedTray ) )
	{
		// 自動検出モードの設定値反映.
		PnlComGetFeedTray()->setPaperSizeAutoDetectOn( e_FeedTray, True );

		// APS/AMS処理関数.
		selectAutoPaperMag();
	} else if( ( TYPE_FeedTrayManual != e_FeedTray )
	           && ( True == PnlComGetFeedTray()->getPaperSize( e_FeedTray ).b_WideOn ) ) {
		APIC_Job* pc_Job = PnlComGetJobCopy();
		if( NULL == pc_Job ) {
			LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
			return;
		}
		TYPC_PaperSize c_PaperSize = PnlComGetFixedPaperSizeCodeToMM( TYPE_SizeCodeNonStandard );
		// 自動検出モードの設定値反映.
		PnlComGetFeedTray()->setPaperSizeAutoDetectOn( e_FeedTray, True );
		// 現在検出されている実サイズをコードに変換する.
		c_PaperSize.c_SizeMM = PnlComGetFeedTray()->getPaperSize( e_FeedTray ).c_SizeMM;
		c_PaperSize.MMtoSizeCode();
		// 用紙サイズをジョブに設定.
		pc_Job->getJobSpec()->pc_Print->c_PaperSize = c_PaperSize;
		// トレイへの設定.
		onlyPanel.setFeedSizeKind( e_FeedTray );
		// APS/AMS処理関数.
		selectAutoPaperMag();
	}
}

//-----------------------------------------------------------------------------------------------
/**
 *	@brief	A020114100 選択トレイの設定変更 : 用紙種類設定処理.
 */
//-----------------------------------------------------------------------------------------------
void PCPC_PaperEnl_copy::setPaperType( uchar option )
{

	// 用紙種類ボタンステータステーブルの取得.
	const uchar puc_AryPaperKindBtnStatus = getPaperKindBtnStatusTbl( ( uchar ) (option - 100) );
	short ss_Type;
	// ボタン押下用紙サイズ検索.
	// 用紙種類テーブルの取得.
	const TYPS_PaperKindInfo* pst_AryPaperKindInfo = getPaperKindInfo( ss_Type );
// comment 2015/10/23 OPS1-Sky_真田 [ZeusS_ZX0][機器No.62]光沢モード＋再生紙/薄紙禁則対応.
//	if( ( combinationCheck( pst_AryPaperKindInfo[puc_AryPaperKindBtnStatus].e_Combination ) )
//	    || ( True == PLBC_JudgeCenterOriAndThickPaperCombination( pst_AryPaperKindInfo[puc_AryPaperKindBtnStatus].e_Combination ) ) )
// Comment 2015/12/23 【HCLT-Harikrishnan】【CQ519679】【ID: 33165】NPD.FUNC.MUST.
	if( NULL == pst_AryPaperKindInfo ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return;
	}
//テーブルは変更しない
	EP_Combination e_Combination_temp = pst_AryPaperKindInfo[puc_AryPaperKindBtnStatus].e_Combination;
	TYPE_PaperType e_PaperType = pst_AryPaperKindInfo[puc_AryPaperKindBtnStatus].e_PaperType;
	if( TYPE_PaperRecycle1 == e_PaperType ) {
		e_Combination_temp = CNK_RecycledPaper;
	}
	
	if( ( combinationCheck( e_Combination_temp ) ) || ( True == PLBC_JudgeCenterOriAndThickPaperCombination( e_Combination_temp ) ) ) {
		uchar			uc_Cnt = 0;										// ループカウンタ.
		TYPE_FeedTray	e_FeedTray = getSelectedFeedTray();				// トレイ種別.

		// 手差しトレイの場合.
		if( TYPE_FeedTrayManual == e_FeedTray )	{
			for( uc_Cnt = 0; uc_Cnt < uc_TrayManualPaperKindBtnFromPaperTypeNum; uc_Cnt++ ) {
				// 用紙種類の比較.
				if( tbl_TrayManualPaperKindBtnFromPaperType[uc_Cnt].uc_BtnID == option ) {
					// 指定用紙を選択状態にする.
					Cmp_PaperTypeSelectList.setSelectPaperType( tbl_TrayManualPaperKindBtnFromPaperType[uc_Cnt].e_PaperType, PNLE_PaperTypeSelect_NoMove );
					break;
				}
			}
//Comment [IT6-Eagle] 機器構成変更No.2 HCLT-Deeparaj 2018/02/20 Adding envelope key for tray 1.
#ifndef DEF_A4_Cus
		} else if( TYPE_FeedTray1 == e_FeedTray || PNLE_PEPARKIND_TYPE_TRY1 == ss_Type )	{
			for( uc_Cnt = 0; uc_Cnt < uc_Tray1PaperKindBtnFromPaperTypeNum; uc_Cnt++ ) {
				// 用紙種類の比較.
				if( tbl_Tray1PaperKindBtnFromPaperType[uc_Cnt].uc_BtnID == option ) {
					// 指定用紙を選択状態にする.
					Cmp_PaperTypeSelectList.setSelectPaperType( tbl_Tray1PaperKindBtnFromPaperType[uc_Cnt].e_PaperType, PNLE_PaperTypeSelect_NoMove );
					break;
				}
			}
#endif	//DEF_A4_Cus.
		} else {	// 手差しトレイ以外の場合.
// A3A4FUM 用紙種類設定 t-okada 2015/01/19 Start 選択トレイの設定変更.
#ifndef DEF_A4_Cus
			for( uc_Cnt = 0; uc_Cnt < uc_TrayPaperKindBtnFromPaperTypeNum; uc_Cnt++ ) {
				// 用紙種類の比較.
				if( tbl_TrayPaperKindBtnFromPaperType[uc_Cnt].uc_BtnID == option ) {
					// 指定用紙を選択状態にする.
					Cmp_PaperTypeSelectList.setSelectPaperType( tbl_TrayPaperKindBtnFromPaperType[uc_Cnt].e_PaperType, PNLE_PaperTypeSelect_NoMove );
					break;
				}
			}
// A3A4FUM 用紙種類設定 t-okada 2015/01/19 Start 選択トレイの設定変更.
#else // DEF_A4_Cus.
			uchar uc_idx = getTrayTblIdxFromFeedTray(e_FeedTray);
			for( uc_Cnt = 0; uc_Cnt < getTrayPaperKindBtnFromPaperTypeTblSize(); uc_Cnt++ ) {
				// 用紙種類の比較.
				if( tbl_TrayPaperKindBtnFromPaperType[uc_idx][uc_Cnt].uc_BtnID == option ) {
					// 指定用紙を選択状態にする.
					Cmp_PaperTypeSelectList.setSelectPaperType( tbl_TrayPaperKindBtnFromPaperType[uc_idx][uc_Cnt].e_PaperType, PNLE_PaperTypeSelect_NoMove );
					break;
				}
			}
#endif // DEF_A4_Cus.
// A3A4FUM 用紙種類設定 t-okada 2015/01/19 End.
		}

// comment 2014/07/25 OPS1-Sky稲井 [Zeus5BK][機器No70] トレイのFD検出廃止.
		//用紙種類は設定するので、フラグは落とす.
// comment 2014/08/25 OPS1-Sky_nozaki [Poseidon][CQ502669] 手差しトレイに用紙を置いても用紙サイズを変更しない.
//		Bool bl_isPaperSizeUnSelect_Bkup = onlyPanel.bl_isPaperSizeUnSelect;
//		onlyPanel.bl_isPaperSizeUnSelect = False;
// comment 2014/12/12 OPS1-Sky東島 [ZeusS][CQ507738].
//		setPaperTypeCom(pst_AryPaperKindInfo[puc_AryPaperKindBtnStatus].e_PaperType,
//		pst_AryPaperKindInfo[puc_AryPaperKindBtnStatus].b_2ndSidedPaper,
//		TYPE_CustomPaperType_UnSelect);													// comment MinervaZeus ADD Sky上西 2011/05/05 [UV-01038][機器1001].
		PNLE_PaperEx e_ExPaper = PNLE_PaperEx_NotSet;
		if( True == PnlComGetPrintJobSetting()->getPaperTypeNormalEx()	// 普通紙Ex使用可能.
		 && ( TYPE_PaperTypeNormal == pst_AryPaperKindInfo[puc_AryPaperKindBtnStatus].e_PaperType  	// 普通紙選択.
		   && PNLE_PaperTypeCategory_Normal == m_e_PaperTypeCategory )
		 && TYPE_SystemType_JP == PnlComGetDestination() ){				// 日本仕向け.
			e_ExPaper = PNLE_PaperEx_On;
		} else {
			e_ExPaper = PNLE_PaperEx_Off;
		}
		setPaperTypeCom( pst_AryPaperKindInfo[puc_AryPaperKindBtnStatus].e_PaperType,
		                 pst_AryPaperKindInfo[puc_AryPaperKindBtnStatus].b_2ndSidedPaper,
						 TYPE_CustomPaperType_UnSelect ,
						 e_ExPaper );
// comment 2014/07/25 OPS1-Sky稲井 [Zeus5BK][機器No70] トレイのFD検出廃止.
		//フラグの復帰.
//		onlyPanel.bl_isPaperSizeUnSelect = bl_isPaperSizeUnSelect_Bkup;
		// 封筒を選択した場合、用紙定形サイズ画面(封筒サイズ)を表示する.
		if( TYPE_PaperTypeEnvelope == pst_AryPaperKindInfo[puc_AryPaperKindBtnStatus].e_PaperType ) {
			PNLC_CopyDataLib::newInstance()->setCopyPaperTypeButton( PNLD_ButtonPaperKindEnvelope );
			PNLC_PopupListControl::newInstance()->addPopup( &gp_PaperSizeMetric_copy, this, NULL );
		}
	}
}

//-----------------------------------------------------------------------------------------------
/**
 *	@brief	A020114100 選択トレイの設定変更 : カスタム用紙種類設定処理.
 */
//-----------------------------------------------------------------------------------------------
void PCPC_PaperEnl_copy::setCustomPaperType( uchar option )
{
	// カスタマイズ用紙インデックス.
	TYPE_CustomPaperType e_CustomPaperType = TYPE_CustomPaperType_UnSelect;		// カスタム用紙属性.

	for( uchar i=0; i< uc_CustomPaperTypeInfoNum; i++ ) {
		if( st_CustomPaperTypeInfo[i].uc_BtnID == option ) {				// カスタム用紙種類情報テーブルとボタンIDが一致.
			e_CustomPaperType = st_CustomPaperTypeInfo[i].e_CustomPaperType;
			break;
		}
	}

	// カスタム用紙属性から用紙種類取得.
	PNLC_SelectPaperTrayCom_DataLib* pDataLib = PNLC_SelectPaperTrayCom_DataLib::newInstance();
	TYPE_PaperType e_PaperType = pDataLib->getPaperTypeFromCustomPaperType( e_CustomPaperType );
	PNLE_PaperEx e_ExPaper = PNLE_PaperEx_NotSet;

	// カスタム用紙(普通紙(Ex))の場合、普通紙を設定し、Exフラグを有効にする.
	if( TYPE_PaperTypeNormalEx == e_PaperType ) {				// 普通紙(Ex).
		e_PaperType = TYPE_PaperTypeNormal;
		e_ExPaper = PNLE_PaperEx_On;
	} else if( TYPE_PaperTypeNormalEx_2nd == e_PaperType ) {	// 普通紙(Ex)両面2面目.
		e_PaperType = TYPE_PaperTypeNormal_2nd;
		e_ExPaper = PNLE_PaperEx_On;
// comment 2014/12/12 OPS1-Sky東島 [ZeusS][CQ507738].
	} else if( TYPE_PaperTypeNormal == e_PaperType ){		// 普通紙.
		e_PaperType = TYPE_PaperTypeNormal;
		e_ExPaper = PNLE_PaperEx_Off;
	} else {
		// 処理なし.
	}

	EP_Combination e_Combination = CNK_Size;		// 機能種類.

	// ボタン押下用紙サイズ検索.
	// 用紙種類テーブルの取得.
	short ss_Type;
	const TYPS_PaperKindInfo* pst_AryPaperKindInfo = getPaperKindInfo( ss_Type );
	const uchar uc_PaperKindInfoSize = getPaperKindInfoSize();
	for( uchar i=0; i<uc_PaperKindInfoSize; i++ ) {
		if( pst_AryPaperKindInfo[i].e_PaperType == e_PaperType ) {		// 用紙種類テーブルと用紙種類が一致.
			e_Combination = pst_AryPaperKindInfo[i].e_Combination;
			break;
		}
	}

	if( combinationCheck( e_Combination ) ) {
		uchar			uc_Cnt = 0;										// ループカウンタ.
		TYPE_FeedTray	e_FeedTray = getSelectedFeedTray();				// トレイ種別.

		// 手差しトレイの場合.
		if( TYPE_FeedTrayManual == e_FeedTray )	{
			for( uc_Cnt = 0; uc_Cnt < uc_TrayManualPaperKindBtnFromPaperTypeNum; uc_Cnt++ ) {
				// 用紙種類の比較.
				if( tbl_TrayManualPaperKindBtnFromPaperType[uc_Cnt].uc_BtnID == option ) {
					// 指定用紙を選択状態にする.
					Cmp_PaperTypeSelectList.setSelectPaperType( tbl_TrayManualPaperKindBtnFromPaperType[uc_Cnt].e_PaperType, PNLE_PaperTypeSelect_NoMove );
					break;
				}
			}
		} else {	// 手差しトレイ以外の場合.
// A3A4FUM 用紙種類設定 t-okada 2015/01/19 Start 選択トレイの設定変更.
#ifndef DEF_A4_Cus
			for( uc_Cnt = 0; uc_Cnt < uc_TrayPaperKindBtnFromPaperTypeNum; uc_Cnt++ ) {
				// 用紙種類の比較.
				if( tbl_TrayPaperKindBtnFromPaperType[uc_Cnt].uc_BtnID == option ) {
					// 指定用紙を選択状態にする.
					Cmp_PaperTypeSelectList.setSelectPaperType( tbl_TrayPaperKindBtnFromPaperType[uc_Cnt].e_PaperType, PNLE_PaperTypeSelect_NoMove );
					break;
				}
			}
// A3A4FUM 用紙種類設定 t-okada 2015/01/19 Start 選択トレイの設定変更.
#else // DEF_A4_Cus.
			uchar uc_idx = getTrayTblIdxFromFeedTray(e_FeedTray);
			for( uc_Cnt = 0; uc_Cnt < getTrayPaperKindBtnFromPaperTypeTblSize(); uc_Cnt++ ) {
				// 用紙種類の比較.
				if( tbl_TrayPaperKindBtnFromPaperType[uc_idx][uc_Cnt].uc_BtnID == option ) {
					// 指定用紙を選択状態にする.
					Cmp_PaperTypeSelectList.setSelectPaperType( tbl_TrayPaperKindBtnFromPaperType[uc_idx][uc_Cnt].e_PaperType, PNLE_PaperTypeSelect_NoMove );
					break;
				}
			}
#endif // DEF_A4_Cus.
// A3A4FUM 用紙種類設定 t-okada 2015/01/19 End.
		}

		//用紙種類は設定するので、フラグは落とす.
		setPaperTypeCom( e_PaperType,							// 用紙種類.
		                 False,									// 両面2画面設定.
		                 e_CustomPaperType,						// カスタム用紙属性.
		                 e_ExPaper );							// Ex設定（ON）.
		//フラグの復帰.
//		onlyPanel.bl_isPaperSizeUnSelect = bl_isPaperSizeUnSelect_Bkup;
	}
}

//-----------------------------------------------------------------------------------------------
/**
 *	@brief	A020114100 選択トレイの設定変更 : 用紙種類ボタンステータステーブルサイズの取得.
 */
//-----------------------------------------------------------------------------------------------
// A3A4FUM 用紙種類設定 t-okada 2015/01/19 Start 選択トレイの設定変更.
#ifndef DEF_A4_Cus
// A3A4FUM 用紙種類設定 t-okada 2015/01/19 End.
uchar PCPC_PaperEnl_copy::getPaperKindBtnStatusTblSize()
{
	uchar uc_PaperKindBtnStatusTblSize = 0;
	// 選択中のトレイが手差しの場合.
	if( TYPE_TrayTypeMultiMan == getSelectedTrayType() ||
	    TYPE_TrayTypeSingleMan == getSelectedTrayType() )
	{
		uc_PaperKindBtnStatusTblSize = uc_SizeNewPaperTypeBtnMultiMan;
	}
	else if( TYPE_TrayTypeNon !=  getSelectedTrayType() )
	{
		uc_PaperKindBtnStatusTblSize = uc_SizeNewPaperTypeBtnStatus;
	}
	else
	{
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
	}
	return uc_PaperKindBtnStatusTblSize;
}
// A3A4FUM 用紙種類設定 t-okada 2015/01/19 Start 選択トレイの設定変更.
#else // DEF_A4_Cus.
uchar PCPC_PaperEnl_copy::getPaperKindBtnStatusTblSize()
{
	uchar uc_PaperKindBtnStatusTblSize = 0;
	TYPE_FeedTray e_trayno = getSelectedFeedTray();

	// 選択中のトレイが手差しの場合.
	if ( TYPE_FeedTrayManual == e_trayno )
	{
		uc_PaperKindBtnStatusTblSize = uc_SizeNewPaperTypeBtnMultiMan;
	}		
	else if ( TYPE_FeedTray1 == e_trayno )
	{
		uc_PaperKindBtnStatusTblSize = PNLD_AryPaperTypeBtnStatusTbl_Tray1Num;
	} 
	else if ( TYPE_FeedTray2 == e_trayno ) 
	{
		uc_PaperKindBtnStatusTblSize = PNLD_AryPaperTypeBtnStatusTbl_Tray2Num;
	} 
	else if ( TYPE_FeedTray3 == e_trayno ) 
	{
		uc_PaperKindBtnStatusTblSize = PNLD_AryPaperTypeBtnStatusTbl_Tray3Num;
	} 
//comment 2023/07/25 FPT.LONGTH32 [IT6_2.3] 機器構成変更_No.75 Add tray 4 for Sparrow.
	else if ( TYPE_FeedTray4 == e_trayno ) 
	{
		uc_PaperKindBtnStatusTblSize = PNLD_AryPaperTypeBtnStatusTbl_Tray4Num;
	} 
	else
	{
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
	}
	return uc_PaperKindBtnStatusTblSize;
}
#endif // DEF_A4_Cus.
// A3A4FUM 用紙種類設定 t-okada 2015/01/19 End.


// A3A4FUM 用紙種類設定 t-okada 2015/01/19 Start 選択トレイの設定変更.
#ifdef DEF_A4_Cus
//-----------------------------------------------------------------------------------------------
/**
 *	@brief	A020114100 選択トレイの設定変更 : 用紙種類ボタンID紐付けテーブルサイズの取得.
 */
//-----------------------------------------------------------------------------------------------
uchar PCPC_PaperEnl_copy::getTrayPaperKindBtnFromPaperTypeTblSize()
{
	uchar uc_TblSize = 0;
	TYPE_FeedTray e_trayno = getSelectedFeedTray();

	// 選択中のトレイが手差しの場合.
	if ( TYPE_FeedTrayManual == e_trayno )
	{
		uc_TblSize = uc_TrayManualPaperKindBtnFromPaperTypeNum;
	}		
	else if ( TYPE_FeedTray1 == e_trayno )
	{
		uc_TblSize = PNLD_Tbl_TrayPaperKindBtnFromPaperType_Tray1Num;
	} 
	else if ( TYPE_FeedTray2 == e_trayno ) 
	{
		uc_TblSize = PNLD_Tbl_TrayPaperKindBtnFromPaperType_Tray2Num;
	} 
	else if ( TYPE_FeedTray3 == e_trayno ) 
	{
		uc_TblSize = PNLD_Tbl_TrayPaperKindBtnFromPaperType_Tray3Num;
	} 
//comment 2023/07/25 FPT.LONGTH32 [IT6_2.3] 機器構成変更_No.75 Add tray 4 for Sparrow.
	else if ( TYPE_FeedTray4 == e_trayno ) 
	{
		uc_TblSize = PNLD_Tbl_TrayPaperKindBtnFromPaperType_Tray4Num;
	}
	else
	{
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
	}
	return uc_TblSize;
}

//-----------------------------------------------------------------------------------------------
/**
 *	@brief	A020114100 選択トレイの設定変更 : トレイ定義からトレイごとのテーブルインデックス取得.
 */
//-----------------------------------------------------------------------------------------------
uchar PCPC_PaperEnl_copy::getTrayTblIdxFromFeedTray( TYPE_FeedTray e_FeedTray )
{
	uchar uc_tblIdx=0;

	if ( TYPE_FeedTray1 == e_FeedTray ) {
		uc_tblIdx = 0;
	} else if ( TYPE_FeedTray2 == e_FeedTray ) {
		uc_tblIdx = 1;
	} else if ( TYPE_FeedTray3 == e_FeedTray ) {
		uc_tblIdx = 2;
//comment 2023/07/25 FPT.LONGTH32 [IT6_2.3] 機器構成変更_No.75 Add tray 4 for Sparrow.
	} else if ( TYPE_FeedTray4 == e_FeedTray ) {
		uc_tblIdx = 3;
	} else {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
	}

	return uc_tblIdx;
}
#endif // DEF_A4_Cus.
// A3A4FUM 用紙種類設定 t-okada 2015/01/19 End.


// A3A4FUM 用紙種類設定 t-okada 2015/01/19 Start 選択トレイの設定変更.
#ifndef DEF_A4_Cus
// A3A4FUM 用紙種類設定 t-okada 2015/01/19 End.
//-----------------------------------------------------------------------------------------------
/**
 *	@brief	A020114100 選択トレイの設定変更 : 用紙種類ボタンステータステーブルの取得.
 */
//-----------------------------------------------------------------------------------------------
const uchar PCPC_PaperEnl_copy::getPaperKindBtnStatusTbl( const uchar uc_Index )
{

	// 選択中のトレイが手差しの場合.
	if( TYPE_TrayTypeMultiMan == getSelectedTrayType() ||
	    TYPE_TrayTypeSingleMan == getSelectedTrayType() )
	{
		if( uc_SizeNewPaperTypeBtnMultiMan <= uc_Index ) {
			LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
			return -1;
		}
		return uc_AryPaperTypeBtnMultiManTbl[uc_Index][1];
//Comment [IT6-Eagle] 機器構成変更No.2 HCLT-Deeparaj 2018/02/20 When Tray 1 selected.
#ifndef DEF_A4_Cus
//Comment [IT6-Eagle] [1044008] HCLT-Deeparaj 2018/02/20 Changing the button index for Colored Paper.
//	} else if( TYPE_FeedTray1 ==  getSelectedTrayType() ) {
	} else if( TYPE_FeedTray1 ==  getSelectedFeedTray() ) {
		if( uc_SizeNewPaperTypeBtnStatusTry1 <= uc_Index ) {
			LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
			return -1;
		}
		return uc_AryPaperTypeBtnStatusTblTry1[uc_Index][1];
#endif	//DEF_A4_Cus.
	}
	else if( TYPE_TrayTypeNon !=  getSelectedTrayType() )
	{
		if( uc_SizeNewPaperTypeBtnStatus <= uc_Index ) {
			LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
			return -1;
		}
		return uc_AryPaperTypeBtnStatusTbl[uc_Index][1];
	}
	else
	{
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return -1;
	}
}

//-----------------------------------------------------------------------------------------------
/**
 *	@brief	A020114100 選択トレイの設定変更 : 用紙種類情報のサイズの取得.
 */
//-----------------------------------------------------------------------------------------------
uchar PCPC_PaperEnl_copy::getPaperKindInfoSize()
{
	uchar uc_PaperKindInfoSize = 0;
	// 選択中のトレイが手差しの場合.
	if( TYPE_TrayTypeMultiMan == getSelectedTrayType() ||
	    TYPE_TrayTypeSingleMan == getSelectedTrayType() )
	{
		uc_PaperKindInfoSize = uc_SizePaperKindNewTblMultiMan;
	}
//Comment [IT6-Eagle] 機器構成変更No.2 HCLT-Deeparaj 2018/02/20 When Tray 1 selected.
#ifndef DEF_A4_Cus
//Comment [IT6-Eagle] [1044008] HCLT-Deeparaj 2018/02/20 Changing the Condition.
//	else if( TYPE_FeedTray1 != getSelectedTrayType() ) {
	else if( TYPE_FeedTray1 == getSelectedFeedTray() ) {
		uc_PaperKindInfoSize = uc_SizePaperKindNewTblTry1;
	}
#endif	//DEF_A4_Cus.
	else if( TYPE_TrayTypeNon != getSelectedTrayType() )
	{
		uc_PaperKindInfoSize = uc_SizePaperKindNewTbl;
	}
	else
	{
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
	}
	return uc_PaperKindInfoSize;
}
// A3A4FUM 用紙種類設定 t-okada 2015/01/19 Start 選択トレイの設定変更.
#else // DEF_A4_Cus.
//-----------------------------------------------------------------------------------------------
/**
 *	@brief	A020114100 選択トレイの設定変更 : 用紙種類ボタンステータステーブルの取得.
 */
//-----------------------------------------------------------------------------------------------
const uchar PCPC_PaperEnl_copy::getPaperKindBtnStatusTbl(const uchar uc_Index)
{
	// 設定変更の対象となるトレイタイプの取得.
	const TYPE_TrayType e_traytype = getSelectedTrayType();

	// 選択中のトレイが手差しの場合.
	if ( TYPE_TrayTypeMultiMan == e_traytype ||
	     TYPE_TrayTypeSingleMan == e_traytype )
	{
		if (uc_SizeNewPaperTypeBtnMultiMan <= uc_Index) {
			LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
			return 0;
		}
		return uc_AryPaperTypeBtnMultiManTbl[uc_Index][1];
	}

	else if ( TYPE_TrayTypeNon !=  e_traytype )
	{
		TYPE_FeedTray e_trayno = getSelectedFeedTray();
		uchar uc_idxFromTray = getTrayTblIdxFromFeedTray(e_trayno);
		return uc_AryPaperTypeBtnStatusTbl[uc_idxFromTray][uc_Index][1];
	}
	else
	{
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return 0;
	}
}

//-----------------------------------------------------------------------------------------------
/**
 *	@brief	A020114100 選択トレイの設定変更 : 用紙種類情報のサイズの取得.
 */
//-----------------------------------------------------------------------------------------------
uchar PCPC_PaperEnl_copy::getPaperKindInfoSize()
{
	uchar uc_PaperKindInfoSize = 0;
	TYPE_FeedTray e_trayno = getSelectedFeedTray();

	// 選択中のトレイが手差しの場合.
	if ( TYPE_FeedTrayManual == e_trayno )
	{
		uc_PaperKindInfoSize = uc_SizePaperKindNewTblMultiMan;
	}
	else if ( TYPE_FeedTray1 == e_trayno )
	{
		uc_PaperKindInfoSize = PNLD_PaperKindNewTbl_Tray1Num;
	} 
	else if ( TYPE_FeedTray2 == e_trayno ) 
	{
		uc_PaperKindInfoSize = PNLD_PaperKindNewTbl_Tray2Num;
	}
	else if ( TYPE_FeedTray3 == e_trayno ) 
	{
		uc_PaperKindInfoSize = PNLD_PaperKindNewTbl_Tray3Num;
	} 
//comment 2023/07/25 FPT.LONGTH32 [IT6_2.3] 機器構成変更_No.75 Add tray 4 for Sparrow.
	else if ( TYPE_FeedTray4 == e_trayno ) 
	{
		uc_PaperKindInfoSize = PNLD_PaperKindNewTbl_Tray4Num;
	}
	else
	{
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
	}
	return uc_PaperKindInfoSize;
}
#endif // DEF_A4_Cus.
// A3A4FUM 用紙種類設定 t-okada 2015/01/19 End.

//-----------------------------------------------------------------------------------------------
/**
 *	@brief	A020114100 選択トレイの設定変更 : 設定変更の対象となるトレイタイプの取得.
 */
//-----------------------------------------------------------------------------------------------
TYPE_FeedTray PCPC_PaperEnl_copy::getSelectedFeedTray()
{
// comment [2015/06/08] [HCLT-Rajamanickam][MinervaSBK][機器構成変更_No.23] Get selected tray for PI.
	PNLC_CopyDataLib* pc_CopyDataLib = PNLC_CopyDataLib::newInstance();
	if( NULL == pc_CopyDataLib ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "[PCPC_PaperEnl_copy::getSelectedFeedTray!" );
		return TYPE_CustomPaperType_UnSelect;
	}
	if( True == pc_CopyDataLib->getPIToggleStatus() ) {
		return pc_CopyDataLib->getSelectedFeedTray();
	} else {
		if( ( NULL == PnlComGetJobCopy() ) ||
			( NULL == PnlComGetJobCopy()->getJobSpec() ) ||
			( NULL == PnlComGetJobCopy()->getJobSpec()->pc_Print) ) {
			LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "[PCPC_PaperEnl_copy::getSelectedFeedTray]Assert Failed!" );
			return TYPE_CustomPaperType_UnSelect;											// カスタム用紙属性[未選択].
		}
	// 設定変更の対象となるトレイタイプの取得.
	return PnlComGetJobCopy()->getJobSpec()->pc_Print->c_PaperTray.getFeedTray();
// comment [2015/06/08] [HCLT-Rajamanickam][MinervaSBK][機器構成変更_No.23] Get selected tray for PI.
	}
}

//-----------------------------------------------------------------------------------------------
/**
 *	@brief	A020114100 選択トレイの設定変更 : 選択中のトレイの取得.
 */
//-----------------------------------------------------------------------------------------------
TYPE_TrayType PCPC_PaperEnl_copy::getSelectedTrayType()
{
	// 設定変更の対象となるトレイを取得.
	return PnlComGetFeedTray()->getTrayType( getSelectedFeedTray() );
}

//-----------------------------------------------------------------------------------------------
/**
 *	@brief	A020114100 選択トレイの設定変更 : 両面2画面設定.
 */
//-----------------------------------------------------------------------------------------------
void PCPC_PaperEnl_copy::setPaperback()
{

	// 設定変更の対象となるトレイの取得.
	const TYPE_FeedTray e_FeedTray = getSelectedFeedTray();
	// 用紙種類、両面2面目フラグの取得.
	TYPC_PaperType c_PaperType = convertPaperType( PnlComGetFeedTray()->getPaperType( e_FeedTray ), True );
// comment 2015/10/23 OPS1-Sky_真田 [ZeusS_ZX0][機器No.62]光沢モード＋再生紙/薄紙禁則対応.
	//	const TYPE_PaperType e_PaperType = c_PaperType.getPaperType();
	TYPE_PaperType e_PaperType = c_PaperType.getPaperType();
	const Bool b_2ndSidedPaper = c_PaperType.getFeed2ndSidedPaper();
	const TYPE_CustomPaperType e_CustomPaperType = c_PaperType.getCustomPaperType();

// comment 2015/10/23 OPS1-Sky_真田 [ZeusS_ZX0][機器No.62]光沢モード＋再生紙/薄紙禁則対応.
	//テーブルは変更しない
	if( ( TYPE_PaperTypeNormal == e_PaperType ) && ( TYPE_CustomPaperType20 == e_CustomPaperType ) ) {
		e_PaperType = TYPE_PaperRecycle1;
	}

	EP_Combination		e_Combination = CNK_Size;
	for( ushort i = 0; i < sizeof( manual2ndSidedPaperTypeSelectTable_CLCD ) / sizeof( Manual2ndSidedPaperTypeSelectTable_CLCD ); i++ )
	{
		if( e_PaperType == manual2ndSidedPaperTypeSelectTable_CLCD[i].e_PaperType )
		{
			// 機能種類設定.
// comment 2015/10/23 OPS1-Sky_真田 [ZeusS_ZX0][機器No.62]光沢モード＋再生紙/薄紙禁則対応.
			//テーブルは変更しない
			EP_Combination e_Combination_temp 	= manual2ndSidedPaperTypeSelectTable_CLCD[i].e_Combination;
			EP_Combination e_Combination_temp2 	= manual2ndSidedPaperTypeSelectTable_CLCD[i].e_Combination2;
			if( TYPE_PaperRecycle1 == e_PaperType ) {
				e_Combination_temp	= CNK_RecycledPaper;
				e_Combination_temp2	= CNK_RecycledPaperRemove;
			}

			e_Combination = ( True == b_2ndSidedPaper ? e_Combination_temp : e_Combination_temp2 );
//			e_Combination = ( True == b_2ndSidedPaper ? manual2ndSidedPaperTypeSelectTable_CLCD[i].e_Combination : manual2ndSidedPaperTypeSelectTable_CLCD[i].e_Combination2 );
			// 組み合わせチェックを行う.
			if( True == combinationCheck( e_Combination ) )
			{
				// 用紙種類の設定.
				setPaperTypeCom( e_PaperType,( True == b_2ndSidedPaper ) ? False : True, e_CustomPaperType );
			}
		}
	}
}

//-----------------------------------------------------------------------------------------------
/**
 *	@brief	A020114100 選択トレイの設定変更 : Ex設定.
 */
//-----------------------------------------------------------------------------------------------
void PCPC_PaperEnl_copy::setPaperEx()
{
	// 設定変更の対象となるトレイの取得.
	const TYPE_FeedTray	e_FeedTray	= getSelectedFeedTray();

	// 用紙種類、両面2面目フラグの取得.
	TYPC_PaperType	c_PaperType	= convertPaperType( PnlComGetFeedTray()->getPaperType( e_FeedTray ), True );
	const TYPE_PaperType	e_PaperType	= c_PaperType.getPaperType();
	const Bool	b_2ndSidedPaper	= c_PaperType.getFeed2ndSidedPaper();
	const TYPE_CustomPaperType	e_CustomPaperType	= c_PaperType.getCustomPaperType();

	if( TYPE_PaperTypeNormal == e_PaperType ) {
		setPaperTypeCom( e_PaperType, b_2ndSidedPaper, e_CustomPaperType, ( ( True == c_PaperType.getPaperTypeNormalEx() ) ? PNLE_PaperEx_Off : PNLE_PaperEx_On ) );
	} else {
		// ブザー制御（NG音）.
		setBuzzerType( PCGE_BuzzerType_InputNg );
	}
}

//-----------------------------------------------------------------------------------------------
/**
 *	@brief	A020114100 選択トレイの設定変更 : ボタン更新処理.
 */
//-----------------------------------------------------------------------------------------------
void PCPC_PaperEnl_copy::setPaperKindBtn()
{
	short ss_Type;
	getPaperKindInfo( ss_Type );

// 用紙種類キーの表示・非表示.
	Bool bl_DipSW49_6 = APIC_SoftDipSw::newInstance()->getSoftDipSwBitForPanelSpecNo( 49, 6 );
//Comment 2018/11/29 HCLT_Sarada [IT6_DenebMLK][制御構成変更_No.45] Declaring system types.
#ifdef DEF_DenebMLKCus
	TYPE_SubSystemType	e_SubSystemType = pc_MachineSetting->getSubSystemType();
	TYPE_SystemType e_SystemType = PnlComGetMachineSetting()->getSystemType();
#endif //DEF_DenebMLKCus.
// comment 2018/11/05 【IT6_Sparrow】【HCLT_Taqiuddin】【AR1029580】 Warning Removal Fix.								
#ifndef DEF_A4_Cus	
	APIC_FeedTray* pc_FeedTray = PnlComGetFeedTray();		// comment CQ348020 2011/06/21 仁枝.
	APIC_SoftDipSw* pc_SoftDipSw = APIC_SoftDipSw::newInstance();	// DipSw（API）・インスタンス取得.		// comment CQ348020 2011/06/21 仁枝.
#endif	
	// 用紙サイズキーの表示・非表示.
	// 手差し.
	if( PNLE_PEPARKIND_TYPE_MUL == ss_Type ) {
		Bool b_activate = True;
#ifndef DEF_A4_Cus		// Comment VenusMLK -> 2016/04/04 OPS2 Sky高田(庸) [CQ525601].
		if( False == pc_SoftDipSw->getSoftDipSwBitForPanelSpecNo( 142, 0 )
		    && TYPE_PaperStatusEmpty == pc_FeedTray->getPaperStatus( TYPE_FeedTrayManual )		// 紙を置いていない.
		  ) {
			b_activate = False;
		}
#endif // DEF_A4_Cus	// Comment VenusMLK <- 2016/04/04 OPS2 Sky高田(庸) [CQ525601].
		Key_CustomSize.activate( b_activate );
		Key_WidePaper.activate( b_activate );
//comment 2014/12/17 HCLT-Vivek [ZeusS] [CQ 507907] Envelope / Postcard, Other, Metric Sizes and Inches keys are greyed out.
		Key_EnvelopeSize.activate( b_activate ); // Envelope / Postcard Key.
		Kex_MetricSize.activate( b_activate ); // Metric Sizes Key.
		Kex_InchSize.activate( b_activate ); // Inches Key.
		Kex_8k16kSize.activate( b_activate ); // Other Key.
// comment CQ348020 2011/06/21 仁枝 end.

		Key_12x18.showEnable( False );
// comment 2016/10/17 SCD2-2G南波 [IT5_4.0][RQ-226] 不定型紙の標準カセット対応.
		Key_12x18_Indeterminate.showEnable( False );
		Key_EnvelopeSize_SP.showEnable( False );
// AS-PVD_14_15 用紙サイズ設定 kuroda Start.
// Comment  2019/01/10 【HCLT-Rajkumar】 【IT6_DenebMLK】 【AR1070372_Reopen】 DenebMLK Doesn't detect FD size and CD width so operation behaviour same as A4.
//#ifdef DEF_A4_Cus
#if defined (DEF_A4_Cus) || defined (DEF_DenebMLKCus)
		// 不定形、封筒/はがき、その他、A系・B系、インチ系.
		Grp_OtherPaperSize.showEnableAll( True );
		// 仕向によって表示されるシンボルを切り替える.
		ChangeKeyTextForMarketArea();
		
		Rbt_SelectPaperSize.showEnableAll( False );					// 検知サイズ候補で使用されるグループ.
		Key_AutoSearch.showEnable( False );							// 自動検出.
#else
		if( True == PnlComGetPrinterSystem()->isManualTrayFDSizeDetect() ) {	// FDセンサーあり.
			if( TYPE_FeedTrayManual == getSelectedFeedTray() ) {	// 手差しトレイ選択時.
				Grp_OtherPaperSize.showEnableAll( True );					// その他サイズで使用されるグループ.
// comment 2016/10/17 SCD2-2G南波 [IT5_4.0][RQ-226] 不定型紙の標準カセット対応.
				Kex_8k16kSize.changePosition( 602, 276 );			//その他キー位置移動.
				Kex_MetricSize.changePosition( 434, 316 );			//A系B系キー位置移動.
				Kex_InchSize.changePosition( 602, 316 );			//インチ系キー位置移動.
				// 仕向によって表示されるシンボルを切り替える.
				ChangeKeyTextForMarketArea();
// comment 2015/05/18 OPS1-MSE渡邊(哲) [MinervaSBK][KRQ-49] 不定形用紙サイズ自動検出機能の搭載.
// Comment 2017/11/29 【HCLT-Tarun】【Eagle】【制御構成変更_No.4】 Removing old function version check and else portion.
				// FDセンサありサイズ測定キーを表示、FDセンサなしサイズ測定キーを非表示.
				Key_PaperSizeMeasurement_copyFD.showEnable( True );
				Key_PaperSizeMeasurement_copyNoneFD.showEnable( False );
			} else {
				Grp_OtherPaperSize.showEnableAll( False );					// その他サイズで使用されるグループ.
			}
			Rbt_SelectPaperSize.showEnableAll( False );					// 検知サイズ候補で使用されるグループ.
			Key_WidePaper.showEnable( True );
			Key_AutoSearch.showEnable( True );
		} else {		// FDセンサーなし.
			if( PNLD_PaperSizeTabDetection == m_sc_PaperSizeTab ) {	// 検知サイズ候補タブ選択中の場合.
				// 候補キーの表示/非表示と選択処理.
				if( TYPE_PaperStatusEmpty == pc_FeedTray->getPaperStatus( TYPE_FeedTrayManual ) ) {	// 用紙が置かれていない場合.
					Rbt_SelectPaperSize.showEnableAll( False );					// 検知サイズ候補で使用されるグループ.
				} else {
					PNLC_DetectPaperSizeLib* cp_DetectPaperSizeLib = PNLC_DetectPaperSizeLib::newInstance();
					if( NULL == cp_DetectPaperSizeLib ) {
						LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
						return;
					}
					PNLS_PaperSizeDisp st_DetectPaperSize;
					cp_DetectPaperSizeLib->getDispPaperSizeSetup( &st_DetectPaperSize );
					for( ushort i = 0; i < PNLD_PaperSizeDispMax; i++ ) {
						if( st_DetectPaperSize.m_st_paperSizeDispParts[i].b_setKeyShow ) {
							Key_SelectPaperSize_ProgramButton[i].changeText( &Ktx_SelectPaperSize[i], st_DetectPaperSize.m_st_paperSizeDispParts[i].us_text_size );
							Key_SelectPaperSize_ProgramButton[i].changeText( &Ktx_SelectPaperSizeOrientation[i], st_DetectPaperSize.m_st_paperSizeDispParts[i].us_text_direction );
							Key_SelectPaperSize_ProgramButton[i].showEnable( True );
// comment 2014/11/17 OPS1-Sky西村 [Poseidon][CQ506137][デモ指摘No.17]サイズ表示の字が小さい .
							//A系B系とインチ系とで文字列の長さが異なる為、SEF/LEFアイコンの座標を動的に変更.
							short s_posX = 0, s_posY = 0, s_sizeX = 56;
							Ktx_SelectPaperSize[i].getPosition(s_posX, s_posY);
							TYPE_SizeCode size = st_DetectPaperSize.m_st_paperSizeDispParts[i].e_PaperSize;
//	comment 2015/03/13 HCLT-Durga [HeliosL][CQ510997&CQ510999]  To set the Thick3 for A6card Type.
//// comment 2015/02/24 HCLT-Ezhil [HeliosL][CQ510368] Paper type selection display based on paper in manual tray.
//							if ( (TYPE_SizeCodePostcard_JpSef == size) || (TYPE_SizeCodeIN4x6Sef == size) || (TYPE_SizeCodePostcard_EuSef == size) ) {
//								
//								if( False == m_bl_Thick3AutoSelectFlag ) {
//									// 設定変更の対象となるトレイの取得.
//									const TYPE_FeedTray	e_FeedTray	= getSelectedFeedTray();
//									// 用紙種類、両面2面目フラグの取得.
//									TYPC_PaperType c_PaperType = PnlComGetFeedTray()->getPaperType(e_FeedTray);	
//									const Bool	b_2ndSidedPaper	= c_PaperType.getFeed2ndSidedPaper();
//									// カスタム用紙種別を取得.
//									const TYPE_CustomPaperType	e_CustomPaperType	= c_PaperType.getCustomPaperType();
//									PNLE_PaperEx e_ExPaper = PNLE_PaperEx_Off;	//// Ex設定（OFF）.
//									setPaperTypeCom( TYPE_PaperTypeThick3, b_2ndSidedPaper, e_CustomPaperType, e_ExPaper );			// Paper type set to Thick3 incase of A6card/4x6/postcard paper set in manual tray.
//									m_bl_Thick3AutoSelectFlag = True;		// Flag updation whether to set paper type as Thick3.
//								}
//							}
							if(TYPE_SizeCode12x18Sef == size || TYPE_SizeCodeLedgerSef == size || TYPE_SizeCodeLegalSef == size ||
								TYPE_SizeCodeLetterSef == size || TYPE_SizeCodeLetterLef == size || TYPE_SizeCodeExecutiveSef == size ||
								TYPE_SizeCodeExecutiveLef == size || TYPE_SizeCodeInvoiceSef == size || TYPE_SizeCodeInvoiceLef == size ) {
#ifdef DEF_LCD_7INCH
								s_sizeX = 72;
#else		// #ifdef DEF_LCD_7INCH.
								s_sizeX = 64;
#endif		// #ifdef DEF_LCD_7INCH.

							}
							Ktx_SelectPaperSizeOrientation[i].changePosition((short)(s_posX + s_sizeX), s_posY);
						} else {
							Key_SelectPaperSize_ProgramButton[i].showEnable( False );
						}
					}
					{	// 用紙の設定状態を用紙候補キーの選択状態に反映.
						m_sc_PaperSizeSelect = PNLD_PaperSizeUnSelect;
						TYPE_FeedTray e_FeedTray = getSelectedFeedTray();
						if( TYPE_FeedTrayManual == e_FeedTray ) {	// 手差しトレイ選択時.
							APIC_FeedTray* pc_FeedTray = PnlComGetFeedTray();
							TYPC_PaperSize c_PaperSize = pc_FeedTray->getPaperSize( e_FeedTray );
							for( uchar i = 0; i < PNLD_PaperSizeDispMax; i++ ) { // 用紙サイズが候補キーと同じ時.
								if( c_PaperSize.e_SizeCode == st_DetectPaperSize.m_st_paperSizeDispParts[i].e_PaperSize ) {
									m_sc_PaperSizeSelect = i;
									break;
								}
							}
						}
					}
					Rbt_SelectPaperSize.select( m_sc_PaperSizeSelect );
				}
				Grp_OtherPaperSize.showEnableAll( False );				// その他サイズで使用されるグループ.
				Key_WidePaper.showEnable( True );						// ワイド紙非表示.
				Key_AutoSearch.showEnable( False );						// 自動検知非表示.
				Key_ChoiseSize.changeSize( 160, 36 );					// 検知サイズ候補拡大.
				Key_OtherSize.changeSize( 160, 32 );					// その他サイズ縮小.
				Rbt_PaperSizeTab.select( PNLD_PaperSizeTabDetection );
// comment 2015/05/18 OPS1-MSE渡邊(哲) [MinervaSBK][KRQ-49] 不定形用紙サイズ自動検出機能の搭載.
// Comment 2017/11/29 【HCLT-Tarun】【Eagle】【制御構成変更_No.4】 Removing old function version check and else portion.
				// FDセンサありサイズ測定キーを非表示、FDセンサなしサイズ測定キーを表示.
				Key_PaperSizeMeasurement_copyFD.showEnable( False );
				Key_PaperSizeMeasurement_copyNoneFD.showEnable( True );
			} else {												// その他サイズタブキー選択の場合.
// comment 2015/09/09 OPS1-MSE服部 [MinervaSBK][CQ517162] FDセンサーなしの機種、かつ、「その他サイズ」タブ押下時、「サイズ測定」キーの非表示処理を追加.
				Key_PaperSizeMeasurement_copyFD.showEnable( False );		// FDセンサありサイズ測定キーを非表示.
				Key_PaperSizeMeasurement_copyNoneFD.showEnable( False );	// FDセンサなしサイズ測定キーを非表示.

				Grp_OtherPaperSize.showEnableAll( True );				// その他サイズで使用されるグループ.
// comment 2016/10/17 SCD2-2G南波 [IT5_4.0][RQ-226] 不定型紙の標準カセット対応.
				Kex_8k16kSize.changePosition( 602, 276 );			//その他キー位置移動.
				Kex_MetricSize.changePosition( 434, 316 );			//A系B系キー位置移動.
				Kex_InchSize.changePosition( 602, 316 );			//インチ系キー位置移動.
				Rbt_SelectPaperSize.showEnableAll( False );				// 検知サイズ候補で使用されるグループ.
				Key_WidePaper.showEnable( True );						// ワイド紙非表示.
				Key_AutoSearch.showEnable( False );						// 自動検知非表示.
				Key_ChoiseSize.changeSize( 160, 32 );					// 検知サイズ候補縮小.
				Key_OtherSize.changeSize( 160, 36 );					// その他サイズ縮小.
				Rbt_PaperSizeTab.select( PNLD_PaperSizeTabOther );
				// 仕向によって表示されるシンボルを切り替える.
				ChangeKeyTextForMarketArea();
			}
		}
#endif
// AS-PVD_14_15 用紙サイズ設定 kuroda End.
		// 両面2面目キー.
		if( True == bl_DipSW49_6 ) {
			Key_DoubleSide.showEnable( False );
		} else {
			Key_DoubleSide.showEnable( True );
		}
		// 手差し以外.
//Comment [IT6-Eagle] 機器構成変更No.2 HCLT-Deeparaj 2018/02/20 When Tray 1 selected.
#ifndef DEF_A4_Cus
	} else if( PNLE_PEPARKIND_TYPE_STD == ss_Type || PNLE_PEPARKIND_TYPE_TRY1 == ss_Type  ) {
#else	//DEF_A4_Cus.
	} else if( PNLE_PEPARKIND_TYPE_STD == ss_Type ) {
#endif	//DEF_A4_Cus.
		Key_EnvelopeSize_SP.showEnable( False );
		if( ( True == APIC_SoftDipSw::newInstance()->getSoftDipSwBitForPanelSpecNo( DEFSOFTWARESW205, 5 ) )		// DipSWNo.205 bit5 ON.
		    && ( TYPE_FeedTray1 == getSelectedFeedTray() || TYPE_FeedTray2 == getSelectedFeedTray() ) ) {
			Grp_OtherPaperSize.showEnableAll( False );		// その他サイズで使用されるグループ.
			Rbt_SelectPaperSize.showEnableAll( False );		// 検知サイズ候補で使用されるグループ.
			Key_DoubleSide.showEnable( False );
			Key_AutoSearch.showEnable( False );
			Key_12x18.showEnable( False );
// comment 2016/10/17 SCD2-2G南波 [IT5_4.0][RQ-226] 不定型紙の標準カセット対応.
			Key_12x18_Indeterminate.showEnable( False );
			Key_WidePaper.showEnable( False );
			Key_EnvelopeSize_SP.showEnable( True );
//Comment [IT6-Eagle] 機器構成変更No.2 HCLT-Deeparaj 2018/02/20 When Tray 1 selected.
#ifndef DEF_A4_Cus
		} else if( TYPE_FeedTray1 == getSelectedFeedTray() || PNLE_PEPARKIND_TYPE_TRY1 == ss_Type ) {
#else	//DEF_A4_Cus.
		} else if( TYPE_FeedTray1 == getSelectedFeedTray() ) {
#endif	//DEF_A4_Cus.
			// [KronosS] Sky川名 2016/10/20 管理No21_44 各トレイ毎の表示用紙サイズをKronosS仕様にあわせる.
#ifdef DEF_KRONOSS_Cus
			Key_EnvelopeSize.showEnable( False ); // 封筒/はがきキー.
#endif // DEF_KRONOSS_Cus.
//Comment [IT6-Eagle] 機器構成変更No.2 HCLT-Deeparaj 2018/02/20 Display Envelope Key.
#ifndef DEF_A4_Cus
			Key_EnvelopeSize.showEnable( True );
//Comment [IT6-Eagle] [1044524] HCLT-Deeparaj 2018/04/19 Change the key text according to the destination.
			ChangeKeyTextForMarketArea();
#endif	//DEF_A4_Cus.
			showEnablePaperSizePartsOfTray1();
		}
		else if( TYPE_FeedTray2 == getSelectedFeedTray() )
		{
// comment 2016/10/17 SCD2-2G南波 [IT5_4.0][RQ-226] 不定型紙の標準カセット対応.
//			Grp_OtherPaperSize.showEnableAll( False );		// その他サイズで使用されるグループ.
//			Rbt_SelectPaperSize.showEnableAll( False );		// 検知サイズ候補で使用されるグループ.
//// comment 2016/03/30 OPS2-HIENG伊藤秀 [CQ525614]自動検出キー非表示処理追加.
//#if defined(DEF_A4_Cus)
//			Key_AutoSearch.showEnable( False );
//			Txt_PaperSizeEx.showEnable( False );
//			Bak_SelectPaperSizeArea.showEnable( False );	// 検知サイズ候補背景.
//			Bak_PaperSizeGrpBaseEx.showEnable( False );
//#else
//			Key_AutoSearch.showEnable( True );
//#endif
////	comment 2015/01/08 HCLT_Naveen [HeliosL][機器構成変更_No.13] Support to hide 12x18 Key in HeliosL Model.
////#ifdef DEF_Zeus5BkCus
//#if defined(DEF_Zeus5BkCus) || defined(DEF_HeliosCus)
//			Key_12x18.showEnable( False );
//#else //	#if defined(DEF_Zeus5BkCus) || defined(DEF_HeliosCus).
//			Key_12x18.showEnable( True );
//#endif //	#if defined(DEF_Zeus5BkCus) || defined(DEF_HeliosCus).
//			Key_WidePaper.showEnable( True );

#ifndef DEF_A4_Cus
			TYPE_FeedTray  e_FeedTray  = getSelectedFeedTray();
			// 不定形対応トレイかの判断.
			if ( PnlComIsUnfixedPaperSizeTray( e_FeedTray ) ) {
				Grp_OtherPaperSize.showEnableAll( True );		// その他サイズで使用されるグループ.
				Kex_8k16kSize.changePosition( 602, 236 );			//その他キー位置移動.
				Kex_MetricSize.changePosition( 434, 276 );			//A系B系キー位置移動.
				Kex_InchSize.changePosition( 602, 276 );			//インチ系キー位置移動.
				Rbt_SelectPaperSize.showEnableAll( False );		// 検知サイズ候補で使用されるグループ.
				Key_PaperSizeMeasurement_copyFD.showEnable( True ); //サイズ測定.
				Key_PaperSizeMeasurement_copyNoneFD.showEnable( False );
				Key_AutoSearch.showEnable( True );					//自動検出.
//Comment [IT6-Eagle] 機器構成変更No.2 HCLT-Deeparaj 2018/02/20 Display Envelope Key.
#ifndef DEF_A4_Cus
				Key_EnvelopeSize.showEnable(True);
#else	//DEF_A4_Cus.
				Key_EnvelopeSize.showEnable(False);					//はがき/封筒.
#endif	//DEF_A4_Cus.
				Key_EnvelopeSize_SP.showEnable( False );
// Comment 2019/07/26 [HCLT_Paul] [IT6_HeliosMLK] [ Seigyu.10 ] Adding the HeliosMLK compiler switch.
#if defined(DEF_Zeus5BkCus) || defined(DEF_HeliosMLKCus)
				Key_12x18_Indeterminate.showEnable( False );
				Key_12x18.showEnable( False );
#else //	#if defined(DEF_Zeus5BkCus) || defined(DEF_HeliosCus).
				Key_12x18_Indeterminate.showEnable( True );
				Key_12x18.showEnable( False );
#endif //	#if defined(DEF_Zeus5BkCus) || defined(DEF_HeliosCus).
				Key_WidePaper.showEnable( True );					//ワイド紙.
			} else {
				Grp_OtherPaperSize.showEnableAll( False );		// その他サイズで使用されるグループ.
				Rbt_SelectPaperSize.showEnableAll( False );		// 検知サイズ候補で使用されるグループ.
				Key_AutoSearch.showEnable( True );
// Comment 2019/07/26 [HCLT_Paul] [IT6_HeliosMLK] [ Seigyu.10 ] Adding the HeliosMLK compiler switch.
#if defined(DEF_Zeus5BkCus) || defined(DEF_HeliosMLKCus)
				Key_12x18.showEnable( False );
				Key_12x18_Indeterminate.showEnable( False );
#else //	#if defined(DEF_Zeus5BkCus) || defined(DEF_HeliosCus).
				Key_12x18.showEnable( True );
				Key_12x18_Indeterminate.showEnable( False );
#endif //	#if defined(DEF_Zeus5BkCus) || defined(DEF_HeliosCus).
				Key_WidePaper.showEnable( True );
			}
#else	//#ifndef DEF_A4_Cus.
			Grp_OtherPaperSize.showEnableAll( False );		// その他サイズで使用されるグループ.
			Rbt_SelectPaperSize.showEnableAll( False );		// 検知サイズ候補で使用されるグループ.
			Key_AutoSearch.showEnable( False );
// [KronosS] Sky川名 2016/10/20 管理No21_44 各トレイ毎の表示用紙サイズをKronosS仕様にあわせる.
#ifdef DEF_KRONOSS_Cus
			Kex_MetricSize.showEnable( True );		// A系・B系キー.
			Kex_InchSize.showEnable( True );		// インチ系キー.
			// FLSTypeに8_1/2inch x 13inchが設定されていた場合その他キーを表示する.
			if( TYPE_FLSType85x13 == PnlComGetMachineSetting()->getFLSType()) {
				Kex_8k16kSize.showEnable( True );		// その他キー.
			}
#else // DEF_KRONOSS_Cus.
			Txt_PaperSizeEx.showEnable( False );
			Bak_SelectPaperSizeArea.showEnable( False );	// 検知サイズ候補背景.
			Bak_PaperSizeGrpBaseEx.showEnable( False );
#endif // DEF_KRONOSS_Cus.
			Key_12x18.showEnable( True );
			Key_12x18_Indeterminate.showEnable( False );
			Key_WidePaper.showEnable( True );
#endif	//#ifndef DEF_A4_Cus.
//Comment 2018/11/29 HCLT_Sarada [IT6_DenebMLK][制御構成変更_No.45] changing paper Size.
#ifdef DEF_DenebMLKCus
			Key_EnvelopeSize.showEnable(False);
			Key_CustomSize.showEnable( False );
			Key_WidePaper.showEnable( False );
		if ( (TYPE_SubSystemType5 != e_SubSystemType) && ( (TYPE_SubSystemType4 == e_SubSystemType) || (TYPE_SubSystemType3 == e_SubSystemType)  ) ) 	{
			Kex_8k16kSize.showEnable( True );
		} 		
		if(TYPE_SystemType_US == e_SystemType){
			Kex_8k16kSize.showEnable( True );		// その他キー.
		}
		if ( TYPE_SystemType_US == e_SystemType ){
			Kex_MetricSize.showEnable( False );
		}	else { 
			Kex_MetricSize.showEnable( True );
		}
//Comment 2018/11/29 HCLT_Sarada [IT6_DenebMLK][AR1060719] Removing Inch for tray2-5 Others2 destination.
//		if( ( TYPE_SubSystemType3 == e_SubSystemType ) || ( TYPE_SystemType_US == e_SystemType ) ) {
		if ( TYPE_SystemType_US == e_SystemType )  {
			Kex_InchSize.showEnable( True );
		}else {
			Kex_InchSize.showEnable( False );
		}
#endif //DEF_DenebMLKCus.
		}
		else if( TYPE_FeedTray3 == getSelectedFeedTray() ) {
			showEnablePaperSizePartsOfTray3();
		}
		else if( TYPE_FeedTray4 == getSelectedFeedTray() ) {
			showEnablePaperSizePartsOfTray4();
		}
		else if( TYPE_TrayTypeLCT == getSelectedTrayType() ) {
			Txt_PaperSize.showEnable( False );
			Bak_PaperSizeAndDirectionArea.showEnable( False );
			Key_AutoSearch.showEnable( False );
			Key_12x18.showEnable( False );
// comment 2016/10/17 SCD2-2G南波 [IT5_4.0][RQ-226] 不定型紙の標準カセット対応.
			Key_12x18_Indeterminate.showEnable( False );
			Grp_OtherPaperSize.showEnableAll( False );					// その他サイズで使用されるグループ.
			Rbt_SelectPaperSize.showEnableAll( False );					// 検知サイズ候補で使用されるグループ.
			Bak_SelectPaperSizeArea.showEnable( False );				// 検知サイズ候補背景.
			Gra_InfoIcon.showEnable( False );							// インフォアイコン.
			Txt_InfoMsg.showEnable( False );							// インフォメッセージ.
			Key_WidePaper.showEnable( False );
			Bak_PaperSizeGrpBase.showEnable( False );
// comment 2014/11/28 OPS1-Sky東島 [ZeusS][機器構成変更No.11] 普通紙用紙分割.
			Txt_PaperSizeEx.showEnable(False);
			Bak_PaperSizeGrpBaseEx.showEnable(False);
		}
//comment 2018/12/08 HCLT-Basker [DenebMLK][制御構成変更_No.45 ]Added new function for tray5 to support DenebMLK models.
#ifdef DEF_DenebMLKCus
		else if( TYPE_FeedTray5 == getSelectedFeedTray() ) {
			showEnablePaperSizePartsOfTray5();
			}
#endif // DEF_DenebMLKCus.

		Key_DoubleSide.showEnable( False );
// comment 2015/05/18 OPS1-MSE渡邊(哲) [MinervaSBK][KRQ-49] 不定形用紙サイズ自動検出機能の搭載.
// comment 2016/10/17 SCD2-2G南波 [IT5_4.0][RQ-226] 不定型紙の標準カセット対応.
//		// FDセンサあり・なしサイズ測定キーを非表示.
//		Key_PaperSizeMeasurement_copyFD.showEnable( False );
//		Key_PaperSizeMeasurement_copyNoneFD.showEnable( False );
#ifndef DEF_A4_Cus
		if ( True != PnlComIsUnfixedPaperSizeTray( getSelectedFeedTray() ) ) {
			// FDセンサあり・なしサイズ測定キーを非表示.
			Key_PaperSizeMeasurement_copyFD.showEnable( False );
			Key_PaperSizeMeasurement_copyNoneFD.showEnable( False );
		}
#else	//#ifndef DEF_A4_Cus.
		// FDセンサあり・なしサイズ測定キーを非表示.
		Key_PaperSizeMeasurement_copyFD.showEnable( False );
		Key_PaperSizeMeasurement_copyNoneFD.showEnable( False );
#endif	//#ifndef DEF_A4_Cus.
	}
	if( True == APIC_SoftDipSw::newInstance()->getSoftDipSwBitForPanelSpecNo( DEFSOFTWARESW218, 3 ) ){
		//DipSW218 bit3=On時はワイドキー非表示.
		Key_WidePaper.showEnable(False);
	}
// 2018/1/2 HCLT-Vandana Modification for [IT6_Sp][機器構成変更_No.201] Inch/Other Paper Size.
#ifdef DEF_SparrowCus
		if( TYPE_FeedTrayManual == getSelectedFeedTray() ) {
			Key_CustomSize.changePosition( 432, 236 );
	}
#endif //#ifdef DEF_SparrowCus
}

//-----------------------------------------------------------------------------------------------
/**
 *	@brief	A020114100 選択トレイの設定変更 : 手差しトレイ以外時のカスタムキー非表示判断処理.
 */
//-----------------------------------------------------------------------------------------------
Bool PCPC_PaperEnl_copy::eraseJudgePaperTypeStd( TYPE_PaperType e_PaperType, TYPE_CustomPaperType e_CustomPaperType )
{
	Bool bl_showEnable = True;		// 表示/非表示フラグ.

	switch( e_PaperType ) {
		case TYPE_PaperTypeOHP:				// OHP.
		case TYPE_PaperTypeThick4:			// 厚紙4.
		case TYPE_PaperTypeEnvelope:		// 封筒.
		case TYPE_PaperTypeIndex:			// インデックス紙.
			bl_showEnable = False;
			break;
		default:
			// 他の用紙種類はチェック不要.
			break;
	}
	// カスタム用紙種別が、カスタム用紙20の場合.
	if( TYPE_CustomPaperType20 == e_CustomPaperType ) {
		bl_showEnable = False;
	}
	return bl_showEnable;
}


//-----------------------------------------------------------------------------------------------
/**
 *	@brief	A020114100 選択トレイの設定変更 : 設定共通処理.
 */
//-----------------------------------------------------------------------------------------------
//void PCPC_PaperEnl_copy::setPaperTypeCom(const TYPE_PaperType e_PaperType, const Bool b_2ndSidedPaper)
//void PCPC_PaperEnl_copy::setPaperTypeCom(const TYPE_PaperType e_PaperType, const Bool b_2ndSidedPaper, const TYPE_CustomPaperType e_CustomPaperType)
void PCPC_PaperEnl_copy::setPaperTypeCom( const TYPE_PaperType e_PaperType, const Bool b_2ndSidedPaper, const TYPE_CustomPaperType e_CustomPaperType, const PNLE_PaperEx e_Ex )
{
	// 対象トレイの用紙種類を取得.
	const TYPE_FeedTray e_FeedTray = getSelectedFeedTray();

	TYPC_PaperType c_PaperType = PnlComGetFeedTray()->getPaperType( e_FeedTray );

	// 画像位置指定の取得.
	const TYPE_WidePaperImagePosition e_WidePaperImagePosition = c_PaperType.getWidePaperImagePosition();

	// 用紙サイズの取得.
	TYPC_PaperSize c_PaperSize = PnlComGetFeedTray()->getPaperSize( e_FeedTray );
	if( ( NULL == PnlComGetJobCopy() ) ||
	    ( NULL == PnlComGetJobCopy()->getJobSpec() ) ||
	    ( NULL == PnlComGetJobCopy()->getJobSpec()->pc_Print ) ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "[PCPC_PaperEnl_copy::setPaperTypeCom]Assert Failed!" );
		return;
	}
	TYPC_Print* pc_Print = PnlComGetJobCopy()->getJobSpec()->pc_Print;

	TYPE_CustomPaperType e_CustomPaperType_local = e_CustomPaperType;
	TYPE_PaperType e_PaperType_local = e_PaperType;;

	// カスタム用紙属性から用紙種類取得.
	PNLC_SelectPaperTrayCom_DataLib* pDataLib = PNLC_SelectPaperTrayCom_DataLib::newInstance();

	// 用紙種別に、再生紙が設定されている場合.
	if( TYPE_PaperRecycle1 == e_PaperType ) {
		e_PaperType_local = pDataLib->getPaperTypeFromCustomPaperType( TYPE_CustomPaperType20 );
		e_CustomPaperType_local = TYPE_CustomPaperType20;
	}

	// ジョブに用紙種類を設定.
	pc_Print->c_PaperType.setPaperType( e_PaperType_local );				// comment MinervaZeus CHG：e_PaperType→e_PaperType_local Sky上西 2011/05/05 [UV-01038][機器1001].
	pc_Print->c_PaperType.setFeed2ndSidedPaper( b_2ndSidedPaper );
	pc_Print->c_PaperType.setWidePaperImagePosition( e_WidePaperImagePosition );
	pc_Print->c_PaperType.setCustomPaperType( e_CustomPaperType_local );	// comment MinervaZeus CHG：e_CustomPaperType→e_CustomPaperType_local Sky上西 2011/05/05 [UV-01038][機器1001].
	if( PNLE_PaperEx_NotSet != e_Ex ) {
		pc_Print->c_PaperType.setPaperTypeNormalEx( ( ( PNLE_PaperEx_On == e_Ex ) ? True : False ) );
	}
// comment 2014/11/28 OPS1-Sky東島 [ZeusS][機器構成変更No.11] 普通紙用紙分割.
	if( TYPE_PaperTypeNormal != e_PaperType ){		// 普通紙でない場合はEco/厚さ変更を解除する.
		pc_Print->c_PaperType.setPaperTypeNormalEx( False );
	}

	// ジョブに用紙サイズを設定.
	pc_Print->c_PaperSize = c_PaperSize;

	// トレイへの設定.
	onlyPanel.setFeedSizeKind( e_FeedTray );

	// APS/AMS処理関数.
	selectAutoPaperMag();
}

//-----------------------------------------------------------------------------------------------
/**
 *	@brief	A020114100 選択トレイの設定変更 : ボタンインデックス取得処理.
 */
//-----------------------------------------------------------------------------------------------
short PCPC_PaperEnl_copy::getPaperTypeBtnIndex( const TYPE_PaperType e_PaperType )
{
	short ss_Type;
	getPaperKindInfo( ss_Type );
	// 手差し.
	if( PNLE_PEPARKIND_TYPE_MUL == ss_Type )
	{
		for( uchar i =0; i < uc_BtPaperTypeMultiManTblSize; i++ )
		{
			if( e_PaperType == uc_BtPaperTypeMultiManTbl[i][0] )
			{
				return uc_BtPaperTypeMultiManTbl[i][1];
			}
		}
	}
//Comment [IT6-Eagle] 機器構成変更No.2 HCLT-Deeparaj 2018/02/20 When Tray 1 is selected.
#ifndef DEF_A4_Cus
	else if( PNLE_PEPARKIND_TYPE_TRY1 == ss_Type )
	{
		for( uchar i =0; i < uc_BtPaperTypeTblSizeTry1; i++ )
		{
			if( e_PaperType == uc_BtPaperTypeTblTry1[i][0] )
			{
				return uc_BtPaperTypeTblTry1[i][1];
			}
		}
	}
#endif	//DEF_A4_Cus.
	else if( PNLE_PEPARKIND_TYPE_STD == ss_Type )
	{
		for( uchar i =0; i < uc_BtPaperTypeTblSize; i++ )
		{
			if( e_PaperType == uc_BtPaperTypeTbl[i][0] )
			{
				return uc_BtPaperTypeTbl[i][1];
			}
		}
	}
	return -1;
}

//-----------------------------------------------------------------------------------------------
/**
 *	@brief	A020114100 選択トレイの設定変更 : アイコンインデックス取得処理.
 */
//-----------------------------------------------------------------------------------------------
short PCPC_PaperEnl_copy::getPaperTypeIconIndex( const TYPE_PaperType e_PaperType )
{
	// 設定変更の対象となるトレイの取得.
	const TYPE_FeedTray e_FeedTray = getSelectedFeedTray();
	// 用紙種類、両面2面目フラグの取得.
	TYPC_PaperType c_PaperType = PnlComGetFeedTray()->getPaperType( e_FeedTray );
	const Bool b_2ndSidedPaper = c_PaperType.getFeed2ndSidedPaper();

	// 両面設定OFF.
	if( False == b_2ndSidedPaper )
	{
		for( uchar i = 0; i < uc_IconPaperTypeSize; i++ )
		{
			if( e_PaperType == uc_IconPaperType[i][0] )
			{
				return uc_IconPaperType[i][1];
			}
		}
		return -1;
	}
	// 両面設定ON.
	else
	{
		for( uchar i = 0; i < uc_IconPaperTypeDobuleSize; i++ )
		{
			if( e_PaperType == uc_IconPaperTypeDobule[i][0] )
			{
				return uc_IconPaperTypeDobule[i][1];
			}
		}
		return -1;
	}
}

//-----------------------------------------------------------------------------------------------
/**
 *	@brief	A020114100 選択トレイの設定変更 : 組み禁時の先設定変更許可通知ハンドラ.
 */
//-----------------------------------------------------------------------------------------------
void PCPC_PaperEnl_copy::procChangePreSettingOk( uchar option )
{

	switch( sp_MessageWarningTable[ onlyPanel.us_NowMessageWarning ].ec_WarningName ) {
		case PNL_ERROR_Combination_ACS :
		case PNL_ERROR_Combination_FullColorMode :
		case PNL_ERROR_Combination_2Color:
		case PNL_ERROR_Combination_MonoColor:
			if( CNK_SpecialPaperOHP == onlyPanel.ep_CombinationOrigin ) {
				if( ( NULL == PnlComGetJobCopy() ) ||
				    ( NULL == PnlComGetJobCopy()->getJobSpec() ) ||
				    ( NULL == PnlComGetJobCopy()->getJobSpec()->pc_Scan ) ||
				    ( NULL == PnlComGetJobCopy()->getJobSpec()->pc_Common ) ) {
					LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "[PCPC_PaperEnl_copy::procChangePreSettingOk]Assert Failed!" );
					return;
				}
				PnlComGetJobCopy()->getJobSpec()->pc_Scan->setAcsOn( False );
				PnlComGetJobCopy()->getJobSpec()->pc_Common->setColorMode( TYPE_ColorModeBlack );
				PnlComGetJobCopy()->getJobSpec()->pc_Common->c_MonoColor.setMonoColorOn( False );
				PnlComGetJobCopy()->getJobSpec()->pc_Common->c_MonoColor.setMonoColor( TYPE_MonoColorRed );
				PnlComGetJobCopy()->getJobSpec()->pc_Common->c_2ColorMode.set2ColorOn( False );
				PnlComGetJobCopy()->getJobSpec()->pc_Common->c_2ColorMode.setMonoColor( TYPE_MonoColorRed );
			}
			break;
		default :
			break;
	}

	if( PNLE_PaperTypeCategory_Normal == m_e_PaperTypeCategory ) {		// 用紙種類カテゴリが標準用紙種類の場合.

		EP_Combination e_Combination = CNK_Size;
		switch( onlyPanel.ep_CombinationOrigin ) {
			case CNK_SpecialPaperOHP:
				e_Combination = CNK_SpecialPaperOHP;
				break;
			default:
				break;
		}

		if( CNK_Size != e_Combination ) {
			// 用紙種類テーブルの取得.
			short ss_Type = 0;
			const TYPS_PaperKindInfo* pst_AryPaperKindInfo = getPaperKindInfo( ss_Type );
			const uchar uc_PaperKindInfoSize = getPaperKindInfoSize();

			for( uchar i = 0;  uc_PaperKindInfoSize > i; ++i ) {
				if( pst_AryPaperKindInfo[ i ].e_Combination == e_Combination ) {
					if( True == combinationCheck( pst_AryPaperKindInfo[ i ].e_Combination, PnlComGetJobCopy() ) ) {
						setPaperTypeCom( pst_AryPaperKindInfo[ i ].e_PaperType, pst_AryPaperKindInfo[ i ].b_2ndSidedPaper );
						break;
					}
				}
			}
		}
	} else {															// 用紙種類カテゴリが標準用紙種類でない(カスタム用紙種類)場合.
		// カスタム用紙種類ボタンIDを指定して、カスタム用紙種類設定処理をコール.
		setCustomPaperType( m_uc_CustomPaperTypeBtnID );
	}
}

//クラス-----------------------------------------------------------------------------------------------------------------.
//概要    ：メッセージの変更.
//備考    ：.
//作成者  ：.
//レビュー：.
//変更履歴：.
//-----------------------------------------------------------------------------------------------.
/**
* @brief A020114100 選択トレイの設定変更 : メッセージの変更.
*/
//-----------------------------------------------------------------------------------------------.
void PCPC_PaperEnl_copy::changeMessage()
{
	ushort us_MainTextID = 0;	// メインメッセージ.
	ushort us_SubTextID1 = 0;	// サブメッセージの１.
	ushort us_SubTextID2 = 0;	// サブメッセージの２.

// comment 20/15/02/27 OPS1南波 Helios CQ510016 FDセンサー判定追加.
//	// 手差しトレイ選択中のみ表示.
//	if( TYPE_FeedTrayManual == getSelectedFeedTray() ) {
	// 手差しトレイ選択中且つ、FDセンサーなしのみ表示.
	if( ( TYPE_FeedTrayManual == getSelectedFeedTray() ) 
	 && ( False == PnlComGetPrinterSystem()->isManualTrayFDSizeDetect() ) ){
// VenusMLK CQ:529869 Sky川名.
#ifdef DEF_A4_Cus
		us_MainTextID = s_pMSGCheckPapeTypeAndSetPaperGuide2_W448H034A;	// Comment VenusMLK <- 2016/07/12 Sky高田(庸) [CQ532425][UD指摘No.9-1]メッセージ変更.
		
#else
// comment 2016/06/17 [HCL_VijayK] [IT5_3.3][CQ1002027] Quater font activity changes.
//		us_MainTextID = s_pMSGCheckPapeTyoeAndSetPaperSize_W448H034A;
// comment 2019/10/15 [HCL_Madhavi] [IT6_HeliosMLk][機器構成変更_No.22] Supports manual feed size detection
#if defined( DEF_HeliosMLKCus )
	 	us_MainTextID = s_pMSGCheckPapeTypeAndSetPaperGuide2_W448H034A;
#else
		us_MainTextID = s_pMSGCheckPapeTyoeAndSetPaperSize_W756H034A;
#endif
#endif
// VenusMLK CQ:529869 Sky川名.
	}

	// 0がセットされる場合、Component側で設定されているメッセージがあれば表示される.
	// 0以外がセットされる場合、このメッセージが表示される.
	Cmp_CopyPaperTypeUniversalSettingChangePMA.changeMainMessageText( us_MainTextID );
	Cmp_CopyPaperTypeUniversalSettingChangePMA.changeSubMessageText2( us_SubTextID1, us_SubTextID2 );
}

//-----------------------------------------------------------------------------------------------
/**
 *	@brief	A020114100 選択トレイの設定変更 : キー文言を仕向けによって変更する.
 */
//-----------------------------------------------------------------------------------------------
void PCPC_PaperEnl_copy::ChangeKeyTextForMarketArea()
{
	// 仕向によって表示されるシンボルを切り替える.
	TYPE_SystemType e_systemType = PnlComGetDestination();	// 仕向け取得.
	switch( e_systemType ) {
		// 封筒／はがき.
		case TYPE_SystemType_JP:	// 国内仕向.
			Key_EnvelopeSize.changeText( s_pKEYEnvPostCard_W108H028A,1 );
			break;
		// Env/4x6.
		case TYPE_SystemType_US:	// 北米仕向.
			Key_EnvelopeSize.changeText( s_pKEYEnv4x6_W108H028A,1 );
			break;
		case TYPE_SystemType_EU:	// 欧州仕向.
		// Env/A6Card.
			Key_EnvelopeSize.changeText( s_pKEYEnvelopeA6Card_W104H028A,1 );
			break;
		default:
			break;
	}
}
// comment 2016/05/19 SCD2-2G-Sky福田 [ZeusSBK][CQ529248] TBD230対応.
//-----------------------------------------------------------------------------------------------
/**
 *	@brief	A020114100 選択トレイの設定変更 : 定型用紙サイズ表示.
 */
//-----------------------------------------------------------------------------------------------
void PCPC_PaperEnl_copy::setupStandardPaperSize()
{
	APIC_FeedTray* pc_FeedTray = PnlComGetFeedTray();
	
	if( NULL == pc_FeedTray ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "[PCPC_PaperEnl_copy::setupStandardPaperSize]Assert Failed!" );
		return;
	}
	
	short ss_SizePosX = 0;
	short ss_SizePosY = 0;
	short ss_DirPosX = 0;
	short ss_DirPosY = 0;
	
	PSinglePartsTextEx* pc_TextPeperSize = &Txt_DispPaperSize1;
	PSinglePartsTextEx* pc_TextPeperDir = &Txt_DispPaperSizeDir1;
	if( True == PnlComGetPrintJobSetting()->getPaperTypeNormalEx() ) {			// 普通紙用紙分割有効.
		pc_TextPeperSize = &Txt_DispPaperSize1Ex;
		pc_TextPeperDir = &Txt_DispPaperSizeDir1Ex;
	}
	
	if( ( NULL == pc_TextPeperSize ) || ( NULL == pc_TextPeperDir ) ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "[PCPC_PaperEnl_copy::setupStandardPaperSize]Assert Failed!" );
		return;
	}

	// 用紙サイズの表示座標取得.
	pc_TextPeperSize->getPosition( ss_SizePosX, ss_SizePosY );
	// 用紙向きの表示座標取得.
	pc_TextPeperDir->getPosition( ss_DirPosX, ss_DirPosY );

	const TYPE_FeedTray e_FeedTray = getSelectedFeedTray();
	// 用紙に関する情報（サイズ・種類・状態）を取得.
	TYPC_PaperSize c_PaperSize = pc_FeedTray->getPaperSize( e_FeedTray );
	if( TYPE_SizeCodeNonStandard == c_PaperSize.e_SizeCode ) {
		// 置き換え無し.
		pc_TextPeperDir->showEnable( False );
		return;
	}
	
	// ワイド紙の場合.
	if( True == c_PaperSize.b_WideOn ) {
		// 置き換え無し.
		pc_TextPeperDir->showEnable( False );
		return;
	}

	TEXT_ID us_SizeTextID = 0;
	TEXT_ID us_DirTextID = 0;
	short ss_DirPosX_Offset = 0;

	// FoolsCapサイズ(読み込みサイズとテキストIDテーブルのシンボルサイズが合致したとき).
	// 	サービス設定項目で設定するものについては別途case文を用意.
	if( TYPE_SizeCodeFlsSef == c_PaperSize.e_SizeCode ) {
		// セットしたサイズはどのサイズかで振り分ける.
		switch( pc_MachineSetting->getFLSType() ) {
			case TYPE_FLSType85x135:	// 8_1/2inch x 13_1/2inch.
				us_SizeTextID = s_pNOTPaperSizeFoolscap_001_W056H036A;
				us_DirTextID = s_pNOTSef_W036H036A;
				ss_DirPosX_Offset = 36;
				break;
			case TYPE_FLSType220x330:	// 220mm x 330mm.
				us_SizeTextID = s_pNOTSettingConfirm_PaperSizeFoolscap_002_W324H020A;
				us_DirTextID = 0;
				break;
			case TYPE_FLSType85x13:		// 8_1/2inch x 13inch.
				us_SizeTextID = s_pNOTPaperSizeFoolscap_003_W056H036A;
				us_DirTextID = s_pNOTSef_W036H036A;
				ss_DirPosX_Offset = 36;
				break;
			case TYPE_FLSType825x13:	// 8_1/4inch x 13inch.
				us_SizeTextID = s_pNOTPaperSizeFoolscap_004_W056H036A;
				us_DirTextID = s_pNOTSef_W036H036A;
				ss_DirPosX_Offset = 36;
				break;
			case TYPE_FLSType8125x1325:	// 8_1/8inch x 13_1/4inch.
				us_SizeTextID = s_pNOTPaperSizeFoolscap_005_W056H036A;
				us_DirTextID = s_pNOTSef_W036H036A;
				ss_DirPosX_Offset = 36;
				break;
			case TYPE_FLSType8x13:		// 8inch x 13inch.
				us_SizeTextID = s_pNOTPaperSizeFoolscap_006_W056H036A;
				us_DirTextID = s_pNOTSef_W036H036A;
				ss_DirPosX_Offset = 36;
				break;
			default:
				// なにもしない.
				break;
		}
		
		// 用紙サイズ.
		pc_TextPeperSize->changeText( us_SizeTextID );
		
		// 用紙向き.
		if( 0 != us_DirTextID ) {
			pc_TextPeperDir->showEnable( True );
			pc_TextPeperDir->changePosition( static_cast<short>( ss_SizePosX + ss_DirPosX_Offset + PNLD_PaperEnl_PaperSizeDir_CorrectX ), ss_DirPosY );
			pc_TextPeperDir->changeText( us_DirTextID );
		} else {
			pc_TextPeperDir->showEnable( False );
		}
		return;
	}
	
	for( uchar uc_index = 0; uc_index < ( sizeof(st_ReplacePaperSizeTable) / sizeof(PNLS_PaperEnl_copyReplaceTable) ); uc_index++ ) {
		// 用紙サイズ一致.
		if( st_ReplacePaperSizeTable[ uc_index ].e_SizeCode == c_PaperSize.e_SizeCode ) {
			if( TYPE_SizeCodeJpLetterY2 == c_PaperSize.e_SizeCode ) {
#if defined ( DEF_KRONOSS_Cus ) || defined ( DEF_SparrowCus ) // 2017/12/21 HCLT-Vandana Modification for [IT6_Sp][機器構成変更_No.282]
				if( TYPE_SystemType_JP != PnlComGetDestination() ) {
					us_SizeTextID = s_pNOTenvC6_W096H018A;
				} else {
					us_SizeTextID = s_pITMY2SEF_W096H018A;
				}
#else // defined ( DEF_KRONOSS_Cus ) || defined ( DEF_SparrowCus )
				if( TYPE_SystemType_JP != PnlComGetDestination() ) {
					us_SizeTextID = s_NOTenvC6;
				} else {
					us_SizeTextID = s_pKEYEnvY2_W096H028A;
				}
#endif // defined ( DEF_KRONOSS_Cus ) || defined ( DEF_SparrowCus )
			} else {
				us_SizeTextID = st_ReplacePaperSizeTable[ uc_index ].us_DstTextID;
			}
			
			// 用紙サイズ.
			pc_TextPeperSize->changeText( us_SizeTextID );
			// 用紙向き.
			if( 0 != st_ReplacePaperSizeTable[ uc_index ].us_DstDirTextID ) {
				pc_TextPeperDir->showEnable( True );
				pc_TextPeperDir->changePosition( static_cast<short>( ss_SizePosX + st_ReplacePaperSizeTable[ uc_index ].ss_PosX_Offset + PNLD_PaperEnl_PaperSizeDir_CorrectX ), ss_DirPosY );
				pc_TextPeperDir->changeText( st_ReplacePaperSizeTable[ uc_index ].us_DstDirTextID );
			} else {
				pc_TextPeperDir->showEnable( False );
			}
			break;
		}
	}
}

//-----------------------------------------------------------------------------------------------
/**
 *	@brief	A020114100 選択トレイの設定変更 : トレイ表示.
 */
//-----------------------------------------------------------------------------------------------
void PCPC_PaperEnl_copy::setupPaperTray()
{
	PNLS_Parts* ps_Grahic = tbl_i01_479_000_X16_Y16;
	
	// 選択されているトレイを取得.
	const TYPE_FeedTray e_FeedTray = getSelectedFeedTray();

	switch( e_FeedTray ) {
		case TYPE_FeedTray1:		//「１」.
			ps_Grahic = tbl_i01_902_000_X32_Y32;
			break;
		case TYPE_FeedTray2:		//「２」.
			ps_Grahic = tbl_i01_903_000_X32_Y32;
			break;
		case TYPE_FeedTray3:		//「３」.
			ps_Grahic = tbl_i01_904_000_X32_Y32;
			break;
		case TYPE_FeedTray4:		//「４」.
			ps_Grahic = tbl_i01_905_000_X32_Y32;
			break;
		case TYPE_FeedTray5:		//「L].
// comment 2018/05/17 SCD2-Sky中野 [DenebMLK][機器構成変更No.7] 5段増設プリント機能追加対応.
#ifdef DEF_DenebMLKCus
			ps_Grahic = tbl_i02_110_000_X32_Y32;
#else
			ps_Grahic = tbl_i01_906_000_X32_Y32;
// comment 2018/05/17 SCD2-Sky中野 [DenebMLK][機器構成変更No.7] 5段増設プリント機能追加対応.
#endif	// DEF_DenebMLKCus.
			break;
		case TYPE_FeedTrayManual:	// 手差し.
			ps_Grahic = tbl_i01_901_000_X32_Y32;
			break;
		default:
			break;
	}
// comment 2016/05/19 SCD2-2G-Sky福田 [ZeusSBK][CQ529248] TBD230.
	// ecoモード(普通紙分割)のときにもトレイアイコンサイズが反映されるように変更.
		if( True == PnlComGetPrintJobSetting()->getPaperTypeNormalEx() ) {			// 普通紙用紙分割有効.
			Gra_PaperSizeAndDirectionAreaEx.changeParts( ps_Grahic );
		} else {
			Gra_PaperSizeAndDirectionArea.changeParts( ps_Grahic );
		}
}

// comment 2019/01/22 SCD2-4G Sky武井 OP_BTS-2216【開発提起】【Nランク】【RQ-720】紙種自動検知がONで用紙種類が未確定の時に、未確定であることがわかるアイコンを用紙種類の設定値表示箇所に表示する。.
/**  
 *	@brief	用紙種類未確定アイコンインデックス取得.
 *	@param[in]	uc_Index		通常の用紙種類アイコンインデックス.
 *	@return	用紙種類未確定アイコンインデックス.
 */
uchar PCPC_PaperEnl_copy::getMediaUndecidedIndex( uchar uc_Index )
{
	if ( PNLD_PAPERTYPEIND_NON == uc_Index ) {
		return PNLD_PAPERTYPEIND_UNDECIDED_NORMAL;		// 用紙種類未確定アイコン.
	}
	for( ushort us_loop = 0; us_loop < sizeof( tbl_PaperTypUndecidedIconIndex )/sizeof( PNLS_PaperTypUndecidedIconIndex ); us_loop++ ) {
		if ( uc_Index == tbl_PaperTypUndecidedIconIndex[us_loop].uc_IconIndex ) {
			// 対応する用紙種類アイコンあり
			return tbl_PaperTypUndecidedIconIndex[us_loop].uc_UndecidedIconIndex;
		}
	}
	return PNLD_PAPERTYPEIND_UNDECIDED_NORMAL;			// 用紙種類未確定アイコン.
}

//-----------------------------------------------------------------------------------------------
/**
 *	@brief	A020114130_用紙ワイド紙 : 画面開始処理.
 */
//-----------------------------------------------------------------------------------------------
void PCPC_PaperSizeWideInch_copy::startGamen()
{
	GFWS_GraphicTable st_ABSizesTable;
	GFWS_GraphicTable st_InchSizesTabTable;

	//A系B系グラフィック.
	st_ABSizesTable.st_Normal.puc_Parts = ( unsigned char* )i00_192_000_n_X24_Y24;
	st_ABSizesTable.st_Normal.uc_PaletteType =             PNLD_PaletteCommon;
	st_ABSizesTable.st_Select.puc_Parts = ( unsigned char* )i00_192_000_n_X24_Y24;
	st_ABSizesTable.st_Select.uc_PaletteType =             PNLD_PaletteCommon;
	st_ABSizesTable.st_Disable.puc_Parts = ( unsigned char* )NULL;
	st_ABSizesTable.st_Disable.uc_PaletteType =            PNLD_PaletteCommon;
	//インチ系グラフィック.
	st_InchSizesTabTable.st_Normal.puc_Parts = ( unsigned char* )i00_191_000_n_X24_Y24;
	st_InchSizesTabTable.st_Normal.uc_PaletteType =             PNLD_PaletteCommon;
	st_InchSizesTabTable.st_Select.puc_Parts = ( unsigned char* )i00_191_000_n_X24_Y24;
	st_InchSizesTabTable.st_Select.uc_PaletteType =             PNLD_PaletteCommon;
	st_InchSizesTabTable.st_Disable.puc_Parts = ( unsigned char* )NULL;
	st_InchSizesTabTable.st_Disable.uc_PaletteType =            PNLD_PaletteCommon;

	//親クラスのstartGamenをコール.
	PPopupDecisionButtonCom::startGamen();
	Cmp_MessageBand.setOnOffButton( False );
	Cmp_MessageBand.setMode( PNLE_PopupMessageAreaMessageModeCopy );
	Cmp_MessageBand.showDestinationCount( False );
	Cmp_MessageBand.setNowPopupGamen( PNLE_CopyPaperWidth );

	register short ss_XPos = 0;
	register short ss_YPos = 0;
	getPosition( &ss_XPos, &ss_YPos );
	Cmp_TabWidePaper.setChildSpaceColor( PNLE_TabChildSpaceColorPaletteCopy );
	if( ( NULL == PnlComGetJobCopy() ) ||
	    ( NULL == PnlComGetJobCopy()->getJobSpec() ) ||
	    ( NULL == PnlComGetJobCopy()->getJobSpec()->pc_Print ) ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "[PCPC_PaperSizeWideInch_copy::startGamen]Assert Failed!" );
		return;
	}
	// JobSpecより用紙サイズ取得.
	TYPE_FeedTray e_FeedTray	= PnlComGetJobCopy()->getJobSpec()->pc_Print->c_PaperTray.getFeedTray();
	TYPC_PaperSize c_PaperSize	= PnlComGetFeedTray()->getPaperSize( e_FeedTray );

	if( TYPE_SystemType_US == PnlComGetDestination() ) {		//北米仕向けの場合.
		pc_PaperSizeSelect.initialSPC( PnlComGetJobCopy(), PNLE_PaperSizeTypeWideInch );
		Cmp_TabWidePaper.tabKeyIconChange( 0, &st_InchSizesTabTable );
		Cmp_TabWidePaper.tabKeyIconChange( 1, &st_ABSizesTable );
// comment 2015/04/17 OPS1 Sky東島 [Helios][CQ511913].
#ifdef DEF_LCD_7INCH
		Cmp_TabWidePaper.tabKeyTextChange( 0, s_pKEYInchSizes_W120H028A );
		Cmp_TabWidePaper.tabKeyTextChange( 1, s_pKEYABSizes_W120H028A );
#else // #ifdef DEF_LCD_7INCH.
		Cmp_TabWidePaper.tabKeyTextChange( 0, s_pKEYInchSizes_W112H028A );
		Cmp_TabWidePaper.tabKeyTextChange( 1, s_pKEYABSizes_W112H028A );
#endif // #ifdef DEF_LCD_7INCH.
		Cmp_TabWidePaper.tabKeyIndexChange( 0, 1 );
		Cmp_TabWidePaper.tabKeyIndexChange( 1, 0 );
	} else {
		pc_PaperSizeSelect.initialSPC( PnlComGetJobCopy(), PNLE_PaperSizeTypeWideMetric );
		Cmp_TabWidePaper.tabKeyIconChange( 0, &st_ABSizesTable );
		Cmp_TabWidePaper.tabKeyIconChange( 1, &st_InchSizesTabTable );
// comment 2015/04/17 OPS1 Sky東島 [Helios][CQ511913].
#ifdef DEF_LCD_7INCH
		Cmp_TabWidePaper.tabKeyTextChange( 0, s_pKEYABSizes_W120H028A );
		Cmp_TabWidePaper.tabKeyTextChange( 1, s_pKEYInchSizes_W120H028A );
#else // #ifdef DEF_LCD_7INCH.
		Cmp_TabWidePaper.tabKeyTextChange( 0, s_pKEYABSizes_W112H028A );
		Cmp_TabWidePaper.tabKeyTextChange( 1, s_pKEYInchSizes_W112H028A );
#endif // #ifdef DEF_LCD_7INCH.
		Cmp_TabWidePaper.tabKeyIndexChange( 0, 0 );
		Cmp_TabWidePaper.tabKeyIndexChange( 1, 1 );
	}

	setTabInfo( c_PaperSize );

	// タブコンポーネントへコールバック関数設定.
	Cmp_TabWidePaper.setCallBackFunc( ans_TabKey );
}

//-----------------------------------------------------------------------------------------------
/**
 *	@brief	A020114130_用紙ワイド紙 : 用紙サイズからタブ情報(タブキー・タブメニュー)を設定する.
 */
//-----------------------------------------------------------------------------------------------
void PCPC_PaperSizeWideInch_copy::setTabInfo( TYPC_PaperSize  c_PaperSize )
{
	short ss_index = -1;

	// 仕向け別タブキー初期表示.
	if( PnlComGetDestination() == TYPE_SystemType_US ) {
		Cmp_TabWidePaper.selectTab( PNLD_TabNumInch );
		m_SelectTabKeyOption = PNLD_TabNumInch;
	} else {
		Cmp_TabWidePaper.selectTab( PNLD_TabNumABSize );
		m_SelectTabKeyOption = PNLD_TabNumABSize;
	}

	// Ａ・Ｂ系確認.
	ss_index = getButtonIndexPaperSizeMetric( c_PaperSize );
	if( ss_index > -1 ) {
		m_SelectTabKeyOption = PNLD_TabNumABSize;
		Cmp_TabWidePaper.selectTab( PNLD_TabNumABSize );
		return;
	}

	// インチ系確認.
	ss_index = getButtonIndexPaperSizeInch( c_PaperSize );
	if( ss_index > -1 ) {
		m_SelectTabKeyOption = PNLD_TabNumInch;
		Cmp_TabWidePaper.selectTab( PNLD_TabNumInch );
		return;
	}
}

//-----------------------------------------------------------------------------------------------
/**
 *	@brief	A020114130_用紙ワイド紙 : テーブルインデックス取得(Ａ・Ｂ).
 */
//-----------------------------------------------------------------------------------------------
short PCPC_PaperSizeWideInch_copy::getButtonIndexPaperSizeMetric( TYPC_PaperSize  c_PaperSize )
{
	short ss_Select = -1;

	if( True == c_PaperSize.b_WideOn ) {
		for( uchar i = 0; i < uc_PaperSizeSelectTableWideMetric; i++ ) {
			if( c_PaperSize.e_SizeCode == st_PaperSizeSelectTableWideMetric[i].e_SizeCode ) {
				ss_Select = ( short )i;
				break;
			}
		}
	}
	return ss_Select;
}

//-----------------------------------------------------------------------------------------------
/**
 *	@brief	A020114130_用紙ワイド紙 : テーブルインデックス取得(インチ).
 */
//-----------------------------------------------------------------------------------------------
short PCPC_PaperSizeWideInch_copy::getButtonIndexPaperSizeInch( TYPC_PaperSize  c_PaperSize )
{
	short ss_Select = -1;
	if( True == c_PaperSize.b_WideOn ) {
		for( uchar i = 0; i < uc_PaperSizeSelectTableWideInch; i++ ) {
			if( c_PaperSize.e_SizeCode == st_PaperSizeSelectTableWideInch[i].e_SizeCode ) {
				ss_Select = ( short )i;
				break;
			}
		}
	}
	return ss_Select;
}
//-----------------------------------------------------------------------------------------------
/**
 *	@brief	A020114130_用紙ワイド紙 : ボタン押下処理.
 */
//-----------------------------------------------------------------------------------------------

void PCPC_PaperSizeWideInch_copy::procButtonOn( uchar option )
{
	TYPE_SizeCode _e_SizeCode = TYPE_SizeCodeUnknown;
	if( ( NULL == PnlComGetJobCopy() ) ||
	    ( NULL == PnlComGetJobCopy()->getJobSpec() ) ||
	    ( NULL == PnlComGetJobCopy()->getJobSpec()->pc_Print ) ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "[PCPC_PaperSizeWideInch_copy::procButtonOn]Assert Failed!" );
		return;
	}
	TYPC_Print* pc_Print = PnlComGetJobCopy()->getJobSpec()->pc_Print;
	TYPC_PaperSize c_PaperSize = pc_PaperSizeSelect.getPaperSize();
// comment 2017/06/16 SCD2-Sky後藤 [IT5_4.1][AR1024210] 不定形サイズ可能トレイの印刷出来るサイズ範囲外の用紙を設定できない処理を追加.
	TYPE_FeedTray e_FeedTray = pc_PaperSizeSelect.getSelectedFeedTray();
	short x = 0, y = 0, w = 0, h = 0;
// Comment 2020/04/18 [HCL_Prasanjeet][IT6_EagLeZ+][OP_BTS-22867] paper type canceletion.
	Bool isBeforePostCardOrEnv = False;
	APIC_FeedTray* pc_FeedTray = PnlComGetFeedTray();
	TYPC_PaperType c_PaperType = pc_FeedTray->getPaperType(e_FeedTray);

    if( ( TYPE_SizeCodePostcard_JpSef == c_PaperSize.e_SizeCode )
#ifdef DEF_A4_Cus
        || (TYPE_SizeCodeDoublePostcard_JpSef == c_PaperSize.e_SizeCode)
#endif
        || ( TYPE_SizeCodeIN4x6Sef == c_PaperSize.e_SizeCode )
        || ( TYPE_SizeCodePostcard_EuSef == c_PaperSize.e_SizeCode ) 
        || ( TYPE_SizeCodeEnvelopeB5 == c_PaperSize.e_SizeCode )
#ifdef DEF_A4_Cus
        || (TYPE_SizeCodeJpLetterY2Lef == c_PaperSize.e_SizeCode)
        || (TYPE_SizeCodeEnvelopeDLLef == c_PaperSize.e_SizeCode)
        || (TYPE_SizeCodeEnvelopeMornarchLef == c_PaperSize.e_SizeCode)
        || (TYPE_SizeCodeJpLetterY3Lef == c_PaperSize.e_SizeCode)        
#endif
        || ( TYPE_SizeCodeEnvelopeC5 == c_PaperSize.e_SizeCode )
        || ( TYPE_SizeCodeEnvelopeDL == c_PaperSize.e_SizeCode )
        || ( TYPE_SizeCodeEnvelopeCom10 == c_PaperSize.e_SizeCode )
        || ( TYPE_SizeCodeEnvelopeMornarch == c_PaperSize.e_SizeCode )
        || ( TYPE_SizeCodeEnvelopeC4 == c_PaperSize.e_SizeCode )
        || ( TYPE_SizeCodeJpLetterY2 == c_PaperSize.e_SizeCode )
        || ( TYPE_SizeCodeJpLetterY3 == c_PaperSize.e_SizeCode )
        || ( TYPE_SizeCodeJpLetterY4 == c_PaperSize.e_SizeCode )
        || ( TYPE_SizeCodeJpLetterL3 == c_PaperSize.e_SizeCode )
        || ( TYPE_SizeCodeJpLetterL4 == c_PaperSize.e_SizeCode )
        || ( TYPE_SizeCodeJpLetterK1 == c_PaperSize.e_SizeCode )
        || ( TYPE_SizeCodeJpLetterK2 == c_PaperSize.e_SizeCode )
        || ( TYPE_SizeCodeJpLetterK3 == c_PaperSize.e_SizeCode ) ){
            isBeforePostCardOrEnv = True;
        }

	switch( option ) {
		case BTNID_KEY_A3WSEF:	_e_SizeCode = TYPE_SizeCodeA3Sef;	break;	// 用紙ワイド紙・A3WTの縦のボタン.
//#endif
		case BTNID_KEY_B4WSEF:	_e_SizeCode = TYPE_SizeCodeB4Sef;	break;	// 用紙ワイド紙・B4WTの縦のボタン.
		case BTNID_KEY_A4WSEF:	_e_SizeCode = TYPE_SizeCodeA4Sef;	break;	// 用紙ワイド紙・A4WYの横のボタン.
		case BTNID_KEY_A4WLEF:	_e_SizeCode = TYPE_SizeCodeA4Lef;	break;	// 用紙ワイド紙・A4WTの縦のボタン.
		case BTNID_KEY_A5WLEF:	_e_SizeCode = TYPE_SizeCodeA5Lef;	break;	// 用紙ワイド紙・A5WYの横のボタン.
		case BTNID_KEY_A5WSEF:	_e_SizeCode = TYPE_SizeCodeA5Sef;	break;	// 用紙ワイド紙・A5WTの縦のボタン.
		case BTNID_KEY_B5WSEF:	_e_SizeCode = TYPE_SizeCodeB5Sef;	break;	// 用紙ワイド紙・B5WYの横のボタン.
		case BTNID_KEY_B5WLEF:	_e_SizeCode = TYPE_SizeCodeB5Lef;	break;	// 用紙ワイド紙・B5WTの縦のボタン.
		case BTNID_KEY_11x17WSEF:	_e_SizeCode = TYPE_SizeCodeLedgerSef;	break;	// 用紙ワイド紙・11×17の縦のボタン.
//#endif
		case BTNID_KEY_8Hx11WSEF:	_e_SizeCode = TYPE_SizeCodeLetterSef;	break;	// 用紙ワイド紙・Letterの縦のボタン.
		case BTNID_KEY_8Hx11WLEF:	_e_SizeCode = TYPE_SizeCodeLetterLef;	break;	// 用紙ワイド紙・Letterの横のボタン.
		case BTNID_KEY_5hx8hWSEF:	_e_SizeCode = TYPE_SizeCodeInvoiceSef;	break;	// 用紙ワイド紙・8 1/2×5 1/2の縦のボタン.
		case BTNID_KEY_5hx8hWLEF:	_e_SizeCode = TYPE_SizeCodeInvoiceLef;	break;	// 用紙ワイド紙・8 1/2×5 1/2の横のボタン.

		case BTNID_KEY_AutoDetect:		// 自動検出.
			if( True == PnlComGetFeedTray()->getPaperSizeAutoDetectOn( pc_PaperSizeSelect.getSelectedFeedTray() ) ) {
				setBuzzerType( PCGE_BuzzerType_InputNg );
				break;
			}
			
// Comment 2020/04/18 [HCL_Prasanjeet][IT6_EagLeZ+][OP_BTS-22867] paper type canceletion.
			if(True == isBeforePostCardOrEnv) {
				pc_Print->c_PaperType.setPaperType( TYPE_PaperTypeNormal );
				c_PaperType.setPaperType( TYPE_PaperTypeNormal );
				pc_FeedTray->setPaperType(e_FeedTray, c_PaperType);
			}
			// 用紙サイズの自動検出設定.
			PnlComGetFeedTray()->setPaperSizeAutoDetectOn( pc_PaperSizeSelect.getSelectedFeedTray(), True );
			// APS/AMS処理関数.
			selectAutoPaperMag();
			break;
// comment 2017/06/16 SCD2-Sky後藤 [IT5_4.1][AR1024210] 不定形サイズ可能トレイの印刷出来るサイズ範囲外の用紙を設定できない処理を追加.
		case BTNID_KEY_ChkAutoDetect:  // 自動検出チェックボックス.
			if( True == PnlComGetFeedTray()->getPaperSizeAutoDetectOn( e_FeedTray ) ) {
				Key_ChkAutoDetect.chkOff();
				if( False == IsSelectButtonPaperSize( c_PaperSize )){
					TYPC_PaperSize c_PaperSizeDefault;
					c_PaperSize = c_PaperSizeDefault;
				}else{
					c_PaperSize = PnlComGetOperationSetting()->getWidePaperSize( c_PaperSize.e_SizeCode, e_FeedTray );
				}
				// 用紙サイズの自動検出設定.
				PnlComGetFeedTray()->setPaperSizeAutoDetectOn( e_FeedTray, False );
				// 用紙サイズをジョブに設定.
				pc_Print->c_PaperSize = c_PaperSize;
				onlyPanel.setFeedSizeKind( pc_PaperSizeSelect.getSelectedFeedTray() );	// トレイへの設定.
				// APS/AMS処理関数.
				selectAutoPaperMag();
			} else {
				Key_ChkAutoDetect.chkOn();
				// 用紙サイズの自動検出設定.
				PnlComGetFeedTray()->setPaperSizeAutoDetectOn( e_FeedTray, True );
				// APS/AMS処理関数.
			}

// Comment 2020/04/18 [HCL_Prasanjeet][IT6_EagLeZ+][OP_BTS-22867] paper type canceletion.
			if(True == isBeforePostCardOrEnv) {
				pc_Print->c_PaperType.setPaperType( TYPE_PaperTypeNormal );
				c_PaperType.setPaperType( TYPE_PaperTypeNormal );
				pc_FeedTray->setPaperType(e_FeedTray, c_PaperType);
			}
			break;
		case BTNID_KEY_12x18SEF:
			if( ( False == PnlComGetFeedTray()->getPaperSizeAutoDetectOn( pc_PaperSizeSelect.getSelectedFeedTray() ) )
			    && ( PCGD_1225_18PaperSizeX == c_PaperSize.c_SizeMM.y )
			    && ( PCGD_1225_18PaperSizeY == c_PaperSize.c_SizeMM.x ) ) {
				setBuzzerType( PCGE_BuzzerType_InputNg );
				break;
			}
			c_PaperSize.c_SizeMM.x = PCGD_1225_18PaperSizeX;
			c_PaperSize.c_SizeMM.y = PCGD_1225_18PaperSizeY;
			c_PaperSize.c_SizeMM = c_PaperSize.c_SizeMM.xyReverse();	// パネル表示上でのＸＹのサイズを入れているので、ＸＹを入れ替える.
			// ワイド紙でないときのみ、非定形をセットする.
			if( c_PaperSize.b_WideOn == False ) {
				c_PaperSize.e_SizeCode = TYPE_SizeCodeNonStandard;
			}
			// 用紙サイズをジョブに設定.
			pc_Print->c_PaperSize = c_PaperSize;
			// 自動検出をオフ.
			PnlComGetFeedTray()->setPaperSizeAutoDetectOn( pc_PaperSizeSelect.getSelectedFeedTray(), False );
			// トレイへの設定.
			onlyPanel.setFeedSizeKind( pc_PaperSizeSelect.getSelectedFeedTray() );
			// APS/AMS処理関数.
			selectAutoPaperMag();
			break;
		case BTNID_KEY_SizeChange:
		{
// comment 2017/06/16 SCD2-Sky後藤 [IT5_4.1][AR1024210] 不定形サイズ可能トレイの印刷出来るサイズ範囲外の用紙を設定できない処理を追加.
//			TYPC_PaperSize c_PaperSize = pc_PaperSizeSelect.getPaperSize();
//			const short ss_SelectMt = getButtonIndexPaperSizeMetric( c_PaperSize );
//			const short ss_SelectIn = getButtonIndexPaperSizeInch( c_PaperSize );
//			if( ( 0<=ss_SelectMt )				// ワイドメトリック定型検出.
//			    || ( 0<=ss_SelectIn )				// ワイドインチ定型検出.
//			  ) {
//				Key_SizeChange.getPosition( x, y );
//				Key_SizeChange.getSize( w, h );
//				PNLC_PopupListControl::newInstance()->addPopup( &gp_A020114150_4, this, NULL, x, y, w, h );	//A020114150_用紙ワイド紙/サイズ変更(A系･B系).
//			} else {
//				setBuzzerType( PCGE_BuzzerType_InputNg );
//			}
			// 不定形可能トレイの場合.
			if( False == PnlComIsUnfixedPaperSizeTray( e_FeedTray ) ) {
				TYPC_PaperSize c_PaperSize = pc_PaperSizeSelect.getPaperSize();
				const short ss_SelectMt = getButtonIndexPaperSizeMetric( c_PaperSize );
				const short ss_SelectIn = getButtonIndexPaperSizeInch( c_PaperSize );
				if( ( 0<=ss_SelectMt )				// ワイドメトリック定型検出.
			  	  || ( 0<=ss_SelectIn )				// ワイドインチ定型検出.
			 	 ) {
					Key_SizeChange.getPosition( x, y );
					Key_SizeChange.getSize( w, h );
					PNLC_PopupListControl::newInstance()->addPopup( &gp_A020114150_4, this, NULL, x, y, w, h );	//A020114150_用紙ワイド紙/サイズ変更(A系･B系).
				} else {
					setBuzzerType( PCGE_BuzzerType_InputNg );
				}
			}else{
				if ( TYPE_FeedTrayManual != e_FeedTray ){
					c_PaperSize.b_WideOn = True;
					c_PaperSize = PnlComGetOperationSetting()->getWidePaperSize( c_PaperSize.e_SizeCode, e_FeedTray );
					// 用紙サイズをジョブに設定.
					pc_Print->c_PaperSize = c_PaperSize;
					onlyPanel.setFeedSizeKind( pc_PaperSizeSelect.getSelectedFeedTray() );	// トレイへの設定.
					// APS/AMS処理関数.
					selectAutoPaperMag();
				}
				Key_SizeChange.getPosition( x, y );
				Key_SizeChange.getSize( w, h );
				PNLC_PopupListControl::newInstance()->addPopup( &gp_A020114150_4, this, NULL, x, y, w, h );	//A020114150_用紙ワイド紙/サイズ変更(A系･B系).
			}
			return;
		}
		default: LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return;
	}
	if( _e_SizeCode != TYPE_SizeCodeUnknown ) {
// Comment 2020/04/18 [HCL_Prasanjeet][IT6_EagLeZ+][OP_BTS-22867] paper type canceletion.
		if(True == isBeforePostCardOrEnv
		 && False == PnlComGetFeedTray()->getPaperSizeAutoDetectOn( pc_PaperSizeSelect.getSelectedFeedTray() )){
			// 用紙種類を封筒に設定.
		  	pc_Print->c_PaperType.setPaperType( TYPE_PaperTypeNormal );
		 	c_PaperType.setPaperType( TYPE_PaperTypeNormal );
			pc_FeedTray->setPaperType(e_FeedTray, c_PaperType);
	    } 
		
		c_PaperSize.e_SizeCode = _e_SizeCode;
		c_PaperSize.b_WideOn = True;
// comment 2017/06/16 SCD2-Sky後藤 [IT5_4.1][AR1024210] 不定形サイズ可能トレイの印刷出来るサイズ範囲外の用紙を設定できない処理を追加.
//// comment 2016/10/28 SCD2-2G西原 [IT5_4.0][RQ-226] 不定型紙の標準カセット対応.
////		if( TYPE_FeedTrayManual == pc_PaperSizeSelect.getSelectedFeedTray() ) {
//		TYPE_FeedTray e_FeedTray = pc_PaperSizeSelect.getSelectedFeedTray();
//		if( True == PnlComIsUnfixedPaperSizeTray( e_FeedTray ) ) {
//
//			// 自動検出をオフ.
////			PnlComGetFeedTray()->setPaperSizeAutoDetectOn( pc_PaperSizeSelect.getSelectedFeedTray(), False );
//			PnlComGetFeedTray()->setPaperSizeAutoDetectOn( e_FeedTray, False );
////		}
//// comment 2016/10/28 SCD2-2G西原 [IT5_4.0][RQ-226] 不定型紙の標準カセット対応.
////		// 手差しトレイの場合.
////		if( PnlComGetJobCopy()->getJobSpec()->pc_Print->c_PaperTray.getFeedTray() == TYPE_FeedTrayManual ) {
////			c_PaperSize = PnlComGetOperationSetting()->getWidePaperSize( c_PaperSize.e_SizeCode, TYPE_FeedTrayManual );
////		}
//		// 不定形可能トレイの場合.
//		if( PnlComIsUnfixedPaperSizeTray( e_FeedTray ) ) {
//			c_PaperSize = PnlComGetOperationSetting()->getWidePaperSize( c_PaperSize.e_SizeCode, e_FeedTray );
//		}
		if( TYPE_FeedTrayManual == pc_PaperSizeSelect.getSelectedFeedTray() ) {
			PnlComGetFeedTray()->setPaperSizeAutoDetectOn( pc_PaperSizeSelect.getSelectedFeedTray(), False );
		}
		if( PnlComGetJobCopy()->getJobSpec()->pc_Print->c_PaperTray.getFeedTray() == TYPE_FeedTrayManual ) {
			c_PaperSize = PnlComGetOperationSetting()->getWidePaperSize( c_PaperSize.e_SizeCode, e_FeedTray );
		}else{
			TYPE_FeedTray e_FeedTray = pc_PaperSizeSelect.getSelectedFeedTray();
			// 不定形可能トレイの場合.
			if( PnlComIsUnfixedPaperSizeTray( e_FeedTray ) ) {
				if( ( False == PnlComGetFeedTray()->getPaperSizeAutoDetectOn( pc_PaperSizeSelect.getSelectedFeedTray() ) )
				&&  ( PCGD_1225_18PaperSizeX != c_PaperSize.c_SizeMM.y ) 
				&&  ( PCGD_1225_18PaperSizeY != c_PaperSize.c_SizeMM.x )
				&&	( PCGD_12_18PaperSizeX   != c_PaperSize.c_SizeMM.x )
				&&	( PCGD_12_18PaperSizeY   != c_PaperSize.c_SizeMM.y ) ){
					c_PaperSize = PnlComGetOperationSetting()->getWidePaperSize( c_PaperSize.e_SizeCode, e_FeedTray );
// comment 2017/08/10 SCD2-Sky後藤 [IT5_4.1][AR1027418] サイズ用紙を一回押すと、全部グレーアウトされる.
					float sf_MinValueX = 0.0f;
					float sf_MinValueY = 0.0f;
					// xyReverseしていないので、XYの値を逆にする.
					if ( TYPE_FeedTray1 == e_FeedTray ) {				//トレイ1の場合.
						sf_MinValueX = PCGD_MinPaperSizeYMPCTray1;		//Xの最小値の設定.
						sf_MinValueY = PCGD_MinPaperSizeXMPCTray1;		//Yの最小値の設定.
					} else if ( TYPE_FeedTray2 == e_FeedTray ) {		//トレイ2の場合.
						sf_MinValueX = PCGD_MinPaperSizeYMPCTray2;		//Xの最小値の設定.
						sf_MinValueY = PCGD_MinPaperSizeXMPCTray2;		//Yの最小値の設定.
					} else {											//手差しトレイの場合.
						sf_MinValueX = PCGD_MinPaperSizeYMPC;			//Xの最小値の設定.
						sf_MinValueY = PCGD_MinPaperSizeXMPC;			//Yの最小値の設定.
					}
					if( c_PaperSize.c_SizeMM.x < sf_MinValueX ){
						c_PaperSize.c_SizeMM.x = sf_MinValueX;
					}
					if( c_PaperSize.c_SizeMM.y < sf_MinValueY ){
						c_PaperSize.c_SizeMM.y = sf_MinValueY;
					}
				}
// comment 2017/08/10 SCD2-Sky後藤 [IT5_4.1][AR1027418] サイズ用紙を一回押すと、全部グレーアウトされる.
//// comment 2017/05/29 SCD2-Sky後藤 [IT5_4.0][1022598] 不定形サイズ可能トレイの印刷出来るサイズ範囲外の用紙を設定できない処理を追加.
//				float sf_MinValueX = 0.0f;
//				float sf_MinValueY = 0.0f;
//				// xyReverseしていないので、XYの値を逆にする.
//				if ( TYPE_FeedTray1 == e_FeedTray ) {				//トレイ1の場合.
//					sf_MinValueX = PCGD_MinPaperSizeYMPCTray1;		//Xの最小値の設定.
//					sf_MinValueY = PCGD_MinPaperSizeXMPCTray1;		//Yの最小値の設定.
//				} else if ( TYPE_FeedTray2 == e_FeedTray ) {		//トレイ2の場合.
//					sf_MinValueX = PCGD_MinPaperSizeYMPCTray2;		//Xの最小値の設定.
//					sf_MinValueY = PCGD_MinPaperSizeXMPCTray2;		//Yの最小値の設定.
//				} else {											//手差しトレイの場合.
//					sf_MinValueX = PCGD_MinPaperSizeYMPC;			//Xの最小値の設定.
//					sf_MinValueY = PCGD_MinPaperSizeXMPC;			//Yの最小値の設定.
//				}
//				if( c_PaperSize.c_SizeMM.x < sf_MinValueX ){
//					c_PaperSize.c_SizeMM.x = sf_MinValueX;
//				}
//				if( c_PaperSize.c_SizeMM.y < sf_MinValueY ){
//					c_PaperSize.c_SizeMM.y = sf_MinValueY;
//				}
			}
		}	
		// 用紙サイズをジョブに設定.
		pc_Print->c_PaperSize = c_PaperSize;

		onlyPanel.setFeedSizeKind( pc_PaperSizeSelect.getSelectedFeedTray() );	// トレイへの設定.
		selectAutoPaperMag();	// APS/AMS処理関数.
	}
	setupGamen();
}
//-----------------------------------------------------------------------------------------------
/**
 *	@brief	A020114130_用紙ワイド紙 : 画面セットアップ.
 */
//-----------------------------------------------------------------------------------------------
void PCPC_PaperSizeWideInch_copy::setupGamen( short option )
{
	
// comment 2017/06/16 SCD2-Sky後藤 [IT5_4.1][AR1024210] 不定形サイズ可能トレイの印刷出来るサイズ範囲外の用紙を設定できない処理を追加.
	// メッセージの変更.
	changeMessage();

	PPopupDecisionButtonCom::setupGamen( option );	// 親クラスのsetupGamen.

// comment 2017/06/16 SCD2-Sky後藤 [IT5_4.1][AR1024210] 不定形サイズ可能トレイの印刷出来るサイズ範囲外の用紙を設定できない処理を追加.
	
	// タイトル表示.
	setupTitle();
	
	TYPE_FeedTray e_FeedTray = pc_PaperSizeSelect.getSelectedFeedTray();
	
	// ワイド定型サイズ検索.
	// タブの表示切替.
	if( m_SelectTabKeyOption == PNLD_TabNumABSize ) {
		setupGamenMetric();
	} else {
		setupGamenInch();
	}
	// 表示サイズセット.
	TYPC_PaperSize c_FeedPaperSize = pc_PaperSizeSelect.getPaperSize();

	if( ( NULL == PnlComGetJobCopy() ) ||
	    ( NULL == PnlComGetJobCopy()->getJobSpec() ) ||
	    ( NULL == PnlComGetJobCopy()->getJobSpec()->pc_Print ) ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "[PCPC_PaperSizeWideInch_copy::setupGamen]Assert Failed!" );
		return;
	}
	//表示サイズの切り替えと表示.
	if( PnlComGetJobCopy()->getJobSpec()->pc_Print->c_PaperTray.getFeedTray() == TYPE_FeedTrayManual ) {
		// 手差しトレイの場合.
		// サイズコードによる変換処理は、用紙サイズ変更時に行う.
	} else {
		c_FeedPaperSize.MMtoSizeCode();
	}
	c_FeedPaperSize.b_WideOn = False;
// comment 2017/06/16 SCD2-Sky後藤 [IT5_4.1][AR1024210] 不定形サイズ可能トレイの印刷出来るサイズ範囲外の用紙を設定できない処理を追加.
	if( ( True == PnlComIsUnfixedPaperSizeTray( e_FeedTray ) )		// 不定形サイズ可能トレイの場合.
	&&  ( TYPE_FeedTrayManual != e_FeedTray ) ){					// 手差しトレイ以外の場合.
		if( ( TYPE_SizeCodeUnknown == c_FeedPaperSize.e_SizeCode ) 
		&&  ( True == PnlComGetFeedTray()->getPaperSizeAutoDetectOn( e_FeedTray ) ) ) {
			printLEDControl( False );
		}
	}
	setSize( c_FeedPaperSize );

#ifndef DEF_A4_Cus
// Comment 2019/07/26 [HCLT_Paul] [IT6_HeliosMLK] [ Seigyu.10 ] Adding the HeliosMLK compiler switch.
#if !defined(DEF_Zeus5BkCus) && !defined(DEF_HeliosMLKCus)
	short ss_Y_ChangeSize_Regular = 144;				// 通常時のサイズ変更キーの座標(画面内の相対座標).
	short ss_Y_ChangeSize_UnfixedTray = 224;			// 不定形サイズ可能時のサイズ変更キーの座標(画面内の相対座標).
#else	//	#if !defined(DEF_Zeus5BkCus) && !defined(DEF_HeliosCus).
	short ss_Y_ChangeSize_Regular = 148;				// 通常時のサイズ変更キーの座標(画面内の相対座標).
	short ss_Y_ChangeSize_UnfixedTray = 228;			// 不定形サイズ可能時のサイズ変更キーの座標(画面内の相対座標).
#endif	//	#if !defined(DEF_Zeus5BkCus) && !defined(DEF_HeliosCus).
	short ss_X_ChangeSize = 0;
	short ss_Y_ChangeSize = 0;
	short ss_X_Screen = 0;
	short ss_Y_Screen = 0;
	this->getPosition( &ss_X_Screen, &ss_Y_Screen );					// 画面の座標を取得.
	Key_SizeChange.getPosition( ss_X_ChangeSize, ss_Y_ChangeSize );		// サイズ変更キーの座標を取得.
#endif	//#ifndef DEF_A4_Cus.

	if( TYPE_FeedTrayManual == pc_PaperSizeSelect.getSelectedFeedTray() ) {
		//手差しトレイ選択時「サイズ変更キー」表示.
		Rbt_WidePaperSetPaperSize.showEnable( False );
		Key_SizeChange.showEnable( True );
// comment 2017/06/16 SCD2-Sky後藤 [IT5_4.1][AR1024210] 不定形サイズ可能トレイの印刷出来るサイズ範囲外の用紙を設定できない処理を追加.
		//手差しトレイ選択時「自動検出チェックキー」非表示.
		Key_ChkAutoDetect.showEnable( False );
		TYPC_PaperSize c_PaperSize = pc_PaperSizeSelect.getPaperSize();
		if( True == IsSelectButtonPaperSize( c_PaperSize ) ){	  
			Key_SizeChange.activate( True );
		} else {
			Key_SizeChange.activate( False );
		}
#ifndef DEF_A4_Cus
		Key_SizeChange.changePosition( ss_X_ChangeSize, short( ss_Y_Screen + ss_Y_ChangeSize_Regular ) );
#endif	//#ifndef DEF_A4_Cus.
	} else {
// comment 2016/10/17 SCD2-2G南波 [IT5_4.0][RQ-226] 不定型紙の標準カセット対応.
//		Key_SizeChange.showEnable( False );
//		Rbt_WidePaperSetPaperSize.showEnable( True );
////			//2段目以外のときは12.25x18のキー非表示.
//////// 2014/08/17 【HCL-Shobana】 [Poseidon] 【制御構成変更_No.61】Deletion of parts.
////#ifndef DEF_Zeus5BkCus
//#if !defined(DEF_Zeus5BkCus) && !defined(DEF_HeliosCus)
//		if( TYPE_FeedTray1 != pc_PaperSizeSelect.getSelectedFeedTray() &&
//		    TYPE_FeedTray2 != pc_PaperSizeSelect.getSelectedFeedTray() ) {
//			//1、2段目以外のときは12.25x18のキー非表示.
//			Key_12x18SEF.showEnable( False );
//		}
//		Rbt_WidePaperSetPaperSize.select( PnlComGetFeedTray()->getPaperSizeAutoDetectOn( pc_PaperSizeSelect.getSelectedFeedTray() ) == True ? 0 : 1 );
//		// 12.25x18のキーは、12×18サイズ検出時のみ表示する.
//		if( ( PnlComGetFeedTray()->getPaperSize( pc_PaperSizeSelect.getSelectedFeedTray() ).c_SizeMM.x == PCGD_12_18PaperSizeY
//		      && PnlComGetFeedTray()->getPaperSize( pc_PaperSizeSelect.getSelectedFeedTray() ).c_SizeMM.y == PCGD_12_18PaperSizeX )
//		    || ( PnlComGetFeedTray()->getPaperSize( pc_PaperSizeSelect.getSelectedFeedTray() ).c_SizeMM.x == PCGD_1225_18PaperSizeY
//		         && PnlComGetFeedTray()->getPaperSize( pc_PaperSizeSelect.getSelectedFeedTray() ).c_SizeMM.y == PCGD_1225_18PaperSizeX ) ) {
//			Key_12x18SEF.activate( True );
//		} else {
//			Key_12x18SEF.activate( False );
//		}
////#endif
//// comment 2015/1/7 OPS1 Sky江波戸 [Poseidon][CQ508319] Poseidonでも選択表示状態の制御は必要.
//#else
//		Rbt_WidePaperSetPaperSize.select( PnlComGetFeedTray()->getPaperSizeAutoDetectOn(pc_PaperSizeSelect.getSelectedFeedTray()) == True ? 0 : 1 );
//#endif	//	#if !defined(DEF_Zeus5BkCus) && !defined(DEF_HeliosCus).
#ifndef DEF_A4_Cus
		//サイズ変更キーの表示.
		TYPE_FeedTray e_FeedTray = PnlComGetJobCopy()->getJobSpec()->pc_Print->c_PaperTray.getFeedTray();
		// 不定形サイズ可能トレイの場合は手差しトレイ以外でもサイズ変更キーを表示.
		if( PnlComIsUnfixedPaperSizeTray( e_FeedTray ) ){
			Key_SizeChange.showEnable( True );
			Key_SizeChange.changePosition( ss_X_ChangeSize, short( ss_Y_Screen + ss_Y_ChangeSize_UnfixedTray ) );
// comment 2017/06/16 SCD2-Sky後藤 [IT5_4.1][AR1024210] 不定形サイズ可能トレイの印刷出来るサイズ範囲外の用紙を設定できない処理を追加.
			TYPC_PaperSize c_PaperSize = pc_PaperSizeSelect.getPaperSize();
			if( True == IsSelectButtonPaperSize( c_PaperSize ) ){	  
				Key_SizeChange.activate( True );
			} else {
				Key_SizeChange.activate( False );
			}
		} else {
			Key_SizeChange.showEnable( False );
			Key_SizeChange.changePosition( ss_X_ChangeSize, short( ss_Y_Screen + ss_Y_ChangeSize_Regular ) );
		}
		Rbt_WidePaperSetPaperSize.showEnable( True );
// Comment 2019/07/26 [HCLT_Paul] [IT6_HeliosMLK] [ Seigyu.10 ] Adding the HeliosMLK compiler switch.
#if !defined(DEF_Zeus5BkCus) && !defined(DEF_HeliosMLKCus)
// comment 2017/06/16 SCD2-Sky後藤 [IT5_4.1][AR1024210] 不定形サイズ可能トレイの印刷出来るサイズ範囲外の用紙を設定できない処理を追加.
#if defined( DEF_ZeusCus )
// Comment 2021/06/01 【FPT-LinhNV46】 【MinervaSSBK】 【機器構成変更_No.21-12】 Add 12.25x18 paper size of Tray 1 for MinervaSSBK.
#ifdef DEF_MinervaSSBKCus
		if( TYPE_FeedTray1 != pc_PaperSizeSelect.getSelectedFeedTray() &&
		    TYPE_FeedTray2 != pc_PaperSizeSelect.getSelectedFeedTray() ) {
			//1、2段目以外のときは12.25x18のキー非表示.
			Key_12x18SEF.showEnable( False );
		}
#else //DEF_MinervaSSBKCus.
		if( TYPE_FeedTray2 != pc_PaperSizeSelect.getSelectedFeedTray() ) {
			//2段目以外のときは12.25x18のキー非表示.
			Key_12x18SEF.showEnable( False );
		}
#endif //DEF_MinervaSSBKCus.
// comment 2017/06/16 SCD2-Sky後藤 [IT5_4.1][AR1024210] 不定形サイズ可能トレイの印刷出来るサイズ範囲外の用紙を設定できない処理を追加.
#endif		
		// 不定形サイズ可能トレイの場合、12_1/4x18キーを選択状態にしない.
		Bool bl_AutoDetect = PnlComGetFeedTray()->getPaperSizeAutoDetectOn( pc_PaperSizeSelect.getSelectedFeedTray() );
		if( PnlComIsUnfixedPaperSizeTray( e_FeedTray ) ){
// comment 2017/06/16 SCD2-Sky後藤 [IT5_4.1][AR1024210] 不定形サイズ可能トレイの印刷出来るサイズ範囲外の用紙を設定できない処理を追加.
//			Rbt_WidePaperSetPaperSize.select( True == bl_AutoDetect ? 0 : -1 );
			Rbt_WidePaperSetPaperSize.select( True == bl_AutoDetect ? 0 : 1 );
			// 不定形サイズ可能トレイの場合、自動検出キーをチェックボックスにする.
			Key_AutoDetect.showEnable( False );
			Key_ChkAutoDetect.showEnable( True );
			if( bl_AutoDetect ){
				Key_ChkAutoDetect.chkOn();
			} else {
				Key_ChkAutoDetect.chkOff();
			}
		} else {
// comment 2017/06/16 SCD2-Sky後藤 [IT5_4.1][AR1024210] 不定形サイズ可能トレイの印刷出来るサイズ範囲外の用紙を設定できない処理を追加.
			Key_AutoDetect.showEnable( True );
			Key_ChkAutoDetect.showEnable( False );
			Rbt_WidePaperSetPaperSize.select( True == bl_AutoDetect ? 0 : 1 );
		}
		// 12.25x18のキーは、12×18サイズ検出時のみ表示する.
		if( ( PnlComGetFeedTray()->getPaperSize( pc_PaperSizeSelect.getSelectedFeedTray() ).c_SizeMM.x == PCGD_12_18PaperSizeY
		      && PnlComGetFeedTray()->getPaperSize( pc_PaperSizeSelect.getSelectedFeedTray() ).c_SizeMM.y == PCGD_12_18PaperSizeX )
	    	|| ( PnlComGetFeedTray()->getPaperSize( pc_PaperSizeSelect.getSelectedFeedTray() ).c_SizeMM.x == PCGD_1225_18PaperSizeY
		         && PnlComGetFeedTray()->getPaperSize( pc_PaperSizeSelect.getSelectedFeedTray() ).c_SizeMM.y == PCGD_1225_18PaperSizeX ) ) {
			Key_12x18SEF.activate( True );
		} else {
			Key_12x18SEF.activate( False );
		}
#else
// comment 2017/06/16 SCD2-Sky後藤 [IT5_4.1][AR1024210] 不定形サイズ可能トレイの印刷出来るサイズ範囲外の用紙を設定できない処理を追加.
		if( PnlComIsUnfixedPaperSizeTray( e_FeedTray ) ){
			// 不定形サイズ可能トレイの場合、自動検出キーをチェックボックスにする.
			Key_AutoDetect.showEnable( False );
			Key_ChkAutoDetect.showEnable( True );
			if( True == PnlComGetFeedTray()->getPaperSizeAutoDetectOn(pc_PaperSizeSelect.getSelectedFeedTray()) ){
				Key_ChkAutoDetect.chkOn();
			} else {
				Key_ChkAutoDetect.chkOff();
			}
		}else{
		Rbt_WidePaperSetPaperSize.select( PnlComGetFeedTray()->getPaperSizeAutoDetectOn(pc_PaperSizeSelect.getSelectedFeedTray()) == True ? 0 : 1 );
			Key_AutoDetect.showEnable( True );
			Key_ChkAutoDetect.showEnable( False );
		}
#endif	//	#if !defined(DEF_Zeus5BkCus) && !defined(DEF_HeliosCus).
#else	//#ifndef DEF_A4_Cus.
		Key_SizeChange.showEnable( False );
		Rbt_WidePaperSetPaperSize.showEnable( True );
//			//2段目以外のときは12.25x18のキー非表示.
////// 2014/08/17 【HCL-Shobana】 [Poseidon] 【制御構成変更_No.61】Deletion of parts.
//#ifndef DEF_Zeus5BkCus
// Comment 2019/07/26 [HCLT_Paul] [IT6_HeliosMLK] [ Seigyu.10 ] Adding the HeliosMLK compiler switch.
#if !defined(DEF_Zeus5BkCus) && !defined(DEF_HeliosMLKCus)
		if( TYPE_FeedTray1 != pc_PaperSizeSelect.getSelectedFeedTray() &&
		    TYPE_FeedTray2 != pc_PaperSizeSelect.getSelectedFeedTray() ) {
			//1、2段目以外のときは12.25x18のキー非表示.
			Key_12x18SEF.showEnable( False );
		}
		Rbt_WidePaperSetPaperSize.select( PnlComGetFeedTray()->getPaperSizeAutoDetectOn( pc_PaperSizeSelect.getSelectedFeedTray() ) == True ? 0 : 1 );
		// 12.25x18のキーは、12×18サイズ検出時のみ表示する.
		if( ( PnlComGetFeedTray()->getPaperSize( pc_PaperSizeSelect.getSelectedFeedTray() ).c_SizeMM.x == PCGD_12_18PaperSizeY
		      && PnlComGetFeedTray()->getPaperSize( pc_PaperSizeSelect.getSelectedFeedTray() ).c_SizeMM.y == PCGD_12_18PaperSizeX )
		    || ( PnlComGetFeedTray()->getPaperSize( pc_PaperSizeSelect.getSelectedFeedTray() ).c_SizeMM.x == PCGD_1225_18PaperSizeY
		         && PnlComGetFeedTray()->getPaperSize( pc_PaperSizeSelect.getSelectedFeedTray() ).c_SizeMM.y == PCGD_1225_18PaperSizeX ) ) {
			Key_12x18SEF.activate( True );
		} else {
			Key_12x18SEF.activate( False );
		}
//#endif
// comment 2015/1/7 OPS1 Sky江波戸 [Poseidon][CQ508319] Poseidonでも選択表示状態の制御は必要.
#else
		Rbt_WidePaperSetPaperSize.select( PnlComGetFeedTray()->getPaperSizeAutoDetectOn(pc_PaperSizeSelect.getSelectedFeedTray()) == True ? 0 : 1 );
#endif	//	#if !defined(DEF_Zeus5BkCus) && !defined(DEF_HeliosCus).
#endif	//#ifndef DEF_A4_Cus.
	}
// Comment 2019/10/14 [HCLT_Shalini] [IT6_HeliosMLK] [ 機器構成変更_No12-4 ] Hiding A3WSEF Paper Size for HeliosMLK.
#if defined(DEF_HeliosMLKCus)
//Comment 2020/03/12 [HCLT_Santhiya] [IT6_HeliosMLK] [ OP_BTS- 21302 ] Hide A3Wide and 11X17 wide paper size for HeliosMLK.
	//	if( TYPE_FeedTrayManual == pc_PaperSizeSelect.getSelectedFeedTray() ) {
	Key_SheetSizeA3WSEF.showEnable( False );
	Key_SheetSize11x17WSEF.showEnable( False );
//	}
#endif
// comment 2016/09/07 SCD2-Sky後藤 [IT5_3.3][1003602]トレイに関係なく非表示となるため削除.
////	comment 2015/01/08 HCLT_Naveen [HeliosL][機器構成変更_No.13] Support to hide A3WSEF and 11x17WSEF Keys for Tray 1 in HeliosL Model.
////#ifdef DEF_Zeus5BkCus
//#if defined(DEF_Zeus5BkCus) || defined(DEF_HeliosCus)
//	if( TYPE_FeedTray1 == pc_PaperSizeSelect.getSelectedFeedTray() ) {
//		Key_SheetSizeA3WSEF.showEnable( False );
//		Key_SheetSize11x17WSEF.showEnable( False );
//	}
//	else {
//		Key_SheetSizeA3WSEF.showEnable( True );
//		Key_SheetSize11x17WSEF.showEnable( True );
//	}
//#endif	//	#if defined(DEF_Zeus5BkCus) || defined(DEF_HeliosCus).
}

//-----------------------------------------------------------------------------------------------
/**
 *	@brief	A020114130_用紙ワイド紙 : Ａ・Ｂ系setupGamen.
 */
//-----------------------------------------------------------------------------------------------
void PCPC_PaperSizeWideInch_copy::setupGamenMetric()
{
	// ワイド定型サイズ検索.
	TYPC_PaperSize c_PaperSize = pc_PaperSizeSelect.getPaperSize();
	c_PaperSize.c_SizeMM = c_PaperSize.c_SizeMM.xyReverse();
	TYPC_PaperSize c_DispKeyPaperSize;
	short ss_Select = -1;

	if( ( NULL == PnlComGetJobCopy() ) ||
	    ( NULL == PnlComGetJobCopy()->getJobSpec() ) ||
	    ( NULL == PnlComGetJobCopy()->getJobSpec()->pc_Print ) ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "[PCPC_PaperSizeWideInch_copy::setupGamenMetric]Assert Failed!" );
		return;
	}
// comment 2016/10/31 SCD2-2G西原 [IT5_4.0][RQ-226] 不定型紙の標準カセット対応.
//	if( PnlComGetJobCopy()->getJobSpec()->pc_Print->c_PaperTray.getFeedTray()!= TYPE_FeedTrayManual ) {
	TYPE_FeedTray e_FeedTray = PnlComGetJobCopy()->getJobSpec()->pc_Print->c_PaperTray.getFeedTray();
	if( False == PnlComIsUnfixedPaperSizeTray( e_FeedTray ) ) {
		// トレイにセットされている用紙サイズより大きいサイズのキーはグレイアウトする.
		for( uchar j = 0; j < uc_PaperSizeSelectTableWideMetric; j++ ) {
			c_DispKeyPaperSize.e_SizeCode = st_PaperSizeSelectTableWideMetric[j].e_SizeCode;
			c_DispKeyPaperSize.SizeCodeToMM();
			// 方向の異なるキーもグレイアウトに変更 .
			if( c_DispKeyPaperSize.c_SizeMM.x > c_PaperSize.c_SizeMM.y || c_DispKeyPaperSize.c_SizeMM.y > c_PaperSize.c_SizeMM.x
			    || ( st_PaperSizeSelectTableWideMetric[j].b_isTate == c_PaperSize.isTate() ) ) {	// c_DispKeyPaperSizeは、xyReverseしていないので、論理を逆にする.
				st_PaperSizeSelectTableWideMetric[j].p_WidePaperKey->activate( False );
			} else {
				st_PaperSizeSelectTableWideMetric[j].p_WidePaperKey->activate( True );
			}
		}
// comment 2017/06/16 SCD2-Sky後藤 [IT5_4.1][AR1024210] 不定形サイズ可能トレイの印刷出来るサイズ範囲外の用紙を設定できない処理を追加.
	} else {	//不定形サイズ可能トレイの場合
		// トレイにセットされている用紙サイズより大きいサイズのキーはグレイアウトする.
		for( uchar j = 0; j < uc_PaperSizeSelectTableWideMetric; j++ ) {
			c_DispKeyPaperSize.e_SizeCode = st_PaperSizeSelectTableWideMetric[j].e_SizeCode;
			c_DispKeyPaperSize.SizeCodeToMM();
			if( TYPE_FeedTrayManual == e_FeedTray ){
				st_PaperSizeSelectTableWideMetric[j].p_WidePaperKey->activate( True );
			} else {
				if( True == PnlComGetFeedTray()->getPaperSizeAutoDetectOn( pc_PaperSizeSelect.getSelectedFeedTray() ) ) {	//自動検出がオンの場合
					// 方向の異なるキーもグレイアウトに変更 .
					TYPC_PaperSize c_UnknownPaperSize;
					c_UnknownPaperSize.e_SizeCode = TYPE_SizeCodeUnknown;
					c_UnknownPaperSize.SizeCodeToMM();
					if( ( c_UnknownPaperSize.c_SizeMM.x == c_PaperSize.c_SizeMM.y ) && ( c_UnknownPaperSize.c_SizeMM.y == c_PaperSize.c_SizeMM.x ) ){
						st_PaperSizeSelectTableWideMetric[j].p_WidePaperKey->activate( True );
					} else if( c_DispKeyPaperSize.c_SizeMM.x > c_PaperSize.c_SizeMM.y || c_DispKeyPaperSize.c_SizeMM.y > c_PaperSize.c_SizeMM.x
				  	  || ( st_PaperSizeSelectTableWideMetric[j].b_isTate == c_PaperSize.isTate() ) ) {	// c_DispKeyPaperSizeは、xyReverseしていないので、論理を逆にする.
						st_PaperSizeSelectTableWideMetric[j].p_WidePaperKey->activate( False );
					} else {
						st_PaperSizeSelectTableWideMetric[j].p_WidePaperKey->activate( True );
					}
				} else {	//自動検出がオフの場合
					st_PaperSizeSelectTableWideMetric[j].p_WidePaperKey->activate( True );
				}
			}
		}
	}
	ss_Select = getButtonIndexPaperSizeMetric( c_PaperSize );
	Rbt_WideInchSize.showEnable( False );
	Rbt_WideABSize.showEnable( True );
	Rbt_WideABSize.select( ss_Select );

}

//-----------------------------------------------------------------------------------------------
/**
 *	@brief	A020114130_用紙ワイド紙 : インチ系setupGamen.
 */
//-----------------------------------------------------------------------------------------------
void PCPC_PaperSizeWideInch_copy::setupGamenInch()
{
	// ワイド定型サイズ検索.
	TYPC_PaperSize c_PaperSize = pc_PaperSizeSelect.getPaperSize();
	c_PaperSize.c_SizeMM = c_PaperSize.c_SizeMM.xyReverse();
	short ss_Select = -1;

	TYPC_PaperSize c_DispKeyPaperSize;
	if( ( NULL == PnlComGetJobCopy() ) ||
	    ( NULL == PnlComGetJobCopy()->getJobSpec() ) ||
	    ( NULL == PnlComGetJobCopy()->getJobSpec()->pc_Print ) ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "[PCPC_PaperSizeWideInch_copy::setupGamenInch]Assert Failed!" );
		return;
	}
// comment 2016/10/31 SCD2-2G西原 [IT5_4.0][RQ-226] 不定型紙の標準カセット対応.
//	if( PnlComGetJobCopy()->getJobSpec()->pc_Print->c_PaperTray.getFeedTray()!= TYPE_FeedTrayManual ) {
	TYPE_FeedTray e_FeedTray = PnlComGetJobCopy()->getJobSpec()->pc_Print->c_PaperTray.getFeedTray();
	if( False == PnlComIsUnfixedPaperSizeTray( e_FeedTray ) ) {
		// トレイにセットされている用紙サイズより大きいサイズのキーはグレイアウトする.
		for( uchar j = 0; j < uc_PaperSizeSelectTableWideInch; j++ ) {
			c_DispKeyPaperSize.e_SizeCode = st_PaperSizeSelectTableWideInch[j].e_SizeCode;
			c_DispKeyPaperSize.SizeCodeToMM();
			// 方向の異なるキーもグレイアウトに変更 .
			if( c_DispKeyPaperSize.c_SizeMM.x > c_PaperSize.c_SizeMM.y || c_DispKeyPaperSize.c_SizeMM.y > c_PaperSize.c_SizeMM.x
			    || ( st_PaperSizeSelectTableWideInch[j].b_isTate == c_PaperSize.isTate() ) ) {	// c_DispKeyPaperSizeは、xyReverseしていないので、論理を逆にする.
				st_PaperSizeSelectTableWideInch[j].p_WidePaperKey->activate( False );
			} else {
				st_PaperSizeSelectTableWideInch[j].p_WidePaperKey->activate( True );
			}
		}
// comment 2017/06/16 SCD2-Sky後藤 [IT5_4.1][AR1024210] 不定形サイズ可能トレイの印刷出来るサイズ範囲外の用紙を設定できない処理を追加.
	} else {	//不定形サイズ可能トレイの場合
		// トレイにセットされている用紙サイズより大きいサイズのキーはグレイアウトする.
		for( uchar j = 0; j < uc_PaperSizeSelectTableWideInch; j++ ) {
			c_DispKeyPaperSize.e_SizeCode = st_PaperSizeSelectTableWideInch[j].e_SizeCode;
			c_DispKeyPaperSize.SizeCodeToMM();
			if( TYPE_FeedTrayManual == e_FeedTray ){
				st_PaperSizeSelectTableWideInch[j].p_WidePaperKey->activate( True );
			} else {
				if( True == PnlComGetFeedTray()->getPaperSizeAutoDetectOn( pc_PaperSizeSelect.getSelectedFeedTray() ) ) {	//自動検出がオンの場合
					// 方向の異なるキーもグレイアウトに変更 .
					TYPC_PaperSize c_UnknownPaperSize;
					c_UnknownPaperSize.e_SizeCode = TYPE_SizeCodeUnknown;
					c_UnknownPaperSize.SizeCodeToMM();
					if( ( c_UnknownPaperSize.c_SizeMM.x == c_PaperSize.c_SizeMM.y ) && ( c_UnknownPaperSize.c_SizeMM.y == c_PaperSize.c_SizeMM.x ) ){
						st_PaperSizeSelectTableWideInch[j].p_WidePaperKey->activate( True );
					} else if( c_DispKeyPaperSize.c_SizeMM.x > c_PaperSize.c_SizeMM.y || c_DispKeyPaperSize.c_SizeMM.y > c_PaperSize.c_SizeMM.x
					    || ( st_PaperSizeSelectTableWideInch[j].b_isTate == c_PaperSize.isTate() ) ) {	// c_DispKeyPaperSizeは、xyReverseしていないので、論理を逆にする.
						st_PaperSizeSelectTableWideInch[j].p_WidePaperKey->activate( False );
					} else {
						st_PaperSizeSelectTableWideInch[j].p_WidePaperKey->activate( True );
					}
				} else {	//自動検出がオフの場合
						st_PaperSizeSelectTableWideInch[j].p_WidePaperKey->activate( True );
				}
			}
		}
	}

	ss_Select = getButtonIndexPaperSizeInch( c_PaperSize );
	Rbt_WideABSize.showEnable( False );
	Rbt_WideInchSize.showEnable( True );
	Rbt_WideInchSize.select( ss_Select );
}


//-----------------------------------------------------------------------------------------------
/**
 *	@brief	A020114130_用紙ワイド紙 : 用紙サイズ/方向を表示.
 */
//-----------------------------------------------------------------------------------------------
void PCPC_PaperSizeWideInch_copy::setSize( TYPC_PaperSize size )
{
	if( ( TYPE_SizeCodeNonStandard != size.e_SizeCode ) && ( True == PnlComGetFeedTray()->getPaperSizeAutoDetectOn( pc_PaperSizeSelect.getSelectedFeedTray() ) ) ) {
		Bool find = False;
		uchar i = 0;
		for( i = 0; i < uc_SizeCode_PaperSizeWide; i++ ) {
			//テーブル内に一致するサイズコードがあるか検索.
			if( tbl_SizeCode_PaperSizeWide[ i ].size == size.e_SizeCode ) { find = True; break; }
		}

		if( True == find ) {
			//一致した場合.
			if( ( True == size.b_WideOn ) && ( s_Dummy != tbl_SizeCode_PaperSizeWide[ i ].us_WideTextID ) ) {
				//ワイド紙指定時.
				Txt_DispPaperSize.changeText( tbl_SizeCode_PaperSizeWide[ i ].us_WideTextID );
			} else {
				Txt_DispPaperSize.changeText( tbl_SizeCode_PaperSizeWide[ i ].us_TextID );
			}
		} else {
			//一致しなかった場合.
			PnlComPutDebugLog( True, UC_PNL_LOGPATTERN_1, "PCPC_PaperSizeWideInch_copy::setSize", UC_PNL_ERROR_USE );
			return;
		}
		//不定型用紙サイズテキスト郡を非表示にし、用紙サイズテキストを表示にする.
		Txt_XSizeText.showEnable( False );
		Txt_YSizeText.showEnable( False );
		Num_SizeXNum.showEnable( False );
		Num_SizeYNum.showEnable( False );
		Txt_DispPaperSize.showEnable( True );
// comment 2017/06/16 SCD2-Sky後藤 [IT5_4.1][AR1024210] 不定形サイズ可能トレイの印刷出来るサイズ範囲外の用紙を設定できない処理を追加.
	} else if( ( TYPE_SizeCodeUnknown == size.e_SizeCode ) && ( False == PnlComGetFeedTray()->getPaperSizeAutoDetectOn( pc_PaperSizeSelect.getSelectedFeedTray() ) ) ) {
		//自動検出キー非選択状態かつ不定形サイズ検出時(自動検出キーの選択を解除し、用紙サイズキー非選択)
		Txt_XSizeText.showEnable( False );
		Txt_YSizeText.showEnable( False );
		Num_SizeXNum.showEnable( False );
		Num_SizeYNum.showEnable( False );
		Txt_DispPaperSize.showEnable( True );
		Txt_DispPaperSize.changeText( s_pNOTDummy_W068H036A );	// 用紙サイズを「？？？」にする
	} else {	//不定形の場合.
// comment 2020/01/17 Sky東島 [EagleBK][OP_BTS-12665].
		correctNonStandardPaperSize( &size , pc_PaperSizeSelect.getSelectedFeedTray() );
		//用紙サイズテキストを非表示にし、不定形用紙サイズテキスト郡を表示にする.
		Txt_DispPaperSize.showEnable( False );
		Txt_XSizeText.showEnable( True );
		Txt_YSizeText.showEnable( True );
		Num_SizeXNum.showEnable( True );
		Num_SizeYNum.showEnable( True );

		//不定形サイズナンバーフィールドにサイズをセット.
//	comment 2020/03/12 HCLT_Sowndarya [HeliosMLK] [OP_BTS-21242] added function to display the minimum value.
//	comment 2020/04/04 HCLT_Hemachander [HeliosMLK] [OP_BTS-21927] Minimum Value Table Update for Paper Size.
		size.c_SizeMM.y = getCorrectMinSize (size.c_SizeMM.y );
		size.c_SizeMM.x = getCorrectMinSize (size.c_SizeMM.x );
//		static const float sf_AryNumMMConversionTbl[][3] = {
//			{420.0f, 420.687561f, 419.988983f },
//			{364.0f, 363.537537f, 364.007416f },
//			{297.0f, 296.862488f, 297.002197f },
//			{297.0f, 296.862549f, 297.002197f },
//			{257.0f, 257.175049f, 256.997192f },
//			{210.0f, 209.550049f, 210.007187f },
//			{182.0f, 182.562546f, 181.990997f },
//			{148.0f, 147.637543f, 148.005798f },
		static const float sf_AryNumMMConversionTbl[][5] = {
			{420.0f, 420.687561f, 419.988983f, 419.988983f , 419.988983f},
			{364.0f, 363.537537f, 364.007416f, 363.537506f , 363.537506f},
			{297.0f, 296.862488f, 297.002197f, 296.862549f , 297.002197f},
			{257.0f, 257.175049f, 256.997192f, 257.174988f , 256.997192f},
			{210.0f, 209.550049f, 210.007187f, 209.550003f , 210.007202f},
			{182.0f, 182.562546f, 181.990997f, 182.575195f , 182.562500f},
			{148.0f, 147.637543f, 148.005798f, 147.637497f , 148.005798f},
		};
		const uchar uc_AryNumMMConversionTbl = ( uchar )( sizeof( sf_AryNumMMConversionTbl ) / sizeof( sf_AryNumMMConversionTbl[0] ) );
//		for( uchar j = 0; j < uc_AryNumMMConversionTbl; j++ ) {
//			if ( size.c_SizeMM.x == sf_AryNumMMConversionTbl[j][1] || size.c_SizeMM.x == sf_AryNumMMConversionTbl[j][2] ) {
//			//If table value is same assign the mm number  value.
//			size.c_SizeMM.x = sf_AryNumMMConversionTbl[j][0];
//			}
//			if ( size.c_SizeMM.y == sf_AryNumMMConversionTbl[j][1] || size.c_SizeMM.y == sf_AryNumMMConversionTbl[j][2] ) {
//			//If table value is same assign the mm number  value.
//			size.c_SizeMM.y = sf_AryNumMMConversionTbl[j][0];
//			}
//		}
		for( uchar j = 0; j < uc_AryNumMMConversionTbl; j++ ) {
			if ( size.c_SizeMM.x == sf_AryNumMMConversionTbl[j][1] || size.c_SizeMM.x == sf_AryNumMMConversionTbl[j][2] || size.c_SizeMM.x == sf_AryNumMMConversionTbl[j][3] || size.c_SizeMM.x == sf_AryNumMMConversionTbl[j][4]) {
			//If table value is same assign the mm number  value.
			size.c_SizeMM.x = sf_AryNumMMConversionTbl[j][0];
			}
			if ( size.c_SizeMM.y == sf_AryNumMMConversionTbl[j][1] || size.c_SizeMM.y == sf_AryNumMMConversionTbl[j][2] || size.c_SizeMM.y == sf_AryNumMMConversionTbl[j][3] || size.c_SizeMM.y == sf_AryNumMMConversionTbl[j][4]) {
			//If table value is same assign the mm number  value.
			size.c_SizeMM.y = sf_AryNumMMConversionTbl[j][0];
			}
		}
		Num_SizeXNum.set( ( float )size.c_SizeMM.y  , True );
		Num_SizeYNum.set( ( float )size.c_SizeMM.x  , True );
	}
}

//-----------------------------------------------------------------------------------------------
/**
 *	@brief	AA020114130_用紙ワイド紙 : タブキー選択時コールバック関数.
 */
//-----------------------------------------------------------------------------------------------
void PCPC_PaperSizeWideInch_copy::ans_TabKey( uchar option )
{
	if( ( NULL == PnlComGetJobCopy() ) ||
	    ( NULL == PnlComGetJobCopy()->getJobSpec() ) ||
	    ( NULL == PnlComGetJobCopy()->getJobSpec()->pc_Print ) ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "[PCPC_PaperSizeWideInch_copy::ans_TabKey]Assert Failed!" );
		return;
	}
	if( TYPE_FeedTray1 == PnlComGetJobCopy()->getJobSpec()->pc_Print->c_PaperTray.getFeedTray() ) {
		gp_PaperSizeWideTray1_copy.m_SelectTabKeyOption = option;


		gp_PaperSizeWideTray1_copy.setupGamen();		//タブインデックス押下時にセットアップ画面をコール.
	} else {
		gp_PaperSizeWideInch_copy.m_SelectTabKeyOption = option;

		gp_PaperSizeWideInch_copy.setupGamen();		//タブインデックス押下時にセットアップ画面をコール.
	}
}
// comment 2017/06/16 SCD2-Sky後藤 [IT5_4.1][AR1024210] 不定形サイズ可能トレイの印刷出来るサイズ範囲外の用紙を設定できない処理を追加.
//-----------------------------------------------------------------------------------------------
/**
 *	@brief	AA020114130_用紙ワイド紙 : タイトルエリア変更処理.
 */
//-----------------------------------------------------------------------------------------------
void PCPC_PaperSizeWideInch_copy::setupTitle()
{
	// 設定変更の対象となるトレイの取得.
	TYPE_FeedTray e_FeedTray = PnlComGetJobCopy()->getJobSpec()->pc_Print->c_PaperTray.getFeedTray();
	TEXT_ID tid_Title = 0;

#ifdef DEF_LCD_7INCH		
	switch ( e_FeedTray )
	{		
		case TYPE_FeedTrayManual:
			tid_Title = s_pITMWidePaper_Manual_W196H024A; // ワイド紙（手差し）.
			break;
		case TYPE_FeedTray1:
			tid_Title = s_pITMWidePaper_Tray1_W196H024A; // ワイド紙（トレイ１）.
			break;
		case TYPE_FeedTray2:
			tid_Title = s_pITMWidePaper_Tray2_W196H024A; // ワイド紙（トレイ２）.
			break;
		case TYPE_FeedTray3:
			tid_Title = s_pITMWidePaper_Tray3_W196H024A; // ワイド紙（トレイ３）.
			break;
		case TYPE_FeedTray4:
			tid_Title = s_pITMWidePaper_Tray4_W196H024A; // ワイド紙（トレイ４）.
			break;
//comment 2018/5/18 SCD2-Sky肥後 [DenebMLK][機器構成変更No.7]5段目トレイ追加対応.
#ifdef DEF_DenebMLKCus
		case TYPE_FeedTray5:
			tid_Title = s_pITMWidePaper_Tray5_W212H024A; // ワイド紙（トレイ５）.
			break;
// comment 2018/05/17 SCD2-Sky中野 [DenebMLK][機器構成変更No.7] 5段増設プリント機能追加対応.
#endif	// DEF_DenebMLKCus.
		default:
			// 何もしない. 
			 break;
	}		
#else // #ifdef DEF_LCD_7INCH.
	switch( e_FeedTray )
	{
		case TYPE_FeedTrayManual:
			tid_Title = s_pITMWidePaper_Manual_W212H024A; // ワイド紙（手差し）.
			break;
		case TYPE_FeedTray1:
			tid_Title = s_pITMWidePaper_Tray1_W212H024A; // ワイド紙（トレイ１）.
			break;
		case TYPE_FeedTray2:
			tid_Title = s_pITMWidePaper_Tray2_W212H024A; // ワイド紙（トレイ２）.
			break;
		case TYPE_FeedTray3:
			tid_Title = s_pITMWidePaper_Tray3_W212H024A; // ワイド紙（トレイ３）.
			break;
		case TYPE_FeedTray4:
			tid_Title = s_pITMWidePaper_Tray4_W212H024A; // ワイド紙（トレイ４）.
			break;
		default:
			// 何もしない. 
			break;
	}
#endif // #ifdef DEF_LCD_7INCH.
	
	// タイトルの切り替え.
	Txt_WidePaper_PaperSizeWide.changeText( tid_Title );
}
//クラス-----------------------------------------------------------------------------------------------------------------.
//概要    ：メッセージの変更.
//備考    ：.
//作成者  ：.
//レビュー：.
//変更履歴：.
//-----------------------------------------------------------------------------------------------.
/**
* @brief A020114130 選択トレイの設定変更 : メッセージの変更.
*/
//-----------------------------------------------------------------------------------------------.
void PCPC_PaperSizeWideInch_copy::changeMessage()
{
	ushort us_TextID = 0;	// メインメッセージ.

	TYPE_FeedTray e_FeedTray = PnlComGetJobCopy()->getJobSpec()->pc_Print->c_PaperTray.getFeedTray();
	TYPC_PaperSize c_PaperSizeResult = pc_PaperSizeSelect.getPaperSize();
	c_PaperSizeResult.MMtoSizeCode();
	TYPC_PaperSize c_PaperSize = pc_PaperSizeSelect.getPaperSize();
	c_PaperSize.c_SizeMM = c_PaperSize.c_SizeMM.xyReverse();			// 12_1/4ｘ18判定する際に、JobSpecに入っているサイズはxy反転しているため.
	
	Bool			bl_Fixed = 	 PnlComIsUnfixedPaperSizeTray( e_FeedTray );													// 不定形可能トレイ.
	Bool			bl_Auto  =   PnlComGetFeedTray()->getPaperSizeAutoDetectOn( e_FeedTray ) ;	// 自動検出判定.
	Bool			bl_SizeNonStandard = False; 	// 12_1/4ｘ18判定.
	
//Comment 2017/01/18 [HCL-Abhay] [IT5 4.2] WarningRemoval. Made parentheses in proper way.	
	if( ( False == bl_Auto )																		// 12×18が選択されているか判定.
//	&&  ( PCGD_12_18PaperSizeX == c_PaperSize.c_SizeMM.x   && PCGD_12_18PaperSizeY == c_PaperSize.c_SizeMM.y )
//	||  ( PCGD_1225_18PaperSizeX == c_PaperSize.c_SizeMM.x && PCGD_1225_18PaperSizeY == c_PaperSize.c_SizeMM.y ) ){
	&& ( ( PCGD_12_18PaperSizeX == c_PaperSize.c_SizeMM.x   && PCGD_12_18PaperSizeY == c_PaperSize.c_SizeMM.y )
	||  ( PCGD_1225_18PaperSizeX == c_PaperSize.c_SizeMM.x && PCGD_1225_18PaperSizeY == c_PaperSize.c_SizeMM.y ) ) ){
		bl_SizeNonStandard = True;
	}																			
	Bool			bl_AutoResult      = ( TYPE_SizeCodeUnknown == c_PaperSizeResult.e_SizeCode ? False : True );				// 自動検出結果.
	Bool			bl_SizeSelect	   = IsSelectButtonPaperSize( c_PaperSize );												// サイズキーの選択.
	if( TYPE_FeedTrayManual == e_FeedTray ){													// 手差しトレイの選択時
		for( uchar i = 0; i < uc_PaperChangeMessageManualTable; i++ ) {
			if( ( bl_AutoResult == st_PaperChangeMessageManualTable[i].bl_AutoResult )			// 自動検出結果.
			&&  ( bl_SizeSelect ==  st_PaperChangeMessageManualTable[i].bl_SizeSelect ) ){		// サイズキー選択.
				us_TextID = st_PaperChangeMessageManualTable[i].us_MSGTextID;
			}
		}
	}else{			
		for( uchar j = 0; j < uc_PaperChangeMessageTable; j++ ) {
			if( ( bl_Fixed == st_PaperChangeMessageTable[j].bl_Fixed )						 // 定形可能トレイ.
			&&  ( bl_Auto ==  st_PaperChangeMessageTable[j].bl_Auto )					 	 // 自動検出チェック判定.
			&&  ( bl_SizeNonStandard == st_PaperChangeMessageTable[j].bl_SizeNonStandard )	 // 12_1/4ｘ18判定.	
			&&  ( bl_AutoResult == st_PaperChangeMessageTable[j].bl_AutoResult ) 			 // 自動検出結果.																	
			&&  ( bl_SizeSelect ==  st_PaperChangeMessageTable[j].bl_SizeSelect ) ){		 // サイズキー選択.
				us_TextID = st_PaperChangeMessageTable[j].us_MSGTextID;
			}
		}
	}
	// 「???」表示時でジョブに反映されている場合、ジョブに設定値が反映されていると.
	//  ペーパーエンプティ警告を検知しないため「用紙を補給してください」を表示する.
	if( ( False == bl_AutoResult )
	&&  ( True == bl_Auto ) ){
		us_TextID = s_pMSGSupplyPaper_W448H034A;											//「用紙を補給してください」.
	}
	// メッセージ変更.
	Cmp_MessageBand.changeMainMessageText( us_TextID );		
}
//クラス-----------------------------------------------------------------------------------------------------------------.
//概要    ：設定終了.
//備考    ：.
//作成者  ：.
//レビュー：.
//変更履歴：.
//-----------------------------------------------------------------------------------------------.
/**
* @brief A020114130 選択トレイの設定変更 : 設定終了.
*/
//-----------------------------------------------------------------------------------------------.
void PCPC_PaperSizeWideInch_copy::endGamen()
{
	// 親のprocOK()をコールする.
	PPopupDecisionButtonCom::endGamen();		

	TYPE_FeedTray e_FeedTray = pc_PaperSizeSelect.getSelectedFeedTray();
	
	TYPC_PaperSize c_PaperSize = pc_PaperSizeSelect.getPaperSize();
	c_PaperSize.MMtoSizeCode();

	// 不定形可能トレイの場合.
	if( ( True == PnlComGetFeedTray()->getPaperSizeAutoDetectOn( e_FeedTray ) )
	&&  ( TYPE_FeedTrayManual != e_FeedTray )
	&&  ( TYPE_SizeCodeUnknown == c_PaperSize.e_SizeCode ) ){
		TYPC_Print* pc_Print = PnlComGetJobCopy()->getJobSpec()->pc_Print;
		
		// 用紙サイズをジョブに設定.
		pc_Print->c_PaperSize = c_PaperSize;
		onlyPanel.setFeedSizeKind( pc_PaperSizeSelect.getSelectedFeedTray() );	// トレイへの設定.
		selectAutoPaperMag();	// APS/AMS処理関数.
	}
}
//クラス-----------------------------------------------------------------------------------------------------------------.
//概要    ：ワイド紙キーが選択されているか..
//備考    ：.
//作成者  ：.
//レビュー：.
//変更履歴：.
//-----------------------------------------------------------------------------------------------.
/**
* @brief A020114130 選択トレイの設定変更 : ワイド紙キーが選択されているか.
*/
//-----------------------------------------------------------------------------------------------.
Bool PCPC_PaperSizeWideInch_copy::IsSelectButtonPaperSize( TYPC_PaperSize c_PaperSize )
{
	Bool bl_Result = False;
	
	const short ss_SelectMt = getButtonIndexPaperSizeMetric( c_PaperSize );
	const short ss_SelectIn = getButtonIndexPaperSizeInch( c_PaperSize );
	if( ( 0<= ss_SelectMt )				// ワイドメトリック定型検出.
	||  ( 0<= ss_SelectIn ) ) {			// ワイドインチ定型検出.
	 	bl_Result = True;
	}
	return bl_Result;
}

/*================================================================================================
  定数・型の宣言.
================================================================================================*/
// 用紙不定形サイズのメモリー番号.
#define PNLD_SELECTMEMORYNO_NON					0				// 対象なし.
#define PNLD_SELECTMEMORYNO_1					1				// Memory1.
#define PNLD_SELECTMEMORYNO_2					2				// Memory2.
#define PNLD_SELECTMEMORYNO_3					3				// Memory3.
#define PNLD_SELECTMEMORYNO_4					4				// Memory4.
#define PNLD_SELECTMEMORYNO_5					5				// Memory5.

// comment [2015/06/01] [HCLT-Karthi][MinervaSBK][機器構成変更_No.23] New screen(J320114210) implimentation.
//-----------------------------------------------------------------------------------------------
/**
 *	@brief	J320114210_ポストインサーター／用紙定形サイズ_指摘版  : Constructor.
 */
//-----------------------------------------------------------------------------------------------	
PCPC_PIPaperSizeMetric_copy::PCPC_PIPaperSizeMetric_copy(
		PDisplayParts**			partsTable, 
		short					ss_partsNumber, 
		Bool					bl_NoWarningGamen,
		PPopupAnimationGroup*	pc_AnimationComponent,
		short					ss_PosX,
		short					ss_PosY,
		short					ss_SizeX,
		short					ss_SizeY,
		PDisplayParts*			pc_FunctionBadgeButton
		)
		: PPopupDecisionButtonCom( partsTable, ss_partsNumber, bl_NoWarningGamen, pc_AnimationComponent, ss_PosX, ss_PosY, ss_SizeX, ss_SizeY, pc_FunctionBadgeButton )
{
}

//-----------------------------------------------------------------------------------------------
/**
 *	@brief	: J320114210_ポストインサーター／用紙定形サイズ_指摘版  : Screen Start Processing.
 *	Remarks : Get the current paper size from JobSpec, to set the parameters.
 */
//-----------------------------------------------------------------------------------------------	
void PCPC_PIPaperSizeMetric_copy::startGamen()
{
	PPopupDecisionButtonCom::startGamen();

	PNLC_CopyDataLib* pc_CopyDataLib = PNLC_CopyDataLib::newInstance();
	if ( NULL == pc_CopyDataLib ) {
		LOGF_debug_PutEventLog(LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "[PCPC_PIPaperSizeMetric_copy::startGamen]");
		return;
	}

	TYPE_FeedTray e_FeedTray	= pc_CopyDataLib->getSelectedFeedTray();
	APIC_FeedTray* pc_FeedTray	= PnlComGetFeedTray();
	TYPC_PaperSize c_PaperSize	= pc_FeedTray->getPaperSize( e_FeedTray );	

	//Set Paper Size.
	Cmp_PIPaperSizeSelectList.setSelectPaperSize( c_PaperSize.e_SizeCode, PNLE_PaperSizeSelect_MoveNoAnimation );	
	
	//Get the Tab selected in Previous Screen	.
	PNLE_CopyPaperTypeButton e_CopyPaperTypeButton = pc_CopyDataLib->getCopyPaperTypeButton();
	PNLE_PaperSizeGroup e_SelectedPaperSizeGroup = PNLE_PaperSizeGroup_Metric;
	
	switch( e_CopyPaperTypeButton ) {
			case PNLD_ButtonPaperKindOther:
				//Others.
				e_SelectedPaperSizeGroup = PNLE_PaperSizeGroup_8K16K;
				break;
			case PNLD_ButtonPaperKindAB:
				// Metric.
				e_SelectedPaperSizeGroup = PNLE_PaperSizeGroup_Metric;
				break;
			case PNLD_ButtonPaperKindInch:
				// Inch.
				e_SelectedPaperSizeGroup = PNLE_PaperSizeGroup_Inch;
				break;
			default:
				break;
		}
	
	
	//Pop-up Message Area.
	Cmp_PIPaperSizeSelectList.setSelectPaperSizeGroup( e_SelectedPaperSizeGroup, PNLE_PaperSizeSelect_MoveNoAnimation );
	Cmp_PISTPSize_PopupMessageArea.setMode( PNLE_PopupMessageAreaMessageModeCopy );	// Set Copy Mode.
	Cmp_PISTPSize_PopupMessageArea.setNowPopupGamen( PNLE_CopyPaperFixSize );			// Set Now Screen.
	Cmp_PISTPSize_PopupMessageArea.setOnOffButton( False );							// ON/OFF Button.
	Cmp_PISTPSize_PopupMessageArea.showDestinationCount( False );						// Hide Destination.
	
}

//-----------------------------------------------------------------------------------------------
/**
 *	@brief	J320114210_ポストインサーター／用紙定形サイズ_指摘版  : Setup the display components.
 */
//-----------------------------------------------------------------------------------------------
void PCPC_PIPaperSizeMetric_copy::setupGamen( short ss_option )
{
	PPopupDecisionButtonCom::setupGamen( ss_option );
}

//-----------------------------------------------------------------------------------------------------------------------
/**
 *	@brief	J320114210_ポストインサーター／用紙定形サイズ_指摘版  : Tab menu in the key is pressed during processing (Qt).
 */
//-----------------------------------------------------------------------------------------------------------------------
void PCPC_PIPaperSizeMetric_copy::procQtButtonOn( ushort us_option )
{
	//Set Specified Paper Size in Selected State.
	Cmp_PIPaperSizeSelectList.setSelectPaperSize( (TYPE_SizeCode)us_option, PNLE_PaperSizeSelect_NoMove );
	
	//Set Selected Paper SIze.
	const TYPC_PaperSize c_PaperSize = PnlComGetFixedPaperSizeCodeToMM( (TYPE_SizeCode)us_option );
	setPaperSize(c_PaperSize);
	
	//Call SetupGamen.
	setupGamen();

}

//----------------------------------------------------------------------------------------------------------------
/**
 *	@brief	J320114210_ポストインサーター／用紙定形サイズ_指摘版  : Tab Menu key is pressed during processing (Qt).
 */
//----------------------------------------------------------------------------------------------------------------
void PCPC_PIPaperSizeMetric_copy::procQtEventNotice( ushort us_option )
{
	//Call Setup Gamen.
	setupGamen();
}
	
//-----------------------------------------------------------------------------------------------
/**
 *	@brief	J320114210_ポストインサーター／用紙定形サイズ_指摘版  : To set the Paper size.
 */
//-----------------------------------------------------------------------------------------------
void PCPC_PIPaperSizeMetric_copy::setPaperSize( TYPC_PaperSize  c_PaperSize )
{
	
	APIC_FeedTray* pc_FeedTray = PnlComGetFeedTray();
	
	//Get Selected Tray.
	const TYPE_FeedTray e_FeedTray = getSelectedFeedTray();
	
	pc_FeedTray->setPaperSize( e_FeedTray, c_PaperSize );
	pc_FeedTray->setPaperSizeAutoDetectOn(e_FeedTray, False);	
}

//-----------------------------------------------------------------------------------------------
/**
 *	@brief	J320114210_ポストインサーター／用紙定形サイズ_指摘版  : Selected tray acquisition.
 *	Return value :	TYPE_FeedTray class.
 */
//-----------------------------------------------------------------------------------------------
TYPE_FeedTray  PCPC_PIPaperSizeMetric_copy::getSelectedFeedTray()
{
	PNLC_CopyDataLib* pc_CopyDataLib = PNLC_CopyDataLib::newInstance();
	if ( NULL == pc_CopyDataLib ) {
		LOGF_debug_PutEventLog(LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "[PCPC_PIPaperSizeMetric_copy::getSelectedFeedTray]Assert Failed!");
		return TYPE_CustomPaperType_UnSelect;
	}
	return pc_CopyDataLib->getSelectedFeedTray();
}	

//-----------------------------------------------------------------------------------------------
/**
 *	@brief	J320114200_ポストインサーター／選択トレイの設定変更  : Screen start processing. 
 */
//-----------------------------------------------------------------------------------------------
void PCPC_PostInserterSetting_copy::startGamen()
{
	//Call Parent class.
	PPopupDecisionButtonCom::startGamen();

	TYPE_PaperType	e_PaperType = getJobPaperType();		// Paper Type.
	
	//Set Paper type to component.
	Cmp_PIPaperTypeSelectList.setSelectPaperType( e_PaperType, PNLE_PaperTypeSelect_MoveNoAnimation );

	//Message Area Initialization.
	Cmp_PICopyPaperTypeUniversalSettingChangePMA.setMode( PNLE_PopupMessageAreaMessageModeCopy );
	Cmp_PICopyPaperTypeUniversalSettingChangePMA.showDestinationCount( False );
	Cmp_PICopyPaperTypeUniversalSettingChangePMA.setOnOffButton( False );
	Cmp_PICopyPaperTypeUniversalSettingChangePMA.setNowPopupGamen( PNLE_CopySelectionTraySettingChange );
}

//-----------------------------------------------------------------------------------------------
/**
 *	@brief	J320114200_ポストインサーター／選択トレイの設定変更  : Screen drawing/displaying processing. 
 */
//-----------------------------------------------------------------------------------------------
void PCPC_PostInserterSetting_copy::setupGamen(short ss_option)
{
	PPopupDecisionButtonCom::setupGamen( ss_option );
	
	changeMessage();
	
	Bool bl_SelectInchSize = False;

	TYPE_FeedTray e_FeedTray = getSelectedFeedTray();
	APIC_FeedTray* pc_FeedTray = PnlComGetFeedTray();
	TYPC_PaperSize c_PaperSize = pc_FeedTray->getPaperSize(e_FeedTray);
	c_PaperSize.c_SizeMM = c_PaperSize.c_SizeMM.xyReverse();	
	
	if ( True == pc_FeedTray->getPaperSizeAutoDetectOn( e_FeedTray ) ){
		Key_PIAutoSearch.select( True );
	}else{
	 	Key_PIAutoSearch.select( False );
		if ( TYPE_SizeCodeNonStandard == c_PaperSize.e_SizeCode ){
			bl_SelectInchSize = True;
		}
	}
		
	//Paper Type Class from Feed Tray API.
	TYPC_PaperType c_PaperType = pc_FeedTray->getPaperType(e_FeedTray);
	const TYPE_PaperType e_PaperType = c_PaperType.getPaperType();
	
	//Paper Type. 

	Cmp_PIPaperTypeSelectList.setSelectPaperType( e_PaperType, PNLE_PaperTypeSelect_MoveNoAnimation );
	//assigning the array index to the display graphic of the paper.
	for ( uchar ss_PaperKindIndex = 0; uc_SizePaperKindNewTblPI > ss_PaperKindIndex; ++ss_PaperKindIndex ) 
	{
		if ( e_PaperType == st_PaperKindNewTblPI[ss_PaperKindIndex].e_PaperType )
		{
			Rot_NewPIPaperTypeIcon.changeGraphic(ss_PaperKindIndex);
			Txt_PIPaperDirectionTypeEx.changeText( puc_PIPaperWeight[ss_PaperKindIndex] );
			break;
		}
	}
	
	PNLS_setOriginalSizeCLCD2Key s_parts;
	
	s_parts.ps_KeyOriginalSize     		=	&Bak_PIPaperSizeAndDirectionAreaEx;			// Size display area.
	s_parts.ps_TxtOriginalSize01		=	&Txt_PIDispPaperSize1Ex;					// The first line paper code display.
	s_parts.ps_TxtOriginalSize02		=	NULL;										// The second line automatic display.
	s_parts.ps_TxtOriginalSizeX			=	&Txt_PIXSizeText1Ex;						// Paper size display X.
	s_parts.ps_TxtOriginalSizeY			=	&Txt_PIYSizeText1Ex;						// Paper size display Y.
	s_parts.ps_NumOriginalSizeX			=	&Num_PISizeXNum1Ex;							// Paper size numerical display X.
	s_parts.ps_NumOriginalSizeY			=	&Num_PISizeYNum1Ex;							// Paper size numeric display Y.
	s_parts.ps_TrayTxtPaperSize			=	&Ktx_PIPaperSizeAndDirectionArea;			// Tray display text.
	s_parts.ps_RemainIconPaperSize		=	NULL;
	s_parts.ps_TypeIconPaperSize		=	NULL;
	
	s_parts.ps_TrayTxtPaperSize			=	&Ktx_KeySelectPaperTray_DispTrayPI;
	if( TYPE_FeedTrayPI1 == e_FeedTray ){
		Ktx_KeySelectPaperTray_DispTrayPI.changeText(s_pNOTTrayF1_W032H040A);
	} else {
		Ktx_KeySelectPaperTray_DispTrayPI.changeText(s_pNOTTrayF2_W032H040A);
	}
	PnlComsetPaperSizeCLCD2( &s_parts, e_FeedTray );
	if ( TYPE_UnitSystemMM_Number == PnlComGetUnitSystem() ) {
		Txt_XSizemmPI.showEnable(bl_SelectInchSize);
		Txt_YSizemmPI.showEnable(bl_SelectInchSize);
	} else {
		Txt_XSizemmPI.showEnable(False);
		Txt_YSizemmPI.showEnable(False);
	}	
//comment 2019/04/26 HCLT-Sivateja [IT6_EagleH][OP_BTS-4773 ] Adding PaperType for PI tray in EagleHI models.
	if(True == PnlComIsEagleHiModel()){
	if( TYPE_SystemType_JP == PnlComGetMachineSetting()->getSystemType() ){
		Key_Ex1.changeText( s_pKEYNormalEcoPaper_W120H028A );
	} else {
		Key_Ex1.changeText( s_pKEYNormalExPaper_W120H028A );
		}
	if( TYPE_PaperTypeNormal == e_PaperType ){
		Key_Ex1.activate( True );
		if( True == c_PaperType.getPaperTypeNormalEx() ){
			Key_Ex1.chkOn();
			Txt_PIPaperDirectionTypeEx.changeText(s_pNOT60to70gm2_W160H024D);
		}	else {
			Key_Ex1.chkOff();
			}
	}	else {
		Key_Ex1.chkOff();
		Key_Ex1.activate( False );
		}
	}
// comment 2016/05/19 SCD2-2G-Sky福田 [ZeusSBK][CQ529248] TBD230対応.
	
	setupStandardPaperSize();
}

//-----------------------------------------------------------------------------------------------.
/**
* @brief J320114200_ポストインサーター／選択トレイの設定変更  : Changing Message.
*/
//-----------------------------------------------------------------------------------------------.
void PCPC_PostInserterSetting_copy::changeMessage()
{
	ushort us_MainTextID = 0;	// メインメッセージ.
	ushort us_SubTextID1 = 0;	// サブメッセージの１.
	ushort us_SubTextID2 = 0;	// サブメッセージの２.

// comment 2016/06/16 [HCL_VijayK] [IT5_3.3][CQ1002027] Quater font activity changes.
//	us_MainTextID = s_pMSGCheckPapeTypeAndSetPaperSizePI_W448H034A;
	us_MainTextID = s_pMSGCheckPapeTypeAndSetPaperSizePI_W756H034A;

	Cmp_PICopyPaperTypeUniversalSettingChangePMA.changeMainMessageText( us_MainTextID );
	Cmp_PICopyPaperTypeUniversalSettingChangePMA.changeSubMessageText2( us_SubTextID1, us_SubTextID2 );
}

//-----------------------------------------------------------------------------------------------
/**
 *	@brief	J320114200_ポストインサーター／選択トレイの設定変更  : Get the paper type selected from the Job.
 */
//-----------------------------------------------------------------------------------------------
TYPE_PaperType PCPC_PostInserterSetting_copy::getJobPaperType()
{
	TYPE_FeedTray  e_FeedTray  = getSelectedFeedTray();					// Get selected tray.
	APIC_FeedTray* pc_FeedTray = PnlComGetFeedTray();					// Get the paper feed tray information.
	TYPC_PaperType c_PaperType = pc_FeedTray->getPaperType(e_FeedTray);	// Switching of paper type code.
	TYPE_PaperType e_PaperType = c_PaperType.getPaperType();			// Get the paper type.
	return e_PaperType;
}
	
//-----------------------------------------------------------------------------------------------
/**
 *	@brief	J320114200_ポストインサーター／選択トレイの設定変更  : Button press processing.
 */
//-----------------------------------------------------------------------------------------------
void PCPC_PostInserterSetting_copy::procButtonOn(uchar uc_option)
{

	PNLC_CopyDataLib* pc_CopyDataLib = PNLC_CopyDataLib::newInstance();
	if ( NULL == pc_CopyDataLib ) {
		LOGF_debug_PutEventLog(LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "[PCPC_PostInserterSetting_copy::procButtonOn]Assert Failed!");
		return;
	}
	
	const TYPE_FeedTray e_FeedTray = pc_CopyDataLib->getSelectedFeedTray();
	APIC_FeedTray* pc_FeedTray = PnlComGetFeedTray();
	TYPC_PaperType c_PaperType = pc_FeedTray->getPaperType(e_FeedTray);
	TYPC_PaperSize c_PaperSize;
	short ss_xPositon = 0;
	short ss_yPosition = 0;
	short ss_width = 0;
	short ss_height = 0;


	switch ( uc_option )
	{
		case BTNID_KEY_AUTOSERCH:					// Automatic detection.
			pc_FeedTray->setPaperSizeAutoDetectOn(e_FeedTray, True);
			break;
		case BTNID_KEY_PAPERKINDP1_1:				// Paper type P1_1 key.
			c_PaperType.setPaperType( TYPE_PaperTypeNormal );
			pc_FeedTray->setPaperType( e_FeedTray, c_PaperType);
//comment 2019/07/08 HCLT-Sivteja] [IT6_EagleH][OP_BTS-4773 ] fix done for ECO/Alterthickness selction according to the paper type and destination.
			if( TYPE_SystemType_JP != PnlComGetMachineSetting()->getSystemType() ){
				c_PaperType.setPaperTypeNormalEx( False );
			} else {
				c_PaperType.setPaperTypeNormalEx( True );
			}
			pc_FeedTray->setPaperType( e_FeedTray, c_PaperType);
			break;
// comment 2018/03/29 SCD2 K.Sato [Eagle][RQ-722]用紙種別追加（厚紙1を2種類に分割）対応.
		case BTNID_KEY_PAPERKINDP1_5:				// Paper type P1_5 key.
			c_PaperType.setPaperType( TYPE_PaperTypeNormalPlus );
			pc_FeedTray->setPaperType( e_FeedTray, c_PaperType);
			break;
		case BTNID_KEY_PAPERKINDP1_6:				// Paper type P1_6 key.
			c_PaperType.setPaperType( TYPE_PaperTypeThick1 );
			pc_FeedTray->setPaperType( e_FeedTray, c_PaperType);
			break;
		case BTNID_KEY_PAPERKINDP1_7:				// Paper type P1_7 key.
			c_PaperType.setPaperType( TYPE_PaperTypeThick1Plus );
			pc_FeedTray->setPaperType( e_FeedTray, c_PaperType);
			break;
		case BTNID_KEY_PAPERKINDP1_8:				// Paper type P1_8 key.
			c_PaperType.setPaperType( TYPE_PaperTypeThick2 );
			pc_FeedTray->setPaperType( e_FeedTray, c_PaperType);
			break;
//comment 2019/04/26 HCLT-Sivateja [IT6_EagleH][OP_BTS-4773 ] Adding PaperType for PI tray in EagleHI models.
		case BTNID_KEY_PAPERKINDP1_9:				// Paper type P1_9 key.
		c_PaperType.setPaperType( TYPE_PaperTypeThick3 );
			pc_FeedTray->setPaperType( e_FeedTray, c_PaperType);
			break;
		case BTNID_KEY_PAPERKINDP1_3:				// Paper type P1_10 key.
			c_PaperType.setPaperType( TYPE_PaperTypeThin1 );
			pc_FeedTray->setPaperType( e_FeedTray, c_PaperType);
			break;
		case BTNID_KEY_PAPERKINDP1_12:				// Paper type P1_11 key.
			c_PaperType.setPaperType( TYPE_PaperTypeThick4 );
			pc_FeedTray->setPaperType( e_FeedTray, c_PaperType);
			break;	
		case BTNID_KEY_PAPERKINDP1_13:				// Paper type label..
			c_PaperType.setPaperType( TYPE_PaperTypeLabel );
			pc_FeedTray->setPaperType( e_FeedTray, c_PaperType);
			break;	
//comment 2019/05/08 HCLT-Sivateja [IT6_EagleH][OP_BTS-4773 ] fixed jenkin error.
	case BTNID_KEY_EX1:	// Exキー.
			{
				const TYPE_PaperType e_PaperType = c_PaperType.getPaperType();
				if( TYPE_PaperTypeNormal == e_PaperType ){
					PNLE_PaperEx e_Ex = (( True == c_PaperType.getPaperTypeNormalEx() ) ? PNLE_PaperEx_Off : PNLE_PaperEx_On );
					if( PNLE_PaperEx_NotSet != e_Ex ) {
						c_PaperType.setPaperTypeNormalEx( ( ( PNLE_PaperEx_On == e_Ex ) ? True : False ) );
					} 
				} else {		// 普通紙でない場合はEco/厚さ変更を解除する.
					c_PaperType.setPaperTypeNormalEx( False );
				}
				pc_FeedTray->setPaperType( e_FeedTray, c_PaperType);
			}
			break;
		case BTNID_KEY_GOTO8K16KSIZE:				// Other key.
			pc_CopyDataLib->setCopyPaperTypeButton( PNLD_ButtonPaperKindOther );
			Kex_8k16kSize.getPosition( ss_xPositon, ss_yPosition );
			Kex_8k16kSize.getSize( ss_width, ss_height );
			PNLC_PopupListControl::newInstance()->addPopup( &gp_PIPaperSizeMetric_copy, this, NULL, ss_xPositon, ss_yPosition, ss_width, ss_height );
			break;
		case BTNID_KEY_GOTOMETRICSIZE:				// AB system key.
			pc_CopyDataLib->setCopyPaperTypeButton( PNLD_ButtonPaperKindAB );
			Kex_MetricSize.getPosition( ss_xPositon, ss_yPosition );
			Kex_MetricSize.getSize( ss_width, ss_height );
			PNLC_PopupListControl::newInstance()->addPopup( &gp_PIPaperSizeMetric_copy, this, NULL, ss_xPositon, ss_yPosition, ss_width, ss_height );
			break;
		case BTNID_KEY_GOTOINCHSIZE:				// Inch system key.
			pc_CopyDataLib->setCopyPaperTypeButton( PNLD_ButtonPaperKindInch );
			Kex_InchSize.getPosition( ss_xPositon, ss_yPosition );
			Kex_InchSize.getSize( ss_width, ss_height );
			PNLC_PopupListControl::newInstance()->addPopup( &gp_PIPaperSizeMetric_copy, this, NULL, ss_xPositon, ss_yPosition, ss_width, ss_height );
			break;
		case BTNID_KEY_GOTONOFIXPAPER:				// Non-standard size key/Custom size key.
			Key_CustomSize.getPosition( ss_xPositon, ss_yPosition );
			Key_CustomSize.getSize( ss_width, ss_height );
			PNLC_PopupListControl::newInstance()->addPopup( &gp_J320114220_4, this, NULL, ss_xPositon, ss_yPosition, ss_width, ss_height );
			break;
		default: LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); break;
	}
	setupGamen();
}

//-----------------------------------------------------------------------------------------------
/**
 *	@brief	J320114200_ポストインサーター／選択トレイの設定変更 : To get the selected feed tray.
 */
//-----------------------------------------------------------------------------------------------
TYPE_FeedTray PCPC_PostInserterSetting_copy::getSelectedFeedTray()
{
	PNLC_CopyDataLib* pc_CopyDataLib = PNLC_CopyDataLib::newInstance();
	if ( NULL == pc_CopyDataLib ) {
		LOGF_debug_PutEventLog(LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "[PCPC_PostInserterSetting_copy::getSelectedFeedTray]Assert Failed!");
		return TYPE_CustomPaperType_UnSelect;
	}

	return pc_CopyDataLib->getSelectedFeedTray();
}

//-----------------------------------------------------------------------------------------------
/**
 *	@brief	J320114200_ポストインサーター／選択トレイの設定変更 : Qt button press processing.
 */
//-----------------------------------------------------------------------------------------------
void PCPC_PostInserterSetting_copy::procQtButtonOn(ushort us_option)
{
	uchar			uc_BtnID = 0;									// Button ID.
	uchar			uc_Cnt = 0;										// Loop counter.
//	TYPE_FeedTray	e_FeedTray = getSelectedFeedTray();				// Tray type.

#ifndef DEF_A4_Cus
	for( uc_Cnt = 0; uc_Cnt < uc_TrayPaperKindBtnFromPaperTypeNum; uc_Cnt++ ) {
		if( tbl_TrayPaperKindBtnFromPaperType[uc_Cnt].e_PaperType == us_option ) {
			uc_BtnID = tbl_TrayPaperKindBtnFromPaperType[uc_Cnt].uc_BtnID;
			procButtonOn(uc_BtnID);
			break;
		}
	}
#else // DEF_A4_Cus.
	TYPE_FeedTray	e_FeedTray = getSelectedFeedTray();				// Tray type.
	uchar uc_idx = getTrayTblIdxFromFeedTray(e_FeedTray);
	for( uc_Cnt = 0; uc_Cnt < getTrayPaperKindBtnFromPaperTypeTblSize(); uc_Cnt++ ) {
		if( tbl_TrayPaperKindBtnFromPaperType[uc_idx][uc_Cnt].e_PaperType == us_option ) {
			uc_BtnID = tbl_TrayPaperKindBtnFromPaperType[uc_idx][uc_Cnt].uc_BtnID;
			procButtonOn(uc_BtnID);
			break;
		}
	}
#endif // DEF_A4_Cus.
}	
//-----------------------------------------------------------------------------------------------
/**
 *	@brief	J320114200_ポストインサーター／選択トレイの設定変更 : Notification of the active state.
 */
//-----------------------------------------------------------------------------------------------
void PCPC_PostInserterSetting_copy::procActivate()
{
	PNLC_CopyDataLib* pc_CopyDataLib = PNLC_CopyDataLib::newInstance();
	if ( NULL == pc_CopyDataLib ) {
		LOGF_debug_PutEventLog(LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "[PCPC_Paper_copy::procButtonOn]Assert Failed!");
		return;
	} 
	pc_CopyDataLib->setPIToggleStatus(True);
	PPopupDecisionButtonCom::procActivate();
}

#ifdef DEF_A4_Cus
//-----------------------------------------------------------------------------------------------
/**
 *	@brief	A020114100 選択トレイの設定変更 : 用紙種類ボタンID紐付けテーブルサイズの取得.
 */
//-----------------------------------------------------------------------------------------------
uchar PCPC_PostInserterSetting_copy::getTrayPaperKindBtnFromPaperTypeTblSize()
{
	uchar uc_TblSize = 0;
	TYPE_FeedTray e_trayno = getSelectedFeedTray();

	// 選択中のトレイが手差しの場合.
	if ( TYPE_FeedTrayManual == e_trayno )
	{
		uc_TblSize = uc_TrayManualPaperKindBtnFromPaperTypeNum;
	}		
	else if ( TYPE_FeedTray1 == e_trayno )
	{
		uc_TblSize = PNLD_Tbl_TrayPaperKindBtnFromPaperType_Tray1Num;
	} 
	else if ( TYPE_FeedTray2 == e_trayno ) 
	{
		uc_TblSize = PNLD_Tbl_TrayPaperKindBtnFromPaperType_Tray2Num;
	} 
	else if ( TYPE_FeedTray3 == e_trayno ) 
	{
		uc_TblSize = PNLD_Tbl_TrayPaperKindBtnFromPaperType_Tray3Num;
	} 
//comment 2023/07/25 FPT.LONGTH32 [IT6_2.3] 機器構成変更_No.75 Add tray 4 for Sparrow.
	else if ( TYPE_FeedTray4 == e_trayno ) 
	{
		uc_TblSize = PNLD_Tbl_TrayPaperKindBtnFromPaperType_Tray4Num;
	}
	else
	{
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
	}
	return uc_TblSize;
}

//-----------------------------------------------------------------------------------------------
/**
 *	@brief	A020114100 選択トレイの設定変更 : トレイ定義からトレイごとのテーブルインデックス取得.
 */
//-----------------------------------------------------------------------------------------------
uchar PCPC_PostInserterSetting_copy::getTrayTblIdxFromFeedTray( TYPE_FeedTray e_FeedTray )
{
	uchar uc_tblIdx=0;

	if ( TYPE_FeedTray1 == e_FeedTray ) {
		uc_tblIdx = 0;
	} else if ( TYPE_FeedTray2 == e_FeedTray ) {
		uc_tblIdx = 1;
	} else if ( TYPE_FeedTray3 == e_FeedTray ) {
		uc_tblIdx = 2;
//comment 2023/07/25 FPT.LONGTH32 [IT6_2.3] 機器構成変更_No.75 Add tray 4 for Sparrow.
	} else if ( TYPE_FeedTray4 == e_FeedTray ) {
		uc_tblIdx = 3;
	} else {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
	}

	return uc_tblIdx;
}
#endif // DEF_A4_Cus.
// comment 2016/05/19 SCD2-2G-Sky福田 [ZeusSBK][CQ529248] TBD230対応.
//-----------------------------------------------------------------------------------------------
/**
 *	@brief	A020114100 選択トレイの設定変更 : 定型用紙サイズ表示.
 */
//-----------------------------------------------------------------------------------------------
void PCPC_PostInserterSetting_copy::setupStandardPaperSize()
{
	APIC_FeedTray* pc_FeedTray = PnlComGetFeedTray();
	
	if( NULL == pc_FeedTray ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "[PCPC_PostInserterSetting_copy::setupStandardPaperSize]Assert Failed!" );
		return;
	}
	
	short ss_SizePosX = 0;
	short ss_SizePosY = 0;
	short ss_DirPosX = 0;
	short ss_DirPosY = 0;

	// 用紙サイズの表示座標取得.
	Txt_PIDispPaperSize1Ex.getPosition( ss_SizePosX, ss_SizePosY );
	// 用紙向きの表示座標取得.
	Txt_PIDispPaperSizeDir1Ex.getPosition( ss_DirPosX, ss_DirPosY );

	const TYPE_FeedTray e_FeedTray = getSelectedFeedTray();
	// 用紙に関する情報（サイズ・種類・状態）を取得.
	TYPC_PaperSize c_PaperSize = pc_FeedTray->getPaperSize( e_FeedTray );
	if( TYPE_SizeCodeNonStandard == c_PaperSize.e_SizeCode ) {
		// 置き換え無し.
		Txt_PIDispPaperSizeDir1Ex.showEnable( False );
		return;
	}
	
	// ワイド紙の場合.
	if( True == c_PaperSize.b_WideOn ) {
		// 置き換え無し.
		Txt_PIDispPaperSizeDir1Ex.showEnable( False );
		return;
	}

	TEXT_ID us_SizeTextID = 0;
	TEXT_ID us_DirTextID = 0;
	short ss_DirPosX_Offset = 0;

	// FoolsCapサイズ.
	if( TYPE_SizeCodeFlsSef == c_PaperSize.e_SizeCode ) {
		switch( pc_MachineSetting->getFLSType() ) {
			case TYPE_FLSType85x135:	// 8_1/2inch x 13_1/2inch.
				us_SizeTextID = s_pNOTPaperSizeFoolscap_001_W056H036A;
				us_DirTextID = s_pNOTSef_W036H036A;
				ss_DirPosX_Offset = 36;
				break;
			case TYPE_FLSType220x330:	// 220mm x 330mm.
				us_SizeTextID = s_pNOTSettingConfirm_PaperSizeFoolscap_002_W324H020A;
				us_DirTextID = 0;
				break;
			case TYPE_FLSType85x13:		// 8_1/2inch x 13inch.
				us_SizeTextID = s_pNOTPaperSizeFoolscap_003_W056H036A;
				us_DirTextID = s_pNOTSef_W036H036A;
				ss_DirPosX_Offset = 36;
				break;
			case TYPE_FLSType825x13:	// 8_1/4inch x 13inch.
				us_SizeTextID = s_pNOTPaperSizeFoolscap_004_W056H036A;
				us_DirTextID = s_pNOTSef_W036H036A;
				ss_DirPosX_Offset = 36;
				break;
			case TYPE_FLSType8125x1325:	// 8_1/8inch x 13_1/4inch.
				us_SizeTextID = s_pNOTPaperSizeFoolscap_005_W056H036A;
				us_DirTextID = s_pNOTSef_W036H036A;
				ss_DirPosX_Offset = 36;
				break;
			case TYPE_FLSType8x13:		// 8inch x 13inch.
				us_SizeTextID = s_pNOTPaperSizeFoolscap_006_W056H036A;
				us_DirTextID = s_pNOTSef_W036H036A;
				ss_DirPosX_Offset = 36;
				break;
			default:
				// なにもしない.
				break;
		}
		
		// 用紙サイズ.
		Txt_PIDispPaperSize1Ex.changeText( us_SizeTextID );

		// 用紙向き.
		if( 0 != us_DirTextID ) {
			Txt_PIDispPaperSizeDir1Ex.showEnable( True );
			Txt_PIDispPaperSizeDir1Ex.changePosition( static_cast<short>( ss_SizePosX + ss_DirPosX_Offset + PNLD_PaperEnl_PaperSizeDir_CorrectX ), ss_DirPosY );
			Txt_PIDispPaperSizeDir1Ex.changeText( us_DirTextID );
		} else {
			Txt_PIDispPaperSizeDir1Ex.showEnable( False );
		}
		return;
	}
	
	for( uchar uc_index = 0; uc_index < ( sizeof(st_ReplacePaperSizeTable) / sizeof(PNLS_PaperEnl_copyReplaceTable) ); uc_index++ ) {
		// 用紙サイズ一致.
		if( c_PaperSize.e_SizeCode == st_ReplacePaperSizeTable[ uc_index ].e_SizeCode ) {
			if( TYPE_SizeCodeJpLetterY2 == c_PaperSize.e_SizeCode ) {
				if( TYPE_SystemType_JP != PnlComGetDestination() ) {
					us_SizeTextID = s_NOTenvC6;
				} else {
					us_SizeTextID = s_pKEYEnvY2_W096H028A;
				}
			} else {
				us_SizeTextID = st_ReplacePaperSizeTable[ uc_index ].us_DstTextID;
			}
			// 用紙サイズ.
			Txt_PIDispPaperSize1Ex.changeText( us_SizeTextID );
			// 用紙向き.
			if( 0 != st_ReplacePaperSizeTable[ uc_index ].us_DstDirTextID ) {
				Txt_PIDispPaperSizeDir1Ex.showEnable( True );
				Txt_PIDispPaperSizeDir1Ex.changePosition( static_cast<short>( ss_SizePosX + st_ReplacePaperSizeTable[ uc_index ].ss_PosX_Offset + PNLD_PaperEnl_PaperSizeDir_CorrectX ), ss_DirPosY );
				Txt_PIDispPaperSizeDir1Ex.changeText( st_ReplacePaperSizeTable[ uc_index ].us_DstDirTextID );
			} else {
				Txt_PIDispPaperSizeDir1Ex.showEnable( False );
			}
			break;
		}
	}
}

