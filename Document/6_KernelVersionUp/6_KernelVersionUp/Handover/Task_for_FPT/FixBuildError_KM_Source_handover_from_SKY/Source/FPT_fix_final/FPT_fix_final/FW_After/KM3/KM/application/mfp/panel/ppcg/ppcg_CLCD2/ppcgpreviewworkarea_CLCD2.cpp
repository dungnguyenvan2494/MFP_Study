/*================================================================================================
［タイトル］	プレビュー.
================================================================================================*/
/*================================================================================================
	インクルード・ファイル
================================================================================================*/
#include <panel/pdp/pdpg/pdpdebug.h>
#include <panel/ppcg/ppcg_CLCD/ppcgactivatableadd_CLCD.h>
#include <panel/ppcg/ppcg_CLCD2/ppcgsingletxt_CLCD2.h>
#include "ppcgpreviewworkarea_CLCD2.h"
#include <panel/pddgrap/pddgrap_CLCD/pddgrapg256_CLCD.h>
#include <panel/pddgrap/pddgrap_CLCD/pddgraptbl.h>
#include <panel/pddtext/pddtext_CLCD/pstr_CLCD.h>
#include <panel/pddtext/pddtext_CLCD/psstr_CLCD.h>
#include <panel/pddtext/pddtext_CLCD/pfstr_CLCD.h>
#include <panel/plibh/plibh_CLCD/plibh_CLCD.h>
// comment 2016/03/15 OPS1-Sky長尾芳人 [操作性改善][不要描画] 不要描画検出機構対応.
#include <panel/pnlmngr/pnlmngrg/PNLC_PartsAccessDebugLib.h>


/*================================================================================================
 マクロ関数の定義.
================================================================================================*/
#define  SIZE( table )			( sizeof( table ) / sizeof( PDisplayParts* ) )	// 部品登録数を得る.


#define PNLD_HideImagePostion_Shadow		 8								// 影表示分の移動量(2Grid).
#define PNLD_MarginStaplePunch				16								// ステープル・パンチ可能用紙 画像移動量(4Gird).
#define PNLD_MarginNotStaplePunch			 8								// ステープル・パンチ可能用紙 画像移動量(2Grid).
#define PNLD_StaplePosition					 6								// ステープル表示位置(用紙から1.5Grid).
#define PNLD_PunchPosition					( PNLD_StaplePosition + 4 )		// パンチ表示位置(ステープル表示位置から1Grid).
#define PNLD_StaplePucnhPositionDownAjust	4								// ステープル・パンチ下方向時の調整値.	// 伊丹Sky平野 2010/10/05 UD評価対応 No.49.
//2016/7/21 CQ532802 sky草柳 プレビューステープル用画像追加.
#ifdef DEF_A4_Cus
#define PNLD_StaplePositionAjust2	2
#define PNLD_StaplePositionAjust4	4
#endif

PSinglePartsResizableGraphic Gra_PreviewCompOriginalForm( tbl_c10_003_000_X228_Y152,	 80,	152,	TYPE_LayoutFree, False, PNLE_Shadow_Parts_4,	-1,	-1 );	// 原稿側:用紙.
PSinglePartsResizableGraphic Gra_PreviewCompCatalog( tbl_p00_260_000_X212_Y148,	 80,	152,	TYPE_LayoutFree, False, PNLE_Shadow_Parts_4,	-1,	-1 );	// 原稿側:カタログ原稿.
PSinglePartsResizableGraphic Gra_PreviewCompOriginalCardCopy( tbl_p00_267_000_X184_Y128,	 80,	152,	TYPE_LayoutFree, False, PNLE_Shadow_Parts_4,	-1,	-1 );	// 原稿側:カード原稿.
PSinglePartsResizableGraphic Gra_PreviewCompOriginalPageShot( tbl_p00_267_000_X184_Y128,	 80,	152,	TYPE_LayoutFree, False, PNLE_Shadow_Parts_4,	-1,	-1 );	// 原稿側:ブック原稿.

PSinglePartsGraphic Gra_PreviewCompOriginalImage( c10_003_000_X228_Y152,	 96,	168,	PNLD_PaletteCommon,	TYPE_LayoutFree,	False );

PSinglePartsResizableGraphic Gra_PreviewCompOriginalDuplexHide( tbl_p00_362_000_X79_Y79,	 80,	152,	TYPE_LayoutFree,	False, PNLE_Shadow_none,	-1,	-1 );		// 原稿側:両面影.
PSinglePartsResizableGraphic Gra_PreviewCompOriginalDuplexShadow( tbl_p00_442_000_X87_Y87,	 80,	152,	TYPE_LayoutFree, False, PNLE_Shadow_Parts_4,	-1,	-1 );			// 原稿側:部分影(右下両面影表示時のみ使用).

PSinglePartsResizableGraphic Gra_PreviewCompOriginalDuplex( tbl_p00_200_000_X212_Y148,	 80,	152,	TYPE_LayoutFree, False, PNLE_Shadow_none,	-1,	-1 );	// 原稿側:両面パーツ.

PSinglePartsGraphic Gra_PreviewCompOriginalIcon( p00_317_000_X32_Y32,		 40,	152,	PNLD_PaletteCommon,	TYPE_LayoutFree,	False );

PSinglePartsGraphic Gra_PreviewCompOriginalIcon2( p00_317_000_X32_Y32,		 40,	192,	PNLD_PaletteCommon,	TYPE_LayoutFree,	False );	// 原稿用アイコン2.
PSinglePartsGraphic Gra_PreviewCompOriginalIcon3( p00_317_000_X32_Y32,		 40,	232,	PNLD_PaletteCommon,	TYPE_LayoutFree,	False );	// 原稿用アイコン3.
PSinglePartsGraphic Gra_PreviewCompOriginalIcon4( p00_317_000_X32_Y32,		 40,	272,	PNLD_PaletteCommon,	TYPE_LayoutFree,	False );	// 原稿用アイコン4.

PSinglePartsResizableGraphic Gra_PreviewCompOutputForm( tbl_c10_003_000_X228_Y152,	 396,	152,	TYPE_LayoutFree, False, PNLE_Shadow_Parts_4,	-1,	-1 );	// 出力側:用紙.

PSinglePartsGraphic Gra_PreviewCompOutputImage( c10_003_000_X228_Y152,	412,	168,	PNLD_PaletteCommon,	TYPE_LayoutFree,	False );
PSinglePartsResizableGraphic Gra_PreviewCompOutputDuplexHide( tbl_p00_362_000_X79_Y79,	 80,	152,	TYPE_LayoutFree,	False,	PNLE_Shadow_none,	-1,	-1 );	// 出力側：両面影.

PSinglePartsResizableGraphic Gra_PreviewCompOutputDuplexShadow( tbl_p00_442_000_X87_Y87,	396,	152,	TYPE_LayoutFree, False,	PNLE_Shadow_Parts_4,	-1,	-1 );	// 出力側:部分影(右下両面影表示時のみ使用).

PSinglePartsResizableGraphic Gra_PreviewCompOutputDuplex( tbl_p00_200_000_X212_Y148,	396,	152,	TYPE_LayoutFree, False, PNLE_Shadow_none,	-1,	-1 );	// 出力側:両面パーツ.

PSinglePartsResizableGraphic Gra_PreviewCompMultiPage( tbl_p00_299_000_X228_Y152,	 396,	152,	TYPE_LayoutFree, False, PNLE_Shadow_Parts_4,	-1,	-1 );	// 出力側:拡大連写.
PSinglePartsResizableGraphic Gra_PreviewCompMultiPageArrow( tbl_p00_529_000_X228_Y152,	 396,	152,	TYPE_LayoutFree, False, PNLE_Shadow_none,	-1,	-1 );		// 出力側:拡大連写矢印.
PSinglePartsResizableGraphic Gra_PreviewCompCardCopy( tbl_p00_300_000_X228_Y152,	 396,	152,	TYPE_LayoutFree, False, PNLE_Shadow_none,	-1,	-1 );		// 出力側:カードコピー.
PSinglePartsResizableGraphic Gra_PreviewCompmMagazin( tbl_p00_283_000_X212_Y148,	 396,	152,	TYPE_LayoutFree, False, PNLE_Shadow_Parts_4,	-1,	-1 );	// 出力側:小冊子.


PSinglePartsGraphic Gra_PreviewCompOutputFold( c10_003_000_X228_Y152,	396,	152,	PNLD_PaletteCommon,	TYPE_LayoutFree,	False );
PSinglePartsGraphic Gra_PreviewCompOutputStaple( c10_003_000_X228_Y152,	396,	152,	PNLD_PaletteCommon,	TYPE_LayoutFree,	False );
PSinglePartsGraphic Gra_PreviewCompOutputPunch( c10_003_000_X228_Y152,	396,	152,	PNLD_PaletteCommon,	TYPE_LayoutFree,	False );
PSinglePartsGraphic Gra_PreviewCompOutputColorIcon( p00_341_000_X32_Y32,		632,	152,	PNLD_PaletteCommon,	TYPE_LayoutFree,	False );
PSinglePartsGraphic Gra_PreviewCompOutputFoldIcon( p00_321_000_X32_Y32,		632,	192,	PNLD_PaletteCommon,	TYPE_LayoutFree,	False );
PSinglePartsGraphic Gra_PreviewCompOutputRepeatIcon( p00_323_000_X64_Y32,		632,	232,	PNLD_PaletteCommon,	TYPE_LayoutFree,	False );
PSinglePartsResizableGraphic Gra_PreviewCompOriginalGuide( tbl_c10_003_000_X228_Y152,	 80,	152,	TYPE_LayoutFree, False, PNLE_Shadow_none,	228,	152 );
PSinglePartsResizableGraphic Gra_PreviewCompOutputGuide( tbl_c10_003_000_X228_Y152,	396,	152,	TYPE_LayoutFree, False, PNLE_Shadow_none,	228,	152 );
PSinglePartsResizableGraphic Gra_PreviewCompArrow( tbl_c10_004_000_X16_Y32,		344,	212,	TYPE_LayoutFree, False, PNLE_Shadow_none,	 16,	 32 );

PSinglePartsTextEx	Txt_PreviewCompOriginal( s_pITMOriginal_W224H024E,80, 124, 224, 24, UC_GPARTS_COLOR_GRAY5,UC_GPARTS_COLOR_GRAY5 , TYPE_LayoutLeftAdjust, TYPE_VLayoutMiddle, PNLE_SELECT_FONT_E, 0 );
PSinglePartsTextEx	Txt_PreviewCompOutput( s_pITMOutput_W224H024E, 396, 124, 224, 24, UC_GPARTS_COLOR_GRAY5, UC_GPARTS_COLOR_GRAY5, TYPE_LayoutLeftAdjust, TYPE_VLayoutMiddle, PNLE_SELECT_FONT_E, 0 );

static PSinglePartsResizableGraphic Gra_PreviewCompOrigComFlame( tbl_p00_921_000_X228_Y152,	 80,	152,	TYPE_LayoutFree, False, PNLE_Shadow_Parts_4 );	// 原稿_枠.
static PSinglePartsGraphic			Gra_PreviewCompOrigComImage( p00_887_000_4C_X196_Y120,		 96,	168,	PNLD_PaletteCommon,	TYPE_LayoutFree, False	);	// 原稿_画像.
static PSinglePartsResizableGraphic Gra_PreviewCompOutputComFlame( tbl_p00_920_000_X228_Y152,	396,	152,	TYPE_LayoutFree, False, PNLE_Shadow_Parts_4 );	// 出力_枠.
static PSinglePartsGraphic			Gra_PreviewCompOutputComImage( p00_887_000_4C_X196_Y120,		396,	152,	PNLD_PaletteCommon,	TYPE_LayoutFree, False	);	// 出力_画像.
static PSinglePartsResizableGraphic Gra_PreviewCompOutputComFinishing( tbl_p00_881_000_X228_Y152,	396,	152 );	// 出力_仕上がり.
PSinglePartsGraphic Gra_PreviewCompOutputComFinishing2( c10_003_000_X228_Y152,	396,	152, PNLD_PaletteCommon,	TYPE_LayoutFree,	False );	// 出力_仕上がり(2つ目). // comment CQ356131 ステータス表示エリア対応(ステープルが表示されない問題) 2011/09/30 酒井(健).
static PSinglePartsResizableGraphic Gra_PreviewCompOutputComIrregularIcon( tbl_p00_200_000_X212_Y148,	396,	152 );	// 出力_欠損.

PDisplayParts* const tbl_PreviewWorkArea []= {
	&Txt_PreviewCompOriginal,
	&Gra_PreviewCompOriginalGuide,
	&Gra_PreviewCompOriginalForm,
	&Gra_PreviewCompOriginalImage,
	&Gra_PreviewCompOrigComFlame,				// 原稿_枠.
	&Gra_PreviewCompOrigComImage,				// 原稿_画像.
	&Gra_PreviewCompOriginalDuplexHide,
	&Gra_PreviewCompOriginalDuplexShadow,	// 原稿側:部分影(右下両面影表示時のみ使用).
	&Gra_PreviewCompOriginalDuplex,
	&Gra_PreviewCompOriginalIcon,
	&Gra_PreviewCompArrow,
	&Txt_PreviewCompOutput,
	&Gra_PreviewCompOutputGuide,
	&Gra_PreviewCompOutputForm,
	&Gra_PreviewCompOutputImage,
	&Gra_PreviewCompOutputComFlame,				// 出力_枠.
	&Gra_PreviewCompOutputComImage,				// 出力_画像.
	&Gra_PreviewCompOutputDuplexHide,
	&Gra_PreviewCompOutputDuplexShadow,	// 出力側:部分影(右下両面影表示時のみ使用).
	&Gra_PreviewCompOutputDuplex,
	&Gra_PreviewCompOutputFold,
	&Gra_PreviewCompOutputStaple,
	&Gra_PreviewCompOutputPunch,
	&Gra_PreviewCompOutputColorIcon,
	&Gra_PreviewCompOutputFoldIcon,
	&Gra_PreviewCompOutputRepeatIcon,
	&Gra_PreviewCompOutputComFinishing,			// 出力_仕上がり.
	&Gra_PreviewCompOutputComFinishing2,		// 出力_仕上がり(2つ目).
	&Gra_PreviewCompOutputComIrregularIcon,		// 出力_欠損.
	&Gra_PreviewCompOriginalIcon2,		// 原稿用アイコン2.
	&Gra_PreviewCompOriginalIcon3,		// 原稿用アイコン3.
	&Gra_PreviewCompOriginalIcon4,		// 原稿用アイコン4.

};
PPreviewWorkArea Key_PreviewWorkArea( ( PDisplayParts** ) tbl_PreviewWorkArea, SIZE( tbl_PreviewWorkArea ) );

// 出力側アイコン表示位置テーブル.
static PNLS_IconPosition st_IconOutputTbl[] =
{
	{ NULL,	632,	152 },
	{ NULL,	632,	192 },
	{ NULL,	632,	232 },
	{ NULL,	632,	272 },
};
const uchar uc_SizeIconPositionTbl = ( uchar )( sizeof( st_IconOutputTbl ) / sizeof( PNLS_IconPosition ) );

// PPreviewWorkArea staticメンバ.
const short	PPreviewWorkArea::m_ss_shiftPreviewPartAmount = 285;	// プレビュー画像のシフト量.

/*================================================================================================
［機能］	コンストラクタ.
--------------------------------------------------------------------------------------------------
［引数］	なし
［戻値］	なし
================================================================================================*/
PPreviewWorkArea::PPreviewWorkArea( PDisplayParts** ppc_AddParts,uchar uc_PartsCount )
	: PGroupParts( ppc_AddParts, uc_PartsCount )
	, ps_mParts( ppc_AddParts )
	, m_ss_OutputIconCount( 0 )					// 出力表示アイコン表示数.
	, m_e_PreviewMode( PNLE_PreviewModeCopy )		// プレビュー表示アプリ種別.
	, m_b_shiftPreviewPartJudge( False )			// プレビュー画像シフト判定フラグ.
{
	m_ss_PreviewIconNormalPosX = st_IconOutputTbl[0].ss_PosX;	// プレビューアイコンの通常表示位置.
}

/*================================================================================================
［機能］	プレビュー画像を非表示の設定.
--------------------------------------------------------------------------------------------------
［引数］	なし
［戻値］	なし
================================================================================================*/
void PPreviewWorkArea::resetDisplay()
{
	DEF_UPDATEDRAWPARTSINFO;		// comment 2016/03/30 OPS1-Sky長尾芳人 [操作性改善][不要描画] 不要描画検出機構対応.
	
	// 全パーツ非表示にする.
	for( uchar i = 0; i < uc_PartsNumber; i++ ) {
		ps_mParts[i]->showEnable( False );
	}

	Bool b_always10keyDispFlag = APIC_OperationSetting::newInstance()->getSoftware10KeyAppears();

	if( True == b_always10keyDispFlag && False == m_b_shiftPreviewPartJudge ) {
		// プレビュー画像を左側へずらす.
		for( int i = 0; i < uc_PartsNumber; i++ ) {
			ps_mParts[i]->shiftPosition( -m_ss_shiftPreviewPartAmount,0,False );
		}
		// 出力アイコンを左側へずらす.
		for( int i = 0; i < uc_SizeIconPositionTbl; i++ ) {
			st_IconOutputTbl[i].ss_PosX = (short)( m_ss_PreviewIconNormalPosX - m_ss_shiftPreviewPartAmount );
		}
		m_b_shiftPreviewPartJudge = True;
	} else if( False == b_always10keyDispFlag && True == m_b_shiftPreviewPartJudge ) {
		// プレビュー画像を元の表示位置に戻す.
		for( int i = 0; i < uc_PartsNumber; i++ ) {
			ps_mParts[i]->shiftPosition( m_ss_shiftPreviewPartAmount,0,False );
		}
		// 出力アイコンを元の表示位置に戻す.
		for( int i = 0; i < uc_SizeIconPositionTbl; i++ ) {
			st_IconOutputTbl[i].ss_PosX = m_ss_PreviewIconNormalPosX;
		}
		m_b_shiftPreviewPartJudge = False;
	}

	// 以下のパーツは、表示.
	if( m_e_PreviewMode == PNLE_PreviewModeCopy && False == b_always10keyDispFlag ) {	// 原稿側はコピー時かつソフト10キー「部数押下時」設定時のみ　「常時」設定時は原稿プレビューを表示しない.
		ps_mParts[PNLE_PreviewWorkAreaOrigText]->showEnable( True );	// 原稿.
		ps_mParts[PNLE_PreviewWorkAreaOrigGuide]->showEnable( True );	// 原稿側:プレビュー表示エリア.
		ps_mParts[PNLE_PreviewWorkAreaArrow]->showEnable( True );		// 矢印.
	}
	ps_mParts[PNLE_PreviewWorkAreaOutputText]->showEnable( True );	// 出力.
	ps_mParts[PNLE_PreviewWorkAreaOutputGuide]->showEnable( True );	// 出力側:プレビュー表示エリア.
}


/*================================================================================================
［機能］	初期設定.
--------------------------------------------------------------------------------------------------
［引数］	なし
［戻値］	なし
================================================================================================*/
void PPreviewWorkArea::initPreviewWorkArea( PNLE_PreviewMode e_PreviewMode )
{
	DEF_UPDATEDRAWPARTSINFO;		// comment 2016/03/30 OPS1-Sky長尾芳人 [操作性改善][不要描画] 不要描画検出機構対応.
	
	PSinglePartsResizableGraphic* cp_origGuide = dynamic_cast<PSinglePartsResizableGraphic*>( ps_mParts[PNLE_PreviewWorkAreaOrigGuide] );
	PSinglePartsResizableGraphic* cp_outputGuide = dynamic_cast<PSinglePartsResizableGraphic*>( ps_mParts[PNLE_PreviewWorkAreaOutputGuide] );
	PSinglePartsResizableGraphic* cp_arrow = dynamic_cast<PSinglePartsResizableGraphic*>( ps_mParts[PNLE_PreviewWorkAreaArrow] );
	if( NULL == cp_origGuide || NULL == cp_outputGuide || NULL == cp_arrow ) {
		PnlComPutDebugLog( True, UC_PNL_LOGPATTERN_1, "PPreviewWorkArea::initPreviewWorkArea", UC_PNL_ERROR_USE );
		return;
	}
	if( PNLE_PreviewModeCopy == e_PreviewMode ) {
		m_e_PreviewMode = PNLE_PreviewModeCopy;
		cp_origGuide->changeParts( tbl_c10_003_000_X228_Y152 );						// 原稿プレビューガイドライン ( Box画面非表示 ).
		cp_outputGuide->changeParts( tbl_c10_003_000_X228_Y152 );					// 出力プレビューガイドライン.
		cp_arrow->changeParts( tbl_c10_004_000_X16_Y32 );							// 矢印 ( Box画面非表示 ).
	} else {
		m_e_PreviewMode = PNLE_PreviewModeBox;
		cp_origGuide->changeParts( tbl_b10_003_000_X228_Y152 );						// 原稿プレビューガイドライン ( Box画面非表示 ).
		cp_outputGuide->changeParts( tbl_b10_003_000_X228_Y152 );					// 出力プレビューガイドライン.
		cp_arrow->changeParts( tbl_b10_004_000_X16_Y32 );							// 矢印 ( Box画面非表示 ).
	}

}
/*================================================================================================
［機能］	Duplex画像を表示の設定
--------------------------------------------------------------------------------------------------
［引数］	uc_previewPart				原稿か出力使用
			s_duplexParts				Duplex画像
			s_duplexHideParts			下地を隠す領域画像
			s_duplexShadowParts			両面部分影(右下めくり専用).
［戻値］	なし
================================================================================================*/
void PPreviewWorkArea::setDuplexImage( uchar uc_previewPart, PNLS_Parts s_duplexParts[PNLD_PartsNum], PNLS_Parts s_duplexHideParts[PNLD_PartsNum], PNLE_DuplexHideImagePositionType e_HideImagePosType, PNLS_Parts s_DuplexShadowParts[PNLD_PartsNum] )
{
	DEF_UPDATEDRAWPARTSINFO;		// comment 2016/03/30 OPS1-Sky長尾芳人 [操作性改善][不要描画] 不要描画検出機構対応.
	
	short ss_PaperPosX = 0;
	short ss_PaperPosY = 0;
	short ss_PaperWidth = 0;
	short ss_PaperHeight = 0;
	short ss_HidePosX = 0;
	short ss_HidePosY = 0;
	short ss_HideWidth = 0;
	short ss_HideHeight = 0;

	// 初期化.
	PNLE_PreviewWorkAreaPartsIndex duplexImageIndex = PNLE_PreviewWorkAreaOrigDuplex;
	PNLE_PreviewWorkAreaPartsIndex duplexHideImageIndex = PNLE_PreviewWorkAreaOrigDuplexHide;
	PNLE_PreviewWorkAreaPartsIndex duplexShadowIndex = PNLE_PreviewWorkAreaOrigDuplexShadow;	// 部分影.

	// 出力を使用の場合.
	if( PPC_PreviewOutputIndex == uc_previewPart ) {
		// 出力Duplexインデックスをセット.
		duplexImageIndex = PNLE_PreviewWorkAreaOutputDuplex;
		duplexHideImageIndex = PNLE_PreviewWorkAreaOutputDuplexHide;
		duplexShadowIndex = PNLE_PreviewWorkAreaOutputDuplexShadow;	// 部分影.
	}
	// 表示パーツを取得.
	PSinglePartsResizableGraphic* origDuplex = dynamic_cast<PSinglePartsResizableGraphic*>( ps_mParts[duplexImageIndex] );		// 両面パーツ.
	PSinglePartsResizableGraphic* cp_HideImage = dynamic_cast<PSinglePartsResizableGraphic*>( ps_mParts[duplexHideImageIndex] );	// 両面影.
	PSinglePartsResizableGraphic* cp_DuplexShadow = dynamic_cast<PSinglePartsResizableGraphic*>( ps_mParts[duplexShadowIndex] );	// 部分影.
	if( NULL == origDuplex || NULL == cp_HideImage || NULL == cp_DuplexShadow ) {
		PnlComPutDebugLog( True, UC_PNL_LOGPATTERN_1, "PPreviewWorkArea::setDuplexImage", UC_PNL_ERROR_USE );
		return;
	}
	// Duplex画像をセット.
	origDuplex->changeParts( s_duplexParts );
	cp_HideImage->changeParts( s_duplexHideParts );
	cp_DuplexShadow->changeParts( s_DuplexShadowParts );	// 部分影.

	// 下地を隠す領域画像の位置を計算.
	origDuplex->getPosition( ss_PaperPosX, ss_PaperPosY );
	origDuplex->getSize( ss_PaperWidth, ss_PaperHeight );
	cp_HideImage->getSize( ss_HideWidth, ss_HideHeight );


	switch( e_HideImagePosType ) {
		case PNLE_DuplexHideImagePositionType_LeftDown:			// 左下.
			ss_HidePosX = ss_PaperPosX;
			ss_HidePosY = (short)( ss_PaperPosY + ss_PaperHeight - ss_HideHeight + PNLD_HideImagePostion_Shadow );
			break;
		case PNLE_DuplexHideImagePositionType_LeftUp:			// 左上.
			ss_HidePosX = ss_PaperPosX;
			ss_HidePosY = ss_PaperPosY;
			break;
		case PNLE_DuplexHideImagePositionType_RightDown:		// 右下.
			ss_HidePosX = (short)( ss_PaperPosX + ss_PaperWidth - ss_HideWidth + PNLD_HideImagePostion_Shadow );
			ss_HidePosY = (short)( ss_PaperPosY + ss_PaperHeight - ss_HideHeight + PNLD_HideImagePostion_Shadow );
			break;
		case PNLE_DuplexHideImagePositionType_RightUp:			// 右上.
			ss_HidePosX = (short)( ss_PaperPosX + ss_PaperWidth - ss_HideWidth + PNLD_HideImagePostion_Shadow );
			ss_HidePosY = ss_PaperPosY;
			break;
		default:
			break;
	}
	// 下地を隠す領域画像の表示位置設定.
	cp_HideImage->changePosition( ss_HidePosX, ss_HidePosY );

	// 表示パーツを表示.
	cp_HideImage->showEnable( True );

	if( e_HideImagePosType == PNLE_DuplexHideImagePositionType_RightDown ) {	// 影下地の位置が右下の時.
		// 部分影、有効領域設定.
		cp_DuplexShadow->changePosition( (short)( ss_HidePosX - PNLD_HideImagePostion_Shadow ), (short)( ss_HidePosY - PNLD_HideImagePostion_Shadow ) );
		cp_DuplexShadow->showEnable( True );
	}

	origDuplex->showEnable( True );
}
/*================================================================================================
［機能］	アイコン表示の設定
--------------------------------------------------------------------------------------------------
［引数］	uc_previewPart				原稿か出力使用
			uc_manuscriptIcon			原稿タイプアイコン
			uc_colorIcon				カラーアイコン
			uc_foldIcon					折りアイコン
			uc_repeatIcon				リピートアイコン
			uc_manuscriptIcon2			原稿タイプアイコン2.
［戻値］	なし
================================================================================================*/
void PPreviewWorkArea::showIcon( uchar uc_previewPart, uchar* uc_manuscriptIcon, uchar* uc_colorIcon, uchar* uc_foldIcon, uchar* uc_repeatIcon, uchar* uc_manuscriptIcon2 )
{
	DEF_UPDATEDRAWPARTSINFO;		// comment 2016/03/30 OPS1-Sky長尾芳人 [操作性改善][不要描画] 不要描画検出機構対応.
	
	m_ss_OutputIconCount	= 0;						// 出力表示アイコン表示数.
	// 原稿表示の場合.
	if( PPC_PreviewOriginalIndex == uc_previewPart ) {
		// 原稿アイコンパーツを取得.
		PSinglePartsGraphic* manuscriptIconPart = dynamic_cast<PSinglePartsGraphic*>( ps_mParts[PNLE_PreviewWorkAreaOrigIcon] );
		PSinglePartsGraphic* manuscriptIconPart2 = dynamic_cast<PSinglePartsGraphic*>( ps_mParts[PNLE_PreviewWorkAreaOrigIcon2] );
		// 原稿アイコンのNULLチェック.
		if( NULL != uc_manuscriptIcon ) {
			if( NULL != manuscriptIconPart ) {
				// 原稿アイコン画像をセット.
				manuscriptIconPart->changeGraphic( uc_manuscriptIcon );
				// 原稿アイコンパーツを表示.
				manuscriptIconPart->showEnableShow( True );
			}
		}
		if( NULL != uc_manuscriptIcon2 ) {
			if( NULL != manuscriptIconPart2 ) {
				// 原稿アイコン画像をセット.
				manuscriptIconPart2->changeGraphic( uc_manuscriptIcon2 );
				// 原稿アイコンパーツを表示.
				manuscriptIconPart2->showEnableShow( True );
			}
		}
		// 出力表示の場合.
	} else {
		// カラーアイコンパーツを取得.
		PSinglePartsGraphic* colorIconPart = dynamic_cast<PSinglePartsGraphic*>( ps_mParts[PNLE_PreviewWorkAreaColorIcon] );
		// 折りアイコンパーツを取得.
		PSinglePartsGraphic* foldIconPart = dynamic_cast<PSinglePartsGraphic*>( ps_mParts[PNLE_PreviewWorkAreaFoldIcon] );
		// リピートアイコンパーツを取得.
		PSinglePartsGraphic* repeatIconPart = dynamic_cast<PSinglePartsGraphic*>( ps_mParts[PNLE_PreviewWorkAreaRepeatIcon] );

		// カラーアイコンのNULLチェック.
		if( NULL != uc_colorIcon ) {
			if( NULL != colorIconPart ) {
				// カラーアイコン画像をセット.
				colorIconPart->changeGraphic( uc_colorIcon );
			}
			// カラーアイコンパーツを表示.
			setupShowIcon( colorIconPart, st_IconOutputTbl, m_ss_OutputIconCount );

		} else {
			// カラーアイコンパーツを非表示.
			setupHideIcon( colorIconPart, st_IconOutputTbl, m_ss_OutputIconCount );
		}
		// 折りアイコンのNULLチェック.
		if( NULL != uc_foldIcon ) {
			if( NULL != foldIconPart ) {
				// 折りアイコン画像をセット.
				foldIconPart->changeGraphic( uc_foldIcon );
			}
			// 折りアイコンパーツを表示.
			setupShowIcon( foldIconPart, st_IconOutputTbl, m_ss_OutputIconCount );
		} else {
			// 折りアイコンパーツを非表示.
			setupHideIcon( foldIconPart, st_IconOutputTbl, m_ss_OutputIconCount );
		}
		// リピートアイコンのNULLチェック.
		if( NULL != uc_repeatIcon ) {
			if( NULL != repeatIconPart ) {
				// リピートアイコン画像をセット.
				repeatIconPart->changeGraphic( uc_repeatIcon );
			}
			// リピートアイコンパーツを表示.
			setupShowIcon( repeatIconPart, st_IconOutputTbl, m_ss_OutputIconCount );
		} else {
			// リピートアイコンパーツを非表示.
			setupHideIcon( repeatIconPart, st_IconOutputTbl, m_ss_OutputIconCount );
		}
	}
}

/*================================================================================================
［機能］	用紙画像を表示の設定.
--------------------------------------------------------------------------------------------------.
［引数］	uc_previewPart				原稿か出力使用.
			uc_mainImage				用紙画像.
［戻値］	なし.
================================================================================================*/
void PPreviewWorkArea::setPaperForm( uchar uc_previewPart, PNLS_Parts s_Parts[PNLD_PartsNum] )
{
	DEF_UPDATEDRAWPARTSINFO;		// comment 2016/03/30 OPS1-Sky長尾芳人 [操作性改善][不要描画] 不要描画検出機構対応.
	
	// 初期化.
	PNLE_PreviewWorkAreaPartsIndex mainImageIndex = ( PPC_PreviewOutputIndex == uc_previewPart ? PNLE_PreviewWorkAreaOutputForm : PNLE_PreviewWorkAreaOrigForm );

	// 表示パーツを取得.
	PSinglePartsResizableGraphic* origPaper = dynamic_cast<PSinglePartsResizableGraphic*>( ps_mParts[mainImageIndex] );
	if( NULL != origPaper ) {
		// 用紙画像をセット.
		origPaper->changeParts( s_Parts );
		// 表示パーツを表示.
		origPaper->showEnableShow( True );
	}
}

/*================================================================================================
［機能］	用紙画像を表示の設定.
--------------------------------------------------------------------------------------------------.
［引数］	uc_previewPart				原稿か出力使用.
			uc_mainImage				用紙画像.
			uc_Palet					パレット情報.
［戻値］	なし.
================================================================================================*/
void PPreviewWorkArea::setMainImage( uchar uc_previewPart, uchar* uc_mainImage, uchar uc_Palet )
{
	DEF_UPDATEDRAWPARTSINFO;		// comment 2016/03/30 OPS1-Sky長尾芳人 [操作性改善][不要描画] 不要描画検出機構対応.
	
	// 初期化.
	PNLE_PreviewWorkAreaPartsIndex mainImageIndex = PNLE_PreviewWorkAreaOrigPaper;

	// 出力を使用の場合.
	if( PPC_PreviewOutputIndex == uc_previewPart ) {
		// 出力用紙インデックスをセット.
		mainImageIndex = PNLE_PreviewWorkAreaOutputPaper;
	}

	// 表示パーツを取得.
	PSinglePartsGraphic* origPaper = dynamic_cast<PSinglePartsGraphic*>( ps_mParts[mainImageIndex] );
	if( NULL != origPaper ) {
		// 用紙画像をセット.
		origPaper->changeGraphic( uc_mainImage, uc_Palet, True );
		// 表示パーツを表示.
		origPaper->showEnableShow( True );
	}


}
/*================================================================================================
［機能］	出力画像の表示位置を設定.
--------------------------------------------------------------------------------------------------.
［引数］	uc_previewPart				原稿か出力使用.
			b_StaplePunchOK				ステープル,パンチ可能フラグ.
［戻値］	なし.
================================================================================================*/
void PPreviewWorkArea::setImagePosition( uchar uc_previewPart, Bool b_StaplePunchOK )
{
	DEF_UPDATEDRAWPARTSINFO;		// comment 2016/03/30 OPS1-Sky長尾芳人 [操作性改善][不要描画] 不要描画検出機構対応.
	
	short ss_ImagePositionX = 0;
	short ss_ImagePositionY = 0;
	// 初期化.
	PNLE_PreviewWorkAreaPartsIndex mainImageIndex = PNLE_PreviewWorkAreaOrigPaper;
	PNLE_PreviewWorkAreaPartsIndex formIndex = uc_previewPart == PPC_PreviewOutputIndex ? PNLE_PreviewWorkAreaOutputForm :  PNLE_PreviewWorkAreaOrigForm;	// 用紙の下地.

	// 下地（白い用紙）パーツを取得.
	PSinglePartsResizableGraphic* form = dynamic_cast<PSinglePartsResizableGraphic*>( ps_mParts[formIndex] );
	if( NULL == form ) {
		PnlComPutDebugLog( True, UC_PNL_LOGPATTERN_1, "PPreviewWorkArea::setImagePosition", UC_PNL_ERROR_USE );
		return;
	}
	short ss_PaperPosX = 0;
	short ss_PaperPosY = 0;
	short ss_PaperWidth = 0;
	short ss_PaperHeight = 0;

	short ss_Margin = 0;

	// 下地の表示位置、サイズを取得.
	form->getPosition( ss_PaperPosX,ss_PaperPosY );
	form->getSize( ss_PaperWidth,ss_PaperHeight );

	// 原稿表示の場合.
	if( PPC_PreviewOriginalIndex == uc_previewPart ) {
		// 原稿用紙インデックスをセット.
		mainImageIndex = PNLE_PreviewWorkAreaOrigPaper;

		if( True == b_StaplePunchOK ) {
			// 4grid開始位置をずらす.
			ss_ImagePositionX = (short)( ss_PaperPosX + PNLD_MarginStaplePunch );
			ss_ImagePositionY = (short)( ss_PaperPosY + PNLD_MarginStaplePunch );
			ss_Margin = PNLD_MarginStaplePunch;

		} else {
			// 2grid開始位置をずらす.
			ss_ImagePositionX = (short)( ss_PaperPosX + PNLD_MarginNotStaplePunch );
			ss_ImagePositionY = (short)( ss_PaperPosY + PNLD_MarginNotStaplePunch );
			ss_Margin = PNLD_MarginNotStaplePunch;
		}
		// 出力表示の場合.
	} else {
		// 出力用紙インデックスをセット.
		mainImageIndex = PNLE_PreviewWorkAreaOutputPaper;

		if( True == b_StaplePunchOK ) {
			// 4grid開始位置をずらす.
			ss_ImagePositionX = (short)( ss_PaperPosX + PNLD_MarginStaplePunch );
			ss_ImagePositionY = (short)( ss_PaperPosY + PNLD_MarginStaplePunch );
			ss_Margin = PNLD_MarginStaplePunch;
		} else {
			// 2grid開始位置をずらす.
			ss_ImagePositionX = (short)( ss_PaperPosX + PNLD_MarginNotStaplePunch );
			ss_ImagePositionY = (short)( ss_PaperPosY + PNLD_MarginNotStaplePunch );
			ss_Margin = PNLD_MarginNotStaplePunch;
		}
	}
	// 表示パーツを取得.
	PSinglePartsGraphic* origPaper = dynamic_cast<PSinglePartsGraphic*>( ps_mParts[ mainImageIndex ] );
	if( NULL == origPaper ) {
		PnlComPutDebugLog( True, UC_PNL_LOGPATTERN_1, "PPreviewWorkArea::setImagePosition", UC_PNL_ERROR_USE );
		return;
	}
	// 画像表示位置設定.
	origPaper->changePosition( ss_ImagePositionX, ss_ImagePositionY );

	// 画像表示位置の表示エリア指定.
	PNLS_ClipArea s_ClipArea;	// クリッピングエリア.
	s_ClipArea.ss_XPos 		= ss_ImagePositionX;
	s_ClipArea.ss_YPos 		= ss_ImagePositionY;
	s_ClipArea.ss_Width 	= (short)( ss_PaperWidth - ss_Margin );
	s_ClipArea.ss_Height 	= (short)( ss_PaperHeight - ss_Margin );

	// 画像表示位置を指定する.
	// 拡大ではみ出す部分は、切れた状態で出す.
	origPaper->setClipArea( s_ClipArea );


}
//==============================================================================.
// プレビューコンポーネント:拡大連写表示処理.
//==============================================================================.
void PPreviewWorkArea::setMultiPageMode()
{
	DEF_UPDATEDRAWPARTSINFO;		// comment 2016/03/30 OPS1-Sky長尾芳人 [操作性改善][不要描画] 不要描画検出機構対応.
	
	PSinglePartsResizableGraphic* multipage = dynamic_cast<PSinglePartsResizableGraphic*>( ps_mParts[PNLE_PreviewWorkAreaMultiPage] );				// 拡大連写.
	PSinglePartsResizableGraphic* multipageArrow = dynamic_cast<PSinglePartsResizableGraphic*>( ps_mParts[PNLE_PreviewWorkAreaMultiPageArrow] );		// 拡大連写矢印.
	if( NULL != multipage ) {
		multipage->showEnable( True );		// 拡大連写.
	}
	if( NULL != multipageArrow ) {
		multipageArrow->showEnable( True );	// 拡大連写矢印.
	}
}

//==============================================================================.
// カタログ原稿表示処理.
//==============================================================================.
void PPreviewWorkArea::setCatalogImage( PNLS_Parts s_PartsFlame[PNLD_PartsNum], PNLS_Parts s_PartsImage[PNLD_PartsNum], short ss_StartPosY, short ss_AdjustSizeX, short ss_AdjustSizeY )
{
	DEF_UPDATEDRAWPARTSINFO;		// comment 2016/03/30 OPS1-Sky長尾芳人 [操作性改善][不要描画] 不要描画検出機構対応.
	
	PSinglePartsResizableGraphic* catalog_Flame = dynamic_cast<PSinglePartsResizableGraphic*>( ps_mParts[PNLE_PreviewWorkAreaComOrigFlame] );	// カタログ原稿.
	if( NULL != catalog_Flame ) {
		catalog_Flame->changeParts( s_PartsFlame );	// グラフィック変更.
		catalog_Flame->showEnable( True );

		short _ss_FlamePosX = 0;
		short _ss_FlamePosY = 0;
		short _ss_FlameWidth = 0;
		short _ss_FlameHeight = 0;
		catalog_Flame->getPosition( _ss_FlamePosX, _ss_FlamePosY );
		catalog_Flame->getSize( _ss_FlameWidth, _ss_FlameHeight );

		PSinglePartsGraphic* catalog_Image = dynamic_cast<PSinglePartsGraphic*>( ps_mParts[PNLE_PreviewWorkAreaComOrigImage] );	// カタログ原稿.
		if( NULL != catalog_Image ) {
			catalog_Image->changeGraphic( s_PartsImage[PNLE_Parts_Usually].puc_Parts );	// グラフィック変更.
			short _ss_ImageWidth = 0;
			short _ss_ImageHeight = 0;
			catalog_Image->getSize( _ss_ImageWidth, _ss_ImageHeight );
			short _ss_ImagePosX = (short)( _ss_FlamePosX + ( _ss_FlameWidth - ss_AdjustSizeX - _ss_ImageWidth ) / 2 );
			short _ss_ImagePosY = (short)( ss_StartPosY + _ss_FlamePosY + ( _ss_FlameHeight - ss_AdjustSizeY - _ss_ImageHeight ) / 2 );
			catalog_Image->changePosition( _ss_ImagePosX, _ss_ImagePosY );
			catalog_Image->showEnable( True );
		}
	}
}

//==============================================================================.
// カード原稿表示処理.
//==============================================================================.
void PPreviewWorkArea::setOriginalCardImage( PNLS_Parts s_Parts[PNLD_PartsNum] )
{
	DEF_UPDATEDRAWPARTSINFO;		// comment 2016/03/30 OPS1-Sky長尾芳人 [操作性改善][不要描画] 不要描画検出機構対応.
	
	PSinglePartsResizableGraphic* originalCardCopy = dynamic_cast<PSinglePartsResizableGraphic*>( ps_mParts[PNLE_PreviewWorkAreaComOrigFlame] );	// 原稿_枠.
	if( NULL != originalCardCopy ) {
		originalCardCopy->changeParts( s_Parts );	// グラフィック変更.
		originalCardCopy->showEnable( True );
	}
}

//==============================================================================.
// ブック原稿表示処理.
//==============================================================================.
void PPreviewWorkArea::setOriginalPageShot( PNLS_Parts s_Parts[PNLD_PartsNum] )
{
	DEF_UPDATEDRAWPARTSINFO;		// comment 2016/03/30 OPS1-Sky長尾芳人 [操作性改善][不要描画] 不要描画検出機構対応.
	
	PSinglePartsResizableGraphic* originalPageShot = dynamic_cast<PSinglePartsResizableGraphic*>( ps_mParts[PNLE_PreviewWorkAreaOrigPageShot] );	// ブック原稿.
	if( NULL != originalPageShot ) {
		originalPageShot->changeParts( s_Parts );	// グラフィック変更.
		originalPageShot->showEnable( True );
	}
}

//==============================================================================.
// 小冊子表示処理(画像のみ).
//==============================================================================.

void PPreviewWorkArea::setMagazineImage( uchar* uc_mainImage, uchar uc_Palet )
{
	DEF_UPDATEDRAWPARTSINFO;		// comment 2016/03/30 OPS1-Sky長尾芳人 [操作性改善][不要描画] 不要描画検出機構対応.
	
	short ss_FramePosX = 0;			// 枠のX位置.
	short ss_FramePosY = 0;			// 枠のY位置.
	short ss_FrameWidth = 0;		// 枠のXサイズ.
	short ss_FrameHeight = 0;		// 枠のYサイズ.

	short ss_MagazineWidth = 0;		// 画像のXサイズ.
	short ss_MagazineHeight = 0;	// 画像のYサイズ.

	short ss_ImagePositionX = 0;	// 画像シフト後のX位置
	short ss_ImagePositionY = 0;	// 画像シフト後のY位置

	// 枠のパーツを取得を取得.
	PSinglePartsResizableGraphic* frame = dynamic_cast<PSinglePartsResizableGraphic*>( ps_mParts[PNLE_PreviewWorkAreaComOutputFlame] );
	// 画像のパーツを取得.
	PSinglePartsGraphic* magazine = dynamic_cast<PSinglePartsGraphic*>( ps_mParts[PNLE_PreviewWorkAreaComOutputImage] );

	if( NULL == frame || NULL == magazine ) {	// 静的解析.
		PnlComPutDebugLog( True, UC_PNL_LOGPATTERN_1, "PPreviewWorkArea::setImagePosition", UC_PNL_ERROR_USE );
		return;
	}

	// 用紙画像をセット.
	magazine->changeGraphic( uc_mainImage, uc_Palet, True );
	// 表示パーツを表示.
	magazine->showEnableShow( True );

	// 枠の位置、サイズを取得.
	frame->getPosition( ss_FramePosX,ss_FramePosY );
	frame->getSize( ss_FrameWidth,ss_FrameHeight );

	// 画像のサイズを取得.
	magazine->getSize( ss_MagazineWidth,ss_MagazineHeight );

	// 画像シフト幅を計算する.
	ss_ImagePositionX = (short)( ss_FramePosX + ( ( ss_FrameWidth - ss_MagazineWidth ) / 2 ) );
	ss_ImagePositionY = (short)( ss_FramePosY + ( ( ss_FrameHeight - ss_MagazineHeight ) / 2 ) );

	// 画像表示位置設定.
	magazine->changePosition( ss_ImagePositionX, ss_ImagePositionY );
}
//==============================================================================.
// 表示処理(汎用版).
//==============================================================================.
void PPreviewWorkArea::setImage( PNLS_Parts s_Parts[PNLD_PartsNum],  PNLE_PreviewWorkAreaPartsIndex mainImageIndex )
{
	DEF_UPDATEDRAWPARTSINFO;		// comment 2016/03/30 OPS1-Sky長尾芳人 [操作性改善][不要描画] 不要描画検出機構対応.
	
	PSinglePartsResizableGraphic* p_mParts = dynamic_cast<PSinglePartsResizableGraphic*>( ps_mParts[mainImageIndex] );
	if( NULL != p_mParts ) {
		p_mParts->changeParts( s_Parts );	// グラフィック変更.
		p_mParts->showEnable( True );
	}
}

//==============================================================================.
// カードコピー表示処理.
//==============================================================================.
void PPreviewWorkArea::setCardImage( PNLS_Parts s_Parts[PNLD_PartsNum], PNLS_Parts s_PaperForm[PNLD_PartsNum] )
{
	DEF_UPDATEDRAWPARTSINFO;		// comment 2016/03/30 OPS1-Sky長尾芳人 [操作性改善][不要描画] 不要描画検出機構対応.
	
	short ss_FramePosX = 0; // 出力枠のX座標.
	short ss_FramePosY = 0; // 出力枠のY座標.

	PSinglePartsResizableGraphic* cardCopyForm = dynamic_cast<PSinglePartsResizableGraphic*>( ps_mParts[PNLE_PreviewWorkAreaComOutputFlame] );	// 出力_枠.
	if( NULL != cardCopyForm ) {
		cardCopyForm->changeParts( s_PaperForm );
		cardCopyForm->showEnable( True );
		cardCopyForm->getPosition( ss_FramePosX, ss_FramePosY );
	}

	PSinglePartsGraphic* cardCopy = dynamic_cast<PSinglePartsGraphic*>( ps_mParts[PNLE_PreviewWorkAreaComOutputImage] );	// 出力_画像.
	if( NULL != cardCopy ) {
		// 出力_画像の開始X/Y座標をデフォルト値に初期化する.
		cardCopy->changePosition( ss_FramePosX, ss_FramePosY );
		cardCopy->changeGraphic( s_Parts[0].puc_Parts, s_Parts[0].uc_PaletteType, True );
		cardCopy->showEnable( True );
	}
}

//==============================================================================.
// ページ連写表示処理.
//==============================================================================.
void PPreviewWorkArea::setPageShotImage( uchar* uc_Parts )
{
	DEF_UPDATEDRAWPARTSINFO;		// comment 2016/03/30 OPS1-Sky長尾芳人 [操作性改善][不要描画] 不要描画検出機構対応.
	
	PSinglePartsGraphic* pc_PageShot = dynamic_cast<PSinglePartsGraphic*>( ps_mParts[PNLE_PreviewWorkAreaComOrigImage] );	// 原稿_画像.
	if( NULL != pc_PageShot ) {
		pc_PageShot->changeGraphic( uc_Parts );	// グラフィック変更.
		pc_PageShot->showEnable( True );
	}
}
/*================================================================================================
［機能］	ステイプル画像を表示の設定.
--------------------------------------------------------------------------------------------------
［引数］	uc_previewPart				原稿か出力使用.
			uc_stapleImage				ステイプル画像.
［戻値］	なし
================================================================================================*/
void PPreviewWorkArea::setStapleImage( TYPE_StapleMode e_StapleMode,PNLE_PreviewStaplePosition e_StaplePosition, uchar* uc_stapleImage, PNLE_PreviewWorkAreaPartsIndex outputStapleIndex , PNLE_PreviewWorkAreaPartsIndex outputFormIndex, TYPE_FoldMode foldMode )		// comment CQ356131 ステータス表示エリア対応(ステープルが表示されない問題) 2011/09/30 酒井(健).
{
	DEF_UPDATEDRAWPARTSINFO;		// comment 2016/03/30 OPS1-Sky長尾芳人 [操作性改善][不要描画] 不要描画検出機構対応.
	
	short ss_SetPosX = 0;		// セットするステープルX座標.
	short ss_SetPosY = 0;		// セットするステープルY座標.


	PSinglePartsGraphic* staplePart = dynamic_cast<PSinglePartsGraphic*>( ps_mParts[outputStapleIndex] );	// ステープルパーツ. comment CQ356131 ステータス表示エリア対応(ステープルが表示されない問題) 2011/09/30 酒井(健).
	if( NULL == staplePart ) {
		PnlComPutDebugLog( True, UC_PNL_LOGPATTERN_1, "PPreviewWorkArea::setStapleImage", UC_PNL_ERROR_USE );
		return;
	}
	staplePart->changeGraphic( uc_stapleImage );					// ステープルのグラフィック変更.

	short ss_StapleWidth = 0;	// ステープルX方向サイズ.
	short ss_StapleHeight = 0;	// ステープルY方向サイズ.

	// 現在のステープルサイズを取得.
	staplePart->getSize( ss_StapleWidth, ss_StapleHeight );

	// 用紙（下地部分）の表示位置、サイズを取得.
	PSinglePartsResizableGraphic* outputForm = dynamic_cast<PSinglePartsResizableGraphic*>( ps_mParts[outputFormIndex] );		// comment CQ354110 ステータス表示エリア対応 2011/09/16 酒井(健).
	if( NULL == outputForm ) {
		PnlComPutDebugLog( True, UC_PNL_LOGPATTERN_1, "PPreviewWorkArea::setStapleImage", UC_PNL_ERROR_USE );
		return;
	}
	short ss_BasePosX = 0;		// 基準パーツのX座標.
	short ss_BasePosY = 0;		// 基準パーツのY座標.
	short ss_BaseWidth = 0;		// 基準パーツのX方向サイズ.
	short ss_BaseHeight = 0;	// 基準パーツのY方向サイズ.

	outputForm->getPosition( ss_BasePosX,ss_BasePosY );
	outputForm->getSize( ss_BaseWidth,ss_BaseHeight );


	// ステープル位置に応じて、表示位置を変える.
	if( e_StapleMode == TYPE_StapleModeCenter ) {	// センターステープル.

		PSinglePartsResizableGraphic* magazine = dynamic_cast<PSinglePartsResizableGraphic*>( ps_mParts[PNLE_PreviewWorkAreaMagazine] );					// 小冊子.
		if( NULL != magazine ) {
			magazine->getPosition( ss_BasePosX,ss_BasePosY );	// 小冊子の表示位置を取得.
			if( ( TYPE_FoldModeCenterInner == foldMode ) || ( TYPE_FoldModeCenterOuter == foldMode ) ) {	// 紙折りモード：外中折りの場合.
				ss_SetPosX = (short)( ss_BasePosX + ( ( ss_BaseWidth - ss_StapleWidth ) / 2 ) );		// 真ん中の位置になるように調整.
				ss_SetPosY = (short)( ss_BasePosY + ( ( ss_BaseHeight - ss_StapleHeight ) / 2 ) );	// 真ん中の位置になるように調整.
			} else {
				// 表示位置は表冊子の表示位置とする.
				ss_SetPosX = ss_BasePosX;
				ss_SetPosY = ss_BasePosY;
			}

		}

	} else if( e_StapleMode == TYPE_StapleModeCorner ) {	// コーナーステープル.

		switch( e_StaplePosition ) {
			case PNLE_PreviewStaplePositionLeftUp:		// 左上.
				ss_SetPosX = (short)( ss_BasePosX + PNLD_StaplePosition );
				ss_SetPosY = ss_BasePosY;
				break;

			case PNLE_PreviewStaplePositionLeftDown:	// 左下.
//2016/7/21 CQ532802 sky草柳 プレビューステープル用画像位置調整.
#ifdef DEF_A4_Cus
				ss_SetPosX = ss_BasePosX + PNLD_StaplePositionAjust4;
				ss_SetPosY = (ss_BasePosY + ss_BaseHeight - ss_StapleHeight - PNLD_StaplePosition) + PNLD_StaplePositionAjust2;
#else
				ss_SetPosX = ss_BasePosX;
				ss_SetPosY = (short)( ss_BasePosY + ss_BaseHeight - ss_StapleHeight - PNLD_StaplePosition );
#endif
				break;

			case PNLE_PreviewStaplePositionRightUp:		// 右上.
#ifdef DEF_A4_Cus
				ss_SetPosX = (ss_BasePosX + ss_BaseWidth - ss_StapleWidth - PNLD_StaplePosition) - PNLD_StaplePositionAjust2;
				ss_SetPosY = ss_BasePosY + PNLD_StaplePositionAjust4;
#else
				ss_SetPosX = (short)( ss_BasePosX + ss_BaseWidth - ss_StapleWidth - PNLD_StaplePosition );
				ss_SetPosY = ss_BasePosY;
#endif
				break;
			default: break;
		}
	} else {

		switch( e_StaplePosition ) {


			case PNLE_PreviewStaplePositionUp:		// 上.
				ss_SetPosX = (short)( ss_BasePosX + ( ( ss_BaseWidth - ss_StapleWidth ) / 2 ) );	// 真ん中の位置になるように調整.
				ss_SetPosY = (short)( ss_BasePosY + PNLD_StaplePosition );
				break;
			case PNLE_PreviewStaplePositionDown:	// 下.
				ss_SetPosX = (short)( ss_BasePosX + ( ( ss_BaseWidth - ss_StapleWidth ) / 2 ) );	// 真ん中の位置になるように調整.
				ss_SetPosY = (short)( ss_BasePosY + ss_BaseHeight - ss_StapleHeight - PNLD_StaplePosition + PNLD_StaplePucnhPositionDownAjust );
				break;
			case PNLE_PreviewStaplePositionLeft:	// 左.
				ss_SetPosX = (short)( ss_BasePosX + PNLD_StaplePosition );
				ss_SetPosY = (short)( ss_BasePosY + ( ( ss_BaseHeight - ss_StapleHeight ) / 2 ) );	// 真ん中の位置になるように調整.
				break;
			case PNLE_PreviewStaplePositionRight:	// 右.
				ss_SetPosX = (short)( ss_BasePosX + ss_BaseWidth - ss_StapleWidth - PNLD_StaplePosition );
				ss_SetPosY = (short)( ss_BasePosY + ( ( ss_BaseHeight - ss_StapleHeight ) / 2 ) );	// 真ん中の位置になるように調整.
				break;
			default:	break;
		}
	}


	staplePart->changePosition( ss_SetPosX,ss_SetPosY );		// ステープル表示位置セット.
	staplePart->showEnableShow( True );							// ステープル表示.

}

/*================================================================================================
［機能］	パンチ画像を表示の設定.
--------------------------------------------------------------------------------------------------
［引数］	uc_previewPart				原稿か出力使用.
			uc_duplexImage				パンチ画像.
［戻値］	なし
================================================================================================*/
void PPreviewWorkArea::setPunchImage( PNLE_PreviewPunchPosition e_PunchPosition, uchar* uc_punchImage )
{
	DEF_UPDATEDRAWPARTSINFO;		// comment 2016/03/30 OPS1-Sky長尾芳人 [操作性改善][不要描画] 不要描画検出機構対応.
	
	short ss_SetPosX = 0;		// セットするパンチX座標.
	short ss_SetPosY = 0;		// セットするパンチY座標.

	// パンチ表示パーツを取得.
	PSinglePartsGraphic* punchPart = dynamic_cast<PSinglePartsGraphic*>( ps_mParts[PNLE_PreviewWorkAreaOutputPunch] );
	if( NULL == punchPart ) {
		PnlComPutDebugLog( True, UC_PNL_LOGPATTERN_1, "PPreviewWorkArea::setPunchImage", UC_PNL_ERROR_USE );
		return;
	}
	punchPart->changeGraphic( uc_punchImage );					// パンチのグラフィック変更.

	short ss_PunchWidth = 0;		// パンチX方向サイズ.
	short ss_PunchHeight = 0;		// パンチY方向サイズ.

	// 現在のパンチサイズを取得.
	punchPart->getSize( ss_PunchWidth,ss_PunchHeight );


	// 用紙（下地部分）の表示位置、サイズを取得.
	PSinglePartsResizableGraphic* outputForm = dynamic_cast<PSinglePartsResizableGraphic*>( ps_mParts[PNLE_PreviewWorkAreaOutputForm] );
	if( NULL == outputForm ) {
		PnlComPutDebugLog( True, UC_PNL_LOGPATTERN_1, "PPreviewWorkArea::setPunchImage", UC_PNL_ERROR_USE );
		return;
	}
	short ss_BasePosX = 0;		// 用紙パーツのX座標.
	short ss_BasePosY = 0;		// 用紙パーツのY座標.
	short ss_BaseWidth = 0;		// 用紙パーツのX方向サイズ.
	short ss_BaseHeight = 0;	// 用紙パーツのY方向サイズ.

	outputForm->getPosition( ss_BasePosX,ss_BasePosY );
	outputForm->getSize( ss_BaseWidth,ss_BaseHeight );


	// パンチ位置に応じて、表示位置を変える.
	switch( e_PunchPosition ) {

		case PNLE_PreviewPunchPositionUp:		// 上.
			ss_SetPosX = (short)( ss_BasePosX + ( ( ss_BaseWidth - ss_PunchWidth ) / 2 ) );	// 真ん中の位置になるように調整.
			ss_SetPosY = (short)( ss_BasePosY + PNLD_PunchPosition );
			break;
		case PNLE_PreviewPunchPositionDown:		// 下.
			ss_SetPosX = (short)( ss_BasePosX + ( ( ss_BaseWidth - ss_PunchWidth ) / 2 ) );	// 真ん中の位置になるように調整.
			ss_SetPosY = (short)( ss_BasePosY + ss_BaseHeight - ss_PunchHeight - PNLD_PunchPosition + PNLD_StaplePucnhPositionDownAjust );
			break;
		case PNLE_PreviewPunchPositionLeft:		// 左.
			ss_SetPosX = (short)( ss_BasePosX + PNLD_PunchPosition );
			ss_SetPosY = (short)( ss_BasePosY + ( ( ss_BaseHeight - ss_PunchHeight ) / 2 ) );	// 真ん中の位置になるように調整.
			break;
		case PNLE_PreviewPunchPositionRight:	// 右.
			ss_SetPosX = (short)( ss_BasePosX + ss_BaseWidth - ss_PunchWidth - PNLD_PunchPosition );
			ss_SetPosY = (short)( ss_BasePosY + ( ( ss_BaseHeight - ss_PunchHeight ) / 2 ) );	// 真ん中の位置になるように調整.
			break;
		default:	break;
	}

	punchPart->changePosition( ss_SetPosX,ss_SetPosY );		// パンチの表示位置セット.
	punchPart->showEnableShow( True );						// パンチ表示.
	PSinglePartsGraphic* staplePart = dynamic_cast<PSinglePartsGraphic*>( ps_mParts[PNLE_PreviewWorkAreaOutputStaple] );	// ステープルパーツ.
	if( NULL != staplePart
	    && e_PunchPosition == PNLE_PreviewPunchPositionDown
	    && staplePart->isNowShown() == True ) {	// 表示位置が下でかつステープル表示中の場合.
		staplePart->showEnableShow( True );	// パンチのパーツ(透明になっている箇所)と重なるので再描画を行う.
	}

}
/*================================================================================================
［機能］	紙折り画像を表示の設定.
--------------------------------------------------------------------------------------------------
［引数］	uc_previewPart				原稿か出力使用.
			uc_duplexImage				紙折り画像.
［戻値］	なし
================================================================================================*/
void PPreviewWorkArea::setFoldImage( uchar uc_previewPart, uchar* uc_foldImage )
{
	DEF_UPDATEDRAWPARTSINFO;		// comment 2016/03/30 OPS1-Sky長尾芳人 [操作性改善][不要描画] 不要描画検出機構対応.
	
	// 出力を使用の場合.
	if( PPC_PreviewOutputIndex == uc_previewPart ) {

		// 紙折り表示パーツを取得.
		PSinglePartsGraphic* foldPart = dynamic_cast<PSinglePartsGraphic*>( ps_mParts[PNLE_PreviewWorkAreaOutputFold] );
		if( NULL != foldPart ) {
			// 紙折り画像をセット.
			foldPart->changeGraphic( uc_foldImage );
			// 紙折り表示パーツを表示.
			foldPart->showEnableShow( True );
		}
	}

}
//==============================================================================.
// プレビューコンポーネント:アイコン表示処理.
//==============================================================================.
void PPreviewWorkArea::setupShowIcon( PSinglePartsGraphic* parts, PNLS_IconPosition* pst_IconPosition,short& ss_Buf )
{
	DEF_UPDATEDRAWPARTSINFO;		// comment 2016/03/30 OPS1-Sky長尾芳人 [操作性改善][不要描画] 不要描画検出機構対応.
	
	if( NULL == parts ) {
		PnlComPutDebugLog( True, UC_PNL_LOGPATTERN_1, "PPreviewWorkArea::setupShowIcon", UC_PNL_ERROR_USE );
		return;
	}
	// アイコン表示位置が全て埋まっていなければ表示.
	if( ss_Buf < uc_SizeIconPositionTbl ) {
		// 同じパーツがすでに入っていないか確認.
		for( uchar i = 0; i < ss_Buf; i++ ) {
			if( ss_Buf != 0 &&( pst_IconPosition + i )->parts == parts ) {	// すでに入っている場合は座標の移動はしない.
				parts->showEnable( True );
				return;
			}
		}
		parts->changePosition( ( pst_IconPosition + ss_Buf )->ss_PosX ,( pst_IconPosition + ss_Buf )->ss_PosY );
		parts->showEnable( True );
		( pst_IconPosition + ss_Buf )->parts = parts;
		ss_Buf++;
	}
}

//==============================================================================.
// プレビューコンポーネント:アイコン非表示処理.
//==============================================================================.
void PPreviewWorkArea::setupHideIcon( PSinglePartsGraphic* parts, PNLS_IconPosition* pst_IconPosition,short& ss_Buf )
{
	DEF_UPDATEDRAWPARTSINFO;		// comment 2016/03/30 OPS1-Sky長尾芳人 [操作性改善][不要描画] 不要描画検出機構対応.
	
	uchar hidePosition = 0;	// 消去するアイコン位置.
	if( ss_Buf != 0 ) {	// アイコンが一つもなければ処理しない.
		// パーツ探索処理.
		for( uchar i = 0; i < ss_Buf; i++ ) {
			if( ( pst_IconPosition + i )->parts == parts ) {
				hidePosition = i;
				break;
			}
		}
		if( hidePosition != 0 ) {	// パーツが見つからなかった場合は、無視する.
			if( NULL == parts ) {
				PnlComPutDebugLog( True, UC_PNL_LOGPATTERN_1, "PPreviewWorkArea::setupHideIcon", UC_PNL_ERROR_USE );
				return;
			}
			parts->showEnable( False );
			// パーツ位置変更処理.
			for( uchar i = (uchar)(hidePosition + 1); i < ss_Buf; i++ ) {
				( pst_IconPosition + i )->parts->changePosition( ( pst_IconPosition + ( i - 1 ) )->ss_PosX ,( pst_IconPosition + ( i - 1 ) )->ss_PosY );
				( pst_IconPosition + ( i - 1 ) )->parts = ( pst_IconPosition + i )->parts;	// 表示位置テーブルのパーツを変更.
			}
			( pst_IconPosition + ss_Buf )->parts = NULL;				// 一番最後の位置のアイコンの表示位置を初期化する.
			ss_Buf--;	// 表示数を減らす.
		}
	}
}
//==============================================================================.
// プレビューコンポーネント:アイコン非表示処理.
//==============================================================================.
void PPreviewWorkArea::setIrregularIcon( PNLS_Parts s_Parts[PNLD_PartsNum] )
{
	DEF_UPDATEDRAWPARTSINFO;		// comment 2016/03/30 OPS1-Sky長尾芳人 [操作性改善][不要描画] 不要描画検出機構対応.
	
	PSinglePartsResizableGraphic* pc_IrregularIcon = dynamic_cast<PSinglePartsResizableGraphic*>( ps_mParts[PNLE_PreviewWorkAreaComOutputIrregularIcon] );	// 	欠損・余白アイコン.
	if( NULL != pc_IrregularIcon ) {
		pc_IrregularIcon->changeParts( s_Parts );	// グラフィック変更.
		pc_IrregularIcon->showEnable( True );
	}
}
//==============================================================================.
// プレビューコンポーネント:原稿プレビューが表示されているかどうかの判定.
//==============================================================================.
Bool PPreviewWorkArea::isOriginalImageShow()
{
	DEF_UPDATEDRAWPARTSINFO;		// comment 2016/03/30 OPS1-Sky長尾芳人 [操作性改善][不要描画] 不要描画検出機構対応.
	
	if( True == Gra_PreviewCompOriginalForm.isNowShown()
	    || True == Gra_PreviewCompOriginalImage.isNowShown()
	    || True == Gra_PreviewCompOrigComFlame.isNowShown()
	    || True == Gra_PreviewCompOrigComImage.isNowShown() )
	{
		return True;
	} else {
		return False;
	}
}
//==============================================================================.
// プレビューコンポーネント:出力プレビューが表示されているかどうかの判定.
//==============================================================================.
Bool PPreviewWorkArea::isOutputImageShow()
{
	DEF_UPDATEDRAWPARTSINFO;		// comment 2016/03/30 OPS1-Sky長尾芳人 [操作性改善][不要描画] 不要描画検出機構対応.
	
	if( True == Gra_PreviewCompOutputForm.isNowShown()
	    || True == Gra_PreviewCompOutputImage.isNowShown()
	    || True == Gra_PreviewCompOutputComFlame.isNowShown()
	    || True == Gra_PreviewCompOutputComImage.isNowShown()
	    || True == Gra_PreviewCompOutputComFinishing.isNowShown()
	    || True == Gra_PreviewCompOutputComFinishing2.isNowShown() )
	{
		return True;
	} else {
		return False;
	}
}

