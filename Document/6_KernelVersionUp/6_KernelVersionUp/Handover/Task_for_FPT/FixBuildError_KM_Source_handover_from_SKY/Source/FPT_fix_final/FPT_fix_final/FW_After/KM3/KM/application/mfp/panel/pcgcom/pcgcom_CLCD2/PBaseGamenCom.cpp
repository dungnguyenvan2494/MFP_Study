//==============================================================================
//［タイトル］	UIベース画面共通クラス.
//==============================================================================

//==============================================================================
// インクルード・ファイル.
//==============================================================================
#include <panel/plibh/plibh_FUM2/plibh_FUM2.h>
#include <panel/pcgcom/pcgcomg/pnlmain.h>
#include <panel/plibh/plibh_CLCD2/plibh_CLCD2.h>
#include "PBaseGamenCom.h"
#include "PNLC_PopupListControl.h"
#include <panel/ppcg/ppcgcom/PNLC_AnimationTimerManager.h>

#include <panel/ppcg/ppcgg/ppcglanguageNewBin.h>
#include <panel/plibh/plibh_CLCD2/PNLC_ScanFaxDataSearchLib.h>	// comment MinervaZeus Sky小尾 2011/08/16 [CQ352234].
#include <panel/pcgact/pcgact_CLCD2/pigact_pA072002.h>			// comment OAP-NUI Sky石賀 2011/09/23 [CQ355137].
#include "../../../../divlib/Common/blkapi/blkshm.h"
#include "panel/pcgcom_ext/pcgcom_extg/PCGC_PublicPopupDummyBase.h"

// comment 2016/03/30 OPS1-Sky長尾芳人 [操作性改善][不要描画] 不要描画検出機構対応.
#include <panel/pnlmngr/pnlmngrg/PNLC_PartsAccessDebugLib.h>
// Comment 2017/01/09 [HCL-Vinod] [IT5 4.2][AR1038184] To close the slide menu system auto rest happens.
#include <panel/plibh/plibh_CLCD2/PNLC_SlideMenuControlLib.h>

#include "system/dbg/APIC_DebugLogSystem.h"		// comment 2018/07/31 SCD2-Sky東野 [IT6] [1056724] 不具合対応.
// comment 2022/06/22 Sky東島 [IT6_2.3][制御構成変更_No.51][サービスレスメンテナンス].
#include <panel/pcgbrowser_ext/pcgbrowser_extg/pigPopupBrowser.h>

/*================================================================================================
	グローバル変数の宣言.
================================================================================================*/
PBaseWidget c_CaptureBase( c71_000_000_t1_X800_Y480, 0, 0, 0, TYPE_LayoutFree, False );		// 画面キャプチャWidget(ベース画面用).
PBaseWidget c_CaptureSlide( c71_000_000_t1_X800_Y480, 0, 0, 0, TYPE_LayoutFree, False );		// 画面キャプチャWidget(スライドインアウト画面用).

/*================================================================================================
	static メンバ変数の宣言.
================================================================================================*/
Bool	PBaseGamenCom::bl_mmbSlideInFinished	= False;		//!< スライドイン完了フラグ.
Bool	PBaseGamenCom::bl_mmbSlideOutFinished	= False;		//!< スライドアウト完了フラグ.
short	PBaseGamenCom::m_ss_GamenWarningNo		= 0;			//!< 保留警告画面番号.
// comment 2018/10/23 [HCLT_Prem][DenebMLK] [AR1050427] Card Copy Support.
PNLS_AddStackPopup	PBaseGamenCom::m_st_AddStackPopup[PNLD_BASEGAMENCOM_ADDPOPUP_NUM] = {	//!< // ポップアップの情報を保持する構造体.
	{ NULL , False },
};
/*================================================================================================
	モジュール内変数の宣言.
================================================================================================*/
// ＬＣＤ内キーメッセージ＋10（*#）キー＋クリアキー.
static const Message s_keyMessage[] = {
	MS_IndexButton,
	MS_GoToPage,
	MS_Ok,
	MS_Cancel,
	MS_End,
	MS_GoToPaper,
	MS_GoToMag,
	MS_GoToFace,
	MS_Button,
	MS_KeyBoard,
	MS_GoToScanColor,
	MS_GoToScanResolution,
	MS_GoToScanColorLevel,
	MS_GoToScanScanSize,
	MS_OneTouchKey,
	MS_BoxUp,
	MS_BoxDown,
	MS_BoxIndex,
	MS_BoxButton,
	MS_DocButton,
	MS_ShortCutKey,
	MS_TabIndex,
	MS_Drag,
	MS_Release,
	MS_10Key,
	MS_Clear,
	MS_DoubleTap,	// 【画面実装課題 No.131】ダブルタップ.
};

// 外キー.
static const Message s_keyMessage2[] = {
	MS_Start,					// スタート.
	MS_Stop,					// ストップ.
	MS_Clear,					// クリア.
	MS_Reset,					// リセット.
	MS_Utility,					// 設定メニュー.
	MS_IDInput,					// ID.
	MS_PowerSave,				// パワーセーブ.
	MS_Help,					// ガイド.
	MS_Enlarge,					// 拡大表示.
	MS_SampleCopy,				// プレビュー.
	MS_IntMode,					// 割り込み.
	MS_Menu,					// メニュー.
	MS_Copy,					// コピー.
	MS_Fax,						// ファクス／スキャン.
	MS_Box,						// ボックス.
	MS_Browser,					// Webブラウザー.
	MS_APPsMenu,				// APP.
	MS_Soft10Key,				// 10キー呼び出し.
	MS_SlideMenu,				// スライドメニュー.
	MS_SpecifyApp,				// 指摘アプリ起動.
	MS_FaxOnly,					// ファクス専用.
// comment 2015/04/30 OPS1-Sky佐藤 [HeliosL][CQ513137] ズームキーの登録キー割り当て対応.
	MS_ZoomIn,					// 表示ズーム.
};

//==============================================================================
//概要     : デストラクタ.
//==============================================================================
PBaseGamenCom::~PBaseGamenCom()
{
}

//==============================================================================
//概要     : 画面開始処理.
//==============================================================================
void PBaseGamenCom::startGamen()
{
// Comment 2017/01/09 [HCL-Vinod] [IT5 4.2][AR1038184] To close the slide menu system auto rest happens.
	if( True == PNLC_SlideMenuControlLib::newInstance()->isOpen() ) { 
		PNLC_SlideMenuControlLib::newInstance()->close(True);
	}
	// 親クラスの処理を呼ぶ.
	PLayerControl::startGamen();

	Bool b_Language = PnlComGetOperationSetting()->getLanguageChoiceSetting();
	if( True== onlyPanel.b_LanguageSystemOut && True == b_Language && PnlComIsMyPanel() != True ) {		// マイパネル使用中の場合は言語再構築処理は行わない. ここから.
// comment 2020/10/22 Sky中野 [IT6_2.1][OP_BTS-28575].
//		onlyPanel.e_Language = PnlComGetMachineSetting()->getLanguage();			// 一時変更言語設定を、元に戻す。
//		PnlComGetOperationSetting()->setSelectLanguage( onlyPanel.e_Language );
		if( NULL != PnlComGetMachineSetting() ) {
			PnlComSetSelectLanguage( PnlComGetMachineSetting()->getLanguage() );
		}
		
		int iret;
		SHM_TYPE_DATA* p = NULL;

		// 共有メモリクラス取得
		DivCommon_Shm* p_DivCommon_Shm = DivCommon_Shm::newInstance();

		// アタッチ
		iret = p_DivCommon_Shm->Shm_attach( ( void* )&p );

		// アタッチ成功
		if( iret == 0 )
		{
			// 共有メモリに値を設定
			p->e_Language_no_Int = onlyPanel.e_Language;
			// デタッチ
			iret = p_DivCommon_Shm->Shm_detattch( p );
		}
// comment 2015/06/29 OPS1-Sky森川(篤) [ZeusS_ZX0][RQ-405] パネルUIのヘブライ語対応(既存問題対応).
//		loadSelectLangType_IOP( onlyPanel.e_Language );								// メッセージデータ取得関数に置き換え.
		loadSelectLangType_IOP( PnlComGetLanguage() );								// メッセージデータ取得関数に置き換え.
		onlyPanel.b_LanguageSystemOut = False;
		// 表示言語とキーボード言語が連動している場合は、キーボード言語を元に戻す.
		// ソフトキーボード.
		if( True == PnlComGetMachineSetting()->getSoftwareKeyboardWithPanalLanguage() ) {
// comment 2015/06/29 OPS1-Sky森川(篤) [ZeusS_ZX0][RQ-405] パネルUIのヘブライ語対応(既存問題対応).
//			PnlComGetOperationSetting()->setKeyboardLanguage( onlyPanel.e_Language );
// comment 2018/12/14 Sky毛穴 [EagleHigh][RQ-258] タイ語とベトナム語の対応.
//			if ( isRightDispLanguage(onlyPanel.e_Language) ) {
			if (   True == isRightDispLanguage( onlyPanel.e_Language )
				|| TYPE_LanguageThai 	== onlyPanel.e_Language			// タイ語.
				|| TYPE_LanguageVietnam == onlyPanel.e_Language			// ベトナム語.
// comment 2022/11/9 Sky奥田 [RQ-938] バルト三国対応.
				|| TYPE_LanguageEstonian	== onlyPanel.e_Language		// エストニア語.
				|| TYPE_LanguageLatvian		== onlyPanel.e_Language		// ラトビア語.
				|| TYPE_LanguageLithuanian	== onlyPanel.e_Language		// リトアニア語.
// comment 2023/2/6 Sky小熊 [IT6_2.3][RQ-970]Welsh as standard language for IT.
				|| TYPE_LanguageWelsh		== onlyPanel.e_Language		// ウェールズ語.
			) {
				PnlComGetOperationSetting()->setKeyboardLanguage( TYPE_LanguageAmerican );
			} else {
				PnlComGetOperationSetting()->setKeyboardLanguage( onlyPanel.e_Language );
			}
		}
		// 外付けキーボード.
		if( True == PnlComGetMachineSetting()->getExtKeyboardWithPanalLanguage() ) {
// comment 2015/06/29 OPS1-Sky森川(篤) [ZeusS_ZX0][RQ-405] パネルUIのヘブライ語対応(既存問題対応).
//			PnlComGetMachineSetting()->setExtKeyboardLanguage( onlyPanel.e_Language );
// comment 2018/12/14 Sky毛穴 [EagleHigh][RQ-258] タイ語とベトナム語の対応.
//			if ( isRightDispLanguage(onlyPanel.e_Language) ) {
			if (   True == isRightDispLanguage( onlyPanel.e_Language )
				|| TYPE_LanguageThai 	== onlyPanel.e_Language			// タイ語.
				|| TYPE_LanguageVietnam == onlyPanel.e_Language			// ベトナム語.
// comment 2022/11/9 Sky奥田 [RQ-938] バルト三国対応.
				|| TYPE_LanguageEstonian	== onlyPanel.e_Language		// エストニア語.
				|| TYPE_LanguageLatvian		== onlyPanel.e_Language		// ラトビア語.
				|| TYPE_LanguageLithuanian	== onlyPanel.e_Language		// リトアニア語.
// comment 2023/2/6 Sky小熊 [IT6_2.3][RQ-970]Welsh as standard language for IT.
				|| TYPE_LanguageWelsh		== onlyPanel.e_Language		// ウェールズ語.
			) {
				PnlComGetMachineSetting()->setExtKeyboardLanguage( TYPE_LanguageAmerican );
			} else {
				PnlComGetMachineSetting()->setExtKeyboardLanguage( onlyPanel.e_Language );
			}
		}
	}

	PNLC_FunctionBadgeLib::newInstance()->SetInvalidFlag( True );
}

//==============================================================================
//概要     : 画面終了処理.
//==============================================================================
void PBaseGamenCom::endGamen()
{
	if( ( PNLE_SlideInOutStatus_In     == e_mmbSlideInOutStatus ) ||
	    ( PNLE_SlideInOutStatus_InReq  == e_mmbSlideInOutStatus ) ||
	    ( PNLE_SlideInOutStatus_Out    == e_mmbSlideInOutStatus ) ||
	    ( PNLE_SlideInOutStatus_OutReq == e_mmbSlideInOutStatus ) ) {							// アニメーション中の画面終了.
		completeGamenAnimationForced();
		e_mmbSlideInOutStatus = PNLE_SlideInOutStatus_Non;	// アニメーション状態を初期化.
	}

	// 親クラスの処理を呼ぶ.
	// アニメーション実行中の画面破棄を避けるためアニメーション強制終了後に終了処理を行う.
	PLayerControl::endGamen();

	e_mRestart = PNLE_BaseRestart_Non;				// ポップアップ閉時の更新要求ステータスを初期化.  comment MinervaZeus Sky小尾 2011/08/16 [CQ352234].
}

//==============================================================================
//概要     : 画面パーツの表示.
//==============================================================================
void PBaseGamenCom::show()
{
	// 親クラスの処理をコール.
	PLayerControl::show();

	// スライドアニメーション開始処理.
	startSlideAnimation();
}

//==============================================================================
//概要     : スライドアニメーション開始処理.
//==============================================================================
void PBaseGamenCom::startSlideAnimation()
{
	// アニメーション要求中でない場合、処理なし.
	if( ( PNLE_SlideInOutStatus_InReq != e_mmbSlideInOutStatus ) &&
	    ( PNLE_SlideInOutStatus_OutReq != e_mmbSlideInOutStatus ) ) {
		return;
	}

	PNLE_SlideInOutInfo e_SlideInOutInfo = PNLE_SlideInOutInfo_In;		// スライドインで初期化.

	// 現在の画面のWidget.
	PBaseWidget* pc_nowGamenWidget = static_cast<PBaseWidget*>( nowGamen->getDisplayParts() );
//Comment 2018/10/24 【HCLT-Sowjanya】【IT6_Sp】【AR-1050932】fixing the segmentation issue.
	if( NULL == pc_nowGamenWidget ) {
			return;
	}
	// スライドインアニメーション要求中の場合.
	if( PNLE_SlideInOutStatus_InReq == e_mmbSlideInOutStatus ) {
		// アニメーション状態を更新.
		e_mmbSlideInOutStatus = PNLE_SlideInOutStatus_In;

		// スライドイン画面をキャプチャ.
		c_CaptureSlide.createCaptureImage( pc_nowGamenWidget );

	}
	// スライドアウトアニメーション要求中の場合.
	else {
		// アニメーション状態を更新.
		e_mmbSlideInOutStatus = PNLE_SlideInOutStatus_Out;

		// ベース画面をキャプチャ.
		c_CaptureBase.createCaptureImage( pc_nowGamenWidget );

		// スライドアウトに設定.
		e_SlideInOutInfo = PNLE_SlideInOutInfo_Out;
	}

	// ベース画面のキャプチャを最前面に移動し表示.
	c_CaptureBase.bringToFront();
	c_CaptureBase.show();

	// スライドアニメーション中はキャプチャ画面の裏に隠れている現在の画面を一旦非表示とする(アニメーション完了のタイミング(procAnimationEnd)で表示させる).
	pc_nowGamenWidget->hide();

	// スライド画面のキャプチャを最前面に移動し表示.
	c_CaptureSlide.bringToFront();
	c_CaptureSlide.show();

	// スライドアニメーション開始.
	pc_mmbSlideInOutAnimation->setAnimationInfo( e_SlideInOutInfo, &c_CaptureSlide, &c_CaptureBase );
	pc_mmbSlideInOutAnimation->attachObserver( this );
	pc_mmbSlideInOutAnimation->startAnimation();

	// メンテナンスモード中か判断し、メンテナンスモード中なら赤バーを表示する.
	if( NULL != m_pc_MaintenanceModeStatusBar )	{
// comment 2015/12/07 MSE天木 [ZeusS_ZX0][CQ521804] メンテナンスモード中のステータスバー表示条件の修正.
		//APIC_SystemSetting* pc_SystemSetting = APIC_SystemSetting::newInstance();
		//APIC_SystemInfo*     pc_SystemInfo    = APIC_SystemInfo::newInstance();

		//if( ( 0 != pc_SystemSetting ) && ( 0 != pc_SystemInfo ) )
			//Bool bl_AllowMaintenanceMode  = pc_SystemSetting->getAllowMaintenanceMode();	//メンテナンスモード許可する/許可しない.
			//Bool bl_MaintenanceModeEnable = pc_SystemInfo->getEnableMaintenanceMode();	//メンテナンスモード有効/無効.

			//if( ( True == bl_AllowMaintenanceMode ) && ( True == bl_MaintenanceModeEnable ) )
		if ( True == PnlComGetSystemInfo()->getEnableMaintenanceMode() ) {
			// メンテナンスモード有効時は表示する.
			GFWC_Common* pc_GUISelf = static_cast< GFWC_Common* >( m_pc_MaintenanceModeStatusBar->getGUIParts() );

			if( NULL != pc_GUISelf ) {
				pc_GUISelf->bringToFront();
				DEF_UPDATEDRAWPARTSINFO_GUI;		// comment 2016/03/30 OPS1-Sky長尾芳人 [操作性改善][不要描画] 不要描画検出機構対応.
			}
		}
	}
}

// ===============================================================================================
/**
 *	@brief		スライドイン開始要求.
 *	@param[in]	( *p_CallbackFunc )( uchar, short, void*, uint32_t )	コールバック関数ポインタ.
 *	@param[in]	ss_SrcPosX												起動元（主に起動ボタン）の位置.
 *	@param[in]	ss_SrcPosY												起動元（主に起動ボタン）の位置.
 *	@param[in]	ss_SrcSizeX												起動元（主に起動ボタン）のサイズ.
 *	@param[in]	ss_SrcSizeY												起動元（主に起動ボタン）のサイズ.
 *	@param[in]	bl_AnimLess												起動アニメーション強制OFFフラグ（True：OFFする、False：OFFしない）.
 *	@param[in]	b_IgnoreQueueInfo = False								起動アニメーション強制OFF時の画面表示完了後、起動・終了予約画面を無視するか否か（True：無視する、False：無視しない）.
 */
// ===============================================================================================
void PBaseGamenCom::opened(
    void ( *p_CallbackFunc )( uchar, short, void*, uint32_t ),
    short ss_SrcPosX,
    short ss_SrcPosY,
    short ss_SrcSizeX,
    short ss_SrcSizeY,											// 起動元（主に起動ボタン）のサイズ.
    Bool  bl_AnimLess,											// 起動アニメーション強制OFFフラグ（True：OFFする、False：OFFしない）.
    Bool  b_IgnoreQueueInfo	/* = False */						// 起動アニメーション強制OFF時の画面表示完了後、起動・終了予約画面を無視するか否か（True：無視する、False：無視しない）.
) {
	// 自身のアニメーション状態チェック.
	if( PNLE_SlideInOutStatus_Non == e_mmbSlideInOutStatus || PNLE_SlideInOutStatus_Finish == e_mmbSlideInOutStatus ) {
		PBasicMessageProc::setNextGamen( this );
		// アニメーションステータスにスライドイン要求中を設定.
		e_mmbSlideInOutStatus = PNLE_SlideInOutStatus_InReq;
		// アニメーションコンポーネントチェック.
		if( bl_AnimLess == False ) {

			// ベース画面をキャプチャ.
			c_CaptureBase.createCaptureImage( static_cast<PBaseWidget*>( nowGamen->getDisplayParts() ) );

			m_bPopupAnimeAct = True;						// アニメーション中.
		} else {
			// アニメーションステータスにアニメーション完了を設定.
			e_mmbSlideInOutStatus = PNLE_SlideInOutStatus_Finish;
		}
	}
}

// ===============================================================================================
/**
 *	@brief		スライドアウト開始要求.
 *	@param[in]	bl_Delay			終了アニメーションをディレイさせるかどうか.
 *	@param[in]	bl_Immediately		即時（アニメーションなしで）終了させるかどうか.
 *	@param[in]	b_IgnoreQueueInfo = False	アニメーションなし時の画面終了処理完了後、起動・終了予約画面を無視するか否か（True：無視する、False：無視しない）.
 */
// ===============================================================================================
void PBaseGamenCom::closed( Bool bl_Delay, Bool bl_Immediately, Bool b_IgnoreQueueInfo )
{
	PBasicMessageProc::setNextGamen( this );						// 自身を遷移先画面に設定.
	if( True == bl_Immediately ) {
		// アニメーション停止.
		pc_mmbSlideInOutAnimation->stopAnimation();
		bl_mmbSlideInFinished = False;
		bl_mmbSlideOutFinished = False;
	} else {
		// 自身のアニメーション状態チェック.
		if( PNLE_SlideInOutStatus_Non == e_mmbSlideInOutStatus || PNLE_SlideInOutStatus_Finish == e_mmbSlideInOutStatus ) {
			// ステータスをスライドアウト要求中に変更.
			e_mmbSlideInOutStatus = PNLE_SlideInOutStatus_OutReq;

			// スライド画面をキャプチャ.
			c_CaptureSlide.createCaptureImage( static_cast<PBaseWidget*>( nowGamen->getDisplayParts() ) );

			m_bPopupAnimeAct = True;						// アニメーション中.
		}
	}
}

// ===============================================================================================
/**
 *	@brief		メッセージディスパッチ（外部からの呼び出し用）.
 *	@param[in]	message			メッセージ.
 *	@param[in]	option			オプション値.
 *	@param[in]	type			種別.
 */
// ===============================================================================================
void PBaseGamenCom::messageDispathPopup( Message message, uchar option, MessageType type, int32_t sl_option, void* st_NtfEventData )
{
	// 自身のアニメーション状態をチェック.
	if( ( PNLE_SlideInOutStatus_In     == e_mmbSlideInOutStatus ) ||		// スライドインアニメーション中.
	    ( PNLE_SlideInOutStatus_InReq  == e_mmbSlideInOutStatus ) ||		// スライドインアニメーション要求中.
	    ( PNLE_SlideInOutStatus_Out    == e_mmbSlideInOutStatus ) ||		// スライドアウトアニメーション中.
	    ( PNLE_SlideInOutStatus_OutReq == e_mmbSlideInOutStatus ) ) {		// スライドアウトアニメーション要求中.
		// スライドイン・アウト中は、キー操作関連のイベントを処理しない.
		for( register uchar i=0; i<sizeof( s_keyMessage ); ++i ) {
			if( s_keyMessage[i] == message ) {
				return;
			}
		}
		PBasicMessageProc::messageDispath( message, option, type, sl_option, st_NtfEventData );
	} else {
		// 通常は、普通にメッセージを処理.
		PBasicMessageProc::messageDispath( message, option, type, sl_option, st_NtfEventData );
	}
}

// ===============================================================================================
/**
 *	@brief		定期起床.
 *	@param[in]	option			オプション.
 */
// ===============================================================================================
void PBaseGamenCom::procTimerWakeUp( uchar option )
{
	PLayerControl::procTimerWakeUp( option );		// 親クラスをコール.

	PNLC_PopupListControl* pc_PopupListCnt = PNLC_PopupListControl::newInstance();
	PLayerControl* pc_LayerControl = pc_PopupListCnt->getNowPopup();					// 最上位ポップアップを取得.
	Bool bl_popupEnable = True;
	if( ( NULL != pc_LayerControl ) && ( True != pc_PopupListCnt->isPopupOpen() ) && ( True != pc_LayerControl->isNowPopupAnimation() ) ) {
		bl_popupEnable = False;	// ポップアップ不在.
	}

	// アニメーション中はsetupGamenさせない.
	if( ( PNLE_SlideInOutStatus_In     == e_mmbSlideInOutStatus ) ||		// スライドインアニメーション中.
	    ( PNLE_SlideInOutStatus_InReq  == e_mmbSlideInOutStatus ) ||		// スライドインアニメーション要求中.
	    ( PNLE_SlideInOutStatus_Out    == e_mmbSlideInOutStatus ) ||		// スライドアウトアニメーション中.
	    ( PNLE_SlideInOutStatus_OutReq == e_mmbSlideInOutStatus ) ) {		// スライドアウトアニメーション要求中.
		m_bPopupAnimeAct = True;
	}

	if( False == bl_popupEnable ) {
		switch( e_mRestart ) {
			case PNLE_BaseRestart_Trigger:
				// アニメーション中でなければ画面の開始処理を呼び出し、描画更新する.
				startGamen();
				refreshNowGamen();
				e_mRestart = PNLE_BaseRestart_Non;
				break;
			case PNLE_BaseRestart_SearchTrigger:
			{
				// 検索開始（初回のみ）.
				PNLC_ScanFaxDataSearchLib* pc_ScanFaxDataSearchLib = PNLC_ScanFaxDataSearchLib::newInstance();		// 検索関連クラス.
				// S/MIME機能設定.
				pc_ScanFaxDataSearchLib->SmimeSetup();
				Bool b_OftenKey = pc_ScanFaxDataSearchLib->isOftenKey();											// 常用フラグ格納用.
				TYPE_SearchKey e_SearchKey = pc_ScanFaxDataSearchLib->getSearchKindKeyword();						// 検索キー格納用.
				TYPE_FaxDefaultAddrKind e_FaxDefaultAddrKind = PnlComGetFaxDefaultAddrKind();	// 宛先種別格納用.
				Bool bl_LimitationFunction 	= PNLC_LimitationFunctionLoginUserCheckLib::newInstance()->isEnableSetAddressLimitationFunction();
				pc_ScanFaxDataSearchLib->setSearchConditionAPI( &b_OftenKey, e_SearchKey, e_FaxDefaultAddrKind, bl_LimitationFunction );	// 検索要求.
				// お待ち下さい画面.
				pc_ManageIGCWaitDisplay.initManageIGCWaitDisplay( PCGD_IGCDisplayStandard, this, this );
				DelayedIGCWaitDisplayTimer::start_popup( this );
				e_mRestart = PNLE_BaseRestart_Non;
			}
			break;
			default: break;
		}
	}
}
// ===============================================================================================
/**
 *	@brief		メッセージディスパッチ（外部からの呼び出し用）.
 *	@param[in]	message			メッセージ.
 *	@param[in]	option			オプション値.
 *	@param[in]	type			種別.
 */
// ===============================================================================================
void PBaseGamenCom::messageDispath( Message message, uchar option, MessageType type, int32_t sl_option, void* st_NtfEventData )
{
	// 自身のアニメーション状態をチェック.
	if( ( PNLE_SlideInOutStatus_In     == e_mmbSlideInOutStatus ) ||		// スライドインアニメーション中.
	    ( PNLE_SlideInOutStatus_InReq  == e_mmbSlideInOutStatus ) ||		// スライドインアニメーション要求中.
	    ( PNLE_SlideInOutStatus_Out    == e_mmbSlideInOutStatus ) ||		// スライドアウトアニメーション中.
	    ( PNLE_SlideInOutStatus_OutReq == e_mmbSlideInOutStatus ) ) {		// スライドアウトアニメーション要求中.
		// スライドイン・アウト中は、キー操作関連のイベントを処理しない.
		for( register uchar i=0; i<sizeof( s_keyMessage ); ++i ) {
			if( s_keyMessage[i] == message ) {
				return;
			}
		}
		for( uchar i=0; i<sizeof( s_keyMessage2 ); ++i ) {
			if( s_keyMessage2[i] == message ) {
				// 外キーの場合はNG音鳴動.
				setBuzzerType( PCGE_BuzzerType_InputNg );
				return;
			}
		}
		PBasicMessageProc::messageDispath( message, option, type, sl_option, st_NtfEventData );
	} else {
		// 通常は、普通にメッセージを処理.
		PBasicMessageProc::messageDispath( message, option, type, sl_option, st_NtfEventData );
	}
}

// ===============================================================================================
/**
 *	@brief		アニメーションタイマーハンドラ.
 */
// ===============================================================================================
void PBaseGamenCom::animateNowGamen2()
{
	PNLC_AnimationTimerManager::getInstance()->DoAnimationTimerStop();
	if( True == bl_WaitFlg ) {
		if( 0 < ss_Wait ) {
			ss_Wait = static_cast<short>( ss_Wait - 80 );		// (暫定)時間はpnlmainのアニメーション時間と差異がでないように定数化する.
		}
		if( 0 >= ss_Wait ) {
			setWait( False );
			PNLC_PopupListControl::newInstance()->actReservedPopup();
		}
	}
}

// ===============================================================================================
/**
 *	@brief		ベース画面（スライドイン時）のオフセット指定.
 *	@param[in]	ss_ShiftX		横方向シフト量.
 *	@param[in]	ss_ShiftY		縦方向シフト量.
 */
// ===============================================================================================
void PBaseGamenCom::setOffset( short ss_ShiftX, short ss_ShiftY )
{
	// パーツテーブルのポインタとパーツ数を取得する.
	PDisplayParts** p_PartsTable = getPartsTable();
	const short ss_PartsNumber = getPartsNumber();

	// パーツテーブルに登録されている全パーツに対して位置変更（shiftPosition）を行う.
	for( register short i=0; i<ss_PartsNumber; ++i ) {
		p_PartsTable[i]->shiftPosition( ss_ShiftX, ss_ShiftY, False );	// 座標移動.
	}
}

// ===============================================================================================
/**
 *	@brief		スライドイン・アウトアニメーション完了通知.
 *	@param[in]	e_SlideInOutInfo	スライドイン or アウト.
 */
// ===============================================================================================
void PBaseGamenCom::procAnimationEnd( PNLE_SlideInOutInfo e_SlideInOutInfo )
{
	// 現在の画面のWidget
	PBaseWidget* pc_nowGamenWidget = static_cast<PBaseWidget*>( nowGamen->getDisplayParts() );

	// アニメーションタイプチェック.
	if( PNLE_SlideInOutInfo_In == e_SlideInOutInfo ) {
		bl_mmbSlideInFinished = True;		// スライドイン完了フラグON.
	} else if( PNLE_SlideInOutInfo_Out == e_SlideInOutInfo ) {
		bl_mmbSlideOutFinished = True;		// スライドアウト完了フラグON.
	}

	// オブザーバー解除.
	pc_mmbSlideInOutAnimation->detachObserver();

	// アニメーション状態を更新.
	if( e_mmbSlideInOutStatus != PNLE_SlideInOutStatus_Non ) {
		e_mmbSlideInOutStatus = PNLE_SlideInOutStatus_Finish;
	}

	// スライドアニメーション中は非表示としていた画面を表示する.
	pc_nowGamenWidget->show();

	// キャプチャ画面Widgetを非表示.
	c_CaptureBase.hide();
	c_CaptureSlide.hide();

	// キャプチャ画像を解放.
	c_CaptureBase.releaseCaptureImage();
	c_CaptureSlide.releaseCaptureImage();
// comment 2018/07/31 SCD2-Sky東野 [IT6] [1056724] 不具合対応 Start.
	APIC_MultiAutoTest *pc_MultiAutoTest = APIC_MultiAutoTest::newInstance();
	if(NULL != pc_MultiAutoTest){
		// スタートキー押下OKの状態をAPIC_MultiAutoTest : DBGへ通知.
		pc_MultiAutoTest->setPanelStatus_StartKeyOk();
	}
// comment 2018/07/31 SCD2-Sky東野 [IT6] [1056724] 不具合対応 End.
}

// ===============================================================================================
/**
 *	@brief		自画面の初期表示処理.
 *	@param[in]	bl_Show		描画(show())するかどうか（True：する、False：しない）.
 */
// ===============================================================================================
void PBaseGamenCom::initGamen( Bool bl_Show )
{
	// 画面遷移時と同様の、全パーツに対する初期表示処理.
	showEnableAllPars();	// パーツテーブルの全パーツを表示状態とする.
	resetParts();			// パーツ内表示フラグのリセット.
	setDefaultBackParts();	// デフォルト背景パーツを登録.
	startGamen();			// 画面開始処理.
	setupGamen();			// 画面セットアップ処理.
	if( True == bl_Show ) {
		show();				// 画面表示.
	}
}

//================================================================================================
//概要     : サムネイル画像取得完了コールバックによる通知.
//備考     :
//作成者   : 第２電情技 川野.
//レビュー :
//変更履歴 :
//================================================================================================
void PBaseGamenCom::procThumbnailCallBack( uchar option )
{
	// サムネイル・プレビュー管理クラス（仕上り）を取得する.
	PCGC_VerifyThumbnailPreviewControl* pc_VerifyControl = PnlComGetVerifyPreviewControl();
	if( NULL == pc_VerifyControl )
	{
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return;
	}

	// 仕上がりプレビュー時以外の処理.
	if( !( pc_VerifyControl->isStartingVerifyPreview() ) ) {
		// サムネイル画像を表示する.
		c_ThumbnailPreviewControl.showThumbnailImage( option );
		return;
	}

	// サムネイル画像を表示する.
	pc_VerifyControl->showThumbnailImage( option );
}


//==============================================================================
//概要     : プレビュー画像取得完了コールバックによる通知.
//==============================================================================
void PBaseGamenCom::procPreviewCallBack( uchar option )
{
	// サムネイル・プレビュー管理クラス（仕上り）を取得する.
	PCGC_VerifyThumbnailPreviewControl* pc_VerifyControl = PnlComGetVerifyPreviewControl();
	if( NULL == pc_VerifyControl ) {	LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );	return;	}

	// 仕上りプレビュー動作中の場合.
	if( True == pc_VerifyControl->isStartingVerifyPreview() ) {
		switch( option ) {
			case PCGE_ImageDisplayTypeLeftErea:			// 左エリア表示.
			case PCGE_ImageDisplayTypeDocPreview:		// プレビュー表示.
			case PCGE_ImageDisplayTypewRightSmall:		// 右プレビュー(小)表示.
				// プレビュー画像を表示する.
				pc_VerifyControl->showPreviewImage( ( PCGE_ImageDisplayType )option );
				// 画面セットアップ.
				if( pc_VerifyControl->isNeedSetupGamenPreviewCallback() ) {
					setupGamen();
				}
				break;
			default:
				PMessageArea::procPreviewCallBack( option );
				break;
		}
	} else {
		PMessageArea::procPreviewCallBack( option );
	}
}

//================================================================================================.
//	[機能]	アニメーション中判定.
//------------------------------------------------------------------------------------------------.
//	[引数]	なし.
//	[戻値]	Bool	True=アニメーション中 / False=アニメーション中ではない.
//================================================================================================.
Bool PBaseGamenCom::isNowAnimation()
{
	Bool bl_RetCode = False;
	// アニメーション中かどうかを判定する.
	if( ( PNLE_SlideInOutStatus_In     == e_mmbSlideInOutStatus ) ||		// スライドインアニメーション中.
	    ( PNLE_SlideInOutStatus_InReq  == e_mmbSlideInOutStatus ) ||		// スライドインアニメーション要求中.
	    ( PNLE_SlideInOutStatus_Out    == e_mmbSlideInOutStatus ) ||		// スライドアウトアニメーション中.
	    ( PNLE_SlideInOutStatus_OutReq == e_mmbSlideInOutStatus ) ||		// スライドアウトアニメーション要求中.
	    ( True == bl_WaitFlg ) ) {										//Wait中.
		bl_RetCode = True;
	}
	return bl_RetCode;
}

// ===============================================================================================
/**
 *	@brief		画面遷移のアニメーションを強制的に完了させる.
 *	@param		無し.
 *	@retval		無し.
 */
// ===============================================================================================
void PBaseGamenCom::completeGamenAnimationForced()
{
	// スライドアニメーション中の場合アニメーションを停止し画面表示を完了させる.
	if( ( PNLE_SlideInOutStatus_In     == e_mmbSlideInOutStatus ) ||		// スライドインアニメーション中.
	    ( PNLE_SlideInOutStatus_InReq  == e_mmbSlideInOutStatus ) ||		// スライドインアニメーション要求中.
	    ( PNLE_SlideInOutStatus_Out    == e_mmbSlideInOutStatus ) ||		// スライドアウトアニメーション中.
	    ( PNLE_SlideInOutStatus_OutReq == e_mmbSlideInOutStatus ) ) {		// スライドアウトアニメーション要求中.
		// アニメーションを強制終了させる（描画更新はしない）.
		if( NULL != pc_mmbSlideInOutAnimation ) {
			pc_mmbSlideInOutAnimation->forceAnimation( PNLE_ControlType_Last, False );
		}
		if( ( PNLE_SlideInOutStatus_In == e_mmbSlideInOutStatus ) || ( PNLE_SlideInOutStatus_InReq  == e_mmbSlideInOutStatus ) ) {		// スライドインアニメーション中.
			bl_mmbSlideInFinished = False;							// スライドイン完了フラグをOFFに設定.
			e_mmbSlideInOutStatus = PNLE_SlideInOutStatus_Finish;	// スライドイン・アウト状態をアニメーション完了状態に設定.
		} else {														// スライドアウトアニメーション中.
			bl_mmbSlideOutFinished = False;							// スライドアウト完了フラグをOFFに設定.
			e_mmbSlideInOutStatus = PNLE_SlideInOutStatus_Non;		// スライドイン・アウト状態をアニメーション無し状態に設定.
		}
		// キャプチャ画像を非表示とする.
		c_CaptureBase.hide();
		c_CaptureSlide.hide();

		// キャプチャ画像を解放.
		c_CaptureBase.releaseCaptureImage();
		c_CaptureSlide.releaseCaptureImage();
		// Wait中はWait状態を解除する.
	} else if( True == bl_WaitFlg ) {
		setWait( False );
	} else  {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return;
	}
}

//==============================================================================
//概要     : 警告検出通知.
//==============================================================================
void PBaseGamenCom::procDetectWarning( uchar option )
{
	PNLC_WarningManager* pc_WarningManager = PNLC_WarningManager::newInstance();
	if( NULL == pc_WarningManager ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return;
	}
	PMGC_PanelManager* pc_PanelManager = PMGC_PanelManager::newInstance();
	PM_TaskID			taskID = MAX_PM_TASKID;

	pc_PanelManager->getPriority( &taskID );
	// パネルが優先タスクでない場合は、待ち合わせを行う.
	if( PM_TASKID_PANEL != taskID ) {
		// comment 2018/11/30 SCD2-Sky竹菴 [Eagle][1068133] アプリ切替えを遅延させていた場合はここでアプリ切替えを実行する.
		if( True == PNLC_AppChangeStatusFormBrowser::getInstance()->getDelayAppChange() ){
			pc_PanelMain->SendAppChgReqForOut( PM_TASKID_PANEL, PM_APPCHGSTS_START, PM_APPCHGSTS_USER, PM_ACCID_MAX );
// comment 2019/12/4 Sky東島 [EagleZ+][OP_BTS-15824].
			PNLC_AppChangeStatusFormBrowser::getInstance()->setDelayAppChange( False );
		}else{
			m_uc_PopupWaitStatus = PNLD_BASEGAMENCOM_WAITPOPUP_WARN;
			return;
		}
	}

	PBasicMessageProc* p_NowTopInstance = getNowGamen();	// 遷移元画面を設定.	// comment 209_3 2010/2/7 柳川 引数 起動元画面を追加 start.
	PNLC_PopupListControl* pc_PopupListControl = PNLC_PopupListControl::newInstance();
	// ベース画面と表示中のポップアップへのメッセージ振り分け.
	if( NULL != pc_PopupListControl && True == pc_PopupListControl->isPopupOpen() ) {
		p_NowTopInstance = pc_PopupListControl->getNowPopup();
	}																				// comment 209_3 2010/2/7 柳川 引数 起動元画面を追加 end.

	// 警告ポップアップの表示を行う.
	pc_WarningManager->openWarning( m_ss_GamenWarningNo, comPanelJob, p_NowTopInstance );

	// 警告音を鳴動させる.
	setGamenWarningTableBuzzer( m_ss_GamenWarningNo );

	// 保留警告番号を初期化する.
	m_ss_GamenWarningNo = 0;

	return;
}

//==============================================================================
//概要     : ポップアップ設定処理.
//==============================================================================
void PBaseGamenCom::setupOpenPopup( PLayerControl* setGamen, Bool bl_AnimLess )
{
	// ポップアップを保持する.
	m_p_stackPopup = setGamen;

	// アニメーション指定を保持する.
	m_bl_AnimLess = bl_AnimLess;
	return;
}

// comment 2018/10/23 [HCLT_Prem][DenebMLK] [AR1050427] Card Copy Support.
//==============================================================================
//概要     : ポップアップ設定処理.
//備考欄   : [UD指摘No.10]_カードコピー改善対応により、A072002000_トップメニュー(統合メニュー)画面で、カードコピーキー押下後、.
//　　　　　10856500_コピー設定画面にて設定された基本画面に遷移後、2つのポップアップ画面を続けて表示する仕様変更を行った。.
//　　　　　2つ目以降のポップアップについては、配列で管理するようにする。.
//==============================================================================
void PBaseGamenCom::addOpenPopup( PLayerControl* pc_setGamen, Bool bl_AnimLess )
{
	uchar i = 0;
	// ポップアップを保持する.
	if( NULL == m_p_stackPopup) {
		m_p_stackPopup = pc_setGamen;
		m_bl_AnimLess  = bl_AnimLess;
	} else {
		for( i = 0; i < PNLD_BASEGAMENCOM_ADDPOPUP_NUM; i++ ) {
			if( NULL == m_st_AddStackPopup[i].pc_AddStackPopup ) {
				m_st_AddStackPopup[i].pc_AddStackPopup    = pc_setGamen;
				m_st_AddStackPopup[i].bl_AddStackAnimLess = bl_AnimLess;
				return;
			}
		}
	}
}
//==============================================================================
//概要     : ポップアップ表示処理.
//==============================================================================
void PBaseGamenCom::procOpenPopup( uchar option )
{
	PNLC_PopupListControl* pc_PopupListControl = PNLC_PopupListControl::newInstance();
	PMGC_PanelManager* pc_PanelManager = PMGC_PanelManager::newInstance();
	PM_TaskID			taskID = MAX_PM_TASKID;
// comment 2018/10/23 [HCLT_Prem][DenebMLK] [AR1050427] Card Copy Support.
	uchar i = 0;
	pc_PanelManager->getPriority( &taskID );
	
// comment 2015/12/03 OPS1-MSE天木 [PZH2.1c][CQ522648]ブラウザでbizhubRAしてファクススキャンに遷移するときの問題対応.
	if(PM_TASKID_PANEL != taskID) {
		// comment 2018/11/30 SCD2-Sky竹菴 [Eagle][1068133] アプリ切替えを遅延させていた場合はここでアプリ切替えを実行する.
		if( True == PNLC_AppChangeStatusFormBrowser::getInstance()->getDelayAppChange() ){
			pc_PanelMain->SendAppChgReqForOut( PM_TASKID_PANEL, PM_APPCHGSTS_START, PM_APPCHGSTS_USER, PM_ACCID_MAX );
// comment 2019/12/4 Sky東島 [EagleZ+][OP_BTS-15824].
			PNLC_AppChangeStatusFormBrowser::getInstance()->setDelayAppChange( False );
// comment 2022/06/22 Sky東島 [IT6_2.3][制御構成変更_No.51][サービスレスメンテナンス].
		} else if( &gp_PopupBrowserWait_popup == m_p_stackPopup ) {
			// コンテンツ表示待機中画面の場合は優先タスクがパネルでなくても表示.
			pc_PopupListControl->addPopup( m_p_stackPopup, this, NULL, 400, 240, 0, 0, True );
			m_p_stackPopup = NULL;
			return;
		}else{
			if ( &gp_PublicPopupDummyBase == nextGamen ) { 
				// ダミー画面に遷移できていない場合は、ダミー画面に対して待ち合わせの設定を行う.
				// IGCWaitがQtのメッセージループでタイマ処理しており、
				// ダミー画面未起動時にMS_OpenPopupメッセージ要求される場合があるため.
				gp_PublicPopupDummyBase.setPopupWaitStatus( PNLD_BASEGAMENCOM_WAITPOPUP_PUBLIC );
			} else {
				// パネルが優先タスクでない場合は、待ち合わせを行う.
				m_uc_PopupWaitStatus = PNLD_BASEGAMENCOM_WAITPOPUP_PUBLIC;
			}
			return;
		}
	}

	// ポップアップを表示できる場合.
	if( ( NULL != pc_PopupListControl ) && ( NULL != m_p_stackPopup ) ) {
		if( &gp_HomeTopMenu == m_p_stackPopup ) {  // トップメニューの場合.
// comment 2019/05/15 Sky東島 [EagleHigh][OP_BTS-6050].
			if( True == pc_PopupListControl->isPopupLayer( &gp_HomeTopMenu ) ){
				// すでにトップメニューが開かれている場合は何もしない.
				m_p_stackPopup = NULL;
				return;
			}
//comment 2015/03/02 OPS1-Sky稲井 [特ROM対応][MZV-35_機種マージ] IWSトップメニューを表示する.
			if (True == APIC_SoftDipSw::newInstance()->getSoftDipSwBitForPanelSpecNo( DEFSOFTWARESW165, 0 )) {
				if ( PNLD_StandardTopMenu == gp_HomeTopMenu.getTopMenuMode() ) {
			short _ss_ScanFaxProgramNo = pc_ProgramKeyDataGetForTopMenu.getProgramNoForTopMenu( 0 );
			pc_ProgramKeyDataGetForTopMenu.clearProgramGetNum();
			if( 0 < _ss_ScanFaxProgramNo ) {
				pc_ProgramKeyDataGetForTopMenu.requestTopMenuProgramKey( _ss_ScanFaxProgramNo );
				pc_ManageIGCWaitDisplay.initManageIGCWaitDisplay( PCGD_IGCDisplayStandard, PCGE_IGCClearPopupDispFlgAdd, &gp_HomeTopMenu );
				nextWaitDisplay_popup( False );	// 透明画面を表示しない.
			} else {
// comment 2018/11/05 SCD2-Sky肥後 [EagleH] WebUIv3FUM対応.
//				PNLC_PopupListControl::newInstance()->addPopup( &gp_HomeTopMenu, NULL, NULL, 0, 0, 0, 0, True );	// A072002000_トップメニュー（統合トップメニュー）.
				PnlComChangeScreenHome();
			}
				} else {
					gp_HomeTopMenu.showIWSTopMenu();
				}
				gp_HomeTopMenu.setTopMenuMode( PNLD_IWSTopMenu );
			} else {
				short _ss_ScanFaxProgramNo = pc_ProgramKeyDataGetForTopMenu.getProgramNoForTopMenu( 0 );
				pc_ProgramKeyDataGetForTopMenu.clearProgramGetNum();
// comment 2019/04/30 [HCLT_Madhavi][IT6-EagleH] [OP_BTS_5721] Launching Classic style HomeTop Menu screen when classic style button is pressed.
	//			if ( 0 < _ss_ScanFaxProgramNo ) {
// comment 2022/04/28 Sky中野 [IT6_2.3] 制御No.43_システムオートリセット改善(WebUIv2/v3統合).
//				if ( ( 0 < _ss_ScanFaxProgramNo ) && PNLE_BrowserChangeTabMode_Home != PNLC_BrowserChangeTabControl::getInstance()->getBrowserChangeTabMode()) {
				if ( ( 0 < _ss_ScanFaxProgramNo )
				  && ( PNLE_WebUIFunctionHome != PNLC_WebUIControl::getInstance()->getDispWebUIScreen() ) ) {
					pc_ProgramKeyDataGetForTopMenu.requestTopMenuProgramKey( _ss_ScanFaxProgramNo );
					pc_ManageIGCWaitDisplay.initManageIGCWaitDisplay( PCGD_IGCDisplayStandard, PCGE_IGCClearPopupDispFlgAdd, &gp_HomeTopMenu );
					nextWaitDisplay_popup(False);	// 透明画面を表示しない.
				} else {
// comment 2018/11/05 SCD2-Sky肥後 [EagleH] WebUIv3FUM対応.
//					PNLC_PopupListControl::newInstance()->addPopup( &gp_HomeTopMenu, NULL, NULL, 0, 0, 0, 0, True );	// A072002000_トップメニュー（統合トップメニュー）.
					PnlComChangeScreenHome();
				}
			}
			// メニューキー受付け対応.
		} else {
// comment 2018/10/23 [HCLT_Prem][DenebMLK] [AR1050427] Card Copy Support.
			// 機能バッジを非表示にする.
// comment 2018/10/26 [HCLT_Neelima][DenebMLK] Jenkin build errors.			
//			PPopupCom* pc_PopupCom = m_p_stackPopup;
			PPopupCom* pc_Popup = dynamic_cast< PPopupCom* >( m_p_stackPopup );
			if ( NULL != pc_Popup ) {
//			pc_PopupCom->setFunctionBadgeShowFlg( False );
				pc_Popup->setFunctionBadgeShowFlg( False );
				// ポップアップを表示する.
				pc_PopupListControl->addPopup( m_p_stackPopup, this, NULL, 400, 240, 0, 0, m_bl_AnimLess );
// comment 2018/10/23 [HCLT_Prem][DenebMLK] [AR1050427] Card Copy Support.
				for( i = 0; i < PNLD_BASEGAMENCOM_ADDPOPUP_NUM; i++ ) {
					if( NULL != m_st_AddStackPopup[i].pc_AddStackPopup ) {
// comment 2018/10/26 [HCLT_Neelima][DenebMLK] Jenkin build errors.	
//						pc_PopupCom = m_st_AddStackPopup[i].pc_AddStackPopup;
						pc_Popup = dynamic_cast< PPopupCom* >( m_st_AddStackPopup[i].pc_AddStackPopup ); 
						if ( NULL != pc_Popup ) {
// comment 2018/10/26 [HCLT_Neelima][DenebMLK] Jenkin build errors.	
//					pc_PopupCom->setFunctionBadgeShowFlg( False );
							pc_Popup->setFunctionBadgeShowFlg( False );
							pc_PopupListControl->addPopup( m_st_AddStackPopup[i].pc_AddStackPopup, this, NULL, 400, 240, 0, 0, m_st_AddStackPopup[i].bl_AddStackAnimLess  );
						}
					}
				}
			}
// comment 2018/10/26 [HCLT_Neelima][DenebMLK] Jenkin build errors.	
			pc_Popup = NULL;
		}
		m_p_stackPopup = NULL;
// comment 2018/10/23 [HCLT_Prem][DenebMLK] [AR1050427] Card Copy Support.
		for( i = 0; i < PNLD_BASEGAMENCOM_ADDPOPUP_NUM; i++ ) {
			m_st_AddStackPopup[i].pc_AddStackPopup    = NULL;
			m_st_AddStackPopup[i].bl_AddStackAnimLess = False;
		}
	}

	return;
}

//==============================================================================
//概要     : 警告取得処理.
//==============================================================================
Bool PBaseGamenCom::getWarning( void )
{
	PNLC_WarningManager* pc_WarningManager = PNLC_WarningManager::newInstance();
	if( NULL == pc_WarningManager ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return False;
	}
	// 警告ステータスを取得する.
// comment 2015/10/12 MSE高橋 [MinervaSBK][CQ519743] 表示中ジョブより警告確認するように修正.
//	if( NULL != comPrintJob && comPanelJob != comPrintJob )
//	{
//		pc_WarningManager->getWarningStatus( m_ss_GamenWarningNo, comPrintJob );
//	} else {
//		pc_WarningManager->getWarningStatus( m_ss_GamenWarningNo, comPanelJob );
//	}

	if( NULL != onlyPanel.cc_DisplayJob ){
		pc_WarningManager->getWarningStatus( m_ss_GamenWarningNo,  onlyPanel.cc_DisplayJob );
	}
	

	// 警告ありならTrueを返却、無しならFalseを返却する.
	if( 0 != m_ss_GamenWarningNo ) {
		return True;
	} else {
		return False;
	}
}

//==============================================================================
//概要     : 警告検出通知処理.
//==============================================================================
void PBaseGamenCom::sendDetectWarningIndi( void )
{
	DetectWarningIndi();

	return;
}

//==============================================================================
//概要     : ポップアップ表示通知処理.
//==============================================================================
void PBaseGamenCom::sendOpenPopupIndi( void )
{
	OpenPopupIndi();

	return;
}

//==============================================================================.
//概要     : ポップアップ予約があるか確認する.
//==============================================================================.
Bool PBaseGamenCom::isOpenPopup()
{
	Bool bl_isPopup = False;
	if( NULL != m_p_stackPopup ) {
		bl_isPopup = True;
	}
	return bl_isPopup;
}

//==============================================================================
//概要     : パネル切替時のベース画面描画要求.
//==============================================================================
void PBaseGamenCom::procChgBaseGamen( uchar option )
{
	// 親クラスをコールし、ベース画面を再描画する.
	PLayerControl::procChgBaseGamen( option );

	// ポップアップ表示を待機させている場合は、処理を行う.
	switch( m_uc_PopupWaitStatus ) {
		// 通常ポップアップ表示待ちの場合.
		case PNLD_BASEGAMENCOM_WAITPOPUP_PUBLIC:
			procOpenPopup( option );
			break;
		// 警告ポップアップ表示待ちの場合.
		case PNLD_BASEGAMENCOM_WAITPOPUP_WARN:
			procDetectWarning( option );
			break;
		// 上記以外.
		default:
			break;
	}
	// 待ち状態を元に戻す.
	m_uc_PopupWaitStatus = PNLD_BASEGAMENCOM_WAITPOPUP_NON;

	return;
}
//==============================================================================.
//概要     : ベース画面共通 : ブラウザキー.
//==============================================================================.
void PBaseGamenCom::procBrowser( uchar option )
{
	// MFP基本機能使用設定が無効の場合.
	if( False == c_ExtPanel.isEnableMFPFunction() ) {
		// キーを受け付けない.
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}

	// 親クラスを呼び出す.
	PLayerControl::procBrowser( option );
}

//==============================================================================.
//概要     : 起動時画面開始処理.
//==============================================================================.
void PBaseGamenCom::preStartGamen()
{
	// 画面インスタンス生成.
	PBasicMessageProc::preStartGamen();
	
}
//==============================================================================.
//概要     : 起動時画面描画処理.
//==============================================================================.	
void PBaseGamenCom::preDrawGamen()
{
	// 画面空描画.
	PBaseWidget* bw = 0;
	
	PDisplayParts* widgetParts = getDisplayParts();
	if( widgetParts != 0 ) 
	{
		bw = dynamic_cast<PBaseWidget*>( widgetParts );
	}	
	
	if( bw != 0 ) 
	{
		this->show();

// Comment 2019/08/10 [HCLT-Catherine][IT6_HeliosMLK][Seigyu.10 ]Compilation switch activity.
#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus) && defined (DEF_LCD_7INCH) ) )
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. Start.
// comment 2018/10/12 Sky東島 [1062683] 2ペイン化対応.
//		if ( GFWE_LCD_PANE_1 == getGamenPane( (PBasicMessageProc*)this ) ) {
		GFWE_LCD_PANE e_LCDPane = getGamenPane( (PBasicMessageProc*)this );
		if( ( GFWE_LCD_PANE_1 == e_LCDPane )
		 || ( GFWE_LCD_PANE_2 == e_LCDPane )
		 || ( GFWE_LCD_PANE_1_Extention == e_LCDPane ) ) {
			// １ペイン,2ペイン,1ペイン(拡大表示)の場合.
			PBaseWidget dummyWidget(		// 画面キャプチャ用Widget(ベース画面用).
			    m00_076_000_X1024_Y600		// const uchar* puc_Parts
				, 0							// short tx
				, 0							// short gy
				, 0							// uchar uc_PaletteType
				, TYPE_LayoutFree			// TYPE_Layout e_LayoutX = TYPE_LayoutFree
				, False						// Bool centeringY = False
				, 0							// int red = 0
				, 0							// int green = 0
				, 0							// int blue = 0
				, 0							// int alpha = 0
				, GFWE_LCD_Size_10_1inch	// GFWE_LCD_Size e_Pos = GFWE_LCD_Size_9inch
				, GFWE_LCD_Size_10_1inch	// GFWE_LCD_Size e_Size = GFWE_LCD_Size_9inch )
			);
			dummyWidget.startGUI( 0 ); //PNLC_PopupListControl::createCaptureParts()参考
			dummyWidget.hide();
			dummyWidget.preDraw( bw ); // paintEventをトリガさせGFW内のキャッシュを生成する.		
		} else {
			// １ペイン以外の場合.
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. End.
#endif // #if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !defined (DEF_HeliosMLKCus) )	// Eagle/SparrowMFP 10.1inch.
		PBaseWidget dummyWidget( c71_000_000_t1_X800_Y480, 0, 0, 0, TYPE_LayoutFree, False );		// 画面キャプチャ用Widget(ベース画面用).
		dummyWidget.startGUI( 0 ); //PNLC_PopupListControl::createCaptureParts()参考
		dummyWidget.hide();
		dummyWidget.preDraw( bw ); // paintEventをトリガさせGFW内のキャッシュを生成する.		
// Comment 2019/08/10 [HCLT-Catherine][IT6_HeliosMLK][Seigyu.10 ]Compilation switch activity.
#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus) && defined (DEF_LCD_7INCH) ) )
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. Start.
		}
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. End.
#endif // #if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
		this->hide();
	}
}


