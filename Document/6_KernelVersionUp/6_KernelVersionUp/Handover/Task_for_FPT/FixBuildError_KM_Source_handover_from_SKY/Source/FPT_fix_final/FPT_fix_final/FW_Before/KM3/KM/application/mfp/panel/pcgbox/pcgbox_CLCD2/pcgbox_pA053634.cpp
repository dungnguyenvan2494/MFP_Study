//-----------------------------------------------------------------------------------------------
/**
 * @file pcgbox_pA053634.cpp
 * @brief 画面クラスの実装（ポップアップ）.
 */
//-----------------------------------------------------------------------------------------------
/*================================================================================================
	インクルード・ファイル
================================================================================================*/
#include <panel/plibh/plibh_CLCD/plibh_Mtui.h>
#include <system/saa/APIC_UsbMemDeviceInfo.h>
#include <system/prv/APIC_PreviewConverter.h>
#include <system/ats/APIC_AutoSelect.h>
#include <system/job/APIC_Job.h>
#include <system/nvd/APIC_Setting.h>
#include <system/sys/APIC_SystemServiceFunction.h>
#include <panel/plibh/plibh_CLCD2/PNLC_StackManJobSpec.h>

#include <panel/pcgbox/pcgbox_CLCD2/pcgbox_pA053634.h>
#include <panel/pcgbox/pcgbox_CLCD2/pipbox_pA053634.h>

#include <panel/pcgcom/pcgcom_CLCD2/PNLC_PopupListControl.h>
#include <panel/plibh/plibh_CLCD2/PNLC_BoxDataLib.h>
#include <panel/plibh/plibh_CLCD2/PNLC_BoxNin1ControlLib.h>
#include <panel/plibh/plibh_CLCD2/PNLC_ScanFaxDataChangeCheckLib.h>
#include <panel/plibh/plibhg/plibh.h>
#include <panel/plibh/plibhg/plibh3.h>
#include <panel/pcgcom/pcgcomg/pcgcom.h>
#include <panel/pcgcom/pcgcom_CLCD2/PPopupAnimationCom.h>
#include <panel/plibh/plibh_CLCD2/PNLC_BoxDataLib.h>
#include <panel/plibh/plibh_CLCD2/PNLC_BoxDocInfoControlLib.h>
#include <panel/plibh/plibh_CLCD2/PNLC_BoxDocumentListInfoControlLib.h>
#include <panel/plibh/plibh_CLCD2/piglib_pA010194.h>
#include <panel/plibh/plibh_CLCD2/PNLC_OutsideKeyCallDataLib.h>
#include <panel/plibh/plibh_CLCD2/PNLC_SlideMenuControlLib.h>
#include "panel/plibh/plibh_CLCD2/PNLC_SoftSlideMenuTenKeyTextDispLib.h"
// comment 2015/07/23 OPS1-Sky浜野 [MinervaSBK][制御構成変更No.75] パスワード入力文字表示対応.
#include "panel/plibh/plibh_CLCD2/PNLC_TimerControlLib.h"


// comment 2015/07/23 OPS1-Sky浜野 [MinervaSBK][制御構成変更No.75] パスワード入力文字表示対応.
short PCPC_FaxSendCommSetFCode_box::m_ss_PassInsertPos = -1;

//関数------------------------------------------------------------------------------------------------------------------
//概要    ：画面開始処理.
//備考    ：.
//戻値    ：.
//作成者  ：AWS_バル.
//変更履歴：.
//------------------------------------------------------------------------------
/**
 * @brief A053634000 Fコード送信 : 画面開始処理
 */
//------------------------------------------------------------------------------
void PCPC_FaxSendCommSetFCode_box::startGamen()
{
	PNLC_OutSideKeyCallDataLib::newInstance()->setSelectNo( PNLE_Soft10KeyUnselected );			// データクリア.

	PPopupDecisionButtonCom::startGamen();		// 親クラスをコールする.

	if( NULL == PnlComGetJobScanFax() ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }
	// Fax用ジョブスペックインスタンスの中にあるFax関連パラメータの先頭アドレスを取得
	TYPC_FaxAct* pc_FaxAct = &( PnlComGetJobScanFax()->getJobSpec()->pc_Fax->c_FaxAct );
	if( NULL == pc_FaxAct ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }

	// ポップアプメッセージの初期化処理.
	Cmp_FaxSendCommSetFCode_boxPopupMessageArea.setOnOffButton( True );
	Cmp_FaxSendCommSetFCode_boxPopupMessageArea.showDestinationCount( False );
	Cmp_FaxSendCommSetFCode_boxPopupMessageArea.changeMessageDisplay( PNLE_PopupMessageAreaMessageHideAll );
	Cmp_FaxSendCommSetFCode_boxPopupMessageArea.setMode( PNLE_PopupMessageAreaMessageModeBox );
	Cmp_FaxSendCommSetFCode_boxPopupMessageArea.setNowPopupGamen( PNLE_BoxFCodeTransmission );
	Cmp_FaxSendCommSetFCode_boxPopupMessageArea.attachMessage( True );	// ポップアップメッセージから画面へキーイベントを通知するようにする.	//【CQ_346758】12m.hosokawa add.

	// 親展送信又は中継依頼送信の場合
	if( ( pc_FaxAct->getTxMode() == TYPE_TxModeMailbox )
	    || ( pc_FaxAct->getTxMode() == TYPE_TxModeRelayReq ) ) {
		Cmp_FaxSendCommSetFCode_boxPopupMessageArea.setOnOffSwitchButtonState( True );
	} else {
		Cmp_FaxSendCommSetFCode_boxPopupMessageArea.setOnOffSwitchButtonState( False );
	}

	ss_mFaxSendCommSetFCodeBackUpJobTableNo = -1;
	PNLC_CopyDataLib::newInstance()->JobSpecBackupAllClear();

	// 機能バッジをTrueにセットする.
	b_FunctionBadge = True;

	m_bl_isSelectedInputField = PnlComIsHard10Key() ? True : False;

	// 画面開始処理共通化.
	startGamenSetup();

	// 入力エリア押下機能有無.
	if( True != PLBC_FuncVersionCheck::isOK_Type4() ) {
		Key_FaxSendCommSetFCode_boxSubAddressArea.setShowPush( False );
		Key_FaxSendCommSetFCode_boxPasswordArea.setShowPush( False );
	}
}

//関数------------------------------------------------------------------------------------------------------------------
//概要    ：表示部品のセットアップ.
//備考    ：.
//戻値    ：.
//作成者  ：AWS_バル.
//変更履歴：.
//------------------------------------------------------------------------------
/**
 * @brief A053634000 Fコード送信 : 表示部品のセットアップ
 */
//------------------------------------------------------------------------------
void PCPC_FaxSendCommSetFCode_box::setupGamen( short option )
{
	PPopupDecisionButtonCom::setupGamen( option );

	PMessageArea::setupPrintLEDFax();
	if( True == b_FunctionBadge ) {
		Key_FuncBadge_FaxSendCommSetFCode_box.chgOff();
		PNLC_FunctionBadgeLib::newInstance()->chgIconAndText( PNLE_APPTYPE_BOX, PNLE_FUNCTIONTYPE_BOXTXNFCODESEND, &Key_FuncBadge_FaxSendCommSetFCode_box );
		Key_FuncBadge_FaxSendCommSetFCode_box.hideMarginText();
		b_FunctionBadge = False;
	}

	setupGamenSetup();		//画面開始処理共通化.
	short ss_SelectTotalAddressCount = 0;		//初期化.
	if( NULL == PnlComGetJobScanFax() ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }
	ss_SelectTotalAddressCount = PnlComGetJobScanFax()->getJobSpec()->pc_Send->c_ComSend.getSelectTotalAddressCount();

	//選択中のアドレス情報の初期化.
	if( ss_SelectTotalAddressCount == 0 ) {
		PPCC_SelectAddressInfo* pc_SelectAddressInfo;
		if( True == onlyPanel.b_nowBoxMode ) {		//モードはボックス？.
			pc_SelectAddressInfo = &c_BoxSelectAddressInfo;		//pc_SelectAddressInfoをセットする.
		} else if( ( TYPE_JobTypeScan == comPanelJob->getJobSpec()->pc_Common->c_JobType.getJobType() )		//ジョブは送信？.
		           || ( TYPE_JobTypeFaxSendQuick == comPanelJob->getJobSpec()->pc_Common->c_JobType.getJobType() )
		           || ( TYPE_JobTypeFaxSendMemory == comPanelJob->getJobSpec()->pc_Common->c_JobType.getJobType() )
		           || ( TYPE_JobTypeFaxSendTimer == comPanelJob->getJobSpec()->pc_Common->c_JobType.getJobType() )
		           || ( TYPE_JobTypeFaxReceivePolling == comPanelJob->getJobSpec()->pc_Common->c_JobType.getJobType() )
		           || ( TYPE_JobTypeFaxReceiveTimer == comPanelJob->getJobSpec()->pc_Common->c_JobType.getJobType() )
		           || ( TYPE_JobTypeFaxReceive == comPanelJob->getJobSpec()->pc_Common->c_JobType.getJobType() ) ) {
			pc_SelectAddressInfo = &c_ScanSelectAddressInfo;		//pc_SelectAddressInfoをセットする.
		} else {
			// ジョブタイプがすでに切り替わっている場合.
			return;
		}
		pc_SelectAddressInfo->initializeSelectAddressInfo();
	}

	updateFuncBadgeButtonStatus();

	short Test_CurX;
	Ctf_FaxSendCommSetFCode_boxSubAddress.getCursorPosition( Test_CurX );

	sendSlide10KeyData();

}

//関数------------------------------------------------------------------------------------------------------------------
//概要    ： 機能バッジ更新関数(ポップアップ側機能バッジ).
//備考    ： .
//戻値    ： 無し.
//レビュー： .
//変更履歴： .
//------------------------------------------------------------------------------
/**
 * @brief A053634000 Fコード送信 : 機能バッジ更新関数(ポップアップ側機能バッジ).
 */
//------------------------------------------------------------------------------
void PCPC_FaxSendCommSetFCode_box::updateFuncBadgeButtonStatus()
{
	PNLC_FunctionBadgeLib::newInstance()->chgIconAndText( PNLE_APPTYPE_BOX, PNLE_FUNCTIONTYPE_BOXTXNFCODESEND, &Key_FuncBadge_FaxSendCommSetFCode_box );
}

//関数------------------------------------------------------------------------------------------------------------------
//概要    ：画面開始処理共通化.
//備考    ：.
//戻値    ：.
//作成者  ：AWS_バル.
//変更履歴：.
//------------------------------------------------------------------------------
/**
 * @brief A053634000 Fコード送信 : 画面開始処理共通化
 */
//------------------------------------------------------------------------------
void PCPC_FaxSendCommSetFCode_box::startGamenSetup()
{
	if( NULL == PnlComGetJobScanFax() ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }
	// Fax用ジョブスペックインスタンスの中にあるFax関連パラメータの先頭アドレスを取得.
	TYPC_FaxAct* pc_FaxAct = &( PnlComGetJobScanFax()->getJobSpec()->pc_Fax->c_FaxAct );

	if( NULL == pc_FaxAct ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }

	us_FcodeFlg = PNLE_FaxSendCommSetFCode_boxInputKeyNonSel;
	memset( sc_LocalString1, '\0', sizeof( sc_LocalString1 ) );
	memset( sc_LocalString2, '\0', sizeof( sc_LocalString2 ) );
	uc_SubCursorPos = 0;
	b_FunctionOffOnFlg = True;
	// 入力タイプ（SUBアドレス）.
	b_SubInputDirect = True;
	// 入力タイプ（パスワード）.
	b_PassInputDirect = True;
	// カーソル非表示(SUBアドレス) .
	e_SubCursorFlg = PNLE_BoxCursorPosDELETE;
	// カーソル非表示(パスワード) .
	e_PassCursorFlg = PNLE_BoxCursorPosDELETE;
	// パスワード設定済み用文字列（「**********」）表示 or 非表示状態に「False（＝非表示）」を設定 .
	b_isStringForFinishingPwdDisplay = False;

	// 親展送信又は中継依頼送信?.
	if( ( pc_FaxAct->getTxMode() == TYPE_TxModeMailbox )
	    || ( pc_FaxAct->getTxMode() == TYPE_TxModeRelayReq ) ) {
		b_FunctionOffOnFlg = True;
		// SUBアドレス選択 .
		us_FcodeFlg = PNLE_FaxSendCommSetFCode_boxInputKeySubAddress;
		// SUBアドレス前回値取得.
		pc_FaxAct->getTxSubAddr( sc_LocalString1 );
		// パスワード前回値取得.
		pc_FaxAct->getTxLocalPwd( sc_LocalString2 );
		// カーソル位置設定.
		uc_SubCursorPos = ( uchar )strlen( sc_LocalString1 );
		// カーソル表示(SUBアドレス).
		if( True == m_bl_isSelectedInputField ) {
			e_SubCursorFlg = PNLE_BoxCursorPosSET;					// OUTPUT：カーソル表示(SUBアドレス) .
		}
		// パスワードが設定されている?.
		if( sc_LocalString2[0] != '\0' ) {
			// パスワード設定済み用文字列（「**********」）表示 or 非表示状態に「True（＝表示）」を設定 .
			b_isStringForFinishingPwdDisplay = True;
		}
	}
	if( True == PnlComIsHard10Key()
	    &&	PNLE_FaxRbtSel1761InputKeySubAddress != us_FcodeFlg
	    &&	PNLE_FaxRbtSel1761InputKeyPassword != us_FcodeFlg ) {
		us_FcodeFlg = PNLE_FaxRbtSel1761InputKeySubAddress;		// OUTPUT： SUBアドレス選択 .
	}
}

//関数------------------------------------------------------------------------------------------------------------------
//概要    ：画面開始処理共通化.
//備考    ：.
//戻値    ：.
//作成者  ：AWS_パトリック.
//レビュー：.
//変更履歴：.
//------------------------------------------------------------------------------
/**
 * @brief A053634000 Fコード送信 : 画面開始処理共通化
 */
//------------------------------------------------------------------------------
void PCPC_FaxSendCommSetFCode_box::setupGamenSetup()
{
	char sc_worktext[PNLD_MaxFaxTxLocalPwd+1];	// ワーク文字列
	ushort us_StrLen = 0; 						// 文字数

	memset( sc_worktext, '\0', sizeof( sc_worktext ) );

	// ＳＵＢアドレス表示
	CursorDisp( &Ctf_FaxSendCommSetFCode_boxSubAddress, ( char* )sc_LocalString1, PNLD_MaxFaxTxSubAddrInput, e_SubCursorFlg, uc_SubCursorPos );
	// フラグの初期化

	e_SubCursorFlg = PNLE_BoxCursorPosNONE;

	// パスワード表示エリア文字列判定
	if( b_isStringForFinishingPwdDisplay == True ) {
		// パスワード設定済み用文字列（「**********」）表示 or 非表示状態が「True（＝表示）」の場合
		// パスワード設定済みであることを表す文字列（「**********」）を表示させる
// comment 2018/07/16 HCL_HariBhaskar [Eagle] Klockwork Changes ID:2525694 [ SV.STRBO.UNBOUND_COPY ].
//		strcpy( sc_worktext, sc_FaxStringForFinishingPwd );
		strncpy( sc_worktext, sc_FaxStringForFinishingPwd,( sizeof ( sc_worktext ) -1 ) );
		// パスワード用カーソル位置設定
		uc_PassCursorPos = ( uchar )strlen( sc_FaxStringForFinishingPwd );
	} else {
		// パスワード設定済み用文字列（「**********」）表示 or 非表示状態が「False（＝非表示）」の場合
		// パスワード「＊」表示文字作成
		us_StrLen = ( ushort )strlen( sc_LocalString2 );
		memset( sc_worktext, '*', us_StrLen );
// comment 2015/07/23 OPS1-Sky浜野 [MinervaSBK][制御構成変更No.75] パスワード入力文字表示対応.
		for( ushort i = 0; i < us_StrLen; i++ ) {
			// パスワード入力文字を表示する場合.
			if( 0 <= m_ss_PassInsertPos ) {
				// 入力文字は、そのまま表示する.
				if( i == m_ss_PassInsertPos ) {
					strncpy( &sc_worktext[i], (char*)(&sc_LocalString2[i]), 1 );
					continue;
				}
			}
			sc_worktext[ i ] = '*';
		}
		sc_worktext[ i ] = '\0';
	}

	// パスワード表示
	CursorDisp( &Ctf_FaxSendCommSetFCode_boxPassword, ( char* )sc_worktext, PNLD_MaxFaxTxFcodePwdInput, e_PassCursorFlg, uc_PassCursorPos );
	// フラグの初期化
	e_PassCursorFlg = PNLE_BoxCursorPosNONE;


	Rbt_FaxSendCommSetFCode_box.select( us_FcodeFlg );

	b_FunctionOffOnFlg = Cmp_FaxSendCommSetFCode_boxPopupMessageArea.getOnOffSwitchButtonState();

	Rbt_FaxSendCommSetFCode_box.activate( b_FunctionOffOnFlg );

	if( False == b_FunctionOffOnFlg ) {
		Txt_FaxSendCommSetFCode_boxSubAddress.changeTextColor( UC_GPARTS_COLOR_GRAY3 );
		Txt_FaxSendCommSetFCode_boxPassword.changeTextColor( UC_GPARTS_COLOR_GRAY3 );
		Key_FaxSendCommSetFCode_boxSubAddressArea.activate( False );
		Key_FaxSendCommSetFCode_boxPasswordArea.activate( False );
		Grp_FaxSendCommSetFCode_boxSubAddressControl.activate( b_FunctionOffOnFlg );
		Grp_FaxSendCommSetFCode_boxPasswordControl.activate( b_FunctionOffOnFlg );
	}

	// ＳＵＢアドレス、パスワードローテーションボタン表示処理

	if( ( PNLE_FaxSendCommSetFCode_boxInputKeySubAddress == us_FcodeFlg )
	    &&( True == b_FunctionOffOnFlg ) ) {

		Key_FaxSendCommSetFCode_boxSubAddressArea.activate( False );
		Key_FaxSendCommSetFCode_boxSubAddressArea.activate( True );
		Key_FaxSendCommSetFCode_boxPasswordArea.activate( False );
		Txt_FaxSendCommSetFCode_boxPassword.changeTextColor( UC_GPARTS_COLOR_GRAY3 );
		Txt_FaxSendCommSetFCode_boxSubAddress.returnTextColor();
		Grp_FaxSendCommSetFCode_boxPasswordControl.activate( False );
		Grp_FaxSendCommSetFCode_boxSubAddressControl.activate( True );
		if( True == m_bl_isSelectedInputField ) {
			if( True == b_BlinkOn ) {
				Ctf_FaxSendCommSetFCode_boxSubAddress.setCursorOption( UC_CURSOR_OPTION_OFF );
			} else {
				Ctf_FaxSendCommSetFCode_boxSubAddress.setCursorOption( UC_CURSOR_OPTION_ON );
			}
		}
	} else if( ( PNLE_FaxSendCommSetFCode_boxInputKeyPassword == us_FcodeFlg )
	           &&( True == b_FunctionOffOnFlg ) ) {

		Key_FaxSendCommSetFCode_boxSubAddressArea.activate( False );
		Key_FaxSendCommSetFCode_boxPasswordArea.activate( False );
		Key_FaxSendCommSetFCode_boxPasswordArea.activate( True );
		Txt_FaxSendCommSetFCode_boxSubAddress.changeTextColor( UC_GPARTS_COLOR_GRAY3 );
		Txt_FaxSendCommSetFCode_boxPassword.returnTextColor();
		Grp_FaxSendCommSetFCode_boxSubAddressControl.activate( False );
		Grp_FaxSendCommSetFCode_boxPasswordControl.activate( True );
		if( True == m_bl_isSelectedInputField ) {
			if( True == b_BlinkOn ) {
				Ctf_FaxSendCommSetFCode_boxPassword.setCursorOption( UC_CURSOR_OPTION_OFF );
			} else {
				Ctf_FaxSendCommSetFCode_boxPassword.setCursorOption( UC_CURSOR_OPTION_ON );
			}
		}
	}
	b_BlinkOn = ( b_BlinkOn ? False : True );				// comment Minerva ソフトテンキー対応 2010/11/19 Sky西原.
	if( ( True == b_FunctionOffOnFlg )
	    &&( '\0' == sc_LocalString1[0] ) ) {
		Key_FaxSendCommSetFCode_boxOK.activate( False );
	} else {
		Key_FaxSendCommSetFCode_boxOK.activate( True );
	}
}

//関数-------------------------------------------------------------------------
//概要    ：カーソル表示処理.
//備考    ：.
//戻値    ：.
//作成者  ：AWS_パトリック.
//レビュー：.
//変更履歴：.
//------------------------------------------------------------------------------
/**
 * @brief A053634000 パスワード送信 : カーソル表示処理
 */
//------------------------------------------------------------------------------
void PCPC_FaxSendCommSetFCode_box::CursorDisp( PCursorTextField* Ctf_TextField,			// 表示するカーソル付きテキスト .
        const char* sc_SetString,							// テキストの表示配列 .
        const short ss_TextMaxSize,							// 表示領域のＭＡＸ値 .
        PNLE_BoxCursorPos e_CursorPosFlg,					// カーソル移動の種別 .
        const PARTS_POS uc_CursorPos )						// カーソル指定時のカーソル位置.
{
	if( NULL == Ctf_TextField || NULL == sc_SetString ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }
	// 文字フォントの設定は未確定のため暫定でSJIS固定 .
	// テキスト表示 .
	Ctf_TextField->changeExternalText( ( const uchar* )sc_SetString, 1, ss_TextMaxSize, getCharSetForPanel() );
	switch( e_CursorPosFlg ) {
		// カーソル左移動 .
		case  PNLE_BoxCursorPosLEFT:
			Ctf_TextField->moveCursorLeft();			// カーソル表示 .
			break;
		// カーソル右移動 .
		case  PNLE_BoxCursorPosRIGHT:
			Ctf_TextField->moveCursorRight();			// カーソル表示 .
			break;
		// カーソル表示及び位置指定 .
		case  PNLE_BoxCursorPosSET:
			// カーソル表示 .
			Ctf_TextField->setCursorOption( True );
			Ctf_TextField->setCursorPosition( uc_CursorPos );
			break;
		// カーソル非表示.
		case  PNLE_BoxCursorPosDELETE:
			Ctf_TextField->setCursorOption( False );	// カーソル非表示.
			break;
		default:  break;	// 何も行わない.
	}
}

//関数-------------------------------------------------------------------------
//概要    ：ON/OFFキー押下処理.
//備考    ：.
//戻値    ：.
//作成者  ：AWS_パトリック.
//レビュー：.
//変更履歴：.
//------------------------------------------------------------------------------
/**
 * @brief A053634000 Fコード送信 : ボタン（押された)
*/
//------------------------------------------------------------------------------
void PCPC_FaxSendCommSetFCode_box::procBoxModeOnOff( Bool state )
{
	if( NULL == PnlComGetJobScanFax() ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }
	// 前処理
	TYPC_JobSpec* pc_JobSpec = PnlComGetJobScanFax()->getJobSpec();		// JobSpecを取得する
	if( NULL == pc_JobSpec ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }
	TYPC_FaxAct* pc_FaxAct = &( pc_JobSpec->pc_Fax->c_FaxAct );
	if( NULL == pc_FaxAct ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }

	// 強制付加設定の場合
	if( True == state ) {
		if( True == combinationCheck( CNK_FaxTxFcord, PnlComGetJobScanFax() )
		    &&  True == combinationCheck_AccumuDoc( CNK_FaxTxFcord, PnlComGetJBJobDocSpec(), PnlComGetJobScanFax() ) ) {
			if( ss_mFaxSendCommSetFCodeBackUpJobTableNo != -1 ) {
				PNLC_CopyDataLib::newInstance()->setJobSpecBackup( ss_mFaxSendCommSetFCodeBackUpJobTableNo, pc_JobSpec );		// JobSpecを前回の状態に戻す.
				PNLC_CopyDataLib::newInstance()->JobSpecBackupClear( ss_mFaxSendCommSetFCodeBackUpJobTableNo );
				ss_mFaxSendCommSetFCodeBackUpJobTableNo = -1;		// バックアップのインデックスを初期化.
				// SUBアドレス値をメンバ変数へ反映させる.
				pc_JobSpec->pc_Fax->c_FaxAct.getTxSubAddr( sc_LocalString1 );
				// パスワード値をメンバ変数へ反映させる.
				pc_JobSpec->pc_Fax->c_FaxAct.getTxLocalPwd( sc_LocalString2 );
			} else {
				procBoxModeSelectDont( pc_JobSpec );	// 初期化する.
			}
			// ハード10キーオプション装着していない場合.
			if( False == PnlComIsHard10Key() ) {
				us_FcodeFlg = PNLE_FaxRbtSel1761InputKeyNonSel;											// OUTPUT：初期化.
			}
		} else {
			Cmp_FaxSendCommSetFCode_boxPopupMessageArea.setOnOffSwitchButtonState( False );
		}
	} else {
		// MTUIテンキー非表示.
		hide10KeyPopup();
		ss_mFaxSendCommSetFCodeBackUpJobTableNo = PNLC_CopyDataLib::newInstance()->saveJobSpecBackup( ss_mFaxSendCommSetFCodeBackUpJobTableNo, pc_JobSpec );
		pc_FaxAct->setTxMode( TYPE_TxModeNormal );
		char sc_LocalTmpString1[ PNLD_MaxFaxTxSubAddr + 1 ];
		char sc_LocalTmpString2[ PNLD_MaxFaxTxLocalPwd + 1 ];
		memset( sc_LocalTmpString1, '\0', sizeof( sc_LocalTmpString1 ) );
		memset( sc_LocalTmpString2, '\0', sizeof( sc_LocalTmpString2 ) );
		pc_FaxAct->setTxSubAddr( sc_LocalTmpString1 );
		pc_FaxAct->setTxLocalPwd( sc_LocalTmpString2 );
		// カーソル非表示(SUBアドレス).
		e_SubCursorFlg = PNLE_BoxCursorPosDELETE;
		// カーソル非表示(パスワード).
		e_PassCursorFlg = PNLE_BoxCursorPosDELETE;
		if( False == PnlComIsHard10Key() ) {
			m_bl_isSelectedInputField = False;
		}
	}
}

//関数-------------------------------------------------------------------------
//概要    ：パスワードとSUBアドレスの取消し処理.
//備考    ：.
//戻値    ：.
//作成者  ：AWS_パトリック.
//レビュー：.
//変更履歴：.
//------------------------------------------------------------------------------
/**
 * @brief A053634000 Fコード送信 : ボタン（押された)
*/
//------------------------------------------------------------------------------
void PCPC_FaxSendCommSetFCode_box::procBoxModeSelectDont( TYPC_JobSpec* pc_JobSpec )
{
//
//	// SUBアドレス値をJobSpecへ反映させる.
//	// パスワード値をJobSpecへ反映させる.
//
	memset( sc_LocalString1,'\0',sizeof( sc_LocalString1 ) );
	memset( sc_LocalString2,'\0',sizeof( sc_LocalString2 ) );
}

//関数-------------------------------------------------------------------------
//概要    ：ボタン（押された).
//備考    ：.
//戻値    ：.
//作成者  ：AWS_バル.
//変更履歴：.
//------------------------------------------------------------------------------
/**
 * @brief A053634000 Fコード送信 : ボタン（押された)
 */
//------------------------------------------------------------------------------
void PCPC_FaxSendCommSetFCode_box::procButtonOn( uchar option )
{
	if( NULL == PnlComGetJobScanFax() ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }
	// Fax用ジョブスペックインスタンスの中にあるFax関連パラメータの先頭アドレスを取得.
	TYPC_FaxAct* pc_FaxAct = &( PnlComGetJobScanFax()->getJobSpec()->pc_Fax->c_FaxAct );

	if( NULL == pc_FaxAct ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }

	ushort us_StrLen = 0;
	char  w_str1[PNLD_MaxFaxTxSubAddr+1];
	char  w_str2[PNLD_MaxFaxTxLocalPwd+1];
	short ss_CurX;
	memset( w_str1, '\0', sizeof( w_str1 ) );
	memset( w_str2, '\0', sizeof( w_str2 ) );
	switch( option ) {
		case PNLE_PopupMessageAreaButtonType_On:		// ON選択.
			procBoxModeOnOff( True );
			break;
		case PNLE_PopupMessageAreaButtonType_Off:		// OFF選択.
			procBoxModeOnOff( False );
			break;
		case PNLD_FaxSendCommSetFCode_boxSubAddress:		// SUBアドレス.
			// ＳＵＢアドレス用カーソル表示.
// comment 2015/11/02 OPS1-Sky後藤 [MinervaSBK][制御構成変更No.75] パスワード入力文字表示対応.
			fixPassInput();
			if( True == m_bl_isSelectedInputField ) {
				e_SubCursorFlg = PNLE_BoxCursorPosSET;
			}
			// カーソル位置設定(SUBアドレス).
			uc_SubCursorPos = ( uchar )strlen( sc_LocalString1 );
			// パスワード用カーソル非表示 .
			e_PassCursorFlg = PNLE_BoxCursorPosDELETE;
			us_FcodeFlg = PNLE_FaxSendCommSetFCode_boxInputKeySubAddress;
			b_PassInputDirect = True;
			break;
		case PNLD_FaxSendCommSetFCode_boxPassword:		//パスワード .
			// ＳＵＢアドレス用カーソル非表示.
			e_SubCursorFlg = PNLE_BoxCursorPosDELETE;
			// パスワード用カーソル表示.
			if( True == m_bl_isSelectedInputField ) {
				e_PassCursorFlg = PNLE_BoxCursorPosSET;
			}
			// カーソル位置設定(パスワード).
			uc_PassCursorPos = ( uchar )strlen( sc_LocalString2 );
			us_FcodeFlg = PNLE_FaxSendCommSetFCode_boxInputKeyPassword;
			b_SubInputDirect = True;
			break;
		case PNLD_FaxSendCommSetFCode_boxSubAddressLeft:	//←カーソル移動（ＳＵＢ）.
// comment 2016/06/17 SCD2-2G Sky松下 [ZeusSBK][CQ527761] ←、→、削除キー長押しの報知音修正.
			// キーの長押しで報知音を鳴らすためOKを設定.
			setBuzzerType( PCGE_BuzzerType_InputOk );
			// SUBアドレスボタンが選択状態時のみカーソル移動処理を実行.
			// 現在のカーソル位置を取得 .
			if( us_FcodeFlg == PNLE_FaxSendCommSetFCode_boxInputKeySubAddress ) {
				Ctf_FaxSendCommSetFCode_boxSubAddress.getCursorPosition( ss_CurX );
				// カーソル位置が先頭の場合、カーソル移動不可 .
				if( 0 == ss_CurX ) {
					setBuzzerType( PCGE_BuzzerType_InputNg );		// 報知音設定.
				}
				e_SubCursorFlg = PNLE_BoxCursorPosLEFT;			// 左へ１移動 .
				m_bl_isSelectedInputField = True;
			} else {
				setBuzzerType( PCGE_BuzzerType_InputNg );		// 報知音設定.
				return;		// 処理実行せずreturn（setupGamen()もコールしない）.
			}
			break;
		case PNLD_FaxSendCommSetFCode_boxSubAddressRight:		//→カーソル移動（ＳＵＢ） .
// comment 2016/06/17 SCD2-2G Sky松下 [ZeusSBK][CQ527761] ←、→、削除キー長押しの報知音修正.
			// キーの長押しで報知音を鳴らすためOKを設定.
			setBuzzerType( PCGE_BuzzerType_InputOk );
			// SUBアドレスボタンが選択状態時のみカーソル移動処理を実行 .
			if( us_FcodeFlg == PNLE_FaxSendCommSetFCode_boxInputKeySubAddress ) {
				us_StrLen = ( ushort )strlen( sc_LocalString1 );					// 掲示板No.の文字列長を取得 .
				Ctf_FaxSendCommSetFCode_boxSubAddress.getCursorPosition( ss_CurX );		// 掲示板No.用カーソルの現在のカーソル位置を取得 .
				// 文字列の後にカーソルがある場合、カーソル移動不可 .
				if( ss_CurX == us_StrLen ) {
					setBuzzerType( PCGE_BuzzerType_InputNg );		// 報知音設定.
				}
				e_SubCursorFlg = PNLE_BoxCursorPosRIGHT;		// 右へ１移動 .
				m_bl_isSelectedInputField = True;
			} else {
				setBuzzerType( PCGE_BuzzerType_InputNg );			// 報知音設定.
				return;			// 処理実行せずreturn（setupGamen()もコールしない）.
			}
			break;
		case PNLD_FaxSendCommSetFCode_boxPasswordLeft:		//←カーソル移動（パスワード）.
// comment 2016/06/17 SCD2-2G Sky松下 [ZeusSBK][CQ527761] ←、→、削除キー長押しの報知音修正.
			// キーの長押しで報知音を鳴らすためOKを設定.
			setBuzzerType( PCGE_BuzzerType_InputOk );
			// パスワードボタンが選択状態時のみカーソル移動処理を実行.
			if( us_FcodeFlg == PNLE_FaxSendCommSetFCode_boxInputKeyPassword ) {
				// パスワード設定済み用文字列（「**********」）表示 or 非表示状態が「True（＝表示）」の場合.
				if( b_isStringForFinishingPwdDisplay == True ) {
					// パスワードクリア（パスワードの再入力）.
					memset( sc_LocalString2, '\0', sizeof( sc_LocalString2 ) );
					// パスワード用カーソルを先頭へ移動 .
					uc_PassCursorPos = 0;
					e_PassCursorFlg = PNLE_BoxCursorPosSET;
					// パスワード設定済み用文字列（「**********」）表示 or 非表示状態に「False（＝非表示）」を設定 .
					b_isStringForFinishingPwdDisplay = False;
				} else {
					// パスワード設定済み用文字列（「**********」）表示 or 非表示状態が「False（＝非表示）」の場合 .
					Ctf_FaxSendCommSetFCode_boxPassword.getCursorPosition( ss_CurX );		// 現在のカーソル位置を取得 .
					// カーソル位置が先頭の場合、カーソル移動不可.
					if( 0 == ss_CurX ) {
						setBuzzerType( PCGE_BuzzerType_InputNg );		// 報知音設定.
					}
					e_PassCursorFlg = PNLE_BoxCursorPosLEFT;			// 左へ１移動.
				}
				m_bl_isSelectedInputField = True;
			} else {
				setBuzzerType( PCGE_BuzzerType_InputNg );		// 報知音設定.
				return;		// 処理実行せずreturn（setupGamen()もコールしない）.
			}
			break;
		case PNLD_FaxSendCommSetFCode_boxPasswordRight:		//→カーソル移動（パスワード）.
// comment 2016/06/17 SCD2-2G Sky松下 [ZeusSBK][CQ527761] ←、→、削除キー長押しの報知音修正.
			// キーの長押しで報知音を鳴らすためOKを設定.
			setBuzzerType( PCGE_BuzzerType_InputOk );
			// パスワードボタンが選択状態時のみカーソル移動処理を実行 .
			if( us_FcodeFlg == PNLE_FaxSendCommSetFCode_boxInputKeyPassword ) {
				us_StrLen = ( ushort )strlen( sc_LocalString2 );								// 文字列長を取得.
				Ctf_FaxSendCommSetFCode_boxPassword.getCursorPosition( ss_CurX );		// 現在のカーソル位置を取得 .
				// 文字列の後にカーソルがある場合、カーソル移動不可.
				if( ss_CurX == us_StrLen ) {
					setBuzzerType( PCGE_BuzzerType_InputNg );		// 報知音設定.
				}
				e_PassCursorFlg = PNLE_BoxCursorPosRIGHT;			// 右へ１移動.
				m_bl_isSelectedInputField = True;
			} else {
				setBuzzerType( PCGE_BuzzerType_InputNg );			// 報知音設定.
				return;			// 処理実行せずreturn（setupGamen()もコールしない）.
			}
			break;
		case PNLD_FaxSendCommSetFCode_boxSubAddressDelete:		//削除（ＳＵＢ） .
// comment 2016/06/17 SCD2-2G Sky松下 [ZeusSBK][CQ527761] ←、→、削除キー長押しの報知音修正.
			// キーの長押しで報知音を鳴らすためOKを設定.
			setBuzzerType( PCGE_BuzzerType_InputOk );
			// SUBアドレスボタンが選択状態時のみカーソル移動処理を実行.
			if( us_FcodeFlg == PNLE_FaxSendCommSetFCode_boxInputKeySubAddress ) {
				us_StrLen = ( ushort )strlen( sc_LocalString1 );	// ＳＵＢアドレス文字数取得.
				Ctf_FaxSendCommSetFCode_boxSubAddress.getCursorPosition( ss_CurX );		// カーソル位置取得.
				// 文字列の後にカーソルがある場合
				if( ss_CurX == us_StrLen ) {
					// 既に文字の先頭が'\0'.
					if( sc_LocalString1[0] == '\0' ) {
						setBuzzerType( PCGE_BuzzerType_InputNg );		// 報知音設定.
					}
					// 文字の削除.
					sc_LocalString1[ ss_CurX - 1 ] = '\0';
					// 左へ１移動.
					e_SubCursorFlg = PNLE_BoxCursorPosLEFT;
					// 文字列の中にカーソルがある場合.
				} else {
					// カーソル位置の次の文字列を格納.
					strncpy( w_str1, &sc_LocalString1[ss_CurX+1] , sizeof( w_str1 ) - 1 );
					// カーソル位置の文字を削除.
					sc_LocalString1[ss_CurX] = '\0';
					// 文字列の結合
					strcat( sc_LocalString1, w_str1 );
				}
				m_bl_isSelectedInputField = True;
			} else {
				// 報知音設定.
				setBuzzerType( PCGE_BuzzerType_InputNg );
				return;// 処理実行せずreturn（setupGamen()もコールしない）.
			}
			break;
		case PNLD_FaxSendCommSetFCode_boxPasswordDelete:		//削除（パスワード） .
// comment 2016/06/17 SCD2-2G Sky松下 [ZeusSBK][CQ527761] ←、→、削除キー長押しの報知音修正.
			// キーの長押しで報知音を鳴らすためOKを設定.
			setBuzzerType( PCGE_BuzzerType_InputOk );
			// パスワードボタンが選択状態時のみカーソル移動処理を実行.
			if( us_FcodeFlg == PNLE_FaxSendCommSetFCode_boxInputKeyPassword ) {
				// パスワード設定済み用文字列（「**********」）表示 or 非表示状態が「True（＝表示）」の場合.
				if( b_isStringForFinishingPwdDisplay == True ) {
					// パスワードクリア（パスワードの再入力）.
					memset( sc_LocalString2, '\0', sizeof( sc_LocalString2 ) );
					// パスワード用カーソルを先頭へ移動 .
					uc_PassCursorPos = 0;
					e_PassCursorFlg = PNLE_BoxCursorPosSET;
					// パスワード設定済み用文字列（「**********」）表示 or 非表示状態に「False（＝非表示）」を設定.
					b_isStringForFinishingPwdDisplay = False;
				} else {
					// パスワード設定済み用文字列（「**********」）表示 or 非表示状態が「False（＝非表示）」の場合 .
					// パスワード文字数取得 .
					us_StrLen = ( ushort )strlen( sc_LocalString2 );
					// カーソル位置取得 .
					Ctf_FaxSendCommSetFCode_boxPassword.getCursorPosition( ss_CurX );		// カーソル位置取得.
					// 文字列の後にカーソルがある場合 .
					// 既に文字の先頭が'\0'	.
					if( ss_CurX == us_StrLen ) {
						// 報知音設定.
						if( sc_LocalString2[0] == '\0' ) {
							setBuzzerType( PCGE_BuzzerType_InputNg );
						}
						// 文字の削除.
						sc_LocalString2[ ss_CurX - 1 ] = '\0';
						// 左へ１移動.
						e_PassCursorFlg = PNLE_BoxCursorPosLEFT;
					} else {
						// 文字列の中にカーソルがある場合.
						// カーソル位置の次の文字列を格納.
						strncpy( w_str2, &sc_LocalString2[ss_CurX+1] , sizeof( w_str2 ) - 1 );
						// カーソル位置の文字を削除.
						sc_LocalString2[ss_CurX] = '\0';
						// 文字列の結合.
						strcat( sc_LocalString2, w_str2 );
					}
// comment 2015/07/23 OPS1-Sky浜野 [MinervaSBK][制御構成変更No.75] パスワード入力文字表示対応.
					// パスワード入力表示位置を更新.
					adjustPassInsert( ss_CurX, 1 );
				}
				m_bl_isSelectedInputField = True;
			} else {
				// 報知音設定.
				setBuzzerType( PCGE_BuzzerType_InputNg );
				return;// 処理実行せずreturn（setupGamen()もコールしない）.
			}
			break;
		//	エラーメッセージ表示.
		case BTNID_KEY_POPUP_OK:
			if( False == Key_FaxSendCommSetFCode_boxOK.isActivating() ) {
				// 報知音設定.
				setBuzzerType( PCGE_BuzzerType_InputNg );
				return;
			} else {
				procOk( option );
				return;
			}
			break;
		// SUBアドレス入力エリア.
		// 送信ID入力エリア.
		case PNLD_FaxSendCommSetFCode_boxSubAddressInputArea:
		case PNLD_FaxSendCommSetFCode_boxPasswordInputArea:
			m_bl_isSelectedInputField = True;

			setupGamen( option );
			// テンキーポップアップ表示.
			show10KeyPopup();
			break;
		default:  break;
	}
	// SUBアドレス値をJobSpecへ反映させる.
	// パスワード値をJobSpecへ反映させる.
//comment 2019/05/07 HCLT-Shalini [IT6_EagleH] [OP_BTS-3217] Restricting the value set based on Mode.
//	if( PNLE_PopupMessageAreaButtonType_Off != option ) {
	if( ( PNLE_PopupMessageAreaButtonType_Off != option ) && ( pc_FaxAct->getTxMode() != TYPE_TxModeClose ) ) {
		// SUBアドレス値をJobSpecへ反映させる.
		pc_FaxAct->setTxSubAddr( sc_LocalString1 );
		// パスワード値をJobSpecへ反映させる.
		pc_FaxAct->setTxLocalPwd( sc_LocalString2 );
	}
	// SUBアドレス設定の場合.
	if( sc_LocalString1[0] != '\0' ) {
		if( PNLE_PopupMessageAreaButtonType_Off != option ) {
			pc_FaxAct->setTxMode( TYPE_TxModeMailbox );
		}
	} else {
		// SUBアドレス未設定の場合.
		if( ( pc_FaxAct->getTxMode() == TYPE_TxModeMailbox )
		    || ( pc_FaxAct->getTxMode() == TYPE_TxModeRelayReq ) ) {

			// 通常送信.
			pc_FaxAct->setTxMode( TYPE_TxModeNormal );
		}
	}
	setupGamen( option );
}

//関数------------------------------------------------------------------------------------------------------------------
//概要    ：ボタン（押し続け、速く変化）.
//戻値    ：無し.
//作成者  ：竹菴.
//変更履歴：.
//------------------------------------------------------------------------------
/**
 * @brief A053634000 Fコード送信 : ボタン（押し続け、速く変化）.
 */
//------------------------------------------------------------------------------
void PCPC_FaxSendCommSetFCode_box::procButtonStillOnFast( uchar option )
{
	// 「←」、「→」、「削除」キーのみ押し続け可能.
	if( ( PNLD_FaxSendCommSetFCode_boxSubAddressLeft   == option )		// ←カーソル移動（ＳＵＢ）.
	    || ( PNLD_FaxSendCommSetFCode_boxSubAddressRight  == option )		// →カーソル移動（ＳＵＢ）.
	    || ( PNLD_FaxSendCommSetFCode_boxSubAddressDelete == option )		// 削除（ＳＵＢ）.
	    || ( PNLD_FaxSendCommSetFCode_boxPasswordLeft     == option )		// ←カーソル移動（パスワード）.
	    || ( PNLD_FaxSendCommSetFCode_boxPasswordRight    == option )		// →カーソル移動（パスワード）.
	    || ( PNLD_FaxSendCommSetFCode_boxPasswordDelete   == option ) ) {	// 削除（パスワード）.
		procButtonOn( option );
	}
}

//関数--------------------------------------------------------------------------
//概要    ：設定終了.
//備考    ：.
//戻値    ：.
//作成者  ：AWS_バル.
//変更履歴：.
//------------------------------------------------------------------------------
/**
 * @brief A053634000 Fコード送信 : 設定終了
 */
//------------------------------------------------------------------------------
void PCPC_FaxSendCommSetFCode_box::procOk( uchar option )
{
// Comment 2015/10/08【HCLT-Rengaraaj】【CQ519679】【ID: 26784】NPD.CHECK.MUST.
//	if( NULL == PnlComGetJobFax( sc_JobPutNo ) ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }
	APIC_Job* pc_JobFax = PnlComGetJobFax( sc_JobPutNo );
	if( NULL == pc_JobFax ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return;
	}
	TYPC_JobSpec* pc_JobSpec = pc_JobFax->getJobSpec();
	if( NULL == pc_JobSpec ){
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return;
	}
	TYPC_Fax * pc_Fax = pc_JobSpec->pc_Fax;
	if( NULL == pc_Fax ){
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return;
	}
	
//	TYPC_FaxAct* pc_FaxAct = &( PnlComGetJobFax( sc_JobPutNo )->getJobSpec()->pc_Fax->c_FaxAct );		//初期化.
	TYPC_FaxAct* pc_FaxAct = &( pc_Fax->c_FaxAct );														//初期化.

	if( NULL == pc_FaxAct ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }

	if( pc_FaxAct->getTxMode() == TYPE_TxModeNormal ) {
		// 親展・中継送信が設定されていない場合はパスワードを初期化する
		// パスワードを初期化する.
		memset( sc_LocalString2, '\0', sizeof( sc_LocalString2 ) );
		// パスワード設定（JobSpecクリア）.
		pc_FaxAct->setTxLocalPwd( sc_LocalString2 );
	}
	PNLC_PopupListControl::newInstance()->closeTop( True );		//普通にポップアップ終了させる.
}

//関数--------------------------------------------------------------------------
//概要    ：テンキー.
//備考    ：.
//戻値    ：.
//作成者  ：AWS_バル.
//------------------------------------------------------------------------------
/**
 * @brief A053634000 Fコード送信 : テンキー
 */
//------------------------------------------------------------------------------
void PCPC_FaxSendCommSetFCode_box::proc10Key( uchar option )
{
	if( NULL == PnlComGetJobScanFax() ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }
	// Fax用ジョブスペックインスタンスの中にあるFax関連パラメータの先頭アドレスを取得.
	TYPC_FaxAct* pc_FaxAct = &( PnlComGetJobScanFax()->getJobSpec()->pc_Fax->c_FaxAct );

	if( NULL == pc_FaxAct ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }

	if( False == Cmp_FaxSendCommSetFCode_boxPopupMessageArea.getOnOffSwitchButtonState() ) {
		setBuzzerType( PCGE_BuzzerType_InputNg );	// 報知音設定.
		return;
	}

	ushort us_StrLen = 0;
	char  w_str1[PNLD_MaxFaxTxSubAddr+1];
	char  w_str2[PNLD_MaxFaxTxLocalPwd+1];
	uchar uc_text;
	memset( w_str1, '\0', sizeof( w_str1 ) );
	memset( w_str2, '\0', sizeof( w_str2 ) );
	short ss_CurX = 0;
	if( option == PNLD_FaxKeySharp ) {
		// 「＃」キーが押下された場合.
		uc_text = '#';
	} else if( option == PNLD_FaxKeyAsterisk ) {
		// 「＊」キー押下された場合.
		uc_text = '*';
	} else {
		// 「0」～「9」キーが押下された場合.
		uc_text = static_cast<uchar>( option + 0x30 );
	}
	// SUBアドレス入力の場合.
	if( us_FcodeFlg == PNLE_FaxSendCommSetFCode_boxInputKeySubAddress ) {
		// SUBアドレス文字数取得.
		us_StrLen = ( ushort )strlen( sc_LocalString1 );
		// カーソル位置取得.
		Ctf_FaxSendCommSetFCode_boxSubAddress.getCursorPosition( ss_CurX );
		if( ss_CurX == us_StrLen ) {
			if( b_SubInputDirect == True ) {
				// SUBアドレスをALLクリア.
				memset( sc_LocalString1, '\0', sizeof( sc_LocalString1 ) );
				// カーソルを先頭へ移動.
				uc_SubCursorPos = 0;
				e_SubCursorFlg = PNLE_BoxCursorPosSET;
				b_SubInputDirect = False;
			}
			// 文字列にして入力.
			InputStringWithKeyboard( sc_LocalString1, uc_text, PNLD_MaxFaxTxSubAddrInput, bl_CapsOn );
			// 最大文字数の場合.
			if( us_StrLen == PNLD_MaxFaxTxSubAddrInput ) {
				// 報知音設定.
				setBuzzerType( PCGE_BuzzerType_InputNg );
			}
		} else {
			if( b_SubInputDirect == True ) {
				// SUBアドレスをALLクリア.
				memset( sc_LocalString1, '\0', sizeof( sc_LocalString1 ) );
				// カーソルを先頭へ移動.
				uc_SubCursorPos = 0;
				e_SubCursorFlg = PNLE_BoxCursorPosSET;
				b_SubInputDirect = False;
			}
			// 最大文字数の場合、入力抑止.
			if( us_StrLen == PNLD_MaxFaxTxSubAddrInput ) {
				// 報知音設定
				setBuzzerType( PCGE_BuzzerType_InputNg );
				return;
			}
			// カーソル位置からの文字列を格納.
			strncpy( w_str1, &sc_LocalString1[ss_CurX] , sizeof( w_str1 ) - 1 );
			// カーソル位置にNULL文字を代入.
			sc_LocalString1[ss_CurX] = '\0';
			// 文字(テンキー)の入力.
			InputStringWithKeyboard( sc_LocalString1, uc_text, PNLD_MaxFaxTxSubAddrInput, bl_CapsOn );
			// 文字列結合.
			strcat( sc_LocalString1, w_str1 );
		}
		// 右へ１移動.
		e_SubCursorFlg = PNLE_BoxCursorPosRIGHT;
	} else if( us_FcodeFlg == PNLE_FaxSendCommSetFCode_boxInputKeyPassword ) {
		// パスワード入力の場合.
		// パスワード設定済み用文字列（「**********」）表示 or 非表示状態が「True（＝表示）」の場合.
		if( b_isStringForFinishingPwdDisplay == True || b_PassInputDirect == True ) {
			// パスワードの再入力とみなす.
			// パスワードクリア.
			memset( sc_LocalString2, '\0', sizeof( sc_LocalString2 ) );
			// 文字(テンキー)の入力.

			InputStringWithKeyboard( sc_LocalString2, uc_text, PNLD_MaxFaxTxFcodePwdInput, bl_CapsOn );
			// パスワード用カーソル位置取得.
			uc_PassCursorPos = ( uchar )strlen( sc_LocalString2 );
			// パスワード用カーソルを指定位置に表示.
			e_PassCursorFlg = PNLE_BoxCursorPosSET;
			// パスワード設定済み用文字列（「**********」）表示 or 非表示状態に「False（＝非表示）」を設定.
			b_isStringForFinishingPwdDisplay = False;
			b_PassInputDirect = False;
// comment 2015/07/10 OPS1-Sky浜野 [MinervaSBK][制御構成変更No.75] パスワード入力文字表示対応.
			// パスワード入力表示開始.
			judgePassInsert( 0 );
		} else {
			// パスワード設定済み用文字列（「**********」）表示 or 非表示状態が「False（＝非表示）」の場合.
			// パスワード文字数取得.
			us_StrLen = ( ushort )strlen( sc_LocalString2 );
			// カーソル位置取得.
			Ctf_FaxSendCommSetFCode_boxPassword.getCursorPosition( ss_CurX );

			if( ss_CurX == us_StrLen ) {
				// 文字列にして入力.
				InputStringWithKeyboard( sc_LocalString2, uc_text, PNLD_MaxFaxTxFcodePwdInput, bl_CapsOn );
				// 最大文字数の場合.
				if( us_StrLen == PNLD_MaxFaxTxFcodePwdInput ) {
					// 報知音設定.
					setBuzzerType( PCGE_BuzzerType_InputNg );
				}
			} else {
				// 最大文字数の場合、入力抑止.
				if( us_StrLen == PNLD_MaxFaxTxFcodePwdInput ) {
					// 報知音設定.
					setBuzzerType( PCGE_BuzzerType_InputNg );
					return;
				}
				// カーソル位置からの文字列を格納.
				strncpy( w_str2, &sc_LocalString2[ss_CurX] , sizeof( w_str2 ) - 1 );
				// カーソル位置にNULLを代入.
				sc_LocalString2[ss_CurX] = '\0';
				// 文字(テンキー)の入力.
				InputStringWithKeyboard( sc_LocalString2, uc_text, PNLD_MaxFaxTxFcodePwdInput, bl_CapsOn );
				// 文字列結合.
				strcat( sc_LocalString2, w_str2 );
			}
// comment 2015/07/10 OPS1-Sky浜野 [MinervaSBK][制御構成変更No.75] パスワード入力文字表示対応.
			// パスワード入力表示開始.
			judgePassInsert( ss_CurX );
			// 右へ１移動. 
			e_PassCursorFlg = PNLE_BoxCursorPosRIGHT;
		}
	} else {
		// 報知音設定.
		setBuzzerType( PCGE_BuzzerType_InputNg );
	}
	// SUBアドレス値をJobSpecへ反映させる.
	pc_FaxAct->setTxSubAddr( sc_LocalString1 );
	// パスワード値をJobSpecへ反映させる.
	pc_FaxAct->setTxLocalPwd( sc_LocalString2 );
	if( sc_LocalString1[0] != '\0' ) {
		// SUBアドレス設定の場合.
		// 親展送信.
		pc_FaxAct->setTxMode( TYPE_TxModeMailbox );
	} else {
		// SUBアドレス未設定の場合.
		// 親展送信又は中継依頼送信の場合は、設定をクリア.
		if( ( pc_FaxAct->getTxMode() == TYPE_TxModeMailbox )
		    || ( pc_FaxAct->getTxMode() == TYPE_TxModeRelayReq ) ) {
			// 通常送信.
			pc_FaxAct->setTxMode( TYPE_TxModeNormal );
		}
	}
	setupGamen();
}

//関数--------------------------------------------------------------------------
//概要    ：FAXキー.
//備考    ：.
//戻値    ：.
//作成者  ：AWS_バル.
//変更履歴：.
//------------------------------------------------------------------------------
/**
 * @brief A053634000 Fコード送信 : FAXキー
 */
//------------------------------------------------------------------------------
void PCPC_FaxSendCommSetFCode_box::procFax( uchar option )
{
	checkSoft10KeyData();
	PMessageArea::procFax( option );
}

//関数--------------------------------------------------------------------------
//概要    ：FAXキー.
//備考    ：.
//戻値    ：.
//作成者  ：Sky_稲葉.
//レビュー：.
//変更履歴：.
//------------------------------------------------------------------------------
/**
 * @brief A053634000 Fコード送信 : FAXキー
 */
//------------------------------------------------------------------------------
void PCPC_FaxSendCommSetFCode_box::procFaxOnly( uchar option )
{
	checkSoft10KeyData();
	PMessageArea::procFaxOnly( option );
}

//関数--------------------------------------------------------------------------
//概要    ：スタート.
//備考    ：.
//戻値    ：.
//作成者  ：AWS_バル.
//変更履歴：.
//------------------------------------------------------------------------------
/**
 * @brief A053634000 Fコード送信 : スタート
 */
//------------------------------------------------------------------------------
void PCPC_FaxSendCommSetFCode_box::procStart( uchar option )
{
	checkSoft10KeyData();
// Comment 2015/10/08【HCLT-Rengaraaj】【CQ519679】【ID: 26785】NPD.CHECK.MUST.
//	if( NULL == PnlComGetJobFax( sc_JobPutNo ) ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }
	APIC_Job* pc_JobFax = PnlComGetJobFax( sc_JobPutNo );
	if( NULL == pc_JobFax ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return;
	}
	TYPC_JobSpec* pc_JobSpec = pc_JobFax->getJobSpec();
	if( NULL == pc_JobSpec ){
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return;
	}
	TYPC_Fax * pc_Fax = pc_JobSpec->pc_Fax;
	if( NULL == pc_Fax ){
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return;
	}
	
//	TYPC_FaxAct* pc_FaxAct = &( PnlComGetJobFax( sc_JobPutNo )->getJobSpec()->pc_Fax->c_FaxAct );
	TYPC_FaxAct* pc_FaxAct = &( pc_Fax->c_FaxAct );
	if( NULL == pc_FaxAct ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }
	if( pc_FaxAct->getTxMode() == TYPE_TxModeNormal ) {
		// 親展・中継送信が設定されていない場合はパスワードを初期化する.
		memset( sc_LocalString2, '\0', sizeof( sc_LocalString2 ) );	// パスワード表示用.
		// パスワード設定（JobSpecクリア）.
		pc_FaxAct->setTxLocalPwd( sc_LocalString2 );
	}

	PNLC_BoxDataKeyControlLib* pc_BoxDataKeyControlLib = PNLC_BoxDataKeyControlLib::newInstance();
	if( NULL == pc_BoxDataKeyControlLib ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }

	pc_BoxDataKeyControlLib->procStartDocNetSendNetSend( option );

}

//関数--------------------------------------------------------------------------
//概要    ：パネルリセット.
//備考    ：.
//戻値    ：.
//作成者  ：AWS_バル.
//変更履歴：.
//------------------------------------------------------------------------------
/**
 * @brief A053634000 Fコード送信 : パネルリセット
 */
//------------------------------------------------------------------------------
void PCPC_FaxSendCommSetFCode_box::procReset( uchar option )
{
	PMessageArea::procReset( option );
}

//関数--------------------------------------------------------------------------
//概要    ：クリアー.
//備考    ：.
//戻値    ：.
//作成者  ：AWS_バル.
//------------------------------------------------------------------------------
/**
 * @brief A053634000 Fコード送信 : クリアー
 */
//------------------------------------------------------------------------------
void PCPC_FaxSendCommSetFCode_box::procClear( uchar option )
{
	if( NULL == PnlComGetJobScanFax() ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }
	// Fax用ジョブスペックインスタンスの中にあるFax関連パラメータの先頭アドレスを取得.
	TYPC_FaxAct* pc_FaxAct = &( PnlComGetJobScanFax()->getJobSpec()->pc_Fax->c_FaxAct );
	if( NULL == pc_FaxAct ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }

	if( False == Cmp_FaxSendCommSetFCode_boxPopupMessageArea.getOnOffSwitchButtonState() ) {
		setBuzzerType( PCGE_BuzzerType_InputNg );	// 報知音設定.
		return;
	}

	if( us_FcodeFlg == PNLE_FaxSendCommSetFCode_boxInputKeySubAddress ) {
		// SUBアドレス入力の場合.
		if( sc_LocalString1[0] == '\0' ) {
			// 報知音設定
			setBuzzerType( PCGE_BuzzerType_InputNg );
		}
		b_SubInputDirect = True;
		// 入力タイプ（SUBアドレス）.
		// SUBアドレスをALLクリア.
		memset( sc_LocalString1, '\0', sizeof( sc_LocalString1 ) );
		// カーソルを先頭へ移動.
		uc_SubCursorPos = 0;
		e_SubCursorFlg = PNLE_BoxCursorPosSET;
		// SUBアドレス値をJobSpecへ反映させる.
		pc_FaxAct->setTxSubAddr( sc_LocalString1 );
		// 通常送信.
		pc_FaxAct->setTxMode( TYPE_TxModeNormal );
	} else if( us_FcodeFlg == PNLE_FaxSendCommSetFCode_boxInputKeyPassword ) {
		// パスワード入力の場合.
		if( sc_LocalString2[0] == '\0' ) {
			// 報知音設定.
			setBuzzerType( PCGE_BuzzerType_InputNg );
		}
		b_PassInputDirect = True;								// 入力タイプ（パスワード）.
		// パスワードをALLクリア.
		memset( sc_LocalString2, '\0', sizeof( sc_LocalString2 ) );
// comment 2015/07/10 OPS1-Sky浜野 [MinervaSBK][制御構成変更No.75] パスワード入力文字表示対応.
		fixPassInput();
		// カーソルを先頭へ移動.
		uc_PassCursorPos = 0;
		e_PassCursorFlg = PNLE_BoxCursorPosSET;
		// パスワード値をJobSpecへ反映させる.
		pc_FaxAct->setTxLocalPwd( sc_LocalString2 );
		// パスワード設定済み用文字列（「**********」）表示 or 非表示状態に「False（＝非表示）」を設定.
		b_isStringForFinishingPwdDisplay = False;
	} else {
		// 報知音設定.
		setBuzzerType( PCGE_BuzzerType_InputNg );
	}
	setupGamen();
}

//関数--------------------------------------------------------------------------
//概要    ：アスタリスクキー.
//備考    ：.
//戻値    ：.
//作成者  ：AWS_バル.
//変更履歴：.
//------------------------------------------------------------------------------
/**
 * @brief A053634000 Fコード送信 : アスタリスクキー
 */
//------------------------------------------------------------------------------
void PCPC_FaxSendCommSetFCode_box::procAster( uchar option )
{
	proc10Key( PNLD_FaxKeyAsterisk );
}

//関数--------------------------------------------------------------------------
//概要    ：シャープキー.
//備考    ：.
//戻値    ：.
//作成者  ：AWS_バル.
//変更履歴：.
//------------------------------------------------------------------------------
/**
 * @brief A053634000 Fコード送信 : シャープキー
 */
//------------------------------------------------------------------------------
void PCPC_FaxSendCommSetFCode_box::procShrp( uchar option )
{
	proc10Key( PNLD_FaxKeySharp );
}

//関数--------------------------------------------------------------------------
//概要    ：ストップキー.
//備考    ：.
//戻値    ：.
//作成者  ：AWS_バル.
//変更履歴：.
//------------------------------------------------------------------------------
/**
 * @brief A053634000 Fコード送信 : ストップキー
 */
//------------------------------------------------------------------------------
void PCPC_FaxSendCommSetFCode_box::procStop( uchar option )
{
	// スキャン連携アプリ起動中は押下を受け付けない.
	if( ScanOapExtSet.isActiveGeneralApplication() ) {
		setBuzzerType( PCGE_BuzzerType_InputNg );
	} else {
		PMessageArea::procStop( option );
	}
}

//関数--------------------------------------------------------------------------
//概要    ：コピーキー.
//備考    ：.
//戻値    ：.
//作成者  ：AWS_バル.
//変更履歴：.
//------------------------------------------------------------------------------
/**
 * @brief A053634000 Fコード送信 : コピーキー
 */
//------------------------------------------------------------------------------
void PCPC_FaxSendCommSetFCode_box::procCopy( uchar option )
{
	checkSoft10KeyData();
	PNLC_ScanFaxDataKeyControlLib* pc_ScanFaxDataKeyControlLib = PNLC_ScanFaxDataKeyControlLib::newInstance();
	if( NULL == pc_ScanFaxDataKeyControlLib ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }
	pc_ScanFaxDataKeyControlLib->procCopy( option );
}

//関数--------------------------------------------------------------------------.
//概要    ：ユビキタスプリントキー.
//備考    ：.
//戻値    ：.
//作成者  ：.
//レビュー：.
//変更履歴：.
//------------------------------------------------------------------------------.
/**
 * @brief A053634000 Fコード送信 : ユビキタスプリントキー
 */
//------------------------------------------------------------------------------.
void PCPC_FaxSendCommSetFCode_box::procUbqPrint( uchar option )
{
	checkSoft10KeyData();
	PNLC_ScanFaxDataKeyControlLib* pc_ScanFaxDataKeyControlLib = PNLC_ScanFaxDataKeyControlLib::newInstance();
	if( NULL == pc_ScanFaxDataKeyControlLib ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }
	pc_ScanFaxDataKeyControlLib->procUbqPrint( option );
}


//関数--------------------------------------------------------------------------
//概要    ：ブラウザキー.
//備考    ：.
//戻値    ：.
//作成者  ：AWS_バル.
//変更履歴：.
//------------------------------------------------------------------------------
/**
 * @brief A053634000 Fコード送信 : ブラウザキー
 */
//------------------------------------------------------------------------------
void PCPC_FaxSendCommSetFCode_box::procBrowser( uchar option )
{
	checkSoft10KeyData();
	PNLC_ScanFaxDataKeyControlLib* pc_ScanFaxDataKeyControlLib = PNLC_ScanFaxDataKeyControlLib::newInstance();
	if( NULL == pc_ScanFaxDataKeyControlLib ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }
	pc_ScanFaxDataKeyControlLib->procBrowser( option );
}


//関数--------------------------------------------------------------------------
//概要    ：ユーティリティキー.
//備考    ：.
//戻値    ：.
//作成者  ：AWS_バル.
//変更履歴：.
//------------------------------------------------------------------------------
/**
* @brief A053634000 Fコード送信 : ユーティリティキー
*/
//------------------------------------------------------------------------------
void PCPC_FaxSendCommSetFCode_box::procUtility( uchar option )
{
	checkSoft10KeyData();
	PNLC_ScanFaxDataKeyControlLib* pc_ScanFaxDataKeyControlLib = PNLC_ScanFaxDataKeyControlLib::newInstance();
	if( NULL == pc_ScanFaxDataKeyControlLib ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }
	pc_ScanFaxDataKeyControlLib->procUtility( option );
}

//関数--------------------------------------------------------------------------
//概要    ：ヘルプ.
//備考    ：.
//戻値    ：.
//作成者  ：AWS_バル.
//変更履歴：.
//------------------------------------------------------------------------------
/**
 * @brief A053634000 Fコード送信 : ヘルプ
 */
//------------------------------------------------------------------------------
void PCPC_FaxSendCommSetFCode_box::procHelp( uchar option )
{
	checkSoft10KeyData();
	PNLC_ScanFaxDataKeyControlLib* pc_ScanFaxDataKeyControlLib = PNLC_ScanFaxDataKeyControlLib::newInstance();
	if( NULL == pc_ScanFaxDataKeyControlLib ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }
	pc_ScanFaxDataKeyControlLib->procHelp( option );
}

//関数--------------------------------------------------------------------------
//概要    ：ユーザー補助.
//備考    ：.
//戻値    ：.
//作成者  ：AWS_バル.
//変更履歴：.
//------------------------------------------------------------------------------
/**
 * @brief A053634000 Fコード送信 : ユーザー補助
 */
//------------------------------------------------------------------------------
void PCPC_FaxSendCommSetFCode_box::procAccessibility( uchar option )
{
	checkSoft10KeyData();
	PNLC_ScanFaxDataKeyControlLib* pc_ScanFaxDataKeyControlLib = PNLC_ScanFaxDataKeyControlLib::newInstance();
	if( NULL == pc_ScanFaxDataKeyControlLib ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }
	pc_ScanFaxDataKeyControlLib->procAccessibility( option );
}

//関数--------------------------------------------------------------------------
//概要    ：パワーセーブ.
//備考    ：.
//戻値    ：.
//作成者  ：AWS_バル.
//変更履歴：.
//------------------------------------------------------------------------------
/**
 * @brief A053634000 Fコード送信 : パワーセーブ
 */
//------------------------------------------------------------------------------
void PCPC_FaxSendCommSetFCode_box::procPowerSave( uchar option )
{
	checkSoft10KeyData();
	PMessageArea::procPowerSave( option );
}

//------------------------------------------------------------------------------.
/**
 *	@brief	A053634000 Fコード送信 : 暗証番号入力
 */
//------------------------------------------------------------------------------.
void PCPC_FaxSendCommSetFCode_box::procIDInput( uchar option )
{
	checkSoft10KeyData();
	PPopupDecisionButtonCom::procIDInput( option );
}

//関数--------------------------------------------------------------------------.
//概要    ：ソフトテンキー呼出しキー.
//備考    ：.
//戻値    ：無し.
//作成者  ：Sky_西原.
//レビュー：.
//変更履歴：.
//------------------------------------------------------------------------------.
/**
* @brief A053634000 Fコード送信 : ソフトテンキー呼出しキー.
*/
//------------------------------------------------------------------------------.
void PCPC_FaxSendCommSetFCode_box::procSoft10Key( uchar option )
{
	if( False == b_FunctionOffOnFlg || False == m_bl_isSelectedInputField ) {
		setBuzzerType( PCGE_BuzzerType_InputNg );		// 報知音設定.
		return;
	}
	// スライドレス、MTUI有効の場合.
	if( True == PLBC_NonSlideMultiTouchFunction::isEnable() ) {
		if( PNLE_FaxRbtSel1761InputKeySubAddress != us_FcodeFlg && PNLE_FaxRbtSel1761InputKeyPassword != us_FcodeFlg ) {
			setBuzzerType( PCGE_BuzzerType_InputNg );		// 報知音設定.
			return;
		}
		// ソフトテンキー表示(センター表示).
// comment 2015/09/16 OPS1 Sky溝部 [操作性改善] テンキー改善対応.
//		PnlComMtuiIf_TenkeyPad::instance()->showDefaultPos();
		PnlComMtuiIf_TenkeyPad::instance()->showDefaultPos( False, 0, 0, 0, 0, PMTE_MtuiIf_TenkeyPattern_SharpAster );
	} else {
		if( PNLE_Soft10KeyUnselected != PNLC_OutSideKeyCallDataLib::newInstance()->getSelectNo() ) { return; }

		Bool	bl_NonSetting = False;

		PNLC_OutSideKeyCallDataLib::newInstance()->setColorMode( PNLE_Soft10KeyColorModeBox );

		if( PNLE_FaxRbtSel1761InputKeySubAddress == us_FcodeFlg ) {	// SUBアドレス選択中.
			PNLC_OutSideKeyCallDataLib::newInstance()->setInputString( sc_LocalString1, PNLD_MaxFaxTxSubAddrInput, False );
			PNLC_OutSideKeyCallDataLib::newInstance()->setSelectNo( PNLD_BoxSUBAddSelected );
			PNLC_PopupListControl::newInstance()->addPopup( &gp_Soft10Key_SUBAddressBox, this );
		}
		else if( PNLE_FaxRbtSel1761InputKeyPassword == us_FcodeFlg ) {	// 送信ID選択中.
			bl_NonSetting = !b_isStringForFinishingPwdDisplay;			// パスワード確定済みなら再入力.
			PNLC_OutSideKeyCallDataLib::newInstance()->setInputString( sc_LocalString2, PNLD_MaxFaxTxFcodePwdInput, bl_NonSetting );
			PNLC_OutSideKeyCallDataLib::newInstance()->setSelectNo( PNLD_BoxTxIDSelected );
			PNLC_PopupListControl::newInstance()->addPopup( &gp_Soft10Key_PasswordBox, this );
		}
		else {
			setBuzzerType( PCGE_BuzzerType_InputNg );		// 報知音設定.
		}
	}
}

//関数--------------------------------------------------------------------------.
//概要    ：アクティブ状態の通知.
//備考    ：.
//戻値    ：無し.
//作成者  ：Sky_西原.
//レビュー：.
//変更履歴：.
//------------------------------------------------------------------------------.
/**
* @brief A053634000 Fコード送信 : アクティブ状態の通知.
*/
//------------------------------------------------------------------------------.
void PCPC_FaxSendCommSetFCode_box::procActivate()
{
	checkSoft10KeyData();

	// 親クラスを呼び出し.
	PPopupDecisionButtonCom::procActivate();
}

//関数--------------------------------------------------------------------------.
//概要    ：ソフトテンキーデータチェック.
//備考    ：.
//戻値    ：無し.
//作成者  ：Sky_西原.
//レビュー：.
//変更履歴：.
//------------------------------------------------------------------------------.
/**
 *	@brief	A053634000 Fコード送信 : ソフトテンキーデータチェック.
 */
//------------------------------------------------------------------------------.
void PCPC_FaxSendCommSetFCode_box::checkSoft10KeyData()
{
	char sc_No = PNLC_OutSideKeyCallDataLib::newInstance()->getSelectNo();

	if( PNLE_Soft10KeyUnselected != sc_No ) {
		APIC_Job* pc_Job = PnlComGetJobScanFax();
		if( NULL == pc_Job ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }
		TYPC_FaxAct* pc_FaxAct = &( pc_Job->getJobSpec()->pc_Fax->c_FaxAct );
		short ss_Len;
		// 入力値を取得して更新.
		switch( sc_No ) {
			case PNLD_BoxSUBAddSelected:
				PNLC_OutSideKeyCallDataLib::newInstance()->getInputString( sc_LocalString1, &ss_Len );
				pc_FaxAct->setTxSubAddr( sc_LocalString1 );
				Ctf_FaxSendCommSetFCode_boxSubAddress.setCursorPosition( ( short )strlen( sc_LocalString1 ) );
				b_SubInputDirect = False;
				break;
			case PNLD_BoxTxIDSelected:
				PNLC_OutSideKeyCallDataLib::newInstance()->getInputString( sc_LocalString2, &ss_Len );
				pc_FaxAct->setTxLocalPwd( sc_LocalString2 );
				Ctf_FaxSendCommSetFCode_boxPassword.setCursorPosition( ( short )strlen( sc_LocalString2 ) );
				b_PassInputDirect = False;
				b_isStringForFinishingPwdDisplay = False;			// テンキー入力したらパスワード入力済みを解除する.
				break;
			default:
				break;
		}
		// データのクリア.
		PNLC_OutSideKeyCallDataLib::newInstance()->setSelectNo( PNLE_Soft10KeyUnselected );
	} else {
		// 0時は、ソフトテンキーは無効.
	}
}

//------------------------------------------------------------------------------.
/**
 *	@brief	A053634000 Fコード送信 : IDキー入力
 */
//------------------------------------------------------------------------------.
void PCPC_FaxSendCommSetFCode_box::procIDInputVoice( uchar option )
{
	checkSoft10KeyData();
	PPopupDecisionButtonCom::procIDInputVoice( option );
}

/**
 * @brief	MTUIテンキーポップアップ表示処理.
 * @param	.
 */
void PCPC_FaxSendCommSetFCode_box::show10KeyPopup()
{
	if( True != PLBC_NonSlideMultiTouchFunction::isEnable() ) {
		if( True == PLBC_FuncVersionCheck::isOK_Type4() ) {
			procSoft10Key( 0 );
		} else {
			setBuzzerType( PCGE_BuzzerType_Off, True );		// ブザー鳴らさない.
		}
	} else {
		if( False == b_FunctionOffOnFlg ) {
			setBuzzerType( PCGE_BuzzerType_InputNg );		// 報知音設定.
		} else {
			// MTUIテンキー表示.
			const short PNLD_TenkeyPad_correct = 15;

			short ss_SizeX, ss_SizeY, ss_KeyPosX, ss_KeyPosY, ss_PosX, ss_PosY;
			Key_FaxSendCommSetFCode_boxSubAddressArea.getPosition( ss_KeyPosX, ss_KeyPosY );
			if( us_FcodeFlg == PNLE_FaxRbtSel1761InputKeySubAddress ) {
				ss_KeyPosX = static_cast<short>( ss_KeyPosX - ( PNLD_Pushgap ) );
				ss_KeyPosY = static_cast<short>( ss_KeyPosY - ( PNLD_Pushgap ) );
			}
			Key_FaxSendCommSetFCode_boxSubAddressArea.getSize( ss_SizeX, ss_SizeY );
			this->getPosition( &ss_PosX, &ss_PosY );
			// 表示位置（画面表示位置＋追加キー表示位置＋キー幅＋補正値：画面表示位置＋メッセージエリア表示位置＋幅）
			PnlComMtuiIf_TenkeyPad::instance()->show( static_cast<short>( ss_KeyPosX+ss_SizeX+PNLD_TenkeyPad_correct ), static_cast<short>( ss_PosY-PNLD_TenkeyPad_correct ) );
		}
	}
}
/**
 * @brief	MTUIテンキーポップアップ非表示処理.
 * @param	.
 */
void PCPC_FaxSendCommSetFCode_box::hide10KeyPopup()
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
void PCPC_FaxSendCommSetFCode_box::procSlideMenu( uchar option )
{
	// 10キー表示可否.
	Bool bl_Show10Key = IsShowSlide10Key();
	PNLC_SlideMenuControlLib::newInstance()->setTenKey( bl_Show10Key );
// comment 2015/10/09 OPS1 Sky溝部 [操作性改善] テンキー改善対応.
	PNLC_SlideMenuControlLib::newInstance()->setTenKeyPattern( PMTE_MtuiIf_TenkeyPattern_SharpAster );

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
Bool PCPC_FaxSendCommSetFCode_box::IsShowSlide10Key()
{
	if( False == b_FunctionOffOnFlg || False == m_bl_isSelectedInputField ) {
		return False;
	}

	if( PNLE_FaxRbtSel1761InputKeySubAddress != us_FcodeFlg && PNLE_FaxRbtSel1761InputKeyPassword != us_FcodeFlg ) {
		return False;
	}

	return True;
}

// ===============================================================================================.
//	概要	: スライドメニュー入力エリアデータ送信.
//  引数    : True：初期化あり、False：初期化なし.
//	戻り値	: なし.
// ===============================================================================================.
void PCPC_FaxSendCommSetFCode_box::sendSlide10KeyData( Bool bl_Flag )
{
	PNLE_OutsideKeyCallType e_type = PNLE_OutsideKeyCall_LEFT_SUBAddress;
	uchar*	puc_ptr = NULL;
	char sc_worktext[PNLD_MaxFaxTxLocalPwd+1];  // ワーク文字列.
	memset( sc_worktext, '\0', sizeof( sc_worktext ) );
	ushort us_StrLen = 0;

	if( PNLE_FaxRbtSel1761InputKeySubAddress == us_FcodeFlg ) {	// SUBアドレス選択中.
		e_type = PNLE_OutsideKeyCall_LEFT_SUBAddress;	//表示タイプ.
		us_StrLen = ( ushort )strlen( sc_LocalString1 );
		puc_ptr = ( uchar* )sc_LocalString1;
	} else if( PNLE_FaxRbtSel1761InputKeyPassword == us_FcodeFlg ) {	// 送信ID選択中.
		e_type = PNLE_OutsideKeyCall_LEFT_Password;		//表示タイプ.
		us_StrLen = ( ushort )strlen( sc_LocalString2 );		// OUTPUT：パスワード「＊」表示文字作成.
		memset( sc_worktext, '*', us_StrLen );
		puc_ptr = ( uchar* )sc_worktext;
	}

	if( True == bl_Flag ) {
		//スライドメニューOpen時の処理.
		PNLC_SoftSlideMenuTenKeyTextDispLib::newInstance()->create( e_type );
	}
// comment 2014/11/20 【HCL-Durga】【CQ-503465】Klockwork Changes ID: 23936 [NPD.GEN.MIGHT] added NULL check condition.
	if( NULL != puc_ptr ) {
	PNLC_SoftSlideMenuTenKeyTextDispLib::newInstance()->update( ( const uchar* )puc_ptr,						//文字列.
	        PNLD_CURSOR_TYPE_INSERT,	//カーソルタイプ.
	        us_StrLen,					//カーソルポジション.
	        True );						//ブリンク.

	}
}

// comment 2015/07/23 OPS1-Sky浜野 [MinervaSBK][制御構成変更No.75] パスワード入力文字表示対応.
//================================================================================================
// 概要		：A053634000 Fコード送信: パスワード入力表示開始処理.
//================================================================================================
void PCPC_FaxSendCommSetFCode_box::judgePassInsert( short ss_InsertPos )
{
	// パスワード入力文字表示設定が有効かチェック.
	if( False == PnlComCheckPassTimerDispSetting() ){
		return;
	}
	// パスワード入力の場合.
	m_ss_PassInsertPos = ss_InsertPos;
		
	PNLC_KeyboardTimerControlLib::newInstance()->startTimer( fixPassInput );
}

//================================================================================================
// 概要		：A053634000 Fコード送信: パスワード入力表示文字位置更新.
//================================================================================================
void PCPC_FaxSendCommSetFCode_box::adjustPassInsert( short ss_DeletePos, short ss_DeleteSize )
{
	// パスワード入力の場合.
	// 削除位置と同じなら入力表示を終了する.
	if( ss_DeletePos == m_ss_PassInsertPos ) {
		fixPassInput();
	// 削除位置が入力表示位置より前なら表示位置を更新する.
	} else if( ss_DeletePos < m_ss_PassInsertPos ) {
		m_ss_PassInsertPos = static_cast<short>( m_ss_PassInsertPos - ss_DeleteSize );
	}
}

//================================================================================================
// 概要		：A053634000 Fコード送信: パスワード入力表示解除.
//================================================================================================
void PCPC_FaxSendCommSetFCode_box::fixPassInput()
{
	m_ss_PassInsertPos = -1;

	PNLC_KeyboardTimerControlLib::newInstance()->cancelTimer();
}

