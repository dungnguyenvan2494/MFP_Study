/*================================================================================================
	インクルード・ファイル  
================================================================================================*/
#include <system/thr/thrInclude/THRC_PortInstance.h>
#include <system/sys/APIC_TabletIF.h>
#include <panel/pcgcom/pcgcomg/pcgcom.h>
// comment 2015/10/30 HCL_SunilNP [ZeusS_ZX0][制御構成変更_No.76-1] Adding error code.
#include <panel/pdp/pdpg/pdp_DetailCode.h>
#include <panel/pcgutil/pcgutil_CLCD/pigutility_CLCD.h>
#include <panel/pcgutil/pcgutil_CLCD/piputility_CLCD.h>
#include <panel/pcgcopy/pcgcopy_CLCD/pipcopy_CLCD.h>
#include <panel/pcgadm/pcgadm_CLCD/pipadmin1_CLCD.h>
#include <panel/pcgadm/pcgadm_CLCD/pipadmin3_CLCD.h>
#include <panel/pcgadm/pcgadm_CLCD/pigadmin1_CLCD.h>
#include <panel/pcgadm/pcgadm_CLCD/pigadmin2_CLCD.h>
#include <panel/pcgadm/pcgadm_CLCD/pigadmin3_CLCD.h>
#include <panel/pcgadm/pcgadm_CLCD/pigadmin4_CLCD.h>
#include <panel/pcgadm/pcgadm_CLCD/pigadmin5_CLCD.h>
#include <panel/pcgadm/pcgadm_CLCD/pigadmin6_CLCD.h>
#include "panel/pcgadm/pcgadm_CLCD/pigadmin7_CLCD.h"	// comment 2013/01/31 OPS1-Sky井柳 [FUM4][WBS No.253,254]管理者Myパネル対応.
#include <panel/pcgadm/pcgadm_CLCD/pigadminc_CLCD.h>
#include <panel/pcgadm/pcgadm_CLCD/pigadmin8_CLCD.h>
#include <panel/pcgutil/pcgutil_CLCD/piputility3_CLCD.h>	// 拡大キーボード対応で追加 06/06/29 11開発内田.
#include <panel/pcgadm/pcgadm_CLCD/pipadmindef_CLCD.h>
#include "pcgadmin1.h"
#include "pcgadmin3.h"
#include <panel/pcgcom/pcgcomg/pcgcom.h>
#include <panel/pddtext/pddtext_CLCD/pstr_CLCD.h>
#include <ctype.h> // for toupper
#include <panel/pcgutil/pcgutil_CLCD/piputilityc_CLCD.h>
#include <panel/pcgwarn/pcgwarng/pctwarng.h>
#include <system/doc/docg/APIC_BoxRegistryImp.h>
#include <panel/pcgact/pcgact_CLCD/pigact_CLCD.h>
#ifdef	DEF_THAMES_ICUSTOM
#include <panel/pcgadm/pcgadm_FUM2/pigadmin_FUM2.h>
#endif	// DEF_THAMES_ICUSTOM
const uchar DEF_BOOKMARK_PAPERSIZEKIND_COUNTERCLEAR					= uchar( 4 );
#include <panel/pcgadm/pcgadm_CLCD/pigadmin2_CLCD.h>
#include "system/sys/APIC_NetworkDevice.h"
#include "system/sys/APIC_SystemStatus.h"
#include "system/sys/APIC_SystemInfo.h"
#include "system/uac/APIC_UserAcount.h"
#include "system/uac/APIC_DNCacheManager.h"
#include <panel/plibh/plibhg/plibhCommonKeybord.h>		// キーボード処理共通化.
#include <panel/plibh/plibh_CLCD/plibhspcextern.h>
#include <panel/plibh/plibhg/plibhspc.h>
#include <system/igl/iglg/IGLC_ImageLogCommon.h>

#include <panel/plibh/plibhg/plibh.h>		// 履歴	2008/09/18	Sky小尾	[UV-00521 Email送信時のReply-toフィールドFromフィールドの修正要望].
#include <panel/pcgadm/pcgadm_CLCD/pigadmin_PKI.h>
#include <system/opl/APIC_OPL.h>																	// 送信操作ログ管理I/F.
#include <panel/plibh/plibh_CLCD2/plibh_CLCD2.h>
#include <panel/plibh/plibh_CLCD2/PNLC_AuthenticationManager.h>
#include <panel/pcgcom_ext/pcgcom_extg/pigIwsAuthBrowserLogin.h> //【IWS1.2 共通 CQ:360403】【IWS Authentication Screen Customization】【HCL_Mahendra】 2011/08/22 Add.
#include "panel/pcgact/pcgact_CLCD2/pigact_pF073000.h"
// Comment 2017/09/08【HCLT-Narsimhmaswamy】【IT5 4.2】【RQ686】【WP595】Multiple Network support.
#include <panel/plibh/plibh_CLCD2/PNLC_NetworkLan.h>

#define DEF_SystemSetHourMax  	24	// Maximum value for Hour.
#define DEF_SystemSetHourMin  	0	// Minimum value for Hour.
#define DEF_SystemSetMinuteMax	60	// Maximum value for Minute.
#define DEF_SystemSetMinuteMin	0	// Minimum value for Minute.
// 2016/05/07 [HCLT-Gadiya sai] [IT5-30]LDAPアプライアンス上でICカードのセルフレジスト+部門連携を動作させる対応にて、MZ-16の不要な処理を元に戻す.
//// comment 2013/08/29 OPS1-Sky奥村 [特Rom対応マージ][MZ-16] 日立ICC FeliCa/MifareカードAD直接連携 追加要望Ph1.
//#include <panel/plibh/plibh_CLCD2/PNLC_ActionUserLoginControlLib.h>

#include "panel/pcgcom/pcgcom_CLCD2/PNLC_NotifyEventInfo.h"

#include "system/sys/APIC_System_DEF.h"
// comment 2014/12/10 OPS1-Sky西原 [HeliosL][制御構成変更No.1]封筒通紙の操作性改善（定着手動離間）.
#include <panel/pcgadm/pcgadm_CLCD/pipadminc_CLCD.h>
// comment 2014/12/17 OPS1-Sky Kawanishi [ZeusS][CQ507643].
#include <panel/plibh/plibh_CLCD2/PNLC_ActionDataLib.h>

// comment 2015/04/10 【HCLT-Dhivya】 【MinervaSBk】 【制御構成変更_No.1】 Include file for MinervaSBk.
#include <panel/pcgadm/pcgadm_MinervaSBk/pipadmin_MinervaSBk.h>
#include <panel/pcgadm/pcgadm_MinervaSBk/pidadmin_MinervaSBk.h> 
#include <panel/pcgadm/pcgadm_MinervaSBk/pigadmin_MinervaSBk.h>
#include <panel/pcgadm/pcgadm_MinervaSBk/pcgadmin_MinervaSBk.h>
#include <panel/pcgact/pcgact_CLCD2/pigact_pA072002.h>		// comment 2015.12.19 OPS1-Sky西原 [ZeusS_ZX0][CQ518290] リセット押下で拒否音が鳴動する問題.
// comment 2016/01/18 OPS1-MSE今岡 [ZeusS_ZX0][CQ523862]リセットキー押下でFAX/SCAN画面遷移するとき拒否音が鳴動する問題.
#include <panel/pcgscanfax/pcgscanfax_CLCD2/pigscanfax_base.h>
#include <panel/pcgscanfax/pcgscanfax_enl_CLCD2/pigscanfaxenl_p10H00.h>
#include <Linux/Platform/InitMemory.h>
#include <panel/pcgscanfax/pcgscanfax_enl_PD/pcgscanfaxenl_10H00400.h>
extern PPD_10H00400 gp_PD_10H00400;
//Comment 2017/10/17 [HCL-Hanuman] [IT5 4.2][AR1030231] Defined PublicUser and BoxAdmin offset.
#define PNLD_PublicUserOffset 	(1)				// Public User Offset.
#define PNLD_BoxAdminOffset 	(2)				// Box Admin Offset.

//comment 2018/9/27 SCD2-Sky二口 [Eagle][AR1062689]管理者パスワード入力画面のHOMEキー押下対応.
#include <panel/pcgbrowser_ext/pcgbrowser_extg/pigCostCenterResetDummy.h>
#include <panel/pcgcom_ext/pcgcom_extg/PCGC_PublicPopupDummyBase.h>
#include <panel/pcgact_ext/pcgact_ext_CLCD2/pigact_pA072002_ext.h>
// Comment 2019/01/24 【HCL_Krishna】【IT6_EagleH】【RQ-506】[WP-1711] include the header file.
#include "../../shlib/RecoveryLib/LIBC_RecoveryLib_IF.h"
// comment 2022/07/25 Sky 前田  [IT6-89(機種マージ)]  USBドングルを使用したMFPロック機能対応.
#include <panel/pcgwarn/pcgwarn_CLCD2/pigwarn_Dongle.h>
// comment 2023/03/03 FPT.ThanhLH11 [IT6_2.3][ALL MACHINE][OP_BTS-41923]一時的な言語変更で、OKキーを押した後に機械語で表示される管理者パスワード画面.
#include "../../../../divlib/Common/blkapi/blkshm.h"

/*================================================================================================
	モジュール内変数の宣言
================================================================================================*/
#define PCGD_PrinterOrganizationNameDispRow     (2)     	// display of Row.
#define PCGD_PrinterOrganizationNameDispCol     (50)     	// display of Column.
PAdminToSystemSetcom* PAdminToSystemSetcom::retGamen = NULL;	// テストコピー後の戻り画面.
TYPE_CommLine PToAdminSetMenucom::e_FaxSettingListFaxLineNo = (TYPE_CommLine)( TYPE_CommLineAll );	// FAX設定値リスト印字回線.
PNLE_AdminPasswordNextGamen PAdminPasswordError::m_e_PassErrorNextGamen = PNLE_AdminPasswordMoveGamen_AdminSetting;
PNLE_AdminPasswordNextGamen PAdminPassword::m_e_NextGamen = PNLE_AdminPasswordMoveGamen_AdminSetting;
PNLE_BoxDeleteMode		PAdminToUserAndDepartSetcom::e_BoxDeleteMode			= PNLE_BoxDeleteMode_Non;	// ボックス削除モード
PBasicMessageProc*		PAdminToUserAndDepartSetcom::pc_BoxDeleteReturnGamen	= NULL;						// 戻り先画面.
LIBC_List<APIC_BoxList>	PAdminToUserAndDepartSetcom::c_BoxDeleteList;										// リスト.
APIC_User* 				PAdminToUserAndDepartSetcom::pc_BoxDeleteUser			= NULL;						// 削除ユーザー情報.
APIC_Track*				PAdminToUserAndDepartSetcom::pc_BoxDeleteTrack			= NULL;						// 削除部門情報.
Bool					PAdminToUserAndDepartSetcom::bl_BoxProcess				= False;
Bool					PAdminToUserAndDepartSetcom::m_b_AuthCacheMode 			= False;					// 認証情報の一時保存.

uchar   PToAdminSetMenucom::uc_FromNetworkMode = PNLE_AdminFromNetworkSetting;	// ネットワーク設定から遷移か拡張ネットワーク設定から遷移かのフラグ.
void ans_AdminDocByUserDeleteUser( int32_t i32_ret,int32_t i32_ret2,int32_t i32_ret3 );
TYPE_ListPrintInfo PToAdminSetMenucom::e_ListPrintInfo = TYPE_ListPrintInfoAll;
Bool PToAdminSetMenucom::bl_isCounterDetails = False;
// comment 2023/03/03 FPT.ThanhLH11 [IT6_2.3][ALL MACHINE][OP_BTS-41923]一時的な言語変更で、OKキーを押した後に機械語で表示される管理者パスワード画面.
Bool PAdminPasswordWide::m_bl_pendingChangeLanguage = False;
//	comment 2017/06/06 SCD2-sky延原 [IT5_4.1][AR1022609] AR1010594追加対策.
void ans_deleteConfirmationPrintAllUser( int32_t si32_Result );
//2015/07/13 SecV1-5 【管理者によるサービスモード制御技術】 Sky 新道.
#define PNLD_AdminPasswordReleaseKeyErrorDisplayTime	5			// 管理者パスワード解除キー画面エラー表示時間.
#define PNLD_AdminPasswordReleaseKeyErrorNoDisplay		0			// 管理者パスワード解除キー入力通常表示状態.
#define PNLD_AdminPasswordReleaseKeyInputMax			30			// 管理者パスワード解除キー入力最大数.
#define PNLD_AdminPasswordReleaseKeyBufSize				(TYPD_AdminReleaseKeyLength * PCGD_BuffSize_UTF8 + PCGD_EndCode)	// 管理者パスワード解除キーバッファサイズ.

/*================================================================================================
// 概要		：管理者モード共通 : 全画面警告(ERROR､CAUTION)発生／解除.
// 戻り値	：なし.
================================================================================================*/
void PAdmincom::procWarningGamenExist( uchar option )
{
	static ushort DataCheck	= 0;					// 全画面警告の状態
	TYPE_WarningName	ec_UtililyWarningName = 0;		// 警告コード
	DataCheck = onlyPanel.isAllGamenWarning( comPanelJob );	// comPanelJobの全画面警告
	if( ( sizeof( sp_GamenWarningTable )/sizeof( sp_GamenWarningTable[0] ) ) > DataCheck ) {
		ec_UtililyWarningName = sp_GamenWarningTable[ DataCheck ].ec_WarningName;	// 発生した警告.
	}
	if( ec_UtililyWarningName == PNL_ERROR_CardReset ) {	// 管理カード再セット警告.
		// 管理カード再セット警告発生時は画面遷移させない.
		// 設定した警告の状態を解除.
		PNLC_WarningManager::newInstance()->setWarningDetectSts( False,PNLE_WarningDetectSts_All );
	} else {
// comment 2018/10/01 Sky雜賀 [AR1061503].
//		// 上記以外の場合、状態遷移を行う.
//		PMessageProcCom::procWarningGamenExist( option );
		if ( ( PNLE_AdminPasswordMoveGamen_SimpleSecuritySetting != PNLC_TransitionInfoCom::getNextGamenPasswordEntry() )
		 || ( TYPE_WarningName_Non != ec_UtililyWarningName ) ) {
			PMessageProcCom::procWarningGamenExist( option );
		}
	}
}
/*================================================================================================
［機能］	管理者モード共通 : 部門禁止設定チェック
================================================================================================*/
Bool PAdmincom::checkBumonDisable()
{
	return True;	// 禁止になった
}

/*================================================================================================
［機能］	管理者モード共通 : 外キー押下時の処理
================================================================================================*/
void PAdmincom::procAdminExternalButton( Bool b_SetBuzzer )
{
// comment 2015.12.19 OPS1-Sky西原 [ZeusS_ZX0][CQ518290] リセット押下で拒否音が鳴動する問題.
//	if( nextGamen != NULL ) {
	if( ( nextGamen != NULL )
	||  ( True == PNLC_PopupListControl::newInstance()->isNowPopup( &gp_HomeTopMenu ) ) ) {		// トップメニューに遷移する場合も含める.
		setAdminGamenExit();	// 管理者モードを抜ける時の処理
		if( onlyPanel.bl_isEnlargeDisplayMode == True ) {	// 拡大表示モード時
			rebuildGamenEnlarge( 0 );	// 状態に応じた画面表示（画面復帰無し）
		} else {
			rebuildGamen( 0 );			// 状態に応じた画面表示（画面復帰無し）
		}
	} else {
		if( True == b_SetBuzzer ) {
// comment 2016/01/18 OPS1-MSE今岡 [ZeusS_ZX0][CQ523862]リセットキー押下でFAX/SCAN画面遷移するとき拒否音が鳴動する問題.
			if( ( True == pc_ManageIGCWaitDisplay.getDisplayFLG() ) 	// 「お待ちください」P.U.
			&& 	( ( pc_ManageIGCWaitDisplay.getnextIGCGamen() == &gp_FromRegAddress_scanfax) || (( pc_ManageIGCWaitDisplay.getnextIGCGamen() == &gp_PD_10H00400) ) ) ) {
				setBuzzerType( PCGE_BuzzerType_InputOk );
				return;
			}
// comment 2022/11/14 FPT.ThanhLH11 [IT6_2.2MR4][ALL MACHINE][OP_BTS-40002]管理者パスワード画面で、電源キーを押下し、拒否音が鳴る問題対応.
			if( True == isNowGamen( &gp_AdminPassword ) ){
				setBuzzerType( PCGE_BuzzerType_InputOk );
				return;
			}
//comment 2015/03/02 OPS1-Sky稲井 [特ROM対応][MZV-35_機種マージ] IWS APP画面の個別対応.
// comment 2023/04/21 FPT.QuyenTT9 [IT6_2.3] [OP_BTS-43060] Fixed a bug that returned a rejection sound when pressing the Reset button, back to  Home screen.
//			if ( ( False == APIC_SoftDipSw::newInstance()->getSoftDipSwBitForPanelSpecNo( DEFSOFTWARESW165, 0 ) )
//			  || ( PNLE_BrowserLoginNoticeRebootState_Wait != c_ExtPanel.getBrowserLoginNoticeRebootState() ) ) {	// ブラウザのReboot待ちの状態以外の場合.
			if ( ( True == APIC_SoftDipSw::newInstance()->getSoftDipSwBitForPanelSpecNo( DEFSOFTWARESW165, 0 ) )
				&& ( PNLE_BrowserLoginNoticeRebootState_Wait != c_ExtPanel.getBrowserLoginNoticeRebootState() ) ) {	// ブラウザのReboot待ちの状態以外の場合.
				setBuzzerType( PCGE_BuzzerType_InputNg );	// 報知音設定
			} else {
				setBuzzerType( PCGE_BuzzerType_InputOk );	// 報知音設定
			}
		}
	}
}
/*================================================================================================
［機能］	管理者モード共通 : カウンタ引き抜き通知
================================================================================================*/
void PAdmincom::procResetAtCountNotice( uchar option )
{
	if( onlyPanel.bl_PowerOffOnMesage == False ) {
		setAdminGamenExit();	// ユーザ認証有り時、procAutoResetReq()でユーザをログアウトされる為、管理者を抜ける操作を先に行う.
		PUtilitycom::procReset( 0 );
	}
}
/*================================================================================================
［機能］	管理者モード共通 : 省エネ解除.
================================================================================================*/
void PAdmincom::procEnergySaveReset( uchar option )
{
	PUtilitycom::procEnergySaveReset( option );
	procAdminExternalButton();
}
/*================================================================================================
［機能］	管理者モード共通 : ユーザーモード
================================================================================================*/
void PAdmincom::procUtility( uchar option )
{
	setAdminGamenExit();	// ユーザ認証有り時、procAutoResetReq()でユーザをログアウトされる為、管理者を抜ける操作を先に行う.
	PUtilitycom::procUtility( option );
// アプリキーの割り付け方が変更されたときのために、LEDを一度全てONにして、再度OFFにする.
#if defined(DEF_THAMES_ICUSTOM) && defined(LINUX)
	checkLED();
#else
	reqLED( PNLE_LED_Box,    		PNLE_LEDStateOn );
	reqLED( PNLE_LED_Scaner, 		PNLE_LEDStateOn );
	reqLED( PNLE_LED_Copy,  		PNLE_LEDStateOn );
	reqLED( PNLE_LED_Fax,  			PNLE_LEDStateOn );
	reqLED( PNLE_LED_ImageDealer,	PNLE_LEDStateOn );
	reqLED( PNLE_LED_MyPanel,		PNLE_LEDStateOn );
	reqLED( PNLE_LED_Browser,		PNLE_LEDStateOn );
	reqLED( PNLE_LED_AppMenu,		PNLE_LEDStateOn );

	reqLED( PNLE_LED_Box,    		PNLE_LEDStateOff );
	reqLED( PNLE_LED_Scaner, 		PNLE_LEDStateOff );
	reqLED( PNLE_LED_Copy,  		PNLE_LEDStateOff );
	reqLED( PNLE_LED_Fax,  			PNLE_LEDStateOff );
	reqLED( PNLE_LED_ImageDealer,	PNLE_LEDStateOff );
	reqLED( PNLE_LED_MyPanel,		PNLE_LEDStateOff );
	reqLED( PNLE_LED_Browser,		PNLE_LEDStateOff );
	reqLED( PNLE_LED_AppMenu,		PNLE_LEDStateOff );
#endif
}
/*================================================================================================
［機能］	管理者モード共通 : リセットキー
================================================================================================*/
void PAdmincom::procReset( uchar option )
{
	setAdminGamenExit();	// ユーザ認証有り時、procAutoResetReq()でユーザをログアウトされる為、管理者を抜ける操作を先に行う.
// comment 2022/07/25 Sky 前田  [IT6-89(機種マージ)]  USBドングルを使用したMFPロック機能対応.
	if (True == PnlComGetSoftDipSw()->getSoftDipSwBitForPanelSpecNo( 244, 3 )
		&& True == onlyPanel.isWarningOn(comPanelJob, PNL_ERROR_InsertTheUSBKey)
	) {
// comment 2022/12/16 Sky 前田  [OP_BTS-41095] 拡大表示時、副電源キーでSleep前に警告画面が表示される問題.
		TYPE_PowerSaveStatus e_PowerStatus = TYPE_PowerSaveStatusFirstBootConfigration;
		if( NULL != ppc_Power ) {
			e_PowerStatus = ppc_Power->getPowerSaveStatus();
		} else {
			// 処理なし.
		}
		if( False == PnlComIsPowerSaveMode( e_PowerStatus ) ) {	// パワーセーブモード以外.
			// Dongle認証_ユーザー情報取得エラー表示.
			setBuzzerType( PCGE_BuzzerType_Error );
			PNLC_PopupListControl::newInstance()->addPopup( &gp_InsertTheUSBKey_warn, this, NULL, 400, 240, 0, 0 );
			return;
		} else {
			// 処理なし.
		}
	}
	PUtilitycom::procReset( option );
	Bool b_BuzzerSet = True;
	// システムオートリセットからのReset要求時は、Buzzer音を鳴らさない.
	if( 100 == option ) {
		b_BuzzerSet = False;
	}
	procAdminExternalButton( b_BuzzerSet );
// アプリキーの割り付け方が変更されたときのために、LEDを一度全てONにして、再度OFFにする.
#if defined(DEF_THAMES_ICUSTOM) && defined(LINUX)
	checkLED();
#else
	reqLED( PNLE_LED_Box,    		PNLE_LEDStateOn );
	reqLED( PNLE_LED_Scaner, 		PNLE_LEDStateOn );
	reqLED( PNLE_LED_Copy,  		PNLE_LEDStateOn );
	reqLED( PNLE_LED_Fax,  			PNLE_LEDStateOn );
	reqLED( PNLE_LED_ImageDealer,	PNLE_LEDStateOn );
	reqLED( PNLE_LED_MyPanel,		PNLE_LEDStateOn );
	reqLED( PNLE_LED_Browser,		PNLE_LEDStateOn );
	reqLED( PNLE_LED_AppMenu,		PNLE_LEDStateOn );

	reqLED( PNLE_LED_Box,    		PNLE_LEDStateOff );
	reqLED( PNLE_LED_Scaner, 		PNLE_LEDStateOff );
	reqLED( PNLE_LED_Copy,  		PNLE_LEDStateOff );
	reqLED( PNLE_LED_Fax,  			PNLE_LEDStateOff );
	reqLED( PNLE_LED_ImageDealer,	PNLE_LEDStateOff );
	reqLED( PNLE_LED_MyPanel,		PNLE_LEDStateOff );
	reqLED( PNLE_LED_Browser,		PNLE_LEDStateOff );
	reqLED( PNLE_LED_AppMenu,		PNLE_LEDStateOff );
#endif
}
/*================================================================================================
［機能］	管理者モード共通 : コピーキー
================================================================================================*/
void PAdmincom::procCopy( uchar option )
{
	PUtilitycom::procCopy( option );
	procAdminExternalButton();
}
/*================================================================================================
［機能］	管理者モード共通 : FAXキー
================================================================================================*/
void PAdmincom::procFax( uchar option )
{
	PUtilitycom::procFax( option );
	procAdminExternalButton();
}

/*================================================================================================
［機能］	管理者モード共通 : FAXキー
================================================================================================*/
void PAdmincom::procFaxOnly( uchar option )
{
	PUtilitycom::procFaxOnly( option );
	procAdminExternalButton();
}


/*================================================================================================
［機能］	管理者モード共通 : ボックスキー
================================================================================================*/
void PAdmincom::procBox( uchar option )
{
	PUtilitycom::procBox( option );
	procAdminExternalButton();
}
/*================================================================================================
［機能］	管理者モード共通 : ユビキタスプリントキー.
================================================================================================*/
void PAdmincom::procUbqPrint( uchar option )
{
	PUtilitycom::procUbqPrint( option );
	procAdminExternalButton();
}
/*================================================================================================
［機能］	管理者モード共通 : ユーザ補助キー
================================================================================================*/
void PAdmincom::procAccessibility( uchar option )
{
	PUtilitycom::procAccessibility( option );
	setAdminGamenExit();
	PUtilitycom::setUtilityGamenExit();						// ユーティリティを抜ける時の処理
	c_autoRestTimer.stopEachJobAutoResetTimer();
}
/*================================================================================================
［機能］	管理者モード共通 : パワーセーブキー
================================================================================================*/
void PAdmincom::procPowerSave( uchar option )
{
	// HDDビジー状態はパワーセーブ移行不可。管理者以外でも必要な場合はPUtilitycomへ.
	APIC_HddInfo* pc_HddInfo = APIC_HddInfo::newInstance();
	if( True == pc_HddInfo->getHddBusy() ) {
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}
// procAdminExternalButton()を呼び出すと、警告ブザーが鳴ってしまうため使用しない
// comment 2014/12/15 【HCLT-Deeparaj】 【CQ-505565】Commenting the compiler switch inorder to raise NG buzzer sound for Powersave button press.
//#ifndef DEF_CLCD
	procAdminExternalButton();
//#endif
	PUtilitycom::procPowerSave( option );
}
/*================================================================================================
［機能］	管理者モード共通 : プレヒートキー押下応答.
================================================================================================*/
void PAdmincom::procPreHeatOnByKeyAns( uchar option )
{
	PUtilitycom::procPreHeatOnByKeyAns( option );
	procAdminExternalButton();
}
/*================================================================================================
［機能］	管理者モード共通 : オートリセットを抑制.
================================================================================================*/
void PAdmincom::procAutoResetReq( uchar option )
{
	onlyPanel.bl_ManualLogOut = False;							// 自動ログアウト.				// comment ZeusMLK Sky増田 2012/04/16 [UV-01212]追加対応.
	setAdminGamenExit();	// ユーザ認証有り時、procAutoResetReq()でユーザをログアウトされる為、管理者を抜ける操作を先に行う.
	PUtilitycom::procAutoResetReq( option );
	onlyPanel.bl_ManualLogOut = True;							// 手動ログアウトフラグ初期化.	// comment ZeusMLK Sky増田 2012/04/16 [UV-01212]追加対応.
}
//================================================================================================
//［機能設定］	管理者モード共通 : オートリセットOFF時の１分タイマー処理要求.
//================================================================================================
void PAdmincom::procAutoResetOffTimerReq()
{
// comment 2023/02/21 Sky中野 [IT6_2.3][OP_BTS-41732].	
	// 初期表示アプリ選択処理.
	PnlComSetInitIndicationApplication();
	onlyPanel.bl_isEnlargeDisplayMode = PnlComIsLargeDisplayInitDisp();
	
	onlyPanel.bl_ManualLogOut = False;							// 自動ログアウト.				// comment ZeusMLK Sky増田 2012/04/16 [UV-01212]追加対応.
// comment 2023/02/21 Sky中野 [IT6_2.3][OP_BTS-41732].	
	setAdminGamenExit();	// ユーザ認証有り時、procAutoResetOffTimerReq()でユーザをログアウトされる為、管理者を抜ける操作を先に行う.
//	procReset( 0 );
	PUtilitycom::procAutoResetOffTimerReq();
	procReset( 0 );
	onlyPanel.bl_ManualLogOut = True;							// 手動ログアウトフラグ初期化.	// comment ZeusMLK Sky増田 2012/04/16 [UV-01212]追加対応.
}

/*================================================================================================
［機能］	管理者モード共通 : ジョブ操作／基本画面キー
================================================================================================*/
void PAdmincom::procJobControl( uchar option )
{
	setAdminGamenExit();	// ユーザ認証有り時、procAutoResetReq()でユーザをログアウトされる為、管理者を抜ける操作を先に行う.
	PUtilitycom::procJobControl( option );
}
/*================================================================================================
［機能］	管理者モード共通 : 定期起床
[変更履歴]:2012/10/03 OPS1-SKY 稲井 [ZeusMLK][CQ370794] 上限値カウンタオーバー解除時、警告音を止める処理を追加.
================================================================================================*/
void PAdmincom::procTimerWakeUp( uchar option )
{
	PUtilitycom::procTimerWakeUp( option );
	// トップメニュー表示時は、以降の制御を行わない.
// Comment 2016/05/21 HCLT-Prasanjeet [ZeusSBK] [CQ526643] Adding the condition check for  Popup topmenu screen.
//  When topmenu screen  in popup layer and warning occur procUtilityEnableJobStart() should not call.
//  This chages done for the defect CQ 360001
//	if( onlyPanel.isTopmenuMode() ) {
	if(( True == PNLC_PopupListControl::newInstance()->isPopupLayer( &gp_HomeTopMenu )) ||((True == PNLC_WarningManager::newInstance()->getTopMenuMitipleStatus()))) {
		return;
	}
	static   Bool bl_PreActive3 = True;	// パターン３（管理者モード）の、前回の受付状態
	register Bool bl_NowActive3;		// パターン３（管理者モード）の、今回の受付状態
	if( isWarningCheckNormal() == False ) {
		bl_NowActive3 = True;
	} else if( bl_BumonLimitWarningAdminMove == True && isWarningCheckBumonLimit() == True ) {
		bl_NowActive3 = True;
	} else if( bl_MagCardWarningAdminMove    == True && isWarningCheckMagCard()    == True ) {
		bl_NowActive3 = True;
	} else if( bl_HddPasswordWarningAdminMove == True && PnlComGetHddInfo()->getLockStatus() == TYPE_HddLockStatus_Locked ) {
		bl_NowActive3 = True;
	} else if( bl_HddEncryptionKeyNGAdminMove == True && ppc_SystemStatus->isEncryptionKeyNG() == True ) {
		bl_NowActive3 = True;
	} else if( bl_FullJobLogNGAdminMove == True && onlyPanel.isWarningOn( comPanelJob, PERROR_FullJobLog ) == True ) {	// No.306018 2008/7/2 12柳川.
		bl_NowActive3 = True;
	} else if( bl_ImageLogOverError == True && onlyPanel.isWarningOn( comPanelJob, PNL_ERROR_ImageLogOverError ) == True ) {	// No.306018 2008/7/2 12柳川.
		bl_NowActive3 = True;
	} else {
		bl_NowActive3 = False;
	}
	if( bl_BumonLimitWarningAdminMove == True && isWarningCheckBumonLimit() == False ) {
		// 上限値オーバーのフラグがTrueなのに、上限値のワーニングが出ていない(上限値オーバーを解除したとき).
		bl_NowActive3 = False;
		setBuzzerType( PCGE_BuzzerType_StopBuzzer );
	}
// No310197対応_警告禁止画面時は画面遷移状態をチェックできなくなるため、Preへ値を設定しない.
	if( False == isNoWarningGamen ) {	// 警告禁止画面でない時.
		if( bl_PreActive3 != bl_NowActive3 && False == bl_NowActive3 ) {
			procUtilityEnableJobStart( 3 );	// 管理者モード受付禁止状態発生（パターン３）.
		}
		bl_PreActive3 = bl_NowActive3;
	}
}
/*================================================================================================
［機能］	管理者モード共通 : ユーティリティメニュー受付禁止ジョブ動作開始
================================================================================================*/
void PAdmincom::procUtilityEnableJobStart( uchar option )
{
	if( option != 3 && option != 4 ) { return; }	// パターン３、又は、４以外の変化は無視する
	
// comment 2016/06/13 SCD2-Sky浜野 [ZeusSBK][CQ530632].
	// 再起動フラグがONの場合は、処理しない.
	if( True == onlyPanel.bl_PowerOffOnMesage ) {
		return;
	}
	
// comment 2016/11/21 SCD2-Sky辻 [特ROM][FQ-OPSD-1600383-OFP252][CQ534112] 設定メニュー選択時に管理者パスワードをかけたマシンで、本体の前ドアを開閉するとパスワード無しでログインできる. 
	if ( ( True == gp_AdminPassword.getAdminPassGamenToDisplay() || PNLE_AdminPasswordMoveGamen_SimpleSecuritySetting == gp_AdminPassword.getNextGamen() )
		&& ( PBasicMessageProc::nowGamen->isNowGamen( &gp_AdminPassword ) || PBasicMessageProc::nowGamen->isNowGamen( &gp_AdminPasswordError ) ) ) {
			// 設定メニューへ遷移する際のパスワード入力画面or簡単セキュリティーへの遷移する際のパスワード入力画面の場合はまだ管理者に入っていないので以下の処理を抜ける.
		return;
	}
	
	setAdminGamenExit();				// 管理者モードを抜ける時の処理
// comment 2016/04/15 [HCLT_Prasanjeet] [ZeusSBK][CQ527139] Added popup screen check.
//	if( onlyPanel.ep_ForceChangeGamen != EP_ForceChangeGamen_Non && nextGamen != NULL ) {
	if ( (EP_ForceChangeGamen_Non != onlyPanel.ep_ForceChangeGamen)
	   && ( NULL != nextGamen
	   || PNLC_PopupListControl::newInstance()->isPopupOpen() ) ) {
		setTurnGamenThis( &gp_UtilityMenu );	// 戻りをユーティリティメニューとする
		// 指定アプリを再起動する必要があれば、ここで行う.
		SpecifiedAppStartingTrans( PNLD_SPECIFIEDAPP_NOT_REBUILD );
	} else {
		setTurnGamenThis( NULL );				// 前の画面に戻らないようにする
		nextGamen = &gp_UtilityMenu;			// ユーティリティメニューへ遷移
	}
	bl_UnSetTurnGamen = True;					// 現在の画面を戻り画面としてセットしない
}
/*================================================================================================
［機能］	管理者モード・ネットワーク設定・IPアドレス共通設定：IPアドレスをASCIIコードで格納する共有データへのセット
================================================================================================*/
void PAdmincom::SetIPAdressToAsciiCodeComdata( uchar* uc_Localdata, char* sc_ComData )
{
	register short _sc_setPosCount = 0;		// 設定する位置のカウンタ
	register uchar _uc_tmpValue;				// 数値を１桁取り出す一時データ
	register Bool bl_3Keta = False;			// ３桁のIPアドレスであることを示す
	for( register short i=0 ; i<4 ; i++ ) {
		if( ( _uc_tmpValue = ( uchar )( ( uc_Localdata[i]%1000 )/100 ) ) > 0 ) {	// １０進数で１００の位のみ取りだし
			sc_ComData[_sc_setPosCount] = (char)( _uc_tmpValue+0x30 );		// 共有データにセット
			_sc_setPosCount++;										// セット位置カウンタをインクリメント
			bl_3Keta = True;										// ３桁目の数値が存在することを示す
		}
		if( ( _uc_tmpValue = ( ( uc_Localdata[i]%100 )/10 ) ) > 0 || bl_3Keta == True ) {	// １０進数で１０の位のみ取りだし
			sc_ComData[_sc_setPosCount] = (char)( _uc_tmpValue+0x30 );		// 共有データにセット
			_sc_setPosCount++;										// セット位置カウンタをインクリメント
		}
		sc_ComData[_sc_setPosCount] = (char)( uc_Localdata[i]%10+0x30 );		// １０進数で１の位のみ取りだし 共有データにセット
		_sc_setPosCount++;										    // セット位置カウンタをインクリメント
		if( i != 3 ) {
			sc_ComData[_sc_setPosCount] = '.';							// アドレスの区切り「.」を入れる
			_sc_setPosCount++;										// セット位置カウンタをインクリメント
		}
	}
	sc_ComData[_sc_setPosCount] = '\0';			// 終端コードを入れる
}
/*================================================================================================
［機能］	管理者モード・ネットワーク設定・IPアドレス共通設定：IPアドレスをASCIIコードで格納する共有データへのセット
================================================================================================*/
Bool PAdmincom::GetIPAdressFromAsciiCodeComdata( uchar* uc_LocalData, char* sc_ComData )
{
	register short _ss_Point = 0;			// IPアドレス文字列ポインター（１文字目から）
	register short _ss_Check;				// 上限値チェック用の変数
	for( register short i=0 ; i<4 ; i++ ) {
		uc_LocalData[i] = 0;	// アドレス値を０にしておく
		while( sc_ComData[ _ss_Point ] != '.' && sc_ComData[ _ss_Point ] != '\0' ) {
			// ピリオドの切れ目か、文字列が無くなるまで１文字ずつピックアップする
			if( sc_ComData[ _ss_Point ] >= '0' && sc_ComData[ _ss_Point ] <= '9' ) {	// 有効文字列（数値）
				_ss_Check  = ( short )( ( uc_LocalData[i] * 10 ) + ( sc_ComData[ _ss_Point ] - '0' ) );
				if( _ss_Check <= 255 ) {	// 足した値が２５５以下
					uc_LocalData[i] = ( uchar )_ss_Check;
				} else {					// 越える場合はループを抜ける
					break;
				}
			}
			_ss_Point++;	// ポインタ進める
		}
		// ピリオドで抜けた場合は、ポインタを進める
		if( sc_ComData[ _ss_Point ] == '.' ) { _ss_Point++; }
	}
	return True;
}
/*================================================================================================
［機能設定］	管理者モード共通 : 画面終了処理
================================================================================================*/
void PAdmincom::endGamen()
{
	PUtilitycom::endGamen();
	if( onlyPanel.ep_ForceChangeGamen == EP_ForceChangeGamen_Warning
	    && onlyPanel.isWarningOn( comPanelJob, PNL_ERROR_HistoryError ) == False
	    && onlyPanel.isWarningOn( comPanelJob, PNL_ERROR_USBKeyBoardDisconnected ) == False
	    && onlyPanel.isWarningOn( comPanelJob, PNL_ERROR_BrowserAuthUnable ) == False
	    && onlyPanel.isWarningOn( comPanelJob, PNL_ERROR_AuthExpireDate ) == False
	    && bl_UnSetTurnGamen == False ) {
		// 履歴警告以外の、警告による遷移は画面を抜ける処理をする。
		setAdminGamenExit();
		setTurnGamenThis( &gp_UtilityMenu );	// 警告によって管理者モードを抜けるので、戻りをユーティリティメニューとする
	}
	// 管理者モードを抜けて、ユーティリティメニュー画面に遷移する場合は、フラグを立てる.
	if( c_SystemControl.getAdminModeOn() == False && nextGamen == &gp_UtilityMenu ) {
		bl_AdminModeToUtilityMove = True;				// 管理者モードからユーティリティへ遷移したフラグ.
	}
}
#ifdef	  DEF_THAMES_ICUSTOM	// アプリ割付.
/**
=======================================================================
@name	procBrowser.
@brief	管理者モード共通 : 外キー処理[ブラウザ].
@param	uchar 	option	(I）	オプション.
@return	なし.
=======================================================================
*/
void PAdmincom::procBrowser( uchar option )
{
	PUtilitycom::procBrowser( option );
	procAdminExternalButton();
}
/*=======================================================================
@name	procAPPsMenu.
@brief	管理者モード共通 : 外キー処理[登録アプリケーションメニュー].
@param	uchar 	option	(I）	オプション.
@return	なし.
@note	[レビュー] 未実施.
@date	2011/09/21	坂口	新規作成.
=======================================================================*/
void PAdmincom::procAPPsMenu( uchar option )
{
	PUtilitycom::procAPPsMenu( option );
	procAdminExternalButton();
}
#endif // DEF_THAMES_ICUSTOM	// アプリ割付.

// ===============================================================================================.
//	概要	: スライドメニュー 登録アプリケーションキー押下.
//  引数    : オプション.
//	戻り値	: なし.
// ===============================================================================================.
void PAdmincom::procSpecifyApp( uchar option )
{
	PUtilitycom::procSpecifyApp( option );
	procAdminExternalButton();
}

/*=======================================================================
@name	procTabletDisconnect.
@brief	管理者モード共通 : タブレット端末切断通知.
@param	uchar 	option	(I）	オプション.
@return	なし.
@note	[レビュー] 未実施.
@date	2012/06/01	嶌岡	新規作成.
=======================================================================*/
void PAdmincom::procTabletDisconnect( uchar option )
{
	PUtilityToMenuCom::procTabletDisconnect( option );		// 2012/12/19 Sky本橋 操作性改善FUMVer4(WBS No.62).

	// 終了処理.
	procForceEnd( option );

}

/*=======================================================================
@name	procTabletConnect.
@brief	管理者モード共通 : タブレット端末接続通知.
@param	uchar 	option	(I）	オプション.
@return	なし.
@note	なし.
@date	2014/01/20 OPS1-Sky浜野 [3rdVerup][制御構成変更_No.8 bizhubRA市場指摘対応][WP241_345].
=======================================================================*/
void PAdmincom::procTabletConnect( uchar option )
{
	PUtilityToMenuCom::procTabletConnect( option );		// 2012/12/19 Sky本橋 操作性改善FUMVer4(WBS No.62).

	// 終了処理.
	procForceEnd( option );
}

/*=======================================================================
@name	procForceEnd.
@brief	管理者モード共通 : 強制終了処理.
@param	uchar 	option	(I）	オプション.
@return	なし.
@note	なし.
@date	2014/01/20 OPS1-Sky浜野 [3rdVerup][制御構成変更_No.8 bizhubRA市場指摘対応][WP241_345].
=======================================================================*/
void PAdmincom::procForceEnd( uchar option )
{
	if( False == onlyPanel.bl_PowerOffOnMesage ) {			//主電源OFF/ON画面から遷移させないためのフラグ.
		if( c_SystemControl.getAdminModeOn() == True ) {	// 管理者モード中は抜ける処理をする.

			setAdminGamenExit();

			if( True == onlyPanel.bl_isEnlargeDisplayMode ) {
				// 拡大表示モード時
				rebuildGamenEnlarge( 2 );
			} else {
				rebuildGamen( 2 );
			}
		}
	}
}
//================================================================================================
// 概要		:管理者モード共通 : する／しないキー（押された）.
// 戻り値	:.
//===============================================================================================
Bool PAdmincom::procButtonOnDoDont( uchar option )
{
	Bool bl_Accept = True;
	switch( option ) {
		case BTNID_KEY_MODESELECTDO:	procAdminModeSelectDo();	break;	// するボタン．
		case BTNID_KEY_MODESELECTDONT:	procAdminModeSelectDont();	break;	// しないボタン.
		default:						bl_Accept = False;			break;
	}
	return bl_Accept;
}
//================================================================================================
// 概要		:管理者モード共通 : 表示部品のセットアップ.
// 戻り値	:.
//===============================================================================================
void PAdmincom::setupGamen( short option )
{
	PUtilityToMenuCom::setupGamen( option );
	// 管理者モードがＯＮのとき
	if( c_SystemControl.getAdminModeOn() == True ) {
		// 左エリア・ジョブリスト 管理者では表示しない(ユーティリティ・管理者共通インスタンス対応).
		Grp_LeftJobList.showEnable( False );
		Grp_LeftJobListType2.showEnable( False );							// 左エリア・ジョブリストグループ（Type2）.
	}
}
//================================================================================================
// 概要		:管理者モード共通 : HDDの準備が整っているかを判断.
// 戻り値	:Bool：True HDD準備OK.
// 備考		:
//===============================================================================================
Bool PAdmincom::isEnableHddAccess()
{
	APIC_HddInfo* pc_HddInfo = APIC_HddInfo::newInstance();
//	comment 2015/11/02 HCL_Ezhil [ZeusS_ZX0][制御構成変更_No.76-1] Adding error code.
//	if( NULL == pc_HddInfo ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return False; }
	if( NULL == pc_HddInfo ) { 
//Comment 2019/06/14 [HCLT_Monisha][EagleH][N Rank] Removed unwanted Detailed code.
//		LAPC_BFW_System *pc_BFW_System=LAPC_BFW_System::newInstance();
//		if (NULL !=pc_BFW_System){
//		    pc_BFW_System->registDetailCode(PNLD_DetailCode_Admin_0x1900660010113201);
//		}
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" ); 
		return False; 
	}
	const Bool b_Hdd = ( ( TYPE_HddStatusFormated == pc_HddInfo->getHddStatus() ) ? True : False );	// HDDの状態をチェック.
	const Bool b_Doc = APIC_BoxRegistry::GetAccessStatus();		// Doc機能へアクセス可能状態を取得する.
	if( ( True == b_Hdd ) && ( True == b_Doc ) ) {
		return	True;
	} else {
		return	False;
	}
}
//================================================================================================
// 概要		:管理者モード共通 : セキュリティー強化設定変更時の画面情報設定
// 戻り値	:なし
//================================================================================================
PNLE_AdminSecurityDisp	PAdmincom::e_DispInfo = PNLE_AdminSecurityDispNON;
PBasicMessageProc*	PAdmincom::p_FromGamen = NULL;
void PAdmincom::setDispInfo( PNLE_AdminSecurityDisp e_DispInfobuf, PBasicMessageProc* p_FromGamenbuf )
{
	e_DispInfo = e_DispInfobuf;
	p_FromGamen = p_FromGamenbuf;
}
//================================================================================================
// 概要		:管理者モード共通 : セキュリティー強化設定変更時の画面情報取得
// 戻り値	:なし
//================================================================================================
PNLE_AdminSecurityDisp PAdmincom::getDispInfoNum()
{
	return( e_DispInfo );
}
//================================================================================================
// 概要		:管理者モード共通 : セキュリティー強化設定変更時の画面情報取得
// 戻り値	:なし
//================================================================================================
PBasicMessageProc* PAdmincom::getDispInfo()
{
	return( p_FromGamen );
}
PNLS_DispTransition PAdmincom::m_st_DispTransition = { NULL, NULL, 0 };
//================================================================================================.
// 概要		:遷移元画面、遷移先画面の情報を設定する .
// 引数		:PNLS_DispTransition 遷移元画面、遷移先画面の情報.
// 戻り値	:なし.
// 備考		:必要が無くなれば、必ず「clearDispTransition」で初期化すること.
//================================================================================================.
void PAdmincom::setDispTransition( PNLS_DispTransition* pst_DispTransitionbuf )
{
// 2015/07/08 SecV1-5 【管理者によるサービスモード制御技術】 Sky 新道.
	if( NULL == pst_DispTransitionbuf ){
//Comment 2019/06/14 [HCLT_Monisha][EagleH][N Rank] Removed unwanted Detailed code.
//		LOGF_debug_PutEventLog(LOGF_DEBUG_EVENT_KIND_ALL, 0, 3, "Assert Failed!"); 
// comment 2015/10/30 HCL_SunilNP [ZeusS_ZX0][制御構成変更_No.76-1] Adding error code.
		LAPC_BFW_System *pc_BFW_System = LAPC_BFW_System::newInstance();
		if ( NULL != pc_BFW_System ) {
			pc_BFW_System->registDetailCode( PNLD_DetailCode_Admin_0x1900030010201401 );
		}
		return;
	}

	m_st_DispTransition = *pst_DispTransitionbuf;
}
//================================================================================================.
// 概要		:遷移元画面、遷移先画面の情報を取得する .
// 引数		:PNLS_DispTransition 遷移元画面、遷移先画面の情報.
// 戻り値	:なし .
//================================================================================================.
void PAdmincom::getDispTransition( PNLS_DispTransition* pst_DispTransitionbuf )
{
// 2015/07/08 SecV1-5 【管理者によるサービスモード制御技術】 Sky 新道.
	if( NULL == pst_DispTransitionbuf ){
//Comment 2019/06/14 [HCLT_Monisha][EagleH][N Rank] Removed unwanted Detailed code.
//		LOGF_debug_PutEventLog(LOGF_DEBUG_EVENT_KIND_ALL, 0, 3, "Assert Failed!"); 
// comment 2015/10/30 HCL_SunilNP [ZeusS_ZX0][制御構成変更_No.76-1] Adding error code.
		LAPC_BFW_System *pc_BFW_System = LAPC_BFW_System::newInstance();
		if ( NULL != pc_BFW_System ) {
			pc_BFW_System->registDetailCode( PNLD_DetailCode_Admin_0x1900030010B01501 );
		}
		return;
	}

	*pst_DispTransitionbuf = m_st_DispTransition;
}
//================================================================================================.
// 概要		:遷移元画面、遷移先画面の情報をクリアする.
// 引数		:なし.
// 戻り値	:なし.
//================================================================================================.
void PAdmincom::clearDispTransition()
{
	memset( &m_st_DispTransition, 0 ,sizeof( m_st_DispTransition ) );
}
// 2015/07/08 SecV1-5 【管理者によるサービスモード制御技術】 Sky 新道.
PNLS_DispTransition PAdmincom::m_s_ReleaseKeyTransition = { NULL, NULL, 0 };
//================================================================================================.
// 概要		:遷移元画面、遷移先画面の情報を設定する .
// 引数		:PNLS_DispTransition 遷移元画面、遷移先画面の情報.
// 戻り値	:なし.
// 備考		:必要が無くなれば、必ず「clearReleaseKeyTransition」で初期化すること.
// 履歴		:2015/07/08 SecV1-5 【管理者によるサービスモード制御技術】 Sky 新道.
//================================================================================================.
void PAdmincom::setReleaseKeyTransition( PNLS_DispTransition* ps_ReleaseKeyTransitionbuf )
{
	if( NULL == ps_ReleaseKeyTransitionbuf ){
		LAPC_BFW_System *pc_BFW_System=LAPC_BFW_System::newInstance();
		if ( NULL != pc_BFW_System ) {
    		pc_BFW_System->registDetailCode(PNLD_DetailCode_Admin_0x1900700010B57101);
		}
//Comment 2019/06/14 [HCLT_Monisha][EagleH][N Rank] Removed unwanted Detailed code.
//		LOGF_debug_PutEventLog(LOGF_DEBUG_EVENT_KIND_ALL, 0, 3, "Assert Failed!"); 
		return;
	}

	m_s_ReleaseKeyTransition = *ps_ReleaseKeyTransitionbuf;
}
//================================================================================================.
// 概要		:遷移元画面、遷移先画面の情報を取得する .
// 引数		:PNLS_DispTransition 遷移元画面、遷移先画面の情報.
// 戻り値	:なし .
// 履歴		:2015/07/08 SecV1-5 【管理者によるサービスモード制御技術】 Sky 新道.
//================================================================================================.
void PAdmincom::getReleaseKeyTransition( PNLS_DispTransition* ps_ReleaseKeyTransitionbuf )
{
	if( NULL == ps_ReleaseKeyTransitionbuf ){
		LAPC_BFW_System *pc_BFW_System=LAPC_BFW_System::newInstance();
		if ( NULL != pc_BFW_System ) {
    		pc_BFW_System->registDetailCode(PNLD_DetailCode_Admin_0x1900700010B57201);
		}
//Comment 2019/06/14 [HCLT_Monisha][EagleH][N Rank] Removed unwanted Detailed code.
//		LOGF_debug_PutEventLog(LOGF_DEBUG_EVENT_KIND_ALL, 0, 3, "Assert Failed!"); 
		return;
	}

	*ps_ReleaseKeyTransitionbuf = m_s_ReleaseKeyTransition;
}
//================================================================================================.
// 概要		:遷移元画面、遷移先画面の情報をクリアする.
// 引数		:なし.
// 戻り値	:なし.
// 履歴		:2015/07/08 SecV1-5 【管理者によるサービスモード制御技術】 Sky 新道.
//================================================================================================.
void PAdmincom::clearReleaseKeyTransition()
{
	memset( &m_s_ReleaseKeyTransition, 0 ,sizeof( m_s_ReleaseKeyTransition ) );
}

APIS_ActiveDirectoryParams	PAdmincom::st_ActiveDirectorySetting;		// 外部サーバーActive Dirctory設定の一時領域.
//================================================================================================
//［機能］	：外部サーバー設定 ActiveDirectoryの一時領域情報設定.
//================================================================================================
void PAdmincom::setAuthServerActiveDirectoryParams( APIS_ActiveDirectoryParams* ps_setActiveDirectorySetting )
{
	memcpy( &st_ActiveDirectorySetting, ps_setActiveDirectorySetting, sizeof( st_ActiveDirectorySetting ) );
}
//================================================================================================
//［機能］	：外部サーバー設定 ActiveDirectoryの一時領域情報設定.
//================================================================================================
void PAdmincom::setAuthServerActiveDirectoryParams_Domain( char* sc_DomainName )
{
	memcpy( &st_ActiveDirectorySetting.sc_DefaultDomain, sc_DomainName, sizeof( st_ActiveDirectorySetting.sc_DefaultDomain ) );
}
//================================================================================================
//［機能］	：外部サーバー設定 ActiveDirectoryの一時領域情報設定.
//================================================================================================
void PAdmincom::setAuthServerActiveDirectoryParams_Timeout( ushort us_Timeout )
{
	st_ActiveDirectorySetting.us_Timeout = us_Timeout;
}
// comment 2016/11/7 SCD2-Sky工藤 [特ROM][SDR-63] 1604_強化機能.
//================================================================================================
//［機能］	：外部サーバー設定 ActiveDirectoryのサイト名一時領域情報設定.
//================================================================================================
void PAdmincom::setAuthServerActiveDirectoryParams_SiteName( char* sc_SiteName )
{
	memcpy( &st_ActiveDirectorySetting.sc_SiteName, sc_SiteName, sizeof( st_ActiveDirectorySetting.sc_SiteName ) );
}
//================================================================================================
//［機能］	：外部サーバー設定 ActiveDirectoryのドメインコントローラー一時領域情報設定.
//================================================================================================
void PAdmincom::setAuthServerActiveDirectoryParams_DomainCtrl( char* sc_DomainCtrl )
{
	memcpy( &st_ActiveDirectorySetting.sc_HostName, sc_DomainCtrl, sizeof( st_ActiveDirectorySetting.sc_HostName ) );
}
//================================================================================================
//［機能］	：外部サーバー設定 ActiveDirectoryの使用設定一時領域情報設定.
//================================================================================================
void PAdmincom::setAuthServerActiveDirectoryParams_UseSetting( Bool bl_UseSetting )
{
	st_ActiveDirectorySetting.b_SRVRecord = bl_UseSetting;
}
//================================================================================================
//［機能］	：外部サーバー設定 ActiveDirectoryの一時領域情報取得.
//================================================================================================
APIS_ActiveDirectoryParams* PAdmincom::getAuthServerActiveDirectoryParams()
{
	return &st_ActiveDirectorySetting;
}
PNLE_PasswordErrorType PAdminKeyBoardCom::e_PasswordError = PNLE_PasswordErrorType_Non;
char PAdminKeyBoardCom::sc_StrKeyboard[ PCGD_MaxKeybordInfoBuffer ];			// 文字列入力用.
char PAdminKeyBoardCom::sc_StrKeyboard2[ PCGD_MaxKeybordInfoBuffer ];			// テキストバッファへのポインタ.
char PAdminKeyBoardCom::sc_StrKeyboard2Check[ PCGD_MaxKeybordInfoBuffer ];		// 再入力のテキストバッファへのポインタ.
//概要    :管理者･キーボード共通 : コンストラクタ.
//備考    :.
PAdminKeyBoardCom::PAdminKeyBoardCom(
    PDisplayParts** partsTable,
    short partsNumber,
    Bool isNoWarningGamen )
	: PAdmincom( partsTable, partsNumber, isNoWarningGamen )
{
	pc_DispField = NULL;
	memset(	sc_StrKeyboard, 0, sizeof( sc_StrKeyboard ) );	// キーボード文字列.
	ss_MaxInputNum = 1;		// 最大入力文字数.
	b_Password = False;		// パスワード表示フラグ.
	b_ShowPass = False;		// 「*」を10個表示する/しない.
	pc_ActivatableField = NULL;

	memset( sc_StrKeyboard2, 0, sizeof( sc_StrKeyboard2 ) );				// テキストバッファへのポインタ.
	memset( sc_StrKeyboard2Check, 0, sizeof( sc_StrKeyboard2Check ) );	// 再入力のテキストバッファへのポインタ.
	b_PasswordSelect = False;		// パスワードキーのフラッグ.
}
//概要    :管理者･キーボード共通 : テンキー.
//備考    :.
void PAdminKeyBoardCom::proc10Key( uchar option )
{
	Kbd_UtilityInputKeybordConv.proc10Key( &option );
	if( 255 == option ) {
		setupGamen();
		return;
	}
	// パスワード入力＋「*」を10個表示状態の場合、
	if( b_Password == True && b_ShowPass == True ) {
		b_ShowPass = False;		// 「*」10個表示を止める.
		procClear( 0 );			// 表示をクリアする.
	}
	char	_sc_Num[2];


	// 入力されたのが0-9キーの時のみ処理を受け付ける.
	if( /*0 <= option &&*/ option <= 9 ) {
		// 数値を文字に変換.
		_sc_Num[0] = (char)( option + '0' );
		_sc_Num[1] = '\0';
		// 文字を追加.
		c_SlideTextMgr.insertStr( sc_StrKeyboard, ( char* )_sc_Num );
	}
	setupGamen();
}
//概要    :管理者･キーボード共通 : クリアー.
//備考    :.
void PAdminKeyBoardCom::procClear( uchar option )
{
	if( '\0' == Kbd_UtilityInputKeybordConv.c_PreString[0] ) {
		// パスワード入力＋「*」を10個表示状態の場合、
		if( b_Password == True && b_ShowPass == True ) {
			b_ShowPass = False;		// 「*」10個表示を止める.
		}
		if( 0 == strlen( sc_StrKeyboard ) ) {
			setBuzzerType( PCGE_BuzzerType_InputNg );	// 報知音設定
		}
		// 文字列の初期化.
		memset(	sc_StrKeyboard, 0, sizeof( sc_StrKeyboard ) );
		Kbd_UtilityInputKeybordConv.GetCursor( pc_ActivatableField );		// 入力文字表示領域のアドレスを取得する.
		// 入力中状態に変更.
		c_SlideTextMgr.setStatus( False );
		// スライド文字列管理クラスの初期化.
		c_SlideTextMgr.init_CLCD( sc_StrKeyboard, ( char )Kbd_UtilityInputKeybordConv.getKeyboardDispLenMax(), ss_MaxInputNum, pc_ActivatableField, getCharSetForPanel() );
		if( True == PLBC_FuncVersionCheck::isOK_Type1() ) {
			PLBC_WideKeyboard::getInstance()->initInputArea_Clear();	// スライド文字列管理クラスの初期化 （Wideキーボード用）.
		}
	} else {
		setBuzzerType( PCGE_BuzzerType_InputNg );	// 報知音設定
	}
	setupGamen();
}
//概要    :管理者･キーボード共通 : キーボードのローカル値クリアー.
//備考    :.
void PAdminKeyBoardCom::procClearKeyboardBuff()
{
	// パスワード入力＋「*」を10個表示状態の場合、
	if( b_Password == True && b_ShowPass == True ) {
		b_ShowPass = False;		// 「*」10個表示を止める.
	}
	// 文字列の初期化.
	memset(	sc_StrKeyboard, 0, sizeof( sc_StrKeyboard ) );
	Kbd_UtilityInputKeybordConv.GetCursor( pc_ActivatableField );		// 入力文字表示領域のアドレスを取得する.
	// スライド文字列管理クラスの初期化.
	c_SlideTextMgr.init_CLCD( sc_StrKeyboard, ( char )Kbd_UtilityInputKeybordConv.getKeyboardDispLenMax(), ss_MaxInputNum, pc_ActivatableField, getCharSetForPanel() );
	if( True == PLBC_FuncVersionCheck::isOK_Type1() ) {
		PLBC_WideKeyboard::getInstance()->initInputArea();	// スライド文字列管理クラスの初期化 （Wideキーボード用）.
	}
	setupGamen();
}
// 概要		:管理者･キーボード共通  : シャープキー
// 戻り値	:なし
void PAdminKeyBoardCom::procShrp( uchar option )
{
	Kbd_UtilityInputKeybordConv.proc10Key( &option );
	if( 255 == option ) {
		setupGamen();
		return;
	}
	// パスワード入力＋「*」を10個表示状態の場合、
	if( b_Password == True && b_ShowPass == True ) {
		b_ShowPass = False;		// 「*」10個表示を止める.
		procClear( 0 );			// 表示をクリアする.
	}
	// アルファベットの時は受け付けない.
	if( Kbd_UtilityInputKeybordConv.b_KeybordAlphabetMode == False ) {
		char	sc_Num[PCGD_NumberKeyCode];
		// 最大入力長まで行っているならば入力しない
		snprintf( sc_Num,sizeof(sc_Num),"#" );		// 入力された#の表示を行う
		c_SlideTextMgr.insertStr( sc_StrKeyboard, sc_Num );		// 文字を追加
	}
	setupGamen();
}
// 概要		:管理者･キーボード共通  : アスタリスクキー
// 戻り値	:なし
void PAdminKeyBoardCom::procAster( uchar option )
{
	Kbd_UtilityInputKeybordConv.proc10Key( &option );
	if( 255 == option ) {
		setupGamen();
		return;
	}
	// パスワード入力＋「*」を10個表示状態の場合、
	if( b_Password == True && b_ShowPass == True ) {
		b_ShowPass = False;		// 「*」10個表示を止める.
		procClear( 0 );			// 表示をクリアする.
	}
	// アルファベットの時は受け付けない.
	if( Kbd_UtilityInputKeybordConv.b_KeybordAlphabetMode == False ) {
		char	sc_Num[PCGD_NumberKeyCode];
		// 最大入力長まで行っているならば入力しない
		snprintf( sc_Num,sizeof(sc_Num),"*" );		// 入力された*の表示を行う
		c_SlideTextMgr.insertStr( sc_StrKeyboard, sc_Num );		// 文字を追加
	}
	setupGamen();
}
//概要    :管理者･キーボード共通 : キーボード.
//備考    :.
void PAdminKeyBoardCom::procKeyBoard( uchar option )
{
	// パスワード入力＋「*」を10個表示状態の場合、
	if( b_Password == True && b_ShowPass == True ) {
		b_ShowPass = False;		// 「*」10個表示を止める.
		procClear( 0 );			// 表示をクリアする.
	}
	// Sta [MTUIv1]反応音追加 キーボード固有ブザー追加.
	if( False == isUSBKeyBoardMode() ) {		// USBキーボードからの入力でなければ、入力音を鳴動.
		setBuzzerType( PCGE_BuzzerType_Motion2 );
	}
	Kbd_UtilityInputKeybordConv.procCursorPosition( ( char* )&c_SlideTextMgr, sc_StrKeyboard, option, False );	// カーソル位置処理
	setupGamen();
}
//概要    :管理者･キーボード共通 : ボタン（押された）.
//備考    :.
void PAdminKeyBoardCom::procButtonOn( uchar option )
{
	// 標準・拡大キーボード処理
	// 標準⇔拡大切換えキー押下処理
	if( 100 == option ) {
		// キーボード初期状態判定フラグをTrueにする.
		onlyPanel.b_InitKeybord = True;
		// 未確定文字列が存在する場合は無変換で確定とする.
		if( '\0' != Kbd_UtilityInputKeybordConv.c_PreString[0] ) {
			procButtonOn( 227 );
		}
		if( PLBD_Enl_KBD_Normal == Kbd_UtilityInputKeybordConv.setInitKeybordStatus2() )	{ nextGamen = this; }	// startGamenから描画するため、nextGamenに自画面を設定.
		else																				{ nextGamen = this; }	// startGamenから描画するため、nextGamenに自画面を設定.
		return;
	}
	// パスワード入力＋「*」を10個表示状態の場合、
	if( b_Password == True && b_ShowPass == True ) {
		b_ShowPass = False;		// 「*」10個表示を止める.
		procClear( 0 );			// 表示をクリアする.
	}
	// ユーティリティ専用キーボード共通ボタン（押された）.
	procButtonOnKeybordUtility( option );
	char	sc_keyCode[PLBD_KBD_BufMax];	// 確定文字列.
	if( True == Kbd_UtilityInputKeybordConv.setKeybordDecideStr( option, sc_keyCode ) )	{ c_SlideTextMgr.insertStr( sc_StrKeyboard, sc_keyCode, Kbd_UtilityInputKeybordConv.getConvCharSetNativeToUniCode() ); }	// 無変換または、変換確定の場合、確定文字列を挿入.
	setupGamen();		// 画面セットアップ.
}
//概要    :管理者･キーボード共通 : キーボードにデータをセット.
//備考    :.
void PAdminKeyBoardCom::setKeyboardData(
    char*	sc_InputData,		// キーボードの初期文字列.
    short	ss_InputMax,		// キーボードの最大入力文字数.
    char	sc_InputWidth )		// キーボードの幅(char単位).
{
	strncpy( sc_StrKeyboard, sc_InputData,sizeof( sc_StrKeyboard )-1 );

	ss_MaxInputNum	= ss_InputMax;
}
//概要    :管理者･キーボード共通 : キーボードの文字列をゲット.
//備考    :.
char* PAdminKeyBoardCom::getKeyboardData(
    char*	sc_OutputData )		// 出力文字列.
{
	strcpy( sc_OutputData, sc_StrKeyboard );
	return sc_OutputData;
}
//概要    :管理者･キーボード共通 : 画面開始処理.
//備考    :.
void PAdminKeyBoardCom::startGamen()
{
	PAdmincom::startGamen();		// 親クラス呼び出し.
	// パス入力選択クラス.
	c_SlideTextMgr.setSelectProc( b_Password );
	// スライド文字列管理クラスの初期化.
	Kbd_UtilityInputKeybordConv.GetCursor( pc_ActivatableField );		// 入力文字表示領域のアドレスを取得する.
	// 11開発内田:標準・拡大キーボード切り替え処理追加.
	c_SlideTextMgr.init_CLCD( sc_StrKeyboard, ( char )Kbd_UtilityInputKeybordConv.getKeyboardDispLenMax(), ss_MaxInputNum, pc_ActivatableField, getCharSetForPanel() );

// comment 2015/09/11 OPS1 Sky東島 [Poseidon2.0c][CQ518664].
//	if ( pc_DispField != 0 ) {
//		pc_DispField->startGUI(m_pc_displayParts);
//	}
//	if ( pc_ActivatableField != 0 ) {
//		pc_ActivatableField->startGUI(m_pc_displayParts);
//	}

// comment 2020/11/30 DWS1-Sky延原 【TAS000000003634】リモートパネル使用時にパスワード入力のキー操作がハイライト表示されてしまう.
	// リモートキーボードかつ拡大表示でない場合、ハイライト表示/キー沈み込み描画を抑制する.
	if ( True == isRemotePanelConnecting() ) {
		LIBG_EnterHighlightDisplayHideSection();	//キー押下時のハイライト表示抑止.
		setPressedActionToPartsTable( this->getPartsTable(), this->getPartsNumber(), False );		// キー沈み込み描画抑止.
		PLBC_WideKeyboard::getInstance()->setPressedAction( False );								// PLBC_WideKeyboardクラスに抑止を設定.
	} else {
		LIBG_LeaveHighlightDisplayHideSection();	//キー押下時のハイライト表示抑止解除.
		// 拡大表示の場合は、常にキー沈み込み描画抑止.
		if ( PLBD_Enl_KBD_Enlarge == Kbd_UtilityInputKeybordConv.getChangeEnlKeybord() ) {
			setPressedActionToPartsTable( this->getPartsTable(), this->getPartsNumber(), False );		// キー沈み込み描画抑止.
			PLBC_WideKeyboard::getInstance()->setPressedAction( False );								// PLBC_WideKeyboardクラスに抑止を設定.
		} else {
			setPressedActionToPartsTable( this->getPartsTable(), this->getPartsNumber(), True );		// キー沈み込み描画実施.
			PLBC_WideKeyboard::getInstance()->setPressedAction( True );									// PLBC_WideKeyboardクラスに描画実施を設定.
		}
	}
}
//概要    :管理者･キーボード共通 : 画面終了処理.
//備考    :.
void PAdminKeyBoardCom::endGamen()
{
// comment 2015/09/11 OPS1 Sky東島 [Poseidon2.0c][CQ518664].
//	if ( pc_ActivatableField != 0 ) {
//		pc_ActivatableField->endGUI();
//	}
//	if ( pc_DispField != 0 ) {
//		pc_DispField->endGUI();
//	}

// comment 2020/11/30 DWS1-Sky延原 【TAS000000003634】リモートパネル使用時にパスワード入力のキー操作がハイライト表示されてしまう.
	LIBG_LeaveHighlightDisplayHideSection();	//キー押下時のハイライト表示抑止終了.
	// 通常表示の場合は、キー沈み込み描画状態をパーツ定義時の状態に戻す.
	// 戻しておかないと拡大キーボードコンポーネントの制御に問題(キーが沈み込む)が出る.
	if ( PLBD_Enl_KBD_Enlarge != Kbd_UtilityInputKeybordConv.getChangeEnlKeybord() ) {
		setPressedActionToPartsTable( this->getPartsTable(), this->getPartsNumber(), False );		// キー沈み込み描画抑止.
	}
	PLBC_WideKeyboard::getInstance()->setPressedAction( True );									// PLBC_WideKeyboardクラスに描画実施を設定.

	PAdmincom::endGamen();		// 親クラス呼び出し
	endGamenKeybordUtility();	// ユーティリティ専用キーボード共通終了処理
	if( onlyPanel.b_InitKeybord == False ) {
		Kbd_UtilityInputKeybordConv = PLBC_ConvertKeybord( PLBD_KBT_ASCII_S, 192, 168 );	// 位置を標準値に戻す.
		Kbd_UtilityInputKeybordConv.setChangeEnlKeybord( PLBD_Enl_KBD_Initial );
	}
}
//概要    :管理者･キーボード共通 : 表示画面のセットアップ.
//備考    :.
void PAdminKeyBoardCom::setupGamen( short option )
{
	if( PLBD_Enl_KBD_Enlarge != Kbd_UtilityInputKeybordConv.getChangeEnlKeybord() ) {
		// 画面固有の処理.
		Key_EnlAdminKeybordCancel.showEnable( False );
	}
	// キーボード拡大状態のsetup処理.
	else {
		Btx_AdminNetworkSetComInputKeybord.showEnable( False );
	}
	PAdmincom::setupGamen( option );
	// ユーティリティ専用キーボード共通セットアップ.
//comment 2015/02/09 OPS1-MSE天木 [ZeusS][CQ509367] IPv4,IPv6入力時には共通のキーボード処理を行わない.
	if ( ( TYPE_USBKeyIPvGamen_v4 != Kbd_UtilityInputKeybordConv.isIPv4v6Gamen() ) && ( TYPE_USBKeyIPvGamen_v6 != Kbd_UtilityInputKeybordConv.isIPv4v6Gamen() ) ) {
	setupGamenKeybordUtility();
	}
	
	// キーボードの種類を取得する.
	KeybordType ec_KeybordType = Kbd_UtilityInputKeybordConv.getKeybordType();
	// 日本語入力の場合は未確定文字列表示用にスライド処理を行う
	c_SlideTextMgr.PreStringSlideControl( sc_StrKeyboard, ec_KeybordType );
	c_SlideTextMgr.drawStr( sc_StrKeyboard );
	if( PLBD_Enl_KBD_Enlarge == Kbd_UtilityInputKeybordConv.getChangeEnlKeybord() ) {
		PLBC_ConvertKeybord* pc_ConvKeyboard = PnlComGetConvertKeybord();
		if( ( NULL != pc_ConvKeyboard )
		    && ( True == pc_ConvKeyboard->isRemoteKeyboardMode() )
		    && ( True == pc_ConvKeyboard->isIMERunning() ) ) {
			// RemoteKeyboardでのIME起動中はクリアキーを非表示にする為何もしない.
		} else {
			Key_SoftEnl_Clear.showEnableShow( True );
		}
	}
	
// comment 2015/08/17 OPS1-Sky浜野 [MinervaSBK][CQ515414] キーボード濁点、半濁点操作改善対応.
	// 濁点・半濁点・小文字キーの描画更新.
	Kbd_UtilityInputKeybordConv.setupGamenReplaceKeybord( &c_SlideTextMgr );
}
//概要    :管理者･キーボード共通 : 現在の入力状態がブランクか？を判断する.
//戻り値  :Bool ブランク状態 ： False.
//備考    :.
Bool PAdminKeyBoardCom::NowInputState()
{
	if( sc_StrKeyboard[0] != '\0' ) {	// 入力された文字列がブランクではないので「True」を返す.
		return	True;
	} else {	return	False;	}	// 入力された文字列がブランクなので「False」を返す.
}
//概要    :管理者･キーボード共通 : パスワード再入力の画面開始処理.
//備考    :.
void PAdminKeyBoardCom::startGamenPasswordReInput()
{
	// パスワード表示フラグ.
	b_Password = True;

	// パスワード再入力のフラグ.
	Kbd_UtilityInputKeybordConv.b_PasswordReInput = True;

	// 初期表示以外は文字列を初期化しない.
	if( PLBD_Enl_KBD_Initial == Kbd_UtilityInputKeybordConv.getChangeEnlKeybord() )
	{

		strncpy( sc_StrKeyboard2 , "", sizeof( sc_StrKeyboard2 ) );
		strncpy( sc_StrKeyboard2Check , "", sizeof( sc_StrKeyboard2Check ) );

		// パスワード再入力の初期化(共通関数前に実行する必要があります).
		b_PasswordSelect = True;

		e_PasswordError = PNLE_PasswordErrorType_Non;
	}

	// キーボード初期状態の場合、キーボード切換え状態を初期状態とする.
	if( True == Kbd_UtilityInputKeybordConv.setInitKeybordStatus1() ) {
		Kbd_UtilityInputKeybordConv.setKeyboardDispLenMax( PCGD_KeyboardDispLenMax, PCGD_KeyboardDispLenMax );	// No.312048 2008/11/25 12柳川 文字の表示領域を52→72に変更.
	}


	// パスワード再入力の設定.
	if( True == b_PasswordSelect )
	{
		// パスワードキーを選択時.
		strncpy( sc_StrKeyboard2, sc_StrKeyboard, sizeof( sc_StrKeyboard2 ) - 1 );
		sc_StrKeyboard2[ sizeof( sc_StrKeyboard2 ) - 1 ] = '\0';
		strncpy( sc_StrKeyboard, sc_StrKeyboard2Check, sizeof( sc_StrKeyboard ) - 1 );
		sc_StrKeyboard[ sizeof( sc_StrKeyboard ) - 1 ] = '\0';

		if( PLBD_Enl_KBD_Enlarge == Kbd_UtilityInputKeybordConv.getChangeEnlKeybord() )
		{
			pc_ActivatableField = &Btx_EnlPasswordReInputCheck;
		}
		else
		{
			pc_ActivatableField = &Btx_PasswordReInputCheck;
		}

		Kbd_UtilityInputKeybordConv.GetCursor( pc_ActivatableField );
		c_SlideTextMgr.init_CLCD( sc_StrKeyboard, ( char )Kbd_UtilityInputKeybordConv.getKeyboardDispLenMax(), ss_MaxInputNum, pc_ActivatableField, getCharSetForPanel() );

		// 日本語入力の場合は未確定文字列表示用にスライド処理を行う.
		c_SlideTextMgr.PreStringSlideControl( sc_StrKeyboard, Kbd_UtilityInputKeybordConv.getKeybordType() );
		c_SlideTextMgr.drawStr( sc_StrKeyboard, True );	//【 CQ362628 : ZeusMLK : Nagaoka : 2012.07.13 : 入力文字と表示文字の差分によるOffset時の不具合対応 】.

// comment  2018/02/16 HCL_Neeraj [IT5_4.2] Klockwork Changes ID: 357067 [SV.STRBO.BOUND_COPY.UNTERM].
//		strncpy( sc_StrKeyboard, sc_StrKeyboard2, sizeof( sc_StrKeyboard2 ) );
		strncpy( sc_StrKeyboard, sc_StrKeyboard2, sizeof( sc_StrKeyboard ) - 1 );
		if( PLBD_Enl_KBD_Enlarge == Kbd_UtilityInputKeybordConv.getChangeEnlKeybord() )
		{
			pc_ActivatableField = &Btx_EnlPasswordReInput;
		}
		else
		{
			pc_ActivatableField = &Btx_PasswordReInput;
		}
		Kbd_UtilityInputKeybordConv.GetCursor( pc_ActivatableField );
	}
	else
	{
		// パスワードの確認入力キーを選択時.
		strncpy( sc_StrKeyboard2Check, sc_StrKeyboard, sizeof( sc_StrKeyboard )-1 );
		strncpy( sc_StrKeyboard, sc_StrKeyboard2, sizeof( sc_StrKeyboard ) - 1 );
		sc_StrKeyboard[ sizeof( sc_StrKeyboard ) - 1] = '\0';
		if( PLBD_Enl_KBD_Enlarge == Kbd_UtilityInputKeybordConv.getChangeEnlKeybord() )
		{
			pc_ActivatableField = &Btx_EnlPasswordReInput;
		}
		else
		{
			pc_ActivatableField = &Btx_PasswordReInput;
		}

		Kbd_UtilityInputKeybordConv.GetCursor( pc_ActivatableField );
		c_SlideTextMgr.init_CLCD( sc_StrKeyboard, ( char )Kbd_UtilityInputKeybordConv.getKeyboardDispLenMax(), ss_MaxInputNum, pc_ActivatableField, getCharSetForPanel() );

		// 日本語入力の場合は未確定文字列表示用にスライド処理を行う.
		c_SlideTextMgr.PreStringSlideControl( sc_StrKeyboard, Kbd_UtilityInputKeybordConv.getKeybordType() );
		c_SlideTextMgr.drawStr( sc_StrKeyboard, True );	//【 CQ362628 : ZeusMLK : Nagaoka : 2012.07.13 : 入力文字と表示文字の差分によるOffset時の不具合対応 】.
//comment 2018/01/17 【HCL-NEERAJ】【IT5_4.2】Klockwork Changes ID:357067 [SV.STRBO.BOUND_COPY.UNTERM].
//		strncpy( sc_StrKeyboard, sc_StrKeyboard2Check, sizeof( sc_StrKeyboard2Check ) );
		strncpy( sc_StrKeyboard, sc_StrKeyboard2Check, sizeof( sc_StrKeyboard ) - 1 );
		if( PLBD_Enl_KBD_Enlarge == Kbd_UtilityInputKeybordConv.getChangeEnlKeybord() )
		{
			pc_ActivatableField = &Btx_EnlPasswordReInputCheck;
		}
		else
		{
			pc_ActivatableField = &Btx_PasswordReInputCheck;
		}
		Kbd_UtilityInputKeybordConv.GetCursor( pc_ActivatableField );
	}

	// キーボードの動作のため、下の関数をコール.
	Kbd_UtilityInputKeybordConv.setDispFieldEnl( &Btx_EnlChangeKeybord );

	// キーボードパーツの位置を変更.
	if( PLBD_Enl_KBD_Enlarge == Kbd_UtilityInputKeybordConv.getChangeEnlKeybord() )
	{
		Key_PasswordReInput.changePosition( 8, 0 );
		Key_PasswordReInputCheck.changePosition( 8, 38 );
	}

	Kbd_UtilityInputKeybordConv.setTxGy( 192, 185 );
}
//概要    :管理者･キーボード共通 : パスワード再入力の画面セットアップ.
//備考    :.
void PAdminKeyBoardCom::setupGamenPasswordReInput( short option )
{
	if( PLBD_Enl_KBD_Enlarge != Kbd_UtilityInputKeybordConv.getChangeEnlKeybord() )
	{

		// 拡大キーボードの入力エリアを非表示.
		Btx_EnlPasswordReInput.showEnable( False );
		Btx_EnlPasswordReInputCheck.showEnable( False );
	}
	else
	{
		// 標準キーボードの入力エリアを非表示.
		Btx_PasswordReInput.showEnable( False );
		Btx_PasswordReInputCheck.showEnable( False );
	}

	// パスワード入力キーの選択処理.
	if( True == b_PasswordSelect )
	{
		Rbt_PasswordReInput.select( 0 );
		if( PLBD_Enl_KBD_Enlarge != Kbd_ScanFaxKeybordConv.getChangeEnlKeybord() )
		{
			Btx_PasswordReInput.setCursorOption( UC_CURSOR_OPTION_ON );
			Btx_PasswordReInputCheck.setCursorOption( UC_CURSOR_OPTION_OFF );
		}
		else
		{
			Btx_EnlPasswordReInput.setCursorOption( UC_CURSOR_OPTION_ON );
			Btx_EnlPasswordReInputCheck.setCursorOption( UC_CURSOR_OPTION_OFF );
		}
	}
	else
	{
		Rbt_PasswordReInput.select( 1 );
		if( PLBD_Enl_KBD_Enlarge != Kbd_ScanFaxKeybordConv.getChangeEnlKeybord() )
		{
			Btx_PasswordReInput.setCursorOption( UC_CURSOR_OPTION_OFF );
			Btx_PasswordReInputCheck.setCursorOption( UC_CURSOR_OPTION_ON );
		}
		else
		{
			Btx_EnlPasswordReInput.setCursorOption( UC_CURSOR_OPTION_OFF );
			Btx_EnlPasswordReInputCheck.setCursorOption( UC_CURSOR_OPTION_ON );
		}
	}

	// OKキーのグレーアウト処理.
	short ss_CurPos = 0;			// カーソル位置.
	short ss_CurPosCheck = 0;		// カーソル位置.

	if( PLBD_Enl_KBD_Enlarge == Kbd_UtilityInputKeybordConv.getChangeEnlKeybord() )
	{
		// 拡大キーボードの時.
		Btx_EnlPasswordReInput.getCursorPosition( ss_CurPos );
		Btx_EnlPasswordReInputCheck.getCursorPosition( ss_CurPosCheck );
	}
	else
	{
		Btx_PasswordReInput.getCursorPosition( ss_CurPos );
		Btx_PasswordReInputCheck.getCursorPosition( ss_CurPosCheck );
	}

	if( ( 0 < ss_CurPos && 0 == ss_CurPosCheck )
	    || ( 0 == ss_CurPos && 0 < ss_CurPosCheck ) )
	{
		if( PLBD_Enl_KBD_Enlarge == Kbd_UtilityInputKeybordConv.getChangeEnlKeybord() )
		{
			// 拡大キーボードの時.
			Key_EnlKeyboardOK.activate( False );	// OKキー(拡大).
		}
		else
		{
			// 標準キーボードの時.
			Key_UtilityOk.activate( False );		// OKキー(標準).
		}
	}
	else
	{
		if( PLBD_Enl_KBD_Enlarge == Kbd_UtilityInputKeybordConv.getChangeEnlKeybord() )
		{
			// 拡大キーボードの時.
			Key_EnlKeyboardOK.activate( True );		// OKキー(拡大).
		}
		else
		{
			// 標準キーボードの時.
			Key_UtilityOk.activate( True );			// OKキー(標準).
		}
	}
}
//概要    :管理者･キーボード共通 : パスワード再入力のボタン（押された）.
//備考    :.
void PAdminKeyBoardCom::procButtonOnPasswordReInput( uchar option )
{
	switch( option )
	{
		case BTNID_KEY_PASSWORD:   // パスワードキー.
// comment 2015/07/23 OPS1-Sky浜野 [MinervaSBK][制御構成変更No.75] パスワード入力文字表示対応.
			// パスワード文字表示を解除する.
			PCGC_SelectTextMgr::fixPassInput();
			setupGamen();
		
			b_PasswordSelect = True;

			if( PLBD_Enl_KBD_Enlarge == Kbd_UtilityInputKeybordConv.getChangeEnlKeybord() )
			{
				// 拡大キーボードのパスワード入力エリア.
				pc_ActivatableField = &Btx_EnlPasswordReInput;
			}
			else
			{
				// 標準キーボードのパスワード入力エリア.
				pc_ActivatableField = &Btx_PasswordReInput;
			}

			Kbd_UtilityInputKeybordConv.GetCursor( pc_ActivatableField );

			// 入力表示の処理.
			strncpy( sc_StrKeyboard2Check, sc_StrKeyboard, sizeof( sc_StrKeyboard2Check ) - 1 );
			sc_StrKeyboard2Check[ sizeof( sc_StrKeyboard2Check ) - 1 ] = '\0';
			strncpy( sc_StrKeyboard, sc_StrKeyboard2, sizeof( sc_StrKeyboard ) - 1 );
			sc_StrKeyboard[ sizeof( sc_StrKeyboard ) - 1 ] = '\0';

			// スライド文字列管理クラスの初期化.
			c_SlideTextMgr.init_CLCD( sc_StrKeyboard, ( char )Kbd_UtilityInputKeybordConv.getKeyboardDispLenMax(), ss_MaxInputNum, pc_ActivatableField, getCharSetForPanel() );
			break;
		case BTNID_KEY_PASSWORDCHANGE:  // パスワードの確認入力キー.
// comment 2015/07/23 OPS1-Sky浜野 [MinervaSBK][制御構成変更No.75] パスワード入力文字表示対応.
			// パスワード文字表示を解除する.
			PCGC_SelectTextMgr::fixPassInput();
			setupGamen();

			b_PasswordSelect = False;
			if( PLBD_Enl_KBD_Enlarge == Kbd_UtilityInputKeybordConv.getChangeEnlKeybord() )
			{
				// 拡大キーボードのパスワードの確認入力エリア.
				pc_ActivatableField = &Btx_EnlPasswordReInputCheck;
			}
			else
			{
				// 標準キーボードのパスワードの確認入力エリア.
				pc_ActivatableField = &Btx_PasswordReInputCheck;
			}

			Kbd_UtilityInputKeybordConv.GetCursor( pc_ActivatableField );

			// 入力表示の処理.
			strncpy( sc_StrKeyboard2, sc_StrKeyboard, sizeof( sc_StrKeyboard2 )-1 );
			strncpy( sc_StrKeyboard, sc_StrKeyboard2Check, sizeof( sc_StrKeyboard ) - 1 );
			sc_StrKeyboard[sizeof( sc_StrKeyboard ) - 1 ] = '\0';

			// スライド文字列管理クラスの初期化.
			c_SlideTextMgr.init_CLCD( sc_StrKeyboard, ( char )Kbd_UtilityInputKeybordConv.getKeyboardDispLenMax(), ss_MaxInputNum, pc_ActivatableField, getCharSetForPanel() );
			break;
		default:
			PAdminKeyBoardCom::procButtonOn( option );
			break;
	}
	if( ( BTNID_KEY_PASSWORD       == option )									// パスワードキー.
	    ||( BTNID_KEY_PASSWORDCHANGE == option ) ) {								// パスワードの確認入力キー.
		if( True == Kbd_UtilityInputKeybordConv.isRemoteKeyboardMode() ) {		// RemoteKeyboardモードでは無い.
			Kbd_UtilityInputKeybordConv.reqIMEEnd();
		}
	}

}
//概要    :管理者･キーボード共通 : パスワード再入力のクリアー.
//備考    :.
void PAdminKeyBoardCom::procClearPasswordReInput( uchar option )
{
	if( '\0' == Kbd_UtilityInputKeybordConv.c_PreString[0] )
	{
		// パスワード入力＋「*」を10個表示状態の場合.
		if( b_Password == True && b_ShowPass == True )
		{
			// 「*」10個表示を止める.
			b_ShowPass = False;
		}
		if( 0 == strlen( sc_StrKeyboard ) && 0 == strlen( sc_StrKeyboard2 ) && 0 == strlen( sc_StrKeyboard2Check ) && True == Kbd_UtilityInputKeybordConv.b_PasswordReInput )
		{
			// 報知音設定.
			setBuzzerType( PCGE_BuzzerType_InputNg );
		}

		// 文字列の初期化.
		memset(	sc_StrKeyboard, 0, sizeof( sc_StrKeyboard ) );

		// エラーの時、入力をクリア.
		memset( sc_StrKeyboard2 ,  0x00, sizeof( sc_StrKeyboard2 ) );
		memset( sc_StrKeyboard2Check ,  0x00, sizeof( sc_StrKeyboard2Check ) );

		// 入力中状態に変更.
		c_SlideTextMgr.setStatus( False );

		// 入力表示を更新.
		if( PLBD_Enl_KBD_Enlarge == Kbd_UtilityInputKeybordConv.getChangeEnlKeybord() )
		{
			pc_ActivatableField = &Btx_EnlPasswordReInputCheck;
		}
		else
		{
			pc_ActivatableField = &Btx_PasswordReInputCheck;
		}

		Kbd_UtilityInputKeybordConv.GetCursor( pc_ActivatableField );
		c_SlideTextMgr.init_CLCD( sc_StrKeyboard, ( char )Kbd_UtilityInputKeybordConv.getKeyboardDispLenMax(), ss_MaxInputNum, pc_ActivatableField, getCharSetForPanel() );

		if( PLBD_Enl_KBD_Enlarge == Kbd_UtilityInputKeybordConv.getChangeEnlKeybord() )
		{
			pc_ActivatableField = &Btx_EnlPasswordReInput;
		}
		else
		{
			pc_ActivatableField = &Btx_PasswordReInput;
		}

		Kbd_UtilityInputKeybordConv.GetCursor( pc_ActivatableField );
		c_SlideTextMgr.init_CLCD( sc_StrKeyboard, ( char )Kbd_UtilityInputKeybordConv.getKeyboardDispLenMax(), ss_MaxInputNum, pc_ActivatableField, getCharSetForPanel() );

		// 選択状態を初期化.
		b_PasswordSelect = True;
	}
	else
	{
		// 報知音設定.
		setBuzzerType( PCGE_BuzzerType_InputNg );
	}
}
//概要    :管理者･キーボード共通 : パスワード再入力の画面終了処理.
//備考    :.
void PAdminKeyBoardCom::endGamenPasswordReInput()
{
	// パスワード再入力のフラグ.
	Kbd_UtilityInputKeybordConv.b_PasswordReInput = False;

	if( onlyPanel.b_InitKeybord == False )
	{
		// キーボード切り替え以外のときは初期値に戻す
		Kbd_UtilityInputKeybordConv.setKeyboardDispLenMax( PCGD_KeyboardDispLenMax,PCGD_EnlKeyboardDispLenMax );
	}

	// 位置を先に戻す.
	Key_PasswordReInput.changePosition( 16, 104 );
	Key_PasswordReInputCheck.changePosition( 16, 143 );
	Kbd_UtilityInputKeybordConv.setTxGy( 192, 168 );

	PAdminKeyBoardCom::endGamen();
}

// 概要		:管理者･キーボード共通 :HOME(メニュー)画面への遷移処理
// 戻り値	:なし.
// 備考		:pcgutility.cpp PUtilityMenu::procGoToPage()のcase BTNID_KEY_UTILITYTOP_HOME以下の処理と
//			 同様の処理。但し本関数の if(False == PNLC_WebUIControl::getInstance()->isEnableWebUI() ) は固有の処理。
// 履歴		:2018/9/27 SCD2-Sky二口 [Eagle][AR1062689]管理者パスワード入力画面のHOMEキー押下対応.
void PAdminKeyBoardCom::goToHomeMenu()
{
	PUtilitycom::setUtilityGamenExit();						// ユーティリティを抜ける時の処理 .
	// 指定アプリを再起動する必要があれば、ここで行う.
	SpecifiedAppStartingTrans( PNLD_SPECIFIEDAPP_DO_REBUILD );
	
	bl_UnSetTurnGamen = True;				// 現在の画面を戻り画面としてセットしない.
	setTurnGamenThis( NULL );				// 戻りをなしとする.
	if (gp_CostCenterResetDummy.bl_CostCenter == True) {
		ScanOapExtSet.req_OpenAPI( PNLE_OAP_REQ_COSTCENTER );
		if(&gp_PublicPopupDummyBase != PBasicMessageProc::nowGamen){
			gp_PublicPopupDummyBase.setupOpenPopup( &gp_ScanOapAuthConnectingExt_act );
			gp_PublicPopupDummyBase.setupNextGamen( NULL );
			gp_PublicPopupDummyBase.setMode( PNLE_PublicPopupDummyModeAct );
			nextGamen = &gp_PublicPopupDummyBase;
			OpenPopupIndi();
		}
		return;
	}
	onlyPanel.setTopmenuMode( True );		// 優先機能をトップメニューへ
	
	if( True == onlyPanel.b_nowBoxMode ){
		onlyPanel.b_nowBoxMode = False;
	}
	
	onlyPanel.setBrowserMode(False);
	
	if( True == onlyPanel.bl_isEnlargeDisplayMode ) {
		// 拡大表示モード時
		rebuildGamenEnlarge( 0 );
	} else {
		if( False == PNLC_WebUIControl::getInstance()->isEnableWebUI() ) {
			// WebUI使用不可の場合
			PNLC_WebUIControl::getInstance()->setUIMode( PNLE_UIModeLegacyUI );		// クラシックスタイルのUIモードをセット
		}	
		rebuildGamen( 0 );
	}
	
	return;
}

// comment 2020/11/30 DWS1-Sky延原 【TAS000000003634】リモートパネル使用時にパスワード入力のキー操作がハイライト表示されてしまう.
// 概要		:管理者･キーボード共通 :リモートパネル接続中 を判定する.
// 戻り値	:リモートパネル接続中状態ならば、呼出し元へTrueを返す.
// 備考		:
// 履歴		:2020/11/30 DWS1-Sky延原 【TAS000000003634】リモートパネル使用時にパスワード入力のキー操作がハイライト表示されてしまう.
Bool PAdminKeyBoardCom::isRemotePanelConnecting( void )
{
	Bool bl_RemotePanelConnecting = False;

	// リモートパネル接続状態を確認する.
	APIC_RemotePanelSetting* pc_RemotePanelSetting = APIC_RemotePanelSetting::newInstance();
	if( NULL != pc_RemotePanelSetting && TYPE_RemoteStatusNone != pc_RemotePanelSetting->getRemoteStatus() ) {
		bl_RemotePanelConnecting = True;
	}

	return bl_RemotePanelConnecting;
}

// 概要		:パーツテーブルから、キーパーツ を検索しキー押下時の沈み込み描画を制御する.
// 戻り値	:なし.
// 備考		:検索中に、PGroupParts が見つかった場合は、再起呼び出しを行う
// 履歴		:2020/11/30 DWS1-Sky延原 【TAS000000003634】リモートパネル使用時にパスワード入力のキー操作がハイライト表示されてしまう.
void PAdminKeyBoardCom::setPressedActionToPartsTable( PDisplayParts** partsTable, short partsNumber, Bool bl_PressedAction )
{
	if ( ( NULL == partsTable ) 
	  || ( 0 >= partsNumber ) ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); 
		return;
	}
	
	PGroupParts*		pc_PGroupParts = NULL;
	
	// partsTable内を検索し、キーパーツ/グループパーツを見つける.
	// キーパーツの場合は沈み込み描画制御を行い、グループパーツの場合は、.
	// グループパーツ内を検索する.
	for ( int n_PartIndex = 0; n_PartIndex < partsNumber; n_PartIndex++ ) {
		setPressedAction( partsTable[n_PartIndex], bl_PressedAction );

		pc_PGroupParts = dynamic_cast< PGroupParts* >( partsTable[n_PartIndex] );
		if ( NULL != pc_PGroupParts ) {
			setPressedActionToGroupParts( pc_PGroupParts, bl_PressedAction );
		}
	}

	return;
}

// 概要		:グループパーツから、キーパーツ を検索しキー押下時の沈み込み描画を制御する.
// 戻り値	:なし.
// 備考		:検索中に、PGroupParts が見つかった場合は、再起呼び出しを行う
// 履歴		:2020/11/30 DWS1-Sky延原 【TAS000000003634】リモートパネル使用時にパスワード入力のキー操作がハイライト表示されてしまう.
void PAdminKeyBoardCom::setPressedActionToGroupParts( PGroupParts* pc_PGroupParts, Bool bl_PressedAction )
{
	if ( NULL == pc_PGroupParts ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); 
		return;
	}
	
	uchar uc_GroupPartsCount = pc_PGroupParts->getPartsNumber();

	PDisplayParts*		pc_PDisplayParts = NULL;
	PGroupParts*		pc_PGroupPartsNext = NULL;
	
	// pc_PGroupParts内を検索し、キーパーツ/グループパーツを見つける.
	// キーパーツの場合は、沈み込み描画制御を行い、グループパーツの場合は、再起呼び出しを行う.
	for ( uchar uc_PartsIndex = 0; uc_PartsIndex < uc_GroupPartsCount; uc_PartsIndex++ ) {
		pc_PDisplayParts = pc_PGroupParts->getPartsAddress( uc_PartsIndex );
		
		setPressedAction( pc_PDisplayParts, bl_PressedAction );

		pc_PGroupPartsNext = dynamic_cast< PGroupParts* >( pc_PDisplayParts );
		if ( NULL != pc_PGroupPartsNext ) {
			setPressedActionToGroupParts( pc_PGroupPartsNext, bl_PressedAction );
		}
	}

	return;
}

// 概要		:ディスプレイパーツがキーパーツならキー押下時の沈み込み描画を制御する.
// 戻り値	:なし.
// 備考		:なし.
// 履歴		:2020/11/30 DWS1-Sky延原 【TAS000000003634】リモートパネル使用時にパスワード入力のキー操作がハイライト表示されてしまう.
void PAdminKeyBoardCom::setPressedAction( PDisplayParts* pc_PDisplayParts, Bool bl_PressedAction )
{
	if ( NULL == pc_PDisplayParts ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); 
		return;
	}
	
	PButton*			pc_PButtonParts = NULL;
	PButtonAdd*			pc_PButtonAddParts = NULL;
	PButtonObject*		pc_PButtonObjectParts = NULL;
	PButtonObjectAdd*	pc_PButtonObjectAddParts = NULL;
	
	// キーパーツの場合は、沈み込み描画制御を行う.
	pc_PButtonParts = dynamic_cast< PButton* >( pc_PDisplayParts );
	if ( NULL != pc_PButtonParts ) {
		pc_PButtonParts->setShowPush( bl_PressedAction );
	}

	pc_PButtonAddParts = dynamic_cast< PButtonAdd* >( pc_PDisplayParts );
	if ( NULL != pc_PButtonAddParts ) {
		pc_PButtonAddParts->setShowPush( bl_PressedAction );
	}

	pc_PButtonObjectParts = dynamic_cast< PButtonObject* >( pc_PDisplayParts );
	if ( NULL != pc_PButtonObjectParts ) {
		pc_PButtonObjectParts->setShowPush( bl_PressedAction );
	}

	pc_PButtonObjectAddParts = dynamic_cast< PButtonObjectAdd* >( pc_PDisplayParts );
	if ( NULL != pc_PButtonObjectAddParts ) {
		pc_PButtonObjectAddParts->setShowPush( bl_PressedAction );
	}

	return;
}

// 概要     :管理者･キーボード共通 : リモートパネル接続通知イベント.
// 戻り値	:なし.
// 備考     :.
// 履歴		:2020/11/30 DWS1-Sky延原 【TAS000000003634】リモートパネル使用時にパスワード入力のキー操作がハイライト表示されてしまう.
void PAdminKeyBoardCom::procRemotePanelStart( uchar option )
{
	// 拡大表示の場合は、ハイライト表示抑止のみ.
	LIBG_EnterHighlightDisplayHideSection();	//キー押下時のハイライト表示抑止.
	if( PLBD_Enl_KBD_Enlarge != Kbd_UtilityInputKeybordConv.getChangeEnlKeybord() ) {
		setPressedActionToPartsTable( this->getPartsTable(), this->getPartsNumber(), False );		// キー沈み込み描画抑止.
	}

	PAdmincom::procRemotePanelStart( option );
}

// 概要     :管理者･キーボード共通 : リモートパネル切断通知イベント.
// 戻り値	:なし.
// 備考     :.
// 履歴		:2020/11/30 DWS1-Sky延原 【TAS000000003634】リモートパネル使用時にパスワード入力のキー操作がハイライト表示されてしまう.
void PAdminKeyBoardCom::procRemotePanelStop( uchar option )
{
	LIBG_LeaveHighlightDisplayHideSection();	//キー押下時のハイライト表示抑止解除.
	// 拡大表示の場合は、ハイライト表示抑止のみ.
	if( PLBD_Enl_KBD_Enlarge != Kbd_UtilityInputKeybordConv.getChangeEnlKeybord() ) {
		setPressedActionToPartsTable( this->getPartsTable(), this->getPartsNumber(), True );		// キー沈み込み描画実施.
	}

	PAdmincom::procRemotePanelStop( option );
}

// 概要     :管理者･キーボード共通 : 非アクティブ状態通知イベント.
// 戻り値	:なし.
// 備考     :.
// 履歴		:2020/11/30 DWS1-Sky延原 【TAS000000003634】リモートパネル使用時にパスワード入力のキー操作がハイライト表示されてしまう.
void PAdminKeyBoardCom::procInactivate()
{
	LIBG_LeaveHighlightDisplayHideSection();	//キー押下時のハイライト表示抑止解除.

	// 親クラスを呼び出し.
	PAdmincom::procInactivate();
}

//================================================================================================
// 概要		:管理者モード・管理者パスワード入力 : ＯＫ.
// 戻り値	:なし.
// 備考		:.
// 			:2015/07/01 SecV1-5 【管理者によるサービスモード制御技術】 Sky 新道.
//================================================================================================
void PAdminPassword::procOk( uchar option )
{
	char _sc_AdminPassword[DEF_AdminPasswordBufSize];
	memset( _sc_AdminPassword, 0, sizeof( _sc_AdminPassword ) );
	getKeyboardData( _sc_AdminPassword );
	TYPE_UserResult _pe_UserResult = TYPE_UserResultOk;
	PanelLog( PNLD_LogKind_Auth, "login() P1[%s] P2[%s] P3[%d] P4[%d] P5[%d]", TYPD_UserName_Admin, _sc_AdminPassword, TYPD_UserFunctionMask_None, _pe_UserResult, JLGE_InspectLogIfOperationPanel, 0 );
	APIC_User* pc_UserAdmin = NULL;

	if( ( TYPE_AdminModeAppPSWC == ppc_SystemStatus->getAdminModeOnApp() )
	    || ( TYPE_AdminLoginModePSWC == ppc_SystemStatus->getAdminLoginMode() ) ) {
	// 2015/07/01 SecV1-5 【管理者によるサービスモード制御技術】 Sky 新道.
		//m_b_isPSWCMessage = True;
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}
// comment 2019/05/15 Chikuan [EagleH][OP_BTS-1792] V1遷移パフォーマンス改善.
	if( PNLE_WebUIScreenChangeStatusIdle != PNLC_WebUIControl_v1::getInstance()->getWebUIScreenChangeStatus() ){
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}

// Comment 2015/10/28 HCLT-Vivek [ZeusS_ZX0][制御構成変更_No 76][WP 2547][WP 2597]  Counter value to be updated when administator mode login is performed with Panel Link using bizhub RA.
	Bool e_BizhubConnectStatus  =  False;                                              //Bizhub connection status


	// 使用可能ストレージ通知内容の取得.
	APIC_TabletIF* pc_TabletIF = APIC_TabletIF::newInstance();
	if( NULL == pc_TabletIF ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return;
	} 
	if( NULL != pc_TabletIF ) {
		 e_BizhubConnectStatus  = pc_TabletIF->getConnectState();   // BizhubRAとの接続状態が「接続」か.

	}

	if( ( True == m_bl_AdminPassGamenToDisplay ) &&
// Comment 2019/11/13 【HCLT-Raja】 【IT6】 【RQ-845】【初期使用時の管理者パスワード変更】
//	    ( PNLE_AdminPasswordMoveGamen_SimpleSecuritySetting != m_e_NextGamen ) ) {
// comment 2022/07/25 Sky 前田  [IT6-89(機種マージ)]  USBドングルを使用したMFPロック機能対応.
//		( PNLE_AdminPasswordMoveGamen_SimpleSecuritySetting != m_e_NextGamen && PNLE_AdminPasswordMoveGamen_AdministratorPasswordSettings != m_e_NextGamen ) ) {
		( PNLE_AdminPasswordMoveGamen_SimpleSecuritySetting != m_e_NextGamen && PNLE_AdminPasswordMoveGamen_UsbDongleRegist != m_e_NextGamen && PNLE_AdminPasswordMoveGamen_AdministratorPasswordSettings != m_e_NextGamen ) ) {
		//ベース画面から遷移してきた場合、管理者パスワードの比較のみとする.
		pc_UserAdmin = APIC_User::newInstance( TYPD_IndexUserAdmin );
		if( NULL == pc_UserAdmin ) {
//	comment 2015/11/02 HCL_Ezhil [ZeusS_ZX0][制御構成変更_No.76-1] Adding error code.
//Comment 2019/06/14 [HCLT_Monisha][EagleH][N Rank] Removed unwanted Detailed code.
//			LAPC_BFW_System *pc_BFW_System=LAPC_BFW_System::newInstance();
//			if (NULL !=pc_BFW_System){
//			    pc_BFW_System->registDetailCode(PNLD_DetailCode_Admin_0x1900030011313301);
//			}
			LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
			return;
		}
		char sc_AdminNowPassword[DEF_AdminPasswordBufSize];			// 現在の管理者パスワードを格納.
		memset( sc_AdminNowPassword,0,sizeof( sc_AdminNowPassword ) );
		pc_UserAdmin->getUserPassword( sc_AdminNowPassword );		// 現在設定されている管理者パスワードを取得.
		if( 0 == strncmp( _sc_AdminPassword, sc_AdminNowPassword, DEF_AdminPasswordBufSize-1 ) ) {
// comment 2016/11/21 SCD2-Sky辻 [特ROM][FQ-OPSD-1600383-OFP252][CQ534112] 設定メニュー選択時に管理者パスワードをかけたマシンで、本体の前ドアを開閉するとパスワード無しでログインできる. 
			m_bl_AuthSuccessed = True;
			//パスワード一致.
// Comment 2015/10/26 HCLT-Vivek [ZeusS_ZX0][制御構成変更_No 76][WP 2547][WP 2597]  Counter value to be updated.
// comment 2016/01/22 【HCLT-Binesh】【ZeusS_ZX0】【CQ524150】 Removing the condition check restricting counter increment for panel link scan alone because the counter increment is applicable for panel link also.
//			if( ( True == e_BizhubConnectStatus ) && ( JBLE_OAP_SCANTODEV == e_OapFuncType ) ) {
			if( True == e_BizhubConnectStatus ) {
       			PnlComIncreaseOperateCnt(TYPE_OperateCnt_BizhubRA_AdminLogin);		  //Counter value updation for bizhub.
			}else {
				PnlComIncreaseOperateCnt( TYPE_OperateCnt_Auth_Admin );		         //Counter value updation admin authentication.
			}
			nextGamen = &gp_UtilityMenu;
		} else {
			//パスワード不一致.
// comment 2016/11/21 SCD2-Sky辻 [特ROM][FQ-OPSD-1600383-OFP252][CQ534112] 設定メニュー選択時に管理者パスワードをかけたマシンで、本体の前ドアを開閉するとパスワード無しでログインできる. 
			PAdminPasswordError::setNextGamen( m_e_NextGamen );
			nextGamen = &gp_AdminPasswordError;
		}
// comment 2015/05/20 OPS1 Sky稲井 [特ROM対応][SDR-46] ProjectChameleon対応（管理者排他制御）.
	//DipSW223 bit0=ON、かつ　getAdminModeOn()=Trueの場合は、IWS側から管理者ログインしていると判断して、パスワード不一致警告を出す.
	}else if( (True == APIC_SoftDipSw::newInstance()->getSoftDipSwBitForPanelSpecNo( DEFSOFTWARESW223 , 0 ))
		   &&( True == c_SystemControl.getAdminModeOn())){
		PAdminPasswordError::setNextGamen( m_e_NextGamen );
		nextGamen = &gp_AdminPasswordError;
	} else {
// comment 2020/04/17 Sky東島 [SparrowBK][OP_BTS-22568].
		if( NULL != comPanelJob ){
			if( EP_PNS_Setting != onlyPanel.ep_PanelStatus ) {
				// ユーザーログアウト時と同様の次ジョブ予約処理を行う.
				// FAXジョブ中に管理者ログインはできないが同様の処理を行う.
				TYPE_JobType e_JobType = comPanelJob->getJobSpec()->pc_Common->c_JobType.getJobType();
				if( TYPE_JobTypeFaxSendQuick == e_JobType || TYPE_JobTypeFaxSendMemory == e_JobType
				    || TYPE_JobTypeFaxSendTimer == e_JobType || TYPE_JobTypeFaxReceive == e_JobType
				    || TYPE_JobTypeIfaxReceive == e_JobType || TYPE_JobTypeFaxReceiveTimer == e_JobType ) {
					// FAXジョブであれば.
					c_JobControl.controlFaxJobChange( True );
				} else {
					c_JobControl.createNewJob( comPanelJob );
				}
				onlyPanel.ep_PanelStatus = judgePanelStatus( comPanelJob->getJobStatus()->getInputStatus(), comPanelJob->getJobStatus()->getOutputStatus(), comPanelJob->getJobStatus()->getFaxJobStatus(), comPanelJob->getJobStatus()->getRealTimePreviewStatus() );
			}
		}
// Comment 2017/11/22【HCLT-Tanu】【IT5 4.2】【RQ686】API Correction.		
//		pc_UserAdmin = APIC_User::login( TYPD_UserName_Admin, _sc_AdminPassword, TYPD_UserFunctionMask_None,&_pe_UserResult, JLGE_InspectLogIfOperationPanel );
		pc_UserAdmin = APIC_User::login( TYPD_UserName_Admin, _sc_AdminPassword, TYPD_UserFunctionMask_None,&_pe_UserResult, JLGE_InspectLogIfOperationPanel,False , False,PNLC_NetworkLan::getInstance()->getAdminNWLine( ) );
		// 管理者認証成功.
		if( _pe_UserResult == TYPE_UserResultOk ) {	// 管理者パスワードが一致
// comment 2016/11/21 SCD2-Sky辻 [特ROM][FQ-OPSD-1600383-OFP252][CQ534112] 設定メニュー選択時に管理者パスワードをかけたマシンで、本体の前ドアを開閉するとパスワード無しでログインできる. 
			m_bl_AuthSuccessed = True;
			TYPS_ExtDetailAuthentication* ps_ExtDetailAuth = NULL;
			ps_ExtDetailAuth = PCGC_AuthOapAPIAccess::getAuthenticationAPI(); // 認証アプリ詳細登録情報取得
			if( ps_ExtDetailAuth ) {
// 	Comment 2017/09/28 [HCL-Harish]【IT5 4.2】【RQ-686】【WP793】Multiple Network support.
//				if( ( TYPE_UserAuthenticationIntermeddiateServer == PnlComGetUserAuthentication()->getUserAuthentication() 		// 中間認証.
//				      || TYPE_UserAuthenticationDeviceAndMidAuth    == PnlComGetUserAuthentication()->getUserAuthentication() ) ) {
				if( ( TYPE_UserAuthenticationIntermeddiateServer == PnlComGetUserAuthentication()->getUserAuthentication( PNLC_NetworkLan::getInstance() ->getAdminNWLine( ) ) 		// 中間認証.
				      || TYPE_UserAuthenticationDeviceAndMidAuth    == PnlComGetUserAuthentication()->getUserAuthentication( PNLC_NetworkLan::getInstance() ->getAdminNWLine( ) ) ) ) {
					//{
					//}
// comment 2017/04/06 SCD2-2G 蒲原 [IT5_4.0][UV-01399] ログアウト方法変更.
//					PCGC_AuthOapAPIAccess::getExtSettingAPI()->setLogoutMethod( TYPE_ExtLogoutMethodPanel,FALSE );
					onlyPanel.setExtLogoutMethod( TYPE_ExtLogoutMethodPanel );
				}
			}
			// 管理者モードに入るときの処理.
			bl_BumonLoginAdminMove = ( PnlComGetTrack() == NULL ) ? False : True;	// 部門ログイン状態で管理者モードに遷移したフラグ
// comment 2023/09/14 Sky東島 [IT6_2.3][OP_BTS-44767].
			// 共通処理でジョブを初期化させないようフラグを設定する.
			onlyPanel.setDontRunInitJobMode( True );
// comment 2015/04/24 OPS1 Sky東島 [Helios][CQ511946].
//			PnlComInitUserAccountInfo( True );		// ユーザ認証、部門管理のlogin情報、ジョブの情報を全て初期化する.
			PnlComInitUserAccountInfo( True , False );		// ユーザ認証、部門管理のlogin情報、ジョブの情報を全て初期化する.
// comment 2020/01/10 Sky東島 [EagleBK][OP_BTS-12168].
			if( TYPE_NetworkSpecific_NONE == m_e_NetworkSpecificBackup ){
				// ネットワークが選択されていない場合はNW1を設定する.
				PNLC_NetworkLan::getInstance()->setLine( TYPE_NetworkSpecific_NW1 );
			} else {
				PNLC_NetworkLan::getInstance()->setLine( m_e_NetworkSpecificBackup );
			}
			// ユーザー切替に伴い、指定アプリの警告を解除する（解除しないとendGamenで、setAdminGamenExit()がコールされる）.
			c_ExtPanel.SpecifiedInitialWarning();
			PnlComSetUser( pc_UserAdmin );			// 管理者ユーザを設定する.
// Comment 2018/01/18 HCLT_Prem [IT5_4.2][AR1034113] Setting Network Type as NW1 when opening Admin screen.
//comment 2018/11/27 [HCL-Sam Sundar] [IT6_Eagle] [AR1068587] - Commenting the below line to not set NW1 as default in password screen.
//			PNLC_NetworkLan::getInstance()->setLine( TYPE_NetworkSpecific_NW1 );
// comment 2023/09/14 Sky東島 [IT6_2.3][OP_BTS-44767].
			// ジョブ初期化不要設定フラグを解除する.
			onlyPanel.setDontRunInitJobMode( False );
			PnlComInitJobMode();					// 認証情報リセット処理.
			c_SystemControl.setAdminModeOn( True );	// 管理者モード中にする.
// Comment 2018/08/16 HCLT_Raja [IT6_Eagle][AR1042590] Set setAdminLoginMode as panel - successfull admin login.
			APIC_SystemStatus::newInstance()->setAdminLoginMode( TYPE_AdminLoginModePanel );
			gp_AdminSecurityPasswordEntry.uc_ErrorCounter = 0;	// 管理者のlocalﾊﾟｽﾜｰﾄﾞｴﾗｰｶｳﾝﾀをｸﾘｱする.
			// 音声ｶﾞｲﾄﾞ中の場合.
			if( TYPE_VoiceGuide_OFF != getVoiceCursorMode() ) {					// 音声ガイド状態をTYPE_VoiceGuide_OFFにセットする.
				PMGC_PanelManager::newInstance()->reqVoiceGuideEnd( False );	// Flashタスクへ音声ガイド終了通知送信.
			}
// comment 2014/12/22 OPS1-Sky Kawanishi [CQ507643] NFC/BLE認証でスリープ復帰+異常発生時、おかしなタイミングでログインしてしまう不具合対応.
			// スリープ復帰⇒NFC/BLE認証フラグの初期化.
			PNLC_ActionDataLib::newInstance()->setAdminNfcBleAuthNotifiedWhileSleep( False );
			// [UV-01159]_ZeusS：ログイン時の権限モードを追加.
			// UV-01159対応により、本画面を省略するため、遷移先を決める処理は、関数化.
			// PUtilitycom::decideAdminLoginNextGamen()へ移動しました.
// Comment 2015/10/26 HCLT-Vivek [ZeusS_ZX0][制御構成変更_No 76][WP 2547] [WP 2597]Counter value to be updated.
// comment 2016/01/22 【HCLT-Binesh】【ZeusS_ZX0】【CQ524150】 Removing the condition check restricting counter increment for panel link scan alone because the counter increment is applicable for panel link also.
//			if( ( True == e_BizhubConnectStatus ) && ( JBLE_OAP_SCANTODEV == e_OapFuncType )  ) {
			if( True == e_BizhubConnectStatus ) {
       			PnlComIncreaseOperateCnt(TYPE_OperateCnt_BizhubRA_AdminLogin);							//Counter value updation for bizhub.
			}else{
				PnlComIncreaseOperateCnt(TYPE_OperateCnt_Auth_Admin);									   //Counter value updation for Admin Authentication.
			}
// comment 2016/03/15 【HCLT-Senthil】【ZeusS_ZX0】【CQ526057】 Stops the authentication polling when entering the admin mode.
			PnlComReqAuthPollingTerminate();	// When entering the administrator mode, It stops the authentication polling.
// comment 2018/08/10 SCD2 T.Nishiba [Eagle][RQ-698-41]新設定メニューのメニュー項目対応.
// comment 2019/05/15 Chikuan [EagleH][OP_BTS-1792] V1遷移パフォーマンス改善.
//			BRWC_Browser::PanelToTask( BRWD_PToT_MES_DeleteCheckReq );	// PAdminSetMenu::setupGamen()にある処理をこちらにも追加した.
			decideAdminLoginNextGamen();

// Comment 2020/06/20【HCLT-Selvaraj】【IT6_2.0MR3】【制御構成変更_No.34】Set Digital post flag when Switch ON the MFP,waking up from power save and sleep.
			if( PnlComGetDigitPostON() ) {
				onlyPanel.setDigitalPostCheck( False );
		   	}
		} else {
			// 管理者認証失敗.
			b_isPenalty = False;
			if( _pe_UserResult == TYPE_UserResultPenalty ) {
				// ペナルティによる認証失敗
				b_isPenalty = True;
			}
			PAdminPasswordError::setNextGamen( m_e_NextGamen );
			nextGamen = &gp_AdminPasswordError;
		}
	}
}
// 任意のタイミングでログ取得の画面遷移は、PUtilitycom::decideAdminLoginNextGamen()からコール.
void PAdminPassword::moveAgreeToLog()
{
	if( False == PnlComGetSystemSetting()->getSecurityLevelUpOn()												// セキュリティー強化モードOFF.
	    && False == PnlComGetSoftDipSw()->getSoftDipSwBitForPanelSpecNo( PNLD_LOGSYSTEMSETTING, 7 )				// DipSW = No.155 bit7）がON（0x00).
	    && TYPE_GetLogConsentShowScreenIncomplete == PnlComGetSystemSetting()->getGetLogConsentShowScreen() ) {	// 情報取得確認画面で設定を行っていない.
		if( NULL != PnlComGetTotalCnt() ) {
			// コピートータルカウントの取得.
			uint32_t	ui32_TotalCnt = 0;
// Comment 2017/10/11 [HCLT-Rajesh] [IT5_4.2] [RQ-686] [WP 403] Multiple Network Support.
			ui32_TotalCnt = PnlComGetTotalCnt()->get( TYPE_TotalCntCopyTotalBlack )
			              +PnlComGetTotalCnt()->get( TYPE_TotalCntCopyTotalFullColor )
			              +PnlComGetTotalCnt()->get( TYPE_TotalCntCopyTotalMonoColor )
			              +PnlComGetTotalCnt()->get( TYPE_TotalCntCopyTotal2Color )
			              +PnlComGetTotalCnt()->get( TYPE_TotalCntCopyLargeBlack )
			              +PnlComGetTotalCnt()->get( TYPE_TotalCntCopyLargeFullColor )
			              +PnlComGetTotalCnt()->get( TYPE_TotalCntCopyLargeMonoColor )
			              +PnlComGetTotalCnt()->get( TYPE_TotalCntCopyLarge2Color )
			              +PnlComGetTotalCnt()->get( TYPE_TotalCntCopyTotalBlack, TYPE_NetworkSpecific_NW2 ) 
						  +PnlComGetTotalCnt()->get( TYPE_TotalCntCopyTotalFullColor, TYPE_NetworkSpecific_NW2 )
			              +PnlComGetTotalCnt()->get( TYPE_TotalCntCopyTotalMonoColor, TYPE_NetworkSpecific_NW2 )
			              +PnlComGetTotalCnt()->get( TYPE_TotalCntCopyTotal2Color, TYPE_NetworkSpecific_NW2 )
			              +PnlComGetTotalCnt()->get( TYPE_TotalCntCopyLargeBlack, TYPE_NetworkSpecific_NW2 )
			              +PnlComGetTotalCnt()->get( TYPE_TotalCntCopyLargeFullColor, TYPE_NetworkSpecific_NW2 )
			              +PnlComGetTotalCnt()->get( TYPE_TotalCntCopyLargeMonoColor, TYPE_NetworkSpecific_NW2 )
			              +PnlComGetTotalCnt()->get( TYPE_TotalCntCopyLarge2Color, TYPE_NetworkSpecific_NW2 );

			//トータルカウントが100枚以上の場合に情報取得確認画面を表示する.
			if( 100 <= ui32_TotalCnt ) {
				// 遷移元、遷移先画面の設定.
				PNLS_DispTransition st_Disp;
				memset( &st_Disp, 0, sizeof( st_Disp ) );
				if( PNLE_AdminPasswordMoveGamen_SimpleSecuritySetting == gp_AdminPassword.getNextGamen() ) {
// comment 2022/08/30 Sky西場 [IT6_2.3][OP_BTS-37620][スリム化].
					ExpPanelLog( PNLD_ExpLogKind_AlwaysLogOut, "[DeleteUnnecessaryProcessing] [%s]", __PRETTY_FUNCTION__, 0, 0, 0, 0, 0 );
//					st_Disp.p_SouceDisp = &gp_AdminSecuritySet;
				} else {
					st_Disp.p_SouceDisp = &gp_UtilityMenu;
				}
				st_Disp.p_NextDisp = nextGamen;							// 遷移先画面を設定する.
				PAdmincom::setDispTransition( &st_Disp );

				nextGamen = &gp_AdminRequestAgreeToUserGetLog;			// D090001000 情報取得確認.
			}
		}
	}
}
//================================================================================================
// 概要		:管理者モード・管理者パスワード入力 : 戻る
// 戻り値	:なし
//================================================================================================
void PAdminPassword::procCancel( uchar option )
{
// comment 2019/05/15 Chikuan [EagleH][OP_BTS-1792] V1遷移パフォーマンス改善.
	if( PNLE_WebUIScreenChangeStatusIdle != PNLC_WebUIControl_v1::getInstance()->getWebUIScreenChangeStatus() ){
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}

// comment 2022/07/25 Sky 前田  [IT6-89(機種マージ)]  USBドングルを使用したMFPロック機能対応.
	if (True == PnlComGetSoftDipSw()->getSoftDipSwBitForPanelSpecNo( 244, 3 )
		&& True == onlyPanel.isWarningOn(comPanelJob, PNL_ERROR_InsertTheUSBKey)
	) {
		// Dongle認証_ユーザー情報取得エラー表示.
		setBuzzerType( PCGE_BuzzerType_Error );
		PNLC_PopupListControl::newInstance()->addPopup( &gp_InsertTheUSBKey_warn, this, NULL, 400, 240, 0, 0 );
		return;
	}

	m_e_NextGamen = PNLE_AdminPasswordMoveGamen_AdminSetting;	// 遷移先画面指定.
	if( True == m_bl_AdminPassGamenToDisplay ) {
// comment 2016/01/28 OPS1-Sky浜野 [ZeusS_ZX0][CQ524401] Myパネル対応.
		// 右詰め表示言語の時はメッセージのロードしなおす.
		TYPE_Language e_CheckLanguage = PnlComGetLanguage( False );
// comment 2018/12/14 Sky毛穴 [EagleHigh][RQ-258] タイ語とベトナム語の対応.
//		if ( True == isRightDispLanguage( e_CheckLanguage ) ) {
		if (   True == isRightDispLanguage( e_CheckLanguage )
			|| TYPE_LanguageThai 	== e_CheckLanguage			// タイ語.
			|| TYPE_LanguageVietnam == e_CheckLanguage			// ベトナム語.
// comment 2022/11/9 Sky奥田 [RQ-938] バルト三国対応.
			|| TYPE_LanguageEstonian	== e_CheckLanguage		// エストニア語.
			|| TYPE_LanguageLatvian		== e_CheckLanguage		// ラトビア語.
			|| TYPE_LanguageLithuanian	== e_CheckLanguage		// リトアニア語.
// comment 2023/2/6 Sky小熊 [IT6_2.3][RQ-970]Welsh as standard language for IT.
			|| TYPE_LanguageWelsh		== e_CheckLanguage		// ウェールズ語.
		) {
			loadSelectLangType_IOP( e_CheckLanguage );
		}
		
		setTurnGamenThis( NULL );				// 戻りをなしとする.
		if( True == onlyPanel.bl_isEnlargeDisplayMode ) {
			// 拡大表示モード時.
			rebuildGamenEnlarge( option );
		} else {
			rebuildGamen( option );
		}
	} else {
		nextGamen = &gp_UtilityMenu;
	}
}
//================================================================================================
// 概要		:管理者モード・管理者パスワード入力 : 画面開始処理
// 戻り値	:なし
//================================================================================================
void PAdminPassword::startGamen()
{
// comment 2016/11/21 SCD2-Sky辻 [特ROM][FQ-OPSD-1600383-OFP252][CQ534112] 設定メニュー選択時に管理者パスワードをかけたマシンで、本体の前ドアを開閉するとパスワード無しでログインできる. 
	m_bl_AuthSuccessed = False;
	// パスワード表示フラグ.
	b_Password = True;
// comment 2020/01/10 Sky東島 [EagleBK][OP_BTS-12168].
	m_e_NetworkSpecificBackup = PNLC_NetworkLan::getInstance()->getLine();
	// 表示エリア.
	pc_DispField = &Tfdc_AdminNetworkSetComInputKeybord;
	pc_ActivatableField = &Btx_AdminNetworkSetComInputKeybord;
	//キーボードクラスへカーソルのアドレスを渡す
	Kbd_UtilityInputKeybordConv.GetCursor( pc_ActivatableField );
	Kbd_UtilityInputKeybordConv.setInitKeybordStatus1();	// キーボード初期状態の場合、キーボード切換え状態を初期状態とする.
	// 拡大表示の場合、入力文字表示領域を変更.
	if( True == Kbd_UtilityInputKeybordConv.setDispFieldEnl( &Btx_EnlChangeKeybord ) ) { pc_ActivatableField = &Btx_EnlChangeKeybord; }
	// キーボードにデータをセット.
	char _sc_AdminPassword[DEF_AdminPasswordBufSize];
	memset( _sc_AdminPassword, 0, sizeof( _sc_AdminPassword ) );
	// 初期表示以外は文字列を初期化しない
	if( PLBD_Enl_KBD_Initial == Kbd_UtilityInputKeybordConv.getChangeEnlKeybord() ) {
		if( False == PLBC_FuncPKICheck::isPKI() ) {
			setKeyboardData( _sc_AdminPassword, DEF_AdminPasswordSize, TYPD_AdminKeybordMax );
		} else {
			setKeyboardData( _sc_AdminPassword, DEF_AdminPasswordExSize, TYPD_AdminKeybordMax );
		}
	}
	PAdminKeyBoardCom::startGamen();
	// ユーティリティ専用キーボード共通開始処理.
	startGamenKeybordUtility9E();
	b_isPenalty = False;	// ｴﾗｰ状態を解除.

	// キーボードを５ヶ国語キーボードとする.
	Kbd_UtilityInputKeybordConv.setKeybord5Country( True );

	onlyPanel.b_InitKeybord = False;
	// 左エリア種類の有効設定（ジョブ表示（通常））startGamen()の最後でコールしてください.
	setLeftAreaEnableType( TYPD_LeftAreaBookmark | TYPD_LeftAreaSoft10Key );
	// タイトルの差し替え.
	// comment 2018/08/10 SCD2 T.Nishiba [Eagle][RQ-698-41]新設定メニューのメニュー項目対応.
	TEXT_ID puc_Title = s_TTLUtilityAdministratorSetting;
	if( PNLE_AdminPasswordMoveGamen_AdminMyPanel == m_e_NextGamen ) {
		puc_Title = s_TTLUtilityAdminMyPanel;
	} else if( PNLE_AdminPasswordMoveGamen_SimpleSecuritySetting  == m_e_NextGamen ) {
		puc_Title = s_TTLSimpleSecuritySettings;
// comment 2022/07/25 Sky 前田  [IT6-89(機種マージ)]  USBドングルを使用したMFPロック機能対応.
	} else if( PNLE_AdminPasswordMoveGamen_UsbDongleRegist  == m_e_NextGamen ) {
		puc_Title = s_pTTLUsbDongle_W608H16I;
	} else if( PNLE_AdminPasswordMoveGamen_SystemExpertModeMenu  == m_e_NextGamen ) {
		puc_Title = s_pTTLUtilityExpertAdjustment_W608H016I;
	} else if( PNLE_AdminPasswordMoveGamen_SecurityHDDManageSet  == m_e_NextGamen ) {
		puc_Title = s_pTTLUtilityStorageManagementSettings_W608H016I;
	} else if( PNLE_AdminPasswordMoveGamen_CardAttestationSetting == m_e_NextGamen ) {
		puc_Title = s_pTTLUtilityCardAttestationSetting_W608H016I;
	}
	Txt_UtilityAndAdminComTitle.changeText( puc_Title );
}
//================================================================================================
// 概要		:管理者モード・管理者パスワード入力 : 表示画面のセットアップ
// 戻り値	:なし
// 履歴		:2015/07/01 SecV1-5 【管理者によるサービスモード制御技術】 Sky 新道.
//================================================================================================
void PAdminPassword::setupGamen( short option )
{
	c_SlideTextMgr.setInsertCharLenCheck( TYPE_InsertCharLenStrUTF8 );	// UTF-8の文字数とバッファサイズで計算する.
	// キーボード標準状態のsetup処理.
	if( PLBD_Enl_KBD_Enlarge != Kbd_UtilityInputKeybordConv.getChangeEnlKeybord() ) {
		PAdminKeyBoardCom::setupGamen( option );
	} else {	// 標準のキーボード画面固有パーツを消す.
		// キーボード拡大状態のsetup処理.
		Grp_KeybordTotalPartsO2.showEnable( False );
		Btx_AdminNetworkSetComInputKeybord.showEnable( False );
		Txt_MessageAdminPassword.showEnable( False );
// 2014/09/05 【SecV1-5】CQ391654対応 日立 村上(幸).
		Key_AdminPasswordReleaseKey.showEnable( False );
		PAdminKeyBoardCom::setupGamen( option );
	}

//	if( True == m_b_isPSWCMessage ) {
//		if( ( TYPE_AdminModeAppPSWC == ppc_SystemStatus->getAdminModeOnApp() )
//		    || ( TYPE_AdminLoginModePSWC == ppc_SystemStatus->getAdminLoginMode() ) ) {
//			Key_UtilityOk.activate( False );	//OKキーグレーアウト.
//			Key_EnlKeyboardOK.activate( False );
//		}
//		else {
//			Key_UtilityOk.activate( True );
//			Key_EnlKeyboardOK.activate( True );
//			m_b_isPSWCMessage = False;	// PSWCからの管理者ログイン中メッセージ表示中にPSWCからの管理者ログアウトした場合、メッセージを元に戻す.
//		}
//	}
// 2014/09/05 【SecV1-5】CQ391654対応 日立 村上(幸).
	APIC_SystemSetting* pc_SystemSetting = APIC_SystemSetting::newInstance();
	if ( NULL == pc_SystemSetting ) {
		LAPC_BFW_System *pc_BFW_System=LAPC_BFW_System::newInstance();
		if ( NULL != pc_BFW_System ) {
    		pc_BFW_System->registDetailCode(PNLD_DetailCode_Admin_0x1900660011357301);
		}
//Comment 2019/06/14 [HCLT_Monisha][EagleH][N Rank] Removed unwanted Detailed code.
//		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, 3, "Assert Failed!" );
		return;
	}

//	if( True == m_b_isPSWCMessage ) {
//		Txt_MessageAdminPassword.changeText( s_MSGUtilityPSWCLogin );	// 「PSWCから管理者ログイン中です\nしばらく待ってから再度ログインしてください」.
	// 設定メニュー＞管理者設定以外から遷移した場合、「解除キー入力」キーを非表示.
	// 拡大表示されている状態で、「解除キー入力」キーを非表示.
	// サービスログイン許可設定が許可の時、「解除キー入力」キーを非表示.
	if( ( PNLE_AdminPasswordMoveGamen_AdminSetting != m_e_NextGamen )
	  ||( PLBD_Enl_KBD_Enlarge == Kbd_UtilityInputKeybordConv.getChangeEnlKeybord() )
	  ||( True == pc_SystemSetting->getEnableServiceLogin() ) ){
		Key_AdminPasswordReleaseKey.showEnable( False );
	} else {
		Key_AdminPasswordReleaseKey.showEnable( True );
	}
// 2015/07/01 SecV1-5 【管理者によるサービスモード制御技術】 Sky 新道.
	if ( ( TYPE_AdminModeAppPSWC == ppc_SystemStatus->getAdminModeOnApp() )
	  || ( TYPE_AdminLoginModePSWC == ppc_SystemStatus->getAdminLoginMode() ) ) {
		Key_UtilityOk.activate( False );	//OKキーグレーアウト.
		Key_AdminPasswordReleaseKey.activate( False );	//解除キー入力キーグレーアウト.
		Key_EnlKeyboardOK.activate( False );
		Txt_MessageAdminPassword.changeText( s_pMSGAdminPasswordPSWCLoginError_W624H048I );	// 「PSWCから管理者ログイン中です\nしばらく待ってから再度ログインまたは操作してください」.
	}
	else {
		Key_UtilityOk.activate( True );
		Key_AdminPasswordReleaseKey.activate( True );
		Key_EnlKeyboardOK.activate( True );
		if( b_isPenalty == False ) {
			Txt_MessageAdminPassword.changeText( s_MSGAdministratorPassword );	// 「管理者パスワードを入力してください」.
//			if( True == PLBC_FuncVersionCheck::isOK_Type1() ) {
//				Txt_MessageAdminPassword.changeText( s_MSGAdministratorPassword );	// 「管理者パスワードを入力してください」.
//			} else {
//				Txt_MessageAdminPassword.changeText( s_MSGMessageAdminPassword2 );	// 「管理者のパスワードを入力してください」.
//			}
		} else {	// 認証操作禁止機能がOFFの場合にﾊﾟｽﾜｰﾄﾞ不一致の場合は、ｴﾗｰﾒｯｾｰｼﾞを表示するだけ.
			Txt_MessageAdminPassword.changeText( s_MSGPasswordNotMatch );	// 「パスワ-ドが一致しません。入力し直してください」.
		}
	}
}
//================================================================================================
// 概要		:管理者モード・管理者パスワード入力 : 中止キー押下
// 戻り値	:なし
// 備考		:
//================================================================================================
void PAdminPassword::procReturn( uchar option )
{
// comment 2019/05/15 Chikuan [EagleH][OP_BTS-1792] V1遷移パフォーマンス改善.
	if( PNLE_WebUIScreenChangeStatusIdle != PNLC_WebUIControl_v1::getInstance()->getWebUIScreenChangeStatus() ){
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}

// comment 2022/07/25 Sky 前田  [IT6-89(機種マージ)]  USBドングルを使用したMFPロック機能対応.
	if (True == PnlComGetSoftDipSw()->getSoftDipSwBitForPanelSpecNo( DEFSOFTWARESW244, 3 )
		&& True == onlyPanel.isWarningOn(comPanelJob, PNL_ERROR_InsertTheUSBKey)
	) {
		// Dongle認証_ユーザー情報取得エラー表示.
		PNLC_PopupListControl::newInstance()->addPopup( &gp_InsertTheUSBKey_warn, this, NULL, 400, 240, 0, 0 );
		return;
	}

	m_e_NextGamen = PNLE_AdminPasswordMoveGamen_AdminSetting;	// 遷移先画面指定.
	if( True == m_bl_AdminPassGamenToDisplay ) {
		setTurnGamenThis( NULL );				// 戻りをなしとする.
		if( True == onlyPanel.bl_isEnlargeDisplayMode ) {
			// 拡大表示モード時.
			rebuildGamenEnlarge( option );
		} else {
			rebuildGamen( option );
		}
	} else {
		nextGamen = &gp_UtilityMenu;
	}
}
//================================================================================================
// 概要		:管理者モード・管理者パスワード入力 : 背景画面のセット
// 戻り値	:なし
// 備考		:
//================================================================================================
void PAdminPassword::setDefaultBackParts()
{
	if( ( PLBD_Enl_KBD_Enlarge == Kbd_UtilityInputKeybordConv.getChangeEnlKeybord() )
	    && ( True == onlyPanel.b_InitKeybord ) ) {	// 拡大用デフォルト背景.
		setDefaultBackPartsInfo( ( PDisplayParts** )tbl_AdminPassword2Default, ss_AdminPassword2Default );
	} else {	// 標準用デフォルト背景.
		setDefaultBackPartsInfo( ( PDisplayParts** )tbl_AdminPasswordDefault, ss_AdminPasswordDefault );
	}
}
void PAdminPassword::setNextGamen( PNLE_AdminPasswordNextGamen	e_NextGamen )	// 遷移先画面指定.
{
	m_e_NextGamen = e_NextGamen;
}

//================================================================================================.
// 概要		:管理者モード・管理者パスワード入力 : HOME(メニュー)画面への遷移処理
// 戻り値	:なし.
// 備考		:.
// 履歴		:2018/9/27 SCD2-Sky二口 [Eagle][AR1062689]管理者パスワード入力画面のHOMEキー押下対応.
//================================================================================================.
void PAdminPassword::procMenu( uchar option )
{
// comment 2019/05/15 Chikuan [EagleH][OP_BTS-1792] V1遷移パフォーマンス改善.
	if( PNLE_WebUIScreenChangeStatusIdle != PNLC_WebUIControl_v1::getInstance()->getWebUIScreenChangeStatus() ){
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}

// comment 2022/07/25 Sky 前田  [IT6-89(機種マージ)]  USBドングルを使用したMFPロック機能対応.
	if (True == PnlComGetSoftDipSw()->getSoftDipSwBitForPanelSpecNo( DEFSOFTWARESW244, 3 )
		&& True == onlyPanel.isWarningOn(comPanelJob, PNL_ERROR_InsertTheUSBKey)
	) {
		// Dongle認証_ユーザー情報取得エラー表示.
		setBuzzerType( PCGE_BuzzerType_Error );
		PNLC_PopupListControl::newInstance()->addPopup( &gp_InsertTheUSBKey_warn, this, NULL, 400, 240, 0, 0 );
		return;
	}

	goToHomeMenu();
}
//================================================================================================
// 概要		:管理者モード・管理者パスワード入力 : 遷移先画面取得.
// 戻り値	:PNLE_AdminPasswordNextGamen : .
// 備考		:
//================================================================================================
PNLE_AdminPasswordNextGamen PAdminPassword::getNextGamen()	// 遷移先画面取得.
{
	return	m_e_NextGamen;
}
//================================================================================================
// 概要		:管理者モード・管理者パスワード入力 : 終了処理.
// 戻り値	:なし.
// 備考		:
//================================================================================================
void PAdminPassword::endGamen()
{
	PAdminKeyBoardCom::endGamen();
	//nextGamenがPadminPasswordError,PAdminPassworクラスを継承してないかチェック.
	if( NULL == dynamic_cast< PAdminPasswordError*>( nextGamen )
	    && NULL == dynamic_cast< PAdminPassword*>( nextGamen ) ) {
//Comment 2015/12/17 HCL_Krithika [ZeusS_ZX0][制御構成変更_No.76-1] [Check-in issue] Not required case.
////	comment 2015/11/02 HCL_Ezhil [ZeusS_ZX0][制御構成変更_No.76-1] Adding error code.
//		LAPC_BFW_System *pc_BFW_System=LAPC_BFW_System::newInstance();
//		if (NULL !=pc_BFW_System){
//		    pc_BFW_System->registDetailCode(PNLD_DetailCode_Admin_0x1900670011313401);
//		}
// comment 2016/11/21 SCD2-Sky辻 [特ROM][FQ-OPSD-1600383-OFP252][CQ534112] 設定メニュー選択時に管理者パスワードをかけたマシンで、本体の前ドアを開閉するとパスワード無しでログインできる. 
		if ( ( False == m_bl_AuthSuccessed )
			&& ( True == m_bl_AdminPassGamenToDisplay 
// comment 2022/07/25 Sky 前田  [IT6-89(機種マージ)]  USBドングルを使用したMFPロック機能対応.
//				|| PNLE_AdminPasswordMoveGamen_SimpleSecuritySetting == m_e_NextGamen ) ) {
				|| PNLE_AdminPasswordMoveGamen_SimpleSecuritySetting == m_e_NextGamen || PNLE_AdminPasswordMoveGamen_UsbDongleRegist == m_e_NextGamen) ) {
			setTurnGamenThis( NULL );
		}
		//遷移先がPadminPasswordError,PAdminPassword以外であれば、フラグを落とす.
		m_bl_AdminPassGamenToDisplay = False;
// comment 2016/11/21 SCD2-Sky辻 [特ROM][FQ-OPSD-1600383-OFP252][CQ534112] 設定メニュー選択時に管理者パスワードをかけたマシンで、本体の前ドアを開閉するとパスワード無しでログインできる. 
	    m_e_NextGamen = PNLE_AdminPasswordMoveGamen_AdminSetting;
	}
}

// 2014/07/22 【管理者によるサービスモード制御技術】 日立 村上(幸).
//================================================================================================.
// 概要		:管理者モード・管理者パスワード入力 : 画面移動.
// 戻り値	:なし.
// 備考		:.
// 履歴		:2015/07/01 SecV1-5 【管理者によるサービスモード制御技術】 Sky 新道.
//================================================================================================.
void PAdminPassword::procGoToPage( uchar uc_option )
{
// comment 2019/05/15 Chikuan [EagleH][OP_BTS-1792] V1遷移パフォーマンス改善.
	if( PNLE_WebUIScreenChangeStatusIdle != PNLC_WebUIControl_v1::getInstance()->getWebUIScreenChangeStatus() ){
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}

	switch( uc_option )
	{
		// 「解除キー入力」キーを押下.
		case DEF_BTNID_KEY_ReleaseKeyInput:
// comment 2022/08/30 Sky西場 [IT6_2.3][OP_BTS-37620][スリム化].
			ExpPanelLog( PNLD_ExpLogKind_AlwaysLogOut, "[DeleteUnnecessaryProcessing] [%s]", __PRETTY_FUNCTION__, 0, 0, 0, 0, 0 );
//			if ( ( TYPE_AdminModeAppPSWC == ppc_SystemStatus->getAdminModeOnApp() )
//			  || ( TYPE_AdminLoginModePSWC == ppc_SystemStatus->getAdminLoginMode() ) ) {
//				setBuzzerType( PCGE_BuzzerType_InputNg );
//				return;
//			}
//			nextGamen = &gp_AdminPasswordReleaseKey;	// 管理者パスワード解除キー入力画面.
			break;
		default:
			break;
	}
}

//================================================================================================
// 概要		:管理者モード・管理者パスワード入力 : 画面遷移元判定フラグ設定.
// 戻り値	:なし
// 備考		: True:ベース画面から管理者パスワードに遷移、False：設定メニュー画面から管理者パスワードに遷移.
//================================================================================================
void PAdminPassword::setAdminPassGamenToDisplay( Bool bl_AdminPassGamenToDisplay )
{
	m_bl_AdminPassGamenToDisplay = bl_AdminPassGamenToDisplay;
}

// comment 2016/11/21 SCD2-Sky辻 [特ROM][FQ-OPSD-1600383-OFP252][CQ534112] 設定メニュー選択時に管理者パスワードをかけたマシンで、本体の前ドアを開閉するとパスワード無しでログインできる. 
//================================================================================================
// 概要		:管理者モード・管理者パスワード入力 : 画面遷移元判定フラグ取得.
// 戻り値	:True/False  True:ベース画面から管理者パスワードに遷移、False：設定メニュー画面から管理者パスワードに遷移.
// 備考		:
//================================================================================================
Bool PAdminPassword::getAdminPassGamenToDisplay( void )
{
	return m_bl_AdminPassGamenToDisplay;
}

//================================================================================================
// 概要		:管理者モード・管理者パスワードエラー画面 : １秒タイマ.
// 戻り値	:なし.
// 備考		:.
//================================================================================================
void PAdminPasswordError::procSecond( uchar option )
{
	static short ss_localCautionTimer = 0;
	// 親クラスの処理.
	PAdminKeyBoardCom::procSecond( option );
	// ５秒で警告を解除する.
	if( ++ss_localCautionTimer == 5 ) {
		// ユーザー認証エラーを解除する.
		nextGamen = &gp_AdminPassword;
		ss_localCautionTimer = 0;
	}
}
//================================================================================================
// 概要		:管理者モード・管理者パスワードエラー画面 : 画面開始処理.
// 戻り値	:なし
// 備考		:
//================================================================================================
void PAdminPasswordError::startGamen()
{
	PNLC_RMPStrInputMng::newInstance()->setInputRefuse( True );

	// パスワード表示フラグ.
	b_Password = True;
	// 表示エリア.
	pc_DispField = &Tfdc_AdminNetworkSetComInputKeybord;
	// 表示エリア.
	pc_ActivatableField = &Btx_AdminNetworkSetComInputKeybord;
	// キーボードにデータをセット.
	char _sc_AdminPassword[DEF_AdminPasswordBufSize];
	memset( _sc_AdminPassword, 0, sizeof( _sc_AdminPassword ) );
	setKeyboardData( _sc_AdminPassword, DEF_AdminPasswordSize, TYPD_AdminKeybordMax );
	PAdminKeyBoardCom::startGamen();
	// ユーティリティ専用キーボード共通開始処理.
	startGamenKeybordUtility9E();

	// キーボードを５ヶ国語キーボードとする.
	Kbd_UtilityInputKeybordConv.setKeybord5Country( True );

	// 左エリア種類の有効設定（ジョブ表示（通常））startGamen()の最後でコールしてください.
	setLeftAreaEnableType( TYPD_LeftAreaBookmark | TYPD_LeftAreaSoft10Key );
	setBuzzerType( PCGE_BuzzerType_Caution1 );		// 報知音設定.
	// タイトルの差し替え.
	// comment 2018/08/10 SCD2 T.Nishiba [Eagle][RQ-698-41]新設定メニューのメニュー項目対応.
	if( PNLE_AdminPasswordMoveGamen_AdminMyPanel == m_e_PassErrorNextGamen ) {
		Txt_UtilityAndAdminComTitle.changeText( s_TTLUtilityAdminMyPanel );
	} else if( PNLE_AdminPasswordMoveGamen_SimpleSecuritySetting  == m_e_PassErrorNextGamen ) {
		Txt_UtilityAndAdminComTitle.changeText( s_TTLSimpleSecuritySettings );
// comment 2022/07/25 Sky 前田  [IT6-89(機種マージ)]  USBドングルを使用したMFPロック機能対応.
	} else if( PNLE_AdminPasswordMoveGamen_UsbDongleRegist  == m_e_PassErrorNextGamen ) {
		Txt_UtilityAndAdminComTitle.changeText( s_pTTLUsbDongle_W608H16I );
	} else if( PNLE_AdminPasswordMoveGamen_SystemExpertModeMenu  == m_e_PassErrorNextGamen ) {
		Txt_UtilityAndAdminComTitle.changeText( s_pTTLUtilityExpertAdjustment_W608H016I );
	} else if( PNLE_AdminPasswordMoveGamen_SecurityHDDManageSet  == m_e_PassErrorNextGamen ) {
		Txt_UtilityAndAdminComTitle.changeText( s_pTTLUtilityStorageManagementSettings_W608H016I );
	} else if( PNLE_AdminPasswordMoveGamen_CardAttestationSetting == m_e_PassErrorNextGamen ) {
		Txt_UtilityAndAdminComTitle.changeText( s_pTTLUtilityCardAttestationSetting_W608H016I );
	} else {
		Txt_UtilityAndAdminComTitle.changeText( s_TTLUtilityAdministratorSetting );
	}
}
//================================================================================================
// 概要		:管理者モード・管理者パスワードエラー画面 : 表示画面のセットアップ
// 戻り値	:なし
// 備考		:
// 履歴		:2015/07/01 SecV1-5 【管理者によるサービスモード制御技術】 Sky 新道.
//================================================================================================
void PAdminPasswordError::setupGamen( short option )
{
	PAdminKeyBoardCom::setupGamen( option );
	// 入力できないキーを非表示にする.
	Kbd_UtilityInputKeybordConv.setDisableKeyCharCode( KC_DisableKeyPattern15 );	// パターン１５(").

	if( gp_AdminPassword.b_isPenalty ) {
		// ペナルティ用メッセージに切り替える
		Txt_MessageAdminPasswordError.changeText( s_MSGCertificatePenaltyAdminPasswordDisable );
	} else {
		// 通常エラーメッセージ
		Txt_MessageAdminPasswordError.changeText( s_MSGPasswordNotMatch );
	}
// 2014/09/18 【SecV1-5】CQ391654対応 日立 村上(幸).
	APIC_SystemSetting* pc_SystemSetting = APIC_SystemSetting::newInstance();
	if ( NULL == pc_SystemSetting ) {
		LAPC_BFW_System *pc_BFW_System=LAPC_BFW_System::newInstance();
		if ( NULL != pc_BFW_System ) {
    		pc_BFW_System->registDetailCode(PNLD_DetailCode_Admin_0x1900660011357401);
		}
//Comment 2019/06/14 [HCLT_Monisha][EagleH][N Rank] Removed unwanted Detailed code.
//		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, 3, "Assert Failed!" );
		return;
	}
	// 設定メニュー＞管理者設定以外から遷移した場合、「解除キー入力」キーを非表示.
	// 拡大表示されている状態で、「解除キー入力」キーを非表示.
	// サービスログイン許可設定が許可の時、「解除キー入力」キーを非表示.
	if( (PNLE_AdminPasswordMoveGamen_AdminSetting != m_e_PassErrorNextGamen)
		|| (PLBD_Enl_KBD_Enlarge == Kbd_UtilityInputKeybordConv.getChangeEnlKeybord())
		|| ( True == pc_SystemSetting->getEnableServiceLogin() ) ){
		Key_AdminPasswordReleaseKey.showEnable( False );
	} else {
		Key_AdminPasswordReleaseKey.showEnable( True );
	}
}
//================================================================================================
// 概要		:管理者モード・管理者パスワードエラー画面 : 背景画面のセット
// 戻り値	:なし
// 備考		:
//================================================================================================
void PAdminPasswordError::setDefaultBackParts()
{
	if( ( PLBD_Enl_KBD_Enlarge == Kbd_UtilityInputKeybordConv.getChangeEnlKeybord() )
	    && ( True == onlyPanel.b_InitKeybord ) ) {	// 拡大用デフォルト背景.
		setDefaultBackPartsInfo( ( PDisplayParts** )tbl_AdminPasswordError2Default, ss_AdminPasswordError2Default );
	} else {	// 標準用デフォルト背景.
		setDefaultBackPartsInfo( ( PDisplayParts** )tbl_AdminPasswordErrorDefault, ss_AdminPasswordErrorDefault );
	}
}
//================================================================================================
// 概要		:管理者モード・管理者パスワードエラー画面 : しおりの遷移可能不可能状態.
// 戻り値	:True/False  遷移する/遷移しない
// 備考		:
//================================================================================================
Bool PAdminPasswordError::isEnableNextGamenBookmark()
{
	return False;
}
// -----------------------------------------------------------------------------------------------
// クラス	：PAdminPasswordError.
// 関数名	：endGamen.
// 引数		：なし.
// 戻り値	：なし.
// 備考		：.
// 作成者	：OPS1 棚.
// -----------------------------------------------------------------------------------------------
void PAdminPasswordError::endGamen()
{
	// 受付け拒否を解除する.
	PNLC_RMPStrInputMng::newInstance()->setInputRefuse( False );

	PAdminKeyBoardCom::endGamen();
	//nextGamenがPadminPasswordクラスを継承してないかチェック.
	if( NULL == dynamic_cast< PAdminPassword*>( nextGamen ) ) {
//Comment 2015/12/17 HCL_Krithika [ZeusS_ZX0][制御構成変更_No.76-1] [Check-in issue] Not required case.
////	comment 2015/11/02 HCL_Ezhil [ZeusS_ZX0][制御構成変更_No.76-1] Adding error code.
//		LAPC_BFW_System *pc_BFW_System=LAPC_BFW_System::newInstance();
//		if (NULL !=pc_BFW_System){
//		    pc_BFW_System->registDetailCode(PNLD_DetailCode_Admin_0x1900670011313601);
//		}
// comment 2016/11/21 SCD2-Sky辻 [特ROM][FQ-OPSD-1600383-OFP252][CQ534112] 設定メニュー選択時に管理者パスワードをかけたマシンで、本体の前ドアを開閉するとパスワード無しでログインできる. 
		if ( True == gp_AdminPassword.getAdminPassGamenToDisplay()
// comment 2022/07/25 Sky 前田  [IT6-89(機種マージ)]  USBドングルを使用したMFPロック機能対応.
//			|| PNLE_AdminPasswordMoveGamen_SimpleSecuritySetting  == m_e_PassErrorNextGamen ) {
			|| PNLE_AdminPasswordMoveGamen_SimpleSecuritySetting  == m_e_PassErrorNextGamen || PNLE_AdminPasswordMoveGamen_UsbDongleRegist  == m_e_PassErrorNextGamen ) {
			setTurnGamenThis( NULL );
		}
		//遷移先がPadminPassword以外であれば、フラグを落とす.
		gp_AdminPassword.setAdminPassGamenToDisplay( False );
// comment 2016/11/21 SCD2-Sky辻 [特ROM][FQ-OPSD-1600383-OFP252][CQ534112] 設定メニュー選択時に管理者パスワードをかけたマシンで、本体の前ドアを開閉するとパスワード無しでログインできる. 
		gp_AdminPassword.setNextGamen( PNLE_AdminPasswordMoveGamen_AdminSetting );
	}
}
//================================================================================================
// 概要		:管理者モード・管理者パスワードエラー画面 : 遷移元画面の設定.
// 戻り値	:
// 備考		:
//================================================================================================
void PAdminPasswordError::setNextGamen( PNLE_AdminPasswordNextGamen	e_NextGamen )	// 遷移先画面指定.
{
	m_e_PassErrorNextGamen = e_NextGamen;
}

//================================================================================================.
// 概要		:管理者モード・管理者パスワードエラー画面 : HOME(メニュー)画面への遷移処理
// 戻り値	:なし.
// 備考		:.
// 履歴		:2018/9/27 SCD2-Sky二口 [Eagle][AR1062689]管理者パスワード入力画面のHOMEキー押下対応.
//================================================================================================.
void PAdminPasswordError::procMenu( uchar option )
{
	goToHomeMenu();
}
//================================================================================================
// 概要		:管理者モード・管理者パスワードエラー画面 : 遷移先画面取得.
// 戻り値	:PNLE_AdminPasswordNextGamen : .
// 備考		:
//================================================================================================
PNLE_AdminPasswordNextGamen PAdminPasswordError::getNextGamen()	// 遷移先画面取得.
{
	return	m_e_PassErrorNextGamen;
}
//================================================================================================
// 概要		:管理者設定・遷移不可画面共通 : しおりの遷移可能不可能状態.
//================================================================================================
Bool PAdminInvalidKeyGamencom::isEnableNextGamenBookmark()
{
	return False;
}
// 概要		:管理者設定メニュー  : 閉じる.
// 戻り値	:なし.
// 備考		:.
void PToAdminMenucom::procOk( uchar option )
{
	PUtilitycom::b_FromAdminMode = True;	// 管理者モードからの遷移フラグ設定.
	setAdminGamenExit();
	nextGamen = &gp_UtilityMenu;	// ユーティリティ・トップメニュー
// アプリキーの割り付け方が変更されたときのために、LEDを一度全てONにして、再度OFFにする.
#if defined(DEF_THAMES_ICUSTOM) && defined(LINUX)
	checkLED();
#else
	reqLED( PNLE_LED_Box,    		PNLE_LEDStateOn );
	reqLED( PNLE_LED_Scaner, 		PNLE_LEDStateOn );
	reqLED( PNLE_LED_Copy,  		PNLE_LEDStateOn );
	reqLED( PNLE_LED_Fax,  			PNLE_LEDStateOn );
	reqLED( PNLE_LED_ImageDealer,	PNLE_LEDStateOn );
	reqLED( PNLE_LED_MyPanel,		PNLE_LEDStateOn );
	reqLED( PNLE_LED_Browser,		PNLE_LEDStateOn );
	reqLED( PNLE_LED_AppMenu,		PNLE_LEDStateOn );

	reqLED( PNLE_LED_Box,    		PNLE_LEDStateOff );
	reqLED( PNLE_LED_Scaner, 		PNLE_LEDStateOff );
	reqLED( PNLE_LED_Copy,  		PNLE_LEDStateOff );
	reqLED( PNLE_LED_Fax,  			PNLE_LEDStateOff );
	reqLED( PNLE_LED_ImageDealer,	PNLE_LEDStateOff );
	reqLED( PNLE_LED_MyPanel,		PNLE_LEDStateOff );
	reqLED( PNLE_LED_Browser,		PNLE_LEDStateOff );
	reqLED( PNLE_LED_AppMenu,		PNLE_LEDStateOff );
#endif
}
//================================================================================================
// 概要		:管理者設定メニュー : 終了
// 戻り値	:なし
// 備考		:
//================================================================================================
void PToAdminMenucom::procEnd( uchar option )
{
	setAdminGamenExit();
	PUtilityToMenuCom::procEnd( option );
}
//================================================================================================
// 概要		:管理者設定メニュー : 画面移動
// 戻り値	:なし
//================================================================================================
void PAdminSetMenu::procGoToPage( uchar option )
{
// comment 2022/04/04 Sky西場 [IT6_2.3][OP_BTS-37620][スリム化].
	ExpPanelLog( PNLD_ExpLogKind_AlwaysLogOut, "[DeleteUnnecessaryProcessing] [%s]", __PRETTY_FUNCTION__, 0, 0, 0, 0, 0 );
//	APIC_NetworkDevice* pc_NetworkDevice = APIC_NetworkDevice::newInstance();    // ネットワークデバイス（ＡＰＩ）・インスタンス取得
//	PNLE_IsFaxMode e_isFaxmode = PNLE_IsFaxMode_NON;	// FAX接続種別
//	TYPE_KeyMode e_KeyMode = pc_MachineSetting->getKeyMode();				//	キーモード.
//	if( bl_BumonLimitWarningAdminMove == False ) {	// 部門上限値オーバー発生時管理者モードに遷移したフラグ
//		// FAX接続種別の取得
//		onlyPanel.isFaxModekind( &e_isFaxmode );
//		if( ( e_isFaxmode != PNLE_IsFaxMode_NON ) && ( e_isFaxmode != PNLE_IsFaxMode_RFAX ) ) {
//			if( ppc_SystemInfo->getExternalIFMode() == TYPE_ExternalIFMode1 ) {
//				switch( option ) {
//					case  1:	// 管理者設定・環境設定
//						b_fromMenu = True;
//						nextGamen = &gp_AdminSystemSet;
//						break;
//					case  2: nextGamen = &gp_AdminAdminAndSelfSet;	break;	// 管理者設定・管理者/本体登録
//					case  3: nextGamen = &gp_AdminAddressInput;		break;	// 管理者設定・宛先登録
//					case  4:	// 管理者設定・ユーザ認証/部門管理
//						if( e_KeyMode == TYPE_KeyModeVendor || e_KeyMode == TYPE_KeyModeVendorKM || e_KeyMode == TYPE_KeyModePageAcses ) {
//							setBuzzerType( PCGE_BuzzerType_InputNg );	// 報知音設定
//							return;
//						}
//						//キーカウンター装置時は、ユーザ認証/部門管理画面へ遷移しない.
//						if( PnlComGetMachineSetting()->getKeyMode() != TYPE_KeyModeKeyCounter ) {
//							b_fromMenu = True;
//							nextGamen = &gp_AdminUserAndDepartSet;
//						}
//						break;
//					case  5:	// 管理者設定・ネットワーク設定
//						if( False == pc_NetworkDevice -> isNetworkInitialing() ) {	// ネットワーク初期化中は遷移できない.
//							b_fromMenu = True;
//							nextGamen = &gp_AdminNetworkSet;
//						}
//						break;
//					case  6:	// 管理者設定・コピー設定
//						// 従来ベンダモード時は、コピー設定画面に遷移しないようにする.
//						if( e_KeyMode == TYPE_KeyModeVendor ) {
//							setBuzzerType( PCGE_BuzzerType_InputNg );	// 報知音設定
//							return;
//						}
//						nextGamen = &gp_AdminCopySetP1;
//						break;
//					case  7: nextGamen = &gp_AdminFaxSetting;		break;	// 管理者設定・ファクス設定
//					case  8: nextGamen = &gp_AdminSysContactSet;	break;	// 管理者設定・システム連携設定
//					case  9: nextGamen = &gp_AdminSecuritySet;		break;	// 管理者設定・セキュリティ設定
//					case 10: PToAdminMenucom::procOk( option );	break;	// ユーティリティ・トップメニュー
//					case BTNID_KEY_LEFTINDEXFunctionSearch:
//						gp_FunctionSearch_act.setSearchMode( PNLD_SearchFunctionAdmin );
//// Comment 2015/10/28 HCLT-Surajit [ZeusS_ZX0][制御構成変更_No 76][WP 2501] Counter value to be updated.
//						PnlComIncreaseOperateCnt(TYPE_OperateCnt_FuncSearchKey);                          //Counter value  updation.
//						PNLC_PopupListControl::newInstance()->addPopup( &gp_FunctionSearch_act, this, NULL, 0, 0, 0, 0, True );			// 機能検索画面.
//						break;
//					default:
//						setBuzzerType( PCGE_BuzzerType_InputNg );	break;	// 報知音設定
//				}
//			} else {
//				switch( option ) {
//					case  1:	// 管理者設定・環境設定
//						b_fromMenu = True;
//						nextGamen = &gp_AdminSystemSet;
//						break;
//					case  2: nextGamen = &gp_AdminAdminAndSelfSet;	break;	// 管理者設定・管理者/本体登録
//					case  3: nextGamen = &gp_AdminAddressInput;		break;	// 管理者設定・宛先登録
//					case  4:	// 管理者設定・ユーザ認証/部門管理
//						if( e_KeyMode == TYPE_KeyModeVendor || e_KeyMode == TYPE_KeyModeVendorKM || e_KeyMode == TYPE_KeyModePageAcses ) {
//							setBuzzerType( PCGE_BuzzerType_InputNg );	// 報知音設定
//							return;
//						}
//						//キーカウンター装置時は、ユーザ認証/部門管理画面へ遷移しない.
//						if( PnlComGetMachineSetting()->getKeyMode() != TYPE_KeyModeKeyCounter )
//						{ nextGamen = &gp_AdminUserAndDepartSet; }
//						break;
//					case  5:	// 管理者設定・ネットワーク設定
//						if( False == pc_NetworkDevice -> isNetworkInitialing() ) {	// ネットワーク初期化中は遷移できない.
//							b_fromMenu = True;
//							nextGamen = &gp_AdminNetworkSet;
//						}
//						break;
//					case  6:	// 管理者設定・コピー設定
//						// 従来ベンダモード時は、コピー設定画面に遷移しないようにする.
//						if( e_KeyMode == TYPE_KeyModeVendor ) {
//							setBuzzerType( PCGE_BuzzerType_InputNg );	// 報知音設定
//							return;
//						}
//						nextGamen = &gp_AdminCopySetP1;
//						break;
//					case  7:	// 管理者設定・プリンタ設定
//						nextGamen = &gp_AdminPrinterSet;
//						break;
//					case  8: nextGamen = &gp_AdminFaxSetting;		break;	// 管理者設定・ファクス設定
//					case  9: nextGamen = &gp_AdminSysContactSet;	break;	// 管理者設定・システム連携設定
//					case  0: nextGamen = &gp_AdminSecuritySet;		break;	// 管理者設定・セキュリティ設定
//					case 10: PToAdminMenucom::procOk( option );	break;	// ユーティリティ・トップメニュー
//					case BTNID_KEY_LEFTINDEXFunctionSearch:
//						gp_FunctionSearch_act.setSearchMode( PNLD_SearchFunctionAdmin );
//// Comment 2015/10/28 HCLT-Surajit [ZeusS_ZX0][制御構成変更_No 76][WP 2501] Counter value to be updated.
//                        PnlComIncreaseOperateCnt(TYPE_OperateCnt_FuncSearchKey);                          //Counter value  updation.
//						PNLC_PopupListControl::newInstance()->addPopup( &gp_FunctionSearch_act, this, NULL, 0, 0, 0, 0, True );			// 機能検索画面.
//						break;
//					default: break;
//				}
//			}
//		} else {	// ファクス回線が未実装のとき
//			if( ppc_SystemInfo->getExternalIFMode() == TYPE_ExternalIFMode1 ) {
//				switch( option ) {
//					case  1:	// 管理者設定・環境設定
//						b_fromMenu = True;
//						nextGamen = &gp_AdminSystemSet;
//						break;
//					case  2: nextGamen = &gp_AdminAdminAndSelfSet;	break;	// 管理者設定・管理者/本体登録
//					case  3: nextGamen = &gp_AdminAddressInput;		break;	// 管理者設定・宛先登録
//					case  4:	// 管理者設定・ユーザ認証/部門管理
//						if( e_KeyMode == TYPE_KeyModeVendor || e_KeyMode == TYPE_KeyModeVendorKM || e_KeyMode == TYPE_KeyModePageAcses ) {
//							setBuzzerType( PCGE_BuzzerType_InputNg );	// 報知音設定
//							return;
//						}
//						//キーカウンター装置時は、ユーザ認証/部門管理画面へ遷移しない.
//						if( PnlComGetMachineSetting()->getKeyMode() != TYPE_KeyModeKeyCounter )
//						{ nextGamen = &gp_AdminUserAndDepartSet; }
//						break;
//					case  5:	// 管理者設定・ネットワーク設定
//						if( False == pc_NetworkDevice -> isNetworkInitialing() ) {	// ネットワーク初期化中は遷移できない.
//							b_fromMenu = True;
//							nextGamen = &gp_AdminNetworkSet;
//						}
//						break;
//					case  6:	// 管理者設定・コピー設定
//						// 従来ベンダモード時は、コピー設定画面に遷移しないようにする.
//						if( e_KeyMode == TYPE_KeyModeVendor ) {
//							setBuzzerType( PCGE_BuzzerType_InputNg );	// 報知音設定
//							return;
//						}
//						nextGamen = &gp_AdminCopySetP1;
//						break;
//					case  7: nextGamen = &gp_AdminSysContactSet; break;	// 管理者設定・システム連携設定
//					case  8: nextGamen = &gp_AdminSecuritySet;	break;	// 管理者設定・セキュリティ設定
//					case 10: PToAdminMenucom::procOk( option );	break;	// ユーティリティ・トップメニュー
//					case BTNID_KEY_LEFTINDEXFunctionSearch:
//						gp_FunctionSearch_act.setSearchMode( PNLD_SearchFunctionAdmin );
//// Comment 2015/10/28 HCLT-Surajit [ZeusS_ZX0][制御構成変更_No 76][WP 2501] Counter value to be updated.
//					    PnlComIncreaseOperateCnt(TYPE_OperateCnt_FuncSearchKey);                          //Counter value  updation.
//						PNLC_PopupListControl::newInstance()->addPopup( &gp_FunctionSearch_act, this, NULL, 0, 0, 0, 0, True );			// 機能検索画面.
//						break;
//					default: break;
//				}
//			} else {
//				switch( option ) {
//					case  1:	// 管理者設定・環境設定
//						b_fromMenu = True;
//						nextGamen = &gp_AdminSystemSet;
//						break;
//					case  2: nextGamen = &gp_AdminAdminAndSelfSet;	break;	// 管理者設定・管理者/本体登録
//					case  3: nextGamen = &gp_AdminAddressInput;		break;	// 管理者設定・宛先登録
//					case  4:	// 管理者設定・ユーザ認証/部門管理
//						if( e_KeyMode == TYPE_KeyModeVendor || e_KeyMode == TYPE_KeyModeVendorKM || e_KeyMode == TYPE_KeyModePageAcses ) {
//							setBuzzerType( PCGE_BuzzerType_InputNg );	// 報知音設定
//							return;
//						}
//						//キーカウンター装置時は、ユーザ認証/部門管理画面へ遷移しない.
//						if( PnlComGetMachineSetting()->getKeyMode() != TYPE_KeyModeKeyCounter ) {
//							b_fromMenu = True;
//							nextGamen = &gp_AdminUserAndDepartSet;
//						}
//						break;
//					case  5:	// 管理者設定・ネットワーク設定
//						if( False == pc_NetworkDevice -> isNetworkInitialing() ) {	// ネットワーク初期化中は遷移できない.
//							b_fromMenu = True;
//							nextGamen = &gp_AdminNetworkSet;
//						}
//						break;
//					case  6:	// 管理者設定・コピー設定
//						// 従来ベンダモード時は、コピー設定画面に遷移しないようにする.
//						if( e_KeyMode == TYPE_KeyModeVendor ) {
//							setBuzzerType( PCGE_BuzzerType_InputNg );	// 報知音設定
//							return;
//						}
//						nextGamen = &gp_AdminCopySetP1;
//						break;
//					case  7: nextGamen = &gp_AdminPrinterSet;		break;	// 管理者設定・プリンタ設定
//					case  8: nextGamen = &gp_AdminSysContactSet;	break;	// 管理者設定・システム連携設定
//					case  9: nextGamen = &gp_AdminSecuritySet;		break;	// 管理者設定・セキュリティ設定
//					case 10: PToAdminMenucom::procOk( option );	break;	// ユーティリティ・トップメニュー
//					case BTNID_KEY_LEFTINDEXFunctionSearch:
//						gp_FunctionSearch_act.setSearchMode( PNLD_SearchFunctionAdmin );
//// Comment 2015/10/28 HCLT-Surajit [ZeusS_ZX0][制御構成変更_No 76][WP 2501] Counter value to be updated.
//						PnlComIncreaseOperateCnt(TYPE_OperateCnt_FuncSearchKey);                          //Counter value  updation.
//						PNLC_PopupListControl::newInstance()->addPopup( &gp_FunctionSearch_act, this, NULL, 0, 0, 0, 0, True );			// 機能検索画面.
//						break;
//					default: break;
//				}
//			}	// Fiery ユーティリティUIマスク対応
//		}
//	} else {
//		if( option == 4 )
//		{ nextGamen = &gp_AdminUserAndDepartSet; }	// 管理者設定・ユーザ認証/部門管理
//		else if( option == 10 )
//		{ PToAdminMenucom::procOk( option ); }		// ユーティリティ・トップメニュー
//	}
}
//================================================================================================
// 概要		:管理者設定メニュー : テンキー
// 戻り値	:なし
//================================================================================================
void PAdminSetMenu::proc10Key( uchar option )
{
	if( option <= 9 ) {
		procGoToPage( option );
	}
}
//===================================================================.
/**
* @brief 	: To launch the screen.
* @note 	: None.
* @param 	: option.
* @return 	: None.
* @author 	: HCLT-Rengaraaj RQ-163[WP 1353] Hide External Memory Backup Button..
*/
//===================================================================.
void PAdminSetMenu::startGamen()
{
	PToAdminMenucom::startGamen();
	if ( True == PLBC_FuncVersionCheck::isOK_IT5_Type4() ) {
		PNLC_USBConnectionPermission c_UsbPermission;
		PNLE_EnableUSBConnectionPermissionForAll e_usbAll = c_UsbPermission.getEnableUSBConnectionPermissionForAll();
		PNLE_EnableUSBConnectionPermissionForAdmin e_usbforAdmin = c_UsbPermission.getEnableUSBConnectionPermissionForAdmin();
		PNLE_EnableUSBConnectionPermissionForService e_usbforService = c_UsbPermission.getEnableUSBConnectionPermissionForService();
		if( ( PNLE_EnableUSBConnectionPermissionForAll_Disable == e_usbAll ) || ( PNLE_EnableUSBConnectionPermissionForAdmin_Disable == e_usbforAdmin ) ) {
			m_bl_USBConnectionPermissionFlag = False;
		} else {
			m_bl_USBConnectionPermissionFlag = True;
		}
		if( ( PNLE_EnableUSBConnectionPermissionForAll_Disable == e_usbAll ) || ( PNLE_EnableUSBConnectionPermissionForService_Disable == e_usbforService ) ) {
			m_bl_USBConnectionPermissionServiceFlag = False;
		} else {
			m_bl_USBConnectionPermissionServiceFlag = True;
		}
	}
	// 左エリア種類の有効設定（ジョブ表示（通常））startGamen()の最後でコールしてください.
	setLeftAreaEnableType( TYPD_LeftAreaBookmark | TYPD_LeftAreaSoft10Key );
}
//================================================================================================
// 概要		:管理者設定メニュー : 表示画面のセットアップ.
// 戻り値	:なし
//================================================================================================
void PAdminSetMenu::setupGamen( short option )
{
// Comment 2017/11/14【HCLT-Abinav】[IT5 4.2]【AR1032658】Hiding Fax settings, Cloud cooperation, System cooperation, OpenAPI authentication management setting, Remote diagnosis setting buttons.
	TYPE_NetworkSpecific e_NetworkSpecific = PNLC_NetworkLan::getInstance()->getAdminNWLine();
	BRWC_Browser::PanelToTask( BRWD_PToT_MES_DeleteCheckReq ); //CQ_386518対応
	Bool bl_EnableHDDExistFunc = True;				// HDD状態による機能有効フラグ.
	if( False == PnlComIsUseSsdScanSend() ) {	// HDD/SSD使用不可.
		bl_EnableHDDExistFunc = False;
	}

	PToAdminMenucom::setupGamen( option );
	APIC_NetworkDevice* pc_NetworkDevice = APIC_NetworkDevice::newInstance();    // ネットワークデバイス（ＡＰＩ）・インスタンス取得
	PNLE_IsFaxMode e_isFaxmode = PNLE_IsFaxMode_NON;	// FAX接続種別


	// HDD非装着、または画像ログ上限警告時.
	if( False == bl_EnableHDDExistFunc || True == bl_ImageLogOverError ) {
		changeDispStatus( PNLE_AdminMenu_Destination , PNLE_PARTSSTATUS_HIDE );	// 宛先登録キー.
	}
	if( bl_BumonLimitWarningAdminMove == False ) {	// 部門上限値オーバー発生時管理者モードに遷移したフラグ
		Btx_AdminMenuNowPageRevSideErea.set( &Num_AdminMenuNowPage, uc_page, False );
		Btx_AdminMenuNowPageRevSideErea.set( &Num_AdminMenuAllPage, 2, False );
		if( False == checkNextPageContents() ) {
			pc_MenuUtilityAndAdminAdd3Page.changeSPCMaxPage( 1 );		// 最大ページ数1.
			Nfd_SPC_UtilityAndAdminMenuTotalPage.showEnable( False );			// 総ページ.
			Nfd_SPC_UtilityAndAdminMenuDspPage.showEnable( False );			// 現ページ.
			Grp_SPC_UtilityAndAdminMenuPage.showEnable( False );				// ページ表示グループ.
			Bak_SPC_UtilityAndAdminMenuPageDsp.showEnable( False );
		}
		if( uc_page == 1 ) {	// 1ページ目.
			onlyPanel.isFaxModekind( &e_isFaxmode );
			if( e_isFaxmode != PNLE_IsFaxMode_NON ) {
				// HDD非装着、または画像ログ上限警告時発生時.
// Comment 2017/11/14【HCLT-Abinav】[IT5 4.2]【AR1032658】Hiding Fax settings key for Network2.
//				if( False == bl_EnableHDDExistFunc || True == bl_ImageLogOverError )
				if( False == bl_EnableHDDExistFunc || True == bl_ImageLogOverError || TYPE_NetworkSpecific_NW2 == e_NetworkSpecific ) {
					changeDispStatus( PNLE_AdminMenu_FaxSet , PNLE_PARTSSTATUS_HIDE );		// ファクス設定.
				} else {
					changeDispStatus( PNLE_AdminMenu_FaxSet , PNLE_PARTSSTATUS_ENABLE );		// ファクス設定.
				}
// Comment 2017/12/29【HCLT-Sashmitha】[IT5 4.2]【AR1035621】Display System cooperation key for Network2.
//				if( ( False == pc_NetworkDevice -> isNetworkInitialing()	// ネットワーク初期化中でない.
				if( False == pc_NetworkDevice -> isNetworkInitialing()	// ネットワーク初期化中でない.
// Comment 2017/11/14【HCLT-Abinav】[IT5 4.2]【AR1032658】Hiding System cooperation key for Network2.
				    && False == bl_ImageLogOverError ) {					// 画像ログ上限警告でない.
//					&& False == bl_ImageLogOverError ) && ( TYPE_NetworkSpecific_NW2 != e_NetworkSpecific ) ) {
					changeDispStatus( PNLE_AdminMenu_SysContactSet , PNLE_PARTSSTATUS_ENABLE );		// システム連携キー. 
				} else {
					changeDispStatus( PNLE_AdminMenu_SysContactSet , PNLE_PARTSSTATUS_HIDE );		// システム連携キー.
				}
			} else {	// ファクス回線が未実装のとき
// Comment 2017/12/29【HCLT-Sashmitha】[IT5 4.2]【AR1035621】Display System cooperation key for Network2.
//// Comment 2017/11/14【HCLT-Abinav】[IT5 4.2]【AR1032658】Hiding System cooperation key for Network2.
//				if( False == pc_NetworkDevice -> isNetworkInitialing()			// ネットワーク初期化中でない.
//				    && False == bl_ImageLogOverError ) {						// 画像ログ上限警告でない.
				if( False == pc_NetworkDevice -> isNetworkInitialing()			// ネットワーク初期化中でない.
				    && False == bl_ImageLogOverError ) {						// 画像ログ上限警告でない.
//                if(( False == pc_NetworkDevice->isNetworkInitialing()        // ネットワーク初期化中でない.
//                   && False == bl_ImageLogOverError )                      // 画像ログ上限警告でない.
//                     && ( TYPE_NetworkSpecific_NW2 != e_NetworkSpecific )) { // Login network is not Network2.
					changeDispStatus( PNLE_AdminMenu_SysContactSet , PNLE_PARTSSTATUS_ENABLE );		// システム連携キー.
				} else {
					changeDispStatus( PNLE_AdminMenu_SysContactSet , PNLE_PARTSSTATUS_HIDE );		// システム連携キー.
				}
			}
			TYPE_KeyMode e_KeyMode = pc_MachineSetting->getKeyMode();				//	キーモード.
			if( ( PnlComGetMachineSetting()->getKeyMode() != TYPE_KeyModeKeyCounter )
			    && ( e_KeyMode != TYPE_KeyModeVendor || True == APIC_SoftDipSw::newInstance()->getSoftDipSwBitForPanelSpecNo( 35, 4 ) )
			    && ( ( ( TYPE_KeyModeVendorKM != e_KeyMode ) && ( False == PLBC_FuncVersionCheck::isOK_TypeMLK() ) )		// ベンダー２装着時ではない、かつ FuncVer2以降ではない.
			         || ( True == PLBC_FuncVersionCheck::isOK_TypeMLK() ) )													// もしくはFuncVer2以降.
			    && ( False == bl_ImageLogOverError && uc_page == 1 ) ) {		// 1ページ目で画像ログ上限警告発生していない場合. No.310819 2008/11/27 12柳川
				changeDispStatus( PNLE_AdminMenu_ManageUserInfo , PNLE_PARTSSTATUS_ENABLE );
			} else {
				changeDispStatus( PNLE_AdminMenu_ManageUserInfo , PNLE_PARTSSTATUS_HIDE );
			}
			// ネットワーク初期化中はネットワークキーを表示しない.
			if( False == pc_NetworkDevice -> isNetworkInitialing() ) {
				changeDispStatus( PNLE_AdminMenu_NetworkSet , PNLE_PARTSSTATUS_ENABLE );		// ネットワーク設定.
				APIC_SoftDipSw* pc_SoftDipSw = APIC_SoftDipSw::newInstance();
				if( NULL == pc_SoftDipSw ) {
//	comment 2015/11/02 HCL_Ezhil [ZeusS_ZX0][制御構成変更_No.76-1] Adding error code.
//Comment 2019/06/14 [HCLT_Monisha][EagleH][N Rank] Removed unwanted Detailed code.
//					LAPC_BFW_System *pc_BFW_System=LAPC_BFW_System::newInstance();
//					if (NULL !=pc_BFW_System){
//					    pc_BFW_System->registDetailCode(PNLD_DetailCode_Admin_0x1900660011513701);
//					}
					LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
					return;
				}
// Comment 2017/11/14【HCLT-Abinav】[IT5 4.2]【AR1032658】Hiding Cloud cooperation key for Network2.
//				if( True == pc_SoftDipSw->getSoftDipSwBitForPanelSpecNo( 157, 4 ) ) {
				if( True == pc_SoftDipSw->getSoftDipSwBitForPanelSpecNo( 157, 4 ) && TYPE_NetworkSpecific_NW2 != e_NetworkSpecific ) {
					changeDispStatus( PNLE_AdminMenu_CloudContact , PNLE_PARTSSTATUS_ENABLE );		// クラウド連携設定.
				} else {
					changeDispStatus( PNLE_AdminMenu_CloudContact , PNLE_PARTSSTATUS_HIDE );		// クラウド連携設定.
				}
			} else {
				changeDispStatus( PNLE_AdminMenu_NetworkSet , PNLE_PARTSSTATUS_HIDE );			// ネットワーク設定.
				changeDispStatus( PNLE_AdminMenu_CloudContact , PNLE_PARTSSTATUS_HIDE );		// クラウド連携設定.
			}
			// コピー設定キーは従来ベンダモード時は表示しない.
			if( e_KeyMode == TYPE_KeyModeVendor						// 従来ベンダモード.
			    || True == bl_ImageLogOverError ) {					// 画像ログ上限警告時.
				changeDispStatus( PNLE_AdminMenu_CopySet , PNLE_PARTSSTATUS_HIDE );			// コピー設定キー.
			} else {
				changeDispStatus( PNLE_AdminMenu_CopySet , PNLE_PARTSSTATUS_ENABLE );			// コピー設定キー.
			}
			if( True == PLBC_FuncVersionCheck::isOK_MiZe_Type3() ) { // 機能拡張3以降.
				if( False == pc_NetworkDevice -> isNetworkInitialing()	// ネットワーク初期化中でない.
				    && False == bl_ImageLogOverError ) {					// 画像ログ上限警告でない.
					TYPE_NetworkIFMode e_NetworkIFMode = APIC_ExNetworkSetting::newInstance()->getNetworkIFMode();
					if( ( True == PnlComGetSystemInfo()->getExNetworkAdapter() )				// 拡張ネットワークアダプタ装着時.
					    && ( TYPE_NetworkIFMode_DualEthernetSta == e_NetworkIFMode				// デュアルIF構成 有線＋無線子機モード.
					         || TYPE_NetworkIFMode_DualEthernetWolLimited == e_NetworkIFMode ) ) {	// デュアルIF構成 有線＋無線子機(ErP復帰限定)モード.
						changeDispStatus( PNLE_AdminMenu_ExNetworkSetting , PNLE_PARTSSTATUS_ENABLE );	// 拡張ネットワーク設定 .
					} else {
						changeDispStatus( PNLE_AdminMenu_ExNetworkSetting , PNLE_PARTSSTATUS_HIDE );	// 拡張ネットワーク設定 .
					}
				} else {
					changeDispStatus( PNLE_AdminMenu_ExNetworkSetting , PNLE_PARTSSTATUS_HIDE );	// 拡張ネットワーク設定 .
				}
			}
		} else {	// 2ページ.
			// 以下条件の場合のみ、「音声ガイド設定」キーを表示.
			// ・仕向けが日本の場合.
			// ・音声ライセンス有効の場合.
			// ・音声データインストール済みの場合.
			// ・PKI認証設定時以外
			//仕向け条件解除.
			if( TYPE_CategoryState_ENABLE == PnlComGetEnablerSetting()->getFEK_CotegoryState( TYPE_FEKType09 )
			    && True == PnlComGetMachineSetting()->getVoiceInstallFlgInfo()
			  ) {
				changeDispStatus( PNLE_AdminMenu_VoiceGuide , PNLE_PARTSSTATUS_ENABLE );		// 音声ガイド設定.
			} else {
				changeDispStatus( PNLE_AdminMenu_VoiceGuide , PNLE_PARTSSTATUS_HIDE );			// 音声ガイド設定.
			}
// Comment 2017/11/14【HCLT-Abinav】[IT5 4.2]【AR1032658】Hiding OpenAPI authentication management setting key for Network2.
            if( TYPE_NetworkSpecific_NW2 != e_NetworkSpecific ) {
			changeDispStatus( PNLE_AdminMenu_OAPAttestationManagement , PNLE_PARTSSTATUS_ENABLE );		// OpenAPI認証管理キー 表示.
            }	
			changeDispStatus( PNLE_AdminMenu_LicenseManage , PNLE_PARTSSTATUS_ENABLE );		// ライセンス管理設定 .
//comment 2015/03/25 HCLT-Venkataramesh [HeliosL] [KRQ-101] Authentication function code button enabling if image log error is not there.
			changeDispStatus( PNLE_AdminMenu_AuthFuncCode , PNLE_PARTSSTATUS_ENABLE );		// 認証機能有効化設定 .

			Bool bl_MaintenanceMode = APIC_SystemInfo::newInstance()->getEnableMaintenanceMode(); // State of Maintenance Mode

			APIC_SoftDipSw* pc_SoftDipSw = APIC_SoftDipSw::newInstance();		// DipSw（API）・インスタンス取得.

//Comment 2015/07/31 HCLT-Rengaraaj RQ-163[WP 1353] Hide External Memory Backup Button.
			if( True == PLBC_FuncVersionCheck::isOK_IT5_Type4() ) {
				if( False == bl_MaintenanceMode ) {									//メンテナンスモードOFF.
					if( ( True == pc_SoftDipSw->getSoftDipSwBit( 38, 7 ) )				// DipSW No.72が0x04(使用する)だった場合.
						&& ( False == pc_NetworkDevice -> isNetworkInitialing() )		// ネットワーク初期化中でない.
						&& ( True == m_bl_USBConnectionPermissionFlag ) ) {				//USB接続許可設定の[一括設定]または[外部メモリー(管理者)]が[禁止]ではない.
						changeDispStatus( PNLE_AdminMenu_ExtBackupMenu , PNLE_PARTSSTATUS_ENABLE );
					} else {
						changeDispStatus( PNLE_AdminMenu_ExtBackupMenu , PNLE_PARTSSTATUS_HIDE );
					}
				} else {																//メンテナンスモードON.
					if( ( False == pc_NetworkDevice -> isNetworkInitialing() ) 			// ネットワーク初期化中でない.
						&& ( True == m_bl_USBConnectionPermissionServiceFlag) ) {		//USB接続許可設定の[一括設定]または[外部メモリー(サービス)]が[禁止]ではない.
						changeDispStatus( PNLE_AdminMenu_ExtBackupMenu , PNLE_PARTSSTATUS_ENABLE );
					} else {
						changeDispStatus( PNLE_AdminMenu_ExtBackupMenu , PNLE_PARTSSTATUS_HIDE );
					}
				}
			} else {
				if((True == pc_SoftDipSw->getSoftDipSwBit( 38, 7 ))					// DipSW No.72が0x04(使用する)だった場合
					&& (False == pc_NetworkDevice -> isNetworkInitialing())) {
					changeDispStatus( PNLE_AdminMenu_ExtBackupMenu , PNLE_PARTSSTATUS_ENABLE );		// 外部メモリーバックアップ.
				} else if((True == bl_MaintenanceMode) && (False == pc_NetworkDevice -> isNetworkInitialing())){
					changeDispStatus( PNLE_AdminMenu_ExtBackupMenu , PNLE_PARTSSTATUS_ENABLE );		// 外部メモリーバックアップ.
				} else {
					changeDispStatus( PNLE_AdminMenu_ExtBackupMenu , PNLE_PARTSSTATUS_HIDE );		// 外部メモリーバックアップ.
				}
			}
// Comment 2017/11/14【HCLT-Abinav】[IT5 4.2]【AR1032658】Hiding Remote diagnosis setting key for Network2.
//			if( True == isConnectedCSRC() )
			if( True == isConnectedCSRC() && TYPE_NetworkSpecific_NW2 != e_NetworkSpecific ) {
				changeDispStatus( PNLE_AdminMenu_RemoteDiagnosiskSet , PNLE_PARTSSTATUS_ENABLE );	// 遠隔診断設定.
			} else {
				changeDispStatus( PNLE_AdminMenu_RemoteDiagnosiskSet , PNLE_PARTSSTATUS_HIDE );		// 遠隔診断設定.
			}
			if( True == bl_ImageLogOverError ) {					// 画像ログ上限警告時、Ecoコピー設定キー非表示.
				changeDispStatus( PNLE_AdminMenu_EcoCopySetting , PNLE_PARTSSTATUS_HIDE );			// Ecoコピー設定キー非表示.
// Comment 2015/05/06 HCLT-Chandrasekar [HeliosL][CQ512869]  Disable the Keys when Image log over error occurs.
	  	  	changeDispStatus( PNLE_AdminMenu_AuthFuncCode, PNLE_PARTSSTATUS_HIDE ); 				// 認証機能有効化設定.
	  	  	changeDispStatus( PNLE_AdminMenu_OAPAttestationManagement, PNLE_PARTSSTATUS_HIDE );		// OpenAPI認証管理設定 .
			changeDispStatus( PNLE_AdminMenu_LicenseManage, PNLE_PARTSSTATUS_HIDE ); 				// ライセンス管理設定 .
			} else {
				changeDispStatus( PNLE_AdminMenu_EcoCopySetting , PNLE_PARTSSTATUS_ENABLE );		// Ecoコピー設定キー表示.
// Comment 2015/05/06 HCLT-Chandrasekar [HeliosL][CQ512869]  Enable the Keys when Image log over error not occurs.
	  	  	changeDispStatus( PNLE_AdminMenu_AuthFuncCode, PNLE_PARTSSTATUS_ENABLE ); 				// 認証機能有効化設定.
// Comment 2017/11/14【HCLT-Abinav】[IT5 4.2]【AR1032658】Hiding OpenAPI authentication management setting key for Network2.
            if( TYPE_NetworkSpecific_NW2 != e_NetworkSpecific ) {
                changeDispStatus( PNLE_AdminMenu_OAPAttestationManagement, PNLE_PARTSSTATUS_ENABLE );	// OpenAPI認証管理設定 .
            } else {
            	changeDispStatus( PNLE_AdminMenu_OAPAttestationManagement, PNLE_PARTSSTATUS_HIDE );
            }	
			changeDispStatus( PNLE_AdminMenu_LicenseManage, PNLE_PARTSSTATUS_ENABLE ); 				// ライセンス管理設定 .
			}
		}
	} else {
		changeDispStatus( PNLE_AdminMenu_ExtBackupMenu , PNLE_PARTSSTATUS_HIDE );		// 外部メモリーバックアップ.
		changeDispStatus( PNLE_AdminMenu_LicenseManage , PNLE_PARTSSTATUS_HIDE );		// ライセンス管理設定.
//comment 2015/03/25 HCLT-Venkataramesh [HeliosL] [KRQ-101] Authentication function code button  disabled if image log error is there.
		changeDispStatus( PNLE_AdminMenu_AuthFuncCode , PNLE_PARTSSTATUS_HIDE );		// 認証機能有効化設定.
		changeDispStatus( PNLE_AdminMenu_RemoteDiagnosiskSet , PNLE_PARTSSTATUS_HIDE );		// 遠隔診断設定.
		changeDispStatus( PNLE_AdminMenu_VoiceGuide , PNLE_PARTSSTATUS_HIDE );		// 音声ガイド設定.
		changeDispStatus( PNLE_AdminMenu_OAPAttestationManagement , PNLE_PARTSSTATUS_HIDE );	// OpenAPI認証管理設定.
		changeDispStatus( PNLE_AdminMenu_SystemSet , PNLE_PARTSSTATUS_HIDE );		// 環境設定   .
		changeDispStatus( PNLE_AdminMenu_RegistAdministrator , PNLE_PARTSSTATUS_HIDE );		// 管理者/本体登録.
		changeDispStatus( PNLE_AdminMenu_Destination , PNLE_PARTSSTATUS_HIDE );		// 遠宛先/ボックス登録 .
		changeDispStatus( PNLE_AdminMenu_NetworkSet , PNLE_PARTSSTATUS_HIDE );		// ネットワーク設定.
		changeDispStatus( PNLE_AdminMenu_CopySet , PNLE_PARTSSTATUS_HIDE );	// コピー設定.
		changeDispStatus( PNLE_AdminMenu_PrinterSet , PNLE_PARTSSTATUS_HIDE );		// プリンター設定.
		changeDispStatus( PNLE_AdminMenu_FaxSet , PNLE_PARTSSTATUS_HIDE );		// ファクス設定 .
		changeDispStatus( PNLE_AdminMenu_SysContactSet , PNLE_PARTSSTATUS_HIDE );		// システム連携 .
		changeDispStatus( PNLE_AdminMenu_Security , PNLE_PARTSSTATUS_HIDE );		// セキュリティー設定 .
		changeDispStatus( PNLE_AdminMenu_ExNetworkSetting , PNLE_PARTSSTATUS_HIDE );	// 拡張ネットワーク設定 .
		changeDispStatus( PNLE_AdminMenu_ManageUserInfo , PNLE_PARTSSTATUS_ENABLE );	// ユーザー認証/部門管理.
		changeDispStatus( PNLE_AdminMenu_CloudContact , PNLE_PARTSSTATUS_HIDE );		// クラウド連携設定.
		pc_MenuUtilityAndAdminAdd3Page.changeSPCMaxPage( 1 );		// 最大ページ数1.
		Nfd_SPC_UtilityAndAdminMenuTotalPage.showEnable( False );			// 総ページ.
		Nfd_SPC_UtilityAndAdminMenuDspPage.showEnable( False );			// 現ページ.
		Grp_SPC_UtilityAndAdminMenuPage.showEnable( False );				// ページ表示グループ.
		Bak_SPC_UtilityAndAdminMenuPageDsp.showEnable( False );
	}
	if( True == isARPReqWaiting() ) { 	// ARP要求待ち [新アーキ支援パフォーマンス改善]LCDDrawタスクCPU占有率削減対応 管理者画面再描画削減.
		Txt_UtilityMenuMessage2.showEnable( False );
		Txt_AdminSetMenuWaitForARPRequest.showEnable( True );
		Gra_HintMessage1.showEnable( True );
	} else {							// 通常表示.
		Txt_AdminSetMenuWaitForARPRequest.showEnable( False );
		Gra_HintMessage1.showEnable( False );
		Txt_UtilityMenuMessage2.showEnable( True );
	}
	if( True == bl_ImageLogOverError && uc_page == 1 ) {		// 1ページ目 画像ログ上限警告時.
		changeDispStatus( PNLE_AdminMenu_SystemSet , PNLE_PARTSSTATUS_HIDE );		// 環境設定   .
		changeDispStatus( PNLE_AdminMenu_RegistAdministrator , PNLE_PARTSSTATUS_HIDE );		// 管理者/本体登録.
		changeDispStatus( PNLE_AdminMenu_PrinterSet , PNLE_PARTSSTATUS_HIDE );		// プリンター設定.
		onlyPanel.isFaxModekind( &e_isFaxmode );
	}

// 2016/11/07 SCD2-Sky_Shindo【CQ534047】サービスコール発生時に管理者設定からサービスログイン許可設定を「許可」に設定可能とする.
//	if( ( True == onlyPanel.isWarningOn( comPanelJob, PFATAL_ServiceCallFirmwareAbnormal ) ) &&		// Firmware異常警告発生中.
//	    ( True == PnlComGetSystemSetting()->getSecurityLevelUpOn() ) ) {							// セキュリティ強化ON.
	if ( ( (True == onlyPanel.isWarningOn( comPanelJob, PFATAL_ServiceCallFirmwareAbnormal ) ) 	// Firmware異常警告発生中.
	    && (True == PnlComGetSystemSetting()->getSecurityLevelUpOn() ) )							// セキュリティ強化ON.
	  || ( True == isWarningServiceCall() )		// サービスコール発生中.
	) {
		pc_MenuUtilityAndAdminAdd3Page.changeSPCMaxPage( 1 );		// 最大ページ数1.
		Nfd_SPC_UtilityAndAdminMenuTotalPage.showEnable( False );			// 総ページ.
		Nfd_SPC_UtilityAndAdminMenuDspPage.showEnable( False );			// 現ページ.
		Grp_SPC_UtilityAndAdminMenuPage.showEnable( False );				// ページ表示グループ.
		Bak_SPC_UtilityAndAdminMenuPageDsp.showEnable( False );
	}
	pc_MenuUtilityAndAdminAdd3Page.SPCsetupGamen();
}
/*================================================================================================
// 概要		:管理者設定メニュー : ヒントアイコン.
// 戻り値	:なし .
// 備考		:
================================================================================================*/
void PAdminSetMenu::procHintIcon( uchar option )
{
	if( True == isARPReqWaiting() ) {						// ヒントアイコン表示時のみ定期起床での判定を行う [新アーキ支援パフォーマンス改善]LCDDrawタスクCPU占有率削減対応 管理者画面再描画削減.
		PnlComSetupHintMessageIcon( &Gra_HintMessage1 );	// ヒントアイコン表示判定処理.
	}
}
// ==================================================================
// 概要		:管理者設定メニュー : 2ﾍﾟｰｼﾞ目表示内容チェック関数.
// 戻り値	:Bool .
// 備考		: HDD非装着時対応.
// ==================================================================
Bool PAdminSetMenu::checkNextPageContents( )
{
	// Firmware異常警告発生中.
// 2016/11/07 SCD2-Sky_Shindo【CQ534047】サービスコール発生時に管理者設定からサービスログイン許可設定を「許可」に設定可能とする.
//	if( True == onlyPanel.isWarningOn( comPanelJob, PFATAL_ServiceCallFirmwareAbnormal ) ) {
	if ( ( True == onlyPanel.isWarningOn( comPanelJob, PFATAL_ServiceCallFirmwareAbnormal ) )
	  || ( True == isWarningServiceCall() )		// サービスコール発生中.
	) {
		return False;	// 使用不可.
	}

	// 画像ログ上限警告でない.
	if( False == bl_ImageLogOverError ) {
		return True;								// OpenAPI認証管理設定、画面カスタマイズ設定使用可.
	}
	// PKI認証時でない.
	if( False == PLBC_FuncPKICheck::isPKI() ) {
		// HDD非装着でない.
		if( False != c_HD_Equip.isOK() ) {
			return True;							// ライセンス管理設定使用可.
		}
		// ライセンス有効、データインストール済み.
		if( TYPE_CategoryState_ENABLE == PnlComGetEnablerSetting()->getFEK_CotegoryState( TYPE_FEKType09 )
		    && True == PnlComGetMachineSetting()->getVoiceInstallFlgInfo() ) {
			return True;							// 音声ガイド設定使用可.
		}
	}
	// DipSW No.72が0x04(使用する)、且つネットワーク初期化中でない.
	if( True == APIC_SoftDipSw::newInstance()->getSoftDipSwBit( 38, 7 )
	    && False == APIC_NetworkDevice::newInstance()->isNetworkInitialing() ) {
		return True;								// 外部メモリーバックアップ使用可.
	}
	// 初期発信終了もしくは登録完了.
	if( True == isConnectedCSRC() ) {
		return True;								// 遠隔診断設定使用可.
	}

	return False;
}
//==================================================================.
// 概要		:管理者設定メニュー : ARP要求待ち.
// 戻り値	:Bool True/待ち状態.
//							新規作成.
//==================================================================.
Bool PAdminSetMenu::isARPReqWaiting()
{
	APIC_NetworkDevice* pc_NetworkDevice = APIC_NetworkDevice::newInstance();		// ネットワークデバイス・インスタンス取得.
	Bool bl_IsARPOn = PnlComGetTcpIpSetting()->getEnableARP_PING();					// ARP/PING=ONの設定.
	Bool bl_IsNetworkInitializing = pc_NetworkDevice->isNetworkInitialing();		// ネットワークがNot Ready.
	Bool bl_IsAutoIP = False;														// IP設定が自動.
// comment 2017/09/20 【HCL-RamKishor】【IT5_4.2】【RQ686】【WP796】【詳細制御仕様書_管理No.33】 The new API is used and to detect the current network type.
//	if( TYPE_IpAddressAuto == PnlComGetTcpIpSetting()->getIpAddress() ) {
	TYPE_NetworkSpecific e_NetworkSpecific = PNLC_NetworkLan::getInstance()->getAdminNWLine();	
	if( TYPE_IpAddressAuto == PnlComGetTcpIpSetting()->getIpAddress( e_NetworkSpecific ) ) {
		bl_IsAutoIP = True;
	}
	Bool bl_IsWaitingNICTascOver3Min = False;										// 本体電源投入後、NICのタスクが起動=オペパネが起動中画面から切り替わってさらに3分.
	uint32_t ui32_CurTime = PnlGetSecondsPassedSinceMillennium();
	const uint32_t PNLD_InitWaiting = 180;
	uint32_t ui32_ElapsedTime = ui32_CurTime - gp_ActionInitialSetup.getPWOnStartTime();
	if( PNLD_InitWaiting <= ui32_ElapsedTime ) {										// オペパネが起動中画面から切り替わって3分以上経過しているかどうか.
		bl_IsWaitingNICTascOver3Min = True;
	}
// comment 2017/09/20 【HCL-RamKishor】【IT5_4.2】【RQ686】【WP796】【詳細制御仕様書_管理No.33】 The new API is used and to detect the current network type.
	if( TYPE_NetworkSpecific_NW2 == e_NetworkSpecific ){
		bl_IsARPOn = False;
	}
	Bool bl_IsARPReqWaiting = False;												// ARP要求待ちで無い.
	if( True == bl_IsWaitingNICTascOver3Min && True == bl_IsAutoIP
	    && True == bl_IsARPOn && True == bl_IsNetworkInitializing ) {
		bl_IsARPReqWaiting = True;													// ARP要求待ち.
	}
	return( bl_IsARPReqWaiting );
}
//================================================================================================
// 概要		:管理者設定メニューへの遷移共通 : 画面移動
// 戻り値	:なし
//================================================================================================
void PToAdminSetMenucom::procGoToPage( uchar option )
{
	switch( option ) {
		case 10: PToAdminMenucom::procOk( option );		break;	// ユーティリティ設定キー
		case 11: PToAdminSetMenucom::procOk( 0 );		break;	// 管理者設定キー
		default:
			setBuzzerType( PCGE_BuzzerType_InputNg );	break;	// 報知音設定
	}
}
//================================================================================================
// 概要		:管理者設定メニューへの遷移共通 : 設定終了
// 戻り値	:なし
//================================================================================================
void PToAdminSetMenucom::procOk( uchar option )
{
	nextGamen = &gp_AdminSetMenu;	// 管理者設定メニュー
}
//================================================================================================
// 概要		:管理者設定メニューへの遷移共通 : 終了
// 戻り値	:なし
//================================================================================================
void PToAdminSetMenucom::procEnd( uchar option )
{
	PToAdminMenucom::procEnd( option );
}
//================================================================================================
// 概要		:管理者設定・管理者設定メニューへの遷移共通 : 初期値設定.
//================================================================================================
Bool PToAdminSetMenucom::bl_FromMenu = False;		// 上階層からの遷移フラグ.
//================================================================================================.
// 概要		:管理者設定・管理者設定メニューへの遷移共通 : 上階層からの遷移フラグ設定.
//================================================================================================.
void PToAdminSetMenucom::setFromMenu( Bool _bl_FromMenu )
{
	bl_FromMenu = _bl_FromMenu;
}
//================================================================================================.
// 概要		:管理者設定・管理者設定メニューへの遷移共通 : 上階層からの遷移フラグ取得.
//================================================================================================.
Bool PToAdminSetMenucom::getFromMenu()
{
	return bl_FromMenu;
}
//================================================================================================
// 概要		:共通 : ネットワークインタフェースの分類を取得.
// 戻り値	:PNLE_NetworkIFStatus
//================================================================================================
PNLE_NetworkIFStatus PToAdminSetMenucom::getNetworkIFStatus()
{
	APIC_ExNetworkSetting* pc_ExNetworkSetting = APIC_ExNetworkSetting::newInstance();

	if( NULL == pc_ExNetworkSetting ) {
//	comment 2015/11/02 HCL_Ezhil [ZeusS_ZX0][制御構成変更_No.76-1] Adding error code.
//Comment 2019/06/14 [HCLT_Monisha][EagleH][N Rank] Removed unwanted Detailed code.
//		LAPC_BFW_System *pc_BFW_System=LAPC_BFW_System::newInstance();
//		if (NULL !=pc_BFW_System){
//		    pc_BFW_System->registDetailCode(PNLD_DetailCode_Admin_0x1900660010513801);
//		}
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
		return PNLE_NetworkIFStatus_None;
	}
	// 「有線のみ.
	TYPE_NetworkIFMode e_NetworkIFMode = pc_ExNetworkSetting->getNetworkIFMode();
	if( TYPE_NetworkIFMode_SingleEthernet == e_NetworkIFMode ) {		// 有線のみ.
		return PNLE_NetworkIFStatus_OnlyWired;							// 有線のみ.
	}
	// 無線のみ / 有線+無線（子機モード).
	else if( ( TYPE_NetworkIFMode_SingleSta == e_NetworkIFMode ) || ( TYPE_NetworkIFMode_DualEthernetSta == e_NetworkIFMode ) ) {
		return PNLE_NetworkIFStatus_ManualMode;							// 手動モード.
	}
	// 有線+無線（親機モード) / 有線+無線（Wi-Fi Direct).
	else if( ( TYPE_NetworkIFMode_DualEthernetAp == e_NetworkIFMode ) || ( TYPE_NetworkIFMode_DualEthernetWfd == e_NetworkIFMode ) ) {
		return PNLE_NetworkIFStatus_APMode;								// APモード.
	}
	else {
		return PNLE_NetworkIFStatus_None;
	}
}

//================================================================================================
// 概要		:管理者設定・環境設定メニューへの遷移共通 : 画面移動
// 戻り値	:なし
//================================================================================================
void PAdminToSystemSetcom::procGoToPage( uchar option )
{
	switch( option ) {
		case 10:	// ユーティリティ設定キー
		case 11: PToAdminSetMenucom::procGoToPage( option );	break;	// 管理者設定キー
		case 12: PAdminToSystemSetcom::procOk( 0 );				break;	// 管理者設定・環境設定
		default:
			setBuzzerType( PCGE_BuzzerType_InputNg );	break;	// 報知音設定
	}
}
//================================================================================================
// 概要		:管理者設定・環境設定メニューへの遷移共通 : 設定終了
// 戻り値	:なし
//================================================================================================
void PAdminToSystemSetcom::procOk( uchar option )
{
// comment 2022/04/04 Sky西場 [IT6_2.3][OP_BTS-37620][スリム化].
//	nextGamen = &gp_AdminSystemSet;		// 管理者設定・環境設定メニュー
	PToAdminMenucom::procOk( option );
	ExpPanelLog( PNLD_ExpLogKind_AlwaysLogOut, "[DeleteUnnecessaryProcessing] [%s]", __PRETTY_FUNCTION__, 0, 0, 0, 0, 0 );
}
/*================================================================================================
// 概要		:管理者設定・環境設定メニューへの遷移共通 : 管理装置の状態チェック.
// 戻り値	: Bool : True→スタートキー赤.
================================================================================================*/
Bool PAdminToSystemSetcom::isManagementDeviceError()
{
	Bool b_Result = False;
	if( PnlComGetMachineSetting()->getKeyMode() == TYPE_KeyModeAdminMachine ) {	// 従来管理装置接続時.
		if( onlyPanel.isWarningOn( comPanelJob, PERROR_CheckD103PowerSupply	) == True		// D103電源確認.
		    || onlyPanel.isWarningOn( comPanelJob, PNL_ERROR_UnsetIDInMagCard	) == True		// 管理カード（磁気カード）部門未登録.
		    || onlyPanel.isWarningOn( comPanelJob, PNL_ERROR_CardReset	) == True		// 管理カード再セット.
		    || onlyPanel.isWarningOn( comPanelJob, PERROR_MagCardUnset_S	) == True		// 管理カード（磁気カード）未挿入(設定時).
		    || ( PnlComGetTrackManager()->getTrackMode( PNLC_NetworkLan::getInstance()->getAdminNWLine() ) != TYPE_TrackModeOff && bl_BumonLoginAdminMove == False ) ) {	// 部門管理する(複写機側で管理)の時で、部門ログインしていない状態で管理者モードに遷移した場合.(True:部門ログインした False:部門ログインしていない)
			b_Result = True;  // いずれかの警告が発生している.
		}
	} else if( PnlComGetMachineSetting()->getKeyMode() == TYPE_KeyModeAdminMachineKM ) {	// 統合管理装置の場合.
		b_Result = False;
	} else if( PnlComGetMachineSetting()->getKeyMode() == TYPE_KeyModeKeyCounter ) {	// キーカウンタの場合.
		if( onlyPanel.isWarningOn( comPanelJob, PERROR_KeyCounterUnset_S	) == True ) {	// キーカウンター未挿入(設定時).
			b_Result = True;  // いずれかの警告が発生している.
		}
	} else {	// 対象の管理装置がついていない.
		b_Result = False;
	}
	return b_Result;
}


//================================================================================================
// 概要		:管理者設定・環境設定・リスト/カウンタ画面遷移共通：設定終了
// 戻り値	:なし
//================================================================================================
void PAdminSystemListAndCounterCom::procOk( uchar option )
{
// comment 2022/04/04 Sky西場 [IT6_2.3][OP_BTS-37620][スリム化].
//	nextGamen = &gp_AdminSystemListAndCounter;	// リスト/カウンタ(2264)
	PToAdminMenucom::procOk( option );
	ExpPanelLog( PNLD_ExpLogKind_AlwaysLogOut, "[DeleteUnnecessaryProcessing] [%s]", __PRETTY_FUNCTION__, 0, 0, 0, 0, 0 );
}
//================================================================================================
// 概要		:管理者設定・環境設定・管理リストプリント(2265)：コンストラクタ
// 戻り値	:なし
//================================================================================================
PAdminSystemDepartListPrint::PAdminSystemDepartListPrint(
    PDisplayParts** partsTable,
    short partsNumber,
    Bool isNoWarningGamen )
	: PAdminSystemListAndCounterCom( partsTable, partsNumber, isNoWarningGamen )
{
	uc_ListPrintKind = 0;	// リストプリント種類
}
//================================================================================================
// 概要		:管理者設定・環境設定・管理リストプリント(2265)：画面遷移キー押下
// 戻り値	:なし
//================================================================================================
void PAdminSystemDepartListPrint::procGoToPage( uchar option )
{
	switch( option ) {
		case  2: PAdminSystemDepartListPrintListPrint::e_ListPrint = TYPE_ListPrintUserChoice; break;	// 設定値リストキー
		default:
			setBuzzerType( PCGE_BuzzerType_InputNg );	break;	// 報知音設定
	}
	nextGamen = &gp_AdminSystemDepartListPrintListPrint;	// 環境設定・リスト/カウンタ・管理リストプリント・リストプリント(2266)
}
//================================================================================================
// 概要		:管理者設定・環境設定・管理リストプリント(2265)：表示画面のセットアップ
// 戻り値	:なし
//================================================================================================
void PAdminSystemDepartListPrint::setupGamen( short option )
{
	PAdminSystemListAndCounterCom::setupGamen( option );
// Comment 2018/01/29 HCLT_Hanuman [IT5_4.2][AR1038061] To improve performance.
///	Comment 2018/01/03 [HCL-Sudheer]【IT5 4.2】【RQ-686】【WP1382,1384,1386,1388, 1390】【AR1035279】Sharing Common Icon Image for Network 1&Network 2 Settings.
//	if( True == PnlComGetVlanSetting()->getVlanEnable() ) {
	if( True == PNLC_NetworkLan::getInstance()->getVlanEnableWrapper() ) {
#ifdef DEF_RES_WSVGA
		Key_GotoAdminSystemSettingList.changeGraphic( NPK_GO1_VLAN_X307_Y38, 19, False );		// 設定値リストキー.
#else
		Key_GotoAdminSystemSettingList.changeGraphic( NPK_GO1_VLAN_X240_Y30, 19, False );		// 設定値リストキー.
#endif //DEF_RES_WSVGA.
		Txt_TitleAdminSystemDepartListPrintMessage.changeText( s_pMSGListAdminPrintShared_W624H050I );		//メニューキーまたはテンキーで出力するレポートを選択してください\■の設定はネットワーク1/ネットワーク2共通で使用されます.
	}else {
#ifdef DEF_RES_WSVGA
		Key_GotoAdminSystemSettingList.changeGraphic( NPK_GO1_X307_Y38, 19, False );			// 設定値リストキー.
#else
		Key_GotoAdminSystemSettingList.changeGraphic( NPK_GO1_X240_Y30, 19, False );			// 設定値リストキー.
#endif //DEF_RES_WSVGA.
		Txt_TitleAdminSystemDepartListPrintMessage.changeText( s_MSGSelectOutputReportItem );	////メニューキーまたはテンキーで出力するレポートを選択してください.
	}
}

//////////////////////////////////////////////////////////////
//概要    : 管理者設定・環境設定・管理リストプリント(2265)：テンキー押下 .
//備考    :
//////////////////////////////////////////////////////////////
void PAdminSystemDepartListPrint::proc10Key( uchar option )
{
	if( option == 1 ) {
		procGoToPage( (uchar)( option + 1 ) );
	} else {
		// 報知音設定
		setBuzzerType( PCGE_BuzzerType_InputNg );
	}
}
//////////////////////////////////////////////////////////////
//概要    : 管理者設定・環境設定・管理リストプリント(2265): 画面開始処理.
//備考    :
//////////////////////////////////////////////////////////////
void PAdminSystemDepartListPrint::startGamen()
{
	PAdminSystemListAndCounterCom::startGamen();
	// 左エリア種類の有効設定（ジョブ表示（通常））startGamen()の最後でコールしてください.
	setLeftAreaEnableType( TYPD_LeftAreaBookmark | TYPD_LeftAreaSoft10Key );
}

//================================================================================================
// 概要		:管理者設定・環境設定・管理リストプリント(2863)：初期値設定
// 戻り値	:なし
//================================================================================================
TYPE_ListPrint PAdminSystemDepartListPrintListPrint::e_ListPrint = TYPE_ListPrintJobMemory; // 印字するリストの種類.
TYPC_Time  PAdminSystemDepartListPrintListPrint::ct_TimeInspectLogList = NULL;
Bool PAdminSystemDepartListPrintListPrint::bl_isMakingImage;			// 画像生成中.
//================================================================================================
// 概要		:管理者設定・環境設定・管理リストプリント(2863) : コールバックイベント
// 戻り値	:なし
//================================================================================================
void PAdminSystemDepartListPrintListPrint::procCallBack(
    TYPE_PNL_CallBack option
)
{
	PAdminSystemDepartListPrint::procCallBack( option );			// 必ず親を呼ぶ
	if( option == TYPE_PNL_CallBack_functionEnd ) {
		pc_FeedTraySelectListPrint.setSPCPrintBusy( False );
		c_autoRestTimer.c_AutoResetAll.resetTimer( True );
		c_autoRestTimer.c_autoResetOffTimer.resetTimer( True );	// オートリセットOFFタイマー.
	}
	setupGamen();
}
//================================================================================================
// 概要		:管理者設定・環境設定・管理リストプリント(2863) : スタート　
// 戻り値	:なし
//================================================================================================
void PAdminSystemDepartListPrintListPrint::ans_starListPrint( ushort us_Result )
{
	bl_isMakingImage = False;			// 画像生成中でない.
}
void PAdminSystemDepartListPrintListPrint::procStart( uchar option )
{
// Comment 2018/02/01 【HCLT-Venkata】【AR1033484】 Adding admin user details to Network Type.
//// Comment 2017/10/25【HCLT-Narsimhmaswamy】【IT5 4.2】【RQ686】【WP1215】Multiple Network support.
//	TYPE_NetworkSpecific e_NWType =  PNLC_NetworkLan::getInstance() ->getAdminNWLine( );
	TYPE_NetworkSpecific e_NWType = TYPE_NetworkSpecific_NW1;
	APIC_User* pc_User = PnlComGetUser();
	if( True == PNLC_NetworkLan::getInstance()->getVlanEnableWrapper() ) {										// Checking VLAN status.
		if( NULL != pc_User ){
			short ss_AuthorityID = PnlComGetAuthorityID();
			if( TYPD_NotSetting != ss_AuthorityID ) {
				TYPS_SpecialAuthorityLoginInfo s_SpecialAuthorityLoginInfo;
				if( pc_User->getSpecialAuthorityLoginInfo( ss_AuthorityID,&s_SpecialAuthorityLoginInfo) ) {		// Checking whether user is having Admin rights or not.
					e_NWType = s_SpecialAuthorityLoginInfo.e_NWType;											// If login user having admin rights setting the login network type to API.
				}
			}
		}
	}
	if( pc_FeedTraySelectListPrint.checkSPCAnyKeyOn() == False ) { return; }	// プリント中は、受付禁止.
	if( printStartDisable() == False ) {
		// システム状態（ＡＰＩ）・インスタンス取得
		APIC_SystemStatus* pc_SystemStatus = APIC_SystemStatus::newInstance();
		if( pc_SystemStatus->checkJobReserve() == False ) { // ジョブ予約不可のときは何もせずリターン.
			setBuzzerType( PCGE_BuzzerType_InputNg );	// 報知音設定
			return;
		}
		// プリント許可の時
		APIC_ListPrint*	pc_ListPrint = APIC_ListPrint::newInstance();
		// 監査ログリストの場合は日時情報をパラメータに入れる.
		if( PAdminSystemDepartListPrintListPrint::e_ListPrint == TYPE_ListPrintInspectLogTime ) {
// Comment 2017/10/25【HCLT-Narsimhmaswamy】【IT5 4.2】【RQ686】【WP1215】Multiple Network support.
//			pc_ListPrint->req_startListPrint( PAdminSystemDepartListPrintListPrint::e_ListPrint, ( uint32_t )pc_FeedTraySelectListPrint.getSPCFeedTray(), ans_starListPrint, -1, 0, &ct_TimeInspectLogList, pc_FeedTraySelectListPrint.getSPCDuplexOn(), TYPE_JobEntryIF_Panel );
// Comment 2021/05/20 Sky奥田【MinervaSSBK】【OP_BTS-34605】req_startListPrint引数追加.
//			pc_ListPrint->req_startListPrint( PAdminSystemDepartListPrintListPrint::e_ListPrint, ( uint32_t )pc_FeedTraySelectListPrint.getSPCFeedTray(), ans_starListPrint, -1, 0, &ct_TimeInspectLogList, pc_FeedTraySelectListPrint.getSPCDuplexOn(), TYPE_JobEntryIF_Panel, 0, e_NWType );
			pc_ListPrint->req_startListPrint( PAdminSystemDepartListPrintListPrint::e_ListPrint, ( uint32_t )pc_FeedTraySelectListPrint.getSPCFeedTray(), ans_starListPrint, -1, 0, &ct_TimeInspectLogList, pc_FeedTraySelectListPrint.getSPCDuplexOn(), TYPE_JobEntryIF_Panel, 0, e_NWType, sizeof(TYPC_Time)  );
		} else if( PAdminSystemDepartListPrintListPrint::e_ListPrint == TYPE_ListPrintTrackCounter ) {
			if( True ==  PLBC_FuncVersionCheck::isOK_IT5_Type1( ) ) {
				TYPS_CounterOption s_CounterOption;
				s_CounterOption.e__ListPrintInfo = PToAdminSetMenucom::getSelectPrintItem();
				s_CounterOption.b_AdditionalCounterEnable = PToAdminSetMenucom::getSelectCounterDetails();
// Comment 2017/10/25【HCLT-Narsimhmaswamy】【IT5 4.2】【RQ686】【WP1215】Multiple Network support.
//				pc_ListPrint->req_startListPrint( PAdminSystemDepartListPrintListPrint::e_ListPrint, ( uint32_t )pc_FeedTraySelectListPrint.getSPCFeedTray(), ans_starListPrint, -1, 0, &s_CounterOption, pc_FeedTraySelectListPrint.getSPCDuplexOn(), TYPE_JobEntryIF_Panel );	// 使用管理リストの場合は、印刷設定情報をパラメータに入れる
// Comment 2021/05/20 Sky奥田【MinervaSSBK】【OP_BTS-34605】req_startListPrint引数追加.
//				pc_ListPrint->req_startListPrint( PAdminSystemDepartListPrintListPrint::e_ListPrint, ( uint32_t )pc_FeedTraySelectListPrint.getSPCFeedTray(), ans_starListPrint, -1, 0, &s_CounterOption, pc_FeedTraySelectListPrint.getSPCDuplexOn(), TYPE_JobEntryIF_Panel, 0, e_NWType );	// 使用管理リストの場合は、印刷設定情報をパラメータに入れる
				pc_ListPrint->req_startListPrint( PAdminSystemDepartListPrintListPrint::e_ListPrint, ( uint32_t )pc_FeedTraySelectListPrint.getSPCFeedTray(), ans_starListPrint, -1, 0, &s_CounterOption, pc_FeedTraySelectListPrint.getSPCDuplexOn(), TYPE_JobEntryIF_Panel, 0, e_NWType, sizeof(TYPS_CounterOption) );	// 使用管理リストの場合は、印刷設定情報をパラメータに入れる
			} else {
// Comment 2017/10/25【HCLT-Narsimhmaswamy】【IT5 4.2】【RQ686】【WP1215】Multiple Network support.
//				pc_ListPrint->req_startListPrint( PAdminSystemDepartListPrintListPrint::e_ListPrint, ( uint32_t )pc_FeedTraySelectListPrint.getSPCFeedTray(), ans_starListPrint, -1, 0, &e_ListPrintInfo, pc_FeedTraySelectListPrint.getSPCDuplexOn(), TYPE_JobEntryIF_Panel );	// 使用管理リストの場合は、印刷設定情報をパラメータに入れる
// Comment 2021/05/20 Sky奥田【MinervaSSBK】【OP_BTS-34605】req_startListPrint引数追加.
//				pc_ListPrint->req_startListPrint( PAdminSystemDepartListPrintListPrint::e_ListPrint, ( uint32_t )pc_FeedTraySelectListPrint.getSPCFeedTray(), ans_starListPrint, -1, 0, &e_ListPrintInfo, pc_FeedTraySelectListPrint.getSPCDuplexOn(), TYPE_JobEntryIF_Panel, 0, e_NWType );	// 使用管理リストの場合は、印刷設定情報をパラメータに入れる
				pc_ListPrint->req_startListPrint( PAdminSystemDepartListPrintListPrint::e_ListPrint, ( uint32_t )pc_FeedTraySelectListPrint.getSPCFeedTray(), ans_starListPrint, -1, 0, &e_ListPrintInfo, pc_FeedTraySelectListPrint.getSPCDuplexOn(), TYPE_JobEntryIF_Panel, 0, e_NWType, sizeof(TYPE_ListPrintInfo) );	// 使用管理リストの場合は、印刷設定情報をパラメータに入れる
			}
// UIFUM5→MinervaZeusマージ↓.
//		}else if(PAdminSystemDepartListPrintListPrint::e_ListPrint == TYPE_ListPrintSendOperationLog){		// リスト種類が送信操作ログの場合.
		} else if( PAdminSystemDepartListPrintListPrint::e_ListPrint == 72 ) {		// リスト種類が送信操作ログの場合.
			OPLS_SendOpeLogOutOption s_SendOpeLogOutOption;						// 送信操作ログ出力要求データ.
			s_SendOpeLogOutOption.cl_StartTime = cl_StartTime;					// 開始日時.
			s_SendOpeLogOutOption.cl_EndTime = cl_EndTime;						// 終了日時.
			APIC_OPL* pc_OPL = APIC_OPL::newInstance();							// 送信操作ログ出力I/Fクラス生成.
			pc_OPL->req_startSendOpeLogOut( PAdminSystemDepartListPrintListPrint::e_ListPrint, 1, pc_FeedTraySelectListPrint.getFaxPaperTray(), ans_starListPrint, ( void* )&s_SendOpeLogOutOption, pc_FeedTraySelectListPrint.getSPCDuplexOn(), TYPE_JobEntryIF_Panel );	// 送信操作ログ出力要求.
// comment 2016/09/13 SCD2-Sky辻 [特ROM][FQ-OPSD-1600252-OFP174] ベンダーモード時のカウンターリスト出力でEco関連情報が出力される.
		} else if ( True == PLBC_FuncVersionCheck::isOK_IT5_Type1( ) 
					&& TYPE_ListPrintCounter == PAdminSystemDepartListPrintListPrint::e_ListPrint ) {
			TYPS_CounterOption s_CounterOption;
			s_CounterOption.b_AdditionalCounterEnable = bl_isCounterDetails;
// Comment 2017/10/25【HCLT-Narsimhmaswamy】【IT5 4.2】【RQ686】【WP1215】Multiple Network support.
//			pc_ListPrint->req_startListPrint( PAdminSystemDepartListPrintListPrint::e_ListPrint, ( uint32_t )pc_FeedTraySelectListPrint.getSPCFeedTray(), ans_starListPrint, -1, 0, &s_CounterOption, pc_FeedTraySelectListPrint.getSPCDuplexOn(), TYPE_JobEntryIF_Panel );
// Comment 2021/05/20 Sky奥田【MinervaSSBK】【OP_BTS-34605】req_startListPrint引数追加.
//			pc_ListPrint->req_startListPrint( PAdminSystemDepartListPrintListPrint::e_ListPrint, ( uint32_t )pc_FeedTraySelectListPrint.getSPCFeedTray(), ans_starListPrint, -1, 0, &s_CounterOption, pc_FeedTraySelectListPrint.getSPCDuplexOn(), TYPE_JobEntryIF_Panel, 0, e_NWType );
			pc_ListPrint->req_startListPrint( PAdminSystemDepartListPrintListPrint::e_ListPrint, ( uint32_t )pc_FeedTraySelectListPrint.getSPCFeedTray(), ans_starListPrint, -1, 0, &s_CounterOption, pc_FeedTraySelectListPrint.getSPCDuplexOn(), TYPE_JobEntryIF_Panel, 0, e_NWType, sizeof(TYPS_CounterOption) );
		} else {
			if( True == PLBC_FuncVersionCheck::isOK_MiZe_Type2()
			    && TYPE_ListPrintFaxSetting == PAdminSystemDepartListPrintListPrint::e_ListPrint ) {
// Comment 2017/10/25【HCLT-Narsimhmaswamy】【IT5 4.2】【RQ686】【WP1215】Multiple Network support.
//				pc_ListPrint->req_startListPrint( PAdminSystemDepartListPrintListPrint::e_ListPrint, ( uint32_t )pc_FeedTraySelectListPrint.getSPCFeedTray(), ans_starListPrint, -1, 0, ( void* )&m_e_FaxLineNum, pc_FeedTraySelectListPrint.getSPCDuplexOn(), TYPE_JobEntryIF_Panel );
// Comment 2021/05/20 Sky奥田【MinervaSSBK】【OP_BTS-34605】req_startListPrint引数追加.
//				pc_ListPrint->req_startListPrint( PAdminSystemDepartListPrintListPrint::e_ListPrint, ( uint32_t )pc_FeedTraySelectListPrint.getSPCFeedTray(), ans_starListPrint, -1, 0, ( void* )&m_e_FaxLineNum, pc_FeedTraySelectListPrint.getSPCDuplexOn(), TYPE_JobEntryIF_Panel, 0, e_NWType );
				pc_ListPrint->req_startListPrint( PAdminSystemDepartListPrintListPrint::e_ListPrint, ( uint32_t )pc_FeedTraySelectListPrint.getSPCFeedTray(), ans_starListPrint, -1, 0, ( void* )&m_e_FaxLineNum, pc_FeedTraySelectListPrint.getSPCDuplexOn(), TYPE_JobEntryIF_Panel, 0, e_NWType, sizeof(TYPE_CommLine) );
			} else {
// Comment 2017/10/25【HCLT-Narsimhmaswamy】【IT5 4.2】【RQ686】【WP1215】Multiple Network support.
//				pc_ListPrint->req_startListPrint( PAdminSystemDepartListPrintListPrint::e_ListPrint, ( uint32_t )pc_FeedTraySelectListPrint.getSPCFeedTray(), ans_starListPrint, -1, 0, NULL, pc_FeedTraySelectListPrint.getSPCDuplexOn(), TYPE_JobEntryIF_Panel );
				pc_ListPrint->req_startListPrint( PAdminSystemDepartListPrintListPrint::e_ListPrint, ( uint32_t )pc_FeedTraySelectListPrint.getSPCFeedTray(), ans_starListPrint, -1, 0, NULL, pc_FeedTraySelectListPrint.getSPCDuplexOn(), TYPE_JobEntryIF_Panel, 0, e_NWType );
			}
		}
		bl_isMakingImage = True;			// 画像生成中.
		pc_FeedTraySelectListPrint.setSPCPrintBusy( True );	// プリント中フラグ
		setupGamen();
	} else {
		setBuzzerType( PCGE_BuzzerType_InputNg );	// 報知音設定
	}
}
//================================================================================================
// 概要		:管理者設定・環境設定・管理リストプリント(2863) : 送信操作ログ出力範囲設定.
// 戻り値	:なし
//================================================================================================
void PAdminSystemDepartListPrintListPrint::setSendOpeLogRange( TYPC_Time cl_StartDate, TYPC_Time cl_EndDate )
{
	cl_StartTime = cl_StartDate;												// 送信操作ログ取得範囲（開始）セット.
	cl_EndTime = cl_EndDate;													// 送信操作ログ取得範囲（終了）セット.
}
//================================================================================================
// 概要		:管理者設定・環境設定・管理リストプリント(2863) : リセットキー
// 戻り値	:なし
//================================================================================================
void PAdminSystemDepartListPrintListPrint::procReset(
    uchar option )
{
	if( pc_FeedTraySelectListPrint.checkSPCAnyKeyOn() == False ) { return; }	// プリント中は、受付禁止.
	PAdmincom::procReset( option );
}
//================================================================================================
// 概要		:管理者設定・環境設定・管理リストプリント(2863) : ユーティリティ
// 戻り値	:なし
//================================================================================================
void PAdminSystemDepartListPrintListPrint::procUtility(
    uchar option )
{
	if( pc_FeedTraySelectListPrint.checkSPCAnyKeyOn() == False ) { return; }	// プリント中は、受付禁止.
	PAdmincom::procUtility( option );
}
//================================================================================================
// 概要		:管理者設定・環境設定・管理リストプリント(2863) : コピーキー
// 戻り値	:なし
//================================================================================================
void PAdminSystemDepartListPrintListPrint::procCopy(
    uchar option )
{
	if( pc_FeedTraySelectListPrint.checkSPCAnyKeyOn() == False ) { return; }	// プリント中は、受付禁止.
	PAdmincom::procCopy( option );
}
//================================================================================================
// 概要		:管理者設定・環境設定・管理リストプリント(2863) : FAX
// 戻り値	:なし
//================================================================================================
void PAdminSystemDepartListPrintListPrint::procFax(
    uchar option )
{
	if( pc_FeedTraySelectListPrint.checkSPCAnyKeyOn() == False ) { return; }	// プリント中は、受付禁止.
	PAdmincom::procFax( option );
}
//================================================================================================
// 概要		:管理者設定・環境設定・管理リストプリント(2863) : FAX
// 戻り値	:なし
//================================================================================================
void PAdminSystemDepartListPrintListPrint::procFaxOnly(
    uchar option )
{
	if( pc_FeedTraySelectListPrint.checkSPCAnyKeyOn() == False ) { return; }	// プリント中は、受付禁止.
	PAdmincom::procFaxOnly( option );
}

//================================================================================================
// 概要		:管理者設定・環境設定・管理リストプリント(2863) : ボックス
// 戻り値	:なし
//================================================================================================
void PAdminSystemDepartListPrintListPrint::procBox(
    uchar option )
{
	if( pc_FeedTraySelectListPrint.checkSPCAnyKeyOn() == False ) { return; }	// プリント中は、受付禁止.
	PAdmincom::procBox( option );
}
//================================================================================================
// 概要		:管理者設定・環境設定・管理リストプリント(2863) : ユビキタスプリント.
// 戻り値	:なし.
//================================================================================================
void PAdminSystemDepartListPrintListPrint::procUbqPrint(
    uchar option )
{
	if( pc_FeedTraySelectListPrint.checkSPCAnyKeyOn() == False ) { return; }	// プリント中は、受付禁止.
	PAdmincom::procUbqPrint( option );
}
//================================================================================================
// 概要		:管理者設定・環境設定・管理リストプリント(2863) : パワーセーブ
// 戻り値	:なし
//================================================================================================
void PAdminSystemDepartListPrintListPrint::procPowerSave(
    uchar option )
{
	if( pc_FeedTraySelectListPrint.checkSPCAnyKeyOn() == False ) { return; }	// プリント中は、受付禁止.
	PAdmincom::procPowerSave( option );
}
//================================================================================================
// 概要		:管理者設定・環境設定・管理リストプリント(2863) : オートリセット.
// 戻り値	:なし
//================================================================================================
void PAdminSystemDepartListPrintListPrint::procAutoResetReq(
    uchar option )
{
	if( pc_FeedTraySelectListPrint.getSPCPrintBusy() == True ) { return; }	// プリント中は、受付禁止
	PAdmincom::procAutoResetReq( option );
}
//================================================================================================
// 概要		:管理者設定・環境設定・管理リストプリント(2863) : ユーザー補助
// 戻り値	:なし
//================================================================================================
void PAdminSystemDepartListPrintListPrint::procAccessibility(
    uchar option )
{
	if( pc_FeedTraySelectListPrint.checkSPCAnyKeyOn() == False ) { return; }	// プリント中は、受付禁止.
	PAdmincom::procAccessibility( option );
}
//================================================================================================
// 概要		:管理者設定・環境設定・管理リストプリント(2863) : ボタン（押された）
// 戻り値	:なし
//================================================================================================
void PAdminSystemDepartListPrintListPrint::procButtonOn(
    uchar option )
{
	if( pc_FeedTraySelectListPrint.checkSPCAnyKeyOn() == False ) { return; }	// プリント中は、受付禁止.
	if( PAdminSystemDepartListPrintListPrint::e_ListPrint == TYPE_ListPrintTrackCounter ) {
		switch( option ) {
			case 22:
				e_ListPrintInfo = TYPE_ListPrintInfoAll;
				break;
			case 23:
				e_ListPrintInfo = TYPE_ListPrintInfoOnly;
				break;
			default:
				break;
		}
	}
	
// comment 2016/09/13 SCD2-Sky辻 [特ROM][FQ-OPSD-1600252-OFP174] ベンダーモード時のカウンターリスト出力でEco関連情報が出力される.
	else if ( TYPE_ListPrintCounter == PAdminSystemDepartListPrintListPrint::e_ListPrint ) {
		switch( option ) {
			case PNLD_BTNID_UTILTOTALCOUNTERPRINT_DONOTPRINT:
				bl_isCounterDetails = False;
				break;
			case PNLD_BTNID_UTILTOTALCOUNTERPRINT_PRINT:
				bl_isCounterDetails = True;
				break;
			default:
				break;
		}	
	}
	if( option != 22 && option != 23 ) {
		pc_FeedTraySelectListPrint.SPCprocButtonOn( option );		// ボタン押下処理.
	}
	setupGamen();
}
//================================================================================================
// 概要		:管理者設定・環境設定・管理リストプリント(2265) : ＯＫ
// 戻り値	:なし
//================================================================================================
void PAdminSystemDepartListPrintListPrint::procOk(
    uchar option )
{
	if( pc_FeedTraySelectListPrint.checkSPCAnyKeyOn() == False ) { return; }	// プリント中は、受付禁止.
	// リスト種類に応じた遷移先の変更.
	switch( PAdminSystemDepartListPrintListPrint::e_ListPrint ) {
		case  TYPE_ListPrintJobMemory:	// 設定メモリリストキー
		case  TYPE_ListPrintUserChoice:	// 設定値リストキー
			nextGamen = &gp_AdminSystemDepartListPrint;	// 管理者設定・環境設定・管理リストプリント.
			break;
		// 2015/07/02 SecV1-5 【管理者によるサービスモード制御技術】 Sky 新道.
		case  TYPE_ListPrintAdminPasswordReleaseKey:
// comment 2022/08/30 Sky西場 [IT6_2.3][OP_BTS-37620][スリム化].
			ExpPanelLog( PNLD_ExpLogKind_AlwaysLogOut, "[DeleteUnnecessaryProcessing] [%s]", __PRETTY_FUNCTION__, 0, 0, 0, 0, 0 );
//			nextGamen = &gp_AdminPasswordReleaseKeyConf;	// 管理者パスワード解除キー確認.
			break;
		case  TYPE_ListPrintTrackCounter:		nextGamen = &gp_AdminUserAndDepartSet;				break;	// 管理者設定・ユーザ認証/部門管理設定メニュー.
// comment 2022/07/20 Sky西場 [IT6_2.3][OP_BTS-37620][スリム化].
//		case TYPE_ListPrintFaxSetting:			nextGamen = &gp_AdminFaxSetting;					break;	// 管理者設定・ユーザ認証/部門管理設定メニュー.
		case TYPE_ListPrintFaxSetting:																		// 管理者設定・ユーザ認証/部門管理設定メニュー.
			ExpPanelLog( PNLD_ExpLogKind_AlwaysLogOut, "[DeleteUnnecessaryProcessing] [%s]", __PRETTY_FUNCTION__, 0, 0, 0, 0, 0 );
			break;
		case TYPE_ListPrintInspectLogTime:		nextGamen = &gp_AdminSecurityInspectLogList;		break;	// 管理者設定・セキュリティ設定・監査ログリスト.

// comment 2022/04/04 Sky西場 [IT6_2.3][OP_BTS-37620][スリム化].
//		case TYPE_ListPrintCounter:		// セールスカウンタリスト.
//		case TYPE_ListPrintUnitCheck:			nextGamen = &gp_AdminSystemListAndCounter;			break;	// 管理者設定・環境設定・リスト/カウンタ.
		case TYPE_ListPrintCounter:		// セールスカウンタリスト.
		case TYPE_ListPrintUnitCheck:
			PToAdminMenucom::procOk( option );
			ExpPanelLog( PNLD_ExpLogKind_AlwaysLogOut, "[DeleteUnnecessaryProcessing] [%s]", __PRETTY_FUNCTION__, 0, 0, 0, 0, 0 );
			break;
// comment 2022/09/06 Sky西場 [IT6_2.3][OP_BTS-37620][スリム化].
//		case TYPE_ListPrintFEKDLKRequestCode:	nextGamen = &gp_AdminLicenseRequestCodePublication; break;	// 管理者設定・ライセンス管理設定・リクエストコード.
		case TYPE_ListPrintFEKDLKRequestCode:	// 管理者設定・ライセンス管理設定・リクエストコード.
			ExpPanelLog( PNLD_ExpLogKind_AlwaysLogOut, "[DeleteUnnecessaryProcessing] [%s]", __PRETTY_FUNCTION__, 0, 0, 0, 0, 0 );
			break;
		default:
			setBuzzerType( PCGE_BuzzerType_InputNg );	// 報知音設定
			break;
	}
}
//================================================================================================
// 概要		:管理者設定・環境設定・管理リストプリント(2863) : 画面開始処理
// 戻り値	:なし
//================================================================================================
void PAdminSystemDepartListPrintListPrint::startGamen()
{
	PUtilityToMenuCom::startGamen();
	bl_isMakingImage = False;			// 画像生成中.
// comment 2016/09/13 SCD2-Sky辻 [特ROM][FQ-OPSD-1600252-OFP174] ベンダーモード時のカウンターリスト出力でEco関連情報が出力される.
	bl_isCounterDetails = False;
	// リスト種類に応じたメッセージの変更.
	switch( PAdminSystemDepartListPrintListPrint::e_ListPrint ) {
		case  TYPE_ListPrintJobMemory: Txt_TitleAdminListPrintListPrint.changeText( s_AdminSystemSetMemoryList ); break;	// 設定メモリリストキー
		case  TYPE_ListPrintFaxSetting: // ファクス設定リスト.
// comment 2022/07/20 Sky西場 [IT6_2.3][OP_BTS-37620][スリム化].
			ExpPanelLog( PNLD_ExpLogKind_AlwaysLogOut, "[DeleteUnnecessaryProcessing] [%s]", __PRETTY_FUNCTION__, 0, 0, 0, 0, 0 );
//			// *** タイトル変更 ***
//			Txt_TitleAdminListPrintListPrint.changeText( s_TTLFaxSettingList_1 );
//			// *** しおり変更 ***
//			// ファクス設定
//			BookMark_UtilityAdminCom.changeStringData( 2, s_BKMBookMarkFaxSetting ) ;
//			BookMark_UtilityAdminCom.changeNextGamen( 2, &gp_AdminFaxSetting, PNL_GOTOKIND_ADMIN ) ;
//			// 設定値リスト
//			BookMark_UtilityAdminCom.changeStringData( 3, s_BKMFaxModeList ) ;
//			BookMark_UtilityAdminCom.changeNextGamen( 3, NULL, PNL_GOTOKIND_ADMIN ) ;
//			// ５段目無し
//			BookMark_UtilityAdminCom.changeStringData( 4, NULL ) ;
//			BookMark_UtilityAdminCom.changeNextGamen( 4, NULL, PNL_GOTOKIND_ADMIN ) ;
//			m_e_FaxLineNum = PToAdminSetMenucom::getFaxSettingFaxLineNumber();
			break ;
		case  TYPE_ListPrintUserChoice: 	Txt_TitleAdminListPrintListPrint.changeText( s_TTLAdministratorSettingPrintMachineManagementListModeList );								break;	// 設定値リストキー
		case  TYPE_ListPrintTrackCounter:	// 使用管理カウンタリストキー
			Txt_TitleAdminListPrintListPrint.changeText( s_CHTAdministratorSettingUserAuthenticationGroupManagementUseManagementCounterListPrint );
			e_ListPrintInfo = TYPE_ListPrintInfoAll;	// 印刷情報の初期設定.
			break;
		case TYPE_ListPrintCounter: // セールスカウンタリスト.
			// *** タイトル変更 ***
			Txt_TitleAdminListPrintListPrint.changeText( s_TTLAdministratorSettingListCounterSalesCounterList );
// comment 2022/04/04 Sky西場 [IT6_2.3][OP_BTS-37620][スリム化].
//			// *** しおり変更 ***
//			// 環境設定
//			BookMark_UtilityAdminCom.changeStringData( 2, s_BKMSystemSetting ) ;			// 環境設定
//			BookMark_UtilityAdminCom.changeNextGamen( 2, &gp_AdminSystemSet, PNL_GOTOKIND_ADMIN ) ;
//			// リスト／カウンター
//			BookMark_UtilityAdminCom.changeStringData( 3, s_BKMListCounter ) ;				// リスト／カウンター
//			BookMark_UtilityAdminCom.changeNextGamen( 3, &gp_AdminSystemListAndCounter, PNL_GOTOKIND_ADMIN ) ;
//			// セールスカウンターリスト
//			BookMark_UtilityAdminCom.changeStringData( 4, s_BKMSalesCounterList ) ;			// セールス\カウンターリスト
//			BookMark_UtilityAdminCom.changeNextGamen( 4, NULL, PNL_GOTOKIND_ADMIN ) ;
			ExpPanelLog( PNLD_ExpLogKind_AlwaysLogOut, "[DeleteUnnecessaryProcessing] [%s]", __PRETTY_FUNCTION__, 0, 0, 0, 0, 0 );
			break;
		case TYPE_ListPrintUnitCheck: // 消耗品確認リスト.
			// *** タイトル変更 ***
			Txt_TitleAdminListPrintListPrint.changeText( s_TTLAdministratorSettingListCounterCheckConsumablesList );
// comment 2022/04/04 Sky西場 [IT6_2.3][OP_BTS-37620][スリム化].
//			// *** しおり変更 ***
//			// 環境設定
//			BookMark_UtilityAdminCom.changeStringData( 2, s_BKMSystemSetting ) ;			// 環境設定
//			BookMark_UtilityAdminCom.changeNextGamen( 2, &gp_AdminSystemSet, PNL_GOTOKIND_ADMIN ) ;
//			// リスト／カウンター
//			BookMark_UtilityAdminCom.changeStringData( 3, s_BKMListCounter ) ;				// リスト／カウンター
//			BookMark_UtilityAdminCom.changeNextGamen( 3, &gp_AdminSystemListAndCounter, PNL_GOTOKIND_ADMIN ) ;
//			// 消耗品確認リスト
//			BookMark_UtilityAdminCom.changeStringData( 4, s_BKMCheckConsumablesList ) ;			// 消耗品確認リスト
//			BookMark_UtilityAdminCom.changeNextGamen( 4, NULL, PNL_GOTOKIND_ADMIN ) ;
			ExpPanelLog( PNLD_ExpLogKind_AlwaysLogOut, "[DeleteUnnecessaryProcessing] [%s]", __PRETTY_FUNCTION__, 0, 0, 0, 0, 0 );
			break;
		case  TYPE_ListPrintFEKDLKRequestCode:	Txt_TitleAdminListPrintListPrint.changeText( s_TTLAdminRequestCodePublicRequestCodePrint );		break ; // リクエストコード印刷.	// *** タイトル変更 ***
		// 2015/07/02 SecV1-5 【管理者によるサービスモード制御技術】 Sky 新道.
		case  TYPE_ListPrintAdminPasswordReleaseKey:										// 管理者パスワード解除キー.
			{
// comment 2022/08/30 Sky西場 [IT6_2.3][OP_BTS-37620][スリム化].
				ExpPanelLog( PNLD_ExpLogKind_AlwaysLogOut, "[DeleteUnnecessaryProcessing] [%s]", __PRETTY_FUNCTION__, 0, 0, 0, 0, 0 );
//				SP_AdminPasswordReleaseKeyPrintBookMarkTblData	bookmarkTbl[PNLD_AdminPasswordReleaseKeyTotalBookmark] = 
//				{				//管理者パスワード解除キーの表示テキスト＆しおり定義テーブル.
//					{
//						 s_pTTLAdminSecuirtySettingsReleaseKeyPrint_W608H016I,
//						 {
//							 { s_BKMSettingMenuBookMark, 	&gp_UtilityMenu, 					PNL_GOTOKIND_UTILITY  }, 
//							 { s_BKMAdminSettingBookMark, 	&gp_AdminSetMenu, 					PNL_GOTOKIND_ADMIN    }, 
//							 { s_pBKMEnableServiceLogin_W128H032I, 	&gp_AdminEnableServiceLogin, 		PNL_GOTOKIND_ADMIN    }, 
//							 { s_pBKMReleaseKey_W128H032I, 			&gp_AdminPasswordReleaseKeyConf, 	PNL_GOTOKIND_ADMIN    }, 
//							 { s_pBKMReleaseKeyPrint_W128H032I, 		NULL, 								PNL_GOTOKIND_UNDEFINED} 
//						 }
//					},
//					{
//						 s_pTTLUtilityReleaseKeyPrint_W608H016I, 
//						 {
//							{ s_BKMSettingMenuBookMark, 	&gp_UtilityMenu, 					PNL_GOTOKIND_UTILITY   }, 
//							{ s_pBKMReleaseKey_W128H032I, 				&gp_AdminPasswordReleaseKeyConf, 	PNL_GOTOKIND_ADMIN     }, 
//							{ s_pBKMReleaseKeyPrint_W128H032I, 		NULL, 								PNL_GOTOKIND_UNDEFINED }, 
//							{ NULL, 						NULL, 								PNL_GOTOKIND_UNDEFINED }, 
//							{ NULL, 						NULL, 								PNL_GOTOKIND_UNDEFINED } 
//						 }
//					},
//					{
//						 s_pTTLAdminSecuirtySettingsReleaseKeyPrint_W608H016I, 
//						 {
//							{ s_BKMSettingMenuBookMark, 	&gp_UtilityMenu, 					PNL_GOTOKIND_UTILITY  }, 
//							{ s_BKMAdminSettingBookMark, 	&gp_AdminSetMenu, 					PNL_GOTOKIND_ADMIN    }, 
//							{ s_BMKSimpleSecuritySettings, 	&gp_AdminSimpleSecuritySetting, 	PNL_GOTOKIND_ADMIN    }, 
//							{ s_pBKMReleaseKey_W128H032I, 				&gp_AdminPasswordReleaseKeyConf, 	PNL_GOTOKIND_ADMIN    }, 
//							{ s_pBKMReleaseKeyPrint_W128H032I, 		NULL, 								PNL_GOTOKIND_UNDEFINED} 
//						 }
//					},
//					{ 
//						s_pTTLAdminSecuirtySettingsReleaseKeyPrint_W608H016I, 
//						 {
//							{ s_BKMSettingMenuBookMark, 	&gp_UtilityMenu, 					PNL_GOTOKIND_UTILITY  }, 
//							{ s_BKMAdminSettingBookMark, 	&gp_AdminSetMenu, 					PNL_GOTOKIND_ADMIN    }, 
//							{ s_BKMSecuritySetting, 		&gp_AdminSecuritySet, 				PNL_GOTOKIND_ADMIN    }, 
//							{ s_pBKMReleaseKey_W128H032I, 				&gp_AdminPasswordReleaseKeyConf, 	PNL_GOTOKIND_ADMIN    }, 
//							{ s_pBKMReleaseKeyPrint_W128H032I, 		NULL, 								PNL_GOTOKIND_UNDEFINED} 
//						 }
//					}
//				};
//				
//				Txt_TitleAdminListPrintListPrint.changeText( bookmarkTbl[m_uc_DispTransition].titleText );		// 表示タイトル.
//				for( uchar i = 1; PNL_DEF_UTILITYADMIN_BOOKMARM_NUM > i ; i++){
//					BookMark_UtilityAdminCom.changeStringData(i, bookmarkTbl[m_uc_DispTransition].bookmark[i].dispString) ;															// しおり表示文字.
//					BookMark_UtilityAdminCom.changeNextGamen(i, bookmarkTbl[m_uc_DispTransition].bookmark[i].gotoGamen, bookmarkTbl[m_uc_DispTransition].bookmark[i].gotoGamenKind) ;	// しおり情報.
//				}
				
				break;
			}
		case 72: // 送信操作ログ.
			// *** タイトル変更 ***.
			Txt_TitleAdminListPrintListPrint.changeText( s_TTLAdminSettingSendOpeLogOutPrintSetting );	// 管理者設定 > 送信操作ログ出力 > 印刷設定.
// comment 2022/04/04 Sky西場 [IT6_2.3][OP_BTS-37620][スリム化].
//			// *** しおり変更 ***.
//			// 送信操作ログ出力.
//			BookMark_UtilityAdminCom.changeStringData( 3, s_BKMAdminSendOpeLogOut ) ;						// 送信操作ログ出力.
//			BookMark_UtilityAdminCom.changeNextGamen( 3, &gp_AdminSystemSendOpeLogOutSetting, PNL_GOTOKIND_ADMIN ) ;	// 送信操作ログ出力設定画面.
//			// 印刷設定.
//			BookMark_UtilityAdminCom.changeStringData( 4, s_BKMAdminSettingPrintSetting ) ;				// 印刷設定.
//			BookMark_UtilityAdminCom.changeNextGamen( 4, NULL, PNL_GOTOKIND_ADMIN ) ;
			ExpPanelLog( PNLD_ExpLogKind_AlwaysLogOut, "[DeleteUnnecessaryProcessing] [%s]", __PRETTY_FUNCTION__, 0, 0, 0, 0, 0 );
			break;
		case TYPE_ListPrintNetworkConfig:			// ネットワーク設定値リスト.
			if( PNLE_AdminFromNetworkSetting == uc_FromNetworkMode ) {		// ネットワーク設定画面からの遷移.
				// *** タイトル変更 ***.
				Txt_TitleAdminListPrintListPrint.changeText( s_TTLAdministratorSettingNetworkSettingNetworkSettingList );			// 管理者設定 > ネットワーク設定 > ネットワーク設定値リスト.
			} else {														// 拡張ネットワーク設定画面からの遷移.
				// *** タイトル変更 ***.
				Txt_TitleAdminListPrintListPrint.changeText( s_TTLAdministratorSettingExtendNetworkSettingNetworkSettingList );		// 管理者設定 > 拡張ネットワーク設定 > ネットワーク設定値リスト.
			}
			break;
		default: break;
	}
	pc_FeedTraySelectListPrint.initSPC( PNLE_FeedTrayPatternListPrint );
	// 左エリア種類の有効設定（ジョブ表示（通常））startGamen()の最後でコールしてください.
	setLeftAreaEnableType( TYPD_LeftAreaBookmark | TYPD_LeftAreaSoft10Key );
}
//================================================================================================.
// 概要		:管理者設定・環境設定・管理リストプリント(2863) : 表示画面のセットアップ.
// 戻り値	:なし.
//================================================================================================.
void PAdminSystemDepartListPrintListPrint::setupGamen( short option	)
{
	PNLE_IsFaxMode 	e_isFaxmode = PNLE_IsFaxMode_NON;					// FAX接続種別. //comment 20120918 FT梅岡[ZeusBK][UV-00877_43]Fax3or4回線対応(基板歯抜け対応).

	PUtilityToMenuCom::setupGamen( option );
	APIC_ActiveJobList* pc_ActiveJobList = APIC_ActiveJobList::newInstance();
	APIC_Job* pc_currentJob = pc_ActiveJobList->getCurrentPrintJob();
	// プリント完了を監視.
	if( pc_FeedTraySelectListPrint.getSPCPrintBusy() == True			// ボタン受付禁止中.
	    && bl_isMakingImage == False	// 画像生成中ではない.
	    && pc_currentJob == NULL ) {	// 実行中のプリントジョブが無い.
		CallBackIndi( TYPE_PNL_CallBack_functionEnd );		// コールバックイベント発生
	}
	// スタートキーＬＥＤの制御
	if( PnlComGetMachineSetting()->getKeyMode() == TYPE_KeyModeAdminMachine	// 従来管理装置.
	    && PnlComGetTrackManager()->getTrackMode( PNLC_NetworkLan::getInstance()->getAdminNWLine() ) != TYPE_TrackModeOff  		 // 部門管理する.
	    && bl_BumonLoginAdminMove == False ) {		// 部門ログインしていない状態で管理者モードに遷移した場合.(True:部門ログインした False:部門ログインしていない)
		// 管理者モード中はカード再セット警告を検知しないので、再セット警告中の判断をするために、bl_BumonLoginAdminMove のフラグを見て判断する.(管理者モードのみの条件)
		printLEDControl( False );	// スタートキーＬＥＤの制御
	} else {
		printLEDControl( pc_FeedTraySelectListPrint.checkSPCStartLED() );
	}
	pc_FeedTraySelectListPrint.SPCsetupGamen();
	if( PAdminSystemDepartListPrintListPrint::e_ListPrint == TYPE_ListPrintTrackCounter ) {
		Bak_85X640Y104W296H320.showEnable( False );									// ユーティリティグルーピング_タイトル(小).
		Key_AdminFaxSettingListLineSelect.showEnable( False );
		Txt_AdminFaxSettingListLineSelect.showEnable( False );
		Bak_UtilityListPrintSide.showEnable( True );							// ユーティリティグルーピング_タイトル(極小)-10920600.
		if( True ==  PLBC_FuncVersionCheck::isOK_IT5_Type1( ) ) {
			Key_UtilityListPrintPrintItem.showEnable( True );
			Bak_UtilityListPrintEntry.showEnable( False );
			Txt_UtilitySetPrintEntry.showEnable( False );
			Rbt_UtilityListPrintEntry.showEnable( False );
		} else {
			Key_UtilityListPrintPrintItem.showEnable( False );
			Bak_UtilityListPrintEntry.showEnable( True );							// ユーティリティグルーピング_タイトル(極小)-10920600.
			Txt_UtilitySetPrintEntry.showEnable( True );
			Rbt_UtilityListPrintEntry.showEnable( True );							// 印刷項目の初期値設定
			Rbt_UtilityListPrintEntry.select( e_ListPrintInfo );							// 印刷項目の初期値設定
		}
// comment 2016/09/13 SCD2-Sky辻 [特ROM][FQ-OPSD-1600252-OFP174] ベンダーモード時のカウンターリスト出力でEco関連情報が出力される.
		Bak_UtilityTotalCounterPrintCounterDetails.showEnable( False );
		Txt_UtilityTotalCounterPrintCounterDetails.showEnable( False );
		Rbt_UtilityTotalCounterPrintCounterDetails.showEnable( False );
	} else if( True == PLBC_FuncVersionCheck::isOK_MiZe_Type2()
	           && TYPE_ListPrintFaxSetting == PAdminSystemDepartListPrintListPrint::e_ListPrint ) {
		Key_UtilityListPrintPrintItem.showEnable( False );
		onlyPanel.isFaxModekind( &e_isFaxmode );
		// 複数回線接続時、回線選択キーを表示.
		if( PNLE_IsFaxMode_MUTIFAX == e_isFaxmode ) {
			Bak_85X640Y104W296H320.showEnable( False );
			Txt_UtilitySetPrintEntry.showEnable( False );									//  「印刷項目」.
			Rbt_UtilityListPrintEntry.showEnable( False );								// 10920600_Do 使用管理カウンターリストプリントで使用されるグループ.
			Bak_UtilityListPrintSide.showEnable( True );									// ユーティリティグルーピング_タイトル(極小)-10920600.
			Bak_UtilityListPrintEntry.showEnable( True );								// ユーティリティグルーピング_タイトル(極小)-10920600.
			Key_AdminFaxSettingListLineSelect.showEnable( True );
			Txt_AdminFaxSettingListLineSelect.showEnable( True );
			if( ( char )TYPE_CommLineAll == ( char )PToAdminSetMenucom::getFaxSettingFaxLineNumber() ) {
				Txt_AdminSetFaxSettingListLineSelect.changeText( s_KEYAllFaxLine );// 全回線.
			} else {
				switch( PToAdminSetMenucom::getFaxSettingFaxLineNumber() ) {
					case TYPE_CommLine1:	Txt_AdminSetFaxSettingListLineSelect.changeText( s_FaxLine1 ); break;	// 回線1.
					case TYPE_CommLine2:	Txt_AdminSetFaxSettingListLineSelect.changeText( s_FaxLine2 ); break;	// 回線2.
					case TYPE_CommLine3:	Txt_AdminSetFaxSettingListLineSelect.changeText( s_FaxLine3 ); break;	// 回線3.
					case TYPE_CommLine4:	Txt_AdminSetFaxSettingListLineSelect.changeText( s_FaxLine4 ); break;	// 回線4.
					default: break;
				}
			}
		} else {
			Bak_UtilityListPrintSide.showEnable( False );									// ユーティリティグルーピング_タイトル(極小)-10920600.
			Bak_UtilityListPrintEntry.showEnable( False );								// ユーティリティグルーピング_タイトル(極小)-10920600.
			Txt_UtilitySetPrintEntry.showEnable( False );									//  「印刷項目」.
			Rbt_UtilityListPrintEntry.showEnable( False );								// 10920600_Do 使用管理カウンターリストプリントで使用されるグループ.
			Key_AdminFaxSettingListLineSelect.showEnable( False );
			Txt_AdminFaxSettingListLineSelect.showEnable( False );
			Bak_85X640Y104W296H320.showEnable( True );
		}
// comment 2016/09/13 SCD2-Sky辻 [特ROM][FQ-OPSD-1600252-OFP174] ベンダーモード時のカウンターリスト出力でEco関連情報が出力される.
		Bak_UtilityTotalCounterPrintCounterDetails.showEnable( False );
		Txt_UtilityTotalCounterPrintCounterDetails.showEnable( False );
		Rbt_UtilityTotalCounterPrintCounterDetails.showEnable( False );
	} else if ( True == PLBC_FuncVersionCheck::isOK_IT5_Type1( ) && TYPE_ListPrintCounter == PAdminSystemDepartListPrintListPrint::e_ListPrint ) {
		Bak_UtilityTotalCounterPrintCounterDetails.showEnable( True );				//「詳細カウンター」背景
		Txt_UtilityTotalCounterPrintCounterDetails.showEnable( True );				//「詳細カウンター」テキスト
		Rbt_UtilityTotalCounterPrintCounterDetails.showEnable( True );				//「詳細カウンター:印字する/印字しない」キー
		if( False == bl_isCounterDetails ) {
			Rbt_UtilityTotalCounterPrintCounterDetails.select( 0 );
		} else {
			Rbt_UtilityTotalCounterPrintCounterDetails.select( 1 );
		}
		Bak_85X640Y104W296H320.showEnable( False );									//「詳細カウンター」非表示時の｢片面/両面｣のグルーピング.
		Bak_UtilityListPrintSide.showEnable( True );								// ｢片面/両面｣のグルーピング.
		Bak_UtilityListPrintEntry.showEnable( True );								//「詳細カウンター」グルーピング(印刷項目のグルーピングを共用する).
		Txt_UtilitySetPrintEntry.showEnable( False );								//「印刷項目」テキスト.
		Rbt_UtilityListPrintEntry.showEnable( False );								//「印刷項目:全ての情報/印字情報のみ」キー.
		Key_AdminFaxSettingListLineSelect.showEnable( False );						//「回線選択」キー.
		Txt_AdminFaxSettingListLineSelect.showEnable( False );						//「回線選択」テキスト.
		Key_UtilityListPrintPrintItem.showEnable( False );							//「印刷項目」キー(機能拡張1以降).
	} else {
		Bak_UtilityListPrintSide.showEnable( False );									// ユーティリティグルーピング_タイトル(極小)-10920600.
		Bak_UtilityListPrintEntry.showEnable( False );								// ユーティリティグルーピング_タイトル(極小)-10920600.
		Txt_UtilitySetPrintEntry.showEnable( False );									//  「印刷項目」.
		Rbt_UtilityListPrintEntry.showEnable( False );								// 10920600_Do 使用管理カウンターリストプリントで使用されるグループ.
		Key_AdminFaxSettingListLineSelect.showEnable( False );
		Txt_AdminFaxSettingListLineSelect.showEnable( False );
		Bak_85X640Y104W296H320.showEnable( True );
		Key_UtilityListPrintPrintItem.showEnable( False );
// comment 2016/09/13 SCD2-Sky辻 [特ROM][FQ-OPSD-1600252-OFP174] ベンダーモード時のカウンターリスト出力でEco関連情報が出力される.
		Bak_UtilityTotalCounterPrintCounterDetails.showEnable( False );
		Txt_UtilityTotalCounterPrintCounterDetails.showEnable( False );
		Rbt_UtilityTotalCounterPrintCounterDetails.showEnable( False );
	}
	if( True == PLBC_FuncVersionCheck::isOK_Type1() ) {
		//・選択したトレイの用紙が所定のサイズでない場合、ペーパーエンプティの場合、「実行」キーグレイアウト.
		//・両面選択時に両面不可の用紙の給紙トレイを選択した場合、「実行」キーグレイアウト.
		if( False == printStartDisable() ) {
			Key_UtilityUnderExec.activate( True );		//実行キー 通常表示.
		} else {
			Key_UtilityUnderExec.activate( False );		//実行キー グレイアウト.
		}
	}
	// 用紙情報の取得を行う.
	APIC_FeedTray* pc_FeedTray = PnlComGetFeedTray();
	TYPC_PaperType c_PaperType = pc_FeedTray->getPaperType( pc_FeedTraySelectListPrint.getSPCFeedTray() );
	//  メッセージ表示の切り替えを行う.
	Bool b_Custom = ( TYPE_CustomPaperType_UnSelect == c_PaperType.getCustomPaperType() ) ? False : True;
	Bool b_Normal = ( TYPE_PaperTypeNormal == c_PaperType.getPaperType() && False == b_Custom ) ? True : False;
	Bool b_Single = ( TYPE_PaperTypeNotDuplex1 == c_PaperType.getPaperType() && False == b_Custom ) ? True : False;
	if( ( False == b_Normal ) &&												// 普通紙以外.
	    ( True == pc_FeedTraySelectListPrint.getSPCDuplexOn() ) ) {				// 両面選択時.
		Txt_MessageUtilityListPrint.changeText( s_MSGSelectPlainPaperDuplexPrinting );	// 両面の場合は普通紙を選択してください.
	} else if( ( ( False == b_Normal )  &&	// 普通紙以外.
	             ( False == b_Single ) ) &&	// 片面専用用紙以外.
	           ( False == pc_FeedTraySelectListPrint.getSPCDuplexOn() ) ) {		// 片面キー選択時.
		Txt_MessageUtilityListPrint.changeText( s_MSGSelectPlainOrSingleSideOnlyPaper_b );
	} else {	// 正常時.
		if( TYPE_ListPrintNetworkConfig == PAdminSystemDepartListPrintListPrint::e_ListPrint ) {				// ネットワーク設定リストの場合.
			TYPE_NetworkIFMode e_NetworkIFMode = APIC_ExNetworkSetting::newInstance()->getNetworkIFMode();
			if( ( True == PnlComGetSystemInfo()->getExNetworkAdapter() )     // 拡張ネットワークアダプタ装着時.
			    && ( ( TYPE_NetworkIFMode_DualEthernetSta == e_NetworkIFMode )    // デュアルIF構成 有線＋無線子機モード.
			         || ( TYPE_NetworkIFMode_DualEthernetWolLimited == e_NetworkIFMode ) ) ) { // デュアルIF構成 有線＋無線子機(ErP復帰限定)モード.
				// 拡張ネットワーク設定が表示される条件を満たしている場合.
				if( TYPE_SystemType_US == PnlComGetMachineSetting()->getSystemType() )	{
					// 仕向がけが北米の場合.
					Txt_MessageUtilityListPrint.changeText( s_MSGSelectPaperNetwork_US );
				} else {
					// 仕向がけが北米以外の場合.
					Txt_MessageUtilityListPrint.changeText( s_MSGSelectPaperNetwork );
				}
			} else {
				if( TYPE_SystemType_US == PnlComGetMachineSetting()->getSystemType() )	{
					// 仕向がけが北米の場合.
// 2015/10/26 HCLT-Reshmi 管理No.1435_6_2 Change list printable paper size/ type Start
#ifdef DEF_A4_Cus
					Txt_MessageUtilityListPrint.changeText( s_MSGListPrintSelectPaper_US );
#else
					Txt_MessageUtilityListPrint.changeText( s_MSGSelectPaper_US );
#endif 
				} else {
					// 仕向がけが北米以外の場合
#ifdef DEF_A4_Cus
					Txt_MessageUtilityListPrint.changeText( s_MSGListPrintSelectPaper );
#else
					Txt_MessageUtilityListPrint.changeText( s_MSGSelectPaper );
#endif 

				}
			}
		} else {
			// 仕向けに応じてメッセージ編集.
			if( TYPE_SystemType_US == PnlComGetMachineSetting()->getSystemType() )	{
#ifdef DEF_A4_Cus
					Txt_MessageUtilityListPrint.changeText( s_MSGListPrintSelectPaper_US );
#else
				Txt_MessageUtilityListPrint.changeText( s_MSGSelectPaper_US );
#endif
			} else {
#ifdef DEF_A4_Cus
					Txt_MessageUtilityListPrint.changeText( s_MSGListPrintSelectPaper );
#else
				Txt_MessageUtilityListPrint.changeText( s_MSGSelectPaper );
#endif
// 2015/10/26 HCLT-Reshmi 管理No.1435_6_2 Change list printable paper size/ type End
			}
		}
	}
// comment 2014/12/10 OPS1-Sky西原 [HeliosL][制御構成変更No.1]封筒通紙の操作性改善（定着手動離間）.
	if( True == PnlComIsFuserRetracted() ){		// 定着離間中は印刷不可のメッセージに切り替え.
		Txt_MessageUtilityListPrint.showEnable( False );
		Txt_AdminFuserRetractedErrorMessage.showEnable( True );
	} else {
		Txt_MessageUtilityListPrint.showEnable( True );
		Txt_AdminFuserRetractedErrorMessage.showEnable( False );
	}
}
//================================================================================================.
// 概要		:管理者設定・環境設定・管理リストプリント(2863) :背景画面のセット..
// 戻り値	:なし.
//================================================================================================.
void PAdminSystemDepartListPrintListPrint::setDefaultBackParts()
{
	if( True == PLBC_FuncVersionCheck::isOK_MiZe_Type2()
	    && TYPE_ListPrintFaxSetting == PAdminSystemDepartListPrintListPrint::e_ListPrint ) {
		// FAX設定値リスト(Minerva機能拡張以降)デフォルト背景.
		setDefaultBackPartsInfo( ( PDisplayParts** )tbl_AdminSystemDepartListPrintListPrintFaxSettingDefault, ss_AdminSystemDepartListPrintListPrintFaxSettingDefault );
	} else {	// 標準用デフォルト背景.
		setDefaultBackPartsInfo( ( PDisplayParts** )tbl_AdminSystemDepartListPrintListPrintDefault, ss_AdminSystemDepartListPrintListPrintDefault );
	}
}
//================================================================================================.
// 概要		:管理者設定・環境設定・管理リストプリント(2863) :背景画面のセット..
// 戻り値	:なし.
//================================================================================================.
void PAdminSystemDepartListPrintListPrint::procGoToPage( uchar option )
{
	if( False == pc_FeedTraySelectListPrint.checkSPCAnyKeyOn() ) { return; }	// プリント中は、受付禁止.
	if( True == PLBC_FuncVersionCheck::isOK_MiZe_Type2()
	    && TYPE_ListPrintFaxSetting == PAdminSystemDepartListPrintListPrint::e_ListPrint ) {
		switch( option ) {
			case BTNID_AdminFaxSettingListLineSelect:
				nextGamen = &gp_AdminSystemDepartListPrintFaxSettingListLineSelect;
				break;
			default:
				PAdminSystemDepartListPrint::procGoToPage( option );	break;
		}
//	}else{	// Minerva機能拡張以前は上位の指示に従う.
//		PAdminSystemDepartListPrint::procGoToPage( option );
//	}
	} else if( True ==  PLBC_FuncVersionCheck::isOK_IT5_Type1( ) ) {
		switch( option ) {
			case BTNID_PAdminSystemDepartListPrint_PRINTITEM:
				nextGamen = &gp_AdminSystemDepartListPrintListPrintPrintItem;
				break;
			default:
				break;
		}

	} else {	// Minerva機能拡張以前は上位の指示に従う.
		PAdminSystemDepartListPrint::procGoToPage( option );
	}
}
#ifdef	  DEF_THAMES_ICUSTOM	// アプリ割付.
//================================================================================================
// 概要		:管理者設定・環境設定・管理リストプリント(2863): ブラウザキー押下.
// 戻り値	:なし.
// 備考		:なし.
//================================================================================================
void PAdminSystemDepartListPrintListPrint::procBrowser( uchar option )
{
	if( pc_FeedTraySelectListPrint.checkSPCAnyKeyOn() == False ) { return; }	// プリント中は、受付禁止.
	PAdmincom::procBrowser( option );
}
#endif // DEF_THAMES_ICUSTOM	// アプリ割付.
//================================================================================================
// 概要		:管理者設定・環境設定・管理リストプリント(2863): 遷移元情報の設定.
// 戻り値	:なし.
// 備考		:なし.
// 履歴		:2015/07/08 SecV1-5【管理者によるサービスモード制御技術】 Sky 新道.
//================================================================================================
void PAdminSystemDepartListPrintListPrint::setDispTransition( uchar uc_DispTransitionDate )
{
	m_uc_DispTransition = uc_DispTransitionDate;	// 遷移元の情報を設定する.
}

//////////////////////////////////////////////////////////////
// 概要		:管理者設定・環境設定・管理リストプリント ：しおりの遷移可能不可能状態.
// 戻り値	:
//////////////////////////////////////////////////////////////
Bool PAdminSystemDepartListPrintListPrint::isEnableNextGamenBookmark()
{
	if( pc_FeedTraySelectListPrint.getSPCPrintBusy() == True ) {
		// プリント中は、受付禁止.
		return False;
	}
	return True;
}
//////////////////////////////////////////////////////////////
// 概要		:管理者設定・環境設定・管理リストプリント : キャンセル.
// 戻り値	:.
//////////////////////////////////////////////////////////////
void PAdminSystemDepartListPrintListPrint::procCancel( uchar option )
{
	if( pc_FeedTraySelectListPrint.checkSPCAnyKeyOn() == False ) { return; }	// プリント中は、受付禁止.
	// リスト種類に応じた遷移先の変更.
	switch( PAdminSystemDepartListPrintListPrint::e_ListPrint ) {
		case  TYPE_ListPrintJobMemory:	// 設定メモリリストキー
		case  TYPE_ListPrintUserChoice:	// 設定値リストキー
			nextGamen = &gp_AdminSystemDepartListPrint;	// 管理者設定・環境設定・管理リストプリント.
			break;
		case  TYPE_ListPrintTrackCounter:	// 使用管理カウンタリストキー
			nextGamen = &gp_AdminUserAndDepartSet;	// 管理者設定・ユーザ認証/部門管理設定メニュー.
			break;
		case TYPE_ListPrintFaxSetting: // ファクス設定リスト.
// comment 2022/07/20 Sky西場 [IT6_2.3][OP_BTS-37620][スリム化].
			ExpPanelLog( PNLD_ExpLogKind_AlwaysLogOut, "[DeleteUnnecessaryProcessing] [%s]", __PRETTY_FUNCTION__, 0, 0, 0, 0, 0 );
//			nextGamen = &gp_AdminFaxSetting;	// 管理者設定・ユーザ認証/部門管理設定メニュー.
			break;
		case TYPE_ListPrintInspectLogTime:	// 監査ログ・指定日時から1000件.
			nextGamen = &gp_AdminSecurityInspectLogList;	// 管理者設定・セキュリティ設定・監査ログリスト.
			break;
		case TYPE_ListPrintCounter:						// カウンタリスト.
		case TYPE_ListPrintUnitCheck:					// 消耗品確認リスト.
// comment 2022/04/04 Sky西場 [IT6_2.3][OP_BTS-37620][スリム化].
//			nextGamen = &gp_AdminSystemListAndCounter;	// 10905600   リスト/カウンター.
			PToAdminMenucom::procOk( option );
			ExpPanelLog( PNLD_ExpLogKind_AlwaysLogOut, "[DeleteUnnecessaryProcessing] [%s]", __PRETTY_FUNCTION__, 0, 0, 0, 0, 0 );
			break;
// comment 2022/09/06 Sky西場 [IT6_2.3][OP_BTS-37620][スリム化].
//		case TYPE_ListPrintFEKDLKRequestCode:	nextGamen = &gp_AdminLicenseRequestCodePublication;	break;	//  リクエストコード.管理者設定・ライセンス管理設定・リクエストコード.
		case TYPE_ListPrintFEKDLKRequestCode:	//  リクエストコード.管理者設定・ライセンス管理設定・リクエストコード.
			ExpPanelLog( PNLD_ExpLogKind_AlwaysLogOut, "[DeleteUnnecessaryProcessing] [%s]", __PRETTY_FUNCTION__, 0, 0, 0, 0, 0 );
			break;
		case 72: // 送信操作ログ.
// comment 2022/04/04 Sky西場 [IT6_2.3][OP_BTS-37620][スリム化].
//			nextGamen = &gp_AdminSystemSendOpeLogOutSetting;           // 管理者設定・環境設定・リスト/カウンタ・送信操作ログ出力設定.
			PToAdminMenucom::procOk( option );
			ExpPanelLog( PNLD_ExpLogKind_AlwaysLogOut, "[DeleteUnnecessaryProcessing] [%s]", __PRETTY_FUNCTION__, 0, 0, 0, 0, 0 );
			break;
		case TYPE_ListPrintNetworkConfig:			// ネットワーク設定値リスト.
// comment 2022/06/17 Sky西場 [IT6_2.3][OP_BTS-37620][スリム化].
			ExpPanelLog( PNLD_ExpLogKind_AlwaysLogOut, "[DeleteUnnecessaryProcessing] [%s]", __PRETTY_FUNCTION__, 0, 0, 0, 0, 0 );
//			if( PNLE_AdminFromNetworkSetting == uc_FromNetworkMode ) {		// ネットワーク設定画面からの遷移.
//				nextGamen = &gp_AdminNetworkSet;							// 管理者設定・ネットワーク設定メニュー.
//			} else {														// 拡張ネットワーク設定画面からの遷移.
//				nextGamen = &gp_AdminExpansionNetworkSetting;				// 管理者設定・拡張ネットワーク設定メニュー.
//			}
			break;
		// 2015/07/02 SecV1-5 【管理者によるサービスモード制御技術】 Sky 新道.
		case  TYPE_ListPrintAdminPasswordReleaseKey:
// comment 2022/08/30 Sky西場 [IT6_2.3][OP_BTS-37620][スリム化].
			ExpPanelLog( PNLD_ExpLogKind_AlwaysLogOut, "[DeleteUnnecessaryProcessing] [%s]", __PRETTY_FUNCTION__, 0, 0, 0, 0, 0 );
//			nextGamen = &gp_AdminPasswordReleaseKeyConf;	// 管理者パスワード解除キー確認.
			break;
		default:
			break;
	}
	// 左エリア種類の有効設定（ジョブ表示（通常））startGamen()の最後でコールしてください.
	setLeftAreaEnableType( TYPD_LeftAreaBookmark | TYPD_LeftAreaSoft10Key );
}

//================================================================================================
// 概要		:管理者設定・ユーザ認証/部門管理(2690): コンストラクタ
// 戻り値	:なし
//================================================================================================
PAdminUserAndDepartSet::PAdminUserAndDepartSet( PDisplayParts** partsTable, short partsNumber, Bool isNoWarningGamen )	// コンストラクタ
	: PToAdminSetMenucom( partsTable, partsNumber, isNoWarningGamen )
{
	uc_page = 1; 			// ﾍﾟｰｼﾞ数の表示.
	b_fromMenu = False;			// ↑画面からの遷移.
	b_EnableAuthDeviceSetting  = False;
}
//================================================================================================
// 概要		:管理者設定・ユーザ認証/部門管理(2690) : 画面移動
// 戻り値	:なし
//================================================================================================
void PAdminUserAndDepartSet::procGoToPage( uchar option )
{
// Comment 2015/08/01 HCL_Jeevitha【ZeusS_ZX0】【RQ-261】Assigned nextgamen for URL Address Setting key.
	if( True == PLBC_FuncVersionCheck::isOK_IT5_Type4() ) {
//Comment 2017/09/04 [HCL-Harish] [IT5 4.2][RQ-686] [WP509] Multiple Network support.
		TYPE_NetworkSpecific e_NetworkSpecific  = PNLC_NetworkLan::getInstance()->getAdminNWLine( ) ;
		if( uc_page == 1 ) {	//P1の時.
			if( bl_BumonLimitWarningAdminMove == False ) {	// 通常時.
				APIC_AuthenticationSeverSetting* apicAuthSrv = APIC_AuthenticationSeverSetting::newInstance();
// comment [2016/12/22] [PDD22山下(日立)] [IT5_4.0][AccuRev.1009508]認証拡張FUM追加コードに対して新詳細コードを追加.
				if( NULL == apicAuthSrv ) {
//Comment 2019/06/14 [HCLT_Monisha][EagleH][N Rank] Removed unwanted Detailed code.
//					LAPC_BFW_System *pc_BFW_System = LAPC_BFW_System::newInstance();
//					if (NULL != pc_BFW_System){    
//					    pc_BFW_System ->registDetailCode( PNLD_DetailCode_Admin_0x19000200104E0301 );
//					}
					LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" ); 
					return;
				}

				switch( option ) {
					case  1:	// 認証方式キー.
						if( True == isAdminMachineDirectLogin() ) {
							setBuzzerType( PCGE_BuzzerType_InputNg );
							break;
						}
						b_fromMenu = True;			// ↑画面からの遷移.
						nextGamen = pc_MenuUtilityAndAdmin.SPCprocGoToPage( option );				// 管理者設定・ユーザ認証/部門管理・ユーザ/部門の連動認証
						break;
					case  2:	// ユーザ認証する場合、ユーザ認証画面へ遷移する.
//						if( PnlComGetUserAuthentication()->getUserAuthentication() != TYPE_UserAuthenticationOff ) {
						if( PnlComGetUserAuthentication()->getUserAuthentication( e_NetworkSpecific ) != TYPE_UserAuthenticationOff ) {
							nextGamen = pc_MenuUtilityAndAdmin.SPCprocGoToPage( option );	// 管理者設定・ユーザ認証/部門管理・ユーザ認証
						} else {
							setBuzzerType( PCGE_BuzzerType_InputNg );	// 報知音設定
						}
						break;
					case  3:	// 部門管理する場合、部門管理画面へ遷移する.
//						if( PnlComGetTrackManager()->getTrackMode() != TYPE_TrackModeOff ) {
						if( PnlComGetTrackManager()->getTrackMode( e_NetworkSpecific ) != TYPE_TrackModeOff ) {
							nextGamen = pc_MenuUtilityAndAdmin.SPCprocGoToPage( option );		// 管理者設定・ユーザ認証/部門管理・部門管理
						} else {
							setBuzzerType( PCGE_BuzzerType_InputNg );	// 報知音設定
						}
						break;
					case  4: // 管理者設定・ユーザ認証/部門管理・認証指定なしプリント
						// 上階層からの遷移.
						PLBC_CheckFromMenu::getInstance()->setFromMenu( True );
						nextGamen = pc_MenuUtilityAndAdmin.SPCprocGoToPage( option );	// 10920500_Mi-K3 認証指定なし印刷.
						break;
					case  5:	// 使用管理カウンタリストキーへ遷移する.
						// ユーザ認証、部門管理共に行わない場合、使用管理カウンタリストキーは非表示なので、遷移しない.
						// ベンダー1、2、管理装置2、認証装置1、キーカウンター装着時は非表示.
//						if( PnlComGetUserAuthentication()->getUserAuthentication() == TYPE_UserAuthenticationOff
//						    && PnlComGetTrackManager()->getTrackMode() == TYPE_TrackModeOff ) {
						if( PnlComGetUserAuthentication()->getUserAuthentication( e_NetworkSpecific ) == TYPE_UserAuthenticationOff
						    && PnlComGetTrackManager()->getTrackMode( e_NetworkSpecific ) == TYPE_TrackModeOff ) {
							// ユーザ認証OFFかつ、部門管理OFFの場合非表示.
							// 他の条件は上位の画面で制限.
							setBuzzerType( PCGE_BuzzerType_InputNg );		// 報知音設定
						} else {
							PAdminSystemDepartListPrintListPrint::e_ListPrint = TYPE_ListPrintTrackCounter;
							PToAdminSetMenucom::setSelectPrintItem( TYPE_ListPrintInfoAll );
							PToAdminSetMenucom::setSelectCounterDetails( False );
							nextGamen = pc_MenuUtilityAndAdmin.SPCprocGoToPage( option );	// 管理者設定 > ユｰザｰ認証/部門管理 > 使用管理カウンターリストプリント.
						}
						break;
					case  6:	// 外部サーバー設定キー.
						if( True == apicAuthSrv->checkServerGroup() ) {
							PLBC_CheckFromMenu::getInstance()->setFromMenu( True );
						}
						nextGamen = pc_MenuUtilityAndAdmin.SPCprocGoToPage( option );
						break;
					case 7:		// 優先/代替サーバー接続状態キー..
						nextGamen = pc_MenuUtilityAndAdmin.SPCprocGoToPage( option );
						break;
					case  8:	// 参照許可設定キー.
						if( False == PnlComIsUseSsdScanSend() ) {	// HDD/SSD使用不可.
							setBuzzerType( PCGE_BuzzerType_InputNg );		// 報知音設定.
						} else {
							nextGamen = pc_MenuUtilityAndAdmin.SPCprocGoToPage( option );	// 管理者設定 > ユｰザｰ認証/部門管理 > 参照許可設定.
						}
						break;
					case  9:	// 認証装置設定キー.
						if( b_EnableAuthDeviceSetting == True
//						    ||( TYPE_UserAuthenticationIntermeddiateServer == PnlComGetUserAuthentication()->getUserAuthentication() 		// 中間認証.
//						        || TYPE_UserAuthenticationDeviceAndMidAuth    == PnlComGetUserAuthentication()->getUserAuthentication() ) ) {	// 本体+中間認証.
						    ||( TYPE_UserAuthenticationIntermeddiateServer == PnlComGetUserAuthentication()->getUserAuthentication( e_NetworkSpecific ) 		// 中間認証.
						        || TYPE_UserAuthenticationDeviceAndMidAuth    == PnlComGetUserAuthentication()->getUserAuthentication( e_NetworkSpecific ) ) ) {	// 本体+中間認証.
							nextGamen = pc_MenuUtilityAndAdmin.SPCprocGoToPage( option );		// 10920900 認証装置設定 .
						} else {	// 条件を満たしていない場合は報知音を鳴らして、遷移しない.
							setBuzzerType( PCGE_BuzzerType_InputNg );		// 報知音設定
						}
						break;
					case  10:	// ユーザー/部門共通設定キー.
						nextGamen = pc_MenuUtilityAndAdmin.SPCprocGoToPage( option );	// ユーザー/部門共通設定.
						break;
					case 11: PToAdminSetMenucom::procGoToPage( option );			break;	// 管理者設定キー
					default:
						setBuzzerType( PCGE_BuzzerType_InputNg );		break;	// 報知音設定
				}
			}
			else {	// 上限値オーバー発生時.
				switch( option ) {
					case  2:	// ユーザ認証する場合、ユーザ認証画面へ遷移する.
//						if( PnlComGetUserAuthentication()->getUserAuthentication() != TYPE_UserAuthenticationOff )
						if( PnlComGetUserAuthentication()->getUserAuthentication( e_NetworkSpecific ) != TYPE_UserAuthenticationOff )
						{ nextGamen = pc_MenuUtilityAndAdmin.SPCprocGoToPage( option ); }	// 管理者設定・ユーザ認証/部門管理・ユーザ認証
						else
						{ setBuzzerType( PCGE_BuzzerType_InputNg ); }	// 報知音設定
						break;
					case  3:	// 部門管理する場合、部門管理画面へ遷移する.
//						if( PnlComGetTrackManager()->getTrackMode() != TYPE_TrackModeOff )
						if( PnlComGetTrackManager()->getTrackMode( e_NetworkSpecific ) != TYPE_TrackModeOff )
						{ nextGamen = pc_MenuUtilityAndAdmin.SPCprocGoToPage( option ); }		// 管理者設定・ユーザ認証/部門管理・部門管理
						else
						{ setBuzzerType( PCGE_BuzzerType_InputNg ); }	// 報知音設定
						break;
					case 11: PToAdminSetMenucom::procGoToPage( option );			break;	// 管理者設定キー
					default:	setBuzzerType( PCGE_BuzzerType_InputNg );		break;	// 報知音設定
				}
			}
		}
		else {	//P2のとき.
			switch( option ) {
				case 1:	//URL Address Setting.
					if( True == c_HD_Equip.isOK() ) {
						nextGamen = pc_MenuUtilityAndAdmin.SPCprocGoToPage( option );
					} else {
						setBuzzerType( PCGE_BuzzerType_InputNg );
					}
					break;
				case  2:	//Home宛先有効設定キー.
					if( bl_BumonLimitWarningAdminMove == False ) {	// 通常時.
						if( False == PnlComIsUseSsdScanSend() ) {	// HDD/SSD使用不可.
							//HDD非装着時は拒否音鳴動.
							setBuzzerType( PCGE_BuzzerType_InputNg );		// 報知音設定.
						} else {
							nextGamen = pc_MenuUtilityAndAdmin.SPCprocGoToPage( option );	// 管理者設定 > ユｰザｰ認証/部門管理 > Home宛先有効設定.
						}
					}
					else {
						nextGamen = pc_MenuUtilityAndAdmin.SPCprocGoToPage( option );	// 50922000 Home宛先有効設定画面へ遷移.
					}
					break;
				case 3:	//送信宛先制限キー.
					if( True == checkNextPageContents() ) {
						// HDD装着時50922100 送信宛先制限画面へ遷移.
						nextGamen = pc_MenuUtilityAndAdmin.SPCprocGoToPage( option );
					} else {
						// HDD非装着時拒否音鳴動.
						setBuzzerType( PCGE_BuzzerType_InputNg );
					}
					break;

				case 4:	//プリント簡易認証キー.
// comment 2016/12/07 Sky東島 [特ROM][AU DoD].
//					if( True == PLBC_FuncPKICheck::isPKI() ) {		// ＰＫＩカード認証設定が有効の場合.
					if( ( True == PLBC_FuncPKICheck::isPKI() )		// ＰＫＩカード認証設定が有効かつAU DODモードではない場合.
					 && ( False == isContactlessPKICard() ) ) {
						setBuzzerType( PCGE_BuzzerType_InputNg );
					} else {
						nextGamen = pc_MenuUtilityAndAdmin.SPCprocGoToPage( option );
					}
					break;
				case 5:	// 証明書検証設定キー.
					if( True == PLBC_FuncPKICheck::isPKI() ) {		// ＰＫＩカード認証設定が有効の場合.
						nextGamen = pc_MenuUtilityAndAdmin.SPCprocGoToPage( option );
					} else {
						setBuzzerType( PCGE_BuzzerType_InputNg );
					}
					break;
				case 6:	// 拡張サーバーダウン時の上限.
					nextGamen = pc_MenuUtilityAndAdmin.SPCprocGoToPage( option );	// C092210000 拡張サーバーダウン時の上限画面へ遷移.
					break;
				case 7:		// 課金情報の更新.
					nextGamen = pc_MenuUtilityAndAdmin.SPCprocGoToPage( option );			// C092220000 課金情報の更新画面へ遷移.
					break;
				case 8:		// AD認証時自己検証設定
					nextGamen	= pc_MenuUtilityAndAdmin.SPCprocGoToPage( option );
// comment 2022/05/20 Sky西場 [IT6_2.3][OP_BTS-37620][スリム化].
//					gp_AdminNetworkSetSSOAuthDomainSet.m_bl_frommenu = True;
					break;
				case 11: PToAdminSetMenucom::procGoToPage( option );			break;	// 管理者設定キー.
				default:
					setBuzzerType( PCGE_BuzzerType_InputNg );		break;	// 報知音設定.
			}
		}
	}
//	if( True == PLBC_FuncVersionCheck::isOK_IT5_Type1() ) {
	else if( True == PLBC_FuncVersionCheck::isOK_IT5_Type1() ) {
		if( uc_page == 1 ) {	//P1の時.
			if( bl_BumonLimitWarningAdminMove == False ) {	// 通常時.
				switch( option ) {
					case  1:	// 認証方式キー.
						if( True == isAdminMachineDirectLogin() ) {
							setBuzzerType( PCGE_BuzzerType_InputNg );
							break;
						}
						b_fromMenu = True;			// ↑画面からの遷移.
						nextGamen = pc_MenuUtilityAndAdmin.SPCprocGoToPage( option );				// 管理者設定・ユーザ認証/部門管理・ユーザ/部門の連動認証
						break;
					case  2:	// ユーザ認証する場合、ユーザ認証画面へ遷移する.
// 	Comment 2017/09/28 [HCL-Harish]【IT5 4.2】【RQ-686】【WP509】Multiple Network support.
//						if( PnlComGetUserAuthentication()->getUserAuthentication() != TYPE_UserAuthenticationOff ) {
						if( PnlComGetUserAuthentication()->getUserAuthentication( PNLC_NetworkLan::getInstance() ->getAdminNWLine( ) ) != TYPE_UserAuthenticationOff ) {
							nextGamen = pc_MenuUtilityAndAdmin.SPCprocGoToPage( option );	// 管理者設定・ユーザ認証/部門管理・ユーザ認証
						} else {
							setBuzzerType( PCGE_BuzzerType_InputNg );	// 報知音設定
						}
						break;
					case  3:	// 部門管理する場合、部門管理画面へ遷移する.
						if( PnlComGetTrackManager()->getTrackMode( PNLC_NetworkLan::getInstance()->getAdminNWLine() ) != TYPE_TrackModeOff ) {
							nextGamen = pc_MenuUtilityAndAdmin.SPCprocGoToPage( option );		// 管理者設定・ユーザ認証/部門管理・部門管理
						} else {
							setBuzzerType( PCGE_BuzzerType_InputNg );	// 報知音設定
						}
						break;
					case  4: // 管理者設定・ユーザ認証/部門管理・認証指定なしプリント
						// 上階層からの遷移.
						PLBC_CheckFromMenu::getInstance()->setFromMenu( True );
						nextGamen = pc_MenuUtilityAndAdmin.SPCprocGoToPage( option );	// 10920500_Mi-K3 認証指定なし印刷.
						break;
					case  5:	// 使用管理カウンタリストキーへ遷移する.
						// ユーザ認証、部門管理共に行わない場合、使用管理カウンタリストキーは非表示なので、遷移しない.
						// ベンダー1、2、管理装置2、認証装置1、キーカウンター装着時は非表示.
// 	Comment 2017/09/28 [HCL-Harish]【IT5 4.2】【RQ-686】【WP509】Multiple Network support.
//						if( PnlComGetUserAuthentication()->getUserAuthentication() == TYPE_UserAuthenticationOff
//							&& PnlComGetTrackManager()->getTrackMode() == TYPE_TrackModeOff ) {
						if( PnlComGetUserAuthentication()->getUserAuthentication( PNLC_NetworkLan::getInstance() ->getAdminNWLine( ) ) == TYPE_UserAuthenticationOff
							&& PnlComGetTrackManager()->getTrackMode(PNLC_NetworkLan::getInstance() ->getAdminNWLine( )) == TYPE_TrackModeOff ) {
							// ユーザ認証OFFかつ、部門管理OFFの場合非表示.
							// 他の条件は上位の画面で制限.
							setBuzzerType( PCGE_BuzzerType_InputNg );		// 報知音設定
						} else {
							PAdminSystemDepartListPrintListPrint::e_ListPrint = TYPE_ListPrintTrackCounter;
							PToAdminSetMenucom::setSelectPrintItem( TYPE_ListPrintInfoAll );
							PToAdminSetMenucom::setSelectCounterDetails( False );
							nextGamen = pc_MenuUtilityAndAdmin.SPCprocGoToPage( option );	// 管理者設定 > ユｰザｰ認証/部門管理 > 使用管理カウンターリストプリント.
						}
						break;
					case  6:	// 外部サーバー設定キー.
						nextGamen = pc_MenuUtilityAndAdmin.SPCprocGoToPage( option );
						break;
					case 7:		// 優先/代替サーバー接続状態キー..
						nextGamen = pc_MenuUtilityAndAdmin.SPCprocGoToPage( option );
						break;
					case  8:	// 参照許可設定キー.
						if( False == PnlComIsUseSsdScanSend() ) {	// HDD/SSD使用不可.
							setBuzzerType( PCGE_BuzzerType_InputNg );		// 報知音設定.
						} else {
							nextGamen = pc_MenuUtilityAndAdmin.SPCprocGoToPage( option );	// 管理者設定 > ユｰザｰ認証/部門管理 > 参照許可設定.
						}
						break;
					case  9:	// 認証装置設定キー.
						if( b_EnableAuthDeviceSetting == True
// 	Comment 2017/09/28 [HCL-Harish]【IT5 4.2】【RQ-686】【WP509】Multiple Network support.
//						    ||( TYPE_UserAuthenticationIntermeddiateServer == PnlComGetUserAuthentication()->getUserAuthentication() 		// 中間認証.
//						        || TYPE_UserAuthenticationDeviceAndMidAuth    == PnlComGetUserAuthentication()->getUserAuthentication() ) ) {	// 本体+中間認証.
	   						||( TYPE_UserAuthenticationIntermeddiateServer == PnlComGetUserAuthentication()->getUserAuthentication( PNLC_NetworkLan::getInstance() ->getAdminNWLine( ) ) 		// 中間認証.
						        || TYPE_UserAuthenticationDeviceAndMidAuth    == PnlComGetUserAuthentication()->getUserAuthentication( PNLC_NetworkLan::getInstance() ->getAdminNWLine( ) ) ) ) {	// 本体+中間認証.
							nextGamen = pc_MenuUtilityAndAdmin.SPCprocGoToPage( option );		// 10920900 認証装置設定 .
						} else {	// 条件を満たしていない場合は報知音を鳴らして、遷移しない.
							setBuzzerType( PCGE_BuzzerType_InputNg );		// 報知音設定
						}
						break;
					case  10:	// ユーザー/部門共通設定キー.
						nextGamen = pc_MenuUtilityAndAdmin.SPCprocGoToPage( option );	// ユーザー/部門共通設定.
						break;
					case 11: PToAdminSetMenucom::procGoToPage( option );			break;	// 管理者設定キー
					default:
						setBuzzerType( PCGE_BuzzerType_InputNg );		break;	// 報知音設定
				}
			}
			else {	// 上限値オーバー発生時.
				switch( option ) {
					case  2:	// ユーザ認証する場合、ユーザ認証画面へ遷移する.
// 	Comment 2017/09/28 [HCL-Harish]【IT5 4.2】【RQ-686】【WP509】Multiple Network support.
//						if( PnlComGetUserAuthentication()->getUserAuthentication() != TYPE_UserAuthenticationOff )
						if( PnlComGetUserAuthentication()->getUserAuthentication( PNLC_NetworkLan::getInstance() ->getAdminNWLine( ) ) != TYPE_UserAuthenticationOff )
						{ nextGamen = pc_MenuUtilityAndAdmin.SPCprocGoToPage( option ); }	// 管理者設定・ユーザ認証/部門管理・ユーザ認証
						else
						{ setBuzzerType( PCGE_BuzzerType_InputNg ); }	// 報知音設定
						break;
					case  3:	// 部門管理する場合、部門管理画面へ遷移する.
						if( PnlComGetTrackManager()->getTrackMode( PNLC_NetworkLan::getInstance()->getAdminNWLine() ) != TYPE_TrackModeOff )
						{ nextGamen = pc_MenuUtilityAndAdmin.SPCprocGoToPage( option ); }		// 管理者設定・ユーザ認証/部門管理・部門管理
						else
						{ setBuzzerType( PCGE_BuzzerType_InputNg ); }	// 報知音設定
						break;
					case 11: PToAdminSetMenucom::procGoToPage( option );			break;	// 管理者設定キー
					default:	setBuzzerType( PCGE_BuzzerType_InputNg );		break;	// 報知音設定
				}
			}
		}
		else {	//P2のとき.
			switch( option ) {
				case  1:	//Home宛先有効設定キー.
					if( bl_BumonLimitWarningAdminMove == False ) {	// 通常時.
						if( False == PnlComIsUseSsdScanSend() ) {	// HDD/SSD使用不可.
							//HDD非装着時は拒否音鳴動.
							setBuzzerType( PCGE_BuzzerType_InputNg );		// 報知音設定.
						} else {
							nextGamen = pc_MenuUtilityAndAdmin.SPCprocGoToPage( option );	// 管理者設定 > ユｰザｰ認証/部門管理 > Home宛先有効設定.
						}
					}
					else {
						nextGamen = pc_MenuUtilityAndAdmin.SPCprocGoToPage( option );	// 50922000 Home宛先有効設定画面へ遷移.
					}
					break;
				case 2:	//送信宛先制限キー.
					if( True == checkNextPageContents() ) {
						// HDD装着時50922100 送信宛先制限画面へ遷移.
						nextGamen = pc_MenuUtilityAndAdmin.SPCprocGoToPage( option );
					} else {
						// HDD非装着時拒否音鳴動.
						setBuzzerType( PCGE_BuzzerType_InputNg );
					}
					break;

				case 3:	//プリント簡易認証キー.
// comment 2016/12/07 Sky東島 [特ROM][AU DoD].
//					if( True == PLBC_FuncPKICheck::isPKI() ) {		// ＰＫＩカード認証設定が有効の場合.
					if( ( True == PLBC_FuncPKICheck::isPKI() )		// ＰＫＩカード認証設定が有効かつAU DODモードではない場合.
					 && ( False == isContactlessPKICard() ) ) {
						setBuzzerType( PCGE_BuzzerType_InputNg );
					} else {
						nextGamen = pc_MenuUtilityAndAdmin.SPCprocGoToPage( option );
					}
					break;
				case 4:	// 証明書検証設定キー.
					if( True == PLBC_FuncPKICheck::isPKI() ) {		// ＰＫＩカード認証設定が有効の場合.
						nextGamen = pc_MenuUtilityAndAdmin.SPCprocGoToPage( option );
					} else {
						setBuzzerType( PCGE_BuzzerType_InputNg );
					}
					break;
				case 5:	// 拡張サーバーダウン時の上限.
					nextGamen = pc_MenuUtilityAndAdmin.SPCprocGoToPage( option );	// C092210000 拡張サーバーダウン時の上限画面へ遷移.
					break;
				case 6:		// 課金情報の更新.
					nextGamen = pc_MenuUtilityAndAdmin.SPCprocGoToPage( option );			// C092220000 課金情報の更新画面へ遷移.
					break;
				case 7:		// AD認証時自己検証設定
					nextGamen	= pc_MenuUtilityAndAdmin.SPCprocGoToPage( option );
// comment 2022/05/20 Sky西場 [IT6_2.3][OP_BTS-37620][スリム化].
//					gp_AdminNetworkSetSSOAuthDomainSet.m_bl_frommenu = True;
					break;
				case 11: PToAdminSetMenucom::procGoToPage( option );			break;	// 管理者設定キー.
				default:
					setBuzzerType( PCGE_BuzzerType_InputNg );		break;	// 報知音設定.
			}
		}
	}
	else {
		if( uc_page == 1 ) {	//P1の時.
			if( bl_BumonLimitWarningAdminMove == False ) {	// 通常時.
				switch( option ) {
					case  1:
						if( True == isAdminMachineDirectLogin() ) {
							setBuzzerType( PCGE_BuzzerType_InputNg );
							break;
						}
						b_fromMenu = True;			// ↑画面からの遷移.
						nextGamen = pc_MenuUtilityAndAdmin.SPCprocGoToPage( option );				// 管理者設定・ユーザ認証/部門管理・ユーザ/部門の連動認証
						break;
					case  2:	// ユーザ認証する場合、ユーザ認証画面へ遷移する.
// 	Comment 2017/09/28 [HCL-Harish]【IT5 4.2】【RQ-686】【WP509】Multiple Network support.
//						if( PnlComGetUserAuthentication()->getUserAuthentication() != TYPE_UserAuthenticationOff ) {
						if( PnlComGetUserAuthentication()->getUserAuthentication( PNLC_NetworkLan::getInstance() ->getAdminNWLine( ) ) != TYPE_UserAuthenticationOff ) {
							nextGamen = pc_MenuUtilityAndAdmin.SPCprocGoToPage( option );	// 管理者設定・ユーザ認証/部門管理・ユーザ認証
						} else {
							setBuzzerType( PCGE_BuzzerType_InputNg );	// 報知音設定
						}
						break;
					case  3:	// 部門管理する場合、部門管理画面へ遷移する.
						if( PnlComGetTrackManager()->getTrackMode( PNLC_NetworkLan::getInstance()->getAdminNWLine() ) != TYPE_TrackModeOff ) {
							nextGamen = pc_MenuUtilityAndAdmin.SPCprocGoToPage( option );		// 管理者設定・ユーザ認証/部門管理・部門管理
						} else {
							setBuzzerType( PCGE_BuzzerType_InputNg );	// 報知音設定
						}
						break;
					case  4: // 管理者設定・ユーザ認証/部門管理・認証指定なしプリント
						// 上階層からの遷移.
						PLBC_CheckFromMenu::getInstance()->setFromMenu( True );
						nextGamen = pc_MenuUtilityAndAdmin.SPCprocGoToPage( option );
						break;
					case  5:	// 使用管理カウンタリストキーへ遷移する.
						// ユーザ認証、部門管理共に行わない場合、使用管理カウンタリストキーは非表示なので、遷移しない.
						// ベンダー1、2、管理装置2、認証装置1、キーカウンター装着時は非表示.
// 	Comment 2017/09/28 [HCL-Harish]【IT5 4.2】【RQ-686】【WP509】Multiple Network support.
//						if( PnlComGetUserAuthentication()->getUserAuthentication() == TYPE_UserAuthenticationOff
//							&& PnlComGetTrackManager()->getTrackMode() == TYPE_TrackModeOff ) {
						if( PnlComGetUserAuthentication()->getUserAuthentication( PNLC_NetworkLan::getInstance() ->getAdminNWLine( ) ) == TYPE_UserAuthenticationOff
							&& PnlComGetTrackManager()->getTrackMode(PNLC_NetworkLan::getInstance() ->getAdminNWLine( )) == TYPE_TrackModeOff ) {
							// ユーザ認証OFFかつ、部門管理OFFの場合非表示.
							// 他の条件は上位の画面で制限.
							setBuzzerType( PCGE_BuzzerType_InputNg );		// 報知音設定
						} else {
							PAdminSystemDepartListPrintListPrint::e_ListPrint = TYPE_ListPrintTrackCounter;
							nextGamen = &gp_AdminSystemDepartListPrintUserCounterListPrint;	// 管理者設定 > ユｰザｰ認証/部門管理 > 使用管理カウンターリストプリント.
						}
						break;
					case  6: nextGamen = pc_MenuUtilityAndAdmin.SPCprocGoToPage( option );					break;		// 管理者モード・管理者設定・ユーザ認証/部門管理・外部サーバ認証.
					case  7:
						if( False == PnlComIsUseSsdScanSend() ) {	// HDD/SSD使用不可.
							setBuzzerType( PCGE_BuzzerType_InputNg );		// 報知音設定.
						} else {
							nextGamen = pc_MenuUtilityAndAdmin.SPCprocGoToPage( option );	// 管理者設定 > ユｰザｰ認証/部門管理 > 参照許可設定.
						}
						break; 		//参照許可設定	// 10921000 参照許可設定.
					case  8://認証装置設定
						if( b_EnableAuthDeviceSetting == True
// 	Comment 2017/09/28 [HCL-Harish]【IT5 4.2】【RQ-686】【WP509】Multiple Network support.
//						    ||( TYPE_UserAuthenticationIntermeddiateServer == PnlComGetUserAuthentication()->getUserAuthentication() 		// 中間認証.
//						        || TYPE_UserAuthenticationDeviceAndMidAuth    == PnlComGetUserAuthentication()->getUserAuthentication() ) ) {	// 本体+中間認証.
						    ||( TYPE_UserAuthenticationIntermeddiateServer == PnlComGetUserAuthentication()->getUserAuthentication( PNLC_NetworkLan::getInstance() ->getAdminNWLine( ) ) 		// 中間認証.
						        || TYPE_UserAuthenticationDeviceAndMidAuth    == PnlComGetUserAuthentication()->getUserAuthentication( PNLC_NetworkLan::getInstance() ->getAdminNWLine( ) ) ) ) {	// 本体+中間認証.
							nextGamen = pc_MenuUtilityAndAdmin.SPCprocGoToPage( option );		// 10920900 認証装置設定 .
						} else {	// 条件を満たしていない場合は報知音を鳴らして、遷移しない.
							setBuzzerType( PCGE_BuzzerType_InputNg );		// 報知音設定
						}
						break;
					case  9:	// ユーザー/部門共通設定キー.
						nextGamen = pc_MenuUtilityAndAdmin.SPCprocGoToPage( option );	// ユーザー/部門共通設定.
						break;
					case  10://Home宛先有効設定.
//						nextGamen = &gp_AdminUserAndDepartHomeAddressSetting;.
//#else.
//						if( True == c_HD_Equip.isOK() ){.
//							//HDD装着時は50922000 Home宛先有効設定画面へ遷移.
//							nextGamen = &gp_AdminUserAndDepartHomeAddressSetting;.
//						} else {.
//							//HDD非装着時は拒否音鳴動.
//							setBuzzerType( PCGE_BuzzerType_InputNg );.
//						}.
////						break;.
						if( False == PnlComIsUseSsdScanSend() ) {	// HDD/SSD使用不可.
							//HDD非装着時は拒否音鳴動.
							setBuzzerType( PCGE_BuzzerType_InputNg );		// 報知音設定.
						} else {
							nextGamen = pc_MenuUtilityAndAdmin.SPCprocGoToPage( option );	// 管理者設定 > ユｰザｰ認証/部門管理 > Home宛先有効設定.
						}
						break;
					case 11: PToAdminSetMenucom::procGoToPage( option );			break;	// 管理者設定キー
					default:
						setBuzzerType( PCGE_BuzzerType_InputNg );		break;	// 報知音設定
				}
			} else {	// 上限値オーバー発生時.
				switch( option ) {
					case  2:	// ユーザ認証する場合、ユーザ認証画面へ遷移する.
// 	Comment 2017/09/28 [HCL-Harish]【IT5 4.2】【RQ-686】【WP509】Multiple Network support.
//						if( PnlComGetUserAuthentication()->getUserAuthentication() != TYPE_UserAuthenticationOff )
						if( PnlComGetUserAuthentication()->getUserAuthentication( PNLC_NetworkLan::getInstance() ->getAdminNWLine( ) ) != TYPE_UserAuthenticationOff )
						{ nextGamen = pc_MenuUtilityAndAdmin.SPCprocGoToPage( option ); }	// 管理者設定・ユーザ認証/部門管理・ユーザ認証
						else
						{ setBuzzerType( PCGE_BuzzerType_InputNg ); }	// 報知音設定
						break;
					case  3:	// 部門管理する場合、部門管理画面へ遷移する.
						if( PnlComGetTrackManager()->getTrackMode( PNLC_NetworkLan::getInstance()->getAdminNWLine() ) != TYPE_TrackModeOff )
						{ nextGamen = pc_MenuUtilityAndAdmin.SPCprocGoToPage( option ); }		// 管理者設定・ユーザ認証/部門管理・部門管理
						else
						{ setBuzzerType( PCGE_BuzzerType_InputNg ); }	// 報知音設定
						break;
					case  10://Home宛先有効設定.
						nextGamen = pc_MenuUtilityAndAdmin.SPCprocGoToPage( option );		break;	// 50922000 Home宛先有効設定画面へ遷移.
					case 11: PToAdminSetMenucom::procGoToPage( option );			break;	// 管理者設定キー
//				default:	break;
					default:	setBuzzerType( PCGE_BuzzerType_InputNg );		break;	// 報知音設定
				}
			}
		} else {	//P2のとき.
			switch( option ) {
				case 1://送信宛先制限.
					if( True == checkNextPageContents() ) {
						// HDD装着時50922100 送信宛先制限画面へ遷移.
						nextGamen = pc_MenuUtilityAndAdmin.SPCprocGoToPage( option );
					} else {
						// HDD非装着時拒否音鳴動.
						setBuzzerType( PCGE_BuzzerType_InputNg );
					}
					break;

				case 2:
//					setBuzzerType( PCGE_BuzzerType_InputNg );
//				} else {
//				|| ( True == PLBC_FuncPKICheck::isPKI() ) ) {		// ＰＫＩカード認証設定が有効の場合.
					if( True == PLBC_FuncPKICheck::isPKI() ) {		// ＰＫＩカード認証設定が有効の場合.
						setBuzzerType( PCGE_BuzzerType_InputNg );
					} else {
						nextGamen = pc_MenuUtilityAndAdmin.SPCprocGoToPage( option );
					}
					break;

				case 3:
					if( True == PLBC_FuncPKICheck::isPKI() ) {		// ＰＫＩカード認証設定が有効の場合.
						nextGamen = pc_MenuUtilityAndAdmin.SPCprocGoToPage( option );
					} else {
						setBuzzerType( PCGE_BuzzerType_InputNg );
					}
					break;
				case 4:	// 拡張サーバーダウン時の上限.
					nextGamen = pc_MenuUtilityAndAdmin.SPCprocGoToPage( option );	// C092210000 拡張サーバーダウン時の上限画面へ遷移.
					break;
				case 5:		// 課金情報の更新.
					nextGamen = pc_MenuUtilityAndAdmin.SPCprocGoToPage( option );			// C092220000 課金情報の更新画面へ遷移.
					break;
				case 11: PToAdminSetMenucom::procGoToPage( option );			break;	// 管理者設定キー.
				default:
					setBuzzerType( PCGE_BuzzerType_InputNg );		break;	// 報知音設定.
			}
		}
	}
}
//================================================================================================.
// 概要		:管理者設定・ユーザ認証/部門管理(2690) : ボタン（押された）.
// 戻り値	:なし.
//================================================================================================.
void PAdminUserAndDepartSet::procButtonOn( uchar option )
{
	if( APIC_KeyDevice::getKeyMode() == TYPE_KeyModePageAcses ) {
		// PageACSES時はユーザー/部門共通設定以外は表示しない.
		setBuzzerType( PCGE_BuzzerType_InputNg );
		setupGamen();
		return;
	}
	switch( option ) {
		case 0:		// 前キー.
		case 1:		// 次キー.
		case 30:	// アスタリスクキー(前)キー.
		case 31:	// シャープキー(次)キー.
			if( True == checkNextPageContents() ) {
				//HDD装着時2ページ目に遷移.
				uc_page = ( uc_page == 1 ? 2 : 1 );
				setPageParts();
			} else {
				//HDD非装着時拒否音鳴動.
				setBuzzerType( PCGE_BuzzerType_InputNg );
			}
			break;
		default:
			setBuzzerType( PCGE_BuzzerType_InputNg );	break;	// 報知音設定.
	}
	setupGamen();
}
//================================================================================================.
// 概要		:管理者設定・ユーザ認証/部門管理(2690) : テンキー.
// 戻り値	:なし.
//================================================================================================.
void PAdminUserAndDepartSet::proc10Key( uchar option )
{
	if( 0 == option ) {
		option	= 10;
	}
	procGoToPage( option );
}
//================================================================================================
// 概要		:管理者設定・ユーザ認証/部門管理(2690) : 表示画面のセットアップ
// 戻り値	:なし
//================================================================================================
void PAdminUserAndDepartSet::setupGamen( short option )
{
	Bool bl_EnableHDDExistFunc = True;				// HDD状態による機能有効フラグ.
// Comment 2018/01/29 HCLT_Hanuman [IT5_4.2][AR1038061] To improve performance.
//// Comment 2018/01/03 [HCL-Shalini]【IT5 4.2】【RQ-686】【WP1382,1384,1386,1388,1390】【AR1035279】Sharing Common Icon Image for Network 1&Network 2 Settings.
//	if( True == PnlComGetVlanSetting()->getVlanEnable() ){
	if( True == PNLC_NetworkLan::getInstance()->getVlanEnableWrapper() ){
		Txt_UtilityMenuMessage2.showEnable( False );
		Txt_UtilityMenuSharedMessage.showEnable( True );
	} else {
		Txt_UtilityMenuSharedMessage.showEnable( False );
		Txt_UtilityMenuMessage2.showEnable( True );
	}
	if( False == PnlComIsUseSsdScanSend() ) {	// HDD/SSD使用不可.
		bl_EnableHDDExistFunc = False;
	}
	//一度全てのパーツを消去.
	Bool	b_Active[PNLD_MENUDATANUM];
	Bool	b_Show[PNLD_MENUDATANUM];
	for( int i = 0; i < PNLD_MENUDATANUM; i++ ) {
		b_Show[i]	= False;
		b_Active[i]	= True;
	}
	Nfd_AdminUserAndDepartComDspPage = uc_page;					// 現ページ設定値.
	Nfd_AdminUserAndDepartComTotalPage = 2;						// 総ページ設定値.
// Comment 2015/08/01 HCL_Jeevitha【ZeusS_ZX0】【RQ-261】Added Display/hide condition for URL Address Setting key.
	if( True == PLBC_FuncVersionCheck::isOK_IT5_Type4() ) {
//Comment 2017/09/04 [HCL-Harish] [IT5 4.2][RQ-686] [WP509] Multiple Network support.
	TYPE_NetworkSpecific e_NetworkSpecific  = PNLC_NetworkLan::getInstance()->getAdminNWLine( ) ;
		if( uc_page == 1 ) {	//P1のとき.
			// No309508対応.
			// 選択不可時、ユーザー認証キー、部門管理キーグレーアウト.
			Bool _bl_ActiveUserAttest = True;		// ユーザー認証活性状態.
			Bool _bl_ActiveDepartManage = True;		// 部門管理活性状態.
//			if( TYPE_UserAuthenticationOff == PnlComGetUserAuthentication()->getUserAuthentication() ) {	// ユーザー認証しない時.
			if( TYPE_UserAuthenticationOff == PnlComGetUserAuthentication()->getUserAuthentication( e_NetworkSpecific ) ) {	// ユーザー認証しない時.
				_bl_ActiveUserAttest = False;	// グレーアウト.
			}
//			if( TYPE_TrackModeOff == PnlComGetTrackManager()->getTrackMode() ) {	// 部門管理しない時.
			if( TYPE_TrackModeOff == PnlComGetTrackManager()->getTrackMode( e_NetworkSpecific ) ) {	// 部門管理しない時.
				_bl_ActiveDepartManage = False;	// グレーアウト.
			}
			b_Active[1]	= _bl_ActiveUserAttest;		// ユーザー認証.
			b_Active[2]	= _bl_ActiveDepartManage;	// 部門管理.
			
			APIC_AuthenticationSeverSetting* apicAuthSrv = APIC_AuthenticationSeverSetting::newInstance();
			if( NULL == apicAuthSrv ) {
// comment [2016/12/22] [PDD22山下(日立)] [IT5_4.0][AccuRev.1009508]認証拡張FUM追加コードに対して新詳細コードを追加.
//Comment 2019/06/14 [HCLT_Monisha][EagleH][N Rank] Removed unwanted Detailed code.
//				LAPC_BFW_System *pc_BFW_System = LAPC_BFW_System::newInstance();
//				if (NULL != pc_BFW_System){    
//				    pc_BFW_System ->registDetailCode( PNLD_DetailCode_Admin_0x19006600104E0401 );
//				}
				LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" ); 
				return;
			}
			// サービス管理機能チョイス設定が「認証装置２」で、HDD接続、ユーザー認証しているとき.
			const TYPE_HddStatus e_HddStatus = PnlComGetHddInfo()->getHddStatus();
			if( TYPE_HddStatusConnecting   != e_HddStatus                                       &&
			    TYPE_HddStatusNon          != e_HddStatus                                       &&
			    TYPE_AUTH_DEVICE_NONE      != PnlComGetAuthDeviceSetting()->getAuthDeviceType() &&
			    TYPE_KeyModeNone           == APIC_KeyDevice::getKeyMode()				       &&
//			    TYPE_UserAuthenticationOff != PnlComGetUserAuthentication()->getUserAuthentication() ) {
			    TYPE_UserAuthenticationOff != PnlComGetUserAuthentication()->getUserAuthentication( e_NetworkSpecific ) ) {
				if( True == apicAuthSrv->checkServerGroup() ) {
					// サーバーグループが有効な場合.
					// ローダブルドライバが使用可能になるまで非活性とする.
// comment 2016/12/20 PDD22 山下(日立) [IT5_4.0][AccuRev.1009671]PKI有効時は認証先切り替えが無効のため、起動直後の選択抑制を行なわないよう修正.
					if( ( False == PnlComGetAuthDeviceCommon()->isAuthModuleIdleCheck() ) 
						&& ( False == PLBC_FuncPKICheck::isPKI() ) ) {
						b_Active[0]	=  False;		// 認証方式.
						b_Active[1]	=  False;		// ユーザー認証.
						b_Active[6]	=  False;		// 優先/代替サーバー接続状態..
					}
				}
			}
			if( bl_BumonLimitWarningAdminMove == False ) {	// 通常時.
				if( APIC_KeyDevice::getKeyMode() == TYPE_KeyModePageAcses ) {
					// PageACSES接続時にはユーザー/部門共通設定以外は非表示にする.
					b_Show[0]	= False;	// 認証方式.
					b_Show[1]	= False;	// ユーザー認証設定.
					b_Show[2]	= False;	// 部門管理設定.
					b_Show[3]	= False;	// 認証指定なし印刷.
					b_Show[4]	= False;	// 使用管理カウンターリスト.
					b_Show[5]	= False;	// 外部サーバー設定.
					b_Show[6]	= False;	// 優先/代替サーバー接続状態.
					b_Show[7]	= False;	// 参照許可設定.
					b_Show[8]	= False;	// 認証装置設定.
					Nfd_AdminUserAndDepartComDspPage.showEnable( False );					// 現ページ数.
					Nfd_AdminUserAndDepartComTotalPage.showEnable( False );					// 総ページ数.
					Bak_AdminUserAndDepartComPageDsp.showEnable( False );					// ページ表示エリア.
					Key_AdminUserAndDepartComPagePrev.showEnable( False );					// 前画面キー.
					Key_AdminUserAndDepartComPageNext.showEnable( False );					// 次画面キー.
					Key_AdminUserAndDepartComPagePrevAster.showEnable( False );				// [*]キー.
					Key_AdminUserAndDepartComPageNextSharp.showEnable( False );				// [#]キー.
				} else {
					b_Show[0]	= ( True == isAdminMachineDirectLogin() ? False : True );	// 認証方式.
					b_Show[1]	= True;		// ユーザ認証.
					b_Show[2]	= True;		// 部門管理.
					b_Show[3]	= True;		// 認証指定なしプリント.
					b_Show[5]	= True;		// 外部サーバー設定キー.
// comment 2016/5/25 [AUTHFUM2] [RQ-523]外部サーバーグループ認証 第22製品開発 山本.					
					// 認証拡張FUM以降、「外部サーバー認証」または「本体＋外部サーバー」の場合以外は非表示.
//Comment 2017/1/11 【HCLT-Ajit】【IT5_4.0】 【Accurev ID:1010410】 Commented the First condition to Hide/Display the Primary/Secondary Server Connection Status Key when DipSwitch Set 221 , 2.
//					APIC_AuthenticationSeverSetting* apicAuthSrv = APIC_AuthenticationSeverSetting::newInstance();
//					if( NULL == apicAuthSrv ) {
//						LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
//						return ;
//					}
//					if( ( True == apicAuthSrv->checkServerGroup() )
//						&& ( TYPE_UserAuthenticationServer != PnlComGetUserAuthentication()->getUserAuthentication() )
//						&& ( TYPE_UserAuthenticationDeviceAndServer != PnlComGetUserAuthentication()->getUserAuthentication() ) ) {
					if ( ( True == PLBC_FuncVersionCheck::isOK_IT5_Type9() )
//						&& ( TYPE_UserAuthenticationServer != PnlComGetUserAuthentication()->getUserAuthentication() )
//						&& ( TYPE_UserAuthenticationDeviceAndServer != PnlComGetUserAuthentication()->getUserAuthentication() ) ) {
						&& ( TYPE_UserAuthenticationServer != PnlComGetUserAuthentication()->getUserAuthentication( e_NetworkSpecific ) )
						&& ( TYPE_UserAuthenticationDeviceAndServer != PnlComGetUserAuthentication()->getUserAuthentication( e_NetworkSpecific ) ) ) {
						b_Show[6]	= False;	// 優先／代替サーバー接続状態キー.
					} else {
						b_Show[6]	= True;		// 優先／代替サーバー接続状態キー.
					}
					
					// サービス管理機能チョイス設定が「認証装置２」で、ユーザー認証しているときのみ表示する.
//						TYPE_KeyModeNone           == APIC_KeyDevice::getKeyMode()                      &&
					if(	TYPE_AUTH_DEVICE_NONE      != PnlComGetAuthDeviceSetting()->getAuthDeviceType() &&
					    TYPE_KeyModeNone           == APIC_KeyDevice::getKeyMode() ) {
						// 認証装置設定キー表示.
						b_EnableAuthDeviceSetting = True;
					} else {
						// 認証装置設定キー非表示.
						b_EnableAuthDeviceSetting = False;
					}
					if( True == onlyPanel.JudgeModel( TYPE_FunctionTypeLow ) ) {	// Lowモデル時.
						b_EnableAuthDeviceSetting = False;
					}
//					b_Show[7]	= bl_EnableHDDExistFunc;	// 参照許可設定キー.
//Comment 2017/09/04 [HCL-Harish] [IT5 4.2][RQ-686] [WP533] Multiple Network support - not applicable for Network2.
					if ( TYPE_NetworkSpecific_NW1 == PNLC_NetworkLan::getInstance()->getAdminNWLine() ) {
						b_Show[7]	= bl_EnableHDDExistFunc;	// 参照許可設定キー.
					} else {
						b_Show[7]	= False;					// 参照許可設定キー.
					}
					b_Show[8]	= b_EnableAuthDeviceSetting;	// 認証装置設定キー.
					// ユーザ認証、部門管理共に行わない場合、使用管理カウンタリストキーを非表示.
					// ベンダー1、2、管理装置2、認証装置1、キーカウンター装着時は非表示.
//					if( PnlComGetUserAuthentication()->getUserAuthentication() == TYPE_UserAuthenticationOff
//					    && PnlComGetTrackManager()->getTrackMode() == TYPE_TrackModeOff )
					if( PnlComGetUserAuthentication()->getUserAuthentication( e_NetworkSpecific ) == TYPE_UserAuthenticationOff
					    && PnlComGetTrackManager()->getTrackMode( e_NetworkSpecific ) == TYPE_TrackModeOff )
						// ユーザ認証OFFかつ、部門管理OFFの場合非表示.
						// 他の条件は上位の画面で制限.
					{ b_Show[4]	= False; }
					else
					{ b_Show[4]	= True; }
					Bool bl_NextPageContent = checkNextPageContents();
					Nfd_AdminUserAndDepartComDspPage.showEnable( bl_NextPageContent );					// 現ページ数.
					Nfd_AdminUserAndDepartComTotalPage.showEnable( bl_NextPageContent );				// 総ページ数.
					Bak_AdminUserAndDepartComPageDsp.showEnable( bl_NextPageContent );					// ページ表示エリア.
					Key_AdminUserAndDepartComPagePrev.showEnable( bl_NextPageContent );					// 前画面キー.
					Key_AdminUserAndDepartComPageNext.showEnable( bl_NextPageContent );					// 次画面キー.
					Key_AdminUserAndDepartComPagePrevAster.showEnable( bl_NextPageContent );			// [*]キー.
					Key_AdminUserAndDepartComPageNextSharp.showEnable( bl_NextPageContent );			// [#]キー.
				}
#ifdef DEF_MonoCus
				Bool bl_LogoutCheckDisp 		= True;			// 50921000_ユーザー／部門共通設定画面の「ログアウト確認画面表示設定」キー.
				Bool bl_RemoteCounterCheckDisp 	= True;			// 50921000_ユーザー／部門共通設定画面の「遠隔カウンター管理」キー.

				// ユーザ認証=なし　＆　部門認証=なし　の場合は、ログアウト確認画面キーは非表示.
//				if( ( TYPE_UserAuthenticationOff == PnlComGetUserAuthentication()->getUserAuthentication() )
//				    && ( TYPE_TrackModeOff == PnlComGetTrackManager()->getTrackMode() ) )
				if( ( TYPE_UserAuthenticationOff == PnlComGetUserAuthentication()->getUserAuthentication( e_NetworkSpecific ) )
				    && ( TYPE_TrackModeOff == PnlComGetTrackManager()->getTrackMode( e_NetworkSpecific ) ) )
				{
					bl_LogoutCheckDisp = False;
				}
				// 本体認証、部門管理、外部サーバー認証 本体認証+外部サーバー認証 利用時以外は遠隔カウンター管理キーを非表示.
//				if( TYPE_UserAuthenticationDevice			!= PnlComGetUserAuthentication()->getUserAuthentication()
//				    &&  TYPE_UserAuthenticationDeviceAndMidAuth	!= PnlComGetUserAuthentication()->getUserAuthentication()
//				    &&  TYPE_UserAuthenticationServer			!= PnlComGetUserAuthentication()->getUserAuthentication()
//				    &&  TYPE_UserAuthenticationDeviceAndServer	!= PnlComGetUserAuthentication()->getUserAuthentication()
//				    &&  TYPE_TrackModeOff						== PnlComGetTrackManager()->getTrackMode() )	{
				if( TYPE_UserAuthenticationDevice			!= PnlComGetUserAuthentication()->getUserAuthentication( e_NetworkSpecific )
				    &&  TYPE_UserAuthenticationDeviceAndMidAuth	!= PnlComGetUserAuthentication()->getUserAuthentication( e_NetworkSpecific )
				    &&  TYPE_UserAuthenticationServer			!= PnlComGetUserAuthentication()->getUserAuthentication( e_NetworkSpecific )
				    &&  TYPE_UserAuthenticationDeviceAndServer	!= PnlComGetUserAuthentication()->getUserAuthentication( e_NetworkSpecific )
				    &&  TYPE_TrackModeOff						== PnlComGetTrackManager()->getTrackMode( e_NetworkSpecific ) )	{
					bl_RemoteCounterCheckDisp = False;
				}
				if( False == bl_LogoutCheckDisp && False == bl_RemoteCounterCheckDisp ) {
					// 50921000_ユーザー／部門共通設定画面に表示項目が無い場合は非表示.
					b_Show[9]	= False;
				} else {
					b_Show[9]	= True;
				}
#else //DEF_MonoCus.
				b_Show[9]	= True;		// ユーザー/部門共通設定キー.
#endif //DEF_MonoCus.
			} else {	// 上限値オーバー発生時.
				b_Show[0]	= False;	// 認証方式.
				b_Show[1]	= False;	// ユーザー認証設定.
				b_Show[2]	= False;	// 部門管理設定.
				b_Show[3]	= False;	// 認証指定なしプリント
				b_Show[4]	= False;	// 使用管理者カウンタリスト.
				b_Show[5]	= False;	// 外部サーバー設定キー.
				b_Show[6]	= False;	// 優先/代替サーバー接続状態.
				b_Show[7]	= False;	// 参照許可設定.
				b_Show[8]	= False;	// 認証装置設定.
				b_Show[9]	= False;	// ユーザー/部門共通設定.
//				if( PnlComGetUserAuthentication()->getUserAuthentication() != TYPE_UserAuthenticationOff )
				if( PnlComGetUserAuthentication( )->getUserAuthentication( e_NetworkSpecific ) != TYPE_UserAuthenticationOff )
				{ b_Show[1]	= True; }		// ユーザ認証有り時、「ユーザ認証」キーを表示.
//				if( PnlComGetTrackManager()->getTrackMode() != TYPE_TrackModeOff )
				if( PnlComGetTrackManager()->getTrackMode( e_NetworkSpecific ) != TYPE_TrackModeOff )
				{ b_Show[2]	= True; }		// 部門管理認証有り時、「部門管理」キーを表示.
			}
//			if( TYPE_UserAuthenticationIntermeddiateServer == PnlComGetUserAuthentication()->getUserAuthentication() 		// 中間認証.
			if( TYPE_UserAuthenticationIntermeddiateServer == PnlComGetUserAuthentication()->getUserAuthentication( e_NetworkSpecific ) 		// 中間認証.
// comment 2017/10/31 【HCL-TANU】【IT5_4.2】【RQ686】【IT Defect AR1030817].
//			    || True == PnlComGetUserAuthentication()->getMidauthControl() ) {
			    || True == PnlComGetUserAuthentication()->getMidauthControl(e_NetworkSpecific) ) {
				b_Show[2]	= False;	// 部門管理設定.
				if( bl_BumonLimitWarningAdminMove == False ) {
					// 通常時.
					b_Show[8]	= True;		// 認証装置設定.
				}
			}
//			if ( TYPE_UserAuthenticationDeviceAndMidAuth == PnlComGetUserAuthentication()->getUserAuthentication()) {		// 本体+中間認証.
			if ( TYPE_UserAuthenticationDeviceAndMidAuth == PnlComGetUserAuthentication()->getUserAuthentication( e_NetworkSpecific )) {		// 本体+中間認証.
				b_Show[2]	= True;		// 部門管理設定.
				if( bl_BumonLimitWarningAdminMove == False ) {
					// 通常時.
					b_Show[8]	= True;		// 認証装置設定.
				}
			}
			APIC_TabletIF* pc_TabletIF = APIC_TabletIF::newInstance();
			if( ( NULL != pc_TabletIF ) && ( True == pc_TabletIF->getConnectState() ) ) {
				b_Show[8]	= False;	// 認証装置設定キー.
			}
// Commment 2018/02/23 HCLT_Vinod [IT5_4.2][AR1041201][FS_SS Miss match] Hiding Account track settings key for NW2.
			if( TYPE_NetworkSpecific_NW2 == PNLC_NetworkLan::getInstance()->getAdminNWLine() ){
				b_Show[2]	= False;	// 部門管理設定.
			}
			setupPageParts( b_Show, b_Active );
			PToAdminSetMenucom::setupGamen( option );
		} else {	//P2のとき.
//Comment 2017/10/27 [HCL-Suganya] [IT5 4.2][RQ-686] [WP165]Multiple Network support.
//			if ( True == c_HD_Equip.isOK() ){  // HDD is installed.
			if ( True == c_HD_Equip.isOK() && ( TYPE_NetworkSpecific_NW2 != PNLC_NetworkLan::getInstance()->getAdminNWLine() ) ) {  // HDD is installed.
				b_Show[0]	= True;
			} else {
				b_Show[0]	= False;
            }
			b_Show[2]	= checkNextPageContents();		//送信宛先制限キー.
			if( True == PLBC_FuncPKICheck::isPKI() ) {		// ＰＫＩカード認証設定が有効の場合.
				b_Show[4]	= True;		// 証明書検証設定.
			} else {
				b_Show[4]	= False;	// 証明書検証設定.
			}

// comment 2016/12/07 Sky東島 [特ROM][AU DoD].
//			if( True == PLBC_FuncPKICheck::isPKI() ) {		// ＰＫＩカード認証設定が有効の場合.
			if( ( True == PLBC_FuncPKICheck::isPKI() )		// ＰＫＩカード認証設定が有効かつAU DODモードではない場合.
			 && ( False == isContactlessPKICard() ) ) {
				b_Show[3]	= False;	// プリント簡易認証.
			} else {
				b_Show[3]	= True;		// プリント簡易認証.
				b_Active[3]	= True;
			}
			if( True == PLBC_FuncVersionCheck::isOK_MiZe_Type2() ) {
				TYPS_ExtDetailAuthentication* ps_DetailAuth = PCGC_AuthOapAPIAccess::getAuthenticationAPI();	// Authアプリ詳細情報取得.
				APIC_UserAuthentication* pc_UserAuth = PnlComGetUserAuthentication();							// ユーザー認証情報取得.
				// 拡張サーバーダウン時の上限キーの表示/非表示.
				if( ( NULL != ps_DetailAuth ) && ( NULL != pc_UserAuth )
				    && ( TYPE_ExtCounterManagementEnable == ps_DetailAuth->s_Function.e_CounterManagement )		// カウンタ管理設定「有効」.
				    && ( TYPE_ExtCounterLimitEnable == ps_DetailAuth->s_Function.e_CounterLimit )					// カウンタ上限管理設定「有効」.
//				    && ( True == pc_UserAuth->getAuthCacheMode() )												// 認証情報の一時保存設定「する」.
//				    && ( ( TYPE_UserAuthenticationIntermeddiateServer == pc_UserAuth->getUserAuthentication() )	// ユーザー認証方式「拡張サーバー認証」or「本体+拡張サーバー」.
//				         || ( TYPE_UserAuthenticationDeviceAndMidAuth == pc_UserAuth->getUserAuthentication() ) ) ) {
				    && ( True == pc_UserAuth->getAuthCacheMode( e_NetworkSpecific ) )												// 認証情報の一時保存設定「する」.
				    && ( ( TYPE_UserAuthenticationIntermeddiateServer == pc_UserAuth->getUserAuthentication( e_NetworkSpecific ) )	// ユーザー認証方式「拡張サーバー認証」or「本体+拡張サーバー」.
// Commment 2018/02/23 HCLT_Vinod [IT5_4.2][AR1041201][FS_SS Miss match] Hiding 拡張サーバーダウン時の上限 key for NW2.
//				         || ( TYPE_UserAuthenticationDeviceAndMidAuth == pc_UserAuth->getUserAuthentication( e_NetworkSpecific ) ) ) ) {
						|| ( TYPE_UserAuthenticationDeviceAndMidAuth == pc_UserAuth->getUserAuthentication( e_NetworkSpecific ) ) ) 
					&& ( TYPE_NetworkSpecific_NW2 != PNLC_NetworkLan::getInstance()->getAdminNWLine() ) ) {
					b_Show[5]	= True;		// 拡張サーバーダウン時の上限.
				} else {
					b_Show[5]	= False;	// 拡張サーバーダウン時の上限.
				}
				// 課金情報の更新キーの表示/非表示.
				if( ( NULL != ps_DetailAuth )
				    && ( TYPE_ExtCounterManagementEnable == ps_DetailAuth->s_Function.e_CounterManagement ) 	// カウンタ管理設定「有効」.
// Comment 2018/01/29 [HCL-Narsimhmaswamy] [IT5 4.2][AR1033051_Reopen] When VLAN  Enabled  hide Update Billing Inforamtion Key.
////Comment 2018/01/10 [HCL-KanuriVenkata] [IT5 4.2][AR1033051] When VLAN  Enabled  hide Update Billing Inforamtion Key
//					&& False == PnlComGetVlanSetting()->judgeVlanEnable()) {	// カウンタ管理設定「有効」.
					&& False == PNLC_NetworkLan::getInstance()->getVlanEnableWrapper() ) {
					b_Show[6]	= True;		// 課金情報の更新.
				} else {
					b_Show[6]	= False;	// 課金情報の更新.
				}
			} else {
				b_Show[5]	= False;	// 拡張サーバーダウン時の上限.
				b_Show[6]	= False;	// 課金情報の更新.
			}
			if( bl_BumonLimitWarningAdminMove == False ) {	// 通常時.
				b_Show[1]	= bl_EnableHDDExistFunc;	// Home宛先有効設定キー.
			}
			b_Show[7]	= True;		// AD認証時自己検証設定.
//Comment 2017/11/27 [HCLT_Suribabu][IT5_4.2][AR1033302] Hiding the SelffVerification Settings for AD Authentication Settings for Network 2.
			TYPE_NetworkSpecific e_NetworkSpecific  = PNLC_NetworkLan::getInstance()->getAdminNWLine();
			if( TYPE_NetworkSpecific_NW2 == e_NetworkSpecific ) {
				b_Show[7]	= False;
			}
			setupPageParts( b_Show, b_Active );
			PToAdminSetMenucom::setupGamen( option );
		}
	}
//	if( True == PLBC_FuncVersionCheck::isOK_IT5_Type1() ) {
	else if( True == PLBC_FuncVersionCheck::isOK_IT5_Type1() ) {
		if( uc_page == 1 ) {	//P1のとき.
			// No309508対応.
			// 選択不可時、ユーザー認証キー、部門管理キーグレーアウト.
			Bool _bl_ActiveUserAttest = True;		// ユーザー認証活性状態.
			Bool _bl_ActiveDepartManage = True;		// 部門管理活性状態.
// 	Comment 2017/09/28 [HCL-Harish]【IT5 4.2】【RQ-686】【WP509】Multiple Network support.
//			if( TYPE_UserAuthenticationOff == PnlComGetUserAuthentication()->getUserAuthentication() ) {	// ユーザー認証しない時.
			if( TYPE_UserAuthenticationOff == PnlComGetUserAuthentication()->getUserAuthentication( PNLC_NetworkLan::getInstance() ->getAdminNWLine( ) ) ) {	// ユーザー認証しない時.
				_bl_ActiveUserAttest = False;	// グレーアウト.
			}
			if( TYPE_TrackModeOff == PnlComGetTrackManager()->getTrackMode( PNLC_NetworkLan::getInstance()->getAdminNWLine() ) ) {	// 部門管理しない時.
				_bl_ActiveDepartManage = False;	// グレーアウト.
			}
			b_Active[1]	= _bl_ActiveUserAttest;		// ユーザー認証.
			b_Active[2]	= _bl_ActiveDepartManage;	// 部門管理.
			if( bl_BumonLimitWarningAdminMove == False ) {	// 通常時.
				if( APIC_KeyDevice::getKeyMode() == TYPE_KeyModePageAcses ) {
					// PageACSES接続時にはユーザー/部門共通設定以外は非表示にする.
					b_Show[0]	= False;	// 認証方式.
					b_Show[1]	= False;	// ユーザー認証設定.
					b_Show[2]	= False;	// 部門管理設定.
					b_Show[3]	= False;	// 認証指定なし印刷.
					b_Show[4]	= False;	// 使用管理カウンターリスト.
					b_Show[5]	= False;	// 外部サーバー設定.
					b_Show[6]	= False;	// 優先/代替サーバー接続状態.
					b_Show[7]	= False;	// 参照許可設定.
					b_Show[8]	= False;	// 認証装置設定.
					Nfd_AdminUserAndDepartComDspPage.showEnable( False );					// 現ページ数.
					Nfd_AdminUserAndDepartComTotalPage.showEnable( False );					// 総ページ数.
					Bak_AdminUserAndDepartComPageDsp.showEnable( False );					// ページ表示エリア.
					Key_AdminUserAndDepartComPagePrev.showEnable( False );					// 前画面キー.
					Key_AdminUserAndDepartComPageNext.showEnable( False );					// 次画面キー.
					Key_AdminUserAndDepartComPagePrevAster.showEnable( False );				// [*]キー.
					Key_AdminUserAndDepartComPageNextSharp.showEnable( False );				// [#]キー.
				} else {
					b_Show[0]	= ( True == isAdminMachineDirectLogin() ? False : True );	// 認証方式.
					b_Show[1]	= True;		// ユーザ認証.
					b_Show[2]	= True;		// 部門管理.
					b_Show[3]	= True;		// 認証指定なしプリント.
					b_Show[5]	= True;		// 外部サーバー設定キー.
					b_Show[6]	= True;		// 優先／代替サーバー接続状態キー.
					// サービス管理機能チョイス設定が「認証装置２」で、ユーザー認証しているときのみ表示する.
//						TYPE_KeyModeNone           == APIC_KeyDevice::getKeyMode()                      &&
					if(	TYPE_AUTH_DEVICE_NONE      != PnlComGetAuthDeviceSetting()->getAuthDeviceType() &&
					    TYPE_KeyModeNone           == APIC_KeyDevice::getKeyMode() ) {
						// 認証装置設定キー表示.
						b_EnableAuthDeviceSetting = True;
					} else {
						// 認証装置設定キー非表示.
						b_EnableAuthDeviceSetting = False;
					}
					if( True == onlyPanel.JudgeModel( TYPE_FunctionTypeLow ) ) {	// Lowモデル時.
						b_EnableAuthDeviceSetting = False;
					}
					b_Show[7]	= bl_EnableHDDExistFunc;	// 参照許可設定キー.
					b_Show[8]	= b_EnableAuthDeviceSetting;	// 認証装置設定キー.
					// ユーザ認証、部門管理共に行わない場合、使用管理カウンタリストキーを非表示.
					// ベンダー1、2、管理装置2、認証装置1、キーカウンター装着時は非表示.
// 	Comment 2017/09/28 [HCL-Harish]【IT5 4.2】【RQ-686】【WP509】Multiple Network support.
//					if( PnlComGetUserAuthentication()->getUserAuthentication() == TYPE_UserAuthenticationOff
//					    && PnlComGetTrackManager()->getTrackMode() == TYPE_TrackModeOff )
					if( PnlComGetUserAuthentication()->getUserAuthentication( PNLC_NetworkLan::getInstance() ->getAdminNWLine( ) ) == TYPE_UserAuthenticationOff
					    && PnlComGetTrackManager()->getTrackMode(PNLC_NetworkLan::getInstance() ->getAdminNWLine( )) == TYPE_TrackModeOff )
						// ユーザ認証OFFかつ、部門管理OFFの場合非表示.
						// 他の条件は上位の画面で制限.
					{ b_Show[4]	= False; }
					else
					{ b_Show[4]	= True; }
					Bool bl_NextPageContent = checkNextPageContents();
					Nfd_AdminUserAndDepartComDspPage.showEnable( bl_NextPageContent );					// 現ページ数.
					Nfd_AdminUserAndDepartComTotalPage.showEnable( bl_NextPageContent );				// 総ページ数.
					Bak_AdminUserAndDepartComPageDsp.showEnable( bl_NextPageContent );					// ページ表示エリア.
					Key_AdminUserAndDepartComPagePrev.showEnable( bl_NextPageContent );					// 前画面キー.
					Key_AdminUserAndDepartComPageNext.showEnable( bl_NextPageContent );					// 次画面キー.
					Key_AdminUserAndDepartComPagePrevAster.showEnable( bl_NextPageContent );			// [*]キー.
					Key_AdminUserAndDepartComPageNextSharp.showEnable( bl_NextPageContent );			// [#]キー.
				}
#ifdef DEF_MonoCus
				Bool bl_LogoutCheckDisp 		= True;			// 50921000_ユーザー／部門共通設定画面の「ログアウト確認画面表示設定」キー.
				Bool bl_RemoteCounterCheckDisp 	= True;			// 50921000_ユーザー／部門共通設定画面の「遠隔カウンター管理」キー.

				// ユーザ認証=なし　＆　部門認証=なし　の場合は、ログアウト確認画面キーは非表示.
// 	Comment 2017/09/28 [HCL-Harish]【IT5 4.2】【RQ-686】【WP509】Multiple Network support.
//				if( ( TYPE_UserAuthenticationOff == PnlComGetUserAuthentication()->getUserAuthentication() )
//						&& ( TYPE_TrackModeOff == PnlComGetTrackManager()->getTrackMode() ) )
				if( ( TYPE_UserAuthenticationOff == PnlComGetUserAuthentication()->getUserAuthentication( PNLC_NetworkLan::getInstance() ->getAdminNWLine( ) ) )
						&& ( TYPE_TrackModeOff == PnlComGetTrackManager()->getTrackMode(PNLC_NetworkLan::getInstance() ->getAdminNWLine( )) ) )
				{
					bl_LogoutCheckDisp = False;
				}
				// 本体認証、部門管理、外部サーバー認証 本体認証+外部サーバー認証 利用時以外は遠隔カウンター管理キーを非表示.
// 	Comment 2017/09/28 [HCL-Harish]【IT5 4.2】【RQ-686】【WP509】Multiple Network support.
//				if( TYPE_UserAuthenticationDevice			!= PnlComGetUserAuthentication()->getUserAuthentication()
//				    &&  TYPE_UserAuthenticationDeviceAndMidAuth	!= PnlComGetUserAuthentication()->getUserAuthentication()
//				    &&  TYPE_UserAuthenticationServer			!= PnlComGetUserAuthentication()->getUserAuthentication()
//				    &&  TYPE_UserAuthenticationDeviceAndServer	!= PnlComGetUserAuthentication()->getUserAuthentication()
//					&&  TYPE_TrackModeOff						== PnlComGetTrackManager()->getTrackMode() )	{
				if( TYPE_UserAuthenticationDevice			!= PnlComGetUserAuthentication()->getUserAuthentication( PNLC_NetworkLan::getInstance() ->getAdminNWLine( ) )
				    &&  TYPE_UserAuthenticationDeviceAndMidAuth	!= PnlComGetUserAuthentication()->getUserAuthentication( PNLC_NetworkLan::getInstance() ->getAdminNWLine( ) )
				    &&  TYPE_UserAuthenticationServer			!= PnlComGetUserAuthentication()->getUserAuthentication( PNLC_NetworkLan::getInstance() ->getAdminNWLine( ) )
				    &&  TYPE_UserAuthenticationDeviceAndServer	!= PnlComGetUserAuthentication()->getUserAuthentication( PNLC_NetworkLan::getInstance() ->getAdminNWLine( ) )
					&&  TYPE_TrackModeOff						== PnlComGetTrackManager()->getTrackMode(PNLC_NetworkLan::getInstance() ->getAdminNWLine( )) )	{
					bl_RemoteCounterCheckDisp = False;
				}
				if( False == bl_LogoutCheckDisp && False == bl_RemoteCounterCheckDisp ) {
					// 50921000_ユーザー／部門共通設定画面に表示項目が無い場合は非表示.
					b_Show[9]	= False;
				} else {
					b_Show[9]	= True;
				}
#else //DEF_MonoCus.
				b_Show[9]	= True;		// ユーザー/部門共通設定キー.
#endif //DEF_MonoCus.
			} else {	// 上限値オーバー発生時.
				b_Show[0]	= False;	// 認証方式.
				b_Show[1]	= False;	// ユーザー認証設定.
				b_Show[2]	= False;	// 部門管理設定.
				b_Show[3]	= False;	// 認証指定なしプリント
				b_Show[4]	= False;	// 使用管理者カウンタリスト.
				b_Show[5]	= False;	// 外部サーバー設定キー.
				b_Show[6]	= False;	// 優先/代替サーバー接続状態.
				b_Show[7]	= False;	// 参照許可設定.
				b_Show[8]	= False;	// 認証装置設定.
				b_Show[9]	= False;	// ユーザー/部門共通設定.
// 	Comment 2017/09/28 [HCL-Harish]【IT5 4.2】【RQ-686】【WP509】Multiple Network support.
//				if( PnlComGetUserAuthentication()->getUserAuthentication() != TYPE_UserAuthenticationOff )
				if( PnlComGetUserAuthentication()->getUserAuthentication( PNLC_NetworkLan::getInstance() ->getAdminNWLine( ) ) != TYPE_UserAuthenticationOff )
				{ b_Show[1]	= True; }		// ユーザ認証有り時、「ユーザ認証」キーを表示.
				if( PnlComGetTrackManager()->getTrackMode( PNLC_NetworkLan::getInstance()->getAdminNWLine() ) != TYPE_TrackModeOff )
				{ b_Show[2]	= True; }		// 部門管理認証有り時、「部門管理」キーを表示.
			}
// 	Comment 2017/09/28 [HCL-Harish]【IT5 4.2】【RQ-686】【WP509】Multiple Network support.
//			if( TYPE_UserAuthenticationIntermeddiateServer == PnlComGetUserAuthentication()->getUserAuthentication() 		// 中間認証.
			if( TYPE_UserAuthenticationIntermeddiateServer == PnlComGetUserAuthentication()->getUserAuthentication(  PNLC_NetworkLan::getInstance() ->getAdminNWLine( ) ) 		// 中間認証.
// comment 2017/10/31 【HCL-TANU】【IT5_4.2】【RQ686】【IT Defect AR1030817].
//				|| True == PnlComGetUserAuthentication()->getMidauthControl() ) {
	         	|| True == PnlComGetUserAuthentication()->getMidauthControl(PNLC_NetworkLan::getInstance() ->getAdminNWLine( )) ) {
				b_Show[2]	= False;	// 部門管理設定.
				if( bl_BumonLimitWarningAdminMove == False ) {
					// 通常時.
					b_Show[8]	= True;		// 認証装置設定.
				}
			}
// comment 2014/12/26 OPS1-Sky石賀 [ZeusS][CQ505574] 不要な条件を削除.
//			if((TYPE_UserAuthenticationDeviceAndMidAuth    == PnlComGetUserAuthentication()->getUserAuthentication()) 		// 本体+中間認証.
//			|| True == PnlComGetUserAuthentication()->getMidauthControl() ){
// 	Comment 2017/09/28 [HCL-Harish]【IT5 4.2】【RQ-686】【WP509】Multiple Network support.
//			if ( TYPE_UserAuthenticationDeviceAndMidAuth == PnlComGetUserAuthentication()->getUserAuthentication()) {		// 本体+中間認証.
			if ( TYPE_UserAuthenticationDeviceAndMidAuth == PnlComGetUserAuthentication()->getUserAuthentication( PNLC_NetworkLan::getInstance() ->getAdminNWLine( ) )) {		// 本体+中間認証.
				b_Show[2]	= True;		// 部門管理設定.
				if( bl_BumonLimitWarningAdminMove == False ) {
					// 通常時.
					b_Show[8]	= True;		// 認証装置設定.
				}
			}
			APIC_TabletIF* pc_TabletIF = APIC_TabletIF::newInstance();
			if( ( NULL != pc_TabletIF ) && ( True == pc_TabletIF->getConnectState() ) ) {
				b_Show[8]	= False;	// 認証装置設定キー.
			}
			setupPageParts( b_Show, b_Active );
			PToAdminSetMenucom::setupGamen( option );
		} else {	//P2のとき.
			b_Show[1]	= checkNextPageContents();		//送信宛先制限キー.
			if( True == PLBC_FuncPKICheck::isPKI() ) {		// ＰＫＩカード認証設定が有効の場合.
				b_Show[3]	= True;		// 証明書検証設定.
			} else {
				b_Show[3]	= False;	// 証明書検証設定.
			}

// comment 2016/12/07 Sky東島 [特ROM][AU DoD].
//			if( True == PLBC_FuncPKICheck::isPKI() ) {		// ＰＫＩカード認証設定が有効の場合.
			if( ( True == PLBC_FuncPKICheck::isPKI() )		// ＰＫＩカード認証設定が有効かつAU DODモードではない場合.
			 && ( False == isContactlessPKICard() ) ) {
				b_Show[2]	= False;	// プリント簡易認証.
			} else {
				b_Show[2]	= True;		// プリント簡易認証.
				b_Active[2]	= True;
			}
			if( True == PLBC_FuncVersionCheck::isOK_MiZe_Type2() ) {												// ZeusBK/Zeus2.1（機能拡張2）以降.
				TYPS_ExtDetailAuthentication* ps_DetailAuth = PCGC_AuthOapAPIAccess::getAuthenticationAPI();	// Authアプリ詳細情報取得.
				APIC_UserAuthentication* pc_UserAuth = PnlComGetUserAuthentication();							// ユーザー認証情報取得.
				// 拡張サーバーダウン時の上限キーの表示/非表示.
				if( ( NULL != ps_DetailAuth ) && ( NULL != pc_UserAuth )
				    && ( TYPE_ExtCounterManagementEnable == ps_DetailAuth->s_Function.e_CounterManagement )		// カウンタ管理設定「有効」.
				    && ( TYPE_ExtCounterLimitEnable == ps_DetailAuth->s_Function.e_CounterLimit )					// カウンタ上限管理設定「有効」.
// 	Comment 2017/09/28 [HCL-Harish]【IT5 4.2】【RQ-686】【WP509】Multiple Network support.
//				    && ( True == pc_UserAuth->getAuthCacheMode() )												// 認証情報の一時保存設定「する」.
//				    && ( ( TYPE_UserAuthenticationIntermeddiateServer == pc_UserAuth->getUserAuthentication() )	// ユーザー認証方式「拡張サーバー認証」or「本体+拡張サーバー」.
//				         || ( TYPE_UserAuthenticationDeviceAndMidAuth == pc_UserAuth->getUserAuthentication() ) ) ) {
					&& ( True == pc_UserAuth->getAuthCacheMode(PNLC_NetworkLan::getInstance() ->getAdminNWLine( )) )												// 認証情報の一時保存設定「する」.
				    && ( ( TYPE_UserAuthenticationIntermeddiateServer == pc_UserAuth->getUserAuthentication( PNLC_NetworkLan::getInstance() ->getAdminNWLine( ) ) )	// ユーザー認証方式「拡張サーバー認証」or「本体+拡張サーバー」.
				         || ( TYPE_UserAuthenticationDeviceAndMidAuth == pc_UserAuth->getUserAuthentication( PNLC_NetworkLan::getInstance() ->getAdminNWLine( ) ) ) ) ) {
				    b_Show[4]	= True;		// 拡張サーバーダウン時の上限.
				} else {
					b_Show[4]	= False;	// 拡張サーバーダウン時の上限.
				}
				// 課金情報の更新キーの表示/非表示.
				if( ( NULL != ps_DetailAuth )
				    && ( TYPE_ExtCounterManagementEnable == ps_DetailAuth->s_Function.e_CounterManagement ) ) {	// カウンタ管理設定「有効」.
					b_Show[5]	= True;		// 課金情報の更新.
				} else {
					b_Show[5]	= False;	// 課金情報の更新.
				}
			} else {
				b_Show[4]	= False;	// 拡張サーバーダウン時の上限.
				b_Show[5]	= False;	// 課金情報の更新.
			}
			if( bl_BumonLimitWarningAdminMove == False ) {	// 通常時.
				b_Show[0]	= bl_EnableHDDExistFunc;	// Home宛先有効設定キー.
			}
			b_Show[6]	= True;		// AD認証時自己検証設定.

			setupPageParts( b_Show, b_Active );
			PToAdminSetMenucom::setupGamen( option );
		}
	}
	else {
		if( uc_page == 1 ) {	//P1のとき.
			// 選択不可時、ユーザー認証キー、部門管理キーグレーアウト.
			Bool _bl_ActiveUserAttest = True;		// ユーザー認証活性状態.
			Bool _bl_ActiveDepartManage = True;		// 部門管理活性状態.
// 	Comment 2017/09/28 [HCL-Harish]【IT5 4.2】【RQ-686】【WP509】Multiple Network support.
//			if( TYPE_UserAuthenticationOff == PnlComGetUserAuthentication()->getUserAuthentication() ) {	// ユーザー認証しない時.
			if( TYPE_UserAuthenticationOff == PnlComGetUserAuthentication()->getUserAuthentication(  PNLC_NetworkLan::getInstance() ->getAdminNWLine( ) ) ) {	// ユーザー認証しない時.
				_bl_ActiveUserAttest = False;	// グレーアウト.
			}
			if( TYPE_TrackModeOff == PnlComGetTrackManager()->getTrackMode( PNLC_NetworkLan::getInstance()->getAdminNWLine() ) ) {	// 部門管理しない時.
				_bl_ActiveDepartManage = False;	// グレーアウト.
			}
			b_Active[1]	= _bl_ActiveUserAttest;		// ユーザー認証.
			b_Active[2]	= _bl_ActiveDepartManage;	// 部門管理.
			if( bl_BumonLimitWarningAdminMove == False ) {	// 通常時.
				if( APIC_KeyDevice::getKeyMode() == TYPE_KeyModePageAcses ) {
					// PageACSES接続時にはユーザー/部門共通設定以外は非表示にする.
					b_Show[0]	= False;	// ユーザ/部門の連動認証.
					b_Show[1]	= False;	// ユーザ認証.
					b_Show[2]	= False;	// 部門管理.
					b_Show[3]	= False;	// 認証指定なしプリント.
					b_Show[4]	= False;	// 使用管理者カウンタリスト.
					b_Show[5]	= False;	// 外部サーバー設定キー.
					b_Show[6]	= False;	// 参照許可設定キー.
					b_Show[7]	= False;	// 認証装置設定キー.
					b_Show[9]	= False;	// Home宛先有効設定キー.
					Nfd_AdminUserAndDepartComDspPage.showEnable( False );					// 現ページ数.
					Nfd_AdminUserAndDepartComTotalPage.showEnable( False );					// 総ページ数.
					Bak_AdminUserAndDepartComPageDsp.showEnable( False );					// ページ表示エリア.
					Key_AdminUserAndDepartComPagePrev.showEnable( False );					// 前画面キー.
					Key_AdminUserAndDepartComPageNext.showEnable( False );					// 次画面キー.
					Key_AdminUserAndDepartComPagePrevAster.showEnable( False );				// [*]キー.
					Key_AdminUserAndDepartComPageNextSharp.showEnable( False );				// [#]キー.
				} else {
					if( True == isAdminMachineDirectLogin() ) {
						b_Show[0]	= False;	// 認証方式.
					} else {
						b_Show[0]	= True;		// 認証方式.
					}
					b_Show[1]	= True;		// ユーザ認証.
					b_Show[2]	= True;		// 部門管理.
					b_Show[3]	= True;		// 認証指定なしプリント.
					b_Show[5]	= True;		//// 外部サーバー設定キー.
					// サービス管理機能チョイス設定が「認証装置２」で、ユーザー認証しているときのみ表示する.
					if(	TYPE_AUTH_DEVICE_NONE      != PnlComGetAuthDeviceSetting()->getAuthDeviceType() &&
					    TYPE_KeyModeNone           == APIC_KeyDevice::getKeyMode()                      &&
// 	Comment 2017/09/28 [HCL-Harish]【IT5 4.2】【RQ-686】【WP509】Multiple Network support.
//						TYPE_UserAuthenticationOff != PnlComGetUserAuthentication()->getUserAuthentication() ) {
						TYPE_UserAuthenticationOff != PnlComGetUserAuthentication()->getUserAuthentication( PNLC_NetworkLan::getInstance() ->getAdminNWLine( ) ) ) {
						// 認証装置設定キー表示.
						b_EnableAuthDeviceSetting = True;
					} else {
						// 認証装置設定キー非表示.
						b_EnableAuthDeviceSetting = False;
					}
					if( True == onlyPanel.JudgeModel( TYPE_FunctionTypeLow ) ) {	// Lowモデル時.
						b_EnableAuthDeviceSetting = False;
					}
					b_Show[6]	= bl_EnableHDDExistFunc;		// 参照許可設定キー.
					b_Show[7]	= b_EnableAuthDeviceSetting;	// 認証装置設定キー.
					// ユーザ認証、部門管理共に行わない場合、使用管理カウンタリストキーを非表示.
					// ベンダー1、2、管理装置2、認証装置1、キーカウンター装着時は非表示.
// 	Comment 2017/09/28 [HCL-Harish]【IT5 4.2】【RQ-686】【WP509】Multiple Network support.
//					if( PnlComGetUserAuthentication()->getUserAuthentication() == TYPE_UserAuthenticationOff
//					    && PnlComGetTrackManager()->getTrackMode() == TYPE_TrackModeOff )
					if( PnlComGetUserAuthentication()->getUserAuthentication( PNLC_NetworkLan::getInstance() ->getAdminNWLine( ) ) == TYPE_UserAuthenticationOff
					    && PnlComGetTrackManager()->getTrackMode(PNLC_NetworkLan::getInstance() ->getAdminNWLine( )) == TYPE_TrackModeOff )
						// ユーザ認証OFFかつ、部門管理OFFの場合非表示.
						// 他の条件は上位の画面で制限.
					{ b_Show[4]	= False; }
					else
					{ b_Show[4]	= True; }
					b_Show[9]	= bl_EnableHDDExistFunc;				// Home宛先有効設定キー.
					Bool bl_NextPageContent = checkNextPageContents();
					Nfd_AdminUserAndDepartComDspPage.showEnable( bl_NextPageContent );					// 現ページ数.
					Nfd_AdminUserAndDepartComTotalPage.showEnable( bl_NextPageContent );				// 総ページ数.
					Bak_AdminUserAndDepartComPageDsp.showEnable( bl_NextPageContent );					// ページ表示エリア.
					Key_AdminUserAndDepartComPagePrev.showEnable( bl_NextPageContent );					// 前画面キー.
					Key_AdminUserAndDepartComPageNext.showEnable( bl_NextPageContent );					// 次画面キー.
					Key_AdminUserAndDepartComPagePrevAster.showEnable( bl_NextPageContent );			// [*]キー.
					Key_AdminUserAndDepartComPageNextSharp.showEnable( bl_NextPageContent );			// [#]キー.
				}
#ifdef DEF_MonoCus
				Bool bl_LogoutCheckDisp 		= True;			// 50921000_ユーザー／部門共通設定画面の「ログアウト確認画面表示設定」キー.
				Bool bl_RemoteCounterCheckDisp 	= True;			// 50921000_ユーザー／部門共通設定画面の「遠隔カウンター管理」キー.

				// ユーザ認証=なし　＆　部門認証=なし　の場合は、ログアウト確認画面キーは非表示.
// 	Comment 2017/09/28 [HCL-Harish]【IT5 4.2】【RQ-686】【WP509】Multiple Network support.
//				if( ( TYPE_UserAuthenticationOff == PnlComGetUserAuthentication()->getUserAuthentication() )
//					&& ( TYPE_TrackModeOff == PnlComGetTrackManager()->getTrackMode() ) )
				if( ( TYPE_UserAuthenticationOff == PnlComGetUserAuthentication()->getUserAuthentication( PNLC_NetworkLan::getInstance() ->getAdminNWLine( ) ) )
					&& ( TYPE_TrackModeOff == PnlComGetTrackManager()->getTrackMode(PNLC_NetworkLan::getInstance() ->getAdminNWLine( )) ) )
				{
					bl_LogoutCheckDisp = False;
				}
				// 本体認証、部門管理、外部サーバー認証 本体認証+外部サーバー認証 利用時以外は遠隔カウンター管理キーを非表示.
// 	Comment 2017/09/28 [HCL-Harish]【IT5 4.2】【RQ-686】【WP509】Multiple Network support.
//				if( TYPE_UserAuthenticationDevice			!= PnlComGetUserAuthentication()->getUserAuthentication()
//				    &&  TYPE_UserAuthenticationDeviceAndMidAuth	!= PnlComGetUserAuthentication()->getUserAuthentication()
//				    &&  TYPE_UserAuthenticationServer			!= PnlComGetUserAuthentication()->getUserAuthentication()
//				    &&  TYPE_UserAuthenticationDeviceAndServer	!= PnlComGetUserAuthentication()->getUserAuthentication()
//				    &&  TYPE_TrackModeOff						== PnlComGetTrackManager()->getTrackMode() )	{
				if( TYPE_UserAuthenticationDevice			!= PnlComGetUserAuthentication()->getUserAuthentication( PNLC_NetworkLan::getInstance() ->getAdminNWLine( )  )
				    &&  TYPE_UserAuthenticationDeviceAndMidAuth	!= PnlComGetUserAuthentication()->getUserAuthentication( PNLC_NetworkLan::getInstance() ->getAdminNWLine( )  )
				    &&  TYPE_UserAuthenticationServer			!= PnlComGetUserAuthentication()->getUserAuthentication( PNLC_NetworkLan::getInstance() ->getAdminNWLine( )  )
				    &&  TYPE_UserAuthenticationDeviceAndServer	!= PnlComGetUserAuthentication()->getUserAuthentication( PNLC_NetworkLan::getInstance() ->getAdminNWLine( )  )
				    &&  TYPE_TrackModeOff						== PnlComGetTrackManager()->getTrackMode( PNLC_NetworkLan::getInstance() ->getAdminNWLine( )  ) )	{
					bl_RemoteCounterCheckDisp = False;
				}
				if( False == bl_LogoutCheckDisp && False == bl_RemoteCounterCheckDisp ) {
					// 50921000_ユーザー／部門共通設定画面に表示項目が無い場合は非表示.
					b_Show[8]	= False;	// ユーザー/部門共通設定キー.
				} else {
					b_Show[8]	= True;		// ユーザー/部門共通設定キー.
				}
#else //DEF_MonoCus.
				b_Show[8]	= True;		// ユーザー/部門共通設定キー.
#endif //DEF_MonoCus.
			} else {	// 上限値オーバー発生時.
				b_Show[0]	= False;	// ユーザ/部門の連動認証
				b_Show[1]	= False;	// ユーザ認証.
				b_Show[2]	= False;	// 部門管理.
				b_Show[3]	= False;	// 認証指定なしプリント.
				b_Show[4]	= False;	// 使用管理者カウンタリスト.
				b_Show[5]	= False;	// 外部サーバー設定キー.
				b_Show[6]	= False;	// 参照許可設定キー.
				b_Show[7]	= False;	//【課題：暫定】認証装置設定キー.
				b_Show[8]	= False;	// ユーザー/部門共通設定キー.
// 	Comment 2017/09/28 [HCL-Harish]【IT5 4.2】【RQ-686】【WP509】Multiple Network support.
//				if( PnlComGetUserAuthentication()->getUserAuthentication() != TYPE_UserAuthenticationOff )
				if( PnlComGetUserAuthentication()->getUserAuthentication( PNLC_NetworkLan::getInstance() ->getAdminNWLine( ) ) != TYPE_UserAuthenticationOff )
				{ b_Show[1]	= True; }		// ユーザ認証有り時、「ユーザ認証」キーを表示.
				if( PnlComGetTrackManager()->getTrackMode( PNLC_NetworkLan::getInstance()->getAdminNWLine() ) != TYPE_TrackModeOff )
				{ b_Show[2]	= True; }		// 部門管理認証有り時、「部門管理」キーを表示.
			}
// 	Comment 2017/09/28 [HCL-Harish]【IT5 4.2】【RQ-686】【WP509】Multiple Network support.
//			if( TYPE_UserAuthenticationIntermeddiateServer == PnlComGetUserAuthentication()->getUserAuthentication() 		// 中間認証.
			if( TYPE_UserAuthenticationIntermeddiateServer == PnlComGetUserAuthentication()->getUserAuthentication( PNLC_NetworkLan::getInstance() ->getAdminNWLine( ) ) 		// 中間認証.
// comment 2017/10/31 【HCL-TANU】【IT5_4.2】【RQ686】【IT Defect AR1030817].
//			    || True == PnlComGetUserAuthentication()->getMidauthControl() ) {
			    || True == PnlComGetUserAuthentication()->getMidauthControl(PNLC_NetworkLan::getInstance() ->getAdminNWLine( )) ) {
				// No309508対応.
				b_Show[2]	= False;	// 部門管理設定.
				if( bl_BumonLimitWarningAdminMove == False ) {
					// 通常時.
					b_Show[7]	= True;		// 認証装置設定.
				}
			}
// 	Comment 2017/09/28 [HCL-Harish]【IT5 4.2】【RQ-686】【WP509】Multiple Network support.
//			if( ( TYPE_UserAuthenticationDeviceAndMidAuth    == PnlComGetUserAuthentication()->getUserAuthentication() ) 		// 本体+中間認証.
			if( ( TYPE_UserAuthenticationDeviceAndMidAuth    == PnlComGetUserAuthentication()->getUserAuthentication( PNLC_NetworkLan::getInstance() ->getAdminNWLine( ) ) ) 		// 本体+中間認証.
// comment 2017/10/31 【HCL-TANU】【IT5_4.2】【RQ686】【IT Defect AR1030817].
//				|| True == PnlComGetUserAuthentication()->getMidauthControl() ) {
				|| True == PnlComGetUserAuthentication()->getMidauthControl(PNLC_NetworkLan::getInstance() ->getAdminNWLine( )) ) {
				b_Show[2]	= True;		// 部門管理設定.
				if( bl_BumonLimitWarningAdminMove == False ) {
					// 通常時.
					b_Show[7]	= True;		// 認証装置設定.
				}
			}
			APIC_TabletIF* pc_TabletIF = APIC_TabletIF::newInstance();
			if( ( NULL != pc_TabletIF ) && ( True == pc_TabletIF->getConnectState() ) ) {
				b_Show[7]	= False;	// 認証装置設定キー.
			}

			setupPageParts( b_Show, b_Active );
			PToAdminSetMenucom::setupGamen( option );
		} else {	//P2のとき.
			b_Show[0]	= checkNextPageContents();	//送信宛先制限キー.
			if( True == PLBC_FuncPKICheck::isPKI() ) {		// ＰＫＩカード認証設定が有効の場合.
				b_Show[2]	= True;		// 証明書検証設定キー.
			} else {
				b_Show[2]	= False;	// 非PKI時は証明書検証設定キーださない.
			}

			if( True == PLBC_FuncPKICheck::isPKI() ) {		// ＰＫＩカード認証設定が有効の場合.
				b_Show[1]	= False;
			} else {
				b_Show[1]	= True;
				b_Active[1]	= True;
			}
			if( True == PLBC_FuncVersionCheck::isOK_MiZe_Type2() ) {												// ZeusBK/Zeus2.1（機能拡張2）以降.
				TYPS_ExtDetailAuthentication* ps_DetailAuth = PCGC_AuthOapAPIAccess::getAuthenticationAPI();	// Authアプリ詳細情報取得.
				APIC_UserAuthentication* pc_UserAuth = PnlComGetUserAuthentication();							// ユーザー認証情報取得.
				// 拡張サーバーダウン時の上限キーの表示/非表示.
				if( ( NULL != ps_DetailAuth ) && ( NULL != pc_UserAuth )
				    && ( TYPE_ExtCounterManagementEnable == ps_DetailAuth->s_Function.e_CounterManagement )		// カウンタ管理設定「有効」.
				    && ( TYPE_ExtCounterLimitEnable == ps_DetailAuth->s_Function.e_CounterLimit )					// カウンタ上限管理設定「有効」.
// 	Comment 2017/09/28 [HCL-Harish]【IT5 4.2】【RQ-686】【WP509】Multiple Network support.
//				    && ( True == pc_UserAuth->getAuthCacheMode() )	
//				    && ( ( TYPE_UserAuthenticationIntermeddiateServer == pc_UserAuth->getUserAuthentication() )	// ユーザー認証方式「拡張サーバー認証」or「本体+拡張サーバー」.
//				         || ( TYPE_UserAuthenticationDeviceAndMidAuth == pc_UserAuth->getUserAuthentication() ) ) ) {
					&& ( True == pc_UserAuth->getAuthCacheMode( PNLC_NetworkLan::getInstance() ->getAdminNWLine( ) ) )	
				    && ( ( TYPE_UserAuthenticationIntermeddiateServer == pc_UserAuth->getUserAuthentication( PNLC_NetworkLan::getInstance() ->getAdminNWLine( ) ) )	// ユーザー認証方式「拡張サーバー認証」or「本体+拡張サーバー」.
				         || ( TYPE_UserAuthenticationDeviceAndMidAuth == pc_UserAuth->getUserAuthentication( PNLC_NetworkLan::getInstance() ->getAdminNWLine( ) ) ) ) ) {
					b_Show[3]	= True;
				} else {
					b_Show[3]	= False;
				}
				// 課金情報の更新キーの表示/非表示.
				if( ( NULL != ps_DetailAuth )
				    && ( TYPE_ExtCounterManagementEnable == ps_DetailAuth->s_Function.e_CounterManagement ) ) {	// カウンタ管理設定「有効」.
					b_Show[4]	= True;
				} else {
					b_Show[4]	= False;
				}
			} else {
				b_Show[3]	= False;
				b_Show[4]	= False;
			}

			setupPageParts( b_Show, b_Active );
			PToAdminSetMenucom::setupGamen( option );
		}
	}
}
//================================================================================================.
// 概要		:管理者設定・ユーザ認証/部門管理設定  : シャープキー.
// 戻り値	:なし.
//================================================================================================.
void PAdminUserAndDepartSet::procShrp( uchar option )
{
	procButtonOn( 31 );
}
//================================================================================================.
// 概要		:管理者設定・ユーザ認証/部門管理設定  : アスタリスクキー.
// 戻り値	:なし.
//================================================================================================.
void PAdminUserAndDepartSet::procAster( uchar option )
{
	procButtonOn( 30 );
}
// ==================================================================.
// 概要		:管理者設定・ユーザ認証/部門管理設定 : 2ﾍﾟｰｼﾞ目表示内容チェック関数.
// 戻り値	:Bool .
// 備考		: HDD非装着時対応.
// ==================================================================.
Bool PAdminUserAndDepartSet::checkNextPageContents( )
{
	Bool bl_Result = True;
	if( False == PnlComIsUseSsdScanSend() ) {	// HDD/SSD使用不可.
		bl_Result = False;
	}
	return bl_Result;
}

//////////////////////////////////////////////////////////////
//概要    : 管理者設定・ユーザ認証/部門管理設定メニュー(2690): 画面開始処理.
//備考    :
//////////////////////////////////////////////////////////////
void PAdminUserAndDepartSet::startGamen()
{
	PToAdminSetMenucom::startGamen();
	if( gp_AdminSetMenu.b_fromMenu == True ) {	// 上階層から遷移してきたときのみAPIから値を取ってくる.
		gp_AdminSetMenu.b_fromMenu = False;	// 上階層からではなくする.
		uc_page = 1;
	}
	b_EnableAuthDeviceSetting = False;

	setPageParts();

	// 左エリア種類の有効設定（ジョブ表示（通常））startGamen()の最後でコールしてください.
	setLeftAreaEnableType( TYPD_LeftAreaBookmark | TYPD_LeftAreaSoft10Key );
}

//====================================================================== .
// 概要   :  ボックス削除中：ボックス削除要求-返答.
//====================================================================== .
void ans_UserAndDepartBoxDelete_Indi( int32_t boxID, int32_t rtn )
{
	if( APID_DocapiMethodOk != rtn ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
	}
	CallBackIndi( TYPE_PNL_CallBack_UserAndDepartBoxDelete_Indi );		// コールバックイベント.
}
//====================================================================== .
// 概要   :  ボックス削除中：ユビキタスプリントボックス全削除-返答.
//====================================================================== .
void ans_AdminDocUbiquitousBoxAllDelete( int32_t i32_SeqID, int32_t i32_Result )
{

	if( APID_UbqResultOK != i32_Result ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
	}

	CallBackIndi( TYPE_PNL_CallBack_UbiquitousBoxAllDelete );			// コールバックイベント.


}

//====================================================================== .
// 概要   :  ボックス削除中：拡張ユーザーデータの削除-返答.
//====================================================================== .
void ans_deleteLargerScaleDatabase()
{
	CallBackIndi( TYPE_PNL_CallBack_deleteLargerScaleDatabase );

}

//関数------------------------------------------------------------------------------------------------------------------.
//概要    ：管理者設定・ユーザ認証/部門管理設定  : ページパーツ登録.
//備考    ：.
//戻値    ：なし.
//作成者  ：-.
//------------------------------------------------------------------------------.
/**
* @brief 管理者設定・ユーザ認証/部門管理設定  : ページパーツ登録.
*/
//------------------------------------------------------------------------------.
void PAdminUserAndDepartSet::setPageParts()
{
// comment 2022/05/20 Sky西場 [IT6_2.3][OP_BTS-37620][スリム化].
//	PBasicMessageProc*	pc_NonAttesPrint	= 0;
//	if( True == PLBC_FuncVersionCheck::isOK_MiZe_Type4() ) {
//		pc_NonAttesPrint	= &gp_AdminUserAndDepartNonAttestPrintWithIPFilter;
//	}
//	else {
//		pc_NonAttesPrint	= &gp_AdminUserAndDepartNonAttestPrint;
//	}
// Comment 2015/08/01 HCL_Jeevitha【ZeusS_ZX0】【RQ-261】Added URL Address Setting key in 2nd page.
	if( True == PLBC_FuncVersionCheck::isOK_IT5_Type9() ) {
		if( 1 == uc_page ) {	// 1ページ目.
			APIC_AuthenticationSeverSetting* apicAuthSrv = APIC_AuthenticationSeverSetting::newInstance();
			if( NULL == apicAuthSrv ) {
// comment [2016/12/22] [PDD22山下(日立)] [IT5_4.0][AccuRev.1009508]認証拡張FUM追加コードに対して新詳細コードを追加.
//Comment 2019/06/14 [HCLT_Monisha][EagleH][N Rank] Removed unwanted Detailed code.
//				LAPC_BFW_System *pc_BFW_System = LAPC_BFW_System::newInstance();
//				if (NULL != pc_BFW_System){    
//				    pc_BFW_System ->registDetailCode( PNLD_DetailCode_Admin_0x19006500104E0501 );
//				}
				LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" ); 
				return;
			}
			
// 	Comment 2018/01/03 [HCL-Shalini]【IT5 4.2】【RQ-686】【WP1382,1384,1386,1388,1390】【AR1035279】Passed third argument as True for Sharing Common Icon Image for Network 1&Network 2 Settings.
			if( True == apicAuthSrv->checkServerGroup() ) {
				// サーバーグループが有効な場合.
// comment 2022/05/20 Sky西場 [IT6_2.3][OP_BTS-37620][スリム化].
				SP_UtilityAndAdminMenuData	sp_UtilityAndAdminMenuData[] = {
					// シンボル                 画面ポインタ.
					{ s_KEYAuthenticationMethod_2,			&gp_AdminUserAndDepartSynchro,True },							// [1]認証方式.
//					{ s_KEYUserAuthenticationSetting_2,		&gp_AdminUserAndDepartUserAttestSet },						// [2]ユーザー認証設定.
//					{ s_KEYGroupManagementSetting,			&gp_AdminUserAndDepartManageSet },							// [3]部門管理設定.
//					{ s_KEYPrintWithoutAuthentication,		pc_NonAttesPrint },											// [4]認証指定なし印刷.
					{ NULL,									NULL },														// [2]ユーザー認証設定.
					{ NULL,									NULL },														// [3]部門管理設定.
					{ NULL,									NULL },														// [4]認証指定なし印刷.
					{ s_KEYUseManagementCounterList,		&gp_AdminSystemDepartListPrintUserCounterListPrint,True },		// [5]使用管理カウンターリスト.
//					{ s_KEYExternalServerSetting,			&gp_AdminAuthServerGroup_AuthServer },						// [6]外部サーバー設定.
//					{ s_KEYAuthServerStatus,				&gp_AdminUserAndDepartAuthServerStatus },					// [7]認証サーバー接続状態.
//					{ s_KEYRefPermit,						&gp_AdminUserAndDepartReferencePermissionSettingMenu },		// [8]参照許可設定.
//					{ s_KEYAttentionDeveiceSetting,			&gp_AdminUserAttentionDeviceSetting,True },						// [9]認証装置設定.
//					{ s_KEYUserGroupCommonSetting,			&gp_AdminUserAndDepartCommonSetting,True}						// [0]ユーザー/部門共通設定.
					{ NULL,									NULL },														// [6]外部サーバー設定.
					{ NULL,									NULL },														// [7]認証サーバー接続状態.
					{ NULL,									NULL },														// [8]参照許可設定.
					{ NULL,									NULL },														// [9]認証装置設定.
					{ NULL,									NULL }														// [0]ユーザー/部門共通設定.
				};
				pc_MenuUtilityAndAdmin.initialSPC( sp_UtilityAndAdminMenuData );	// メニュー画面データセット.
			} else {
// comment 2022/05/20 Sky西場 [IT6_2.3][OP_BTS-37620][スリム化].
				SP_UtilityAndAdminMenuData	sp_UtilityAndAdminMenuData[] = {
					// シンボル                 画面ポインタ.
					{ s_KEYAuthenticationMethod_2,			&gp_AdminUserAndDepartSynchro,True },							// [1]認証方式.
//					{ s_KEYUserAuthenticationSetting_2,		&gp_AdminUserAndDepartUserAttestSet },						// [2]ユーザー認証設定.
//					{ s_KEYGroupManagementSetting,			&gp_AdminUserAndDepartManageSet },							// [3]部門管理設定.
//					{ s_KEYPrintWithoutAuthentication,		pc_NonAttesPrint },											// [4]認証指定なし印刷.
					{ NULL,									NULL },														// [2]ユーザー認証設定.
					{ NULL,									NULL },														// [3]部門管理設定.
					{ NULL,									NULL },														// [4]認証指定なし印刷.
					{ s_KEYUseManagementCounterList,		&gp_AdminSystemDepartListPrintUserCounterListPrint,True },		// [5]使用管理カウンターリスト.
//					{ s_KEYExternalServerSetting,			&gp_AdminUserAndDepartAccountServer },						// [6]外部サーバー設定.
//					{ s_KEYExternalServerStatus,			&gp_AdminUserAndDepartExternalServerStatus },				// [7]優先/代替サーバー接続状態.
//					{ s_KEYRefPermit,						&gp_AdminUserAndDepartReferencePermissionSettingMenu },		// [8]参照許可設定.
//					{ s_KEYAttentionDeveiceSetting,			&gp_AdminUserAttentionDeviceSetting,True },						// [9]認証装置設定.
//					{ s_KEYUserGroupCommonSetting,			&gp_AdminUserAndDepartCommonSetting,True }						// [0]ユーザー/部門共通設定.
					{ NULL,									NULL },														// [6]外部サーバー設定.
					{ NULL,									NULL },														// [7]優先/代替サーバー接続状態.
					{ NULL,									NULL },														// [8]参照許可設定.
					{ NULL,									NULL },														// [9]認証装置設定.
					{ NULL,									NULL }														// [0]ユーザー/部門共通設定.
				};
				pc_MenuUtilityAndAdmin.initialSPC( sp_UtilityAndAdminMenuData );	// メニュー画面データセット.
			}
		}
		else {	// 2ページ目.
// comment 2022/05/20 Sky西場 [IT6_2.3][OP_BTS-37620][スリム化].
			SP_UtilityAndAdminMenuData	sp_UtilityAndAdminMenuData[] = {
				// シンボル                 画面ポインタ.
//				{ s_KEYUrlAddressSetting,				&gp_AdminUserAndDepartURLAddressSetting},					// [1]URL Address Setting.
//				{ s_KEYHomeAddressSetting,				&gp_AdminUserAndDepartHomeAddressSetting },					// [2]Home宛先有効設定.
//				{ s_KEYSendAddressPermission,			&gp_AdminUserAndDepartSendAddressPermission },				// [3]送信宛先制限.
//				{ s_KEYAdminSimpleAuth_2,				&gp_AdminPrintSimpleAuth },								// [4]簡易認証.
//				{ s_KEYCertificateInspectionSetting,	&gp_AdminUserAndDepartVerifyCertificate_PKI },				// [5]証明書検証設定.
//				{ s_KEYDepartServerDownLimitSetting,	&gp_AdminUserAndDepartServerDownLimitSetting },				// [6]拡張サーバーダウン時の上限.
//				{ s_KEYCreditInfoUpdate,				&gp_AdminUserAndDepartCreditInfoUpdate },					// [7]課金情報の更新.
//				{ s_KEYAdminAuthSecuritySetting,		&gp_AdminNetworkSetSSOAuthDomainSet },						// [8]AD認証時自己検証設定.
				{ NULL,									NULL },														// [1]URL Address Setting.
				{ NULL,									NULL },														// [2]Home宛先有効設定.
				{ NULL,									NULL },														// [3]送信宛先制限.
				{ NULL,									NULL },														// [4]簡易認証.
				{ NULL,									NULL },														// [5]証明書検証設定.
				{ NULL,									NULL },														// [6]拡張サーバーダウン時の上限.
				{ NULL,									NULL },														// [7]課金情報の更新.
				{ NULL,									NULL },														// [8]AD認証時自己検証設定.
				{ NULL,									NULL },														// [9]なし.
				{ NULL,									NULL },														// [0]なし.
			};
			pc_MenuUtilityAndAdmin.initialSPC( sp_UtilityAndAdminMenuData );	// メニュー画面データセット.
		}
// 	Comment 2018/01/03 [HCL-Shalini]【IT5 4.2】【RQ-686】【WP1382,1384,1386,1388,1390】【AR1035279】Passed third argument as True for Sharing Common Icon Image for Network 1&Network 2 Settings.
	}else if( True == PLBC_FuncVersionCheck::isOK_IT5_Type4() ) {
// comment 2022/05/20 Sky西場 [IT6_2.3][OP_BTS-37620][スリム化].
		ExpPanelLog( PNLD_ExpLogKind_AlwaysLogOut, "[DeleteUnnecessaryProcessing] [%s]", __PRETTY_FUNCTION__, 0, 0, 0, 0, 0 );
//		if( 1 == uc_page ) {	// 1ページ目.
//			SP_UtilityAndAdminMenuData	sp_UtilityAndAdminMenuData[] = {
//				// シンボル                 画面ポインタ.
//				{ s_KEYAuthenticationMethod_2,			&gp_AdminUserAndDepartSynchro,True },							// [1]認証方式.
//				{ s_KEYUserAuthenticationSetting_2,		&gp_AdminUserAndDepartUserAttestSet },						// [2]ユーザー認証設定.
//				{ s_KEYGroupManagementSetting,			&gp_AdminUserAndDepartManageSet },							// [3]部門管理設定.
//				{ s_KEYPrintWithoutAuthentication,		pc_NonAttesPrint },											// [4]認証指定なし印刷.
//				{ s_KEYUseManagementCounterList,		&gp_AdminSystemDepartListPrintUserCounterListPrint,True },		// [5]使用管理カウンターリスト.
//				{ s_KEYExternalServerSetting,			&gp_AdminUserAndDepartAccountServer },						// [6]外部サーバー設定.
//				{ s_KEYExternalServerStatus,			&gp_AdminUserAndDepartExternalServerStatus },				// [7]優先/代替サーバー接続状態.
//				{ s_KEYRefPermit,						&gp_AdminUserAndDepartReferencePermissionSettingMenu },		// [8]参照許可設定.
//				{ s_KEYAttentionDeveiceSetting,			&gp_AdminUserAttentionDeviceSetting,True },						// [9]認証装置設定.
//				{ s_KEYUserGroupCommonSetting,			&gp_AdminUserAndDepartCommonSetting,True }						// [0]ユーザー/部門共通設定.
//			};
//			pc_MenuUtilityAndAdmin.initialSPC( sp_UtilityAndAdminMenuData );	// メニュー画面データセット.
//		}
//		else {	// 2ページ目.
//			SP_UtilityAndAdminMenuData	sp_UtilityAndAdminMenuData[] = {
//				// シンボル                 画面ポインタ.
//				{ s_KEYUrlAddressSetting,				&gp_AdminUserAndDepartURLAddressSetting},					// [1]URL Address Setting.
//				{ s_KEYHomeAddressSetting,				&gp_AdminUserAndDepartHomeAddressSetting },					// [2]Home宛先有効設定.
//				{ s_KEYSendAddressPermission,			&gp_AdminUserAndDepartSendAddressPermission },				// [3]送信宛先制限.
//// Comment 2016/01/21 Sky竹内【ZeusS_ZX0】【CQ524284】【開発提起】【RQ-189】ワーディング整合結果を反映する。簡易認証⇒簡易認証設定.
//				//{ s_KEYAdminSimpleAuth,					&gp_AdminPrintSimpleAuth },									// [4]簡易認証.
//				{ s_KEYAdminSimpleAuth_2,					&gp_AdminPrintSimpleAuth },									// [4]簡易認証.
//				{ s_KEYCertificateInspectionSetting,	&gp_AdminUserAndDepartVerifyCertificate_PKI },				// [5]証明書検証設定.
//				{ s_KEYDepartServerDownLimitSetting,	&gp_AdminUserAndDepartServerDownLimitSetting },				// [6]拡張サーバーダウン時の上限.
//				{ s_KEYCreditInfoUpdate,				&gp_AdminUserAndDepartCreditInfoUpdate },					// [7]課金情報の更新.
//				{ s_KEYAdminAuthSecuritySetting,		&gp_AdminNetworkSetSSOAuthDomainSet },						// [8]AD認証時自己検証設定.
//				{ NULL,									NULL },														// [9]なし.
//				{ NULL,									NULL },														// [0]なし.
//			};
//			pc_MenuUtilityAndAdmin.initialSPC( sp_UtilityAndAdminMenuData );	// メニュー画面データセット.
//		}
	}
// comment 2015/02/10 [HCL-Vishnu] [MiSBK]【RQ-189】 Change the function name [プリント簡易認証]-> [簡易認証].
//	if ( True == PLBC_FuncVersionCheck::isOK_IT5_Type3() ) {
	else if ( True == PLBC_FuncVersionCheck::isOK_IT5_Type3() ) {
// comment 2022/05/20 Sky西場 [IT6_2.3][OP_BTS-37620][スリム化].
		ExpPanelLog( PNLD_ExpLogKind_AlwaysLogOut, "[DeleteUnnecessaryProcessing] [%s]", __PRETTY_FUNCTION__, 0, 0, 0, 0, 0 );
//		if( 1 == uc_page ) {	// 1ページ目.
//			SP_UtilityAndAdminMenuData	sp_UtilityAndAdminMenuData[] = {
//				// シンボル                 画面ポインタ.
//				{ s_KEYAuthenticationMethod_2,			&gp_AdminUserAndDepartSynchro },							// [1]認証方式.
//				{ s_KEYUserAuthenticationSetting_2,		&gp_AdminUserAndDepartUserAttestSet },						// [2]ユーザー認証設定.
//				{ s_KEYGroupManagementSetting,			&gp_AdminUserAndDepartManageSet },							// [3]部門管理設定.
//				{ s_KEYPrintWithoutAuthentication,		pc_NonAttesPrint },											// [4]認証指定なし印刷.
//				{ s_KEYUseManagementCounterList,		&gp_AdminSystemDepartListPrintUserCounterListPrint },		// [5]使用管理カウンターリスト.
//				{ s_KEYExternalServerSetting,			&gp_AdminUserAndDepartAccountServer },						// [6]外部サーバー設定.
//				{ s_KEYExternalServerStatus,			&gp_AdminUserAndDepartExternalServerStatus },				// [7]優先/代替サーバー接続状態.
//				{ s_KEYRefPermit,						&gp_AdminUserAndDepartReferencePermissionSettingMenu },		// [8]参照許可設定.
//				{ s_KEYAttentionDeveiceSetting,			&gp_AdminUserAttentionDeviceSetting },						// [9]認証装置設定.
//				{ s_KEYUserGroupCommonSetting,			&gp_AdminUserAndDepartCommonSetting }						// [0]ユーザー/部門共通設定.
//			};
//			pc_MenuUtilityAndAdmin.initialSPC( sp_UtilityAndAdminMenuData );	// メニュー画面データセット.
//		}
//		else {	// 2ページ目.
//			SP_UtilityAndAdminMenuData	sp_UtilityAndAdminMenuData[] = {
//				// シンボル                 画面ポインタ.
//				{ s_KEYHomeAddressSetting,				&gp_AdminUserAndDepartHomeAddressSetting },					// [1]Home宛先有効設定.
//				{ s_KEYSendAddressPermission,			&gp_AdminUserAndDepartSendAddressPermission },				// [2]送信宛先制限.
//				{ s_KEYAdminSimpleAuth,					&gp_AdminPrintSimpleAuth },									// [3]簡易認証.
//				{ s_KEYCertificateInspectionSetting,	&gp_AdminUserAndDepartVerifyCertificate_PKI },				// [4]証明書検証設定.
//				{ s_KEYDepartServerDownLimitSetting,	&gp_AdminUserAndDepartServerDownLimitSetting },				// [5]拡張サーバーダウン時の上限.
//				{ s_KEYCreditInfoUpdate,				&gp_AdminUserAndDepartCreditInfoUpdate },					// [6]課金情報の更新.
//				{ s_KEYAdminAuthSecuritySetting,		&gp_AdminNetworkSetSSOAuthDomainSet },						// [7]AD認証時自己検証設定.
//				{ NULL,									NULL },														// [8]なし.
//				{ NULL,									NULL },														// [9]なし.
//				{ NULL,									NULL },														// [0]なし.
//			};
//			pc_MenuUtilityAndAdmin.initialSPC( sp_UtilityAndAdminMenuData );	// メニュー画面データセット.
//		}
	}
//	if ( True == PLBC_FuncVersionCheck::isOK_IT5_Type1() ) {
	else if ( True == PLBC_FuncVersionCheck::isOK_IT5_Type1() ) {
// comment 2022/05/20 Sky西場 [IT6_2.3][OP_BTS-37620][スリム化].
		ExpPanelLog( PNLD_ExpLogKind_AlwaysLogOut, "[DeleteUnnecessaryProcessing] [%s]", __PRETTY_FUNCTION__, 0, 0, 0, 0, 0 );
//		if ( 1 == uc_page ) {	// 1ページ目.
//			SP_UtilityAndAdminMenuData	sp_UtilityAndAdminMenuData[] = {
//				// シンボル                 画面ポインタ.
//				{ s_KEYAuthenticationMethod_2,			&gp_AdminUserAndDepartSynchro },							// [1]認証方式.
//				{ s_KEYUserAuthenticationSetting_2,		&gp_AdminUserAndDepartUserAttestSet },						// [2]ユーザー認証設定.
//				{ s_KEYGroupManagementSetting,			&gp_AdminUserAndDepartManageSet },							// [3]部門管理設定.
//				{ s_KEYPrintWithoutAuthentication,		pc_NonAttesPrint },											// [4]認証指定なし印刷.
//				{ s_KEYUseManagementCounterList,		&gp_AdminSystemDepartListPrintUserCounterListPrint },		// [5]使用管理カウンターリスト.
//				{ s_KEYExternalServerSetting,			&gp_AdminUserAndDepartAccountServer },						// [6]外部サーバー設定.
//				{ s_KEYExternalServerStatus,			&gp_AdminUserAndDepartExternalServerStatus },				// [7]優先/代替サーバー接続状態.
//				{ s_KEYRefPermit,						&gp_AdminUserAndDepartReferencePermissionSettingMenu },		// [8]参照許可設定.
//				{ s_KEYAttentionDeveiceSetting,			&gp_AdminUserAttentionDeviceSetting },						// [9]認証装置設定.
//				{ s_KEYUserGroupCommonSetting,			&gp_AdminUserAndDepartCommonSetting }						// [0]ユーザー/部門共通設定.
//			};
//			pc_MenuUtilityAndAdmin.initialSPC( sp_UtilityAndAdminMenuData );	// メニュー画面データセット.
//		}
//		else {	// 2ページ目.
//			SP_UtilityAndAdminMenuData	sp_UtilityAndAdminMenuData[] = {
//				// シンボル                 画面ポインタ.
//				{ s_KEYHomeAddressSetting,				&gp_AdminUserAndDepartHomeAddressSetting },					// [1]Home宛先有効設定.
//				{ s_KEYSendAddressPermission,			&gp_AdminUserAndDepartSendAddressPermission },				// [2]送信宛先制限.
//				{ s_SYSAdminSimpleAuthPrint,			&gp_AdminPrintSimpleAuth },									// [3]プリント簡易認証.
//				{ s_KEYCertificateInspectionSetting,	&gp_AdminUserAndDepartVerifyCertificate_PKI },				// [4]証明書検証設定.
//				{ s_KEYDepartServerDownLimitSetting,	&gp_AdminUserAndDepartServerDownLimitSetting },				// [5]拡張サーバーダウン時の上限.
//				{ s_KEYCreditInfoUpdate,				&gp_AdminUserAndDepartCreditInfoUpdate },					// [6]課金情報の更新.
//				{ s_KEYAdminAuthSecuritySetting,		&gp_AdminNetworkSetSSOAuthDomainSet },						// [7]AD認証時自己検証設定.
//				{ NULL,									NULL },														// [8]なし.
//				{ NULL,									NULL },														// [9]なし.
//				{ NULL,									NULL },														// [0]なし.
//			};
//			pc_MenuUtilityAndAdmin.initialSPC( sp_UtilityAndAdminMenuData );	// メニュー画面データセット.
//		}
	}
	else {
// comment 2022/05/20 Sky西場 [IT6_2.3][OP_BTS-37620][スリム化].
		ExpPanelLog( PNLD_ExpLogKind_AlwaysLogOut, "[DeleteUnnecessaryProcessing] [%s]", __PRETTY_FUNCTION__, 0, 0, 0, 0, 0 );
//		if( 1 == uc_page ) {	// 1ページ目.
//			SP_UtilityAndAdminMenuData	sp_UtilityAndAdminMenuData[] = {
//				// シンボル                 画面ポインタ.
//				{ s_KEYAuthenticationMethod_2,			&gp_AdminUserAndDepartSynchro },							// [1]認証方式.
//				{ s_KEYUserAuthenticationSetting_2,		&gp_AdminUserAndDepartUserAttestSet },						// [2]ユーザー認証設定.
//				{ s_KEYGroupManagementSetting,			&gp_AdminUserAndDepartManageSet },							// [3]部門管理設定.
//				{ s_KEYPrintWithoutAuthentication,		pc_NonAttesPrint },											// [4]認証指定なし印刷.
//				{ s_KEYUseManagementCounterList,		&gp_AdminSystemDepartListPrintUserCounterListPrint },		// [5]使用管理カウンターリスト.
//				{ s_KEYExternalServerSetting,			&gp_AdminUserAndDepartAccountServer },						// [6]外部サーバー設定.
//				{ s_KEYRefPermit,						&gp_AdminUserAndDepartReferencePermissionSettingMenu },		// [7]参照許可設定.
//				{ s_KEYAttentionDeveiceSetting,			&gp_AdminUserAttentionDeviceSetting },						// [8]認証装置設定.
//				{ s_KEYUserGroupCommonSetting,			&gp_AdminUserAndDepartCommonSetting },						// [9]ユーザー/部門共通設定.
//				{ s_KEYHomeAddressSetting,				&gp_AdminUserAndDepartHomeAddressSetting },					// [0]Home宛先有効設定.
//			};
//			pc_MenuUtilityAndAdmin.initialSPC( sp_UtilityAndAdminMenuData );	// メニュー画面データセット.
//		}
//		else {	// 2ページ目.
//			SP_UtilityAndAdminMenuData	sp_UtilityAndAdminMenuData[] = {
//				// シンボル                 画面ポインタ.
//				{ s_KEYSendAddressPermission,			&gp_AdminUserAndDepartSendAddressPermission },				// [1]送信宛先制限.
//				{ s_SYSAdminSimpleAuthPrint,			&gp_AdminPrintSimpleAuth },									// [2]プリント簡易認証.
//				{ s_KEYCertificateInspectionSetting,	&gp_AdminUserAndDepartVerifyCertificate_PKI },				// [3]証明書検証設定.
//				{ s_KEYDepartServerDownLimitSetting,	&gp_AdminUserAndDepartServerDownLimitSetting },				// [4]拡張サーバーダウン時の上限.
//				{ s_KEYCreditInfoUpdate,				&gp_AdminUserAndDepartCreditInfoUpdate },					// [5]課金情報の更新.
//				{ NULL,									NULL },														// [6]なし.
//				{ NULL,									NULL },														// [7]なし.
//				{ NULL,									NULL },														// [8]なし.
//				{ NULL,									NULL },														// [9]なし.
//				{ NULL,									NULL }														// [0]なし.
//			};
//			pc_MenuUtilityAndAdmin.initialSPC( sp_UtilityAndAdminMenuData );	// メニュー画面データセット.
//		}
	}
}

//================================================================================================.
// 概要		:管理者設定・ユーザ認証/部門管理設定  : ページパーツ表示処理.
// 戻り値	:なし.
//================================================================================================.
void PAdminUserAndDepartSet::setupPageParts( Bool* b_Show, Bool* b_Active )
{
	for( int i = 0; i < PNLD_MENUDATANUM; i++ ) {
		if( False == b_Show[i] ) {
			pc_MenuUtilityAndAdmin.changeSPCStatus( ( uchar )( i + 1 ), PNLE_PARTSSTATUS_HIDE );
		}
		else {
			pc_MenuUtilityAndAdmin.changeSPCStatus( ( uchar )( i + 1 ), PNLE_PARTSSTATUS_ENABLE );
			if( True == b_Active[i] ) {
				pc_MenuUtilityAndAdmin.changeSPCStatus( ( uchar )( i + 1 ), PNLE_PARTSSTATUS_ACTIVATE );
			}
			else {
				pc_MenuUtilityAndAdmin.changeSPCStatus( ( uchar )( i + 1 ), PNLE_PARTSSTATUS_INACTIVATE );
			}
		}
	}
	pc_MenuUtilityAndAdmin.SPCsetupGamen();
}

//================================================================================================
// 概要		:管理者設定・ユーザ認証/部門管理メニューへの遷移共通 : 画面移動
// 戻り値	:なし
//================================================================================================
void PAdminToUserAndDepartSetcom::procGoToPage( uchar option )
{
	switch( option ) {
		case 10:	// ユーティリティ設定キー
		case 11: PToAdminSetMenucom::procGoToPage( option );	break;	// 管理者設定キー
		case 12: PAdminToUserAndDepartSetcom::procOk( 0 );		break;	// 管理者設定・ユーザ認証/部門管理
		default:
			setBuzzerType( PCGE_BuzzerType_InputNg );	break;	// 報知音設定
	}
}
//================================================================================================
// 概要		:管理者設定・ユーザ認証/部門管理メニューへの遷移共通 : 設定終了
// 戻り値	:なし
//================================================================================================
void PAdminToUserAndDepartSetcom::procOk( uchar option )
{
	nextGamen = &gp_AdminUserAndDepartSet;		// 管理者設定・ユーザ認証/部門管理メニュー
}
//================================================================================================
// 概要		:管理者設定・ユーザ認証/部門管理メニューへの遷移共通 : コールバックイベント.
// 戻り値	:なし
//================================================================================================
void PAdminToUserAndDepartSetcom::procCallBack( TYPE_PNL_CallBack option )
{
	PToAdminSetMenucom::procCallBack( option );			// 必ず親を呼ぶ
	if( option == TYPE_PNL_CallBack_DataClearAll )
	{ nextGamen = &gp_AdminUserAndDepartSet; }		// 管理者設定・ユーザ認証/部門管理メニュー
}

//====================================================================== .
// 概要   :  ボックス削除中：削除情報設定処理.
//====================================================================== .
void PAdminToUserAndDepartSetcom::setUserAndDepartBoxDelete(
    APIC_User* 			pc_setUser,
    APIC_Track* 		pc_setTrack,
    PNLE_BoxDeleteMode 	e_setBoxDeleteMode,
    PBasicMessageProc* 	pc_ReturnGamen,
    Bool 				bl_setBoxProcess )
{
	// 削除中画面へ遷移.
	nextGamen = &gp_AdminBoxDeleteWait;								// 削除中画面に遷移.
	// 変数の設定.
	e_BoxDeleteMode			= e_setBoxDeleteMode;					// ボックス削除モード.
	pc_BoxDeleteReturnGamen	= pc_ReturnGamen;						// 戻り先画面.
	pc_BoxDeleteUser		= pc_setUser;							// ユーザー情報.
	pc_BoxDeleteTrack		= pc_setTrack;							// 部門情報.
	bl_BoxProcess			= bl_setBoxProcess;
}
//====================================================================== .
// 概要   :  ボックス削除中：削除開始処理.
//====================================================================== .
void PAdminToUserAndDepartSetcom::start_UserAndDepartBoxDelete()
{
	APIC_Box* pc_Box = cp_DocList;
	int32_t i32_Ret = APID_DocapiMethodOk;
	switch( e_BoxDeleteMode ) {
		case PNLE_BoxDeleteMode_UserDelete:					// ユーザー削除.
			// 暗号化PDFボックスの削除
			PanelLog( PNLD_LogKind_Admin, "setBoxId: P1[%d]", DOCD_PDFBox, 0, 0, 0, 0, 0 );
// Commment 2017/09/22 HCL_Venkataramesh [IT5_4.2][RQ-686][WP689] setBoxId support for Network2 user.
//			pc_Box->setBoxId( DOCD_PDFBox );
			PNLC_NetworkLan::getInstance()->setBoxIdPNL( pc_Box, DOCD_PDFBox );
			// BoxID(暗号化PDF BOX)をインスタンスに設定.
			PanelLog( PNLD_LogKind_Admin, "req_deleteDocByUserId: P1[%d] P2[%s]", pc_BoxDeleteUser->getUserNo(), "ans_AdminDocByUserDeleteUser", 0, 0, 0, 0 );
			i32_Ret = pc_Box->req_deleteDocByUserId( pc_BoxDeleteUser->getUserNo(), ans_AdminDocByUserDeleteUser, PnlComGetUser(), PnlComGetTrack(), PnlComGetAuthorityID() );	// PDF削除.
			if( APID_DocapiMethodOk != i32_Ret ) {
				LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
			}
			break;
		case PNLE_BoxDeleteMode_TrackDelete:				// 部門削除.
			if( True == bl_BoxProcess ) {
				DeleteBox_Track();							// グループ・掲示板削除処理
			} else {
				ChangeBoxKind_Track();						// グループ・掲示板共有化処理
			}
			break;
		case PNLE_BoxDeleteMode_AuthInfoInit:				// 認証方法変更.
			// 暗号化PDFボックスの削除
			PanelLog( PNLD_LogKind_Admin, "setBoxId: P1[%d]", DOCD_PDFBox, 0, 0, 0, 0, 0 );
// Commment 2017/09/22 HCL_Venkataramesh [IT5_4.2][RQ-686][WP689] setBoxId support for Network2 user.
//			pc_Box->setBoxId( DOCD_PDFBox );																	// BoxID(暗号化PDF BOX)をインスタンスに設定.
			PNLC_NetworkLan::getInstance()->setBoxIdPNL( pc_Box, DOCD_PDFBox );
			PanelLog( PNLD_LogKind_Admin, "req_deleteDocByAllUserId: P1[%s]", "ans_AdminDocByUserDeleteUser", 0, 0, 0, 0, 0 );
			pc_Box->req_deleteDocByAllUserId( ans_AdminDocByUserDeleteUser, PnlComGetUser(), PnlComGetTrack(), PnlComGetAuthorityID() );			// PDF削除.
			break;
		case PNLE_BoxDeleteMode_UbiquitousDelete:			// ユビキタスプリントボックス全削除.
		case PNLE_BoxDeleteMode_UbiquitousStorageOff:		// ユビキタス ストレージ無効（ユビキタスプリントボックス削除）.
		{
			APIC_UbiquitousBox* pc_UbiquitousBox = PnlComGetUbiquitousBox();
			if( NULL == pc_UbiquitousBox ) {
//	comment 2015/11/02 HCL_Ezhil [ZeusS_ZX0][制御構成変更_No.76-1] Adding error code.
//Comment 2019/06/14 [HCLT_Monisha][EagleH][N Rank] Removed unwanted Detailed code.
//				LAPC_BFW_System *pc_BFW_System=LAPC_BFW_System::newInstance();
//				if (NULL !=pc_BFW_System){
//				    pc_BFW_System->registDetailCode(PNLD_DetailCode_Admin_0x1900650010413D01);
//				}
				LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
				return;
			}

			pc_UbiquitousBox->req_delAll( e_BoxDeleteMode, ans_AdminDocUbiquitousBoxAllDelete );	// ユビキタスボックス内文書の全ユーザの全文書の削除要求.
		}
		break;
		case PNLE_BoxDeleteMode_UbiquitousStorageOn:			// ユビキタス ストレージ有効（SMB共有フォルダー削除）.
		{
			// ユビキタス ストレージ機能を有効で設定する.
			PnlComGetUbiquitousSetting()->setStorageOn( True, &c_MioSettingNotifier );
		}
		break;
		case  PNLE_BoxDeleteMode_UbiquitousClientOn:			// ユビキタス クライアント設定有効.
		case  PNLE_BoxDeleteMode_UbiquitousClientOff:			// ユビキタス クライアント設定無効.
		{
			Bool b_ClientSet = ( PNLE_BoxDeleteMode_UbiquitousClientOn == e_BoxDeleteMode ) ? ( True ) : ( False );
			PnlComGetUbiquitousSetting()->setClientOn( b_ClientSet, &c_MioSettingNotifier );
		}
		break;
// comment 2016/06/09 ユビキタス [RQ-553][Enhancement of Max num of LK-114 devices][step2].
		case  PNLE_BoxDeleteMode_UbiquitousTopologyOn:			// ユビキタス トポロジー機能設定有効.
		case  PNLE_BoxDeleteMode_UbiquitousTopologyOff:			// ユビキタス トポロジー機能設定無効.
		{
			Bool bl_TopologySet = ( PNLE_BoxDeleteMode_UbiquitousTopologyOn == e_BoxDeleteMode ) ? ( True ) : ( False );
			PnlComGetUbiquitousSetting()->setTopologyManagerOn( bl_TopologySet, &c_MioSettingNotifier );
		}
		break;
		case PNLE_BoxDeleteMode_LargeScaleDataClear:			// 拡張ユーザーデータの削除.
		{
			APIC_User::req_deleteLargerScaleDatabase( ans_deleteLargerScaleDatabase );
		}
		break;

		default:
			LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
			// ボックス削除処理を終了させる.
			CallBackIndi( TYPE_PNL_CallBack_UserAndDepartBoxDelete_Complete );	// コールバックイベント.
			break;
	}
}

//====================================================================== .
// 概要   :  ボックス削除中：ボックス削除完了処理.
//====================================================================== .
void PAdminToUserAndDepartSetcom::end_UserAndDepartBoxDelete()
{
	switch( e_BoxDeleteMode ) {
		case PNLE_BoxDeleteMode_UserDelete:					// 個人ボックス削除.
			if( NULL != pc_BoxDeleteUser ) {
				PnlComGetAuthDeviceCommon()->AuthUserDataCancel( PnlComGetUser() );
				PnlComGetAuthDeviceCommon()->AuthUserDataCancel( PnlComGetUser(), TYPE_AuthUserData_Select2 );
				pc_BoxDeleteUser->setRegistration( False, TYPE_RegistrationTypeOnlyOne, TYPE_InspectLogIfOperationPanel, False, TYPE_Registration_State_New, False, PnlComGetUser(), PnlComGetAuthorityID() );	// 登録状態をFalse(情報削除)する.
			}
			break;
		case PNLE_BoxDeleteMode_TrackDelete:				// グループボックス削除.
			if( NULL != pc_BoxDeleteTrack ) {
//Comment 2017/09/29 [HCL-Harish] [IT5 4.2][RQ-686] [WP533/545] Multiple Network support - User/Track.
//				APIC_User* pc_User = APIC_User::newInstance( PnlComConvertDeviceUserIndex( 1 ) );		// ユーザ CommonAPIインスタンス取得
				APIC_User* pc_User = APIC_User::newInstance( PnlComConvertDeviceUserIndex( 1 ) + PnlComConvertDeviceUserIndexVLAN() );		// ユーザ CommonAPIインスタンス取得
				if( NULL != pc_User ) {
					pc_User->clearTrackNo( pc_BoxDeleteTrack->getTrackNo(),TYPE_RegistrationTypeOnlyOne ,JLGE_InspectLogIfOperationPanel );	// 引数で指定した部門番号が設定されている全ユーザーに対して部門番号をクリアする.
				}
				pc_BoxDeleteTrack->setRegistration( False, TYPE_RegistrationTypeOnlyOne, JLGE_InspectLogIfOperationPanel, False, TYPE_Registration_State_New, PnlComGetUser(), PnlComGetAuthorityID() );	// 部門の登録状態をFalse(情報削除)にする.
			}
			break;
		case PNLE_BoxDeleteMode_AuthInfoInit:				// 個人、グループボックスすべて削除.
			AuthInfoInit();
			break;
		case PNLE_BoxDeleteMode_UbiquitousDelete:			// ユビキタスプリントボックス全削除.
		case PNLE_BoxDeleteMode_UbiquitousStorageOn:		// ユビキタス ストレージ有効（SMB共有フォルダー削除）.
		case PNLE_BoxDeleteMode_UbiquitousStorageOff:		// ユビキタスボックス削除＆プリントストレージ有効.
		case PNLE_BoxDeleteMode_UbiquitousClientOn:			// ユビキタス クライアント設定有効.
		case PNLE_BoxDeleteMode_UbiquitousClientOff:		// ユビキタス クライアント設定無効.
// comment 2016/06/09 ユビキタス [RQ-553][Enhancement of Max num of LK-114 devices][step2].
		case  PNLE_BoxDeleteMode_UbiquitousTopologyOn:		// ユビキタス トポロジー機能設定有効.
		case  PNLE_BoxDeleteMode_UbiquitousTopologyOff:		// ユビキタス トポロジー機能設定無効.
		case PNLE_BoxDeleteMode_LargeScaleDataClear:		// 拡張ユーザーデータの削除.
			break;

		default:
			LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
			break;
	}
	// 戻り先画面に遷移.
	nextGamen = pc_BoxDeleteReturnGamen;

	// ボックス削除に必要な情報の初期化.
	e_BoxDeleteMode			= PNLE_BoxDeleteMode_Non;				// ボックス削除モード.
	pc_BoxDeleteReturnGamen	= NULL;									// 戻り先画面.
	pc_BoxDeleteUser		= NULL;									// ユーザー情報.
	pc_BoxDeleteTrack		= NULL;									// 部門情報.
	bl_BoxProcess			= False;

	// リストの初期化(通常はリストに情報は残っていないはず).
	c_BoxDeleteList.Lock();
	APIC_BoxList* c_PtrBoxList = c_BoxDeleteList.findFirst();		// リスト情報の先頭の情報をc_PtrBoxListへコピー.
	while( NULL != c_PtrBoxList ) {
		c_BoxDeleteList.del( c_PtrBoxList );						// リストから削除.
		free( c_PtrBoxList );										// 取得領域解放.
		c_PtrBoxList = c_BoxDeleteList.findFirst();					// 先頭リスト取得.
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
	}
	c_BoxDeleteList.UnLock();
}
//====================================================================== .
// 概要   :  ボックス削除中：個別ボックス削除処理
//====================================================================== .
void PAdminToUserAndDepartSetcom::req_UserAndDepartBoxDelete_UserTrack()
{
	c_BoxDeleteList.Lock();
	APIC_BoxList* c_PtrBoxList = c_BoxDeleteList.findFirst();								// リスト情報の先頭の情報をc_PtrBoxListへコピー.
	// 削除対象のボックスが存在.
	if( NULL != c_PtrBoxList ) {
		// ボックス削除要求.
		int32_t i32_result = APID_DocapiMethodOk;										// 結果.
		APIC_User* pc_Admin = APIC_User::newInstance( TYPD_IndexUserAdmin );
		i32_result = cp_BoxList->req_deleteBox(	pc_Admin,							// ユーザーインスタンス.
		                                        c_PtrBoxList->sc_BoxPassword,		// ボックスパスワード.
		                                        c_PtrBoxList->sl_BoxID,				// ボックスID.
		                                        ans_UserAndDepartBoxDelete_Indi,	// コールバック関数.
		                                        TYPD_DefaultNonBox,					// ボックス種別(指定なし).
//												NULL );								// 部門管理インスタンス.
		                                        NULL,								// 部門管理インスタンス.
		                                        TYPE_RegistrationTypeOnlyOne,		// 監査ログタイプ.
		                                        PnlComGetAuthorityID() );			// ユーザー識別ID.
		// 要求NGの場合.
		if( APID_DocapiMethodOk != i32_result ) {
			LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
			CallBackIndi( TYPE_PNL_CallBack_UserAndDepartBoxDelete_Indi );			// コールバックイベント.
		}
		c_BoxDeleteList.del( c_PtrBoxList );										// 先頭の情報を消すので、次の情報が先頭に来る.
		free( c_PtrBoxList );														// 取得領域解放.
	} else {
		// 全ボックスの個別削除完了.
		CallBackIndi( TYPE_PNL_CallBack_UserAndDepartBoxDelete_Complete );			// コールバックイベント.
	}
	c_BoxDeleteList.UnLock();
}

//----------------------------------------------------------------------------------------------------------------------//
//====================================================================== .
// 概要   :  ボックス削除中：個人・掲示板ボックス削除処理.
//====================================================================== .
void PAdminToUserAndDepartSetcom::DeleteBox_User()
{
	if( NULL != pc_BoxDeleteUser ) {
		// 削除対象のBoxリスト作成.
		cp_BoxList->getBoxList( pc_BoxDeleteUser, 0, -1, 1, 0, TYPE_SearchKeyAA, &c_BoxDeleteList, False );			// ユーザが作ったボックスリスト情報を取得.
		cp_BoxList->getSpecialBoxListByUser( pc_BoxDeleteUser, 13, 0, -1, 1, &c_BoxDeleteList, LIBE_CharSetDefault );	// ユーザが作ったボックスリスト情報を取得.
		// ボックス削除処理.
		req_UserAndDepartBoxDelete_UserTrack();
	}
}

//====================================================================== .
// 概要   :  ボックス削除中：個人・掲示板ボックス共有化処理.
//====================================================================== .
void PAdminToUserAndDepartSetcom::ChangeBoxKind_User()
{
	APIC_User* pc_Admin	= APIC_User::newInstance( TYPD_IndexUserAdmin );		// 管理者インスタンス取得.
	APIC_BoxInfo c_tmpBoxInfo;	// ボックス情報.
	APIC_BoxOption c_BoxOption;	// オプション情報.
	APIC_BoxList* c_PtrBoxList;	// ボックスリスト情報.
	LIBC_List<APIC_BoxList> c_BoxList;
	int32_t i32_ret = APID_DocapiMethodOk;
	int32_t i32_Count = 0;			// 変更をするボックス数.
	cp_BoxList->getBoxList( pc_BoxDeleteUser, 0, -1, 1, 0, TYPE_SearchKeyAA, &c_BoxList, False );	// ユーザが作ったボックスリスト情報を取得.
	cp_BoxList->getMaxBoxCnt( pc_BoxDeleteUser, &i32_Count );	// ユーザが作ったボックス数を取得.
// comment 2017/02/02 SCD2-Sky浜野 [IT5_3.4][1013451].
//	cp_BoxList->updateConfirmationPrintUserID( pc_BoxDeleteUser );	// No42597 ドキュメント内のユーザ更新.
//【CQ510665】【RQ-218CostCenterの再設定】 HCL-Surya 2014/11/24 Add start.
	char * psc_costcid = NULL;
	if(PNLC_ActionDataLib::newInstance() != NULL){
		psc_costcid = PNLC_ActionDataLib::newInstance()->getCostCenterID();
	}
//【CQ510665】【RQ-218CostCenterの再設定】 HCL-Surya 2014/11/24 Add End.
	for( int32_t i = 0; i < i32_Count; i++ ) {
		c_PtrBoxList = c_BoxList.findFirst();		// リスト情報の先頭の情報をc_PtrBoxListへコピー.
		if( NULL != c_PtrBoxList ) {
			PanelLog( PNLD_LogKind_Admin, "setBoxId: P1[%d]", c_PtrBoxList->sl_BoxID, 0, 0, 0, 0, 0 );
// Commment 2017/09/22 HCL_Venkataramesh [IT5_4.2][RQ-686][WP689] setBoxId support for Network2 user.
//			cp_DocList->setBoxId( c_PtrBoxList->sl_BoxID );				// ボックスIDをセット.
			PNLC_NetworkLan::getInstance()->setBoxIdPNL( cp_DocList, c_PtrBoxList->sl_BoxID );
			//選択ボックス情報取得.
			PanelLog( PNLD_LogKind_Admin, "getBoxInfo P1[%p] P2[%p]", pc_BoxDeleteUser, &c_tmpBoxInfo, 0, 0, 0, 0 );
			i32_ret = cp_DocList->getBoxInfo( pc_BoxDeleteUser, &c_tmpBoxInfo );
			c_tmpBoxInfo.ul_OwnUserID = APID_BoxUserID_Public;			// 所有者をpublicにする.
			//選択ボックス情報取得.
			PanelLog( PNLD_LogKind_Admin, "getBoxOption: P1[%p] P2[%s] P3[%p]", pc_BoxDeleteUser, "****", &c_BoxOption, 0, 0, 0 );
			i32_ret = cp_DocList->getBoxOption( pc_BoxDeleteUser, c_PtrBoxList->sc_BoxPassword, &c_BoxOption );
			c_BoxOption.sl_BoxCreateInfo = APIE_BoxmakerAuto;			// BOX属性を共有にする.
			//ボックス情報設定.
			PanelLog( PNLD_LogKind_Admin, "changeBoxKind: P1[%p] P2[%s] P3[%p] P4[%p]", pc_Admin, "****", &c_tmpBoxInfo, &c_BoxOption, 0, 0 );
			i32_ret = cp_DocList->changeBoxKind( pc_Admin, c_PtrBoxList->sc_BoxPassword, &c_tmpBoxInfo, &c_BoxOption,
//【CQ510665】【RQ-218CostCenterの再設定】 HCL-Surya 2014/11/24 Mod Start.
//												LIBE_CharSetDefault, APID_DocapiNull, TYPE_RegistrationTypeOnlyOne, APID_DocapiNull, APID_DocapiNull, False, PnlComGetAuthorityID() );
												LIBE_CharSetDefault, APID_DocapiNull, TYPE_RegistrationTypeOnlyOne, APID_DocapiNull, APID_DocapiNull, False, PnlComGetAuthorityID(),psc_costcid );
//【CQ510665】【RQ-218CostCenterの再設定】 HCL-Surya 2014/11/24 Mod End.
			c_BoxList.del( c_PtrBoxList );	// 先頭の情報を消すので、次の情報が先頭に来る.
			// 取得領域解放
			free( c_PtrBoxList );
		}
	}
	// 個人掲示板.
	cp_BoxList->getSpecialBoxListByUser( pc_BoxDeleteUser, 13, 0, -1, 1, &c_BoxList, LIBE_CharSetDefault );	// ユーザが作ったボックスリスト情報を取得.
	i32_Count = c_BoxList.getCount();
// comment 2017/02/02 SCD2-Sky浜野 [IT5_3.4][1013451].
//	cp_BoxList->updateConfirmationPrintUserID( pc_BoxDeleteUser );
	for( int32_t i = 0; i < i32_Count; i++ ) {
		c_PtrBoxList = c_BoxList.findFirst();		// リスト情報の先頭の情報をc_PtrBoxListへコピー.
		if( NULL != c_PtrBoxList ) {
			PanelLog( PNLD_LogKind_Admin, "setBoxId: P1[%d]", c_PtrBoxList->sl_BoxID, 0, 0, 0, 0, 0 );
// Commment 2017/09/22 HCL_Venkataramesh [IT5_4.2][RQ-686][WP689] setBoxId support for Network2 user.
//			cp_DocList->setBoxId( c_PtrBoxList->sl_BoxID );				// ボックスIDをセット.
			PNLC_NetworkLan::getInstance()->setBoxIdPNL( cp_DocList, c_PtrBoxList->sl_BoxID );
			//選択ボックス情報取得.
			PanelLog( PNLD_LogKind_Admin, "getBoxInfo P1[%p] P2[%p]", pc_BoxDeleteUser, &c_tmpBoxInfo, 0, 0, 0, 0 );
			i32_ret = cp_DocList->getBoxInfo( pc_BoxDeleteUser, &c_tmpBoxInfo );
			c_tmpBoxInfo.ul_OwnUserID = APID_BoxUserID_Public;			// 所有者をpublicにする.
			//選択ボックス情報取得.
			PanelLog( PNLD_LogKind_Admin, "getBoxOption: P1[%p] P2[%s] P3[%p]", pc_BoxDeleteUser, "****", &c_BoxOption, 0, 0, 0 );
			i32_ret = cp_DocList->getBoxOption( pc_BoxDeleteUser, c_PtrBoxList->sc_BoxPassword, &c_BoxOption );
			c_BoxOption.sl_BoxCreateInfo = APIE_BoxmakerAuto;			// BOX属性を共有にする.
			//ボックス情報設定.
			PanelLog( PNLD_LogKind_Admin, "changeBoxKind: P1[%p] P2[%s] P3[%p] P4[%p]", pc_Admin, "****", &c_tmpBoxInfo, &c_BoxOption, 0, 0 );
			i32_ret = cp_DocList->changeBoxKind( pc_Admin, c_PtrBoxList->sc_BoxPassword, &c_tmpBoxInfo, &c_BoxOption,
//【CQ510665】【RQ-218CostCenterの再設定】 HCL-Surya 2014/11/24 Mod Start.
//												LIBE_CharSetDefault, APID_DocapiNull, TYPE_RegistrationTypeOnlyOne, APID_DocapiNull, APID_DocapiNull, False, PnlComGetAuthorityID() );
												LIBE_CharSetDefault, APID_DocapiNull, TYPE_RegistrationTypeOnlyOne, APID_DocapiNull, APID_DocapiNull, False, PnlComGetAuthorityID(),psc_costcid );
//【CQ510665】【RQ-218CostCenterの再設定】 HCL-Surya 2014/11/24 Mod End.
			// リスト解放
			c_BoxList.del( c_PtrBoxList );	// 先頭の情報を消すので、次の情報が先頭に来る.
			// 取得領域解放
			free( c_PtrBoxList );
		}
	}
	if( APID_DocapiMethodOk != i32_ret ) { PanelLog( PNLD_LogKind_Admin, " RET[%d]",  i32_ret, 0, 0, 0, 0, 0 ); }
	CallBackIndi( TYPE_PNL_CallBack_UserAndDepartBoxDelete_Complete );			// コールバックイベント.
}
//====================================================================== .
// 概要   :  ボックス削除中：グループ・掲示板ボックス削除処理.
//====================================================================== .
void PAdminToUserAndDepartSetcom::DeleteBox_Track()
{
	if( NULL != pc_BoxDeleteTrack ) {
		// 削除対象のBoxリスト作成.
		cp_BoxList->getBoxList( NULL, 0, -1, 1, 0, TYPE_SearchKeyAA, &c_BoxDeleteList, False, LIBE_CharSetDefault, pc_BoxDeleteTrack );	// 部門が作った個人ボックスのリスト情報を取得.
		cp_BoxList->getSpecialBoxListByUser( NULL, 13, 0, -1, 1, &c_BoxDeleteList, LIBE_CharSetDefault, pc_BoxDeleteTrack );			// 部門が作った掲示板ボックスリスト情報を取得.
		// ボックス削除処理.
		req_UserAndDepartBoxDelete_UserTrack();
	}
}
//====================================================================== .
// 概要   :  ボックス削除中：グループ・掲示板ボックス共有化処理.
//====================================================================== .
void PAdminToUserAndDepartSetcom::ChangeBoxKind_Track()
{
	APIC_User* pc_Admin	= APIC_User::newInstance( TYPD_IndexUserAdmin );		// 管理者インスタンス取得.
	APIC_BoxInfo c_tmpBoxInfo;	// ボックス情報.
	APIC_BoxOption c_BoxOption;	// オプション情報.
	APIC_BoxList* c_PtrBoxList;	// ボックスリスト情報.
	LIBC_List<APIC_BoxList> c_BoxList;
	int32_t i32_ret = APID_DocapiMethodOk;
	int32_t i32_Count = 0;			// 変更をするボックス数.
//【CQ510665】【RQ-218CostCenterの再設定】 HCL-Surya 2014/11/24 Add start.
	char * psc_costcid = NULL;
	if(PNLC_ActionDataLib::newInstance() != NULL){
		psc_costcid = PNLC_ActionDataLib::newInstance()->getCostCenterID();
	}
//【CQ510665】【RQ-218CostCenterの再設定】 HCL-Surya 2014/11/24 Add End.
	//////////////////////
	//	通常ボックスの削除.
	// 部門が作ったボックスリスト情報、ボックス数を取得.
	cp_BoxList->getBoxList( NULL, 0, -1, 1, &c_BoxList, LIBE_CharSetDefault, pc_BoxDeleteTrack );
	i32_Count = c_BoxList.getCount();
	for( int32_t i = 0; i < i32_Count; i++ ) {
		c_PtrBoxList = c_BoxList.findFirst();		// リスト情報の先頭の情報をc_PtrBoxListへコピー.
		if( NULL != c_PtrBoxList ) {
			PanelLog( PNLD_LogKind_Admin, "setBoxId: P1[%d]", c_PtrBoxList->sl_BoxID, 0, 0, 0, 0, 0 );
// Commment 2017/09/22 HCL_Venkataramesh [IT5_4.2][RQ-686][WP689] setBoxId support for Network2 user.
//			cp_DocList->setBoxId( c_PtrBoxList->sl_BoxID );				// ボックスIDをセット.
			PNLC_NetworkLan::getInstance()->setBoxIdPNL( cp_DocList, c_PtrBoxList->sl_BoxID );
			//選択ボックス情報取得.
			PanelLog( PNLD_LogKind_Admin, "getBoxInfo P1[%p] P2[%p] P3[%d] P4[%p]", NULL, &c_tmpBoxInfo, -1, pc_BoxDeleteTrack, 0, 0 );
			i32_ret = cp_DocList->getBoxInfo( NULL, &c_tmpBoxInfo, LIBE_CharSetDefault, pc_BoxDeleteTrack );
			c_tmpBoxInfo.ul_OwnUserID = APID_BoxUserID_Public;			// 所有者をpublicにする.
			//選択ボックス情報取得.
			PanelLog( PNLD_LogKind_Admin, "getBoxOption: P1[%p] P2[%s] P3[%p] P4[%p]", NULL, "****", &c_BoxOption, pc_BoxDeleteTrack, 0, 0 );
			i32_ret = cp_DocList->getBoxOption( NULL, c_PtrBoxList->sc_BoxPassword, &c_BoxOption, pc_BoxDeleteTrack );
			c_BoxOption.sl_BoxCreateInfo = APIE_BoxmakerAuto;			// BOX属性を共有にする.
			//ボックス情報設定.
			PanelLog( PNLD_LogKind_Admin, "changeBoxKind: P1[%p] P2[%s] P3[%p] P4[%p]", pc_Admin, "****", &c_tmpBoxInfo, &c_BoxOption, 0, 0 );
			i32_ret = cp_DocList->changeBoxKind( pc_Admin,
			                                    c_PtrBoxList->sc_BoxPassword,
			                                    &c_tmpBoxInfo,
//												&c_BoxOption);
			                                    &c_BoxOption,
			                                    LIBE_CharSetDefault,
			                                    APID_DocapiNull,
			                                    TYPE_RegistrationTypeOnlyOne,
			                                    APID_DocapiNull,
			                                    APID_DocapiNull,
			                                    False,
//【CQ510665】【RQ-218CostCenterの再設定】 HCL-Surya 2014/11/24 Mod Start.
//												PnlComGetAuthorityID() );
												PnlComGetAuthorityID(),
												psc_costcid );
//【CQ510665】【RQ-218CostCenterの再設定】 HCL-Surya 2014/11/24 Mod End.
			// リスト解放
			c_BoxList.del( c_PtrBoxList );	// 先頭の情報を消すので、次の情報が先頭に来る.
			// 取得領域解放
			free( c_PtrBoxList );
		}
	}
	//////////////////////
	//	掲示板ボックスの削除.
	// 部門が作った掲示板ボックスリスト情報、ボックス数を取得.
	cp_BoxList->getSpecialBoxListByUser( NULL, 13, 0, -1, 1, &c_BoxList, LIBE_CharSetDefault, pc_BoxDeleteTrack );
	i32_Count = c_BoxList.getCount();
	for( int32_t i = 0; i < i32_Count; i++ ) {
		c_PtrBoxList = c_BoxList.findFirst();		// リスト情報の先頭の情報をc_PtrBoxListへコピー.
		if( NULL != c_PtrBoxList ) {
			PanelLog( PNLD_LogKind_Admin, "setBoxId: P1[%d]", c_PtrBoxList->sl_BoxID, 0, 0, 0, 0, 0 );
// Commment 2017/09/22 HCL_Venkataramesh [IT5_4.2][RQ-686][WP689] setBoxId support for Network2 user.
//			cp_DocList->setBoxId( c_PtrBoxList->sl_BoxID );				// ボックスIDをセット.
			PNLC_NetworkLan::getInstance()->setBoxIdPNL( cp_DocList, c_PtrBoxList->sl_BoxID );
			//選択ボックス情報取得.
			PanelLog( PNLD_LogKind_Admin, "getBoxInfo P1[%p] P2[%p] P3[%d] P4[%p]", NULL, &c_tmpBoxInfo, -1, pc_BoxDeleteTrack, 0, 0 );
			i32_ret = cp_DocList->getBoxInfo( NULL, &c_tmpBoxInfo, LIBE_CharSetDefault, pc_BoxDeleteTrack );
			c_tmpBoxInfo.ul_OwnUserID = APID_BoxUserID_Public;			// 所有者をpublicにする.
			//選択ボックス情報取得.
			PanelLog( PNLD_LogKind_Admin, "getBoxOption: P1[%p] P2[%s] P3[%p] P4[%p]", NULL, "****", &c_BoxOption, pc_BoxDeleteTrack, 0, 0 );
			i32_ret = cp_DocList->getBoxOption( NULL, c_PtrBoxList->sc_BoxPassword, &c_BoxOption, pc_BoxDeleteTrack );
			c_BoxOption.sl_BoxCreateInfo = APIE_BoxmakerAuto;			// BOX属性を共有にする.
			//ボックス情報設定.
			PanelLog( PNLD_LogKind_Admin, "changeBoxKind: P1[%p] P2[%s] P3[%p] P4[%p]", pc_Admin, "****", &c_tmpBoxInfo, &c_BoxOption, 0, 0 );
			i32_ret = cp_DocList->changeBoxKind( pc_Admin, c_PtrBoxList->sc_BoxPassword, &c_tmpBoxInfo, &c_BoxOption,
//【CQ510665】【RQ-218CostCenterの再設定】 HCL-Surya 2014/11/24 Mod Start.
//												LIBE_CharSetDefault, APID_DocapiNull, TYPE_RegistrationTypeOnlyOne, APID_DocapiNull, APID_DocapiNull, False, PnlComGetAuthorityID() );
												LIBE_CharSetDefault, APID_DocapiNull, TYPE_RegistrationTypeOnlyOne, APID_DocapiNull, APID_DocapiNull, False, PnlComGetAuthorityID(),psc_costcid );
//【CQ510665】【RQ-218CostCenterの再設定】 HCL-Surya 2014/11/24 Mod End.
			// リスト解放
			c_BoxList.del( c_PtrBoxList );	// 先頭の情報を消すので、次の情報が先頭に来る.
			// 取得領域解放
			free( c_PtrBoxList );
		}
	}
	if( APID_DocapiMethodOk != i32_ret ) { PanelLog( PNLD_LogKind_Admin, " RET[%d]",  i32_ret, 0, 0, 0, 0, 0 ); }
	CallBackIndi( TYPE_PNL_CallBack_UserAndDepartBoxDelete_Complete );			// コールバックイベント.
}
//====================================================================== .
// 概要   :  ボックス削除中：認証情報初期化
//====================================================================== .
void PAdminToUserAndDepartSetcom::AuthInfoInit()
{
	APIC_User* pc_User1 = NULL;
	APIC_Track* pc_Track = NULL;
	APIC_TrackCnt* pc_TrackCnt = NULL;

	cp_BoxList->updateConfirmationPrintAllUserID();							// No48730 ドキュメント内の全ユーザをPublicに変更.
//Comment 2017/09/04 [HCL-Harish] [IT5 4.2][RQ-686] [WP509] Multiple Network support.
	TYPE_NetworkSpecific e_NetworkSpecific  = PNLC_NetworkLan::getInstance()->getAdminNWLine( ) ;
	// ユーザ情報のクリア.
//	if( TYPE_UserAuthenticationIntermeddiateServer == PnlComGetUserAuthentication()->getUserAuthentication() ) {	// 中間認証.
	if( TYPE_UserAuthenticationIntermeddiateServer == PnlComGetUserAuthentication()->getUserAuthentication( e_NetworkSpecific ) ) {	// 中間認証.
		pc_Track = APIC_Track::newInstance( TYPD_IndexTrackPublic );
		pc_TrackCnt = APIC_TrackCnt::newInstance( TYPD_IndexTrackPublic );
// Comment 2017/09/12【HCLT-Sindhu】【IT5 4.2】【RQ686】【WP1012】Public user support for Network 2.
//		pc_User1 = APIC_User::newInstance( TYPD_IndexUserPublic );
		if( TYPE_NetworkSpecific_NW1  ==  PNLC_NetworkLan::getInstance() ->getAdminNWLine( ) ) {
   			pc_User1 = APIC_User::newInstance( TYPD_IndexUserPublic );
		} else {
   			pc_User1 = APIC_User::newInstance( TYPD_IndexUserPublicNW2 );
		}
	} else {
//Comment 2017/09/29 [HCL-Harish] [IT5 4.2][RQ-686] [WP533/545] Multiple Network support - User/Track.
//		pc_Track = APIC_Track::newInstance( 1 );
		pc_Track = APIC_Track::newInstance( 1 + PnlComConvertDeviceUserIndexVLAN() );
//【CQ510665】【RQ-218CostCenterの再設定】 HCL-Surya 2014/11/24 Add Start.
		PNLC_ActionDataLib* pc_ActionDataLib = PNLC_ActionDataLib::newInstance();
		if( NULL != pc_ActionDataLib ){
//Comment 2017/09/29 [HCL-Harish] [IT5 4.2][RQ-686] [WP533/545] Multiple Network support - User/Track.
//			pc_TrackCnt = APIC_TrackCnt::newInstance( 1,pc_ActionDataLib->getCostCenterID() );
			pc_TrackCnt = APIC_TrackCnt::newInstance( 1 + PnlComConvertDeviceUserIndexVLAN() ,pc_ActionDataLib->getCostCenterID() );
		}else{
//【CQ510665】【RQ-218CostCenterの再設定】 HCL-Surya 2014/11/24 Add End.
//Comment 2017/09/29 [HCL-Harish] [IT5 4.2][RQ-686] [WP533/545] Multiple Network support - User/Track.
//		pc_TrackCnt = APIC_TrackCnt::newInstance( 1 );
		pc_TrackCnt = APIC_TrackCnt::newInstance( 1 + PnlComConvertDeviceUserIndexVLAN());
		}//【CQ510665】【RQ-218CostCenterの再設定】 HCL-Surya 2014/11/24 Add.
//Comment 2017/09/29 [HCL-Harish] [IT5 4.2][RQ-686] [WP533/545] Multiple Network support - User/Track.
//		pc_User1 = APIC_User::newInstance( PnlComConvertDeviceUserIndex( 1 ) );
		pc_User1 = APIC_User::newInstance( PnlComConvertDeviceUserIndex( 1 ) + PnlComConvertDeviceUserIndexVLAN() );
	}
	// ユーザ情報のクリア.
	if( NULL != pc_User1 ) {
		pc_User1->factoryReset();	// 工場出荷時設定にする.
	}
	// 部門情報のクリア.
	if( NULL != pc_Track ) {
		pc_Track->factoryReset();	// 工場出荷時設定にする.
	}
	// カウンタのクリア.
	if( NULL != pc_TrackCnt ) {
		pc_TrackCnt->factoryReset();	// 工場出荷時設定にする.
	}
}
//====================================================================== .
// 概要   :  ボックス削除中：削除モード取得処理.
//====================================================================== .
PNLE_BoxDeleteMode PAdminToUserAndDepartSetcom::getBoxDeleteMode()
{
	return e_BoxDeleteMode;
}
//====================================================================== .
// 概要   :  ボックス削除中：削除ユーザー取得処理.
//====================================================================== .
APIC_User* PAdminToUserAndDepartSetcom::getBoxDeleteUser()
{
	return pc_BoxDeleteUser;
}
//====================================================================== .
// 概要   :  ボックス削除中：削除部門取得処理.
//====================================================================== .
APIC_Track*	PAdminToUserAndDepartSetcom::getBoxDeleteTrack()
{
	return pc_BoxDeleteTrack;
}
//====================================================================== .
// 概要   :  個人・グループ・掲示板削除フラグ取得処理.(True = 削除 False = 共有化).
//====================================================================== .
Bool PAdminToUserAndDepartSetcom::getBoxDeleteProcess()
{
	return bl_BoxProcess;
}
//====================================================================== .
// 概要   :  現在設定されているユーザーカウンター数を返す.
//====================================================================== .
short PAdminToUserAndDepartSetcom::GetUserCounter()
{
	TYPE_UserAuthentication UserAuthentication = TYPE_UserAuthenticationOff;


	//-------------------------------------------------------.
	// 本APIはZeusBK(基本)以降で有効なAPI.
	// それ以前の機種については処理を行わず、常に0を返す.
	//-------------------------------------------------------.
	short ss_UserCnt = 0;

	if( True == PLBC_FuncVersionCheck::isOK_MiZe_Type2() ) {

		// 認証方式、部門管理設定を取得する.
// 	Comment 2017/09/28 [HCL-Harish]【IT5 4.2】【RQ-686】【WP509】Multiple Network support.
		TYPE_NetworkSpecific e_NetworkSpecific  = PNLC_NetworkLan::getInstance()->getAdminNWLine( ) ;
//		UserAuthentication = PnlComGetUserAuthentication()->getUserAuthentication();
//		TrackMode = PnlComGetTrackManager()->getTrackMode();
//Comment 2017/10/29 [HCL-Harish] [IT5 4.2][WP1146] User Registeration and track - Counter.
		UserAuthentication = PnlComGetUserAuthentication()->getUserAuthentication( e_NetworkSpecific );
//		TrackMode = PnlComGetTrackManager()->getTrackMode(PNLC_NetworkLan::getInstance() ->getAdminNWLine( ));

		// 設定されている認証方式、部門管理設定からユーザーカウンター数を算出する.
		if( TYPE_UserAuthenticationDeviceAndMidAuth == UserAuthentication ) {
			// 「本体+拡張サーバー認証」.
//Comment 2017/09/29 [HCL-Harish] [IT5 4.2][RQ-686] [WP533/545] Multiple Network support - User/Track.
//			ss_UserCnt = PnlComGetTrackManager()->getMaxTrack() - PnlComGetTrackManager()->getTrackCntNo() - PnlComGetTrackManager()->getCacheCntNo();
			ss_UserCnt = GetMaxTrackCounter() - PnlComGetTrackManager()->getTrackCntNo() - PnlComGetTrackManager()->getCacheCntNo();
		}
		else {
//Comment 2017/09/29 [HCL-Harish] [IT5 4.2][RQ-686] [WP533/545] Multiple Network support - User/Track.
//			ss_UserCnt = PnlComGetTrackManager()->getMaxTrack() - PnlComGetTrackManager()->getTrackCntNo();
			if ( TYPE_NetworkSpecific_NW1 == e_NetworkSpecific) {
				ss_UserCnt = GetMaxTrackCounter() - PnlComGetTrackManager()->getTrackCntNo();
			} else {
				ss_UserCnt = GetMaxTrackCounter();
			}
		}
	}

	return ss_UserCnt;
}
//Comment 2017/09/04 [HCL-Harish] [IT5 4.2][RQ-686] [WP533] Multiple Network support - To Get Applicable User Count at the Time of VLAN.
short PAdminToUserAndDepartSetcom::GetMaxTrackCounter()
{
	short ss_UserCnt = 0;

// Comment 2018/01/29 HCLT_Hanuman [IT5_4.2][AR1038061] To improve performance.
//	if (True == APIC_VlanSetting::newInstance()->getVlanEnable()) {
	if ( True ==  PNLC_NetworkLan::getInstance()->getVlanEnableWrapper() ) {
//Comment 2017/10/29 [HCL-Harish] [IT5 4.2][WP1146] User Registeration and track - Counter.
//		if ( TYPE_UserAuthenticationOff != PnlComGetUserAuthentication()->getUserAuthentication( TYPE_NetworkSpecific_NW2 )
//			&& TYPE_UserAuthenticationOff != PnlComGetUserAuthentication()->getUserAuthentication( TYPE_NetworkSpecific_NW1 ) ) {
//			ss_UserCnt = PnlComGetTrackManager()->getMaxTrack() / 2;
// Comment 2018/01/18 HCLT_Binesh [IT5_4.2][AR1034977] User count value is obtained from library when NW1 user authentication is not set to API.
//		if ( TYPE_NetworkSpecific_NW1 == PNLC_NetworkLan::getInstance() ->getAdminNWLine( ) ) {
//			ss_UserCnt = PnlComGetTrackManager()->getMaxTrack() - PnlComGetTrackManager()->getNW2UserCntNo();
		if ( TYPE_NetworkSpecific_NW1 == PNLC_NetworkLan::getInstance() ->getAdminNWLine( ) ) {
			if ( TYPE_UserAuthenticationOff != PnlComGetUserAuthentication()->getUserAuthentication( TYPE_NetworkSpecific_NW2 )
				&&  TYPE_UserAuthenticationOff == PnlComGetUserAuthentication()->getUserAuthentication( TYPE_NetworkSpecific_NW1 ) ) {
					ss_UserCnt = PnlComGetTrackManager()->getMaxTrack() - PNLC_NetworkLan::getInstance()->getNW2UserCntNoLib();					
			} else {
					ss_UserCnt = PnlComGetTrackManager()->getMaxTrack() - PnlComGetTrackManager()->getNW2UserCntNo();
			}
		} else {
//			ss_UserCnt = PnlComGetTrackManager()->getMaxTrack();
			ss_UserCnt = PnlComGetTrackManager()->getNW2UserCntNo();
		}
	} else {
		ss_UserCnt = PnlComGetTrackManager()->getMaxTrack();
	} 
	return ss_UserCnt;
}

short PAdminToUserAndDepartSetcom::GetMaxUserCounter()
{
	short ss_UserCnt = 0;

// Comment 2018/01/29 HCLT_Hanuman [IT5_4.2][AR1038061] To improve performance.
//	if (True == APIC_VlanSetting::newInstance()->getVlanEnable()) {
	if ( True ==  PNLC_NetworkLan::getInstance()->getVlanEnableWrapper() ) {
//Comment 2017/10/29 [HCL-Harish] [IT5 4.2][WP1146] User Registeration and track - Counter.
//		if ( TYPE_UserAuthenticationOff != PnlComGetUserAuthentication()->getUserAuthentication( TYPE_NetworkSpecific_NW2 )
//			&& TYPE_UserAuthenticationOff != PnlComGetUserAuthentication()->getUserAuthentication( TYPE_NetworkSpecific_NW1 ) ) {
//			ss_UserCnt = PnlComGetUserAuthentication()->getMaxUser() / 2;
		if ( TYPE_NetworkSpecific_NW1 == PNLC_NetworkLan::getInstance() ->getAdminNWLine( ) ) {
			ss_UserCnt = PnlComGetUserAuthentication()->getMaxUser() - PnlComGetTrackManager()->getNW2UserCntNo();
		} else {
//			ss_UserCnt = PnlComGetUserAuthentication()->getMaxUser();
			ss_UserCnt = PnlComGetTrackManager()->getNW2UserCntNo();
		}
	} else {
		ss_UserCnt = PnlComGetUserAuthentication()->getMaxUser();
	} 
	return ss_UserCnt;
}
////////////////////////////////////////////////////////////////
Bool  PAdminUserAndDepartSynchro::b_AuthCardServerSearching = False;			// 認証カード検索機能On/Off.
uchar PAdminUserAndDepartSynchro::uc_AuthCardServerIndex = 0;					// 連携する認証カードサーバーのインデックス.
APIS_IccLDAPParams PAdminUserAndDepartSynchro::s_AuthCardParams = {0};			// 認証カード情報サーバー設定.
APIS_IccLDAPParams PAdminUserAndDepartSynchro::m_s_AuthCardSecondaryServerParams = {0};	// 認証カード情報代替サーバー設定.
// comment 2015/04/17 OPS1-MSE渡邊(哲) [MinervaSBK][RQ-278] LDAPサーバーへのカード情報登録機能対応.
Bool  PAdminUserAndDepartSynchro::m_bl_CardRegistration = False;				// カード情報登録設定On/Off.
TYPE_ServerGroupCardSearch PAdminUserAndDepartSynchro::m_e_CardAttr = TYPE_ServerGroupCardSearch_CapitalAndSmall;
// comment 2016/5/25 [AUTHFUM2] [RQ-526]外部サーバーLDAP検索高速化 第22製品開発 山本.
Bool PAdminUserAndDepartSynchro::b_ExtServerDNCache = False;					// 外部サーバーDNキャッシュ ON/OFF.
// comment 2016/5/25 [AUTHFUM2] [RQ-531]本体認証データ登録拡張 第22製品開発 山本.
Bool PAdminUserAndDepartSynchro::b_AuthInsideLargeScale = False;				// 拡張ユーザーDB ON/OFF.
Bool PAdminUserAndDepartSynchro::b_LScaleClear = False;							// 拡張ユーザーDBの削除フラグ.
Bool PAdminUserAndDepartSynchro::b_LScaleSettingClear = False;					// 拡張ユーザーDB設定のクリアフラグ.

//================================================================================================
// 概要		:管理者設定・ユーザ認証/部門管理・ユーザ/部門の連動認証(2691) : コンストラクタ
// 戻り値	:なし
//================================================================================================
PAdminUserAndDepartSynchro::PAdminUserAndDepartSynchro( PDisplayParts** partsTable, short partsNumber, Bool isNoWarningGamen )	// コンストラクタ
	: PAdminToUserAndDepartSetcom( partsTable, partsNumber, isNoWarningGamen )
{
	sc_UserAttest = 2;			// しない、外部、本体.
	ss_PublicUser = 0;			// パブリックユーザ許可する、しない.
	b_Manage = True;			// 部門管理する、しない.
	ss_departForm = -1;			// 部門名+パスワード、パスワードのみ.
	b_Synchro = False;			// 連動する/しない.
	sc_LimitArrive = 1;			// 上限値到達時動作.
	sc_NowPage = 1;				// 現在のページ.
	ss_UserCounter = 50;		// ユーザの数.
	b_Direct = True;			// 直接入力.
	sc_UserAttest_At = 2;		// しない、外部、本体の設定値が変動したか？.
	b_Manage_At = True;			// 部門管理する、しないの設定値が変動したか？.
	ss_departForm_At = -1;		// 部門名+パスワード、パスワードのみの設定値が変動したか？.
	ss_UserCounter_At = 50;		// ユーザの数が変動したか？.
// comment 2017/02/27 SCD2-Sky延原 [IT5_4.0][AR1005533].
//// comment 2014/03/04 【HCL-Baskaran】[HeliosL] Klockwork Changes ID: 25933 [ UNINIT.CTOR.MUST ].
//	m_e_NFCAuthenticationSettingEnable = TYPE_NFCAuth_OFF;
// comment 2014/11/25 OPS1-Sky工藤 [ZeusS][UV-01446].
// comment 2014/09/17 HCL_Senthil [ZeusS][UV-01446]NFC authentication setting.
//	m_e_NFCAuthenticationSettingEnable = TYPE_NFCAuth_OFF;
// comment 2012/09/24 OPS1-Sky山本(勇) [ZeusBK][UV-01158]拡張サーバーダウン時の対応.
// 画面のコンストラクタは、パネル初期化allnewInstance()前に実行されるため、getInitialExtCounter()ではユーザー最大数が取得できないため使用しない.
	m_ss_ExtCounter_At = 0;	// 拡張カウンターの数が変動したか？.
	m_ss_ExtCounter = 0;	// 拡張カウンターの数.
	
	b_GotoSelectDisp = False;	// 遷移処理選択情報.
	b_Fixed = False;			// 入力確定フラグ.
	b_isError = False;			// 範囲外の入力値か True：範囲外、False：範囲内.
	ss_ConnectTime = 1;			// チケット保持時間.
	b_isEnableFieryPullScan = False;					// FieryからのPullスキャン使用可否 True:無条件許可、False:無条件破棄
	bMiddleServerWarning = False;
	bOpenAPI_NG = False;
	bBoxAdminPublicUser_NG = False;
	sc_KeySelect = -1;				// 左画面キー選択.
	b_MiddleServerSetting = True;	// 拡張サーバー認証設定をあらわす(True:使用しない False:使用する).
	b_MiddleServerSetting_At = True;	// 拡張サーバー認証設定が変動したか？.
	b_AuthorityManagement = False;	// 権限管理/カウンター管理.		//DonauLastFum【Erie拡張】【13笹岡】20090518 add.
	b_UserInfoOverwrite = False;						// ユーザー情報の上書き許可、禁止.
	sc_DefaulUserAttest = TYPE_UserAuthenticationOff;	// デフォルト認証方式.
	b_DoRequest = False;								// 制限機能使用時のログインの要求する/しない(True:要求する/False:要求しない).
	m_e_ZeroBillingJobEnable = TYPE_FreeJobSettingDisable; //【CQ:500916】【UV-01276】【管理No.267】残金ゼロ時の機能制限解除要望 【HCL_Jegan】2014/07/28 Add.
	m_e_UserAuthZeroBillingJob = TYPE_UserAuthenticationOff; //【CQ:500916】【UV-01276】【管理No.267】残金ゼロ時の機能制限解除要望 【HCL_Jegan】2014/07/28 Add.

	bOutsideServerWarning = False;
	b_NoAttestToBody = False;
	bAuthDeviceBody_NG = False;									// 生体認証装置装着フラグ.
	b_UserOn_At = False;										// 画面表示時のﾕｰｻﾞｰ名の設定値.
	sc_TotalPage = 0;											// トータルページ数.
	memset( sc_UserCommand_At, 0, sizeof( sc_UserCommand_At ) );	// 画面表示時のユーザ名とする属性.
// comment 2017/02/27 SCD2-Sky延原 [IT5_4.0][AR1005533].
	m_e_NFCUse = TYPE_NFCUse_OFF;
	m_e_NFCAuth = TYPE_NFCAuth_OFF;
	m_e_BLEUse = TYPE_BLEUse_OFF;
// comment 2014/03/04 【HCL-Baskaran】 [HeliosL] Klockwork Changes ID: 25950 [ UNINIT.CTOR.MUST ].
	m_e_BLEAuth = TYPE_BLEAuth_OFF;
	// comment 静的解析対応(22933) 110414 CEC山田 初期化対応 End.
// comment 2014/11/25 OPS1-Sky工藤 [ZeusS][UV-01446].
// comment 2014/09/12 OPS1嵯峨山 [ZeusS] RQ-223 モバイル連携 BluetoothLE対応.
//	m_e_BLEAuth = TYPE_BLEAuth_OFF;
	
// comment 2015/03/19 OPS1-MSE竹中 [MinervaSBK][RQ-278] LDAPサーバーへのカード情報登録機能対応.
// comment 2015/06/01 OPS1-MSE竹中 [MinervaSBK][CQ513861] 管理データクリア画面の遷移条件判定が不要となったため、変更前の設定値を保持する変数を削除.
//	m_bl_CardRegistration_At = False;	// カード情報登録設定On/Off(変更前).
	m_bl_CardRegistration = False;		// カード情報登録設定On/Off.
	m_e_CardAttr = TYPE_ServerGroupCardSearch_CapitalAndSmall;
	b_ExtServerDNCache  = False;		// 外部サーバーDNキャッシュ ON/OFF.
// comment 2016/07/01 [AUTHFUM2] [RQ-531]本体認証データ登録拡張 第22製品開発 森田.
	b_AuthInsideLargeScale_At  = False;	// 拡張ユーザーDB設定が変動したか？.
}
//================================================================================================
// 概要		:管理者設定・ユーザ認証/部門管理・ユーザ/部門の連動認証(2691) : ボタン（押された
// 戻り値	:なし
//================================================================================================
void PAdminUserAndDepartSynchro::procButtonOn( uchar option )
{
	// HDD接続状況を取得.
	switch( option ) {
		case  0:	//	ページ変更（前画面へ）
			if( True == PLBC_FuncVersionCheck::isOK_MiZe_Type2() ) {
				if( 2 == sc_NowPage ) {
					if( 5 == sc_UserAttest ) {
						if( 1 > ss_UserCounter || 1 > m_ss_ExtCounter ) {
							b_isError = True;			// 範囲オーバー
							setupGamen();
						}
						else {
							sc_NowPage = 1;
						}
					}
					else if( b_Manage == True && sc_UserAttest != 0 ) {
						if( 1 > ss_UserCounter ) {
//Comment 2017/12/29 [HCL-Harish] [IT5 4.2][AR1034967] not setting error flag in case of NW2 having Authentication in NW1 page Movement.
//							b_isError = True;			// 範囲オーバー
//							setupGamen();
							if( True ==  PnlComGetVlanSetting()->judgeVlanEnable() ) {
								if ( TYPE_NetworkSpecific_NW1 == PNLC_NetworkLan::getInstance() ->getAdminNWLine( ) 
									&&  TYPE_UserAuthenticationOff != PnlComGetUserAuthentication()->getUserAuthentication( TYPE_NetworkSpecific_NW2 ) 
									&&  TYPE_UserAuthenticationOff == PnlComGetUserAuthentication()->getUserAuthentication( TYPE_NetworkSpecific_NW1 ) ) {
									sc_NowPage = 1;				// Move a Page to First.
								} else {
									b_isError = True;			// 範囲オーバー
									setupGamen();
								}
							} else {
								b_isError = True;			// 範囲オーバー
								setupGamen();
							}
						}
						else {
							sc_NowPage = 1;
						}
					}
					else {
						--sc_NowPage;
						if( sc_NowPage == 0 )
						{ sc_NowPage = sc_TotalPage; }
					}
				}
				else {
					--sc_NowPage;
					if( sc_NowPage == 0 )
					{ sc_NowPage = sc_TotalPage; }
				}
			}
			else {
				if( sc_NowPage == 2 && b_Manage == True && sc_UserAttest != 0 ) {	// 2ページ目、ユーザ認証、部門管理を行うとき入力範囲を確認する.
					if( ss_UserCounter < 1 ) {
						b_isError = True;			// 範囲オーバー
						setupGamen();
					} else {
						sc_NowPage = 1;
					}
				} else {
					--sc_NowPage;
					if( sc_NowPage == 0 )	//	１ページより下になった場合、最大ページに移動する
					{ sc_NowPage = sc_TotalPage; }
				}
			}
			break;
		case  1:	//	ページ変更（次画面へ）
			if( True == PLBC_FuncVersionCheck::isOK_MiZe_Type2() ) {
				if( 2 == sc_NowPage ) {
					if( 5 == sc_UserAttest ) {
						if( 1 > ss_UserCounter || 1 > m_ss_ExtCounter ) {
							b_isError = True;			// 範囲オーバー
							setupGamen();
						}
						else {
							++sc_NowPage;
							if( sc_NowPage > sc_TotalPage )	//	最大ページより上になった場合、１ページに移動する.
							{ sc_NowPage = 1; }
						}
					}
					else if( b_Manage == True && sc_UserAttest != 0 ) {
						if( 1 > ss_UserCounter ) {
//Comment 2017/12/29 [HCL-Harish] [IT5 4.2][AR1034967] not setting error flag in case of NW2 having Authentication in NW1 page Movement.
//							b_isError = True;			// 範囲オーバー
//							setupGamen();
							if( True ==  PnlComGetVlanSetting()->judgeVlanEnable() ) {
								if ( TYPE_NetworkSpecific_NW1 == PNLC_NetworkLan::getInstance() ->getAdminNWLine( ) 
									&&  TYPE_UserAuthenticationOff != PnlComGetUserAuthentication()->getUserAuthentication( TYPE_NetworkSpecific_NW2 ) 
									&&  TYPE_UserAuthenticationOff == PnlComGetUserAuthentication()->getUserAuthentication( TYPE_NetworkSpecific_NW1 ) ) {
									++sc_NowPage;
									if( sc_NowPage > sc_TotalPage )	//	最大ページより上になった場合、１ページに移動する.
									{ sc_NowPage = 1; }
								} else {
									b_isError = True;			// 範囲オーバー
									setupGamen();
								}
							} else {
								b_isError = True;			// 範囲オーバー
								setupGamen();
							}
						}
						else {
							++sc_NowPage;
							if( sc_NowPage > sc_TotalPage )	//	最大ページより上になった場合、１ページに移動する.
							{ sc_NowPage = 1; }
						}
					}
					else {
						++sc_NowPage;
						if( sc_NowPage > sc_TotalPage )	//	最大ページより上になった場合、１ページに移動する.
						{ sc_NowPage = 1; }
					}
				}
				else {
					++sc_NowPage;
					if( sc_NowPage > sc_TotalPage )	//	最大ページより上になった場合、１ページに移動する.
					{ sc_NowPage = 1; }
				}
			}
			else {
				if( sc_NowPage == 2 && b_Manage == True && sc_UserAttest != 0 ) {	// 2ページ目、ユーザ認証、部門管理を行うとき入力範囲を確認する.
					if( ss_UserCounter < 1 ) {
						b_isError = True;			// 範囲オーバー
						setupGamen();
					} else {
						++sc_NowPage;
						if( sc_NowPage > sc_TotalPage )	//	最大ページより上になった場合、１ページに移動する.
						{ sc_NowPage = 1; }
					}
				} else {
					++sc_NowPage;
					if( sc_NowPage > sc_TotalPage )	//	最大ページより上になった場合、１ページに移動する.
					{ sc_NowPage = 1; }
				}
			}
			break;
		case 14:	// 認証しない
			if( PnlComGetSystemSetting()->getSecurityLevelUpOn() == False ) {	// セキュリティ強化モードOnの時は受け付けない.
				sc_UserAttest = 0;
				ss_PublicUser = -1;		// ユーザ認証しないときはPUBLICユーザは選択状態ではなくす.
				if( b_Manage == True ) {	// 部門がOnの場合.
					if( b_Synchro == False ) {	// さらに、連動がOnの時
						b_Synchro = True;			// ユーザ認証がOffの時、連動できない
						pdpc_UserAndTrackMode.setTrackModeLink( PNLE_TrackModeLink_UnLink );
						ss_departForm = 0;			// 部門管理がOff→Onの時、管理方式が選択されていない状態になり、そのままOKすると困る為、0を入れる
					}
					ss_UserCounter = 0;
					if( True == PLBC_FuncVersionCheck::isOK_MiZe_Type2() ) {
						m_ss_ExtCounter = 0;
					}
				} else {	// 部門もOFFの場合(両方OFF).
					ss_UserCounter = 0;
					if( True == PLBC_FuncVersionCheck::isOK_MiZe_Type2() ) {
						m_ss_ExtCounter = 0;
					}
				}
				// 認証カード検索機能をOFFにする.
				b_AuthCardServerSearching = False;
			}
			b_isEnableFieryPullScan = False;	// 認証がOffの時、初期値(Prohibit/許可しない)に設定する
			b_NoAttestToBody = False;
			break;
		case 15:	// 外部サーバー認証.
			if( PnlComGetMachineSetting()->getKeyMode() != TYPE_KeyModeAdminMachine ) {	// キーモードが「管理装置」の場合、本体装置認証を受け付けない(管理装置とユーザ認証は併用できない為).
				if( sc_UserAttest != 1 ) {	// ユーザ認証が既に「する」の時は受け付けない(ユーザ数が変化されては困るため).
					if( sc_UserAttest == 0 ) {	// 認証しない → 本体認証 の場合
						ss_PublicUser = 0;	// パブリックユーザ不許可(デフォルト).
						if( b_Manage == True ) {	// 部門認証がするの場合.
//Comment 2017/09/29 [HCL-Harish] [IT5 4.2][RQ-686] [WP533/545] Multiple Network support - User/Track.
//							ss_UserCounter = PnlComGetTrackManager()->getMaxTrack() / 2;	// 仮の値 HDDあり時500、なし時 50を代入.
							ss_UserCounter = GetMaxTrackCounter() / 2;	// 仮の値 HDDあり時500、なし時 50を代入.
							ss_departForm = 0;			// 「部門名+パスワード」に設定(ユーザ認証、部門管理ともにOn時、認証方式は「部門名+パスワード」).
							b_Synchro = False;			// 連動するがデフォルト.
							pdpc_UserAndTrackMode.setTrackModeLink( PNLE_TrackModeLink_UnLink );
						}
					}
					sc_UserAttest = 1;	// 外部サーバ認証.
				}
			}
			b_NoAttestToBody = False;
			break;
		case 16:	// 中間サーバー認証.
			if( PnlComGetMachineSetting()->getKeyMode() != TYPE_KeyModeAdminMachine ) {	// キーモードが「管理装置」の場合、認証を受け付けない(管理装置とユーザ認証は併用できない為).
				if( sc_UserAttest != 3 ) {	// ユーザ認証が既に「中間サーバー認証」の時は受け付けない(ユーザ数が変化されては困るため).
					if( sc_UserAttest == 0 ) {	// 認証しない → 中間サーバー認証 の場合
						ss_PublicUser = 0;	// パブリックユーザ不許可(デフォルト).
						if( b_Manage == True ) {	// 部門認証がするの場合.
//Comment 2017/09/29 [HCL-Harish] [IT5 4.2][RQ-686] [WP533/545] Multiple Network support - User/Track.
//							ss_UserCounter = PnlComGetTrackManager()->getMaxTrack() / 2;	// 仮の値 HDDあり時500、なし時 50を代入.
							ss_UserCounter = GetMaxTrackCounter() / 2;	// 仮の値 HDDあり時500、なし時 50を代入.
							ss_departForm = 0;			// 「部門名+パスワード」に設定(ユーザ認証、部門管理ともにOn時、認証方式は「部門名+パスワード」).
							b_Synchro = False;				// 連動するがデフォルト.
							pdpc_UserAndTrackMode.setTrackModeLink( PNLE_TrackModeLink_Link );
						}
					}
					sc_UserAttest = 3;	// 中間サーバー認証.
					// 認証カード検索機能をOFFにする.
					b_AuthCardServerSearching = False;
				}
			}
			b_NoAttestToBody = False;
			break;
		case 2:	// 本体装置認証
			if( PnlComGetMachineSetting()->getKeyMode() == TYPE_KeyModeAdminMachineKM ) {
				b_NoAttestToBody = False;
				if( sc_UserAttest != 2 ) {	// ユーザ認証が既に「する」の時は受け付けない(ユーザ数が変化されては困るため).
					if( sc_UserAttest == 0 ) {	// 認証しない → 本体認証 の場合
						ss_PublicUser = 1;	// パブリックユーザ許可.
						b_NoAttestToBody = True;
					}
					sc_UserAttest = 2;	// 本体装置認証にする.
					// 認証カード検索機能をOFFにする.
					b_AuthCardServerSearching = False;
				}
				break;
			}
			if( PnlComGetMachineSetting()->getKeyMode() != TYPE_KeyModeAdminMachine ) {	// キーモードが「管理装置」の場合、本体装置認証を受け付けない(管理装置とユーザ認証は併用できない為).
				if( sc_UserAttest != 2 ) {	// ユーザ認証が既に「する」の時は受け付けない(ユーザ数が変化されては困るため).
					if( sc_UserAttest == 0 ) {	// 認証しない → 本体認証 の場合
						ss_PublicUser = 0;	// パブリックユーザ不許可(デフォルト).
						if( b_Manage == True ) {	// 部門認証がするの場合.
//Comment 2017/09/29 [HCL-Harish] [IT5 4.2][RQ-686] [WP533/545] Multiple Network support - User/Track.
//							ss_UserCounter = PnlComGetTrackManager()->getMaxTrack() / 2;	// 仮の値 HDDあり時500、なし時 50を代入.
							ss_UserCounter = GetMaxTrackCounter() / 2;	// 仮の値 HDDあり時500、なし時 50を代入.
							ss_departForm = 0;			// 「部門名+パスワード」に設定(ユーザ認証、部門管理ともにOn時、認証方式は「部門名+パスワード」).
							b_Synchro = False;				// 連動するがデフォルト.
							pdpc_UserAndTrackMode.setTrackModeLink( PNLE_TrackModeLink_Link );
						}
					}
					sc_UserAttest = 2;	// 本体装置認証にする.
					// 認証カード検索機能をOFFにする.
					b_AuthCardServerSearching = False;
				}
			}
			break;
		case 3:	// パブリックユーザ許可しない
			if( PnlComGetMachineSetting()->getKeyMode() == TYPE_KeyModeAdminMachineKM ) {
				//統合管理装置では、受付け禁止.
				break;
			}
			if( sc_UserAttest != 0 )			// ユーザ認証がOnの時のみ設定可能(キーを効かす)
			{ ss_PublicUser = 0; }
			break;
		case 4:	// パブリックユーザ許可する
			if( sc_UserAttest != 0 ) {			// ユーザ認証がOnの時のみ設定可能(キーを効かす)
				if( True == PLBC_FuncVersionCheck::isOK_Type1() ) {
					if( PnlComGetMachineSetting()->getKeyMode() != TYPE_KeyModeAdminMachineKM ) {	// 統合管理装置非接続時のみ設定可能(キーを効かす)[1.8対応 柳川].
						ss_PublicUser = 1;
					}
				} else {
					if( b_Manage == False || b_Synchro == False )	// 部門管理しないか連動するの時のみ設定可能(キーを効かす) .
						if( PnlComGetMachineSetting()->getKeyMode() != TYPE_KeyModeAdminMachineKM )	// 統合管理装置非接続時のみ設定可能(キーを効かす)[1.8対応 柳川].
						{ ss_PublicUser = 1; }
				}
			}
			break;
		// 制限機能使用時のログイン設定.
		case 17: b_DoRequest = True;					break;	// 要求する.
		case 18: b_DoRequest = False; 					break;	// 要求しない.
		case 5:	// 部門管理しない
			b_Manage = False;
			b_Synchro = True;					// 部門管理がOffの時、連動できない
			pdpc_UserAndTrackMode.setTrackModeLink( PNLE_TrackModeLink_UnLink );
			ss_departForm = -1;				// 部門管理がOffの時、管理方式は選択されない

			if( True == PLBC_FuncVersionCheck::isOK_MiZe_Type2() ) {
				if( 5 == sc_UserAttest ) {
					// HDD時有りなしで値が変わるためメソッドから取得する.
					// 本体+拡張サーバー認証の場合.
//Comment 2017/09/29 [HCL-Harish] [IT5 4.2][RQ-686] [WP533/545] Multiple Network support - User/Track.
//					ss_UserCounter = PnlComGetUserAuthentication()->getMaxUser() / 2;
					ss_UserCounter = GetMaxUserCounter() / 2;
					m_ss_ExtCounter = getInitialExtCounter();
				}
				else if( 0 != sc_UserAttest ) {
					// 本体+拡張サーバー認証以外の場合.
//Comment 2017/09/29 [HCL-Harish] [IT5 4.2][RQ-686] [WP533/545] Multiple Network support - User/Track.					
//					ss_UserCounter = PnlComGetTrackManager()->getMaxTrack();
					ss_UserCounter = GetMaxTrackCounter();
				}
				else {
					// ﾕｰｻﾞ認証 しないの場合(両方しない).
					ss_UserCounter = 0;
				}
			}
			else {
				if( sc_UserAttest != 0 ) {	// ﾕｰｻﾞ認証 するの場合.
//Comment 2017/09/29 [HCL-Harish] [IT5 4.2][RQ-686] [WP533/545] Multiple Network support - User/Track.					
//					ss_UserCounter = PnlComGetTrackManager()->getMaxTrack();
					ss_UserCounter = GetMaxTrackCounter();
				} else {	// ﾕｰｻﾞ認証 しないの場合(両方しない).
					ss_UserCounter = 0;
				}
			}
			b_isEnableFieryPullScan = False;	// 部門管理がOffの時、初期値(Prohibit/許可しない)に設定する
			break;
		case 6:	// 部門管理する
			if( PnlComGetMachineSetting()->getKeyMode() == TYPE_KeyModeAdminMachineKM ) {
				//統合管理装置(ユーザ認証する)では、部門管理はできない.
				break;
			}
			if( b_Manage != True ) {	// 部門管理が既に「する」の時ユーザ数が変化されては困るため.
				b_Manage = True;
				ss_departForm = 0;				// 部門管理がOff→Onの時、管理方式が選択されていない状態になり、そのままOKすると困る為、0を入れる

				if( True == PLBC_FuncVersionCheck::isOK_MiZe_Type2() ) {
					if( 5 == sc_UserAttest ) {
						// HDD時有りなしで値が変わるためメソッドから取得する.
//Comment 2017/09/29 [HCL-Harish] [IT5 4.2][RQ-686] [WP533/545] Multiple Network support - User/Track.						
//						ss_UserCounter = PnlComGetUserAuthentication()->getMaxUser() / 2;
						ss_UserCounter = GetMaxUserCounter() / 2;
						m_ss_ExtCounter = getInitialExtCounter();
						b_Synchro = False;				// 連動するがデフォルト.
						pdpc_UserAndTrackMode.setTrackModeLink( PNLE_TrackModeLink_Link );
					}
					else if( 0 != sc_UserAttest ) {
//Comment 2017/09/29 [HCL-Harish] [IT5 4.2][RQ-686] [WP533/545] Multiple Network support - User/Track.					
//						ss_UserCounter = PnlComGetTrackManager()->getMaxTrack() / 2;	// 仮の値 HDDあり時500、なし時 50を代入.
						ss_UserCounter = GetMaxTrackCounter() / 2;	// 仮の値 HDDあり時500、なし時 50を代入.
						b_Synchro = False;				// 連動するがデフォルト.
						pdpc_UserAndTrackMode.setTrackModeLink( PNLE_TrackModeLink_Link );
					}
					else {
						// 処理なし.
					}
				}
				else {
					if( sc_UserAttest != 0 ) {	// ユーザ認証が既にするの場合.
//Comment 2017/09/29 [HCL-Harish] [IT5 4.2][RQ-686] [WP533/545] Multiple Network support - User/Track.						
//						ss_UserCounter = PnlComGetTrackManager()->getMaxTrack() / 2;	// 仮の値 HDDあり時500、なし時 50を代入.
						ss_UserCounter = GetMaxTrackCounter() / 2;	// 仮の値 HDDあり時500、なし時 50を代入.
						b_Synchro = False;				// 連動するがデフォルト.
						pdpc_UserAndTrackMode.setTrackModeLink( PNLE_TrackModeLink_Link );
					}
				}
			}
			break;
		case 7:	// 部門名+パスワード
			if( b_Synchro != False && b_Manage == True )	// 部門管理する＋連動しないの場合.
			{ ss_departForm = 0; }
			break;
		case 8:	// パスワードのみ
			if( PnlComGetMachineSetting()->getKeyMode() != TYPE_KeyModeAdminMachine
			    &&PnlComGetMachineSetting()->getKeyMode() != TYPE_KeyModeAdminMachineKM ) {	// [1.8対応 柳川].
				// キーモードが「管理装置(従来、統合共に)」の場合、パスワードのみ設定を受け付けない(管理装置とパスワードのみ設定は併用できない為).
				if( b_Synchro != False && b_Manage == True )	// 部門管理する＋連動しないの場合.
					if( sc_UserAttest == 0 ) // ユーザ認証 OFFの場合(ユーザ認証、部門管理共にするのときは部門名+パスワードだから).
					{ ss_departForm = 1; }
			}
			break;
		case 9:	// 連動する
			if( sc_UserAttest != 0 && b_Manage == True ) {	// ユーザ認証、部門管理共にOnの時のみ設定できる
				b_Synchro = False;
				pdpc_UserAndTrackMode.setTrackModeLink( PNLE_TrackModeLink_Link );
			}
			break;
		case 10:	{	// 連動しない
			if( PnlComGetMachineSetting()->getKeyMode() != TYPE_KeyModeAdminMachineKM ) {	// 統合管理装置非接続時のみ設定可能(キーを効かす)[1.8対応 柳川].
				b_Synchro = True;
				pdpc_UserAndTrackMode.setTrackModeLink( PNLE_TrackModeLink_UnLink );
				if( False == PLBC_FuncVersionCheck::isOK_Type1() ) {
					ss_PublicUser = 0;	// 連動しないとき、publicユーザは許可しないに変更する(禁止組み合わせ).
				}
			}
		}
		break;
		case BTNID_AdminUserAndDepartSynchro_UserSelect:		// ユーザーが設定するキー.
			b_Synchro = False;
			pdpc_UserAndTrackMode.setTrackModeLink( PNLE_TrackModeLink_UserSelect );
			break;
		case 11: sc_LimitArrive = 0;					break;	// ジョブ飛び越し.
		case 12: sc_LimitArrive = 1;					break;	// ジョブ停止.
		case BTNID_AdminUserAndDepartSynchro_LimitArriveDelete:	sc_LimitArrive = 2;	break;	// ジョブ削除.

		case 20: b_isEnableFieryPullScan = True;		break;	// 許可する
		case 21: b_isEnableFieryPullScan = False; 		break;	// 許可しない
		case 22: b_AuthorityManagement = True;			break;	// 管理する.
		case 23: b_AuthorityManagement = False; 		break;	// 管理しない.
		case 50:	// パブリックユーザ許可後、ログインする
			if( sc_UserAttest != 0 ) {			// ユーザ認証がOnの時のみ設定可能(キーを効かす)
				if( True == PLBC_FuncVersionCheck::isOK_Type1() ) {
					if( PnlComGetMachineSetting()->getKeyMode() != TYPE_KeyModeAdminMachineKM ) {	// 統合管理装置非接続時のみ設定可能(キーを効かす).
						ss_PublicUser = 2;
					}
				} else {
					if( b_Manage == False || b_Synchro == False )	// 部門管理しないか連動するの時のみ設定可能(キーを効かす) .
						if( PnlComGetMachineSetting()->getKeyMode() != TYPE_KeyModeAdminMachineKM )	// 統合管理装置非接続時のみ設定可能(キーを効かす).
						{ ss_PublicUser = 2; }
				}
			}
			break;
		case 25:	sc_KeySelect = 0;				break;	// ユーザー認証キー.
		case 26:	sc_KeySelect = 1;				break;	// パブリックユーザーキー.
		case 36:	sc_KeySelect = 2;				break;	// 制限機能使用時のログインキー.
		case 27:	sc_KeySelect = 3;				break;	// 部門管理キー.
		case 28:	sc_KeySelect = 4;				break;	// 部門管理認証方式キー.
		case 29:	sc_KeySelect = 6;				break;	// ユーザ認証/部門認証の連動キー.
		case 30:	sc_KeySelect = 7;				break;	// 上限値到達時の動作キー.
		case 31:	sc_KeySelect = 8;				break;	// スキャン設定.
		case 32:	sc_KeySelect = 9;				break;	// 権限管理/カウンター管理キー.	//DonauLastFum【Erie拡張】【13笹岡】20090518 add.
		case BTNID_PKI_Key_MiddleServerSetting:		// 拡張サーバー認証キー.
			sc_KeySelect = 5;
			break;
		case BTNID_PKI_EnableMiddleServerSetting:	// 拡張サーバー認証を使用する.
			b_MiddleServerSetting = False;
			sc_UserAttest = 3;		// 拡張サーバー認証.
			// 認証カード検索機能をOFFにする.
			b_AuthCardServerSearching = False;
			break;
		case BTNID_PKI_DisableMiddleServerSetting:	// 拡張サーバー認証を使用しない.
			b_MiddleServerSetting = True;
			sc_UserAttest = 1;		// 外部サーバー認証.
			break;
		case BTNID_ZeroBillingCharge:
			sc_KeySelect = DEF_ZeroBillingChargeDisp;				break;
		case BTNID_ZeroBillingChargePermitOn:
			m_e_ZeroBillingJobEnable = TYPE_FreeJobSettingEnable;	break;
		case BTNID_ZeroBillingChargePermitOff:
			m_e_ZeroBillingJobEnable = TYPE_FreeJobSettingDisable;	break;
// comment 2017/02/27 SCD2-Sky延原 [IT5_4.0][AR1005533] NFC使用設定/認証設定.
		case BTNID_AdminNFCUseSetting:			// NFC使用設定.
			sc_KeySelect = DEF_AdminNFCUseSetting;
			break;
		case BTNID_AdminNFCUseSettingON:		// NFC使用設定：使用する.
			m_e_NFCUse = TYPE_NFCUse_ON;
			break;
		case BTNID_AdminNFCUseSettingOFF:		// NFC使用設定：使用しない.
			m_e_NFCUse = TYPE_NFCUse_OFF;
			m_e_NFCAuth = TYPE_NFCAuth_OFF; // NFC認証設定も「使用しない」に設定.
			break;
		case BTNID_AdminNFCAuthSetting:			// NFC認証設定.
			sc_KeySelect = DEF_AdminNFCAuthSetting;
			break;
		case BTNID_AdminNFCAuthSettingON:		// NFC認証設定：使用する.
			m_e_NFCAuth = TYPE_NFCAuth_ON;
			break;
		case BTNID_AdminNFCAuthSettingOFF:		// NFC認証設定：使用しない.
			m_e_NFCAuth = TYPE_NFCAuth_OFF;
			break;
// comment 2017/02/27 SCD2-Sky延原 [IT5_4.0][AR1005533] Bluetooth LE使用設定/認証設定.
//		case BTNID_AdminUserAndDepartSynchroBluetoothLEAuthSetting: 	// BluetoothLE認証設定.
//			// Rtb_AdminUserAndDepartSynchro3pの3番目のキーの配列情報. ( 11 - 9 )
////				sc_KeySelect = 11;
//			sc_KeySelect = 12;
//			break;
		case BTNID_AdminBLEUseSetting:			// Bluetooth LE使用設定.
			sc_KeySelect = DEF_AdminBLEUseSetting;
			break;
		case BTNID_AdminBLEUseSettingON:		// Bluetooth LE使用設定：使用する.
			m_e_BLEUse = TYPE_BLEUse_ON;
			break;
		case BTNID_AdminBLEUseSettingOFF:		// Bluetooth LE使用設定：使用しない.
			m_e_BLEUse = TYPE_BLEUse_OFF;
			m_e_BLEAuth = TYPE_BLEAuth_OFF;	// Bluetooth LE認証設定も「使用しない」に設定.
			break;
		case BTNID_AdminBLEAuthSetting:			// Bluetooth LE認証設定.
			sc_KeySelect = DEF_AdminBLEAuthSetting;
			break;
		case BTNID_AdminBLEAuthSettingON:		// Bluetooth LE認証設定：使用する.
			m_e_BLEAuth = TYPE_BLEAuth_ON;
			break;
		case BTNID_AdminBLEAuthSettingOFF:		// Bluetooth LE認証設定：使用しない.
			m_e_BLEAuth = TYPE_BLEAuth_OFF;
			break;
// comment 2014/11/25 OPS1-Sky工藤 [ZeusS][UV-01446].
//			case BTNID_AdminUserAndDepartSynchroBluetoothLEAuthSetON:
//				m_e_BLEAuth = TYPE_BLEAuth_ON;
//				break;
//			case BTNID_AdminUserAndDepartSynchroBluetoothLEAuthSetOFF:
//				m_e_BLEAuth = TYPE_BLEAuth_OFF;
//				break;
// comment 2017/02/27 SCD2-Sky延原 [IT5_4.0][AR1005533].
////comment 2014/09/15 HCLT_Utsav [ZeusS][UV-01446] Adding NFC option in Authentication Screen(F097700000) for IT5.
//		case BTNID_AdminNFCAuthenticationSetting:		// select NFC認証設定 key.
//			sc_KeySelect = DEF_AdminNFCAuthenticationSetting;
//			break;
// comment 2014/11/25 OPS1-Sky工藤 [ZeusS][UV-01446].
//			case BTNID_AdminNFCAuthenticationSettingOn:		// It uses it. 
//				m_e_NFCAuthenticationSettingEnable = TYPE_NFCAuth_ON;
//				break;
//			case BTNID_AdminNFCAuthenticationSettingOff:
//				m_e_NFCAuthenticationSettingEnable = TYPE_NFCAuth_OFF;
//				break;
// comment 2016/5/25 [AUTHFUM2] [RQ-526]外部サーバーLDAP検索高速化 第22製品開発 山本.
		case BTNID_AdminUserAndDepartSynchroExtServerDNCacheON: 	// 外部サーバーDNキャッシュ 使用するキー.
			b_ExtServerDNCache = True;
			break;
		case BTNID_AdminUserAndDepartSynchroExtServerDNCacheOFF:	// 外部サーバーDNキャッシュ 使用しないキー.
			b_ExtServerDNCache = False;
			break;
		case BTNID_AdminUserAndDepartSynchroExtServerDNCache: 		// 外部サーバーDNキャッシュキー.
			sc_KeySelect = 13;
			break;
// comment 2016/5/25 [AUTHFUM2] [RQ-531]本体認証データ登録拡張 第22製品開発 山本.
		case BTNID_AdminUserAndDepartSynchroAuthInsideLargeScaleON:	// 拡張ユーザーDB 使用するキー.
			b_AuthInsideLargeScale = True;
			break;
		case BTNID_AdminUserAndDepartSynchroAuthInsideLargeScaleOFF:// 拡張ユーザーDB 使用しないキー.
			b_AuthInsideLargeScale = False;
			break;
		case BTNID_AdminUserAndDepartSynchroAuthInsideLargeScale:	// 拡張ユーザーDBキー.
			sc_KeySelect = 14;
			break;
		default:	break;	// for warning!
	}
	setupGamen();
}
//================================================================================================
// 概要		:管理者設定・ユーザ認証/部門管理・ユーザ/部門の連動認証(2691) : 画面移動.
// 戻り値	:なし.
//================================================================================================
void PAdminUserAndDepartSynchro::procGoToPage( uchar option )
{
	if( BTNID_GOTO_AdminAuthCardSearchSetting == option ) {
		APIC_AuthenticationSeverSetting* apicAuthSrv = APIC_AuthenticationSeverSetting::newInstance();
		if( NULL == apicAuthSrv ) {
// comment [2016/12/22] [PDD22山下(日立)] [IT5_4.0][AccuRev.1009508]認証拡張FUM追加コードに対して新詳細コードを追加.
//Comment 2019/06/14 [HCLT_Monisha][EagleH][N Rank] Removed unwanted Detailed code.
//			LAPC_BFW_System *pc_BFW_System = LAPC_BFW_System::newInstance();
//			if (NULL != pc_BFW_System){    
//			    pc_BFW_System ->registDetailCode( PNLD_DetailCode_Admin_0x19000200104E0601 );
//			}
			LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" ); 
			return;
		}
		if( ( True == apicAuthSrv->checkServerGroup() )
			&& ( True == PnlComGetAuthDeviceCommon()->checkIccChangeAuthServer() )
// comment [2017/01/17] [PDD22山下(日立)] [IT5_4.0][AccuRev.1009975]権限管理/カウンタ管理の値を不揮発値ではなく、RAM上の変数値を参照するように変更.
			&& ( False == b_AuthorityManagement ) ) {
			// サーバーグループ設定が有効な場合.
			PLBC_CheckFromMenu::getInstance()->setFromMenu( True );
// comment 2022/05/20 Sky西場 [IT6_2.3][OP_BTS-37620][スリム化].
			ExpPanelLog( PNLD_ExpLogKind_AlwaysLogOut, "[DeleteUnnecessaryProcessing] [%s]", __PRETTY_FUNCTION__, 0, 0, 0, 0, 0 );
//			nextGamen = &gp_AdminAuthServerGroup_LDAPICAuth;
		} else {
// comment 2022/05/20 Sky西場 [IT6_2.3][OP_BTS-37620][スリム化].
			ExpPanelLog( PNLD_ExpLogKind_AlwaysLogOut, "[DeleteUnnecessaryProcessing] [%s]", __PRETTY_FUNCTION__, 0, 0, 0, 0, 0 );
//			gp_AdminAuthCardSearchSetting.b_FromMenu=True;
//			nextGamen = &gp_AdminAuthCardSearchSetting;							// 80926000_認証カード検索設定画面へ遷移.
		}
	}
	if( option == 4 ) {
		b_GotoSelectDisp = True;
		nextGamen = &gp_AdminUserAndDepartSynchroKeepConnectionLimitInput;	// 10990130	チケット保持時間.
	}
	if( option == 25 ) {
		if( True == PLBC_FuncVersionCheck::isOK_MiZe_Type2() ) {
			// 複数ページ構成のユーザー認証方式画面に遷移する.
			if( True == onlyPanel.isWarningOn( comPanelJob, PNL_ERROR_BrowserAuthUnable ) || True == onlyPanel.isWarningOn( comPanelJob, PNL_ERROR_AuthExpireDate ) ) {
				gp_AdminUserAndDepartSynchroDirectUserAuthMultiComposition.setSrcAdminUserAndDepartSynchro( this );	//	呼び出し元画面インスタンスを設定.
				gp_AdminUserAndDepartSynchroDirectUserAuthMultiComposition.initDisp();		// 画面の初期化.
				nextGamen = &gp_AdminUserAndDepartSynchroDirectUserAuthMultiComposition;	// 80920000 ユーザー認証方式.
			} else {
				gp_AdminUserAndDepartSynchroUserAuthMultiComposition.initDisp();			// 画面の初期化.
				nextGamen = &gp_AdminUserAndDepartSynchroUserAuthMultiComposition;			// 80920000 ユーザー認証方式.
			}
		} else {
			if( True == onlyPanel.isWarningOn( comPanelJob, PNL_ERROR_BrowserAuthUnable ) || True == onlyPanel.isWarningOn( comPanelJob, PNL_ERROR_AuthExpireDate ) ) {
				gp_AdminUserAndDepartSynchroDirectUserAuth.setSrcAdminUserAndDepartSynchro( this );	//	呼び出し元画面インスタンスを設定.
				nextGamen = &gp_AdminUserAndDepartSynchroDirectUserAuth;	// 80920000 ユーザー認証方式.
			} else {
				nextGamen = &gp_AdminUserAndDepartSynchroUserAuth;			// 80920000 ユーザー認証方式.
			}
		}
	}

	if( 1 == option ) {
		if( True == PLBC_FuncVersionCheck::isOK_MiZe_Type2() ) {
			if( ( 0 != sc_UserAttest && 5 == sc_UserAttest ) || ( 0 != sc_UserAttest && True == b_Manage && 3 != sc_UserAttest ) ) {
				// 認証方式が「本体+拡張サーバー」の場合、もしくは、認証「する」部門管理設定「する」の場合.
				b_GotoSelectDisp = True;
				nextGamen = &gp_AdminUserAndDepartSynchroCounter;			// C092012000 カウンター割当て数.
			}
		}
		else {
			if( b_Manage == True && sc_UserAttest != 0 ) {
				b_GotoSelectDisp = True;
				nextGamen = &gp_AdminUserAndDepartSynchroUserCounter;		// 10920120   ユーザーカウンター割当て数.
			}
		}
	}
// comment 2017/02/27 SCD2-Sky延原 [IT5_4.0][AR1005533].
//// comment 2014/11/25 OPS1-Sky工藤 [ZeusS][UV-01446][RQ-223].
//	if( BTNID_GOTO_AdminNFCAuthSetting == option ) {
//		nextGamen = &gp_AdminMobilePairingPSMobilePairingNFCSetting;		// H0920120_NFC使用設定画面.
//	} else {
//		// 処理なし.
//	}
//	if( BTNID_GOTO_AdminBLEAuthSetting == option ) {
//		nextGamen = &gp_AdminMobilePairingPSMobilePairingBLESetting;		// H0920130_Bluetooth LE使用設定画面.
//	} else {
//		// 処理なし.
//	}

	if( NULL != nextGamen ) {
		sc_KeySelect = -1;
	}
}
//================================================================================================
// 概要		:管理者設定・ユーザ認証/部門管理・ユーザ/部門の連動認証(2691) : 設定値の保存処理.
// 戻り値	:なし
//================================================================================================
void PAdminUserAndDepartSynchro::setAuthenticationData()
{
// ユーザ認証設定.
//Comment 2017/09/04 [HCL-Harish] [IT5 4.2][RQ-686] [WP509] Multiple Network support.
	TYPE_NetworkSpecific e_NetworkSpecific  = PNLC_NetworkLan::getInstance()->getAdminNWLine( ) ;
//Comment 2017/10/29 [HCL-Harish] [IT5 4.2][WP1146] User Registeration and track - Counter.
	bool bl_JudgeAuthMethodNW2 = False;
	if( PnlComGetUserAuthentication()->getUserAuthentication( TYPE_NetworkSpecific_NW2 ) == TYPE_UserAuthenticationOff ) {
		bl_JudgeAuthMethodNW2 = True;
	}
	if( gp_AdminUserAndDepartSynchro.sc_UserAttest == 0 ) {
//		PnlComGetUserAuthentication()->setUserAuthentication( TYPE_UserAuthenticationOff, JLGE_InspectLogIfOperationPanel, False, PnlComGetUser(), PnlComGetAuthorityID() );	// ＯＦＦ.
		PnlComGetUserAuthentication()->setUserAuthentication( TYPE_UserAuthenticationOff, JLGE_InspectLogIfOperationPanel, False, PnlComGetUser(), PnlComGetAuthorityID(), e_NetworkSpecific );	// ＯＦＦ.
		// ユーザー認証しないの場合のみ、SMTP ユーザー認証使用するしないの設定を変更する.
// comment 2017/11/15 【HCL-Sindhu】【IT5_4.2】【RQ686】【WP1360】 Multiple Network Support Additional WP.
//// 2016/02/01 CQ521285 監査ログ HCD-PP対応 Sky大嶋 MOD Start.
//		PnlComGetMailSendSetting()->setSmtpCertificationType( TYPE_SmtpCertificationTypeDefault, JLGE_InspectLogOperateIDL_Admin, JLGE_InspectLogIfOperationPanel, False );
		PnlComGetMailSendSetting()->setSmtpCertificationType( TYPE_SmtpCertificationTypeDefault, JLGE_InspectLogOperateIDL_Admin, JLGE_InspectLogIfOperationPanel, False, PNLC_NetworkLan::getInstance()->getAdminNWLine());
// 2016/02/01 CQ521285 監査ログ HCD-PP対応 Sky大嶋 MOD End.
	} else if( gp_AdminUserAndDepartSynchro.sc_UserAttest == 1 )
//	{ PnlComGetUserAuthentication()->setUserAuthentication( TYPE_UserAuthenticationServer, JLGE_InspectLogIfOperationPanel, False, PnlComGetUser(), PnlComGetAuthorityID() ); }	// 外部サーバーでの認証.
	{ PnlComGetUserAuthentication()->setUserAuthentication( TYPE_UserAuthenticationServer, JLGE_InspectLogIfOperationPanel, False, PnlComGetUser(), PnlComGetAuthorityID(), e_NetworkSpecific ); }	// 外部サーバーでの認証.
	else if( gp_AdminUserAndDepartSynchro.sc_UserAttest == 3 ) {
		// 中間サーバーでの認証.
//		PnlComGetUserAuthentication()->setUserAuthentication( TYPE_UserAuthenticationIntermeddiateServer, JLGE_InspectLogIfOperationPanel, False, PnlComGetUser(), PnlComGetAuthorityID() );
		PnlComGetUserAuthentication()->setUserAuthentication( TYPE_UserAuthenticationIntermeddiateServer, JLGE_InspectLogIfOperationPanel, False, PnlComGetUser(), PnlComGetAuthorityID(), e_NetworkSpecific );
	} else if( gp_AdminUserAndDepartSynchro.sc_UserAttest == 4 ) {
		// 本体+外部サーバーでの認証.
//		PnlComGetUserAuthentication()->setUserAuthentication( TYPE_UserAuthenticationDeviceAndServer, JLGE_InspectLogIfOperationPanel, False, PnlComGetUser(), PnlComGetAuthorityID() );
		PnlComGetUserAuthentication()->setUserAuthentication( TYPE_UserAuthenticationDeviceAndServer, JLGE_InspectLogIfOperationPanel, False, PnlComGetUser(), PnlComGetAuthorityID(), e_NetworkSpecific );
	} else if( gp_AdminUserAndDepartSynchro.sc_UserAttest == 5 ) {
//		PnlComGetUserAuthentication()->setUserAuthentication( TYPE_UserAuthenticationDeviceAndMidAuth, JLGE_InspectLogIfOperationPanel, False, PnlComGetUser(), PnlComGetAuthorityID() );
		PnlComGetUserAuthentication()->setUserAuthentication( TYPE_UserAuthenticationDeviceAndMidAuth, JLGE_InspectLogIfOperationPanel, False, PnlComGetUser(), PnlComGetAuthorityID(), e_NetworkSpecific );
	} else
//	{ PnlComGetUserAuthentication()->setUserAuthentication( TYPE_UserAuthenticationDevice, JLGE_InspectLogIfOperationPanel, False, PnlComGetUser(), PnlComGetAuthorityID() ); }	// 本体装置での認証.
	{ PnlComGetUserAuthentication()->setUserAuthentication( TYPE_UserAuthenticationDevice, JLGE_InspectLogIfOperationPanel, False, PnlComGetUser(), PnlComGetAuthorityID(), e_NetworkSpecific ); }	// 本体装置での認証.
// ユーザー認証許可.
	if( ( gp_AdminUserAndDepartSynchro.sc_UserAttest_At != 2 && gp_AdminUserAndDepartSynchro.sc_UserAttest == 2 )
	    || ( gp_AdminUserAndDepartSynchro.sc_UserAttest_At == 2 && gp_AdminUserAndDepartSynchro.sc_UserAttest != 2 ) ) {
		gp_IwsAuthBrowserLogin.notifyIwsReset( False );
	}
	switch( gp_AdminUserAndDepartSynchro.sc_UserAttest ) {
		case 1:
		case 4:
		case 5:
// Comment 2017/11/03 [HCL-Narsimhmaswamy] [IT5 4.2][RQ-686] [AR1030809].
//	// 2016/02/01 CQ521285 監査ログ HCD-PP対応 Sky大嶋 MOD Start.
//		case 3:	PnlComGetUserAuthenticationSetting()->setEnableUserAuthentication( True, JLGE_InspectLogOperateIDL_Admin, JLGE_InspectLogIfOperationPanel, False );		break;	// ユーザー認証許可.
		case 3:	PnlComGetUserAuthenticationSetting()->setEnableUserAuthentication( True, JLGE_InspectLogOperateIDL_Admin, JLGE_InspectLogIfOperationPanel, False, e_NetworkSpecific );		break;	// ユーザー認証許可.
// 2016/02/01 CQ521285 監査ログ HCD-PP対応 Sky大嶋 MOD End.
		default:

// Comment 2017/11/03 [HCL-Narsimhmaswamy] [IT5 4.2][RQ-686] [AR1030809].
//	// 2016/02/01 CQ521285 監査ログ HCD-PP対応 Sky大嶋 MOD Start.
//			PnlComGetUserAuthenticationSetting()->setEnableUserAuthentication( False, JLGE_InspectLogOperateIDL_Admin, JLGE_InspectLogIfOperationPanel, False );			break;	// ユーザー認証不許可.
			PnlComGetUserAuthenticationSetting()->setEnableUserAuthentication( False, JLGE_InspectLogOperateIDL_Admin, JLGE_InspectLogIfOperationPanel, False, e_NetworkSpecific );			break;	// ユーザー認証不許可.
// 2016/02/01 CQ521285 監査ログ HCD-PP対応 Sky大嶋 MOD End.
	}
//	comment 2015/11/02 HCL_Ezhil [ZeusS_ZX0][制御構成変更_No.76-1] Adding error code.
//Comment 2019/06/14 [HCLT_Monisha][EagleH][N Rank] Removed unwanted Detailed code.
//	LAPC_BFW_System *pc_BFW_System=LAPC_BFW_System::newInstance();
// パブリックユーザ設定.
//	PnlComGetUserAuthentication()->setEnablePublicUserNoAuthentication( False );
	PnlComGetUserAuthentication()->setEnablePublicUserNoAuthentication( False, e_NetworkSpecific );
// Publicユーザ許可.
	if( gp_AdminUserAndDepartSynchro.ss_PublicUser == 0 ) {	// パブリックユーザー 許可しない.
//		PnlComGetUserAuthentication()->setCertificationAction( TYPE_CertificationActionOn, PnlComGetUser() );	//  ユーザー認証管理を行う
		PnlComGetUserAuthentication()->setCertificationAction( TYPE_CertificationActionOn, PnlComGetUser(), e_NetworkSpecific );	//  ユーザー認証管理を行う
	} else if( gp_AdminUserAndDepartSynchro.ss_PublicUser == 1 ) {	// パブリックユーザー 許可する.
//		PnlComGetUserAuthentication()->setCertificationAction( TYPE_CertificationActionUserAndPublic, PnlComGetUser() );	// ユーザー認証管理/PUBLICユーザー併用
		PnlComGetUserAuthentication()->setCertificationAction( TYPE_CertificationActionUserAndPublic, PnlComGetUser(), e_NetworkSpecific );	// ユーザー認証管理/PUBLICユーザー併用
		if( PnlComGetMachineSetting()->getKeyMode() == TYPE_KeyModeAdminMachineKM ) {
			//Publicユーザの機能制限を強制解除する.
// Comment 2017/09/12【HCLT-Sindhu】【IT5 4.2】【RQ686】【WP1012】Public user support for Network 2.
//			APIC_User* pc_User = APIC_User::newInstance( TYPD_IndexUserPublic );	// PublicユーザのCommonAPIインスタンス取得
			APIC_User* pc_User = NULL;
			if( TYPE_NetworkSpecific_NW1  ==  PNLC_NetworkLan::getInstance() ->getAdminNWLine( ) ) {
				pc_User = APIC_User::newInstance( TYPD_IndexUserPublic );	// PublicユーザのCommonAPIインスタンス取得
			} else {
				pc_User = APIC_User::newInstance( TYPD_IndexUserPublicNW2 );	// PublicユーザのCommonAPIインスタンス取得
			}
//	comment 2015/11/02 HCL_Ezhil [ZeusS_ZX0][制御構成変更_No.76-1] Adding error code.
//			if( NULL == pc_User ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }
			if( NULL == pc_User ) { 
//Comment 2019/06/14 [HCLT_Monisha][EagleH][N Rank] Removed unwanted Detailed code.
//				if (NULL !=pc_BFW_System){
//				    pc_BFW_System->registDetailCode(PNLD_DetailCode_Admin_0x1900030010413E01);
//				}
				LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" ); 
				return; 
			}
			TYPS_FunctionLimitation Limit_set = pc_User->getFunctionLimitation();
#if !defined(DEF_MonoCus)
			Limit_set.e_EnableCopy				 = TYPE_FuncLimit_Enable;					// コピー					：禁止/ブラックのみ許可/許可.
#else //DEF_MonoCus.
			Limit_set.e_EnableCopy				 = TYPE_FuncLimit_OnlyBlack;					// コピー					：禁止/ブラックのみ許可/許可.
#endif //DEF_MonoCus.
			Limit_set.e_EnableScan				 = TYPE_FuncLimit_Enable;					// スキャナー操作			：禁止/ブラックのみ許可/許可.
			Limit_set.e_EnableFax				 = TYPE_FuncLimit_Enable;					// FAX(IFAX)操作			：禁止/ブラックのみ許可/許可.
			Limit_set.e_EnableBoxOperation		 = TYPE_FuncLimit_Enable;					// 蓄積文書操作				：禁止/許可.
#if !defined(DEF_MonoCus)
			Limit_set.e_EnablePrintOfSendDocument= TYPE_FuncLimit_Enable;					// 送信機能印字				：禁止/ブラックのみ許可/許可.
#else //DEF_MonoCus.
			Limit_set.e_EnablePrintOfSendDocument= TYPE_FuncLimit_OnlyBlack;					// 送信機能印字				：禁止/ブラックのみ許可/許可.
#endif //DEF_MonoCus.
			Limit_set.e_EnableScanToUsb			 = TYPE_FuncLimit_Enable;					// USBメモリー保存			：禁止/許可.
			Limit_set.e_EnableUsbMemoryReading	 = TYPE_FuncLimit_Enable;					// 外部メモリー文書読込み	：禁止/許可.
			Limit_set.e_EnableAddressDirect		 = TYPE_FunctionLimit_AddressDirect_Enable;	// 手動宛先入力				：許可/不許可/G3 FAXのみ許可.
			Limit_set.e_EnableBrowser			 = TYPE_FuncLimit_Enable;					// Webブラウザー			：禁止/許可.
			pc_User->setFunctionLimitation( Limit_set, PnlComGetUser(), JLGE_InspectLogIfOperationPanel, False, PnlComGetAuthorityID() );	// 機能制限保存 .
			PnlComGetAuthDeviceCommon()->AuthUserDataCancel( PnlComGetUser() );
			PnlComGetAuthDeviceCommon()->AuthUserDataCancel( PnlComGetUser(), TYPE_AuthUserData_Select2 );
			pc_User->setRegistration( True, TYPE_RegistrationTypeMax, JLGE_InspectLogIfNon, False, TYPE_Registration_State_New, False, PnlComGetUser(), PnlComGetAuthorityID() );	// ユーザの登録状態をTrueにする.
//Comment 2017/10/17 [HCL-Hanuman] [IT5 4.2][AR1030231] .
//			APIC_Track* pc_Track = APIC_Track::newInstance( TYPD_IndexTrackPublic );			// PublicユーザのCommonAPIインスタンス取得.
			APIC_Track* pc_Track = NULL;
			if( TYPE_NetworkSpecific_NW1 == PNLC_NetworkLan::getInstance() ->getAdminNWLine ( ) ) {
				pc_Track = APIC_Track::newInstance( TYPD_IndexTrackPublic );			// PublicユーザのCommonAPIインスタンス取得.
			} else {
				pc_Track = APIC_Track::newInstance( TYPD_IndexTrackPublicNW2 );			// PublicユーザのCommonAPIインスタンス取得.
			}
			if( NULL != pc_Track ) {	// NULLチェックを追加.
				pc_Track->setRegistration( True, TYPE_RegistrationTypeMax, JLGE_InspectLogIfNon, False, TYPE_Registration_State_New, PnlComGetUser(), PnlComGetAuthorityID() );	// 部門の登録状態をTrueにする.
			}
		}
	} else if( gp_AdminUserAndDepartSynchro.ss_PublicUser == 2 ) {	// パブリックユーザ許可後、ログインする.
//		PnlComGetUserAuthentication()->setCertificationAction( TYPE_CertificationActionUserAndPublic, PnlComGetUser() );	// ユーザー認証管理/PUBLICユーザー併用
//		PnlComGetUserAuthentication()->setEnablePublicUserNoAuthentication( True );
		PnlComGetUserAuthentication()->setCertificationAction( TYPE_CertificationActionUserAndPublic, PnlComGetUser(), e_NetworkSpecific );	// ユーザー認証管理/PUBLICユーザー併用
		PnlComGetUserAuthentication()->setEnablePublicUserNoAuthentication( True, e_NetworkSpecific );
	} else
//	{ PnlComGetUserAuthentication()->setCertificationAction( TYPE_CertificationActionOnlyPublic, PnlComGetUser() ); }	// ユーザー認証なし(PUBLICユーザーのみ)
	{ PnlComGetUserAuthentication()->setCertificationAction( TYPE_CertificationActionOnlyPublic, PnlComGetUser(), e_NetworkSpecific ); }	// ユーザー認証なし(PUBLICユーザーのみ)
// 部門管理設定.
	if( gp_AdminUserAndDepartSynchro.b_Manage == False )
//	{ PnlComGetTrackManager()->setTrackMode( TYPE_TrackModeOff ); }					//	認証しない
	{ PnlComGetTrackManager()->setTrackMode( TYPE_TrackModeOff, e_NetworkSpecific ); }					//	認証しない
	else if( gp_AdminUserAndDepartSynchro.ss_departForm == 1 )
//	{ PnlComGetTrackManager()->setTrackMode( TYPE_TrackModeOnlyPassword ); }	//	パスワードのみ
	{ PnlComGetTrackManager()->setTrackMode( TYPE_TrackModeOnlyPassword, e_NetworkSpecific ); }	//	パスワードのみ
	else if( gp_AdminUserAndDepartSynchro.ss_departForm == 0 && gp_AdminUserAndDepartSynchro.b_Synchro == True )
//	{ PnlComGetTrackManager()->setTrackMode( TYPE_TrackModeNoAndPassword ); }	//	部門名＋パスワード
	{ PnlComGetTrackManager()->setTrackMode( TYPE_TrackModeNoAndPassword, e_NetworkSpecific ); }	//	部門名＋パスワード
	else if( PNLE_TrackModeLink_UserSelect == pdpc_UserAndTrackMode.getTrackModeLink() ) {
//		PnlComGetTrackManager()->setTrackMode( TYPE_TrackModeUserSelect );					// ユーザー設定を使用する.
		PnlComGetTrackManager()->setTrackMode( TYPE_TrackModeUserSelect, e_NetworkSpecific );					// ユーザー設定を使用する.
	} else {
//		PnlComGetTrackManager()->setTrackMode( TYPE_TrackModeOnWithUserAuthentication );	// ユーザー認証と合わせて部門管理する.
		PnlComGetTrackManager()->setTrackMode( TYPE_TrackModeOnWithUserAuthentication, e_NetworkSpecific );	// ユーザー認証と合わせて部門管理する.
	}
// Comment 2018/01/29 HCLT_Hanuman [IT5_4.2][AR1038061] To improve performance.
//// Comment 2018/01/18 HCLT_Binesh [IT5_4.2][AR1034977] Moving the code from bottom of function as NW2 user count need to set to API before calculating the track and cache count.
//	if ( True == APIC_VlanSetting::newInstance()->getVlanEnable() ) {
	if ( True == PNLC_NetworkLan::getInstance()->getVlanEnableWrapper() ) {
		if ( TYPE_UserAuthenticationOff == PnlComGetUserAuthentication()->getUserAuthentication( TYPE_NetworkSpecific_NW2 ) ) {
			PnlComGetTrackManager()->setNW2UserCntNo(0);
		} else {
			if ( TYPE_UserAuthenticationOff == PnlComGetUserAuthentication()->getUserAuthentication( TYPE_NetworkSpecific_NW1 ) 
				&& TYPE_UserAuthenticationOff != PnlComGetUserAuthentication()->getUserAuthentication( TYPE_NetworkSpecific_NW2 ) ) {
					if ( TYPE_TrackModeOff == PnlComGetTrackManager()->getTrackMode( TYPE_NetworkSpecific_NW1 ) ) {
						PnlComGetTrackManager()->setNW2UserCntNo(1000);
					} else {
						PnlComGetTrackManager()->setNW2UserCntNo(500);
					}
			} else if ( TYPE_UserAuthenticationOff != PnlComGetUserAuthentication()->getUserAuthentication( TYPE_NetworkSpecific_NW2 ) ) {
				if ( TYPE_TrackModeOff != PnlComGetTrackManager()->getTrackMode( TYPE_NetworkSpecific_NW1 )
					 || TYPE_UserAuthenticationOff != PnlComGetUserAuthentication()->getUserAuthentication( TYPE_NetworkSpecific_NW1 ) ) {
					PnlComGetTrackManager()->setNW2UserCntNo(500);

				}
			}
		}
	} else {
		PnlComGetTrackManager()->setNW2UserCntNo(0);
	}
// ユーザカウント数を保存
	if( True == PLBC_FuncVersionCheck::isOK_MiZe_Type2() ) {
		if( gp_AdminUserAndDepartSynchro.sc_UserAttest == 0 && gp_AdminUserAndDepartSynchro.b_Manage == True ) {
			// 部門認証のみ.
//Comment 2017/09/29 [HCL-Harish] [IT5 4.2][RQ-686] [WP533/545] Multiple Network support - User/Track.			
//			PnlComGetTrackManager()->setTrackCntNo( PnlComGetTrackManager()->getMaxTrack() );
//Comment 2017/11/01 [HCL-Harish] [IT5 4.2][AR1030218-reopen] User Registeration and track - Counter.
//			PnlComGetTrackManager()->setTrackCntNo( GetMaxTrackCounter() );
			if ( TYPE_UserAuthenticationOff != PnlComGetUserAuthentication()->getUserAuthentication( TYPE_NetworkSpecific_NW2 )  ) {
				PnlComGetTrackManager()->setTrackCntNo( PnlComGetTrackManager()->getMaxTrack() / 2 );
			} else {
				PnlComGetTrackManager()->setTrackCntNo( PnlComGetTrackManager()->getMaxTrack() );
			}
		}
		else if( gp_AdminUserAndDepartSynchro.sc_UserAttest == 5 && gp_AdminUserAndDepartSynchro.b_Manage == True ) {
			// 本体+拡張サーバー認証、部門認証あり.
//Comment 2017/09/29 [HCL-Harish] [IT5 4.2][RQ-686] [WP533/545] Multiple Network support - User/Track.			
//			PnlComGetTrackManager()->setTrackCntNo( PnlComGetTrackManager()->getMaxTrack() - gp_AdminUserAndDepartSynchro.ss_UserCounter - gp_AdminUserAndDepartSynchro.m_ss_ExtCounter );
			PnlComGetTrackManager()->setTrackCntNo( GetMaxTrackCounter() - gp_AdminUserAndDepartSynchro.ss_UserCounter - gp_AdminUserAndDepartSynchro.m_ss_ExtCounter );
			PnlComGetTrackManager()->setCacheCntNo( gp_AdminUserAndDepartSynchro.m_ss_ExtCounter );
		}
		else if( gp_AdminUserAndDepartSynchro.sc_UserAttest == 5 && gp_AdminUserAndDepartSynchro.b_Manage == False ) {
			// 本体+拡張サーバー認証、部門管理なし.
			// 「本体+拡張サーバー認証」の場合は、部門カウンターの設定は無いが、
			// ユーザーカウンター+拡張カウンターが合計1000未満になるケースが仕様として存在する為、
			// 計算上必要となっている。
//Comment 2017/09/29 [HCL-Harish] [IT5 4.2][RQ-686] [WP533/545] Multiple Network support - User/Track.			
//			PnlComGetTrackManager()->setTrackCntNo( PnlComGetTrackManager()->getMaxTrack() - gp_AdminUserAndDepartSynchro.ss_UserCounter - gp_AdminUserAndDepartSynchro.m_ss_ExtCounter );
			PnlComGetTrackManager()->setTrackCntNo( GetMaxTrackCounter() - gp_AdminUserAndDepartSynchro.ss_UserCounter - gp_AdminUserAndDepartSynchro.m_ss_ExtCounter );
			PnlComGetTrackManager()->setCacheCntNo( gp_AdminUserAndDepartSynchro.m_ss_ExtCounter );
		}
		else if( gp_AdminUserAndDepartSynchro.sc_UserAttest != 0 && gp_AdminUserAndDepartSynchro.b_Manage == False ) {
			// ユーザ認証のみ.
// Comment 2018/01/29 HCLT_Hanuman [IT5_4.2][AR1038061] To improve performance.
////Comment 2017/10/29 [HCL-Harish] [IT5 4.2][WP1146] User Registeration and track - Counter.
////		PnlComGetTrackManager()->setTrackCntNo( 0 );
//			if( True == APIC_VlanSetting::newInstance()->getVlanEnable() ) {
			if ( True ==  PNLC_NetworkLan::getInstance()->getVlanEnableWrapper() ) {
// Comment 2018/01/29 HCLT_Hanuman [IT5_4.2][AR1038061] To improve performance.
				if ( TYPE_NetworkSpecific_NW1 == e_NetworkSpecific ) {
					PnlComGetTrackManager()->setTrackCntNo( 0 );
				} else {
					if ( bl_JudgeAuthMethodNW2 == True ) {
						if ( TYPE_TrackModeOff != PnlComGetTrackManager()->getTrackMode( TYPE_NetworkSpecific_NW1 ) ) {
							PnlComGetTrackManager()->setTrackCntNo( PnlComGetTrackManager()->getTrackCntNo() / 2 );
						} else if ( TYPE_UserAuthenticationDeviceAndMidAuth ==  PnlComGetUserAuthentication()->getUserAuthentication( TYPE_NetworkSpecific_NW1 )){
//Comment 2017/11/01 [HCL-Harish] [IT5 4.2][AR1030218-reopen] Total Count of NW1&NW2 Users + extension + Track should be 1000.
//							PnlComGetTrackManager()->setTrackCntNo( GetMaxTrackCounter() - (gp_AdminUserAndDepartSynchro.ss_UserCounter / 2 ) - gp_AdminUserAndDepartSynchro.m_ss_ExtCounter );	
							PnlComGetTrackManager()->setTrackCntNo( 100 );
						}
						if ( TYPE_UserAuthenticationIntermeddiateServer == PnlComGetUserAuthentication()->getUserAuthentication( TYPE_NetworkSpecific_NW1 ) 
					 		|| TYPE_UserAuthenticationDeviceAndMidAuth == PnlComGetUserAuthentication()->getUserAuthentication( TYPE_NetworkSpecific_NW1 ) ) {
							PnlComGetTrackManager()->setCacheCntNo( PnlComGetTrackManager()->getCacheCntNo() / 2 );
						}
					}
				}
			} else {
				PnlComGetTrackManager()->setTrackCntNo( 0 );
			}
		}
		else if( gp_AdminUserAndDepartSynchro.sc_UserAttest != 0 && gp_AdminUserAndDepartSynchro.b_Manage == True ) {
			// ユーザ、部門認証共にする.
//Comment 2017/09/29 [HCL-Harish] [IT5 4.2][RQ-686] [WP533/545] Multiple Network support - User/Track.			
//			PnlComGetTrackManager()->setTrackCntNo( PnlComGetTrackManager()->getMaxTrack() - gp_AdminUserAndDepartSynchro.ss_UserCounter );
			PnlComGetTrackManager()->setTrackCntNo( GetMaxTrackCounter() - gp_AdminUserAndDepartSynchro.ss_UserCounter );
		}
		else if( gp_AdminUserAndDepartSynchro.sc_UserAttest == 0 && gp_AdminUserAndDepartSynchro.b_Manage == False ) {
			// 両方しない場合は0を入れておく.
// Comment 2018/01/29 HCLT_Hanuman [IT5_4.2][AR1038061] To improve performance.
////Comment 2017/10/29 [HCL-Harish] [IT5 4.2][WP1146] User Registeration and track - Counter.
////		PnlComGetTrackManager()->setTrackCntNo( 0 );
//			if( True == APIC_VlanSetting::newInstance()->getVlanEnable() ) {
			if ( True ==  PNLC_NetworkLan::getInstance()->getVlanEnableWrapper() ) {
				if ( TYPE_NetworkSpecific_NW1 == e_NetworkSpecific ) {
					PnlComGetTrackManager()->setTrackCntNo( 0 );
				} else {
					if ( bl_JudgeAuthMethodNW2 == False ) {
						if ( TYPE_TrackModeOff != PnlComGetTrackManager()->getTrackMode( TYPE_NetworkSpecific_NW1 ) ) {
							PnlComGetTrackManager()->setTrackCntNo( PnlComGetTrackManager()->getTrackCntNo() * 2 );
//Comment 2017/11/01 [HCL-Harish] [IT5 4.2][AR1030218-reopen] Total Count of NW1&NW2 Users + extension + Track should be 1000.
//						}
						} else if ( TYPE_UserAuthenticationDeviceAndMidAuth ==  PnlComGetUserAuthentication()->getUserAuthentication( TYPE_NetworkSpecific_NW1 ) ) {
							PnlComGetTrackManager()->setTrackCntNo( 200 );	
						}	
						if ( TYPE_UserAuthenticationIntermeddiateServer == PnlComGetUserAuthentication()->getUserAuthentication( TYPE_NetworkSpecific_NW1 ) 
					 		|| TYPE_UserAuthenticationDeviceAndMidAuth == PnlComGetUserAuthentication()->getUserAuthentication( TYPE_NetworkSpecific_NW1 ) ) {
							PnlComGetTrackManager()->setCacheCntNo( PnlComGetTrackManager()->getCacheCntNo() * 2 );
						}
					}
				}	
			} else {
				PnlComGetTrackManager()->setTrackCntNo( 0 );
			}
		}
		else {
			// ここに入る事は無いが、0を入れておく.
			PnlComGetTrackManager()->setTrackCntNo( 0 );
		}
	}
	else {
		if( gp_AdminUserAndDepartSynchro.sc_UserAttest == 0 && gp_AdminUserAndDepartSynchro.b_Manage == True )	// 部門認証のみ.
//Comment 2017/09/29 [HCL-Harish] [IT5 4.2][RQ-686] [WP533/545] Multiple Network support - User/Track.		
//		{ PnlComGetTrackManager()->setTrackCntNo( PnlComGetTrackManager()->getMaxTrack() ); }
		{ PnlComGetTrackManager()->setTrackCntNo( GetMaxTrackCounter() ); }
		if( gp_AdminUserAndDepartSynchro.sc_UserAttest != 0 && gp_AdminUserAndDepartSynchro.b_Manage == False )	// ユーザ認証のみ.
		{ PnlComGetTrackManager()->setTrackCntNo( 0 ); }
		if( gp_AdminUserAndDepartSynchro.sc_UserAttest != 0 && gp_AdminUserAndDepartSynchro.b_Manage == True )	// ユーザ、部門認証共にする.
//Comment 2017/09/29 [HCL-Harish] [IT5 4.2][RQ-686] [WP533/545] Multiple Network support - User/Track.	
//		{ PnlComGetTrackManager()->setTrackCntNo( PnlComGetTrackManager()->getMaxTrack() - gp_AdminUserAndDepartSynchro.ss_UserCounter ); }
		{ PnlComGetTrackManager()->setTrackCntNo( GetMaxTrackCounter() - gp_AdminUserAndDepartSynchro.ss_UserCounter ); }
		if( gp_AdminUserAndDepartSynchro.sc_UserAttest == 0 && gp_AdminUserAndDepartSynchro.b_Manage == False )
		{ PnlComGetTrackManager()->setTrackCntNo( 0 ); }	// 両方しない場合は0を入れておく.
	}
	APIC_CertInfoSetting*	pc_CertInfoSetting = APIC_CertInfoSetting::newInstance();
	uint32_t ui32_CertID = pc_CertInfoSetting->getDefaultCert();		// デフォルト証明書IDを取得、SSLが発行されていない場合は0が返る.
	if( gp_AdminUserAndDepartSynchro.sc_UserAttest_At != 1 && gp_AdminUserAndDepartSynchro.sc_UserAttest == 1 	// 外部認証ではない → 外部認証設定の場合.
#ifdef SIMULATOR	// シミュレータで有効
	    && onlyPanel.bl_SSLCertInfo == True )	// SSLの認証情報のCountryCodeがNullでない場合は認証されている.
#else
	    && 0 != ui32_CertID )			// SSLのデフォルト証明書IDが0以外の場合(SSLが発行されていない場合は0が入っている).
#endif	// #ifdef SIMULATOR
	{
// 2016/02/01 CQ521285 監査ログ HCD-PP対応 Sky大嶋 MOD Start.
		PnlComGetTcpSocketSetting()->setEnableSSL( True, JLGE_InspectLogOperateIDL_Admin, JLGE_InspectLogIfOperationPanel, False );						// TcpSocketのSSL設定を強制的にONにする.
//Comment 2017/12/19 [HCL-Catherine] [IT5 4.2][RQ-686] [ WP1377]The new API is used and to detect the current network type, new API arguments are passed.
//		PnlComGetOpenApiSetting()->setSslOn( True, JLGE_InspectLogOperateIDL_Admin, JLGE_InspectLogIfOperationPanel, False );							// IS OpenAPIのSSL設定を強制的にONにする.
		PnlComGetOpenApiSetting()->setSslOn( True, JLGE_InspectLogOperateIDL_Admin, JLGE_InspectLogIfOperationPanel, False, NULL, e_NetworkSpecific );							// IS OpenAPIのSSL設定を強制的にONにする.
		PnlComGetPswcMibSetting()->setHttpsType(TYPE_HttpsTypeAlwaysEnable, JLGE_InspectLogOperateIDL_Admin, JLGE_InspectLogIfOperationPanel, False);	// PSWCのSSL設定を強制的にONにする.
// 2016/02/01 CQ521285 監査ログ HCD-PP対応 Sky大嶋 MOD End.
	}
	if( gp_AdminUserAndDepartSynchro.sc_UserAttest_At != 3 && gp_AdminUserAndDepartSynchro.sc_UserAttest == 3 	// 中間サーバー認証ではない → 中間サーバー認証設定の場合.
#ifdef SIMULATOR	// シミュレータで有効
	    && onlyPanel.bl_SSLCertInfo == True )	// SSLの認証情報のCountryCodeがNullでない場合は認証されている.
#else
	    && 0 != ui32_CertID )			// SSLのデフォルト証明書IDが0以外の場合(SSLが発行されていない場合は0が入っている).
#endif	// #ifdef SIMULATOR
	{
// 2016/02/01 CQ521285 監査ログ HCD-PP対応 Sky大嶋 MOD Start.
		PnlComGetTcpSocketSetting()->setEnableSSL( True, JLGE_InspectLogOperateIDL_Admin, JLGE_InspectLogIfOperationPanel, False );						// TcpSocketのSSL設定を強制的にONにする.
//Comment 2017/12/19 [HCL-Catherine] [IT5 4.2][RQ-686] [ WP1377]The new API is used and to detect the current network type, new API arguments are passed.
//		PnlComGetOpenApiSetting()->setSslOn( True, JLGE_InspectLogOperateIDL_Admin, JLGE_InspectLogIfOperationPanel, False );							// IS OpenAPIのSSL設定を強制的にONにする.
		PnlComGetOpenApiSetting()->setSslOn( True, JLGE_InspectLogOperateIDL_Admin, JLGE_InspectLogIfOperationPanel, False, NULL, e_NetworkSpecific );							// IS OpenAPIのSSL設定を強制的にONにする.
		PnlComGetPswcMibSetting()->setHttpsType(TYPE_HttpsTypeAlwaysEnable, JLGE_InspectLogOperateIDL_Admin, JLGE_InspectLogIfOperationPanel, False);	// PSWCのSSL設定を強制的にONにする.
// 2016/02/01 CQ521285 監査ログ HCD-PP対応 Sky大嶋 MOD End.
	}
	if( gp_AdminUserAndDepartSynchro.sc_UserAttest_At != 4 && gp_AdminUserAndDepartSynchro.sc_UserAttest == 4 	// 本体+外部サーバー認証ではない → 本体+外部サーバー認証設定の場合.
#ifdef SIMULATOR	// シミュレータで有効
	    && onlyPanel.bl_SSLCertInfo == True )	// SSLの認証情報のCountryCodeがNullでない場合は認証されている.
#else
	    && 0 != ui32_CertID )			// SSLのデフォルト証明書IDが0以外の場合(SSLが発行されていない場合は0が入っている).
#endif	// #ifdef SIMULATOR
	{
// 2016/02/01 CQ521285 監査ログ HCD-PP対応 Sky大嶋 MOD Start.
		PnlComGetTcpSocketSetting()->setEnableSSL( True, JLGE_InspectLogOperateIDL_Admin, JLGE_InspectLogIfOperationPanel, False );						// TcpSocketのSSL設定を強制的にONにする.
//Comment 2017/12/19 [HCL-Catherine] [IT5 4.2][RQ-686] [ WP1377]The new API is used and to detect the current network type, new API arguments are passed.
//		PnlComGetOpenApiSetting()->setSslOn( True, JLGE_InspectLogOperateIDL_Admin, JLGE_InspectLogIfOperationPanel, False );							// IS OpenAPIのSSL設定を強制的にONにする.
		PnlComGetOpenApiSetting()->setSslOn( True, JLGE_InspectLogOperateIDL_Admin, JLGE_InspectLogIfOperationPanel, False, NULL, e_NetworkSpecific  );							// IS OpenAPIのSSL設定を強制的にONにする.
		PnlComGetPswcMibSetting()->setHttpsType(TYPE_HttpsTypeAlwaysEnable, JLGE_InspectLogOperateIDL_Admin, JLGE_InspectLogIfOperationPanel, False);	// PSWCのSSL設定を強制的にONにする.
// 2016/02/01 CQ521285 監査ログ HCD-PP対応 Sky大嶋 MOD End.
	}

	if( gp_AdminUserAndDepartSynchro.sc_UserAttest_At != 5 && gp_AdminUserAndDepartSynchro.sc_UserAttest == 5 	// 本体+拡張サーバー認証ではない → 本体+外部サーバー認証設定の場合.
	    && 0 != ui32_CertID )			// SSLのデフォルト証明書IDが0以外の場合(SSLが発行されていない場合は0が入っている).
	{
// 2016/02/01 CQ521285 監査ログ HCD-PP対応 Sky大嶋 MOD Start.
		PnlComGetTcpSocketSetting()->setEnableSSL( True, JLGE_InspectLogOperateIDL_Admin, JLGE_InspectLogIfOperationPanel, False );						// TcpSocketのSSL設定を強制的にONにする.
//Comment 2017/12/19 [HCL-Catherine] [IT5 4.2][RQ-686] [ WP1377]The new API is used and to detect the current network type, new API arguments are passed.
//		PnlComGetOpenApiSetting()->setSslOn( True, JLGE_InspectLogOperateIDL_Admin, JLGE_InspectLogIfOperationPanel, False );							// IS OpenAPIのSSL設定を強制的にONにする.
		PnlComGetOpenApiSetting()->setSslOn( True, JLGE_InspectLogOperateIDL_Admin, JLGE_InspectLogIfOperationPanel, False, NULL, e_NetworkSpecific );							// IS OpenAPIのSSL設定を強制的にONにする.
		PnlComGetPswcMibSetting()->setHttpsType(TYPE_HttpsTypeAlwaysEnable, JLGE_InspectLogOperateIDL_Admin, JLGE_InspectLogIfOperationPanel, False);	// PSWCのSSL設定を強制的にONにする.
// 2016/02/01 CQ521285 監査ログ HCD-PP対応 Sky大嶋 MOD End.
	}
	if( ppc_SystemInfo->getExternalIFMode() == TYPE_ExternalIFMode1 ) {
		PnlComGetSystemSetting()->setEnableFieryPullScan( gp_AdminUserAndDepartSynchro.b_isEnableFieryPullScan );
	}
// 2016/02/01 CQ521285 監査ログ HCD-PP対応 Sky大嶋 MOD Start.
	PnlComGetUserAuthenticationSetting()->setActiveDirectoryTicketLife( gp_AdminUserAndDepartSynchro.ss_ConnectTime, JLGE_InspectLogOperateIDL_Admin, JLGE_InspectLogIfOperationPanel, False );
// 2016/02/01 CQ521285 監査ログ HCD-PP対応 Sky大嶋 MOD End.
// comment 2017/01/20 SCD2-Sky後藤 [IT5_4.0][1012607]Authentication Manager使用時に認証しないに設定すると本体装置認証、本体＋外部サーバー認証キーがグレーアウトしてしまう.
//	PnlComGetUserAuthentication()->setMidauthControl( gp_AdminUserAndDepartSynchro.b_AuthorityManagement );	// 権限管理/カウンター管理情報の設定.	//DonauLastFum【Erie拡張】【13笹岡】20090518 add.
	APIC_UserAuthentication* pc_UserAuth = PnlComGetUserAuthentication();
	if( NULL != pc_UserAuth ){
//		if( TYPE_UserAuthenticationServer == pc_UserAuth->getUserAuthentication() ){				// 外部サーバー認証時.
		if( TYPE_UserAuthenticationServer == pc_UserAuth->getUserAuthentication( e_NetworkSpecific ) ){				// 外部サーバー認証時.
			PnlComGetUserAuthentication()->setMidauthControl( gp_AdminUserAndDepartSynchro.b_AuthorityManagement );	// 権限管理/カウンター管理情報の設定.	//DonauLastFum【Erie拡張】【13笹岡】20090518 add.
		}
	}
	if( True == PLBC_FuncPKICheck::isPKI() ) {
		if( False == gp_AdminUserAndDepartSynchro.b_MiddleServerSetting ) {
			// 拡張サーバー認証に設定する.
//			PnlComGetUserAuthentication()->setUserAuthentication( TYPE_UserAuthenticationIntermeddiateServer, TYPE_InspectLogIfOperationPanel, False, PnlComGetUser(), PnlComGetAuthorityID() );
//			PnlComGetTrackManager()->setTrackMode( TYPE_TrackModeOff );
			PnlComGetUserAuthentication()->setUserAuthentication( TYPE_UserAuthenticationIntermeddiateServer, TYPE_InspectLogIfOperationPanel, False, PnlComGetUser(), PnlComGetAuthorityID(), e_NetworkSpecific );
			PnlComGetTrackManager()->setTrackMode( TYPE_TrackModeOff, e_NetworkSpecific );
		} else {
			// 外部サーバー認証に設定する.
//			PnlComGetUserAuthentication()->setUserAuthentication( TYPE_UserAuthenticationServer, TYPE_InspectLogIfOperationPanel, False, PnlComGetUser(), PnlComGetAuthorityID() );
			PnlComGetUserAuthentication()->setUserAuthentication( TYPE_UserAuthenticationServer, TYPE_InspectLogIfOperationPanel, False, PnlComGetUser(), PnlComGetAuthorityID(), e_NetworkSpecific );
		}
	}

// Comment 2018/01/18 HCLT_Binesh [IT5_4.2][AR1034977] Commenting the code from bottom of function as NW2 user count need to set to API before calculating the track and cache count.
////Comment 2017/10/29 [HCL-Harish] [IT5 4.2][WP1146] User Registeration and track - Counter.
//	if ( True == APIC_VlanSetting::newInstance()->getVlanEnable() ) {
//		if ( TYPE_UserAuthenticationOff == PnlComGetUserAuthentication()->getUserAuthentication( TYPE_NetworkSpecific_NW2 ) ) {
//			PnlComGetTrackManager()->setNW2UserCntNo(0);
//		} else {
//			if ( TYPE_UserAuthenticationOff == PnlComGetUserAuthentication()->getUserAuthentication( TYPE_NetworkSpecific_NW1 ) 
//				&& TYPE_UserAuthenticationOff != PnlComGetUserAuthentication()->getUserAuthentication( TYPE_NetworkSpecific_NW2 ) ) {
//					if ( TYPE_TrackModeOff == PnlComGetTrackManager()->getTrackMode( TYPE_NetworkSpecific_NW1 ) ) {
//						PnlComGetTrackManager()->setNW2UserCntNo(1000);
//					} else {
//						PnlComGetTrackManager()->setNW2UserCntNo(500);
//					}
//			} else if ( TYPE_UserAuthenticationOff != PnlComGetUserAuthentication()->getUserAuthentication( TYPE_NetworkSpecific_NW2 ) ) {
//				if ( TYPE_TrackModeOff != PnlComGetTrackManager()->getTrackMode( TYPE_NetworkSpecific_NW1 )
//					 || TYPE_UserAuthenticationOff != PnlComGetUserAuthentication()->getUserAuthentication( TYPE_NetworkSpecific_NW1 ) ) {
//					PnlComGetTrackManager()->setNW2UserCntNo(500);
//				}
//			}
//		}
//	} else {
//		PnlComGetTrackManager()->setNW2UserCntNo(0);
//	}
	APIC_AuthenticationSeverSetting* apicAuthSrv = APIC_AuthenticationSeverSetting::newInstance();
//	comment 2015/11/02 HCL_Ezhil [ZeusS_ZX0][制御構成変更_No.76-1] Adding error code.
//	if( NULL == apicAuthSrv ) {LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }
	if( NULL == apicAuthSrv ) {
//Comment 2019/06/14 [HCLT_Monisha][EagleH][N Rank] Removed unwanted Detailed code.
//		if (NULL !=pc_BFW_System){
//		    pc_BFW_System->registDetailCode(PNLD_DetailCode_Admin_0x1900030010413F01);
//		}
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" ); 
		return; 
	}
	
	if( ( False == apicAuthSrv->checkServerGroup() ) 
		|| ( False == PnlComGetAuthDeviceCommon()->checkIccChangeAuthServer() ) 
// comment 2017/10/31 【HCL-TANU】【IT5_4.2】【RQ686】【IT Defect AR1030817].
//		|| ( True == PnlComGetUserAuthentication()->getMidauthControl() ) ) {
	    || ( True == PnlComGetUserAuthentication()->getMidauthControl(PNLC_NetworkLan::getInstance() ->getAdminNWLine( )) ) ) {
		// サーバーグループ設定が無効な場合.
				
		// 連携する外部サーバのIndex番号を設定.
		apicAuthSrv->setAuthCardServerIndex( uc_AuthCardServerIndex );

		// 認証サーバー情報を設定.
		APIS_AuthenticationServerInfo server_info;	// サーバー情報.
		memset( &server_info, 0, sizeof( server_info ) );
		server_info.e_ServerType = TYPE_AuthenticationServerTypeIccLDAP;
		server_info.s_IccLDAP = s_AuthCardParams;
//		apicAuthSrv->setServerInfo( APID_AuthenticationSeverTypeIccLDAP, &server_info );
		apicAuthSrv->setServerInfo( APID_AuthenticationSeverTypeIccLDAP, &server_info, e_NetworkSpecific );

		// 代替認証サーバー情報を設定.
		APIS_AuthenticationServerInfo secondaryServer_info;	// 代替サーバー情報.
		memset( &secondaryServer_info, 0, sizeof( secondaryServer_info ) );
		secondaryServer_info.e_ServerType = TYPE_AuthenticationServerTypeIccLDAP;
		secondaryServer_info.s_IccLDAP = m_s_AuthCardSecondaryServerParams;
//		apicAuthSrv->setServerInfo( APID_AuthenticationSeverTypeIccLDAPSecond, &secondaryServer_info );
		apicAuthSrv->setServerInfo( APID_AuthenticationSeverTypeIccLDAPSecond, &secondaryServer_info, e_NetworkSpecific );

	// comment 2015/03/19 OPS1-MSE竹中 [MinervaSBK][RQ-278] LDAPサーバーへのカード情報登録機能対応.
		// カード情報登録設定On/Offを設定.
		apicAuthSrv->setIccLdapCardWriteEnable( m_bl_CardRegistration );
	}
	
	// 認証検索機能On/Offを設定.
//	apicAuthSrv->setAuthCardSearch( b_AuthCardServerSearching );
	apicAuthSrv->setAuthCardSearch( b_AuthCardServerSearching, e_NetworkSpecific );
	
	// サーバーグループ有効時に、認証サーバ切り替えが未対応又は拡張サーバーが有効の場合.
	if( ( True == apicAuthSrv->checkServerGroup() ) 
		&& ( ( False == PnlComGetAuthDeviceCommon()->checkIccChangeAuthServer() ) 
// comment 2017/10/31 【HCL-TANU】【IT5_4.2】【RQ686】【IT Defect AR1030817].
//			|| ( True == PnlComGetUserAuthentication()->getMidauthControl() ) ) ) {
	        || ( True == PnlComGetUserAuthentication()->getMidauthControl(PNLC_NetworkLan::getInstance() ->getAdminNWLine( )) ) ) ) {
		// 検索時のカード情報文字種を設定.
		apicAuthSrv->setCardSearchLetters( m_e_CardAttr );
	}
		
	// ユーザー情報の上書き設定を変更する.
//	apicAuthSrv->setUserControlOverwrite( gp_AdminUserAndDepartSynchro.b_UserInfoOverwrite );
	apicAuthSrv->setUserControlOverwrite( gp_AdminUserAndDepartSynchro.b_UserInfoOverwrite, e_NetworkSpecific );
	// デフォルト認証方式を変更する.
//	PnlComGetUserAuthentication()->setDefaultUserAuthentication( gp_AdminUserAndDepartSynchro.sc_DefaulUserAttest );
	PnlComGetUserAuthentication()->setDefaultUserAuthentication( gp_AdminUserAndDepartSynchro.sc_DefaulUserAttest, e_NetworkSpecific );
// comment 2015/2/12 OPS1-Sky工藤 [ZeusS][CQ509844].
	if( True == PLBC_FuncVersionCheck::isOK_IT5_Type1()){
		APIC_NFCSetting* pc_NFCSetting = APIC_NFCSetting::newInstance();
		if ( NULL != pc_NFCSetting ) {
// comment 2017/02/27 SCD2-Sky延原 [IT5_4.0][AR1005533].
//			pc_NFCSetting->setUseNFC( onlyPanel.getNFCUse() );
//			pc_NFCSetting->setAuthNFC( onlyPanel.getNFCAuth() );
			onlyPanel.setNFCUse( m_e_NFCUse );
			onlyPanel.setNFCAuth( m_e_NFCAuth );
			pc_NFCSetting->setUseNFC( m_e_NFCUse );
			pc_NFCSetting->setAuthNFC( m_e_NFCAuth );
		}
		APIC_BLESetting* pc_BLESetting = APIC_BLESetting::newInstance();
		if ( NULL != pc_BLESetting ) {
// 2016/02/01 CQ521285 監査ログHCD-PP対応	Sky平田	Start.
// comment 2017/02/27 SCD2-Sky延原 [IT5_4.0][AR1005533].
//			pc_BLESetting->setUseBLE( onlyPanel.getBLEUse(), JLGE_InspectLogOperateIDL_Admin, JLGE_InspectLogIfOperationPanel, False );
//			pc_BLESetting->setAuthBLE( onlyPanel.getBLEAuth(), JLGE_InspectLogOperateIDL_Admin, JLGE_InspectLogIfOperationPanel, False );
			onlyPanel.setBLEUse( m_e_BLEUse );
			onlyPanel.setBLEAuth( m_e_BLEAuth );
			pc_BLESetting->setUseBLE( m_e_BLEUse, JLGE_InspectLogOperateIDL_Admin, JLGE_InspectLogIfOperationPanel, False );
			pc_BLESetting->setAuthBLE( m_e_BLEAuth, JLGE_InspectLogOperateIDL_Admin, JLGE_InspectLogIfOperationPanel, False );
// 2016/02/01 CQ521285 監査ログHCD-PP対応	Sky平田	End.
		}
	}
// comment 2016/5/25 [AUTHFUM2] [RQ-526]外部サーバーLDAP検索高速化 第22製品開発 山本.
	// 外部サーバーDNキャッシュ.
	APIC_DNCacheManager* apicDNCacheMan = APIC_DNCacheManager::newInstance();
	if( NULL != apicDNCacheMan ) {
//		apicDNCacheMan->setDNCache( b_ExtServerDNCache == True ? TYPE_DNCache_Enable : TYPE_DNCache_Disable );
		apicDNCacheMan->setDNCache( b_ExtServerDNCache == True ? TYPE_DNCache_Enable : TYPE_DNCache_Disable, e_NetworkSpecific);
	}

// comment 2016/5/25 [AUTHFUM2] [RQ-531]本体認証データ登録拡張 第22製品開発 山本.
// comment 2016/11/14 PDD22 出口(日立) [IT5_4.0][AccuRev.1006928]【RQ-531】本体認証データ登録拡張：拡張ユーザーDBの設定値クリア処理を追加.
// 拡張ユーザーDB設定値をクリアする条件を満たしている場合、拡張ユーザーDB設定値を使用しないに設定する.
// 条件を満たしていない場合は、指定された値をそのまま設定する.
	// 拡張ユーザーDB.
	if( True == b_LScaleSettingClear ) {
		// 拡張ユーザーDB設定値のクリアの場合.
		PnlComGetUserAuthentication()->setAuthInsideLargeScale( False );			// 拡張ユーザーDBを使用しないにする.
	}
	else {
		PnlComGetUserAuthentication()->setAuthInsideLargeScale( b_AuthInsideLargeScale );
	}
	b_LScaleSettingClear = False;

	// 認証情報の一時保存.
//	PnlComGetUserAuthentication()->setAuthCacheMode( PAdminToUserAndDepartSetcom::m_b_AuthCacheMode );
	PnlComGetUserAuthentication()->setAuthCacheMode( PAdminToUserAndDepartSetcom::m_b_AuthCacheMode, e_NetworkSpecific );
}
//================================================================================================
// 概要		:管理者設定・ユーザ認証/部門管理・ユーザ/部門の連動認証(2691) : ＯＫキー
// 戻り値	:なし
//================================================================================================
void PAdminUserAndDepartSynchro::procOk( uchar option )
{
	onlyPanel.setSelectServerIndex( APID_AuthenticationServerIndexNone );
	if( bMiddleServerWarning || bOutsideServerWarning ) {
		// 警告画面の閉じるキー.
		bMiddleServerWarning = False;
		bOutsideServerWarning = False;
		setupGamen();
		return ;
	}
	// 警告画面表示チェック.
	bOutsideServerWarning = CheckOutsideServerWarning();
	if( bOutsideServerWarning ) {
		setupGamen();
		return;
	}
// comment 2014/11/25 OPS1-Sky工藤 [ZeusS][UV-01446].
//	//comment 2014/09/15 HCLT_Utsav [ZeusS][UV-01446] Adding NFC option . 
//	if( True == PLBC_FuncVersionCheck::isOK_IT5_Type1()){ 
//		if( NULL!= PnlComGetNFCSetting() ){ 
//			// To set the NFC authentication setting. 
//			 PnlComGetNFCSetting()->setAuthNFC(m_e_NFCAuthenticationSettingEnable); 
//		} 
//	} 


//	if ( ss_UserCounter < 1 && b_Manage == True && sc_UserAttest != 0 ) {	// ユーザ数が1より小さく、ユーザ認証、部門管理を行うとき入力範囲を確認する.
//		b_isError = True;			// 範囲オーバー .
//		setupGamen();.
//	if( True == PLBC_FuncVersionCheck::isOK_MiZe_Type2() && 0 != sc_UserAttest && ( 1 > ss_UserCounter || ( 1 > m_ss_ExtCounter && 5 == sc_UserAttest ) ) ){
	if( ( 5 == sc_UserAttest ) && ( ( 1 > ss_UserCounter ) || ( 1 > m_ss_ExtCounter ) ) ) {		// "拡張サーバー+本体"の場合、カウンターの値が不正かチェックする.
		b_isError = True;			// 範囲オーバー .
		setupGamen();
	}
//Comment 2017/12/29 [HCL-Harish] [IT5 4.2][AR1032952] not setting error flag in case of Making NW1 to Authenticate + Account Track when NW2 having Authentication.
//	else if( ss_UserCounter < 1 && b_Manage == True && sc_UserAttest != 0 ) {
	else if( ss_UserCounter < 1 && b_Manage == True && sc_UserAttest != 0 
			&& TYPE_UserAuthenticationOff == PnlComGetUserAuthentication()->getUserAuthentication( TYPE_NetworkSpecific_NW2 ) ) {
		b_isError = True;			// 範囲オーバー .
		setupGamen();
	} else {
		Bool b_Clear = False;	// データのクリアをするかしないか。True：する、False：しない.
		if( sc_UserAttest_At != 0 && sc_UserAttest == 0 ) 		// ユーザ認証がする → しないのとき.
		{ b_Clear = True; }
		else if( sc_UserAttest_At == 0 && sc_UserAttest != 0 )	// ユーザ認証がしない → するのとき.
		{ b_Clear = True; }
		else if( b_Manage_At == True && b_Manage == False )		// 部門管理する → しないとき
		{
			b_Clear = True;

		}
		else if( b_Manage_At == False && b_Manage == True )		// 部門管理しない → するとき
		{
			b_Clear = True;

		}
		else if( ss_departForm_At == 0 && ss_departForm == 1 )	// 部門管理認証方式変更時.
		{ b_Clear = True; }
		else if( ss_departForm_At == 1 && ss_departForm == 0 )	// 部門管理認証方式変更時.
		{ b_Clear = True; }
		else if( ss_UserCounter_At != ss_UserCounter )	// ユーザ数変更時.
		{ b_Clear = True; }
		else if( True == PLBC_FuncVersionCheck::isOK_MiZe_Type2() && m_ss_ExtCounter_At != m_ss_ExtCounter )	// 拡張サーバーカウンター変更時.
		{ b_Clear = True; }
		Bool b_AuthOap = False;	// 中間認証設定を解除するかしないか。True：する、False：しない.
		if( ( ( 3 == sc_UserAttest_At ) && ( ( 3 != sc_UserAttest ) && ( 5 != sc_UserAttest ) ) ) ||		// 拡張サーバー認証から拡張サーバーor本体＋拡張サーバー以外に変更.
		    ( ( 5 == sc_UserAttest_At ) && ( ( 3 != sc_UserAttest ) && ( 5 != sc_UserAttest ) ) ) ) {		// 本体＋拡張サーバー認証から拡張サーバーor本体＋拡張サーバー以外に変更.
			// PKI機能が有効の場合、拡張サーバー認証設定を解除しない.
			if( True != PLBC_FuncPKICheck::isPKI() ) { b_AuthOap = True; }
		}
		else if( ( ( 3 == sc_UserAttest_At ) && ( 5 == sc_UserAttest ) ) ||	// 拡張サーバー認証⇒本体＋拡張サーバー認証.
		         ( ( 5 == sc_UserAttest_At ) && ( 3 == sc_UserAttest ) ) ) {	// 本体＋拡張サーバー認証⇒拡張サーバー認証.
			// 拡張サーバー認証設定は解除せず、使用管理データクリア画面を表示する.
			b_Clear = True;
		}
// comment 2016/11/14 PDD22 出口(日立) [IT5_4.0][AccuRev.1006928]【RQ-531】本体認証データ登録拡張：拡張ユーザーDBの削除対応.
// 拡張ユーザーDB削除条件と拡張ユーザーDB設定値クリア条件をチェックする.
		b_LScaleClear = False;														// 拡張ユーザーDBを削除するかしないか。True：する、False：しない.
		b_LScaleSettingClear = False;												// 拡張ユーザーDB設定クリア(False:クリアしない).
		if ( True == PnlComGetUser()->isEnableDataRegistrationExtension() ) {
			if ( ( ( ( 1 != sc_UserAttest_At ) && ( 2 != sc_UserAttest_At ) && ( 4 != sc_UserAttest_At ) )
				&& ( ( 2 == sc_UserAttest ) || ( 4 == sc_UserAttest ) ) )			// 本体認証、本体+外部サーバー認証に変更された場合.
				|| ( b_AuthInsideLargeScale_At != b_AuthInsideLargeScale ) ) {		// 拡張ユーザーDB設定が変更された.
				b_LScaleClear = True;												// 拡張DBを削除する.
			}
			if ( ( ( 2 == sc_UserAttest_At ) || ( 4 == sc_UserAttest_At ) )
				&& ( ( 2 != sc_UserAttest ) && ( 4 != sc_UserAttest ) )				// 本体認証又は本体+外部サーバー認証以外に変更された場合.
				&& ( True == b_AuthInsideLargeScale_At ) ) {						// 拡張ユーザーDB設定が「使用する」だった場合.
				b_LScaleClear = True;												// 拡張DBを削除する.
				b_LScaleSettingClear = True;										// 拡張ユーザーDB設定をクリアする.
			}
			if ( b_Manage_At != b_Manage ) {										// 部門管理変更.
				b_LScaleClear = True;												// 拡張DBを削除する.
			}
		}
		Bool b_ErieManage = False;	// 権限管理/カウンター管理を解除するかしないか。 True:する、False:しない.
		if( ( b_AuthorityManagement == True )						// 権限管理/カウンター管理｢管理する｣.
		    && ( sc_UserAttest_At == 1 && sc_UserAttest != 1 ) ) {		// 外部サーバ → 他の認証方式に変更.
			b_ErieManage = True;
// comment 2017/10/31 【HCL-TANU】【IT5_4.2】【RQ686】【IT Defect AR1030817].
//		} else if( PnlComGetUserAuthentication()->getMidauthControl() == True && b_AuthorityManagement == False ) {	// 権限管理/カウンター管理が管理する → 管理しない.
	    } else if( PnlComGetUserAuthentication()->getMidauthControl(PNLC_NetworkLan::getInstance() ->getAdminNWLine( )) == True && b_AuthorityManagement == False ) {	// 権限管理/カウンター管理が管理する → 管理しない.
			b_ErieManage = True;
		}
//DonauLastFum【Erie拡張】【13笹岡】20090518 end.

// comment 2015/03/19 OPS1-MSE竹中 [MinervaSBK][RQ-278] LDAPサーバーへのカード情報登録機能対応.
		// 認証カード検索機能Offの場合に、ユーザーの意図に反したデータクリア確認画面表示をさせないよう、.
		// Onの場合にのみ、その画面配下の設定変更判定を行い、クリアフラグを立てるように変更する.
		if ( True == b_AuthCardServerSearching ) {
			// 認証カード検索機能Onの場合.

// comment 2015/06/01 OPS1-MSE竹中 [MinervaSBK][CQ513861] 管理データクリア画面の遷移条件からカード情報登録設定変更を外す.
//			if ( ( False == m_bl_CardRegistration_At )
//				&& ( True == m_bl_CardRegistration ) ) {
//				// カード情報登録設定がOffからOnに変更された場合、使用管理データクリア画面を表示.
//				b_Clear = True;
//			}

			if( b_UserOn_At != s_AuthCardParams.b_UserOn ) {				// ﾕｰｻﾞ名の設定値が変更された時.
				b_Clear = True;
			}
			if( 0 != strcmp( sc_UserCommand_At, s_AuthCardParams.sc_UserCommand ) ) {	// ユーザ名とする属性が変更された時.
				b_Clear = True;
			}
		}
//Comment 2017/10/07 [HCL-Harish] [IT5 4.2][RQ-686] [WP744] Multiple Network support- Data Clear when VLAN Setting.
		Bool b_Vlane = False;	// データのクリアをするかしないか。True：する、False：しない.
		if( True == PLBC_FuncVersionCheck::isOK_IT5_Type11() ) {
// Comment 2018/01/29 HCLT_Hanuman [IT5_4.2][AR1038061] To improve performance.
//			if (True == APIC_VlanSetting::newInstance()->getVlanEnable()) {
			if ( True ==  PNLC_NetworkLan::getInstance()->getVlanEnableWrapper() ) {
				if ( ( ( 1 == sc_UserAttest_At ) && ( 1 != sc_UserAttest ) ) || ( ( 2 == sc_UserAttest_At ) && ( 2 != sc_UserAttest ) )
					|| ( ( 3 == sc_UserAttest_At ) && ( 3 != sc_UserAttest ) ) || ( ( 4 == sc_UserAttest_At ) && ( 4 != sc_UserAttest ) )
					|| ( ( 5 == sc_UserAttest_At ) && ( 5 != sc_UserAttest ) ) ) {
					b_Vlane = True;
				}
			}
		}
		// 制限機能使用時のログイン設定の設定.
		if( True == b_DoRequest ) {
//Comment 2017/10/11 [HCL-Swathi] [IT5 4.2] [AR1030574] Multiple Network support.
//			PnlComGetSystemSetting()->setLimitationFunctionUsingLogin( TYPE_LimitationFunctionUsingLoginOn );		// 要求する.
			PnlComGetSystemSetting()->setLimitationFunctionUsingLogin( TYPE_LimitationFunctionUsingLoginOn, PNLC_NetworkLan::getInstance()->getAdminNWLine() );		// 要求する.
		} else {
//Comment 2017/10/11 [HCL-Swathi] [IT5 4.2] [AR1030574] Multiple Network support.
//			PnlComGetSystemSetting()->setLimitationFunctionUsingLogin( TYPE_LimitationFunctionUsingLoginOff );		// 要求しない.
			PnlComGetSystemSetting()->setLimitationFunctionUsingLogin( TYPE_LimitationFunctionUsingLoginOff, PNLC_NetworkLan::getInstance()->getAdminNWLine() );		// 要求しない.
		}
		if( NULL != PnlComGetSystemSetting() ) {
			TYPE_FreeJobSetting e_ZeroBillingJobEnablebuf = PnlComGetSystemSetting()->getEnableZeroBillingJob();
			if( e_ZeroBillingJobEnablebuf != m_e_ZeroBillingJobEnable ) {
				PnlComGetSystemSetting()->setEnableZeroBillingJob( m_e_ZeroBillingJobEnable );
				if( m_e_ZeroBillingJobEnable == TYPE_FreeJobSettingEnable ) {
					PNLE_IsFaxMode e_isFaxmode = PNLE_IsFaxMode_NON;
					onlyPanel.isFaxModekind( &e_isFaxmode );
					if( ( PNLE_IsFaxMode_NON != e_isFaxmode ) && ( APIC_ExtSetting::getNotifyType() == TYPE_ExtNotifyTypeCreditEachJob ) ) {
						if( ( PnlComIsUseSsdScanSend() == True ) && ( False == bl_ImageLogOverError ) ) {
							PnlComGetSystemSetting()->setOffHookQuickSendType( True );			//Enable Off Hook Fax.
						}
					}
				}
			}
		}
		// 上限値到達時動作.
		if( sc_LimitArrive == 0 )
// Comment 2017/11/22【HCLT-Tanu】【IT5 4.2】【RQ686】API Correction.		
//		{ PnlComGetCopyJobSetting()->setStopModeByTrackCntLimit( TYPE_StopModeByTrackCntLimitStopEnableNextJob ); }//	ジョブ飛び越し.
		{ PnlComGetCopyJobSetting()->setStopModeByTrackCntLimit( TYPE_StopModeByTrackCntLimitStopEnableNextJob,PNLC_NetworkLan::getInstance()->getAdminNWLine() ); }//	ジョブ飛び越し.
		else if( sc_LimitArrive == 1 )
// Comment 2017/11/22【HCLT-Tanu】【IT5 4.2】【RQ686】API Correction.		
//		{ PnlComGetCopyJobSetting()->setStopModeByTrackCntLimit( TYPE_StopModeByTrackCntLimitStopDisableNextJob ); }//	ジョブ停止.
		{ PnlComGetCopyJobSetting()->setStopModeByTrackCntLimit( TYPE_StopModeByTrackCntLimitStopDisableNextJob,PNLC_NetworkLan::getInstance()->getAdminNWLine() ); }//	ジョブ停止.
		else if( 2 == sc_LimitArrive )
// Comment 2017/11/22【HCLT-Tanu】【IT5 4.2】【RQ686】API Correction.		
//		{ PnlComGetCopyJobSetting()->setStopModeByTrackCntLimit( TYPE_StopModeByTrackCntLimitJobDelete ); }			// ジョブ削除.
		{ PnlComGetCopyJobSetting()->setStopModeByTrackCntLimit( TYPE_StopModeByTrackCntLimitJobDelete,PNLC_NetworkLan::getInstance()->getAdminNWLine() ); }			// ジョブ削除.
		// セキュリティー強化設定が有効かつパブリックユーザーが「許可する」の場合.
		// セキュリティー強化設定が有効かつPKI認証設定時でないかつユーザー認証が「本体認証」「外部サーバー認証」「本体＋外部サーバー」以外の場合.
		if( ( True == PnlComGetSystemSetting()->getSecurityLevelUpOn() )
		    && ( ( 0 != ss_PublicUser )
// comment 2015/2/12 OPS1-Sky工藤 [ZeusS][CQ509844].
//		   || ( ( True != PLBC_FuncPKICheck::isPKI() ) && ( 1 != sc_UserAttest ) && ( 2 != sc_UserAttest ) && ( 4 != sc_UserAttest ) ) ) ) {
		   || ( ( True != PLBC_FuncPKICheck::isPKI() ) && ( 1 != sc_UserAttest ) && ( 2 != sc_UserAttest ) && ( 4 != sc_UserAttest ) )
// comment 2017/02/27 SCD2-Sky延原 [IT5_4.0][AR1005533].
//		   || ( ( True == PLBC_FuncVersionCheck::isOK_IT5_Type1() ) && ( ( TYPE_NFCUse_ON == onlyPanel.getNFCUse() ) || ( TYPE_BLEUse_ON == onlyPanel.getBLEUse() ) ) ) ) ) {
		   || ( ( True == PLBC_FuncVersionCheck::isOK_IT5_Type1() ) && ( ( TYPE_NFCUse_ON == m_e_NFCUse ) || ( TYPE_BLEUse_ON == m_e_BLEUse ) ) ) ) ) {
			// データクリアなし.
			if( b_Clear == False ) {
				setDispInfo( PNLE_AdminUserAndDepartSynchro, this );		// セキュリティーモード解除確認画面で使用する為.
			} else {
				setDispInfo( PNLE_AdminUserAndDepartSynchroClear, this );	// セキュリティーモード解除確認画面で使用する為.
			}
			nextGamen = &gp_AdminSecurityReinforceConfirm;								// セキュリティーモード解除確認
//Comment 2017/10/07 [HCL-Harish] [IT5 4.2][RQ-686] [WP744] Multiple Network support- Data Clear when VLAN Setting.
//		} else if( b_Clear == False && b_AuthOap == False && b_ErieManage == False ) {	// データのクリアが必要ないので、設定値を保存して元の画面へ遷移.	//DonauLastFum【Erie拡張】【13笹岡】20090518 add.
		} else if( b_Clear == False && b_AuthOap == False && b_ErieManage == False && b_Vlane == False) {	// データのクリアが必要ないので、設定値を保存して元の画面へ遷移.	//DonauLastFum【Erie拡張】【13笹岡】20090518 add.
			if( ( True == PLBC_FuncPKICheck::isPKI() )
			    && ( b_MiddleServerSetting_At != b_MiddleServerSetting ) ) {
				nextGamen = &gp_AdminUserAndDepartDataClear;		// 管理者設定・ユーザ認証/部門管理・ユーザ/部門の連動認証・使用管理ﾃﾞｰﾀｸﾘｱ.
			} else if( ( ( True == b_AuthInsideLargeScale ) &&																// 拡張ユーザーDBが使用するの場合.
						( ( ( 2 == sc_UserAttest_At ) || ( 4 == sc_UserAttest_At ) ) && ( 1 == sc_UserAttest ) ) ) ||		// 本体認証又は本体+外部サーバー認証から、外部サーバー認証に変更.
					   ( ( False == b_AuthInsideLargeScale ) && ( True == b_AuthInsideLargeScale_At ) ) ){					//拡張ユーザーDB設定を「使用しない」に変更.
				nextGamen = &gp_AdminUserAndDepartLargeScaleUserDataClear;		// 拡張ユーザーデータの削除確認.
			} else {
				setAuthenticationData();	// 設定値の保存処理.
// comment 2016/11/14 PDD22 出口(日立) [IT5_4.0][AccuRev.1006928]【RQ-531】本体認証データ登録拡張：拡張ユーザーDBの削除対応.
				if ( True == b_LScaleClear ) {
					// 拡張ユーザーDBを削除する場合.
					APIC_User::req_deleteLargerScaleDatabase( ans_deleteLargerScaleDatabase );
					b_LScaleClear = False;
				}
				PAdminToUserAndDepartSetcom::procOk( option );		// 管理者設定・ユーザ認証/部門管理メニュー
			}
		} else if( b_AuthOap == True ) {
// comment 2016/11/14 PDD22 出口(日立) [IT5_4.0][AccuRev.1006928]【RQ-531】本体認証データ登録拡張：拡張ユーザーDBの削除対応.
			if ( True == b_LScaleClear ) {
				// 拡張ユーザーDBを削除する場合.
				APIC_User::req_deleteLargerScaleDatabase( ans_deleteLargerScaleDatabase );
				b_LScaleClear = False;
			}
			nextGamen = &gp_AdminAuthOapClear;
		} else if( b_ErieManage == True ) {						// 権限管理/カウンター管理を解除.
			nextGamen = &gp_AdminAuthorityManagementClear;		// 「50925190_権限管理／カウンター管理設定解除」画面へ.
		} else	// データのクリアが必要なので、クリア画面へ遷移
		{ nextGamen = &gp_AdminUserAndDepartDataClear; }		// 管理者設定・ユーザ認証/部門管理・ユーザ/部門の連動認証・使用管理ﾃﾞｰﾀｸﾘｱ.
	}
//	comment 2015/11/02 HCL_Ezhil [ZeusS_ZX0][制御構成変更_No.76-1] Adding error code.
//Comment 2019/06/14 [HCLT_Monisha][EagleH][N Rank] Removed unwanted Detailed code.
//	LAPC_BFW_System *pc_BFW_System=LAPC_BFW_System::newInstance();
//Comment 2017/09/04 [HCL-Harish] [IT5 4.2][RQ-686] [WP509] Multiple Network support.
//	if( TYPE_LimitationFunctionUsingLoginOn == PnlComGetSystemSetting()->getLimitationFunctionUsingLogin() ) {
	TYPE_NetworkSpecific e_NetworkSpecific  = PNLC_NetworkLan::getInstance()->getAdminNWLine( ) ;
	if( TYPE_LimitationFunctionUsingLoginOn == PnlComGetSystemSetting()->getLimitationFunctionUsingLogin( e_NetworkSpecific ) ) {
		char sc_ActiveAppIndex = PAdminNetworkIWSSet::getActiveAppIndex( TYPE_IwsAppTypeAuth );
		if( TYPE_InvalidIndex != sc_ActiveAppIndex ) { // Check whether active Authentication Screen application is available.
			char sc_AppID[TYPD_IwsMaxID+1];
			memset( sc_AppID,'\0',sizeof( sc_AppID ) );
			APIC_IWSAppInfo* pc_IWSAppInfo = APIC_IWSAppInfo::newInstance();
			if( NULL == pc_IWSAppInfo ) {
//	comment 2015/11/02 HCL_Ezhil [ZeusS_ZX0][制御構成変更_No.76-1] Adding error code.
//Comment 2019/06/14 [HCLT_Monisha][EagleH][N Rank] Removed unwanted Detailed code.
//				if (NULL !=pc_BFW_System){
//				    pc_BFW_System->registDetailCode(PNLD_DetailCode_Admin_0x1900030010414001);
//				}
				LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
				return;
			}
			if( NULL == ppc_IwsAppInfo ) {
//	comment 2015/11/02 HCL_Ezhil [ZeusS_ZX0][制御構成変更_No.76-1] Adding error code.
//Comment 2019/06/14 [HCLT_Monisha][EagleH][N Rank] Removed unwanted Detailed code.
//				if (NULL !=pc_BFW_System){
//				    pc_BFW_System->registDetailCode(PNLD_DetailCode_Admin_0x1900030010414101);
//				}
				LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
				return;
			}
			ppc_IwsAppInfo->getActiveApplication( sc_AppID , TYPE_IwsAppTypeAuth );
			pc_IWSAppInfo->setApplicationState( sc_AppID , TYPE_IwsAppInActive );
		}
	}
// comment 2014/11/25 OPS1-Sky工藤 [ZeusS][UV-01446].
//// comment 2014/09/12 OPS1嵯峨山 [ZeusS] RQ-223 モバイル連携 BluetoothLE対応.
//
//	if ( True == PLBC_FuncVersionCheck::isOK_IT5_Type1() ) { 
//		APIC_BLESetting* pc_BLESetting = APIC_BLESetting::newInstance();
//		if ( NULL != pc_BLESetting ) {
//			pc_BLESetting->setAuthBLE( m_e_BLEAuth );
//		}
//	}

}
//================================================================================================
// 概要		:管理者設定・ユーザ認証/部門管理・ユーザ/部門の連動認証(2691) : 戻るキー
// 戻り値	:なし
//================================================================================================
void PAdminUserAndDepartSynchro::procCancel( uchar option )
{
	PAdminToUserAndDepartSetcom::procOk( option );
}
//================================================================================================
// 概要		:管理者設定・ユーザ認証/部門管理・ユーザ/部門の連動認証(2691) : 画面開始処理
// 戻り値	:なし
//================================================================================================
void PAdminUserAndDepartSynchro::startGamen()
{
	PAdminToUserAndDepartSetcom::startGamen();
//Comment 2017/09/04 [HCL-Harish] [IT5 4.2][RQ-686] [WP509] Multiple Network support.
	TYPE_NetworkSpecific e_NetworkSpecific  = PNLC_NetworkLan::getInstance()->getAdminNWLine( ) ;
	if( gp_AdminUserAndDepartSet.b_fromMenu == True ) {	// 下層画面以外から遷移してきた場合.
		gp_AdminUserAndDepartSet.b_fromMenu = False;	// 上からの遷移状態を解除.
// Comment 2018/01/29 HCLT_Hanuman [IT5_4.2][AR1038061] To improve performance.
//// Comment 2018/01/18 HCLT_Binesh [IT5_4.2][AR1034977] Initializing the NW2 user count library when coming through NW1 admin line.
//		if (True == APIC_VlanSetting::newInstance()->getVlanEnable()) {
		if (True == PNLC_NetworkLan::getInstance()->getVlanEnableWrapper() ) {
			if ( TYPE_NetworkSpecific_NW1 == PNLC_NetworkLan::getInstance() ->getAdminNWLine() ) {
				PNLC_NetworkLan::getInstance()->setNW2UserCntNoLib(0);
			}	
		}
//		switch( PnlComGetUserAuthentication()->getUserAuthentication() ) {
		switch( PnlComGetUserAuthentication()->getUserAuthentication( e_NetworkSpecific ) ) {
			case TYPE_UserAuthenticationOff:					sc_UserAttest = 0;		break;	// OFF
			case TYPE_UserAuthenticationServer:					sc_UserAttest = 1;		break;	// 外部サーバーでの認証
			case TYPE_UserAuthenticationDevice:					sc_UserAttest = 2;		break;	// 本体装置での認証
			case TYPE_UserAuthenticationIntermeddiateServer:	sc_UserAttest = 3;		break;	// 中間サーバーでの認証
			case TYPE_UserAuthenticationDeviceAndServer:		sc_UserAttest = 4;		break;	// 本体+外部サーバーでの認証
			case TYPE_UserAuthenticationDeviceAndMidAuth:		sc_UserAttest = 5;		break;	// 本体+拡張サーバーでの認証.
			default: break;
		}
//		switch( PnlComGetUserAuthentication()->getCertificationAction( ) ) {
		switch( PnlComGetUserAuthentication()->getCertificationAction( e_NetworkSpecific ) ) {
			case TYPE_CertificationActionOn:			ss_PublicUser = 0;		break;	// ユーザー認証管理を行う
			case TYPE_CertificationActionUserAndPublic:	ss_PublicUser = 1;		break;	// ユーザー認証管理/PUBLICユーザー併用
			case TYPE_CertificationActionOnlyPublic:	ss_PublicUser = -1;		break;	// ユーザー認証なし(PUBLICユーザーのみ)
			default: break;
		}
		//Publicでユーザ認証なしで使用できる設定かチェック
//		if( ( True == PnlComGetUserAuthentication()->getEnablePublicUserNoAuthentication() )
		if( ( True == PnlComGetUserAuthentication()->getEnablePublicUserNoAuthentication( e_NetworkSpecific ) )
		    &&( ss_PublicUser == 1 ) ) {
			//Public認証許可後、ログイン設定有効
			ss_PublicUser = 2;
		}
		if( sc_UserAttest == 0 )
		{ ss_PublicUser = -1; }		// ユーザ認証しないときは未選択状態にする.

		// 制限機能使用時のログイン設定の取得.
//		if( TYPE_LimitationFunctionUsingLoginOn == PnlComGetSystemSetting()->getLimitationFunctionUsingLogin() ) {
		if( TYPE_LimitationFunctionUsingLoginOn == PnlComGetSystemSetting()->getLimitationFunctionUsingLogin( e_NetworkSpecific ) ) {
			b_DoRequest = True;		// 要求する.
		} else {
			b_DoRequest = False;	// 要求しない.
		}
//		switch( PnlComGetTrackManager()->getTrackMode() ) {
		switch( PnlComGetTrackManager()->getTrackMode( e_NetworkSpecific ) ) {
			case TYPE_TrackModeOnWithUserAuthentication:	//	ユーザー認証と合わせて部門管理する
				b_Synchro = False;		// 連動する
				pdpc_UserAndTrackMode.setTrackModeLink( PNLE_TrackModeLink_Link );		// 連動する.
				ss_departForm = 0;		// 部門名＋パスワード
				b_Manage = True;		// 部門管理する
				break;
			case TYPE_TrackModeOff:							//	認証しない
				b_Synchro = True;		// 連動しない
				pdpc_UserAndTrackMode.setTrackModeLink( PNLE_TrackModeLink_UnLink );	// 連動しない.
				ss_departForm = -1;		// 認証方式未選択
				b_Manage = False;		// 部門管理しない
				break;
			case TYPE_TrackModeOnlyPassword:				//	パスワードのみ
				if( True == PLBC_FuncPKICheck::isPKI() ) { b_Synchro = False; }	// 連動する.
				else { b_Synchro = True; }										// 連動しない.
				pdpc_UserAndTrackMode.setTrackModeLink( PNLE_TrackModeLink_UnLink );	// 連動しない.
				ss_departForm = 1;		// パスワードのみ
				b_Manage = True;		// 部門管理する
				break;
			case TYPE_TrackModeNoAndPassword:				//	部門名＋パスワード
				if( True == PLBC_FuncPKICheck::isPKI() ) { b_Synchro = False; }	// 連動する.
				else { b_Synchro = True; }										// 連動しない.
				pdpc_UserAndTrackMode.setTrackModeLink( PNLE_TrackModeLink_UnLink );	// 連動しない.
				ss_departForm = 0;		// 部門名＋パスワード
				b_Manage = True;		// 部門管理する
				break;
			case TYPE_TrackModeUserSelect:					// ユーザー設定を使用する.
				b_Synchro = False;		// 連動する.
				pdpc_UserAndTrackMode.setTrackModeLink( PNLE_TrackModeLink_UserSelect );		// ユーザーが設定.
				ss_departForm = 0;		// 部門名＋パスワード.
				b_Manage = True;		// 部門管理する.
				break;
			default: break;
		}
// Comment 2017/11/22【HCLT-Tanu】【IT5 4.2】【RQ686】API Correction.	
//		if( PnlComGetCopyJobSetting()->getStopModeByTrackCntLimit() == TYPE_StopModeByTrackCntLimitStopEnableNextJob )		//	ジョブ飛び越し.
	    if( PnlComGetCopyJobSetting()->getStopModeByTrackCntLimit(PNLC_NetworkLan::getInstance()->getAdminNWLine()) == TYPE_StopModeByTrackCntLimitStopEnableNextJob )		//	ジョブ飛び越し.
		{ sc_LimitArrive = 0; }
//		else if( PnlComGetCopyJobSetting()->getStopModeByTrackCntLimit() == TYPE_StopModeByTrackCntLimitStopDisableNextJob )	//	ジョブ停止.
	    else if( PnlComGetCopyJobSetting()->getStopModeByTrackCntLimit(PNLC_NetworkLan::getInstance()->getAdminNWLine()) == TYPE_StopModeByTrackCntLimitStopDisableNextJob )	//	ジョブ停止.
		{ sc_LimitArrive = 1; }
//		else if( TYPE_StopModeByTrackCntLimitJobDelete == PnlComGetCopyJobSetting()->getStopModeByTrackCntLimit() )			// ジョブ削除.
	    else if( TYPE_StopModeByTrackCntLimitJobDelete == PnlComGetCopyJobSetting()->getStopModeByTrackCntLimit(PNLC_NetworkLan::getInstance()->getAdminNWLine()) )			// ジョブ削除.
		{ sc_LimitArrive = 2; }
		if( True == PLBC_FuncVersionCheck::isOK_MiZe_Type2() ) {
			if( 5 == sc_UserAttest && True == b_Manage ) {
				// 認証方式「本体+拡張サーバー」、部門管理「する」.
//Comment 2017/09/29 [HCL-Harish] [IT5 4.2][RQ-686] [WP533/545] Multiple Network support - User/Track.				
//				ss_UserCounter = PnlComGetTrackManager()->getMaxTrack() - PnlComGetTrackManager()->getTrackCntNo() - PnlComGetTrackManager()->getCacheCntNo();	// ユーザカウンタ数を所得.
				ss_UserCounter = GetMaxTrackCounter() - PnlComGetTrackManager()->getTrackCntNo() - PnlComGetTrackManager()->getCacheCntNo();	// ユーザカウンタ数を所得.
				m_ss_ExtCounter = PnlComGetTrackManager()->getCacheCntNo();
			}
			else if( 5 == sc_UserAttest ) {
				// 認証方式「本体+拡張サーバー」、部門管理「しない」.
				// 「本体+拡張サーバー認証」の場合は、部門カウンターの設定は無いが、
				// ユーザーカウンター+拡張カウンターが合計1000未満になるケースが仕様として存在する為、
				// 計算上必要となっている。
//Comment 2017/09/29 [HCL-Harish] [IT5 4.2][RQ-686] [WP533/545] Multiple Network support - User/Track.				
//				ss_UserCounter = PnlComGetTrackManager()->getMaxTrack() - PnlComGetTrackManager()->getTrackCntNo() - PnlComGetTrackManager()->getCacheCntNo();	// ユーザカウンタ数を所得.
				ss_UserCounter = GetMaxTrackCounter() - PnlComGetTrackManager()->getTrackCntNo() - PnlComGetTrackManager()->getCacheCntNo();	// ユーザカウンタ数を所得.
				m_ss_ExtCounter = PnlComGetTrackManager()->getCacheCntNo();
			}
			else {
//Comment 2017/09/29 [HCL-Harish] [IT5 4.2][RQ-686] [WP533/545] Multiple Network support - User/Track.				
//				ss_UserCounter = PnlComGetTrackManager()->getMaxTrack() - PnlComGetTrackManager()->getTrackCntNo();	// ユーザカウンタ数を所得.
				ss_UserCounter = GetMaxTrackCounter() - PnlComGetTrackManager()->getTrackCntNo();	// ユーザカウンタ数を所得.
			}
		}
		else {
//Comment 2017/09/29 [HCL-Harish] [IT5 4.2][RQ-686] [WP533/545] Multiple Network support - User/Track.			
//			ss_UserCounter = PnlComGetTrackManager()->getMaxTrack() - PnlComGetTrackManager()->getTrackCntNo();	// ユーザカウンタ数を所得.
			ss_UserCounter = GetMaxTrackCounter() - PnlComGetTrackManager()->getTrackCntNo();	// ユーザカウンタ数を所得.
		}
		if( ppc_SystemInfo->getExternalIFMode() == TYPE_ExternalIFMode1 ) {
			b_isEnableFieryPullScan = PnlComGetSystemSetting()->getEnableFieryPullScan();
		}
		sc_UserAttest_At = sc_UserAttest;	// Start時の設定値を保存しておく.
		b_Manage_At = b_Manage;
		ss_departForm_At = ss_departForm;
		ss_UserCounter_At = ss_UserCounter;
		if( True == PLBC_FuncVersionCheck::isOK_MiZe_Type2() ) {
			m_ss_ExtCounter_At = m_ss_ExtCounter;
		}
		b_Direct = False;		// 直接入力・Off.
		b_Fixed = True;			// 入力確定.
		b_isError = False;		// 範囲オーバーかの結果を格納
		sc_NowPage = 1;			// 1ページ目を表示.
		sc_KeySelect = -1;				// 左画面キー選択.
		ss_ConnectTime = PnlComGetUserAuthenticationSetting()->getActiveDirectoryTicketLife();
		APIC_AuthenticationSeverSetting* apicAuthSrv = APIC_AuthenticationSeverSetting::newInstance();
//	comment 2015/11/02 HCL_Ezhil [ZeusS_ZX0][制御構成変更_No.76-1] Adding error code.
//		if( NULL == apicAuthSrv ) {LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }
		if( NULL == apicAuthSrv ) {
//Comment 2019/06/14 [HCLT_Monisha][EagleH][N Rank] Removed unwanted Detailed code.
//			LAPC_BFW_System *pc_BFW_System=LAPC_BFW_System::newInstance();
//			if (NULL !=pc_BFW_System){
//			    pc_BFW_System->registDetailCode(PNLD_DetailCode_Admin_0x1900650010414201);
//			}
			LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" ); 
			return; 
		}
		// 認証検索機能On/Offを取得.
//		b_AuthCardServerSearching = apicAuthSrv->getAuthCardSearch();
		b_AuthCardServerSearching = apicAuthSrv->getAuthCardSearch( e_NetworkSpecific );

		if( ( False == apicAuthSrv->checkServerGroup() ) 
			|| ( False == PnlComGetAuthDeviceCommon()->checkIccChangeAuthServer() )
// comment 2017/10/31 【HCL-TANU】【IT5_4.2】【RQ686】【IT Defect AR1030817].
//			|| ( True == PnlComGetUserAuthentication()->getMidauthControl() ) ) {
	        || ( True == PnlComGetUserAuthentication()->getMidauthControl(PNLC_NetworkLan::getInstance() ->getAdminNWLine( )) ) ) {
			// 外部サーバーグループ設定が無効な場合.
			
			// 連携する外部サーバのIndex番号取得.
			uc_AuthCardServerIndex = apicAuthSrv->getAuthCardServerIndex();

			// 認証サーバー情報を取得.
			APIS_AuthenticationServerInfo server_info; // サーバー情報.
//			apicAuthSrv->getServerInfo( APID_AuthenticationSeverTypeIccLDAP, &server_info );
			apicAuthSrv->getServerInfo( APID_AuthenticationSeverTypeIccLDAP, &server_info, e_NetworkSpecific );
			s_AuthCardParams = server_info.s_IccLDAP;

			// 代替サーバー情報を取得.
			APIS_AuthenticationServerInfo secondaryServer_info; // サーバー情報.
//			apicAuthSrv->getServerInfo( APID_AuthenticationSeverTypeIccLDAPSecond, &secondaryServer_info );
			apicAuthSrv->getServerInfo( APID_AuthenticationSeverTypeIccLDAPSecond, &secondaryServer_info, e_NetworkSpecific );
			m_s_AuthCardSecondaryServerParams = secondaryServer_info.s_IccLDAP;

// comment 2015/03/19 OPS1-MSE竹中 [MinervaSBK][RQ-278] LDAPサーバーへのカード情報登録機能対応.
			// カード情報登録設定On/Offを取得.
			m_bl_CardRegistration = apicAuthSrv->getIccLdapCardWriteEnable();
		}
		
		// サーバーグループ有効時に、認証サーバ切り替えが未対応又は拡張サーバーが有効の場合.
		if( ( True == apicAuthSrv->checkServerGroup() ) 
			&& ( ( False == PnlComGetAuthDeviceCommon()->checkIccChangeAuthServer() )
// comment 2017/10/31 【HCL-TANU】【IT5_4.2】【RQ686】【IT Defect AR1030817].
//				|| ( True == PnlComGetUserAuthentication()->getMidauthControl() ) ) ) {
	            || ( True == PnlComGetUserAuthentication()->getMidauthControl(PNLC_NetworkLan::getInstance() ->getAdminNWLine( )) ) ) ) {
			m_e_CardAttr =apicAuthSrv->getCardSearchLetters();
		}
		if( NULL!= PnlComGetUserAuthentication() ) {
//			m_e_UserAuthZeroBillingJob = PnlComGetUserAuthentication()->getUserAuthentication();
			m_e_UserAuthZeroBillingJob = PnlComGetUserAuthentication()->getUserAuthentication( e_NetworkSpecific );
		}
		if( NULL!= PnlComGetSystemSetting() ) {
			m_e_ZeroBillingJobEnable = PnlComGetSystemSetting()->getEnableZeroBillingJob();
		}
//【CQ:500916】【UV-01276】【管理No.267】残金ゼロ時の機能制限解除要望 【HCL_Jegan】2014/07/28 Add End.
// comment 2014/11/25 OPS1-Sky工藤 [ZeusS][UV-01446].
//comment 2014/09/15 HCLT_Utsav [ZeusS][UV-01446] Adding NFC option in Authentication Screen(F097700000) for IT5.
//		if( True == PLBC_FuncVersionCheck::isOK_IT5_Type1()){
//			if( NULL!= PnlComGetNFCSetting() ){
//				m_e_NFCAuthenticationSettingEnable = PnlComGetNFCSetting()->getAuthNFC();	// To get the NFC authentication setting.
//			}
//		}
		
		// 開始時の設定値を保存しておく.
		b_UserOn_At = s_AuthCardParams.b_UserOn;
		memcpy( sc_UserCommand_At, s_AuthCardParams.sc_UserCommand, sizeof( sc_UserCommand_At ) );
// comment 2015/06/01 OPS1-MSE竹中 [MinervaSBK][CQ513861] 管理データクリア画面の遷移条件判定が不要となったため、変更前の設定値を保持する変数を削除.
//		m_bl_CardRegistration_At = m_bl_CardRegistration;

// comment 2014/12/09 OPS1-Sky Kawanishi [ZeusS][CQ507566] NFC使用設定キーが表示されない問題対応.
//// comment 2014/09/12 OPS1嵯峨山 [ZeusS] RQ-223 モバイル連携 BluetoothLE対応.
//		APIC_SystemInfo* pc_SystemInfo = APIC_SystemInfo::newInstance() ;
//		// トータルページ設定.
//		if(( TYPE_AUTH_DEVICE_ICC          == PnlComGetAuthDeviceSetting()->getAuthDeviceType() )	// ICカード認証.
//		 ||( TYPE_AUTH_DEVICE_LOADABLE_ICC == PnlComGetAuthDeviceSetting()->getAuthDeviceType() )){	// ローダブルデバイスドライバーICカード認証.
//			// デバイスタイプがICカードリーダかローダブルICカード認証の時.
//			sc_TotalPage = 3;
//// comment MinervaZeus -> Sky籔野 2011/02/09 [UV-00483].
////【CQ:500916】【UV-01276】【管理No.267】残金ゼロ時の機能制限解除要望 【HCL_Jegan】2014/07/28 Mod Start.
//		//} else if ( True == PnlComGetUserAuthentication()->getMidauthControl() ) {
//		} else if (( True == PnlComGetUserAuthentication()->getMidauthControl()) || 
////【CQ:500916】【UV-01276】【管理No.267】残金ゼロ時の機能制限解除要望-DIPSW Addition 【HCL_Jegan】2014/10/06 Mod Start.
//			//(( TYPE_UserAuthenticationIntermeddiateServer == m_e_UserAuthZeroBillingJob )||( TYPE_UserAuthenticationDeviceAndMidAuth  == m_e_UserAuthZeroBillingJob)) ) {
//
//			((( TYPE_UserAuthenticationIntermeddiateServer == m_e_UserAuthZeroBillingJob )||( TYPE_UserAuthenticationDeviceAndMidAuth  == m_e_UserAuthZeroBillingJob)) && ( True == PLBC_FuncVersionCheck::isOK_IT5_Type1() ) ) ) {
////【CQ:500916】【UV-01276】【管理No.267】残金ゼロ時の機能制限解除要望-DIPSW Addition 【HCL_Jegan】2014/10/06 Mod End.
////【CQ:500916】【UV-01276】【管理No.267】残金ゼロ時の機能制限解除要望 【HCL_Jegan】2014/07/28  Mod End.
//		 	// 権限管理/カウンター管理が設定されている場合.
//			sc_TotalPage = 3;
//// comment MinervaZeus <- Sky籔野 2011/02/09 [UV-00483].
//// comment 2014/09/12 OPS1嵯峨山 [ZeusS] RQ-223 モバイル連携 BluetoothLE対応.
//		} else if ( ( True == PLBC_FuncVersionCheck::isOK_IT5_Type1() ) 
//		&&	( NULL != pc_SystemInfo )
//		&&  ( TYPE_LocalIFKit_ON == pc_SystemInfo->getLocalIFKit() ) ) {
		// IT5_2nd 以降、NFC 使用設定キーが必ず表示される.
		if ( True == PLBC_FuncVersionCheck::isOK_IT5_Type1() ) {
			sc_TotalPage = 3;
// comment 2017/02/27 SCD2-Sky延原 [IT5_4.0][AR1005533].
			// ローカルIFキット(Bluetooth LE)装着時は、4Pになる.
			APIC_SystemInfo* pc_SystemInfo = APIC_SystemInfo::newInstance() ;
			if( ( NULL == pc_SystemInfo )
		    	|| ( TYPE_LocalIFKit_ON == pc_SystemInfo->getLocalIFKit() ) ) {
				sc_TotalPage = 4;
		    }
		} else { // IT5_2nd より前.
			if( ( TYPE_AUTH_DEVICE_ICC          == PnlComGetAuthDeviceSetting()->getAuthDeviceType() )	// ICカード認証.
			    ||( TYPE_AUTH_DEVICE_LOADABLE_ICC == PnlComGetAuthDeviceSetting()->getAuthDeviceType() ) ) {	// ローダブルデバイスドライバーICカード認証.
				// デバイスタイプがICカードリーダかローダブルICカード認証の時.
				sc_TotalPage = 3;
// comment 2017/10/31 【HCL-TANU】【IT5_4.2】【RQ686】【IT Defect AR1030817].
//				} else if ( True == PnlComGetUserAuthentication()->getMidauthControl() ) {
				} else if ( True == PnlComGetUserAuthentication()->getMidauthControl(PNLC_NetworkLan::getInstance() ->getAdminNWLine( )) ) {
				// 権限管理/カウンター管理が設定されている場合.
				sc_TotalPage = 3;
			} else {
				// 上記以外の時.
				sc_TotalPage = 2;
			}
		}
// UV-00678 ユーザ認証用のカード情報がLDAPサーバに登録されている場合の運用形態のサポート Sky福井 End.
// 2010/10/25 【Minerva/Zeus先行開発】UV-00676 外部認証サーバーダウン時の運用保証機能 Sky佐藤 Add Start.
		// ユーザー情報の上書き設定.
//		b_UserInfoOverwrite = apicAuthSrv->getUserControlOverwrite();
		b_UserInfoOverwrite = apicAuthSrv->getUserControlOverwrite( e_NetworkSpecific );
		// デフォルト認証方式.
//		sc_DefaulUserAttest = PnlComGetUserAuthentication()->getDefaultUserAuthentication();
		sc_DefaulUserAttest = PnlComGetUserAuthentication()->getDefaultUserAuthentication( e_NetworkSpecific );
// 2010/10/25 【Minerva/Zeus先行開発】UV-00676 外部認証サーバーダウン時の運用保証機能 Sky佐藤 Add End.
// comment 2014/11/25 OPS1-Sky工藤 [ZeusS][UV-01446].
// comment 2014/09/12 OPS1嵯峨山 [ZeusS] RQ-223 モバイル連携 BluetoothLE対応.
//		if ( True == PLBC_FuncVersionCheck::isOK_IT5_Type1() ) { 
//			APIC_BLESetting* pc_BLESetting = APIC_BLESetting::newInstance();
//			if ( NULL != pc_BLESetting ) {
//				m_e_BLEAuth = pc_BLESetting->getAuthBLE();
//			}
//		}
// comment 2015/2/12 OPS1-Sky工藤 [ZeusS][CQ509844].
		if ( True == PLBC_FuncVersionCheck::isOK_IT5_Type1() ) { 
			APIC_NFCSetting* pc_NFCSetting = APIC_NFCSetting::newInstance();
			if ( NULL != pc_NFCSetting ) {
// comment 2017/02/27 SCD2-Sky延原 [IT5_4.0][AR1005533].
//				onlyPanel.setNFCUse( pc_NFCSetting->getUseNFC() );
//				onlyPanel.setNFCAuth( pc_NFCSetting->getAuthNFC() );
				m_e_NFCUse = pc_NFCSetting->getUseNFC();
				m_e_NFCAuth = pc_NFCSetting->getAuthNFC();
			}
			
			APIC_BLESetting* pc_BLESetting = APIC_BLESetting::newInstance();
			if ( NULL != pc_BLESetting ) {
// comment 2017/02/27 SCD2-Sky延原 [IT5_4.0][AR1005533].
//				onlyPanel.setBLEUse( pc_BLESetting->getUseBLE() );
//				onlyPanel.setBLEAuth( pc_BLESetting->getAuthBLE() );
				m_e_BLEUse = pc_BLESetting->getUseBLE();
				m_e_BLEAuth = pc_BLESetting->getAuthBLE();
			}
		}
	
// comment 2016/5/25 [AUTHFUM2] [RQ-531]本体認証データ登録拡張 第22製品開発 山本.
		// 拡張ユーザーDBの設定値を取得する.
		b_AuthInsideLargeScale = PnlComGetUserAuthentication()->getAuthInsideLargeScale();
		b_AuthInsideLargeScale_At = b_AuthInsideLargeScale;
// comment 2016/5/25 [AUTHFUM2] [RQ-526]外部サーバーLDAP検索高速化 第22製品開発 山本.	
		APIC_DNCacheManager* apicDNCacheMan = APIC_DNCacheManager::newInstance();
		if( NULL == apicDNCacheMan ) {LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return;}
		// 以下の場合に、トータルページを4ページにする.
//		if( ( apicDNCacheMan->getDNCache() != TYPE_DNCache_Invalid )	// DNキャッシュがデバイス構成上利用不可でない.
		if( ( apicDNCacheMan->getDNCache( e_NetworkSpecific ) != TYPE_DNCache_Invalid )	// DNキャッシュがデバイス構成上利用不可でない.
			|| ( True == PnlComGetUser()->isEnableDataRegistrationExtension() )) {
			// 4ページ目を表示.
			sc_TotalPage = 4;
		}
		
//		TYPE_DNCache e_DNCache =  apicDNCacheMan->getDNCache();
		TYPE_DNCache e_DNCache =  apicDNCacheMan->getDNCache( e_NetworkSpecific );
		if( TYPE_DNCache_Enable == e_DNCache ) {
			b_ExtServerDNCache = True;
		} else {
			b_ExtServerDNCache = False;
		}

		// 認証情報の一時保存設定を取得.
//		PAdminToUserAndDepartSetcom::m_b_AuthCacheMode = PnlComGetUserAuthentication()->getAuthCacheMode();
		PAdminToUserAndDepartSetcom::m_b_AuthCacheMode = PnlComGetUserAuthentication()->getAuthCacheMode( e_NetworkSpecific );

// comment [2017/01/17] [PDD22山下(日立)] [IT5_4.0][AccuRev.1009975]下層からの画面遷移時は不揮発値を反映しないように変更.
		// 権限管理/カウンター管理設定値の取得.
// comment 2017/10/31 【HCL-TANU】【IT5_4.2】【RQ686】【IT Defect AR1030817].
//		b_AuthorityManagement = PnlComGetUserAuthentication()->getMidauthControl();
	    b_AuthorityManagement = PnlComGetUserAuthentication()->getMidauthControl(PNLC_NetworkLan::getInstance() ->getAdminNWLine( ));
	}
	b_NoAttestToBody = False;
	bMiddleServerWarning = False;
	bOutsideServerWarning = False;	// 外部サーバー認証関連.
// comment [2017/01/17] [PDD22山下(日立)] [IT5_4.0][AccuRev.1009975]下層からの画面遷移時に設定値を更新してしまうため、処理を移動.
	// 権限管理/カウンター管理設定値の取得.
//	b_AuthorityManagement = PnlComGetUserAuthentication()->getMidauthControl();
	// 左エリア種類の有効設定（ジョブ表示（通常））startGamen()の最後でコールしてください.
	setLeftAreaEnableType( TYPD_LeftAreaBookmark | TYPD_LeftAreaSoft10Key );
	if( True == PLBC_FuncPKICheck::isPKI() ) {
//		if( TYPE_UserAuthenticationIntermeddiateServer == PnlComGetUserAuthentication()->getUserAuthentication() ) {
		if( TYPE_UserAuthenticationIntermeddiateServer == PnlComGetUserAuthentication()->getUserAuthentication( e_NetworkSpecific ) ) {
			// 拡張サーバー認証を使用する.
			b_MiddleServerSetting = False;
			sc_UserAttest = 3;
		} else {
			// 外部サーバー認証を使用する.
			b_MiddleServerSetting = True;
			sc_UserAttest = 1;
		}
		b_MiddleServerSetting_At = b_MiddleServerSetting;
	}
}
//================================================================================================
// 概要		:管理者設定・ユーザ認証/部門管理・ユーザ/部門の連動認証(2691) : 表示画面のセットアップ
// 戻り値	:なし
//================================================================================================
void PAdminUserAndDepartSynchro::setupGamen( short option )
{
	PAdminToUserAndDepartSetcom::setupGamen( option );
	if( b_isError == True ) {
		// エラー発生中
		if( onlyPanel.getErrorNumberFieldPartsKind() == EP_ErrorPartsKind_ErrorOff ) {
			// １キー入力 or 一定時間でのエラー解除された場合
			// パネルの状態をエラーなしにする
			onlyPanel.setErrorNumberFieldPartsKind( EP_ErrorPartsKind_Non );
			// エラー表示を行わず、正常値を表示させる
			b_isError = False;
		}
	}
// Comment 2018/01/29 HCLT_Hanuman [IT5_4.2][AR1038061] To improve performance.
// 	Comment 2018/01/03 [HCL-Shalini]【IT5 4.2】【RQ-686】【WP1382,1384,1386,1388,1390】【AR1035279】Sharing Common Icon Image for Network 1&Network 2 Settings.
//	if( True == PnlComGetVlanSetting()->getVlanEnable() ){
	if( True == PNLC_NetworkLan::getInstance()->getVlanEnableWrapper() ){
			Key_AdminUserAndDepartSynchroLimitArrive.changeGraphic( NPK_NM1_VLAN_X240_Y30, 19, False );
			Key_AdminUserAndDepartSynchroKeepConnectionLimit.changeGraphic( NPK_NM1_VLAN_X240_Y30, 19, False );
	}
	Nfd_AdminNetworkSetComDspPage = sc_NowPage;			// 現ページ設定値
	Nfd_AdminNetworkSetComTotalPage = sc_TotalPage;		// 総ページ設定値
	Bak_UtilityMenuSettingBackground.showEnable( False );
	Grp_AdminUserAndDepartSynchroGraP1.showEnable( False );		// ページ1を非表示.
	Grp_AdminUserAndDepartSynchroGraP2.showEnable( False );		// ページ2を非表示.
	Grp_AdminUserAndDepartSynchroGraP2plus.showEnable( False );		// ページ2(ユーザ、部門認証あり時)を非表示.
	Grp_AdminUserAndDepartSynchroGraP3.showEnable( False );		// ページ3を非表示.
// comment 2016/5/25 [AUTHFUM2] [RQ-526]外部サーバーLDAP検索高速化 第22製品開発 山本.
	Grp_AdminUserAndDepartSynchroGraP4.showEnable( False );		// ページ4を非表示.
	setupGamen_CLCD();
}

//================================================================================================.
// 概要		:管理者設定・ユーザ認証/部門管理・ユーザ/部門の連動認証(2691) : 拡張カウンターの初期値取得.
// 戻り値	:short 拡張カウンターの初期値 HDD有り：300 HDD無し・1Gモデル：30.
//================================================================================================.
short PAdminUserAndDepartSynchro::getInitialExtCounter()
{
	short ss_ExtCounter = PNLD_InitialExtCounterWithoutHDD;
	short ss_MaxUser = 0;
	APIC_UserAuthentication* pc_UserAuth = PnlComGetUserAuthentication();	// ユーザー認証情報取得.
	if( NULL == pc_UserAuth ) {											// ユーザー情報が取れない場合はHDD無しとする.
		return ss_ExtCounter;
	}
// Comment 2018/01/29 HCLT_Hanuman [IT5_4.2][AR1038061] To improve performance.
////Comment 2017/09/04 [HCL-Harish] [IT5 4.2][RQ-686] [WP533] Multiple Network support - User Count.
////ss_MaxUser = pc_UserAuth->getMaxUser();									// ユーザーの最大登録数を取得する.
//	if (True == APIC_VlanSetting::newInstance()->getVlanEnable()) {
	if ( True == PNLC_NetworkLan::getInstance()->getVlanEnableWrapper() ) {
//Comment 2017/10/29 [HCL-Harish] [IT5 4.2][WP1146] User Registeration and track - Counter.
//		if ( TYPE_UserAuthenticationOff != PnlComGetUserAuthentication()->getUserAuthentication( TYPE_NetworkSpecific_NW2 )
//			&& TYPE_UserAuthenticationOff != PnlComGetUserAuthentication()->getUserAuthentication( TYPE_NetworkSpecific_NW1 ) ) {
//			ss_MaxUser = GetMaxUserCounter() / 2;
//		} else {
//			ss_MaxUser = GetMaxUserCounter();
//		}
		ss_MaxUser = GetMaxUserCounter();
	} else {
		ss_MaxUser = pc_UserAuth->getMaxUser();
	}
	if( TYPD_MaxUserWithHDD == ss_MaxUser ) {								// HDD有りモデルの場合.
		ss_ExtCounter = PNLD_InitialExtCounterWithHDD;
//	}
	} else if ( TYPD_MaxUserWithHDD / 2 == ss_MaxUser ){
		ss_ExtCounter = PNLD_InitialExtCounterWithHDD / 2;
	}
	return ss_ExtCounter;
}

//////////////////////////////////////////////////////////////.
// 概要		:管理者設定・ユーザ認証/部門管理・ユーザ/部門の連動認証 : 表示画面のセットアップ.
// 戻り値	:.
//////////////////////////////////////////////////////////////.
void PAdminUserAndDepartSynchro::setupGamen_CLCD()
{
// Comment 2018/01/29 HCLT_Hanuman [IT5_4.2][AR1038061] To improve performance.
//// Comment 2018/01/03 [HCL-Shalini]【IT5 4.2】【RQ-686】【WP1382,1384,1386,1388,1390】【AR1035279】Sharing Common Icon Image for Network 1&Network 2 Settings.
//	if( True == PnlComGetVlanSetting()->getVlanEnable() ){
	if( True == PNLC_NetworkLan::getInstance()->getVlanEnableWrapper() ){
		Txt_MessageSettingValue.showEnable( False );
		Txt_UtilityMenuSharedMessage2.showEnable( True );
	} else {
		Txt_UtilityMenuSharedMessage2.showEnable( False );
		Txt_MessageSettingValue.showEnable( True );
	}
	// セキュリティ強化設定ＯＮ時は、ユーザー認証「認証しない」キー・パブリックユーザーキーは非表示.
	Bool b_SecurityLevelUpOn = PnlComGetSystemSetting()->getSecurityLevelUpOn();	// セキュリティ強化設定取得.
	TYPE_KeyMode e_KeyMode = PnlComGetMachineSetting()->getKeyMode();				//	キーモード.
	APIC_AuthenticationSeverSetting* apicAuthSrv = APIC_AuthenticationSeverSetting::newInstance();				// サーバー情報へのアクセスクラス取得.
	Bak_UtilityMenuSettingBackground.showEnable( False );				// 右設定エリアユーティリティグルーピングタイトル.
	Rtb_AdminUserAndDepartSynchroUserAttestKey.showEnable( False );
	Rtb_AdminUserAndDepartSynchroPublicUserKey.showEnable( False );
	Rtb_AdminUserAndDepartSynchroManageKey.showEnable( False );
	Rtb_AdminUserAndDepartSynchroDepartFormKey.showEnable( False );
	Rtb_AdminUserAndDepartSynchroLimitArriveKey.showEnable( False );
	Rbt_AdminUserAndDepartSynchroMiddleServerAttestSetONOFF.showEnable( False );
	Rtb_AdminUserAndDepartSynchroDoRequestKey.showEnable( False );
	if( bMiddleServerWarning || bOutsideServerWarning ) {
		// 警告画面表示要.
		// 上部メッセージを変更.
		Txt_MessageSettingValue.changeText( s_MSGAdminServerSettingWarn );
		// OKキー非表示.
		Key_UtilityOk.showEnableShow( False );
		ushort usNGList[] = {s_Dummy,s_Dummy,s_Dummy,s_Dummy};
		const ushort cusMaxList = sizeof( usNGList )/sizeof( ushort );
		// NG項目の抜き出し.
		uchar ucIndex_buf = 0;
		if( bMiddleServerWarning ) {
			if( bBoxAdminPublicUser_NG ) {
				usNGList[ucIndex_buf] = s_EXPAdminBoxPublicUser;
				ucIndex_buf++;
			}
			if( bOpenAPI_NG ) {
				if( ucIndex_buf < cusMaxList ) {
					usNGList[ucIndex_buf] = s_EXPOpenAPI;
				}
			}
			if( bAuthDeviceBody_NG ) {
				// 生体認証装置が設定されている.
				if( ucIndex_buf < cusMaxList ) {
					usNGList[ucIndex_buf] = s_ITMAdminAuthenticationDeviceSet;
				}
			}
		} else if( bOutsideServerWarning ) {
			if( bAuthDeviceBody_NG ) {
				// 生体認証装置が設定されている.
				if( ucIndex_buf < cusMaxList ) {
					if( True == PLBC_FuncVersionCheck::isOK_TypeMLK() ) {
						uchar ucAuthDeviceType = PnlComGetAuthDeviceSetting()->getAuthDeviceType();
						if( TYPE_AUTH_DEVICE_ICC == ucAuthDeviceType ) {			// ICカード認証.
							usNGList[ucIndex_buf] = s_ITMAdminLDAPAndCardAuthSetting; // 「LDAP-ICカード認証設定」.
						} else {
							usNGList[ucIndex_buf] = s_ITMAdminAuthenticationDeviceSet;
						}
					} else {
						usNGList[ucIndex_buf] = s_ITMAdminAuthenticationDeviceSet;
					}
				}
			}
		}
		// 表示板への展開.
		for( int i = 0; i < cusMaxList; i++ ) {
			if( usNGList[i] != s_Dummy ) {
				Gra_AdminUserAndDepartSynchroPlate[i].changeText( usNGList[i] );
				Gra_AdminUserAndDepartSynchroPlate[i].showEnable( True );
			} else {
				Gra_AdminUserAndDepartSynchroPlate[i].showEnable( False );
			}
		}
		Nfd_AdminNetworkSetComDspPage.showEnable( False );				// 現ページ設定値
		Nfd_AdminNetworkSetComTotalPage.showEnable( False );				// 総ページ設定値
		// 警告画面用部品を表示.
		Grp_AdminUserAndDepartSynchroMiddleServerWarning.showEnable( True );
		return;
	} else {
		// 通常設定画面表示.
		Grp_AdminUserAndDepartSynchroMiddleServerWarning.showEnable( False );
		// OKキー表示.
		Key_UtilityOk.showEnableShow( True );
		Nfd_AdminNetworkSetComDspPage.showEnable( True );					// 現ページ設定値
		Nfd_AdminNetworkSetComTotalPage.showEnable( True );				// 総ページ設定値
	}
	// ↓ 1ページ目の処理 .
	if( sc_NowPage == 1 ) {
// comment 2017/02/27 SCD2-Sky延原 [IT5_4.0][AR1005533] 1Pのキー選択が残る.
//		Rtb_AdminUserAndDepartSynchro.select( (short)( sc_KeySelect - 1 ) );			// 10920010-1   認証方式1Pで使用されるグループ.
		if( True == PLBC_FuncPKICheck::isPKI() ) {
			if( 0 <= sc_KeySelect && sc_KeySelect <= 5 )	{ Bak_UtilityMenuSettingBackground.showEnable( True ); }			// 右設定エリアユーティリティグルーピングタイトル.
		} else {
			if( 0 <= sc_KeySelect && sc_KeySelect <= 4 )	{ Bak_UtilityMenuSettingBackground.showEnable( True ); }			// 右設定エリアユーティリティグルーピングタイトル.
		}
		// 右エリアの設定表示処理.
		switch( sc_KeySelect ) {
			case 0:	// 認証設定選択時.
				// 右エリア「認証しない」キーの表示処理.
				Key_AdminUserAndDepartSynchroNoAttest.showEnable( b_SecurityLevelUpOn == True ? False : True );
				// 右エリア「外部サーバー認証」キーの表示処理.
				if( e_KeyMode == TYPE_KeyModeAdminMachineKM ) {	// 統合管理装置接続時.
					Key_AdminUserAndDepartSynchroOutside.showEnable( False );
				} else {	// 統合管理装置未接続時.
					Key_AdminUserAndDepartSynchroOutside.showEnable( True );
					if( ( apicAuthSrv ) && ( apicAuthSrv->getDefaultServerIndex() != APID_AuthenticationServerIndexNone ) ) {	// 外部サーバー登録済み.
						Key_AdminUserAndDepartSynchroOutside.activate( True );
					} else {	// 外部サーバー登録なし.
						Key_AdminUserAndDepartSynchroOutside.activate( False );
					}
				}
				// 右エリア｢本体装置認証｣キーの表示処理.
				if( b_AuthorityManagement == True ) {		// 権限管理/カウンター管理｢管理する｣.
					Key_AdminUserAndDepartSynchroBody.activate( False );	// グレーアウト.
				} else {
					Key_AdminUserAndDepartSynchroBody.activate( True );		// 活性化.
				}

				if( True != PLBC_FuncPKICheck::isPKI() ) {
					Rtb_AdminUserAndDepartSynchroUserAttestKey.select( sc_UserAttest );		// ユーザ認証方法、しない、外部、本体.
					Rtb_AdminUserAndDepartSynchroUserAttestKey.showEnable( True );
				}
				break;
			case 1:	// パブリックユーザー選択時.
			{
				Rtb_AdminUserAndDepartSynchroPublicUserKey.select( ss_PublicUser );		// パブリックユーザ許可する、しない.
				Rtb_AdminUserAndDepartSynchroPublicUserKey.showEnable( True );
				// 右エリア「許可しない」「許可後、ログイン」キーの表示処理.
				if( e_KeyMode == TYPE_KeyModeAdminMachineKM ) {	// 統合管理装置接続時.
					Key_AdminUserAndDepartSynchroPermitLogin.showEnable( False );		// 許可後、ログインキー.
					Key_AdminUserAndDepartSynchroNotPermit.showEnable( False );			// 許可しないキー.
					Key_AdminUserAndDepartSynchroPermit.showEnable( True );				// 許可するキー.
				} else if( TYPE_KeyModeVendorKM == e_KeyMode ) {						// ベンダー2装着時.
					Key_AdminUserAndDepartSynchroPermitLogin.showEnable( True );		// 許可する(ﾛｸﾞｲﾝなし)キー.
					Key_AdminUserAndDepartSynchroNotPermit.showEnable( False );			// 許可しないキー.
					Key_AdminUserAndDepartSynchroPermit.showEnable( False );			// 許可する(ﾛｸﾞｲﾝあり)キー.
				} else {	// 統合管理装置未接続＋強化設定OFFの場合.
					Key_AdminUserAndDepartSynchroNotPermit.showEnable( True );			// 許可しないキー.
					Key_AdminUserAndDepartSynchroPermit.showEnable( True );				// 許可するキー.
					Key_AdminUserAndDepartSynchroPermitLogin.showEnable( True );		// 許可後、ログインキー.
				}
				break;
			}
			case 2:	// 制限機能使用時のログイン選択時.
				Rtb_AdminUserAndDepartSynchroDoRequestKey.select( b_DoRequest );		// 要求する/要求しない.
				Rtb_AdminUserAndDepartSynchroDoRequestKey.showEnable( True );
				break;
			case 3:	// 部門管理選択時.
				// 右エリア「管理する」キーの表示処理.
				if( ( TYPE_KeyModeAdminMachineKM == e_KeyMode )
				    || ( TYPE_KeyModeVendorKM == e_KeyMode ) ) {
					Key_AdminUserAndDepartSynchroManage.showEnable( False );
				} else {
					Key_AdminUserAndDepartSynchroManage.showEnable( True );
				}
				Rtb_AdminUserAndDepartSynchroManageKey.select( b_Manage );				// 部門管理する、しない.
				Rtb_AdminUserAndDepartSynchroManageKey.showEnable( True );
				break;
			case 4:	// 部門管理認証方式選択時.
				// 右エリア「パスワードのみ」キーの表示処理.
				if( b_SecurityLevelUpOn == True ) {	//  セキュリティ強化設定ON時はパスワードのみを認めない.
					Key_AdminUserAndDepartSynchroOnlyPass.showEnable( False );		// パスワードのみキー.
				} else {	// 強化モードOFF時.
					Key_AdminUserAndDepartSynchroOnlyPass.showEnable( True );			// パスワードのみキー.
					if( sc_UserAttest != 0 && b_Manage == True	) {	// ユーザー認証する かつ.部門管理「管理する」の場合、「パスワードのみ」キーはグレーアウト.
						Key_AdminUserAndDepartSynchroOnlyPass.activate( False );	// パスワードのみキー.
					} else {
						Key_AdminUserAndDepartSynchroOnlyPass.activate( True );		// パスワードのみキー.
					}
				}
				Rtb_AdminUserAndDepartSynchroDepartFormKey.select( ss_departForm );		// 部門管理方式、部門名+パスワード、パスワードのみ.
				Rtb_AdminUserAndDepartSynchroDepartFormKey.showEnable( True );
				break;
			case 5:	// 拡張サーバー認証.
				// 使用する、使用しない キーを表示する.
				Rbt_AdminUserAndDepartSynchroMiddleServerAttestSetONOFF.select( b_MiddleServerSetting );
				Rbt_AdminUserAndDepartSynchroMiddleServerAttestSetONOFF.showEnable( True );
				break;
			default:
				break;
		}
		// 「ユーザ認証」の設定値表示.
		switch( sc_UserAttest ) {
			case 0: Btx_AdminUserAndDepartSynchroUserAttest.changeText( s_AdminUserAndDepartNoAttest );		break;	// 認証しない.
			case 1: Btx_AdminUserAndDepartSynchroUserAttest.changeText( s_AdminUserAndDepartOutside );		break;	// 外部サーバー認証.
			case 2: Btx_AdminUserAndDepartSynchroUserAttest.changeText( s_AdminUserAndDepartBody );			break;	// 本体装置認証.
			case 3: Btx_AdminUserAndDepartSynchroUserAttest.changeText( s_EXPMiddleAuthentication_1 );		break;	// 拡張サーバー認証.
			case 4: Btx_AdminUserAndDepartSynchroUserAttest.changeText( s_ITMAdminUserAndDepartBodyAndOutside );	break;	// 本体+外部サーバー認証.
			case 5: Btx_AdminUserAndDepartSynchroUserAttest.changeText( s_ITMAdminUserAndDepartExtensionServer );	break;	// 本体+拡張サーバー認証.
			default: LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );			break;
		}
		// 「パブリックユーザ」の設定値表示＋グレーアウト処理.
		if( ss_PublicUser == -1
		    || ( ( False == PLBC_FuncVersionCheck::isOK_Type1() ) && ( sc_UserAttest != 0 && b_Manage == True && b_Synchro == True ) )	// パブリックユーザ設定禁止状態時.or ユーザー認証する かつ.部門管理「管理する」かつ 「連動しない」の場合、「パブリックユーザー」キーはグレーアウト.
		    || ( ( True == PLBC_FuncVersionCheck::isOK_Type1() )																		// もしくは、Amur/Donau機能拡張1で,もしくは APIの返り値がNULLでなく、中間認証+Publicユーザー登録変更禁止時.
		         && ( NULL != PnlComGetUserAuthentication() )																			//
//		         && ( TYPE_UserAuthenticationIntermeddiateServer == PnlComGetUserAuthentication()->getUserAuthentication() )
		         && ( TYPE_UserAuthenticationIntermeddiateServer == PnlComGetUserAuthentication()->getUserAuthentication( PNLC_NetworkLan::getInstance()->getAdminNWLine( ) ) )
		         && ( False == PnlComGetUserAuthentication()->getEnablePublicUserInfo() ) ) ) {
			Key_AdminUserAndDepartSynchroPublicUser.activate( False );				// キーをグレーアウト.
			Btx_AdminUserAndDepartSynchroPublicUser.showEnableAdd( False );			// 設定内容表示.
			Btx_AdminUserAndDepartSynchroPublicUser.activate( False );				// 設定内容をグレーアウト.
		} else if( True == b_AuthorityManagement												// 権限管理/カウンター管理｢管理する｣.
		           && False == PnlComGetUserAuthentication()->getEnablePublicUserInfo() ) {		// Publicユーザ情報変更不許可.
			Key_AdminUserAndDepartSynchroPublicUser.activate( False );				// キーをグレーアウト.
			Btx_AdminUserAndDepartSynchroPublicUser.showEnableAdd( False );			// 設定内容表示.
			Btx_AdminUserAndDepartSynchroPublicUser.activate( False );				// 設定内容をグレーアウト.
		} else {	// パブリックユーザ設定可能状態.
			Key_AdminUserAndDepartSynchroPublicUser.activate( True );		// キーを活性状態.
			Btx_AdminUserAndDepartSynchroPublicUser.showEnableAdd( True );	// 設定内容表示.
			Btx_AdminUserAndDepartSynchroPublicUser.activate( True );		// 設定内容を活性状態.
			switch( ss_PublicUser ) {	// 設定値の表示.
				case 0:	Btx_AdminUserAndDepartSynchroPublicUser.changeText( s_AdminUserAndDepartNotPermit );	break;	// 許可しない.
				case 1:	Btx_AdminUserAndDepartSynchroPublicUser.changeText( s_EXPLoginRequired );				break;	// 許可する(ログインあり).
				case 2:	Btx_AdminUserAndDepartSynchroPublicUser.changeText( s_EXPPermission_1 );				break;	// 許可する(ログインなし).
				default: LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );			break;
			}
		}
		// 「ユーザー認証」or「拡張サーバー認証」の設定値表示＋グレーアウト処理.
		Bool b_EnableUserKey = False;			// "ユーザー認証"キーを非表示にする.
		if (True == PLBC_FuncPKICheck::isPKI()) {
		} else {
			// PKI機能が無効である場合、"ユーザー認証"キーを表示する.
			b_EnableUserKey = True;
		}
		// "拡張サーバー認証"キーを表示しない.
		Key_AdminUserAndDepartSynchroMiddleServerAttest.showEnable(False);
		Btx_AdminUserAndDepartSynchroMiddleServerAttest.showEnable(False);
		if (True == b_EnableUserKey) {	// "ユーザー認証"キーを表示する.
			Key_AdminUserAndDepartSynchroUserAttest.showEnable(True);
			Btx_AdminUserAndDepartSynchroUserAttest.showEnableAdd(True);
			Btx_AdminUserAndDepartSynchroUserAttest.showEnable(True);
		} else {						// "ユーザー認証"キーを表示しない.
			Key_AdminUserAndDepartSynchroUserAttest.showEnable(False);
			Btx_AdminUserAndDepartSynchroUserAttest.showEnable(False);
		}
		// 「制限機能使用時のログイン」の設定値表示＋グレーアウト処理.
		if( ( False == Key_AdminUserAndDepartSynchroPublicUser.isNowShowEnable() )
		    || ( ( False == Key_AdminUserAndDepartSynchroPermit.isNowShowEnable() )
		         && ( False == Key_AdminUserAndDepartSynchroPermitLogin.isNowShowEnable() ) ) ) {
			// パブリックユーザキーが非表示の場合.
			// もしくは、パブリックユーザー｢許可する(ログインなし)｣/「許可する(ログインあり)」キーが共に非表示の場合.
			Key_AdminUserAndDepartSynchroLoginFuncPermit.showEnable( False );				// キーを非表示.
			Btx_AdminUserAndDepartSynchroLoginFuncPermit.showEnable( False );				// 設定内容を非表示.
			b_DoRequest = False;															// 設定値をデフォルト(要求しない)に戻す.
		} else {
			Key_AdminUserAndDepartSynchroLoginFuncPermit.showEnable( True );				// キーを表示.
			Btx_AdminUserAndDepartSynchroLoginFuncPermit.showEnable( True );				// 設定内容を表示.
			char sc_AppID[TYPD_IwsMaxID+1];
			memset( sc_AppID,'\0',sizeof( sc_AppID ) );
			onlyPanel.getIwsAuthDefaultID( sc_AppID );
			if( ( ( False == Key_AdminUserAndDepartSynchroPublicUser.isNowActivate() ) ||
			      // パブリックユーザキーがグレイアウト、または認証連携ブラウザの場合.
			      ( 3 == sc_UserAttest && TYPE_ExtTypeBrowserAuth == PnlComGetOapAuthenticationAplType() ) ) ||
			    ( ( '\0' != sc_AppID[0] ) ) ) {
				Key_AdminUserAndDepartSynchroLoginFuncPermit.activate( False );					// キーをグレーアウト.
				Btx_AdminUserAndDepartSynchroLoginFuncPermit.showEnableAdd( False );			// 設定内容非表示.
				Btx_AdminUserAndDepartSynchroLoginFuncPermit.activate( False );					// 設定内容をグレーアウト.
				b_DoRequest = False;															// 設定値をデフォルト(要求しない)に戻す.
			} else {
				Key_AdminUserAndDepartSynchroLoginFuncPermit.activate( True );					// キーを活性状態.
				Btx_AdminUserAndDepartSynchroLoginFuncPermit.showEnableAdd( True );				// 設定内容表示.
				Btx_AdminUserAndDepartSynchroLoginFuncPermit.activate( True );					// 設定内容を活性状態.
				Btx_AdminUserAndDepartSynchroLoginFuncPermit.changeText( ( False == b_DoRequest ) ? s_SYSAdminNotDoRequest : s_SYSAdminDoRequest );	// 設定値の表示.
			}
		}
		// 「部門管理」の設定値表示＋グレーアウト処理.
		Bool b_ActivateDepartManage = False;	// "部門管理"キーをグレーアウト表示にする.
		if( True == PLBC_FuncPKICheck::isPKI() ) {
// comment 2014/12/02 OPS1-Sky白籏 [Ze5BK][503961] 権限管理/カウンター管理する場合、グレイアウト表示 Start.
//			if (False == b_MiddleServerSetting) { b_Manage = False; }
			if (True  == b_AuthorityManagement) { b_Manage = False; }
// comment 2014/12/02 OPS1-Sky白籏 [Ze5BK][503961] 権限管理/カウンター管理する場合、グレイアウト表示 End.
			else { b_ActivateDepartManage = True; }
		} else {
			if( sc_UserAttest == 3		// 中間認証選択時、グレーアウト.
			    || b_AuthorityManagement == True ) {		// 権限管理/カウンター管理｢管理する｣.
				b_Synchro = True;		// 連動しない
				ss_departForm = -1;		// 認証方式未選択
				b_Manage = False;		// 部門管理しない
			} else { b_ActivateDepartManage = True; }
		}
		Key_AdminUserAndDepartSynchroDepartManage.activate( b_ActivateDepartManage );
		Txt_AdminUserAndDepartSynchroDepartManage.showEnable( b_ActivateDepartManage );
		Btx_AdminUserAndDepartSynchroDepartManage.changeText( ( False == b_Manage ) ? s_AdminUserAndDepartNotManage : s_AdminUserAndDepartManage );
		Btx_AdminUserAndDepartSynchroDepartManage.activate( b_ActivateDepartManage );
		// 「部門管理認証方式」の設定値表示＋グレーアウト処理.
		// 条件：認証方式未選択状態(部門管理しない)時は、部門管理認証方式はグレーアウト.
		if( ( TYPE_KeyModeAdminMachineKM == e_KeyMode ) 						// 統合管理装置接続時.
		    || ( TYPE_KeyModeVendorKM == e_KeyMode ) ) {							// ベンダー２装着時.
			Key_AdminUserAndDepartSynchroDepartForm.showEnable( False );		// 部門管理認証方式キー.
			Btx_AdminUserAndDepartSynchroDepartForm.showEnable( False );		// 部門管理認証方式.
		} else {
			Key_AdminUserAndDepartSynchroDepartForm.showEnable( True );			// 部門管理認証方式キー.
			Btx_AdminUserAndDepartSynchroDepartForm.showEnable( True );			// 部門管理認証方式.
			if( ss_departForm == -1 || sc_UserAttest == 3 ) {	// 認証方式未選択状態(部門管理しない) or 中間認証選択時.
				Key_AdminUserAndDepartSynchroDepartForm.activate( False );		// キーをグレーアウト.
				Txt_AdminUserAndDepartSynchroDepartForm.showEnable( False );	// 設定内容表示.
				Btx_AdminUserAndDepartSynchroDepartForm.activate( False );		// 設定内容をグレーアウト.
			} else {
				Key_AdminUserAndDepartSynchroDepartForm.activate( True );		// キーを活性状態.
				Txt_AdminUserAndDepartSynchroDepartForm.showEnable( True );		// 設定内容表示.
				Btx_AdminUserAndDepartSynchroDepartForm.activate( True );		// 設定内容を活性状態.
				switch( ss_departForm ) {
					case 0:		Btx_AdminUserAndDepartSynchroDepartForm.changeText( s_KEYTrackNameAndPassword );	break;	// 部門名+パスワード.// シンボル差し替え( 73235 2006/11/28 12谷本 ).
					case 1:		Btx_AdminUserAndDepartSynchroDepartForm.changeText( s_KEYPasswordOnly );			break;	// パスワードのみ.// シンボル差し替え( 65827 2006/08/09 13野﨑 ).
					default:	LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );		break;
				}
			}
		}
		// メッセージの表示設定.
		// 1ページ目の設定項目が選択されていない場合.
		//  「項目を選択し、設定を変更できます」.
		// 1ページ目の設定項目が選択されている場合.
		//  「設定値のキー選択で設定できます」.
		if( -1 == sc_KeySelect || 6 <= sc_KeySelect ) {
			Txt_MessageSettingValue.changeText( s_MSGChangeSettingValue_a );
// comment 2017/02/27 SCD2-Sky延原 [IT5_4.0][AR1005533] 1Pのキー選択が残る.
			Rtb_AdminUserAndDepartSynchro.select( -1 );						// 認証方式1Pで使用されるグループ、キー非選択状態.
		} else {
			Txt_MessageSettingValue.changeText( s_MSGSelectSettingValue_02 );
// comment 2017/02/27 SCD2-Sky延原 [IT5_4.0][AR1005533] 1Pのキー選択が残る.
			Rtb_AdminUserAndDepartSynchro.select( (short)(sc_KeySelect - 1 ) );			// 認証方式1Pで使用されるグループ、キー選択状態.
		}
		Grp_AdminUserAndDepartSynchroGraP1.showEnable( True );		// ページ1を表示.
		// ↑ 1ページ目の処理 .
		// ↓ 2ページ目の処理 .
	} else if( sc_NowPage == 2 ) {
		Bak_UtilityMenuSettingBackground.showEnable( True );
		Grp_AdminUserAndDepartSynchroGraP2plus.showEnable( True );				// ページ2を表示.
		// メッセージの表示設定.
// comment 2017/02/27 SCD2-Sky延原 [IT5_4.0][AR1005533] メッセージが正しく表示されない.
//		if( sc_KeySelect <= 5 ) {	// 未選択状態.
		if( sc_KeySelect <= 5 || sc_KeySelect >= 9 ) {	// 未選択状態.
			Txt_MessageSettingValue.changeText( s_MSGChangeSettingValue_a );		//「項目を選択し、設定を変更できます」を表示.
			Rtb_AdminUserAndDepartSynchro2p.select( - 1 );						// 認証方式2Pの設定選択キー.
		} else {
			Txt_MessageSettingValue.changeText( s_MSGSelectSettingValue_02 );
			Rtb_AdminUserAndDepartSynchro2p.select( (short)( sc_KeySelect - 6 ) );			// 認証方式2Pの設定選択キー.
		}
		// 「ユーザー認証/部門認証の連動」の設定値表示＋グレーアウト処理.
		if( True == PLBC_FuncPKICheck::isPKI() ) {
			Key_AdminUserAndDepartSynchroUserDepartAttest.showEnable( False );	// キー.
			Btx_AdminUserAndDepartSynchroUserDepartAttest.showEnable( False );	// 設定内容.
		} else {
			if( b_Synchro == False ) {
				if( PNLE_TrackModeLink_UserSelect == pdpc_UserAndTrackMode.getTrackModeLink() ) {
					Btx_AdminUserAndDepartSynchroUserDepartAttest.changeText( s_KEYUserSettings );					// ユーザーが設定する.
				} else {
					Btx_AdminUserAndDepartSynchroUserDepartAttest.changeText( s_AdminUserAndDepartSynchroOn );		// 連動する.
				}
			} else {
				Btx_AdminUserAndDepartSynchroUserDepartAttest.changeText( s_AdminUserAndDepartSynchroOff );			// 連動しない.
			}
			if( e_KeyMode == TYPE_KeyModeAdminMachineKM ) {								// 統合管理装置接続時は非表示.
				Key_AdminUserAndDepartSynchroUserDepartAttest.showEnable( False );		// キー.
				Btx_AdminUserAndDepartSynchroUserDepartAttest.showEnable( False );		// 設定内容.
			} else {
				Key_AdminUserAndDepartSynchroUserDepartAttest.showEnable( True );		// キー.
				Btx_AdminUserAndDepartSynchroUserDepartAttest.showEnable( True );		// 設定内容.
				if( sc_UserAttest != 0 && b_Manage == True && sc_UserAttest != 3 ) {	// ユーザ認証する+部門管理する+中間認証設定ではない場合、「ユーザ認証/部門認証の連動」 有効.
					Key_AdminUserAndDepartSynchroUserDepartAttest.activate( True );		// キー.
					Txt_AdminUserAndDepartSynchroUserDepartAttest.showEnable( True ); 	// 設定表示.
					Btx_AdminUserAndDepartSynchroUserDepartAttest.activate( True );		// 設定内容.
				} else {	// 「ユーザ認証/部門認証の連動」をグレーアウト.
					Key_AdminUserAndDepartSynchroUserDepartAttest.activate( False );	// キー.
					Txt_AdminUserAndDepartSynchroUserDepartAttest.showEnable( False ); 	// 設定表示.
					Btx_AdminUserAndDepartSynchroUserDepartAttest.activate( False );	// 設定内容.
				}
			}
		}
		// 「上限値到達時の動作」の設定値表示.
		switch( sc_LimitArrive ) {
			case 0:		Btx_AdminUserAndDepartSynchroLimitArrive.changeText( s_KEYAdminUserAndDepartDepartManageLimitArriveActionJump );	break;	// ジョブ飛越しキー.
			case 1:		Btx_AdminUserAndDepartSynchroLimitArrive.changeText( s_KEYStopJob );												break;
			case 2:		Btx_AdminUserAndDepartSynchroLimitArrive.changeText( s_KEYAdminUserAndDepartDepartManageLimitArriveActionDelete );	break;	// ジョブ削除.
			default:	LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );										break;
		}
		// MiZe基本拡張2以降はキーに変更がある為、タイプ別に処理を分ける.
		// 「ユーザーカウンター割当て数」の設定値表示＋グレーアウト処理.
		if( True == PLBC_FuncVersionCheck::isOK_MiZe_Type2() ) {
			Key_AdminUserAndDepartSynchroCoutControlI.changeText( s_KEYCounterAllocate );		// キー.
			Btx_AdminUserAndDepartSynchroCoutControlI.showEnable( False );		// 設定内容.

			if( e_KeyMode == TYPE_KeyModeAdminMachineKM ) {	// 統合管理装置接続時は非表示.
				Key_AdminUserAndDepartSynchroCoutControlI.showEnable( False );
			} else {
				Key_AdminUserAndDepartSynchroCoutControlI.showEnable( True );
				if( ( 0 != sc_UserAttest && 5 == sc_UserAttest ) || ( 0 != sc_UserAttest && True == b_Manage && 3 != sc_UserAttest ) ) {
					Key_AdminUserAndDepartSynchroCoutControlI.activate( True );
				} else {
					Key_AdminUserAndDepartSynchroCoutControlI.activate( False );
				}
			}
		}
		else {
			if( e_KeyMode == TYPE_KeyModeAdminMachineKM ) {	// 統合管理装置接続時は非表示.
				Key_AdminUserAndDepartSynchroCoutControlI.showEnable( False );		// キー.
				Btx_AdminUserAndDepartSynchroCoutControlI.showEnable( False );		// 設定内容.
			} else {
				Key_AdminUserAndDepartSynchroCoutControlI.showEnable( True );		// キー.
				Btx_AdminUserAndDepartSynchroCoutControlI.showEnable( True );		// 設定内容.
				if( sc_UserAttest != 0 && b_Manage == True && sc_UserAttest != 3 ) {	// ユーザ認証する+部門管理する+中間認証設定ではない場合「ユーザーカウンター割当て数」 有効.
					Key_AdminUserAndDepartSynchroCoutControlI.activate( True );		// キー.
					Num_AdminUserAndDepartSynchroControlInput.showEnable( True );	// 設定表示.
					Btx_AdminUserAndDepartSynchroCoutControlI.activate( True );		// 設定内容.
				} else {
					Key_AdminUserAndDepartSynchroCoutControlI.activate( False );	// キー.
					Num_AdminUserAndDepartSynchroControlInput.showEnable( False );	// 設定表示.
					Btx_AdminUserAndDepartSynchroCoutControlI.activate( False );	// 設定内容.
				}
				Btx_AdminUserAndDepartSynchroCoutControlI = ss_UserCounter;			// ユーザカウンタ割り当て数.
			}
		}
		// 「チケット保持時間」の設定値表示.
		Grp_AdminUserAndDepartSynchroKeepConnectionLimit.set( ss_ConnectTime, b_Fixed, -1, b_isError );		// チケット保持時間.
		// 「スキャン設定」の設定値表示.
		// 条件：Fiery接続時のみ表示.
		Bool bl_IsFiery = ( ppc_SystemInfo->getExternalIFMode() == TYPE_ExternalIFMode1 ) ? True : False;		// Fiery接続フラグ.
		if( True == bl_IsFiery ) { bl_IsFiery = c_HD_Equip.isOK(); }
		Key_AdminUserAndDepartSynchroPullScanFiery.showEnable( bl_IsFiery );		// スキャン設定キー.
		Btx_AdminUserAndDepartSynchroPullScanFiery.showEnable( bl_IsFiery );		// 許可しない／許可する・背景.
		Btx_AdminUserAndDepartSynchroPullScanFiery.changeText( ( b_isEnableFieryPullScan == True )? s_AdminUserAndDepartUsePermit : s_AdminUserAndDepartNotPermit );		// 許可しない／許可する・背景.
		// 右側表示処理.
		// 先に右側表示のパーツを非表示とする.
		Rtb_AdminUserAndDepartSynchroKey.showEnable( False );							// ユーザー認証/部門認証の連動キー・非表示
		Rtb_AdminUserAndDepartSynchroLimitArriveKey.showEnable( False );			// 上限値到達の設定キー・非表示.
		Rtb_AdminUserAndDepartSynchroPullScanFieryKey.showEnable( False );			// スキャンキーの設定キー・非表示.
		Bak_UtilityMenuSettingBackground.showEnable( False );						// 右設定エリアユーティリティグルーピングタイトル.
		// 右エリア「ユーザー認証/部門認証の連動」の設定値表示.
		if( 6 == sc_KeySelect ) {	// ユーザー認証/部門認証の連動・選択中.
			if( b_Manage == True && sc_UserAttest != 0 ) {
				switch( pdpc_UserAndTrackMode.getTrackModeLink() ) {
					case PNLE_TrackModeLink_Link:		Rtb_AdminUserAndDepartSynchroKey.select( 0 );	break;
					case PNLE_TrackModeLink_UnLink:		Rtb_AdminUserAndDepartSynchroKey.select( 1 );	break;
					case PNLE_TrackModeLink_UserSelect:	Rtb_AdminUserAndDepartSynchroKey.select( 2 );	break;
					default :	LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );	break;
				}
				Bak_UtilityMenuSettingBackground.showEnable( True );						// 右設定エリアユーティリティグルーピングタイトル.
				Rtb_AdminUserAndDepartSynchroKey.showEnable( True );
			}
			// 右エリア「上限値到達時の動作」の設定値表示.
		} else if( 7 == sc_KeySelect ) {	// 上限値到達・選択中.
			Bak_UtilityMenuSettingBackground.showEnable( True );						// 右設定エリアユーティリティグルーピングタイトル.
			Rtb_AdminUserAndDepartSynchroLimitArriveKey.select( sc_LimitArrive );		// 上限値到達時の動作.
			Key_AdminUserAndDepartSynchroLimitArriveDelete.showEnable( PLBC_FuncVersionCheck::isOK_IT5_Type1() );	// 「ジョブ削除」キーは、機能拡張1以降表示する.
			Rtb_AdminUserAndDepartSynchroLimitArriveKey.showEnable( True );				// 上限値到達の設定キー・表示.
			// 右エリア「スキャン設定」の設定値表示.
		} else if( 8 == sc_KeySelect ) {	// スキャン設定・選択中.
			Bak_UtilityMenuSettingBackground.showEnable( True );						// 右設定エリアユーティリティグルーピングタイトル.
			Rtb_AdminUserAndDepartSynchroPullScanFieryKey.select( ( b_isEnableFieryPullScan == True )? 1 : 0 );	// 許可する／しない.
			Rtb_AdminUserAndDepartSynchroPullScanFieryKey.showEnable( True );			// スキャンキーの設定キー・表示.
		}		//DonauLastFum【Erie拡張】【13笹岡】20090518 del.
		// ↑ 2ページ目の処理 .
		// ↓ 3ページ目の処理 .
	} else if( sc_NowPage == 3 ) {
// comment 2017/02/27 SCD2-Sky延原 [IT5_4.0][AR1005533] 
//		Bak_UtilityMenuSettingBackground.showEnable( True );
		Bak_UtilityMenuSettingBackground.showEnable( False );						// 右設定エリアユーティリティグルーピングタイトル.
		Grp_AdminUserAndDepartSynchroGraP3.showEnable( True );		// ページ3を表示.
		// メッセージの表示設定.
// comment 2017/02/27 SCD2-Sky延原 [IT5_4.0][AR1005533] メッセージが正しく表示されない.
//		if( sc_KeySelect <= 8 ) {	// 未選択状態.
		if( sc_KeySelect <= 8 || sc_KeySelect >= 13 ) {	// 未選択状態.
			Txt_MessageSettingValue.changeText( s_MSGChangeSettingValue_a );		//「項目を選択し、設定を変更できます」を表示.
			Rtb_AdminUserAndDepartSynchro3p.select( - 1 );							// 認証方式3Pの設定選択キー.
		} else {
			Txt_MessageSettingValue.changeText( s_MSGSelectSettingValue_02 );
			Rtb_AdminUserAndDepartSynchro3p.select( (short)( sc_KeySelect - 9 ) );				// 認証方式3Pの設定選択キー.
		}
		// 権限管理/カウンター管理キーの表示設定.
// comment 2017/10/31 【HCL-TANU】【IT5_4.2】【RQ686】【IT Defect AR1030817].
//		if( True == PnlComGetUserAuthentication()->getMidauthControl() ) {			// 権限管理/カウンター管理が設定されている場合.
	    if( True == PnlComGetUserAuthentication()->getMidauthControl(PNLC_NetworkLan::getInstance() ->getAdminNWLine( )) ) {			// 権限管理/カウンター管理が設定されている場合.
// comment 2014/12/02 OPS1-Sky白籏 [Ze5BK][503961] PKIカード認証かつ部門管理する場合、グレイアウト表示 Start.
//			if( 1 == sc_UserAttest ){												// 認証方式が外部サーバ認証の場合.
			if(( 1 == sc_UserAttest ) && !(( True == PLBC_FuncPKICheck::isPKI() ) && ( True == b_Manage ))){
// comment 2014/12/02 OPS1-Sky白籏 [Ze5BK][503961] PKIカード認証かつ部門管理する場合、グレイアウト表示 End.
				Key_AdminUserAndDepartSynchroErieManagement.activate( True );		// 権限管理/カウンター管理キー活性化.
				Btx_AdminUserAndDepartSynchroErieManagement.activate( True );		// 権限管理/カウンター管理設定内容活性化.
				// 権限管理/カウンター管理設定内容の変更
				Btx_AdminUserAndDepartSynchroErieManagement.changeText( ( b_AuthorityManagement == True )? s_KEYManage : s_KEYNotManage );
			} else {																	// 認証方式が外部サーバ認証以外の場合.
				Key_AdminUserAndDepartSynchroErieManagement.activate( False );		// 権限管理/カウンター管理キーグレーアウト.
				Btx_AdminUserAndDepartSynchroErieManagement.activate( False );		// 権限管理/カウンター管理設定内容グレーアウト.
			}
		} else {																		// 権限管理/カウンター管理が設定されていない場合.
			Key_AdminUserAndDepartSynchroErieManagement.showEnable( False );		// 権限管理/カウンター管理キー非表示.
			Btx_AdminUserAndDepartSynchroErieManagement.showEnable( False );		// 権限管理/カウンター管理設定内容非表示.
		}

// Comment 2018/01/29 HCLT_Hanuman [IT5_4.2][AR1038061] To improve performance.
////Comment 2018/01/10 [HCL-KanuriVenkata] [IT5 4.2][AR1033051] When VLAN Enabled hide Zero Balance Job Execution
////	if( ( ( TYPE_UserAuthenticationIntermeddiateServer == m_e_UserAuthZeroBillingJob )||( TYPE_UserAuthenticationDeviceAndMidAuth  == m_e_UserAuthZeroBillingJob ) ) && ( True == PLBC_FuncVersionCheck::isOK_IT5_Type1() ) ) {
//		if( ( ( TYPE_UserAuthenticationIntermeddiateServer == m_e_UserAuthZeroBillingJob )||( TYPE_UserAuthenticationDeviceAndMidAuth  == m_e_UserAuthZeroBillingJob ) ) 
//			&& ( True == PLBC_FuncVersionCheck::isOK_IT5_Type1() )  && ( False == APIC_VlanSetting::newInstance()->getVlanEnable() ) ) {
		if( ( ( TYPE_UserAuthenticationIntermeddiateServer == m_e_UserAuthZeroBillingJob )||( TYPE_UserAuthenticationDeviceAndMidAuth  == m_e_UserAuthZeroBillingJob ) ) 
			&& ( True == PLBC_FuncVersionCheck::isOK_IT5_Type1() )  && ( False == PNLC_NetworkLan::getInstance()->getVlanEnableWrapper() ) ) {
			Key_AdminUserAndDepartSynchroZeroBillingJob.showEnable( True );
			Btx_AdminUserAndDepartSynchroZeroBillingJob.showEnable( True );
			Btx_AdminUserAndDepartSynchroZeroBillingJob.changeText( ( m_e_ZeroBillingJobEnable == TYPE_FreeJobSettingEnable )? s_KEYZeroBillingJobAllow : s_KEYZeroBillingJobNotAllow );
		} else {
			Key_AdminUserAndDepartSynchroZeroBillingJob.showEnable( False );
			Btx_AdminUserAndDepartSynchroZeroBillingJob.showEnable( False );
		}
		if( True == PLBC_FuncVersionCheck::isOK_IT5_Type1() ) {
// comment 2017/02/27 SCD2-Sky延原 [IT5_4.0][AR1005533] NFC使用設定/NFC認証設定パーツの表示.
//			Key_AdminUserAndDepartSynchroNFCAuthenticationSetting.showEnable( True );		// NFC認証設定 key.
//			Btx_AdminUserAndDepartSynchroNFCAuthenticationSetting.showEnable( True );		// Show display area of NFC auth setting.
//// comment 2015/2/12 OPS1-Sky工藤 [ZeusS][CQ509844].	
////// comment 2014/11/25 OPS1-Sky工藤 [ZeusS][UV-01446].
//////			Btx_AdminUserAndDepartSynchroNFCAuthenticationSetting.changeText( (m_e_NFCAuthenticationSettingEnable == TYPE_NFCAuth_ON)? s_KEYUse_02 : s_KEYNotUse_02 );
////			Btx_AdminUserAndDepartSynchroNFCAuthenticationSetting.changeText( ( PnlComGetNFCSetting()->getUseNFC() == TYPE_NFCUse_ON)? s_KEYUse_02 : s_KEYNotUse_02 );
//			Btx_AdminUserAndDepartSynchroNFCAuthenticationSetting.changeText( ( onlyPanel.getNFCUse() == TYPE_NFCUse_ON)? s_KEYUse_02 : s_KEYNotUse_02 );
			Btx_AdminUserAndDepartSynchroNFCUseSetting.changeText( ( TYPE_NFCUse_ON == m_e_NFCUse ) ? s_KEYUse_02 : s_KEYNotUse_02 );		// NFC使用設定、設定内容表示.
			Rbt_AdminUserAndDepartSynchroNFCUseSetONOFF.showEnable( False );		// NFC使用設定、選択キー.
			if ( DEF_AdminNFCUseSetting == sc_KeySelect ) {		// NFC使用設定キー選択時、設定選択用キーを表示.
				Rbt_AdminUserAndDepartSynchroNFCUseSetONOFF.select( ( TYPE_NFCUse_ON == m_e_NFCUse ) ? 0 : 1 );	// 使用する／使用しない.
				Rbt_AdminUserAndDepartSynchroNFCUseSetONOFF.showEnable( True );
				Bak_UtilityMenuSettingBackground.showEnable( True );
			}
			Key_AdminUserAndDepartSynchroNFCUseSetting.showEnable( True );			// NFC使用設定キー.
			Btx_AdminUserAndDepartSynchroNFCUseSetting.showEnable( True );			// NFC使用設定、設定内容表示領域背景.

			Key_AdminUserAndDepartSynchroNFCAuthSetting.activate( False );			// NFC認証設定キー.
			Txt_AdminUserAndDepartSynchroNFCAuthSetting.showEnable( False );		// NFC認証設定、設定内容表示領域.
			Btx_AdminUserAndDepartSynchroNFCAuthSetting.activate( False );			// NFC認証設定、設定内容表示領域背景.
			Rbt_AdminUserAndDepartSynchroNFCAuthSetONOFF.showEnable( False );		// NFC認証設定、選択キー.
			if ( TYPE_NFCUse_ON == m_e_NFCUse ) {
				Key_AdminUserAndDepartSynchroNFCAuthSetting.activate( True );			// NFC認証設定キー.
				Txt_AdminUserAndDepartSynchroNFCAuthSetting.showEnable( True );		// NFC認証設定、設定内容表示領域.
				Btx_AdminUserAndDepartSynchroNFCAuthSetting.activate( True );			// NFC認証設定、設定内容表示領域背景.
				Btx_AdminUserAndDepartSynchroNFCAuthSetting.changeText( ( TYPE_NFCAuth_ON == m_e_NFCAuth ) ? s_KEYUse_02 : s_KEYNotUse_02 );		// NFC使用設定、設定内容表示.
				if ( DEF_AdminNFCAuthSetting == sc_KeySelect ) {		// NFC認証設定キー選択時、設定選択用キーを表示.
					Rbt_AdminUserAndDepartSynchroNFCAuthSetONOFF.select( ( TYPE_NFCAuth_ON == m_e_NFCAuth ) ? 0 : 1 );	// 使用する／使用しない.
					Rbt_AdminUserAndDepartSynchroNFCAuthSetONOFF.showEnable( True );
					Bak_UtilityMenuSettingBackground.showEnable( True );
				}
			}
			Key_AdminUserAndDepartSynchroNFCAuthSetting.showEnable( True );			// NFC認証設定キー.
			Btx_AdminUserAndDepartSynchroNFCAuthSetting.showEnable( True );			// NFC認証設定、設定内容表示領域背景.
		} else {
// comment 2017/02/27 SCD2-Sky延原 [IT5_4.0][AR1005533].
//			Key_AdminUserAndDepartSynchroNFCAuthenticationSetting.showEnable( False );		// NFC認証設定 key.
//			Btx_AdminUserAndDepartSynchroNFCAuthenticationSetting.showEnable( False );		// Hide display area of NFC auth setting.
			Key_AdminUserAndDepartSynchroNFCUseSetting.showEnable( False );			// NFC使用設定キー.
			Btx_AdminUserAndDepartSynchroNFCUseSetting.showEnable( False );			// NFC使用設定、設定内容表示領域背景.
			Rbt_AdminUserAndDepartSynchroNFCUseSetONOFF.showEnable( False );		// NFC使用設定選択キー.
			Key_AdminUserAndDepartSynchroNFCAuthSetting.showEnable( False );		// NFC使用設定キー.
			Btx_AdminUserAndDepartSynchroNFCAuthSetting.showEnable( False );		// NFC使用設定、設定内容表示領域背景.
			Rbt_AdminUserAndDepartSynchroNFCAuthSetONOFF.showEnable( False );		// NFC使用設定選択キー.
		}
//		Rtb_AdminUserAndDepartSynchroNFCAuthenticationSetting.showEnable( False );
//		if( DEF_AdminNFCAuthenticationSetting == sc_KeySelect ){		// NFC認証設定 key selected.
//			Bak_UtilityMenuSettingBackground.showEnable( True );
//			Rtb_AdminUserAndDepartSynchroNFCAuthenticationSetting.select( (m_e_NFCAuthenticationSettingEnable == TYPE_NFCAuth_ON)? 0 : 1 );	// 管理する／管理しない.
//			Rtb_AdminUserAndDepartSynchroNFCAuthenticationSetting.showEnable( True );
//		}
		// 右側表示処理.
		// 先に右側表示のパーツを非表示とする.
		Rtb_AdminUserAndDepartSynchroErieManageKey.showEnable( False );				// 権限管理/カウンター管理の設定キー・非表示.		//DonauLastFum【Erie拡張】【13笹岡】20090518 add.
// comment 2017/02/27 SCD2-Sky延原 [IT5_4.0][AR1005533] 
//		Bak_UtilityMenuSettingBackground.showEnable( False );						// 右設定エリアユーティリティグルーピングタイトル.
//		Rtb_AdminUserAndDepartSynchroNFCAuthenticationSetting.showEnable( False );
//		if( DEF_AdminNFCAuthenticationSetting == sc_KeySelect ) {		// NFC認証設定 key selected.
//			Bak_UtilityMenuSettingBackground.showEnable( True );
//			Rtb_AdminUserAndDepartSynchroNFCAuthenticationSetting.select( ( m_e_NFCAuthenticationSettingEnable == TYPE_NFCAuth_ON )? 0 : 1 );	// 管理する／管理しない.
//			Rtb_AdminUserAndDepartSynchroNFCAuthenticationSetting.showEnable( True );
//		}
//		Rbt_AdminUserAndDepartSynchroBluetoothLEAuthSetONOFF.showEnable( False );
		// 右エリア「権限管理/カウンター管理」の設定値表示.
		if( 9 == sc_KeySelect ) {	// 権限管理/カウンター管理・選択中.
			Bak_UtilityMenuSettingBackground.showEnable( True );											// 右設定エリアユーティリティグルーピングタイトル.
			Rtb_AdminUserAndDepartSynchroErieManageKey.select( ( b_AuthorityManagement == True )? 0 : 1 );	// 管理する／管理しない.
			Rtb_AdminUserAndDepartSynchroErieManageKey.showEnable( True );									// 権限管理/カウンター管理の設定キー・表示.
			// 外部サーバー未登録の場合、管理しないキーをグレーアウト.
			Bool b_OutsideServer = False;
			if( apicAuthSrv != NULL ) {
// comment [2016/12/09] [PDD22山下(日立)] [IT5_4.0][AccuRev.1008785]サーバーグループ有効時はデフォルトサーバーグループを判定する.
				if( True == apicAuthSrv->checkServerGroup() ) {
//					if( APID_AuthenticationServerIndexNone != apicAuthSrv->getDefaultServerGroupIndex() ) {			// 外部サーバー登録.
					if( APID_AuthenticationServerIndexNone != apicAuthSrv->getDefaultServerGroupIndex( PNLC_NetworkLan::getInstance()->getAdminNWLine( ) ) ) {			// 外部サーバー登録.
						b_OutsideServer = True;
					}
				} else {
					if( APID_AuthenticationServerIndexNone != apicAuthSrv->getDefaultServerIndex() ) {			// 外部サーバー登録.
						b_OutsideServer = True;
					}
				}
			}
			if( b_OutsideServer == True ) {																	// 外部サーバー登録あり.
				Key_AdminUserAndDepartSynchroErieNotManage.activate( True );								// 管理しないキー活性化.
			} else {																						// 外部サーバー未登録.
				Rtb_AdminUserAndDepartSynchroErieManageKey.select( 0 );										// 管理する選択.
				Key_AdminUserAndDepartSynchroErieNotManage.activate( False );								// 管理しないキーグレーアウト.
			}
		}
		Rtb_AdminUserAndDepartSynchroZeroBillingJobKey.showEnable( False );
		if( DEF_ZeroBillingChargeDisp == sc_KeySelect ) {
			Bak_UtilityMenuSettingBackground.showEnable( True );											// 右設定エリアユーティリティグルーピングタイトル.
//			Rtb_AdminUserAndDepartSynchroZeroBillingJobKey.select( (m_e_ZeroBillingJobEnable == TYPE_FreeJobSettingEnable)? 0 : 1 );	// 管理する／管理しない.
			Rtb_AdminUserAndDepartSynchroZeroBillingJobKey.select( ( m_e_ZeroBillingJobEnable == TYPE_FreeJobSettingDisable )? 0 : 1 );	// 実行不可／実行可能ジョブのみ.
			Rtb_AdminUserAndDepartSynchroZeroBillingJobKey.showEnable( True );
		}
		Bool b_ActiveAuthCardSearchSetting;
		ushort us_TextID;
		Bool b_EnabeAuthCardSearchSetting = False;
		if( ( TYPE_AUTH_DEVICE_ICC          == PnlComGetAuthDeviceSetting()->getAuthDeviceType() )	// ICカード認証.
		    ||( TYPE_AUTH_DEVICE_LOADABLE_ICC == PnlComGetAuthDeviceSetting()->getAuthDeviceType() ) ) {	// ローダブルデバイスドライバーICカード認証.
			b_EnabeAuthCardSearchSetting = True;
		}
		if( ( 1 == sc_UserAttest ) || ( 4 == sc_UserAttest ) ) {			// // ユーザー認証が外部サーバーか外部サーバー＋中間サーバーか本体＋外部サーバー認証の時.
			// 認証カード検索設定キーを活性化.
			b_ActiveAuthCardSearchSetting = True;
		}
		else {
			// 認証カード検索設定キーを非活性化(グレイアウト).
			b_ActiveAuthCardSearchSetting = False;
		}
		if( False == b_AuthCardServerSearching ) {
			// 使用しない.
			us_TextID = s_KEYNotUse_02;
		}
		else {
			// 使用する.
			us_TextID = s_KEYUse_02;
		}
		Key_AdminUserAndDepartSynchroAuthCardSearchSetting.showEnable( b_EnabeAuthCardSearchSetting );
		Btx_AdminUserAndDepartSynchroAuthCardSearchSetting.showEnable( b_EnabeAuthCardSearchSetting );
		Key_AdminUserAndDepartSynchroAuthCardSearchSetting.activate( b_ActiveAuthCardSearchSetting );
		Btx_AdminUserAndDepartSynchroAuthCardSearchSetting.activate( b_ActiveAuthCardSearchSetting );
		Btx_AdminUserAndDepartSynchroAuthCardSearchSetting.changeText( us_TextID );
		Btx_AdminUserAndDepartSynchroAuthCardSearchSetting.showEnableAdd( b_ActiveAuthCardSearchSetting );
// comment 2017/02/27 SCD2-Sky延原 [IT5_4.0][AR1005533] Bluetooth LE使用/認証設定は、4Pへ.
//		APIC_SystemInfo* pc_SystemInfo = APIC_SystemInfo::newInstance() ;
//		if( ( False == PLBC_FuncVersionCheck::isOK_IT5_Type1() )
//		    ||	( NULL == pc_SystemInfo )
//		    || ( TYPE_LocalIFKit_OFF == pc_SystemInfo->getLocalIFKit() ) ) {
//			Btx_AdminUserAndDepartSynchroBluetoothLEAuthSetting.showEnable( False );
//			Key_AdminUserAndDepartSynchroBluetoothLEAuthSetting.showEnable( False );
//			Rbt_AdminUserAndDepartSynchroBluetoothLEAuthSetONOFF.showEnable( False );
//		} else {
//// comment 2014/11/25 OPS1-Sky工藤 [ZeusS][UV-01446].
//// comment 2015/01/08 【HCL-Rengaraaj】【CQ523700】 Enable BluetoothLE Use/Do not use text handling.
////			APIC_BLESetting* pc_BLESetting = APIC_BLESetting::newInstance();
////			if ( NULL != pc_BLESetting ) {
////			if ( TYPE_BLEAuth_ON == m_e_BLEAuth) {
////				if ( TYPE_BLEUse_ON == pc_BLESetting->getUseBLE() ) {
//			if ( onlyPanel.getBLEUse() == TYPE_BLEUse_ON ) {
//				Btx_AdminUserAndDepartSynchroBluetoothLEAuthSetting.changeText( s_KEYUse_02 );
//			} else {
//				Btx_AdminUserAndDepartSynchroBluetoothLEAuthSetting.changeText( s_KEYNotUse_02 );
//			}
////			}
////			if( 11 == sc_KeySelect ){	// BluetoothLE・選択中.
//			if( 12 == sc_KeySelect ) {	// BluetoothLE・選択中.
//				Bak_UtilityMenuSettingBackground.showEnable( True );											// 右設定エリアユーティリティグルーピングタイトル.
//// comment 2015/01/08 【HCL-Rengaraaj】【CQ523700】 Enable BluetoothLE Use/Do not use text handling.
////				if( TYPE_BLEAuth_ON == m_e_BLEAuth ) {
//				if( onlyPanel.getBLEAuth() == TYPE_BLEAuth_ON ) {
//					Rbt_AdminUserAndDepartSynchroBluetoothLEAuthSetONOFF.select( 0 );
//				} else {
//					Rbt_AdminUserAndDepartSynchroBluetoothLEAuthSetONOFF.select( 1 );
//				}
//				Rbt_AdminUserAndDepartSynchroBluetoothLEAuthSetONOFF.showEnable( True );
//			}
//			Key_AdminUserAndDepartSynchroBluetoothLEAuthSetting.showEnable( True );
//			Btx_AdminUserAndDepartSynchroBluetoothLEAuthSetting.showEnable( True );
//		}
// comment 2016/5/25 [AUTHFUM2] [RQ-526]外部サーバーLDAP検索高速化 第22製品開発 山本.
		// ↑ 3ページ目の処理
		// ↓ 4ページ目の処理
	} else {
//		Bak_UtilityMenuSettingBackground.showEnable( True );
		Grp_AdminUserAndDepartSynchroGraP4.showEnable( True );  // ページ4を表示.
// comment 2017/02/27 SCD2-Sky延原 [IT5_4.0][AR1005533] 
		// 先に右側表示のパーツ背景を非表示とする.
		Bak_UtilityMenuSettingBackground.showEnable( False );						// 右設定エリアユーティリティグルーピングタイトル.
		// メッセージの変更.
		if( sc_KeySelect <= 12 ) { // 未選択状態.
			Txt_MessageSettingValue.changeText( s_MSGChangeSettingValue_a );			//「項目を選択し、設定を変更できます」を表示.
			Rtb_AdminUserAndDepartSynchro4p.select( - 1 );								// 認証方式4Pの設定選択キー.
		}else {
			Txt_MessageSettingValue.changeText( s_MSGSelectSettingValue_02 );
			Rtb_AdminUserAndDepartSynchro4p.select( sc_KeySelect - 13 ); 				// 認証方式4Pの設定選択キー.
		}

// comment 2017/02/27 SCD2-Sky延原 [IT5_4.0][AR1005533] // Bluetooth LE使用設定/認証設定.
		APIC_SystemInfo* pc_SystemInfo = APIC_SystemInfo::newInstance() ;
		if( ( False == PLBC_FuncVersionCheck::isOK_IT5_Type1() )
		    ||	( NULL == pc_SystemInfo )
		    || ( TYPE_LocalIFKit_OFF == pc_SystemInfo->getLocalIFKit() ) ) {
			Key_AdminUserAndDepartSynchroBLEUseSetting.showEnable( False );
			Btx_AdminUserAndDepartSynchroBLEUseSetting.showEnable( False );
			Rbt_AdminUserAndDepartSynchroBLEUseSetONOFF.showEnable( False );
			Key_AdminUserAndDepartSynchroBLEAuthSetting.showEnable( False );
			Btx_AdminUserAndDepartSynchroBLEAuthSetting.showEnable( False );
			Rbt_AdminUserAndDepartSynchroBLEAuthSetONOFF.showEnable( False );
		} else {
			Btx_AdminUserAndDepartSynchroBLEUseSetting.changeText( ( TYPE_BLEUse_ON == m_e_BLEUse ) ? s_KEYUse_02 : s_KEYNotUse_02 );		// Bluetooth LE使用設定、設定内容表示.
			Rbt_AdminUserAndDepartSynchroBLEUseSetONOFF.showEnable( False );		// Bluetooth LE使用設定、選択キー.

			if( DEF_AdminBLEUseSetting == sc_KeySelect ) {	// BluetoothLE・選択中.
				Rbt_AdminUserAndDepartSynchroBLEUseSetONOFF.select( ( TYPE_BLEUse_ON == m_e_BLEUse ) ? 0 : 1 );	// 使用する／使用しない.
				Rbt_AdminUserAndDepartSynchroBLEUseSetONOFF.showEnable( True );
				Bak_UtilityMenuSettingBackground.showEnable( True );											// 右設定エリアユーティリティグルーピングタイトル.
			}
			Key_AdminUserAndDepartSynchroBLEUseSetting.showEnable( True );			// Bluetooth LE使用設定キー.
			Btx_AdminUserAndDepartSynchroBLEUseSetting.showEnable( True );			// Bluetooth LE使用設定、設定内容表示領域背景.

			Key_AdminUserAndDepartSynchroBLEAuthSetting.activate( False );			// Bluetooth LE認証設定キー.
			Txt_AdminUserAndDepartSynchroBLEAuthSetting.showEnable( False );		// Bluetooth LE認証設定、設定内容表示領域.
			Btx_AdminUserAndDepartSynchroBLEAuthSetting.activate( False );			// Bluetooth LE認証設定、設定内容表示領域背景.
			Rbt_AdminUserAndDepartSynchroBLEAuthSetONOFF.showEnable( False );		// Bluetooth LE認証設定、選択キー.
			if ( TYPE_BLEUse_ON == m_e_BLEUse ) {
				Key_AdminUserAndDepartSynchroBLEAuthSetting.activate( True );			// Bluetooth LE認証設定キー.
				Txt_AdminUserAndDepartSynchroBLEAuthSetting.showEnable( True );			// Bluetooth LE認証設定、設定内容表示領域.
				Btx_AdminUserAndDepartSynchroBLEAuthSetting.activate( True );			// Bluetooth LE認証設定、設定内容表示領域背景.
				Btx_AdminUserAndDepartSynchroBLEAuthSetting.changeText( ( TYPE_BLEAuth_ON == m_e_BLEAuth ) ? s_KEYUse_02 : s_KEYNotUse_02 );		// Bluetooth LE使用設定、設定内容表示.
				if ( DEF_AdminBLEAuthSetting == sc_KeySelect ) {		// Bluetooth LE認証設定キー選択時、設定選択用キーを表示.
					Rbt_AdminUserAndDepartSynchroBLEAuthSetONOFF.select( ( TYPE_BLEAuth_ON == m_e_BLEAuth ) ? 0 : 1 );	// 使用する／使用しない.
					Rbt_AdminUserAndDepartSynchroBLEAuthSetONOFF.showEnable( True );
					Bak_UtilityMenuSettingBackground.showEnable( True );
				}
			}
			Key_AdminUserAndDepartSynchroBLEAuthSetting.showEnable( True );			// Bluetooth LE認証設定キー.
			Btx_AdminUserAndDepartSynchroBLEAuthSetting.showEnable( True );			// Bluetooth LE認証設定、設定内容表示領域背景.
		}

		// DNキャッシュキーの表示設定.
		APIC_DNCacheManager* apicDNCacheMan = APIC_DNCacheManager::newInstance();
		if( NULL == apicDNCacheMan ) {LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return;}
		// DNキャッシュがデバイス構成上設定利用不可時の場合、非表示.
// Comment 2017/11/22【HCLT-Tanu】【IT5 4.2】【RQ686】API Correction.		
//		if( apicDNCacheMan->getDNCache() == TYPE_DNCache_Invalid ) {
	    if( apicDNCacheMan->getDNCache(PNLC_NetworkLan::getInstance()->getAdminNWLine( )) == TYPE_DNCache_Invalid ) {
			Key_AdminUserAndDepartSynchroDNCache.showEnable( False );
			Btx_AdminUserAndDepartSynchroDNCache.showEnable( False );
		} else {
			// ユーザー認証が外部サーバーか外部サーバー＋中間サーバーか本体＋外部サーバー認証の時、活性化.
			if( ( 1 == sc_UserAttest ) || ( 4 == sc_UserAttest ) ) {
// comment 2017/02/27 SCD2-Sky延原 [IT5_4.0][AR1005533] 
				Ktx_AdminUserAndDepartSynchroDNCache.showEnable( True );
				Key_AdminUserAndDepartSynchroDNCache.activate( True );
				Btx_AdminUserAndDepartSynchroDNCache.activate( True );
			} else { // それ以外はグレーアウト.
// comment 2017/02/27 SCD2-Sky延原 [IT5_4.0][AR1005533] 
				Ktx_AdminUserAndDepartSynchroDNCache.showEnable( False );
				Key_AdminUserAndDepartSynchroDNCache.activate( False );
				Btx_AdminUserAndDepartSynchroDNCache.activate( False );
			}
		}
		
		// 拡張ユーザーDBの表示設定.	
		if( True == PnlComGetUser()->isEnableDataRegistrationExtension() ) {
// Comment 2018/02/08 【HCLT-Suganya】 【IT5 4.2】 【AR1037948】 拡張ユーザーDB key changed to shared from NW1/NW2 support as per changes in FS.
			if( True == PNLC_NetworkLan::getInstance()->getVlanEnableWrapper() ) {
				Key_AdminUserAndDepartSynchroAuthInsideLargeScale.changeGraphic(NPK_NM1_VLAN_X240_Y30,19, False );
			}
			Key_AdminUserAndDepartSynchroAuthInsideLargeScale.showEnable( True );
			Btx_AdminUserAndDepartSynchroAuthInsideLargeScale.showEnable( True );
			if( ( 2 == sc_UserAttest ) || ( 4 == sc_UserAttest ) ) {	// ユーザー認証が「本体装置認証」or「本体＋外部サーバー認証」の時、活性化.
// comment 2017/02/27 SCD2-Sky延原 [IT5_4.0][AR1005533] 
				Ktx_AdminUserAndDepartSynchroAuthInsideLargeScale.showEnable( True );
				Key_AdminUserAndDepartSynchroAuthInsideLargeScale.activate( True );
				Btx_AdminUserAndDepartSynchroAuthInsideLargeScale.activate( True );
			} else { 					// それ以外はグレーアウト.
// comment 2017/02/27 SCD2-Sky延原 [IT5_4.0][AR1005533] 
				Ktx_AdminUserAndDepartSynchroAuthInsideLargeScale.showEnable( False );
// Comment 2018/02/08 【HCLT-Suganya】 【IT5 4.2】 【AR1037948】 拡張ユーザーDB key changed to shared from NW1/NW2 support as per changes in FS.
				Key_AdminUserAndDepartSynchroAuthInsideLargeScale.changeGraphic(NPK_NM1_X240_Y30,19, False );
				Key_AdminUserAndDepartSynchroAuthInsideLargeScale.activate( False );
				Btx_AdminUserAndDepartSynchroAuthInsideLargeScale.activate( False );
			}
		} else {
			Key_AdminUserAndDepartSynchroAuthInsideLargeScale.showEnable( False );
			Btx_AdminUserAndDepartSynchroAuthInsideLargeScale.showEnable( False );
		}

		// 先に右側表示のパーツを非表示とする.
// comment 2017/02/27 SCD2-Sky延原 [IT5_4.0][AR1005533] 
//		Bak_UtilityMenuSettingBackground.showEnable( False );						// 右設定エリアユーティリティグルーピングタイトル.
		Rbt_AdminUserAndDepartSynchroDNCacheKey.showEnable( False );				// 外部サーバーDNキャッシュ：使用する/使用しないキー.
		Rbt_AdminUserAndDepartSynchroAuthInsideLargeScaleKey.showEnable( False );	// 拡張ユーザーDB：使用する/使用しないキー.

		Btx_AdminUserAndDepartSynchroDNCache.changeText( ( b_ExtServerDNCache == True )? s_KEYUse_02 : s_KEYNotUse_02 );					// 外部サーバーDNキャッシュ 設定値表示.
// Comment 2017/12/20 【HCLT-Sashmitha】 【IT5 4.2】 【AR1033226】 Message for Extended user DB has been changed as per FS.
//		Btx_AdminUserAndDepartSynchroAuthInsideLargeScale.changeText( ( b_AuthInsideLargeScale == True )? s_KEYUse_02 : s_KEYNotUse_02 );	// 拡張ユーザーDB 設定値表示.
		Btx_AdminUserAndDepartSynchroAuthInsideLargeScale.changeText( ( b_AuthInsideLargeScale == True )? s_KEYPermission_02 : s_pKEYNotPermission_W268H036A );	// 拡張ユーザーDB 設定値表示.
		
		if( 13 == sc_KeySelect ){			// 外部サーバーDNキャッシュ 選択中.
			Bak_UtilityMenuSettingBackground.showEnable( True );						// 右設定エリアユーティリティグルーピングタイトル.
			Rbt_AdminUserAndDepartSynchroDNCacheKey.select( ( b_ExtServerDNCache == True )? 0 : 1 );
			Rbt_AdminUserAndDepartSynchroDNCacheKey.showEnable( True );
		} else if( 14 == sc_KeySelect ) {	// 拡張ユーザーDB 選択中.
			Bak_UtilityMenuSettingBackground.showEnable( True );						// 右設定エリアユーティリティグルーピングタイトル.
			Rbt_AdminUserAndDepartSynchroAuthInsideLargeScaleKey.select( ( b_AuthInsideLargeScale == True )? 0 : 1 );
			Rbt_AdminUserAndDepartSynchroAuthInsideLargeScaleKey.showEnable( True );
		}
	}
	//comment 2017/09/11 [IT5_4.2][RQ-686] Multiple Network support: NW2 not Supported option removal.
	TYPE_NetworkSpecific e_NetworkSpecific =  PNLC_NetworkLan::getInstance()->getAdminNWLine();
	if (TYPE_NetworkSpecific_NW2 == e_NetworkSpecific) {
		Key_AdminUserAndDepartSynchroDepartManage.showEnable( False );				// Account Track.
		Btx_AdminUserAndDepartSynchroDepartManage.showEnable( False );
		Key_AdminUserAndDepartSynchroDepartForm.showEnable( False );				// Account Track Input Method.
		Btx_AdminUserAndDepartSynchroDepartForm.showEnable( False );
		Key_AdminUserAndDepartSynchroUserDepartAttest.showEnable( False );			// Synchronize User Authentication & Account Track
		Btx_AdminUserAndDepartSynchroUserDepartAttest.showEnable( False );
//Comment 2017/11/29 [HCL-Catherine] [IT5 4.2][RQ-686] [AR1033184]Multiple Network support to hide counter allocation number.
	 	Key_AdminUserAndDepartSynchroCoutControlI.showEnable( False );			    // counter allocation number.
		Btx_AdminUserAndDepartSynchroCoutControlI.showEnable( False );
		Key_AdminUserAndDepartSynchroPullScanFiery.showEnable( False );				// Scanner Setting.
		Btx_AdminUserAndDepartSynchroPullScanFiery.showEnable( False );
		Key_AdminUserAndDepartSynchroErieManagement.showEnable( False );			// User Permission Counter Settings.
		Btx_AdminUserAndDepartSynchroErieManagement.showEnable( False );
		Key_AdminUserAndDepartSynchroNFCUseSetting.showEnable( False );				// Enable NFC.
		Btx_AdminUserAndDepartSynchroNFCUseSetting.showEnable( False );
		Key_AdminUserAndDepartSynchroNFCAuthSetting.showEnable( False );			// NFC Authentication Setting.
		Btx_AdminUserAndDepartSynchroNFCAuthSetting.showEnable( False );
//Comment 2017/11/29 [HCL-Catherine] [IT5 4.2][RQ-686] [AR1033184]Multiple Network support to hide Zero Balance Job Execution.
//Comment 2018/01/10 [HCL-KanuriVenkata] [IT5 4.2][AR1033051] When VLAN  Enabled  hide Zero Balance Job Execution.
//		Key_AdminUserAndDepartSynchroZeroBillingJob.showEnable( False );            //Zero Balance Job Execution.
//		Btx_AdminUserAndDepartSynchroZeroBillingJob.showEnable( False );
		Key_AdminUserAndDepartSynchroBLEUseSetting.showEnable( False );				// Enable Bluetooth LE.
		Btx_AdminUserAndDepartSynchroBLEUseSetting.showEnable( False );
		Key_AdminUserAndDepartSynchroBLEAuthSetting.showEnable( False );			// Bluetooth LE Authentication Setting.
		Btx_AdminUserAndDepartSynchroBLEAuthSetting.showEnable( False );
//Comment 2017/11/29 [HCLT-kanurivenkata] [IT5 4.2][AR1037984]Enable Extended User DB key for NW2
//Comment 2017/11/29 [HCL-Catherine] [IT5 4.2][RQ-686] [AR1033184]Multiple Network support to hide Extended User DB key.
//      Key_AdminUserAndDepartSynchroAuthInsideLargeScale.showEnable( False );      //Extended User DB.
//      Btx_AdminUserAndDepartSynchroAuthInsideLargeScale.showEnable( False );
	}
}
//////////////////////////////////////////////////////////////
// 概要		:管理者設定・ユーザ認証/部門管理・ユーザ/部門の連動認証 : 中間サーバー認証警告チェック.
// 戻り値	:True:Warning要、False：Warning不要.
//////////////////////////////////////////////////////////////
Bool PAdminUserAndDepartSynchro::CheckMiddleServerWarning()
{
//	if(sc_UserAttest != 3) return False; // 中間サーバー認証のみ有効.
	if( ( sc_UserAttest != 3 ) && ( sc_UserAttest != 5 ) ) { return False; } // 中間サーバー認証のみ有効.
	// OpenAPI設定.
	APIC_OpenApiSetting* apicOpenApi = PnlComGetOpenApiSetting();
// Comment 2017/12/19 [HCL-Catherine] [IT5 4.2][RQ-686] [ WP1377]The new API is used and to detect the current network type, new API arguments are passed.
	TYPE_NetworkSpecific e_NetworkSpecific = PNLC_NetworkLan::getInstance()->getAdminNWLine();
	bOpenAPI_NG = True;
	if( apicOpenApi ) {
//		if( apicOpenApi->getEnableOpenAPI() ) {
		if( apicOpenApi->getEnableOpenAPI( e_NetworkSpecific ) ) {
			// 設定有効.
			bOpenAPI_NG = False;
		}
	}
	// ボックス管理者/パブリックユーザー使用設定.
	bBoxAdminPublicUser_NG = True;
	// パブリックユーザ使用設定.
	switch( ss_PublicUser ) {
		case 1:		// 許可.
		case 2:		// 許可後ログイン.
			// 使用許可.
			bBoxAdminPublicUser_NG = False;
			break;
		default:
			break;
	}
	if( bBoxAdminPublicUser_NG ) {
		// ボックス管理者情報をチェック.
		APIC_UserAuthentication* apicUserAuth = PnlComGetUserAuthentication();
		if( apicUserAuth ) {
			// ボックス管理者設定.
			if( apicUserAuth->getEnableBoxAdmin() ) {
				// 設定有効.
				bBoxAdminPublicUser_NG = False;
			}
		}
	}
	// 生体認証装置設定.
	if( bOpenAPI_NG || bAuthDeviceBody_NG || bBoxAdminPublicUser_NG ) {
		return True;
	}
	return False;
}
//////////////////////////////////////////////////////////////
// 概要		:管理者設定・ユーザ認証/部門管理・ユーザ/部門の連動認証 : 外部サーバー認証警告チェック.
// 戻り値	:True:Warning要、False：Warning不要.
//////////////////////////////////////////////////////////////
Bool PAdminUserAndDepartSynchro::CheckOutsideServerWarning()
{
	if( sc_UserAttest != 1 ) { return False; } // 外部サーバー認証のみ有効.
	if( True == b_AuthCardServerSearching ) {	// 認証検索機能がONの時.
		// デバイスタイプは必ずICカード認証かローダブルデバイスドライバーICカード認証なので警告無しにしておく.
		bAuthDeviceBody_NG = False;
// comment 2017/03/27 MSE岡田 [IT5_4.0][1010035] カード認証装置装着時に権限管理/カウンター項目が「管理する」であれば、警告ありを返す.
//	} else if( True == b_AuthorityManagement ) {	// Erie拡張時
//		if( ( TYPE_AUTH_DEVICE_BIO          == PnlComGetAuthDeviceSetting()->getAuthDeviceType() )	// 生体認証.
//		    ||( TYPE_AUTH_DEVICE_BIO2         == PnlComGetAuthDeviceSetting()->getAuthDeviceType() ) ) {	// 生体認証2.
//			bAuthDeviceBody_NG = True;
//		} else {
//			bAuthDeviceBody_NG = False;
//		}
	} else {									// 認証検索機能がOFFの時.
		// 生体認証装置設定.
		if( ( TYPE_AUTH_DEVICE_BIO          == PnlComGetAuthDeviceSetting()->getAuthDeviceType() )	// 生体認証.
		    ||( TYPE_AUTH_DEVICE_BIO2         == PnlComGetAuthDeviceSetting()->getAuthDeviceType() )	// 生体認証2.
		    ||( TYPE_AUTH_DEVICE_ICC          == PnlComGetAuthDeviceSetting()->getAuthDeviceType() )	// ICカード認証.
		    ||( TYPE_AUTH_DEVICE_LOADABLE_ICC == PnlComGetAuthDeviceSetting()->getAuthDeviceType() ) ) {	// ローダブルデバイスドライバーICカード認証.
			// 生体認証装置設定時は中間認証サーバー設定NG.// 061109 ICカード認証時も同様.
			bAuthDeviceBody_NG = True;
		} else {
			bAuthDeviceBody_NG = False;
		}
	}
	if( True				 == APIC_SoftDipSw::newInstance()->getSoftDipSwBit( 38, 2 ) 		// DipSW29(bit7)がON.
	    || True					 == APIC_SoftDipSw::newInstance()->getSoftDipSwBit( 29, 6 ) 		// DipSW37(bit1)がON.
	    || True					 == APIC_SoftDipSw::newInstance()->getSoftDipSwBit( 36, 4 )			// DipSW37(bit5)がON.
	    || True					 == APIC_SoftDipSw::newInstance()->getSoftDipSwBit( 29, 7 ) ) {		// DipSW37(bit2)がON.

		bAuthDeviceBody_NG = False;
	}

	Bool bl_SpRomMode = PnlComIsUseCachedUserInfoScanToHome();
	if( True == bl_SpRomMode ) {
		bAuthDeviceBody_NG = False;
	}

	return bAuthDeviceBody_NG;
}

//================================================================================================
// 概要		:管理者設定・ユーザ認証/部門管理・ユーザ/部門の連動認証・使用管理ﾃﾞｰﾀｸﾘｱ : コールバック関数.
// 戻り値	:なし.
//================================================================================================
void ans_setDataClearAll( int32_t i32_Result )
{
	if( i32_Result != APID_DocapiMethodOk ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); }	//削除失敗.
	CallBackIndi( TYPE_PNL_CallBack_DataClearAll );
}
//================================================================================================
// 概要		:Touch&Go削除CallBack.
// 戻り値	:なし.
//================================================================================================
void ans_AdminTouchAndGoDelete( int32_t i32_ret )
{
	if( APID_DocapiMethodOk != i32_ret ) {
		//削除失敗
		CallBackIndi( TYPE_PNL_CallBack_TouchAndGoDeleteNG );
	} else {
		//確認要求完了
		CallBackIndi( TYPE_PNL_CallBack_TouchAndGoDeleteOK );
	}
}
//================================================================================================
// 概要		:Touch&Go削除(ユーザ用)CallBack.
//================================================================================================
void ans_AdminTouchAndGoDeleteUser( int32_t i32_ret )
{
	CallBackIndi( TYPE_PNL_CallBack_TouchAndGoDelete );	// ユーザ削除時、Touch&Go文書の削除完了通知.
	if( APID_DocapiMethodOk != i32_ret ) {
		SYSF_logMsg( "ans_AdminTouchAndGoDeleteUser == NG \n",1,2,3,4,5,6 );
	}
}
//================================================================================================
// 概要		:暗号PDF削除(ユーザ用)CallBack.
//================================================================================================
void ans_AdminDocByUserDeleteUser( int32_t i32_ret,int32_t i32_ret2,int32_t i32_ret3 )
{
	PanelLog( PNLD_LogKind_Admin, "ans_AdminDocByUserDeleteUser: P1[%d] P2[%d] P3[%d]", i32_ret, i32_ret2, i32_ret3, 0, 0, 0 );
	CallBackIndi( TYPE_PNL_CallBack_PDFBoxDelete );	// ユーザ削除時、暗号化PDF文書の削除完了通知.
	if( APID_DocapiMethodOk != i32_ret || APID_DocapiMethodOk != i32_ret2 || APID_DocapiMethodOk != i32_ret3 ) {
		SYSF_logMsg( "ans_AdminDocByUserDeleteUser == NG i32_ret=[%d],i32_ret2=[%d],i32_ret3=[%d] \n",i32_ret,i32_ret2,i32_ret3,4,5,6 );
	}
}

//================================================================================================.
// 概要		:管理者設定・ユーザ認証/部門管理・ユーザ/部門の連動認証・使用管理ﾃﾞｰﾀｸﾘｱ : コンストラクタ.
// 戻り値	:なし.
// 履歴		: 2016/01/08 SecV2-6 ボックス共有化キー非表示条件追加対応 Sky 新道.
//================================================================================================.
PAdminUserAndDepartDataClear::PAdminUserAndDepartDataClear( PDisplayParts** pc_partsTable, short ss_partsNumber, Bool b_isNoWarningGamen )
		: PAdminUserAndDepartSynchro( pc_partsTable, ss_partsNumber, b_isNoWarningGamen )
		, bl_DispModeBoxProcess( False )	// ボックスの処理方法を選択させるモードフラグ.
		, bl_DeleteBox( False )				// ボックス削除フラグ.
		, m_bl_UbqStorageOn( False )		// ユビキタスプリントストレージ設定.
		, m_bl_UbqClientOn( False )			// ユビキタスプリントクライアント設定.
		, m_b_PublicShow( True )			// ボックス共有化表示フラグ.
		, uc_KeySelect( 0 )					// キー選択.
{
}

//================================================================================================.
// 概要		:管理者設定・ユーザ認証/部門管理・ユーザ/部門の連動認証・使用管理ﾃﾞｰﾀｸﾘｱ : ボタン（押された.
// 戻り値	:なし.
//================================================================================================.
void PAdminUserAndDepartDataClear::procGoToPage( uchar option )
{
// Comment 2017/09/07【HCLT-Narsimhmaswamy】【IT5 4.2】【RQ686】【WP595】Multiple Network support.
	TYPE_NetworkSpecific e_NWType=  PNLC_NetworkLan::getInstance() ->getAdminNWLine( );
	APIC_User* pc_UserBoxAdmin = APIC_User::newInstance( TYPD_IndexUserBoxAdmin );	// Ｂｏｘ管理者のパスワードをデフォルトへ戻す為.
	TYPC_SslCertInfo c_SslCertInfo = PnlComGetSslSetting()->getSslCertInfo( 0 );	// SSLの認証情報を取得.	// 304966 2008/6/23 12柳川 引数「0」(Defaultの証明書)を渡す.
	switch( option ) {
		case 1:													// はい.
		case BTNID_UserAndDepartSelectBoxProcess_BoxDelete:		// ----/ボックス削除.
			// 受付音を下記削除処理前に鳴動させる.
			callsendBuzzerType();
			setAuthenticationData();	// 設定値の保存処理.
			// 遷移元画面がセキュリティー強化解除確認の場合.
			if( PNLE_AdminUserAndDepartSynchroClear == PAdmincom::getDispInfoNum() ) {
				// CommonAPIへレベルを保存.
				PnlComGetSystemSetting()->setSecurityLevelUpOn( False, PnlComGetUser(), JLGE_InspectLogIfOperationPanel, False, PnlComGetAuthorityID() );
			}
			if( gp_AdminUserAndDepartSynchro.sc_UserAttest_At == 0 && gp_AdminUserAndDepartSynchro.sc_UserAttest != 0 ) {	// 【206_1】ユーザー認証ON時の送信者アドレスの変更禁止（デフォルト）  2008/00/00 制御12 高松.
				if( True == PLBC_FuncVersionCheck::isOK_Type1() ) {
					//Amur/Donau機能拡張1時.
// comment 2015/03/13 OPS1-Sky大野 [MZV-60] 認証アプリケーション登録時、「Fromアドレス変更」の設定を変更しない要望.
//					PnlComGetScanJobSetting()->setFromChange( TYPE_FromChange_Disable_UserAdress );
					if ( False == APIC_SoftDipSw::newInstance()->getSoftDipSwBitForPanelSpecNo( DEFSOFTWARESW215, 6 ) ) {
// comment 2017/11/23 【HCL-Suganya】【IT5_4.2】【RQ686】 Multiple Network Support.
//					PnlComGetScanJobSetting()->setFromChange( TYPE_FromChange_Disable_UserAdress );
					PnlComGetScanJobSetting()->setFromChange( TYPE_FromChange_Disable_UserAdress , PNLC_NetworkLan::getInstance() ->getAdminNWLine() );
					}
// comment 2015/03/13 OPS1-Sky大野 [MZV-60] 認証アプリケーション登録時、「Fromアドレス変更」の設定を変更しない要望.
				} else {
					//Donau基本時.
// comment 2017/11/23 【HCL-Suganya】【IT5_4.2】【RQ686】 Multiple Network Support.
//					PnlComGetScanJobSetting()->setFromChange( TYPE_FromChangeOnlyAdminAndUserAddress );
					PnlComGetScanJobSetting()->setFromChange( TYPE_FromChangeOnlyAdminAndUserAddress , PNLC_NetworkLan::getInstance() ->getAdminNWLine() );
				}
			}
			PCGC_RedialSetting* pc_FaxRedialSetting;
			pc_FaxRedialSetting = PCGC_RedialSetting::newInstance();
			if( NULL == pc_FaxRedialSetting ) {
//	comment 2015/11/02 HCL_Ezhil [ZeusS_ZX0][制御構成変更_No.76-1] Adding error code.
//Comment 2019/06/14 [HCLT_Monisha][EagleH][N Rank] Removed unwanted Detailed code.
//				LAPC_BFW_System *pc_BFW_System=LAPC_BFW_System::newInstance();
//				if (NULL !=pc_BFW_System){
//				    pc_BFW_System->registDetailCode(PNLD_DetailCode_Admin_0x1900020010414501);
//				}
				LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, ( char* )"Assert Failed!" );
				return;
			}
			pc_FaxRedialSetting->deleteRedialAddress();		//履歴から選択の宛先を削除.
// Comment 2017/09/07【HCLT-Narsimhmaswamy】【IT5 4.2】【RQ686】【WP595】Multiple Network support.
//			PnlComGetPrintJobSetting()->setPrintJobHoldMode( False, JLGE_InspectLogIfOperationPanel, True, PnlComGetUser(), PnlComGetAuthorityID() );
			PnlComGetPrintJobSetting()->setPrintJobHoldMode( False, JLGE_InspectLogIfOperationPanel, True, PnlComGetUser(), PnlComGetAuthorityID(),e_NWType );
			if( gp_AdminUserAndDepartSynchro.sc_UserAttest == 0 && gp_AdminUserAndDepartSynchro.b_Manage == False ) {
				// ユーザ認証、部門管理共にしない場合、ボックス管理者設定を不許可にする.
				PnlComGetUserAuthentication()->setEnableBoxAdmin( False, PnlComGetUser(), JLGE_InspectLogIfOperationPanel, False, PnlComGetAuthorityID() );
				// Box管理者を使用しない場合、Default値「PASSWORD」を入れる.
				pc_UserBoxAdmin->setUserPassword( "PASSWORD", PnlComGetUser(), TYPE_RegistrationTypeOnlyOne, JLGE_InspectLogIfOperationPanel, False, False, PnlComGetAuthorityID() );
			}
			//ユーザ認証しない時
			if( gp_AdminUserAndDepartSynchro.sc_UserAttest == 0 ) {
				if( TRUE == APIC_MyServerSetting::getRegistration( TYPE_MyServerTypeMyPanel ) ) {		// Myパネル(Huron)が有効【CQ.331292】第一OSW 酒井健.
					PnlComApplicationKeyReset( TYPE_ApplicationKeyIdMyPanel );
				}
			}

			// 遷移元画面がセキュリティー強化解除確認の場合.
			if( PNLE_AdminUserAndDepartSynchroClear == PAdmincom::getDispInfoNum() ) {
				setUserAndDepartBoxDelete( NULL, NULL, PNLE_BoxDeleteMode_AuthInfoInit, &gp_AdminSecurityReinforceEnd, bl_DeleteBox );	// ボックス削除開始処理.
			} else {
				if( True == PLBC_FuncVersionCheck::isOK_MiZe_Type5() ) {
// comment 2016/11/24 SCD2-Sky南波 [IT5_3.4][制御No.35] WP55.
//					if( ( True == m_bl_UbqStorageOn ) || ( True == m_bl_UbqClientOn ) ) {
					if( ( ( True == m_bl_UbqStorageOn ) || ( True == m_bl_UbqClientOn ) ) 
					 && ( True == PnlComGetUbiquitousSetting()->isNeedReboot() ) ) {
						setUserAndDepartBoxDelete( NULL, NULL, PNLE_BoxDeleteMode_AuthInfoInit, &gp_AdminHDDCodingWordSettingPowerOffOnForUserAuth, bl_DeleteBox );	// ボックス削除開始処理.
					} else {
						setUserAndDepartBoxDelete( NULL, NULL, PNLE_BoxDeleteMode_AuthInfoInit, &gp_AdminUserAndDepartSet, bl_DeleteBox );			// ボックス削除開始処理.
					}
				} else {
					setUserAndDepartBoxDelete( NULL, NULL, PNLE_BoxDeleteMode_AuthInfoInit, &gp_AdminUserAndDepartSet, bl_DeleteBox );		// ボックス削除開始処理.
				}
			}
			if( True == onlyPanel.getBrowserMode() ) {
				setTurnGamenThis( NULL );
			}
			setupGamen();
			break;
		case 0:
			setDispInfo( PNLE_AdminSecurityDispNON, NULL );	// セキュリティー強化モード解除確認画面用変数クリア.
			nextGamen = &gp_AdminUserAndDepartSynchro;	break;	// 管理者設定・ユーザ認証/部門管理・ユーザ/部門の連動認証.
		default:	break;	// for warning!
	}
}
//================================================================================================
// 概要		:管理者設定・ユーザ認証/部門管理・ユーザ/部門の連動認証・使用管理ﾃﾞｰﾀｸﾘｱ : 表示画面のセットアップ
// 戻り値	:なし
// 履歴		: 2016/01/08 SecV2-6 ボックス共有化キー非表示条件追加対応 Sky 新道.
//================================================================================================
void PAdminUserAndDepartDataClear::setupGamen( short option )
{
	PAdminUserAndDepartSynchro::setupGamen( option );
	if( False == bl_DispModeBoxProcess ) {
		Rbt_UtilityCheckYesNo.showEnable( True );						// はい・いいえキー表示.
		Rbt_UserDepart_SelectBoxProcess.showEnable( False );			// ボックス処理方法選択キー非表示.
		Rbt_UtilityCheckYesNo.select( uc_KeySelect );						// はい・いいえキー選択処理.

		if( True == PLBC_FuncVersionCheck::isOK_MiZe_Type2() ) {
			if( ( True == PLBC_FuncVersionCheck::isOK_MiZe_Type5() )				// 機能拡張5以降.
// comment 2016/11/24 SCD2-Sky南波 [IT5_3.4][制御No.35] WP55.
//			    && ( ( True == m_bl_UbqStorageOn ) || ( True == m_bl_UbqClientOn ) ) ) {
			    && ( ( True == m_bl_UbqStorageOn ) || ( True == m_bl_UbqClientOn ) ) 
			    && ( True == PnlComGetUbiquitousSetting()->isNeedReboot() ) ) {
				Txt_TitleAdminUserAndDepartDataClear.changeText( s_MSGAdminClearData );		// 全ての使用管理データをクリアしますか？\nクリアするには、主電源のOFF/ONが必要になります.
			} else {
				Txt_TitleAdminUserAndDepartDataClear.changeText( s_MSGAdminUserAndDepartDataClearMessage_2 );	// 使用管理データをクリアしますか?.
			}
		} else {
			Txt_TitleAdminUserAndDepartDataClear.changeText( s_MSGAdminUserAndDepartDataClearMessage );	// すべての使用管理データをクリアしますか?.
		}
	} else {
		Rbt_UtilityCheckYesNo.showEnable( False );						// はい・いいえキー非表示.
		Rbt_UserDepart_SelectBoxProcess.showEnable( True );				// ボックス処理方法選択キー表示.
		Rbt_UserDepart_SelectBoxProcess.select( uc_KeySelect );			// ボックス処理方法選択キー選択処理.
		Key_UserDepart_BoxSharing.showEnable(m_b_PublicShow);			// ボックス共有化キー表示.
		if( True == PLBC_FuncVersionCheck::isOK_MiZe_Type2() ) {
			if( ( True == PLBC_FuncVersionCheck::isOK_MiZe_Type5() )				// 機能拡張5以降.
// comment 2016/11/24 SCD2-Sky南波 [IT5_3.4][制御No.35] WP55.
//			    && ( ( True == m_bl_UbqStorageOn ) || ( True == m_bl_UbqClientOn ) ) ) {
			    && ( ( True == m_bl_UbqStorageOn ) || ( True == m_bl_UbqClientOn ) ) 
			    && ( True == PnlComGetUbiquitousSetting()->isNeedReboot() ) ) {
				Txt_TitleAdminUserAndDepartDataClear.changeText( s_MSGAdminDataClearedSelectBoxProcessMethod_PowerOnOff );	//すべての使用管理データをクリアします\n個人またはグループボックスが存在するため\nボックスの処理方法を選択してください\n処理には、主電源のOFF/ONが必要になります.
			} else {
				Txt_TitleAdminUserAndDepartDataClear.changeText( s_MSGAdminDataClearedSelectBoxProcessMethod_2 );			// 使用管理データをクリアします\個人またはグループボックスが存在するため\ボックスの処理方法を選択してください.
			}
		} else {
			Txt_TitleAdminUserAndDepartDataClear.changeText( s_MSGAdminDataClearedSelectBoxProcessMethod );			// すべての使用管理データをクリアします\個人またはグループボックスが存在するため\ボックスの処理方法を選択してください.
		}
	}
}

//////////////////////////////////////////////////////////////
// 概要		: 10925170   使用管理データクリア: ボタン（押された）.
// 戻り値	:
//////////////////////////////////////////////////////////////
void PAdminUserAndDepartDataClear::procButtonOn( uchar option )
{
	switch( option ) {												// ボックス無し時	｜　ボックス有り時.
		//=======================================
		case 0:	uc_KeySelect = 0;		break;						// いいえ			｜　ユーザー削除中止.
		case 1:	uc_KeySelect = 1;		break;						// はい				｜	ボックス共有化.
		case BTNID_UserAndDepartSelectBoxProcess_BoxDelete:			// －				｜　ボックス削除.
			uc_KeySelect = BTNID_UserAndDepartSelectBoxProcess_BoxDelete;
			break;
		default:	LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );	break;
	}
	setupGamen();
}
//////////////////////////////////////////////////////////////
// 概要		: 10925170   使用管理データクリア: ＯＫ.
// 戻り値	:
//////////////////////////////////////////////////////////////
void PAdminUserAndDepartDataClear::procOk( uchar option )
{
	if( 2 == uc_KeySelect )		{ bl_DeleteBox = True; }
	procGoToPage( uc_KeySelect );

// Comment 2017/09/12【HCLT-Sindhu】【IT5 4.2】【RQ686】【WP1012】Public user support for Network 2.
//	APIC_User* pc_public = APIC_User::newInstance( TYPD_IndexUserPublic );
	APIC_User* pc_public = NULL;
	if( TYPE_NetworkSpecific_NW1  ==  PNLC_NetworkLan::getInstance() ->getAdminNWLine( ) ) {
		pc_public = APIC_User::newInstance( TYPD_IndexUserPublic );
	} else {
		pc_public = APIC_User::newInstance( TYPD_IndexUserPublicNW2 );
	}
	if( NULL != pc_public ) {
		APIC_UserAuthentication* cp_UserAuthentication_wk = PnlComGetUserAuthentication();
		if( cp_UserAuthentication_wk ) {
// Comment 2017/09/12【HCLT-Sindhu】【IT5 4.2】【RQ686】【WP1012】Public user support for Network 2.
//			if( pc_public->getUserNo() == TYPD_IndexUserPublic ) {
			if( (pc_public->getUserNo() == TYPD_IndexUserPublic) || (pc_public->getUserNo() == TYPD_IndexUserPublicNW2) ) {
				TYPS_FunctionLimitation Limit_set;												// 機能制限.
				Limit_set = pc_public->getFunctionLimitation();
				//ユーザ認証なし、PUBLICUSER
// 	Comment 2017/09/28 [HCL-Harish]【IT5 4.2】【RQ-686】【WP509】Multiple Network support.
//				if( TYPE_UserAuthenticationOff == cp_UserAuthentication_wk->getUserAuthentication() ) {
				if( TYPE_UserAuthenticationOff == cp_UserAuthentication_wk->getUserAuthentication( PNLC_NetworkLan::getInstance() ->getAdminNWLine( ) ) ) {

					// Enable USB
					Limit_set.e_EnableUsbMemoryReading	 = TYPE_FuncLimit_Enable;
					Limit_set.e_EnableScanToUsb			 = TYPE_FuncLimit_Enable;


				}
				//ユーザ認証あり、PUBLICUSER
// 	Comment 2017/09/28 [HCL-Harish]【IT5 4.2】【RQ-686】【WP509】Multiple Network support.
//				else if( TYPE_UserAuthenticationOff != cp_UserAuthentication_wk->getUserAuthentication() ) {
				else if( TYPE_UserAuthenticationOff != cp_UserAuthentication_wk->getUserAuthentication( PNLC_NetworkLan::getInstance() ->getAdminNWLine( ) ) ) {
					// Enable USB
					Limit_set.e_EnableUsbMemoryReading	 = TYPE_FuncLimit_Disable;
					Limit_set.e_EnableScanToUsb			 = TYPE_FuncLimit_Disable;
				}
				pc_public->setFunctionLimitation( Limit_set, pc_public, JLGE_InspectLogIfNon, False, PnlComGetAuthorityID() );	// 機能制限保存.
				pc_public->setRegistration( True, TYPE_RegistrationTypeMax, JLGE_InspectLogIfNon, False, TYPE_Registration_State_New, False, PnlComGetUser(), PnlComGetAuthorityID() );
			}
		}
	}
}
//////////////////////////////////////////////////////////////
//概要    : 管理者設定・ユーザ認証/部門管理・ユーザ/部門の連動認証・使用管理ﾃﾞｰﾀｸﾘｱ(2715): 画面開始処理.
//備考    :
//履歴    : 2016/01/08 SecV2-6 ボックス共有化キー非表示条件追加対応 Sky 新道.
//////////////////////////////////////////////////////////////
void PAdminUserAndDepartDataClear::startGamen()
{
	PAdminToUserAndDepartSetcom::startGamen();
	uc_KeySelect = 0;						// キー選択.
	bl_DeleteBox = False;							// ボックス削除フラグ.
	// ユーザーがボックスを所有しているかのチェック.
	bl_DispModeBoxProcess = cp_BoxList->existUserGroupBox();

	if( True == PLBC_FuncVersionCheck::isOK_MiZe_Type5() ) {
		m_bl_UbqStorageOn = PnlComGetUbiquitousSetting()->getStorageOn();		// ユビキタスプリントストレージ設定.
		m_bl_UbqClientOn = PnlComGetUbiquitousSetting()->getClientOn();			// ユビキタスプリントクライアント設定.
	}
	if( True == bl_DispModeBoxProcess ){		// ボックスの処理方法を選択させるモード時.
		APIC_SystemSetting* pc_SystemSetting = APIC_SystemSetting::newInstance();
		if ( NULL == pc_SystemSetting ) {
			LAPC_BFW_System *pc_BFW_System=LAPC_BFW_System::newInstance();
			if ( NULL != pc_BFW_System ) {
	    		pc_BFW_System->registDetailCode(PNLD_DetailCode_Admin_0x1900650010457501);
			}
//Comment 2019/06/14 [HCLT_Monisha][EagleH][N Rank] Removed unwanted Detailed code.
//			LOGF_debug_PutEventLog(LOGF_DEBUG_EVENT_KIND_ALL, 0, 3, "Assert Failed!");
			return;
		}
		m_b_PublicShow = pc_SystemSetting->getEnableBoxSetting(TYPE_EnableBoxSettingKindPublicBox);	// ボックス共有化表示フラグ.
	}

	// 左エリア種類の有効設定（ジョブ表示（通常））startGamen()の最後でコールしてください.
	setLeftAreaEnableType( TYPD_LeftAreaBookmark | TYPD_LeftAreaSoft10Key );
}

//================================================================================================
// Brief :  Administrator Settings/User Authentication/Printer Counterlist/Print Item( H092061000).
// Returns : None.
//================================================================================================
void PAdminSystemDepartListPrintListPrintPrintItem::setupGamen( short option )
{
	PAdminSystemListAndCounterCom ::setupGamen();
	Rbt_UtilityListPrintPrintItemPrintItem.select( e_ListPrintInfo );
	Rbt_UtilityListPrintPrintItemCounterDetails.select( bl_isCounterDetails );
}
void PAdminSystemDepartListPrintListPrintPrintItem::startGamen( void )
{
	PAdminSystemListAndCounterCom ::startGamen();

	Txt_TitleAdminListPrintListPrint.changeText( s_TTLAdminPrintItem );
	Txt_MessageUtilityListPrint.changeText( s_MSGItemSelect );

	e_ListPrintInfo = PToAdminSetMenucom::getSelectPrintItem();
	bl_isCounterDetails = PToAdminSetMenucom::getSelectCounterDetails();

	setLeftAreaEnableType( TYPD_LeftAreaBookmark | TYPD_LeftAreaSoft10Key );
}
void PAdminSystemDepartListPrintListPrintPrintItem::procOk( uchar option )
{
	PToAdminSetMenucom::setSelectPrintItem( e_ListPrintInfo );
	PToAdminSetMenucom::setSelectCounterDetails( bl_isCounterDetails );
	nextGamen=&gp_AdminSystemDepartListPrintUserCounterListPrint;
}
void PAdminSystemDepartListPrintListPrintPrintItem::procButtonOn( uchar option )
{
	switch( option ) {
		case PNLD_BTNID_PAdminSystemDepartListPrint_PRINTAll:
			e_ListPrintInfo = TYPE_ListPrintInfoAll;
			break;
		case PNLD_BTNID_PAdminSystemDepartListPrint_PRINTOne:
			e_ListPrintInfo = TYPE_ListPrintInfoOnly;
			break;
		case PNLD_BTNID_PAdminSystemDepartListPrint_NotPrint:
			bl_isCounterDetails = False;
			break;
		case PNLD_BTNID_PAdminSystemDepartListPrint_Print:
			bl_isCounterDetails = True;
			break;
		default:
			break;
	}
}

//================================================================================================
// カウンタ用の変数.
//================================================================================================
int32_t i32_AllPrt_1_1 = 0;		//	トータル全色		（計算による算出）.
int32_t i32_AllPrt_1_2 = 0;		//	トータルブラック	（計算による算出）.
int32_t i32_AllPrt_1_3 = 0;		//	トータルカラー		（計算による算出）.
int32_t i32_AllPrt_1_4 = 0;		//	トータル２色		（計算による算出）.
int32_t i32_AllPrt_1_5 = 0;		//	トータルモノカラー	（計算による算出）.
int32_t i32_AllPrt_2_1 = 0;		//	トータル上限値.
int32_t i32_AllPrt_2_2 = 0;		//	ブラック上限値.
int32_t i32_AllPrt_2_3 = 0;		//	カラー上限値.
int32_t i32_AllPrt_3_1 = 0;		//	大サイズ全色		（計算による算出）.
int32_t i32_AllPrt_3_2 = 0;		//	大サイズブラック	（計算による算出）.
int32_t i32_AllPrt_3_3 = 0;		//	大サイズカラー		（計算による算出）.
int32_t i32_AllPrt_3_4 = 0;		//	大サイズ２色		（計算による算出）.
int32_t i32_AllPrt_3_5 = 0;		//	大サイズモノカラー	（計算による算出）.
int32_t i32_Manuscript = 0;		//	原稿枚数.
int32_t i32_CopyPaper  = 0;		//	用紙枚数.
// コピー.
int32_t i32_Copy_1_1 = 0; 		//	トータル全色	（計算による算出）.
int32_t i32_Copy_1_2 = 0;		//	トータルブラック.
int32_t i32_Copy_1_3 = 0;		//	トータルカラー.
int32_t i32_Copy_1_4 = 0;		//	トータル２色
int32_t i32_Copy_1_5 = 0;		//	トータルモノカラー.
int32_t i32_Copy_3_1 = 0;		//	大サイズ全色	（計算による算出）.
int32_t i32_Copy_3_2 = 0;		//	大サイズブラック.
int32_t i32_Copy_3_3 = 0;		//	大サイズカラー.
int32_t i32_Copy_3_4 = 0;		//	大サイズ２色.
int32_t i32_Copy_3_5 = 0;		//	大サイズモノカラー.
// プリンタ.
int32_t i32_Print_1_1 = 0;		//	トータル全色	（計算による算出）.
int32_t i32_Print_1_2 = 0;		//	トータルブラック.
int32_t i32_Print_1_3 = 0;		//	トータルカラー.
int32_t i32_Print_1_4 = 0;		//	トータル２色.
int32_t i32_Print_3_1 = 0;		//	大サイズ全色	（計算による算出）.
int32_t i32_Print_3_2 = 0;		//	大サイズブラック.
int32_t i32_Print_3_3 = 0;		//	大サイズカラー.
int32_t i32_Print_3_4 = 0;		//	大サイズ２色.
// スキャナ/ファクス.
int32_t i32_ScnFax_1_1 = 0;		//	トータル読取り.
int32_t i32_ScnFax_1_2 = 0;		//	トータルプリント(ﾌﾞﾗｯｸ).
int32_t i32_ScnFax_1_3 = 0;		//	トータルプリント(ｶﾗｰ).
int32_t i32_ScnFax_3_1 = 0;		//	大サイズ読み取り.
int32_t i32_ScnFax_3_2 = 0;		//	大サイズプリント(ﾌﾞﾗｯｸ).
int32_t i32_ScnFax_3_3 = 0;		//	大サイズプリント(ｶﾗｰ).
int32_t i32_FaxSend    = 0;		//	ファックス送信枚数.
// その他.
int32_t i32_Other2SideTotal    = 0;		//	両面印刷カウンター.
int32_t i32_OtherPrintPaperNum = 0;		//	印刷枚数カウンター.

//================================================================================================
//	部門名画面遷移時の部門名
//================================================================================================
char sc_DepName[TYPD_DeparNameMax+1];

//====================================================================== .
// 10889000コメント207.
//====================================================================== .
void PAdminPasswordWide::startGamen()
{
// comment 2020/11/30 DWS1-Sky延原 【TAS000000003634】リモートパネル使用時にパスワード入力のキー操作がハイライト表示されてしまう.
//// Comment 2019/01/28 【HCL_Krishna】【IT6_EagleH】【RQ-506】[WP-1711] Notify sys:Manager for stop Highlight display.
//	LIBG_EnterHighlightDisplayHideSection();
	PLBC_WideKeyboard::getInstance()->startWideKeyboard( PNLE_WideKeyboard_Normal, &Txt_UtilityAndAdminComTitle, PNLE_WideKeyboardExNone ) ;
	PAdminPassword::startGamen();
	m_bl_MaintenanceMode = False;
	char sc_AdminUserPassword[ UACD_MAX_UserPassword ];
	strncpy( sc_AdminUserPassword, sc_AdministratorDefaultPass, sizeof( sc_AdminUserPassword )-1 );
	Bool bl_AllowMaintenanceMode = False;
	Bool bl_MaintenanceModeEnable = False;
	APIC_SystemSetting* pc_SystemSetting = APIC_SystemSetting::newInstance();
	APIC_SystemInfo* pc_SystemInfo = APIC_SystemInfo::newInstance();
	bl_MaintenanceModeEnable = pc_SystemInfo->getEnableMaintenanceMode();
	bl_AllowMaintenanceMode = pc_SystemSetting->getAllowMaintenanceMode();
	if( ( True == bl_AllowMaintenanceMode ) && ( True == bl_MaintenanceModeEnable ) ) {
		Btx_InputDspAreaWide.setPassword( ( uchar* )sc_AdminUserPassword,sizeof( sc_AdminUserPassword ),False );
		setKeyboardData( sc_AdminUserPassword, DEF_AdminPasswordSize, TYPD_AdminKeybordMax );
		m_bl_MaintenanceMode = True;
	}
	PLBC_WideKeyboard::getInstance()->initInputArea( &Kbd_UtilityInputKeybordConv,&c_SlideTextMgr,sc_StrKeyboard, (char)Kbd_UtilityInputKeybordConv.getKeyboardDispLenMax(), ss_MaxInputNum,&Btx_InputDspAreaWide,getCharSetForPanel() ) ;	// スライド文字列管理クラスの初期化 .
}

//====================================================================== .
// 10889000コメント207.
//====================================================================== .
void PAdminPasswordWide::endGamen()
{
// comment 2020/11/30 DWS1-Sky延原 【TAS000000003634】リモートパネル使用時にパスワード入力のキー操作がハイライト表示されてしまう.
//// Comment 2019/01/28 【HCL_Krishna】【IT6_EagleH】【RQ-506】[WP-1711] Notify sys:Manager for stop Highlight display.
//    LIBG_LeaveHighlightDisplayHideSection();
	PLBC_WideKeyboard::getInstance()->endWideKeyboard() ;
	PAdminPassword::endGamen() ;
}

//====================================================================== .
// 10889000コメント207.
//====================================================================== .
void PAdminPasswordWide::procClear( uchar option )
{
// comment 2019/05/15 Chikuan [EagleH][OP_BTS-1792] V1遷移パフォーマンス改善.
	if( PNLE_WebUIScreenChangeStatusIdle != PNLC_WebUIControl_v1::getInstance()->getWebUIScreenChangeStatus() ){
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}

	PAdminPassword::procClear( option );
	PLBC_WideKeyboard::getInstance()->initInputArea_Clear() ;
	setupGamen();
}

//====================================================================== .
// 10889000コメント207.
//====================================================================== .
void PAdminPasswordWide::setDefaultBackParts()
{
	if( ( PLBD_Enl_KBD_Enlarge == Kbd_ScanKeybordConv.getChangeEnlKeybord() ) && ( True == onlyPanel.b_InitKeybord ) ) {
		setDefaultBackPartsInfo( ( PDisplayParts** )tbl_AdminPasswordDefaultWideEnl, ss_AdminPasswordDefaultWideEnl );	// 拡大用デフォルト背景.
	} else {
		setDefaultBackPartsInfo( ( PDisplayParts** )tbl_AdminPasswordDefaultWide, ss_AdminPasswordDefaultWide );	// 標準用デフォルト背景.
	}
}

//====================================================================== .
//* @brief  : Warning Check function.
//* @note   : None.
//* @param  : None.
//* @return : なし.
//====================================================================== .
void PAdminPasswordWide::procTimerWakeUp( uchar option )
{
	PAdminPassword::procTimerWakeUp( option );

	if( True == m_bl_MaintenanceMode ) {
// comment 2020/07/28【HCL-Selvaraj】【IT6_2.0MR3】【OP_BTS-26107】 Blocking to call procOK again when WebUIV1 opening is inprogress.
		if ( PNLE_WebUIScreenChangeStatusDuring == PNLC_WebUIControl_v1::getInstance()->getWebUIScreenChangeStatus() ) {
			return;
		}
		procOk( option );
	}
}

// comment 2023/03/03 FPT.ThanhLH11 [IT6_2.3][ALL MACHINE][OP_BTS-41923]一時的な言語変更で、OKキーを押した後に機械語で表示される管理者パスワード画面.
//================================================================================================
// 概要		:管理者モード・管理者パスワードエラー画面 : オブザーバー通知処理.
// 戻り値	:なし.
// 備考		:.
//================================================================================================
void PAdminPasswordWide::procObserver( TYPE_PNL_Observer e_PNL_Observer )
{
	switch( e_PNL_Observer ) {
		case TYPE_PNL_Observer_WebUINotifyChangeCompleteV1:
			if( True == PAdminPasswordWide::m_bl_pendingChangeLanguage ) {
				// オブザーバー通知.
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
				loadSelectLangType_IOP( PnlComGetLanguage() );						// 言語データ切り替え処理.
				PAdminPasswordWide::m_bl_pendingChangeLanguage = False;
			}
		default:
			PAdminPassword::procObserver( e_PNL_Observer );
			break;
	}
}

//====================================================================== .
// 10889000コメント209.
//====================================================================== .
void PAdminPasswordErrorWide::startGamen()
{
	PLBC_WideKeyboard::getInstance()->startWideKeyboard( PNLE_WideKeyboard_Normal, &Txt_UtilityAndAdminComTitle, PNLE_WideKeyboardExNone ) ;
	PAdminPasswordError::startGamen();
	PLBC_WideKeyboard::getInstance()->initInputArea( &Kbd_UtilityInputKeybordConv,&c_SlideTextMgr,sc_StrKeyboard, (char)Kbd_UtilityInputKeybordConv.getKeyboardDispLenMax(), ss_MaxInputNum,&Btx_InputDspAreaWide,getCharSetForPanel() ) ;	// スライド文字列管理クラスの初期化 .
}

//====================================================================== .
// 10889000コメント209.
//====================================================================== .
void PAdminPasswordErrorWide::endGamen()
{
	PLBC_WideKeyboard::getInstance()->endWideKeyboard() ;
	PAdminPasswordError::endGamen() ;
}

//====================================================================== .
// 10889000コメント209.
//====================================================================== .
void PAdminPasswordErrorWide::procClear( uchar option )
{
	PAdminPasswordError::procClear( option );
	PLBC_WideKeyboard::getInstance()->initInputArea_Clear() ;
	setupGamen();
}

//====================================================================== .
// 10889000コメント209.
//====================================================================== .
void PAdminPasswordErrorWide::setDefaultBackParts()
{
	if( ( PLBD_Enl_KBD_Enlarge == Kbd_ScanKeybordConv.getChangeEnlKeybord() ) && ( True == onlyPanel.b_InitKeybord ) ) {
		setDefaultBackPartsInfo( ( PDisplayParts** )tbl_AdminPasswordErrorDefaultWideEnl, ss_AdminPasswordErrorDefaultWideEnl );	// 拡大用デフォルト背景.
	} else {
		setDefaultBackPartsInfo( ( PDisplayParts** )tbl_AdminPasswordErrorDefaultWide, ss_AdminPasswordErrorDefaultWide );	// 標準用デフォルト背景.
	}
}

//====================================================================== .
// 概要		:10889000 管理者モード・管理者パスワードエラー画面 : 表示画面のセットアップ.
// 戻り値	:なし.
// 備考		:.
//====================================================================== .
void PAdminPasswordErrorWide::setupGamen( short option )
{
	PAdminPasswordError::setupGamen( option );

	if( gp_AdminPassword.b_isPenalty ) {
		// ペナルティ用メッセージに切り替える.
		Txt_MessageAdminPasswordError.changeText( s_MSGCertificatePenaltyAdminPasswordDisable );
	} else {
		// 通常エラーメッセージ.
		Txt_MessageAdminPasswordError.changeText( s_MSGPasswordNotMatch );
	}
}


//================================================================================================.
//［機能］	CSRCシステムとの初期接続状態を取得する.
//------------------------------------------------------------------------------------------------.
//［引数］	なし.
//［戻値］	True	CSRCと初期接続済み状態である.
//			False	CSRCと初期接続済み状態でない.
//				CSRC初期接続済み条件の判断制御をメソッド化した.
//================================================================================================.
Bool PAdminSetMenu::isConnectedCSRC() {
	// PnlComGetRdsSetting()はNULLとなりえない.よってNULLチェックしない.
	//   NVDC_NetworkSetting.cppより.
	// PnlComGetRds()はNULLとなりえない.よってNULLチェックしない.
	//   SYSC_RdsApi.cppより.
	Bool bl_Connected = False;
	if( TYPE_HostStatRgst == PnlComGetRdsSetting()->getHostStat() && False == PnlComGetRds()->isNowTrans( TYPE_RdsTrgKindIni ) ) {
		bl_Connected = True;
	}
	return ( bl_Connected );
}

//================================================================================================
// 概要		:管理者設定メニュー : キーの表示判定.
// 引数：PNLE_AdminMenu_KeyID e_Menu	機能キーの種類.
//     : PNLE_PARTSSTATUS keyStatus		表示/非表示.
// 戻り値.
//================================================================================================
void PAdminSetMenu::changeDispStatus( PNLE_AdminMenu_KeyID e_Menu, PNLE_PARTSSTATUS keyStatus ) {

//comment 2015/03/16 HCLT-Venkataramesh [HeliosL] [KRQ-101] Authentication function code button addition for function version 2 onwards.
	if ( False == PLBC_FuncVersionCheck::isOK_IT5_Type2() ){
		if ( e_Menu > PNLE_AdminMenu_LicenseManage ) {
			if( 0 != st_MenuIndex[e_Menu-1].uc_AdminMenu_Page && 0 != st_MenuIndex[e_Menu-1].uc_AdminMenu_Index ){
				pc_MenuUtilityAndAdminAdd3Page.changeSPCStatus( st_MenuIndex[e_Menu-1].uc_AdminMenu_Page, st_MenuIndex[e_Menu-1].uc_AdminMenu_Index, keyStatus );
			}
		} else {
			if( 0 != st_MenuIndex[e_Menu].uc_AdminMenu_Page && 0 != st_MenuIndex[e_Menu].uc_AdminMenu_Index ){
				pc_MenuUtilityAndAdminAdd3Page.changeSPCStatus( st_MenuIndex[e_Menu].uc_AdminMenu_Page, st_MenuIndex[e_Menu].uc_AdminMenu_Index, keyStatus );
			}
		}
	} else {
		// 表示指定したキーのページ、番号が0以外の場合表示処理を行う.
		if( 0 != st_MenuIndex[e_Menu].uc_AdminMenu_Page && 0 != st_MenuIndex[e_Menu].uc_AdminMenu_Index ){
			// キー表示情報の切り替え.
			pc_MenuUtilityAndAdminAdd3Page.changeSPCStatus( st_MenuIndex[e_Menu].uc_AdminMenu_Page, st_MenuIndex[e_Menu].uc_AdminMenu_Index, keyStatus );
		}
	}

}

//================================================================================================.
// 概要		:M092011020_拡張ユーザーデータの削除確認 : コンストラクタ.
//================================================================================================.
PAdminUserAndDepartLargeScaleUserDataClear::PAdminUserAndDepartLargeScaleUserDataClear( PDisplayParts** partsTable, short partsNumber, Bool isNoWarningGamen )
	: PAdminUserAndDepartSynchro( partsTable, partsNumber, isNoWarningGamen )
{
	uc_KeySelect = 0;
}
//================================================================================================.
// 概要		:M092011020_拡張ユーザーデータの削除確認 : 画面開始処理.
//================================================================================================.
void PAdminUserAndDepartLargeScaleUserDataClear::startGamen()
{
	PAdminToUserAndDepartSetcom::startGamen();
	uc_KeySelect = 0;						// キー選択.

	// 左エリア種類の有効設定.
	setLeftAreaEnableType( TYPD_LeftAreaBookmark | TYPD_LeftAreaSoft10Key );
}
//================================================================================================.
// 概要		:M092011020_拡張ユーザーデータの削除確認 : 表示画面のセットアップ.
//================================================================================================.
void PAdminUserAndDepartLargeScaleUserDataClear::setupGamen( short ss_option )
{
	PAdminUserAndDepartSynchro::setupGamen( ss_option );
	Rbt_UtilityCheckYesNo.select( uc_KeySelect );		// はい・いいえキー選択表示.
}
//================================================================================================.
// 概要		:M092011020_拡張ユーザーデータの削除確認 ボタン押下.
//================================================================================================.
void PAdminUserAndDepartLargeScaleUserDataClear::procButtonOn( uchar uc_option )
{
	switch( uc_option ) {
		case 0:	uc_KeySelect = 0;		break;						// いいえ.
		case 1:	uc_KeySelect = 1;		break;						// はい.
		default:	LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );	break;
	}
	setupGamen();
}
//================================================================================================.
// 概要		:M092011020_拡張ユーザーデータの削除確認 設定終了.
//================================================================================================.
void PAdminUserAndDepartLargeScaleUserDataClear::procOk( uchar uc_option )
{
	switch( uc_KeySelect ) {
		case 0:													// いいえ.
			nextGamen = &gp_AdminUserAndDepartSynchro;			// 管理者設定・ユーザ認証/部門管理.
			break;	
		case 1:													// はい.
			// 受付音を下記削除処理前に鳴動させる.
			callsendBuzzerType();
			
			// 設定値の保存.
			setAuthenticationData();
			
			// 拡張ユーザーデータ削除開始.
			setUserAndDepartBoxDelete( NULL, NULL, PNLE_BoxDeleteMode_LargeScaleDataClear, &gp_AdminUserAndDepartSet, False );
			if( True == onlyPanel.getBrowserMode() ) {
				setTurnGamenThis( NULL );
			}
			setupGamen();
			break;
		default:
			break;
	}
}

//	comment 2017/06/06 SCD2-sky延原 [IT5_4.1][AR1022609] AR1010594追加対策.
//================================================================================================.
// 概要		:個人ユーザーの確認印刷文書削除 CallBack.
//================================================================================================.
void ans_deleteConfirmationPrintUser( int32_t si32_Result )
{
	if ( APID_DocapiMethodOk != si32_Result ) {
		// 削除失敗.
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
	}
	// 削除完了通知をコールバックイベントに通知 .
	CallBackIndi( TYPE_PNL_CallBack_DeleteConfirmationPrintUserComplete );
}
//================================================================================================.
// 概要		:全ユーザーの確認印刷文書削除 CallBack.
//================================================================================================.
void ans_deleteConfirmationPrintAllUser( int32_t si32_Result )
{
	if ( APID_DocapiMethodOk != si32_Result ) {
		// 削除失敗.
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
	}
	// 削除完了通知をコールバックイベントに通知 .
	CallBackIndi( TYPE_PNL_CallBack_DeleteConfirmationPrintAllUserComplete );
}
