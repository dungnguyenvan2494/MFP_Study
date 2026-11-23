/*================================================================================================
［タイトル］	表示活性化（網掛け）状態を変更可能なパーツ
================================================================================================*/
/*================================================================================================
	インクルード・ファイル
================================================================================================*/
#include "GFWC_System.h"
#include "GFWC_Window.h"
#include <panel/pddtext/pddtext_CLCD/pstr_CLCD.h>
#include <panel/pdp/pdpg/pdpdebug.h>
#include "panel/ppcg/ppcg_CLCD/ppcgdispp_CLCD.h"
#include <panel/ppcg/ppcg_CLCD/pidispd_CLCD.h>
#include <panel/plibl/plibl_CLCD/tskpnldraw_CLCD.h>
#include "ppcgtextf_CLCD2.h"
#include "ppcgnumber_CLCD2.h"
#include "ppcgobjectadd_CLCD2.h"

#include "panel/pcgcom/pcgcomg/dyncreate_debug.h"
// comment 2016/03/15 OPS1-Sky長尾芳人 [操作性改善][不要描画] 不要描画検出機構対応.
#include <panel/pnlmngr/pnlmngrg/PNLC_PartsAccessDebugLib.h>


/*================================================================================================
［機能］	コンストラクタ.
--------------------------------------------------------------------------------------------------
［引数］	PNLS_Parts	 	 s_Parts         : 表示パーツ（グラフィック）
			short		 	 tx              : 表示位置Ｘ座標（ドット単位）
			short		 	 gy              : 表示位置Ｙ座標（ドット単位）
			PSingleParts** 	 ppc_AddParts    : ボタン共通パーツ郡
			uchar 			 uc_PartsCount	 : ボタン共通パーツ数
			Bool			 b_AddRangeSet	 : 親の表示位置・サイズの設定有無
			PNLE_Shadow		 e_Shadow		 : 影表示
			short			 SizeX			 : X方向サイズ
			short			 SizeY			 : Y方向サイズ
［戻値］
================================================================================================*/
PObjectAdd::PObjectAdd( PNLS_Parts s_Parts[PNLD_PartsNum],
                        short tx,
                        short gy,
                        PSingleParts** ppc_AddParts,
                        uchar uc_PartsCount,
                        Bool b_AddRangeSet,
                        PNLE_Shadow e_Shadow,
                        short SizeX,
                        short SizeY,
                        GFWE_LCD_Size e_Pos,
                        GFWE_LCD_Size e_Size
                      )
	: PActivatableAdd( puc_Parts = s_Parts[PNLE_Parts_Usually].puc_Parts, tx, gy, uc_PaletteType = s_Parts[PNLE_Parts_Usually].uc_PaletteType,  ppc_AddParts, uc_PartsCount, b_AddRangeSet, e_Pos, e_Size )

{
	//メンバ変数を初期化.
	PObjectAdd::m_ShowSizeX = SizeX;
	PObjectAdd::m_ShowSizeY = SizeY;

	m_eShadow = e_Shadow;

	for( uchar i =0; i < PNLD_PartsNum; i++ ) {
		m_sParts[i] = s_Parts[i];
	}
	short ss_Xsize = 0;
	short ss_Ysize = 0;
	getSize( ss_Xsize, ss_Ysize );
	for( i = 0; uc_PartsCount > i; ++i ) {
		// ボタン共通パーツへのポインタがNULLの場合
		if( NULL == ppc_AddParts[ i ] ) {
			PnlComPutDebugLog( True, UC_PNL_LOGPATTERN_1, "PActivatableAdd::PActivatableAdd", UC_PNL_ERROR_PARAMETER );
			return;
		}
		// 既にオリジナルの画像サイズが設定されているが、PObject以下は可変サイズを再セットする.
		ppc_AddParts[ i ]->setParentPositionX( tx, ss_Xsize, b_AddRangeSet );	// 親の表示位置（横方向）の設定
		ppc_AddParts[ i ]->setParentPositionY( gy, ss_Ysize, b_AddRangeSet );	// 親の表示位置（縦方向）の設定
	}

}

/*================================================================================================
［機能］	GUIパーツの生成処理.
--------------------------------------------------------------------------------------------------
［引数］	PDisplayParts* parent : 親パーツ.
［戻値］	無し.
 [変更履歴]	2013/08/23 MSE 若宮 GUIパーツ動的生成対応.
================================================================================================*/
void PObjectAdd::startGUI( PDisplayParts* parent )
{
	DEF_UPDATEDRAWPARTSINFO;		// comment 2016/03/30 OPS1-Sky長尾芳人 [操作性改善][不要描画] 不要描画検出機構対応.
	
	// 表示パーツのNULLチェック.
	if( NULL == puc_Parts ) {
		PnlComPutDebugLog( True, UC_PNL_LOGPATTERN_1, "PObjectAdd::startGUI", UC_PNL_ERROR_PARAMETER );
		return;
	}

	GFWC_Area_Normal* pc_GUISelf = static_cast<GFWC_Area_Normal*>( getGUIParts() );
	if( !pc_GUISelf )
	{
		GFWC_Common*	pc_GUIParent = NULL;	// 親パーツのGUIパーツ.

		// 親パーツのGUIパーツを取得する.
		if( NULL != parent ) {
			pc_GUIParent = static_cast<GFWC_Common*>( parent->getGUIParts() );
			if( NULL == pc_GUIParent ) {
				PnlComPutDebugLog( True, UC_PNL_LOGPATTERN_1, "PObjectAdd::startGUI", UC_PNL_ERROR_IMPOSSIBLE );
				return;
			}
		}

		// GUIパーツを生成する.
		pc_GUISelf = new GFWC_Area_Normal( pc_GUIParent, getPosScaleSetting(), getSizeScaleSetting() );
		if( NULL == pc_GUISelf ) {
			PnlComPutDebugLog( True, UC_PNL_LOGPATTERN_1, "PObjectAdd::startGUI", UC_PNL_ERROR_IMPOSSIBLE );
			return;
		}

		// GUIパーツを基底クラスにて管理する.
		setGUIParts( pc_GUISelf );
	}
	else
	{
		// 生成済みの場合は重なりを再現するためにbringToFrontを実施
		pc_GUISelf->bringToFront();
	}

	// 親子関係を設定する.
	PDisplayParts::setParentParts( parent );

	// 表示位置を設定する.
	setPositionToGUIparts();

	// 表示サイズを設定する.
	setSizeToGUIparts();

	// 影を設定する.
	GFWE_SHADOW e_GuiShadow = convertToShadow( m_eShadow );
	pc_GUISelf->setShadow( e_GuiShadow );

	// 活性状態を設定する.
	pc_GUISelf->setActivation( isActivating() == True ? GFWD_TRUE : GFWD_FALSE );

	// グラフィックテーブルを設定する.
	GFWS_GraphicTable st_GraphicTable = convertToGraphicTable( m_sParts );
	pc_GUISelf->setGraphicTable( st_GraphicTable );
	DEF_UPDATEDRAWPARTSINFO_GUI;		// comment 2016/03/30 OPS1-Sky長尾芳人 [操作性改善][不要描画] 不要描画検出機構対応.

	// 追加パーツが登録されている時.
	_SHOW_PARTS_START();
	if( NULL != ppc_AddParts ) {
		for( int i = 0; i < uc_PartsCount; i++ ) {
			if( ppc_AddParts[i] != NULL ) {
				_SHOW_PARTS_INFO( ppc_AddParts[i], i );
				ppc_AddParts[i]->startGUI( this );
				_SHOW_PARTS_GFWINFO( ppc_AddParts[i] );
			}
		}
	}
	_SHOW_PARTS_END();
}

/*================================================================================================
［機能］	GUIパーツの破棄処理.
--------------------------------------------------------------------------------------------------
［引数］	無し.
［戻値］	無し.
 [変更履歴]	2013/08/23 MSE 若宮 GUIパーツ動的生成対応.
================================================================================================*/
void PObjectAdd::endGUI()
{
	DEF_UPDATEDRAWPARTSINFO;		// comment 2016/03/30 OPS1-Sky長尾芳人 [操作性改善][不要描画] 不要描画検出機構対応.
	
	// 追加パーツが登録されている時.
	if( NULL != ppc_AddParts ) {
		for( int i = 0; i < uc_PartsCount; i++ ) {
			if( ppc_AddParts[i] != NULL ) {
				ppc_AddParts[i]->endGUI();
			}
		}
	}

	PDisplayParts::endGUI();
}

/*================================================================================================
［機能］	状態に合わせて表示パーツをセットする.
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	無し
 [変更履歴]	2013/08/19 MSE 佐々木 GUIフレームワーク(GUIパーツ未生成時のデータアクセス制御)対応.
================================================================================================*/
void PObjectAdd::setParts()
{
	DEF_UPDATEDRAWPARTSINFO;		// comment 2016/03/30 OPS1-Sky長尾芳人 [操作性改善][不要描画] 不要描画検出機構対応.
	
	// 非活性化状態でも選択状態でもない場合（通常状態）
	puc_Parts = m_sParts[PNLE_Parts_Usually].puc_Parts;
	uc_PaletteType = m_sParts[PNLE_Parts_Usually].uc_PaletteType;

	if( 0x00 == ( flag & IsActivate ) ) {
		// 非表示活性化の場合
		if( NULL != m_sParts[PNLE_Parts_Nonactivate].puc_Parts ) {
			puc_Parts = m_sParts[PNLE_Parts_Nonactivate].puc_Parts;
			uc_PaletteType = m_sParts[PNLE_Parts_Nonactivate].uc_PaletteType;
		}
	} else if( NowSelect == ( flag & NowSelect ) ) {
		// 選択状態の場合
		if( NULL != m_sParts[PNLE_Parts_Select].puc_Parts ) {
			puc_Parts = m_sParts[PNLE_Parts_Select].puc_Parts;
			uc_PaletteType = m_sParts[PNLE_Parts_Select].uc_PaletteType;
		}
	}
}
/*================================================================================================
［機能］	パーツを変更する.
--------------------------------------------------------------------------------------------------
［引数］	PNLS_Parts s_Parts[PNLD_PartsNum] : 表示パーツ.
［戻値］	無し
================================================================================================*/
void PObjectAdd::changeParts( PNLS_Parts s_Parts[PNLD_PartsNum] )
{
	DEF_UPDATEDRAWPARTSINFO;		// comment 2016/03/30 OPS1-Sky長尾芳人 [操作性改善][不要描画] 不要描画検出機構対応.
	
	if( NULL== s_Parts[PNLE_Parts_Usually].puc_Parts ) {		//パーツが登録されているかチェック.
		return;
	}
	// グラフィックが同じときは処理しない.
	if( False == isNeedRenewParts( s_Parts ) ) { return; }

	for( uchar i = 0; i < PNLD_PartsNum; i++ ) {
		m_sParts[i] = s_Parts[i];		//パーツを変更する.
	}

	GFWC_Area_Normal* pc_GUISelf = static_cast<GFWC_Area_Normal*>( getGUIParts() );
	if( NULL != pc_GUISelf ) {
		// グラフィックを設定する.
		struct GFWS_GraphicTable st_GraphicTable = convertToGraphicTable( s_Parts );

		pc_GUISelf->setGraphicTable( st_GraphicTable );
		DEF_UPDATEDRAWPARTSINFO_GUI;		// comment 2016/03/30 OPS1-Sky長尾芳人 [操作性改善][不要描画] 不要描画検出機構対応.
	}

	if( ( True == isNowShown() ) && ( False == isAddParts() ) ) {		//現在表示中で共通パーツではない場合.
		showParts();
	}
}

/*================================================================================================
［機能］	ボタン共通パーツに対してグラフィックを変更する.
--------------------------------------------------------------------------------------------------
［引数］	PSinglePartsResizableGraphic* pc_AddParts : ボタン共通パーツ.
			PNLS_Parts s_Parts[PNLD_PartsNum]         : 表示パーツ.
［戻値］	無し
================================================================================================*/
void PObjectAdd::changeParts( PSinglePartsResizableGraphic* pc_AddParts, PNLS_Parts s_Parts[PNLD_PartsNum] )
{
	DEF_UPDATEDRAWPARTSINFO;		// comment 2016/03/30 OPS1-Sky長尾芳人 [操作性改善][不要描画] 不要描画検出機構対応.
	
	if( False == checkAddParts( pc_AddParts ) ) {
		// 引数で指定したボタン共通パーツが不正な場合
		PnlComPutDebugLog( True, UC_PNL_LOGPATTERN_1, "PActivatableAdd::changeGraphic", UC_PNL_ERROR_PARAMETER );
		return;
	}
	// グラフィックが同じときは処理しない.
	if( False == pc_AddParts->isNeedRenewParts( s_Parts ) ) { return; }


	pc_AddParts->changeParts( s_Parts );

	if( True == isNowShown() ) {
		// 現在、表示画面上に表示中の時
		showParts();	// 実パーツを表示をする
	}
}
/*================================================================================================
［機能］	影表示を変更する.
--------------------------------------------------------------------------------------------------
［引数］	PNLE_Shadow	e_Shadow 		:影表示.
［戻値］	無し.
 [変更履歴]	2013/08/19 MSE 佐々木 GUIフレームワーク(GUIパーツ未生成時のデータアクセス制御)対応.
 [変更履歴]	2013/08/20 MSE 宮本 GUIフレームワーク(GUIパーツ未生成時のデータアクセス制御)対応.
================================================================================================*/
void PObjectAdd::changeShadow( PNLE_Shadow e_Shadow )
{
	DEF_UPDATEDRAWPARTSINFO;		// comment 2016/03/30 OPS1-Sky長尾芳人 [操作性改善][不要描画] 不要描画検出機構対応.
	
	if( e_Shadow == m_eShadow ) {		// 影設定が同じ場合.
		return;
	}

	// 既存パーツの影表示の更新.
	m_eShadow = e_Shadow;

	GFWC_Area_Normal* pc_GUISelf = static_cast<GFWC_Area_Normal*>( getGUIParts() );
	if( NULL != pc_GUISelf ) {
		// 表示サイズを設定する.
		setSizeToGUIparts();

		// 影を設定する.
		GFWE_SHADOW e_GuiShadow = convertToShadow( m_eShadow );
		pc_GUISelf->setShadow( e_GuiShadow );
		DEF_UPDATEDRAWPARTSINFO_GUI;		// comment 2016/03/30 OPS1-Sky長尾芳人 [操作性改善][不要描画] 不要描画検出機構対応.
	}

	if( ( True == isNowShown() )  && ( False == isAddParts() ) ) {	//現在表示中かつ共通パーツではない場合.
		showParts();
	}
}
/*================================================================================================
［機能］	実パーツを消去する.
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	無し
 [変更履歴]	2013/08/19 MSE 佐々木 GUIフレームワーク(GUIパーツ未生成時のデータアクセス制御)対応.
================================================================================================*/
void PObjectAdd::hideParts()
{
	DEF_UPDATEDRAWPARTSINFO;		// comment 2016/03/30 OPS1-Sky長尾芳人 [操作性改善][不要描画] 不要描画検出機構対応.
	
	// 表示位置を取得する .
	register short ss_Tx = 0;
	register short ss_Gy = 0;
	getPosition( ss_Tx, ss_Gy );
	// サイズを取得する .
	register short ss_SizeX = 0;
	register short ss_SizeY = 0;
	getSize( ss_SizeX, ss_SizeY );

	PNLS_ClipArea s_DrawArea;
	//描画エリアにパーツの位置、サイズをセット.
	s_DrawArea.ss_XPos = ss_Tx;
	s_DrawArea.ss_YPos = ss_Gy;
	s_DrawArea.ss_Width = ss_SizeX;
	s_DrawArea.ss_Height = ss_SizeY;

	if( True == correctDrawArea( s_DrawArea, s_mClipArea ) ) {	//パーツがクリッピングエリア内.
		// クリッピングエリア内の実パーツを消去 .
		GFWC_Common* pc_GUISelf = static_cast<GFWC_Common*>( getGUIParts() );
		if( NULL != pc_GUISelf ) {
			pc_GUISelf->hide();
			DEF_UPDATEDRAWPARTSINFO_GUI;		// comment 2016/03/30 OPS1-Sky長尾芳人 [操作性改善][不要描画] 不要描画検出機構対応.
		}

	}
}

/*================================================================================================
［機能］	実パーツを表示する.
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	無し
 [変更履歴]	2013/08/19 MSE 佐々木 GUIフレームワーク(GUIパーツ未生成時のデータアクセス制御)対応.
================================================================================================*/
void PObjectAdd::showParts()
{
	DEF_UPDATEDRAWPARTSINFO;		// comment 2016/03/30 OPS1-Sky長尾芳人 [操作性改善][不要描画] 不要描画検出機構対応.
	
	//状態に合わせて表示パーツをセット.
	setParts();
	// 表示位置を取得する
	register short ss_Tx = 0;
	register short ss_Gy = 0;
	getPosition( ss_Tx, ss_Gy );
	// サイズを取得する .
	register short Xsize = 0;
	register short Ysize = 0;
	getSize( Xsize, Ysize );

	PNLS_ClipArea s_DrawArea;
	//描画エリアにパーツの位置、サイズをセット.
	s_DrawArea.ss_XPos = ss_Tx;
	s_DrawArea.ss_YPos = ss_Gy;
	s_DrawArea.ss_Width = Xsize;
	s_DrawArea.ss_Height = Ysize;

	if( True == correctDrawArea( s_DrawArea, s_mClipArea ) ) {		//パーツがクリッピングエリア内.
		GFWC_Common* pc_GUISelf = static_cast<GFWC_Common*>( getGUIParts() );
		if( NULL != pc_GUISelf ) {
			pc_GUISelf->setBank( convertToBank( getBank() ) );
			pc_GUISelf->show();
			DEF_UPDATEDRAWPARTSINFO_GUI;		// comment 2016/03/30 OPS1-Sky長尾芳人 [操作性改善][不要描画] 不要描画検出機構対応.
		}

	}
	for( uchar i = 0; uc_PartsCount > i; ++i ) {	// ボタン共通パーツが登録されている時
		ppc_AddParts[ i ]->show();
	}
}

/*================================================================================================
［機能］	表示中のパーツサイズを変更する.
--------------------------------------------------------------------------------------------------
［引数］	short	ss_XSize 		:X方向サイズ.
			short	ss_YSize 		:Y方向サイズ.
［戻値］	無し.
 [変更履歴]	2013/08/20 MSE 宮本 GUIフレームワーク(GUIパーツ未生成時のデータアクセス制御)対応.
================================================================================================*/
void PObjectAdd::changeSize( short ss_SizeX, short ss_SizeY )
{
	DEF_UPDATEDRAWPARTSINFO;		// comment 2016/03/30 OPS1-Sky長尾芳人 [操作性改善][不要描画] 不要描画検出機構対応.
	
	// サイズを取得する .
	register short SizeX = 0;
	register short SizeY = 0;
	getSize( SizeX, SizeY );

	if( ( SizeX == ss_SizeX ) && ( SizeY == ss_SizeY ) ) {		//サイズに変更がない場合.
		return;
	}
	// 既存パーツサイズの更新.
	PObjectAdd::m_ShowSizeX = ss_SizeX;
	PObjectAdd::m_ShowSizeY = ss_SizeY;

	getSize( SizeX, SizeY );

	GFWC_Area_Normal* pc_GUISelf = static_cast<GFWC_Area_Normal*>( getGUIParts() );
	if( NULL != pc_GUISelf ) {
		// GUIパーツの表示サイズを設定する(※影を含むサイズを指定する).
		PNLE_Shadow e_shadow = PNLE_Shadow_none;
		convertFromShadow( pc_GUISelf->getShadow(), e_shadow );
		GFWC_Size	c_Size = addShadowSize( SizeX, SizeY, e_shadow );		// サイズ情報.
		pc_GUISelf->setSize( c_Size );
		DEF_UPDATEDRAWPARTSINFO_GUI;		// comment 2016/03/30 OPS1-Sky長尾芳人 [操作性改善][不要描画] 不要描画検出機構対応.
	}

	// 子パーツリスト数がボタン共通パーツのパーツの個数分存在するとき、可変サイズを再設定.
	//   ここでリターンせずに再表示があれば行う.
	for( uchar i = 0; uc_PartsCount > i; ++i ) {
		// 既にオリジナルの画像サイズが設定されているが、PObject以下は可変サイズを再セットする.
		ppc_AddParts[ i ]->setParentPositionX( tx, SizeX, m_bAddRangeSet );	// 親の表示位置（横方向）の設定
		ppc_AddParts[ i ]->setParentPositionY( gy, SizeY, m_bAddRangeSet );	// 親の表示位置（縦方向）の設定
	}

	if( ( True == isNowShown() )  && ( False == isAddParts() ) ) {	//現在表示中かつ共通パーツではない場合.
		showParts();
	}
}

/*================================================================================================
［機能］	無効領域を受け、部分再描画する
--------------------------------------------------------------------------------------------------
［引数］	PNLS_ClipArea s_InvalidArea：無効領域
［戻値］	無し
 [変更履歴]	2013/08/19 MSE 佐々木 GUIフレームワーク(GUIパーツ未生成時のデータアクセス制御)対応.
================================================================================================*/
void PObjectAdd::InvalidateRect( PNLS_ClipArea s_InvalidArea )
{
	DEF_UPDATEDRAWPARTSINFO;		// comment 2016/03/30 OPS1-Sky長尾芳人 [操作性改善][不要描画] 不要描画検出機構対応.
	
	if( True != isNowShown() ) {
		// 現在表示中でない場合は表示しない.
		return;
	}
	PNLS_ClipArea	s_DrawArea = s_InvalidArea;					//描画範囲を無効領域で初期化.
	if( False == correctDrawArea( s_DrawArea, s_mClipArea ) ) {	//無効領域がクリッピングエリア外.
		return;
	}
	// 表示位置を取得する.
	register short ss_Tx = 0;
	register short ss_Gy = 0;
	getPosition( ss_Tx, ss_Gy );
	// サイズを取得する.
	register short Xsize = 0;
	register short Ysize = 0;
	getSize( Xsize, Ysize );

	PNLS_ClipArea	s_CorrectArea = s_DrawArea;					//比較範囲に補正した無効エリアをセットする.
	//描画エリアにパーツの位置、サイズをセット.
	s_DrawArea.ss_XPos = ss_Tx;
	s_DrawArea.ss_YPos = ss_Gy;
	s_DrawArea.ss_Width = Xsize;
	s_DrawArea.ss_Height = Ysize;

	uchar uc_ShadowDistance = 0;		// 影のずれ幅.
	if( PNLE_Shadow_none != m_eShadow ) {
		// 影があるときはパーツに描画エリアに影のずれ幅を加える.
		switch( m_eShadow ) {
			//影のずれ幅が2Pixel）.
			case PNLE_Shadow_Parts_2:
			case PNLE_Shadow_Parts_2Full:	// comment MiZe 2011/08/02 Sky松本 [348333]影種追加.
			case PNLE_Shadow_Icon:
				uc_ShadowDistance = PNLD_ShadowDistance_2;
				break;
			//影のずれ幅が4Pixel）.
			case PNLE_Shadow_Parts_4:
			case PNLE_Shadow_Parts_4Full:	// comment MiZe 2011/08/02 Sky松本 [348333]影種追加.
				uc_ShadowDistance = PNLD_ShadowDistance_4;
				break;
			default: break;
		}
		s_DrawArea.ss_Width = (short)( s_DrawArea.ss_Width + uc_ShadowDistance );
		s_DrawArea.ss_Height = (short)( s_DrawArea.ss_Height + uc_ShadowDistance );
	}
	if( True == correctDrawArea( s_DrawArea, s_CorrectArea ) ) {		//パーツが補正範囲内.
		setParts();
		for( uchar i = 0; uc_PartsCount > i; ++i ) {		// ボタン共通パーツ数繰り返す.
			ppc_AddParts[ i ]->InvalidateRect( s_InvalidArea );
		}

	}
}

/*================================================================================================
［機能］	パーツサイズを取得する.
--------------------------------------------------------------------------------------------------
［引数］	short	ss_XSize 		:X方向サイズ.
			short	ss_YSize 		:Y方向サイズ.
［戻値］	無し.
 [変更履歴]	2013/08/19 MSE 佐々木 GUIフレームワーク(GUIパーツ未生成時のデータアクセス制御)対応.
================================================================================================*/
void PObjectAdd::getSize( short& ss_XSize, short& ss_YSize )
{
	DEF_UPDATEDRAWPARTSINFO;		// comment 2016/03/30 OPS1-Sky長尾芳人 [操作性改善][不要描画] 不要描画検出機構対応.
	
	short XSize;
	short YSize;
	PSinglePartsGraphic::getSize( XSize, YSize );			//画像のオリジナルサイズを取得.

	//-1の場合、オリジナルサイズをセット.
	ss_XSize = ( -1 == PObjectAdd::m_ShowSizeX )? XSize : PObjectAdd::m_ShowSizeX;
	ss_YSize = ( -1 == PObjectAdd::m_ShowSizeY )? YSize : PObjectAdd::m_ShowSizeY;
}

/*================================================================================================
［機能］	インデックスが0のボタン共通パーツに対して数値を代入する
--------------------------------------------------------------------------------------------------
［引数］	float number : 代入値
［戻値］	無し
================================================================================================*/
void PObjectAdd::operator=( float number )
{
	// この演算子をオーバーライドしないと、コンパイラは＝演算子をコンストラクタと間違えてしまう.
	PActivatableAdd::operator=( number );	// インデックスが0のボタン共通パーツに対して数値を代入する.
}
/*================================================================================================
［機能］	表示活性化状態を変更する
--------------------------------------------------------------------------------------------------
［引数］	Bool act : 表示活性化状態（True:表示活性化状態／False:非表示活性化状態）
［戻値］	無し
================================================================================================*/
void PObjectAdd::activate( Bool act )
{
	DEF_UPDATEDRAWPARTSINFO;		// comment 2016/03/30 OPS1-Sky長尾芳人 [操作性改善][不要描画] 不要描画検出機構対応.
	
	register Bool nowActivate = False;
	if( flag & IsActivate ) {
		nowActivate = True;
	}
	if( nowActivate == act ) {
		// 表示活性化状態が変化しない時
		return;
	}
	if( True == act ) {
		flag |= IsActivate;
	} else {
		flag &= (uchar)( ~IsActivate );
	}

	PObjectAdd::setParts();
	PObjectAdd::setSizeToGUIparts();

	GFWC_Common* pc_GUISelf = static_cast<GFWC_Common*>( getGUIParts() );
	if( NULL != pc_GUISelf ) {
		pc_GUISelf->setActivation( True == act ? GFWD_TRUE : GFWD_FALSE );
		DEF_UPDATEDRAWPARTSINFO_GUI;		// comment 2016/03/30 OPS1-Sky長尾芳人 [操作性改善][不要描画] 不要描画検出機構対応.
	}

	for( uchar i = 0; uc_PartsCount > i; ++i ) {
		PSinglePartsText* pc_Text = dynamic_cast< PSinglePartsText* >( ppc_AddParts[ i ] );
		PSinglePartsGraphic* pc_Graphic = dynamic_cast< PSinglePartsGraphic* >( ppc_AddParts[ i ] );
		if( NULL != pc_Text ) {
			// テキスト系のボタン共通パーツの場合
			UC_GPARTS_COLORNUMBER uc_ColorNumber = getGraphicColorNumber( puc_Parts );	// 画像データの色数を取得する
			pc_Text->setParentActivate( act, getBank(), uc_ColorNumber, uc_PaletteType );	// 親の選択状態表示を自身に設定する
		} else if( NULL != pc_Graphic ) {
			// グラフィック系のボタン共通パーツの場合
			pc_Graphic->setParentActivate( act );	// 親の選択状態表示を自身に設定する
		} else {
			PnlComPutDebugLog( True, UC_PNL_LOGPATTERN_1, "PActivatableAdd::activate", UC_PNL_ERROR_IMPOSSIBLE );
			return;
		}
	}
	if( True == isNowShown() ) {
		// 現在、表示画面上に表示中の時
		showParts();	// 実パーツを表示する
	}
}
/*================================================================================================
［機能］	表示活性化状態を更新する（テキスト系のボタン共通パーツは表示可能状態の切り替え）
--------------------------------------------------------------------------------------------------
［引数］	Bool act : 表示活性化状態（True:表示活性化状態／False:非表示活性化状態）
［戻値］	無し
================================================================================================*/
void PObjectAdd::activateNonDisp( Bool act )
{
	DEF_UPDATEDRAWPARTSINFO;		// comment 2016/03/30 OPS1-Sky長尾芳人 [操作性改善][不要描画] 不要描画検出機構対応.
	
	register Bool nowActivate = False;
	if( flag & IsActivate ) {
		nowActivate = True;
	}
	if( nowActivate == act ) {
		// 表示活性化状態が変化しない時
		return;
	}
	if( True == act ) {
		flag |= IsActivate;
	} else {
		flag &= (uchar)( ~IsActivate );
	}

	GFWC_Common* pc_GUISelf = static_cast<GFWC_Common*>( getGUIParts() );
	if( NULL != pc_GUISelf ) {
		pc_GUISelf->setActivation( True == act ? GFWD_TRUE : GFWD_FALSE );
		DEF_UPDATEDRAWPARTSINFO_GUI;		// comment 2016/03/30 OPS1-Sky長尾芳人 [操作性改善][不要描画] 不要描画検出機構対応.
	}

	for( uchar i = 0; uc_PartsCount > i; ++i ) {
		PSinglePartsText* pc_Text = dynamic_cast< PSinglePartsText* >( ppc_AddParts[ i ] );
		PSinglePartsGraphic* pc_Graphic = dynamic_cast< PSinglePartsGraphic* >( ppc_AddParts[ i ] );
		if( NULL != pc_Text ) {
			// テキスト系のボタン共通パーツの場合
			pc_Text->setParentActivateNonDisp( act );	// 親の表示活性化状態を自身に設定する（表示可能状態の切り替え）
		} else if( NULL != pc_Graphic ) {
			// グラフィック系のボタン共通パーツの場合
			pc_Graphic->setParentActivateNonDisp( act );	// 親の選択状態表示を自身に設定する
		} else {
			PnlComPutDebugLog( True, UC_PNL_LOGPATTERN_1, "PActivatableAdd::activateNonDisp", UC_PNL_ERROR_IMPOSSIBLE );
			return;
		}
	}
}
/*================================================================================================
［機能］	ボタン共通パーツに対してフォントサイズを変更する
--------------------------------------------------------------------------------------------------
［引数］	PSingleParts* 			pc_AddParts		: ボタン共通パーツ
			const PNLE_SELECT_FONT	e_SelectFont	: フォント指定
［戻値］	無し
================================================================================================*/
void PObjectAdd::changeFontsize( PSingleParts* pc_AddParts, PNLE_SELECT_FONT e_SelectFont )
{
	DEF_UPDATEDRAWPARTSINFO;		// comment 2016/03/30 OPS1-Sky長尾芳人 [操作性改善][不要描画] 不要描画検出機構対応.
	
	// 引数で指定したボタン共通パーツが不正な場合
	if( False == checkAddParts( pc_AddParts ) ) {
		PnlComPutDebugLog( True, UC_PNL_LOGPATTERN_1, "PObjectAdd::changeFontsize", UC_PNL_ERROR_PARAMETER );
		return;
	}
	// ボタン共通パーツを元のクラスにキャスト.
	PTextFieldEx* pc_TextField = dynamic_cast< PTextFieldEx* >( pc_AddParts );
	PNumberFieldEx* pc_NumberField = dynamic_cast< PNumberFieldEx* >( pc_AddParts );
	if( NULL != pc_TextField ) {								// PTextFieldExの場合.
		if( True == pc_TextField->isNeedRenewFontsize( e_SelectFont ) ) {		// 更新の必要がある場合のみ変更する.
			pc_TextField->changeFontsize( e_SelectFont );
		}
	} else if( NULL != pc_NumberField ) {						// PNumberFieldExの場合.
		if( True == pc_NumberField->isNeedRenewFontsize( e_SelectFont ) ) {	// 更新の必要がある場合のみ変更する.
			pc_NumberField->changeFontsize( e_SelectFont );
		}
	} else {
		// 上記テキスト系ボタン共通パーツに本関数を使用した場合
		PnlComPutDebugLog( True, UC_PNL_LOGPATTERN_1, "PObjectAdd::changeFontsize", UC_PNL_ERROR_USE );
		return;
	}
	// 現在、表示画面上に表示中の時
	if( isNowShown() == True ) {
		showParts();	// 実パーツを表示する.
	}
}
/*================================================================================================
［機能］	ボタン共通パーツに対してフォントサイズを元のサイズに戻す
--------------------------------------------------------------------------------------------------
［引数］	PSingleParts* 			pc_AddParts		: ボタン共通パーツ
［戻値］	無し
================================================================================================*/
void PObjectAdd::returnFontsize( PSingleParts* pc_AddParts )
{
	DEF_UPDATEDRAWPARTSINFO;		// comment 2016/03/30 OPS1-Sky長尾芳人 [操作性改善][不要描画] 不要描画検出機構対応.
	
	// 引数で指定したボタン共通パーツが不正な場合
	if( False == checkAddParts( pc_AddParts ) ) {
		PnlComPutDebugLog( True, UC_PNL_LOGPATTERN_1, "PObjectAdd::changeFontsize", UC_PNL_ERROR_PARAMETER );
		return;
	}
	// ボタン共通パーツを元のクラスにキャスト.
	PTextFieldEx* pc_TextField = dynamic_cast< PTextFieldEx* >( pc_AddParts );
	PNumberFieldEx* pc_NumberField = dynamic_cast< PNumberFieldEx* >( pc_AddParts );
	if( NULL != pc_TextField ) {								// PTextFieldExの場合.
		if( True == pc_TextField->isNeedRenewReturnFontsize() ) {		// 更新の必要がある場合のみ変更する.
			pc_TextField->returnFontsize();
		}
	} else if( NULL != pc_NumberField ) {						// PNumberFieldExの場合.
		if( True == pc_NumberField->isNeedRenewReturnFontsize() ) {		// 更新の必要がある場合のみ変更する.
			pc_NumberField->returnFontsize();
		}
	} else {
		// 上記テキスト系ボタン共通パーツに本関数を使用した場合
		PnlComPutDebugLog( True, UC_PNL_LOGPATTERN_1, "PObjectAdd::returnFontsize", UC_PNL_ERROR_USE );
		return;
	}
	// 現在、表示画面上に表示中の時
	if( isNowShown() == True ) {
		showParts();	// 実パーツを表示する.
	}
}
/*================================================================================================
［機能］	ボタン共通パーツに対してカーソルタイプを設定する
--------------------------------------------------------------------------------------------------
［引数］	PSingleParts* 			pc_AddParts		: ボタン共通パーツ
［戻値］	無し
================================================================================================*/
void PObjectAdd::setCursorType( PSingleParts* pc_AddParts, PNLD_CURSOR_TYPE uc_CurOption )
{
	DEF_UPDATEDRAWPARTSINFO;		// comment 2016/03/30 OPS1-Sky長尾芳人 [操作性改善][不要描画] 不要描画検出機構対応.
	
	// 引数で指定したボタン共通パーツが不正な場合
	if( False == checkAddParts( pc_AddParts ) ) {
		PnlComPutDebugLog( True, UC_PNL_LOGPATTERN_1, "PObjectAdd::changeFontsize", UC_PNL_ERROR_PARAMETER );
		return;
	}
	// ボタン共通パーツを元のクラスにキャスト.
	PNumberFieldEx* pc_NumberField = dynamic_cast< PNumberFieldEx* >( pc_AddParts );
	if( NULL != pc_NumberField ) {												// PNumberFieldExの場合.
		if( True == pc_NumberField->isNeedRenewCursorType( uc_CurOption ) ) {		// 更新の必要がある場合のみ変更する.
			pc_NumberField->setCursorType( uc_CurOption );
		}
	} else {
		// 上記テキスト系ボタン共通パーツに本関数を使用した場合
		PnlComPutDebugLog( True, UC_PNL_LOGPATTERN_1, "PObjectAdd::returnFontsize", UC_PNL_ERROR_USE );
		return;
	}
	// 現在、表示画面上に表示中の時
	if( isNowShown() == True ) {
		showParts();	// 実パーツを表示する.
	}
}

/*================================================================================================
［機能］	表示の更新が必要かを得る .
--------------------------------------------------------------------------------------------------
［引数］	PNLS_Parts s_Parts[PNLD_PartsNum] : 表示パーツ.
［戻値］	無し
 [変更履歴]	2013/08/19 MSE 佐々木 GUIフレームワーク(GUIパーツ未生成時のデータアクセス制御)対応.
================================================================================================*/
Bool PObjectAdd::isNeedRenewParts( PNLS_Parts s_Parts[PNLD_PartsNum] )
{
	DEF_UPDATEDRAWPARTSINFO;		// comment 2016/03/30 OPS1-Sky長尾芳人 [操作性改善][不要描画] 不要描画検出機構対応.
	
	Bool b_isChange = False;
	for( uchar j = 0; j < PNLD_PartsNum; j++ ) {
		if( m_sParts[j].puc_Parts != s_Parts[j].puc_Parts ) {
			b_isChange = True;
			break;
		}
	}
	return b_isChange;
}
/*================================================================================================
［機能］	パーツをリセットする
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	無し
================================================================================================*/
void PObjectAdd::resetParts()
{
	DEF_UPDATEDRAWPARTSINFO;		// comment 2016/03/30 OPS1-Sky長尾芳人 [操作性改善][不要描画] 不要描画検出機構対応.
	
	PActivatableAdd::resetParts();	// パーツをリセットする
	setParts();
}

/*================================================================================================
［機能］	表示サイズをGUIパーツに反映.
--------------------------------------------------------------------------------------------------
［戻値］	無し.
================================================================================================*/
void PObjectAdd::setSizeToGUIparts( void )
{
	DEF_UPDATEDRAWPARTSINFO;		// comment 2016/03/30 OPS1-Sky長尾芳人 [操作性改善][不要描画] 不要描画検出機構対応.
	
	PObjectAdd::setSizeToGUIparts( m_eShadow );
}

/*================================================================================================
［機能］	表示サイズをGUIパーツに反映(影パターン指定).
--------------------------------------------------------------------------------------------------
［引数］	PNLE_Shadow : 影パターン.
［戻値］	無し.
================================================================================================*/
void PObjectAdd::setSizeToGUIparts( PNLE_Shadow e_Shadow )
{
	DEF_UPDATEDRAWPARTSINFO;		// comment 2016/03/30 OPS1-Sky長尾芳人 [操作性改善][不要描画] 不要描画検出機構対応.
	
	GFWC_Common* pc_GUISelf = static_cast<GFWC_Common*>( getGUIParts() );
	if( NULL != pc_GUISelf ) {
		short ss_Width = 0;
		short ss_Height = 0;
		getPartsSize( ss_Width, ss_Height );

		// 影属性があるので、その分を考慮したサイズを設定する
		pc_GUISelf->setDrawArea( GFWC_Rect( 0, 0, -1, -1 ) );
		pc_GUISelf->setSize( addShadowSize( ss_Width, ss_Height, e_Shadow ) );
		DEF_UPDATEDRAWPARTSINFO_GUI;		// comment 2016/03/30 OPS1-Sky長尾芳人 [操作性改善][不要描画] 不要描画検出機構対応.
	}
}
