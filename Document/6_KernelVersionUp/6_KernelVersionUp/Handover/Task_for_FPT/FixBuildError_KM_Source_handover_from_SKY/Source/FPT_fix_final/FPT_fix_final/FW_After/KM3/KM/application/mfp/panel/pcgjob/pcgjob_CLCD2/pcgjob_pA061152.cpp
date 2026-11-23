//------------------------------------------------------------------------------
/**
 * @file pcgjob_pA061152.cpp
 * @brief 画面クラスの実装（ポップアップ画面）.
 */
//------------------------------------------------------------------------------

//================================================================================================
//  インクルード・ファイル.
//================================================================================================
#include <panel/plibh/plibh_CLCD/plibh_Mtui.h>
#include <system/ats/APIC_AutoSelect.h>
#include <panel/pcgjob/pcgjob_CLCD2/pcgjob_pA061152.h>
#include <panel/pcgjob/pcgjob_CLCD2/pigjob_pA061152.h>
#include <panel/pcgjob/pcgjob_CLCD2/pipjob_pA061152.h>
#include <panel/plibh/plibhg/pdpcomb.h>
#include <panel/pcgcom/pcgcom_CLCD2/PNLC_PopupListControl.h>
#include <panel/plibh/plibh_CLCD2/plibh_CLCD2.h>
#include <panel/plibh/plibh_CLCD2/plibhinclude_CLCD2.h>
#include <panel/pcgwarn/pcgwarn_CLCD2/pigwarn_pA0A3040.h>
#include <panel/plibh/plibh_CLCD2/PNLC_FuncPatternSettingLib.h>
#include <panel/pcgjob/pcgjob_CLCD2/pigjob_pA061150.h>

#include <panel/pcgjob/pcgjob_CLCD2/pigjob_pA061150.h>
#include <system/saa/APIC_UsbMemDeviceInfo.h>	// MinervaZeus Sky桐島 2010/09/16 [72版新UI化].
#include <panel/plibh/plibh_CLCD2/piglib_pA010194.h>
#include <panel/plibh/plibh_CLCD2/PNLC_OutsideKeyCallDataLib.h>
#include <panel/pcgjob/pcgjob_CLCD2/pigjob_pA061155.h>
#include <panel/plibh/plibh_CLCD2/PNLC_SlideMenuControlLib.h>
#include "panel/plibh/plibh_CLCD2/PNLC_SoftSlideMenuTenKeyTextDispLib.h"
#include <panel/pcgjob/pcgjob_PD/pcgjob_A061152110.h>
#include <panel/pcgjob/pcgjob_PD/pcgjob_A061152210.h>
#include <panel/pcgjob/pcgjob_PD/pcgjob_A061152310.h>
#include <panel/pcgjob/pcgjob_PD/pcgjob_B061152420.h>
extern PA061152110 gp_A061152110;
extern PA061152210 gp_A061152210;
extern PA061152310 gp_A061152310;
extern PB061152420 gp_B061152420;
#include <panel/pcgjob/pcgjob_PD/pcgjob_B061152410.h>
extern PB061152410 gp_B061152410;

/*================================================================================================
  列挙型変数のタグ宣言.
================================================================================================*/
static const struct PNLS_CheckFeedTrayTable_CLCD {
	TYPE_FeedTray    e_FeedTray;                  // 給紙口.
	TYPE_TemporaryRescueUnit e_TemporaryRescueUnit;// 一時切り離し対象.
} s_checkFeedTrayTable_CLCD[] = {
	{ TYPE_FeedTrayManual, TYPE_TemporaryRescueUnitTrayManual }, // 手差し.
	{ TYPE_FeedTray1,  TYPE_TemporaryRescueUnitTray1  }, // トレイ1.
	{ TYPE_FeedTray2,  TYPE_TemporaryRescueUnitTray2  }, // トレイ2.
	{ TYPE_FeedTray3,  TYPE_TemporaryRescueUnitTray3  }, // トレイ3.
	{ TYPE_FeedTray4,  TYPE_TemporaryRescueUnitTray4  }, // トレイ4.
// comment Sky森本（弘） 2018/05/18 [DenebMLK][機器構成変更_No.7] 5段増設プリント機能追加対応．
#ifdef DEF_DenebMLKCus
	{ TYPE_FeedTray5,  TYPE_TemporaryRescueUnitTray5  }, // トレイ5.
#else
	{ TYPE_FeedTray5,  TYPE_TemporaryRescueUnitTrayLCT  }, // LCT.
#endif	// DEF_DenebMLKCus
};

/*================================================================================================
 マクロ関数の定義.
================================================================================================*/

//関数------------------------------------------------------------------------------------------------------------------
//概要    ：
//備考    ：
//戻値    ：
//作成者  ：
//レビュー：
//変更履歴：
void PCPC_AuxilPageCover_job::startGamen()
{
	PPopupJobSetupChangeCom::startGamen();

	m_ss_JobSpecBackupTableNo = -1;

	m_b_FunctionOffOnFlg = isOn( CNK_CoverMode, PnlComGetJobCopy() );

	m_b_InitFeedTray = !m_b_FunctionOffOnFlg;//<HCL>  334698 <30/11/2010> Add.

	// ON/OFFキーの選択状態変更.
	Cmp_AuxilPageCover_job_MessageBand.setOnOffButton( True );
	Cmp_AuxilPageCover_job_MessageBand.setOnOffSwitchButtonState( m_b_FunctionOffOnFlg );	//ON/OFFキーの選択状態変更.
	Cmp_AuxilPageCover_job_MessageBand.showDestinationCount( False );
	Cmp_AuxilPageCover_job_MessageBand.attachMessage( True );
	Cmp_AuxilPageCover_job_MessageBand.setMode( PNLE_PopupMessageAreaMessageModeAdmin );
	Cmp_AuxilPageCover_job_MessageBand.setNowPopupGamen( PNLE_AuxilPageCover_job );
//Comment 2020/05/19 HCL-Madhavi  [IT6_HeliosMLK][OP_BTS-23959] uninstall tray rest to available tray.		
	TYPC_IntersheetMode c_CoversheetBack = PnlComGetJobCopy()->getJobSpec()->pc_Print->c_Intersheet.getBackMode();// ウラ表紙の取得.
	TYPC_IntersheetMode c_CoversheetFront = PnlComGetJobCopy()->getJobSpec()->pc_Print->c_Intersheet.getFrontMode();// オモテ表紙の取得.
	if( False == m_b_FunctionOffOnFlg) {	// ON/OFF: ON.
		c_CoversheetFront.setFeedTray( getCoversheetFeedTray() );							// オモテ 表紙の選択トレイを変更.
		c_CoversheetBack.setFeedTray( getCoversheetFeedTray() );								// ウラ 表紙の選択トレイを変更.
		PnlComGetJobCopy()->getJobSpec()->pc_Print->c_Intersheet.setFrontMode( c_CoversheetFront );	// 変更されたオモテ 表紙の選択トレイを設定.
		PnlComGetJobCopy()->getJobSpec()->pc_Print->c_Intersheet.setBackMode( c_CoversheetBack );	// 変更されたウラ 表紙の選択トレイを設定.
	}

}
//Comment 2020/05/19 HCL-Madhavi  [IT6_HeliosMLK][OP_BTS-23959] uninstall tray rest to available tray.		
//カバー用紙の給紙口取得.
TYPE_FeedTray PCPC_AuxilPageCover_job::getCoversheetFeedTray()
{
	static const struct PNLS_CheckFeedTrayTable_CLCD {
		TYPE_FeedTray				e_FeedTray;				// 給紙口.
		TYPE_TemporaryRescueUnit	e_TemporaryRescueUnit;	// 一時切り離し対象.
	} st_checkFeedTrayTable_CLCD[] = {
		{ TYPE_FeedTrayManual,	TYPE_TemporaryRescueUnitTrayManual	},	// 手差し.
		{ TYPE_FeedTray1,		TYPE_TemporaryRescueUnitTray1		},	// トレイ1.
		{ TYPE_FeedTray2,		TYPE_TemporaryRescueUnitTray2		},	// トレイ2.
		{ TYPE_FeedTray3,		TYPE_TemporaryRescueUnitTray3		},	// トレイ3.
		{ TYPE_FeedTray4,		TYPE_TemporaryRescueUnitTray4		},	// トレイ4.
#ifdef DEF_DenebMLKCus
		{ TYPE_FeedTray5,		TYPE_TemporaryRescueUnitTray5		},	// トレイ5.
#else
		{ TYPE_FeedTray5,		TYPE_TemporaryRescueUnitTrayLCT		},	// LCT .
#endif	// DEF_DenebMLKCus.
	};
	static const ushort us_CheckFeedTraySize = static_cast<uchar>( sizeof( st_checkFeedTrayTable_CLCD ) / sizeof( PNLS_CheckFeedTrayTable_CLCD ) );

	TYPE_FeedTray e_FeedTray = PnlComGetOperationSetting()->getInsertFeedTray();	// デフォルトはユーティリティの設定から取得.
	if( TYPE_TrayTypeNon == PnlComGetFeedTray()->getTrayType( e_FeedTray ) || TYPE_TemporaryRescueStatusSet == PnlComGetSystemStatus()->getTemporaryRescueUserStatus( st_checkFeedTrayTable_CLCD[e_FeedTray].e_TemporaryRescueUnit ) ) { // 指定されたトレイが装着されていない or 暫定救済中の場合はテーブルの優先順に従い、使用可能なトレイに切り替える.
		for( ushort i = 0; i < us_CheckFeedTraySize; i++ ) {
			if( TYPE_TrayTypeNon != PnlComGetFeedTray()->getTrayType( st_checkFeedTrayTable_CLCD[i].e_FeedTray )
			 && TYPE_TemporaryRescueStatusReset == PnlComGetSystemStatus()->getTemporaryRescueUserStatus( st_checkFeedTrayTable_CLCD[i].e_TemporaryRescueUnit ) ) {
				e_FeedTray = st_checkFeedTrayTable_CLCD[i].e_FeedTray;	// 選択トレイを設定.
				break;
			}
		}
	}
	return e_FeedTray;
}
//関数------------------------------------------------------------------------------------------------------------------
//概要    ：
//備考    ：
//戻値    ：
//作成者  ：
//レビュー：
//変更履歴：
void PCPC_AuxilPageCover_job::setupGamen( short option )
{
	// 親クラスの処理をコール.
	PPopupJobSetupChangeCom::setupGamen( option );

	if( NULL == PnlComGetJobCopy() ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }
	TYPC_Print* pc_Print = PnlComGetJobCopy()->getJobSpec()->pc_Print;

	// オモテ表紙の取得.
	TYPC_IntersheetMode c_CoversheetFront = pc_Print->c_Intersheet.getFrontMode();

	// ウラ表紙の取得.
	TYPC_IntersheetMode c_CoversheetBack = pc_Print->c_Intersheet.getBackMode();

	if( m_b_FunctionOffOnFlg == True ) {

		short ss_SelectFront = -1;
		// オモテ表紙の用紙挿入モードを取得し、ローテーションボタンのインデックスを設定.
		switch( c_CoversheetFront.getPaperInsertion() ) {
			case TYPE_PaperInsertionOff:   ss_SelectFront        = PNLD_AuxilPageCover_job_Index_None; break;  // オモテ表紙なし.
			case TYPE_PaperInsertionWithImage:  ss_SelectFront   = PNLD_AuxilPageCover_job_Index_Copy; break;  // オモテコピー.
			case TYPE_PaperInsertionWithoutImage: ss_SelectFront = PNLD_AuxilPageCover_job_Index_Blank; break; // オモテ白紙.
			default: LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return;
		}

		// オモテ表紙の用紙挿入モードを設定.
		Rbt_AuxilPageCover_job_FrontCover.select( ss_SelectFront );

		// ウラ表紙の取得.

		short ss_SelectBack = -1;
		// ウラ表紙の用紙挿入モードを取得し、ローテーションボタンのインデックスを設定.
		switch( c_CoversheetBack.getPaperInsertion() ) {
			case TYPE_PaperInsertionOff:   ss_SelectBack        = PNLD_AuxilPageCover_job_Index_None; break;  // ウラ表紙なし.
			case TYPE_PaperInsertionWithImage:  ss_SelectBack   = PNLD_AuxilPageCover_job_Index_Copy; break;  // ウラコピー.
			case TYPE_PaperInsertionWithoutImage: ss_SelectBack = PNLD_AuxilPageCover_job_Index_Blank; break; // ウラ白紙.
			default: LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return;
		}

		// ウラ表紙の用紙挿入モードを設定.
		Rbt_AuxilPageCover_job_BackCover.select( ss_SelectBack );


		// ON／OFFボタンの表示処理.
		setupGamenDoDont();

		Txt_AuxilPageCover_job_FrontCover.activate( True );
		Txt_AuxilPageCover_job_BackCover.activate( True );

	} else {
		// キー非活性.
		Rbt_AuxilPageCover_job_FrontCover.activate( False );
		Rbt_AuxilPageCover_job_BackCover.activate( False );
		Key_AuxilPageCover_job_FrontPaper.activate( False );
		Key_AuxilPageCover_job_BackPaper.activate( False );
		Txt_AuxilPageCover_job_FrontCover.activate( False );
		Txt_AuxilPageCover_job_BackCover.activate( False );
		Key_AuxilPageCover_job_OK.activate( True );
	}

	// オモテ表示・ウラ表紙の選択トレイの表示処理のライブラリ関数をコールする.
	// オモテ表示.
	PNLS_setOriginalSizeCLCD2Key s_Front_parts;
	s_Front_parts.ps_KeyOriginalSize		=	&Key_AuxilPageCover_job_FrontPaper;			// キー.
	s_Front_parts.ps_TxtOriginalSize01		=	&Ktx_AuxilPageCover_job_FrontPaper1;			// １行目 用紙コード表示.
	s_Front_parts.ps_TxtOriginalSize02		=	NULL;										// ２行目 自動表示.
	s_Front_parts.ps_TxtOriginalSizeX		=	&Ktx_AuxilPageCover_job_FrontPaperX;			// 用紙サイズ表示 X.
	s_Front_parts.ps_TxtOriginalSizeY		=	&Ktx_AuxilPageCover_job_FrontPaperY;			// 用紙サイズ表示 Y.
	s_Front_parts.ps_NumOriginalSizeX		=	&Tfd_AuxilPageCover_job_FrontPaperX;			// 用紙サイズ数値表示 X.
	s_Front_parts.ps_NumOriginalSizeY		=	&Tfd_AuxilPageCover_job_FrontPaperY;			// 用紙サイズ数値表示 Y.
	s_Front_parts.ps_TrayTxtPaperSize		=	&Ktx_AuxilPageCover_job_FrontPaper2;			// トレイ表示テキスト.
	s_Front_parts.ps_TrayIconPaperSize		=	&Gra_AuxilPageCover_job_FrontPaperFree;		// トレイ表示アイコン（手差しアイコン）.
	s_Front_parts.ps_RemainIconPaperSize	=	&Gra_AuxilPageCover_job_FrontPaperLevel;
	s_Front_parts.ps_TypeIconPaperSize		=	&Gra_AuxilPageCover_job_FrontPaperType;

	// ボタン表示内容を変更.
//Comment 2020/05/19 HCL-Madhavi  [IT6_HeliosMLK][OP_BTS-23959] uninstall tray rest to available tray.		
//	TYPE_FeedTray e_FeedTray_Front = ( True == m_b_InitFeedTray )?  PnlComGetOperationSetting()->getInsertFeedTray() : c_CoversheetFront.getFeedTray();
	TYPE_FeedTray e_FeedTray_Front = c_CoversheetFront.getFeedTray();
	PnlComsetPaperSizeCLCD2Grap( &s_Front_parts, e_FeedTray_Front, PNLE_PaperSizeShowKind_Type1 );

	// ウラ表示.
	PNLS_setOriginalSizeCLCD2Key s_Back_parts;
	s_Back_parts.ps_KeyOriginalSize			=	&Key_AuxilPageCover_job_BackPaper;			// キー.

	s_Back_parts.ps_TxtOriginalSize01		=	&Ktx_AuxilPageCover_job_BackPaper1;			// １行目 用紙コード表示.
	s_Back_parts.ps_TxtOriginalSize02		=	NULL;										// ２行目 自動表示.
	s_Back_parts.ps_TxtOriginalSizeX		=	&Ktx_AuxilPageCover_job_BackPaperX;			// 用紙サイズ表示 X.
	s_Back_parts.ps_TxtOriginalSizeY		=	&Ktx_AuxilPageCover_job_BackPaperY;			// 用紙サイズ表示 Y.
	s_Back_parts.ps_NumOriginalSizeX		=	&Tfd_AuxilPageCover_job_BackPaperX;			// 用紙サイズ数値表示 X.
	s_Back_parts.ps_NumOriginalSizeY		=	&Tfd_AuxilPageCover_job_BackPaperY;			// 用紙サイズ数値表示 Y.
	s_Back_parts.ps_TrayTxtPaperSize		=	&Ktx_AuxilPageCover_job_BackPaper2;			// トレイ表示テキスト.
	s_Back_parts.ps_TrayIconPaperSize		=	&Gra_AuxilPageCover_job_BackPaperFree;				// トレイ表示アイコン（手差しアイコン）.
	s_Back_parts.ps_RemainIconPaperSize		=	&Gra_AuxilPageCover_job_BackPaperLevel;
	s_Back_parts.ps_TypeIconPaperSize		=	&Gra_AuxilPageCover_job_BackPaperType;

	// ボタン表示内容を変更.
//Comment 2020/05/19 HCL-Madhavi  [IT6_HeliosMLK][OP_BTS-23959] uninstall tray rest to available tray.		
//	TYPE_FeedTray e_FeedTray_Back = ( True == m_b_InitFeedTray )?  PnlComGetOperationSetting()->getInsertFeedTray() : c_CoversheetBack.getFeedTray();
	TYPE_FeedTray e_FeedTray_Back = c_CoversheetBack.getFeedTray();
	PnlComsetPaperSizeCLCD2Grap( &s_Back_parts, e_FeedTray_Back, PNLE_PaperSizeShowKind_Type1 );

	// パターンカスタマイズ.
	if( PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_CoverSheet ) ) {		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
		// 表カバー.
		if( PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_CoverSheet_FrontMode ) == False ) {		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
			Rbt_AuxilPageCover_job_FrontCover.showEnable( False );		// 用紙設定ローテーション.
			Key_AuxilPageCover_job_FrontPaper.showEnable( False );		// 用紙キー.
			Txt_AuxilPageCover_job_FrontCover.showEnable( False );		// 背景文字.
			Gra_AuxilPageCover_job_FrontCoverBack.showEnable( False );	// 背景.
		}
		// 表カバーの用紙.
		if( PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_CoverSheet_FrontPaperSetting ) == False ) {		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
			Key_AuxilPageCover_job_FrontPaper.showEnable( False );		// 用紙キー.
		}
		// 裏カバー.
		if( PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_CoverSheet_RearMode ) == False ) {		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
			Rbt_AuxilPageCover_job_BackCover.showEnable( False );		// 用紙設定ローテーション.
			Key_AuxilPageCover_job_BackPaper.showEnable( False );		// 用紙キー.
			Txt_AuxilPageCover_job_BackCover.showEnable( False );		// 背景文字.
			Gra_AuxilPageCover_job_BackCoverBack.showEnable( False );	// 背景.
		}
		// 裏カバーの用紙.
		if( PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_CoverSheet_RearPaperSetting ) == False ) {		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
			Key_AuxilPageCover_job_BackPaper.showEnable( False );		// 用紙キー.
		}
	} else {
		// 表カバー.
		Rbt_AuxilPageCover_job_FrontCover.showEnable( False );		// 用紙設定ローテーション.
		Key_AuxilPageCover_job_FrontPaper.showEnable( False );		// 用紙キー.
		Txt_AuxilPageCover_job_FrontCover.showEnable( False );		// 背景文字.
		Gra_AuxilPageCover_job_FrontCoverBack.showEnable( False );	// 背景.
		// 裏カバー.
		Rbt_AuxilPageCover_job_BackCover.showEnable( False );		// 用紙設定ローテーション.
		Key_AuxilPageCover_job_BackPaper.showEnable( False );		// 用紙キー.
		Txt_AuxilPageCover_job_BackCover.showEnable( False );		// 背景文字.
		Gra_AuxilPageCover_job_BackCoverBack.showEnable( False );	// 背景.
	}
}
//関数------------------------------------------------------------------------------------------------------------------
//概要    ：
//備考    ：
//戻値    ：
//作成者  ：
//レビュー：
//変更履歴：
void PCPC_AuxilPageCover_job::endGamen()
{
	// バックアップデータがある場合.
	if( m_ss_JobSpecBackupTableNo != -1 ) {
		// バックアップしたJobSpecをクリアする.
		PnlComGetCopyDataLib()->JobSpecBackupClear( m_ss_JobSpecBackupTableNo );
		m_ss_JobSpecBackupTableNo = -1;
	}

	// 親クラスの処理をコール.
	PPopupJobSetupChangeCom::endGamen();
}

//関数------------------------------------------------------------------------------------------------------------------
//概要    ：
//備考    ：
//戻値    ：
//作成者  ：
//レビュー：
//変更履歴：
void PCPC_AuxilPageCover_job::setupGamenDoDont()
{
	if( NULL == PnlComGetJobCopy() ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }
	TYPC_Print* pc_Print = PnlComGetJobCopy()->getJobSpec()->pc_Print;
	// オモテ表紙、ウラ表紙の取得.
	TYPC_IntersheetMode c_CoversheetFront = pc_Print->c_Intersheet.getFrontMode();
	TYPC_IntersheetMode c_CoversheetBack = pc_Print->c_Intersheet.getBackMode();
	// 「ON」キー選択、かつ表カバーなし選択、かつ裏カバーなし選択の時、OKキーをグレーアウトにする.
	if( m_b_FunctionOffOnFlg == True &&
	    c_CoversheetFront.getPaperInsertion() == TYPE_PaperInsertionOff &&
	    c_CoversheetBack.getPaperInsertion() == TYPE_PaperInsertionOff ) {
		Key_AuxilPageCover_job_OK.activate( False );
	} else {
		Key_AuxilPageCover_job_OK.activate( True );
	}

	// 各ボタンの活性状態の処理	.
	Rbt_AuxilPageCover_job_FrontCover.activate( m_b_FunctionOffOnFlg );
	Rbt_AuxilPageCover_job_BackCover.activate( m_b_FunctionOffOnFlg );

	Bool b_isActive = m_b_FunctionOffOnFlg;
	if( b_isActive == True && c_CoversheetFront.getPaperInsertion() == TYPE_PaperInsertionOff ) {
		b_isActive = False;
	}

	// 表カバー用紙キーを活性/非活性状態にする.
	Key_AuxilPageCover_job_FrontPaper.activate( b_isActive );

	if( m_b_FunctionOffOnFlg == True && c_CoversheetFront.getPaperInsertion() != TYPE_PaperInsertionOff ) {
		if( c_CoversheetFront.getFeedTray() == TYPE_FeedTrayPI1 ||
		    c_CoversheetFront.getFeedTray() == TYPE_FeedTrayPI2 ) {
			// 表コピーキーをグレーアウトにする.
			Key_AuxilPageCover_job_FrontCopy.activate( False );
		} else {
			// 表コピーキーを活性状態にする.
			Key_AuxilPageCover_job_FrontCopy.activate( True );
		}
	}

	b_isActive = m_b_FunctionOffOnFlg;
	if( b_isActive == True && c_CoversheetBack.getPaperInsertion() == TYPE_PaperInsertionOff ) {
		b_isActive = False;
	}

	// 裏カバー用紙キーを活性/非活性状態にする.
	Key_AuxilPageCover_job_BackPaper.activate( b_isActive );

	if( m_b_FunctionOffOnFlg == True && c_CoversheetBack.getPaperInsertion() != TYPE_PaperInsertionOff ) {
		if( c_CoversheetBack.getFeedTray() == TYPE_FeedTrayPI1 ||
		    c_CoversheetBack.getFeedTray() == TYPE_FeedTrayPI2 ) {
			// 裏コピーキーをグレーアウトにする.
			Key_AuxilPageCover_job_BackCopy.activate( False );
		} else {
			// 裏コピーキーを活性状態にする.
			Key_AuxilPageCover_job_BackCopy.activate( True );
		}
	}

	if( True == isOn( CNK_ProgramJob, PnlComGetJobCopy() ) ) {
		Key_AuxilPageCover_job_FrontCopy.activate( False );
		Key_AuxilPageCover_job_BackCopy.activate( False );

	}
}

//関数------------------------------------------------------------------------------------------------------------------
//概要    ：
//備考    ：
//戻値    ：
//作成者  ：
//レビュー：
//変更履歴：
void PCPC_AuxilPageCover_job::procButtonOn( uchar option )
{
	if( BTNID_KEY_POPUP_OK == option ) {
		procOk( option );
		return;
	}
	short ss_SrcPosX = 0,ss_SrcPosY = 0;	// 表示位置.
	short ss_SrcSizeX = 0 ,ss_SrcSizeY = 0;	// 表示サイズ.

	switch( option ) {
		case PNLD_BTNID_FRONT_COPY: setCoversheet( True, TYPE_PaperInsertionWithImage );  break; // 「表コピー」ボタン押下.
		case PNLD_BTNID_FRONT_BLANK: setCoversheet( True, TYPE_PaperInsertionWithoutImage ); break; // 「表白紙」ボタン押下.
		case PNLD_BTNID_BACK_COPY: // 「裏コピー」ボタン押下.
			// 組み合わせチェックを行う .
			if( combinationCheck_AccumuDoc( CNK_CoverModeBack, PnlComGetJBJobSpec(), PnlComGetJobCopy() ) &&
			    combinationCheck( CNK_CoverModeBack, PnlComGetJobCopy() ) == True ) {
				// 表紙の設定.
				setCoversheet( False, TYPE_PaperInsertionWithImage );
			}
			break;
		case PNLD_BTNID_BACK_BLANK: // 「裏白紙」ボタン押下.
			// 組み合わせチェックを行う .
			if( combinationCheck_AccumuDoc( CNK_CoverModeBack, PnlComGetJBJobSpec(), PnlComGetJobCopy() ) &&
			    combinationCheck( CNK_CoverModeBack, PnlComGetJobCopy() ) == True ) {
				// 表紙の設定.
				setCoversheet( False, TYPE_PaperInsertionWithoutImage );
			}
			break;
		case PNLD_BTNID_FRONT_NOTDO: setCoversheet( True, TYPE_PaperInsertionOff ); break; // しない（表表紙）.
		case PNLD_BTNID_BACK_NOTDO: setCoversheet( False, TYPE_PaperInsertionOff ); break; // しない（裏表紙）.
		case PNLD_BTNID_FRONT_PAPER:
			if( NULL == PnlComGetJobCopy() ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }
			if( PnlComGetJobCopy()->getJobSpec()->pc_Print->c_Intersheet.getFrontMode().getPaperInsertion() != TYPE_PaperInsertionOff ) {
				// カバー用紙選択オモテ表紙トレイ選択フラグを設定.
				gp_A061152110.setCoversheetFront( True );
				Key_AuxilPageCover_job_FrontPaper.getPosition( ss_SrcPosX, ss_SrcPosY );
				Key_AuxilPageCover_job_FrontPaper.getSize( ss_SrcSizeX, ss_SrcSizeY );
				// 画面遷移.
				PNLC_PopupListControl::newInstance()->addPopup( &gp_A061152110, this, NULL, ss_SrcPosX,ss_SrcPosY,ss_SrcSizeX,ss_SrcSizeY );
				return;
			} else {
				// 画面ブザー禁止.
				setGamenBuzzerCancel();
			}
			break;
		case PNLD_BTNID_BACK_PAPER:
			if( isOn( CNK_CoverModeBack, PnlComGetJobCopy() ) == True ) {
				if( False == isOn( CNK_Magazine, PnlComGetJobCopy() ) &&
				    ( isOn( CNK_Centerori, PnlComGetJobCopy() ) == False ) &&
				    ( isOn( CNK_CenterBinding, PnlComGetJobCopy() ) == False ) &&
				    ( isOn( CNK_CenterStaple, PnlComGetJobCopy() ) == False ) ) {
					// カバー用紙選択オモテ表紙トレイ選択フラグを設定.
					gp_A061152110.setCoversheetFront( False );
					Key_AuxilPageCover_job_BackPaper.getPosition( ss_SrcPosX, ss_SrcPosY );
					Key_AuxilPageCover_job_BackPaper.getSize( ss_SrcSizeX, ss_SrcSizeY );
					// 画面遷移.
					PNLC_PopupListControl::newInstance()->addPopup( &gp_A061152110, this, NULL, ss_SrcPosX,ss_SrcPosY,ss_SrcSizeX,ss_SrcSizeY );
					return;
				} else {
					// 画面ブザー禁止.
					setGamenBuzzerCancel();
				}
			} else {
				// 画面ブザー禁止.
				setGamenBuzzerCancel();
			}
			break;
		// ON選択
		case PNLE_PopupMessageAreaButtonType_On:
			if( ( True == combinationCheck_AccumuDoc( CNK_CoverMode, PnlComGetJBJobSpec(), PnlComGetJobCopy() ) )
			    && ( True == combinationCheck( CNK_CoverMode, PnlComGetJobCopy() ) ) ) {
				// ON状態を設定する.
				setOn();
			}
			else {
				// 組合せ禁止のため警告画面を表示し、[OFF] ボタンを選択状態にする.
				Cmp_AuxilPageCover_job_MessageBand.setOnOffSwitchButtonState( m_b_FunctionOffOnFlg );	//ON/OFFキーの選択状態変更.
			}
			break;

		// OFF選択
		case PNLE_PopupMessageAreaButtonType_Off:
			// OFF状態を設定する.
			setOff();
			break;

		default: LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return;
	}

	setupGamen();

}

//関数------------------------------------------------------------------------------------------------------------------
//概要    ：
//備考    ：
//戻値    ：
//作成者  ：
//レビュー：
//変更履歴：
void PCPC_AuxilPageCover_job::setOn()
{
	static const struct PNLS_CheckFeedTrayTable_CLCD {
		TYPE_FeedTray    e_FeedTray;                  // 給紙口.
		TYPE_TemporaryRescueUnit e_TemporaryRescueUnit;// 一時切り離し対象.
	} s_checkFeedTrayTable_CLCD[] = {
		{ TYPE_FeedTrayManual, TYPE_TemporaryRescueUnitTrayManual }, // 手差し.
		{ TYPE_FeedTray1,  TYPE_TemporaryRescueUnitTray1  }, // トレイ1.
		{ TYPE_FeedTray2,  TYPE_TemporaryRescueUnitTray2  }, // トレイ2.
		{ TYPE_FeedTray3,  TYPE_TemporaryRescueUnitTray3  }, // トレイ3.
		{ TYPE_FeedTray4,  TYPE_TemporaryRescueUnitTray4  }, // トレイ4.
// comment Sky森本（弘） 2018/05/18 [DenebMLK][機器構成変更_No.7] 5段増設プリント機能追加対応．
#ifdef DEF_DenebMLKCus
		{ TYPE_FeedTray5,  TYPE_TemporaryRescueUnitTray5  }, // トレイ5.
#else
		{ TYPE_FeedTray5,  TYPE_TemporaryRescueUnitTrayLCT  }, // LCT .
#endif	// DEF_DenebMLKCus
	};
	if( NULL == PnlComGetJobCopy() ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }
	// バックアップデータがある場合.
	if( m_ss_JobSpecBackupTableNo != -1 ) {
		// JobSpecを取得する.
		TYPC_JobSpec* pc_JobSpec = PnlComGetJobCopy()->getJobSpec();
		// バックアップしたJobSpecに戻す.
		PnlComGetCopyDataLib()->setJobSpecBackup( m_ss_JobSpecBackupTableNo , pc_JobSpec );
		// バックアップしたJobSpecをクリアする.
		PnlComGetCopyDataLib()->JobSpecBackupClear( m_ss_JobSpecBackupTableNo );
		m_ss_JobSpecBackupTableNo = -1;
	} else {
		// Print情報の取得.
		TYPC_Print* pc_Print = PnlComGetJobCopy()->getJobSpec()->pc_Print;
		// 表/裏 表紙の取得.
		TYPC_IntersheetMode c_CoversheetFront = pc_Print->c_Intersheet.getFrontMode();
		TYPC_IntersheetMode c_CoversheetBack  = pc_Print->c_Intersheet.getBackMode();
		// カバーシートの設定状態を判定.
		if( False == isOn( CNK_CoverMode, PnlComGetJobCopy() ) ) {
			APIC_OperationSetting* pc_OperationSetting = PnlComGetOperationSetting();
			// デフォルトはユーティリティの設定から取得.
			TYPE_FeedTray e_FeedTray = pc_OperationSetting->getInsertFeedTray();
			if( ppc_FeedTray->getTrayType( e_FeedTray ) == TYPE_TrayTypeNon ||
			    ppc_SystemStatus->getTemporaryRescueUserStatus( s_checkFeedTrayTable_CLCD[e_FeedTray].e_TemporaryRescueUnit ) == TYPE_TemporaryRescueStatusSet ) {
//comment 2018/5/18 SCD2-Sky肥後 [DenebMLK][機器構成変更No.7]5段目トレイ追加対応.
//				uchar uc_CheckFeedTraySize = ( uchar )( sizeof( s_checkFeedTrayTable_CLCD )/sizeof( s_checkFeedTrayTable_CLCD ) );
				uchar uc_CheckFeedTraySize = static_cast<uchar>( sizeof( s_checkFeedTrayTable_CLCD )/sizeof( s_checkFeedTrayTable_CLCD[0] ) );
				for( uchar i = 0; i < uc_CheckFeedTraySize; i++ ) {
					if( ppc_FeedTray->getTrayType( s_checkFeedTrayTable_CLCD[i].e_FeedTray ) != TYPE_TrayTypeNon &&
					    ppc_SystemStatus->getTemporaryRescueUserStatus( s_checkFeedTrayTable_CLCD[i].e_TemporaryRescueUnit ) == TYPE_TemporaryRescueStatusReset ) {
						// 選択トレイを設定.
						e_FeedTray = s_checkFeedTrayTable_CLCD[i].e_FeedTray;
						break;

					}
				}
			}
			// オモテ・ウラ表紙の選択トレイを変更.
			c_CoversheetFront.setFeedTray( e_FeedTray );
			c_CoversheetBack.setFeedTray( e_FeedTray );

			// デフォルト設定、オモテ表紙をONする.
			c_CoversheetFront.setPaperInsertion( TYPE_PaperInsertionWithImage ); // オモテ表紙.

			// 変更されたオモテ・ウラ表紙の選択トレイを設定.
			pc_Print->c_Intersheet.setFrontMode( c_CoversheetFront );
			pc_Print->c_Intersheet.setBackMode( c_CoversheetBack );

		}

		m_b_InitFeedTray = False;	// 初期化完了.//<HCL>  334698 <30/11/2010> Add.
	}

	// ON状態を設定
	m_b_FunctionOffOnFlg = True;
}

//関数------------------------------------------------------------------------------------------------------------------
//概要    ：
//備考    ：
//戻値    ：
//作成者  ：
//レビュー：
//変更履歴：
void PCPC_AuxilPageCover_job::setOff()
{
	// JobSpecのデータを保存する.
	if( NULL == PnlComGetJobCopy() ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }
	TYPC_JobSpec* pc_JobSpec = PnlComGetJobCopy()->getJobSpec();
	// JobSpecのデータを保存する.
	m_ss_JobSpecBackupTableNo = PnlComGetCopyDataLib()->saveJobSpecBackup( m_ss_JobSpecBackupTableNo, pc_JobSpec );

	// 表紙の取消し.
	cancel( CNK_CoverMode, CNK_Size, PnlComGetJobCopy() );
	// OFF状態を設定.
	m_b_FunctionOffOnFlg = False;
}

//関数------------------------------------------------------------------------------------------------------------------
//概要    ：
//備考    ：
//戻値    ：
//作成者  ：
//レビュー：
//変更履歴：
void PCPC_AuxilPageCover_job::setCoversheet( const Bool b_Front, const TYPE_PaperInsertion e_PaperInsertion )
{
	static const struct PNLS_CheckFeedTrayTable_CLCD {
		TYPE_FeedTray    e_FeedTray;                  // 給紙口.
		TYPE_TemporaryRescueUnit e_TemporaryRescueUnit;// 一時切り離し対象.
	} s_checkFeedTrayTable_CLCD[] = {
		{ TYPE_FeedTrayManual, TYPE_TemporaryRescueUnitTrayManual }, // 手差し.
		{ TYPE_FeedTray1,  TYPE_TemporaryRescueUnitTray1  }, // トレイ1.
		{ TYPE_FeedTray2,  TYPE_TemporaryRescueUnitTray2  }, // トレイ2.
		{ TYPE_FeedTray3,  TYPE_TemporaryRescueUnitTray3  }, // トレイ3.
		{ TYPE_FeedTray4,  TYPE_TemporaryRescueUnitTray4  }, // トレイ4.
// comment Sky森本（弘） 2018/05/18 [DenebMLK][機器構成変更_No.7] 5段増設プリント機能追加対応．
#ifdef DEF_DenebMLKCus
		{ TYPE_FeedTray5,  TYPE_TemporaryRescueUnitTray5  }, // トレイ5.
#else
		{ TYPE_FeedTray5,  TYPE_TemporaryRescueUnitTrayLCT  }, // LCT.
#endif	// DEF_DenebMLKCus
	};
	if( NULL == PnlComGetJobCopy() ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }
	TYPC_Print* pc_Print = PnlComGetJobCopy()->getJobSpec()->pc_Print;
	TYPC_IntersheetMode c_Coversheet;

	if( False != b_Front ) {
		// オモテ表紙.
		c_Coversheet = pc_Print->c_Intersheet.getFrontMode();
	} else {
		// ウラ表紙.
		c_Coversheet = pc_Print->c_Intersheet.getBackMode();
	}

	// 用紙挿入モードを取得.
	const TYPE_PaperInsertion e_OldPaperInsertion = c_Coversheet.getPaperInsertion();
	TYPE_PaperInsertion e_NewPaperInsertion = e_PaperInsertion;
	// 前回と同じ設定を選択した場合は、受付拒否音を鳴らし、処理を行わない.
	if( e_OldPaperInsertion == e_PaperInsertion ) {
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return ;
	}

	c_Coversheet.setPaperInsertion( e_NewPaperInsertion );

	if( TYPE_PaperInsertionWithImage == e_NewPaperInsertion ) {
		// オモテ表紙・ウラ表紙の選択トレイを取得.
		TYPE_FeedTray e_FeedTray = c_Coversheet.getFeedTray();
		if( ( TYPE_FeedTrayPI1 == e_FeedTray ) || ( TYPE_FeedTrayPI2 == e_FeedTray ) ) {
			c_Coversheet.setFeedTray( TYPE_FeedTray1 );
		}
	}

	if( e_NewPaperInsertion == TYPE_PaperInsertionOff ) {
		APIC_OperationSetting* pc_OperationSetting = PnlComGetOperationSetting();
		TYPE_FeedTray e_FeedTray = pc_OperationSetting->getInsertFeedTray();
		if( False == isOn( CNK_ProgramJob, PnlComGetJobCopy() ) ) {
			if( ppc_FeedTray->getTrayType( e_FeedTray ) == TYPE_TrayTypeNon ||
			    ppc_SystemStatus->getTemporaryRescueUserStatus( s_checkFeedTrayTable_CLCD[e_FeedTray].e_TemporaryRescueUnit ) == TYPE_TemporaryRescueStatusSet ) {
//comment 2018/5/18 SCD2-Sky肥後 [DenebMLK][機器構成変更No.7]5段目トレイ追加対応.
//				uchar uc_CheckFeedTraySize = ( uchar )( sizeof( s_checkFeedTrayTable_CLCD )/sizeof( s_checkFeedTrayTable_CLCD ) );
				uchar uc_CheckFeedTraySize = static_cast<uchar>( sizeof( s_checkFeedTrayTable_CLCD )/sizeof( s_checkFeedTrayTable_CLCD[0] ) );
				for( uchar i = 0; i < uc_CheckFeedTraySize; i++ ) {
					if( ppc_FeedTray->getTrayType( s_checkFeedTrayTable_CLCD[i].e_FeedTray ) != TYPE_TrayTypeNon &&
					    ppc_SystemStatus->getTemporaryRescueUserStatus( s_checkFeedTrayTable_CLCD[i].e_TemporaryRescueUnit ) == TYPE_TemporaryRescueStatusReset ) {
						// 選択トレイを設定.
						e_FeedTray = s_checkFeedTrayTable_CLCD[i].e_FeedTray;
						break;
					}
				}
			}
		} else {
			e_FeedTray = TYPE_FeedTrayPI1;
		}
		c_Coversheet.setFeedTray( e_FeedTray );
	}

	if( False != b_Front ) {
		// 表表紙.
		pc_Print->c_Intersheet.setFrontMode( c_Coversheet );
	} else {
		// 裏表紙.
		pc_Print->c_Intersheet.setBackMode( c_Coversheet );
	}
}


/*================================================================================================
 マクロ関数の定義.
================================================================================================*/

PCPC_AuxilPageChapterEdit_job_com::PCPC_AuxilPageChapterEdit_job_com(
    PDisplayParts** partsTable,
    short partsNumber,
    Bool isNoWarningGamen,
    PPopupAnimationGroup* pc_AnimationComponent,
    short ss_PosX,
    short ss_PosY,
    short ss_SizeX,
    short ss_SizeY,
    PDisplayParts* pc_FunctionBadgeButton )
	: PPopupJobSetupChangeCom( partsTable, partsNumber, isNoWarningGamen,
	                           pc_AnimationComponent, ss_PosX, ss_PosY, ss_SizeX, ss_SizeY, pc_FunctionBadgeButton )
{
	m_uc_NowPage = PNLE_AuxilPageInsertEditCom_JobPageNumber01;
	m_ss_InputPageNumber = 0;
	m_ss_InputPageNumberBackup = 0;

	for( uchar i = 0; PNLD_InsertPageNumber_Max > i; i++ ) {

		// ページ番号の設定.
		m_s_PageNumberStatus[i].ss_PageNumber = 0;

		// 入力済みフラグの設定.
		m_s_PageNumberStatus[i].b_Input = False;

		// ページ番号の選択状態を設定.
		m_s_PageNumberStatus[i].b_Select = False;

	}
}

//------------------------------------------------------------------------------
/**
* @brief 共通処理 : 画面開始処理.
*/
//------------------------------------------------------------------------------
void PCPC_AuxilPageChapterEdit_job_com::startGamen()
{
	PPopupJobSetupChangeCom::startGamen();

	m_uc_NowPage = PNLE_AuxilPageInsertEditCom_JobPageNumber01;
	m_ss_InputPageNumber = 0;
	m_ss_InputPageNumberBackup = 0;
}

//------------------------------------------------------------------------------
/**
* @brief 共通処理 : テンキー.
*/
//------------------------------------------------------------------------------
void PCPC_AuxilPageChapterEdit_job_com::proc10Key( uchar option )
{

	if( option <= 9 ) {
		// テンキーで入力された値の算出.
		m_ss_InputPageNumber = ( short )proc10KeyInput( ( int32_t )( m_ss_InputPageNumber ),PCGD_MaxPageNumber, option, False, EP_InputMaxLimitPattern_Refuse );
	}
}

//------------------------------------------------------------------------------
/**
* @brief 共通処理 : クリアー.
*/
//------------------------------------------------------------------------------
void PCPC_AuxilPageChapterEdit_job_com::procClear( uchar option )
{
	if( m_ss_InputPageNumber == 0 && checkPageSelectStatus() == False ) {

		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}

	// ページ番号の選択を解除する.
	clearPageSelectStatus();

	// 入力エリアの値を初期化する.
	clearInputPageNumber();

}

//------------------------------------------------------------------------------
/**
* @brief 共通処理 : ページ番号の取得.
*/
//------------------------------------------------------------------------------
void PCPC_AuxilPageChapterEdit_job_com::getPageNumber( short* const pss_PageNumber )
{
	// 検索開始位置を設定する.
	const uchar uc_StartIndex = ( ( m_uc_NowPage == PNLE_AuxilPageInsertEditCom_JobPageNumber01 ) ? uchar( 0 ) : uchar( PNLD_InsertPageNumber_HALF ) );

	for( uchar i = 0, j = uc_StartIndex; PNLD_InsertPageNumber_HALF > i; i++, j++ ) {

		// ページ番号を取得する.
		pss_PageNumber[i] = m_s_PageNumberStatus[j].ss_PageNumber;

	}
}

//------------------------------------------------------------------------------
/**
* @brief 共通処理 : ページ番号の設定.
*/
//------------------------------------------------------------------------------
void PCPC_AuxilPageChapterEdit_job_com::setPageNumber( const short* pss_PageNumber )
{
	// 引数のNULLチェック.
	if( NULL == pss_PageNumber ) {

		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return;
	}

	for( uchar i = 0; PNLD_InsertPageNumber_Max > i; i++ ) {

		// ページ番号の設定.
		m_s_PageNumberStatus[i].ss_PageNumber = pss_PageNumber[i];

		// 入力済みフラグの設定.
		m_s_PageNumberStatus[i].b_Input = ( ( 0 == pss_PageNumber[i] ) ? ( False ) : ( True ) );

		// ページ番号の選択状態を設定.
		m_s_PageNumberStatus[i].b_Select = False;

	}
}

//------------------------------------------------------------------------------
/**
* @brief 共通処理 : ページ番号の選択状態を取得する.
*/
//------------------------------------------------------------------------------
Bool PCPC_AuxilPageChapterEdit_job_com::getPageSelectStatus( uchar uc_Index )
{
	if( m_uc_NowPage == PNLE_AuxilPageInsertEditCom_JobPageNumber02 ) {

		uc_Index = static_cast<uchar>( uc_Index + PNLD_InsertPageNumber_HALF );

	}

	// ページ番号の選択状態を取得する.
	return m_s_PageNumberStatus[uc_Index].b_Select;

}

//------------------------------------------------------------------------------
/**
* @brief 共通処理 : ページ番号の選択状態を設定する.
*/
//------------------------------------------------------------------------------
void PCPC_AuxilPageChapterEdit_job_com::setPageSelectStatus( PNLE_AuxilPageInsertEditCom_JobSelectPageNumber e_SelectPageNumber )
{
	uchar uc_Index = e_SelectPageNumber;

	if( m_uc_NowPage == PNLE_AuxilPageInsertEditCom_JobPageNumber02 ) {

		uc_Index = static_cast<uchar>( uc_Index + PNLD_InsertPageNumber_HALF );

	}

	if( m_s_PageNumberStatus[uc_Index].b_Select == True ) {

		m_s_PageNumberStatus[uc_Index].b_Select = False;

	} else {

		m_s_PageNumberStatus[uc_Index].b_Select = True;

	}
}

//------------------------------------------------------------------------------
/**
* @brief 共通処理 : ページ番号を非選択にする.
*/
//------------------------------------------------------------------------------
void PCPC_AuxilPageChapterEdit_job_com::clearPageSelectStatus()
{
	for( uchar i = 0; i < PNLD_InsertPageNumber_Max; i++ ) {
		// ページ番号を非選択にする.
		m_s_PageNumberStatus[i].b_Select = False;
	}

}

//------------------------------------------------------------------------------
/**
* @brief 共通処理 : ページ番号が選択されているかどうか判定する.
*/
//------------------------------------------------------------------------------
Bool PCPC_AuxilPageChapterEdit_job_com::checkPageSelectStatus()
{
	for( uchar i = 0; i < PNLD_InsertPageNumber_Max; i++ ) {

		if( m_s_PageNumberStatus[i].b_Select == True ) {

			return True;

		}
	}

	return False;

}

//------------------------------------------------------------------------------
/**
* @brief 共通処理 : ページ数を取得する.
*/
//------------------------------------------------------------------------------
uchar PCPC_AuxilPageChapterEdit_job_com::getNowPage()
{
	uchar uc_PageNumber = PNLE_AuxilPageInsertEditCom_JobPageNumber01;
	for( uchar i = 0; PNLD_InsertPageNumber_Max > i; i++ ) {

		if( m_s_PageNumberStatus[i].b_Select == True ) {

			if( PNLD_InsertPageNumber_HALF <= i ) {

				uc_PageNumber = PNLE_AuxilPageInsertEditCom_JobPageNumber02;
			}
			break;
		}
	}

	return uc_PageNumber;
}

//------------------------------------------------------------------------------
/**
* @brief 共通処理 : ページ番号の登録数を取得する.
*/
//------------------------------------------------------------------------------
uchar PCPC_AuxilPageChapterEdit_job_com::getPageNumberCount()
{
	// ページ番号の登録数.
	uchar uc_count = 0;

	for( uchar i = 0; PNLD_InsertPageNumber_Max > i; i++ ) {

		// ページ番号が設定されている場合.
		if( m_s_PageNumberStatus[i].ss_PageNumber != 0 ) {

			// ページ番号の登録数をインクリメントする.
			uc_count++;
		}
	}

	return uc_count;

}

//------------------------------------------------------------------------------
/**
* @brief 共通処理 : ページ番号の取得.
*/
//------------------------------------------------------------------------------
void PCPC_AuxilPageChapterEdit_job_com::getPageNumberSetting( short* const pss_PageNumber )
{
	for( uchar i = 0; i < PNLD_InsertPageNumber_Max; i++ ) {

		// ページ番号を取得する.
		pss_PageNumber[i] = m_s_PageNumberStatus[i].ss_PageNumber;

	}
}

//------------------------------------------------------------------------------
/**
* @brief 共通処理 : 追加されたページ番号の位置を検索する.
*/
//------------------------------------------------------------------------------
Bool PCPC_AuxilPageChapterEdit_job_com::SearchPageNumber()
{
	Bool b_ret = False;
	uchar i = 0;
	//for( uchar i = 0; i < PNLD_InsertPageNumber_Max; i++ ) {
	for( i = 0; i < PNLD_InsertPageNumber_Max; i++ ) {

		if( m_s_PageNumberStatus[i].ss_PageNumber == m_ss_InputPageNumberBackup ) {

			break;

		}
	}

	if( PNLD_InsertPageNumber_HALF <= i ) {

		b_ret = True;

	}

	return b_ret;

}

//------------------------------------------------------------------------------
/**
* @brief 共通処理 : 重複しているページ番号があるかどうか判定する.
*/
//------------------------------------------------------------------------------
Bool PCPC_AuxilPageChapterEdit_job_com::checkPageNumber()
{
	for( uchar i = 0; i < PNLD_InsertPageNumber_Max; i++ ) {

		if( m_s_PageNumberStatus[i].ss_PageNumber == m_ss_InputPageNumber ) {

			return True;

		}
	}

	return False;

}

//------------------------------------------------------------------------------
/**
* @brief 共通処理 : 入力エリアの値を取得.
*/
//------------------------------------------------------------------------------
short PCPC_AuxilPageChapterEdit_job_com::getInputPageNumber()
{
	return m_ss_InputPageNumber;
}

//------------------------------------------------------------------------------
/**
* @brief 共通処理 : 入力エリアの値をクリア.
*/
//------------------------------------------------------------------------------
void PCPC_AuxilPageChapterEdit_job_com::clearInputPageNumber()
{
	m_ss_InputPageNumber = 0;
}

//------------------------------------------------------------------------------
/**
* @brief 共通処理 : ページ番号の更新.
*/
//------------------------------------------------------------------------------
void PCPC_AuxilPageChapterEdit_job_com::renewPageNumber()
{
	for( uchar i = 0; PNLD_InsertPageNumber_Max > i; i++ ) {

		// ページ番号が0の場合.
		if( m_s_PageNumberStatus[i].ss_PageNumber == 0 ) {

			// ページ番号を設定する.
			m_s_PageNumberStatus[i].ss_PageNumber = m_ss_InputPageNumber;
			m_s_PageNumberStatus[i].b_Input = True;

			// 入力エリアの値をバックアップする.
			m_ss_InputPageNumberBackup = m_ss_InputPageNumber;

			// 入力エリアの値を初期化する.
			clearInputPageNumber();
			break;
		}
	}
}

//------------------------------------------------------------------------------
/**
* @brief 共通処理 : ページ番号の削除.
*/
//------------------------------------------------------------------------------
void PCPC_AuxilPageChapterEdit_job_com::deletePageNumber()
{
	for( uchar i = 0; PNLD_InsertPageNumber_Max > i; i++ ) {

		if( m_s_PageNumberStatus[i].b_Select == True ) {

			// ページ番号を削除する.
			m_s_PageNumberStatus[i].ss_PageNumber = 0;
			m_s_PageNumberStatus[i].b_Input = False;
			m_s_PageNumberStatus[i].b_Select = False;

		}
	}
}

//------------------------------------------------------------------------------
/**
* @brief 共通処理 : ソート.
*/
//------------------------------------------------------------------------------
void PCPC_AuxilPageChapterEdit_job_com::procSortExec()
{
	short ss_temp;
	Bool  bl_temp;
	ushort i,j;
	ushort us_Number_Cnt_Max = PNLD_InsertPageNumber_Max;

	for( i=0; i < ( ushort )( us_Number_Cnt_Max-1 ); i++ ) {

		for( j=0; j < ( ushort )( us_Number_Cnt_Max-1-i ); j++ ) {

			if( ( ( m_s_PageNumberStatus[j+1].b_Input == True ) && ( m_s_PageNumberStatus[j+1].ss_PageNumber != 0 ) ) &&
			    ( ( m_s_PageNumberStatus[j].b_Input == False ) ||
			      ( m_s_PageNumberStatus[j].ss_PageNumber == 0 ) ||
			      ( ( m_s_PageNumberStatus[j].ss_PageNumber != 0 ) && ( m_s_PageNumberStatus[j].ss_PageNumber > m_s_PageNumberStatus[j+1].ss_PageNumber ) ) ) ) {

				ss_temp = m_s_PageNumberStatus[j].ss_PageNumber;
				m_s_PageNumberStatus[j].ss_PageNumber = m_s_PageNumberStatus[j+1].ss_PageNumber;
				m_s_PageNumberStatus[j+1].ss_PageNumber = ss_temp;

				bl_temp = m_s_PageNumberStatus[j].b_Input;
				m_s_PageNumberStatus[j].b_Input = m_s_PageNumberStatus[j+1].b_Input;
				m_s_PageNumberStatus[j+1].b_Input = bl_temp;

				bl_temp = m_s_PageNumberStatus[j].b_Select;
				m_s_PageNumberStatus[j].b_Select = m_s_PageNumberStatus[j+1].b_Select;
				m_s_PageNumberStatus[j+1].b_Select = bl_temp;

			}
		}
	}
}
//------------------------------------------------------------------------------
/**
* @brief A061152200 章分け : 表示部品のセットアップ.
*/
//------------------------------------------------------------------------------
void PCPC_AuxilPageChapterEdit_job::startGamen()
{
	PNLC_OutSideKeyCallDataLib::newInstance()->setSelectNo( PNLE_Soft10KeyUnselected );			// データクリア.

	// エラーフラグの初期化.
	m_b_ErrorOn = False;

	// 章分け登録フラグ.
	m_b_RegistrationFlg = False;

	// JobSpecのデータ保存管理用テーブルNoの初期化.
	m_ss_JobSpecBackupTableNo = -1;

	PCPC_AuxilPageChapterEdit_job_com::startGamen();

	// 章分け番号の取得.
	getChapterPageNumber();

	// 主テキストメッセージを変更する.
// Comment 2016/10/26 [HCL_Ravi] [IT5_4.0] [1006281] Changes made for conversion to half font.
//	Cmp_AuxilPageChapterEdit_jobMessage.changeMainMessageText( s_pMSGSetChapterPositionByKeypad_W448H034A );
	Cmp_AuxilPageChapterEdit_jobMessage.changeMainMessageText( s_pMSGSetChapterPositionByKeypad_W560H034A );

	// ON/OFFを表示.
	Cmp_AuxilPageChapterEdit_jobMessage.setOnOffButton( True );

	// ON/OFFキーの選択状態変更.
	Cmp_AuxilPageChapterEdit_jobMessage.setOnOffSwitchButtonState( isOn( CNK_Chapter, PnlComGetJobCopy() ) );
	Cmp_AuxilPageChapterEdit_jobMessage.attachMessage( True );
	Cmp_AuxilPageChapterEdit_jobMessage.setMode( PNLE_PopupMessageAreaMessageModeAdmin );
	Cmp_AuxilPageChapterEdit_jobMessage.showDestinationCount( False );
	Cmp_AuxilPageChapterEdit_jobMessage.setNowPopupGamen( PNLE_AuxilPageChapterEdit_job );
	// スクロールバーから画面へキーを流す.
	Cmp_AuxilPageChapterEdit_jobScrollBar.attachMessage( True );

	// インサートブリンク制御.
	m_b_BlinkOn = isOn( CNK_Chapter, PnlComGetJobCopy() );
	m_bl_isSelectedInputField = ( True == PnlComIsHard10Key() ) ? True : False; // comment 2012/06/21 OPS1川手 [MinervaBK][CQ366316]ソフト10キーUD対応 No.1 [数値入力エリアの初期化].

	// 入力エリア押下機能有無.
	if( True != PLBC_FuncVersionCheck::isOK_Type4() ) {
		Key_AuxilPageChapterEdit_jobPage.setShowPush( False );
		Key_AuxilPageChapterEdit_jobPageError.setShowPush( False );
	}

	PNLS_setOriginalSizeCLCD2Key s_parts;
	s_parts.ps_KeyOriginalSize      =   &Key_AuxilPageChapterEdit_jobInsertSheet;
	s_parts.ps_TxtOriginalSize01	=	&Ktx_AuxilPageChapterEdit_jobInsertSheet2;			// １行目 用紙コード表示.
	s_parts.ps_TxtOriginalSize02	=	NULL;												// ２行目 自動表示.
	s_parts.ps_TxtOriginalSizeX		=	&Ktx_AuxilPageChapterEdit_jobInsertSheetX;			// 用紙サイズ表示 X.
	s_parts.ps_TxtOriginalSizeY		=	&Ktx_AuxilPageChapterEdit_jobInsertSheetY;			// 用紙サイズ表示 Y.
	s_parts.ps_NumOriginalSizeX		=	&Num_AuxilPageChapterEdit_jobInsertSheetX;			// 用紙サイズ数値表示 X.
	s_parts.ps_NumOriginalSizeY		=	&Num_AuxilPageChapterEdit_jobInsertSheetY;			// 用紙サイズ数値表示 Y.
	s_parts.ps_TrayTxtPaperSize		= 	&Ktx_AuxilPageChapterEdit_jobInsertSheet1;			// トレイ表示テキスト.
	s_parts.ps_TrayIconPaperSize	= 	&Gra_AuxilPageChapterEdit_jobInsertSheet1;			// トレイ表示アイコン（手差しアイコン）.
	s_parts.ps_RemainIconPaperSize	=	NULL;
	s_parts.ps_TypeIconPaperSize	=	NULL;

	// 章分け紙の給紙トレイはインターシートトレイ選択で設定されているトレイにする.
	TYPE_FeedTray e_FeedTray = PnlComGetOperationSetting()->getInsertFeedTray();
	// デフォルトトレイが使用不可の場合、使用できるトレイを設定.
	static const struct DefaultFeedTrayTable_CLCD {
		TYPE_FeedTray				e_FeedTray;					// 給紙口.
		TYPE_TemporaryRescueUnit	e_TemporaryRescueUnit;		// 一時切り離し対象
	} defaultFeedTrayTable_CLCD[] = {
		{ TYPE_FeedTrayManual,	TYPE_TemporaryRescueUnitTrayManual	},	// 手差し.
		{ TYPE_FeedTray1,		TYPE_TemporaryRescueUnitTray1		},	// トレイ1.
		{ TYPE_FeedTray2,		TYPE_TemporaryRescueUnitTray2		},	// トレイ2
		{ TYPE_FeedTray3,		TYPE_TemporaryRescueUnitTray3		},	// トレイ3
		{ TYPE_FeedTray4,		TYPE_TemporaryRescueUnitTray4		},	// トレイ4
// comment Sky森本（弘） 2018/05/18 [DenebMLK][機器構成変更_No.7] 5段増設プリント機能追加対応．
#ifdef DEF_DenebMLKCus
		{ TYPE_FeedTray5,		TYPE_TemporaryRescueUnitTray5		},	// トレイ5.
#else
		{ TYPE_FeedTray5,		TYPE_TemporaryRescueUnitTrayLCT		},	// LCT
#endif	// DEF_DenebMLKCus
	};
	if( ppc_FeedTray->getTrayType( e_FeedTray ) == TYPE_TrayTypeNon
	    || ppc_SystemStatus->getTemporaryRescueUserStatus( defaultFeedTrayTable_CLCD[e_FeedTray].e_TemporaryRescueUnit ) == TYPE_TemporaryRescueStatusSet ) {
//comment 2018/5/18 SCD2-Sky肥後 [DenebMLK][機器構成変更No.7]5段目トレイ追加対応.
//		uchar uc_CheckFeedTraySize = ( uchar )( sizeof( defaultFeedTrayTable_CLCD )/sizeof( DefaultFeedTrayTable_CLCD ) );
		uchar uc_CheckFeedTraySize = ( uchar )( sizeof( defaultFeedTrayTable_CLCD )/sizeof( defaultFeedTrayTable_CLCD[0] ) );
		for( uchar i = 0; i < uc_CheckFeedTraySize; i++ ) {
			if( ppc_FeedTray->getTrayType( defaultFeedTrayTable_CLCD[i].e_FeedTray ) != TYPE_TrayTypeNon
			    && ppc_SystemStatus->getTemporaryRescueUserStatus( defaultFeedTrayTable_CLCD[i].e_TemporaryRescueUnit ) == TYPE_TemporaryRescueStatusReset ) {
				e_FeedTray = defaultFeedTrayTable_CLCD[i].e_FeedTray;
				break;
			}
		}
	}

	PnlComsetPaperSizeCLCD2Grap( &s_parts, e_FeedTray, PNLE_PaperSizeShowKind_Type1 );
}

//------------------------------------------------------------------------------
/**
* @brief A061152200 章分け : 表示部品のセットアップ.
*/
//------------------------------------------------------------------------------
void PCPC_AuxilPageChapterEdit_job::setupGamen( short option )
{
	// 章分け番号.
	short ss_AryPageNumber[PNLD_InsertPageNumber_HALF];
	// 章分け番号の登録数.
	uchar uc_Count = 0;

	PCPC_AuxilPageChapterEdit_job_com::setupGamen( option );

	if( False == Cmp_AuxilPageChapterEdit_jobMessage.getOnOffSwitchButtonState() ) {

		// 入力エラー表示を非表示にする.
		Key_AuxilPageChapterEdit_jobPageError.showEnable( False );
		// 章分け位置入力エリアの表示.
		Key_AuxilPageChapterEdit_jobPage.showEnable( True );

		Key_AuxilPageChapterEdit_jobOK.activate( True );
		Grp_AuxilPageChapterEdit_jobGrayOut.activate( False );

		Ktx_AuxilPageChapterEdit_jobInsertSheetX.showEnable( False );
		Ktx_AuxilPageChapterEdit_jobInsertSheetY.showEnable( False );
//comment 2018/5/18 SCD2-Sky肥後 [DenebMLK][機器構成変更No.7]5段目トレイ追加対応.
//		Gra_AuxilPageChapterEdit_jobInsertSheet1.showEnable( False );

		// 章分け番号を取得する.
		getPageNumber( &( ss_AryPageNumber[0] ) );

		for( uchar index = 0; index < PNLD_InsertPageNumber_HALF; index++ ) {

			if( ss_AryPageNumber[index] != 0 ) {

				// 章分け番号を設定する.
				( ( PNormalButton* )tbl_AuxilPageChapterEdit_jobEditNum[index] )->set( ss_AryPageNumber[index], True );
				// 章分け位置指定キーを表示する.
				tbl_AuxilPageChapterEdit_jobEditNum[index]->showEnable( True );
				//
				( ( PNormalButton* )tbl_AuxilPageChapterEdit_jobEditNum[index] )->activate( False );
			} else {

				// 章分け位置指定キーを非表示にする.
				tbl_AuxilPageChapterEdit_jobEditNum[index]->showEnable( False );

			}
		}
		m_b_BlinkOn = False;
		Key_AuxilPageChapterEdit_jobPage.showEnableAdd( &Gra_AuxilPageChapterEdit_jobInsertBlink, m_b_BlinkOn );
		return;
	}

	Key_AuxilPageChapterEdit_jobOK.activate( True );
	Grp_AuxilPageChapterEdit_jobGrayOut.activate( True );

	// 章分けトレイ設定.
	setupChpaterTray();

	// ページ数を取得する.
	m_uc_NowPage = Cmp_AuxilPageChapterEdit_jobScrollBar.getSliderPage();

	// 章分け位置指定の登録数を取得する.
	uc_Count = getPageNumberCount();

	if( uc_Count == 0 ) {
		// ページ番号を1ページ目に変更する.
		m_uc_NowPage = PNLE_AuxilPageInsertEditCom_JobPageNumber01;
		// スライダを非活性化する.
		Cmp_AuxilPageChapterEdit_jobScrollBar.activate( False );
		// [OK]、[章分け紙]キーを非活性.
		Key_AuxilPageChapterEdit_jobOK.activate( False );
		Key_AuxilPageChapterEdit_jobInsertSheet.activate( False );
	} else if( uc_Count < 16 ) {
		// ページ番号を1ページ目に変更する.
		m_uc_NowPage = PNLE_AuxilPageInsertEditCom_JobPageNumber01;
		// スライダを非活性化する.
		Cmp_AuxilPageChapterEdit_jobScrollBar.activate( False );
	} else {
		if( uc_Count == PNLD_InsertPageNumber_Max ) {
			Key_AuxilPageChapterEdit_jobAdd.activate( False );
			Key_AuxilPageChapterEdit_jobPage.activate( False );		// ページ表示エリアを非活性化. // comment CQ351466 2011/07/27.
		}

		if( m_b_RegistrationFlg == True ) {
			if( SearchPageNumber() == True ) {
				// ページ番号を2ページ目に変更する.
				m_uc_NowPage = PNLE_AuxilPageInsertEditCom_JobPageNumber02;
			} else {
				// ページ番号を1ページ目に変更する.
				m_uc_NowPage = PNLE_AuxilPageInsertEditCom_JobPageNumber01;
			}
			// スライダ移動.
			Cmp_AuxilPageChapterEdit_jobScrollBar.moveSliderPage( m_uc_NowPage );
			m_b_RegistrationFlg = False;
		}
	}

	// 章分け番号を取得する.
	getPageNumber( &( ss_AryPageNumber[0] ) );

	for( uchar i = 0; i < PNLD_InsertPageNumber_HALF; i++ ) {
		if( ss_AryPageNumber[i] != 0 ) {
			// 章分け番号を設定する.
			( ( PNormalButton* )tbl_AuxilPageChapterEdit_jobEditNum[i] )->set( ss_AryPageNumber[i], True );
			// 章分け位置指定キーを表示する.
			tbl_AuxilPageChapterEdit_jobEditNum[i]->showEnable( True );
			( ( PNormalButton* )tbl_AuxilPageChapterEdit_jobEditNum[i] )->select( getPageSelectStatus( i ) );
		} else {
			// 章分け位置指定キーを非表示にする.
			tbl_AuxilPageChapterEdit_jobEditNum[i]->showEnable( False );
		}
	}
	if( True == m_b_ErrorOn ) {
		// 1キー入力or一定時間でのエラー解除された場合.
		if( EP_ErrorPartsKind_ErrorOff == onlyPanel.getErrorNumberFieldPartsKind() ) {
			// パネルの状態をエラーなしにする.
			onlyPanel.setErrorNumberFieldPartsKind( EP_ErrorPartsKind_Non );
			// エラー表示を行わず、正常値を表示させる.
			m_b_ErrorOn = False;
		}
	}

	// 章分け位置入力エリアの値を取得する.
	short ss_InputPageNumber = getInputPageNumber();

	if( m_b_ErrorOn == True ) {
		// [追加]キーを非活性化.
		Key_AuxilPageChapterEdit_jobAdd.activate( False );

	} else {

		// 入力エラーを非表示.
		Key_AuxilPageChapterEdit_jobPageError.showEnable( False );
		// 章分け位置入力エリアの表示.
		Key_AuxilPageChapterEdit_jobPage.showEnable( True );


		if( ss_InputPageNumber != 0 ) {

			// 入力されている値を設定.
			Key_AuxilPageChapterEdit_jobPage.set( ss_InputPageNumber, True, -1, False );
			// 入力エリアの値を表示.
			Key_AuxilPageChapterEdit_jobPage.showEnableAdd( True );

		} else {

			// 入力エリアの値を非表示.
			Key_AuxilPageChapterEdit_jobPage.showEnableAdd( False );

			// [追加]キーを非活性化.
			Key_AuxilPageChapterEdit_jobAdd.activate( False );

		}

	}
	// インサートブリンク制御.
	if( ( uc_Count < 30 ) && ( True == m_bl_isSelectedInputField ) ) {
		if( m_b_BlinkOn ) {
			m_b_BlinkOn = False;
		} else {
			m_b_BlinkOn = True;
		}
	} else {
		m_b_BlinkOn = False;
	}
	Key_AuxilPageChapterEdit_jobPage.showEnableAdd( &Gra_AuxilPageChapterEdit_jobInsertBlink, m_b_BlinkOn );

	if( checkPageSelectStatus() == False ) {
		Key_AuxilPageChapterEdit_jobDelete.activate( False );
	}
	if( False == Key_AuxilPageChapterEdit_jobOK.isActivating() ) {
		//Falseの時だけ設定し青は基底処理におまかせ.
		printLEDControl( False );
	}
	sendSlide10KeyData();

}

//------------------------------------------------------------------------------
/**
* @brief A061152200 章分け : ボタン（押された).
*/
//------------------------------------------------------------------------------
void PCPC_AuxilPageChapterEdit_job::procButtonOn( uchar option )
{
	short ss_SrcPosX,ss_SrcPosY;	// 表示位置.
	short ss_SrcSizeX,ss_SrcSizeY;	// 表示サイズ.

// ボタンID重複のためスクロールバーのメッセージを別処理にしました.
	if( True == isMessageActive( &Cmp_AuxilPageChapterEdit_jobScrollBar ) ) {
		// ボタンID重複対応.
		switch( option ) {
			// ページ切替えキー.
			case PNLE_AuxilPageChapterEdit_ScrollBarKeyType_Dec:
			case PNLE_AuxilPageChapterEdit_ScrollBarKeyType_Inc:
				break;
			default:
				break;
		}
		setupGamen();
		return;
	}
	if( option == PNLE_AuxilPageChapterEdit_jobInsertSheet ) {
		Key_AuxilPageChapterEdit_jobInsertSheet.getPosition( ss_SrcPosX, ss_SrcPosY );
		Key_AuxilPageChapterEdit_jobInsertSheet.getSize( ss_SrcSizeX, ss_SrcSizeY );
		PNLC_PopupListControl::newInstance()->addPopup( &gp_A061152210, this, NULL, ss_SrcPosX, ss_SrcPosY, ss_SrcSizeX, ss_SrcSizeY );// 章分け紙.
		return;
	} else {
		switch( option ) {
			case PNLE_AuxilPageChapterEdit_jobEditNum01:
			case PNLE_AuxilPageChapterEdit_jobEditNum02:
			case PNLE_AuxilPageChapterEdit_jobEditNum03:
			case PNLE_AuxilPageChapterEdit_jobEditNum04:
			case PNLE_AuxilPageChapterEdit_jobEditNum05:
			case PNLE_AuxilPageChapterEdit_jobEditNum06:
			case PNLE_AuxilPageChapterEdit_jobEditNum07:
			case PNLE_AuxilPageChapterEdit_jobEditNum08:
			case PNLE_AuxilPageChapterEdit_jobEditNum09:
			case PNLE_AuxilPageChapterEdit_jobEditNum10:
			case PNLE_AuxilPageChapterEdit_jobEditNum11:
			case PNLE_AuxilPageChapterEdit_jobEditNum12:
			case PNLE_AuxilPageChapterEdit_jobEditNum13:
			case PNLE_AuxilPageChapterEdit_jobEditNum14:
			case PNLE_AuxilPageChapterEdit_jobEditNum15:
				setPageSelectStatus( ( PNLE_AuxilPageInsertEditCom_JobSelectPageNumber )( option  - PNLE_AuxilPageChapterEdit_jobEditNum01 ) ); break;
			case PNLE_AuxilPageChapterEdit_jobAdd:
				if( checkPageNumber() == True ) {
					//gp_AuxilPageChapterPaperSet_job not defined.
					PNLC_PopupListControl::newInstance()->addPopup( &gp_InsertDuplicateDesignation_warn, this, NULL, 144, 220, 88, 32 );// 挿入位置指定重複.
					return;
				} else {
					// 章分け番号の更新.
					renewPageNumber();
					// 章分け番号のソート.
					procSortExec();
					// 章分け番号を設定.
					setChapterPageNumber();
					// 章分け登録フラグを有効にする.
					m_b_RegistrationFlg = True;
				}
				break;
			case PNLE_AuxilPageChapterEdit_jobDelete:
				// スライダ移動.
				Cmp_AuxilPageChapterEdit_jobScrollBar.moveSliderPage( getNowPage() );
				// 章分け番号の削除.
				deletePageNumber( );
				// 章分け番号のソート.
				procSortExec();
				// 章分け番号を設定.
				setChapterPageNumber();
				break;
			case PNLE_AuxilPageChapterEdit_jobDoCopy:
				if( combinationCheck( CNK_ChapterPaperOn, PnlComGetJobCopy(), False, True ) == True ) {
					setChapterTrayUse( True );
				}
				break;
			case PNLE_AuxilPageChapterEdit_jobNotDoCopy:
				setChapterTrayUse( False );
				break;
			case PNLE_PopupMessageAreaButtonType_On:
				if( ( True == combinationCheck_AccumuDoc( CNK_Chapter, PnlComGetJBJobSpec(), PnlComGetJobCopy() ) )
				    && ( True == combinationCheck( CNK_Chapter, PnlComGetJobCopy() ) ) ) {
					// ON状態を設定する.
					setOn();
				}
				else {
					// 組合せ禁止のため警告画面を表示し、[OFF] ボタンを選択状態にする.
					Cmp_AuxilPageChapterEdit_jobMessage.setOnOffSwitchButtonState( False );
				}
				break;
			case PNLE_PopupMessageAreaButtonType_Off:
				setOff();
				break;
			case PNLE_AuxilPageChapterEdit_jobNumericInputArea:
				m_bl_isSelectedInputField = True;	// comment 2012/06/21 OPS1川手 [MinervaBK][CQ366316]ソフト10キーUD対応 No.1 [数値入力エリアの選択状態更新].
				// テンキーポップアップ表示.
				show10KeyPopup();
				break;
			default: LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return;
		}
	}
	// 再描画.
	setupGamen();
}

//------------------------------------------------------------------------------
/**
* @brief A061152200 章分け : 設定取消.
*/
//------------------------------------------------------------------------------
void PCPC_AuxilPageChapterEdit_job::procCancel( uchar option )
{
	m_b_CancelOn = True;
	PCPC_AuxilPageChapterEdit_job_com::procCancel( option );
}

//------------------------------------------------------------------------------
/**
* @brief A061152200 章分け : ONボタン押下後の処理.
*/
//------------------------------------------------------------------------------
void PCPC_AuxilPageChapterEdit_job::setOn()
{
	if( NULL == PnlComGetJobCopy() ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }
	TYPC_Print* pc_Print = PnlComGetJobCopy()->getJobSpec()->pc_Print;
	pc_Print->c_Chapter.setOn( True );
	// バックアップデータがある場合.
	if( m_ss_JobSpecBackupTableNo != -1 ) {
		// JobSpecを取得する.
		TYPC_JobSpec* pc_JobSpec = PnlComGetJobCopy()->getJobSpec();
		// バックアップしたJobSpecに戻す.
		PnlComGetCopyDataLib()->setJobSpecBackup( m_ss_JobSpecBackupTableNo , pc_JobSpec );
		// バックアップしたJobSpecをクリアする.
		PnlComGetCopyDataLib()->JobSpecBackupClear( m_ss_JobSpecBackupTableNo );
		m_ss_JobSpecBackupTableNo = -1;
	} else {
		// 両面印刷.
		pc_Print->setDuplexMode( TYPE_DuplexModeOn );
		// 章分け紙は使用しない.
		setChapterTrayUse( False );
	}
	setupGamen();
}

//------------------------------------------------------------------------------
/**
* @brief A061152200 章分け : OFFボタン押下後の処理.
*/
//------------------------------------------------------------------------------
void PCPC_AuxilPageChapterEdit_job::setOff()
{
	// JobSpecのデータを保存する.
	if( NULL == PnlComGetJobCopy() ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }
	TYPC_JobSpec* pc_JobSpec = PnlComGetJobCopy()->getJobSpec();
	// JobSpecのデータを保存する.
	m_ss_JobSpecBackupTableNo = PnlComGetCopyDataLib()->saveJobSpecBackup( m_ss_JobSpecBackupTableNo, pc_JobSpec );
	// 章分けの取消し.
	cancel( CNK_Chapter, CNK_Size, PnlComGetJobCopy() );
	if( False == PnlComIsHard10Key() ) {
		m_bl_isSelectedInputField = False;
	}
	// MTUIテンキー非表示.
	hide10KeyPopup();
	setupGamen();
}

//------------------------------------------------------------------------------
/**
* @brief A061152200 章分け : 章分けトレイに関する表示処理.
*/
//------------------------------------------------------------------------------
void PCPC_AuxilPageChapterEdit_job::setupChpaterTray()
{
	PNLS_setOriginalSizeCLCD2Key s_parts;
	if( NULL == PnlComGetJobCopy() ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }
	TYPC_Print* pc_Print = PnlComGetJobCopy()->getJobSpec()->pc_Print;
	TYPE_FeedTray e_FeedTray = pc_Print->c_Chapter.getFeedTray();

	s_parts.ps_KeyOriginalSize      =   &Key_AuxilPageChapterEdit_jobInsertSheet;
	s_parts.ps_TxtOriginalSize01	=	&Ktx_AuxilPageChapterEdit_jobInsertSheet2;			// １行目 用紙コード表示.
	s_parts.ps_TxtOriginalSize02	=	NULL;												// ２行目 自動表示.
	s_parts.ps_TxtOriginalSizeX		=	&Ktx_AuxilPageChapterEdit_jobInsertSheetX;			// 用紙サイズ表示 X.
	s_parts.ps_TxtOriginalSizeY		=	&Ktx_AuxilPageChapterEdit_jobInsertSheetY;			// 用紙サイズ表示 Y.
	s_parts.ps_NumOriginalSizeX		=	&Num_AuxilPageChapterEdit_jobInsertSheetX;			// 用紙サイズ数値表示 X.
	s_parts.ps_NumOriginalSizeY		=	&Num_AuxilPageChapterEdit_jobInsertSheetY;			// 用紙サイズ数値表示 Y.
	s_parts.ps_TrayTxtPaperSize		= 	&Ktx_AuxilPageChapterEdit_jobInsertSheet1;			// トレイ表示テキスト.
	s_parts.ps_TrayIconPaperSize	= 	&Gra_AuxilPageChapterEdit_jobInsertSheet1;			// トレイ表示アイコン（手差しアイコン）.
	s_parts.ps_RemainIconPaperSize	=	NULL;
	s_parts.ps_TypeIconPaperSize	=	NULL;

	// 章分けトレイ使用時.
	Bool b_DisplayChapterTray = False;
	short ss_SelectChapterTray = 0;
	if( pc_Print->c_Chapter.getChapterPaperOn() ) {
		// 章分け紙ボタン表示、章分けトレイ使用.
		b_DisplayChapterTray = True;
		ss_SelectChapterTray = 1;
		PnlComsetPaperSizeCLCD2Grap( &s_parts, e_FeedTray, PNLE_PaperSizeShowKind_Type1 );
		Key_AuxilPageChapterEdit_jobInsertSheet.activate( True );
	} else {
		Key_AuxilPageChapterEdit_jobInsertSheet.activate( False );
	}
	Rtb_AuxilPageChapterEdit_jobInsertMethod.select( ss_SelectChapterTray );
}

//------------------------------------------------------------------------------
/**
* @brief A061152200 章分け : 画面終了処理.
*/
//------------------------------------------------------------------------------
void PCPC_AuxilPageChapterEdit_job::endGamen()
{
	if( m_b_CancelOn == False ) {
		setChapterPageNumber();  // 章分け番号の設定.
	}

	if( m_ss_JobSpecBackupTableNo != -1 ) {
		// バックアップしたJobSpecをクリアする.
		PnlComGetCopyDataLib()->JobSpecBackupClear( m_ss_JobSpecBackupTableNo );
		m_ss_JobSpecBackupTableNo = -1;
	}
	PCPC_AuxilPageChapterEdit_job_com::endGamen();
}

//------------------------------------------------------------------------------
/**
* @brief A061152200 章分け : 章分け紙使用状態の設定.
*/
//------------------------------------------------------------------------------
void PCPC_AuxilPageChapterEdit_job::setChapterTrayUse( const Bool b_use )
{
	if( NULL == PnlComGetJobCopy() ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }
	TYPC_Print* pc_Print = PnlComGetJobCopy()->getJobSpec()->pc_Print;
	// APSの取得.
	if( b_use ) {
		if( isOn( CNK_APS, PnlComGetJobCopy() ) == True ) {
			// APS解除.
			cancel( CNK_APS, CNK_Size, PnlComGetJobCopy() );
		}
	}
	// 常にトレイをチェックする.
	// 章分け紙使用状態を設定.
//	TYPE_FeedTray e_FeedTray = TYPE_FeedTrayUnselect;
	// 章分け紙の給紙トレイはインターシートトレイ選択で設定されているトレイにする.
	APIC_OperationSetting* pc_OperationSetting = PnlComGetOperationSetting();
	TYPE_FeedTray e_FeedTray = pc_OperationSetting->getInsertFeedTray();
	// デフォルトトレイが使用不可の場合、使用できるトレイを設定.
	static const struct CheckFeedTrayTable_CLCD {
		TYPE_FeedTray				e_FeedTray;					// 給紙口.
		TYPE_TemporaryRescueUnit	e_TemporaryRescueUnit;		// 一時切り離し対象
	} checkFeedTrayTable_CLCD[] = {
		{ TYPE_FeedTrayManual,	TYPE_TemporaryRescueUnitTrayManual	},	// 手差し.
		{ TYPE_FeedTray1,		TYPE_TemporaryRescueUnitTray1		},	// トレイ1.
		{ TYPE_FeedTray2,		TYPE_TemporaryRescueUnitTray2		},	// トレイ2
		{ TYPE_FeedTray3,		TYPE_TemporaryRescueUnitTray3		},	// トレイ3
		{ TYPE_FeedTray4,		TYPE_TemporaryRescueUnitTray4		},	// トレイ4
// comment Sky森本（弘） 2018/05/18 [DenebMLK][機器構成変更_No.7] 5段増設プリント機能追加対応．
#ifdef DEF_DenebMLKCus
		{ TYPE_FeedTray5,		TYPE_TemporaryRescueUnitTray5		},	// トレイ5.
#else
		{ TYPE_FeedTray5,		TYPE_TemporaryRescueUnitTrayLCT		},	// LCT
#endif	// DEF_DenebMLKCus
	};
	if( ppc_FeedTray->getTrayType( e_FeedTray ) == TYPE_TrayTypeNon
	    || ppc_SystemStatus->getTemporaryRescueUserStatus( checkFeedTrayTable_CLCD[e_FeedTray].e_TemporaryRescueUnit ) == TYPE_TemporaryRescueStatusSet ) {
		uchar uc_CheckFeedTraySize = ( uchar )( sizeof( checkFeedTrayTable_CLCD )/sizeof( CheckFeedTrayTable_CLCD ) );
		for( uchar i = 0; i < uc_CheckFeedTraySize; i++ ) {
			if( ppc_FeedTray->getTrayType( checkFeedTrayTable_CLCD[i].e_FeedTray ) != TYPE_TrayTypeNon
			    && ppc_SystemStatus->getTemporaryRescueUserStatus( checkFeedTrayTable_CLCD[i].e_TemporaryRescueUnit ) == TYPE_TemporaryRescueStatusReset ) {
				e_FeedTray = checkFeedTrayTable_CLCD[i].e_FeedTray;
				break;
			}
		}
	}
	// 章分け紙トレイの設定.
	pc_Print->c_Chapter.setFeedTray( e_FeedTray );
	// 章分け紙使用状態を設定.
	pc_Print->c_Chapter.setChapterPaperOn( b_use );
}

//------------------------------------------------------------------------------
/**
* @brief A061152200 章分け : リセットキー押下.
*/
//------------------------------------------------------------------------------
void PCPC_AuxilPageChapterEdit_job::procReset( uchar option )
{
	PCPC_AuxilPageChapterEdit_job_com::procReset( option );
	if( NULL == PnlComGetJobCopy() ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }
}

//------------------------------------------------------------------------------
/**
* @brief A061152200 章分け : 章分け番号の取得.
*/
//------------------------------------------------------------------------------
void PCPC_AuxilPageChapterEdit_job::getChapterPageNumber()
{
	if( NULL == PnlComGetJobCopy() ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }
	TYPC_Print* pc_Print = PnlComGetJobCopy()->getJobSpec()->pc_Print;
	// 章分け番号の取得.
	short ss_AryPageNumber[PNLD_ChapterPageNumber_Max];				// 取得用 要素数：99.
	short ss_ChapterPageNumber[PNLD_InsertPageNumber_Max];			// 設定用 要素数：30.
	// ページ番号の設定.
	pc_Print->c_Chapter.getPages( &( ss_AryPageNumber[0] ) );

	for( uchar i = 0 ; PNLD_InsertPageNumber_Max > i; i++ ) {
		ss_ChapterPageNumber[ i ] = ss_AryPageNumber[ i ];
	}
	// ページ番号の設定.
	setPageNumber( &( ss_ChapterPageNumber[0] ) );

}

//------------------------------------------------------------------------------
/**
* @brief A061152200 章分け : 章分け番号の設定.
*/
//------------------------------------------------------------------------------
void PCPC_AuxilPageChapterEdit_job::setChapterPageNumber()
{
	if( NULL == PnlComGetJobCopy() ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }
	TYPC_Print* pc_Print = PnlComGetJobCopy()->getJobSpec()->pc_Print;
	// ページ番号の取得.
	short ss_AryPageNumber[PNLD_ChapterPageNumber_Max];				// 設定用 要素数：99.
	short ss_ChapterPageNumber[PNLD_InsertPageNumber_Max];			// 取得用 要素数：30.

	memset( ss_AryPageNumber, 0, sizeof( ss_AryPageNumber ) );

	getPageNumberSetting( &( ss_ChapterPageNumber[0] ) );

	for( uchar i = 0; PNLD_InsertPageNumber_Max > i; i++ ) {
		ss_AryPageNumber[ i ] = ss_ChapterPageNumber[ i ];
	}

	// 章分け番号・箇所数の設定.
	pc_Print->c_Chapter.setPages( &( ss_AryPageNumber[0] ) );
	pc_Print->c_Chapter.setCount( getPageNumberCount() );
}

//------------------------------------------------------------------------------
/**
* @brief A061152200 章分け : テンキー押下.
*/
//------------------------------------------------------------------------------
void PCPC_AuxilPageChapterEdit_job::proc10Key( uchar option )
{
	if( False == isOn( CNK_Chapter, PnlComGetJobCopy() )
	    || ( 30 <= getPageNumberCount() ) ) {

		// 無効音を鳴らす.
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;

	}

	PCPC_AuxilPageChapterEdit_job_com::proc10Key( option );
	setupGamen();
}
//------------------------------------------------------------------------------
/**
* @brief A061152200 章分け : クリアーキー押下.
*/
//------------------------------------------------------------------------------
void PCPC_AuxilPageChapterEdit_job::procClear( uchar option )
{
	if( False == isOn( CNK_Chapter, PnlComGetJobCopy() ) ) {

		// 無効音を鳴らす.
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;

	}

	PCPC_AuxilPageChapterEdit_job_com::procClear( option );
	setupGamen();
}

//------------------------------------------------------------------------------
/**
* @brief A061152200 章分け : スタートキー.
*/
//------------------------------------------------------------------------------
void PCPC_AuxilPageChapterEdit_job::procStart( uchar option )
{
	PNLC_OutSideKeyCallDataLib::newInstance()->setSelectNo( PNLE_Soft10KeyUnselected );	// データクリア.
	if( NULL == PnlComGetJobCopy() ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }
	TYPC_Print* pc_Print = PnlComGetJobCopy()->getJobSpec()->pc_Print;
	if( ( True == isOn( CNK_Chapter, PnlComGetJobCopy() ) )   // する状態.
	    && ( 0 == pc_Print->c_Chapter.getCount() ) ) {    // ページ設定0.
//		// 状態をOFFに戻す
		setBuzzerType( PCGE_BuzzerType_InputNg );
	} else {
		PCPC_AuxilPageChapterEdit_job_com::procStart( option );
	}
}

//------------------------------------------------------------------------------
/**
* @brief A061152200 章分け : ユーティリティキー.
*/
//------------------------------------------------------------------------------
void PCPC_AuxilPageChapterEdit_job::procUtility( uchar option )
{
	PNLC_OutSideKeyCallDataLib::newInstance()->setSelectNo( PNLE_Soft10KeyUnselected );	// データクリア.
	if( NULL == PnlComGetJobCopy() ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }
	TYPC_Print* pc_Print = PnlComGetJobCopy()->getJobSpec()->pc_Print;
	// USBアクセス(印刷、保存)中は受け付け禁止.
	APIC_UsbMemDeviceInfo* pc_USBMemoryInfo = APIC_UsbMemDeviceInfo::newInstance();
	if( NULL == pc_USBMemoryInfo ) {
		PnlComPutDebugLog( False, UC_PNL_LOGPATTERN_1, "PCPC_AuxilPageChapterEdit_job::procUtility", UC_PNL_ERROR_IMPOSSIBLE );
		return;
	}
	if( ( True == pc_USBMemoryInfo->isAccess( TYPE_UsbMemAccessRead, TYPE_UsbMemAppIdStorageAccessApp ) ) ||
	    ( True == pc_USBMemoryInfo->isAccess( TYPE_UsbMemAccessWrite, TYPE_UsbMemAppIdNetTxProcess ) ) ) {
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}
	if( ( True == isOn( CNK_Chapter, PnlComGetJobCopy() ) )   // する状態.
	    && ( 0 == pc_Print->c_Chapter.getCount() ) ) {    // ページ設定0.
		onlyPanel.setErrorNumberFieldPartsKind( EP_ErrorPartsKind_NumberField );
		m_b_ErrorOn = True;
		setupGamen();
	} else {
		PCPC_AuxilPageChapterEdit_job_com::procUtility( option );
	}
}

//------------------------------------------------------------------------------.
/**
 *	@brief	A061152200 章分け : 割り込みモード
 */
//------------------------------------------------------------------------------.
void PCPC_AuxilPageChapterEdit_job::procIntMode( uchar option )
{
	PNLC_OutSideKeyCallDataLib::newInstance()->setSelectNo( PNLE_Soft10KeyUnselected );	// データクリア.
	PCPC_AuxilPageChapterEdit_job_com::procIntMode( option );
}
//------------------------------------------------------------------------------.
/**
 *	@brief	A061152200 章分け : 暗証番号入力
 */
//------------------------------------------------------------------------------.
void PCPC_AuxilPageChapterEdit_job::procIDInput( uchar option )
{
	PNLC_OutSideKeyCallDataLib::newInstance()->setSelectNo( PNLE_Soft10KeyUnselected );	// データクリア.
	PCPC_AuxilPageChapterEdit_job_com::procIDInput( option );
}
//------------------------------------------------------------------------------.
/**
 *	@brief	A061152200 章分け : コピー
 */
//------------------------------------------------------------------------------.
void PCPC_AuxilPageChapterEdit_job::procCopy( uchar option )
{
	// MFP基本機能使用設定が無効の場合.
	if( False == c_ExtPanel.isEnableMFPFunction() ) {
		// キーを受け付けない.
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}

	PNLC_OutSideKeyCallDataLib::newInstance()->setSelectNo( PNLE_Soft10KeyUnselected );	// データクリア.
	PCPC_AuxilPageChapterEdit_job_com::procCopy( option );
}
//------------------------------------------------------------------------------.
/**
 *	@brief	A061152200 章分け : 拡大表示
 */
//------------------------------------------------------------------------------.
void PCPC_AuxilPageChapterEdit_job::procEnlarge( uchar option )
{
	PNLC_OutSideKeyCallDataLib::newInstance()->setSelectNo( PNLE_Soft10KeyUnselected );	// データクリア.
	PCPC_AuxilPageChapterEdit_job_com::procEnlarge( option );
}
//------------------------------------------------------------------------------.
/**
 *	@brief	A061152200 章分け : 確認コピー
 */
//------------------------------------------------------------------------------.
void PCPC_AuxilPageChapterEdit_job::procSampleCopy( uchar option )
{
	PNLC_OutSideKeyCallDataLib::newInstance()->setSelectNo( PNLE_Soft10KeyUnselected );	// データクリア.
	PCPC_AuxilPageChapterEdit_job_com::procSampleCopy( option );
}
//------------------------------------------------------------------------------.
/**
 *	@brief	A061152200 章分け : ジョブメモリ
 */
//------------------------------------------------------------------------------.
void PCPC_AuxilPageChapterEdit_job::procJobMemory( uchar option )
{
	PNLC_OutSideKeyCallDataLib::newInstance()->setSelectNo( PNLE_Soft10KeyUnselected );	// データクリア.
	PCPC_AuxilPageChapterEdit_job_com::procJobMemory( option );
}
//------------------------------------------------------------------------------.
/**
 *	@brief	A061152200 章分け : FAX
 */
//------------------------------------------------------------------------------.
void PCPC_AuxilPageChapterEdit_job::procFax( uchar option )
{
	// MFP基本機能使用設定が無効の場合.
	if( False == c_ExtPanel.isEnableMFPFunction() ) {
		// キーを受け付けない.
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}

	PNLC_OutSideKeyCallDataLib::newInstance()->setSelectNo( PNLE_Soft10KeyUnselected );	// データクリア.
	PCPC_AuxilPageChapterEdit_job_com::procFax( option );
}
//------------------------------------------------------------------------------.
/**
 *	@brief	A061152200 章分け : FAX
 */
//------------------------------------------------------------------------------.
void PCPC_AuxilPageChapterEdit_job::procFaxOnly( uchar option )
{
	// MFP基本機能使用設定が無効の場合.
	if( False == c_ExtPanel.isEnableMFPFunction() ) {
		// キーを受け付けない.
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}

	PNLC_OutSideKeyCallDataLib::newInstance()->setSelectNo( PNLE_Soft10KeyUnselected );	// データクリア.
	PCPC_AuxilPageChapterEdit_job_com::procFaxOnly( option );
}

// [制御構成変更] ソフトテンキー外キー対応の為メソッド追加.
//------------------------------------------------------------------------------.
/**
 *	@brief	A061152200 章分け : ボックス
 */
//------------------------------------------------------------------------------.
void PCPC_AuxilPageChapterEdit_job::procBox( uchar option )
{
	// MFP基本機能使用設定が無効の場合.
	if( False == c_ExtPanel.isEnableMFPFunction() ) {
		// キーを受け付けない.
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}

	PNLC_OutSideKeyCallDataLib::newInstance()->setSelectNo( PNLE_Soft10KeyUnselected );	// データクリア.
	PCPC_AuxilPageChapterEdit_job_com::procBox( option );
}
//------------------------------------------------------------------------------.
/**
 *	@brief	A061152200 章分け : ユビキタスプリント
 */
//------------------------------------------------------------------------------.
void PCPC_AuxilPageChapterEdit_job::procUbqPrint( uchar option )
{
	// MFP基本機能使用設定が無効の場合.
	if( False == c_ExtPanel.isEnableMFPFunction() ) {
		// キーを受け付けない.
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}

	PNLC_OutSideKeyCallDataLib::newInstance()->setSelectNo( PNLE_Soft10KeyUnselected );	// データクリア.
	PCPC_AuxilPageChapterEdit_job_com::procUbqPrint( option );
}
//------------------------------------------------------------------------------.
/**
 *	@brief	A061152200 章分け : パワーセーブ
 */
//------------------------------------------------------------------------------.
void PCPC_AuxilPageChapterEdit_job::procPowerSave( uchar option )
{
	PNLC_OutSideKeyCallDataLib::newInstance()->setSelectNo( PNLE_Soft10KeyUnselected );	// データクリア.
	PCPC_AuxilPageChapterEdit_job_com::procPowerSave( option );
}
//------------------------------------------------------------------------------.
/**
 *	@brief	A061152200 章分け : ヘルプ
 */
//------------------------------------------------------------------------------.
void PCPC_AuxilPageChapterEdit_job::procHelp( uchar option )
{
	PNLC_OutSideKeyCallDataLib::newInstance()->setSelectNo( PNLE_Soft10KeyUnselected );	// データクリア.
	PCPC_AuxilPageChapterEdit_job_com::procHelp( option );
}
//------------------------------------------------------------------------------.
/**
 *	@brief	A061152200 章分け : ユーザー補助
 */
//------------------------------------------------------------------------------.
void PCPC_AuxilPageChapterEdit_job::procAccessibility( uchar option )
{
	PNLC_OutSideKeyCallDataLib::newInstance()->setSelectNo( PNLE_Soft10KeyUnselected );	// データクリア.
	PCPC_AuxilPageChapterEdit_job_com::procAccessibility( option );
}
//------------------------------------------------------------------------------.
/**
 *	@brief	A061152200 章分け : IDキー処理
 */
//------------------------------------------------------------------------------.
void PCPC_AuxilPageChapterEdit_job::procIDInputVoice( uchar option )
{
	PNLC_OutSideKeyCallDataLib::newInstance()->setSelectNo( PNLE_Soft10KeyUnselected );	// データクリア.
	PCPC_AuxilPageChapterEdit_job_com::procIDInputVoice( option );
}
//------------------------------------------------------------------------------.
/**
 *	@brief	A061152200 章分け : ブラウザ
 */
//------------------------------------------------------------------------------.
void PCPC_AuxilPageChapterEdit_job::procBrowser( uchar option )
{
	// MFP基本機能使用設定が無効の場合.
	if( False == c_ExtPanel.isEnableMFPFunction() ) {
		// キーを受け付けない.
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}

	PNLC_OutSideKeyCallDataLib::newInstance()->setSelectNo( PNLE_Soft10KeyUnselected );	// データクリア.
	PCPC_AuxilPageChapterEdit_job_com::procBrowser( option );
}
//------------------------------------------------------------------------------.
/**
* @brief A061152200 章分け : ソフトテンキー呼出しキー.
*/
//------------------------------------------------------------------------------.
void PCPC_AuxilPageChapterEdit_job::procSoft10Key( uchar option )
{
	if( False == isOn( CNK_Chapter, PnlComGetJobCopy() )
	    || ( 30 <= getPageNumberCount() )
	    || ( False == m_bl_isSelectedInputField ) ) {	// comment 2012/06/21 OPS1川手 [MinervaBK][CQ366316]ソフト10キーUD対応 No.1 [数値入力エリア非選択状態の条件を追加].
		// 無効音を鳴らす.
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}

	// スライドレス、MTUI有効の場合.
	if( True == PLBC_NonSlideMultiTouchFunction::isEnable() ) {
		// ソフトテンキー表示(センター表示).
		PnlComMtuiIf_TenkeyPad::instance()->showDefaultPos();
	} else {
		if( PNLE_Soft10KeyUnselected != PNLC_OutSideKeyCallDataLib::newInstance()->getSelectNo() ) { return; }

		Bool bl_NonSetting = ( m_ss_InputPageNumber > 0 ) ? False : True;		// 未入力状態チェック.
		PNLC_OutSideKeyCallDataLib::newInstance()->setColorMode( PNLE_Soft10KeyColorModeCommon );
		PNLC_OutSideKeyCallDataLib::newInstance()->setSelectNo( PNLE_Soft10KeySelected );
		PNLC_OutSideKeyCallDataLib::newInstance()->setInputNumeric( m_ss_InputPageNumber, False, 1, PCGD_MaxPageNumber, bl_NonSetting );
		PNLC_PopupListControl::newInstance()->addPopup( &gp_Soft10Key_PageLocation, this );
	}
}

//------------------------------------------------------------------------------.
/**
* @brief A061152200 章分け : アクティブ状態の通知.
*/
//------------------------------------------------------------------------------.
void PCPC_AuxilPageChapterEdit_job::procActivate()
{
	checkSoft10KeyData();

	// 親クラスを呼び出し.
	PCPC_AuxilPageChapterEdit_job_com::procActivate();
}

//------------------------------------------------------------------------------.
/**
 *	@brief	A061152200 章分け : ソフトテンキーデータチェック.
 */
//------------------------------------------------------------------------------.
void PCPC_AuxilPageChapterEdit_job::checkSoft10KeyData()
{
	char sc_No = PNLC_OutSideKeyCallDataLib::newInstance()->getSelectNo();
	if( PNLE_Soft10KeyUnselected != sc_No ) {
		float f_Num;
		// 入力値を取得して更新.
		Bool bl_NonSetting = PNLC_OutSideKeyCallDataLib::newInstance()->getInputNumeric( &f_Num );
		if( False == bl_NonSetting ) {
			m_ss_InputPageNumber = ( short )f_Num;
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
void PCPC_AuxilPageChapterEdit_job::show10KeyPopup()
{
	if( True != PLBC_NonSlideMultiTouchFunction::isEnable() ) {
		if( True == PLBC_FuncVersionCheck::isOK_Type4() ) {
			procSoft10Key( 0 );
		} else {
			setBuzzerType( PCGE_BuzzerType_Off, True );		// ブザー鳴らさない.
		}
	} else {
		if( False == isOn( CNK_Chapter, PnlComGetJobCopy() )
		    || ( 30 <= getPageNumberCount() ) ) {
			setBuzzerType( PCGE_BuzzerType_InputNg );		// 報知音設定.
		} else {
			// ソフトテンキー表示.
			const short PNLD_TenkeyPad_correctX = 10;
			const short PNLD_TenkeyPad_correctY = 64;

			short ss_SizeX, ss_SizeY, ss_KeyPosX, ss_KeyPosY, ss_PosX, ss_PosY;
			Key_AuxilPageChapterEdit_jobAdd.getPosition( ss_KeyPosX, ss_KeyPosY );
			Key_AuxilPageChapterEdit_jobAdd.getSize( ss_SizeX, ss_SizeY );
			this->getPosition( &ss_PosX, &ss_PosY );
			// 表示位置（画面表示位置＋追加キー表示位置＋キー幅＋補正値：画面表示位置＋メッセージエリア表示位置＋幅）
			PnlComMtuiIf_TenkeyPad::instance()->show( static_cast<short>( ss_KeyPosX+ss_SizeX+PNLD_TenkeyPad_correctX ), static_cast<short>( ss_PosY+PNLD_TenkeyPad_correctY ) );
		}
	}
}

/**
 * @brief	MTUIテンキーポップアップ非表示処理.
 * @param	.
 */
void PCPC_AuxilPageChapterEdit_job::hide10KeyPopup()
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
void PCPC_AuxilPageChapterEdit_job::procSlideMenu( uchar option )
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
Bool PCPC_AuxilPageChapterEdit_job::IsShowSlide10Key()
{
	if( ( False == isOn( CNK_Chapter, PnlComGetJobCopy() ) )
	    ||	( 30 <= getPageNumberCount() )
	    ||	( False == m_bl_isSelectedInputField ) ) {
		return False;
	}

	return True;
}

// ===============================================================================================.
//	概要	: スライドメニュー入力エリアデータ送信.
//  引数    : True：初期化あり、False：初期化なし.
//	戻り値	: なし.
// ===============================================================================================.
void PCPC_AuxilPageChapterEdit_job::sendSlide10KeyData( Bool bl_Flag )
{
	uchar puc_Dst[PNLD_SLIDEMENU_TEXTBUFFER_SIZE];				//整数部(/小数)変換後格納先.
	memset( puc_Dst,0,sizeof( puc_Dst ) );

	if( True == bl_Flag ) {
		//スライドメニューOpen時の処理.
		PNLC_SoftSlideMenuTenKeyTextDispLib::newInstance()->create( PNLE_OutsideKeyCall_RIGHT_D_PAGELOCATION );
		PNLC_SoftSlideMenuTenKeyConvLib::newInstance()->setParam( PnlComGetUnitSystem(), "%3d", NULL );
	}

	//エラーチェックはなし.
	if( 0 != m_ss_InputPageNumber ) {
		PNLC_SoftSlideMenuTenKeyConvLib::newInstance()->getSetText( puc_Dst,( int32_t )m_ss_InputPageNumber,True,False );
	}
	//インサート固定.
	PNLC_SoftSlideMenuTenKeyTextDispLib::newInstance()->update( puc_Dst,						//文字列.
	        PNLD_CURSOR_TYPE_INSERT,	//カーソルタイプ.
	        ( short )strlen( ( char* )puc_Dst ),		//カーソルポジション.
	        True );						//ブリンク.
}

/*================================================================================================
 マクロ関数の定義.
================================================================================================*/

/*================================================================================================
PCPC_AuxilPageIntersheet_job_com
================================================================================================*/

PCPC_AuxilPageIntersheet_job_com::PCPC_AuxilPageIntersheet_job_com(
    PDisplayParts** partsTable,
    short partsNumber,
    Bool isNoWarningGamen,
    PPopupAnimationGroup* pc_AnimationComponent,
    short ss_PosX,
    short ss_PosY,
    short ss_SizeX,
    short ss_SizeY,
    PDisplayParts* pc_FunctionBadgeButton
): PPopupJobSetupChangeCom( partsTable, partsNumber, isNoWarningGamen, pc_AnimationComponent,
	                            ss_PosX, ss_PosY, ss_SizeX, ss_SizeY, pc_FunctionBadgeButton )
{
	m_uc_NowPage = PNLE_AuxilPageIntersheetEdit_job_comPageNumber01;
	m_ss_InputPageNumber = 0;
	m_ss_InputPageNumberBackup = 0;

	for( uchar i = 0; PNLD_InsertPageNumber_Max > i; i++ ) {

		// ページ番号の設定.
		m_s_PageNumberStatus[i].ss_PageNumber = 0;

		// 入力済みフラグの設定.
		m_s_PageNumberStatus[i].b_Input = False;

		// ページ番号の選択状態を設定.
		m_s_PageNumberStatus[i].b_Select = False;

	}
}

//------------------------------------------------------------------------------
/**
* @brief 共通処理 : 画面開始処理.
*/
//------------------------------------------------------------------------------
void PCPC_AuxilPageIntersheet_job_com::startGamen()
{
	PPopupJobSetupChangeCom::startGamen();

	m_uc_NowPage = PNLE_AuxilPageIntersheetEdit_job_comPageNumber01;
	m_ss_InputPageNumber = 0;
	m_ss_InputPageNumberBackup = 0;
}

//------------------------------------------------------------------------------
/**
* @brief 共通処理 : テンキー.
*/
//------------------------------------------------------------------------------
void PCPC_AuxilPageIntersheet_job_com::proc10Key( uchar option )
{
	Bool b_Error = False;
	// 保持値が０の場合は０入力禁止.
	if( 0 == m_ss_InputPageNumber && 0 == option ) {
		PBasicMessageProc::setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}
	if( option <= 9 ) {
		// テンキーで入力された値の算出.
		m_ss_InputPageNumber = ( short )proc10KeyInput( ( int32_t )( m_ss_InputPageNumber ),PCGD_MaxPageNumber, option, False, EP_InputMaxLimitPattern_Refuse, &b_Error );
		// エラー時は警告音を鳴らす.
		if( b_Error ) {
			PBasicMessageProc::setBuzzerType( PCGE_BuzzerType_InputNg );
		}
	}
}

//------------------------------------------------------------------------------
/**
* @brief 共通処理 : クリアー.
*/
//------------------------------------------------------------------------------
void PCPC_AuxilPageIntersheet_job_com::procClear( uchar option )
{
	if( m_ss_InputPageNumber == 0 && checkPageSelectStatus() == False ) {

		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}

	// ページ番号の選択を解除する.
	clearPageSelectStatus();

	// 入力エリアの値を初期化する.
	clearInputPageNumber();

}

//------------------------------------------------------------------------------
/**
* @brief 共通処理 : ページ番号の取得.
*/
//------------------------------------------------------------------------------
void PCPC_AuxilPageIntersheet_job_com::getPageNumber( short* const pss_PageNumber )
{
	// 検索開始位置を設定する.
	const uchar uc_StartIndex = ( ( m_uc_NowPage == PNLE_AuxilPageIntersheetEdit_job_comPageNumber01 ) ? uchar( 0 ) : uchar( PNLD_InsertPageNumber_HALF ) );

	for( uchar i = 0, j = uc_StartIndex; PNLD_InsertPageNumber_HALF > i; i++, j++ ) {

		// ページ番号を取得する.
		pss_PageNumber[i] = m_s_PageNumberStatus[j].ss_PageNumber;

	}
}

//------------------------------------------------------------------------------
/**
* @brief 共通処理 : ページ番号の設定.
*/
//------------------------------------------------------------------------------
void PCPC_AuxilPageIntersheet_job_com::setPageNumber( const short* pss_PageNumber )
{
	// 引数のNULLチェック.
	if( NULL == pss_PageNumber ) {

		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return;
	}

	for( uchar i = 0; PNLD_InsertPageNumber_Max > i; i++ ) {

		// ページ番号の設定.
		m_s_PageNumberStatus[i].ss_PageNumber = pss_PageNumber[i];

		// 入力済みフラグの設定.
		m_s_PageNumberStatus[i].b_Input = ( ( 0 == pss_PageNumber[i] ) ? ( False ) : ( True ) );

		// ページ番号の選択状態を設定.
		m_s_PageNumberStatus[i].b_Select = False;

	}
}

//------------------------------------------------------------------------------
/**
* @brief 共通処理 : ページ番号の選択状態を取得する.
*/
//------------------------------------------------------------------------------
Bool PCPC_AuxilPageIntersheet_job_com::getPageSelectStatus( uchar uc_Index )
{
	if( m_uc_NowPage == PNLE_AuxilPageIntersheetEdit_job_comPageNumber02 ) {

		uc_Index = static_cast<uchar>( uc_Index + PNLD_InsertPageNumber_HALF );

	}

	// ページ番号の選択状態を取得する.
	return m_s_PageNumberStatus[uc_Index].b_Select;

}

//------------------------------------------------------------------------------
/**
* @brief 共通処理 : ページ番号の選択状態を設定する.
*/
//------------------------------------------------------------------------------
void PCPC_AuxilPageIntersheet_job_com::setPageSelectStatus( PNLE_AuxilPageIntersheetEdit_job_comSelectPageNumber e_SelectPageNumber )
{
	uchar uc_Index = e_SelectPageNumber;

	if( m_uc_NowPage == PNLE_AuxilPageIntersheetEdit_job_comPageNumber02 ) {

		uc_Index = static_cast<uchar>( uc_Index + PNLD_InsertPageNumber_HALF );

	}

	if( m_s_PageNumberStatus[uc_Index].b_Select == True ) {

		m_s_PageNumberStatus[uc_Index].b_Select = False;

	} else {

		m_s_PageNumberStatus[uc_Index].b_Select = True;

	}
}

//------------------------------------------------------------------------------
/**
* @brief 共通処理 : ページ番号を非選択にする.
*/
//------------------------------------------------------------------------------
void PCPC_AuxilPageIntersheet_job_com::clearPageSelectStatus()
{
	for( uchar i = 0; i < PNLD_InsertPageNumber_Max; i++ ) {
		// ページ番号を非選択にする.
		m_s_PageNumberStatus[i].b_Select = False;
	}

}

//------------------------------------------------------------------------------
/**
* @brief 共通処理 : ページ番号が選択されているかどうか判定する.
*/
//------------------------------------------------------------------------------
Bool PCPC_AuxilPageIntersheet_job_com::checkPageSelectStatus()
{
	for( uchar i = 0; i < PNLD_InsertPageNumber_Max; i++ ) {

		if( m_s_PageNumberStatus[i].b_Select == True ) {

			return True;

		}
	}

	return False;

}

//------------------------------------------------------------------------------
/**
* @brief 共通処理 : ページ数を取得する.
*/
//------------------------------------------------------------------------------
uchar PCPC_AuxilPageIntersheet_job_com::getNowPage()
{
	uchar uc_PageNumber = PNLE_AuxilPageIntersheetEdit_job_comPageNumber01;
	for( uchar i = 0; PNLD_InsertPageNumber_Max > i; i++ ) {

		if( m_s_PageNumberStatus[i].b_Select == True ) {

			if( PNLD_InsertPageNumber_HALF <= i ) {

				uc_PageNumber = PNLE_AuxilPageIntersheetEdit_job_comPageNumber02;
			}
			break;
		}
	}

	return uc_PageNumber;
}

//------------------------------------------------------------------------------
/**
* @brief 共通処理 : ページ番号の登録数を取得する.
*/
//------------------------------------------------------------------------------
uchar PCPC_AuxilPageIntersheet_job_com::getPageNumberCount()
{
	// ページ番号の登録数.
	uchar uc_count = 0;

	for( uchar i = 0; PNLD_InsertPageNumber_Max > i; i++ ) {

		// ページ番号が設定されている場合.
		if( m_s_PageNumberStatus[i].ss_PageNumber != 0 ) {

			// ページ番号の登録数をインクリメントする.
			uc_count++;
		}
	}

	return uc_count;

}

//------------------------------------------------------------------------------
/**
* @brief 共通処理 : ページ番号の取得.
*/
//------------------------------------------------------------------------------
void PCPC_AuxilPageIntersheet_job_com::getPageNumberSetting( short* const pss_PageNumber )
{
	for( uchar i = 0; i < PNLD_InsertPageNumber_Max; i++ ) {

		// ページ番号を取得する.
		pss_PageNumber[i] = m_s_PageNumberStatus[i].ss_PageNumber;

	}
}

//------------------------------------------------------------------------------
/**
* @brief 共通処理 : 追加されたページ番号の位置を検索する.
*/
//------------------------------------------------------------------------------
Bool PCPC_AuxilPageIntersheet_job_com::SearchPageNumber()
{
	Bool b_ret = False;
	uchar i;
	for( i = 0; i < PNLD_InsertPageNumber_Max; i++ ) {

		if( m_s_PageNumberStatus[i].ss_PageNumber == m_ss_InputPageNumberBackup ) {

			break;

		}
	}

	if( PNLD_InsertPageNumber_HALF <= i ) {

		b_ret = True;

	}

	return b_ret;

}

//------------------------------------------------------------------------------
/**
* @brief 共通処理 : 重複しているページ番号があるかどうか判定する.
*/
//------------------------------------------------------------------------------
Bool PCPC_AuxilPageIntersheet_job_com::checkPageNumber()
{
	for( uchar i = 0; i < PNLD_InsertPageNumber_Max; i++ ) {

		if( m_s_PageNumberStatus[i].ss_PageNumber == m_ss_InputPageNumber ) {

			return True;

		}
	}

	return False;

}

//------------------------------------------------------------------------------
/**
* @brief 共通処理 : 入力エリアの値を取得.
*/
//------------------------------------------------------------------------------
short PCPC_AuxilPageIntersheet_job_com::getInputPageNumber()
{
	return m_ss_InputPageNumber;
}

//------------------------------------------------------------------------------
/**
* @brief 共通処理 : 入力エリアの値をクリア.
*/
//------------------------------------------------------------------------------
void PCPC_AuxilPageIntersheet_job_com::clearInputPageNumber()
{
	m_ss_InputPageNumber = 0;
}

//------------------------------------------------------------------------------
/**
* @brief 共通処理 : ページ番号の更新.
*/
//------------------------------------------------------------------------------
void PCPC_AuxilPageIntersheet_job_com::renewPageNumber()
{
	for( uchar i = 0; PNLD_InsertPageNumber_Max > i; i++ ) {

		// ページ番号が0の場合.
		if( m_s_PageNumberStatus[i].ss_PageNumber == 0 ) {

			// ページ番号を設定する.
			m_s_PageNumberStatus[i].ss_PageNumber = m_ss_InputPageNumber;
			m_s_PageNumberStatus[i].b_Input = True;

			// 入力エリアの値をバックアップする.
			m_ss_InputPageNumberBackup = m_ss_InputPageNumber;

			// 入力エリアの値を初期化する.
			clearInputPageNumber();
			break;
		}
	}
}

//------------------------------------------------------------------------------
/**
* @brief 共通処理 : ページ番号の削除.
*/
//------------------------------------------------------------------------------
void PCPC_AuxilPageIntersheet_job_com::deletePageNumber()
{
	for( uchar i = 0; PNLD_InsertPageNumber_Max > i; i++ ) {

		if( m_s_PageNumberStatus[i].b_Select == True ) {

			// ページ番号を削除する.
			m_s_PageNumberStatus[i].ss_PageNumber = 0;
			m_s_PageNumberStatus[i].b_Input = False;
			m_s_PageNumberStatus[i].b_Select = False;

		}
	}
}

//------------------------------------------------------------------------------
/**
* @brief 共通処理 : ソート.
*/
//------------------------------------------------------------------------------
void PCPC_AuxilPageIntersheet_job_com::procSortExec()
{
	short ss_temp;
	Bool  bl_temp;
	ushort i,j;
	ushort us_Number_Cnt_Max = PNLD_InsertPageNumber_Max;

	for( i=0; i < ( ushort )( us_Number_Cnt_Max-1 ); i++ ) {

		for( j=0; j < ( ushort )( us_Number_Cnt_Max-1-i ); j++ ) {

			if( ( ( m_s_PageNumberStatus[j+1].b_Input == True ) && ( m_s_PageNumberStatus[j+1].ss_PageNumber != 0 ) ) &&
			    ( ( m_s_PageNumberStatus[j].b_Input == False ) ||
			      ( m_s_PageNumberStatus[j].ss_PageNumber == 0 ) ||
			      ( ( m_s_PageNumberStatus[j].ss_PageNumber != 0 ) && ( m_s_PageNumberStatus[j].ss_PageNumber > m_s_PageNumberStatus[j+1].ss_PageNumber ) ) ) ) {

				ss_temp = m_s_PageNumberStatus[j].ss_PageNumber;
				m_s_PageNumberStatus[j].ss_PageNumber = m_s_PageNumberStatus[j+1].ss_PageNumber;
				m_s_PageNumberStatus[j+1].ss_PageNumber = ss_temp;

				bl_temp = m_s_PageNumberStatus[j].b_Input;
				m_s_PageNumberStatus[j].b_Input = m_s_PageNumberStatus[j+1].b_Input;
				m_s_PageNumberStatus[j+1].b_Input = bl_temp;

				bl_temp = m_s_PageNumberStatus[j].b_Select;
				m_s_PageNumberStatus[j].b_Select = m_s_PageNumberStatus[j+1].b_Select;
				m_s_PageNumberStatus[j+1].b_Select = bl_temp;

			}
		}
	}
}


//------------------------------------------------------------------------------
/**
* @brief A061152300 インターシート : コンストラクタ.
*/
//------------------------------------------------------------------------------
PCPC_AuxilPageIntersheetEdit_job::PCPC_AuxilPageIntersheetEdit_job(
    PDisplayParts** partsTable,
    short partsNumber,
    Bool isNoWarningGamen,
    PPopupAnimationGroup* pc_AnimationComponent,
    short ss_PosX,
    short ss_PosY,
    short ss_SizeX,
    short ss_SizeY,
    PDisplayParts* pc_FunctionBadgeButton
): PCPC_AuxilPageIntersheet_job_com( partsTable, partsNumber, isNoWarningGamen, pc_AnimationComponent, ss_PosX, ss_PosY, ss_SizeX, ss_SizeY, pc_FunctionBadgeButton )
	,
	m_b_BlinkOn( False )
{
	// エラーフラグの初期化.
	m_b_ErrorOn = False;

	// インターシートページ登録フラグ.
	m_b_RegistrationFlg = False;

	// JobSpecのデータ保存管理用テーブルNoの初期化.
	m_ss_JobSpecBackupTableNo = -1;

	m_bl_isSelectedInputField = True; 	// comment 2012/06/21 OPS1川手 [MinervaBK][CQ366316]ソフト10キーUD対応 No.1 [数値入力エリアの初期化].

}

//------------------------------------------------------------------------------
/**
* @brief A061152300 インターシート : 画面開始処理.
*/
//------------------------------------------------------------------------------
void PCPC_AuxilPageIntersheetEdit_job::startGamen()
{
	PNLC_OutSideKeyCallDataLib::newInstance()->setSelectNo( PNLE_Soft10KeyUnselected );			// データクリア.

	// エラーフラグの初期化.
	m_b_ErrorOn = False;

	// インターシートページ登録フラグ.
	m_b_RegistrationFlg = False;

	// JobSpecのデータ保存管理用テーブルNoの初期化
	m_ss_JobSpecBackupTableNo = -1;

	PCPC_AuxilPageIntersheet_job_com::startGamen();

	// インターシートページ番号を取得する.
	getIntersheetPageNumber();

	// 主テキストメッセージを変更する.
// Comment 2016/10/26 [HCL_Ravi] [IT5_4.0] [1006281] Changes made for conversion to half font.
	//Cmp_AuxilPageIntersheetEdit_jobMessage.changeMainMessageText( s_pMSGSetInsertPositionByKeypad_W448H034A );
	Cmp_AuxilPageIntersheetEdit_jobMessage.changeMainMessageText( s_pMSGSetInsertPositionByKeypad_W546H034A );

	// ON/OFFを表示.
	Cmp_AuxilPageIntersheetEdit_jobMessage.setOnOffButton( True );

	// ON/OFFキーの選択状態変更.
	Cmp_AuxilPageIntersheetEdit_jobMessage.setOnOffSwitchButtonState( isOn( CNK_MiddleMode, PnlComGetJobCopy() ) );
	Cmp_AuxilPageIntersheetEdit_jobMessage.changeMessageDisplay( PNLE_PopupMessageAreaMessageShowAll );
	Cmp_AuxilPageIntersheetEdit_jobMessage.setMode( PNLE_PopupMessageAreaMessageModeAdmin );
	Cmp_AuxilPageIntersheetEdit_jobMessage.attachMessage( True );
	Cmp_AuxilPageIntersheetEdit_jobMessage.showDestinationCount( False );
	Cmp_AuxilPageIntersheetEdit_jobMessage.setNowPopupGamen( PNLE_AuxilPageIntersheetEdit_job );

	Key_AuxilPageIntersheetEdit_jobInsertSheetX.showEnable( False ); //.activate(False);
	Key_AuxilPageIntersheetEdit_jobInsertSheetY.showEnable( False ); //activate(False);
	// スクロールバーから画面へキーを流す.
	Cmp_AuxilPageIntersheetEdit_jobScrollBar.attachMessage( True );

	m_bl_isSelectedInputField = ( True == PnlComIsHard10Key() ) ? True : False; // comment 2012/06/21 OPS1川手 [MinervaBK][CQ366316]ソフト10キーUD対応 No.1 [数値入力エリアの初期化].
	// 入力エリア押下機能有無.
	if( True != PLBC_FuncVersionCheck::isOK_Type4() ) {
		Key_AuxilPageIntersheetEdit_jobPage.setShowPush( False );
		Key_AuxilPageIntersheetEdit_jobPageError.setShowPush( False );
	}
//comment 2018/5/18 SCD2-Sky肥後 [DenebMLK][機器構成変更No.7]5段目トレイ追加対応.
	if( False == isOn( CNK_MiddleMode, PnlComGetJobCopy() ) ) {
		// インターシートの取得.
		TYPC_Print* pc_Print = PnlComGetJobCopy()->getJobSpec()->pc_Print;
		TYPC_IntersheetMode c_IntersheetMode = pc_Print->c_Intersheet.getMiddleMode();
		TYPE_FeedTray e_FeedTray = getIntersheetFeedTray();
		for(uchar i = 0;  i < PNLD_InsertPageNumber_Max; i++ ) {
			// インターシートの選択トレイを設定.
			pc_Print->c_Intersheet.setFeedTrayMiddle( i, e_FeedTray );
		}
		// インターシートの選択トレイを設定.
		c_IntersheetMode.setFeedTray( e_FeedTray );
		pc_Print->c_Intersheet.setMiddleMode( c_IntersheetMode );
	}

}

//------------------------------------------------------------------------------
/**
* @brief A061152300 インターシート : 表示部品のセットアップ.
*/
//------------------------------------------------------------------------------
void PCPC_AuxilPageIntersheetEdit_job::setupGamen( short option )
{
	// インターシートページ番号.
	short ss_AryPageNumber[PNLD_InsertPageNumber_HALF];
	// インターシートページ番号の登録数/
	uchar uc_Count = 0;
	// ループカウンタ.
	uchar i = 0;
//comment 2018/5/18 SCD2-Sky肥後 [DenebMLK][機器構成変更No.7]5段目トレイ追加対応.
	// インターシートトレイに関するの表示.
	TYPE_FeedTray e_FeedTray = getIntersheetFeedTray();


	PCPC_AuxilPageIntersheet_job_com::setupGamen( option );

	if( NULL == PnlComGetJobCopy() ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }
	if( False == isOn( CNK_MiddleMode, PnlComGetJobCopy() ) ) {

		// [OK][中止][ON][OFF]以外を非活性化.
		Txt_AuxilPageIntersheetEdit_jobInsertPagePosition.activate( False );
		Txt_AuxilPageIntersheetEdit_jobInsertMethod.activate( False );
		Cmp_AuxilPageIntersheetEdit_jobScrollBar.activate( False );
		Key_AuxilPageIntersheetEdit_jobPageError.showEnable( False );
		Key_AuxilPageIntersheetEdit_jobPage.showEnable( True );
		Key_AuxilPageIntersheetEdit_jobPage.activate( False );
		Key_AuxilPageIntersheetEdit_jobAdd.activate( False );
		Key_AuxilPageIntersheetEdit_jobInsertSheet.activate( False );
		Key_AuxilPageIntersheetEdit_jobDelete.activate( False );
		Key_AuxilPageIntersheetEdit_jobOK.activate( True );
		Rtb_AuxilPageIntersheetEdit_jobEditNum.activate( False );
		Rtb_AuxilPageIntersheetEdit_jobInsertMethod.activate( False );

		// インターシートページ番号を取得する.
		getPageNumber( &( ss_AryPageNumber[0] ) );

		for( i = 0; i < PNLD_InsertPageNumber_HALF; i++ ) {

			if( ss_AryPageNumber[i] != 0 ) {
				// インターシートページ番号を設定する.
				( ( PNormalButton* )tbl_AuxilPageIntersheetEdit_jobEditNum[i] )->set( ss_AryPageNumber[i], True );

				// 挿入位置指定キーを表示する.
				tbl_AuxilPageIntersheetEdit_jobEditNum[i]->showEnable( True );

				// インターシートページ番号を非活性化にする.
				( ( PNormalButton* )tbl_AuxilPageIntersheetEdit_jobEditNum[i] )->activate( False );

			} else {
				// 挿入位置指定キーを非表示にする.
				tbl_AuxilPageIntersheetEdit_jobEditNum[i]->showEnable( False );
			}
		}
		// インサートブリンク制御(非表示).
		m_b_BlinkOn = False;
		Key_AuxilPageIntersheetEdit_jobPage.showEnable( False );
		Gra_AuxilPageIntersheetEdit_jobInsertBlink.showEnable( m_b_BlinkOn );
		Key_AuxilPageIntersheetEdit_jobPage.showEnable( True );

	} else {

		// [OK][中止][ON][OFF]以外を活性化
		Txt_AuxilPageIntersheetEdit_jobInsertPagePosition.activate( True );
		Txt_AuxilPageIntersheetEdit_jobInsertMethod.activate( True );
		Cmp_AuxilPageIntersheetEdit_jobScrollBar.activate( True );
		Key_AuxilPageIntersheetEdit_jobPage.activate( True );
		Key_AuxilPageIntersheetEdit_jobAdd.activate( True );
		Key_AuxilPageIntersheetEdit_jobInsertSheet.activate( True );
		Key_AuxilPageIntersheetEdit_jobDelete.activate( True );
		Key_AuxilPageIntersheetEdit_jobOK.activate( True );
		Rtb_AuxilPageIntersheetEdit_jobEditNum.activate( True );
		Rtb_AuxilPageIntersheetEdit_jobInsertMethod.activate( True );

		// インターシートページ番号の登録数を取得する.
		uc_Count = getPageNumberCount();

		// 挿入紙の選択トレイを取得.
		TYPC_Print* pc_Print = PnlComGetJobCopy()->getJobSpec()->pc_Print;
//comment 2018/5/18 SCD2-Sky肥後 [DenebMLK][機器構成変更No.7]5段目トレイ追加対応.
//		TYPE_FeedTray e_FeedTray = pc_Print->c_Intersheet.getFeedTrayMiddle( 0 );
		e_FeedTray = pc_Print->c_Intersheet.getFeedTrayMiddle( 0 );
		// 挿入紙の選択トレイを取得.
		TYPC_IntersheetMode c_IntersheetMode = pc_Print->c_Intersheet.getMiddleMode();
		const TYPE_FeedTray e_FeedTrayMiddle = c_IntersheetMode.getFeedTray();

		if( e_FeedTray != e_FeedTrayMiddle ) {

			LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
			return;
		}
//comment 2018/5/18 SCD2-Sky肥後 [DenebMLK][機器構成変更No.7]5段目トレイ追加対応.
//		PNLS_setOriginalSizeCLCD2Key s_parts;
//
//		s_parts.ps_KeyOriginalSize  =    &Key_AuxilPageIntersheetEdit_jobInsertSheet;	// キー.
//		s_parts.ps_TxtOriginalSize01 =   &Ktx_AuxilPageIntersheetEdit_jobInsertSheet2;	// １行目 用紙コード表示.
//		s_parts.ps_TxtOriginalSize02 =   NULL;										// ２行目 自動表示.
//		s_parts.ps_TxtOriginalSizeX =    &Key_AuxilPageIntersheetEdit_jobInsertSheetX;	// 用紙サイズ表示 X.
//		s_parts.ps_TxtOriginalSizeY =    &Key_AuxilPageIntersheetEdit_jobInsertSheetY;	// 用紙サイズ表示 Y.
//		s_parts.ps_NumOriginalSizeX =    &Num_AuxilPageIntersheetEdit_jobInsertSheetX;	// 用紙サイズ数値表示 X.
//		s_parts.ps_NumOriginalSizeY =    &Num_AuxilPageIntersheetEdit_jobInsertSheetY;	// 用紙サイズ数値表示 Y.
//		s_parts.ps_TrayTxtPaperSize =    &Ktx_AuxilPageIntersheetEdit_jobInsertSheet1;
//		s_parts.ps_TrayIconPaperSize =   &Gra_AuxilPageIntersheetEdit_jobInsertSheet_Tray1;
//		s_parts.ps_RemainIconPaperSize = NULL;
//		s_parts.ps_TypeIconPaperSize =   NULL;
//
//		// インターシートトレイに関するの表示.
//		PnlComsetPaperSizeCLCD2Grap( &s_parts, e_FeedTray, PNLE_PaperSizeShowKind_Type1 );

		short ss_SelectPaperInsertion = -1;

		switch( c_IntersheetMode.getPaperInsertion() ) {
			// OFF.
			case TYPE_PaperInsertionOff:   ss_SelectPaperInsertion = -1; break;
			// 画像付き挿入.
			case TYPE_PaperInsertionWithImage:  ss_SelectPaperInsertion = 0; break;
			// 画像無し挿入.
			case TYPE_PaperInsertionWithoutImage: ss_SelectPaperInsertion = 1; break;
			default: LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return;
		}

		Rtb_AuxilPageIntersheetEdit_jobInsertMethod.select( ss_SelectPaperInsertion );

		if( c_IntersheetMode.getFeedTray() == TYPE_FeedTrayPI1 ||
		    c_IntersheetMode.getFeedTray() == TYPE_FeedTrayPI2 ) {
			Key_AuxilPageIntersheetEdit_jobDoCopy.activate( False );
		}

		// ページ数を取得する.
		m_uc_NowPage = Cmp_AuxilPageIntersheetEdit_jobScrollBar.getSliderPage();

		// インターシートページ番号の登録数を取得する.
		uc_Count = getPageNumberCount();

		if( uc_Count == 0 ) {
			// ページ数を変更する.
			m_uc_NowPage = PNLE_AuxilPageIntersheetEdit_job_comPageNumber01;

			// スライダを非活性化する.
			Cmp_AuxilPageIntersheetEdit_jobScrollBar.activate( False );

			// [OK]キーを非活性.
			Key_AuxilPageIntersheetEdit_jobOK.activate( False );

			// [挿入紙]キーを非活性.
			Key_AuxilPageIntersheetEdit_jobInsertSheet.activate( False );

		} else if( uc_Count < 16 ) {
			// ページ数を1ページ目に変更する.
			m_uc_NowPage = PNLE_AuxilPageIntersheetEdit_job_comPageNumber01;

			// スライダを非活性化する.
			Cmp_AuxilPageIntersheetEdit_jobScrollBar.activate( False );

		} else {
			if( m_b_RegistrationFlg == True ) {

				if( SearchPageNumber() == True ) {
					// ページ数を2ページ目に変更する.
					m_uc_NowPage = PNLE_AuxilPageIntersheetEdit_job_comPageNumber02;

					// スライダ移動.
					Cmp_AuxilPageIntersheetEdit_jobScrollBar.moveSliderPage( m_uc_NowPage );

				} else {
					// ページ数を1ページ目に変更する.
					m_uc_NowPage = PNLE_AuxilPageIntersheetEdit_job_comPageNumber01;

					// スライダ移動.
					Cmp_AuxilPageIntersheetEdit_jobScrollBar.moveSliderPage( m_uc_NowPage );
				}
				m_b_RegistrationFlg = False;
			}
		}

		// インターシートページ番号を取得する.
		getPageNumber( &( ss_AryPageNumber[0] ) );

		for( i = 0; i < PNLD_InsertPageNumber_HALF; i++ ) {

			if( ss_AryPageNumber[i] != 0 ) {
				// インターシートページ番号を設定する.
				( ( PNormalButton* )tbl_AuxilPageIntersheetEdit_jobEditNum[i] )->set( ss_AryPageNumber[i], True );

				// 挿入位置指定キーを表示する.
				tbl_AuxilPageIntersheetEdit_jobEditNum[i]->showEnable( True );

				// インターシートページ番号を選択状態にする.
				( ( PNormalButton* )tbl_AuxilPageIntersheetEdit_jobEditNum[i] )->select( getPageSelectStatus( i ) );

			} else {
				// 挿入位置指定キーを非表示にする.
				tbl_AuxilPageIntersheetEdit_jobEditNum[i]->showEnable( False );
			}
		}

		if( checkPageSelectStatus() == False ) {
			// [削除]キーを非活性化.
			Key_AuxilPageIntersheetEdit_jobDelete.activate( False );
		}

		// インターシートページ番号入力エリアがエラー表示中の場合.
		if( True == m_b_ErrorOn ) {
			// 1キー入力or一定時間でのエラー解除された場合.
			if( EP_ErrorPartsKind_ErrorOff == onlyPanel.getErrorNumberFieldPartsKind() ) {
				// パネルの状態をエラーなしにする.
				onlyPanel.setErrorNumberFieldPartsKind( EP_ErrorPartsKind_Non );
				// エラー表示を行わず、正常値を表示させる.
				m_b_ErrorOn = False;
			}
		}

		// 挿入位置入力エリアの値を取得する.
		short ss_InputPageNumber = getInputPageNumber();

		if( m_b_ErrorOn == True ) {
			// [追加]キーを非活性化.
			Key_AuxilPageIntersheetEdit_jobAdd.activate( False );

		} else {

			// 入力エラーを非表示.
			Key_AuxilPageIntersheetEdit_jobPageError.showEnable( False );
			// ページ表示エリアの表示.
			Key_AuxilPageIntersheetEdit_jobPage.showEnable( True );


			if( ss_InputPageNumber != 0 ) {

				// 入力されている値を設定.
				Key_AuxilPageIntersheetEdit_jobPage.set( ss_InputPageNumber, True, -1, False );
				// 入力エリアの値を表示.
				Key_AuxilPageIntersheetEdit_jobPage.showEnableAdd( True );

			} else {

				// 入力エリアの値を非表示.
				Key_AuxilPageIntersheetEdit_jobPage.showEnableAdd( False );

				// [追加]キーを非活性化.
				Key_AuxilPageIntersheetEdit_jobAdd.activate( False );

			}

		}
		// インサートブリンク制御.
		if( True == m_b_BlinkOn ||
		    False == m_bl_isSelectedInputField ) {	// comment 2012/06/21 OPS1川手 [MinervaBK][CQ366316]ソフト10キーUD対応 No.1 [数値入力エリアの選択状態更新].
			m_b_BlinkOn = False;
		} else {
			m_b_BlinkOn = True;
		}
		Key_AuxilPageIntersheetEdit_jobPage.showEnable( False );
		Gra_AuxilPageIntersheetEdit_jobInsertBlink.showEnable( m_b_BlinkOn );
		Key_AuxilPageIntersheetEdit_jobPage.showEnable( True );


		if( uc_Count >= 30 ) {
			Key_AuxilPageIntersheetEdit_jobPage.activate( False );
			// インサートブリンク制御.
			m_b_BlinkOn = False;

			Key_AuxilPageIntersheetEdit_jobPage.showEnable( False );
			Gra_AuxilPageIntersheetEdit_jobInsertBlink.showEnable( m_b_BlinkOn );
			Key_AuxilPageIntersheetEdit_jobPage.showEnable( True );
			Key_AuxilPageIntersheetEdit_jobAdd.activate( False );
		}
	}
//comment 2018/5/18 SCD2-Sky肥後 [DenebMLK][機器構成変更No.7]5段目トレイ追加対応.
	PNLS_setOriginalSizeCLCD2Key s_parts;

	s_parts.ps_KeyOriginalSize  =    &Key_AuxilPageIntersheetEdit_jobInsertSheet;	// キー.
	s_parts.ps_TxtOriginalSize01 =   &Ktx_AuxilPageIntersheetEdit_jobInsertSheet2;	// １行目 用紙コード表示.
	s_parts.ps_TxtOriginalSize02 =   NULL;										// ２行目 自動表示.
	s_parts.ps_TxtOriginalSizeX =    &Key_AuxilPageIntersheetEdit_jobInsertSheetX;	// 用紙サイズ表示 X.
	s_parts.ps_TxtOriginalSizeY =    &Key_AuxilPageIntersheetEdit_jobInsertSheetY;	// 用紙サイズ表示 Y.
	s_parts.ps_NumOriginalSizeX =    &Num_AuxilPageIntersheetEdit_jobInsertSheetX;	// 用紙サイズ数値表示 X.
	s_parts.ps_NumOriginalSizeY =    &Num_AuxilPageIntersheetEdit_jobInsertSheetY;	// 用紙サイズ数値表示 Y.
	s_parts.ps_TrayTxtPaperSize =    &Ktx_AuxilPageIntersheetEdit_jobInsertSheet1;
	s_parts.ps_TrayIconPaperSize =   &Gra_AuxilPageIntersheetEdit_jobInsertSheet_Tray1;
	s_parts.ps_RemainIconPaperSize = NULL;
	s_parts.ps_TypeIconPaperSize =   NULL;

	// インターシートトレイに関するの表示.
	PnlComsetPaperSizeCLCD2Grap( &s_parts, e_FeedTray, PNLE_PaperSizeShowKind_Type1 );
		
	if( PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_InsertSheet ) ) {		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
		if( PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_InsertSheet_PaperSetting ) == False ) {		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
			Key_AuxilPageIntersheetEdit_jobInsertSheet.showEnable( False );	// 挿入紙キー.
		}
	} else {
		Key_AuxilPageIntersheetEdit_jobInsertSheet.showEnable( False );		// 挿入紙キー.
	}
	// [OK] キーと [スタート] ボタンは連動させる.
	if( False == Key_AuxilPageIntersheetEdit_jobOK.isActivating() ) {
		//Falseの時だけ設定し青は基底処理におまかせ.
		printLEDControl( False );
	}
	sendSlide10KeyData();

}

//------------------------------------------------------------------------------
/**
* @brief A061152300 インターシート : 画面終了処理.
*/
//------------------------------------------------------------------------------
void PCPC_AuxilPageIntersheetEdit_job::endGamen()
{
	// バックアップデータがある場合.
	if( m_ss_JobSpecBackupTableNo != -1 ) {

		// バックアップしたJobSpecをクリアする.
		PnlComGetCopyDataLib()->JobSpecBackupClear( m_ss_JobSpecBackupTableNo );

		m_ss_JobSpecBackupTableNo = -1;
	}

	// 親クラスの処理をコール.
	PCPC_AuxilPageIntersheet_job_com::endGamen();
}

//------------------------------------------------------------------------------
/**
* @brief A061152300 インターシート : ボタン（押された).
*/
//------------------------------------------------------------------------------
void PCPC_AuxilPageIntersheetEdit_job::procButtonOn( uchar option )
{

	short ss_SrcPosX,ss_SrcPosY;	// 表示位置.
	short ss_SrcSizeX,ss_SrcSizeY;	// 表示サイズ.

// ボタンID重複のためスクロールバーのメッセージを別処理にしました.
	if( True == isMessageActive( &Cmp_AuxilPageIntersheetEdit_jobScrollBar ) ) {
		// ボタンID重複対応.
		switch( option ) {
			// ページ切替えキー.
			case PNLE_ScrollBarKeyType_Dec:
			case PNLE_ScrollBarKeyType_Inc:
				break;
			default:
				break;
		}
		setupGamen();
		return;
	}
	if( PNLE_AuxilPageIntersheetEdit_jobInsertSheet == option ) {

		Key_AuxilPageIntersheetEdit_jobInsertSheet.getPosition( ss_SrcPosX,ss_SrcPosY );
		Key_AuxilPageIntersheetEdit_jobInsertSheet.getSize( ss_SrcSizeX,ss_SrcSizeY );
		PNLC_PopupListControl::newInstance()->addPopup( &gp_A061152310, this, NULL, ss_SrcPosX, ss_SrcPosY, ss_SrcSizeX, ss_SrcSizeY );
		return;

	} else {

		switch( option ) {
			case PNLE_AuxilPageIntersheetEdit_jobEditNum01:	setPageSelectStatus( PNLE_AuxilPageIntersheetEdit_job_comSelectPageNumber01 ); break;
			case PNLE_AuxilPageIntersheetEdit_jobEditNum02:	setPageSelectStatus( PNLE_AuxilPageIntersheetEdit_job_comSelectPageNumber02 ); break;
			case PNLE_AuxilPageIntersheetEdit_jobEditNum03:	setPageSelectStatus( PNLE_AuxilPageIntersheetEdit_job_comSelectPageNumber03 ); break;
			case PNLE_AuxilPageIntersheetEdit_jobEditNum04:	setPageSelectStatus( PNLE_AuxilPageIntersheetEdit_job_comSelectPageNumber04 ); break;
			case PNLE_AuxilPageIntersheetEdit_jobEditNum05:	setPageSelectStatus( PNLE_AuxilPageIntersheetEdit_job_comSelectPageNumber05 ); break;
			case PNLE_AuxilPageIntersheetEdit_jobEditNum06:	setPageSelectStatus( PNLE_AuxilPageIntersheetEdit_job_comSelectPageNumber06 ); break;
			case PNLE_AuxilPageIntersheetEdit_jobEditNum07:	setPageSelectStatus( PNLE_AuxilPageIntersheetEdit_job_comSelectPageNumber07 ); break;
			case PNLE_AuxilPageIntersheetEdit_jobEditNum08:	setPageSelectStatus( PNLE_AuxilPageIntersheetEdit_job_comSelectPageNumber08 ); break;
			case PNLE_AuxilPageIntersheetEdit_jobEditNum09:	setPageSelectStatus( PNLE_AuxilPageIntersheetEdit_job_comSelectPageNumber09 ); break;
			case PNLE_AuxilPageIntersheetEdit_jobEditNum10:	setPageSelectStatus( PNLE_AuxilPageIntersheetEdit_job_comSelectPageNumber10 ); break;
			case PNLE_AuxilPageIntersheetEdit_jobEditNum11:	setPageSelectStatus( PNLE_AuxilPageIntersheetEdit_job_comSelectPageNumber11 ); break;
			case PNLE_AuxilPageIntersheetEdit_jobEditNum12:	setPageSelectStatus( PNLE_AuxilPageIntersheetEdit_job_comSelectPageNumber12 ); break;
			case PNLE_AuxilPageIntersheetEdit_jobEditNum13:	setPageSelectStatus( PNLE_AuxilPageIntersheetEdit_job_comSelectPageNumber13 ); break;
			case PNLE_AuxilPageIntersheetEdit_jobEditNum14:	setPageSelectStatus( PNLE_AuxilPageIntersheetEdit_job_comSelectPageNumber14 ); break;
			case PNLE_AuxilPageIntersheetEdit_jobEditNum15:	setPageSelectStatus( PNLE_AuxilPageIntersheetEdit_job_comSelectPageNumber15 ); break;
			// インターシートページ番号の追加.
			case PNLE_AuxilPageIntersheetEdit_jobAdd:
				if( checkPageNumber() == True ) {

					Key_AuxilPageIntersheetEdit_jobAdd.getPosition( ss_SrcPosX,ss_SrcPosY );
					Key_AuxilPageIntersheetEdit_jobAdd.getSize( ss_SrcSizeX,ss_SrcSizeY );
					PNLC_PopupListControl::newInstance()->addPopup( &gp_InsertDuplicateDesignation_warn, this, NULL, ss_SrcPosX, ss_SrcPosY, ss_SrcSizeX, ss_SrcSizeY );
					return;

				} else {
					// インターシートページ番号の更新.
					renewPageNumber();

					// インターシートページ番号のソート.
					procSortExec();

					// インターシートページ番号を設定.
					setIntersheetPageNumber();

					// インターシートページ登録フラグを有効にする.
					m_b_RegistrationFlg = True;
				}
				break;
			// インターシートページ番号の削除.
			case PNLE_AuxilPageIntersheetEdit_jobDelete:
				// スライダ移動.
				Cmp_AuxilPageIntersheetEdit_jobScrollBar.moveSliderPage( getNowPage() );

				// インターシートページ番号の削除.
				deletePageNumber();

				// インターシートページ番号のソート.
				procSortExec();

				// インターシートページ番号を設定.
				setIntersheetPageNumber();
				break;
			// インターシート用紙挿入モードの設定.
			case PNLE_AuxilPageIntersheetEdit_jobDoCopy:
				setIntersheetMode( TYPE_PaperInsertionWithImage );
				break;
			// インターシート用紙挿入モードの設定.
			case PNLE_AuxilPageIntersheetEdit_jobNotDoCopy:
				setIntersheetMode( TYPE_PaperInsertionWithoutImage );
				break;
			// ON選択.
			case PNLE_PopupMessageAreaButtonType_On:
				if( ( True == combinationCheck_AccumuDoc( CNK_MiddleMode, PnlComGetJBJobSpec(), PnlComGetJobCopy() ) )
				    && ( True == combinationCheck( CNK_MiddleMode, PnlComGetJobCopy() ) ) ) {
					// ON状態を設定する.
					setOn();
				}
				else {
					// 組合せ禁止のため警告画面を表示し、[OFF] ボタンを選択状態にする.
					Cmp_AuxilPageIntersheetEdit_jobMessage.setOnOffSwitchButtonState( False );
				}
				break;
			// OFF選択.
			case PNLE_PopupMessageAreaButtonType_Off:
				setOff();
				break;
			case PNLE_AuxilPageIntersheetEdit_jobNumericInputArea:
				m_bl_isSelectedInputField = True;	// comment 2012/06/21 OPS1川手 [MinervaBK][CQ366316]ソフト10キーUD対応 No.1 [数値入力エリアの選択状態更新].
				// テンキーポップアップ表示.
				show10KeyPopup();
				break;
			default: LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return;
		}
	}

	setupGamen();

}

//------------------------------------------------------------------------------
/**
* @brief A061152300 インターシート : ON状態を設定する.
*/
//------------------------------------------------------------------------------
void PCPC_AuxilPageIntersheetEdit_job::setOn()
{
//comment 2018/5/18 SCD2-Sky肥後 [DenebMLK][機器構成変更No.7]5段目トレイ追加対応.
//	static const struct TYPS_CheckFeedTrayTable_CLCD {
//		TYPE_FeedTray				e_FeedTray;				// 給紙口.
//		TYPE_TemporaryRescueUnit	e_TemporaryRescueUnit;	// 一時切り離し対象.
//	} s_checkFeedTrayTable_CLCD[] = {
//		{ TYPE_FeedTrayManual, TYPE_TemporaryRescueUnitTrayManual }, // 手差し.
//		{ TYPE_FeedTray1,  TYPE_TemporaryRescueUnitTray1    }, // トレイ1.
//		{ TYPE_FeedTray2,  TYPE_TemporaryRescueUnitTray2    }, // トレイ2.
//		{ TYPE_FeedTray3,  TYPE_TemporaryRescueUnitTray3    }, // トレイ3.
//		{ TYPE_FeedTray4,  TYPE_TemporaryRescueUnitTray4    }, // トレイ4.
//		{ TYPE_FeedTray5,  TYPE_TemporaryRescueUnitTrayLCT  }, // LCT.
//	};


	// ループカウンタ.
	uchar i = 0;

	// バックアップデータがある場合.
	if( m_ss_JobSpecBackupTableNo != -1 ) {

		// JobSpecを取得する.
		if( NULL == PnlComGetJobCopy() ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }
		TYPC_JobSpec* pc_JobSpec = PnlComGetJobCopy()->getJobSpec();

		// バックアップしたJobSpecに戻す
		PnlComGetCopyDataLib()->setJobSpecBackup( m_ss_JobSpecBackupTableNo , pc_JobSpec );

		// バックアップしたJobSpecをクリアする.
		PnlComGetCopyDataLib()->JobSpecBackupClear( m_ss_JobSpecBackupTableNo );

		m_ss_JobSpecBackupTableNo = -1;
	} else {

		// インターシートの取得.
		TYPC_Print* pc_Print = PnlComGetJobCopy()->getJobSpec()->pc_Print;
		TYPC_IntersheetMode c_IntersheetMode = pc_Print->c_Intersheet.getMiddleMode();

		if( TYPE_PaperInsertionOff == c_IntersheetMode.getPaperInsertion() ) {
//comment 2018/5/18 SCD2-Sky肥後 [DenebMLK][機器構成変更No.7]5段目トレイ追加対応.
//			APIC_OperationSetting* pc_OperationSetting = PnlComGetOperationSetting();
			// デフォルトの用紙挿入モードは「コピーする」.
			c_IntersheetMode.setPaperInsertion( TYPE_PaperInsertionWithImage );
//comment 2018/5/18 SCD2-Sky肥後 [DenebMLK][機器構成変更No.7]5段目トレイ追加対応.
//			// デフォルトはユーティリティの設定から取得.
//			TYPE_FeedTray e_FeedTray = pc_OperationSetting->getInsertFeedTray();
//
//			if( ppc_FeedTray->getTrayType( e_FeedTray ) == TYPE_TrayTypeNon ||
//			    ppc_SystemStatus->getTemporaryRescueUserStatus( s_checkFeedTrayTable_CLCD[e_FeedTray].e_TemporaryRescueUnit ) == TYPE_TemporaryRescueStatusSet ) {
//				uchar uc_CheckFeedTraySize = ( uchar )( sizeof( s_checkFeedTrayTable_CLCD )/sizeof( s_checkFeedTrayTable_CLCD ) );
//				for( i = 0; i < uc_CheckFeedTraySize; i++ ) {
//
//					if( ppc_FeedTray->getTrayType( s_checkFeedTrayTable_CLCD[i].e_FeedTray ) != TYPE_TrayTypeNon &&
//					    ppc_SystemStatus->getTemporaryRescueUserStatus( s_checkFeedTrayTable_CLCD[i].e_TemporaryRescueUnit ) == TYPE_TemporaryRescueStatusReset ) {
//
//						// 選択トレイを設定.
//						e_FeedTray = s_checkFeedTrayTable_CLCD[i].e_FeedTray;
//						break;
//					}
//				}
//			}
			TYPE_FeedTray e_FeedTray = getIntersheetFeedTray();
			for( i = 0;  i < PNLD_InsertPageNumber_Max; i++ ) {
				// インターシートの選択トレイを設定.
				pc_Print->c_Intersheet.setFeedTrayMiddle( i, e_FeedTray );
			}

			// インターシートの選択トレイを設定.
			c_IntersheetMode.setFeedTray( e_FeedTray );
			pc_Print->c_Intersheet.setMiddleMode( c_IntersheetMode );
		}
	}
}

//------------------------------------------------------------------------------
/**
* @brief A061152300 インターシート : OFF状態を設定する.
*/
//------------------------------------------------------------------------------
void PCPC_AuxilPageIntersheetEdit_job::setOff()
{
	// JobSpecのデータを保存する.
	if( NULL == PnlComGetJobCopy() ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }
	TYPC_JobSpec* pc_JobSpec = PnlComGetJobCopy()->getJobSpec();

	// JobSpecのデータを保存する.
	m_ss_JobSpecBackupTableNo = PnlComGetCopyDataLib()->saveJobSpecBackup( m_ss_JobSpecBackupTableNo, pc_JobSpec );

	// インターシートの取消し.
	cancel( CNK_MiddleMode, CNK_Size, PnlComGetJobCopy() );

	if( False == PnlComIsHard10Key() ) {
		m_bl_isSelectedInputField = False;
	}
	// MTUIテンキー非表示.
	hide10KeyPopup();
}

//------------------------------------------------------------------------------
/**
* @brief A061152300 インターシート : クリアー.
*/
//------------------------------------------------------------------------------
void PCPC_AuxilPageIntersheetEdit_job::procClear( uchar option )
{

	if( False == isOn( CNK_MiddleMode, PnlComGetJobCopy() ) ) {

		// 無効音を鳴らす.
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;

	}

	PCPC_AuxilPageIntersheet_job_com::procClear( option );

	setupGamen();

}

//------------------------------------------------------------------------------
/**
* @brief A061152300 インターシート : テンキー.
*/
//------------------------------------------------------------------------------
void PCPC_AuxilPageIntersheetEdit_job::proc10Key( uchar option )
{
	uchar uc_Count = 0;
	if( False == isOn( CNK_MiddleMode, PnlComGetJobCopy() ) ) {

		// 無効音を鳴らす.
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;

	}
	uc_Count = getPageNumberCount();
	if( uc_Count >= 30 ) {
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}


	PCPC_AuxilPageIntersheet_job_com::proc10Key( option );

	setupGamen();

}

//------------------------------------------------------------------------------
/**
* @brief A061152300 インターシート : スタートキー.
*/
//------------------------------------------------------------------------------
void PCPC_AuxilPageIntersheetEdit_job::procStart( uchar option )
{
	PNLC_OutSideKeyCallDataLib::newInstance()->setSelectNo( PNLE_Soft10KeyUnselected );	// データクリア.
	if( NULL == PnlComGetJobCopy() ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }
	TYPC_Print* pc_Print = PnlComGetJobCopy()->getJobSpec()->pc_Print;

	if( ( isOn( CNK_MiddleMode, PnlComGetJobCopy() ) == True ) &&
	    ( pc_Print->c_Intersheet.getCount() == 0 ) ) {

		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}

	PCPC_AuxilPageIntersheet_job_com::procStart( option );

}

//------------------------------------------------------------------------------
/**
* @brief A061152300 インターシート : ユーティリティキー.
*/
//------------------------------------------------------------------------------
void PCPC_AuxilPageIntersheetEdit_job::procUtility( uchar option )
{
	PNLC_OutSideKeyCallDataLib::newInstance()->setSelectNo( PNLE_Soft10KeyUnselected );	// データクリア.
	if( NULL == PnlComGetJobCopy() ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }
	TYPC_Print* pc_Print = PnlComGetJobCopy()->getJobSpec()->pc_Print;
	// USBアクセス(印刷、保存)中は受け付け禁止.
	APIC_UsbMemDeviceInfo* pc_USBMemoryInfo = APIC_UsbMemDeviceInfo::newInstance();
	if( NULL == pc_USBMemoryInfo ) {
		PnlComPutDebugLog( False, UC_PNL_LOGPATTERN_1, "PCPC_AuxilPageIntersheetEdit_job::procUtility", UC_PNL_ERROR_IMPOSSIBLE );
		return;
	}
	if( ( True == pc_USBMemoryInfo->isAccess( TYPE_UsbMemAccessRead, TYPE_UsbMemAppIdStorageAccessApp ) ) ||
	    ( True == pc_USBMemoryInfo->isAccess( TYPE_UsbMemAccessWrite, TYPE_UsbMemAppIdNetTxProcess ) ) ) {
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}
	if( ( isOn( CNK_MiddleMode, PnlComGetJobCopy() ) == True ) &&
	    ( pc_Print->c_Intersheet.getCount() == 0 ) ) {

		// エラー状態セット.
		onlyPanel.setErrorNumberFieldPartsKind( EP_ErrorPartsKind_NumberField );
		m_b_ErrorOn = True;
		setupGamen();
		return;
	}

	PCPC_AuxilPageIntersheet_job_com::procUtility( option );

}

//------------------------------------------------------------------------------
/**
* @brief A061152300 インターシート : リセットキー押下.
*/
//------------------------------------------------------------------------------
void PCPC_AuxilPageIntersheetEdit_job::procReset( uchar option )
{
	PCPC_AuxilPageIntersheet_job_com::procReset( option );
	if( NULL == PnlComGetJobCopy() ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }
}

//------------------------------------------------------------------------------
/**
* @brief A061152300 インターシート : インターシートページ番号の取得.
*/
//------------------------------------------------------------------------------
void PCPC_AuxilPageIntersheetEdit_job::getIntersheetPageNumber()
{
	// Print情報の取得.
	if( NULL == PnlComGetJobCopy() ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }
	TYPC_Print* pc_Print = PnlComGetJobCopy()->getJobSpec()->pc_Print;

	// 差込ページ番号の取得.
	short ss_AryPageNumber[PNLD_IntersheetPageNumber_Max];		// 配列の要素数 99個.
	short ss_IntersheetPageNumber[PNLD_InsertPageNumber_Max];	// 配列の要素数 30個.
	pc_Print->c_Intersheet.getPages( &( ss_AryPageNumber[0] ) );


	for( uchar i = 0; i < PNLD_InsertPageNumber_Max; i++ ) {
		ss_IntersheetPageNumber[i] = ss_AryPageNumber[i];
	}

	// ページ番号の設定.
	setPageNumber( &( ss_IntersheetPageNumber[0] ) );

}

//------------------------------------------------------------------------------
/**
* @brief A061152300 インターシート : インターシートページ番号の設定.
*/
//------------------------------------------------------------------------------
void PCPC_AuxilPageIntersheetEdit_job::setIntersheetPageNumber()
{
	// Print情報取得.
	if( NULL == PnlComGetJobCopy() ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }
	TYPC_Print* pc_Print = PnlComGetJobCopy()->getJobSpec()->pc_Print;

	// 差込ページ番号の取得.
	short ss_AryPageNumber[PNLD_IntersheetPageNumber_Max];		// 配列の要素数 99個.
	short ss_IntersheetPageNumber[PNLD_InsertPageNumber_Max];	// 配列の要素数 30個.
	getPageNumberSetting( &( ss_IntersheetPageNumber[0] ) );

	memset( ss_AryPageNumber, 0, sizeof( ss_AryPageNumber ) );

	for( uchar i = 0; i < PNLD_InsertPageNumber_Max; i++ ) {
		ss_AryPageNumber[i] = ss_IntersheetPageNumber[i];
	}

	pc_Print->c_Intersheet.setPages( &( ss_AryPageNumber[0] ) );
	pc_Print->c_Intersheet.setCount( getPageNumberCount() );

}

//------------------------------------------------------------------------------
/**
* @brief A061152300 インターシート : インターシート用紙挿入モードの設定.
*/
//------------------------------------------------------------------------------
void PCPC_AuxilPageIntersheetEdit_job::setIntersheetMode( const TYPE_PaperInsertion e_PaperInsertion )
{
	if( NULL == PnlComGetJobCopy() ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }
	TYPC_Print* pc_Print = PnlComGetJobCopy()->getJobSpec()->pc_Print;
	// インターシート用紙挿入モードの設定.
	TYPC_IntersheetMode c_IntersheetMode = pc_Print->c_Intersheet.getMiddleMode();
	c_IntersheetMode.setPaperInsertion( e_PaperInsertion );
	// 「コピーする」が選択されたときトレイがフィニッシャーのPIトレイならば内蔵トレイ1に変更
	if( e_PaperInsertion == TYPE_PaperInsertionWithImage ) {
		// インターシートの選択トレイを取得.
		TYPE_FeedTray e_FeedTray = c_IntersheetMode.getFeedTray();
		// PIトレイが選択されている場合は自動的にトレイ1を選択する.
		if( ( TYPE_FeedTrayPI1 == e_FeedTray ) || ( TYPE_FeedTrayPI2 == e_FeedTray ) ) {
			// システム側からの処理指定に基づくforループ
			// - 各ページの設定全てに反映させておく
			for( uchar i = 0; PNLD_InsertPageNumber_Max > i; ++i ) {
				// インターシートの選択トレイを設定.
				pc_Print->c_Intersheet.setFeedTrayMiddle( i, TYPE_FeedTray1 );
			}
			c_IntersheetMode.setFeedTray( TYPE_FeedTray1 );
		}
	}
	pc_Print->c_Intersheet.setMiddleMode( c_IntersheetMode );
}

//------------------------------------------------------------------------------.
/**
 *	@brief	A061152300 インターシート : 割り込みモード
 */
//------------------------------------------------------------------------------.
void PCPC_AuxilPageIntersheetEdit_job::procIntMode( uchar option )
{
	PNLC_OutSideKeyCallDataLib::newInstance()->setSelectNo( PNLE_Soft10KeyUnselected );	// データクリア.
	PCPC_AuxilPageIntersheet_job_com::procIntMode( option );
}
//------------------------------------------------------------------------------.
/**
 *	@brief	A061152300 インターシート : 暗証番号入力
 */
//------------------------------------------------------------------------------.
void PCPC_AuxilPageIntersheetEdit_job::procIDInput( uchar option )
{
	PNLC_OutSideKeyCallDataLib::newInstance()->setSelectNo( PNLE_Soft10KeyUnselected );	// データクリア.
	PCPC_AuxilPageIntersheet_job_com::procIDInput( option );
}
//------------------------------------------------------------------------------.
/**
 *	@brief	A061152300 インターシート : コピー
 */
//------------------------------------------------------------------------------.
void PCPC_AuxilPageIntersheetEdit_job::procCopy( uchar option )
{
	// MFP基本機能使用設定が無効の場合.
	if( False == c_ExtPanel.isEnableMFPFunction() ) {
		// キーを受け付けない.
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}

	PNLC_OutSideKeyCallDataLib::newInstance()->setSelectNo( PNLE_Soft10KeyUnselected );	// データクリア.
	PCPC_AuxilPageIntersheet_job_com::procCopy( option );
}
//------------------------------------------------------------------------------.
/**
 *	@brief	A061152300 インターシート : 拡大表示
 */
//------------------------------------------------------------------------------.
void PCPC_AuxilPageIntersheetEdit_job::procEnlarge( uchar option )
{
	PNLC_OutSideKeyCallDataLib::newInstance()->setSelectNo( PNLE_Soft10KeyUnselected );	// データクリア.
	PCPC_AuxilPageIntersheet_job_com::procEnlarge( option );
}
//------------------------------------------------------------------------------.
/**
 *	@brief	A061152300 インターシート : 確認コピー
 */
//------------------------------------------------------------------------------.
void PCPC_AuxilPageIntersheetEdit_job::procSampleCopy( uchar option )
{
	PNLC_OutSideKeyCallDataLib::newInstance()->setSelectNo( PNLE_Soft10KeyUnselected );	// データクリア.
	PCPC_AuxilPageIntersheet_job_com::procSampleCopy( option );
}
//------------------------------------------------------------------------------.
/**
 *	@brief	A061152300 インターシート : ジョブメモリ
 */
//------------------------------------------------------------------------------.
void PCPC_AuxilPageIntersheetEdit_job::procJobMemory( uchar option )
{
	PNLC_OutSideKeyCallDataLib::newInstance()->setSelectNo( PNLE_Soft10KeyUnselected );	// データクリア.
	PCPC_AuxilPageIntersheet_job_com::procJobMemory( option );
}
//------------------------------------------------------------------------------.
/**
 *	@brief	A061152300 インターシート : FAX
 */
//------------------------------------------------------------------------------.
void PCPC_AuxilPageIntersheetEdit_job::procFax( uchar option )
{
	// MFP基本機能使用設定が無効の場合.
	if( False == c_ExtPanel.isEnableMFPFunction() ) {
		// キーを受け付けない.
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}

	PNLC_OutSideKeyCallDataLib::newInstance()->setSelectNo( PNLE_Soft10KeyUnselected );	// データクリア.
	PCPC_AuxilPageIntersheet_job_com::procFax( option );
}
//------------------------------------------------------------------------------.
/**
 *	@brief	A061152300 インターシート : FAX
 */
//------------------------------------------------------------------------------.
void PCPC_AuxilPageIntersheetEdit_job::procFaxOnly( uchar option )
{
	// MFP基本機能使用設定が無効の場合.
	if( False == c_ExtPanel.isEnableMFPFunction() ) {
		// キーを受け付けない.
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}

	PNLC_OutSideKeyCallDataLib::newInstance()->setSelectNo( PNLE_Soft10KeyUnselected );	// データクリア.
	PCPC_AuxilPageIntersheet_job_com::procFaxOnly( option );
}

// [制御構成変更] ソフトテンキー外キー対応の為メソッド追加.
//------------------------------------------------------------------------------.
/**
 *	@brief	A061152300 インターシート : ボックス
 */
//------------------------------------------------------------------------------.
void PCPC_AuxilPageIntersheetEdit_job::procBox( uchar option )
{
	// MFP基本機能使用設定が無効の場合.
	if( False == c_ExtPanel.isEnableMFPFunction() ) {
		// キーを受け付けない.
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}

	PNLC_OutSideKeyCallDataLib::newInstance()->setSelectNo( PNLE_Soft10KeyUnselected );	// データクリア.
	PCPC_AuxilPageIntersheet_job_com::procBox( option );
}
//------------------------------------------------------------------------------.
/**
 *	@brief	A061152300 インターシート : ユビキタスプリント
 */
//------------------------------------------------------------------------------.
void PCPC_AuxilPageIntersheetEdit_job::procUbqPrint( uchar option )
{
	// MFP基本機能使用設定が無効の場合.
	if( False == c_ExtPanel.isEnableMFPFunction() ) {
		// キーを受け付けない.
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}

	PNLC_OutSideKeyCallDataLib::newInstance()->setSelectNo( PNLE_Soft10KeyUnselected );	// データクリア.
	PCPC_AuxilPageIntersheet_job_com::procUbqPrint( option );
}
//------------------------------------------------------------------------------.
/**
 *	@brief	A061152300 インターシート : パワーセーブ
 */
//------------------------------------------------------------------------------.
void PCPC_AuxilPageIntersheetEdit_job::procPowerSave( uchar option )
{
	PNLC_OutSideKeyCallDataLib::newInstance()->setSelectNo( PNLE_Soft10KeyUnselected );	// データクリア.
	PCPC_AuxilPageIntersheet_job_com::procPowerSave( option );
}
//------------------------------------------------------------------------------.
/**
 *	@brief	A061152300 インターシート : ヘルプ
 */
//------------------------------------------------------------------------------.
void PCPC_AuxilPageIntersheetEdit_job::procHelp( uchar option )
{
	PNLC_OutSideKeyCallDataLib::newInstance()->setSelectNo( PNLE_Soft10KeyUnselected );	// データクリア.
	PCPC_AuxilPageIntersheet_job_com::procHelp( option );
}
//------------------------------------------------------------------------------.
/**
 *	@brief	A061152300 インターシート : ユーザー補助
 */
//------------------------------------------------------------------------------.
void PCPC_AuxilPageIntersheetEdit_job::procAccessibility( uchar option )
{
	PNLC_OutSideKeyCallDataLib::newInstance()->setSelectNo( PNLE_Soft10KeyUnselected );	// データクリア.
	PCPC_AuxilPageIntersheet_job_com::procAccessibility( option );
}
//------------------------------------------------------------------------------.
/**
 *	@brief	A061152300 インターシート : IDキー処理
 */
//------------------------------------------------------------------------------.
void PCPC_AuxilPageIntersheetEdit_job::procIDInputVoice( uchar option )
{
	PNLC_OutSideKeyCallDataLib::newInstance()->setSelectNo( PNLE_Soft10KeyUnselected );	// データクリア.
	PCPC_AuxilPageIntersheet_job_com::procIDInputVoice( option );
}
//------------------------------------------------------------------------------.
/**
 *	@brief	A061152300 インターシート : ブラウザ
 */
//------------------------------------------------------------------------------.
void PCPC_AuxilPageIntersheetEdit_job::procBrowser( uchar option )
{
	// MFP基本機能使用設定が無効の場合.
	if( False == c_ExtPanel.isEnableMFPFunction() ) {
		// キーを受け付けない.
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}

	PNLC_OutSideKeyCallDataLib::newInstance()->setSelectNo( PNLE_Soft10KeyUnselected );	// データクリア.
	PCPC_AuxilPageIntersheet_job_com::procBrowser( option );
}
//------------------------------------------------------------------------------.
/**
* @brief A061152300 インターシート : ソフトテンキー呼出しキー.
*/
//------------------------------------------------------------------------------.
void PCPC_AuxilPageIntersheetEdit_job::procSoft10Key( uchar option )
{
	uchar uc_Count = 0;
	uc_Count = getPageNumberCount();

	if( ( False == isOn( CNK_MiddleMode, PnlComGetJobCopy() ) )
	    || ( 30 <= uc_Count )
	    || ( False == m_bl_isSelectedInputField ) ) {
		// 無効音を鳴らす.
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}

	// スライドレス、MTUI有効の場合.
	if( True == PLBC_NonSlideMultiTouchFunction::isEnable() ) {
		// ソフトテンキー表示(センター表示).
		PnlComMtuiIf_TenkeyPad::instance()->showDefaultPos();
	} else {
		if( PNLE_Soft10KeyUnselected != PNLC_OutSideKeyCallDataLib::newInstance()->getSelectNo() ) { return; }

		Bool bl_NonSetting = ( m_ss_InputPageNumber > 0 ) ? False : True;		// 未入力状態チェック.
		PNLC_OutSideKeyCallDataLib::newInstance()->setColorMode( PNLE_Soft10KeyColorModeCommon );
		PNLC_OutSideKeyCallDataLib::newInstance()->setSelectNo( PNLE_Soft10KeySelected );
		PNLC_OutSideKeyCallDataLib::newInstance()->setInputNumeric( m_ss_InputPageNumber, False, 1, PCGD_MaxPageNumber, bl_NonSetting );
		PNLC_PopupListControl::newInstance()->addPopup( &gp_Soft10Key_PageLocation, this );
	}
}

//------------------------------------------------------------------------------.
/**
* @brief A061152300 インターシート : アクティブ状態の通知.
*/
//------------------------------------------------------------------------------.
void PCPC_AuxilPageIntersheetEdit_job::procActivate()
{
	checkSoft10KeyData();

	// 親クラスを呼び出し.
	PCPC_AuxilPageIntersheet_job_com::procActivate();
}

//------------------------------------------------------------------------------.
/**
 *	@brief	A061152300 インターシート : ソフトテンキーデータチェック.
 */
//------------------------------------------------------------------------------.
void PCPC_AuxilPageIntersheetEdit_job::checkSoft10KeyData()
{
	char sc_No = PNLC_OutSideKeyCallDataLib::newInstance()->getSelectNo();
	if( PNLE_Soft10KeyUnselected != sc_No ) {
		float f_Num;
		// 入力値を取得して更新.
		Bool bl_NonSetting = PNLC_OutSideKeyCallDataLib::newInstance()->getInputNumeric( &f_Num );
		if( False == bl_NonSetting ) {
			m_ss_InputPageNumber = ( short )f_Num;
		}
		// データのクリア.
		PNLC_OutSideKeyCallDataLib::newInstance()->setSelectNo( PNLE_Soft10KeyUnselected );
	} else {
		// 0時は、ソフトテンキーは無効.
	}
}

/**
* @brief A061152300 インターシート : PKIカード状態変化.
*/
//------------------------------------------------------------------------------.
void PCPC_AuxilPageIntersheetEdit_job::procPkiCardStatusChange( uchar option )
{
	if( option != PDPD_CardStatus_Inserted ) {											// PKIカード挿入ではない場合.
		PCPC_AuxilPageIntersheet_job_com::procPkiCardStatusChange( option );
	}
}

/**
 * @brief	MTUIテンキーポップアップ表示処理.
 * @param	.
 */
void PCPC_AuxilPageIntersheetEdit_job::show10KeyPopup()
{
	if( True != PLBC_NonSlideMultiTouchFunction::isEnable() ) {
		if( True == PLBC_FuncVersionCheck::isOK_Type4() ) {
			procSoft10Key( 0 );
		} else {
			setBuzzerType( PCGE_BuzzerType_Off, True );		// ブザー鳴らさない.
		}
	} else {
		if( False == isOn( CNK_MiddleMode, PnlComGetJobCopy() )
		    || ( 30 <= getPageNumberCount() ) ) {
			setBuzzerType( PCGE_BuzzerType_InputNg );		// 報知音設定.
		} else {
			// MTUIテンキー表示.
			const short PNLD_TenkeyPad_correctX = 10;
			const short PNLD_TenkeyPad_correctY = 64;

			short ss_SizeX, ss_SizeY, ss_KeyPosX, ss_KeyPosY, ss_PosX, ss_PosY;
			Key_AuxilPageIntersheetEdit_jobAdd.getPosition( ss_KeyPosX, ss_KeyPosY );
			Key_AuxilPageIntersheetEdit_jobAdd.getSize( ss_SizeX, ss_SizeY );
			this->getPosition( &ss_PosX, &ss_PosY );
			// 表示位置（画面表示位置＋追加キー表示位置＋キー幅＋補正値：画面表示位置＋メッセージエリア表示位置＋幅）
			PnlComMtuiIf_TenkeyPad::instance()->show( static_cast<short>( ss_KeyPosX+ss_SizeX+PNLD_TenkeyPad_correctX ), static_cast<short>( ss_PosY+PNLD_TenkeyPad_correctY ) );
		}
	}
}

/**
 * @brief	MTUIテンキーポップアップ非表示処理.
 * @param	.
 */
void PCPC_AuxilPageIntersheetEdit_job::hide10KeyPopup()
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
void PCPC_AuxilPageIntersheetEdit_job::procSlideMenu( uchar option )
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
Bool PCPC_AuxilPageIntersheetEdit_job::IsShowSlide10Key()
{
	uchar uc_Count = 0;
	uc_Count = getPageNumberCount();

	if( ( False == isOn( CNK_MiddleMode, PnlComGetJobCopy() ) )
	    ||	( 30 <= uc_Count )
	    ||	( False == m_bl_isSelectedInputField ) ) {
		return False;
	}

	return True;
}

// ===============================================================================================.
//	概要	: スライドメニュー入力エリアデータ送信.
//  引数    : True：初期化あり、False：初期化なし.
//	戻り値	: なし.
// ===============================================================================================.
void PCPC_AuxilPageIntersheetEdit_job::sendSlide10KeyData( Bool bl_Flag )
{
	uchar puc_Dst[PNLD_SLIDEMENU_TEXTBUFFER_SIZE];				//整数部(/小数)変換後格納先.
	memset( puc_Dst,0,sizeof( puc_Dst ) );

	if( True == bl_Flag ) {
		//スライドメニューOpen時の処理.
		PNLC_SoftSlideMenuTenKeyTextDispLib::newInstance()->create( PNLE_OutsideKeyCall_RIGHT_D_PAGELOCATION );
		PNLC_SoftSlideMenuTenKeyConvLib::newInstance()->setParam( PnlComGetUnitSystem(), "%3d", NULL );
	}

	//エラーチェックはなし.
	if( 0 != m_ss_InputPageNumber ) {
		PNLC_SoftSlideMenuTenKeyConvLib::newInstance()->getSetText( puc_Dst,( int32_t )m_ss_InputPageNumber,True,False );
	}
	//インサート固定.
	PNLC_SoftSlideMenuTenKeyTextDispLib::newInstance()->update( puc_Dst,						//文字列.
	        PNLD_CURSOR_TYPE_INSERT,	//カーソルタイプ.
	        ( short )strlen( ( char* )puc_Dst ),		//カーソルポジション.
	        True );						//ブリンク.
}

//comment 2018/5/18 SCD2-Sky肥後 [DenebMLK][機器構成変更No.7]5段目トレイ追加対応.
// ===============================================================================================.
//	［クラス］	A061152300 インターシート .
//	概要	：給紙口を取得.
//	戻り値	：void.
//	備考	：.
// ===============================================================================================.
TYPE_FeedTray PCPC_AuxilPageIntersheetEdit_job::getIntersheetFeedTray()
{
	static const struct TYPS_CheckFeedTrayTable_CLCD {
		TYPE_FeedTray				e_FeedTray;				// 給紙口.
		TYPE_TemporaryRescueUnit	e_TemporaryRescueUnit;	// 一時切り離し対象.
	} s_checkFeedTrayTable_CLCD[] = {
		{ TYPE_FeedTrayManual, TYPE_TemporaryRescueUnitTrayManual }, // 手差し.
		{ TYPE_FeedTray1,  TYPE_TemporaryRescueUnitTray1    }, // トレイ1.
		{ TYPE_FeedTray2,  TYPE_TemporaryRescueUnitTray2    }, // トレイ2.
		{ TYPE_FeedTray3,  TYPE_TemporaryRescueUnitTray3    }, // トレイ3.
		{ TYPE_FeedTray4,  TYPE_TemporaryRescueUnitTray4    }, // トレイ4.
// comment Sky森本（弘） 2018/05/18 [DenebMLK][機器構成変更_No.7] 5段増設プリント機能追加対応．
#ifdef DEF_DenebMLKCus
		{ TYPE_FeedTray5,  TYPE_TemporaryRescueUnitTray5  }, // トレイ5.
#else
		{ TYPE_FeedTray5,  TYPE_TemporaryRescueUnitTrayLCT  }, // LCT.
#endif	// DEF_DenebMLKCus
	};
	// インターシートの取得.
// comment 2018/11/16 [HCL_Nandhini] [DenebMLK] [AR1059379] - Warning removal Fix.
//	TYPC_Print* pc_Print = PnlComGetJobCopy()->getJobSpec()->pc_Print;

	// デフォルトはユーティリティの設定から取得.
	TYPE_FeedTray e_FeedTray = PnlComGetOperationSetting()->getInsertFeedTray();

	if( ppc_FeedTray->getTrayType( e_FeedTray ) == TYPE_TrayTypeNon ||
	    ppc_SystemStatus->getTemporaryRescueUserStatus( s_checkFeedTrayTable_CLCD[e_FeedTray].e_TemporaryRescueUnit ) == TYPE_TemporaryRescueStatusSet ) {
		uchar uc_CheckFeedTraySize = static_cast<uchar>( sizeof( s_checkFeedTrayTable_CLCD )/sizeof( s_checkFeedTrayTable_CLCD[0] ) );
		for(ushort i = 0; i < uc_CheckFeedTraySize; i++ ) {
			if( ppc_FeedTray->getTrayType( s_checkFeedTrayTable_CLCD[i].e_FeedTray ) != TYPE_TrayTypeNon &&
			    ppc_SystemStatus->getTemporaryRescueUserStatus( s_checkFeedTrayTable_CLCD[i].e_TemporaryRescueUnit ) == TYPE_TemporaryRescueStatusReset ) {
				// 選択トレイを設定.
				e_FeedTray = s_checkFeedTrayTable_CLCD[i].e_FeedTray;
				break;
			}
		}
	}
	return e_FeedTray;
}

// ===============================================================================================.
//	［クラス］	PCPC_CopyAuxiliaryBooklet_job B061152400 小冊子 .
//	概要	：コールバック関数.
//	戻り値	：void.
//	備考	：.
// ===============================================================================================.
void PCPC_CopyAuxiliaryBooklet_job::ans_getBoxPrintBookletMag( TYPC_Point<float> c_Mag, Bool bl_result )
{
	if( True == bl_result ) {
		CallBackIndi( TYPE_PNL_CallBack_GetBookletMagSuccess );
		APIC_Job* pc_Job = PnlComGetJobCopy();
		if( NULL == pc_Job ) {
			LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, ( char* )"Assert Failed!" );
			return;
		}
		TYPC_Common* pc_Common = pc_Job->getJobSpec()->pc_Common;
		pc_Common->c_Mag.setAmsOn( False );
		pc_Common->c_Mag.setMagType( TYPE_MagTypeUnselect );
		pc_Common->c_Mag.setMag( c_Mag );					// 倍率値の設定.
	} else {
		CallBackIndi( TYPE_PNL_CallBack_GetBookletMagFail );
	}
}
// ===============================================================================================.
//	［クラス］	PCPC_CopyAuxiliaryBooklet_job B061152400 小冊子.
//	概要	：コールバック発生イベント.
//	戻り値	：void.
//	備考	：.
// ===============================================================================================.
void PCPC_CopyAuxiliaryBooklet_job::procCallBack( TYPE_PNL_CallBack option )
{
	switch( option ) {
		case TYPE_PNL_CallBack_GetBookletMagSuccess:
		{
			pc_ManageIGCWaitDisplay.setDisplayFLGOff();		// 表示フラグＯＦＦ.
			break;
		}
		case TYPE_PNL_CallBack_GetBookletMagFail:
		{
			setBuzzerType( PCGE_BuzzerType_InputNg );		// 受付拒否音.
			PNLC_PopupListControl::newInstance()->closeTop( False , True );
			PNLC_PopupListControl::newInstance()->addPopup( &gp_B061152420, this, NULL, 400, 240 );
			pc_ManageIGCWaitDisplay.setDisplayFLGOff();		// 表示フラグＯＦＦ.
			break;
		}
		default:
			PPopupDecisionButtonCom::procCallBack( option );
			break;
	}
}
// ===============================================================================================.
//	［クラス］	PCPC_CopyAuxiliaryBooklet_job B061152400 小冊子 .
//	概要	：画面開始処理.
//	戻り値	：void.
//	備考	：.
// ===============================================================================================.
void PCPC_CopyAuxiliaryBooklet_job::startGamen()
{
	APIC_Job* pc_Job = PnlComGetJobCopy();
	if( NULL == pc_Job ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, ( char* )"Assert Failed!" );
		return;
	}
	PPopupJobSetupChangeCom::startGamen();		// 親クラスをコールする.

	ss_mBackUpJobTableNo = -1;					// バックアップジョブテーブルNo.初期化.
	// メッセージエリアコンポーネント初期設定.
	Cmp_BookletOnOffMessageArea_job.setMode( PNLE_PopupMessageAreaMessageModeAdmin );
	Cmp_BookletOnOffMessageArea_job.showDestinationCount( False );
	Cmp_BookletOnOffMessageArea_job.setNowPopupGamen( PNLE_JobBooklet );
	Cmp_BookletOnOffMessageArea_job.attachMessage( True );
	Cmp_BookletOnOffMessageArea_job.setOnOffButton( True );

	m_bl_bookletActivation = isOn( CNK_Magazine, pc_Job );

	if( True == m_bl_bookletActivation ) {
		Cmp_BookletOnOffMessageArea_job.setOnOffSwitchButtonState( True );
	} else {
		Cmp_BookletOnOffMessageArea_job.setOnOffSwitchButtonState( False );
	}

	onlyPanel.setFinishingProgram( False );		// 仕上りプログラムキーの状態設定OFF.

	Gra_BookletLeftDifference_job.showEnable( False );			// 小冊子_左開きアイコン.
	Gra_BookletRightDifference_job.showEnable( False );			// 小冊子_右開きアイコン.

	m_bl_bindDirection = False;									// 「左開き/右開き」のフラグ初期化 初期値「左開き」.

}
// ===============================================================================================.
//	［クラス］	PCPC_CopyAuxiliaryBooklet_job B061152400 小冊子 .
//	概要	：小冊子初期設定.
//	戻り値	：void.
//	備考	：.
// ===============================================================================================.
void PCPC_CopyAuxiliaryBooklet_job::setupAuxiliaryBooklet()
{
	APIC_Job* pc_Job = PnlComGetJobCopy();
	if( NULL == pc_Job ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, ( char* )"Assert Failed!" );
		return;
	}
	TYPC_Print* pc_Print = pc_Job->getJobSpec()->pc_Print;
	TYPC_Common* pc_Common = pc_Job->getJobSpec()->pc_Common;

	// 小冊子をOn.
	Cmp_BookletOnOffMessageArea_job.setOnOffSwitchButtonState( True );
	pc_Print->c_Nin1.setBookletOn( True );
	pc_Print->c_Nin1.setNin1( TYPE_Nin1_2in1 );
	pc_Print->setDuplexMode( TYPE_DuplexModeOn );

	combinationCheck( CNK_Magazine, pc_Job, True, False );

	ushort us_ChangeOutMode = pc_Common->getChangeOutMode();
	us_ChangeOutMode |= TYPE_ChangeOutMode_BoxMag;	// 倍率変更フラグをつける.
	pc_Common->setChangeOutMode( us_ChangeOutMode );

	if( True == combinationCheck_AccumuDoc( CNK_InterlockZoom, PnlComGetJBJobSpec(), pc_Job, False, False , CNK_Size, False, True )
	    && True == combinationCheck( CNK_InterlockZoom, pc_Job, False, False ) ) {
		if( False == isOn( CNK_APS, pc_Job ) ) {		// APSが設定されていない.
			JBLS_SetSelectJob* p_SelectJobAddr = NULL;
			p_SelectJobAddr = getJobListLib()->getSelectJobAddr(); //GetSelectJobAddr();
			if( NULL == p_SelectJobAddr
			    || NULL == p_SelectJobAddr->c_SelectDoc ) {
				LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
				return;
			}
			if( PnlComGetOperationSetting()->getEnableNin1Mag() ) {
				APIC_AutoSelect* c_AutoSelect = comPanelJob->getAutoSelect();
				if( NULL == c_AutoSelect ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }
// comment 2016/05/16 OPS1-Sky能多 [ZeusSBK][CQ530659] PanelLockError対策.
//				Bool bl_result = False;
//				bl_result = c_AutoSelect->req_getBoxPrintBookletMag( DOCD_ConfirmationPrintBox , p_SelectJobAddr->c_SelectDoc->docID ,ans_getBoxPrintBookletMag );
//				if( True == bl_result ) {
				ATSE_getBoxPrintBookletMagResult e_result = c_AutoSelect->req_getBoxPrintBookletMag( DOCD_ConfirmationPrintBox , p_SelectJobAddr->c_SelectDoc->docID ,ans_getBoxPrintBookletMag );
				if( ATSE_getBoxPrintBookletMagResult_OK == e_result ) {
					// 開始成功時は、お待ちください画面を表示する.
					pc_ManageIGCWaitDisplay.initManageIGCWaitDisplay( PCGD_IGCDisplayStandard, PCGE_IGCClearPopupDispFlgWait, this ) ;
					nextWaitDisplay_popup() ;
				} else {
					setBuzzerType( PCGE_BuzzerType_InputNg );		// 受付拒否音.
					PNLC_PopupListControl::newInstance()->addPopup( &gp_B061152420, this, NULL, 400, 240 );
				}
			}
		} else if( PnlComGetOperationSetting()->getEnableNin1Mag() ) {	// Nin1優先倍率が有効な場合.
			// お勧め倍率の設定.
			PnlComSetNin1DefaultMag( pc_Print, pc_Common );
		}
	}
	// とじ方向の取得.
	TYPE_BindDirection e_BindDirection = pc_Print->c_BindDirection.getBindDirection();
	if( TYPE_BindDirectionLeft != e_BindDirection && TYPE_BindDirectionRight != e_BindDirection ) {
		onlyPanel.setPanelBindDirection( pc_Job->getJobSpec(), TYPE_BindDirectionLeft );
	}
	// APS/AMS処理関数.
	selectAutoPaperMag();
}

//関数------------------------------------------------------------------------------------------------------------------
//概要    ：初期表示設定.
//備考    ：.
//戻値　　：無し.
//作成者　：.
//レビュー：.
//-----------------------------------------------------------------------------------------------
/**
 *	@brief A020151000 小冊子 : 初期表示設定.
 */
//-----------------------------------------------------------------------------------------------
// ===============================================================================================.
//	［クラス］	PCPC_CopyAuxiliaryBooklet_job B061152400 小冊子 表示セットアップ.
//	概要	：画面開始処理.
//	戻り値	：void.
//	備考	：.
// ===============================================================================================.
void PCPC_CopyAuxiliaryBooklet_job::setupGamen( short option )
{
	PPopupJobSetupChangeCom::setupGamen( option );		// 親クラスをコールする.
	APIC_Job* pc_Job = PnlComGetJobCopy();
	if( NULL == pc_Job ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, ( char* )"Assert Failed!" );
		return;
	}
	m_bl_bookletActivation = Cmp_BookletOnOffMessageArea_job.getOnOffSwitchButtonState();		// ON/OFF状態取得.

	TYPC_Print* pc_Print = pc_Job->getJobSpec()->pc_Print;
	// 現在の開き方向の値を取得.
	TYPE_BindDirection e_BindDirection = pc_Print->c_BindDirection.getBindDirection();
	if( TYPE_BindDirectionRight == e_BindDirection ) {
		e_BindDirection = TYPE_BindDirectionRight;		//右開き.
	} else {
		e_BindDirection = TYPE_BindDirectionLeft;		//左開き.
	}
	if( True == m_bl_bookletActivation ) {		// ON/OFFボタン判定.
		switch( e_BindDirection ) {
			case TYPE_BindDirectionLeft:		// 左開きの場合.
				Gra_BookletRightDifference_job.showEnable( False );
				Gra_BookletLeftDifference_job.showEnable( True );
				Gra_BookletLeftDifference_job.activate( True );
				m_bl_bindDirection = False;
				break;
			case TYPE_BindDirectionRight:		// 右開きの場合.
				Gra_BookletLeftDifference_job.showEnable( False );
				Gra_BookletRightDifference_job.showEnable( True );
				Gra_BookletRightDifference_job.activate( True );
				m_bl_bindDirection = True;
				break;
			default: break;
		}
	} else {
		if( m_bl_bindDirection ) {
			e_BindDirection = TYPE_BindDirectionRight;		//右開き.
		} else {
			e_BindDirection = TYPE_BindDirectionLeft;		//左開き.
		}

		switch( e_BindDirection ) {
			case TYPE_BindDirectionLeft:		// 左開きの場合.
				Gra_BookletLeftDifference_job.showEnable( True );
				Gra_BookletLeftDifference_job.activate( False );
				break;
			case TYPE_BindDirectionRight:		// 右開きの場合.
				Gra_BookletRightDifference_job.showEnable( True );
				Gra_BookletRightDifference_job.activate( False );
				break;
			default: break;
		}
	}
	if( TYPE_BindDirectionLeft == e_BindDirection ) {
		Rbt_BookletBindDirection_job.select( 0 );		// 左開きキー.
	} else {
		Rbt_BookletBindDirection_job.select( 1 );		// 右開きキー.
	}
	m_ss_bindSetting = 0;				// しない.
	if( isOn( CNK_Centerori, pc_Job ) ) {
		m_ss_bindSetting = 1;			// 中折り.
	} else if( isOn( CNK_CenterBinding, pc_Job ) ) {
		m_ss_bindSetting = 2;			// 中とじ.
	}
// A3A4FUM so_okada 2015.02.12 Start.
#ifndef DEF_A4_Cus
	Rbt_BookletBindSetting_job.select( m_ss_bindSetting );			// 紙折り/中とじ設定キー.

	// 活性化処理.
	Ktx_CopyFoldAndBind_job.activate( m_bl_bookletActivation );					// 紙折り/中とじタイトル.
	Rbt_BookletBindSetting_job.activate( m_bl_bookletActivation );					// 紙折り/中とじ設定キー.
#endif //DEF_A4_Cus.
// A3A4FUM so_okada 2015.02.12 End.

	Rbt_BookletBindDirection_job.activate( m_bl_bookletActivation );				// 左開き/右開きキー.
	Key_CopyAuxiliaryBookletCoverSheet_job.activate( m_bl_bookletActivation );		// カバーシートキー.

	// カバーシートを設定する.
	TYPC_IntersheetMode c_CoversheetFront = pc_Print->c_Intersheet.getFrontMode();
	TYPE_PaperInsertion e_PaperInsertion = c_CoversheetFront.getPaperInsertion();
	switch( e_PaperInsertion ) {
		case TYPE_PaperInsertionOff:				// しない.
#ifdef DEF_LCD_7INCH
			Key_CopyAuxiliaryBookletCoverSheet_job.changeText( s_pITMOFF_W116H018A, 2 );	// OFF
#else //  DEF_LCD_7INCH.
			Key_CopyAuxiliaryBookletCoverSheet_job.changeText( s_pITMOFF_W108H036A, 2 );	// OFF
#endif  //  DEF_LCD_7INCH.
			break;
		case TYPE_PaperInsertionWithImage:			// 表コピー.
#ifdef DEF_LCD_7INCH
			Key_CopyAuxiliaryBookletCoverSheet_job.changeText( s_pITMFrontCopy_W116H018A, 2 );
#else //  DEF_LCD_7INCH.
			Key_CopyAuxiliaryBookletCoverSheet_job.changeText( s_pBDGFrontCopy_W080H036A, 2 );
#endif  //  DEF_LCD_7INCH.
			break;
		case TYPE_PaperInsertionWithoutImage:		// 表白紙.
#ifdef DEF_LCD_7INCH
			Key_CopyAuxiliaryBookletCoverSheet_job.changeText( s_pITMFrontBlank_W116H018A, 2 );
#else //  DEF_LCD_7INCH.
			Key_CopyAuxiliaryBookletCoverSheet_job.changeText( s_pKEYFrontBlank_W108H034A, 2 );
#endif  //  DEF_LCD_7INCH.
			break;
		default:
			break;
	}
// A3A4FUM so_okada 2015.02.12 Start.
#ifndef DEF_A4_Cus
	if( False == isHalfFoldEnable() && False == isCenterStapleFoldEnable() ) {
		Bak_FoldBind_job.showEnable( False );					// 紙折り/中とじ背景.
		Ktx_CopyFoldAndBind_job.showEnable( False );			// 紙折り/中とじタイトル.
		Rbt_BookletBindSetting_job.showEnable( False );			// 紙折り/中とじ設定キー.
	}
#endif //DEF_A4_Cus.
// A3A4FUM so_okada 2015.02.12 End.
	showPatternCustomize();
}

// ===============================================================================================.
//	［クラス］	PCPC_CopyAuxiliaryBooklet_job B061152400 小冊子 .
//	概要	：画面終了.
//	戻り値	：void.
//	備考	：.
// ===============================================================================================.
void PCPC_CopyAuxiliaryBooklet_job::endGamen()
{
	if( -1 != ss_mBackUpJobTableNo ) {
		PnlComGetCopyDataLib()->JobSpecBackupClear( ss_mBackUpJobTableNo );
		ss_mBackUpJobTableNo = -1;
	}
	PPopupJobSetupChangeCom::endGamen();
}

// ===============================================================================================.
//	［クラス］	PCPC_CopyAuxiliaryBooklet_job B061152400 小冊子 .
//	概要	：ボタン（押された).
//	戻り値	：void.
//	備考	：.
// ===============================================================================================.
void PCPC_CopyAuxiliaryBooklet_job::procButtonOn( uchar option )
{
	APIC_Job* pc_Job = PnlComGetJobCopy();
	if( NULL == pc_Job ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, ( char* )"Assert Failed!" );
		return;
	}
	TYPC_Print* pc_Print = pc_Job->getJobSpec()->pc_Print;

	short s_xPositon = PNLD_ZeroInit, s_yPosition = PNLD_ZeroInit, s_width = PNLD_ZeroInit, s_height = PNLD_ZeroInit;
	PNLC_PopupListControl* pc_PopupListControl = PNLC_PopupListControl::newInstance();

	// オプションチェック.
	switch( option ) {

		case PNLD_LEFTBIND_JOB:										//「左開き」キーのフラグは選択する.
			onlyPanel.setPanelBindDirection( pc_Job->getJobSpec(), TYPE_BindDirectionLeft );
			// APS/AMS処理関数.
			selectAutoPaperMag();
			m_bl_bindDirection = False;
			break;

		case PNLD_RIGHTBIND_JOB:										//「右開き」キーのフラグは選択する.
			onlyPanel.setPanelBindDirection( pc_Job->getJobSpec(), TYPE_BindDirectionRight );
			// APS/AMS処理関数.
			selectAutoPaperMag();
			m_bl_bindDirection = True;
			break;

		case PNLD_NOTDO_JOB:											// しない.
			if( True == isOn( CNK_CenterBinding, pc_Job ) ) {
				cancel( CNK_CenterBinding, CNK_Size, pc_Job );		// 中綴じの取消し.
			}
			if( True == isOn( CNK_Centerori, pc_Job ) ) {
				cancel( CNK_Centerori, CNK_Size, pc_Job );			// 中折りの取消し.
			}
			m_ss_bindSetting = 0;		//「しない」キーのフラグは選択する.
			// APS/AMS処理関数.
			selectAutoPaperMag();
			break;

		case PNLD_CENTERFOLD_JOB:									// 中折り.
			// 機能設定状況の取得.
			if( False == isOn( CNK_Centerori, pc_Job ) ) {
				// 中折りのBOX事前警告.
				const TYPE_BoxNgSpec e_BoxNgSpec[] = { TYPE_BoxNgSpec_FoldSizeCenterInner,		// 折り用紙サイズ不適合(内中折り).
				                                       TYPE_BoxNgSpec_FoldSizeCenterOuter,		// 折り用紙サイズ不適合(外中折り).
				                                       TYPE_BoxNgSpec_FoldTypeCenterInner,		// 折り用紙タイプ不適合(内中折り).
				                                       TYPE_BoxNgSpec_FoldTypeCenterOuter
				                                     };	// 折り用紙タイプ不適合(外中折り).
				// 警告チェック.
				for( ushort i = 0; i < sizeof( e_BoxNgSpec ) / sizeof( e_BoxNgSpec[0] ); i++ ) {
					if( True == dispBoxPrintWarning( e_BoxNgSpec[i], PnlComGetJBJobDocStatus() ) ) {
						return;
					}
				}
				// 組み合わせチェック.
				if( combinationCheck_AccumuDoc( CNK_Centerori, PnlComGetJBJobSpec(), pc_Job, False, True )
				    && combinationCheck( CNK_Centerori, pc_Job, False, True ) ) {
					combinationCheck( CNK_Centerori, pc_Job, True, True );
					pc_Print->c_Finishing.setFoldMode( TYPE_FoldModeCenterOuter );		// 中折りの設定.
					m_ss_bindSetting = 1;		//「中折り」キーのフラグは選択する.
					// APS/AMS処理関数.
					selectAutoPaperMag();
				}
			}
			break;

		case PNLD_CENTERSTAPLE_JOB:									// 中とじ.
			// 機能設定状況の取得.
			if( False == isOn( CNK_CenterBinding, pc_Job ) ) {
				// 中綴じのBOX事前警告.
				const TYPE_BoxNgSpec e_BoxNgSpec[] = { TYPE_BoxNgSpec_StapleCenterWidthMix,
				                                       TYPE_BoxNgSpec_StapleCenterPaperSize,
				                                       TYPE_BoxNgSpec_StapleCenterPaperType,
				                                       TYPE_BoxNgSpec_StapleCenterDirection,
				                                       TYPE_BoxNgSpec_SaddlePaperSize,
				                                       TYPE_BoxNgSpec_SaddlePaperType,
				                                       TYPE_BoxNgSpec_FoldSizeCenterInner,
				                                       TYPE_BoxNgSpec_FoldSizeCenterOuter,
				                                       TYPE_BoxNgSpec_FoldTypeCenterInner,
				                                       TYPE_BoxNgSpec_FoldTypeCenterOuter
				                                     };
				for( ushort i = 0; i < sizeof( e_BoxNgSpec ) / sizeof( e_BoxNgSpec[0] ); i++ ) {
					if( True == dispBoxPrintWarning( e_BoxNgSpec[i], PnlComGetJBJobDocStatus() ) ) {
						return;
					}
				}
				// 組み合わせチェック.
				if( combinationCheck_AccumuDoc( CNK_CenterBinding, PnlComGetJBJobSpec(), pc_Job, False, True )
				    && combinationCheck( CNK_CenterBinding, pc_Job, False, True ) ) {
					combinationCheck( CNK_CenterBinding, pc_Job, True, True );
					// 中とじの設定.
					pc_Print->c_Finishing.setStapleMode( TYPE_StapleModeCenter );
					pc_Print->c_Finishing.setStaplePosition( TYPE_BindDirectionAuto );
					pc_Print->c_Finishing.setFoldMode( TYPE_FoldModeCenterInner );
					m_ss_bindSetting = 2;		//「中とじ」キーのフラグは選択する.
					// APS/AMS処理関数.
					selectAutoPaperMag();
				}
			}
			break;
		case PNLD_COVERSHEET_JOB:									// カバーシートキー
			Key_CopyAuxiliaryBookletCoverSheet_job.getPosition( s_xPositon, s_yPosition );
			Key_CopyAuxiliaryBookletCoverSheet_job.getSize( s_width, s_height );
			pc_PopupListControl->addPopup( &gp_B061152410, this, NULL, s_xPositon, s_yPosition, s_width, s_height );		// B061152410.
			break;
		case PNLE_PopupMessageAreaButtonType_On:								// ONキー.
			if( True == isOn( CNK_Magazine, pc_Job ) ) {
				setBuzzerType( PCGE_BuzzerType_InputNg );
			} else if( -1 != ss_mBackUpJobTableNo ) {
				// 前のON状態に戻す.
				PnlComGetCopyDataLib()->setJobSpecBackup( ss_mBackUpJobTableNo, pc_Job->getJobSpec() );
				PnlComGetCopyDataLib()->JobSpecBackupClear( ss_mBackUpJobTableNo );
				ss_mBackUpJobTableNo = -1;
				// APS/AMS処理関数.
				selectAutoPaperMag();
			} else if( False == dispBoxPrintWarning( TYPE_BoxNgSpec_OutputSettingMagChange, PnlComGetJBJobDocStatus() )
			           && False == dispBoxPrintWarning( TYPE_BoxNgSpec_DupPaperError, PnlComGetJBJobDocStatus() )
			           && combinationCheck_AccumuDoc( CNK_Magazine, PnlComGetJBJobSpec(), pc_Job, False, True )
			           && combinationCheck( CNK_Magazine, pc_Job, False, True ) ) {
				setOn( False );	// 中綴じ設定ON(用紙トレイ設定無し).
			} else {
				Cmp_BookletOnOffMessageArea_job.setOnOffSwitchButtonState( False );
			}
			break;

		case PNLE_PopupMessageAreaButtonType_Off:								// OFFキー.
			if( False == isOn( CNK_Magazine, pc_Job ) ) {
				setBuzzerType( PCGE_BuzzerType_InputNg );
			} else {
				ss_mBackUpJobTableNo = PnlComGetCopyDataLib()->saveJobSpecBackup( ss_mBackUpJobTableNo, pc_Job->getJobSpec() );
				cancel( CNK_Magazine, CNK_Size, pc_Job );
				TYPC_Common* pc_Common = pc_Job->getJobSpec()->pc_Common;
				ushort us_ChangeOutMode = pc_Common->getChangeOutMode();
				us_ChangeOutMode |= TYPE_ChangeOutMode_BoxMag;	// 倍率変更フラグをつける.
				pc_Common->setChangeOutMode( us_ChangeOutMode );
				selectAutoPaperMag();	// APS/AMS処理関数.
			}
			break;
		default: break;
	}
	if( True == pc_PopupListControl->isNowPopup( this ) ) {
		setupGamen( option );		// 表示部品のセットアップ.
	}
}

// ===============================================================================================.
//	［クラス］	PCPC_CopyAuxiliaryBooklet_job B061152400 小冊子 .
//	概要	：各部品表示／非表示.
//	戻り値	：void.
//	備考	：.
// ===============================================================================================.
void PCPC_CopyAuxiliaryBooklet_job::showPatternCustomize( )
{
	if( False == PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_Booklet_LeftBind ) ) {
		Key_LeftBind_job.showEnable( False );
	}
	if( False == PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_Booklet_RightBind ) ) {
		Key_RightBind_job.showEnable( False );
	}
	if( False == PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_Booklet_CenterStapleFold ) ) {
		Key_BookletCenterStaple_job.showEnable( False );
	}
	if( False == PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_Booklet_HalfFold ) ) {
		Key_BookletCenterFold_job.showEnable( False );
	}
	if( False == PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_Booklet_CoverMode ) ) {
		Key_CopyAuxiliaryBookletCoverSheet_job.showEnable( False );
	}

	if( False == PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_Booklet_LeftBind ) &&
	    False == PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_Booklet_RightBind ) ) {
		Gra_BookletRightDifference_job.showEnable( False );
		Gra_BookletLeftDifference_job.showEnable( False );
		Bak_LeftRightDifference_job.showEnable( False );
		Bak_LeftRightDifferenceSub_job.showEnable( False );
	}
// A3A4FUM so_okada 2015.02.12 Start.
#ifndef DEF_A4_Cus	
	if( False == PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_Booklet_CenterStapleFold ) &&
	    False == PnlComGetFunctionShowCopy( TYPE_PNLMENU_COPY_Booklet_HalfFold ) ) {
		Bak_FoldBind_job.showEnable( False );
		Ktx_CopyFoldAndBind_job.showEnable( False );
		Rbt_BookletBindSetting_job.showEnable( False );
	}
#endif //DEF_A4_Cus.
// A3A4FUM so_okada 2015.02.12 End.	
}

// ===============================================================================================.
//	［クラス］	PCPC_CopyAuxiliaryBooklet_job B061152400 小冊子 .
//	概要	：小冊子設定のOn.
//	戻り値	：void.
//	備考	：.
// ===============================================================================================.
void PCPC_CopyAuxiliaryBooklet_job::setOn( Bool b_setPaperTray )
{
	// 小冊子クイック設定モードのOn・Offに関わらず、小冊子をOnにした場合はトレイ選択・中綴じ設定が必要なため設定処理を関数化.
	APIC_Job* pc_Job = PnlComGetJobCopy();
	if( NULL == pc_Job ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return;
	}


	// 小冊子設定.
	setupAuxiliaryBooklet();

	// 中綴じまたは中折りの設定.
	TYPC_Print* pc_Print = pc_Job->getJobSpec()->pc_Print;
	// フィニッシャーおよびサドルユニットの装着確認と暫定救済中でないかのチェック.
	if( ( PCGE_FoldBindFunction_PaperFoldCenterBind == chkFoldBindFunction() )
	    && ( TYPE_TemporaryRescueStatusReset == PnlComGetSystemStatus()->getTemporaryRescueUserStatus( TYPE_TemporaryRescueUnitFolder ) ) ) {
		// 中綴じOffの場合のみ設定する.
		if( False == isOn( CNK_Centerori, pc_Job ) && False == isOn( CNK_CenterBinding, pc_Job ) ) {
			Bool bl_Error = False;
			// 中綴じのBOX事前警告.
			const TYPE_BoxNgSpec e_BoxNgSpec[] = { TYPE_BoxNgSpec_StapleCenterWidthMix,
			                                       TYPE_BoxNgSpec_StapleCenterPaperSize,
			                                       TYPE_BoxNgSpec_StapleCenterPaperType,
			                                       TYPE_BoxNgSpec_StapleCenterDirection,
			                                       TYPE_BoxNgSpec_SaddlePaperSize,
			                                       TYPE_BoxNgSpec_SaddlePaperType,
			                                       TYPE_BoxNgSpec_FoldSizeCenterInner,
			                                       TYPE_BoxNgSpec_FoldSizeCenterOuter,
			                                       TYPE_BoxNgSpec_FoldTypeCenterInner,
			                                       TYPE_BoxNgSpec_FoldTypeCenterOuter
			                                     };
			for( ushort i = 0; i < sizeof( e_BoxNgSpec ) / sizeof( e_BoxNgSpec[0] ); i++ ) {
				if( True == checkBoxPrintWarning( e_BoxNgSpec[i], PnlComGetJBJobDocStatus() ) ) {
					bl_Error = True;
				}
			}
			if( False == bl_Error ) {
				if( True == combinationCheck_AccumuDoc( CNK_CenterBinding, PnlComGetJBJobSpec(), pc_Job, False, False , CNK_Size, False, True )
				    && True == combinationCheck( CNK_CenterBinding, pc_Job, False, False ) ) {
					combinationCheck( CNK_CenterBinding, pc_Job, True, False );
					// 中綴じの設定.
					pc_Print->c_Finishing.setStapleMode( TYPE_StapleModeCenter );
					pc_Print->c_Finishing.setStaplePosition( TYPE_BindDirectionAuto );
					pc_Print->c_Finishing.setFoldMode( TYPE_FoldModeCenterInner );
				}
			}
		}
		if( False == isOn( CNK_CenterBinding, pc_Job ) && False == isOn( CNK_Centerori, pc_Job ) ) {
			Bool bl_Error = False;
			// 中折りのBOX事前警告.
			const TYPE_BoxNgSpec e_BoxNgSpec[] = { TYPE_BoxNgSpec_FoldSizeCenterInner,		// 折り用紙サイズ不適合(内中折り).
			                                       TYPE_BoxNgSpec_FoldSizeCenterOuter,		// 折り用紙サイズ不適合(外中折り).
			                                       TYPE_BoxNgSpec_FoldTypeCenterInner,		// 折り用紙タイプ不適合(内中折り).
			                                       TYPE_BoxNgSpec_FoldTypeCenterOuter
			                                     };	// 折り用紙タイプ不適合(外中折り).
			// 警告チェック.
			for( ushort i = 0; i < sizeof( e_BoxNgSpec ) / sizeof( e_BoxNgSpec[0] ); i++ ) {
				if( True == checkBoxPrintWarning( e_BoxNgSpec[i], PnlComGetJBJobDocStatus() ) ) {
					bl_Error = True;
					break;
				}
			}
			if( False == bl_Error ) {
				if( True == combinationCheck_AccumuDoc( CNK_Centerori, PnlComGetJBJobSpec(), pc_Job, False, False , CNK_Size, False, True )
				    && True == combinationCheck( CNK_Centerori, pc_Job, False, False ) ) {
					combinationCheck( CNK_Centerori, pc_Job, True, False );
					// 中折りの設定.
					pc_Print->c_Finishing.setFoldMode( TYPE_FoldModeCenterOuter );
				}
			}
		}
	}
	// APS/AMS処理関数.
	selectAutoPaperMag();
}

